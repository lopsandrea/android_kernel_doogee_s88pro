// SPDX-License-Identifier: GPL-2.0
/*
 * MiraMEMS mir3da (DA218-B) accelerometer, Doogee S88 Pro -- the "cust"
 * translation unit.
 *
 * Reconstructed from the disassembly of the factory kernel. Not a line comes
 * from another phone: the mir3da source exists neither in the ALPS tree nor
 * in the .186 tree of the other project. The name and placement of this file
 * are proven, not inferred by adjacency.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include <accel.h>
#include <cust_acc.h>
#include <hwmsen_helper.h>
#include <hwmsensor.h>
#include <sensors_io.h>

#include "mir3da_core.h"

/* ------------------------------------------------------------------ */
/* Log                                                                 */
/* ------------------------------------------------------------------ */

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * MI_ERR() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define MI_ERR(fmt, arg...) \
	do { if (Log_level & MIR_ERR)  printk(KERN_ERR "[MIR3DA] " fmt, ##arg); } while (0)
#define MI_MSG(fmt, arg...) \
	do { if (Log_level & MIR_MSG)  printk(KERN_ERR "[MIR3DA] " fmt, ##arg); } while (0)
#define MI_DATA(fmt, arg...) \
	do { if (Log_level & MIR_DATA) printk(KERN_ERR "[MIR3DA] " fmt, ##arg); } while (0)
/* "%s is called, line: %d\n"@0xffffff80091bf916 */
#define MI_FUN \
	do { if (Log_level & MIR_FUN) \
		printk(KERN_ERR "[MIR3DA] " "%s is called, line: %d\n", __func__, __LINE__); \
	} while (0)

/*
 * "Assertion failed! %s,%d,%s,%s\n"@0xffffff80091bfe85 — this message does
 * NOT carry the "[MIR3DA] " prefix (the cited address already points at the
 * text right after "\x013"), so the macro does not add it.
 */
#define ASSERT(expr) \
	do { if (!(expr)) \
		printk(KERN_ERR "Assertion failed! %s,%d,%s,%s\n", \
		       __FILE__, __LINE__, __func__, #expr); \
	} while (0)

/* ------------------------------------------------------------------ */
/* Private constants and data                                             */
/* ------------------------------------------------------------------ */

/*
 * "mir3da"@0xffffff80091bf966: .driver.name of the i2c_driver at
 * 0xffffff800992ebc0 and .name of the acc_init_info at 0xffffff800992eb60.
 */
#define MIR3DA_DRV_NAME		"mir3da"

/*
 * "1.0"@0xffffff80091bff91: it is at once the .version of the
 * module_version_attribute (0xffffff800992eb58) and the first argument
 * of the sprintf in mir3da_version_show ("913e4442 add x2, x2, #0xf91").
 */
#define MIR3DA_DRIVER_VERSION	"1.0"

/* "52800280 mov w0, #0x14" in mir3da_get_data: msleep(20). */
#define MIR3DA_POWERON_DELAY_MS	20

/*
 * The size of the string buffer is measured, not chosen: in
 * mir3da_get_data the buffer sits at sp+20 and the databuf array at
 * x29-0x6c = sp+276, that is 256 bytes later; in
 * mir3da_factory_get_raw_data the clearing runs from
 * "a900ffff stp xzr, xzr, [sp,#8]" to "a90fffff stp xzr, xzr, [sp,#248]",
 * that is exactly 256 bytes.
 */
#define MIR3DA_BUFSIZE		256

/*
 * The axis indexes. "3940f308 ldrb w8, [x24,#60]" and the two that follow
 * read cvt.map[0..2]; "39c0e30d ldrsb w13, [x24,#56]" and the two
 * that follow read cvt.sign[0..2].
 */
#define MIR3DA_AXIS_X		0
#define MIR3DA_AXIS_Y		1
#define MIR3DA_AXIS_Z		2
#define MIR3DA_AXES_NUM		3

/*
 * "5284c9e9 mov w9, #0x264f" in mir3da_get_data and "5284c9eb mov w11, #0x264f"
 * in mir3da_factory_get_cali: 9807, the value of g in mg that MediaTek
 * calls GRAVITY_EARTH_1000.
 */
#define GRAVITY_EARTH_1000	9807

/* "321603e2 orr w2, wzr, #0x400" (mir3da_probe) e
 * "321603eb orr w11, wzr, #0x400" (mir3da_factory_set_cali): 1024 LSB/g. */
#define MIR3DA_GAIN		1024

/*
 * "7114551f cmp w8, #0x515" in mir3da_get_data: the threshold beyond which
 * the software offset of the Z axis subtracts 1 g from the reading. The
 * comparison is `b.lt` on the absolute value, so the source condition is
 * > 1300.
 */
#define MIR3DA_CALI_Z_LIMIT	1300

/* "5120014a sub w10, w10, #0x800": 2048 = 2 * MIR3DA_GAIN. */
#define MIR3DA_CALI_Z_ADJUST	2048

/*
 * "71004d1f cmp w8, #0x13": the ID the chip has to answer at register 0x01
 * for the I2C address 0x26 to be the right one.
 */
#define MIR3DA_REG_CHIP_ID	0x01
#define MIR3DA_CHIP_ID		0x13

/*
 * "528004c8 mov w8, #0x26" and "528004e8 mov w8, #0x27": the two possible
 * I2C addresses of the chip.
 */
#define MIR3DA_I2C_ADDR_A	0x26
#define MIR3DA_I2C_ADDR_B	0x27

/*
 * "5295d700 mov w0, #0xaeb8" + "72a028e0 movk w0, #0x147, lsl #16" =
 * 0x147aeb8 = 21475000 = 5000 * 0x10C7, that is udelay(5000) = mdelay(5).
 */
#define MIR3DA_CHIP_ID_RETRY_MS	5

/*
 * How many times mir3da_probe re-reads the identity register before
 * falling back to the second address: the `bl i2c_smbus_read_byte_data` calls at
 * 0x87889d4, 0x87889f8, 0x8788a1c, 0x8788a40, 0x8788a64, 0x8788a88 are
 * six, and the five `bl __const_udelay` sit only between one and the next.
 */
#define MIR3DA_CHIP_ID_RETRY	6

/*
 * This section was reconstructed from the factory kernel disassembly (104 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct mir3da_data {
	struct i2c_client	*client;	/* +0   */
	struct acc_hw		hw;		/* +8   */
	struct hwmsen_convert	cvt;		/* +56  */
	bool			flush;		/* +64  */
	atomic_t		campo_0x44;	/* +68  */
	atomic_t		suspended;	/* +72  */
	int			campo_0x4c;	/* +76  */
	s16			cali_sw[8];	/* +80  */
	s16			data[MIR3DA_AXES_NUM];	/* +96 */
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009cba000).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static bool mir3da_init_flag;
static struct mir3da_data *mir3da_obj;
static struct i2c_client *mir3da_i2c_client;
static PLAT_HANDLE mir3da_handle;
static struct semaphore mir3da_sem;
static bool mir3da_sensor_power;

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static bool mir3da_gain_x;
static bool mir3da_gain_y;
static bool mir3da_gain_z;

static int mir3da_local_init(void);
static int mir3da_local_remove(void);
static int mir3da_setPowerMode(struct i2c_client *client, bool on);
static int mir3da_flush(void);

/*
 * "mir3da"@0xffffff80091bf966 — the same string serves .name here and
 * .driver.name of the i2c_driver: the linker keeps a single copy.
 * The .platform_diver_addr field (+24, 0xffffff800992eb78) is NULL in the
 * factory binary.
 */
static struct acc_init_info mir3da_init_info = {
	.name = MIR3DA_DRV_NAME,
	.init = mir3da_local_init,
	.uninit = mir3da_local_remove,
};

/* ------------------------------------------------------------------ */
/* The operations table the core has handed to it */
/* ------------------------------------------------------------------ */

/*
 * The four functions that follow are global (`T` in stock.map) because
 * the core takes them by address from general_op_s: no function in the
 * block calls them with a `bl`.
 */
int i2c_smbus_read(PLAT_HANDLE handle, u8 addr, u8 *data)
{
	*data = i2c_smbus_read_byte_data((struct i2c_client *)handle, addr);
	return 0;
}

int i2c_smbus_read_block(PLAT_HANDLE handle, u8 addr, u8 len, u8 *buf)
{
	return i2c_smbus_read_i2c_block_data((struct i2c_client *)handle,
					     addr, len, buf);
}

int i2c_smbus_write(PLAT_HANDLE handle, u8 addr, u8 data)
{
	return i2c_smbus_write_byte_data((struct i2c_client *)handle,
					 addr, data);
}

/*
 * "52912b00 mov w0, #0x8958" + "72a00820 movk w0, #0x41, lsl #16" =
 * 0x418958 = 4295000 = 1000 * 0x10C7, that is udelay(1000), inside a loop
 * counting from -msec to 0 ("cb20c113 sub x19, x8, w0, sxtw",
 * "b1000673 adds x19, x19, #0x1", "54ffff83 b.cc"): that is exactly the
 * expansion of mdelay() for a non-constant argument.
 */
void msdelay(int msec)
{
	mdelay(msec);
}

/* "chip init failed !\n\n"@0xffffff80091bf945 */
static int get_address(PLAT_HANDLE handle)
{
	if (handle == NULL) {
		MI_ERR("chip init failed !\n\n");
		return -1;
	}
	/* "79400400 ldrh w0, [x0,#2]": i2c_client.addr sits at +2. */
	return ((struct i2c_client *)handle)->addr;
}

/*
 * The structure at 0xffffff800992eac0, slot by slot from .rela.dyn.  The
 * four slots the factory leaves at zero stay at zero here: I have no
 * name for them and I am not inventing a function to put there.
 */
static struct general_op_s mir3da_general_ops = {
	.smi2c_read = i2c_smbus_read,
	.smi2c_write = i2c_smbus_write,
	.smi2c_read_block = i2c_smbus_read_block,
	.get_address = get_address,
	.myprintk = printk,
	.mysprintf = sprintf,
	.msdelay = msdelay,
};

/* ------------------------------------------------------------------ */
/* Calibration — the three inlined functions (lines 119, 131, 146) */
/* ------------------------------------------------------------------ */

/*
 * Factory line 119: "52800ee2 mov w2, #0x77" in the MI_FUN inlined
 * into mir3da_factory_clear_cali, with __func__ =
 * 'mir3da_resetCalibration' at 0xffffff80091bfc70.
 * The clearing is "a9057e7f stp xzr, xzr, [x19,#80]", sixteen bytes.
 */
static int mir3da_resetCalibration(PLAT_HANDLE handle)
{
	struct mir3da_data *obj = i2c_get_clientdata(mir3da_i2c_client);

#line 119
	MI_FUN;
	memset(obj->cali_sw, 0, sizeof(obj->cali_sw));

	return 0;
}

/*
 * Line 131: "52801062 mov w2, #0x83", __func__ =
 * 'mir3da_readCalibration' at 0xffffff80091bfdd0.  The three reads are
 * "79c0a2a8 ldrsh w8, [x21,#80]", "79c0a6a9 ldrsh w9, [x21,#82]",
 * "79c0aaaa ldrsh w10, [x21,#84]".
 */
static int mir3da_readCalibration(PLAT_HANDLE handle, int *dat)
{
	struct mir3da_data *obj = i2c_get_clientdata(mir3da_i2c_client);

#line 131
	MI_FUN;
	dat[MIR3DA_AXIS_X] = obj->cali_sw[MIR3DA_AXIS_X];
	dat[MIR3DA_AXIS_Y] = obj->cali_sw[MIR3DA_AXIS_Y];
	dat[MIR3DA_AXIS_Z] = obj->cali_sw[MIR3DA_AXIS_Z];

	return 0;
}

/*
 * Line 146: "52801242 mov w2, #0x92", __func__ =
 * 'mir3da_writeCalibration' a 0xffffff80091bfd2f.
 *
 * "null ptr!!\n\n"@0xffffff80091bfd52
 * "write_cali  raw cali_sw[%d][%d][%d] dat[%d][%d][%d]\n"@0xffffff80091bfd6a
 * "write_cali  new cali_sw[%d][%d][%d] \n"@0xffffff80091bfdaa
 *
 * The final wait is "52912b00 mov w0, #0x8958" + "72a00820 movk w0,
 * #0x41, lsl #16" = udelay(1000) = mdelay(1).
 */
static int mir3da_writeCalibration(PLAT_HANDLE handle, int *dat)
{
	struct mir3da_data *obj = i2c_get_clientdata(mir3da_i2c_client);
	int cali[MIR3DA_AXES_NUM];

#line 146
	MI_FUN;
	if (obj == NULL) {
		MI_ERR("null ptr!!\n\n");
		return -1;
	}

	mir3da_readCalibration(handle, cali);
	MI_MSG("write_cali  raw cali_sw[%d][%d][%d] dat[%d][%d][%d]\n",
	       cali[MIR3DA_AXIS_X], cali[MIR3DA_AXIS_Y], cali[MIR3DA_AXIS_Z],
	       dat[MIR3DA_AXIS_X], dat[MIR3DA_AXIS_Y], dat[MIR3DA_AXIS_Z]);

	obj->cali_sw[MIR3DA_AXIS_X] = cali[MIR3DA_AXIS_X] + dat[MIR3DA_AXIS_X];
	obj->cali_sw[MIR3DA_AXIS_Y] = cali[MIR3DA_AXIS_Y] + dat[MIR3DA_AXIS_Y];
	obj->cali_sw[MIR3DA_AXIS_Z] = cali[MIR3DA_AXIS_Z] + dat[MIR3DA_AXIS_Z];

	MI_MSG("write_cali  new cali_sw[%d][%d][%d] \n",
	       obj->cali_sw[MIR3DA_AXIS_X], obj->cali_sw[MIR3DA_AXIS_Y],
	       obj->cali_sw[MIR3DA_AXIS_Z]);
	mdelay(1);

	return 0;
}

/* ------------------------------------------------------------------ */
/* The sysfs attributes (lines ~300-500 of the factory source) */
/* ------------------------------------------------------------------ */

/* Line 307: "52802662 mov w2, #0x133".  "%d\n"@0xffffff80092ae2ec.
 * The `enable` that is read is one byte: "394013e2 ldrb w2, [sp,#4]". */
static ssize_t mir3da_enable_show(struct device_driver *ddri, char *buf)
{
	int result;
	u8 enable;
	/*
	 * THE HANDLE IS READ BEFORE THE PRINT, and the binary says so: the
	 * factory saves x20 ("a9014ff4 stp x20, x19, [sp,#16]" against our
	 * "f9000bf3 str x19") and loads the global at
	 * "f940b934 ldr"@0xffffff80087899e0, that is BEFORE the
	 * "361800e8 tbz"@0xffffff80087899e4 that skips the MI_FUN printk. A
	 * value live across a call is a value the source has already read.
	 */
	PLAT_HANDLE h = mir3da_handle;

#line 307
	MI_FUN;
	result = mir3da_get_enable(h, &enable);
	if (result < 0)
		return -EINVAL;

	return sprintf(buf, "%d\n", enable);
}

/*
 * "b40002c1 cbz x1, ..." — the first check is on `buf`, and the value
 * returned in that case is "92800000 mov x0, #0xffffffffffffffff", -1.
 * The conversion base is "52800142 mov w2, #0xa", decimal.
 * "f100001f cmp x0, #0x0" + "1a9f07e1 cset w1, ne" is the conversion to
 * bool of the argument passed to mir3da_set_enable.
 */
static ssize_t mir3da_enable_store(struct device_driver *ddri,
				   const char *buf, size_t count)
{
	unsigned long enable;
	PLAT_HANDLE h;

	if (buf == NULL)
		return -1;

	/*
	 * AS IN `mir3da_enable_show`: the handle is read BEFORE the call, not at
	 * the point of use. The factory loads it at
	 * "f940b914 ldr"@0xffffff8008789a6c, that is before the
	 * "941b7d60 bl"@0xffffff8008789a80 towards <simple_strtoul>, and keeps it
	 * in x20 across that call -- which is exactly why it saves x20
	 * ("a9be4ff4 stp x20, x19, [sp,#-32]!").
	 */
	h = mir3da_handle;

	enable = simple_strtoul(buf, NULL, 10);
	if (mir3da_set_enable(h, enable != 0) < 0)
		return -EINVAL;

	/*
	 * THE RETURN VALUE GOES THROUGH AN `int`, and the binary says so:
	 * "93407e68 sxtw"@0xffffff8008789a98 sign-extends the low 32 bits of
	 * w19 (where `count` lives) before selecting it. With a `size_t` returned
	 * directly that extension is unnecessary and clang does not emit it -- those
	 * were the four bytes of difference.
	 */
	return (int)count;
}

/*
 * "x= %d;y=%d;z=%d\n"@0xffffff80091bfe5b
 * "reading failed!"@0xffffff80091bfe6c — sixteen constant bytes that clang
 * copies with "a9402508 ldp x8, x9, [x8]" + "a9002668 stp x8, x9, [x19]" and
 * a constant length "32000fe0 orr w0, wzr, #0xf": it is an sprintf
 * with no specifiers, reduced to a copy.
 */
static ssize_t mir3da_axis_data_show(struct device_driver *ddri, char *buf)
{
	int result;
	short x, y, z;

	result = mir3da_read_data(mir3da_handle, &x, &y, &z);
	if (result == 0)
		return sprintf(buf, "x= %d;y=%d;z=%d\n", x, y, z);

	return sprintf(buf, "reading failed!");
}

static ssize_t mir3da_reg_data_show(struct device_driver *ddri, char *buf)
{
	return mir3da_get_reg_data(mir3da_handle, buf);
}

/*
 * Line 376: "52802f02 mov w2, #0x178".
 * "0x%x, 0x%x\n"@0xffffff800916770a
 * "set[0x%x]->[0x%x]\n\n"@0xffffff80091bff4c
 * The text of the asserted expression, 'result==0' at 0xffffff80091bff37, is
 * the stringification of #expr: I do not write it as a literal, ASSERT produces it.
 */
static ssize_t mir3da_reg_data_store(struct device_driver *ddri,
				     const char *buf, size_t count)
{
	int addr, data;
	int result;

	sscanf(buf, "0x%x, 0x%x\n", &addr, &data);
	result = mir3da_register_write(mir3da_handle, addr, data);
	ASSERT(result == 0);
	MI_MSG("set[0x%x]->[0x%x]\n\n", addr, data);

	return count;
}

/* "%d\n"@0xffffff80092ae2ec */
static ssize_t mir3da_log_level_show(struct device_driver *ddri, char *buf)
{
	return sprintf(buf, "%d\n", Log_level);
}

static ssize_t mir3da_log_level_store(struct device_driver *ddri,
				      const char *buf, size_t count)
{
	Log_level = simple_strtoul(buf, NULL, 10);

	return count;
}

/*
 * "x=%d ,y=%d ,z=%d\n"@0xffffff80091bff6f.  The three integers are cleared
 * before the call — "f9000bff str xzr, [sp,#16]" and
 * "b9000fff str wzr, [sp,#12]" — and they are `int`, not `short`: the
 * read-back is "29420be3 ldp w3, w2, [sp,#16]".
 */
static ssize_t mir3da_primary_offset_show(struct device_driver *ddri, char *buf)
{
	int x = 0, y = 0, z = 0;

	mir3da_get_primary_offset(mir3da_handle, &x, &y, &z);

	return sprintf(buf, "x=%d ,y=%d ,z=%d\n", x, y, z);
}

/* "%s\n"@0xffffff8009226be1, "MiraMEMS"@0xffffff80091bff81 */
static ssize_t mir3da_vendor_show(struct device_driver *ddri, char *buf)
{
	return sprintf(buf, "%s\n", "MiraMEMS");
}

/* "%s_%s\n"@0xffffff80091bff8a */
static ssize_t mir3da_version_show(struct device_driver *ddri, char *buf)
{
	return sprintf(buf, "%s_%s\n", MIR3DA_DRIVER_VERSION,
		       MIR3DA_CORE_VERSION);
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800992ecb8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct driver_attribute driver_attr_enable = {
	.attr = { .name = "enable", .mode = 0644 },
	.show = mir3da_enable_show,
	.store = mir3da_enable_store,
};
static struct driver_attribute driver_attr_axis_data = {
	.attr = { .name = "axis_data", .mode = 0444 },
	.show = mir3da_axis_data_show,
};
static struct driver_attribute driver_attr_reg_data = {
	.attr = { .name = "reg_data", .mode = 0644 },
	.show = mir3da_reg_data_show,
	.store = mir3da_reg_data_store,
};
static struct driver_attribute driver_attr_log_level = {
	.attr = { .name = "log_level", .mode = 0644 },
	.show = mir3da_log_level_show,
	.store = mir3da_log_level_store,
};
static struct driver_attribute driver_attr_primary_offset = {
	.attr = { .name = "primary_offset", .mode = 0444 },
	.show = mir3da_primary_offset_show,
};
static struct driver_attribute driver_attr_vendor = {
	.attr = { .name = "vendor", .mode = 0444 },
	.show = mir3da_vendor_show,
};
static struct driver_attribute driver_attr_version = {
	.attr = { .name = "version", .mode = 0444 },
	.show = mir3da_version_show,
};

static struct driver_attribute *mir3da_attr_list[] = {
	&driver_attr_enable,
	&driver_attr_axis_data,
	&driver_attr_reg_data,
	&driver_attr_log_level,
	&driver_attr_primary_offset,
	&driver_attr_vendor,
	&driver_attr_version,
};

/* "driver_create_file (%s) = %d\n\n"@0xffffff80091bfe1f */
static int mir3da_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(mir3da_attr_list);

	if (driver == NULL)
		return -EINVAL;

	for (idx = 0; idx < num; idx++) {
		err = driver_create_file(driver, mir3da_attr_list[idx]);
		if (err) {
			MI_MSG("driver_create_file (%s) = %d\n\n",
			       mir3da_attr_list[idx]->attr.name, err);
			break;
		}
	}

	return err;
}

static int mir3da_delete_attr(struct device_driver *driver)
{
	int idx;
	int num = ARRAY_SIZE(mir3da_attr_list);

	if (driver == NULL)
		return -EINVAL;

	for (idx = 0; idx < num; idx++)
		driver_remove_file(driver, mir3da_attr_list[idx]);

	return 0;
}

/* ------------------------------------------------------------------ */
/* Suspend (lines 538 and 566)                                       */
/* ------------------------------------------------------------------ */

/*
 * Line 538: "52804342 mov w2, #0x21a".
 * "null pointer!!\n\n"@0xffffff80091c03d7
 * "write power control fail!!\n\n"@0xffffff80091c03f3
 * "d1008273 sub x19, x19, #0x20" is to_i2c_client(dev): `struct device`
 * sits at +32 inside `struct i2c_client`.
 */
static int mir3da_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mir3da_data *obj = dev_get_drvdata(dev);

#line 538
	MI_FUN;
	if (obj == NULL) {
		MI_ERR("null pointer!!\n\n");
		return -EINVAL;
	}

	atomic_set(&obj->suspended, 1);
	down_interruptible(&mir3da_sem);
	if (mir3da_setPowerMode(client, false)) {
		up(&mir3da_sem);
		MI_ERR("write power control fail!!\n\n");
		return -1;
	}
	up(&mir3da_sem);

	return 0;
}

/*
 * Line 566: "528046c2 mov w2, #0x236".
 * "chip resume fail!!\n\n"@0xffffff80091c0429
 *
 * A factory divergence reproduced on purpose: on the branch where
 * mir3da_chip_resume fails the factory does NOT release the semaphore —
 * "14000019 b ..." jumps straight to the exit without the `bl up`. The
 * disassembly says so, so the code here does the same.
 */
static int mir3da_resume(struct device *dev)
{
	struct mir3da_data *obj = dev_get_drvdata(dev);
	int err;

#line 566
	MI_FUN;
	if (obj == NULL) {
		MI_ERR("null pointer!!\n\n");
		return -EINVAL;
	}

	down_interruptible(&mir3da_sem);
	err = mir3da_chip_resume(obj->client);
	if (err) {
		MI_ERR("chip resume fail!!\n\n");
		return err;
	}

	err = mir3da_setPowerMode(obj->client, true);
	up(&mir3da_sem);
	if (err) {
		MI_ERR("write power control fail!!\n\n");
		return err;
	}
	atomic_set(&obj->suspended, 0);

	return err;
}

/* ------------------------------------------------------------------ */
/* The control and data path (lines ~600-720)                 */
/* ------------------------------------------------------------------ */

/*
 * "mir3da_setPowerMode(), enable = %d\n"@0xffffff80091c00bd
 * "Sensor power status should not be set again!!!\n\n"@0xffffff80091c00ec
 * "remain flush, will call mir3da_flush in setPowerMode\n\n"@0xffffff80091c0128
 */
static int mir3da_setPowerMode(struct i2c_client *client, bool on)
{
	MI_MSG("mir3da_setPowerMode(), enable = %d\n", on);

	if (on == mir3da_sensor_power)
		MI_ERR("Sensor power status should not be set again!!!\n\n");

	if (mir3da_set_enable(client, on))
		return -1;

	mir3da_sensor_power = on;

	if (mir3da_obj->flush) {
		if (on) {
			MI_MSG("remain flush, will call mir3da_flush in setPowerMode\n\n");
			mir3da_flush();
		} else {
			mir3da_obj->flush = false;
		}
	}

	return 0;
}

static int mir3da_open_report_data(int open)
{
	return 0;
}

/*
 * mir3da_enable_nodata() was reconstructed from the factory kernel disassembly (0xffffff80091bffbb).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mir3da_enable_nodata(int en)
{
	int res = 0;
	int retry;
	bool power = false;

	if (en == 1)
		power = true;

	MI_ERR("mir3da_enable_nodata 0\n");

	for (retry = 0; retry < 3; retry++) {
		res = mir3da_setPowerMode(mir3da_i2c_client, power);
		if (res == 0) {
			MI_ERR("mir3da_SetPowerMode done\n\n");
			break;
		}
		MI_ERR("mir3da_SetPowerMode fail\n\n");
	}

	MI_ERR("mir3da_enable_nodata 1\n");

	if (res != 0) {
		MI_ERR("mir3da_SetPowerMode fail!\n\n");
		return -1;
	}

	MI_MSG("mir3da_enable_nodata OK!\n\n");
	MI_ERR("mir3da_enable_nodata 2\n");

	return 0;
}

/* "mir3da_batch 0\n"@0xffffff80091c016a */
static int mir3da_batch(int flag, int64_t samplingPeriodNs,
			int64_t maxBatchReportLatencyNs)
{
	MI_ERR("mir3da_batch 0\n");

	return 0;
}

/*
 * "mir3da_flush 0\n"@0xffffff80091c0185
 * "mir3da_flush 1\n"@0xffffff80091c01a0
 * The sensor-off branch leaves before the second message: in the binary
 * "340001a8 cbz w8, ..." leads to 0x8788fb0, which jumps straight
 * to the epilogue.
 */
static int mir3da_flush(void)
{
	int err;

	MI_ERR("mir3da_flush 0\n");

	if (!mir3da_sensor_power) {
		mir3da_obj->flush = true;
		return 0;
	}

	err = acc_flush_report();
	if (err >= 0)
		mir3da_obj->flush = false;

	MI_ERR("mir3da_flush 1\n");

	return err;
}

/*
 * "mir3da_set_delay \n"@0xffffff80091c01bb
 * "mir3daset_delay (%d), chip only use 1024HZ\n\n"@0xffffff80091c01d9
 * The division is signed and 32-bit — "529bd068 mov w8, #0xde83" +
 * "72a86368 movk w8, #0x431b, lsl #16" + "9b287e68 smull x8, w19, w8" +
 * "9372fd08 asr x8, x8, #50" is the magic reciprocal of 1000000 for an
 * `int`, not for the parameter's `u64`.
 */
static int mir3da_set_delay(u64 ns)
{
	int value = (int)ns / 1000 / 1000;

	MI_ERR("mir3da_set_delay \n");
	MI_MSG("mir3daset_delay (%d), chip only use 1024HZ\n\n", value);

	return 0;
}

/*
 * mir3da_ReadSensorData() was reconstructed from the factory kernel disassembly (0xffffff80091c0211).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mir3da_ReadSensorData(struct i2c_client *client, char *buf,
				 int bufsize)
{
	struct mir3da_data *obj;
	int res;
	/*
	 * NOT initialised: in mir3da_get_data the array sits at x29-0x6c and the
	 * disassembly has not a single store of zero towards that area. An
	 * `= {0}` would produce the clearing, because clang cannot prove that
	 * obj->cvt.map[] is a permutation and hence that all three cells get
	 * written.
	 */
	int databuf[MIR3DA_AXES_NUM];

	if (client == NULL) {
		*buf = 0;
		return 0;
	}
	obj = i2c_get_clientdata(client);

	if (!mir3da_sensor_power) {
		res = mir3da_setPowerMode(client, true);
		if (res)
			MI_ERR("Power on mir3da error %d!\n\n", res);
		msleep(MIR3DA_POWERON_DELAY_MS);
	}

	res = mir3da_read_data(client, &obj->data[MIR3DA_AXIS_X],
			       &obj->data[MIR3DA_AXIS_Y],
			       &obj->data[MIR3DA_AXIS_Z]);
	if (res) {
		MI_ERR("I2C error: ret value=%d\n", res);
		return res;
	}

	MI_MSG("read_sensor_data map[%d][%d][%d] sign[%d][%d][%d]\n",
	       obj->cvt.map[MIR3DA_AXIS_X], obj->cvt.map[MIR3DA_AXIS_Y],
	       obj->cvt.map[MIR3DA_AXIS_Z], obj->cvt.sign[MIR3DA_AXIS_X],
	       obj->cvt.sign[MIR3DA_AXIS_Y], obj->cvt.sign[MIR3DA_AXIS_Z]);
	MI_MSG("read_sensor_data xyz_0[%d][%d][%d] cali_sw[%d][%d][%d]\n",
	       obj->data[MIR3DA_AXIS_X], obj->data[MIR3DA_AXIS_Y],
	       obj->data[MIR3DA_AXIS_Z], obj->cali_sw[MIR3DA_AXIS_X],
	       obj->cali_sw[MIR3DA_AXIS_Y], obj->cali_sw[MIR3DA_AXIS_Z]);

	obj->data[MIR3DA_AXIS_X] +=
		obj->cvt.sign[MIR3DA_AXIS_X] * obj->cali_sw[obj->cvt.map[MIR3DA_AXIS_X]];
	obj->data[MIR3DA_AXIS_Y] +=
		obj->cvt.sign[MIR3DA_AXIS_Y] * obj->cali_sw[obj->cvt.map[MIR3DA_AXIS_Y]];
	obj->data[MIR3DA_AXIS_Z] +=
		obj->cvt.sign[MIR3DA_AXIS_Z] * obj->cali_sw[obj->cvt.map[MIR3DA_AXIS_Z]];

	MI_MSG("read_sensor_data xyz_1[%d][%d][%d]\n",
	       obj->data[MIR3DA_AXIS_X], obj->data[MIR3DA_AXIS_Y],
	       obj->data[MIR3DA_AXIS_Z]);

	databuf[obj->cvt.map[MIR3DA_AXIS_X]] =
		obj->cvt.sign[MIR3DA_AXIS_X] * obj->data[MIR3DA_AXIS_X];
	databuf[obj->cvt.map[MIR3DA_AXIS_Y]] =
		obj->cvt.sign[MIR3DA_AXIS_Y] * obj->data[MIR3DA_AXIS_Y];
	databuf[obj->cvt.map[MIR3DA_AXIS_Z]] =
		obj->cvt.sign[MIR3DA_AXIS_Z] * obj->data[MIR3DA_AXIS_Z];

	MI_MSG("read_sensor_data xyz_2[%d][%d][%d]\n",
	       databuf[obj->cvt.map[MIR3DA_AXIS_X]],
	       databuf[obj->cvt.map[MIR3DA_AXIS_Y]],
	       databuf[obj->cvt.map[MIR3DA_AXIS_Z]]);

	if (abs(obj->cali_sw[MIR3DA_AXIS_Z]) > MIR3DA_CALI_Z_LIMIT)
		databuf[obj->cvt.map[MIR3DA_AXIS_Z]] -= MIR3DA_CALI_Z_ADJUST;

	MI_DATA("mir3da data map: %d, %d, %d!\n\n", databuf[MIR3DA_AXIS_X],
		databuf[MIR3DA_AXIS_Y], databuf[MIR3DA_AXIS_Z]);

	databuf[MIR3DA_AXIS_X] =
		databuf[MIR3DA_AXIS_X] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;
	databuf[MIR3DA_AXIS_Y] =
		databuf[MIR3DA_AXIS_Y] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;
	databuf[MIR3DA_AXIS_Z] =
		databuf[MIR3DA_AXIS_Z] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;

	MI_MSG("read_sensor_data xyz_3[%d][%d][%d]\n", databuf[MIR3DA_AXIS_X],
	       databuf[MIR3DA_AXIS_Y], databuf[MIR3DA_AXIS_Z]);

	sprintf(buf, "%04x %04x %04x", databuf[MIR3DA_AXIS_X],
		databuf[MIR3DA_AXIS_Y], databuf[MIR3DA_AXIS_Z]);

	MI_DATA("mir3da data mg: x= %d, y=%d, z=%d\n\n", databuf[MIR3DA_AXIS_X],
		databuf[MIR3DA_AXIS_Y], databuf[MIR3DA_AXIS_Z]);

	return res;
}

/*
 * "%x %x %x"@0xffffff80091411f7.
 * The `status` returned is "321f03e8 orr w8, wzr, #0x2", that is 2:
 * SENSOR_STATUS_ACCURACY_MEDIUM.
 */
static int mir3da_get_data(int *x, int *y, int *z, int *status)
{
	char buff[MIR3DA_BUFSIZE];

	down_interruptible(&mir3da_sem);
	mir3da_ReadSensorData(mir3da_i2c_client, buff, MIR3DA_BUFSIZE);
	up(&mir3da_sem);

	sscanf(buff, "%x %x %x", x, y, z);
	*status = SENSOR_STATUS_ACCURACY_MEDIUM;

	return 0;
}

/* ------------------------------------------------------------------ */
/* The factory path (lines 725-793) */
/* ------------------------------------------------------------------ */

/*
 * Line 725: "52805aa2 mov w2, #0x2d5".
 * "%s enable sensor failed!\n\n"@0xffffff80091bfb53
 * The mir3da_batch message seen at the tail of this function is the
 * inlined mir3da_batch: its return value is constant 0, so clang removed
 * the check and no second error message exists in the binary.
 */
static int mir3da_factory_enable_sensor(bool enabledisable,
					int64_t sample_periods_ms)
{
	int err;

#line 725
	MI_FUN;
	err = mir3da_enable_nodata(enabledisable);
	if (err) {
		MI_ERR("%s enable sensor failed!\n\n", __func__);
		return -1;
	}

	mir3da_batch(0, sample_periods_ms, 0);

	return 0;
}

/*
 * Line 745: "52805d22 mov w2, #0x2e9".
 * "mir3da_factory_get_data %d %d %d\n"@0xffffff80091bfb91
 */
static int mir3da_factory_get_data(int32_t data[3], int *status)
{
#line 745
	MI_FUN;
	mir3da_get_data(&data[0], &data[1], &data[2], status);
	MI_MSG("mir3da_factory_get_data %d %d %d\n", data[0], data[1], data[2]);

	return 0;
}

/*
 * Line 759: "52805ee2 mov w2, #0x2f7".
 * "I2C error: ret value=%d\n"@0xffffff80091bfc0c
 * "support mir3da_factory_get_raw_data!\n\n"@0xffffff80091bfbda
 * "%04x %04x %04x"@0xffffff80091bfc25
 *
 * A factory defect reproduced: the function formats the three axes into a
 * local buffer and never writes into `data[3]`, which is the output
 * parameter.  There is no store on x0 anywhere in the binary.
 */
static int mir3da_factory_get_raw_data(int32_t data[3])
{
	struct mir3da_data *obj;
	char strbuf[MIR3DA_BUFSIZE] = {0};
	int res;

#line 759
	MI_FUN;
	if (mir3da_i2c_client != NULL) {
		obj = i2c_get_clientdata(mir3da_i2c_client);
		res = mir3da_read_data(mir3da_i2c_client,
				       &obj->data[MIR3DA_AXIS_X],
				       &obj->data[MIR3DA_AXIS_Y],
				       &obj->data[MIR3DA_AXIS_Z]);
		if (res)
			MI_ERR("I2C error: ret value=%d\n", res);
		else
			sprintf(strbuf, "%04x %04x %04x",
				obj->data[MIR3DA_AXIS_X],
				obj->data[MIR3DA_AXIS_Y],
				obj->data[MIR3DA_AXIS_Z]);
	}

	MI_MSG("support mir3da_factory_get_raw_data!\n\n");

	return 0;
}

/* Line 769: "52806022 mov w2, #0x301". */
static int mir3da_factory_enable_calibration(void)
{
#line 769
	MI_FUN;

	return 0;
}

/* Line 778: "52806142 mov w2, #0x30a". */
static int mir3da_factory_clear_cali(void)
{
#line 778
	MI_FUN;
	mir3da_resetCalibration(mir3da_handle);

	return 0;
}

/*
 * mir3da_factory_set_cali() was reconstructed from the factory kernel disassembly (0xffffff80091bfcab).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mir3da_factory_set_cali(int32_t offset[3])
{
	int err;
	int cali[MIR3DA_AXES_NUM];

#line 793
	MI_FUN;
	MI_MSG("mir3da_factory_set_cali ori %d %d %d\n", offset[0], offset[1],
	       offset[2]);

	cali[MIR3DA_AXIS_X] = offset[MIR3DA_AXIS_X] *
			      (mir3da_gain_x * MIR3DA_GAIN) / GRAVITY_EARTH_1000;
	cali[MIR3DA_AXIS_Y] = offset[MIR3DA_AXIS_Y] *
			      (mir3da_gain_y * MIR3DA_GAIN) / GRAVITY_EARTH_1000;
	cali[MIR3DA_AXIS_Z] = offset[MIR3DA_AXIS_Z] *
			      (mir3da_gain_z * MIR3DA_GAIN) / GRAVITY_EARTH_1000;

	MI_MSG("mir3da_factory_set_cali new %d %d %d\n", cali[MIR3DA_AXIS_X],
	       cali[MIR3DA_AXIS_Y], cali[MIR3DA_AXIS_Z]);

	err = mir3da_writeCalibration(mir3da_handle, cali);
	if (err) {
		MI_ERR("mir3da_WriteCalibration failed!\n\n");
		return -1;
	}

	return 0;
}

/*
 * "mir3da_factory_get_cali %d %d %d\n"@0xffffff80091bfdf2.
 * The inverse conversion is "5284c9eb mov w11, #0x264f" +
 * "110ffd0b add w11, w8, #0x3ff" + "130a7d01 asr w1, w8, #10":
 * a multiplication by 9807 and a signed division by 1024.
 */
static int mir3da_factory_get_cali(int32_t offset[3])
{
	int cali[MIR3DA_AXES_NUM];

	mir3da_readCalibration(mir3da_handle, cali);

	offset[MIR3DA_AXIS_X] =
		cali[MIR3DA_AXIS_X] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;
	offset[MIR3DA_AXIS_Y] =
		cali[MIR3DA_AXIS_Y] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;
	offset[MIR3DA_AXIS_Z] =
		cali[MIR3DA_AXIS_Z] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;

	MI_MSG("mir3da_factory_get_cali %d %d %d\n", offset[MIR3DA_AXIS_X],
	       offset[MIR3DA_AXIS_Y], offset[MIR3DA_AXIS_Z]);

	return 0;
}

static int mir3da_factory_do_self_test(void)
{
	return 0;
}

/*
 * The table at 0xffffff800992ec78, eight pointers in the order
 * .rela.dyn lists them.  The structure containing it sits at
 * 0xffffff800992ec68 and is { gain = 1, sensitivity = 1, fops = &table }:
 * the two integers are read directly in .data, "01 00 00 00 01 00 00 00".
 */
static struct accel_factory_fops mir3da_factory_fops = {
	.enable_sensor = mir3da_factory_enable_sensor,
	.get_data = mir3da_factory_get_data,
	.get_raw_data = mir3da_factory_get_raw_data,
	.enable_calibration = mir3da_factory_enable_calibration,
	.clear_cali = mir3da_factory_clear_cali,
	.set_cali = mir3da_factory_set_cali,
	.get_cali = mir3da_factory_get_cali,
	.do_self_test = mir3da_factory_do_self_test,
};

static struct accel_factory_public mir3da_factory_device = {
	.gain = 1,
	.sensitivity = 1,
	.fops = &mir3da_factory_fops,
};

/* ------------------------------------------------------------------ */
/* probe / remove (line 872) and registration (lines 1045-1074)          */
/* ------------------------------------------------------------------ */

/*
 * "[%s] gain: %d  %d  %d\n\n"@0xffffff80091bfb04 — the __func__ that goes
 * with it is 'mir3da_SetGain' at 0xffffff80091bfb1c, so in the factory
 * source this is a function of its own, inlined into mir3da_probe.
 * The three arguments are constants in the binary ("321603e2 orr w2, wzr,
 * #0x400" and the two that follow): 1024 three times, that is the three
 * flags just set true multiplied by MIR3DA_GAIN.
 */
static int mir3da_SetGain(void)
{
	mir3da_gain_x = true;
	mir3da_gain_y = true;
	mir3da_gain_z = true;

	MI_MSG("[%s] gain: %d  %d  %d\n\n", __func__,
	       mir3da_gain_x * MIR3DA_GAIN, mir3da_gain_y * MIR3DA_GAIN,
	       mir3da_gain_z * MIR3DA_GAIN);

	return 0;
}

/*
 * mir3da_probe() was reconstructed from the factory kernel disassembly (0xffffff80091bf9b5).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int mir3da_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct mir3da_data *obj;
	struct acc_control_path ctl = {0};
	struct acc_data_path data = {0};
	int err = 0;
	int i;

#line 872
	MI_FUN;

	obj = kzalloc(sizeof(struct mir3da_data), GFP_KERNEL);
	if (obj == NULL) {
		MI_ERR("kzalloc failed!\n");
		err = -ENOMEM;
		goto exit;
	}

	err = get_accel_dts_func(client->dev.of_node, &obj->hw);
	if (err < 0) {
		MI_ERR("get cust_baro dts info fail\n\n");
		goto exit_kfree;
	}

	err = hwmsen_get_convert(obj->hw.direction, &obj->cvt);
	if (err) {
		MI_ERR("invalid direction: %d\n\n", obj->hw.direction);
		goto exit_kfree;
	}

	if (client->addr != MIR3DA_I2C_ADDR_A)
		client->addr = MIR3DA_I2C_ADDR_A;

	obj->client = client;
	i2c_set_clientdata(client, obj);
	atomic_set(&obj->campo_0x44, 0);
	atomic_set(&obj->suspended, 0);
	mir3da_obj = obj;
	mir3da_i2c_client = client;
	sema_init(&mir3da_sem, 1);

	if (mir3da_install_general_ops(&mir3da_general_ops)) {
		MI_ERR("Install ops failed !\n\n");
		err = 0;
		goto exit_kfree;
	}

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff80087889cc, 212 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	for (i = 0; i < MIR3DA_CHIP_ID_RETRY; i++) {
		if ((i2c_smbus_read_byte_data(mir3da_i2c_client,
					      MIR3DA_REG_CHIP_ID) & 0xff)
		    == MIR3DA_CHIP_ID)
			break;
		if (i < MIR3DA_CHIP_ID_RETRY - 1)
			mdelay(MIR3DA_CHIP_ID_RETRY_MS);
	}
	if (i == MIR3DA_CHIP_ID_RETRY)
		client->addr = MIR3DA_I2C_ADDR_B;

	mir3da_handle = mir3da_core_init(mir3da_i2c_client);
	if (mir3da_handle == NULL) {
		MI_ERR("chip init failed !\n\n");
		err = 0;
		goto exit_kfree;
	}

	mir3da_SetGain();

	ctl.is_use_common_factory = false;
	err = accel_factory_device_register(&mir3da_factory_device);
	if (err) {
		MI_ERR("acc_factory register failed.\n\n");
		goto exit_kfree;
	}

	err = mir3da_create_attr(&mir3da_init_info.platform_diver_addr->driver);
	if (err) {
		MI_ERR("create attribute result = %d\n\n", err);
		err = -EINVAL;
		goto exit_kfree;
	}

	ctl.open_report_data = mir3da_open_report_data;
	ctl.enable_nodata = mir3da_enable_nodata;
	ctl.set_delay = mir3da_set_delay;
	ctl.batch = mir3da_batch;
	ctl.flush = mir3da_flush;
	ctl.is_report_input_direct = false;
	ctl.is_support_batch = obj->hw.is_batch_supported;

	err = acc_register_control_path(&ctl);
	if (err) {
		MI_ERR("register acc control path err\n\n");
		goto exit_kfree;
	}

	data.get_data = mir3da_get_data;
	data.vender_div = 1000;
	err = acc_register_data_path(&data);
	if (err) {
		MI_ERR("register acc data path err= %d\n\n", err);
		goto exit_kfree;
	}

	mir3da_init_flag = false;

	return err;

exit_kfree:
	kfree(obj);
exit:
	MI_ERR("%s: err = %d\n\n", __func__, err);
	mir3da_handle = NULL;
	mir3da_i2c_client = NULL;
	mir3da_obj = NULL;
	mir3da_init_flag = true;

	return err;
}

/* "mir3da_delete_attr fail: %d\n\n"@0xffffff80091c039f */
static int mir3da_remove(struct i2c_client *client)
{
	int err;

	err = mir3da_delete_attr(&mir3da_init_info.platform_diver_addr->driver);
	if (err)
		MI_ERR("mir3da_delete_attr fail: %d\n\n", err);

	mir3da_i2c_client = NULL;
	i2c_unregister_device(client);
	accel_factory_device_deregister(&mir3da_factory_device);
	kfree(i2c_get_clientdata(client));

	return 0;
}

/*
 * "mediatek,gsensor"@0xffffff8008f53f28 is the compatible of the Device
 * Tree node: that address is offset +64 (the `compatible` field) of the
 * struct of_device_id at 0xffffff8008f53ee8 pointed to by
 * .driver.of_match_table (0xffffff800992ebe8). The table has two entries —
 * between its start and the struct dev_pm_ops at 0xffffff8008f54078 there
 * are exactly 400 bytes, that is 2 * sizeof(struct of_device_id).
 */
static const struct of_device_id gsensor_of_match[] = {
	{ .compatible = "mediatek,gsensor" },
	{},
};

/*
 * "mir3da" — the copy inside the i2c_device_id sits at 0xffffff8008f54130 and
 * the table occupies 64 bytes (two 32-byte entries) up to 0xffffff8008f54170.
 */
static const struct i2c_device_id mir3da_i2c_id[] = {
	{ MIR3DA_DRV_NAME, 0 },
	{},
};

/*
 * The struct dev_pm_ops at 0xffffff8008f54078 has six relocated slots:
 * +16 and +48 -> mir3da_suspend, +24 and +56 -> mir3da_resume, +32 ->
 * mir3da_suspend, +40 -> mir3da_resume, that is suspend/resume/freeze/thaw/
 * poweroff/restore.  That is exactly what SIMPLE_DEV_PM_OPS produces.
 */
static SIMPLE_DEV_PM_OPS(mir3da_pm_ops, mir3da_suspend, mir3da_resume);

/*
 * The i2c_driver at 0xffffff800992eb80: .probe at +16 and .remove at +24
 * (at this sublevel struct i2c_driver still has attach_adapter at +8),
 * .driver at +64, .id_table at +184.  The .class field (+0) is not
 * initialised.
 */
static struct i2c_driver mir3da_i2c_driver = {
	.probe = mir3da_probe,
	.remove = mir3da_remove,
	.id_table = mir3da_i2c_id,
	.driver = {
		.name = MIR3DA_DRV_NAME,
		.of_match_table = gsensor_of_match,
		.pm = &mir3da_pm_ops,
	},
};

/*
 * Line 1045: "528082a2 mov w2, #0x415".
 * "add driver error\n\n"@0xffffff80091bf98a
 * The first argument of i2c_register_driver is "aa1f03e0 mov x0, xzr",
 * that is THIS_MODULE with the driver built into the kernel: it is
 * i2c_add_driver().
 */
static int mir3da_local_init(void)
{
#line 1045
	MI_FUN;

	if (i2c_add_driver(&mir3da_i2c_driver)) {
		MI_ERR("add driver error\n\n");
		return -1;
	}

	return mir3da_init_flag ? -1 : 0;
}

/* Line 1063: "528084e2 mov w2, #0x427". */
static int mir3da_local_remove(void)
{
#line 1063
	MI_FUN;
	i2c_del_driver(&mir3da_i2c_driver);

	return 0;
}

/*
 * mir3da_init() was reconstructed from the factory kernel disassembly (0xffffff80093740fc).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int __init mir3da_init(void)
{
#line 1074
	MI_FUN;
	acc_driver_add(&mir3da_init_info);

	return 0;
}

module_init(mir3da_init);

static void __exit mir3da_exit(void)
{
	MI_FUN;
}

module_exit(mir3da_exit);

/*
 * MODULE_VERSION() was reconstructed from the factory kernel disassembly (0xffffff800992eb18).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_accelerometer_da218-B_mir3da_cust.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
MODULE_VERSION(MIR3DA_DRIVER_VERSION);
MODULE_LICENSE("GPL");
