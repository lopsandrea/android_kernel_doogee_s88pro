// SPDX-License-Identifier: GPL-2.0
/*
 * Ilitek TDDI touch panel, Doogee S88 Pro -- group G, firmware and flash.
 *
 * Thirteen map symbols, [0xffffff8008a6714c, 0xffffff8008a6b528), 17372
 * bytes; the boundary was measured twice, independently.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fcntl.h>
#include <linux/firmware.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#include "ilitek.h"

/*
 * ilitek_ice_mode_write() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_ice_mode_write(u32 addr, u32 data, int len);
int ilitek_ice_mode_read(u32 addr, u32 *data, int len);
int ilitek_ice_mode_bit_mask_write(u32 addr, u32 mask, u32 value);
int ilitek_ice_mode_ctrl(bool enable, bool mcu);

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a67b98, 56 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_g_c48 {
	u8 __ignoto_0[48];
	/*
	 * +48 32-bit unsigned. The current firmware version:
	 * "b9403104 ldr"@0xffffff8008a69ad8 prints it next to the new one
	 * and "b9403108 ldr"@0xffffff8008a69af0 compares it, in the format
	 * "\x016ILITEK: (%s, %d): New FW ver = 0x%x, Current FW ver = 0x%x\n"@0xffffff8009246fae.
	 * ADDED BY THE SECOND BATCH: before that, no written function read it.
	 */
	u32 c48;
	/*
	 * +52 32-bit unsigned. Compared with 0x010402
	 * ("b9403529 ldr"@0xffffff8008a69864 + "6b0a013f cmp"@0xffffff8008a69868
	 * + "540003c9 b.ls"@0xffffff8008a6986c) to decide `idev->c641`.
	 * ADDED BY THE SECOND BATCH.
	 */
	u32 c52;
	/*
	 * +56 32-bit unsigned. The maximum length that
	 * `ilitek_tddi_fw_read_hw_crc` accepts; the format at
	 * 0xffffff8009245e56 calls it "max count".
	 */
	u32 c56;
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009989480, 16 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_flash_tab {
	/* +0  16-bit. "Flash MID" (name FROM THE BINARY, see above). */
	u16 c0;
	/* +2  16-bit. "Flash DEV_ID" (name FROM THE BINARY). */
	u16 c2;
	/*
	 * +4  32-bit. NEVER READ by group G: no instruction in the block
	 * uses this offset. The value is the one in the bytes, and what it
	 * means is not decided here.
	 */
	u32 c4;
	/*
	 * +8  32-bit. Copied into `idev->c548` ("b9022549 str"@0xffffff8008a6adf0),
	 * which the format at 0xffffff800924669d calls "Flash program page".
	 */
	u32 c8;
	/*
	 * +12 32-bit. Copied into `idev->c552` ("b9022948 str"@0xffffff8008a6ae08),
	 * which the format at 0xffffff80092466ca calls "Flash sector".
	 */
	u32 c12;
};

/*
 * The values are the bytes read at 0xffffff8009989480 (16 per row, as in
 * the survey dump, §5.1). Entry 0 has MID and DEV_ID at zero and is the
 * FALLBACK: when nothing matches, the code prints "Not found flash id in
 * tab, use default" and rejoins the common tail by reading entry 0
 * ("d00078e8 adrp"@0xffffff8008a6b094 + "91120108 add"@0xffffff8008a6b098).
 */
/*
 * NAME CHOSEN: the binary names neither the table nor its type. What it
 * does name is two of the five fields (see above).
 */
struct ilitek_flash_tab ilitek_flashtab[8] = {
	{ 0x0000, 0x0000, 0x00040000, 0x100, 0x1000 },
	{ 0x00ef, 0x6011, 0x00020000, 0x100, 0x1000 },
	{ 0x00ef, 0x6012, 0x00040000, 0x100, 0x1000 },
	{ 0x00c8, 0x6012, 0x00040000, 0x100, 0x1000 },
	{ 0x00c8, 0x6013, 0x00080000, 0x100, 0x1000 },
	{ 0x0085, 0x6013, 0x00400000, 0x100, 0x1000 },
	{ 0x00c2, 0x2812, 0x00040000, 0x100, 0x1000 },
	{ 0x001c, 0x3812, 0x00040000, 0x100, 0x1000 },
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0ff5b8, 32 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_fw_blk {
	/*
	 * +0  8 bytes, pointer to a constant string. Written only by
	 * `ilitek_tddi_fw_update_block_info`, which is not in this batch.
	 */
	const char *c0;
	/*
	 * +8  32-bit. Start address of the block: it is the lower bound
	 * of the `ilitek_tddi_fw_flash_erase` loop
	 * ("b8408f04 ldr"@0xffffff8008a6a7b4) and the first CRC argument in
	 * `ilitek_fw_calc_file_crc` ("b9400949 ldr"@0xffffff8008a6b100).
	 */
	u32 c8;
	/* +12 32 bit. Upper bound. The GUARD of both loops is
	 * `c12 == 0` ("34000ec5 cbz"@0xffffff8008a6a7b0,
	 * "34000868 cbz"@0xffffff8008a6b0fc). */
	u32 c12;
	/* +16 32 bit. Length: `ilitek_fw_calc_file_crc` computes the CRC su
	 * c8..c8+c16-4 ("b940114a ldr"@0xffffff8008a6b104,
	 * "5100116b sub"@0xffffff8008a6b10c). */
	u32 c16;
	/* +20 32 bit. NEVER touched by group G. */
	u32 c20;
	/* +24 32 bit. Written only by `ilitek_tddi_fw_upgrade`. */
	u32 c24;
	/* +28 one byte. Written only by `ilitek_tddi_fw_upgrade`. */
	u8 c28;
	/*
	 * +29..31 NEVER touched by group G. They are padding up to 32, which is
	 * the measured stride.
	 */
	u8 __ignoto_29[3];
};

/*
 * NAME CHOSEN: the binary names neither the array nor its type. It does
 * name the byte sitting right after it, "Block Num": the format at
 * 0xffffff8009246d95 -- which belongs to `ilitek_tddi_fw_upgrade` and so is
 * NOT citable from this file, because no literal of this file is its tail --
 * names it together with "star_addr" (spelt exactly so in the binary,
 * without the `t`) and "end_addr". It is that byte which fixes the end of
 * the array at 224 bytes.
 */
static struct ilitek_fw_blk ilitek_fw_blk[7];

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0ff5a8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * WHAT THIS FILE DOES NOT DECLARE, AND WHY.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
/* "/sdcard/iram_dump"@0xffffff8009245b8a */
#define ILI_DUMP_IRAM_PATH	"/sdcard/iram_dump"
/* "/sdcard/flash_dump"@0xffffff8009246288 */
#define ILI_DUMP_FLASH_PATH	"/sdcard/flash_dump"

/*
 * ilitek_tddi_fw_iram_read() was reconstructed from the factory kernel disassembly (0xffffff8009246737).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tddi_fw_iram_read(u8 *buf, int start, int end, int len)
{
	int i, addr = 0, r_len = 0x1000;
	/*
	 * "b90017ff str"@0xffffff8008a6729c -- cleared together, four
	 * bytes in one `str wzr`: it is the initialiser, and it is OUTSIDE the loop
	 */
	u8 cmd[4] = {0};

	for (i = start; i < end; i += r_len, addr += r_len) {
		/* "0b150288 add"@0xffffff8008a672b8 + "6b19011f cmp"@0xffffff8008a672bc */
		if ((i + r_len) > len)
			/* "1b15e915 msub"@0xffffff8008a672c8 */
			r_len = end % r_len;

		/* "528004bc mov"@0xffffff8008a672ac -- 0x25 is the command, and sits
		 * in cmd[0]: "390053fc strb"@0xffffff8008a672dc writes at [sp,#20] */
		cmd[0] = 0x25;
		/* "53107e88 lsr"@0xffffff8008a672d0 + "39005fe8 strb"@0xffffff8008a672d4 */
		cmd[3] = (i & 0x00FF0000) >> 16;
		/* "53087e88 lsr"@0xffffff8008a672d8 + "39005be8 strb"@0xffffff8008a672e0 */
		cmd[2] = (i & 0x0000FF00) >> 8;
		/* "390057f4 strb"@0xffffff8008a672e4 */
		cmd[1] = (i & 0x000000FF);

		/*
		 * "f9418528 ldr"@0xffffff8008a672e8 = idev->c776, called with
		 * "d63f0100 blr"@0xffffff8008a672f4; the error branch is
		 * "37f80a20 tbnz"@0xffffff8008a672f8, that is the sign bit alone
		 */
		if (idev->c776(cmd, 4) < 0) {
			/* factory line 136;
			 * "\x013ILITEK: (%s, %d): Failed to write iram data\n"@0xffffff8009246750 */
#line 136
			ILI_ERR("Failed to write iram data\n");
			return -ENODEV;
		}

		/*
		 * "8b3bc260 add"@0xffffff8008a67300 -- buf + addr, index SIGN-EXTENDED
		 * (`sxtw`), which is the second proof that `addr` is an `int`;
		 * "f9418908 ldr"@0xffffff8008a67308 = idev->c784
		 */
		if (idev->c784(buf + addr, r_len) < 0) {
			/* factory line 141;
			 * "\x013ILITEK: (%s, %d): Failed to Read iram data\n"@0xffffff800924677f */
#line 141
			ILI_ERR("Failed to Read iram data\n");
			return -ENODEV;
		}

		/* "1b167f69 mul"@0xffffff8008a67318 (per 100) +
		 * "1ada0d23 sdiv"@0xffffff8008a6731c +
		 * "b901bd03 str"@0xffffff8008a67320 */
		idev->c444 = (addr * 100) / end;
		/*
		 * factory line 145. Guarded by
		 * "396562e8 ldrb"@0xffffff8008a67324 + "34000108 cbz"@0xffffff8008a67328,
		 * hence ILI_DBG and not ILI_INFO. The trailing '%' is an ARGUMENT,
		 * "528004a4 mov"@0xffffff8008a67338 = 0x25 in w4, and the format
		 * @0xffffff80092467ad ends with "%d%c" WITHOUT a \n.
		 */
		/* "\x016ILITEK: (%s, %d): Reading iram data .... %d%c"@0xffffff80092467ad */
#line 145
		ILI_DBG("Reading iram data .... %d%c", idev->c444, '%');
	}

	return 0;
}

/*
 * ilitek_fw_dump_iram_data() was reconstructed from the factory kernel disassembly (0xffffff8008a6714c, 912 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_fw_dump_iram_data(u32 start, u32 end, bool mcu)
{
	struct file *f = NULL;
	u8 *fw_buf = NULL;
	int ret = 0, len, stato;
	loff_t pos = 0;
	mm_segment_t old_fs;

	/*
	 * "b901bd1f str"@0xffffff8008a67194 -- cleared BEFORE entering
	 * ICE mode
	 */
	idev->c444 = 0;

	ret = ilitek_ice_mode_ctrl(true, false);
	if (ret < 0) {
		/* factory line 161;
		 * "\x013ILITEK: (%s, %d): Enable ice mode failed\n"@0xffffff8009245adc */
#line 161
		ILI_ERR("Enable ice mode failed\n");
		goto out;
	}

	/* "4b14027a sub"@0xffffff8008a671a0 + "11000759 add"@0xffffff8008a671a4 */
	len = end - start + 1;
	/*
	 * "7140a33f cmp"@0xffffff8008a671a8 -- 0x28 << 12 = 0x28000, and the
	 * comparison is SIGNED ("5400058d b.le"@0xffffff8008a671ac)
	 */
	if (len > 0x28000) {
		/* factory line 169;
		 * "\x013ILITEK: (%s, %d): len is larger than buffer, abort\n"@0xffffff8009245b21 */
#line 169
		ILI_ERR("len is larger than buffer, abort\n");
		ret = -ENOMEM;
		goto out;
	}

	/*
	 * kzalloc() was reconstructed from the factory kernel disassembly (0xffffff8008a6725c).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	fw_buf = kzalloc(0x28000, GFP_KERNEL);
	/*
	 * "b140041f cmn"@0xffffff8008a67278 then "b4000c13 cbz"@0xffffff8008a67280:
	 * IS_ERR BEFORE the comparison with NULL, in that order
	 */
	if (IS_ERR(fw_buf) || fw_buf == NULL) {
		/* factory line 176;
		 * "\x013ILITEK: (%s, %d): Failed to allocate update_buf\n"@0xffffff8009245b57 */
#line 176
		ILI_ERR("Failed to allocate update_buf\n");
		ret = -ENOMEM;
		goto out;
	}

	/* "32001fe1 orr"@0xffffff8008a67288 -- 0xff, not 0 */
	memset(fw_buf, 0xFF, 0x28000);

	ret = ilitek_tddi_fw_iram_read(fw_buf, start, end - start, len);
	if (ret < 0)
		goto out;

	/*
	 * "52804821 mov"@0xffffff8008a67360 = 0x241 = O_WRONLY|O_CREAT|O_TRUNC
	 * and "52805082 mov"@0xffffff8008a67364 = 0x284 = 644 DECIMAL. The mode
	 * is written without the leading zero in the factory source: 644 decimal
	 * is 01204 octal, not 0644. It is a factory defect and is reproduced
	 * (rule 7).
	 */
	f = filp_open(ILI_DUMP_IRAM_PATH, O_WRONLY | O_CREAT | O_TRUNC, 644);
	if (IS_ERR(f) || f == NULL) {
		/*
		 * factory line 190. "%ld" with PTR_ERR: the argument is there,
		 * "aa1403e3 mov"@0xffffff8008a67430 puts x20 (the pointer) in
		 * x3, and it is a 64-bit value.
		 */
		/* "\x013ILITEK: (%s, %d): Failed to open the file at %ld.\n"@0xffffff8009245b9c */
#line 190
		ILI_ERR("Failed to open the file at %ld.\n", PTR_ERR(f));
		ret = -ENOENT;
		goto out;
	}

	/*
	 * The TWO `set_fs` calls in a row are in the binary, not a typo:
	 * "f90006b6 str"@0xffffff8008a67388 and "f90006b6 str"@0xffffff8008a673a4
	 * write the same -1 into `current_thread_info()->addr_limit`, each
	 * followed by its own `set_bit(TIF_FSCHECK)`
	 * ("940fbed4 bl"@0xffffff8008a6739c, "940fbecd bl"@0xffffff8008a673b8).
	 * KERNEL_DS and get_ds() are the same thing on arm64, and the constant
	 * -1 is materialised ONCE ("92800016 mov"@0xffffff8008a6737c).
	 */
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	set_fs(get_ds());
	pos = 0;
	/*
	 * "93407f22 sxtw"@0xffffff8008a673bc -- the length is SIGN-extended,
	 * a third proof that `len` is an `int`. The return value is unused.
	 */
	vfs_write(f, fw_buf, len, &pos);
	set_fs(old_fs);

	filp_close(f, NULL);
	/*
	 * factory line 202. NOT guarded by `ilitek_dbg_en`, and the level
	 * of the format @0xffffff8009245bd1 is \x016 = KERN_INFO.
	 */
	/* "\x016ILITEK: (%s, %d): Save iram data to %s\n"@0xffffff8009245bd1 */
