// SPDX-License-Identifier: GPL-2.0
/*
 * usb_insert_detect -- USB insertion detection on the Doogee S88 Pro.
 *
 * Eight functions, running contiguously right after the end of the mt5725
 * block. Reconstructed from the disassembly of the factory kernel.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_usb_insert_detect_usb_insert_detect.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/wait.h>

#include <mt-plat/charger_class.h>

/*
 * `mt5725`'s, and only READ from here: 0xffffff800a110fec.
 * "b94feea8 ldr"@0xffffff8008acec60 and "b94feea8 ldr"@0xffffff8008acecb4.
 */
extern int mt5725_rvs_online;

extern struct charger_device *get_charger_by_name(const char *name);

/*
 * A HEADER DELTA, not done here by the rule on shared files: no
 * ALPS header declares `mt_charger_set_opa_mode`, which lives in
 * drivers/misc/mediatek/pmic/mt6370/ (see
 * patches/kernel-stock/mt6370-charger-set-opa-mode.patch). As
 * `mt5725.c` already does, it is declared LOCALLY.
 */
extern int mt_charger_set_opa_mode(struct charger_device *chg_dev, bool en);

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a110ff4).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_usb_insert_detect_usb_insert_detect.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int usb_insert_stato;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a111000).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_usb_insert_detect_usb_insert_detect.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct charger_device *usb_insert_detect_chg_dev;
static struct pinctrl *usb_insert_detect_pinctrl;
static struct pinctrl_state *usb_insert_detect_pin_init;
static struct device_node *usb_insert_detect_node;
static int usb_insert_detect_gpio;
static int usb_insert_detect_irq;
static int usb_insert_detect_flag;

/*
 * THE WAKE FLAG IS A BYTE, not an `int`, and the binary says so twice:
 * "394672e8 ldrb"@0xffffff8008acebb0 reads it as a byte and
 * "39067109 strb"@0xffffff8008aced30 writes it as a byte. The test is
 * "37000388 tbnz w8, #0", that is on bit 0 alone: it is a `bool`.
 */
static bool usb_insert_detect_thread_flag;

/*
 * 0xffffff80099a77d8, written to zero by the probe
 * ("b907d93f str"@0xffffff8008acead0) and by the kthread with the gpio value
 * ("b907db20 str"@0xffffff8008acec3c).
 */
static int usb_insert_detect_gpio_state;

static DECLARE_WAIT_QUEUE_HEAD(usb_insert_detect_waiter);

/*
 * usb_insert_online() was reconstructed from the factory kernel disassembly (0xffffff8008ace8f0, 20 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_usb_insert_detect_usb_insert_detect.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int usb_insert_online(void)
{
	return usb_insert_stato != 0;
}

/*
 * usb_insert_detect_show_debug @0xffffff8008aced50, 48 bytes.
 *   "321403e1 orr"@0xffffff8008aced6c   w1 = 0x1000 = PAGE_SIZE
 *   "b9419903 ldr"@0xffffff8008aced5c   the flag, at +408
 *   "93407c00 sxtw"@0xffffff8008aced74  the return value is sign-extended to 64
 *                                        bits: it is an `ssize_t`
 */
static ssize_t usb_insert_detect_show_debug(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", usb_insert_detect_flag);
}

