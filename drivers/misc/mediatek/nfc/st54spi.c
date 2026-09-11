// SPDX-License-Identifier: GPL-2.0
/*
 * ST54 secure-element SPI channel, reconstructed exclusively from the factory
 * S88 Pro kernel. This is not derived from another phone driver or from the
 * upstream spidev implementation.
 *
 * Factory ranges (semi-open):
 *   .text     [0xffffff800895c900, 0xffffff800895ede8)
 *   .init.text[0xffffff800937f188, 0xffffff800937f280)
 *
 * The names of private fields and of ioctl number 99 are not present in the
 * binary; neutral names ending in their observed offset/number are used.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st54spi.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/bitmap.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/platform_data/spi-mt65xx.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>

/*
 * These two functions are the observed one-way ABI with st21nfc.c:
 *   0xffffff800895ec4c: adrp x0, st54spi_st21nfc_cb
 *   0xffffff800895ec58: bl st21nfc_register_st54spi_cb
 *   0xffffff800895ec84: bl st21nfc_unregister_st54spi_cb
 */
extern void st21nfc_register_st54spi_cb(void (*cb)(int, void *), void *data);
extern void st21nfc_unregister_st54spi_cb(void);

#define ST54SPI_N_MINORS	1
#define ST54SPI_MODE_MASK	0x0fff
#define ST54SPI_FACTORY_FILE "/home/jenkins/jks-WTK6739_Q0_MP1_AP/alps/kernel-4.14/drivers/misc/mediatek/nfc/st54spi.c"

/* Number 99 and the 32-bit direction are visible in the dispatch constants. */
#define ST54SPI_IOC_RD_NR99	_IOR(SPI_IOC_MAGIC, 99, __u32)
#define ST54SPI_IOC_WR_NR99	_IOW(SPI_IOC_MAGIC, 99, __u32)

struct st54spi_data {
	dev_t devt;			/* +0x00 */
	spinlock_t spi_lock;		/* +0x04 */
	struct spi_device *spi;		/* +0x08 */
	struct spi_device *spi_saved;	/* +0x10 */
	struct list_head device_entry;	/* +0x18 */
	struct mutex buf_lock;		/* +0x28 */
	unsigned int users;		/* +0x48 */
	u8 *tx_buffer;			/* +0x50 */
	u8 *rx_buffer;			/* +0x58 */
	u32 speed_hz;			/* +0x60 */
	int power_gpio;			/* +0x64 */
	int request_68;			/* +0x68: callback-side request */
	int request_6c;			/* +0x6c: ioctl/open-side request */
	int power_state;			/* +0x70 */
};

static unsigned int bufsiz = 4096;
/*
 * The module_param is NOT decorative: it is the reason the factory calls
 * __kmalloc instead of the known-size allocation. By taking the address of
 * bufsiz in a struct of the __param section, clang can no longer prove nobody
 * writes it, and the size stops being a constant: "b944d100 ldr"@0xffffff800895df50
 * READS it from memory before "97e3c90b bl"@0xffffff800895df5c. Without it,
 * st54spi_open grows by 12 bytes.
 */
module_param(bufsiz, uint, S_IRUGO);
static DECLARE_BITMAP(minors, ST54SPI_N_MINORS);
static DEFINE_MUTEX(device_list_lock);
static LIST_HEAD(device_list);
static int st54spi_major;
static struct class *st54spi_class;

/*
 * Raw .data at 0xffffff80099555a0 has seven u32 zeroes except +0x10=12012.
 * probe writes ceil(109200000 / max_speed_hz) to +0x14 and +0x18:
 *   0xffffff800895ea6c: mov w9, #0x427f
 *   0xffffff800895ea70: movk w9, #0x682, lsl #16  (109199999)
 *   0xffffff800895ea84: udiv w8, w9, w8
 *   0xffffff800895ea88: stp w8, w8, [x10]
 */
static struct mtk_chip_config st54spi_chip_config = {
	.tx_mlsb = 0,
	.rx_mlsb = 0,
	.cs_pol = 0,
	.sample_sel = 0,
	.cs_setuptime = 12012,
	.cs_holdtime = 0,
	.cs_idletime = 0,
	.deassert_mode = 0,
};

static void st54spi_power_on(struct st54spi_data *st54spi);
static void st54spi_power_off(struct st54spi_data *st54spi);
static void st54spi_st21nfc_cb(int operation, void *data);

static __always_inline struct spi_device *
st54spi_get_spi(struct st54spi_data *st54spi)
{
	struct spi_device *spi;

	spin_lock_irq(&st54spi->spi_lock);
	spi = st54spi->spi;
	if (spi && !get_device(&spi->dev))
		spi = NULL;
	spin_unlock_irq(&st54spi->spi_lock);
	return spi;
}