#line 202
	ILI_INFO("Save iram data to %s\n", ILI_DUMP_IRAM_PATH);

	/*
	 * ilitek_ice_mode_ctrl() was reconstructed from the factory kernel disassembly (0xffffff8008a671ec).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	ilitek_ice_mode_ctrl(false, false);
	/* factory line 206. "SUCCESS"@0xffffff80090f6e9d;
	 * "\x016ILITEK: (%s, %d): Dump IRAM %s\n"@0xffffff8009245bfb
	 * "SUCCESS"@0xffffff80090f6e9d */
	ILI_INFO("Dump IRAM %s\n", "SUCCESS");
	/*
	 * "52800c88 mov"@0xffffff8008a674d0 then "17ffff50 b"@0xffffff8008a674d4:
	 * the constant 100 is the ONLY thing this branch computes before
	 * jumping into the shared tail.
	 */
	stato = 100;
	goto fine;

out:
	ilitek_ice_mode_ctrl(false, false);
	/* factory line 206. "FAIL"@0xffffff8009249687;
	 * "\x016ILITEK: (%s, %d): Dump IRAM %s\n"@0xffffff8009245bfb
	 * "FAIL"@0xffffff8009249687 */
	ILI_INFO("Dump IRAM %s\n", "FAIL");
	/* "12800008 mov"@0xffffff8008a67210 */
	stato = -1;

fine:
	/*
	 * "f9452309 ldr"@0xffffff8008a67214 + "b901bd28 str"@0xffffff8008a67218:
	 * the read of `idev` and the write of +444 are a SINGLE one in the
	 * binary, shared by the two branches.
	 */
	idev->c444 = stato;
	/*
	 * On the branch where `kzalloc` failed the pointer is NOT cleared:
	 * "17ffff73 b"@0xffffff8008a67418 jumps PAST the
	 * "aa1f03f3 mov"@0xffffff8008a671e0 that zeroes x19. The factory would
	 * therefore pass an ERR_PTR to `kfree`. It is reproduced as it stands
	 * (rule 7): in practice `kzalloc` never returns an ERR_PTR, so the
	 * branch is dead, but the code is there.
	 */
	if (fw_buf != NULL)
		kfree(fw_buf);
}

/*
 * ilitek_tddi_flash_poll_busy() was reconstructed from the factory kernel disassembly (0xffffff8008a6b350, 472 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tddi_flash_poll_busy(int timer)
{
	/* "b90007ff str"@0xffffff8008a6b390 -- cleared before the loop */
	u32 tmp = 0;

	/* "52820000 mov"@0xffffff8008a6b374 + "72a00080 movk"@0xffffff8008a6b37c
	 * = 0x41000 */
	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* line 244;
		 * "\x013ILITEK: (%s, %d): Pull cs low failed\n"@0xffffff800924114c */
#line 244
		ILI_ERR("Pull cs low failed\n");

	/* "52954aa1 mov"@0xffffff8008a6b3b4 + "72a00cc1 movk"@0xffffff8008a6b3bc
	 * = 0x66aa55, and the length is 3 ("320007e2 orr"@0xffffff8008a6b3c0) */
	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* line 247;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 247
		ILI_ERR("Write key failed\n");

	/* "528000a1 mov"@0xffffff8008a6b3e8 = 5 */
	if (ilitek_ice_mode_write(0x41008, 0x5, 1) < 0)
		/* line 250;
		 * "\x013ILITEK: (%s, %d): Write 0x5 cmd failed\n"@0xffffff800924724c */
#line 250
		ILI_ERR("Write 0x5 cmd failed\n");

	do {
		if (ilitek_ice_mode_write(0x41008, 0xFF, 1) < 0)
			/* line 254;
			 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0 */
#line 254
			ILI_ERR("Write dummy failed\n");

		/* see G5 */
		mdelay(1);

		/* "910013e1 add"@0xffffff8008a6b45c -- &tmp, and the length is
		 * 1 ("320003e2 orr"@0xffffff8008a6b460) */
		if (ilitek_ice_mode_read(0x41010, &tmp, sizeof(u8)) < 0)
			/* line 259;
			 * "\x013ILITEK: (%s, %d): Read flash busy error\n"@0xffffff8009247276 */
#line 259
			ILI_ERR("Read flash busy error\n");

		/*
		 * "394013e8 ldrb"@0xffffff8008a6b47c + "7200051f tst"@0xffffff8008a6b480:
		 * the read for the COMPARISON is one byte, the one for the final
		 * print is 32-bit ("b94007e3 ldr"@0xffffff8008a6b4d8).
		 */
		if ((tmp & 0x3) == 0)
			break;
	} while (--timer >= 0);

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* line 266;
		 * "\x013ILITEK: (%s, %d): Pull cs high failed\n"@0xffffff80092472a1 */
#line 266
		ILI_ERR("Pull cs high failed\n");

	if (timer > 0)
		return 0;

	/* line 269;
	 * "\x013ILITEK: (%s, %d): Flash polling busy timeout ! tmp = %x\n"@0xffffff80092472ca */
#line 269
	ILI_ERR("Flash polling busy timeout ! tmp = %x\n", tmp);
	return -1;
}

/*
 * ===========================================================================
 * ilitek_tddi_flash_clear_dma -- 0xffffff8008a674dc, 312 bytes
 * ===========================================================================
 * Returns `void`: no path materialises w0 before the epilogue
 * (0xffffff8008a67608), and the two known calls -- 0xffffff8008a59c58 and
 * 0xffffff8008a5b324 -- do not read it.
 *
 * The three `%lu`: see G7. The value printed is the MASK, not the datum
 * written -- "32101fe3 orr"@0xffffff8008a67520 puts 0xff0000 into w3 while
 * the datum passed to the call was 0x20000
 * ("320f03e2 orr"@0xffffff8008a674f8). It is not a misreading: that is
 * how it is in the binary.
 */
void ilitek_tddi_flash_clear_dma(void)
{
	if (ilitek_ice_mode_bit_mask_write(0x41000, 0xFF0000, (2 << 16)) < 0)
		/* line 279;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 279
		ILI_ERR("Write %lu at %x failed\n", 0xFF0000UL, 0x41000);

	/* "11004293 add"@0xffffff8008a6752c -- 0x41000 + 0x10 */
	if (ilitek_ice_mode_bit_mask_write(0x41010, 0xFF000000, 0) < 0)
		/* line 282;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 282
		ILI_ERR("Write %lu at %x failed\n", 0xFF000000UL, 0x41010);

	if (ilitek_ice_mode_bit_mask_write(0x41000, 0x01000000, 0) < 0)
		/* line 285;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 285
		ILI_ERR("Write %lu at %x failed\n", 0x01000000UL, 0x41000);

	/* "11003293 add"@0xffffff8008a675a0 -- 0x41000 + 0xc */
	if (ilitek_ice_mode_write(0x4100C, 0x00, 1) < 0)
		/* line 288;
		 * "\x013ILITEK: (%s, %d): Write 0x0 at %x failed\n"@0xffffff8009242ece */
#line 288
		ILI_ERR("Write 0x0 at %x failed\n", 0x4100C);

	if (ilitek_ice_mode_write(0x41010, 0xFF, 1) < 0)
		/* line 291;
		 * "\x013ILITEK: (%s, %d): Write 0xFF at %x failed\n"@0xffffff8009245c39 */
#line 291
		ILI_ERR("Write 0xFF at %x failed\n", 0x41010);
}

/*
 * ilitek_tddi_flash_read_int_flag() was reconstructed from the factory kernel disassembly (0xffffff8008a67614, 264 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_flash_read_int_flag(void)
{
	int timer = 500;
	u32 flag = 0;

	do {
		if (ilitek_ice_mode_read(0x0, &flag, sizeof(u32)) < 0)
			/* line 301;
			 * "\x013ILITEK: (%s, %d): Read flash int flag error\n"@0xffffff8009245c66 */
#line 301
			ILI_ERR("Read flash int flag error\n");

		/* line 303. Guarded: "396562e8 ldrb"@0xffffff8008a67670 +
		 * "340000c8 cbz"@0xffffff8008a67674 */
		/* "\x016ILITEK: (%s, %d): int flag = %x\n"@0xffffff8009245cb5 */
#line 303
		ILI_DBG("int flag = %x\n", flag);

		/*
		 * "35000183 cbnz"@0xffffff8008a67690 -- the comparison is against
		 * ZERO, not against a particular flag value
		 */
		if (flag)
			break;
	} while (timer-- > 0);

	if (timer > 0)
		return 0;

	/*
	 * line 309. The argument is still in w3 from the `cbnz`: the binary does
	 * NOT reload it, but the value printed is `flag`.
	 */
	/* "\x013ILITEK: (%s, %d): Read Flash INT flag timeout !, flag = 0x%x\n"@0xffffff8009245cd8 */
#line 309
	ILI_ERR("Read Flash INT flag timeout !, flag = 0x%x\n", flag);
	return -1;
}

/*
 * ilitek_tddi_flash_dma_write() was reconstructed from the factory kernel disassembly (0xffffff8008a6771c, 1092 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_flash_dma_write(u32 start, u32 end, u32 len)
{
	if (ilitek_ice_mode_bit_mask_write(0x41000, 0xFF0000, (1 << 16)) < 0)
		/* line 318;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 318
		ILI_ERR("Write %lu at %x failed\n", 0xFF0000UL, 0x41000);

	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* line 321;
		 * "\x013ILITEK: (%s, %d): Pull cs low failed\n"@0xffffff800924114c */
#line 321
		ILI_ERR("Pull cs low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* line 324;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 324
		ILI_ERR("Write key failed\n");

	/* "52800161 mov"@0xffffff8008a677dc = 0x0b */
	if (ilitek_ice_mode_write(0x41008, 0x0b, 1) < 0)
		/* line 327;
		 * "\x013ILITEK: (%s, %d): Write 0x0b at %x failed\n"@0xffffff8009245d34 */
#line 327
		ILI_ERR("Write 0x0b at %x failed\n", 0x41008);

	if (ilitek_tddi_flash_read_int_flag() < 0) {
		/* line 330;
		 * "\x013ILITEK: (%s, %d): Write 0xb timeout \n"@0xffffff8009245d61 */
#line 330
		ILI_ERR("Write 0xb timeout \n");
		return;
	}

	/*
	 * "52900080 mov"@0xffffff8008a677f4 + "72a00080 movk"@0xffffff8008a677f8
	 * = 0x48004; the mask and the value are the same 1<<25
	 * ("320703e1 orr"@0xffffff8008a677fc, "320703e2 orr"@0xffffff8008a67800)
	 */
	if (ilitek_ice_mode_bit_mask_write(0x48004, (1 << 25), (1 << 25)) < 0)
		/* line 335;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 335
		ILI_ERR("Write %lu at %x failed\n", (1UL << 25), 0x48004);

	/* "53105ea1 ubfx"@0xffffff8008a67834 */
	if (ilitek_ice_mode_write(0x41008, (start & 0xFF0000) >> 16, 1) < 0)
		/* line 338;
		 * "\x013ILITEK: (%s, %d): Write %x at %x failed\n"@0xffffff8009242f72 */
#line 338
		ILI_ERR("Write %x at %x failed\n", start & 0xFF0000, 0x41008);

	if (ilitek_tddi_flash_read_int_flag() < 0) {
		/* line 341;
		 * "\x013ILITEK: (%s, %d): Write addr1 timeout\n"@0xffffff8009245d89 */
#line 341
		ILI_ERR("Write addr1 timeout\n");
		return;
	}

	if (ilitek_ice_mode_bit_mask_write(0x48004, (1 << 25), (1 << 25)) < 0)
		/* line 346;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 346
		ILI_ERR("Write %lu at %x failed\n", (1UL << 25), 0x48004);

	/*
	 * "53083eb6 ubfx"@0xffffff8008a6788c -- the result stays in w22
	 * because it is needed TWICE, for the write and for the print
	 */
	if (ilitek_ice_mode_write(0x41008, (start & 0x00FF00) >> 8, 1) < 0)
		/* line 349;
		 * "\x013ILITEK: (%s, %d): Write %x at %x failed\n"@0xffffff8009242f72 */
#line 349
		ILI_ERR("Write %x at %x failed\n", (start & 0x00FF00) >> 8, 0x41008);

	if (ilitek_tddi_flash_read_int_flag() < 0) {
		/* line 352;
		 * "\x013ILITEK: (%s, %d): Write addr2 timeout\n"@0xffffff8009245db2 */
#line 352
		ILI_ERR("Write addr2 timeout\n");
		return;
	}

	if (ilitek_ice_mode_bit_mask_write(0x48004, (1 << 25), (1 << 25)) < 0)
		/* line 357;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 357
		ILI_ERR("Write %lu at %x failed\n", (1UL << 25), 0x48004);

	/* "12001eb5 and"@0xffffff8008a678ec */
	if (ilitek_ice_mode_write(0x41008, (start & 0x0000FF), 1) < 0)
		/* line 360;
		 * "\x013ILITEK: (%s, %d): Write %x at %x failed\n"@0xffffff8009242f72 */
#line 360
		ILI_ERR("Write %x at %x failed\n", (start & 0x0000FF), 0x41008);

	if (ilitek_tddi_flash_read_int_flag() < 0) {
		/* line 363;
		 * "\x013ILITEK: (%s, %d): Write addr3 timeout\n"@0xffffff8009245ddb */
#line 363
		ILI_ERR("Write addr3 timeout\n");
		return;
	}

	if (ilitek_ice_mode_bit_mask_write(0x48004, (1 << 25), (1 << 25)) < 0)
		/* line 368;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 368
		ILI_ERR("Write %lu at %x failed\n", (1UL << 25), 0x48004);

	if (ilitek_ice_mode_bit_mask_write(0x41000, (1 << 24), 0) < 0)
		/* line 371;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 371
		ILI_ERR("Write %lu at %x failed\n", (1UL << 24), 0x41000);

	if (ilitek_ice_mode_write(0x41008, 0x00, 1) < 0)
		/* line 374;
		 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0 */
#line 374
		ILI_ERR("Write dummy failed\n");

	if (ilitek_tddi_flash_read_int_flag() < 0) {
		/* line 377;
		 * "\x013ILITEK: (%s, %d): Write dummy timeout\n"@0xffffff8009245e04 */
#line 377
		ILI_ERR("Write dummy timeout\n");
		return;
	}

	if (ilitek_ice_mode_bit_mask_write(0x48004, (1 << 25), (1 << 25)) < 0)
		/* line 382;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 382
		ILI_ERR("Write %lu at %x failed\n", (1UL << 25), 0x48004);

	/* "110012e0 add"@0xffffff8008a679dc = 0x41008 + 4, length 4
	 * ("321e03e2 orr"@0xffffff8008a679e0) */
	if (ilitek_ice_mode_write(0x4100C, len, 4) < 0)
		/* line 385;
		 * "\x013ILITEK: (%s, %d): Write length failed\n"@0xffffff8009245e2d */
#line 385
		ILI_ERR("Write length failed\n");
}

/*
 * ===========================================================================
 * ilitek_tddi_flash_write_enable -- 0xffffff8008a6b274, 220 bytes
 * ===========================================================================
 * `t` in the map: `static`. Returns `void` -- the last instruction before
 * the epilogue is a `bl printk` and no path materialises w0.
 */
