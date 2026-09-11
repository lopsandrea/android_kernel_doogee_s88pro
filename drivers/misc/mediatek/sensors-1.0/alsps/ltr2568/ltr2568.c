// SPDX-License-Identifier: GPL-2.0
/*
 * Lite-On LTR2568 ambient light and proximity sensor, Doogee S88 Pro.
 *
 * Reconstructed from the disassembly of the factory kernel, not ported from
 * another phone. The sibling driver ltr303.c (ALPS,
 * drivers/misc/mediatek/sensors-1.0/alsps/ltr303/) served only to find the
 * shape of the subsystem; every constant here comes from the binary.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/sched/clock.h>

/*
 * The includes are in the form that compiles with the real command line of
 * the `alsps` subtree (`-I.../sensors-1.0/alsps`), the same one `alsps.c`
 * uses. In the factory build the subdirectory Makefile would add
 * `-I.../alsps/inc` and the form would be `#include "cust_alsps.h"`: it is a
 * difference in source form that does not change a single instruction.
 */
#include "inc/cust_alsps.h"
#include "inc/alsps.h"

/*
 * The tag and the shape of the messages come from the literals: every printk
 * in the driver starts with '\x013[ALS/PS] %s %d : '. \x01 followed by '3' is
 * KERN_ERR, so the factory uses KERN_ERR for informational messages too. In
 * the run 0xffffff80091bdea3..0xffffff80091be827 there is not a SINGLE literal
 * with the \x016 (KERN_INFO) prefix.
 */
