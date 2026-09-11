// SPDX-License-Identifier: GPL-2.0
/*
 * STMicroelectronics ST21NFC driver (I2C/NCI side), Doogee S88 Pro.
 *
 * Reconstructed from the disassembly of the factory kernel. Not a line comes
 * from a public ST source: every constant, every structure offset and every
 * literal carries the disassembly line it was read from.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st21nfc_st21nfc.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st21nfc_st21nfc.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/time.h>

/*
 * clk_buf_ctrl(): st21nfc_dev_open and st21nfc_release call it
 *   "97effb30 bl" (0xffffff800895c73c) -> ffffff800855b3fc <clk_buf_ctrl>
 * It is already in our ALPS build (drivers/misc/mediatek/base/power/spm_v2 and
 * friends); its header is not among the exported ones, so it is declared here.
 */
extern int clk_buf_ctrl(unsigned int id, bool onoff);

/*
 * The maximum length of the read/write buffer.
 *
 * "f104105f cmp" (0xffffff800895bbd0)   cmp x2, #0x104           (st21nfc_dev_read, count)
 * "52802088 mov" (0xffffff800895bbe0)   mov w8, #0x104           // #260
 * "f104105f cmp" (0xffffff800895be78)   cmp x2, #0x104           (st21nfc_dev_write, count)
 */
#define MAX_BUFFER_SIZE 260

/*
 * The ioctl magic: st21nfc_dev_ioctl immediately discards anything that does not
 * carry 0xea in the _IOC_TYPE field.
 *   "12181c29 and" (0xffffff800895c060)   and w9, w1, #0xff00
 *   "529d400a mov" (0xffffff800895c064)   mov w10, #0xea00       // #59904
 *   "6b0a013f cmp" (0xffffff800895c068)   cmp w9, w10
 *   "b27bf7e0 orr" (0xffffff800895c110)   orr x0, xzr, #0xffffffffffffffe7   // -25 = -ENOTTY
 */
#define ST21NFC_MAGIC 0xEA

/*
 * _IOR() was reconstructed from the factory kernel disassembly (0xffffff800895c0d0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st21nfc_st21nfc.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define ST21NFC_GET_WAKEUP		_IOR(ST21NFC_MAGIC, 0x01, unsigned int)
#define ST21NFC_PULSE_REQUEST		_IOR(ST21NFC_MAGIC, 0x02, unsigned int)
#define ST21NFC_SET_POLARITY_RISING	_IOR(ST21NFC_MAGIC, 0x03, unsigned int)
#define ST21NFC_SET_POLARITY_FALLING	_IOR(ST21NFC_MAGIC, 0x04, unsigned int)
#define ST21NFC_SET_POLARITY_HIGH	_IOR(ST21NFC_MAGIC, 0x05, unsigned int)
#define ST21NFC_SET_POLARITY_LOW	_IOR(ST21NFC_MAGIC, 0x06, unsigned int)
#define ST21NFC_GET_POLARITY		_IOR(ST21NFC_MAGIC, 0x07, unsigned int)
#define ST21NFC_RECOVERY		_IOR(ST21NFC_MAGIC, 0x08, unsigned int)

#define ST21NFC_LEGACY_GET_WAKEUP		_IO(ST21NFC_MAGIC, 0x01)
#define ST21NFC_LEGACY_PULSE_REQUEST		_IO(ST21NFC_MAGIC, 0x02)
#define ST21NFC_LEGACY_SET_POLARITY_RISING	_IO(ST21NFC_MAGIC, 0x03)
#define ST21NFC_LEGACY_SET_POLARITY_FALLING	_IO(ST21NFC_MAGIC, 0x04)
#define ST21NFC_LEGACY_SET_POLARITY_HIGH	_IO(ST21NFC_MAGIC, 0x05)
#define ST21NFC_LEGACY_SET_POLARITY_LOW		_IO(ST21NFC_MAGIC, 0x06)
#define ST21NFC_LEGACY_GET_POLARITY		_IO(ST21NFC_MAGIC, 0x07)
#define ST21NFC_LEGACY_RECOVERY			_IO(ST21NFC_MAGIC, 0x08)

/*
 * The ninth command falls outside both tables and has a comparison of its own:
 *   "529d4129 mov" (0xffffff800895c160)   mov w9, #0xea09        // #59913
 *   "72a80089 movk" (0xffffff800895c164)  movk w9, #0x4004, lsl #16    -> 0x4004EA09
 *   "6b09011f cmp" (0xffffff800895c168)   cmp w8, w9
 * 0x4004EA09 = _IOW(0xEA, 9, unsigned int).
 */
#define ST21NFC_USE_ESE			_IOW(ST21NFC_MAGIC, 0x09, unsigned int)

/*
 * DRIVER_VERSION: the same string serves both MODULE_VERSION and
 * the 'version' sysfs attribute.
 *   "90004622 adrp" (0xffffff800895c8e4) + "9117d442 add" (0xffffff800895c8ec)  -> 0xffffff80092205f5 '2.2.0.1'
 * and module_version_attribute.version at 0xffffff8009955398 points there.
 */
#define DRIVER_VERSION "2.2.0.1"

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800922018e).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st21nfc_st21nfc.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct st21nfc_platform_data {
	struct mutex read_mutex;		/* +0x00 */
	struct i2c_client *client;		/* +0x20 */
	unsigned int irq_gpio;			/* +0x28 */
	unsigned int reset_gpio;		/* +0x2c */
	unsigned int ena_gpio;			/* +0x30 */
	int polarity_mode;			/* +0x34 */
	unsigned int active_polarity;		/* +0x38 */
	unsigned int reserved_unread;		/* +0x3c -- see above */
	spinlock_t irq_enabled_lock;		/* +0x40 */
};

/*
 * struct st21nfc_dev -- kzalloc(184).
 *   "52801702 mov" (0xffffff800895b74c)   mov w2, #0xb8          // #184
 *   "52901801 mov" (0xffffff800895b744)   mov w1, #0x80c0        // GFP_KERNEL|__GFP_ZERO
 *   "72a02801 movk" (0xffffff800895b748)  movk w1, #0x140, lsl #16     -> 0x014080c0
 *
 *   +0x00 read_wq       __init_waitqueue_head receives x0 = st21nfc_dev
 *                       "aa1603e0 mov" (0xffffff800895ba48) mov x0, x22
 *   +0x18 st21nfc_device (struct miscdevice, 80 bytes): file->private_data gets
 *         it from misc_open, and the driver works back with container_of:
 *         "d1006293 sub" (0xffffff800895c0e0)  sub x19, x20, #0x18
 *   +0x68 irq_enabled   "3941a2c8 ldrb" (0xffffff800895bb04)  ldrb w8, [x22,#104]
 *   +0x70 platform_data
 */
struct st21nfc_dev {
	wait_queue_head_t read_wq;
	struct miscdevice st21nfc_device;
	bool irq_enabled;
	struct st21nfc_platform_data platform_data;
};

/*
 * The structure nfc_parse_dt fills in, and which on the non-DT path comes from
 * client->dev.platform_data. It is a different structure: 16 bytes, four integers.
 *   "321c03e1 orr" (0xffffff800895b654)   orr w1, wzr, #0x10     // 16 = sizeof
 *   "97ee5980 bl" (0xffffff800895b660)    -> ffffff80084f1c60 <devm_kmalloc>
 * Field order, from the offsets used:
 *   +0  irq_gpio       "b9000263 str" (0xffffff800895b81c)  str w3, [x19]
 *   +4  ena_gpio       "b9400660 ldr" (0xffffff800895b990)  ldr w0, [x19,#4]
 *   +8  reset_gpio     "b9000a62 str" (0xffffff800895b6b8)  str w2, [x19,#8]
 *   +12 polarity_mode  "b9000e7f str" (0xffffff800895b858)  str wzr, [x19,#12]
 */
struct st21nfc_i2c_platform_data {
	unsigned int irq_gpio;
	unsigned int ena_gpio;
	unsigned int reset_gpio;
	int polarity_mode;
};

/*
 * The three statics in the file's .bss, with their factory addresses:
 *   0xffffff800a0dc558  st21nfc_st54spi_cb    "f902ad14 str" (0xffffff800895b5d4)  str x20, [x8,#1368]
 *   0xffffff800a0dc560  st21nfc_st54spi_data  "f902b133 str" (0xffffff800895b5d8)  str x19, [x9,#1376]
 *   0xffffff800a0dc568  irq_is_attached       "3915a15f strb" (0xffffff800895b91c) strb wzr, [x10,#1384]
 *   0xffffff800a0dc569  device_open           "3915a57f strb" (0xffffff800895b920) strb wzr, [x11,#1385]
 * The two bools are read/written with LDRB/STRB: they are 8-bit, not 32.
 */
static void (*st21nfc_st54spi_cb)(int, void *);
static void *st21nfc_st54spi_data;
static bool irq_is_attached;
static bool device_open;

static irqreturn_t st21nfc_dev_irq_handler(int irq, void *dev_id);
static const struct file_operations st21nfc_dev_fops;
static struct attribute_group st21nfc_attr_grp;

/*
 * st21nfc_register_st54spi_cb() was reconstructed from the factory kernel disassembly (0xffffff800895b5a0, 68 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st21nfc_st21nfc.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void st21nfc_register_st54spi_cb(void (*cb)(int, void *), void *data)
{
	printk(KERN_INFO "%s\n", __func__);
	st21nfc_st54spi_cb = cb;
	st21nfc_st54spi_data = data;
}

/*
 * st21nfc_unregister_st54spi_cb -- 0xffffff800895b5e4, 52 byte, symbol T.
 * "9138ec21 add" (0xffffff800895b5f8)   add x1, x1, #0xe3b   -> 0xffffff800921fe3b
 *                                          'st21nfc_unregister_st54spi_cb'
 * "f902ad1f str" (0xffffff800895b608)   str xzr, [x8,#1368]
 * "f902b13f str" (0xffffff800895b60c)   str xzr, [x9,#1376]
 */
void st21nfc_unregister_st54spi_cb(void)
{
	printk(KERN_INFO "%s\n", __func__);
	st21nfc_st54spi_cb = NULL;
	st21nfc_st54spi_data = NULL;
}

