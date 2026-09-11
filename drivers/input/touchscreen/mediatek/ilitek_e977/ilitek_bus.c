// SPDX-License-Identifier: GPL-2.0
/*
 * Ilitek TDDI touch panel, Doogee S88 Pro -- group B, the I2C bus.
 *
 * Seven functions, [0xffffff8008a55664, 0xffffff8008a55d0c), 1704 bytes.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_bus.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include "ilitek.h"

/*
 * The 240-byte container structure: the `i2c_driver` and, right after it, the
 * pointer to the `hwif`. See the file header.
 */
struct ilitek_i2c_drv {
	struct i2c_driver drv;			/* +0,   232 byte measured */
	struct ilitek_hwif_info *hwif;		/* +232 */
};

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_bus.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
/* +808, "int (*c808)(void)" in ilitek.h. Group E. */
int ilitek_tddi_move_mp_code_flash(void);
/* +816, "int (*c816)(int mode)" in ilitek.h. Group E. */
int ilitek_tddi_move_gesture_code_flash(int mode);
/* +840, "int (*c840)(void)" in ilitek.h. Group E. */
int ilitek_tddi_touch_esd_gesture_flash(void);
/*
 * Group E. "940018b2 bl"@0xffffff8008a55acc: two arguments, the buffer and
 * the length. Of the RETURN value the site decides only that the low byte is
 * used ("2a0003f7 mov"@0xffffff8008a55adc then "3833ca97 strb"@0xffffff8008a55b30):
 * `u8` is the narrowest compatible reading, and not the only one.
 */
u8 ilitek_calc_packet_checksum(void *buf, u32 len);
/*
 * Group E. "940013e8 bl"@0xffffff8008a55b38: five arguments, of which the
 * site fixes only the CLASSES -- x0 and x4 pointers, w1/w2/w3 32-bit
 * integers -- and the return value is unused. The parameter names are
 * positional on purpose: no measurement says what the 8, the len+1 and the 0
 * are.
 */
void ilitek_dump_data(void *data, int a, int b, int c, const char *label);

/*
 * FOUR OF THE SEVEN ARE `static`, AND THE MAP SAYS SO, not a choice:
 * `oracolo/stock.map` marks them lower-case `t` -- `ilitek_i2c_probe`,
 * `ilitek_i2c_remove`, `ilitek_i2c_write`, `ilitek_i2c_read` -- while
 * `core_spi_setup`, `ilitek_tddi_interface_dev_init` and
 * `ilitek_tddi_interface_dev_exit` are `T`.
 */
static int ilitek_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id);
static int ilitek_i2c_remove(struct i2c_client *client);
static int ilitek_i2c_write(void *buf, int len);
static int ilitek_i2c_read(void *buf, int len);

/*
 * The `id_table`, in `.rodata` at 0xffffff8008f7ea58
 * ("9129616b add"@0xffffff8008a556f8 on "b000294b adrp"@0xffffff8008a556e4).
 * The CONTENTS are read from the bytes, not inferred: at that address there
 * is "ILITEK_TDDI"@0xffffff8008f7ea58 followed by zeroes up to +64, that is a
 * `name` of 11 characters, `driver_data` at 0, and the terminating entry all
 * zero (sizeof(struct i2c_device_id) = 32: 20 of name, 4 of padding,
 * 8 of data).
 */
static const struct i2c_device_id ilitek_i2c_id[] = {
	{ "ILITEK_TDDI", 0 },
	{ },
};

/*
 * ===========================================================================
 * 1) core_spi_setup -- 0xffffff8008a55664, 44 bytes
 * factory: line 197
 * ===========================================================================
 * The SPI path is not implemented in the factory build: it prints and
 * returns 0.
 */
int core_spi_setup(void)
{
	/*
	 * "\x013ILITEK: (%s, %d): Not support this interface\n"@0xffffff800923f209,
	 * __func__ = "core_spi_setup"@0xffffff800923f239,
	 * __LINE__ = 197 ("528018a2 mov"@0xffffff8008a5567c).
	 */
#line 197
	ILI_ERR("Not support this interface\n");

	/* "2a1f03e0 mov"@0xffffff8008a55684 -- w0 = wzr, that is zero. */
	return 0;
}

