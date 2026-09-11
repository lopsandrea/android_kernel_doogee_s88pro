// SPDX-License-Identifier: GPL-2.0
/*
 * Ilitek TDDI touch panel, Doogee S88 Pro -- group E, touch and reporting.
 *
 * Twenty-two functions, [0xffffff8008a5aad8, 0xffffff8008a5da3c), 12132
 * bytes. The boundary was closed with three independent proofs: adjacency
 * with no padding, a .rodata cluster disjoint from the neighbours', and a
 * __LINE__ clash with the neighbouring groups.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include "ilitek.h"

/*
 * ilitek_tddi_fw_upgrade_handler_arg() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
extern int ilitek_tddi_fw_upgrade_handler_arg(void *data)
	__asm__("ilitek_tddi_fw_upgrade_handler");

/*
 * ===========================================================================
 * THE ILITEK FUNCTIONS OUTSIDE THE BLOCK -- declared, not defined
 * ===========================================================================
 * HEADER DELTA 2. The signatures come from the call sites in THIS block,
 * and the comment says how much each site really decides. None is defined
 * here: the link fails, and that is the honest outcome (rule 6).
 */

/*
 * Group D. "97fff16f bl"@0xffffff8008a5b8cc: x0 is a pointer to a string
 * ("sleep"@0xffffff800911c431, materialised by
 * "9110c400 add"@0xffffff8008a5b8c4), w1 a cleared integer
 * ("2a1f03e1 mov"@0xffffff8008a5b8c8), the return value tested on the sign
 * bit alone ("36f802e0 tbz"@0xffffff8008a5b8d4).
 */
int ilitek_tddi_ic_func_ctrl(const char *name, int ctrl);

/*
 * ilitek_calc_packet_checksum() was reconstructed from the factory kernel disassembly (0xffffff8008a5bd94, 48 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
u8 ilitek_calc_packet_checksum(u8 *buf, int len)
{
	int i;
	int somma = 0;

	for (i = 0; i < len; i++)
		somma += buf[i];

	/* "4b0803e0 neg"@0xffffff8008a5bdb4 */
	return (u8)-somma;
}

/*
 * ilitek_dump_data() was reconstructed from the factory kernel disassembly (0xffffff8008a5aad8, 396 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_dump_data(void *data, int width, int count, int per_row,
		      const char *label)
{
	int i;
	int columns;
	u8 *p8;
	u32 *p32;
	s16 *p16;

	/* "39656108 ldrb"@0xffffff8008a5aafc, "34000a28 cbz"@0xffffff8008a5ab00 */
	if (!ilitek_dbg_en)
		return;

	/*
	 * "1a88c078 csel"@0xffffff8008a5ab10 -- `gt`, so 0 and the negatives
	 * take the 31 from "320013e8 orr"@0xffffff8008a5ab08
	 */
	columns = (per_row > 0) ? per_row : 31;

	/* "b40008c0 cbz"@0xffffff8008a5ab14 */
	if (!data) {
		/* "\x013ILITEK: (%s, %d): The data going to dump is NULL\n"@0xffffff8009241c80,
		 * __func__ "ilitek_dump_data"@0xffffff8009241cb4, line 75 */
#line 75
		ILI_ERR("The data going to dump is NULL\n");
		return;
	}

	/* "\x01c\n\n"@0xffffff8009241cc5, "97db626a bl"@0xffffff8008a5ab2c */
	printk(KERN_CONT "\n\n");
	/* "\x01cILITEK: Dump %s data\n"@0xffffff8009241cca, x1 = label
	 * ("aa1603e1 mov"@0xffffff8008a5ab38) */
	printk(KERN_CONT "ILITEK: Dump %s data\n", label);
	/* "\x01cILITEK: "@0xffffff8009241ce2, "97db6263 bl"@0xffffff8008a5ab48 */
	printk(KERN_CONT "ILITEK: ");

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5ab54).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	p8 = (width == 8) ? data : NULL;
	p32 = (width == 32 || width == 10) ? data : NULL;
	p16 = (width == 16) ? data : NULL;

	for (i = 0; i < count; i++) {
		if (width == 8)
			printk(KERN_CONT " %4x ", p8[i]);
		else if (width == 32)
			printk(KERN_CONT " %4x ", p32[i]);
		else if (width == 10)
			printk(KERN_CONT " %4d ", p32[i]);
		else if (width == 16)
			printk(KERN_CONT " %4d ", p16[i]);

		if ((i % columns) == (columns - 1)) {
			/*
			 * "\x01c\n"@0xffffff80090ddcf1 -- it is NOT in the block's
			 * .rodata cluster: it is a string merged by the
			 * linker, "97db6234 bl"@0xffffff8008a5ac04
			 */
			printk(KERN_CONT "\n");
			printk(KERN_CONT "ILITEK: ");
		}
	}
	/* "97db622c bl"@0xffffff8008a5ac24 -- "\x01c\n\n" again */
	printk(KERN_CONT "\n\n");
}

/*
 * ilitek_tddi_touch_press() was reconstructed from the factory kernel disassembly (0xffffff8008a5ca74, 228 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_touch_press(u16 x, u16 y, u16 p, u16 id)
{
	/* "\x016ILITEK: (%s, %d): Touch Press: id = %d, x = %d, y = %d, p = %d\n"@0xffffff8009242a67,
	 * __func__ "ilitek_tddi_touch_press"@0xffffff8009242aa9, line 692,
	 * guard "34000168 cbz"@0xffffff8008a5caa0 */
#line 692
	ILI_DBG("Touch Press: id = %d, x = %d, y = %d, p = %d\n", id, x, y, p);

	/* "97ffa747 bl"@0xffffff8008a5cae4, x0 = idev->c16
	 * ("f9400900 ldr"@0xffffff8008a5cae0) */
	input_report_abs(idev->c16, ABS_MT_SLOT, id);
	/* "97ffba4c bl"@0xffffff8008a5caf8, w1 = 0 = MT_TOOL_FINGER
	 * ("2a1f03e1 mov"@0xffffff8008a5caf0), w2 = 1
	 * ("320003e2 orr"@0xffffff8008a5caec) */
	input_mt_report_slot_state(idev->c16, MT_TOOL_FINGER, true);
	/* "97ffa73c bl"@0xffffff8008a5cb10 */
	input_report_abs(idev->c16, ABS_MT_POSITION_X, x);
	/* "97ffa736 bl"@0xffffff8008a5cb28 */
	input_report_abs(idev->c16, ABS_MT_POSITION_Y, y);
	/* "97ffa730 bl"@0xffffff8008a5cb40 */
	input_report_abs(idev->c16, ABS_MT_PRESSURE, p);
}

/*
 * ilitek_tddi_touch_release() was reconstructed from the factory kernel disassembly (0xffffff8008a5cb58, 124 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_touch_release(u16 x, u16 y, u16 id)
{
	/* "\x016ILITEK: (%s, %d): Touch Release: id = %d, x = %d, y = %d\n"@0xffffff8009242ac1,
	 * line 717, guard "34000148 cbz"@0xffffff8008a5cb70 */
#line 717
	ILI_DBG("Touch Release: id = %d, x = %d, y = %d\n", id, x, y);

	/* "97ffa714 bl"@0xffffff8008a5cbb0 */
	input_report_abs(idev->c16, ABS_MT_SLOT, id);
	/* "97ffba19 bl"@0xffffff8008a5cbc4, w1 = 0 e w2 = 0
	 * ("2a1f03e1 mov"@0xffffff8008a5cbb8,
	 * "2a1f03e2 mov"@0xffffff8008a5cbbc) */
	input_mt_report_slot_state(idev->c16, MT_TOOL_FINGER, false);
}

/*
 * ilitek_tddi_touch_release_all_point() was reconstructed from the factory kernel disassembly (0xffffff8008a5cbd4, 232 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_touch_release_all_point(void)
{
	int i;

	/* "11000673 add"@0xffffff8008a5cc54 -- passo 1 */
	for (i = 0; i < 10; i++)
		ilitek_tddi_touch_release(0, 0, i);

	/* "97ffa6e3 bl"@0xffffff8008a5cc74 */
	input_report_key(idev->c16, BTN_TOUCH, 0);
	/* "97ffa6dd bl"@0xffffff8008a5cc8c */
	input_report_key(idev->c16, BTN_TOOL_FINGER, 0);
	/* "97ffa6d7 bl"@0xffffff8008a5cca4 */
	input_sync(idev->c16);
}

/*
 * ilitek_tddi_proximity_near() was reconstructed from the factory kernel disassembly (0xffffff8008a5b894, 172 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_proximity_near(int mode)
{
	int ret = 0;

	/* "3909f509 strb"@0xffffff8008a5b8ac -- idev+637 = 1 */
	idev->c637 = 1;

	switch (mode) {
	case 1:
		/* "97fff16f bl"@0xffffff8008a5b8cc, x0 = "sleep"@0xffffff800911c431,
		 * w1 = 0 ("2a1f03e1 mov"@0xffffff8008a5b8c8) */
		ret = ilitek_tddi_ic_func_ctrl("sleep", 0);
		/* "36f802e0 tbz"@0xffffff8008a5b8d4 -- the sign bit alone */
		if (ret < 0)
			/*
			 * "\x013ILITEK: (%s, %d): Write sleep in cmd failed\n"@0xffffff800923e559
			 * -- outside the block's cluster: it is merged with
			 * group D. Line 340.
			 */
#line 340
			ILI_ERR("Write sleep in cmd failed\n");
		break;
	case 0:
		/*
		 * "\x016ILITEK: (%s, %d): DDI POWER OFF, do nothing\n"@0xffffff8009241f99,
		 * line 343, NOT guarded by the diagnostic byte
		 */
#line 343
		ILI_INFO("DDI POWER OFF, do nothing\n");
		break;
	default:
		/* "\x013ILITEK: (%s, %d): Unknown mode (%d)\n"@0xffffff8009241fc8,
		 * line 346, w3 = mode ("2a0003e3 mov"@0xffffff8008a5b8b4) */
#line 346
		ILI_ERR("Unknown mode (%d)\n", mode);
		/* "128002b3 mov"@0xffffff8008a5b92c -- -22 = -EINVAL */
		ret = -EINVAL;
		break;
	}

	/* "2a1303e0 mov"@0xffffff8008a5b934 */
	return ret;
}

/*
 * ilitek_tddi_proximity_far() was reconstructed from the factory kernel disassembly (0xffffff8008a5b940, 320 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_proximity_far(int mode)
{
	int ret = 0;
	u8 cmd[2] = {0};

	/* "3949f508 ldrb"@0xffffff8008a5b968 + "34000128 cbz"@0xffffff8008a5b96c */
	if (!idev->c637) {
		/* "\x016ILITEK: (%s, %d): No proximity near event, break\n"@0xffffff8009241fef,
		 * line 359 */
#line 359
		ILI_INFO("No proximity near event, break\n");
		return 0;
	}

	switch (mode) {
	case 2:
		/*
		 * "97ffe155 bl"@0xffffff8008a5b984 -- the return value is NOT used:
		 * "2a1f03f3 mov"@0xffffff8008a5b988 clears `ret` right after
		 */
		ilitek_tddi_gesture_recovery();
		break;
	case 3:
		/* "79000be8 strh"@0xffffff8008a5b9c8 -- 0x0af6 in two byte */
		cmd[0] = 0xf6;
		cmd[1] = 0x0a;
		/* "\x016ILITEK: (%s, %d): write prepare gesture command 0xF6 0x0A\n"@0xffffff800924203d,
		 * line 379 */
#line 379
		ILI_INFO("write prepare gesture command 0xF6 0x0A\n");
		/* "f9418508 ldr"@0xffffff8008a5b9dc (idev+776) +
		 * "d63f0100 blr"@0xffffff8008a5b9e0, w1 = 2
		 * ("321f03e1 orr"@0xffffff8008a5b9d8) */
		ret = idev->c776(cmd, 2);
		/* "37f80260 tbnz"@0xffffff8008a5b9e4 */
		if (ret < 0) {
			/*
			 * "\x016ILITEK: (%s, %d): write prepare gesture command error\n"@0xffffff800924207a,
			 * line 382 -- KERN_INFO, not KERN_ERR, even though it is an
			 * error path. That is how it is in the binary.
			 */
#line 382
			ILI_INFO("write prepare gesture command error\n");
			break;
		}
		/* "32000fe0 orr"@0xffffff8008a5b9e8 = 0xf, then
		 * "97ffdfa8 bl"@0xffffff8008a5b9ec */
		ret = ilitek_tddi_switch_tp_mode(0x0f);
		/* "36f802c0 tbz"@0xffffff8008a5b9f4 */
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): Switch to gesture mode failed during proximity far\n"@0xffffff80092420b3,
			 * line 388 */