/*
 * st21nfc_platform_probe -- 0xffffff800895b618, 8 byte.
 * "2a1f03e0 mov" (0xffffff800895b618)   mov w0, wzr
 * "d65f03c0 ret" (0xffffff800895b61c)   ret
 */
static int st21nfc_platform_probe(struct platform_device *pdev)
{
	return 0;
}

/*
 * st21nfc_platform_remove -- 0xffffff800895b620, 8 byte.
 * "2a1f03e0 mov" (0xffffff800895b620)   mov w0, wzr
 * "d65f03c0 ret" (0xffffff800895b624)   ret
 */
static int st21nfc_platform_remove(struct platform_device *pdev)
{
	return 0;
}

/*
 * st21nfc_loc_set_polaritymode() was reconstructed from the factory kernel disassembly (0xffffff800895c2bc).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st21nfc_st21nfc.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int st21nfc_loc_set_polaritymode(struct st21nfc_dev *st21nfc_dev,
					int mode)
{
	struct i2c_client *client = st21nfc_dev->platform_data.client;
	int ret;

	if (irq_is_attached) {
		free_irq(client->irq, st21nfc_dev);
		irq_is_attached = false;
	}

	ret = irq_set_irq_type(client->irq, mode);
	if (ret) {
		/*
		 * "91166c00 add" (0xffffff800895c378)  add x0, x0, #0x59b -> 0xffffff800922059b
		 *                 '\x013%s : set_irq_type failed!!!!!!!\n'
		 * "913a5c21 add" (0xffffff800895c37c)  add x1, x1, #0xe97 -> __FILE__ (not __func__)
		 */
		printk(KERN_ERR "%s : set_irq_type failed!!!!!!!\n", __FILE__);
		return -ENODEV;
	}

	/* "39014297 strb" (0xffffff800895c2f0)  strb w23, [x20,#80]   con w23 = 1 */
	st21nfc_dev->irq_enabled = true;

	ret = request_irq(client->irq, st21nfc_dev_irq_handler,
			  st21nfc_dev->platform_data.polarity_mode,
			  client->name, st21nfc_dev);
	if (ret)
		return -ENODEV;

	irq_is_attached = true;
	return ret;
}

/*
 * st21nfc_probe -- 0xffffff800895b628, 1332 byte.
 */
