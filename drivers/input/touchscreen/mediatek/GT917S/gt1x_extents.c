/* drivers/input/touchscreen/gt1x_extents.c
 *
 * 2010 - 2014 Goodix Technology.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be a reference
 * to you, when you are integrating the GOODiX's CTP IC into your system,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Version: 1.0
 * Revision Record:
 *      V1.0:  first release. 2014/09/28.
 *
 */

/*
 * Goodix GT917S touch panel, Doogee S88 Pro -- the "extents" block.
 *
 * This is a correction against the factory binary, not a rewrite: the
 * starting point was the ALPS copy
 * (drivers/input/touchscreen/mediatek/GT5688/), brought in by an earlier
 * survey. Factory block: [0xffffff8008a7221c, 0xffffff8008a73854) minus
 * gt1x_gesture_debug (which lives in gt1x_wtk.c) = 5556 bytes, 11 functions,
 * measured with the factory compiler (clang r353983c, LLVM 9.0.3).
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_extents.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/wait.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/proc_fs.h> /*proc */
#include <linux/ratelimit.h>
#include <linux/uaccess.h>
#ifdef CONFIG_GTP_REQUEST_FW_UPDATE
#include <linux/firmware.h>
#endif
#include "include/gt1x_tpd_common.h"
#include <asm/ioctl.h>

#define GT1151_FW_SIZE 5000
#ifdef CONFIG_GTP_REQUEST_FW_UPDATE
#define GT1151_PATCH_JUMP_FW "gt1151_patch_jump_"
#define HOTKNOT_AUTH_FW "gt1151_hotknot_auth_"
#endif
/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_extents.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
unsigned char gt1x_patch_jump_fw[GT1151_FW_SIZE];
unsigned char hotknot_auth_fw[GT1151_FW_SIZE];
#ifdef CONFIG_GTP_GESTURE_WAKEUP

#define GESTURE_NODE "goodix_gesture"
#define GESTURE_MAX_POINT_COUNT 64

#pragma pack(1)
typedef struct {
	u8 ic_msg[6]; /*from the first byte */
	u8 gestures[4];
	u8 data[3 + GESTURE_MAX_POINT_COUNT * 4 +
		80]; /*80 bytes for extra data */
} st_gesture_data;
#pragma pack()

#define SETBIT(longlong, bit) (longlong[bit / 8] |= (1 << bit % 8))
#define CLEARBIT(longlong, bit) (longlong[bit / 8] &= (~(1 << bit % 8)))
#define QUERYBIT(longlong, bit) (!!(longlong[bit / 8] & (1 << bit % 8)))

int gesture_enabled = 1;
enum DOZE_T gesture_doze_status = DOZE_DISABLED;

/*
 * NOT `static`: gt1x_wtk.c -- another translation unit -- reads it and
 * writes it (lines 509 and 512), so in the factory build it is global.  The GT5688
 * copy this file comes from kept it static, because there no other file
 * touches it.  This too is a difference no SIZE measurement can
 * see -- the generated code is the same -- and one the link finds at once.
 */
u8 gestures_flag[32];
static st_gesture_data gesture_data;
static struct mutex gesture_data_mutex;
static u8 c6fc;

static ssize_t gt1x_gesture_data_read(struct file *file, char __user *page,
				      size_t size, loff_t *ppos)
{
	s32 ret = -1;

#line 69
	GTP_DEBUG("visit gt1x_gesture_data_read. ppos:%d", (int)*ppos);
	if (*ppos)
		return 0;
	if (size == 4) {
		ret = copy_to_user(((u8 __user *)page), "GT1X", 4);
		return 4;
	}
	ret = simple_read_from_buffer(page, size, ppos, &gesture_data,
				      sizeof(gesture_data));

	GTP_DEBUG("Got the gesture data.");
	return ret;
}

static ssize_t gt1x_gesture_data_write(struct file *filp,
				       const char __user *buff, size_t len,
				       loff_t *off)
{
	s32 ret = 0;

	GTP_DEBUG_FUNC();

	ret = copy_from_user(&gesture_enabled, buff, 1);
	if (ret) {
#line 91
		GTP_ERROR("copy_from_user failed.");
		return -EPERM;
	}

	GTP_DEBUG("gesture enabled:%x, ret:%d", gesture_enabled, ret);

	return len;
}

int gesture_enter_doze(void)
{
	int retry = 0;

	GTP_DEBUG_FUNC();
#line 143
	GTP_DEBUG("Entering doze mode...");
	while (retry++ < 5) {
		if (!gt1x_send_cmd(0x08, 0)) {
			gesture_doze_status = DOZE_ENABLED;
			GTP_DEBUG("Working in doze mode!");
			return 0;
		}
		msleep(10);
	}
	GTP_ERROR("Send doze cmd failed.");
	return -1;
}