#define LTR2568_DEV_NAME	"ltr2568"	/* "ltr2568"@0xffffff80091bdfb7 */
#define APS_TAG			"[ALS/PS] "
#define APS_ERR(fmt, args...)	printk(KERN_ERR APS_TAG "%s %d : " fmt, \
					__func__, __LINE__, ##args)

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ltr2568_priv {
	struct alsps_hw *hw;			/* +0x00 "a9005015 stp" x21,x20,[x0] */
	struct i2c_client *client;		/* +0x08 idem */
	struct work_struct eint_work;		/* +0x10 "f9000809 str" x9,[x0,#16] */

	/* misc */
	u16 als_modulus;			/* +0x30 "7900600a strh" w10,[x0,#48], w10 = 0x10 */
	atomic_t i2c_retry;			/* +0x34 "b940351a ldr" w26,[x8,#52] */
	atomic_t als_suspend;			/* +0x38 "b900381f str" wzr,[x0,#56] */
	atomic_t als_debounce;			/* +0x3c "b9003c0b str" w11,[x0,#60], w11 = 300 */
	atomic_t als_deb_on;			/* +0x40 "b900401f str" wzr,[x0,#64] */
	atomic_t als_deb_end;			/* +0x44 "b900441f str" wzr,[x0,#68] */
	atomic_t ps_mask;			/* +0x48 "b900481f str" wzr,[x0,#72] */
	atomic_t ps_debounce;			/* +0x4c "b9004c0b str" w11,[x0,#76], w11 = 300 */
	atomic_t ps_deb_on;			/* +0x50 "b900501f str" wzr,[x0,#80] */
	atomic_t ps_deb_end;			/* +0x54 "b900541f str" wzr,[x0,#84] */
	atomic_t ps_suspend;			/* +0x58 "b9405904 ldr" w4,[x8,#88] in show_status */
	atomic_t trace;				/* +0x5c "b9405d03 ldr" w3,[x8,#92] in show_trace */

	struct device_node *irq_node;		/* +0x60 "f9003008 str" x8,[x0,#96] */
	int irq;				/* +0x68 "b9006900 str" w0,[x8,#104] in setup_eint */

	/* data */
	u16 als;				/* +0x6c "9101b101 add" x1, x8, #0x6c */
	u16 ps;					/* +0x6e "9101b901 add" x1, x8, #0x6e */
	u8 _align;				/* +0x70 */
	u16 als_level_num;			/* +0x72 "b8072009 stur" w9,[x0,#114], w9 = 0x0010000f */
	u16 als_value_num;			/* +0x74 high half of the same stur: 16 */
	u32 als_level[C_CUST_ALS_LEVEL - 1];	/* +0x78 store_alslv "def" writes 120..179 */
	u32 als_value[C_CUST_ALS_LEVEL];	/* +0xb4 store_alsval "def" writes 180..243 */
	int ps_cali;				/* +0xf4 "b900f51f str" wzr,[x8,#244] */

	/*
	 * This section was reconstructed from the factory kernel disassembly.
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	u32 _sconosciuto_248;
	u32 _sconosciuto_252;

	atomic_t ps_thd_val_high;		/* +0x100 "b9010109 str" w9,[x8,#256] */
	atomic_t ps_thd_val_low;		/* +0x104 "b9010509 str" w9,[x8,#260] */
	atomic_t als_thd_val_high;		/* +0x108 "b9010808 str" w8,[x0,#264] <- hw+168 */
	atomic_t als_thd_val_low;		/* +0x10c "b9010c08 str" w8,[x0,#268] <- hw+172 */
	u32 ps_threshold;			/* +0x110 "b9011008 str" w8,[x0,#272] <- hw+152 */

	ulong enable;				/* +0x118 "91046016 add" x22, x0, #0x118 */
	/*
	 * This section was reconstructed from the factory kernel disassembly (8 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	ulong pending_intr;
};						/* fine a 296 = 0x128 */

enum CMC_BIT {
	CMC_BIT_ALS = 1,	/* "320003e0 orr" w0, wzr, #0x1 (clear_bit in probe) */
	CMC_BIT_PS = 2,		/* "321f03e0 orr" w0, wzr, #0x2 (clear_bit in probe) */
};

/*
 * A const table at 0xffffff80091be828, eight entries, indexed by bits 3..5 of a
 * register:
 *   "d343fd08 lsr" x8, x8, #3 ; "927e0908 and" x8, x8, #0x1c ; ldr w8,[x9,x8]
 * in ltr2568_i2c_probe.
 */
static const int ltr2568_als_meas_rate[8] = {
	0, 20, 40, 60, 80, 100, 120, 140,
};

/*
 * The 34 registers read by ltr2568_show_reg, at 0xffffff8008f53ae0. The count
 * comes from the loop limit: "f10222ff cmp" x23, #0x88 with stride 4 -> 34.
 */
static const int ltr2568_reg_dump[34] = {
	0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
	0x89, 0x8a, 0x8b, 0x8c, 0x91, 0x92, 0x93, 0x94, 0x95, 0x98,
	0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa4, 0xb6, 0xb7,
	0xad, 0xb4, 0xba, 0xb9,
};

struct alsps_hw alsps_cust;			/* 0xffffff8009cb9fd8 */
static struct alsps_hw *hw = &alsps_cust;

static struct ltr2568_priv *ltr2568_obj;	/* 0xffffff8009cba0b0 */
static struct i2c_client *ltr2568_i2c_client;	/* 0xffffff8009cba0c0 */
static DEFINE_MUTEX(ltr2568_mutex);		/* 0xffffff800992e670 */

/*
 * 0xffffff8009cba0bc. Read with `ldrb` and cleared with `strb wzr`, so it is a
 * byte, not an int: in ltr2568_local_init the sequence is
 *   "3942f108 ldrb" w8,[x8,#188] ; "2a2803e8 mvn" w8, w8 ;
 *   "13000100 sbfx" w0, w8, #0, #1
 * that is `return flag ? 0 : -1` on a bool. An `int init_flag = -1` would have
 * given a 32-bit read and a `cmn`.
 */
static bool ltr2568_init_flag;

/*
 * 0xffffff8009cba0b8. Set to 1 by setup_eint after request_irq, compared with
 * 2 by eint_handler. It is an `int`: "b900b913 str" w19,[x8,#184] (32-bit).
 */
static int ltr2568_irq_enabled;

/* 0xffffff8009cba0c8, written with `strb` by ps_get_data: one byte. */
static u8 ltr2568_intr_flag;

/* 0xffffff8009cba0cc, "394332c8 ldrb" w8,[x22,#204] in ltr2568_als_enable. */
static bool ltr2568_als_on;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009cba0d0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * 0xffffff8009cba0d0. A byte: "39434288 ldrb" w8,[x20,#208] in ps_enable_nodata,
 * "39034297 strb" w23,[x20,#208] (1) and "3903429f strb" wzr,[x20,#208] (0) in
 * the two branches of the same function, "39434108 ldrb" w8,[x8,#208] in
 * ltr2568_ps_read. It is the "PS on" state: ps_enable_nodata compares it with
 * the request and, if they match, does NOT touch the bus.
 */
static bool ltr2568_ps_on;

/*
 * 0xffffff8009cba0d4. A 32-bit int ("b900d55f str" wzr,[x10,#212],
 * "b940d548 ldr" w8,[x10,#212], "b900d548 str" w8,[x10,#212]). It counts
 * consecutive PS readings equal to zero: in ltr2568_ps_read it is incremented
 * and compared with 4 ("7100111f cmp" w8, #0x4), cleared as soon as a reading
 * is non-zero, and cleared by ps_enable_nodata at power-on too.
 */
static int ltr2568_ps_zero_cnt;

/*
 * 0xffffff8009cba0d8. A byte ("39436109 ldrb" w9,[x8,#216],
 * "3903614b strb" w11,[x10,#216] with w11 = 1, "3903611f strb" wzr,[x8,#216]).
 * It says which lower threshold ltr2568_ps_read uses to decide whether a
 * sample falls inside the window: if 1, obj->ps_thd_val_low, otherwise
 * ltr2568_ps_base - 10. Set to 1 when four zeroes in a row open a
 * recalibration, cleared when the window closes.
 */
static bool ltr2568_ps_cali_on;

/*
 * 0xffffff8009cba0dc. An int ("b900dd14 str" w20,[x8,#220] in ltr2568_ps_read,
 * "b900dd35 str" w21,[x9,#220] in ps_enable_nodata, "b940dd29 ldr"
 * w9,[x9,#220]). It is the reference PS value: the average of the five samples
 * at power-on calibration, or the sample that closed the window.
 */
static int ltr2568_ps_base;

/*
 * 0xffffff8009cba0e0. A u16 -- "7941c12a ldrh" w10,[x9,#224] and
 * "7901c12a strh" w10,[x9,#224], sixteen bits. It is the index into the
 * sliding window, compared with 8 ("f100215f cmp" x10, #0x8).
 */
static u16 ltr2568_ps_idx;

/*
 * 0xffffff8009cba0e2. A byte ("3943896b ldrb" w11,[x11,#226],
 * "3903896c strb" w12,[x11,#226]). It says the window has filled at least
 * once: from then on ltr2568_ps_read computes the average instead of
 * accumulating.
 */
static bool ltr2568_ps_full;

/*
 * 0xffffff8009cba0e4. A u16 ("7941c9ca ldrh" w10,[x14,#228],
 * "7901c9ca strh" w10,[x14,#228]). How many consecutive samples of the window
 * lie within +-10 of the average; at 7 the calibration closes
 * ("71001d9f cmp" w12, #0x7).
 */
static u16 ltr2568_ps_stable;

/*
 * 0xffffff8009cba0e8..0x0f7. Eight u16s: "9103a18c add" x12, x12, #0xe8 followed
 * by "782a7994 strh" w20,[x12,x10,lsl #1] (stride 2) and by the eight reads
 * ldrh [x12], [x12,#2] ... [x12,#14]. It is the sliding window of samples.
 */
static u16 ltr2568_ps_buf[8];

/* 0xffffff8009cba0f8, "b900f928 str" w8,[x9,#248] in ltr2568_i2c_probe. */
static int ltr2568_meas_rate;

/*
 * 0xffffff8009cba0fc. An int, read and rewritten ONLY by ltr2568_als_read
 * ("b940fd15 ldr" w21,[x8,#252] and "b900fd15 str" w21,[x8,#252]). It is the
 * last light value returned: on the error paths the function returns this
 * instead of a fresh value.
 */
static int ltr2568_als_last;

/* 0xffffff8009cba100, "3904011f strb" wzr,[x8,#256] in ltr2568_als_read. */
static bool ltr2568_als_first;

/*
 * 0xffffff8009cba104 and 0x0108. Two ints, read and written only by
 * ltr2568_als_read ("b941054b ldr" w11,[x10,#260] / "b9010549 str"
 * w9,[x10,#260] ; "b941096b ldr" w11,[x11,#264] / "b90109ab str"
 * w11,[x13,#264]). They are the previous filtered values of the two ALS
 * channels -- 0x104 for the register pair 0x8b/0x8c, 0x108 for 0x89/0x8a --
 * that is, the state of the settling filter.
 */
static int ltr2568_als_prev1;
static int ltr2568_als_prev0;

/* 0xffffff8009cba10c / 0x110 / 0x114, used by ltr2568_ps_set_thres. */
static int ltr2568_thres_high;
static int ltr2568_thres_low;
static bool ltr2568_thres_override;

/*
 * ltr2568_als_read() was reconstructed from the factory kernel disassembly (0xffffff800992e668).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ltr2568_als_gain = 12;

/*
 * 0xffffff800992e66c, initial value 400 (in the factory .data:
 * "90 01 00 00"). An int: "b9466d09 ldr" w9,[x8,#1644] and "b9066d17 str"
 * w23,[x8,#1644]. It is the proximity offset written into registers
 * 0x9e/0x9f: ps_enable_nodata recomputes it from the average of five samples
 * ("5104b2b7 sub" w23, w21, #0x12c) and ltr2568_ps_read lowers it by 300 on
 * every fourth null reading ("7104b129 subs" w9, w9, #0x12c ;
 * "1a9fc135 csel" w21, w9, wzr, gt).
 */
static int ltr2568_ps_offset = 400;

/*
 * 0xffffff800992e690, initial value 64 ("40 00 00 00"). It is the third
 * argument of the printk "ALS sensor gainrange %d!" ("b9469103 ldr"
 * w3,[x8,#1680]).
 *
 * ltr2568_als_read shows its full role: it is the ALS gain in natural units
 * (1, 4, 16, 64), always re-read from the table
 * ("b90692c8 str" w8,[x22,#1680]) and used as the divisor in the conversion
 * to lux ("1aca0d29 sdiv" w9, w9, w10).
 */
static int ltr2568_als_gainrange = 64;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff80091be848, 16 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const int ltr2568_als_gain_table[4] = {
	1, 4, 16, 64,
};

static int ltr2568_local_init(void);
static int ltr2568_remove(void);

static struct alsps_init_info ltr2568_init_info = {	/* 0xffffff800992e550 */
	.name = LTR2568_DEV_NAME,			/* reloc +0x00 -> 0xffffff80091bdfb7 */
	.init = ltr2568_local_init,			/* reloc +0x08 */
	.uninit = ltr2568_remove,			/* reloc +0x10 */
};

static const struct i2c_device_id ltr2568_i2c_id[] = {	/* 0xffffff8008f53aa0 */
	{LTR2568_DEV_NAME, 0},
	{},
};

static const struct of_device_id alsps_of_match[] = {	/* 0xffffff8008f53858 */
	{.compatible = "mediatek,alsps"},	/* "mediatek,alsps"@0xffffff8008f53898 */
	{},
};

static int ltr2568_i2c_probe(struct i2c_client *client,
			     const struct i2c_device_id *id);
static int ltr2568_i2c_remove(struct i2c_client *client);
static int ltr2568_i2c_detect(struct i2c_client *client,
			      struct i2c_board_info *info);
static int ltr2568_i2c_suspend(struct device *dev);
static int ltr2568_i2c_resume(struct device *dev);

/*
 * 0xffffff8008f539e8: six pointers at +0x10..+0x38 (suspend, resume, freeze,
 * thaw, poweroff, restore), alternating between ltr2568_i2c_suspend and
 * ltr2568_i2c_resume, and nothing else. That is exactly SET_SYSTEM_SLEEP_PM_OPS.
 */
static const struct dev_pm_ops ltr2568_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(ltr2568_i2c_suspend, ltr2568_i2c_resume)
};

static struct i2c_driver ltr2568_i2c_driver = {		/* 0xffffff800992e570 */
	.probe = ltr2568_i2c_probe,			/* reloc +0x10 */
	.remove = ltr2568_i2c_remove,			/* reloc +0x18 */
	.detect = ltr2568_i2c_detect,			/* reloc +0xc0 */
	.id_table = ltr2568_i2c_id,			/* reloc +0xb8 -> 0xffffff8008f53aa0 */
	.driver = {					/* 0xffffff800992e5b0 */
		.name = LTR2568_DEV_NAME,		/* reloc +0x00 */
		.of_match_table = alsps_of_match,	/* reloc +0x28 -> 0xffffff8008f53858 */
		.pm = &ltr2568_pm_ops,			/* reloc +0x68 -> 0xffffff8008f539e8 */
	},
};

/*
 * All four `static`, as in the factory (`t` in stock.map). The forward
 * declarations are needed because the debug attributes and the framework ops,
 * which come earlier in the file, call them.
 */
static int ltr2568_ps_read(struct i2c_client *client, u16 *data);
static int ltr2568_als_read(struct i2c_client *client, u16 *data);
static int ps_enable_nodata(int en);
static void ltr2568_eint_work(struct work_struct *work);

/* ------------------------------------------------------------------ i2c -- */

/*
 * ltr2568_master_send() was reconstructed from the factory kernel disassembly (0xffffff80091be1bd, 20 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ltr2568_master_send(struct i2c_client *client, u8 addr, u8 *data)
{
	int err = 0;
	int i;
	int max_try;
	u8 buf[2];

	atomic_read(&ltr2568_obj->trace);	/* "b9405d1f ldr" wzr,[x8,#92] */
	max_try = atomic_read(&ltr2568_obj->i2c_retry);

	for (i = 0; i < max_try; i++) {
		mutex_lock(&ltr2568_mutex);
		if (!client) {
			mutex_unlock(&ltr2568_mutex);
			err = -EINVAL;		/* "128002b9 mov" w25, #0xffffffea */
		} else {
			buf[0] = addr;
			buf[1] = data[0];
			err = i2c_master_send(client, buf, 2);
			if (err < 0) {
				/* "\x013[ALS/PS] %s %d : send command error!!\n"@0xffffff80091be1d5 */
#line 303
				APS_ERR("send command error!!\n");
				mutex_unlock(&ltr2568_mutex);
				err = -EFAULT;	/* "128001b9 mov" w25, #0xfffffff2 */
			} else {
				mutex_unlock(&ltr2568_mutex);
			}
		}
		if (err == 0)
			break;
		/*
		 * "5291b780 mov" w0, #0x8dbc with "72a000c0 movk" w0, #0x6, lsl #16
		 * -> __const_udelay(0x68dbc), that is udelay(100).
		 */
		udelay(100);
	}

	if (err == 0)
		err = 1;		/* "320003f9 orr" w25, wzr, #0x1 */

	return err;
}

/*
 * No printk, hence no line number. It uses i2c_transfer with two messages
 * ("321f03e2 orr" w2, wzr, #0x2): writing one register byte plus reading
 * `len` bytes.
 *
 * The same discarded volatile read of obj->trace as in ltr2568_master_send
 * ("b9405d1f ldr" wzr,[x8,#92] at 0xffffff8008784ffc), again before the read
 * of i2c_retry ("b9403519 ldr" w25,[x8,#52]).
 */
static int ltr2568_master_recv(struct i2c_client *client, u8 addr, u8 *data,
			       u8 len)
{
	int err = 0;
	int i;
	int max_try;
	/*
	 * NOT initialised here: the loop reassigns `addr` on every round, and the
	 * factory writes it ONLY there. With `= addr` in the declaration clang emits
	 * a "strb w1, [sp,#4]" before the loop that is not in the binary.
	 */
	u8 beg;
	struct i2c_msg msgs[2];

	atomic_read(&ltr2568_obj->trace);	/* "b9405d1f ldr" wzr,[x8,#92] */
	max_try = atomic_read(&ltr2568_obj->i2c_retry);

	if (max_try < 1)	/* "7100073f cmp" w25, #0x1 ; "540005eb b.lt" */
		return len;

	for (i = 0; i < max_try; i++) {
		beg = addr;
		msgs[0].addr = client->addr;
		msgs[0].flags = 0;
		msgs[0].len = 1;	/* "790037fc strh" w28,[sp,#26], w28 = 1 */
		msgs[0].buf = &beg;
		msgs[1].addr = client->addr;
		msgs[1].flags = I2C_M_RD;	/* "321003e9 orr" w9, wzr, #0x10000 */
		msgs[1].len = len;
		msgs[1].buf = data;

		mutex_lock(&ltr2568_mutex);
		if (!client || len >= 9) {	/* "7100251f cmp" w8, #0x9 ; "540000a3 b.cc" */
			mutex_unlock(&ltr2568_mutex);
			err = -EINVAL;		/* "128002b8 mov" w24, #0xffffffea */
		} else {
			err = i2c_transfer(client->adapter, msgs, 2);
			mutex_unlock(&ltr2568_mutex);
			if (err == 2)		/* "71000b1f cmp" w24, #0x2 */
				return len;
			err = -EIO;	/* "321d7bf8 orr" w24, wzr, #0xfffffffb */
		}
		udelay(100);	/* "5291b780 mov" w0, #0x8dbc + "72a000c0 movk" #0x6, lsl #16 */
	}

	return err;
}

/* --------------------------------------------------------------- thresholds -- */

/*
 * Line 775. Two forms, chosen by the flag at 0xffffff8009cba114: if it is 1 it
 * uses the pair of globals 0xffffff8009cba110 / 0xffffff8009cba10c, otherwise
 * obj->ps_thd_val_low / obj->ps_thd_val_high.
 *
 * Registers: 0x9c/0x9d (low threshold, low and high byte) and 0x9a/0x9b (high
 * threshold) -- "52801389 mov" w9, #0x9c ; "528013a9 mov" w9, #0x9d ;
 * "52801349 mov" w9, #0x9a ; "52801369 mov" w9, #0x9b.
 *
 * Here the factory does NOT go through ltr2568_master_send: it calls
 * i2c_master_send directly with a 2-byte buffer ("940c1710 bl" -> i2c_master_send).
 */
static int ltr2568_ps_set_thres(void)
{
	struct ltr2568_priv *obj = ltr2568_obj;
	struct i2c_client *client = obj->client;
	u8 databuf[2];
	int res;

	databuf[0] = 0x9c;
	if (ltr2568_thres_override) {
		databuf[1] = (u8)ltr2568_thres_low;
		res = i2c_master_send(client, databuf, 2);
		if (res <= 0)
			goto EXIT_ERR;
		databuf[0] = 0x9d;
		databuf[1] = (u8)(ltr2568_thres_low >> 8);
		res = i2c_master_send(client, databuf, 2);
		if (res <= 0)
			goto EXIT_ERR;
		databuf[0] = 0x9a;
		databuf[1] = (u8)ltr2568_thres_high;
		res = i2c_master_send(client, databuf, 2);
		if (res <= 0)
			goto EXIT_ERR;
		databuf[0] = 0x9b;
		databuf[1] = (u8)(ltr2568_thres_high >> 8);
		res = i2c_master_send(client, databuf, 2);
		if (res <= 0)
			goto EXIT_ERR;
		ltr2568_thres_override = false;	/* "3904529f strb" wzr,[x20,#276] */
		return 0;
	}

	databuf[1] = (u8)atomic_read(&obj->ps_thd_val_low);
	res = i2c_master_send(client, databuf, 2);
	if (res <= 0)
		goto EXIT_ERR;
	databuf[0] = 0x9d;
	databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) >> 8);
	res = i2c_master_send(client, databuf, 2);
	if (res <= 0)
		goto EXIT_ERR;
	databuf[0] = 0x9a;
	databuf[1] = (u8)atomic_read(&obj->ps_thd_val_high);
	res = i2c_master_send(client, databuf, 2);
	if (res <= 0)
		goto EXIT_ERR;
	databuf[0] = 0x9b;
	databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) >> 8);
	res = i2c_master_send(client, databuf, 2);
	if (res <= 0)
		goto EXIT_ERR;
	return 0;

