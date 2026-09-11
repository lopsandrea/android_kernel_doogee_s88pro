/* SPDX-License-Identifier: GPL-2.0 */
/*
 * MiraMEMS mir3da (DA218-B) -- the core interface, reconstructed from the
 * disassembly of the Doogee S88 Pro factory kernel.
 *
 * BEWARE: this header declares ONLY what mir3da_cust.c actually uses, and
 * every signature below is inferred from the call sites (registers used,
 * width of the loads and stores on the pointers passed), not from a public
 * source.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#ifndef __MIR3DA_CORE_H__
#define __MIR3DA_CORE_H__

#include <linux/types.h>

/*
 * MIR_ERR() was reconstructed from the factory kernel disassembly (0xffffff800992ed98).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define MIR_ERR		(1 << 0)
#define MIR_WARN	(1 << 1)
#define MIR_MSG		(1 << 2)
#define MIR_FUN		(1 << 3)
#define MIR_DATA	(1 << 4)

extern int Log_level;

/*
 * The core takes an opaque "handle". In the factory driver the same
 * pointer is passed sometimes as a `struct i2c_client *` (mir3da_probe
 * calls mir3da_core_init with mir3da_i2c_client) and sometimes as the value
 * returned by mir3da_core_init (mir3da_handle): they are the same object,
 * but the type that unites them in the source is not observable. `void *` is
 * the only choice compatible with both call sites.
 */
typedef void *PLAT_HANDLE;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800992eac0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_core.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct general_op_s {
	int (*smi2c_read)(PLAT_HANDLE handle, u8 addr, u8 *data);
	int (*smi2c_write)(PLAT_HANDLE handle, u8 addr, u8 data);
	int (*smi2c_read_block)(PLAT_HANDLE handle, u8 addr, u8 len, u8 *buf);
	void *slot_24;
	void *slot_32;
	void *slot_40;
	int (*get_address)(PLAT_HANDLE handle);
	void *slot_56;
	int (*myprintk)(const char *fmt, ...);
	int (*mysprintf)(char *buf, const char *fmt, ...);
	void (*msdelay)(int msec);
};

/*
 * The signatures that follow come from the call sites in mir3da_cust.c.
 * Here they are declarations; the definitions live in the other translation
 * unit, mir3da_core.c, reconstructed for 19 of the 29 stock functions.
 */
int mir3da_install_general_ops(struct general_op_s *ops);
PLAT_HANDLE mir3da_core_init(PLAT_HANDLE handle);
int mir3da_chip_resume(PLAT_HANDLE handle);
int mir3da_read_data(PLAT_HANDLE handle, short *x, short *y, short *z);
int mir3da_get_enable(PLAT_HANDLE handle, u8 *enable);
int mir3da_set_enable(PLAT_HANDLE handle, bool enable);
int mir3da_get_reg_data(PLAT_HANDLE handle, char *buf);
int mir3da_register_write(PLAT_HANDLE handle, u8 addr, u8 data);
int mir3da_get_primary_offset(PLAT_HANDLE handle, int *x, int *y, int *z);

/*
 * "1.0.0_2017-06-27-16:31:30_"@0xffffff80091bff95, the second argument of the
 * sprintf in mir3da_version_show ("913e5463 add x3, x3, #0xf95").  In the
 * factory driver this string is wired into the core: here it lives
 * in the header because that is the only place mir3da_cust.c uses it.
 */
#define MIR3DA_CORE_VERSION	"1.0.0_2017-06-27-16:31:30_"

#endif /* __MIR3DA_CORE_H__ */
