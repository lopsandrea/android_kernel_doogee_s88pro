/*
 * Copyright (C) 2019 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*****************************************************************************
 *
 * Filename:
 * ---------
 *	 gc8034mipi_Sensor.c
 *
 * Project:
 * --------
 *	 ALPS
 *
 * Description:
 * ------------
 *	 Source code of Sensor driver
 *
 ****************************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/types.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "gc8034mipi_Sensor.h"

#define PFX "gc8034_camera_sensor"

/*
 * LOG_INF() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc8034_mipi_raw_gc8034mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define LOG_INF(format, args...)    \
	pr_info(PFX "[%s] " format, __func__, ##args)

static DEFINE_SPINLOCK(imgsensor_drv_lock);

static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = GC8034_SENSOR_ID,

	.checksum_value = 0xf7375923,

	.pre = {
		.pclk = 320000000,
		.linelength = 4272,
		.framelength = 2500,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1632,
		.grabwindow_height = 1224,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.cap = {
		.pclk = 320000000,
		.linelength = 4272,
		.framelength = 2500,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.normal_video = {
		.pclk = 320000000,
		.linelength = 4272,
		.framelength = 2500,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.hs_video = {
		.pclk = 320000000,
		.linelength = 4272,
		.framelength = 2500,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1632,
		.grabwindow_height = 1224,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.slim_video = {
		.pclk = 320000000,
		.linelength = 4272,
		.framelength = 2500,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1280,
		.grabwindow_height = 720,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},

	.margin = 16,
	.min_shutter = 4,
	.max_frame_length = 0x3fff,
	.ae_shut_delay_frame = 0,
	.ae_sensor_gain_delay_frame = 0,
	.ae_ispGain_delay_frame = 2,
	.ihdr_support = 0,
	.ihdr_le_firstline = 0,
	.sensor_mode_num = 5,

	.cap_delay_frame = 2,
	.pre_delay_frame = 2,
	.video_delay_frame = 2,
	.hs_video_delay_frame = 2,
	.slim_video_delay_frame = 2,

	.isp_driving_current = ISP_DRIVING_2MA,
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,
	.mipi_sensor_type = MIPI_OPHY_NCSI2,
	.mipi_settle_delay_mode = MIPI_SETTLEDELAY_AUTO,
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_R,
	.mclk = 24,
	.mipi_lane_num = SENSOR_MIPI_4_LANE,
	.i2c_addr_table = {0x6e, 0xff},
	.i2c_speed = 400,
};

static struct imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,
	.sensor_mode = IMGSENSOR_MODE_INIT,
	.shutter = 0x3D0,
	.gain = 0x100,
	.dummy_pixel = 0,
	.dummy_line = 0,
	.current_fps = 300,
	.autoflicker_en = KAL_FALSE,
	.test_pattern = KAL_FALSE,
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,
	.ihdr_mode = 0,
	.i2c_write_id = 0x6e,
};

/* Sensor output window information */
static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[5] = {
	{3264, 2448,    0,    0, 3264, 2448, 1632, 1224,
	    0,    0, 1632, 1224,    0,    0, 1632, 1224},
	{3264, 2448,    0,    0, 3264, 2448, 3264, 2448,
	    0,    0, 3264, 2448,    0,    0, 3264, 2448},
	{3264, 2448,    0,    0, 3264, 2448, 3264, 2448,
	    0,    0, 3264, 2448,    0,    0, 3264, 2448},
	{3264, 2448,    0,    0, 3264, 2448, 1632, 1224,
	    0,    0, 1632, 1224,    0,    0, 1632, 1224},
	{3264, 2448,    0,  264, 3264, 1836, 1280,  720,
	    0,    0, 1280,  720,    0,    0, 1280,  720},
};

static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[1] = { (char)(addr & 0xFF) };

	iReadRegI2C(pu_send_cmd, 1, (u8 *)&get_byte, 1,
		    imgsensor.i2c_write_id);

	return get_byte;
}

static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[2] = { (char)(addr & 0xFF), (char)(para & 0xFF) };

	iWriteRegI2C(pu_send_cmd, 2, imgsensor.i2c_write_id);
}


/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800871c23c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc8034_mipi_raw_gc8034mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct gc8034_dd_t {
	kal_uint16 x;
	kal_uint16 y;
	kal_uint8 t;
};

struct gc8034_otp_t {
	kal_uint8 dd_flag;
	kal_uint8 dd_cnt;
	struct gc8034_dd_t dd_param[80];
	kal_uint8 reg_flag;
	kal_uint8 reg_num;
	kal_uint8 reg_page[10];
	kal_uint8 reg_addr[10];
	kal_uint8 reg_value[10];
	kal_uint8 wb_flag;
	kal_uint16 rg_gain;
	kal_uint16 bg_gain;
	kal_uint8 golden_flag;
	kal_uint16 golden_rg;
	kal_uint16 golden_bg;
	kal_uint8 module_id;
	kal_uint8 lens_id;
	kal_uint8 vcm_id;
	kal_uint8 vcm_driver_id;
	kal_uint8 year;
	kal_uint8 month;
	kal_uint8 day;
};

static struct gc8034_otp_t gc8034_otp;

/*
 * gc8034_read_otp_group @0xffffff800871e538, 672 bytes.
 *
 * It opens the OTP window at an address and reads `len` bytes from register
 * 0xd7. The nine reads the extractor shows as `iWriteRegI2C(0xd7)` with no
 * data are unrolled reads, not writes: the only way to tell is to notice
 * that the call is `iReadRegI2C`.
 */
