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
 *	 s5k3p3sxmipi_Sensor.c
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

#include "s5k3p3sxmipi_Sensor.h"

#define PFX "s5k3p3sx_camera_sensor"
#define LOG_INF(format, args...)    \
	pr_info(PFX "[%s] " format, __func__, ##args)

static DEFINE_SPINLOCK(imgsensor_drv_lock);

static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = S5K3P3SX_SENSOR_ID,

	/*
	 * 0x0623a073, not an invented number: it is what
	 * GET_TEST_PATTERN_CHECKSUM_VALUE returns, built from
	 * "52940e68 mov"@0xffffff800872151c (0xa073) and
	 * "72a0c468 movk"@0xffffff8008721520 (0x623 in the high half).
	 */
	.checksum_value = 0x0623a073,

	.pre = {
		.pclk = 280000000,
		.linelength = 5148,
		.framelength = 1800,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2320,
		.grabwindow_height = 1748,
		/*
		 * This section was reconstructed from the factory kernel disassembly (0xffffff8008720e3c).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_s5k3p3sx_mipi_raw_s5k3p3sxmipi_Sensor.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		.mipi_data_lp2hs_settle_dc = 23,
		.max_framerate = 300,
	},
	.cap = {
		.pclk = 280000000,
		.linelength = 5148,
		.framelength = 1800,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2320,
		.grabwindow_height = 1748,
		.mipi_data_lp2hs_settle_dc = 23,
		.max_framerate = 300,
	},
	.normal_video = {
		.pclk = 280000000,
		.linelength = 5148,
		.framelength = 1800,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2320,
		.grabwindow_height = 1748,
		.mipi_data_lp2hs_settle_dc = 23,
		.max_framerate = 300,
	},
	.hs_video = {
		.pclk = 280000000,
		.linelength = 5148,
		.framelength = 1800,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1920,
		.grabwindow_height = 1080,
		.mipi_data_lp2hs_settle_dc = 23,
		.max_framerate = 300,
	},
	.slim_video = {
		.pclk = 280000000,
		.linelength = 5148,
		.framelength = 1800,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 2320,
		.grabwindow_height = 1748,
		.mipi_data_lp2hs_settle_dc = 23,
		.max_framerate = 300,
	},

	/*
	 * EIGHT, not ten. It is the margin between the shutter and the frame
	 * length, and it is readable in SET_ESHUTTER:
	 *   "5100210b sub"@0xffffff8008721008    min_frame_length - 8
	 *   "110022ca add"@0xffffff8008721000    shutter + 8
	 * With ten the driver computes a frame length that does not
	 * match the exposure requested.
	 */
	.margin = 8,
	.min_shutter = 5,   /* "710016df cmp"@0xffffff8008721034 */
	.max_frame_length = 0xffff,
	.ae_shut_delay_frame = 0,
	.ae_sensor_gain_delay_frame = 0,
	.ae_ispGain_delay_frame = 2,
	.ihdr_support = 0,
	.ihdr_le_firstline = 0,
	.sensor_mode_num = 5,

	.cap_delay_frame = 3,
	.pre_delay_frame = 3,
	.video_delay_frame = 3,
	.hs_video_delay_frame = 3,
	.slim_video_delay_frame = 3,

	.isp_driving_current = ISP_DRIVING_2MA,
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,
	.mipi_sensor_type = MIPI_OPHY_NCSI2,
	.mipi_settle_delay_mode = MIPI_SETTLEDELAY_MANUAL,
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_Gr,
	.mclk = 24,
	.mipi_lane_num = SENSOR_MIPI_4_LANE,
	/*
	 * TWO addresses, and 0x20 comes first. `open` tries 0x20
	 * ("321b03e8 orr"@0xffffff800871fc20, written into i2c_write_id by
	 * "390a42a8 strb"@0xffffff800871fc28) and only if that fails moves to 0x5a
	 * ("52800b48 mov"@0xffffff800871fccc). The first version had read only the
	 * second one, which is the one that appears further down in the
	 * disassembly.
	 */
	.i2c_addr_table = {0x20, 0x5a, 0xff},
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
	.i2c_write_id = 0x20,
};


/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008721634, 380 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_s5k3p3sx_mipi_raw_s5k3p3sxmipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info = {
	.i4OffsetX = 0,
	.i4OffsetY = 4,
	.i4PitchX = 64,
	.i4PitchY = 64,
	.i4PairNum = 16,
	.i4SubBlkW = 16,
	.i4SubBlkH = 16,
	.i4PosL = {
		{8, 7}, {60, 7}, {24, 11}, {44, 11},
		{12, 27}, {56, 27}, {28, 31}, {40, 31},
		{28, 39}, {40, 39}, {12, 43}, {56, 43},
		{24, 59}, {44, 59}, {8, 63}, {60, 63} },
	.i4PosR = {
		{8, 11}, {60, 11}, {24, 15}, {44, 15},
		{12, 23}, {56, 23}, {28, 27}, {40, 27},
		{28, 43}, {40, 43}, {12, 47}, {56, 47},
		{24, 55}, {44, 55}, {8, 59}, {60, 59} },
	.iMirrorFlip = 0,
	.i4BlockNumX = 0,
	.i4BlockNumY = 0,
	.i4LeFirst = 0,
	.i4Crop = {
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0} },
};

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008f4a360, 128 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_s5k3p3sx_mipi_raw_s5k3p3sxmipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[4] = {
	{2320, 1748,    0,    0, 2320, 1748, 2320, 1748,
	    0,    0, 2320, 1748,    0,    0, 2320, 1748},
	{2320, 1748,    0,    0, 2320, 1748, 2320, 1748,
	    0,    0, 2320, 1748,    0,    0, 2320, 1748},
	{2320, 1748,  200,  334, 1920, 1080, 1920, 1080,
	    0,    0, 1920, 1080,    0,    0, 1920, 1080},
	{2320, 1748,    0,    0, 2320, 1748, 2320, 1748,
	    0,    0, 2320, 1748,    0,    0, 2320, 1748},
};

