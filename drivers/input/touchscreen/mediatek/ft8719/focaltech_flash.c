// SPDX-License-Identifier: GPL-2.0
/*
 * FocalTech FT8719 touch panel, Doogee S88 Pro (MT6771) -- the flash and boot
 * state translation unit. Complete: seventeen functions out of seventeen.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read: the ALPS tree this file builds against contains public FocalTech
 * drivers (focaltech_fhd_touch/, focaltech_touch/) which were not opened.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7e3a8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

/*
 * FTS_INFO() was reconstructed from the factory kernel disassembly (0xffffff800924e4d6).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define FTS_INFO(fmt, args...)		printk(KERN_INFO "[FTS][Info]" fmt "\n", ##args)
#define FTS_ERROR(fmt, args...)		printk(KERN_ERR "[FTS][Error]" fmt "\n", ##args)

/*
 * FTS_DEBUG() was reconstructed from the factory kernel disassembly (0xffffff800924e7b4).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define FTS_DEBUG(fmt, args...)		printk("[FTS]" fmt "\n", ##args)

/*
 * ==========================================================================
 * THE CONSTANTS, read from the immediates
 * ==========================================================================
 * THE PACKET SIZE, 32.  It is not written as 32 in any single place: it
 * appears as `lsr #5` for the division, `#0x1f` for the remainder and `#0x20`
 * for the stride, and the three sit together in fts_flash_read_buf:
 *   "53057c68 lsr"@0xffffff8008a7e058   packet_number = len >> 5
 *   "7200107f tst"@0xffffff8008a7e05c   len & 0x1f
 *   "12001076 and"@0xffffff8008a7e064   remainder = len & 0x1f
 *   "321b03f8 orr"@0xffffff8008a7e09c   w24 = 0x20
 *   "11008294 add"@0xffffff8008a7e0fc   w20 += 0x20  (the address)
 *   "91008273 add"@0xffffff8008a7e104   x19 += 0x20  (the buffer)
 * `lsr` and not `asr`, and `and` with no sign correction: `len` is UNSIGNED
 * (class A3).
 */
#define FTS_FLASH_PACKET_LENGTH		32

/*
 * The flash read command and its length:
 *   "320007e8 orr"@0xffffff8008a7e088   w8 = 3, then
 *   "390013e8 strb"@0xffffff8008a7e08c  packet_buf[0] = 3
 *   "321e03e2 orr"@0xffffff8008a7e0b4   w2 = 4, the writelen of the i2c_write
 *   "320003e0 orr"@0xffffff8008a7e0d4   w0 = 1, the argument of msleep
 */
#define FTS_CMD_READ			0x03
#define FTS_CMD_READ_LEN		4
#define FTS_CMD_READ_DELAY		1

/*
 * The flash write command, its six-byte header and the status
 * command:
 *   "528017e8 mov"@0xffffff8008a7de78   w8 = 0xbf, then
 *   "3900c3e8 strb"@0xffffff8008a7de7c  packet_buf[0] = 0xbf
 *   "1100190b add"@0xffffff8008a7defc   the data index is j + 6
 *   "11001aa2 add"@0xffffff8008a7df14   the writelen is packet_len + 6
 *   "52800d58 mov"@0xffffff8008a7dea8   w24 = 0x6a, then
 *   "3900b3f8 strb"@0xffffff8008a7df6c  cmd = 0x6a
 *   "7101933f cmp"@0xffffff8008a7df98   attempt number 100
 */
#define FTS_CMD_WRITE			0xBF
#define FTS_CMD_WRITE_LEN		6
#define FTS_CMD_FLASH_STATUS		0x6A
#define FTS_RETRIES_WRITE		100

/*
 * The two bytes of the enter-boot command, and the delay that follows them:
 *   "52954aa8 mov"@0xffffff8008a7d3dc   w8 = 0xaa55, then
 *   "79000be8 strh"@0xffffff8008a7d3ec  cmd[0]=0x55, cmd[1]=0xaa
 *   "52800140 mov"@0xffffff8008a7d3f8   w0 = 10, the argument of msleep
 *   "52801209 mov"@0xffffff8008a7d404   w9 = 0x90, then
 *   "b90007e9 str"@0xffffff8008a7d408   cmd[0..3] = 0x90,0,0,0 -- a 32-bit
 *                                       `str`, not a `strb`
 */
#define FTS_CMD_START1			0x55
#define FTS_CMD_START2			0xAA
#define FTS_CMD_START_DELAY		10
#define FTS_CMD_READ_ID			0x90

/*
 * The boot state wait loop, in fts_fwupg_check_state:
 *   "52800280 mov"@0xffffff8008a7d7ac   w0 = 20, the argument of msleep
 *   "71007abf cmp"@0xffffff8008a7d7b8   round number 30
 */
#define FTS_UPGRADE_LOOP		30
#define FTS_DELAY_UPGRADE_LOOP		20

/*
 * The reset command and the delay after it, in fts_fwupg_reset_in_boot
 * and in its inlined copy inside fts_flash_read:
 *   "32000be9 orr"@0xffffff8008a7d858   w9 = 7   (out of line)
 *   "32000be8 orr"@0xffffff8008a7e21c   w8 = 7   (inlined)
 *   "52800a00 mov"@0xffffff8008a7d880   w0 = 0x50 = 80  (out of line)
 *   "52800a00 mov"@0xffffff8008a7e240   w0 = 0x50 = 80  (inlined)
 */
#define FTS_REG_RESET_FW		0x07
#define FTS_DELAY_UPGRADE_RESET		80

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7e914).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define FTS_REG_FW_VER			0xa6
#define FTS_REG_PARAM_VER		0xb5
#define FTS_REG_PARAM_STATUS		0xb6

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7d470).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define FTS_RUN_IN_ROM			2
#define FTS_RUN_IN_PRAM			3
#define FTS_RUN_IN_BOOTLOADER		4

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7d410, 384 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct fts_ts_data {
	u8 c0[16];		/* not touched by these six functions */
	u8 c16;
	u8 c17[17];		/* not touched by these six functions */
	u8 c34;
	u8 c35;
	u8 c36;
	u8 c37;
	u8 c38;
	u8 c39;
	u8 c40[344];		/* up to 384 = the size measured by unit C */
};

/*
 * `fts_data` is the ONLY global the binary names, and it names it in plain
 * sight in unit C's message:
 *   "\x013[FTS][Error]Failed to allocate memory for fts_data\n"@0xffffff800924f718
 * It sits at 0xffffff800a100ad8, unit C (focaltech_core.c) defines it and here
 * it is `extern`.  That it is not `static` there is PROVEN: nineteen functions
 * spread over four translation units read it, among them the two in this
 * file.
 */
