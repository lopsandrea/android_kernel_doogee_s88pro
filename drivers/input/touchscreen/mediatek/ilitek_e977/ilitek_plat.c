// SPDX-License-Identifier: GPL-2.0
/*
 * Ilitek TDDI touch panel, Doogee S88 Pro -- group C, the MTK platform glue.
 *
 * Thirteen functions in .text ([0xffffff8008a55d0c, 0xffffff8008a5692c), 3104
 * bytes) plus ilitek_plat_dev_init in .init.text (0xffffff80093833a8, 112
 * bytes).
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

#include "tpd.h"
#include "ilitek.h"

/*
 * ===========================================================================
 * THE FOUR MODULE GLOBALS -- see C3
 * ===========================================================================
 */

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0fc958).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
u8 ilitek_dbg_en;

/*
 * 0xffffff800a0fca48, one byte. It is 1 once the IRQ has been mapped once,
 * and it is there to avoid redoing `of_find_matching_node_and_match` on every
 * registration: read by "39692289 ldrb"@0xffffff8008a56284 and written by
 * "39292289 strb"@0xffffff8008a562e4, both in `ilitek_plat_irq_register`.
 * Nobody names it.
 */
static bool ilitek_irq_mappato;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a0fc954, 560 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int ilitek_probe_riuscito;

/*
 * The `of_match_table` of the `hwif`, read from the bytes at 0xffffff8008f7ea98:
 * `compatible` = "mediatek,cap_touch"@0xffffff8008f7ead8 (offset +64 inside
 * the entry, with `sizeof(struct of_device_id)` = 200), everything else zero,
 * and the second entry all zero.
 */
static const struct of_device_id ilitek_of_match[] = {
	{ .compatible = "mediatek,cap_touch" },
	{ },
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008f7ea98).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static const struct of_device_id ilitek_of_match_irq[] = {
	{ .compatible = "mediatek,touch" },
	{ },
};

/*
 * WHICH ONES ARE `static` IS TOLD BY THE MAP, not a choice: six of the
 * fourteen are `T` (`tp_reset`, `input_register`, the four `irq_*` apart from
 * the ISRs) and eight are `t` -- the two ISRs, the three `tpd_*`,
 * `plat_probe`, `plat_remove` and `plat_dev_init`.
 */
static int ilitek_plat_probe(void);
static int ilitek_plat_remove(void);
static irqreturn_t ilitek_plat_isr_top_half(int irq, void *dev_id);
static irqreturn_t ilitek_plat_isr_bottom_half(int irq, void *dev_id);

/*
 * The object `tpd_local_init` passes to `ilitek_tddi_dev_init`, read entry by
 * entry from the relocations (see the file header). It is NOT built at runtime.
 */
static struct ilitek_hwif_info hwif = {
	.type = 0x18,				/* 0xffffff8009987370, not relocated */
	.name = "ILITEK_TDDI",			/* addend 0xffffff800923f97f */
	.owner = NULL,				/* 0xffffff8009987380, not relocated */
	.of_match_table = ilitek_of_match,	/* addend 0xffffff8008f7ea98 */
	.c32 = ilitek_plat_probe,		/* addend 0xffffff8008a566c4 */
	.c40 = ilitek_plat_remove,		/* addend 0xffffff8008a568fc */
	.driver = NULL,				/* 0xffffff80099873a0, not relocated */
};

/*
 * ===========================================================================
 * 1) ilitek_plat_tp_reset -- 0xffffff8008a55d0c, 168 byte
 * factory: line 36
 * ===========================================================================
 */
void ilitek_plat_tp_reset(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): edge delay = %d\n"@0xffffff800923f5d3,
	 * __func__ = "ilitek_plat_tp_reset"@0xffffff800923f5f8,
	 * __LINE__ = 36 ("52800482 mov"@0xffffff8008a55d34),
	 * argument = idev+616 ("b9426903 ldr"@0xffffff8008a55d2c).
	 */
#line 36
	ILI_INFO("edge delay = %d\n", idev->c616);

	/* "b9415900 ldr"@0xffffff8008a55d44 + "97ffecd9 bl"@0xffffff8008a55d48 */
	tpd_gpio_output(idev->c344, 1);
	/*
	 * "52912b00 mov"@0xffffff8008a55d4c plus
	 * "72a00820 movk"@0xffffff8008a55d50 give 0x418958 = 4.295.000, and
	 * `udelay(n)` compiles into `__const_udelay(n * 0x10c7)` with
	 * 0x10c7 = 4295: 4.295.000 / 4295 = 1000 exact.
	 */
	udelay(1000);

	/* "2a1f03e1 mov"@0xffffff8008a55d5c + "97ffecd2 bl"@0xffffff8008a55d64 */
	tpd_gpio_output(idev->c344, 0);
	/*
	 * "5295d700 mov"@0xffffff8008a55d68 plus
	 * "72a028e0 movk"@0xffffff8008a55d6c give 0x147aeb8 = 21,475,000,
	 * that is 5000 * 4295.
	 */
	udelay(5000);

	/* "320003e1 orr"@0xffffff8008a55d78 + "97ffeccb bl"@0xffffff8008a55d80 */
	tpd_gpio_output(idev->c344, 1);

	/*
	 * mdelay() was reconstructed from the factory kernel disassembly (0xffffff8008a55d88).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	mdelay(idev->c616);
}

/*
 * ===========================================================================
 * 2) ilitek_plat_input_register -- 0xffffff8008a55db4, 828 bytes
 * ===========================================================================
 * No `printk`: this function has not a single factory line to annotate, and
 * does not contribute to divergence C1.
 */
