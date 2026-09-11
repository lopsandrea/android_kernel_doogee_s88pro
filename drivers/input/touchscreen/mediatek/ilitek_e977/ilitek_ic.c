// SPDX-License-Identifier: GPL-2.0
/*
 * Ilitek TDDI touch panel, Doogee S88 Pro -- group D, the chip registers.
 *
 * Twenty-nine functions, [0xffffff8008a5692c, 0xffffff8008a5aad8), 16812
 * bytes. It is the foundation group: groups E, F, G and H all call into it.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/wait.h>

#include "ilitek.h"

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0fca50).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_ic_c48 {
	/* +0  8 bit. "39400105 ldrb"@0xffffff8008a5aa00 /
	 * "3900010a strb"@0xffffff8008a5a9c8 (= pid >> 8) */
	u8 c0;
	/* +1  8 bit. "39000509 strb"@0xffffff8008a5a9d4 (= pid) */
	u8 c1;
	/* +2  16 bit. "7900050a strh"@0xffffff8008a5a9b8 (= pid >> 16) */
	u16 c2;
	/* +4  16 bit. "79000909 strh"@0xffffff8008a569d4 = 0x9881 or
	 * "79000909 strh"@0xffffff8008a56a3c = 0x9878 */
	u16 c4;
	/*
	 * +6..7: A HOLE. None of the 29 functions touches it; do NOT fill it, it is
	 * what keeps +8 where it is.
	 */
	u8 __ignoto_6[2];
	/* +8  32 bit. "b9000953 str"@0xffffff8008a569b0 (= the first argument
	 * of `ilitek_tddi_ic_check_support`) */
	u32 c8;
	/*
	 * +12 32-bit. Written only by `ilitek_tddi_ic_init`
	 * ("b9000d2a str"@0xffffff8008a5aaa4) with the value 0x0004009c; read by
	 * `get_info` as a read address ("b9400d00 ldr"@0xffffff8008a5a8f8)
	 */
	u32 c12;
	/* +16 32 bit = 0x0005100c. Low half of "f900092a str"@0xffffff8008a5aa94 */
	u32 c16;
	/* +20 32-bit = 0x00044008. High half of the same `str` */
	u32 c20;
	/* +24 32 bit = 0x00051010. "2903212b stp"@0xffffff8008a5aac4 */
	u32 c24;
	/*
	 * +28 32-bit = 0x00040050. The same `stp`; read by `whole_reset` as an
	 * address ("b9401d2a ldr"@0xffffff8008a58220)
	 */
	u32 c28;
	/* +32 32 bit = 0x000400a0. "2904292c stp"@0xffffff8008a5aab0 */
	u32 c32;
	/* +36 32 bit = 0x000400a4. Same `stp` */
	u32 c36;
	/* +40 32 bit. Written by `get_info` through `ilitek_ice_mode_read`
	 * ("9100a101 add"@0xffffff8008a5a934) and then truncated to one byte
	 * ("3940a12a ldrb"@0xffffff8008a5a9e0 + "b900292a str"@0xffffff8008a5a9e4) */
	u32 c40;
	/* +44 32 bit. Idem ("9100b101 add"@0xffffff8008a5a968,
	 * "3940b12a ldrb"@0xffffff8008a5a9ec) */
	u32 c44;
	/*
	 * +48 32-bit. "b9003149 str"@0xffffff8008a5a17c: `get_fw_ver` puts
	 * the four version bytes in reversed order (`rev`) there
	 */
	u32 c48;
	/* +52 32 bit. "b9003509 str"@0xffffff8008a59edc: `get_core_ver` puts there
	 * (b1<<16)|(b2<<8)|b3 */
	u32 c52;
	/* +56 32 bit = 0x1ffff. "b9003909 str"@0xffffff8008a56aa4 */
	u32 c56;
	/*
	 * +60 32-bit = 0x19881 ("b9003d48 str"@0xffffff8008a569c0) or
	 * 0x19878 ("b9003d48 str"@0xffffff8008a56a28).
	 * `ilitek_tddi_ic_whole_reset` writes it into register +28 and prints it
	 * with the format
	 * "\x016ILITEK: (%s, %d): ic whole reset key = 0x%x, edge_delay = %d\n"@0xffffff8009240674
	 * : the binary calls "key" THE VALUE PRINTED,
	 * not the field, so the name stays `c60`.
	 */
	u32 c60;
	/* +64 32 bit = 0x1000 or 0x2000.
	 * "b900412a str"@0xffffff8008a56a94, "b900414b str"@0xffffff8008a56a68 */
	u32 c64;
	/* +68 8 bit. Set to 1 by `ilitek_tddi_ic_init`
	 * ("3901112c strb"@0xffffff8008a5aa9c), zeroed by `check_support`
	 * ("3901113f strb"@0xffffff8008a56a08) */
	u8 c68;
	/* +69..71: padding up to the 8-byte alignment of +72 */
	u8 __ignoto_69[3];
	/*
	 * void() was reconstructed from the factory kernel disassembly (0xffffff8008a569e0).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	void (*c72)(void);
};

/*
 * `idev->c48` is `void *` in `ilitek.h`: here it is needed typed. See HEADER
 * DELTA 1 in the report. It is not a local: it is an expression, so that
 * every use re-reads `idev` and `idev->c48` as the binary does.
 */
#define IC	((struct ilitek_ic_c48 *)idev->c48)

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009987570, 44 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_protocol_info {
	u32 c0;			/* the version: 0x050000 .. 0x050700 */
	u32 c4;
	u32 c8;			/* read length in `get_protocl_ver` */
	u32 c12;
	u32 c16;
	u32 c20;
	u32 c24;		/* read length in `get_core_ver` */
	u32 c28;
	u32 c32;
	u32 c36;
	u32 c40;
};

