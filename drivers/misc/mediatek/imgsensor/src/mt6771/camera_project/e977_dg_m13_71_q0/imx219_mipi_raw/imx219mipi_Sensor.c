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
 *	 imx219mipi_Sensor.c
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

#include "imx219mipi_Sensor.h"

#define PFX "imx219_camera_sensor"
#define LOG_INF(format, args...)    \
	pr_info(PFX "[%s] " format, __func__, ##args)

static DEFINE_SPINLOCK(imgsensor_drv_lock);

static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = IMX219_SENSOR_ID,

	.checksum_value = 0x21d70000,

	.pre = {
		.pclk = 137600000,
		.linelength = 3448,
		.framelength = 1332,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1632,
		.grabwindow_height = 1224,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.cap = {
		.pclk = 137600000,
		.linelength = 3448,
		.framelength = 1332,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.normal_video = {
		.pclk = 137600000,
		.linelength = 3448,
		.framelength = 1332,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 3264,
		.grabwindow_height = 2448,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.hs_video = {
		.pclk = 137600000,
		.linelength = 3448,
		.framelength = 1332,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1920,
		.grabwindow_height = 1080,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},
	.slim_video = {
		.pclk = 137600000,
		.linelength = 3448,
		.framelength = 1332,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1280,
		.grabwindow_height = 720,
		.mipi_data_lp2hs_settle_dc = 85,
		.max_framerate = 300,
	},

	.margin = 10,
	.min_shutter = 4,
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
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_R,
	.mclk = 24,
	.mipi_lane_num = SENSOR_MIPI_2_LANE,
	.i2c_addr_table = {0x21, 0xff},
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
	.i2c_write_id = 0x21,
};

static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[5] = {
	{3264, 2448,    0,    0, 3264, 2448, 1632, 1224,
	    0,    0, 1632, 1224,    0,    0, 1632, 1224},
	{3264, 2448,    0,    0, 3264, 2448, 3264, 2448,
	    0,    0, 3264, 2448,    0,    0, 3264, 2448},
	{3264, 2448,    0,    0, 3264, 2448, 3264, 2448,
	    0,    0, 3264, 2448,    0,    0, 3264, 2448},
	{3264, 2448,    0,  684, 3264, 1080, 1920, 1080,
	    0,    0, 1920, 1080,    0,    0, 1920, 1080},
	{3264, 2448,    0,    0, 3264, 2448, 1280,  720,
	    0,    0, 1280,  720,    0,    0, 1280,  720},
};

/*
 * Samsung legge e scrive con l'indirizzo a SEDICI bit: il buffer e' di
 * quattro byte, non di due, e le due meta' vanno spezzate a mano.
 */
static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[2] = { (char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pu_send_cmd, 2, (u8 *)&get_byte, 1,
		    imgsensor.i2c_write_id);

	return get_byte;
}

/*
 * TRE byte: indirizzo a sedici bit, dato a otto. Samsung ne usa quattro,
 * Sony tre, e GalaxyCore due -- e sono tre driver che si somigliano in tutto
 * il resto.
 */
static void write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[3] = { (char)(addr >> 8), (char)(addr & 0xFF),
				(char)(para & 0xFF) };

	iWriteRegI2C(pu_send_cmd, 3, imgsensor.i2c_write_id);
}

static void write_cmos_sensor_16(kal_uint32 addr, kal_uint32 para)
{
	char pu_send_cmd[4] = { (char)(addr >> 8), (char)(addr & 0xFF),
				(char)(para >> 8), (char)(para & 0xFF) };

	iWriteRegI2C(pu_send_cmd, 4, imgsensor.i2c_write_id);
}


static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data);


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

	write_cmos_sensor(0x01, 0x00);
	write_cmos_sensor(0x30eb, 0x05);
	write_cmos_sensor(0x30eb, 0x0c);
	write_cmos_sensor(0x300a, 0xff);
	write_cmos_sensor(0x300b, 0xff);
	write_cmos_sensor(0x30eb, 0x05);
	write_cmos_sensor(0x30eb, 0x09);
	write_cmos_sensor(0x114, 0x03);
	write_cmos_sensor(0x01, 0x28);
	write_cmos_sensor(0x12a, 0x18);
	write_cmos_sensor(0x01, 0x2b);
	write_cmos_sensor(0x160, 0x05);
	write_cmos_sensor(0x161, 0x34);
	write_cmos_sensor(0x162, 0x0d);
	write_cmos_sensor(0x163, 0x78);
	write_cmos_sensor(0x01, 0x64);
	write_cmos_sensor(0x01, 0x65);
	write_cmos_sensor(0x166, 0x0c);
	write_cmos_sensor(0x167, 0xcf);
	write_cmos_sensor(0x01, 0x68);
	write_cmos_sensor(0x01, 0x69);
	write_cmos_sensor(0x16a, 0x09);
	write_cmos_sensor(0x16b, 0x9f);
	write_cmos_sensor(0x16c, 0x06);
	write_cmos_sensor(0x16d, 0x68);
	write_cmos_sensor(0x16e, 0x04);
	write_cmos_sensor(0x16f, 0xd0);
	write_cmos_sensor(0x170, 0x01);
	write_cmos_sensor(0x171, 0x01);
	write_cmos_sensor(0x174, 0x01);
	write_cmos_sensor(0x175, 0x01);
	write_cmos_sensor(0x18c, 0x0a);
	write_cmos_sensor(0x18d, 0x0a);
	write_cmos_sensor(0x301, 0x05);
	write_cmos_sensor(0x303, 0x01);
	write_cmos_sensor(0x304, 0x03);
	write_cmos_sensor(0x305, 0x03);
	write_cmos_sensor(0x03, 0x06);
	write_cmos_sensor(0x307, 0x2b);
	write_cmos_sensor(0x309, 0x0a);
	write_cmos_sensor(0x30b, 0x01);
	write_cmos_sensor(0x03, 0x0c);
	write_cmos_sensor(0x30d, 0x2e);
	write_cmos_sensor(0x45, 0x5e);
	write_cmos_sensor(0x471e, 0x4b);
	write_cmos_sensor(0x4767, 0x0f);
	write_cmos_sensor(0x4750, 0x14);
	write_cmos_sensor(0x45, 0x40);
	write_cmos_sensor(0x47b4, 0x14);
	write_cmos_sensor(0x4713, 0x30);
	write_cmos_sensor(0x478b, 0x10);
	write_cmos_sensor(0x478f, 0x10);
	write_cmos_sensor(0x4793, 0x10);
	write_cmos_sensor(0x4797, 0x0e);
	write_cmos_sensor(0x479b, 0x0e);
	write_cmos_sensor(0x100, 0x01);
	write_cmos_sensor(0x01, 0x72);
	write_cmos_sensor(0x01, 0x72);

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

	write_cmos_sensor(0x01, 0x00);
	write_cmos_sensor(0x30eb, 0x05);
	write_cmos_sensor(0x30eb, 0x0c);
	write_cmos_sensor(0x300a, 0xff);
	write_cmos_sensor(0x300b, 0xff);
	write_cmos_sensor(0x30eb, 0x05);
	write_cmos_sensor(0x30eb, 0x09);
	write_cmos_sensor(0x114, 0x03);
	write_cmos_sensor(0x01, 0x28);
	write_cmos_sensor(0x12a, 0x18);
	write_cmos_sensor(0x01, 0x2b);
	write_cmos_sensor(0x160, 0x04);
	write_cmos_sensor(0x161, 0x8e);
	write_cmos_sensor(0x162, 0x0d);
	write_cmos_sensor(0x163, 0x78);
	write_cmos_sensor(0x164, 0x02);
	write_cmos_sensor(0x165, 0xa8);
	write_cmos_sensor(0x166, 0x0a);
	write_cmos_sensor(0x167, 0x27);
	write_cmos_sensor(0x168, 0x02);
	write_cmos_sensor(0x169, 0xb4);
	write_cmos_sensor(0x16a, 0x06);
	write_cmos_sensor(0x16b, 0xeb);
	write_cmos_sensor(0x16c, 0x07);
	write_cmos_sensor(0x16d, 0x80);
	write_cmos_sensor(0x16e, 0x04);
	write_cmos_sensor(0x16f, 0x38);
	write_cmos_sensor(0x170, 0x01);
	write_cmos_sensor(0x171, 0x01);
	write_cmos_sensor(0x01, 0x74);
	write_cmos_sensor(0x01, 0x75);
	write_cmos_sensor(0x18c, 0x0a);
	write_cmos_sensor(0x18d, 0x0a);
	write_cmos_sensor(0x301, 0x05);
	write_cmos_sensor(0x303, 0x01);
	write_cmos_sensor(0x304, 0x03);
	write_cmos_sensor(0x305, 0x03);
	write_cmos_sensor(0x03, 0x06);
	write_cmos_sensor(0x307, 0x57);
	write_cmos_sensor(0x309, 0x0a);
	write_cmos_sensor(0x30b, 0x01);
	write_cmos_sensor(0x03, 0x0c);
	write_cmos_sensor(0x03, 0x0d);

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

	write_cmos_sensor(0x01, 0x00);
	write_cmos_sensor(0x30eb, 0x05);
	write_cmos_sensor(0x30eb, 0x0c);
	write_cmos_sensor(0x300a, 0xff);
	write_cmos_sensor(0x300b, 0xff);
	write_cmos_sensor(0x30eb, 0x05);
	write_cmos_sensor(0x30eb, 0x09);
	write_cmos_sensor(0x114, 0x03);
	write_cmos_sensor(0x01, 0x28);
	write_cmos_sensor(0x12a, 0x18);
	write_cmos_sensor(0x01, 0x2b);
	write_cmos_sensor(0x160, 0x09);
	write_cmos_sensor(0x161, 0xf0);
	write_cmos_sensor(0x162, 0x0d);
	write_cmos_sensor(0x163, 0x78);
	write_cmos_sensor(0x01, 0x64);
	write_cmos_sensor(0x01, 0x65);
	write_cmos_sensor(0x166, 0x0c);
	write_cmos_sensor(0x167, 0xcf);
	write_cmos_sensor(0x168, 0x01);
	write_cmos_sensor(0x169, 0x32);
	write_cmos_sensor(0x16a, 0x08);
	write_cmos_sensor(0x16b, 0x6d);
	write_cmos_sensor(0x16c, 0x06);
	write_cmos_sensor(0x16d, 0x68);
	write_cmos_sensor(0x16e, 0x03);
	write_cmos_sensor(0x16f, 0x9e);
	write_cmos_sensor(0x170, 0x01);
	write_cmos_sensor(0x171, 0x01);
	write_cmos_sensor(0x174, 0x01);
	write_cmos_sensor(0x175, 0x01);
	write_cmos_sensor(0x18c, 0x0a);
	write_cmos_sensor(0x18d, 0x0a);
	write_cmos_sensor(0x301, 0x05);
	write_cmos_sensor(0x303, 0x01);
	write_cmos_sensor(0x304, 0x03);
	write_cmos_sensor(0x305, 0x03);
	write_cmos_sensor(0x03, 0x06);
	write_cmos_sensor(0x307, 0x2b);
	write_cmos_sensor(0x309, 0x0a);
	write_cmos_sensor(0x30b, 0x01);
	write_cmos_sensor(0x03, 0x0c);
	write_cmos_sensor(0x30d, 0x2e);
	write_cmos_sensor(0x45, 0x5e);
	write_cmos_sensor(0x471e, 0x4b);
	write_cmos_sensor(0x4767, 0x0f);
	write_cmos_sensor(0x4750, 0x14);
	write_cmos_sensor(0x45, 0x40);
	write_cmos_sensor(0x47b4, 0x14);
	write_cmos_sensor(0x4713, 0x30);
	write_cmos_sensor(0x478b, 0x10);
	write_cmos_sensor(0x478f, 0x10);
	write_cmos_sensor(0x4793, 0x10);
	write_cmos_sensor(0x4797, 0x0e);
	write_cmos_sensor(0x479b, 0x0e);
	write_cmos_sensor(0x100, 0x01);
	write_cmos_sensor(0x01, 0x72);
	write_cmos_sensor(0x01, 0x72);

	return ERROR_NONE;
}