void ilitek_plat_input_register(void)
{
	int i;

	/*
	 * "f9416108 ldr"@0xffffff8008a55dc8 reads the `tpd` global of
	 * `mtk_tpd` (0xffffff800a0fc2c0), "f9400d00 ldr"@0xffffff8008a55dd4
	 * takes its field at +24 -- the `input_dev` -- and
	 * "f9000920 str"@0xffffff8008a55de0 writes it into idev+16. It is EXACTLY
	 * the citation `ilitek.h` carries beside the `c16` field.
	 */
	idev->c16 = tpd->dev;

	/*
	 * The physical keys, if the device tree declares any:
	 * "b9400109 ldr"@0xffffff8008a55de4 is `use_tpd_button`
	 * (0xffffff800a0fbf94 = &tpd_dts_data + 12) and
	 * "b9400508 ldr"@0xffffff8008a55dec is `tpd_key_num` (+16), tested
	 * against 1 with a SIGNED comparison
	 * ("540001cb b.lt"@0xffffff8008a55df4).
	 * The loop indexes `tpd_key_local` (+20) with
	 * "b9401502 ldr"@0xffffff8008a55e08.
	 */
	if (tpd_dts_data.use_tpd_button && tpd_dts_data.tpd_key_num >= 1) {
		for (i = 0; i < tpd_dts_data.tpd_key_num; i++)
			/* "97ffc814 bl"@0xffffff8008a55e10 */
			input_set_capability(idev->c16, EV_KEY,
					     tpd_dts_data.tpd_key_local[i]);
	}

	/*
	 * set_bit() was reconstructed from the factory kernel disassembly (0xffffff8008a55e2c).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	set_bit(EV_ABS, idev->c16->evbit);
	set_bit(EV_SYN, idev->c16->evbit);
	set_bit(EV_KEY, idev->c16->evbit);
	set_bit(BTN_TOUCH, idev->c16->keybit);
	set_bit(BTN_TOOL_FINGER, idev->c16->keybit);
	set_bit(INPUT_PROP_DIRECT, idev->c16->propbit);

	/*
	 * "52800741 mov"@0xffffff8008a55ea0 is 58 = ABS_MT_PRESSURE,
	 * "32001fe3 orr"@0xffffff8008a55ea4 is 255, and min/fuzz/flat are zero
	 * ("2a1f03e2 mov"@0xffffff8008a55ea8, "2a1f03e4 mov"@0xffffff8008a55eb0,
	 * "2a1f03e5 mov"@0xffffff8008a55eb4).
	 */
	input_set_abs_params(idev->c16, ABS_MT_PRESSURE, 0, 255, 0, 0);
	/*
	 * "52800141 mov"@0xffffff8008a55ec0 is 10 slots,
	 * "321f03e2 orr"@0xffffff8008a55ec4 is 2 = INPUT_MT_DIRECT.
	 */
	input_mt_init_slots(idev->c16, 10, INPUT_MT_DIRECT);

	/*
	 * The fourteen key codes, each with its own
	 * "97ffc7xx bl"@... towards `input_set_capability`. The values are the
	 * immediates, in the order the binary materialises them.
	 */
	/* "52800e82 mov"@0xffffff8008a55ed8 -- 116 */
	input_set_capability(idev->c16, EV_KEY, KEY_POWER);
	/* "52800ce2 mov"@0xffffff8008a55eec -- 103 */
	input_set_capability(idev->c16, EV_KEY, KEY_UP);
	/* "52800d82 mov"@0xffffff8008a55f00 -- 108 */
	input_set_capability(idev->c16, EV_KEY, KEY_DOWN);
	/* "52800d22 mov"@0xffffff8008a55f14 -- 105 */
	input_set_capability(idev->c16, EV_KEY, KEY_LEFT);
	/* "52800d42 mov"@0xffffff8008a55f28 -- 106 */
	input_set_capability(idev->c16, EV_KEY, KEY_RIGHT);
	/* "321d07e2 orr"@0xffffff8008a55f3c -- 24 */
	input_set_capability(idev->c16, EV_KEY, KEY_O);
	/* "52800242 mov"@0xffffff8008a55f50 -- 18 */
	input_set_capability(idev->c16, EV_KEY, KEY_E);
	/* "52800642 mov"@0xffffff8008a55f64 -- 50 */
	input_set_capability(idev->c16, EV_KEY, KEY_M);
	/* "52800222 mov"@0xffffff8008a55f78 -- 17 */
	input_set_capability(idev->c16, EV_KEY, KEY_W);
	/* "320013e2 orr"@0xffffff8008a55f8c -- 31 */
	input_set_capability(idev->c16, EV_KEY, KEY_S);
	/* "528005e2 mov"@0xffffff8008a55fa0 -- 47 */
	input_set_capability(idev->c16, EV_KEY, KEY_V);
	/* "52800582 mov"@0xffffff8008a55fb4 -- 44 */
	input_set_capability(idev->c16, EV_KEY, KEY_Z);
	/* "528005c2 mov"@0xffffff8008a55fc8 -- 46 */
	input_set_capability(idev->c16, EV_KEY, KEY_C);
	/* "52800422 mov"@0xffffff8008a55fdc -- 33 */
	input_set_capability(idev->c16, EV_KEY, KEY_F);

	/*
	 * The same fourteen keys, a second time and inline. See C4:
	 * it is redundant and the factory has it. The first five land in
	 * `keybit[1]` (+56), the other nine in `keybit[0]` (+48), and the mask
	 * says which bit: 0x10000000000000 is bit 52, that is key
	 * 64+52 = 116.
	 */
	/* "b24c0129 orr"@0xffffff8008a55ff4 -- bit 52 of keybit[1] = 116 */
	__set_bit(KEY_POWER, idev->c16->keybit);
	/* "b2590129 orr"@0xffffff8008a56008 -- bit 39 = 103 */
	__set_bit(KEY_UP, idev->c16->keybit);
	/* "b2540129 orr"@0xffffff8008a5601c -- bit 44 = 108 */
	__set_bit(KEY_DOWN, idev->c16->keybit);
	/* "b2570129 orr"@0xffffff8008a56030 -- bit 41 = 105 */
	__set_bit(KEY_LEFT, idev->c16->keybit);
	/* "b2560129 orr"@0xffffff8008a56044 -- bit 42 = 106 */
	__set_bit(KEY_RIGHT, idev->c16->keybit);
	/* "b268014a orr"@0xffffff8008a56058 -- bit 24 of keybit[0] */
	__set_bit(KEY_O, idev->c16->keybit);
	/* "b26e014a orr"@0xffffff8008a56068 -- bit 18 */
	__set_bit(KEY_E, idev->c16->keybit);
	/* "b24e014a orr"@0xffffff8008a56078 -- bit 50 */
	__set_bit(KEY_M, idev->c16->keybit);
	/* "b26f014a orr"@0xffffff8008a56088 -- bit 17 */
	__set_bit(KEY_W, idev->c16->keybit);
	/* "b261014a orr"@0xffffff8008a56098 -- bit 31 */
	__set_bit(KEY_S, idev->c16->keybit);
	/* "b251014a orr"@0xffffff8008a560a8 -- bit 47 */
	__set_bit(KEY_V, idev->c16->keybit);
	/* "b254014a orr"@0xffffff8008a560b8 -- bit 44 */
	__set_bit(KEY_Z, idev->c16->keybit);
	/* "b252014a orr"@0xffffff8008a560c8 -- bit 46 */
	__set_bit(KEY_C, idev->c16->keybit);
	/* "b25f0129 orr"@0xffffff8008a560d8 -- bit 33 */
	__set_bit(KEY_F, idev->c16->keybit);
}

