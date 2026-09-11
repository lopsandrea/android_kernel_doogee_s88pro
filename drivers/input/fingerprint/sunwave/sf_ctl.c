// SPDX-License-Identifier: GPL-2.0
/*
 * Sunwave/TrustKernel SPI fingerprint reader, Doogee S88 Pro -- the control
 * side: probe/remove, the misc node, ioctl, sysfs, input.
 *
 * Two translation units, and that is not a stylistic choice: it mirrors what
 * the factory binary shows. Reconstructed from the disassembly of the factory
 * kernel; every constant carries the disassembly line it was read from -- a
 * value without one is a defect.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/fb.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/pm_wakeup.h>
#include <linux/spi/spi.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>

/*
 * The same situation: `ffffff80087adb34 T tee_spi_transfer` in the oracle, the
 * source is in the tree
 * (drivers/misc/mediatek/tkcore/core/tee_fp.c:90, EXPORT_SYMBOL at line 166)
 * but no header in include/ declares it.
 */
extern int tee_spi_transfer(void *conf, uint32_t conf_size,
			    void *inbuf, void *outbuf, uint32_t size);

#include "sunwave.h"

static long sf_ctl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static long sf_ctl_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static ssize_t sf_show_version(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t sunwave_chip_info_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t sunwave_version_show(struct device *dev, struct device_attribute *attr, char *buf);
static int sf_probe(struct spi_device *spi);
static int sf_remove(struct spi_device *spi);
static void sf_ctl_device_event(struct work_struct *ws);
static int sf_fb_notifier_callback(struct notifier_block *self,
				   unsigned long event, void *data);
static irqreturn_t sf_ctl_device_irq(int irq, void *dev_id);

/*
 * DEVICE_ATTR() was reconstructed from the factory kernel disassembly (0xffffff800998dde0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static DEVICE_ATTR(tee_version, 0644, sf_show_version, NULL);
static DEVICE_ATTR(chip_info, 0644, sunwave_chip_info_show, NULL);
static DEVICE_ATTR(version, 0644, sunwave_version_show, NULL);

static struct attribute *sf_ctl_attrs[] = {
	&dev_attr_tee_version.attr,
	&dev_attr_chip_info.attr,
	&dev_attr_version.attr,
	NULL,
};

static struct attribute_group sf_ctl_attr_group = {
	.attrs = sf_ctl_attrs,
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998de08, 128 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct file_operations sf_ctl_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = sf_ctl_ioctl,
	.compat_ioctl = sf_ctl_compat_ioctl,
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998dbf0, 416 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct sf_ctl_device sf_ctl_dev = {
	.miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "sunwave_fp",
		.fops = &sf_ctl_fops,
	},
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a100b0c, 352 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct sf_tee_version {
	char solution[0x20];		/* 0x00 */
	char ca[0x60 - 0x20];		/* 0x20 */
	char ta[0xa0 - 0x60];		/* 0x60 */
	char alg[0xc0 - 0xa0];		/* 0xa0 */
	char nav[0xe0 - 0xc0];		/* 0xc0 */
	char driver[0x100 - 0xe0];	/* 0xe0 */
	char firmware[0x120 - 0x100];	/* 0x100 */
	char sensor[0x140 - 0x120];	/* 0x120 */
	char vendor[0x160 - 0x140];	/* 0x140 */
};

/*
 * This section was reconstructed from the factory kernel disassembly (32 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct sf_tee_version g_tee_version;

/*
 * A 32-byte buffer at 0xffffff800a100c6c. TWO functions write it with the
 * same literal and the same length:
 *   sf_ctl_driver_init  "9131b294  add x20, x20, #0xc6c" +
 *                       "911f8c21  add x1, x1, #0x7e3" +
 *                       "321b03e2  orr w2, wzr, #0x20" + bl <strncpy> +
 *                       "39007e9f  strb wzr, [x20,#31]"
 *   sf_ctl_ioctl        "9131b000  add x0, x0, #0xc6c" + the same three lines +
 *                       "39007c1f  strb wzr, [x0,#31]"
 * with "v2.2.36-2018-11-14"@0x92507e3. sf_ctl_ioctl then copies it to user
 * space with "321b03e2 orr w2, wzr, #0x20" + bl <__arch_copy_to_user>: 32 bytes.
 */
