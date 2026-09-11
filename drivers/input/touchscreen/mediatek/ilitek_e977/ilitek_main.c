// SPDX-License-Identifier: GPL-2.0
/*
 * Ilitek TDDI touch panel, Doogee S88 Pro -- group A, the driver core.
 *
 * Reconstructed from the disassembly of the factory kernel, not ported from
 * another phone and not adapted from a public source. Ilitek TDDI drivers
 * circulate publicly and the ALPS tree holds touch drivers of the same family
 * (GT1151/, GT5688/): none was read, neither for the code nor for the names.
 * Every derived constant was checked against the factory binary.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/device.h>	/* per wakeup_source_unregister, via pm_wakeup.h */
#include <linux/errno.h>
#include <linux/delay.h>	/* per mdelay */
#include <linux/err.h>		/* per IS_ERR */
#include <linux/fs.h>		/* per filp_open, filp_close, struct file */
#include <linux/gpio.h>
#include <linux/interrupt.h>	/* per irq_set_irq_wake */
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>	/* per __memset (via asm/string.h), strstr */
#include <linux/uaccess.h>	/* per get_fs, set_fs, KERNEL_DS */
#include <linux/workqueue.h>

#include "ilitek.h"

/*
 * THE GLOBALS. The definitions live here by declared choice: the binary does
 * not say in which of the eight units they are defined (the oracle has no
 * data symbols -- 30,100 `T`, 237 `W`, 23,372 `t`, zero data), so "in
 * ilitek_main.c" is a choice and not a measurement. What the binary DOES say
 * is their layout in .bss, which is verified in `ilitek.h`.
 */
struct ilitek_tddi_dev *idev;
/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct workqueue_struct *ilitek_wq_esd;
struct delayed_work ilitek_esd_work;
struct workqueue_struct *ilitek_wq_bat;
struct delayed_work ilitek_bat_work;

/*
 * The functions from the other seven groups that this file calls. They will
 * live in `ilitek.h` once those groups are written; for now they are declared
 * here, and every signature comes from its own call site in group A.
 */
/* group B (I2C bus) */
extern int ilitek_tddi_interface_dev_init(struct ilitek_hwif_info *hwif);
extern void ilitek_tddi_interface_dev_exit(struct ilitek_tddi_dev *dev);
/* group C (platform) */
extern void ilitek_plat_tp_reset(void);
extern void ilitek_plat_input_register(void);
/* group D (chip registers) */
extern int ilitek_tddi_ic_code_reset(void);
extern int ilitek_tddi_ic_whole_reset(void);
extern int ilitek_tddi_ic_check_otp_prog_mode(void);
extern int ilitek_tddi_ic_func_ctrl(const char *name, int ctrl);
/*
 * "52800640 mov"@0xffffff8008a54468 (#50) and "52800281 mov"@0xffffff8008a5446c
 * (#20): two `int`s, the result tested on the sign bit alone
 * ("36f800e0 tbz"@0xffffff8008a54474).
 */
extern int ilitek_tddi_ic_check_busy(int count, int delay);
/*
 * group D (chip registers), read from the single site in `report_handler`:
 * "940011b2 bl"@0xffffff8008a54bb0, no arguments, result unused.
 */
extern void ilitek_tddi_ic_get_pc_counter_forwdt(void);
/*
 * ilitek_tddi_touch_release_all_point() was reconstructed from the factory kernel disassembly (0xffffff8008a54dfc).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
extern void ilitek_tddi_touch_release_all_point(void);
extern void ilitek_tddi_report_ap_mode(void *buf, int len);
extern void ilitek_tddi_report_i2cuart_mode(void *buf, int len);
extern void ilitek_tddi_report_debug_mode(void *buf, int len);
extern void ilitek_tddi_report_gesture_mode(void *buf, int len);
extern void demo_debug_info_mode(void *buf, long len);
extern u8 ilitek_calc_packet_checksum(void *buf, u32 len);
extern void ilitek_dump_data(void *data, int a, int b, int c,
			     const char *label);
/*
 * group D (chip registers), called by `ilitek_tddi_init`. None takes
 * arguments except `ilitek_ice_mode_ctrl`, which takes two
 * ("320003e0 orr"@0xffffff8008a55034 + "2a1f03e1 mov"@0xffffff8008a55038 the
 * first time, two zeroes the second); for `ilitek_tddi_ic_get_info` and
 * `ilitek_ice_mode_ctrl` the site tests the sign bit of the return value, for
 * the other five the return value is unused. The PARAMETER NAMES are
 * positional: the binary passes constants and does not say what they are.
 */
extern void ilitek_tddi_ic_init(void);
extern int ilitek_ice_mode_ctrl(int a0, int a1);
extern int ilitek_tddi_ic_get_info(void);
extern void ilitek_tddi_ic_get_core_ver(void);
extern void ilitek_tddi_ic_get_protocl_ver(void);
extern void ilitek_tddi_ic_get_fw_ver(void);
extern void ilitek_tddi_ic_get_tp_info(void);
extern void ilitek_tddi_ic_get_panel_info(void);
/* group G (firmware and flash). "9400562d bl"@0xffffff8008a55108 */
extern void ilitek_tddi_fw_read_flash_info(void);
/* group H (/proc nodes). "94007179 bl"@0xffffff8008a55104 */
extern void ilitek_tddi_node_init(void);
/*
 * group E. Of this symbol group A uses ONLY THE ADDRESS
 * ("910c614a add"@0xffffff8008a55014, then "f901a90a str"@0xffffff8008a5502c
 * inside `idev->c848[0]`): no call site lands here, so the signature is NOT
 * measured. Declaring it as a function would mean inventing one; it is
 * declared as an opaque object, which produces the same `adrp`+`add`.
 */
extern u8 demo_debug_info_id0[];
/* group F (MP testing) */
extern int ilitek_tddi_mp_test_main(char *apk, bool lcm_on);
/* group G (firmware and flash) */
extern int ilitek_tddi_fw_upgrade(int open_file_method);

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_mp_test_handler -- 0xffffff8008a536f8, 404 bytes
 * factory: lines 93..127
 * ---------------------------------------------------------------------------
 * The second parameter is a `bool`: the factory masks it with 1 before
 * passing it ("12000261 and"@0xffffff8008a537dc) and tests it with
 * "360002f3 tbz"@0xffffff8008a537f0, which is how clang tests an `i1`.
 * The first is a pointer, passed untouched to `ilitek_tddi_mp_test_main`
 * ("aa0003f4 mov"@0xffffff8008a53748, "aa1403e0 mov"@0xffffff8008a537e0).
 *
 * MIND THE SHAPE: the factory does NOT call `ilitek_tddi_switch_tp_mode`
 * here -- it has inlined only its `mode == 1` path, as the `__func__` of the
 * three `printk`s between 0xffffff8008a53754 and 0xffffff8008a537cc shows: it
 * reads "ilitek_tddi_switch_tp_mode"@0xffffff800923e096 while the address is
 * inside `ilitek_tddi_mp_test_handler`. It is clang inlining and specialising
 * on a constant, not the source duplicating: here there is a call.
 */
int ilitek_tddi_mp_test_handler(char *apk, bool lcm_on)
{
	int ret = 0;

	/* "b942cd09 ldr"@0xffffff8008a53710, "34000129 cbz"@0xffffff8008a53714 */
	if (READ_ONCE(idev->c716)) {
		/* "\x013ILITEK: (%s, %d): fw upgrade processing, ignore\n"
		 * @0xffffff800923df8b */
#line 93
		ILI_ERR("fw upgrade processing, ignore\n");
		/*
		 * "12800df4 mov"@0xffffff8008a53730: #0xffffff90 = -112. It is not
		 * a kernel errno that makes sense here: see D7 in the file header.
		 */
		return -112;
	}

	/* "b902d109 str"@0xffffff8008a5373c */
	WRITE_ONCE(idev->c720, 1);

	/* "7100055f cmp"@0xffffff8008a5374c, "54000460 b.eq"@0xffffff8008a53750 */
	if (idev->c320 != 1) {
		/*
		 * "320003e9 orr"@0xffffff8008a53738 sets the value 1, which is
		 * the mode passed; the call is inlined, see above.
		 */
		if (ilitek_tddi_switch_tp_mode(1) < 0) {
			/* "\x013ILITEK: (%s, %d): Switch MP mode failed\n"
			 * @0xffffff800923dfda,
			 * "52800cc2 mov"@0xffffff8008a53874 -> line 102 */
#line 102
			ILI_ERR("Switch MP mode failed\n");
			/* "12800dd4 mov"@0xffffff8008a5387c: #0xffffff91 = -111 */
			ret = -111;
			goto out;
		}
	}

	/* "94002b8d bl"@0xffffff8008a537e4 -> <ilitek_tddi_mp_test_main> */
	ret = ilitek_tddi_mp_test_main(apk, lcm_on);

out:
	/*
	 * "360002f3 tbz"@0xffffff8008a537f0 and "3707fb93 tbnz"@0xffffff8008a53884:
	 * the restore runs only if `lcm_on`, on BOTH paths -- including the one
	 * that has just failed the switch.
	 */
	if (lcm_on) {
		/* "b901411f str"@0xffffff8008a537f8 */
		idev->c320 = 0;
		/* "b9426d09 ldr"@0xffffff8008a537f4,
		 * "7100053f cmp"@0xffffff8008a537fc */
		if (idev->c620 == 1) {
			/* "9400009d bl"@0xffffff8008a53804 */
			if (ilitek_tddi_fw_upgrade_handler() < 0)
				/* "\x013ILITEK: (%s, %d): FW upgrade failed
				 * during mp test\n"@0xffffff800923e005,
				 * "321e13e2 orr"@0xffffff8008a5381c -> line 124 */
#line 124
				ILI_ERR("FW upgrade failed during mp test\n");
		} else {
			/* "b9426500 ldr"@0xffffff8008a53824,
			 * "940000c8 bl"@0xffffff8008a53828 */
			if (ilitek_tddi_reset_ctrl(idev->c612) < 0)
				/* "\x013ILITEK: (%s, %d): TP Reset failed
				 * during mp test\n"@0xffffff800923e03b,
				 * "32001be2 orr"@0xffffff8008a53840 -> line 127 */
#line 127
				ILI_ERR("TP Reset failed during mp test\n");
		}
	}

	/* "b902d11f str"@0xffffff8008a5384c */
	WRITE_ONCE(idev->c720, 0);
	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_switch_tp_mode -- 0xffffff8008a5388c, 492 bytes
 * factory: lines 150..183
 * ---------------------------------------------------------------------------
 * The parameter is one byte wide: "12001c13 and"@0xffffff8008a538a8 masks it
 * with 0xff before every use.
 */
int ilitek_tddi_switch_tp_mode(u8 mode)
{
	int ret = 0;
	bool ges_dbg;

	/* "b902d909 str"@0xffffff8008a538ac */
	idev->c728 = 1;

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a538b0).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	ges_dbg = (mode == 15) & (idev->c324 == 1);

	/* "b9014113 str"@0xffffff8008a538b4 */
	idev->c320 = mode;

	switch (mode) {
	case 0:
		/*
		 * "\x016ILITEK: (%s, %d): Switch to AP mode\n"
		 * @0xffffff800923e06f, line 150. NOT guarded by the diagnostic
		 * byte: the `bl` at 0xffffff8008a538ec has no
		 * `ldrb [.,#2392]` before it on its own path.
		 */
#line 150
		ILI_INFO("Switch to AP mode\n");
		/* "b9426d09 ldr"@0xffffff8008a538f4 */
		if (idev->c620 == 1) {
			/* "9400005e bl"@0xffffff8008a53900 */
			if (ilitek_tddi_fw_upgrade_handler() < 0)
				/*
				 * "\x013ILITEK: (%s, %d): FW upgrade failed\n"
				 * @0xffffff800923e0b1, line 153.
				 * `ret` STAYS ZERO: "2a1f03f4 mov"@0xffffff8008a53a70
				 * and "2a1f03f4 mov"@0xffffff8008a53908 clear w20
				 * on both paths.
				 */
#line 153
				ILI_ERR("FW upgrade failed\n");
		} else {
			/* "b9426500 ldr"@0xffffff8008a539e8,
			 * "94000057 bl"@0xffffff8008a539ec */
			ret = ilitek_tddi_reset_ctrl(idev->c612);
			if (ret < 0)
				/* "\x013ILITEK: (%s, %d): TP Reset failed\n"
				 * @0xffffff800923e0d8, line 158 */
#line 158
				ILI_ERR("TP Reset failed\n");
		}
		break;

	case 1:
		/* "\x016ILITEK: (%s, %d): Switch to Test mode\n"
		 * @0xffffff800923e15a, line 171 */
#line 171
		ILI_INFO("Switch to Test mode\n");
		/* "f9419508 ldr"@0xffffff8008a53950 (+808 =
		 * `ilitek_tddi_move_mp_code_flash`),
		 * "d63f0100 blr"@0xffffff8008a53954 */
		ret = idev->c808();
		break;

	case 15:
		/* "f941990b ldr"@0xffffff8008a53910 (+816 =
		 * `ilitek_tddi_move_gesture_code_flash`),
		 * "b9421900 ldr"@0xffffff8008a53914 (l'argument is c536),
		 * "d63f0160 blr"@0xffffff8008a5391c */
		ret = idev->c816(idev->c536);
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): Move gesture code failed\n"
			 * @0xffffff800923e0fd, line 164 */
#line 164
			ILI_ERR("Move gesture code failed\n");
		if (ges_dbg) {
			/* "\x016ILITEK: (%s, %d): Enable gesture debug func\n"
			 * @0xffffff800923e12b, line 166 */
#line 166
			ILI_INFO("Enable gesture debug func\n");
			/* "32000be0 orr"@0xffffff8008a53a48: #0x7,
			 * "9400008d bl"@0xffffff8008a53a4c */
			ilitek_set_tp_data_len(7);
		}
		break;

	default:
		/* "\x013ILITEK: (%s, %d): Unknown TP mode: %x\n"
		 * @0xffffff800923e183, line 175 */
#line 175
		ILI_ERR("Unknown TP mode: %x\n", mode);
		/* "12800014 mov"@0xffffff8008a53980: #0xffffffff = -1 */
		ret = -1;
		break;
	}

	if (ret < 0)
		/*
		 * "\x013ILITEK: (%s, %d): Switch TP mode (%d) failed \n"
		 * @0xffffff800923e1ac, line 181. The space before the \n is there
		 * in the factory build too.
		 */
