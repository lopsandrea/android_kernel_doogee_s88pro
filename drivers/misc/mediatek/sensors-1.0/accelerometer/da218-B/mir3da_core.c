// SPDX-License-Identifier: GPL-2.0
/*
 * MiraMEMS mir3da (DA218-B) accelerometer, Doogee S88 Pro -- the "core"
 * translation unit.
 *
 * Reconstructed from the disassembly of the factory kernel. Not a line comes
 * from another phone: the mir3da source exists neither in the ALPS tree nor
 * in the .186 tree of the other project.
 *
 * A note on the file name: "mir3da_core.c" is the convention mir3da_cust.c
 * and mir3da_core.h use for this second translation unit, not a measurement
 * -- the string "mir3da_core" does not appear in the binary.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#include <linux/kernel.h>
#include <linux/types.h>

#include "mir3da_core.h"

/* ------------------------------------------------------------------ */
/* Log                                                                  */
/* ------------------------------------------------------------------ */

/*
 * "b94d9d08 ldr w8, [x8,#3484]" and the `ldrb`s of the tests give the same
 * variable as mir3da_cust.c: 0xffffff800992ed98.  Defined here, declared
 * `extern` in mir3da_core.h — see the comment up there for the measurement.
 */
int Log_level = MIR_ERR;

/*
 * MI_ERR() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define MI_ERR(fmt, ...)						\
	do {								\
		if (Log_level & MIR_ERR)				\
			p_mir3da_general_ops->myprintk("[MIR3DA] " fmt,	\
						       ##__VA_ARGS__);	\
	} while (0)

#define MI_MSG(fmt, ...)						\
	do {								\
		if (Log_level & MIR_MSG)				\
			p_mir3da_general_ops->myprintk("[MIR3DA] " fmt,	\
						       ##__VA_ARGS__);	\
	} while (0)

#define MI_DATA(fmt, ...)						\
	do {								\
		if (Log_level & MIR_DATA)				\
			p_mir3da_general_ops->myprintk("[MIR3DA] " fmt,	\
						       ##__VA_ARGS__);	\
	} while (0)

/* ------------------------------------------------------------------ */
/* The data model */
/* ------------------------------------------------------------------ */

/*
 * { short addr; u8 mask; u8 value; } — stride 4, measured on mir3da_set_odr
 * (see the header, §2).  `addr` is signed because -1 terminates the
 * arrays: "79c03575 ldrsh w21, [x11,#26]" followed by
 * "37f80395 tbnz w21, #31, ...".
 */
struct mir_reg_obj {
	short		addr;
	unsigned char	mask;
	unsigned char	value;
};

/*
 * The entry in the chip table: 184 bytes, measured on the multiplier that
 * every core function uses to index it —
 *   "5280170a mov w10, #0xb8"  (184)
 *   "9b0a2128 madd x8, x9, x10, x8"
 *
 * The field offsets are not apportioned by eye: each is the number that
 * appears in an access in the disassembly.
 */
struct mir_chip_ops {
	/* +0   the name, first argument of the "%s" in mir3da_core_init */
	char			asic[8];
	/* +8   two bytes of zero, no reader (see §2 and §4b) */
	unsigned char		sconosciuto_8[2];
	/* +10  "3976c921 ldrb w1, [x9,#3506]" (= 0x992edb2, base+10) */
	struct mir_reg_obj	chip_id;
	/* +14  "3976d921 ldrb w1, [x9,#3510]" (= 0x992edb6, base+14) */
	struct mir_reg_obj	mod_id;
	/* +18  "79402515 ldrh w21, [x8,#18]" in mir3da_chip_resume */
	struct mir_reg_obj	soft_reset;
	/* +22  "39405901 ldrb w1, [x8,#22]" in mir3da_get_enable */
	struct mir_reg_obj	power_mode;
	/*
	 * +26  "79c03575 ldrsh w21, [x11,#26]" with
	 *      "8b28096b add x11, x11, w8, uxtb #2" and the loop ending at
	 *      "7100293f cmp w9, #0xa" / "54fffc09 b.ls ..." — eleven entries,
	 *      the last two at -1.
	 */
	struct mir_reg_obj	init_regs[11];
	/*
	 * +70  mir3da_read_offset unrolls all nine:
	 *      "79c08d01 ldrsh w1, [x8,#70]" ... "79c0cd01 ldrsh w1, [x8,#102]"
	 */
	struct mir_reg_obj	offset_regs[9];
	/* +106 "7940d533 ldrh w19, [x9,#106]" in mir3da_set_odr, three entries */
	struct mir_reg_obj	odr_regs[3];
	/*
	 * +118 "3941d921 ldrb w1, [x9,#118]" in mir3da_read_raw_data, which
	 *      reads six in a row ("321f07e2 orr w2, wzr, #0x6").
	 */
	struct mir_reg_obj	data_regs[6];
	/* +142 "3942392a ldrb w10, [x9,#142]" — shift of the high byte */
	unsigned char		data_msb_shift;
	/* +143 "39423d29 ldrb w9, [x9,#143]" — used as "8 - this" */
	unsigned char		data_shift_base;
	/* +144 eight bytes of zero with no relocation, no reader (§4b) */
	unsigned char		sconosciuto_144[8];
	/* +152 and +160: signature not observable (§4c) */
	void			*calibrate;
	void			*auto_calibrate;
	/* +168 "f9405508 ldr x8, [x8,#168]" in mir3da_interrupt_ops */
	int			(*interrupt_ops)(PLAT_HANDLE handle, void *ops);
	/* +176 "f9405908 ldr x8, [x8,#176]" in mir3da_get_reg_data */
	int			(*get_reg_data)(PLAT_HANDLE handle, char *buf);
};

/*
 * The `struct` that NSA_interrupt_ops receives in x1.  No caller in the
 * factory kernel, so the field names do not exist: they are the offsets.
 * See §4d for the ambiguity of the field at +8.
 */
struct mir3da_int_ops_s {
	int	op;		/* +0  "b9400028 ldr w8, [x1]", 0..3 */
	int	field_4;	/* +4  "b9400688 ldr w8, [x20,#4]" */
	union {
		int		field_8;	/* "b9400a88 ldr w8, [x20,#8]" */
		unsigned char	campo_8b[4];	/* "39402296 ldrb w22, [x20,#8]" */
	};
	unsigned char	field_12;	/* +12 "39403282 ldrb w2, [x20,#12]" */
	unsigned char	field_13;	/* +13 "39403696 ldrb w22, [x20,#13]" */
	unsigned char	field_14;	/* +14 never read */
	unsigned char	field_15;	/* +15 "39403e97 ldrb w23, [x20,#15]" */
};

/* chip_info, .bss 0xffffff8009cba1a8, 16 byte */
struct mir3da_chip_info {
	/* +0  "3906a128 strb w8, [x9,#424]" — the register 0xC0 raw */
	unsigned char	reg_c0;
	/* +4  "b900012b str w11, [x9]" with x9 = 0x9cba1ac. No reader. */
	int		field_4;
	/* +8  "b941b108 ldr w8, [x8,#432]" — it is 2, 3 or 4 */
	int		field_8;
	/* +12 "b901b509 str w9, [x8,#436]" — ranges from 2 to 6 */
	int		field_12;
};

/* ------------------------------------------------------------------ */
/* Prototypes                                                            */
/* ------------------------------------------------------------------ */