/*
 * ===========================================================================
 * 2) ilitek_tddi_interface_dev_init -- 0xffffff8008a55690, 212 byte
 * factory: lines 306, 311
 * ===========================================================================
 */
int ilitek_tddi_interface_dev_init(struct ilitek_hwif_info *hwif)
{
	struct ilitek_i2c_drv *d;

	/*
	 * kzalloc() was reconstructed from the factory kernel disassembly (0xffffff8008a5569c, 240 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_bus.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	d = kzalloc(sizeof(*d), GFP_KERNEL);
	/* "b4000300 cbz"@0xffffff8008a556bc */
	if (!d) {
		/*
		 * "\x013ILITEK: (%s, %d): faied to allocate i2c_driver\n"@0xffffff800923f248
		 * -- the "faied" typo is the factory's and is reported as it stands.
		 * __func__ = "ilitek_tddi_interface_dev_init"@0xffffff800923f27a,
		 * __LINE__ = 306 ("52802642 mov"@0xffffff8008a5572c).
		 */
#line 306
		ILI_ERR("faied to allocate i2c_driver\n");
		/* "12800160 mov"@0xffffff8008a55734 -- -12 = -ENOMEM */
		return -ENOMEM;
	}

	/*
	 * "39400268 ldrb"@0xffffff8008a556c0, "7100611f cmp"@0xffffff8008a556c4,
	 * "540003a1 b.ne"@0xffffff8008a556c8. 0x18 is I2C.
	 */
	if (hwif->type != 0x18) {
		/*
		 * "\x013ILITEK: (%s, %d): Not I2C dev\n"@0xffffff800923f299,
		 * __LINE__ = 311 ("528026e2 mov"@0xffffff8008a5574c).
		 *
		 * NO `kfree` ON THIS PATH. Between the `b.ne` and the `ret`
		 * the binary executes only adrp/add/mov/bl printk and
		 * "128002a0 mov"@0xffffff8008a55754: the memory just
		 * allocated IS LEAKED in the factory build. It is a real leak, and it is
		 * reproduced -- adding the missing `kfree` here would be
		 * code that is not in the binary.
		 */
#line 311
		ILI_ERR("Not I2C dev\n");
		/* "128002a0 mov"@0xffffff8008a55754 -- -22 = -EINVAL */
		return -EINVAL;
	}

	/* "f9001a60 str"@0xffffff8008a556d0 -- hwif+48 */
	hwif->driver = &d->drv;

	/* "f9400668 ldr"@0xffffff8008a556cc + "f9002008 str"@0xffffff8008a556dc */
	d->drv.driver.name = hwif->name;
	/* "f9400a68 ldr"@0xffffff8008a556e0 + "f9002808 str"@0xffffff8008a556f0 */
	d->drv.driver.owner = hwif->owner;
	/* "f9400e68 ldr"@0xffffff8008a556f4 + "f9003408 str"@0xffffff8008a55708 */
	d->drv.driver.of_match_table = hwif->of_match_table;

	/*
	 * The two function pointers come out of a single `stp`:
	 * "a9012809 stp"@0xffffff8008a55700 onto +16 and +24. The addresses are
	 * "911d9129 add"@0xffffff8008a556e8 -> 0xffffff8008a55764 = ilitek_i2c_probe
	 * and "9127414a add"@0xffffff8008a556ec -> 0xffffff8008a559d0 = ilitek_i2c_remove.
	 */
	d->drv.probe = ilitek_i2c_probe;
	d->drv.remove = ilitek_i2c_remove;

	/* "f9005c0b str"@0xffffff8008a55704 -- +184 */
	d->drv.id_table = ilitek_i2c_id;

	/* "f9007413 str"@0xffffff8008a5570c -- +232, the extra field */
	d->hwif = hwif;

	/*
	 * "aa1f03e0 mov"@0xffffff8008a55710 sets x0 to zero (a NULL `owner`)
	 * and "9400d3a9 bl"@0xffffff8008a55714 calls i2c_register_driver.
	 * THE RETURN VALUE IS THE CALL'S: after the `bl` comes
	 * "14000010 b"@0xffffff8008a55718 towards the epilogue, which does not touch w0.
	 */
	return i2c_register_driver(NULL, &d->drv);
}