/*
 * Samsung reads and writes with a SIXTEEN-bit address: the buffer is
 * four bytes, not two, and the two halves have to be split by hand.
 */
/*
 * read_cmos_sensor() was reconstructed from the factory kernel disassembly (0xffffff800871fc64).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_s5k3p3sx_mipi_raw_s5k3p3sxmipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[2] = { (char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pu_send_cmd, 2, (u8 *)&get_byte, 1,
		    imgsensor.i2c_write_id);

	return get_byte;
}

/* The identifier lives in two consecutive registers, one per byte. */
static kal_uint16 read_sensor_id(void)
{
	return (kal_uint16)((read_cmos_sensor(0x0000) << 8) |
			    read_cmos_sensor(0x0001));
}

static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[4] = { (char)(addr >> 8), (char)(addr & 0xFF),
				(char)(para >> 8), (char)(para & 0xFF) };

	iWriteRegI2C(pu_send_cmd, 4, imgsensor.i2c_write_id);
}

/* Three bytes: a sixteen-bit address, an eight-bit datum. Only the gain uses it. */
static void write_cmos_sensor_8(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[3] = { (char)(addr >> 8), (char)(addr & 0xFF),
				(char)(para & 0xFF) };

	iWriteRegI2C(pu_send_cmd, 3, imgsensor.i2c_write_id);
}


static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data);


