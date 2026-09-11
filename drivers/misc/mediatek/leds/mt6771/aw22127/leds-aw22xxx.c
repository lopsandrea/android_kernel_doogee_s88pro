// SPDX-License-Identifier: GPL-2.0
/*
 * Awinic aw22xxx RGB LED controller, Doogee S88 Pro.
 *
 * The driver's private structure was derived from the offsets the functions
 * read and write on "chip" (the pointer obtained from dev_get_drvdata()),
 * built up field by field as each batch identified one -- the same method
 * used for aw87329.c.
 *
 * Reconstructed from the disassembly of the factory kernel.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/workqueue.h>

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct aw22xxx_chip {
	struct i2c_client *client;			/* 0x000 */
	struct device *dev;				/* 0x008 */
	struct led_classdev cdev;			/* 0x010, sizeof 0x150 measured */
	struct work_struct brightness_work;		/* 0x160 */
	struct work_struct task_work;			/* 0x180 */
	struct work_struct fw_work;			/* 0x1a0 */
	struct work_struct cfg_work;			/* 0x1c0 */
	struct hrtimer fw_timer;			/* 0x1e0, sizeof 0x40 read */
	struct mutex cfg_lock;				/* 0x220 */
	int reset_gpio;					/* 0x240 */
	int irq_gpio;					/* 0x244 */
	u8 flags;					/* 0x248 */
	u8 chipid;					/* 0x249 */
	u8 fw;						/* 0x24a */
	u8 fw_update;					/* 0x24b */
	u32 imax;					/* 0x24c */
	u8 reserved_250[0x254 - 0x250];		/* 0x250..0x253, not identified */
	u8 task0;					/* 0x254 */
	u8 task1;					/* 0x255 */
	u8 effect;					/* 0x256 */
	u8 cfg;						/* 0x257 */
	u32 rgb[9];					/* 0x258..0x27b */
};

/*
 * This section was reconstructed from the factory kernel disassembly (640 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct aw22xxx_container {
	unsigned int len;
	unsigned int version;
	unsigned int bist;
	unsigned int key;
	unsigned char data[];
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008f55b98).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const u8 aw22xxx_reg_access[256] = {
	0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x03,
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 0xffffff8008f55bd8 .. 0xffffff8008f55c96: 191 byte a 0 (indices 96..254) */
	[255] = 0x03,	/* 0xffffff8008f55c97, see above */
};

/*
 * The names of the 14 "cfg" configurations, 0x40 bytes each, read from .rodata
 * (batch 2, used by aw22xxx_cfg_show):
 * $ objdump -s --start-address=0xffffff8009932650 --stop-address=0xffffff80099329d0 oracolo/stock.elf
 * (only the non-null text of each 0x40-byte entry, with its start address
 * alongside -- the rest of every entry is zero, the compiler adds it by
 * itself for a char[N] array initialised with a string shorter than N)
 */
static const char aw22xxx_cfg_names[14][0x40] = {
	"aw22xxx_cfg_led_off.bin",		/* 0xffffff8009932650 */
	"aw22xxx_cfg_led_on.bin",		/* 0xffffff8009932690 */
	"aw22xxx_cfg_led_breath.bin",		/* 0xffffff80099326d0 */
	"aw22xxx_cfg_led_collision.bin",	/* 0xffffff8009932710 */
	"aw22xxx_cfg_led_skyline.bin",		/* 0xffffff8009932750 */
	"aw22xxx_cfg_led_flower.bin",		/* 0xffffff8009932790 */
	"aw22xxx_cfg_audio_skyline.bin",	/* 0xffffff80099327d0 */
	"aw22xxx_cfg_audio_flower.bin",	/* 0xffffff8009932810 */
	"music_sync.bin",			/* 0xffffff8009932850 */
	"call_reminder.bin",			/* 0xffffff8009932890 */
	"charging.bin",			/* 0xffffff80099328d0 */
	"full_charged.bin",			/* 0xffffff8009932910 */
	"power_onoff.bin",			/* 0xffffff8009932950 */
	"short_message_notice.bin",		/* 0xffffff8009932990 */
};

/*
 * The names of the 13 "imax" maximum currents, 0x20 bytes each (batch 2, used by
 * aw22xxx_imax_show):
 * $ objdump -s --start-address=0xffffff80099329d0 --stop-address=0xffffff8009932b70 oracolo/stock.elf
 */
static const char aw22xxx_imax_names[13][0x20] = {
	"AW22XXX_IMAX_2mA",	/* 0xffffff80099329d0 */
	"AW22XXX_IMAX_3mA",	/* 0xffffff80099329f0 */
	"AW22XXX_IMAX_4mA",	/* 0xffffff8009932a10 */
	"AW22XXX_IMAX_6mA",	/* 0xffffff8009932a30 */
	"AW22XXX_IMAX_9mA",	/* 0xffffff8009932a50 */
	"AW22XXX_IMAX_10mA",	/* 0xffffff8009932a70 */
	"AW22XXX_IMAX_15mA",	/* 0xffffff8009932a90 */
	"AW22XXX_IMAX_20mA",	/* 0xffffff8009932ab0 */
	"AW22XXX_IMAX_30mA",	/* 0xffffff8009932ad0 */
	"AW22XXX_IMAX_40mA",	/* 0xffffff8009932af0 */
	"AW22XXX_IMAX_45mA",	/* 0xffffff8009932b10 */
	"AW22XXX_IMAX_60mA",	/* 0xffffff8009932b30 */
	"AW22XXX_IMAX_75mA",	/* 0xffffff8009932b50 */
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008f55cd8, 32 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const u8 aw22xxx_imax_clamp_table[32] = {
	0x08, 0x00, 0x09, 0x01, 0x02, 0x0b, 0x03, 0x0c,
	0x04, 0x0e, 0x05, 0x06, 0x07, 0x00, 0x00, 0x00,
	0x00, 0x03, 0x06, 0x06, 0x06, 0x09, 0x0c, 0x0f,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/*
 * g_aw22xxx_chip() was reconstructed from the factory kernel disassembly (0xffffff8009d05000).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct aw22xxx_chip *g_aw22xxx_chip;

/*
 * Forward declarations: aw22xxx_led_effect1 (position 1) calls
 * aw22xxx_i2c_write, which in the factory build sits at position 2 (after), and
 * aw22xxx_i2c_probe (position 4) calls almost everything else in the file.
 * It is not an invention: it is what the emission order imposes (Task 2,
 * Step 1 of the plan). The real body stays at its position in the sequence.
 */
static void aw22xxx_i2c_write(struct aw22xxx_chip *chip, u8 reg, u8 value);
static int aw22xxx_i2c_read(struct aw22xxx_chip *chip, u8 reg, u8 *value);
static void aw22xxx_hw_reset(struct aw22xxx_chip *chip);
static irqreturn_t aw22xxx_irq(int irq, void *data);
static void aw22xxx_brightness_work(struct work_struct *work);
static void aw22xxx_task_work(struct work_struct *work);
static void aw22xxx_set_brightness(struct led_classdev *cdev,
				    enum led_brightness brightness);
static enum hrtimer_restart aw22xxx_fw_timer_func(struct hrtimer *timer);
static void aw22xxx_fw_work_routine(struct work_struct *work);
static void aw22xxx_cfg_work_routine(struct work_struct *work);
static void aw22xxx_fw_loaded(const struct firmware *cont, void *context);
static void aw22xxx_cfg_loaded(const struct firmware *cont, void *context);
static const struct attribute_group aw22xxx_attribute_group;

/*
 * aw22xxx_led_effect1() was reconstructed from the factory kernel disassembly (356 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void aw22xxx_led_effect1(void)
{
	aw22xxx_i2c_write(g_aw22xxx_chip, 0xff, 0x00); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x02, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x03, 0x10); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0c, 0x00); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x04, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x09, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x06, 0x6a); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x04, 0x03); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x41); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0b, 0x0f); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0c, 0x03); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x20);
}

/*
 * aw22xxx_i2c_write() was reconstructed from the factory kernel disassembly (148 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw22xxx_i2c_write(struct aw22xxx_chip *chip, u8 reg, u8 value)
{
	int ret;

	ret = i2c_smbus_write_byte_data(chip->client, reg, value);
	if (ret < 0) {
		printk(KERN_ERR "%s: i2c_write cnt=%d error=%d\n",
		       "aw22xxx_i2c_write", 0, ret);
		msleep(1);
		ret = i2c_smbus_write_byte_data(chip->client, reg, value);
		if (ret < 0) {
			printk(KERN_ERR "%s: i2c_write cnt=%d error=%d\n",
			       "aw22xxx_i2c_write", 1, ret);
			msleep(1);
		}
	}
}

/*
 * aw22xxx_led_effect2 -- position 3 of the target sequence, batch 2.
 * ffffff800879f5e8, 356 bytes. Global (T), zero parameters. Exactly the same
 * shape as aw22xxx_led_effect1 (same registers, same values, thirteen
 * msleep(1)), **except for the last write**: reg=0x05 val=0x2e (46) instead
 * of val=0x20 (32) -- read twice on the disassembly to be sure
 * it was not a transcription slip (ffffff800879f738: "mov w2,#0x2e").
 */
void aw22xxx_led_effect2(void)
{
	aw22xxx_i2c_write(g_aw22xxx_chip, 0xff, 0x00); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x02, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x03, 0x10); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0c, 0x00); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x04, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x09, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x06, 0x6a); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x04, 0x03); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x41); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0b, 0x0f); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0c, 0x03); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x2e);
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff80091c72e8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * aw22xxx_parse_dt() was reconstructed from the factory kernel disassembly (0xffffff80091c72a7).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int aw22xxx_parse_dt(struct device *dev, struct aw22xxx_chip *aw22xxx,
			     struct device_node *np)
{
	aw22xxx->reset_gpio = of_get_named_gpio(np, "reset-gpio", 0);
	if (aw22xxx->reset_gpio < 0) {
		dev_err(dev, "%s: no reset gpio provided, will not HW reset device\n",
			"aw22xxx_parse_dt");
		return -1;
	} else {
		dev_info(dev, "%s: reset gpio provided ok\n", "aw22xxx_parse_dt");
	}