#line 388
			ILI_ERR("Switch to gesture mode failed during proximity far\n");
		break;
	default:
		/* same literal as `proximity_near`,
		 * "\x013ILITEK: (%s, %d): Unknown mode (%d)\n"@0xffffff8009241fc8,
		 * line 391 */
#line 391
		ILI_ERR("Unknown mode (%d)\n", mode);
		/* "128002b3 mov"@0xffffff8008a5ba28 */
		ret = -EINVAL;
		break;
	}

	/* "3909f51f strb"@0xffffff8008a5ba50 -- idev+637 = 0 */
	idev->c637 = 0;
	return ret;
}

/*
 * ilitek_tddi_move_gesture_code_flash() was reconstructed from the factory kernel disassembly (0xffffff8008a5ba80, 72 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_move_gesture_code_flash(int mode)
{
	/* "\x016ILITEK: (%s, %d): Switch to Gesture mode, lpwg cmd = %d\n"@0xffffff80092420fb,
	 * __func__ "ilitek_tddi_move_gesture_code_flash"@0xffffff8009242136,
	 * line 405, w3 = idev->c536 */
#line 405
	ILI_INFO("Switch to Gesture mode, lpwg cmd = %d\n", idev->c536);

	/* "97ffe072 bl"@0xffffff8008a5bab8 */
	return ilitek_set_tp_data_len(idev->c536);
}

/*
 * ===========================================================================
 * ilitek_tddi_move_mp_code_iram -- 0xffffff8008a5b864, 48 bytes
 * ===========================================================================
 * Factory line: 322 ("52802842 mov"@0xffffff8008a5b87c).
 *
 * D1: "aa1f03e0 mov"@0xffffff8008a5b884 clears x0 BEFORE the
 * "97ffe07c bl"@0xffffff8008a5b888. Group A never does that.
 */
int ilitek_tddi_move_mp_code_iram(void)
{
	/* "\x016ILITEK: (%s, %d): Download MP code to iram\n"@0xffffff8009241f32,
	 * __func__ "ilitek_tddi_move_mp_code_iram"@0xffffff8009241f60, line 322 */
#line 322
	ILI_INFO("Download MP code to iram\n");

	/* "97ffe07c bl"@0xffffff8008a5b888 */
	return ilitek_tddi_fw_upgrade_handler_arg(NULL);
}

/*
 * Group H. "9400521d bl"@0xffffff8008a5ccf8: x0 is the buffer, w1 the
 * length, the return value is unused.
 */
void netlink_reply_msg(void *data, int len);

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0fcaa0, 8 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct {
	/*
	 * +0: written by "78296b57 strh"@0xffffff8008a5c78c (the loop index),
	 * read back by "785fc2c3 ldurh"@0xffffff8008a5c840 as the FOURTH
	 * argument of `touch_press`, which the format calls `id`
	 */
	u16 c0;
	/* +2: written by "79000549 strh"@0xffffff8008a5c764, read back by
	 * "785fe2c0 ldurh"@0xffffff8008a5c84c come FIRST argument, `x` */
	u16 c2;
	/* +4: written by "79000949 strh"@0xffffff8008a5c780, read back by
	 * "794002c1 ldrh"@0xffffff8008a5c848 come SECOND argument, `y` */
	u16 c4;
	/* +6: written by "79000d49 strh"@0xffffff8008a5c79c, read back by
	 * "794006c2 ldrh"@0xffffff8008a5c844 come THIRD argument, `p` */
	u16 c6;
} points[10];

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5cd28, 16 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_record_debug {
	/* +0: one byte, read by "386a690a ldrb"@0xffffff8008a5cd2c and set to 1
	 * by "3829690a strb"@0xffffff8008a5cdac */
	u8 c0;
	/*
	 * +1..+7: no instruction in the block touches them. It is NOT padding
	 * that can be declared: it is a hole the binary does not describe.
	 */
	u8 __ignoto_1[7];
	/*
	 * +8: a pointer, "f9400500 ldr"@0xffffff8008a5cd70, the destination of
	 * a `memcpy`
	 */
	void *c8;
};

/*
 * ilitek_tddi_touch_send_debug_data() was reconstructed from the factory kernel disassembly (0xffffff8008a5ccbc, 288 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ilitek_tddi_touch_send_debug_data(void *data, int len)
{
	struct ilitek_record_debug *anello;
	int i;

	/* "91024100 add"@0xffffff8008a5ccdc (idev + 0x90 = 144) +
	 * "941059a7 bl"@0xffffff8008a5cce0 */
	mutex_lock(&idev->debug_mutex);

	/* "39484928 ldrb"@0xffffff8008a5cce8 + "34000168 cbz"@0xffffff8008a5ccec */
	if (idev->c530) {
		/* "9400521d bl"@0xffffff8008a5ccf8 */
		netlink_reply_msg(data, len);
		goto fine;
	}

	/* "3948b128 ldrb"@0xffffff8008a5cd18 + "34ffff08 cbz"@0xffffff8008a5cd1c */
	if (!idev->c556)
		goto fine;

	/* "b9423123 ldr"@0xffffff8008a5cd20 */
	i = idev->c560;
	/* "f9412928 ldr"@0xffffff8008a5cd24 */
	anello = idev->c592;

	/* "386a690a ldrb"@0xffffff8008a5cd2c + "340000ea cbz"@0xffffff8008a5cd30 */
	if (anello[i].c0) {
		/* "1a890143 csel"@0xffffff8008a5cd40 -- backwards, circular */
		i = (i == 0) ? 1023 : i - 1;
	} else {
		/*
		 * "b902312a str"@0xffffff8008a5cd68 -- forward, circular;
		 * `i` does NOT change, it is only the stored index that advances
		 */
		idev->c560 = (i + 1) % 1024;
	}

	/* "f9400500 ldr"@0xffffff8008a5cd70 + "b4000260 cbz"@0xffffff8008a5cd74 */
	if (!anello[i].c8) {
		/*
		 * "\x016ILITEK: (%s, %d): BUFFER %d error\n"@0xffffff800924305c,
		 * __func__ "ilitek_tddi_touch_send_debug_data"@0xffffff8009243081,
		 * line 677. The argument is `i`: the binary does NOT reload it into w3
		 * because w3 already holds it -- it is the index just computed.
		 */
#line 677
		ILI_INFO("BUFFER %d error\n", i);
		goto fine;
	}

	/* "940febdd bl"@0xffffff8008a5cd8c */
	memcpy(anello[i].c8, data, (len < 2048) ? len : 2048);
	/* "f94522a8 ldr"@0xffffff8008a5cd90 (re-read of `idev`),
	 * "f9412908 ldr"@0xffffff8008a5cda0 (re-read of `idev->c592`) e
	 * "3829690a strb"@0xffffff8008a5cdac */
	((struct ilitek_record_debug *)idev->c592)[i].c0 = 1;
	/*
	 * "9108e100 add"@0xffffff8008a5cdb4 (idev + 0x238 = 568) +
	 * "97db0172 bl"@0xffffff8008a5cdb8, with w1 = 3 = TASK_NORMAL
	 * ("320007e1 orr"@0xffffff8008a5cd9c), w2 = 1
	 * ("320003e2 orr"@0xffffff8008a5cda4) and x3 = NULL
	 * ("aa1f03e3 mov"@0xffffff8008a5cda8): it is `wake_up`, which is a macro
	 */
	wake_up(&idev->inq);

fine:
	/* "941059b2 bl"@0xffffff8008a5cd04 */
	mutex_unlock(&idev->debug_mutex);
}

/*
 * ilitek_tddi_report_ap_mode() was reconstructed from the factory kernel disassembly (0xffffff8008a5c680, 1012 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_report_ap_mode(u8 *data, int len)
{
	int i;
	u16 x, y;

	/* "a9047f5f stp"@0xffffff8008a5c6d0 and the four following: 80 byte */
	memset(points, 0x0, sizeof(points));
	/* "b901611f str"@0xffffff8008a5c6e4 */
	idev->c352 = 0;

	for (i = 0; i < 10; i++) {
		/* "7103fd3f cmp"@0xffffff8008a5c6fc,
		 * "7103fd1f cmp"@0xffffff8008a5c708,
		 * "7103fd1f cmp"@0xffffff8008a5c714 */
		if (data[(4 * i) + 1] == 0xff && data[(4 * i) + 2] == 0xff &&
		    data[(4 * i) + 3] == 0xff) {
			/* "b9016509 str"@0xffffff8008a5c7d8 with w9 = 0
			 * ("2a1f03e9 mov"@0xffffff8008a5c6f0) */
			idev->c356[i] = 0;
			continue;
		}

		/* "33180d63 bfi"@0xffffff8008a5c730 */
		x = ((data[(4 * i) + 1] & 0xf0) << 4) | data[(4 * i) + 2];
		/* "33180d24 bfi"@0xffffff8008a5c734 */
		y = ((data[(4 * i) + 1] & 0x0f) << 8) | data[(4 * i) + 3];

		/* "340000ca cbz"@0xffffff8008a5c738 */
		if (idev->c641) {
			/* "79000523 strh"@0xffffff8008a5c744 */
			points[idev->c352].c2 = x;
			points[idev->c352].c4 = y;
		} else {
			/* "7941e909 ldrh"@0xffffff8008a5c750 (idev+244) */
			points[idev->c352].c2 = x * idev->c244 >> 11;
			/* "7941ed09 ldrh"@0xffffff8008a5c768 (idev+246) */
			points[idev->c352].c4 = y * idev->c246 >> 11;
		}
		/* "78296b57 strh"@0xffffff8008a5c78c */
		points[idev->c352].c0 = i;
		/* "79000d49 strh"@0xffffff8008a5c79c */
		points[idev->c352].c6 = data[(4 * i) + 4];

		/* "\x016ILITEK: (%s, %d): original x = %d, y = %d\n"@0xffffff8009242b17,
		 * __func__ "ilitek_tddi_report_ap_mode"@0xffffff8009242b44,
		 * line 781, guard "340000cb cbz"@0xffffff8008a5c7a0 */
#line 781
		ILI_DBG("original x = %d, y = %d\n", x, y);

		/* "11000529 add"@0xffffff8008a5c7bc */
		idev->c352++;
		/* "b9016509 str"@0xffffff8008a5c7d8 with w9 = 1
		 * ("320003e9 orr"@0xffffff8008a5c7c4) */
		idev->c356[i] = 1;
	}

	/*
	 * "\x016ILITEK: (%s, %d): figner number = %d, LastTouch = %d\n"@0xffffff8009242b5f,
	 * line 787, guard "34000148 cbz"@0xffffff8008a5c7e4.
	 * "figner" is a FACTORY typo and is reproduced (rule 7).
	 * The format NAMES `idev->c436`: `LastTouch`. See the header delta.
	 */