static char g_driver_version[32];

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998def8, 80 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static u32 sf_spi_conf[20] = {
	10, 10, 15, 15, 10, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 5, 0, 0, 0,
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998d998, 200 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct of_device_id sf_of_match[] = {
	{ .compatible = "mediatek,spi-fingerprint", },
	{ .compatible = "mediatek,goodix-fp", },
	{ .compatible = "goodix,goodix-fp", },
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998d900).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct spi_driver sf_spi_driver = {
	.driver = {
		.name = "sunwave-fp",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
		.of_match_table = sf_of_match,
	},
	.probe = sf_probe,
	.remove = sf_remove,
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009411a38, 72 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct spi_board_info sf_spi_board_info[] __initdata = {
	{
		.modalias = "sunwave-fp",
		.bus_num = 0,
		.chip_select = 2,
		.mode = 0,
	},
};

/*
 * sf_read_sensor_id() was reconstructed from the factory kernel disassembly (2048 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int sf_read_sensor_id(void);
static int sf_ctl_init_input(struct sf_ctl_device *ctl);
static int sf_ctl_init_irq(struct sf_ctl_device *ctl);

static int sf_probe(struct spi_device *spi)
{
	struct sf_ctl_device *ctl = &sf_ctl_dev;
	int err;

	printk(KERN_INFO "sunwave-sf_ctl-%d: sunwave %s enter\n", 653, "sf_probe");
	ctl->spi = spi;
	spi->mode = 0;
	spi->bits_per_word = 8;
	spi->max_speed_hz = 1000000;
	spi_setup(spi);

	err = sf_platform_init(ctl);
	if (err) {
		ctl->spi = NULL;
		printk(KERN_ERR "sunwave-sf_ctl-%d: sf_platform_init failed with %d.\n", 670, err);
		return err;
	}

	wakeup_source_prepare(&ctl->ws, "sf_wakelock");
	wakeup_source_add(&ctl->ws);

	/* "f9404aa8  ldr x8, [x21,#144]" + "d63f0100  blr x8", x0 = base. */
	err = ctl->init_gpio_pins(ctl);
	if (err) {
		printk(KERN_ERR "sunwave-sf_ctl-%d: gpio_init failed with %d.\n", 686, err);
		ctl->free_gpio(ctl);
		sf_platform_exit(ctl);
		ctl->spi = NULL;
		return err;
	}

	if (sf_read_sensor_id()) {
		printk(KERN_ERR "sunwave-sf_ctl-%d: sunwave probe read chip id is failed\n", 732);
		ctl->free_gpio(ctl);
		sf_platform_exit(ctl);
		ctl->spi = NULL;
		/* "12800014  mov w20, #0xffffffff": -1, not an -Exxx. */
		return -1;
	}

	err = sf_ctl_init_input(ctl);
	if (err) {
		ctl->free_gpio(ctl);
		sf_platform_exit(ctl);
		ctl->spi = NULL;
		printk(KERN_ERR "sunwave-sf_ctl-%d: sf_ctl_init_input failed with %d.\n", 761, err);
		return err;
	}

	err = misc_register(&ctl->miscdev);
	if (err) {
		ctl->free_gpio(ctl);
		sf_platform_exit(ctl);
		printk(KERN_ERR "sunwave-sf_ctl-%d: misc_register(..) = %d.\n", 771, err);
		input_unregister_device(ctl->input);
		ctl->spi = NULL;
		return err;
	}

	/*
	 * "f9400288  ldr x8, [x20]" with x20 = 0x998dc20 = base+48
	 * (miscdev.this_device), "91004100  add x0, x8, #0x10" (&dev->kobj,
	 * offset 0x10 of struct device on this sublevel) and
	 * "91378021  add x1, x1, #0xde0" = 0x998dde0 = the attribute_group.
	 * The outcome is not looked at: w0 is overwritten right after.
	 */
	sysfs_create_group(&ctl->miscdev.this_device->kobj, &sf_ctl_attr_group);
	INIT_WORK(&ctl->work, sf_ctl_device_event);
	g_probe_done = 1;

	err = sf_ctl_init_irq(ctl);
	if (err) {
		printk(KERN_ERR "sunwave-sf_ctl-%d: sf_ctl_init_irq failed with %d.\n", 790, err);
		input_unregister_device(ctl->input);
		misc_deregister(&ctl->miscdev);
		ctl->free_gpio(ctl);
		sf_platform_exit(ctl);
		ctl->spi = NULL;
		return err;
	}

	ctl->notifier.notifier_call = sf_fb_notifier_callback;
	fb_register_client(&ctl->notifier);
	printk(KERN_ERR "sunwave-sf_ctl-%d: %s leave\n", 831, "sf_probe");
	return err;
}

/*
 * sf_read_sensor_id() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int sf_read_sensor_id(void)
{
	struct sf_ctl_device *ctl = &sf_ctl_dev;
	u8 tx[16];
	u8 rx[16];
	int ret = -1;
	int retry;
	int i = 0;

	/* "f9400688  ldr x8, [x20,#8]" with x20 = base+168, that is base+176. */
	ctl->reset();
	/* "f9400288  ldr x8, [x20]" + "320003e0 orr w0, wzr, #0x1". */
	ctl->spi_clock_enable(true);

	sf_spi_conf[2] = 60;
	sf_spi_conf[3] = 60;
	sf_spi_conf[8] = 0;
	sf_spi_conf[9] = 0;
	sf_spi_conf[14] = 0;

	ctl->spi->max_speed_hz = 1000000;
	ctl->spi->bits_per_word = 8;
	ctl->spi->mode = 0;
	spi_setup(ctl->spi);

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) enter.\n", 1001, "sf_read_sensor_id");
	msleep(10);

	retry = 3;
	do {
		memset(tx, 0, sizeof(tx));
		memset(rx, 0, sizeof(rx));
		tx[0] = 0xa0;
		tx[1] = 0x5f;
		if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 6)) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed\n", 1013);
			continue;
		}
		if (rx[2] == 'S' && rx[3] == 'u' && rx[4] == 'n' && rx[5] == 'W') {
			printk(KERN_INFO "sunwave-sf_ctl-%d: read chip is ok\n", 1019);
			ret = 0;
			goto done;
		}

		memset(tx, 0, sizeof(tx));
		memset(rx, 0, sizeof(rx));
		tx[0] = 0x60;
		tx[1] = 0x9f;
		tx[2] = 0x28;
		tx[3] = 0x02;
		tx[4] = 0x00;
		if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 7)) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed\n", 1034);
			continue;
		}
		if (rx[5] == 0x82) {
			printk(KERN_INFO "sunwave-sf_ctl-%d: read chip is ok\n", 1039);
			ret = 0;
			goto done;
		}

		memset(tx, 0, sizeof(tx));
		memset(rx, 0, sizeof(rx));
		tx[0] = 0x60;
		tx[1] = 0x28;
		tx[2] = 0x02;
		if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 6)) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed\n", 1053);
			continue;
		}
		if (rx[4] == 0x82) {
			printk(KERN_INFO "sunwave-sf_ctl-%d: read chip is ok\n", 1058);
			ret = 0;
			goto done;
		}

		do {
			/* "f94652a8  ldr x8, [x21,#3232]" = 0x998dca0 = base+176. */
			ctl->reset();
			msleep(2 * i + 1);

			memset(tx, 0, sizeof(tx));
			memset(rx, 0, sizeof(rx));
			tx[0] = 0x32;
			tx[1] = 0xcd;
			tx[4] = 0x84;
			if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 5)) {
				printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed step 1\n", 1081);
				continue;
			}
			msleep(1);

			memset(tx, 0, sizeof(tx));
			memset(rx, 0, sizeof(rx));
			tx[0] = 0x32;
			tx[1] = 0xcd;
			tx[4] = 0x81;
			if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 5)) {
				printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed step 2\n", 1096);
				continue;
			}
			msleep(5);

			memset(tx, 0, sizeof(tx));
			memset(rx, 0, sizeof(rx));
			tx[0] = 0x81;
			tx[1] = 0x7e;
			tx[4] = 0x21;
			if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 10)) {
				printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed step 3\n", 1114);
				continue;
			}
			if (rx[8] == 0x82) {
				printk(KERN_INFO "sunwave-sf_ctl-%d: read chip is ok: 0x%02x%02x\n",
				       1119, rx[8], rx[9]);
				memset(tx, 0, sizeof(tx));
				memset(rx, 0, sizeof(rx));
				tx[0] = 0x32;
				tx[1] = 0xcd;
				tx[4] = 0x83;
				tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 5);
				ret = 0;
				goto done;
			}
			printk(KERN_ERR "sunwave-sf_ctl-%d: read chip: 0x%02x%02x\n",
			       1131, rx[8], rx[9]);
		} while (i++ < 10);
	} while (retry-- != 0);

