// SPDX-License-Identifier: GPL-2.0
/*
 * Maxic MT5725 wireless charger, Doogee S88 Pro (E977).
 *
 * Sixty-one functions; 55 of them adjacent, from MT5725_soft_reset
 * (0xffffff8008aca8d4) to Mt5725_get_rxdetect (0xffffff8008ace894), 16372
 * bytes of code.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was opened: every line is derived from the bytes of the binary.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/power_supply.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/pm_wakeup.h>
#include <linux/sysfs.h>
#include <mt-plat/mtk_battery.h>
#include <mt-plat/charger_class.h>
#include <mt-plat/mtk_boot_common.h>

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008ace8c8, 40 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#include "../mtk_charger_intf.h"

/* The two firmware blobs, extracted with a script from the oracle bytes. */
#include "mt5725_fw.h"

/*
 * The binary NAMES this union and its member, inside the message that
 * mt5725_charger_routine_thread prints:
 *   " MT5725_write_buffer(mte, REG_VFC, temp.ptr, 2)\n"@0xffffff80092637fe
 * `temp` is the name of the variable, `ptr` the name of the byte member. The
 * name of the 16-bit member is NOT named by the binary: it is called `c0`.
 */
union mt5725_u16 {
	u16	c0;
	u8	ptr[2];
};

/* ------------------------------------------------------------------------ */
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008acd378).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct mt5725_chip {
	u8			c0[8];
	/* +0x08 "a900d113 stp"@0xffffff8008acd450 (x19 = i2c_client) */
	struct i2c_client	*c8;
	/* +0x10 argument of dev_err: "f9400a60 ldr"@0xffffff8008acabb0 */
	struct device		*c16;
	/* +0x18 argument of regmap_read: "f9400c00 ldr"@0xffffff8008acae00 */
	struct regmap		*c24;
	/* +0x20 / +0x28 "a9022909 stp"@0xffffff8008acd420 (MT5725_read, MT5725_write) */
	int			(*c32)(struct mt5725_chip *chip, u16 reg, u8 *val);
	int			(*c40)(struct mt5725_chip *chip, u16 reg, u8 val);
	/* +0x30 / +0x38 "a903310b stp"@0xffffff8008acd454 (read_buffer, write_buffer) */
	int			(*c48)(struct mt5725_chip *chip, u16 reg, u8 *buf, u32 len);
	int			(*c56)(struct mt5725_chip *chip, u16 reg, u8 *buf, u32 len);
	/* +0x40 "b90042a0 str"@0xffffff8008acd50c (gpio for "dc-gpio") */
	int			c64;
	u8			c68[4];
	/* +0x48 "b9004aa0 str"@0xffffff8008acd594 (gpio for "eint_wpc") */
	int			c72;
	u8			c76[4];
	/* +0x50 "b9405108 ldr"@0xffffff8008ace8b0 (read by Mt5725_get_rxdetect) */
	int			c80;
	u8			c84[4];
	/* +0x58 "97d97c4e bl"@0xffffff8008acd3c4 (__mutex_init su chip+0x58) */
	struct mutex		slock;
	/* +0x78 "9101e10a add"@0xffffff8008acd484, func = MT5725_int_delayed_work_func */
	struct delayed_work	c120;
	/* +0xd8 "9103810b add"@0xffffff8008acd488, func = mt5725_charger_work_func */
	struct delayed_work	c216;
	/* +0x138 "9104e2a8 add"@0xffffff8008acd4f8, func = MT5725_good_int_delayed_work_func */
	struct delayed_work	c312;
	u8			c408[4];
	/* +0x19c "b9019d1f str"@0xffffff8008acd45c (cleared by probe) */
	int			c412;
	u8			c416[400];
	/* +0x330 "b9033114 str"@0xffffff8008ace520 (written by mt5725_set_otg) */
	int			c816;
	/* +0x334 "b9033509 str"@0xffffff8008ace6bc (written by mt5725_set_otp) */
	int			c820;
	/* +0x338 "b9433903 ldr"@0xffffff8008ace298 (read by mt5725_get_reverse_charger) */
	int			c824;
	u8			c828[4];
};

/* ------------------------------------------------------------------------ */
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a110fe4).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mt5725_wls_online;			/* 0xffffff800a110fe4 */
int mt5725_rvs_online;			/* 0xffffff800a110fec */
extern int usb_insert_stato;		/* 0xffffff800a110ff4, of usb_insert_detect */

/*
 * NOT `static`: MT5725_good_irq loads it at FULL WIDTH
 *   "b94fe908 ldr"@0xffffff8008acdd60
 * and MT5725_good_int_delayed_work_func clears it at full width
 *   "b90fe91f str"@0xffffff8008acdbe0
 * Had it internal linkage clang would narrow it to a byte
 * (`ldrb`/`strb`), because it only takes the values 0 and 1.
 */
int mt5725_good_irq_ready;		/* 0xffffff800a110fe8 */
static struct mt5725_chip *mt5725_chip_p;	/* 0xffffff800a111000 */
static struct mt5725_chip *mt5725_chip_q;	/* 0xffffff800a111008 */
static int mt5725_fastcharge_timeout;	/* 0xffffff800a111010 */
u8 mt5725_fastcharge_state;		/* 0xffffff800a110ff8 */
/*
 * NOT `static`: mt5725_set_hwen writes it and NOBODY in the whole image
 * reads it. Were it `static`, the compiler would delete the write (and the
 * variable), while the factory binary executes it:
 *   "b9002d28 str"@0xffffff8008ace73c
 */
int mt5725_hwen;			/* 0xffffff800a11102c */
static int mt5725_irq_num;		/* 0xffffff800a111070 */
static int mt5725_good_irq_num;		/* 0xffffff800a111074 */
static struct charger_device *mt5725_chg_dev;	/* 0xffffff800a111090 */
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a111024).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mt5725_g111024[2];			/* 0xffffff800a111024, 0xffffff800a111028 */
/* name MEASURED: "rxdetect_count = 0\n"@0xffffff80092637ea */
static int mt5725_rxdetect_count;	/* 0xffffff800a111168 */
/*
 * UNSIGNED: the comparison with 2 in mt5725_charger_routine_thread is
 * "54000083 b.cc"@0xffffff8008acde44, not a b.lt
 */
static volatile unsigned int mt5725_g11101c;	/* 0xffffff800a11101c */
/* read as ONE byte by MT5725_fastcharge_change: "39405109 ldrb"@0xffffff8008acc8a8
 * e "39406109 ldrb"@0xffffff8008acc8d0 */
static u8 mt5725_g111014;		/* 0xffffff800a111014 */
static u8 mt5725_g111018;		/* 0xffffff800a111018 */
/*
 * written by MT5725_irq_handle and by NOBODY else; nobody reads it anywhere
 * in the image: "39008109 strb"@0xffffff8008accdfc
 */
u8 mt5725_g111020;			/* 0xffffff800a111020 */
static int mt5725_last_chrg_current;	/* 0xffffff800a111150 */
static struct input_dev *mt5725_input;	/* 0xffffff800a111158 */
static struct wakeup_source mt5725_ws;	/* 0xffffff800a111098 */
static int mt5725_rt_mode_pin;		/* 0xffffff800a111160 */

/* ------------------------------------------------------------------------ */
/*
 * MT5725_otp_write is `t` in the oracle map, that is STATIC, and it is
 * called by MT5725_write_otpok_flag, which PRECEDES it in the binary
 * (0xffffff8008acb4dc against 0xffffff8008acb53c): a forward declaration is
 * needed.
 */
static void MT5725_otp_write(u32 addr, u8 *buf, u32 len);


/*
 * HEADER DELTA (not done here, by the rule on shared files):
 * `mt_charger_set_opa_mode` is called by mt5725_set_otg, mt5725_get_otp and
 * MT5725_otp_process ("97ec1175 bl"@0xffffff8008ace4fc) but no ALPS header
 * declares it. As `ilitek_bus.c` already does for group E, it is declared
 * LOCALLY here and the delta is described in the report.
 */
extern int mt_charger_set_opa_mode(struct charger_device *chg_dev, bool en);

/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/*
 * A HISTORICAL NOTE, now moot. While MT5725_otp_write was not yet written,
 * MT5725_run_pgm_fw had no callers in this file and clang, at -O2, would
 * have deleted it: `__used` was needed to be able to MEASURE it, and it was
 * a declared compilation divergence. Now MT5725_otp_write exists and calls
 * it ("97fffee3 bl"@0xffffff8008acb584), so `__used` has been REMOVED and
 * the measurement no longer carries that margin.
 */

/*
 * MT5725_read_buffer -- 0xffffff8008acadf8, 32 bytes.
 * `reg` is a u16: the body masks it before widening it to `unsigned int`
 *   "12003c21 and"@0xffffff8008acae04
 * `len` is a 32-bit UNSIGNED: the widening to 64 bits is `mov w3,w3`,
 * not `sxtw`
 *   "2a0303e3 mov"@0xffffff8008acae08
 *   "97e90f78 bl"@0xffffff8008acae0c   (regmap_bulk_read)
 */
static int MT5725_read_buffer(struct mt5725_chip *chip, u16 reg, u8 *buf, u32 len)
{
	return regmap_bulk_read(chip->c24, reg, buf, len);
}

/*
 * MT5725_write -- 0xffffff8008acd8e0, 76 byte.
 *   "12003c21 and"@0xffffff8008acd8f4   (reg u16)
 *   "12001c42 and"@0xffffff8008acd8f8   (val u8)
 *   "97e8ff3d bl"@0xffffff8008acd8fc    (regmap_write)
 *   "36f800c0 tbz"@0xffffff8008acd904   (if (ret < 0))
 *   "MT5725 write error: %d\n"@0xffffff8009263201
 */
static int MT5725_write(struct mt5725_chip *chip, u16 reg, u8 val)
{
	int ret;

	ret = regmap_write(chip->c24, reg, val);
	if (ret < 0)
		dev_err(chip->c16, "MT5725 write error: %d\n", ret);

	return ret;
}

/*
 * MT5725_read -- 0xffffff8008acd87c, 100 bytes.
 * The canary is there because the address of the local ESCAPES in the call
 * (stack-protector-strong, CONFIG_CC_STACKPROTECTOR_STRONG=y in the factory).
 *   "97e9026e bl"@0xffffff8008acd8a8    (regmap_read)
 *   "37f80060 tbnz"@0xffffff8008acd8ac  (if (ret >= 0) *val = ...)
 *   "39000268 strb"@0xffffff8008acd8b4
 */
static int MT5725_read(struct mt5725_chip *chip, u16 reg, u8 *val)
{
	unsigned int v;
	int ret;

	ret = regmap_read(chip->c24, reg, &v);
	if (ret >= 0)
		*val = v;

	return ret;
}