/*
 * The inlined `sync`. The name is OURS -- the factory leaves no symbol for it,
 * as with st54spi_get_spi above -- but its existence and the type of its
 * `status` are readable in the binary:
 *   "12800d60 mov"@0xffffff800895ca14   puts -ESHUTDOWN in w0, 32-bit
 *   "35000080 cbnz"@0xffffff800895ca08  tests w0, 32-bit
 *   "93407c15 sxtw"@0xffffff800895ca1c  widens it to 64 bits in the caller
 * That is: inside, `status` is an int; only the return value is an ssize_t.
 * Keeping everything 64-bit in st54spi_read turns the comparison into a csel and
 * the function changes shape.
 *
 * And here NO device reference is taken: get_device and put_device, in the
 * image, appear only inside the two ioctls.
 */
static __always_inline ssize_t
st54spi_sync(struct st54spi_data *st54spi, struct spi_message *message)
{
	struct spi_device *spi;
	int status;

	spin_lock_irq(&st54spi->spi_lock);
	spi = st54spi->spi;
	spin_unlock_irq(&st54spi->spi_lock);

	if (!spi)
		status = -ESHUTDOWN;
	else
		status = spi_sync(spi, message);

	if (status == 0)
		status = message->actual_length;

	return status;
}

static ssize_t st54spi_read(struct file *filp, char __user *buf,
			    size_t count, loff_t *f_pos)
{
	struct st54spi_data *st54spi = filp->private_data;
	struct spi_transfer t = { .rx_buf = st54spi->rx_buffer,
				  .len = count,
				  .speed_hz = st54spi->speed_hz };
	struct spi_message m;
	ssize_t status;
	unsigned long missing;

	if (count > bufsiz)
		return -EMSGSIZE;
	printk(KERN_INFO "st54spi Read: %d bytes\n", (int)count);

	mutex_lock(&st54spi->buf_lock);
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	status = st54spi_sync(st54spi, &m);
	if (status > 0) {
		missing = copy_to_user(buf, st54spi->rx_buffer, status);
		if (missing == status)
			status = -EFAULT;
		else
			status -= missing;
	}
	mutex_unlock(&st54spi->buf_lock);
	printk(KERN_INFO "st54spi Read: status: %d\n", (int)status);
	return status;
}

static ssize_t st54spi_write(struct file *filp, const char __user *buf,
			     size_t count, loff_t *f_pos)
{
	struct st54spi_data *st54spi = filp->private_data;
	struct spi_transfer t = { .tx_buf = st54spi->tx_buffer,
				  .len = count,
				  .speed_hz = st54spi->speed_hz };
	struct spi_message m;
	ssize_t status;
	unsigned long missing;

	if (count > bufsiz)
		return -EMSGSIZE;
	printk(KERN_INFO "st54spi Write: %d bytes\n", (int)count);

	mutex_lock(&st54spi->buf_lock);
	missing = copy_from_user(st54spi->tx_buffer, buf, count);
	if (missing) {
		status = -EFAULT;
		goto out;
	}

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	status = st54spi_sync(st54spi, &m);
out:
	mutex_unlock(&st54spi->buf_lock);
	printk(KERN_INFO "st54spi Write: status: %d\n", (int)status);
	return status;
}

static noinline struct spi_ioc_transfer *
st54spi_get_ioc_message(unsigned int cmd,
			struct spi_ioc_transfer __user *u_ioc,
			unsigned int *n_ioc)
{
	struct spi_ioc_transfer *ioc;
	size_t size;

	if ((cmd & 0xc000ffffU) != 0x40006b00U)
		return ERR_PTR(-ENOTTY);
	size = _IOC_SIZE(cmd);
	if (size % sizeof(*ioc))
		return ERR_PTR(-EINVAL);
	*n_ioc = size / sizeof(*ioc);
	if (!*n_ioc)
		return NULL;

	ioc = kmalloc(size, GFP_KERNEL);
	if (!ioc)
		return ERR_PTR(-ENOMEM);
	if (__copy_from_user(ioc, u_ioc, size)) {
		kfree(ioc);
		return ERR_PTR(-EFAULT);
	}
	return ioc;
}

