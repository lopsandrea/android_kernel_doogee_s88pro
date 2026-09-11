/*
 * GC0310 MIPI YUV camera sensor -- reconstructed from the factory binary.
 *
 * It exists in no public tree: ALPS has a gc0310.c for x86 under
 * staging/media/atomisp, a different platform and a different interface.
 * Every line here comes from stock.elf.
 *
 * That the file belongs in camera_project/e977_dg_m13_71_q0/ is not stated by
 * a __FILE__ -- GC0310 does not use that macro -- but by adjacency in
 * stock.map.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/string.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "imgsensor_i2c.h"

#include "imgsensor_hw.h"
#include "kd_camera_feature.h"

#include "gc0310mipi_Sensor.h"

/*
 * gi2c is NOT declared in imgsensor_i2c.h: the .c defines it and that is
 * all. The factory reaches it all the same -- "f000ac28 adrp"@0xffffff800870aa3c
 * plus `add #0x258` give 0xffffff8258, which is &gi2c.inst[1].pi2c_client --
 * so its source declares it itself too.
 */
extern struct IMGSENSOR_I2C gi2c;

/*
 * THE MUTEX BELONGS TO THIS FILE, and the proof is an address.
 *
 * Every I2C operation of GC0310 takes the mutex at
 * 0xffffff800991a098 ("90009080 adrp"@0xffffff800870ac0c plus `add #0x098`),
 * while GC032A takes the one at 0xffffff800991b860. Two different addresses
 * for the same operation are two different objects: a DEFINE_MUTEX per
 * file, not imgsensor's shared one.
 */
static DEFINE_MUTEX(gc0310_mutex);

/*
 * THE SYNTHETIC NAMES ARE THE ADDRESSES. These four objects are static and
 * stock.map does not name them; christening them "cap_state" or "video_mode"
 * would mean passing a reading off as a measurement.
 */
static kal_bool g9c95930;
static kal_bool g9c95938;
static kal_bool g9c9593c;
static kal_uint8 g9c95940;
static MUINT32 g9c959bc;
static MSDK_SENSOR_CONFIG_STRUCT g9c95944;

#define GC0310MIPI_I2C_ADDR 0x21   /* seven bit: "52800429 mov"@0xffffff800870ac24 */
#define GC0310MIPI_I2C_SPEED 400   /* 0x61a80 = 400000 */

/*
 * THE I2C INSTANCE IS NUMBER 1, and it follows from the offset:
 * imgsensor_i2c_init shows that gi2c.inst[N] sits at 0xffffff8009c90250 +
 * N * 4096 ("8b083128 add"@0xffffff8008704970 is a twelve-bit shift),
 * so 0xffffff8009c91250 is inst[1].
 */
#define GC0310MIPI_INST (&gi2c.inst[IMGSENSOR_I2C_DEV_1])

/*
 * GC0310MIPI_write_cmos_sensor() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void GC0310MIPI_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
	struct IMGSENSOR_I2C_INST *pinst = GC0310MIPI_INST;
	int ret = 0;
	char puSendCmd[2] = { (char)addr, (char)para };

	mutex_lock(&gc0310_mutex);

	pinst->msg[0].addr = GC0310MIPI_I2C_ADDR;
	pinst->msg[0].flags = 0;
	pinst->msg[0].len = 2;
	pinst->msg[0].buf = puSendCmd;

	if (mtk_i2c_transfer(pinst->pi2c_client->adapter, pinst->msg, 1, 0,
			     GC0310MIPI_I2C_SPEED * 1000) != 1)
		pr_err("--lijian--I2C write failed (0x%x)! speed(0=%d)\n",
		       ret, GC0310MIPI_I2C_SPEED);

	mutex_unlock(&gc0310_mutex);
}

/*
 * THE READ, twin of the write: two messages instead of one, and the
 * value comes back from the two bytes of the receive buffer.
 */
static kal_uint16 GC0310MIPI_read_cmos_sensor_(kal_uint8 addr)
{
	struct IMGSENSOR_I2C_INST *pinst = GC0310MIPI_INST;
	int ret = 0;
	char puSendCmd = (char)addr;
	kal_uint16 get_byte = 0;

	mutex_lock(&gc0310_mutex);

	pinst->msg[0].addr = GC0310MIPI_I2C_ADDR;
	pinst->msg[0].flags = 0;
	pinst->msg[0].len = 1;
	pinst->msg[0].buf = &puSendCmd;

	pinst->msg[1].addr = GC0310MIPI_I2C_ADDR;
	pinst->msg[1].flags = I2C_M_RD;
	pinst->msg[1].len = 1;
	pinst->msg[1].buf = (u8 *)&get_byte;

	if (mtk_i2c_transfer(pinst->pi2c_client->adapter, pinst->msg, 2, 0,
			     GC0310MIPI_I2C_SPEED * 1000) != 2)
		pr_err("--lijian--I2C read failed (0x%x)! speed(0=%d)\n",
		       ret, GC0310MIPI_I2C_SPEED);

	mutex_unlock(&gc0310_mutex);

	return get_byte;
}

/*
 * gc0310_i2c_write() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int gc0310_i2c_write(u8 *pwrite_data, u16 write_length, u16 write_per_cycle,
		     u16 id, int speed)
{
	struct IMGSENSOR_I2C_INST *pinst = GC0310MIPI_INST;
	struct i2c_msg *pmsg = pinst->msg;
	u8 *pdata = pwrite_data;
	u8 *pend = pwrite_data + write_length;
	int ret = 0;
	int i = 0;

	mutex_lock(&gc0310_mutex);

	while (pdata < pend && i < IMGSENSOR_I2C_CMD_LENGTH_MAX) {
		pmsg[i].addr = id >> 1;
		pmsg[i].flags = 0;
		pmsg[i].len = write_per_cycle;
		pmsg[i].buf = pdata;
		pdata += write_per_cycle;
		i++;
	}

	if (mtk_i2c_transfer(pinst->pi2c_client->adapter, pinst->msg, i, 0,
			     ((speed > 0) && (speed <= 1000))
				? speed * 1000 : GC0310MIPI_I2C_SPEED * 1000)
			!= i) {
		pr_err("--lijian--I2C write failed (0x%x)! speed(0=%d)\n",
		       ret, speed);
		ret = -1;
	}

	mutex_unlock(&gc0310_mutex);

	return ret;
}

int gc0310_i2c_read(u8 *pwrite_data, u16 write_length, u8 *pread_data,
		    u16 read_length, u16 id, int speed)
{
	struct IMGSENSOR_I2C_INST *pinst = GC0310MIPI_INST;
	int ret = 0;

	mutex_lock(&gc0310_mutex);

	pinst->msg[0].addr = id >> 1;
	pinst->msg[0].flags = 0;
	pinst->msg[0].len = write_length;
	pinst->msg[0].buf = pwrite_data;

	pinst->msg[1].addr = id >> 1;
	pinst->msg[1].flags = I2C_M_RD;
	pinst->msg[1].len = read_length;
	pinst->msg[1].buf = pread_data;

	if (mtk_i2c_transfer(pinst->pi2c_client->adapter, pinst->msg, 2, 0,
			     ((speed > 0) && (speed <= 1000))
				? speed * 1000 : GC0310MIPI_I2C_SPEED * 1000)
			!= 2) {
		pr_err("--lijian--I2C read failed (0x%x)! speed(0=%d)\n",
		       ret, speed);
		ret = -1;
	}

	mutex_unlock(&gc0310_mutex);

	return ret;
}

/*
 * The shutter lives in two registers and is put back together with a graft:
 * "331d7113 bfi"@0xffffff800870ae20 puts the bits of 0x03 above the byte of
 * 0x04. The first is read as a byte, the second as a half-word -- the two
 * loads, `ldrb` and `ldrh`, say so.
 */
