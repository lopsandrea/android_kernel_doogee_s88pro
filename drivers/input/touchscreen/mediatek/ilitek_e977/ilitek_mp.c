// SPDX-License-Identifier: GPL-2.0
/*
 * Ilitek TDDI touch panel, Doogee S88 Pro (MediaTek MT6771) -- group F, the
 * MP self-test.
 *
 * Twenty-four emitted functions, [0xffffff8008a5da3c, 0xffffff8008a6714c),
 * 38672 bytes. Includes the .ini parser, the CDC test paths and
 * ilitek_tddi_mp_test_main, which carries thirteen inlined functions.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read: Ilitek TDDI drivers do circulate publicly and the ALPS tree holds
 * touch drivers of the same family (GT1151/, GT5688/), none of which were
 * opened. Every derived constant was checked against the factory binary.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/firmware.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/timekeeping.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#include "ilitek.h"

/*
 * ILI_CONT() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define ILI_CONT(fmt, ...)	printk(KERN_CONT fmt, ##__VA_ARGS__)

/*
 * ILI_KFREE() was reconstructed from the factory kernel disassembly (0xffffff8008a654d8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define ILI_KFREE(p)					\
	do {						\
		if (p) {				\
			kfree(p);			\
			(p) = NULL;			\
		}					\
	} while (0)

/*
 * ===========================================================================
 * THE LOCAL DECLARATIONS (see "THE HEADER DELTA" in the file header)
 * ===========================================================================
 */

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5db54).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_ini_item {
	char c0[100];		/* section name */
	char c100[100];		/* key name */
	char c200[2000];	/* the value text */
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a603bc).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	/*
	 * This section was reconstructed from the factory kernel disassembly.
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	int c2200;		/* section name length. ROLE measured:
				 * "b9489908 ldr"@0xffffff8008a605a0 reads it back and
				 * "7101951f cmp"@0xffffff8008a605a4 compares it with
				 * 0x65 = 101, the name limit (100).
				 * THE SIGN IS A CHOICE, NOT A MEASUREMENT: that
				 * `ldr` is 32-bit and no other group F instruction
				 * reads +2200. `int` for symmetry with the other two,
				 * which are proven instead.
				 */
	int c2204;		/* key name length, signed:
				 * "b9889d42 ldrsw"@0xffffff8008a60558 reads +2204
				 * and SIGN-extends it; the role is proven by what
				 * follows -- "7101945f cmp"@0xffffff8008a6055c
				 * (0x65 = 101) and the copy at +100
				 * ("91019100 add"@0xffffff8008a60568)
				 */
	int c2208;		/* value length, signed:
				 * "b988a142 ldrsw"@0xffffff8008a604c0 reads +2208
				 * and SIGN-extends it; the role is proven by
				 * "711f445f cmp"@0xffffff8008a604c4 (0x7d1 = 2001)
				 * and the copy at +200
				 * ("91032100 add"@0xffffff8008a604d0)
				 */
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800924477e, 24 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_core_mp {
	/*
	 * THE FIRST 232 BYTES, opened up by batch F5. Batch F1 declared them
	 * padding (`u8 c0[232]`) because none of its functions touched them:
	 * they are written by `ilitek_tddi_mp_init_item`, which is inlined into
	 * `ilitek_tddi_mp_test_main` and which batch F5 read.
	 * THE FIRST ARGUMENT OF EVERY `printk` IN THAT BLOCK IS THE FORMAT AND
	 * THE SECOND THE `__func__` (class B1): verified by reading the two
	 * strings, "ilitek_tddi_mp_init_item"@0xffffff800924384f is the name.
	 */
	int c0;		/* +0: "b90002ea str"@0xffffff8008a5e6ec, four byte;
			 * printed by "\x016ILITEK: (%s, %d): CHIP = 0x%x\n"
			 * @0xffffff80092438b5 */
	u16 c4;		/* +4: "79000aea strh"@0xffffff8008a5e6f4, DUE byte */
	u8 c6;		/* +6: "39001aea strb"@0xffffff8008a5e6fc */
	u8 c7;		/* +7: "39001eea strb"@0xffffff8008a5e704 */
	int c8;		/* +8: "b9000aea str"@0xffffff8008a5e70c; printed by
			 * "\x016ILITEK: (%s, %d): Firmware version = %x\n"
			 * @0xffffff80092438d6 */
	int c12;	/* +12: first word of "2901a6eb stp"@0xffffff8008a5e718;
			 * "\x016ILITEK: (%s, %d): Protocol version = %x\n"
			 * @0xffffff8009243901. It is also the field the test's
			 * version check compares with `str2hex` of the .ini
			 * value "protocol", shifted by 8 bits:
			 * "b94b0d08 ldr"@0xffffff8008a5ec6c +
			 * "6b48201f cmp"@0xffffff8008a5ec70 (`lsr #8`)
			 */
	int c16;	/* +16: second word of the same `stp`;
			 * "\x016ILITEK: (%s, %d): Core version = %x\n"
			 * @0xffffff800924392c
			 */
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0fcb14).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	char c20[128];
	char c148[64];
	int c212;	/* +212: "b900d6e9 str"@0xffffff8008a5e730, four byte */
	u8 c216;	/* +216: "390362ff strb"@0xffffff8008a5e744 clears it; it is
			 * the retry guard -- "396f6108 ldrb"@0xffffff8008a60018 +
			 * "34000128 cbz"@0xffffff8008a6001c jointly skip the
			 * "MP failed, doing retry" message and the call to
			 * `mp_do_retry`. NONE of the 24 group F functions WRITES it
			 * with a non-zero value: the retry, in the factory build,
			 * never starts
			 */
	u8 c217[10];	/* +217..226: declared padding */
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0fcbe3).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	u8 c227;
	int c228;	/* +228: "b900e6e9 str"@0xffffff8008a5e724;
			 * "\x016ILITEK: (%s, %d): Read CDC Length = %d\n"
			 * @0xffffff8009243953 */
	int c232;	/* 0xffffff800a0fcbe8 -- signed `int`: it is the divisor
			 * of "1ac80ee9 sdiv"@0xffffff8008a5dab0 (SIGNED division)
			 * and the number of columns per printed row
			 */
	int c236;	/* 0xffffff800a0fcbec -- signed `int`, the limit of the
			 * ROW index: "b94bed6b ldr"@0xffffff8008a5e078 reads it
			 * and "6b0b01ff cmp"@0xffffff8008a5e0b4 with
			 * "5400018a b.ge"@0xffffff8008a5e0b8 uses it as the
			 * upper bound with a SIGNED comparison. In the first
			 * draft of F1 this field did not exist: the four bytes
			 * sat inside a `u8 c236[20]` padding, because no F1
			 * function touches them
			 */
	int c240;	/* +240: cleared by "a90f7eff stp"@0xffffff8008a5e73c,
			 * which covers SIXTEEN bytes, 240..255
			 */
	int c244;	/* +244: second word of the same `stp` */
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff800924477e).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	int key_len;
	int c252;	/* +252: second half of the `stp` that zeroes 248..255 */
	int c256;	/* 0xffffff800a0fcc00 -- signed `int`: read both by
			 * "b94c02c8 ldr"@0xffffff8008a5da7c (the loop guard) and
			 * "b98c02c8 ldrsw"@0xffffff8008a5dacc (SIGN extension to
			 * the index). It is the number of nodes
			 */
	int c260;	/* +260: no written function touches it */
	/*
	 * +264: THE OVERALL TEST RESULT. Three measured writes:
	 * "b9010af4 str"@0xffffff8008a5e750 sets it to -1 at initialisation
	 * (w20 = -1 from "12800014 mov"@0xffffff8008a5e6d8),
	 * "b900011f str"@0xffffff8008a61c70 clears it when every executed item
	 * has a result >= 0, and
	 * "b90c0909 str"@0xffffff8008a61cac puts it back to -1 otherwise.
	 * `mp_test_free` returns it to -1: "b90c0913 str"@0xffffff8008a5f044.
	 */
	int c264;
	u8 c268[20];	/* +268..287: declared padding */
	/*
	 * FOUR CONSECUTIVE `int`s, 0xffffff800a0fcc20..0xffffff800a0fcc2f,
	 * added by batch F4: before that they sat inside the `c260[44]`
	 * padding, because no F1/F2/F3 function touches them.
	 * `mp_comp_result_before_retry` reads them in PAIRS, with two `ldp`s of
	 * two words each, and the pairs are adjacent: that is what proves they
	 * are four 4-byte fields and not two 8-byte ones.
	 */
	int c288;	/* 0xffffff800a0fcc20: first word of the pair
			 * "2944354c ldp"@0xffffff8008a66084 (x10 = page+3072,
			 * offset 32)
			 */
	int c292;	/* 0xffffff800a0fcc24: second word of the same `ldp` */
	int c296;	/* 0xffffff800a0fcc28: first word of the pair
			 * "2945354c ldp"@0xffffff8008a663a0 (same x10,
			 * offset 40)
			 */
	int c300;	/* 0xffffff800a0fcc2c: second word of the same `ldp`.
			 * THE SIGN IS NOT MEASURED: the four values are only
			 * copied into an array of `int`, never compared
			 */
	/*
	 * SIX CONSECUTIVE POINTERS, 0xffffff800a0fcc30..0xffffff800a0fcc58.
	 * That they are pointers is measured: they are read with `ldr x`
	 * (eight bytes) and the value read is immediately used as the base of
	 * an `ldr w`/`str w` scaled by 4. That there are SIX of them, and
	 * consecutive, is shown by `compare_MaxMin_result`, which touches them
	 * all in twenty-four instructions.
	 */
	int *tx_delta_buf;	/* "f9461964 ldr"@0xffffff8008a65924 (pagina+3120) */
	int *rx_delta_buf;	/* "f9461e25 ldr"@0xffffff8008a65974 (pagina+3128) */
	int *tx_max_buf;	/* "f9400985 ldr"@0xffffff8008a65944 ([x12,#16] con
			 * x12 = 0xffffff800a0fcc30) */
	int *tx_min_buf;	/* "f94625a5 ldr"@0xffffff8008a65964 (pagina+3144) */
	int *rx_max_buf;	/* "f9400e45 ldr"@0xffffff8008a65984 ([x18,#24] con
			 * x18 = 0xffffff800a0fcc38) */
	int *rx_min_buf;	/* "f9462c05 ldr"@0xffffff8008a659a4 (pagina+3160) */
	/*
	 * +352 and +356: TWO `int`s, not a `u64`. That there are two is proven by
	 * the FOUR-byte read of the second -- "b94166e8 ldr"@0xffffff8008a5e840
	 * -- while initialisation writes them together with
	 * "f900b2ea str"@0xffffff8008a5e740 (eight bytes, x10 = 240 from
	 * "321c0fea orr"@0xffffff8008a5e720): 352 = 240, 356 = 0. They are two
	 * adjacent stores merged by the compiler.
	 */
	int c352;
	int c356;	/* +356: chooses between "Polling"@0xffffff800923427d and
			 * "Interrupt"@0xffffff8009234cfb --
			 * "7100011f cmp"@0xffffff8008a5e854 +
			 * "9a890143 csel"@0xffffff8008a5e85c, `eq` chooses
			 * "Interrupt" */
	u8 c360;	/* +360: no written function touches it */
	u8 c361;	/* +361: "3905a6ff strb"@0xffffff8008a5e758 clears it and
			 * "3931a509 strb"@0xffffff8008a5f748 sets it to 1
			 * when the .ini key "goldenmode" does not match
			 * "spec option". It governs the message
			 * "WARNING! Golden and SPEC in ini file aren't
			 * matched!!" and the choice of the CSV file name
			 */
	u8 c362[6];	/* +362..367: alignment padding. THE TOTAL WIDTH IS
			 * MEASURED: "52802e02 mov"@0xffffff8008a5e6a8 passes
			 * 368 to `__memset` on the base of the structure
			 * ("aa1703e0 mov"@0xffffff8008a5e6ac, x0 = x23 =
			 * 0xffffff800a0fcb00)
			 */
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff80099876d0, 152 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_mp_item {
	char *c0;	/* +0x00, eight bytes: "f94002a4 ldr"@0xffffff8008a66ca8
			 * reads it and "940fe858 bl"@0xffffff8008a5e1a0 passes it to
			 * `strlen`; in `mp_do_retry` it ends up in the `%s` argument
			 * of the message at 0xffffff800924539e (cited in full
			 * above `mp_do_retry`). It is a
			 * `char *`, and it is the item name
			 */
	char *c8;	/* +0x08, eight bytes: the item RESULT buffer.
			 * `ilitek_tddi_mp_init_item` puts the result of
			 * `kmalloc(16, GFP_KERNEL)` there
			 * ("97dfc0a0 bl"@0xffffff8008a5ea2c towards
			 * <kmem_cache_alloc_trace>, "321c03e2 orr"@0xffffff8008a5ea24
			 * = 16) with "f8178360 stur"@0xffffff8008a5ea30, and
			 * immediately writes the five bytes "FAIL\0" into it --
			 * "b9000019 str"@0xffffff8008a5ea40 (w25 = 0x4c494146 =
			 * 'F','A','I','L' from "528828d9 mov"@0xffffff8008a5e88c
			 * plus "72a98939 movk"@0xffffff8008a5e8ac) and
			 * "3900101f strb"@0xffffff8008a5ea44 (the NUL).
			 * `mp_test_free` frees it: "f85b0280 ldur"@0xffffff8008a5f100
			 * + "97dfc5aa bl"@0xffffff8008a5f108 towards <kfree>.
			 * THE `char *` TYPE IS MEASURED from the five bytes of
			 * text; the NAME is not
			 */
	int c16;	/* +0x10, `int`: "b94001c4 ldr"@0xffffff8008a658cc,
			 * four bytes, compared with 4 and with 5
			 * ("7100109f cmp"@0xffffff8008a658d0 and
			 * "7100149f cmp"@0xffffff8008a658d8)
			 */
	u8 c20;		/* +0x14, one byte: it is the FIRST byte of the command
			 * `pin_test` writes to the chip
			 * ("39405108 ldrb"@0xffffff8008a64fb8, then
			 * "390003e8 strb"@0xffffff8008a64fc8 into `cmd[0]`).
			 * NAME NOT IN THE BINARY: `c<offset>` (rule 5)
			 */
	u8 c21;		/* +0x15, one byte compared with 1:
			 * "39405508 ldrb"@0xffffff8008a6539c +
			 * "7100051f cmp"@0xffffff8008a653a0. It governs the two
			 * `bench_mark_*` in `create_mp_test_frame_buffer`.
			 * NAME NOT IN THE BINARY
			 */
	u8 c22;		/* +0x16 = 22, ONE byte. It is NOT padding: the .ini key
			 * "type option"@0xffffff8009244f28 ends up in there --
			 * "39005900 strb"@0xffffff8008a5f510, a single byte.
			 * `mp_show_result` does not read it back; the one that does is
			 * `ilitek_tddi_mp_init_item`, which clears it together with
			 * +0x15 using "7818537f sturh"@0xffffff8008a5e954 (TWO
			 * bytes starting at 0x15). NAME NOT IN THE BINARY
			 */
	u8 c23;		/* +0x17 = 23, ONE byte. It is NOT padding: it is the
			 * "this item has to run" flag, written by
			 * `mp_sort_item` with "381ff2a8 sturb"@0xffffff8008a5eeb8
			 * as `(katoi(the "enable" value) != 0)`
			 * ("1a9f07e8 cset"@0xffffff8008a5eeb4, condition `ne`),
			 * read back by "39405d04 ldrb"@0xffffff8008a5ff90 for the
			 * `run` field of the message
			 * "\x016ILITEK: (%s, %d): %s: run = %d, max = %d, min = %d, frame_count = %d\n"
			 * @0xffffff8009245045 and by
			 * "385f314b ldurb"@0xffffff8008a61c3c in the final count.
			 * `mp_test_free` clears it
			 * ("381bf29f sturb"@0xffffff8008a5f04c).
			 * THE NAME `run` COMES FROM THE FORMAT, but the format does not
			 * write it as a FIELD name: it stays `c23` (rule 5)
			 */
	/*
	 * This section was reconstructed from the factory kernel disassembly.
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	u8 c24;
	u8 c25[3];	/* +0x19..0x1b: padding, always zero across the 50 entries */
	int c28;	/* +0x1c = 28, `int`: "b940018e ldr"@0xffffff8008a6655c
			 * reads it as FOUR bytes (x12 = item + 0x1c,
			 * "9100710c add"@0xffffff8008a66550) and the value read
			 * is copied into an array of `int`. The SIGN is not
			 * measured: no comparison proves it. It is zero in all
			 * 50 `.data` entries, so it is written at run time.
			 * NAME NOT IN THE BINARY
			 */
	int c32;	/* +0x20 = 32, FOUR bytes. It is NOT padding:
			 * `ilitek_tddi_mp_init_item` sets it to -1 together with
			 * `c28` with "f818c377 stur"@0xffffff8008a5e95c (eight
			 * bytes starting at 0x1c, x23 = 0xffffffff00000000 from
			 * "b2607ff7 mov"@0xffffff8008a5e8a4: the low half --
			 * `c28` -- is 0 and the high one -- this field -- is -1),
			 * and `mp_test_free` puts it back to -1 together with `c36`
			 * with "f81c8293 stur"@0xffffff8008a5f050.
			 * NAME NOT IN THE BINARY
			 */
	int c36;	/* +0x24, `int`: `pin_test` writes 0 or -1 into it
			 * ("b9002528 str"@0xffffff8008a65164, four bytes,
			 * with "12800008 mov"@0xffffff8008a65144 = -1 on one
			 * branch and "2a1f03e8 mov"@0xffffff8008a650e0 = 0
			 * on the other). NAME NOT IN THE BINARY
			 */
	int c40;	/* +0x28 = 40, `int`: "b94001b0 ldr"@0xffffff8008a66560
			 * (x13 = item + 0x28, "9100a10d add"@0xffffff8008a66554).
			 * It is the companion of `c28`: the two are copied in the
			 * same loop, one into the upper threshold vector and the
			 * other into the lower one.
			 * NAME NOT IN THE BINARY
			 */
	int c44;	/* +0x2c = 44, FOUR bytes. It is NOT padding:
			 * "b819c374 stur"@0xffffff8008a5e968 sets it to -1
			 * at initialisation and
			 * "b81d4293 stur"@0xffffff8008a5f054 puts it back to -1 in
			 * `mp_test_free`. No written function READS it.
			 * NAME NOT IN THE BINARY
			 */
	int c48;	/* +0x30 = 48, `int`: "b8430f6a ldr"@0xffffff8008a66614
			 * (pre-indexed, x27 = item + 48) and
			 * "7100095f cmp"@0xffffff8008a66618 compares it with 2
			 * using "54002c4b b.lt"@0xffffff8008a6661c, that is SIGNED.
			 * It is the number of times the measurement is repeated:
			 * the factor `core_mp.c256` is multiplied by to size the
			 * working vector ("1b0a7d09 mul"@0xffffff8008a66630) and
			 * the limit of the inner sorting loop.
			 * NAME NOT IN THE BINARY
			 */
	int c52;	/* +0x34 = 52, `int`: "b940356b ldr"@0xffffff8008a66584,
			 * four bytes, tested only against zero
			 * ("3400008b cbz"@0xffffff8008a66588).
			 * NAME NOT IN THE BINARY
			 */
	int c56;	/* +0x38 = 56: first word of the pair
			 * "29472929 ldp"@0xffffff8008a667c8
			 */
	int c60;	/* +0x3c = 60: second word of the same `ldp`.
			 * THEY ARE SIGNED `int`s: the product by `c48` ends up
			 * in a division by 100 done with
			 * "9b2b7d4a smull"@0xffffff8008a667e4 +
			 * "9365fd4a asr"@0xffffff8008a667f0, that is clang's
			 * sequence for a SIGNED division
			 */
	/*
	 * +0x40..+0x53: FIVE `int`s, opened up by batch F5. Batch F3 declared
	 * them padding (`u8 c64[20]`). The four `tdf`s are read in PAIRS by two
	 * adjacent `ldp`s -- "29481103 ldp"@0xffffff8008a5ffc4 (+64,+68) and
	 * "29491905 ldp"@0xffffff8008a5ffc8 (+72,+76) -- and that is what proves
	 * they are four four-byte fields.
	 */
	int c64;	/* +0x40 = 64: "b9004100 str"@0xffffff8008a5fa74 */
	int c68;	/* +0x44 = 68: "b9004500 str"@0xffffff8008a5fc64 */
	int c72;	/* +0x48 = 72: "b9004900 str"@0xffffff8008a5fd5c */
	int c76;	/* +0x4c = 76: "b9004d00 str"@0xffffff8008a5fe70 */
	int c80;	/* +0x50 = 80: "b9005100 str"@0xffffff8008a5f72c, the
			 * value of the .ini key "goldenmode"
			 * @0xffffff8009244f72; it is immediately compared with
			 * `c21` ("6b08001f cmp"@0xffffff8008a5f738)
			 */
	/*
	 * THREE BYTES THE BINARY NAMES -- this is the exception to rule 5, the
	 * same as `core_mp.key_len`: three `printk`s in `pin_test` read these
	 * three bytes and print them with a format that spells out their name.
	 */
	/*
	 * CORRECTION FROM BATCH F3c-bis: the three citations below carried the
	 * TAIL of the message at the address of the WHOLE message. While
	 * `pin_test` was not yet written the tool marked them NON_ANCORATA
	 * (no literal with that text in the code) and it went unnoticed;
	 * once the function was written the literal exists,
	 * `verificacitazioni.py` compares the bytes and the three become
	 * DIVERGENT. They are now written in full, with the prefix
	 * "\x016ILITEK: (%s, %d): ", and it is exactly the same correction
	 * batch F3c had to make on the six "Failed to allocate <field> mem".
	 */
	u8 delay_time;		/* +0x54 = 84: "38454ee3 ldrb"@0xffffff8008a64f88
				 * (pre-index, x23 = element + 84) and the
				 * format
				 * "\x016ILITEK: (%s, %d): delay_time = 0x%x\n"@0xffffff8009244910 */
	u8 test_int_pin;	/* +0x55 = 85: "39415723 ldrb"@0xffffff8008a64f4c
				 * e the format
				 * "\x016ILITEK: (%s, %d): test_int_pin = 0x%x\n"@0xffffff80092448bc */
	u8 int_pulse_test;	/* +0x56 = 86: "38456f03 ldrb"@0xffffff8008a64f6c
				 * (pre-index, x24 = element + 86) and the
				 * format
				 * "\x016ILITEK: (%s, %d): int_pulse_test = 0x%x\n"@0xffffff80092448e5 */
	u8 c87;			/* +0x57: alignment padding up to the
				 * pointer at +88
				 */
	/*
	 * SIX POINTERS, +0x58..+0x80. THE NAMES COME FROM THE BINARY: each one
	 * is written by an allocation in `create_mp_test_frame_buffer` whose
	 * error path prints "Failed to allocate <name> mem". There is a SINGLE
	 * inference step -- the message names what was being allocated, and the
	 * `str` before it says where -- and for `max_buf` and `min_buf` there is
	 * independent confirmation too: they are the two vectors
	 * `compare_MaxMin_result` updates with the maximum and the minimum.
	 */
	int *result_buf;	/* +0x58 = 88: "f8458e68 ldr"@0xffffff8008a652e8,
				 * "f9000260 str"@0xffffff8008a65310;
				 * "\x013ILITEK: (%s, %d): Failed to allocate result_buf mem\n"
				 * @0xffffff8009243cec */
	int *buf;		/* +0x60 = 96: "f8460ec8 ldr"@0xffffff8008a652b4,
				 * "f90002c0 str"@0xffffff8008a652d4;
				 * "\x013ILITEK: (%s, %d): Failed to allocate buf mem\n"
				 * @0xffffff8009243cbc. It is the ONLY one allocated
				 * con `vmalloc` */
	int *max_buf;		/* +0x68 = 104: "f94001e5 ldr"@0xffffff8008a658e0
				 * (eight bytes), and the value read is immediately
				 * the base of an `ldr w` scaled by 4
				 * ("b86368a6 ldr"@0xffffff8008a658f0);
				 * "\x013ILITEK: (%s, %d): Failed to allocate max_buf mem\n"
				 * @0xffffff8009243d23
				 */
	int *min_buf;		/* +0x70 = 112: "f9400205 ldr"@0xffffff8008a6590c;
				 * "\x013ILITEK: (%s, %d): Failed to allocate min_buf mem\n"
				 * @0xffffff8009243d57 */
	int *bench_mark_max;	/* +0x78 = 120: "f8478e68 ldr"@0xffffff8008a653b0;
				 * "\x013ILITEK: (%s, %d): Failed to allocate bench_mark_max mem\n"
				 * @0xffffff8009243d8b */
	int *bench_mark_min;	/* +0x80 = 128: "f8480e68 ldr"@0xffffff8008a653c0;
				 * "\x013ILITEK: (%s, %d): Failed to allocate bench_mark_min mem\n"
				 * @0xffffff8009243dc6 */
	int *c136;	/* +0x88 = 136, EIGHT bytes. It is NOT padding: it is a
			 * pointer `mp_test_free` frees --
			 * "f9401a80 ldr"@0xffffff8008a5f0f0,
			 * "97dfc5ae bl"@0xffffff8008a5f0f8 towards <kfree>,
			 * "f9001a9f str"@0xffffff8008a5f0fc (set back to NULL).
			 * THE TYPE IS NOW MEASURED (batch F3c, it used to be `void *`):
			 * `open_test_sp` allocates it with
			 * "97dfb3de bl"@0xffffff8008a63410 towards <__kmalloc>
			 * after "d37ef500 lsl"@0xffffff8008a63408 (the count times
			 * FOUR) and "f8088f80 str"@0xffffff8008a63420 writes it
			 * at +136; then it walks it with stride 4 using
			 * "b86bd98c ldr"@0xffffff8008a63f58 and
			 * "b871d9b2 ldr"@0xffffff8008a640a4. It is an `int *`, and
			 * it is the NODE TYPE vector that
			 * `parser_ini_nodetype` fills
			 * ("97ffe97e bl"@0xffffff8008a634f8)
			 */
	int (*c144)(int);
			/*
			 * +0x90, FUNCTION POINTER:
			 * "f9404aa8 ldr"@0xffffff8008a66cd0 reads it and
			 * "d63f0100 blr"@0xffffff8008a66cd8 calls it with
			 * w0 = the index of the item
			 * ("2a1303e0 mov"@0xffffff8008a66cd4). The return type
			 * is NOT proven by this batch: `mp_do_retry` discards w0
			 * right after. `int` is a CHOICE
			 */
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff80099876d0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_mp_item mp_items[50] = {
	{ .c0 = "baseline data(bg)", .c16 = 0, .c20 = 2, .c24 = 1 },
	{ .c0 = "untouch signal data(bg-raw-4096) - mutual", .c16 = 0, .c20 = 3, .c24 = 1 },
	{ .c0 = "manual bk data(mutual)", .c16 = 0, .c20 = 16, .c24 = 1 },
	{ .c0 = "calibration data(dac) - self", .c16 = 1, .c20 = 12, .c24 = 1 },
	{ .c0 = "baselin data(bg,self_tx,self_r)", .c16 = 1, .c20 = 15, .c24 = 1 },
	{ .c0 = "untouch signal data(bg-raw-4096) - self", .c16 = 1, .c20 = 13, .c24 = 1 },
	{ .c0 = "raw data(no bk) - self", .c16 = 1, .c20 = 14, .c24 = 1 },
	{ .c0 = "raw data(have bk) - self", .c16 = 1, .c20 = 11, .c24 = 1 },
	{ .c0 = "manual bk dac data(self_tx,self_rx)", .c16 = 1, .c20 = 17, .c24 = 1 },
	{ .c0 = "calibration data(dac/icon)", .c16 = 2, .c20 = 20, .c24 = 1 },
	{ .c0 = "key baseline data", .c16 = 2, .c20 = 22, .c24 = 1 },
	{ .c0 = "key raw data", .c16 = 2, .c20 = 7, .c24 = 1 },
	{ .c0 = "key raw bk dac", .c16 = 2, .c20 = 21, .c24 = 1 },
	{ .c0 = "key raw open test", .c16 = 2, .c20 = 18, .c24 = 1 },
	{ .c0 = "key raw short test", .c16 = 2, .c20 = 19, .c24 = 1 },
	{ .c0 = "st calibration data(dac)", .c16 = 3, .c20 = 26, .c24 = 1 },
	{ .c0 = "st baseline data(bg)", .c16 = 3, .c20 = 28, .c24 = 1 },
	{ .c0 = "st raw data(no bk)", .c16 = 3, .c20 = 23, .c24 = 1 },
	{ .c0 = "st raw(have bk)", .c16 = 3, .c20 = 27, .c24 = 1 },
	{ .c0 = "st open data", .c16 = 3, .c20 = 24, .c24 = 1 },
	{ .c0 = "tx short test", .c16 = 0, .c20 = 25, .c24 = 1 },
	{ .c0 = "rx open", .c16 = 0, .c20 = 6, .c24 = 1 },
	{ .c0 = "untouch cm data", .c16 = 0, .c20 = 9, .c24 = 1 },
	{ .c0 = "untouch cs data", .c16 = 0, .c20 = 10, .c24 = 1 },
	{ .c0 = "tx/rx delta", .c16 = 4, .c20 = 30, .c24 = 1 },
	{ .c0 = "untouch peak to peak", .c16 = 5, .c20 = 3, .c24 = 1 },
	{ .c0 = "pixel raw (no bk)", .c16 = 6, .c20 = 5, .c24 = 1 },
	{ .c0 = "pixel raw (have bk)", .c16 = 6, .c20 = 8, .c24 = 1 },
	{ .c0 = "noise peak to peak(cut panel)", .c16 = 8, .c20 = 0, .c24 = 1 },
	{ .c0 = "open test(integration)", .c16 = 7, .c20 = 4, .c24 = 1 },
	{ .c0 = "open test(cap)", .c16 = 7, .c20 = 4, .c24 = 1 },
	{ .c0 = "pin test ( int and rst )", .c16 = 10, .c20 = 97, .c24 = 1 },
	{ .c0 = "noise peak to peak(with panel)", .c16 = 8, .c20 = 0, .c24 = 1 },
	{ .c0 = "noise peak to peak(ic only)", .c16 = 8, .c20 = 29, .c24 = 1 },
	{ .c0 = "open test(integration)_sp", .c16 = 7, .c20 = 0, .c24 = 1 },
	{ .c0 = "raw data(no bk)", .c16 = 0, .c20 = 5, .c24 = 1 },
	{ .c0 = "raw data(have bk)", .c16 = 0, .c20 = 8, .c24 = 1 },
	{ .c0 = "calibration data(dac)", .c16 = 0, .c20 = 1, .c24 = 1 },
	{ .c0 = "short test -ili9881", .c16 = 9, .c20 = 4, .c24 = 1 },
	{ .c0 = "short test", .c16 = 9, .c20 = 0, .c24 = 1 },
	{ .c0 = "doze raw data", .c16 = 0, .c20 = 0, .c24 = 1 },
	{ .c0 = "doze peak to peak", .c16 = 8, .c20 = 0, .c24 = 1 },
	{ .c0 = "open test_c", .c16 = 7, .c20 = 0, .c24 = 1 },
	{ .c0 = "touch deltac", .c16 = 0, .c20 = 0, .c24 = 1 },
	{ .c0 = "raw data(have bk) (lcm off)", .c16 = 0, .c20 = 0, .c24 = 0 },
	{ .c0 = "raw data(no bk) (lcm off)", .c16 = 0, .c20 = 0, .c24 = 0 },
	{ .c0 = "noise peak to peak(with panel) (lcm off)", .c16 = 8, .c20 = 0, .c24 = 0 },
	{ .c0 = "noise peak to peak(ic only) (lcm off)", .c16 = 8, .c20 = 0, .c24 = 0 },
	{ .c0 = "raw data_td (lcm off)", .c16 = 0, .c20 = 0, .c24 = 0 },
	{ .c0 = "peak to peak_td (lcm off)", .c16 = 8, .c20 = 0, .c24 = 0 },
};

/*
 * Defined elsewhere (divergence F1-D2): in the binary they live in `.bss`
 * and no function outside group F touches them, so in the factory source
 * they are almost certainly `static` to the MP test file. Here they CANNOT
 * be: never written in this batch, the compiler would prove them constant
 * zero.
 */
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0fcb00).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_core_mp core_mp;

/*
 * 0xffffff800a0ff400: signed `int`, the number of .ini entries loaded.
 * "b984011b ldrsw"@0xffffff8008a61fa8 (sign extension) and
 * "b9440328 ldr"@0xffffff8008a5db3c. NAME CHOSEN.
 */
int ilitek_ini_num;

/*
 * 0xffffff800a0ff408: pointer to the run of entries.
 * "f942051c ldr"@0xffffff8008a61fb8 and "f942071b ldr"@0xffffff8008a5db58.
 * Whether 0xa0ff400 and 0xa0ff408 are two distinct symbols or two fields of
 * one structure is NOT decided by the binary: clang redoes the `adrp` for
 * every offset either way. The measurement does not change. NAME CHOSEN.
 */
struct ilitek_ini_item *ilitek_ini_data;

/*
 * 0xffffff8008a6b5b8, beyond the flash group: declared and NOT defined
 * (rule 6). "94003660 bl"@0xffffff8008a5dc38. It takes a `char *` and returns
 * an `int`: the result ends up in an array of `int` through
 * "b836d900 str"@0xffffff8008a5dc48 (`str w0`, four bytes).
 */
extern int katoi(char *str);

void parser_ini_nodetype(int *type_ptr, char *desp, int frame_len);
void parser_ini_benchmark(int *max_ptr, int *min_ptr, u8 pct, char *desp,
			  int frame_len);
void dump_node_type_buffer(int *ptr);

/*
 * 0xffffff8008a593c8, group D: declared and NOT defined (rule 6).
 * `pin_test` calls it twice, "97ffd0d1 bl"@0xffffff8008a65084 and
 * "97ffd0bd bl"@0xffffff8008a650d4, and at both sites it prepares NO
 * argument register and tests the result as a negative `int`
 * ("37f80300 tbnz"@0xffffff8008a65088, bit 31): hence `int (void)`.
 * `ilitek.h` does not declare it -- it is in the header delta (entry 8).
 */
int ilitek_tddi_ic_check_int_stat(void);

/*
 * dump_benchmark_data() was reconstructed from the factory kernel disassembly (0xffffff8008a6573c, 300 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static __used void dump_benchmark_data(int *max_ptr, int *min_ptr)
{
	int i;

	if (!ilitek_dbg_en)
		return;

	/* "52802b82 mov"@0xffffff8008a65778 -- __LINE__ = 348 (F1-D1) */
	/* F1-D1: `#line` brings `__LINE__` back to factory line 348 */
#line 348
	ILI_INFO("Dump Benchmark Max\n");
	/* "b94c02e8 ldr"@0xffffff8008a65784 e "7100051f cmp"@0xffffff8008a65788 */
	for (i = 0; i < core_mp.c256; i++) {
		/*
		 * "b8787a81 ldr"@0xffffff8008a657a8 -- ldr w1, [x20,x24,lsl #2]:
		 * four bytes per element, hence `int *`
		 */
		printk(KERN_CONT "%d, ", max_ptr[i]);
		/*
		 * "1ac80f09 sdiv"@0xffffff8008a657b8 and
		 * "1b08e129 msub"@0xffffff8008a657bc are the SIGNED remainder;
		 * "51000508 sub"@0xffffff8008a657c0 is the -1. The field is
		 * loaded once only ("b94beb28 ldr"@0xffffff8008a657b4) and
		 * used for both uses: that is CSE, not two reads
		 */
		if (i % core_mp.c232 == core_mp.c232 - 1)
			printk(KERN_CONT "\n");
	}

	/*
	 * KERN_CONT, three fewer arguments: "9138f400 add"@0xffffff8008a657e8
	 * loads the format into x0 and there is no x1/w2 -- no `__func__`,
	 * no `__LINE__`. It is not ILI_INFO, it is a bare `printk`
	 */
	printk(KERN_CONT "Dump Denchmark Min\n");
	for (i = 0; i < core_mp.c256; i++) {
		/* "b8767a61 ldr"@0xffffff8008a65814 */
		printk(KERN_CONT "%d, ", min_ptr[i]);
		if (i % core_mp.c232 == core_mp.c232 - 1)
			printk(KERN_CONT "\n");
	}
}

/*
 * dump_node_type_buffer() was reconstructed from the factory kernel disassembly (0xffffff8008a5da3c, 180 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void dump_node_type_buffer(int *ptr)
{
	int i;

	/*
	 * "39656108 ldrb"@0xffffff8008a5da54 -- page+2392 = 0xffffff800a0fc958,
	 * a single byte; "34000428 cbz"@0xffffff8008a5da58 jumps to the epilogue
	 */
	if (!ilitek_dbg_en)
		return;

	/* "52802e62 mov"@0xffffff8008a5da70 -- __LINE__ = 371 (F1-D1) */
	/* F1-D1: `#line` brings `__LINE__` back to factory line 371 */
#line 371
	ILI_INFO("Dump NodeType\n");
	/* "b94c02c8 ldr"@0xffffff8008a5da7c, "7100051f cmp"@0xffffff8008a5da80,
	 * "540002cb b.lt"@0xffffff8008a5da84: la guard `c256 < 1` */
	for (i = 0; i < core_mp.c256; i++) {
		/* "b8777a61 ldr"@0xffffff8008a5daa0 */
		printk(KERN_CONT "%d, ", ptr[i]);
		/* "b94beb08 ldr"@0xffffff8008a5daac (pagina+3048),
		 * "1ac80ee9 sdiv"@0xffffff8008a5dab0,
		 * "1b08dd29 msub"@0xffffff8008a5dab4,
		 * "51000508 sub"@0xffffff8008a5dab8,
		 * "6b08013f cmp"@0xffffff8008a5dabc */
		if (i % core_mp.c232 == core_mp.c232 - 1)
			/*
			 * "aa1503e0 mov"@0xffffff8008a5dac4 loads
			 * 0xffffff80090ddcf1, which carries the bytes 01 63 0a:
			 * KERN_CONT + "\n". The linker shares it with another
			 * 170 functions in the kernel
			 */
			printk(KERN_CONT "\n");
		/*
		 * "b98c02c8 ldrsw"@0xffffff8008a5dacc -- the limit is RE-READ on
		 * every iteration: it is a global and the `printk` might change it
		 */
	}
}