done:
	/* "f9464d08 ldr x8,[x8,#3224]" + "2a1f03e0 mov w0, wzr": on = false. */
	ctl->spi_clock_enable(false);
	return ret;
}

/*
 * sf_ctl_init_input() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int sf_ctl_init_input(struct sf_ctl_device *ctl)
{
	int err;

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) enter.\n", 1173, "sf_ctl_init_input");
	ctl->input = input_allocate_device();
	if (!ctl->input) {
		printk(KERN_ERR "sunwave-sf_ctl-%d: input_allocate_device(..) failed.\n", 1177);
		return -ENOMEM;
	}

	ctl->input->name = "sf-keys";
	__set_bit(EV_KEY, ctl->input->evbit);
	__set_bit(KEY_HOME, ctl->input->keybit);
	__set_bit(KEY_MENU, ctl->input->keybit);
	__set_bit(KEY_BACK, ctl->input->keybit);
	__set_bit(KEY_F10, ctl->input->keybit);
	__set_bit(KEY_ENTER, ctl->input->keybit);
	__set_bit(KEY_UP, ctl->input->keybit);
	__set_bit(KEY_LEFT, ctl->input->keybit);
	__set_bit(KEY_RIGHT, ctl->input->keybit);
	__set_bit(KEY_DOWN, ctl->input->keybit);
	__set_bit(KEY_WAKEUP, ctl->input->keybit);

	err = input_register_device(ctl->input);
	if (err) {
		printk(KERN_ERR "sunwave-sf_ctl-%d: input_register_device(..) = %d.\n", 1196, err);
		input_free_device(ctl->input);
		ctl->input = NULL;
		return -ENODEV;
	}
	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) leave.\n", 1202, "sf_ctl_init_input");
	return 0;
}

/*
 * sf_ctl_init_irq() was reconstructed from the factory kernel disassembly (0xffffff8008a83920).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int sf_ctl_init_irq(struct sf_ctl_device *ctl)
{
	int err;

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) enter.\n", 1156, "sf_ctl_init_irq");
	err = request_irq(ctl->irq, sf_ctl_device_irq, IRQF_TRIGGER_FALLING, "sf-irq", NULL);
	if (err)
		printk(KERN_ERR "sunwave-sf_ctl-%d: request_irq(..) = %d.\n", 1162, err);
	irq_set_irq_wake(ctl->irq, 1);
	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) leave.\n", 1166, "sf_ctl_init_irq");
	return err;
}

/*
 * sf_remove() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int sf_remove(struct spi_device *spi)
{
	if (sf_ctl_dev.spi && sf_ctl_dev.init_gpio_pins) {
		fb_unregister_client(&sf_ctl_dev.notifier);
		if (sf_ctl_dev.input)
			input_unregister_device(sf_ctl_dev.input);
		/* "37f800c0  tbnz w0, #31, ...+0x60": skips se irq < 0. */
		if (sf_ctl_dev.irq >= 0) {
			free_irq(sf_ctl_dev.irq, &sf_ctl_dev);
			sf_ctl_dev.irq = 0;
		}
		misc_deregister(&sf_ctl_dev.miscdev);
		wakeup_source_remove(&sf_ctl_dev.ws);
		wakeup_source_drop(&sf_ctl_dev.ws);
		/* "f9404e68  ldr x8, [x19,#152]" + "d63f0100  blr x8", x0 = base. */
		sf_ctl_dev.free_gpio(&sf_ctl_dev);
		sf_platform_exit(&sf_ctl_dev);
		sf_ctl_dev.spi = NULL;
	}
	return 0;
}