static void ilitek_tddi_flash_write_enable(void)
{
	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/*
		 * line 391. The format has "CS" in CAPITALS here
		 * (@0xffffff8009245ecb) and in lower case elsewhere
		 * (@0xffffff800924114c): they are two different literals.
		 */
		/* "\x013ILITEK: (%s, %d): Pull CS low failed\n"@0xffffff8009245ecb */
#line 391
		ILI_ERR("Pull CS low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* line 394;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 394
		ILI_ERR("Write key failed\n");

	/* "321f07e1 orr"@0xffffff8008a6b2ec = 6 */
	if (ilitek_ice_mode_write(0x41008, 0x6, 1) < 0)
		/* line 397;
		 * "\x013ILITEK: (%s, %d): Write 0x6 failed\n"@0xffffff80092471e1 */
#line 397
		ILI_ERR("Write 0x6 failed\n");

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* line 400;
		 * "\x013ILITEK: (%s, %d): Pull CS high failed\n"@0xffffff8009247207 */
#line 400
		ILI_ERR("Pull CS high failed\n");
}

/*
 * ilitek_tddi_fw_read_hw_crc() was reconstructed from the factory kernel disassembly (0xffffff8008a67b60, 1112 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_fw_read_hw_crc(u32 start, u32 end)
{
	int retry = 500;
	u32 busy = 0, crc = 0;

	/* "f9401908 ldr"@0xffffff8008a67b98 + "b9403904 ldr"@0xffffff8008a67b9c,
	 * an UNSIGNED comparison ("54000122 b.cs"@0xffffff8008a67ba4) */
	if (end > ((struct ilitek_g_c48 *)idev->c48)->c56) {
		/*
		 * line 412. The second argument (w4) is NOT reloaded: it is
		 * still the one from the `ldr` of the comparison.
		 */
		/* "\x013ILITEK: (%s, %d): The length (%x) written into firmware is greater than max count (%x)\n"@0xffffff8009245e56 */
		ILI_ERR("The length (%x) written into firmware is greater than max count (%x)\n",
#line 412
			end, ((struct ilitek_g_c48 *)idev->c48)->c56);
		return -1;
	}

	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* line 417;
		 * "\x013ILITEK: (%s, %d): Pull CS low failed\n"@0xffffff8009245ecb */
#line 417
		ILI_ERR("Pull CS low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* line 420;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 420
		ILI_ERR("Write key failed\n");

	/* "52800761 mov"@0xffffff8008a67c3c = 0x3b */
	if (ilitek_ice_mode_write(0x41008, 0x3b, 1) < 0)
		/* line 423;
		 * "\x013ILITEK: (%s, %d): Write 0x3b failed\n"@0xffffff8009245ef3 */
#line 423
		ILI_ERR("Write 0x3b failed\n");

	/* "53105e81 ubfx"@0xffffff8008a67c68 */
	if (ilitek_ice_mode_write(0x41008, (start & 0xFF0000) >> 16, 1) < 0)
		/*
		 * line 426 -- the SAME format @0xffffff80092411c6 for all
		 * three address writes
		 */
		/* "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 426
		ILI_ERR("Write address failed\n");

	/* "53083e81 ubfx"@0xffffff8008a67c98 */
	if (ilitek_ice_mode_write(0x41008, (start & 0x00FF00) >> 8, 1) < 0)
		/* line 429;
		 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 429
		ILI_ERR("Write address failed\n");

	/* "12001e81 and"@0xffffff8008a67cc8 */
	if (ilitek_ice_mode_write(0x41008, (start & 0x0000FF), 1) < 0)
		/* line 432;
		 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 432
		ILI_ERR("Write address failed\n");

	/* "51001714 sub"@0xffffff8008a67cf4 -- 0x41008 - 5 = 0x41003 */
	if (ilitek_ice_mode_write(0x41003, 0x01, 1) < 0)
		/* line 435;
		 * "\x013ILITEK: (%s, %d): Write enable Dio_Rx_dual failed\n"@0xffffff8009245f1a */
#line 435
		ILI_ERR("Write enable Dio_Rx_dual failed\n");

	if (ilitek_ice_mode_write(0x41008, 0xFF, 1) < 0)
		/* line 438;
		 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0 */
#line 438
		ILI_ERR("Write dummy failed\n");

	/* "11001300 add"@0xffffff8008a67d54 -- 0x41008 + 4; length 3 */
	if (ilitek_ice_mode_write(0x4100C, end, 3) < 0)
		/* line 441;
		 * "\x013ILITEK: (%s, %d): Write Set Receive count failed\n"@0xffffff8009245f4f */
#line 441
		ILI_ERR("Write Set Receive count failed\n");

	/* "529000e0 mov"@0xffffff8008a67d80 + "72a00080 movk"@0xffffff8008a67d84
	 * = 0x48007 */
	if (ilitek_ice_mode_write(0x48007, 0x02, 1) < 0)
		/* line 444;
		 * "\x013ILITEK: (%s, %d): Write clearing int flag failed\n"@0xffffff8009245f83 */
#line 444
		ILI_ERR("Write clearing int flag failed\n");

	/*
	 * "11003b15 add"@0xffffff8008a67db0 -- 0x41008 + 0xe = 0x41016, and it is
	 * the same 041016 the format @0xffffff8009245fb7 names
	 */
	if (ilitek_ice_mode_write(0x41016, 0x00, 1) < 0)
		/* line 447;
		 * "\x013ILITEK: (%s, %d): Write 0x0 at 0x041016 failed\n"@0xffffff8009245fb7 */
#line 447
		ILI_ERR("Write 0x0 at 0x041016 failed\n");

	if (ilitek_ice_mode_write(0x41016, 0x01, 1) < 0)
		/* line 450;
		 * "\x013ILITEK: (%s, %d): Write Checksum_En failed\n"@0xffffff8009245fe9 */
#line 450
		ILI_ERR("Write Checksum_En failed\n");

	/* "11002300 add"@0xffffff8008a67e0c -- 0x41008 + 8 = 0x41010 */
	if (ilitek_ice_mode_write(0x41010, 0xFF, 1) < 0)
		/* line 453;
		 * "\x013ILITEK: (%s, %d): Write start to receive failed\n"@0xffffff8009246017 */
#line 453
		ILI_ERR("Write start to receive failed\n");

	do {
		if (ilitek_ice_mode_read(0x48007, &busy, sizeof(u8)) < 0)
			/* line 457;
			 * "\x013ILITEK: (%s, %d): Read busy error\n"@0xffffff800924604a */
#line 457
			ILI_ERR("Read busy error\n");

		/* line 459, guarded by "39656348 ldrb"@0xffffff8008a67e70;
		 * "\x016ILITEK: (%s, %d): busy = %x\n"@0xffffff800924606f */
#line 459
		ILI_DBG("busy = %x\n", busy);

		/*
		 * "394013e8 ldrb"@0xffffff8008a67e8c +
		 * "37080188 tbnz"@0xffffff8008a67e90: bit 1 alone
		 */
		if ((busy & 0x02) == 0x02)
			break;
	} while (retry-- > 0);

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* line 465;
		 * "\x013ILITEK: (%s, %d): Write CS high failed\n"@0xffffff800924608e */
#line 465
		ILI_ERR("Write CS high failed\n");

	if (retry <= 0) {
		/* line 468;
		 * "\x013ILITEK: (%s, %d): Read HW CRC timeout !, busy = 0x%x\n"@0xffffff80092460b8 */
#line 468
		ILI_ERR("Read HW CRC timeout !, busy = 0x%x\n", busy);
		return -1;
	}

	if (ilitek_ice_mode_write(0x41003, 0x00, 1) < 0)
		/* line 473;
		 * "\x013ILITEK: (%s, %d): Write disable dio_Rx_dual failed\n"@0xffffff80092460f0 */
#line 473
		ILI_ERR("Write disable dio_Rx_dual failed\n");

	/* "11005300 add"@0xffffff8008a67f0c -- 0x41008 + 0x14 = 0x4101c;
	 * "910003e1 mov"@0xffffff8008a67f10 -- &crc is at the top of the stack,
	 * length 4 */
	if (ilitek_ice_mode_read(0x4101C, &crc, sizeof(u32)) < 0) {
		/* line 476;
		 * "\x013ILITEK: (%s, %d): Read hw crc error\n"@0xffffff8009246126 */
#line 476
		ILI_ERR("Read hw crc error\n");
		return -1;
	}

	return crc;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_read_flash_data -- 0xffffff8008a67fb8, 712 bytes
 * ===========================================================================
 * FOUR parameters, fixed by the call site of
 * `ilitek_tddi_fw_dump_flash_data`:
 *   "2a1703e0 mov"@0xffffff8008a68348  w0 start
 *   "2a1403e1 mov"@0xffffff8008a6834c  w1 end
 *   "aa1503e2 mov"@0xffffff8008a68350  x2 pointer
 *   "2a1603e3 mov"@0xffffff8008a68354  w3 length
 *
 * EVERYTHING IS UNSIGNED here, the opposite of `ilitek_tddi_fw_iram_read`:
 *   "54000129 b.ls"@0xffffff8008a67ff0    length check
 *   "383c4a68 strb"@0xffffff8008a681dc    index `uxtw`, not `sxtw`
 *   "1ad40b23 udiv"@0xffffff8008a681d8    unsigned percentage
 *   "54fffb69 b.ls"@0xffffff8008a68214    loop end
 */
int ilitek_tddi_fw_read_flash_data(u32 start, u32 end, u8 *data, u32 len)
{
	u32 i, index = 0;
	/*
	 * `tmp` is NOT cleared: between the prologue and the first use the factory
	 * writes nothing into its slot, and with `= 0` clang emitted a
	 * "b90007ff str wzr, [sp,#4]" that is not in the binary -- four bytes,
	 * and they were the whole difference.
	 */
	u32 tmp;

	/* "4b000023 sub"@0xffffff8008a67fe4 + "6b04007f cmp"@0xffffff8008a67fe8 */
	if ((end - start) > len) {
		/* line 489;
		 * "\x013ILITEK: (%s, %d): the length (%d) reading crc is over than len(%d)\n"@0xffffff800924614d */
		ILI_ERR("the length (%d) reading crc is over than len(%d)\n",
#line 489
			end - start, len);
		return -1;
	}

	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* line 494;
		 * "\x013ILITEK: (%s, %d): Write cs low failed\n"@0xffffff80092461b2 */
#line 494
		ILI_ERR("Write cs low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* line 497;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 497
		ILI_ERR("Write key failed\n");

	/* "320007e1 orr"@0xffffff8008a68090 = 3 */
	if (ilitek_ice_mode_write(0x41008, 0x03, 1) < 0)
		/* line 500;
		 * "\x013ILITEK: (%s, %d): Write 0x3 failed\n"@0xffffff80092461db */
#line 500
		ILI_ERR("Write 0x3 failed\n");

	if (ilitek_ice_mode_write(0x41008, (start & 0xFF0000) >> 16, 1) < 0)
		/* line 503;
		 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 503
		ILI_ERR("Write address failed\n");

	if (ilitek_ice_mode_write(0x41008, (start & 0x00FF00) >> 8, 1) < 0)
		/* line 506;
		 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 506
		ILI_ERR("Write address failed\n");

	if (ilitek_ice_mode_write(0x41008, (start & 0x0000FF), 1) < 0)
		/* line 509;
		 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 509
		ILI_ERR("Write address failed\n");

	/*
	 * The loop entry guard is "6b15029f cmp"@0xffffff8008a68130
	 * + "54000142 b.cs"@0xffffff8008a68134, that is `start <= end`.
	 */
	for (i = start; i <= end; i++) {
		if (ilitek_ice_mode_write(0x41008, 0xFF, 1) < 0)
			/* line 513;
			 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0 */
#line 513
			ILI_ERR("Write dummy failed\n");

		if (ilitek_ice_mode_read(0x41010, &tmp, sizeof(u8)) < 0)
			/* line 516;
			 * "\x013ILITEK: (%s, %d): Read flash data error!\n"@0xffffff8009246201 */
#line 516
			ILI_ERR("Read flash data error!\n");

		data[index] = tmp;
		index++;

		/*
		 * "1b087eb9 mul"@0xffffff8008a68178 builds start*100 outside
		 * the loop and "11019339 add"@0xffffff8008a68210 raises it by
		 * 100 each round: it is the strength reduction of `i * 100`.
		 */
		idev->c444 = (i * 100) / end;

		/*
		 * line 521, guarded by "396562e8 ldrb"@0xffffff8008a681ec.
		 * The '%' is the w4 argument ("528004a4 mov"@0xffffff8008a681f8) and
		 * the format @0xffffff800924622d ends WITHOUT a \n.
		 */
		/* "\x016ILITEK: (%s, %d): Reading flash data .... %d%c"@0xffffff800924622d */
#line 521
		ILI_DBG("Reading flash data .... %d%c", idev->c444, '%');
	}

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* line 525;
		 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
#line 525
		ILI_ERR("Write cs high failed\n");

	return 0;
}

/*
 * ilitek_tddi_fw_dump_flash_data() was reconstructed from the factory kernel disassembly (0xffffff8008a68280, 608 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_fw_dump_flash_data(u32 start, u32 end, bool user)
{
	struct file *f = NULL;
	u8 *hex_buffer = NULL;
	u32 start_addr, end_addr;
	int ret = 0, length;
	loff_t pos = 0;
	mm_segment_t old_fs;

	/* "b901bd1f str"@0xffffff8008a682d0 */
	idev->c444 = 0;

	f = filp_open(ILI_DUMP_FLASH_PATH, O_WRONLY | O_CREAT | O_TRUNC, 644);
	if (IS_ERR(f) || f == NULL) {
		/*
		 * line 543 -- the SAME format @0xffffff8009245b9c that
		 * `ilitek_fw_dump_iram_data` uses
		 */
		/* "\x013ILITEK: (%s, %d): Failed to open the file at %ld.\n"@0xffffff8009245b9c */
#line 543
		ILI_ERR("Failed to open the file at %ld.\n", PTR_ERR(f));
		ret = -1;
		goto out;
	}

	ret = ilitek_ice_mode_ctrl(true, false);
	if (ret < 0)
		goto out;

	/*
	 * "720002ff tst"@0xffffff8008a682f8 -- bit 0 alone of the third
	 * parameter; "320043e8 orr"@0xffffff8008a682fc = 0x1ffff
	 */
	if (user) {
		start_addr = 0x0;
		end_addr = 0x1FFFF;
	} else {
		start_addr = start;
		end_addr = end;
	}

	/* "4b170288 sub"@0xffffff8008a68308 + "11000515 add"@0xffffff8008a6830c */
	length = end_addr - start_addr + 1;
	/*
	 * line 561. NOT guarded: level \x016 with no guard;
	 * "\x016ILITEK: (%s, %d): len = %d\n"@0xffffff80092462ba
	 */
#line 561
	ILI_INFO("len = %d\n", length);

	/*
	 * "93407eb6 sxtw"@0xffffff8008a6832c -- the length is SIGN-extended
	 * before the `vmalloc`: `length` is an `int`.
	 */
	hex_buffer = vmalloc(length);
	if (IS_ERR(hex_buffer) || hex_buffer == NULL) {
		/* line 565;
		 * "\x013ILITEK: (%s, %d): Failed to allocate buf memory, %ld\n"@0xffffff80092462d8 */
#line 565
		ILI_ERR("Failed to allocate buf memory, %ld\n", PTR_ERR(hex_buffer));
		filp_close(f, NULL);
		ret = -1;
		goto out;
	}

	ret = ilitek_tddi_fw_read_flash_data(start_addr, end_addr, hex_buffer, length);
	if (ret < 0)
		/*
		 * A FACTORY DEFECT, REPRODUCED (rule 7): this branch
		 * jumps to the tail WITHOUT `filp_close` and WITHOUT `vfree`
		 * ("37f807e0 tbnz"@0xffffff8008a68360 goes straight to
		 * 0xffffff8008a6845c, where the tail starts with the
		 * `ilitek_ice_mode_ctrl`). It leaks the file and the buffer.
		 */
		goto out;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	set_fs(get_ds());
	pos = 0;
	vfs_write(f, hex_buffer, length, &pos);
	set_fs(old_fs);

	filp_close(f, NULL);
	vfree(hex_buffer);

out:
	ilitek_ice_mode_ctrl(false, false);
	/* line 586;
	 * "\x016ILITEK: (%s, %d): Dump flash %s\n"@0xffffff8009246310
	 * "FAIL"@0xffffff8009249687
	 * "SUCCESS"@0xffffff80090f6e9d */
#line 586
	ILI_INFO("Dump flash %s\n", (ret < 0) ? "FAIL" : "SUCCESS");
	idev->c444 = (ret < 0) ? -1 : 100;
	return ret;
}

/*
 * ilitek_tddi_flash_protect() was reconstructed from the factory kernel disassembly (0xffffff800924749a).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ilitek_tddi_flash_protect(bool enable)
{
	/* line 593. Level \x016 and no guard;
	 * "\x016ILITEK: (%s, %d): %s flash protection\n"@0xffffff8009247471
	 * "Disable"@0xffffff80090df610 */
#line 593
	ILI_INFO("%s flash protection\n", enable ? "Enable" : "Disable");

	ilitek_tddi_flash_write_enable();

	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* line 598;
		 * "\x013ILITEK: (%s, %d): Write cs low failed\n"@0xffffff80092461b2 */
#line 598
		ILI_ERR("Write cs low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* line 601;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 601
		ILI_ERR("Write key failed\n");

	if (ilitek_ice_mode_write(0x41008, 0x1, 1) < 0)
		/* line 604;
		 * "\x013ILITEK: (%s, %d): Write 0x1 failed\n"@0xffffff80092474b4 */
#line 604
		ILI_ERR("Write 0x1 failed\n");

	if (ilitek_ice_mode_write(0x41008, 0x0, 1) < 0)
		/* line 607;
		 * "\x013ILITEK: (%s, %d): Write 0x0 failed\n"@0xffffff80092474da */
#line 607
		ILI_ERR("Write 0x0 failed\n");

	/*
	 * "79444103 ldrh"@0xffffff8008a6af2c -- the choice is on the MID, not
	 * on the ID; "7103207f cmp"@0xffffff8008a6af30 (0xc8) and
	 * "7103bc7f cmp"@0xffffff8008a6af38 (0xef)
	 */
	switch (idev->c544) {
	case 0xEF:
		/*
		 * "128c0209 mov"@0xffffff8008a6af44 = -24593 = -0x6011 and
		 * "7100051f cmp"@0xffffff8008a6af4c with 1: that is the RANGE check
		 * clang folds `a == 0x6011 || a == 0x6012` into.
		 */
		if (idev->c546 == 0x6011 || idev->c546 == 0x6012) {
			if (ilitek_ice_mode_write(0x41008, 0x0, 1) < 0)
				/* line 617;
				 * "\x013ILITEK: (%s, %d): Write 0x0 at %x failed\n"@0xffffff8009242ece */
				ILI_ERR("Write 0x0 at %x failed\n", 0x41008);
		}
		break;
	case 0xC8:
		/*
		 * "52860129 mov"@0xffffff8008a6af90 = 0x3009 and
		 * "6b48053f cmp"@0xffffff8008a6af94 compares with `dev_id >> 1`:
		 * that is the shape clang folds `a == 0x6012 || a == 0x6013` into,
		 * two constants differing in bit 0 alone.
		 */
		if (idev->c546 == 0x6012 || idev->c546 == 0x6013) {
			if (ilitek_ice_mode_write(0x41008, 0x0, 1) < 0)
				/* line 628;
				 * "\x013ILITEK: (%s, %d): Write 0x0 at %x failed\n"@0xffffff8009242ece */
				ILI_ERR("Write 0x0 at %x failed\n", 0x41008);
		}
		break;
	default:
		/*
		 * line 634. The argument is still in w3 from the comparison;
		 * "\x013ILITEK: (%s, %d): Can't find flash id(0x%x), ignore protection\n"@0xffffff8009247500
		 */
		ILI_ERR("Can't find flash id(0x%x), ignore protection\n",
#line 634
			idev->c544);
		break;
	}

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* line 639;
		 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
#line 639
		ILI_ERR("Write cs high failed\n");
}

/*
 * ilitek_tddi_fw_flash_erase() was reconstructed from the factory kernel disassembly (0xffffff8008a6a758, 612 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tddi_fw_flash_erase(void)
{
	int i;
	u32 addr;

	/* "f1001e7f cmp"@0xffffff8008a6a98c -- seven entries */
	for (i = 0; i < 7; i++) {
		/* "34000ec5 cbz"@0xffffff8008a6a7b0 */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		if (ilitek_fw_blk[i].c12 / 8192 <= 14 &&
		    ilitek_fw_blk[i].c8 / 4096 > 28)
			continue;

		/*
		 * line 797. Level \x016, no guard. The two addresses
		 * are already in w4 and w5 from the two `ldr`s above: the binary does NOT
		 * reload them, and that is the proof that the arguments are exactly
		 * those two fields in that order.
		 */
		/* "\x016ILITEK: (%s, %d): Block[%d]: Erasing from (0x%x) to (0x%x) \n"@0xffffff80092473be */
		ILI_INFO("Block[%d]: Erasing from (0x%x) to (0x%x) \n",
#line 797
			 i, ilitek_fw_blk[i].c8, ilitek_fw_blk[i].c12);

		/*
		 * "6b08033f cmp"@0xffffff8008a6a7f0 + "54000ca8 b.hi"@0xffffff8008a6a7f4
		 * -- UNSIGNED guard of the inner loop
		 */
		for (addr = ilitek_fw_blk[i].c8; addr <= ilitek_fw_blk[i].c12;
		     addr += idev->c552) {
			ilitek_tddi_flash_write_enable();

			if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
				/* line 803;
				 * "\x013ILITEK: (%s, %d): Write cs low failed\n"@0xffffff80092461b2 */
#line 803
				ILI_ERR("Write cs low failed\n");

			if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
				/* line 806;
				 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 806
				ILI_ERR("Write key failed\n");

			/*
			 * "b945e288 ldr"@0xffffff8008a6a850 -- 0xa0ff000+1504 =
			 * 0xa0ff5e0 = ilitek_fw_blk[1].c8, that is the entry
			 * `ilitek_tddi_fw_update_block_info` calls "AP"
			 */
			if (addr == ilitek_fw_blk[1].c8) {
				/* "52801b01 mov"@0xffffff8008a6a864 = 0xd8 */
				if (ilitek_ice_mode_write(0x41008, 0xD8, 1) < 0)
					/*
					 * line 810. The format says "0xB" but
					 * the command written is 0xD8: it is a
					 * defect of the factory message and it
					 * is reproduced.
					 */
					/* "\x013ILITEK: (%s, %d): Write 0xB at %x failed\n"@0xffffff8009247418 */
#line 810
					ILI_ERR("Write 0xB at %x failed\n", 0x41008);
			} else {
				/* "321b03e1 orr"@0xffffff8008a6a88c = 0x20 */
				if (ilitek_ice_mode_write(0x41008, 0x20, 1) < 0)
					/* line 813;
					 * "\x013ILITEK: (%s, %d): Write 0x20 at %x failed\n"@0xffffff8009247444 */
#line 813
					ILI_ERR("Write 0x20 at %x failed\n", 0x41008);
			}

			/*
			 * "12181f21 and"@0xffffff8008a6a8b8,
			 * "33105f21 bfxil"@0xffffff8008a6a8bc,
			 * "33101f21 bfi"@0xffffff8008a6a8c4:
			 * the three address bytes swapped and sent in ONE
			 * 3-byte write ("320007e2 orr"@0xffffff8008a6a8cc)
			 */
			if (ilitek_ice_mode_write(0x41008,
						  ((addr & 0xFF0000) >> 16) |
						  (addr & 0x00FF00) |
						  ((addr & 0x0000FF) << 16), 3) < 0)
				/* line 818;
				 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 818
				ILI_ERR("Write address failed\n");

			if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
				/* line 821;
				 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
#line 821
				ILI_ERR("Write cs high failed\n");

			mdelay(1);

			/*
			 * "5281b597 mov"@0xffffff8008a6a790 = 3500 and
			 * "52803e96 mov"@0xffffff8008a6a78c = 500, chosen with
			 * "1a9602e0 csel"@0xffffff8008a6a928 on the comparison
			 * RELOADED after the `mdelay`
			 * ("b945e288 ldr"@0xffffff8008a6a920)
			 */
			if (ilitek_tddi_flash_poll_busy(addr == ilitek_fw_blk[1].c8 ?
							3500 : 500) < 0)
				return -1;

			if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
				/* line 835;
				 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
#line 835
				ILI_ERR("Write cs high failed\n");

			/*
			 * "b9400308 ldr"@0xffffff8008a6a95c -- the comparison is
			 * on the BLOCK's `c8`, not on `addr`
			 */
			if (ilitek_fw_blk[i].c8 == ilitek_fw_blk[1].c8)
				break;
		}
	}

	return 0;
}

/*
 * calc_crc32() was reconstructed from the factory kernel disassembly (0xffffff8008a6b12c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static u32 calc_crc32(u32 start_addr, u32 end_addr, u8 *data)
{
	int i, j;
	u32 CRC_POLY = 0x04C11DB7;
	u32 ReturnCRC = 0xFFFFFFFF;
	u32 len = start_addr + end_addr;

	for (i = start_addr; i < len; i++) {
		/*
		 * "4a0b62eb eor"@0xffffff8008a6b128 -- the byte enters from
		 * bit 24
		 */
		ReturnCRC ^= (data[i] << 24);

		for (j = 0; j < 8; j++) {
			if ((ReturnCRC & 0x80000000) != 0)
				ReturnCRC = ReturnCRC << 1 ^ CRC_POLY;
			else
				ReturnCRC = ReturnCRC << 1;
		}
	}

	return ReturnCRC;
}

