// SPDX-License-Identifier: GPL-2.0
/*
 * FocalTech FT8719 touch panel, Doogee S88 Pro (MT6771) -- the core
 * translation unit, and that alone. Declared partial batch.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read. In particular the ALPS tree this file builds against does contain
 * a public FocalTech driver,
 * drivers/input/touchscreen/mediatek/focaltech_fhd_touch/focaltech_core.c: it
 * was NOT opened. Should a future batch open it, that has to be declared and
 * every line re-checked against the binary.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <uapi/linux/sched/types.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/string.h>
#include "tpd.h"

/*
 * FTS_DEBUG() was reconstructed from the factory kernel disassembly (0xffffff800924f634).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define FTS_DEBUG(fmt, args...)		printk("[FTS]" fmt "\n", ##args)
#define FTS_INFO(fmt, args...)		printk(KERN_INFO "[FTS][Info]" fmt "\n", ##args)
#define FTS_ERROR(fmt, args...)		printk(KERN_ERR "[FTS][Error]" fmt "\n", ##args)
#define FTS_FUNC_ENTER()		printk("[FTS]%s: Enter\n", __func__)
#define FTS_FUNC_EXIT()			printk("[FTS]%s: Exit(%d)\n", __func__, __LINE__)

/* "Focaltech V2.2 20180321"@0xffffff800924e1c9, second argument of the
 * FTS_INFO in tpd_driver_init: "91072421 add"@0xffffff8009383498 puts
 * 0x924e000 + 0x1c9 into x1. */
#define FTS_DRIVER_VERSION		"Focaltech V2.2 20180321"

/*
 * Constants read from the immediates, not from the names.
 * "52801461 mov"@0xffffff8008a7eb04   -> w1 = 0xa3, chip id register
 * "528013e1 mov"@0xffffff8008a7f554   -> w1 = 0x9f, second byte of the chip id
 * "52801501 mov"@0xffffff8008a7ee14   -> w1 = 0xa8, vendor id
 * "528014a1 mov"@0xffffff8008a7f0b8   -> w1 = 0xa5, power mode
 * "528014c1 mov"@0xffffff8008a7fa10   -> w1 = 0xa6, fw version
 * "52800c80 mov"@0xffffff8008a7eb28   -> 100 ms between two reads
 * "710fa2df cmp"@0xffffff8008a7eb34   -> 1000 ms maximum wait
 */
#define FTS_REG_CHIP_ID			0xa3
#define FTS_REG_CHIP_ID2		0x9f
#define FTS_REG_MODULE_ID		0xa8
#define FTS_REG_POWER_MODE		0xa5
#define FTS_REG_FW_VER			0xa6
#define INTERVAL_READ_REG		100
#define TIMEOUT_READ_REG		1000

/*
 * "d10943ff sub"@0xffffff8008a7f320 -- the frame of tpd_probe is 0x250 bytes
 * beyond the 96 of the saved registers; the sprintf buffer is at sp+0x40.
 * See DIVERGENCE 1 at the end.
 */
#define FTS_TP_INFO_LEN			512
/*
 * "9128e000 add"@0xffffff8008a801e8 + the table stride of 40:
 * "52800509 mov"@0xffffff8008a7f8cc -> w9 = 40, and
 * "9b292261 smaddl"@0xffffff8008a7f8d0 -> x1 = table + id*40.
 */
#define FTS_MODULE_NAME_LEN		40
#define FTS_MODULE_NAME_NUM		9

/*
 * kfree() on its own already handles NULL; the binary puts the check in ALL
 * THE SAME:
 *   "b4000060 cbz"@0xffffff8008a7f6e0   x0 == 0 -> skip the kfree
 *   "f900aa9f str"@0xffffff8008a7f6e8   and then clears the field
 * so in the source there is a macro doing both.
 */
#define kfree_safe(pbuf) do { \
	if (pbuf) { \
		kfree(pbuf); \
		pbuf = NULL; \
	} \
} while (0)

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7f354).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * The 16 bytes at 0xffffff800924fee8, copied wholesale into the context:
 *   "a9402508 ldp"@0xffffff8008a7f728   x8,x9 <- 0x924fee8
 *   "a901a688 stp"@0xffffff8008a7f72c   -> ts_data + 24
 * Contents read from the image (objdump -s):
 *   ffffff800924fee8  0d000000 00000000 87198719 87a987b9
 * Bytes 8 and 9 are the two the binary then compares and prints:
 *   "71021f9f cmp"@0xffffff8008a7f588  -> 0x87
 *   "7100677f cmp"@0xffffff8008a7f590  -> 0x19
 */
struct fts_ic_ids {
	u64 c0;			/* = 0x0d */
	u8 c8;			/* = 0x87; read by "39408281 ldrb"@0xffffff8008a7f738 */
	u8 c9;			/* = 0x19; read by "39408682 ldrb"@0xffffff8008a7f73c */
	u8 c10;			/* = 0x87 */
	u8 c11;			/* = 0x19 */
	u8 c12;			/* = 0x87 */
	u8 c13;			/* = 0xa9; cfr "7102a6df cmp"@0xffffff8008a7f688 */
	u8 c14;			/* = 0x87 */
	u8 c15;			/* = 0xb9; cfr "7102e6df cmp"@0xffffff8008a7f680 */
};

struct fts_event {
	int c0;			/* x;  "b94002e3 ldr"@0xffffff8008a7ffcc -> ABS_MT_POSITION_X */
	int c4;			/* y;  "b94006e3 ldr"@0xffffff8008a7ffe4 -> ABS_MT_POSITION_Y */
	int c8;			/* "b9400ac3 ldr"@0xffffff8008a7ff74 -> ABS_MT_PRESSURE */
	int c12;		/* "b9400ea8 ldr"@0xffffff8008a7fef0, bit 1 = giu'/su' */
	int c16;		/* "b94012a3 ldr"@0xffffff8008a7ff28, slot index */
	int c20;		/* "b94016c3 ldr"@0xffffff8008a7ff9c -> ABS_MT_TOUCH_MAJOR */
};				/* 24 byte: "8b130668 add"+"d37df100 lsl"@0xffffff8008a7f484/84 */

struct fts_ts_data {
	struct i2c_client *c0;	/* "f9000015 str"@0xffffff8008a7f374 = the probe's client */
	struct input_dev *c8;	/* "f9000408 str"@0xffffff8008a7f38c = tpd->dev */
	u8 c16;			/* "79002298 strh"@0xffffff8008a7f514 writes c16=1 e c17=0 */
	u8 c17;			/* read by "39404688 ldrb"@0xffffff8008a7f5e0 */
	u8 c18[6];		/* never touched; padding up to the alignment of c24 */
	struct fts_ic_ids c24;	/* "a901a688 stp"@0xffffff8008a7f72c */
	struct workqueue_struct *c40;	/* "f9001680 str"@0xffffff8008a7f3f0 */
	u8 c48[224];		/* never touched by this unit */
	struct task_struct *c272;	/* "f9008a97 str"@0xffffff8008a7f7d4 */
	spinlock_t c280;	/* "b9011a9f str"@0xffffff8008a7f418, then
				 * "91046100 add"@0xffffff8008a7ed18 -> +0x118 */
	struct mutex report_mutex;	/* +288; NAMED by the binary, see above */
	int c320;		/* irq: "b9014280 str"@0xffffff8008a7f840 */
	u8 c324;		/* "39051288 strb"@0xffffff8008a7f0f0 = 1 in tpd_suspend */
	u8 c325;		/* "39451688 ldrb"@0xffffff8008a7f034 */
	u8 c326;		/* "39451909 ldrb"@0xffffff8008a7ed28 under the spinlock */
	u8 c327;		/* never touched; padding */
	struct fts_event *c328;	/* "f900a680 str"@0xffffff8008a7f490 */
	u8 *c336;		/* "f900aa80 str"@0xffffff8008a7f474 */
	int c344;		/* "b9015a88 str"@0xffffff8008a7f46c, the byte count */
	int c348;		/* "b9415f29 ldr"@0xffffff8008a7fffc, bit mask */
	u8 c352;		/* "39058328 strb"@0xffffff8008a7ff04 */
	u8 c353[3];		/* padding */
	int c356;		/* "b901672c str"@0xffffff8008a7fdfc, counter of points */
	int c360;		/* "b9016b21 str"@0xffffff8008a7fd30 */
	u8 c364[20];		/* up to 384 = the measured size */
};