/*
 * ===========================================================================
 * 3) ilitek_i2c_probe -- 0xffffff8008a55764, 620 byte
 * factory: lines 207..239
 * ===========================================================================
 */
static int ilitek_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	struct device_driver *drv;

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a55774).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_bus.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	drv = client->dev.driver;

	/*
	 * "\x016ILITEK: (%s, %d): ilitek i2c probe\n"@0xffffff800923f2fd,
	 * __func__ = "ilitek_i2c_probe"@0xffffff800923f323,
	 * __LINE__ = 207 ("528019e2 mov"@0xffffff8008a5578c).
	 */
#line 207
	ILI_INFO("ilitek i2c probe\n");

	/* "b4000313 cbz"@0xffffff8008a55794 */
	if (!client) {
		/*
		 * "\x013ILITEK: (%s, %d): i2c client is NULL\n"@0xffffff800923f334,
		 * __LINE__ = 210 ("52801a42 mov"@0xffffff8008a55804).
		 */
#line 210
		ILI_ERR("i2c client is NULL\n");
		/* "12800240 mov"@0xffffff8008a5580c -- -19 = -ENODEV */
		return -ENODEV;
	}

	/*
	 * "79400668 ldrh"@0xffffff8008a55798 -- client+2 = `client->addr`,
	 * "7101051f cmp"@0xffffff8008a5579c against 0x41 = 65.
	 */
	if (client->addr != 0x41) {
		/*
		 * "52800828 mov"@0xffffff8008a557ac loads 0x41 and
		 * "79000668 strh"@0xffffff8008a557c0 writes it into client+2:
		 * the address is CORRECTED, not merely reported. The store
		 * happens BEFORE the printk.
		 */
		client->addr = 0x41;
		/*
		 * "\x016ILITEK: (%s, %d): i2c addr doesn't be set up, use default : 0x%x\n"@0xffffff800923f35c,
		 * __LINE__ = 216 ("52801b02 mov"@0xffffff8008a557b8),
		 * and the fourth argument is 0x41 again
		 * ("52800823 mov"@0xffffff8008a557bc).
		 */
#line 215
		ILI_INFO("i2c addr doesn't be set up, use default : 0x%x\n",
			 0x41);
	}

	/*
	 * `i2c_check_functionality` is `static inline` and gets inlined:
	 * "f9400e60 ldr"@0xffffff8008a557c8 (client+24 = `adapter`),
	 * "f9400808 ldr"@0xffffff8008a557cc (adapter+16 = `algo`),
	 * "f9400908 ldr"@0xffffff8008a557d0 (algo+16 = `functionality`),
	 * "d63f0100 blr"@0xffffff8008a557d4, and the bit tested is number 0
	 * ("37000240 tbnz"@0xffffff8008a557d8), which is I2C_FUNC_I2C.
	 */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		/*
		 * "\x013ILITEK: (%s, %d): i2c functions are not supported!\n"@0xffffff800923f3a0,
		 * __LINE__ = 220 ("52801b82 mov"@0xffffff8008a557ec).
		 */