EXIT_ERR:
	/* "\x013[ALS/PS] %s %d : set thres: %d\n"@0xffffff80091be23d */
#line 775
	APS_ERR("set thres: %d\n", res);
	return -1;
}

/* --------------------------------------------------------- ALS power-on -- */

/*
 * Line 1116. It reads register 0x80, forces bit 0 to 1 or 0 and writes it
 * back; then mdelay(10). The flag at 0xffffff8009cba0cc says whether the ALS
 * is already in the requested state, and in that case the function does not
 * touch the bus.
 */
static int ltr2568_als_enable(struct i2c_client *client, int enable)
{
	u8 databuf[1];
	int res;

	databuf[0] = 0;			/* "390013ff strb" wzr,[sp,#4] */
	if (enable && ltr2568_als_on)
		return 0;
	if (!enable && !ltr2568_als_on)
		return 0;

	/* register 0x80: "321903e1 orr" w1, wzr, #0x80 */
	ltr2568_master_recv(client, 0x80, databuf, 1);
	if (enable) {
		databuf[0] |= 0x01;	/* "32000108 orr" w8, w8, #0x1 */
		ltr2568_als_on = true;
		ltr2568_als_first = false;
	} else {
		databuf[0] &= 0xfe;	/* "121f1908 and" w8, w8, #0xfe */
		ltr2568_als_on = false;
	}
	res = ltr2568_master_send(client, 0x80, databuf);
	if (res < 0) {
		/* "\x013[ALS/PS] %s %d : ALS: enable als err: %d en: %d \n"@0xffffff80091be64a */
#line 1116
		APS_ERR("ALS: enable als err: %d en: %d \n", res, enable);
		return res;
	}
	/* ten __const_udelay(0x418958) consecutive = mdelay(10) */
	mdelay(10);
	return 0;
}

/*
 * ltr2568_get_als_value() was reconstructed from the factory kernel disassembly (0xffffff80091be6bf).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ltr2568_get_als_value(struct ltr2568_priv *obj, u16 als)
{
	int idx;
	int invalid = 0;

	for (idx = 0; idx < obj->als_level_num; idx++) {
		if (als < obj->hw->als_level[idx])
			break;
	}
	if (idx >= obj->als_value_num) {
		/* "\x013[ALS/PS] %s %d : exceed range\n"@0xffffff80091be69e */
#line 1295
		APS_ERR("exceed range\n");
		idx = obj->als_value_num - 1;
	}

	if (atomic_read(&obj->als_deb_on) == 1) {
		unsigned long endt = atomic_read(&obj->als_deb_end);

		if (time_after(jiffies, endt))
			atomic_set(&obj->als_deb_on, 0);

		if (atomic_read(&obj->als_deb_on) == 1)
			invalid = 1;
	}

	if (!invalid)
		return als;

	/* "\x013[ALS/PS] %s %d : ALS: %05d => %05d (-1)\n"@0xffffff80091be6d5 */
#line 1320
	APS_ERR("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);
	return -1;
}

/* ----------------------------------------------------- debug attributes -- */