	aw22xxx->irq_gpio = of_get_named_gpio(np, "irq-gpio", 0);
	if (aw22xxx->irq_gpio < 0) {
		dev_err(dev, "%s: no irq gpio provided, will not suppport intterupt\n",
			"aw22xxx_parse_dt");
		return -1;
	} else {
		dev_info(dev, "%s: irq gpio provided ok\n", "aw22xxx_parse_dt");
	}

	return 0;
}

/*
 * aw22xxx_read_chipid() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int aw22xxx_read_chipid(struct aw22xxx_chip *aw22xxx)
{
	unsigned char cnt = 0;
	unsigned char reg_val = 0;	/* "strb wzr,[sp,#12]"@ffffff800879f964 */
	int ret;

	aw22xxx_i2c_write(aw22xxx, 0xff, 0x00);
	aw22xxx_i2c_write(aw22xxx, 0x01, 0x55);
	msleep(2);

	while (cnt < 2) {
		ret = aw22xxx_i2c_read(aw22xxx, 0x01, &reg_val);
		if (ret < 0) {
			dev_err(aw22xxx->dev,
				"%s: failed to read register AW22XXX_REG_ID: %d\n",
				"aw22xxx_read_chipid", ret);
			return -EIO;
		}
		if (reg_val == 0x76) {
			printk(KERN_INFO "%s aw22xxx detected\n",
			       "aw22xxx_read_chipid");
			aw22xxx_i2c_read(aw22xxx, 0x00, &reg_val);
			switch (reg_val) {
			case 0x18:
				aw22xxx->chipid = 1;
				printk(KERN_INFO "%s: chipid: aw22118\n",
				       "aw22xxx_read_chipid");
				break;
			case 0x27:
				aw22xxx->chipid = 2;
				printk(KERN_INFO "%s: chipid: aw22127\n",
				       "aw22xxx_read_chipid");
				break;
			default:
				printk(KERN_ERR "%s: unknown id=0x%02x\n",
				       "aw22xxx_read_chipid", reg_val);
				break;
			}
			return 0;
		}
		printk(KERN_INFO "%s unsupported device revision (0x%x)\n",
		       "aw22xxx_read_chipid", reg_val);
		cnt++;
		msleep(1);
	}

	return -EINVAL;
}

/*
 * aw22xxx_interrupt_clear -- inlined into aw22xxx_i2c_probe.
 * ffffff800879fb40..ffffff800879fb6c. It merely reads register 0x0a
 * ("mov w1,#0xa"@ffffff800879fb50) and prints its value: reading it is what
 * clears the chip interrupt status, there is no write at all.
 *
 * Strings:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_interrupt_clear"@0xffffff80091c74a1
 *   KERN_INFO "%s: reg INTST=0x%x\n"@0xffffff80091c74b9
 */
static void aw22xxx_interrupt_clear(struct aw22xxx_chip *aw22xxx)
{
	unsigned char reg_val;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_interrupt_clear");
	aw22xxx_i2c_read(aw22xxx, 0x0a, &reg_val);
	printk(KERN_INFO "%s: reg INTST=0x%x\n", "aw22xxx_interrupt_clear",
	       reg_val);
}

/*
 * aw22xxx_interrupt_setup -- inlined into aw22xxx_i2c_probe.
 * ffffff800879fb28..ffffff800879fb90. It prints its own entry, calls
 * aw22xxx_interrupt_clear and sets bit 0 of register 0x09.
 *
 * Strings:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_interrupt_setup"@0xffffff80091c7489
 */
static void aw22xxx_interrupt_setup(struct aw22xxx_chip *aw22xxx)
{
	unsigned char reg_val;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_interrupt_setup");
	aw22xxx_interrupt_clear(aw22xxx);
	aw22xxx_i2c_read(aw22xxx, 0x09, &reg_val);
	aw22xxx_i2c_write(aw22xxx, 0x09, reg_val | 0x01);
}