static int st54spi_message(struct st54spi_data *st54spi,
			   struct spi_ioc_transfer *u_xfers,
			   unsigned int n_xfers)
{
	struct spi_message msg;
	struct spi_transfer *k_xfers, *k_tmp;
	struct spi_ioc_transfer *u_tmp;
	struct spi_device *spi;
	u8 *tx_buf = st54spi->tx_buffer;
	u8 *rx_buf = st54spi->rx_buffer;
	unsigned int total = 0;
	int status = -EFAULT;

	k_xfers = kcalloc(n_xfers, sizeof(*k_xfers), GFP_KERNEL);
	if (!k_xfers)
		return -ENOMEM;

	spi_message_init(&msg);
	for (u_tmp = u_xfers, k_tmp = k_xfers;
	     u_tmp < u_xfers + n_xfers; u_tmp++, k_tmp++) {
		if (u_tmp->len > INT_MAX - total) {
			status = -EMSGSIZE;
			goto done;
		}
		total += u_tmp->len;

		if (u_tmp->rx_buf) {
			if (u_tmp->len > bufsiz - (rx_buf - st54spi->rx_buffer)) {
				status = -EMSGSIZE;
				goto done;
			}
			k_tmp->rx_buf = rx_buf;
			rx_buf += u_tmp->len;
		}
		if (u_tmp->tx_buf) {
			unsigned long missing;

			if (u_tmp->len > bufsiz - (tx_buf - st54spi->tx_buffer)) {
				status = -EMSGSIZE;
				goto done;
			}
			k_tmp->tx_buf = tx_buf;
			missing = copy_from_user(tx_buf,
					(const void __user *)(uintptr_t)u_tmp->tx_buf,
					u_tmp->len);
			if (missing)
				goto done;
			tx_buf += u_tmp->len;
		}

		k_tmp->len = u_tmp->len;
		k_tmp->cs_change = !!u_tmp->cs_change;
		k_tmp->tx_nbits = u_tmp->tx_nbits;
		k_tmp->rx_nbits = u_tmp->rx_nbits;
		k_tmp->bits_per_word = u_tmp->bits_per_word;
		k_tmp->delay_usecs = u_tmp->delay_usecs;
		k_tmp->speed_hz = u_tmp->speed_hz ? u_tmp->speed_hz :
						      st54spi->speed_hz;
		spi_message_add_tail(k_tmp, &msg);
		dev_dbg(&st54spi->spi->dev,
			"  xfer len %u %s%s%s%dbits %u usec %uHz\n",
			u_tmp->len, u_tmp->rx_buf ? "rx " : "",
			u_tmp->tx_buf ? "tx " : "",
			u_tmp->cs_change ? "cs " : "",
			u_tmp->bits_per_word, u_tmp->delay_usecs,
			k_tmp->speed_hz);
	}

	/*
	 * NO device reference here: the factory takes get_device/put_device ONLY in
	 * the two ioctls. In st54spi_read, st54spi_write and st54spi_message the
	 * pointer is read under the lock and that is all -- having counted the calls
	 * each function makes in the image, read/write/message have one
	 * _raw_spin_lock_irq and one _raw_spin_unlock_irq and nothing else. Going
	 * through st54spi_get_spi costs three more calls and eight bytes.
	 */
	spin_lock_irq(&st54spi->spi_lock);
	spi = st54spi->spi;
	spin_unlock_irq(&st54spi->spi_lock);
	if (!spi) {
		status = -ESHUTDOWN;
		goto done;
	}
	status = spi_sync(spi, &msg);
	if (status < 0)
		goto done;

	rx_buf = st54spi->rx_buffer;
	for (u_tmp = u_xfers; u_tmp < u_xfers + n_xfers; u_tmp++) {
		if (!u_tmp->rx_buf)
			continue;
		if (copy_to_user((void __user *)(uintptr_t)u_tmp->rx_buf,
				 rx_buf, u_tmp->len)) {
			status = -EFAULT;
			goto done;
		}
		rx_buf += u_tmp->len;
	}
	status = total;
done:
	kfree(k_xfers);
	return status;
}