/*
 * MT5725_write_buffer -- 0xffffff8008acabe8, 140 bytes.
 * The `chip->c40` function pointer is RE-READ on every round: the `ldr` sits
 * at the HEAD of the loop, not before it
 *   "f94016a8 ldr"@0xffffff8008acac10
 *   "39400282 ldrb"@0xffffff8008acac14
 *   "d63f0100 blr"@0xffffff8008acac20
 *   "34000223 cbz"@0xffffff8008acabfc    (len == 0 -> ret = 0)
 */
static int MT5725_write_buffer(struct mt5725_chip *chip, u16 reg, u8 *buf, u32 len)
{
	int ret = 0;
	u32 i;

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008aca904).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	i = 0;
	while (ret >= 0) {
		if (i == len)
			return ret;
		ret = chip->c40(chip, reg, *buf);
		i++;
		reg++;
		buf++;
	}
	dev_err(chip->c16, "MT5725 write error: %d\n", ret);

	return ret;
}


/*
 * c_acab34() was reconstructed from the factory kernel disassembly (0xffffff8008acab38).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int c_acab34(struct mt5725_chip *chip, u16 reg, u16 val)
{
	int ret;

	ret = chip->c40(chip, reg, val);
	if (ret >= 0)
		ret = chip->c40(chip, reg + 1, val >> 8);
	if (ret < 0)
		dev_err(chip->c16, "MT5725 write error: %d\n", ret);

	return ret;
}

/*
 * c_ace34c() was reconstructed from the factory kernel disassembly (0xffffff8008aca904).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int c_ace34c(struct mt5725_chip *chip, u16 reg, u8 *buf, u32 len)
{
	int ret = 0;
	u32 i;

	for (i = 0; i < len && ret >= 0; i++) {
		ret = chip->c40(chip, reg, *buf);
		reg++;
		buf++;
	}
	if (ret < 0)
		dev_err(chip->c16, "MT5725 write error: %d\n", ret);

	return ret;
}

/*
 * MT5725_soft_reset -- 0xffffff8008aca8d4, 136 bytes.
 * MT5725_write_buffer is INLINED here: the loop is the same, with len=1
 *   "7100069f cmp"@0xffffff8008aca904   (w20 against 1)
 *   "528a4001 mov"@0xffffff8008aca8f8   (register 0x5200)
 *   "321903e8 orr"@0xffffff8008aca8f4   (value 0x80)
 *   "52801900 mov"@0xffffff8008aca940   (msleep(200))
 * The local is NOT an array: were it one, stack-protector-strong would put
 * a canary there, and there is none.
 */
void MT5725_soft_reset(void)
{
	u8 val = 0x80;

	MT5725_write_buffer(mt5725_chip_p, 0x5200, &val, 1);
	msleep(200);
}

/*
 * mt5725_reverse_charge() was reconstructed from the factory kernel disassembly (0xffffff8008aca95c, 424 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void mt5725_reverse_charge(bool en)
{
	if (en) {
		gpio_set_value(mt5725_rt_mode_pin, 1);
		if (usb_insert_stato) {
			MT5725_soft_reset();
		} else {
			mt_charger_set_opa_mode(mt5725_chg_dev, 1);
			charger_dev_set_boost_current_limit(mt5725_chg_dev,
							    2200000);
		}
		if (!mt5725_ws.active)
			__pm_stay_awake(&mt5725_ws);
		mt5725_rvs_online = 1;
	} else {
		gpio_set_value(mt5725_rt_mode_pin, 0);
		if (usb_insert_stato) {
			MT5725_soft_reset();
		} else {
			mt_charger_set_opa_mode(mt5725_chg_dev, 0);
			charger_dev_set_boost_current_limit(mt5725_chg_dev,
							    1500000);
		}
		__pm_relax(&mt5725_ws);
		mt5725_rvs_online = 0;
	}
}

/*
 * MT5725_power_status -- 0xffffff8008acab04, 8 byte.
 *   "320003e0 orr"@0xffffff8008acab04   (w0 = 1)
 */
int MT5725_power_status(void)
{
	return 1;
}

/*
 * is_reverse_charger_online -- 0xffffff8008acab0c, 12 byte.
 * Reads the global 0xffffff800a110fec at full width
 *   "b94fed00 ldr"@0xffffff8008acab10
 */
int is_reverse_charger_online(void)
{
	return mt5725_rvs_online;
}

/*
 * download_code -- 0xffffff8008acae18, 4 bytes: a single `ret`.
 *   "d65f03c0 ret"@0xffffff8008acae18
 */
void download_code(void)
{
}

/*
 * updata_wireless_online -- 0xffffff8008acd2a4, 4 bytes: a single `ret`.
 *   "d65f03c0 ret"@0xffffff8008acd2a4
 */
void updata_wireless_online(void)
{
}

/*
 * mt5725_get_hwen -- 0xffffff8008ace6f0, 8 bytes: returns 0.
 *   "aa1f03e0 mov"@0xffffff8008ace6f0
 */
static ssize_t mt5725_get_hwen(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	return 0;
}

/*
 * SizeofFskPkt() was reconstructed from the factory kernel disassembly (0xffffff8008acbeac, 88 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
u8 SizeofFskPkt(u8 header)
{
	if (header < 0x20)
		return 1;
	else if (header < 0x80)
		return ((header - 0x20) >> 4) + 2;
	else if (header < 0xe0)
		return ((header - 0x80) >> 3) + 8;
	else
		return ((header - 0xe0) >> 2) + 20;
}


/*
 * MT5725_send_ppp() was reconstructed from the factory kernel disassembly (0xffffff8008acbbe0, 312 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void MT5725_send_ppp(u8 *data)
{
	u16 reg_cmd = 0x0001;

	c_ace34c(mt5725_chip_p, 0x0040, data, SizeofFskPkt(data[0]) + 1);
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
}
EXPORT_SYMBOL(MT5725_send_ppp);

/*
 * fast_vfc() was reconstructed from the factory kernel disassembly (0xffffff8008acab18, 208 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void fast_vfc(u16 vfc)
{
	u16 reg_cmd = 0x0080;

	c_acab34(mt5725_chip_p, 0x0022, vfc);
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
	printk("%s,write reg_cmd : 0x%04x,\n", __func__, reg_cmd);
}


/*
 * fastcharge_afc -- 0xffffff8008acac74, 388 bytes.
 *   "394007e8 ldrb"@0xffffff8008acacac / "36000908 tbz"@0xffffff8008acacb0
 *                                        (bit 0 of the second byte of register 0x0c)
 *   "52800502 mov"@0xffffff8008acacd0   (0x28) / "52800462 mov"@0xffffff8008acacec (0x23)
 *                                        -> 0x2328 into registers 0x22/0x23
 *   "321903e2 orr"@0xffffff8008acad18   (0x80 into register 0x0e)
 *   "52801402 mov"@0xffffff8008acad78   (0xa0 into register 0x06)
 *   "MT5725 %s ,version 0.1 Tx support samsung_afc\n"@0xffffff8009262d3f
 *   "fastcharge_afc"@0xffffff8009262d6e
 *   "%s,version 0.1 write reg_clr : 0x%04x,\n"@0xffffff8009262d7d
 *   "%s,version 0.1 write reg_cmd : 0x%04x,\n"@0xffffff8009262da5
 */
void fastcharge_afc(void)
{
	u8 buf[8];
	u16 vfc, reg_clr, reg_cmd;

	MT5725_read_buffer(mt5725_chip_p, 0x000c, buf, 2);
	if (buf[1] & 0x01) {
		printk("MT5725 %s ,version 0.1 Tx support samsung_afc\n",
		       __func__);
		vfc = 0x2328;
		c_acab34(mt5725_chip_p, 0x0022, vfc);
		reg_clr = 0x0080;
		c_acab34(mt5725_chip_p, 0x000e, reg_clr);
		printk("%s,version 0.1 write reg_clr : 0x%04x,\n", __func__,
		       reg_clr);
		reg_cmd = 0x00a0;
		c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
		printk("%s,version 0.1 write reg_cmd : 0x%04x,\n", __func__,
		       reg_cmd);
	}
}

/*
 * MT5725_send_EPT -- 0xffffff8008acbd18, 404 bytes.
 * FOUR MT5725_write_buffer calls inlined. The three on 16-bit locals are
 * UNROLLED by the compiler; the one on the two-byte array stays a loop
 *   "71000adf cmp"@0xffffff8008acbdf8
 * -- it is the same asymmetry described among the open divergences.
 *   "321c03e2 orr"@0xffffff8008acbd4c   (0x10 into register 0x0e)
 *   "321b03e2 orr"@0xffffff8008acbd94   (0x20 into register 0x06)
 *   "52800c80 mov"@0xffffff8008acbdd4   (msleep(100))
 *   "390037f3 strb"@0xffffff8008acbde8  (buf[1] = ept)
 *   "390033e8 strb"@0xffffff8008acbdf4  (buf[0] = 0x02)
 *   "321a03e1 orr"@0xffffff8008acbdec   (first register 0x40)
 *   "320003e2 orr"@0xffffff8008acbe3c   (0x01 into register 0x06)
 */
void MT5725_send_EPT(u8 ept)
{
	u16 reg_clr, reg_cmd;
	u8 buf[12];

	reg_clr = 0x0010;
	c_acab34(mt5725_chip_p, 0x000e, reg_clr);
	reg_cmd = 0x0020;
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
	msleep(100);
	buf[0] = 0x02;
	buf[1] = ept;
	MT5725_write_buffer(mt5725_chip_p, 0x0040, buf, 2);
	reg_cmd = 0x0001;
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
}