/*
 * aw22xxx_parse_led_cdev() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int aw22xxx_parse_led_cdev(struct aw22xxx_chip *aw22xxx,
				   struct device_node *np)
{
	struct device_node *temp;
	int ret = -1;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_parse_led_cdev");

	for_each_child_of_node(np, temp) {
		ret = of_property_read_string(temp, "aw22xxx,name",
					       &aw22xxx->cdev.name);
		if (ret < 0) {
			dev_err(aw22xxx->dev,
				"Failure reading led name, ret = %d\n", ret);
			return ret;
		}
		ret = of_property_read_u32(temp, "aw22xxx,imax",
					    &aw22xxx->imax);
		if (ret < 0) {
			dev_err(aw22xxx->dev,
				"Failure reading imax, ret = %d\n", ret);
			return ret;
		}
		ret = of_property_read_u32(temp, "aw22xxx,brightness",
					    &aw22xxx->cdev.brightness);
		if (ret < 0) {
			dev_err(aw22xxx->dev,
				"Failure reading brightness, ret = %d\n", ret);
			return ret;
		}
		ret = of_property_read_u32(temp, "aw22xxx,max_brightness",
					    &aw22xxx->cdev.max_brightness);
		if (ret < 0) {
			dev_err(aw22xxx->dev,
				"Failure reading max brightness, ret = %d\n",
				ret);
			return ret;
		}
	}

	INIT_WORK(&aw22xxx->brightness_work, aw22xxx_brightness_work);
	INIT_WORK(&aw22xxx->task_work, aw22xxx_task_work);

	aw22xxx->cdev.brightness_set = aw22xxx_set_brightness;
	ret = led_classdev_register(aw22xxx->dev, &aw22xxx->cdev);
	if (ret) {
		dev_err(aw22xxx->dev, "unable to register led ret=%d\n", ret);
		return ret;
	}

	ret = sysfs_create_group(&aw22xxx->cdev.dev->kobj,
				  &aw22xxx_attribute_group);
	if (ret) {
		dev_err(aw22xxx->dev, "led sysfs ret: %d\n", ret);
		led_classdev_unregister(&aw22xxx->cdev);
		return ret;
	}

	return 0;
}

/*
 * aw22xxx_i2c_probe() was reconstructed from the factory kernel disassembly (0xffffff80091c71b9, 1904 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int aw22xxx_i2c_probe(struct i2c_client *client,
			      const struct i2c_device_id *id)
{
	struct aw22xxx_chip *aw22xxx;
	struct device_node *np = client->dev.of_node;
	int ret;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_i2c_probe");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "check_functionality failed\n");
		return -EIO;
	}

	aw22xxx = devm_kzalloc(&client->dev, sizeof(struct aw22xxx_chip),
			        GFP_KERNEL);
	g_aw22xxx_chip = aw22xxx;
	if (aw22xxx == NULL)
		return -ENOMEM;

	aw22xxx->client = client;
	aw22xxx->dev = &client->dev;
	i2c_set_clientdata(client, aw22xxx);
	mutex_init(&aw22xxx->cfg_lock);

	if (np) {
		ret = aw22xxx_parse_dt(&client->dev, g_aw22xxx_chip, np);
		if (ret) {
			dev_err(&client->dev,
				"%s: failed to parse device tree node\n",
				"aw22xxx_i2c_probe");
			ret = -1;
			goto err_kfree;
		}
	} else {
		g_aw22xxx_chip->reset_gpio = -1;
		g_aw22xxx_chip->irq_gpio = -1;
	}

	if (gpio_is_valid(g_aw22xxx_chip->reset_gpio)) {
		ret = devm_gpio_request_one(&client->dev,
					     g_aw22xxx_chip->reset_gpio,
					     GPIOF_OUT_INIT_LOW, "aw22xxx_rst");
		if (ret) {
			dev_err(&client->dev, "%s: rst request failed\n",
				"aw22xxx_i2c_probe");
			goto err_kfree;
		}
	}

	if (gpio_is_valid(g_aw22xxx_chip->irq_gpio)) {
		ret = devm_gpio_request_one(&client->dev,
					     g_aw22xxx_chip->irq_gpio,
					     GPIOF_IN, "aw22xxx_int");
		if (ret) {
			dev_err(&client->dev, "%s: int request failed\n",
				"aw22xxx_i2c_probe");
			goto err_kfree;
		}
	}

	aw22xxx_hw_reset(g_aw22xxx_chip);

	ret = aw22xxx_read_chipid(g_aw22xxx_chip);
	if (ret < 0) {
		dev_err(&client->dev, "%s: aw22xxx_read_chipid failed ret=%d\n",
			"aw22xxx_i2c_probe", ret);
		goto err_gpio_free;
	}

	if (gpio_is_valid(g_aw22xxx_chip->irq_gpio) &&
	    !(g_aw22xxx_chip->flags & 0x01)) {
		aw22xxx_interrupt_setup(g_aw22xxx_chip);
		ret = devm_request_threaded_irq(&client->dev,
						 gpio_to_irq(g_aw22xxx_chip->irq_gpio),
						 NULL, aw22xxx_irq,
						 IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
						 "aw22xxx", g_aw22xxx_chip);
		if (ret) {
			dev_err(&client->dev,
				"%s: failed to request IRQ %d: %d\n",
				"aw22xxx_i2c_probe",
				gpio_to_irq(g_aw22xxx_chip->irq_gpio), ret);
			goto err_gpio_free;
		}
	} else {
		dev_info(&client->dev, "%s skipping IRQ registration\n",
			  "aw22xxx_i2c_probe");
		g_aw22xxx_chip->flags |= 0x01;
	}

	i2c_set_clientdata(client, g_aw22xxx_chip);
	aw22xxx_parse_led_cdev(g_aw22xxx_chip, np);

	aw22xxx = g_aw22xxx_chip;
	hrtimer_init(&aw22xxx->fw_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	aw22xxx->fw_timer.function = aw22xxx_fw_timer_func;
	INIT_WORK(&aw22xxx->fw_work, aw22xxx_fw_work_routine);
	INIT_WORK(&aw22xxx->cfg_work, aw22xxx_cfg_work_routine);
	hrtimer_start(&aw22xxx->fw_timer, ktime_set(10, 0), HRTIMER_MODE_REL);

	printk(KERN_INFO "%s probe completed successfully!\n",
	       "aw22xxx_i2c_probe");
	return 0;

err_gpio_free:
	devm_gpio_free(&client->dev, g_aw22xxx_chip->reset_gpio);
	devm_gpio_free(&client->dev, g_aw22xxx_chip->irq_gpio);
err_kfree:
	devm_kfree(&client->dev, g_aw22xxx_chip);
	g_aw22xxx_chip = NULL;
	return ret;
}

/*
 * aw22xxx_i2c_remove() was reconstructed from the factory kernel disassembly (176 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int aw22xxx_i2c_remove(struct i2c_client *client)
{
	struct aw22xxx_chip *aw22xxx = i2c_get_clientdata(client);

	printk(KERN_INFO "%s: enter\n", "aw22xxx_i2c_remove");

	sysfs_remove_group(&aw22xxx->cdev.dev->kobj, &aw22xxx_attribute_group);
	led_classdev_unregister(&aw22xxx->cdev);

	devm_free_irq(&client->dev, gpio_to_irq(aw22xxx->irq_gpio), aw22xxx);

	if (gpio_is_valid(aw22xxx->reset_gpio))
		devm_gpio_free(&client->dev, aw22xxx->reset_gpio);
	if (gpio_is_valid(aw22xxx->irq_gpio))
		devm_gpio_free(&client->dev, aw22xxx->irq_gpio);

	devm_kfree(&client->dev, aw22xxx);

	return 0;
}

/*
 * aw22xxx_hw_reset() was reconstructed from the factory kernel disassembly (0xffffff80091c719a, 156 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw22xxx_hw_reset(struct aw22xxx_chip *chip)
{
	printk(KERN_INFO "%s: enter\n", "aw22xxx_hw_reset");
	mutex_lock(&chip->cfg_lock);
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff800879ffa4).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	if (chip && gpio_is_valid(chip->reset_gpio)) {
		gpiod_set_raw_value_cansleep(gpio_to_desc(chip->reset_gpio), 0);
		msleep(1);
		gpiod_set_raw_value_cansleep(gpio_to_desc(chip->reset_gpio), 1);
		msleep(1);
	} else {
		dev_err(chip->dev, "%s:  failed\n", "aw22xxx_hw_reset");
	}
	mutex_unlock(&chip->cfg_lock);
}

/*
 * aw22xxx_irq() was reconstructed from the factory kernel disassembly (344 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static irqreturn_t aw22xxx_irq(int irq, void *data)
{
	struct aw22xxx_chip *chip = data;
	u8 reg_val;
	u8 reg_val1;
	u8 reg_val2;
	u8 reg_val3;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_irq");

	aw22xxx_i2c_read(chip, 0x0a, &reg_val);
	printk(KERN_INFO "%s: reg INTST=0x%x\n", "aw22xxx_irq", reg_val);

	if (reg_val & 0x01) {
		printk(KERN_INFO "%s: functions compelte!\n", "aw22xxx_irq");

		aw22xxx_i2c_write(chip, 0xff, 0x00);
		aw22xxx_i2c_read(chip, 0x04, &reg_val1);
		aw22xxx_i2c_write(chip, 0x04, reg_val1 & (~0x02));
		aw22xxx_i2c_read(chip, 0x04, &reg_val2);
		aw22xxx_i2c_write(chip, 0x04, reg_val2 & (~0x01));
		aw22xxx_i2c_read(chip, 0x02, &reg_val3);
		aw22xxx_i2c_write(chip, 0x02, reg_val3 & (~0x01));
		msleep(2);

		printk(KERN_INFO "%s: enter standby mode!\n", "aw22xxx_irq");
	}

	printk(KERN_INFO "%s exit\n", "aw22xxx_irq");

	return IRQ_HANDLED;
}

/*
 * aw22xxx_i2c_read() was reconstructed from the factory kernel disassembly (164 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int aw22xxx_i2c_read(struct aw22xxx_chip *chip, u8 reg, u8 *value)
{
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, reg);
	if (ret < 0) {
		printk(KERN_ERR "%s: i2c_read cnt=%d error=%d\n",
		       "aw22xxx_i2c_read", 0, ret);
		msleep(1);
		ret = i2c_smbus_read_byte_data(chip->client, reg);
		if (ret < 0) {
			printk(KERN_ERR "%s: i2c_read cnt=%d error=%d\n",
			       "aw22xxx_i2c_read", 1, ret);
			msleep(1);
			return ret;
		}
	}
	*value = (u8)ret;
	return ret;
}

/*
 * aw22xxx_brightness_work() was reconstructed from the factory kernel disassembly (464 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw22xxx_brightness_work(struct work_struct *work)
{
	struct aw22xxx_chip *chip =
		container_of(work, struct aw22xxx_chip, brightness_work);
	u8 reg_val;
	u8 reg_val1;
	u8 reg_val2;
	u8 reg_val3;
	u8 reg_val4;
	u8 imax;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_brightness_work");

	aw22xxx_i2c_write(chip, 0x05, 0xff);
	aw22xxx_i2c_read(chip, 0x04, &reg_val);
	aw22xxx_i2c_write(chip, 0x04, reg_val & (~0x02));
	aw22xxx_i2c_read(chip, 0x02, &reg_val1);
	aw22xxx_i2c_write(chip, 0x02, reg_val1 & (~0x01));
	msleep(2);

	if (chip->cdev.brightness) {
		aw22xxx_i2c_read(chip, 0x02, &reg_val2);
		aw22xxx_i2c_write(chip, 0x02, reg_val2 | 0x01);
		msleep(2);
		aw22xxx_i2c_read(chip, 0x04, &reg_val3);
		aw22xxx_i2c_write(chip, 0x04, reg_val3 | 0x01);

		imax = min_t(u8, chip->imax, 0x0f);
		aw22xxx_i2c_write(chip, 0xff, 0x00);
		aw22xxx_i2c_write(chip, 0x0b, imax);

		aw22xxx_i2c_write(chip, 0x21, 0xe1);
		aw22xxx_i2c_write(chip, 0x22, 0x00);
		aw22xxx_i2c_write(chip, 0x20, 0x02);
		aw22xxx_i2c_write(chip, 0x23, 0x3d);
		aw22xxx_i2c_write(chip, 0x20, 0x00);

		aw22xxx_i2c_write(chip, 0x05, 0x82);

		aw22xxx_i2c_read(chip, 0x04, &reg_val4);
		aw22xxx_i2c_write(chip, 0x04, reg_val4 | 0x02);
	}
}

/*
 * aw22xxx_task_work() was reconstructed from the factory kernel disassembly (476 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw22xxx_task_work(struct work_struct *work)
{
	struct aw22xxx_chip *chip =
		container_of(work, struct aw22xxx_chip, task_work);
	u8 reg_val;
	u8 reg_val1;
	u8 reg_val2;
	u8 reg_val3;
	u8 reg_val4;
	u8 imax;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_task_work");

	aw22xxx_i2c_write(chip, 0x05, 0xff);
	aw22xxx_i2c_read(chip, 0x04, &reg_val);
	aw22xxx_i2c_write(chip, 0x04, reg_val & (~0x02));
	aw22xxx_i2c_read(chip, 0x02, &reg_val1);
	aw22xxx_i2c_write(chip, 0x02, reg_val1 & (~0x01));
	msleep(2);

	if (chip->task0) {
		aw22xxx_i2c_read(chip, 0x02, &reg_val2);
		aw22xxx_i2c_write(chip, 0x02, reg_val2 | 0x01);
		msleep(2);
		aw22xxx_i2c_read(chip, 0x04, &reg_val3);
		aw22xxx_i2c_write(chip, 0x04, reg_val3 | 0x01);

		imax = min_t(u8, chip->imax, 0x0f);
		aw22xxx_i2c_write(chip, 0xff, 0x00);
		aw22xxx_i2c_write(chip, 0x0b, imax);

		aw22xxx_i2c_write(chip, 0x21, 0xe1);
		aw22xxx_i2c_write(chip, 0x22, 0x00);
		aw22xxx_i2c_write(chip, 0x20, 0x02);
		aw22xxx_i2c_write(chip, 0x23, 0x3d);
		aw22xxx_i2c_write(chip, 0x20, 0x00);

		aw22xxx_i2c_write(chip, 0x05, chip->task0);
		aw22xxx_i2c_write(chip, 0x06, chip->task1);

		aw22xxx_i2c_read(chip, 0x04, &reg_val4);
		aw22xxx_i2c_write(chip, 0x04, reg_val4 | 0x02);
	}
}

/*
 * aw22xxx_set_brightness() was reconstructed from the factory kernel disassembly (40 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw22xxx_set_brightness(struct led_classdev *cdev,
				    enum led_brightness brightness)
{
	struct aw22xxx_chip *chip =
		container_of(cdev, struct aw22xxx_chip, cdev);

	chip->cdev.brightness = brightness;
	schedule_work(&chip->brightness_work);
}

/*
 * aw22xxx_reg_show() was reconstructed from the factory kernel disassembly (232 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw22xxx_reg_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	u8 probe_val = 0;
	u8 val = 0;
	ssize_t len = 0;
	int i;

	aw22xxx_i2c_read(chip, 0xff, &probe_val);
	for (i = 0; i < 256; i++) {
		if (!probe_val && !(aw22xxx_reg_access[i] & 0x1))
			continue;
		aw22xxx_i2c_read(chip, i, &val);
		len += snprintf(buf + len, PAGE_SIZE - len,
				 "reg:0x%02x=0x%02x \n", i, val);
	}
	return len;
}

/*
 * aw22xxx_reg_store() was reconstructed from the factory kernel disassembly (0xffffff80091411fa, 136 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw22xxx_reg_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int databuf[2] = { 0, 0 };

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2)
		aw22xxx_i2c_write(chip, (u8)databuf[0], (u8)databuf[1]);
	return count;
}

/*
 * aw22xxx_hwen_show -- position 14, batch 2. ffffff80087a0748, 72 bytes.
 * String: "hwen=%d\n"@0xffffff80091c7680.
 */