kal_uint16 GC0310MIPI_Read_Shutter(void)
{
	kal_uint8 temp_reg1;
	kal_uint16 temp_reg2;

	temp_reg1 = GC0310MIPI_read_cmos_sensor_(0x04);
	temp_reg2 = GC0310MIPI_read_cmos_sensor_(0x03);

	return (temp_reg2 << 8) | temp_reg1;
}

void GC0310MIPI_write_reg(kal_uint8 addr, kal_uint8 para)
{
	GC0310MIPI_write_cmos_sensor(addr, para);
}

kal_uint16 GC0310MIPI_read_cmos_sensor(kal_uint8 addr)
{
	return GC0310MIPI_read_cmos_sensor_(addr);
}

kal_uint16 GC0310MIPI_read_reg(kal_uint8 addr)
{
	return GC0310MIPI_read_cmos_sensor_(addr);
}

/*
 * GC0310MIPI_Sensor_Init() was reconstructed from the factory kernel disassembly (0xffffff800870e994).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void GC0310MIPI_Sensor_Init(void)
{
	GC0310MIPI_write_cmos_sensor(0xfe, 0xf0);
	GC0310MIPI_write_cmos_sensor(0xfe, 0xf0);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0xfc, 0x0e);
	GC0310MIPI_write_cmos_sensor(0xfc, 0x0e);
	GC0310MIPI_write_cmos_sensor(0xf2, 0x80);
	GC0310MIPI_write_cmos_sensor(0xf3, 0x00);
	GC0310MIPI_write_cmos_sensor(0xf7, 0x1b);
	GC0310MIPI_write_cmos_sensor(0xf8, 0x04);
	GC0310MIPI_write_cmos_sensor(0xf9, 0x8e);
	GC0310MIPI_write_cmos_sensor(0xfa, 0x11);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x03);
	GC0310MIPI_write_cmos_sensor(0x40, 0x08);
	GC0310MIPI_write_cmos_sensor(0x42, 0x00);
	GC0310MIPI_write_cmos_sensor(0x43, 0x00);
	GC0310MIPI_write_cmos_sensor(0x01, 0x03);
	GC0310MIPI_write_cmos_sensor(0x10, 0x84);
	GC0310MIPI_write_cmos_sensor(0x01, 0x03);
	GC0310MIPI_write_cmos_sensor(0x02, 0x33);
	GC0310MIPI_write_cmos_sensor(0x03, 0x94);
	GC0310MIPI_write_cmos_sensor(0x04, 0x01);
	GC0310MIPI_write_cmos_sensor(0x05, 0x00);
	GC0310MIPI_write_cmos_sensor(0x06, 0x80);
	GC0310MIPI_write_cmos_sensor(0x11, 0x1e);
	GC0310MIPI_write_cmos_sensor(0x12, 0x00);
	GC0310MIPI_write_cmos_sensor(0x13, 0x05);
	GC0310MIPI_write_cmos_sensor(0x15, 0x10);
	GC0310MIPI_write_cmos_sensor(0x21, 0x10);
	GC0310MIPI_write_cmos_sensor(0x22, 0x01);
	GC0310MIPI_write_cmos_sensor(0x23, 0x10);
	GC0310MIPI_write_cmos_sensor(0x24, 0x02);
	GC0310MIPI_write_cmos_sensor(0x25, 0x10);
	GC0310MIPI_write_cmos_sensor(0x26, 0x03);
	GC0310MIPI_write_cmos_sensor(0x29, 0x06);
	GC0310MIPI_write_cmos_sensor(0x2a, 0x0a);
	GC0310MIPI_write_cmos_sensor(0x2b, 0x06);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0x00, 0x2f);
	GC0310MIPI_write_cmos_sensor(0x01, 0x0f);
	GC0310MIPI_write_cmos_sensor(0x02, 0x04);
	GC0310MIPI_write_cmos_sensor(0x03, 0x03);
	GC0310MIPI_write_cmos_sensor(0x04, 0x50);
	GC0310MIPI_write_cmos_sensor(0x09, 0x00);
	GC0310MIPI_write_cmos_sensor(0x0a, 0x00);
	GC0310MIPI_write_cmos_sensor(0x0b, 0x00);
	GC0310MIPI_write_cmos_sensor(0x0c, 0x04);
	GC0310MIPI_write_cmos_sensor(0x0d, 0x01);
	GC0310MIPI_write_cmos_sensor(0x0e, 0xe8);
	GC0310MIPI_write_cmos_sensor(0x0f, 0x02);
	GC0310MIPI_write_cmos_sensor(0x10, 0x88);
	GC0310MIPI_write_cmos_sensor(0x16, 0x00);
	GC0310MIPI_write_cmos_sensor(0x17, 0x14);
	GC0310MIPI_write_cmos_sensor(0x18, 0x1a);
	GC0310MIPI_write_cmos_sensor(0x19, 0x14);
	GC0310MIPI_write_cmos_sensor(0x1b, 0x48);
	GC0310MIPI_write_cmos_sensor(0x1c, 0x1c);
	GC0310MIPI_write_cmos_sensor(0x1e, 0x6b);
	GC0310MIPI_write_cmos_sensor(0x1f, 0x28);
	GC0310MIPI_write_cmos_sensor(0x20, 0x8b);
	GC0310MIPI_write_cmos_sensor(0x21, 0x49);
	GC0310MIPI_write_cmos_sensor(0x22, 0xb0);
	GC0310MIPI_write_cmos_sensor(0x23, 0x04);
	GC0310MIPI_write_cmos_sensor(0x24, 0x16);
	GC0310MIPI_write_cmos_sensor(0x34, 0x20);
	GC0310MIPI_write_cmos_sensor(0x26, 0x23);
	GC0310MIPI_write_cmos_sensor(0x28, 0xff);
	GC0310MIPI_write_cmos_sensor(0x29, 0x00);
	GC0310MIPI_write_cmos_sensor(0x32, 0x00);
	GC0310MIPI_write_cmos_sensor(0x33, 0x10);
	GC0310MIPI_write_cmos_sensor(0x37, 0x20);
	GC0310MIPI_write_cmos_sensor(0x38, 0x10);
	GC0310MIPI_write_cmos_sensor(0x47, 0x80);
	GC0310MIPI_write_cmos_sensor(0x4e, 0x66);
	GC0310MIPI_write_cmos_sensor(0xa8, 0x02);
	GC0310MIPI_write_cmos_sensor(0xa9, 0x80);
	GC0310MIPI_write_cmos_sensor(0x40, 0xff);
	GC0310MIPI_write_cmos_sensor(0x41, 0x21);
	GC0310MIPI_write_cmos_sensor(0x42, 0xcf);
	GC0310MIPI_write_cmos_sensor(0x44, 0x02);
	GC0310MIPI_write_cmos_sensor(0x45, 0xa8);
	GC0310MIPI_write_cmos_sensor(0x46, 0x02);
	GC0310MIPI_write_cmos_sensor(0x4a, 0x11);
	GC0310MIPI_write_cmos_sensor(0x4b, 0x01);
	GC0310MIPI_write_cmos_sensor(0x4c, 0x20);
	GC0310MIPI_write_cmos_sensor(0x4d, 0x05);
	GC0310MIPI_write_cmos_sensor(0x4f, 0x01);
	GC0310MIPI_write_cmos_sensor(0x50, 0x01);
	GC0310MIPI_write_cmos_sensor(0x55, 0x01);
	GC0310MIPI_write_cmos_sensor(0x56, 0xe0);
	GC0310MIPI_write_cmos_sensor(0x57, 0x02);
	GC0310MIPI_write_cmos_sensor(0x58, 0x80);
	GC0310MIPI_write_cmos_sensor(0x70, 0x70);
	GC0310MIPI_write_cmos_sensor(0x5a, 0x84);
	GC0310MIPI_write_cmos_sensor(0x5b, 0xc9);
	GC0310MIPI_write_cmos_sensor(0x5c, 0xed);
	GC0310MIPI_write_cmos_sensor(0x77, 0x74);
	GC0310MIPI_write_cmos_sensor(0x78, 0x40);
	GC0310MIPI_write_cmos_sensor(0x79, 0x5f);
	GC0310MIPI_write_cmos_sensor(0x82, 0x14);
	GC0310MIPI_write_cmos_sensor(0x83, 0x0b);
	GC0310MIPI_write_cmos_sensor(0x89, 0xf0);
	GC0310MIPI_write_cmos_sensor(0x8f, 0xaa);
	GC0310MIPI_write_cmos_sensor(0x90, 0x8c);
	GC0310MIPI_write_cmos_sensor(0x91, 0x90);
	GC0310MIPI_write_cmos_sensor(0x92, 0x03);
	GC0310MIPI_write_cmos_sensor(0x93, 0x03);
	GC0310MIPI_write_cmos_sensor(0x94, 0x05);
	GC0310MIPI_write_cmos_sensor(0x95, 0x65);
	GC0310MIPI_write_cmos_sensor(0x96, 0xf0);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0x9a, 0x20);
	GC0310MIPI_write_cmos_sensor(0x9b, 0x80);
	GC0310MIPI_write_cmos_sensor(0x9c, 0x40);
	GC0310MIPI_write_cmos_sensor(0x9d, 0x80);
	GC0310MIPI_write_cmos_sensor(0xa1, 0x30);
	GC0310MIPI_write_cmos_sensor(0xa2, 0x32);
	GC0310MIPI_write_cmos_sensor(0xa4, 0x30);
	GC0310MIPI_write_cmos_sensor(0xa5, 0x30);
	GC0310MIPI_write_cmos_sensor(0xaa, 0x10);
	GC0310MIPI_write_cmos_sensor(0xac, 0x22);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0xbf, 0x08);
	GC0310MIPI_write_cmos_sensor(0xc0, 0x16);
	GC0310MIPI_write_cmos_sensor(0xc1, 0x28);
	GC0310MIPI_write_cmos_sensor(0xc2, 0x41);
	GC0310MIPI_write_cmos_sensor(0xc3, 0x5a);
	GC0310MIPI_write_cmos_sensor(0xc4, 0x6c);
	GC0310MIPI_write_cmos_sensor(0xc5, 0x7a);
	GC0310MIPI_write_cmos_sensor(0xc6, 0x96);
	GC0310MIPI_write_cmos_sensor(0xc7, 0xac);
	GC0310MIPI_write_cmos_sensor(0xc8, 0xbc);
	GC0310MIPI_write_cmos_sensor(0xc9, 0xc9);
	GC0310MIPI_write_cmos_sensor(0xca, 0xd3);
	GC0310MIPI_write_cmos_sensor(0xcb, 0xdd);
	GC0310MIPI_write_cmos_sensor(0xcc, 0xe5);
	GC0310MIPI_write_cmos_sensor(0xcd, 0xf1);
	GC0310MIPI_write_cmos_sensor(0xce, 0xfa);
	GC0310MIPI_write_cmos_sensor(0xcf, 0xff);
	GC0310MIPI_write_cmos_sensor(0xd0, 0x40);
	GC0310MIPI_write_cmos_sensor(0xd1, 0x34);
	GC0310MIPI_write_cmos_sensor(0xd2, 0x34);
	GC0310MIPI_write_cmos_sensor(0xd3, 0x40);
	GC0310MIPI_write_cmos_sensor(0xd6, 0xf2);
	GC0310MIPI_write_cmos_sensor(0xd7, 0x1b);
	GC0310MIPI_write_cmos_sensor(0xd8, 0x18);
	GC0310MIPI_write_cmos_sensor(0xdd, 0x03);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0x05, 0x30);
	GC0310MIPI_write_cmos_sensor(0x06, 0x75);
	GC0310MIPI_write_cmos_sensor(0x07, 0x40);
	GC0310MIPI_write_cmos_sensor(0x08, 0xb0);
	GC0310MIPI_write_cmos_sensor(0x0a, 0xc5);
	GC0310MIPI_write_cmos_sensor(0x0b, 0x11);
	GC0310MIPI_write_cmos_sensor(0x0c, 0x00);
	GC0310MIPI_write_cmos_sensor(0x12, 0x52);
	GC0310MIPI_write_cmos_sensor(0x13, 0x38);
	GC0310MIPI_write_cmos_sensor(0x18, 0x95);
	GC0310MIPI_write_cmos_sensor(0x19, 0x96);
	GC0310MIPI_write_cmos_sensor(0x1f, 0x20);
	GC0310MIPI_write_cmos_sensor(0x20, 0xc0);
	GC0310MIPI_write_cmos_sensor(0x3e, 0x40);
	GC0310MIPI_write_cmos_sensor(0x3f, 0x57);
	GC0310MIPI_write_cmos_sensor(0x40, 0x7d);
	GC0310MIPI_write_cmos_sensor(0x03, 0x60);
	GC0310MIPI_write_cmos_sensor(0x44, 0x02);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0x1c, 0x91);
	GC0310MIPI_write_cmos_sensor(0x21, 0x15);
	GC0310MIPI_write_cmos_sensor(0x50, 0x80);
	GC0310MIPI_write_cmos_sensor(0x56, 0x04);
	GC0310MIPI_write_cmos_sensor(0x59, 0x08);
	GC0310MIPI_write_cmos_sensor(0x5b, 0x02);
	GC0310MIPI_write_cmos_sensor(0x61, 0x8d);
	GC0310MIPI_write_cmos_sensor(0x62, 0xa7);
	GC0310MIPI_write_cmos_sensor(0x63, 0xd0);
	GC0310MIPI_write_cmos_sensor(0x65, 0x06);
	GC0310MIPI_write_cmos_sensor(0x66, 0x06);
	GC0310MIPI_write_cmos_sensor(0x67, 0x84);
	GC0310MIPI_write_cmos_sensor(0x69, 0x08);
	GC0310MIPI_write_cmos_sensor(0x6a, 0x25);
	GC0310MIPI_write_cmos_sensor(0x6b, 0x01);
	GC0310MIPI_write_cmos_sensor(0x6c, 0x00);
	GC0310MIPI_write_cmos_sensor(0x6d, 0x02);
	GC0310MIPI_write_cmos_sensor(0x6e, 0xf0);
	GC0310MIPI_write_cmos_sensor(0x6f, 0x80);
	GC0310MIPI_write_cmos_sensor(0x76, 0x80);
	GC0310MIPI_write_cmos_sensor(0x78, 0xaf);
	GC0310MIPI_write_cmos_sensor(0x79, 0x75);
	GC0310MIPI_write_cmos_sensor(0x7a, 0x40);
	GC0310MIPI_write_cmos_sensor(0x7b, 0x50);
	GC0310MIPI_write_cmos_sensor(0x7c, 0x0c);
	GC0310MIPI_write_cmos_sensor(0x90, 0xc9);
	GC0310MIPI_write_cmos_sensor(0x91, 0xbe);
	GC0310MIPI_write_cmos_sensor(0x92, 0xe2);
	GC0310MIPI_write_cmos_sensor(0x93, 0xc9);
	GC0310MIPI_write_cmos_sensor(0x95, 0x1b);
	GC0310MIPI_write_cmos_sensor(0x96, 0xe2);
	GC0310MIPI_write_cmos_sensor(0x97, 0x49);
	GC0310MIPI_write_cmos_sensor(0x98, 0x1b);
	GC0310MIPI_write_cmos_sensor(0x9a, 0x49);
	GC0310MIPI_write_cmos_sensor(0x9b, 0x1b);
	GC0310MIPI_write_cmos_sensor(0x9c, 0xc3);
	GC0310MIPI_write_cmos_sensor(0x9d, 0x49);
	GC0310MIPI_write_cmos_sensor(0x9f, 0xc7);
	GC0310MIPI_write_cmos_sensor(0xa0, 0xc8);
	GC0310MIPI_write_cmos_sensor(0xa1, 0x00);
	GC0310MIPI_write_cmos_sensor(0xa2, 0x00);
	GC0310MIPI_write_cmos_sensor(0x86, 0x00);
	GC0310MIPI_write_cmos_sensor(0x87, 0x00);
	GC0310MIPI_write_cmos_sensor(0x88, 0x00);
	GC0310MIPI_write_cmos_sensor(0x89, 0x00);
	GC0310MIPI_write_cmos_sensor(0xa4, 0xb9);
	GC0310MIPI_write_cmos_sensor(0xa5, 0xa0);
	GC0310MIPI_write_cmos_sensor(0xa6, 0xba);
	GC0310MIPI_write_cmos_sensor(0xa7, 0x92);
	GC0310MIPI_write_cmos_sensor(0xa9, 0xba);
	GC0310MIPI_write_cmos_sensor(0xaa, 0x80);
	GC0310MIPI_write_cmos_sensor(0xab, 0x9d);
	GC0310MIPI_write_cmos_sensor(0xac, 0x7f);
	GC0310MIPI_write_cmos_sensor(0xae, 0xbb);
	GC0310MIPI_write_cmos_sensor(0xaf, 0x9d);
	GC0310MIPI_write_cmos_sensor(0xb0, 0xc8);
	GC0310MIPI_write_cmos_sensor(0xb1, 0x97);
	GC0310MIPI_write_cmos_sensor(0xb3, 0xb7);
	GC0310MIPI_write_cmos_sensor(0xb4, 0x7f);
	GC0310MIPI_write_cmos_sensor(0xb5, 0x00);
	GC0310MIPI_write_cmos_sensor(0xb6, 0x00);
	GC0310MIPI_write_cmos_sensor(0x8b, 0x00);
	GC0310MIPI_write_cmos_sensor(0x8c, 0x00);
	GC0310MIPI_write_cmos_sensor(0x8d, 0x00);
	GC0310MIPI_write_cmos_sensor(0x8e, 0x00);
	GC0310MIPI_write_cmos_sensor(0x94, 0x55);
	GC0310MIPI_write_cmos_sensor(0x99, 0xa6);
	GC0310MIPI_write_cmos_sensor(0x9e, 0xaa);
	GC0310MIPI_write_cmos_sensor(0xa3, 0x0a);
	GC0310MIPI_write_cmos_sensor(0x8a, 0x00);
	GC0310MIPI_write_cmos_sensor(0xa8, 0x55);
	GC0310MIPI_write_cmos_sensor(0xad, 0x55);
	GC0310MIPI_write_cmos_sensor(0xb2, 0x55);
	GC0310MIPI_write_cmos_sensor(0xb7, 0x05);
	GC0310MIPI_write_cmos_sensor(0x8f, 0x00);
	GC0310MIPI_write_cmos_sensor(0xb8, 0xcb);
	GC0310MIPI_write_cmos_sensor(0xb9, 0x9b);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0xd0, 0x38);
	GC0310MIPI_write_cmos_sensor(0xd1, 0x00);
	GC0310MIPI_write_cmos_sensor(0xd2, 0x02);
	GC0310MIPI_write_cmos_sensor(0xd3, 0x04);
	GC0310MIPI_write_cmos_sensor(0xd4, 0x38);
	GC0310MIPI_write_cmos_sensor(0xd5, 0x12);
	GC0310MIPI_write_cmos_sensor(0xd6, 0x30);
	GC0310MIPI_write_cmos_sensor(0xd7, 0x00);
	GC0310MIPI_write_cmos_sensor(0xd8, 0x0a);
	GC0310MIPI_write_cmos_sensor(0xd9, 0x16);
	GC0310MIPI_write_cmos_sensor(0xda, 0x39);
	GC0310MIPI_write_cmos_sensor(0xdb, 0xf8);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0xc1, 0x3c);
	GC0310MIPI_write_cmos_sensor(0xc2, 0x50);
	GC0310MIPI_write_cmos_sensor(0xc3, 0x00);
	GC0310MIPI_write_cmos_sensor(0xc4, 0x40);
	GC0310MIPI_write_cmos_sensor(0xc5, 0x30);
	GC0310MIPI_write_cmos_sensor(0xc6, 0x30);
	GC0310MIPI_write_cmos_sensor(0xc7, 0x10);
	GC0310MIPI_write_cmos_sensor(0xc8, 0x00);
	GC0310MIPI_write_cmos_sensor(0xc9, 0x00);
	GC0310MIPI_write_cmos_sensor(0xdc, 0x20);
	GC0310MIPI_write_cmos_sensor(0xdd, 0x10);
	GC0310MIPI_write_cmos_sensor(0xdf, 0x00);
	GC0310MIPI_write_cmos_sensor(0xde, 0x00);
	GC0310MIPI_write_cmos_sensor(0x01, 0x10);
	GC0310MIPI_write_cmos_sensor(0x0b, 0x31);
	GC0310MIPI_write_cmos_sensor(0x0e, 0x50);
	GC0310MIPI_write_cmos_sensor(0x0f, 0x0f);
	GC0310MIPI_write_cmos_sensor(0x10, 0x6e);
	GC0310MIPI_write_cmos_sensor(0x12, 0xa0);
	GC0310MIPI_write_cmos_sensor(0x15, 0x60);
	GC0310MIPI_write_cmos_sensor(0x16, 0x60);
	GC0310MIPI_write_cmos_sensor(0x17, 0xe0);
	GC0310MIPI_write_cmos_sensor(0xcc, 0x0c);
	GC0310MIPI_write_cmos_sensor(0xcd, 0x10);
	GC0310MIPI_write_cmos_sensor(0xce, 0xa0);
	GC0310MIPI_write_cmos_sensor(0xcf, 0xe6);
	GC0310MIPI_write_cmos_sensor(0x45, 0xf7);
	GC0310MIPI_write_cmos_sensor(0x46, 0xff);
	GC0310MIPI_write_cmos_sensor(0x47, 0x15);
	GC0310MIPI_write_cmos_sensor(0x48, 0x03);
	GC0310MIPI_write_cmos_sensor(0x4f, 0x60);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0x05, 0x02);
	GC0310MIPI_write_cmos_sensor(0x06, 0xd1);
	GC0310MIPI_write_cmos_sensor(0x07, 0x00);
	GC0310MIPI_write_cmos_sensor(0x08, 0x22);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0x25, 0x00);
	GC0310MIPI_write_cmos_sensor(0x26, 0x6a);
	GC0310MIPI_write_cmos_sensor(0x27, 0x02);
	GC0310MIPI_write_cmos_sensor(0x28, 0x12);
	GC0310MIPI_write_cmos_sensor(0x29, 0x03);
	GC0310MIPI_write_cmos_sensor(0x2a, 0x50);
	GC0310MIPI_write_cmos_sensor(0x2b, 0x05);
	GC0310MIPI_write_cmos_sensor(0x2c, 0xcc);
	GC0310MIPI_write_cmos_sensor(0x2d, 0x07);
	GC0310MIPI_write_cmos_sensor(0x2e, 0x74);
	GC0310MIPI_write_cmos_sensor(0x3c, 0x20);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x03);
	GC0310MIPI_write_cmos_sensor(0x10, 0x94);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
}

/*
 * GC0310MIPIGammaSelect() was reconstructed from the factory kernel disassembly (0xffffff800870aff0, 144 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void GC0310MIPIGammaSelect(kal_uint8 index)
{
	switch (index) {
	case 0:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0xbf, 0x06);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x12);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x22);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x35);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x4b);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x5f);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x72);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x8d);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xa4);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xb8);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xc8);
		GC0310MIPI_write_cmos_sensor(0xca, 0xd4);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xde);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xe6);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xf1);
		GC0310MIPI_write_cmos_sensor(0xce, 0xf8);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0xbf, 0x0b);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x17);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x2a);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x41);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x54);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x66);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x74);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x8c);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xa3);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xb5);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xc4);
		GC0310MIPI_write_cmos_sensor(0xca, 0xd0);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xdb);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xe5);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xf0);
		GC0310MIPI_write_cmos_sensor(0xce, 0xf7);
		GC0310MIPI_write_cmos_sensor(0xcf, 0xff);
		break;
	case 2:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x0b);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x17);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x2a);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x41);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x54);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x66);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x74);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x8c);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xa3);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xb5);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xc4);
		GC0310MIPI_write_cmos_sensor(0xca, 0xd0);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xdb);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xe5);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xf0);
		break;
	case 3:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x0e);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x1c);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x34);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x48);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x5a);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x6b);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x7b);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x95);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xab);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xbf);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xce);
		GC0310MIPI_write_cmos_sensor(0xca, 0xd9);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xe4);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xec);
		break;
	case 4:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x10);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x20);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x38);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x4e);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x63);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x76);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x87);
		GC0310MIPI_write_cmos_sensor(0xc6, 0xa2);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xb8);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xca);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xd8);
		GC0310MIPI_write_cmos_sensor(0xca, 0xe3);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xeb);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xf0);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xf8);
		break;
	case 1:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x08);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x0f);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x21);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x32);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x43);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x50);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x5e);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x78);
		GC0310MIPI_write_cmos_sensor(0xc7, 0x90);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xa6);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xb9);
		GC0310MIPI_write_cmos_sensor(0xca, 0xc9);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xd6);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xe0);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xee);
		break;
	case 5:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x14);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x28);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x44);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x5d);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x72);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x86);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x95);
		GC0310MIPI_write_cmos_sensor(0xc6, 0xb1);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xc6);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xd5);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xe1);
		GC0310MIPI_write_cmos_sensor(0xca, 0xea);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xf1);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xf5);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xfb);
		break;
	case 6:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x0b);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x16);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x29);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x3c);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x4f);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x5f);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x6f);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x8a);
		GC0310MIPI_write_cmos_sensor(0xc7, 0x9f);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xb4);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xc6);
		GC0310MIPI_write_cmos_sensor(0xca, 0xd3);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xdd);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xe5);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xf1);
		GC0310MIPI_write_cmos_sensor(0xce, 0xfa);
		GC0310MIPI_write_cmos_sensor(0xcf, 0xff);
		break;
	default:
		break;
	}
}


/*
 * FOUR FUNCTIONS EMPTY IN THE FACTORY. They are not stubs: in the binary
 * their whole body is `ret`, four bytes, and stock.map lists them at that
 * size. Reproducing them empty is what rule 7 requires.
 */