static int st21nfc_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int ret;
	struct st21nfc_i2c_platform_data *platform_data;
	struct st21nfc_dev *st21nfc_dev;
	struct device_node *node;

	/*
	 * "f9413408 ldr" (0xffffff800895b640)   ldr x8, [x0,#616]   client->dev.of_node
	 *   (i2c_client.dev sits at +32, struct device.of_node at +584)
	 * "b40003a8 cbz" (0xffffff800895b64c)   cbz x8, ffffff800895b6c0
	 */
	if (client->dev.of_node) {
		/*
		 * "91008015 add" (0xffffff800895b648)   add x21, x0, #0x20      &client->dev
		 * "321c03e1 orr" (0xffffff800895b654)   orr w1, wzr, #0x10      16 byte
		 * "52901802 mov" (0xffffff800895b650)   mov w2, #0x80c0
		 * "72a02802 movk" (0xffffff800895b658)  movk w2, #0x140, lsl #16   -> 0x014080c0
		 *                  = GFP_KERNEL | __GFP_ZERO  (devm_kzalloc)
		 * "97ee5980 bl" (0xffffff800895b660)    -> ffffff80084f1c60 <devm_kmalloc>
		 * "12800177 mov" (0xffffff800895b718)   mov w23, #0xfffffff4    // -12 = -ENOMEM
		 */
		platform_data = devm_kzalloc(&client->dev,
					     sizeof(struct st21nfc_i2c_platform_data),
					     GFP_KERNEL);
		if (!platform_data)
			return -ENOMEM;

		/*
		 * "913be000 add" (0xffffff800895b674)  add x0, x0, #0xef8 -> 0xffffff800921fef8
		 *                 '\x016%s : Parse st21nfc DTS\n'
		 * "913c4821 add" (0xffffff800895b678)  add x1, x1, #0xf12 -> 'st21nfc_probe'
		 */
		printk(KERN_INFO "%s : Parse st21nfc DTS\n", __func__);

		/*
		 * nfc_parse_dt: it has no symbol of its own (inlined), but the
		 * binary names it all the same -- it is the __func__ of the printk at
		 * 0x895b844:
		 *   "9109a821 add" (0xffffff800895b850)  add x1, x1, #0x26a -> 0xffffff800922026a
		 *                                         'nfc_parse_dt'
		 * The code below is what replaces it, line by line;
		 * the message keeps its real __func__.
		 */
		/*
		 * "91083c42 add" (0xffffff800895b684)  add x2, x2, #0x20f -> 0xffffff800922020f
		 *                 'mediatek,nfc-gpio-v2'
		 * "94082741 bl" (0xffffff800895b690)   -> ffffff8008b65394 <of_find_compatible_node>
		 */
		node = of_find_compatible_node(NULL, NULL,
					       "mediatek,nfc-gpio-v2");
		if (!node) {
			/*
			 * "913c8000 add" (0xffffff800895b7d0)  add x0, x0, #0xf20
			 *                 -> '\x013%s : ret =%d\n'
			 * "12800002 mov" (0xffffff800895b7d8)  mov w2, #0xffffffff    // -1
			 */
			printk(KERN_ERR "%s : ret =%d\n", __func__, -1);
			return -1;
		}

		/*
		 * "91015821 add" (0xffffff800895b69c)  add x1, x1, #0x56  -> 'gpio-rst-std'
		 * "97ec7461 bl" (0xffffff800895b6ac)   -> ffffff8008478830 <of_get_named_gpio_flags>
		 * "37f809a0 tbnz" (0xffffff800895b6b4) tbnz w0, #31
		 */
		ret = of_get_named_gpio_flags(node, "gpio-rst-std", 0, NULL);
		if (ret < 0)
			/*
			 * "91089000 add" (0xffffff800895b7f0)  add x0, x0, #0x224
			 *          -> '\x013%s: get NFC RST GPIO failed (%d)'
			 * "913a5c21 add" (0xffffff800895b7f4)  add x1, x1, #0xe97 -> __FILE__
			 */
			printk(KERN_ERR "%s: get NFC RST GPIO failed (%d)", __FILE__,
			       ret);
		else
			/* "b9000a62 str" (0xffffff800895b6b8)  str w2, [x19,#8] */
			platform_data->reset_gpio = ret;

		/*
		 * "913fcc21 add" (0xffffff800895b800)  add x1, x1, #0xff3 -> 'gpio-irq-std'
		 * "97ec7408 bl" (0xffffff800895b810)   -> <of_get_named_gpio_flags>
		 */
		ret = of_get_named_gpio_flags(node, "gpio-irq-std", 0, NULL);
		if (ret < 0)
			/*
			 * "91091c00 add" (0xffffff800895b82c)  add x0, x0, #0x247
			 *          -> '\x013%s: get NFC IRQ GPIO failed (%d)'
			 * "913a5c21 add" (0xffffff800895b830)  add x1, x1, #0xe97 -> __FILE__
			 */
			printk(KERN_ERR "%s: get NFC IRQ GPIO failed (%d)", __FILE__,
			       ret);
		else
			/* "b9000263 str" (0xffffff800895b81c)  str w3, [x19] */
			platform_data->irq_gpio = ret;

		/* "b9000e7f str" (0xffffff800895b858)  str wzr, [x19,#12] */
		platform_data->polarity_mode = 0;

		/*
		 * "9109dc00 add" (0xffffff800895b84c)  add x0, x0, #0x277 -> 0xffffff8009220277
		 *   '\x016[dsc]%s : get reset_gpio[%d], irq_gpio[%d],
		 *    polarity_mode[%d]\n'
		 * "9109a821 add" (0xffffff800895b850)  add x1, x1, #0x26a -> 'nfc_parse_dt'
		 * "b9400a62 ldr" (0xffffff800895b840)  ldr w2, [x19,#8]   reset_gpio
		 * "b9400263 ldr" (0xffffff800895b864)  ldr w3, [x19]      irq_gpio
		 * "2a1f03e4 mov" (0xffffff800895b854)  mov w4, wzr        polarity_mode
		 */
		printk(KERN_INFO "[dsc]%s : get reset_gpio[%d], irq_gpio[%d], polarity_mode[%d]\n",
			"nfc_parse_dt", platform_data->reset_gpio,
			platform_data->irq_gpio, platform_data->polarity_mode);

		/*
		 * "913cc000 add" (0xffffff800895b870)  add x0, x0, #0xf30
		 *                 -> '\x016%s : Parsed st21nfc DTS %d %d\n'
		 * "b9400a62 ldr" (0xffffff800895b860)  ldr w2, [x19,#8]
		 * "b9400263 ldr" (0xffffff800895b864)  ldr w3, [x19]
		 */
		printk(KERN_INFO "%s : Parsed st21nfc DTS %d %d\n", __func__,
			platform_data->reset_gpio, platform_data->irq_gpio);
	} else {
		/*
		 * "f9405a93 ldr" (0xffffff800895b6c0)  ldr x19, [x20,#176]  client->dev.platform_data
		 * "913d4400 add" (0xffffff800895b6cc)  add x0, x0, #0xf51
		 *                 -> '\x013%s : No st21nfc DTS\n'
		 */
		platform_data = client->dev.platform_data;
		printk(KERN_ERR "%s : No st21nfc DTS\n", __func__);
	}

	/*
	 * "b5fff313 cbnz" (0xffffff800895b87c)  cbnz x19, ffffff800895b6dc
	 * "128002b7 mov" (0xffffff800895b880)   mov w23, #0xffffffea    // -22 = -EINVAL
	 */
	if (!platform_data)
		return -EINVAL;

	/*
	 * The pr_debug that .ddebug attributes to st21nfc.c line 826.
	 * "91158000 add" (0xffffff800895bb44)  add x0, x0, #0x560 -> 0xffffff8009a33560, the
	 *                 _ddebug descriptor
	 * "913da042 add" (0xffffff800895bb48)  add x2, x2, #0xf68 -> 0xffffff800921ff68
	 *                 'nfc-nci probe: %s, inside nfc-nci flags = %x\n'
	 * "913c4863 add" (0xffffff800895bb4c)  add x3, x3, #0xf12 -> 'st21nfc_probe'
	 * "79400284 ldrh" (0xffffff800895bb34) ldrh w4, [x20]      client->flags
	 * "97ebfefa bl" (0xffffff800895bb54)   -> ffffff800845b73c <__dynamic_dev_dbg>
	 */
	dev_dbg(&client->dev, "nfc-nci probe: %s, inside nfc-nci flags = %x\n",
		__func__, client->flags);

	/*
	 * "f9400e80 ldr" (0xffffff800895b6e8)  ldr x0, [x20,#24]   client->adapter
	 * "f9400808 ldr" (0xffffff800895b6ec)  ldr x8, [x0,#16]    adapter->algo
	 * "f9400908 ldr" (0xffffff800895b6f0)  ldr x8, [x8,#16]    algo->functionality
	 * "37000140 tbnz" (0xffffff800895b6f8) tbnz w0, #0         I2C_FUNC_I2C = 0x00000001
	 * "12800257 mov" (0xffffff800895b710)  mov w23, #0xffffffed    // -19 = -ENODEV
	 */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		/*
		 * "913e5800 add" (0xffffff800895b704)  add x0, x0, #0xf96
		 *                 -> '\x013%s : need I2C_FUNC_I2C\n'
		 */
		printk(KERN_ERR "%s : need I2C_FUNC_I2C\n", __func__);
		return -ENODEV;
	}

	/*
	 * "913ec000 add" (0xffffff800895b730)  add x0, x0, #0xfb0 -> 0xffffff800921ffb0
	 *   '\x016%s : after i2c_check_functionality %d %d\n'
	 * "b9400a62 ldr" (0xffffff800895b720)  ldr w2, [x19,#8]   reset_gpio
	 * "b9400263 ldr" (0xffffff800895b724)  ldr w3, [x19]      irq_gpio
	 */
	printk(KERN_INFO "%s : after i2c_check_functionality %d %d\n", __func__,
		platform_data->reset_gpio, platform_data->irq_gpio);

	/*
	 * "52801702 mov" (0xffffff800895b74c)  mov w2, #0xb8      // 184 = sizeof(struct st21nfc_dev)
	 * "97e3cd57 bl" (0xffffff800895b750)   -> ffffff800824ecac <kmem_cache_alloc_trace>
	 * "12800177 mov" (0xffffff800895b888)  mov w23, #0xfffffff4    // -12 = -ENOMEM
	 */
	st21nfc_dev = kzalloc(sizeof(*st21nfc_dev), GFP_KERNEL);
	if (st21nfc_dev == NULL) {
		ret = -ENOMEM;
		goto err_exit;
	}

	/*
	 * "913f7000 add" (0xffffff800895b764)  add x0, x0, #0xfdc
	 *                 -> '\x016%s : dev_cb_addr %p\n'
	 */
	printk(KERN_INFO "%s : dev_cb_addr %p\n", __func__, st21nfc_dev);

	/*
	 * "b9400268 ldr" (0xffffff800895b774)  ldr w8, [x19]      + "b9009ac8 str" (0xffffff800895b780)  str w8, [x22,#152]
	 * "b9400668 ldr" (0xffffff800895b784)  ldr w8, [x19,#4]   + "b900a2c8 str" (0xffffff800895b788)  str w8, [x22,#160]
	 * "b9400a68 ldr" (0xffffff800895b78c)  ldr w8, [x19,#8]   + "b9009ec8 str" (0xffffff800895b790)  str w8, [x22,#156]
	 * "f9004ad4 str" (0xffffff800895b798)  str x20, [x22,#144]
	 * "b9400e68 ldr" (0xffffff800895b794)  ldr w8, [x19,#12]  + "b900a6c8 str" (0xffffff800895b79c)  str w8, [x22,#164]
	 */
	st21nfc_dev->platform_data.irq_gpio = platform_data->irq_gpio;
	st21nfc_dev->platform_data.ena_gpio = platform_data->ena_gpio;
	st21nfc_dev->platform_data.reset_gpio = platform_data->reset_gpio;
	st21nfc_dev->platform_data.client = client;
	st21nfc_dev->platform_data.polarity_mode = platform_data->polarity_mode;

	/*
	 * "90004621 adrp" (0xffffff800895b778) + "913fcc21 add" (0xffffff800895b77c)  add x1, x1, #0xff3 -> 'gpio-irq-std'
	 * "b9400260 ldr" (0xffffff800895b7a0)  ldr w0, [x19]
	 * "97ec7348 bl" (0xffffff800895b7a4)   -> ffffff80084784c4 <gpio_request>
	 */
	ret = gpio_request(platform_data->irq_gpio, "gpio-irq-std");
	if (ret) {
		/*
		 * "91000000 add" (0xffffff800895b7b4)  add x0, x0, #0x0 -> 0xffffff8009220000
		 *                 '\x013%s : gpio_request failed\n'
		 * "913a5c21 add" (0xffffff800895b7bc)  add x1, x1, #0xe97 -> __FILE__
		 */
		printk(KERN_ERR "%s : gpio_request failed\n", __FILE__);
		goto err_free_dev;
	}

	/*
	 * "91007000 add" (0xffffff800895b89c)  add x0, x0, #0x1c -> '\x016%s : IRQ GPIO = %d\n'
	 */
	printk(KERN_INFO "%s : IRQ GPIO = %d\n", __func__, platform_data->irq_gpio);

	/*
	 * "97ec5d60 bl" (0xffffff800895b8ac)   -> ffffff8008472e2c <gpio_to_desc>
	 * "97ec627b bl" (0xffffff800895b8b0)   -> ffffff800847429c <gpiod_direction_input>
	 */
	ret = gpio_direction_input(platform_data->irq_gpio);
	if (ret) {
		/*
		 * "9100c800 add" (0xffffff800895b8bc)  add x0, x0, #0x32
		 *                 -> '\x013%s : gpio_direction_input failed\n'
		 * "913a5c21 add" (0xffffff800895b8c4)  add x1, x1, #0xe97 -> __FILE__
		 * "12800257 mov" (0xffffff800895b8cc)  mov w23, #0xffffffed    // -19 = -ENODEV
		 */
		printk(KERN_ERR "%s : gpio_direction_input failed\n", __FILE__);
		ret = -ENODEV;
		goto err_free_dev;
	}

	/*
	 * "b9400268 ldr" (0xffffff800895b908)  ldr w8, [x19]
	 * "f9404ac9 ldr" (0xffffff800895b90c)  ldr x9, [x22,#144]
	 * "b902e928 str" (0xffffff800895b918)  str w8, [x9,#744]     client->irq
	 */
	st21nfc_dev->platform_data.client->irq = platform_data->irq_gpio;

	/*
	 * "3915a15f strb" (0xffffff800895b91c) strb wzr, [x10,#1384]
	 * "3915a57f strb" (0xffffff800895b920) strb wzr, [x11,#1385]
	 */
	irq_is_attached = false;
	device_open = false;

	/* "b9400a60 ldr" (0xffffff800895b924)  ldr w0, [x19,#8] + "34000340 cbz" (0xffffff800895b928) */
	if (platform_data->reset_gpio != 0) {
		/*
		 * "91015821 add" (0xffffff800895b930)  add x1, x1, #0x56 -> 'gpio-rst-std'
		 * "97ec72e4 bl" (0xffffff800895b934)   -> <gpio_request>
		 */
		ret = gpio_request(platform_data->reset_gpio, "gpio-rst-std");
		if (ret) {
			/*
			 * "91018c00 add" (0xffffff800895b940)  add x0, x0, #0x63
			 *          -> '\x013%s : reset gpio_request failed\n'
			 * "17ffffdf b" (0xffffff800895b944)    -> 0x895b8c0, that is the branch with
			 *          x1 = __FILE__ and w23 = -ENODEV
			 */
			printk(KERN_ERR "%s : reset gpio_request failed\n", __FILE__);
			ret = -ENODEV;
			goto err_free_dev;
		}

		/*
		 * "91021400 add" (0xffffff800895b954)  add x0, x0, #0x85
		 *                 -> '\x016%s : RST GPIO = %d\n'
		 */
		printk(KERN_INFO "%s : RST GPIO = %d\n", __func__,
			platform_data->reset_gpio);

		/*
		 * "97ec62bf bl" (0xffffff800895b96c)  -> ffffff8008474468 <gpiod_direction_output_raw>
		 *                   with w1 = 1
		 */
		ret = gpio_direction_output(platform_data->reset_gpio, 1);
		if (ret) {
			/*
			 * "91026c00 add" (0xffffff800895b978)  add x0, x0, #0x9b
			 *   -> '\x013%s : reset gpio_direction_output failed\n'
			 */
			printk(KERN_ERR "%s : reset gpio_direction_output failed\n",
			       __FILE__);
			ret = -ENODEV;
			goto err_free_dev;
		}

		/*
		 * "b9409ec0 ldr" (0xffffff800895b980)  ldr w0, [x22,#156]   -- st21nfc_dev, not
		 *                 platform_data: that is exactly how it is in the binary
		 * "97ec66a3 bl" (0xffffff800895b98c)   -> ffffff8008475418 <gpiod_set_raw_value> (1)
		 */
		gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 1);
	}

	/* "b9400660 ldr" (0xffffff800895b990)  ldr w0, [x19,#4] + "34000200 cbz" (0xffffff800895b994) */
	if (platform_data->ena_gpio != 0) {
		/*
		 * "91031821 add" (0xffffff800895b99c)  add x1, x1, #0xc6 -> 'st21nfc_ena'
		 */
		ret = gpio_request(platform_data->ena_gpio, "st21nfc_ena");
		if (ret) {
			/*
			 * "91034800 add" (0xffffff800895b9ac)  add x0, x0, #0xd2
			 *          -> '\x013%s : ena gpio_request failed\n'
			 */
			printk(KERN_ERR "%s : ena gpio_request failed\n", __FILE__);
			ret = -ENODEV;
			goto err_free_dev;
		}

		ret = gpio_direction_output(platform_data->ena_gpio, 1);
		if (ret) {
			/*
			 * "9103c800 add" (0xffffff800895b9cc)  add x0, x0, #0xf2
			 *   -> '\x013%s : ena gpio_direction_output failed\n'
			 */
			printk(KERN_ERR "%s : ena gpio_direction_output failed\n",
			       __FILE__);
			ret = -ENODEV;
			goto err_free_dev;
		}
	}

	/*
	 * "91046c42 add" (0xffffff800895b9d8)  add x2, x2, #0x11b -> 'mediatek,irq_nfc-eint'
	 * "9408266b bl" (0xffffff800895b9e8)   -> <of_find_compatible_node>
	 */
	node = of_find_compatible_node(NULL, NULL, "mediatek,irq_nfc-eint");
	if (node) {
		/*
		 * "94083ea1 bl" (0xffffff800895b9f4)  -> ffffff8008b6b478 <irq_of_parse_and_map>
		 *                   with w1 = 0
		 * "b902ea80 str" (0xffffff800895b9fc) str w0, [x20,#744]
		 * "9104c400 add" (0xffffff800895ba08) add x0, x0, #0x131
		 *                -> '\x016%s : MT IRQ GPIO = %d\n'
		 * "320003e1 orr" (0xffffff800895ba18) orr w1, wzr, #0x1
		 * "97df7d22 bl" (0xffffff800895ba1c)  -> ffffff800813aea4 <irq_set_irq_wake>
		 */
		client->irq = irq_of_parse_and_map(node, 0);
		printk(KERN_INFO "%s : MT IRQ GPIO = %d\n", __func__, client->irq);
		irq_set_irq_wake(client->irq, 1);
	} else {
		/*
		 * "91052800 add" (0xffffff800895ba2c)  add x0, x0, #0x14a
		 *   -> '\x013%s : can not find NFC eint compatible node\n'
		 */
		printk(KERN_ERR "%s : can not find NFC eint compatible node\n",
		       __func__);
	}

	/*
	 * "9105e021 add" (0xffffff800895ba40)  add x1, x1, #0x178 -> '&st21nfc_dev->read_wq'
	 * "97df060b bl" (0xffffff800895ba4c)   -> ffffff800811d278 <__init_waitqueue_head>
	 */
	init_waitqueue_head(&st21nfc_dev->read_wq);

	/*
	 * "91063821 add" (0xffffff800895ba58)  add x1, x1, #0x18e
	 *                 -> '&st21nfc_dev->platform_data.read_mutex'
	 * "aa1703e0 mov" (0xffffff800895ba60)  mov x0, x23   (= st21nfc_dev + 0x70)
	 * "97df42a6 bl" (0xffffff800895ba64)   -> ffffff800812c4fc <__mutex_init>
	 */
	mutex_init(&st21nfc_dev->platform_data.read_mutex);

	/* "b80b0f1f str" (0xffffff800895ba7c)  str wzr, [x24,#176]!   (4 byte at +0xb0) */
	spin_lock_init(&st21nfc_dev->platform_data.irq_enabled_lock);

	/*
	 * "32001fe8 orr" (0xffffff800895ba6c)  orr w8, wzr, #0xff    MISC_DYNAMIC_MINOR = 255
	 * "b8018f28 str" (0xffffff800895ba80)  str w8, [x25,#24]!
	 * "913a3d29 add" (0xffffff800895ba84)  add x9, x9, #0xe8f    -> 'st21nfc'
	 * "9124614a add" (0xffffff800895ba88)  add x10, x10, #0x918  -> 0xffffff8008f70918
	 *                                          = &st21nfc_dev_fops
	 * "a9022ac9 stp" (0xffffff800895ba90)  stp x9, x10, [x22,#32]
	 * "f90022d5 str" (0xffffff800895ba94)  str x21, [x22,#64]    parent = &client->dev
	 */
	st21nfc_dev->st21nfc_device.minor = MISC_DYNAMIC_MINOR;
	st21nfc_dev->st21nfc_device.name = "st21nfc";
	st21nfc_dev->st21nfc_device.fops = &st21nfc_dev_fops;
	st21nfc_dev->st21nfc_device.parent = &client->dev;

	/* "f9005e96 str" (0xffffff800895ba98)  str x22, [x20,#184] */
	i2c_set_clientdata(client, st21nfc_dev);

	/* "97edff23 bl" (0xffffff800895ba9c)  -> ffffff80084db728 <misc_register> */
	ret = misc_register(&st21nfc_dev->st21nfc_device);
	if (ret) {
		/*
		 * "9106d400 add" (0xffffff800895baac)  add x0, x0, #0x1b5
		 *                 -> '\x016ret of misc_register:%d\n'
		 *                 (without __func__: the only one in this file)
		 * "91074000 add" (0xffffff800895babc)  add x0, x0, #0x1d0
		 *                 -> '\x013%s : misc_register failed\n'
		 * "17ffff3e b" (0xffffff800895bac0)    -> 0x895b7b8, with x1 = __FILE__
		 */
		printk(KERN_INFO "ret of misc_register:%d\n", ret);
		printk(KERN_ERR "%s : misc_register failed\n", __FILE__);
		goto err_free_dev;
	}

	/*
	 * "9100c280 add" (0xffffff800895bac8)  add x0, x20, #0x30   &client->dev.kobj
	 * "91114021 add" (0xffffff800895bacc)  add x1, x1, #0x450   -> 0xffffff8009955450
	 *                                         = &st21nfc_attr_grp
	 * "97e6a008 bl" (0xffffff800895bad0)   -> ffffff8008303af0 <sysfs_create_group>
	 */
	ret = sysfs_create_group(&client->dev.kobj, &st21nfc_attr_grp);
	if (ret) {
		/*
		 * "9107b400 add" (0xffffff800895bae0)  add x0, x0, #0x1ed
		 *          -> '\x013%s : sysfs_create_group failed\n'
		 * "913a5c21 add" (0xffffff800895bae4)  add x1, x1, #0xe97 -> __FILE__
		 * "97edff66 bl" (0xffffff800895baf0)   -> ffffff80084db888 <misc_deregister>
		 * "2a1f03f7 mov" (0xffffff800895baf4)  mov w23, wzr   -- ret zeroed, not ret=err
		 */
		printk(KERN_ERR "%s : sysfs_create_group failed\n", __FILE__);
		misc_deregister(&st21nfc_dev->st21nfc_device);
		ret = 0;
		goto err_free_dev;
	}

	/*
	 * "aa1803e0 mov" (0xffffff800895bafc)  mov x0, x24   (= st21nfc_dev + 0xb0)
	 * "94146c62 bl" (0xffffff800895bb00)   -> ffffff8008e76c88 <_raw_spin_lock_irqsave>
	 * "3941a2c8 ldrb" (0xffffff800895bb04) ldrb w8, [x22,#104]
	 * "97df7c09 bl" (0xffffff800895bb18)   -> ffffff800813ab3c <disable_irq_nosync>
	 * "3901a2df strb" (0xffffff800895bb1c) strb wzr, [x22,#104]
	 * "94146ccf bl" (0xffffff800895bb28)   -> ffffff8008e76e64 <_raw_spin_unlock_irqrestore>
	 * "2a1f03f7 mov" (0xffffff800895bb2c)  mov w23, wzr
	 */
	{
		unsigned long flags;

		spin_lock_irqsave(&st21nfc_dev->platform_data.irq_enabled_lock,
				  flags);
		if (st21nfc_dev->irq_enabled) {
			disable_irq_nosync(st21nfc_dev->platform_data.client->irq);
			st21nfc_dev->irq_enabled = false;
		}
		spin_unlock_irqrestore(&st21nfc_dev->platform_data.irq_enabled_lock,
				       flags);
	}

	return 0;