int squareRoot(int val);
int mir3da_read_offset(PLAT_HANDLE handle, unsigned char *offset);
int mir3da_temp_calibrate(int *x, int *y, int *z);
int mir3da_module_detect(PLAT_HANDLE handle);
int mir3da_parse_chip_info(PLAT_HANDLE handle);

static int mir3da_read_raw_data(PLAT_HANDLE handle, short *x, short *y,
				short *z);
static int NSA_interrupt_ops(PLAT_HANDLE handle, void *ops);
static int NSA_get_reg_data(PLAT_HANDLE handle, char *buf);
static int NSA_NTO_calibrate(void);
static int NSA_NTO_auto_calibrate(void);

/* ------------------------------------------------------------------ */
/* The factory data                                                     */
/* ------------------------------------------------------------------ */

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800992eda8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct mir_chip_ops mir3da_chip_ops_tbl[] = {
	{
		.asic		= "NSA_NTO",
		.chip_id	= { 0x01, 0xff, 0x13 },
		.mod_id		= { 0x14, 0xff, 0x00 },
		.soft_reset	= { 0x00, 0x24, 0x24 },
		.power_mode	= { 0x11, 0x80, 0x80 },
		.init_regs	= {
			{ 0x0f, 0x03, 0x00 },
			{ 0x11, 0xff, 0x3e },
			{ 0x10, 0xff, 0x07 },
			{ 0x17, 0xff, 0x00 },
			{ 0x1a, 0xff, 0x00 },
			{ 0x7f, 0xff, 0x83 },
			{ 0x7f, 0xff, 0x69 },
			{ 0x7f, 0xff, 0xbd },
			{ 0x20, 0x0f, 0x05 },
			{   -1, 0x00, 0x00 },
			{   -1, 0x00, 0x00 },
		},
		.offset_regs	= {
			{ 0x83, 0xff, 0x00 },
			{ 0x84, 0xff, 0x00 },
			{ 0x85, 0xff, 0x00 },
			{ 0x86, 0xff, 0x00 },
			{ 0x87, 0xff, 0x00 },
			{ 0x88, 0xff, 0x00 },
			{ 0x38, 0xff, 0x00 },
			{ 0x39, 0xff, 0x00 },
			{ 0x3a, 0xff, 0x00 },
		},
		.odr_regs	= {
			{ 0x10, 0x0f, 0x06 },
			{ 0x10, 0x0f, 0x07 },
			{ 0x10, 0x0f, 0x08 },
		},
		.data_regs	= {
			{ 0x02, 0xff, 0x00 },
			{ 0x03, 0xff, 0x00 },
			{ 0x04, 0xff, 0x00 },
			{ 0x05, 0xff, 0x00 },
			{ 0x06, 0xff, 0x00 },
			{ 0x07, 0xff, 0x00 },
		},
		.data_msb_shift	 = 8,
		.data_shift_base = 4,
		.calibrate	 = (void *)NSA_NTO_calibrate,
		.auto_calibrate	 = (void *)NSA_NTO_auto_calibrate,
		.interrupt_ops	 = NSA_interrupt_ops,
		.get_reg_data	 = NSA_get_reg_data,
	},
};

/*
 * .data 0xffffff800992ed9c = ff ff ff ff.  Set to 0 only by
 * mir3da_module_detect ("b90d9d1f str wzr, [x8,#3484]") and compared with
 * -1 by mir3da_parse_chip_info ("3100051f cmn w8, #0x1").
 */
/*
 * mir3da_interrupt_ops() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_chip_index = -1;

/* .data 0xffffff800992eda0 = ff ff ff ff; the stored outcome of the parse. */
static int mir3da_chip_info_res = -1;

/* .bss 0xffffff8009cba198 e 0xffffff8009cba1a0 */
static struct general_op_s *p_mir3da_general_ops;
static struct mir_chip_ops *p_mir3da_chip_ops;

/* .bss 0xffffff8009cba1a8 */
static struct mir3da_chip_info chip_info;

/* .bss 0xffffff8009cba1b8 .. 0xffffff8009cba1cc — the state machine of §3 */
static int		count_static;
static short		last_x;
static short		last_y;
static short		last_z;
static unsigned char	have_last;
static int		z_offset;

/*
 * A compile-time shape check: no code generated, but if one day a field
 * moves the build stops here instead of producing a driver that reads the
 * wrong register.  The offsets are the ones cited one by one above.
 */
static inline void __maybe_unused mir3da_controlla_forma(void)
{
	BUILD_BUG_ON(sizeof(struct mir_reg_obj) != 4);
	BUILD_BUG_ON(sizeof(struct mir_chip_ops) != 184);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, chip_id) != 10);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, mod_id) != 14);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, soft_reset) != 18);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, power_mode) != 22);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, init_regs) != 26);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, offset_regs) != 70);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, odr_regs) != 106);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, data_regs) != 118);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, data_msb_shift) != 142);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, data_shift_base) != 143);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, calibrate) != 152);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, auto_calibrate) != 160);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, interrupt_ops) != 168);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, get_reg_data) != 176);
	BUILD_BUG_ON(offsetof(struct mir3da_chip_info, field_4) != 4);
	BUILD_BUG_ON(offsetof(struct mir3da_chip_info, field_8) != 8);
	BUILD_BUG_ON(offsetof(struct mir3da_chip_info, field_12) != 12);
	BUILD_BUG_ON(offsetof(struct mir3da_int_ops_s, field_15) != 15);
}

/* ------------------------------------------------------------------ */
/* squareRoot @0xffffff800878a0a8, 76 byte                              */
/* ------------------------------------------------------------------ */

/*
 * squareRoot() was reconstructed from the factory kernel disassembly (4 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int squareRoot(int val)
{
	unsigned long i;
	unsigned long b = 0x40000000UL;
	int r = 0;
	int t;

	if (val < 0)
		return 0;

	/*
	 * unroll() was reconstructed from the factory kernel disassembly (444 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
#pragma clang loop unroll(disable)
	for (i = 0; i < 32; i += 2) {
		t = r + (int)(b >> i);
		if (val >= t) {
			r = (int)(b >> i) | (r >> 1);
			val = val - t;
		} else {
			r = r >> 1;
		}
	}

	return r;
}

/* ------------------------------------------------------------------ */
/* mir3da_register_write @0xffffff800878a148, 32 byte — CONFINE         */
/* ------------------------------------------------------------------ */

/*
 * A single forward:
 *   "f940cd08 ldr x8, [x8,#408]"   the general_op_s
 *   "f9400508 ldr x8, [x8,#8]"     the smi2c_write slot
 *   "d63f0100 blr x8"
 * x0/x1/x2 are never touched: the arguments go through as they are.
 */
int mir3da_register_write(PLAT_HANDLE handle, u8 addr, u8 data)
{
	return p_mir3da_general_ops->smi2c_write(handle, addr, data);
}

/* ------------------------------------------------------------------ */
/* mir3da_read_data @0xffffff800878a288, 276 byte — CONFINE             */
/* ------------------------------------------------------------------ */

/*
 * The three scratch integers are cleared first of all —
 *   "f9000bff str xzr, [sp,#16]" (two) and "b9000fff str wzr, [sp,#12]" —
 * and that matters: on the branch that skips the calibration, *z receives
 * that zero.  The condition choosing the branch is literal:
 *   "71012d3f cmp w9, #0x4b" / "7102313f cmp w9, #0x8c" /
 *   "7103293f cmp w9, #0xca" on chip_info.reg_c0, and
 *   "7100153f cmp w9, #0x5" on chip_info.field_12.
 */