static void gc8034_read_otp_group(kal_uint16 addr, kal_uint8 *buf,
				  kal_uint16 len)
{
	kal_uint16 i = 0;
	kal_uint8 page = (kal_uint8)((addr >> 8) & 0x1f);

	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xd4, page);
	write_cmos_sensor(0xd5, addr & 0xff);
	write_cmos_sensor(0xf3, 0x20);
	write_cmos_sensor(0xf4, read_cmos_sensor(0xf4) | 0x02);
	write_cmos_sensor(0xf3, 0x80);

	for (i = 0; i < len; i++)
		buf[i] = read_cmos_sensor(0xd7);

	write_cmos_sensor(0xf3, 0x00);
	write_cmos_sensor(0xf4, read_cmos_sensor(0xf4) & 0xfd);
}

/*
 * gc8034_gcore_read_otp_info @0xffffff800871e7d8, 2228 bytes.
 *
 * It reads, in order: the module (0x58), the lens (0x60), the focus
 * motor (0x68), the date (0x70), the white balance gains, the golden
 * values at 0x0a70 and finally the dead pixel table,
 * one byte at a time with the address advancing by eight.
 */
static void gc8034_gcore_read_otp_info(void)
{
	kal_uint8 buf[8];
	kal_uint16 i = 0;
	kal_uint16 addr = 0;

	gc8034_read_otp_group(0x0058, &gc8034_otp.module_id, 1);
	gc8034_read_otp_group(0x0060, &gc8034_otp.lens_id, 1);
	gc8034_read_otp_group(0x0068, &gc8034_otp.vcm_id, 1);
	gc8034_read_otp_group(0x0070, buf, 3);
	gc8034_otp.year = buf[0];
	gc8034_otp.month = buf[1];
	gc8034_otp.day = buf[2];

	gc8034_read_otp_group(0x0000, buf, 5);
	gc8034_otp.wb_flag = buf[0];
	gc8034_otp.rg_gain = (kal_uint16)((buf[1] << 8) | buf[2]);
	gc8034_otp.bg_gain = (kal_uint16)((buf[3] << 8) | buf[4]);

	gc8034_read_otp_group(0x0a70, buf, 5);
	gc8034_otp.golden_flag = buf[0];
	gc8034_otp.golden_rg = (kal_uint16)((buf[1] << 8) | buf[2]);
	gc8034_otp.golden_bg = (kal_uint16)((buf[3] << 8) | buf[4]);

	gc8034_read_otp_group(addr, &gc8034_otp.dd_cnt, 1);
	if (gc8034_otp.dd_cnt > 80)
		gc8034_otp.dd_cnt = 80;

	addr = 0x0008;
	for (i = 0; i < gc8034_otp.dd_cnt; i++) {
		gc8034_read_otp_group(addr, buf, 5);
		gc8034_otp.dd_param[i].x =
			(kal_uint16)(((buf[1] & 0x0f) << 8) | buf[0]);
		gc8034_otp.dd_param[i].y =
			(kal_uint16)((buf[2] << 4) | ((buf[1] & 0xf0) >> 4));
		gc8034_otp.dd_param[i].t = buf[3];
		addr += 8;
	}

	LOG_INF("DD total_number = %d\n", gc8034_otp.dd_cnt);
	LOG_INF("flag_dd = 0x%x\n", gc8034_otp.dd_flag);

	gc8034_otp.dd_flag = 1;
}

/*
 * The application of the dead pixels, at the tail of `open`.
 *
 * The four bytes per entry come out of two reshuffled half-words:
 * "531c6f1c lsl"@0xffffff800871c1d0 shifts y by four,
 * "53082ef3 ubfx"@0xffffff800871c1d4 takes the four high bits of x, and
 * "331c0f17 bfi"@0xffffff800871c204 puts them back together into a single byte.
 */
static void gc8034_gcore_update_dd(void)
{
	kal_uint8 i = 0;

	LOG_INF("otp load start!\n");

	write_cmos_sensor(0xfe, 0x01);
	write_cmos_sensor(0xbe, 0x00);
	write_cmos_sensor(0xa9, 0x01);

	for (i = 0; i < gc8034_otp.dd_cnt; i++) {
		write_cmos_sensor(0xaa, i);
		write_cmos_sensor(0xac, gc8034_otp.dd_param[i].x & 0x00ff);
		write_cmos_sensor(0xac,
				  ((gc8034_otp.dd_param[i].y << 4) & 0x00f0) |
				  ((gc8034_otp.dd_param[i].x >> 8) & 0x000f));
		write_cmos_sensor(0xac,
				  (gc8034_otp.dd_param[i].y >> 4) & 0x00ff);
		write_cmos_sensor(0xac, gc8034_otp.dd_param[i].t);
		LOG_INF("dd[%d] x=%d y=%d t=%d\n", i,
			gc8034_otp.dd_param[i].x, gc8034_otp.dd_param[i].y,
			gc8034_otp.dd_param[i].t);
	}

	write_cmos_sensor(0xbe, 0x01);
	write_cmos_sensor(0xfe, 0x00);
}