/*
 * parser_get_ini_key_value() was reconstructed from the factory kernel disassembly (0xffffff8008a61f88, 332 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static __used int parser_get_ini_key_value(char *section, char *key, char *value)
{
	int i;
	int len;

	/*
	 * "b984011b ldrsw"@0xffffff8008a61fa8 and
	 * "7100077f cmp"@0xffffff8008a61fac: the count is read ONCE only and
	 * never read back -- `strlen`/`strncmp` are `readonly` to the compiler,
	 * `memcpy` is not (and indeed after the `memcpy` the pointer is read
	 * again: "f9420508 ldr"@0xffffff8008a6209c)
	 */
	for (i = 0; i < ilitek_ini_num; i++) {
		/*
		 * "eb0002df cmp"@0xffffff8008a61fe4 -- a 64-bit comparison between
		 * two `size_t`. `strlen(section)` is computed once only,
		 * outside the loop ("940fd8ce bl"@0xffffff8008a61fc8)
		 */
		if (strlen(section) != strlen(ilitek_ini_data[i].c0))
			continue;
		/* "940fd8e3 bl"@0xffffff8008a61ff8 with x2 = strlen(section) */
		if (strncmp(section, ilitek_ini_data[i].c0, strlen(section)) != 0)
			continue;
		/*
		 * "940fd8bd bl"@0xffffff8008a6200c -- `strlen(key)` is INSIDE the
		 * loop instead, and that proves it comes after the section
		 * comparison in the source: outside that branch it is not
		 * guaranteed and the compiler cannot hoist it
		 */
		if (strlen(key) != strlen(ilitek_ini_data[i].c100))
			continue;
		if (strncmp(key, ilitek_ini_data[i].c100, strlen(key)) != 0)
			continue;

		/*
		 * "5280fa08 mov"@0xffffff8008a62074 -- 0x7d0 = 2000, and
		 * "1a88b128 csel"@0xffffff8008a62080 with condition `lt`:
		 * a SIGNED minimum, so both operands are `int`.
		 * "93407d02 sxtw"@0xffffff8008a62084 widens it for the
		 * `memcpy` argument. WHICH source form the factory used is not
		 * decided by the binary: `min_t` and the ternary give the same
		 */
		len = min_t(int, ilitek_ini_data[i].c2208, 2000);
		memcpy(value, ilitek_ini_data[i].c200, len);
		/*
		 * "39656108 ldrb"@0xffffff8008a62090 and
		 * "340001c8 cbz"@0xffffff8008a62094: the guard skips the
		 * construction of ALL the arguments, not just the call --
		 * that is the shape of ILI_DBG.
		 * "52803162 mov"@0xffffff8008a620bc -- __LINE__ = 395 (F1-D1).
		 * x5 = "91019105 add"@0xffffff8008a620b0 (+0x64) and
		 * x6 = "91032106 add"@0xffffff8008a620b4 (+0xc8)
		 */
		/* F1-D1: `#line` brings `__LINE__` back to factory line 395 */
#line 395
		ILI_DBG("(key: %s, value:%s) => (ini key: %s, val: %s)\n", key, value, ilitek_ini_data[i].c100, ilitek_ini_data[i].c200);
		return 0;
	}

	return -2;
}

/*
 * parser_ini_nodetype() was reconstructed from the factory kernel disassembly (0xffffff8008a5daf0, 500 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void parser_ini_nodetype(int *type_ptr, char *desp, int frame_len)
{
	int i, j;
	int count = 0;		/* "2a1f03f6 mov"@0xffffff8008a5db4c: zeroed
				 * BEFORE the outer loop, so it lives across
				 * all sections
				 */
	char str[512];		/* "321703e2 orr"@0xffffff8008a5db28: 0x200 */

	memset(str, 0, sizeof(str));

	/* "b9440328 ldr"@0xffffff8008a5db3c e "7100051f cmp"@0xffffff8008a5db40 */
	for (i = 0; i < ilitek_ini_num; i++) {
		int index;
		char pre;

		/* "94102c01 bl"@0xffffff8008a5db64 e
		 * "b40009a0 cbz"@0xffffff8008a5db68 */
		if (strstr(ilitek_ini_data[i].c0, desp) == NULL)
			continue;
		/*
		 * "f100241f cmp"@0xffffff8008a5db7c (9) and
		 * "52800122 mov"@0xffffff8008a5db88 (9): they are
		 * `strlen("node type")` folded by the compiler
		 */
		if (strlen(ilitek_ini_data[i].c100) != strlen("node type") ||
		    strncmp(ilitek_ini_data[i].c100, "node type", strlen("node type")) != 0)
			continue;

		index = 0;
		/* "52800589 mov"@0xffffff8008a5dbb0 -- 0x2c = ',' */
		pre = ',';
		/*
		 * "37f807cb tbnz"@0xffffff8008a5dba4: if bit 31 of the
		 * length is set everything is skipped -- it is the guard of a
		 * `j <= len` loop with a SIGNED `len`.
		 * "54fff9ed b.le"@0xffffff8008a5dc78 closes with `<=`, not `<`
		 */
		for (j = 0; j <= ilitek_ini_data[i].c2208; j++) {
			/*
			 * "3943218c ldrb"@0xffffff8008a5dbc0 -- the read sits
			 * at the TOP, before both comparisons: that is what
			 * forces the character test to come first in the `||`
			 * (see the note in the file header)
			 */
			char c = ilitek_ini_data[i].c200[j];

			/* "7100ed9f cmp"@0xffffff8008a5dbcc -- 0x3b = ';';
			 * "6b0b015f cmp"@0xffffff8008a5dbc4 -- j against len */
			if (c == ';' || j == ilitek_ini_data[i].c2208) {
				/*
				 * "12001d29 and"@0xffffff8008a5dbdc:
				 * masking to 8 bits before the comparison --
				 * `pre` is one byte wide, not an `int`;
				 * "7100b93f cmp"@0xffffff8008a5dbe0 -- 0x2e='.'
				 */
				if (pre != '.') {
					/*
					 * "7108015f cmp"@0xffffff8008a5dbf4 (0x200)
					 * and "1a89b148 csel"@0xffffff8008a5dc00
					 * (`lt`, signed)
					 */
					int len = min_t(int, j - index, 512);
					int value;

					/*
					 * the memset-tail plus memcpy pair is
					 * what clang produces from
					 * `memset(str,0,512); memcpy(str,...)`:
					 * "940fe8f8 bl"@0xffffff8008a5dc20 and
					 * "940fe834 bl"@0xffffff8008a5dc30
					 */
					memset(str, 0, sizeof(str));
					memcpy(str, &ilitek_ini_data[i].c200[index], len);
					/* "94003660 bl"@0xffffff8008a5dc38 */
					value = katoi(str);
					/*
					 * the comparison comes AFTER the call:
					 * "6b1302df cmp"@0xffffff8008a5dc3c and
					 * "5400020a b.ge"@0xffffff8008a5dc40.
					 * Clang does not speculate calls, so in
					 * the source `katoi` comes first -- which
					 * exact form the factory used is not
					 * decided by the binary
					 */
					if (count >= frame_len) {
						/* "52803502 mov"@0xffffff8008a5dc88
						 * -- __LINE__ = 424 (F1-D1);
						 * "2a1603e3 mov"@0xffffff8008a5dc94
						 * -- w3 = count, the `%d` IS THERE */
						/* "\x013ILITEK: (%s, %d): count(%d) is larger than frame length, break\n"@0xffffff80092430ed,
						 * __func__ = "parser_ini_nodetype"@0xffffff800924312f */
						/* F1-D1: `#line` brings `__LINE__` back to factory line 424 */
#line 424
						ILI_ERR("count(%d) is larger than frame length, break\n", count);
						break;
					}
					/*
					 * "b836d900 str"@0xffffff8008a5dc48 --
					 * `str w0, [x8,w22,sxtw #2]`: the index is
					 * SIGN-extended, four bytes
					 */
					type_ptr[count] = value;
					count++;
				}
				index = j + 1;
				/*
				 * a re-read, not a reuse: on the branch that skips the
				 * field the compiler merges it into the read at the
				 * top ("2a0c03e9 mov"@0xffffff8008a5dc68),
				 * on the one that converts it has to re-read after the
				 * calls ("3943210c ldrb"@0xffffff8008a5dc5c)
				 */
				pre = ilitek_ini_data[i].c200[j];
			}
		}
	}
}

/*
 * parser_ini_benchmark() was reconstructed from the factory kernel disassembly (0xffffff8008a5dce4, 860 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void parser_ini_benchmark(int *max_ptr, int *min_ptr, u8 pct, char *desp,
			  int frame_len)
{
	int i, j;
	int k = 0;		/* "2a1f03f4 mov"@0xffffff8008a5ddb0: outside the
				 * outer loop, like `count` in nodetype
				 */
	int tmp[4];
	char str[512];
	char section[256] = {0};	/* the sixteen "stp xzr, xzr" from
					 * 0xffffff8008a5dd54 a
					 * 0xffffff8008a5dd94: 256 byte */

	memset(str, 0, sizeof(str));
	/* "9410320b bl"@0xffffff8008a5dd94, con
	 * x2 = "%s%s%s"@0xffffff8009179329,
	 * x4 = "_"@0xffffff80090f3fa2,
	 * x5 = "benchmark_data"@0xffffff8009243e2e,
	 * w1 = "321803e1 orr"@0xffffff8008a5dd4c (0x100 = 256) */
	snprintf(section, sizeof(section), "%s%s%s", desp, "_", "benchmark_data");

	for (i = 0; i < ilitek_ini_num; i++) {
		int index;
		char pre;

		/*
		 * "eb00037f cmp"@0xffffff8008a5dddc; the `strncmp` takes as its
		 * length `strlen(rec.c0)`, not `strlen(section)`:
		 * "aa1b03e2 mov"@0xffffff8008a5ddec
		 */
		if (strlen(ilitek_ini_data[i].c0) != strlen(section) ||
		    strncmp(ilitek_ini_data[i].c0, section, strlen(ilitek_ini_data[i].c0)) != 0)
			continue;
		/* "f100381f cmp"@0xffffff8008a5de08 (0xe = 14) e
		 * "321f0be2 orr"@0xffffff8008a5de14 (14): is
		 * `strlen("benchmark_data")` folded */
		if (strlen(ilitek_ini_data[i].c100) != strlen("benchmark_data") ||
		    strncmp(ilitek_ini_data[i].c100, "benchmark_data", strlen("benchmark_data")) != 0)
			continue;

		index = 0;
		/* "52800589 mov"@0xffffff8008a5de3c -- ',' */
		pre = ',';
		/* "37f80e4b tbnz"@0xffffff8008a5de30 e
		 * "54fff34d b.le"@0xffffff8008a5dfd8: same `<=` loop */
		for (j = 0; j <= ilitek_ini_data[i].c2208; j++) {
			/*
			 * here the delimiters are THREE and not one, and clang
			 * reduces them to a bit mask:
			 * "7100ed9f cmp"@0xffffff8008a5de50 (c <= 0x3b),
			 * "d2ca001c mov"@0xffffff8008a5dda8 and
			 * "f2e1001c movk"@0xffffff8008a5ddbc build
			 * 0x0800500000000000, which has exactly bits 44 (','),
			 * 46 ('.') and 59 (';') set, and
			 * "ea1c015f tst"@0xffffff8008a5de60 tests it
			 */
			if (ilitek_ini_data[i].c200[j] == ',' ||
			    ilitek_ini_data[i].c200[j] == '.' ||
			    ilitek_ini_data[i].c200[j] == ';' ||
			    j == ilitek_ini_data[i].c2208) {
				/* "12001d29 and"@0xffffff8008a5de68 e
				 * "7100b93f cmp"@0xffffff8008a5de6c */
				if (pre != '.') {
					int len = min_t(int, j - index, 512);
					int row;

					memset(str, 0, sizeof(str));
					memcpy(str, &ilitek_ini_data[i].c200[index], len);
					/*
					 * "940035bd bl"@0xffffff8008a5dec4 and
					 * "b828d920 str"@0xffffff8008a5deec:
					 * the write into `tmp` happens BEFORE
					 * the bound check -- the opposite of
					 * what nodetype does
					 */
					tmp[k % 4] = katoi(str);
					/*
					 * "11000e88 add"@0xffffff8008a5decc,
					 * "1a94b108 csel"@0xffffff8008a5ded4 and
					 * "13027d03 asr"@0xffffff8008a5ded8:
					 * division by 4 rounding towards zero,
					 * that is, `k` is a signed `int`;
					 * "121e7508 and"@0xffffff8008a5dedc
					 * and "4b080288 sub"@0xffffff8008a5dee0
					 * are the remainder
					 */
					row = k / 4;
					if (row >= frame_len) {
						/*
						 * "52803a02 mov"@0xffffff8008a5dfe8
						 * -- __LINE__ = 464. NO w3:
						 * the `%d` in the format has no
						 * argument. A FACTORY DEFECT,
						 * reproduced (F1-D5)
						 */
						/* "\x013ILITEK: (%s, %d): count (%d) is larger than frame length, break\n"@0xffffff8009243143,
						 * __func__ = "parser_ini_benchmark"@0xffffff8009243186 */
						/* F1-D1: `#line` brings `__LINE__` back to factory line 464 */
#line 464
						ILI_ERR("count (%d) is larger than frame length, break\n");
						break;
					}
					/* "71000d1f cmp"@0xffffff8008a5def4 */
					if (k % 4 == 3) {
						/* "b94123e8 ldr"@0xffffff8008a5defc
						 * (tmp[0]) e
						 * "7100051f cmp"@0xffffff8008a5df00 */
						if (tmp[0] == 1) {
							/* "72001d5f tst"@0xffffff8008a5df14 */
							if (pct) {
								/*
								 * "1b087d29 mul"@0xffffff8008a5df20,
								 * "5290a3eb mov"@0xffffff8008a5df1c +
								 * "72aa3d6b movk"@0xffffff8008a5df24
								 * (0x51eb851f) and
								 * "9365fd29 asr"@0xffffff8008a5df30
								 * (>>37): that is the division by 100
								 */
								max_ptr[row] = tmp[1] + tmp[1] * tmp[2] / 100;
								min_ptr[row] = tmp[1] - tmp[1] * tmp[3] / 100;
							} else {
								/* "0b080129 add"@0xffffff8008a5df90 e
								 * "4b0a0108 sub"@0xffffff8008a5dfa0 */
								max_ptr[row] = tmp[1] + tmp[2];
								min_ptr[row] = tmp[1] - tmp[3];
							}
						} else {
							/* "32007bea orr"@0xffffff8008a5df78
							 * -- 0x7fffffff;
							 * "320103e8 orr"@0xffffff8008a5df84
							 * -- 0x80000000 */
							max_ptr[row] = INT_MAX;
							min_ptr[row] = INT_MIN;
						}
					}
					k++;
				}
				index = j + 1;
				/* "3943210c ldrb"@0xffffff8008a5dfbc e
				 * "2a0c03e9 mov"@0xffffff8008a5dfc8 */
				pre = ilitek_ini_data[i].c200[j];
			}
		}
	}
}

/*
 * parser_get_u8_array() was reconstructed from the factory kernel disassembly (0xffffff8008a659d0, 292 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static __used int parser_get_u8_array(char *key, u8 *buf, int len)
{
	char *s = key;
	char *tok;
	long long lval = 0;	/* "a90083ff stp"@0xffffff8008a659f8 zeroes the
				 * slot at [sp,#8], which
				 * "910023e2 add"@0xffffff8008a65ab0 passes to
				 * `kstrtoll`: eight bytes, hence `long long`
				 */
	int ret;
	int i;

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a659f0).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	if (strlen(key) == 0 || len < 1) {
		/* "\x013ILITEK: (%s, %d): Can't find any characters inside buffer\n"@0xffffff800924409a,
		 * __func__ = "parser_get_u8_array"@0xffffff80092440d7 */
		/* F1-D1: `#line` brings `__LINE__` back to factory line 532 */
#line 532
		ILI_ERR("Can't find any characters inside buffer\n");
		return -1;
	}

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a65a0c).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	if (key[0] == ' ' || key[0] == '\t' || key[0] == '\n' ||
	    key[0] == '\r' || key[0] == '\f' || key[0] == '\b')
		return 0;

	for (i = 0; i < len; i++) {
		/* "94100b8b bl"@0xffffff8008a65aa8, with x1 =
		 * ","@0xffffff8009245907 e x0 = &s ([sp,#0x10]) */
		tok = strsep(&s, ",");
		/* "b4fffd20 cbz"@0xffffff8008a65aac */
		if (tok == NULL)
			break;
		/* "321c03e1 orr"@0xffffff8008a65ab4 -- base 0x10 = 16;
		 * "97e779fb bl"@0xffffff8008a65ab8 */
		ret = kstrtoll(tok, 16, &lval);
		/* "340000e0 cbz"@0xffffff8008a65abc */
		if (ret)
			/* "528044a2 mov"@0xffffff8008a65ac4 -- __LINE__ = 549;
			 * "2a0003e3 mov"@0xffffff8008a65ac0 -- w3 = ret */
			/* "\x016ILITEK: (%s, %d): convert string too long, ret = %d\n"@0xffffff80092440eb */
			/* F1-D1: `#line` brings `__LINE__` back to factory line 549 */
#line 549
			ILI_INFO("convert string too long, ret = %d\n", ret);
		else
			/*
			 * "f94007e8 ldr"@0xffffff8008a65ad8 (eight bytes) and
			 * "38346a68 strb"@0xffffff8008a65adc (one): the
			 * truncation is the factory's
			 */
			buf[i] = lval;
	}

	return i;
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009245a9c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const int mp_nb_dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
static const int mp_nb_dy[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

/*
 * compare_charge() was reconstructed from the factory kernel disassembly (0xffffff8008a5e040, 192 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int compare_charge(int *p, int x, int y, int *type, int t0, int t1, int t2)
{
	int idx = core_mp.c232 * y + x;
	int ret = p[idx];
	int thr;
	int i;

	/* "34000500 cbz"@0xffffff8008a5e054 */
	if (ret == 0)
		return ret;

	/*
	 * "b86a786a ldr"@0xffffff8008a5e05c -- a single read of `type[idx]`,
	 * and the three `tbnz`/`tbz` test bits of the same register
	 */
	if (type[idx] & 0x01)
		thr = t0;
	else if (type[idx] & 0x02)
		thr = t1;
	else if (type[idx] & 0x04)
		thr = t2;
	else
		return ret;

	for (i = 0; i < 8; i++) {
		int nx = mp_nb_dx[i] + x;
		int ny = mp_nb_dy[i] + y;
		int nidx;

		/* "2b0101ce adds"@0xffffff8008a5e098 +
		 * "54000264 b.mi"@0xffffff8008a5e09c */
		if (nx < 0 || nx >= core_mp.c232)
			continue;
		/* "37f801cf tbnz"@0xffffff8008a5e0b0 (bit 31) */
		if (ny < 0 || ny >= core_mp.c236)
			continue;
		nidx = core_mp.c232 * ny + nx;
		if (type[nidx] == 0)
			continue;
		if (type[nidx] & 0x18)
			continue;
		/* "3400008e cbz"@0xffffff8008a5e0d8 */
		if (p[nidx] == 0)
			continue;
		/* "4b0001ce sub"@0xffffff8008a5e0dc */
		if (p[nidx] - ret > thr)
			return thr;
	}

	return ret;
}

/*
 * full_open_rate_compare() was reconstructed from the factory kernel disassembly (0xffffff8008a5e100, 80 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int full_open_rate_compare(int *p1, int *p2, int x, int y, int type, int rate)
{
	int ret = 1;
	int idx;

	/* "34000224 cbz"@0xffffff8008a5e104 e
	 * "37180204 tbnz"@0xffffff8008a5e108 */
	if (type == 0 || (type & 0x08))
		return ret;

	idx = core_mp.c232 * y + x;
	/* "6b09011f cmp"@0xffffff8008a5e140 + "1a9fb7e8 cset"@0xffffff8008a5e144 */
	if (p1[idx] < (p2[idx] * rate) / 100)
		ret = 0;

	return ret;
}

/*
 * check_int_level() was reconstructed from the factory kernel disassembly (0xffffff8008a5e528, 240 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int check_int_level(bool high)
{
	int timer = 500;
	int ret;

	while (timer--) {
		ret = gpio_get_value(66);
		/* "\x016ILITEK: (%s, %d): int GPIO level = %d\n"@0xffffff80092431c1,
		 * __func__ = "check_int_level"@0xffffff80092431ea;
		 * "5280fb02 mov"@0xffffff8008a5e564 -- __LINE__ = 2008 */
		/* F1-D1: `#line` brings `__LINE__` back to factory line 2008 */
#line 2008
		ILI_INFO("int GPIO level = %d\n", ret);

		if (high) {
			if (ret) {
				/* "\x016ILITEK: (%s, %d): check int high sucess \n"@0xffffff80092431fa;
				 * "5280fb62 mov"@0xffffff8008a5e5e0 -- __LINE__ = 2011 */
#line 2011
				ILI_INFO("check int high sucess \n");
				return 0;
			}
		} else {
			if (!ret) {
				/* "\x016ILITEK: (%s, %d): check int low sucess \n"@0xffffff8009243226;
				 * "5280fc22 mov"@0xffffff8008a5e5f8 -- __LINE__ = 2017 */
#line 2017
				ILI_INFO("check int low sucess \n");
				return 0;
			}
		}

		/* "528431c0 mov"@0xffffff8008a5e588 = 8590 = 2 * 0x10c7 */
		udelay(2);
		/*
		 * the decrement is in the condition: see finding F2-S2
		 * in the header of this function
		 */
	}

	/* "\x016ILITEK: (%s, %d): check int %s fail \n"@0xffffff8009243251;
	 * "5280fd42 mov"@0xffffff8008a5e5c0 -- __LINE__ = 2026 */
#line 2026
	ILI_INFO("check int %s fail \n", high ? "High" : "Low");
	return -1;
}

/*
 * compare_MaxMin_result() was reconstructed from the factory kernel disassembly (0xffffff8008a65868, 360 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static __used void compare_MaxMin_result(int index, int *buf)
{
	int i;
	int j;

	/* "7100045f cmp"@0xffffff8008a65870 + "54000acb b.lt"@0xffffff8008a65874 */
	for (i = 0; i < core_mp.c236; i++) {
		/* "7100047f cmp"@0xffffff8008a658c0 +
		 * "540007eb b.lt"@0xffffff8008a658c4 */
		for (j = 0; j < core_mp.c232; j++) {
			int idx;

			/*
			 * "b94001c4 ldr"@0xffffff8008a658cc: the field is
			 * re-read on every iteration, for the same aliasing reason
			 */
			switch (mp_items[index].c16) {
			case 4:
				idx = i * core_mp.c232 + j;
				/* "6b0400bf cmp"@0xffffff8008a6593c +
				 * "540000ea b.ge"@0xffffff8008a65940 */
				if (core_mp.tx_delta_buf[idx] < buf[idx])
					core_mp.tx_max_buf[idx] = buf[idx];
				/* "6b0400bf cmp"@0xffffff8008a6595c +
				 * "540000ad b.le"@0xffffff8008a65960 */
				if (core_mp.tx_delta_buf[idx] > buf[idx])
					core_mp.tx_min_buf[idx] = buf[idx];
				/* "6b0400bf cmp"@0xffffff8008a6597c +
				 * "540000ea b.ge"@0xffffff8008a65980 */
				if (core_mp.rx_delta_buf[idx] < buf[idx])
					core_mp.rx_max_buf[idx] = buf[idx];
				/* "6b0400bf cmp"@0xffffff8008a6599c +
				 * "5400006d b.le"@0xffffff8008a659a0 */
				if (core_mp.rx_delta_buf[idx] > buf[idx])
					core_mp.rx_min_buf[idx] = buf[idx];
				break;
			case 5:
				return;
			default:
				idx = i * core_mp.c232 + j;
				/* "6b0300df cmp"@0xffffff8008a658f8 +
				 * "5400008a b.ge"@0xffffff8008a658fc */
				if (mp_items[index].max_buf[idx] < buf[idx])
					mp_items[index].max_buf[idx] = buf[idx];
				/* "6b0300df cmp"@0xffffff8008a65914 +
				 * "540004ad b.le"@0xffffff8008a65918 */
				if (mp_items[index].min_buf[idx] > buf[idx])
					mp_items[index].min_buf[idx] = buf[idx];
				break;
			}
		}
	}
}

/*
 * self_test() was reconstructed from the factory kernel disassembly (0xffffff8008a64e94, 44 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static __used int self_test(int index)
{
	/* "\x013ILITEK: (%s, %d): TDDI has no self to be tested currently\n"@0xffffff8009244805,
	 * __func__ = "self_test"@0xffffff8009244842;
	 * "52814402 mov"@0xffffff8008a64eac -- __LINE__ = 2592 */
#line 2592
	ILI_ERR("TDDI has no self to be tested currently\n");
	return -1;
}

static __used int st_test(int index)
{
	/* "\x013ILITEK: (%s, %d): ST Test is not supported by the driver\n"@0xffffff800924484c,
	 * __func__ = "st_test"@0xffffff8009244888;
	 * "528144c2 mov"@0xffffff8008a64ed8 -- __LINE__ = 2598 */
#line 2598
	ILI_ERR("ST Test is not supported by the driver\n");
	return -1;
}

/*
 * mp_comp_result_before_retry() was reconstructed from the factory kernel disassembly (0xffffff8008a66c84, 156 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mp_comp_result_before_retry(int index);

static __used void mp_do_retry(int index, int count)
{
	/* "34000321 cbz"@0xffffff8008a66c94 */
	if (count == 0) {
		/* "\x016ILITEK: (%s, %d): Finish retry action\n"@0xffffff8009245369,
		 * __func__ = "mp_do_retry"@0xffffff8009245392;
		 * "52816222 mov"@0xffffff8008a66d08 -- __LINE__ = 2833 */
#line 2833
		ILI_INFO("Finish retry action\n");
		return;
	}

	/* "\x016ILITEK: (%s, %d): retry = %d, item = %s\n"@0xffffff800924539e;
	 * "528162a2 mov"@0xffffff8008a66cc4 -- __LINE__ = 2837 */
#line 2837
	ILI_INFO("retry = %d, item = %s\n", count, mp_items[index].c0);

	mp_items[index].c144(index);

	if (mp_comp_result_before_retry(index) < 0)
		mp_do_retry(index, count - 1);
}

/*
 * mp_compare_cdc_show_result() was reconstructed from the factory kernel disassembly (0xffffff8008a66d20, 1068 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static __used void mp_compare_cdc_show_result(int index, int *data, char *buf,
					      int *plen, int type, int *max,
					      int *min, char *desp)
{
	int len = *plen;
	int ret = 0;
	int i;
	int j;

	/* "b140043f cmn"@0xffffff8008a66d54 + "b4001af6 cbz"@0xffffff8008a66d60 */
	if (IS_ERR(data) || data == NULL) {
		/* "\x013ILITEK: (%s, %d): The data of test item is null (%p)\n"@0xffffff80092451d1,
		 * __func__ = "mp_compare_cdc_show_result"@0xffffff80092458d1;
		 * "52809062 mov"@0xffffff8008a670cc -- __LINE__ = 1155 */
		/* F1-D1: `#line` brings `__LINE__` back to factory line 1155 */
#line 1155
		ILI_ERR("The data of test item is null (%p)\n", data);
		/* "12800d1c mov"@0xffffff8008a670d8 -- w28 = 0xffffff97 = -105 */
		ret = -105;
		goto out;
	}

	/* "7100051f cmp"@0xffffff8008a66d70 + "5400052b b.lt"@0xffffff8008a66d74 */
	for (i = 0; i < core_mp.c232; i++) {
		/*
		 * "350001fa cbnz"@0xffffff8008a66d98: the test is on the value
		 * BEFORE the increment, that is, on `i`
		 */
		if (i == 0) {
			if (ilitek_dbg_en)
				/*
				 * "\n %s "@0xffffff80092458ee (the tail of the
				 * KERN_CONT string that starts at
				 * 0xffffff80092458ec)
				 */
				ILI_CONT("\n %s ", desp);
			/* "\n\t   %s ,"@0xffffff80092458f4 */
			len += snprintf(buf + len, 1024 * 1024 - len,
					"\n\t   %s ,", desp);
		}
		if (ilitek_dbg_en)
			/* "  X_%d\t,"@0xffffff8009245900 */
			ILI_CONT("  X_%d\t,", i + 1);
		/* "\t X_%d  ,"@0xffffff8009245909 */
		len += snprintf(buf + len, 1024 * 1024 - len,
				"\t X_%d  ,", i + 1);
	}

	if (ilitek_dbg_en)
		/*
		 * ILI_CONT() was reconstructed from the factory kernel disassembly (0xffffff80090ddcf3).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		ILI_CONT("\n");
	len += snprintf(buf + len, 1024 * 1024 - len, "\n");

	/* "7100051f cmp"@0xffffff8008a66e58 + "5400146b b.lt"@0xffffff8008a66e5c */
	for (i = 0; i < core_mp.c236; i++) {
		if (ilitek_dbg_en)
			/* "  Y_%d\t,"@0xffffff8009245915 */
			ILI_CONT("  Y_%d\t,", i + 1);
		/* "\t Y_%d  ,"@0xffffff800924591e */
		len += snprintf(buf + len, 1024 * 1024 - len,
				"\t Y_%d  ,", i + 1);

		for (j = 0; j < core_mp.c232; j++) {
			int idx = i * core_mp.c232 + j;

			/* "7100253f cmp"@0xffffff8008a66ee8 (con 9) */
			if (mp_items[index].c16 == 9) {
				if (data[idx] < min[idx]) {
					if (ilitek_dbg_en)
						/* " #%7d "@0xffffff800924592a */
						ILI_CONT(" #%7d ", data[idx]);
					/* "#%7d,"@0xffffff8009245931 */
					len += snprintf(buf + len,
							1024 * 1024 - len,
							"#%7d,", data[idx]);
					ret = -1;
				} else {
					if (ilitek_dbg_en)
						/* " %7d "@0xffffff8009245939 */
						ILI_CONT(" %7d ", data[idx]);
					/* " %7d, "@0xffffff800924593f */
					len += snprintf(buf + len,
							1024 * 1024 - len,
							" %7d, ", data[idx]);
				}
			} else if (data[idx] > max[idx] && type == 2) {
				if (ilitek_dbg_en)
					/* " *%7d "@0xffffff8009245950 */
					ILI_CONT(" *%7d ", data[idx]);
				/* "*%7d,"@0xffffff8009245957 */
				len += snprintf(buf + len, 1024 * 1024 - len,
						"*%7d,", data[idx]);
				ret = -1;
			} else if (data[idx] < min[idx] && type == 2) {
				if (ilitek_dbg_en)
					ILI_CONT(" #%7d ", data[idx]);
				len += snprintf(buf + len, 1024 * 1024 - len,
						"#%7d,", data[idx]);
				ret = -1;
			} else if (type == 0 && (data[idx] == INT_MAX ||
						 data[idx] == INT_MIN)) {
				if (ilitek_dbg_en)
					/* "%s"@0xffffff8009101762 */
					ILI_CONT("%s", "BYPASS,");
				/*
				 * "BYPASS,"@0xffffff8009245946 -- the same address
				 * serves both as the `printk` argument and as the
				 * `snprintf` format:
				 * "f0003ef7 adrp"@0xffffff8008a66fd4 with
				 * "91251af7 add"@0xffffff8008a66fd8 materialises it
				 * once only, in x23
				 */
				len += snprintf(buf + len, 1024 * 1024 - len,
						"BYPASS,");
			} else {
				if (ilitek_dbg_en)
					ILI_CONT(" %7d ", data[idx]);
				len += snprintf(buf + len, 1024 * 1024 - len,
						" %7d, ", data[idx]);
			}
		}

		if (ilitek_dbg_en)
			ILI_CONT("\n");
		len += snprintf(buf + len, 1024 * 1024 - len, "\n");
	}

out:
	/* "71000abf cmp"@0xffffff8008a67098 + "54000441 b.ne"@0xffffff8008a6709c */
	if (type == 2) {
		/* "340002dc cbz"@0xffffff8008a670a0 */
		if (ret == 0) {
			/* "\x016\n Result : PASS\n"@0xffffff800924595d */
			printk(KERN_INFO "\n Result : PASS\n");
			/* "Result : PASS\n"@0xffffff8009245961 */
			len += snprintf(buf + len, 1024 * 1024 - len,
					"Result : PASS\n");
		} else {
			/* "\x016\n Result : FAIL\n"@0xffffff8009245970 */
			printk(KERN_INFO "\n Result : FAIL\n");
			/* "Result : FAIL\n"@0xffffff8009245974 */
			len += snprintf(buf + len, 1024 * 1024 - len,
					"Result : FAIL\n");
		}
	}

	/* "b9000118 str"@0xffffff8008a67128 */
	*plen = len;
}
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a65f8c, 3320 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * ipio_kfree() was reconstructed from the factory kernel disassembly (0xffffff8008a66164).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ipio_kfree(void **mem)
{
	if (*mem != NULL) {
		kfree(*mem);
		*mem = NULL;
	}
}

/*
 * mp_test_data_sort_average() was reconstructed from the factory kernel disassembly (0xffffff8008a6660c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void mp_test_data_sort_average(int *data, int index, int *avg_result)
{
	int i, j, k, tmp;
	int idx0, idx1;
	int len;
	int up, down;
	int *u32sum_raw_data = NULL;
	int *sum_buf = NULL;

	/* "b8430f6a ldr"@0xffffff8008a66614 + "7100095f cmp"@0xffffff8008a66618
	 * + "54002c4b b.lt"@0xffffff8008a6661c */
	if (mp_items[index].c48 < 2)
		return;

	/* "b140073f cmn"@0xffffff8008a66620 + "b4000859 cbz"@0xffffff8008a66628 */
	if (IS_ERR(data) || data == NULL) {
		/* "\x013ILITEK: (%s, %d): Input wrong address\n"@0xffffff800924521f,
		 * __func__ = "mp_test_data_sort_average"@0xffffff8009245248;
		 * "528149c2 mov"@0xffffff8008a66740 -- __LINE__ = 2638 */
#line 2638
		ILI_ERR("Input wrong address\n");
		return;
	}

	/* "1b0a7d09 mul"@0xffffff8008a66630, "37f808a9 tbnz"@0xffffff8008a66638,
	 * "97dfa74f bl"@0xffffff8008a6664c */
	u32sum_raw_data = kcalloc(core_mp.c256 * mp_items[index].c48,
				  sizeof(int), GFP_KERNEL);
	/*
	 * "36f807e8 tbz"@0xffffff8008a66658 and "37f82168 tbnz"@0xffffff8008a66750
	 * are the same test on the second count: the second allocation is made
	 * EVEN when the first has overflowed
	 * ("aa1f03f5 mov"@0xffffff8008a6674c zeroes the first pointer and carries on);
	 * "97dfa709 bl"@0xffffff8008a66764
	 */
	sum_buf = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	/*
	 * THE ORDER IS THE BINARY'S, and it is worth twelve bytes (see the file
	 * header): "b140041f cmn"@0xffffff8008a66768 + "b4002060 cbz"@0xffffff8008a66770
	 * test `sum_buf`, "b14006bf cmn"@0xffffff8008a66774 +
	 * "b4002015 cbz"@0xffffff8008a6677c test `u32sum_raw_data`
	 */
	if (IS_ERR(sum_buf) || sum_buf == NULL ||
	    IS_ERR(u32sum_raw_data) || u32sum_raw_data == NULL) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to allocate u32sum_raw_data FRAME buffer\n"@0xffffff8009245262;
		 * "52814aa2 mov"@0xffffff8008a66b8c -- __LINE__ = 2645.
		 * NO `kfree` HERE: this is the factory's memory leak
		 */
#line 2645
		ILI_ERR("Failed to allocate u32sum_raw_data FRAME buffer\n");
		return;
	}

	/*
	 * THE LOCAL IS MEASURED: "1b097d09 mul"@0xffffff8008a6678c once
	 * only, and the loop walks with "b840472a ldr"@0xffffff8008a667a0,
	 * "f1000508 subs"@0xffffff8008a667a4, "b800452a str"@0xffffff8008a667a8
	 */
	len = core_mp.c256 * mp_items[index].c48;
	for (i = 0; i < len; i++)
		u32sum_raw_data[i] = data[i];

	/*
	 * "29472929 ldp"@0xffffff8008a667c8 reads the two fields as a pair;
	 * "1b087d4a mul"@0xffffff8008a667d8 and "1b087d28 mul"@0xffffff8008a667dc;
	 * the division by 100 is "9b2b7d4a smull"@0xffffff8008a667e4 +
	 * "9365fd4a asr"@0xffffff8008a667f0
	 */
	up = mp_items[index].c60 * mp_items[index].c48 / 100;
	down = mp_items[index].c56 * mp_items[index].c48 / 100;

	/*
	 * "\x016ILITEK: (%s, %d): Up=%d, Down=%d -%s\n"@0xffffff80092452a7;
	 * the guard is "39656189 ldrb"@0xffffff8008a667e0 (ilitek_dbg_en),
	 * hence ILI_DBG and not ILI_INFO;
	 * "52814c02 mov"@0xffffff8008a66838 -- __LINE__ = 2656;
	 * "f8696905 ldr"@0xffffff8008a66828 puts the item name into the
	 * fifth argument, that is, into the `%s`
	 */