/*
 * ===========================================================================
 * 3) ilitek_plat_irq_disable -- 0xffffff8008a560f0, 160 byte
 * factory: lines 226, 232
 * ===========================================================================
 */
void ilitek_plat_irq_disable(void)
{
	unsigned long flag;

	/* "91034100 add"@0xffffff8008a56104 -- idev+208 = &idev->c208 */
	spin_lock_irqsave(&idev->c208, flag);

	/* "b942c109 ldr"@0xffffff8008a56114 + "340002e9 cbz"@0xffffff8008a56118 */
	if (!idev->c704)
		goto fine;

	/* "b9414d00 ldr"@0xffffff8008a5611c + "340001c0 cbz"@0xffffff8008a56120 */
	if (!idev->c332) {
		/*
		 * "\x013ILITEK: (%s, %d): gpio_to_irq (%d) is incorrect\n"@0xffffff800923f60d,
		 * __func__ = "ilitek_plat_irq_disable"@0xffffff800923f640,
		 * __LINE__ = 226 ("52801c42 mov"@0xffffff8008a56168).
		 * The third argument is a fixed ZERO
		 * ("2a1f03e3 mov"@0xffffff8008a5616c): the compiler knows that
		 * on this path `idev->c332` is zero and substitutes it.
		 */
#line 226
		ILI_ERR("gpio_to_irq (%d) is incorrect\n", idev->c332);
		goto fine;
	}

	/* "97db9286 bl"@0xffffff8008a56124 */
	disable_irq_nosync(idev->c332);
	/* "b902c11f str"@0xffffff8008a5612c */
	idev->c704 = 0;
	/*
	 * "\x016ILITEK: (%s, %d): Disable irq success\n"@0xffffff800923f658,
	 * __LINE__ = 232 ("52801d02 mov"@0xffffff8008a5614c).
	 * It is `ILI_DBG`, not `ILI_INFO`: the guard is
	 * "39656108 ldrb"@0xffffff8008a56134 plus
	 * "340001e8 cbz"@0xffffff8008a56138, and it skips the construction of the
	 * arguments, not just the `printk`.
	 */
#line 232
	ILI_DBG("Disable irq success\n");

fine:
	/* "91034100 add"@0xffffff8008a5617c + "94108339 bl"@0xffffff8008a56180 */
	spin_unlock_irqrestore(&idev->c208, flag);
}

/*
 * ===========================================================================
 * 4) ilitek_plat_irq_enable -- 0xffffff8008a56190, 168 byte
 * factory: lines 248, 254
 * ===========================================================================
 */
void ilitek_plat_irq_enable(void)
{
	unsigned long flag;

	/* "91034100 add"@0xffffff8008a561a4 */
	spin_lock_irqsave(&idev->c208, flag);

	/*
	 * "b942c109 ldr"@0xffffff8008a561b4, "7100053f cmp"@0xffffff8008a561b8:
	 * the comparison is with 1, not with zero -- symmetric but not identical to
	 * `irq_disable`'s.
	 */
	if (idev->c704 == 1)
		goto fine;

	/* "b9414d00 ldr"@0xffffff8008a561c0 + "340001e0 cbz"@0xffffff8008a561c4 */
	if (!idev->c332) {
		/*
		 * Same format as `irq_disable`:
		 * "\x013ILITEK: (%s, %d): gpio_to_irq (%d) is incorrect\n"@0xffffff800923f60d,
		 * __func__ = "ilitek_plat_irq_enable"@0xffffff800923f681,
		 * __LINE__ = 248 ("321d13e2 orr"@0xffffff8008a56210).
		 */
#line 248
		ILI_ERR("gpio_to_irq (%d) is incorrect\n", idev->c332);
		goto fine;
	}

	/* "97db92fb bl"@0xffffff8008a561c8 */
	enable_irq(idev->c332);
	/* "b902c109 str"@0xffffff8008a561d4 */
	idev->c704 = 1;
	/*
	 * "\x016ILITEK: (%s, %d): Enable irq success\n"@0xffffff800923f698,
	 * __LINE__ = 254 ("321f1be2 orr"@0xffffff8008a561f4),
	 * guard "39656108 ldrb"@0xffffff8008a561dc.
	 */
#line 254
	ILI_DBG("Enable irq success\n");

fine:
	/* "9410830f bl"@0xffffff8008a56228 */
	spin_unlock_irqrestore(&idev->c208, flag);
}