/*
 * MT5725_fsk_handle() was reconstructed from the factory kernel disassembly (0xffffff8008acbf04, 1020 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void MT5725_fsk_handle(void)
{
	u8 data[100];
	u8 tx[12];
	u8 buf[8];
	u8 header, size, i;
	u16 reg_clr, reg_cmd;

	MT5725_read_buffer(mt5725_chip_p, 0x0056, buf, 1);
	header = buf[0];
	size = SizeofFskPkt(header);
	MT5725_read_buffer(mt5725_chip_p, 0x0056, buf, 1);

	for (i = 0; i < size; i++) {
		MT5725_read_buffer(mt5725_chip_p, 0x0057, buf, 1);
		data[i] = buf[0];
	}

	if (header != 0x1f)
		return;

	if (data[0] == 0xee) {
		reg_clr = 0x0010;
		c_acab34(mt5725_chip_p, 0x000e, reg_clr);
		reg_cmd = 0x0020;
		c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
		msleep(100);
		tx[0] = 0x58;
		MT5725_read_buffer(mt5725_chip_p, 0x001a, buf, 2);
		tx[1] = 0xee;
		tx[2] = buf[1];
		tx[3] = buf[0];
		MT5725_read_buffer(mt5725_chip_p, 0x0018, buf, 2);
		tx[4] = buf[1];
		tx[5] = buf[0];
		MT5725_write_buffer(mt5725_chip_p, 0x0040, tx, 6);
	} else if (data[0] == 0xdd) {
		reg_clr = 0x0010;
		c_acab34(mt5725_chip_p, 0x000e, reg_clr);
		reg_cmd = 0x0020;
		c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
		msleep(100);
		tx[0] = 0x58;
		MT5725_read_buffer(mt5725_chip_p, 0x0004, buf, 2);
		tx[1] = 0xdd;
		tx[2] = buf[1];
		tx[3] = buf[0];
		MT5725_read_buffer(mt5725_chip_p, 0x0003, buf, 2);
		tx[4] = buf[1];
		tx[5] = buf[0];
		MT5725_write_buffer(mt5725_chip_p, 0x0040, tx, 6);
	} else {
		return;
	}

	reg_cmd = 0x0001;
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
	printk("ATE send ASK TO TX\n");
}

/*
 * MT5725_rx_sram_updata() was reconstructed from the factory kernel disassembly (0xffffff8008acc300, 700 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * 0xffffff8008f85f80. The field of zeroes runs up to 0xffffff8008f86048, where
 * the `of_device_id` begins: 200 bytes in all. The loop reads 100 of them
 * ("710192bf cmp"@0xffffff8008acc360), and 100 is therefore the only MEASURED
 * size; if the factory object were 200 bytes there would be no telling,
 * because the rest is zero like the whole field. The choice of 100 is declared.
 */
static const u8 mt5725_rx_sram[100] = { 0 };

void MT5725_rx_sram_updata(void)
{
	u8 buf[8];

	MT5725_read_buffer(mt5725_chip_p, 0x0004, buf, 2);
	if (buf[1] & ~0x10) {
		MT5725_write_buffer(mt5725_chip_p, 0x0600,
				    (u8 *)mt5725_rx_sram, 100);
		buf[0] = 0x25;
		buf[1] = 0x57;
		c_ace34c(mt5725_chip_p, 0x0008, buf, 2);
		buf[0] = 0x40;
		buf[1] = 0x00;
		c_ace34c(mt5725_chip_p, 0x0006, buf, 2);
		msleep(4);
	} else {
		buf[0] = 0x25;
		buf[1] = 0x57;
		c_ace34c(mt5725_chip_p, 0x0008, buf, 2);
		buf[0] = 0x40;
		buf[1] = 0x00;
		c_ace34c(mt5725_chip_p, 0x0006, buf, 2);
		msleep(2);
		MT5725_write_buffer(mt5725_chip_p, 0x0600,
				    (u8 *)mt5725_rx_sram, 100);
	}
	buf[0] = 0x00;
	buf[1] = 0x00;
	c_ace34c(mt5725_chip_p, 0x0006, buf, 2);
}
EXPORT_SYMBOL(MT5725_rx_sram_updata);

/*
 * MT5725_enable_afc -- 0xffffff8008acc5bc, 140 bytes.
 *   "52800848 mov"@0xffffff8008acc5dc   (0x0042 -> registers 0x38, 0x39)
 *   "321d0be1 orr"@0xffffff8008acc5e0   (register 0x38)
 *   "790013e8 strh"@0xffffff8008acc5e8  (a single `strh`: the local is NOT an
 *                                        array, otherwise there would be the
 *                                        stack-protector-strong canary)
 *   "2078MT5725_enable_afc\n"@0xffffff8009262e81
 */
void MT5725_enable_afc(void)
{
	u16 val = 0x0042;

	MT5725_write_buffer(mt5725_chip_p, 0x0038, (u8 *)&val, 2);
	printk("2078MT5725_enable_afc\n");
}

/*
 * MT5725_enable_apple -- 0xffffff8008acc648, 128 bytes. No printk.
 *   "52800b48 mov"@0xffffff8008acc668   (0x005a)
 *   "321d0be1 orr"@0xffffff8008acc66c   (register 0x38)
 */
void MT5725_enable_apple(void)
{
	u16 val = 0x005a;

	MT5725_write_buffer(mt5725_chip_p, 0x0038, (u8 *)&val, 2);
}

/*
 * MT5725_enable_epp -- 0xffffff8008acc6c8, 128 bytes. No printk.
 *   "529dddc8 mov"@0xffffff8008acc6e8   (0xeeee)
 *   "321d03e1 orr"@0xffffff8008acc6ec   (register 0x08)
 */
void MT5725_enable_epp(void)
{
	u16 val = 0xeeee;

	MT5725_write_buffer(mt5725_chip_p, 0x0008, (u8 *)&val, 2);
}

/*
 * MT5725_SetFodPara -- 0xffffff8008accc00, 116 bytes.
 * The 16 bytes of FOD parameters live in `.data` at 0xffffff80099a74c8, that
 * is WRITABLE (right before the struct i2c_driver): they are not `const`.
 *   "911322b5 add"@0xffffff8008accc24   (address 0x…99a74c8)
 *   "7100429f cmp"@0xffffff8008accc28   (16 bytes)
 *   "52800d01 mov"@0xffffff8008accc20   (first register 0x68)
 */
static u8 mt5725_fod_para[16] = {
	0x32, 0x51, 0x31, 0x42, 0x31, 0x3f, 0x31, 0x40,
	0x30, 0x41, 0x31, 0x2c, 0x0a, 0x0a, 0x00, 0x00,
};

void MT5725_SetFodPara(void)
{
	MT5725_write_buffer(mt5725_chip_p, 0x0068, mt5725_fod_para, 16);
}


/*
 * MT5725_run_pgm_fw() was reconstructed from the factory kernel disassembly (0xffffff8008acb110, 972 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void MT5725_run_pgm_fw(void)
{
	u8 val;
	u16 val16;
	u32 offset = 0;
	u32 remaining = sizeof(MT5725_pgm_fw);
	u32 chunk;

	val = 0x95;
	MT5725_write_buffer(mt5725_chip_p, 0x5808, &val, 1);
	MT5725_write_buffer(mt5725_chip_p, 0x5808, &val, 1);
	MT5725_write_buffer(mt5725_chip_p, 0x5808, &val, 1);
	val = 0x01;
	MT5725_write_buffer(mt5725_chip_p, 0x5800, &val, 1);
	val = 0x57;
	MT5725_write_buffer(mt5725_chip_p, 0x5244, &val, 1);
	val16 = 0x420f;
	c_acab34(mt5725_chip_p, 0x5218, val16);
	val = 0x0e;
	MT5725_write_buffer(mt5725_chip_p, 0x5208, &val, 1);
	msleep(200);

	printk("len:%d\n", (int)sizeof(MT5725_pgm_fw));
	do {
		chunk = (remaining < 128) ? remaining : 128;
		printk("length:%d\n", chunk);
		c_ace34c(mt5725_chip_p, 0x800 + offset, (u8 *)&MT5725_pgm_fw[offset], chunk);
		offset += chunk;
		printk("offset:%d\n", offset);
		remaining -= chunk;
		msleep(10);
	} while (remaining);

	msleep(200);
	val = 0x8e;
	MT5725_write_buffer(mt5725_chip_p, 0x5200, &val, 1);
	msleep(200);
}


/*
 * MT5725_otp_read -- 0xffffff8008acae1c, 756 bytes.
 * It reads the OTP in blocks of 128 bytes. The fourth parameter is NOT used by
 * the body: no instruction touches w3.
 *   "1101fc49 add"@0xffffff8008acae44   (len + 127)
 *   "53077d34 lsr"@0xffffff8008acae50   (>>7: number of blocks)
 *   "12196135 and"@0xffffff8008acae58   (&~0x7f: rounded length)
 *   "d379e388 lsl"@0xffffff8008acae9c   (block * 128)
 *   "321903e8 orr"@0xffffff8008acaf40   (0x0080 into register 4)
 *   "321d03e8 orr"@0xffffff8008acaf90   (0x08 into register 0, ONE byte only:
 *                                        "7100043f cmp"@0xffffff8008acaf9c)
 *   "7100211f cmp"@0xffffff8008acaffc   (status == 8: still busy)
 *   "7100091f cmp"@0xffffff8008acb040   (status == 2: ready)
 *   "7107d35f cmp"@0xffffff8008acb034   (500 rounds of waiting)
 *   "321903e3 orr"@0xffffff8008acb05c   (128 bytes read from register 8)
 *   "f102029f cmp"@0xffffff8008acb084   (128 bytes printed)
 *   "MT5725_run_pgm_fw,length:%d\n"@0xffffff8009262dcd
 *   "5725status:%d"@0xffffff8009262dea
 *   "error! Read OTP TImeout\n"@0xffffff8009262df8
 *   "OtpRead:"@0xffffff8009262e11
 *   ",%x"@0xffffff8009262e1a
 *   "OtpRead l670 error\n"@0xffffff8009262e1e
 *   "\n"@0xffffff8009245994
 */