#line 787
	ILI_DBG("figner number = %d, LastTouch = %d\n", idev->c352, idev->c436);

	/* "340009c9 cbz"@0xffffff8008a5c814 */
	if (idev->c352) {
		/*
		 * "7100053f cmp"@0xffffff8008a5c818 +
		 * "5400034b b.lt"@0xffffff8008a5c81c -- the loop guard is
		 * SEPARATE from the test above because `c352` is signed
		 */
		for (i = 0; i < idev->c352; i++) {
			/* "97ffa7f1 bl"@0xffffff8008a5c83c */
			input_report_key(idev->c16, BTN_TOUCH, 1);
			/*
			 * "94000089 bl"@0xffffff8008a5c850 -- a REAL `bl`:
			 * `touch_press` is NOT inlined. It is the negative
			 * control for D5.
			 */
			ilitek_tddi_touch_press(points[i].c2, points[i].c4,
						points[i].c6, points[i].c0);
			/* "97ffa7e6 bl"@0xffffff8008a5c868 */
			input_report_key(idev->c16, BTN_TOOL_FINGER, 1);
		}

		for (i = 0; i < 10; i++) {
			/* "b9416549 ldr"@0xffffff8008a5c89c +
			 * "350003a9 cbnz"@0xffffff8008a5c8a0, e
			 * "b9418d49 ldr"@0xffffff8008a5c8a4 +
			 * "7100053f cmp"@0xffffff8008a5c8a8 */
			if (idev->c356[i] == 0 && idev->c396[i] == 1)
				ilitek_tddi_touch_release(0, 0, i);
			/* "b9018d09 str"@0xffffff8008a5c918 */
			idev->c396[i] = idev->c356[i];
		}
		/* "97ffa7b1 bl"@0xffffff8008a5c93c */
		input_sync(idev->c16);
		/* "b901b509 str"@0xffffff8008a5ca48 with w9 = idev->c352
		 * ("b9416109 ldr"@0xffffff8008a5c944) */
		idev->c436 = idev->c352;
	} else if (idev->c436) {
		/* "b941b509 ldr"@0xffffff8008a5c94c +
		 * "340007e9 cbz"@0xffffff8008a5c950 */
		for (i = 0; i < 10; i++) {
			/* "b9416549 ldr"@0xffffff8008a5c96c +
			 * "350003a9 cbnz"@0xffffff8008a5c970 */
			if (idev->c356[i] == 0 && idev->c396[i] == 1)
				ilitek_tddi_touch_release(0, 0, i);
			/* "b9018d09 str"@0xffffff8008a5c9e8 */
			idev->c396[i] = idev->c356[i];
		}
		/* "97ffa77d bl"@0xffffff8008a5ca0c */
		input_report_key(idev->c16, BTN_TOUCH, 0);
		/* "97ffa777 bl"@0xffffff8008a5ca24 */
		input_report_key(idev->c16, BTN_TOOL_FINGER, 0);
		/* "97ffa771 bl"@0xffffff8008a5ca3c */
		input_sync(idev->c16);
		/* "b901b509 str"@0xffffff8008a5ca48 with w9 = 0
		 * ("2a1f03e9 mov"@0xffffff8008a5ca44) */
		idev->c436 = 0;
	}

	/* "9400009a bl"@0xffffff8008a5ca54 */
	ilitek_tddi_touch_send_debug_data(data, len);
}

/*
 * demo_debug_info_mode() was reconstructed from the factory kernel disassembly (0xffffff8008a5c610, 112 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void demo_debug_info_mode(u8 *data, long len)
{
	void (*f)(void *, unsigned long);
	u8 c43, c44;

	/* "52800561 mov"@0xffffff8008a5c620 (43) +
	 * "94000016 bl"@0xffffff8008a5c628 */
	ilitek_tddi_report_ap_mode(data, 43);

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5c648, 100 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	/* "3842ce74 ldrb"@0xffffff8008a5c62c -- pre-index with writeback:
	 * x19 becomes `data + 44` e w20 takes `data[44]` */
	c44 = data[44];
	/* "385ff275 ldurb"@0xffffff8008a5c63c -- `data + 44 - 1` */
	c43 = data[43];

	/*
	 * "\x016ILITEK: (%s, %d): info len = %d ,id = %d\n"@0xffffff8009242a26,
	 * __func__ "demo_debug_info_mode"@0xffffff8009242a52, line 646.
	 * It is NOT guarded by the diagnostic byte.
	 */
#line 646
	ILI_INFO("info len = %d ,id = %d\n", c43, c44);

	/* "8b140d08 add"@0xffffff8008a5c664 (`lsl #3`: pointers a 8 byte) +
	 * "f941a908 ldr"@0xffffff8008a5c668 (idev+848) +
	 * "d63f0100 blr"@0xffffff8008a5c66c */
	f = idev->c848[c44];
	f(&data[44], c43);
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0fcaf0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static u8 alternanza[10];

/*
 * ilitek_tddi_report_debug_mode() was reconstructed from the factory kernel disassembly (0xffffff8008a5cddc, 1040 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_report_debug_mode(u8 *data, int len)
{
	int i;
	u16 x, y;
	u8 pr;

	/* "a9047f5f stp"@0xffffff8008a5ce3c and the four following */
	memset(points, 0x0, sizeof(points));
	/* "b901611f str"@0xffffff8008a5ce50 */
	idev->c352 = 0;

	for (i = 0; i < 10; i++) {
		/* "7103fd1f cmp"@0xffffff8008a5ce70 (the first two, merged) and
		 * "7103fd1f cmp"@0xffffff8008a5ce7c (the third) */
		if (data[(3 * i) + 5] == 0xff && data[(3 * i) + 6] == 0xff &&
		    data[(3 * i) + 7] == 0xff) {
			/* "b9016509 str"@0xffffff8008a5cf4c with w9 = 0
			 * ("2a1f03e9 mov"@0xffffff8008a5ce5c) */
			idev->c356[i] = 0;
			continue;
		}

		/* "33180d63 bfi"@0xffffff8008a5ce94 */
		x = ((data[(3 * i) + 5] & 0xf0) << 4) | data[(3 * i) + 6];
		/* "33180d24 bfi"@0xffffff8008a5ce9c */
		y = ((data[(3 * i) + 5] & 0x0f) << 8) | data[(3 * i) + 7];

		/* "340000ca cbz"@0xffffff8008a5cea0 */
		if (idev->c641) {
			/* "79000523 strh"@0xffffff8008a5ceac */
			points[idev->c352].c2 = x;
			points[idev->c352].c4 = y;
		} else {
			/* "7941e909 ldrh"@0xffffff8008a5ceb8 (idev+244) */
			points[idev->c352].c2 = x * idev->c244 >> 11;
			/* "7941ed09 ldrh"@0xffffff8008a5ced0 (idev+246) */
			points[idev->c352].c4 = y * idev->c246 >> 11;
		}
		/* "78296b57 strh"@0xffffff8008a5cef4 */
		points[idev->c352].c0 = i;

		/* "1a93166a cinc"@0xffffff8008a5cf08 */
		pr = (alternanza[idev->c352] == 1) ? 2 : 1;
		/* "38296b8a strb"@0xffffff8008a5cf0c */
		alternanza[idev->c352] = pr;
		/* "79000d8a strh"@0xffffff8008a5cf10 */
		points[idev->c352].c6 = pr;

		/* same literal as `report_ap_mode`,
		 * "\x016ILITEK: (%s, %d): original x = %d, y = %d\n"@0xffffff8009242b17,
		 * __func__ "ilitek_tddi_report_debug_mode"@0xffffff8009242b97,
		 * line 871, guard "340000cb cbz"@0xffffff8008a5cf14 */
#line 871
		ILI_DBG("original x = %d, y = %d\n", x, y);

		/* "11000529 add"@0xffffff8008a5cf30 */
		idev->c352++;
		/* "b9016509 str"@0xffffff8008a5cf4c with w9 = 1
		 * ("320003e9 orr"@0xffffff8008a5cf38) */
		idev->c356[i] = 1;
	}

	/* line 877, guard "34000148 cbz"@0xffffff8008a5cf58 */
#line 877
	ILI_DBG("figner number = %d, LastTouch = %d\n", idev->c352, idev->c436);

	/* "340009c9 cbz"@0xffffff8008a5cf88 */
	if (idev->c352) {
		/* "5400034b b.lt"@0xffffff8008a5cf90 */
		for (i = 0; i < idev->c352; i++) {
			/* "97ffa614 bl"@0xffffff8008a5cfb0 */
			input_report_key(idev->c16, BTN_TOUCH, 1);
			/* "97fffeac bl"@0xffffff8008a5cfc4 -- `bl` VERA */
			ilitek_tddi_touch_press(points[i].c2, points[i].c4,
						points[i].c6, points[i].c0);
			/* "97ffa609 bl"@0xffffff8008a5cfdc */
			input_report_key(idev->c16, BTN_TOOL_FINGER, 1);
		}

		for (i = 0; i < 10; i++) {
			/* "350003a9 cbnz"@0xffffff8008a5d014 +
			 * "7100053f cmp"@0xffffff8008a5d01c */
			if (idev->c356[i] == 0 && idev->c396[i] == 1)
				ilitek_tddi_touch_release(0, 0, i);
			/* "b9018d09 str"@0xffffff8008a5d08c */
			idev->c396[i] = idev->c356[i];
		}
		/* "97ffa5d4 bl"@0xffffff8008a5d0b0 */
		input_sync(idev->c16);
		/* "b901b509 str"@0xffffff8008a5d1bc */
		idev->c436 = idev->c352;
	} else if (idev->c436) {
		/* "340007e9 cbz"@0xffffff8008a5d0c4 */
		for (i = 0; i < 10; i++) {
			/* "350003a9 cbnz"@0xffffff8008a5d0e4 */
			if (idev->c356[i] == 0 && idev->c396[i] == 1)
				ilitek_tddi_touch_release(0, 0, i);
			/* "b9018d09 str"@0xffffff8008a5d15c */
			idev->c396[i] = idev->c356[i];
		}
		/* "97ffa5a0 bl"@0xffffff8008a5d180 */
		input_report_key(idev->c16, BTN_TOUCH, 0);
		/* "97ffa59a bl"@0xffffff8008a5d198 */
		input_report_key(idev->c16, BTN_TOOL_FINGER, 0);
		/* "97ffa594 bl"@0xffffff8008a5d1b0 */
		input_sync(idev->c16);
		/* "b901b509 str"@0xffffff8008a5d1bc with w9 = 0
		 * ("2a1f03e9 mov"@0xffffff8008a5d1b8) */
		idev->c436 = 0;
	}

	/* "97fffebd bl"@0xffffff8008a5d1c8 */
	ilitek_tddi_touch_send_debug_data(data, len);
}

/*
 * ilitek_tddi_report_i2cuart_mode() was reconstructed from the factory kernel disassembly (0xffffff8008a5d804, 568 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_report_i2cuart_mode(u8 *data, int len)
{
	int type;
	int actual_len;
	int need_read_len;
	int one_data_bytes = 0;
	int uart_len;
	u8 *uart_buf = NULL;
	u8 *total_buf = NULL;
	int ret;

	/* "12000c76 and"@0xffffff8008a5d82c */
	type = data[3] & 0x0f;
	/* "51001435 sub"@0xffffff8008a5d830 */
	actual_len = len - 5;

	/* "\x016ILITEK: (%s, %d): data[3] = %x, type = %x, actual_len = %d\n"@0xffffff8009242cf4,
	 * line 1071, guard "34000128 cbz"@0xffffff8008a5d834 */
	ILI_DBG("data[3] = %x, type = %x, actual_len = %d\n",
#line 1071
		data[3], type, actual_len);

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5d858).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	need_read_len = data[1] * data[2];

	if (type == 0 || type == 1 || type == 6)
		/* "320003e4 orr"@0xffffff8008a5d86c */
		one_data_bytes = 1;
	else if (type == 2 || type == 3)
		/* "321f03e4 orr"@0xffffff8008a5d998 */
		one_data_bytes = 2;
	else if (type == 4 || type == 5)
		/* "531e7524 lsl"@0xffffff8008a5d9d0 */
		one_data_bytes = 4;

	/* "1b042916 madd"@0xffffff8008a5d888 */
	need_read_len = need_read_len * one_data_bytes + 1;

	/* "\x016ILITEK: (%s, %d): need_read_len = %d  one_data_bytes = %d\n"@0xffffff8009242d52,
	 * line 1084, guard "34000109 cbz"@0xffffff8008a5d88c */
	ILI_DBG("need_read_len = %d  one_data_bytes = %d\n",
#line 1084
		need_read_len, one_data_bytes);

	/*
	 * "6b1502d7 subs"@0xffffff8008a5d8ac + "540000aa b.ge"@0xffffff8008a5d8b0
	 * -- a single instruction for both the comparison AND the difference
	 */
	if (need_read_len < actual_len) {
		/* "97fffd00 bl"@0xffffff8008a5d8bc */
		ilitek_tddi_touch_send_debug_data(data, len);
		return;
	}

	uart_len = need_read_len - actual_len;
	/* "\x016ILITEK: (%s, %d): uart len = %d\n"@0xffffff8009242d8f,
	 * line 1092, guard "34000108 cbz"@0xffffff8008a5d8c8 */