int mir3da_read_data(PLAT_HANDLE handle, short *x, short *y, short *z)
{
	int nx = 0, ny = 0, nz = 0;
	int res;

	res = mir3da_read_raw_data(handle, x, y, z);
	if (res) {
		/* "[MIR3DA] mir3da_read_raw_data failed, rst = %d\n"
		 * @0xffffff80091c045e, loaded from
		 * "91117800 add x0, x0, #0x45e"
		 */
		MI_ERR("mir3da_read_raw_data failed, rst = %d\n", res);
		return res;
	}

	if (chip_info.reg_c0 != 0x4B && chip_info.reg_c0 != 0x8C &&
	    chip_info.reg_c0 != 0xCA && chip_info.field_12 != 5) {
		nx = *x;
		ny = *y;
		nz = *z;

		mir3da_temp_calibrate(&nx, &ny, &nz);

		*x = nx;
		*y = ny;
	}

	*z = nz;

	return res;
}

/* ------------------------------------------------------------------ */
/* mir3da_read_raw_data @0xffffff800878a39c, 412 byte (static)         */
/* ------------------------------------------------------------------ */

/*
 * Six bytes in one go — "321f07e2 orr w2, wzr, #0x6" — and success
 * is measured on the COUNT returned, not on zero:
 *   "7100181f cmp w0, #0x6" / "54000701 b.ne ..."
 *
 * The recomposition of the three axes is the same three times:
 *   "1aca216a lsl w10, w11, w10"     high byte << data_msb_shift (8)
 *   "2a0d014a orr w10, w10, w13"     | low byte
 *   "4b0901c9 sub w9, w14, w9"       8 - data_shift_base (4)
 *   "13003d4a sxth w10, w10"         sign extension to 16 bits
 *   "1ac92949 asr w9, w10, w9"       arithmetic shift
 */