/*
 * ===========================================================================
 * 5) ilitek_plat_irq_unregister -- 0xffffff8008a56238, 40 byte
 * ===========================================================================
 * No `printk`.
 */
void ilitek_plat_irq_unregister(void)
{
	/*
	 * "f9400d00 ldr"@0xffffff8008a5624c takes idev+24 (the client `device`,
	 * written by group B), "b9414d01 ldr"@0xffffff8008a56250
	 * the IRQ, and "aa1f03e2 mov"@0xffffff8008a56248 sets the third
	 * argument to NULL.
	 */
	devm_free_irq(idev->c24, idev->c332, NULL);
}

/*
 * ===========================================================================
 * 6) ilitek_plat_irq_register -- 0xffffff8008a56260, 324 byte
 * factory: lines 329, 339, 341, 344
 * ===========================================================================
 */
int ilitek_plat_irq_register(int type)
{
	int ret;

	/* "b902c11f str"@0xffffff8008a56280 */
	idev->c704 = 0;

	/*
	 * The IRQ is mapped ONCE ONLY:
	 * "39692289 ldrb"@0xffffff8008a56284 reads the flag and
	 * "37000309 tbnz"@0xffffff8008a56288 skips the whole block if it is already
	 * one.
	 */
	if (!ilitek_irq_mappato) {
		struct device_node *node;

		/*
		 * "91020021 add"@0xffffff8008a56290 -> 0xffffff8008f7e080,
		 * which is the "mediatek,touch" table and NOT the `hwif` one
		 * (0xffffff8008f7ea98, "mediatek,cap_touch"): two different
		 * addresses, two different tables. First and third arguments NULL
		 * ("aa1f03e0 mov"@0xffffff8008a56294,
		 * "aa1f03e2 mov"@0xffffff8008a56298).
		 * "94043cc9 bl"@0xffffff8008a5629c
		 */
		node = of_find_matching_node_and_match(NULL,
						       ilitek_of_match_irq,
						       NULL);
		/* "b40000e0 cbz"@0xffffff8008a562a0 */
		if (node)
			/*
			 * "2a1f03e1 mov"@0xffffff8008a562a4 (index 0) plus
			 * "94045474 bl"@0xffffff8008a562a8, and the result goes
			 * into idev+332 ("b9014d00 str"@0xffffff8008a562b4).
			 */
			idev->c332 = irq_of_parse_and_map(node, 0);

		/*
		 * "\x016ILITEK: (%s, %d): idev->irq_num = %d\n"@0xffffff800923f6c0,
		 * __func__ = "ilitek_plat_irq_register"@0xffffff800923f6e8,
		 * __LINE__ = 329 ("52802922 mov"@0xffffff8008a562d4).
		 * THE FORMAT NAMES THE FIELD: `idev->irq_num` is idev+332.
		 */
#line 329
		ILI_INFO("idev->irq_num = %d\n", idev->c332);
		/* "39292289 strb"@0xffffff8008a562e4 */
		ilitek_irq_mappato = true;
	}

	/*
	 * devm_request_threaded_irq() was reconstructed from the factory kernel disassembly (0xffffff8008a562f0).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	ret = devm_request_threaded_irq(idev->c24, idev->c332,
					ilitek_plat_isr_top_half,
					ilitek_plat_isr_bottom_half,
					type | IRQF_ONESHOT, "ilitek", NULL);

	/*
	 * "7100067f cmp"@0xffffff8008a56318 and "71000a7f cmp"@0xffffff8008a56324:
	 * the type is printed only if it is 1 or 2, and for no other value.
	 */
	if (type == 1)
		/*
		 * "\x016ILITEK: (%s, %d): IRQ TYPE = IRQF_TRIGGER_RISING\n"@0xffffff800923f73d,
		 * __LINE__ = 341 ("52802aa2 mov"@0xffffff8008a56354).
		 */
#line 341
		ILI_INFO("IRQ TYPE = IRQF_TRIGGER_RISING\n");
	else if (type == 2)
		/*
		 * "\x016ILITEK: (%s, %d): IRQ TYPE = IRQF_TRIGGER_FALLING\n"@0xffffff800923f708,
		 * __LINE__ = 339 ("52802a62 mov"@0xffffff8008a5633c).
		 */
#line 339
		ILI_INFO("IRQ TYPE = IRQF_TRIGGER_FALLING\n");

	/* "34000154 cbz"@0xffffff8008a5635c */
	if (ret)
		/*
		 * "\x013ILITEK: (%s, %d): Failed to register irq handler, irq = %d, ret = %d\n"@0xffffff800923f771,
		 * __LINE__ = 344 ("52802b02 mov"@0xffffff8008a56378),
		 * arguments idev+332 ("b9414d03 ldr"@0xffffff8008a56370) e
		 * `ret` ("2a1403e4 mov"@0xffffff8008a5637c).
		 */
		ILI_ERR("Failed to register irq handler, irq = %d, ret = %d\n",
#line 344
			idev->c332, ret);

	/*
	 * "b902c109 str"@0xffffff8008a56390 -- the flag goes to 1 ALWAYS, even
	 * when the registration failed. That is what the binary does.
	 */
	idev->c704 = 1;

	/* "2a1403e0 mov"@0xffffff8008a5638c */
	return ret;
}

/*
 * ===========================================================================
 * 7) ilitek_plat_isr_top_half -- 0xffffff8008a563a4, 384 byte
 * factory: lines 263, 269, 275, 286, 292
 * ===========================================================================
 */