static void sensor_init(void)
{
	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x6010, 0x01);
	mdelay(1);

	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x6214, 0x7971);
	write_cmos_sensor(0x6218, 0x100);
	write_cmos_sensor(0x602a, 0xf408);
	write_cmos_sensor(0x6f12, 0x48);
	write_cmos_sensor(0x602a, 0xf40c);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x602a, 0xf4aa);
	write_cmos_sensor(0x6f12, 0x60);
	write_cmos_sensor(0x602a, 0xf442);
	write_cmos_sensor(0x6f12, 0x800);
	write_cmos_sensor(0x602a, 0xf43e);
	write_cmos_sensor(0x6f12, 0x400);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x602a, 0xf4a4);
	write_cmos_sensor(0x6f12, 0x10);
	write_cmos_sensor(0x602a, 0xf4ac);
	write_cmos_sensor(0x6f12, 0x56);
	write_cmos_sensor(0x602a, 0xf480);
	write_cmos_sensor(0x6f12, 0x08);
	write_cmos_sensor(0x602a, 0xf492);
	write_cmos_sensor(0x6f12, 0x16);
	write_cmos_sensor(0x602a, 0x3e58);
	write_cmos_sensor(0x6f12, 0x56);
	write_cmos_sensor(0x602a, 0x39ee);
	write_cmos_sensor(0x6f12, 0x206);
	write_cmos_sensor(0x602a, 0x39e8);
	write_cmos_sensor(0x6f12, 0x205);
	write_cmos_sensor(0x602a, 0x3a36);
	write_cmos_sensor(0x6f12, 0xb3f0);
	write_cmos_sensor(0x602a, 0x32b2);
	write_cmos_sensor(0x6f12, 0x132);
	write_cmos_sensor(0x602a, 0x3a38);
	write_cmos_sensor(0x6f12, 0x6c);
	write_cmos_sensor(0x602a, 0x3552);
	write_cmos_sensor(0x6f12, 0xd0);
	write_cmos_sensor(0x602a, 0x3194);
	write_cmos_sensor(0x6f12, 0x1001);
	write_cmos_sensor(0x6028, 0x2000);
	write_cmos_sensor(0x602a, 0x13ec);
	write_cmos_sensor(0x6f12, 0x8011);
	write_cmos_sensor(0x6f12, 0x8011);
	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x602a, 0x39ba);
	write_cmos_sensor(0x6f12, 0x01);
	write_cmos_sensor(0x602a, 0x3004);
	write_cmos_sensor(0x6f12, 0x08);
	write_cmos_sensor(0x602a, 0x39aa);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6028, 0x2000);
	write_cmos_sensor(0x602a, 0x26c);
	write_cmos_sensor(0x6f12, 0x41f0);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x602a, 0x37d4);
	write_cmos_sensor(0x6f12, 0x2d);
	write_cmos_sensor(0x602a, 0x37da);
	write_cmos_sensor(0x6f12, 0x5d);
	write_cmos_sensor(0x602a, 0x37e0);
	write_cmos_sensor(0x6f12, 0x8d);
	write_cmos_sensor(0x602a, 0x37e6);
	write_cmos_sensor(0x6f12, 0xbd);
	write_cmos_sensor(0x602a, 0x37ec);
	write_cmos_sensor(0x6f12, 0xed);
	write_cmos_sensor(0x602a, 0x37f2);
	write_cmos_sensor(0x6f12, 0x11d);
	write_cmos_sensor(0x602a, 0x37f8);
	write_cmos_sensor(0x6f12, 0x14d);
	write_cmos_sensor(0x602a, 0x37fe);
	write_cmos_sensor(0x6f12, 0x17d);
	write_cmos_sensor(0x602a, 0x3804);
	write_cmos_sensor(0x6f12, 0x1ad);
	write_cmos_sensor(0x602a, 0x380a);
	write_cmos_sensor(0x6f12, 0x1dd);
	write_cmos_sensor(0x602a, 0x3810);
	write_cmos_sensor(0x6f12, 0x20d);
	write_cmos_sensor(0x602a, 0x32a6);
	write_cmos_sensor(0x6f12, 0x06);
	write_cmos_sensor(0x602a, 0x32be);
	write_cmos_sensor(0x6f12, 0x06);
	write_cmos_sensor(0x602a, 0x3210);
	write_cmos_sensor(0x6f12, 0x06);
	write_cmos_sensor(0x6028, 0x2000);
	write_cmos_sensor(0x602a, 0x2ef8);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6f12, 0x448);
	write_cmos_sensor(0x6f12, 0x349);
	write_cmos_sensor(0x6f12, 0x160);
	write_cmos_sensor(0x6f12, 0xc26a);
	write_cmos_sensor(0x6f12, 0x511a);
	write_cmos_sensor(0x6f12, 0x8180);
	write_cmos_sensor(0x6f12, 0xf0);
	write_cmos_sensor(0x6f12, 0x21b8);
	write_cmos_sensor(0x6f12, 0x2000);
	write_cmos_sensor(0x6f12, 0x2f78);
	write_cmos_sensor(0x6f12, 0x2000);
	write_cmos_sensor(0x6f12, 0x18e0);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6f12, 0x10b5);
	write_cmos_sensor(0x6f12, 0xe4c);
	write_cmos_sensor(0x6f12, 0xb4f8);
	write_cmos_sensor(0x6f12, 0x5820);
	write_cmos_sensor(0x6f12, 0x2388);
	write_cmos_sensor(0x6f12, 0xa2eb);
	write_cmos_sensor(0x6f12, 0x5302);
	write_cmos_sensor(0x6f12, 0xa4f8);
	write_cmos_sensor(0x6f12, 0x5820);
	write_cmos_sensor(0x6f12, 0x6288);
	write_cmos_sensor(0x6f12, 0x5208);
	write_cmos_sensor(0x6f12, 0x6280);
	write_cmos_sensor(0x6f12, 0xf0);
	write_cmos_sensor(0x6f12, 0x14f8);
	write_cmos_sensor(0x6f12, 0xb4f8);
	write_cmos_sensor(0x6f12, 0x5800);
	write_cmos_sensor(0x6f12, 0x2188);
	write_cmos_sensor(0x6f12, 0xeb);
	write_cmos_sensor(0x6f12, 0x5100);
	write_cmos_sensor(0x6f12, 0xa4f8);
	write_cmos_sensor(0x6f12, 0x5800);
	write_cmos_sensor(0x6f12, 0x6088);
	write_cmos_sensor(0x6f12, 0x4000);
	write_cmos_sensor(0x6f12, 0x6080);
	write_cmos_sensor(0x6f12, 0x10bd);
	write_cmos_sensor(0x6f12, 0xaff2);
	write_cmos_sensor(0x6f12, 0x3300);
	write_cmos_sensor(0x6f12, 0x249);
	write_cmos_sensor(0x6f12, 0x863);
	write_cmos_sensor(0x6f12, 0x7047);
	write_cmos_sensor(0x6f12, 0x2000);
	write_cmos_sensor(0x6f12, 0x1998);
	write_cmos_sensor(0x6f12, 0x2000);
	write_cmos_sensor(0x6f12, 0x460);
	write_cmos_sensor(0x6f12, 0x43f2);
	write_cmos_sensor(0x6f12, 0x290c);
	write_cmos_sensor(0x6f12, 0xc0f2);
	write_cmos_sensor(0x6f12, 0x0c);
	write_cmos_sensor(0x6f12, 0x6047);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6f12, 0x3103);
	write_cmos_sensor(0x6f12, 0x22);
	write_cmos_sensor(0x6f12, 0x00);
	write_cmos_sensor(0x6f12, 0x01);
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

	write_cmos_sensor(0x100, 0x00);
	mdelay(1);

	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x344, 0x00);
	write_cmos_sensor(0x346, 0x00);
	write_cmos_sensor(0x348, 0x90f);
	write_cmos_sensor(0x34a, 0x6d3);
	write_cmos_sensor(0x34c, 0x910);
	write_cmos_sensor(0x34e, 0x6d4);
	write_cmos_sensor(0x3002, 0x01);
	write_cmos_sensor(0x136, 0x1800);
	write_cmos_sensor(0x304, 0x06);
	write_cmos_sensor(0x306, 0x8c);
	write_cmos_sensor(0x302, 0x01);
	write_cmos_sensor(0x300, 0x08);
	write_cmos_sensor(0x30c, 0x04);
	write_cmos_sensor(0x30e, 0x78);
	write_cmos_sensor(0x30a, 0x01);
	write_cmos_sensor(0x308, 0x08);
	write_cmos_sensor(0x3008, 0x01);
	write_cmos_sensor(0x3a0c, 0x78);
	write_cmos_sensor(0x800, 0x00);
	write_cmos_sensor(0x200, 0x200);
	write_cmos_sensor(0x202, 0x100);
	write_cmos_sensor(0x21c, 0x200);
	write_cmos_sensor(0x21e, 0x100);
	write_cmos_sensor(0x342, 0x141c);
	write_cmos_sensor(0x340, 0x708);
	write_cmos_sensor(0x114, 0x300);
	write_cmos_sensor(0x3072, 0x3c0);
	write_cmos_sensor(0x100, 0x100);
	write_cmos_sensor(0x101, 0x00);

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

	write_cmos_sensor(0x100, 0x00);
	mdelay(1);

	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x344, 0xc8);
	write_cmos_sensor(0x346, 0x14e);
	write_cmos_sensor(0x348, 0x847);
	write_cmos_sensor(0x34a, 0x585);
	write_cmos_sensor(0x34c, 0x780);
	write_cmos_sensor(0x034e, 0x0438);
	write_cmos_sensor(0x0230, 0x0100);

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

	write_cmos_sensor(0x100, 0x00);
	mdelay(1);

	write_cmos_sensor(0x6028, 0x4000);
	write_cmos_sensor(0x344, 0x00);
	write_cmos_sensor(0x346, 0x00);
	write_cmos_sensor(0x348, 0x90f);
	write_cmos_sensor(0x34a, 0x6d3);
	write_cmos_sensor(0x34c, 0x910);
	write_cmos_sensor(0x34e, 0x6d4);
	write_cmos_sensor(0x3002, 0x01);
	write_cmos_sensor(0x136, 0x1800);
	write_cmos_sensor(0x304, 0x06);
	write_cmos_sensor(0x306, 0x8c);
	write_cmos_sensor(0x302, 0x01);
	write_cmos_sensor(0x300, 0x08);
	write_cmos_sensor(0x30c, 0x04);
	write_cmos_sensor(0x30e, 0x78);
	write_cmos_sensor(0x30a, 0x01);
	write_cmos_sensor(0x308, 0x08);
	write_cmos_sensor(0x3008, 0x01);
	write_cmos_sensor(0x3a0c, 0x78);
	write_cmos_sensor(0x800, 0x00);
	write_cmos_sensor(0x200, 0x200);
	write_cmos_sensor(0x202, 0x100);
	write_cmos_sensor(0x21c, 0x200);
	write_cmos_sensor(0x21e, 0x100);
	write_cmos_sensor(0x342, 0x141c);
	write_cmos_sensor(0x340, 0x708);
	write_cmos_sensor(0x114, 0x300);
	write_cmos_sensor(0x3072, 0x3c0);
	write_cmos_sensor(0x100, 0x100);
	write_cmos_sensor(0x101, 0x00);

	return ERROR_NONE;
}

