// SPDX-License-Identifier: GPL-2.0
/*
 * Sunwave/TrustKernel SPI fingerprint reader, Doogee S88 Pro -- the hardware
 * side: GPIO, pinctrl, regulator, SPI clock, reset.
 *
 * Two translation units, and that is not a stylistic choice: it mirrors what
 * the factory binary shows. Reconstructed from the disassembly of the factory
 * kernel; every constant carries the disassembly line it was read from -- a
 * value without one is a defect.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_hw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>

/*
 * Declared `extern` and not from a header: in the factory kernel these two
 * functions are global (`ffffff8008991ce8 T mt_spi_enable_master_clk`,
 * `ffffff8008991cb4 T mt_spi_disable_master_clk` in oracolo/stock.map) but
 * the ALPS tree does not export them in any header -- the other fingerprint
 * driver in the tree does the same
 * (drivers/input/fingerprint/goodix/gf_spi_tee.h:296).
 */
extern void mt_spi_enable_master_clk(struct spi_device *spidev);
extern void mt_spi_disable_master_clk(struct spi_device *spidev);

#include "sunwave.h"

static int sf_ctl_device_init_gpio_pins(struct sf_ctl_device *ctl);
static int sf_ctl_device_power(bool on);
static int sf_spi_clock_enable(bool on);
static int sf_ctl_device_reset(void);
static int sf_ctl_device_free_gpio(struct sf_ctl_device *ctl);

/*
 * A global pointer to the structure above, at 0xffffff800a100c90 (.bss).
 * sf_ctl_device_init_gpio_pins fills it ("f9064900  str x0, [x8,#3216]"
 * with x8 = adrp 0xffffff800a100000, that is 0xa100c90); it is read back by
 * sf_ctl_device_power and sf_spi_clock_enable at the same offset
 * ("f9464908  ldr x8, [x8,#3216]" in both).
 */
static struct sf_ctl_device *g_ctl_dev;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a100000).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_hw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct pinctrl *g_pinctrl;
static bool g_clk_enabled;
static struct pinctrl_state *g_state_rst_low;
static struct pinctrl_state *g_state_rst_high;
static struct pinctrl_state *g_state_eint_init;
static struct pinctrl_state *g_state_cs_init;
static struct pinctrl_state *g_state_ck_init;
static struct pinctrl_state *g_state_miso_init;
static struct pinctrl_state *g_state_mosi_init;

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_hw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int g_probe_done;