static struct ilitek_protocol_info protocol_info[] = {
	/* the bytes come from `relocazioni.py`/`rodata.py`, they are not inferred */
	{ 0x050000, 4, 4, 14, 30, 5, 5, 2, 8, 3,  8 },
	{ 0x050100, 4, 3, 14, 30, 5, 5, 3, 8, 3,  8 },
	{ 0x050200, 4, 4, 14, 30, 5, 5, 3, 8, 3,  8 },
	{ 0x050300, 9, 4, 14, 30, 5, 5, 3, 8, 3,  8 },
	{ 0x050400, 9, 4, 14, 30, 5, 5, 3, 8, 15, 8 },
	{ 0x050500, 9, 4, 14, 30, 5, 5, 3, 8, 15, 14 },
	{ 0x050600, 9, 4, 14, 30, 5, 5, 3, 8, 15, 14 },
	{ 0x050700, 9, 4, 14, 30, 5, 5, 3, 8, 15, 14 },
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff80099873a8, 24 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_func_ctrl {
	const char *name;	/* +0,  resolved from the addends of `.rela.dyn` */
	u8 cmd[8];		/* +8,  see above: 8 is a CHOICE between 6, 7 and 8 */
	u32 len;		/* +16 */
};

static struct ilitek_func_ctrl func_ctrl[] = {
	{ "sense",			{0x01, 0x01, 0x00}, 3 },
	{ "sleep",			{0x01, 0x02, 0x00}, 3 },
	{ "glove",			{0x01, 0x06, 0x00}, 3 },
	{ "stylus",			{0x01, 0x07, 0x00}, 3 },
	{ "tp_scan_mode",		{0x01, 0x08, 0x00}, 3 },
	{ "lpwg",			{0x01, 0x0A, 0x00}, 3 },
	{ "gesture",			{0x01, 0x0B, 0x3F}, 3 },
	{ "phone_cover",		{0x01, 0x0C, 0x00}, 3 },
	{ "finger_sense",		{0x01, 0x0F, 0x00}, 3 },
	{ "phone_cover_window",		{0x0E, 0x00, 0x00}, 3 },
	{ "proximity",			{0x01, 0x10, 0x00}, 3 },
	{ "plug",			{0x01, 0x11, 0x00}, 3 },
	{ "edge_palm",			{0x01, 0x12, 0x00}, 3 },
	{ "lock_point",			{0x01, 0x13, 0x00}, 3 },
	{ "active",			{0x01, 0x14, 0x00}, 3 },
	{ "idle",			{0x01, 0x19, 0x00}, 3 },
	{ "gesture_demo_en",		{0x01, 0x16, 0x00}, 3 },
	{ "tp_recore",			{0x01, 0x18, 0x00}, 3 },
	{ "knock_en",			{0x01, 0x0A, 0x08, 0x03}, 6 },
};

/*
 * ===========================================================================
 * THE CHIP STRUCTURE, THE OBJECT
 * ===========================================================================
 * `.bss` at 0xffffff800a0fca50, zeroed; `.rela.dyn` has no relocation with
 * that addend, so NO static datum points at it: it is reachable only
 * through `idev->c48`, which `ilitek_tddi_ic_init` fills in
 * ("a9032149 stp"@0xffffff8008a5aad0).
 */
static struct ilitek_ic_c48 chip;

/*
 * ===========================================================================
 * THE GROUP D FUNCTIONS THIS BATCH DID NOT WRITE
 * ===========================================================================
 * Declared and NOT defined: a link failure is the honest outcome, a stub
 * would make them look written (project rule 6).
 * The signatures come from the call sites that live in this file.
 */

/*
 * fix_tp_proc_info() was reconstructed from the factory kernel disassembly (0xffffff8008a5151c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fix_tp_proc_info(char *buf, u8 len);

/*
 * Group G, not written. The only call from group D out of the group and
 * into the driver: "94003621 bl"@0xffffff8008a59c58 towards
 * <ilitek_tddi_flash_clear_dma> (0xffffff8008a674dc). No arguments and
 * the return value unused: that is all the site decides.
 */
void ilitek_tddi_flash_clear_dma(void);

static void ilitek_tddi_ic_wr_pack(int packet);
static void firmware_hd_dma_crc_off_ili9881(void);
static void firmware_hd_dma_crc_off_ili7807(void);

/*
 * The functions OF THIS FILE used before they are defined. The order of the
 * definitions below is that of the factory SOURCE LINES (82, 129, 151, 177,
 * ...), read from the `__LINE__`s of the `printk`s; the emission order in the
 * binary is another one, and is not information about the source.
 * `ilitek.h` does not declare them -- verified: of the six, `ilitek.h` names
 * only `ilitek_tddi_ic_func_ctrl`, and it does so inside a comment (line 571),
 * not in a prototype. That is HEADER DELTA 4.
 */
int ilitek_ice_mode_write(u32 addr, u32 data, int len);
int ilitek_ice_mode_read(u32 addr, u32 *data, int len);
int ilitek_tddi_ic_watch_dog_ctrl(bool write, bool on);
int ilitek_tddi_ic_check_support(u32 pid, u16 id);
void ilitek_tddi_ic_get_pc_counter(void);

/*
 * ilitek_tddi_ic_check_support() was reconstructed from the factory kernel disassembly (0xffffff8008a5692c, 428 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_check_support(u32 pid, u16 id)
{
	if (id != 0x9881 && pid != 0x98811103 && pid != 0x98811104 &&
	    pid != 0x9881 && id != 0x7807 && pid != 0x78071000 &&
	    pid != 0x78071001 && pid != 0x7807) {
#line 82
		ILI_INFO("ERROR, ILITEK CHIP (%x, %x) Not found !!\n", pid, id);
		return -1;
	}

	IC->c8 = pid;

	if (id == 0x9881) {
		IC->c60 = 0x19881;
		IC->c4 = 0x9881;
		IC->c72 = firmware_hd_dma_crc_off_ili9881;
		/* "f941a109 ldr"@0xffffff8008a569e4 = idev->c832;
		 * "3941114a ldrb"@0xffffff8008a569f0 = IC->c68 */
		if (idev->c832 && IC->c68) {
			((void (*)(int))idev->c832)(0);
			IC->c68 = 0;
		}
		/* "5281e00a mov"@0xffffff8008a56a10 + "72b3102a movk"@0xffffff8008a56a14 */
		IC->c64 = (pid == 0x98810F00) ? 0x1000 : 0x2000;
	} else {
		IC->c60 = 0x19878;
		IC->c4 = 0x9878;
		IC->c72 = firmware_hd_dma_crc_off_ili7807;
		IC->c64 = 0x2000;
		if (pid != 0x78071000 && pid != 0x78071001)
			IC->c68 = 0;
		/* "121d726a and"@0xffffff8008a56a78 */
		if ((pid & 0xFFFFFFF8) == 0x78071000)
			idev->c639 = 1;
	}

	IC->c56 = 0x1FFFF;
	return 0;
}

/*
 * firmware_hd_dma_crc_off_ili9881() was reconstructed from the factory kernel disassembly (0xffffff8008a56ad8, 312 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void firmware_hd_dma_crc_off_ili9881(void)
{
	ilitek_ice_mode_write(0x041016, 0x00, 1);
	ilitek_ice_mode_write(0x041048, 0x01, 4);
}

/*
 * ===========================================================================
 * firmware_hd_dma_crc_off_ili7807 -- 0xffffff8008a56c10, 316 byte
 * ===========================================================================
 * "5282e4a9 mov"@0xffffff8008a56cc4 + "72a08209 movk"@0xffffff8008a56cc8
 * = 0x04101725 -> addr 0x041017; "320007ea orr"@0xffffff8008a56ccc = data 3,
 * "528000a1 mov"@0xffffff8008a56ce0 = len+4 = 5 -> len 1.
 */
static void firmware_hd_dma_crc_off_ili7807(void)
{
	ilitek_ice_mode_write(0x041016, 0x00, 1);
	ilitek_ice_mode_write(0x041017, 0x03, 1);
}

/*
 * ===========================================================================
 * ilitek_ice_mode_bit_mask_write -- 0xffffff8008a56d4c, 404 bytes, lines 129..140
 * ===========================================================================
 * "0a350108 bic"@0xffffff8008a56d9c = data & ~mask,
 * "0a15028a and"@0xffffff8008a56d98 = value & mask,
 * "2a0a0114 orr"@0xffffff8008a56da0 the merge. The order of the two arguments
 * is decided by which register enters which operation: w21 = x1 = mask,
 * w20 = x2 = value.
 */
