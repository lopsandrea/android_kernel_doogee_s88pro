/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Ilitek TDDI touch panel, Doogee S88 Pro -- the interface shared by the
 * driver's eight translation units.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read: Ilitek TDDI drivers circulate publicly and the ALPS tree holds
 * touch drivers of the same family (GT1151/, GT5688/); neither was used, for
 * orientation or for names. Should a future batch use one, that has to be
 * declared and every line re-checked against this binary.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#ifndef __ILITEK_H
#define __ILITEK_H

#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a55310).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009987370).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_hwif_info {
	/*
	 * +0  8-bit. The interface type. Compared with 0x18 both here
	 * ("39400268 ldrb"@0xffffff8008a556c0, "7100611f cmp"@0xffffff8008a556c4)
	 * and in `ilitek_tddi_dev_init` of group A
	 * ("39400008 ldrb"@0xffffff8008a55310).
	 */
	u8 type;
	u8 __ignoto_1[7];
	/* +8   -> `drv->driver.name`. "f9400668 ldr"@0xffffff8008a556cc */
	const char *name;
	/* +16  -> `drv->driver.owner`. "f9400a68 ldr"@0xffffff8008a556e0 */
	struct module *owner;
	/* +24  -> `drv->driver.of_match_table`. "f9400e68 ldr"@0xffffff8008a556f4 */
	const struct of_device_id *of_match_table;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5596c).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	int (*c32)(void);
	/*
	 * +40  Second function pointer. NONE of the 154 calls it, and
	 * group B kept it as a hole. It is not one: the `hwif` object is
	 * a global in `.data` at 0xffffff8009987370, and the relocation at
	 * 0xffffff8009987398 carries the addend 0xffffff8008a568fc, that is
	 * `ilitek_plat_remove`. The SIGNATURE is the definition's, not a
	 * call site's -- here the call site does not exist.
	 */
	int (*c40)(void);
	/*
	 * +48  The allocated `i2c_driver`. Written here
	 * ("f9001a60 str"@0xffffff8008a556d0) and read back by
	 * `ilitek_tddi_interface_dev_exit` ("f9401913 ldr"@0xffffff8008a55a18)
	 * for `i2c_del_driver` and `kfree`.
	 */
	struct i2c_driver *driver;
};



/*
 * Only the type of the pointer at +32 is needed here. `linux/pm_wakeup.h` is
 * NOT included: in this tree it is not self-contained (it uses
 * `struct device` without including it) and pulling it in would force every
 * unit to include `linux/device.h` for something concerning a single
 * function. Whoever calls `wakeup_source_unregister` includes
 * `linux/device.h` themselves.
 */
struct wakeup_source;

/*
 * Only the type of the pointer at +16 is needed here, and `linux/input.h` is
 * a heavy include that would pull in half a subsystem for one pointer.
 * Whoever dereferences its fields -- only `ilitek_plat.c` -- includes it
 * itself.
 */