static ssize_t aw22xxx_hwen_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);

	return snprintf(buf, PAGE_SIZE, "hwen=%d\n",
			 gpiod_get_raw_value(gpio_to_desc(chip->reset_gpio)));
}

/*
 * aw22xxx_hwen_store() was reconstructed from the factory kernel disassembly (0xffffff800926f8f8, 236 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw22xxx_hwen_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val = 0;	/* "str wzr,[sp,#4]"@ffffff80087a07cc */

	if (sscanf(buf, "%x", &val) == 1) {
		if (val == 1) {
			aw22xxx_hw_reset(chip);
		} else {
			printk(KERN_INFO "%s: enter\n", "aw22xxx_hw_off");
			if (chip && gpio_is_valid(chip->reset_gpio)) {
				gpiod_set_raw_value_cansleep(
					gpio_to_desc(chip->reset_gpio), 0);
				msleep(1);
			} else {
				dev_err(chip->dev, "%s:  failed\n",
					"aw22xxx_hw_off");
			}
		}
	}
	return count;
}

/*
 * aw22xxx_fw_show -- position 16, batch 2. ffffff80087a087c, 48 bytes.
 * String: "firmware name = %s\n"@0xffffff80091c7698. The second argument
 * (x3, adrp+add at 0x91c76ac) is another constant string, not a field of the
 * chip: "aw22xxx_fw.bin"@0xffffff80091c76ac -- the file name is always
 * that one, the attribute does not read it from any state.
 */