static irqreturn_t ilitek_plat_isr_top_half(int irq, void *dev_id)
{
	/* "b9414d09 ldr"@0xffffff8008a563bc + "6b00013f cmp"@0xffffff8008a563c0 */
	if (irq != idev->c332) {
		/*
		 * "\x013ILITEK: (%s, %d): Incorrect irq number (%d)\n"@0xffffff800923f802,
		 * __func__ = "ilitek_plat_isr_top_half"@0xffffff800923f831,
		 * __LINE__ = 263 ("528020e2 mov"@0xffffff8008a56430).
		 */
#line 263
		ILI_ERR("Incorrect irq number (%d)\n", irq);
		/* "2a1f03f3 mov"@0xffffff8008a56438 -- IRQ_NONE */
		return IRQ_NONE;
	}

	/* "b942dd09 ldr"@0xffffff8008a563c8 + "7100053f cmp"@0xffffff8008a563cc */
	if (idev->c732 == 1) {
		/* "b902dd1f str"@0xffffff8008a563d4 */
		idev->c732 = 0;
		/*
		 * "\x016ILITEK: (%s, %d): interrupt for mp test, ignore\n"@0xffffff800923f84a,
		 * __LINE__ = 269 ("528021a2 mov"@0xffffff8008a563f4),
		 * guard "39656129 ldrb"@0xffffff8008a563dc.
		 */
#line 269
		ILI_DBG("interrupt for mp test, ignore\n");
		/*
		 * "9108e100 add"@0xffffff8008a56400 -- idev+568 = &idev->inq;
		 * "320007e1 orr"@0xffffff8008a56404 is TASK_NORMAL (3),
		 * "320003e2 orr"@0xffffff8008a56408 is 1, the last is NULL:
		 * is `wake_up(&idev->inq)`.
		 */
		wake_up(&idev->inq);
		/* "320003f3 orr"@0xffffff8008a56410 -- IRQ_HANDLED */
		return IRQ_HANDLED;
	}

	/* "3949f509 ldrb"@0xffffff8008a56440 + "340000e9 cbz"@0xffffff8008a56444 */
	if (idev->c637) {
		/*
		 * "\x016ILITEK: (%s, %d): Proximity event, ignore interrupt!\n"@0xffffff800923f87d,
		 * __LINE__ = 275 ("52802262 mov"@0xffffff8008a56458).
		 * It is NOT guarded by the debug byte: the jump at
		 * "14000029 b"@0xffffff8008a5645c goes straight to the `printk`.
		 */
#line 275
		ILI_INFO("Proximity event, ignore interrupt!\n");
		/* "320003f3 orr"@0xffffff8008a56504 -- IRQ_HANDLED */
		return IRQ_HANDLED;
	}

	/*
	 * The format that NAMES SEVEN FIELDS -- see the header.
	 * "\x016ILITEK: (%s, %d): report: %d, rst: %d, fw: %d, switch: %d, mp: %d, sleep: %d, esd: %d\n"@0xffffff800923f8b5,
	 * __LINE__ = 286 ("528023c2 mov"@0xffffff8008a56498),
	 * guard "39656289 ldrb"@0xffffff8008a56464.
	 * The last two arguments go through the stack:
	 * "b9000be8 str"@0xffffff8008a5649c and "b90003e9 str"@0xffffff8008a564a0.
	 */
	ILI_DBG("report: %d, rst: %d, fw: %d, switch: %d, mp: %d, sleep: %d, esd: %d\n",
		idev->c531, idev->c708, idev->c716, idev->c728,
#line 286
		idev->c720, idev->c724, idev->c736);

	/*
	 * Seven tests in a chain, in the exact order of the binary: the first is
	 * "is reporting on?" and the other six "is anything in progress?".
	 *   "39484d09 ldrb"@0xffffff8008a564ac  c531 == 0 -> ignore
	 *   "b942c509 ldr"@0xffffff8008a564b4   c708 != 0 -> ignore
	 *   "b942cd09 ldr"@0xffffff8008a564bc   c716 != 0 -> ignore
	 *   "b942d909 ldr"@0xffffff8008a564c4   c728 != 0 -> ignore
	 *   "b942d109 ldr"@0xffffff8008a564cc   c720 != 0 -> ignore
	 *   "b942d509 ldr"@0xffffff8008a564d4   c724 != 0 -> ignore
	 *   "b942e108 ldr"@0xffffff8008a564dc   c736 != 0 -> ignore
	 */
	if (!idev->c531 || idev->c708 || idev->c716 || idev->c728 ||
	    idev->c720 || idev->c724 || idev->c736) {
		/*
		 * "\x016ILITEK: (%s, %d): ignore interrupt !\n"@0xffffff800923f90e,
		 * __LINE__ = 292 ("52802482 mov"@0xffffff8008a564fc),
		 * guard "39656288 ldrb"@0xffffff8008a564e4.
		 */
#line 292
		ILI_DBG("ignore interrupt !\n");
		/* "320003f3 orr"@0xffffff8008a56504 */
		return IRQ_HANDLED;
	}

	/* "321f03f3 orr"@0xffffff8008a5651c -- IRQ_WAKE_THREAD (2) */
	return IRQ_WAKE_THREAD;
}

/*
 * ===========================================================================
 * 8) ilitek_plat_isr_bottom_half -- 0xffffff8008a56524, 108 byte
 * factory: line 302
 * ===========================================================================
 */
static irqreturn_t ilitek_plat_isr_bottom_half(int irq, void *dev_id)
{
	/*
	 * "f8470c08 ldr"@0xffffff8008a56538 is a PRE-INDEXED `ldr`:
	 * it computes idev+112 (the `touch_mutex`) and in the same move reads its
	 * first field, `owner`. "f100211f cmp"@0xffffff8008a5653c compares it
	 * with 8 UNSIGNED, which is how clang renders `(owner & ~7) != 0` --
	 * that is, `mutex_is_locked()`.
	 */
	if (mutex_is_locked(&idev->touch_mutex)) {
		/*
		 * "\x016ILITEK: (%s, %d): touch is locked, ignore\n"@0xffffff800923f936,
		 * __func__ = "ilitek_plat_isr_bottom_half"@0xffffff800923f963,
		 * __LINE__ = 302 ("528025c2 mov"@0xffffff8008a56578),
		 * guard "39656108 ldrb"@0xffffff8008a56560.
		 */
#line 302
		ILI_DBG("touch is locked, ignore\n");
		/* "320003e0 orr"@0xffffff8008a56584 -- IRQ_HANDLED */
		return IRQ_HANDLED;
	}

	/* "9410738e bl"@0xffffff8008a56544 */
	mutex_lock(&idev->touch_mutex);
	/* "97fff879 bl"@0xffffff8008a56548 */
	ilitek_tddi_report_handler();
	/* "9101c100 add"@0xffffff8008a56550 + "9410739e bl"@0xffffff8008a56554 */
	mutex_unlock(&idev->touch_mutex);

	return IRQ_HANDLED;
}