static void sensor_init(void)
{
	write_cmos_sensor(0xf4, 0x80);
	write_cmos_sensor(0xf5, 0x19);
	write_cmos_sensor(0xf6, 0x44);
	write_cmos_sensor(0xf8, 0x63);
	write_cmos_sensor(0xfa, 0x45);
	write_cmos_sensor(0xf9, 0x00);
	write_cmos_sensor(0xf7, 0x95);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0xea);
	write_cmos_sensor(0xfe, 0x03);
	write_cmos_sensor(0x03, 0x9a);
	write_cmos_sensor(0x18, 0x07);
	write_cmos_sensor(0x01, 0x07);
	write_cmos_sensor(0xfc, 0xee);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x88, 0x03);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x03, 0x08);
	write_cmos_sensor(0x04, 0xc6);
	write_cmos_sensor(0x05, 0x02);
	write_cmos_sensor(0x06, 0x16);
	write_cmos_sensor(0x07, 0x00);
	write_cmos_sensor(0x08, 0x10);
	write_cmos_sensor(0x0a, 0x3a);
	write_cmos_sensor(0x0b, 0x00);
	write_cmos_sensor(0x0c, 0x04);
	write_cmos_sensor(0x0d, 0x09);
	write_cmos_sensor(0x0e, 0xa0);
	write_cmos_sensor(0x0f, 0x0c);
	write_cmos_sensor(0x10, 0xd4);
	write_cmos_sensor(0x17, 0xc0);
	write_cmos_sensor(0x18, 0x02);
	write_cmos_sensor(0x19, 0x17);
	write_cmos_sensor(0x1e, 0x50);
	write_cmos_sensor(0x1f, 0x80);
	write_cmos_sensor(0x21, 0x4c);
	write_cmos_sensor(0x25, 0x00);
	write_cmos_sensor(0x28, 0x4a);
	write_cmos_sensor(0x2d, 0x89);
	write_cmos_sensor(0xca, 0x02);
	write_cmos_sensor(0xcb, 0x00);
	write_cmos_sensor(0xcc, 0x39);
	write_cmos_sensor(0xce, 0xd0);
	write_cmos_sensor(0xcf, 0x93);
	write_cmos_sensor(0xd0, 0x1b);
	write_cmos_sensor(0xd1, 0xaa);
	write_cmos_sensor(0xd2, 0xcb);
	write_cmos_sensor(0xd8, 0x40);
	write_cmos_sensor(0xd9, 0xff);
	write_cmos_sensor(0xda, 0x0e);
	write_cmos_sensor(0xdb, 0xb0);
	write_cmos_sensor(0xdc, 0x0e);
	write_cmos_sensor(0xde, 0x08);
	write_cmos_sensor(0xe4, 0xc6);
	write_cmos_sensor(0xe5, 0x08);
	write_cmos_sensor(0xe6, 0x10);
	write_cmos_sensor(0xed, 0x2a);
	write_cmos_sensor(0xfe, 0x02);
	write_cmos_sensor(0x59, 0x02);
	write_cmos_sensor(0x5a, 0x04);
	write_cmos_sensor(0x5b, 0x08);
	write_cmos_sensor(0x5c, 0x20);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x1a, 0x09);
	write_cmos_sensor(0x1d, 0x13);
	write_cmos_sensor(0xfe, 0x10);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xfe, 0x10);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x20, 0x55);
	write_cmos_sensor(0x33, 0x83);
	write_cmos_sensor(0xfe, 0x01);
	write_cmos_sensor(0xdf, 0x06);
	write_cmos_sensor(0xe7, 0x18);
	write_cmos_sensor(0xe8, 0x20);
	write_cmos_sensor(0xe9, 0x16);
	write_cmos_sensor(0xea, 0x17);
	write_cmos_sensor(0xeb, 0x50);
	write_cmos_sensor(0xec, 0x6c);
	write_cmos_sensor(0xed, 0x9b);
	write_cmos_sensor(0xee, 0xd8);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x80, 0x13);
	write_cmos_sensor(0x84, 0x01);
	write_cmos_sensor(0x89, 0x03);
	write_cmos_sensor(0x8d, 0x03);
	write_cmos_sensor(0x8f, 0x14);
	write_cmos_sensor(0xad, 0x00);
	write_cmos_sensor(0x66, 0x0c);
	write_cmos_sensor(0xbc, 0x09);
	write_cmos_sensor(0xc2, 0x7f);
	write_cmos_sensor(0xc3, 0xff);
	write_cmos_sensor(0x90, 0x01);
	write_cmos_sensor(0x92, 0x04);
	write_cmos_sensor(0x94, 0x05);
	write_cmos_sensor(0x95, 0x09);
	write_cmos_sensor(0x96, 0x90);
	write_cmos_sensor(0x97, 0x0c);
	write_cmos_sensor(0x98, 0xc0);
	write_cmos_sensor(0xb0, 0x90);
	write_cmos_sensor(0xb1, 0x01);
	write_cmos_sensor(0xb2, 0x00);
	write_cmos_sensor(0xb6, 0x00);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x40, 0x22);
	write_cmos_sensor(0x41, 0x20);
	write_cmos_sensor(0x42, 0x02);
	write_cmos_sensor(0x43, 0x08);
	write_cmos_sensor(0x4e, 0x0f);
	write_cmos_sensor(0x4f, 0xf0);
	write_cmos_sensor(0x58, 0x80);
	write_cmos_sensor(0x59, 0x80);
	write_cmos_sensor(0x5a, 0x80);
	write_cmos_sensor(0x5b, 0x80);
	write_cmos_sensor(0x5c, 0x00);
	write_cmos_sensor(0x5d, 0x00);
	write_cmos_sensor(0x5e, 0x00);
	write_cmos_sensor(0x5f, 0x00);
	write_cmos_sensor(0x6b, 0x01);
	write_cmos_sensor(0x6c, 0x00);
	write_cmos_sensor(0x6d, 0x0c);
	write_cmos_sensor(0xfe, 0x01);
	write_cmos_sensor(0xbf, 0x40);
	write_cmos_sensor(0xfe, 0x01);
	write_cmos_sensor(0x68, 0x77);
	write_cmos_sensor(0xfe, 0x01);
	write_cmos_sensor(0x60, 0x00);
	write_cmos_sensor(0x61, 0x10);
	write_cmos_sensor(0x62, 0x60);
	write_cmos_sensor(0x63, 0x30);
	write_cmos_sensor(0x64, 0x00);
	write_cmos_sensor(0xfe, 0x01);
	write_cmos_sensor(0xa8, 0x60);
	write_cmos_sensor(0xa2, 0xd1);
	write_cmos_sensor(0xc8, 0x57);
	write_cmos_sensor(0xa1, 0xb8);
	write_cmos_sensor(0xa3, 0x91);
	write_cmos_sensor(0xc0, 0x50);
	write_cmos_sensor(0xd0, 0x05);
	write_cmos_sensor(0xd1, 0xb2);
	write_cmos_sensor(0xd2, 0x1f);
	write_cmos_sensor(0xd3, 0x00);
	write_cmos_sensor(0xd4, 0x00);
	write_cmos_sensor(0xd5, 0x00);
	write_cmos_sensor(0xd6, 0x00);
	write_cmos_sensor(0xd7, 0x00);
	write_cmos_sensor(0xd8, 0x00);
	write_cmos_sensor(0xd9, 0x00);
	write_cmos_sensor(0xa4, 0x10);
	write_cmos_sensor(0xa5, 0x20);
	write_cmos_sensor(0xa6, 0x60);
	write_cmos_sensor(0xa7, 0x80);
	write_cmos_sensor(0xab, 0x18);
	write_cmos_sensor(0xc7, 0xc0);
	write_cmos_sensor(0xfe, 0x01);
	write_cmos_sensor(0x20, 0x02);
	write_cmos_sensor(0x21, 0x02);
	write_cmos_sensor(0x23, 0x42);
	write_cmos_sensor(0xfe, 0x03);
	write_cmos_sensor(0x02, 0x03);
	write_cmos_sensor(0x04, 0x80);
	write_cmos_sensor(0x11, 0x2b);
	write_cmos_sensor(0x12, 0xf0);
	write_cmos_sensor(0x13, 0x0f);
	write_cmos_sensor(0x15, 0x10);
	write_cmos_sensor(0x16, 0x29);
	write_cmos_sensor(0x17, 0xff);
	write_cmos_sensor(0x19, 0xaa);
	write_cmos_sensor(0x1a, 0x02);
	write_cmos_sensor(0x21, 0x05);
	write_cmos_sensor(0x22, 0x06);
	write_cmos_sensor(0x23, 0x2b);
	write_cmos_sensor(0x24, 0x00);
	write_cmos_sensor(0x25, 0x12);
	write_cmos_sensor(0x26, 0x07);
	write_cmos_sensor(0x29, 0x07);
	write_cmos_sensor(0x2a, 0x12);
	write_cmos_sensor(0x2b, 0x07);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x3f, 0x00);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xf7, 0x97);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0xee);
	write_cmos_sensor(0xf2, 0x01);
	write_cmos_sensor(0xf4, 0x88);
}