#line 2656
	ILI_DBG("Up=%d, Down=%d -%s\n", up, down, mp_items[index].c0);

	if (ilitek_dbg_en) {
		/*
		 * "\n[Show Original frist%d and last%d node data]\n"@0xffffff80092452d1 (the
		 * tail of the KERN_CONT string that starts at 0xffffff80092452cf)
		 * -- the "frist" typo for "first" is the factory's and is
		 * reproduced (rule 7). The two 5s are
		 * "528000a1 mov"@0xffffff8008a66858 and
		 * "528000a2 mov"@0xffffff8008a6685c
		 */
		ILI_CONT("\n[Show Original frist%d and last%d node data]\n", 5, 5);
		for (i = 0; i < core_mp.c256; i++) {
			for (j = 0; j < mp_items[index].c48; j++) {
				/* "7100139f cmp"@0xffffff8008a66888 +
				 * "1a9f97f7 cset"@0xffffff8008a66890 +
				 * "1a9fa7ea cset"@0xffffff8008a668a4 +
				 * "0a17014a and"@0xffffff8008a668a8 +
				 * "370000ca tbnz"@0xffffff8008a668ac */
				if (i < 5 || i >= core_mp.c256 - 5)
					/* "%d,"@0xffffff8009245302 (tail of
					 * 0xffffff8009245300);
					 * the index is
					 * "1b197108 madd"@0xffffff8008a668b0
					 * and the datum
					 * "b868daa1 ldr"@0xffffff8008a668b4 */
					ILI_CONT("%d,",
						 u32sum_raw_data[core_mp.c256 * j + i]);
			}
			/* "7100179f cmp"@0xffffff8008a668d0 +
			 * "540000a3 b.cc"@0xffffff8008a668d4 */
			if (i < 5 || i >= core_mp.c256 - 5)
				/*
				 * the literal is the bare "\n", at
				 * 0xffffff80090ddcf1 ("f00033a0
				 * adrp"@0xffffff8008a668e8): it is the same one
				 * `mp_compare_cdc_show_result` uses, and it is the
				 * reason for the one `--eccezione` that remains
				 */
				ILI_CONT("\n");
		}
	}

	for (i = 0; i < core_mp.c256; i++) {
		/* "5100054b sub"@0xffffff8008a66918 +
		 * "7100057f cmp"@0xffffff8008a6691c */
		for (j = 0; j < mp_items[index].c48 - 1; j++) {
			/* "2a2803eb mvn"@0xffffff8008a66928 +
			 * "0b0b014c add"@0xffffff8008a6692c +
			 * "7100059f cmp"@0xffffff8008a66930 */
			for (k = 0; k < mp_items[index].c48 - j - 1; k++) {
				/*
				 * THE TWO INDICES ARE LOCALS, not rewritten
				 * expressions: "1b0c25b0 madd"@0xffffff8008a66940
				 * and "1b0c25af madd"@0xffffff8008a66948 compute
				 * them BEFORE the comparison, and the two `str`s
				 * of the swap reuse them instead of re-reading
				 * `core_mp.c256`
				 */
				idx0 = core_mp.c256 * k + i;
				idx1 = core_mp.c256 * (k + 1) + i;

				/* "b870daad ldr"@0xffffff8008a6694c +
				 * "b86fdaae ldr"@0xffffff8008a66950 +
				 * "6b0e01bf cmp"@0xffffff8008a66954 +
				 * "540000cd b.le"@0xffffff8008a66958 */
				if (u32sum_raw_data[idx0] > u32sum_raw_data[idx1]) {
					/* "b82a7aae str"@0xffffff8008a66964 +
					 * "b82f7aad str"@0xffffff8008a66968 */
					tmp = u32sum_raw_data[idx0];
					u32sum_raw_data[idx0] = u32sum_raw_data[idx1];
					u32sum_raw_data[idx1] = tmp;
				}
			}
		}
	}

	if (ilitek_dbg_en) {
		/*
		 * "\n[After sorting frist%d and last%d node data]\n"@0xffffff8009245308
		 * (tail of 0xffffff8009245306);
		 * the two 5s are "528000a1 mov"@0xffffff8008a669b4 and
		 * "528000a2 mov"@0xffffff8008a669b8
		 */
		ILI_CONT("\n[After sorting frist%d and last%d node data]\n", 5, 5);
		for (i = 0; i < core_mp.c256; i++) {
			/*
			 * "b9402ff9 ldr"@0xffffff8008a669e8 re-reads `down` as the
			 * starting value; the limit is `c48 - up`,
			 * "4b16012a sub"@0xffffff8008a669dc +
			 * "6b0a02ff cmp"@0xffffff8008a669e0 +
			 * "5400028a b.ge"@0xffffff8008a669e4
			 */
			for (j = down; j < mp_items[index].c48 - up; j++) {
				if (i < 5 || i >= core_mp.c256 - 5)
					ILI_CONT("%d,",
						 u32sum_raw_data[core_mp.c256 * j + i]);
			}
			if (i < 5 || i >= core_mp.c256 - 5)
				ILI_CONT("\n");
		}
	}

	for (i = 0; i < core_mp.c256; i++) {
		/* "b82a791f str"@0xffffff8008a66a84 */
		sum_buf[i] = 0;
		/* "4b1601ac sub"@0xffffff8008a66a88 +
		 * "6b0c02ff cmp"@0xffffff8008a66a8c +
		 * "5400018a b.ge"@0xffffff8008a66a94;
		 * the index is "1b0e29ef madd"@0xffffff8008a66aa4 and the datum
		 * "b86fdaaf ldr"@0xffffff8008a66aa8 */
		for (j = down; j < mp_items[index].c48 - up; j++)
			/*
			 * "0b0c01ec add"@0xffffff8008a66ab8 +
			 * "b82a790c str"@0xffffff8008a66abc: the write is
			 * INSIDE the loop, so there is no local accumulator
			 */
			sum_buf[i] += u32sum_raw_data[core_mp.c256 * j + i];
		/*
		 * "0b1602eb add"@0xffffff8008a66a7c (down+up, outside the loop),
		 * "4b0b01ad sub"@0xffffff8008a66ac4,
		 * "1acd0d8c sdiv"@0xffffff8008a66ac8 (SIGNED),
		 * "b82a792c str"@0xffffff8008a66acc
		 */
		avg_result[i] = sum_buf[i] / (mp_items[index].c48 - (down + up));
	}

	if (ilitek_dbg_en) {
		/*
		 * "\n[Average result frist%d and last%d node data]\n"@0xffffff8009245339
		 * (tail of 0xffffff8009245337);
		 * the two 5s are "528000a1 mov"@0xffffff8008a66af4 and
		 * "528000a2 mov"@0xffffff8008a66af8
		 */
		ILI_CONT("\n[Average result frist%d and last%d node data]\n", 5, 5);
		/*
		 * "370000a9 tbnz"@0xffffff8008a66b20 tests, at the top of the round, the
		 * boolean computed in the tail of the PREVIOUS round
		 * ("f1000eff cmp"@0xffffff8008a66b34,
		 * "1a9f97ea cset"@0xffffff8008a66b40,
		 * "eb29c2ff cmp"@0xffffff8008a66b44,
		 * "1a9fa7e9 cset"@0xffffff8008a66b48,
		 * "0a090149 and"@0xffffff8008a66b50)
		 */
		for (i = 0; i < core_mp.c256; i++) {
			if (i < 5 || i >= core_mp.c256 - 5)
				ILI_CONT("%d,", avg_result[i]);
		}
		/*
		 * OUTSIDE THE LOOP, and on an `i` that by now equals `core_mp.c256`:
		 * "37000089 tbnz"@0xffffff8008a66b58 tests the same boolean again
		 * AFTER the exit. See the file header: it is the only writing that
		 * explains both that test and the fact that with
		 * `core_mp.c256 < 1` the "\n" is printed all the same
		 * ("540002ab b.lt"@0xffffff8008a66b08)
		 */
		if (i < 5 || i >= core_mp.c256 - 5)
			ILI_CONT("\n");
	}

	/*
	 * "aa1503e0 mov"@0xffffff8008a66b68 + "97dfa711 bl"@0xffffff8008a66b6c,
	 * then "f94003e0 ldr"@0xffffff8008a66b70 +
	 * "97dfa70f bl"@0xffffff8008a66b74. NO guard: here clang knows the
	 * two pointers are not null
	 */
	kfree(u32sum_raw_data);
	kfree(sum_buf);
}

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * mp_compare_cdc_result() was reconstructed from the factory kernel disassembly (0xffffff8008a662e4).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mp_compare_cdc_result(int index, int *data, int *max, int *min)
{
	int i;

	/* "b140047f cmn"@0xffffff8008a660a8 + "b4001023 cbz"@0xffffff8008a660b0 */
	if (IS_ERR(data) || data == NULL) {
		/* "\x013ILITEK: (%s, %d): The data of test item is null (%p)\n"@0xffffff80092451d1,
		 * __func__ = "mp_compare_cdc_result"@0xffffff8009245209;
		 * "52815482 mov"@0xffffff8008a662c4 -- __LINE__ = 2724 */
#line 2724
		ILI_ERR("The data of test item is null (%p)\n", data);
		/* "12800018 mov"@0xffffff8008a662cc */
		return -1;
	}

	/* "b94002a9 ldr"@0xffffff8008a660b4 + "7100253f cmp"@0xffffff8008a660b8
	 * + "540010c1 b.ne"@0xffffff8008a660bc */
	if (mp_items[index].c16 == 9) {
		for (i = 0; i < core_mp.c256; i++) {
			/* "b86a686b ldr"@0xffffff8008a660d0 +
			 * "b86a6a8a ldr"@0xffffff8008a660d4 +
			 * "6b0a017f cmp"@0xffffff8008a660d8 +
			 * "54000f8b b.lt"@0xffffff8008a660dc */
			if (data[i] < min[i])
				return -1;
		}
	} else {
		for (i = 0; i < core_mp.c256; i++) {
			/* "b86b686a ldr"@0xffffff8008a662e4 +
			 * "b86b6a6b ldr"@0xffffff8008a662e8 +
			 * "6b0b015f cmp"@0xffffff8008a662ec +
			 * "54fffeec b.gt"@0xffffff8008a662f0, then
			 * "b8697a8b ldr"@0xffffff8008a662f8 +
			 * "6b0b015f cmp"@0xffffff8008a662fc +
			 * "54fffe6b b.lt"@0xffffff8008a66300 */
			if (data[i] > max[i] || data[i] < min[i])
				return -1;
		}
	}

	/* "2a1f03f8 mov"@0xffffff8008a66310 */
	return 0;
}

/*
 * mp_comp_result_before_retry() was reconstructed from the factory kernel disassembly (0xffffff8008a65f8c, 3320 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mp_comp_result_before_retry(int index)
{
	int i;
	int res = 0;
	int ret = 0;
	int *thr_max = NULL;
	int *thr_min = NULL;

	/* "b8410ea8 ldr"@0xffffff8008a65fc0 + "7100291f cmp"@0xffffff8008a65fc8;
	 * "b9402515 ldr"@0xffffff8008a65fd8 */
	if (mp_items[index].c16 == 10)
		return mp_items[index].c36;

	/* "b98c0348 ldrsw"@0xffffff8008a65fe4 +
	 * "37f80848 tbnz"@0xffffff8008a65fe8 (the overflow test of
	 * `kmalloc_array`) + "97dfa8e4 bl"@0xffffff8008a65ff8 */
	thr_max = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a66000 + "b4000873 cbz"@0xffffff8008a66008 */
	if (IS_ERR(thr_max) || thr_max == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate threshold FRAME buffer\n"@0xffffff800924512f,
		 * __func__ = "mp_comp_result_before_retry"@0xffffff800924516e;
		 * "52815882 mov"@0xffffff8008a66100 -- __LINE__ = 2756 */
#line 2756
		ILI_ERR("Failed to allocate threshold FRAME buffer\n");
		/* "12800d55 mov"@0xffffff8008a66108 (-107) +
		 * "12800018 mov"@0xffffff8008a6610c (-1) */
		ret = -107;
		res = -1;
		goto out;
	}

	thr_min = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	if (IS_ERR(thr_min) || thr_min == NULL) {
		/* same literal, different line:
		 * "52815982 mov"@0xffffff8008a66154 -- __LINE__ = 2764 */
#line 2764
		ILI_ERR("Failed to allocate threshold FRAME buffer\n");
		ret = -107;
		res = -1;
		goto out;
	}

	/* "b94002a8 ldr"@0xffffff8008a66034 + "7100111f cmp"@0xffffff8008a66038 */
	if (mp_items[index].c16 == 4) {
		/*
		 * THE ORDER IS THE BINARY'S (divergence F4-D2):
		 * "f9461d08 ldr"@0xffffff8008a66044 (rx, page+3128) before
		 * "f9461908 ldr"@0xffffff8008a66058 (tx, page+3120)
		 */
		if (IS_ERR(core_mp.rx_delta_buf) || core_mp.rx_delta_buf == NULL ||
		    IS_ERR(core_mp.tx_delta_buf) || core_mp.tx_delta_buf == NULL) {
			/*
			 * "\x013ILITEK: (%s, %d): This test item (%s) has no data inside its buffer\n"@0xffffff800924518a;
			 * "52815aa2 mov"@0xffffff8008a662a8 -- __LINE__ = 2773.
			 * The item name is the fourth argument,
			 * "f9400363 ldr"@0xffffff8008a6626c on the other branch
			 */
#line 2773
			ILI_ERR("This test item (%s) has no data inside its buffer\n", mp_items[index].c0);
			ret = -107;
			res = -1;
			goto out;
		}

		/*
		 * "2944354c ldp"@0xffffff8008a66084: the two fields are read as a
		 * pair and RE-READ on every iteration, because the two writes
		 * may alias them
		 */
		for (i = 0; i < core_mp.c256; i++) {
			thr_max[i] = core_mp.c288;
			thr_min[i] = core_mp.c292;
		}

		/* "f9462123 ldr"@0xffffff8008a660a4 (pagina+3136) */
		res = mp_compare_cdc_result(index, core_mp.tx_max_buf,
					    thr_max, thr_min);
		/*
		 * "f9462503 ldr"@0xffffff8008a66318 (page+3144): there is NO
		 * `mov wzr` on the success path here, so `res` is not
		 * reassigned when the comparison passes
		 */
		if (mp_compare_cdc_result(index, core_mp.tx_min_buf,
					  thr_max, thr_min) < 0)
			res = -1;

		/* "2945354c ldp"@0xffffff8008a663a0: the SECOND pair of fields,
		 * at +296 and +300 */
		for (i = 0; i < core_mp.c256; i++) {
			thr_max[i] = core_mp.c296;
			thr_min[i] = core_mp.c300;
		}

		/* "f9462923 ldr"@0xffffff8008a663c0 (pagina+3152) */
		if (mp_compare_cdc_result(index, core_mp.rx_max_buf,
					  thr_max, thr_min) < 0)
			res = -1;
		/* "f9462d03 ldr"@0xffffff8008a6642c (pagina+3160) */
		if (mp_compare_cdc_result(index, core_mp.rx_min_buf,
					  thr_max, thr_min) < 0)
			res = -1;
	} else {
		/*
		 * "f8460d28 ldr"@0xffffff8008a661b0 (+96),
		 * "f8468d48 ldr"@0xffffff8008a661c8 (+104),
		 * "f8470f28 ldr"@0xffffff8008a661e0 (+112),
		 * "f8458f08 ldr"@0xffffff8008a661f8 (+88): four `IS_ERR`
		 * tests plus zero, in this order
		 */
		if (IS_ERR(mp_items[index].buf) || mp_items[index].buf == NULL ||
		    IS_ERR(mp_items[index].max_buf) || mp_items[index].max_buf == NULL ||
		    IS_ERR(mp_items[index].min_buf) || mp_items[index].min_buf == NULL ||
		    IS_ERR(mp_items[index].result_buf) || mp_items[index].result_buf == NULL) {
			/*
			 * same literal as line 2773, different line:
			 * "52815d82 mov"@0xffffff8008a66280 -- __LINE__ = 2796.
			 * HERE `ret` STAYS ZERO ("2a1f03f5 mov"@0xffffff8008a66288),
			 * while at line 2773 it is -107: that is the factory's
			 * asymmetry
			 */
#line 2796
			ILI_ERR("This test item (%s) has no data inside its buffer\n", mp_items[index].c0);
			res = -1;
			goto out;
		}

		/* "3940550b ldrb"@0xffffff8008a66210 +
		 * "7100057f cmp"@0xffffff8008a66218 */
		if (mp_items[index].c21 == 1) {
			/*
			 * "9101e10c add"@0xffffff8008a66234 (+0x78) and
			 * "9102010d add"@0xffffff8008a66238 (+0x80): the two
			 * pointers are RE-READ on every iteration
			 */
			for (i = 0; i < core_mp.c256; i++) {
				thr_max[i] = mp_items[index].bench_mark_max[i];
				thr_min[i] = mp_items[index].bench_mark_min[i];
			}
		} else {
			/* "9100710c add"@0xffffff8008a66550 (+0x1c) e
			 * "9100a10d add"@0xffffff8008a66554 (+0x28) */
			for (i = 0; i < core_mp.c256; i++) {
				thr_max[i] = mp_items[index].c28;
				thr_min[i] = mp_items[index].c40;
			}
		}

		/* "b940356b ldr"@0xffffff8008a66584 +
		 * "3400008b cbz"@0xffffff8008a66588 +
		 * "7100217f cmp"@0xffffff8008a66590 */
		if (mp_items[index].c52 != 0 && mp_items[index].c16 != 8) {
			/*
			 * "f9400139 ldr"@0xffffff8008a6660c (buf) and
			 * "f9400309 ldr"@0xffffff8008a66610 (result_buf) are the
			 * two arguments prepared right before the inlined body
			 */
			mp_test_data_sort_average(mp_items[index].buf, index,
						  mp_items[index].result_buf);
			/* "f9400303 ldr"@0xffffff8008a66ba4 */
			res = mp_compare_cdc_result(index,
						    mp_items[index].result_buf,
						    thr_max, thr_min);
		} else {
			/* "f9400143 ldr"@0xffffff8008a66598 */
			res = mp_compare_cdc_result(index,
						    mp_items[index].max_buf,
						    thr_max, thr_min);
			/* "f9400323 ldr"@0xffffff8008a666a0 */
			if (mp_compare_cdc_result(index,
						  mp_items[index].min_buf,
						  thr_max, thr_min) < 0)
				res = -1;
		}
	}

out:
	/*
	 * BY ADDRESS, not by value: see the header of `ipio_kfree`.
	 * "aa1303e0 mov"@0xffffff8008a66164 + "97dfa992 bl"@0xffffff8008a66168,
	 * then "b4000074 cbz"@0xffffff8008a6616c for the second
	 */
	ipio_kfree((void **)&thr_max);
	ipio_kfree((void **)&thr_min);

	/* "b9002518 str"@0xffffff8008a66180 (four byte, offset 36) +
	 * "2a1503e0 mov"@0xffffff8008a66184 */
	mp_items[index].c36 = res;
	return ret;
}

/*
 * ===========================================================================
 * THE CONSTANTS OF BATCH F5 -- the VALUES are measured, the NAMES are CHOSEN
 * ===========================================================================
 */
/* "5280065c mov"@0xffffff8008a5e8a0 e "f100cb9f cmp"@0xffffff8008a5ef38 */
#define MP_TEST_ITEM			50
/* "5280e113 mov"@0xffffff8008a600d4 e "711c211f cmp"@0xffffff8008a601a4 */
#define PARSER_MAX_KEY_NUM		1800
/* "f118013f cmp"@0xffffff8008a601ec */
#define PARSER_MAX_CFG_BUF		1536
/* "7101951f cmp"@0xffffff8008a605a4 (#0x65 come EXCLUSIVE bound) */
#define PARSER_MAX_KEY_NAME_LEN		100
/* "711f445f cmp"@0xffffff8008a604c4 (#0x7d1 come EXCLUSIVE bound) */
#define PARSER_MAX_KEY_VALUE_LEN	2000
/* "f10192ff cmp"@0xffffff8008a5ef48 */
#define PARSER_MAX_SECTION_NUM		100
/* "320c03e0 orr"@0xffffff8008a60054 */
#define CSV_FILE_SIZE			(1024 * 1024)
/* "321403fb orr"@0xffffff8008a607e8 (0x1000) e
 * "321f2be1 orr"@0xffffff8008a607ac (0xffe = 0x1000 - 2) */
#define APK_BUF_SIZE			4096

/*
 * "2.0.6.0.191122"@0xffffff800923ed58, cited twice -- by
 * "91356063 add"@0xffffff8008a5e76c and by "91356063 add"@0xffffff8008a609b0.
 * The macro NAME is CHOSEN.
 */
#define DRIVER_VERSION			"2.0.6.0.191122"

/*
 * 0xffffff800a0fc963, that is eleven bytes after `ilitek_dbg_en`. One byte:
 * "392f8e88 strb"@0xffffff8008a5edb4 writes it and
 * "392f8e9f strb"@0xffffff8008a5ed28 clears it. NONE of the functions
 * written READS it: it is here because `mp_get_timing_info` writes it, and
 * whoever reads it lives in another group. NAME CHOSEN; the merge batch will
 * have to decide which file defines it (an entry in the header delta).
 */
u8 ilitek_mp_ddi_mode;

/*
 * ipio_vfree() was reconstructed from the factory kernel disassembly (0xffffff8008a5f140).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ipio_vfree(void **mem)
{
	if (*mem != NULL) {
		vfree(*mem);
		*mem = NULL;
	}
}

/*
 * ilitek_tddi_mp_test_main() was reconstructed from the factory kernel disassembly (0xffffff8008a5e618, 14704 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * mutual_test() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mutual_test(int index);
static int open_test_sp(int index);
static int open_test_cap(int index);
static int key_test(int index);
static int pin_test(int index);

/*
 * str2hex() was reconstructed from the factory kernel disassembly (0xffffff8008a6b528).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
extern int str2hex(char *str);

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5e6d4).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_c48 {
	u8 c0;		/* "3940012a ldrb"@0xffffff8008a5e6f8 */
	u8 c1;		/* "3940052a ldrb"@0xffffff8008a5e700 */
	u16 c2;		/* "7940052a ldrh"@0xffffff8008a5e6f0, DUE byte */
	u8 c4[4];	/* declared padding */
	int c8;		/* "b940092a ldr"@0xffffff8008a5e6e4 */
	u8 c12[36];	/* declared padding */
	int c48;	/* first word of "2946252a ldp"@0xffffff8008a5e708 */
	int c52;	/* second word of the same `ldp` */
	u8 c56[8];	/* declared padding */
	int c64;	/* "b9404129 ldr"@0xffffff8008a5e72c */
};

struct ilitek_c56 {
	int c0;		/* "b940014b ldr"@0xffffff8008a5e714 */
	u8 c4[32];	/* declared padding */
	int c36;	/* "b9402549 ldr"@0xffffff8008a5e71c */
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0ff458).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_mp_run {
	int num;
	int idx[50];
};
struct ilitek_mp_run mp_run;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0fccf0, 5000 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
char seq_item[100][100];

/*
 * 0xffffff800a0fcc70, right AFTER `core_mp` (which ends at
 * 0xffffff800a0fcc70 because it is 368 bytes long) and right BEFORE
 * `seq_item`: the width of 128 is therefore MEASURED from the two bounds.
 * "9131c063 add"@0xffffff8008a607a8 passes it to `snprintf` as the `%s`
 * argument and "9131c000 add"@0xffffff8008a61d10 as the destination.
 */
char csv_path[128];

/*
 * 0xffffff800a0ff524. The width of 128 is MEASURED from the second argument
 * of the `snprintf` that fills it: "321903e1 orr"@0xffffff8008a61cfc.
 */
char csv_time[128];

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0ff420).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int *mp_frame_buf0;
int *mp_frame_buf1;
int *frame1_cbk700;
int *frame1_cbk250;
int *frame1_cbk200;
int *cap_dac;
int *cap_raw;

/*
 * parser_get_int_data() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int parser_get_int_data(char *section, char *keyname, char *rv, int len)
{
	int ret = 0;
	int size = 0;
	/*
	 * "321703e2 orr"@0xffffff8008a5f298 -- 512 bytes cleared on entry,
	 * BEFORE the pointer check: it is a declaration initialiser,
	 * not a hand-written `memset`
	 * ("940fe358 bl"@0xffffff8008a5f2a0 precedes
	 * "b4000193 cbz"@0xffffff8008a5f2a4)
	 */
	char value[512] = {0};

	if (rv == NULL || section == NULL || keyname == NULL) {
		/* "\x013ILITEK: (%s, %d): Parameters are invalid\n"@0xffffff80092437ca,
		 * __func__ = "parser_get_int_data"@0xffffff80092437f6;
		 * "528046c2 mov"@0xffffff8008a5f270 -- __LINE__ = 566 */
#line 566
		ILI_ERR("Parameters are invalid\n");
		return -EINVAL;
	}

	/* "94000b4f bl"@0xffffff8008a5f24c to <parser_get_ini_key_value> */
	ret = parser_get_ini_key_value(section, keyname, value);

	/*
	 * "94102cd8 bl"@0xffffff8008a5f260 towards <snprintf>, with
	 * x2 = "%s"@0xffffff8009100e35 ("9138d442 add"@0xffffff8008a5ed50)
	 */
	size = snprintf(rv, len, "%s", value);

	if (ret < 0)
		return 0;

	return size;
}

/*
 * ===========================================================================
 * parser_ini_str_trim_r -- line 625, inlined into ilitek_tddi_mp_test_main
 * ===========================================================================
 * It strips the leading spaces from the line. The threshold of 512 is measured
 * ("f10802bf cmp"@0xffffff8008a60268 + "540000a2 b.cs"@0xffffff8008a6026c):
 * below the threshold it works in the stack array, above it allocates.
 *
 * THE RETURN VALUE ON THE ERROR PATH IS THE EMPTY LITERAL, and that is
 * measured: "90003480 adrp"@0xffffff8008a602e4 + "91069c00 add"@0xffffff8008a602e8
 * put the address 0xffffff80090f01a7 into x0, which `leggi_stringa.py` reads
 * as `b''` (len=0), and the jump "1400000e b"@0xffffff8008a602ec goes
 * straight to the caller's `strlen`. It is NOT NULL: the caller would be
 * calling `strlen(NULL)`.
 */
static char *parser_ini_str_trim_r(char *buf)
{
	int i, len;
	char tmp[512] = {0};
	char *p = tmp;
	char *alloc = NULL;

	/* "940fe029 bl"@0xffffff8008a6025c to <__pi_strlen> */
	len = strlen(buf);

	if (len >= 512) {
		/* "97dfc03f bl"@0xffffff8008a6028c to <__kmalloc>, flags
		 * 0x14080c0 = GFP_KERNEL | __GFP_ZERO
		 * ("52901801 mov"@0xffffff8008a60280 +
		 * "72a02801 movk"@0xffffff8008a60284) */
		alloc = kzalloc(len, GFP_KERNEL);
		p = alloc;
		if (IS_ERR(p) || p == NULL) {
			/* "\x013ILITEK: (%s, %d): Failed to allocate tmp buf\n"
			 * @0xffffff8009244de4,
			 * __func__ = "parser_ini_str_trim_r"@0xffffff8009244e14;
			 * "52804e22 mov"@0xffffff8008a602d4 -- __LINE__ = 625 */
#line 625
			ILI_ERR("Failed to allocate tmp buf\n");
			/* ""@0xffffff80090f01a7 */
			return "";
		}
	}

	/* "38686b49 ldrb"@0xffffff8008a602b0 + "7100813f cmp"@0xffffff8008a602b4
	 * (0x20 = the space) */
	for (i = 0; i < len; i++) {
		if (buf[i] != ' ')
			break;
	}

	if (i < len) {
		/*
		 * "9410207b bl"@0xffffff8008a60300 towards <strncpy>, with
		 * x2 = sxtw(len - i) ("4b080288 sub"@0xffffff8008a602f4 +
		 * "93407d02 sxtw"@0xffffff8008a602f8)
		 */
		strncpy(p, buf + i, len - i);
	}

	/* "94102077 bl"@0xffffff8008a60310, x2 = x21 = sxtw(len) */
	strncpy(buf, p, len);

	/* "b400009c cbz"@0xffffff8008a60314 + "97dfc125 bl"@0xffffff8008a6031c */
	if (alloc)
		kfree(alloc);

	return buf;
}

/*
 * parser_get_ini_phy_data() was reconstructed from the factory kernel disassembly (0xffffff8008a601ec).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int parser_get_ini_phy_data(char *data, int size)
{
	int i, n = 0, offset = 0, seq = 0, ret = 0, len = 0;
	char *ini_buf = NULL, *tmp_sec = NULL;

	/* "97dfbad7 bl"@0xffffff8008a60150 to <kmem_cache_alloc_trace>,
	 * flags 0x14080c0 = GFP_KERNEL | __GFP_ZERO
	 * ("52901801 mov"@0xffffff8008a60144 +
	 * "72a02801 movk"@0xffffff8008a60148), size 1537
	 * ("5280c022 mov"@0xffffff8008a6014c) */
	ini_buf = kzalloc(PARSER_MAX_CFG_BUF + 1, GFP_KERNEL);
	if (IS_ERR(ini_buf) || ini_buf == NULL) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to allocate ini_buf memory, %ld\n"
		 * @0xffffff8009244be9,
		 * __func__ = "parser_get_ini_phy_data"@0xffffff8009244bd1;
		 * "528052e2 mov"@0xffffff8008a6067c -- __LINE__ = 663.
		 * The call spans TWO lines and `__LINE__` is the line of the
		 * CLOSING parenthesis: the directive therefore carries 662
		 */
#line 662
		ILI_ERR("Failed to allocate ini_buf memory, %ld\n",
			PTR_ERR(ini_buf));
		ret = -ENOMEM;
		goto out;
	}

	/* "97dfbace bl"@0xffffff8008a60174, same size */
	tmp_sec = kzalloc(PARSER_MAX_CFG_BUF + 1, GFP_KERNEL);
	if (IS_ERR(tmp_sec) || tmp_sec == NULL) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to allocate tmpSectionName memory, %ld\n"
		 * @0xffffff8009244c25;
		 * "528053c2 mov"@0xffffff8008a606d8 -- __LINE__ = 670; the
		 * call spans two lines, so the directive carries 669
		 */
#line 669
		ILI_ERR("Failed to allocate tmpSectionName memory, %ld\n",
			PTR_ERR(tmp_sec));
		ret = -ENOMEM;
		goto out;
	}

	/*
	 * "52827102 mov"@0xffffff8008a60190 -- 5000, that is HALF the vector.
	 * A FACTORY DEFECT, reproduced (rule 7)
	 */
	memset(seq_item, 0, 5000);

	while (1) {
		/* "711c211f cmp"@0xffffff8008a601a4 +
		 * "540024ec b.gt"@0xffffff8008a601a8 */
		if (ilitek_ini_num > PARSER_MAX_KEY_NUM) {
			/* "\x013ILITEK: (%s, %d): MAX_KEY_NUM: Out of length\n"
			 * @0xffffff8009244c68;
			 * "52805542 mov"@0xffffff8008a60654 -- __LINE__ = 682 */
#line 682
			ILI_ERR("MAX_KEY_NUM: Out of length\n");
			goto out;
		}

		/* "6b1b029f cmp"@0xffffff8008a601c0 +
		 * "540024ca b.ge"@0xffffff8008a601c4 */
		if (offset >= size)
			goto out;

		/*
		 * READING ONE LINE. The character is read from the text with
		 * base and index ("3868694b ldrb"@0xffffff8008a601d0); 0x0d and
		 * 0x0a are tested in this order
		 * ("7100357f cmp"@0xffffff8008a601d8 and
		 * "7100297f cmp"@0xffffff8008a601e0), and the NUL before them all
		 * ("340002ab cbz"@0xffffff8008a601d4).
		 */
		for (i = 0; i < PARSER_MAX_CFG_BUF; i++) {
			if (data[offset + i] == 0) {
				n = i + 1;
				break;
			}
			if (data[offset + i] == 0x0d || data[offset + i] == 0x0a) {
				/* "39400529 ldrb"@0xffffff8008a6020c -- the
				 * NEXT character */
				if (data[offset + i + 1] == 0x0a ||
				    data[offset + i + 1] == 0x0d)
					n = i + 2;
				else
					n = i + 1;
				break;
			}
			/* "38286b4b strb"@0xffffff8008a601f0 */
			ini_buf[i] = data[offset + i];
		}

		/* "38286b5f strb"@0xffffff8008a60234 e
		 * "38296b5f strb"@0xffffff8008a601fc */
		ini_buf[i] = 0;

		/*
		 * "36f801c9 tbz"@0xffffff8008a60200 tests BIT 31 of `n`,
		 * that is, the sign. The branch is UNREACHABLE (`n` is never
		 * negative) but the code is there and has to be reproduced (rule 7).
		 */
		if (n < 0) {
			/* "\x013ILITEK: (%s, %d): End of Line\n"@0xffffff8009244c98;
			 * "52805662 mov"@0xffffff8008a608c8 -- __LINE__ = 691 */
#line 691
			ILI_ERR("End of Line\n");
			goto out;
		}

		/* "0b140134 add"@0xffffff8008a60238 */
		offset += n;

		/*
		 * "940fdff7 bl"@0xffffff8008a60324 towards <__pi_strlen> on the
		 * value returned by `parser_ini_str_trim_r`
		 */
		len = strlen(parser_ini_str_trim_r(ini_buf));

		/* "34000095 cbz"@0xffffff8008a60330 e
		 * "71008d1f cmp"@0xffffff8008a60338 (0x23 = '#') */
		if (len == 0 || ini_buf[0] == '#')
			continue;

		/* "71000ebf cmp"@0xffffff8008a60354 (3),
		 * "71016d1f cmp"@0xffffff8008a6035c (0x5b = '['),
		 * "7101753f cmp"@0xffffff8008a60374 (0x5d = ']') */
		if (len >= 3 && ini_buf[0] == '[' && ini_buf[len - 1] != ']') {
			/* "\x013ILITEK: (%s, %d): Bad Section: %s\n"
			 * @0xffffff8009244cb9;
			 * "52805802 mov"@0xffffff8008a60b94 -- __LINE__ = 704 */
#line 704
			ILI_ERR("Bad Section: %s\n", ini_buf);
			ret = -EINVAL;
			goto out;
		}

		/* "71016d1f cmp"@0xffffff8008a6037c +
		 * "54000fe0 b.eq"@0xffffff8008a60380 */
		if (ini_buf[0] == '[') {
			/* "51000ab3 sub"@0xffffff8008a60588 --
			 * "b9089913 str"@0xffffff8008a60590 writes at +2200 */
			ilitek_ini_data[ilitek_ini_num].c2200 = len - 2;
			if (ilitek_ini_data[ilitek_ini_num].c2200 >
			    PARSER_MAX_KEY_NAME_LEN) {
				/* "\x013ILITEK: (%s, %d): MAX_KEY_NAME_LEN: Out Of Length\n"
				 * @0xffffff8009244cde ("91337800 add"@0xffffff8008a61f50);
				 * "528058e2 mov"@0xffffff8008a61f58 -- __LINE__ = 711 */
#line 711
				ILI_ERR("MAX_KEY_NAME_LEN: Out Of Length\n");
				ret = -1;
				goto out;
			}
			/* "f10196ff cmp"@0xffffff8008a605b0 (0x65 = 101) +
			 * "5400cd62 b.cs"@0xffffff8008a605b4 */
			if (seq >= PARSER_MAX_SECTION_NUM + 1) {
				/*
				 * "\x013ILITEK: (%s, %d): seq_item is over than its define (%d), abort\n"
				 * @0xffffff8009244d13;
				 * "528059a2 mov"@0xffffff8008a61f70 --
				 * __LINE__ = 717; the call spans two lines,
				 * so the directive carries 716
				 */
#line 716
				ILI_ERR("seq_item is over than its define (%d), abort\n",
					seq);
				ret = -1;
				goto out;
			}
			/*
			 * "38286b5f strb"@0xffffff8008a605d0 -- the trailing ']'
			 * replaced by the NUL
			 */
			ini_buf[len - 1] = 0;
			/* "94101fc6 bl"@0xffffff8008a605d4, x2 = 1537 */
			strncpy(tmp_sec, ini_buf + 1, PARSER_MAX_CFG_BUF + 1);
			/* "94101fbe bl"@0xffffff8008a605f4, x2 = 100 */
			strncpy(seq_item[seq], tmp_sec, PARSER_MAX_KEY_NAME_LEN);
			/*
			 * "910006f7 add"@0xffffff8008a60604 -- the increment comes
			 * BEFORE the message, and the message recomputes the
			 * address with the NEW value
			 * ("9b1522e3 madd"@0xffffff8008a6061c): it prints the
			 * FOLLOWING entry, which is empty. A FACTORY DEFECT,
			 * reproduced (rule 7)
			 */
			seq++;
			/* "\x016ILITEK: (%s, %d): Section Name: %s, Len: %d, offset = %d\n"
			 * @0xffffff8009244d55;
			 * "52805b02 mov"@0xffffff8008a60620 -- __LINE__ = 728 */
#line 728
			ILI_DBG("Section Name: %s, Len: %d, offset = %d\n", seq_item[seq], len - 2, offset);
			continue;
		}

		/* "94102053 bl"@0xffffff8008a603a0 to <strncpy>, x2 = 100 */
		strncpy(ilitek_ini_data[ilitek_ini_num].c0, tmp_sec,
			PARSER_MAX_KEY_NAME_LEN);
		/* "940fdfd6 bl"@0xffffff8008a603a8 +
		 * "b9089900 str"@0xffffff8008a603bc */
		ilitek_ini_data[ilitek_ini_num].c2200 = strlen(tmp_sec);

		/*
		 * THE SEARCH FOR THE SEPARATOR. "7100f53f cmp"@0xffffff8008a603d0
		 * (0x3d = '='), "71016d3f cmp"@0xffffff8008a603d8 ('[') and
		 * "7101753f cmp"@0xffffff8008a603e0 (']') inside the same
		 * loop: if a bracket turns up the line is thrown away.
		 */
		n = -1;
		for (i = 0; i < len; i++) {
			if (ini_buf[i] == '=') {
				n = i;
				break;
			}
			if (ini_buf[i] == '[' || ini_buf[i] == ']')
				break;
		}

		if (n > 0) {
			/* "b9089d13 str"@0xffffff8008a60548 -- at +2204 */
			ilitek_ini_data[ilitek_ini_num].c2204 = n;
			if (ilitek_ini_data[ilitek_ini_num].c2204 >
			    PARSER_MAX_KEY_NAME_LEN) {
				/* "\x013ILITEK: (%s, %d): MAX_KEY_NAME_LEN: Out Of Length\n"
				 * @0xffffff8009244cde ("91337800 add"@0xffffff8008a61f38);
				 * "52806102 mov"@0xffffff8008a61f40 -- __LINE__ = 776 */
#line 776
				ILI_ERR("MAX_KEY_NAME_LEN: Out Of Length\n");
				ret = -1;
				goto out;
			}
			/* "940fdde4 bl"@0xffffff8008a60570 to <__memcpy>,
			 * destination element + 0x64 = 100
			 * ("91019100 add"@0xffffff8008a60568) */
			memcpy(ilitek_ini_data[ilitek_ini_num].c100, ini_buf,
			       ilitek_ini_data[ilitek_ini_num].c2204);
			/* "4b130295 sub"@0xffffff8008a60574 */
			len = len - 1 - n;
		} else {
			/*
			 * "941021d5 bl"@0xffffff8008a60414 towards <strstr> with
			 * "benchmark_data"@0xffffff8009243e2e
			 */
			if (strstr(&ilitek_ini_data[ilitek_ini_num].c0[0],
				   "benchmark_data") != NULL) {
				/* "321f0be9 orr"@0xffffff8008a60424 -- 14 */
				ilitek_ini_data[ilitek_ini_num].c2204 = 14;
				strncpy(ilitek_ini_data[ilitek_ini_num].c100,
					"benchmark_data",
					PARSER_MAX_KEY_NAME_LEN);
			/* "941021c1 bl"@0xffffff8008a60464 con
			 * "node type"@0xffffff80092430e3 */
			} else if (strstr(&ilitek_ini_data[ilitek_ini_num].c0[0],
					  "node type") != NULL) {
				/* "52800129 mov"@0xffffff8008a60474 -- 9 */
				ilitek_ini_data[ilitek_ini_num].c2204 = 9;
				strncpy(ilitek_ini_data[ilitek_ini_num].c100,
					"node type", PARSER_MAX_KEY_NAME_LEN);
			} else {
				continue;
			}
		}

		/* "b908a115 str"@0xffffff8008a604b0 -- at +2208 */
		ilitek_ini_data[ilitek_ini_num].c2208 = len;
		if (ilitek_ini_data[ilitek_ini_num].c2208 >
		    PARSER_MAX_KEY_VALUE_LEN) {
			/* "52806262 mov"@0xffffff8008a60b74 -- __LINE__ = 787 */
#line 787
			ILI_ERR("MAX_KEY_VALUE_LEN: Out Of Length\n");
			ret = -1;
			goto out;
		}

		/* "940fde09 bl"@0xffffff8008a604dc to <__memcpy>,
		 * destination element + 0xc8 = 200
		 * ("91032100 add"@0xffffff8008a604d0) */
		memcpy(ilitek_ini_data[ilitek_ini_num].c200, ini_buf + n + 1,
		       ilitek_ini_data[ilitek_ini_num].c2208);

		/* "\x016ILITEK: (%s, %d): %s = %s\n"@0xffffff8009244dc7;
		 * "52806382 mov"@0xffffff8008a604fc -- __LINE__ = 796 */
#line 796
		ILI_DBG("%s = %s\n", ilitek_ini_data[ilitek_ini_num].c100, ilitek_ini_data[ilitek_ini_num].c200);

		/* "11000508 add"@0xffffff8008a60520 */
		ilitek_ini_num++;
	}