/*
 * ===========================================================================
 * 9) ilitek_plat_gpio_register -- INLINED into ilitek_plat_probe
 * factory: lines 176..206
 * ===========================================================================
 * See C2: the `__func__` of all its `printk`s is
 * "ilitek_plat_gpio_register"@0xffffff800923fade, not
 * "ilitek_plat_probe"@0xffffff800923fa57.
 */
static int ilitek_plat_gpio_register(void)
{
	int ret = 0;

	/*
	 * "b26003e9 orr"@0xffffff8008a566fc builds 0x100000000 and
	 * "f900ad09 str"@0xffffff8008a56710 writes it at idev+344 with ONE 64-BIT
	 * STORE: it fills two 32-bit fields in one go, c344 = 0 and c348 = 1.
	 */
	idev->c344 = 0;
	idev->c348 = 1;

	/*
	 * "\x016ILITEK: (%s, %d): TP INT: %d\n"@0xffffff800923fabe,
	 * __LINE__ = 176 ("52801602 mov"@0xffffff8008a56704).
	 * The third argument is the CONSTANT 1
	 * ("320003e3 orr"@0xffffff8008a56708): the compiler knows the value of
	 * `idev->c348`, it has just written it.
	 */
#line 176
	ILI_INFO("TP INT: %d\n", idev->c348);
	/*
	 * "\x016ILITEK: (%s, %d): TP RESET: %d\n"@0xffffff800923faf8,
	 * __LINE__ = 177 ("52801622 mov"@0xffffff8008a56724).
	 * Here the value is RE-READ from memory instead
	 * ("b9415903 ldr"@0xffffff8008a56728).
	 */
#line 177
	ILI_INFO("TP RESET: %d\n", idev->c344);

	/*
	 * `gpio_is_valid` with `ARCH_NR_GPIOS` = 512: the comparison is unsigned
	 * against 0x200 ("7108007f cmp"@0xffffff8008a5673c and
	 * "540006e2 b.cs"@0xffffff8008a56740).
	 */
	if (!gpio_is_valid(idev->c348)) {
		/*
		 * "\x013ILITEK: (%s, %d): Invalid INT gpio: %d\n"@0xffffff800923fb1a,
		 * __LINE__ = 180 ("52801682 mov"@0xffffff8008a5682c).
		 */
#line 180
		ILI_ERR("Invalid INT gpio: %d\n", idev->c348);
		return -EBADR;
	}

	/* "7108011f cmp"@0xffffff8008a56748 + "54000762 b.cs"@0xffffff8008a5674c */
	if (!gpio_is_valid(idev->c344)) {
		/*
		 * "\x013ILITEK: (%s, %d): Invalid RESET gpio: %d\n"@0xffffff800923fb44,
		 * __LINE__ = 185 ("52801722 mov"@0xffffff8008a56848).
		 */
#line 185
		ILI_ERR("Invalid RESET gpio: %d\n", idev->c344);
		return -EBADR;
	}

	/*
	 * "912dc021 add"@0xffffff8008a56754 -> "TP_INT"@0xffffff800923fb70,
	 * "97e8875a bl"@0xffffff8008a5675c.
	 */
	ret = gpio_request(idev->c348, "TP_INT");
	/* "36f80220 tbz"@0xffffff8008a56760 -- the sign bit alone */
	if (ret < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): Request IRQ GPIO failed, ret = %d\n"@0xffffff800923fb77,
		 * __LINE__ = 191 ("528017e2 mov"@0xffffff8008a56778).
		 */
#line 191
		ILI_ERR("Request IRQ GPIO failed, ret = %d\n", ret);
		/* "97e88711 bl"@0xffffff8008a56788 */
		gpio_free(idev->c348);
		/* "97e8874a bl"@0xffffff8008a5679c -- it TRIES AGAIN, same name */
		ret = gpio_request(idev->c348, "TP_INT");
		/* "37f805c0 tbnz"@0xffffff8008a567a0 */
		if (ret < 0)
			/*
			 * "\x013ILITEK: (%s, %d): Retrying request INT GPIO still failed , ret = %d\n"@0xffffff800923fbae,
			 * __LINE__ = 195 ("52801862 mov"@0xffffff8008a5686c).
			 */
			ILI_ERR("Retrying request INT GPIO still failed , ret = %d\n",
#line 195
				ret);
	}

	/*
	 * "912fd421 add"@0xffffff8008a567ac -> "TP_RESET"@0xffffff800923fbf5,
	 * "97e88744 bl"@0xffffff8008a567b4.
	 */
	ret = gpio_request(idev->c344, "TP_RESET");
	/* "36f805e0 tbz"@0xffffff8008a567bc */
	if (ret < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): Request RESET GPIO failed, ret = %d\n"@0xffffff800923fbfe,
		 * __LINE__ = 202 ("52801942 mov"@0xffffff8008a567d0).
		 */