s32 gesture_event_handler(struct input_dev *dev)
{
	u8 doze_buf[4] = {0};
	s32 ret = -1;
	int len, extra_len;
	u8 gesture, flag;
	u8 chksum;
	int i;

	if (gesture_doze_status == DOZE_ENABLED) {
		ret = gt1x_i2c_read(GTP_REG_WAKEUP_GESTURE, doze_buf, 4);
		if (ret < 0)
			return 0;
		gesture = doze_buf[0];
		len = doze_buf[1];
		flag = doze_buf[2];
		extra_len = doze_buf[3];
		GTP_DEBUG("0x%x = 0x%02X,0x%02X,0x%02X,0x%02X",
#line 186
			  GTP_REG_WAKEUP_GESTURE, gesture, len, flag, extra_len);
		if (len > GESTURE_MAX_POINT_COUNT) {
#line 189
			GTP_ERROR("Gesture contain too many points!(%d)", len);
			len = GESTURE_MAX_POINT_COUNT;
		}
		if (extra_len > 32) {
			GTP_ERROR("Gesture contain too many extra data!(%d)",
				  extra_len);
			extra_len = 32;
		}
		{
			u8 buf[extra_len + 1];

			ret = gt1x_i2c_read(GTP_REG_WAKEUP_GESTURE + 4, buf,
					    extra_len + 1);
			if (ret < 0) {
#line 206
				GTP_ERROR("Read extra gesture data failed.");
				return 0;
			}
			if (flag & 0x80) {
				buf[extra_len] += doze_buf[0] + doze_buf[1] +
						  doze_buf[2] + doze_buf[3];
				chksum = 0;
				for (i = 0; i < extra_len + 1; i++)
					chksum += buf[i];
				if (chksum) {
#line 218
					GTP_ERROR("Gesture checksum error.");
					if (c6fc) {
						c6fc = 0;
						ret = 0;
						goto clear_gesture_reg;
					}
					c6fc = 1;
					return 0;
				}
				c6fc = 0;
			}
			mutex_lock(&gesture_data_mutex);
			memcpy(&gesture_data.data[4 + len * 4], buf, extra_len);
			mutex_unlock(&gesture_data_mutex);
		}
		if (!gesture || !QUERYBIT(gestures_flag, gesture)) {
			GTP_INFO("Gesture[0x%02X] has been disabled.",
#line 242
				 doze_buf[0]);
			doze_buf[0] = 0;
			gt1x_i2c_write(GTP_REG_WAKEUP_GESTURE, doze_buf, 1);
			gesture_enter_doze();
			return 0;
		}
		if (len) {
			u8 coor_buf[len * 4 + 2];

			ret = gt1x_i2c_read(GTP_REG_WAKEUP_GESTURE_DETAIL,
					    coor_buf, len * 4);
			if (ret < 0) {
#line 255
				GTP_ERROR("Read gesture data failed.");
				return 0;
			}
			ret = gt1x_i2c_read(0x819F, &coor_buf[len * 4], 2);
			if (ret < 0) {
#line 262
				GTP_ERROR("Read gesture data failed.");
				return 0;
			}
			mutex_lock(&gesture_data_mutex);
			memcpy(&gesture_data.data[4], coor_buf, len * 4);
			mutex_unlock(&gesture_data_mutex);
		}
		mutex_lock(&gesture_data_mutex);
		gesture_data.data[0] = gesture; /*gesture type*/
		gesture_data.data[1] = len; /*gesture points number*/
		gesture_data.data[2] = doze_buf[2] & 0x7f;
		gesture_data.data[3] = extra_len;
		mutex_unlock(&gesture_data_mutex);

		GTP_DEBUG("--lan-- Gesture: 0x%02X, points: %d", doze_buf[0],
#line 296
			  doze_buf[1]);

		switch (gesture_data.data[0]) {
		case 0x5e:
			input_report_key(dev, KEY_POWER, 1);
			input_sync(dev);
			input_report_key(dev, KEY_POWER, 0);
			input_sync(dev);
			break;
		case 0x63:
			input_report_key(dev, KEY_C, 1);
			input_sync(dev);
			input_report_key(dev, KEY_C, 0);
			input_sync(dev);
			break;
		case 0x65:
			input_report_key(dev, KEY_E, 1);
			input_sync(dev);
			input_report_key(dev, KEY_E, 0);
			input_sync(dev);
			break;
		case 0x6d:
			input_report_key(dev, KEY_M, 1);
			input_sync(dev);
			input_report_key(dev, KEY_M, 0);
			input_sync(dev);
			break;
		case 0x6f:
			input_report_key(dev, KEY_O, 1);
			input_sync(dev);
			input_report_key(dev, KEY_O, 0);
			input_sync(dev);
			break;
		case 0x73:
			input_report_key(dev, KEY_S, 1);
			input_sync(dev);
			input_report_key(dev, KEY_S, 0);
			input_sync(dev);
			break;
		case 0x76:
			input_report_key(dev, KEY_V, 1);
			input_sync(dev);
			input_report_key(dev, KEY_V, 0);
			input_sync(dev);
			break;
		case 0x77:
			input_report_key(dev, KEY_W, 1);
			input_sync(dev);
			input_report_key(dev, KEY_W, 0);
			input_sync(dev);
			break;
		case 0x7a:
			input_report_key(dev, KEY_Z, 1);
			input_sync(dev);
			input_report_key(dev, KEY_Z, 0);
			input_sync(dev);
			break;
		case 0xaa:
			input_report_key(dev, KEY_RIGHT, 1);
			input_sync(dev);
			input_report_key(dev, KEY_RIGHT, 0);
			input_sync(dev);
			break;
		case 0xab:
			input_report_key(dev, KEY_DOWN, 1);
			input_sync(dev);
			input_report_key(dev, KEY_DOWN, 0);
			input_sync(dev);
			break;
		case 0xba:
			input_report_key(dev, KEY_UP, 1);
			input_sync(dev);
			input_report_key(dev, KEY_UP, 0);
			input_sync(dev);
			break;
		case 0xbb:
			input_report_key(dev, KEY_LEFT, 1);
			input_sync(dev);
			input_report_key(dev, KEY_LEFT, 0);
			input_sync(dev);
			break;
		case 0xcc:
			input_report_key(dev, KEY_F, 1);
			input_sync(dev);
			input_report_key(dev, KEY_F, 0);
			input_sync(dev);
			break;
		default:
			break;
		}

clear_gesture_reg:
		doze_buf[0] = 0;
		gt1x_i2c_write(GTP_REG_WAKEUP_GESTURE, doze_buf, 1);
		return ret;
	}
	return -1;
}