static ssize_t aw22xxx_fw_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "firmware name = %s\n",
			 "aw22xxx_fw.bin");
}

/*
 * aw22xxx_fw_store() was reconstructed from the factory kernel disassembly (0xffffff800926f8f8, 152 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw22xxx_fw_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val = 0;	/* "str wzr,[sp,#4]"@ffffff80087a08e4 */

	if (sscanf(buf, "%x", &val) == 1) {
		chip->fw = (u8)val;
		if (val == 1)
			schedule_work(&chip->fw_work);
	}
	return count;
}

/*
 * aw22xxx_cfg_show() was reconstructed from the factory kernel disassembly (0xffffff80091c76bb, 164 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw22xxx_cfg_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	ssize_t len = 0;
	int i;

	for (i = 0; i < 14; i++)
		len += snprintf(buf + len, PAGE_SIZE - len, "cfg[%x] = %s\n",
				 i, aw22xxx_cfg_names[i]);
	len += snprintf(buf + len, PAGE_SIZE - len, "current cfg = %s\n",
			 aw22xxx_cfg_names[chip->effect]);
	return len;
}

/*
 * aw22xxx_cfg_store -- position 19, batch 2. ffffff80087a09e8, 148 bytes.
 *
 * Format "%d"@0xffffff8009216030 (not "%x": different from hwen/fw/task0/
 * task1/imax, verified by reading the bytes at the address), one conversion
 * (cmp w0,#0x1). chip->cfg = (u8)val (offset 583) always, if converted; if
 * the value is non-zero (the cbz jumps otherwise),
 * schedule_work(&chip->cfg_work) (offset cdev+0x1b0=chip+0x1c0; the "8" in
 * "orr w0,wzr,#0x8"@ffffff80087a0a48 is WORK_CPU_UNBOUND, see the note at
 * the top of the file).
 */
static ssize_t aw22xxx_cfg_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val = 0;	/* "str wzr,[sp,#4]"@ffffff80087a0a20 */

	if (sscanf(buf, "%d", &val) == 1) {
		chip->cfg = (u8)val;
		if (chip->cfg)
			schedule_work(&chip->cfg_work);
	}
	return count;
}

/*
 * aw22xxx_effect_show() was reconstructed from the factory kernel disassembly (0xffffff80091c7758, 48 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw22xxx_effect_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);

	return snprintf(buf, PAGE_SIZE, "effect = 0x%02x\n", chip->effect);
}

/*
 * aw22xxx_effect_store -- position 21, batch 2. ffffff80087a0aac, 108 bytes.
 * Format "%d"@0xffffff8009216030 (as in cfg_store). **No check on the
 * return value of sscanf** (no cmp/branch between the bl sscanf and the
 * read of [sp,#4]): the same bug of an uninitialised "val" read in
 * aw22xxx_task0_store, reproduced the same way (val uninitialised).
 */
static ssize_t aw22xxx_effect_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val;

	sscanf(buf, "%d", &val);
	chip->effect = (u8)val;
	return count;
}

/*
 * aw22xxx_imax_show() was reconstructed from the factory kernel disassembly (0xffffff80091c76db, 464 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw22xxx_imax_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	ssize_t len = 0;
	int i;

	for (i = 0; i < 13; i++)
		len += snprintf(buf + len, PAGE_SIZE - len, "imax[%x] = %s\n",
				 i, aw22xxx_imax_names[i]);
	len += snprintf(buf + len, PAGE_SIZE - len,
			 "current id = 0x%02x, imax = %s\n",
			 chip->imax, aw22xxx_imax_names[chip->imax]);
	return len;
}

/*
 * aw22xxx_imax_store() was reconstructed from the factory kernel disassembly (0xffffff800926f8f8, 180 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw22xxx_imax_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val;
	u8 clamp;

	sscanf(buf, "%x", &val);
	clamp = aw22xxx_imax_clamp_table[val];
	chip->imax = val;
	if (clamp >= 0xf)
		clamp = 0xf;
	aw22xxx_i2c_write(chip, 0xff, 0x00);
	aw22xxx_i2c_write(chip, 0x0b, clamp);
	return count;
}

/*
 * aw22xxx_rgb_show -- position 24, batch 2. ffffff80087a0d9c, 312 bytes.
 * Nine lines "rgb[%d] = 0x%06x\n"@0xffffff80091c770a, one per chip->rgb[i]
 * (offset 584+4i, batch 2, i=0..8).
 */
static ssize_t aw22xxx_rgb_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	ssize_t len = 0;
	int i;

	for (i = 0; i < 9; i++)
		len += snprintf(buf + len, PAGE_SIZE - len,
				 "rgb[%d] = 0x%06x\n", i, chip->rgb[i]);
	return len;
}

/*
 * aw22xxx_rgb_store -- position 25, batch 2. ffffff80087a0ed4, 124 bytes.
 * Format "%x %x"@0xffffff80091411fa (the same as aw22xxx_reg_store), index
 * and value. **No check on the return value of sscanf nor on the
 * bounds of the index** (0..8 expected, not verified: "add x8,x20,x8,lsl#2"
 * @ffffff80087a0f1c reads the raw index, an index >8 writes outside
 * chip->rgb[]).
 *
 * A DEFECT CORRECTED (batch 3 review, D4): with separate "unsigned int idx, val"
 * the function measured 116 against the factory's 124 (-8), the same
 * gap and the same cause as aw22xxx_reg_store above -- the factory source
 * uses a two-element array, not two scalars. With
 * "unsigned int databuf[2]" the function is 124/124, instruction for
 * instruction identical to the factory (the same "mov x9,sp; add x3,x9,#0x4"
 * verified by compiling outside the tree).
 */
static ssize_t aw22xxx_rgb_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int databuf[2];

	sscanf(buf, "%x %x", &databuf[0], &databuf[1]);
	chip->rgb[databuf[0]] = databuf[1];
	return count;
}

/*
 * aw22xxx_task0_show -- position 26 of the target sequence, batch 1.
 * ffffff80087a0f50, 48 bytes. It is one of the five 48-byte *_show functions (fw,
 * effect, task0, task1, ledeffect): the driver's minimal print, calibrated
 * here before being repeated (see batch 2).
 *
 * String: "task0 = 0x%02x\n"@0xffffff80091c7722.
 */
static ssize_t aw22xxx_task0_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);

	return snprintf(buf, PAGE_SIZE, "task0 = 0x%02x\n", chip->task0);
}

/*
 * aw22xxx_task0_store() was reconstructed from the factory kernel disassembly (0xffffff800926f8f8, 128 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw22xxx_task0_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val;

	sscanf(buf, "%x", &val);
	chip->task0 = (u8)val;
	schedule_work(&chip->task_work);
	return count;
}

/*
 * aw22xxx_task1_show -- position 28, batch 2. ffffff80087a1000, 48 bytes.
 * Exactly the same shape as aw22xxx_task0_show: the minimal print confirmed a
 * fourth time (fw, effect, task0 already in batch 1; task1 here).
 * String: "task1 = 0x%02x\n"@0xffffff80091c7738. Field chip->task1
 * (offset 581, batch 2).
 */
static ssize_t aw22xxx_task1_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);

	return snprintf(buf, PAGE_SIZE, "task1 = 0x%02x\n", chip->task1);
}

/*
 * aw22xxx_task1_store -- position 29, batch 2. ffffff80087a1030, 108 bytes.
 * The same shape as aw22xxx_effect_store: format "%x"@0xffffff800926f8f8, return
 * value **unchecked**, "val" uninitialised if sscanf fails.
 * It writes chip->task1 (offset 581) and returns, without queueing any work
 * (unlike task0_store: there is no bl queue_work_on here).
 */
static ssize_t aw22xxx_task1_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val;

	sscanf(buf, "%x", &val);
	chip->task1 = (u8)val;
	return count;
}