#line 202
		ILI_ERR("Request RESET GPIO failed, ret = %d\n", ret);
		/* "97e886fa bl"@0xffffff8008a567e4 */
		gpio_free(idev->c344);
		/* "97e88733 bl"@0xffffff8008a567f8 */
		ret = gpio_request(idev->c344, "TP_RESET");
		/* "36f803c0 tbz"@0xffffff8008a56800 */
		if (ret < 0)
			/*
			 * "\x013ILITEK: (%s, %d): Retrying request RESET GPIO still failed , ret = %d\n"@0xffffff800923fc37,
			 * __LINE__ = 206 ("528019c2 mov"@0xffffff8008a56814).
			 */
			ILI_ERR("Retrying request RESET GPIO still failed , ret = %d\n",
#line 206
				ret);
	}

	/*
	 * "97e8716b bl"@0xffffff8008a56880 towards `gpio_to_desc` followed by
	 * "97e87686 bl"@0xffffff8008a56884 towards `gpiod_direction_input`: that is
	 * `gpio_direction_input`, which the kernel defines that way.
	 * THE RESULT IS NOT USED.
	 */
	gpio_direction_input(idev->c348);

	return ret;
}

/*
 * ===========================================================================
 * 10) ilitek_plat_probe -- 0xffffff8008a566c4, 568 bytes
 * factory: lines 365, 372, 375
 * ===========================================================================
 * It is the `c32` field of the `hwif`, called by `ilitek_i2c_probe` of group
 * B: see the header.
 */
static int ilitek_plat_probe(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): platform probe\n"@0xffffff800923fa33,
	 * __func__ = "ilitek_plat_probe"@0xffffff800923fa57,
	 * __LINE__ = 365 ("52802da2 mov"@0xffffff8008a566e0).
	 */
#line 365
	ILI_INFO("platform probe\n");

	/* "37f80173 tbnz"@0xffffff8008a56888 on the result of the inlined one */
	if (ilitek_plat_gpio_register() < 0)
		/*
		 * "\x013ILITEK: (%s, %d): Register gpio failed\n"@0xffffff800923fa69,
		 * __LINE__ = 372 ("52802e82 mov"@0xffffff8008a568c4).
		 * It does NOT return: it carries on regardless, and that is what the
		 * binary does -- after the `printk` at "97db7303 bl"@0xffffff8008a568c8
		 * comes "97fff95f bl"@0xffffff8008a568cc, the same
		 * `ilitek_tddi_init` as on the successful path.
		 */
#line 372
		ILI_ERR("Register gpio failed\n");

	/* "97fff96f bl"@0xffffff8008a5688c */
	if (ilitek_tddi_init() < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): platform probe failed\n"@0xffffff800923fa93,
		 * __LINE__ = 375 ("52802ee2 mov"@0xffffff8008a568e4).
		 */
#line 375
		ILI_ERR("platform probe failed\n");
		/* "12800240 mov"@0xffffff8008a568ec -- -19 = -ENODEV */
		return -ENODEV;
	}

	/*
	 * "b9415500 ldr"@0xffffff8008a56898 reads idev+340 -- the trigger type,
	 * set to 2 by group B -- and
	 * "97fffe71 bl"@0xffffff8008a5689c registers the IRQ.
	 * THE RESULT IS NOT USED.
	 */
	ilitek_plat_irq_register(idev->c340);

	/* "b9095509 str"@0xffffff8008a568ac */
	ilitek_probe_riuscito = 1;

	/* "2a1f03e0 mov"@0xffffff8008a568a0 */
	return 0;
}

/*
 * ===========================================================================
 * 11) ilitek_plat_remove -- 0xffffff8008a568fc, 48 byte
 * factory: line 385
 * ===========================================================================
 */
static int ilitek_plat_remove(void)
{
	/*
	 * The format is "\x016ILITEK: (%s, %d): "@0xffffff800923e290 and it ends
	 * there -- an EMPTY message, the same literal
	 * `ilitek_i2c_remove` of group B uses.
	 * __func__ = "ilitek_plat_remove"@0xffffff800923fc80,
	 * __LINE__ = 385 ("52803022 mov"@0xffffff8008a56914).
	 */
#line 385
	ILI_INFO("");

	/* "97fffa46 bl"@0xffffff8008a5691c */
	ilitek_tddi_dev_remove();

	/* "2a1f03e0 mov"@0xffffff8008a56920 */
	return 0;
}

/*
 * ===========================================================================
 * 12) tpd_local_init -- 0xffffff8008a56590, 188 bytes
 * factory: lines 407, 410, 414
 * ===========================================================================
 * HOMONYMOUS: three definitions in the map, this is the first. See the header.
 */
static int tpd_local_init(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): TPD init device driver\n"@0xffffff800923f98b,
	 * __func__ = "tpd_local_init"@0xffffff800923f9b7,
	 * __LINE__ = 407 ("528032e2 mov"@0xffffff8008a565a8).
	 */
	ILI_INFO("TPD init device driver\n");

	/*
	 * "910dc000 add"@0xffffff8008a565b4 -> 0xffffff8009987370, the static
	 * `hwif`; "97fffb53 bl"@0xffffff8008a565b8.
	 * "37f80240 tbnz"@0xffffff8008a565bc tests the sign bit alone.
	 */
	if (ilitek_tddi_dev_init(&hwif) < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to register i2c/spi bus driver\n"@0xffffff800923f9c6,
		 * __LINE__ = 410 ("52803342 mov"@0xffffff8008a56614).
		 */
		ILI_ERR("Failed to register i2c/spi bus driver\n");
		/* "12800240 mov"@0xffffff8008a5661c -- -19 = -ENODEV */
		return -ENODEV;
	}

	/* "b9495508 ldr"@0xffffff8008a565c4 + "34000308 cbz"@0xffffff8008a565c8 */
	if (!ilitek_probe_riuscito) {
		/*
		 * "\x013ILITEK: (%s, %d): Add error touch panel driver\n"@0xffffff800923fa01,
		 * __LINE__ = 414 ("528033c2 mov"@0xffffff8008a56638).
		 */
		ILI_ERR("Add error touch panel driver\n");
		/* "12800000 mov"@0xffffff8008a56640 -- minus one, not -ENODEV */
		return -1;
	}

	/* "b94f9508 ldr"@0xffffff8008a565d0 + "340000c8 cbz"@0xffffff8008a565d4 */
	if (tpd_dts_data.use_tpd_button)
		/*
		 * "913e6021 add"@0xffffff8008a565dc computes &tpd_key_num,
		 * "91007022 add"@0xffffff8008a565e0 the third argument, and
		 * "b8404420 ldr"@0xffffff8008a565e4 is a POST-INDEXED `ldr`
		 * that loads `tpd_key_num` and leaves in x1 the address of the
		 * following field, `tpd_key_local`.
		 * "97ffefa3 bl"@0xffffff8008a565e8
		 */
		tpd_button_setting(tpd_dts_data.tpd_key_num,
				   tpd_dts_data.tpd_key_local,
				   tpd_dts_data.tpd_key_dim_local);

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a565f8).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	tpd_load_status = 1;

	/* "2a1f03e0 mov"@0xffffff8008a565f4 */
	return 0;
}