err_free_dev:
	/* "97e3d3b7 bl" (0xffffff800895b8d4)  -> ffffff80082507b0 <kfree> */
	kfree(st21nfc_dev);
err_exit:
	/*
	 * "b9400260 ldr" (0xffffff800895b8d8)  ldr w0, [x19]     + "97ec72bc bl" (0xffffff800895b8dc) <gpio_free>
	 * "b9400660 ldr" (0xffffff800895b8e0)  ldr w0, [x19,#4]  + "34000040 cbz" (0xffffff800895b8e4) + "97ec72b9 bl" (0xffffff800895b8e8)
	 */
	gpio_free(platform_data->irq_gpio);
	if (platform_data->ena_gpio != 0)
		gpio_free(platform_data->ena_gpio);
	return ret;
}

/*
 * st21nfc_remove -- 0xffffff800895bb5c, 80 byte.
 *
 * "f9405c13 ldr" (0xffffff800895bb68)  ldr x19, [x0,#184]     i2c_get_clientdata
 * "b942e800 ldr" (0xffffff800895bb6c)  ldr w0, [x0,#744]      client->irq
 * "97df80c7 bl" (0xffffff800895bb74)   -> ffffff800813be90 <free_irq>
 * "91006260 add" (0xffffff800895bb78)  add x0, x19, #0x18     &st21nfc_dev->st21nfc_device
 * "97edff43 bl" (0xffffff800895bb7c)   -> ffffff80084db888 <misc_deregister>
 * "b9409a60 ldr" (0xffffff800895bb80)  ldr w0, [x19,#152]  + "97ec7212 bl" (0xffffff800895bb84) <gpio_free>
 * "b940a260 ldr" (0xffffff800895bb88)  ldr w0, [x19,#160]  + "34000040 cbz" (0xffffff800895bb8c) + "97ec720f bl" (0xffffff800895bb90)
 * "97e3d306 bl" (0xffffff800895bb98)   -> ffffff80082507b0 <kfree>
 * "2a1f03e0 mov" (0xffffff800895bba0)  mov w0, wzr
 */
static int st21nfc_remove(struct i2c_client *client)
{
	struct st21nfc_dev *st21nfc_dev = i2c_get_clientdata(client);

	free_irq(client->irq, st21nfc_dev);
	misc_deregister(&st21nfc_dev->st21nfc_device);
	gpio_free(st21nfc_dev->platform_data.irq_gpio);
	if (st21nfc_dev->platform_data.ena_gpio != 0)
		gpio_free(st21nfc_dev->platform_data.ena_gpio);
	kfree(st21nfc_dev);

	return 0;
}

/*
 * st21nfc_dev_read -- 0xffffff800895bbac, 680 byte.
 */
