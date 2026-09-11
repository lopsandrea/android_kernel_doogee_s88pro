// SPDX-License-Identifier: GPL-2.0
/*
 * Goodix GT917S touch panel, Doogee S88 Pro (MediaTek MT6771) -- the declared
 * sub-batch of the gt1x_* block.
 *
 * Four functions out of 87, 620 bytes out of 38000: the part of the block
 * that cannot be obtained by compiling ALPS, plus the one function no project
 * tool could see. The rest of the block is a measurement, not a rewrite.
 *
 * Reconstructed from the disassembly of the factory kernel.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_wtk.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/err.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/types.h>

/*
 * GTP_INFO() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_wtk.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define GTP_INFO(fmt, arg...) \
	printk("<<GTP-INF>>[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#define GTP_DEBUG(fmt, arg...) \
	printk("<<GTP-DBG>>[%s:%d]" fmt "\n", __func__, __LINE__, ##arg)
/*
 * GTP_ERROR: the same shape as GTP_INFO with the ERR tag. Proven by the bytes of
 * the `dump_to_file` message, which carries the space after the square bracket
 * as GTP_INFO does and unlike GTP_DEBUG:
 *   "<<GTP-ERR>>[%s:%d] can not open file: %s\n\n"@0xffffff800924d8ca
 */
#define GTP_ERROR(fmt, arg...) \
	printk("<<GTP-ERR>>[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)

/*
 * ===========================================================================
 * WHAT LIVES IN OTHER UNITS -- DECLARED, NOT DEFINED
 * ===========================================================================
 * No stubs. If the link fails that is the honest outcome: it says the unit that
 * defines the symbol has not been written yet. A stub would make it look
 * written.
 */

/*
 * From `drivers/input/touchscreen/mediatek/tpd.h`: the structure
 * `tpd_driver_add`/`tpd_driver_remove` take. Only the pointer is needed here,
 * so the type stays incomplete.
 */
struct tpd_driver_t;
extern int tpd_driver_remove(struct tpd_driver_t *tpd_drv);

/*
 * The Goodix instance: 0xffffff800998a578, read from the survey (§6.1)
 * and confirmed here by the argument of the deregistration,
 * "d0002ee0 adrp"@0xffffff80093acb18 + "9115e000 add"@0xffffff80093acb1c
 * (page 0xffffff800998a000 + 0x578). It is defined in `gt1x_tpd.c`.
 */
extern struct tpd_driver_t gt1x_tpd_driver;

/*
 * `gesture_enabled` -- 0xffffff800998a2d0. NAME FROM ALPS (G3).
 * What the binary measures: FULL-WIDTH access on the write,
 * "b902d109 str"@0xffffff8008a72a50 (`str w9,[x8,#720]`), so it is NOT
 * a `static` with few values narrowed to `strb` by the compiler (class
 * A1). ALPS declares it `int gesture_enabled;` without `static`, and the
 * measurement does not contradict that.
 */
extern int gesture_enabled;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a10087d, 32 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_wtk.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
extern u8 gestures_flag[32];

/*
 * `gesture_doze_status` -- 0xffffff800a1006f0. NAME FROM ALPS (G3).
 * What the binary measures: a full-width write of zero,
 * "b906f15f str"@0xffffff8008a72a78 (`str wzr,[x10,#1776]`), and ONLY on the
 * power-down branch. The value 0 is `DOZE_DISABLED` in ALPS; here the type
 * stays `int` because the binary measures the width, not the enum.
 */
extern int gesture_doze_status;

/*
 * gt1x_gesture_debug() was reconstructed from the factory kernel disassembly (0xffffff8008a72a38, 132 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_wtk.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void gt1x_gesture_debug(int on)
{
	if (on) {
		gesture_enabled = 1;
		memset(gestures_flag, 0xff, sizeof(gestures_flag));
	} else {
		gesture_enabled = 0;
		memset(gestures_flag, 0x00, sizeof(gestures_flag));
		gesture_doze_status = 0;
	}
#line 431
	GTP_DEBUG("Gesture debug %s", on ? "on" : "off");
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998a468, 40 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_wtk.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static char d998a468[6][40] = {
	"xinpengda", "none_1", "hongzhan", "none_3", "none_4", "none_5",
};

/*
 * get_module_name() was reconstructed from the factory kernel disassembly (0xffffff8008a76ca8, 76 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_wtk.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void get_module_name(int index, char *name)
{
	if (d998a468[index])
		strcpy(name, d998a468[index]);
	else
		strcpy(name, "unknow");
}

/*
 * gt1x_i2c_read() was reconstructed from the factory kernel disassembly (0xffffff8008a76cf4).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_wtk.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
extern s32 gt1x_i2c_read(u16 addr, u8 *buffer, s32 len);

/*
 * dump_to_file() was reconstructed from the factory kernel disassembly (0xffffff8008a7aed8, 360 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_wtk.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void dump_to_file(u16 addr, s32 len, char *filename)
{
	struct file *flp = NULL;
	u8 buf[128];
	s32 ret = 0;
	s32 cnt = 0;
	s32 pos = 0;

#line 1331
	GTP_INFO("Dump(0x%04X, %d bytes) to file: %s\n", addr, len, filename);

	flp = filp_open(filename, O_CREAT | O_RDWR, 0666);
	if (IS_ERR(flp)) {
#line 1334
		GTP_ERROR("can not open file: %s\n", filename);
		return;
	}

	flp->f_op->llseek(flp, 0, SEEK_SET);

	while (len > 0) {
		cnt = (len < 128) ? len : 128;
		memset(buf, 0x33, cnt);
		ret = gt1x_i2c_read(addr + pos, buf, cnt);
		if (ret)
			memset(buf, 0x33, cnt);
		flp->f_op->write(flp, buf, cnt, &flp->f_pos);
		len -= cnt;
		pos += cnt;
	}

	filp_close(flp, NULL);
}

/*
 * tpd_driver_exit() was reconstructed from the factory kernel disassembly (0xffffff80093acaf8, 52 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_wtk.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void __exit tpd_driver_exit(void)
{
	GTP_INFO("MediaTek GT1x touch panel driver exit.");
	tpd_driver_remove(&gt1x_tpd_driver);
}

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_wtk.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