void gesture_clear_wakeup_data(void)
{
	mutex_lock(&gesture_data_mutex);
	memset(gesture_data.data, 0, 4);
	mutex_unlock(&gesture_data_mutex);
}
#endif /*CONFIG_GTP_GESTURE_WAKEUP*/

/*HotKnot module*/
#ifdef CONFIG_GTP_HOTKNOT

#define HOTKNOT_NODE "hotknot"

u8 hotknot_enabled;
u8 hotknot_transfer_mode;

static int hotknot_open(struct inode *node, struct file *flip)
{
#line 445
	GTP_DEBUG("Hotknot is enabled.");
	hotknot_enabled = 1;
	return 0;
}

static int hotknot_release(struct inode *node, struct file *filp)
{
	GTP_DEBUG("Hotknot is disabled.");
	hotknot_enabled = 0;
	return 0;
}

static s32 hotknot_enter_transfer_mode(void)
{
	int ret = 0;
	u8 buffer[5] = {0};

	hotknot_transfer_mode = 1;
#ifdef CONFIG_GTP_ESD_PROTECT
	gt1x_esd_switch(SWITCH_OFF);
#endif

	gt1x_irq_disable();
	gt1x_send_cmd(GTP_CMD_HN_TRANSFER, 0);
	msleep(100);
	gt1x_irq_enable();

	ret = gt1x_i2c_read(0x8140, buffer, sizeof(buffer));
	if (ret) {
		hotknot_transfer_mode = 0;
		return ret;
	}

	buffer[4] = 0;
	GTP_DEBUG("enter transfer mode: %s ", buffer);
	if (strcmp(buffer, "GHot")) {
		hotknot_transfer_mode = 0;
		return ERROR_HN_VER;
	}

	return 0;
}

static s32 hotknot_load_hotknot_subsystem(void)
{
	return hotknot_enter_transfer_mode();
}

static s32 hotknot_load_authentication_subsystem(void)
{
	s32 ret = 0;
	u8 buffer[5] = {0};

	ret = gt1x_hold_ss51_dsp_no_reset();
	if (ret < 0) {
#line 499
		GTP_ERROR("Hold ss51 fail!");
		return ERROR;
	}

	if (gt1x_chip_type == CHIP_TYPE_GT1X) {
		GTP_INFO("hotknot load jump code.");
		ret = gt1x_load_patch(gt1x_patch_jump_fw, 4096, 0, 1024 * 8);
		if (ret < 0) {
			GTP_ERROR("Load jump code fail!");
			return ret;
		}
		GTP_INFO("hotknot load auth code.");
		ret = gt1x_load_patch(hotknot_auth_fw, 4096, 4096, 1024 * 8);
		if (ret < 0) {
			GTP_ERROR("Load auth system fail!");
			return ret;
		}
	} else {
		GTP_INFO("hotknot load auth code.");
		ret = gt1x_load_patch(hotknot_auth_fw, 4096, 0, 1024 * 6);
		if (ret < 0) {
			GTP_ERROR("load auth system fail!");
			return ret;
		}
	}

	ret = gt1x_startup_patch();
	if (ret < 0) {
		GTP_ERROR("Startup auth system fail!");
		return ret;
	}
	ret = gt1x_i2c_read(GTP_REG_VERSION, buffer, 4);
	if (ret < 0) {
		GTP_ERROR("i2c read error!");
		return ERROR_IIC;
	}
	buffer[4] = 0;
	GTP_INFO("Current System version: %s", buffer);
	return 0;
}