extern struct fts_ts_data *fts_data;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a100ac8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct fts_upgrade_func {
	u8 c0[32];		/* not touched by the functions written so far */
	u32 c32;		/* FOUR bytes: "b940218c ldr"@0xffffff8008a7cee0
				 * and "b9402129 ldr"@0xffffff8008a7cf2c are
				 * `ldr w`, not `ldrb`.  Compared with zero by
				 * "7100019f cmp"@0xffffff8008a7cee4: if it is zero
				 * the pramboot checksum is an XOR of bytes,
				 * if it is not it is a CRC-16 with
				 * polynomial 0x8408
				 */
	u32 c36;		/* FOUR bytes: "b9402502 ldr"@0xffffff8008a7e68c and
				 * "b9402508 ldr"@0xffffff8008a7e918 are `ldr w`.
				 * It is the offset, inside the firmware, of the
				 * version byte: "b9400aa1 ldr"@0xffffff8008a7e688
				 * loads the length and
				 * "6b02003f cmp"@0xffffff8008a7e690 compares it
				 * with this field before indexing with
				 * "38686935 ldrb"@0xffffff8008a7e920
				 */
	u8 c40[16];		/* not touched, up to +56 */
	u32 c56;		/* FOUR bytes: "b9403902 ldr"@0xffffff8008a7e7c8
				 * and "b9403908 ldr"@0xffffff8008a7e7ec are `ldr w`.
				 * Same role as c36 but for the `param` path:
				 * "6b02003f cmp"@0xffffff8008a7e7cc and then
				 * "3868693a ldrb"@0xffffff8008a7e7f0
				 */
	u8 c60[4];		/* not touched, up to +64 */
	u8 c64;			/* "39410108 ldrb"@0xffffff8008a7d3cc, then
				 * "34000068 cbz"@0xffffff8008a7d3d0: if it is
				 * non-zero fts_i2c_hid2std is called
				 */
	u8 c65;			/* "39410508 ldrb"@0xffffff8008a7d9f8, then
				 * "34000188 cbz"@0xffffff8008a7d9fc: if it is
				 * non-zero the pramboot is written, if it is zero
				 * we go to "confirm in bootloader".  ONE BYTE:
				 * the read is `ldrb`, not `ldr`
				 */
	u8 c66[6];		/* not touched, up to +72 */
	u8 *c72;		/* EIGHT bytes: "f9402517 ldr"@0xffffff8008a7cd20
				 * is an `ldr x`; the value is used as the base of
				 * "386b4aeb ldrb"@0xffffff8008a7cecc, that is, it is
				 * a pointer to bytes
				 */
	u32 c80;		/* FOUR bytes: "b9405114 ldr"@0xffffff8008a7cd28
				 * is `ldr w`; it is the length, compared
				 * UNSIGNED by "71047e9f cmp"@0xffffff8008a7cd2c
				 * + "540005a8 b.hi"@0xffffff8008a7cd30
				 */
	u8 c84[12];		/* not touched, up to +96 */
	int (*c96)(struct i2c_client *client, u8 *buf, u32 len);
				/*
				 * EIGHT bytes: "f9403108 ldr"@0xffffff8008a7e740 and
				 * "f9403108 ldr"@0xffffff8008a7e534 are `ldr x`,
				 * and the value ends up in an indirect call,
				 * "d63f0100 blr"@0xffffff8008a7e754.  THE THREE
				 * ARGUMENTS are read from the registers at the site:
				 * "aa1303e0 mov"@0xffffff8008a7e750 (x0=client),
				 * "f9400281 ldr"@0xffffff8008a7e748 (x1=fw) and
				 * "b9400a82 ldr"@0xffffff8008a7e74c (w2=len, 32-bit)
				 */
	u8 c104[16];		/* not touched, up to +120 */
	int (*c120)(struct i2c_client *client, u8 *buf, u32 len);
				/*
				 * EIGHT bytes: "f9403d08 ldr"@0xffffff8008a7e780 and
				 * "f9403d08 ldr"@0xffffff8008a7e894, `ldr x`,
				 * called by "d63f0100 blr"@0xffffff8008a7e898
				 * with the same three arguments as c96
				 */
	int (*c128)(struct i2c_client *client, u8 *buf, u32 len);
				/*
				 * EIGHT bytes: "f9404108 ldr"@0xffffff8008a7e520,
				 * `ldr x`, called by the same
				 * "d63f0100 blr"@0xffffff8008a7e548 as c96 --
				 * the two branches of fts_upgrade_bin converge on the
				 * call
				 */
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998d7e0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

struct fts_upgrade {
	u8 *c0;			/* EIGHT bytes: "f94002a9 ldr"@0xffffff8008a7e680 and
				 * "f9400349 ldr"@0xffffff8008a7e7bc are `ldr x`,
				 * and the value is used as the base of
				 * "38686935 ldrb"@0xffffff8008a7e920, that is, it is a
				 * pointer to bytes.  It is also the second
				 * argument of the two indirect calls,
				 * "f9400281 ldr"@0xffffff8008a7e748
				 */
	u32 c8;			/* FOUR bytes: "b9400aa1 ldr"@0xffffff8008a7e688
				 * is `ldr w`; it is the length, compared UNSIGNED
				 * with c36 by "6b02003f cmp"@0xffffff8008a7e690
				 * + "54001322 b.cs"@0xffffff8008a7e694
				 */
	u8 c12[20];		/* not touched, up to +32 */
	struct fts_upgrade_func *c32;
				/* "f9401288 ldr"@0xffffff8008a7d3c4 */
};

struct fts_upgrade *fts_g_a100ac8;

/*
 * ==========================================================================
 * THE BOUNDARY WITH THE OTHER UNITS -- DECLARED, NOT DEFINED HERE
 * ==========================================================================
 * The signatures come from the registers at the call sites.
 */
int fts_i2c_read(struct i2c_client *client, char *writebuf, int writelen,
		 char *readbuf, int readlen);	/* unita' E, 0xffffff8008a802ac */
int fts_i2c_write(struct i2c_client *client, char *writebuf, int writelen);
						/* unita' E, 0xffffff8008a80494 */
void fts_i2c_hid2std(struct i2c_client *client);
						/* unita' E, 0xffffff8008a80668 */
int fts_i2c_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue);
						/* unita' E, 0xffffff8008a805c4 */