#line 1092
	ILI_DBG("uart len = %d\n", uart_len);

	/*
	 * kzalloc() was reconstructed from the factory kernel disassembly (0xffffff8008a5d8e8).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	uart_buf = kzalloc(uart_len, GFP_KERNEL);
	if (IS_ERR(uart_buf) || uart_buf == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate uart_buf memory %ld\n"@0xffffff8009242db2,
		 * line 1096, x3 = the pointer
		 * ("aa1503e3 mov"@0xffffff8008a5d9b4) */
		ILI_ERR("Failed to allocate uart_buf memory %ld\n",
#line 1096
			PTR_ERR(uart_buf));
		/* "b4000375 cbz"@0xffffff8008a5d9bc */
		if (uart_buf)
			/* "97dfcb63 bl"@0xffffff8008a5da24 */
			kfree(uart_buf);
		return;
	}

	/* "f9418908 ldr"@0xffffff8008a5d91c (idev+784) +
	 * "d63f0100 blr"@0xffffff8008a5d920 */
	ret = idev->c784(uart_buf, uart_len);
	/* "37f805a0 tbnz"@0xffffff8008a5d924 */
	if (ret < 0) {
		/* "\x013ILITEK: (%s, %d): i2cuart read data failed\n"@0xffffff8009242dee,
		 * line 1101 -- no arguments */
#line 1101
		ILI_ERR("i2cuart read data failed\n");
		/* "97dfcb63 bl"@0xffffff8008a5da24, with no NULL test */
		kfree(uart_buf);
		return;
	}

	/*
	 * "0b1402e8 add"@0xffffff8008a5d928 + "93407d18 sxtw"@0xffffff8008a5d92c
	 * + "97dfca93 bl"@0xffffff8008a5d93c, with the same flags
	 * 0x14080c0 ("52901801 mov"@0xffffff8008a5d930): `kzalloc` again
	 */
	total_buf = kzalloc(uart_len + len, GFP_KERNEL);
	if (IS_ERR(total_buf) || total_buf == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate total_buf memory %ld\n"@0xffffff8009242e1c,
		 * line 1107, x3 = the pointer
		 * ("aa1703e3 mov"@0xffffff8008a5da0c) */
		ILI_ERR("Failed to allocate total_buf memory %ld\n",
#line 1107
			PTR_ERR(total_buf));
		/* "97dfcb66 bl"@0xffffff8008a5da18 -- `uart_buf` with no test */
		kfree(uart_buf);
		/* "b4000077 cbz"@0xffffff8008a5da1c */
		if (total_buf)
			kfree(total_buf);
		return;
	}

	/* "93407e94 sxtw"@0xffffff8008a5d950 + "940fe8e8 bl"@0xffffff8008a5d960 */
	memcpy(total_buf, data, len);
	/* "8b1402e0 add"@0xffffff8008a5d964 + "940fe8e4 bl"@0xffffff8008a5d970 */
	memcpy(total_buf + len, uart_buf, uart_len);
	/*
	 * "97fffcd0 bl"@0xffffff8008a5d97c, with the TOTAL length
	 * ("2a1803e1 mov"@0xffffff8008a5d978)
	 */
	ilitek_tddi_touch_send_debug_data(total_buf, uart_len + len);
	/* "97dfcb8b bl"@0xffffff8008a5d984 */
	kfree(uart_buf);
	/* "97dfcb63 bl"@0xffffff8008a5da24 */
	kfree(total_buf);
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5c338, 12 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_info_id0 {
	/* bits 0..7: "394023e3 ldrb"@0xffffff8008a5c378, with no mask */
	u8 id;
	/* bit 8..10: "12000903 and"@0xffffff8008a5c3a0 */
	u32 app_sys_powr_state_e:3;
	/* bit 11..13: "53031503 ubfx"@0xffffff8008a5c3bc */
	u32 app_sys_state_e:3;
	/* bit 14..15: "53061d03 ubfx"@0xffffff8008a5c3d8 */
	u32 tp_state_e:2;
	/* bit 16..17: "53082503 ubfx"@0xffffff8008a5c3f4 */
	u32 touch_palm_state_e:2;
	/* bit 18..20: "530a3103 ubfx"@0xffffff8008a5c410 */
	u32 app_an_statu_e:3;
	/* bit 21: "530d3503 ubfx"@0xffffff8008a5c42c */
	u32 app_sys_check_bg_abnormal:1;
	/* bit 22: "530e3903 ubfx"@0xffffff8008a5c448 */
	u32 g_b_wrong_bg:1;
	/* bit 23: A HOLE. No instruction in the block reads it. */
	u32 __ignoto_23:1;
	/*
	 * bits 24..27: "12000d03 and"@0xffffff8008a5c464 on the half-word read
	 * from offset 3
	 */
	u32 status_of_dynamic_th_e:4;
	/* bit 28..31: HOLE of four bits. */
	u32 __ignoto_28:4;
	/* bit 32..34: "12000903 and"@0xffffff8008a5c480 on the byte 4 */
	u32 algo_pt_status0:3;
	/* bit 35..37: "531b7503 ubfx"@0xffffff8008a5c49c */
	u32 algo_pt_status1:3;
	/*
	 * bits 38..40: "d35e8103 ubfx"@0xffffff8008a5c4b8 -- it straddles the
	 * boundary between byte 4 and byte 5
	 */
	u32 algo_pt_status2:3;
	/* bit 41..43: "d3618d03 ubfx"@0xffffff8008a5c4d4 */
	u32 algo_pt_status3:3;
	/* bit 44..46: "d3649903 ubfx"@0xffffff8008a5c4f0 */
	u32 algo_pt_status4:3;
	/* bit 47..49: "d367a503 ubfx"@0xffffff8008a5c50c */
	u32 algo_pt_status5:3;
	/* bit 50..52: "d36ab103 ubfx"@0xffffff8008a5c528 */
	u32 algo_pt_status6:3;
	/* bit 53..55: "d36dbd03 ubfx"@0xffffff8008a5c544 */
	u32 algo_pt_status7:3;
	/*
	 * bits 56..58: "12000903 and"@0xffffff8008a5c560 on the word read
	 * from offset 7
	 */
	u32 algo_pt_status8:3;
	/* bit 59..61: "d373d503 ubfx"@0xffffff8008a5c57c */
	u32 algo_pt_status9:3;
	/* bit 62..63: HOLE. See the file header. */
	u32 __ignoto_62:2;
	/*
	 * bit 64: "12000103 and"@0xffffff8008a5c598 on the word read from
	 * offset 8
	 */
	u32 hopping_flag:1;
	/* bit 65..69: "d379f503 ubfx"@0xffffff8008a5c5b4 */
	u32 hopping_index:5;
	/* bit 70..71: the two HIGH bits of `frequency` */
	u32 frequency_alti:2;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5c5c4).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	u32 frequency_bassi:8;
	/*
	 * bits 80..95: never read by any instruction in the block. They are
	 * there because the structure measures 12 bytes and the fields cover 10.
	 */
	u32 __ignoto_80:16;
} __packed;

/*
 * demo_debug_info_id0() was reconstructed from the factory kernel disassembly (0xffffff8008a5c318, 760 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void demo_debug_info_id0(void *data, unsigned long len)
{
	struct ilitek_info_id0 s;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5c330).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	int l = (int)len;

	/* "940fee6c bl"@0xffffff8008a5c350 */
	memcpy(&s, data, (l < (int)sizeof(s)) ? (size_t)l : sizeof(s));

	/* "\x016ILITEK: (%s, %d): id0 len = %d, struct size = %d\n"@0xffffff800924264a,
	 * __func__ "demo_debug_info_id0"@0xffffff800924267e, line 610,
	 * w3 = len truncated ("2a1403e3 mov"@0xffffff8008a5c370), w4 = 12 */
#line 610
	ILI_INFO("id0 len = %d, struct size = %d\n", l, 12);
#line 612
	ILI_INFO("id = %d\n", s.id);
	ILI_INFO("app_sys_powr_state_e = %d\n", s.app_sys_powr_state_e);
	ILI_INFO("app_sys_state_e = %d\n", s.app_sys_state_e);
	ILI_INFO("tp_state_e = %d\n", s.tp_state_e);
	ILI_INFO("touch_palm_state_e = %d\n", s.touch_palm_state_e);
	ILI_INFO("app_an_statu_e = %d\n", s.app_an_statu_e);
	ILI_INFO("app_sys_check_bg_abnormal = %d\n", s.app_sys_check_bg_abnormal);
	ILI_INFO("g_b_wrong_bg = %d\n", s.g_b_wrong_bg);
	ILI_INFO("status_of_dynamic_th_e = %d\n", s.status_of_dynamic_th_e);
	ILI_INFO("algo_pt_status0 = %d\n", s.algo_pt_status0);
	ILI_INFO("algo_pt_status1 = %d\n", s.algo_pt_status1);
	ILI_INFO("algo_pt_status2 = %d\n", s.algo_pt_status2);
	ILI_INFO("algo_pt_status3 = %d\n", s.algo_pt_status3);
	ILI_INFO("algo_pt_status4 = %d\n", s.algo_pt_status4);
	ILI_INFO("algo_pt_status5 = %d\n", s.algo_pt_status5);
	ILI_INFO("algo_pt_status6 = %d\n", s.algo_pt_status6);
	ILI_INFO("algo_pt_status7 = %d\n", s.algo_pt_status7);
	ILI_INFO("algo_pt_status8 = %d\n", s.algo_pt_status8);
	ILI_INFO("algo_pt_status9 = %d\n", s.algo_pt_status9);
	ILI_INFO("hopping_flag = %d\n", s.hopping_flag);
	ILI_INFO("hopping_index = %d\n", s.hopping_index);
	/* "2a090103 orr"@0xffffff8008a5c5d8 */
#line 633
	ILI_INFO("frequency = %d\n", (s.frequency_alti << 8) | s.frequency_bassi);
}

/*
 * Group D. The signatures come from the call sites in THIS block.
 * "97ffeced bl"@0xffffff8008a5bdf4: two integers, the return value tested on
 * the sign bit alone ("36f800e0 tbz"@0xffffff8008a5bdf8).
 */
int ilitek_ice_mode_ctrl(int enable, int mcu);
/*
 * Group D. "97ffec90 bl"@0xffffff8008a5be58 (towards ilitek_ice_mode_write,
 * 0xffffff8008a57098): w0 a 32-bit address, w1 a 32-bit datum, w2 the
 * length in bytes ("321e03e2 orr"@0xffffff8008a5be54, that is 4).
 */
int ilitek_ice_mode_write(u32 addr, u32 data, int len);
/*
 * Group D. "97ffebf9 bl"@0xffffff8008a5befc: w0 a 32-bit address,
 * x1 a POINTER ("910013e1 add"@0xffffff8008a5bef0 -- the address of a
 * local on the stack), w2 the length.
 */
int ilitek_ice_mode_read(u32 addr, void *data, int len);