struct input_dev;

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * This section was reconstructed from the factory kernel disassembly (888 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_tddi_dev {
	/* +0   pointer. L=4 S=1. "f9000113 str"@0xffffff8008a558a8 */
	void *c0;
	/*
	 * +8   64-bit. L=3 S=1. "f900051f str"@0xffffff8008a558bc -- the store
	 * in `probe` is `str wzr`, that is an explicit NULL.
	 * It is the first argument passed to the function at +792, which none
	 * of the 154 writes: see the comment at +792.
	 */
	void *c8;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a55de0).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	struct input_dev *c16;
	/*
	 * +24  64 bit. L=5 S=1. "f9000d14 str"@0xffffff8008a558c0
	 */
	void *c24;
	/*
	 * +32  pointer to `struct wakeup_source`. L=3 S=1.
	 * "f9001128 str"@0xffffff8008a551b0 writes it in `ilitek_tddi_init`;
	 * the TYPE is measured in `ilitek_tddi_dev_remove`, which passes it to
	 * `wakeup_source_unregister` ("f9401100 ldr"@0xffffff8008a552cc,
	 * "97eaae70 bl"@0xffffff8008a552d4 towards <wakeup_source_unregister>).
	 */
	struct wakeup_source *c32;
	/*
	 * +40  pointer. L=1 S=1. "f900150b str"@0xffffff8008a558ec
	 * `ilitek_i2c_probe` writes there the pointer read back from
	 * `client->dev.driver` + 232, that is the `struct ilitek_hwif_info *`
	 * attached to the 240-byte `i2c_driver`.
	 */
	void *c40;
	/* +48  pointer. L=57 S=1. "a9032149 stp"@0xffffff8008a5aad0 */
	void *c48;
	/* +56  pointer. L=13 S=3. "f9001d0a str"@0xffffff8008a5a814 */
	void *c56;
	/*
	 * +64  pointer to a block of 56 byte: `kzalloc(56, GFP_KERNEL)` in
	 * `probe` -- gfp 0x14080c0, size 0x38 ("321d0be2 orr"@0xffffff8008a5588c,
	 * "f9002100 str"@0xffffff8008a5589c) -- e `kfree` in
	 * `ilitek_tddi_dev_remove` ("f9402100 ldr"@0xffffff8008a552e8).
	 * L=2 S=1.
	 */
	void *c64;
	/*
	 * +72..111, 40 bytes: A DECLARED UNKNOWN HOLE. It is not padding --
	 * the following alignment (mutex, 8) does not require a single byte.
	 * 80 instructions in the block carry an immediate offset in this
	 * range, ZERO of them on `idev`: six functions involved never load
	 * the global (`ilitek_node_compat_ioctl`, `ilitek_tddi_mp_test_main`,
	 * `open_test_sp`, `ilitek_tddi_fw_upgrade`,
	 * `ilitek_tddi_interface_dev_init`), and where the analysis resolves the
	 * base, the base is another global.
	 * DO NOT FILL IT with plausible fields: that would shift everything after it.
	 */
	u8 __ignoto_72[40];
	/*
	 * +112 `struct mutex`. Name FROM THE BINARY: "&idev->touch_mutex"
	 * @0xffffff800923ed67. "9101c100 add"@0xffffff8008a53f44
	 */
	struct mutex touch_mutex;
	/*
	 * +144 `struct mutex`. Name FROM THE BINARY: "&idev->debug_mutex"
	 * @0xffffff800923ed7a. "91024100 add"@0xffffff8008a54eac
	 */
	struct mutex debug_mutex;
	/*
	 * +176 `struct mutex`. Name FROM THE BINARY: "&idev->debug_read_mutex"
	 * @0xffffff800923ed8d. "9102c100 add"@0xffffff8008a54ec8
	 */
	struct mutex debug_read_mutex;
	/* +208 `spinlock_t`, from `spin_lock_init`.
	 * "91034100 add"@0xffffff8008a56104. Padding 212..215 up to 216. */
	spinlock_t c208;
	/*
	 * +216 `const char *`. L=0 S=1. "f9006d09 str"@0xffffff8008a558c8:
	 * `probe` writes 0xffffff80091aa860 there, which holds "I2C".
	 * `ilitek_tddi_dev_init` chooses between that literal and "SPI"
	 * (0xffffff800923ef05) with a `csel` based on the first byte of the
	 * `hwif`. They are two real data literals, not field names.
	 */
	const char *c216;
	/*
	 * +224 one byte. L=2 S=3. "39038109 strb"@0xffffff8008a53b0c.
	 * In `ilitek_tddi_fw_upgrade_handler` it is the guard that makes the
	 * input be registered ONCE only: if it is zero, "Registre touch to
	 * input subsystem" is printed, it is set to 1 and the registration
	 * happens. Padding 225..227 up to 228.
	 */
	u8 c224;
	/*
	 * +228 32-bit. L=1 S=2. "291ca504 stp"@0xffffff8008a57e04 (merged with the
	 * field at +232 by a single `stp`)
	 */
	u32 c228;
	/* +232 32 bit. L=3 S=2. "291ca504 stp"@0xffffff8008a57e04 */
	u32 c232;
	/* +236 16 bit. L=0 S=1. "7901d905 strh"@0xffffff8008a5a58c */
	u16 c236;
	/* +238 16 bit. L=0 S=1. "7901dd06 strh"@0xffffff8008a5a59c */
	u16 c238;
	/* +240 16 bit. L=0 S=1. "7901e103 strh"@0xffffff8008a5a568 */
	u16 c240;
	/* +242 16 bit. L=0 S=1. "7901e504 strh"@0xffffff8008a5a57c */
	u16 c242;
	/* +244 16 bit. L=9 S=3. "7901e948 strh"@0xffffff8008a5a258 (merged with
	 * +246) */
	u16 c244;
	/* +246 16 bit. L=9 S=2. "7901ed49 strh"@0xffffff8008a5a25c */
	u16 c246;
	/*
	 * +248..+251 four bytes. All four enter, and ONLY there,
	 * the packet length computed by `ilitek_set_tp_data_len`
	 * for formats 1 and 7:
	 *   len = (c249 * c248 + c250 + c251) * 2 + 56
	 * ("3943e109 ldrb"@0xffffff8008a53ce4 .. "3943ed0c ldrb"@0xffffff8008a53cf0,
	 *  "1b092d49 madd"@0xffffff8008a53cf8, "531f7929 lsl"@0xffffff8008a53d00,
	 *  "1100e135 add"@0xffffff8008a53d04).
	 * They are written in `ilitek_tddi_ic_get_tp_info` (group D):
	 * "3903e109 strb"@0xffffff8008a5a5a4 and those that follow.
	 */
	u8 c248;	/* L=7 S=1 */
	u8 c249;	/* L=6 S=1 */
	u8 c250;	/* L=1 S=1 */
	u8 c251;	/* L=1 S=1 */
	/*
	 * +252..263, 12 bytes: A DECLARED UNKNOWN HOLE. It is not padding: the
	 * next field is 8-aligned and 252 -> 256 would need four bytes.
	 * Zero instructions in the block carry an immediate offset here.
	 */
	u8 __ignoto_252[12];
	/*
	 * +264 pointer to a block of 2048 byte: `kzalloc(2048, GFP_ATOMIC)`
	 * in `probe` -- gfp 0x1088020, size 0x800 ("321503e2 orr"@0xffffff8008a55860,
	 * "f9008500 str"@0xffffff8008a55870) -- e `kfree` in
	 * `ilitek_tddi_dev_remove` ("f9408500 ldr"@0xffffff8008a552dc).
	 * L=8 S=1.
	 */
	void *c264;
	/*
	 * +272..287, 16 bytes: A DECLARED UNKNOWN HOLE. Zero instructions in the
	 * block with this immediate offset.
	 */
	u8 __ignoto_272[16];
	/* +288 64 bit. L=1 S=4. "f900911f str"@0xffffff8008a68680 (store of
	 * explicit zero) */
	u64 c288;
	/* +296 64 bit. L=4 S=6. "f9009500 str"@0xffffff8008a68690 */
	u64 c296;
	/*
	 * +304..319, 16 bytes: A DECLARED UNKNOWN HOLE. Zero instructions in the
	 * block with this immediate offset.
	 */
	u8 __ignoto_304[16];
	/*
	 * +320 32-bit. L=10 S=10. "b9014109 str"@0xffffff8008a5376c.
	 * It is the current TP mode: `ilitek_tddi_switch_tp_mode` writes its own
	 * argument into it ("b9014113 str"@0xffffff8008a538b4) and reads it back
	 * to print it ("b9414103 ldr"@0xffffff8008a539bc, format
	 * "Actual TP mode = %d"@0xffffff800923e1dd).
	 */
	int c320;
	/*
	 * +324 32-bit. L=1 S=2. "b901451f str"@0xffffff8008a5503c (merged with
	 * +328 by a wide store). It is the packet data format:
	 * `ilitek_set_tp_data_len` writes its own argument into it
	 * ("b9014513 str"@0xffffff8008a53df8).
	 */
	int c324;
	/*
	 * +328 32-bit. L=1 S=1. "b901490a str"@0xffffff8008a558f4. It is the
	 * packet length computed by `ilitek_set_tp_data_len`
	 * ("b9014915 str"@0xffffff8008a53dfc).
	 */
	int c328;
	/* +332 32 bit. L=10 S=1. "b9014d00 str"@0xffffff8008a562b4 */
	int c332;
	/*
	 * +336..339, 4 bytes: alignment padding (the field at +340 is 4 bytes,
	 * so the compiler would not produce any here -- but the range is empty
	 * and has to be declared). Made explicit because it is NOT alignment
	 * arithmetic: nobody touches it and nobody knows what it is.
	 */
	u8 __ignoto_336[4];
	/* +340 32 bit. L=2 S=1. "b901550b str"@0xffffff8008a55960 */
	int c340;
	/*
	 * +344 32-bit. L=9 S=1. "b9415900 ldr"@0xffffff8008a55270 (merged with
	 * +348). It is a GPIO number: `ilitek_tddi_dev_remove` passes it to
	 * `gpio_free` ("97e88c56 bl"@0xffffff8008a55274).
	 */
	int c344;
	/*
	 * +348 32-bit. L=5 S=0. "b9415d00 ldr"@0xffffff8008a55264. This one too
	 * is a GPIO number, released BEFORE +344
	 * ("97e88c59 bl"@0xffffff8008a55268). Which is the reset and which the
	 * interrupt is not decided by this batch: group C will say.
	 * S=0 across the whole block: it is written only by the `devm_kzalloc` or
	 * by code outside the boundary.
	 */
	int c348;
	/*
	 * +352 32-bit SIGNED (`ldrsw`). L=20 S=4.
	 * "b901611f str"@0xffffff8008a5c6e4
	 */
	s32 c352;
	/* +356 array of 10 `int`, indexed. "b9016509 str"@0xffffff8008a5c7d8 */
	int c356[10];
	/* +396 array of 10 `int`, indexed. "b9418d49 ldr"@0xffffff8008a5c8a4 */
	int c396[10];
	/* +436 32 bit. L=4 S=2. "b901b509 str"@0xffffff8008a5ca48 */
	int c436;
	/*
	 * +440..443, 4 bytes: A DECLARED UNKNOWN HOLE. It is not padding: +436
	 * and +444 are both 4 bytes and would be contiguous without it. Zero
	 * instructions in the block with this immediate offset.
	 */
	u8 __ignoto_440[4];
	/*
	 * +444 32-bit. L=2 S=9. "b901bd1f str"@0xffffff8008a53a98.
	 * It is the firmware update progress in percent:
	 * `ilitek_tddi_fw_upgrade_handler` clears it at the start
	 * ("b901bd1f str"@0xffffff8008a53a98) and puts 100 (0x64) or -1 in it at
	 * the end ("52800c89 mov"@0xffffff8008a53ae0,
	 * "b901bd09 str"@0xffffff8008a53aec).
	 */
	int c444;
	/*
	 * +448 32 bit. L=1 S=1. "b901c10a str"@0xffffff8008a55944.
	 * E' the only one argument of `ilitek_tddi_fw_upgrade`
	 * ("b941c100 ldr"@0xffffff8008a53a94).
	 */
	int c448;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a69800, 75 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	u8 c452[75];
	/*
	 * +527 one byte, 0/1 (clang emits `eor #1` on a byte just read and
	 * written back: a `bool` with `x = !x` or a `u8` with `x ^= 1`, and the two
	 * stay indistinguishable). L=7 S=1.
	 * "39083d03 strb"@0xffffff8008a6d28c.
	 * It is the common guard of BOTH branches of `ilitek_tddi_wq_ctrl`:
	 * "39483d28 ldrb"@0xffffff8008a5408c (the ESD branch) and
	 * "39483d28 ldrb"@0xffffff8008a54124 (the battery branch).
	 */
	u8 c527;
	/*
	 * +528 one byte. L=5 S=3. "3908412a strb"@0xffffff8008a540ac:
	 * `ilitek_tddi_wq_ctrl` writes `(ctrl != 0)` there on the ESD branch.
	 */
	u8 c528;
	/*
	 * +529 one byte. L=5 S=3. "3908452a strb"@0xffffff8008a54144: the same
	 * on the battery branch.
	 */
	u8 c529;
	/* +530 one byte, 0/1 (`eor #1`). L=5 S=3.
	 * "3908491f strb"@0xffffff8008a55958 */
	u8 c530;
	/* +531 one byte. L=3 S=1. "39084d0a strb"@0xffffff8008a55954 */
	u8 c531;
	/*
	 * +532 one byte, 0/1 (`eor #1`). L=5 S=1.
	 * "39085103 strb"@0xffffff8008a6d318. Padding 533..535 up to 536.
	 */
	u8 c532;
	/*
	 * +536 32-bit. L=5 S=2. "b9021909 str"@0xffffff8008a6d400 (merged with
	 * +540). Read by `ilitek_set_tp_data_len` on format 3
	 * ("b9421908 ldr"@0xffffff8008a53d3c) and passed to the function at +816
	 * by `ilitek_tddi_switch_tp_mode` ("b9421900 ldr"@0xffffff8008a53914).
	 */
	int c536;
	/*
	 * +540 32-bit. L=0 S=1. "b9021d28 str"@0xffffff8008a6d06c.
	 * `ilitek_set_tp_data_len` reads it on format 3
	 * ("b9421d03 ldr"@0xffffff8008a53d38) and passes it as the value to
	 * `ilitek_tddi_ic_func_ctrl("gesture_demo_en", ...)`
	 * ("b9421d01 ldr"@0xffffff8008a53d8c).
	 */
	int c540;
	/* +544 16 bit. L=2 S=2. "79044143 strh"@0xffffff8008a6adc8 */
	u16 c544;
	/* +546 16 bit. L=2 S=1. "79044544 strh"@0xffffff8008a6addc */
	u16 c546;
	/* +548 32 bit. L=5 S=1. "b9022549 str"@0xffffff8008a6adf0 */
	int c548;
	/* +552 32 bit. L=2 S=1. "b9022948 str"@0xffffff8008a6ae08 */
	int c552;
	/*
	 * +556 one byte, 0/1 (`eor #1`). L=4 S=5.
	 * "3908b11f strb"@0xffffff8008a5595c. Padding 557..559 up to 560.
	 */
	u8 c556;
	/* +560 32 bit. L=3 S=3. "b902312a str"@0xffffff8008a5cd68 (merged with
	 * +564) */
	int c560;
	/*
	 * +564 32-bit SIGNED (`ldrsw`). L=13 S=2.
	 * "b9023509 str"@0xffffff8008a6c5d4
	 */
	s32 c564;
	/*
	 * +568 `wait_queue_head_t`. Name FROM THE BINARY: "&(idev->inq)"
	 * @0xffffff800923eda5 -- and the source writes the parentheses.
	 * "9108e100 add"@0xffffff8008a54ee4
	 */
	wait_queue_head_t inq;
	/* +592 pointer. L=25 S=5. "f9012a7f str"@0xffffff8008a718c8 */
	void *c592;
	/* +600 32 bit. L=3 S=1. "b90259c3 str"@0xffffff8008a71378 */
	int c600;
	/* +604 32 bit. L=3 S=1. "b9025dc4 str"@0xffffff8008a7137c */
	int c604;
	/* +608 32 bit. L=0 S=1. "b90261c5 str"@0xffffff8008a71380 */
	int c608;
	/*
	 * +612 32-bit. L=8 S=1. "b9426500 ldr"@0xffffff8008a53824 (merged with
	 * +616: "f9000169 str"@0xffffff8008a55904 writes 0x0000006400000002,
	 * that is 612 = 2 and 616 = 100 in one go).
	 * It is the reset mode passed to `ilitek_tddi_reset_ctrl`
	 * ("b9426500 ldr"@0xffffff8008a53824 and "b9426500 ldr"@0xffffff8008a539e8).
	 */
	int c612;
	/*
	 * +616 32-bit SIGNED (`ldrsw`). L=4 S=0.
	 * "b9426903 ldr"@0xffffff8008a55d2c. It is 100 after `ilitek_tddi_init`.
	 */
	s32 c616;
	/*
	 * +620 32-bit. L=3 S=1. "b9026d1f str"@0xffffff8008a55948.
	 * It is the choice between re-flashing the firmware and starting again from
	 * the reset: `ilitek_tddi_mp_test_handler` and `ilitek_tddi_switch_tp_mode`
	 * compare it with 1 ("b9426d09 ldr"@0xffffff8008a537f4,
	 * "b9426d09 ldr"@0xffffff8008a538f4).
	 */
	int c620;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a60888).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	int c624;
	/* +628 one byte. L=1 S=1. "3909d11f strb"@0xffffff8008a55950 */
	u8 c628;
	/*
	 * +629 one byte. L=1 S=1. "3949d508 ldrb"@0xffffff8008a58b8c (merged with
	 * +630 by "79000169 strh"@0xffffff8008a55020)
	 */
	u8 c629;
	/* +630 one byte. L=1 S=2. "3909d91f strb"@0xffffff8008a53c64: zeroed
	 * at the tail of `ilitek_tddi_reset_ctrl`. */
	u8 c630;
	/*
	 * +631 one byte. L=1 S=9. "3909dd13 strb"@0xffffff8008a53f28.
	 * Set to 1 while `ilitek_tddi_gesture_recovery` and
	 * `ilitek_tddi_spi_recovery` are running, cleared right after
	 * ("3909dd1f strb"@0xffffff8008a53f3c).
	 */
	u8 c631;
	/* +632 one byte. L=1 S=2. "3909e11f strb"@0xffffff8008a547c8 */
	u8 c632;
	/*
	 * +633, 1 byte: A DECLARED UNKNOWN HOLE, wedged between two `u8`s that
	 * are touched. Zero instructions in the block with this immediate offset.
	 * It is NOT padding: between two bytes none is needed.
	 */
	u8 __ignoto_633;
	/* +634 one byte. L=1 S=0. "3949e908 ldrb"@0xffffff8008a544dc */
	u8 c634;
	/* +635 one byte. L=0 S=2. "3909ed09 strb"@0xffffff8008a5469c */
	u8 c635;
	/* +636 one byte. L=7 S=8. "3909f11f strb"@0xffffff8008a55138 */
	u8 c636;
	/* +637 one byte. L=3 S=2. "3909f509 strb"@0xffffff8008a5b8ac */
	u8 c637;
	/* +638 one byte. L=1 S=2. "3909f906 strb"@0xffffff8008a69444 */
	u8 c638;
	/* +639 one byte. L=1 S=2. "3909fd1f strb"@0xffffff8008a55024 */
	u8 c639;
	/*
	 * +640 one byte. L=0 S=6. "390a011f strb"@0xffffff8008a53bb0.
	 * `ilitek_tddi_reset_ctrl` clears it before the reset and puts it back to 1
	 * at the end ("390a010a strb"@0xffffff8008a53c70), but only on the branch
	 * that is NOT the code reset.
	 */
	u8 c640;
	/* +641 one byte. L=3 S=4. "390a052a strb"@0xffffff8008a5a348 */
	u8 c641;
	/* +642 one byte. L=1 S=1. "390a0928 strb"@0xffffff8008a6d4f4 */
	u8 c642;
	/* +643 one byte. L=1 S=2. "390a0d09 strb"@0xffffff8008a6f914 */
	u8 c643;
	/* +644 32 bit. L=0 S=1. "b902851f str"@0xffffff8008a55100 */
	int c644;
	/*
	 * +648 32-bit. L=1 S=1. "b902890b str"@0xffffff8008a550f4.
	 * Padding 652..655 up to the 8-byte field at 656.
	 */
	int c648;
	/* +656 64 bit. L=2 S=1. "f9014903 str"@0xffffff8008a550dc */
	u64 c656;
	/* +664 64 bit. L=1 S=1. "f9014d05 str"@0xffffff8008a550e0 */
	u64 c664;
	/* +672 64 bit. L=2 S=1. "f9015106 str"@0xffffff8008a550e4 */
	u64 c672;
	/* +680 64 bit. L=1 S=1. "f9015504 str"@0xffffff8008a550e8 */
	u64 c680;
	/* +688 64 bit. L=1 S=1. "f9015909 str"@0xffffff8008a550ec */
	u64 c688;
	/* +696 64 bit. L=2 S=1. "f9015d0a str"@0xffffff8008a550f0 */
	u64 c696;
	/* +704 32 bit. L=2 S=5. "b902c11f str"@0xffffff8008a54f18 */
	int c704;
	/*
	 * +708 32 bit. L=5 S=3. "b902c509 str"@0xffffff8008a53b64.
	 * Set to 1 on entry to `ilitek_tddi_reset_ctrl` e zeroed in
	 * coda ("b902c51f str"@0xffffff8008a53c6c).
	 */
	int c708;
	/*
	 * +712 32-bit. L=51 S=6 -- the most-read field of the structure.
	 * "b902c91f str"@0xffffff8008a53c4c: `ilitek_tddi_reset_ctrl` clears it,
	 * but ONLY on the branch that is not the code reset.
	 */
	int c712;
	/*
	 * +716 32-bit. L=5 S=3. "b902cd09 str"@0xffffff8008a53a90.
	 * It is the "firmware update in progress" flag:
	 * `ilitek_tddi_fw_upgrade_handler` sets it to 1 on entry and clears it on
	 * exit ("b902cd1f str"@0xffffff8008a53b34), and
	 * `ilitek_tddi_mp_test_handler` refuses to start while it is set
	 * ("b942cd09 ldr"@0xffffff8008a53710).
	 */
	int c716;
	/*
	 * +720 32-bit. L=4 S=3. "b902d109 str"@0xffffff8008a5373c.
	 * The twin of the previous one for the MP test: 1 on entry to
	 * `ilitek_tddi_mp_test_handler`, 0 on exit
	 * ("b902d11f str"@0xffffff8008a5384c).
	 */
	int c720;
	/* +724 32 bit. L=3 S=4. "b902d509 str"@0xffffff8008a542b4 */
	int c724;
	/*
	 * +728 32 bit. L=3 S=4. "b902d909 str"@0xffffff8008a53768.
	 * 1 on entry to `ilitek_tddi_switch_tp_mode`, 0 on exit
	 * ("b902d91f str"@0xffffff8008a539d4).
	 */
	int c728;
	/* +732 32 bit. L=4 S=17. "b902dd1f str"@0xffffff8008a54f30 */
	int c732;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a53efc).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	int c736;
	/*
	 * +744 `struct completion`, 32 bytes: `done` at +744
	 * ("b902e91f str"@0xffffff8008a54f0c) and `wait` at +752, where
	 * `__init_waitqueue_head` lands ("910ba100 add"@0xffffff8008a5539c).
	 *
	 * AN OPEN DIVERGENCE: it is initialised and SIGNALLED, never waited on.
	 * `init_completion` in `ilitek_tddi_init`, `complete_all` once
	 * only -- in `ilitek_tddi_wq_esd_check`
	 * ("910ba100 add"@0xffffff8008a5539c, "97db23e1 bl"@0xffffff8008a553a0
	 * towards <complete_all>) -- and `grep -c wait_for_completion` over the
	 * whole disassembled block gives 0. Either it is dead code, or the wait is in
	 * a function outside the boundary of the 154. This batch does NOT close it:
	 * it has found who signals it, not who waits for it.
	 */
	struct completion c744;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a558e0).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	int (*c776)(void *buf, int len);
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a558d8).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	int (*c784)(void *buf, int len);
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a6d934).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	void *c792;
	/*
	 * +800 function pointer. L=4 S=1. "f901911f str"@0xffffff8008a558fc
	 * (`probe` writes `wzr` there, an explicit NULL).
	 * `ilitek_tddi_wq_esd_spi_check` calls it with no arguments and compares
	 * the result with 0xa3 ("f9419108 ldr"@0xffffff8008a54018).
	 */
	int (*c800)(void);
	/*
	 * +808 function pointer = `ilitek_tddi_move_mp_code_flash`. L=2 S=1.
	 * "f9019509 str"@0xffffff8008a55918. Called with no arguments by
	 * `ilitek_tddi_switch_tp_mode` ("f9419508 ldr"@0xffffff8008a53950).
	 */
	int (*c808)(void);
	/*
	 * +816 function pointer = `ilitek_tddi_move_gesture_code_flash`.
	 * L=2 S=1. "f901990b str"@0xffffff8008a55920. Called with one
	 * argument, `c536` ("f941990b ldr"@0xffffff8008a53910,
	 * "b9421900 ldr"@0xffffff8008a53914).
	 */
	int (*c816)(int mode);
	/*
	 * +824 function pointer = `ilitek_tddi_wq_esd_i2c_check`. L=1 S=1.
	 * "f9019d09 str"@0xffffff8008a55934. Called with no arguments by
	 * `ilitek_tddi_wq_esd_check` ("f9419d08 ldr"@0xffffff8008a55370).
	 */
	int (*c824)(void);
	/*
	 * +832 64-bit. L=4 S=1. "f901a11f str"@0xffffff8008a558f8: the only
	 * store is `wzr`, that is NULL, and all four reads are guarded by a
	 * `cbz`. The field is simply always off in this configuration -- unlike
	 * +792, here the guard is present.
	 */
	void *c832;
	/*
	 * +840 function pointer = `ilitek_tddi_touch_esd_gesture_flash`.
	 * L=2 S=1. "f901a50b str"@0xffffff8008a5593c. Called with no arguments
	 * by `ilitek_tddi_gesture_recovery` ("f941a509 ldr"@0xffffff8008a53f24).
	 */
	int (*c840)(void);
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a5c668).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	void *c848[5];
};