static s32 hotknot_recovery_main_system(void)
{
	gt1x_irq_disable();
	gt1x_reset_guitar();
	gt1x_irq_enable();
#ifdef CONFIG_GTP_ESD_PROTECT
	gt1x_esd_switch(SWITCH_ON);
#endif
	hotknot_transfer_mode = 0;
	return 0;
}

#ifdef CONFIG_HOTKNOT_BLOCK_RW
DECLARE_WAIT_QUEUE_HEAD(bp_waiter);
static u8 got_hotknot_state;
static u8 got_hotknot_extra_state;
static u8 wait_hotknot_state;
static u8 force_wake_flag;
static u8 block_enable;
s32 hotknot_paired_flag;

static s32 hotknot_block_rw(u8 rqst_hotknot_state, s32 wait_hotknot_timeout)
{
	s32 ret = 0;

	wait_hotknot_state |= rqst_hotknot_state;
	GTP_DEBUG(
		"Goodix tool received wait polling state:0x%x,timeout:%d, all wait state:0x%x",
		rqst_hotknot_state, wait_hotknot_timeout, wait_hotknot_state);
	got_hotknot_state &= (~rqst_hotknot_state);

	set_current_state(TASK_INTERRUPTIBLE);
	if (wait_hotknot_timeout <= 0) {
		wait_event_interruptible(bp_waiter,
					 force_wake_flag ||
						 rqst_hotknot_state ==
							 (got_hotknot_state &
							  rqst_hotknot_state));
	} else {
		wait_event_interruptible_timeout(
			bp_waiter,
			force_wake_flag ||
				rqst_hotknot_state == (got_hotknot_state &
						       rqst_hotknot_state),
			wait_hotknot_timeout);
	}

	wait_hotknot_state &= (~rqst_hotknot_state);

	if (rqst_hotknot_state != (got_hotknot_state & rqst_hotknot_state)) {
		GTP_ERROR("Wait 0x%x block polling waiter failed.",
			  rqst_hotknot_state);
		ret = -1;
	}

	force_wake_flag = 0;
	return ret;
}

static void hotknot_wakeup_block(void)
{
	GTP_DEBUG("Manual wakeup all block polling waiter!");
	got_hotknot_state = 0;
	wait_hotknot_state = 0;
	force_wake_flag = 1;
	hotknot_paired_flag = 0;
	wake_up_interruptible(&bp_waiter);
}

s32 hotknot_event_handler(u8 *data)
{
	u8 hn_pxy_state = 0;
	u8 hn_pxy_state_bak = 0;
	static u8 hn_paired_cnt;
	u8 hn_state_buf[10] = {0};
	u8 finger = data[0];
	u8 id = 0;

	if (block_enable && !hotknot_paired_flag && (finger & 0x0F)) {
		id = data[1];
		hn_pxy_state = data[2] & 0x80;
		hn_pxy_state_bak = data[3] & 0x80;
		if ((id == 32) && (hn_pxy_state == 0x80) &&
		    (hn_pxy_state_bak == 0x80)) {
#ifdef HN_DBLCFM_PAIRED
			if (hn_paired_cnt++ < 2)
				return 0;
#endif
			GTP_DEBUG("HotKnot paired!");
			if (wait_hotknot_state & HN_DEVICE_PAIRED) {
				GTP_DEBUG(
					"INT wakeup HN_DEVICE_PAIRED block polling waiter");
				got_hotknot_state |= HN_DEVICE_PAIRED;
				wake_up_interruptible(&bp_waiter);
			}
			block_enable = 0;
			hotknot_paired_flag = 1;
			return 0;
		}
		got_hotknot_state &= (~HN_DEVICE_PAIRED);
		hn_paired_cnt = 0;
	}

	if (hotknot_paired_flag) {
		s32 ret = -1;

		ret = gt1x_i2c_read(GTP_REG_HN_STATE, hn_state_buf, 6);
		if (ret < 0) {
			GTP_ERROR("I2C transfer error. errno:%d\n ", ret);
			return 0;
		}

		got_hotknot_state = 0;

		GTP_DEBUG("wait_hotknot_state:%x", wait_hotknot_state);
		GTP_DEBUG("[0x8800~0x8803]=0x%x,0x%x,0x%x,0x%x",
			  hn_state_buf[0], hn_state_buf[1], hn_state_buf[2],
			  hn_state_buf[3]);

		if (wait_hotknot_state & HN_MASTER_SEND) {
			if ((hn_state_buf[0] == 0x03) ||
			    (hn_state_buf[0] == 0x04) ||
			    (hn_state_buf[0] == 0x07)) {
				GTP_DEBUG(
					"Wakeup HN_MASTER_SEND block polling waiter");
				got_hotknot_state |= HN_MASTER_SEND;
				got_hotknot_extra_state = hn_state_buf[0];
				wake_up_interruptible(&bp_waiter);
			}
		} else if (wait_hotknot_state & HN_SLAVE_RECEIVED) {
			if ((hn_state_buf[1] == 0x03) ||
			    (hn_state_buf[1] == 0x04) ||
			    (hn_state_buf[1] == 0x07)) {
				GTP_DEBUG(
					"Wakeup HN_SLAVE_RECEIVED block polling waiter:0x%x",
					hn_state_buf[1]);
				got_hotknot_state |= HN_SLAVE_RECEIVED;
				got_hotknot_extra_state = hn_state_buf[1];
				wake_up_interruptible(&bp_waiter);
			}
		} else if (wait_hotknot_state & HN_MASTER_DEPARTED) {
			if (hn_state_buf[0] == 0x07) {
				GTP_DEBUG(
					"Wakeup HN_MASTER_DEPARTED block polling waiter");
				got_hotknot_state |= HN_MASTER_DEPARTED;
				wake_up_interruptible(&bp_waiter);
			}
		} else if (wait_hotknot_state & HN_SLAVE_DEPARTED) {
			if (hn_state_buf[1] == 0x07) {
				GTP_DEBUG(
					"Wakeup HN_SLAVE_DEPARTED block polling waiter");
				got_hotknot_state |= HN_SLAVE_DEPARTED;
				wake_up_interruptible(&bp_waiter);
			}
		}
		return 0;
	}

	return -1;
}
#endif /*CONFIG_HOTKNOT_BLOCK_RW*/
#endif /*CONFIG_GTP_HOTKNOT*/