static ssize_t st21nfc_dev_read(struct file *filp, char __user *buf,
				size_t count, loff_t *offset)
{
	struct st21nfc_dev *st21nfc_dev = container_of(filp->private_data,
						       struct st21nfc_dev,
						       st21nfc_device);
	char buffer[MAX_BUFFER_SIZE];
	struct timeval start_tv, lock_tv, end_tv;
	long usec;
	int value;
	ssize_t ret;

	/*
	 * "f104105f cmp" (0xffffff800895bbd0)   cmp x2, #0x104
	 * "9a883057 csel" (0xffffff800895bbe4)  csel x23, x2, x8, cc
	 */
	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	/* "aa1703f4 mov" (0xffffff800895bbe8)  mov x20, x23 -- the value returned is count up to here */
	ret = count;

	/*
	 * "b94082a0 ldr" (0xffffff800895bbec)   ldr w0, [x21,#128]   irq_gpio
	 * "97ec5c8f bl" (0xffffff800895bbf0)    -> <gpio_to_desc>
	 * "97ec63a7 bl" (0xffffff800895bbf4)    -> ffffff8008474a90 <gpiod_get_raw_value>
	 * "7100041f cmp" (0xffffff800895bbf8)   cmp w0, #0x1
	 * "b94092a8 ldr" (0xffffff800895bc00)   ldr w8, [x21,#144]   active_polarity
	 * "35000400 cbnz" (0xffffff800895bc0c)  cbnz w0, ...
	 * "7100051f cmp" (0xffffff800895bc14)   cmp w8, #0x1
	 */
	value = gpio_get_value(st21nfc_dev->platform_data.irq_gpio);
	if ((value > 0 && st21nfc_dev->platform_data.active_polarity == 0) ||
	    (value == 0 && st21nfc_dev->platform_data.active_polarity == 1)) {
		/*
		 * "910b2400 add" (0xffffff800895bc24)  add x0, x0, #0x2c9
		 *                 -> '\x016%s : read called but no IRQ.\n'
		 * "321f17e1 orr" (0xffffff800895bc34)  orr w1, wzr, #0x7e     // 0x7E
		 * "9413f0f1 bl" (0xffffff800895bc3c)   -> ffffff8008e58000 <__memset>
		 */
		printk(KERN_INFO "%s : read called but no IRQ.\n", __func__);
		memset(buffer, 0x7E, count);
		/*
		 * This section was reconstructed from the factory kernel disassembly (0xffffff800895bc40).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st21nfc_st21nfc.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		if (copy_to_user(buf, buffer, count)) {
			printk(KERN_WARNING "%s : failed to copy to user space\n", __func__);
			return -EFAULT;
		}
		return count;
	}

	/*
	 * "910083e0 add" (0xffffff800895bc90)  add x0, sp, #0x20
	 * "97dfe837 bl" (0xffffff800895bc9c)   -> ffffff8008155d78 <do_gettimeofday>
	 */
	do_gettimeofday(&start_tv);

	/*
	 * "910162b6 add" (0xffffff800895bc94)  add x22, x21, #0x58   &platform_data.read_mutex
	 * "94145db6 bl" (0xffffff800895bca4)   -> ffffff8008e7337c <mutex_lock>
	 */
	mutex_lock(&st21nfc_dev->platform_data.read_mutex);

	do_gettimeofday(&lock_tv);

	/*
	 * "f9403ea0 ldr" (0xffffff800895bcb0)  ldr x0, [x21,#120]   client
	 * "9404bd54 bl" (0xffffff800895bcbc)   -> ffffff8008a8b20c <i2c_master_recv>
	 */
	ret = i2c_master_recv(st21nfc_dev->platform_data.client, buffer, count);

	mutex_unlock(&st21nfc_dev->platform_data.read_mutex);

	do_gettimeofday(&end_tv);

	/*
	 * "52884818 mov" (0xffffff800895bc8c)  mov w24, #0x4240
	 * "72a001f8 movk" (0xffffff800895bc98) movk w24, #0xf, lsl #16   -> 1000000
	 * "9b182122 madd" (0xffffff800895bce4) madd x2, x9, x24, x8
	 * "5284e208 mov" (0xffffff800895bce8)  mov w8, #0x2710           // 10000
	 * "eb08005f cmp" (0xffffff800895bcec)  cmp x2, x8
	 */
	usec = (end_tv.tv_sec - start_tv.tv_sec) * 1000000 +
		(end_tv.tv_usec - start_tv.tv_usec);
	if (usec >= 10000) {
		/*
		 * "910c3800 add" (0xffffff800895bd00)  add x0, x0, #0x30e
		 *          -> '\x013%s: took over 10ms (%ld usec)\n'
		 */
		printk(KERN_ERR "%s: took over 10ms (%ld usec)\n", __func__, usec);
		usec = (end_tv.tv_sec - lock_tv.tv_sec) * 1000000 +
			(end_tv.tv_usec - lock_tv.tv_usec);
		/*
		 * "910cbc00 add" (0xffffff800895bd18)  add x0, x0, #0x32f
		 *          -> '\x013%s: %ld usec spent in i2c_master_recv\n'
		 */
		printk(KERN_ERR "%s: %ld usec spent in i2c_master_recv\n", __func__,
		       usec);
	}

	/* "37f80195 tbnz" (0xffffff800895bd30)  tbnz w21, #31 */
	if (ret < 0) {
		/*
		 * "910d6000 add" (0xffffff800895bd68)  add x0, x0, #0x358
		 *          -> '\x013%s: i2c_master_recv returned %d\n'
		 */
		printk(KERN_ERR "%s: i2c_master_recv returned %d\n", __func__, (int)ret);
		return ret;
	}

	/* "eb1402ff cmp" (0xffffff800895bd38)  cmp x23, x20 + "54000222 b.cs" (0xffffff800895bd3c) */
	if (ret > count) {
		/*
		 * "910dec00 add" (0xffffff800895bd48)  add x0, x0, #0x37b
		 *   -> '\x013%s: received too many bytes from i2c (%d)\n'
		 * "b27dfbf4 orr" (0xffffff800895bd58)  orr x20, xzr, #0xfffffffffffffffb  // -5
		 */
		printk(KERN_ERR "%s: received too many bytes from i2c (%d)\n", __func__,
		       (int)ret);
		return -EIO;
	}

	/* "9413ee4a bl" (0xffffff800895bdd8)  -> ffffff8008e57700 <__arch_copy_to_user> */
	if (copy_to_user(buf, buffer, ret)) {
		/*
		 * "910ba400 add" (0xffffff800895bde8)  add x0, x0, #0x2e9
		 *   -> '\x014%s : failed to copy to user space\n'  (KERN_WARNING)
		 * "928001b4 mov" (0xffffff800895bdf4)  mov x20, #0xfffffffffffffff2   // -14
		 */
		printk(KERN_WARNING "%s : failed to copy to user space\n", __func__);
		return -EFAULT;
	}

	return ret;
}

/*
 * st21nfc_dev_write -- 0xffffff800895be54, 304 byte.
 */
static ssize_t st21nfc_dev_write(struct file *filp, const char __user *buf,
				 size_t count, loff_t *offset)
{
	struct st21nfc_dev *st21nfc_dev = container_of(filp->private_data,
						       struct st21nfc_dev,
						       st21nfc_device);
	char buffer[MAX_BUFFER_SIZE];
	int ret = -1;

	/*
	 * "f104105f cmp" (0xffffff800895be78)   cmp x2, #0x104
	 * "9a883053 csel" (0xffffff800895be8c)  csel x19, x2, x8, cc
	 */
	if (count > MAX_BUFFER_SIZE)
		count = MAX_BUFFER_SIZE;

	/* "9413ec78 bl" (0xffffff800895bee0)  -> ffffff8008e570c0 <__arch_copy_from_user> */
	if (copy_from_user(buffer, buf, count)) {
		/*
		 * "910ee800 add" (0xffffff800895bf6c)  add x0, x0, #0x3ba
		 *   -> '\x013%s : failed to copy from user space\n'
		 * "928001a0 mov" (0xffffff800895bf78)  mov x0, #0xfffffffffffffff2   // -14
		 */
		printk(KERN_ERR "%s : failed to copy from user space\n", __func__);
		return -EFAULT;
	}

	/*
	 * "f9403ec0 ldr" (0xffffff800895beec)  ldr x0, [x22,#120]   client
	 * "9404bc7a bl" (0xffffff800895bef8)   -> ffffff8008a8b0e0 <i2c_master_send>
	 */
	ret = i2c_master_send(st21nfc_dev->platform_data.client, buffer, count);
	/* "eb20c27f cmp" (0xffffff800895bf00)  cmp x19, w0, sxtw */
	if (ret != count) {
		/*
		 * "910f8400 add" (0xffffff800895bf10)  add x0, x0, #0x3e1
		 *   -> '\x013%s : i2c_master_send returned %d\n'
		 * "321d7be2 orr" (0xffffff800895bf1c)  orr w2, wzr, #0xfffffffb   // -5 = -EIO
		 */
		printk(KERN_ERR "%s : i2c_master_send returned %d\n", __func__, ret);
		ret = -EIO;
	}

	return ret;
}

/*
 * st21nfc_poll -- 0xffffff800895bf84, 200 byte.
 *
 * "f9406415 ldr" (0xffffff800895bf94)  ldr x21, [x0,#200]   filp->private_data
 * "d10062a1 sub" (0xffffff800895bfa0)  sub x1, x21, #0x18   &st21nfc_dev->read_wq
 * "d63f0100 blr" (0xffffff800895bfb0)  blr x8               poll_table->_qproc(filp, wq, wait)
 */
static unsigned int st21nfc_poll(struct file *filp, poll_table *wait)
{
	struct st21nfc_dev *st21nfc_dev = container_of(filp->private_data,
						       struct st21nfc_dev,
						       st21nfc_device);
	unsigned int mask = 0;
	unsigned long flags;
	int value;

	poll_wait(filp, &st21nfc_dev->read_wq, wait);

	/*
	 * "b94082a0 ldr" (0xffffff800895bfb4)   ldr w0, [x21,#128]
	 * "97ec62b5 bl" (0xffffff800895bfbc)    -> <gpiod_get_raw_value>
	 * "340002a0 cbz" (0xffffff800895bfc0)   cbz w0, ffffff800895c014
	 * "7100041f cmp" (0xffffff800895bfc4)   cmp w0, #0x1
	 * "b94092a8 ldr" (0xffffff800895bfcc)   ldr w8, [x21,#144]
	 * "7100051f cmp" (0xffffff800895bfd0)   cmp w8, #0x1
	 * "b94092a8 ldr" (0xffffff800895c014)   ldr w8, [x21,#144] + "34fffe08 cbz" (0xffffff800895c018)
	 */
	value = gpio_get_value(st21nfc_dev->platform_data.irq_gpio);
	if ((value == 0 && st21nfc_dev->platform_data.active_polarity == 0) ||
	    (value > 0 && st21nfc_dev->platform_data.active_polarity == 1)) {
		/*
		 * "910262b3 add" (0xffffff800895bfd8)  add x19, x21, #0x98   &irq_enabled_lock
		 * "94146b2a bl" (0xffffff800895bfe0)   -> <_raw_spin_lock_irqsave>
		 * "394142a8 ldrb" (0xffffff800895bfe4) ldrb w8, [x21,#80]    irq_enabled
		 * "97df7ad1 bl" (0xffffff800895bff8)   -> <disable_irq_nosync>
		 * "390142bf strb" (0xffffff800895bffc) strb wzr, [x21,#80]
		 * "52800820 mov" (0xffffff800895c00c)  mov w0, #0x41   // 65 = POLLIN | POLLRDNORM
		 */
		spin_lock_irqsave(&st21nfc_dev->platform_data.irq_enabled_lock,
				  flags);
		if (st21nfc_dev->irq_enabled) {
			disable_irq_nosync(st21nfc_dev->platform_data.client->irq);
			st21nfc_dev->irq_enabled = false;
		}
		spin_unlock_irqrestore(&st21nfc_dev->platform_data.irq_enabled_lock,
				       flags);
		mask |= POLLIN | POLLRDNORM;
	} else {
		/*
		 * "394142a8 ldrb" (0xffffff800895c01c) ldrb w8, [x21,#80]  + "350000c8 cbnz" (0xffffff800895c020)
		 * "390142a9 strb" (0xffffff800895c02c) strb w9, [x21,#80]  (w9 = 1)
		 * "97df7b60 bl" (0xffffff800895c034)   -> ffffff800813adb4 <enable_irq>
		 */
		if (!st21nfc_dev->irq_enabled) {
			st21nfc_dev->irq_enabled = true;
			enable_irq(st21nfc_dev->platform_data.client->irq);
		}
	}

	return mask;
}

/*
 * st21nfc_dev_ioctl -- 0xffffff800895c04c, 1696 byte.
 */
static long st21nfc_dev_ioctl(struct file *filp, unsigned int cmd,
			      unsigned long arg)
{
	struct st21nfc_dev *st21nfc_dev;
	struct miscdevice *misc = filp->private_data;
	unsigned int value;
	int ret = 0;
	int err = 0;

	/*
	 * "12181c29 and" (0xffffff800895c060)   and w9, w1, #0xff00
	 * "6b0a013f cmp" (0xffffff800895c068)   cmp w9, w10   (w10 = 0xea00)
	 * "b27bf7e0 orr" (0xffffff800895c110)   orr x0, xzr, #0xffffffffffffffe7   // -25
	 */
	if (_IOC_TYPE(cmd) != ST21NFC_MAGIC)
		return -ENOTTY;

	/*
	 * "f9406414 ldr" (0xffffff800895c070)   ldr x20, [x0,#200]   filp->private_data
	 * "d1006293 sub" (0xffffff800895c0e0)   sub x19, x20, #0x18
	 */
	st21nfc_dev = container_of(misc, struct st21nfc_dev, st21nfc_device);

	/*
	 * "36f80161 tbz" (0xffffff800895c078)   tbz w1, #31        _IOC_DIR & _IOC_READ
	 * "5310750a ubfx" (0xffffff800895c088)  ubfx w10, w8, #16, #14    _IOC_SIZE
	 * "36f00168 tbz" (0xffffff800895c0a4)   tbz w8, #30        _IOC_DIR & _IOC_WRITE
	 * "928001a0 mov" (0xffffff800895c158)   mov x0, #0xfffffffffffffff2   // -14 = -EFAULT
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg,
				 _IOC_SIZE(cmd));
	if (err == 0 && (_IOC_DIR(cmd) & _IOC_WRITE))
		err = !access_ok(VERIFY_READ, (void __user *)arg,
				 _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;

	switch (cmd) {
	case ST21NFC_GET_WAKEUP:
	case ST21NFC_LEGACY_GET_WAKEUP:
		/*
		 * "b9408280 ldr" (0xffffff800895c130)  ldr w0, [x20,#128]
		 * "97ec6256 bl" (0xffffff800895c138)   -> <gpiod_get_raw_value>
		 * "34001f60 cbz" (0xffffff800895c13c)  cbz w0, ffffff800895c528
		 * "7100041f cmp" (0xffffff800895c140)  cmp w0, #0x1
		 * "b9409288 ldr" (0xffffff800895c148)  ldr w8, [x20,#144]
		 * "7100051f cmp" (0xffffff800895c14c)  cmp w8, #0x1
		 * "b9409288 ldr" (0xffffff800895c528)  ldr w8, [x20,#144] + "35000d28 cbnz" (0xffffff800895c52c)
		 * "320003e8 orr" (0xffffff800895c530)  orr w8, wzr, #0x1
		 */
		ret = gpio_get_value(st21nfc_dev->platform_data.irq_gpio);
		if ((ret == 0 &&
		     st21nfc_dev->platform_data.active_polarity == 0) ||
		    (ret > 0 &&
		     st21nfc_dev->platform_data.active_polarity == 1))
			ret = 1;
		else
			ret = 0;
		break;

	case ST21NFC_PULSE_REQUEST:
	case ST21NFC_LEGACY_PULSE_REQUEST:
		/*
		 * "91128400 add" (0xffffff800895c1d4)  add x0, x0, #0x4a1
		 *                 -> '\x016%s Double Pulse Request\n'
		 * "9112f021 add" (0xffffff800895c1d8)  add x1, x1, #0x4bc -> 'st21nfc_dev_ioctl'
		 */
		printk(KERN_INFO "%s Double Pulse Request\n", __func__);
		/* "b9408681 ldr" (0xffffff800895c1e0)  ldr w1, [x20,#132] + "34002761 cbz" (0xffffff800895c1e4) */
		if (st21nfc_dev->platform_data.reset_gpio != 0) {
			/*
			 * "f942ae68 ldr" (0xffffff800895c1ec)  ldr x8, [x19,#1368]
			 * "f942b121 ldr" (0xffffff800895c1f8)  ldr x1, [x9,#1376]
			 * "320003e0 orr" (0xffffff800895c1fc)  orr w0, wzr, #0x1
			 * "d63f0100 blr" (0xffffff800895c200)  blr x8
			 */
			if (st21nfc_st54spi_cb != NULL)
				st21nfc_st54spi_cb(1, st21nfc_st54spi_data);

			/*
			 * "91133800 add" (0xffffff800895c20c)  add x0, x0, #0x4ce
			 *                 -> '\x016Pulse Request gpio is %d\n'
			 */
			printk(KERN_INFO "Pulse Request gpio is %d\n",
				st21nfc_dev->platform_data.reset_gpio);

			/*
			 * "97ec647e bl" (0xffffff800895c220)  <gpiod_set_raw_value> (w1 = 0)
			 * "52800280 mov" (0xffffff800895c224) mov w0, #0x14    // 20
			 * "97dfda7b bl" (0xffffff800895c228)  -> ffffff8008152c14 <msleep>
			 * then 1 / 0 / 1, always with msleep(20)
			 */
			gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 0);
			msleep(20);
			gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 1);
			msleep(20);
			gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 0);
			msleep(20);
			gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 1);
			msleep(20);

			/*
			 * "9113a800 add" (0xffffff800895c27c)  add x0, x0, #0x4ea
			 *          -> '\x016%s done Double Pulse Request\n'
			 */
			printk(KERN_INFO "%s done Double Pulse Request\n", __func__);

			/*
			 * "f942ae68 ldr" (0xffffff800895c288)  ldr x8, [x19,#1368]
			 * "2a1f03e0 mov" (0xffffff800895c298)  mov w0, wzr
			 * "d63f0100 blr" (0xffffff800895c29c)  blr x8
			 */
			if (st21nfc_st54spi_cb != NULL)
				st21nfc_st54spi_cb(0, st21nfc_st54spi_data);
		}
		break;

	case ST21NFC_SET_POLARITY_RISING:
	case ST21NFC_LEGACY_SET_POLARITY_RISING:
		/*
		 * "9110b800 add" (0xffffff800895c2a8)  add x0, x0, #0x42e
		 *   -> '\x016 ### ST21NFC_SET_POLARITY_RISING ###\n'
		 * "b20003e8 mov" (0xffffff800895c2b0)  mov x8, #0x100000001
		 * "f808c288 stur" (0xffffff800895c2b4) stur x8, [x20,#140]
		 *   -> polarity_mode (+0xa4) = 1, active_polarity (+0xa8) = 1
		 */
		printk(KERN_INFO " ### ST21NFC_SET_POLARITY_RISING ###\n");
		st21nfc_dev->platform_data.polarity_mode = IRQF_TRIGGER_RISING;
		st21nfc_dev->platform_data.active_polarity = 1;
		st21nfc_loc_set_polaritymode(st21nfc_dev, IRQF_TRIGGER_RISING);
		ret = 0;
		break;

	case ST21NFC_SET_POLARITY_FALLING:
	case ST21NFC_LEGACY_SET_POLARITY_FALLING:
		/*
		 * "91101400 add" (0xffffff800895c32c)  add x0, x0, #0x405
		 *   -> '\x016 ### ST21NFC_SET_POLARITY_FALLING ###\n'
		 * "321f03e8 orr" (0xffffff800895c334)  orr w8, wzr, #0x2
		 * "f808c288 stur" (0xffffff800895c338) stur x8, [x20,#140]
		 *   -> polarity_mode = 2, active_polarity = 0
		 */
		printk(KERN_INFO " ### ST21NFC_SET_POLARITY_FALLING ###\n");
		st21nfc_dev->platform_data.polarity_mode = IRQF_TRIGGER_FALLING;
		st21nfc_dev->platform_data.active_polarity = 0;
		st21nfc_loc_set_polaritymode(st21nfc_dev, IRQF_TRIGGER_FALLING);
		ret = 0;
		break;

	case ST21NFC_SET_POLARITY_HIGH:
	case ST21NFC_LEGACY_SET_POLARITY_HIGH:
		/*
		 * "9111ec00 add" (0xffffff800895c38c)  add x0, x0, #0x47b
		 *   -> '\x016 ### ST21NFC_SET_POLARITY_HIGH ###\n'
		 * "d2800088 mov" (0xffffff800895c394)  mov x8, #0x4
		 * "f2c00028 movk" (0xffffff800895c398) movk x8, #0x1, lsl #32
		 * "f808c288 stur" (0xffffff800895c39c) stur x8, [x20,#140]
		 *   -> polarity_mode = 4, active_polarity = 1
		 */
		printk(KERN_INFO " ### ST21NFC_SET_POLARITY_HIGH ###\n");
		st21nfc_dev->platform_data.polarity_mode = IRQF_TRIGGER_HIGH;
		st21nfc_dev->platform_data.active_polarity = 1;
		st21nfc_loc_set_polaritymode(st21nfc_dev, IRQF_TRIGGER_HIGH);
		ret = 0;
		break;

	case ST21NFC_SET_POLARITY_LOW:
	case ST21NFC_LEGACY_SET_POLARITY_LOW:
		/*
		 * "91115800 add" (0xffffff800895c3dc)  add x0, x0, #0x456
		 *   -> '\x016 ### ST21NFC_SET_POLARITY_LOW ###\n'
		 * "321d03e8 orr" (0xffffff800895c3e4)  orr w8, wzr, #0x8
		 * "f808c288 stur" (0xffffff800895c3e8) stur x8, [x20,#140]
		 *   -> polarity_mode = 8, active_polarity = 0
		 */
		printk(KERN_INFO " ### ST21NFC_SET_POLARITY_LOW ###\n");
		st21nfc_dev->platform_data.polarity_mode = IRQF_TRIGGER_LOW;
		st21nfc_dev->platform_data.active_polarity = 0;
		st21nfc_loc_set_polaritymode(st21nfc_dev, IRQF_TRIGGER_LOW);
		ret = 0;
		break;

	case ST21NFC_GET_POLARITY:
	case ST21NFC_LEGACY_GET_POLARITY:
		/* "b9408e88 ldr" (0xffffff800895c458)  ldr w8, [x20,#140] */
		ret = st21nfc_dev->platform_data.polarity_mode;
		break;

	case ST21NFC_RECOVERY:
	case ST21NFC_LEGACY_RECOVERY:
		/*
		 * "91142800 add" (0xffffff800895c468)  add x0, x0, #0x50a
		 *                 -> '\x016%s Recovery Request\n'
		 */
		printk(KERN_INFO "%s Recovery Request\n", __func__);
		/* "b9408680 ldr" (0xffffff800895c474)  ldr w0, [x20,#132] + "340007e0 cbz" (0xffffff800895c478) */
		if (st21nfc_dev->platform_data.reset_gpio != 0) {
			/*
			 * "3955a2a8 ldrb" (0xffffff800895c480) ldrb w8, [x21,#1384]
			 * "97df7e7e bl" (0xffffff800895c498)   -> <free_irq>
			 * "3915a2bf strb" (0xffffff800895c49c) strb wzr, [x21,#1384]
			 */
			if (irq_is_attached) {
				free_irq(st21nfc_dev->platform_data.client->irq,
					 st21nfc_dev);
				irq_is_attached = false;
			}
			/* pulse low for 20 millisecs */
			gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 0);
			msleep(20);
			gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 1);
			msleep(20);
			gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 0);
			msleep(20);

			/*
			 * "b9408280 ldr" (0xffffff800895c4e8)  ldr w0, [x20,#128]   irq_gpio
			 * "97ec5fdd bl" (0xffffff800895c4f4)   -> <gpiod_direction_output_raw> (1)
			 */
			ret = gpio_direction_output(st21nfc_dev->platform_data.irq_gpio,
						    1);
			if (ret) {
				/*
				 * "91148400 add" (0xffffff800895c500)  add x0, x0, #0x521
				 *   -> '\x013%s : gpio_direction_output failed\n'
				 * "9112f021 add" (0xffffff800895c5f4)  add x1, x1, #0x4bc
				 *                 -> 'st21nfc_dev_ioctl'
				 * "12800248 mov" (0xffffff800895c5fc)  mov w8, #0xffffffed  // -19
				 */
				printk(KERN_ERR "%s : gpio_direction_output failed\n",
				       __func__);
				ret = -ENODEV;
				break;
			}
			gpio_set_value(st21nfc_dev->platform_data.irq_gpio, 1);
			msleep(20);
			gpio_set_value(st21nfc_dev->platform_data.reset_gpio, 1);
			/*
			 * "91151800 add" (0xffffff800895c568)  add x0, x0, #0x546
			 *   -> '\x016%s done double Pulse Request\n'
			 */
			printk(KERN_INFO "%s done double Pulse Request\n", __func__);
		}
		msleep(20);
		gpio_set_value(st21nfc_dev->platform_data.irq_gpio, 0);
		msleep(20);
		gpio_set_value(st21nfc_dev->platform_data.irq_gpio, 1);
		msleep(20);
		gpio_set_value(st21nfc_dev->platform_data.irq_gpio, 0);
		msleep(20);
		/*
		 * "91159800 add" (0xffffff800895c5cc)  add x0, x0, #0x566
		 *   -> '\x016%s Recovery procedure finished\n'
		 */
		printk(KERN_INFO "%s Recovery procedure finished\n", __func__);
		/* "97ec5f2f bl" (0xffffff800895c5e0)  -> <gpiod_direction_input> */
		ret = gpio_direction_input(st21nfc_dev->platform_data.irq_gpio);
		if (ret) {
			/*
			 * "9100c800 add" (0xffffff800895c5ec)  add x0, x0, #0x32
			 *   -> '\x013%s : gpio_direction_input failed\n'
			 */
			printk(KERN_ERR "%s : gpio_direction_input failed\n", __func__);
			ret = -ENODEV;
		}
		break;

	case ST21NFC_USE_ESE:
		/*
		 * "2a1f03e8 mov" (0xffffff800895c644)  mov w8, wzr
		 * "b9400149 ldr" (0xffffff800895c648)  ldr w9, [x10]     get_user(value, arg)
		 * "128001a8 mov" (0xffffff800895c604)  mov w8, #0xfffffff2   // -14 = -EFAULT
		 */
		ret = get_user(value, (unsigned int __user *)arg);
		if (ret == 0) {
			/*
			 * "f942ad4a ldr" (0xffffff800895c6b0)  ldr x10, [x10,#1368]
			 * "f942b101 ldr" (0xffffff800895c6bc)  ldr x1, [x8,#1376]
			 * "7100013f cmp" (0xffffff800895c6c0)  cmp w9, #0x0
			 * "321f03e8 orr" (0xffffff800895c6c4)  orr w8, wzr, #0x2
			 * "1a880500 cinc" (0xffffff800895c6c8) cinc w0, w8, ne    -> 2 if 0, 3 if !=0
			 * "d63f0140 blr" (0xffffff800895c6cc)  blr x10
			 */
			if (st21nfc_st54spi_cb != NULL)
				st21nfc_st54spi_cb(value != 0 ? 3 : 2,
						   st21nfc_st54spi_data);
		}
		break;

	default:
		/*
		 * "91162000 add" (0xffffff800895c510)  add x0, x0, #0x588
		 *                 -> '\x013%s bad ioctl %u\n'
		 * "9112f021 add" (0xffffff800895c514)  add x1, x1, #0x4bc -> 'st21nfc_dev_ioctl'
		 * "128002a8 mov" (0xffffff800895c520)  mov w8, #0xffffffea   // -22 = -EINVAL
		 */
		printk(KERN_ERR "%s bad ioctl %u\n", __func__, cmd);
		ret = -EINVAL;
		break;
	}

	/* "93407d00 sxtw" (0xffffff800895c6d4)  sxtw x0, w8 */
	return ret;
}