/* Line 1335. "(%d %d %d %d %d)\n"@0xffffff80091be4e6 */
static ssize_t ltr2568_show_config(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	if (!ltr2568_obj) {
		/* "\x013[ALS/PS] %s %d : ltr2568_obj is null!!\n"@0xffffff80091be1fe */
#line 1335
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	res = snprintf(buf, PAGE_SIZE, "(%d %d %d %d %d)\n",
		       atomic_read(&ltr2568_obj->i2c_retry),
		       atomic_read(&ltr2568_obj->als_debounce),
		       atomic_read(&ltr2568_obj->ps_mask),
		       ltr2568_obj->ps_threshold,
		       atomic_read(&ltr2568_obj->ps_debounce));
	return res;
}

/* Line 1350/1364. "%d %d %d %d %d"@0xffffff800921596c */
static ssize_t ltr2568_store_config(struct device_driver *ddri, const char *buf,
				    size_t count)
{
	int retry, als_deb, mask, thres, ps_deb;

	if (!ltr2568_obj) {
#line 1350
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	if (sscanf(buf, "%d %d %d %d %d", &retry, &als_deb, &mask, &thres,
		   &ps_deb) == 5) {
		atomic_set(&ltr2568_obj->i2c_retry, retry);
		atomic_set(&ltr2568_obj->als_debounce, als_deb);
		atomic_set(&ltr2568_obj->ps_mask, mask);
		ltr2568_obj->ps_threshold = thres;
		atomic_set(&ltr2568_obj->ps_debounce, ps_deb);
	} else {
		/* "\x013[ALS/PS] %s %d : invalid content: '%s', length = %zu\n"@0xffffff80091be49a */
#line 1364
		APS_ERR("invalid content: '%s', length = %zu\n", buf, count);
	}

	return count;
}

/* Line 1374. "0x%04X\n"@0xffffff80091be5fe */
static ssize_t ltr2568_show_trace(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	if (!ltr2568_obj) {
#line 1374
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}
	res = snprintf(buf, PAGE_SIZE, "0x%04X\n",
		       atomic_read(&ltr2568_obj->trace));
	return res;
}

/* Line 1387/1397. "0x%x"@0xffffff8009142911 */
static ssize_t ltr2568_store_trace(struct device_driver *ddri, const char *buf,
				   size_t count)
{
	int trace;

	if (!ltr2568_obj) {
#line 1387
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	if (sscanf(buf, "0x%x", &trace) == 1)
		atomic_set(&ltr2568_obj->trace, trace);
	else
#line 1397
		APS_ERR("invalid content: '%s', length = %zu\n", buf, count);

	return count;
}

/* Line 1408. "0x%04X(%d)\n"@0xffffff80091be457 */
static ssize_t ltr2568_show_als(struct device_driver *ddri, char *buf)
{
	int res;

	if (!ltr2568_obj) {
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	res = ltr2568_als_read(ltr2568_obj->client, &ltr2568_obj->als);
	return snprintf(buf, PAGE_SIZE, "0x%04X(%d)\n", res, res);
}

/* Line 1421. The same format as show_als. */
static ssize_t ltr2568_show_ps(struct device_driver *ddri, char *buf)
{
	int res;

	if (!ltr2568_obj) {
#line 1421
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	res = ltr2568_ps_read(ltr2568_obj->client, &ltr2568_obj->ps);
	return snprintf(buf, PAGE_SIZE, "0x%04X(%d)\n", res, res);
}

/* "aa1f03e0 mov" x0, xzr ; "d65f03c0 ret" -- eight byte. */
static ssize_t ltr2568_show_send(struct device_driver *ddri, char *buf)
{
	return 0;
}

/* Line 1483/1488. "%x %x"@0xffffff80091411fa */
static ssize_t ltr2568_store_send(struct device_driver *ddri, const char *buf,
				  size_t count)
{
	int addr, cmd;

	if (!ltr2568_obj) {
#line 1483
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	} else if (sscanf(buf, "%x %x", &addr, &cmd) != 2) {
		/* "\x013[ALS/PS] %s %d : invalid format: '%s'\n"@0xffffff80091be534 */
#line 1488
		APS_ERR("invalid format: '%s'\n", buf);
		return 0;
	}

	return count;
}

/* "aa1f03e0 mov" x0, xzr ; "d65f03c0 ret" -- eight byte. */
static ssize_t ltr2568_show_recv(struct device_driver *ddri, char *buf)
{
	return 0;
}

/* Line 1508/1513. "%x"@0xffffff800926f8f8 */
static ssize_t ltr2568_store_recv(struct device_driver *ddri, const char *buf,
				  size_t count)
{
	int addr;

	if (!ltr2568_obj) {
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	} else if (sscanf(buf, "%x", &addr) != 1) {
#line 1513
		APS_ERR("invalid format: '%s'\n", buf);
		return 0;
	}

	return count;
}

/*
 * Line 1527. "CUST: %d, (%d %d)\n"@0xffffff80091be59a,
 * "CUST: NULL\n"@0xffffff80091be5ad (which the compiler reduces to two
 * immediate stores, "5289898a mov" w10, #0x4c4c and "52800174 mov" w20, #0xb),
 * "MISC: %d %d\n"@0xffffff80091be5b9.
 */
static ssize_t ltr2568_show_status(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;

	if (!ltr2568_obj) {
#line 1527
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	if (ltr2568_obj->hw) {
		len += snprintf(buf + len, PAGE_SIZE - len, "CUST: %d, (%d %d)\n",
				ltr2568_obj->hw->i2c_num,
				ltr2568_obj->hw->power_id,
				ltr2568_obj->hw->power_vol);
	} else {
		len += snprintf(buf + len, PAGE_SIZE - len, "CUST: NULL\n");
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "MISC: %d %d\n",
			atomic_read(&ltr2568_obj->als_suspend),
			atomic_read(&ltr2568_obj->ps_suspend));
	return len;
}

/*
 * Inlined into store_alslv and store_alsval. The two separators are
 * "7100811f cmp" w8, #0x20 (space) and "7100291f cmp" w8, #0xa (newline);
 * the format is "%d"@0xffffff8009216030.
 */
#define IS_SPACE(CH) (((CH) == ' ') || ((CH) == '\n'))
static int read_int_from_buf(struct ltr2568_priv *obj, const char *buf,
			     size_t count, u32 data[], int len)
{
	int idx = 0;
	char *cur = (char *)buf, *end = (char *)(buf + count);

	while (idx < len) {
		while ((cur < end) && IS_SPACE(*cur))
			cur++;

		if (sscanf(cur, "%d", &data[idx]) != 1)
			break;

		idx++;
		while ((cur < end) && !IS_SPACE(*cur))
			cur++;
	}

	return idx;
}

/* Line 1582. "%d "@0xffffff8009295b89 and "\n"@0xffffff8009245994 */
static ssize_t ltr2568_show_alslv(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	int idx;

	if (!ltr2568_obj) {
#line 1582
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	for (idx = 0; idx < ltr2568_obj->als_level_num; idx++)
		len += snprintf(buf + len, PAGE_SIZE - len, "%d ",
				ltr2568_obj->hw->als_level[idx]);

	len += snprintf(buf + len, PAGE_SIZE - len, "\n");
	return len;
}

/* Line 1598/1608. "def"@0xffffff800920c7c5 */
static ssize_t ltr2568_store_alslv(struct device_driver *ddri, const char *buf,
				   size_t count)
{
	if (!ltr2568_obj) {
#line 1598
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	} else if (!strcmp(buf, "def"))
		memcpy(ltr2568_obj->als_level, ltr2568_obj->hw->als_level,
		       sizeof(ltr2568_obj->als_level));
	else if (ltr2568_obj->als_level_num !=
		 read_int_from_buf(ltr2568_obj, buf, count,
				   ltr2568_obj->hw->als_level,
				   ltr2568_obj->als_level_num))
#line 1608
		APS_ERR("invalid format: '%s'\n", buf);

	return count;
}

/* Line 1619. */
static ssize_t ltr2568_show_alsval(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	int idx;

	if (!ltr2568_obj) {
#line 1619
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	for (idx = 0; idx < ltr2568_obj->als_value_num; idx++)
		len += snprintf(buf + len, PAGE_SIZE - len, "%d ",
				ltr2568_obj->hw->als_value[idx]);

	len += snprintf(buf + len, PAGE_SIZE - len, "\n");
	return len;
}

/* Line 1635/1645. */
static ssize_t ltr2568_store_alsval(struct device_driver *ddri, const char *buf,
				    size_t count)
{
	if (!ltr2568_obj) {
#line 1635
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	} else if (!strcmp(buf, "def"))
		memcpy(ltr2568_obj->als_value, ltr2568_obj->hw->als_value,
		       sizeof(ltr2568_obj->als_value));
	else if (ltr2568_obj->als_value_num !=
		 read_int_from_buf(ltr2568_obj, buf, count,
				   ltr2568_obj->hw->als_value,
				   ltr2568_obj->als_value_num))
#line 1645
		APS_ERR("invalid format: '%s'\n", buf);

	return count;
}

/*
 * It reads the 34 ltr2568_reg_dump registers one by one.
 * "reg:0x%04X value: 0x%04X\n"@0xffffff80091be5ec
 */
static ssize_t ltr2568_show_reg(struct device_driver *ddri, char *buf)
{
	int i, len = 0;
	u8 val;

	for (i = 0; i < 34; i++) {
		ltr2568_master_recv(ltr2568_obj->client, ltr2568_reg_dump[i],
				    &val, 1);
		len += snprintf(buf + len, PAGE_SIZE - len,
				"reg:0x%04X value: 0x%04X\n",
				ltr2568_reg_dump[i], val);
	}

	return len;
}

/*
 * The ten attributes, in the order they sit in .data starting at
 * 0xffffff800992e718 (stride 0x20). The mode is 0644 for all ten:
 * the `mode` field is 0x1a4 in every entry.
 */
static DRIVER_ATTR(als,    0644, ltr2568_show_als,    NULL);
static DRIVER_ATTR(ps,     0644, ltr2568_show_ps,     NULL);
static DRIVER_ATTR(trace,  0644, ltr2568_show_trace,  ltr2568_store_trace);
static DRIVER_ATTR(config, 0644, ltr2568_show_config, ltr2568_store_config);
static DRIVER_ATTR(alslv,  0644, ltr2568_show_alslv,  ltr2568_store_alslv);
static DRIVER_ATTR(alsval, 0644, ltr2568_show_alsval, ltr2568_store_alsval);
static DRIVER_ATTR(status, 0644, ltr2568_show_status, NULL);
static DRIVER_ATTR(send,   0644, ltr2568_show_send,   ltr2568_store_send);
static DRIVER_ATTR(recv,   0644, ltr2568_show_recv,   ltr2568_store_recv);
static DRIVER_ATTR(reg,    0644, ltr2568_show_reg,    NULL);

static struct driver_attribute *ltr2568_attr_list[] = {
	&driver_attr_als,	/* 0xffffff800992e718, name -> 'als' */
	&driver_attr_ps,	/* 0xffffff800992e738, name -> 'ps' */
	&driver_attr_trace,	/* 0xffffff800992e758, name -> 'trace' */
	&driver_attr_config,	/* 0xffffff800992e778, name -> 'config' */
	&driver_attr_alslv,	/* 0xffffff800992e798, name -> 'alslv' */
	&driver_attr_alsval,	/* 0xffffff800992e7b8, name -> 'alsval' */
	&driver_attr_status,	/* 0xffffff800992e7d8, name -> 'status' */
	&driver_attr_send,	/* 0xffffff800992e7f8, name -> 'send' */
	&driver_attr_recv,	/* 0xffffff800992e818, name -> 'recv' */
	&driver_attr_reg,	/* 0xffffff800992e838, name -> 'reg' */
};

/*
 * Line 1689 -- the name comes from the `__func__` at 0xffffff80091be432. In
 * the factory build it is inlined into ltr2568_i2c_probe, which unrolls the
 * ten driver_create_file calls.
 * "\x013[ALS/PS] %s %d : driver_create_file (%s) = %d\n"@0xffffff80091be401
 */
static int ltr2568_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(ltr2568_attr_list);

	if (driver == NULL)
		return -EINVAL;

	for (idx = 0; idx < num; idx++) {
		err = driver_create_file(driver, ltr2568_attr_list[idx]);
		if (err) {
			APS_ERR("driver_create_file (%s) = %d\n",
#line 1689
				ltr2568_attr_list[idx]->attr.name, err);
			break;
		}
	}

	return err;
}

/*
 * Inlined into ltr2568_i2c_remove, which unrolls the ten driver_remove_file
 * calls without checking their outcome.
 */
static int ltr2568_delete_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(ltr2568_attr_list);

	if (!driver)
		return -EINVAL;

	for (idx = 0; idx < num; idx++)
		driver_remove_file(driver, ltr2568_attr_list[idx]);

	return err;
}

/* ------------------------------------------------------------ interrupt -- */

/*
 * ltr2568_eint_handler() was reconstructed from the factory kernel disassembly (0xffffff8008783904).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static irqreturn_t ltr2568_eint_handler(int irq, void *desc)
{
	if (ltr2568_irq_enabled == 2) {
		disable_irq_nosync(ltr2568_obj->irq);
		if (ltr2568_obj)
			queue_work_on(8, system_wq, &ltr2568_obj->eint_work);
	}

	return IRQ_HANDLED;
}

/*
 * ltr2568_eint_work() was reconstructed from the factory kernel disassembly (544 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ltr2568_eint_work(struct work_struct *work)
{
	struct ltr2568_priv *obj =
		container_of(work, struct ltr2568_priv, eint_work);
	u8 databuf[2];
	int value;
	int res;

	obj->ps = ltr2568_ps_read(obj->client, &obj->ps);

	if (obj->ps > atomic_read(&obj->ps_thd_val_high)) {
		value = 0;
		ltr2568_intr_flag = 1;
	} else if (obj->ps < atomic_read(&obj->ps_thd_val_low)) {
		value = 1;
		ltr2568_intr_flag = 0;
	} else {
		value = 1;
	}

	if (atomic_read(&obj->ps_suspend)) {
		value = -1;	/* "12800013 mov" w19, #0xffffffff */
	} else if (atomic_read(&obj->ps_deb_on) == 1) {
		unsigned long endt = atomic_read(&obj->ps_deb_end);

		if (time_after(jiffies, endt))
			atomic_set(&obj->ps_deb_on, 0);

		if (atomic_read(&obj->ps_deb_on) == 1)
			value = -1;	/* "5a9f1273 csinv" w19, w19, wzr, ne */
	} else if (obj->als > 50000) {	/* "52986a09 mov" w9, #0xc350 */
		value = 1;
	}

	databuf[0] = 0x9c;		/* "52801389 mov" w9, #0x9c */
	if (ltr2568_intr_flag == 1) {
		databuf[1] = (u8)atomic_read(&obj->ps_thd_val_low);
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)		/* "7100041f cmp" w0, #0x1 ; "b.lt" */
			goto EXIT;
		databuf[0] = 0x9d;
		databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) >> 8);
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)
			goto EXIT;
		databuf[0] = 0x9a;
		databuf[1] = 0xff;
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)
			goto EXIT;
		databuf[0] = 0x9b;
		databuf[1] = 0xff;
	} else {
		databuf[1] = 0x01;
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)
			goto EXIT;
		databuf[0] = 0x9d;
		databuf[1] = 0x00;
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)
			goto EXIT;
		databuf[0] = 0x9a;	/* "52801348 mov" w8, #0x9a */
		databuf[1] = (u8)atomic_read(&obj->ps_thd_val_high);
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)
			goto EXIT;
		databuf[0] = 0x9b;	/* "52801368 mov" w8, #0x9b */
		databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) >> 8);
	}

	res = i2c_master_send(obj->client, databuf, 2);
	if (res <= 0)
		goto EXIT;

	ps_report_interrupt_data(value);

EXIT:
	enable_irq(obj->irq);
}

/*
 * Line 1921/1927/1941/1946/1954. "pin_cfg"@0xffffff80091bdee5,
 * "debounce"@0xffffff800911984a, "p-sensor"@0xffffff80091bdf24,
 * "ALS-eint"@0xffffff80091bdf5d.
 */
int ltr2568_setup_eint(struct i2c_client *client)
{
	int ret;
	struct pinctrl *pinctrl;
	struct pinctrl_state *pins_cfg;
	u32 ints[2] = {0, 0};

	pinctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(pinctrl)) {
		ret = PTR_ERR(pinctrl);
		/* "\x013[ALS/PS] %s %d : Cannot find alsps pinctrl!\n"@0xffffff80091bdea3 */
#line 1921
		APS_ERR("Cannot find alsps pinctrl!\n");
	}
	pins_cfg = pinctrl_lookup_state(pinctrl, "pin_cfg");
	if (IS_ERR(pins_cfg)) {
		ret = PTR_ERR(pins_cfg);
		/* "\x013[ALS/PS] %s %d : Cannot find alsps pinctrl pin_cfg!\n"@0xffffff80091bdeed */
		APS_ERR("Cannot find alsps pinctrl pin_cfg!\n");
	}

	if (ltr2568_obj->irq_node) {
		of_property_read_u32_array(ltr2568_obj->irq_node, "debounce",
					   ints, ARRAY_SIZE(ints));
		gpio_request(ints[0], "p-sensor");
		gpio_set_debounce(ints[0], ints[1]);
		pinctrl_select_state(pinctrl, pins_cfg);
		ltr2568_obj->irq = irq_of_parse_and_map(ltr2568_obj->irq_node, 0);
		if (!ltr2568_obj->irq) {
			/* "\x013[ALS/PS] %s %d : irq_of_parse_and_map fail!!\n"@0xffffff80091bdf2d */
#line 1941
			APS_ERR("irq_of_parse_and_map fail!!\n");
			return -EINVAL;	/* "128002a0 mov" w0, #0xffffffea */
		}
		if (request_irq(ltr2568_obj->irq, ltr2568_eint_handler,
				IRQF_TRIGGER_NONE, "ALS-eint", NULL)) {
			/* "\x013[ALS/PS] %s %d : IRQ LINE NOT AVAILABLE!!\n"@0xffffff80091bdf66 */
#line 1946
			APS_ERR("IRQ LINE NOT AVAILABLE!!\n");
			return -EINVAL;
		}
		enable_irq_wake(ltr2568_obj->irq);
		ltr2568_irq_enabled = 1;	/* "320003f3 orr" w19, wzr, #0x1 */
	} else {
		/* "\x013[ALS/PS] %s %d : null irq node!!\n"@0xffffff80091bdf93 */
#line 1954
		APS_ERR("null irq node!!\n");
		return -EINVAL;
	}

	return 0;
}