void GC0310MIPI_Set_Shutter(kal_uint16 iShutter)
{
}

void GC0310MIPI_config_window(kal_uint16 startx, kal_uint16 starty,
			      kal_uint16 width, kal_uint16 height)
{
}

void GC0310MIPI_SetGain(kal_uint16 iGain)
{
}

void GC0310MIPI_Write_More_Registers(void)
{
}

UINT32 GC0310MIPIClose(void)
{
	return ERROR_NONE;
}

UINT32 GC0310MIPIGetResolution(
	MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	pSensorResolution->SensorPreviewWidth = GC0310MIPI_IMAGE_SENSOR_PV_WIDTH;
	pSensorResolution->SensorPreviewHeight = GC0310MIPI_IMAGE_SENSOR_PV_HEIGHT;
	pSensorResolution->SensorFullWidth = GC0310MIPI_IMAGE_SENSOR_FULL_WIDTH;
	pSensorResolution->SensorFullHeight = GC0310MIPI_IMAGE_SENSOR_FULL_HEIGHT;
	pSensorResolution->SensorVideoWidth = GC0310MIPI_IMAGE_SENSOR_VIDEO_WIDTH;
	pSensorResolution->SensorVideoHeight = GC0310MIPI_IMAGE_SENSOR_VIDEO_HEIGHT;

	return ERROR_NONE;
}