int MT5725_otp_read(u32 addr, u8 *buf, u32 len, int flag)
{
	union mt5725_u16 temp;
	u32 blocks = (len + 127) / 128;
	u32 i, j;

	/*
	 * volatile() was reconstructed from the factory kernel disassembly (0xffffff8008acafe4).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	asm volatile("" :: "r"(flag));

	MT5725_run_pgm_fw();
	printk("MT5725_run_pgm_fw,length:%d\n", (len + 127) & ~0x7f);

	for (i = 0; i < blocks; i++) {
		temp.c0 = 0;
		MT5725_write_buffer(mt5725_chip_p, 0x0000, temp.ptr, 2);
		temp.c0 = addr + i * 128;
		MT5725_write_buffer(mt5725_chip_p, 0x0002, temp.ptr, 2);
		temp.c0 = 0x0080;
		MT5725_write_buffer(mt5725_chip_p, 0x0004, temp.ptr, 2);
		temp.ptr[0] = 0x08;
		MT5725_write_buffer(mt5725_chip_p, 0x0000, temp.ptr, 1);

		msleep(50);
		MT5725_read_buffer(mt5725_chip_p, 0x0000, temp.ptr, 2);

		j = 0;
		while (temp.ptr[0] == 8) {
			msleep(50);
			MT5725_read_buffer(mt5725_chip_p, 0x0000, temp.ptr, 2);
			printk("5725status:%d", temp.c0);
			j++;
			if (j > 500) {
				printk("error! Read OTP TImeout\n");
				return 0;
			}
		}
		if (temp.ptr[0] != 2) {
			printk("OtpRead l670 error\n");
			return 0;
		}

		printk("OtpRead:");
		MT5725_read_buffer(mt5725_chip_p, 0x0008, buf + i * 128, 128);
		for (j = 0; j < 128; j++)
			printk(",%x", buf[i * 128 + j]);
		printk("\n");
	}

	return 1;
}

/*
 * MT5725_write_otpok_flag -- 0xffffff8008acb4dc, 96 bytes.
 *   "53087c0a lsr"@0xffffff8008acb4f4   (addr >> 8, LOGICAL shift)
 *   "7100dd5f cmp"@0xffffff8008acb4fc   (>= 0x37)
 *   "5294ab49 mov"@0xffffff8008acb4f0   (0xa55a)
 *   "72b4ab49 movk"@0xffffff8008acb4f8  (0xa55a in the high half)
 *   "321e03e2 orr"@0xffffff8008acb510   (4 bytes)
 * The canary is there because the address of the local ESCAPES into
 * MT5725_otp_write.
 */
void MT5725_write_otpok_flag(u32 addr)
{
	u32 flag = 0xa55aa55a;

	if ((addr >> 8) >= 0x37)
		MT5725_otp_write(addr, (u8 *)&flag, 4);
}

/*
 * MT5725_otp_write() was reconstructed from the factory kernel disassembly (0xffffff8008acb53c, 1156 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void MT5725_otp_write(u32 addr, u8 *buf, u32 len)
{
	struct {
		u16	c0;
		u16	c2;
		u16	c4;
		u16	c6;
		u8	c8[256];
	} pkt;
	union mt5725_u16 temp;
	u32 offset = 0;
	u32 remaining = len;
	u32 retries = 8;
	u32 i;
	u32 zeros;
	int k;
	u16 count, sum, cur, eff;

	printk("MT5725_run_pgm_fw,size:%d\n", len);
	MT5725_run_pgm_fw();

	while (remaining) {
		cur = addr + offset;
		if (remaining < 0x81)
			count = remaining;
		else
			count = 0x80;

		pkt.c0 = 0;
		pkt.c2 = cur;
		pkt.c4 = count;
		pkt.c6 = cur;

		sum = cur;
		for (i = 0; i < count; i++) {
			pkt.c8[i] = buf[offset + i];
			sum += pkt.c8[i];
		}
		pkt.c6 = sum;

		zeros = 0;
		for (k = count - 1; k > 0; k--) {
			if (pkt.c8[k])
				break;
			zeros++;
		}

		if (zeros != count) {
			eff = count - zeros;
			pkt.c4 = eff;
			pkt.c6 = sum + eff;

			temp.c0 = 0x0000;
			MT5725_write_buffer(mt5725_chip_p, 0x0000, temp.ptr, 2);
			temp.c0 = cur;
			MT5725_write_buffer(mt5725_chip_p, 0x0002, temp.ptr, 2);
			temp.c0 = eff;
			MT5725_write_buffer(mt5725_chip_p, 0x0004, temp.ptr, 2);
			temp.c0 = sum + eff;
			MT5725_write_buffer(mt5725_chip_p, 0x0006, temp.ptr, 2);
			MT5725_write_buffer(mt5725_chip_p, 0x0008, pkt.c8, eff);
			temp.ptr[0] = 0x01;
			MT5725_write_buffer(mt5725_chip_p, 0x0000, temp.ptr, 1);

			msleep(100);
			MT5725_read_buffer(mt5725_chip_p, 0x0000, temp.ptr, 1);
			i = 0;
			while (temp.ptr[0] == 0x01) {
				msleep(100);
				MT5725_read_buffer(mt5725_chip_p, 0x0000,
						   temp.ptr, 1);
				printk("otp write status:%d\n", temp.ptr[0]);
				i++;
				if (i > 500)
					return;
			}

			if (temp.ptr[0] == 0x02) {
				printk("PGM_STATUS_PROGOK\n");
			} else if (temp.ptr[0] == 0x04) {
				if (retries == 0) {
					printk("PGM_STATUS_ERRCS\n");
					return;
				}
				printk("Find ERRCS\n");
				retries--;
				continue;
			} else if (temp.ptr[0] == 0x20) {
				if (retries == 0) {
					printk("PGM_STATUS_ERRPGM\n");
					return;
				}
				printk("Find ERRPGM\n");
				retries--;
				continue;
			} else {
				if (retries == 0) {
					printk("PGM_STATUS_NUKNOWN\n");
					return;
				}
				printk("Find NUKNOWN\n");
				retries--;
				continue;
			}
		}

		remaining -= count;
		offset += count;
	}
}

/*
 * MT5725_otp_process() was reconstructed from the factory kernel disassembly (0xffffff8008acb9c0, 544 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int MT5725_otp_process(void)
{
	u8 buf[128];
	u8 *rd;
	size_t i;

	/* ---- MT5725_otp_write_check, inlined ---- */
	mt_charger_set_opa_mode(mt5725_chg_dev, 1);
	msleep(50);
	MT5725_otp_read(0x3d00, buf, 4, 1);
	if (buf[0] == 0x5a && buf[1] == 0xa5 &&
	    buf[2] == 0x5a && buf[3] == 0xa5) {
		printk("MT5725 OTP Write");
		printk("MT5725_otp_write_check Done\n");
		mt_charger_set_opa_mode(mt5725_chg_dev, 0);
		printk("mt_charger_set_opa_mode done\n");
		mt5725_chip_q->c820 = 5;
		return 1;
	}
	printk("MT5725 OTP NOT Write");
	/* ---- fine MT5725_otp_write_check ---- */

	mt5725_chip_q->c820 = 3;
	MT5725_otp_write(0, (u8 *)MT5725_otp_fw, sizeof(MT5725_otp_fw));
	mt5725_chip_q->c820 = 4;

	/*
	 * NOT sizeof(MT5725_otp_fw): the binary asks for 0x3b00 = 15104 bytes,
	 * 80 more than the firmware -- "52876000 mov"@0xffffff8008acbab8
	 */
	rd = kmalloc(0x3b00, GFP_KERNEL);
	if (!rd) {
		printk("devm_kzalloc Error\n");
		mt_charger_set_opa_mode(mt5725_chg_dev, 0);
		mt5725_chip_q->c820 = 6;
		return 0;
	}

	mt_charger_set_opa_mode(mt5725_chg_dev, 0);
	msleep(100);
	mt_charger_set_opa_mode(mt5725_chg_dev, 1);
	msleep(50);
	MT5725_otp_read(0, rd, sizeof(MT5725_otp_fw), 1);

	for (i = 0; i < sizeof(MT5725_otp_fw); i++) {
		if (MT5725_otp_fw[i] != rd[i]) {
			printk("MT5725_otp_verify FALSE");
			mt_charger_set_opa_mode(mt5725_chg_dev, 0);
			mt5725_chip_q->c820 = 6;
			return 0;
		}
	}

	kfree(rd);
	printk("MT5725_otp_verify TRUE");
	*(u32 *)buf = 0xa55aa55a;
	MT5725_otp_write(0x3d00, buf, 4);
	mt_charger_set_opa_mode(mt5725_chg_dev, 0);
	mt5725_chip_q->c820 = 1;

	return 1;
}
/*
 * MT5725_check_chargetimeout -- 0xffffff8008accb88, 120 bytes.
 *   "97f87595 bl"@0xffffff8008accb98    (battery_get_vbus)
 *   "713e7e7f cmp"@0xffffff8008accbc0   (vbus against 3999)
 *   "54000168 b.hi"@0xffffff8008accbc4  (an UNSIGNED comparison: the variable
 *                                        is unsigned, not the `signed int`
 *                                        battery_get_vbus declares)
 *   "397fe2a3 ldrb"@0xffffff8008accba8  (the state is read as ONE byte: it is
 *                                        `static` and narrowed)
 *   "FastChargeTimeout=%d, vbus=%d,FastChargeState:%d\n"@0xffffff8009262f26
 *   "FastChargeTimeout=0, vbus=%d\n"@0xffffff8009262f58
 */
void MT5725_check_chargetimeout(void)
{
	unsigned int vbus = battery_get_vbus();

	printk("FastChargeTimeout=%d, vbus=%d,FastChargeState:%d\n",
	       mt5725_fastcharge_timeout, vbus, mt5725_fastcharge_state);

	if (vbus < 4000) {
		if (mt5725_fastcharge_timeout) {
			mt5725_fastcharge_timeout--;
		} else {
			mt5725_fastcharge_state = 0;
			printk("FastChargeTimeout=0, vbus=%d\n", vbus);
		}
	}
}

/*
 * ech_wls_set_chrg_current() was reconstructed from the factory kernel disassembly (0xffffff8008acd180, 184 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ech_wls_set_chrg_current(int cur_max, int cur_now)
{
	union power_supply_propval val;
	struct power_supply *psy;

	psy = power_supply_get_by_name("wireless");
	if (!psy) {
		pr_info("%s: get power supply failed\n", __func__);
		return;
	}

	if (mt5725_last_chrg_current != cur_max) {
		val.intval = cur_max;
		power_supply_set_property(psy, POWER_SUPPLY_PROP_CURRENT_MAX, &val);
		val.intval = cur_now;
		power_supply_set_property(psy, POWER_SUPPLY_PROP_CURRENT_NOW, &val);
		mt5725_last_chrg_current = cur_max;
	}
}

/*
 * wls_get_online -- 0xffffff8008acd238, 56 byte.
 *   "\x016wls_work_online1 = %d.\n"@0xffffff8009263187
 *   "b94fe661 ldr"@0xffffff8008acd248   (0xffffff800a110fe4, full width)
 *   "1a9f07e0 cset"@0xffffff8008acd264  (returns 0/1)
 */
int wls_get_online(void)
{
	pr_info("wls_work_online1 = %d.\n", mt5725_wls_online);
	return mt5725_wls_online != 0;
}

/*
 * rvs_get_online -- 0xffffff8008acd270, 52 bytes.
 * A FACTORY DEFECT REPRODUCED: it prints the global `wls` and returns the
 * global `rvs`, which are two different variables.
 *   "b94fe501 ldr"@0xffffff8008acd27c   (prints 0xffffff800a110fe4)
 *   "b94fed08 ldr"@0xffffff8008acd290   (returns 0xffffff800a110fec)
 */
int rvs_get_online(void)
{
	pr_info("wls_work_online1 = %d.\n", mt5725_wls_online);
	return mt5725_rvs_online != 0;
}