/* ------------------------------------------------------- sensor init -- */

/*
 * Line 2060/2064/2068. A sequence of register writes, each aborted at the
 * first error. The registers and the values come one by one from the
 * disassembly:
 *   0x7f <- 0x00   "32001be1 orr" w1, wzr, #0x7f
 *   0xb6 <- 0x00   "528016c1 mov" w1, #0xb6
 *   0xb7 <- 0x10   "528016e1 mov" w1, #0xb7 / "321c03e8 orr" w8, wzr, #0x10
 *   0xa4 <- 0x04   "52801481 mov" w1, #0xa4 / "321e03e8 orr" w8, wzr, #0x4
 *   0xad <- 0x18   "528015a1 mov" w1, #0xad / "321d07e8 orr" w8, wzr, #0x18
 *   0x83 <- 0xaf   "52801061 mov" w1, #0x83 / "528015f5 mov" w21, #0xaf
 *   0x82 <- 0xaf   "52801041 mov" w1, #0x82
 *   0x84 <- 0x03   "52801081 mov" w1, #0x84 / "320007e8 orr" w8, wzr, #0x3
 * then, if hw->polling_mode_ps is zero:
 *   0x98 <- 0x81   "52801301 mov" w1, #0x98 / "52801028 mov" w8, #0x81
 *   0x99 <- 0x10   "52801321 mov" w1, #0x99 / "321c03e8 orr" w8, wzr, #0x10
 * and in every case:
 *   0x80 <- ltr2568_als_gain | 0x20  "321b0108 orr" w8, w8, #0x20
 *   0x85 <- 0xa5   "528010a1 mov" w1, #0x85 / "528014a8 mov" w8, #0xa5
 */
static int ltr2568_sensor_init(void)
{
	struct ltr2568_priv *obj = ltr2568_obj;
	struct i2c_client *client = obj->client;
	u8 databuf[1];
	int res;

	databuf[0] = 0x00;
	res = ltr2568_master_send(client, 0x7f, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0x00;
	res = ltr2568_master_send(client, 0xb6, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0x10;
	res = ltr2568_master_send(client, 0xb7, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0x04;
	res = ltr2568_master_send(client, 0xa4, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0x18;
	res = ltr2568_master_send(client, 0xad, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0xaf;
	res = ltr2568_master_send(client, 0x83, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0xaf;
	res = ltr2568_master_send(client, 0x82, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0x03;
	res = ltr2568_master_send(client, 0x84, databuf);
	if (res < 0)
		goto EXIT_ERR;

	if (!obj->hw->polling_mode_ps) {
		databuf[0] = 0x81;
		res = ltr2568_master_send(client, 0x98, databuf);
		if (res < 0)
			goto EXIT_ERR;
		databuf[0] = 0x10;
		res = ltr2568_master_send(client, 0x99, databuf);
		if (res < 0)
			goto EXIT_ERR;
	}

	databuf[0] = ltr2568_als_gain | 0x20;
	ltr2568_master_send(client, 0x80, databuf);

	/* "\x013[ALS/PS] %s %d : ALS sensor gainrange %d!\n"@0xffffff80091be115 */
#line 2060
	APS_ERR("ALS sensor gainrange %d!\n", ltr2568_als_gainrange);

	databuf[0] = 0xa5;
	ltr2568_master_send(client, 0x85, databuf);

	/*
	 * The citation is on a single line on purpose: splitting it over two lines
	 * makes it invisible to verificacitazioni.py, a defect already seen.
	 * "\x013[ALS/PS] %s %d : ALS sensor integration & measurement rate: %d!\n"@0xffffff80091be156
	 * The third argument is the immediate "528000a3 mov" w3, #0x5.
	 */
#line 2064
	APS_ERR("ALS sensor integration & measurement rate: %d!\n", 5);
	return 0;

EXIT_ERR:
	/* "\x013[ALS/PS] %s %d : sensor init: %d\n"@0xffffff80091be199 */
#line 2068
	APS_ERR("sensor init: %d\n", res);
	return 1;	/* "320003e0 orr" w0, wzr, #0x1 */
}

/*
 * Line 2105/2112 -- the name comes from the `__func__` at 0xffffff80091be297.
 * In the factory build it is inlined into ltr2568_i2c_probe.
 *
 * The two registers touched in the middle are 0xb4 (written twice, with 0x1c
 * and then 0x1d) and 0xb9 (read): "52801681 mov" w1, #0xb4 ; "321e0be8 orr"
 * w8, wzr, #0x1c ; "528003a8 mov" w8, #0x1d ; "52801721 mov" w1, #0xb9.
 * The byte read indexes ltr2568_als_meas_rate through bits 3..5.
 */
static int ltr2568_init_client(void)
{
	struct i2c_client *client = ltr2568_obj->client;
	u8 databuf[1];
	int res;

	mdelay(200);	/* 200 rounds of __const_udelay(0x418958), that is udelay(1000) */

	res = ltr2568_sensor_init();
	if (res)
		goto EXIT_ERR;

	databuf[0] = 0x1c;
	res = ltr2568_master_send(client, 0xb4, databuf);
	if (res < 0)
		goto SETUP;
	databuf[0] = 0x1d;
	res = ltr2568_master_send(client, 0xb4, databuf);
	if (res < 0)
		goto SETUP;
	res = ltr2568_master_recv(client, 0xb9, databuf, 1);
	if (res < 0)
		goto SETUP;
	ltr2568_meas_rate = ltr2568_als_meas_rate[(databuf[0] >> 3) & 0x7];

SETUP:
	res = ltr2568_setup_eint(client);
	if (res != 0) {
		/* "\x013[ALS/PS] %s %d : setup eint: %d\n"@0xffffff80091be274 */
#line 2105
		APS_ERR("setup eint: %d\n", res);
		goto EXIT_ERR;
	}
	return 0;

EXIT_ERR:
	/* "\x013[ALS/PS] %s %d : init dev: %d\n"@0xffffff80091be2ab */
	APS_ERR("init dev: %d\n", res);
	return 1;
}

/* --------------------------------------- prossimita': reading and thresholds -- */

/*
 * ltr2568_ps_get_thres() was reconstructed from the factory kernel disassembly (0xffffff80091be228).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ltr2568_ps_get_thres(u16 ps)
{
	struct ltr2568_priv *obj = ltr2568_obj;
	int high, low;

	if (!obj) {
		/* "\x013[ALS/PS] %s %d : ltr2568_obj is null!!\n"@0xffffff80091be1fe */
		APS_ERR("ltr2568_obj is null!!\n");
		return;
	}
	if (ps == 0)		/* "340004d4 cbz" w20, ... */
		return;

	if (ps < 100) {
		high = ps + 100;	/* "11019288 add" w8, w20, #0x64 */
		low = ps + 50;		/* "1100ca8a add" w10, w20, #0x32 */
	} else if (ps < 200) {
		high = ps + 150;	/* "11025a88 add" w8, w20, #0x96 */
		low = ps + 60;		/* "1100f28a add" w10, w20, #0x3c */
	} else if (ps < 300) {
		high = ps + 150;
		low = ps + 60;
	} else if (ps < 400) {
		high = ps + 150;
		low = ps + 60;
	} else if (ps < 600) {
		high = ps + 180;	/* "1102d288 add" w8, w20, #0xb4 */
		low = ps + 90;		/* "11016a8a add" w10, w20, #0x5a */
	} else if (ps < 1000) {
		high = ps + 300;	/* "1104b288 add" w8, w20, #0x12c */
		low = ps + 180;		/* "1102d28a add" w10, w20, #0xb4 */
	} else {
		high = 1600;
		low = 1400;
	}

	atomic_set(&obj->ps_thd_val_high, high);	/* "b9010128 str" w8,[x9,#256] */
	atomic_set(&obj->ps_thd_val_low, low);		/* "b901052a str" w10,[x9,#260] */
	ltr2568_thres_override = true;			/* "3904512b strb" w11,[x9,#276] */
	ltr2568_thres_low = low;			/* "b901118a str" w10,[x12,#272] */
	ltr2568_thres_high = high;			/* "b9010da8 str" w8,[x13,#268] */
}

/*
 * ltr2568_ps_read() was reconstructed from the factory kernel disassembly (1168 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ltr2568_ps_read(struct i2c_client *client, u16 *data)
{
	u8 databuf[2];
	u8 cfgbuf[2];
	u8 offbuf[2];
	u8 status;
	u16 ps;
	u16 sum;
	u16 avg;
	u16 lo, hi;
	int idx;
	int i;
	int res;

	ltr2568_master_recv(client, 0x83, databuf, 1);
	if (databuf[0] != 0xaf) {
		ltr2568_sensor_init();
		if (ltr2568_als_on == 1) {
			/* "52800429 mov" w9, #0x21 ; "2a090115 orr" w21, w8, w9 */
			cfgbuf[0] = ltr2568_als_gain | 0x21;
			ltr2568_master_send(client, 0x80, cfgbuf);
		}
		if (ltr2568_ps_on == 1) {
			/* "121e0ea8 and" w8, w21, #0x3c ; "52800349 mov" w9, #0x1a */
			cfgbuf[0] = (cfgbuf[0] & 0x3c) | 0x1a;
			ltr2568_master_send(client, 0x81, cfgbuf);
		}
		mdelay(10);	/* ten __const_udelay(0x418958) */
	}

	ltr2568_master_recv(client, 0x91, databuf, 1);
	status = databuf[0];
	ltr2568_master_recv(client, 0x92, databuf, 2);
	if (status & 0x04)	/* "371007f5 tbnz" w21, #2 */
		goto zero;

	ps = databuf[0] | ((databuf[1] & 0x07) << 8);

	if (ps == 0) {
		/*
		 * Four null readings in a row lower the offset by 300 and
		 * reopen the calibration window.
		 */
		if (++ltr2568_ps_zero_cnt < 4)	/* "7100111f cmp" w8, #0x4 */
			goto zero;
		ltr2568_ps_zero_cnt = 0;
		if (ltr2568_ps_offset == 0)
			goto zero;

		ltr2568_ps_cali_on = true;
		ltr2568_ps_idx = 0;
		ltr2568_ps_full = false;
		ltr2568_ps_stable = 0;

		/* "7104b129 subs" w9, w9, #0x12c ; "1a9fc135 csel" w21, w9, wzr, gt */
		if (ltr2568_ps_offset - 300 > 0)
			ltr2568_ps_offset = ltr2568_ps_offset - 300;
		else
			ltr2568_ps_offset = 0;

		client = ltr2568_obj->client;	/* "f9400554 ldr" x20,[x10,#8] */
		offbuf[0] = (u8)ltr2568_ps_offset;
		res = ltr2568_master_send(client, 0x9e, offbuf);
		if (res >= 0) {
			offbuf[0] = (u8)(ltr2568_ps_offset >> 8);
			ltr2568_master_send(client, 0x9f, offbuf);
		}
		goto zero;
	}

	ltr2568_ps_zero_cnt = 0;
	if (ltr2568_ps_cali_on == 1) {
		if (ps >= atomic_read(&ltr2568_obj->ps_thd_val_low))
			goto out;
	} else {
		/* "51002929 sub" w9, w9, #0xa */
		if (ps >= ltr2568_ps_base - 10)
			goto out;
	}

	if (ltr2568_ps_idx >= 8)	/* "f100215f cmp" x10, #0x8 */
		ltr2568_ps_full = true;

	if (ltr2568_ps_full) {
		idx = ltr2568_ps_idx & 0x07;	/* "9240094a and" x10, x10, #0x7 */
		ltr2568_ps_buf[idx] = ps;

		sum = 0;
		for (i = 0; i < 8; i++)
			sum += ltr2568_ps_buf[i];
		/*
		 * "53033c63 ubfx" w3, w3, #3, #13: the sum is truncated to
		 * sixteen bits BEFORE the division, that is, it is a u16 in the
		 * source too -- with an int the field would have been wider.
		 */
		avg = sum >> 3;

		/*
		 * "529ffecd mov" w13, #0xfff6 with "12003dad and" w13, w13,
		 * #0xffff: the lower bound is computed at SIXTEEN bits and can
		 * therefore wrap; the upper one cannot ("11002863 add" w3, w3,
		 * #0xa with no mask, because avg fits in thirteen bits).
		 */
		lo = avg - 10;
		hi = avg + 10;
		for (i = 0; i < 8; i++) {
			if (ltr2568_ps_buf[i] > lo && ltr2568_ps_buf[i] < hi)
				ltr2568_ps_stable++;
			else
				ltr2568_ps_stable = 0;
		}

		ltr2568_ps_idx = idx + 1;
		if (ltr2568_ps_stable < 7)	/* "71001d9f cmp" w12, #0x7 */
			goto out;
	} else {
		ltr2568_ps_buf[ltr2568_ps_idx] = ps;
		ltr2568_ps_stable = 0;
		ltr2568_ps_idx = ltr2568_ps_idx + 1;
		goto out;
	}

	ltr2568_ps_cali_on = false;	/* "3903611f strb" wzr,[x8,#216] */
	ltr2568_ps_base = ps;		/* "b900dd14 str" w20,[x8,#220] */
	ltr2568_ps_get_thres(ps);
	ltr2568_ps_set_thres();
	goto out;

zero:
	ps = 0;			/* "2a1f03f4 mov" w20, wzr */
out:
	*data = ps;		/* "79000274 strh" w20,[x19] */
	return ps;
}

/* ---------------------------------------------------- luce: settling -- */

/*
 * ltr2568_als_filter() was reconstructed from the factory kernel disassembly (0xffffff8008785b24).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ltr2568_als_filter(int cur, int prev)
{
	int diff = cur - prev;
	int adiff = abs(diff);
	int step;
	int r;

	if (adiff > 500)
		return cur;

	step = diff * adiff * 70 / (adiff * 7 + 1000);
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008785b8c, 52 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	if (abs(step) % 10 >= 5) {
		if (step > 0)
			r = step / 10 + 1;
		else
			r = step / 10 - 1;
	} else {
		r = step / 10;
	}

	if (abs(r) >= 1)
		return prev + r;
	if (adiff < 3)
		return cur;
	if (diff >= 1)		/* "7100059f cmp" w12, #0x1 */
		return prev + 2;
	return prev - 2;
}

/*
 * ltr2568_als_read() was reconstructed from the factory kernel disassembly (1268 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ltr2568_als_read(struct i2c_client *client, u16 *data)
{
	u8 databuf[4];
	u8 cfgbuf[2];
	int als0, als1;
	int val0, val1;
	int lux;
	int idx;
	int res;

	res = ltr2568_master_recv(client, 0x88, databuf, 1);
	if (res < 0)
		goto last;

	/* "d3431528 ubfx" x8, x9, #3, #3 -- three bits over a table of four */
	ltr2568_als_gainrange = ltr2568_als_gain_table[(databuf[0] >> 3) & 0x07];

	if (databuf[0] & 0x40) {	/* "37300369 tbnz" w9, #6 */
		switch (ltr2568_als_gainrange) {
		case 64:
			idx = 2;
			break;
		case 16:
			idx = 1;
			break;
		case 4:
			idx = 0;
			break;
		case 1:
			goto last;
		default:
			idx = -2;
			break;
		}
		/* "531e76b8 lsl" w24, w21, #2 ; "52800428 mov" w8, #0x21 */
		cfgbuf[0] = (idx << 2) | 0x21;
		ltr2568_master_send(client, 0x80, cfgbuf);
		ltr2568_als_gain = idx << 2;
		ltr2568_als_first = false;
		ltr2568_als_gainrange = ltr2568_als_gain_table[idx];
		goto last;
	}

	res = ltr2568_master_recv(client, 0x89, databuf, 4);
	if (res < 0)
		goto last;
	/* "33181d15 bfi" w21, w8, #8, #8 e "33181d39 bfi" w25, w9, #8, #8 */
	als0 = databuf[0] | (databuf[1] << 8);
	als1 = databuf[2] | (databuf[3] << 8);

	if (ltr2568_als_gainrange == 1) {
		res = ltr2568_master_recv(client, 0x95, cfgbuf, 1);
		if (res < 0)
			cfgbuf[0] = 1;
		if (cfgbuf[0] < 1)	/* "1a9f8518 csinc" w24, w8, wzr, hi */
			cfgbuf[0] = 1;
		/* "0b183f28 add" w8, w25, w24, lsl #15 ; "51402119 sub" w25, w8, #0x8, lsl #12 */
		als1 = als1 + cfgbuf[0] * 32768 - 32768;
	}

	if (als1 == 0)		/* "1a9503e8 csel" w8, wzr, w21, eq */
		als0 = 0;

	if (ltr2568_als_first) {
		val1 = ltr2568_als_filter(als1, ltr2568_als_prev1);
		ltr2568_als_prev1 = val1;
		val0 = ltr2568_als_filter(als0, ltr2568_als_prev0);
	} else {
		ltr2568_als_first = true;
		ltr2568_als_prev1 = als1;
		val1 = als1;
		val0 = als0;
	}
	ltr2568_als_prev0 = val0;

	val1 -= (ltr2568_als_gainrange * ltr2568_meas_rate) / 512;
	if (val1 > 0)
		lux = (val1 * 14560 / ltr2568_als_gainrange) / 1000;
	else
		lux = 0;

	if (als1 < 50 || als0 < 50) {
		switch (ltr2568_als_gainrange) {
		case 1:
			idx = 1;
			break;
		case 4:
			idx = 2;
			break;
		case 16:
			idx = 3;
			break;
		case 64:
			goto out;
		default:
			idx = 0;
			break;
		}
		/* "331e0728 bfi" w8, w25, #2, #2 -- here the index is in two bits */
		cfgbuf[0] = (idx << 2) | 0x21;
		ltr2568_master_send(client, 0x80, cfgbuf);
		ltr2568_als_gain = idx << 2;
		ltr2568_als_first = false;
		ltr2568_als_gainrange = ltr2568_als_gain_table[idx];
	} else if (als1 > 50000 || als0 > 50000) {
		switch (ltr2568_als_gainrange) {
		case 64:
			idx = 2;
			break;
		case 16:
			idx = 1;
			break;
		case 4:
			idx = 0;
			break;
		case 1:
			goto out;
		default:
			idx = -2;
			break;
		}
		cfgbuf[0] = (idx << 2) | 0x21;
		ltr2568_master_send(client, 0x80, cfgbuf);
		ltr2568_als_gain = idx << 2;
		ltr2568_als_first = false;
		ltr2568_als_gainrange = ltr2568_als_gain_table[idx];
	}
	goto out;

last:
	lux = ltr2568_als_last;
out:
	*data = lux;			/* "79000275 strh" w21,[x19] */
	ltr2568_als_last = lux;		/* "b900fd15 str" w21,[x8,#252] */
	return lux;
}