int ilitek_ice_mode_bit_mask_write(u32 addr, u32 mask, u32 value)
{
	int ret = 0;
	u32 data = 0;

	ret = ilitek_ice_mode_read(addr, &data, sizeof(u32));
	if (ret < 0) {
#line 129
		ILI_ERR("Read data error\n");
		return -1;
	}

	data = (data & (~mask)) | (value & mask);

	/* \x016 + guard "39656129 ldrb"@0xffffff8008a56d94 -> ILI_DBG */
	ILI_DBG("mask value data = %x\n", data);

	ret = ilitek_ice_mode_write(addr, data, 4);
	if (ret < 0)
		ILI_ERR("Failed to re-write data in ICE mode, ret = %d\n", ret);

	return ret;
}

/*
 * ilitek_ice_mode_write() was reconstructed from the factory kernel disassembly (0xffffff8008a57098, 272 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_ice_mode_write(u32 addr, u32 data, int len)
{
	int i, ret = 0;
	u8 txbuf[64] = {0};

	if (!idev->c712) {
#line 151
		ILI_ERR("ice mode not enabled\n");
		return -1;
	}

	txbuf[0] = 0x25;
	txbuf[1] = (char)((addr & 0x000000FF) >> 0);
	txbuf[2] = (char)((addr & 0x0000FF00) >> 8);
	txbuf[3] = (char)((addr & 0x00FF0000) >> 16);

	for (i = 0; i < len; i++)
		txbuf[i + 4] = (char)(data >> (8 * i));

	ret = idev->c776(txbuf, len + 4);
	if (ret < 0)
		ILI_ERR("Failed to write data in ice mode, ret = %d\n", ret);

	return ret;
}

/*
 * ilitek_ice_mode_read() was reconstructed from the factory kernel disassembly (0xffffff8008a56ee0, 440 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_ice_mode_read(u32 addr, u32 *data, int len)
{
	int ret = 0;
	u8 *rxbuf = NULL;
	u8 txbuf[4] = {0};

	if (!idev->c712) {
#line 177
		ILI_ERR("ice mode not enabled\n");
		return -1;
	}

	txbuf[0] = 0x25;
	txbuf[1] = (char)((addr & 0x000000FF) >> 0);
	txbuf[2] = (char)((addr & 0x0000FF00) >> 8);
	txbuf[3] = (char)((addr & 0x00FF0000) >> 16);

	ret = idev->c776(txbuf, 4);
	if (ret < 0)
		goto out;

	rxbuf = kzalloc(len, GFP_KERNEL);
	if (IS_ERR(rxbuf) || rxbuf == NULL) {
		ILI_ERR("Failed to allocate rxbuf, %ld\n", PTR_ERR(rxbuf));
		ret = -ENOMEM;
		goto out;
	}

	ret = idev->c784(rxbuf, len);
	if (ret < 0)
		goto out;

	if (len == 1)
		*data = rxbuf[0];
	else
		*data = (rxbuf[0] | rxbuf[1] << 8 | rxbuf[2] << 16 |
			 rxbuf[3] << 24);

out:
	if (ret < 0)
#line 208
		ILI_ERR("Failed to read data in ice mode, ret = %d\n", ret);

	if (rxbuf)
		kfree(rxbuf);

	return ret;
}

/*
 * ilitek_ice_mode_ctrl() was reconstructed from the factory kernel disassembly (0xffffff8008a571a8, 1240 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_ice_mode_ctrl(bool enable, bool mcu)
{
	int ret = 0, retry = 0;
	u8 cmd_open[4] = {0x25, 0x62, 0x10, 0x18};
	u8 cmd_close[4] = {0x1B, 0x62, 0x10, 0x18};
	/*
	 * `pid` is NOT initialised in the factory build: in the whole function there
	 * is no write to [sp,#12] -- only the three reads
	 * "b9400fe0 ldr"@0xffffff8008a5732c,
	 * "b9400fe0 ldr"@0xffffff8008a573c0 and
	 * "b9400fe0 ldr"@0xffffff8008a57454, each after the
	 * `ilitek_ice_mode_read` that fills the variable. With `= 0` our object
	 * emitted an extra `str wzr, [sp,#12]` that the binary does not have.
	 */
	u32 pid;

	ILI_DBG("%s ICE mode, mcu on = %d\n", (enable ? "Enable" : "Disable"),
#line 221
		mcu);

	if (enable) {
		if (idev->c712) {
			ILI_INFO("ice mode already enabled\n");
			return 0;
		}

		/* "320013e9 orr"@0xffffff8008a5729c = 0x1f */
		if (mcu)
			cmd_open[0] = 0x1F;

		/* "b902c909 str"@0xffffff8008a572a8 */
		idev->c712 = 1;

		for (retry = 0; retry < 3; retry++) {
			if (idev->c776(cmd_open, sizeof(cmd_open)) < 0)
#line 236
				ILI_ERR("write ice mode cmd error\n");

			if (idev->c832 && IC->c68)
				((void (*)(int))idev->c832)(1);

			if (ilitek_ice_mode_read(IC->c12, &pid,
						 sizeof(u32)) < 0)
				ILI_ERR("Read pid error\n");

			/* "53107c01 lsr"@0xffffff8008a57330 */
			if (!ilitek_tddi_ic_check_support(pid, pid >> 16))
				break;
		}

		if (retry >= 3) {
#line 250
			ILI_ERR("Enter to ICE Mode failed !!\n");
			ret = -1;
			goto out;
		}

		if (ilitek_ice_mode_write(0x47002, 0x00, 1) < 0)
#line 259
			ILI_ERR("Write 0x0 at 0x47002 failed\n");

		return 0;
	}

	if (!idev->c712) {
#line 263
		ILI_INFO("ice mode already disabled\n");
		return 0;
	}

	if (idev->c639) {
		for (retry = 0; retry < 3; retry++) {
			ret = idev->c776(cmd_close, sizeof(cmd_close));
			if (ret < 0)
#line 270
				ILI_ERR("write ice mode disable failed\n");

			/* "71028c1f cmp"@0xffffff8008a57280 = 0xa3 */
			if (!idev->c800 || idev->c800() == 0xA3) {
				ret = 0;
				break;
			}
			usleep_range(1000, 1000);
		}

		if (retry >= 3) {
#line 287
			ILI_ERR("Failed to exit ice mode\n");
			ret = -EIO;
		}
	} else {
		ret = idev->c776(cmd_close, sizeof(cmd_close));
		if (ret < 0)
#line 294
			ILI_ERR("Exit to ICE Mode failed !!\n");
	}

out:
	/* "b902c91f str"@0xffffff8008a57600 */
	idev->c712 = 0;
	return ret;
}