static void binning_setting(void)
{
	write_cmos_sensor(0xf2, 0x00);
	write_cmos_sensor(0xf4, 0x80);
	write_cmos_sensor(0xf5, 0x19);
	write_cmos_sensor(0xf6, 0x44);
	write_cmos_sensor(0xf8, 0x63);
	write_cmos_sensor(0xfa, 0x45);
	write_cmos_sensor(0xf9, 0x00);
	write_cmos_sensor(0xf7, 0x9d);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0xea);
	write_cmos_sensor(0xfe, 0x03);
	write_cmos_sensor(0x03, 0x9a);
	write_cmos_sensor(0x18, 0x07);
	write_cmos_sensor(0x01, 0x07);
	write_cmos_sensor(0xfc, 0xee);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x3f, 0x00);
	write_cmos_sensor(0xfe, 0x10);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xfe, 0x10);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x80, 0x10);
	write_cmos_sensor(0xad, 0x30);
	write_cmos_sensor(0x66, 0x2c);
	write_cmos_sensor(0xbc, 0x49);
	write_cmos_sensor(0x90, 0x01);
	write_cmos_sensor(0x92, 0x04);
	write_cmos_sensor(0x94, 0x05);
	write_cmos_sensor(0x95, 0x04);
	write_cmos_sensor(0x96, 0xc8);
	write_cmos_sensor(0x97, 0x06);
	write_cmos_sensor(0x98, 0x60);
	write_cmos_sensor(0xfe, 0x03);
	write_cmos_sensor(0x02, 0x03);
	write_cmos_sensor(0x04, 0x80);
	write_cmos_sensor(0x11, 0x2b);
	write_cmos_sensor(0x12, 0xf8);
	write_cmos_sensor(0x13, 0x07);
	write_cmos_sensor(0x15, 0x10);
	write_cmos_sensor(0x16, 0x29);
	write_cmos_sensor(0x17, 0xff);
	write_cmos_sensor(0x19, 0xaa);
	write_cmos_sensor(0x1a, 0x02);
	write_cmos_sensor(0x21, 0x02);
	write_cmos_sensor(0x22, 0x03);
	write_cmos_sensor(0x23, 0x0a);
	write_cmos_sensor(0x24, 0x00);
	write_cmos_sensor(0x25, 0x12);
	write_cmos_sensor(0x26, 0x04);
	write_cmos_sensor(0x29, 0x04);
	write_cmos_sensor(0x2a, 0x02);
	write_cmos_sensor(0x2b, 0x04);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x3f, 0xd0);
}