/* --------------------------------------------------- proximity: power-on -- */

/*
 * ltr2568_ps_enable() was reconstructed from the factory kernel disassembly (0xffffff80091be774).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ltr2568_ps_enable(struct i2c_client *client, int enable)
{
	u8 databuf[2];
	u8 gainbuf[2];
	int res;

	ltr2568_master_recv(client, 0x81, databuf, 1);
	databuf[0] = (databuf[0] & 0x26) | 0x18;

	if (enable) {
		databuf[0] |= 0x02;
		ltr2568_ps_on = true;
		ltr2568_ps_zero_cnt = 0;
		ltr2568_ps_cali_on = false;
	} else {
		databuf[0] &= 0x3c;
		ltr2568_ps_on = false;
		/* "321b0108 orr" w8, w8, #0x20 */
		gainbuf[0] = ltr2568_als_gain | 0x20;
		ltr2568_master_send(client, 0x80, gainbuf);
	}

	res = ltr2568_master_send(client, 0x81, databuf);
	if (res < 0)
		/* "\x013[ALS/PS] %s %d : PS: enable ps err: %d en: %d \n"@0xffffff80091be742 */
#line 833
		APS_ERR("PS: enable ps err: %d en: %d \n", res, enable);

	return res;
}

/* ------------------------------------------------ alsps framework ops -- */

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int als_open_report_data(int open)
{
	return 0;
}