/*
 * reverse_charger_rxdetect -- 0xffffff8008acd2a8, 152 bytes.
 *   "97fdea59 bl"@0xffffff8008acd2b4    (input_allocate_device)
 *   "940e2708 bl"@0xffffff8008acd2cc    (set_bit -- a REAL function on arm64,
 *                                        not the inline `__set_bit`)
 *   "52800ae0 mov"@0xffffff8008acd2d4   (key code 87)
 *   "52800328 mov"@0xffffff8008acd2e8   (id.bustype = 0x19 = BUS_HOST)
 *   "12800173 mov"@0xffffff8008acd31c   (-12 = -ENOMEM)
 *   "RXDETECT"@0xffffff80092631be
 *   "\x016alloc input device failed\n"@0xffffff80092631a1
 *   "\x016input device register failed\n"@0xffffff80092631c7
 *   "\x016input register success\n"@0xffffff80092631e7
 */
int reverse_charger_rxdetect(void)
{
	int ret;

	mt5725_input = input_allocate_device();
	if (!mt5725_input) {
		pr_info("alloc input device failed\n");
		return -ENOMEM;
	}

	set_bit(EV_KEY, mt5725_input->evbit);
	/* 87: the binary does not name it, it is read as a constant */
	set_bit(87, mt5725_input->keybit);
	mt5725_input->id.bustype = BUS_HOST;
	mt5725_input->name = "RXDETECT";

	ret = input_register_device(mt5725_input);
	if (ret)
		pr_info("input device register failed\n");
	else
		pr_info("input register success\n");

	return ret;
}


/*
 * MT5725_irq_handle() was reconstructed from the factory kernel disassembly (0xffffff8008accc74, 1292 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void MT5725_irq_handle(void)
{
	union mt5725_u16 temp;
	union mt5725_u16 chip_id;
	u8 version[2];
	u16 int_flag;
	u16 reg_cmd;
	u16 epp;
	int ret, gpio;

	printk("----------------MT5725_delayed_work low interrupt-----------------------\n");

	ret = MT5725_read_buffer(mt5725_chip_p, 0x000c, temp.ptr, 2);
	gpio = gpio_get_value(mt5725_chip_p->c72);
	if (ret < 0 || gpio)
		temp.c0 = 0;
	int_flag = temp.c0;
	printk("%s ,version 0.1 little-endian val:0x%04x\n", __func__, int_flag);

	if (temp.c0 & 0x8000) {
		mt5725_g11101c = 0;
		ech_wls_set_chrg_current(200, 200);
		printk("MT5725 %s , INTFALG_PowerON\n", __func__);
	}
	if (temp.c0 & 0x0040) {
		mt5725_good_irq_ready = 1;
		printk("MT5725 %s , LDO ON\n", __func__);
	}
	if (temp.c0 & 0x0008) {
		mt5725_g111020 = 1;
		printk("MT5725 %s , MT5725 is Ready\n", __func__);
		mt5725_g111024[0] = 100;
		schedule_delayed_work(&mt5725_chip_q->c216, 0);

		MT5725_read_buffer(mt5725_chip_p, 0x0000, chip_id.ptr, 2);
		printk("MT5725 %s , 0x%x%x\n", __func__, chip_id.ptr[1],
		       chip_id.ptr[0]);
		if (((chip_id.ptr[1] << 8) | chip_id.ptr[0]) == 0x5725)
			printk("MT5725 ID Correct query\n");
		else
			printk("MT5725 ID error :%d\n ", chip_id.c0);

		MT5725_read_buffer(mt5725_chip_p, 0x0004, version, 2);
		printk("MT5725 chip_version old : 0x%02x,0x%02x\n",
		       version[1], version[0]);
	}
	if (temp.c0 & 0x0080)
		printk("MT5725 %s , MT5725 LDO  off\n", __func__);
	if (temp.c0 & 0x0100) {
		mt5725_g111014 = 1;
		printk("MT5725 %s ,version 0.1 Tx support samsung_afc\n",
		       __func__);
		mt5725_g11101c = 6;
	}
	if (temp.c0 & 0x0200) {
		mt5725_g111018 = 1;
		epp = 0x2328;
		c_acab34(mt5725_chip_p, 0x001e, epp);
		printk("MT5725 %s ,version 0.1 Tx support INT_EPP\n", __func__);
		reg_cmd = 0x0420;
	} else {
		reg_cmd = 0x0020;
	}

	if ((temp.c0 & 0x0100) && mt5725_rvs_online == 1) {
		mt5725_chip_q->c80 = 1;
		printk("MT5725 %s ,rxdetect: INT_POWER_TRANS\n", __func__);
	}
	if ((temp.c0 & 0x0040) && mt5725_rvs_online == 1) {
		mt5725_chip_q->c80 = 0;
		printk("MT5725 %s ,rxdetect: INT_REMOVE_POWER\n", __func__);
	}
	if (temp.c0 & 0x0010) {
		MT5725_fsk_handle();
		printk("MT5725 %s , FSK successfully  off\n", __func__);
	}

	c_acab34(mt5725_chip_p, 0x000e, int_flag);
	printk("%s,version 0.1 write reg_clr : 0x%04x,\n", __func__, int_flag);
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
	printk("%s,version 0.1 write reg_cmd : 0x%04x,\n", __func__, reg_cmd);

	if (!gpio_get_value(mt5725_chip_p->c72)) {
		MT5725_read_buffer(mt5725_chip_p, 0x000c, temp.ptr, 2);
		if (temp.c0 != 0 && temp.c0 != 0xffff &&
		    battery_get_vbus() > 4000)
			schedule_delayed_work(&mt5725_chip_q->c120, 13);
	}

	enable_irq(mt5725_irq_num);
}

/*
 * MT5725_int_delayed_work_func -- 0xffffff8008acd92c, 20 bytes.
 *   "97fffcd0 bl"@0xffffff8008acd934    (MT5725_irq_handle)
 * The `work` parameter is unused.
 */
static void MT5725_int_delayed_work_func(struct work_struct *work)
{
	MT5725_irq_handle();
}

/*
 * MT5725_irq -- 0xffffff8008acdd18, 68 bytes.
 *   "b9407100 ldr"@0xffffff8008acdd28   (0xffffff800a111070, the IRQ number)
 *   "97d9b383 bl"@0xffffff8008acdd30    (disable_irq_nosync)
 *   "321d03e0 orr"@0xffffff8008acdd40   (cpu = 8 = WORK_CPU_UNBOUND with
 *                                        CONFIG_NR_CPUS=8: it is what
 *                                        schedule_delayed_work expands to)
 *   "9101e262 add"@0xffffff8008acdd3c   (&chip->c120, offset 0x78)
 *   "320003e0 orr"@0xffffff8008acdd50   (IRQ_HANDLED)
 */
static irqreturn_t MT5725_irq(int irq, void *data)
{
	struct mt5725_chip *chip = data;

	disable_irq_nosync(mt5725_irq_num);
	schedule_delayed_work(&chip->c120, 0);

	return IRQ_HANDLED;
}

/*
 * MT5725_good_irq -- 0xffffff8008acdd5c, 96 byte.
 *   "b94fe908 ldr"@0xffffff8008acdd60   (0xffffff800a110fe8 == 1?)
 *   "b94fed08 ldr"@0xffffff8008acdd70   (0xffffff800a110fec == 0?)
 *   "b9407500 ldr"@0xffffff8008acdd88   (0xffffff800a111074, second IRQ)
 *   "9104e262 add"@0xffffff8008acdd9c   (&chip->c312, offset 0x138)
 */
static irqreturn_t MT5725_good_irq(int irq, void *data)
{
	struct mt5725_chip *chip = data;

	if (mt5725_good_irq_ready == 1 && !mt5725_rvs_online) {
		disable_irq_nosync(mt5725_good_irq_num);
		schedule_delayed_work(&chip->c312, 0);
	}

	return IRQ_HANDLED;
}

/*
 * fast_charging_store -- 0xffffff8008acdf6c, 140 bytes.
 *   "97e5d92f bl"@0xffffff8008acdf98    (kstrtouint, base 10)
 *   "79400be0 ldrh"@0xffffff8008acdfa8  (only 16 bits of the value are needed:
 *                                        it is the load narrowing clang
 *                                        applies when the use is a u16)
 *   "5289c428 mov"@0xffffff8008acdfac   (20001)
 *   "MT5725 Parameter error\n"@0xffffff8009263643
 */
static ssize_t fast_charging_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	unsigned int val;
	int ret;

	ret = kstrtouint(buf, 10, &val);
	if (ret)
		return ret;

	if ((u16)val >= 20001)
		printk("MT5725 Parameter error\n");
	else
		fast_vfc(val);

	return count;
}

/*
 * get_reg() was reconstructed from the factory kernel disassembly (0xffffff8008acdff8, 212 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const u16 mt5725_reg_tab[7][2] = {
	{ 0x0000, 0x0001 }, { 0x001a, 0x0001 }, { 0x000c, 0x0001 },
	{ 0x000e, 0x0002 }, { 0x001e, 0x0003 }, { 0x0022, 0x0003 },
	{ 0x0006, 0x0007 },
};

static ssize_t get_reg(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	u8 val[2];
	ssize_t len = 0;
	int i;

	for (i = 0; i < 7; i++) {
		if (i == 3)
			continue;
		MT5725_read_buffer(mt5725_chip_p, mt5725_reg_tab[i][0], val, 2);
		len += snprintf(buf + len, PAGE_SIZE - len,
				"reg:0x%02x=0x%02x%02x\n",
				mt5725_reg_tab[i][0], val[0], val[1]);
	}

	return len;
}

/*
 * mt5725_get_reverse_charger -- 0xffffff8008ace27c, 52 byte.
 *   "b9433903 ldr"@0xffffff8008ace298   (chip->c824, offset 824)
 *   "321403e1 orr"@0xffffff8008ace29c   (PAGE_SIZE)
 *   "reverse_charger en : %d\n"@0xffffff80092636ed
 */
static ssize_t mt5725_get_reverse_charger(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	return snprintf(buf, PAGE_SIZE, "reverse_charger en : %d\n",
			mt5725_chip_q->c824);
}

/*
 * mt5725_get_otg -- 0xffffff8008ace47c, 52 byte.
 *   "b9433108 ldr"@0xffffff8008ace498   (chip->c816, offset 816)
 *   "%u\n"@0xffffff80090f4c93
 */
static ssize_t mt5725_get_otg(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", mt5725_chip_q->c816);
}

/*
 * Mt5725_get_rxdetect -- 0xffffff8008ace894. The map credits it with 92 bytes
 * but its `ret` sits at 0xffffff8008ace8c4: that is 52. The 40 bytes that
 * follow are the weak, discarded copy of is_usb_rdy (see the file header).
 *   "b9405108 ldr"@0xffffff8008ace8b0   (chip->c80, offset 80)
 *   "d65f03c0 ret"@0xffffff8008ace8c4
 */
static ssize_t Mt5725_get_rxdetect(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", mt5725_chip_q->c80);
}