/*
 * THE WINDOW IS ALWAYS THE SAME, and it starts from line 1: the factory
 * builds 0x01DA_0278_0001_0000 in a single register and writes it with an
 * eight-byte store, that is {0, 1, 632, 474}.
 */
UINT32 GC0310MIPIPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
			 MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	g9c95930 = (pSensorConfigData->SensorOperationMode == 1)
			? KAL_TRUE : KAL_FALSE;

	pImageWindow->GrabStartX = 0;
	pImageWindow->GrabStartY = 1;
	pImageWindow->ExposureWindowWidth = GC0310MIPI_IMAGE_SENSOR_PV_WIDTH;
	pImageWindow->ExposureWindowHeight = GC0310MIPI_IMAGE_SENSOR_PV_HEIGHT;

	memcpy(&g9c95944, pSensorConfigData,
	       sizeof(MSDK_SENSOR_CONFIG_STRUCT));

	return ERROR_NONE;
}

UINT32 GC0310MIPICapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
			 MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	g9c95938 = KAL_TRUE;

	pImageWindow->GrabStartX = 0;
	pImageWindow->GrabStartY = 1;
	pImageWindow->ExposureWindowWidth = GC0310MIPI_IMAGE_SENSOR_FULL_WIDTH;
	pImageWindow->ExposureWindowHeight = GC0310MIPI_IMAGE_SENSOR_FULL_HEIGHT;

	memcpy(&g9c95944, pSensorConfigData,
	       sizeof(MSDK_SENSOR_CONFIG_STRUCT));

	return ERROR_NONE;
}