#line 220
		ILI_ERR("i2c functions are not supported!\n");
		/* the same epilogue as the previous branch: -19 = -ENODEV */
		return -ENODEV;
	}

	/*
	 * 888 bytes cleared on the client's `device`:
	 * "91008274 add"@0xffffff8008a55820 computes client+0x20 = &client->dev
	 * (32 = offsetof(i2c_client, dev), measured),
	 * "52806f01 mov"@0xffffff8008a55828 is 0x378 = 888 =
	 * sizeof(struct ilitek_tddi_dev), and the gfp in w2 is again
	 * GFP_KERNEL | __GFP_ZERO ("52901802 mov"@0xffffff8008a55824 plus
	 * "72a02802 movk"@0xffffff8008a5582c).
	 * "f90522c0 str"@0xffffff8008a55844 writes it into the `idev` global.
	 */
	idev = devm_kzalloc(&client->dev, sizeof(struct ilitek_tddi_dev),
			    GFP_KERNEL);
	/*
	 * The check is TWO tests and not one, in this order:
	 * "b140041f cmn"@0xffffff8008a55840 with "54000988 b.hi"@0xffffff8008a55848
	 * is `IS_ERR` (a pointer in the last page), and
	 * "b4000963 cbz"@0xffffff8008a5584c is the NULL.
	 */
	if (IS_ERR(idev) || !idev) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to allocate idev memory, %ld\n"@0xffffff800923f3d6,
		 * __LINE__ = 226 ("52801c42 mov"@0xffffff8008a55988).
		 *
		 * THE THIRD ARGUMENT IS NOT PASSED. The format declares a
		 * %ld but between the construction of the arguments and the `bl` there is
		 * no store into w3/x3: the factory prints garbage. The
		 * omission is reproduced -- adding `PTR_ERR(idev)` here
		 * would be an instruction that is not in the binary.
		 */