static long st54spi_ioctl(struct file *filp, unsigned int cmd,
			  unsigned long arg)
{
	struct st54spi_data *st54spi = filp->private_data;
	struct spi_device *spi;
	struct spi_ioc_transfer *ioc;
	unsigned int n_ioc;
	u32 tmp;
	u32 save32;
	u16 save_mode;
	u8 save8;
	int retval = 0;

	if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC)
		return -ENOTTY;
	if ((_IOC_DIR(cmd) & _IOC_READ) &&
	    !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd)))
		return -EFAULT;
	if ((_IOC_DIR(cmd) & _IOC_WRITE) &&
	    !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd)))
		return -EFAULT;

	spi = st54spi_get_spi(st54spi);
	printk(KERN_INFO "st54spi ioctl cmd %d\n", cmd);
	if (!spi)
		return -ESHUTDOWN;

	mutex_lock(&st54spi->buf_lock);
	switch (cmd) {
	case SPI_IOC_RD_MODE:
		tmp = spi->mode & ST54SPI_MODE_MASK;
		retval = put_user((u8)tmp, (__u8 __user *)arg);
		break;
	case SPI_IOC_RD_LSB_FIRST:
		tmp = !!(spi->mode & SPI_LSB_FIRST);
		retval = put_user((u8)tmp, (__u8 __user *)arg);
		break;
	case SPI_IOC_RD_BITS_PER_WORD:
		retval = put_user(spi->bits_per_word, (__u8 __user *)arg);
		break;
	case SPI_IOC_RD_MAX_SPEED_HZ:
		retval = put_user(st54spi->speed_hz, (__u32 __user *)arg);
		break;
	case SPI_IOC_RD_MODE32:
		tmp = spi->mode & ST54SPI_MODE_MASK;
		retval = put_user(tmp, (__u32 __user *)arg);
		break;
	case ST54SPI_IOC_RD_NR99:
		/*
		 * The value sits INSIDE the put_user: __put_user_check evaluates
		 * access_ok() and the pointer mask before (x), and the
		 * factory reads the gpio after the barrier.  With a scratch
		 * variable the read would rise above the check.
		 * "b94066c0 ldr"@0xffffff800895cf6c after
		 * "d503229f hint"@0xffffff800895cf68
		 */
		retval = put_user(st54spi->power_gpio ?
					  gpio_get_value(st54spi->power_gpio) : 0,
				  (__u32 __user *)arg);
		break;

	/*
	 * The two cases are ONE in the binary: four entries converge on the same
	 * block, two of which pass through "12001d48 and"@0xffffff800895d680 (the
	 * truncation to 8 bits of the get_user).  The factory has four bl spi_setup,
	 * not five.  And the dev_dbg receives the MERGED value, not the original tmp:
	 * "2a1603e3 mov"@0xffffff800895dcc8 passes w22, which comes from
	 * "2a090116 orr"@0xffffff800895dc94.
	 */
	case SPI_IOC_WR_MODE:
	case SPI_IOC_WR_MODE32:
		if (cmd == SPI_IOC_WR_MODE)
			retval = get_user(tmp, (__u8 __user *)arg);
		else
			retval = get_user(tmp, (__u32 __user *)arg);
		if (retval)
			break;
		if (tmp & ~ST54SPI_MODE_MASK) {
			retval = -EINVAL;
			break;
		}
		save_mode = spi->mode;
		tmp |= spi->mode & ~ST54SPI_MODE_MASK;
		spi->mode = (u16)tmp;
		retval = spi_setup(spi);
		if (retval < 0)
			spi->mode = save_mode;
		else
			dev_dbg(&spi->dev, "spi mode %x\n", tmp);
		break;
	case SPI_IOC_WR_LSB_FIRST:
		retval = get_user(tmp, (__u8 __user *)arg);
		if (retval)
			break;
		save_mode = spi->mode;
		if (tmp)
			spi->mode |= SPI_LSB_FIRST;
		else
			spi->mode &= ~SPI_LSB_FIRST;
		retval = spi_setup(spi);
		if (retval < 0)
			spi->mode = save_mode;
		else
			dev_dbg(&spi->dev, "%csb first\n", tmp ? 'l' : 'm');
		break;
	case SPI_IOC_WR_BITS_PER_WORD:
		retval = get_user(tmp, (__u8 __user *)arg);
		if (retval)
			break;
		save8 = spi->bits_per_word;
		spi->bits_per_word = tmp;
		retval = spi_setup(spi);
		if (retval < 0)
			spi->bits_per_word = save8;
		else
			dev_dbg(&spi->dev, "%d bits per word\n", tmp);
		break;
	case SPI_IOC_WR_MAX_SPEED_HZ:
		retval = get_user(tmp, (__u32 __user *)arg);
		if (retval)
			break;
		save32 = spi->max_speed_hz;
		spi->max_speed_hz = tmp;
		retval = spi_setup(spi);
		if (retval >= 0)
			st54spi->speed_hz = tmp;
		else
			dev_dbg(&spi->dev, "%d Hz (max)\n", tmp);
		spi->max_speed_hz = save32;
		break;
	case ST54SPI_IOC_WR_NR99:
		retval = get_user(tmp, (__u32 __user *)arg);
		if (retval)
			break;
		/*
		 * A FACTORY DEFECT reproduced: st54spi comes from
		 * filp->private_data, which open has already filled in; the
		 * check cannot fail.  The factory has it all the same:
		 * "b40002d6 cbz"@0xffffff800895d9b0 jumps to the common branch
		 * that sets retval to zero and calls dev_dbg regardless.
		 */
		if (st54spi) {
			printk(KERN_INFO "st54spi sehal pwr_req: %d\n", !!tmp);
			/*
			 * The constant sits in the branches, not outside: the factory
			 * does "320003e9 orr"@0xffffff800895d9d0 (the value 1)
			 * and "b9006edf str"@0xffffff800895d9ec (wzr), not a
			 * !!tmp computed once.
			 */
			if (tmp) {
				st54spi->request_6c = 1;
				if (!st54spi->power_state)
					st54spi_power_on(st54spi);
			} else {
				st54spi->request_6c = 0;
				if (st54spi->power_state == 1 &&
				    !st54spi->request_68)
					st54spi_power_off(st54spi);
			}
		}
		retval = 0;
		/*
		 * the RAW tmp, not !!tmp: "2a1703e3 mov"@0xffffff800895dcf0
		 * passes w23, the same register as the "34000117 cbz"
		 * @0xffffff800895d9c8.  The printk above normalises it instead.
		 */
		dev_dbg(&spi->dev, "SE_POWER_REQ set: %d\n", tmp);
		break;
	default:
		ioc = st54spi_get_ioc_message(cmd,
				(struct spi_ioc_transfer __user *)arg, &n_ioc);
		if (IS_ERR(ioc)) {
			retval = PTR_ERR(ioc);
			break;
		}
		if (!ioc)
			break;
		retval = st54spi_message(st54spi, ioc, n_ioc);
		kfree(ioc);
		break;
	}
	mutex_unlock(&st54spi->buf_lock);
	put_device(&spi->dev);
	printk(KERN_INFO "st54spi ioctl retval %d\n", retval);
	return retval;
}