out:
	/* "b500043a cbnz"@0xffffff8008a60664 +
	 * "97dfc031 bl"@0xffffff8008a606ec to <kfree> */
	if (ini_buf)
		kfree(ini_buf);
	/* "b4000073 cbz"@0xffffff8008a606f0 +
	 * "97dfc02e bl"@0xffffff8008a606f8 */
	if (tmp_sec)
		kfree(tmp_sec);
	return ret;
}

/*
 * ilitek_tddi_mp_ini_parser() was reconstructed from the factory kernel disassembly (0xffffff8008a5eb10).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ilitek_tddi_mp_ini_parser(void)
{
	int ret = 0, fsize = 0, n = 0;
	char *tmp = NULL;
	struct file *f = NULL;
	const struct firmware *fw = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	/* "\x016ILITEK: (%s, %d): ini file path = %s\n"@0xffffff80092449fa,
	 * __func__ = "ilitek_tddi_mp_ini_parser"@0xffffff8009244a22;
	 * "52806602 mov"@0xffffff8008a5ea68 -- __LINE__ = 816.
	 * "f9415513 ldr"@0xffffff8008a5ea60 reads `idev->c680` */
#line 816
	ILI_INFO("ini file path = %s\n", (char *)idev->c680);

	/* "97e01bf5 bl"@0xffffff8008a5ea88 to <filp_open>, w1 = 0
	 * ("2a1f03e1 mov"@0xffffff8008a5ea84) e w2 = 644
	 * ("52805082 mov"@0xffffff8008a5ea7c) */
	f = filp_open((char *)idev->c680, O_RDONLY, 644);
	if (IS_ERR(f) || f == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to open ini file at %ld, trying to request\n"
		 * @0xffffff8009244a3c;
		 * "52806682 mov"@0xffffff8008a5eac0 -- __LINE__ = 820 */
#line 820
		ILI_ERR("Failed to open ini file at %ld, trying to request\n", PTR_ERR(f));

		/* "\x016ILITEK: (%s, %d): request path = %s\n"@0xffffff8009244a83;
		 * "528066e2 mov"@0xffffff8008a5eadc -- __LINE__ = 823;
		 * "f9415914 ldr"@0xffffff8008a5eae0 reads `idev->c688` */
#line 823
		ILI_INFO("request path = %s\n", (char *)idev->c688);

		/* "97eaa39f bl"@0xffffff8008a5eb00 to <request_firmware>,
		 * x2 = `idev->c24` ("f9400d02 ldr"@0xffffff8008a5eafc) */
		if (request_firmware(&fw, (char *)idev->c688,
				     (struct device *)idev->c24) < 0) {
			/* "\x013ILITEK: (%s, %d): Request ini file failed\n"
			 * @0xffffff8009244aaa;
			 * "52806722 mov"@0xffffff8008a5efbc -- __LINE__ = 825 */
#line 825
			ILI_ERR("Request ini file failed\n");
			return -1;
		}
		/* "f9414be8 ldr"@0xffffff8008a5eb0c + "f940011b ldr"@0xffffff8008a5eb14 */
		fsize = fw->size;
		f = NULL;
	} else {
		/* "f9400f08 ldr"@0xffffff8008a5ea9c (f_path.dentry, +24),
		 * "f9401908 ldr"@0xffffff8008a5eaa4 (d_inode, +48),
		 * "91014108 add"@0xffffff8008a5eaa8 (+0x50 = i_size) */
		fsize = f->f_path.dentry->d_inode->i_size;
	}

	/* "\x016ILITEK: (%s, %d): ini file size = %d\n"@0xffffff8009244ad7;
	 * "52806922 mov"@0xffffff8008a5eb28 -- __LINE__ = 841 */
#line 841
	ILI_INFO("ini file size = %d\n", fsize);

	/* "7100037f cmp"@0xffffff8008a5eb34 + "540004ad b.le"@0xffffff8008a5eb38 */
	if (fsize <= 0) {
		/* "\x013ILITEK: (%s, %d): The size of file is invaild\n"
		 * @0xffffff8009244aff;
		 * "52806962 mov"@0xffffff8008a5ebdc -- __LINE__ = 843 */
#line 843
		ILI_ERR("The size of file is invaild\n");
		ret = -EINVAL;
		goto out;
	}

	/*
	 * "97df8f48 bl"@0xffffff8008a5eb50 towards <vmalloc>, x0 = fsize + 1
	 * ("8b080268 add"@0xffffff8008a5eb48 with x8 = 0x100000000 and
	 * "9360fd00 asr"@0xffffff8008a5eb4c, that is 32-bit signed arithmetic)
	 */
	tmp = vmalloc(fsize + 1);
	if (IS_ERR(tmp) || tmp == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate tmp memory, %ld\n"
		 * @0xffffff8009244b30;
		 * "52806a42 mov"@0xffffff8008a5ebfc -- __LINE__ = 850 */
#line 850
		ILI_ERR("Failed to allocate tmp memory, %ld\n", PTR_ERR(tmp));
		ret = -ENOMEM;
		goto out;
	}

	/*
	 * "d5384114 mrs"@0xffffff8008a5eb68 (sp_el0),
	 * "f9400697 ldr"@0xffffff8008a5eb6c (addr_limit at +8),
	 * "940fe0da bl"@0xffffff8008a5eb84 towards <set_bit> with w0 = 5
	 */
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	/* "97e02048 bl"@0xffffff8008a5eba0 to <vfs_read> */
	vfs_read(f, tmp, fsize, &pos);
	set_fs(old_fs);

	/* "38336adf strb"@0xffffff8008a600cc */
	tmp[fsize] = 0;

	/* "b904039f str"@0xffffff8008a600dc -- 0xffffff800a0ff400 zeroed */
	ilitek_ini_num = 0;

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a600d4, 2208 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	for (n = 0; n < PARSER_MAX_KEY_NUM; n++) {
		memset(ilitek_ini_data[n].c0, 0, PARSER_MAX_KEY_NAME_LEN);
		memset(ilitek_ini_data[n].c100, 0, PARSER_MAX_KEY_NAME_LEN);
		memset(ilitek_ini_data[n].c200, 0, PARSER_MAX_KEY_VALUE_LEN);
		ilitek_ini_data[n].c2200 = 0;
		ilitek_ini_data[n].c2204 = 0;
		ilitek_ini_data[n].c2208 = 0;
	}

	/*
	 * "7100077f cmp"@0xffffff8008a60108 plus the loop of
	 * "386a690b ldrb"@0xffffff8008a60120 over `_ctype` at
	 * 0xffffff8008fc9748: `tst w11,#1` is `isupper`, `+0x20` is the
	 * conversion. It is the kernel's `tolower`, not a driver table
	 */
	for (n = 0; n < fsize; n++)
		tmp[n] = tolower(tmp[n]);

	ret = parser_get_ini_phy_data(tmp, fsize);
	if (ret < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to get physical ini data, ret = %d\n"
		 * @0xffffff8009244b68;
		 * "52806de2 mov"@0xffffff8008a60734 -- __LINE__ = 879 */
#line 879
		ILI_ERR("Failed to get physical ini data, ret = %d\n", ret);
		goto out;
	}

	/* "\x016ILITEK: (%s, %d): Parsed ini file done\n"@0xffffff8009244ba7;
	 * "52806e62 mov"@0xffffff8008a60710 -- __LINE__ = 883 */
#line 883
	ILI_INFO("Parsed ini file done\n");
	ret = 0;

out:
	/* "b5ff27b6 cbnz"@0xffffff8008a6071c + "97df8d89 bl"@0xffffff8008a5ec14 */
	if (tmp)
		vfree(tmp);
	/* "97e01cd0 bl"@0xffffff8008a5ec20 to <filp_close>, x1 = NULL */
	filp_close(f, NULL);
	return ret;
}

/*
 * mp_get_timing_info() was reconstructed from the factory kernel disassembly (0xffffff800a0fc963).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mp_get_timing_info(void)
{
	int ret = 0, len = 0;
	char str[256] = {0};
	u8 timing_info_raw[16] = {0};

	/*
	 * "392f8e9f strb"@0xffffff8008a5ed28 -- cleared BEFORE the
	 * `parser_get_ini_key_value`
	 */
	ilitek_mp_ddi_mode = 0;

	/* "94000c91 bl"@0xffffff8008a5ed44, section
	 * "pv5_4 command"@0xffffff800924332d, key
	 * "timing_info_raw"@0xffffff8009244e86; the size of the output is
	 * 0x100 = 256 ("321803e1 orr"@0xffffff8008a5ed5c) */
	len = parser_get_int_data("pv5_4 command", "timing_info_raw", str, 256);
	/* "37f813c2 tbnz"@0xffffff8008a5ed6c -- the bit 31 of `len` */
	if (len < 0) {
		ret = -1;
		goto out;
	}

	/*
	 * parser_get_u8_array() was reconstructed from the factory kernel disassembly (0xffffff8008a5ed78).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	ret = parser_get_u8_array(str, timing_info_raw, len);
	/* "37f81340 tbnz"@0xffffff8008a5ed7c */
	if (ret < 0)
		goto out;

	/*
	 * "39495be8 ldrb"@0xffffff8008a5ed80 reads the SEVENTH byte of the
	 * array (sp+598 with base sp+592) and
	 * "1a9f07e8 cset"@0xffffff8008a5eda0 turns it into a 0/1
	 */
	ilitek_mp_ddi_mode = (timing_info_raw[6] != 0);

	/* "\x016ILITEK: (%s, %d): DDI Mode = %s\n"@0xffffff8009244e96,
	 * __func__ = "mp_get_timing_info"@0xffffff8009244eb9;
	 * "52814782 mov"@0xffffff8008a5edb0 -- __LINE__ = 2620.
	 * "9a891143 csel"@0xffffff8008a5eda4 chooses between
	 * "Long V"@0xffffff8009244ecc (condition `ne`) e
	 * "Long H"@0xffffff8009244ed3 */
#line 2620
	ILI_INFO("DDI Mode = %s\n", ilitek_mp_ddi_mode ? "Long V" : "Long H");
out:
	return ret;
}

/*
 * ilitek_tddi_mp_init_item() was reconstructed from the factory kernel disassembly (0xffffff8008f7ec40).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ilitek_tddi_mp_init_item(void)
{
	int i;

	/* "52802e02 mov"@0xffffff8008a5e6a8 -- 368 = sizeof(core_mp) */
	memset(&core_mp, 0, sizeof(core_mp));

	/* "f9401909 ldr"@0xffffff8008a5e6d4 e "f9401d0a ldr"@0xffffff8008a5e710 */
	core_mp.c0 = ((struct ilitek_c48 *)idev->c48)->c8;
	core_mp.c4 = ((struct ilitek_c48 *)idev->c48)->c2;
	core_mp.c6 = ((struct ilitek_c48 *)idev->c48)->c0;
	core_mp.c7 = ((struct ilitek_c48 *)idev->c48)->c1;
	core_mp.c8 = ((struct ilitek_c48 *)idev->c48)->c48;
	core_mp.c12 = ((struct ilitek_c56 *)idev->c56)->c0;
	core_mp.c16 = ((struct ilitek_c48 *)idev->c48)->c52;
	core_mp.c228 = ((struct ilitek_c56 *)idev->c56)->c36;
	core_mp.c212 = ((struct ilitek_c48 *)idev->c48)->c64;
	/* "291d22e9 stp"@0xffffff8008a5e748 -- the two bytes of `idev` at +248 e
	 * +249 end up in two adjacent `int`s */
	core_mp.c232 = idev->c248;
	core_mp.c236 = idev->c249;
	/* "1b097d08 mul"@0xffffff8008a5e74c */
	core_mp.c256 = core_mp.c232 * core_mp.c236;
	/* "b9010af4 str"@0xffffff8008a5e750 -- w20 = -1 */
	core_mp.c264 = -1;
	/* "f900b2ea str"@0xffffff8008a5e740 -- eight byte, 352 = 240 e 356 = 0 */
	core_mp.c352 = 240;
	core_mp.c356 = 0;
	/*
	 * FOUR CLEARS THE `memset` HAS ALREADY DONE, which are nonetheless
	 * THERE in the binary: "a90f7eff stp"@0xffffff8008a5e73c (sixteen bytes
	 * at 240), "390362ff strb"@0xffffff8008a5e744 (216) and
	 * "3905a6ff strb"@0xffffff8008a5e758 (361). They are not compiler
	 * redundancy: `__memset` is a CALL, hence opaque, and the compiler
	 * cannot know those bytes are already zero. They are therefore
	 * assignments in the source, and have to be reproduced.
	 */
	core_mp.c240 = 0;
	core_mp.c244 = 0;
	core_mp.key_len = 0;
	core_mp.c252 = 0;
	core_mp.c216 = 0;
	core_mp.c361 = 0;

	/* "\x016ILITEK: (%s, %d): ============== TP & Panel info ================\n"
	 * @0xffffff800924380a, __func__ =
	 * "ilitek_tddi_mp_init_item"@0xffffff800924384f;
	 * "52818522 mov"@0xffffff8008a5e6e0 -- __LINE__ = 3113 */
#line 3113
	ILI_INFO("============== TP & Panel info ================\n");
	/* "\x016ILITEK: (%s, %d): Driver version = %s\n"@0xffffff8009243868,
	 * x3 = "2.0.6.0.191122"@0xffffff800923ed58 */
#line 3114
	ILI_INFO("Driver version = %s\n", DRIVER_VERSION);
	/* "\x016ILITEK: (%s, %d): TP Module = %s\n"@0xffffff8009243891,
	 * "f9414903 ldr"@0xffffff8008a5e78c reads `idev->c656` */
#line 3115
	ILI_INFO("TP Module = %s\n", (char *)idev->c656);
	/* "\x016ILITEK: (%s, %d): CHIP = 0x%x\n"@0xffffff80092438b5 */
#line 3116
	ILI_INFO("CHIP = 0x%x\n", core_mp.c0);
	/* "\x016ILITEK: (%s, %d): Firmware version = %x\n"@0xffffff80092438d6 */
#line 3117
	ILI_INFO("Firmware version = %x\n", core_mp.c8);
	/* "\x016ILITEK: (%s, %d): Protocol version = %x\n"@0xffffff8009243901 */
#line 3118
	ILI_INFO("Protocol version = %x\n", core_mp.c12);
	/* "\x016ILITEK: (%s, %d): Core version = %x\n"@0xffffff800924392c */
#line 3119
	ILI_INFO("Core version = %x\n", core_mp.c16);
	/* "\x016ILITEK: (%s, %d): Read CDC Length = %d\n"@0xffffff8009243953 */
#line 3120
	ILI_INFO("Read CDC Length = %d\n", core_mp.c228);
	/*
	 * "\x016ILITEK: (%s, %d): X length = %d, Y length = %d\n"
	 * @0xffffff800924397d -- "295d12e3 ldp"@0xffffff8008a5e810 reads the two
	 * fields TOGETHER, and that proves they are adjacent
	 */
#line 3121
	ILI_INFO("X length = %d, Y length = %d\n", core_mp.c232, core_mp.c236);
	/* "\x016ILITEK: (%s, %d): Frame length = %d\n"@0xffffff80092439af */
#line 3122
	ILI_INFO("Frame length = %d\n", core_mp.c256);
	/* "\x016ILITEK: (%s, %d): Check busy method = %s\n"@0xffffff80092439d6 */
#line 3123
	ILI_INFO("Check busy method = %s\n", core_mp.c356 ? "Polling" : "Interrupt");
	/* "\x016ILITEK: (%s, %d): ===============================================\n"
	 * @0xffffff8009243a02 */
#line 3124
	ILI_INFO("===============================================\n");

	/* "5280065c mov"@0xffffff8008a5e8a0 -- 50 turns */
	for (i = 0; i < MP_TEST_ITEM; i++) {
		/* "7818537f sturh"@0xffffff8008a5e954 -- DUE byte, 0x15 e 0x16 */
		mp_items[i].c21 = 0;
		mp_items[i].c22 = 0;
		/* "3818737f sturb"@0xffffff8008a5e958 */
		mp_items[i].c23 = 0;
		/*
		 * "f818c377 stur"@0xffffff8008a5e95c -- eight bytes at 0x1c:
		 * the low half is 0 and the high one -1
		 */
		mp_items[i].c28 = 0;
		mp_items[i].c32 = -1;
		/* "f819437f stur"@0xffffff8008a5e964 -- eight byte a 0x24 */
		mp_items[i].c36 = 0;
		mp_items[i].c40 = 0;
		/* "b819c374 stur"@0xffffff8008a5e968 */
		mp_items[i].c44 = -1;
		/* "a93a7f7f stp"@0xffffff8008a5e974 and the others: 0x30..0x56 */
		mp_items[i].c48 = 0;
		mp_items[i].c52 = 0;
		mp_items[i].c56 = 0;
		mp_items[i].c60 = 0;
		mp_items[i].c64 = 0;
		mp_items[i].c68 = 0;
		mp_items[i].c72 = 0;
		mp_items[i].c76 = 0;
		mp_items[i].c80 = 0;
		mp_items[i].delay_time = 0;
		mp_items[i].test_int_pin = 0;
		mp_items[i].int_pulse_test = 0;
		/* "f81c837f stur"@0xffffff8008a5e984 and the three `stp` a 0x60,
		 * 0x70, 0x80 */
		mp_items[i].result_buf = NULL;
		mp_items[i].buf = NULL;
		mp_items[i].max_buf = NULL;
		mp_items[i].min_buf = NULL;
		mp_items[i].bench_mark_max = NULL;
		mp_items[i].bench_mark_min = NULL;
		mp_items[i].c136 = NULL;

		/*
		 * This section was reconstructed from the factory kernel disassembly.
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		switch (mp_items[i].c16) {
		case 0:
		case 4:
		case 5:
		case 6:
		case 8:
		case 9:
			/* entry 0x00 -> "f9000373 str"@0xffffff8008a5e99c */
			mp_items[i].c144 = mutual_test;
			break;
		case 1:
			/* entry 0x02 -> "d0000028 adrp"@0xffffff8008a5e9a4 +
			 * "913a5108 add"@0xffffff8008a5e9a8 =
			 * 0xffffff8008a64e94 */
			mp_items[i].c144 = self_test;
			break;
		case 2:
			/* entry 0x05 -> 0xffffff8008a6493c */
			mp_items[i].c144 = key_test;
			break;
		case 3:
			/* entry 0x08 -> 0xffffff8008a64ec0 */
			mp_items[i].c144 = st_test;
			break;
		case 7:
			/*
			 * entry 0x0b. The item name decides between three
			 * functions: "f1002c1f cmp"@0xffffff8008a5e9d8 (11) and
			 * "f100641f cmp"@0xffffff8008a5e9e0 (25) compare the
			 * length FIRST, and only if that matches is `strncmp`
			 * called -- that is the shape the binary carries, not
			 * a `strcmp`.
			 */
			if (strlen(mp_items[i].c0) == strlen("open test_c") &&
			    strncmp(mp_items[i].c0, "open test_c",
				    strlen("open test_c")) == 0)
				mp_items[i].c144 = open_test_cap;
			else if (strlen(mp_items[i].c0) ==
					 strlen("open test(integration)_sp") &&
				 strncmp(mp_items[i].c0,
					 "open test(integration)_sp",
					 strlen("open test(integration)_sp")) == 0)
				mp_items[i].c144 = open_test_sp;
			else
				mp_items[i].c144 = mutual_test;
			break;
		case 10:
			/* entry 0x1e -> 0xffffff8008a64eec */
			mp_items[i].c144 = pin_test;
			break;
		}

		/*
		 * "97dfc0a0 bl"@0xffffff8008a5ea2c towards
		 * <kmem_cache_alloc_trace> with 16 bytes
		 * ("321c03e2 orr"@0xffffff8008a5ea24) and flags 0x14000c0 =
		 * GFP_KERNEL WITHOUT __GFP_ZERO
		 * ("51402275 sub"@0xffffff8008a5e888 subtracts 0x8000 from the
		 * constant 0x14080c0 that is needed elsewhere)
		 */
		mp_items[i].c8 = kmalloc(16, GFP_KERNEL);
		/*
		 * "b9000019 str"@0xffffff8008a5ea40 (w25 = 0x4c494146 = "FAIL")
		 * plus "3900101f strb"@0xffffff8008a5ea44 (the NUL): five
		 * bytes, that is a copy of "FAIL" INCLUDING the terminator
		 */
		memcpy(mp_items[i].c8, "FAIL", sizeof("FAIL"));
	}
}

/*
 * ===========================================================================
 * mp_sort_item -- line 3363
 * ===========================================================================
 * It walks the sections read from the .ini file in the order they appear and,
 * for each, looks for the test item with the same name; if it finds one and
 * the "enable" key is non-zero, and if the item's `c24` field matches the
 * mode (screen on or off), it appends it to the list to run.
 *
 * THE COMPARISON IS IN TWO STEPS, and the binary carries them: first the two
 * lengths ("940fe4fe bl"@0xffffff8008a5ef08 and "940fe4fb bl"@0xffffff8008a5ef14,
 * "eb00029f cmp"@0xffffff8008a5ef18), then the `strncmp`
 * ("940fe516 bl"@0xffffff8008a5ef2c).
 */
static int mp_sort_item(bool lcm_on)
{
	int i, j, len = 0;
	char str[128] = {0};

	/*
	 * "f802e11f stur"@0xffffff8008a5ee28 and the three `stp`s that FOLLOW it
	 * ("a9027d1f stp"@0xffffff8008a5ee2c, "a9017d1f stp"@0xffffff8008a5ee30,
	 * "a9007d1f stp"@0xffffff8008a5ee34 -- the first draft said "that precede
	 * it", and the disassembly disproves it):
	 * 54 bytes cleared starting at 0xffffff800a0ff458.
	 * 54 = 4 + 50 = `sizeof(mp_run.num)` plus the NUMBER OF ELEMENTS where
	 * the number of BYTES was needed (50 * 4 = 200). It is a factory defect
	 * and is reproduced (rule 7): of the indices only the first twelve and a
	 * half stay cleared
	 */
	memset(&mp_run, 0, sizeof(mp_run.num) + MP_TEST_ITEM);

	/* "f10192ff cmp"@0xffffff8008a5ef48 (#0x64) */
	for (i = 0; i < PARSER_MAX_SECTION_NUM; i++) {
		/* "f100cb9f cmp"@0xffffff8008a5ef38 (#0x32) */
		for (j = 0; j < MP_TEST_ITEM; j++) {
			if (strlen(seq_item[i]) != strlen(mp_items[j].c0))
				continue;
			if (strncmp(seq_item[i], mp_items[j].c0,
				    strlen(seq_item[i])) != 0)
				continue;

			/* "94000c44 bl"@0xffffff8008a5ee78, key
			 * "enable"@0xffffff800924755d, output of 128 bytes
			 * ("321903e1 orr"@0xffffff8008a5ee84) */
			len = parser_get_int_data(mp_items[j].c0, "enable",
						  str, 128);
			/*
			 * "940031c3 bl"@0xffffff8008a5eeac towards <katoi>;
			 * "1a9f07e8 cset"@0xffffff8008a5eeb4 with `ne` and
			 * "381ff2a8 sturb"@0xffffff8008a5eeb8
			 */
			mp_items[j].c23 = (katoi(str) != 0);
			/* "340003c0 cbz"@0xffffff8008a5eebc */
			if (!mp_items[j].c23)
				continue;
			/*
			 * "394002a8 ldrb"@0xffffff8008a5eec0 +
			 * "6b1b011f cmp"@0xffffff8008a5eec4 -- `c24` against the
			 * mode, read as ONE byte
			 */
			if (mp_items[j].c24 != lcm_on)
				continue;
			/* "7100c87f cmp"@0xffffff8008a5eed4 +
			 * "5400be0c b.gt"@0xffffff8008a5eed8 */
			if (mp_run.num > MP_TEST_ITEM) {
				/* "\x013ILITEK: (%s, %d): Test item(%d) is invaild, abort\n"
				 * @0xffffff8009244eda,
				 * __func__ = "mp_sort_item"@0xffffff8009244f0f;
				 * "5281a462 mov"@0xffffff8008a606a8 --
				 * __LINE__ = 3363 */
#line 3363
				ILI_ERR("Test item(%d) is invaild, abort\n", mp_run.num);
				return -1;
			}
			/* "b900051c str"@0xffffff8008a5eeec e
			 * "11000508 add"@0xffffff8008a5eef4 */
			mp_run.idx[mp_run.num] = j;
			mp_run.num++;
		}
	}
	/*
	 * "b9445908 ldr"@0xffffff8008a5ef54 + "7100051f cmp"@0xffffff8008a5ef60
	 * + "5400878b b.lt"@0xffffff8008a5ef64 do NOT belong to this function:
	 * they are the `for` loop guard of the CALLER, which jumps straight to
	 * `mp_show_result` when there is nothing to run.
	 * An early draft of this batch put an `if (mp_run.num < 1) return 0;`
	 * here -- code that is not in the binary (class B6) -- and it was
	 * removed.
	 */
	return 0;
}

/*
 * parser_get_tdf() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int parser_get_tdf(char *str, int type)
{
	int i, j = 0, k = 0, dot = 0, len, ret;
	char tmp[10] = {0};

	/* "940fe26c bl"@0xffffff8008a5f950 to <__pi_strlen> */
	len = strlen(str);

	for (i = 0; i < len; i++) {
		if (str[i] == '.') {
			dot = 1;
			continue;
		}
		/*
		 * "382b4acc strb"@0xffffff8008a5f984 -- index extended UNSIGNED
		 * (`uxtw`), so `j` is not a signed `int` used as an index:
		 * it is the compiler knowing it is non-negative
		 */
		tmp[j++] = str[i];
		if (dot)
			k++;
	}

	/* "94002f06 bl"@0xffffff8008a5f9a0 to <katoi> */
	ret = katoi(tmp);

	if (type == 9) {
		if (k == 1)
			ret = ret * 10;
		else if (k == 0)
			ret = ret * 100;
	}

	return ret;
}

/*
 * mp_test_run() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void mp_test_run(int index)
{
	int ret = 0;
	char str[512] = {0};

	/* "94000ac9 bl"@0xffffff8008a5f464, key
	 * "spec option"@0xffffff8009244f1c; "38015f20 strb"@0xffffff8008a5f4a4
	 * writes ONE byte at +0x15 */
	parser_get_int_data(mp_items[index].c0, "spec option", str, 512);
	mp_items[index].c21 = katoi(str);

	/* key "type option"@0xffffff8009244f28;
	 * "39005900 strb"@0xffffff8008a5f510, ONE byte at +0x16 */
	parser_get_int_data(mp_items[index].c0, "type option", str, 512);
	mp_items[index].c22 = katoi(str);

	/* key "frame count"@0xffffff8009244f34;
	 * "b8030f00 str"@0xffffff8008a5f57c, FOUR bytes at +0x30 */
	parser_get_int_data(mp_items[index].c0, "frame count", str, 512);
	mp_items[index].c48 = katoi(str);

	/* key "trimmed mean"@0xffffff8009244f40;
	 * "b9003500 str"@0xffffff8008a5f5e8 */
	parser_get_int_data(mp_items[index].c0, "trimmed mean", str, 512);
	mp_items[index].c52 = katoi(str);

	/* key "lowest percentage"@0xffffff8009244f4d;
	 * "b9003900 str"@0xffffff8008a5f654 */
	parser_get_int_data(mp_items[index].c0, "lowest percentage", str, 512);
	mp_items[index].c56 = katoi(str);

	/* key "highest percentage"@0xffffff8009244f5f;
	 * "b9003d00 str"@0xffffff8008a5f6c0 */
	parser_get_int_data(mp_items[index].c0, "highest percentage", str, 512);
	mp_items[index].c60 = katoi(str);

	/* key "goldenmode"@0xffffff8009244f72;
	 * "b9005100 str"@0xffffff8008a5f72c */
	parser_get_int_data(mp_items[index].c0, "goldenmode", str, 512);
	mp_items[index].c80 = katoi(str);
	/* "340000e0 cbz"@0xffffff8008a5f730,
	 * "39400328 ldrb"@0xffffff8008a5f734 (`c21`, ONE byte) +
	 * "6b08001f cmp"@0xffffff8008a5f738 +
	 * "54000080 b.eq"@0xffffff8008a5f73c */
	if (mp_items[index].c80 != 0 &&
	    mp_items[index].c80 != mp_items[index].c21)
		core_mp.c361 = 1;

	/* "b8410f37 ldr"@0xffffff8008a5f750 (pre-index su +0x10) +
	 * "71002aff cmp"@0xffffff8008a5f754 (#0xa) */
	if (mp_items[index].c16 == 10) {
		/* key "test int pin"@0xffffff8009244f7d;
		 * "39015500 strb"@0xffffff8008a5f7cc, ONE byte at +0x55 */
		parser_get_int_data(mp_items[index].c0, "test int pin", str, 512);
		mp_items[index].test_int_pin = katoi(str);
		/* key "int pulse test"@0xffffff8009244f8a;
		 * "39015900 strb"@0xffffff8008a5f838, +0x56 */
		parser_get_int_data(mp_items[index].c0, "int pulse test", str, 512);
		mp_items[index].int_pulse_test = katoi(str);
		/* key "delay time"@0xffffff8009244f99;
		 * "39015100 strb"@0xffffff8008a5f8a4, +0x54 */
		parser_get_int_data(mp_items[index].c0, "delay time", str, 512);
		mp_items[index].delay_time = katoi(str);
	}

	/* "710026ff cmp"@0xffffff8008a5f8c0 (#0x9) +
	 * "540001a1 b.ne"@0xffffff8008a5f8c4 */
	if (mp_items[index].c16 == 9) {
		/* key "v_tdf_1"@0xffffff8009244fa4 */
		parser_get_int_data(mp_items[index].c0, "v_tdf_1", str, 512);
		mp_items[index].c64 = parser_get_tdf(str, mp_items[index].c16);
		/* key "v_tdf_2"@0xffffff8009244fac */
		parser_get_int_data(mp_items[index].c0, "v_tdf_2", str, 512);
		mp_items[index].c68 = parser_get_tdf(str, mp_items[index].c16);
		/* key "h_tdf_1"@0xffffff8009244fb4 */
		parser_get_int_data(mp_items[index].c0, "h_tdf_1", str, 512);
		mp_items[index].c72 = parser_get_tdf(str, mp_items[index].c16);
		/* key "h_tdf_2"@0xffffff8009244fbc */
		parser_get_int_data(mp_items[index].c0, "h_tdf_2", str, 512);
		mp_items[index].c76 = parser_get_tdf(str, mp_items[index].c16);
	} else {
		/* key "v_tdf"@0xffffff8009244fc4 */
		parser_get_int_data(mp_items[index].c0, "v_tdf", str, 512);
		mp_items[index].c64 = parser_get_tdf(str, mp_items[index].c16);
		/* key "h_tdf"@0xffffff8009244fca */
		parser_get_int_data(mp_items[index].c0, "h_tdf", str, 512);
		mp_items[index].c72 = parser_get_tdf(str, mp_items[index].c16);
	}

	/* "f1002c1f cmp"@0xffffff8008a5fe80 (11) e
	 * "940fe13b bl"@0xffffff8008a5fe98 con
	 * "tx/rx delta"@0xffffff80092435e3 */
	if (strlen(mp_items[index].c0) == strlen("tx/rx delta") &&
	    strncmp(mp_items[index].c0, "tx/rx delta",
		    strlen("tx/rx delta")) == 0) {
		/* key "tx max"@0xffffff8009244fd0;
		 * "b90c2100 str"@0xffffff8008a5f290 -- 0xffffff800a0fcc20 */
		parser_get_int_data(mp_items[index].c0, "tx max", str, 512);
		core_mp.c288 = katoi(str);
		/* key "tx min"@0xffffff8009244fd7 */
		parser_get_int_data(mp_items[index].c0, "tx min", str, 512);
		core_mp.c292 = katoi(str);
		/* key "rx max"@0xffffff8009244fde */
		parser_get_int_data(mp_items[index].c0, "rx max", str, 512);
		core_mp.c296 = katoi(str);
		/* key "rx min"@0xffffff8009244fe5 */
		parser_get_int_data(mp_items[index].c0, "rx min", str, 512);
		core_mp.c300 = katoi(str);

		/* "\x016ILITEK: (%s, %d): %s: Tx Max = %d, Tx Min = %d, Rx Max = %d,  Rx Min = %d\n"
		 * @0xffffff8009244fec, __func__ =
		 * "mp_test_run"@0xffffff8009245039;
		 * "52819682 mov"@0xffffff8008a5f404 -- __LINE__ = 3252.
		 * "29401504 ldp"@0xffffff8008a5f3f4 reads `c288` e `c292`
		 * TOGETHER */
#line 3252
		ILI_DBG("%s: Tx Max = %d, Tx Min = %d, Rx Max = %d,  Rx Min = %d\n", mp_items[index].c0, core_mp.c288, core_mp.c292, core_mp.c296, core_mp.c300);
	} else {
		/* key "max"@0xffffff8009244fe1;
		 * "b9001d00 str"@0xffffff8008a5ff04, +0x1c */
		parser_get_int_data(mp_items[index].c0, "max", str, 512);
		mp_items[index].c28 = katoi(str);
		/* key "min"@0xffffff8009244fe8;
		 * "b9002900 str"@0xffffff8008a5ff78, +0x28 */
		parser_get_int_data(mp_items[index].c0, "min", str, 512);
		mp_items[index].c40 = katoi(str);
	}

	/*
	 * "\x016ILITEK: (%s, %d): %s: run = %d, max = %d, min = %d, frame_count = %d\n"
	 * @0xffffff8009245045; "528197a2 mov"@0xffffff8008a5ffa4 --
	 * __LINE__ = 3261. The `run` field is read as ONE byte
	 * ("39405d04 ldrb"@0xffffff8008a5ff90)
	 */
#line 3261
	ILI_DBG("%s: run = %d, max = %d, min = %d, frame_count = %d\n", mp_items[index].c0, mp_items[index].c23, mp_items[index].c28, mp_items[index].c40, mp_items[index].c48);

	/*
	 * "\x016ILITEK: (%s, %d): v_tdf_1 = %d, v_tdf_2 = %d, h_tdf_1 = %d, h_tdf_2 = %d"
	 * @0xffffff800924508d -- WITHOUT a trailing "\n", and that is how it is in the
	 * binary; "52819802 mov"@0xffffff8008a5ffd0 -- __LINE__ = 3264
	 */
#line 3264
	ILI_DBG("v_tdf_1 = %d, v_tdf_2 = %d, h_tdf_1 = %d, h_tdf_2 = %d", mp_items[index].c64, mp_items[index].c68, mp_items[index].c72, mp_items[index].c76);

	/* "\x016ILITEK: (%s, %d): Run MP Test Item : %s\n"@0xffffff80092450d8;
	 * "52819842 mov"@0xffffff8008a5ffe8 -- __LINE__ = 3266 */