/*
 * st21nfc_dev_open -- 0xffffff800895c6ec, 100 byte.
 *
 * "3955a509 ldrb" (0xffffff800895c6fc)  ldrb w9, [x8,#1385]    device_open
 * "7100053f cmp" (0xffffff800895c700)   cmp w9, #0x1
 * "9116f800 add" (0xffffff800895c710)   add x0, x0, #0x5be
 *                  -> '\x013%s : device already opened ret= %d\n'
 * "91179021 add" (0xffffff800895c714)   add x1, x1, #0x5e4 -> 'st21nfc_dev_open'
 * "321c6fe2 orr" (0xffffff800895c718)   orr w2, wzr, #0xfffffff0    // -16 = -EBUSY
 * "321c6ff3 orr" (0xffffff800895c71c)   orr w19, wzr, #0xfffffff0
 * "320003e9 orr" (0xffffff800895c72c)   orr w9, wzr, #0x1 + "3915a509 strb" (0xffffff800895c730) strb w9, [x8,#1385]
 * "321f03e0 orr" (0xffffff800895c734)   orr w0, wzr, #0x2      CLK_BUF_NFC = 2
 * "320003e1 orr" (0xffffff800895c738)   orr w1, wzr, #0x1
 * "97effb30 bl" (0xffffff800895c73c)    -> ffffff800855b3fc <clk_buf_ctrl>
 */