/*
 * mt5725_set_otg -- 0xffffff8008ace4b0, 152 bytes.
 *   "940e74c1 bl"@0xffffff8008ace4e0    (sscanf, format "%d")
 *   "%d"@0xffffff8009216030
 *   "97ec1175 bl"@0xffffff8008ace4fc    (mt_charger_set_opa_mode)
 *   "b9033114 str"@0xffffff8008ace520   (chip->c816)
 * The locals area is 72 bytes for a single integer read at +8: the local
 * is a 64-byte ARRAY that stack-protector-strong glues to the canary.
 * Its LENGTH is measured (72 - 8), its type is a choice.
 */
static ssize_t mt5725_set_otg(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	unsigned int databuf[16];
	int en;

	sscanf(buf, "%d", &databuf[0]);
	if (databuf[0]) {
		mt_charger_set_opa_mode(mt5725_chg_dev, 1);
		en = 1;
	} else {
		mt_charger_set_opa_mode(mt5725_chg_dev, 0);
		en = 0;
	}
	mt5725_chip_q->c816 = en;

	return count;
}

/*
 * mt5725_set_otp -- 0xffffff8008ace670, 128 byte.
 *   "7100091f cmp"@0xffffff8008ace6a8   (value == 2)
 *   "321f03e9 orr"@0xffffff8008ace6b8   (chip->c820 = 2)
 *   "97fff4c0 bl"@0xffffff8008ace6c0    (MT5725_otp_process)
 */
static ssize_t mt5725_set_otp(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	unsigned int databuf[16];

	sscanf(buf, "%d", &databuf[0]);
	if (databuf[0] == 2) {
		mt5725_chip_q->c820 = 2;
		MT5725_otp_process();
	}

	return count;
}

/*
 * mt5725_set_hwen -- 0xffffff8008ace6f8, 116 bytes.
 *   "1a9f07e8 cset"@0xffffff8008ace738  (the global takes 0/1, not the value)
 *   "b9002d28 str"@0xffffff8008ace73c   (0xffffff800a11102c, full width:
 *                                        a WRITE-ONLY global, nobody in the
 *                                        whole image reads it)
 */
static ssize_t mt5725_set_hwen(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	unsigned int databuf[16];

	sscanf(buf, "%d", &databuf[0]);
	mt5725_hwen = databuf[0] != 0;

	return count;
}

/*
 * mt5725_get_otp() was reconstructed from the factory kernel disassembly (0xffffff8008ace548, 296 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t mt5725_get_otp(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	u8 otpbuf[128];
	int ret;

	mt_charger_set_opa_mode(mt5725_chg_dev, 1);
	msleep(50);
	MT5725_otp_read(0x3d00, otpbuf, 4, 1);

	if (otpbuf[0] == 0x5a && otpbuf[1] == 0xa5 &&
	    otpbuf[2] == 0x5a && otpbuf[3] == 0xa5) {
		printk("MT5725 OTP Write");
		printk("MT5725_otp_write_check Done\n");
		mt_charger_set_opa_mode(mt5725_chg_dev, 0);
		ret = 5;
	} else {
		printk("MT5725 OTP NOT Write");
		if (mt5725_chip_q->c820 == 6)
			return sprintf(buf, "%u\n", 6);
		mt_charger_set_opa_mode(mt5725_chg_dev, 0);
		ret = 0;
	}
	mt5725_chip_q->c820 = ret;

	return sprintf(buf, "%u\n", ret);
}

/*
 * Mt5725_set_vout() was reconstructed from the factory kernel disassembly (0xffffff8008ace76c, 296 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t Mt5725_set_vout(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	unsigned int val;
	u16 vout;
	u16 reg_cmd = 0x0400;

	kstrtouint(buf, 10, &val);
	vout = val;
	printk("%s,write reg_cmd : 0x%04x,\n", __func__, vout);
	c_acab34(mt5725_chip_p, 0x001e, vout);
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
	printk("%s,write reg_cmd : 0x%04x,\n", __func__, reg_cmd);

	return 0;
}

/*
 * mt5725_set_reverse_charger -- 0xffffff8008ace2b0, 460 bytes.
 *   "97fff199 bl"@0xffffff8008ace2f8    (mt5725_reverse_charge(1))
 *   "b9033914 str"@0xffffff8008ace308   (chip->c824 = 1)
 *   "52800280 mov"@0xffffff8008ace304   (msleep(20))
 *   "52804ce8 mov"@0xffffff8008ace340   (0x267 written to registers 0x40/0x41)
 *   "52805f28 mov"@0xffffff8008ace3bc   (0x2f9 written to registers 0x3e/0x3f)
 *   " 0x40 val = 0x%04x\n"@0xffffff8009263706
 *   " 0x40 valw = 0x%04x\n"@0xffffff800926371a
 * A FACTORY DEFECT REPRODUCED: the third print uses the string " 0x40 valw"
 * even though the register read back is 0x3e.
 */
static ssize_t mt5725_set_reverse_charger(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	union mt5725_u16 temp;
	unsigned int databuf[16];

	sscanf(buf, "%d", &databuf[0]);
	if (databuf[0]) {
		mt5725_reverse_charge(1);
		mt5725_chip_q->c824 = 1;
		msleep(20);

		MT5725_read_buffer(mt5725_chip_p, 0x0040, temp.ptr, 2);
		printk(" 0x40 val = 0x%04x\n", temp.c0);
		temp.c0 = 0x0267;
		c_ace34c(mt5725_chip_p, 0x0040, temp.ptr, 2);
		temp.c0 = 0;
		MT5725_read_buffer(mt5725_chip_p, 0x0040, temp.ptr, 2);
		printk(" 0x40 valw = 0x%04x\n", temp.c0);

		temp.c0 = 0x02f9;
		c_ace34c(mt5725_chip_p, 0x003e, temp.ptr, 2);
		temp.c0 = 0;
		MT5725_read_buffer(mt5725_chip_p, 0x003e, temp.ptr, 2);
		printk(" 0x40 valw = 0x%04x\n", temp.c0);
	} else {
		mt5725_reverse_charge(0);
		mt5725_chip_q->c824 = 0;
	}

	return count;
}


/*
 * set_reg -- 0xffffff8008ace0cc, 432 bytes.
 * The `case` is wired by the compiler into a rotation:
 *   "51001909 sub"@0xffffff8008ace13c   (v - 6)
 *   "13890929 ror"@0xffffff8008ace140   (rotate right by 2)
 *   "7100095f cmp"@0xffffff8008ace148 / "54000463 b.cc"@0xffffff8008ace14c
 *   "7100093f cmp"@0xffffff8008ace150 / "54000420 b.eq"@0xffffff8008ace154
 *   "35000789 cbnz"@0xffffff8008ace158
 * Resolving: ror(v-6,2) == 0 -> v = 0x06; == 2 -> v = 0x0e; in {6,7} ->
 * v = 0x1e and v = 0x22. They are exactly four of the seven registers in
 * get_reg's table.
 *   "%x %x"@0xffffff80091411fa
 *   "MT5725 %s : %d 0x%x 0x%x\n"@0xffffff8009263672
 *   "set_reg"@0xffffff8009297df9
 *   "MT5725 get reg: 0x%04x  set reg: 0x%04x \n"@0xffffff800926368c
 *   "MT5725 Set reg : [0x%04x]  0x%x 0x%x \n"@0xffffff80092636b6
 *   "33181d01 bfi"@0xffffff8008ace18c    (val = buf[0] | buf[1] << 8)
 *
 * A FACTORY DEFECT REPRODUCED: in the second print the first `%04x` receives
 * the VALUE (databuf[1]) and not the register ("2a1703e1 mov"@0xffffff8008ace1ec).
 */
static ssize_t set_reg(struct device *dev, struct device_attribute *attr,
		       const char *buf, size_t count)
{
	u8 rbuf[2];
	unsigned int databuf[2];
	u16 val, regval;
	int ret;

	ret = sscanf(buf, "%x %x", &databuf[0], &databuf[1]);
	printk("MT5725 %s : %d 0x%x 0x%x\n", __func__, ret, databuf[0],
	       databuf[1]);

	if (ret == 2) {
		switch (databuf[0]) {
		case 0x06:
			regval = databuf[1];
			MT5725_read_buffer(mt5725_chip_p, databuf[0], rbuf, 2);
			val = rbuf[0] | (rbuf[1] << 8);
			regval |= val;
			printk("MT5725 get reg: 0x%04x  set reg: 0x%04x \n",
			       val, regval);
			MT5725_write_buffer(mt5725_chip_p, databuf[0],
					    (u8 *)&regval, 2);
			break;
		case 0x0e:
		case 0x1e:
		case 0x22:
			regval = databuf[1];
			printk("MT5725 Set reg : [0x%04x]  0x%x 0x%x \n",
			       databuf[1], regval & 0xff, regval >> 8);
			MT5725_write_buffer(mt5725_chip_p, databuf[0],
					    (u8 *)&regval, 2);
			break;
		}
	}

	return count;
}

/* ------------------------------------------------------------------------ */
/*
 * The sysfs group, read with `relocazioni.py --sysfs-group` from the table at
 * 0xffffff80099a75c0 (8 entries + NULL) and from the group at 0xffffff8008f862f0.
 *   "mt5725group"@0xffffff8009263629 (the group's `name` field at
 *   0xffffff8008f862f0 points here)
 */
static DEVICE_ATTR(fast_charging, 0644, NULL, fast_charging_store);
static DEVICE_ATTR(reg, 0644, get_reg, set_reg);
static DEVICE_ATTR(reverse_charger, 0660, mt5725_get_reverse_charger,
		   mt5725_set_reverse_charger);
static DEVICE_ATTR(otg, 0660, mt5725_get_otg, mt5725_set_otg);
static DEVICE_ATTR(otp, 0660, mt5725_get_otp, mt5725_set_otp);
static DEVICE_ATTR(mt5725_en, 0660, mt5725_get_hwen, mt5725_set_hwen);
static DEVICE_ATTR(epp_set_vout, 0660, NULL, Mt5725_set_vout);
static DEVICE_ATTR(rxdetect, 0660, Mt5725_get_rxdetect, NULL);

static struct attribute *mt5725_attributes[] = {
	&dev_attr_fast_charging.attr,
	&dev_attr_reg.attr,
	&dev_attr_reverse_charger.attr,
	&dev_attr_otg.attr,
	&dev_attr_otp.attr,
	&dev_attr_mt5725_en.attr,
	&dev_attr_epp_set_vout.attr,
	&dev_attr_rxdetect.attr,
	NULL,
};