#line 3266
	ILI_INFO("Run MP Test Item : %s\n", mp_items[index].c0);

	/* "f9404908 ldr"@0xffffff8008a5fffc (+0x90) +
	 * "d63f0100 blr"@0xffffff8008a60004 with w0 = the index */
	mp_items[index].c144(index);

	/*
	 * "940017e0 bl"@0xffffff8008a6000c towards
	 * <mp_comp_result_before_retry>; "36f80180 tbz"@0xffffff8008a60010
	 * tests bit 31
	 */
	ret = mp_comp_result_before_retry(index);
	/* "396f6108 ldrb"@0xffffff8008a60018 + "34000128 cbz"@0xffffff8008a6001c */
	if (ret < 0 && core_mp.c216) {
		/* "\x016ILITEK: (%s, %d): MP failed, doing retry\n"
		 * @0xffffff8009245103; "52819902 mov"@0xffffff8008a60024 --
		 * __LINE__ = 3272 */
#line 3272
		ILI_INFO("MP failed, doing retry\n");
		/* "320007e1 orr"@0xffffff8008a60034 -- w1 = 3 */
		mp_do_retry(index, 3);
	}
}

/*
 * mp_print_csv_cdc_cmd() was reconstructed from the factory kernel disassembly (0xffffff800924446b).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void mp_print_csv_cdc_cmd(char *csv, int *csv_len, int index)
{
	int i, len = *csv_len;
	char str[128] = {0};
	char *open_sp_cmd[] = {"open dac", "open raw1", "open raw2", "open raw3"};
	char *open_c_cmd[] = {"open cap1 dac", "open cap1 raw"};
	char *name = mp_items[index].c0;

	/* "940fdbef bl"@0xffffff8008a61344 (strlen) +
	 * "f100641f cmp"@0xffffff8008a61354 (25) +
	 * "940fdc06 bl"@0xffffff8008a6136c (strncmp) con
	 * "open test(integration)_sp"@0xffffff800924319b */
	if (strlen(name) == strlen("open test(integration)_sp") &&
	    strncmp(name, "open test(integration)_sp",
		    strlen("open test(integration)_sp")) == 0) {
		for (i = 0; i < 4; i++) {
			if (parser_get_int_data("pv5_4 command", open_sp_cmd[i],
						str, 128) < 0) {
				/* "\x013ILITEK: (%s, %d): Failed to get CDC command %s from ini\n"
				 * @0xffffff8009245864, __func__ =
				 * "mp_print_csv_cdc_cmd"@0xffffff800924589f;
				 * "52808c82 mov"@0xffffff8008a613cc --
				 * __LINE__ = 1124 */
#line 1124
				ILI_ERR("Failed to get CDC command %s from ini\n", open_sp_cmd[i]);
				continue;
			}
			/* "%s = ,%s\n"@0xffffff80092458b4 */
			len += snprintf(csv + len, (CSV_FILE_SIZE - len),
					"%s = ,%s\n", open_sp_cmd[i], str);
		}
	/* "f1002c1f cmp"@0xffffff8008a61348 (11) +
	 * "940fdbe3 bl"@0xffffff8008a613f8 con
	 * "open test_c"@0xffffff80092431b5 */
	} else if (strlen(name) == strlen("open test_c") &&
		   strncmp(name, "open test_c", strlen("open test_c")) == 0) {
		for (i = 0; i < 2; i++) {
			if (parser_get_int_data("pv5_4 command", open_c_cmd[i],
						str, 128) < 0) {
				/* "52808da2 mov"@0xffffff8008a60c7c --
				 * __LINE__ = 1133 */
#line 1133
				ILI_ERR("Failed to get CDC command %s from ini\n", open_sp_cmd[i]);
				continue;
			}
			len += snprintf(csv + len, (CSV_FILE_SIZE - len),
					"%s = ,%s\n", open_c_cmd[i], str);
		}
	} else {
		if (parser_get_int_data("pv5_4 command", name, str, 128) < 0) {
			/* "52808e82 mov"@0xffffff8008a61494 -- __LINE__ = 1140 */
#line 1140
			ILI_ERR("Failed to get CDC command %s from ini\n", name);
			return;
		}
		/* "CDC command = ,%s\n"@0xffffff80092458be */
		len += snprintf(csv + len, (CSV_FILE_SIZE - len),
				"CDC command = ,%s\n", str);
	}

	*csv_len = len;
}

/*
 * mp_show_result() was reconstructed from the factory kernel disassembly (0xffffff8008a61d58).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mp_show_result(bool lcm_on)
{
	int i, j, ret = 0, len = 0, pass = 0, frame_cnt = 0, index = 0;
	char *csv = NULL, *csv_name = NULL;
	char str[128] = {0};
	int *max_threshold = NULL, *min_threshold = NULL;
	struct file *f = NULL;
	struct timespec64 ts;
	struct rtc_time tm;
	mm_segment_t old_fs;
	loff_t pos = 0;

	/* "320c03e0 orr"@0xffffff8008a60054 -- 0x100000 = 1 MiB;
	 * "97df8a05 bl"@0xffffff8008a6005c to <vmalloc> */
	csv = vmalloc(CSV_FILE_SIZE);
	if (IS_ERR(csv) || csv == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate CSV mem\n"
		 * @0xffffff80092453c9, __func__ =
		 * "mp_show_result"@0xffffff80092453f9;
		 * "52816542 mov"@0xffffff8008a60758 -- __LINE__ = 2858 */
#line 2858
		ILI_ERR("Failed to allocate CSV mem\n");
		/* "12800d56 mov"@0xffffff8008a60768 -- -107 */
		ret = -107;
		goto out;
	}

	/*
	 * TWO `kcalloc`s, not two `kmalloc`s: the proof is
	 * "37f84148 tbnz"@0xffffff8008a60084 (and its twin
	 * "37f84168 tbnz"@0xffffff8008a600a4), which is what the overflow check
	 * `n > SIZE_MAX / size` reduces to when `size` is the constant 4 and `n` a
	 * sign-extended `int`. `kmalloc` does not generate it.
	 * The flags 0x14080c0 confirm it: GFP_KERNEL | __GFP_ZERO.
	 */
	max_threshold = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	min_threshold = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	if (IS_ERR(max_threshold) || max_threshold == NULL ||
	    IS_ERR(min_threshold) || min_threshold == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate threshold FRAME buffer\n"
		 * @0xffffff800924512f; "52816642 mov"@0xffffff8008a60b50 --
		 * __LINE__ = 2866 */
#line 2866
		ILI_ERR("Failed to allocate threshold FRAME buffer\n");
		/* "12800d56 mov"@0xffffff8008a60b58 -- -107 */
		ret = -107;
		goto out;
	}

	/*
	 * "940005a0 bl"@0xffffff8008a60908 with "date"@0xffffff80091c8500 and
	 * "94000591 bl"@0xffffff8008a60944 with "version"@0xffffff800926923d.
	 * On the error path the compiler writes SEVEN bytes, not eight --
	 * "b9000269 str"@0xffffff8008a60924 (four) plus
	 * "b8003268 stur"@0xffffff8008a60920 at +3 (another four, three
	 * overlapping): a copy of length 7, that is WITHOUT the terminator.
	 * The buffer is already zeroed by `memset(&core_mp)`, though.
	 */
	if (parser_get_ini_key_value("pv5_4 command", "date", core_mp.c20) < 0)
		memcpy(core_mp.c20, "Unknown", strlen("Unknown"));

	if (parser_get_ini_key_value("pv5_4 command", "version", core_mp.c148) < 0)
		memcpy(core_mp.c148, "Unknown", strlen("Unknown"));

	/*
	 * "9410270f bl"@0xffffff8008a60984 and the ten that follow. The second
	 * argument is always "0x100000 - len"
	 * ("4b130288 sub"@0xffffff8008a60974)
	 */
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "==============================================================================\n");
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "ILITek C-TP Utility V%s\t%x : Driver Sensor Test\n", DRIVER_VERSION, core_mp.c0);
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Confidentiality Notice:\n");
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Any information of this tool is confidential and privileged.\n");
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "@ ILI TECHNOLOGY CORP. All Rights Reserved.\n");
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "==============================================================================\n");
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Firmware Version ,0x%x\n", core_mp.c8);
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Panel information ,XCH=%d, YCH=%d\n", core_mp.c232, core_mp.c236);
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "INI Release Version ,%s\n", core_mp.c20);
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "INI Release Date ,%s\n", core_mp.c148);
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Test Item:\n");

	/* "\t  ---%s\n"@0xffffff800924585a */
	for (i = 0; i < mp_run.num; i++)
		len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\t  ---%s\n", mp_items[mp_run.idx[i]].c0);

	/*
	 * "91002ec8 add"@0xffffff8008a60b24 (+11) +
	 * "7100491f cmp"@0xffffff8008a60b30 (#0x12 = 18) +
	 * "5400040d b.le"@0xffffff8008a60b38: the loop fills the header up to
	 * nineteen lines
	 */
	for (i = mp_run.num + 11; i < 19; i++)
		len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\n");

	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "==============================================================================\n");

	for (i = 0; i < mp_run.num; i++) {
		index = mp_run.idx[i];

		/*
		 * "b9402768 ldr"@0xffffff8008a611bc (+36) +
		 * "34000128 cbz"@0xffffff8008a611c4. The two messages do NOT carry
		 * the "ILITEK: (%s, %d): " prefix: they are bare `printk`s at
		 * level KERN_INFO ("\x016\n[%s],OK \n"@0xffffff8009245408 and
		 * "\x016\n[%s],NG \n"@0xffffff800924541f), and the two strings
		 * written into the CSV are two others
		 * ("\n[%s],OK\n"@0xffffff8009245415 and
		 * "\n[%s],NG\n"@0xffffff800924542c) -- they differ by the
		 * SPACE before the newline
		 */
		if (mp_items[index].c36 == 0) {
			printk(KERN_INFO "\n[%s],OK \n", mp_items[index].c0);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\n[%s],OK\n", mp_items[index].c0);
		} else {
			printk(KERN_INFO "\n[%s],NG \n", mp_items[index].c0);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\n[%s],NG\n", mp_items[index].c0);
		}

		/* "b8410d48 ldr"@0xffffff8008a6122c (pre-index su +0x10) +
		 * "7100291f cmp"@0xffffff8008a61230 (#0xa) */
		if (mp_items[index].c16 == 10) {
			printk(KERN_INFO "Test INT Pin = %d\n", mp_items[index].test_int_pin);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Test INT Pin = %d\n", mp_items[index].test_int_pin);
			printk(KERN_INFO "Pulse Test = %d\n", mp_items[index].int_pulse_test);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Pulse Test = %d\n", mp_items[index].int_pulse_test);
			printk(KERN_INFO "Delay Time = %d\n", mp_items[index].delay_time);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Delay Time = %d\n", mp_items[index].delay_time);
			continue;
		}

		mp_print_csv_cdc_cmd(csv, &len, index);

		/* "b8430e81 ldr"@0xffffff8008a614c8 (pre-index su +0x30) */
		printk(KERN_INFO "Frame count = %d\n", mp_items[index].c48);
		len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Frame count = %d\n", mp_items[index].c48);

		/* "b8434f08 ldr"@0xffffff8008a61508 (pre-index su +0x34) +
		 * "34000508 cbz"@0xffffff8008a6150c, e
		 * "7100211f cmp"@0xffffff8008a61518 (#0x8) */
		if (mp_items[index].c52 != 0 && mp_items[index].c16 != 8) {
			printk(KERN_INFO "lowest percentage = %d\n", mp_items[index].c56);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "lowest percentage = %d\n", mp_items[index].c56);
			printk(KERN_INFO "highest percentage = %d\n", mp_items[index].c60);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "highest percentage = %d\n", mp_items[index].c60);
		}

		/* "39405508 ldrb"@0xffffff8008a615b0 (+0x15) +
		 * "7100051f cmp"@0xffffff8008a615c0 */
		if (mp_items[index].c21 == 1) {
			/*
			 * "f8478d01 ldr"@0xffffff8008a615cc (+120) and
			 * "f940014d ldr"@0xffffff8008a615ec (+128): the two
			 * pointers are RE-READ on every iteration
			 */
			for (j = 0; j < core_mp.c256; j++) {
				max_threshold[j] = mp_items[index].bench_mark_max[j];
				min_threshold[j] = mp_items[index].bench_mark_min[j];
			}
			/* "940015bb bl"@0xffffff8008a61634, w4 = 0
			 * ("2a1f03e4 mov"@0xffffff8008a61624),
			 * x7 = "Max_Bench"@0xffffff80092454ba */
			mp_compare_cdc_show_result(index, mp_items[index].bench_mark_max, csv, &len, 0, max_threshold, min_threshold, "Max_Bench");
			/* x7 = "Min_Bench"@0xffffff80092454c4 */
			mp_compare_cdc_show_result(index, mp_items[index].bench_mark_min, csv, &len, 0, max_threshold, min_threshold, "Min_Bench");
		} else {
			/* "b841ce61 ldr"@0xffffff8008a6167c (+28) e
			 * "b940012a ldr"@0xffffff8008a61698 (+40) */
			for (j = 0; j < core_mp.c256; j++) {
				max_threshold[j] = mp_items[index].c28;
				min_threshold[j] = mp_items[index].c40;
			}
			printk(KERN_INFO "Max = %d\n", mp_items[index].c28);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Max = %d\n", mp_items[index].c28);
			printk(KERN_INFO "Min = %d\n", mp_items[index].c40);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Min = %d\n", mp_items[index].c40);
		}

		/* "f100641f cmp"@0xffffff8008a6175c (25) +
		 * "940fdb04 bl"@0xffffff8008a61774 */
		if (strlen(mp_items[index].c0) == strlen("open test(integration)_sp") &&
		    strncmp(mp_items[index].c0, "open test(integration)_sp",
			    strlen("open test(integration)_sp")) == 0) {
			/* "f9421901 ldr"@0xffffff8008a61784 (0xffffff800a0ff430),
			 * x7 = "frame1 cbk700"@0xffffff80092454e6 */
			mp_compare_cdc_show_result(index, frame1_cbk700, csv, &len, 1, max_threshold, min_threshold, "frame1 cbk700");
			/* "f9421d01 ldr"@0xffffff8008a617b0,
			 * x7 = "frame1 cbk250"@0xffffff80092454f4 */
			mp_compare_cdc_show_result(index, frame1_cbk250, csv, &len, 1, max_threshold, min_threshold, "frame1 cbk250");
			/* "f9422101 ldr"@0xffffff8008a617dc,
			 * x7 = "frame1 cbk200"@0xffffff8009245502 */
			mp_compare_cdc_show_result(index, frame1_cbk200, csv, &len, 1, max_threshold, min_threshold, "frame1 cbk200");
		}

		/* "f1002c1f cmp"@0xffffff8008a61810 (11) +
		 * "940fdad7 bl"@0xffffff8008a61828 */
		if (strlen(mp_items[index].c0) == strlen("open test_c") &&
		    strncmp(mp_items[index].c0, "open test_c",
			    strlen("open test_c")) == 0) {
			/* "f9422501 ldr"@0xffffff8008a61838,
			 * x7 = "CAP_DAC"@0xffffff8009245510 */
			mp_compare_cdc_show_result(index, cap_dac, csv, &len, 1, max_threshold, min_threshold, "CAP_DAC");
			/* "f9422901 ldr"@0xffffff8008a61864,
			 * x7 = "CAP_RAW"@0xffffff8009245518 */
			mp_compare_cdc_show_result(index, cap_raw, csv, &len, 1, max_threshold, min_threshold, "CAP_RAW");
		}

		/* "7100111f cmp"@0xffffff8008a61894 (#0x4) */
		if (mp_items[index].c16 == 4) {
			/*
			 * "f9461d08 ldr"@0xffffff8008a618a0 reads `rx_delta_buf`
			 * FIRST and "f9461908 ldr"@0xffffff8008a618b8 reads
			 * `tx_delta_buf` AFTERWARDS -- the order is SUGGESTED by
			 * the binary, not measured: they are two pure loads inside
			 * an `||` and the compiler may swap them
			 */
			if (IS_ERR(core_mp.rx_delta_buf) || core_mp.rx_delta_buf == NULL ||
			    IS_ERR(core_mp.tx_delta_buf) || core_mp.tx_delta_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): This test item (%s) has no data inside its buffer\n"
				 * @0xffffff800924518a;
				 * "52817062 mov"@0xffffff8008a61afc --
				 * __LINE__ = 2947 */
#line 2947
				ILI_ERR("This test item (%s) has no data inside its buffer\n", mp_items[index].c0);
				continue;
			}
			/* "29442a69 ldp"@0xffffff8008a618e4 reads `c288` e
			 * `c292` TOGETHER */
			for (j = 0; j < core_mp.c256; j++) {
				max_threshold[j] = core_mp.c288;
				min_threshold[j] = core_mp.c292;
			}
			/* x7 = "TX Max Hold"@0xffffff8009245547 e
			 * "TX Min Hold"@0xffffff8009245553, w4 = 2 */
			mp_compare_cdc_show_result(index, core_mp.tx_max_buf, csv, &len, 2, max_threshold, min_threshold, "TX Max Hold");
			mp_compare_cdc_show_result(index, core_mp.tx_min_buf, csv, &len, 2, max_threshold, min_threshold, "TX Min Hold");
			/* "29452a69 ldp"@0xffffff8008a61964 -- `c296` e `c300` */
			for (j = 0; j < core_mp.c256; j++) {
				max_threshold[j] = core_mp.c296;
				min_threshold[j] = core_mp.c300;
			}
			/* x7 = "RX Max Hold"@0xffffff800924555f e
			 * "RX Min Hold"@0xffffff800924556b */
			mp_compare_cdc_show_result(index, core_mp.rx_max_buf, csv, &len, 2, max_threshold, min_threshold, "RX Max Hold");
			mp_compare_cdc_show_result(index, core_mp.rx_min_buf, csv, &len, 2, max_threshold, min_threshold, "RX Min Hold");
			continue;
		}

		/* "f8460f89 ldr"@0xffffff8008a619f0 (+96, pre-index),
		 * "f9403521 ldr"@0xffffff8008a61a0c (+104),
		 * "f8470e69 ldr"@0xffffff8008a61a28 (+112, pre-index) */
		if (IS_ERR(mp_items[index].buf) || mp_items[index].buf == NULL ||
		    IS_ERR(mp_items[index].max_buf) || mp_items[index].max_buf == NULL ||
		    IS_ERR(mp_items[index].min_buf) || mp_items[index].min_buf == NULL) {
			/* "52817122 mov"@0xffffff8008a61ad8 -- __LINE__ = 2953 */
#line 2953
			ILI_ERR("This test item (%s) has no data inside its buffer\n", mp_items[index].c0);
			continue;
		}

		/* "7100091f cmp"@0xffffff8008a61a38 (#0x2) */
		if (mp_items[index].c16 == 2) {
			/*
			 * This section was reconstructed from the factory kernel disassembly (0xffffff8009245522).
			 *
			 * The working notes -- the disassembly citations, the measurements against
			 * the factory binary and the reasoning behind each choice -- are in
			 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
			 * in the oracolo repository. They are kept in Italian, as the project's
			 * internal record.
			 */
			for (j = 0; j < core_mp.key_len; j++) {
				ILI_CONT("KEY_%02d ", j);
				len += snprintf(csv + len, (CSV_FILE_SIZE - len), "KEY_%02d,", j);
			}
			ILI_CONT("\n");
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\n");
			/* " %3d   "@0xffffff8009245538 (tail of
			 * 0xffffff8009245536) e " %3d, "@0xffffff8009245540 */
			for (j = 0; j < core_mp.key_len; j++) {
				ILI_CONT(" %3d   ", mp_items[index].buf[j]);
				len += snprintf(csv + len, (CSV_FILE_SIZE - len), " %3d, ", mp_items[index].buf[j]);
			}
			ILI_CONT("\n");
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\n");
			continue;
		}

		/* "7100211f cmp"@0xffffff8008a60f48 (#0x8) +
		 * "b9400308 ldr"@0xffffff8008a60f50 (`c52`) */
		if (mp_items[index].c16 != 8 && mp_items[index].c52 != 0) {
			/* "f9402d01 ldr"@0xffffff8008a60f68 (+88),
			 * x7 = "Mean result"@0xffffff8009245577 */
			mp_compare_cdc_show_result(index, mp_items[index].result_buf, csv, &len, 2, max_threshold, min_threshold, "Mean result");
		} else {
			/* x7 = "Max Hold"@0xffffff8009245562 e
			 * "Min Hold"@0xffffff800924556e */
			mp_compare_cdc_show_result(index, mp_items[index].max_buf, csv, &len, 2, max_threshold, min_threshold, "Max Hold");
			mp_compare_cdc_show_result(index, mp_items[index].min_buf, csv, &len, 2, max_threshold, min_threshold, "Min Hold");
		}

		/* "320003f4 orr"@0xffffff8008a60ff0 -- 1 */
		if (mp_items[index].c16 == 8)
			frame_cnt = 1;
		else
			frame_cnt = mp_items[index].c48;

		/* "Frame %d"@0xffffff8009245583;
		 * "1b157d08 mul"@0xffffff8008a61088 (core_mp.c256 * j) +
		 * "8b28c921 add"@0xffffff8008a6108c (`lsl #2`) */
		for (j = 0; j < frame_cnt; j++) {
			memset(str, 0, sizeof(str));
			snprintf(str, (CSV_FILE_SIZE - len), "Frame %d", j + 1);
			mp_compare_cdc_show_result(index, &mp_items[index].buf[core_mp.c256 * j], csv, &len, 1, max_threshold, min_threshold, str);
		}
	}

	/*
	 * memset() was reconstructed from the factory kernel disassembly (0xffffff800a0fcc70, 128 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	memset(csv_path, 0, sizeof(csv_path));
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "==============================================================================\n");
	/* "Result_Summary\t\t\t\n"@0xffffff8009245983 */
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Result_Summary\t\t\t\n");

	/*
	 * "9a970362 csel"@0xffffff8008a61c00 chooses between
	 * "\t  {%s}\t   ,OK\n"@0xffffff8009245996 (condition `eq`, that is
	 * `c36 == 0`) and "\t  {%s}\t   ,NG\n"@0xffffff80092459a6
	 */
	for (i = 0; i < mp_run.num; i++) {
		index = mp_run.idx[i];
		len += snprintf(csv + len, (CSV_FILE_SIZE - len), (mp_items[index].c36 == 0) ? "\t  {%s}\t   ,OK\n" : "\t  {%s}\t   ,NG\n", mp_items[index].c0);
	}

	/*
	 * THE FINAL COUNT. "385f314b ldurb"@0xffffff8008a61c3c reads `c23`
	 * (ONE byte) and "b940014b ldr"@0xffffff8008a61c44 reads `c36` (four),
	 * with "37f802cb tbnz"@0xffffff8008a61c48 on bit 31. The round covers
	 * ALL fifty items ("f100c93f cmp"@0xffffff8008a61c54),
	 * not only the ones that ran.
	 */
	for (i = 0; i < MP_TEST_ITEM; i++) {
		if (!mp_items[i].c23)
			continue;
		if (mp_items[i].c36 < 0)
			goto fail;
		pass++;
	}

	/* "34000208 cbz"@0xffffff8008a61c60 */
	if (pass == 0)
		goto fail;

	/* "b900011f str"@0xffffff8008a61c70 */
	core_mp.c264 = 0;
	/*
	 * "39418509 ldrb"@0xffffff8008a61c6c (+97 on 0xffffff800a0fcc08, that is
	 * `core_mp` + 361) + "340006e9 cbz"@0xffffff8008a61c74
	 */
	if (core_mp.c361) {
		/* "\x013ILITEK: (%s, %d): WARNING! Golden and SPEC in ini file aren't matched!!\n"
		 * @0xffffff80092455ef; "52817c22 mov"@0xffffff8008a61c88 --
		 * __LINE__ = 3041 */
#line 3041
		ILI_ERR("WARNING! Golden and SPEC in ini file aren't matched!!\n");
		/* "mp_warning"@0xffffff80092455e4 */
		csv_name = "mp_warning";
	} else {
		/* "mp_pass"@0xffffff800924558c */
		csv_name = "mp_pass";
	}

	/*
	 * getnstimeofday64() was reconstructed from the factory kernel disassembly (0xffffff8008a61d3c).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	getnstimeofday64(&ts);
	rtc_time64_to_tm(ts.tv_sec, &tm);
	/* "%04d%02d%02d-%02d%02d%02d"@0xffffff80092459b6, 128 byte
	 * ("321903e1 orr"@0xffffff8008a61da8) */
	snprintf(csv_time, 128, "%04d%02d%02d-%02d%02d%02d",
		 (int)tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		 tm.tm_hour, tm.tm_min, tm.tm_sec);
	/* "36000096 tbz"@0xffffff8008a61dc4 tests BIT 0 of `lcm_on`;
	 * "%s/%s_%s.csv"@0xffffff800924559c */
	snprintf(csv_path, (CSV_FILE_SIZE - len), "%s/%s_%s.csv",
		 lcm_on ? "/sdcard/ilitek_mp_lcm_on_log" :
			  "/sdcard/ilitek_mp_lcm_off_log",
		 csv_name, csv_time);
	/* "2a1f03f6 mov"@0xffffff8008a61df0 */
	ret = 0;
	goto write;

fail:
	/* "b90c0909 str"@0xffffff8008a61cac -- w9 = -1 */
	core_mp.c264 = -1;

	/* "97dbcdd9 bl"@0xffffff8008a61cbc to <getnstimeofday64> e
	 * "94008933 bl"@0xffffff8008a61cc8 to <rtc_time64_to_tm>;
	 * "111db103 add"@0xffffff8008a61cec (+1900) e
	 * "11000524 add"@0xffffff8008a61cf0 (+1) */
	getnstimeofday64(&ts);
	rtc_time64_to_tm(ts.tv_sec, &tm);
	snprintf(csv_time, 128, "%04d%02d%02d-%02d%02d%02d",
		 (int)tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		 tm.tm_hour, tm.tm_min, tm.tm_sec);
	/* "36000096 tbz"@0xffffff8008a61d18; "mp_fail"@0xffffff8009245594 */
	snprintf(csv_path, (CSV_FILE_SIZE - len), "%s/%s_%s.csv",
		 lcm_on ? "/sdcard/ilitek_mp_lcm_on_log" :
			  "/sdcard/ilitek_mp_lcm_off_log",
		 "mp_fail", csv_time);
	/* "12800016 mov"@0xffffff8008a61d48 */
	ret = -1;

write:
	/* "\x016ILITEK: (%s, %d): Open CSV : %s\n"@0xffffff800924563a;
	 * "52817d42 mov"@0xffffff8008a61e0c -- __LINE__ = 3050 */
#line 3050
	ILI_INFO("Open CSV : %s\n", csv_path);

	/* "97e00f0e bl"@0xffffff8008a61e24 to <filp_open>, w1 = 577
	 * ("52804821 mov"@0xffffff8008a61e18) e w2 = 644 */
	f = filp_open(csv_path, O_WRONLY | O_CREAT | O_TRUNC, 644);
	if (IS_ERR(f) || f == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to open CSV file"
		 * @0xffffff800924565d; "52817e02 mov"@0xffffff8008a61e90 --
		 * __LINE__ = 3056 */
#line 3056
		ILI_ERR("Failed to open CSV file");
		/*
		 * The branch has no `mov` of its own: "17fffa34 b"@0xffffff8008a61e98
		 * jumps to "12800d56 mov"@0xffffff8008a60768, that is -107
		 */
		ret = -107;
		goto out;
	}

	/* "\x016ILITEK: (%s, %d): Open CSV succeed, its length = %d\n "
	 * @0xffffff8009245689; "52817ea2 mov"@0xffffff8008a61e4c --
	 * __LINE__ = 3061 */
#line 3061
	ILI_INFO("Open CSV succeed, its length = %d\n ", len);

	/* "7144011f cmp"@0xffffff8008a61e58 -- 0x100 << 12 = 0x100000 */
	if (len >= CSV_FILE_SIZE) {
		/* "52817f02 mov"@0xffffff8008a61e70 -- __LINE__ = 3064 */
#line 3064
		ILI_ERR("The length saved to CSV is too long !\n");
		/* "12800d16 mov"@0xffffff8008a61e78 -- -105 */
		ret = -105;
		goto out;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	/* "97e0148f bl"@0xffffff8008a61ed8 to <vfs_write> */
	vfs_write(f, csv, len, &pos);
	set_fs(old_fs);
	/* "97e01014 bl"@0xffffff8008a61f10 to <filp_close> */
	filp_close(f, NULL);

	/* "\x016ILITEK: (%s, %d): Writing Data into CSV succeed\n"
	 * @0xffffff80092456fc; "52818082 mov"@0xffffff8008a61f24 --
	 * __LINE__ = 3076 */
#line 3076
	ILI_INFO("Writing Data into CSV succeed\n");

out:
	/* "b4000078 cbz"@0xffffff8008a60770 + "97df86b0 bl"@0xffffff8008a60778 */
	if (csv)
		vfree(csv);
	if (max_threshold)
		kfree(max_threshold);
	if (min_threshold)
		kfree(min_threshold);
	return ret;
}

/*
 * mp_copy_ret_to_apk() was reconstructed from the factory kernel disassembly (0xffffff8008a607a0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void mp_copy_ret_to_apk(char *buf)
{
	int i, len = 0, index = 0;

	/* "b40007f3 cbz"@0xffffff8008a60794 */
	if (buf == NULL) {
		/* "\x013ILITEK: (%s, %d): apk buffer is null\n"
		 * @0xffffff80092459d0, __func__ =
		 * "mp_copy_ret_to_apk"@0xffffff80092459f8;
		 * "52819fc2 mov"@0xffffff8008a608a0 -- __LINE__ = 3326 */
#line 3326
		ILI_ERR("apk buffer is null\n");
		return;
	}

	/* "CSV path: %s\n\n"@0xffffff8009245a0b */
	len = snprintf(buf + 2, (APK_BUF_SIZE - 2), "CSV path: %s\n\n", csv_path);
	len += 2;

	for (i = 0; i < mp_run.num; i++) {
		index = mp_run.idx[i];
		/* "3100051f cmn"@0xffffff8008a60818 -- the comparison with -1 */
		if (mp_items[index].c36 == -1) {
			/* "\x013ILITEK: (%s, %d): [%s] = FAIL\n"
			 * @0xffffff8009245a1a; "5281a0c2 mov"@0xffffff8008a6083c
			 * -- __LINE__ = 3334 */
#line 3334
			ILI_ERR("[%s] = FAIL\n", mp_items[index].c0);
			/* "[%s] = FAIL\n"@0xffffff8009245a2e */
			len += snprintf(buf + len, (APK_BUF_SIZE - len), "[%s] = FAIL\n", mp_items[index].c0);
		} else {
			/* "\x016ILITEK: (%s, %d): [%s] = PASS\n"
			 * @0xffffff8009245a3b; "5281a122 mov"@0xffffff8008a60820
			 * -- __LINE__ = 3337 */
#line 3337
			ILI_INFO("[%s] = PASS\n", mp_items[index].c0);
			/* "[%s] = PASS\n"@0xffffff8009245a4f */
			len += snprintf(buf + len, (APK_BUF_SIZE - len), "[%s] = PASS\n", mp_items[index].c0);
		}
	}

	/* "b9027113 str"@0xffffff8008a60888 -- `idev` + 624 */
	idev->c624 = len;
}

/*
 * ===========================================================================
 * mp_test_free -- line 3282
 * ===========================================================================
 * The order of the frees is the binary's, and it is not the order of the
 * allocations: first the three `mp_frame_buf` vectors the "open test sp"
 * test fills (indices 2, 3, 4), then the two no written function reads
 * (indices 0 and 1).
 */
static void mp_test_free(void)
{
	int i;

	/*
	 * "\x016ILITEK: (%s, %d): Free all allocated mem for MP\n"
	 * @0xffffff8009245a5c, __func__ = "mp_test_free"@0xffffff8009245a8f;
	 * "52819a42 mov"@0xffffff8008a5f010 -- __LINE__ = 3282.
	 * It is NOT guarded by `ilitek_dbg_en`: no `ldrb` precedes it
	 */
#line 3282
	ILI_INFO("Free all allocated mem for MP\n");

	/* "b90c0913 str"@0xffffff8008a5f044 -- w19 = -1 */
	core_mp.c264 = -1;

	for (i = 0; i < MP_TEST_ITEM; i++) {
		/* "381bf29f sturb"@0xffffff8008a5f04c */
		mp_items[i].c23 = 0;
		/*
		 * "f81c8293 stur"@0xffffff8008a5f050 -- EIGHT bytes at +32, that is
		 * `c32` and `c36` together, both set to -1
		 */
		mp_items[i].c32 = -1;
		mp_items[i].c36 = -1;
		/* "b81d4293 stur"@0xffffff8008a5f054 */
		mp_items[i].c44 = -1;

		/* "b85b8288 ldur"@0xffffff8008a5f048 (+16) +
		 * "7100111f cmp"@0xffffff8008a5f058 (#0x4) */
		if (mp_items[i].c16 == 4) {
			/* "f9461ee0 ldr"@0xffffff8008a5f060 -- 0xffffff800a0fcc38 */
			ipio_kfree((void **)&core_mp.rx_delta_buf);
			/* "f9461b00 ldr"@0xffffff8008a5f070 -- 0xffffff800a0fcc30 */
			ipio_kfree((void **)&core_mp.tx_delta_buf);
			ipio_kfree((void **)&core_mp.tx_max_buf);
			ipio_kfree((void **)&core_mp.tx_min_buf);
			ipio_kfree((void **)&core_mp.rx_max_buf);
			ipio_kfree((void **)&core_mp.rx_min_buf);
			continue;
		}

		/* "385bd288 ldurb"@0xffffff8008a5f0c4 (+21) +
		 * "7100051f cmp"@0xffffff8008a5f0c8 */
		if (mp_items[i].c21 == 1) {
			/* "f9401280 ldr"@0xffffff8008a5f0d0 (+120) e
			 * "f9401680 ldr"@0xffffff8008a5f0e0 (+128) */
			ipio_kfree((void **)&mp_items[i].bench_mark_max);
			ipio_kfree((void **)&mp_items[i].bench_mark_min);
		}
		/* "f9401a80 ldr"@0xffffff8008a5f0f0 (+136) */
		ipio_kfree((void **)&mp_items[i].c136);
		/* "f85b0280 ldur"@0xffffff8008a5f100 (+8) */
		ipio_kfree((void **)&mp_items[i].c8);
		/* "f9400280 ldr"@0xffffff8008a5f110 (+88) */
		ipio_kfree((void **)&mp_items[i].result_buf);
		/* "f9400a80 ldr"@0xffffff8008a5f120 (+104) */
		ipio_kfree((void **)&mp_items[i].max_buf);
		/* "f9400e80 ldr"@0xffffff8008a5f130 (+112) */
		ipio_kfree((void **)&mp_items[i].min_buf);
		/* "f9400680 ldr"@0xffffff8008a5f140 (+96) +
		 * "97df8c3c bl"@0xffffff8008a5f148 to <vfree> */
		ipio_vfree((void **)&mp_items[i].buf);
	}

	/* "f9421a60 ldr"@0xffffff8008a5f160 -- 0xffffff800a0ff430 */
	ipio_kfree((void **)&frame1_cbk700);
	/* "f9421e60 ldr"@0xffffff8008a5f174 -- 0xffffff800a0ff438 */
	ipio_kfree((void **)&frame1_cbk250);
	/* "f9422260 ldr"@0xffffff8008a5f18c -- 0xffffff800a0ff440 */
	ipio_kfree((void **)&frame1_cbk200);
	/* "f9421260 ldr"@0xffffff8008a5f1a0 -- 0xffffff800a0ff420 */
	ipio_kfree((void **)&mp_frame_buf0);
	/* "f9421660 ldr"@0xffffff8008a5f1b4 -- 0xffffff800a0ff428 */
	ipio_kfree((void **)&mp_frame_buf1);
}