/*
 * usb_insert_detect_store_debug() was reconstructed from the factory kernel disassembly (0xffffff8008aced80, 124 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_usb_insert_detect_usb_insert_detect.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t usb_insert_detect_store_debug(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t count)
{
	if (sscanf(buf, "%d", &usb_insert_detect_flag) == 0)
		printk("%s param get failed\n", "usb_insert_detect_store_debug");
	else
		pr_debug("usb_insert_detect_flag  = %d\n", usb_insert_detect_flag);

	return count;
}

static DEVICE_ATTR(usb_insert_detect_debug, 0664, usb_insert_detect_show_debug,
		   usb_insert_detect_store_debug);

/*
 * usb_insert_detect_eint_interrupt_handler() was reconstructed from the factory kernel disassembly (0xffffff8008aced00, 80 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_usb_insert_detect_usb_insert_detect.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static irqreturn_t usb_insert_detect_eint_interrupt_handler(int irq, void *dev_id)
{
	printk("[USB_INSERT_DETECT] eint interrupt\n");

	usb_insert_detect_thread_flag = true;
	wake_up(&usb_insert_detect_waiter);
	disable_irq_nosync(usb_insert_detect_irq);

	return IRQ_HANDLED;
}

/*
 * usb_insert_detect_thread_kthread() was reconstructed from the factory kernel disassembly (0xffffff8008aceb60, 416 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_usb_insert_detect_usb_insert_detect.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int usb_insert_detect_thread_kthread(void *x)
{
	while (1) {
		wait_event_interruptible(usb_insert_detect_waiter,
					 usb_insert_detect_thread_flag);
		usb_insert_detect_thread_flag = false;

		usb_insert_detect_gpio_state =
			gpio_get_value(usb_insert_detect_gpio);
		printk("[USB_INSERT_DETECT] gpio value=%d\n",
		       usb_insert_detect_gpio_state);

		if (usb_insert_detect_gpio_state == 1) {
			irq_set_irq_type(usb_insert_detect_irq,
					 IRQ_TYPE_LEVEL_LOW);
			usb_insert_detect_flag = 0;
			usb_insert_stato = 1;
			if (mt5725_rvs_online) {
				mt_charger_set_opa_mode(usb_insert_detect_chg_dev, 0);
				charger_dev_set_boost_current_limit(usb_insert_detect_chg_dev,
								    1500000);
				msleep(40);
				charger_dev_enable(usb_insert_detect_chg_dev, 1);
			}
			printk("[USB_INSERT_DETECT] usb_insert_detect_gpio_state = 1\n");
		} else {
			irq_set_irq_type(usb_insert_detect_irq,
					 IRQ_TYPE_LEVEL_HIGH);
			usb_insert_detect_flag = 1;
			usb_insert_stato = 0;
			if (mt5725_rvs_online) {
				charger_dev_enable(usb_insert_detect_chg_dev, 0);
				msleep(40);
				mt_charger_set_opa_mode(usb_insert_detect_chg_dev, 1);
				charger_dev_set_boost_current_limit(usb_insert_detect_chg_dev,
								    2200000);
			}
			printk("[USB_INSERT_DETECT] uninsert usb_insert_detect_gpio_state = 0\n");
		}

		enable_irq(usb_insert_detect_irq);
	}

	return 0;
}

/*
 * usb_insert_detect_probe @0xffffff8008ace904, 596 bytes.
 *
 * A FAILURE OF `device_create_file` DOES NOT STOP THE PROBE: after
 * "34000080 cbz"@0xffffff8008ace928 the two branches rejoin on the same
 * "97d992e5 bl"@0xffffff8008ace940 towards <printk> and carry on. It is a
 * factory defect, reproduced.
 *
 * "add x19, x0, #0x10"@0xffffff8008ace914 is `&pdev->dev`.
 */