/* Line 2133/2139. "als_enable_nodata"@0xffffff80091be606 */
static int als_enable_nodata(int en)
{
	int res = 0;

	if (!ltr2568_obj) {
		APS_ERR("ltr2568_obj is null!!\n");
		return -1;
	}

	res = ltr2568_als_enable(ltr2568_obj->client, en);
	if (res) {
		/* "\x013[ALS/PS] %s %d : als_enable_nodata is failed!!\n"@0xffffff80091be618 */
#line 2139
		APS_ERR("als_enable_nodata is failed!!\n");
		return -1;
	}

	mutex_lock(&ltr2568_mutex);
	if (en)
		set_bit(CMC_BIT_ALS, &ltr2568_obj->enable);
	else
		clear_bit(CMC_BIT_ALS, &ltr2568_obj->enable);
	mutex_unlock(&ltr2568_mutex);
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int als_set_delay(u64 ns)
{
	return 0;
}

/*
 * Eight bytes: "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret". In the factory build
 * als_batch does NOT call als_set_delay (which does exist as a separate
 * symbol): it returns zero directly.
 */
static int als_batch(int flag, int64_t samplingPeriodNs,
		     int64_t maxBatchReportLatencyNs)
{
	return 0;
}

static int als_flush(void)
{
	return als_flush_report();
}

/* Line 2180. "als_get_data"@0xffffff80091be691 */
static int als_get_data(int *value, int *status)
{
	int err = 0;

	if (!ltr2568_obj) {
		APS_ERR("ltr2568_obj is null!!\n");
		return -1;
	}

	ltr2568_obj->als = ltr2568_als_read(ltr2568_obj->client,
					    &ltr2568_obj->als);
	*value = ltr2568_get_als_value(ltr2568_obj, ltr2568_obj->als);
	if (*value < 0)
		err = -1;
	/* "321f03e8 orr" w8, wzr, #0x2 -- SENSOR_STATUS_ACCURACY_MEDIUM */
	*status = SENSOR_STATUS_ACCURACY_MEDIUM;

	return err;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ps_open_report_data(int open)
{
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ps_set_delay(u64 ns)
{
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ps_batch(int flag, int64_t samplingPeriodNs,
		    int64_t maxBatchReportLatencyNs)
{
	return 0;
}

static int ps_flush(void)
{
	return ps_flush_report();
}

/*
 * ps_enable_nodata() was reconstructed from the factory kernel disassembly (0xffffff80091be700, 1164 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ps_enable_nodata(int en)
{
	struct ltr2568_priv *obj = ltr2568_obj;
	struct i2c_client *client;
	u8 databuf[2];
	u8 offbuf[2];
	int res = 0;
	int sum = 0;
	int avg;
	int i;

	if (!obj) {
		/* "\x013[ALS/PS] %s %d : ltr2568_obj is null!!\n"@0xffffff80091be1fe */
#line 2212
		APS_ERR("ltr2568_obj is null!!\n");
		return -1;
	}
	client = obj->client;

	/*
	 * If the PS is already in the requested state the factory does NOT touch
	 * the bus, but it updates the enable bit all the same: the two jumps
	 * ("37001c48 tbnz" w8, #0 and "37001be8 tbnz" w8, #0 after
	 * "52000108 eor" w8, w8, #0x1) both point at the mutex block.
	 */
	if ((en && ltr2568_ps_on) || (!en && !ltr2568_ps_on))
		goto ENABLE_BIT;

	res = ltr2568_ps_enable(client, en);
	if (res < 0)
		goto EXIT_ERR;

	if (!en && ltr2568_als_on) {
		databuf[0] = ltr2568_als_gain | 0x21;
		ltr2568_master_send(client, 0x80, databuf);
	}
	mdelay(10);

	if (en && !ltr2568_obj->hw->polling_mode_ps) {
		client = ltr2568_obj->client;
		offbuf[0] = 0;
		res = ltr2568_master_send(client, 0x9e, offbuf);
		if (res >= 0) {
			offbuf[0] = 0;
			ltr2568_master_send(client, 0x9f, offbuf);
		}

		databuf[0] = 0x02;	/* "321f03e9 orr" w9, wzr, #0x2 */
		res = ltr2568_master_send(ltr2568_obj->client, 0x84, databuf);
		if (res < 0) {
			res = -1;
		} else {
			for (i = 0; i < 5; i++) {	/* "710016bf cmp" w21, #0x5 */
				msleep(30);	/* "321f0fe0 orr" w0, wzr, #0x1e */
				res = ltr2568_ps_read(ltr2568_obj->client,
						      &ltr2568_obj->ps);
				if (res < 0) {
					i--;
					continue;
				}
				sum += res;
			}

			avg = sum / 5;
			if (avg > 300) {
				/* "5104b2b7 sub" w23, w21, #0x12c */
				ltr2568_ps_offset = avg - 300;
				avg = 300;	/* "52802595 mov" w21, #0x12c */
			} else {
				ltr2568_ps_offset = 0;
			}
			ltr2568_ps_base = avg;

			client = ltr2568_obj->client;
			offbuf[0] = (u8)ltr2568_ps_offset;
			res = ltr2568_master_send(client, 0x9e, offbuf);
			if (res >= 0) {
				offbuf[0] = (u8)(ltr2568_ps_offset >> 8);
				ltr2568_master_send(client, 0x9f, offbuf);
			}

			ltr2568_ps_get_thres(avg);

			databuf[0] = 0x03;	/* "320007e9 orr" w9, wzr, #0x3 */
			res = ltr2568_master_send(ltr2568_obj->client, 0x84,
						  databuf);
			/* "131f7c14 asr" w20, w0, #31 */
			res = (res < 0) ? -1 : 0;
		}
		ltr2568_ps_set_thres();
	}

	/* "b940b909 ldr" w9,[x8,#184] ; "7100053f cmp" w9, #0x1 */
	if (en && ltr2568_irq_enabled == 1)
		ltr2568_irq_enabled = 2;

	if (res < 0)
		goto EXIT_ERR;

ENABLE_BIT:
	mutex_lock(&ltr2568_mutex);
	if (en)
		set_bit(CMC_BIT_PS, &ltr2568_obj->enable);
	else
		clear_bit(CMC_BIT_PS, &ltr2568_obj->enable);
	mutex_unlock(&ltr2568_mutex);
	return 0;

EXIT_ERR:
	/* "\x013[ALS/PS] %s %d : ps_enable_nodata is failed!!\n"@0xffffff80091be711 */
#line 2218
	APS_ERR("ps_enable_nodata is failed!!\n");
	return -1;
}

/*
 * Line 2254. "ps_get_data"@0xffffff80091be786. The threshold of 50000 is
 * "52986a0a mov" w10, #0xc350.
 */
static int ps_get_data(int *value, int *status)
{
	int err = 0;
	struct ltr2568_priv *obj = ltr2568_obj;

	if (!obj) {
#line 2254
		APS_ERR("ltr2568_obj is null!!\n");
		return -1;
	}

	obj->ps = ltr2568_ps_read(obj->client, &obj->ps);
	if (obj->ps > atomic_read(&obj->ps_thd_val_high)) {
		*value = 0;
		ltr2568_intr_flag = 1;
	} else if (obj->ps < atomic_read(&obj->ps_thd_val_low)) {
		*value = 1;
		ltr2568_intr_flag = 0;
	} else {
		*value = 1;
	}

	if (atomic_read(&obj->ps_suspend)) {
		*value = -1;
		err = -1;
	} else if (atomic_read(&obj->ps_deb_on) == 1) {
		unsigned long endt = atomic_read(&obj->ps_deb_end);

		if (time_after(jiffies, endt))
			atomic_set(&obj->ps_deb_on, 0);

		if (atomic_read(&obj->ps_deb_on) == 1) {
			*value = -1;
			err = -1;
		}
	} else if (obj->als > 50000) {
		*value = 1;
	}

	*status = SENSOR_STATUS_ACCURACY_MEDIUM;
	return err;
}

/* ----------------------------------------------------------- factory ops -- */

/*
 * Line 2279. "\x013[ALS/PS] %s %d : %s:%s failed\n"@0xffffff80091be2cc,
 * "enable"@0xffffff800924755d, "disable"@0xffffff8009247611. The choice between
 * the two literals is "9a881124 csel" x4, x9, x8, ne.
 */
static int ltr2568_als_factory_enable_sensor(bool enable_disable,
					     int64_t sample_periods_ms)
{
	int err;

	err = als_enable_nodata(enable_disable ? 1 : 0);
	if (err) {
		APS_ERR("%s:%s failed\n", __func__,
#line 2279
			enable_disable ? "enable" : "disable");
		return -1;
	}
	return 0;
}

static int ltr2568_als_factory_get_data(int32_t *data)
{
	int status;

	return als_get_data(data, &status);
}

/* Line 2301/2307. "\x013[ALS/PS] %s %d : obj is null!!\n"@0xffffff80091be30f */
static int ltr2568_als_factory_get_raw_data(int32_t *data)
{
	int err;

	if (!ltr2568_obj) {
#line 2301
		APS_ERR("obj is null!!\n");
		return -1;
	}
	err = ltr2568_als_read(ltr2568_obj->client, &ltr2568_obj->als);
	if (err < 0) {
		/* "\x013[ALS/PS] %s %d : %s failed\n"@0xffffff80091be352 */
		APS_ERR("%s failed\n", __func__);
		return -1;
	}
	*data = ltr2568_obj->als;
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ltr2568_als_factory_enable_calibration(void)
{
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ltr2568_als_factory_clear_cali(void)
{
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ltr2568_als_factory_set_cali(int32_t offset)
{
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ltr2568_als_factory_get_cali(int32_t *offset)
{
	return 0;
}

/* Line 2338. */
static int ltr2568_ps_factory_enable_sensor(bool enable_disable,
					    int64_t sample_periods_ms)
{
	int err;

	err = ps_enable_nodata(enable_disable ? 1 : 0);
	if (err) {
		APS_ERR("%s:%s failed\n", __func__,
#line 2338
			enable_disable ? "enable" : "disable");
		return -1;
	}
	return 0;
}

/*
 * "b90007ff str" wzr,[sp,#4] clears the local before the call, and
 * "131f7c00 asr" w0, w0, #31 turns the result of ps_get_data into 0 or -1.
 */
static int ltr2568_ps_factory_get_data(int32_t *data)
{
	int status = 0;

	return ps_get_data(data, &status) < 0 ? -1 : 0;
}

/* Line 2364. */
static int ltr2568_ps_factory_get_raw_data(int32_t *data)
{
	int err;

	err = ltr2568_ps_read(ltr2568_obj->client, &ltr2568_obj->ps);
	if (err < 0) {
#line 2364
		APS_ERR("%s failed\n", __func__);
		return -1;
	}
	*data = ltr2568_obj->ps;
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ltr2568_ps_factory_enable_calibration(void)
{
	return 0;
}

/* "b900f51f str" wzr,[x8,#244] */
static int ltr2568_ps_factory_clear_cali(void)
{
	ltr2568_obj->ps_cali = 0;
	return 0;
}

/* "b900f500 str" w0,[x8,#244] */
static int ltr2568_ps_factory_set_cali(int32_t offset)
{
	ltr2568_obj->ps_cali = offset;
	return 0;
}

/* "b940f508 ldr" w8,[x8,#244] ; "b9000008 str" w8,[x0] */
static int ltr2568_ps_factory_get_cali(int32_t *offset)
{
	*offset = ltr2568_obj->ps_cali;
	return 0;
}

/*
 * Line 2407.
 * "\x013[ALS/PS] %s %d : set_psensor_threshold fail\n"@0xffffff80091be3b1
 */
static int ltr2568_ps_factory_set_threshold(int32_t threshold[2])
{
	struct ltr2568_priv *obj = ltr2568_obj;

	atomic_set(&obj->ps_thd_val_high, obj->ps_cali + threshold[0]);
	atomic_set(&obj->ps_thd_val_low, threshold[1] + obj->ps_cali);
	if (ltr2568_ps_set_thres() <= 0) {	/* "7100001f cmp" w0, #0x0 ; "5400008d b.le" */
#line 2407
		APS_ERR("set_psensor_threshold fail\n");
		return -1;
	}
	return 0;
}

/* "4b0a0129 sub" w9, w9, w10 -- the thresholds come back net of the calibration. */
static int ltr2568_ps_factory_get_threshold(int32_t threshold[2])
{
	struct ltr2568_priv *obj = ltr2568_obj;

	threshold[0] = atomic_read(&obj->ps_thd_val_high) - obj->ps_cali;
	threshold[1] = atomic_read(&obj->ps_thd_val_low) - obj->ps_cali;
	return 0;
}

static struct alsps_factory_fops ltr2568_factory_fops = {	/* 0xffffff800992e698 */
	.als_enable_sensor = ltr2568_als_factory_enable_sensor,
	.als_get_data = ltr2568_als_factory_get_data,
	.als_get_raw_data = ltr2568_als_factory_get_raw_data,
	.als_enable_calibration = ltr2568_als_factory_enable_calibration,
	.als_clear_cali = ltr2568_als_factory_clear_cali,
	.als_set_cali = ltr2568_als_factory_set_cali,
	.als_get_cali = ltr2568_als_factory_get_cali,

	.ps_enable_sensor = ltr2568_ps_factory_enable_sensor,
	.ps_get_data = ltr2568_ps_factory_get_data,
	.ps_get_raw_data = ltr2568_ps_factory_get_raw_data,
	.ps_enable_calibration = ltr2568_ps_factory_enable_calibration,
	.ps_clear_cali = ltr2568_ps_factory_clear_cali,
	.ps_set_cali = ltr2568_ps_factory_set_cali,
	.ps_get_cali = ltr2568_ps_factory_get_cali,
	.ps_set_threshold = ltr2568_ps_factory_set_threshold,
	.ps_get_threshold = ltr2568_ps_factory_get_threshold,
};

/*
 * 0xffffff800992e658: two u32s at 1 and 1 ("01 00 00 00 01 00 00 00") followed
 * by the pointer to the fops.
 */
static struct alsps_factory_public ltr2568_factory_device = {
	.gain = 1,
	.sensitivity = 1,
	.fops = &ltr2568_factory_fops,
};

/* ------------------------------------------------------- driver i2c -- */

/* Line 2461/2538/2550/2566/2575/2591/2600/2616. */
static int ltr2568_i2c_probe(struct i2c_client *client,
			     const struct i2c_device_id *id)
{
	struct ltr2568_priv *obj = NULL;
	struct als_control_path als_ctl = {0};
	struct als_data_path als_data = {0};
	struct ps_control_path ps_ctl = {0};
	struct ps_data_path ps_data = {0};
	int err = 0;

	client->addr = 0x23;	/* "52800468 mov" w8, #0x23 ; "79000688 strh" w8,[x20,#2] */
	err = get_alsps_dts_func(client->dev.of_node, hw);
	if (err < 0) {
		/* "\x013[ALS/PS] %s %d : get customization info from dts failed\n"@0xffffff80091bdff7 */
#line 2461
		APS_ERR("get customization info from dts failed\n");
		err = -EFAULT;	/* "128001b4 mov" w20, #0xfffffff2 */
		goto exit;
	}

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj) {
		err = -ENOMEM;	/* "12800174 mov" w20, #0xfffffff4 */
		goto exit;
	}
	memset(obj, 0, sizeof(*obj));
	ltr2568_obj = obj;
	obj->hw = hw;
	INIT_WORK(&obj->eint_work, ltr2568_eint_work);
	obj->client = client;
	i2c_set_clientdata(client, obj);

	atomic_set(&obj->als_debounce, 300);	/* "5280258b mov" w11, #0x12c */
	atomic_set(&obj->als_deb_on, 0);
	atomic_set(&obj->als_deb_end, 0);
	atomic_set(&obj->ps_debounce, 300);
	atomic_set(&obj->ps_deb_on, 0);
	atomic_set(&obj->ps_deb_end, 0);
	atomic_set(&obj->ps_mask, 0);
	atomic_set(&obj->als_suspend, 0);
	atomic_set(&obj->ps_thd_val_high, obj->hw->ps_threshold_high);
	atomic_set(&obj->ps_thd_val_low, obj->hw->ps_threshold_low);
	obj->ps_threshold = obj->hw->ps_threshold;
	atomic_set(&obj->als_thd_val_high, obj->hw->als_threshold_high);
	atomic_set(&obj->als_thd_val_low, obj->hw->als_threshold_low);

	obj->irq_node = client->dev.of_node;
	obj->enable = 0;
	obj->pending_intr = 0;
	obj->als_level_num = ARRAY_SIZE(obj->hw->als_level);
	obj->als_value_num = ARRAY_SIZE(obj->hw->als_value);
	obj->als_modulus = (400 * 100) / (16 * 150);	/* = 16, "321c03ea orr" w10, wzr, #0x10 */

	memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
	memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
	obj->ps_cali = 0;
	atomic_set(&obj->i2c_retry, 3);		/* "320007ea orr" w10, wzr, #0x3 */
	clear_bit(CMC_BIT_ALS, &obj->enable);
	clear_bit(CMC_BIT_PS, &obj->enable);

	ltr2568_i2c_client = client;
	err = ltr2568_init_client();
	if (err)
		goto exit_init_failed;

	err = alsps_factory_device_register(&ltr2568_factory_device);
	if (err) {
		/* "\x013[ALS/PS] %s %d : ltr2568_device register failed\n"@0xffffff80091be044 */
#line 2538
		APS_ERR("ltr2568_device register failed\n");
		goto exit_init_failed;
	}

	als_ctl.is_use_common_factory = false;
	ps_ctl.is_use_common_factory = false;
	err = ltr2568_create_attr(&(ltr2568_i2c_driver.driver));
	if (err) {
		/* "\x013[ALS/PS] %s %d : create attribute err = %d\n"@0xffffff80091be077 */
#line 2550
		APS_ERR("create attribute err = %d\n", err);
		goto exit_init_failed;
	}

	als_ctl.open_report_data = als_open_report_data;
	als_ctl.enable_nodata = als_enable_nodata;
	als_ctl.set_delay = als_set_delay;
	als_ctl.batch = als_batch;
	als_ctl.flush = als_flush;
	als_ctl.is_report_input_direct = false;
	als_ctl.is_support_batch = false;
	err = als_register_control_path(&als_ctl);
	if (err) {
		/* "\x013[ALS/PS] %s %d : register fail = %d\n"@0xffffff80091be0a5 */
#line 2566
		APS_ERR("register fail = %d\n", err);
		goto exit_init_failed;
	}

	als_data.get_data = als_get_data;
	als_data.vender_div = 100;	/* "52800c89 mov" w9, #0x64 */
	err = als_register_data_path(&als_data);
	if (err) {
#line 2575
		APS_ERR("register fail = %d\n", err);
		goto exit_init_failed;
	}

	ps_ctl.open_report_data = ps_open_report_data;
	ps_ctl.enable_nodata = ps_enable_nodata;
	ps_ctl.set_delay = ps_set_delay;
	ps_ctl.batch = ps_batch;
	ps_ctl.flush = ps_flush;
	ps_ctl.is_report_input_direct = false;
	ps_ctl.is_support_batch = false;
	ps_ctl.is_polling_mode = obj->hw->polling_mode_ps;
	err = ps_register_control_path(&ps_ctl);
	if (err) {
#line 2591
		APS_ERR("register fail = %d\n", err);
		goto exit_init_failed;
	}

	ps_data.get_data = ps_get_data;
	ps_data.vender_div = 100;
	err = ps_register_data_path(&ps_data);
	if (err) {
		/* "\x013[ALS/PS] %s %d : tregister fail = %d\n"@0xffffff80091be0cc */
		APS_ERR("tregister fail = %d\n", err);
		goto exit_init_failed;
	}

	ltr2568_init_flag = true;	/* "3902f109 strb" w9,[x8,#188], w9 = 1 */
	return 0;

exit_init_failed:
	kfree(obj);
exit:
	/* "\x013[ALS/PS] %s %d : %s: err = %d\n"@0xffffff80091be0f4 */
#line 2616
	APS_ERR("%s: err = %d\n", __func__, err);
	ltr2568_init_flag = false;	/* "3902f11f strb" wzr,[x8,#188] */
	return err;
}

/*
 * The ten driver_remove_file calls are unrolled: that is ltr2568_delete_attr
 * inlined. There is no check of its outcome, and no clearing of
 * ltr2568_i2c_client.
 */
static int ltr2568_i2c_remove(struct i2c_client *client)
{
	ltr2568_delete_attr(&(ltr2568_i2c_driver.driver));
	alsps_factory_device_deregister(&ltr2568_factory_device);
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}

/*
 * Eight bytes of name copied with a single 64-bit store:
 * "d28e8d88 mov" x8, #0x746c ; "f2a64e48 movk" x8, #0x3272, lsl #16 ;
 * "f2c6c6a8 movk" x8, #0x3635, lsl #32 ; "f2e00708 movk" x8, #0x38, lsl #48
 * = 'ltr2568\0'.
 */
static int ltr2568_i2c_detect(struct i2c_client *client,
			      struct i2c_board_info *info)
{
	strcpy(info->type, LTR2568_DEV_NAME);
	return 0;
}

/*
 * Line 2656/2664. "f9404c08 ldr" x8,[x0,#152] is i2c_get_clientdata(dev) done
 * on a `struct device *` (the offset of dev.driver_data): the signature is
 * dev_pm_ops's, not i2c_driver's old one.
 */
static int ltr2568_i2c_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ltr2568_priv *obj = i2c_get_clientdata(client);
	int err;

	if (!obj) {
		/* "\x013[ALS/PS] %s %d : null pointer!!\n"@0xffffff80091be792 */
#line 2656
		APS_ERR("null pointer!!\n");
		return -EINVAL;	/* "128002a0 mov" w0, #0xffffffea */
	}
	atomic_set(&obj->als_suspend, 1);
	err = ltr2568_als_enable(obj->client, 0);
	if (err < 0) {
		/* "\x013[ALS/PS] %s %d : disable als: %d\n"@0xffffff80091be7c9 */
#line 2664
		APS_ERR("disable als: %d\n", err);
		return err;
	}
	return 0;
}

/* Line 2692/2714. */
static int ltr2568_i2c_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ltr2568_priv *obj = i2c_get_clientdata(client);
	int err;

	if (!obj) {
#line 2692
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}
	atomic_set(&obj->als_suspend, 0);

	if (test_bit(CMC_BIT_ALS, &obj->enable)) {
		err = ltr2568_als_enable(obj->client, 1);
		if (err < 0) {
			/* "\x013[ALS/PS] %s %d : enable als fail: %d\n"@0xffffff80091be800 */
#line 2714
			APS_ERR("enable als fail: %d\n", err);
		}
	}
	return 0;
}

/* "3902f11f strb" wzr,[x8,#188] -- it clears the flag, it does not set it to -1. */
static int ltr2568_remove(void)
{
	i2c_del_driver(&ltr2568_i2c_driver);
	ltr2568_init_flag = false;
	return 0;
}

/*
 * Line 2742. The tail is
 *   "3942f108 ldrb" w8,[x8,#188] ; "2a2803e8 mvn" w8, w8 ;
 *   "13000100 sbfx" w0, w8, #0, #1
 * that is `return ltr2568_init_flag ? 0 : -1` on a bool.
 */
static int ltr2568_local_init(void)
{
	if (i2c_add_driver(&ltr2568_i2c_driver)) {
		/* "\x013[ALS/PS] %s %d : add driver error\n"@0xffffff80091bdfbf */
#line 2742
		APS_ERR("add driver error\n");
		return -1;	/* "12800000 mov" w0, #0xffffffff */
	}

	if (!ltr2568_init_flag)
		return -1;

	return 0;
}

/*
 * 0xffffff8009373bd8, the __init section, 32 bytes: a single call to
 * alsps_driver_add(&ltr2568_init_info) and `return 0`.
 */
static int __init ltr2568_init(void)
{
	alsps_driver_add(&ltr2568_init_info);
	return 0;
}

/*
 * ltr2568_exit() was reconstructed from the factory kernel disassembly (0xffffff80093aa4bc, 4 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_alsps_ltr2568_ltr2568.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void __exit ltr2568_exit(void)
{
}

module_init(ltr2568_init);
module_exit(ltr2568_exit);

MODULE_AUTHOR("Liteon");
MODULE_DESCRIPTION("LTR-2568 ALS/PS Driver");
MODULE_LICENSE("GPL");