/*
 * Control does NOT look at the scenario: it just calls Preview, and returns
 * TRUE instead of ERROR_NONE. In the binary there is no comparison on x0, and
 * the 21 instructions that follow are those of Preview inlined.
 */
UINT32 GC0310MIPIControl(enum MSDK_SCENARIO_ID_ENUM ScenarioId,
			 MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
			 MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	GC0310MIPIPreview(pImageWindow, pSensorConfigData);

	return TRUE;
}

/*
 * Fifteen or thirty frames per second, everything else is a message and
 * nothing more: the return value is TRUE even when the rate is
 * wrong.
 */
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008716f04).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
/*
 * It reads 0x42, raises or lowers bit 1, writes it back. The binary tests bit 0
 * of the argument ("36000073 tbz"), not the whole argument: that is what a
 * kal_bool passed to an `if` does.
 */
static void GC0310MIPI_awb_enable(kal_bool enable)
{
	kal_uint16 temp_AWB_Reg = GC0310MIPI_read_cmos_sensor_(0x42);

	if (enable)
		temp_AWB_Reg |= 0x02;
	else
		temp_AWB_Reg &= ~0x02;

	GC0310MIPI_write_cmos_sensor(0x42, temp_AWB_Reg);
}

/*
 * GC0310MIPI_set_param_wb() was reconstructed from the factory kernel disassembly (0xffffff8008f49e8a).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
UINT32 GC0310MIPI_set_param_wb(UINT16 para)
{
	if (para > AWB_MODE_TUNGSTEN)
		return FALSE;

	switch (para) {
	case AWB_MODE_OFF:
		break;

	case AWB_MODE_AUTO:
		GC0310MIPI_awb_enable(KAL_TRUE);
		break;

	case AWB_MODE_DAYLIGHT:
		GC0310MIPI_awb_enable(KAL_FALSE);
		GC0310MIPI_write_cmos_sensor(0x77, 0x74);
		GC0310MIPI_write_cmos_sensor(0x78, 0x52);
		GC0310MIPI_write_cmos_sensor(0x79, 0x40);
		break;

	case AWB_MODE_CLOUDY_DAYLIGHT:
		GC0310MIPI_awb_enable(KAL_FALSE);
		GC0310MIPI_write_cmos_sensor(0x77, 0x8c);
		GC0310MIPI_write_cmos_sensor(0x78, 0x50);
		GC0310MIPI_write_cmos_sensor(0x79, 0x40);
		break;

	case AWB_MODE_FLUORESCENT:
		GC0310MIPI_awb_enable(KAL_FALSE);
		GC0310MIPI_write_cmos_sensor(0x77, 0x40);
		GC0310MIPI_write_cmos_sensor(0x78, 0x42);
		GC0310MIPI_write_cmos_sensor(0x79, 0x50);
		break;

	case AWB_MODE_INCANDESCENT:
		GC0310MIPI_awb_enable(KAL_FALSE);
		GC0310MIPI_write_cmos_sensor(0x77, 0x48);
		GC0310MIPI_write_cmos_sensor(0x78, 0x40);
		GC0310MIPI_write_cmos_sensor(0x79, 0x5c);
		break;

	case AWB_MODE_TUNGSTEN:
		GC0310MIPI_awb_enable(KAL_FALSE);
		GC0310MIPI_write_cmos_sensor(0x77, 0x40);
		GC0310MIPI_write_cmos_sensor(0x78, 0x54);
		GC0310MIPI_write_cmos_sensor(0x79, 0x70);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

/*
 * EFFECTS: eleven cases, five of which fall into the default.
 * Table at 0xffffff8008f49e9e.
 */