/*
 * ilitek_tddi_touch_esd_gesture_flash() was reconstructed from the factory kernel disassembly (0xffffff8008a5bdc4, 588 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_touch_esd_gesture_flash(void)
{
	int ret = 0;
	int retry = 100;
	u32 answer = 0;

	/* "97ffeced bl"@0xffffff8008a5bdf4 */
	if (ilitek_ice_mode_ctrl(1, 0) < 0)
		/* "\x013ILITEK: (%s, %d): Enable ice mode failed during gesture recovery\n"@0xffffff80092423dd,
		 * __func__ "ilitek_tddi_touch_esd_gesture_flash"@0xffffff8009242421,
		 * line 490 */
#line 490
		ILI_ERR("Enable ice mode failed during gesture recovery\n");

	/* "\x016ILITEK: (%s, %d): ESD Gesture PWD Addr = 0x%x, Answer = 0x%x\n"@0xffffff8009242445,
	 * line 493, NOT guarded */
	ILI_INFO("ESD Gesture PWD Addr = 0x%x, Answer = 0x%x\n",
#line 493
		 0x40054, 0xa67c9dfe);

	/* "97ffec90 bl"@0xffffff8008a5be58 */
	if (ilitek_ice_mode_write(0x40054, 0xf38a94ef, 4) < 0)
		/* "\x013ILITEK: (%s, %d): write password failed\n"@0xffffff8009242485,
		 * line 497 */
#line 497
		ILI_ERR("write password failed\n");

	/* "b901411f str"@0xffffff8008a5be84 -- idev+320 = 0, BEFORE the reset */
	idev->c320 = 0;
	/* "b9426500 ldr"@0xffffff8008a5be80 (idev+612) +
	 * "97ffdf30 bl"@0xffffff8008a5be88 */
	if (ilitek_tddi_reset_ctrl(idev->c612) < 0)
		/* "\x013ILITEK: (%s, %d): TP Reset failed during gesture recovery\n"@0xffffff80092424b0,
		 * line 502 */
#line 502
		ILI_ERR("TP Reset failed during gesture recovery\n");

	/* "97ffecbe bl"@0xffffff8008a5beb0, this time with (1, 1) */
	if (ilitek_ice_mode_ctrl(1, 1) < 0)
		/* same literal as line 490, line 505 */
#line 505
		ILI_ERR("Enable ice mode failed during gesture recovery\n");

	do {
		/* "97ffebf9 bl"@0xffffff8008a5befc */
		if (ilitek_ice_mode_read(0x40054, &answer, 4) < 0)
			/* "\x013ILITEK: (%s, %d): Read gesture answer error\n"@0xffffff80092424ed,
			 * line 510 */
#line 510
			ILI_ERR("Read gesture answer error\n");

		/* "6b17007f cmp"@0xffffff8008a5bf18 */
		if (answer != 0xa67c9dfe)
			/* "\x016ILITEK: (%s, %d): answer = 0x%x != (0x%x)\n"@0xffffff800924251c,
			 * line 512, NOT guarded */
#line 512
			ILI_INFO("answer = 0x%x != (0x%x)\n", answer, 0xa67c9dfe);

		/* "940feecd bl"@0xffffff8008a5bf40 */
		mdelay(1);
		retry--;
	} while (answer != 0xa67c9dfe && retry > 0);

	/* "5400012c b.gt"@0xffffff8008a5bf64 */
	if (retry <= 0) {
		/* "\x013ILITEK: (%s, %d): Enter gesture failed\n"@0xffffff8009242549,
		 * line 518 */
#line 518
		ILI_ERR("Enter gesture failed\n");
		/* "12800013 mov"@0xffffff8008a5bf80 */
		ret = -1;
	} else {
		/* "\x016ILITEK: (%s, %d): Enter gesture successfully\n"@0xffffff8009242573,
		 * line 521 */
#line 521
		ILI_INFO("Enter gesture successfully\n");
		/* "2a1f03f3 mov"@0xffffff8008a5bfa0 */
		ret = 0;
	}

	/* "97ffec7f bl"@0xffffff8008a5bfac, con (0, 1) */
	if (ilitek_ice_mode_ctrl(0, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Disable ice mode failed during gesture recovery\n"@0xffffff80092425a3,
		 * line 525 */
#line 525
		ILI_ERR("Disable ice mode failed during gesture recovery\n");

	/*
	 * "f9419909 ldr"@0xffffff8008a5bfd0 (idev+816) +
	 * "b9421900 ldr"@0xffffff8008a5bfd4 (idev+536) +
	 * "d63f0120 blr"@0xffffff8008a5bfd8. The return value is NOT used: the
	 * epilogue returns w19 ("2a1303e0 mov"@0xffffff8008a5bff0).
	 * Field +816 is `ilitek_tddi_move_gesture_code_flash`, and this is an
	 * independent CONFIRMATION of what group B had written.
	 */
	idev->c816(idev->c536);

	return ret;
}

/*
 * ilitek_tddi_move_gesture_code_iram() was reconstructed from the factory kernel disassembly (0xffffff8008a5bac8, 716 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_move_gesture_code_iram(int mode)
{
	int i;
	u8 cmd[3] = {0};

	/* "3949f908 ldrb"@0xffffff8008a5bb08 (idev+638) +
	 * "34000b88 cbz"@0xffffff8008a5bb0c */
	if (!idev->c638) {
		/* "\x016ILITEK: (%s, %d): Switch to Gesture mode, lpwg cmd = %d,\n no need to load gesture code by driver\n"@0xffffff800924215a,
		 * line 418, w3 = mode ("2a1303e3 mov"@0xffffff8008a5bc90) */
		ILI_INFO("Switch to Gesture mode, lpwg cmd = %d,\n no need to load gesture code by driver\n",
#line 418
			 mode);
		/* "97ffdff9 bl"@0xffffff8008a5bc9c */
		if (ilitek_set_tp_data_len(mode) < 0)
			/* "\x013ILITEK: (%s, %d): Failed to set tp data length\n"@0xffffff80092421e1,
			 * line 420 */
#line 420
			ILI_ERR("Failed to set tp data length\n");
		return 0;
	}

	/* "\x016ILITEK: (%s, %d): Load gesture code by driver\n"@0xffffff8009242213,
	 * line 423 */
#line 423
	ILI_INFO("Load gesture code by driver\n");
	/* "97fff0d5 bl"@0xffffff8008a5bb34, x0 = "lpwg"@0xffffff800923e90a
	 * (merged with group D), w1 = 3 ("320007e1 orr"@0xffffff8008a5bb30) */
	if (ilitek_tddi_ic_func_ctrl("lpwg", 3) < 0)
		/* "\x013ILITEK: (%s, %d): write gesture flag failed\n"@0xffffff8009242244,
		 * line 427 */
#line 427
		ILI_ERR("write gesture flag failed\n");

	/* same literal as `move_gesture_code_flash`, line 429 */
#line 429
	ILI_INFO("Switch to Gesture mode, lpwg cmd = %d\n", mode);
	/* "97ffe043 bl"@0xffffff8008a5bb74 */
	if (ilitek_set_tp_data_len(mode) < 0)
		/* line 431 */
#line 431
		ILI_ERR("Failed to set tp data length\n");

	/*
	 * "7100271f cmp"@0xffffff8008a5bc54 + "54fffb89 b.ls"@0xffffff8008a5bc58
	 * -- an UNSIGNED comparison with 9, that is ten iterations
	 */
	for (i = 0; i < 10; i++) {
		/* "79000bf9 strh"@0xffffff8008a5bbcc con w25 = 0xaf6
		 * ("52815ed9 mov"@0xffffff8008a5bba8) */
		cmd[0] = 0xf6;
		cmd[1] = 0x0a;
		/*
		 * "d63f0100 blr"@0xffffff8008a5bbdc, w1 = 2
		 * ("321f03e1 orr"@0xffffff8008a5bbd4). The return value is NOT used.
		 */
		idev->c776(cmd, 2);

		/* "79000bfa strh"@0xffffff8008a5bbe4 con w26 = 0xa01
		 * ("5281403a mov"@0xffffff8008a5bbac) e
		 * "39001bfb strb"@0xffffff8008a5bbe8 con w27 = 5
		 * ("528000bb mov"@0xffffff8008a5bbb0) */
		cmd[0] = 0x01;
		cmd[1] = 0x0a;
		cmd[2] = 0x05;
		/* "d63f0100 blr"@0xffffff8008a5bbf8, w1 = 3 */
		if (idev->c776(cmd, 3) < 0)
			/* line 444, NO newline */
#line 444
			ILI_ERR("write 0x1,0xA,0x5 error");

		/* "f9418908 ldr"@0xffffff8008a5bc1c (idev+784) +
		 * "d63f0100 blr"@0xffffff8008a5bc20, w1 = 1 */
		if (idev->c784(cmd, 1) < 0)
			/* "\x013ILITEK: (%s, %d): read gesture ready byte error\n"@0xffffff800924229f,
			 * line 447 */
#line 447
			ILI_ERR("read gesture ready byte error\n");

		/* "\x016ILITEK: (%s, %d): gesture ready byte = 0x%x\n"@0xffffff80092422d2,
		 * line 449, guard "340000c8 cbz"@0xffffff8008a5bc2c */
#line 449
		ILI_DBG("gesture ready byte = 0x%x\n", cmd[0]);

		/* "7102447f cmp"@0xffffff8008a5bc48 */
		if (cmd[0] == 0x91) {
			/* "\x016ILITEK: (%s, %d): Gesture check fw ready\n"@0xffffff8009242301,
			 * line 451 */
#line 451
			ILI_INFO("Gesture check fw ready\n");
			/* D1: "aa1f03e0 mov"@0xffffff8008a5bcd4 +
			 * "97ffdf68 bl"@0xffffff8008a5bcd8 */
			if (ilitek_tddi_fw_upgrade_handler_arg(NULL) < 0)
				/* "\x013ILITEK: (%s, %d): FW upgrade failed during moving code\n"@0xffffff8009242377,
				 * line 462 */
#line 462
				ILI_ERR("FW upgrade failed during moving code\n");

			/* "79000be8 strh"@0xffffff8008a5bd00 (0xa01) +
			 * "39001be8 strb"@0xffffff8008a5bd08 (6) */
			cmd[0] = 0x01;
			cmd[1] = 0x0a;
			cmd[2] = 0x06;
			/* "d63f0100 blr"@0xffffff8008a5bd18 */
			if (idev->c776(cmd, 3) < 0)
				/* line 469, NO newline */
#line 469
				ILI_ERR("write 0x1,0xA,0x6 error");
			return 0;
		}
	}

	/*
	 * "\x013ILITEK: (%s, %d): Gesture is not ready (0x%x), try to run its recovery\n"@0xffffff800924232d,
	 * line 457. w3 still holds `cmd[0]`
	 * ("394013e3 ldrb"@0xffffff8008a5bc44): the binary does not reload it.
	 */
#line 457
	ILI_ERR("Gesture is not ready (0x%x), try to run its recovery\n", cmd[0]);
	/*
	 * "97ffe060 bl"@0xffffff8008a5bd58 -- the RETURN VALUE is the
	 * function's, not zero
	 */
	return ilitek_tddi_gesture_recovery();
}

/*
 * ilitek_tddi_touch_esd_gesture_iram() was reconstructed from the factory kernel disassembly (0xffffff8008a5c010, 776 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_touch_esd_gesture_iram(void)
{
	int i;
	u32 addr;
	u32 answer = 0;
	u8 cmd[3] = {0};

	/* "97ffec57 bl"@0xffffff8008a5c04c */
	if (ilitek_ice_mode_ctrl(1, 0) < 0)
		/* line 539 */
#line 539
		ILI_ERR("Enable ice mode failed during gesture recovery\n");

	/*
	 * "f9401908 ldr"@0xffffff8008a5c080 (idev+48) +
	 * "b9403508 ldr"@0xffffff8008a5c090 (+52 inside the object pointed to).
	 * `ilitek.h` declares `idev->c48` as `void *` and does not describe what
	 * it points at: here the access is written with an explicit offset, and the
	 * real structure is a HEADER DELTA the merge batch will have to decide on
	 * once the group that writes it has measured it.
	 */
	addr = (*(u32 *)((u8 *)idev->c48 + 52) > 0x10401) ? 0x40054 : 0x25ff8;

	/* same literal as the flash variant, line 547 */
	ILI_INFO("ESD Gesture PWD Addr = 0x%x, Answer = 0x%x\n",
#line 547
		 addr, 0x5b92e7f4);

	/* "97ffebef bl"@0xffffff8008a5c0dc */
	if (ilitek_ice_mode_write(addr, 0xf38a94ef, 4) < 0)
		/* line 551 */