static void set_dummy(void)
{
	write_cmos_sensor(0x0340, imgsensor.frame_length);
	write_cmos_sensor(0x0342, imgsensor.line_length);
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

	shutter = (shutter < imgsensor_info.min_shutter)
		  ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter >
		   (imgsensor_info.max_frame_length - imgsensor_info.margin))
		  ? (imgsensor_info.max_frame_length - imgsensor_info.margin)
		  : shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 /
			       imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else
			write_cmos_sensor(0x0340, imgsensor.frame_length);
	} else {
		write_cmos_sensor(0x0340, imgsensor.frame_length);
	}

	write_cmos_sensor(0x0202, shutter);

	LOG_INF("shutter =%d, framelength =%d\n",
		shutter, imgsensor.frame_length);
}

static void set_shutter(kal_uint32 shutter)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.shutter = shutter;
	spin_unlock(&imgsensor_drv_lock);

	write_shutter(shutter);
}

/*
 * gain2reg() was reconstructed from the factory kernel disassembly (0xffffff8008721108).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_s5k3p3sx_mipi_raw_s5k3p3sxmipi_Sensor.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static kal_uint16 gain2reg(const kal_uint16 gain)
{
	/* "53013eb3 ubfx"@0xffffff80087210d0 -- gain >> 1, fifteen bit */
	return (kal_uint16)(gain >> 1);
}