/* The constant the 16 bytes of c24 come from. */
static const struct fts_ic_ids fts_ctype = {
	0x0dULL, 0x87, 0x19, 0x87, 0x19, 0x87, 0xa9, 0x87, 0xb9,
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800924f718).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct fts_ts_data *fts_data;

/*
 * HEADER DELTA (not done here: this is a SHARED file). `tpd_type_cap` is
 * defined in drivers/input/touchscreen/mediatek/tpd_setting.c and declared
 * in tpd_debug.h, which tpd.h includes ONLY under TPD_DEBUG_CODE. Here it is
 * declared locally, as ilitek_bus.c already does for the group E functions;
 * the right home would be tpd.h.
 */
extern int tpd_type_cap;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a100ae0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct i2c_client *fts_g_a100ae0;

/*
 * 0xffffff800a100ad0, in .bss (so zero), passed as the FIRST argument to
 * tpd_gpio_output ("97ff4902 bl"@0xffffff8008a7eca4) by fts_reset_proc,
 * tpd_probe and tpd_resume. NO instruction anywhere in the image writes it.
 * It is not static: a `static int` never assigned would be 0 and clang would
 * have folded it to `mov w0, wzr`; here there is a full-width load instead,
 * "b94ad280 ldr"@0xffffff8008a7ec9c. This is defect class A1 used the other
 * way round.
 */
int fts_g_a100ad0;

/*
 * 0xffffff800998c910, in .data (value 1), passed to tpd_gpio_as_int
 * ("97ff460a bl"@0xffffff8008a7f818). Nobody writes it; same reasoning as
 * above: "b9491100 ldr"@0xffffff8008a7f814 is a full-width load, so it is
 * not static.
 */
int fts_g_998c910 = 1;

/*
 * 0xffffff800a100aec: set to 1 by tpd_eint_interrupt_handler
 * ("392bb109 strb"@0xffffff8008a801f8), cleared and waited on by
 * touch_event_handler ("396bb388 ldrb"+"370002c8 tbnz"@0xffffff8008a7fc68/68).
 * The accesses are NARROWED (ldrb/strb + tbnz #0), that is, consistent with a
 * `static int` that clang narrowed to two values.
 */
static int fts_g_a100aec;

/*
 * 0xffffff800998ca38: offsets +0x08 and +0x10 point at +0x08, that is an
 * empty list, that is STATIC initialisation (no __init_waitqueue_head in
 * tpd_probe). __wake_up from "97da7461 bl"@0xffffff8008a801fc.
 */
static DECLARE_WAIT_QUEUE_HEAD(fts_g_998ca38);

/*
 * 0xffffff800998ca50, 360 bytes in .data, indexed with multiplier 40
 * by the value get_module_id returns ("9b292261 smaddl"@0xffffff8008a7f8d0).
 * Contents read from the image, not assumed (objdump -s):
 *   ffffff800998ca50  6e6f6e65 5f300000 ...  "none_0"
 *   ffffff800998ca78  6e6f6e65 5f310000 ...  "none_1"   (stride 40)
 *   ... up to "none_8" at 0xffffff800998cb90.
 * It is not const: it lives in .data and not in .rodata.
 */
static char fts_g_998ca50[FTS_MODULE_NAME_NUM][FTS_MODULE_NAME_LEN] = {
	"none_0", "none_1", "none_2", "none_3", "none_4",
	"none_5", "none_6", "none_7", "none_8",
};

/*
 * ==========================================================================
 * THE BOUNDARY WITH THE OTHER FIVE TRANSLATION UNITS
 * ==========================================================================
 * These functions are NOT written. They are declared and left undefined:
 * a link failure is the honest outcome, a stub would make them look
 * written (rule 6, class B4). The factory address of each is alongside, so
 * the next batch knows where to look.
 */
int fts_i2c_read(struct i2c_client *client, char *writebuf, int writelen,
		 char *readbuf, int readlen);		/* 0xffffff8008a802ac */
int fts_i2c_write(struct i2c_client *client, char *writebuf, int writelen);
							/* 0xffffff8008a80494 */
int fts_i2c_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue);
							/* 0xffffff8008a805c4 */
int fts_i2c_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue);
							/* 0xffffff8008a80614 */
int fts_i2c_hid2std(struct i2c_client *client);		/* 0xffffff8008a80668 */
int fts_i2c_init(void);					/* 0xffffff8008a80750 */
int fts_i2c_exit(void);					/* 0xffffff8008a80798 */
int fts_ex_mode_init(struct i2c_client *client);	/* 0xffffff8008a8020c */
int fts_ex_mode_exit(struct i2c_client *client);	/* 0xffffff8008a80280 */
int fts_ex_mode_recovery(struct i2c_client *client);	/* 0xffffff8008a802a4 */
int fts_create_apk_debug_channel(struct fts_ts_data *ts_data);
							/* 0xffffff8008a7b600 */
void fts_release_apk_debug_channel(struct fts_ts_data *ts_data);
							/* 0xffffff8008a7b664 */
int fts_create_sysfs(struct i2c_client *client);	/* 0xffffff8008a7b680 */
int fts_remove_sysfs(struct i2c_client *client);	/* 0xffffff8008a7b6e4 */

/*
 * It does not belong to the block: it is one of the FOUR Wingtech functions
 * the factory patch adds to mtk_tpd.c (0xffffff8008a5151c..0xffffff8008a515e8)
 * and that our ALPS tree does not have. Called by tpd_probe:
 * "97ff46ac bl"@0xffffff8008a7fa6c.
 */
int fix_tp_proc_info(char *buf, int len);		/* 0xffffff8008a5151c */

/*
 * Forward declarations for the unit: the order of the factory text is not
 * the order of the source (the FTS_FUNC_EXIT line numbers prove it), and the
 * data structures come first.
 */
static int tpd_local_init(void);
static void tpd_suspend(struct device *h);
static void tpd_resume(struct device *h);
static int tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_remove(struct i2c_client *client);
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
static int touch_event_handler(void *unused);

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998c918, 56 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const struct of_device_id fts_dt_match[] = {
	/* "mediatek,cap_touch" is the `compatible` a 0xffffff8008f7f460 + 0x40. */
	{ .compatible = "mediatek,cap_touch" },
	{},
};
MODULE_DEVICE_TABLE(of, fts_dt_match);

static const struct i2c_device_id fts_tpd_id[] = {
	/* 0xffffff8008f7f5f0: name = "fts_ts", driver_data = 0, then the
	 * zero terminator. */
	{ "fts_ts", 0 },
	{},
};

static struct i2c_driver tpd_i2c_driver = {
	.driver = {
		.name = "fts_ts",
		.of_match_table = of_match_ptr(fts_dt_match),
	},
	.probe = tpd_probe,
	.remove = tpd_remove,
	.id_table = fts_tpd_id,
	.detect = tpd_i2c_detect,
};

