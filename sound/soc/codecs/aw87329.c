// SPDX-License-Identifier: GPL-2.0
/*
 * Awinic AW87329 audio power amplifier, Doogee S88 Pro.
 *
 * Reconstructed from the disassembly of the factory kernel, not adapted from
 * another phone. Every register and every constant in this file carries the
 * disassembly line it was read from -- a value without one is a defect.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

/*
 * The driver's private structure, derived from the offsets the batch 1 and
 * batch 2 functions read and write on g_aw87329.
 *
 * Offset 0xc, left "unidentified" by batch 1 (none of that batch's five
 * functions touched it), was identified here:
 * aw87329_audio_kspk/_drcv/_abrcv/_off all check it first
 * (ldrb w9,[x8,#12]; cbz -> "%s: aw87329 init failed\n") -- it is a "the
 * driver is initialised" flag, not a placeholder.
 */
struct aw87329 {
	struct i2c_client *i2c;
	/*
	 * Offset 0x8. The GPIO number used to power the chip on/off and to
	 * reset it. Compared with 0x1ff (511) and passed
	 * to gpio_to_desc(): see aw87329_hw_on/_hw_off/_hw_reset.
	 */
	int hwen_gpio;
	/*
	 * Offset 0xc. Identified in batch 2 (see above): non-zero when
	 * the driver has been initialised successfully.
	 */
	u8 inited;
	/*
	 * Offset 0xd. The "chip powered" flag: 1 after a successful hw_on/hw_reset,
	 * 0 after hw_off or after a failed hw_reset.
	 */
	u8 hwen_flag;
	/*
	 * Offsets 0xe/0xf/0x10, identified in batch 2. One flag per
	 * mode: non-zero if that mode's register table has been loaded from
	 * outside (aw87329_kspk_reg/_drcv_reg/_abrcv_reg, below), otherwise the
	 * default table in .rodata is used. See aw87329_kspk_reg_val and its two
	 * sisters.
	 */
	u8 kspk_reg_loaded;
	u8 drcv_reg_loaded;
	u8 abrcv_reg_loaded;
	/*
	 * This section was reconstructed from the factory kernel disassembly.
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	struct hrtimer timer;
	struct work_struct work;
};

static struct aw87329 *g_aw87329;

/*
 * This section was reconstructed from the factory kernel disassembly (8 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static u8 *aw87329_kspk_reg;
static u8 *aw87329_drcv_reg;
static u8 *aw87329_abrcv_reg;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008faf0b3).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const u8 aw87329_kspk_reg_default[11] = {
	0x39, 0x0e, 0xa3, 0x06, 0x05, 0x10, 0x07, 0x52, 0x06, 0x08, 0x96,
};
static const u8 aw87329_drcv_reg_default[11] = {
	0x39, 0x0a, 0xab, 0x06, 0x05, 0x00, 0x0f, 0x52, 0x09, 0x08, 0x97,
};
static const u8 aw87329_abrcv_reg_default[11] = {
	0x39, 0x0a, 0xaf, 0x06, 0x05, 0x00, 0x0f, 0x52, 0x09, 0x08, 0x97,
};

/*
 * i2c_read_reg and i2c_write_reg are defined at the bottom/in the middle of
 * the file (they follow the target sequence order, not the call order:
 * see driver-riscritti.md). aw87329_read_chipid calls the first,
 * aw87329_audio_kspk the second, both before the point where they are
 * defined -- hence the forward declarations, otherwise -Werror=
 * implicit-function-declaration stops the build.
 */
static int i2c_read_reg(u8 reg_addr);
static void i2c_write_reg(u8 reg_addr, u8 val);

/*
 * aw87329_hw_on() was reconstructed from the factory kernel disassembly (0xffffff8008c585cc, 164 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int aw87329_hw_on(void)
{
	printk(KERN_INFO "%s enter\n", "aw87329_hw_on");

	if (g_aw87329 && gpio_is_valid(g_aw87329->hwen_gpio)) {
		gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 0);
		msleep(2);
		gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 1);
		msleep(2);
		g_aw87329->hwen_flag = 1;
	} else {
		dev_err(&g_aw87329->i2c->dev, "%s:  failed\n", "aw87329_hw_on");
	}

	return 0;
}

/*
 * aw87329_hw_off() was reconstructed from the factory kernel disassembly (0xffffff8008c58670, 132 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int aw87329_hw_off(void)
{
	printk(KERN_INFO "%s enter\n", "aw87329_hw_off");

	if (g_aw87329 && gpio_is_valid(g_aw87329->hwen_gpio)) {
		gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 0);
		msleep(2);
		g_aw87329->hwen_flag = 0;
	} else {
		dev_err(&g_aw87329->i2c->dev, "%s:  failed\n", "aw87329_hw_off");
	}

	return 0;
}

/*
 * aw87329_kspk_reg_val() was reconstructed from the factory kernel disassembly (0xffffff8008c586f4, 56 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
u8 aw87329_kspk_reg_val(u8 index)
{
	if (g_aw87329->kspk_reg_loaded)
		return aw87329_kspk_reg[index + 4];

	return aw87329_kspk_reg_default[index];
}

/*
 * aw87329_drcv_reg_val: 0xffffff8008c5872c, 56 bytes, global (T).
 * Identical to aw87329_kspk_reg_val, on the field/pointer/table of the
 * receiver mode (drcv): offset 0xf, aw87329_drcv_reg (offset 760+8),
 * aw87329_drcv_reg_default (.rodata #0xbe).
 *
 * ffffff8008c5872c..58738: g_aw87329->drcv_reg_loaded (ldrb w8,[x8,#15]).
 * ffffff8008c5873c..5874c: the "loaded" branch: aw87329_drcv_reg[index+4]
 *   (ldr x8,[x8,#768]; ldrb w0,[x8,#4]! after add x8,x8,w0,uxtb).
 * ffffff8008c58750..58760: the default branch: rodata+0xbe+index.
 */
u8 aw87329_drcv_reg_val(u8 index)
{
	if (g_aw87329->drcv_reg_loaded)
		return aw87329_drcv_reg[index + 4];

	return aw87329_drcv_reg_default[index];
}

/*
 * aw87329_abrcv_reg_val() was reconstructed from the factory kernel disassembly (0xffffff8008c58764, 56 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
u8 aw87329_abrcv_reg_val(u8 index)
{
	if (g_aw87329->abrcv_reg_loaded)
		return aw87329_abrcv_reg[index + 4];

	return aw87329_abrcv_reg_default[index];
}

/*
 * aw87329_audio_kspk() was reconstructed from the factory kernel disassembly (0xffffff8008c5879c, 552 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int aw87329_audio_kspk(void)
{
	u8 val;

	if (!g_aw87329) {
		printk(KERN_ERR "%s: aw87329 is NULL\n", "aw87329_audio_kspk");
		return 1;
	}

	if (!g_aw87329->inited) {
		printk(KERN_ERR "%s: aw87329 init failed\n", "aw87329_audio_kspk");
		return 1;
	}

	if (!g_aw87329->hwen_flag)
		aw87329_hw_on();

	val = aw87329_kspk_reg_val(1);
	i2c_write_reg(1, val & ~0x8);

	val = aw87329_kspk_reg_val(2);
	i2c_write_reg(2, val);
	val = aw87329_kspk_reg_val(3);
	i2c_write_reg(3, val);
	val = aw87329_kspk_reg_val(4);
	i2c_write_reg(4, val);
	val = aw87329_kspk_reg_val(5);
	i2c_write_reg(5, val);
	val = aw87329_kspk_reg_val(6);
	i2c_write_reg(6, val);
	val = aw87329_kspk_reg_val(7);
	i2c_write_reg(7, val);
	val = aw87329_kspk_reg_val(8);
	i2c_write_reg(8, val);
	val = aw87329_kspk_reg_val(9);
	i2c_write_reg(9, val);
	val = aw87329_kspk_reg_val(10);
	i2c_write_reg(10, val);

	val = aw87329_kspk_reg_val(1);
	i2c_write_reg(1, val);

	return 0;
}

/*
 * i2c_write_reg() was reconstructed from the factory kernel disassembly (0xffffff8008c589c4, 168 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void i2c_write_reg(u8 reg_addr, u8 val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(g_aw87329->i2c, reg_addr, val);
	if (ret < 0) {
		printk(KERN_ERR "%s: i2c_write cnt=%d error=%d\n",
		       "i2c_write_reg", 0, ret);
		msleep(2);

		ret = i2c_smbus_write_byte_data(g_aw87329->i2c, reg_addr, val);
		if (ret < 0) {
			printk(KERN_ERR "%s: i2c_write cnt=%d error=%d\n",
			       "i2c_write_reg", 1, ret);
			msleep(2);
		}
	}
}

/*
 * aw87329_audio_drcv: 0xffffff8008c58a6c, 552 bytes, global (T).
 * Identical to aw87329_audio_kspk, substituting the flag/table of the
 * receiver mode (offset 0xf, aw87329_drcv_reg_val) and the name in the
 * messages. Same sequence of eleven registers (1..10, then 1), same
 * single "and ...,#0xfffffff7" on the first write of register 1. Same
 * warning as aw87329_audio_kspk about inlining: no "bl
 * aw87329_drcv_reg_val" in the factory binary.
 *
 * Strings (same page, verified):
 *   "%s: aw87329 is NULL\n"@0x92a2fa2 (shared with audio_kspk)
 *     -- ffffff8008c58c60 adrp x0 / ffffff8008c58c64 add x0,x0,#0xfa2
 *   "%s: aw87329 init failed\n"@0x92a2fcc (shared)
 *     -- ffffff8008c58c6c adrp x0 / ffffff8008c58c70 add x0,x0,#0xfcc
 *   "aw87329_audio_drcv"@0x92a2fe7
 *     -- ffffff8008c58c74 adrp x1 / ffffff8008c58c78 add x1,x1,#0xfe7
 */
int aw87329_audio_drcv(void)
{
	u8 val;

	if (!g_aw87329) {
		printk(KERN_ERR "%s: aw87329 is NULL\n", "aw87329_audio_drcv");
		return 1;
	}

	if (!g_aw87329->inited) {
		printk(KERN_ERR "%s: aw87329 init failed\n", "aw87329_audio_drcv");
		return 1;
	}

	if (!g_aw87329->hwen_flag)
		aw87329_hw_on();

	val = aw87329_drcv_reg_val(1);
	i2c_write_reg(1, val & ~0x8);

	val = aw87329_drcv_reg_val(2);
	i2c_write_reg(2, val);
	val = aw87329_drcv_reg_val(3);
	i2c_write_reg(3, val);
	val = aw87329_drcv_reg_val(4);
	i2c_write_reg(4, val);
	val = aw87329_drcv_reg_val(5);
	i2c_write_reg(5, val);
	val = aw87329_drcv_reg_val(6);
	i2c_write_reg(6, val);
	val = aw87329_drcv_reg_val(7);
	i2c_write_reg(7, val);
	val = aw87329_drcv_reg_val(8);
	i2c_write_reg(8, val);
	val = aw87329_drcv_reg_val(9);
	i2c_write_reg(9, val);
	val = aw87329_drcv_reg_val(10);
	i2c_write_reg(10, val);

	val = aw87329_drcv_reg_val(1);
	i2c_write_reg(1, val);

	return 0;
}

/*
 * aw87329_audio_abrcv() was reconstructed from the factory kernel disassembly (0xffffff8008c58c94, 552 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int aw87329_audio_abrcv(void)
{
	u8 val;

	if (!g_aw87329) {
		printk(KERN_ERR "%s: aw87329 is NULL\n", "aw87329_audio_abrcv");
		return 1;
	}

	if (!g_aw87329->inited) {
		printk(KERN_ERR "%s: aw87329 init failed\n", "aw87329_audio_abrcv");
		return 1;
	}

	if (!g_aw87329->hwen_flag)
		aw87329_hw_on();

	val = aw87329_abrcv_reg_val(1);
	i2c_write_reg(1, val & ~0x8);

	val = aw87329_abrcv_reg_val(2);
	i2c_write_reg(2, val);
	val = aw87329_abrcv_reg_val(3);
	i2c_write_reg(3, val);
	val = aw87329_abrcv_reg_val(4);
	i2c_write_reg(4, val);
	val = aw87329_abrcv_reg_val(5);
	i2c_write_reg(5, val);
	val = aw87329_abrcv_reg_val(6);
	i2c_write_reg(6, val);
	val = aw87329_abrcv_reg_val(7);
	i2c_write_reg(7, val);
	val = aw87329_abrcv_reg_val(8);
	i2c_write_reg(8, val);
	val = aw87329_abrcv_reg_val(9);
	i2c_write_reg(9, val);
	val = aw87329_abrcv_reg_val(10);
	i2c_write_reg(10, val);

	val = aw87329_abrcv_reg_val(1);
	i2c_write_reg(1, val);

	return 0;
}

/*
 * aw87329_audio_off() was reconstructed from the factory kernel disassembly (0xffffff8008c58ebc, 212 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int aw87329_audio_off(void)
{
	if (!g_aw87329) {
		printk(KERN_ERR "%s: aw87329 is NULL\n", "aw87329_audio_off");
		return 1;
	}

	if (!g_aw87329->inited) {
		printk(KERN_ERR "%s: aw87329 init failed\n", "aw87329_audio_off");
		return 1;
	}

	if (g_aw87329->hwen_flag)
		i2c_write_reg(1, 0xc);

	printk(KERN_INFO "%s enter\n", "aw87329_hw_off");

	if (!g_aw87329 || !gpio_is_valid(g_aw87329->hwen_gpio)) {
		dev_err(&g_aw87329->i2c->dev, "%s:  failed\n", "aw87329_hw_off");
		return 0;
	}

	gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 0);
	msleep(2);
	g_aw87329->hwen_flag = 0;

	return 0;
}

/*
 * aw87329_hw_reset: 0xffffff8008c58f90, 168 bytes, global (T).
 *
 * The same skeleton as aw87329_hw_on (two pulses, 0 then 1, hwen_flag=1
 * on the successful path), with a single verified difference: on the error
 * path it also writes hwen_flag = 0 before dev_err (strb wzr,[x8,#13]
 * at 0xffffff8008c59020, absent from the else of hw_on/hw_off) -- 168 bytes
 * against 164 for hw_on, consistent with that extra instruction.
 *
 * ffffff8008c58f9c..58fac: printk(KERN_INFO "%s enter\n"@0x92a2f79,
 *   "aw87329_hw_reset"@0x92a3020)
 *   -- ffffff8008c58fa0 adrp x1 / ffffff8008c58fa8 add x1,x1,#0x20.
 * ffffff8008c58fb0..58fc4: the same guard if (g_aw87329 && gpio_is_valid(...)).
 * ffffff8008c58fc8..59004: two rounds of gpio_to_desc()+gpiod_set_raw_value_
 *   cansleep()+msleep(2), 0 then 1, then hwen_flag = 1.
 * ffffff8008c59008..59024: the else path, hwen_flag = 0 then dev_err(&g_aw87329->
 *   i2c->dev, "%s:  failed\n"@0x91c7380, "aw87329_hw_reset"@0x92a3020) -- the
 *   same name string as the printk above, reloaded from
 *   ffffff8008c59010 adrp x2 / ffffff8008c5901c add x2,x2,#0x20.
 */
int aw87329_hw_reset(void)
{
	printk(KERN_INFO "%s enter\n", "aw87329_hw_reset");

	if (g_aw87329 && gpio_is_valid(g_aw87329->hwen_gpio)) {
		gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 0);
		msleep(2);
		gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 1);
		msleep(2);
		g_aw87329->hwen_flag = 1;
	} else {
		g_aw87329->hwen_flag = 0;
		dev_err(&g_aw87329->i2c->dev, "%s:  failed\n", "aw87329_hw_reset");
	}

	return 0;
}