static void fullsize_setting(void)
{
	write_cmos_sensor(0xf2, 0x00);
	write_cmos_sensor(0xf4, 0x80);
	write_cmos_sensor(0xf5, 0x19);
	write_cmos_sensor(0xf6, 0x44);
	write_cmos_sensor(0xf8, 0x63);
	write_cmos_sensor(0xfa, 0x45);
	write_cmos_sensor(0xf9, 0x00);
	write_cmos_sensor(0xf7, 0x95);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0xea);
	write_cmos_sensor(0xfe, 0x03);
	write_cmos_sensor(0x03, 0x9a);
	write_cmos_sensor(0x18, 0x07);
	write_cmos_sensor(0x01, 0x07);
	write_cmos_sensor(0xfc, 0xee);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x3f, 0x00);
	write_cmos_sensor(0xfe, 0x10);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xfe, 0x10);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x80, 0x13);
	write_cmos_sensor(0xad, 0x00);
	write_cmos_sensor(0x66, 0x0c);
	write_cmos_sensor(0xbc, 0x09);
	write_cmos_sensor(0x90, 0x01);
	write_cmos_sensor(0x92, 0x08);
	write_cmos_sensor(0x94, 0x09);
	write_cmos_sensor(0x95, 0x09);
	write_cmos_sensor(0x96, 0x90);
	write_cmos_sensor(0x97, 0x0c);
	write_cmos_sensor(0x98, 0xc0);
	write_cmos_sensor(0xfe, 0x03);
	write_cmos_sensor(0x02, 0x03);
	write_cmos_sensor(0x04, 0x80);
	write_cmos_sensor(0x11, 0x2b);
	write_cmos_sensor(0x12, 0xf0);
	write_cmos_sensor(0x13, 0x0f);
	write_cmos_sensor(0x15, 0x10);
	write_cmos_sensor(0x16, 0x29);
	write_cmos_sensor(0x17, 0xff);
	write_cmos_sensor(0x19, 0xaa);
	write_cmos_sensor(0x1a, 0x02);
	write_cmos_sensor(0x21, 0x05);
	write_cmos_sensor(0x22, 0x06);
	write_cmos_sensor(0x23, 0x2b);
	write_cmos_sensor(0x24, 0x00);
	write_cmos_sensor(0x25, 0x12);
	write_cmos_sensor(0x26, 0x07);
	write_cmos_sensor(0x29, 0x07);
	write_cmos_sensor(0x2a, 0x12);
	write_cmos_sensor(0x2b, 0x07);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x3f, 0xd0);
}

static void slim_video_setting(void)
{
	write_cmos_sensor(0xf2, 0x00);
	write_cmos_sensor(0xf4, 0x80);
	write_cmos_sensor(0xf5, 0x19);
	write_cmos_sensor(0xf6, 0x44);
	write_cmos_sensor(0xf8, 0x63);
	write_cmos_sensor(0xfa, 0x45);
	write_cmos_sensor(0xf9, 0x00);
	write_cmos_sensor(0xf7, 0x9d);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0xea);
	write_cmos_sensor(0xfe, 0x03);
	write_cmos_sensor(0x03, 0x9a);
	write_cmos_sensor(0x18, 0x07);
	write_cmos_sensor(0x01, 0x07);
	write_cmos_sensor(0xfc, 0xee);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x3f, 0x00);
	write_cmos_sensor(0xfe, 0x10);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xfe, 0x10);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x80, 0x10);
	write_cmos_sensor(0xad, 0x30);
	write_cmos_sensor(0x66, 0x2c);
	write_cmos_sensor(0xbc, 0x49);
	write_cmos_sensor(0x90, 0x01);
	write_cmos_sensor(0x91, 0x01);
	write_cmos_sensor(0x92, 0x00);
	write_cmos_sensor(0x94, 0xb5);
	write_cmos_sensor(0x95, 0x02);
	write_cmos_sensor(0x96, 0xd0);
	write_cmos_sensor(0x97, 0x05);
	write_cmos_sensor(0x98, 0x00);
	write_cmos_sensor(0xfe, 0x01);
	write_cmos_sensor(0x62, 0x28);
	write_cmos_sensor(0x63, 0x10);
	write_cmos_sensor(0xfe, 0x03);
	write_cmos_sensor(0x02, 0x03);
	write_cmos_sensor(0x04, 0x80);
	write_cmos_sensor(0x11, 0x2b);
	write_cmos_sensor(0x12, 0x40);
	write_cmos_sensor(0x13, 0x06);
	write_cmos_sensor(0x15, 0x10);
	write_cmos_sensor(0x16, 0x29);
	write_cmos_sensor(0x17, 0xff);
	write_cmos_sensor(0x19, 0xaa);
	write_cmos_sensor(0x1a, 0x02);
	write_cmos_sensor(0x21, 0x02);
	write_cmos_sensor(0x22, 0x03);
	write_cmos_sensor(0x23, 0x0a);
	write_cmos_sensor(0x24, 0x00);
	write_cmos_sensor(0x25, 0x12);
	write_cmos_sensor(0x26, 0x04);
	write_cmos_sensor(0x29, 0x04);
	write_cmos_sensor(0x2a, 0x02);
	write_cmos_sensor(0x2b, 0x04);
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x3f, 0xd0);
}