#line 181
		ILI_ERR("Switch TP mode (%d) failed \n", mode);

	/*
	 * Guarded: "39656108 ldrb"@0xffffff8008a539a4 on the diagnostic byte,
	 * "34000128 cbz"@0xffffff8008a539a8 skips the loading of the argument
	 * as well. "\x016ILITEK: (%s, %d): Actual TP mode = %d\n"
	 * @0xffffff800923e1dd, line 183.
	 */
#line 183
	ILI_DBG("Actual TP mode = %d\n", idev->c320);

	/* "b902d91f str"@0xffffff8008a539d4 */
	idev->c728 = 0;
	return ret;
}

/*
 * ilitek_tddi_gesture_recovery() was reconstructed from the factory kernel disassembly (0xffffff8008a53ed8, 144 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_gesture_recovery(void)
{
	int ret;
	bool locked;

	/* "f8470c15 ldr"@0xffffff8008a53ef4 */
	locked = mutex_is_locked(&idev->touch_mutex);

	/*
	 * "b9027013 str"@0xffffff8008a53efc: base already advanced to idev+112,
	 * hence 112 + 624 = 736.
	 * THE ORDER IS MEASURED: the store falls BEFORE
	 * "94107d1e bl"@0xffffff8008a53f04 towards <mutex_lock>, on both paths.
	 * Writing the field after the lock forces clang to reload the global, and
	 * the function grows by eight bytes (measured: 152 against 144).
	 */
	WRITE_ONCE(idev->c736, 1);

	if (!locked)
		mutex_lock(&idev->touch_mutex);

	/* "\x016ILITEK: (%s, %d): Doing gesture recovery\n"
	 * @0xffffff800923e206, line 198 */
#line 198
	ILI_INFO("Doing gesture recovery\n");

	/* "3909dd13 strb"@0xffffff8008a53f28 */
	idev->c631 = 1;
	/* "f941a509 ldr"@0xffffff8008a53f24 (+840 =
	 * `ilitek_tddi_touch_esd_gesture_flash`),
	 * "d63f0120 blr"@0xffffff8008a53f2c */
	ret = idev->c840();
	/* "3909dd1f strb"@0xffffff8008a53f3c */
	idev->c631 = 0;

	if (!locked)
		/* "9101c100 add"@0xffffff8008a53f44 (idev + 0x70 = 112),
		 * "94107d21 bl"@0xffffff8008a53f48 */
		mutex_unlock(&idev->touch_mutex);

	/* "b902e11f str"@0xffffff8008a53f50 */
	WRITE_ONCE(idev->c736, 0);
	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_spi_recovery -- 0xffffff8008a53f68, 160 bytes
 * factory: lines 219..222
 * ---------------------------------------------------------------------------
 * It returns `void`: the epilogue at 0xffffff8008a53ff8 does not write w0,
 * unlike that of `ilitek_tddi_gesture_recovery`
 * ("2a1303e0 mov"@0xffffff8008a53f54), which is its structural twin.
 */
void ilitek_tddi_spi_recovery(void)
{
	int ret;
	bool locked;

	/* "f8470c14 ldr"@0xffffff8008a53f84, "f1001e9f cmp"@0xffffff8008a53f88 */
	locked = mutex_is_locked(&idev->touch_mutex);

	/*
	 * "b9027015 str"@0xffffff8008a53f8c: base already advanced to idev+112, and
	 * as in its twin the store falls BEFORE
	 * "94107cfa bl"@0xffffff8008a53f94 towards <mutex_lock>.
	 */
	WRITE_ONCE(idev->c736, 1);

	if (!locked)
		mutex_lock(&idev->touch_mutex);

	/* "\x016ILITEK: (%s, %d): Doing spi recovery\n"@0xffffff800923e24f,
	 * line 219 */
#line 219
	ILI_INFO("Doing spi recovery\n");

	/* "3909dd15 strb"@0xffffff8008a53fb4 */
	idev->c631 = 1;
	/* "97fffeb0 bl"@0xffffff8008a53fb8 */
	ret = ilitek_tddi_fw_upgrade_handler();
	if (ret < 0)
		/*
		 * "\x013ILITEK: (%s, %d): FW upgrade failed\n"
		 * @0xffffff800923e0b1, line 222 -- the same literal
		 * `ilitek_tddi_switch_tp_mode` uses at line 153.
		 */
#line 222
		ILI_ERR("FW upgrade failed\n");
	/* "3909dd1f strb"@0xffffff8008a53fe0 */
	idev->c631 = 0;

	if (!locked)
		/* "9101c100 add"@0xffffff8008a53fe8,
		 * "94107cf8 bl"@0xffffff8008a53fec */
		mutex_unlock(&idev->touch_mutex);

	/* "b902e11f str"@0xffffff8008a53ff4 */
	WRITE_ONCE(idev->c736, 0);
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_wq_esd_spi_check -- 0xffffff8008a54008, 40 bytes
 * factory: line unknown (no printk; it lies between 222 and 238)
 * ---------------------------------------------------------------------------
 */
int ilitek_tddi_wq_esd_spi_check(void)
{
	/* "f9419108 ldr"@0xffffff8008a54018 (+800),
	 * "d63f0100 blr"@0xffffff8008a5401c */
	int ret = idev->c800();

	/*
	 * "71028c1f cmp"@0xffffff8008a54020 with #0xa3,
	 * "5a9f03e0 csetm"@0xffffff8008a54024: `csetm` sets -1, not 1.
	 */
	return (ret != 0xa3) ? -1 : 0;
}

/*
 * ilitek_tddi_wq_esd_i2c_check() was reconstructed from the factory kernel disassembly (0xffffff8008a54030, 56 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_wq_esd_i2c_check(void)
{
	/*
	 * Guarded: "39656108 ldrb"@0xffffff8008a54034,
	 * "34000148 cbz"@0xffffff8008a54038 -- and the jump skips even the
	 * function prologue ("a9bf7bfd stp"@0xffffff8008a5403c), which clang
	 * places inside the branch.
	 */
#line 238
	ILI_DBG("");
	return 0;
}

/*
 * ilitek_tddi_wq_esd_check() was reconstructed from the factory kernel disassembly (0xffffff8008a5535c, 96 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ilitek_tddi_wq_esd_check(struct work_struct *work)
{
	/* "f9419d08 ldr"@0xffffff8008a55370 (+824),
	 * "d63f0100 blr"@0xffffff8008a55374,
	 * "36f80100 tbz"@0xffffff8008a55378 */
	if (idev->c824() < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): SPI ACK failed, doing spi
		 * recovery\n"@0xffffff800923eabd, line 245 -- the same
		 * literal `ilitek_tddi_report_handler` uses at line
		 * 638.
		 */
#line 245
		ILI_ERR("SPI ACK failed, doing spi recovery\n");
		/* "97fffaf5 bl"@0xffffff8008a55394 */
		ilitek_tddi_spi_recovery();
	}

	/* "910ba100 add"@0xffffff8008a5539c, "97db23e1 bl"@0xffffff8008a553a0 */
	complete_all(&idev->c744);
	/* "2a1f03e0 mov"@0xffffff8008a553a8 (type 0),
	 * "320003e1 orr"@0xffffff8008a553a4 (ctrl 1),
	 * "97fffb2f bl"@0xffffff8008a553ac */
	ilitek_tddi_wq_ctrl(0, 1);
}

/*
 * The path `read_power_status` opens. It is a SINGLE literal used TWICE --
 * as the argument of `filp_open` ("9102bc00 add"@0xffffff8008a5540c)
 * and as the `%s` argument of the error message
 * ("9102bc63 add"@0xffffff8008a554c8) -- and it is the same address,
 * "/sys/class/power_supply/battery/status"@0xffffff800923f0af.
 */
#define ILITEK_POWER_STATUS_PATH	"/sys/class/power_supply/battery/status"

