// SPDX-License-Identifier: GPL-2.0
/*
 * ST54 secure-element SPI channel, reconstructed exclusively from the
 * factory S88 Pro kernel.  This is not derived from another phone driver or
 * from the upstream spidev implementation.
 *
 * Factory ranges (semi-open):
 *   .text     [0xffffff800895c900, 0xffffff800895ede8)
 *   .init.text[0xffffff800937f188, 0xffffff800937f280)
 *
 * The names of private fields and of ioctl number 99 are not present in the
 * binary.  Neutral names ending in their observed offset/number are used
 * below; their behaviour, not their original spelling, is observable.
 *
 * ------------------------------------------------------------------------
 * STATO DELLA MISURA, 2026-08-23: 9 funzioni su 14 uguali al byte, e il
 * numero e' SCESO da 11 mentre il codice migliorava. Va detto per intero,
 * perche' altrimenti sembra un peggioramento.
 *
 * Chiuse in questo giro: st54spi_ioctl (era +48), st54spi_compat_ioctl
 * (era +8) e st54spi_release (era -4), tutte e tre ora esatte.
 *
 * Poi tre correzioni, ognuna letta nel binario, hanno RIAPERTO st54spi_read
 * e st54spi_write, che erano esatte:
 *
 *   1. `bufsiz` e' un module_param. La fabbrica la LEGGE dalla memoria
 *      ("b944d100 ldr"@0xffffff800895df50 in open, "b944d108 ldr"
 *      @0xffffff800895c928 in read); senza il module_param clang la ripiega
 *      a 4096 e l'allocazione cambia funzione. Avvicina open (da +12 a +4)
 *      e message (da -56 a -48), e ALLONTANA read e write, che erano esatte
 *      e diventano +8: li' c'era un secondo scarto che il primo
 *      COMPENSAVA, lo stesso schema di aw22127.
 *   2. read, write e message NON prendono il riferimento al dispositivo:
 *      censite le chiamate funzione per funzione nell'immagine, get_device e
 *      put_device compaiono solo dentro le due ioctl. E' un difetto di
 *      fabbrica (niente protegge il puntatore per la durata del
 *      trasferimento) ed e' riprodotto.
 *   3. Dentro la `sync` incorporata `status` e' un int, non un ssize_t:
 *      "93407c15 sxtw"@0xffffff800895ca1c lo allarga nel chiamante.
 *
 * Cosa RESTA, e non e' stato trovato: read -16, write -24, message -84. Il
 * telaio di fabbrica di st54spi_read e' 304 byte ("d104c3ff sub"
 * @0xffffff800895c900), il nostro 272, e la fabbrica ha due scritture in
 * piu' oltre la fine della zona che azzeriamo. Provato e NON risolutivo:
 * spostare l'inizializzatore della spi_transfer dopo il controllo su bufsiz,
 * e chiuderlo in un blocco interno dopo il mutex_lock -- clang lo risolleva
 * in tutti e due i casi, zero byte di differenza. Le strutture spi_transfer
 * e spi_message di questo albero ALPS non mostrano campi in piu' o in meno
 * a occhio, quindi la causa dei 32 byte di telaio non e' stata isolata.
 * ------------------------------------------------------------------------
 *
 * Every literal used by the implementation is tied to its bytes in
 * stock.elf.  KERN_INFO contributes the leading SOH+'6' bytes at the cited
 * address; debug strings have no KERN prefix:
 *
 * "Loading st54spi driver\n"@0xffffff80092205fd
 * "Loading st54spi driver, major: %d\n"@0xffffff8009220617
 * "spidev"@0xffffff800922063c
 * "Loading st54spi driver: %d\n"@0xffffff8009220643
 * "st54spi Read: %d bytes\n"@0xffffff8009220661
 * "st54spi Read: status: %d\n"@0xffffff800922067b
 * "st54spi Write: %d bytes\n"@0xffffff8009220697
 * "st54spi Write: status: %d\n"@0xffffff80092206b2
 * "st54spi ioctl cmd %d\n"@0xffffff80092206cf
 * "spi mode %x\n"@0xffffff800922074e
 * "%csb first\n"@0xffffff800922075b
 * "%d bits per word\n"@0xffffff8009220767
 * "%d Hz (max)\n"@0xffffff8009220779
 * "SE_POWER_REQ set: %d\n"@0xffffff8009220786
 * "st54spi ioctl retval %d\n"@0xffffff800922079c
 * "st54spi sehal pwr_req: %d\n"@0xffffff80092207b7
 * "%s : restored polarity and force 1 xfer\n"@0xffffff80092207d4
 * "%s : change NSS polarity to %d\n"@0xffffff8009220810
 * "  xfer len %u %s%s%s%dbits %u usec %uHz\n"@0xffffff8009220854
 * "rx "@0xffffff800922087d
 * ""@0xffffff8009220880
 * "tx "@0xffffff8009220881
 * "cs "@0xffffff8009220885
 * "st54spi compat_ioctl cmd %d\n"@0xffffff8009220889
 * "st54spi compat_ioctl retval %d\n"@0xffffff80092208a8
 * "st54spi: nothing for minor %d\n"@0xffffff80092208d7
 * "st54spi: open\n"@0xffffff80092208f6
 * "st54spi: open - force power on\n"@0xffffff8009220907
 * "st54spi: release\n"@0xffffff8009220929
 * "st54spi: release - may allow power off\n"@0xffffff800922093d
 * "no minor number available!\n"@0xffffff8009220988
 * "Replaced chip_info!\n"@0xffffff80092209a4
 * "Added into chip_info!\n"@0xffffff80092209b9
 * "gpio-power-std"@0xffffff80092209d0
 * "%s : power request failed (%d)\n"@0xffffff80092209df
 * "%s : power GPIO = %d\n"@0xffffff8009220a01
 * "%s : reset direction_output failed\n"@0xffffff8009220a19
 * "%s : Register with st21nfc driver, %p\n"@0xffffff8009220a3f
 * "st,st54spi"@0xffffff8009220a68
 * "st54spi"@0xffffff8009220a6b
 * "%s: get ST54 failed (%d)"@0xffffff8009220a73
 * "%s : get num err.\n"@0xffffff8009220a8e
 * "st54spi_parse_dt"@0xffffff8009220aa3
 * "[dsc]%s : get power_gpio[%d]\n"@0xffffff8009220ab4
 * "%s : dir %d data %p\n"@0xffffff8009220ad4
 * "%s : Unregister from st21nfc driver\n"@0xffffff8009220afe
 * "spi"@0xffffff800922621d
 * "/home/jenkins/jks-WTK6739_Q0_MP1_AP/alps/kernel-4.14/drivers/misc/mediatek/nfc/st54spi.c"@0xffffff80092206f5
 *
 * The verifier exceptions are both non-runtime tokens: "GPL" is
 * MODULE_LICENSE metadata whose built-in .modinfo is discarded, while
 * "-Wformat" belongs only to the local diagnostic pragma required to retain
 * the stock printk call with its missing integer argument.
 *
 * Instruction anchors for the non-string facts on which the reconstruction
 * depends (encoding and address are both checked by verificaistruzioni.py):
 *
 * "mov x21,#0xffffffffffffffa6"@0xffffff800895c934  read: -EMSGSIZE
 * "mov w0,#0xffffff94"@0xffffff800895ca14           read: -ESHUTDOWN
 * "mov x9,#0xfffffffffffffff2"@0xffffff800895ca88   read: -EFAULT
 * "mov x20,#0xffffffffffffffa6"@0xffffff800895cb10  write: -EMSGSIZE
 * "mov w0,#0xffffff94"@0xffffff800895cc54           write: -ESHUTDOWN
 * "mov x20,#0xfffffffffffffff2"@0xffffff800895ccb4  write: -EFAULT
 * "and w9,w1,#0xff00"@0xffffff800895cce0            ioctl magic
 * "mov w10,#0x6b00"@0xffffff800895cce4              ioctl magic
 * "orr x0,xzr,#0xffffffffffffffe7"@0xffffff800895ce54 ioctl: -ENOTTY
 * "and w8,w0,#0xc000ffff"@0xffffff800895e148        message mask
 * "tst x20,#0x1f"@0xffffff800895e15c               32-byte user record
 * "orr w8,wzr,#0x60"@0xffffff800895e234             96-byte kernel record
 * "add x23,x23,#0x20"@0xffffff800895e438            next user record
 * "add x25,x25,#0x60"@0xffffff800895e444            next kernel record
 * "mov w20,#0xffffff94"@0xffffff800895e588          message: -ESHUTDOWN
 * "mov w0,#0x5dc"@0xffffff800895e688                power-on sleep low
 * "mov w1,#0x7d0"@0xffffff800895e68c                power-on sleep high
 * "mov w0,#0x7d0"@0xffffff800895e8f8                power-off sleep low
 * "mov w1,#0x1194"@0xffffff800895e8fc               power-off sleep high
 * "orr w2,wzr,#0x78"@0xffffff800895e964             private allocation
 * "orr w1,wzr,#0x1"@0xffffff800895e9b4              one minor
 * "mov w0,#0xfa0"@0xffffff800895ed84                callback sleep low
 * "mov w1,#0x1f40"@0xffffff800895ed88               callback sleep high
 * "orr w2,wzr,#0x1"@0xffffff800937f1b4              one registered minor
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
 * Il module_param NON e' decorativo: e' la ragione per cui la fabbrica chiama
 * __kmalloc invece dell'allocazione a dimensione nota. Prendendo l'indirizzo
 * di bufsiz in una struct della sezione __param, clang non puo' piu'
 * dimostrare che nessuno la scriva, e la dimensione smette di essere una
 * costante: "b944d100 ldr"@0xffffff800895df50 la LEGGE dalla memoria prima di
 * "97e3c90b bl"@0xffffff800895df5c. Senza, st54spi_open cresce di 12 byte.
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
 * La `sync` incorporata. Il nome e' NOSTRO -- la fabbrica non ne lascia il
 * simbolo, come per st54spi_get_spi qui sopra -- ma la sua esistenza e il tipo
 * del suo `status` si leggono nel binario:
 *   "12800d60 mov"@0xffffff800895ca14   mette -ESHUTDOWN in w0, a 32 bit
 *   "35000080 cbnz"@0xffffff800895ca08  prova w0, a 32 bit
 *   "93407c15 sxtw"@0xffffff800895ca1c  lo allarga a 64 bit nel chiamante
 * Cioe': dentro, `status` e' un int; solo il valore di ritorno e' ssize_t.
 * Tenendo tutto a 64 bit in st54spi_read il confronto diventa un csel e la
 * funzione cambia forma.
 *
 * E qui NON si prende il riferimento al dispositivo: get_device e put_device,
 * nell'immagine, compaiono solo dentro le due ioctl.
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

	/* NIENTE riferimento al dispositivo qui: la fabbrica prende
	 * get_device/put_device SOLO nelle due ioctl. In st54spi_read,
	 * st54spi_write e st54spi_message il puntatore si legge sotto il
	 * lock e basta -- censite le chiamate di ciascuna funzione
	 * nell'immagine, read/write/message hanno un _raw_spin_lock_irq e
	 * un _raw_spin_unlock_irq e nient'altro. Passare da
	 * st54spi_get_spi vale tre chiamate in piu' e otto byte. */
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
		/* Il valore sta DENTRO la put_user: __put_user_check valuta
		 * access_ok() e la maschera del puntatore prima di (x), e la
		 * fabbrica legge il gpio dopo la barriera.  Con una variabile
		 * d'appoggio la lettura salirebbe sopra il controllo.
		 * "b94066c0 ldr"@0xffffff800895cf6c dopo
		 * "d503229f hint"@0xffffff800895cf68 */
		retval = put_user(st54spi->power_gpio ?
					  gpio_get_value(st54spi->power_gpio) : 0,
				  (__u32 __user *)arg);
		break;

	/* I due casi sono UNO SOLO nel binario: quattro ingressi confluiscono
	 * sullo stesso blocco, due dei quali passano da "12001d48 and"
	 * @0xffffff800895d680 (la troncatura a 8 bit della get_user).  La
	 * fabbrica ha quattro bl spi_setup, non cinque.  E la dev_dbg riceve
	 * il valore FUSO, non il tmp originale: "2a1603e3 mov"@0xffffff800895dcc8
	 * passa w22, che viene da "2a090116 orr"@0xffffff800895dc94. */
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
		/* DIFETTO DI FABBRICA riprodotto: st54spi viene da
		 * filp->private_data, che la open ha gia' riempito; il
		 * controllo non puo' fallire.  La fabbrica pero' ce l'ha:
		 * "b40002d6 cbz"@0xffffff800895d9b0 salta al ramo comune
		 * che mette retval a zero e chiama la dev_dbg lo stesso. */
		if (st54spi) {
			printk(KERN_INFO "st54spi sehal pwr_req: %d\n", !!tmp);
			/* La costante sta nei rami, non fuori: la fabbrica
			 * fa "320003e9 orr"@0xffffff800895d9d0 (il valore 1)
			 * e "b9006edf str"@0xffffff800895d9ec (wzr), non un
			 * !!tmp calcolato una volta sola. */
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
		/* tmp GREZZO, non !!tmp: "2a1703e3 mov"@0xffffff800895dcf0
		 * passa w23, lo stesso registro del "34000117 cbz"
		 * @0xffffff800895d9c8.  La printk sopra invece normalizza. */
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
	/* int, non long: la fabbrica chiude con "93407e80 sxtw"
	 * @0xffffff800895de8c, che estende w20 a 64 bit.  Con un long
	 * il registro sarebbe gia' a 64 bit e l'estensione non ci sarebbe. */
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
		/* Il ciclo cammina col PUNTATORE e conta ALL'INDIETRO.
		 * La fabbrica chiude il corpo con "f1000529 subs"
		 * @0xffffff800895de3c e "54ffff61 b.ne"@0xffffff800895de48:
		 * un solo decremento che mette le bandiere, senza cmp.
		 * Con "for (i = 0; i < n_ioc; i++)" clang tiene un contatore
		 * che sale e confronta con n_ioc, e sono quattro byte in piu'.
		 *
		 * E rx viene PRIMA di tx: "b940010a ldr"@0xffffff800895de34
		 * legge l'offset 8 (rx_buf), poi "b85f810b ldur"
		 * @0xffffff800895de38 l'offset 0 (tx_buf). */
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

		/* `spi_saved` si legge DENTRO il lock e si prova FUORI, nel
		 * ramo in cui `spi` e' nullo -- non insieme a `spi` prima di
		 * sbloccare. La fabbrica ha due sblocchi, uno per ramo:
		 *   "f9400a75 ldr"@0xffffff800895e0fc   (spi_saved in x21,
		 *                                        prima dello sblocco)
		 *   "b5000075 cbnz"@0xffffff800895e108  (provato DOPO)
		 * La forma `if (!spi && !spi_saved)` prima dello sblocco vale
		 * un ramo in piu' e un registro salvato in meno. */
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
 * DUE SEZIONI DI LUCCHETTO, NON UNA, e la guardia della prima sta FUORI dal
 * lucchetto. La forma qui sotto viene dal binario, non dalla simmetria:
 *
 *   "f9400a68 ldr"@0xffffff800895e694 legge `spi_saved` (+16) e
 *   "b4000108 cbz"@0xffffff800895e698 salta la sezione intera se e' nullo --
 *   la prova e' PRIMA di "9414619d bl"@0xffffff800895e6a4, che e'
 *   `_raw_spin_lock_irq`. Dentro, "a900fe68 stp"@0xffffff800895e6b0 scrive
 *   in una volta sola `spi = spi_saved` (+8) e `spi_saved = NULL` (+16).
 *
 *   "f9400668 ldr"@0xffffff800895e6b8 rilegge `spi` (+8) SENZA lucchetto e
 *   "b4000588 cbz"@0xffffff800895e6bc salta tutto il resto se e' nullo.
 *
 *   La SECONDA sezione di lucchetto viene DOPO la `printk`
 *   ("97df536a bl"@0xffffff800895e72c): "94146173 bl"@0xffffff800895e74c
 *   prende il lucchetto, "f9400674 ldr"@0xffffff800895e750 rilegge `spi` in
 *   x20, "941461d4 bl"@0xffffff800895e758 lo rilascia. NON c'e' nessuna
 *   `get_device` in mezzo, quindi NON e' `st54spi_get_spi`.
 *
 *   `t` e `m` sono azzerate DENTRO il blocco, non in testa alla funzione:
 *   le sei "stp xzr, xzr" a 0xffffff800895e6c0..0xffffff800895e6d4 e le
 *   cinque a 0xffffff800895e704..0xffffff800895e714 stanno tutte DOPO il
 *   "b4000588 cbz", e "b940626a ldr"@0xffffff800895e6d8 legge `speed_hz`
 *   (+96) li' dentro.
 */
static void st54spi_power_on(struct st54spi_data *st54spi)
{
	/*
	 * UNA `struct completion` SULLA PILA, INIZIALIZZATA E MAI LETTA.
	 * E' un difetto di fabbrica, ed e' riprodotto (regola 7).
	 *
	 * Il binario la mostra per intero. Il telaio di fabbrica e' 0x120 contro
	 * i nostri 0x100 -- trentadue byte in piu' -- e queste cinque istruzioni,
	 * che il nostro build non aveva, la costruiscono:
	 *
	 *   "d10123a8 sub"@0xffffff800895e730    x8 = x29 - 0x48
	 *   "91004108 add"@0xffffff800895e738    x8 = x29 - 0x38
	 *   "b81b83bf stur"@0xffffff800895e744   [x29-0x48] = 0   (done)
	 *   "b81c03bf stur"@0xffffff800895e740   [x29-0x40] = 0   (wait.lock)
	 *   "a93ca3a8 stp"@0xffffff800895e748    [x29-0x38] = x8, x8
	 *
	 * L'ultima e' la firma: un `list_head` che punta a se' stesso, cioe'
	 * `INIT_LIST_HEAD`, e sta a x29-0x38 che e' proprio il valore di x8. Coi
	 * due interi azzerati prima, la disposizione e' `done` a +0, `wait.lock`
	 * a +8, `wait.head` a +16: e' `struct completion`.
	 *
	 * NON E' MAI RILETTA -- in tutta la funzione nessuna istruzione tocca
	 * x29-0x48..-0x30 dopo l'inizializzazione -- e il suo indirizzo non esce:
	 * x8 serve solo a scrivere dentro se stessa. La fabbrica salva anche x28
	 * e non lo usa mai, che e' l'altro segno dello stesso residuo.
	 *
	 * IL NOME E' LA CONVENZIONE DEL KERNEL per una completion sulla pila, non
	 * una misura: e' una variabile locale, e un nome nel binario non ce l'ha
	 * in nessun caso.
	 *
	 * MISURATO: senza, `st54spi_power_on` sta a 316 contro 336 (-20) e
	 * `st54spi_power_off` a 388 contro 408 (-20). Con, il residuo scende a
	 * +4: resta una sola istruzione, ed e' la forma con cui i trentadue byte
	 * vengono azzerati (noi due `stp xzr, xzr`, lei due `stur wzr` da quattro
	 * byte piu' il paio).
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
	 * LA STESSA `struct completion` MAI USATA DI `st54spi_power_on`, con lo
	 * stesso difetto e la stessa prova. Qui sta a x29-0x58 invece che a
	 * x29-0x48, e il telaio di fabbrica e' 0x150 contro i nostri 0x130:
	 *
	 *   "b81a83bf stur"@0xffffff800895e890   [x29-0x58] = 0   (done)
	 *   "b81b03bf stur"@0xffffff800895e88c   [x29-0x50] = 0   (wait.lock)
	 *   "a93babaa stp"@0xffffff800895e894    [x29-0x48] = x10, x10
	 *
	 * dove x10 vale x29-0x48, cioe' l'indirizzo di quello stesso slot:
	 * `INIT_LIST_HEAD`. Anche qui la fabbrica salva x28
	 * ("f9008bfc str"@0xffffff800895e7a4) e non lo usa mai.
	 *
	 * MISURATO: senza, 388 contro 408 (-20); con, il residuo scende a +4.
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
			 * LA SECONDA LETTURA DI `spi` E' SOTTO LUCCHETTO E
			 * NON E' `st54spi_get_spi`: dopo la `printk`
			 * ("97df531a bl"@0xffffff800895e86c) il binario
			 * rilegge il campo senza lucchetto
			 * ("f9400668 ldr"@0xffffff800895e870) e poi prende il
			 * lucchetto ("94146120 bl"@0xffffff800895e898) per
			 * rileggerlo in x20 ("f9400674 ldr"@0xffffff800895e89c)
			 * e rilasciarlo ("94146181 bl"@0xffffff800895e8a4).
			 * In mezzo NON c'e' nessuna `get_device`.
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
	 * LA GUARDIA STA FUORI DAL LUCCHETTO, come in `st54spi_power_on`:
	 * "f9400668 ldr"@0xffffff800895e8d4 legge `spi` e
	 * "b4000108 cbz"@0xffffff800895e8d8 salta la sezione intera PRIMA di
	 * "9414610d bl"@0xffffff800895e8e4, che e' `_raw_spin_lock_irq`.
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
 * La funzione di uscita, trovata dal censimento della `.exit.text` del
 * 2026-08-21.  Non era mai stata cercata: `oracolo/stock.map` e' `nm -n` di
 * uno stock.elf la cui symtab viene dalla tabella kallsyms dell'immagine, e
 * kallsyms -- con `# CONFIG_KALLSYMS_ALL is not set` nel config di fabbrica --
 * emette solo i simboli in [_stext,_etext] e [_sinittext,_einittext]
 * (alps-mtkwatch/scripts/kallsyms.c righe 48-51).  `.exit.text` comincia
 * esattamente a `_einittext` = 0xffffff80093a8518, dove la mappa finisce; su
 * arm64 il linker non la scarta mai (vmlinux.lds.S righe 18-19 e 147,
 * `ARM_EXIT_KEEP(x)` incondizionato).
 *
 * 0xffffff80093ac044, 76 byte:
 *   "f81e0ff3 str"@0xffffff80093ac044   str x19, [sp,#-32]!
 *   "9113e273 add"@0xffffff80093ac054   add x19,x19,#0x4f8 -> 0xffffff80099554f8
 *                                       = 0xffffff80099554d8 + 0x20, cioe'
 *                                       &st54spi_driver.driver: st54spi_driver
 *                                       e' a 0xffffff80099554d8, lo stesso
 *                                       indirizzo che st54spi_init passa a
 *                                       __spi_register_driver
 *                                       ("91136021 add"@0xffffff800937f228)
 *   "97c50924 bl"@0xffffff80093ac05c    -> ffffff80084ee4ec <driver_unregister>
 *                                       cioe' l'inline spi_unregister_driver
 *   "f942b900 ldr"@0xffffff80093ac064   ldr x0,[x8,#1392] -> 0xffffff800a0dc570
 *                                       = st54spi_class, la stessa cella che
 *                                       st54spi_init scrive
 *                                       ("f902baa0 str"@0xffffff800937f1fc)
 *   "97c509f9 bl"@0xffffff80093ac068    -> ffffff80084ee84c <class_destroy>
 *   "b9457900 ldr"@0xffffff80093ac070   ldr w0,[x8,#1400] -> 0xffffff800a0dc578
 *                                       = st54spi_major
 *   "f9400263 ldr"@0xffffff80093ac074   ldr x3,[x19] -- **una LETTURA**, non un
 *                                       adrp+add di un letterale: il nome
 *                                       passato a __unregister_chrdev e'
 *                                       st54spi_driver.driver.name, il campo a
 *                                       spiazzamento 0 di struct device_driver
 *   "321803e2 orr"@0xffffff80093ac078   orr w2,wzr,#0x100 = 256
 *   "2a1f03e1 mov"@0xffffff80093ac07c   mov w1, wzr = 0
 *   "97bb0a17 bl"@0xffffff80093ac080    -> ffffff800826e8dc <__unregister_chrdev>
 *                                       che con (0, 256, name) e' esattamente
 *                                       l'inline unregister_chrdev()
 *   "d65f03c0 ret"@0xffffff80093ac08c
 *
 * Nessuna printk: questi 76 byte non contengono nessun riferimento a .rodata.
 *
 * **IL NOME e' una scelta**, non una misura: senza printk non c'e' `__func__`,
 * e la stringa "st54spi_exit" non compare in nessun punto dell'immagine
 * (0 occorrenze su tutti i byte).  `st54spi_exit` e' il nome simmetrico a
 * `st54spi_init`.  Anche il `module_exit()` e' dichiarato come scelta: il
 * puntatore che genera sta in `.exitcall.exit`, scartata da vmlinux.lds
 * (`EXIT_CALL` dentro /DISCARD/, riga 98), mentre la funzione sopravvive
 * comunque perche' per un built-in `__exit` implica `__used`
 * (include/linux/init.h righe 78-85).
 *
 * MISURA DI DIMENSIONE NON FATTA: questo lotto non ha un albero di build per
 * st54spi.  I 76 byte di fabbrica sono letti dal disassemblato; il confronto
 * con il nostro `.o` va ancora fatto.
 */
static void __exit st54spi_exit(void)
{
	spi_unregister_driver(&st54spi_driver);
	class_destroy(st54spi_class);
	unregister_chrdev(st54spi_major, st54spi_driver.driver.name);
}
module_exit(st54spi_exit);

MODULE_LICENSE("GPL");