static struct tpd_driver_t tpd_device_driver = {
	.tpd_device_name = "fts_ts",
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
};

/*
 * ==========================================================================
 * THE FUNCTIONS
 * ==========================================================================
 * The order is that of the factory SOURCE, reconstructed from the line numbers
 * FTS_FUNC_EXIT leaves in the binary, NOT that of the text: in the text
 * tpd_probe (line 1089) comes after tpd_resume (line 1341).
 */

/*
 * fts_wait_tp_to_valid @0xffffff8008a7eac0, 196 byte, visibilita' T.
 */
int fts_wait_tp_to_valid(struct i2c_client *client)
{
	int ret = 0;
	int cnt = 0;
	u8 reg_value = 0;
	/*
	 * The chip id is read ONCE, BEFORE the loop:
	 * "39408114 ldrb"@0xffffff8008a7eafc sits at 0xffffff8008a7eafc and the loop
	 * begins at 0xffffff8008a7eb00 ("910013e2 add"). If the source re-read it
	 * inside the comparison, fts_i2c_read_reg would invalidate it and the
	 * ldrb would sit inside the loop -- that is class A2 the other way round.
	 */
	u8 chip_id = fts_data->c24.c8;

	do {
		ret = fts_i2c_read_reg(client, FTS_REG_CHIP_ID, &reg_value);
		if ((ret < 0) || (reg_value != chip_id)) {
			/*
			 * "37f80060 tbnz"@0xffffff8008a7eb14 is the ret<0 test;
			 * "6b14003f cmp"@0xffffff8008a7eb18 the reg==chip one.
			 */
			FTS_DEBUG("TP Not Ready, ReadData = 0x%x", reg_value);
		} else {
			FTS_INFO("TP Ready, Device ID = 0x%x", reg_value);
			return 0;
		}
		cnt++;
		msleep(INTERVAL_READ_REG);
		/*
		 * clang reduces the counter to a stride of 100:
		 * "110192d6 add"@0xffffff8008a7eb30 and
		 * "710fa2df cmp"@0xffffff8008a7eb34 (0x3e8 = 1000).
		 */
	} while ((cnt * INTERVAL_READ_REG) < TIMEOUT_READ_REG);

	/* "321d7be0 orr"@0xffffff8008a7eb3c -> w0 = 0xfffffffb = -5 = -EIO */
	return -EIO;
}

/*
 * fts_tp_state_recovery @0xffffff8008a7eb84, 240 bytes, visibility T.
 * The body of fts_wait_tp_to_valid is INLINED into it (same instructions,
 * "9400068c bl"@0xffffff8008a7ebe4 calls fts_i2c_read_reg directly) and the
 * return value is thrown away: after the "TP Ready" branch execution carries
 * on at 0xffffff8008a7ec28 just as after the timeout ("14000005 b"@0xffffff8008a7ec14).
 */
void fts_tp_state_recovery(struct i2c_client *client)
{
	FTS_FUNC_ENTER();
	fts_wait_tp_to_valid(client);
	/* "9400059e bl"@0xffffff8008a7ec2c -> fts_ex_mode_recovery (unita' D) */
	fts_ex_mode_recovery(client);
	FTS_FUNC_EXIT();
}

/*
 * fts_reset_proc @0xffffff8008a7ec74, 124 bytes, visibility T.
 * This one is inlined into tpd_probe and tpd_resume too, but it has a symbol
 * of its own: clang emitted both the out-of-line copy and the inlined one.
 */
int fts_reset_proc(int hdelayms)
{
	FTS_FUNC_ENTER();
	/* "b94ad280 ldr"@0xffffff8008a7ec9c loads the pin at full width,
	 * "2a1f03e1 mov"@0xffffff8008a7eca0 puts the level a 0. */
	tpd_gpio_output(fts_g_a100ad0, 0);
	/* "52800280 mov"@0xffffff8008a7eca8 -> w0 = 20 */
	msleep(20);
	/* "320003e1 orr"@0xffffff8008a7ecb4 -> level 1 */
	tpd_gpio_output(fts_g_a100ad0, 1);
	/* "34000073 cbz"@0xffffff8008a7ecbc: the msleep is conditional */
	if (hdelayms)
		msleep(hdelayms);
	FTS_FUNC_EXIT();
	return 0;
}

/*
 * fts_irq_disable @0xffffff8008a7ecf0, 132 bytes, visibility T.
 * The c326 field is read and written UNDER the c280 spinlock, and fts_data is
 * re-read from the global after the _raw_spin_lock_irqsave
 * ("f9456e88 ldr"@0xffffff8008a7ed20): the source uses `fts_data->...` and not
 * a local, otherwise the pointer would sit in a callee-saved register.
 */
void fts_irq_disable(void)
{
	unsigned long irqflags;

	FTS_FUNC_ENTER();
	spin_lock_irqsave(&fts_data->c280, irqflags);
	/* "350000c9 cbnz"@0xffffff8008a7ed2c: it disables only if not already disabled */
	if (!fts_data->c326) {
		/* "b9414100 ldr"@0xffffff8008a7ed30 -> irq at +320 */
		disable_irq_nosync(fts_data->c320);
		fts_data->c326 = 1;
	}
	spin_unlock_irqrestore(&fts_data->c280, irqflags);
	FTS_FUNC_EXIT();
}

/*
 * fts_irq_enable @0xffffff8008a7ed74, 128 bytes, visibility T.
 * Four bytes fewer than fts_irq_disable: the flag write is `strb wzr`
 * ("3905191f strb"@0xffffff8008a7edc0) instead of `orr w9,wzr,#1` + `strb`.
 */
void fts_irq_enable(void)
{
	unsigned long irqflags;

	FTS_FUNC_ENTER();
	spin_lock_irqsave(&fts_data->c280, irqflags);
	/* "340000a9 cbz"@0xffffff8008a7edb0 -- the test is inverted with respect to
	 * fts_irq_disable */
	if (fts_data->c326) {
		/* "97daefff bl"@0xffffff8008a7edb8 -> enable_irq */
		enable_irq(fts_data->c320);
		fts_data->c326 = 0;
	}
	spin_unlock_irqrestore(&fts_data->c280, irqflags);
	FTS_FUNC_EXIT();
}

/*
 * fts_release_all_finger -- INLINED into tpd_resume, with no symbol of its own.
 * The "__func__" it prints gives it away (class B5):
 *   "fts_release_all_finger"@0xffffff800924fe55 is the x1 argument of the
 *   FTS_FUNC_ENTER at "97dad0df bl"@0xffffff8008a7f158, inside tpd_resume.
 * The FTS_FUNC_EXIT carries line 1c3 = 451 ("52803862 mov"@0xffffff8008a7f1f0).
 */
static void fts_release_all_finger(void)
{
	struct input_dev *input_dev = fts_data->c8;
	u32 finger_count = 0;

	FTS_FUNC_ENTER();
	mutex_lock(&fts_data->report_mutex);
	/*
	 * The limit is RE-READ on every iteration: "b94f92e8 ldr"@0xffffff8008a7f19c
	 * sits inside the loop, not before it. The comparison is unsigned
	 * ("6b08029f cmp"+"54fffe83 b.cc"@0xffffff8008a7f1a8/a8), so the counter is
	 * a u32.
	 */
	for (finger_count = 0; finger_count < tpd_dts_data.touch_max_num; finger_count++) {
		/* "528005e2 mov"@0xffffff8008a7f17c -> ABS_MT_SLOT = 0x2f */
		input_mt_slot(input_dev, finger_count);
		input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, false);
	}
	/* "52802942 mov"@0xffffff8008a7f1b0 -> BTN_TOUCH = 0x14a */
	input_report_key(input_dev, BTN_TOUCH, 0);
	input_sync(input_dev);
	mutex_unlock(&fts_data->report_mutex);
	FTS_FUNC_EXIT();
}