/*
 * aw22xxx_ledeffect_show -- position 30, batch 2. ffffff80087a109c, 48 bytes.
 * The fifth and last of the 48-byte *_show functions (fw, effect, task0, task1, now
 * ledeffect): the print comes out identical a fifth time. Unlike the
 * others, it reads from a **global**, not from chip: "ldr w3,[x8,#4084]" from
 * adrp x8,0xffffff8009d05000 -- address 0xffffff8009d05ff4, 12 bytes after
 * g_aw22xxx_chip (0xffffff8009d05fe8).
 *
 * String: "idflageffect = 0x%02x\n"@0xffffff80091c7752 (**not**
 * "effect = 0x%02x\n": that is the shared tail aw22xxx_effect_show
 * uses, six bytes later -- see the note there. The instruction offset
 * here is "add x2,x2,#0x752"@ffffff80087a10b4, read on *this*
 * function).
 */
static int g_aw22xxx_ledeffect;

static ssize_t aw22xxx_ledeffect_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	return snprintf(buf, PAGE_SIZE, "idflageffect = 0x%02x\n", g_aw22xxx_ledeffect);
}

/*
 * aw22xxx_ledeffect_store() was reconstructed from the factory kernel disassembly (0xffffff8009216030, 136 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw22xxx_ledeffect_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned int val;

	sscanf(buf, "%d", &val);
	g_aw22xxx_ledeffect = val;
	if (val == 2)
		aw22xxx_led_effect2();
	else if (val == 1)
		aw22xxx_led_effect1();
	return count;
}

/*
 * aw22xxx_fw_timer_func() was reconstructed from the factory kernel disassembly (72 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static enum hrtimer_restart aw22xxx_fw_timer_func(struct hrtimer *timer)
{
	struct aw22xxx_chip *chip =
		container_of(timer, struct aw22xxx_chip, fw_timer);

	printk(KERN_INFO "%s: enter\n", "aw22xxx_fw_timer_func");

	schedule_work(&chip->fw_work);

	return HRTIMER_NORESTART;
}

/*
 * aw22xxx_fw_update() was reconstructed from the factory kernel disassembly (0xffffff80091c7797, 136 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw22xxx_fw_update(struct aw22xxx_chip *chip)
{
	printk(KERN_INFO "%s: enter\n", "aw22xxx_fw_update");

	chip->fw_update = 1;
	request_firmware_nowait(THIS_MODULE, 1, "aw22xxx_fw.bin", chip->dev,
				 GFP_KERNEL, chip, aw22xxx_fw_loaded);
}

static void aw22xxx_fw_work_routine(struct work_struct *work)
{
	struct aw22xxx_chip *chip =
		container_of(work, struct aw22xxx_chip, fw_work);

	printk(KERN_INFO "%s: enter\n", "aw22xxx_fw_work_routine");

	aw22xxx_fw_update(chip);
}

/*
 * aw22xxx_cfg_update() was reconstructed from the factory kernel disassembly (0xffffff80091c7ae1, 240 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw22xxx_cfg_update(struct aw22xxx_chip *chip)
{
	printk(KERN_INFO "%s: enter\n", "aw22xxx_cfg_update");

	if (chip->effect > 13) {
		printk(KERN_ERR "%s: effect 0x%02x over max value \n",
		       "aw22xxx_cfg_update", chip->effect);
		return;
	}

	printk(KERN_INFO "%s: cfg name=%s\n", "aw22xxx_cfg_update",
	       aw22xxx_cfg_names[chip->effect]);

	if (chip->fw_update != 2) {
		printk(KERN_ERR "%s: fw update error: not compelte \n",
		       "aw22xxx_cfg_update");
		return;
	}

	mutex_lock(&chip->cfg_lock);
	request_firmware_nowait(THIS_MODULE, 1,
				 aw22xxx_cfg_names[chip->effect], chip->dev,
				 GFP_KERNEL, chip, aw22xxx_cfg_loaded);
}

static void aw22xxx_cfg_work_routine(struct work_struct *work)
{
	struct aw22xxx_chip *chip =
		container_of(work, struct aw22xxx_chip, cfg_work);

	printk(KERN_INFO "%s: enter\n", "aw22xxx_cfg_work_routine");

	aw22xxx_cfg_update(chip);
}

/*
 * aw22xxx_i2c_writes() was reconstructed from the factory kernel disassembly (1 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int aw22xxx_i2c_writes(struct aw22xxx_chip *chip, u8 reg,
			       u8 *buf, unsigned int len)
{
	u8 *data;
	int ret;

	data = kmalloc(len + 1, GFP_KERNEL);
	if (data == NULL) {
		printk(KERN_ERR "%s: can not allocate memory\n",
		       "aw22xxx_i2c_writes");
		return -ENOMEM;
	}

	data[0] = reg;
	memcpy(&data[1], buf, len);

	ret = i2c_master_send(chip->client, data, len + 1);
	if (ret < 0)
		printk(KERN_ERR "%s: i2c master send error\n",
		       "aw22xxx_i2c_writes");

	kfree(data);

	return ret;
}

/*
 * aw22xxx_led_init() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static inline void aw22xxx_led_init(struct aw22xxx_chip *chip)
{
	u8 reg_val;
	u8 reg_val1;
	u8 imax;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_led_init");

	aw22xxx_i2c_write(chip, 0x01, 0x55);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x02, &reg_val);
	aw22xxx_i2c_write(chip, 0x02, reg_val | 0x01);
	msleep(2);

	imax = aw22xxx_imax_clamp_table[chip->imax];
	if (imax > 0x0f)
		imax = 0x0f;
	aw22xxx_i2c_write(chip, 0xff, 0x00);
	aw22xxx_i2c_write(chip, 0x0b, imax);

	aw22xxx_i2c_read(chip, 0x02, &reg_val1);
	aw22xxx_i2c_write(chip, 0x02, reg_val1 & (~0x01));
	msleep(2);

	printk(KERN_INFO "%s: exit\n", "aw22xxx_led_init");
}

/*
 * aw22xxx_container_update -- inlined into aw22xxx_fw_loaded.
 * ffffff80087a1714..ffffff80087a1bec.
 *
 * It programs the chip's internal flash with the container contents and then
 * re-reads the "bist" to verify it. The write loop sends blocks of at
 * most 128 bytes ("cmp w8,#0x80; csel w25,w8,w28,cc" with w28 = 0x80,
 * @ffffff80087a19cc/19d0) to the current address, written into registers 0x22
 * (high byte) and 0x21 (low byte).
 *
 * The outcome ends up in chip->fw_update: 2 if the bist matches or if it is not
 * finished yet, 3 if it does not match ("orr w8,wzr,#0x2"@ffffff80087a1bbc,
 * "orr w8,wzr,#0x3"@ffffff80087a1bec, "strb w8,[x19,#587]"
 * @ffffff80087a1bf4).
 *
 * Strings:
 *   "aw22xxx_container_update"@0xffffff80091c7a10
 *   KERN_ERR "%s: bist check is running, reg0x24=0x%02x\n"@0xffffff80091c79b0
 *   KERN_INFO "%s: bist check pass, bist=0x%04x\n"@0xffffff80091c78ff
 *   KERN_ERR "%s: bist check fail, bist=0x%04x\n"@0xffffff80091c7a29
 *   KERN_ERR "%s: fw update failed, please reset phone\n"@0xffffff80091c7a4d
 */
static void aw22xxx_container_update(struct aw22xxx_chip *chip,
				      struct aw22xxx_container *aw22xxx_fw)
{
	unsigned int i;
	unsigned int len;
	u8 reg_val;
	u8 reg_val1;
	u8 reg_val2;
	u8 reg_val3;
	u8 reg_val4;
	unsigned int flash_bist;