/*
 * aw87329_read_chipid() was reconstructed from the factory kernel disassembly (0xffffff8008c59038, 152 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int aw87329_read_chipid(void)
{
	int val;

	val = i2c_read_reg(0) & 0xff;
	if (val != 0x39) {
		printk(KERN_INFO "%s: aw87329 chipid=0x%x error\n",
		       "aw87329_read_chipid", val);
		msleep(2);

		val = i2c_read_reg(0) & 0xff;
		if (val != 0x39) {
			printk(KERN_INFO "%s: aw87329 chipid=0x%x error\n",
			       "aw87329_read_chipid", val);
			msleep(2);
			return -EINVAL;
		}
	}

	printk(KERN_INFO "%s: aw87329 chipid=0x%x\n", "aw87329_read_chipid", 0x39);
	return 0;
}

/*
 * i2c_read_reg() was reconstructed from the factory kernel disassembly (0xffffff8008c590d0, 168 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int i2c_read_reg(u8 reg_addr)
{
	int ret;

	ret = i2c_smbus_read_byte_data(g_aw87329->i2c, reg_addr);
	if (ret < 0) {
		printk(KERN_ERR "%s: i2c_read cnt=%d error=%d\n",
		       "i2c_read_reg", 0, ret);
		msleep(2);

		ret = i2c_smbus_read_byte_data(g_aw87329->i2c, reg_addr);
		if (ret < 0) {
			printk(KERN_ERR "%s: i2c_read cnt=%d error=%d\n",
			       "i2c_read_reg", 1, ret);
			msleep(2);
		}
	}

	return ret;
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008c59e68, 68 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * Forward declaration: defined at the bottom of the file, taken by address by
 * aw87329_i2c_probe (hrtimer.function and work.func) -- see above.
 */