#line 226
		ILI_ERR("Failed to allocate idev memory, %ld\n");
		/* "12800160 mov"@0xffffff8008a55990 -- -12 = -ENOMEM */
		return -ENOMEM;
	}

	/*
	 * 2048 bytes, GFP_ATOMIC | __GFP_ZERO -- the gfp is 0x01088020
	 * ("52900401 mov"@0xffffff8008a55858 plus
	 * "72a02101 movk"@0xffffff8008a5585c), DIFFERENT from the other three, and the
	 * size 0x800 is "321503e2 orr"@0xffffff8008a55860. The compiler
	 * resolves it to `kmalloc_caches[11]` ("f9435900 ldr"@0xffffff8008a55854,
	 * +1712). "f9008500 str"@0xffffff8008a55870 puts it at idev+264.
	 */
	idev->c264 = kzalloc(0x800, GFP_ATOMIC);
	/* same double test: "b140041f cmn"@0xffffff8008a5586c, "b4000900 cbz"@0xffffff8008a55878 */
	if (IS_ERR(idev->c264) || !idev->c264) {
		/*
		 * "\x013ILITEK: (%s, %d): failed to allocate touch report buffer\n"@0xffffff800923f40f,
		 * __LINE__ = 233 ("52801d22 mov"@0xffffff8008a559a8).
		 */
#line 233
		ILI_ERR("failed to allocate touch report buffer\n");
		/* "12800160 mov"@0xffffff8008a559c8 -- -12 = -ENOMEM */
		return -ENOMEM;
	}

	/*
	 * 56 byte, GFP_KERNEL | __GFP_ZERO: "321d0be2 orr"@0xffffff8008a5588c
	 * (0x38 = 56), `kmalloc_caches[6]` ("f9434500 ldr"@0xffffff8008a55880,
	 * +1672), "f9002100 str"@0xffffff8008a5589c su idev+64.
	 */
	idev->c64 = kzalloc(56, GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a55898, "b4000860 cbz"@0xffffff8008a558a4 */
	if (IS_ERR(idev->c64) || !idev->c64) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to allocate gresture coordinate buffer\n"@0xffffff800923f44b
		 * -- the "gresture" typo is the factory's.
		 * __LINE__ = 239 ("52801de2 mov"@0xffffff8008a559c0).
		 */
#line 239
		ILI_ERR("Failed to allocate gresture coordinate buffer\n");
		/* "12800160 mov"@0xffffff8008a559c8 -- -12 = -ENOMEM */
		return -ENOMEM;
	}

	/* "f9000113 str"@0xffffff8008a558a8 -- idev+0 = client */
	idev->c0 = client;
	/* "f900051f str"@0xffffff8008a558bc -- idev+8 = NULL explicit */
	idev->c8 = NULL;
	/* "f9000d14 str"@0xffffff8008a558c0 -- idev+24 = &client->dev (x20) */
	idev->c24 = &client->dev;
	/*
	 * "b0003aa9 adrp"@0xffffff8008a558b0 plus "91218129 add"@0xffffff8008a558b4
	 * -> 0xffffff80091aa860 = "I2C", written to idev+216 by
	 * "f9006d09 str"@0xffffff8008a558c8.
	 */
	idev->c216 = "I2C";
	/*
	 * "f94056ab ldr"@0xffffff8008a558c4 (the first of the two derivations)
	 * and "f900150b str"@0xffffff8008a558ec -- idev+40 = hwif.
	 */
	idev->c40 = container_of(drv, struct ilitek_i2c_drv, drv.driver)->hwif;

	/*
	 * The two transport pointers:
	 * "9129214a add"@0xffffff8008a558d4 -> 0xffffff8008a55a48 = ilitek_i2c_write,
	 * written at +776 by "f901850a str"@0xffffff8008a558e0;
	 * "91309129 add"@0xffffff8008a558d0 -> 0xffffff8008a55c24 = ilitek_i2c_read,
	 * written at +784 by "f9018909 str"@0xffffff8008a558d8.
	 */
	idev->c776 = ilitek_i2c_write;
	idev->c784 = ilitek_i2c_read;

	/*
	 * "b901490a str"@0xffffff8008a558f4 -- idev+328 = 43
	 * ("5280056a mov"@0xffffff8008a558e4).
	 */
	idev->c328 = 43;

	/*
	 * A single 64-bit `str` fills TWO 32-bit fields:
	 * "9109910b add"@0xffffff8008a558f0 computes idev+0x264 = idev+612 and
	 * "f9000169 str"@0xffffff8008a55904 writes x9 there, built as
	 * 2 ("d2800049 mov"@0xffffff8008a558dc) with 0x64 in the high half
	 * ("f2c00c89 movk"@0xffffff8008a558e8): c612 = 2 and c616 = 100.
	 */
	idev->c612 = 2;
	idev->c616 = 100;

	/* "f901a11f str"@0xffffff8008a558f8 -- idev+832 = NULL */
	idev->c832 = NULL;
	/* "f901911f str"@0xffffff8008a558fc -- idev+800 = NULL */
	idev->c800 = NULL;
	/* "b901411f str"@0xffffff8008a55900 -- idev+320 = 0 */
	idev->c320 = 0;

	/*
	 * "91319129 add"@0xffffff8008a55910 -> 0xffffff8008a5ac64 =
	 * ilitek_tddi_move_mp_code_flash, at +808
	 * ("f9019509 str"@0xffffff8008a55918).
	 */
	idev->c808 = ilitek_tddi_move_mp_code_flash;
	/*
	 * "912a016b add"@0xffffff8008a55914 -> 0xffffff8008a5ba80 =
	 * ilitek_tddi_move_gesture_code_flash, at +816
	 * ("f901990b str"@0xffffff8008a55920).
	 */
	idev->c816 = ilitek_tddi_move_gesture_code_flash;
	/*
	 * "9100c129 add"@0xffffff8008a55928 -> 0xffffff8008a54030 =
	 * ilitek_tddi_wq_esd_i2c_check, at +824
	 * ("f9019d09 str"@0xffffff8008a55934).
	 */
	idev->c824 = ilitek_tddi_wq_esd_i2c_check;
	/*
	 * "9137116b add"@0xffffff8008a5592c -> 0xffffff8008a5bdc4 =
	 * ilitek_tddi_touch_esd_gesture_flash, at +840
	 * ("f901a50b str"@0xffffff8008a5593c).
	 */
	idev->c840 = ilitek_tddi_touch_esd_gesture_flash;

	/* "b901c10a str"@0xffffff8008a55944 -- idev+448 = 1 */
	idev->c448 = 1;
	/* "b9026d1f str"@0xffffff8008a55948 -- idev+620 = 0 */
	idev->c620 = 0;
	/* "f9010d09 str"@0xffffff8008a5594c -- idev+536 = 4 ("321e03e9 orr"@0xffffff8008a55938) */
	idev->c536 = 4;
	/* "3909d11f strb"@0xffffff8008a55950 -- idev+628 = 0 */
	idev->c628 = 0;
	/* "39084d0a strb"@0xffffff8008a55954 -- idev+531 = 1 */
	idev->c531 = 1;
	/* "3908491f strb"@0xffffff8008a55958 -- idev+530 = 0 */
	idev->c530 = 0;
	/* "3908b11f strb"@0xffffff8008a5595c -- idev+556 = 0 */
	idev->c556 = 0;
	/* "b901550b str"@0xffffff8008a55960 -- idev+340 = 2 ("321f03eb orr"@0xffffff8008a55940) */
	idev->c340 = 2;
	/* "3909f10a strb"@0xffffff8008a55964 -- idev+636 = 1 */
	idev->c636 = 1;

	/*
	 * container_of() was reconstructed from the factory kernel disassembly (0xffffff8008a55968).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_bus.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	return container_of(drv, struct ilitek_i2c_drv, drv.driver)->hwif->c32();
}

/*
 * ===========================================================================
 * 4) ilitek_i2c_remove -- 0xffffff8008a559d0, 44 byte
 * factory: line 291
 * ===========================================================================
 */