#line 551
		ILI_ERR("write password failed\n");

	/* "b901411f str"@0xffffff8008a5c104 */
	idev->c320 = 0;
	/* D1: "97ffde5c bl"@0xffffff8008a5c108 */
	if (ilitek_tddi_fw_upgrade_handler_arg(NULL) < 0)
		/* "\x013ILITEK: (%s, %d): FW upgrade failed during gesture recovery\n"@0xffffff800924260b,
		 * line 556 */
#line 556
		ILI_ERR("FW upgrade failed during gesture recovery\n");

	/* "390a010a strb"@0xffffff8008a5c134 -- idev+640 = 1 */
	idev->c640 = 1;
	/* "3949f109 ldrb"@0xffffff8008a5c130 (idev+636) +
	 * "350000a9 cbnz"@0xffffff8008a5c138, e alternatively
	 * "6b14011f cmp"@0xffffff8008a5c144 + "54000069 b.ls"@0xffffff8008a5c148
	 * (unsigned, against 0x10400) */
	if (idev->c636 || *(u32 *)((u8 *)idev->c48 + 52) > 0x10400)
		/* "97dbdab1 bl"@0xffffff8008a5c150, w0 = 50 */
		msleep(50);

	/* "97ffec13 bl"@0xffffff8008a5c15c */
	if (ilitek_ice_mode_ctrl(1, 1) < 0)
		/* line 565 */
#line 565
		ILI_ERR("Enable ice mode failed during gesture recovery\n");

	for (i = 100; i > 0; i--) {
		/* "97ffeb4f bl"@0xffffff8008a5c1a4 */
		if (ilitek_ice_mode_read(addr, &answer, 4) < 0)
			/* line 570 */
#line 570
			ILI_ERR("Read gesture answer error\n");

		/* "6b18007f cmp"@0xffffff8008a5c1c0 */
		if (answer != 0x5b92e7f4)
			/* line 573 */
			ILI_INFO("answer = 0x%x != (0x%x)\n",
#line 573
				 answer, 0x5b92e7f4);

		/* "940fee23 bl"@0xffffff8008a5c1e8 */
		mdelay(1);

		/* "54000160 b.eq"@0xffffff8008a5c1f4 */
		if (answer == 0x5b92e7f4)
			break;
	}

	if (answer != 0x5b92e7f4) {
		/* line 578 */
#line 578
		ILI_ERR("Enter gesture failed\n");
		/* "128007a0 mov"@0xffffff8008a5c218 */
		return -ETIME;
	}

	/* line 581 */
#line 581
	ILI_INFO("Enter gesture successfully\n");

	/* "97ffebda bl"@0xffffff8008a5c240 */
	if (ilitek_ice_mode_ctrl(0, 1) < 0)
		/* line 585 */
#line 585
		ILI_ERR("Disable ice mode failed during gesture recovery\n");

	/* "b9014109 str"@0xffffff8008a5c26c -- idev+320 = 0xf
	 * ("32000fe9 orr"@0xffffff8008a5c264) */
	idev->c320 = 0x0f;
	/* "97ffde84 bl"@0xffffff8008a5c270 */
	ilitek_set_tp_data_len(idev->c536);
	/* D1: "aa1f03e0 mov"@0xffffff8008a5c274 +
	 * "97ffde00 bl"@0xffffff8008a5c278 */
	if (ilitek_tddi_fw_upgrade_handler_arg(NULL) < 0)
		/* line 590 */
#line 590
		ILI_ERR("FW upgrade failed during gesture recovery\n");

	/* "79000be8 strh"@0xffffff8008a5c2a0 (0xa01) +
	 * "39001be8 strb"@0xffffff8008a5c2a8 (6) */
	cmd[0] = 0x01;
	cmd[1] = 0x0a;
	cmd[2] = 0x06;
	/* "d63f0100 blr"@0xffffff8008a5c2b8 */
	if (idev->c776(cmd, 3) < 0)
		/* line 597, NO newline */
#line 597
		ILI_ERR("write 0x1,0xA,0x6 error");

	/* "390a011f strb"@0xffffff8008a5c2e0 -- idev+640 = 0 */
	idev->c640 = 0;
	return 0;
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5d284, 56 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_gesture {
	/*
	 * +0 u16: "790002e3 strh"@0xffffff8008a5d298, and read back as the
	 * `switch` selector ("794002ea ldrh"@0xffffff8008a5d388)
	 */
	u16 c0;
	/*
	 * +2 one byte: "39000ae8 strb"@0xffffff8008a5d4c8. The format calls it
	 * `clockwise`.
	 */
	u8 c2;
	/*
	 * +3: no instruction touches it. It is needed all the same to align the
	 * 4-byte field that follows.
	 */
	u8 __ignoto_3;
	/*
	 * +4 32-bit: "b90006e8 str"@0xffffff8008a5d544 and
	 * "b94006e4 ldr"@0xffffff8008a5d6d4. The format calls it `Type`.
	 */
	int c4;
	/*
	 * +8: six elements of eight bytes. Of the first and the last `u16` of
	 * each element (offsets +0 and +6) the binary says nothing in this
	 * block: they are cleared and never touched again.
	 */
	struct {
		u16 c0;
		/* "790016e9 strh"@0xffffff8008a5d2c0 for element 0 */
		u16 c2;
		/* "79001ae9 strh"@0xffffff8008a5d2d0 for element 0 */
		u16 c4;
		u16 c6;
	} pos[6];
};

/*
 * ilitek_tddi_report_gesture_mode() was reconstructed from the factory kernel disassembly (0xffffff8008a5d1ec, 1560 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_report_gesture_mode(u8 *data, int len)
{
	struct ilitek_gesture *g;
	struct input_dev *in;
	int transfer;
	u8 buf[170];
	int i;
	u8 code;
	u16 xa, ya, xb, yb;

	/* "a90d7fff stp"@0xffffff8008a5d228 and the others */
	memset(buf, 0x0, sizeof(buf));

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5d220).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	g = idev->c64;
	in = idev->c16;
	transfer = idev->c641;

	for (i = 0; i < len; i++)
		buf[i] = data[i];

	/* "f90002ff str"@0xffffff8008a5d284 and the three `stp`s before it */
	memset(g, 0x0, sizeof(*g));

	/* "790002e3 strh"@0xffffff8008a5d298 */
	code = buf[1];
	g->c0 = code;
	/* "\x016ILITEK: (%s, %d): gesture code = 0x%x, score = %d\n"@0xffffff8009242bb5,
	 * __func__ "ilitek_tddi_report_gesture_mode"@0xffffff8009242bea,
	 * line 932, w3 = buf[1] e w4 = buf[36]
	 * ("394173e4 ldrb"@0xffffff8008a5d29c) */
#line 932
	ILI_INFO("gesture code = 0x%x, score = %d\n", code, buf[36]);

	/* "33180d49 bfi"@0xffffff8008a5d2bc and the pair
	 * "121c0d0a and"@0xffffff8008a5d2b4 + "53047d4a lsr"@0xffffff8008a5d2b8 */
	g->pos[0].c2 = ((buf[4] & 0xf0) << 4) | buf[5];
	/* "12000d08 and"@0xffffff8008a5d2c8 + "33180d09 bfi"@0xffffff8008a5d2cc */
	g->pos[0].c4 = ((buf[4] & 0x0f) << 8) | buf[6];
	g->pos[1].c2 = ((buf[7] & 0xf0) << 4) | buf[8];
	g->pos[1].c4 = ((buf[7] & 0x0f) << 8) | buf[9];
	g->pos[2].c2 = ((buf[16] & 0xf0) << 4) | buf[17];
	g->pos[2].c4 = ((buf[16] & 0x0f) << 8) | buf[18];
	g->pos[3].c2 = ((buf[19] & 0xf0) << 4) | buf[20];
	g->pos[3].c4 = ((buf[19] & 0x0f) << 8) | buf[21];
	g->pos[4].c2 = ((buf[22] & 0xf0) << 4) | buf[23];
	g->pos[4].c4 = ((buf[22] & 0x0f) << 8) | buf[24];
	g->pos[5].c2 = ((buf[25] & 0xf0) << 4) | buf[26];
	g->pos[5].c4 = ((buf[25] & 0x0f) << 8) | buf[27];

	switch (g->c0) {
	case 0x58:
		/* "\x016ILITEK: (%s, %d): Double Click key event\n"@0xffffff8009242c0a,
		 * line 950 */
#line 950
		ILI_INFO("Double Click key event\n");
		/* "97ffa504 bl"@0xffffff8008a5d3f0, w2 = 116 = KEY_POWER
		 * ("52800e82 mov"@0xffffff8008a5d3e0) */
		input_report_key(in, KEY_POWER, 1);
		/* "97ffa4ff bl"@0xffffff8008a5d404 */
		input_sync(in);
		/* "97ffa4fa bl"@0xffffff8008a5d418 */
		input_report_key(in, KEY_POWER, 0);
		/* "97ffa4f5 bl"@0xffffff8008a5d42c */
		input_sync(in);
		/* "b90006e9 str"@0xffffff8008a5d438 with w9 = 0x58 */
		g->c4 = 0x58;
		/* "39000af9 strb"@0xffffff8008a5d43c */
		g->c2 = 1;
		/*
		 * "b840a2e8 ldur"@0xffffff8008a5d430 +
		 * "b80122e8 stur"@0xffffff8008a5d440: clang merges the two adjacent
		 * `u16`s into a single word
		 */
		g->pos[1].c2 = g->pos[0].c2;
		g->pos[1].c4 = g->pos[0].c4;
		break;
	/*
	 * ILITEK_SIMPLE_GESTURE() was reconstructed from the factory kernel disassembly.
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
#define ILITEK_SIMPLE_GESTURE(code)					\
	do {								\
		g->c4 = (code);					\
		g->c2 = 1;						\
	} while (0)

	case 0x60:
		ILITEK_SIMPLE_GESTURE(0x60);
		break;
	case 0x61:
		ILITEK_SIMPLE_GESTURE(0x61);
		break;
	case 0x62:
		ILITEK_SIMPLE_GESTURE(0x62);
		break;
	case 0x63:
		ILITEK_SIMPLE_GESTURE(0x63);
		break;
	case 0x64:
		ILITEK_SIMPLE_GESTURE(0x64);
		break;
	case 0x65:
		ILITEK_SIMPLE_GESTURE(0x65);
		break;
	case 0x66:
		ILITEK_SIMPLE_GESTURE(0x66);
		break;
	case 0x67:
		ILITEK_SIMPLE_GESTURE(0x67);
		break;
	case 0x68:
		ILITEK_SIMPLE_GESTURE(0x68);
		break;
#undef ILITEK_SIMPLE_GESTURE
	case 0x69:
		/* "52800d28 mov"@0xffffff8008a5d4b4 +
		 * "b90006e8 str"@0xffffff8008a5d4b8 */
		g->c4 = 0x69;
		/*
		 * "1a8883e8 csel"@0xffffff8008a5d4c4, condition `hi`:
		 * an UNSIGNED comparison with 1. `buf[34]` sits at sp+90
		 * ("39416be8 ldrb"@0xffffff8008a5d4bc) and the buffer starts at
		 * sp+56 ("9100e3e0 add"@0xffffff8008a5d26c): 90-56 = 34.
		 */
		g->c2 = (buf[34] > 1) ? 0 : buf[34];
		/* "53047d08 lsr"@0xffffff8008a5d4e8 +
		 * "33180d09 bfi"@0xffffff8008a5d4ec */
		xa = ((buf[28] >> 4) << 8) | buf[29];
		/* "33180d0a bfi"@0xffffff8008a5d4e4 */
		ya = ((buf[28] & 0x0f) << 8) | buf[30];
		/* "53047d68 lsr"@0xffffff8008a5d4f0 +
		 * "33180d0c bfi"@0xffffff8008a5d4f8 */
		xb = ((buf[31] >> 4) << 8) | buf[32];
		/* "33180d6d bfi"@0xffffff8008a5d4f4 */
		yb = ((buf[31] & 0x0f) << 8) | buf[33];
		/*
		 * four points built from two, with two averages:
		 * "0b0a01a8 add"@0xffffff8008a5d500 +
		 * "53017d08 lsr"@0xffffff8008a5d50c and
		 * "0b09018a add"@0xffffff8008a5d504 +
		 * "53017d49 lsr"@0xffffff8008a5d510
		 */
		g->pos[2].c2 = (xb + xa) >> 1;
		g->pos[2].c4 = ya;
		g->pos[3].c2 = xa;
		g->pos[3].c4 = (yb + ya) >> 1;
		g->pos[4].c2 = (xb + xa) >> 1;
		g->pos[4].c4 = yb;
		g->pos[5].c2 = xb;
		g->pos[5].c4 = (yb + ya) >> 1;
		break;
	case 0x6a:
		/* "52800d48 mov"@0xffffff8008a5d534, then the shared tail
		 * "b90006e8 str"@0xffffff8008a5d544 +
		 * "39000ae9 strb"@0xffffff8008a5d548 */
		g->c4 = 0x6a;
		g->c2 = 1;
		break;
	case 0x6b:
		/* "52800d68 mov"@0xffffff8008a5d53c */
		g->c4 = 0x6b;
		g->c2 = 1;
		break;
	case 0x6f:
		/* "52800de8 mov"@0xffffff8008a5d798 */
		g->c4 = 0x6f;
		g->c2 = 1;
		/* "33180d49 bfi"@0xffffff8008a5d7b8 and the three following */
		g->pos[2].c2 = ((buf[10] & 0xf0) << 4) | buf[11];
		g->pos[2].c4 = ((buf[10] & 0x0f) << 8) | buf[12];
		g->pos[3].c2 = ((buf[13] & 0xf0) << 4) | buf[14];
		g->pos[3].c4 = ((buf[13] & 0x0f) << 8) | buf[15];
		break;
	default:
		/* "\x013ILITEK: (%s, %d): Unknown gesture code\n"@0xffffff8009242c36,
		 * line 1031 */
#line 1031
		ILI_ERR("Unknown gesture code\n");
		break;
	}

	/*
	 * "394a0515 ldrb"@0xffffff8008a5d25c (idev+641), read ONCE at the
	 * top of the function and reused by every branch
	 * ("35001495 cbnz"@0xffffff8008a5d444 and its four twins)
	 */
	if (!transfer) {
		struct ilitek_tddi_dev *d = idev;

		for (i = 0; i < 6; i++) {
			/* "1b097d49 mul"@0xffffff8008a5d55c +
			 * "111ffd2a add"@0xffffff8008a5d560 +
			 * "1a89b149 csel"@0xffffff8008a5d568 +
			 * "530b7d29 lsr"@0xffffff8008a5d56c */
			g->pos[i].c2 = g->pos[i].c2 * d->c244 / 2048;
			g->pos[i].c4 = g->pos[i].c4 * d->c246 / 2048;
		}
	}

	/* "\x016ILITEK: (%s, %d): Transfer = %d, Type = %d, clockwise = %d\n"@0xffffff8009242c60,
	 * line 1050 */
	ILI_INFO("Transfer = %d, Type = %d, clockwise = %d\n",
#line 1050
		 transfer, g->c4, g->c2);

	/*
	 * "\x016ILITEK: (%s, %d): Gesture Points: (%d, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)\n"@0xffffff8009242c9e,
	 * line 1057. Twelve arguments: the first five in registers, the seven
	 * remaining on the stack ("b90003e8 str"@0xffffff8008a5d754 and the six
	 * that follow).
	 */
	ILI_INFO("Gesture Points: (%d, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)\n",
		 g->pos[0].c2, g->pos[0].c4, g->pos[1].c2, g->pos[1].c4,
		 g->pos[2].c2, g->pos[2].c4, g->pos[3].c2, g->pos[3].c4,
#line 1057
		 g->pos[4].c2, g->pos[4].c4, g->pos[5].c2, g->pos[5].c4);

	/* "97fffd56 bl"@0xffffff8008a5d764 */
	ilitek_tddi_touch_send_debug_data(data, len);
}