/*
 * sf_platform_init() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_hw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int sf_platform_init(struct sf_ctl_device *ctl)
{
	int err = 0;

	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) enter.\n", 517, "sf_platform_init");
	if (ctl) {
		ctl->init_gpio_pins = sf_ctl_device_init_gpio_pins;
		ctl->free_gpio = sf_ctl_device_free_gpio;
		ctl->power = sf_ctl_device_power;
		ctl->spi_clock_enable = sf_spi_clock_enable;
		ctl->reset = sf_ctl_device_reset;
	} else {
		printk(KERN_ERR "sunwave-sf_hw-%d: %s() ctl_dev is NULL.\n", 527, "sf_platform_init");
		err = -1;
	}
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) leave.\n", 531, "sf_platform_init");
	return err;
}

/*
 * sf_ctl_device_init_gpio_pins() was reconstructed from the factory kernel disassembly (984 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_hw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int sf_ctl_device_init_gpio_pins(struct sf_ctl_device *ctl)
{
	struct device_node *node;
	struct platform_device *pdev;
	const char *name;
	const char *sel;
	int err;

	g_ctl_dev = ctl;
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) enter.\n", 374, "sf_ctl_device_init_gpio_pins");

	ctl->spi->dev.of_node = of_find_compatible_node(NULL, NULL, "goodix,goodix-fp");
	if (!ctl->spi->dev.of_node) {
		printk(KERN_ERR "sunwave-sf_hw-%d: of_find_compatible_node(..) failed.\n", 378);
		return -ENODEV;
	}

	node = of_find_compatible_node(NULL, NULL, "mediatek,goodix-fp");
	if (!node)
		printk(KERN_INFO "sunwave-sf_hw-%d: Cannot find eint node in dts file!!!!!!.\n", 386);

	ctl->irq = irq_of_parse_and_map(node, 0);
	printk(KERN_INFO "sunwave-sf_hw-%d: irq number is %d.\n", 391, ctl->irq);

	pdev = of_find_device_by_node(node);
	if (pdev) {
		g_pinctrl = devm_pinctrl_get(&pdev->dev);
		if (!g_pinctrl) {
			printk(KERN_ERR "sunwave-sf_hw-%d: pinctrl_get(..) failed.\n", 397);
			return -ENODEV;
		}
	}

	printk(KERN_ERR "sunwave-sf_hw-%d: Lookup pinctrl state NOW!!!.\n", 401);

	name = "state_spi_rst_low";
	g_state_rst_low = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_rst_low)
		goto lookup_failed;
	name = "state_spi_rst_high";
	g_state_rst_high = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_rst_high)
		goto lookup_failed;
	name = "state_spi_eint_init";
	g_state_eint_init = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_eint_init)
		goto lookup_failed;
	name = "state_spi_cs_init";
	g_state_cs_init = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_cs_init)
		goto lookup_failed;
	name = "state_spi_ck_init";
	g_state_ck_init = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_ck_init)
		goto lookup_failed;
	name = "state_spi_miso_init";
	g_state_miso_init = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_miso_init)
		goto lookup_failed;
	name = "state_spi_mosi_init";
	g_state_mosi_init = pinctrl_lookup_state(g_pinctrl, name);
	if (g_state_mosi_init)
		goto lookup_done;
lookup_failed:
	printk(KERN_ERR "sunwave-sf_hw-%d: can't find '%s' pinctrl_state.\n", 411, name);
	printk(KERN_ERR "sunwave-sf_hw-%d: %s() failed.\n", 418, "sf_ctl_device_init_gpio_pins");
lookup_done:

	sel = "state_spi_cs_init";
	err = pinctrl_select_state(g_pinctrl, g_state_cs_init);
	if (err)
		goto select_failed;
	printk(KERN_INFO "sunwave-sf_hw-%d: pinctrl_select_state(%s) ok.\n", 430, sel);
	sel = "state_spi_ck_init";
	err = pinctrl_select_state(g_pinctrl, g_state_ck_init);
	if (err)
		goto select_failed;
	printk(KERN_INFO "sunwave-sf_hw-%d: pinctrl_select_state(%s) ok.\n", 430, sel);
	sel = "state_spi_miso_init";
	err = pinctrl_select_state(g_pinctrl, g_state_miso_init);
	if (err)
		goto select_failed;
	printk(KERN_INFO "sunwave-sf_hw-%d: pinctrl_select_state(%s) ok.\n", 430, sel);
	sel = "state_spi_mosi_init";
	err = pinctrl_select_state(g_pinctrl, g_state_mosi_init);
	if (err)
		goto select_failed;
	printk(KERN_INFO "sunwave-sf_hw-%d: pinctrl_select_state(%s) ok.\n", 430, sel);
	goto select_done;
select_failed:
	printk(KERN_ERR "sunwave-sf_hw-%d: %s() pinctrl_select_state(%s) failed.\n",
	       426, "sf_ctl_device_init_gpio_pins", sel);
select_done:

	ctl->vdd_reg = regulator_get(&pdev->dev, "vfingerprint");
	if (IS_ERR(ctl->vdd_reg)) {
		err = PTR_ERR(ctl->vdd_reg);
		printk(KERN_ERR "sunwave-sf_hw-%d: Regulator get failed vdd err = %d\n", 493, err);
		return err;
	}
	if (regulator_count_voltages(ctl->vdd_reg) > 0) {
		err = regulator_set_voltage(ctl->vdd_reg, 2800000, 2800000);
		if (err) {
			printk(KERN_ERR "sunwave-sf_hw-%d: Regulator set_vtg failed vdd err = %d\n", 502, err);
			return err;
		}
	}

	sf_ctl_device_power(true);
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) ok! exit.\n", 509, "sf_ctl_device_init_gpio_pins");
	return err;
}

/*
 * sf_ctl_device_power() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_hw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int sf_ctl_device_power(bool on)
{
	struct sf_ctl_device *ctl = g_ctl_dev;
	int ret;

	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) enter.\n", 236, "sf_ctl_device_power_by_regulator");
	if (!ctl->vdd_reg) {
		printk(KERN_ERR "sunwave-sf_hw-%d: ctl_dev->vdd_reg is NULL.\n", 239);
		return -ENODEV;
	}
	if (on) {
		ret = regulator_enable(ctl->vdd_reg);
		if (ret) {
			printk(KERN_ERR "sunwave-sf_hw-%d: Regulator vdd enable failed err = %d\n", 247, ret);
			return ret;
		}
		msleep(10);
	}
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) leave.\n", 262, "sf_ctl_device_power_by_regulator");
	return 0;
}

/*
 * sf_spi_clock_enable() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_hw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int sf_spi_clock_enable(bool on)
{
	if (!g_clk_enabled && on) {
		mt_spi_enable_master_clk(g_ctl_dev->spi);
		g_clk_enabled = true;
	} else if (g_clk_enabled && !on) {
		mt_spi_disable_master_clk(g_ctl_dev->spi);
		g_clk_enabled = false;
	}
	return 0;
}

/*
 * Position 15. sf_ctl_device_reset(void): a reset pulse through pinctrl
 * (high -> low -> high), not a direct GPIO. The sequence is verified from the
 * arguments of pinctrl_select_state in the disassembly:
 * select(rst_high); msleep(1); select(rst_low); msleep(10);
 * ret = select(rst_high).
 *   "sunwave-sf_hw-%d: %s(..) enter.\n"@0x9250c69   line 162
 *   "sf_ctl_device_reset"@0x92510b1   (funcname; here it DOES match the symbol name)
 *   "sunwave-sf_hw-%d: sf_pinctrl is NULL.\n"@0x92510c5   line 173
 *   "sunwave-sf_hw-%d: %s(..) leave.\n"@0x9250cc8   line 196
 */