static int ilitek_i2c_remove(struct i2c_client *client)
{
	/*
	 * The format is "\x016ILITEK: (%s, %d): "@0xffffff800923e290 and it ends
	 * there -- an empty message, see B4 in the file header.
	 * __func__ = "ilitek_i2c_remove"@0xffffff800923f5c1,
	 * __LINE__ = 291 ("52802462 mov"@0xffffff8008a559e8).
	 */
#line 291
	ILI_INFO("");

	/* "2a1f03e0 mov"@0xffffff8008a559f0 */
	return 0;
}

/*
 * ilitek_tddi_interface_dev_exit() was reconstructed from the factory kernel disassembly (0xffffff8008a559fc, 76 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_bus.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_interface_dev_exit(struct ilitek_tddi_dev *dev)
{
	struct i2c_driver *drv;

	/* "f9401408 ldr"@0xffffff8008a55a08 + "f9401913 ldr"@0xffffff8008a55a18 */
	drv = ((struct ilitek_hwif_info *)dev->c40)->driver;

	/*
	 * "\x016ILITEK: (%s, %d): remove i2c dev\n"@0xffffff800923f2ba,
	 * __func__ = "ilitek_tddi_interface_dev_exit"@0xffffff800923f2de,
	 * __LINE__ = 333 ("528029a2 mov"@0xffffff8008a55a20).
	 * The two fields are read BEFORE the printk.
	 */
#line 333
	ILI_INFO("remove i2c dev\n");

	/* "9400d325 bl"@0xffffff8008a55a2c */
	i2c_del_driver(drv);
	/*
	 * "b4000073 cbz"@0xffffff8008a55a30 skips the `kfree` when the pointer
	 * is null: it is the shape `kfree(NULL)` does NOT produce by itself, so
	 * the guard is in the source.
	 */
	if (drv)
		/* "97dfeb5e bl"@0xffffff8008a55a38 */
		kfree(drv);
}