int fts_i2c_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue);
						/* unita' E, 0xffffff8008a80614 */
int fts_wait_tp_to_valid(struct i2c_client *client);
						/* unita' C, 0xffffff8008a7eac0 */

/*
 * fts_pram_init() was reconstructed from the factory kernel disassembly (0xffffff8008a7e29c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_pram_init(struct i2c_client *client);
						/*
						 * 0xffffff8008a7d544, `t` in the
						 * map: static.  Two call
						 * sites inside
						 * fts_pram_write_init,
						 * "94000218 bl"@0xffffff8008a7cce4
						 * and "94000087 bl"@0xffffff8008a7d328
						 */
int fts_fwupg_get_boot_state(struct i2c_client *client, int *fw_sts);
						/* 0xffffff8008a7d34c */
int fts_fwupg_reset_to_romboot(struct i2c_client *client);
						/* 0xffffff8008a7d624 */
bool fts_fwupg_check_state(struct i2c_client *client, int rstate);
						/* 0xffffff8008a7d764 */

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7cc4c, 1792 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * fts_crc16_calc_host() was reconstructed from the factory kernel disassembly (0xffffff8008a7cf48).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static u16 fts_crc16_calc_host(u8 *pbuf, u16 length)
{
	u16 ecc = 0;
	u16 i = 0;
	u16 j = 0;

	for (i = 0; i < length; i += 2) {
		ecc ^= ((pbuf[i] << 8) | (pbuf[i + 1]));
		for (j = 0; j < 16; j++) {
			if (ecc & 0x01)
				ecc = (u16)((ecc >> 1) ^ 0x8408);
			else
				ecc >>= 1;
		}
	}

	return ecc;
}