/*
 * ilitek_tddi_mp_test_main() was reconstructed from the factory kernel disassembly (0xffffff8008a5e618, 14704 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_mp_test_main(char *apk, bool lcm_on)
{
	int i, ret = 0;
	char *val = NULL, *hex = NULL;

	/*
	 * "321903e0 orr"@0xffffff8008a5e644 (128) +
	 * "97df9088 bl"@0xffffff8008a5e650, and the twin pair at
	 * 0xffffff8008a5e658/0xffffff8008a5e65c. THE NAMES ARE CHOSEN
	 */
	val = vmalloc(128);
	hex = vmalloc(128);

	/* "3943e103 ldrb"@0xffffff8008a5e668 e
	 * "3943e504 ldrb"@0xffffff8008a5e66c read `idev` at +248 and +249, UN
	 * byte each; "34001243 cbz"@0xffffff8008a5e674 e
	 * "34001224 cbz"@0xffffff8008a5e678 */
	if (idev->c248 == 0 || idev->c249 == 0) {
		/* "\x013ILITEK: (%s, %d): Invalid frame length (%d, %d)\n"
		 * @0xffffff8009243282, __func__ =
		 * "ilitek_tddi_mp_test_main"@0xffffff80092432b5;
		 * "5281a702 mov"@0xffffff8008a5e8cc -- __LINE__ = 3384 */
#line 3384
		ILI_ERR("Invalid frame length (%d, %d)\n", idev->c248, idev->c249);
		/* "12800d16 mov"@0xffffff8008a5e8d4 -- -105 */
		ret = -105;
		goto out;
	}

	/*
	 * "52982400 mov"@0xffffff8008a5e67c + "72a00780 movk"@0xffffff8008a5e680
	 * -- 0x3cc120 = 3981600 = 2212 * 1800, that is
	 * sizeof(struct ilitek_ini_item) * PARSER_MAX_KEY_NUM.
	 * "f9020500 str"@0xffffff8008a5e690 -- 0xffffff800a0ff408
	 */
	ilitek_ini_data = vmalloc(sizeof(struct ilitek_ini_item) * PARSER_MAX_KEY_NUM);
	if (IS_ERR(ilitek_ini_data) || ilitek_ini_data == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to malloc ini_info\n"
		 * @0xffffff80092432ce; "5281a7e2 mov"@0xffffff8008a5e8ec --
		 * __LINE__ = 3391 */
#line 3391
		ILI_ERR("Failed to malloc ini_info\n");
		/* "12800d56 mov"@0xffffff8008a5e8f4 -- -107 */
		ret = -107;
		goto out;
	}

	ilitek_tddi_mp_init_item();

	if (ilitek_tddi_mp_ini_parser() < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to parsing INI file\n"
		 * @0xffffff80092432fd; "5281a902 mov"@0xffffff8008a5efd4 --
		 * __LINE__ = 3400 */
#line 3400
		ILI_ERR("Failed to parsing INI file\n");
		/* "12800cd6 mov"@0xffffff8008a5efdc -- -103 */
		ret = -103;
		goto out;
	}

	/* "94000cd2 bl"@0xffffff8008a5ec40 con
	 * "pv5_4 command"@0xffffff800924332d e
	 * "protocol"@0xffffff80090e4a28 */
	parser_get_ini_key_value("pv5_4 command", "protocol", val);
	/* "94102e59 bl"@0xffffff8008a5ec5c, w1 = 8
	 * ("321d03e1 orr"@0xffffff8008a5ec50), x2 = "0x%s"@0xffffff800924333b */
	snprintf(hex, 8, "0x%s", val);
	/* "94003231 bl"@0xffffff8008a5ec64 to <str2hex> e
	 * "6b48201f cmp"@0xffffff8008a5ec70 -- `core_mp.c12` shifted EIGHT
	 * bits to the right */
	if (str2hex(hex) != (core_mp.c12 >> 8)) {
		/*
		 * "\x013ILITEK: (%s, %d): ERROR! MP Protocol version is invaild, 0x%x\n"
		 * @0xffffff8009243340; "5281aa22 mov"@0xffffff8008a5ef9c --
		 * __LINE__ = 3409. `str2hex` IS CALLED TWICE: the second one
		 * is "94003169 bl"@0xffffff8008a5ef84, and its result is
		 * the `%x` argument
		 */
#line 3409
		ILI_ERR("ERROR! MP Protocol version is invaild, 0x%x\n", str2hex(hex));
		/* "12800c96 mov"@0xffffff8008a5efa4 -- -101 */
		ret = -101;
		goto out;
	}

	if (mp_get_timing_info() < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to get timing info from ini\n"
		 * @0xffffff8009243381; "5281ab02 mov"@0xffffff8008a5eff4 --
		 * __LINE__ = 3416 */
#line 3416
		ILI_ERR("Failed to get timing info from ini\n");
		/* "12800cf6 mov"@0xffffff8008a5effc -- -104 */
		ret = -104;
		goto out;
	}

	if (mp_sort_item(lcm_on) < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to sort test item\n"
		 * @0xffffff80092433b9; "5281abe2 mov"@0xffffff8008a606c0 --
		 * __LINE__ = 3423. The jump
		 * "17fffa45 b"@0xffffff8008a606c4 reuses the `printk` of the
		 * line 3400 branch
		 */
#line 3423
		ILI_ERR("Failed to sort test item\n");
		/*
		 * The branch reuses the line 3400 `printk` and with it its
		 * constant: "12800cd6 mov"@0xffffff8008a5efdc -- -103
		 */
		ret = -103;
		goto out;
	}

	/*
	 * "f000b4e8 adrp"@0xffffff8008a60040 +
	 * "b9845908 ldrsw"@0xffffff8008a60044 -- the limit is RE-READ on every
	 * iteration, so in the source it sits inside the `for` condition and not
	 * in a hoisted local (class A2, read the other way round)
	 */
	for (i = 0; i < mp_run.num; i++)
		mp_test_run(mp_run.idx[i]);

	ret = mp_show_result(lcm_on);
	mp_copy_ret_to_apk(apk);

out:
	mp_test_free();

	/*
	 * "a94453f5 ldp"@0xffffff8008a5f178 re-reads the two pointers from the
	 * stack; "b4000074 cbz"@0xffffff8008a5f1c4 and
	 * "b4000075 cbz"@0xffffff8008a5f1d0. NEITHER is set back to NULL:
	 * they are locals
	 */
	if (val)
		vfree(val);
	if (hex)
		vfree(hex);

	/*
	 * "f9420660 ldr"@0xffffff8008a5f1e0 + "f902067f str"@0xffffff8008a5f1ec
	 * -- this one IS set back to NULL, and it is a global
	 */
	ipio_vfree((void **)&ilitek_ini_data);

	return ret;
}

/*
 * This section was reconstructed from the factory kernel disassembly (7148 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * TWO LOCAL DECLARATIONS (a new entry in the HEADER DELTA): `ilitek.h` does
 * not carry them, and the two groups that define them are other files of the
 * same object. `ilitek_touch.c` defines `ilitek_dump_data` (0xffffff8008a5aad8)
 * and `ilitek_ic.c` defines `ilitek_tddi_ic_check_busy`
 * (0xffffff8008a594d4, 560 bytes). The signatures are COPIED from those two
 * files, not invented here.
 */
void ilitek_dump_data(void *data, int width, int count, int per_row,
		      const char *label);
int ilitek_tddi_ic_check_busy(int count, int delay);

/*
 * create_mp_test_frame_buffer() was reconstructed from the factory kernel disassembly (0xffffff8008a651d0, 1388 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#line 1881
static __used int create_mp_test_frame_buffer(int index, int frame_count)
{
	/*
	 * "\x016ILITEK: (%s, %d): Create MP frame buffers (index = %d), count = %d\n"
	 * @0xffffff8009243b0c, __func__ =
	 * "create_mp_test_frame_buffer"@0xffffff8009243b52; the guard is
	 * "39656108 ldrb"@0xffffff8008a651e4 + "34000128 cbz"@0xffffff8008a651f0;
	 * "5280eba2 mov"@0xffffff8008a65204 -- __LINE__ = 1885. The call spans
	 * TWO lines and `__LINE__` is the line of the CLOSING parenthesis:
	 * the directive therefore carries 1884
	 */
#line 1884
	ILI_DBG("Create MP frame buffers (index = %d), count = %d\n",
		index, frame_count);

	/* "b9401108 ldr"@0xffffff8008a65224 (+0x10) +
	 * "7100111f cmp"@0xffffff8008a65228 + "540003e1 b.ne"@0xffffff8008a6522c */
	if (mp_items[index].c16 == 4) {
		/* "f9461a68 ldr"@0xffffff8008a65234 (0xffffff800a0fcc30) +
		 * "b4000cc8 cbz"@0xffffff8008a65238 */
		if (!core_mp.tx_delta_buf) {
			core_mp.tx_delta_buf = kcalloc(core_mp.c256,
						       sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.tx_delta_buf) ||
			    core_mp.tx_delta_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate tx_delta_buf mem\n"
				 * @0xffffff8009243b6e;
				 * "5280ec62 mov"@0xffffff8008a654d0 -- __LINE__ = 1891 */
#line 1891
				ILI_ERR("Failed to allocate tx_delta_buf mem\n");
				ILI_KFREE(core_mp.tx_delta_buf);
				return -ENOMEM;
			}
		}

		/* "f9461e68 ldr"@0xffffff8008a65240 (0xffffff800a0fcc38) */
		if (!core_mp.rx_delta_buf) {
			core_mp.rx_delta_buf = kcalloc(core_mp.c256,
						       sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.rx_delta_buf) ||
			    core_mp.rx_delta_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate rx_delta_buf mem\n"
				 * @0xffffff8009243ba7;
				 * "5280ed82 mov"@0xffffff8008a65554 -- __LINE__ = 1900 */
#line 1900
				ILI_ERR("Failed to allocate rx_delta_buf mem\n");
				ILI_KFREE(core_mp.rx_delta_buf);
				return -ENOMEM;
			}
		}

		/* "f9462268 ldr"@0xffffff8008a6524c (0xffffff800a0fcc40) */
		if (!core_mp.tx_max_buf) {
			core_mp.tx_max_buf = kcalloc(core_mp.c256,
						     sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.tx_max_buf) ||
			    core_mp.tx_max_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate tx_max_buf mem\n"
				 * @0xffffff8009243be0;
				 * "5280eea2 mov"@0xffffff8008a655a8 -- __LINE__ = 1909 */
#line 1909
				ILI_ERR("Failed to allocate tx_max_buf mem\n");
				ILI_KFREE(core_mp.tx_max_buf);
				return -ENOMEM;
			}
		}

		/* "f9462668 ldr"@0xffffff8008a65258 (0xffffff800a0fcc48) */
		if (!core_mp.tx_min_buf) {
			core_mp.tx_min_buf = kcalloc(core_mp.c256,
						     sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.tx_min_buf) ||
			    core_mp.tx_min_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate tx_min_buf mem\n"
				 * @0xffffff8009243c17;
				 * "5280efc2 mov"@0xffffff8008a65654 -- __LINE__ = 1918 */
#line 1918
				ILI_ERR("Failed to allocate tx_min_buf mem\n");
				ILI_KFREE(core_mp.tx_min_buf);
				return -ENOMEM;
			}
		}

		/* "f9462a68 ldr"@0xffffff8008a65264 (0xffffff800a0fcc50) */
		if (!core_mp.rx_max_buf) {
			core_mp.rx_max_buf = kcalloc(core_mp.c256,
						     sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.rx_max_buf) ||
			    core_mp.rx_max_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate rx_max_buf mem\n"
				 * @0xffffff8009243c4e;
				 * "5280f0e2 mov"@0xffffff8008a65684 -- __LINE__ = 1927 */
#line 1927
				ILI_ERR("Failed to allocate rx_max_buf mem\n");
				ILI_KFREE(core_mp.rx_max_buf);
				return -ENOMEM;
			}
		}

		/* "f9462e68 ldr"@0xffffff8008a65270 (0xffffff800a0fcc58) */
		if (!core_mp.rx_min_buf) {
			core_mp.rx_min_buf = kcalloc(core_mp.c256,
						     sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.rx_min_buf) ||
			    core_mp.rx_min_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate rx_min_buf mem\n"
				 * @0xffffff8009243c85;
				 * "5280f202 mov"@0xffffff8008a656b4 -- __LINE__ = 1936 */
#line 1936
				ILI_ERR("Failed to allocate rx_min_buf mem\n");
				ILI_KFREE(core_mp.rx_min_buf);
				return -ENOMEM;
			}
		}
	} else {
		/* "f8460ec8 ldr"@0xffffff8008a652b4 -- pre-index, +0x60 = 96 */
		if (!mp_items[index].buf) {
			mp_items[index].buf = vmalloc(core_mp.c256 *
						      frame_count * sizeof(int));
			if (IS_ERR(mp_items[index].buf) ||
			    mp_items[index].buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate buf mem\n"
				 * @0xffffff8009243cbc;
				 * "5280f322 mov"@0xffffff8008a65470 -- __LINE__ = 1945 */
#line 1945
				ILI_ERR("Failed to allocate buf mem\n");
				ILI_KFREE(mp_items[index].buf);
				return -ENOMEM;
			}
		}

		/* "f8458e68 ldr"@0xffffff8008a652e8 -- pre-index, +0x58 = 88 */
		if (!mp_items[index].result_buf) {
			mp_items[index].result_buf = kcalloc(core_mp.c256,
							     sizeof(int), GFP_KERNEL);
			if (IS_ERR(mp_items[index].result_buf) ||
			    mp_items[index].result_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate result_buf mem\n"
				 * @0xffffff8009243cec;
				 * "5280f442 mov"@0xffffff8008a65538 -- __LINE__ = 1954 */
#line 1954
				ILI_ERR("Failed to allocate result_buf mem\n");
				ILI_KFREE(mp_items[index].result_buf);
				return -ENOMEM;
			}
		}

		/* "f8468e68 ldr"@0xffffff8008a65324 -- pre-index, +0x68 = 104 */
		if (!mp_items[index].max_buf) {
			mp_items[index].max_buf = kcalloc(core_mp.c256,
							  sizeof(int), GFP_KERNEL);
			if (IS_ERR(mp_items[index].max_buf) ||
			    mp_items[index].max_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate max_buf mem\n"
				 * @0xffffff8009243d23;
				 * "5280f562 mov"@0xffffff8008a6558c -- __LINE__ = 1963 */
#line 1963
				ILI_ERR("Failed to allocate max_buf mem\n");
				ILI_KFREE(mp_items[index].max_buf);
				return -ENOMEM;
			}
		}

		/* "f8470e68 ldr"@0xffffff8008a65360 -- pre-index, +0x70 = 112 */
		if (!mp_items[index].min_buf) {
			mp_items[index].min_buf = kcalloc(core_mp.c256,
							  sizeof(int), GFP_KERNEL);
			if (IS_ERR(mp_items[index].min_buf) ||
			    mp_items[index].min_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate min_buf mem\n"
				 * @0xffffff8009243d57;
				 * "5280f682 mov"@0xffffff8008a6560c -- __LINE__ = 1972 */
#line 1972
				ILI_ERR("Failed to allocate min_buf mem\n");
				ILI_KFREE(mp_items[index].min_buf);
				return -ENOMEM;
			}
		}

		/* "39405508 ldrb"@0xffffff8008a6539c + "7100051f cmp"@0xffffff8008a653a0 */
		if (mp_items[index].c21 == 1) {
			/* "f8478e68 ldr"@0xffffff8008a653b0 -- +0x78 = 120 */
			if (!mp_items[index].bench_mark_max) {
				mp_items[index].bench_mark_max = kcalloc(core_mp.c256,
									 sizeof(int),
									 GFP_KERNEL);
				if (IS_ERR(mp_items[index].bench_mark_max) ||
				    mp_items[index].bench_mark_max == NULL) {
					/* "\x013ILITEK: (%s, %d): Failed to allocate bench_mark_max mem\n"
					 * @0xffffff8009243d8b;
					 * "5280f7c2 mov"@0xffffff8008a656ec -- __LINE__ = 1982 */
#line 1982
					ILI_ERR("Failed to allocate bench_mark_max mem\n");
					ILI_KFREE(mp_items[index].bench_mark_max);
					return -ENOMEM;
				}
			}

			/* "f8480e68 ldr"@0xffffff8008a653c0 -- +0x80 = 128 */
			if (!mp_items[index].bench_mark_min) {
				mp_items[index].bench_mark_min = kcalloc(core_mp.c256,
									 sizeof(int),
									 GFP_KERNEL);
				if (IS_ERR(mp_items[index].bench_mark_min) ||
				    mp_items[index].bench_mark_min == NULL) {
					/* "\x013ILITEK: (%s, %d): Failed to allocate bench_mark_min mem\n"
					 * @0xffffff8009243dc6;
					 * "5280f8c2 mov"@0xffffff8008a65710 -- __LINE__ = 1990 */
#line 1990
					ILI_ERR("Failed to allocate bench_mark_min mem\n");
					ILI_KFREE(mp_items[index].bench_mark_min);
					return -ENOMEM;
				}
			}
		}
	}

	/* "2a1f03e0 mov"@0xffffff8008a653c8 */
	return 0;
}

/*
 * allnode_open_cdc_data() was reconstructed from the factory kernel disassembly (0xffffff8008a65af4, 1176 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#line 1581
static __used int allnode_open_cdc_data(int mode, int *buf)
{
	int i = 0, idx = 0, ret = 0, len = 0, tmp = 0;
	u8 cmd[15] = {0};
	char str[128] = {0};
	u8 *ori = NULL;
	char *key[] = {"open dac", "open raw1", "open raw2", "open raw3",
		       "open cap1 dac", "open cap1 raw"};

	len = core_mp.c232 * core_mp.c236 * 2 + 2;

	/* "\x016ILITEK: (%s, %d): Read X/Y Channel length = %d, mode = %d\n"
	 * @0xffffff80092444ae, __func__ =
	 * "allnode_open_cdc_data"@0xffffff80092444eb; the guard is
	 * "39656129 ldrb"@0xffffff8008a65b6c + "34000129 cbz"@0xffffff8008a65ba8;
	 * "5280c662 mov"@0xffffff8008a65bbc -- __LINE__ = 1587 */
#line 1587
	ILI_DBG("Read X/Y Channel length = %d, mode = %d\n", len, mode);

	/* "71000adf cmp"@0xffffff8008a65bcc + "5400012c b.gt"@0xffffff8008a65bd0 */
	if (len <= 2) {
		/* "\x013ILITEK: (%s, %d): Length is invalid\n"@0xffffff8009243e9d;
		 * "5280c6c2 mov"@0xffffff8008a65be4 -- __LINE__ = 1590 */
#line 1590
		ILI_ERR("Length is invalid\n");
		ret = -105;
		goto out;
	}

	/*
	 * "97fff0da bl"@0xffffff8008a65c20 towards <parser_get_ini_key_value> and
	 * "94101261 bl"@0xffffff8008a65c3c towards <snprintf>: this is the INLINING
	 * of `parser_get_int_data`, and its two consecutive `tbnz`s --
	 * "37f80055 tbnz"@0xffffff8008a65c40 and "37f81000 tbnz"@0xffffff8008a65c44
	 * -- are the expression `(ret < 0) ? 0 : size` tested against zero.
	 * "9138d442 add"@0xffffff8008a65c2c puts "%s"@0xffffff8009100e35 in x2,
	 * "910cb400 add"@0xffffff8008a65c14 "pv5_4 command"@0xffffff800924332d in
	 * x0 and "321903e1 orr"@0xffffff8008a65c38 the size 128 in w1
	 */
	if (parser_get_int_data("pv5_4 command", key[mode], str, sizeof(str)) < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to parse PV54 command, ret = %d\n"
		 * @0xffffff8009244501;
		 * "5280c7a2 mov"@0xffffff8008a65e54 -- __LINE__ = 1597 */
#line 1597
		ILI_ERR("Failed to parse PV54 command, ret = %d\n", ret);
		ret = -106;
		goto out;
	}

	/*
	 * "97ffff5f bl"@0xffffff8008a65c54, w2 = 15
	 * ("32000fe2 orr"@0xffffff8008a65c50). The value returned is NOT tested
	 */
	parser_get_u8_array(str, cmd, sizeof(cmd));

	/* "97ffd39a bl"@0xffffff8008a65c70 con
	 * "Open SP command"@0xffffff800924453d in x4 */
	ilitek_dump_data(cmd, 8, sizeof(cmd), 0, "Open SP command");

	/* "b902dd09 str"@0xffffff8008a65c88 -- `idev` + 732 = 1 */
	idev->c732 = 1;

	/*
	 * "b94be541 ldr"@0xffffff8008a65c90 reads 0xffffff800a0fcbe4, that is
	 * `core_mp.c228`; "f9418508 ldr"@0xffffff8008a65c8c takes `idev->c776`
	 * and "d63f0100 blr"@0xffffff8008a65c94 calls it
	 */
	if (idev->c776(cmd, core_mp.c228) < 0) {
		/* "\x013ILITEK: (%s, %d): Write CDC command failed\n"
		 * @0xffffff8009243f06;
		 * "5280c922 mov"@0xffffff8008a65eb4 -- __LINE__ = 1609 */
#line 1609
		ILI_ERR("Write CDC command failed\n");
		ret = -100;
		goto out;
	}

	/*
	 * "b94c6508 ldr"@0xffffff8008a65ca0 reads 0xffffff800a0fcc64
	 * (`core_mp.c356`), "7100051f cmp"@0xffffff8008a65ca4 compares it with 1
	 * and "540010e1 b.ne"@0xffffff8008a65ca8 picks the other branch
	 */
	if (core_mp.c356 == 1)
		/* "97ffce08 bl"@0xffffff8008a65cb4, w0 = w1 = 50 */
		ret = ilitek_tddi_ic_check_busy(50, 50);
	else
		/* "97ffcd41 bl"@0xffffff8008a65ec4, no arguments */
		ret = ilitek_tddi_ic_check_int_stat();

	if (ret < 0) {
		ret = -110;
		goto out;
	}

	/* "b94b1108 ldr"@0xffffff8008a65cc4 reads 0xffffff800a0fcb10
	 * (`core_mp.c16`); the constant 0x10401 is
	 * "52808029 mov"@0xffffff8008a65cc8 + "72a00029 movk"@0xffffff8008a65ccc */
	if ((unsigned int)core_mp.c16 <= 0x10401U) {
		/*
		 * "529e5ec9 mov"@0xffffff8008a65cdc (0xf2f6) +
		 * "790173e9 strh"@0xffffff8008a65ce0: TWO bytes in one go
		 */
		cmd[0] = 0xF6;
		cmd[1] = 0xF2;
		/* "321f03e1 orr"@0xffffff8008a65cec -- length 2 */
		if (idev->c776(cmd, 2) < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x, 0x%x) error\n"
			 * @0xffffff8009243f34, con
			 * "3942e3e3 ldrb"@0xffffff8008a65f08 e
			 * "3942e7e4 ldrb"@0xffffff8008a65f0c;
			 * "5280cbe2 mov"@0xffffff8008a65f20 -- __LINE__ = 1631 */
#line 1631
			ILI_ERR("Write (0x%x, 0x%x) error\n", cmd[0], cmd[1]);
			ret = -100;
			goto out;
		}
		/*
		 * "52912b00 mov"@0xffffff8008a65cfc + "72a00820 movk"@0xffffff8008a65d00
		 * = 0x418958 = 4295000 = 1000 * 0x10c7, that is `udelay(1000)`
		 */
		mdelay(1);

		/*
		 * "b24002f7 orr"@0xffffff8008a65d04 sets bit 0 of the
		 * pointer -- `sp+184` is eight-aligned, so it is
		 * `&cmd[1]`
		 */
		if (idev->c776(&cmd[1], 1) < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x) error\n"
			 * @0xffffff8009243f62, con
			 * "3942e7e3 ldrb"@0xffffff8008a65f64;
			 * "5280cd02 mov"@0xffffff8008a65f78 -- __LINE__ = 1640 */
#line 1640
			ILI_ERR("Write (0x%x) error\n", cmd[1]);
			ret = -108;
			goto out;
		}
		mdelay(1);
	}

	/*
	 * "52901801 mov"@0xffffff8008a65d30 + "72a02801 movk"@0xffffff8008a65d38
	 * = 0x14080c0, that is GFP_KERNEL | __GFP_ZERO: it is `kzalloc`, and the
	 * size is not constant, so the call goes to <__kmalloc>
	 * ("97dfa993 bl"@0xffffff8008a65d3c)
	 */
	ori = kzalloc(len, GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a65d44 + "54000c88 b.hi"@0xffffff8008a65d48
	 * (IS_ERR) e "b4000c77 cbz"@0xffffff8008a65d4c (NULL) */
	if (IS_ERR(ori) || ori == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate ori, (%ld)\n"
		 * @0xffffff8009243f8a, con "aa1703e3 mov"@0xffffff8008a65eec;
		 * "5280ce82 mov"@0xffffff8008a65ee8 -- __LINE__ = 1652 */
#line 1652
		ILI_ERR("Failed to allocate ori, (%ld)\n", PTR_ERR(ori));
		ret = -107;
		goto out;
	}

	/*
	 * "f9418908 ldr"@0xffffff8008a65d5c takes `idev->c784` and
	 * "d63f0100 blr"@0xffffff8008a65d60 calls it with w1 = len
	 */
	if (idev->c784(ori, len) < 0) {
		/* "\x013ILITEK: (%s, %d): Read cdc data error, len = %d\n"
		 * @0xffffff8009243fbd;
		 * "5280cf62 mov"@0xffffff8008a65f40 -- __LINE__ = 1659 */
#line 1659
		ILI_ERR("Read cdc data error, len = %d\n", len);
		ret = -108;
		goto out;
	}

	/* "97ffd356 bl"@0xffffff8008a65d80 con
	 * "Open SP CDC original"@0xffffff800924454d */
	ilitek_dump_data(ori, 8, len, 0, "Open SP CDC original");

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a65df4).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	for (i = 0; i < core_mp.c256; i++) {
		idx = (2 * i) + 1;

		/* "121d7a8a and"@0xffffff8008a65d98 (mode & ~4) +
		 * "340000ea cbz"@0xffffff8008a65db4: is `mode == 0 || mode == 4`
		 * folded into a single test */
		if (mode == 0 || mode == 4) {
			/*
			 * "120019d0 and"@0xffffff8008a65dd4 +
			 * "721901df tst"@0xffffff8008a65dd8 +
			 * "5a90060e cneg"@0xffffff8008a65ddc, and the twin
			 * pair for the second byte
			 */
			tmp = ((ori[idx] & 0x80) ?
			       -(ori[idx] & 0x7F) :
			       (ori[idx] & 0x7F)) +
			      ((ori[idx + 1] & 0x80) ?
			       -(ori[idx + 1] & 0x7F) :
			       (ori[idx + 1] & 0x7F));
		} else {
			/* "33181dcf bfi"@0xffffff8008a65dc0 merges the two bytes */
			tmp = (ori[idx] << 8) + ori[idx + 1];
			/*
			 * "32103df0 orr"@0xffffff8008a65dc4 (| 0xffff0000) and
			 * "1a9001ee csel"@0xffffff8008a65dc8: subtracting 65536
			 * from a value that fits in sixteen bits IS an `orr`,
			 * and the test is on the RAW HIGH BYTE, not on bit 15 of
			 * the merged value: "721901df tst"@0xffffff8008a65dbc tests
			 * w14 (that is, `ori[2i+1]`) and comes BEFORE the `bfi`.
			 * Written as `tmp >= 0x8000` the same condition becomes
			 * `lsr w16,w15,#15` + `cmp` + `csel`, one instruction more
			 */
			if (ori[idx] & 0x80)
				tmp -= 0x10000;
		}
		/*
		 * A SINGLE `str` for both branches
		 * ("b8297a6e str"@0xffffff8008a65df0): the value goes through a
		 * temporary variable, not through `buf[i]`. Written with
		 * `buf[i]` as the accumulator the `str`s become TWO, because the
		 * compiler cannot rule out that `buf` points inside `core_mp`
		 */
		buf[i] = tmp;
	}

	/* "97ffd32d bl"@0xffffff8008a65e24 con
	 * "Open SP CDC combined"@0xffffff8009244562, w1 = 10
	 * ("52800141 mov"@0xffffff8008a65e1c) e w3 = `core_mp.c232`
	 * ("b94be903 ldr"@0xffffff8008a65e10) */
	ilitek_dump_data(buf, 10, core_mp.c256, core_mp.c232,
			 "Open SP CDC combined");

out:
	/* "b902dd1f str"@0xffffff8008a65e6c e "b902dd1f str"@0xffffff8008a65efc */
	idev->c732 = 0;
	/*
	 * "b50002d7 cbnz"@0xffffff8008a65f00 + "97dfaa15 bl"@0xffffff8008a65f5c
	 * towards <kfree>. On the six paths where `ori` is still NULL the free
	 * does not appear at all, and that is why there are TWO exits:
	 * 0xffffff8008a65e64 without and 0xffffff8008a65ef8 with
	 */
	ipio_kfree((void **)&ori);
	return ret;
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0ff410).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_open_para {
	int tvch;
	int tvcl;
	int gain;
};
struct ilitek_open_para open_para;