static enum hrtimer_restart cfg_timer_func(struct hrtimer *timer);
static void cfg_work_routine(struct work_struct *work);

/*
 * Forward declaration: the sysfs show/store pair, taken by address by the
 * DEVICE_ATTRs below -- needed before aw87329_i2c_probe, which creates
 * the sysfs group, but the real functions are written further down in the
 * order of the target sequence (positions 15-21). Same reason as the forward
 * declaration of i2c_read_reg/i2c_write_reg in batches 1-2.
 */
static ssize_t aw87329_get_reg(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t aw87329_set_reg(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t aw87329_get_hwen(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t aw87329_set_hwen(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count);
static ssize_t aw87329_get_update(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t aw87329_set_update(struct device *dev, struct device_attribute *attr,
				   const char *buf, size_t count);
static ssize_t aw87329_get_mode(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t aw87329_set_mode(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count);

/*
 * Forward declaration: aw87329_set_update (position 21) and cfg_work_routine
 * (outside the 27, see below) take the address of
 * aw87329_kspk_cfg_loaded (position 22) before it is defined; and the
 * chain of firmware requests has each callback taking the address of the
 * next before it is defined (kspk->drcv->abrcv,
 * in the target sequence order 22-23-24).
 */
static void aw87329_kspk_cfg_loaded(const struct firmware *fw, void *context);
static void aw87329_drcv_cfg_loaded(const struct firmware *fw, void *context);
static void aw87329_abrcv_cfg_loaded(const struct firmware *fw, void *context);

/*
 * DEVICE_ATTR() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static DEVICE_ATTR(reg, 0660, aw87329_get_reg, aw87329_set_reg);
static DEVICE_ATTR(hwen, 0660, aw87329_get_hwen, aw87329_set_hwen);
static DEVICE_ATTR(update, 0660, aw87329_get_update, aw87329_set_update);
static DEVICE_ATTR(mode, 0660, aw87329_get_mode, aw87329_set_mode);

static struct attribute *aw87329_attributes[] = {
	&dev_attr_reg.attr,
	&dev_attr_hwen.attr,
	&dev_attr_update.attr,
	&dev_attr_mode.attr,
	NULL,
};

static struct attribute_group aw87329_attribute_group = {
	.attrs = aw87329_attributes,
};

/*
 * aw87329_i2c_probe() was reconstructed from the factory kernel disassembly (0xffffff8008c59178, 708 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int aw87329_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device_node *np = client->dev.of_node;
	struct device *dev = &client->dev;
	struct aw87329 *priv;
	int gpio;
	int ret;

	printk(KERN_INFO "%s Enter\n", "aw87329_i2c_probe");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(dev, "%s: check_functionality failed\n", "aw87329_i2c_probe");
		return -ENODEV;
	}

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	g_aw87329 = priv;
	if (!priv)
		return -ENOMEM;

	priv->i2c = client;
	i2c_set_clientdata(client, priv);

	if (np) {
		gpio = of_get_named_gpio(np, "reset-gpio", 0);
		priv->hwen_gpio = gpio;
		if (gpio < 0) {
			dev_err(dev, "%s: no reset gpio provided\n", "aw87329_parse_dt");
			dev_err(dev, "%s: failed to parse device tree node\n",
				"aw87329_i2c_probe");
			ret = -1;
			goto err;
		}

		dev_info(dev, "%s: reset gpio provided ok\n", "aw87329_parse_dt");

		if (gpio_is_valid(priv->hwen_gpio)) {
			ret = devm_gpio_request_one(dev, priv->hwen_gpio,
						     GPIOF_OUT_INIT_LOW, "aw87329_rst");
			if (ret) {
				dev_err(dev, "%s: rst request failed\n", "aw87329_i2c_probe");
				goto err;
			}
		}
	} else {
		priv->hwen_gpio = -1;
	}

	aw87329_hw_reset();
	ret = aw87329_read_chipid();
	if (ret < 0) {
		dev_err(dev, "%s: aw87329_read_chipid failed ret=%d\n",
			"aw87329_i2c_probe", ret);
		devm_gpio_free(dev, g_aw87329->hwen_gpio);
		goto err;
	}

	ret = sysfs_create_group(&dev->kobj, &aw87329_attribute_group);
	if (ret < 0)
		dev_info(dev, "%s error creating sysfs attr files\n", "aw87329_i2c_probe");

	priv->kspk_reg_loaded = 0;
	priv->drcv_reg_loaded = 0;
	priv->abrcv_reg_loaded = 0;

	hrtimer_init(&priv->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	priv->timer.function = cfg_timer_func;
	INIT_WORK(&priv->work, cfg_work_routine);
	hrtimer_start_range_ns(&priv->timer, ns_to_ktime(5000000000ULL), 0, HRTIMER_MODE_REL);

	/* aw87329_hw_off() inlined by the compiler, see above. */
	aw87329_hw_off();

	priv->inited = 1;
	return 0;

err:
	/*
	 * A COMMON ERROR TAIL for every path after a successful allocation.
	 * The factory shows it at 0xffffff8008c59418..59428: it re-reads
	 * g_aw87329 ("f9417ac1 ldr"@0xffffff8008c59418), calls devm_kfree
	 * ("97e262eb bl"@0xffffff8008c59420) and clears the global. Without it,
	 * the structure stays allocated and g_aw87329 points at memory freed
	 * by devm when the device is removed.
	 */
	devm_kfree(dev, g_aw87329);
	g_aw87329 = NULL;
	return ret;
}

/*
 * aw87329_i2c_remove() was reconstructed from the factory kernel disassembly (0xffffff8008c5943c, 48 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int aw87329_i2c_remove(struct i2c_client *client)
{
	if (gpio_is_valid(g_aw87329->hwen_gpio))
		devm_gpio_free(&client->dev, g_aw87329->hwen_gpio);

	return 0;
}

/*
 * aw87329_get_reg: 0xffffff8008c5946c, 128 bytes, static (t).
 *
 * It lists registers 0..10 (eleven, the same count as the batch 2 default
 * tables), reading them through i2c_read_reg, in a loop that however
 * counts to 15, skipping indexes 11..14 (continue, not break):
 *
 * ffffff8008c59480..59494: w23 = 0x1000 (PAGE_SIZE, orr w23,wzr,#0x1000);
 *   x22 = &"reg:0x%02x=0x%02x\n"@0x92a3179 (ffffff8008c59480 adrp x22 on
 *   page 0xffffff80092a3000 / ffffff8008c59494 add x22,x22,#0x179 --
 *   loaded once only OUTSIDE the loop, which is also the confirmation
 *   that the loop is there).
 * ffffff8008c59498..5949c: cmp w21,#0xa; b.hi -> 0x594c8 (if index>10,
 *   skip the body, go to the increment: continue).
 * ffffff8008c594a0..594c4: val = i2c_read_reg(i) & 0xff (and w4,w0,#0xff);
 *   len += snprintf(buf+len, PAGE_SIZE-len, "reg:0x%02x=0x%02x\n", i,
 *   val) (x1 = x23-x19 = remaining size, x0 = x20+x19 = buf+len).
 * ffffff8008c594c8..594d0: i++; cmp w21,#0xf(15); b.ne -> back to the loop
 *   (carry on until i != 15).
 * ffffff8008c594d4: return len (x19, sxtw accumulated).
 */
static ssize_t aw87329_get_reg(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;
	unsigned char val;
	unsigned char i;

	for (i = 0; i < 15; i++) {
		if (i > 10)
			continue;

		val = i2c_read_reg(i) & 0xff;
		len += snprintf(buf + len, PAGE_SIZE - len, "reg:0x%02x=0x%02x\n", i, val);
	}

	return len;
}

/*
 * aw87329_set_reg() was reconstructed from the factory kernel disassembly (0xffffff8008c594ec, 120 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw87329_set_reg(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned int reg = 0;
	unsigned int val = 0;

	if (sscanf(buf, "%x %x", &reg, &val) == 2)
		i2c_write_reg(reg, val);

	return count;
}

/*
 * aw87329_get_hwen: 0xffffff8008c59564, 52 bytes, static (t).
 *
 * ffffff8008c5956c..59588: snprintf(buf, PAGE_SIZE, "hwen: %d\n"@0x92a318c,
 *   g_aw87329->hwen_flag) -- ffffff8008c59578 adrp x2 / ffffff8008c5957c add
 *   x2,x2,#0x18c; the value = ldrb
 *   w3,[x8,#13] (hwen_flag, the offset verified in batches 1/2); w1=0x1000
 *   (PAGE_SIZE).
 * ffffff8008c5958c: sxtw x0,w0 -- return (ssize_t)ret.
 */
static ssize_t aw87329_get_hwen(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "hwen: %d\n", g_aw87329->hwen_flag);
}

/*
 * aw87329_set_hwen() was reconstructed from the factory kernel disassembly (0xffffff8008c59598, 216 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw87329_set_hwen(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count)
{
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008c595c8).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	unsigned int val;

	sscanf(buf, "%d", &val);
	if (val)
		aw87329_hw_on();
	else
		aw87329_hw_off();

	return count;
}

/*
 * aw87329_get_update: 0xffffff8008c59670, 8 bytes, static (t).
 *
 * ffffff8008c59670..59674: mov x0,xzr; ret -- **return 0, without
 *   reading dev, attr or buf**. Two instructions, exactly 8 bytes.
 *   It is not a misreading on our part: this attribute's "get", in the
 *   factory build, never writes anything into the sysfs buffer (the file,
 *   when read, always comes back empty). The behaviour is reproduced as it is.
 */
static ssize_t aw87329_get_update(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

/*
 * aw87329_set_update() was reconstructed from the factory kernel disassembly (0xffffff8008c59678, 200 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw87329_set_update(struct device *dev, struct device_attribute *attr,
				   const char *buf, size_t count)
{
	unsigned int val;
	int ret;

	sscanf(buf, "%d", &val);
	if (val) {
		g_aw87329->kspk_reg_loaded = 0;
		g_aw87329->drcv_reg_loaded = 0;
		g_aw87329->abrcv_reg_loaded = 0;

		ret = request_firmware_nowait(NULL, true, "aw87329_kspk.bin",
					       &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
					       aw87329_kspk_cfg_loaded);
		if (ret)
			printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
			       "aw87329_set_update", "aw87329_kspk.bin");
	}

	return count;
}

/*
 * aw87329_kspk_cfg_loaded() was reconstructed from the factory kernel disassembly (0xffffff8008c59740, 576 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw87329_kspk_cfg_loaded(const struct firmware *fw, void *context)
{
	unsigned int i;
	size_t len;
	int ret;

	if (!fw) {
		printk(KERN_ERR "%s: failed to read %s\n",
		       "aw87329_kspk_cfg_loaded", "aw87329_kspk.bin");
		release_firmware(fw);
		/*
		 * request_firmware_nowait() was reconstructed from the factory kernel disassembly (0xffffff8008c598b8, 48 bytes).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		ret = request_firmware_nowait(NULL, true, "aw87329_drcv.bin",
					       &g_aw87329->i2c->dev, GFP_KERNEL,
					       NULL, aw87329_drcv_cfg_loaded);
		if (ret) {
			printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
			       "aw87329_kspk_cfg_loaded", "aw87329_drcv.bin");
			g_aw87329->drcv_reg_loaded = 0;
		}
		return;
	}

	printk(KERN_INFO "%s: loaded %s - size: %zu\n",
	       "aw87329_kspk_cfg_loaded", "aw87329_kspk.bin", fw->size);

	for (i = 0; i < fw->size; i++)
		printk(KERN_INFO "%s: cont: addr:0x%02x, data:0x%02x\n",
		       "aw87329_kspk_cfg_loaded", i, fw->data[i]);

	len = fw->size;
	aw87329_kspk_reg = kzalloc(len + 4, GFP_KERNEL);
	if (!aw87329_kspk_reg) {
		release_firmware(fw);
		printk(KERN_ERR "%s: error allocating memory\n", "aw87329_kspk_cfg_loaded");
		return;
	}

	*(u32 *)aw87329_kspk_reg = fw->size;
	memcpy(aw87329_kspk_reg + 4, fw->data, fw->size);
	release_firmware(fw);

	for (i = 0; i < len; i++)
		printk(KERN_INFO "%s: spk_cnt: addr:0x%02x, data:0x%02x\n",
		       "aw87329_kspk_cfg_loaded", i, aw87329_kspk_reg_val(i));

	g_aw87329->kspk_reg_loaded = 1;

	ret = request_firmware_nowait(NULL, true, "aw87329_drcv.bin",
				       &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
				       aw87329_drcv_cfg_loaded);
	if (ret) {
		printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
		       "aw87329_kspk_cfg_loaded", "aw87329_drcv.bin");
		g_aw87329->drcv_reg_loaded = 0;
	}
}

/*
 * aw87329_drcv_cfg_loaded() was reconstructed from the factory kernel disassembly (0xffffff8008c59980, 576 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw87329_drcv_cfg_loaded(const struct firmware *fw, void *context)
{
	unsigned int i;
	size_t len;
	int ret;

	if (!fw) {
		printk(KERN_ERR "%s: failed to read %s\n",
		       "aw87329_drcv_cfg_loaded", "aw87329_drcv.bin");
		release_firmware(fw);
		/*
		 * As in `aw87329_kspk_cfg_loaded`: the call is repeated here,
		 * there is no `goto` to the tail. See the note there in full.
		 */
		ret = request_firmware_nowait(NULL, true, "aw87329_abrcv.bin",
					       &g_aw87329->i2c->dev, GFP_KERNEL,
					       NULL, aw87329_abrcv_cfg_loaded);
		if (ret) {
			printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
			       "aw87329_drcv_cfg_loaded", "aw87329_abrcv.bin");
			g_aw87329->abrcv_reg_loaded = 0;
		}
		return;
	}

	printk(KERN_INFO "%s: loaded %s - size: %zu\n",
	       "aw87329_drcv_cfg_loaded", "aw87329_drcv.bin", fw->size);

	for (i = 0; i < fw->size; i++)
		printk(KERN_INFO "%s: cont: addr:0x%02x, data:0x%02x\n",
		       "aw87329_drcv_cfg_loaded", i, fw->data[i]);

	len = fw->size;
	aw87329_drcv_reg = kzalloc(len + 4, GFP_KERNEL);
	if (!aw87329_drcv_reg) {
		release_firmware(fw);
		printk(KERN_ERR "%s: error allocating memory\n", "aw87329_drcv_cfg_loaded");
		return;
	}

	*(u32 *)aw87329_drcv_reg = fw->size;
	memcpy(aw87329_drcv_reg + 4, fw->data, fw->size);
	release_firmware(fw);

	for (i = 0; i < len; i++)
		printk(KERN_INFO "%s: rcv_cnt: addr:0x%02x, data:0x%02x\n",
		       "aw87329_drcv_cfg_loaded", i, aw87329_drcv_reg_val(i));

	g_aw87329->drcv_reg_loaded = 1;

	ret = request_firmware_nowait(NULL, true, "aw87329_abrcv.bin",
				       &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
				       aw87329_abrcv_cfg_loaded);
	if (ret) {
		printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
		       "aw87329_drcv_cfg_loaded", "aw87329_abrcv.bin");
		g_aw87329->abrcv_reg_loaded = 0;
	}
}