/*
 * Group D. "97ffeeeb bl"@0xffffff8008a5b1a0: w0 a 32-bit address, w1
 * a mask, w2 the value; the return value is tested on the sign bit alone
 * ("36f80120 tbz"@0xffffff8008a5b1a4).
 */
int ilitek_ice_mode_bit_mask_write(u32 addr, u32 mask, u32 value);
/*
 * Group G. "9400306e bl"@0xffffff8008a5b324: no arguments, return value
 * unused.
 */
int ilitek_tddi_flash_clear_dma(void);

/*
 * dma_clear_reg_setting() was reconstructed from the factory kernel disassembly (0xffffff8008a5b17c, 440 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void dma_clear_reg_setting(void)
{
	/* "97ffeeeb bl"@0xffffff8008a5b1a0 */
	if (ilitek_ice_mode_bit_mask_write(0x48080, 0x3000000, 0) < 0)
		/* line 112 */
#line 112
		ILI_ERR("Write %lu at %x failed\n", 0x3000000UL, 0x48080);

	/* "97ffeedd bl"@0xffffff8008a5b1d8 */
	if (ilitek_ice_mode_bit_mask_write(0x48008, 0x40000, 0x40000) < 0)
		/* line 116 */
#line 116
		ILI_ERR("Write %lu at %x failed\n", 0x40000UL, 0x48008);

	/* "97ffefa0 bl"@0xffffff8008a5b218, w2 = 4 */
	if (ilitek_ice_mode_write(0x720c4, 0x0, 4) < 0)
		/* "\x013ILITEK: (%s, %d): Write 0x00000000 at %x failed\n"@0xffffff8009242e9b,
		 * line 120 */