#define GOODIX_MAGIC_NUMBER 'G'
#define NEGLECT_SIZE_MASK (~(_IOC_SIZEMASK << _IOC_SIZESHIFT))

#define GESTURE_ENABLE_TOTALLY _IO(GOODIX_MAGIC_NUMBER, 1) /* 1*/
#define GESTURE_DISABLE_TOTALLY _IO(GOODIX_MAGIC_NUMBER, 2)
#define GESTURE_ENABLE_PARTLY _IO(GOODIX_MAGIC_NUMBER, 3)
#define GESTURE_DISABLE_PARTLY _IO(GOODIX_MAGIC_NUMBER, 4)
/*#define SET_ENABLED_GESTURE         */
/* (_IOW(GOODIX_MAGIC_NUMBER, 5, u8) & NEGLECT_SIZE_MASK)*/
#define GESTURE_DATA_OBTAIN                                                    \
	(_IOR(GOODIX_MAGIC_NUMBER, 6, u8) & NEGLECT_SIZE_MASK)
#define GESTURE_DATA_ERASE _IO(GOODIX_MAGIC_NUMBER, 7)

/*#define HOTKNOT_LOAD_SUBSYSTEM (_IOW(GOODIX_MAGIC_NUMBER, 6, u8) & */
/* NEGLECT_SIZE_MASK)*/
#define HOTKNOT_LOAD_HOTKNOT _IO(GOODIX_MAGIC_NUMBER, 20)
#define HOTKNOT_LOAD_AUTHENTICATION _IO(GOODIX_MAGIC_NUMBER, 21)
#define HOTKNOT_RECOVERY_MAIN _IO(GOODIX_MAGIC_NUMBER, 22)
/*#define HOTKNOT_BLOCK_RW      (_IOW(GOODIX_MAGIC_NUMBER, 6, u8) & */
/* NEGLECT_SIZE_MASK)*/
#define HOTKNOT_DEVICES_PAIRED _IO(GOODIX_MAGIC_NUMBER, 23)
#define HOTKNOT_MASTER_SEND _IO(GOODIX_MAGIC_NUMBER, 24)
#define HOTKNOT_SLAVE_RECEIVE _IO(GOODIX_MAGIC_NUMBER, 25)
/*#define HOTKNOT_DEVICES_COMMUNICATION*/
#define HOTKNOT_MASTER_DEPARTED _IO(GOODIX_MAGIC_NUMBER, 26)
#define HOTKNOT_SLAVE_DEPARTED _IO(GOODIX_MAGIC_NUMBER, 27)
#define HOTKNOT_WAKEUP_BLOCK _IO(GOODIX_MAGIC_NUMBER, 29)