#ifdef CONFIG_COMPAT
static long st54spi_compat_ioctl(struct file *filp, unsigned int cmd,
				 unsigned long arg)
{
	struct st54spi_data *st54spi;
	struct spi_device *spi;
	struct spi_ioc_transfer *ioc, *p;
	unsigned int n_ioc, i;
	/*
	 * int, not long: the factory closes with "93407e80 sxtw"
	 * @0xffffff800895de8c, which extends w20 to 64 bits.  With a long
	 * the register would already be 64-bit and the extension absent.
	 */
	int retval = 0;
	void __user *argp = compat_ptr(arg);

	if ((cmd & 0xc000ffffU) != 0x40006b00U)
		return st54spi_ioctl(filp, cmd, (unsigned long)argp);
	if (!access_ok(VERIFY_READ, argp, _IOC_SIZE(cmd)))
		return -EFAULT;

	st54spi = filp->private_data;
	spi = st54spi_get_spi(st54spi);
	printk(KERN_INFO "st54spi compat_ioctl cmd %d\n", cmd);
	if (!spi)
		return -ESHUTDOWN;

	mutex_lock(&st54spi->buf_lock);
	ioc = st54spi_get_ioc_message(cmd, argp, &n_ioc);
	if (IS_ERR(ioc)) {
		retval = PTR_ERR(ioc);
	} else if (!ioc) {
		retval = 0;
	} else {
		/*
		 * This section was reconstructed from the factory kernel disassembly (0xffffff800895de3c).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st54spi.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		for (p = ioc, i = n_ioc; i; i--, p++) {
			p->rx_buf = (u32)p->rx_buf;
			p->tx_buf = (u32)p->tx_buf;
		}
		retval = st54spi_message(st54spi, ioc, n_ioc);
		kfree(ioc);
	}
	mutex_unlock(&st54spi->buf_lock);
	put_device(&spi->dev);
	printk(KERN_INFO "st54spi compat_ioctl retval %d\n", (int)retval);
	return retval;
}
#endif

static int st54spi_open(struct inode *inode, struct file *filp)
{
	struct st54spi_data *st54spi = NULL, *iter;
	int status = -ENXIO;

	mutex_lock(&device_list_lock);
	list_for_each_entry(iter, &device_list, device_entry) {
		if (iter->devt == inode->i_rdev) {
			st54spi = iter;
			break;
		}
	}
	if (!st54spi) {
		pr_debug("st54spi: nothing for minor %d\n", iminor(inode));
		goto out;
	}
	printk(KERN_INFO "st54spi: open\n");

	if (!st54spi->tx_buffer) {
		st54spi->tx_buffer = kmalloc(bufsiz, GFP_KERNEL);
		if (!st54spi->tx_buffer) {
			status = -ENOMEM;
			goto out;
		}
	}
	if (!st54spi->rx_buffer) {
		st54spi->rx_buffer = kmalloc(bufsiz, GFP_KERNEL);
		if (!st54spi->rx_buffer) {
			kfree(st54spi->tx_buffer);
			st54spi->tx_buffer = NULL;
			status = -ENOMEM;
			goto out;
		}
	}
	st54spi->users++;
	filp->private_data = st54spi;
	nonseekable_open(inode, filp);
	mutex_unlock(&device_list_lock);
	printk(KERN_INFO "st54spi sehal pwr_req: %d\n", 1);
	st54spi->request_6c = 1;
	if (!st54spi->power_state) {
		printk(KERN_INFO "st54spi: open - force power on\n");
		st54spi_power_on(st54spi);
	}
	return 0;
out:
	mutex_unlock(&device_list_lock);
	return status;
}

static int st54spi_release(struct inode *inode, struct file *filp)
{
	struct st54spi_data *st54spi;
	struct spi_device *spi, *spi_saved;

	mutex_lock(&device_list_lock);
	st54spi = filp->private_data;
	filp->private_data = NULL;
	printk(KERN_INFO "st54spi: release\n");
	st54spi->users--;
	if (!st54spi->users) {
		printk(KERN_INFO "st54spi: release - may allow power off\n");
		if (st54spi) {
			printk(KERN_INFO "st54spi sehal pwr_req: %d\n", 0);
			st54spi->request_6c = 0;
			if (st54spi->power_state == 1 && !st54spi->request_68)
				st54spi_power_off(st54spi);
		}
		kfree(st54spi->tx_buffer);
		st54spi->tx_buffer = NULL;
		kfree(st54spi->rx_buffer);
		st54spi->rx_buffer = NULL;

		/*
		 * `spi_saved` is read INSIDE the lock and tested OUTSIDE, on the
		 * branch where `spi` is null -- not together with `spi` before
		 * unlocking. The factory has two unlocks, one per branch:
		 *   "f9400a75 ldr"@0xffffff800895e0fc   (spi_saved into x21,
		 *                                        before the unlock)
		 *   "b5000075 cbnz"@0xffffff800895e108  (tested AFTERWARDS)
		 * The form `if (!spi && !spi_saved)` before the unlock costs one
		 * branch more and one saved register less.
		 */
		spin_lock_irq(&st54spi->spi_lock);
		spi = st54spi->spi;
		if (spi) {
			st54spi->speed_hz = spi->max_speed_hz;
			spin_unlock_irq(&st54spi->spi_lock);
		} else {
			spi_saved = st54spi->spi_saved;
			spin_unlock_irq(&st54spi->spi_lock);
			if (!spi_saved)
				kfree(st54spi);
		}
	}
	mutex_unlock(&device_list_lock);
	return 0;
}