/*
 * ilitek_tddi_ic_watch_dog_ctrl() was reconstructed from the factory kernel disassembly (0xffffff8008a57680, 1696 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_watch_dog_ctrl(bool write, bool on)
{
	int timeout = 50;
	u32 read = 0;

	if (!idev->c712) {
#line 307
		ILI_ERR("ice mode wasn't enabled\n");
		return -1;
	}

	if (IC->c16 == 0 || IC->c2 == 0) {
		ILI_ERR("WDT/CHIP ID is invalid\n");
		return -EINVAL;
	}

	if (!idev->c628) {
#line 318
		ILI_INFO("WDT ctrl is off, do nothing\n");
		return 0;
	}

	if (!write) {
		if (ilitek_ice_mode_read(IC->c16, &read, sizeof(u8)) < 0) {
			ILI_ERR("Read wdt error\n");
			return -1;
		}
		ILI_INFO("Read WDT: %s\n", (read ? "ON" : "OFF"));
		return read;
	}

	ILI_INFO("%s WDT, key = %x\n", (on ? "Enable" : "Disable"), IC->c4);

	if (on) {
		if (ilitek_ice_mode_write(IC->c16, 0x1, 1) < 0)
			ILI_ERR("Wrie WDT key failed\n");
	} else {
		udelay(300);
		if (ilitek_ice_mode_write(IC->c16, IC->c4 & 0xFF, 1) < 0)
#line 340
			ILI_ERR("Write WDT key failed\n");
		if (ilitek_ice_mode_write(IC->c16, IC->c4 >> 8, 1) < 0)
			ILI_ERR("Write WDT key failed\n");
	}

	while (timeout > 0) {
		udelay(40);

		if (ilitek_ice_mode_read(0x51018, &read, sizeof(u8)) < 0)
#line 348
			ILI_ERR("Read wdt active error\n");

		ILI_DBG("ret = %x\n", read);

		if (on) {
			if (read == 0xA5) {
#line 374
				ILI_INFO("WDT turn on succeed\n");
				return 0;
			}
		} else {
			if (read == 0x5A) {
#line 376
				ILI_INFO("WDT turn off succeed\n");
				if (ilitek_ice_mode_write(IC->c16, 0x00, 1) < 0)
					ILI_ERR("Write turn off cmd failed\n");
				return 0;
			}

			if (ilitek_ice_mode_write(IC->c16, 0x00, 1) < 0)
#line 360
				ILI_ERR("Write 0x0 at %x\n", IC->c16);

			if (ilitek_ice_mode_write(IC->c16, 0x98, 1) < 0)
#line 362
				ILI_ERR("Write 0x98 at %x\n", IC->c16);
		}
		timeout--;
	}

#line 368
	ILI_ERR("WDT turn on/off timeout !, ret = %x\n", read);
	ilitek_tddi_ic_get_pc_counter();
	return -EINVAL;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_pc_counter -- 0xffffff8008a57d20, 360 bytes, lines 693..708
 * ===========================================================================
 * THE TWO ADDRESSES ARE LOCALS, and the measurement proves it:
 * "2942cd14 ldp"@0xffffff8008a57d58 reads them ONCE at the top, and the two
 * tail `printk`s still use w20 and w19 ("2a1403e3 mov"@0xffffff8008a57e00,
 * "2a1303e3 mov"@0xffffff8008a57e24) without re-reading them. With `IC->c20`
 * written at every use the compiler would reload after every opaque call
 * (class A2 the other way round).
 */
void ilitek_tddi_ic_get_pc_counter(void)
{
	int ice = idev->c712;
	u32 pc = 0, latch = 0;
	u32 pc_addr = IC->c20;
	u32 latch_addr = IC->c24;

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
#line 693
			ILI_ERR("Enable ice mode failed while reading pc counter\n");

	if (ilitek_ice_mode_read(pc_addr, &pc, sizeof(u32)) < 0)
		ILI_ERR("Read pc conter error\n");

	if (ilitek_ice_mode_read(latch_addr, &latch, sizeof(u32)) < 0)
		ILI_ERR("Read pc conter error\n");

	idev->c228 = pc;
	idev->c232 = latch;
	ILI_ERR("read pc (addr: 0x%x) = 0x%x\n", pc_addr, idev->c228);
	ILI_ERR("read latch (addr: 0x%x) = 0x%x\n", latch_addr, idev->c232);

	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Disable ice mode failed while reading pc counter\n");
}

/*
 * ilitek_tddi_ic_func_ctrl() was reconstructed from the factory kernel disassembly (0xffffff8008a57e88, 456 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_func_ctrl(const char *name, int ctrl)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(func_ctrl); i++) {
		if (strlen(name) != strlen(func_ctrl[i].name))
			continue;
		if (strncmp(name, func_ctrl[i].name, strlen(name)))
			continue;
		if (strlen(name) == strlen(func_ctrl[i].name))
			break;
	}

	if (i >= ARRAY_SIZE(func_ctrl)) {
#line 396
		ILI_ERR("Not found function ctrl, %s\n", name);
		return -1;
	}

	/* "7141411f cmp"@0xffffff8008a57f5c */
	if (((struct ilitek_protocol_info *)idev->c56)->c0 == 0x050000) {
		ILI_ERR("Non support function ctrl with protocol v5.0\n");
		return -1;
	}

	/* "1117fd29 add"@0xffffff8008a57f88 = 0x50000 + 0x5ff */
	if (((struct ilitek_protocol_info *)idev->c56)->c0 > 0x0505FF) {
		if ((strlen(func_ctrl[i].name) == strlen("gesture") &&
		     !strncmp(func_ctrl[i].name, "gesture",
			      strlen(func_ctrl[i].name))) ||
		    (strlen(func_ctrl[i].name) == strlen("phone_cover_window") &&
		     !strncmp(func_ctrl[i].name, "phone_cover_window",
			      strlen(func_ctrl[i].name)))) {
			ILI_INFO("Non support %s function ctrl\n",
#line 410
				 func_ctrl[i].name);
			return -1;
		}
	}

	func_ctrl[i].cmd[2] = ctrl;

	ILI_INFO("func = %s, len = %d, cmd = 0x%x, 0%x, 0x%x\n",
		 func_ctrl[i].name, func_ctrl[i].len, func_ctrl[i].cmd[0],
		 func_ctrl[i].cmd[1], func_ctrl[i].cmd[2]);

	if (idev->c776(func_ctrl[i].cmd, func_ctrl[i].len) < 0) {
#line 423
		ILI_ERR("Write TP function failed\n");
		return -1;
	}

	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_code_reset -- 0xffffff8008a58050, 328 bytes, lines 436..444
 * ===========================================================================
 * `ice` IS A LOCAL, and the measurement proves it: "b942c914 ldr"@0xffffff8008a58074
 * reads +712 once only and "35000174 cbnz"@0xffffff8008a58140 reuses the
 * same w20 after three opaque calls.
 * The address and the datum come from the bytes:
 * "528804a9 mov"@0xffffff8008a580c0 + "72a08009 movk"@0xffffff8008a580c4
 * = 0x04004025 -> 0x25 0x40 0x00 0x04 -> addr 0x040040, and
 * "528015ca mov"@0xffffff8008a580c8 = 0xae with len+4 = 5.
 */
int ilitek_tddi_ic_code_reset(void)
{
	int ret = 0;
	int ice = idev->c712;

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
#line 436
			ILI_ERR("Enable ice mode failed before code reset\n");

	ret = ilitek_ice_mode_write(0x40040, 0xAE, 1);
	if (ret < 0)
		ILI_ERR("ic code reset failed\n");

	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Enable ice mode failed after code reset\n");

	return ret;
}

/*
 * ilitek_tddi_ic_whole_reset() was reconstructed from the factory kernel disassembly (0xffffff8008a58198, 468 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_whole_reset(void)
{
	int ret = 0;
	int ice = idev->c712;

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
#line 455
			ILI_ERR("Enable ice mode failed before chip reset\n");

	ILI_INFO("ic whole reset key = 0x%x, edge_delay = %d\n",
		 IC->c60, idev->c616);

	ret = ilitek_ice_mode_write(IC->c28, IC->c60, 4);
	if (ret < 0) {
		ILI_ERR("ic whole reset failed\n");
		goto out;
	}

	/* "940ffdf3 bl"@0xffffff8008a582a8 to <__const_udelay> */
	mdelay(idev->c616);