static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;

	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			*sensor_id = ((read_cmos_sensor(0xf0) << 8) |
				      read_cmos_sensor(0xf1));
			if (*sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n",
					imgsensor.i2c_write_id, *sensor_id);
				return ERROR_NONE;
			}
			LOG_INF("Read sensor id fail, id: 0x%x\n",
				imgsensor.i2c_write_id);
			retry--;
		} while (retry > 0);
		i++;
		retry = 2;
	}

	if (*sensor_id != imgsensor_info.sensor_id) {
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}

	return ERROR_NONE;
}

/*
 * gc8034_gcore_update_chipversion: the name is in the binary, among the
 * strings __func__ references.
 *
 * It opens the OTP window at 0x0b40, waits one millisecond
 * ("52912b00 mov"@0xffffff800871c324 is __const_udelay(0x418958), that is a
 * thousand microseconds) and reads 0xd7. The two bits it looks at are 0x06:
 * "121f0513 and"@0xffffff800871c380.
 */
static void gc8034_gcore_update_chipversion(void)
{
	kal_uint8 val = 0;

	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xd4, 0x0b);
	write_cmos_sensor(0xd5, 0x40);
	mdelay(1);
	write_cmos_sensor(0xf3, 0x20);
	val = read_cmos_sensor(0xd7);

	if (val & 0x06) {
		write_cmos_sensor(0xfe, 0x00);
		write_cmos_sensor(0xd2, 0x00);
		write_cmos_sensor(0xcb, 0x00);
	}

	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xf2, 0x00);
	write_cmos_sensor(0xf4, 0x80);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xf7, 0x95);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0x00);
	write_cmos_sensor(0xfc, 0xee);
}

static void set_dummy(void)
{
	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x41, (imgsensor.frame_length >> 8) & 0x3f);
	write_cmos_sensor(0x42, imgsensor.frame_length & 0xff);
}

static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{
	kal_uint32 frame_length = imgsensor.frame_length;

	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;

	spin_lock(&imgsensor_drv_lock);
	if (frame_length >= imgsensor.min_frame_length)
		imgsensor.frame_length = frame_length;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;

	imgsensor.dummy_line =
		imgsensor.frame_length - imgsensor.min_frame_length;

	LOG_INF("dummyline = %d, dummypixels = %d\n",
		imgsensor.dummy_line, imgsensor.dummy_pixel);

	if (imgsensor.frame_length > imgsensor_info.max_frame_length) {
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line =
			imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);

	set_dummy();
}

static void write_shutter(kal_uint32 shutter)
{
	kal_uint16 realtime_fps = 0;

	spin_lock(&imgsensor_drv_lock);
	if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);

	if (shutter < imgsensor_info.min_shutter)
		shutter = imgsensor_info.min_shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 /
			       imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else
			set_dummy();
	} else {
		set_dummy();
	}

	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0x03, (shutter >> 8) & 0x3f);
	write_cmos_sensor(0x04, shutter & 0xff);

	LOG_INF("Exit! shutter = %d, framelength = %d\n",
		shutter, imgsensor.frame_length);
}

static void set_shutter(kal_uint32 shutter)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.shutter = shutter;
	spin_unlock(&imgsensor_drv_lock);

	write_shutter(shutter);
}

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = gain << 4;

	/* BASEGAIN is 0x40, that is 64: it is a DIVISOR, not a shift. */
	reg_gain = reg_gain / BASEGAIN;

	if (reg_gain < 0x40)
		reg_gain = 0x40;
	else if (reg_gain > 0x400)
		reg_gain = 0x400;

	return (kal_uint16)reg_gain;
}

static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain = gain2reg(gain);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain;
	spin_unlock(&imgsensor_drv_lock);

	write_cmos_sensor(0xfe, 0x00);
	write_cmos_sensor(0xb6, (reg_gain >> 8) & 0x0f);
	write_cmos_sensor(0xb1, (reg_gain >> 6) & 0x0f);
	write_cmos_sensor(0xb2, (reg_gain << 2) & 0xfc);

	LOG_INF("gain = %d, reg_gain = 0x%x\n", gain, reg_gain);

	return gain;
}

static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	binning_setting();

	return ERROR_NONE;
}

static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
		LOG_INF("current_fps %d is not support, use cap: %d fps!\n",
			imgsensor.current_fps,
			imgsensor_info.cap.max_framerate / 10);

	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	imgsensor.pclk = imgsensor_info.cap.pclk;
	imgsensor.line_length = imgsensor_info.cap.linelength;
	imgsensor.frame_length = imgsensor_info.cap.framelength;
	imgsensor.min_frame_length = imgsensor_info.cap.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	fullsize_setting();

	return ERROR_NONE;
}

static kal_uint32 normal_video(
	MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	fullsize_setting();

	return ERROR_NONE;
}

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			   MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	binning_setting();

	return ERROR_NONE;
}

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			     MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	slim_video_setting();

	return ERROR_NONE;
}