/*
 * ===========================================================================
 * 13) tpd_suspend -- 0xffffff8008a5664c, 60 byte
 * factory: line 360
 * ===========================================================================
 */
static void tpd_suspend(struct device *dev)
{
	/*
	 * "2a1f03e0 mov"@0xffffff8008a56654 -- the argument is ZERO, and
	 * "97fff70c bl"@0xffffff8008a56658 calls
	 * `ilitek_tddi_sleep_handler`. The `dev` argument is NOT used.
	 */
	if (ilitek_tddi_sleep_handler(0) < 0)
		/*
		 * "\x013ILITEK: (%s, %d): TP suspend failed\n"@0xffffff800923fc93,
		 * __func__ = "tpd_suspend"@0xffffff800923fcba,
		 * __LINE__ = 360 ("52802d02 mov"@0xffffff8008a56678).
		 */
		ILI_ERR("TP suspend failed\n");
}

/*
 * tpd_resume() was reconstructed from the factory kernel disassembly (0xffffff8008a56688, 60 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void tpd_resume(struct device *dev)
{
	/*
	 * "321f03e0 orr"@0xffffff8008a56690 -- the argument is 2, and
	 * "97fff6fd bl"@0xffffff8008a56694 calls the same function as
	 * `tpd_suspend`.
	 */
	if (ilitek_tddi_sleep_handler(2) < 0)
		/*
		 * "\x013ILITEK: (%s, %d): TP resume failed\n"@0xffffff800923fcc6,
		 * __func__ = "tpd_resume"@0xffffff800923fcec,
		 * __LINE__ = 354 ("52802c42 mov"@0xffffff8008a566b4).
		 */
		ILI_ERR("TP resume failed\n");
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009987338, 56 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct tpd_driver_t ilitek_device_driver = {
	.tpd_device_name = "ILITEK_TDDI",	/* TO BE CHECKED: not read */
	.tpd_local_init = tpd_local_init,	/* TO BE CHECKED: not read */
	.suspend = tpd_suspend,			/* TO BE CHECKED: not read */
	.resume = tpd_resume,			/* TO BE CHECKED: not read */
};

/*
 * ===========================================================================
 * 15) ilitek_plat_dev_init -- 0xffffff80093833a8, 112 bytes, `.init.text`
 * factory: lines 436, 440
 * ===========================================================================
 * It is the only function of the block that is NOT in `.text`: the map finds
 * it at 0xffffff80093833a8, almost 10 MB further on, and that is why
 * `verificaistruzioni.py` needs the second range.
 */
static int __init ilitek_plat_dev_init(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): ILITEK TP driver init for MTK\n"@0xffffff800923fcf7,
	 * __func__ = "ilitek_plat_dev_init"@0xffffff800923fd2a,
	 * __LINE__ = 436 ("52803682 mov"@0xffffff80093833c0).
	 */
#line 436
	ILI_INFO("ILITEK TP driver init for MTK\n");

	/* "97db35fa bl"@0xffffff80093833c8 -- no arguments */
	tpd_get_dts_info();

	/*
	 * "910ce000 add"@0xffffff80093833d0 -> 0xffffff8009987338 and
	 * "97db37ec bl"@0xffffff80093833d4; "37f80080 tbnz"@0xffffff80093833d8
	 * tests the sign bit alone.
	 */
	if (tpd_driver_add(&ilitek_device_driver) < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): ILITEK add TP driver failed\n"@0xffffff800923fd3f,
		 * __LINE__ = 440 ("52803702 mov"@0xffffff80093833f8).
		 */
#line 440
		ILI_ERR("ILITEK add TP driver failed\n");
		/* "97db3828 bl"@0xffffff8009383408 */
		tpd_driver_remove(&ilitek_device_driver);
		/* "12800240 mov"@0xffffff800938340c -- -19 = -ENODEV */
		return -ENODEV;
	}

	/* "2a1f03e0 mov"@0xffffff80093833dc */
	return 0;
}

/*
 * module_init() was reconstructed from the factory kernel disassembly (0xffffff800941f160).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_plat.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
module_init(ilitek_plat_dev_init);

/*
 * ===========================================================================
 * 16) ilitek_plat_dev_exit -- 0xffffff80093acac4, 52 byte, `.exit.text`
 * factory: line 449
 * ===========================================================================
 */
static void __exit ilitek_plat_dev_exit(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): ilitek driver has been removed\n"@0xffffff800923f7b9,
	 * __func__ = "ilitek_plat_dev_exit"@0xffffff800923f7ed,
	 * __LINE__ = 449 ("52803822 mov"@0xffffff80093acadc).
	 */
	ILI_INFO("ilitek driver has been removed\n");

	/*
	 * "910ce000 add"@0xffffff80093acae8 -> 0xffffff8009987338, the same
	 * `tpd_driver_t` that `ilitek_plat_dev_init` registers.
	 * "97da926f bl"@0xffffff80093acaec
	 */
	tpd_driver_remove(&ilitek_device_driver);
}

module_exit(ilitek_plat_dev_exit);