/*
 * fts_input_init -- INLINED into tpd_probe, with no symbol of its own.
 *   "fts_input_init"@0xffffff800924f9d4 is the x1 of the FTS_FUNC_ENTER at
 *   "97dad028 bl"@0xffffff8008a7f434, inside tpd_probe.
 * It has TWO exits: line 845 on success ("528069a2 mov"@0xffffff8008a7f4a8)
 * and line 852 on error ("52806a82 mov"@0xffffff8008a7f6fc).
 */
static int fts_input_init(struct i2c_client *client)
{
	int point_num = 0;

	FTS_FUNC_ENTER();
	/* "321f03e2 orr"@0xffffff8008a7f444 -> w2 = 2 = INPUT_MT_DIRECT */
	input_mt_init_slots(tpd->dev, tpd_dts_data.touch_max_num, INPUT_MT_DIRECT);

	/*
	 * The second load is a ldrsw ("b98f9273 ldrsw"@0xffffff8008a7f450) and
	 * its value survives the __kmalloc: if the source re-read
	 * tpd_dts_data.touch_max_num for the second kzalloc, there would be a third
	 * load after the call. So there is a local, read once.
	 */
	point_num = tpd_dts_data.touch_max_num;
	/* "1b082668 madd"@0xffffff8008a7f460 -> w8 = point_num*6 + 3 */
	fts_data->c344 = point_num * 6 + 3;
	fts_data->c336 = kzalloc(fts_data->c344, GFP_KERNEL);
	if (!fts_data->c336) {
		FTS_ERROR("failed to alloc memory for point buf!");
		goto err_point_buf;
	}

	/* "8b130668 add"+"d37df100 lsl"@0xffffff8008a7f484/84 -> point_num*24 */
	fts_data->c328 = kzalloc(point_num * sizeof(struct fts_event), GFP_KERNEL);
	if (!fts_data->c328) {
		FTS_ERROR("failed to alloc memory for point events!");
		goto err_event_buf;
	}

	FTS_FUNC_EXIT();
	return 0;

err_event_buf:
	kfree_safe(fts_data->c336);
err_point_buf:
	FTS_FUNC_EXIT();
	/*
	 * "12800177 mov"@0xffffff8008a7f710 -> w23 = 0xfffffff4 = -12 = -ENOMEM,
	 * set by the CALLER after the FTS_ERROR of "fts input initialize fail".
	 */
	return -ENOMEM;
}

/*
 * get_module_id @0xffffff8008a7edf4, 332 bytes, visibility T.
 * The two prints are TPD_DMESG from tpd.h, not FTS_*: the factory format is
 *   "\x016mtk-tpd:[%s:%d] fts ret = %d ,verdor_id = 0x%x.\n"@0xffffff800924f69b
 * which is exactly `pr_info(TPD_DEVICE ":[%s:%d] " a, __func__, __LINE__, ...)`
 * with TPD_DEVICE = "mtk-tpd". The lines are 892 ("52806f82 mov"@0xffffff8008a7ee40)
 * and 898 ("52807042 mov"@0xffffff8008a7eea0). "verdor_id" is the factory
 * spelling and is reproduced (rule 7).
 */
int get_module_id(void)
{
	int ret = 0;
	u8 vendor_id = 0;
	/*
	 * "12001e68 and"@0xffffff8008a7ee74 truncates the counter to 8 bits before
	 * the comparison: it is a u8, not an int (class A4).
	 */
	u8 i = 0;

	ret = fts_i2c_read_reg(fts_g_a100ae0, FTS_REG_MODULE_ID, &vendor_id);
	TPD_DMESG("fts ret = %d ,verdor_id = 0x%x.\n", ret, vendor_id);
	/* "7100067f cmp"@0xffffff8008a7ee4c -> ret == 1 */
	if (ret != 1) {
		/*
		 * "7100111f cmp"@0xffffff8008a7ee78 + "54000068 b.hi" -> (u8)i > 4
		 *
		 * THE UNROLLING WAS THE ONLY THING SEPARATING THIS FUNCTION
		 * FROM THE FACTORY, and the pragma is declared scaffolding --
		 * it emits no code. The binary has a REAL LOOP with a counter:
		 * "11000673 add"@0xffffff8008a7ee70 increments w19 and
		 * "12001e68 and"@0xffffff8008a7ee74 truncates it to eight bits before
		 * the comparison. On our side clang unrolled the five rounds and the
		 * function measured 400 bytes against 332. As with mir3da's
		 * `squareRoot`, WHY THE FACTORY DOES NOT UNROLL IS NOT EXPLAINED:
		 * it is the same clang version and the same options.
		 */
#pragma clang loop unroll(disable)
		for (i = 0; i < 5; i++) {
			ret = fts_i2c_read_reg(fts_g_a100ae0, FTS_REG_MODULE_ID,
					       &vendor_id);
			if (ret == 1)
				break;
		}
	}
	TPD_DMESG("fts ret = %d ,verdor_id = 0x%x.\n", ret, vendor_id);

	switch (vendor_id) {
	case 0x25:		/* "7100951f cmp"@0xffffff8008a7eec4 */
		return 0;	/* "2a1f03e0 mov"@0xffffff8008a7eecc */
	case 0x82:		/* "7102091f cmp"@0xffffff8008a7eee4 */
		return 1;	/* "320003e0 orr"@0xffffff8008a7eeec */
	case 0x67:		/* "71019d1f cmp"@0xffffff8008a7eed4 */
		return 2;	/* "321f03e0 orr"@0xffffff8008a7ef04 */
	case 0x11:		/* "7100451f cmp"@0xffffff8008a7eeb4 */
		return 3;	/* "320007e0 orr"@0xffffff8008a7eef4 */
	case 0xd2:		/* "7103491f cmp"@0xffffff8008a7eedc */
		return 4;	/* "321e03e0 orr"@0xffffff8008a7ef0c */
	case 0x12:		/* "7100491f cmp"@0xffffff8008a7eebc */
		return 5;	/* "528000a0 mov"@0xffffff8008a7eefc */
	default:
		return 8;	/* "321d03e0 orr"@0xffffff8008a7ef14 */
	}
}

/*
 * fts_read_parse_touchdata -- INLINED into touch_event_handler.
 * It has neither a symbol nor an FTS_FUNC_ENTER: what gives it away as a
 * separate function is the DOUBLE load of tpd_dts_data.touch_max_num.
 * "b98f9113 ldrsw"@0xffffff8008a7fce0 puts it in x19 and x19 survives the
 * call to fts_i2c_read; then, after mutex_lock,
 * "b94f9138 ldr"@0xffffff8008a7fe9c RELOADS it into w24.  Two distinct locals
 * in two distinct bodies; a single body would have a single local.
 */