/*
 * Position 3. sf_ctl_device_event(struct work_struct *ws): the handler for the
 * work_struct sf_ctl_device_irq queues. It does not use the parameter and does
 * not return anything (no "mov w0,..." before the ret).
 *
 *   "sunwave-sf_ctl-%d: %s(..) enter.\n"@0x9250783   line 233
 *   "sf_ctl_device_event"@0x9250ab5                  (funcname)
 *
 * The environment array is a local, not a static: the disassembly copies its
 * 16 bytes from a read-only template onto the stack
 * ("a9402929  ldp x9, x10, [x9]" with x9 = 0xffffff8008f7f7a8, then
 * "a900abe9  stp x9, x10, [sp,#8]"). The template is, read with
 * tools/relocazioni.py:
 *   0xffffff8008f7f7a8: relocated, addend 0xffffff8009250aa4  -> "SPI_STATE=finger"@0x9250aa4
 *   0xffffff8008f7f7b0: not relocated                         -> NULL
 *
 * kobject_uevent_env receives x0 = *(0x998dc20) + 0x10: the field at
 * offset 48 is miscdev.this_device (struct device *), and in this
 * sublevel struct device has kobj at offset 0x10 (after parent and p).
 * KOBJ_CHANGE = 2 is "321f03e1  orr w1, wzr, #0x2".
 */