out:
	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Enable ice mode failed after chip reset\n");

	return ret;
}

/*
 * ilitek_tddi_ic_wr_pack() was reconstructed from the factory kernel disassembly (0xffffff8008a584e8, 540 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ilitek_tddi_ic_wr_pack(int packet)
{
	int retry = 100;
	u32 reg_data = 0;

	while (retry--) {
		if (ilitek_ice_mode_read(0x73010, &reg_data, sizeof(u8)) < 0)
#line 484
			ILI_ERR("Read 0x73010 error\n");

		if ((reg_data & 0x02) == 0) {
			ILI_INFO("check ok 0x73010 read 0x%X retry = %d\n",
#line 487
				 reg_data, retry);
			break;
		}
		mdelay(10);
	}

	if (retry <= 0)
		ILI_INFO("check 0x73010 error read 0x%X\n", reg_data);

	if (ilitek_ice_mode_write(0x73000, packet, 4) < 0)
		ILI_ERR("Write %x at 0x73000\n", packet);
}

/*
 * ilitek_tddi_ic_set_ddi_reg_onepage() was reconstructed from the factory kernel disassembly (0xffffff8008a5836c, 380 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_ic_set_ddi_reg_onepage(u8 page, u8 reg, u8 data)
{
	int ice = idev->c712;
	int wdt;
	/* the names `setpage`/`setreg` come from the format, they are not chosen */
	u32 setpage = 0x1FFFFF00 | page;
	u32 setreg = 0x1F000100 | (reg << 16) | data;

#line 536
	ILI_INFO("setpage =  0x%X setreg = 0x%X\n", setpage, setreg);

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
			ILI_ERR("Enable ice mode failed before writing ddi reg\n");

	wdt = ilitek_tddi_ic_watch_dog_ctrl(0, 0);
	if (wdt)
		if (ilitek_tddi_ic_watch_dog_ctrl(1, 0) < 0)
			ILI_ERR("Disable WDT failed before writing ddi reg\n");

	ilitek_tddi_ic_wr_pack(0x1FFF9527);
	ilitek_tddi_ic_wr_pack(setpage);
	ilitek_tddi_ic_wr_pack(setreg);
	ilitek_tddi_ic_wr_pack(0x1FFF9500);

	if (wdt)
		if (ilitek_tddi_ic_watch_dog_ctrl(1, 1) < 0)
#line 558
			ILI_ERR("Enable WDT failed after writing ddi reg\n");

	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Disable ice mode failed after writing ddi reg\n");
}

/*
 * ilitek_tddi_ic_rd_pack() was reconstructed from the factory kernel disassembly (0xffffff8009241af1).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static u32 ilitek_tddi_ic_rd_pack(int packet)
{
	int retry = 100;
	u32 reg_data = 0;

	while (retry--) {
		if (ilitek_ice_mode_read(0x4800A, &reg_data, sizeof(u8)) < 0)
#line 509
			ILI_ERR("Read 0x4800A error\n");

		if ((reg_data & 0x02) == 0x02) {
			ILI_INFO("check  ok 0x4800A read 0x%X retry = %d\n",
#line 512
				 reg_data, retry);
			break;
		}
		mdelay(10);
	}

	if (retry <= 0)
#line 518
		ILI_INFO("check 0x4800A error read 0x%X\n", reg_data);

	if (ilitek_ice_mode_write(0x4800A, packet, 1) < 0)
		ILI_ERR("Write 0x2 at 0x4800A\n");

	if (ilitek_ice_mode_read(0x73016, &reg_data, sizeof(u8)) < 0)
		ILI_ERR("Read 0x73016 error\n");

	return reg_data;
}

/*
 * ilitek_tddi_ic_get_ddi_reg_onepage() was reconstructed from the factory kernel disassembly (0xffffff8008a58704, 1108 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_ic_get_ddi_reg_onepage(u8 page, u8 reg, u8 *data)
{
	int ice = idev->c712;
	int wdt;
	u32 setpage = 0x1FFFFF00 | page;
	u32 setreg = 0x2F000100 | (reg << 16);

#line 572
	ILI_INFO("setpage = 0x%X setreg = 0x%X\n", setpage, setreg);

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
			ILI_ERR("Enable ice mode failed before reading ddi reg\n");

	wdt = ilitek_tddi_ic_watch_dog_ctrl(0, 0);
	if (wdt)
		if (ilitek_tddi_ic_watch_dog_ctrl(1, 0) < 0)
			ILI_ERR("Disable WDT failed before reading ddi reg\n");

	ilitek_tddi_ic_wr_pack(0x1FFF9527);
	ilitek_tddi_ic_wr_pack(setpage);
	ilitek_tddi_ic_wr_pack(0x1FFF9487);

	if (ilitek_ice_mode_write(0x4800A, 0x02, 1) < 0)
#line 592
		ILI_ERR("Write 0x2 at 0x4800A\n");

	ilitek_tddi_ic_wr_pack(setreg);
	*data = ilitek_tddi_ic_rd_pack(0x02);

	ILI_INFO("check page = 0x%X, reg = 0x%X, read 0x%X\n", page, reg,
#line 595
		 *data);

	ilitek_tddi_ic_wr_pack(0x1FFF9400);
	ilitek_tddi_ic_wr_pack(0x1FFF9500);

	if (wdt)
		if (ilitek_tddi_ic_watch_dog_ctrl(1, 1) < 0)
#line 604
			ILI_ERR("Enable WDT failed after reading ddi reg\n");

	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Disable ice mode failed after reading ddi reg\n");
}

/*
 * ilitek_tddi_ic_check_otp_prog_mode() was reconstructed from the factory kernel disassembly (0xffffff8008a58b58, 932 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_ic_check_otp_prog_mode(void)
{
	int retry = 5;
	u32 prog_mode, prog_done;

	if (!idev->c629)
		return;

	if (ilitek_ice_mode_ctrl(1, 0) < 0) {
#line 620
		ILI_ERR("enter ice mode failed in otp\n");
		return;
	}

	if (ilitek_tddi_ic_watch_dog_ctrl(1, 0) < 0) {
		ILI_ERR("disable WDT failed in otp\n");
		return;
	}

	do {
		if (ilitek_ice_mode_write(0x43008, 0x80, 1) < 0)
			ILI_ERR("Write 0x80 at 0x43008 failed\n");

		if (ilitek_ice_mode_write(0x43030, 0x00, 1) < 0)
			ILI_ERR("Write 0x0 at 0x43030 failed\n");

		if (ilitek_ice_mode_write(0x4300C, 0x04, 1) < 0)
			ILI_ERR("Write 0x4 at 0x4300C failed\n");

		mdelay(1);

		if (ilitek_ice_mode_write(0x4300C, 0x04, 1) < 0)
#line 643
			ILI_ERR("Write 0x4 at 0x4300C\n");

		if (ilitek_ice_mode_read(0x43030, &prog_done, sizeof(u8)) < 0)
			ILI_ERR("Read prog_done error\n");

		if (ilitek_ice_mode_read(0x43008, &prog_mode, sizeof(u8)) < 0)
			ILI_ERR("Read prog_mode error\n");

		ILI_INFO("otp prog_mode = 0x%x, prog_done = 0x%x\n", prog_mode,
#line 651
			 prog_done);

		if (prog_mode == 0x80 && prog_done == 0x00)
			break;
	} while (--retry > 0);

	if (retry <= 0)
#line 657
		ILI_ERR("OTP Program mode error!\n");
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_pc_counter_forwdt -- 0xffffff8008a59278, 336 bytes, lines 720..732
 * ===========================================================================
 * The twin of the previous one, with two MEASURED differences: `idev->c712`
 * is not kept in a local ("b942c909 ldr"@0xffffff8008a592a0, w9 is a
 * temporary) and the tail does not call `ilitek_ice_mode_ctrl` but clears the
 * field directly ("b902c91f str"@0xffffff8008a593a0).
 */