static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;
	imgsensor.pclk = imgsensor_info.cap.pclk;
	imgsensor.line_length = imgsensor_info.cap.linelength;
	imgsensor.frame_length = imgsensor_info.cap.framelength;
	imgsensor.min_frame_length = imgsensor_info.cap.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	write_cmos_sensor(0x01, 0x00);
	write_cmos_sensor(0x30eb, 0x05);
	write_cmos_sensor(0x30eb, 0x0c);
	write_cmos_sensor(0x300a, 0xff);
	write_cmos_sensor(0x300b, 0xff);
	write_cmos_sensor(0x30eb, 0x05);
	write_cmos_sensor(0x30eb, 0x09);
	write_cmos_sensor(0x114, 0x03);
	write_cmos_sensor(0x01, 0x28);
	write_cmos_sensor(0x12a, 0x18);
	write_cmos_sensor(0x01, 0x2b);
	write_cmos_sensor(0x160, 0x09);
	write_cmos_sensor(0x161, 0xf0);
	write_cmos_sensor(0x162, 0x0d);
	write_cmos_sensor(0x163, 0x78);
	write_cmos_sensor(0x01, 0x64);
	write_cmos_sensor(0x01, 0x65);
	write_cmos_sensor(0x166, 0x0c);
	write_cmos_sensor(0x167, 0xcf);
	write_cmos_sensor(0x01, 0x68);
	write_cmos_sensor(0x01, 0x69);
	write_cmos_sensor(0x16a, 0x09);
	write_cmos_sensor(0x16b, 0x9f);
	write_cmos_sensor(0x16c, 0x0c);
	write_cmos_sensor(0x16d, 0xd0);
	write_cmos_sensor(0x16e, 0x09);
	write_cmos_sensor(0x16f, 0xa0);
	write_cmos_sensor(0x170, 0x01);
	write_cmos_sensor(0x171, 0x01);
	write_cmos_sensor(0x01, 0x74);
	write_cmos_sensor(0x01, 0x75);
	write_cmos_sensor(0x18c, 0x0a);
	write_cmos_sensor(0x18d, 0x0a);
	write_cmos_sensor(0x301, 0x05);
	write_cmos_sensor(0x303, 0x01);
	write_cmos_sensor(0x304, 0x03);
	write_cmos_sensor(0x305, 0x03);
	write_cmos_sensor(0x03, 0x06);
	write_cmos_sensor(0x307, 0x53);
	write_cmos_sensor(0x309, 0x0a);
	write_cmos_sensor(0x30b, 0x01);
	write_cmos_sensor(0x03, 0x0c);
	write_cmos_sensor(0x30d, 0x56);
	write_cmos_sensor(0x45, 0x5e);
	write_cmos_sensor(0x471e, 0x4b);
	write_cmos_sensor(0x4767, 0x0f);
	write_cmos_sensor(0x4750, 0x14);
	write_cmos_sensor(0x45, 0x40);
	write_cmos_sensor(0x47b4, 0x14);
	write_cmos_sensor(0x4713, 0x30);
	write_cmos_sensor(0x478b, 0x10);
	write_cmos_sensor(0x478f, 0x10);
	write_cmos_sensor(0x4793, 0x10);
	write_cmos_sensor(0x4797, 0x0e);
	write_cmos_sensor(0x479b, 0x0e);
	write_cmos_sensor(0x100, 0x01);
	write_cmos_sensor(0x01, 0x72);
	write_cmos_sensor(0x01, 0x72);

	return ERROR_NONE;
}