/*
 * core_i2c_write() was reconstructed from the factory kernel disassembly (0xffffff8008a55a48, 476 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_bus.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int core_i2c_write(void *buf, int len)
{
	struct i2c_msg msg;
	u8 *mpbuf = NULL;
	int ret;

	/*
	 * The message: "790013e9 strh"@0xffffff8008a55a94 sets the address
	 * (read from idev->c0->addr, "79400529 ldrh"@0xffffff8008a55a84),
	 * "790017ff strh"@0xffffff8008a55a88 clears the flags (a write),
	 * "79001be1 strh"@0xffffff8008a55a8c sets the length and
	 * "f9000be0 str"@0xffffff8008a55a90 the buffer.
	 */
	msg.addr = ((struct i2c_client *)idev->c0)->addr;
	msg.flags = 0;
	msg.len = len;
	msg.buf = buf;

	/*
	 * The checksum path is taken only if all three tests pass,
	 * in the order the binary puts them:
	 *   "530a7d29 lsr"@0xffffff8008a55aa0 and "7105053f cmp"@0xffffff8008a55aa4
	 *      -- see B3: the first u32 of c56, shifted by 10, against 0x141;
	 *   "394002a9 ldrb"@0xffffff8008a55aac and "7103c53f cmp"@0xffffff8008a55ab0
	 *      -- the first byte of the command against 0xf1;
	 *   "b9414109 ldr"@0xffffff8008a55ab8 and "7100053f cmp"@0xffffff8008a55abc
	 *      -- the TP mode (idev+320) against 1.
	 */
	if ((*(u32 *)idev->c56 >> 10) >= 0x141 &&
	    ((u8 *)buf)[0] == 0xf1 &&
	    idev->c320 == 1) {
		u8 checksum;

		/* "940018b2 bl"@0xffffff8008a55acc -> ilitek_calc_packet_checksum */
		checksum = ilitek_calc_packet_checksum(buf, len);

		/*
		 * `len + 1` ("11000668 add"@0xffffff8008a55ad0), zeroed,
		 * GFP_KERNEL: the size is variable, so the compiler calls
		 * `__kmalloc` and not a `kmem_cache_alloc_trace`
		 * ("97dfea28 bl"@0xffffff8008a55ae8).
		 */
		mpbuf = kzalloc(len + 1, GFP_KERNEL);
		/* "b140041f cmn"@0xffffff8008a55aec, "b4000840 cbz"@0xffffff8008a55af8 */
		if (IS_ERR(mpbuf) || !mpbuf) {
			/*
			 * "\x013ILITEK: (%s, %d): Failed to allocate mpbuf mem\n"@0xffffff800923f4fc,
			 * __func__ = "core_i2c_write"@0xffffff800923f52e,
			 * __LINE__ = 93 ("52800ba2 mov"@0xffffff8008a55c10).
			 */
#line 93
			ILI_ERR("Failed to allocate mpbuf mem\n");
			/* "12800173 mov"@0xffffff8008a55c18 -- -12 = -ENOMEM */
			return -ENOMEM;
		}

		/*
		 * memcpy() was reconstructed from the factory kernel disassembly (0xffffff8008a55b14).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_bus.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		memcpy(mpbuf, buf, min_t(int, msg.len, len));
		/* "3833ca97 strb"@0xffffff8008a55b30 -- mpbuf[len] = checksum */
		mpbuf[len] = checksum;

		/* "f9000bf4 str"@0xffffff8008a55b34 -- msg.buf before the dump */
		msg.buf = mpbuf;
		/*
		 * "940013e8 bl"@0xffffff8008a55b38 towards ilitek_dump_data, with
		 * "mp cdc cmd with checksum"@0xffffff800923f53d in x4,
		 * 8 in w1 ("321d03e1 orr"@0xffffff8008a55b20), len+1 in w2 and
		 * 0 in w3 ("2a1f03e3 mov"@0xffffff8008a55b2c).
		 */
		ilitek_dump_data(mpbuf, 8, len + 1, 0,
				 "mp cdc cmd with checksum");
		/* "79001bf6 strh"@0xffffff8008a55b40 -- msg.len = len + 1 */
		msg.len = len + 1;
	}

	/*
	 * "f9400108 ldr"@0xffffff8008a55b4c (idev+0 = client) e
	 * "f9400d00 ldr"@0xffffff8008a55b58 (client+24 = adapter);
	 * "320003e2 orr"@0xffffff8008a55b54 is the count 1.
	 * "9400d529 bl"@0xffffff8008a55b5c
	 */
	ret = i2c_transfer(((struct i2c_client *)idev->c0)->adapter, &msg, 1);

	/*
	 * "b4000074 cbz"@0xffffff8008a55b64 plus "97dfeb11 bl"@0xffffff8008a55b6c:
	 * the `kfree` is guarded, and it comes AFTER the transfer.
	 */
	if (mpbuf)
		kfree(mpbuf);

	/*
	 * "7100067f cmp"@0xffffff8008a55b70 -- a single message transferred.
	 * The successful branch goes to "2a1f03f3 mov"@0xffffff8008a55b8c (zero),
	 * the other to "12800013 mov"@0xffffff8008a55b78 (minus one).
	 */
	if (ret != 1)
		return -1;

	return 0;
}