	aw22xxx_i2c_write(chip, 0x24, 0x00);
	aw22xxx_i2c_write(chip, 0xff, 0x00);
	aw22xxx_i2c_write(chip, 0x01, 0x55);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x02, &reg_val);
	aw22xxx_i2c_write(chip, 0x02, reg_val | 0x01);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x04, &reg_val1);
	aw22xxx_i2c_write(chip, 0x04, reg_val1 | 0x01);

	aw22xxx_i2c_write(chip, 0x80, 0xec);
	aw22xxx_i2c_write(chip, 0x35, 0x29);
	aw22xxx_i2c_write(chip, 0x38, aw22xxx_fw->key);

	aw22xxx_i2c_write(chip, 0x22, 0x00);
	aw22xxx_i2c_write(chip, 0x21, 0x00);
	aw22xxx_i2c_write(chip, 0x20, 0x03);
	aw22xxx_i2c_write(chip, 0x30, 0x03);
	aw22xxx_i2c_write(chip, 0x23, 0x00);
	msleep(40);
	aw22xxx_i2c_write(chip, 0x30, 0x00);

	aw22xxx_i2c_write(chip, 0x22, 0x40);
	aw22xxx_i2c_write(chip, 0x21, 0x00);
	aw22xxx_i2c_write(chip, 0x30, 0x02);
	aw22xxx_i2c_write(chip, 0x23, 0x00);
	msleep(6);
	aw22xxx_i2c_write(chip, 0x30, 0x00);

	aw22xxx_i2c_write(chip, 0x22, 0x42);
	aw22xxx_i2c_write(chip, 0x21, 0x00);
	aw22xxx_i2c_write(chip, 0x30, 0x02);
	aw22xxx_i2c_write(chip, 0x23, 0x00);
	msleep(6);
	aw22xxx_i2c_write(chip, 0x30, 0x00);

	aw22xxx_i2c_write(chip, 0x22, 0x44);
	aw22xxx_i2c_write(chip, 0x21, 0x00);
	aw22xxx_i2c_write(chip, 0x30, 0x02);
	aw22xxx_i2c_write(chip, 0x23, 0x00);
	msleep(6);
	aw22xxx_i2c_write(chip, 0x30, 0x00);

	aw22xxx_i2c_write(chip, 0x20, 0x00);
	aw22xxx_i2c_write(chip, 0x20, 0x03);

	for (i = 0; i < aw22xxx_fw->len; i += len) {
		aw22xxx_i2c_write(chip, 0x22, i >> 8);
		aw22xxx_i2c_write(chip, 0x21, i & 0xff);
		aw22xxx_i2c_write(chip, 0x11, 0x01);
		aw22xxx_i2c_write(chip, 0x30, 0x04);

		len = aw22xxx_fw->len - i;
		if (len > 128)
			len = 128;
		aw22xxx_i2c_writes(chip, 0x23, &aw22xxx_fw->data[i], len);

		aw22xxx_i2c_write(chip, 0x11, 0x00);
		aw22xxx_i2c_write(chip, 0x30, 0x00);
	}

	aw22xxx_i2c_write(chip, 0x20, 0x00);

	aw22xxx_i2c_write(chip, 0x01, 0x55);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x02, &reg_val2);
	aw22xxx_i2c_write(chip, 0x02, reg_val2 | 0x01);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x04, &reg_val3);
	aw22xxx_i2c_write(chip, 0x04, reg_val3 | 0x01);

	aw22xxx_i2c_write(chip, 0x22, (aw22xxx_fw->len - 1) >> 8);
	aw22xxx_i2c_write(chip, 0x21, (aw22xxx_fw->len - 1) & 0xff);
	aw22xxx_i2c_write(chip, 0x24, 0x07);
	msleep(5);

	aw22xxx_i2c_read(chip, 0x24, &reg_val4);
	if (reg_val4 != 0x05) {
		printk(KERN_ERR "%s: bist check is running, reg0x24=0x%02x\n",
		       "aw22xxx_container_update", reg_val4);
		aw22xxx_i2c_write(chip, 0x24, 0x00);
		chip->fw_update = 2;
		return;
	}

	aw22xxx_i2c_read(chip, 0x25, &reg_val4);
	flash_bist = reg_val4;
	aw22xxx_i2c_read(chip, 0x26, &reg_val4);
	flash_bist |= reg_val4 << 8;

	if (flash_bist != aw22xxx_fw->bist) {
		printk(KERN_ERR "%s: bist check fail, bist=0x%04x\n",
		       "aw22xxx_container_update", flash_bist);
		printk(KERN_ERR "%s: fw update failed, please reset phone\n",
		       "aw22xxx_container_update");
		chip->fw_update = 3;
		return;
	}

	printk(KERN_INFO "%s: bist check pass, bist=0x%04x\n",
	       "aw22xxx_container_update", flash_bist);
	aw22xxx_i2c_write(chip, 0x24, 0x00);
	chip->fw_update = 2;
}