static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
	 MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("E\\n");

	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	write_cmos_sensor(0x01, 0x00);
	write_cmos_sensor(0x30eb, 0x05);
	write_cmos_sensor(0x30eb, 0x0c);
	write_cmos_sensor(0x300a, 0xff);
	write_cmos_sensor(0x300b, 0xff);
	write_cmos_sensor(0x30eb, 0x05);
	write_cmos_sensor(0x30eb, 0x09);
	write_cmos_sensor(0x114, 0x03);
	write_cmos_sensor(0x01, 0x28);
	write_cmos_sensor(0x12a, 0x18);
	write_cmos_sensor(0x01, 0x2b);
	write_cmos_sensor(0x160, 0x09);
	write_cmos_sensor(0x161, 0xf0);
	write_cmos_sensor(0x162, 0x0d);
	write_cmos_sensor(0x163, 0x78);
	write_cmos_sensor(0x01, 0x64);
	write_cmos_sensor(0x01, 0x65);
	write_cmos_sensor(0x166, 0x0c);
	write_cmos_sensor(0x167, 0xcf);
	write_cmos_sensor(0x01, 0x68);
	write_cmos_sensor(0x01, 0x69);
	write_cmos_sensor(0x16a, 0x09);
	write_cmos_sensor(0x16b, 0x9f);
	write_cmos_sensor(0x16c, 0x0c);
	write_cmos_sensor(0x16d, 0xd0);
	write_cmos_sensor(0x16e, 0x09);
	write_cmos_sensor(0x16f, 0xa0);
	write_cmos_sensor(0x170, 0x01);
	write_cmos_sensor(0x171, 0x01);
	write_cmos_sensor(0x01, 0x74);
	write_cmos_sensor(0x01, 0x75);
	write_cmos_sensor(0x18c, 0x0a);
	write_cmos_sensor(0x18d, 0x0a);
	write_cmos_sensor(0x301, 0x05);
	write_cmos_sensor(0x303, 0x01);
	write_cmos_sensor(0x304, 0x03);
	write_cmos_sensor(0x305, 0x03);
	write_cmos_sensor(0x03, 0x06);
	write_cmos_sensor(0x307, 0x53);
	write_cmos_sensor(0x309, 0x0a);
	write_cmos_sensor(0x30b, 0x01);
	write_cmos_sensor(0x03, 0x0c);
	write_cmos_sensor(0x03, 0x0d);

	return ERROR_NONE;
}