/*
 * open() was reconstructed from the factory kernel disassembly (0xffffff800871ac48, 6604 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_gc8034_mipi_raw_gc8034mipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static kal_uint32 open(void)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint16 sensor_id = 0;

	LOG_INF("PLATFORM:MT6771,MIPI 4LANE\n");

	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = ((read_cmos_sensor(0xf0) << 8) |
				     read_cmos_sensor(0xf1));
			if (sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("i2c write id: 0x%x, sensor id: 0x%x\n",
					imgsensor.i2c_write_id, sensor_id);
				break;
			}
			LOG_INF("Read sensor id fail, id: 0x%x\n",
				imgsensor.i2c_write_id);
			retry--;
		} while (retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}

	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;

	sensor_init();

	if (!gc8034_otp.dd_flag)
		gc8034_gcore_read_otp_info();
	gc8034_gcore_update_dd();

	gc8034_gcore_update_chipversion();

	spin_lock(&imgsensor_drv_lock);
	imgsensor.autoflicker_en = KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.shutter = 0x3D0;
	imgsensor.gain = 0x100;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_mode = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}

/* close @0xffffff800871e510, 40 bytes: one printk and a return, nothing else. */
static kal_uint32 close(void)
{
	LOG_INF("E\n");

	return ERROR_NONE;
}

/*
 * get_resolution @0xffffff800871c704, 100 bytes.
 *
 * The ten sizes come out of three wide stores: "mov x8,#0x660" builds
 * 0x0990_0CC0_04C8_0660, which read as four half-words is 1632, 1224,
 * 3264, 2448.
 */
static kal_uint32 get_resolution(
	MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	sensor_resolution->SensorFullWidth =
		imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight =
		imgsensor_info.cap.grabwindow_height;

	sensor_resolution->SensorPreviewWidth =
		imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight =
		imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth =
		imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight =
		imgsensor_info.normal_video.grabwindow_height;

	sensor_resolution->SensorHighSpeedVideoWidth =
		imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight =
		imgsensor_info.hs_video.grabwindow_height;

	sensor_resolution->SensorSlimVideoWidth =
		imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight =
		imgsensor_info.slim_video.grabwindow_height;

	return ERROR_NONE;
}

/*
 * get_info @0xffffff800871c614, 240 bytes.
 *
 * It is SHORTER than the imx230 one, and the difference is not an oversight:
 * here PDAF_Support, HDR_Support and TEMPERATURE_SUPPORT are not written. The
 * store offsets match the structure fields one by one -- 13 is
 * SensorClockPolarity, 22 SensorInterruptDelayLines, 161 SensorModeNum -- and
 * whoever rewrites them can check without compiling.
 */
static kal_uint32 get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			   MSDK_SENSOR_INFO_STRUCT *sensor_info,
			   MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4;
	sensor_info->SensorResetActiveHigh = FALSE;
	sensor_info->SensorResetDelayCount = 5;

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat =
		imgsensor_info.sensor_output_dataformat;

	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame =
		imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame =
		imgsensor_info.slim_video_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0;
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;
	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;
	sensor_info->AESensorGainDelayFrame =
		imgsensor_info.ae_sensor_gain_delay_frame;
	sensor_info->AEISPGainDelayFrame =
		imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;

	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3;
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2;
	sensor_info->SensorPixelClockCount = 3;
	sensor_info->SensorDataLatchCount = 2;

	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0;
	sensor_info->SensorHightSampling = 0;
	sensor_info->SensorPacketECCOrder = 1;

	sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =
		imgsensor_info.pre.mipi_data_lp2hs_settle_dc;

	sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
	sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;

	return ERROR_NONE;
}

/*
 * control @0xffffff800871dbb4, 2396 bytes.
 *
 * Five cases in a jump table at 0xffffff8008f49ff6, read as bytes:
 * 0x00, 0x3f, 0x67, 0x86, 0xa7. The fallback branch -- the `b.hi` one --
 * does the same as `preview` and returns 4, that is
 * ERROR_INVALID_SCENARIO_ID ("321e03e0 orr"@0xffffff800871dd34).
 */
static kal_uint32 control(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			  MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		preview(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		capture(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		normal_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		hs_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		slim_video(image_window, sensor_config_data);
		break;
	default:
		LOG_INF("Error ScenarioId setting");
		preview(image_window, sensor_config_data);
		return ERROR_INVALID_SCENARIO_ID;
	}

	return ERROR_NONE;
}

static kal_uint32 set_video_mode(UINT16 framerate)
{
	if (framerate == 0)
		return ERROR_NONE;

	if (framerate == 300 && imgsensor.autoflicker_en)
		framerate = 296;
	else if (framerate == 150 && imgsensor.autoflicker_en)
		framerate = 146;

	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);

	set_max_framerate(imgsensor.current_fps, 1);

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	LOG_INF("enable = %d, framerate = %d\n", enable, framerate);

	spin_lock(&imgsensor_drv_lock);
	if (enable)
		imgsensor.autoflicker_en = KAL_TRUE;
	else
		imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}