static int fts_read_parse_touchdata(struct fts_ts_data *data)
{
	int i = 0;
	int ret = 0;
	int base = 0;
	int pointid = 0;
	int max_touch_num = tpd_dts_data.touch_max_num;
	u8 *buf = data->c336;
	struct fts_event *events = data->c328;

	/*
	 * "f900011f str"@0xffffff8008a7fcf0 zeroes EIGHT bytes at +356: clang has
	 * merged the two clears of c356 and c360, which are adjacent.
	 */
	data->c356 = 0;
	data->c360 = 0;

	/*
	 * "32001fe1 orr"@0xffffff8008a7fcf8 -> w1 = 0xff;
	 * "b9815b22 ldrsw"@0xffffff8008a7fcf4 -> the length is c344, signed
	 */
	memset(buf, 0xff, data->c344);
	buf[0] = 0x00;

	/*
	 * "320003e2 orr"@0xffffff8008a7fd10 -> writelen = 1;
	 * the same buf is both source and destination ("aa1a03e1 mov"/"aa1a03e3 mov"
	 * @0xffffff8008a7fd14/18).
	 */
	ret = fts_i2c_read(data->c0, buf, 1, buf, data->c344);
	if (ret < 0) {
		FTS_ERROR("read touchdata failed, ret:%d", ret);
		return ret;
	}

	/* "12000d01 and"@0xffffff8008a7fd28 -> point_num = buf[2] & 0x0f */
	data->c360 = buf[2] & 0x0f;

	/*
	 * "71003c3f cmp"@0xffffff8008a7fd2c -> 0x0f, and then SIX comparisons with 0xff
	 * on buf[1]..buf[6] ("7103fd1f cmp"@0xffffff8008a7fd44 and the five
	 * that follow).
	 */
	if ((data->c360 == 0x0f) && data->c16 &&
	    (buf[1] == 0xff) && (buf[2] == 0xff) && (buf[3] == 0xff) &&
	    (buf[4] == 0xff) && (buf[5] == 0xff) && (buf[6] == 0xff)) {
		FTS_INFO("touch buff is 0xff, need recovery state");
		fts_tp_state_recovery(data->c0);
		return -EIO;
	}

	if (data->c360 > max_touch_num) {
		FTS_INFO("invalid point_num(%d)", data->c360);
		return -EIO;
	}

	for (i = 0; i < max_touch_num; i++) {
		base = 8 + 6 * i;
		/*
		 * "71027d9f cmp"@0xffffff8008a7fdd4 + "54000568 b.hi": clang has
		 * rewritten `(buf[base-3] >> 4) >= 10` as `buf[base-3] > 0x9f`.
		 */
		pointid = buf[base - 3] >> 4;
		if (pointid >= 10)
			break;
		else if (pointid >= max_touch_num) {
			FTS_ERROR("ID(%d) beyond max_touch_number", pointid);
			return -EINVAL;
		}

		data->c356++;
		/* "33180d8e bfi"@0xffffff8008a7fe0c: (buf[base-5]&0x0f)<<8 | buf[base-4] */
		events[i].c0 = ((buf[base - 5] & 0x0f) << 8) + buf[base - 4];
		events[i].c4 = ((buf[base - 3] & 0x0f) << 8) + buf[base - 2];
		/* "53067d8c lsr"@0xffffff8008a7fe30 -> >> 6 */
		events[i].c12 = buf[base - 5] >> 6;
		events[i].c16 = buf[base - 3] >> 4;
		events[i].c20 = buf[base] >> 4;
		events[i].c8 = buf[base - 1];

		/*
		 * "321f018c orr"+"7100099f cmp"@0xffffff8008a7fe40/40: (flag|2)==2,
		 * that is, flag is 0 or 2.
		 */
		if (((events[i].c12 == 0) || (events[i].c12 == 2)) && (data->c360 == 0)) {
			FTS_INFO("abnormal touch data from fw");
			return -EIO;
		}
	}

	if (data->c356 == 0) {
		FTS_INFO("no touch point information");
		return -EIO;
	}

	return 0;
}

/*
 * fts_input_report_b -- INLINED into touch_event_handler, as above.
 * TPD_RES_Y is NAMED by the binary: it is the global at 0xffffff800996b2b8
 * that tpd_probe prints with "[FTS]TPD_RES_Y:%d\n"@0xffffff800924f949.
 * The comparison is 32-bit UNSIGNED ("6b09011f cmp"+"54000209 b.ls"
 * @0xffffff8008a7fee0/e4): in this unit TPD_RES_Y is 32-bit unsigned,
 * while tpd.h declares it `unsigned long`. See DIVERGENCE 2.
 */
static int fts_input_report_b(struct fts_ts_data *data)
{
	int i = 0;
	int touchs = 0;
	bool va_reported = false;
	u32 max_touch_num = tpd_dts_data.touch_max_num;
	u32 res_y = (u32)TPD_RES_Y;
	struct fts_event *events = data->c328;

	/*
	 * The loop limit is RE-READ on every round:
	 * "b9816728 ldrsw"@0xffffff8008a80094 sits at the end of the body.
	 */
	for (i = 0; i < data->c356; i++) {
		if (tpd_dts_data.use_tpd_button && (events[i].c4 > res_y)) {
			if ((events[i].c12 == 0) || (events[i].c12 == 2)) {
				data->c352 = 1;
				/* "97ff48db bl"@0xffffff8008a7ff10 -> tpd_button */
				tpd_button(events[i].c0, events[i].c4, 1);
				FTS_DEBUG("Key(%d, %d) DOWN", events[i].c0, events[i].c4);
			} else {
				data->c352 = 0;
				tpd_button(events[i].c0, events[i].c4, 0);
				FTS_DEBUG("Key(%d, %d) UP", events[i].c0, events[i].c4);
			}
			continue;
		}

		/*
		 * "6b18007f cmp"+"54000bc2 b.cs"@0xffffff8008a7ff30/30: the comparison
		 * with the maximum is unsigned, so max_touch_num is a u32.
		 */
		if (events[i].c16 >= max_touch_num)
			break;

		input_mt_slot(tpd->dev, events[i].c16);

		if ((events[i].c12 == 0) || (events[i].c12 == 2)) {
			input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, true);
			/* "320017e3 orr"@0xffffff8008a7ff80 -> 0x3f */
			if (events[i].c8 <= 0)
				events[i].c8 = 0x3f;
			/* "52800742 mov"@0xffffff8008a7ff90 -> ABS_MT_PRESSURE = 0x3a */
			input_report_abs(tpd->dev, ABS_MT_PRESSURE, events[i].c8);
			/* "52800123 mov"@0xffffff8008a7ffa8 -> 9 */
			if (events[i].c20 <= 0)
				events[i].c20 = 9;
			/* "321c07e2 orr"@0xffffff8008a7ffb8 -> ABS_MT_TOUCH_MAJOR = 0x30 */
			input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, events[i].c20);
			input_report_abs(tpd->dev, ABS_MT_POSITION_X, events[i].c0);
			input_report_abs(tpd->dev, ABS_MT_POSITION_Y, events[i].c4);

			/* "9ac82368 lsl"@0xffffff8008a80008 -> 1 << id, a 64 bit */
			data->c348 |= BIT(events[i].c16);
			touchs |= BIT(events[i].c16);
			va_reported = true;
			FTS_DEBUG("[B]P%d(%d, %d)[p:%d,tm:%d] DOWN!",
				  events[i].c16, events[i].c0, events[i].c4,
				  events[i].c8, events[i].c20);
		} else {
			va_reported = true;
			input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, false);
			/* "0a280128 bic"@0xffffff8008a8005c */
			data->c348 &= ~BIT(events[i].c16);
			FTS_DEBUG("[B]P%d UP!", events[i].c16);
		}
	}

	/*
	 * "6b1a011f cmp"+"54000260 b.eq"@0xffffff8008a800c0/c0 skips the whole
	 * loop when the two values match; the "cbz w24" at
	 * 0xffffff8008a800a8 is the loop guard, which clang hoisted above it.
	 */
	if (data->c348 ^ touchs) {
		for (i = 0; i < max_touch_num; i++) {
			/*
			 * "93407d08 sxtw"@0xffffff8008a7fbdc: the 32-bit xor is
			 * SIGN-extended to 64 before the tst, that is, BIT() is
			 * 64-bit and the other operand is an int.
			 */
			if (BIT(i) & (data->c348 ^ touchs)) {
				va_reported = true;
				FTS_DEBUG("[B]P%d UP!", i);
				input_mt_slot(tpd->dev, i);
				input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, false);
			}
		}
	}
	data->c348 = touchs;

	if (va_reported) {
		if (!touchs || !data->c360) {
			FTS_DEBUG("[B]Points All Up!");
			input_report_key(tpd->dev, BTN_TOUCH, 0);
		} else {
			input_report_key(tpd->dev, BTN_TOUCH, 1);
		}
		input_sync(tpd->dev);
	}

	return 0;
}