static void set_test_pattern_scritture(kal_bool enable)
{
	if (!enable)
		return;

	write_cmos_sensor(0x06, 0x00);
	write_cmos_sensor(0x601, 0x02);
	write_cmos_sensor(0x624, 0x06);
	write_cmos_sensor(0x625, 0x68);
	write_cmos_sensor(0x626, 0x04);
	write_cmos_sensor(0x627, 0xd0);
	write_cmos_sensor(0x61, 0x28);
	write_cmos_sensor(0x6129, 0x02);
	write_cmos_sensor(0x613c, 0x06);
	write_cmos_sensor(0x06, 0x00);
	write_cmos_sensor(0x601, 0x02);
	write_cmos_sensor(0x624, 0x0c);
	write_cmos_sensor(0x625, 0xd0);
	write_cmos_sensor(0x626, 0x09);
	write_cmos_sensor(0x627, 0xa0);
	write_cmos_sensor(0x61, 0x28);
	write_cmos_sensor(0x6129, 0x02);
	write_cmos_sensor(0x613c, 0x0c);
	write_cmos_sensor(0x613d, 0xd0);
	write_cmos_sensor(0x613e, 0x09);
	write_cmos_sensor(0x613f, 0xa0);
	write_cmos_sensor(0x06, 0x00);
	write_cmos_sensor(0x06, 0x01);
	write_cmos_sensor(0x624, 0x06);
	write_cmos_sensor(0x625, 0x68);
	write_cmos_sensor(0x626, 0x04);
	write_cmos_sensor(0x627, 0xd0);
	write_cmos_sensor(0x61, 0x28);
	write_cmos_sensor(0x6129, 0x02);
	write_cmos_sensor(0x613c, 0x06);
	write_cmos_sensor(0x613d, 0x68);
	write_cmos_sensor(0x613e, 0x04);
	write_cmos_sensor(0x613f, 0xd0);
	write_cmos_sensor(0x65, 0x06);
	write_cmos_sensor(0x65, 0x07);
}