#define IO_IIC_READ (_IOR(GOODIX_MAGIC_NUMBER, 100, u8) & NEGLECT_SIZE_MASK)
#define IO_IIC_WRITE (_IOW(GOODIX_MAGIC_NUMBER, 101, u8) & NEGLECT_SIZE_MASK)
#define IO_RESET_GUITAR _IO(GOODIX_MAGIC_NUMBER, 102)
#define IO_DISABLE_IRQ _IO(GOODIX_MAGIC_NUMBER, 103)
#define IO_ENABLE_IRQ _IO(GOODIX_MAGIC_NUMBER, 104)
#define IO_GET_VERSION (_IOR(GOODIX_MAGIC_NUMBER, 110, u8) & NEGLECT_SIZE_MASK)
#define IO_PRINT (_IOW(GOODIX_MAGIC_NUMBER, 111, u8) & NEGLECT_SIZE_MASK)
#define IO_VERSION "V1.3-20150420"
#define IO_NR28 (_IOR(GOODIX_MAGIC_NUMBER, 28, u8) & NEGLECT_SIZE_MASK)
#ifdef CONFIG_COMPAT
#define COMPAT_GESTURE_ENABLE_TOTALLY _IO(GOODIX_MAGIC_NUMBER, 1) /*1*/
#define COMPAT_GESTURE_DISABLE_TOTALLY _IO(GOODIX_MAGIC_NUMBER, 2)
#define COMPAT_GESTURE_ENABLE_PARTLY _IO(GOODIX_MAGIC_NUMBER, 3)
#define COMPAT_GESTURE_DISABLE_PARTLY _IO(GOODIX_MAGIC_NUMBER, 4)
/*#define SET_ENABLED_GESTURE  (_IOW(GOODIX_MAGIC_NUMBER, 5, u8) & */
/* NEGLECT_SIZE_MASK)*/
#define COMPAT_GESTURE_DATA_OBTAIN                                             \
	(_IOR(GOODIX_MAGIC_NUMBER, 6, u8) & NEGLECT_SIZE_MASK)
#define COMPAT_GESTURE_DATA_ERASE _IO(GOODIX_MAGIC_NUMBER, 7)

/*#define HOTKNOT_LOAD_SUBSYSTEM  (_IOW(GOODIX_MAGIC_NUMBER, 6, u8) & */
/* NEGLECT_SIZE_MASK)*/
#define COMPAT_HOTKNOT_LOAD_HOTKNOT _IO(GOODIX_MAGIC_NUMBER, 20)
#define COMPAT_HOTKNOT_LOAD_AUTHENTICATION _IO(GOODIX_MAGIC_NUMBER, 21)
#define COMPAT_HOTKNOT_RECOVERY_MAIN _IO(GOODIX_MAGIC_NUMBER, 22)
/*#define HOTKNOT_BLOCK_RW   (_IOW(GOODIX_MAGIC_NUMBER, 6, u8) & */
/* NEGLECT_SIZE_MASK)*/
#define COMPAT_HOTKNOT_DEVICES_PAIRED _IO(GOODIX_MAGIC_NUMBER, 23)
#define COMPAT_HOTKNOT_MASTER_SEND _IO(GOODIX_MAGIC_NUMBER, 24)
#define COMPAT_HOTKNOT_SLAVE_RECEIVE _IO(GOODIX_MAGIC_NUMBER, 25)
/*#define HOTKNOT_DEVICES_COMMUNICATION*/
#define COMPAT_HOTKNOT_MASTER_DEPARTED _IO(GOODIX_MAGIC_NUMBER, 26)
#define COMPAT_HOTKNOT_SLAVE_DEPARTED _IO(GOODIX_MAGIC_NUMBER, 27)
#define COMPAT_HOTKNOT_WAKEUP_BLOCK _IO(GOODIX_MAGIC_NUMBER, 29)

#define COMPAT_IO_IIC_READ                                                     \
	(_IOR(GOODIX_MAGIC_NUMBER, 100, u8) & NEGLECT_SIZE_MASK)
#define COMPAT_IO_IIC_WRITE                                                    \
	(_IOW(GOODIX_MAGIC_NUMBER, 101, u8) & NEGLECT_SIZE_MASK)
#define COMPAT_IO_RESET_GUITAR _IO(GOODIX_MAGIC_NUMBER, 102)
#define COMPAT_IO_DISABLE_IRQ _IO(GOODIX_MAGIC_NUMBER, 103)
#define COMPAT_IO_ENABLE_IRQ _IO(GOODIX_MAGIC_NUMBER, 104)
#define COMPAT_IO_GET_VERSION                                                  \
	(_IOR(GOODIX_MAGIC_NUMBER, 110, u8) & NEGLECT_SIZE_MASK)
#define COMPAT_IO_PRINT (_IOW(GOODIX_MAGIC_NUMBER, 111, u8) & NEGLECT_SIZE_MASK)
#endif
#define CMD_HEAD_LENGTH 20
static s32 io_iic_read(u8 *data, int buf_size, void __user *arg)
{
	s32 err = ERROR;
	s32 data_length = 0;
	u16 addr = 0;

	err = copy_from_user(data, arg, CMD_HEAD_LENGTH);
	if (err) {
		GTP_ERROR("Can't access the memory.");
		return ERROR_MEM;
	}

	addr = data[0] << 8 | data[1];
	data_length = data[2] << 8 | data[3];

	err = gt1x_i2c_read(addr, &data[CMD_HEAD_LENGTH], data_length);
	if (!err) {
		err = copy_to_user(&((u8 __user *)arg)[CMD_HEAD_LENGTH],
				   &data[CMD_HEAD_LENGTH], data_length);
		if (err) {
			GTP_ERROR(
				"ERROR when copy to user.[addr: %04x], [read length:%d]",
#line 734
				addr, data_length);
			return ERROR_MEM;
		}
		err = CMD_HEAD_LENGTH + data_length;
	}

	return err;
}