static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;

	/*
	 * The range is 64..2048, not 64..1024:
	 * "510102a8 sub"@0xffffff8008721084 subtracts 0x40 and
	 * "711f051f cmp"@0xffffff8008721088 compares with 0x7c1, that is, it
	 * accepts anything between 64 and 2048 without correction.
	 */
	if (gain < BASEGAIN || gain > 32 * BASEGAIN) {
		LOG_INF("Error gain setting\n");

		if (gain < BASEGAIN)
			gain = BASEGAIN;
		else if (gain > 32 * BASEGAIN)
			gain = 32 * BASEGAIN;
	}

	reg_gain = gain2reg(gain);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain;
	spin_unlock(&imgsensor_drv_lock);

	write_cmos_sensor_8(0x0204, (reg_gain >> 8) & 0xff);
	write_cmos_sensor_8(0x0205, reg_gain & 0xff);

	return gain;
}

static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	/*
	 * write_cmos_sensor() was reconstructed from the factory kernel disassembly (0xffffff80087219e8).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_s5k3p3sx_mipi_raw_s5k3p3sxmipi_Sensor.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	write_cmos_sensor(0x0100, 0x0000);
	/*
	 * "941cd7df bl"@0xffffff8008721af8 -- one millisecond for the frame in
	 * flight to finish before reconfiguring
	 */
	mdelay(1);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	imgsensor.pclk = imgsensor_info.cap.pclk;
	imgsensor.line_length = imgsensor_info.cap.linelength;
	imgsensor.frame_length = imgsensor_info.cap.framelength;
	imgsensor.min_frame_length = imgsensor_info.cap.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	preview(image_window, sensor_config_data);

	return ERROR_NONE;
}

