// SPDX-License-Identifier: GPL-2.0
/*
 * FocalTech FT8719 touch panel, Doogee S88 Pro (MT6771) -- the "modes"
 * translation unit, and that alone. Declared partial batch.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read; the two FocalTech drivers already in the ALPS tree were not
 * opened.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_ex_mode.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/sysfs.h>

/*
 * The two macros this unit uses, read from the whole formats:
 *   "\x013[FTS][Error][Mode]create sysfs failed.\n"@0xffffff800924fef8  (KERN_ERR)
 *   "[FTS][Mode]create sysfs succeeded\n"@0xffffff800924ff22            (no level)
 * The SOH+'3' of the first and the '[' of the second prove they are two
 * different macros; "[Mode]" is part of the message and not of the prefix, because
 * it appears AFTER "[FTS][Error]".
 */
#define FTS_DEBUG(fmt, args...)		printk("[FTS]" fmt "\n", ##args)
#define FTS_ERROR(fmt, args...)		printk(KERN_ERR "[FTS][Error]" fmt "\n", ##args)

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998cbb8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_ex_mode.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct attribute *fts_g_a100b00[] = {
	NULL,
};

static struct attribute_group fts_g_998cbb8 = {
	.attrs = fts_g_a100b00,
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a100af0, 8 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_ex_mode.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct {
	int c0;
	int c4;
	int c8;
} fts_g_a100af0;

/*
 * fts_ex_mode_init() was reconstructed from the factory kernel disassembly (0xffffff8008a8020c, 116 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_ex_mode.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_ex_mode_init(struct i2c_client *client)
{
	int ret = 0;

	fts_g_a100af0.c0 = 0;
	fts_g_a100af0.c4 = 0;
	fts_g_a100af0.c8 = 0;

	ret = sysfs_create_group(&client->dev.kobj, &fts_g_998cbb8);
	if (ret != 0) {
		FTS_ERROR("[Mode]create sysfs failed.");
		sysfs_remove_group(&client->dev.kobj, &fts_g_998cbb8);
		return -EIO;
	}

	FTS_DEBUG("[Mode]create sysfs succeeded");

	return 0;
}

/*
 * ===========================================================================
 * fts_ex_mode_exit @ 0xffffff8008a80280, 36 bytes
 * ===========================================================================
 *   "9100c000 add"@0xffffff8008a8028c   x0 = client + 0x30
 *   "912ee021 add"@0xffffff8008a80290   x1 = 0x998c000 + 0xbb8, the same group
 *   "97e20f17 bl"@0xffffff8008a80294    sysfs_remove_group
 *   "2a1f03e0 mov"@0xffffff8008a80298   w0 = 0
 * The return value of sysfs_remove_group (which is `void`) is beside the point: the
 * `mov w0, wzr` AFTER the `bl` is an explicit `return 0`.
 */
int fts_ex_mode_exit(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &fts_g_998cbb8);

	return 0;
}

/*
 * fts_ex_mode_recovery() was reconstructed from the factory kernel disassembly (0xffffff8008a802a4, 8 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_ex_mode.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_ex_mode_recovery(struct i2c_client *client)
{
	return 0;
}

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_ex_mode.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