/*
 * touch_event_handler @0xffffff8008a7fb48, 1676 byte, visibilita' t.
 * The thread created by tpd_probe ("912d2000 add"@0xffffff8008a7f7a8 puts into x0
 * exactly 0xffffff8008a7fb48).
 */
static int touch_event_handler(void *unused)
{
	/* "321e03e8 orr"@0xffffff8008a7fb88 -> w8 = 4, written to sp+0x40;
	 * "321f03e1 orr"@0xffffff8008a7fb78 -> w1 = 2 = SCHED_RR. */
	struct sched_param param = { .sched_priority = 4 };
	struct fts_ts_data *ts_data = fts_data;
	int ret = 0;

	sched_setscheduler(current, SCHED_RR, &param);

	do {
		/*
		 * "d5033bbf dmb"@0xffffff8008a7fc60 -- set_current_state is an
		 * smp_store_mb, that is WRITE_ONCE plus a barrier.
		 */
		set_current_state(TASK_INTERRUPTIBLE);
		wait_event_interruptible(fts_g_998ca38, fts_g_a100aec != 0);
		fts_g_a100aec = 0;
		set_current_state(TASK_RUNNING);

		FTS_DEBUG("touch_event_handler start");

		ret = fts_read_parse_touchdata(ts_data);
		if (ret == 0) {
			mutex_lock(&ts_data->report_mutex);
			fts_input_report_b(ts_data);
			mutex_unlock(&ts_data->report_mutex);
		}
		/* "97d961bc bl"@0xffffff8008a80188 -> kthread_should_stop */
	} while (!kthread_should_stop());

	return 0;
}

/*
 * tpd_eint_interrupt_handler @0xffffff8008a801d4, 56 byte, visibilita' t.
 * Passed to request_threaded_irq by tpd_probe ("91075021 add"@0xffffff8008a7f86c).
 */
static irqreturn_t tpd_eint_interrupt_handler(int irq, void *dev_id)
{
	/* "392bb109 strb"@0xffffff8008a801f8 -- NARROWED write to one byte */
	fts_g_a100aec = 1;
	/* "320003e1 orr"+"320003e2 orr"+"aa1f03e3 mov"@0xffffff8008a801f4/f0/f4
	 * -> __wake_up(q, 1, 1, NULL) = wake_up_interruptible */
	wake_up_interruptible(&fts_g_998ca38);
	/* "320003e0 orr"@0xffffff8008a80200 -> IRQ_HANDLED */
	return IRQ_HANDLED;
}

/*
 * fts_get_ic_information -- INLINED into tpd_probe.
 * Like the two above it has no symbol, and like those it gives itself away
 * by the fact that ts_data->c0 is RELOADED into a callee-saved register
 * before the loop ("f9400296 ldr"@0xffffff8008a7f510) instead of reusing the
 * `client` parameter that tpd_probe already holds in x21: that is the local
 * of a separate body.
 */
static int fts_get_ic_information(struct fts_ts_data *ts_data)
{
	int ret = 0;
	int cnt = 0;
	u8 chip_id[2] = { 0 };
	u8 id_cmd[4] = { 0 };
	u32 id_cmd_len = 0;
	struct i2c_client *client = ts_data->c0;

	/*
	 * "79002298 strh"@0xffffff8008a7f514 writes TWO bytes with the value 1:
	 * c16 = 1 and c17 = 0, merged by clang into a single strh.
	 */
	ts_data->c16 = 1;
	ts_data->c17 = 0;

	for (cnt = 0; cnt < TIMEOUT_READ_REG; cnt += INTERVAL_READ_REG) {
		ret = fts_i2c_read_reg(client, FTS_REG_CHIP_ID, &chip_id[0]);
		ret = fts_i2c_read_reg(client, FTS_REG_CHIP_ID2, &chip_id[1]);
		if ((ret < 0) || (!chip_id[0]) || (!chip_id[1])) {
			FTS_DEBUG("i2c read invalid, read:0x%02x%02x",
				  chip_id[0], chip_id[1]);
		} else {
			FTS_DEBUG("verify id:0x%02x%02x", chip_id[0], chip_id[1]);
			if ((chip_id[0] == 0x87) && (chip_id[1] == 0x19)) {
				ts_data->c24 = fts_ctype;
				break;
			}
			FTS_DEBUG("TP not ready, read:0x%02x%02x",
				  chip_id[0], chip_id[1]);
		}
		msleep(INTERVAL_READ_REG);
	}

	/* "710f9e7f cmp"@0xffffff8008a7f5cc -> 0x3e7 = 999, that is `cnt >= 1000` */
	if (cnt >= TIMEOUT_READ_REG) {
		FTS_INFO("fw is invalid, need read boot id");
		if (ts_data->c17)
			fts_i2c_hid2std(client);

		/*
		 * "52954aa8 mov"@0xffffff8008a7f5f8 -> 0xaa55, written with a single
		 * strh: the two bytes are assigned one after the other.
		 */
		id_cmd[0] = 0x55;
		id_cmd[1] = 0xaa;
		ret = fts_i2c_write(client, id_cmd, 2);
		if (ret < 0) {
			FTS_ERROR("start cmd write fail");
			return ret;
		}
		msleep(10);

		/*
		 * "52801209 mov"+"b90043e9 str"@0xffffff8008a7f640/40: 0x90 and then
		 * three zeroes, written as one 4-byte `str w`.
		 */
		id_cmd[0] = 0x90;
		id_cmd[1] = 0x00;
		id_cmd[2] = 0x00;
		id_cmd[3] = 0x00;
		/*
		 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7f61c).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		if (!ts_data->c16)
			id_cmd_len = 4;
		else
			id_cmd_len = 1;
		ret = fts_i2c_read(client, id_cmd, id_cmd_len, chip_id, 2);
		if ((ret < 0) || (!chip_id[0]) || (!chip_id[1])) {
			FTS_ERROR("read boot id fail");
			return -EIO;
		}
		FTS_DEBUG("verify id:0x%02x%02x", chip_id[0], chip_id[1]);
		if ((chip_id[0] == 0x87) &&
		    ((chip_id[1] == 0x19) || (chip_id[1] == 0xb9) ||
		     (chip_id[1] == 0xa9))) {
			ts_data->c24 = fts_ctype;
		} else {
			FTS_ERROR("can't get ic informaton");
			/* "321a6ff7 orr"@0xffffff8008a7f950 -> 0xffffffc3 = -61 */
			return -ENODATA;
		}
	}

	FTS_INFO("get ic information, chip id = 0x%02x%02x",
		 ts_data->c24.c8, ts_data->c24.c9);
	return 0;
}