static s32 io_iic_write(u8 *data)
{
	s32 err = ERROR;
	s32 data_length = 0;
	u16 addr = 0;

	addr = data[0] << 8 | data[1];
	data_length = data[2] << 8 | data[3];

	err = gt1x_i2c_write(addr, &data[CMD_HEAD_LENGTH], data_length);
	if (!err)
		err = CMD_HEAD_LENGTH + data_length;

	return err;
}

/*@return, 0:operate successfully
 *         > 0: the length of memory size ioctl has accessed,
 *         error otherwise.
 */
static long gt1x_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	u32 value = 0;
	s32 ret = 0; /*the initial value must be 0*/
	u8 *data = NULL;
	int cnt = 30;
	s32 data_length = 0;

	/* Blocking when firmwaer updating */
	while (cnt-- && update_info.status)
		ssleep(1);

	if (_IOC_DIR(cmd)) {
		s32 err = -1;

		data_length = _IOC_SIZE(cmd);
		data = kzalloc(data_length, GFP_KERNEL);
		memset(data, 0, data_length);

		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			err = copy_from_user(data, (void __user *)arg,
					     data_length);
			if (err) {
				GTP_ERROR("Can't access the memory.");
				kfree(data);
				return -1;
			}
		}
	} else {
		value = (u32)arg;
	}

	switch (cmd & NEGLECT_SIZE_MASK) {
	case IO_GET_VERSION:
		if ((u8 __user *)arg) {
			ret = copy_to_user(((u8 __user *)arg), IO_VERSION,
					   sizeof(IO_VERSION));
			if (!ret)
				ret = sizeof(IO_VERSION);
#line 811
			GTP_INFO("%s", IO_VERSION);
		}
		break;
	case IO_NR28:
		ret = copy_to_user(((u8 __user *)arg), "GOODIX,GT1X",
				   sizeof("GOODIX,GT1X"));
		if (!ret)
			ret = sizeof("GOODIX,GT1X");
		break;
	case IO_IIC_READ:
		ret = io_iic_read(data, data_length, (void __user *)arg);
		break;

	case IO_IIC_WRITE:
		ret = io_iic_write(data);
		break;

	case IO_RESET_GUITAR:
		gt1x_irq_disable();
		gt1x_reset_guitar();
		gt1x_irq_enable();
		break;

	case IO_DISABLE_IRQ:
		gt1x_irq_disable();
		break;

	case IO_ENABLE_IRQ:
		gt1x_irq_enable();
		break;

	/*print a string to syc log messages between application and kernel.*/
	case IO_PRINT:
		if (data)
			GTP_INFO("%s", (char *)data);
		break;

#ifdef CONFIG_GTP_GESTURE_WAKEUP
	case GESTURE_ENABLE_TOTALLY:
		GTP_DEBUG("Gesture switch ON.");
		gesture_enabled = 1;
		break;

	case GESTURE_DISABLE_TOTALLY:
		GTP_DEBUG("Gesture switch OFF.");
		gesture_enabled = 0;
		break;

	case GESTURE_ENABLE_PARTLY:
		SETBIT(gestures_flag, (u8)value);
		GTP_DEBUG("Gesture flag: 0x%02X enabled.", value);
		break;

	case GESTURE_DISABLE_PARTLY:
		CLEARBIT(gestures_flag, (u8)value);
		GTP_DEBUG("Gesture flag: 0x%02X disabled.", value);
		break;

	case GESTURE_DATA_OBTAIN:
		GTP_DEBUG("Obtain gesture data.");

		mutex_lock(&gesture_data_mutex);
		ret = copy_to_user(((u8 __user *)arg), &gesture_data.data,
				   4 + gesture_data.data[1] * 4 +
					   gesture_data.data[3]);
		if (ret) {
#line 874
			GTP_ERROR("ERROR when copy gesture data to user.");
			ret = ERROR_MEM;
		} else {
			ret = 4 + gesture_data.data[1] * 4 +
			      gesture_data.data[3];
		}
		mutex_unlock(&gesture_data_mutex);
		break;

	case GESTURE_DATA_ERASE:
#line 883
		GTP_DEBUG("ERASE_GESTURE_DATA");
		gesture_clear_wakeup_data();
		break;
#endif /*CONFIG_GTP_GESTURE_WAKEUP*/

#ifdef CONFIG_GTP_HOTKNOT
	case HOTKNOT_LOAD_HOTKNOT:
		ret = hotknot_load_hotknot_subsystem();
		break;

	case HOTKNOT_LOAD_AUTHENTICATION:
		ret = hotknot_load_authentication_subsystem();
		break;

	case HOTKNOT_RECOVERY_MAIN:
		ret = hotknot_recovery_main_system();
		break;
#ifdef CONFIG_HOTKNOT_BLOCK_RW
	case HOTKNOT_DEVICES_PAIRED:
		hotknot_paired_flag = 0;
		force_wake_flag = 0;
		block_enable = 1;
		ret = hotknot_block_rw(HN_DEVICE_PAIRED, (s32)value);
		break;

	case HOTKNOT_MASTER_SEND:
		ret = hotknot_block_rw(HN_MASTER_SEND, (s32)value);
		if (!ret)
			ret = got_hotknot_extra_state;
		break;

	case HOTKNOT_SLAVE_RECEIVE:
		ret = hotknot_block_rw(HN_SLAVE_RECEIVED, (s32)value);
		if (!ret)
			ret = got_hotknot_extra_state;
		break;

	case HOTKNOT_MASTER_DEPARTED:
		ret = hotknot_block_rw(HN_MASTER_DEPARTED, (s32)value);
		break;

	case HOTKNOT_SLAVE_DEPARTED:
		ret = hotknot_block_rw(HN_SLAVE_DEPARTED, (s32)value);
		break;

	case HOTKNOT_WAKEUP_BLOCK:
		hotknot_wakeup_block();
		break;
#endif /*CONFIG_HOTKNOT_BLOCK_RW*/
#endif /*CONFIG_GTP_HOTKNOT*/

	default:
#line 944
		GTP_INFO("Unknown cmd.");
		ret = -1;
		break;
	}

	if (data != NULL)
		kfree(data);
	return ret;
}
#ifdef CONFIG_GTP_HOTKNOT
#ifdef CONFIG_COMPAT
/*
 * In the factory build there is NO switch: 56 bytes in all, 14 instructions, and the only
 * constants are 40, 72 and -25. The body is a plain forward.
 *   "f9401408 ldr"@0xffffff8008a737b0   ldr x8, [x0,#40]  -> file->f_op
 *   "f9402508 ldr"@0xffffff8008a737b8   ldr x8, [x8,#72]  -> ->unlocked_ioctl
 *   "b27bf7e0 orr"@0xffffff8008a737d8   x0 = -25 = -ENOTTY, on both cbz
 *   "92407c42 and"@0xffffff8008a737c8   x2 &= 0xffffffff  -> compat_ptr(arg)
 *   "d63f0100 blr"@0xffffff8008a737cc   an indirect call, the only one in the body
 * The `and` comes AFTER the two cbz and before the blr: it is the only use of arg32.
 */