static int st21nfc_dev_open(struct inode *inode, struct file *filp)
{
	int ret = 0;

	if (device_open) {
		ret = -EBUSY;
		printk(KERN_ERR "%s : device already opened ret= %d\n", __func__, ret);
	} else {
		device_open = true;
	}

	clk_buf_ctrl(2, 1);

	return ret;
}

/*
 * st21nfc_release -- 0xffffff800895c750, 40 byte.
 *
 * "321f03e0 orr" (0xffffff800895c758)   orr w0, wzr, #0x2
 * "2a1f03e1 mov" (0xffffff800895c75c)   mov w1, wzr
 * "97effb27 bl" (0xffffff800895c760)    -> ffffff800855b3fc <clk_buf_ctrl>
 * "3915a51f strb" (0xffffff800895c768)  strb wzr, [x8,#1385]
 * "2a1f03e0 mov" (0xffffff800895c76c)   mov w0, wzr
 */
static int st21nfc_release(struct inode *inode, struct file *filp)
{
	clk_buf_ctrl(2, 0);
	device_open = false;

	return 0;
}

/*
 * st21nfc_dev_irq_handler -- 0xffffff800895c778, 112 byte.
 *
 * "9102c034 add" (0xffffff800895c788)   add x20, x1, #0xb0    &platform_data.irq_enabled_lock
 * "9414693d bl" (0xffffff800895c794)    -> ffffff8008e76c88 <_raw_spin_lock_irqsave>
 * "3941a268 ldrb" (0xffffff800895c798)  ldrb w8, [x19,#104]   irq_enabled
 * "f9404a68 ldr" (0xffffff800895c7a4)   ldr x8, [x19,#144]    client
 * "b942e900 ldr" (0xffffff800895c7a8)   ldr w0, [x8,#744]     client->irq
 * "97df78e4 bl" (0xffffff800895c7ac)    -> ffffff800813ab3c <disable_irq_nosync>
 * "3901a27f strb" (0xffffff800895c7b0)  strb wzr, [x19,#104]
 * "941469aa bl" (0xffffff800895c7bc)    -> ffffff8008e76e64 <_raw_spin_unlock_irqrestore>
 * "320007e1 orr" (0xffffff800895c7c0)   orr w1, wzr, #0x3     TASK_INTERRUPTIBLE|TASK_UNINTERRUPTIBLE
 * "320003e2 orr" (0xffffff800895c7c4)   orr w2, wzr, #0x1     nr_exclusive = 1
 * "aa1f03e3 mov" (0xffffff800895c7cc)   x3 = 0
 * "97df02ec bl" (0xffffff800895c7d0)    -> ffffff800811d380 <__wake_up>
 * "320003e0 orr" (0xffffff800895c7dc)   orr w0, wzr, #0x1     IRQ_HANDLED
 */
