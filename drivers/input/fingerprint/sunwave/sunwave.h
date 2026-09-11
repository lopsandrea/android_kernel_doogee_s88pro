// SPDX-License-Identifier: GPL-2.0
/*
 * The interface between the Sunwave driver's two translation units.
 *
 * It holds what sf_ctl.c and sf_hw.c have to share, and nothing else: the
 * private data structure (which sf_ctl.c instantiates and sf_hw.c fills with
 * function pointers), the two global functions of the hardware side, and the
 * variable sf_probe writes and sf_ctl_driver_init reads.
 *
 * Reconstructed from the disassembly of the factory kernel.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sunwave.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#ifndef __SUNWAVE_H
#define __SUNWAVE_H

#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/notifier.h>
#include <linux/pm_wakeup.h>
#include <linux/spi/spi.h>
#include <linux/types.h>
#include <linux/workqueue.h>

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998dbf0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sunwave.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct sf_ctl_device {
	/*
	 * Offset 0. sf_remove: "aa1303e0  mov x0, x19" with x19 at the
	 * base, right before bl <misc_deregister> at 0xffffff8008a8315c.
	 * this_device of this miscdevice sits at 48, which is the offset
	 * read by sf_ctl_device_event ("f9461108  ldr x8, [x8,#3104]" ->
	 * 0x998dc20 = base+48) -- it matches the struct miscdevice ABI of
	 * this sublevel, which is the cross-check confirming that the
	 * first field really is a miscdevice.
	 */
	struct miscdevice miscdev;
	/* Offset 80: unidentified, none of the 18 functions touches it. */
	unsigned char __resv0[8];
	/*
	 * Offset 88 (0x58). sf_remove:
	 * "b94c4900  ldr w0, [x8,#3144]" (0x998dc48 = base+88), then
	 * "b9005a7f  str wzr, [x19,#88]".
	 */
	int irq;
	/*
	 * Offset 96 (0x60). sf_probe initialises a work_struct here:
	 * "f9001a8a  str x10, [x20,#48]" with x10 = 0xfffffffe0
	 * (WORK_STRUCT_NO_POOL), "f9001e8b  str x11, [x20,#56]" and
	 * "a904328b  stp x11, x12, [x20,#64]" with x11 = &work.entry (a list
	 * pointing at itself) and x12 = 0xffffff8008a8319c =
	 * sf_ctl_device_event -- that is, INIT_WORK(&ctl->work,
	 * sf_ctl_device_event), with x20 = 0x998dc20 = base+48.
	 */
	struct work_struct work;
	/*
	 * Offset 128 (0x80). sf_remove: "f9400260  ldr x0, [x19]" with
	 * x19 = 0x998dc70 = base+128, right before
	 * "bl <input_unregister_device>".
	 */
	struct input_dev *input;
	/*
	 * Offset 136 (0x88). sf_ctl_device_power:
	 * "f9404500  ldr x0, [x8,#136]".
	 */
	struct regulator *vdd_reg;
	/* Offset 144 (0x90). sf_platform_init: "a9093288  stp x8, x12, [x20,#144]". */
	int (*init_gpio_pins)(struct sf_ctl_device *ctl);
	/* Offset 152 (0x98). sf_platform_init, the same stp as above (x12). */
	int (*free_gpio)(struct sf_ctl_device *ctl);
	/*
	 * Offset 160 (0xa0). sf_platform_init: "a90a2a89  stp x9, x10, [x20,#160]".
	 * The parameter is a bool, not an int -- see the long comment above
	 * sf_ctl_device_power.
	 */
	int (*power)(bool on);
	/*
	 * Offset 168 (0xa8). sf_platform_init, the same stp as above (x10).
	 * Here too the parameter is a bool: sf_spi_clock_enable tests "on" with
	 * 360000e0 tbz w0,#0 at 0xffffff8008a83edc, a single bit. This
	 * pointer IS invoked: sf_ctl_ioctl does
	 * "f9464d08  ldr x8, [x8,#3224]" + "320003e0 orr w0, wzr, #0x1" +
	 * "d63f0100 blr x8" (and the variant with w0 = 0), and sf_probe invokes it
	 * from sf_read_sensor_id.
	 */
	int (*spi_clock_enable)(bool on);
	/*
	 * Offset 176 (0xb0). sf_platform_init: "f9005a8b  str x11, [x20,#176]".
	 * Invoked by sf_ctl_ioctl: "f9465108  ldr x8, [x8,#3232]" +
	 * "d63f0100 blr x8", with no arguments.
	 */
	int (*reset)(void);
	/*
	 * Offset 184 (0xb8). sf_probe: "9102e2b4  add x20, x21, #0xb8"
	 * then wakeup_source_prepare/wakeup_source_add; sf_remove:
	 * "9102e274  add x20, x19, #0xb8" then wakeup_source_remove/_drop.
	 * The offset of the next field (368) minus 184 makes 184, which is
	 * exactly sizeof(struct wakeup_source) on this sublevel: that is the
	 * confirmation that there is nothing else between the two fields.
	 */
	struct wakeup_source ws;
	/*
	 * Offset 368 (0x170). sf_probe: "91046260  add x0, x19, #0x118"
	 * with x19 = 0x998dc48 = base+88, that is base+368, passed to
	 * fb_register_client; and "f9008e68  str x8, [x19,#280]" (the same
	 * address) with x8 = 0xffffff8008a83218 = sf_fb_notifier_callback,
	 * that is notifier_call. sf_remove: "9103c260  add x0, x19, #0xf0" with
	 * x19 = base+128, again base+368, passed to fb_unregister_client.
	 */
	struct notifier_block notifier;
	/* Offset 392: unidentified, none of the 18 touches it. */
	unsigned char __resv1[8];
	/*
	 * Offset 400 (0x190). An int, not a pointer: sf_ctl_ioctl
	 * reads it with "b94d818b  ldr w11, [x12,#3456]" (0x998dd80 = base+400,
	 * `ldr w` = 32-bit) and hands it to user space with the
	 * bics/csel sequence + "b900014b  str w11, [x10]" which is put_user on 4 bytes.
	 * The NAME is not readable from the binary: no literal names it.
	 * None of the 18 functions ever writes it.
	 */
	int field_400;
	int __pad_404;
	/*
	 * Offset 408 (0x198). sf_probe: "f900ceb4  str x20, [x21,#408]"
	 * with x20 = the spi_device passed to probe; sf_ctl_device_free_gpio:
	 * "f940ce68  ldr x8, [x19,#408]".
	 */
	struct spi_device *spi;
};

int sf_platform_init(struct sf_ctl_device *ctl);
void sf_platform_exit(struct sf_ctl_device *ctl);

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a100ce0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_fingerprint_sunwave_sunwave.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
extern int g_probe_done;

#endif /* __SUNWAVE_H */