/*
 * ===========================================================================
 * ilitek_fw_calc_file_crc -- 0xffffff8008a6b0b4, 448 bytes
 * ===========================================================================
 * `t` in the map. HERE it is NOT `static`, for the same reason as
 * `ilitek_tddi_fw_flash_erase`: the two call sites (0xffffff8008a6969c and
 * 0xffffff8008a6a6c0) are inside `ilitek_tddi_fw_upgrade`. Divergence G2b.
 */
static int ilitek_fw_calc_file_crc(u8 *pfw)
{
	int i;
	u32 ex_addr, data_crc, file_crc;

	/* "f1001e7f cmp"@0xffffff8008a6b20c -- seven entries, come in
	 * `ilitek_tddi_fw_flash_erase` */
	for (i = 0; i < 7; i++) {
		/* "34000868 cbz"@0xffffff8008a6b0fc */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		ex_addr = ilitek_fw_blk[i].c12;
		/*
		 * "5100116b sub"@0xffffff8008a6b10c -- the length passed is
		 * c16 MINUS 4: the last four bytes are the CRC itself
		 */
		data_crc = calc_crc32(ilitek_fw_blk[i].c8,
				      ilitek_fw_blk[i].c16 - 4, pfw);

		/*
		 * "53081d29 lsl"@0xffffff8008a6b1d8,
		 * "33101d49 bfi"@0xffffff8008a6b1dc,
		 * "33181d69 bfi"@0xffffff8008a6b1e0,
		 * "2a080138 orr"@0xffffff8008a6b1e4:
		 * four bytes in BIG-ENDIAN order, and the first three indexed
		 * `uxtw` ("38694a89 ldrb"@0xffffff8008a6b1c4)
		 */
		file_crc = pfw[ex_addr - 3] << 24 | pfw[ex_addr - 2] << 16 |
			   pfw[ex_addr - 1] << 8 | pfw[ex_addr];

		/* line 912. Level \x016, no guard;
		 * "\x016ILITEK: (%s, %d): data crc = %x, file crc = %x\n"@0xffffff8009246b49 */
#line 912
		ILI_INFO("data crc = %x, file crc = %x\n", data_crc, file_crc);

		/* "6b1802ff cmp"@0xffffff8008a6b200 */
		if (data_crc != file_crc) {
			/* line 915;
			 * "\x013ILITEK: (%s, %d): Content of fw file is broken. (%d, %x, %x)\n"@0xffffff8009246b93 */
			ILI_ERR("Content of fw file is broken. (%d, %x, %x)\n",
#line 915
				i, data_crc, file_crc);
			return -1;
		}
	}

	/* line 920;
	 * "\x016ILITEK: (%s, %d): Content of fw file is correct\n"@0xffffff8009246bd3 */
#line 920
	ILI_INFO("Content of fw file is correct\n");
	return 0;
}

/*
 * ilitek_tddi_fw_read_flash_info() was reconstructed from the factory kernel disassembly (0xffffff8008a6a9bc, 1784 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_fw_read_flash_info(void)
{
	int i;
	int ice = idev->c712;
	u8 buf[4] = {0};
	u16 flash_id = 0, flash_mid = 0;
	u32 tmp = 0;

	if (!ice) {
		if (ilitek_ice_mode_ctrl(true, false) < 0)
			/* line 1361;
			 * "\x013ILITEK: (%s, %d): Enable ice mode failed while reading flash info\n"@0xffffff8009246574 */
#line 1361
			ILI_ERR("Enable ice mode failed while reading flash info\n");
	}

	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* line 1365;
		 * "\x013ILITEK: (%s, %d): Write cs low failed\n"@0xffffff80092461b2 */