void ilitek_tddi_ic_get_pc_counter_forwdt(void)
{
	u32 pc = 0, latch = 0;
	u32 pc_addr = IC->c20;
	u32 latch_addr = IC->c24;

	if (!idev->c712)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
#line 720
			ILI_ERR("Enable ice mode failed while reading pc counter\n");

	if (ilitek_ice_mode_read(pc_addr, &pc, sizeof(u32)) < 0)
		ILI_ERR("Read pc conter error\n");

	if (ilitek_ice_mode_read(latch_addr, &latch, sizeof(u32)) < 0)
		ILI_ERR("Read pc conter error\n");

	idev->c228 = pc;
	idev->c232 = latch;
	ILI_ERR("read pc (addr: 0x%x) = 0x%x\n", pc_addr, idev->c228);
	ILI_ERR("read latch (addr: 0x%x) = 0x%x\n", latch_addr, idev->c232);

#line 732
	ILI_ERR("force Disable ice mode\n");
	idev->c712 = 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_check_int_stat -- 0xffffff8008a593c8, 268 bytes, line 740
 * ===========================================================================
 * The macro is `wait_event_interruptible_timeout`: it is recognised by the four
 * pieces the binary carries in plain sight -- `init_wait_entry`
 * ("97db1149 bl"@0xffffff8008a593fc), `prepare_to_wait_event` with
 * TASK_INTERRUPTIBLE ("320003e2 orr"@0xffffff8008a59408 = 1),
 * `schedule_timeout` ("94107160 bl"@0xffffff8008a5942c) and `finish_wait`
 * ("97db11bf bl"@0xffffff8008a5946c). The wait queue is `idev->inq`
 * at +568 ("9108e100 add"@0xffffff8008a5940c = 0x238 = 568).
 * The timeout is 750 ("52805dd3 mov"@0xffffff8008a59420), and with CONFIG_HZ=250
 * `msecs_to_jiffies(3000)` = (3000+3)/4 = 750.
 */
int ilitek_tddi_ic_check_int_stat(void)
{
	if (!wait_event_interruptible_timeout(idev->inq, !idev->c732,
					      msecs_to_jiffies(3000))) {
#line 740
		ILI_ERR("Error! Interrupt for MP isn't received\n");
		idev->c732 = 0;
		return -1;
	}

	return 0;
}

/*
 * ilitek_tddi_ic_check_busy() was reconstructed from the factory kernel disassembly (0xffffff8008a594d4, 560 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_check_busy(int count, int delay)
{
	u8 cmd[2] = {0};
	u8 busy = 0, rby = 0;

	cmd[0] = 0xF6;
	cmd[1] = 0xF3;

	if (idev->c320 == 0)
		rby = 0x41;
	else if (idev->c320 == 1)
		rby = 0x51;
	else {
#line 760
		ILI_ERR("Unknown TP mode (0x%x)\n", idev->c320);
		return -EINVAL;
	}

	ILI_INFO("read byte = %x, delay = %d\n", rby, delay);

	do {
		if (idev->c776(cmd, sizeof(cmd)) < 0)
			ILI_ERR("Write %x,%x failed\n", 0xF6, 0xF3);

		if (idev->c776(cmd + 1, 1) < 0)
#line 770
			ILI_ERR("Write %x failed\n", 0xF3);

		if (idev->c784(&busy, 1) < 0)
#line 772
			ILI_ERR("Read check busy failed\n");

		ILI_DBG("busy = 0x%x\n", busy);

		if (busy == rby) {
			ILI_INFO("Check busy free\n");
			return 0;
		}

		mdelay(delay);
	} while (--count > 0);

	ILI_ERR("Check busy (0x%x) timeout !\n", busy);
	ilitek_tddi_ic_get_pc_counter();
	return -1;
}

/*
 * ilitek_tddi_ic_spi_speed_ctrl() was reconstructed from the factory kernel disassembly (0xffffff8008a58efc, 892 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_ic_spi_speed_ctrl(bool enable)
{
	int ret = 0;

#line 662
	ILI_INFO("%s spi speed up\n", (enable ? "Enable" : "Disable"));

	if (enable) {
		ret = ilitek_ice_mode_write(0x063820, 0x00000101, 4);
		if (ret < 0)
#line 666
			ILI_ERR("Write 0x00000101 at 0x063820 failed\n");
		ret = ilitek_ice_mode_write(0x042c34, 0x00000008, 4);
		if (ret < 0)
			ILI_ERR("Write 0x00000008 at 0x042c34 failed\n");
		ret = ilitek_ice_mode_write(0x063820, 0x00000000, 4);
		if (ret < 0)
			ILI_ERR("Write 0x00000000 at 0x063820 failed\n");
	} else {
		ret = ilitek_ice_mode_write(0x063820, 0x00000101, 4);
		if (ret < 0)
#line 675
			ILI_ERR("Write 0x00000101 at 0x063820 failed\n");
		ret = ilitek_ice_mode_write(0x042c34, 0x00000000, 4);
		if (ret < 0)
			ILI_ERR("Write 0x00000000 at 0x042c34 failed\n");
		ret = ilitek_ice_mode_write(0x063820, 0x00000000, 4);
		if (ret < 0)
			ILI_ERR("Write 0x00000000 at 0x063820 failed\n");
	}
}

/*
 * ilitek_tddi_ic_get_project_id() was reconstructed from the factory kernel disassembly (0xffffff8008a59704, 1648 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_get_project_id(u8 *buf, int size)
{
	int i = 0;
	int ice = idev->c712;
	u32 tmp;

	if (!buf) {
#line 796
		ILI_ERR("pdata is null\n");
		return -ENOMEM;
	}

	ILI_INFO("Read size = %d\n", size);

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
			ILI_ERR("Enable ice mode failed while reading project id\n");

	if (ilitek_ice_mode_write(0x041000, 0x0, 1) < 0)
		ILI_ERR("Pull cs low failed\n");

	/* "52954aaa mov"@0xffffff8008a5986c = 0xaa55, "52800ccb mov"
	 * @0xffffff8008a59870 = 0x66, len+4 = 7 -> 3 bytes of data */
	if (ilitek_ice_mode_write(0x041004, 0x66AA55, 3) < 0)
#line 809
		ILI_ERR("Write key failed\n");

	if (ilitek_ice_mode_write(0x041008, 0x03, 1) < 0)
		ILI_ERR("Write 0x03 at 0x041008\n");

	if (ilitek_ice_mode_write(0x041008, 0x01, 1) < 0)
		ILI_ERR("Write address failed\n");

	if (ilitek_ice_mode_write(0x041008, 0xD0, 1) < 0)
#line 817
		ILI_ERR("Write address failed\n");

	if (ilitek_ice_mode_write(0x041008, 0x00, 1) < 0)
#line 819
		ILI_ERR("Write address failed\n");

	for (i = 0; i < size; i++) {
		if (ilitek_ice_mode_write(0x041008, 0xFF, 1) < 0)
			ILI_ERR("Write dummy failed\n");

		if (ilitek_ice_mode_read(0x041010, &tmp, sizeof(u8)) < 0)
#line 825
			ILI_ERR("Read project id error\n");

		buf[i] = tmp;
#line 827
		ILI_INFO("project_id[%d] = 0x%x\n", i, buf[i]);
	}

	ilitek_tddi_flash_clear_dma();

	if (ilitek_ice_mode_write(0x041000, 0x1, 1) < 0)
		ILI_ERR("Pull cs high\n");

	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Disable ice mode failed while reading project id\n");

	return 0;
}