/*
 * 0xffffff800a0fc958() was reconstructed from the factory kernel disassembly (0xffffff800a0fca40).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * 0xffffff800a0fc958 (page + 2392). One byte: `ldrb`, never read any wider.
 * It is the guard of the KERN_INFO level messages the driver prints only
 * when diagnostics are on: "39656108 ldrb"@0xffffff8008a537a8,
 * "39656269 ldrb"@0xffffff8008a540b8, and another 130 sites in the block.
 * NAME CHOSEN -- the binary does not give it.
 */
extern u8 ilitek_dbg_en;

/* 0xffffff800a0fc960 (pagina + 2400): `struct workqueue_struct *`.
 * "f944b288 ldr"@0xffffff8008a54098, passed to `flush_workqueue`
 * ("97d9ee2d bl"@0xffffff8008a54220) e a `destroy_workqueue`
 * ("97d9f2b5 bl"@0xffffff8008a5529c). NAME CHOSEN. */
extern struct workqueue_struct *ilitek_wq_esd;

/* 0xffffff800a0fc968 (pagina + 2408): `struct delayed_work`, 96 byte.
 * NAME CHOSEN. */
extern struct delayed_work ilitek_esd_work;

/* 0xffffff800a0fc9c8 (pagina + 2504): `struct workqueue_struct *`.
 * "f944e688 ldr"@0xffffff8008a54130. NAME CHOSEN. */