static void set_dummy(void)
{
	write_cmos_sensor_16(0x0160, imgsensor.frame_length);
	write_cmos_sensor_16(0x0162, imgsensor.line_length);
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
			write_cmos_sensor_16(0x0160, imgsensor.frame_length);
	} else {
		write_cmos_sensor_16(0x0160, imgsensor.frame_length);
	}

	write_cmos_sensor_16(0x015a, shutter);

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

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = gain / 2;

	return (kal_uint16)reg_gain;
}

static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain;

	if (gain < BASEGAIN || gain > 16 * BASEGAIN) {
		LOG_INF("Error gain setting\n");

		if (gain < BASEGAIN)
			gain = BASEGAIN;
		else if (gain > 16 * BASEGAIN)
			gain = 16 * BASEGAIN;
	}

	reg_gain = gain2reg(gain);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain;
	spin_unlock(&imgsensor_drv_lock);

	write_cmos_sensor(0x0157, reg_gain & 0xff);

	return gain;
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
			*sensor_id = read_cmos_sensor(0x0000);
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
			sensor_id = read_cmos_sensor(0x0000);
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
 * get_resolution @0xffffff8008720e7c, 100 byte.
 *
 * Le dieci misure escono da tre `movk` in fila: 0x0910 = 2320 e 0x06d4 = 1748
 * per quattro scenari, 0x0780 = 1920 e 0x0438 = 1080 per il video veloce.
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

	set_test_pattern_scritture(enable);

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

UINT32 IMX219_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc = &sensor_func;

	return ERROR_NONE;
}