static int ilitek_i2c_write(void *buf, int len)
{
	int ret;

	/* "34000941 cbz"@0xffffff8008a55a6c -- the only test is on `len` */
	if (!len) {
		/*
		 * "\x013ILITEK: (%s, %d): i2c write len is invalid\n"@0xffffff800923f48e,
		 * __func__ = "ilitek_i2c_write"@0xffffff800923f4bc,
		 * __LINE__ = 156 ("52801382 mov"@0xffffff8008a55ba4).
		 */
#line 156
		ILI_ERR("i2c write len is invalid\n");
		/* "128002b3 mov"@0xffffff8008a55bac -- -22 = -EINVAL */
		return -EINVAL;
	}

	ret = core_i2c_write(buf, len);

	if (ret < 0) {
		/*
		 * "b942c508 ldr"@0xffffff8008a55b80 reads idev+708 and
		 * "7100051f cmp"@0xffffff8008a55b84 compares it with 1: if the
		 * reset is in progress the error is NOT printed and the function
		 * succeeds ("2a1f03f3 mov"@0xffffff8008a55b8c).
		 */
		if (idev->c708 == 1) {
			ret = 0;
		} else {
			/*
			 * "\x013ILITEK: (%s, %d): i2c write error, ret = %d\n"@0xffffff800923f4cd,
			 * __LINE__ = 166 ("528014c2 mov"@0xffffff8008a55bc4),
			 * and the fourth argument is `ret`
			 * ("2a1303e3 mov"@0xffffff8008a55bc8).
			 */
#line 166
			ILI_ERR("i2c write error, ret = %d\n", ret);
		}
	}

	return ret;
}

/*
 * ===========================================================================
 * 7) ilitek_i2c_read -- 0xffffff8008a55c24, 232 byte
 * factory: lines 178, 188
 * ===========================================================================
 */
static int ilitek_i2c_read(void *buf, int len)
{
	struct i2c_msg msg;
	int ret;

	/* "34000321 cbz"@0xffffff8008a55c40 */
	if (!len) {
		/*
		 * "\x013ILITEK: (%s, %d): i2c read len is invalid\n"@0xffffff800923f556,
		 * __func__ = "ilitek_i2c_read"@0xffffff800923f583,
		 * __LINE__ = 178 ("52801642 mov"@0xffffff8008a55cb4).
		 */
#line 178
		ILI_ERR("i2c read len is invalid\n");
		/* "128002b3 mov"@0xffffff8008a55cbc -- -22 = -EINVAL */
		return -EINVAL;
	}

	/*
	 * Identical to `ilitek_i2c_write` except for the flags:
	 * "790017ea strh"@0xffffff8008a55c60 writes 1
	 * ("320003ea orr"@0xffffff8008a55c4c), that is I2C_M_RD.
	 */
	msg.addr = ((struct i2c_client *)idev->c0)->addr;
	msg.flags = I2C_M_RD;
	msg.len = len;
	msg.buf = buf;

	/* "9400d4e1 bl"@0xffffff8008a55c7c */
	ret = i2c_transfer(((struct i2c_client *)idev->c0)->adapter, &msg, 1);
	/*
	 * "7100041f cmp"@0xffffff8008a55c80 plus
	 * "1a800273 csel"@0xffffff8008a55c84: if the transfer succeeds the
	 * return value is `len`, otherwise `i2c_transfer`'s error.
	 */
	if (ret == 1)
		ret = len;

	/* "36f802d3 tbz"@0xffffff8008a55c88 -- the sign bit alone */
	if (ret < 0) {
		/* "b942c508 ldr"@0xffffff8008a55c90, "7100051f cmp"@0xffffff8008a55c94 */
		if (idev->c708 == 1) {
			/* "2a1f03f3 mov"@0xffffff8008a55c9c */
			ret = 0;
		} else {
			/*
			 * "\x013ILITEK: (%s, %d): i2c read error, ret = %d\n"@0xffffff800923f593,
			 * __LINE__ = 188 ("52801782 mov"@0xffffff8008a55cd4),
			 * fourth argument `ret` ("2a1303e3 mov"@0xffffff8008a55cd8).
			 */
#line 188
			ILI_ERR("i2c read error, ret = %d\n", ret);
		}
	}

	return ret;
}