extern struct workqueue_struct *ilitek_wq_bat;

/* 0xffffff800a0fc9d0 (pagina + 2512): `struct delayed_work`, 96 byte.
 * NAME CHOSEN. */
extern struct delayed_work ilitek_bat_work;

/* 0xffffff800a0fca40 (page + 2624). NAME FROM THE BINARY. */
extern struct ilitek_tddi_dev *idev;

/*
 * ILI_ERR() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define ILI_ERR(fmt, ...) \
	printk(KERN_ERR "ILITEK: (%s, %d): " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define ILI_INFO(fmt, ...) \
	printk(KERN_INFO "ILITEK: (%s, %d): " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define ILI_DBG(fmt, ...)						\
	do {								\
		if (ilitek_dbg_en)					\
			printk(KERN_INFO "ILITEK: (%s, %d): " fmt,	\
			       __func__, __LINE__, ##__VA_ARGS__);	\
	} while (0)

/*
 * ilitek_tddi_mp_test_handler() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_mp_test_handler(char *apk, bool lcm_on);
int ilitek_tddi_switch_tp_mode(u8 mode);
int ilitek_tddi_fw_upgrade_handler(void);
int ilitek_tddi_reset_ctrl(int mode);
int ilitek_set_tp_data_len(int format);
int ilitek_tddi_gesture_recovery(void);
void ilitek_tddi_spi_recovery(void);
int ilitek_tddi_wq_esd_spi_check(void);
int ilitek_tddi_wq_esd_i2c_check(void);
void ilitek_tddi_wq_ctrl(int type, int ctrl);
int ilitek_tddi_dev_init(struct ilitek_hwif_info *hwif);
void ilitek_tddi_dev_remove(void);

/*
 * ilitek_tddi_sleep_handler() was reconstructed from the factory kernel disassembly (0xffffff8008a56654).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_sleep_handler(int mode);
void ilitek_tddi_report_handler(void);
int ilitek_tddi_init(void);

/*
 * ilitek_plat_tp_reset() was reconstructed from the factory kernel disassembly (0xffffff8008a5627c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_plat_tp_reset(void);
void ilitek_plat_input_register(void);
void ilitek_plat_irq_disable(void);
void ilitek_plat_irq_enable(void);
void ilitek_plat_irq_unregister(void);
int ilitek_plat_irq_register(int type);


#endif /* __ILITEK_H */