static long gt1x_compat_ioctl(struct file *file, unsigned int cmd,
			      unsigned long arg)
{
	long ret;
	void __user *arg32 = compat_ptr(arg);

	if (!file->f_op || !file->f_op->unlocked_ioctl)
		return -ENOTTY;

	ret = file->f_op->unlocked_ioctl(file, cmd, (unsigned long)arg32);

	return ret;
}
#endif
#endif
static const struct file_operations gt1x_fops = {
	.owner = THIS_MODULE,
#ifdef CONFIG_GTP_GESTURE_WAKEUP
	.read = gt1x_gesture_data_read,
	.write = gt1x_gesture_data_write,
#endif
	.unlocked_ioctl = gt1x_ioctl,
};

#ifdef CONFIG_GTP_HOTKNOT
static const struct file_operations hotknot_fops = {
	.unlocked_ioctl = gt1x_ioctl,
	.open = hotknot_open,
	.release = hotknot_release,
#ifdef CONFIG_COMPAT
	.compat_ioctl = gt1x_compat_ioctl,
#endif
};

static struct miscdevice hotknot_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = HOTKNOT_NODE,
	.fops = &hotknot_fops,
};
#endif

s32 gt1x_init_node(void)
{
#ifdef CONFIG_GTP_GESTURE_WAKEUP
	struct proc_dir_entry *proc_entry = NULL;

	mutex_init(&gesture_data_mutex);
	memset(gestures_flag, 0xff, sizeof(gestures_flag));
	memset((u8 *)&gesture_data, 0, sizeof(st_gesture_data));

	proc_entry = proc_create(GESTURE_NODE, 0666, NULL, &gt1x_fops);
	if (proc_entry == NULL) {
#line 1007
		GTP_ERROR("CAN't create proc entry /proc/%s.", GESTURE_NODE);
		return -1;
	}
	GTP_INFO("Created proc entry /proc/%s.", GESTURE_NODE);
#endif

#ifdef CONFIG_GTP_HOTKNOT
	if (misc_register(&hotknot_misc_device)) {
#line 1016
		GTP_ERROR("CAN't create misc device in /dev/hotknot.");
		return -1;
	}
	GTP_INFO("Created misc device in /dev/hotknot.");
#endif
	return 0;
}

void gt1x_deinit_node(void)
{
#ifdef CONFIG_GTP_GESTURE_WAKEUP
	remove_proc_entry(GESTURE_NODE, NULL);
#endif

#ifdef CONFIG_GTP_HOTKNOT
	misc_deregister(&hotknot_misc_device);
#endif
}