/*
 * allnode_open_cdc_result() was reconstructed from the factory kernel disassembly (0xffffff8008a5e150, 984 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void allnode_open_cdc_result(int index, int *buf, int *dac, int *raw)
{
	int i;
	int len = strlen(mp_items[index].c0);

	if (len == (int)strlen("open test(integration)_sp") &&
	    strncmp(mp_items[index].c0, "open test(integration)_sp",
		    strlen(mp_items[index].c0)) == 0) {
		for (i = 0; i < core_mp.c256; i++) {
			buf[i] = (dac[i] *
				  ((core_mp.c4 == 0x9881) ? 1610000 : 1310000) /
				  100 -
				  (8192 - raw[i]) * 140000 / 16384 * 36 / 10) /
				 (open_para.tvch - open_para.tvcl) / 2;
		}
	} else if (len == (int)strlen("open test_c") &&
		   strncmp(mp_items[index].c0, "open test_c",
			   strlen(mp_items[index].c0)) == 0) {
		for (i = 0; i < core_mp.c256; i++) {
			char str[32] = {0};
			int accuracy = 0;
			int rawv = raw[i];
			int dacv = dac[i];
			int vdiff = open_para.tvch - open_para.tvcl;
			int gain = open_para.gain;
			int c4 = core_mp.c4;
			int c6 = core_mp.c6;

			if (c4 == 0x9881) {
				buf[i] = (dacv * 16146 / 2 +
					  (rawv - 8192) * 244080 / 16384) /
					 vdiff / 100 / gain;
			} else {
				parser_get_int_data("open test_c", "accuracy",
						    str, sizeof(str));
				accuracy = katoi(str);

				if (c6 == 27) {
					if (accuracy)
						buf[i] = (dacv * 14664 / 2 *
							  10 +
							  (rawv - 8192) *
							  158625 / 1024) /
							 vdiff / 100 / gain;
					else
						buf[i] = (dacv * 14664 / 2 +
							  (rawv - 8192) *
							  253800 / 16384) /
							 vdiff / 100 / gain;
				} else {
					if (accuracy)
						buf[i] = (dacv * 13143 / 2 *
							  10 +
							  (rawv - 8192) *
							  152550 / 1024) /
							 vdiff / 100 / gain;
					else
						buf[i] = (dacv * 16146 / 2 +
							  (rawv - 8192) *
							  244080 / 16384) /
							 vdiff / 100 / gain;
				}
			}
		}
	}
}

/*
 * open_test_sp() was reconstructed from the factory kernel disassembly (0xffffff8008a632bc, 3932 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int open_test_sp(int index)
{
	int i;
	int j;
	int k;
	int x;
	int y;
	int ret = 0;
	int full_open_rate = 0;
	int charge_aa = 0;
	int charge_border = 0;
	int charge_notch = 0;
	int frame_count = mp_items[index].c48;
	int *cdc[frame_count][9];
	char str[512] = {0};

#line 2226
	ILI_DBG("index = %d, desp = %s, Frame Count = %d\n", index, mp_items[index].c0, mp_items[index].c48);

	if (mp_items[index].c48 < 1) {
#line 2233
		ILI_ERR("Frame count is zero, which is at least set as 1\n");
		mp_items[index].c48 = 1;
	}

	if (create_mp_test_frame_buffer(index, mp_items[index].c48) < 0) {
		ret = -107;
		goto out;
	}

	if (frame1_cbk700 == NULL) {
		frame1_cbk700 = kcalloc(core_mp.c256, sizeof(int),
					  GFP_KERNEL);
		if (IS_ERR(frame1_cbk700) || frame1_cbk700 == NULL) {
#line 2246
			ILI_ERR("Failed to allocate frame1_cbk700 buffer\n");
			return -107;
		}
	} else {
		memset(frame1_cbk700, 0, core_mp.c256);
	}

	if (frame1_cbk250 == NULL) {
		frame1_cbk250 = kcalloc(core_mp.c256, sizeof(int),
					  GFP_KERNEL);
		if (IS_ERR(frame1_cbk250) || frame1_cbk250 == NULL) {
#line 2256
			ILI_ERR("Failed to allocate frame1_cbk250 buffer\n");
			ipio_kfree((void **)&frame1_cbk700);
			return -107;
		}
	} else {
		memset(frame1_cbk250, 0, core_mp.c256);
	}

	if (frame1_cbk200 == NULL) {
		frame1_cbk200 = kcalloc(core_mp.c256, sizeof(int),
					  GFP_KERNEL);
		if (IS_ERR(frame1_cbk200) || frame1_cbk200 == NULL) {
#line 2267
			ILI_ERR("Failed to allocate cbk buffer\n");
			ipio_kfree((void **)&frame1_cbk700);
			ipio_kfree((void **)&frame1_cbk250);
			return -107;
		}
	} else {
		memset(frame1_cbk200, 0, core_mp.c256);
	}

	mp_items[index].c136 = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	if (IS_ERR(mp_items[index].c136) || mp_items[index].c136 == NULL) {
#line 2278
		ILI_ERR("Failed to allocate node_type FRAME buffer\n");
		return -107;
	}

	for (i = 0; i < core_mp.c236; i++) {
		for (j = 0; j < core_mp.c232; j++) {
			mp_items[index].max_buf[i * core_mp.c232 + j] = INT_MIN;
			mp_items[index].min_buf[i * core_mp.c232 + j] = INT_MAX;
		}
	}

	if (mp_items[index].c21 == 1) {
		parser_ini_benchmark(mp_items[index].bench_mark_max,
				     mp_items[index].bench_mark_min,
				     mp_items[index].c22, mp_items[index].c0,
				     core_mp.c256);
		dump_benchmark_data(mp_items[index].bench_mark_max,
				    mp_items[index].bench_mark_min);
	}

	parser_ini_nodetype(mp_items[index].c136, "full open", core_mp.c256);
	dump_node_type_buffer(mp_items[index].c136);

	parser_get_int_data(mp_items[index].c0, "charge_aa", str, 512);
	charge_aa = katoi(str);
	parser_get_int_data(mp_items[index].c0, "charge_border", str, 512);
	charge_border = katoi(str);
	parser_get_int_data(mp_items[index].c0, "charge_notch", str, 512);
	charge_notch = katoi(str);
	parser_get_int_data(mp_items[index].c0, "full open", str, 512);
	full_open_rate = katoi(str);
	parser_get_int_data(mp_items[index].c0, "tvch", str, 512);
	open_para.tvch = katoi(str);
	ret = parser_get_int_data(mp_items[index].c0, "tvcl", str, 512);
	open_para.tvcl = katoi(str);

	if (ret < 0) {
#line 2324
		ILI_ERR("Failed to get parameters from ini file\n");
		ret = -106;
		goto out;
	}

#line 2330
	ILI_DBG("open_test_sp: frame_cont %d, AA %d, Border %d, Notch %d, full_open_rate %d\n", mp_items[index].c48, charge_aa, charge_border, charge_notch, full_open_rate);

	/*
	 * the limit is RE-READ on every iteration: "b9800368 ldrsw"@0xffffff8008a63c4c
	 * and "eb08029f cmp"@0xffffff8008a63c5c. None of the nine allocations
	 * is checked -- in the factory build there is no `cbz` between one
	 * "97dfb21e bl"@0xffffff8008a63b10 and the next (rule 7)
	 */
	for (i = 0; i < mp_items[index].c48; i++) {
		cdc[i][0] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][1] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][2] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][3] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][4] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][5] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][6] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][7] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][8] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	}

	/*
	 * "9b0b6355 madd"@0xffffff8008a63ca8 computes the row (index * 0x48) and
	 * "f8440ea1 ldr"@0xffffff8008a63cac reads column 8 with the pre-index
	 */
	for (i = 0; i < mp_items[index].c48; i++) {
		ret = allnode_open_cdc_data(0, cdc[i][8]);
		if (ret < 0) {
#line 2347
			ILI_ERR("Failed to get Open SP DAC data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(1, cdc[i][0]);
		if (ret < 0) {
#line 2352
			ILI_ERR("Failed to get Open SP Raw1 data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(2, cdc[i][1]);
		if (ret < 0) {
#line 2357
			ILI_ERR("Failed to get Open SP Raw2 data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(3, cdc[i][2]);
		if (ret < 0) {
#line 2362
			ILI_ERR("Failed to get Open SP Raw3 data, %d\n", ret);
			goto out;
		}

		allnode_open_cdc_result(index, cdc[i][3], cdc[i][8], cdc[i][0]);
		allnode_open_cdc_result(index, cdc[i][4], cdc[i][8], cdc[i][1]);
		allnode_open_cdc_result(index, cdc[i][5], cdc[i][8], cdc[i][2]);

		if (i == 0) {
			memcpy(frame1_cbk700, cdc[i][3], core_mp.c256 * 4);
			memcpy(frame1_cbk250, cdc[i][4], core_mp.c256 * 4);
			memcpy(frame1_cbk200, cdc[i][5], core_mp.c256 * 4);
		}

		ilitek_dump_data(cdc[i][3], 10, core_mp.c256, core_mp.c232,
				 "cbk 700");
		ilitek_dump_data(cdc[i][4], 10, core_mp.c256, core_mp.c232,
				 "cbk 250");
		ilitek_dump_data(cdc[i][5], 10, core_mp.c256, core_mp.c232,
				 "cbk 200");

		/*
		 * "b94006ec ldr"@0xffffff8008a63e24 reads `core_mp.c236` and
		 * "5400054d b.le"@0xffffff8008a63e2c skips the loop: the
		 * comparison is `<= 0`, that is SIGNED. The three pointers are
		 * re-read on every iteration ("f940030c ldr"@0xffffff8008a63e68 and
		 * "f94002ae ldr"@0xffffff8008a63e6c)
		 */
		for (j = 0, k = 0; j < core_mp.c236; j++) {
			for (x = 0; x < core_mp.c232; x++) {
				cdc[i][6][k] = cdc[i][4][k] * 100 /
					       cdc[i][3][k];
				cdc[i][7][k] = cdc[i][3][k] - cdc[i][5][k];
				k++;
			}
		}

		ilitek_dump_data(cdc[i][6], 10, core_mp.c256, core_mp.c232,
				 "origin charge rate");
		ilitek_dump_data(cdc[i][7], 10, core_mp.c256, core_mp.c232,
				 "origin full open");

		/*
		 * `full_open_rate_compare` inlined: the `type` passed is
		 * read with the running index
		 * ("b86bd98c ldr"@0xffffff8008a63f58) while the function
		 * recomputes its own index with
		 * "1b03290e madd"@0xffffff8008a63f68 (c232 * y + x). The two
		 * writes of the zero branch are
		 * "b82cd9bf str"@0xffffff8008a63fac (buf) and
		 * "b82b799f str"@0xffffff8008a63fb4 (column 6)
		 */
		for (y = 0, k = 0; y < core_mp.c236; y++) {
			for (x = 0; x < core_mp.c232; x++) {
				if (full_open_rate_compare(cdc[i][7],
							   cdc[i][3], x, y,
							   mp_items[index].c136[k],
							   full_open_rate) == 0) {
					mp_items[index].buf[i * core_mp.c256 + k] = 0;
					cdc[i][6][k] = 0;
				}
				k++;
			}
		}

		ilitek_dump_data(&mp_items[index].buf[i * core_mp.c256], 10,
				 core_mp.c256, core_mp.c232,
				 "after full_open_rate_compare");

		/*
		 * `compare_charge` inlined: the three `tbnz`/`tbz` on bits 0, 1
		 * and 2 of the type -- "370000af tbnz"@0xffffff8008a64058,
		 * "3708006f tbnz"@0xffffff8008a64060 and
		 * "3610038f tbz"@0xffffff8008a64068 -- pick between the three
		 * threshold values in the order aa, border, notch, and
		 * "f100221f cmp"@0xffffff8008a640d0 closes the eight rounds over the
		 * neighbours
		 */
		for (y = 0, k = 0; y < core_mp.c236; y++) {
			for (x = 0; x < core_mp.c232; x++) {
				mp_items[index].buf[i * core_mp.c256 + k] =
					compare_charge(cdc[i][6], x, y,
						       mp_items[index].c136,
						       charge_aa,
						       charge_border,
						       charge_notch);
				k++;
			}
		}

		ilitek_dump_data(&mp_items[index].buf[i * core_mp.c256], 10,
				 core_mp.c256, core_mp.c232,
				 "after compare charge rate");

		compare_MaxMin_result(index,
				      &mp_items[index].buf[i * core_mp.c256]);
	}

out:
	/* "f8488e80 ldr"@0xffffff8008a635f4 (pre-index at +136),
	 * "97dfb46d bl"@0xffffff8008a635fc to <kfree>,
	 * "f900029f str"@0xffffff8008a63600 */
	ipio_kfree((void **)&mp_items[index].c136);
	for (i = 0; i < mp_items[index].c48; i++) {
		ipio_kfree((void **)&cdc[i][0]);
		ipio_kfree((void **)&cdc[i][1]);
		ipio_kfree((void **)&cdc[i][2]);
		ipio_kfree((void **)&cdc[i][3]);
		ipio_kfree((void **)&cdc[i][4]);
		ipio_kfree((void **)&cdc[i][5]);
		ipio_kfree((void **)&cdc[i][6]);
		ipio_kfree((void **)&cdc[i][7]);
		ipio_kfree((void **)&cdc[i][8]);
	}
	return ret;
}

/*
 * open_test_cap() was reconstructed from the factory kernel disassembly (0xffffff8008a64218, 1828 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int open_test_cap(int index)
{
	int i;
	int k;
	int j;
	int ret = 0;
	int frame_count = mp_items[index].c48;
	int *cdc[frame_count][3];
	char str[512] = {0};

	if (frame_count < 1) {
#line 2443
		ILI_ERR("Frame count is zero, which is at least set as 1\n");
		frame_count = 1;
		mp_items[index].c48 = frame_count;
	}

	if (create_mp_test_frame_buffer(index, frame_count) < 0) {
		ret = -107;
		goto out;
	}

	if (cap_dac == NULL) {
		cap_dac = kcalloc(core_mp.c256, sizeof(int),
					  GFP_KERNEL);
		if (IS_ERR(cap_dac) || cap_dac == NULL) {
#line 2455
			ILI_ERR("Failed to allocate cap_dac buffer\n");
			return -107;
		}
	} else {
		memset(cap_dac, 0, core_mp.c256);
	}

	if (cap_raw == NULL) {
		cap_raw = kcalloc(core_mp.c256, sizeof(int),
					  GFP_KERNEL);
		if (IS_ERR(cap_raw) || cap_raw == NULL) {
#line 2465
			ILI_ERR("Failed to allocate cap_raw buffer\n");
			ipio_kfree((void **)&cap_dac);
			return -107;
		}
	} else {
		memset(cap_raw, 0, core_mp.c256);
	}

	for (i = 0; i < core_mp.c236; i++) {
		for (j = 0; j < core_mp.c232; j++) {
			mp_items[index].max_buf[i * core_mp.c232 + j] = INT_MIN;
			mp_items[index].min_buf[i * core_mp.c232 + j] = INT_MAX;
		}
	}

	if (mp_items[index].c21 == 1) {
		parser_ini_benchmark(mp_items[index].bench_mark_max,
				     mp_items[index].bench_mark_min,
				     mp_items[index].c22, mp_items[index].c0,
				     core_mp.c256);
		dump_benchmark_data(mp_items[index].bench_mark_max,
				    mp_items[index].bench_mark_min);
	}

	parser_get_int_data(mp_items[index].c0, "gain", str, 512);
	open_para.gain = katoi(str);
	parser_get_int_data(mp_items[index].c0, "tvch", str, 512);
	open_para.tvch = katoi(str);
	ret = parser_get_int_data(mp_items[index].c0, "tvcl", str, 512);
	open_para.tvcl = katoi(str);
	if (ret < 0) {
#line 2500
		ILI_ERR("Failed to get parameters from ini file\n");
		ret = -106;
		goto out;
	}

#line 2506
	ILI_DBG("open_test_c: frame_cont = %d, gain = %d, tvch = %d, tvcl = %d\n", mp_items[index].c48, open_para.gain, open_para.tvch, open_para.tvcl);

	for (i = 0; i < mp_items[index].c48; i++) {
		cdc[i][0] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][1] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][2] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	}

	for (i = 0; i < mp_items[index].c48; i++) {
		ret = allnode_open_cdc_data(4, cdc[i][0]);
		if (ret < 0) {
#line 2517
			ILI_ERR("Failed to get Open CAP DAC data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(5, cdc[i][1]);
		if (ret < 0) {
#line 2522
			ILI_ERR("Failed to get Open CAP RAW data, %d\n", ret);
			goto out;
		}
		allnode_open_cdc_result(index, cdc[i][2], cdc[i][0],
					cdc[i][1]);

		if (i == 0) {
			memcpy(cap_dac, cdc[i][0],
			       core_mp.c256 * 4);
			memcpy(cap_raw, cdc[i][1],
			       core_mp.c256 * 4);
		}

		ilitek_dump_data(cdc[i][2], 10, core_mp.c256, core_mp.c232,
				 "DCL_Cap");

		for (j = 0, k = 0; j < core_mp.c236; j++) {
			int m;

			for (m = 0; m < core_mp.c232; m++) {
				mp_items[index].buf[i * core_mp.c256 + k] =
					cdc[i][2][k];
				k++;
			}
		}

		compare_MaxMin_result(index,
				      &mp_items[index].buf[i * core_mp.c256]);
	}

out:
	for (i = 0; i < mp_items[index].c48; i++) {
		ipio_kfree((void **)&cdc[i][0]);
		ipio_kfree((void **)&cdc[i][1]);
		ipio_kfree((void **)&cdc[i][2]);
	}
	return ret;
}

/*
 * allnode_key_cdc_data() was reconstructed from the factory kernel disassembly (0xffffff8009244769).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int allnode_key_cdc_data(int index)
{
	int i = 0, idx = 0, ret = 0, len = 0;
	u8 cmd[3] = {0};
	u8 *ori = NULL;

	/*
	 * "b94bfaa8 ldr"@0xffffff8008a64a10 (core_mp.key_len, 0xffffff800a0fcbf8)
	 * and "531f791b lsl"@0xffffff8008a64a30 (by two). The read sits at the top
	 * of the caller's loop body -- "b94bfaa8 ldr"@0xffffff8008a64d1c does it
	 * again on every iteration -- so in the source it is inside the function,
	 * not hoisted before the loop (class A2 the other way round)
	 */
	len = core_mp.key_len * 2;

	/* "\x016ILITEK: (%s, %d): Read key's length = %d\n"@0xffffff800924473d,
	 * __func__ = "allnode_key_cdc_data"@0xffffff8009244769; the guard is
	 * "396562e9 ldrb"@0xffffff8008a64a2c + "34000229 cbz"@0xffffff8008a64a3c;
	 * "5280b042 mov"@0xffffff8008a64a48 -- __LINE__ = 1410 */
#line 1410
	ILI_DBG("Read key's length = %d\n", len);
	/*
	 * "\x016ILITEK: (%s, %d): core_mp.key_len = %d\n"@0xffffff800924477e --
	 * this is the format that NAMES the field, that is, the exception to rule 5.
	 * The argument is "b94bfaa3 ldr"@0xffffff8008a64a64, a RE-READ of the
	 * field; the second guard is "396562e8 ldrb"@0xffffff8008a64a5c +
	 * "34000108 cbz"@0xffffff8008a64a60;
	 * "5280b062 mov"@0xffffff8008a64a70 -- __LINE__ = 1411
	 */
#line 1411
	ILI_DBG("core_mp.key_len = %d\n", core_mp.key_len);

	/*
	 * "7100037f cmp"@0xffffff8008a64a80 + "5400196d b.le"@0xffffff8008a64a84:
	 * the comparison is SIGNED and against zero, not against 2 as in its twin
	 */
	if (len <= 0) {
		/* "\x013ILITEK: (%s, %d): Length is invalid\n"@0xffffff8009243e9d;
		 * "5280b0c2 mov"@0xffffff8008a64dc0 -- __LINE__ = 1414 */
#line 1414
		ILI_ERR("Length is invalid\n");
		/* "12800016 mov"@0xffffff8008a64dc8 */
		ret = -1;
		goto out;
	}

	/* "52801e2a mov"@0xffffff8008a64a90 (0xf1) +
	 * "390093ea strb"@0xffffff8008a64a94 */
	cmd[0] = 0xF1;
	/* "39400268 ldrb"@0xffffff8008a64a88 (mp_items[index].c20, one byte) +
	 * "390097e8 strb"@0xffffff8008a64a98 */
	cmd[1] = mp_items[index].c20;
	/* "39009bff strb"@0xffffff8008a64aa0 */
	cmd[2] = 0x00;

	/* "320003e8 orr"@0xffffff8008a64a9c + "b902dd28 str"@0xffffff8008a64aa4 */
	idev->c732 = 1;

	/* "f9418528 ldr"@0xffffff8008a64aa8 (idev->c776),
	 * "320007e1 orr"@0xffffff8008a64ab0 (w1 = 3) e
	 * "d63f0100 blr"@0xffffff8008a64ab4 */
	ret = idev->c776(cmd, sizeof(cmd));
	if (ret < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): Write CDC command failed\n"
		 * @0xffffff8009243f06; "5280b282 mov"@0xffffff8008a64de4 --
		 * __LINE__ = 1428. The value returned is the call's:
		 * "2a0003f6 mov"@0xffffff8008a64dd0
		 */
#line 1428
		ILI_ERR("Write CDC command failed\n");
		goto out;
	}

	/* "b94c6508 ldr"@0xffffff8008a64ac0 (core_mp.c356, 0xffffff800a0fcc64) +
	 * "7100051f cmp"@0xffffff8008a64ac4 + "540000e1 b.ne"@0xffffff8008a64ac8 */
	if (core_mp.c356 == 1)
		/* "52800640 mov"@0xffffff8008a64acc + "52800641 mov"@0xffffff8008a64ad0
		 * (w0 = w1 = 50) + "97ffd280 bl"@0xffffff8008a64ad4 */
		ret = ilitek_tddi_ic_check_busy(50, 50);
	else
		/* "97ffd239 bl"@0xffffff8008a64ae4, no arguments */
		ret = ilitek_tddi_ic_check_int_stat();

	/*
	 * "36f800a0 tbz"@0xffffff8008a64adc and "37f81a40 tbnz"@0xffffff8008a64aec.
	 * There is NO rewrite of `ret` here: its twin
	 * `allnode_open_cdc_data` writes -110, this one returns the value it got
	 */
	if (ret < 0)
		goto out;

	/* "b94b1108 ldr"@0xffffff8008a64af4 (core_mp.c16, 0xffffff800a0fcb10),
	 * "52808029 mov"@0xffffff8008a64af8 + "72a00029 movk"@0xffffff8008a64afc
	 * (0x10401), "6b09011f cmp"@0xffffff8008a64b00 +
	 * "540002c8 b.hi"@0xffffff8008a64b04 (an UNSIGNED comparison) */
	if ((unsigned int)core_mp.c16 <= 0x10401U) {
		/*
		 * "529e5ec9 mov"@0xffffff8008a64b0c (0xf2f6) +
		 * "79004be9 strh"@0xffffff8008a64b10: TWO bytes in one go
		 */
		cmd[0] = 0xF6;
		cmd[1] = 0xF2;
		/* "321f03e1 orr"@0xffffff8008a64b1c -- length 2 */
		ret = idev->c776(cmd, 2);
		if (ret < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x, 0x%x) error\n"
			 * @0xffffff8009243f34, con
			 * "394093e3 ldrb"@0xffffff8008a64df0 e
			 * "394097e4 ldrb"@0xffffff8008a64df4;
			 * "5280b502 mov"@0xffffff8008a64e0c -- __LINE__ = 1448 */
#line 1448
			ILI_ERR("Write (0x%x, 0x%x) error\n", cmd[0], cmd[1]);
			goto out;
		}
		/* "52912b00 mov"@0xffffff8008a64b28 +
		 * "72a00820 movk"@0xffffff8008a64b2c = 0x418958 = 1000 * 0x10c7 */
		mdelay(1);

		/*
		 * "b2400128 orr"@0xffffff8008a64a08 sets bit 0 of the
		 * pointer -- `sp+36` is four-aligned, so it is
		 * `&cmd[1]` -- and "f9000be8 str"@0xffffff8008a64a0c saves it
		 * outside the loop; "f9400be0 ldr"@0xffffff8008a64b38 re-reads it
		 */
		ret = idev->c776(&cmd[1], 1);
		if (ret < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x) error\n"
			 * @0xffffff8009243f62, con
			 * "394097e3 ldrb"@0xffffff8008a64e18;
			 * "5280b622 mov"@0xffffff8008a64e2c -- __LINE__ = 1457 */
#line 1457
			ILI_ERR("Write (0x%x) error\n", cmd[1]);
			goto out;
		}
		mdelay(1);
	}

	/*
	 * "93407f60 sxtw"@0xffffff8008a64b60 (the length is a signed `int`),
	 * "52901801 mov"@0xffffff8008a64b5c +
	 * "72a02801 movk"@0xffffff8008a64b64 = 0x14080c0 = GFP_KERNEL |
	 * __GFP_ZERO, "97dfae08 bl"@0xffffff8008a64b68 towards <__kmalloc>
	 */
	ori = kzalloc(len, GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a64b70 + "540007a8 b.hi"@0xffffff8008a64b74
	 * (IS_ERR) e "b4000797 cbz"@0xffffff8008a64b78 (NULL) */
	if (IS_ERR(ori) || ori == NULL) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to allocate ori mem (%ld)\n"
		 * @0xffffff80092447a8, with "aa1703e3 mov"@0xffffff8008a64c7c;
		 * "5280b782 mov"@0xffffff8008a64c70 -- __LINE__ = 1468.
		 * FACTORY DEFECT D1: `ret` is NOT touched here
		 */
#line 1468
		ILI_ERR("Failed to allocate ori mem (%ld)\n", PTR_ERR(ori));
		goto out;
	}

	/* "f9418908 ldr"@0xffffff8008a64b88 (idev->c784) +
	 * "d63f0100 blr"@0xffffff8008a64b8c, w1 = len
	 * ("2a1b03e1 mov"@0xffffff8008a64b84) */
	ret = idev->c784(ori, len);
	if (ret < 0) {
		/* "\x013ILITEK: (%s, %d): Read cdc data error, len = %d\n"
		 * @0xffffff8009243fbd; "5280b862 mov"@0xffffff8008a64c9c --
		 * __LINE__ = 1475 */
#line 1475
		ILI_ERR("Read cdc data error, len = %d\n", len);
		goto out;
	}

	/* "321d03e1 orr"@0xffffff8008a64b9c (w1 = 8),
	 * "911f7884 add"@0xffffff8008a64bac ("Key CDC original"
	 * @0xffffff80092447de) e "97ffd7ca bl"@0xffffff8008a64bb0 */
	ilitek_dump_data(ori, 8, len, 0, "Key CDC original");

	/* "f9421720 ldr"@0xffffff8008a64bb4 + "b40007e0 cbz"@0xffffff8008a64bb8 */
	if (mp_frame_buf1 == NULL) {
		/*
		 * the shape of `kcalloc`: "b98bfaa8 ldrsw"@0xffffff8008a64cb4
		 * (SIGNED), "37f80168 tbnz"@0xffffff8008a64cbc (the overflow
		 * check of `kmalloc_array`),
		 * "d37ef500 lsl"@0xffffff8008a64cc4 (times four),
		 * "97dfadaf bl"@0xffffff8008a64ccc towards <__kmalloc> and
		 * "f9021720 str"@0xffffff8008a64cd8
		 */
		mp_frame_buf1 = kcalloc(core_mp.key_len, sizeof(int),
					GFP_KERNEL);
		/* "b140041f cmn"@0xffffff8008a64cd4 +
		 * "540000a8 b.hi"@0xffffff8008a64cdc e
		 * "b5fff763 cbnz"@0xffffff8008a64ce0 */
		if (IS_ERR(mp_frame_buf1) || mp_frame_buf1 == NULL) {
			/*
			 * "\x013ILITEK: (%s, %d): Failed to allocate FrameBuffer mem (%ld)\n"
			 * @0xffffff8009244004; "5280b982 mov"@0xffffff8008a64cf8
			 * -- __LINE__ = 1484. Here too `ret` is NOT touched
			 */
#line 1484
			ILI_ERR("Failed to allocate FrameBuffer mem (%ld)\n", PTR_ERR(mp_frame_buf1));
			goto out;
		}
	} else {
		/*
		 * DEFECT D2: "b98bfaa2 ldrsw"@0xffffff8008a64bbc passes the
		 * number of ELEMENTS as the number of BYTES
		 */
		memset(mp_frame_buf1, 0, core_mp.key_len);
	}

	/*
	 * DEFECT D3: the guard is `core_mp.c256`
	 * ("b94c0362 ldr"@0xffffff8008a64bcc, re-read at the end of the round with
	 * "b94c0362 ldr"@0xffffff8008a64c30), not `core_mp.key_len`.
	 *
	 * THE TWO INDEX CHAINS ARE DISTINCT, as in the twin: x9 starts from
	 * 0x100000000 ("b26003e9 orr"@0xffffff8008a64bdc) and x10 from 0x200000000
	 * ("b25f03ea orr"@0xffffff8008a64be0), both with stride 0x200000000
	 * ("8b140129 add"@0xffffff8008a64c38 and "8b14014a add"@0xffffff8008a64c40),
	 * and the two indices come out of "9360fd2b asr"@0xffffff8008a64bf0 and
	 * "9360fd4c asr"@0xffffff8008a64bf8
	 */
	for (i = 0; i < core_mp.c256; i++) {
		idx = (2 * i) + 1;

		/* "3940026b ldrb"@0xffffff8008a64be4 +
		 * "7100517f cmp"@0xffffff8008a64be8 (0x14 = 20) +
		 * "54000241 b.ne"@0xffffff8008a64bec */
		if (mp_items[index].c20 == 20)
			/*
			 * "1200196d and"@0xffffff8008a64c00 +
			 * "7219017f tst"@0xffffff8008a64c04 +
			 * "5a8d05ad cneg"@0xffffff8008a64c0c for the first byte
			 * and "1200198b and"@0xffffff8008a64c08 +
			 * "7219019f tst"@0xffffff8008a64c10 +
			 * "5a8b056b cneg"@0xffffff8008a64c14 for the second;
			 * the SIGNED division by two is
			 * "7100017f cmp"@0xffffff8008a64c20 +
			 * "1a8ba56b cinc"@0xffffff8008a64c24 +
			 * "13017d6b asr"@0xffffff8008a64c28, and the final `str`
			 * is "b828798b str"@0xffffff8008a64c2c
			 */
			mp_frame_buf1[i] = (((ori[idx] & 0x80) ? -(ori[idx] & 0x7F) : (ori[idx] & 0x7F)) +
					    ((ori[idx + 1] & 0x80) ? -(ori[idx + 1] & 0x7F) : (ori[idx + 1] & 0x7F))) / 2;
	}

	/* "321b03e1 orr"@0xffffff8008a64c58 (w1 = 32),
	 * "b94be903 ldr"@0xffffff8008a64c50 (core_mp.c232) e
	 * "911fbc84 add"@0xffffff8008a64c5c ("Key CDC combined data"
	 * @0xffffff80092447ef) */
	ilitek_dump_data(mp_frame_buf1, 32, core_mp.c256, core_mp.c232,
			 "Key CDC combined data");

out:
	/*
	 * "b902dd1f str"@0xffffff8008a64c88, "b902dd1f str"@0xffffff8008a64d0c and
	 * "b902dd1f str"@0xffffff8008a64e38: THREE copies, one per group of
	 * paths
	 */
	idev->c732 = 0;
	/*
	 * "b5000437 cbnz"@0xffffff8008a64c8c + "97dfaea7 bl"@0xffffff8008a64d14
	 * towards <kfree>. On the paths where `ori` is still NULL the
	 * free does not appear at all
	 */
	ipio_kfree((void **)&ori);
	return ret;
}

/*
 * key_test() was reconstructed from the factory kernel disassembly (0xffffff8008a6493c, 1368 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int key_test(int index)
{
	int i = 0, j = 0, ret = 0;

	/*
	 * "\x016ILITEK: (%s, %d): Item = %s, Frame Count = %d\n"
	 * @0xffffff80092446c0, __func__ = "key_test"@0xffffff80092446f1; the
	 * guard is "396562e8 ldrb"@0xffffff8008a64974 +
	 * "340001a8 cbz"@0xffffff8008a64978; the two arguments are
	 * "f9400103 ldr"@0xffffff8008a6498c (the name, field at +0) and
	 * "b9403104 ldr"@0xffffff8008a64990 (the count, field at +48);
	 * "52814022 mov"@0xffffff8008a649a4 -- __LINE__ = 2561
	 */
#line 2561
	ILI_DBG("Item = %s, Frame Count = %d\n", mp_items[index].c0, mp_items[index].c48);

	/* DEFECT D5: "b8430d01 ldr"@0xffffff8008a649bc (pre-index, x8 becomes
	 * `&mp_items[index].c48`) + "34001e61 cbz"@0xffffff8008a649c4 */
	if (mp_items[index].c48 == 0) {
		/* "\x013ILITEK: (%s, %d): Frame count is zero, which at least sets as 1\n"
		 * @0xffffff80092446fa; "52814082 mov"@0xffffff8008a64da0 --
		 * __LINE__ = 2564 */
#line 2564
		ILI_ERR("Frame count is zero, which at least sets as 1\n");
		/* "128002b6 mov"@0xffffff8008a64da8 */
		ret = -22;
		goto out;
	}

	/*
	 * "94000201 bl"@0xffffff8008a649cc. The second argument is the SAME
	 * read of the field at +48 that has just served the `cbz`: w1 is not
	 * reloaded between "b8430d01 ldr"@0xffffff8008a649bc and the call.
	 * "2a0003f6 mov"@0xffffff8008a649d0 + "37f82420 tbnz"@0xffffff8008a649d4
	 */
	ret = create_mp_test_frame_buffer(index, mp_items[index].c48);
	if (ret < 0)
		goto out;

	/*
	 * "b9400108 ldr"@0xffffff8008a649e0 + "7100051f cmp"@0xffffff8008a649e4 +
	 * "54001c4b b.lt"@0xffffff8008a649e8 at the top, and at the bottom
	 * "f9400fe9 ldr"@0xffffff8008a64d5c + "b9400129 ldr"@0xffffff8008a64d64 +
	 * "6b09039f cmp"@0xffffff8008a64d68: the field is RE-READ on every round
	 */
	for (i = 0; i < mp_items[index].c48; i++) {
		ret = allnode_key_cdc_data(index);
		if (ret < 0) {
			/* "\x013ILITEK: (%s, %d): Failed to initialise CDC data, %d\n"
			 * @0xffffff8009243ad5, con
			 * "2a1603e3 mov"@0xffffff8008a64e50;
			 * "52814202 mov"@0xffffff8008a64e4c -- __LINE__ = 2576 */
#line 2576
			ILI_ERR("Failed to initialise CDC data, %d\n", ret);
			goto out;
		}

		/*
		 * DEFECT D4: no contribution from the frame counter.
		 * "b94bfaa8 ldr"@0xffffff8008a64d1c + "7100051f cmp"@0xffffff8008a64d20
		 * at the top and "b98bfaa8 ldrsw"@0xffffff8008a64d48 at the bottom: the
		 * guard re-reads `core_mp.key_len`. The two pointers are
		 * re-read on every round too --
		 * "f9421728 ldr"@0xffffff8008a64d30 (mp_frame_buf1) and
		 * "f940030b ldr"@0xffffff8008a64d38 (the field at +96)
		 */
		for (j = 0; j < core_mp.key_len; j++)
			mp_items[index].buf[j] = mp_frame_buf1[j];
	}

	/* "f9403101 ldr"@0xffffff8008a64d84 (the field at +96) +
	 * "940002b8 bl"@0xffffff8008a64d88 */
	compare_MaxMin_result(index, mp_items[index].buf);

out:
	return ret;
}

/*
 * pin_test() was reconstructed from the factory kernel disassembly (0xffffff8008a64eec, 740 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int pin_test(int index)
{
	int ret = 0;
	u8 cmd[5] = {0};

	/*
	 * "\x016ILITEK: (%s, %d): PIN test start"@0xffffff8009244890 -- WITHOUT
	 * a line ending (defect D6), __func__ = "pin_test"@0xffffff80092448b3;
	 * "5280fe82 mov"@0xffffff8008a64f24 -- __LINE__ = 2036. No
	 * guard: it is `ILI_INFO`
	 */
#line 2036
	ILI_INFO("PIN test start");
	/* "39415723 ldrb"@0xffffff8008a64f4c (+0x55) con
	 * "5280fea2 mov"@0xffffff8008a64f58 -- __LINE__ = 2037 */
#line 2037
	ILI_INFO("test_int_pin = 0x%x\n", mp_items[index].test_int_pin);
	/* "38456f03 ldrb"@0xffffff8008a64f6c (pre-index, +0x56) con
	 * "5280fec2 mov"@0xffffff8008a64f78 -- __LINE__ = 2038 */
#line 2038
	ILI_INFO("int_pulse_test = 0x%x\n", mp_items[index].int_pulse_test);
	/* "38454ee3 ldrb"@0xffffff8008a64f88 (pre-index, +0x54) con
	 * "5280fee2 mov"@0xffffff8008a64f94 -- __LINE__ = 2039 */
#line 2039
	ILI_INFO("delay_time = 0x%x\n", mp_items[index].delay_time);

	/*
	 * "39415728 ldrb"@0xffffff8008a64fa0 (a RE-READ of the field) +
	 * "7100051f cmp"@0xffffff8008a64fa8 + "54000381 b.ne"@0xffffff8008a64fac
	 */
	if (mp_items[index].test_int_pin == 1) {
		/* "39405108 ldrb"@0xffffff8008a64fb8 (+0x14) +
		 * "390003e8 strb"@0xffffff8008a64fc8 */
		cmd[0] = mp_items[index].c20;
		/* "320003ea orr"@0xffffff8008a64fc4 +
		 * "390007ea strb"@0xffffff8008a64fcc */
		cmd[1] = 0x1;

		/* "321f03e1 orr"@0xffffff8008a64fd8 (w1 = 2) +
		 * "d63f0100 blr"@0xffffff8008a64fdc +
		 * "37f808c0 tbnz"@0xffffff8008a64fe0 */
		if (idev->c776(cmd, 2) < 0) {
			/* "\x013ILITEK: (%s, %d): Write command failed\n"
			 * @0xffffff8009244937;
			 * "32002be2 orr"@0xffffff8008a65108 -- __LINE__ = 2047 */
#line 2047
			ILI_ERR("Write command failed\n");
			/* "12800c73 mov"@0xffffff8008a65140 */
			ret = -100;
			goto out;
		}

		/*
		 * "320003e0 orr"@0xffffff8008a64fe4 (w0 = 1) +
		 * "97ffe550 bl"@0xffffff8008a64fe8 +
		 * "37f807e0 tbnz"@0xffffff8008a64fec. The assignment to `ret` is
		 * DEAD on every path (each exit rewrites it), and the
		 * compiler removes it: there is no
		 * "mov w19, w0" here
		 */
		ret = check_int_level(true);
		if (ret < 0) {
			/* "12800d93 mov"@0xffffff8008a650e8 */
			ret = -109;
			goto out;
		}

		/* "390007ff strb"@0xffffff8008a64ff4 */
		cmd[1] = 0x0;

		/* "321f03e1 orr"@0xffffff8008a64ffc (w1 = 2) +
		 * "d63f0100 blr"@0xffffff8008a65004 +
		 * "37f80840 tbnz"@0xffffff8008a65008 */
		if (idev->c776(cmd, 2) < 0) {
			/* "528101c2 mov"@0xffffff8008a65120 -- __LINE__ = 2062 */
#line 2062
			ILI_ERR("Write command failed\n");
			ret = -100;
			goto out;
		}

		/*
		 * "2a1f03e0 mov"@0xffffff8008a6500c (w0 = 0) +
		 * "97ffe546 bl"@0xffffff8008a65010. HERE the assignment is LIVE
		 * -- "2a0003f3 mov"@0xffffff8008a65014 -- because the value
		 * can reach the return through the `int_pulse_test != 1`
		 * branch
		 */
		ret = check_int_level(false);
		if (ret < 0) {
			ret = -109;
			goto out;
		}
	}

	/*
	 * "39400308 ldrb"@0xffffff8008a6501c (+0x56, the pre-indexed register
	 * from before) + "7100051f cmp"@0xffffff8008a65020 +
	 * "54000661 b.ne"@0xffffff8008a65024
	 */
	if (mp_items[index].int_pulse_test == 1) {
		/* "\x016ILITEK: (%s, %d): MP IRQ Rising Trigger Test\n"
		 * @0xffffff8009244961;
		 * "52810382 mov"@0xffffff8008a65038 -- __LINE__ = 2076 */
#line 2076
		ILI_INFO("MP IRQ Rising Trigger Test\n");

		/* "321f03e9 orr"@0xffffff8008a65044 +
		 * "390007e9 strb"@0xffffff8008a65048 */
		cmd[1] = 0x2;
		/*
		 * "394002e8 ldrb"@0xffffff8008a65040 (+0x54) +
		 * "39000be8 strb"@0xffffff8008a6504c. It is the ONLY write of
		 * `cmd[2]` in the function (defect D8)
		 */
		cmd[2] = mp_items[index].delay_time;

		/* "97ffc47a bl"@0xffffff8008a65050 */
		ilitek_plat_irq_unregister();
		/* "320003e0 orr"@0xffffff8008a65054 (w0 = 1) +
		 * "97ffc481 bl"@0xffffff8008a6505c +
		 * "37f80440 tbnz"@0xffffff8008a65060 */
		ret = ilitek_plat_irq_register(1);
		if (ret < 0) {
			ret = -109;
			goto out;
		}

		/* "320003f4 orr"@0xffffff8008a65058 +
		 * "b902dd14 str"@0xffffff8008a65074 */
		idev->c732 = 1;
		/* "320007e1 orr"@0xffffff8008a65070 (w1 = 3) +
		 * "d63f0100 blr"@0xffffff8008a6507c +
		 * "37f80540 tbnz"@0xffffff8008a65080 */
		if (idev->c776(cmd, 3) < 0) {
			/* "52810542 mov"@0xffffff8008a65138 -- __LINE__ = 2090 */
#line 2090
			ILI_ERR("Write command failed\n");
			ret = -100;
			goto out;
		}

		/* "97ffd0d1 bl"@0xffffff8008a65084 +
		 * "37f80300 tbnz"@0xffffff8008a65088 */
		ret = ilitek_tddi_ic_check_int_stat();
		if (ret < 0) {
			ret = -109;
			goto out;
		}

		/* "\x016ILITEK: (%s, %d): MP IRQ Falling Trigger Test\n"
		 * @0xffffff8009244991;
		 * "528106a2 mov"@0xffffff8008a6509c -- __LINE__ = 2101 */
#line 2101
		ILI_INFO("MP IRQ Falling Trigger Test\n");

		/* "97ffc465 bl"@0xffffff8008a650a4 */
		ilitek_plat_irq_unregister();
		/* "321f03e0 orr"@0xffffff8008a650a8 (w0 = 2) +
		 * "97ffc46d bl"@0xffffff8008a650ac +
		 * "37f801c0 tbnz"@0xffffff8008a650b0 */
		ret = ilitek_plat_irq_register(2);
		if (ret < 0) {
			ret = -109;
			goto out;
		}

		/* "320003e9 orr"@0xffffff8008a650b8 +
		 * "b902dd09 str"@0xffffff8008a650c4 */
		idev->c732 = 1;
		/* "320007e1 orr"@0xffffff8008a650c0 (w1 = 3) +
		 * "d63f0100 blr"@0xffffff8008a650cc +
		 * "37f80720 tbnz"@0xffffff8008a650d0 */
		if (idev->c776(cmd, 3) < 0) {
			/* "52810822 mov"@0xffffff8008a651c4 -- __LINE__ = 2113 */
#line 2113
			ILI_ERR("Write command failed\n");
			ret = -100;
			goto out;
		}

		/* "97ffd0bd bl"@0xffffff8008a650d4 +
		 * "37f80080 tbnz"@0xffffff8008a650d8 +
		 * "2a0003f3 mov"@0xffffff8008a650dc */
		ret = ilitek_tddi_ic_check_int_stat();
		if (ret < 0) {
			ret = -109;
			goto out;
		}
	}

out:
	/*
	 * "b9002528 str"@0xffffff8008a65164 with the value propagated along the
	 * paths: "2a1f03e8 mov"@0xffffff8008a650e0,
	 * "2a1f03e8 mov"@0xffffff8008a650f0 and
	 * "12800008 mov"@0xffffff8008a65144
	 */
	if (ret < 0)
		mp_items[index].c36 = -1;
	else
		mp_items[index].c36 = 0;

	/* "\x016ILITEK: (%s, %d): Change to defualt IRQ trigger type\n"
	 * @0xffffff80092449c2 (defect D7);
	 * "52810a62 mov"@0xffffff8008a65160 -- __LINE__ = 2131 */
#line 2131
	ILI_INFO("Change to defualt IRQ trigger type\n");
	/* "97ffc433 bl"@0xffffff8008a6516c */
	ilitek_plat_irq_unregister();
	/*
	 * "b9415500 ldr"@0xffffff8008a65178 (idev + 340) +
	 * "97ffc439 bl"@0xffffff8008a6517c. THE RESULT IS NOT USED
	 */
	ilitek_plat_irq_register(idev->c340);
	return ret;
}

/*
 * ILI_ABS() was reconstructed from the factory kernel disassembly (0xffffff8008a629b0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define ILI_ABS(x)	((x) < 0 ? -(x) : (x))
#define ILI_MAX(a, b)	((a) > (b) ? (a) : (b))

/*
 * ipio_strcmp() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int ipio_strcmp(const char *s1, const char *s2)
{
	return (strlen(s1) != strlen(s2)) ? -1 : strncmp(s1, s2, strlen(s1));
}

/*
 * codeToOhm() was reconstructed from the factory kernel disassembly (0xffffff8009244147).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int codeToOhm(int code, u16 *v)
{
	int pull = 0;
	int ohm = 0;

	/* "340003a9 cbz"@0xffffff8008a62e58 */
	if (code == 0) {
		/* "\x013ILITEK: (%s, %d): code is invalid\n"@0xffffff8009244122,
		 * __func__ = "codeToOhm"@0xffffff8009244147;
		 * "5280ab22 mov"@0xffffff8008a62ed4 -- __LINE__ = 1369 */
#line 1369
		ILI_ERR("code is invalid\n");
		return 0;
	}

	pull = v[0] - v[1];
	ohm = ((pull << 22) / (code * 63) * 100 -
	       ((core_mp.c6 == 26 && core_mp.c4 == 0x7807) ? 1500 : 930)) / 1000;

	return ohm;
}

/*
 * mp_cdc_init_cmd_common() was reconstructed from the factory kernel disassembly (0xffffff8009244083).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mp_cdc_init_cmd_common(u8 *cmd, int len, int index)
{
	int ret = 0;
	/* "a90e7fff stp"@0xffffff8008a623d4 and the seven twins zero out
	 * 128 byte (sp+112..sp+239): is the declaration initialiser */
	char str[128] = {0};

	memset(cmd, 0xFF, len);

	if ((unsigned int)core_mp.c12 > 0x503FFU) {
		core_mp.c228 = 15;
		/*
		 * "97fffedf bl"@0xffffff8008a6240c towards
		 * <parser_get_ini_key_value> and "94102066 bl"@0xffffff8008a62428
		 * towards <snprintf>: this is the inlining of
		 * `parser_get_int_data`, and its two consecutive `tbnz`s --
		 * "37f80055 tbnz"@0xffffff8008a6242c and
		 * "37f86e20 tbnz"@0xffffff8008a62430 -- are the expression
		 * `(ret < 0) ? 0 : size` tested against zero
		 */
		ret = parser_get_int_data("pv5_4 command", mp_items[index].c0,
					  str, sizeof(str));
		if (ret < 0)
			return ret;

		/* "94000d64 bl"@0xffffff8008a62440, w2 = 15
		 * ("32000fe2 orr"@0xffffff8008a6243c) */
		return parser_get_u8_array(str, cmd, len);
	}

	/* "52801e28 mov"@0xffffff8008a6245c (0xf1) +
	 * "390183e8 strb"@0xffffff8008a62460 */
	cmd[0] = 0xF1;
	/* "39400115 ldrb"@0xffffff8008a62458 (mp_items[index].c20) +
	 * "390187f5 strb"@0xffffff8008a62468 */
	cmd[1] = mp_items[index].c20;
	/* "39018bff strb"@0xffffff8008a62454 */
	cmd[2] = 0x0;
	/* "b90be509 str"@0xffffff8008a62474 */
	core_mp.c228 = 3;

	/* "f100581f cmp"@0xffffff8008a62480 (0x16 = 22) e
	 * "open test(integration)"@0xffffff8009243648;
	 * "321f03e8 orr"@0xffffff8008a624a0 + "39018be8 strb"@0xffffff8008a624a4 */
	if (ipio_strcmp(mp_items[index].c0, "open test(integration)") == 0)
		cmd[2] = 0x2;

	/* "f100381f cmp"@0xffffff8008a624b0 (0xe = 14) e
	 * "open test(cap)"@0xffffff800924365f;
	 * "320007e8 orr"@0xffffff8008a624d0 + "39018be8 strb"@0xffffff8008a624d4 */
	if (ipio_strcmp(mp_items[index].c0, "open test(cap)") == 0)
		cmd[2] = 0x3;

	/* "b9400108 ldr"@0xffffff8008a624dc + "7100211f cmp"@0xffffff8008a624e0 */
	if (mp_items[index].c16 == 8) {
		/* "53087ec8 lsr"@0xffffff8008a624fc +
		 * "39018be8 strb"@0xffffff8008a62500 */
		cmd[2] = mp_items[index].c48 >> 8;
		/* "39018ff6 strb"@0xffffff8008a62508 */
		cmd[3] = mp_items[index].c48;
		/* "390193ff strb"@0xffffff8008a624f4 */
		cmd[4] = 0;
		/* "b90be509 str"@0xffffff8008a6250c */
		core_mp.c228 = 5;

		/* "f100741f cmp"@0xffffff8008a62514 (0x1d = 29) e
		 * "noise peak to peak(cut panel)"@0xffffff800924362a;
		 * "320003e7 orr"@0xffffff8008a628d0 +
		 * "390193e7 strb"@0xffffff8008a628d4 */
		if (ipio_strcmp(mp_items[index].c0,
				"noise peak to peak(cut panel)") == 0)
			cmd[4] = 1;

		/*
		 * "\x016ILITEK: (%s, %d): P2P CMD: %d,%d,%d,%d,%d\n"
		 * @0xffffff8009244056, __func__ =
		 * "mp_cdc_init_cmd_common"@0xffffff8009244083; the guard is
		 * "39656348 ldrb"@0xffffff8008a6253c +
		 * "34000168 cbz"@0xffffff8008a62540;
		 * "5280c402 mov"@0xffffff8008a62554 -- __LINE__ = 1568.
		 * THE FIVE ARGUMENTS come from the bytes just written, propagated
		 * by the compiler: "52801e23 mov"@0xffffff8008a62558 (0xf1),
		 * "2a1503e4 mov"@0xffffff8008a62564,
		 * "53083ec5 ubfx"@0xffffff8008a6254c,
		 * "12001ec6 and"@0xffffff8008a62550 and w7
		 */
#line 1568
		ILI_DBG("P2P CMD: %d,%d,%d,%d,%d\n", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
	}

	return ret;
}