static int mir3da_read_raw_data(PLAT_HANDLE handle, short *x, short *y,
				short *z)
{
	unsigned char buf[6] = { 0 };
	int res;

	res = p_mir3da_general_ops->smi2c_read_block(handle,
			p_mir3da_chip_ops[mir3da_chip_index].data_regs[0].addr,
			6, buf);
	if (res != 6) {
		/* "[MIR3DA] i2c block read failed\n\n" @0xffffff80091c058d */
		MI_ERR("i2c block read failed\n\n");
		return -1;
	}

	/*
	 * This section was reconstructed from the factory kernel disassembly (404 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	*x = (short)((short)((buf[1] << p_mir3da_chip_ops[mir3da_chip_index].data_msb_shift) | buf[0]) >>
		     (8 - p_mir3da_chip_ops[mir3da_chip_index].data_shift_base));
	*y = (short)((short)((buf[3] << p_mir3da_chip_ops[mir3da_chip_index].data_msb_shift) | buf[2]) >>
		     (8 - p_mir3da_chip_ops[mir3da_chip_index].data_shift_base));
	*z = (short)((short)((buf[5] << p_mir3da_chip_ops[mir3da_chip_index].data_msb_shift) | buf[4]) >>
		     (8 - p_mir3da_chip_ops[mir3da_chip_index].data_shift_base));

	/* "[MIR3DA] mir3da_raw: x=%d, y=%d, z=%d\n" @0xffffff80091c062b */
	MI_DATA("mir3da_raw: x=%d, y=%d, z=%d\n", *x, *y, *z);

	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_temp_calibrate @0xffffff800878a538, 1656 byte                 */
/* ------------------------------------------------------------------ */

/*
 * mir3da_temp_calibrate() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_temp_calibrate(int *x, int *y, int *z)
{
	int delta_x, delta_y, delta_z, delta_sum;
	int old_x, old_y, old_z;
	int is_static = 0;
	int tmp;
	/*
	 * "b9800057 ldrsw x23, [x2]" before anything else, then
	 * "9361fd08 asr x8, x8, #33" (/5) and
	 * "0b080918 add w24, w8, w8, lsl #2" (*5) and
	 * "4b1802f7 sub w23, w23, w24": the remainder of the division by 5.
	 */
	int rem = *z % 5;

	/* "[MIR3DA] start mir3da_temp_calibrate\n" @0xffffff80091c05d5 */
	MI_MSG("start mir3da_temp_calibrate\n");

	/* "71031d5f cmp w10, #0xc7" / "5400024c b.gt ..." — 199, that is < 200 */
	if (abs(*x) < 200 && abs(*y) < 200) {
		/* "7100059f cmp w12, #0x1" su have_last */
		if (have_last != 1) {
			last_x = (short)*x;
			last_y = (short)*y;
			last_z = (short)*z;
			have_last = 1;
		}

		old_x = last_x;
		old_y = last_y;
		old_z = last_z;

		last_x = (short)*x;
		last_y = (short)*y;
		last_z = (short)*z;

		delta_x = abs((short)*x - old_x);
		delta_y = abs((short)*y - old_y);
		delta_z = abs((short)*z - old_z);
		delta_sum = delta_x + delta_y + delta_z;

		/* "7100f03f cmp w1, #0x3c" / "1a8ba7e2 csinc w2, wzr, w11, ge" */
		if (delta_sum >= 60)
			count_static = 0;
		else
			count_static = count_static + 1;

		/* "[MIR3DA] delta_sum=%d count_static=%d\n" @0xffffff80091c05ae */
		MI_MSG("delta_sum=%d count_static=%d\n", delta_sum,
		       count_static);

		/* "7100285f cmp w2, #0xa" */
		if (count_static >= 10) {
			count_static = 10;
			is_static = 1;
		}

		/*
		 * "320c03eb orr w11, wzr, #0x100000" = 1024*1024, then two
		 * "msub" e squareRoot inlined.
		 */
		tmp = squareRoot(1024 * 1024 - (*x) * (*x) - (*y) * (*y)) + rem;

		if (is_static) {
			if (z_offset == 0) {
				/* "5a89a529 cneg w9, w9, lt" then
				 * "4b080121 sub w1, w9, w8"
				 */
				z_offset = (*z < 0 ? -tmp : tmp) - *z;
			} else if (abs(abs(*z + z_offset) - 1024) >= 131) {
				/* "71020d7f cmp w11, #0x83" */
				*z = (*z < 0 ? -1 : 1) * tmp;
				z_offset = 0;
			} else {
				*z = (*z < 0 ? -1 : 1) * tmp - z_offset;
			}
		} else if (z_offset == 0) {
			*z = (*z < 0 ? -1 : 1) * tmp;
		}

		*x = *x * 130 / 200;
		*y = *y * 130 / 200;
	} else if (abs(abs(*x) - 1024) < 200 && z_offset != 0 &&
		   abs(*y) < 200) {
		/*
		 * "7110055f cmp w10, #0x401" separates the two cases on abs(*x),
		 * "7100051f cmp w8, #0x1" separates the sign of *x.
		 */
		if (abs(*x) > 1024) {
			if (*x > 0)
				*x = *x - (abs(*x) - 1024) * 70 / 200;
			else
				*x = *x + (abs(*x) - 1024) * 70 / 200;
		} else {
			if (*x > 0)
				*x = *x + (1024 - abs(*x)) * 70 / 200;
			else
				*x = *x - (1024 - abs(*x)) * 70 / 200;
		}
		*y = *y * 130 / 200;
	} else if (abs(*x) < 200 && z_offset != 0 &&
		   abs(abs(*y) - 1024) < 200) {
		if (abs(*y) > 1024) {
			if (*y > 0)
				*y = *y - (abs(*y) - 1024) * 70 / 200;
			else
				*y = *y + (abs(*y) - 1024) * 70 / 200;
		} else {
			if (*y > 0)
				*y = *y + (1024 - abs(*y)) * 70 / 200;
			else
				*y = *y - (1024 - abs(*y)) * 70 / 200;
		}
		*x = *x * 130 / 200;
	} else if (z_offset == 0) {
		if (abs(*x) < 200 && abs(abs(*y) - 1024) < 200)
			*z = (*z < 0 ? -1 : 1) * abs(*x) * 130 / 200;
		else if (abs(*y) < 200 && abs(abs(*x) - 1024) < 200)
			*z = (*z < 0 ? -1 : 1) * abs(*y) * 130 / 200;
		else
			*z = (*z < 0 ? -1 : 1) *
			     (squareRoot(1024 * 1024 - (*x) * (*x) -
					 (*y) * (*y)) + rem);
	}

	/* "[MIR3DA] end mir3da_temp_calibrate z_offset=%d\n" @0xffffff80091c05fb */
	MI_MSG("end mir3da_temp_calibrate z_offset=%d\n", z_offset);

	/* "35ffdc01 cbnz w1, ..." / "12800000 mov w0, #0xffffffff" */
	if (z_offset == 0)
		return -1;

	*z = *z + z_offset;

	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_read_offset @0xffffff800878acd0, 492 byte                     */
/* ------------------------------------------------------------------ */

/*
 * In the binary the loop is fully unrolled — nine copies, with offsets
 * 70, 74, 78, 82, 86, 90, 94, 98, 102 and the nine "add x2, x19, #N" from 0 to 8.
 * The early exit on a negative `addr` returns 0 explicitly
 * ("2a1f03e0 mov w0, wzr" at 0xffffff800878aea4).
 */
/*
 * mir3da_read_offset() was reconstructed from the factory kernel disassembly (0xffffff800878ad24, 64 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_read_offset(PLAT_HANDLE handle, unsigned char *offset)
{
	int res = 0;
	int i;

	for (i = 0; i < 9; i++) {
		if (p_mir3da_chip_ops[mir3da_chip_index].offset_regs[i].addr < 0)
			return 0;

		res = p_mir3da_general_ops->smi2c_read(handle,
			p_mir3da_chip_ops[mir3da_chip_index].offset_regs[i].addr,
			&offset[i]);
		if (res)
			return res;
	}

	return res;
}

/* ------------------------------------------------------------------ */
/* mir3da_get_enable @0xffffff800878af84, 164 byte — CONFINE            */
/* ------------------------------------------------------------------ */

/*
 * "390013ff strb wzr, [sp,#4]" clears the byte before the read, and
 * the outcome is the complement of the bit:
 *   "6a0a011f tst w8, w10" / "1a9f17e8 cset w8, eq"
 */
int mir3da_get_enable(PLAT_HANDLE handle, u8 *enable)
{
	struct mir_chip_ops *chip = &p_mir3da_chip_ops[mir3da_chip_index];
	unsigned char reg_data = 0;
	int res;

	res = p_mir3da_general_ops->smi2c_read(handle, chip->power_mode.addr,
					       &reg_data);
	if (res)
		return res;

	*enable = ((chip->power_mode.mask & reg_data) == 0);

	return res;
}

/* ------------------------------------------------------------------ */
/* mir3da_set_enable @0xffffff800878b028, 220 byte — CONFINE            */
/* ------------------------------------------------------------------ */

/*
 * "72001c3f tst w1, #0xff" / "54000060 b.eq ...": it is the POWER-DOWN
 * that writes the table value (0x80, the suspend bit), not the power-up.
 * The scratch byte is NOT cleared before the read, unlike in
 * mir3da_get_enable: the `strb wzr` is missing in the binary.
 */
int mir3da_set_enable(PLAT_HANDLE handle, bool enable)
{
	struct mir_chip_ops *chip = &p_mir3da_chip_ops[mir3da_chip_index];
	unsigned char reg_data;
	unsigned char val;
	int res;

	if (enable)
		val = 0;
	else
		val = chip->power_mode.value;

	res = p_mir3da_general_ops->smi2c_read(handle, chip->power_mode.addr,
					       &reg_data);
	if (res)
		return res;

	/* "0a350108 bic w8, w8, w21" / "0a15028a and w10, w20, w21" */
	reg_data = (reg_data & ~chip->power_mode.mask) |
		   (val & chip->power_mode.mask);

	return p_mir3da_general_ops->smi2c_write(handle,
						 chip->power_mode.addr,
						 reg_data);
}

/* ------------------------------------------------------------------ */
/* mir3da_get_reg_data @0xffffff800878b104, 48 byte — CONFINE           */
/* ------------------------------------------------------------------ */

/*
 * Only a forward through the table:
 *   "f9405908 ldr x8, [x8,#176]" / "d63f0100 blr x8"
 */
int mir3da_get_reg_data(PLAT_HANDLE handle, char *buf)
{
	return p_mir3da_chip_ops[mir3da_chip_index].get_reg_data(handle, buf);
}

/* ------------------------------------------------------------------ */
/* mir3da_module_detect @0xffffff800878b204, 260 byte                   */
/* ------------------------------------------------------------------ */

/*
 * mir3da_module_detect() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_module_detect(PLAT_HANDLE handle)
{
	/*
	 * This section was reconstructed from the factory kernel disassembly.
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	unsigned char reg_data;
	unsigned char reg_data2;
	struct mir_chip_ops *chip;
	int res;
	int i;

	for (i = 0; i < ARRAY_SIZE(mir3da_chip_ops_tbl); i++) {
		chip = &mir3da_chip_ops_tbl[i];

		res = p_mir3da_general_ops->smi2c_read(handle,
						       chip->chip_id.addr,
						       &reg_data);
		if (res)
			return res;

		reg_data = reg_data & chip->chip_id.mask;
		if (reg_data != chip->chip_id.value)
			return -1;

		res = p_mir3da_general_ops->smi2c_read(handle,
						       chip->mod_id.addr,
						       &reg_data2);
		if (res)
			return res;

		reg_data2 = reg_data2 & chip->mod_id.mask;
		if (reg_data2 != chip->mod_id.value)
			return -1;

		/* "[MIR3DA] Found Gsensor MIR3DA !\n" @0xffffff80091c048e */
		MI_MSG("Found Gsensor MIR3DA !\n");

		mir3da_chip_index = i;

		return 0;
	}

	return -1;
}

/* ------------------------------------------------------------------ */
/* mir3da_parse_chip_info @0xffffff800878b308, 524 byte                 */
/* ------------------------------------------------------------------ */