/*
 * read_power_status() was reconstructed from the factory kernel disassembly (0xffffff8008a554d0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int read_power_status(u8 *buf)
{
	struct file *f;
	mm_segment_t old_fs;
	ssize_t byte;

	/*
	 * "f9400675 ldr"@0xffffff8008a553f0 (current + 8),
	 * "f9000669 str"@0xffffff8008a553f4 with x9 = -1 = KERNEL_DS,
	 * "d503379f dsb"@0xffffff8008a553f8 + "d5033fdf isb"@0xffffff8008a553fc,
	 * "941006ba bl"@0xffffff8008a55404 towards <set_bit> with w0 = 5 =
	 * TIF_FSCHECK: that is this tree's expansion of `set_fs`.
	 */
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* "2a1f03e1 mov"@0xffffff8008a55410 (O_RDONLY = 0),
	 * "2a1f03e2 mov"@0xffffff8008a55414 (mode 0),
	 * "97e04190 bl"@0xffffff8008a5541c */
	f = filp_open(ILITEK_POWER_STATUS_PATH, O_RDONLY, 0);
	/*
	 * THE ORDER OF THE TWO TERMS IS MEASURED: IS_ERR first
	 * ("b140041f cmn"@0xffffff8008a55420 with #0x1,lsl#12 plus
	 * "54000488 b.hi"@0xffffff8008a55424), then the NULL
	 * ("b4000440 cbz"@0xffffff8008a5542c). The kernel's `IS_ERR_OR_NULL`
	 * tests the NULL FIRST, so this is NOT that macro.
	 */
	if (IS_ERR(f) || f == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to open %s\n"
		 * @0xffffff800923f0d6, line 263 */
#line 263
		ILI_ERR("Failed to open %s\n", ILITEK_POWER_STATUS_PATH);
		return -1;
	}

	/* "f9401688 ldr"@0xffffff8008a55430 (f->f_op),
	 * "f9400508 ldr"@0xffffff8008a55440 (+8 = llseek),
	 * "aa1f03e1 mov"@0xffffff8008a55438 (offset 0),
	 * "2a1f03e2 mov"@0xffffff8008a5543c (whence 0 = SEEK_SET),
	 * "d63f0100 blr"@0xffffff8008a55444 */
	f->f_op->llseek(f, 0, SEEK_SET);
	/*
	 * "f9400908 ldr"@0xffffff8008a55458 (+16 = read),
	 * "9101a283 add"@0xffffff8008a5544c = f + 104, that is &f->f_pos,
	 * "52800282 mov"@0xffffff8008a55454 (20 bytes),
	 * "d63f0100 blr"@0xffffff8008a55460
	 */
	byte = f->f_op->read(f, buf, 20, &f->f_pos);

	/*
	 * guarded: "39656108 ldrb"@0xffffff8008a55468.
	 * "\x016ILITEK: (%s, %d): Read %d bytes\n"@0xffffff800923f10f,
	 * line 270. A FACTORY DEFECT: the argument is passed as 64-bit
	 * ("aa0003e3 mov"@0xffffff8008a55470 is `mov x3, x0`, not `mov w3,w0`)
	 * while the format says `%d`. See divergence D10 in the file header.
	 */
#line 270
	ILI_DBG("Read %d bytes\n", byte);

	/*
	 * "f9000675 str"@0xffffff8008a5548c puts the old limit back, then
	 * "b10006bf cmn"@0xffffff8008a554a4: the comparison with -1 is the
	 * `fs == KERNEL_DS` path that the UAO ALTERNATIVE brings with it.
	 */
	set_fs(old_fs);
	/* "aa1f03e1 mov"@0xffffff8008a554f8 (NULL),
	 * "97e04299 bl"@0xffffff8008a554fc */
	filp_close(f, NULL);
	return 0;
}

/*
 * ilitek_tddi_wq_bat_check() was reconstructed from the factory kernel disassembly (0xffffff8008a553bc, 680 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_bat_status;

static void ilitek_tddi_wq_bat_check(struct work_struct *work)
{
	/*
	 * "a9007fff stp"@0xffffff8008a553ec (16 bytes) plus
	 * "b90013ff str"@0xffffff8008a553e8 (4): twenty bytes cleared on the
	 * stack. The canary at [sp,#24] says it is `-fstack-protector-strong`
	 * seeing an array there.
	 */
	u8 str[20] = {0};

	/*
	 * There is no "36f800e0 tbz": the return value is NOT tested with a jump
	 * of its own -- the error branch is the continuation of the failed
	 * `filp_open`, "54000488 b.hi"@0xffffff8008a55424, which after line 263
	 * prints this one too and carries on.
	 */
	if (read_power_status(str) < 0)
		/* "\x013ILITEK: (%s, %d): Read power status failed\n"
		 * @0xffffff800923ef94, line 283 */
#line 283
		ILI_ERR("Read power status failed\n");

	/*
	 * guarded: "39656268 ldrb"@0xffffff8008a55504.
	 * "\x016ILITEK: (%s, %d): Batter Status: %s\n"@0xffffff800923efdb,
	 * line 285. The "Batter" typo is the factory's.
	 */
#line 285
	ILI_DBG("Batter Status: %s\n", str);

	/*
	 * "Charging"@0xffffff800925eb49 ("912d2421 add"@0xffffff8008a5552c),
	 * "Full"@0xffffff800927a583 ("91160c21 add"@0xffffff8008a55540),
	 * "Fully charged"@0xffffff800923f002 ("91000821 add"@0xffffff8008a55554).
	 * The first two end with a `cbnz` ("b5000160 cbnz"@0xffffff8008a55538,
	 * "b50000c0 cbnz"@0xffffff8008a5554c), the third with a `cbz`
	 * ("b40004e0 cbz"@0xffffff8008a55560): it is a three-term `||`.
	 * That "Full" makes "Fully charged" unreachable is a factory defect
	 * and it is reproduced.
	 */
	if (strstr(str, "Charging") || strstr(str, "Full") ||
	    strstr(str, "Fully charged")) {
		/* "b94a3a88 ldr"@0xffffff8008a55568,
		 * "7100051f cmp"@0xffffff8008a5556c con #1,
		 * "540002c0 b.eq"@0xffffff8008a55570 */
		if (ilitek_bat_status != 1) {
			/* guarded: "39656268 ldrb"@0xffffff8008a55574.
			 * "\x016ILITEK: (%s, %d): Charging mode\n"
			 * @0xffffff800923f010, line 290 */
#line 290
			ILI_DBG("Charging mode\n");
			/* "plug"@0xffffff80090fa501,
			 * "2a1f03e1 mov"@0xffffff8008a5559c (ctrl 0),
			 * "94000a3a bl"@0xffffff8008a555a0 */
			if (ilitek_tddi_ic_func_ctrl("plug", 0) < 0)
				/* "\x013ILITEK: (%s, %d): Write plug in
				 * failed\n"@0xffffff800923f033, line 292 */
#line 292
				ILI_ERR("Write plug in failed\n");
			/* "320003e8 orr"@0xffffff8008a555c0 +
			 * "b90a3a88 str"@0xffffff8008a555c4 */
			ilitek_bat_status = 1;
		}
	} else {
		/* "7100091f cmp"@0xffffff8008a55604 con #2,
		 * "54fffe00 b.eq"@0xffffff8008a55608 */
		if (ilitek_bat_status != 2) {
			/* guarded: "39656268 ldrb"@0xffffff8008a5560c.
			 * "\x016ILITEK: (%s, %d): Not charging mode\n"
			 * @0xffffff800923f05d, line 297 */
#line 297
			ILI_DBG("Not charging mode\n");
			/* "320003e1 orr"@0xffffff8008a55634 (ctrl 1),
			 * "94000a14 bl"@0xffffff8008a55638 */
			if (ilitek_tddi_ic_func_ctrl("plug", 1) < 0)
				/* "\x013ILITEK: (%s, %d): Write plug out
				 * failed\n"@0xffffff800923f084, line 299 */
#line 299
				ILI_ERR("Write plug out failed\n");
			/*
			 * "321f03e8 orr"@0xffffff8008a55658 (#2) and the SAME
			 * "b90a3a88 str"@0xffffff8008a555c4 as the other branch,
			 * reached through "17ffffda b"@0xffffff8008a5565c.
			 */
			ilitek_bat_status = 2;
		}
	}

	/* "320003e0 orr"@0xffffff8008a555c8, "320003e1 orr"@0xffffff8008a555cc,
	 * "97fffaa6 bl"@0xffffff8008a555d0 */
	ilitek_tddi_wq_ctrl(1, 1);
}

/*
 * ilitek_tddi_wq_ctrl() was reconstructed from the factory kernel disassembly (0xffffff8008a54068, 544 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_wq_ctrl(int type, int ctrl)
{
	switch (type) {
	case 0:
		/* "340009e8 cbz"@0xffffff8008a54090 */
		if (!idev->c527)
			break;
		/* "f944b288 ldr"@0xffffff8008a54098 (0xffffff800a0fc960),
		 * "b40009e8 cbz"@0xffffff8008a5409c */
		if (!ilitek_wq_esd) {
			/* "\x013ILITEK: (%s, %d): wq esd is null\n"
			 * @0xffffff800923e2c2, line 312 */
#line 312
			ILI_ERR("wq esd is null\n");
			break;
		}
		/* "1a9f07ea cset"@0xffffff8008a540a4 (ne),
		 * "3908412a strb"@0xffffff8008a540ac */
		idev->c528 = (ctrl != 0);
		/* "7100043f cmp"@0xffffff8008a540a8 con #0x1,
		 * "54000b01 b.ne"@0xffffff8008a540b0 */
		if (ctrl == 1) {
			/* guarded: "39656269 ldrb"@0xffffff8008a540b8.
			 * "\x016ILITEK: (%s, %d): execute esd check\n"
			 * @0xffffff800923e2fa, line 317 */
#line 317
			ILI_DBG("execute esd check\n");
			/*
			 * "97d9ed7b bl"@0xffffff8008a540f0,
			 * "370006c0 tbnz"@0xffffff8008a540f4 on bit 0 of the
			 * return value
			 */
			if (!queue_delayed_work(ilitek_wq_esd,
						&ilitek_esd_work, 1000))
				/* guarded: "39656268 ldrb"@0xffffff8008a540f8.
				 * "\x016ILITEK: (%s, %d): esd check was already
				 * on queue\n"@0xffffff800923e321, line 319 */
#line 319
				ILI_DBG("esd check was already on queue\n");
		} else {
			/* "97d9f281 bl"@0xffffff8008a54218 */
			cancel_delayed_work_sync(&ilitek_esd_work);
			/* "97d9ee2d bl"@0xffffff8008a54220 */
			flush_workqueue(ilitek_wq_esd);
			/* guarded: "39656108 ldrb"@0xffffff8008a54228.
			 * "\x016ILITEK: (%s, %d): cancel esd wq\n"
			 * @0xffffff800923e355, line 323 */
#line 323
			ILI_DBG("cancel esd wq\n");
		}
		break;

	case 1:
		/* "34000528 cbz"@0xffffff8008a54128 */
		if (!idev->c527)
			break;
		/* "f944e688 ldr"@0xffffff8008a54130 (0xffffff800a0fc9c8),
		 * "b4000608 cbz"@0xffffff8008a54134 */
		if (!ilitek_wq_bat) {
			/* "\x013ILITEK: (%s, %d): WQ BAT is null\n"
			 * @0xffffff800923e378, line 330 */
#line 330
			ILI_ERR("WQ BAT is null\n");
			break;
		}
		/* "1a9f07ea cset"@0xffffff8008a5413c,
		 * "3908452a strb"@0xffffff8008a54144 */
		idev->c529 = (ctrl != 0);
		/* "7100043f cmp"@0xffffff8008a54140,
		 * "54000821 b.ne"@0xffffff8008a54148 */
		if (ctrl == 1) {
			/* guarded: "39656269 ldrb"@0xffffff8008a54150.
			 * "\x016ILITEK: (%s, %d): execute bat check\n"
			 * @0xffffff800923e39c, line 335 */
#line 335
			ILI_DBG("execute bat check\n");
			/* "97d9ed55 bl"@0xffffff8008a54188,
			 * "37000200 tbnz"@0xffffff8008a5418c */
			if (!queue_delayed_work(ilitek_wq_bat,
						&ilitek_bat_work, 500))
				/* guarded: "39656268 ldrb"@0xffffff8008a54190.
				 * "\x016ILITEK: (%s, %d): bat check was already
				 * on queue\n"@0xffffff800923e3c3, line 337 */
#line 337
				ILI_DBG("bat check was already on queue\n");
		} else {
			/* "97d9f272 bl"@0xffffff8008a54254 */
			cancel_delayed_work_sync(&ilitek_bat_work);
			/* "97d9ee1e bl"@0xffffff8008a5425c */
			flush_workqueue(ilitek_wq_bat);
			/* guarded: "39656108 ldrb"@0xffffff8008a54264.
			 * "\x016ILITEK: (%s, %d): cancel bat wq\n"
			 * @0xffffff800923e3f7, line 341 */
#line 341
			ILI_DBG("cancel bat wq\n");
		}
		break;

	default:
		/*
		 * "\x013ILITEK: (%s, %d): Unknown WQ type, %d\n"
		 * @0xffffff800923e41a, line 346. NOT guarded: the nearest
		 * "39656108 ldrb" is on another path.
		 */