#line 120
		ILI_ERR("Write 0x00000000 at %x failed\n", 0x720c4);

	/* "97ffef93 bl"@0xffffff8008a5b24c, w2 = 1 */
	if (ilitek_ice_mode_write(0x720c8, 0x0, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Write 0x0 at %x failed\n"@0xffffff8009242ece,
		 * line 122 */
#line 122
		ILI_ERR("Write 0x0 at %x failed\n", 0x720c8);

	/* "97ffeeb2 bl"@0xffffff8008a5b284 */
	if (ilitek_ice_mode_bit_mask_write(0x720c8, 0x83000000, 0x80000000) < 0)
		/* line 124 */
#line 124
		ILI_ERR("Write %lu at %x failed\n", 0x83000000UL, 0x720c8);

	/* "97ffeea3 bl"@0xffffff8008a5b2c0 */
	if (ilitek_ice_mode_bit_mask_write(0x720c0, 0xf0000, 0) < 0)
		/* line 128 */
#line 128
		ILI_ERR("Write %lu at %x failed\n", 0xf0000UL, 0x720c0);

	/* "97ffee95 bl"@0xffffff8008a5b2f8 */
	if (ilitek_ice_mode_bit_mask_write(0x48004, 0x2000000, 0x2000000) < 0)
		/* line 130 */
#line 130
		ILI_ERR("Write %lu at %x failed\n", 0x2000000UL, 0x48004);

	/* "9400306e bl"@0xffffff8008a5b324 */
	ilitek_tddi_flash_clear_dma();
}

/*
 * Group G. "9400303d bl"@0xffffff8008a5b628: three 32-bit integers
 * ("0b140261 add"@0xffffff8008a5b61c computes the second as the sum of the
 * other two), return value unused.
 */
int ilitek_tddi_flash_dma_write(u32 inizio, u32 fine, u32 len);
/*
 * Group D. "97fff9ba bl"@0xffffff8008a5adec: two integers
 * ("52800640 mov"@0xffffff8008a5ade0 and "52800641 mov"@0xffffff8008a5ade4,
 * that is 50 and 50; the other site passes 300 and 50), return value
 * tested on the sign bit.
 */
int ilitek_tddi_ic_check_busy(int conta, int ritardo);

/*
 * dma_trigger_reg_setting() was reconstructed from the factory kernel disassembly (0xffffff8008a5b334, 1328 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void dma_trigger_reg_setting(u32 reg, u32 flash, u32 size)
{
	int i;
	u32 stato = 0;

	/* "97ffee71 bl"@0xffffff8008a5b388 */
	if (ilitek_ice_mode_bit_mask_write(0x720c0, 0x2000000, 0x2000000) < 0)
		/* line 143 */
#line 143
		ILI_ERR("Write %lu at %x failed\n", 0x2000000UL, 0x720c0);

	/* "97ffef32 bl"@0xffffff8008a5b3d0, datum 0x41010
	 * ("52820201 mov"@0xffffff8008a5b3b8 + "72a00081 movk"@0xffffff8008a5b3c0) */
	if (ilitek_ice_mode_write(0x720c4, 0x41010, 4) < 0)
		/* "\x013ILITEK: (%s, %d): Write 0x00041010 at %x failed\n"@0xffffff8009242f12,
		 * line 147 */
#line 147
		ILI_ERR("Write 0x00041010 at %x failed\n", 0x720c4);

	/* "97ffef25 bl"@0xffffff8008a5b404 */
	if (ilitek_ice_mode_write(0x720c8, 0x00, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Write 0x00 at %x failed\n"@0xffffff8009242f45,
		 * line 149 */
#line 149
		ILI_ERR("Write 0x00 at %x failed\n", 0x720c8);

	/* "97ffee46 bl"@0xffffff8008a5b434 */
	if (ilitek_ice_mode_bit_mask_write(0x720c8, 0x83000000, 0x80000000) < 0)
		/* line 151 */
#line 151
		ILI_ERR("Write %lu at %x failed\n", 0x83000000UL, 0x720c8);

	/* "97ffee38 bl"@0xffffff8008a5b46c */
	if (ilitek_ice_mode_bit_mask_write(0x720d0, 0x80000000, 0x00) < 0)
		/* line 155 */
#line 155
		ILI_ERR("Write %lu at %x failed\n", 0x80000000UL, 0x720d0);

	/* "97ffeefd bl"@0xffffff8008a5b4a4, w2 = 3 */
	if (ilitek_ice_mode_write(0x720d4, reg, 3) < 0)
		/* "\x013ILITEK: (%s, %d): Write %x at %x failed\n"@0xffffff8009242f72,
		 * line 159 */
#line 159
		ILI_ERR("Write %x at %x failed\n", reg, 0x720d4);

	/* "97ffeeef bl"@0xffffff8008a5b4dc */
	if (ilitek_ice_mode_write(0x720d8, 0x01, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Write 0x01 at %x failed\n"@0xffffff8009242f9d,
		 * line 161 */
#line 161
		ILI_ERR("Write 0x01 at %x failed\n", 0x720d8);

	/*
	 * FACTORY DEFECT 1: `write` instead of `bit_mask_write`.
	 * "97ffeee3 bl"@0xffffff8008a5b50c
	 */
	if (ilitek_ice_mode_write(0x720d8, 0x83000000, 0x80000000) < 0)
		/* line 163 */
#line 163
		ILI_ERR("Write %lu at %x failed\n", 0x83000000UL, 0x720d8);

	/* "97ffeed5 bl"@0xffffff8008a5b544 */
	if (ilitek_ice_mode_write(0x720dc, size, 4) < 0)
		/* line 167 */
#line 167
		ILI_ERR("Write %x at %x failed\n", size, 0x720dc);

	/* "97ffedf5 bl"@0xffffff8008a5b578 */
	if (ilitek_ice_mode_bit_mask_write(0x720dc, 0xf000000, 0x00) < 0)
		/* line 169 */
#line 169
		ILI_ERR("Write %lu at %x failed\n", 0xf000000UL, 0x720dc);

	/* "97ffede5 bl"@0xffffff8008a5b5b8, address derived from 0x48004
	 * ("11020116 add"@0xffffff8008a5b5a8, +0x80) */
	if (ilitek_ice_mode_bit_mask_write(0x48084, 0x20000, 0x20000) < 0)
		/* line 173 */
#line 173
		ILI_ERR("Write %lu at %x failed\n", 0x20000UL, 0x48084);

	/* "97ffedd7 bl"@0xffffff8008a5b5f0 */
	if (ilitek_ice_mode_bit_mask_write(0x720c0, 0xf0000, 0x10000) < 0)
		/* line 177 */
#line 177
		ILI_ERR("Write %lu at %x failed\n", 0xf0000UL, 0x720c0);

	/* "9400303d bl"@0xffffff8008a5b628 */
	ilitek_tddi_flash_dma_write(flash, flash + size, size);

	/* "97ffedc4 bl"@0xffffff8008a5b63c */
	if (ilitek_ice_mode_bit_mask_write(0x48004, 0x2020000, 0x2020000) < 0)
		/* line 184 */
#line 184
		ILI_ERR("Write %lu at %x failed\n", 0x2020000UL, 0x48004);

	/* "97ffedb5 bl"@0xffffff8008a5b678, address derived from 0x41010
	 * ("11000f36 add"@0xffffff8008a5b668, +3) */
	if (ilitek_ice_mode_bit_mask_write(0x41013, 0x01, 0x01) < 0)
		/* line 186 */
#line 186
		ILI_ERR("Write %lu at %x failed\n", 0x01UL, 0x41013);

	/* "97ffee7a bl"@0xffffff8008a5b6b0 */
	if (ilitek_ice_mode_write(0x41010, 0xff, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Trigger DMA failed\n"@0xffffff8009241e5f,
		 * line 190 */
#line 190
		ILI_ERR("Trigger DMA failed\n");

	/* the counter runs from -30 to 0: thirty rounds */
	for (i = 30; i > 0; i--) {
		/* "97ffedf8 bl"@0xffffff8008a5b700 */
		if (ilitek_ice_mode_read(0x48004, &stato, 4) < 0) {
			/* "\x013ILITEK: (%s, %d): Read 0x%x error\n"@0xffffff8009242fca,
			 * line 195 */
#line 195
			ILI_ERR("Read 0x%x error\n", 0x48004);
			continue;
		}

		/* "\x016ILITEK: (%s, %d): fw dma stat = %x\n"@0xffffff8009242fef,
		 * line 200, guard "340000c8 cbz"@0xffffff8008a5b70c */
#line 200
		ILI_DBG("fw dma stat = %x\n", stato);

		/* "37080308 tbnz"@0xffffff8008a5b728 -- the bit 17 */
		if (stato & 0x20000)
			goto alto;

		/* "94106a28 bl"@0xffffff8008a5b734, 1000 e 1000 */
		usleep_range(1000, 1000);
	}

	/*
	 * "\x013ILITEK: (%s, %d): DMA fail: Regsiter = 0x%x Flash = 0x%x, Size = %d\n"@0xffffff8009243015,
	 * line 211. `Regsiter` is the factory's spelling.
	 */
	ILI_ERR("DMA fail: Regsiter = 0x%x Flash = 0x%x, Size = %d\n",
#line 211
		reg, flash, size);

alto:
	/* "51004320 sub"@0xffffff8008a5b788, that is 0x41010 - 0x10 = 0x41000 */
	if (ilitek_ice_mode_write(0x41000, 0x01, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Pull CS High failed\n"@0xffffff8009241e87,
		 * line 215 */
#line 215
		ILI_ERR("Pull CS High failed\n");

	/* ten `__const_udelay(0x418958)`, from
	 * "940ff0ae bl"@0xffffff8008a5b7bc a "940ff093 bl"@0xffffff8008a5b828 */
	mdelay(10);
}

/*
 * ilitek_tddi_move_mp_code_flash() was reconstructed from the factory kernel disassembly (0xffffff8008a5ac64, 1304 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_touch.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_move_mp_code_flash(void)
{
	int ret = 0;
	u8 buf[16] = {0};
	u32 flash_addr, mp_size, addr_inizio, addr_fine;
	int overlay;

	/* "790013e9 strh"@0xffffff8008a5aca0 -- 0x1f0, that is 0xf0 and 0x01 */
	buf[0] = 0xf0;
	buf[1] = 0x01;
	/* "d63f0100 blr"@0xffffff8008a5acac, w1 = 2 */
	ret = idev->c776(buf, 2);
	/* "37f82480 tbnz"@0xffffff8008a5acb0 */
	if (ret < 0)
		goto fine;

	/* "390023e9 strb"@0xffffff8008a5acbc */
	buf[0] = 0xfe;
	/* "d63f0100 blr"@0xffffff8008a5accc, w1 = 1 */
	ret = idev->c776(buf, 1);
	if (ret < 0)
		goto fine;

	/* "a900ffff stp"@0xffffff8008a5acd8 */
	memset(buf, 0x0, sizeof(buf));

	/*
	 * "f9401d08 ldr"@0xffffff8008a5ace4 (idev+56) +
	 * "b9402903 ldr"@0xffffff8008a5acf4 (+40 inside the object pointed to).
	 * `ilitek.h` declares `idev->c56` as `void *`: a HEADER DELTA, as for
	 * `c48`. Line 241,
	 * "\x016ILITEK: (%s, %d): read mp info length = %d\n"@0xffffff8009241cfd
	 */
	ILI_INFO("read mp info length = %d\n",
#line 241
		 *(int *)((u8 *)idev->c56 + 40));

	/* "f9418908 ldr"@0xffffff8008a5ad08 (idev+784) +
	 * "d63f0100 blr"@0xffffff8008a5ad10 */
	ret = idev->c784(buf, *(int *)((u8 *)idev->c56 + 40));
	if (ret < 0)
		goto fine;

	/* "97ffff68 bl"@0xffffff8008a5ad38, con "MP overlay info"@0xffffff8009241d4a
	 * in x4, 8 in w1 e 0 in w3 */
	ilitek_dump_data(buf, 8, *(int *)((u8 *)idev->c56 + 40), 0,
			 "MP overlay info");

	/* "33181d13 bfi"@0xffffff8008a5ad58 + "33101d33 bfi"@0xffffff8008a5ad78 */
	flash_addr = buf[3] | (buf[2] << 8) | (buf[1] << 16);
	/* "33181d36 bfi"@0xffffff8008a5ad60 + "33101d56 bfi"@0xffffff8008a5ad80 */
	mp_size = buf[6] | (buf[5] << 8) | (buf[4] << 16);
	/* "33181d54 bfi"@0xffffff8008a5ad68 + "33101d14 bfi"@0xffffff8008a5ad88 */
	addr_inizio = buf[9] | (buf[8] << 8) | (buf[7] << 16);
	/* "33181d15 bfi"@0xffffff8008a5ad70 + "33101d35 bfi"@0xffffff8008a5ad8c */
	addr_fine = buf[12] | (buf[11] << 8) | (buf[10] << 16);

	/* "0a080138 and"@0xffffff8008a5adac */
	overlay = (addr_inizio != 0) && (addr_fine != 0);
	overlay = (buf[0] == 0xfe) && overlay;

	/* "\x016ILITEK: (%s, %d): MP info Overlay: Enable = %d, addr = 0x%x ~ 0x%x, flash addr = 0x%x, mp size = 0x%x\n"@0xffffff8009241d5a,
	 * line 261 */
	ILI_INFO("MP info Overlay: Enable = %d, addr = 0x%x ~ 0x%x, flash addr = 0x%x, mp size = 0x%x\n",
#line 261
		 overlay, addr_inizio, addr_fine, flash_addr, mp_size);

	/* "b901411f str"@0xffffff8008a5ade8 */
	idev->c320 = 0;
	/* "97fff9ba bl"@0xffffff8008a5adec */
	ret = ilitek_tddi_ic_check_busy(50, 50);
	/* "37f81a80 tbnz"@0xffffff8008a5adf0 */
	if (ret < 0)
		goto fine;

	/* "97fff0eb bl"@0xffffff8008a5adfc */
	ret = ilitek_ice_mode_ctrl(1, 0);
	if (ret < 0)
		goto fine;

	/* "34000458 cbz"@0xffffff8008a5ae04 */
	if (overlay) {
		/* "4b1502c8 sub"@0xffffff8008a5ae08 +
		 * "11000516 add"@0xffffff8008a5ae10 */
		mp_size = mp_size - addr_fine + 1;
		/* "\x016ILITEK: (%s, %d): MP andes init size = %d , MP text size = %d\n"@0xffffff8009241dc3,
		 * line 276 */
		ILI_INFO("MP andes init size = %d , MP text size = %d\n",
#line 276
			 addr_inizio, mp_size);

		/* "940000d2 bl"@0xffffff8008a5ae34 */
		dma_clear_reg_setting();
		/* "\x016ILITEK: (%s, %d): [Move ANDES.INIT to DRAM]\n"@0xffffff8009241e04,
		 * line 280 */
#line 280
		ILI_INFO("[Move ANDES.INIT to DRAM]\n");
		/* "94000137 bl"@0xffffff8008a5ae58 */
		dma_trigger_reg_setting(0, flash_addr, addr_inizio);
		/* "940000c8 bl"@0xffffff8008a5ae5c */
		dma_clear_reg_setting();

		/* "\x016ILITEK: (%s, %d): [Move MP.TEXT to DRAM]\n"@0xffffff8009241e33,
		 * line 285 */
#line 285
		ILI_INFO("[Move MP.TEXT to DRAM]\n");
		/* "0b130281 add"@0xffffff8008a5ae74 +
		 * "9400012d bl"@0xffffff8008a5ae80 */
		dma_trigger_reg_setting(addr_fine, addr_inizio + flash_addr,
					mp_size);
		/* "940000be bl"@0xffffff8008a5ae84 */
		dma_clear_reg_setting();
	} else {
		/* "97fff07e bl"@0xffffff8008a5aea0, address derived from
		 * 0x41000 ("11004100 add"@0xffffff8008a5ae94, +0x10) */
		if (ilitek_ice_mode_write(0x41010, 0xff, 1) < 0)
			/* line 292 */
#line 292
			ILI_ERR("Trigger DMA failed\n");
		/* thirty `__const_udelay` */
		mdelay(30);
		/* "97fff018 bl"@0xffffff8008a5b038 */
		if (ilitek_ice_mode_write(0x41000, 0x01, 1) < 0)
			/* line 298 */
			ILI_ERR("Pull CS High failed\n");
		/* ten `__const_udelay` */
		mdelay(10);
	}

	/* "97ffe29d bl"@0xffffff8008a5b0d4, w0 = 1 */
	if (ilitek_tddi_reset_ctrl(1) < 0)
		/* "\x013ILITEK: (%s, %d): IC Code reset failed during moving mp code\n"@0xffffff8009241eb0,
		 * line 304 */
#line 304
		ILI_ERR("IC Code reset failed during moving mp code\n");

	/* "97fff02b bl"@0xffffff8008a5b0fc */
	ret = ilitek_ice_mode_ctrl(0, 0);
	if (ret < 0)
		goto fine;

	/* "b9014109 str"@0xffffff8008a5b114 -- idev+320 = 1 */
	idev->c320 = 1;
	/* "97fff8ef bl"@0xffffff8008a5b118, 300 e 50 */
	ret = ilitek_tddi_ic_check_busy(300, 50);
	/* "36f80120 tbz"@0xffffff8008a5b120 */
	if (ret < 0)
		/* "\x013ILITEK: (%s, %d): Check cdc timeout failed after moved mp code\n"@0xffffff8009241ef0,
		 * line 314 */
#line 314
		ILI_ERR("Check cdc timeout failed after moved mp code\n");

fine:
	/* "2a1703e0 mov"@0xffffff8008a5b158 */
	return ret;
}