/*
 * mir3da_parse_chip_info() was reconstructed from the factory kernel disassembly (0xffffff8008f54170).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_parse_chip_info(PLAT_HANDLE handle)
{
	unsigned char res = 0xFF;
	unsigned char reg_c0 = 0xFF;
	unsigned char reg_c1 = 0xFF;
	unsigned char reg_8f = 0xFF;
	unsigned int sel;
	int t;

	/* "3100051f cmn w8, #0x1" */
	if (mir3da_chip_index == -1)
		return res;

	res = p_mir3da_general_ops->smi2c_read(handle, 0xC0, &reg_c0);
	if (res)
		return res;

	chip_info.reg_c0 = reg_c0;

	/* "7101011f cmp w8, #0x40" / "54000062 b.cs ..." */
	if (reg_c0 < 0x40)
		return -1;

	/* "53067d0b lsr w11, w8, #6" */
	chip_info.field_4 = reg_c0 >> 6;
	chip_info.field_8 = 2;
	chip_info.field_12 = 2;

	/* "53037d08 lsr w8, w8, #3" / "11001d08 add w8, w8, #0x7" /
	 * "12000908 and w8, w8, #0x7" / "7100091f cmp w8, #0x2"
	 */
	t = ((reg_c0 >> 3) + 7) & 7;
	if (t <= 2)
		chip_info.field_8 = t + 2;

	res = p_mir3da_general_ops->smi2c_read(handle, 0xC1, &reg_c1);
	if (res)
		return res;

	if (chip_info.field_8 == 2) {
		res = p_mir3da_general_ops->smi2c_read(handle, 0x8F, &reg_c0);
		if (res)
			return res;

		/* "53067d15 lsr w21, w8, #6" / "331e0135 bfi w21, w9, #2, #1" */
		sel = (reg_c1 >> 6);
		sel = (sel & ~0x4u) | ((reg_c0 & 1u) << 2);
	} else {
		/* "53057d15 lsr w21, w8, #5" */
		sel = reg_c1 >> 5;
	}

	res = p_mir3da_general_ops->smi2c_read(handle, 0x8F, &reg_8f);
	if (res)
		return res;

	/* "710012bf cmp w21, #0x4" / "540004a8 b.hi ..." */
	if (sel > 4)
		return res;

	switch (sel) {
	case 0:
		/* "39c033e8 ldrsb w8, [sp,#12]" / "37f80328 tbnz w8, #31" */
		if ((signed char)reg_8f < 0)
			chip_info.field_12 = 3;
		else
			chip_info.field_12 = 2;
		return 0;
	case 1:
		chip_info.field_12 = 4;
		return 0;
	case 2:
		return res;
	case 3:
		/* "7100113f cmp w9, #0x4" / "7a4b0144 ccmp w10, w11, #0x4, eq"
		 * con w11 = 0x5a, then "1a890529 cinc w9, w9, ne" su 5.
		 */
		if (chip_info.field_8 == 4 && chip_info.reg_c0 == 0x5A)
			chip_info.field_12 = 5;
		else
			chip_info.field_12 = 6;
		return 0;
	case 4:
		chip_info.field_12 = 6;
		return 0;
	}

	return res;
}

/* ------------------------------------------------------------------ */
/* mir3da_install_general_ops @0xffffff800878b514, 32 byte — CONFINE    */
/* ------------------------------------------------------------------ */

/*
 * "b40000c0 cbz x0, ..." / "12800000 mov w0, #0xffffffff" for the NULL,
 * "f900cd28 str x8, [x9,#408]" for the rest.
 */
int mir3da_install_general_ops(struct general_op_s *ops)
{
	if (!ops)
		return -1;

	p_mir3da_general_ops = ops;

	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_core_init @0xffffff800878b534, 416 byte — CONFINE             */
/* ------------------------------------------------------------------ */

/*
 * mir3da_core_init() was reconstructed from the factory kernel disassembly (0xffffff800878b5c8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
PLAT_HANDLE mir3da_core_init(PLAT_HANDLE handle)
{
	int res;

	p_mir3da_chip_ops = mir3da_chip_ops_tbl;

	/* "37f802c8 tbnz w8, #31, ..." — the index is -1 until a probe happens */
	if (mir3da_chip_index < 0) {
		if (mir3da_module_detect(handle)) {
			/* "[MIR3DA] Can't find Mir3da gsensor!!\n"
			 * @0xffffff80091c04af
			 */
			MI_ERR("Can't find Mir3da gsensor!!\n");
			return NULL;
		}
	}

	/*
	 * "[MIR3DA] Probe gsensor module: %s\n" @0xffffff80091c04d5, with
	 * "9b2a5101 smaddl x1, w8, w10, x20" as the second argument: it is the
	 * entry, that is its first field, the name.
	 */
	MI_MSG("Probe gsensor module: %s\n",
	       p_mir3da_chip_ops[mir3da_chip_index].asic);

	res = mir3da_chip_resume(handle);
	if (res) {
		/*
		 * "[MIR3DA] chip resume fail!!\n\n" @0xffffff80091c0420 —
		 * it is the TAIL of the mir3da_cust.c literal at
		 * 0xffffff80091c041e, which the linker merged.
		 */
		MI_ERR("chip resume fail!!\n\n");
		return NULL;
	}

	return handle;
}

/* ------------------------------------------------------------------ */
/* mir3da_chip_resume @0xffffff800878b6d4, 640 byte — CONFINE           */
/* ------------------------------------------------------------------ */

/*
 * The initialisation loop counts to eleven and stops early on a negative
 * `addr`:
 *   "11000748 add w8, w26, #0x1" / "12001d09 and w9, w8, #0xff" /
 *   "7100293f cmp w9, #0xa" / "54fffc09 b.ls ..."
 * that is, the index is 8-bit and the comparison is unsigned.
 */