#line 346
		ILI_ERR("Unknown WQ type, %d\n", type);
		break;
	}
}

/*
 * ilitek_tddi_wq_init() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ilitek_tddi_wq_init(void)
{
	/*
	 * "esd_check"@0xffffff80091ff832, "321d03e1 orr"@0xffffff8008a54f44
	 * (flag = 8 = WQ_MEM_RECLAIM), "2a1f03e2 mov"@0xffffff8008a54f48
	 * (max_active = 0), "aa1f03e3 mov"@0xffffff8008a54f4c and
	 * "aa1f03e4 mov"@0xffffff8008a54f50 (key and lock_name NULL, that is the
	 * macro's non-LOCKDEP path), "97d9f063 bl"@0xffffff8008a54f54.
	 * The first argument is the literal DIRECTLY, not "%s": no
	 * `create_workqueue`/`create_singlethread_workqueue`, which would pass
	 * "%s" and different flags.
	 */
	ilitek_wq_esd = alloc_workqueue("esd_check", WQ_MEM_RECLAIM, 0);
	/* "bat_check"@0xffffff800923efd1, "97d9f05a bl"@0xffffff8008a54f78 */
	ilitek_wq_bat = alloc_workqueue("bat_check", WQ_MEM_RECLAIM, 0);

	/*
	 * "b4001408 cbz"@0xffffff8008a54f88 towards the first
	 * `warn_slowpath_null` (line 356) and
	 * "b40014a0 cbz"@0xffffff8008a54f8c towards the second (line 357).
	 * After the WARN execution CARRIES ON: "b5ffeba0 cbnz"@0xffffff8008a5521c
	 * and "17ffff58 b"@0xffffff8008a55230 both re-enter at
	 * 0xffffff8008a54f90. No error return.
	 */
	WARN_ON(!ilitek_wq_esd);
	WARN_ON(!ilitek_wq_bat);

	/*
	 * "9125a273 add"@0xffffff8008a54f94 = 0xffffff800a0fc968,
	 * "910d7108 add"@0xffffff8008a54fa0 = ilitek_tddi_wq_esd_check,
	 * "f9000275 str"@0xffffff8008a54fb8 (work.data = 0xfffffffe0),
	 * "a9012269 stp"@0xffffff8008a54fc0 (entry.prev and work.func),
	 * "320b03e1 orr"@0xffffff8008a54fac (#0x200000 = TIMER_IRQSAFE),
	 * "97dbef6a bl"@0xffffff8008a54fc4 towards <init_timer_key>,
	 * "a903ce76 stp"@0xffffff8008a54ff4 (timer.function =
	 * 0xffffff80080cf6bc = delayed_work_timer_fn, timer.data = the block
	 * itself).
	 */
	INIT_DELAYED_WORK(&ilitek_esd_work, ilitek_tddi_wq_esd_check);
	/* "912742f7 add"@0xffffff8008a54fd0 = 0xffffff800a0fc9d0,
	 * "910ef108 add"@0xffffff8008a54fdc = ilitek_tddi_wq_bat_check */
	INIT_DELAYED_WORK(&ilitek_bat_work, ilitek_tddi_wq_bat_check);
}

/*
 * ilitek_tddi_sleep_handler() was reconstructed from the factory kernel disassembly (0xffffff8008a54288, 1188 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_sleep_handler(int mode)
{
	int ret = 0;
	bool stop_sense;

	/* "9101c100 add"@0xffffff8008a542a4, "94107c35 bl"@0xffffff8008a542a8 */
	mutex_lock(&idev->touch_mutex);
	/* "b902d509 str"@0xffffff8008a542b4 */
	WRITE_ONCE(idev->c724, 1);

	/* "35000069 cbnz"@0xffffff8008a542bc (c716),
	 * "34000229 cbz"@0xffffff8008a542c4 (c720) */
	if (READ_ONCE(idev->c716) || READ_ONCE(idev->c720)) {
		/*
		 * "\x016ILITEK: (%s, %d): fw upgrade or mp still running,
		 * ignore sleep requst\n"@0xffffff800923e443, line 379.
		 * The "requst" typo is the factory's and is reproduced.
		 */
#line 379
		ILI_INFO("fw upgrade or mp still running, ignore sleep requst\n");
		/* "b902d51f str"@0xffffff8008a542e8 BEFORE the unlock */
		WRITE_ONCE(idev->c724, 0);
		/* "94107c38 bl"@0xffffff8008a542ec */
		mutex_unlock(&idev->touch_mutex);
		/* "2a1f03f3 mov"@0xffffff8008a542f0 */
		return 0;
	}

	/* inlined, see the file header */
	ilitek_tddi_wq_ctrl(0, 0);
	ilitek_tddi_wq_ctrl(1, 0);

	/* "94000745 bl"@0xffffff8008a543dc */
	ilitek_plat_irq_disable();

	/* "\x016ILITEK: (%s, %d): Sleep Mode = %d\n"@0xffffff800923e4a6,
	 * line 389; the argument is `mode` ("2a1303e3 mov"@0xffffff8008a543f4). */
#line 389
	ILI_INFO("Sleep Mode = %d\n", mode);

	stop_sense = idev->c642 ||
		     *(u32 *)((u8 *)idev->c48 + 52) < 0x10403;

	switch (mode) {
	case 0:
		/* "\x016ILITEK: (%s, %d): TP suspend start\n"
		 * @0xffffff800923e4cb, line 400 */
#line 400
		ILI_INFO("TP suspend start\n");
		/* "340002d5 cbz"@0xffffff8008a54438 */
		if (stop_sense) {
			/* "sense"@0xffffff80092419c4,
			 * "91271000 add"@0xffffff8008a54440,
			 * "2a1f03e1 mov"@0xffffff8008a54444 (ctrl = 0) */
			if (ilitek_tddi_ic_func_ctrl("sense", 0) < 0)
				/* "\x013ILITEK: (%s, %d): Write sense stop cmd
				 * failed\n"@0xffffff800923e4f1, line 403 */
#line 403
				ILI_ERR("Write sense stop cmd failed\n");
			/* "52800640 mov"@0xffffff8008a54468 (#50),
			 * "52800281 mov"@0xffffff8008a5446c (#20) */
			if (ilitek_tddi_ic_check_busy(50, 20) < 0)
				/* "\x013ILITEK: (%s, %d): Check busy timeout
				 * during suspend\n"@0xffffff800923e522, line 406 */
#line 406
				ILI_ERR("Check busy timeout during suspend\n");
		}
		/* "39485108 ldrb"@0xffffff8008a54494,
		 * "34000b88 cbz"@0xffffff8008a54498 */
		if (idev->c532) {
			/* "32000fe0 orr"@0xffffff8008a5449c (#0xf) */
			ilitek_tddi_switch_tp_mode(0x0f);
			/* "b9414d00 ldr"@0xffffff8008a544ac (c332),
			 * "320003e1 orr"@0xffffff8008a544a8 (#1),
			 * "97db9a7d bl"@0xffffff8008a544b0 */
			irq_set_irq_wake(idev->c332, 1);
			/* "94000737 bl"@0xffffff8008a544b4 */
			ilitek_plat_irq_enable();
		} else {
			/* "sleep"@0xffffff800911c431,
			 * "320007e1 orr"@0xffffff8008a54610 (ctrl = 3) */
			if (ilitek_tddi_ic_func_ctrl("sleep", 3) < 0)
				/* "\x013ILITEK: (%s, %d): Write sleep in cmd
				 * failed\n"@0xffffff800923e559, line 415 */
#line 415
				ILI_ERR("Write sleep in cmd failed\n");
		}
		/* "\x016ILITEK: (%s, %d): TP suspend end\n"
		 * @0xffffff800923e588, line 417 */
#line 417
		ILI_INFO("TP suspend end\n");
		/* "3909ed09 strb"@0xffffff8008a5469c,
		 * "390a011f strb"@0xffffff8008a546a0 */
		idev->c635 = 1;
		idev->c640 = 0;
		break;

	case 1:
		/* "\x016ILITEK: (%s, %d): TP deep suspend start\n"
		 * @0xffffff800923e5ac, line 422 */
#line 422
		ILI_INFO("TP deep suspend start\n");
		/* "340002d5 cbz"@0xffffff8008a54560 */
		if (stop_sense) {
			if (ilitek_tddi_ic_func_ctrl("sense", 0) < 0)
				/*
				 * same literal as line 403,
				 * @0xffffff800923e4f1, here line 425
				 */
#line 425
				ILI_ERR("Write sense stop cmd failed\n");
			if (ilitek_tddi_ic_check_busy(50, 20) < 0)
				/* "\x013ILITEK: (%s, %d): Check busy timeout
				 * during deep suspend\n"@0xffffff800923e5d7,
				 * line 428 */
#line 428
				ILI_ERR("Check busy timeout during deep suspend\n");
		}
		/* "39485108 ldrb"@0xffffff8008a545bc,
		 * "34000468 cbz"@0xffffff8008a545c0 */
		if (idev->c532) {
			ilitek_tddi_switch_tp_mode(0x0f);
			irq_set_irq_wake(idev->c332, 1);
			ilitek_plat_irq_enable();
		} else {
			if (ilitek_tddi_ic_func_ctrl("sleep", 3) < 0)
				/* "\x013ILITEK: (%s, %d): Write deep sleep in
				 * cmd failed\n"@0xffffff800923e613, line 437 */
#line 437
				ILI_ERR("Write deep sleep in cmd failed\n");
		}
		/* "\x016ILITEK: (%s, %d): TP deep suspend end\n"
		 * @0xffffff800923e647, line 439 */
#line 439
		ILI_INFO("TP deep suspend end\n");
		idev->c635 = 1;
		idev->c640 = 0;
		break;

	case 2:
		/*
		 * "3949e908 ldrb"@0xffffff8008a544dc,
		 * "35001168 cbnz"@0xffffff8008a544e0: if the byte is set the whole
		 * body of the wake-up is skipped, but NOT the re-enabling of the
		 * interrupt.
		 */
		if (!idev->c634) {
			/* "\x016ILITEK: (%s, %d): TP resume start\n"
			 * @0xffffff800923e670, line 445 */
#line 445
			ILI_INFO("TP resume start\n");
			/* "39485109 ldrb"@0xffffff8008a54500,
			 * "340000a9 cbz"@0xffffff8008a54504 */
			if (idev->c532)
				/* "2a1f03e1 mov"@0xffffff8008a5450c (0) */
				irq_set_irq_wake(idev->c332, 0);
			/* "b901411f str"@0xffffff8008a5451c */
			idev->c320 = 0;
			/* "7100053f cmp"@0xffffff8008a54520 con #1 */
			if (idev->c620 == 1) {
				/* "97fffd54 bl"@0xffffff8008a54528 */
				if (ilitek_tddi_fw_upgrade_handler() < 0)
					/* "\x013ILITEK: (%s, %d): FW upgrade
					 * failed during resume\n"
					 * @0xffffff800923e695, line 454 */
#line 454
					ILI_ERR("FW upgrade failed during resume\n");
			} else {
				/* "b9426500 ldr"@0xffffff8008a546a8 (c612),
				 * "97fffd27 bl"@0xffffff8008a546ac */
				if (ilitek_tddi_reset_ctrl(idev->c612) < 0)
					/* "\x013ILITEK: (%s, %d): TP Reset
					 * failed during resume\n"
					 * @0xffffff800923e6ca, line 457 */
#line 457
					ILI_ERR("TP Reset failed during resume\n");
			}
			/*
			 * the TWO real calls, not inlined:
			 * "97fffe64 bl"@0xffffff8008a546d8 and
			 * "97fffe61 bl"@0xffffff8008a546e4
			 */
			ilitek_tddi_wq_ctrl(0, 1);
			ilitek_tddi_wq_ctrl(1, 1);
			/*
			 * "3909ed1f strb"@0xffffff8008a54700,
			 * "390a0113 strb"@0xffffff8008a54704: they fall BEFORE
			 * "97db7b73 bl"@0xffffff8008a54708 towards <printk>.
			 */
			idev->c635 = 0;
			idev->c640 = 1;
			/* "\x016ILITEK: (%s, %d): TP resume end\n"
			 * @0xffffff800923e6fd, line 463 */
#line 463
			ILI_INFO("TP resume end\n");
		}
		/* "940006a1 bl"@0xffffff8008a5470c */
		ilitek_plat_irq_enable();
		break;

	default:
		/* "\x013ILITEK: (%s, %d): Unknown sleep mode, %d\n"
		 * @0xffffff800923e720, line 468 */
#line 468
		ILI_ERR("Unknown sleep mode, %d\n", mode);
		/* "128002b3 mov"@0xffffff8008a54600 = -22 */
		ret = -EINVAL;
		break;
	}

	/* "94002130 bl"@0xffffff8008a54714 */
	ilitek_tddi_touch_release_all_point();
	/* "b902d51f str"@0xffffff8008a54720 */
	WRITE_ONCE(idev->c724, 0);
	/* "94107b2a bl"@0xffffff8008a54724 */
	mutex_unlock(&idev->touch_mutex);
	return ret;
}