static void sf_ctl_device_event(struct work_struct *ws)
{
	char *envp[2] = { "SPI_STATE=finger", NULL };

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) enter.\n", 233, "sf_ctl_device_event");
	kobject_uevent_env(&sf_ctl_dev.miscdev.this_device->kobj, KOBJ_CHANGE, envp);
}

/*
 * sf_fb_notifier_callback() was reconstructed from the factory kernel disassembly (0xffffff8008f7f7b8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int sf_fb_notifier_callback(struct notifier_block *self,
				   unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int *blank;

	if (event == FB_EVENT_BLANK) {
		blank = evdata->data;
		if (*blank == FB_BLANK_POWERDOWN) {
			char *envp[2] = { "SCREEN_STATUS=OFF", NULL };

			kobject_uevent_env(&sf_ctl_dev.miscdev.this_device->kobj,
					   KOBJ_CHANGE, envp);
		} else if (*blank == FB_BLANK_UNBLANK) {
			char *envp[2] = { "SCREEN_STATUS=ON", NULL };

			kobject_uevent_env(&sf_ctl_dev.miscdev.this_device->kobj,
					   KOBJ_CHANGE, envp);
		}
	}
	return 0;
}

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct sf_key_event {
	int key;	/* "b94003e8  ldr w8, [sp]"    */
	int value;	/* "b94007e8  ldr w8, [sp,#4]" */
};

static int sf_ctl_report_key_event(struct input_dev *input, struct sf_key_event *kevent)
{
	int code;

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) enter.\n", 259, "sf_ctl_report_key_event");
	switch (kevent->key) {
	case 1:
		code = KEY_HOME;
		break;
	case 2:
		code = KEY_MENU;
		break;
	case 3:
		code = KEY_BACK;
		break;
	case 4:
		code = KEY_F10;
		break;
	case 5:
		code = KEY_ENTER;
		break;
	case 6:
		code = KEY_UP;
		break;
	case 7:
		code = KEY_LEFT;
		break;
	case 8:
		code = KEY_RIGHT;
		break;
	case 9:
		code = KEY_DOWN;
		break;
	case 10:
		code = KEY_WAKEUP;
		break;
	default:
		code = KEY_UNKNOWN;
		break;
	}
	input_report_key(input, code, kevent->value != 0);
	input_sync(input);
	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) leave.\n", 308, "sf_ctl_report_key_event");
	return 0;
}