/*
 * aw22xxx_fw_loaded() was reconstructed from the factory kernel disassembly (2820 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw22xxx_fw_loaded(const struct firmware *cont, void *context)
{
	struct aw22xxx_chip *chip = context;
	struct aw22xxx_container *aw22xxx_fw;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff80087a1354).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	unsigned char temp_buf[32] = { 0 };
	unsigned short check_sum = 0;
	u8 reg_val;
	u8 reg_val1;
	u8 reg_val2;
	unsigned int flash_bist;
	int i;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_fw_loaded");

	if (!cont) {
		printk(KERN_ERR "%s: failed to read %s\n", "aw22xxx_fw_loaded",
		       "aw22xxx_fw.bin");
		release_firmware(cont);
		return;
	}

	printk(KERN_INFO "%s: loaded %s - size: %zu\n", "aw22xxx_fw_loaded",
	       "aw22xxx_fw.bin", cont->size);

	for (i = 2; i < cont->size; i++)
		check_sum += cont->data[i];

	if (check_sum != ((cont->data[0] << 8) | cont->data[1])) {
		printk(KERN_ERR "%s: check sum err: check_sum=0x%04x\n",
		       "aw22xxx_fw_loaded", check_sum);
		release_firmware(cont);
		return;
	}

	printk(KERN_INFO "%s: check sum pass : 0x%04x\n", "aw22xxx_fw_loaded",
	       check_sum);

	aw22xxx_fw = kzalloc(cont->size + sizeof(struct aw22xxx_container),
			      GFP_KERNEL);
	if (!aw22xxx_fw) {
		release_firmware(cont);
		printk(KERN_ERR "%s: Error allocating memory\n",
		       "aw22xxx_fw_loaded");
		return;
	}

	printk(KERN_INFO "%s: fw chip_id : 0x%02x\n", "aw22xxx_fw_loaded",
	       cont->data[2]);

	memcpy(temp_buf, &cont->data[3], 16);
	printk(KERN_INFO "%s: fw customer: %s\n", "aw22xxx_fw_loaded",
	       temp_buf);

	memcpy(temp_buf, &cont->data[19], 8);
	printk(KERN_INFO "%s: fw project: %s\n", "aw22xxx_fw_loaded", temp_buf);

	aw22xxx_fw->version = (cont->data[27] << 24) | (cont->data[28] << 16) |
			       (cont->data[29] << 8) | cont->data[30];
	printk(KERN_INFO "%s: fw version : 0x%04x\n", "aw22xxx_fw_loaded",
	       aw22xxx_fw->version);

	aw22xxx_fw->bist = (cont->data[34] << 8) | cont->data[35];
	printk(KERN_INFO "%s: fw bist : 0x%04x\n", "aw22xxx_fw_loaded",
	       aw22xxx_fw->bist);

	aw22xxx_fw->key = cont->data[36];
	printk(KERN_INFO "%s: fw key : 0x%04x\n", "aw22xxx_fw_loaded",
	       aw22xxx_fw->key);

	aw22xxx_fw->len = (cont->data[38] << 8) | cont->data[39];
	printk(KERN_INFO "%s: fw len : 0x%04x\n", "aw22xxx_fw_loaded",
	       aw22xxx_fw->len);

	memcpy(aw22xxx_fw->data, &cont->data[40], aw22xxx_fw->len);
	release_firmware(cont);

	aw22xxx_i2c_write(chip, 0x01, 0x55);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x02, &reg_val1);
	aw22xxx_i2c_write(chip, 0x02, reg_val1 | 0x01);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x04, &reg_val2);
	aw22xxx_i2c_write(chip, 0x04, reg_val2 | 0x01);

	aw22xxx_i2c_write(chip, 0x22, (aw22xxx_fw->len - 1) >> 8);
	aw22xxx_i2c_write(chip, 0x21, (aw22xxx_fw->len - 1) & 0xff);
	aw22xxx_i2c_write(chip, 0x24, 0x07);
	msleep(5);

	aw22xxx_i2c_read(chip, 0x24, &reg_val);
	if (reg_val == 0x05) {
		aw22xxx_i2c_read(chip, 0x25, &reg_val);
		flash_bist = reg_val;
		aw22xxx_i2c_read(chip, 0x26, &reg_val);
		flash_bist |= reg_val << 8;

		if (flash_bist == aw22xxx_fw->bist) {
			printk(KERN_INFO "%s: bist check pass, bist=0x%04x\n",
			       "aw22xxx_fw_loaded", flash_bist);
			if (!chip->fw) {
				kfree(aw22xxx_fw);
				aw22xxx_i2c_write(chip, 0x24, 0x00);
				aw22xxx_led_init(chip);
				chip->fw_update = 2;
				return;
			}
			printk(KERN_INFO
			       "%s: fw version: 0x%04x, force update fw\n",
			       "aw22xxx_fw_loaded", aw22xxx_fw->version);
		} else {
			printk(KERN_INFO
			       "%s: bist check fail, fw bist=0x%04x, flash bist=0x%04x\n",
			       "aw22xxx_fw_loaded", aw22xxx_fw->bist,
			       flash_bist);
			printk(KERN_INFO
			       "%s: find new fw: 0x%04x, need update\n",
			       "aw22xxx_fw_loaded", aw22xxx_fw->version);
		}
	} else {
		printk(KERN_ERR "%s: bist check is running, reg0x24=0x%02x\n",
		       "aw22xxx_fw_loaded", reg_val);
		printk(KERN_INFO "%s: fw need update\n", "aw22xxx_fw_loaded");
	}

	aw22xxx_container_update(chip, aw22xxx_fw);
	kfree(aw22xxx_fw);

	chip->fw = 0;
	aw22xxx_led_init(chip);

	printk(KERN_INFO "%s: exit\n", "aw22xxx_fw_loaded");
}

/*
 * aw22xxx_cfg_loaded() was reconstructed from the factory kernel disassembly (640 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw22xxx_cfg_loaded(const struct firmware *cont, void *context)
{
	struct aw22xxx_chip *chip = context;
	unsigned char flag = 0;
	unsigned char reg;
	unsigned char val;
	int i;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_cfg_loaded");

	if (!cont) {
		printk(KERN_ERR "%s: failed to read %s\n", "aw22xxx_cfg_loaded",
		       aw22xxx_cfg_names[chip->effect]);
		release_firmware(cont);
		return;
	}

	printk(KERN_INFO "%s: loaded %s - size: %zu\n", "aw22xxx_cfg_loaded",
	       aw22xxx_cfg_names[chip->effect], cont->size);

	for (i = 0; i < cont->size; i += 2) {
		if (cont->data[i] == 0xff)
			flag = *(cont->data + i + 1);

		if (chip->cfg == 1) {
			aw22xxx_i2c_write(chip, cont->data[i],
					   *(cont->data + i + 1));
			pr_debug("%s: addr:0x%02x, data:0x%02x\n",
				 "aw22xxx_cfg_loaded", cont->data[i],
				 *(cont->data + i + 1));
		} else if (flag == 1) {
			if (cont->data[i] >= 0x10 && cont->data[i] <= 0x2a) {
				reg = cont->data[i] - 0x10;
				val = chip->rgb[reg / 3] >>
				      (16 - (reg % 3) * 8);
				aw22xxx_i2c_write(chip, cont->data[i], val);
				pr_debug("%s: addr:0x%02x, data:0x%02x\n",
					 "aw22xxx_cfg_loaded", cont->data[i],
					 val);
			} else {
				aw22xxx_i2c_write(chip, cont->data[i],
						   *(cont->data + i + 1));
				pr_debug("%s: addr:0x%02x, data:0x%02x\n",
					 "aw22xxx_cfg_loaded", cont->data[i],
					 *(cont->data + i + 1));
			}
		} else {
			aw22xxx_i2c_write(chip, cont->data[i],
					   *(cont->data + i + 1));
			pr_debug("%s: addr:0x%02x, data:0x%02x\n",
				 "aw22xxx_cfg_loaded", cont->data[i],
				 *(cont->data + i + 1));
		}

		if (flag == 0) {
			if (cont->data[i] == 0x02 &&
			    (*(cont->data + i + 1) & 0x01))
				msleep(2);
		}
	}

	release_firmware(cont);

	printk(KERN_INFO "%s: cfg update complete\n", "aw22xxx_cfg_loaded");

	mutex_unlock(&chip->cfg_lock);
}

/*
 * DEVICE_ATTR() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static DEVICE_ATTR(reg, 0644, aw22xxx_reg_show, aw22xxx_reg_store);
static DEVICE_ATTR(hwen, 0644, aw22xxx_hwen_show, aw22xxx_hwen_store);
static DEVICE_ATTR(fw, 0644, aw22xxx_fw_show, aw22xxx_fw_store);
static DEVICE_ATTR(cfg, 0644, aw22xxx_cfg_show, aw22xxx_cfg_store);
static DEVICE_ATTR(effect, 0644, aw22xxx_effect_show, aw22xxx_effect_store);
static DEVICE_ATTR(imax, 0644, aw22xxx_imax_show, aw22xxx_imax_store);
static DEVICE_ATTR(rgb, 0644, aw22xxx_rgb_show, aw22xxx_rgb_store);
static DEVICE_ATTR(task0, 0644, aw22xxx_task0_show, aw22xxx_task0_store);
static DEVICE_ATTR(task1, 0644, aw22xxx_task1_show, aw22xxx_task1_store);
static DEVICE_ATTR(ledeffect, 0644, aw22xxx_ledeffect_show, aw22xxx_ledeffect_store);

static struct attribute *aw22xxx_attributes[] = {
	&dev_attr_reg.attr,
	&dev_attr_hwen.attr,
	&dev_attr_fw.attr,
	&dev_attr_cfg.attr,
	&dev_attr_effect.attr,
	&dev_attr_imax.attr,
	&dev_attr_rgb.attr,
	&dev_attr_task0.attr,
	&dev_attr_task1.attr,
	&dev_attr_ledeffect.attr,
	NULL,
};

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const struct attribute_group aw22xxx_attribute_group = {
	.attrs = aw22xxx_attributes,
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009932218).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const struct of_device_id aw22xxx_dt_match[] = {
	{ .compatible = "awinic,aw22xxx_led" },
	{ },
};

static const struct i2c_device_id aw22xxx_i2c_id[] = {
	{ "aw22xxx_led", 0 },
	{ },
};

static struct i2c_driver aw22xxx_i2c_driver = {
	.driver = {
		.name = "aw22xxx_led",
		.of_match_table = aw22xxx_dt_match,
	},
	.probe = aw22xxx_i2c_probe,
	.remove = aw22xxx_i2c_remove,
	.id_table = aw22xxx_i2c_id,
};

/*
 * aw22xxx_i2c_init() was reconstructed from the factory kernel disassembly (0xffffff800941fe18, 84 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_leds_mt6771_aw22127_leds-aw22xxx.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int __init aw22xxx_i2c_init(void)
{
	int ret;

	printk(KERN_INFO "aw22xxx driver version %s\n", "v1.1.3");

	ret = i2c_add_driver(&aw22xxx_i2c_driver);
	if (ret)
		printk(KERN_ERR "fail to add aw22xxx device into i2c\n");

	return ret;
}
late_initcall(aw22xxx_i2c_init);

static void __exit aw22xxx_i2c_exit(void)
{
	i2c_del_driver(&aw22xxx_i2c_driver);
}

module_exit(aw22xxx_i2c_exit);