UINT32 GC0310MIPI_set_param_effect(UINT16 para)
{
	if (para > MEFFECT_SEPIABLUE)
		return FALSE;

	switch (para) {
	case MEFFECT_OFF:
		GC0310MIPI_write_cmos_sensor(0x43, 0x00);
		break;

	case MEFFECT_MONO:
		GC0310MIPI_write_cmos_sensor(0x43, 0x02);
		GC0310MIPI_write_cmos_sensor(0xda, 0x00);
		break;

	case MEFFECT_SEPIA:
		GC0310MIPI_write_cmos_sensor(0x43, 0x02);
		GC0310MIPI_write_cmos_sensor(0xda, 0xd0);
		GC0310MIPI_write_cmos_sensor(0xdb, 0x28);
		break;

	case MEFFECT_NEGATIVE:
		GC0310MIPI_write_cmos_sensor(0x43, 0x01);
		break;

	case MEFFECT_SEPIAGREEN:
		GC0310MIPI_write_cmos_sensor(0x43, 0x02);
		GC0310MIPI_write_cmos_sensor(0xda, 0xc0);
		GC0310MIPI_write_cmos_sensor(0xdb, 0xc0);
		break;

	case MEFFECT_SEPIABLUE:
		GC0310MIPI_write_cmos_sensor(0x43, 0x02);
		GC0310MIPI_write_cmos_sensor(0xda, 0x50);
		GC0310MIPI_write_cmos_sensor(0xdb, 0xe0);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

UINT32 GC0310MIPIGetSensorID(UINT32 *sensorID)
{
	kal_uint8 retry = 30;

	if (g9c90244 != 3) {
		*sensorID = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}

	do {
		*sensorID = (GC0310MIPI_read_cmos_sensor_(0xf0) << 8)
			  | GC0310MIPI_read_cmos_sensor_(0xf1);
		if (*sensorID == GC0310_SENSOR_ID)
			break;
		printk("Read Sensor ID Fail = 0x%04x\n", *sensorID);
		mdelay(1000);
	} while (--retry);

	printk("---GC0310MIPIGetSensorID--*sensorID==%d--\n", *sensorID);

	if (*sensorID != GC0310_SENSOR_ID) {
		*sensorID = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}

	return ERROR_NONE;
}

/*
 * GC0310MIPIOpen() was reconstructed from the factory kernel disassembly (0xffffff80087172c4).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
UINT32 GC0310MIPIOpen(void)
{
	kal_int8 i = 0;
	kal_uint16 sensor_id = 0;

	printk("<Jet> Entry GC0310MIPIOpen!!!\r\n");

	if (g9c90244 != 3)
		return ERROR_SENSOR_CONNECT_FAIL;

	mdelay(10);

	for (i = 0; i < 3; i++) {
		sensor_id = (GC0310MIPI_read_cmos_sensor_(0xf0) << 8)
			  | GC0310MIPI_read_cmos_sensor_(0xf1);
		if (sensor_id != GC0310_SENSOR_ID) {
			printk("GC0310MIPI Read Sensor ID Fail[open] = 0x%x\n",
			       sensor_id);
			return ERROR_SENSOR_CONNECT_FAIL;
		}
	}

	printk("GC0310MIPI_ Sensor Read ID OK \r\n");

	GC0310MIPI_Sensor_Init();

	return ERROR_NONE;
}

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
/*
 * THREE WRITES, AND THE SECOND DOES NOT GO TO 0xFE. The two-byte
 * buffer holds the address in [sp,#4] and the datum in [sp,#5]: the second
 * write changes the address TO 0x3C before the branch ("52800788 orr"@0xffffff800870e78c
 * followed by `strb w8, [sp,#4]`) and then only the datum in the two branches. I had
 * read the second strb and not the first, and the function was 216 bytes short.
 *
 * And at the tail it leaves the argument value in a global.
 */
void GC0310MIPINightMode(kal_bool enable)
{
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);

	if (enable) {
		GC0310MIPI_write_cmos_sensor(0x3c, 0x30);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		g9c9593c = 1;
	} else {
		GC0310MIPI_write_cmos_sensor(0x3c, 0x20);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		g9c9593c = 0;
	}
}

/*
 * GC0310ReadBV() was reconstructed from the factory kernel disassembly (0xffffff800871aa54).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
UINT32 GC0310ReadBV(void)
{
	kal_uint8 temp;

	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0x21, 0x16);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);

	temp = GC0310MIPI_read_cmos_sensor_(0xef);

	GC0310MIPI_read_cmos_sensor_(0x48);
	GC0310MIPI_read_cmos_sensor_(0xf0);
	GC0310MIPI_read_cmos_sensor_(0xf1);

	return temp;
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800871a4b0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
/*
 * GC0310MIPI_set_param_exposure() was reconstructed from the factory kernel disassembly (0xffffff8008718f7c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc0310_mipi_yuv_gc0310mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
UINT32 GC0310MIPI_set_param_exposure(UINT16 para)
{
	switch (para) {
	case AE_EV_COMP_00:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x38);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_10:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x40);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_20:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x48);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_30:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x50);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_n10:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x30);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_n20:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x28);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_n30:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x20);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	default:
		break;
	}

	return TRUE;
}

/*
 * ONLY TWO MAINS FREQUENCIES, and the rest does nothing: "7100051f cmp"
 * compares with 1 and the other branch returns without writing. Seventeen
 * writes per side, and the last two are shared -- the factory emits 15 + 17
 * because the 50 Hz branch jumps into the tail of the other after putting
 * its own value for 0x2E, which is 0x74 against 0x40.
 */
UINT32 GC0310MIPI_set_param_banding(UINT16 para)
{
	switch (para) {
	case AE_FLICKER_MODE_50HZ:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0x05, 0x02);
		GC0310MIPI_write_cmos_sensor(0x06, 0xd1);
		GC0310MIPI_write_cmos_sensor(0x07, 0x00);
		GC0310MIPI_write_cmos_sensor(0x08, 0x22);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x25, 0x00);
		GC0310MIPI_write_cmos_sensor(0x26, 0x6a);
		GC0310MIPI_write_cmos_sensor(0x27, 0x02);
		GC0310MIPI_write_cmos_sensor(0x28, 0x12);
		GC0310MIPI_write_cmos_sensor(0x29, 0x03);
		GC0310MIPI_write_cmos_sensor(0x2a, 0x50);
		GC0310MIPI_write_cmos_sensor(0x2b, 0x05);
		GC0310MIPI_write_cmos_sensor(0x2c, 0xcc);
		GC0310MIPI_write_cmos_sensor(0x2d, 0x07);
		GC0310MIPI_write_cmos_sensor(0x2e, 0x74);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_FLICKER_MODE_60HZ:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0x05, 0x02);
		GC0310MIPI_write_cmos_sensor(0x06, 0x60);
		GC0310MIPI_write_cmos_sensor(0x07, 0x00);
		GC0310MIPI_write_cmos_sensor(0x08, 0x58);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x25, 0x00);
		GC0310MIPI_write_cmos_sensor(0x26, 0x60);
		GC0310MIPI_write_cmos_sensor(0x27, 0x02);
		GC0310MIPI_write_cmos_sensor(0x28, 0x40);
		GC0310MIPI_write_cmos_sensor(0x29, 0x03);
		GC0310MIPI_write_cmos_sensor(0x2a, 0x60);
		GC0310MIPI_write_cmos_sensor(0x2b, 0x06);
		GC0310MIPI_write_cmos_sensor(0x2c, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2d, 0x08);
		GC0310MIPI_write_cmos_sensor(0x2e, 0x40);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	default:
		break;
	}

	return TRUE;
}