/*
 * ilitek_tddi_fw_upgrade_handler() was reconstructed from the factory kernel disassembly (0xffffff8008a53a78, 208 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_fw_upgrade_handler(void)
{
	int ret;

	/* "b902cd09 str"@0xffffff8008a53a90 */
	WRITE_ONCE(idev->c716, 1);
	/* "b901bd1f str"@0xffffff8008a53a98 */
	idev->c444 = 0;

	/* "b941c100 ldr"@0xffffff8008a53a94, "94005291 bl"@0xffffff8008a53a9c */
	ret = ilitek_tddi_fw_upgrade(idev->c448);
	/*
	 * "34000120 cbz"@0xffffff8008a53aa4: the comparison is against zero, not
	 * on the sign.
	 */
	if (ret) {
#line 488
		ILI_INFO("FW upgrade fail\n");	/* line 488 */
		/* "12800009 mov"@0xffffff8008a53ac0: #0xffffffff = -1 */
		idev->c444 = -1;
	} else {
		/* "\x016ILITEK: (%s, %d): FW upgrade pass\n"
		 * @0xffffff800923e790, line 491 */
#line 491
		ILI_INFO("FW upgrade pass\n");
		/* "52800c89 mov"@0xffffff8008a53ae0: #0x64 = 100 */
		idev->c444 = 100;
	}

	/* "3943810a ldrb"@0xffffff8008a53ae8, "3500022a cbnz"@0xffffff8008a53af0 */
	if (!idev->c224) {
		/*
		 * THE ORDER IS MEASURED, not a convenience: "39038109 strb"
		 * @0xffffff8008a53b0c falls BEFORE "97db7e71 bl"
		 * @0xffffff8008a53b10 towards <printk> and reuses the `x8` loaded at
		 * "f9452288 ldr"@0xffffff8008a53ae4. Writing the field after the
		 * `printk` forces clang to reload the global, and the function
		 * grows by four bytes (measured: 212 against 208).
		 */
		idev->c224 = 1;
		/*
		 * "\x016ILITEK: (%s, %d): Registre touch to input subsystem\n"
		 * @0xffffff800923e7b5, line 497. The "Registre" typo is the
		 * factory's.
		 */
#line 497
		ILI_INFO("Registre touch to input subsystem\n");
		/* "940008a8 bl"@0xffffff8008a53b14 */
		ilitek_plat_input_register();
		/* "2a1f03e0 mov"@0xffffff8008a53b1c + "320003e1 orr"@0xffffff8008a53b18
		 * -> (0, 1); "94000152 bl"@0xffffff8008a53b20 */
		ilitek_tddi_wq_ctrl(0, 1);
		/* "320003e0 orr"@0xffffff8008a53b24 + "320003e1 orr"@0xffffff8008a53b28
		 * -> (1, 1); "9400014f bl"@0xffffff8008a53b2c */
		ilitek_tddi_wq_ctrl(1, 1);
	}

	/* "b902cd1f str"@0xffffff8008a53b34 */
	WRITE_ONCE(idev->c716, 0);
	return ret;
}

/*
 * ilitek_set_tp_data_len() was reconstructed from the factory kernel disassembly (0xffffff8008a53c80, 600 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_set_tp_data_len(int format)
{
	u8 cmd[2] = {0};	/* "79000bff strh"@0xffffff8008a53ca8: zeroed */
	int mode, len = 43, data_fmt = 0, ret = 0;

	/* "b9414114 ldr"@0xffffff8008a53cc4 */
	mode = idev->c320;

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a53ca0).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	switch (format) {
	case 0:
	case 6:
		/*
		 * The table sends these two straight to the tail: the initial values
		 * stand, `len = 43` and `data_fmt = 0`.
		 */
		break;

	case 1:
	case 7:
		/* "321f03f6 orr"@0xffffff8008a53cf4: #0x2 */
		data_fmt = 2;
		/* "3943e109 ldrb"@0xffffff8008a53ce4 (c248),
		 * "3943e50a ldrb"@0xffffff8008a53ce8 (c249),
		 * "3943e90b ldrb"@0xffffff8008a53cec (c250),
		 * "3943ed0c ldrb"@0xffffff8008a53cf0 (c251),
		 * "1b092d49 madd"@0xffffff8008a53cf8 -> c249*c248 + c250,
		 * "0b0c0129 add"@0xffffff8008a53cfc -> + c251,
		 * "531f7929 lsl"@0xffffff8008a53d00 -> * 2,
		 * "1100e135 add"@0xffffff8008a53d04 -> + 0x38 = 56 */
		len = (idev->c249 * idev->c248 + idev->c250 + idev->c251) * 2 + 56;
		break;

	case 2:
		/* "321e03f6 orr"@0xffffff8008a53d2c: #0x4 */
		data_fmt = 4;
		/* "321e0ff5 orr"@0xffffff8008a53d30: #0x3c = 60 */
		len = 60;
		break;

	case 3:
		/*
		 * "b9421d03 ldr"@0xffffff8008a53d38 (c540),
		 * "b9421908 ldr"@0xffffff8008a53d3c (c536),
		 * "7100111f cmp"@0xffffff8008a53d54 with #0x4,
		 * "1a890148 csel"@0xffffff8008a53d60 -> 174 : 47,
		 * "1a8b0189 csel"@0xffffff8008a53d64 -> 170 : 8,
		 * "7100047f cmp"@0xffffff8008a53d68 with #0x1,
		 * "1a890115 csel"@0xffffff8008a53d78 picks between the two pairs.
		 * The four constants: "528005e9 mov"@0xffffff8008a53d44 (47),
		 * "528015ca mov"@0xffffff8008a53d48 (174),
		 * "321d03eb orr"@0xffffff8008a53d4c (8),
		 * "5280154c mov"@0xffffff8008a53d50 (170).
		 */
		if (idev->c540 == 1)
			len = (idev->c536 == 4) ? 174 : 47;
		else
			len = (idev->c536 == 4) ? 170 : 8;
		/* "\x016ILITEK: (%s, %d): Gesture demo mode control = %d\n"
		 * @0xffffff800923e7ec, line 551 */
#line 551
		ILI_INFO("Gesture demo mode control = %d\n", idev->c540);
		/* "gesture_demo_en"@0xffffff800923e837,
		 * "b9421d01 ldr"@0xffffff8008a53d8c (l'argument is c540),
		 * "9400103e bl"@0xffffff8008a53d90 */
		ilitek_tddi_ic_func_ctrl("gesture_demo_en", idev->c540);
		/* "\x016ILITEK: (%s, %d): knock_en setting\n"
		 * @0xffffff800923e847, line 553 */
#line 553
		ILI_INFO("knock_en setting\n");
		/* "knock_en"@0xffffff800923e86d,
		 * "321d03e1 orr"@0xffffff8008a53db0: #0x8,
		 * "94001035 bl"@0xffffff8008a53db4 */
		ilitek_tddi_ic_func_ctrl("knock_en", 8);
		break;

	case 4:
		/* "321f03f6 orr"@0xffffff8008a53dc4: #0x2 */
		data_fmt = 2;
		/* "52801555 mov"@0xffffff8008a53dc8: #0xaa = 170 */
		len = 170;
		break;

	case 5:
		/* "320003f6 orr"@0xffffff8008a53dd0: #0x1 */
		data_fmt = 1;
		/* "321d03f5 orr"@0xffffff8008a53dd4: #0x8 */
		len = 8;
		break;

	default:
		/*
		 * "\x013ILITEK: (%s, %d): Unknow TP data format\n"
		 * @0xffffff800923e876, line 557. The "Unknow" typo is the
		 * factory's.
		 * The comparison that leads here is UNSIGNED
		 * ("54000308 b.hi"@0xffffff8008a53cac), so a negative `format`
		 * lands here and not in the table.
		 */
#line 557
		ILI_ERR("Unknow TP data format\n");
		/* "12800014 mov"@0xffffff8008a53d24: #0xffffffff = -1 */
		return -1;
	}

	/*
	 * THE ORDER IS MEASURED: "b9014513 str"@0xffffff8008a53df8 and
	 * "b9014915 str"@0xffffff8008a53dfc fall BEFORE
	 * "97db7db5 bl"@0xffffff8008a53e00 towards <printk>, and clang cannot move
	 * a store across an opaque call in either direction -- the order emitted is
	 * therefore the order of the source. Written after the `printk`, the global
	 * stays live here and has to be reloaded further on, at the +776 site where
	 * the factory has "f94522e8 ldr"@0xffffff8008a53e50: 596 bytes against 600
	 * (measured).
	 */
	idev->c324 = format;
	idev->c328 = len;

	/* "\x016ILITEK: (%s, %d): TP mode = %d, format = %d, len = %d\n"
	 * @0xffffff800923e8a1, line 564 */