static int usb_insert_detect_probe(struct platform_device *pdev)
{
	struct task_struct *thread;
	struct pinctrl_state *pin_init;
	struct pinctrl *pinctrl;
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_usb_insert_detect_debug);
	if (ret)
		printk("[usb_insert_detect]error creating sysfs files: usb_insert_detect_debug\n");
	else
		printk("[USB_INSERT_DETECT] dev attr usb_insert_detect_debug register success\n");

	thread = kthread_create(usb_insert_detect_thread_kthread, NULL,
				"usb_insert_detect_thread_kthread");
	if (IS_ERR(thread)) {
		printk("[USB_INSERT_DETECT] kthread create failed\n");
		return -1;
	}
	wake_up_process(thread);
	printk("[USB_INSERT_DETECT] kthread create success\n");

	usb_insert_detect_chg_dev = get_charger_by_name("primary_chg");
	if (!usb_insert_detect_chg_dev) {
		printk(KERN_NOTICE "%s: get primary charger device failed\n",
		       "usb_insert_detect_probe");
		return -1;
	}

	/*
	 * devm_pinctrl_get() was reconstructed from the factory kernel disassembly (0xffffff8008ace9ac).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_usb_insert_detect_usb_insert_detect.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	pinctrl = devm_pinctrl_get(&pdev->dev);
	usb_insert_detect_pinctrl = pinctrl;
	if (IS_ERR(pinctrl)) {
		printk("[USB_INSERT_DETECT] Cannot find usb_insert_detect pinctrl!\n");
		return PTR_ERR(pinctrl);
	}

	pin_init = pinctrl_lookup_state(pinctrl,
					"state_usb_insert_detect_gpio_init");
	usb_insert_detect_pin_init = pin_init;
	if (IS_ERR(pin_init)) {
		printk("[USB_INSERT_DETECT] Cannot find usb_insert_detect pin_init!\n");
		return PTR_ERR(pin_init);
	}

	pinctrl_select_state(usb_insert_detect_pinctrl, pin_init);
	printk("[USB_INSERT_DETECT] set usb_insert_detect pin_init success!\n");

	usb_insert_detect_node =
		of_find_compatible_node(NULL, NULL, "mediatek,usb_insert_detect");
	if (!usb_insert_detect_node) {
		printk("[USB_INSERT_DETECT] find usb_insert_detect node failed\n");
		return 0;
	}

	usb_insert_detect_gpio =
		of_get_named_gpio(usb_insert_detect_node, "chip_en", 0);
	if (usb_insert_detect_gpio < 0) {
		printk(KERN_ERR "%s get usb_insert_detect_eint_pin failed!\n",
		       "usb_insert_detect_probe");
		return usb_insert_detect_gpio;
	}

	ret = gpio_request(usb_insert_detect_gpio, "usb_insert_detect_eint_pin");
	if (ret < 0) {
		printk(KERN_ERR "%s gpio_request failed, gpio=%d\n",
		       "usb_insert_detect_probe", usb_insert_detect_gpio);
		return ret;
	}

	usb_insert_detect_irq = irq_of_parse_and_map(usb_insert_detect_node, 0);
	usb_insert_detect_gpio_state = 0;
	usb_insert_detect_flag = 1;

	ret = request_irq(usb_insert_detect_irq,
			  usb_insert_detect_eint_interrupt_handler,
			  IRQF_TRIGGER_HIGH, "usb_insert_detect", NULL);
	printk("[USB_INSERT_DETECT] request_irq IRQF_TRIGGER_HIGH\n");
	if (ret) {
		printk("[USB_INSERT_DETECT] usb_insert_detect set irq failed!!\n");
		return ret;
	}

	printk("[USB_INSERT_DETECT] usb_insert_detect set irq success!!\n");
	return ret;
}

/*
 * usb_insert_detect_remove @0xffffff8008aceb58, 8 bytes: two instructions,
 * "2a1f03e0 mov w0, wzr" and "d65f03c0 ret". It does not stop the kthread, does not free
 * the gpio, does not remove the sysfs attribute -- it is a factory defect, and it is
 * reproduced.
 */
static int usb_insert_detect_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id usb_insert_detect_of_match[] = {
	{ .compatible = "mediatek,usb_insert_detect" },
	{},
};

static struct platform_driver usb_insert_detect_driver = {
	.probe = usb_insert_detect_probe,
	.remove = usb_insert_detect_remove,
	.driver = {
		.name = "usb_insert_detect",
		.of_match_table = usb_insert_detect_of_match,
	},
};

/*
 * usb_insert_detect_init @0xffffff8009385a80, 52 bytes, in `.init.text`.
 *
 * "12800240 mov"@0xffffff8009385aa8 returns -19 = -ENODEV.
 *
 * THERE IS NO `module_exit`: the map has no `usb_insert_detect_exit`,
 * and no `__exitcall` either. The driver registers itself and never unregisters.
 */
static int __init usb_insert_detect_init(void)
{
	if (platform_driver_register(&usb_insert_detect_driver)) {
		printk("[USB_INSERT_DETECT] failed to register driver");
		return -ENODEV;
	}

	return 0;
}

module_init(usb_insert_detect_init);

MODULE_DESCRIPTION("Doogee S88 Pro USB insert detect");
MODULE_LICENSE("GPL");