static const struct file_operations st54spi_fops = {
	.owner = THIS_MODULE,
	.read = st54spi_read,
	.write = st54spi_write,
	.unlocked_ioctl = st54spi_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = st54spi_compat_ioctl,
#endif
	.open = st54spi_open,
	.release = st54spi_release,
	.llseek = no_llseek,
};

/*
 * st54spi_power_on() was reconstructed from the factory kernel disassembly (0xffffff800895e694).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st54spi.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void st54spi_power_on(struct st54spi_data *st54spi)
{
	/*
	 * DECLARE_COMPLETION() was reconstructed from the factory kernel disassembly (0xffffff800895e730).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st54spi.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	DECLARE_COMPLETION(attesa_mai_usata);

	if (st54spi->power_gpio)
		gpio_set_value(st54spi->power_gpio, 1);
	usleep_range(1500, 2000);

	if (st54spi->spi_saved) {
		spin_lock_irq(&st54spi->spi_lock);
		st54spi->spi = st54spi->spi_saved;
		st54spi->spi_saved = NULL;
		spin_unlock_irq(&st54spi->spi_lock);
	}

	if (st54spi->spi) {
		struct spi_transfer t = { .speed_hz = st54spi->speed_hz };
		struct spi_message m;
		struct spi_device *spi;

		spi_message_init(&m);
		spi_message_add_tail(&t, &m);
		printk(KERN_INFO "%s : restored polarity and force 1 xfer\n",
		       __func__);

		spin_lock_irq(&st54spi->spi_lock);
		spi = st54spi->spi;
		spin_unlock_irq(&st54spi->spi_lock);

		if (spi)
			spi_sync(spi, &m);
	}
	st54spi->power_state = 1;
}

static void st54spi_power_off(struct st54spi_data *st54spi)
{
	struct spi_device *spi;
	struct mtk_chip_config polarity_cfg = { };
	void *controller_data;
	struct spi_transfer t = { .speed_hz = st54spi->speed_hz };
	struct spi_message m;
	/*
	 * DECLARE_COMPLETION() was reconstructed from the factory kernel disassembly (0xffffff800895e890).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st54spi.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	DECLARE_COMPLETION(attesa_mai_usata);

	spi = st54spi->spi;
	if (spi) {
		controller_data = spi->controller_data;
		if (controller_data) {
			/* The first four u32 are copied; the remaining four are zero. */
			polarity_cfg.tx_mlsb =
				((struct mtk_chip_config *)controller_data)->tx_mlsb;
			polarity_cfg.rx_mlsb =
				((struct mtk_chip_config *)controller_data)->rx_mlsb;
			polarity_cfg.cs_pol =
				!((struct mtk_chip_config *)controller_data)->cs_pol;
			polarity_cfg.sample_sel =
				((struct mtk_chip_config *)controller_data)->sample_sel;
			printk(KERN_INFO "%s : change NSS polarity to %d\n",
			       __func__, polarity_cfg.cs_pol);
			spi->controller_data = &polarity_cfg;
			spi_message_init(&m);
			spi_message_add_tail(&t, &m);
			/*
			 * THE SECOND READ OF `spi` IS UNDER THE LOCK AND
			 * IS NOT `st54spi_get_spi`: after the `printk`
			 * ("97df531a bl"@0xffffff800895e86c) the binary
			 * re-reads the field without the lock
			 * ("f9400668 ldr"@0xffffff800895e870) and then takes the
			 * lock ("94146120 bl"@0xffffff800895e898) to
			 * re-read it into x20 ("f9400674 ldr"@0xffffff800895e89c)
			 * and release it ("94146181 bl"@0xffffff800895e8a4).
			 * In between there is NO `get_device`.
			 */
			spin_lock_irq(&st54spi->spi_lock);
			spi = st54spi->spi;
			spin_unlock_irq(&st54spi->spi_lock);
			if (spi)
				spi_sync(spi, &m);
			spi->controller_data = controller_data;
		}
	}
	if (st54spi->power_gpio)
		gpio_set_value(st54spi->power_gpio, 0);

	/*
	 * THE GUARD SITS OUTSIDE THE LOCK, as in `st54spi_power_on`:
	 * "f9400668 ldr"@0xffffff800895e8d4 reads `spi` and
	 * "b4000108 cbz"@0xffffff800895e8d8 skips the whole section BEFORE
	 * "9414610d bl"@0xffffff800895e8e4, which is `_raw_spin_lock_irq`.
	 */
	if (st54spi->spi) {
		spin_lock_irq(&st54spi->spi_lock);
		st54spi->spi_saved = st54spi->spi;
		st54spi->spi = NULL;
		spin_unlock_irq(&st54spi->spi_lock);
	}
	usleep_range(2000, 4500);
	st54spi->power_state = 0;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