/*
 * sf_ctl_ioctl() was reconstructed from the factory kernel disassembly (1268 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static long sf_ctl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct sf_ctl_device *ctl = &sf_ctl_dev;
	struct sf_key_event kevent;
	int err = 0;

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(_IO(type,nr) nr= 0x%08x, ..)\n",
	       327, "sf_ctl_ioctl", _IOC_NR(cmd));

	switch (cmd) {
	/* 0x00007302 and 0x00006b05: "f9465108 ldr x8,[x8,#3232]" + blr, no arguments. */
	case _IO('s', 0x02):
	case _IO('k', 0x05):
		ctl->reset();
		break;
	/* 0x00007305: "f9464d08 ldr x8,[x8,#3224]" + "320003e0 orr w0,wzr,#0x1" + blr. */
	case _IO('s', 0x05):
		ctl->spi_clock_enable(true);
		break;
	/* 0x00007306: same ldr, "2a1f03e0 mov w0, wzr". */
	case _IO('s', 0x06):
		ctl->spi_clock_enable(false);
		break;
	/*
	 * 0x0000730b e 0x40046b04: "f946c500 ldr x0,[x8,#3464]" (0x998dd88 =
	 * base+408 = ctl->spi), "b902d813 str w19,[x0,#728]" con x19 = arg,
	 * then "97fc1e88 bl <spi_setup>".
	 */
	case _IO('s', 0x0b):
	case _IOW('k', 0x04, u32):
		ctl->spi->max_speed_hz = arg;
		spi_setup(ctl->spi);
		break;
	/*
	 * 0x0000730d and 0x40046b11: an access check on 4 bytes
	 * ("b100114a adds x10, x10, #0x4"), "b94d818b ldr w11,[x12,#3456]"
	 * (0x998dd80 = base+400) and "b900014b str w11,[x10]". No printk
	 * on failure: "128001a8 mov w8, #0xfffffff2" = -14 = -EFAULT.
	 */
	case _IO('s', 0x0d):
	case _IOW('k', 0x11, u32):
		if (put_user(ctl->field_400, (int __user *)arg))
			err = -EFAULT;
		break;
	/*
	 * 0x40087309: a copy_from_user of 8 bytes ("321d03e2 orr w2, wzr, #0x8")
	 * onto the stack, then "f9463913 ldr x19,[x8,#3184]" = ctl->input and the
	 * inlined sf_ctl_report_key_event. The failure branch goes through
	 * "940f522b bl <__memset>" -- the uncopied tail cleared -- which is
	 * the signature of copy_from_user and not of __copy_from_user.
	 */
	case _IOW('s', 0x09, struct sf_key_event):
		if (copy_from_user(&kevent, (void __user *)arg, sizeof(kevent))) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: copy_from_user(..) failed.\n", 389);
			err = -EFAULT;
			break;
		}
		sf_ctl_report_key_event(ctl->input, &kevent);
		break;
	/*
	 * 0x80087320: an strncpy of the version literal into the 32-byte buffer
	 * at 0xa100c6c, the terminator forced with "39007c1f strb wzr,[x0,#31]",
	 * then an __arch_copy_to_user of 32 bytes towards arg.
	 */
	case _IOR('s', 0x20, unsigned long):
		strncpy(g_driver_version, "v2.2.36-2018-11-14", sizeof(g_driver_version));
		g_driver_version[sizeof(g_driver_version) - 1] = '\0';
		if (copy_to_user((void __user *)arg, g_driver_version, sizeof(g_driver_version))) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: copy_to_user(..) failed.\n", 424);
			err = -EFAULT;
		}
		break;
	/* 0x00007330: "52802c02 mov w2,#0x160" towards 0xa100b0c, with __memset at the tail. */
	case _IO('s', 0x30):
		if (copy_from_user(&g_tee_version, (void __user *)arg, sizeof(g_tee_version))) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: sf_hw_info_t copy_from_user(..) failed.\n", 434);
			err = -EFAULT;
		}
		break;
	/* 0x00007331: the same length, __arch_copy_to_user, without __memset. */
	case _IO('s', 0x31):
		if (copy_to_user((void __user *)arg, &g_tee_version, sizeof(g_tee_version))) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: sf_hw_info_t copy_to_user(..) failed.\n", 444);
			err = -EFAULT;
		}
		break;
	/*
	 * Nine commands with an empty body: the jump table (or the tree) sends
	 * them to the common exit at 0xa83704 where w8 is still zero, not to the
	 * -EINVAL branch at 0xa8360c. They are therefore real cases, not the default.
	 */
	case _IO('s', 0x00):
	case _IO('s', 0x01):
	case _IO('s', 0x03):
	case _IO('s', 0x04):
	case _IO('s', 0x07):
	case _IO('s', 0x08):
	case _IO('s', 0x21):
	case _IOR('k', 0x04, u32):
	case _IOWR('s', 0x0a, u64):
		break;
	/* "128002a8  mov w8, #0xffffffea" = -22 = -EINVAL. */
	default:
		err = -EINVAL;
		break;
	}
	return err;
}