/*
 * fts_pram_write_buf() was reconstructed from the factory kernel disassembly (0xffffff8008a7ce00).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_pram_write_buf(struct i2c_client *client, u8 *buf, u32 len)
{
	int ret = 0;
	u32 i = 0;
	u32 j = 0;
	u32 offset = 0;
	u32 remainder = 0;
	u32 packet_number = 0;
	u32 packet_len = 0;
	u8 packet_buf[FTS_FLASH_PACKET_LENGTH + FTS_CMD_WRITE_LEN] = { 0 };
	u8 ecc_tmp = 0;
	int ecc_in_host = 0;

	FTS_INFO("write pramboot to pram");
	if ((NULL == fts_g_a100ac8) && (NULL == fts_g_a100ac8->c32)) {
		FTS_ERROR("fwupgrade/func is null");
		return -EINVAL;
	}

	FTS_INFO("pramboot len=%d", len);
	if ((len < 0x120) || (len > 0x10000)) {
		FTS_ERROR("pramboot length(%d) fail", len);
		return -EINVAL;
	}

	packet_number = len / FTS_FLASH_PACKET_LENGTH;
	remainder = len % FTS_FLASH_PACKET_LENGTH;
	if (remainder > 0)
		packet_number++;
	packet_len = FTS_FLASH_PACKET_LENGTH;

	packet_buf[0] = 0xAE;
	for (i = 0; i < packet_number; i++) {
		offset = i * FTS_FLASH_PACKET_LENGTH;
		packet_buf[1] = (u8)((offset >> 16) & 0xFF);
		packet_buf[2] = (u8)((offset >> 8) & 0xFF);
		packet_buf[3] = (u8)(offset & 0xFF);

		if ((i == (packet_number - 1)) && remainder)
			packet_len = remainder;

		packet_buf[4] = (u8)((packet_len >> 8) & 0xFF);
		packet_buf[5] = (u8)(packet_len & 0xFF);

		for (j = 0; j < packet_len; j++) {
			packet_buf[FTS_CMD_WRITE_LEN + j] = buf[offset + j];
			if (0 == fts_g_a100ac8->c32->c32)
				ecc_tmp ^= packet_buf[FTS_CMD_WRITE_LEN + j];
		}

		ret = fts_i2c_write(client, packet_buf,
				    packet_len + FTS_CMD_WRITE_LEN);
		if (ret < 0) {
			FTS_ERROR("pramboot write data(%d) fail", i);
			return ret;
		}
	}

	if (0 == fts_g_a100ac8->c32->c32)
		ecc_in_host = (int)ecc_tmp;
	else
		ecc_in_host = (int)fts_crc16_calc_host(buf, len);

	return ecc_in_host;
}

/*
 * fts_pram_ecc_cal_algo() was reconstructed from the factory kernel disassembly (0xffffff8008a7d0f4).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_pram_ecc_cal_algo(struct i2c_client *client, u32 saddr, u32 len)
{
	int ret = 0;
	u32 i = 0;
	u8 val[2] = { 0 };
	u8 cmd[7] = { 0 };

	FTS_INFO("read out pramboot checksum");

	cmd[0] = 0xCC;
	cmd[1] = (u8)((saddr >> 16) & 0xFF);
	cmd[2] = (u8)((saddr >> 8) & 0xFF);
	cmd[3] = (u8)(saddr & 0xFF);
	cmd[4] = (u8)((len >> 16) & 0xFF);
	cmd[5] = (u8)((len >> 8) & 0xFF);
	cmd[6] = (u8)(len & 0xFF);
	ret = fts_i2c_write(client, cmd, 7);
	if (ret < 0) {
		FTS_ERROR("write pramboot ecc cal cmd fail");
		return ret;
	}

	cmd[0] = 0xCE;
	for (i = 0; i < 100; i++) {
		msleep(1);
		ret = fts_i2c_read(client, cmd, 1, val, 1);
		if (ret < 0) {
			FTS_ERROR("ecc_finish read cmd fail");
			return ret;
		}

		if (0 == val[0])
			break;
	}
	if (i >= 100) {
		FTS_ERROR("wait ecc finish fail");
		return -EIO;
	}

	cmd[0] = 0xCD;
	ret = fts_i2c_read(client, cmd, 1, val, 2);
	if (ret < 0) {
		FTS_ERROR("read pramboot ecc fail");
		return ret;
	}

	return (int)((val[0] << 8) + val[1]);
}

static int fts_pram_ecc_cal_xor(struct i2c_client *client)
{
	int ret = 0;
	u8 reg_val = 0;

	FTS_INFO("read out pramboot checksum");

	ret = fts_i2c_read_reg(client, 0xCC, &reg_val);
	if (ret < 0) {
		FTS_ERROR("read pramboot ecc fail");
		return ret;
	}

	return (int)reg_val;
}

static int fts_pram_ecc_cal(struct i2c_client *client, u32 saddr, u32 len)
{
	int ret = 0;

	if ((NULL == fts_g_a100ac8) && (NULL == fts_g_a100ac8->c32)) {
		FTS_ERROR("fwupgrade/func is null");
		return -EINVAL;
	}

	if (0 != fts_g_a100ac8->c32->c32)
		ret = fts_pram_ecc_cal_algo(client, saddr, len);
	else
		ret = fts_pram_ecc_cal_xor(client);

	return ret;
}

/*
 * fts_pram_start() was reconstructed from the factory kernel disassembly (0xffffff8008a7d1d0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_pram_start(struct i2c_client *client)
{
	u8 cmd = 0x08;
	int ret = 0;

	FTS_INFO("remap to start pramboot");

	ret = fts_i2c_write(client, &cmd, 1);
	if (ret < 0) {
		FTS_ERROR("write start pram cmd fail");
		return ret;
	}

	msleep(10);

	return 0;
}

/*
 * fts_pram_write_remap() was reconstructed from the factory kernel disassembly (0xffffff8008a7cd04).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_pram_write_remap(struct i2c_client *client)
{
	int ret = 0;
	int ecc_in_host = 0;
	int ecc_in_tp = 0;
	u8 *pb_buf = NULL;
	u32 pb_len = 0;
	struct fts_upgrade *upg = fts_g_a100ac8;

	FTS_INFO("write pram and remap");
	if ((NULL == upg) || (NULL == upg->c32) || (NULL == upg->c32->c72)) {
		FTS_ERROR("upgrade/pramboot is null");
		return -EINVAL;
	}

	pb_buf = upg->c32->c72;
	pb_len = upg->c32->c80;
	if (pb_len < 0x120) {
		FTS_ERROR("pramboot length(%d) fail", pb_len);
		return -EINVAL;
	}

	ecc_in_host = fts_pram_write_buf(client, pb_buf, pb_len);
	if (ecc_in_host < 0) {
		FTS_ERROR("write pramboot fail");
		return ecc_in_host;
	}

	ecc_in_tp = fts_pram_ecc_cal(client, 0, pb_len);
	if (ecc_in_tp < 0) {
		FTS_ERROR("read pramboot ecc fail");
		return ecc_in_tp;
	}

	FTS_INFO("pram ecc in tp:%x, host:%x", ecc_in_tp, ecc_in_host);
	if (ecc_in_host != ecc_in_tp) {
		FTS_ERROR("pramboot ecc check fail");
		return -EIO;
	}

	ret = fts_pram_start(client);
	if (ret < 0) {
		FTS_ERROR("pram start fail");
		return ret;
	}

	return 0;
}

/*
 * fts_pram_write_init() was reconstructed from the factory kernel disassembly (0xffffff8008a7cc4c, 1792 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_pram_write_init(struct i2c_client *client)
{
	int ret = 0;
	bool state = false;
	int fw_sts = 0;
	struct fts_upgrade *upg = fts_g_a100ac8;

	FTS_INFO("**********pram write and init**********");
	if ((NULL == upg) || (NULL == upg->c32)) {
		FTS_ERROR("upgrade/func is null");
		return -EINVAL;
	}

	if (!upg->c32->c65) {
		FTS_ERROR("ic not support pram");
		return -EINVAL;
	}

	FTS_DEBUG("check whether tp is in romboot or not ");
	fts_fwupg_get_boot_state(client, &fw_sts);
	if (FTS_RUN_IN_ROM != fw_sts) {
		if (FTS_RUN_IN_PRAM == fw_sts) {
			FTS_INFO("tp is in pramboot, need send reset cmd before upgrade");
			ret = fts_pram_init(client);
			if (ret < 0) {
				FTS_ERROR("pramboot(before) init fail");
				return ret;
			}
		}

		FTS_INFO("tp isn't in romboot, need send reset to romboot");
		ret = fts_fwupg_reset_to_romboot(client);
		if (ret < 0) {
			FTS_ERROR("reset to romboot fail");
			return ret;
		}
	}

	ret = fts_pram_write_remap(client);
	if (ret < 0) {
		FTS_ERROR("pram write fail, ret=%d", ret);
		return ret;
	}

	FTS_DEBUG("after write pramboot, confirm run in pramboot");
	state = fts_fwupg_check_state(client, FTS_RUN_IN_PRAM);
	if (!state) {
		FTS_ERROR("not in pramboot");
		return -EIO;
	}

	ret = fts_pram_init(client);
	if (ret < 0) {
		FTS_ERROR("pramboot init fail");
		return ret;
	}

	return 0;
}

/*
 * fts_fwupg_get_boot_state() was reconstructed from the factory kernel disassembly (0xffffff8008a7d34c, 504 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_fwupg_get_boot_state(struct i2c_client *client, int *fw_sts)
{
	int ret = 0;
	u8 cmd[4] = { 0 };
	u8 id[2] = { 0 };
	struct fts_upgrade *upg = fts_g_a100ac8;
	u8 rom_h = fts_data->c34;
	u8 rom_l = fts_data->c35;
	u8 pram_h = fts_data->c36;
	u8 pram_l = fts_data->c37;
	u8 boot_h = fts_data->c38;
	u8 boot_l = fts_data->c39;

	FTS_INFO("**********read boot id**********");
	if ((NULL == fw_sts) || (NULL == upg) || (NULL == upg->c32)) {
		FTS_ERROR("upgrade/func/fw_sts is null");
		return -EINVAL;
	}

	if (upg->c32->c64)
		fts_i2c_hid2std(client);

	cmd[0] = FTS_CMD_START1;
	cmd[1] = FTS_CMD_START2;
	ret = fts_i2c_write(client, cmd, 2);
	if (ret < 0) {
		FTS_ERROR("write 55 aa cmd fail");
		return ret;
	}

	msleep(FTS_CMD_START_DELAY);

	cmd[0] = FTS_CMD_READ_ID;
	cmd[1] = 0x00;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	ret = fts_i2c_read(client, cmd, fts_data->c16 ? 1 : 4, id, 2);
	if (ret < 0) {
		FTS_ERROR("write 90 cmd fail");
		return ret;
	}

	FTS_INFO("read boot id:0x%02x%02x", id[0], id[1]);
	if ((id[0] == rom_h) && (id[1] == rom_l)) {
		FTS_INFO("tp run in romboot");
		*fw_sts = FTS_RUN_IN_ROM;
	} else if ((id[0] == pram_h) && (id[1] == pram_l)) {
		FTS_INFO("tp run in pramboot");
		*fw_sts = FTS_RUN_IN_PRAM;
	} else if ((id[0] == boot_h) && (id[1] == boot_l)) {
		FTS_INFO("tp run in bootloader");
		*fw_sts = FTS_RUN_IN_BOOTLOADER;
	}

	return 0;
}

/*
 * fts_pram_init() was reconstructed from the factory kernel disassembly (0xffffff8008a7d544, 224 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_pram_init(struct i2c_client *client)
{
	int ret = 0;
	u8 reg_val = 0;
	u8 wbuf[3] = { 0 };

	FTS_INFO("pramboot initialization");

	wbuf[0] = 0x05;
	ret = fts_i2c_read(client, wbuf, 1, &reg_val, 1);
	if (ret < 0) {
		FTS_ERROR("read flash type fail");
		return ret;
	}

	wbuf[0] = 0x05;
	wbuf[1] = reg_val;
	wbuf[2] = 0x00;
	ret = fts_i2c_write(client, wbuf, 3);
	if (ret < 0) {
		FTS_ERROR("write flash type fail");
		return ret;
	}

	return 0;
}

/*
 * fts_fwupg_reset_to_romboot() was reconstructed from the factory kernel disassembly (0xffffff8008a7d624, 320 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_fwupg_reset_to_romboot(struct i2c_client *client)
{
	int ret = 0;
	u32 i = 0;
	u8 cmd = FTS_REG_RESET_FW;
	int state = 0;

	ret = fts_i2c_write(client, &cmd, 1);
	if (ret < 0) {
		FTS_ERROR("pram/rom/bootloader reset cmd write fail");
		return ret;
	}
	mdelay(10);

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		fts_fwupg_get_boot_state(client, &state);
		if (FTS_RUN_IN_ROM == state)
			break;

		mdelay(5);
	}
	if (i >= FTS_UPGRADE_LOOP) {
		FTS_ERROR("reset to romboot fail");
		return -EIO;
	}

	return 0;
}

/*
 * fts_fwupg_check_state() was reconstructed from the factory kernel disassembly (0xffffff8008a7d764, 148 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
bool fts_fwupg_check_state(struct i2c_client *client, int rstate)
{
	u32 i = 0;
	int fw_sts = 0;

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		fts_fwupg_get_boot_state(client, &fw_sts);
		if (rstate == fw_sts)
			return true;

		msleep(FTS_DELAY_UPGRADE_LOOP);
	}

	return false;
}

/*
 * fts_fwupg_check_fw_valid() was reconstructed from the factory kernel disassembly (0xffffff8008a7d7f8, 64 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
bool fts_fwupg_check_fw_valid(struct i2c_client *client)
{
	int ret = 0;

	ret = fts_wait_tp_to_valid(client);
	if (ret < 0) {
		FTS_INFO("tp fw invaild");
		return false;
	}

	FTS_INFO("tp fw vaild");
	return true;
}

/*
 * fts_fwupg_reset_in_boot() was reconstructed from the factory kernel disassembly (0xffffff8008a7d838, 148 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_fwupg_reset_in_boot(struct i2c_client *client)
{
	int ret = 0;
	u8 cmd = FTS_REG_RESET_FW;

	FTS_INFO("reset in boot environment");
	ret = fts_i2c_write(client, &cmd, 1);
	if (ret < 0) {
		FTS_ERROR("pram/rom/bootloader reset cmd write fail");
		return ret;
	}

	msleep(FTS_DELAY_UPGRADE_RESET);

	return 0;
}

/*
 * fts_fwupg_reset_to_boot() was reconstructed from the factory kernel disassembly (0xffffff8008a7d8cc, 140 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_fwupg_reset_to_boot(struct i2c_client *client)
{
	int ret = 0;

	FTS_INFO("send 0xAA and 0x55 to FW, reset to boot environment");
	ret = fts_i2c_write_reg(client, 0xFC, 0xAA);
	if (ret < 0) {
		FTS_ERROR("write FC=0xAA fail");
		return ret;
	}

	msleep(10);

	ret = fts_i2c_write_reg(client, 0xFC, 0x55);
	if (ret < 0) {
		FTS_ERROR("write FC=0x55 fail");
		return ret;
	}

	msleep(FTS_DELAY_UPGRADE_RESET);

	return 0;
}

/*
 * fts_fwupg_enter_into_boot() was reconstructed from the factory kernel disassembly (0xffffff8008a7d958, 348 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_fwupg_enter_into_boot(struct i2c_client *client)
{
	int ret = 0;
	struct fts_upgrade *upg = fts_g_a100ac8;

	FTS_INFO("***********enter into pramboot/bootloader***********");
	if ((NULL == upg) || (NULL == upg->c32)) {
		FTS_ERROR("upgrade/func is null");
		return -EINVAL;
	}

	if (fts_fwupg_check_fw_valid(client)) {
		ret = fts_fwupg_reset_to_boot(client);
		if (ret < 0) {
			FTS_ERROR("enter into romboot/bootloader fail");
			return ret;
		}
	}

	if (upg->c32->c65) {
		FTS_INFO("pram supported, write pramboot and init");
		ret = fts_pram_write_init(client);
		if (ret < 0) {
			FTS_ERROR("pram write_init fail");
			return ret;
		}
	} else {
		FTS_DEBUG("pram not supported, confirm in bootloader");
		if (!fts_fwupg_check_state(client, FTS_RUN_IN_BOOTLOADER)) {
			FTS_ERROR("fw not in bootloader, fail");
			return -EIO;
		}
	}

	return 0;
}

/*
 * fts_fwupg_check_flash_status() was reconstructed from the factory kernel disassembly (0xffffff8008a7daf8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_fwupg_check_flash_status(struct i2c_client *client, u16 expect,
					u32 retries, u32 retries_delay)
{
	u32 i = 0;
	u8 cmd = 0;
	u8 value[2] = { 0 };
	u16 read_status = 0;

	for (i = 0; i < retries; i++) {
		cmd = FTS_CMD_FLASH_STATUS;
		fts_i2c_read(client, &cmd, 1, value, 2);
		read_status = (((u16)value[0]) << 8) + value[1];
		if (expect == read_status)
			return 0;

		msleep(retries_delay);
	}

	return -EIO;
}

/*
 * fts_fwupg_erase() was reconstructed from the factory kernel disassembly (0xffffff8008a7dab4, 268 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_fwupg_erase(struct i2c_client *client, u32 delay)
{
	int ret = 0;
	u8 cmd = 0;

	FTS_INFO("**********erase now**********");

	cmd = 0x61;
	ret = fts_i2c_write(client, &cmd, 1);
	if (ret < 0) {
		FTS_ERROR("erase cmd fail");
		return ret;
	}

	msleep(delay);

	ret = fts_fwupg_check_flash_status(client, 0xF0AA, 50, 200);
	if (ret < 0) {
		FTS_ERROR("ecc flash status check fail");
		return ret;
	}

	return 0;
}

/*
 * fts_fwupg_ecc_cal() was reconstructed from the factory kernel disassembly (0xffffff8008a7dbc0, 540 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_fwupg_ecc_cal(struct i2c_client *client, u32 saddr, u32 len)
{
	int ret = 0;
	u32 i = 0;
	u32 packet_number = 0;
	u32 packet_len = 0;
	u32 remainder = 0;
	u32 addr = 0;
	u8 cmd[6] = { 0 };
	u8 val[2] = { 0 };

	FTS_INFO("**********read out checksum**********");

	cmd[0] = 0x64;
	ret = fts_i2c_write(client, cmd, 1);
	if (ret < 0) {
		FTS_ERROR("ecc init cmd write fail");
		return ret;
	}

	packet_number = len / 0xFFFE;
	remainder = len % 0xFFFE;
	if (remainder > 0)
		packet_number++;
	packet_len = 0xFFFE;
	FTS_INFO("ecc calc num:%d, remainder:%d", packet_number, remainder);

	cmd[0] = 0x65;
	for (i = 0; i < packet_number; i++) {
		addr = saddr + 0xFFFE * i;
		cmd[1] = (u8)((addr >> 16) & 0xFF);
		cmd[2] = (u8)((addr >> 8) & 0xFF);
		cmd[3] = (u8)(addr & 0xFF);

		if ((i == (packet_number - 1)) && remainder)
			packet_len = remainder;

		cmd[4] = (u8)((packet_len >> 8) & 0xFF);
		cmd[5] = (u8)(packet_len & 0xFF);

		FTS_DEBUG("ecc calc startaddr:0x%04x, len:%d", addr, packet_len);
		ret = fts_i2c_write(client, cmd, 6);
		if (ret < 0) {
			FTS_ERROR("ecc calc cmd write fail");
			return ret;
		}

		msleep(packet_len / 256);

		fts_fwupg_check_flash_status(client, 0xF055, 10, 50);
	}

	cmd[0] = 0x66;
	ret = fts_i2c_read(client, cmd, 1, val, 1);
	if (ret < 0) {
		FTS_ERROR("ecc read cmd write fail");
		return ret;
	}

	return (int)val[0];
}

/*
 * The status the TP returns when the packet has gone through: the index of
 * the packet plus 0x1000.
 *   "1ad50ae8 udiv"@0xffffff8008a7df48   w8 = addr / packet_len
 *   "11400508 add"@0xffffff8008a7df4c    w8 += 0x1 << 12 = 0x1000
 *   "12003d17 and"@0xffffff8008a7df54    and the result is truncated to 16 bits
 */