static int st54spi_probe(struct spi_device *spi)
{
	struct st54spi_data *st54spi;
	struct mtk_chip_config *chip = spi->controller_data;
	struct device *dev;
	struct device_node *np;
	unsigned long minor;
	int gpio, ret;
	int status = 0;

	st54spi = kzalloc(sizeof(*st54spi), GFP_KERNEL);
	if (!st54spi)
		return -ENOMEM;
	st54spi->spi = spi;
	spin_lock_init(&st54spi->spi_lock);
	mutex_init(&st54spi->buf_lock);
	INIT_LIST_HEAD(&st54spi->device_entry);
	BUILD_BUG_ON(sizeof(*st54spi) != 0x78);

	mutex_lock(&device_list_lock);
	minor = find_first_zero_bit(minors, ST54SPI_N_MINORS);
	if (minor >= ST54SPI_N_MINORS) {
		dev_dbg(&spi->dev, "no minor number available!\n");
		status = -ENODEV;
	} else {
		st54spi->devt = MKDEV(st54spi_major, minor);
		dev = device_create(st54spi_class, &spi->dev, st54spi->devt,
				    st54spi, "st54spi");
		if (IS_ERR(dev))
			status = PTR_ERR(dev);
		else {
			set_bit(minor, minors);
			list_add(&st54spi->device_entry, &device_list);
		}
	}
	mutex_unlock(&device_list_lock);

	st54spi->speed_hz = spi->max_speed_hz;
	st54spi_chip_config.cs_holdtime =
		(109199999U + spi->max_speed_hz) / spi->max_speed_hz;
	st54spi_chip_config.cs_idletime = st54spi_chip_config.cs_holdtime;
	if (chip) {
		chip->cs_setuptime = st54spi_chip_config.cs_setuptime;
		chip->cs_holdtime = st54spi_chip_config.cs_holdtime;
		chip->cs_idletime = st54spi_chip_config.cs_idletime;
		dev_dbg(&spi->dev, "Replaced chip_info!\n");
	} else {
		spi->controller_data = &st54spi_chip_config;
		dev_dbg(&spi->dev, "Added into chip_info!\n");
	}
	if (status) {
		kfree(st54spi);
	} else {
		spi_set_drvdata(spi, st54spi);
	}

	np = of_find_compatible_node(NULL, NULL, "st,st54spi");
	if (np) {
		gpio = of_get_named_gpio(np, "gpio-power-std", 0);
		if (gpio < 0)
			printk(KERN_INFO "%s : power request failed (%d)\n",
			       ST54SPI_FACTORY_FILE, gpio);
		else
			st54spi->power_gpio = gpio;
	} else {
		/* The stock call supplies only the %s argument; keep that defect. */
		printk(KERN_INFO "%s: get ST54 failed (%d)",
		       "st54spi_parse_dt");
		printk(KERN_INFO "%s : get num err.\n", "st54spi_parse_dt");
	}
	printk(KERN_INFO "[dsc]%s : get power_gpio[%d]\n",
	       "st54spi_parse_dt", st54spi->power_gpio);
	if (st54spi->power_gpio) {
		ret = gpio_request(st54spi->power_gpio, "gpio-power-std");
		if (ret)
			printk(KERN_INFO "%s : power request failed (%d)\n",
			       ST54SPI_FACTORY_FILE, ret);
		printk(KERN_INFO "%s : power GPIO = %d\n",
		       __func__, st54spi->power_gpio);
		ret = gpio_direction_output(st54spi->power_gpio, 1);
		if (ret)
			printk(KERN_INFO "%s : reset direction_output failed\n",
			       ST54SPI_FACTORY_FILE);
		gpio_set_value(st54spi->power_gpio, 1);
	}
	printk(KERN_INFO "%s : Register with st21nfc driver, %p\n",
	       __func__, st54spi);
	st21nfc_register_st54spi_cb(st54spi_st21nfc_cb, st54spi);
	return status;
}
#pragma GCC diagnostic pop