/*
 * sf_ctl_compat_ioctl() was reconstructed from the factory kernel disassembly (24 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static long sf_ctl_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return sf_ctl_ioctl(filp, cmd, (u32)arg);
}

/*
 * sf_show_version() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t sf_show_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	int n = 0;

	n += sprintf(buf + n, "solution:%s\n", g_tee_version.solution);
	n += sprintf(buf + n, "ca      :%s\n", g_tee_version.ca);
	n += sprintf(buf + n, "ta      :%s\n", g_tee_version.ta);
	n += sprintf(buf + n, "alg     :%s\n", g_tee_version.alg);
	n += sprintf(buf + n, "nav     :%s\n", g_tee_version.nav);
	n += sprintf(buf + n, "driver  :%s\n", g_tee_version.driver);
	n += sprintf(buf + n, "firmware:%s\n", g_tee_version.firmware);
	n += sprintf(buf + n, "sensor  :%s\n", g_tee_version.sensor);
	n += sprintf(buf + n, "vendor  :%s\n", g_tee_version.vendor);
	return n;
}

/*
 * sunwave_chip_info_show() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t sunwave_chip_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "chip   : %s %s\nid     : 0x0 lib:%s\nvendor : fw:%s\nmore   : fingerprint\n",
		       g_tee_version.sensor, g_tee_version.ca,
		       g_tee_version.alg, g_tee_version.firmware);
}

/*
 * sunwave_version_show() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t sunwave_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", g_tee_version.driver);
}

/*
 * sf_ctl_device_irq() was reconstructed from the factory kernel disassembly (0xffffff800985e000).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static irqreturn_t sf_ctl_device_irq(int irq, void *dev_id)
{
	disable_irq_nosync(irq);
	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(irq = %d, ..) toggled.\n",
	       241, "sf_ctl_device_irq", irq);
	schedule_work(&sf_ctl_dev.work);
	__pm_wakeup_event(&sf_ctl_dev.ws, 1250);
	enable_irq(irq);
	return IRQ_HANDLED;
}

/*
 * sf_ctl_driver_init() was reconstructed from the factory kernel disassembly (0xffffff8008a840d4, 216 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int __init sf_ctl_driver_init(void)
{
	int err;

	printk(KERN_INFO "sunwave-sf_ctl-%d: '%s' SW_BUS_NAME = %s\n",
	       1209, "sf_ctl_driver_init", "spi bus");
	if (g_probe_done == 1)
		return 0;

	spi_register_board_info(sf_spi_board_info, ARRAY_SIZE(sf_spi_board_info));
	err = spi_register_driver(&sf_spi_driver);
	/* "36f800e0  tbz w0, #31, ...+0x84": it prints only if err < 0. */
	if (err < 0)
		printk(KERN_ERR "sunwave-sf_ctl-%d: %s, Failed to register SPI driver.\n",
		       1230, "sf_ctl_driver_init");

	printk(KERN_INFO "sunwave-sf_ctl-%d: sunwave fingerprint device control driver registered.\n", 1263);
	strncpy(g_driver_version, "v2.2.36-2018-11-14", sizeof(g_driver_version));
	g_driver_version[sizeof(g_driver_version) - 1] = '\0';
	printk(KERN_INFO "sunwave-sf_ctl-%d: driver version: '%s'.\n", 1264, g_driver_version);
	return err;
}
module_init(sf_ctl_driver_init);

/*
 * sf_ctl_driver_exit() was reconstructed from the factory kernel disassembly (0xffffff80093a8518, 44 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_ctl.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void __exit sf_ctl_driver_exit(void)
{
	spi_unregister_driver(&sf_spi_driver);
	printk(KERN_INFO "sunwave-sf_ctl-%d: sunwave fingerprint device control driver released.\n", 1282);
}
module_exit(sf_ctl_driver_exit);

MODULE_DESCRIPTION("Sunwave TrustKernel TEE fingerprint driver");
MODULE_LICENSE("GPL");