#line 564
	ILI_INFO("TP mode = %d, format = %d, len = %d\n", mode, format, len);

	/*
	 * "32000268 orr"@0xffffff8008a53e04 (format | 1),
	 * "71001d1f cmp"@0xffffff8008a53e08 with #0x7,
	 * "54000220 b.eq"@0xffffff8008a53e0c: clang has folded `format == 6 ||
	 * format == 7` into `(format | 1) == 7`.
	 * "34000214 cbz"@0xffffff8008a53e10 adds `mode == 0`.
	 */
	if (format == 6 || format == 7 || mode == 0) {
		/* "321c0fe9 orr"@0xffffff8008a53e54: #0xf0,
		 * "390013e9 strb"@0xffffff8008a53e58,
		 * "390017f6 strb"@0xffffff8008a53e5c */
		cmd[0] = 0xf0;
		cmd[1] = data_fmt;
		/* "f9418508 ldr"@0xffffff8008a53e60 (+776 = `ilitek_i2c_write`),
		 * "910013e0 add"@0xffffff8008a53e64 (x0 = cmd),
		 * "321f03e1 orr"@0xffffff8008a53e68 (#0x2),
		 * "d63f0100 blr"@0xffffff8008a53e6c */
		ret = idev->c776(cmd, 2);
		if (ret < 0) {
			/* "\x013ILITEK: (%s, %d): switch to format %d failed\n"
			 * @0xffffff800923e8da, line 574 */
#line 574
			ILI_ERR("switch to format %d failed\n", format);
			/* "2a1f03e0 mov"@0xffffff8008a53e94 (#0),
			 * "97fffe7d bl"@0xffffff8008a53e98 */
			ilitek_tddi_switch_tp_mode(0);
		}
	/* "71003e9f cmp"@0xffffff8008a53e14 con #0xf,
	 * "54000441 b.ne"@0xffffff8008a53e18 */
	} else if (mode == 15) {
		/* "lpwg"@0xffffff800923e90a,
		 * "2a1603e1 mov"@0xffffff8008a53e24 (the argument is data_fmt),
		 * "94001018 bl"@0xffffff8008a53e28 */
		ret = ilitek_tddi_ic_func_ctrl("lpwg", data_fmt);
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): write gesture mode failed\n"
			 * @0xffffff800923e90f, line 580 */
#line 580
			ILI_ERR("write gesture mode failed\n");
	}
	/*
	 * "2a1f03f4 mov"@0xffffff8008a53ea0: in the other cases the return value
	 * is zero, not the previous value.
	 */

	return ret;
}

/*
 * ilitek_tddi_report_handler() was reconstructed from the factory kernel disassembly (0xffffff8008a5472c, 1820 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_report_handler(void)
{
	int len, ret, pid;
	u8 checksum, dbg_en_prima;
	u8 *buf;

	/* "39484d09 ldrb"@0xffffff8008a54744 (c531),
	 * "340001a9 cbz"@0xffffff8008a54748, then four `cbnz` and one `cbz`:
	 * "35000129 cbnz"@0xffffff8008a54758 (c708),
	 * "350000e9 cbnz"@0xffffff8008a54760 (c716),
	 * "350000a9 cbnz"@0xffffff8008a54768 (c728),
	 * "35000069 cbnz"@0xffffff8008a54770 (c720),
	 * "34000169 cbz"@0xffffff8008a54778 (c724). */
	/*
	 * "39656276 ldrb"@0xffffff8008a54750: the read falls BETWEEN the first and
	 * the second condition of the guard below, where clang hoisted it; in the
	 * source it comes before the guard.
	 */
	dbg_en_prima = ilitek_dbg_en;

	if (!idev->c531 || READ_ONCE(idev->c708) || READ_ONCE(idev->c716) ||
	    idev->c728 || READ_ONCE(idev->c720) || READ_ONCE(idev->c724)) {
		/* "\x016ILITEK: (%s, %d): ignore report request\n"
		 * @0xffffff800923e93e, line 598 */
#line 598
		ILI_INFO("ignore report request\n");
		return;
	}

	/* "3949e109 ldrb"@0xffffff8008a547a4,
	 * "34000149 cbz"@0xffffff8008a547a8 */
	if (idev->c632) {
		/* "\x016ILITEK: (%s, %d): ignore int triggered by recovery\n"
		 * @0xffffff800923e984, line 603 */
#line 603
		ILI_INFO("ignore int triggered by recovery\n");
		/* "3909e11f strb"@0xffffff8008a547c8 */
		idev->c632 = 0;
		return;
	}

	/* inlined, see the file header */
	ilitek_tddi_wq_ctrl(0, 0);
	ilitek_tddi_wq_ctrl(1, 0);

	/* "b9414109 ldr"@0xffffff8008a548a0 (c320),
	 * "71003d3f cmp"@0xffffff8008a548a4 con #0xf */
	if (idev->c320 == 0x0f) {
		/* "f9401100 ldr"@0xffffff8008a548ac (c32),
		 * "97eab22a bl"@0xffffff8008a548b0 */
		__pm_stay_awake(idev->c32);
		/* the forty `bl __const_udelay`, see the file header */
		mdelay(40);
	}

	/* "b9414913 ldr"@0xffffff8008a54a9c */
	len = idev->c328;
	/*
	 * guarded: "39656269 ldrb"@0xffffff8008a54a98.
	 * "\x016ILITEK: (%s, %d): Packget length = %d\n"
	 * @0xffffff800923e9ba, line 618. The "Packget" typo is the
	 * factory's and is reproduced.
	 */
#line 618
	ILI_DBG("Packget length = %d\n", len);

	/*
	 * "34000373 cbz"@0xffffff8008a54ac0 and
	 * "7120067f cmp"@0xffffff8008a54ac4 + "5400032a b.ge"@0xffffff8008a54ac8:
	 * the first term is a comparison against ZERO, not against one -- a
	 * NEGATIVE `len` passes both checks. That is what the binary does.
	 */
	if (len == 0 || len > 2048) {
		/* "\x013ILITEK: (%s, %d): Length of packet (%d) is invaild\n"
		 * @0xffffff800923e9e3, line 621 */
#line 621
		ILI_ERR("Length of packet (%d) is invaild\n", len);
		goto out;
	}

	/*
	 * __memset() was reconstructed from the factory kernel disassembly (0xffffff8008a54ad0).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	__memset(idev->c264, 0, 2048);

	/* "f9418909 ldr"@0xffffff8008a54ae8 (+784),
	 * "d63f0120 blr"@0xffffff8008a54af0 */
	ret = idev->c784(idev->c264, len);
	/* "37f804a0 tbnz"@0xffffff8008a54af8 */
	if (ret < 0) {
		/* "\x013ILITEK: (%s, %d): Read report packet failed, ret = %d\n"
		 * @0xffffff800923ea19, line 629 */
#line 629
		ILI_ERR("Read report packet failed, ret = %d\n", ret);
		/* "31000a7f cmn"@0xffffff8008a54ba8: a comparison with -2 */
		if (ret == -2) {
			/* "940011b2 bl"@0xffffff8008a54bb0 */
			ilitek_tddi_ic_get_pc_counter_forwdt();
			/*
			 * "71003d3f cmp"@0xffffff8008a54bbc (c320 with 0xf),
			 * "34000069 cbz"@0xffffff8008a54bc8 (c532),
			 * "34000be8 cbz"@0xffffff8008a54bd0 (c637): the third
			 * term is NEGATED -- the jump to the gesture branch is
			 * a `cbz`, so we get there when c637 is ZERO.
			 */
			if (idev->c320 == 0x0f && idev->c532 && !idev->c637) {
				/* "\x013ILITEK: (%s, %d): Gesture failed,
				 * doing gesture recovery\n"
				 * @0xffffff800923ea52, line 633 */
#line 633
				ILI_ERR("Gesture failed, doing gesture recovery\n");
				/* inlined, see the file header */
				if (ilitek_tddi_gesture_recovery() < 0)
					/* "\x013ILITEK: (%s, %d): Failed to
					 * recover gesture\n"
					 * @0xffffff800923ea8e, line 635 */
#line 635
					ILI_ERR("Failed to recover gesture\n");
				/*
				 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a54bf8, 1820 bytes).
				 *
				 * The working notes -- the disassembly citations, the measurements against
				 * the factory binary and the reasoning behind each choice -- are in
				 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
				 * in the oracolo repository. They are kept in Italian, as the project's
				 * internal record.
				 */
				idev->c632 = 1;
			} else {
				/* "\x013ILITEK: (%s, %d): SPI ACK failed,
				 * doing spi recovery\n"@0xffffff800923eabd,
				 * line 638 */
#line 638
				ILI_ERR("SPI ACK failed, doing spi recovery\n");
				/* "97fffcdf bl"@0xffffff8008a54bec */
				ilitek_tddi_spi_recovery();
				idev->c632 = 1;
			}
		}
		goto out;
	}

	/*
	 * "32007be8 orr"@0xffffff8008a54afc + "6b08027f cmp"@0xffffff8008a54b00
	 * + "54000220 b.eq"@0xffffff8008a54b04: 0x7fffffff is written as a raw
	 * value, not translated into a name the binary does not give.
	 */
	if (ret == 0x7fffffff)
		goto out;

	/* "7120067f cmp"@0xffffff8008a54b08 + "540007ab b.lt"@0xffffff8008a54b0c */
	if (ret > 2048) {
		/* "\x013ILITEK: (%s, %d): Returned length (%d) is invaild\n"
		 * @0xffffff800923eaf5, line 650 */
#line 650
		ILI_ERR("Returned length (%d) is invaild\n", ret);
		goto out;
	}

	/* "finger report"@0xffffff800923eb2a,
	 * "321d03e1 orr"@0xffffff8008a54c0c (#8),
	 * "2a1f03e3 mov"@0xffffff8008a54c18 (0),
	 * "940017af bl"@0xffffff8008a54c1c */
	ilitek_dump_data(idev->c264, 8, ret, 0, "finger report");

	/*
	 * "51000674 sub"@0xffffff8008a54c24 (ret - 1),
	 * "94001c59 bl"@0xffffff8008a54c30,
	 * "12001c03 and"@0xffffff8008a54c38: of the return value the low byte is
	 * what matters.
	 */
	checksum = ilitek_calc_packet_checksum(idev->c264, ret - 1);

	/*
	 * "3874c904 ldrb"@0xffffff8008a54c40 = [x8, w20, sxtw]: the index is
	 * SIGN-extended, that is, it is an `int`.
	 * "6b04007f cmp"@0xffffff8008a54c44, "54000060 b.eq"@0xffffff8008a54c48,
	 * then "3949d929 ldrb"@0xffffff8008a54c4c (c630) and
	 * "34000549 cbz"@0xffffff8008a54c50: the second term is NEGATED.
	 */
	if (checksum != ((u8 *)idev->c264)[ret - 1] && !idev->c630) {
		/* "\x013ILITEK: (%s, %d): Wrong checksum, checksum = %x,
		 * buf = %x, len = %d\n"@0xffffff800923eb38, line 661 */
		ILI_ERR("Wrong checksum, checksum = %x, buf = %x, len = %d\n",
#line 661
			checksum, ((u8 *)idev->c264)[ret - 1], ret);
		/* "320003ea orr"@0xffffff8008a54d18 +
		 * "3925610a strb"@0xffffff8008a54d20 */
		ilitek_dbg_en = 1;
		/* "finger report with wrong"@0xffffff800923eb7f,
		 * "94001766 bl"@0xffffff8008a54d40 */
		ilitek_dump_data(idev->c264, 8, ret, 0,
				 "finger report with wrong");
		/*
		 * "39256296 strb"@0xffffff8008a54d44: it writes back the value read
		 * at the top, not a constant.
		 */
		ilitek_dbg_en = dbg_en_prima;
		goto out;
	}

	/* "39400114 ldrb"@0xffffff8008a54c5c */
	pid = ((u8 *)idev->c264)[0];
	/* guarded: "39656129 ldrb"@0xffffff8008a54c58.
	 * "\x016ILITEK: (%s, %d): Packet ID = %x\n"@0xffffff800923eb98,
	 * line 669 */