static int st54spi_remove(struct spi_device *spi)
{
	struct st54spi_data *st54spi = spi_get_drvdata(spi);

	printk(KERN_INFO "%s : Unregister from st21nfc driver\n", __func__);
	st21nfc_unregister_st54spi_cb();
	spin_lock_irq(&st54spi->spi_lock);
	st54spi->spi = NULL;
	st54spi->spi_saved = NULL;
	spin_unlock_irq(&st54spi->spi_lock);

	mutex_lock(&device_list_lock);
	list_del(&st54spi->device_entry);
	device_destroy(st54spi_class, st54spi->devt);
	clear_bit(MINOR(st54spi->devt), minors);
	if (!st54spi->users)
		kfree(st54spi);
	mutex_unlock(&device_list_lock);
	return 0;
}

static void st54spi_st21nfc_cb(int operation, void *data)
{
	struct st54spi_data *st54spi = data;

	if (!st54spi)
		return;
	printk(KERN_INFO "%s : dir %d data %p\n", __func__, operation, data);
	switch (operation) {
	case 0:
		usleep_range(4000, 8000);
		st54spi_power_on(st54spi);
		break;
	case 1:
		if (st54spi->power_state)
			st54spi_power_off(st54spi);
		break;
	case 2:
		st54spi->request_68 = 0;
		if (st54spi->power_state == 1 && !st54spi->request_6c)
			st54spi_power_off(st54spi);
		break;
	case 3:
		st54spi->request_68 = 1;
		if (!st54spi->power_state)
			st54spi_power_on(st54spi);
		break;
	}
}

static const struct of_device_id st54spi_of_match[] = {
	{ .compatible = "st,st54spi" },
	{ }
};

static struct spi_driver st54spi_driver = {
	.driver = {
		.name = "st54spi",
		.owner = THIS_MODULE,
		.of_match_table = st54spi_of_match,
	},
	.probe = st54spi_probe,
	.remove = st54spi_remove,
};

static int __init st54spi_init(void)
{
	int status;

	printk(KERN_INFO "Loading st54spi driver\n");
	st54spi_major = register_chrdev(0, "spi", &st54spi_fops);
	printk(KERN_INFO "Loading st54spi driver, major: %d\n", st54spi_major);
	st54spi_class = class_create(THIS_MODULE, "spidev");
	if (IS_ERR(st54spi_class)) {
		unregister_chrdev(st54spi_major, "spi");
		return PTR_ERR(st54spi_class);
	}
	status = spi_register_driver(&st54spi_driver);
	if (status < 0) {
		class_destroy(st54spi_class);
		unregister_chrdev(st54spi_major, "spi");
	}
	printk(KERN_INFO "Loading st54spi driver: %d\n", status);
	return status;
}
module_init(st54spi_init);

/*
 * st54spi_exit() was reconstructed from the factory kernel disassembly (0xffffff80093a8518).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st54spi.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void __exit st54spi_exit(void)
{
	spi_unregister_driver(&st54spi_driver);
	class_destroy(st54spi_class);
	unregister_chrdev(st54spi_major, st54spi_driver.driver.name);
}
module_exit(st54spi_exit);

MODULE_LICENSE("GPL");