int mir3da_chip_resume(PLAT_HANDLE handle)
{
	struct mir_chip_ops *chip = &p_mir3da_chip_ops[mir3da_chip_index];
	unsigned char reg_data;
	unsigned char addr;
	int res;
	unsigned char i;

	res = p_mir3da_general_ops->smi2c_read(handle, chip->soft_reset.addr,
					       &reg_data);
	if (res == 0) {
		reg_data = (reg_data & ~chip->soft_reset.mask) |
			   (chip->soft_reset.mask & chip->soft_reset.value);
		res = p_mir3da_general_ops->smi2c_write(handle,
						chip->soft_reset.addr,
						reg_data);
	}

	/* "528000a0 mov w0, #0x5" through the slot msdelay (+80) */
	p_mir3da_general_ops->msdelay(5);

	if (res) {
		/* "[MIR3DA] Do softreset failed !\n" @0xffffff80091c04f8 */
		MI_ERR("Do softreset failed !\n");
		return res;
	}

	for (i = 0; i < 11; i++) {
		if (chip->init_regs[i].addr < 0)
			break;

		res = p_mir3da_general_ops->smi2c_read(handle,
						chip->init_regs[i].addr,
						&reg_data);
		if (res)
			return res;

		reg_data = (reg_data & ~chip->init_regs[i].mask) |
			   (chip->init_regs[i].mask & chip->init_regs[i].value);

		res = p_mir3da_general_ops->smi2c_write(handle,
						chip->init_regs[i].addr,
						reg_data);
		if (res)
			return res;
	}

	/* "52800140 mov w0, #0xa" */
	p_mir3da_general_ops->msdelay(10);

	/* "36f800a8 tbz w8, #31, ..." on the stored outcome */
	if (mir3da_chip_info_res < 0) {
		mir3da_chip_info_res = mir3da_parse_chip_info(handle);
		if (mir3da_chip_info_res < 0) {
			/* "[MIR3DA] Can't parse Mir3da gsensor chipinfo!!\n"
			 * @0xffffff80091c0518
			 */
			MI_ERR("Can't parse Mir3da gsensor chipinfo!!\n");
			return -1;
		}
	}

	if (chip_info.field_8 == 2) {
		/* "f9401908 ldr x8, [x8,#48]" — the slot get_address */
		addr = p_mir3da_general_ops->get_address(handle);

		/* "7101311f cmp w8, #0x4c" / "7100991f cmp w8, #0x26" */
		if (addr == 0x4C || addr == 0x26) {
			res = p_mir3da_general_ops->smi2c_read(handle, 0x8C,
							       &reg_data);
			if (res == 0) {
				/* "12001502 and w2, w8, #0x3f" */
				reg_data = reg_data & 0x3F;
				p_mir3da_general_ops->smi2c_write(handle, 0x8C,
								  reg_data);
			}
		}
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_get_primary_offset @0xffffff800878b954, 1052 byte — CONFINE   */
/* ------------------------------------------------------------------ */

/*
 * Nine bytes of offsets on the frame — "f9000fff str xzr, [sp,#24]" plus
 * "390083ff strb wzr, [sp,#32]" — read, then put back at the end.
 * In between: a softreset, the init registers, a hundred milliseconds
 * ("52800c80 mov w0, #0x64"), twenty samples ("52800297 mov w23, #0x14")
 * averaged over twenty ("9363fd08 asr x8, x8, #35" with the magic 0x66666667).
 *
 * The second loop, the one that rewrites the offsets, starts from 1 and uses
 * `i - 1` as the index ("51000688 sub w8, w20, #0x1"), and its exit
 * condition is "710022bf cmp w21, #0x8" / "54000068 b.hi ...".
 */
int mir3da_get_primary_offset(PLAT_HANDLE handle, int *x, int *y, int *z)
{
	struct mir_chip_ops *chip = &p_mir3da_chip_ops[mir3da_chip_index];
	unsigned char offset[9] = { 0 };
	unsigned char reg_data;
	short sx, sy, sz;
	int res;
	unsigned char i;
	int j;

	res = mir3da_read_offset(handle, offset);
	if (res) {
		/* "[MIR3DA] Read offset failed !\n" @0xffffff80091c0548 */
		MI_ERR("Read offset failed !\n");
		return -1;
	}

	res = p_mir3da_general_ops->smi2c_read(handle, chip->soft_reset.addr,
					       &reg_data);
	if (res == 0) {
		reg_data = (reg_data & ~chip->soft_reset.mask) |
			   (chip->soft_reset.mask & chip->soft_reset.value);
		res = p_mir3da_general_ops->smi2c_write(handle,
						chip->soft_reset.addr,
						reg_data);
	}

	p_mir3da_general_ops->msdelay(5);

	if (res) {
		MI_ERR("Do softreset failed !\n");
		return -1;
	}

	for (i = 0; i < 11; i++) {
		if (chip->init_regs[i].addr < 0)
			break;

		res = p_mir3da_general_ops->smi2c_read(handle,
						chip->init_regs[i].addr,
						&reg_data);
		if (res == 0) {
			reg_data = (reg_data & ~chip->init_regs[i].mask) |
				   (chip->init_regs[i].mask &
				    chip->init_regs[i].value);
			res = p_mir3da_general_ops->smi2c_write(handle,
						chip->init_regs[i].addr,
						reg_data);
		}

		if (res) {
			/*
			 * "[MIR3DA] Write register[0x%x] error!\n"
			 * @0xffffff80091c0567, with the register as the argument:
			 * "79c03521 ldrsh w1, [x9,#26]"
			 */
			MI_ERR("Write register[0x%x] error!\n",
			       chip->init_regs[i].addr);

			for (j = 1; j <= 9; j++) {
				if (chip->offset_regs[j - 1].addr < 0)
					return -1;
				if (p_mir3da_general_ops->smi2c_write(handle,
						chip->offset_regs[j - 1].addr,
						offset[j - 1]))
					return -1;
			}

			return -1;
		}
	}

	p_mir3da_general_ops->msdelay(100);

	*z = 0;
	*y = 0;
	*x = 0;

	for (i = 0; i < 20; i++) {
		sx = 0;
		sy = 0;
		sz = 0;

		mir3da_read_raw_data(handle, &sx, &sy, &sz);

		*x = *x + sx;
		*y = *y + sy;
		*z = *z + sz;

		p_mir3da_general_ops->msdelay(5);
	}

	*x = *x / 20;
	*y = *y / 20;
	*z = *z / 20;

	for (j = 1; j <= 9; j++) {
		if (chip->offset_regs[j - 1].addr < 0)
			break;

		res = p_mir3da_general_ops->smi2c_write(handle,
						chip->offset_regs[j - 1].addr,
						offset[j - 1]);
		if (res)
			break;
	}

	if (chip_info.reg_c0 == 0x4B || chip_info.reg_c0 == 0x8C ||
	    chip_info.reg_c0 == 0xCA || chip_info.field_12 == 5)
		*z = 0;

	return 0;
}

/* ------------------------------------------------------------------ */
/* NSA_NTO_calibrate    @0xffffff800878be88, 8 byte (static)           */
/* NSA_NTO_auto_calibrate @0xffffff800878be90, 8 byte (static)         */
/* ------------------------------------------------------------------ */

/*
 * Two instructions each, identical:
 *   "2a1f03e0 mov w0, wzr"
 *   "d65f03c0 ret"
 * No function in the factory kernel calls them, neither directly nor
 * through slots +152/+160 of the table.  Their signature is not
 * observable: see the header, §4c.  They are not stubs written to make the
 * link succeed — they are the exact reproduction of two bodies that are
 * like this in the factory.
 */
static int NSA_NTO_calibrate(void)
{
	return 0;
}

static int NSA_NTO_auto_calibrate(void)
{
	return 0;
}

/* ------------------------------------------------------------------ */
/* NSA_interrupt_ops @0xffffff800878be98, 1064 byte (static)           */
/* ------------------------------------------------------------------ */

/*
 * NSA_interrupt_ops() was reconstructed from the factory kernel disassembly (0xffffff8008f54175).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int NSA_interrupt_ops(PLAT_HANDLE handle, void *arg)
{
	struct mir3da_int_ops_s *ops = arg;
	unsigned char reg_data;
	unsigned char t;

	switch (ops->op) {
	case 0:
		if (p_mir3da_general_ops->smi2c_read(handle, 0x21,
						     &reg_data) == 0) {
			/* "33000ec2 bfxil w2, w22, #0, #4" */
			reg_data = (reg_data & ~0x0F) |
				   (ops->campo_8b[1] & 0x0F);
			p_mir3da_general_ops->smi2c_write(handle, 0x21,
							  reg_data);
		}

		if (p_mir3da_general_ops->smi2c_read(handle, 0x20, &reg_data))
			return 0;

		/* "2a1406c8 orr w8, w22, w20, lsl #1" /
		 * "2a160908 orr w8, w8, w22, lsl #2" /
		 * "2a140d08 orr w8, w8, w20, lsl #3"
		 */
		t = ops->campo_8b[0] | (ops->field_4 << 1) |
		    (ops->campo_8b[0] << 2) | (ops->field_4 << 3);
		reg_data = (reg_data & ~0x0F) | (t & 0x0F);
		p_mir3da_general_ops->smi2c_write(handle, 0x20, reg_data);
		break;

	case 1:
		if (ops->field_4 == 2) {
			if (p_mir3da_general_ops->smi2c_read(handle, 0x16,
							     &reg_data))
				return 0;
			/* "321c0502 orr w2, w8, #0x30" */
			reg_data = reg_data | 0x30;
			p_mir3da_general_ops->smi2c_write(handle, 0x16,
							  reg_data);
		} else if (ops->field_4 == 1) {
			if (p_mir3da_general_ops->smi2c_read(handle, 0x16,
							     &reg_data))
				return 0;
			/* "32000902 orr w2, w8, #0x7" */
			reg_data = reg_data | 0x07;
			p_mir3da_general_ops->smi2c_write(handle, 0x16,
							  reg_data);
		}
		break;

	case 2:
		if (ops->field_8 == 2) {
			if (p_mir3da_general_ops->smi2c_read(handle, 0x2B,
							     &reg_data) == 0) {
				/* "330012c2 bfxil w2, w22, #0, #5" */
				reg_data = (reg_data & ~0x1F) |
					   (ops->field_12 & 0x1F);
				p_mir3da_general_ops->smi2c_write(handle, 0x2B,
								  reg_data);
			}

			if (p_mir3da_general_ops->smi2c_read(handle, 0x2A,
							     &reg_data) == 0) {
				/* "2a161ae8 orr w8, w23, w22, lsl #6" and the
				 * two masks "12800cea mov w10, #0xffffff98"
				 * e "52800ceb mov w11, #0x67"
				 */
				t = ops->field_15 | (ops->field_13 << 6);
				reg_data = (reg_data & ~0x67) | (t & 0x67);
				p_mir3da_general_ops->smi2c_write(handle, 0x2A,
								  reg_data);
			}

			if (ops->field_4 == 1) {
				if (p_mir3da_general_ops->smi2c_read(handle,
							0x1B, &reg_data))
					return 0;
				reg_data = reg_data | 0x30;
				p_mir3da_general_ops->smi2c_write(handle, 0x1B,
								  reg_data);
			} else if (ops->field_4 == 0) {
				if (p_mir3da_general_ops->smi2c_read(handle,
							0x19, &reg_data))
					return 0;
				reg_data = reg_data | 0x30;
				p_mir3da_general_ops->smi2c_write(handle, 0x19,
								  reg_data);
			}
		} else if (ops->field_8 == 1) {
			p_mir3da_general_ops->smi2c_write(handle, 0x28,
							  ops->field_12);

			if (p_mir3da_general_ops->smi2c_read(handle, 0x27,
							     &reg_data) == 0) {
				/* "330006c2 bfxil w2, w22, #0, #2" */
				reg_data = (reg_data & ~0x03) |
					   (ops->field_13 & 0x03);
				p_mir3da_general_ops->smi2c_write(handle, 0x27,
								  reg_data);
			}

			if (ops->field_4 == 1) {
				if (p_mir3da_general_ops->smi2c_read(handle,
							0x1B, &reg_data))
					return 0;
				/* "321e0102 orr w2, w8, #0x4" */
				reg_data = reg_data | 0x04;
				p_mir3da_general_ops->smi2c_write(handle, 0x1B,
								  reg_data);
			} else if (ops->field_4 == 0) {
				if (p_mir3da_general_ops->smi2c_read(handle,
							0x19, &reg_data))
					return 0;
				reg_data = reg_data | 0x04;
				p_mir3da_general_ops->smi2c_write(handle, 0x19,
								  reg_data);
			}
		}
		break;

	case 3:
		if (ops->field_4 == 2) {
			if (p_mir3da_general_ops->smi2c_read(handle, 0x16,
							     &reg_data))
				return 0;
			/* "121a7502 and w2, w8, #0xffffffcf" */
			reg_data = reg_data & ~0x30;
			p_mir3da_general_ops->smi2c_write(handle, 0x16,
							  reg_data);
		} else if (ops->field_4 == 1) {
			if (p_mir3da_general_ops->smi2c_read(handle, 0x16,
							     &reg_data))
				return 0;
			/* "121d7102 and w2, w8, #0xfffffff8" */
			reg_data = reg_data & ~0x07;
			p_mir3da_general_ops->smi2c_write(handle, 0x16,
							  reg_data);
		}
		break;

	default:
		/* "[MIR3DA] Unsupport operation !\n" @0xffffff80091c0652 */
		MI_ERR("Unsupport operation !\n");
		break;
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* NSA_get_reg_data @0xffffff800878c2c0, 280 byte (static)             */
/* ------------------------------------------------------------------ */

/*
 * A dump of 211 registers, 0x00..0xD2 —
 *   "110006d6 add w22, w22, #0x1" / "71034edf cmp w22, #0xd3"
 * — sixteen per row: "72000edf tst w22, #0xf".  The outcome of the read is
 * NOT checked (no `cbnz` after the `blr`), as in the factory.
 * The three strings: "---------start---------" @0xffffff80091c0672,
 * "\n%02x\t" @0xffffff80091c068a, "%02X " @0xffffff80091b570b,
 * "\n--------end---------\n" @0xffffff80091c0691.
 */
static int NSA_get_reg_data(PLAT_HANDLE handle, char *buf)
{
	unsigned char reg_data;
	int len;
	int n;
	int i;

	len = p_mir3da_general_ops->mysprintf(buf, "---------start---------");

	for (i = 0; i < 0xD3; i++) {
		if ((i & 0xF) == 0)
			len += p_mir3da_general_ops->mysprintf(buf + len,
							       "\n%02x\t", i);

		p_mir3da_general_ops->smi2c_read(handle, i, &reg_data);

		len += p_mir3da_general_ops->mysprintf(buf + len, "%02X ",
						       reg_data);
	}

	n = p_mir3da_general_ops->mysprintf(buf + len,
					    "\n--------end---------\n");

	return n + len;
}

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/* ------------------------------------------------------------------ */
/* mir3da_register_read @0xffffff800878a0f4, 32 byte                    */
/* ------------------------------------------------------------------ */

/*
 * mir3da_register_read() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_register_read(PLAT_HANDLE handle, u8 addr, u8 *data)
{
	return p_mir3da_general_ops->smi2c_read(handle, addr, data);
}

/* ------------------------------------------------------------------ */
/* mir3da_register_read_continuously @0xffffff800878a114, 52 byte       */
/* ------------------------------------------------------------------ */

/*
 * mir3da_register_read_continuously() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_register_read_continuously(PLAT_HANDLE handle, u8 addr, u8 count,
				      u8 *data)
{
	return count != p_mir3da_general_ops->smi2c_read_block(handle, addr,
							       count, data);
}

/* ------------------------------------------------------------------ */
/* mir3da_register_mask_write @0xffffff800878a168, 164 byte             */
/* ------------------------------------------------------------------ */

/*
 * mir3da_register_mask_write() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_register_mask_write(PLAT_HANDLE handle, u8 addr, u8 mask, u8 data)
{
	int res;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff800878a1ac).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	u8 tmp;

	res = p_mir3da_general_ops->smi2c_read(handle, addr, &tmp);
	if (res)
		return res;

	tmp &= ~mask;
	tmp |= data & mask;

	return p_mir3da_general_ops->smi2c_write(handle, addr, tmp);
}

/* ------------------------------------------------------------------ */
/* mir3da_direction_remap @0xffffff800878a20c, 124 byte                 */
/* ------------------------------------------------------------------ */

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008f5417c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct mir3da_direction {
	int sign_x;
	int sign_y;
	int sign_z;
	int swap_xy;
};

static const struct mir3da_direction mir3da_dir_tbl[8] = {
	{ 0, 0, 0, 0 },
	{ 0, 1, 0, 1 },
	{ 1, 1, 0, 0 },
	{ 1, 0, 0, 1 },
	{ 1, 0, 1, 0 },
	{ 0, 0, 1, 1 },
	{ 0, 1, 1, 0 },
	{ 1, 1, 1, 1 },
};

int mir3da_direction_remap(short *x, short *y, short *z, int direction)
{
	const struct mir3da_direction *d = &mir3da_dir_tbl[direction];
	short tmp;

	/* "79400008 ldrh w8, [x0]" ... "79000008 strh w8, [x0]" */
	*x = *x - 2 * (*x * d->sign_x);
	/* "79400028 ldrh w8, [x1]" ... "79000028 strh w8, [x1]" */
	*y = *y - 2 * (*y * d->sign_y);
	/* "7940004a ldrh w10, [x2]" ... "79000049 strh w9, [x2]" */
	*z = *z - 2 * (*z * d->sign_z);

	if (d->swap_xy) {
		/* "79400029 ldrh w9, [x1]" e "7940000a ldrh w10, [x0]" */
		tmp = *x;
		*x = *y;
		*y = tmp;
	}

	return d->sign_z ? -1 : 1;
}

/* ------------------------------------------------------------------ */
/* cycle_read_xyz @0xffffff800878abb0, 288 byte                         */
/* ------------------------------------------------------------------ */

/*
 * cycle_read_xyz() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int cycle_read_xyz(PLAT_HANDLE handle, int *x, int *y, int *z, int count)
{
	short raw_x, raw_y, raw_z;
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff800878abf4).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	unsigned int i;

	*z = 0;
	*y = 0;
	*x = 0;

	for (i = 0; i < count; i++) {
		raw_z = 0;
		raw_y = 0;
		raw_x = 0;

		/* "97fffddf bl <mir3da_read_raw_data>" */
		mir3da_read_raw_data(handle, &raw_x, &raw_y, &raw_z);

		*x += raw_x;
		*y += raw_y;
		*z += raw_z;

		/* "f9402908 ldr x8, [x8,#80]" -- the slot msdelay */
		p_mir3da_general_ops->msdelay(5);
	}

	/* "1ad40d08 sdiv w8, w8, w20", three times */
	*x = *x / count;
	*y = *y / count;
	*z = *z / count;

	/* "2a1f03e0 mov w0, wzr" */
	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_write_offset @0xffffff800878aebc, 144 byte                    */