#line 1365
		ILI_ERR("Write cs low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* line 1368;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 1368
		ILI_ERR("Write key failed\n");

	/* "528013e1 mov"@0xffffff8008a6aa94 = 0x9f */
	if (ilitek_ice_mode_write(0x41008, 0x9F, 1) < 0)
		/* line 1371;
		 * "\x013ILITEK: (%s, %d): Write 0x9F failed\n"@0xffffff80092465d8 */
#line 1371
		ILI_ERR("Write 0x9F failed\n");

	/*
	 * FOUR ITERATIONS, and the binary unrolled all four with the
	 * SAME line number (0x55f = 1375 and 0x562 = 1378 each appear
	 * four times): it is a loop in the source, not four hand-written
	 * copies. The fourth value read is never used -- the last read
	 * (0xffffff8008a6ac18) is not followed by any `ldrb` -- which is
	 * exactly what one expects from a `buf[3]` that is never read back.
	 */
	for (i = 0; i < 4; i++) {
		if (ilitek_ice_mode_write(0x41008, 0xFF, 1) < 0)
			/* line 1375;
			 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0 */
#line 1375
			ILI_ERR("Write dummy failed\n");

		if (ilitek_ice_mode_read(0x41010, &tmp, sizeof(u8)) < 0)
			/* line 1378;
			 * "\x013ILITEK: (%s, %d): Read flash info error\n"@0xffffff80092465ff */
#line 1378
			ILI_ERR("Read flash info error\n");

		buf[i] = tmp;
	}

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* line 1384;
		 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
#line 1384
		ILI_ERR("Write cs high failed\n");

	/* "394013f5 ldrb"@0xffffff8008a6ab18 */
	flash_mid = buf[0];
	/* "33181f59 bfi"@0xffffff8008a6ac6c -- the high byte is the SECOND
	 * one read and the low one the THIRD */
	flash_id = buf[1] << 8 | buf[2];

	/*
	 * The search is unrolled over the eight entries: eight
	 * `ldrh`+`cmp`+`b.ne` triples, the last of which
	 * ("7949e103 ldrh"@0xffffff8008a6ada0) jumps to the "not found" branch.
	 */
	for (i = 0; i < ARRAY_SIZE(ilitek_flashtab); i++) {
		if (flash_mid == ilitek_flashtab[i].c0 &&
		    flash_id == ilitek_flashtab[i].c2) {
			idev->c544 = ilitek_flashtab[i].c0;
			idev->c546 = ilitek_flashtab[i].c2;
			idev->c548 = ilitek_flashtab[i].c8;
			idev->c552 = ilitek_flashtab[i].c12;
			break;
		}
	}

	if (i >= ARRAY_SIZE(ilitek_flashtab)) {
		/* line 1401. Level \x016, no guard;
		 * "\x016ILITEK: (%s, %d): Not found flash id in tab, use default\n"@0xffffff800924662a */
#line 1401
		ILI_INFO("Not found flash id in tab, use default\n");
		idev->c544 = ilitek_flashtab[0].c0;
		idev->c546 = ilitek_flashtab[0].c2;
		idev->c548 = ilitek_flashtab[0].c8;
		idev->c552 = ilitek_flashtab[0].c12;
	}

	/*
	 * line 1408. The two arguments are NOT reloaded: they are the same
	 * registers just written.
	 */
	/* "\x016ILITEK: (%s, %d): Flash MID = %x, Flash DEV_ID = %x\n"@0xffffff8009246666 */
#line 1408
	ILI_INFO("Flash MID = %x, Flash DEV_ID = %x\n", idev->c544, idev->c546);
	/*
	 * line 1409. This one IS reloaded
	 * ("b9422503 ldr"@0xffffff8008a6ae20), because the `printk` above
	 * may have written to memory.
	 */
	/* "\x016ILITEK: (%s, %d): Flash program page = %d\n"@0xffffff800924669d */
#line 1409
	ILI_INFO("Flash program page = %d\n", idev->c548);
	/* line 1410 ("b9422903 ldr"@0xffffff8008a6ae3c);
	 * "\x016ILITEK: (%s, %d): Flash sector = %d\n"@0xffffff80092466ca */
#line 1410
	ILI_INFO("Flash sector = %d\n", idev->c552);

	ilitek_tddi_flash_protect(false);

	if (!ice) {
		if (ilitek_ice_mode_ctrl(false, false) < 0)
			/* line 1416;
			 * "\x013ILITEK: (%s, %d): Disable ice mode failed while reading flash info\n"@0xffffff80092466f1 */
#line 1416
			ILI_ERR("Disable ice mode failed while reading flash info\n");
	}
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a684e0, 8824 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * ilitek_tddi_ic_watch_dog_ctrl() was reconstructed from the factory kernel disassembly (0xffffff8008a69a10).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_watch_dog_ctrl(bool write, bool enable);
int ilitek_tddi_ic_get_fw_ver(void);
int ilitek_tddi_ic_get_core_ver(void);
int ilitek_tddi_ic_get_protocl_ver(void);
int ilitek_tddi_ic_get_tp_info(void);
int ilitek_tddi_ic_get_panel_info(void);

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a698dc).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define EFW_CONVERT_FILE	114
#define EFW_ICE_MODE		115
#define EFW_WDT			116
#define EFW_CRC			117
#define EFW_REST		118
#define EFW_ERASE		119
#define EFW_PROGRAM		120

/*
 * ERR_ALLOC_MEM() was reconstructed from the factory kernel disassembly (0xffffff8008a6856c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define ERR_ALLOC_MEM(m)	((IS_ERR(m) || (m) == NULL) ? 1 : 0)

#define ipio_vfree(v)				\
	do {					\
		if (*(v) != NULL) {		\
			vfree(*(v));		\
			*(v) = NULL;		\
		}				\
	} while (0)

/*
 * The size of the firmware buffer. "52900000 mov"@0xffffff8008a68558 +
 * "72a00040 movk"@0xffffff8008a6855c = 0x28000 = 163840, which is both the
 * size of the `vmalloc` and the upper bound on the address
 * `ilitek_tddi_fw_hex_convert` accepts ("6b10007f cmp"@0xffffff8008a68e74).
 * NAME CHOSEN.
 */
#define MAX_HEX_FILE_SIZE	0x28000

/*
 * The number of `ilitek_fw_blk` entries. It is 7, and five independent
 * comparisons say so: "f1001e9f cmp"@0xffffff8008a69aa8,
 * "f1001e9f cmp"@0xffffff8008a69bf4, "f1001e9f cmp"@0xffffff8008a69f78,
 * "f1001e9f cmp"@0xffffff8008a6a098 and "f1001f9f cmp"@0xffffff8008a6a550.
 * The 6 the two error messages print is 7-1
 * ("321f07e4 orr"@0xffffff8008a6a6f8, "321f07e4 orr"@0xffffff8008a6a720,
 * "321f07e4 orr"@0xffffff8008a6a744). NAME CHOSEN.
 */
#define FW_BLOCK_INFO_NUM	7

/*
 * The two file-opening methods. The VALUES are measured -- the `switch`
 * in `ilitek_tdd_fw_hex_open` compares with 1 ("7100051f cmp"@0xffffff8008a685f4)
 * and with 0 ("350014c8 cbnz"@0xffffff8008a685fc) -- and the NAMES come from
 * the two strings the line 1149 message prints:
 * "REQUEST_FIRMWARE"@0xffffff8009246834 and "FILP_OPEN"@0xffffff800924682a.
 * This is the exception to rule 5: here the binary NAMES the two values.
 */
#define REQUEST_FIRMWARE	0
#define FILP_OPEN		1

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * vmalloc() was reconstructed from the factory kernel disassembly (0xffffff800a0ff5a8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
u8 *pfw;

/*
 * 0xffffff800a0ff5b0. Eight bytes ("f902dae8 str"@0xffffff8008a6a260,
 * "f942dae8 ldr"@0xffffff8008a6a480), a copy of `idev->c696`
 * ("f9415d28 ldr"@0xffffff8008a6a24c) and checked with `ERR_ALLOC_MEM`
 * ("b140051f cmn"@0xffffff8008a6a250). It is the ILI file buffer: all its
 * uses are inside `ilitek_tddi_fw_ili_convert`. NAME CHOSEN.
 */
static u8 *ilitek_ilifile;

/*
 * 0xffffff800a0ff698. ONE byte: "391a6154 strb"@0xffffff8008a696c0 and
 * "395a6145 ldrb"@0xffffff8008a69948, never read any wider.
 * NAME from the format
 * "\x016ILITEK: (%s, %d): star_addr = 0x%06X, end_addr = 0x%06X, Block Num = %d\n"@0xffffff8009246d95,
 * whose third argument is exactly this byte
 * ("395a6145 ldrb"@0xffffff8008a69948 -> w5). A single inference step.
 */
static u8 ilitek_blk_num;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0ff69c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static u32 ilitek_star_addr;
static u32 ilitek_end_addr;

/*
 * 0xffffff800a0ff6a4. Four bytes ("b906a503 str"@0xffffff8008a6992c,
 * "b946a523 ldr"@0xffffff8008a69ac0). NAME from the format
 * "\x016ILITEK: (%s, %d): New FW ver = 0x%x\n"@0xffffff8009246d6e, whose
 * only argument is the value just written here
 * ("5ac00903 rev"@0xffffff8008a69924 -> w3 -> str).
 */
static u32 ilitek_new_ver;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0ff6a8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static u32 ilitek_hex_tag;

/*
 * hex_to_dec() was reconstructed from the factory kernel disassembly (0xffffff8008a68948).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int hex_to_dec(char *phex, s32 len)
{
	int ret = 0, i = 0, temp = 0, s = 0;

	for (i = 0, s = (len - 1) * 4; i < len; i++, s -= 4) {
		if ((phex[i] >= '0') && (phex[i] <= '9'))
			temp = phex[i] - '0';
		else if ((phex[i] >= 'a') && (phex[i] <= 'f'))
			temp = (phex[i] - 'a') + 10;
		else if ((phex[i] >= 'A') && (phex[i] <= 'F'))
			temp = (phex[i] - 'A') + 10;
		else
			return -1;

		ret |= (temp << s);
	}

	return ret;
}

/*
 * ilitek_tddi_fw_check_hex_hw_crc() was reconstructed from the factory kernel disassembly (0xffffff8009247340).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tddi_fw_check_hex_hw_crc(u8 *pfw)
{
	u32 i, len = 0, hex_crc, hw_crc;

	/* "f1001e9f cmp"@0xffffff8008a6a098 */
	for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
		/* "340007a8 cbz"@0xffffff8008a69fa0 */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		/*
		 * "51000d08 sub"@0xffffff8008a69fa8 -- minus THREE, not minus
		 * four: the last three bytes of the block do not enter the CRC
		 */
		len = ilitek_fw_blk[i].c12 - ilitek_fw_blk[i].c8 - 3;

		hex_crc = calc_crc32(ilitek_fw_blk[i].c8, len, pfw);
		hw_crc = ilitek_tddi_fw_read_hw_crc(ilitek_fw_blk[i].c8, len);

		/* line 225;
		 * "\x016ILITEK: (%s, %d): Block = %d, Hex CRC = %x, HW CRC = %x\n"@0xffffff8009247305 */
		ILI_DBG("Block = %d, Hex CRC = %x, HW CRC = %x\n",
#line 225
			i, hex_crc, hw_crc);

		/* "6b15037f cmp"@0xffffff8008a6a088 */
		if (hex_crc != hw_crc) {
			/* line 228;
			 * "\x013ILITEK: (%s, %d): Hex and HW CRC NO matched !!!\n"@0xffffff8009247360 */
#line 228
			ILI_ERR("Hex and HW CRC NO matched !!!\n");
			return -1;
		}
	}

	/* line 233;
	 * "\x016ILITEK: (%s, %d): Hex and HW CRC match!\n"@0xffffff8009247393 */
#line 233
	ILI_INFO("Hex and HW CRC match!\n");
	return 0;
}

/*
 * ilitek_tddi_fw_check_ver() was reconstructed from the factory kernel disassembly (0xffffff8009246f12).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tddi_fw_check_ver(u8 *pfw)
{
	u32 i, crc;
	u32 flash_crc = 0;
	u32 hw_crc[FW_BLOCK_INFO_NUM];

	/* "f1001e9f cmp"@0xffffff8008a69aa8 */
	for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
		/* "34000395 cbz"@0xffffff8008a69a34 */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		/*
		 * "51000ea0 sub"@0xffffff8008a69a3c -- the three final bytes of the
		 * block plus the fourth, read as one four-byte integer:
		 * the range passed is [c12-3, c12] and the length is 4
		 * ("321e03e3 orr"@0xffffff8008a69a44)
		 */
		if (ilitek_tddi_fw_read_flash_data(ilitek_fw_blk[i].c12 - 3,
						   ilitek_fw_blk[i].c12,
						   (u8 *)&flash_crc, 4) < 0) {
			/* line 661;
			 * "\x013ILITEK: (%s, %d): Read Flash failed\n"@0xffffff8009246eeb */
#line 661
			ILI_ERR("Read Flash failed\n");
			return -1;
		}

		/*
		 * "321e7be9 orr"@0xffffff8008a69a58 = -3, then
		 * "4b1b0129 sub"@0xffffff8008a69a5c and
		 * "0b150121 add"@0xffffff8008a69a60: the length is
		 * c12 - c8 - 3, the same as in `check_hex_hw_crc`
		 */
		hw_crc[i] = ilitek_tddi_fw_read_hw_crc(ilitek_fw_blk[i].c8,
					ilitek_fw_blk[i].c12 -
					ilitek_fw_blk[i].c8 - 3);

		/* line 669;
		 * "\x016ILITEK: (%s, %d): Block = %d, HW CRC = 0x%06x, Flash CRC = 0x%06x\n"@0xffffff8009246f2b */
		ILI_INFO("Block = %d, HW CRC = 0x%06x, Flash CRC = 0x%06x\n",
#line 669
			 i, hw_crc[i], be32_to_cpu(flash_crc));

		/* "6b15039f cmp"@0xffffff8008a69a98 */
		if (be32_to_cpu(flash_crc) != hw_crc[i]) {
			/* line 673;
			 * "\x016ILITEK: (%s, %d): HW and Flash CRC not matched, do upgrade\n"@0xffffff8009246f70 */
#line 673
			ILI_INFO("HW and Flash CRC not matched, do upgrade\n");
			return -1;
		}

		flash_crc = 0;
	}

	/*
	 * line 680. `idev->c48` is read TWICE -- first for the print
	 * ("f9401908 ldr"@0xffffff8008a69ac8 + "b9403104 ldr"@0xffffff8008a69ad8)
	 * and then for the comparison ("f9401908 ldr"@0xffffff8008a69aec +
	 * "b9403108 ldr"@0xffffff8008a69af0) -- because `printk` may write to
	 * memory and the compiler cannot keep it in a register;
	 * "\x016ILITEK: (%s, %d): New FW ver = 0x%x, Current FW ver = 0x%x\n"@0xffffff8009246fae
	 */
	ILI_INFO("New FW ver = 0x%x, Current FW ver = 0x%x\n",
#line 680
		 ilitek_new_ver, ((struct ilitek_g_c48 *)idev->c48)->c48);

	/* "6b08013f cmp"@0xffffff8008a69af4 */
	if (ilitek_new_ver != ((struct ilitek_g_c48 *)idev->c48)->c48) {
		/* line 682;
		 * "\x016ILITEK: (%s, %d): FW version not matched, do upgrade\n"@0xffffff8009246fec */
#line 682
		ILI_INFO("FW version not matched, do upgrade\n");
		return -1;
	}

	/* "f1001e9f cmp"@0xffffff8008a69bf4 */
	for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
		/* "340006e9 cbz"@0xffffff8008a69b14 */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		crc = calc_crc32(ilitek_fw_blk[i].c8,
				 ilitek_fw_blk[i].c12 -
				 ilitek_fw_blk[i].c8 - 3, pfw);

		/*
		 * line 693. The third argument is the HARDWARE CRC saved by the
		 * first loop, the fourth is the one just computed on the file:
		 * "2a1503e4 mov"@0xffffff8008a69bd8 (w4 = hw_crc[i]) and
		 * "2a1b03e5 mov"@0xffffff8008a69bdc (w5 = crc);
		 * "\x016ILITEK: (%s, %d): Block = %d, HW CRC = 0x%06x, Hex CRC = 0x%06x\n"@0xffffff8009247024
		 */
		ILI_INFO("Block = %d, HW CRC = 0x%06x, Hex CRC = 0x%06x\n",
#line 693
			 i, hw_crc[i], crc);

		/* "6b15037f cmp"@0xffffff8008a69be4 */
		if (crc != hw_crc[i]) {
			/* line 695;
			 * "\x013ILITEK: (%s, %d): Hex and HW CRC not matched, do upgrade\n"@0xffffff8009247067 */
#line 695
			ILI_ERR("Hex and HW CRC not matched, do upgrade\n");
			return -1;
		}
	}

	/* line 700;
	 * "\x016ILITEK: (%s, %d): Firmware is the newest version, upgrade abort\n"@0xffffff80092470a3 */
#line 700
	ILI_INFO("Firmware is the newest version, upgrade abort\n");
	return 0;
}