/*
 * tpd_probe() was reconstructed from the factory kernel disassembly (0xffffff8008a7f304, 1936 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int retval = 0;
	int ret = 0;
	int i = 0;
	int sensor_id = 0;
	u8 fwver = 0;
	char *name = NULL;
	char module_name[FTS_MODULE_NAME_LEN];
	char tp_info[FTS_TP_INFO_LEN];
	struct device_node *node = NULL;
	struct fts_ts_data *ts_data = NULL;

	FTS_FUNC_ENTER();

	/*
	 * "321907e1 orr"@0xffffff8008a7f354 -> 0x180 = 384 bytes;
	 * "52901802 mov"+"72a02802 movk"@0xffffff8008a7f358/58 -> 0x014080c0 =
	 * GFP_KERNEL | __GFP_ZERO, that is devm_kzalloc.
	 */
	ts_data = devm_kzalloc(&client->dev, sizeof(*ts_data), GFP_KERNEL);
	if (!ts_data) {
		FTS_ERROR("Failed to allocate memory for fts_data");
		/* "12800177 mov"@0xffffff8008a7f6b8 -> -12 = -ENOMEM */
		return -ENOMEM;
	}

	ts_data->c0 = client;
	fts_data = ts_data;
	/* "f9400d08 ldr"@0xffffff8008a7f37c -> tpd->dev, offset 24 of tpd_device */
	ts_data->c8 = tpd->dev;
	/* "790006b6 strh"@0xffffff8008a7f390 -> client->addr = 0x38 */
	client->addr = 0x38;
	fts_g_a100ae0 = client;
	/*
	 * The comparison is made AFTER the assignment, so it is never true; the
	 * load "794006a1 ldrh"@0xffffff8008a7f398 is there all the same because
	 * the kernel is built with -fno-strict-aliasing and the `str` of the
	 * global might, as far as the compiler knows, overlap it. It is a factory
	 * defect and is reproduced (rule 7).
	 */
	if (client->addr != 0x38) {
		FTS_INFO("[TPD]Change i2c addr 0x%02x to %x", client->addr, 0x38);
		client->addr = 0x38;
		FTS_INFO("[TPD]i2c addr=0x%x\n", client->addr);
	}

	/*
	 * create_singlethread_workqueue() was reconstructed from the factory kernel disassembly (0xffffff8008a7f3ec).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	ts_data->c40 = create_singlethread_workqueue("fts_wq");
	if (!ts_data->c40)
		FTS_ERROR("failed to create fts workqueue");

	spin_lock_init(&ts_data->c280);
	mutex_init(&ts_data->report_mutex);

	fts_i2c_init();

	ret = fts_input_init(client);
	if (ret) {
		FTS_ERROR("fts input initialize fail");
		retval = -ENOMEM;
		goto err_wq;
	}

	/* "52801900 mov"@0xffffff8008a7f4f8 -> 200 ms */
	fts_reset_proc(200);

	ret = fts_get_ic_information(ts_data);
	if (ret) {
		FTS_ERROR("not focal IC, unregister driver");
		retval = ret;
		goto err_wq;
	}

	ret = fts_create_apk_debug_channel(ts_data);
	if (ret)
		FTS_ERROR("create apk debug node fail");

	ret = fts_create_sysfs(client);
	if (ret)
		FTS_ERROR("create sysfs node fail");

	ret = fts_ex_mode_init(client);
	if (ret)
		FTS_ERROR("init glove/cover/charger fail");

	/*
	 * "97d964d3 bl"@0xffffff8008a7f7b8 kthread_create_on_node with node -1 and
	 * "97d9b159 bl"@0xffffff8008a7f7e4 wake_up_process only if it is not an
	 * ERR_PTR: that is the expansion of kthread_run.
	 */
	ts_data->c272 = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	if (IS_ERR(ts_data->c272)) {
		/*
		 * "2a1703e1 mov"@0xffffff8008a7f7d0 passes the LOW 32 bits of the
		 * error pointer: in the source there is an int, not a long.
		 */
		retval = PTR_ERR(ts_data->c272);
		FTS_ERROR("[TPD]Failed to create kernel thread_tpd,ret:%d", retval);
		ts_data->c272 = NULL;
		goto err_wq;
	}

	/*
	 * "9a88b121 csel"@0xffffff8008a7f800 chooses between "FAIL"@0xffffff8009249687
	 * and "PASS"@0xffffff800924966f with the condition `lt` on ret, which here is
	 * still fts_ex_mode_init's.
	 */
	FTS_DEBUG("[TPD]Touch Panel Device Probe %s!", (ret < 0) ? "FAIL" : "PASS");

	tpd_gpio_as_int(fts_g_998c910);

	node = of_find_matching_node_and_match(NULL, touch_of_match, NULL);
	if (node) {
		ts_data->c320 = irq_of_parse_and_map(node, 0);
		client->irq = ts_data->c320;
		FTS_INFO("IRQ request succussfully, irq:%d client:%d",
			 ts_data->c320, client->irq);
		/*
		 * "321f03e3 orr"@0xffffff8008a7f874 -> flags = 2 =
		 * IRQF_TRIGGER_FALLING; thread_fn and dev_id are NULL.
		 */
		retval = request_threaded_irq(ts_data->c320,
					      tpd_eint_interrupt_handler, NULL,
					      IRQF_TRIGGER_FALLING,
					      "TOUCH_PANEL-eint", NULL);
	} else {
		FTS_ERROR("Can not find touch eint device node!");
		retval = -ENODATA;
	}
	if (retval != 0) {
		FTS_ERROR("request irq failed");
		goto err_irq;
	}

	/*
	 * 0xffffff800a0fc954: the tpd_probe in mtk_tpd.c READS it and the three
	 * touch drivers write it, that is, it is tpd_load_status.
	 */
	tpd_load_status = 1;
	/* TPD_RES_Y is read as 32 bit ("b942b901 ldr"@0xffffff8008a7f890)
	 * while tpd.h declares it `unsigned long`: see DIVERGENCE 2. */
	FTS_DEBUG("TPD_RES_Y:%d", (u32)TPD_RES_Y);

	sensor_id = get_module_id();
	/*
	 * This is NOT an FTS_*: the format has neither a level nor a prefix.
	 * "yang.liu*****log***sensor_id=%d\n"@0xffffff800924f95c
	 */
	printk("yang.liu*****log***sensor_id=%d\n", sensor_id);

	name = fts_g_998ca50[sensor_id];
	if (name)
		strcpy(module_name, name);
	else
		strcpy(module_name, "unknown");

	/*
	 * unroll() was reconstructed from the factory kernel disassembly (0xffffff8008a7fa00).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
#pragma clang loop unroll(disable)
	for (i = 0; i < 5; i++) {
		fts_i2c_read_reg(client, FTS_REG_FW_VER, &fwver);
		/* "528000a0 mov"@0xffffff8008a7fa1c -> 5 ms */
		msleep(5);
		if ((fwver == 0x05) || (fwver == 0x03))
			break;
	}

	/*
	 * "940fab45 bl"@0xffffff8008a7fa60 sprintf, with SIX arguments: the fifth
	 * and the sixth are the same fwver ("2a0503e6 mov"@0xffffff8008a7fa5c).
	 */
	ret = sprintf(tp_info,
		      "TP IC :%s,TP module :%s,TP I2C adr : 0x%x,SW FirmWare:0x%x,Sample FirmWare:0x%x",
		      "ft8719", module_name, client->addr, fwver, fwver);
	fix_tp_proc_info(tp_info, ret);

	FTS_FUNC_EXIT();
	return 0;

err_irq:
	if (ts_data->c272) {
		kthread_stop(ts_data->c272);
		ts_data->c272 = NULL;
	}
err_wq:
	if (ts_data->c40)
		destroy_workqueue(ts_data->c40);
	kfree_safe(ts_data->c336);
	kfree_safe(ts_data->c328);
	devm_kfree(&client->dev, ts_data);
	FTS_FUNC_EXIT();
	return retval;
}