#define FTS_FLASH_STATUS_OK		0x1000

/*
 * fts_flash_write_buf() was reconstructed from the factory kernel disassembly (0xffffff8008a7dddc, 584 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_flash_write_buf(struct i2c_client *client, u32 saddr, u8 *buf,
			u32 len, u32 retry)
{
	int ret = 0;
	u32 i = 0;
	u32 j = 0;
	u32 packet_number = 0;
	u32 packet_len = 0;
	u32 addr = 0;
	u32 offset = 0;
	u32 remainder = 0;
	u8 packet_buf[FTS_FLASH_PACKET_LENGTH + FTS_CMD_WRITE_LEN] = { 0 };
	u8 ecc = 0;
	u8 cmd = 0;
	u8 value[2] = { 0 };
	u16 read_status = 0;
	u16 wr_ok = 0;

	FTS_INFO("**********write data to flash**********");
	if ((NULL == buf) || (0 == len)) {
		FTS_ERROR("buf is NULL or len is 0");
		return -EINVAL;
	}

	FTS_INFO("data buf start addr=0x%x, len=0x%x", saddr, len);
	packet_number = len / FTS_FLASH_PACKET_LENGTH;
	remainder = len % FTS_FLASH_PACKET_LENGTH;
	if (remainder > 0)
		packet_number++;
	packet_len = FTS_FLASH_PACKET_LENGTH;
	FTS_INFO("write data, num:%d remainder:%d", packet_number, remainder);

	packet_buf[0] = FTS_CMD_WRITE;
	for (i = 0; i < packet_number; i++) {
		offset = i * FTS_FLASH_PACKET_LENGTH;
		addr = saddr + offset;
		packet_buf[1] = (u8)((addr >> 16) & 0xFF);
		packet_buf[2] = (u8)((addr >> 8) & 0xFF);
		packet_buf[3] = (u8)(addr & 0xFF);

		if ((i == (packet_number - 1)) && remainder)
			packet_len = remainder;

		packet_buf[4] = (u8)((packet_len >> 8) & 0xFF);
		packet_buf[5] = (u8)(packet_len & 0xFF);

		for (j = 0; j < packet_len; j++) {
			packet_buf[FTS_CMD_WRITE_LEN + j] = buf[offset + j];
			ecc ^= packet_buf[FTS_CMD_WRITE_LEN + j];
		}

		ret = fts_i2c_write(client, packet_buf,
				    packet_len + FTS_CMD_WRITE_LEN);
		if (ret < 0) {
			FTS_ERROR("app write fail");
			return ret;
		}

		mdelay(retry);

		wr_ok = addr / packet_len + FTS_FLASH_STATUS_OK;
		for (j = 0; j < FTS_RETRIES_WRITE; j++) {
			cmd = FTS_CMD_FLASH_STATUS;
			ret = fts_i2c_read(client, &cmd, 1, value, 2);
			read_status = (((u16)value[0]) << 8) + value[1];
			if (wr_ok == read_status)
				break;

			mdelay(1);
		}
	}

	return (int)ecc;
}

/*
 * fts_flash_read_buf() was reconstructed from the factory kernel disassembly (0xffffff8008a7e024, 352 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_flash_read_buf(struct i2c_client *client, u32 saddr, u8 *buf, u32 len)
{
	int ret = 0;
	u32 i = 0;
	u32 packet_number = 0;
	u32 packet_len = 0;
	u32 addr = 0;
	u32 offset = 0;
	u32 remainder = 0;
	u8 packet_buf[FTS_CMD_READ_LEN];

	if ((NULL == buf) || (0 == len)) {
		FTS_ERROR("buf is NULL or len is 0");
		return -EINVAL;
	}

	packet_number = len / FTS_FLASH_PACKET_LENGTH;
	remainder = len % FTS_FLASH_PACKET_LENGTH;
	if (remainder > 0)
		packet_number++;
	packet_len = FTS_FLASH_PACKET_LENGTH;
	FTS_INFO("read packet_number:%d, remainder:%d", packet_number, remainder);

	packet_buf[0] = FTS_CMD_READ;
	for (i = 0; i < packet_number; i++) {
		offset = i * FTS_FLASH_PACKET_LENGTH;
		addr = saddr + offset;
		packet_buf[1] = (u8)((addr >> 16) & 0xFF);
		packet_buf[2] = (u8)((addr >> 8) & 0xFF);
		packet_buf[3] = (u8)(addr & 0xFF);

		if ((i == (packet_number - 1)) && remainder)
			packet_len = remainder;

		ret = fts_i2c_write(client, packet_buf, FTS_CMD_READ_LEN);
		if (ret < 0) {
			FTS_ERROR("pram/bootloader write 03 command fail");
			return ret;
		}

		msleep(FTS_CMD_READ_DELAY);

		ret = fts_i2c_read(client, NULL, 0, buf + offset, packet_len);
		if (ret < 0) {
			FTS_ERROR("pram/bootloader read 03 command fail");
			return ret;
		}
	}

	return 0;
}

/*
 * fts_flash_read() was reconstructed from the factory kernel disassembly (0xffffff8008a7e184, 280 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_flash_read(struct i2c_client *client, u32 addr, u8 *buf, u32 len)
{
	int ret = 0;

	FTS_INFO("***********read flash***********");
	if ((NULL == buf) || (0 == len)) {
		FTS_ERROR("buf is NULL or len is 0");
		return -EINVAL;
	}

	ret = fts_fwupg_enter_into_boot(client);
	if (ret < 0) {
		FTS_ERROR("enter into pramboot/bootloader fail");
		goto read_flash_err;
	}

	ret = fts_flash_read_buf(client, addr, buf, len);
	if (ret < 0) {
		FTS_ERROR("read flash fail");
		goto read_flash_err;
	}

read_flash_err:
	ret = fts_fwupg_reset_in_boot(client);
	if (ret < 0)
		FTS_ERROR("reset to normal boot fail");

	return ret;
}

/*
 * fts_read_file() was reconstructed from the factory kernel disassembly (0xffffff8008a7e29c, 456 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_read_file(char *file_name, u8 **file_buf)
{
	int ret = 0;
	char file_path[128] = { 0 };
	struct file *filp = NULL;
	struct inode *inode;
	mm_segment_t old_fs;
	loff_t pos;
	loff_t file_len = 0;

	if ((NULL == file_name) || (NULL == file_buf)) {
		FTS_ERROR("filename/filebuf is NULL");
		return -EINVAL;
	}

	snprintf(file_path, 128, "%s%s", "/sdcard/", file_name);
	filp = filp_open(file_path, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		FTS_ERROR("open %s file fail", file_path);
		return -ENOENT;
	}

	inode = filp->f_inode;
	file_len = inode->i_size;
	*file_buf = vmalloc(file_len);
	if (NULL == *file_buf) {
		FTS_ERROR("file buf malloc fail");
		filp_close(filp, NULL);
		return -ENOMEM;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	ret = vfs_read(filp, *file_buf, file_len, &pos);
	if (ret < 0)
		FTS_ERROR("read file fail");
	FTS_INFO("file len:%d read len:%d pos:%d", (u32)file_len, ret, (u32)pos);
	filp_close(filp, NULL);
	set_fs(old_fs);

	return ret;
}

/*
 * fts_upgrade_bin() was reconstructed from the factory kernel disassembly (0xffffff8008a7e464, 412 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_upgrade_bin(struct i2c_client *client, char *fw_name, bool force)
{
	int ret = 0;
	u8 *fw_file_buf = NULL;
	struct fts_upgrade *upg = fts_g_a100ac8;

	FTS_INFO("start upgrade with fw bin");
	if ((NULL == upg) || (NULL == upg->c32)) {
		FTS_ERROR("upgrade/func is null");
		return -EINVAL;
	}

	ret = fts_read_file(fw_name, &fw_file_buf);
	if ((ret < 0x120) || (ret > 0x20000)) {
		FTS_ERROR("read fw bin file(sdcard) fail, len:%d", ret);
		goto err_bin;
	}

	FTS_INFO("fw bin file len:%d", ret);
	if (force) {
		if (upg->c32->c128) {
			ret = upg->c32->c128(client, fw_file_buf, ret);
		} else {
			FTS_INFO("force_upgrade function is null, no upgrade");
			goto err_bin;
		}
	} else {
		if (upg->c32->c96) {
			ret = upg->c32->c96(client, fw_file_buf, ret);
		} else {
			FTS_INFO("upgrade function is null, no upgrade");
		}
	}

	if (ret < 0) {
		FTS_ERROR("upgrade fw bin failed");
		fts_fwupg_reset_in_boot(client);
		goto err_bin;
	}

	FTS_INFO("upgrade fw bin success");

err_bin:
	if (fw_file_buf) {
		vfree(fw_file_buf);
		fw_file_buf = NULL;
	}

	return ret;
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7e600, 1216 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * fts_fwupg_get_ver_in_host() was reconstructed from the factory kernel disassembly (0xffffff8008a7e674).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_fwupg_get_ver_in_host(u8 *ver)
{
	struct fts_upgrade *upg = fts_g_a100ac8;

	if ((NULL == upg) || (NULL == upg->c32) || (NULL == upg->c0)) {
		FTS_ERROR("fts_data/upgrade/func/fw/ver is NULL");
		return -EINVAL;
	}

	if (upg->c8 < upg->c32->c36) {
		FTS_ERROR("fw len(0x%0x) < fw ver offset(0x%x)", upg->c8,
			  upg->c32->c36);
		return -EINVAL;
	}

	FTS_INFO("fw version offset:0x%x", upg->c32->c36);
	*ver = upg->c0[upg->c32->c36];

	return 0;
}

/*
 * fts_fwupg_get_ver_in_tp() was reconstructed from the factory kernel disassembly (0xffffff800924e02f).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_fwupg_get_ver_in_tp(struct i2c_client *client, u8 *ver)
{
	int ret = 0;

	ret = fts_i2c_read_reg(client, FTS_REG_FW_VER, ver);
	if (ret < 0) {
		FTS_ERROR("read fw ver from tp fail");
		return ret;
	}

	return 0;
}

/*
 * fts_fwupg_need_upgrade() was reconstructed from the factory kernel disassembly (0xffffff8008a7e658).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static bool fts_fwupg_need_upgrade(struct i2c_client *client)
{
	int ret = 0;
	bool fwvalid = false;
	u8 fw_ver_in_host = 0;
	u8 fw_ver_in_tp = 0;

	fwvalid = fts_fwupg_check_fw_valid(client);
	if (fwvalid) {
		ret = fts_fwupg_get_ver_in_host(&fw_ver_in_host);
		if (ret < 0) {
			FTS_ERROR("get fw ver in host fail");
			return false;
		}

		ret = fts_fwupg_get_ver_in_tp(client, &fw_ver_in_tp);
		if (ret < 0) {
			FTS_ERROR("get fw ver in tp fail");
			return false;
		}

		FTS_INFO("fw version in tp:%x, host:%x", fw_ver_in_tp,
			 fw_ver_in_host);
		if (fw_ver_in_tp < fw_ver_in_host)
			return true;
	} else {
		FTS_INFO("fw invalid, need upgrade fw");
		return true;
	}

	return false;
}

/*
 * fts_param_get_ver_in_host() was reconstructed from the factory kernel disassembly (0xffffff800924f309).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_param_get_ver_in_host(u8 *ver)
{
	struct fts_upgrade *upg = fts_g_a100ac8;

	if ((NULL == upg) || (NULL == upg->c32) || (NULL == upg->c0)) {
		FTS_ERROR("fts_data/upgrade/func/fw/ver is NULL");
		return -EINVAL;
	}

	if (upg->c8 < upg->c32->c56) {
		FTS_ERROR("fw len(%x) < paramcfg ver offset(%x)", upg->c8,
			  upg->c32->c56);
		return -EINVAL;
	}

	FTS_INFO("fw paramcfg version offset:%x", upg->c32->c56);
	*ver = upg->c0[upg->c32->c56];
	if ((0xFF == *ver) || (0x00 == *ver)) {
		FTS_INFO("param version in host invalid");
		return -EINVAL;
	}

	return 0;
}

/*
 * fts_param_get_ver_in_tp() was reconstructed from the factory kernel disassembly (0xffffff800924e057).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int fts_param_get_ver_in_tp(struct i2c_client *client, u8 *ver)
{
	int ret = 0;

	ret = fts_i2c_read_reg(client, FTS_REG_PARAM_VER, ver);
	if (ret < 0) {
		FTS_ERROR("read fw param ver from tp fail");
		return ret;
	}

	if ((0xFF == *ver) || (0x00 == *ver)) {
		FTS_INFO("param version in tp invalid");
		return -EIO;
	}

	return 0;
}

/*
 * fts_param_need_upgrade() was reconstructed from the factory kernel disassembly (0xffffff8008a7e794).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static bool fts_param_need_upgrade(struct i2c_client *client)
{
	int ret = 0;
	bool fwvalid = false;
	u8 ide_para_status = 0;
	u8 param_ver_in_tp = 0;
	u8 param_ver_in_host = 0;

	fwvalid = fts_fwupg_check_fw_valid(client);
	if (!fwvalid) {
		FTS_INFO("fw is invalid, no upgrade paramcfg");
		return false;
	}

	ret = fts_param_get_ver_in_host(&param_ver_in_host);
	if (ret < 0) {
		FTS_ERROR("param version in host invalid");
		return false;
	}

	ret = fts_i2c_read_reg(client, FTS_REG_PARAM_STATUS, &ide_para_status);
	if (ret < 0) {
		FTS_ERROR("read IDE PARAM STATUS in tp fail");
		return false;
	}

	if (0 == (ide_para_status & 0x80)) {
		FTS_INFO("no IDE VER in tp");
		return false;
	}

	if (0 != (ide_para_status & 0x7F)) {
		FTS_INFO("IDE VER, param invalid, need upgrade param");
		return true;
	}

	ret = fts_param_get_ver_in_tp(client, &param_ver_in_tp);
	if (ret < 0) {
		FTS_ERROR("get IDE param ver in tp fail");
		return false;
	}

	FTS_INFO("fw paramcfg version in tp:%x, host:%x", param_ver_in_tp,
		 param_ver_in_host);
	if (param_ver_in_tp < param_ver_in_host)
		return true;

	return false;
}

/*
 * fts_fwupg_upgrade() was reconstructed from the factory kernel disassembly (0xffffff8008a7e600, 1216 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_fwupg_upgrade(struct i2c_client *client, struct fts_upgrade *upg)
{
	int ret = 0;
	int upgrade_count = 0;
	bool upgrade_flag = false;
	u8 ver = 0;

	FTS_INFO("fw auto upgrade function");
	if ((NULL == upg) || (NULL == upg->c32)) {
		FTS_ERROR("upg/upg->func is null");
		return -EINVAL;
	}

	upgrade_flag = fts_fwupg_need_upgrade(client);
	FTS_INFO("fw upgrade flag:%d", upgrade_flag);
	do {
		upgrade_count++;
		if (upgrade_flag) {
			FTS_INFO("upgrade fw app(times:%d)", upgrade_count);
			if (upg->c32->c96) {
				ret = upg->c32->c96(client, upg->c0, upg->c8);
				if (ret < 0) {
					fts_fwupg_reset_in_boot(client);
				} else {
					fts_fwupg_get_ver_in_tp(client, &ver);
					FTS_INFO("success upgrade to fw version %02x",
						 ver);
					break;
				}
			} else {
				FTS_ERROR("upgrade func/upgrade is null, return immediately");
				return -ENODATA;
			}
		} else {
			FTS_INFO("fw don't need upgrade");
			if (NULL == upg->c32->c120)
				break;

			if (fts_param_need_upgrade(client)) {
				FTS_INFO("upgrade param area(times:%d)",
					 upgrade_count);
				ret = upg->c32->c120(client, upg->c0, upg->c8);
				if (ret < 0) {
					fts_fwupg_reset_in_boot(client);
				} else {
					fts_param_get_ver_in_tp(client, &ver);
					FTS_INFO("success upgrade to fw param version %02x",
						 ver);
					break;
				}
			} else {
				FTS_INFO("param don't need upgrade");
				break;
			}
		}
	} while (upgrade_count < 2);

	return ret;
}


/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_flash.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