/*
 * ilitek_tddi_fw_flash_program() was reconstructed from the factory kernel disassembly (0xffffff8009247127).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tddi_fw_flash_program(u8 *pfw)
{
	u8 buf[512];
	u32 i, addr, k;
	int percent;
	bool skip = true;

	/*
	 * "321703e2 orr"@0xffffff8008a69ce4 = 0x200 = 512, and
	 * "2a1f03e1 mov"@0xffffff8008a69ce8 = 0: it is an explicit `memset`,
	 * not an initialiser -- the array is cleared INSIDE the body, after
	 * the caller's `bl ilitek_tddi_fw_flash_erase`
	 */
	memset(buf, 0, sizeof(buf));

	/* "f1001e9f cmp"@0xffffff8008a69f78 */
	for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
		/*
		 * "b840cf05 ldr"@0xffffff8008a69d08 -- pre-indexed: the
		 * binary keeps &ilitek_fw_blk[i].c12 live and derives c8 from it with
		 * a second pre-indexed one ("b8408f64 ldr"@0xffffff8008a69d10)
		 */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		if (ilitek_fw_blk[i].c12 / 8192 <= 14 &&
		    ilitek_fw_blk[i].c8 / 4096 > 28)
			continue;

		/* line 718;
		 * "\x016ILITEK: (%s, %d): Block[%d]: Programing from (0x%x) to (0x%x)\n"@0xffffff80092470e6 */
		ILI_INFO("Block[%d]: Programing from (0x%x) to (0x%x)\n",
#line 718
			 i, ilitek_fw_blk[i].c8, ilitek_fw_blk[i].c12);

		/* "6b08037f cmp"@0xffffff8008a69d50 + "540010e2 b.cs"@0xffffff8008a69d54
		 * -- UNSIGNED guard */
		for (addr = ilitek_fw_blk[i].c8; addr < ilitek_fw_blk[i].c12;
		     addr += idev->c548) {
			/*
			 * "528104aa mov"@0xffffff8008a69d64 +
			 * "72a0820a movk"@0xffffff8008a69d68 +
			 * "b90033ea str"@0xffffff8008a69d6c: 0x04100825
			 * written in ONE four-byte `str`, which is how
			 * clang merges four consecutive constant `strb`s
			 */
			buf[0] = 0x25;
			buf[1] = 0x08;
			buf[2] = 0x10;
			buf[3] = 0x04;

			/*
			 * "eb0c039f cmp"@0xffffff8008a69db4 -- the limit is
			 * RE-READ from idev on every round
			 * ("b942250c ldr"@0xffffff8008a69da4), not hoisted into
			 * a local: it is defect class A2 the other way round,
			 * and it has to be left as it is
			 */
			for (k = 0; k < idev->c548; k++) {
				/* "6b09017f cmp"@0xffffff8008a69d88 +
				 * "54000069 b.ls"@0xffffff8008a69d8c */
				if ((addr + k) <= ilitek_end_addr)
					buf[4 + k] = pfw[addr + k];
				else
					/* "32001feb orr"@0xffffff8008a69d90 */
					buf[4 + k] = 0xFF;

				/* "7103fd7f cmp"@0xffffff8008a69da8 +
				 * "1a9f17eb cset"@0xffffff8008a69dac +
				 * "0a0b02b5 and"@0xffffff8008a69db8 */
				if (buf[4 + k] != 0xFF)
					skip = false;
			}

			/* "36000095 tbz"@0xffffff8008a69dc0 */
			if (skip) {
				if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
					/* line 738;
					 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
					ILI_ERR("Write cs high failed\n");
				return -EFW_PROGRAM;
			}

			ilitek_tddi_flash_write_enable();

			if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
				/* line 745;
				 * "\x013ILITEK: (%s, %d): Write cs low failed\n"@0xffffff80092461b2 */
#line 745
				ILI_ERR("Write cs low failed\n");

			/* "52954aa1 mov"@0xffffff8008a69e0c +
			 * "72a00cc1 movk"@0xffffff8008a69e14 = 0x66AA55 */
			if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
				/* line 748;
				 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 748
				ILI_ERR("Write key failed\n");

			if (ilitek_ice_mode_write(0x41008, 0x2, 1) < 0)
				/* line 751;
				 * "\x013ILITEK: (%s, %d): Write 0x2 failed\n"@0xffffff8009247144 */
#line 751
				ILI_ERR("Write 0x2 failed\n");

			/* "12181f61 and"@0xffffff8008a69e74,
			 * "33105f61 bfxil"@0xffffff8008a69e78,
			 * "33101f61 bfi"@0xffffff8008a69e7c -- the same three
			 * swapped bytes of `ilitek_tddi_fw_flash_erase` */
			if (ilitek_ice_mode_write(0x41008,
						  ((addr & 0xFF0000) >> 16) |
						  (addr & 0x00FF00) |
						  ((addr & 0x0000FF) << 16),
						  3) < 0)
				/* line 755;
				 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 755
				ILI_ERR("Write address failed\n");

			/*
			 * "11001121 add"@0xffffff8008a69ebc -- four bytes of
			 * header plus the page
			 */
			if (idev->c776(buf, idev->c548 + 4) < 0) {
				/* line 759 */
				ILI_ERR("Failed to program data at start_addr = 0x%X, k = 0x%X, addr = 0x%x\n",
#line 759
					addr, k, addr + k);

				if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
					/* line 761 */
					ILI_ERR("Write cs high failed\n");
				return -EFW_PROGRAM;
			}

			if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
				/* line 766 */
#line 766
				ILI_ERR("Write cs high failed\n");

			/*
			 * "79444108 ldrh"@0xffffff8008a69efc + "7103bd1f cmp"@0xffffff8008a69f00
			 * -- the field the format at 0xffffff8009246666 calls
			 * "Flash MID", compared with 0xEF
			 */
			if (idev->c544 == 0xEF)
				/*
				 * "52912b00 mov"@0xffffff8008a69f08 +
				 * "72a00820 movk"@0xffffff8008a69f0c = 0x418958
				 * = 1000 * 0x10c7. Divergence G5: `mdelay(1)` and
				 * `udelay(1000)` give the same instruction
				 */
				mdelay(1);
			else
				/* "52800140 mov"@0xffffff8008a69f18 = 10 */
				if (ilitek_tddi_flash_poll_busy(10) < 0)
					return -EFW_PROGRAM;

			/*
			 * "52800caa mov"@0xffffff8008a69f30 = 101,
			 * "1b0a7f6a mul"@0xffffff8008a69f34,
			 * "1ac90949 udiv"@0xffffff8008a69f3c: the division is
			 * UNSIGNED, but the clamp at 90
			 * ("7101693f cmp"@0xffffff8008a69f40 +
			 * "1a8ab129 csel"@0xffffff8008a69f48) is SIGNED --
			 * which is what one gets by assigning the quotient
			 * to an `int` and comparing that
			 */
			percent = (addr * 101) / ilitek_end_addr;
			idev->c444 = (percent < 90) ? percent : 90;

			/*
			 * "2a1f03f5 mov"@0xffffff8008a69f58 -- see the header:
			 * `skip` is cleared HERE, not at the start of the page
			 */
			skip = false;
		}
	}

	return 0;
}

/*
 * ilitek_tddi_fw_flash_upgrade() was reconstructed from the factory kernel disassembly (0xffffff8009246e19).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tddi_fw_flash_upgrade(u8 *pfw)
{
	int ret = 0;

	/*
	 * "b9426500 ldr"@0xffffff8008a699c0 -- the field at +612, which
	 * `ilitek.h` documents as the reset mode
	 */
	ret = ilitek_tddi_reset_ctrl(idev->c612);
	if (ret < 0) {
		/* line 849;
		 * "\x013ILITEK: (%s, %d): TP reset failed during flash progam\n"@0xffffff8009246de0 */
#line 849
		ILI_ERR("TP reset failed during flash progam\n");
		return -EFW_REST;
	}

	idev->c636 = false;
	/*
	 * "36f800a0 tbz"@0xffffff8008a699e0 -- the return value is tested but
	 * the error does NOT stop anything: it prints and carries on
	 */
	if (ilitek_tddi_ic_get_fw_ver() < 0)
		/* line 856;
		 * "\x013ILITEK: (%s, %d): Get firmware ver failed before upgrade\n"@0xffffff8009246e36 */
#line 856
		ILI_ERR("Get firmware ver failed before upgrade\n");
	idev->c636 = true;

	ret = ilitek_ice_mode_ctrl(true, false);
	if (ret < 0)
		/* "12800e54 mov"@0xffffff8008a69c80 = -115 */
		return -EFW_ICE_MODE;

	ret = ilitek_tddi_ic_watch_dog_ctrl(true, false);
	if (ret < 0)
		/* "12800e74 mov"@0xffffff8008a69c88 = -116 */
		return -EFW_WDT;

	/*
	 * "36f838a0 tbz"@0xffffff8008a69c24 -- if the firmware is already the
	 * newest we leave here, without programming anything
	 */
	if (ilitek_tddi_fw_check_ver(pfw) == 0)
		goto out;

	/*
	 * "3100041f cmn"@0xffffff8008a69cd8 -- the comparison is with EXACTLY -1,
	 * not with "less than zero": `ilitek_tddi_fw_flash_erase` returns
	 * only 0 or -1
	 */
	if (ilitek_tddi_fw_flash_erase() == -1)
		/* "12800ed4 mov"@0xffffff8008a6a0e0 = -119 */
		return -EFW_ERASE;

	ret = ilitek_tddi_fw_flash_program(pfw);
	if (ret < 0)
		return ret;

	/* "54ffc841 b.ne"@0xffffff8008a6a090 -> "12800e94 mov"@0xffffff8008a699b0
	 * = -117 */
	if (ilitek_tddi_fw_check_hex_hw_crc(pfw) < 0)
		return -EFW_CRC;

	ret = ilitek_tddi_reset_ctrl(idev->c612);
	if (ret < 0) {
		/* line 894;
		 * "\x013ILITEK: (%s, %d): TP reset failed after flash progam\n"@0xffffff8009246eb3 */
		ILI_ERR("TP reset failed after flash progam\n");
		return -EFW_REST;
	}

	return 0;

out:
	ret = ilitek_ice_mode_ctrl(false, false);
	if (ret < 0) {
		/* line 870;
		 * "\x013ILITEK: (%s, %d): Disable ice mode failed, call reset instead\n"@0xffffff8009246e72 */
#line 870
		ILI_ERR("Disable ice mode failed, call reset instead\n");

		if (ilitek_tddi_reset_ctrl(idev->c612) < 0) {
			/*
			 * line 872. THE MESSAGE IS THE ONE FROM LINE 849,
			 * "during" and not "after", and it is a factory
			 * defect that is reproduced (rule 7): the two
			 * "91378000 add"@0xffffff8008a69c54 and
			 * "91378000 add"@0xffffff8008a69c6c both lead
			 * to 0xffffff8009246de0, while line 894 uses a
			 * DIFFERENT literal, 0xffffff8009246eb3
			 * ("913acc00 add"@0xffffff8008a6a0d8), which says
			 * "after".
			 */
#line 872
			ILI_ERR("TP reset failed during flash progam\n");
			return -EFW_REST;
		}
	}

	return 0;
}

/*
 * ilitek_tddi_fw_update_block_info() was reconstructed from the factory kernel disassembly (0xffffff8009246d4d).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ilitek_tddi_fw_update_block_info(u8 *pfw)
{
	u32 index;

	ilitek_fw_blk[1].c0 = "AP";
	ilitek_fw_blk[2].c0 = "DATA";
	ilitek_fw_blk[3].c0 = "TUNING";
	ilitek_fw_blk[4].c0 = "GESTURE";
	ilitek_fw_blk[5].c0 = "MP";

	ilitek_fw_blk[1].c28 = 1;
	ilitek_fw_blk[2].c28 = 1;
	ilitek_fw_blk[3].c28 = 1;
	ilitek_fw_blk[4].c28 = 4;
	ilitek_fw_blk[5].c28 = 5;

	/* "b9400d0a ldr"@0xffffff8008a6979c reads ilitek_fw_blk[1].c12 e
	 * "51013d53 sub"@0xffffff8008a697bc subtracts 0x4f = 79 */
	index = ilitek_fw_blk[1].c12 - 79;

	/* line 941;
	 * "\x016ILITEK: (%s, %d): Parsing hex info start addr = 0x%x\n"@0xffffff8009246d15 */
#line 941
	ILI_INFO("Parsing hex info start addr = 0x%x\n", index);

	/*
	 * 75 bytes, and the extension is given by the nine eight-byte `str`s plus
	 * the overlapping four-byte tail: "a940292b ldp"@0xffffff8008a697f0
	 * (offset 0), ..., "b844712a ldur"@0xffffff8008a6982c (offset 71).
	 * 71 + 4 = 75. The target is idev + 0x1c4 = 452
	 * ("9107110c add"@0xffffff8008a697f4).
	 */
	memcpy(idev->c452, pfw + index, 75);

	/*
	 * "5280804a mov"@0xffffff8008a6985c + "72a0002a movk"@0xffffff8008a69860
	 * = 0x10402, and the comparison is UNSIGNED
	 * ("540003c9 b.ls"@0xffffff8008a6986c). The field is at +52 of the block
	 * pointed to by `idev->c48` ("b9403529 ldr"@0xffffff8008a69864).
	 */
	if (((struct ilitek_g_c48 *)idev->c48)->c52 > 0x010402)
		/*
		 * "1a9f07ea cset"@0xffffff8008a69878 -- the byte is 0 or 1, not
		 * the value read
		 */
		idev->c641 = (idev->c452[0] != 0);
	else
		/* "390a051f strb"@0xffffff8008a698e4 */
		idev->c641 = 0;

	/*
	 * line 945. The two strings are "ON"@0xffffff8009250b5c and
	 * "OFF"@0xffffff8009250b6d, picked with two separate `adrp`+`add` pairs and
	 * not with a `csel`, because the branch was already split;
	 * "\x016ILITEK: (%s, %d): Transfer touch coordinate = %s\n"@0xffffff8009241619
	 */
	ILI_INFO("Transfer touch coordinate = %s\n",
#line 945
		 idev->c641 ? "ON" : "OFF");

	/*
	 * "b941f508 ldr"@0xffffff8008a6991c reads the four bytes at idev+500
	 * -- which live inside `c452[75]`, at index 48 -- and
	 * "5ac00903 rev"@0xffffff8008a69924 reverses them: it is a 32-bit
	 * BIG-ENDIAN integer inside the block just copied from the file
	 */
	ilitek_new_ver = (idev->c452[48] << 24) | (idev->c452[49] << 16) |
			 (idev->c452[50] << 8) | idev->c452[51];

	/* line 952;
	 * "\x016ILITEK: (%s, %d): New FW ver = 0x%x\n"@0xffffff8009246d6e */
#line 952
	ILI_INFO("New FW ver = 0x%x\n", ilitek_new_ver);

	/*
	 * line 953. `star_addr` is spelt that way IN THE BINARY, without the `t`,
	 * and it is reproduced;
	 * "\x016ILITEK: (%s, %d): star_addr = 0x%06X, end_addr = 0x%06X, Block Num = %d\n"@0xffffff8009246d95
	 */
	ILI_INFO("star_addr = 0x%06X, end_addr = 0x%06X, Block Num = %d\n",
#line 953
		 ilitek_star_addr, ilitek_end_addr, ilitek_blk_num);
}