#line 669
	ILI_DBG("Packet ID = %x\n", pid);

	/*
	 * The pointer is RE-READ from the field: "f9408508 ldr"@0xffffff8008a54c84
	 * reloads `idev->c264` after the `printk`, something clang would not do
	 * if the source kept it in a local across the call.
	 */
	buf = idev->c264;
	/*
	 * "7102de9f cmp"@0xffffff8008a54c88 with #0xb7,
	 * "38403d14 ldrb"@0xffffff8008a54c90 = [x8,#3]! (pre-indexed with
	 * writeback: the pointer advances by 3 and stays advanced).
	 */
	if (pid == 0xb7) {
		buf += 3;
		pid = buf[0];
	}

	switch (pid) {
	case 0x5a:
		/* "94001e23 bl"@0xffffff8008a54df4 */
		ilitek_tddi_report_ap_mode(buf, ret);
		break;
	case 0x5c:
		/*
		 * "93407e61 sxtw"@0xffffff8008a54dfc: the second argument is
		 * 64-bit, sign-extended from an `int`.
		 */
		demo_debug_info_mode(buf, ret);
		break;
	case 0x7a:
		/* "940022d2 bl"@0xffffff8008a54cbc */
		ilitek_tddi_report_i2cuart_mode(buf, ret);
		break;
	case 0xa7:
		/* "94001ff2 bl"@0xffffff8008a54e14 */
		ilitek_tddi_report_debug_mode(buf, ret);
		break;
	case 0xaa:
		/* "940020f2 bl"@0xffffff8008a54e24 */
		ilitek_tddi_report_gesture_mode(buf, ret);
		break;
	case 0xae:
		/*
		 * "39400503 ldrb"@0xffffff8008a54cdc = buf[1].
		 * "\x016ILITEK: (%s, %d): gesture fail reason code = 0x%02x"
		 * @0xffffff800923ebbc, line 690: it does NOT end with \n, and that is a
		 * factory defect which is reproduced.
		 */
#line 690
		ILI_INFO("gesture fail reason code = 0x%02x", buf[1]);
		break;
	default:
		/* "\x013ILITEK: (%s, %d): Unknown packet id, %x\n"
		 * @0xffffff800923ebf2, line 696 */
#line 696
		ILI_ERR("Unknown packet id, %x\n", pid);
		break;
	}

out:
	/* "71003d3f cmp"@0xffffff8008a54b50 (c320 con 0xf),
	 * "54000160 b.eq"@0xffffff8008a54b54 */
	if (idev->c320 != 0x0f) {
		ilitek_tddi_wq_ctrl(0, 1);
		ilitek_tddi_wq_ctrl(1, 1);
	}
	/*
	 * the field is RE-READ: "b9414109 ldr"@0xffffff8008a54b74 after a
	 * second "f94522a8 ldr"@0xffffff8008a54b70 on `idev`. They are two
	 * distinct `if`s in the source, not one with an `else`.
	 */
	if (idev->c320 == 0x0f)
		/* "97eaaf43 bl"@0xffffff8008a54b84 */
		__pm_relax(idev->c32);
}

/*
 * ilitek_tddi_reset_ctrl() was reconstructed from the factory kernel disassembly (0xffffff8008a53b48, 312 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_reset_ctrl(int mode)
{
	int ret = 0;

	/* "b902c509 str"@0xffffff8008a53b64 */
	WRITE_ONCE(idev->c708, 1);

	/* "7100041f cmp"@0xffffff8008a53b60, "54000221 b.ne"@0xffffff8008a53b68 */
	if (mode == 1) {
		/*
		 * "\x016ILITEK: (%s, %d): TP IC Code RST \n"
		 * @0xffffff800923ec1d, line 723. The space before the \n is the
		 * factory's.
		 */
#line 723
		ILI_INFO("TP IC Code RST \n");
		/* "94001133 bl"@0xffffff8008a53b84 */
		ret = ilitek_tddi_ic_code_reset();
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): IC Code reset failed\n"
			 * @0xffffff800923ec59, line 726 */
#line 726
			ILI_ERR("IC Code reset failed\n");
		goto out;
	}

	/* "390a011f strb"@0xffffff8008a53bb0 */
	idev->c640 = 0;
	/* "940013e9 bl"@0xffffff8008a53bb4 */
	ilitek_tddi_ic_check_otp_prog_mode();

	switch (mode) {
	case 0:
		/* "\x016ILITEK: (%s, %d): TP IC whole RST\n"
		 * @0xffffff800923ec83, line 729 */
#line 729
		ILI_INFO("TP IC whole RST\n");
		/* "94001166 bl"@0xffffff8008a53c00 */
		ret = ilitek_tddi_ic_whole_reset();
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): IC whole reset failed\n"
			 * @0xffffff800923eca8, line 732 */
#line 732
			ILI_ERR("IC whole reset failed\n");
		break;
	case 2:
		/* "\x016ILITEK: (%s, %d): TP HW RST\n"@0xffffff800923ecd3,
		 * line 735 */
#line 735
		ILI_INFO("TP HW RST\n");
		/* "9400084c bl"@0xffffff8008a53bdc */
		ilitek_plat_tp_reset();
		/*
		 * "2a1f03f3 mov"@0xffffff8008a53be0: the return value is cleared
		 * even when `mode` was 2, not left at the previous value.
		 */
		ret = 0;
		break;
	default:
		/* "\x013ILITEK: (%s, %d): Unknown reset mode, %d\n"
		 * @0xffffff800923ecf2, line 739 */
#line 739
		ILI_ERR("Unknown reset mode, %d\n", mode);
		/* "128002b3 mov"@0xffffff8008a53c44: #0xffffffea = -22 */
		ret = -EINVAL;
		break;
	}

	/* "b902c91f str"@0xffffff8008a53c4c */
	WRITE_ONCE(idev->c712, 0);

out:
	/*
	 * The tail writes five things, and two of them with a SINGLE instruction:
	 * "d2c00569 mov"@0xffffff8008a53c54 loads 0x2b00000000 and
	 * "f9000169 str"@0xffffff8008a53c68 writes it at `idev + 324`
	 * ("9105110b add"@0xffffff8008a53c60, #0x144 = 324), that is c324 = 0 and
	 * c328 = 43 in one go. That clang can merge the two stores is also the
	 * proof that neither field is volatile.
	 */
	idev->c324 = 0;
	idev->c328 = 43;
	/* "3909d91f strb"@0xffffff8008a53c64 */
	idev->c630 = 0;
	/* "b902c51f str"@0xffffff8008a53c6c */
	WRITE_ONCE(idev->c708, 0);
	/* "390a010a strb"@0xffffff8008a53c70 con w10 = 1
	 * ("320003ea orr"@0xffffff8008a53c58) */
	idev->c640 = 1;
	return ret;
}

/*
 * ilitek_update_tp_module_info() was reconstructed from the factory kernel disassembly (0xffffff800923f17f).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void ilitek_update_tp_module_info(void)
{
	/* "\x013ILITEK: (%s, %d): Couldn't find any tp modules, applying
	 * default settings\n"@0xffffff800923f132, line 844 */
#line 844
	ILI_ERR("Couldn't find any tp modules, applying default settings\n");

	/* "DEF"@0xffffff80090de706, "f9014903 str"@0xffffff8008a550dc */
	idev->c656 = (u64)(uintptr_t)"DEF";
	/* "/sdcard/ILITEK_FW"@0xffffff800923f19c,
	 * "f9014d05 str"@0xffffff8008a550e0 */
	idev->c664 = (u64)(uintptr_t)"/sdcard/ILITEK_FW";
	/* "ILITEK_FW"@0xffffff800923f1a4, "f9015106 str"@0xffffff8008a550e4 */
	idev->c672 = (u64)(uintptr_t)"ILITEK_FW";
	/* "/sdcard/mp.ini"@0xffffff800923f1ae,
	 * "f9015504 str"@0xffffff8008a550e8 */
	idev->c680 = (u64)(uintptr_t)"/sdcard/mp.ini";
	/* "mp.ini"@0xffffff800923f1b6, "f9015909 str"@0xffffff8008a550ec */
	idev->c688 = (u64)(uintptr_t)"mp.ini";
	/*
	 * ""@0xffffff800996b2f8 -- twenty bytes of zeroes, that is the empty
	 * literal merged by the linker. "f9015d0a str"@0xffffff8008a550f0
	 */
	idev->c696 = (u64)(uintptr_t)"";
	/* "5298080b mov"@0xffffff8008a550a4 plus
	 * "72a0002b movk"@0xffffff8008a550c8 = 0xc040 | (1 << 16) = 0x1c040 =
	 * 114752. "b902890b str"@0xffffff8008a550f4 */
	idev->c648 = 0x1c040;

	/*
	 * "\x016ILITEK: (%s, %d): Found %s module: ini path = %s,
	 * fw path = (%s, %s, %d)\n"@0xffffff800923f1bd, line 859.
	 * THE ORDER OF THE ARGUMENTS IS MEASURED, and it is NOT that of the stores:
	 * x3 = +656, x4 = +680, x5 = +664, x6 = +672, w7 = +648
	 * ("911c1863 add"@0xffffff8008a550b0, "9106b884 add"@0xffffff8008a550bc,
	 * "910670a5 add"@0xffffff8008a550b4, "910690c6 add"@0xffffff8008a550b8,
	 * "52980807 mov"@0xffffff8008a550ac + "72a00027 movk"@0xffffff8008a550d4).
	 */
	ILI_INFO("Found %s module: ini path = %s, fw path = (%s, %s, %d)\n",
		 (const char *)(uintptr_t)idev->c656,
		 (const char *)(uintptr_t)idev->c680,
		 (const char *)(uintptr_t)idev->c664,
		 (const char *)(uintptr_t)idev->c672,
#line 859
		 idev->c648);

	/*
	 * "b902851f str"@0xffffff8008a55100, with `idev` RELOADED after the
	 * `printk` ("f9452288 ldr"@0xffffff8008a550fc): it is a separate
	 * statement, after the print. Whether it lives inside this function or in
	 * the caller the binary does not say -- inlining erases the boundary --
	 * and it is a CHOICE.
	 */
	idev->c644 = 0;
}

/*
 * ilitek_tddi_init() was reconstructed from the factory kernel disassembly (0xffffff8008a54e48, 1004 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_init(void)
{
	/* "\x016ILITEK: (%s, %d): driver version = %s\n"
	 * @0xffffff800923ed1e, line 870; the argument is
	 * "2.0.6.0.191122"@0xffffff800923ed58
	 * ("91356063 add"@0xffffff8008a54e70). */