/*
 * ilitek_tddi_ic_get_core_ver() was reconstructed from the factory kernel disassembly (0xffffff8008a59d74, 408 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_get_core_ver(void)
{
	int ret = 0;
	u8 cmd[2] = {0};
	u8 buf[10] = {0};

	if (idev->c636) {
		buf[1] = idev->c452[68];
		buf[2] = idev->c452[69];
		buf[3] = idev->c452[70];
		goto out;
	}

	cmd[0] = 0xF6;
	cmd[1] = 0x23;

	if (idev->c776(cmd, 2) < 0) {
#line 859
		ILI_ERR("write core ver err\n");
		ret = -1;
		goto out;
	}

	if (idev->c776(cmd + 1, 1) < 0) {
		ILI_ERR("write core ver err\n");
		ret = -1;
		goto out;
	}

	if (idev->c784(buf,
		       ((struct ilitek_protocol_info *)idev->c56)->c24) < 0) {
#line 871
		ILI_ERR("i2c/spi read core ver err\n");
		ret = -1;
		goto out;
	}

	if (buf[0] != 0x23) {
		ILI_ERR("Invalid core ver\n");
		ret = -1;
	}

out:
	ILI_INFO("Core version = %d.%d.%d\n", buf[1], buf[2], buf[3]);
	IC->c52 = buf[1] << 16 | buf[2] << 8 | buf[3];
	return ret;
}

/*
 * ilitek_tddi_fw_uart_ctrl() was reconstructed from the factory kernel disassembly (0xffffff8008a59f0c, 248 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_fw_uart_ctrl(u8 ctrl)
{
	u8 cmd[4] = {0};

	if (ctrl > 1) {
#line 892
		ILI_INFO("Unknown cmd, ignore\n");
		return;
	}

	ILI_INFO("%s UART mode\n", (ctrl ? "Enable" : "Disable"));

	cmd[0] = 0x40;
	cmd[1] = 0x03;
	cmd[2] = 0x00;
	cmd[3] = ctrl;

	if (idev->c776(cmd, 4) < 0) {
		ILI_INFO("Write fw uart cmd failed\n");
		return;
	}

	idev->c630 = !!ctrl;
}

/*
 * ilitek_tddi_ic_get_fw_ver() was reconstructed from the factory kernel disassembly (0xffffff8008a5a004, 456 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_get_fw_ver(void)
{
	int ret = 0;
	u8 cmd[2] = {0};
	u8 buf[10] = {0};
	char str[512];

	if (idev->c636) {
		buf[1] = idev->c452[48];
		buf[2] = idev->c452[49];
		buf[3] = idev->c452[50];
		buf[4] = idev->c452[51];
		goto out;
	}

	cmd[0] = 0xF6;
	cmd[1] = 0x21;

	if (idev->c776(cmd, 2) < 0) {
#line 934
		ILI_ERR("write firmware ver err\n");
		ret = -1;
		goto out;
	}

	if (idev->c776(cmd + 1, 1) < 0) {
		ILI_ERR("write firmware ver err\n");
		ret = -1;
		goto out;
	}

	if (idev->c784(buf,
		       ((struct ilitek_protocol_info *)idev->c56)->c4) < 0) {
#line 946
		ILI_ERR("i2c/spi read firmware ver err\n");
		ret = -1;
		goto out;
	}

	if (buf[0] != 0x21) {
		ILI_ERR("Invalid firmware ver\n");
		ret = -1;
	}

out:
	ILI_INFO("Firmware version = %d.%d.%d.%d\n", buf[1], buf[2], buf[3],
#line 957
		 buf[4]);
	IC->c48 = buf[1] << 24 | buf[2] << 16 | buf[3] << 8 | buf[4];

	fix_tp_proc_info(str,
			 sprintf(str,
				 "TP IC: ILITEK,TP MODULE: hongzhan,TP I2C ADR: 0x41,SW FirmWare: 0x%06X,Sample FirmWare: 0x%06X",
				 IC->c48, IC->c48));
	return ret;
}

/*
 * ilitek_tddi_ic_get_panel_info() was reconstructed from the factory kernel disassembly (0xffffff8008a5a1cc, 536 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_get_panel_info(void)
{
	int ret = 0;
	u8 cmd = 0x29;
	u8 buf[10] = {0};
	/*
	 * TWO LOCALS, and the measurement forces them (class A2 the other way round):
	 * "a9432149 ldp"@0xffffff8008a5a208 reads `idev->c48` and `idev->c56`
	 * in one go, "39405113 ldrb"@0xffffff8008a5a214 takes the LENGTH
	 * as ONE BYTE -- it is the local that is `u8`, not the field, which the
	 * other four reads take as 32-bit -- and "b9403536 ldr"
	 * @0xffffff8008a5a218 the version. Both survive three opaque calls in
	 * callee-saved registers.
	 */
	u32 ver = IC->c52;
	u8 len = ((struct ilitek_protocol_info *)idev->c56)->c20;

	/* "51000aa8 sub"@0xffffff8008a5a220 = 0x10402 - 2 */
	if (idev->c636 && ver > 0x010400) {
		buf[1] = idev->c452[16];
		buf[2] = idev->c452[17];
		buf[3] = idev->c452[18];
		buf[4] = idev->c452[19];
		idev->c244 = buf[1] | (buf[2] << 8);
		idev->c246 = buf[3] | (buf[4] << 8);
		goto out;
	}

	if (idev->c776(&cmd, sizeof(cmd)) < 0)
#line 987
		ILI_ERR("Write panel info error\n");

	/* "1a938121 csel"@0xffffff8008a5a29c */
	ret = idev->c784(buf, (ver > 0x010402) ? 6 : len);
	if (ret < 0)
#line 991
		ILI_ERR("Read panel info error\n");

	if (buf[0] != 0x29) {
		ILI_INFO("Invalid panel info, use default resolution\n");
		idev->c244 = 1080;
		idev->c246 = 2340;
		idev->c641 = 0;
	} else {
		idev->c244 = buf[1] << 8 | buf[2];
		idev->c246 = buf[3] << 8 | buf[4];
		idev->c641 = (IC->c52 > 0x010402) && buf[5];
		ILI_INFO("Transfer touch coordinate = %s\n",
#line 1002
			 idev->c641 ? "ON" : "OFF");
	}