static kal_uint32 normal_video(
	MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\n");

	/*
	 * write_cmos_sensor() was reconstructed from the factory kernel disassembly (0xffffff80087219e8).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_s5k3p3sx_mipi_raw_s5k3p3sxmipi_Sensor.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	write_cmos_sensor(0x0100, 0x0000);
	/*
	 * "941cd7df bl"@0xffffff8008721af8 -- one millisecond for the frame in
	 * flight to finish before reconfiguring
	 */
	mdelay(1);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	preview(image_window, sensor_config_data);

	return ERROR_NONE;
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
			*sensor_id = read_sensor_id();
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
			sensor_id = read_sensor_id();
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

static kal_uint32 close(void)
{
	LOG_INF("E\n");

	return ERROR_NONE;
}

/*
 * get_resolution @0xffffff8008720e7c, 100 bytes.
 *
 * The ten sizes come out of three `movk`s in a row: 0x0910 = 2320 and 0x06d4 = 1748
 * for four scenarios, 0x0780 = 1920 and 0x0438 = 1080 for the fast video.
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

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008720e58, 200 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_s5k3p3sx_mipi_raw_s5k3p3sxmipi_Sensor.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	sensor_info->PDAF_Support = 0;

	return ERROR_NONE;
}

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

	if (enable)
		write_cmos_sensor(0x0600, 0x0002);
	else
		write_cmos_sensor(0x0600, 0x0000);

	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}

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
			/* the table has four entries: the last is index 3 */
			memcpy((void *)wininfo,
			       (void *)&imgsensor_winsize_info[3],
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
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008f4a198).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_imgsensor_src_mt6771_camera_project_e977_dg_m13_71_q0_s5k3p3sx_mipi_raw_s5k3p3sxmipi_Sensor.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	case SENSOR_FEATURE_GET_PDAF_INFO:
		LOG_INF("SENSOR_FEATURE_GET_PDAF_INFO scenarioId:%llu\n",
			*feature_data);
		/* capture only: "f100051f cmp"@0xffffff800872161c */
		if (*feature_data == MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG)
			memcpy((void *)(uintptr_t)(*(feature_data + 1)),
			       (void *)&imgsensor_pd_info,
			       sizeof(struct SET_PD_BLOCK_INFO_T));
		break;
	case SENSOR_FEATURE_GET_PDAF_DATA:
		/* in the factory build it is only a message: no data is copied */
		LOG_INF("SENSOR_FEATURE_GET_PDAF_DATA\n");
		break;
	case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
		LOG_INF("SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY scenarioId:%llu\n",
			*feature_data);
		/*
		 * Five scenarios, and only the second answers one: the table at
		 * 0xffffff8008f4a260 is {0x00, 0x97, 0x00, 0x00, 0x00} and only
		 * the 0x97 entry leads to the block that writes 1
		 * ("320003e9 orr"@0xffffff80087218f0).
		 */
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 1;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 0;
			break;
		}
		break;
	case SENSOR_FEATURE_SET_HDR_SHUTTER:
		LOG_INF("SENSOR_FEATURE_SET_HDR_SHUTTER LE=%d, SE=%d\n",
			(UINT16) *feature_data, (UINT16) *(feature_data + 1));
		break;
	default:
		break;
	}

	return ERROR_NONE;
}


#define S5K3P3SX_EEPROM_I2C_ADDR 0xa2
#define S5K3P3SX_MAX_PDAF_SIZE   1404

static kal_uint8 s5k3p3sx_eeprom_data[S5K3P3SX_MAX_PDAF_SIZE];

/*
 * read_3P3_eeprom @0xffffff80087225d4, 432 bytes, a GLOBAL symbol (T).
 *
 * It reads the phase detection calibration from the external memory. The two
 * messages here are `__dynamic_pr_debug` in the disassembly, not `printk`:
 * they are the only real pr_debugs in the driver, everything else is pr_info.
 */
UINT32 read_3P3_eeprom(kal_uint16 addr, char *data, kal_uint32 size)
{
	char pu_send_cmd[2] = { (char)(addr >> 8), (char)(addr & 0xFF) };

	pr_debug("read_3P3_eeprom addr = 0x%x, size = %d\n", addr, size);

	kdSetI2CSpeed(400);

	if (iReadRegI2C(pu_send_cmd, 2, (u8 *)s5k3p3sx_eeprom_data,
			size, S5K3P3SX_EEPROM_I2C_ADDR) < 0) {
		pr_debug("read_3P3_eeprom fail\n");
		return 0;
	}

	memcpy(data, s5k3p3sx_eeprom_data, size);

	return size;
}


static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 S5K3P3SX_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc = &sensor_func;

	return ERROR_NONE;
}