/*
 * ilitek_tddi_fw_ili_convert() was reconstructed from the factory kernel disassembly (0xffffff8009246c37).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tddi_fw_ili_convert(u8 *pfw)
{
	u32 i, addr, start, end, size, blk;
	u8 type;

	if (ERR_ALLOC_MEM((void *)idev->c696))
		return -1;

	ilitek_ilifile = (u8 *)idev->c696;

	/*
	 * "51010133 sub"@0xffffff8008a6a26c -- minus 0x40 = 64, which is
	 * the ILI file header, and
	 * "528ff809 mov"@0xffffff8008a6a270 + "72a00049 movk"@0xffffff8008a6a274
	 * = 0x27fc0 = MAX_HEX_FILE_SIZE - 64
	 */
	size = idev->c648 - 64;
	if (size > (MAX_HEX_FILE_SIZE - 64)) {
		/* line 969;
		 * "\x013ILITEK: (%s, %d): size of ILI file is invalid\n"@0xffffff8009246c06 */
#line 969
		ILI_ERR("size of ILI file is invalid\n");
		return -1;
	}

	/* "7103fd3f cmp"@0xffffff8008a6a378 and the three twins */
	if (ilitek_ilifile[22] != 0xFF && ilitek_ilifile[23] != 0xFF &&
	    ilitek_ilifile[24] != 0xFF && ilitek_ilifile[25] != 0xFF) {
		/* line 975;
		 * "\x013ILITEK: (%s, %d): Invaild ILI format, abort!\n"@0xffffff8009246c52 */
#line 975
		ILI_ERR("Invaild ILI format, abort!\n");
		return -1;
	}

	/*
	 * line 979. The two arguments are bytes 32 and 33
	 * ("39408103 ldrb"@0xffffff8008a6a3bc, "39408504 ldrb"@0xffffff8008a6a3c0);
	 * "\x016ILITEK: (%s, %d): Start to parse ILI file, type = %d, block_count = %d\n"@0xffffff8009246c82
	 */
	ILI_INFO("Start to parse ILI file, type = %d, block_count = %d\n",
#line 979
		 ilitek_ilifile[32], ilitek_ilifile[33]);

	/* "b9069d1f str"@0xffffff8008a6a3f0 */
	ilitek_star_addr = 0;
	memset(ilitek_fw_blk, 0x0, sizeof(ilitek_fw_blk));
	/* "b906a2df str"@0xffffff8008a6a430 */
	ilitek_end_addr = 0;
	/* "b906a91f str"@0xffffff8008a6a434 */
	ilitek_hex_tag = 0;

	/*
	 * "3940813b ldrb"@0xffffff8008a6a438 -- a SINGLE read, and the value
	 * stays in w27, a callee-saved register, across all the `printk`s of the
	 * loop ("6a1b011f tst"@0xffffff8008a6a470 and
	 * "36380a1b tbz"@0xffffff8008a6a568 reuse it). An `ilitek_ilifile[32]`
	 * re-read inside the loop would force the compiler to reload it after
	 * every `printk`: so in the factory source it is a LOCAL.
	 * NAME from the line 979 format, which calls it "type".
	 */
	type = ilitek_ilifile[32];

	/* "3400133b cbz"@0xffffff8008a6a43c */
	if (type) {
		/*
		 * "528015e9 mov"@0xffffff8008a6a450 = 0xAF, written BEFORE the
		 * loop ("b906a909 str"@0xffffff8008a6a468)
		 */
		ilitek_hex_tag = 0xAF;

		/* "f1001f9f cmp"@0xffffff8008a6a550 */
		for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
			/* "1adc22c8 lsl"@0xffffff8008a6a46c +
			 * "6a1b011f tst"@0xffffff8008a6a470 */
			if (!((1 << i) & type))
				continue;

			/* "f1001b9f cmp"@0xffffff8008a6a478 +
			 * "54001322 b.cs"@0xffffff8008a6a47c */
			if (i + 1 > FW_BLOCK_INFO_NUM - 1) {
				/*
				 * line 1001. The second argument is the
				 * constant 6 ("321f07e4 orr"@0xffffff8008a6a6f8);
				 * "\x013ILITEK: (%s, %d): ERROR! block num is larger than its define (%d, %d)\n"@0xffffff8009246a42
				 */
				ILI_ERR("ERROR! block num is larger than its define (%d, %d)\n",
#line 1001
					i + 1, FW_BLOCK_INFO_NUM - 1);
				return -1;
			}

			if (i == 5) {
				start = (ilitek_ilifile[0] << 16) |
					(ilitek_ilifile[1] << 8) |
					 ilitek_ilifile[2];
				end   = (ilitek_ilifile[3] << 16) |
					(ilitek_ilifile[4] << 8) |
					 ilitek_ilifile[5];
			} else {
				start = (ilitek_ilifile[34 + i * 6] << 16) |
					(ilitek_ilifile[35 + i * 6] << 8) |
					 ilitek_ilifile[36 + i * 6];
				end   = (ilitek_ilifile[37 + i * 6] << 16) |
					(ilitek_ilifile[38 + i * 6] << 8) |
					 ilitek_ilifile[39 + i * 6];
			}

			/*
			 * "b81f0304 stur"@0xffffff8008a6a4e0 (c8),
			 * "b81f4305 stur"@0xffffff8008a6a508 (c12),
			 * "b9000314 str"@0xffffff8008a6a4fc (c24): all three
			 * BEFORE the comparison "6b0400bf cmp"@0xffffff8008a6a504
			 */
			ilitek_fw_blk[i + 1].c8 = start;
			ilitek_fw_blk[i + 1].c12 = end;
			ilitek_fw_blk[i + 1].c24 = 0x7FFFFFFF;

			/* "540001e0 b.eq"@0xffffff8008a6a50c */
			if (start == end)
				continue;

			/*
			 * "4b0402c8 sub"@0xffffff8008a6a510 +
			 * "0b050108 add"@0xffffff8008a6a518 = 1 - start + end,
			 * stored AFTER the comparison
			 * ("b81f8308 stur"@0xffffff8008a6a52c)
			 */
			ilitek_fw_blk[i + 1].c16 = end - start + 1;

			/* line 1019;
			 * "\x016ILITEK: (%s, %d): Block[%d]: start_addr = %x, end = %x\n"@0xffffff8009246ccc */
			ILI_INFO("Block[%d]: start_addr = %x, end = %x\n",
#line 1019
				 i + 1, start, end);

			/*
			 * "f100e67f cmp"@0xffffff8008a6a534 compares the index
			 * with 57 = 39 + 6*3, that is i == 3, and the resulting
			 * block is the fourth -- the same one
			 * `ilitek_tddi_fw_hex_convert` marks with
			 * "710012ff cmp"@0xffffff8008a69434
			 */
			if (i == 3)
				/* "3909f916 strb"@0xffffff8008a6a544 */
				idev->c638 = 1;
		}

		/*
		 * "36380a1b tbz"@0xffffff8008a6a568 -- bit 7 of byte 32,
		 * which the loop above has not used
		 */
		if (type & 0x80) {
			/*
			 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a6a5b0).
			 *
			 * The working notes -- the disassembly citations, the measurements against
			 * the factory binary and the reasoning behind each choice -- are in
			 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
			 * in the oracolo repository. They are kept in Italian, as the project's
			 * internal record.
			 */
			for (i = 0; i < 3; i++) {
				/* "34000223 cbz"@0xffffff8008a6a574 */
				blk = ilitek_ilifile[9 + i * 4];
				if (!blk)
					continue;

				addr = (ilitek_ilifile[6 + i * 4] << 16) |
				       (ilitek_ilifile[7 + i * 4] << 8) |
					ilitek_ilifile[8 + i * 4];

				/* "34000144 cbz"@0xffffff8008a6a590 */
				if (!addr)
					continue;

				/* "b9001904 str"@0xffffff8008a6a5ac */
				ilitek_fw_blk[blk].c24 = addr;

				/* line 1033;
				 * "\x016ILITEK: (%s, %d): Tag 0xB0: change Block[%d] to addr = 0x%x\n"@0xffffff8009246adf */
				ILI_INFO("Tag 0xB0: change Block[%d] to addr = 0x%x\n",
					 blk, addr);
			}
		}
	} else {
		/* "528015c9 mov"@0xffffff8008a6a6a0 = 0xAE */
		ilitek_hex_tag = 0xAE;
	}

	/*
	 * memcpy() was reconstructed from the factory kernel disassembly (0xffffff8008a6a6b4).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	memcpy(pfw, ilitek_ilifile + 64, size);

	if (ilitek_fw_calc_file_crc(pfw) < 0)
		return -1;

	/*
	 * "b906a2d3 str"@0xffffff8008a6a6d4 and
	 * "391a6128 strb"@0xffffff8008a6a6d8, with byte 33 of the file
	 * ("39408508 ldrb"@0xffffff8008a6a6d0). `ilitek_star_addr` is NOT
	 * rewritten here: it keeps the zero from above.
	 */
	ilitek_end_addr = size;
	ilitek_blk_num = ilitek_ilifile[33];

	return 0;
}

/*
 * ilitek_tddi_fw_hex_convert() was reconstructed from the factory kernel disassembly (0xffffff8009246a8b).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tddi_fw_hex_convert(u8 *phex, u32 size, u8 *pfw)
{
	u32 i = 0, j = 0, k = 0, num = 0;
	u32 len = 0, addr = 0, type = 0;
	u32 start_addr = 0, end_addr = 0, ex_addr = 0;
	u32 nowAddr, offset, block_count = 0;

	memset(ilitek_fw_blk, 0x0, sizeof(ilitek_fw_blk));

	/*
	 * "6b1702bf cmp"@0xffffff8008a68f34 + "54ffd023 b.cc"@0xffffff8008a68f38
	 * -- an UNSIGNED guard, and the loop is skipped entirely when
	 * `size` is zero ("34006bd7 cbz"@0xffffff8008a6890c).
	 *
	 * IT HAS NO INCREMENT OF ITS OWN, and that is measured: "0b080135 add"@0xffffff8008a68f30
	 * is the ONLY write of w21 in the body, and it adds to the `w8` that already
	 * holds `i + 11 + len*2` only the `w9` that is 1 or 2. A `for (; i < size; i++)`
	 * with `i = offset` at the tail would give `+2` or `+3`, that is one character
	 * too many per record.
	 */
	while (i < size) {
		len = hex_to_dec(&phex[i + 1], 2);
		addr = hex_to_dec(&phex[i + 3], 4);
		type = hex_to_dec(&phex[i + 7], 2);

		/* "710012df cmp"@0xffffff8008a68b8c */
		if (type == 0x04) {
			ex_addr = hex_to_dec(&phex[i + 9], 4);
		/* "71000adf cmp"@0xffffff8008a68b84 */
		} else if (type == 0x02) {
			/*
			 * "530c7d04 lsr"@0xffffff8008a68e50 -- UNSIGNED.
			 * Written as a single expression starting from the
			 * `int` return of `hex_to_dec` it would be an `asr`: it is
			 * the passage through the `u32` variable that decides the
			 * mnemonic, and the binary says which of the two it is
			 */
			ex_addr = hex_to_dec(&phex[i + 9], 4);
			ex_addr = ex_addr >> 12;
		/* "320002c8 orr"@0xffffff8008a68bd4 + "7102bd1f cmp"@0xffffff8008a68bd8
		 * -- (type | 1) == 0xAF is how clang writes
		 * `type == 0xAE || type == 0xAF` */
		} else if (type == 0xAE || type == 0xAF) {
			/* "b906a916 str"@0xffffff8008a68bec */
			ilitek_hex_tag = type;

			/* "2a0203f7 mov"@0xffffff8008a68be8 -- the assignment
			 * is UNCONDITIONAL and precedes the comparison
			 * "7102bedf cmp"@0xffffff8008a68be0 */
			num = block_count;
			if (type == 0xAF)
				num = hex_to_dec(&phex[i + 21], 2);

			/*
			 * "71001eff cmp"@0xffffff8008a69000 +
			 * "5400b962 b.cs"@0xffffff8008a69004 -- it is written
			 * `>= 7`, not `> 6`: the twin at line 1099 carries
			 * the other form, and the two encodings differ
			 */
			if (num >= FW_BLOCK_INFO_NUM) {
				/* line 1080;
				 * "\x013ILITEK: (%s, %d): ERROR! block num is larger than its define (%d, %d)\n"@0xffffff8009246a42 */
				ILI_ERR("ERROR! block num is larger than its define (%d, %d)\n",
					num, FW_BLOCK_INFO_NUM - 1);
				return -1;
			}

			/* "b9000944 str"@0xffffff8008a69230 (c8),
			 * "2901a505 stp"@0xffffff8008a69428 (c12 e c16 in one
			 * single pair), "b900190a str"@0xffffff8008a69424 (c24) */
			ilitek_fw_blk[num].c8 = hex_to_dec(&phex[i + 9], 6);
			ilitek_fw_blk[num].c12 = hex_to_dec(&phex[i + 15], 6);
			ilitek_fw_blk[num].c16 = ilitek_fw_blk[num].c12 -
						 ilitek_fw_blk[num].c8 + 1;
			ilitek_fw_blk[num].c24 = 0x7FFFFFFF;

			/*
			 * line 1088. THE FORMAT DOES NOT END WITH `\n`, and it is not
			 * a transcription slip: the literal at
			 * 0xffffff8009246aa6 is 56 bytes long and the last is 'x';
			 * "\x016ILITEK: (%s, %d): Block[%d]: start_addr = %x, end = %x"@0xffffff8009246aa6
			 */
			ILI_INFO("Block[%d]: start_addr = %x, end = %x",
				 num, ilitek_fw_blk[num].c8,
#line 1088
				 ilitek_fw_blk[num].c12);

			/* "710012ff cmp"@0xffffff8008a69434 +
			 * "3909f906 strb"@0xffffff8008a69444 */
			if (num == 4)
				idev->c638 = 1;

			/* "11000782 add"@0xffffff8008a6944c */
			block_count++;
		/* "7102c2df cmp"@0xffffff8008a68c3c + "b946a908 ldr"@0xffffff8008a68c48
		 * + "7102bd1f cmp"@0xffffff8008a68c4c */
		} else if (type == 0xB0 && ilitek_hex_tag == 0xAF) {
			num = hex_to_dec(&phex[i + 15], 2);

			/*
			 * "7100187f cmp"@0xffffff8008a69190 +
			 * "5400abc8 b.hi"@0xffffff8008a69194 -- `> 6`, the twin
			 * form of the one at line 1080
			 */
			if (num > FW_BLOCK_INFO_NUM - 1) {
				/* line 1099 */
				ILI_ERR("ERROR! block num is larger than its define (%d, %d)\n",
					num, FW_BLOCK_INFO_NUM - 1);
				return -1;
			}

			/* "b9001904 str"@0xffffff8008a69604 */
			ilitek_fw_blk[num].c24 = hex_to_dec(&phex[i + 9], 6);

			/* line 1104;
			 * "\x016ILITEK: (%s, %d): Tag 0xB0: change Block[%d] to addr = 0x%x\n"@0xffffff8009246adf */
			ILI_INFO("Tag 0xB0: change Block[%d] to addr = 0x%x\n",
				 num, ilitek_fw_blk[num].c24);
		}

		/* "0b044363 add"@0xffffff8008a68e68 */
		nowAddr = addr + (ex_addr << 16);

		/*
		 * "38684b49 ldrb"@0xffffff8008a68e64 -- the character right
		 * after the record, read BEFORE the bound check
		 */
		if (phex[i + 1 + 2 + 4 + 2 + len * 2 + 2] == 0x0D)
			offset = i + 1 + 2 + 4 + 2 + len * 2 + 2 + 2;
		else
			offset = i + 1 + 2 + 4 + 2 + len * 2 + 2 + 1;

		/* "6b10007f cmp"@0xffffff8008a68e74 +
		 * "540043e8 b.hi"@0xffffff8008a68e78 -- UNSIGNED */
		if (nowAddr > MAX_HEX_FILE_SIZE) {
			/* line 1115;
			 * "\x013ILITEK: (%s, %d): Invalid hex format %d\n"@0xffffff8009246b1e */
#line 1115
			ILI_ERR("Invalid hex format %d\n", nowAddr);
			return -1;
		}

		/* "350005b6 cbnz"@0xffffff8008a68e7c */
		if (type == 0x00) {
			/*
			 * see the header: this `if` never fires, and is
			 * reproduced
			 */
			if (nowAddr < start_addr)
				start_addr = nowAddr;

			/*
			 * "0b180074 add"@0xffffff8008a68e84 -- a plain
			 * assignment, not a maximum
			 */
			end_addr = nowAddr + len;

			/*
			 * "34000538 cbz"@0xffffff8008a68e8c -- the inner loop
			 * is skipped entirely when `len` is zero, and the
			 * round count is ((len*2 - 1) >> 1) + 1
			 * ("5100054a sub"@0xffffff8008a68e90 +
			 * "53017d4a lsr"@0xffffff8008a68e94 +
			 * "1100054a add"@0xffffff8008a68e98), which is how clang
			 * counts a `for` with stride 2
			 */
			for (j = 0, k = 0; j < len * 2; j += 2, k++)
				pfw[nowAddr + k] = hex_to_dec(&phex[i + 9 + j], 2);
		}

		i = offset;
	}

	if (ilitek_fw_calc_file_crc(pfw) < 0)
		return -1;

	/* "b9069d1c str"@0xffffff8008a696b8, "b906a136 str"@0xffffff8008a696bc,
	 * "391a6154 strb"@0xffffff8008a696c0 */
	ilitek_star_addr = start_addr;
	ilitek_end_addr = end_addr;
	ilitek_blk_num = block_count;

	return 0;
}