static kal_uint32 set_max_framerate_by_scenario(
	enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate)
{
	kal_uint32 frame_length;

	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		frame_length = imgsensor_info.pre.pclk / framerate * 10 /
			       imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.pre.framelength)
			? (frame_length - imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		frame_length = imgsensor_info.normal_video.pclk / framerate *
			       10 / imgsensor_info.normal_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length >
			 imgsensor_info.normal_video.framelength)
			? (frame_length -
			   imgsensor_info.normal_video.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.normal_video.framelength +
			imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		frame_length = imgsensor_info.cap.pclk / framerate * 10 /
			       imgsensor_info.cap.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.cap.framelength)
			? (frame_length - imgsensor_info.cap.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.cap.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		frame_length = imgsensor_info.hs_video.pclk / framerate * 10 /
			       imgsensor_info.hs_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.hs_video.framelength)
			? (frame_length -
			   imgsensor_info.hs_video.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.hs_video.framelength +
			imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		frame_length = imgsensor_info.slim_video.pclk / framerate *
			       10 / imgsensor_info.slim_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.slim_video.framelength)
			? (frame_length -
			   imgsensor_info.slim_video.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.slim_video.framelength +
			imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		break;
	default:
		frame_length = imgsensor_info.pre.pclk / framerate * 10 /
			       imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.pre.framelength)
			? (frame_length - imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		set_dummy();
		LOG_INF("error scenario_id = %d, we use preview scenario\n",
			scenario_id);
		break;
	}

	return ERROR_NONE;
}

static kal_uint32 get_default_framerate_by_scenario(
	enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate)
{
	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		*framerate = imgsensor_info.pre.max_framerate;
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		*framerate = imgsensor_info.normal_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		*framerate = imgsensor_info.cap.max_framerate;
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		*framerate = imgsensor_info.hs_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		*framerate = imgsensor_info.slim_video.max_framerate;
		break;
	default:
		break;
	}

	return ERROR_NONE;
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);

	if (enable) {
		write_cmos_sensor(0xfe, 0x01);
		write_cmos_sensor(0x8c, 0x11);
	} else {
		write_cmos_sensor(0xfe, 0x01);
		write_cmos_sensor(0x8c, 0x10);
	}
	write_cmos_sensor(0xfe, 0x00);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}

/*
 * feature_control @0xffffff800871c768, 5196 bytes.
 *
 * NINETEEN cases out of a hundred and ten, and that is not read by eye: the
 * jump table sits at 0xffffff8008f49f02 and is made of half-words, ninety-one
 * of which are 0x04d5 -- that is, the fallback branch. The nineteen holding
 * something else are the ones below, in numeric order.
 */
static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
				  UINT8 *feature_para, UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16 = (UINT16 *) feature_para;
	UINT16 *feature_data_16 = (UINT16 *) feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *) feature_para;
	UINT32 *feature_data_32 = (UINT32 *) feature_para;
	unsigned long long *feature_data = (unsigned long long *) feature_para;

	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data =
		(MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

	LOG_INF("feature_id = %d\n", feature_id);

	switch (feature_id) {
	case SENSOR_FEATURE_GET_PERIOD:
		*feature_return_para_16++ = imgsensor.line_length;
		*feature_return_para_16 = imgsensor.frame_length;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
		*feature_return_para_32 = imgsensor.pclk;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_ESHUTTER:
		set_shutter(*feature_data);
		break;
	case SENSOR_FEATURE_SET_GAIN:
		set_gain((UINT16) *feature_data);
		break;
	case SENSOR_FEATURE_SET_REGISTER:
		write_cmos_sensor(sensor_reg_data->RegAddr,
				  sensor_reg_data->RegData);
		break;
	case SENSOR_FEATURE_GET_REGISTER:
		sensor_reg_data->RegData =
			read_cmos_sensor(sensor_reg_data->RegAddr);
		LOG_INF("adb_i2c_read 0x%x = 0x%x\n",
			sensor_reg_data->RegAddr, sensor_reg_data->RegData);
		break;
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
		*feature_return_para_32 = LENS_DRIVER_ID_DO_NOT_CARE;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_VIDEO_MODE:
		set_video_mode(*feature_data);
		break;
	case SENSOR_FEATURE_CHECK_SENSOR_ID:
		get_imgsensor_id(feature_return_para_32);
		break;
	case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
		set_auto_flicker_mode((BOOL) *feature_data_16,
				      *(feature_data_16 + 1));
		break;
	case SENSOR_FEATURE_SET_TEST_PATTERN:
		set_test_pattern_mode((BOOL) *feature_data);
		break;
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
		set_max_framerate_by_scenario(
			(enum MSDK_SCENARIO_ID_ENUM) *feature_data,
			*(feature_data + 1));
		break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		get_default_framerate_by_scenario(
			(enum MSDK_SCENARIO_ID_ENUM) *(feature_data),
			(MUINT32 *) (uintptr_t) (*(feature_data + 1)));
		break;
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
		*feature_return_para_32 = imgsensor_info.checksum_value;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_FRAMERATE:
		LOG_INF("current fps: %d\n", *feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.current_fps = *feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_SET_HDR:
		LOG_INF("ihdr enable: %d\n", *feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.ihdr_mode = *feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_GET_CROP_INFO:
		LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId: %d\n",
			(UINT32) *feature_data);
		wininfo = (struct SENSOR_WINSIZE_INFO_STRUCT *)
			  (uintptr_t) (*(feature_data + 1));

		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[1],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[2],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[3],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[4],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[0],
			       sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		}
		break;
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
		LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n",
			(UINT16) *feature_data,
			(UINT16) *(feature_data + 1),
			(UINT16) *(feature_data + 2));
		break;
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) =
				imgsensor_info.cap.pclk;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) =
				imgsensor_info.normal_video.pclk;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) =
				imgsensor_info.hs_video.pclk;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) =
				imgsensor_info.slim_video.pclk;
			break;
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *) (uintptr_t) (*(feature_data + 1)) =
				imgsensor_info.pre.pclk;
			break;
		}
		break;
	default:
		break;
	}

	return ERROR_NONE;
}


static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 GC8034_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc = &sensor_func;

	return ERROR_NONE;
}