static const struct attribute_group mt5725_attr_group = {
	.name = "mt5725group",
	.attrs = mt5725_attributes,
};
/*
 * MT5725_remove -- 0xffffff8008acd858, 36 byte.
 *   "9100c000 add"@0xffffff8008acd864   (&client->dev.kobj: dev at +0x20,
 *                                        kobj at +0x10 inside struct device)
 *   "97e0d9a1 bl"@0xffffff8008acd86c    (sysfs_remove_group)
 *   "2a1f03e0 mov"@0xffffff8008acd870   (returns 0)
 */
static int MT5725_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &mt5725_attr_group);
	return 0;
}




/*
 * MT5725_fastcharge_change -- 0xffffff8008acc874, 788 bytes.
 * SEVEN MT5725_write_buffer calls inlined, all on the SAME local at sp+8:
 * registers 0x5808 (THREE TIMES with the same value 0x95 -- that is what the
 * binary does), 0x5800, 0x5244, 0x5218 (two bytes), 0x5208.
 *   "397fe261 ldrb"@0xffffff8008acc88c   (state as one byte)
 *   "5280fa00 mov"@0xffffff8008acc89c    (msleep(2000))
 *   "97f355e5 bl"@0xffffff8008acc904     (get_boot_mode)
 *   "7100201f cmp"@0xffffff8008acc908    (== 8 = KERNEL_POWER_OFF_CHARGING_BOOT)
 *   "528012a8 mov"@0xffffff8008acc91c    (0x95)
 *   "528b0101 mov"@0xffffff8008acc920    (register 0x5808)
 *   "528b0001 mov"@0xffffff8008acca0c    (register 0x5800, value 0x01)
 *   "528a4881 mov"@0xffffff8008acca60    (register 0x5244, value 0x57)
 *   "528841e8 mov"@0xffffff8008accab0    (0x420f) / "528a4301 mov"@0xffffff8008accab4 (register 0x5218)
 *   "321f0be8 orr"@0xffffff8008accb04    (0x0e) / "528a4101 mov"@0xffffff8008accb08 (register 0x5208)
 *   "INT_AFC_SUPPORT\n"@0xffffff8009262ebc
 *   "FastChargeState = 1;\n"@0xffffff8009262ecd
 *   "INT_EPP_SUPPORT\n"@0xffffff8009262ee3
 *   "FastChargeState = 0;\n"@0xffffff8009262ef4
 *   "MT5725_fastcharge_change:%d"@0xffffff8009262f0a
 * The last `printk` looks at first sight like a `%d` with no argument: it is
 * not. The state value has been loaded into w1 from the very beginning
 * ("397fe261 ldrb"@0xffffff8008acc88c loads into w1 itself, the register of the
 * second argument) and there is no need to reload it.
 */
void MT5725_fastcharge_change(void)
{
	union mt5725_u16 val;

	if (mt5725_fastcharge_state == 1) {
		msleep(2000);
		if (mt5725_g111018 == 1) {
			mt5725_g111018 = 0;
			mt5725_fastcharge_state = 0;
			printk("INT_EPP_SUPPORT\n");
		} else {
			mt5725_fastcharge_state = 0;
			printk("FastChargeState = 0;\n");
		}
	} else if (mt5725_fastcharge_state == 0) {
		msleep(2000);
		if (mt5725_g111014 == 1) {
			mt5725_g111014 = 0;
			mt5725_fastcharge_state = 0;
			printk("INT_AFC_SUPPORT\n");
		} else {
			mt5725_fastcharge_state = 1;
			if (get_boot_mode() == KERNEL_POWER_OFF_CHARGING_BOOT)
				return;
			val.ptr[0] = 0x95;
			MT5725_write_buffer(mt5725_chip_p, 0x5808, val.ptr, 1);
			MT5725_write_buffer(mt5725_chip_p, 0x5808, val.ptr, 1);
			MT5725_write_buffer(mt5725_chip_p, 0x5808, val.ptr, 1);
			val.ptr[0] = 0x01;
			MT5725_write_buffer(mt5725_chip_p, 0x5800, val.ptr, 1);
			val.ptr[0] = 0x57;
			MT5725_write_buffer(mt5725_chip_p, 0x5244, val.ptr, 1);
			val.c0 = 0x420f;
			MT5725_write_buffer(mt5725_chip_p, 0x5218, val.ptr, 2);
			val.ptr[0] = 0x0e;
			MT5725_write_buffer(mt5725_chip_p, 0x5208, val.ptr, 1);
			msleep(200);
			printk("FastChargeState = 1;\n");
		}
	} else {
		printk("MT5725_fastcharge_change:%d", mt5725_fastcharge_state);
	}
}

/*
 * MT5725_fastcharge_select() was reconstructed from the factory kernel disassembly (0xffffff8008acc748, 300 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void MT5725_fastcharge_select(void)
{
	if (mt5725_fastcharge_state == 1) {
		MT5725_enable_epp();
		printk("MT5725_enable_epp\n");
	} else if (mt5725_fastcharge_state == 0) {
		MT5725_enable_afc();
		printk("MT5725_enable_afc\n");
	} else {
		printk("MT5725_rpp_mode\n");
		return;
	}
	mt5725_fastcharge_timeout = 10;
}

/*
 * MT5725_good_int_delayed_work_func -- 0xffffff8008acdb50, 456 bytes.
 * TWO inlined functions, both NAMED by their own `__func__`:
 *   ech_wls_set_chrg_current  "ech_wls_set_chrg_current"@0xffffff800926316e
 *   set_charger_type          "set_charger_type"@0xffffff800926351b
 *
 *   "1a9f17e1 cset"@0xffffff8008acdb90   (online = !gpio_get_value(...))
 *   "b90fe6a1 str"@0xffffff8008acdb9c    (0xffffff800a110fe4)
 *   "321e03e1 orr"@0xffffff8008acdbb8    (IRQ_TYPE_LEVEL_HIGH = 4)
 *   "321d03e1 orr"@0xffffff8008acdbc4    (IRQ_TYPE_LEVEL_LOW = 8)
 *   "b90fe91f str"@0xffffff8008acdbe0    (0xffffff800a110fe8 = 0)
 *   "b900252a str"@0xffffff8008acdbe4    (0xffffff800a111024 = 200)
 *   "321e03e1 orr"@0xffffff8008acdc74    (POWER_SUPPLY_PROP_ONLINE = 4)
 *   "320003e1 orr"@0xffffff8008acdc90    (POWER_SUPPLY_PROP_CHARGE_TYPE = 1)
 *   "321d03e8 orr"@0xffffff8008acdc88    (the value 8)
 *   "97d9b434 bl"@0xffffff8008acdce4     (enable_irq)
 *   "\x016wls_work_online: %d.\n"@0xffffff8009263503
 *   "charger"@0xffffff800925fee0
 *   "\x016set_charger_type: %d.\n"@0xffffff800926352c
 *   "\x016POWER_SUPPLY_PROP_CHARGE_TYPE: %d.\n"@0xffffff8009263545
 */
static void MT5725_good_int_delayed_work_func(struct work_struct *work)
{
	union power_supply_propval val;
	struct power_supply *psy;

	mt5725_wls_online = !gpio_get_value(mt5725_chip_q->c64);
	pr_info("wls_work_online: %d.\n", mt5725_wls_online);

	if (mt5725_wls_online == 1) {
		irq_set_irq_type(mt5725_good_irq_num, IRQ_TYPE_LEVEL_HIGH);
	} else {
		irq_set_irq_type(mt5725_good_irq_num, IRQ_TYPE_LEVEL_LOW);
		mt5725_good_irq_ready = 0;
		mt5725_g111024[0] = 200;
		ech_wls_set_chrg_current(200, 200);
	}

	/* ---- set_charger_type, inlined ---- */
	psy = power_supply_get_by_name("charger");
	if (!psy) {
		pr_info("%s: get power supply failed\n", "set_charger_type");
	} else {
		val.intval = mt5725_wls_online;
		pr_info("set_charger_type: %d.\n", val.intval);
		power_supply_set_property(psy, POWER_SUPPLY_PROP_ONLINE, &val);
		if (val.intval) {
			val.intval = 8;
			if (!power_supply_set_property(psy,
					POWER_SUPPLY_PROP_CHARGE_TYPE, &val))
				pr_info("POWER_SUPPLY_PROP_CHARGE_TYPE: %d.\n",
					val.intval);
		} else {
			power_supply_set_property(psy,
					POWER_SUPPLY_PROP_CHARGE_TYPE, &val);
		}
	}
	/* ---- fine set_charger_type ---- */

	enable_irq(mt5725_good_irq_num);
}


/*
 * mt5725_charger_work_func -- 0xffffff8008acd940, 528 bytes.
 * ech_wls_set_chrg_current is INLINED into both branches
 * ("ech_wls_set_chrg_current"@0xffffff800926316e).
 *   "11019108 add"@0xffffff8008acd968   (+100)
 *   "5282ede8 mov"@0xffffff8008acd974   (5999)
 *   "540003ec b.gt"@0xffffff8008acd97c  (a SIGNED comparison: here the voltage
 *                                        is a signed integer, unlike in
 *                                        MT5725_check_chargetimeout)
 *   "710fa6bf cmp"@0xffffff8008acd994   (1001) / "7112c6bf cmp"@0xffffff8008acda0c (1201)
 *   "1b087d21 mul"@0xffffff8008acda94   (x1000)
 *   "52801f43 mov"@0xffffff8008acdb1c   (delay 250)
 *   "5V set charger current %d\n "@0xffffff80092634cb
 *   "9V set charger current %d\n "@0xffffff80092634e7
 */
static void mt5725_charger_work_func(struct work_struct *work)
{
	int cur;

	mt5725_g111024[0] += 100;

	if (battery_get_vbus() <= 5999) {
		cur = mt5725_g111024[0];
		mt5725_g111024[1] = 1000;
		if (cur > 1000) {
			cur = 1000;
			mt5725_g111024[0] = 1000;
		}
		ech_wls_set_chrg_current(cur, 1000);
		charger_dev_set_input_current(mt5725_chg_dev,
					      mt5725_g111024[0] * 1000);
		printk("5V set charger current %d\n ", mt5725_g111024[0]);
		if (mt5725_g111024[0] <= 999)
			schedule_delayed_work(&mt5725_chip_q->c216, 250);
	} else {
		cur = mt5725_g111024[0];
		mt5725_g111024[1] = 1500;
		if (cur > 1200) {
			cur = 1200;
			mt5725_g111024[0] = 1200;
		}
		ech_wls_set_chrg_current(cur, 1500);
		charger_dev_set_input_current(mt5725_chg_dev,
					      mt5725_g111024[0] * 1000);
		printk("9V set charger current %d\n ", mt5725_g111024[0]);
		if (mt5725_g111024[0] <= 1199)
			schedule_delayed_work(&mt5725_chip_q->c216, 250);
	}
}