static int sf_ctl_device_reset(void)
{
	int ret;

	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) enter.\n", 162, "sf_ctl_device_reset");
	if (!g_pinctrl) {
		printk(KERN_ERR "sunwave-sf_hw-%d: sf_pinctrl is NULL.\n", 173);
		return -1;
	}
	pinctrl_select_state(g_pinctrl, g_state_rst_high);
	msleep(1);
	pinctrl_select_state(g_pinctrl, g_state_rst_low);
	msleep(10);
	ret = pinctrl_select_state(g_pinctrl, g_state_rst_high);
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) leave.\n", 196, "sf_ctl_device_reset");
	return ret;
}

/*
 * Position 16. sf_ctl_device_free_gpio(struct sf_ctl_device *ctl): frees
 * pinctrl and regulator, clears the cached device_node in ctl->spi.
 *   "sunwave-sf_hw-%d: %s(..) enter, free resource.\n"@0x92510ee   line 322
 *   "sf_ctl_device_free_gpio"@0x9251120   (funcname)
 *   "sunwave-sf_hw-%d: %s(..) leave.\n"@0x9250cc8   line 363
 */
static int sf_ctl_device_free_gpio(struct sf_ctl_device *ctl)
{
	printk(KERN_ERR "sunwave-sf_hw-%d: %s(..) enter, free resource.\n", 322, "sf_ctl_device_free_gpio");
	if (ctl->spi->dev.of_node)
		ctl->spi->dev.of_node = NULL;
	if (g_pinctrl) {
		pinctrl_put(g_pinctrl);
		g_pinctrl = NULL;
	}
	if (ctl->vdd_reg) {
		regulator_put(ctl->vdd_reg);
		ctl->vdd_reg = NULL;
	}
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) leave.\n", 363, "sf_ctl_device_free_gpio");
	return 0;
}

/*
 * sf_platform_exit() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sf_hw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void sf_platform_exit(struct sf_ctl_device *ctl)
{
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) enter.\n", 537, "sf_platform_exit");
	if (ctl) {
		ctl->spi_clock_enable = NULL;
		ctl->reset = NULL;
		ctl->free_gpio = NULL;
		ctl->power = NULL;
		ctl->init_gpio_pins = NULL;
	} else {
		printk(KERN_ERR "sunwave-sf_hw-%d: %s() ctl_dev is NULL.\n", 547, "sf_platform_exit");
	}
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) leave.\n", 550, "sf_platform_exit");
}