/*
 * aw87329_abrcv_cfg_loaded() was reconstructed from the factory kernel disassembly (0xffffff8008c59bc0, 416 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void aw87329_abrcv_cfg_loaded(const struct firmware *fw, void *context)
{
	unsigned int i;
	size_t len;

	if (!fw) {
		printk(KERN_ERR "%s: failed to read %s\n",
		       "aw87329_abrcv_cfg_loaded", "aw87329_abrcv.bin");
		release_firmware(fw);
		return;
	}

	printk(KERN_INFO "%s: loaded %s - size: %zu\n",
	       "aw87329_abrcv_cfg_loaded", "aw87329_abrcv.bin", fw->size);

	for (i = 0; i < fw->size; i++)
		printk(KERN_INFO "%s: cont: addr:0x%02x, data:0x%02x\n",
		       "aw87329_abrcv_cfg_loaded", i, fw->data[i]);

	len = fw->size;
	aw87329_abrcv_reg = kzalloc(len + 4, GFP_KERNEL);
	if (!aw87329_abrcv_reg) {
		release_firmware(fw);
		printk(KERN_ERR "%s: error allocating memory\n", "aw87329_abrcv_cfg_loaded");
		return;
	}

	*(u32 *)aw87329_abrcv_reg = fw->size;
	memcpy(aw87329_abrcv_reg + 4, fw->data, fw->size);
	release_firmware(fw);

	for (i = 0; i < len; i++)
		printk(KERN_INFO "%s: rcv_cnt: addr:0x%02x, data:0x%02x\n",
		       "aw87329_abrcv_cfg_loaded", i, aw87329_abrcv_reg_val(i));

	g_aw87329->abrcv_reg_loaded = 1;
}

/*
 * aw87329_get_mode() was reconstructed from the factory kernel disassembly (0xffffff8008c59d60, 104 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw87329_get_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;

	len += sprintf(buf + len, "0: off mode\n");
	len += sprintf(buf + len, "1: kspk mode\n");
	len += sprintf(buf + len, "2: drcv mode\n");
	len += sprintf(buf + len, "3: abrcv mode\n");

	return len;
}

/*
 * aw87329_set_mode() was reconstructed from the factory kernel disassembly (0xffffff8008c59dc8, 160 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t aw87329_set_mode(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count)
{
	unsigned int val = 0;

	sscanf(buf, "%d", &val);
	switch (val) {
	case 1:
		aw87329_audio_kspk();
		break;
	case 2:
		aw87329_audio_drcv();
		break;
	case 3:
		aw87329_audio_abrcv();
		break;
	case 0:
	default:
		aw87329_audio_off();
		break;
	}

	return count;
}

/*
 * cfg_timer_func() was reconstructed from the factory kernel disassembly (0xffffff8008c59e68, 68 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static enum hrtimer_restart cfg_timer_func(struct hrtimer *timer)
{
	printk(KERN_INFO "%s enter\n", "cfg_timer_func");

	queue_work_on(8, system_wq, &g_aw87329->work);

	return HRTIMER_NORESTART;
}

/*
 * cfg_work_routine() was reconstructed from the factory kernel disassembly (0xffffff8008c59eac, 124 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void cfg_work_routine(struct work_struct *work)
{
	int ret;

	printk(KERN_INFO "%s enter\n", "cfg_work_routine");

	ret = request_firmware_nowait(NULL, true, "aw87329_kspk.bin",
				       &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
				       aw87329_kspk_cfg_loaded);
	if (ret)
		printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
		       "cfg_work_routine", "aw87329_kspk.bin");
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff80099c9f90).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const struct i2c_device_id aw87329_i2c_id[] = {
	{ "aw87329_pa", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, aw87329_i2c_id);

static const struct of_device_id aw87329_of_match[] = {
	{ .compatible = "awinic,aw87329_pa" },
	{ },
};
MODULE_DEVICE_TABLE(of, aw87329_of_match);

static struct i2c_driver aw87329_i2c_driver = {
	.driver = {
		.name = "aw87329_pa",
		.owner = THIS_MODULE,
		.of_match_table = aw87329_of_match,
	},
	.probe = aw87329_i2c_probe,
	.remove = aw87329_i2c_remove,
	.id_table = aw87329_i2c_id,
};

/*
 * aw87329_pa_init() was reconstructed from the factory kernel disassembly (0xffffff8009390f6c, 124 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int __init aw87329_pa_init(void)
{
	int ret;

	printk(KERN_INFO "%s enter\n", "aw87329_pa_init");
	printk(KERN_INFO "%s: driver version: %s\n", "aw87329_pa_init", "v1.1.2");

	ret = i2c_register_driver(THIS_MODULE, &aw87329_i2c_driver);
	if (ret)
		printk(KERN_INFO "****[%s] Unable to register driver (%d)\n",
		       "aw87329_pa_init", ret);

	return ret;
}

module_init(aw87329_pa_init);

static void __exit aw87329_pa_exit(void)
{
	printk(KERN_INFO "%s enter\n", "aw87329_pa_exit");
	i2c_del_driver(&aw87329_i2c_driver);
}

module_exit(aw87329_pa_exit);

/*
 * MODULE_DESCRIPTION() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/sound_soc_codecs_aw87329.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
MODULE_DESCRIPTION("AW87329 PA Driver");
MODULE_LICENSE("GPL v2");