/*
 * mt5725_charger_routine_thread() was reconstructed from the factory kernel disassembly (0xffffff8008acddbc, 432 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mt5725_charger_routine_thread(void *data)
{

	while (1) {
		if (mt5725_rvs_online == 1 && mt5725_chip_q->c80 == 0) {
			mt5725_rxdetect_count++;
			if (mt5725_rxdetect_count >= 120) {
				input_event(mt5725_input, EV_KEY, 87, 1);
				input_event(mt5725_input, EV_SYN, SYN_REPORT, 0);
				input_event(mt5725_input, EV_KEY, 87, 0);
				input_event(mt5725_input, EV_SYN, SYN_REPORT, 0);
				printk("KEY_RXDETECT_STATUS 0\n");
			}
		} else {
			mt5725_rxdetect_count = 0;
			printk("rxdetect_count = 0\n");
		}

		if (mt5725_g11101c >= 2) {
			mt5725_g11101c--;
		} else if (mt5725_g11101c == 1) {
			/*
			 * A WORD writer, not a buffer writer: the factory
			 * folds both bytes into immediates and never touches
			 * the stack. With a buffer the second would be re-read
			 * from [sp,#9].
			 */
			c_acab34(mt5725_chip_p, 0x0022, 0x2328);
			c_acab34(mt5725_chip_p, 0x0006, 0x0080);
			mt5725_g11101c--;
			printk(" MT5725_write_buffer(mte, REG_VFC, temp.ptr, 2)\n");
		}

		msleep(1000);
	}
}

/*
 * mt5725_regmap_config -- 0xffffff8008f86218, 216 bytes in `.rodata`.
 * Read with relocazioni.py: reg_bits = 16, val_bits = 8, everything else zero.
 *   "91086021 add"@0xffffff8008acd3d8   (the address passed to __regmap_init_i2c)
 */
static const struct regmap_config mt5725_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
};

/*
 * MT5725_probe() was reconstructed from the factory kernel disassembly (0xffffff8008acd340, 1304 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int MT5725_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct mt5725_chip *chip;
	struct task_struct *task;
	struct device_node *node;
	int gpio, irq, ret;

	printk("MT5725 probe.\n");
	printk("0.0.1");

	mt5725_chip_q = devm_kzalloc(&client->dev,
				     sizeof(struct mt5725_chip), GFP_KERNEL);
	if (!mt5725_chip_q)
		return -ENOMEM;

	mt5725_chg_dev = get_charger_by_name("primary_chg");
	if (!mt5725_chg_dev) {
		printk(KERN_NOTICE "%s: get primary charger device failed\n",
		       __func__);
		return -1;
	}

	/*
	 * `mutex_init` stringifies its own argument: for the binary to contain
	 * "&chip->slock" the variable MUST be called `chip`. It is the only point
	 * in MT5725_probe where the pointer goes through a local.
	 */
	chip = mt5725_chip_q;
	mutex_init(&chip->slock);
	printk("MT5725 chip.\n");

	mt5725_chip_q->c24 = regmap_init_i2c(client, &mt5725_regmap_config);
	if (!mt5725_chip_q->c24) {
		printk(KERN_ERR "MT5725 parent regmap is missing\n");
		return -EINVAL;
	}
	printk("MT5725 regmap.\n");

	mt5725_chip_q->c8 = client;
	mt5725_chip_q->c16 = &client->dev;
	mt5725_chip_q->c32 = MT5725_read;
	mt5725_chip_q->c40 = MT5725_write;
	mt5725_chip_q->c48 = MT5725_read_buffer;
	mt5725_chip_q->c56 = MT5725_write_buffer;
	mt5725_chip_p = mt5725_chip_q;
	mt5725_chip_q->c412 = 0;
	INIT_DELAYED_WORK(&mt5725_chip_q->c120, MT5725_int_delayed_work_func);
	INIT_DELAYED_WORK(&mt5725_chip_q->c216, mt5725_charger_work_func);
	/*
	 * From here the pointer stays in a REGISTER until the end of the inlined
	 * MT5725_parse_dt: the factory loads it once
	 * ("f94006d5 ldr"@0xffffff8008acd4e8) and then reuses it across the
	 * call to of_get_named_gpio_flags ("b90042a0 str"@0xffffff8008acd50c
	 * writes to that register, not to a re-read value).
	 */
	INIT_DELAYED_WORK(&mt5725_chip_q->c312,
			  MT5725_good_int_delayed_work_func);

	/* ---- MT5725_parse_dt, inlined ---- */
	chip = mt5725_chip_q;
	gpio = of_get_named_gpio_flags(client->dev.of_node, "dc-gpio", 0, NULL);
	chip->c64 = gpio;
	if (gpio < 0)
		printk(KERN_ERR "%s : no dc gpio provided \n ",
		       "MT5725_parse_dt");
	else
		printk(KERN_INFO
		       "%s:dc gpio provided od.mt5725->dc_gpio = %d\n",
		       "MT5725_parse_dt", gpio);

	gpio = of_get_named_gpio_flags(client->dev.of_node, "eint_wpc", 0, NULL);
	chip->c72 = gpio;
	if (gpio < 0)
		printk(KERN_ERR "%s : no irq gpio provided \n ",
		       "MT5725_parse_dt");
	else
		printk(KERN_INFO
		       "%s:irq gpio provided od.mt5725->irq_gpio = %d\n",
		       "MT5725_parse_dt", gpio);
	/* ---- fine MT5725_parse_dt ---- */

	if ((unsigned int)mt5725_chip_q->c64 <= 511) {
		ret = devm_gpio_request_one(&client->dev, mt5725_chip_q->c64,
					    GPIOF_IN, "mt5725_int");
		if (ret) {
			printk(KERN_ERR "%s : irq_gpio request failed\n",
			       __func__);
			return ret;
		}

		irq = gpio_to_irq(mt5725_chip_q->c72);
		mt5725_irq_num = irq;
		devm_request_threaded_irq(&client->dev, irq, NULL, MT5725_irq,
					  IRQF_TRIGGER_LOW | IRQF_ONESHOT,
					  "mt5725", mt5725_chip_q);
		irq_set_irq_wake(mt5725_irq_num, 1);

		irq = gpio_to_irq(mt5725_chip_q->c64);
		mt5725_good_irq_num = irq;
		ret = devm_request_threaded_irq(&client->dev, irq, NULL,
						MT5725_good_irq,
						IRQF_TRIGGER_LOW | IRQF_ONESHOT,
						"mt5725_good", mt5725_chip_q);
		irq_set_irq_wake(mt5725_good_irq_num, 1);
		if (ret) {
			printk(KERN_ERR "failed to request IRQ %d : %d\n",
			       gpio_to_irq(mt5725_chip_q->c72), ret);
			return ret;
		}
		printk(KERN_ERR "sucess to request IRQ %d : %d\n",
		       gpio_to_irq(mt5725_chip_q->c72), ret);
		if (!gpio_get_value(mt5725_chip_p->c72)) {
			printk(KERN_ERR "%s The interruption has come \n",
			       __func__);
			MT5725_irq_handle();
		}
	} else {
		printk(KERN_INFO "%s skipping IRQ registration\n", __func__);
	}

	ret = sysfs_create_group(&client->dev.kobj, &mt5725_attr_group);
	printk("MT5725 probed successfully\n");

	/* ---- gpio_wpc_rt_mode_init, inlined ---- */
	node = of_find_compatible_node(NULL, NULL, "mediatek,usb_iddig_bi_eint");
	if (!node) {
		printk(KERN_ERR "find usb node failed\n");
	} else {
		mt5725_rt_mode_pin = of_get_named_gpio_flags(node,
					"gpio_wpc_rt_mode_pin", 0, NULL);
		if (mt5725_rt_mode_pin < 0) {
			printk(KERN_ERR "%s get gpio_wpc_rt_mode_pin failed!\n",
			       "gpio_wpc_rt_mode_init");
		} else if (gpio_request(mt5725_rt_mode_pin,
					"gpio_wpc_rt_mode_pin") < 0) {
			printk(KERN_ERR "%s gpio_request failed, gpio=%d\n",
			       "gpio_wpc_rt_mode_init", mt5725_rt_mode_pin);
		} else {
			gpio_direction_output(mt5725_rt_mode_pin, 0);
			printk("set gpio_wpc_rt_mode_pin success\n");
		}
	}
	/* ---- fine gpio_wpc_rt_mode_init ---- */

	wakeup_source_init(&mt5725_ws, "reverse_charger_wake_lock");

	task = kthread_create(mt5725_charger_routine_thread, NULL,
			      "mt5725_charger_thread");
	if (!IS_ERR(task))
		wake_up_process(task);

	reverse_charger_rxdetect();

	return ret;
}

/* ------------------------------------------------------------------------ */
/*
 * The match tables and the driver struct, read from the oracle
 * relocations.
 *   "mediatek,mt5725"@0xffffff8008f86088 (compatible, of_device_id at 0xf86048)
 *   "mt5725"@0xffffff8008f861d8          (i2c_device_id at 0xf861d8)
 *   struct i2c_driver at 0xffffff80099a74d8 (the address passed to
 *   i2c_register_driver by mt5725_driver_init:
 *   "91136021 add"@0xffffff8009385a6c)
 */
static const struct of_device_id mt5725_match_table[] = {
	{ .compatible = "mediatek,mt5725", },
	{ },
};

static const struct i2c_device_id mt5725_id[] = {
	{ "mt5725", 0 },
	{ },
};

static struct i2c_driver mt5725_driver = {
	.driver = {
		.name = "mt5725",
		.owner = THIS_MODULE,
		.of_match_table = mt5725_match_table,
	},
	.probe = MT5725_probe,
	.remove = MT5725_remove,
	.id_table = mt5725_id,
};

/*
 * module_i2c_driver() was reconstructed from the factory kernel disassembly (0xffffff8009385a60, 32 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_power_supply_mediatek_charger_mt5725_mt5725.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
module_i2c_driver(mt5725_driver);

/*
 * NOT written: MODULE_LICENSE, MODULE_DESCRIPTION, MODULE_AUTHOR and
 * MODULE_DEVICE_TABLE. The oracle image contains NO
 * `.modinfo` at all:
 *   ./venv/bin/python3 - <<'EOF'
 *   d = open('oracolo/stock.elf','rb').read()
 *   for t in (b'license=GPL', b'description=', b'author=', b'alias=i2c:'):
 *       print(t, d.count(t))
 *   EOF
 *   b'license=GPL' 0
 *   b'description=' 0
 *   b'author=' 0
 *   b'alias=i2c:' 0
 * Nothing can be derived from those lines -- neither that they are there, nor
 * what text they carry. Writing them would be inventing (rule 6).
 */