/* ------------------------------------------------------------------ */

/*
 * mir3da_write_offset() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_write_offset(PLAT_HANDLE handle, u8 *offset)
{
	int i, res;

	for (i = 0; i < 9; i++) {
		/*
		 * This section was reconstructed from the factory kernel disassembly (0xffffff800878aef0, 436 bytes).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		if (p_mir3da_chip_ops[mir3da_chip_index].offset_regs[i].addr < 0)
			break;

		/* "38796a62 ldrb w2, [x19,x25]" -- offset[i] */
		res = p_mir3da_general_ops->smi2c_write(handle,
			p_mir3da_chip_ops[mir3da_chip_index].offset_regs[i].addr,
			offset[i]);
		if (res)
			return res;
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_calibrate @0xffffff800878af4c, 8 byte                         */
/* ------------------------------------------------------------------ */

/*
 * mir3da_calibrate() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_calibrate(PLAT_HANDLE handle, int z_dir)
{
	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_interrupt_ops @0xffffff800878af54, 48 byte                    */
/* ------------------------------------------------------------------ */

/*
 * mir3da_interrupt_ops() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_interrupt_ops(PLAT_HANDLE handle, void *ops)
{
	return p_mir3da_chip_ops[mir3da_chip_index].interrupt_ops(handle, ops);
}

/* ------------------------------------------------------------------ */
/* mir3da_set_odr @0xffffff800878b134, 208 byte                         */
/* ------------------------------------------------------------------ */