out:
	ILI_INFO("Panel info: width = %d, height = %d\n", idev->c244,
#line 1006
		 idev->c246);
	return ret;
}

/*
 * ilitek_tddi_ic_get_tp_info() was reconstructed from the factory kernel disassembly (0xffffff8008a5a3e4, 568 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_get_tp_info(void)
{
	int ret = 0;
	u8 cmd[2] = {0};
	u8 buf[20] = {0};

	/* "5280802a mov"@0xffffff8008a5a424 + "72a0002a movk"@0xffffff8008a5a428 */
	if (idev->c636 && IC->c52 >= 0x010401) {
		buf[1] = idev->c452[5];
		buf[2] = idev->c452[7];
		buf[3] = idev->c452[8];
		buf[4] = idev->c452[9];
		buf[5] = idev->c452[10];
		buf[6] = idev->c452[11];
		buf[11] = buf[7] = idev->c452[12];
		buf[12] = buf[8] = idev->c452[14];
		goto out;
	}

	cmd[0] = 0xF6;
	cmd[1] = 0x20;

	if (idev->c776(cmd, 2) < 0) {
#line 1034
		ILI_ERR("Write tp info error\n");
		ret = -1;
		goto out;
	}

	if (idev->c776(cmd + 1, 1) < 0) {
		ILI_ERR("Write tp info error\n");
		ret = -1;
		goto out;
	}

	if (idev->c784(buf,
		       ((struct ilitek_protocol_info *)idev->c56)->c12) < 0) {
#line 1046
		ILI_ERR("Read tp info error\n");
		ret = -1;
		goto out;
	}

	if (buf[0] != 0x20) {
		ILI_ERR("Invalid tp info\n");
		ret = -1;
	}

out:
	idev->c240 = buf[1];
	idev->c242 = buf[2];
	idev->c236 = buf[3] | (buf[4] << 8);
	idev->c238 = buf[5] | (buf[6] << 8);
	idev->c248 = buf[7];
	idev->c249 = buf[8];
	idev->c250 = buf[11];
	idev->c251 = buf[12];

	ILI_INFO("TP Info: min_x = %d, min_y = %d, max_x = %d, max_y = %d\n",
		 idev->c240, idev->c242, idev->c236, idev->c238);
	ILI_INFO("TP Info: xch = %d, ych = %d, stx = %d, srx = %d\n",
#line 1068
		 idev->c248, idev->c249, idev->c250, idev->c251);
	return ret;
}

/*
 * ilitek_tddi_ic_check_protocol_ver() was reconstructed from the factory kernel disassembly (0xffffff8008a5a61c, 672 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ilitek_tddi_ic_check_protocol_ver(u32 pver)
{
	int i = 0;

	if (((struct ilitek_protocol_info *)idev->c56)->c0 == pver) {
#line 1077
		ILI_INFO("same procotol version, do nothing\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(protocol_info) - 1; i++) {
		if (protocol_info[i].c0 == pver) {
			idev->c56 = &protocol_info[i];
			ILI_INFO("update protocol version = %x\n",
#line 1084
				 ((struct ilitek_protocol_info *)idev->c56)->c0);
			return;
		}
	}

	ILI_INFO("Not found a correct protocol version in list, use newest version\n");
	idev->c56 = &protocol_info[ARRAY_SIZE(protocol_info) - 1];
}

int ilitek_tddi_ic_get_protocl_ver(void)
{
	int ret = 0;
	u32 ver;
	u8 cmd[2] = {0};
	u8 buf[10] = {0};

	if (idev->c636) {
		buf[1] = idev->c452[72];
		buf[2] = idev->c452[73];
		buf[3] = idev->c452[74];
		goto out;
	}

	cmd[0] = 0xF6;
	cmd[1] = 0x22;

	if (idev->c776(cmd, 2) < 0) {
		ILI_ERR("Write protocol version error\n");
		ret = -1;
		goto out;
	}

	if (idev->c776(cmd + 1, 1) < 0) {
		ILI_ERR("Write protocol version error\n");
		ret = -1;
		goto out;
	}

	if (idev->c784(buf,
		       ((struct ilitek_protocol_info *)idev->c56)->c8) < 0) {
#line 1123
		ILI_ERR("Read protocol version error\n");
		ret = -1;
		goto out;
	}

	if (buf[0] != 0x22) {
		ILI_ERR("Invalid protocol ver\n");
		ret = -1;
	}

out:
	ver = buf[1] << 16 | buf[2] << 8 | buf[3];
	ilitek_tddi_ic_check_protocol_ver(ver);

	ILI_INFO("Protocol version = %d.%d.%d\n",
		 ((struct ilitek_protocol_info *)idev->c56)->c0 >> 16,
		 (((struct ilitek_protocol_info *)idev->c56)->c0 >> 8) & 0xFF,
		 ((struct ilitek_protocol_info *)idev->c56)->c0 & 0xFF);
	return ret;
}

/*
 * ilitek_tddi_ic_get_info() was reconstructed from the factory kernel disassembly (0xffffff8008a5a8bc, 436 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_ic_get_info(void)
{
	int ret = 0;
	u32 pid = IC->c8;

	if (!idev->c712) {
#line 1150
		ILI_ERR("ice mode doesn't enable\n");
		return -1;
	}

	if (!pid) {
		ret = ilitek_ice_mode_read(IC->c12, &pid, sizeof(u32));
		if (ret < 0)
#line 1156
			ILI_ERR("Read chip id error\n");
	}

	ret = ilitek_ice_mode_read(IC->c32, &IC->c40, sizeof(u32));
	if (ret < 0)
#line 1159
		ILI_ERR("Read otp id error\n");

	ret = ilitek_ice_mode_read(IC->c36, &IC->c44, sizeof(u32));
	if (ret < 0)
#line 1161
		ILI_ERR("Read ana id error\n");

	IC->c8 = pid;
	IC->c2 = pid >> 16;
	IC->c0 = pid >> 8;
	IC->c1 = pid;
	IC->c40 &= 0xFF;
	IC->c44 &= 0xFF;

	ILI_INFO("CHIP: PID = %x, ID = %x, TYPE = %x, VER = %x, OTP = %x, ANA = %x\n",
#line 1176
		 IC->c8, IC->c2, IC->c0, IC->c1, IC->c40, IC->c44);

	return ilitek_tddi_ic_check_support(IC->c8, IC->c2);
}

/*
 * ilitek_tddi_ic_init() was reconstructed from the factory kernel disassembly (0xffffff8008a5aa70, 104 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_ic.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_ic_init(void)
{
	chip.c12 = 0x4009C;
	chip.c16 = 0x5100C;
	chip.c20 = 0x44008;
	chip.c24 = 0x51010;
	chip.c28 = 0x40050;
	chip.c32 = 0x400A0;
	chip.c36 = 0x400A4;
	chip.c68 = 1;

	idev->c48 = &chip;
	idev->c56 = &protocol_info[ARRAY_SIZE(protocol_info) - 1];
}