/*
 * THIRTY-SIX WRITES: one before the branch, thirty-four if the test
 * pattern is switched on, a single one if it is switched off. The message at the top prints
 * the argument masked to one bit ("12000021 and"@0xffffff80087194d4).
 */
UINT32 GC0310MIPISetTestPatternMode(kal_bool bEnable)
{
	printk("test pattern bEnable:=%d\n", bEnable);

	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);

	if (bEnable) {
		GC0310MIPI_write_cmos_sensor(0x4f, 0x00);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x0a, 0x45);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0x48, 0x00);
		GC0310MIPI_write_cmos_sensor(0x03, 0x03);
		GC0310MIPI_write_cmos_sensor(0x04, 0x50);
		GC0310MIPI_write_cmos_sensor(0x18, 0x1a);
		GC0310MIPI_write_cmos_sensor(0x40, 0x08);
		GC0310MIPI_write_cmos_sensor(0x41, 0x00);
		GC0310MIPI_write_cmos_sensor(0x42, 0x00);
		GC0310MIPI_write_cmos_sensor(0x70, 0x50);
		GC0310MIPI_write_cmos_sensor(0x71, 0x20);
		GC0310MIPI_write_cmos_sensor(0x72, 0x40);
		GC0310MIPI_write_cmos_sensor(0x77, 0x40);
		GC0310MIPI_write_cmos_sensor(0x78, 0x40);
		GC0310MIPI_write_cmos_sensor(0x79, 0x40);
		GC0310MIPI_write_cmos_sensor(0xd0, 0x40);
		GC0310MIPI_write_cmos_sensor(0xd1, 0x20);
		GC0310MIPI_write_cmos_sensor(0xd2, 0x20);
		GC0310MIPI_write_cmos_sensor(0x26, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2a, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2b, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2c, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2d, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2e, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2f, 0x00);
		GC0310MIPI_write_cmos_sensor(0x30, 0x00);
		GC0310MIPI_write_cmos_sensor(0x31, 0x00);
		GC0310MIPI_write_cmos_sensor(0x9b, 0x00);
		GC0310MIPI_write_cmos_sensor(0x9e, 0xc0);
		GC0310MIPI_write_cmos_sensor(0xdd, 0x00);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0x4c, 0x04);
	} else {
		GC0310MIPI_write_cmos_sensor(0x4c, 0x00);
	}

	return ERROR_NONE;
}