/*
 * mir3da_set_odr() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_set_odr(PLAT_HANDLE handle, int delay)
{
	struct mir_reg_obj *odr;
	int idx, res;
	short addr;
	u8 mask, value;
	u8 tmp;

	/*
	 * It is THE INDEX that is chosen, not the pointer: the binary does the two
	 * `cset`/`csel` on an integer and then a SINGLE piece of address
	 * arithmetic ("9b0c2549 madd" plus "8b0b0929 add x9, x9, x11, lsl #2").
	 * Choosing between three distinct pointers makes clang produce three paths.
	 */
	if (delay < 6)
		idx = 2;
	else if (delay < 11)
		idx = 1;
	else
		idx = 0;

	odr = &p_mir3da_chip_ops[mir3da_chip_index].odr_regs[idx];

	/*
	 * This section was reconstructed from the factory kernel disassembly.
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	addr = odr->addr;
	mask = odr->mask;
	value = odr->value;

	res = p_mir3da_general_ops->smi2c_read(handle, addr, &tmp);
	if (res)
		return res;

	tmp &= ~mask;
	tmp |= value & mask;

	return p_mir3da_general_ops->smi2c_write(handle, addr, tmp);
}

/* ------------------------------------------------------------------ */
/* mir3da_temp_calibrate_detect_static @0xffffff800878bd70, 280 byte    */
/* ------------------------------------------------------------------ */

/*
 * mir3da_temp_calibrate_detect_static() was reconstructed from the factory kernel disassembly (0xffffff800878bd9c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int mir3da_temp_calibrate_detect_static(short x, short y, short z)
{
	int delta_sum;

	/* "39472109 ldrb w9, [x8,#456]" */
	if (have_last != 1) {
		/* "79037920 strh w0, [x9,#444]" and the two sisters */
		last_x = x;
		last_y = y;
		last_z = z;
		/* "3907210c strb w12, [x8,#456]" */
		have_last = 1;
	}

	/*
	 * "4b2aa16a sub w10, w11, w10, sxth" plus
	 * "5a8a554a cneg w10, w10, mi", three times, and the two sums
	 * "0b0a0129 add w9, w9, w10" and "0b080121 add w1, w9, w8".
	 */
	delta_sum = abs(x - last_x) + abs(y - last_y) + abs(z - last_z);

	/* "79037960 strh w0, [x11,#444]" and its two sisters: the update comes AFTER */
	last_x = x;
	last_y = y;
	last_z = z;

	/*
	 * "7100f03f cmp w1, #0x3c" plus "1a8aa7e2 csinc w2, wzr, w10, ge":
	 * zero if it has passed the threshold, otherwise one more.
	 */
	if (delta_sum >= 60)
		count_static = 0;
	else
		count_static = count_static + 1;

	/* "[MIR3DA] delta_sum=%d count_static=%d\n"@0xffffff80091c05ae */
	MI_MSG("delta_sum=%d count_static=%d\n", delta_sum, count_static);

	/* "7100285f cmp w2, #0xa" -- the ceiling */
	if (count_static >= 10) {
		count_static = 10;
		return 1;
	}

	return 0;
}