static irqreturn_t st21nfc_dev_irq_handler(int irq, void *dev_id)
{
	struct st21nfc_dev *st21nfc_dev = dev_id;
	unsigned long flags;

	spin_lock_irqsave(&st21nfc_dev->platform_data.irq_enabled_lock, flags);
	if (st21nfc_dev->irq_enabled) {
		disable_irq_nosync(st21nfc_dev->platform_data.client->irq);
		st21nfc_dev->irq_enabled = false;
	}
	spin_unlock_irqrestore(&st21nfc_dev->platform_data.irq_enabled_lock,
			       flags);

	wake_up(&st21nfc_dev->read_wq);

	return IRQ_HANDLED;
}

/*
 * st21nfc_show_i2c_addr() was reconstructed from the factory kernel disassembly (0xffffff800895c7e8, 60 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st21nfc_st21nfc.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t st21nfc_show_i2c_addr(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);

	if (client != NULL)
		return sprintf(buf, "0x%.2x\n", client->addr);
	return 0;
}

/*
 * st21nfc_change_i2c_addr() was reconstructed from the factory kernel disassembly (0xffffff800895c824, 176 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st21nfc_st21nfc.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t st21nfc_change_i2c_addr(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	struct st21nfc_dev *data = dev_get_drvdata(dev);
	long long val = 0;

	if (data != NULL && data->platform_data.client != NULL) {
		if (kstrtoll(buf, 10, &val) == 0) {
			mutex_lock(&data->platform_data.read_mutex);
			data->platform_data.client->addr = val;
			mutex_unlock(&data->platform_data.read_mutex);
			return count;
		}
		return -EINVAL;
	}

	return 0;
}

/*
 * st21nfc_version -- 0xffffff800895c8d4, 44 byte.
 *
 * "912f8421 add" (0xffffff800895c8e8)   add x1, x1, #0xbe1 -> 0xffffff8009226be1 '%s\n'
 * "9117d442 add" (0xffffff800895c8ec)   add x2, x2, #0x5f5 -> 0xffffff80092205f5 '2.2.0.1'
 * "941437a1 bl" (0xffffff800895c8f0)    -> ffffff8008e6a774 <sprintf>
 * "93407c00 sxtw" (0xffffff800895c8f4)  sxtw x0, w0
 */
static ssize_t st21nfc_version(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", DRIVER_VERSION);
}

/*
 * The sysfs group, read from .rela.dyn and not by eye.
 *
 * attribute_group at 0xffffff8009955450:
 *   +0x18 attrs -> 0xffffff8009955478
 * the array at 0xffffff8009955478: [0]=0xffffff8009955490, [1]=0xffffff80099554b0,
 *   [2]=NULL
 * device_attribute at 0xffffff8009955490:
 *   attr.name = 0xffffff80091bc537 'i2c_addr'
 *   attr.mode = 0x1a4 = 0644
 *   show  = st21nfc_show_i2c_addr
 *   store = st21nfc_change_i2c_addr
 * device_attribute at 0xffffff80099554b0:
 *   attr.name = 0xffffff800926923d 'version'
 *   attr.mode = 0x124 = 0444
 *   show  = st21nfc_version
 *   store = NULL
 */
static DEVICE_ATTR(i2c_addr, 0644, st21nfc_show_i2c_addr,
		   st21nfc_change_i2c_addr);
static DEVICE_ATTR(version, 0444, st21nfc_version, NULL);

static struct attribute *st21nfc_attrs[] = {
	&dev_attr_i2c_addr.attr,
	&dev_attr_version.attr,
	NULL,
};

static struct attribute_group st21nfc_attr_grp = {
	.attrs = st21nfc_attrs,
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008f70918).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_nfc_st21nfc_st21nfc.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const struct file_operations st21nfc_dev_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.read = st21nfc_dev_read,
	.write = st21nfc_dev_write,
	.poll = st21nfc_poll,
	.unlocked_ioctl = st21nfc_dev_ioctl,
	.compat_ioctl = st21nfc_dev_ioctl,
	.open = st21nfc_dev_open,
	.release = st21nfc_release,
};

/*
 * i2c_device_id at 0xffffff8008f708d8: one entry, {"st21nfc", 0}, then the
 * terminator (0xffffff8008f708f8, all zero).
 */
static const struct i2c_device_id st21nfc_id[] = {
	{"st21nfc", 0},
	{}
};

/*
 * of_device_id at 0xffffff8008f70748: one entry with compatible 'mediatek,nfc'
 * (read at +64, which is the offset of the compatible[128] field in struct
 * of_device_id), then the terminator at 0xffffff8008f70810.
 */
static const struct of_device_id nfc_switch_of_match[] = {
	{.compatible = "mediatek,nfc"},
	{},
};

/*
 * i2c_driver at 0xffffff8009955270 (in .data):
 *   +0x10 probe    st21nfc_probe
 *   +0x18 remove   st21nfc_remove
 *   +0x40 driver.name          -> 'st21nfc'
 *   +0x68 driver.of_match_table -> 0xffffff8008f70748
 *   +0xb8 id_table             -> 0xffffff8008f708d8
 * driver.owner (+0x50) is NULL, as for every built-in.
 */
static struct i2c_driver st21nfc_dev_driver = {
	.id_table = st21nfc_id,
	.probe = st21nfc_probe,
	.remove = st21nfc_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "st21nfc",
		.of_match_table = nfc_switch_of_match,
	},
};

/*
 * of_device_id a 0xffffff8008f705b8: compatible 'mediatek,nfc-gpio-v2',
 * terminator at 0xffffff8008f70680.
 */
static const struct of_device_id nfc_gpio_of_match[] = {
	{.compatible = "mediatek,nfc-gpio-v2"},
	{},
};

/*
 * platform_driver a 0xffffff80099553a0 (in .data):
 *   +0x00 probe   st21nfc_platform_probe
 *   +0x08 remove  st21nfc_platform_remove
 *   +0x28 driver.name           -> 'st21nfc'
 *   +0x50 driver.of_match_table -> 0xffffff8008f705b8
 */
static struct platform_driver st21nfc_platform_driver = {
	.probe = st21nfc_platform_probe,
	.remove = st21nfc_platform_remove,
	.driver = {
		.name = "st21nfc",
		.owner = THIS_MODULE,
		.of_match_table = nfc_gpio_of_match,
	},
};

/*
 * st21nfc_dev_init -- 0xffffff800937f14c, 60 bytes, in .init.text.
 *
 * "9139d400 add" (0xffffff800937f158)   add x0, x0, #0xe75 -> 0xffffff800921fe75
 *                  '\x016Loading st21nfc driver\n'
 * "97b6d0de bl" (0xffffff800937f15c)    -> ffffff80081334d4 <printk>
 * "910e8000 add" (0xffffff800937f164)   add x0, x0, #0x3a0 -> 0xffffff80099553a0
 *                  = &st21nfc_platform_driver
 * "aa1f03e1 mov" (0xffffff800937f168)   mov x1, xzr        owner = NULL (built-in)
 * "97c5c21c bl" (0xffffff800937f16c)    -> ffffff80084ef9dc <__platform_driver_register>
 * "9109c021 add" (0xffffff800937f174)   add x1, x1, #0x270 -> 0xffffff8009955270
 *                  = &st21nfc_dev_driver
 * "aa1f03e0 mov" (0xffffff800937f178)   mov x0, xzr
 * "97dc2d0f bl" (0xffffff800937f17c)    -> ffffff8008a8a5b8 <i2c_register_driver>
 * The value returned is i2c_register_driver's (there is no other mov w0).
 */
static int __init st21nfc_dev_init(void)
{
	printk(KERN_INFO "Loading st21nfc driver\n");

	platform_driver_register(&st21nfc_platform_driver);

	return i2c_add_driver(&st21nfc_dev_driver);
}

module_init(st21nfc_dev_init);

static void __exit st21nfc_dev_exit(void)
{
	printk(KERN_INFO "Unloading st21nfc driver\n");

	i2c_del_driver(&st21nfc_dev_driver);
}

module_exit(st21nfc_dev_exit);

/*
 * MODULE_VERSION is proven: it leaves the module_version_attribute at
 * 0xffffff8009955358 in the binary, with .module_name 'st21nfc' and
 * .version '2.2.0.1'.
 *
 * MODULE_LICENSE is mandatory and leaves no trace in a built-in (.modinfo
 * does not make it into the image). MODULE_AUTHOR and MODULE_DESCRIPTION are
 * NOT written: their text is not in the binary and inventing it would be
 * invented data.
 */
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