UINT32 GC0310MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
				MUINT8 *pFeaturePara, MUINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16 = (UINT16 *)pFeaturePara;
	UINT32 *pFeatureReturnPara32 = (UINT32 *)pFeaturePara;
	UINT32 *pFeatureData32 = (UINT32 *)pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData =
		(MSDK_SENSOR_REG_INFO_STRUCT *)pFeaturePara;

	switch (FeatureId) {
	case SENSOR_FEATURE_GET_RESOLUTION:
		*pFeatureReturnPara16++ = GC0310MIPI_IMAGE_SENSOR_FULL_WIDTH;
		*pFeatureReturnPara16 = GC0310MIPI_IMAGE_SENSOR_FULL_HEIGHT;
		*pFeatureParaLen = 4;
		break;

	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
		*pFeatureReturnPara32 = 0xFFFFFFFF;
		*pFeatureParaLen = 4;
		break;

	case SENSOR_FEATURE_SET_NIGHTMODE:
		GC0310MIPINightMode((BOOL)*pFeaturePara);
		break;

	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
		g9c959bc = *pFeatureData32;
		break;

	case SENSOR_FEATURE_SET_REGISTER:
		GC0310MIPI_write_cmos_sensor(pSensorRegData->RegAddr,
					     pSensorRegData->RegData);
		break;

	case SENSOR_FEATURE_GET_REGISTER:
		pSensorRegData->RegData =
			GC0310MIPI_read_cmos_sensor_(pSensorRegData->RegAddr);
		break;

	case SENSOR_FEATURE_GET_CONFIG_PARA:
		memcpy(pFeaturePara, &g9c95944,
		       sizeof(MSDK_SENSOR_CONFIG_STRUCT));
		*pFeatureParaLen = sizeof(MSDK_SENSOR_CONFIG_STRUCT);
		break;

	case SENSOR_FEATURE_GET_GROUP_COUNT:
		*pFeatureReturnPara32 = 0;
		*pFeatureParaLen = 4;
		break;

	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
		*pFeatureReturnPara32 = 0xFFFFFFFF;
		*pFeatureParaLen = 4;
		break;

	case SENSOR_FEATURE_SET_VIDEO_MODE:
		GC0310MIPIYUVSetVideoMode(*pFeatureReturnPara16);
		break;

	case SENSOR_FEATURE_CHECK_SENSOR_ID:
		GC0310MIPIGetSensorID(pFeatureReturnPara32);
		break;

	case SENSOR_FEATURE_SET_TEST_PATTERN:
		GC0310MIPISetTestPatternMode((BOOL)*pFeaturePara);
		break;

	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		switch (*pFeatureData32) {
		default:
			*(MUINT32 *)(uintptr_t)(*(pFeatureData32 + 1)) = 300;
			break;
		}
		break;

	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
		*pFeatureReturnPara32 = 0x8CA5EFA8;
		*pFeatureParaLen = 4;
		break;

	case SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		*pFeatureReturnPara32 =
			(GC0310MIPI_read_cmos_sensor_(0x48) & 0x0e) ? 1 : 0;
		break;

	default:
		break;
	}

	return ERROR_NONE;
}

UINT32 GC0310MIPIGetInfo(enum MSDK_SCENARIO_ID_ENUM ScenarioId,
			 MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
			 MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	pSensorInfo->SensorPreviewResolutionX =
		GC0310MIPI_IMAGE_SENSOR_PV_WIDTH;
	pSensorInfo->SensorPreviewResolutionY =
		GC0310MIPI_IMAGE_SENSOR_PV_HEIGHT;
	pSensorInfo->SensorFullResolutionX =
		GC0310MIPI_IMAGE_SENSOR_FULL_WIDTH;
	pSensorInfo->SensorFullResolutionY =
		GC0310MIPI_IMAGE_SENSOR_FULL_HEIGHT;

	pSensorInfo->SensorClockFreq = 24;
	pSensorInfo->SensorCameraPreviewFrameRate = 30;
	pSensorInfo->SensorVideoFrameRate = 30;
	pSensorInfo->SensorStillCaptureFrameRate = 10;
	pSensorInfo->SensorWebCamCaptureFrameRate = 15;

	pSensorInfo->SensorClockPolarity = 1;
	pSensorInfo->SensorClockFallingPolarity = 1;
	pSensorInfo->SensorClockRisingCount = 0;
	pSensorInfo->SensorClockFallingCount = 2;
	pSensorInfo->SensorClockDividCount = 3;
	pSensorInfo->SensorPixelClockCount = 3;
	pSensorInfo->SensorDataLatchCount = 2;
	pSensorInfo->SensorHsyncPolarity = 1;
	pSensorInfo->SensorVsyncPolarity = 1;
	pSensorInfo->SensorInterruptDelayLines = 1;

	pSensorInfo->SensorResetActiveHigh = FALSE;
	pSensorInfo->SensorResetDelayCount = 1;
	pSensorInfo->SensroInterfaceType = 1;
	pSensorInfo->SensorOutputDataFormat = 6;
	pSensorInfo->SensorMIPILaneNumber = 0;

	pSensorInfo->CaptureDelayFrame = 2;
	pSensorInfo->PreviewDelayFrame = 2;
	pSensorInfo->VideoDelayFrame = 4;

	pSensorInfo->SensorGrabStartX = 0;
	pSensorInfo->SensorGrabStartY = 1;

	pSensorInfo->SensorDrivingCurrent = 0;
	pSensorInfo->SensorMasterClockSwitch = 0;
	pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	pSensorInfo->SensorWidthSampling = 0;
	pSensorInfo->SensorHightSampling = 0;
	pSensorInfo->SensorPacketECCOrder = 1;

	g9c95940 = 3;

	memcpy(pSensorConfigData, &g9c95944,
	       sizeof(MSDK_SENSOR_CONFIG_STRUCT));

	return ERROR_NONE;
}

UINT32 GC0310MIPIYUVSetVideoMode(UINT16 u2FrameRate)
{
	g9c95930 = KAL_TRUE;

	if (u2FrameRate != 15 && u2FrameRate != 30)
		printk("Wrong Frame Rate");

	return TRUE;
}
/*
 * THE STRUCTURE WITH SEVEN POINTERS, and the seventh is the one ALPS lacks.
 *
 * GC0310ReadBV sits at +48, right after SensorClose: it is the pointer
 * camera_main3_yuv_bv_show calls, and it is precisely because the ALPS
 * drivers leave it NULL that turning on CONFIG_CUSTOM_KERNEL_IMGSENSOR
 * without these sensors introduces a null pointer call reachable from user
 * space. With GC0310 written, that hole closes by itself.
 */
static struct SENSOR_FUNCTION_STRUCT SensorFuncGC0310MIPI = {
	GC0310MIPIOpen,
	GC0310MIPIGetInfo,
	GC0310MIPIGetResolution,
	GC0310MIPIFeatureControl,
	GC0310MIPIControl,
	GC0310MIPIClose,
	GC0310ReadBV
};

UINT32 GC0310_MIPI_YUV_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	if (pfFunc != NULL)
		*pfFunc = &SensorFuncGC0310MIPI;

	return ERROR_NONE;
}