/*
 * allnode_mutual_cdc_data() was reconstructed from the factory kernel disassembly (0xffffff8009243e85).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int allnode_mutual_cdc_data(int index)
{
	int i = 0, idx = 0, ret = 0, len = 0, tmp = 0, val = 0;
	u8 cmd[15] = {0};
	u8 *ori = NULL;

	/*
	 * "294026e8 ldp"@0xffffff8008a62348 reads in ONE go the two adjacent
	 * fields `core_mp.c232` and `core_mp.c236`,
	 * "1b097d08 mul"@0xffffff8008a62358 multiplies them,
	 * "531f7908 lsl"@0xffffff8008a6235c doubles the result and
	 * "11000919 add"@0xffffff8008a62360 adds 2
	 */
	len = core_mp.c232 * core_mp.c236 * 2 + 2;

	/* "\x016ILITEK: (%s, %d): Read X/Y Channel length = %d\n"
	 * @0xffffff8009243e53; the guard is
	 * "3965634a ldrb"@0xffffff8008a62350 +
	 * "3400010a cbz"@0xffffff8008a62368;
	 * "5280d622 mov"@0xffffff8008a62374 -- __LINE__ = 1713 */
#line 1713
	ILI_DBG("Read X/Y Channel length = %d\n", len);

	/* "71000b3f cmp"@0xffffff8008a62388 + "5400706d b.le"@0xffffff8008a6238c */
	if (len <= 2) {
		/* "\x013ILITEK: (%s, %d): Length is invalid\n"@0xffffff8009243e9d;
		 * "5280d682 mov"@0xffffff8008a631a8 -- __LINE__ = 1716 */
#line 1716
		ILI_ERR("Length is invalid\n");
		ret = -105;
		goto out;
	}

	if (mp_cdc_init_cmd_common(cmd, sizeof(cmd), index) < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to get cdc command\n"
		 * @0xffffff8009243ec4;
		 * "5280d7a2 mov"@0xffffff8008a63204 -- __LINE__ = 1725 */
#line 1725
		ILI_ERR("Failed to get cdc command\n");
		ret = -100;
		goto out;
	}

	/* "97ffe153 bl"@0xffffff8008a6258c con
	 * "Mutual CDC command"@0xffffff8009243ef3, w1 = 8
	 * ("321d03e1 orr"@0xffffff8008a6257c) e w2 = `core_mp.c228`
	 * ("b94be502 ldr"@0xffffff8008a62570) */
	ilitek_dump_data(cmd, 8, core_mp.c228, 0, "Mutual CDC command");

	/* "320003e9 orr"@0xffffff8008a62598 + "b902dd09 str"@0xffffff8008a625a0 */
	idev->c732 = 1;

	/* "f9418508 ldr"@0xffffff8008a625a4 (idev->c776),
	 * "b94be6a1 ldr"@0xffffff8008a625a8 (core_mp.c228) e
	 * "d63f0100 blr"@0xffffff8008a625ac */
	if (idev->c776(cmd, core_mp.c228) < 0) {
		/* "\x013ILITEK: (%s, %d): Write CDC command failed\n"
		 * @0xffffff8009243f06;
		 * "5280d8e2 mov"@0xffffff8008a631c8 -- __LINE__ = 1735 */
#line 1735
		ILI_ERR("Write CDC command failed\n");
		ret = -100;
		goto out;
	}

	/* "b94c6508 ldr"@0xffffff8008a625b8 (core_mp.c356) +
	 * "7100051f cmp"@0xffffff8008a625bc */
	if (core_mp.c356 == 1)
		/* "52800640 mov"@0xffffff8008a625c4 + "52800641 mov"@0xffffff8008a625c8 */
		ret = ilitek_tddi_ic_check_busy(50, 50);
	else
		/* "97ffdb7d bl"@0xffffff8008a625d4, no arguments */
		ret = ilitek_tddi_ic_check_int_stat();

	/* "37f85fa0 tbnz"@0xffffff8008a625e0 */
	if (ret < 0) {
		ret = -110;
		goto out;
	}

	/*
	 * "b94b1108 ldr"@0xffffff8008a625e8 (core_mp.c16), the constant 0x10401
	 * is "52808029 mov"@0xffffff8008a625ec + "72a00029 movk"@0xffffff8008a625f0,
	 * and "540002a8 b.hi"@0xffffff8008a625f8 tests UNSIGNED
	 */
	if ((unsigned int)core_mp.c16 <= 0x10401U) {
		/*
		 * "529e5ec9 mov"@0xffffff8008a62600 (0xf2f6) +
		 * "7900c3e9 strh"@0xffffff8008a62604: TWO bytes in one go
		 */
		cmd[0] = 0xF6;
		cmd[1] = 0xF2;
		/* "321f03e1 orr"@0xffffff8008a62610 -- length 2 */
		if (idev->c776(cmd, 2) < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x, 0x%x) error\n"
			 * @0xffffff8009243f34, con
			 * "394183e3 ldrb"@0xffffff8008a63218 e
			 * "394187e4 ldrb"@0xffffff8008a6321c;
			 * "5280dba2 mov"@0xffffff8008a63230 -- __LINE__ = 1757 */
#line 1757
			ILI_ERR("Write (0x%x, 0x%x) error\n", cmd[0], cmd[1]);
			ret = -100;
			goto out;
		}
		/* "52912b00 mov"@0xffffff8008a6261c +
		 * "72a00820 movk"@0xffffff8008a62620 = 0x418958 = 1000 * 0x10c7 */
		mdelay(1);

		/*
		 * "b2400129 orr"@0xffffff8008a622f0 sets bit 0 of the
		 * pointer -- `sp+96` is eight-aligned, so it is
		 * `&cmd[1]` -- and "f90017e9 str"@0xffffff8008a622f4 saves it
		 * outside the loop; "f94017e0 ldr"@0xffffff8008a6262c re-reads it
		 */
		if (idev->c776(&cmd[1], 1) < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x) error\n"
			 * @0xffffff8009243f62, con
			 * "394187e3 ldrb"@0xffffff8008a6323c;
			 * "5280dcc2 mov"@0xffffff8008a63250 -- __LINE__ = 1766 */
#line 1766
			ILI_ERR("Write (0x%x) error\n", cmd[1]);
			ret = -100;
			goto out;
		}
		mdelay(1);
	}

	/*
	 * "93407f20 sxtw"@0xffffff8008a62650 (the length is a signed `int`),
	 * "52901801 mov"@0xffffff8008a6264c +
	 * "72a02801 movk"@0xffffff8008a62654 = 0x14080c0 = GFP_KERNEL |
	 * __GFP_ZERO, "97dfb74b bl"@0xffffff8008a6265c towards <__kmalloc>
	 */
	ori = kzalloc(len, GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a62664 + "54000ea8 b.hi"@0xffffff8008a62668
	 * (IS_ERR) e "b4000e95 cbz"@0xffffff8008a6266c (NULL) */
	if (IS_ERR(ori) || ori == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate ori, (%ld)\n"
		 * @0xffffff8009243f8a, con "aa1503e3 mov"@0xffffff8008a62850;
		 * "5280de42 mov"@0xffffff8008a62844 -- __LINE__ = 1778 */
#line 1778
		ILI_ERR("Failed to allocate ori, (%ld)\n", PTR_ERR(ori));
		ret = -107;
		goto out;
	}

	/* "f9418908 ldr"@0xffffff8008a6267c (idev->c784) +
	 * "d63f0100 blr"@0xffffff8008a62680, w1 = len */
	if (idev->c784(ori, len) < 0) {
		/* "\x013ILITEK: (%s, %d): Read cdc data error, len = %d\n"
		 * @0xffffff8009243fbd;
		 * "5280df22 mov"@0xffffff8008a62878 -- __LINE__ = 1785 */
#line 1785
		ILI_ERR("Read cdc data error, len = %d\n", len);
		ret = -108;
		goto out;
	}

	/* "97ffe10e bl"@0xffffff8008a626a0 con
	 * "Mutual CDC original"@0xffffff8009243ff0 */
	ilitek_dump_data(ori, 8, len, 0, "Mutual CDC original");

	/* "f9421100 ldr"@0xffffff8008a626a8 + "b4000f40 cbz"@0xffffff8008a626ac */
	if (mp_frame_buf0 == NULL) {
		/*
		 * the shape of `kcalloc`: "b98c0108 ldrsw"@0xffffff8008a62898
		 * (SIGNED), "37f80248 tbnz"@0xffffff8008a6289c (the overflow
		 * check of `kmalloc_array`),
		 * "d37ef500 lsl"@0xffffff8008a628a4 (times four),
		 * "97dfb6b7 bl"@0xffffff8008a628ac towards <__kmalloc> and
		 * "f9021100 str"@0xffffff8008a628bc
		 */
		mp_frame_buf0 = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		/* "b140041f cmn"@0xffffff8008a628b4 +
		 * "54000188 b.hi"@0xffffff8008a628c0 e
		 * "b5ffefe3 cbnz"@0xffffff8008a628c8 */
		if (IS_ERR(mp_frame_buf0) || mp_frame_buf0 == NULL) {
			/* "\x013ILITEK: (%s, %d): Failed to allocate FrameBuffer mem (%ld)\n"
			 * @0xffffff8009244004;
			 * "5280e062 mov"@0xffffff8008a628f8 -- __LINE__ = 1795 */
#line 1795
			ILI_ERR("Failed to allocate FrameBuffer mem (%ld)\n", PTR_ERR(mp_frame_buf0));
			ret = -107;
			goto out;
		}
	} else {
		/*
		 * DEFECT F6-D1: "b98c0322 ldrsw"@0xffffff8008a626b4 passes the
		 * number of ELEMENTS as the number of BYTES
		 */
		memset(mp_frame_buf0, 0, core_mp.c256);
	}

	/*
	 * THE TWO INDEX CHAINS ARE DISTINCT, as in the two twins: x26
	 * starts from 0x100000000 ("b26003fa orr"@0xffffff8008a626d4) and x22 from
	 * 0x200000000 ("b25f03f6 orr"@0xffffff8008a626d8), both with stride
	 * 0x200000000 ("8b08035a add"@0xffffff8008a62800 and
	 * "8b0802d6 add"@0xffffff8008a62808), and the two indices come out of
	 * "9360ff48 asr"@0xffffff8008a62710 and "9360fec9 asr"@0xffffff8008a62718
	 */
	for (i = 0; i < core_mp.c256; i++) {
		idx = (2 * i) + 1;

		/* "calibration data(dac)"@0xffffff8009243552,
		 * "f100541f cmp"@0xffffff8008a626ec (0x15 = 21) */
		/*
		 * "38686aa8 ldrb"@0xffffff8008a62714 comes BEFORE
		 * "34000300 cbz"@0xffffff8008a6271c: `ori[idx]` is read ONCE
		 * ONLY for both branches, while `ori[idx + 1]` is
		 * read TWICE ("38696aa9 ldrb"@0xffffff8008a62720 and
		 * "38696aa9 ldrb"@0xffffff8008a6277c), once per branch
		 */
		val = ori[idx];

		if (ipio_strcmp(mp_items[index].c0, "calibration data(dac)") == 0) {
			/*
			 * "1200190a and"@0xffffff8008a62780 +
			 * "7219011f tst"@0xffffff8008a62784 +
			 * "5a8a0548 cneg"@0xffffff8008a62788 for the first byte
			 * and the twin pair for the second; the SIGNED
			 * division by two is "7100011f cmp"@0xffffff8008a627a4 +
			 * "1a88a508 cinc"@0xffffff8008a627a8 +
			 * "13017d08 asr"@0xffffff8008a627ac, and the `str` is
			 * "b83c7928 str"@0xffffff8008a627b0
			 */
			mp_frame_buf0[i] = (((val & 0x80) ? -(val & 0x7F) : (val & 0x7F)) +
					    ((ori[idx + 1] & 0x80) ? -(ori[idx + 1] & 0x7F) : (ori[idx + 1] & 0x7F))) / 2;
		} else {
			/*
			 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a62730).
			 *
			 * The working notes -- the disassembly citations, the measurements against
			 * the factory binary and the reasoning behind each choice -- are in
			 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
			 * in the oracolo repository. They are kept in Italian, as the project's
			 * internal record.
			 */
			tmp = (val << 8) + ori[idx + 1];
			if (val & 0x80)
				tmp -= 0x10000;
			mp_frame_buf0[i] = tmp;
		}

		/*
		 * This section was reconstructed from the factory kernel disassembly (0xffffff8009243580).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		if (ipio_strcmp(mp_items[index].c0, "raw data(no bk)") == 0 ||
		    ipio_strcmp(mp_items[index].c0, "raw data(no bk) (lcm off)") == 0)
			mp_frame_buf0[i] -= core_mp.c212;
	}

	/* "97ffe0a9 bl"@0xffffff8008a62834 con
	 * "Mutual CDC combined"@0xffffff8009244042, w1 = 32
	 * ("321b03e1 orr"@0xffffff8008a6282c) e w3 = `core_mp.c232`
	 * ("b94beb63 ldr"@0xffffff8008a62824) */
	ilitek_dump_data(mp_frame_buf0, 32, core_mp.c256, core_mp.c232,
			 "Mutual CDC combined");

out:
	/*
	 * "b902dd1f str"@0xffffff8008a62864, "b902dd1f str"@0xffffff8008a62914
	 * and "b902dd1f str"@0xffffff8008a63260: THREE copies, one per group of
	 * paths
	 */
	idev->c732 = 0;
	/*
	 * "b5000595 cbnz"@0xffffff8008a62868 + "97dfb7a5 bl"@0xffffff8008a6291c
	 * towards <kfree>. On the paths where `ori` is still NULL the
	 * free does not appear at all
	 */
	ipio_kfree((void **)&ori);
	return ret;
}

/*
 * mutual_test() was reconstructed from the factory kernel disassembly (0xffffff8008a620d4, 4584 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_mp.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mutual_test(int index)
{
	int i = 0, j = 0, x = 0, y = 0, ret = 0;
	int frame_count = 0;

	/*
	 * "\x016ILITEK: (%s, %d): index = %d, desp = %s, Frame Count = %d\n"
	 * @0xffffff8009243a47, __func__ = "mutual_test"@0xffffff8009243a84; the
	 * guard is "39656348 ldrb"@0xffffff8008a6210c +
	 * "340001c8 cbz"@0xffffff8008a62110; the three arguments are
	 * "2a1603e3 mov"@0xffffff8008a62140 (the index),
	 * "f9400104 ldr"@0xffffff8008a62124 (the name, field at +0) and
	 * "b9403105 ldr"@0xffffff8008a62128 (the count, field at +48);
	 * "52810be2 mov"@0xffffff8008a6213c -- __LINE__ = 2143
	 */
#line 2143
	ILI_DBG("index = %d, desp = %s, Frame Count = %d\n", index, mp_items[index].c0, mp_items[index].c48);

	/*
	 * "b8430ee1 ldr"@0xffffff8008a62158 (pre-indexed, x23 becomes
	 * `&mp_items[index].c48`) + "7100003f cmp"@0xffffff8008a6215c +
	 * "5400012c b.gt"@0xffffff8008a62160: the comparison is SIGNED and against
	 * zero, not `== 0` as in `key_test`
	 */
	if (mp_items[index].c48 < 1) {
		/* "\x013ILITEK: (%s, %d): Frame count is zero, which is at least set as 1\n"
		 * @0xffffff8009243a90;
		 * "52810cc2 mov"@0xffffff8008a62174 -- __LINE__ = 2150 */
#line 2150
		ILI_ERR("Frame count is zero, which is at least set as 1\n");
		/* "320003e1 orr"@0xffffff8008a6217c +
		 * "b90002e1 str"@0xffffff8008a62180 */
		mp_items[index].c48 = 1;
	}

	/*
	 * "94000c12 bl"@0xffffff8008a62188. The second argument is the SAME
	 * read of the field at +48 that has just served the comparison: w1 is not
	 * reloaded between "b8430ee1 ldr"@0xffffff8008a62158 and the call.
	 * "2a0003f5 mov"@0xffffff8008a62198 + "37f807c0 tbnz"@0xffffff8008a6218c
	 */
	ret = create_mp_test_frame_buffer(index, mp_items[index].c48);
	if (ret < 0) {
		/* "12800d55 mov"@0xffffff8008a62284 */
		ret = -107;
		goto out;
	}

	/*
	 * DEFECT F6-D2: the guard reads `mp_items[0].c16`.
	 * "b94bec51 ldr"@0xffffff8008a62194 (core_mp.c236) and
	 * "b94be930 ldr"@0xffffff8008a621a8 (core_mp.c232) are the two limits;
	 * the four constants are "320103ee orr"@0xffffff8008a621cc
	 * (0x80000000) and "32007bef orr"@0xffffff8008a621d0 (0x7fffffff)
	 */
	for (x = 0; x < core_mp.c236; x++) {
		for (y = 0; y < core_mp.c232; y++) {
			/* "b946e152 ldr"@0xffffff8008a621e0 +
			 * "7100125f cmp"@0xffffff8008a621e4 */
			if (mp_items[0].c16 == 4) {
				/* "f9402db2 ldr"@0xffffff8008a621ec (core_mp+320)
				 * + "b830da4e str"@0xffffff8008a621fc */
				core_mp.tx_max_buf[x * core_mp.c232 + y] = INT_MIN;
				/* "f94035b2 ldr"@0xffffff8008a62204 (core_mp+336)
				 * + "b830da4e str"@0xffffff8008a6220c */
				core_mp.rx_max_buf[x * core_mp.c232 + y] = INT_MIN;
				/*
				 * "910181a1 add"@0xffffff8008a621f8 (core_mp+328)
				 * and "912fa1ad add"@0xffffff8008a621c8 give the
				 * base; the `str` is
				 * "b830d832 str"@0xffffff8008a62230, merged with the
				 * `else` branch
				 */
				core_mp.tx_min_buf[x * core_mp.c232 + y] = INT_MAX;
				/*
				 * "9101c1a0 add"@0xffffff8008a621f4 (core_mp+344);
				 * the `str` is "b830da4f str"@0xffffff8008a62240,
				 * merged with the `else` branch
				 */
				core_mp.rx_min_buf[x * core_mp.c232 + y] = INT_MAX;
			} else {
				/* "320103f2 orr"@0xffffff8008a6221c +
				 * "aa0b03e1 mov"@0xffffff8008a62220
				 * (&mp_items[index].max_buf, from
				 * "9101a18b add"@0xffffff8008a621c0) */
				mp_items[index].max_buf[x * core_mp.c232 + y] = INT_MIN;
				/* "aa0c03e0 mov"@0xffffff8008a62224
				 * (&mp_items[index].min_buf, from
				 * "9101c18c add"@0xffffff8008a621c4) */
				mp_items[index].min_buf[x * core_mp.c232 + y] = INT_MAX;
			}
		}
	}

	/*
	 * "b8410d28 ldr"@0xffffff8008a6226c (pre-indexed, x9 becomes
	 * `&mp_items[index].c16`) + "7100211f cmp"@0xffffff8008a62270 +
	 * "540000a1 b.ne"@0xffffff8008a62278; "320003e8 orr"@0xffffff8008a6227c
	 * (one) and "b94002e8 ldr"@0xffffff8008a6228c (the field at +48);
	 * "b90047e8 str"@0xffffff8008a62290 saves it at [sp,#68]
	 */
	if (mp_items[index].c16 == 8)
		frame_count = 1;
	else
		frame_count = mp_items[index].c48;

	/* "39405508 ldrb"@0xffffff8008a6229c (mp_items[index].c21, ONE byte) +
	 * "7100051f cmp"@0xffffff8008a622a0 */
	if (mp_items[index].c21 == 1) {
		/*
		 * "a94786a0 ldp"@0xffffff8008a622bc reads in ONE go the two
		 * adjacent pointers at +120 and +128,
		 * "39405aa2 ldrb"@0xffffff8008a622c4 the byte at +22,
		 * "f94002a3 ldr"@0xffffff8008a622c0 the name and
		 * "b94c0124 ldr"@0xffffff8008a622b8 `core_mp.c256`;
		 * "97ffee87 bl"@0xffffff8008a622c8
		 */
		parser_ini_benchmark(mp_items[index].bench_mark_max,
				     mp_items[index].bench_mark_min,
				     mp_items[index].c22, mp_items[index].c0,
				     core_mp.c256);
		/*
		 * "a94786a0 ldp"@0xffffff8008a622cc RE-READS the two pointers +
		 * "94000d1a bl"@0xffffff8008a622d4
		 */
		dump_benchmark_data(mp_items[index].bench_mark_max,
				    mp_items[index].bench_mark_min);
	}

	/* "b94047e8 ldr"@0xffffff8008a622d8 + "7100051f cmp"@0xffffff8008a622dc
	 * + "54007d0b b.lt"@0xffffff8008a622e0 at the head, and at the tail
	 * "294a2bf5 ldp"@0xffffff8008a63180 (ret and i together) +
	 * "b94047e8 ldr"@0xffffff8008a63184 + "6b08015f cmp"@0xffffff8008a6318c */
	for (i = 0; i < frame_count; i++) {
		ret = allnode_mutual_cdc_data(index);
		if (ret < 0) {
			/* "\x013ILITEK: (%s, %d): Failed to initialise CDC data, %d\n"
			 * @0xffffff8009243ad5, con
			 * "2a1503e3 mov"@0xffffff8008a63278;
			 * "52811162 mov"@0xffffff8008a63274 -- __LINE__ = 2187 */
#line 2187
			ILI_ERR("Failed to initialise CDC data, %d\n", ret);
			goto out;
		}

		/*
		 * "b9400108 ldr"@0xffffff8008a6292c: the field is RE-READ after
		 * the data read
		 */
		switch (mp_items[index].c16) {
		case 4: {
			/*
			 * "f9421129 ldr"@0xffffff8008a62970: the pointer is
			 * read ONCE ONLY, outside both loops
			 */
			int *frame = mp_frame_buf0;

			for (x = 0; x < core_mp.c236; x++) {
				for (y = 0; y < core_mp.c232; y++) {
					/* "6b0d011f cmp"@0xffffff8008a6298c +
					 * "54000180 b.eq"@0xffffff8008a62994 */
					if (x != core_mp.c236 - 1)
						/* "f94026ef ldr"@0xffffff8008a629ac
						 * (core_mp+304, tx_delta_buf) +
						 * "6b0a01ca subs"@0xffffff8008a629b0
						 * + "5a8a554a cneg"@0xffffff8008a629b4
						 * + "b82d69ea str"@0xffffff8008a629b8.
						 * DEFECT F6-D3: the index is
						 * `+ 1`, the right-hand neighbour */
						core_mp.tx_delta_buf[x * core_mp.c232 + y] =
							ILI_ABS(frame[x * core_mp.c232 + y] -
								frame[x * core_mp.c232 + y + 1]);

					/* "6b0d019f cmp"@0xffffff8008a629c8 +
					 * "54000160 b.eq"@0xffffff8008a629cc */
					if (y != core_mp.c232 - 1)
						/* "f9402aee ldr"@0xffffff8008a629e4
						 * (core_mp+312, rx_delta_buf) +
						 * "b82b69ca str"@0xffffff8008a629f0 */
						core_mp.rx_delta_buf[x * core_mp.c232 + y] =
							ILI_ABS(frame[x * core_mp.c232 + y] -
								frame[x * core_mp.c232 + y + 1]);
				}
			}
			break;
		}
		case 5: {
			/* "f9421108 ldr"@0xffffff8008a62a34 */
			int *frame = mp_frame_buf0;

			for (x = 0; x < core_mp.c236; x++) {
				for (y = 0; y < core_mp.c232; y++) {
					/* "6b0e019f cmp"@0xffffff8008a62a64 +
					 * "5400008d b.le"@0xffffff8008a62a68 +
					 * "b82e69ac str"@0xffffff8008a62a70 */
					if (frame[x * core_mp.c232 + y] >
					    mp_items[index].max_buf[x * core_mp.c232 + y])
						mp_items[index].max_buf[x * core_mp.c232 + y] =
							frame[x * core_mp.c232 + y];

					/*
					 * "6b0d019f cmp"@0xffffff8008a62a80 +
					 * "540000aa b.ge"@0xffffff8008a62a84 +
					 * "b82d69cc str"@0xffffff8008a62a8c.
					 * "b86e690c ldr"@0xffffff8008a62a74
					 * RE-READS `frame[idx]` after the previous
					 * `str`
					 */
					if (frame[x * core_mp.c232 + y] <
					    mp_items[index].min_buf[x * core_mp.c232 + y])
						mp_items[index].min_buf[x * core_mp.c232 + y] =
							frame[x * core_mp.c232 + y];

					/* "4b0d018c sub"@0xffffff8008a62aac +
					 * "b82b69cc str"@0xffffff8008a62ab0 */
					mp_items[index].buf[x * core_mp.c232 + y] =
						mp_items[index].max_buf[x * core_mp.c232 + y] -
						mp_items[index].min_buf[x * core_mp.c232 + y];
				}
			}
			break;
		}
		case 6: {
			/* "f9421108 ldr"@0xffffff8008a62aec */
			int *frame = mp_frame_buf0;

			for (x = 0; x < core_mp.c236; x++) {
				for (y = 0; y < core_mp.c232; y++) {
					int tmp[4] = {0};
					/*
					 * "b86bd90d ldr"@0xffffff8008a62c24 comes BEFORE
					 * "340001ec cbz"@0xffffff8008a62c30: the
					 * node is read ONCE ONLY for all nine
					 * paths
					 */
					int v = frame[x * core_mp.c232 + y];

					/*
					 * "2a09014c orr"@0xffffff8008a62c28 +
					 * "340001ec cbz"@0xffffff8008a62c30:
					 * `x == 0 && y == 0` folded into a
					 * single test, and the ZERO that comes out
					 * is reused as the value of `tmp[3]`
					 */
					if (x == 0 && y == 0) {
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else if (y == 0 && x == core_mp.c236 - 1) {
						/* "5100060e sub"@0xffffff8008a62c48 e
						 * "11000610 add"@0xffffff8008a62c50 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else if (x == 0 && y == core_mp.c232 - 1) {
						/* "110005d0 add"@0xffffff8008a62c9c e
						 * "510005ce sub"@0xffffff8008a62ca4 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
					} else if (y == core_mp.c232 - 1 && x == core_mp.c236 - 1) {
						/*
						 * "510005ce sub"@0xffffff8008a62b24:
						 * a single access, the left-hand neighbour
						 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
					} else if (x == 0) {
						/* "110005cf add"@0xffffff8008a62b38 e
						 * "510005ce sub"@0xffffff8008a62b40 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[2] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else if (y == 0) {
						/* "5100060e sub"@0xffffff8008a62b6c e
						 * "1100060f add"@0xffffff8008a62b74 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
						tmp[2] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else if (x == core_mp.c236 - 1) {
						/* "510005cf sub"@0xffffff8008a62ba8 e
						 * "110005ce add"@0xffffff8008a62bb0 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[2] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else if (y == core_mp.c232 - 1) {
						/*
						 * merged with the following branch by the three
						 * "1a8d13ec csel"@0xffffff8008a62c0c,
						 * "1a9011af csel"@0xffffff8008a62c10 and
						 * "1a8d120e csel"@0xffffff8008a62c14,
						 * driven by
						 * "6a0e023f tst"@0xffffff8008a62c08
						 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[2] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else {
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
						tmp[2] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[3] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					}

					/* "6b1001df cmp"@0xffffff8008a62cc4 +
					 * "1a90c1cd csel"@0xffffff8008a62cc8,
					 * "6b0d01ff cmp"@0xffffff8008a62ccc +
					 * "1a8dc1ed csel"@0xffffff8008a62cd4,
					 * "6b0d019f cmp"@0xffffff8008a62cd8 +
					 * "1a8dc18c csel"@0xffffff8008a62cdc, e
					 * "b82b79cc str"@0xffffff8008a62ce0 */
					mp_items[index].buf[x * core_mp.c232 + y] =
						ILI_MAX(tmp[3], ILI_MAX(tmp[2], ILI_MAX(tmp[1], tmp[0])));
				}
			}
			break;
		}
		case 7: {
			/*
			 * "f9421119 ldr"@0xffffff8008a62d0c: the pointer is
			 * read at the top of the case, BEFORE the two comparisons on the
			 * name
			 */
			int *frame = mp_frame_buf0;

			/*
			 * "f100581f cmp"@0xffffff8008a62d2c (0x16 = 22) and
			 * "open test(integration)"@0xffffff8009243648.
			 * THERE IS A SINGLE `strlen` for both comparisons
			 * ("940fd579 bl"@0xffffff8008a62d1c): the compiler
			 * shares it
			 */
			if (ipio_strcmp(mp_items[index].c0, "open test(integration)") == 0) {
				/* "b8696b2a ldr"@0xffffff8008a62d6c +
				 * "b829696a str"@0xffffff8008a62d78 */
				for (j = 0; j < core_mp.c256; j++)
					mp_items[index].buf[j] = frame[j];

			/*
			 * "f100381f cmp"@0xffffff8008a62d20 (0xe = 14) and
			 * "open test(cap)"@0xffffff800924365f.
			 * It is an `else if`, and that is MEASURED: there is a SINGLE
			 * `strlen` for both comparisons. With two independent `if`s
			 * the first one's loop writes to memory and the compiler
			 * MUST call `strlen` again (its argument might have been
			 * overwritten): two calls instead of one
			 */
			} else if (ipio_strcmp(mp_items[index].c0, "open test(cap)") == 0) {
				for (x = 0; x < core_mp.c236; x++) {
					for (y = 0; y < core_mp.c232; y++) {
						int sum = 0;
						int cnt = 0;
						int avg = 0;
						/* "b860db2e ldr"@0xffffff8008a62fc4 */
						int v = frame[x * core_mp.c232 + y];

						/*
						 * "29402eea ldp"@0xffffff8008a6313c reads
						 * `core_mp.c232` and `core_mp.c236` in ONE
						 * go, once per round
						 */
						if (x - 1 >= 0 && x - 1 < core_mp.c236 &&
						    y - 1 >= 0 && y - 1 < core_mp.c232) {
							/* "1b0c354f madd"@0xffffff8008a62ffc +
							 * "b86fdb2f ldr"@0xffffff8008a63000 */
							sum += frame[(x - 1) * core_mp.c232 + (y - 1)];
							cnt++;
						}
						if (x - 1 >= 0 && x - 1 < core_mp.c236 &&
						    y >= 0 && y < core_mp.c232) {
							/* "11000421 add"@0xffffff8008a63014 +
							 * "b861db21 ldr"@0xffffff8008a63018 */
							sum += frame[(x - 1) * core_mp.c232 + y];
							cnt++;
						}
						if (x - 1 >= 0 && x - 1 < core_mp.c236 &&
						    y + 1 >= 0 && y + 1 < core_mp.c232) {
							/* "11000821 add"@0xffffff8008a63034 +
							 * "b861db21 ldr"@0xffffff8008a63038 */
							sum += frame[(x - 1) * core_mp.c232 + (y + 1)];
							cnt++;
						}
						if (x >= 0 && x < core_mp.c236 &&
						    y + 1 >= 0 && y + 1 < core_mp.c232) {
							/* "0b1001a1 add"@0xffffff8008a63068 +
							 * "11000821 add"@0xffffff8008a6306c */
							sum += frame[x * core_mp.c232 + (y + 1)];
							cnt++;
						}
						if (x + 1 >= 0 && x + 1 < core_mp.c236 &&
						    y + 1 >= 0 && y + 1 < core_mp.c232) {
							/* "1b083541 madd"@0xffffff8008a63090 +
							 * "11000821 add"@0xffffff8008a63094 */
							sum += frame[(x + 1) * core_mp.c232 + (y + 1)];
							cnt++;
						}
						if (x + 1 >= 0 && x + 1 < core_mp.c236 &&
						    y >= 0 && y < core_mp.c232) {
							/* "1b083541 madd"@0xffffff8008a630d8 +
							 * "11000421 add"@0xffffff8008a630dc */
							sum += frame[(x + 1) * core_mp.c232 + y];
							cnt++;
						}
						if (x + 1 >= 0 && x + 1 < core_mp.c236 &&
						    y - 1 >= 0 && y - 1 < core_mp.c232) {
							/* "1b083541 madd"@0xffffff8008a630bc +
							 * "b861db21 ldr"@0xffffff8008a630c0 */
							sum += frame[(x + 1) * core_mp.c232 + (y - 1)];
							cnt++;
						}
						if (x >= 0 && x < core_mp.c236 &&
						    y - 1 >= 0 && y - 1 < core_mp.c232) {
							/* "0b1001aa add"@0xffffff8008a63110 +
							 * "b86adb2a ldr"@0xffffff8008a63114 */
							sum += frame[x * core_mp.c232 + (y - 1)];
							cnt++;
						}

						/* "0b0e01ea add"@0xffffff8008a63124 +
						 * "1100062f add"@0xffffff8008a63128 +
						 * "1acf0d4a sdiv"@0xffffff8008a6312c */
						avg = (sum + v) / (cnt + 1);
						/* "1b047dce mul"@0xffffff8008a63130 (100,
						 * from "52800c84 mov"@0xffffff8008a62fa0) +
						 * "1aca0dca sdiv"@0xffffff8008a63134 +
						 * "b82b7a0a str"@0xffffff8008a63138 */
						mp_items[index].buf[x * core_mp.c232 + y] = v * 100 / avg;
					}
				}
			}
			break;
		}
		case 9: {
			/*
			 * The four half-words are written
			 * UNCONDITIONALLY, before the protocol choice:
			 * "7901e3e9 strh"@0xffffff8008a62de4,
			 * "7901e7ea strh"@0xffffff8008a62df8,
			 * "7900e3eb strh"@0xffffff8008a62e04 and
			 * "7900e7ec strh"@0xffffff8008a62e1c
			 */
			u16 v1[2];
			u16 v2[2];

			v1[0] = mp_items[index].c64;
			v1[1] = mp_items[index].c68;
			v2[0] = mp_items[index].c72;
			v2[1] = mp_items[index].c76;

			/* "b940010d ldr"@0xffffff8008a62e14 (core_mp.c12) +
			 * "6b0901bf cmp"@0xffffff8008a62e20 +
			 * "540007a9 b.ls"@0xffffff8008a62e24 */
			if ((unsigned int)core_mp.c12 > 0x503FFU) {
				for (j = 0; j < core_mp.c256; j++)
					/* "396f8d4a ldrb"@0xffffff8008a62e48
					 * (core_mp.c227) +
					 * "7100015f cmp"@0xffffff8008a62e50 +
					 * "9a96032a csel"@0xffffff8008a62e54;
					 * "b8757929 ldr"@0xffffff8008a62e4c e
					 * "b828d949 str"@0xffffff8008a62f04 */
					mp_items[index].buf[i * core_mp.c256 + j] =
						codeToOhm(mp_frame_buf0[j],
							  (core_mp.c227 == 0) ? v2 : v1);
			} else {
				/* "b869794a ldr"@0xffffff8008a62f3c +
				 * "b828d96a str"@0xffffff8008a62f44 */
				for (j = 0; j < core_mp.c256; j++)
					mp_items[index].buf[i * core_mp.c256 + j] =
						mp_frame_buf0[j];
			}
			break;
		}
		default:
			/*
			 * "f942114a ldr"@0xffffff8008a62da8: the global pointer
			 * is RE-READ on every iteration, and this is the
			 * measured difference from cases 4..7.
			 * "1b082448 madd"@0xffffff8008a62db0 +
			 * "b828d96a str"@0xffffff8008a62dbc
			 */
			for (j = 0; j < core_mp.c256; j++)
				mp_items[index].buf[i * core_mp.c256 + j] =
					mp_frame_buf0[j];
			break;
		}

		/* "1b027d08 mul"@0xffffff8008a63170 +
		 * "8b28c921 add"@0xffffff8008a63174 +
		 * "940009bb bl"@0xffffff8008a6317c */
		compare_MaxMin_result(index, &mp_items[index].buf[core_mp.c256 * i]);
	}

out:
	return ret;
}