/*
 * tpd_remove @0xffffff8008a7fa94, 152 bytes, visibility t.
 * "f9405c13 ldr"@0xffffff8008a7faa0 reads client + 184: that is
 * i2c_get_clientdata, that is client->dev.driver_data. tpd_probe NEVER
 * calls i2c_set_clientdata, so here the pointer is whatever the i2c core
 * left there. It is a factory defect and is reproduced (rule 7).
 */
static int tpd_remove(struct i2c_client *client)
{
	struct fts_ts_data *ts_data = i2c_get_clientdata(client);

	FTS_FUNC_ENTER();
	fts_remove_sysfs(client);
	fts_ex_mode_exit(client);
	fts_release_apk_debug_channel(ts_data);
	fts_i2c_exit();
	kfree_safe(ts_data->c336);
	kfree_safe(ts_data->c328);
	if (ts_data->c40)
		destroy_workqueue(ts_data->c40);
	FTS_FUNC_EXIT();
	return 0;
}

/*
 * tpd_i2c_detect @0xffffff8008a7fb2c, 28 bytes, visibility t.
 * The eight bytes written into [x1] are "mtk-tpd\0", materialised with a
 * mov plus three movks: "d28e8da8 mov"@0xffffff8008a7fb2c = 0x746d = "mt".
 */
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	strcpy(info->type, TPD_DEVICE);
	return 0;
}

/*
 * tpd_local_init @0xffffff8008a7ef40, 176 bytes, visibility t.
 * THREE definitions of this name in the map (ilitek 0xffffff8008a56590,
 * goodix 0xffffff8008a775bc, focaltech 0xffffff8008a7ef40): this is the
 * third, and it is field +0x08 of the "fts_ts" tpd_driver_t.
 */
static int tpd_local_init(void)
{
	FTS_FUNC_ENTER();
	/* "aa1f03e0 mov"@0xffffff8008a7ef64 puts x0 = NULL (THIS_MODULE of a
	 * built-in) and "94002d94 bl"@0xffffff8008a7ef68 calls
	 * i2c_register_driver: it is i2c_add_driver. */
	if (i2c_add_driver(&tpd_i2c_driver) != 0) {
		FTS_ERROR("[TPD]: Unable to add fts i2c driver!!");
		FTS_FUNC_EXIT();
		/* "12800000 mov"@0xffffff8008a7ef94 -> -1, not an -Exxx */
		return -1;
	}

	/*
	 * "b94f9508 ldr"@0xffffff8008a7efa4 reads tpd_dts_data + 12;
	 * "b8404420 ldr"@0xffffff8008a7efb8 reads +16 and post-increments,
	 * "91007022 add"@0xffffff8008a7efb4 makes +16+28 = +44. These are
	 * use_tpd_button, tpd_key_num and tpd_key_dim_local of struct
	 * tpd_dts_info, that is, the base is 0xffffff800a0fbf88.
	 */
	if (tpd_dts_data.use_tpd_button) {
		tpd_button_setting(tpd_dts_data.tpd_key_num,
				   tpd_dts_data.tpd_key_local,
				   tpd_dts_data.tpd_key_dim_local);
	}

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0fc938).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	tpd_type_cap = 1;
	FTS_FUNC_EXIT();
	return 0;
}

/*
 * tpd_suspend @0xffffff8008a7eff0, 280 bytes, visibility t.
 * It inlines fts_irq_disable (the __func__ "fts_irq_disable"@0xffffff800924f67c
 * appears at "97dad11e bl"@0xffffff8008a7f05c, inside tpd_suspend).
 */
static void tpd_suspend(struct device *h)
{
	int ret = 0;
	struct fts_ts_data *ts_data = fts_data;

	FTS_FUNC_ENTER();
	/* "39451288 ldrb"@0xffffff8008a7f01c -> +324 */
	if (ts_data->c324) {
		FTS_INFO("Already in suspend state");
		return;
	}
	/* "39451688 ldrb"@0xffffff8008a7f034 -> +325 */
	if (ts_data->c325) {
		FTS_INFO("fw upgrade in process, can't suspend");
		return;
	}

	fts_irq_disable();

	/* "320007e2 orr"@0xffffff8008a7f0bc -> the value 3 into register 0xa5 */
	ret = fts_i2c_write_reg(ts_data->c0, FTS_REG_POWER_MODE, 3);
	if (ret < 0)
		FTS_ERROR("Set TP to sleep mode fail, ret=%d!", ret);

	ts_data->c324 = 1;
	FTS_FUNC_EXIT();
}

/*
 * tpd_resume @0xffffff8008a7f108, 508 bytes, visibility t.
 * It inlines fts_release_all_finger, fts_reset_proc and fts_irq_enable, in
 * that order (the three __func__ follow one another at 0xffffff8008a7f158,
 * 0xffffff8008a7f214 and 0xffffff8008a7f270).
 */
static void tpd_resume(struct device *h)
{
	struct fts_ts_data *ts_data = fts_data;

	FTS_FUNC_ENTER();
	/* "34000d48 cbz"@0xffffff8008a7f13c: if it is not suspended, return at once */
	if (!ts_data->c324) {
		FTS_DEBUG("Already in awake state");
		return;
	}

	fts_release_all_finger();

	/*
	 * "394042a8 ldrb"+"350002e8 cbnz"@0xffffff8008a7f1fc/fc: the reset happens
	 * only if c16 is ZERO, and tpd_probe sets it to 1, so it never happens.
	 * Reproduced as it stands.
	 */
	if (!ts_data->c16)
		fts_reset_proc(200);

	fts_tp_state_recovery(ts_data->c0);
	fts_irq_enable();
	ts_data->c324 = 0;
	FTS_FUNC_EXIT();
}

/*
 * tpd_driver_init @0xffffff8009383470, 168 bytes, in .init.text.
 * TWO definitions: the other is the goodix one at 0xffffff8009383418.
 * This is ours because it prints "Focaltech V2.2 20180321" and passes the
 * "fts_ts" tpd_driver_t (0xffffff800998c918) to tpd_driver_add.
 */
static int __init tpd_driver_init(void)
{
	FTS_FUNC_ENTER();
	FTS_INFO("Driver version: %s", FTS_DRIVER_VERSION);
	/* "97db35c4 bl"@0xffffff80093834a0 */
	tpd_get_dts_info();
	/*
	 * "7100083f cmp"@0xffffff80093834ac -> 2;
	 * "71002c3f cmp"@0xffffff80093834bc -> 0xb, that is `> 10`.
	 * The argument of the FTS_INFO that follows is the w1 register already
	 * loaded: "913a3400 add"@0xffffff80093834d0 only puts the format in x0.
	 */
	if (tpd_dts_data.touch_max_num < 2)
		tpd_dts_data.touch_max_num = 2;
	else if (tpd_dts_data.touch_max_num > 10)
		tpd_dts_data.touch_max_num = 10;
	FTS_INFO("tpd max touch num:%d", tpd_dts_data.touch_max_num);

	/* "36f80080 tbz"@0xffffff80093834e4 -> the test is `< 0` */
	if (tpd_driver_add(&tpd_device_driver) < 0)
		FTS_ERROR("[TPD]: Add FTS Touch driver failed!!");

	FTS_FUNC_EXIT();
	return 0;
}

/*
 * tpd_driver_exit() was reconstructed from the factory kernel disassembly (0xffffff80093acb2c, 80 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void __exit tpd_driver_exit(void)
{
	FTS_FUNC_ENTER();
	tpd_driver_remove(&tpd_device_driver);
	FTS_FUNC_EXIT();
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

MODULE_LICENSE("GPL");

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800924f7d5).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