#line 870
	ILI_INFO("driver version = %s\n", "2.0.6.0.191122");

	/* "9101c100 add"@0xffffff8008a54e90 (+112),
	 * "97db5d99 bl"@0xffffff8008a54e98 to <__mutex_init> */
	mutex_init(&idev->touch_mutex);
	/* "91024100 add"@0xffffff8008a54eac (+144) */
	mutex_init(&idev->debug_mutex);
	/* "9102c100 add"@0xffffff8008a54ec8 (+176) */
	mutex_init(&idev->debug_read_mutex);
	/*
	 * "9108e100 add"@0xffffff8008a54ee4 (+568),
	 * "97db20e3 bl"@0xffffff8008a54eec towards <__init_waitqueue_head>.
	 * The parentheses really are there: the literal is "&(idev->inq)".
	 */
	init_waitqueue_head(&(idev->inq));
	/*
	 * "b900d11f str"@0xffffff8008a54f08: four bytes to zero, which is
	 * `spin_lock_init` without lockdep on arm64.
	 */
	spin_lock_init(&idev->c208);
	/*
	 * "b902e91f str"@0xffffff8008a54f0c (done at +744) plus
	 * "910bc100 add"@0xffffff8008a54f00 (+752) and
	 * "97db20da bl"@0xffffff8008a54f10: it is `init_completion`, and the proof
	 * is the literal at 0xffffff80090e2854, which `leggi_stringa.py` reads as
	 * `&x->wait` and which lives INSIDE the kernel macro, not in the driver.
	 */
	init_completion(&idev->c744);

	/*
	 * THE ORDER IS MEASURED AND IT IS NOT ASCENDING: 704, 712, 708, 716, 720,
	 * 724, 732, 736 -- "b902c11f str"@0xffffff8008a54f18,
	 * "b902c91f str"@0xffffff8008a54f1c, "b902c51f str"@0xffffff8008a54f20,
	 * "b902cd1f str"@0xffffff8008a54f24, "b902d11f str"@0xffffff8008a54f28,
	 * "b902d51f str"@0xffffff8008a54f2c, "b902dd1f str"@0xffffff8008a54f30,
	 * "b902e11f str"@0xffffff8008a54f34.
	 * +704 and +708 are adjacent and 8-aligned: were they contiguous in the
	 * source clang would merge them into a single `str xzr`. It does not, so
	 * the source order is this one.
	 */
	WRITE_ONCE(idev->c704, 0);
	WRITE_ONCE(idev->c712, 0);
	WRITE_ONCE(idev->c708, 0);
	WRITE_ONCE(idev->c716, 0);
	WRITE_ONCE(idev->c720, 0);
	WRITE_ONCE(idev->c724, 0);
	WRITE_ONCE(idev->c732, 0);
	WRITE_ONCE(idev->c736, 0);

	/* "940016ce bl"@0xffffff8008a54f38 */
	ilitek_tddi_ic_init();

	/*
	 * ilitek_tddi_wq_init() was reconstructed from the factory kernel disassembly (0xffffff8008a54f3c).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	ilitek_tddi_wq_init();

	/*
	 * "9109d50b add"@0xffffff8008a55018 (idev + 629) plus
	 * "79000169 strh"@0xffffff8008a55020: a 16-bit store of the value 1
	 * covers +629 = 1 and +630 = 0. They are two fields, not one.
	 */
	idev->c629 = 1;
	idev->c630 = 0;
	/* "3909fd1f strb"@0xffffff8008a55024 */
	idev->c639 = 0;
	/* "3909dd1f strb"@0xffffff8008a55028 */
	idev->c631 = 0;
	/*
	 * "910c614a add"@0xffffff8008a55014 = 0xffffff8008a5c318 =
	 * `demo_debug_info_id0` (group E), "f901a90a str"@0xffffff8008a5502c.
	 * It is the only measurement the block gives on the contents of +848: it
	 * says element 0 is this symbol, NOT how many elements there are
	 * (divergence D4).
	 */
	idev->c848[0] = demo_debug_info_id0;

	/* "b901451f str"@0xffffff8008a5503c */
	idev->c324 = 0;
	/* "3903811f strb"@0xffffff8008a55040 */
	idev->c224 = 0;

	/* "320003e0 orr"@0xffffff8008a55034 (1), "2a1f03e1 mov"@0xffffff8008a55038
	 * (0), "94000859 bl"@0xffffff8008a55044,
	 * "37f80c80 tbnz"@0xffffff8008a55048 */
	if (ilitek_ice_mode_ctrl(1, 0) < 0) {
		/* "\x013ILITEK: (%s, %d): Not found ilitek chips\n"
		 * @0xffffff800923edb2, line 910 */
#line 910
		ILI_ERR("Not found ilitek chips\n");
		/* "12800240 mov"@0xffffff8008a551f0 = -19 */
		return -ENODEV;
	}

	/* "9400161c bl"@0xffffff8008a5504c,
	 * "36f800e0 tbz"@0xffffff8008a55050 */
	if (ilitek_tddi_ic_get_info() < 0)
		/* "\x013ILITEK: (%s, %d): Chip info is incorrect\n"
		 * @0xffffff800923edde, line 915 */
#line 915
		ILI_ERR("Chip info is incorrect\n");

	/* inlined, see above */
	ilitek_update_tp_module_info();

	/* "94007179 bl"@0xffffff8008a55104 */
	ilitek_tddi_node_init();
	/* "9400562d bl"@0xffffff8008a55108 */
	ilitek_tddi_fw_read_flash_info();

	/* "2a1f03e0 mov"@0xffffff8008a5510c, "2a1f03e1 mov"@0xffffff8008a55110,
	 * "94000825 bl"@0xffffff8008a55114 */
	if (ilitek_ice_mode_ctrl(0, 0) < 0)
		/*
		 * "\x013ILITEK: (%s, %d): Failed to disable ice mode failed
		 * during init\n"@0xffffff800923ee0a, line 932. The repeated
		 * "failed ... failed" is the factory's.
		 */
#line 932
		ILI_ERR("Failed to disable ice mode failed during init\n");

	/* "3909f11f strb"@0xffffff8008a55138 */
	idev->c636 = 0;
	/* "9400130e bl"@0xffffff8008a5513c */
	ilitek_tddi_ic_get_core_ver();
	/* "94001537 bl"@0xffffff8008a55140 */
	ilitek_tddi_ic_get_protocl_ver();
	/* "940013b0 bl"@0xffffff8008a55144 */
	ilitek_tddi_ic_get_fw_ver();
	/* "940014a7 bl"@0xffffff8008a55148 */
	ilitek_tddi_ic_get_tp_info();
	/* "94001420 bl"@0xffffff8008a5514c */
	ilitek_tddi_ic_get_panel_info();
	/*
	 * "3909f113 strb"@0xffffff8008a5516c: it falls BEFORE
	 * "97db78d9 bl"@0xffffff8008a55170 towards <printk>.
	 */
	idev->c636 = 1;

	/*
	 * "\x016ILITEK: (%s, %d): Registre touch to input subsystem\n"
	 * @0xffffff800923e7b5, line 948. The "Registre" typo is the
	 * factory's -- and it is the SAME literal
	 * `ilitek_tddi_fw_upgrade_handler` uses at line 497.
	 */
#line 948
	ILI_INFO("Registre touch to input subsystem\n");
	/* "94000310 bl"@0xffffff8008a55174 */
	ilitek_plat_input_register();

	/*
	 * the two real calls: "97fffbba bl"@0xffffff8008a55180 and
	 * "97fffbb7 bl"@0xffffff8008a5518c
	 */
	ilitek_tddi_wq_ctrl(0, 1);
	ilitek_tddi_wq_ctrl(1, 1);

	/* "39038113 strb"@0xffffff8008a5519c, BEFORE the following call */
	idev->c224 = 1;

	/* "ili_wakelock"@0xffffff800923ee4d ("91393400 add"@0xffffff8008a55198),
	 * "97eaae77 bl"@0xffffff8008a551a0,
	 * "f9001128 str"@0xffffff8008a551b0 */
	idev->c32 = wakeup_source_register("ili_wakelock");
	/* "b5000208 cbnz"@0xffffff8008a551b4 */
	if (!idev->c32)
		/* "\x013ILITEK: (%s, %d): wakeup source request failed\n"
		 * @0xffffff800923ee5a, line 957 */
#line 957
		ILI_ERR("wakeup source request failed\n");

	/*
	 * The return value is ZERO even when the wakeup source is missing: there
	 * are TWO `mov w0, wzr`, "2a1f03e0 mov"@0xffffff8008a551ac on the path that
	 * skips the print and "2a1f03e0 mov"@0xffffff8008a551d0 on the one that
	 * makes it. The only non-zero return is the -19 of the "Not found ilitek
	 * chips" branch.
	 */
	return 0;
}

/*
 * ilitek_tddi_dev_remove() was reconstructed from the factory kernel disassembly (0xffffff8008a55234, 208 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_dev_remove(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): remove ilitek dev\n"@0xffffff800923ee8c,
	 * line 964. It is printed BEFORE the check on `idev`.
	 */
#line 964
	ILI_INFO("remove ilitek dev\n");

	/* "b40004c8 cbz"@0xffffff8008a55260 */
	if (!idev)
		return;

	/*
	 * "b9415d00 ldr"@0xffffff8008a55264 (c348) then
	 * "b9415900 ldr"@0xffffff8008a55270 (c344): this is the order, the higher
	 * field first.
	 */
	gpio_free(idev->c348);
	gpio_free(idev->c344);

	/* "f944b288 ldr"@0xffffff8008a5527c, "b4000108 cbz"@0xffffff8008a55280 */
	if (ilitek_wq_esd) {
		/* "97d9ee64 bl"@0xffffff8008a5528c */
		cancel_delayed_work_sync(&ilitek_esd_work);
		/* "97d9ea10 bl"@0xffffff8008a55294 */
		flush_workqueue(ilitek_wq_esd);
		/* "97d9f2b5 bl"@0xffffff8008a5529c */
		destroy_workqueue(ilitek_wq_esd);
	}

	/* "f944e688 ldr"@0xffffff8008a552a4, "b4000108 cbz"@0xffffff8008a552a8 */
	if (ilitek_wq_bat) {
		/* "97d9ee5a bl"@0xffffff8008a552b4 */
		cancel_delayed_work_sync(&ilitek_bat_work);
		/* "97d9ea06 bl"@0xffffff8008a552bc */
		flush_workqueue(ilitek_wq_bat);
		/* "97d9f2ab bl"@0xffffff8008a552c4 */
		destroy_workqueue(ilitek_wq_bat);
	}

	/*
	 * "b4000060 cbz"@0xffffff8008a552d0: the guard is there even though
	 * `wakeup_source_unregister` already handles NULL.
	 */
	if (idev->c32)
		wakeup_source_unregister(idev->c32);

	/* "f9408500 ldr"@0xffffff8008a552dc + "97dfed34 bl"@0xffffff8008a552e0,
	 * then "f9402100 ldr"@0xffffff8008a552e8 +
	 * "97dfed31 bl"@0xffffff8008a552ec: both `kfree`, with no guard. */
	kfree(idev->c264);
	kfree(idev->c64);

	ilitek_tddi_interface_dev_exit(idev);
}

/*
 * ilitek_tddi_dev_init() was reconstructed from the factory kernel disassembly (0xffffff8008a55304, 88 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_main.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_tddi_dev_init(struct ilitek_hwif_info *hwif)
{
	/* "\x016ILITEK: (%s, %d): TP Interface: %s\n"@0xffffff800923eeca,
	 * line 993 */
	ILI_INFO("TP Interface: %s\n",
#line 993
		 (*(const u8 *)hwif == 0x18) ? "I2C" : "SPI");

	/* "940000d1 bl"@0xffffff8008a5534c: the argument is `hwif`, turned
	 * without touching it ("aa1303e0 mov"@0xffffff8008a55348). */
	return ilitek_tddi_interface_dev_init(hwif);
}