/*
 * ilitek_tdd_fw_hex_open() was reconstructed from the factory kernel disassembly (0xffffff8009246813).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tdd_fw_hex_open(u8 open_file_method, u8 *pfw)
{
	int ret = 0, fsize = 1;
	const struct firmware *fw = NULL;
	struct file *f = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	/*
	 * line 1149. The two `%s`es are picked with two `csel`s
	 * ("9a891149 csel"@0xffffff8008a685b8 for the path and
	 * "9a881123 csel"@0xffffff8008a685dc for the method name), so they are
	 * two ternary operators and not two separate branches;
	 * "\x016ILITEK: (%s, %d): Open file method = %s, path = %s\n"@0xffffff80092467dd,
	 * "FILP_OPEN"@0xffffff800924682a, "REQUEST_FIRMWARE"@0xffffff8009246834
	 */
	ILI_INFO("Open file method = %s, path = %s\n",
		 open_file_method ? "FILP_OPEN" : "REQUEST_FIRMWARE",
		 open_file_method ? (const char *)idev->c664 :
#line 1149
				    (const char *)idev->c672);

	switch (open_file_method) {
	case REQUEST_FIRMWARE:
		/* "f9415101 ldr"@0xffffff8008a68608 (+672, the name) and
		 * "f9400d02 ldr"@0xffffff8008a6860c (+24, the device) */
		if (request_firmware(&fw, (const char *)idev->c672,
				     (struct device *)idev->c24) < 0) {
			/* line 1154;
			 * "\x013ILITEK: (%s, %d): Request firmware failed, try again\n"@0xffffff8009246845 */
#line 1154
			ILI_ERR("Request firmware failed, try again\n");

			if (request_firmware(&fw, (const char *)idev->c672,
					     (struct device *)idev->c24) < 0) {
				/* line 1156;
				 * "\x013ILITEK: (%s, %d): Request firmware failed after retry\n"@0xffffff800924687d */
#line 1156
				ILI_ERR("Request firmware failed after retry\n");
				ret = -1;
				goto out;
			}
		}

		/* "f9400114 ldr"@0xffffff8008a68658 */
		fsize = fw->size;
		/* line 1163;
		 * "\x016ILITEK: (%s, %d): fsize = %d\n"@0xffffff80092468b6 */
#line 1163
		ILI_INFO("fsize = %d\n", fsize);

		/* "7100029f cmp"@0xffffff8008a6866c + "54007f8d b.le"@0xffffff8008a68670
		 * -- SIGNED */
		if (fsize <= 0) {
			/* line 1165;
			 * "\x013ILITEK: (%s, %d): The size of file is zero\n"@0xffffff80092468d6 */
#line 1165
			ILI_ERR("The size of file is zero\n");
			release_firmware(fw);
			ret = -1;
			goto out;
		}

		idev->c288 = 0;
		/*
		 * "93407e94 sxtw"@0xffffff8008a68678 -- `fsize` is SIGN-extended
		 * before becoming the size of the `vmalloc`
		 */
		idev->c296 = (u64)vmalloc(fsize);
		if (ERR_ALLOC_MEM((void *)idev->c296)) {
			/* line 1174;
			 * "\x013ILITEK: (%s, %d): Failed to allocate tp_fw by vmalloc, try again\n"@0xffffff8009246904 */
#line 1174
			ILI_ERR("Failed to allocate tp_fw by vmalloc, try again\n");
			idev->c296 = (u64)vmalloc(fsize);
			if (ERR_ALLOC_MEM((void *)idev->c296)) {
				/* line 1177;
				 * "\x013ILITEK: (%s, %d): Failed to allocate tp_fw after retry\n"@0xffffff8009246948 */
#line 1177
				ILI_ERR("Failed to allocate tp_fw after retry\n");
				release_firmware(fw);
				ret = -ENOMEM;
				goto out;
			}
		}

		/*
		 * "f9400501 ldr"@0xffffff8008a686d8 -- field +8 of
		 * `struct firmware`, that is `data`
		 */
		memcpy((u8 *)idev->c296, fw->data, fsize);
		/* "f9009114 str"@0xffffff8008a686e4 */
		idev->c288 = fsize;
		release_firmware(fw);
		break;
	case FILP_OPEN:
		/*
		 * "52803482 mov"@0xffffff8008a68744 = 0x1a4 = 420 decimal,
		 * which is 644 in OCTAL written without the leading zero: the
		 * factory source passes 644, and those are the wrong permissions
		 * for an `open` for reading. Reproduced.
		 */
		f = filp_open((const char *)idev->c664, O_RDONLY, 644);
		if (ERR_ALLOC_MEM(f)) {
			/*
			 * line 1192. The argument is the pointer itself, not
			 * `PTR_ERR`: "aa1403e3 mov"@0xffffff8008a69654 passes x20
			 * as it stands;
			 * "\x013ILITEK: (%s, %d): Failed to open the file at %ld\n"@0xffffff8009246982
			 */
#line 1192
			ILI_ERR("Failed to open the file at %ld\n", (long)f);
			ret = -1;
			goto out;
		}

		/* "f9401288 ldr"@0xffffff8008a68764 (+32 of `struct file` =
		 * `f_inode`) + "f9402915 ldr"@0xffffff8008a68774 (+80 of
		 * `struct inode` = `i_size`) */
		fsize = f->f_inode->i_size;
		/* line 1198 */
#line 1198
		ILI_INFO("fsize = %d\n", fsize);

		if (fsize <= 0) {
			/* line 1200;
			 * "\x013ILITEK: (%s, %d): The size of file is invaild\n"@0xffffff8009244aff */
#line 1200
			ILI_ERR("The size of file is invaild\n");
			filp_close(f, NULL);
			ret = -1;
			goto out;
		}

		ipio_vfree((void **)&idev->c296);
		idev->c288 = 0;
		idev->c296 = (u64)vmalloc(fsize);
		if (ERR_ALLOC_MEM((void *)idev->c296)) {
			/* line 1210 */
#line 1210
			ILI_ERR("Failed to allocate tp_fw by vmalloc, try again\n");
			idev->c296 = (u64)vmalloc(fsize);
			if (ERR_ALLOC_MEM((void *)idev->c296)) {
				/* line 1213 */
#line 1213
				ILI_ERR("Failed to allocate tp_fw after retry\n");
				filp_close(f, NULL);
				ret = -ENOMEM;
				goto out;
			}
		}

		old_fs = get_fs();
		set_fs(KERNEL_DS);
		set_fs(get_ds());
		/* "f90017ff str"@0xffffff8008a68854 */
		pos = 0;
		vfs_read(f, (char *)idev->c296, fsize, &pos);
		set_fs(old_fs);
		filp_close(f, NULL);
		/* "f9009115 str"@0xffffff8008a6a664 -- AFTER the `filp_close` */
		idev->c288 = fsize;
		break;
	default:
		/*
		 * line 1231. The argument is masked to one byte
		 * ("12001e83 and"@0xffffff8008a6889c), which is the width of the
		 * parameter;
		 * "\x013ILITEK: (%s, %d): Unknown open file method, %d\n"@0xffffff80092469b6
		 */
#line 1231
		ILI_ERR("Unknown open file method, %d\n", open_file_method);
		break;
	}

	/*
	 * line 1236. The condition is `ERR_ALLOC_MEM(c296) || !c288`, and the
	 * second test is 64-bit ("b4006b17 cbz"@0xffffff8008a688c8);
	 * "\x013ILITEK: (%s, %d): fw data/size is invaild\n"@0xffffff80092469e8
	 */
	if (ERR_ALLOC_MEM((void *)idev->c296) || !idev->c288) {
#line 1236
		ILI_ERR("fw data/size is invaild\n");
		ret = -1;
		goto out;
	}

	/* line 1243;
	 * "\x013ILITEK: (%s, %d): Convert hex file failed\n"@0xffffff8009246a15 */
	if (ilitek_tddi_fw_hex_convert((u8 *)idev->c296, idev->c288, pfw) < 0) {
#line 1243
		ILI_ERR("Convert hex file failed\n");
		ret = -1;
	}

out:
	/*
	 * "b4000060 cbz"@0xffffff8008a6974c + "97df62ba bl"@0xffffff8008a69750
	 * + "f900969f str"@0xffffff8008a69754 -- the common tail of ALL the
	 * paths, the error ones included
	 */
	ipio_vfree((void **)&idev->c296);
	return ret;
}

/*
 * ilitek_tddi_fw_upgrade() was reconstructed from the factory kernel disassembly (0xffffff8008a684e0, 8824 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_fw.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_fw_upgrade(int open_file_method)
{
	int i, ret = 0, retry = 3;

	/* "39438109 ldrb"@0xffffff8008a68518 (+224),
	 * "3949dd08 ldrb"@0xffffff8008a68520 (+631) */
	if (!idev->c224 || idev->c631 || ERR_ALLOC_MEM(pfw)) {
		if (ERR_ALLOC_MEM(pfw)) {
			ipio_vfree((void **)&pfw);
			pfw = vmalloc(MAX_HEX_FILE_SIZE);
			if (ERR_ALLOC_MEM(pfw)) {
				/*
				 * line 1261. The argument is the pointer
				 * itself (there is no "aa0303e0 mov": x0 is not
				 * prepared, the `printk` has no third argument
				 * because the format takes it from w3 and w3 is
				 * not written). See G13;
				 * "\x013ILITEK: (%s, %d): Failed to allocate pfw memory, %ld\n"@0xffffff8009246333
				 */
#line 1261
				ILI_ERR("Failed to allocate pfw memory, %ld\n");
				ipio_vfree((void **)&pfw);
				ret = -ENOMEM;
				goto out;
			}
		}

		/* "32001fe8 orr"@0xffffff8008a68580 = 0xff,
		 * "eb13013f cmp"@0xffffff8008a6859c against 0x28000 */
		for (i = 0; i < MAX_HEX_FILE_SIZE; i++)
			pfw[i] = 0xff;

		/* "37f809d3 tbnz"@0xffffff8008a69758 */
		if (ilitek_tdd_fw_hex_open(open_file_method, pfw) < 0) {
			/* line 1272;
			 * "\x013ILITEK: (%s, %d): Open hex file fail, try upgrade from ILI file\n"@0xffffff8009246382 */
#line 1272
			ILI_ERR("Open hex file fail, try upgrade from ILI file\n");

			/* "394a0d28 ldrb"@0xffffff8008a698ac (+643) */
			if (!idev->c643) {
				/* line 1280;
				 * "\x013ILITEK: (%s, %d): Ignore update from ILI file\n"@0xffffff80092463c5 */
#line 1280
				ILI_ERR("Ignore update from ILI file\n");
				ipio_vfree((void **)&pfw);
				/*
				 * "12800e34 mov"@0xffffff8008a698dc -- and the
				 * jump goes to 0xffffff8008a6a2f8, that is it SKIPS
				 * the clearing of `c636` and the five re-reads too:
				 * it is a `return`, not a `goto`
				 */
				return -EFW_CONVERT_FILE;
			}

			if (ilitek_tddi_fw_ili_convert(pfw) < 0) {
				/* line 1286;
				 * "\x013ILITEK: (%s, %d): Convert ILI file error\n"@0xffffff80092463f6 */
#line 1286
				ILI_ERR("Convert ILI file error\n");
				ret = -EFW_CONVERT_FILE;
				goto out;
			}
		}

		ilitek_tddi_fw_update_block_info(pfw);
	}

	do {
		ret = ilitek_tddi_fw_flash_upgrade(pfw);
		if (ret == 0)
			goto done;

		/* line 1299;
		 * "\x013ILITEK: (%s, %d): Upgrade failed, do retry!\n"@0xffffff8009246422 */
#line 1299
		ILI_ERR("Upgrade failed, do retry!\n");
	} while (--retry);

	/* line 1303;
	 * "\x013ILITEK: (%s, %d): Failed to upgrade fw %d times, erasing flash\n"@0xffffff8009246451 */
#line 1303
	ILI_ERR("Failed to upgrade fw %d times, erasing flash\n", retry);

	if (ilitek_ice_mode_ctrl(true, false) < 0)
		/* line 1305;
		 * "\x013ILITEK: (%s, %d): Enable ice mode failed while erasing flash\n"@0xffffff8009246493 */
#line 1305
		ILI_ERR("Enable ice mode failed while erasing flash\n");

	if (ilitek_tddi_fw_flash_erase() < 0)
		/* line 1307;
		 * "\x013ILITEK: (%s, %d): Failed to erase flash\n"@0xffffff80092464d3 */
#line 1307
		ILI_ERR("Failed to erase flash\n");

	if (ilitek_ice_mode_ctrl(false, false) < 0)
		/* line 1309;
		 * "\x013ILITEK: (%s, %d): Disable ice mode failed after erase flash\n"@0xffffff80092464fe */
#line 1309
		ILI_ERR("Disable ice mode failed after erase flash\n");

	if (ilitek_tddi_reset_ctrl(idev->c612) < 0)
		/* line 1311;
		 * "\x013ILITEK: (%s, %d): TP reset failed after erase flash\n"@0xffffff800924653d */
#line 1311
		ILI_ERR("TP reset failed after erase flash\n");

out:
	/* "3909f11f strb"@0xffffff8008a6a2b8 */
	idev->c636 = false;

done:
	/* "b942c908 ldr"@0xffffff8008a6a2bc (+712) */
	if (idev->c712)
		ilitek_ice_mode_ctrl(false, false);

	ilitek_tddi_ic_get_core_ver();
	ilitek_tddi_ic_get_protocl_ver();
	ilitek_tddi_ic_get_fw_ver();
	ilitek_tddi_ic_get_tp_info();
	ilitek_tddi_ic_get_panel_info();

	/* "3949f109 ldrb"@0xffffff8008a6a2e8 + "35000069 cbnz"@0xffffff8008a6a2ec */
	if (!idev->c636)
		idev->c636 = true;

	return ret;
}
