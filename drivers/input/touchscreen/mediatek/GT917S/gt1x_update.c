/* drivers/input/touchscreen/gt1x_update.c
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
 */
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#ifdef CONFIG_GTP_REQUEST_FW_UPDATE
#include <linux/firmware.h>
#endif
#include "gt1x_config.h"
#include "include/gt1x_tpd_common.h"
#ifdef CONFIG_GTP_HEADER_FW_UPDATE
#include "gt1x_firmware.h"
#endif

#ifdef CONFIG_GTP_REQUEST_FW_UPDATE
#define GT1151_DEFAULT_FW "gt1151_default_"
#endif
#undef CONFIG_GTP_FOPEN_FW_UPDATE

#define UPDATE_FILE_PATH_1 "/data/_goodix_update_.bin"
#define UPDATE_FILE_PATH_2 "/sdcard/_goodix_update_.bin"

#define CONFIG_FILE_PATH_1 "/data/_gt1x_config_.cfg"
#define CONFIG_FILE_PATH_2 "/sdcard/_gt1x_config_.cfg"

#define FOUND_FW_PATH_1 0x01
#define FOUND_FW_PATH_2 0x02
#define FOUND_CFG_PATH_1 0x04
#define FOUND_CFG_PATH_2 0x08

#define PACK_SIZE 256

/*hardware register define*/
#define _bRW_MISCTL__SRAM_BANK 0x4048
#define _bRW_MISCTL__MEM_CD_EN 0x4049
#define _bRW_MISCTL__CACHE_EN 0x404B
#define _bRW_MISCTL__TMR0_EN 0x40B0
#define _rRW_MISCTL__SWRST_B0_ 0x4180
#define _bWO_MISCTL__CPU_SWRST_PULSE 0x4184
#define _rRW_MISCTL__BOOTCTL_B0_ 0x4190
#define _rRW_MISCTL__BOOT_OPT_B0_ 0x4218
#define _rRW_MISCTL__BOOT_CTL_ 0x5094
#define _bRW_MISCTL__DSP_MCU_PWR_ 0x4010
#define _bRW_MISCTL__PATCH_AREA_EN_ 0x404D

/*
 * 1.  firmware structure
 *    header: 128b
 *
 *   offset           size          content
 *   0                 4              firmware length
 *   4                 2              checksum
 *   6                 6              target MASK name
 *   12               3              target MASK version
 *   15               6              TP subsystem PID
 *   21               3              TP subsystem version
 *   24               1              subsystem count
 *   25               1              chip type                             0x91:
 * GT1X,   0x92: GT2X
 *   26               6              reserved
 *   32               8              subsystem info[0]
 *   32               8              subsystem info[1]
 *   .....
 *   120             8              subsystem info[11]
 *
 *   body: followed header
 *
 *   128             N0              subsystem[0]
 *   128+N0       N1              subsystem[1]
 *   ....
 *
 * 2. subsystem info structure
 *   offset           size          content
 *   0                 1              subsystem type
 *   1                 2              subsystem length
 *   3                 2              stored address in flash           addr =
 * value * 256
 *   5                 3              reserved
 *
 */

#define FW_HEAD_SIZE 128
#define FW_HEAD_SUBSYSTEM_INFO_SIZE 8
#define FW_HEAD_OFFSET_SUBSYSTEM_INFO_BASE 32

#define FW_SECTION_TYPE_SS51_ISP 0x01
#define FW_SECTION_TYPE_SS51_PATCH 0x02
#define FW_SECTION_TYPE_SS51_PATCH_OVERLAY 0x03
#define FW_SECTION_TYPE_DSP 0x04
#define FW_SECTION_TYPE_HOTKNOT 0x05
#define FW_SECTION_TYPE_GESTURE 0x06
#define FW_SECTION_TYPE_GESTURE_OVERLAY 0x07
#define FW_SECTION_TYPE_FLASHLESS_FAST_POWER 0x08

#define UPDATE_TYPE_HEADER 0
#define UPDATE_TYPE_FILE 1

#define UPDATE_STATUS_IDLE 0
#define UPDATE_STATUS_RUNNING 1

struct fw_subsystem_info {
	int type;
	int length;
	u32 address;
	int offset;
};

#pragma pack(1)
struct fw_info {
	u32 length;
	u16 checksum;
	u8 target_mask[6];
	u8 target_mask_version[3];
	u8 pid[6];
	u8 version[3];
	u8 subsystem_count;
	u8 chip_type;
	u8 reserved[6];
	struct fw_subsystem_info subsystem[12];
};
#pragma pack()

struct fw_update_info update_info = {
	.status = UPDATE_STATUS_IDLE, .progress = 0, .max_progress = 9};

/*
 * HEADER DELTA: in the factory build `struct fw_update_info` has one extra
 * `int` at offset 16, and from `firmware` onwards every field shifts by eight
 * bytes.  The twelve offsets are MEASURED one by one -- citations at the end,
 * NOTE (1). The line to add to the header is `int c16;`, and the merge batch
 * does it.
 */
struct fw_update_info_fab {
	int update_type;
	int status;
	int progress;
	int max_progress;
	int c16;
	struct fw_info *firmware;
	u32 fw_length;
	char *fw_name;
	u8 *buffer;
	mm_segment_t old_fs;
	struct file *fw_file;
	u8 *fw_data;
};
#define upd (*(struct fw_update_info_fab *)&update_info)


/* Forward declarations: the order here is that of the __LINE__s, NOTE (2). */
static u8 gt1x_search_update_files(void);
int gt1x_update_prepare(char *filename);
int gt1x_check_firmware(void);
int gt1x_update_judge(void);
u8 *gt1x_get_fw_data(u32 offset, int length);
int gt1x_run_ss51_isp(u8 *ss51_isp, int length);
int gt1x_burn_subsystem(struct fw_subsystem_info *subsystem);
void gt1x_update_cleanup(void);
int gt1x_recall_check(u8 *chk_src, u16 start_addr, u16 chk_length);
int gt1x_read_flash(u32 addr, int length);
u16 gt1x_calc_checksum(u8 *fw, u32 length);

/**
 * @return: return 0 if success, otherwise return a negative number
 *          which contains the error code.
 */
s32 gt1x_check_fs_mounted(char *path_name)
{
	struct path root_path;
	struct path path;
	s32 err;
	err = kern_path("/", LOOKUP_FOLLOW, &root_path);
	if (err)
		return ERROR_PATH;
	err = kern_path(path_name, LOOKUP_FOLLOW, &path);
	if (err) {
		err = ERROR_PATH;
		goto check_fs_fail;
	}
	if (path.mnt->mnt_sb == root_path.mnt->mnt_sb)
		err = ERROR_PATH;
	else
		err = 0;
	path_put(&path);
check_fs_fail:
	path_put(&root_path);
	return err;
}

int gt1x_i2c_write_with_readback(u16 addr, u8 *buffer, int length)
{
	u8 buf[100];
	int ret = gt1x_i2c_write(addr, buffer, length);
	if (ret)
		return ret;
	ret = gt1x_i2c_read(addr, buf, length);
	if (ret)
		return ret;
	if (memcmp(buf, buffer, length))
		return ERROR_CHECK;
	return 0;
}

#define getU32(a) ((u32)getUint((u8 *)(a), 4))
#define getU16(a) ((u16)getUint((u8 *)(a), 2))
u32 getUint(u8 *buffer, int len)
{
	u32 num = 0;
	int i;
	for (i = 0; i < len; i++) {
		num <<= 8;
		num += buffer[i];
	}
	return num;
}
int gt1x_auto_update_proc(void *data)
{
	u8 ret;
	u8 config[GTP_CONFIG_MAX_LENGTH] = {0};
	mm_segment_t old_fs;
	GTP_INFO("Start auto update thread...");
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = gt1x_search_update_files();
	set_fs(old_fs);
	if (ret & (FOUND_FW_PATH_1 | FOUND_FW_PATH_2))
		gt1x_update_firmware(ret & FOUND_FW_PATH_1 ?
					     UPDATE_FILE_PATH_1 :
					     UPDATE_FILE_PATH_2);
	if (ret & (FOUND_CFG_PATH_1 | FOUND_CFG_PATH_2)) {
		int cfg_len;
		cfg_len = gt1x_parse_config(ret & FOUND_CFG_PATH_1 ?
						    CONFIG_FILE_PATH_1 :
						    CONFIG_FILE_PATH_2,
					    config);
		if (cfg_len > 0) {
			if (gt1x_i2c_write(GTP_REG_CONFIG_DATA, config,
					   GTP_CONFIG_MAX_LENGTH))


				GTP_ERROR("Update config failed!");
			else
				GTP_INFO("Update config successfully!");
		}
	}
	return 0;
}

/*
 * gt1x_search_update_files -- in the factory build it has no symbol of its own:
 * it is
 *
 * inlined into gt1x_auto_update_proc, and what says so is the __func__ of its
 * three printks, "gt1x_search_update_files"@0xffffff800924da9d, different from
 * the container's, "gt1x_auto_update_proc"@0xffffff800924c6a0.
 * It is class B5 seen from the good side.  See NOTE (3).
 */
static u8 gt1x_search_update_files(void)
{
	int i = 0;
	struct file *fp = NULL;
	u8 found_flag = 0;
	GTP_INFO("Search firmware file...");
	for (i = 40; i > 0; i--) {
		msleep(500);
		if (gt1x_check_fs_mounted("/data")) {


			GTP_DEBUG("filesystem is not ready");
			continue;
		}
		fp = filp_open(UPDATE_FILE_PATH_1, O_RDONLY, 0);
		if (!IS_ERR(fp)) {
			found_flag |= FOUND_FW_PATH_1;
		} else {
			fp = filp_open(UPDATE_FILE_PATH_2, O_RDONLY, 0);
			if (!IS_ERR(fp))
				found_flag |= FOUND_FW_PATH_2;
		}
		if (!IS_ERR(fp))
			filp_close(fp, NULL);
		fp = filp_open(CONFIG_FILE_PATH_1, O_RDONLY, 0);
		if (IS_ERR(fp)) {
			fp = filp_open(CONFIG_FILE_PATH_2, O_RDONLY, 0);
			if (!IS_ERR(fp))
				found_flag |= FOUND_CFG_PATH_2;
		} else {
			found_flag |= FOUND_CFG_PATH_1;
		}
		if (!IS_ERR(fp))
			filp_close(fp, NULL);
		if (found_flag)
			break;










		GTP_INFO("Not found firmware or config file, retry.");
	}
	return found_flag;
}

/*
 * no gt1x_esd_switch: the factory calls only printk and
 * gt1x_irq_disable ("97fff88c bl"@0xffffff8008a78c14).
 */
void gt1x_enter_update_mode(void)
{
	GTP_DEBUG("Enter FW update mode.");
	gt1x_irq_disable();
}

/*
 * the factory does NOT re-check the flash: after the gt1x_burn_subsystem loop
 *
 *
 * it goes straight to the exit ("17ffff55 b"@0xffffff8008a78bcc towards
 * 0xffffff8008a78920), with no second gt1x_run_ss51_isp and no
 * gt1x_check_subsystem_in_flash loop.  See NOTE (4).
 */
int gt1x_update_firmware(char *filename)
{
	int i = 0;
	int ret = 0;
	u8 *p;
	if (upd.status != UPDATE_STATUS_IDLE) {
		GTP_ERROR("Update process is running!");
		return ERROR;
	}
	upd.status = UPDATE_STATUS_RUNNING;
	upd.progress = 0;
	gt1x_enter_update_mode();
	ret = gt1x_update_prepare(filename);
	if (ret) {
		upd.status = UPDATE_STATUS_IDLE;
		return ret;
	}
	ret = gt1x_check_firmware();
	if (ret) {
		upd.status = 2;
		goto gt1x_update_exit;
	}
	upd.max_progress = upd.firmware->subsystem_count + 3;
	upd.progress++;
	ret = gt1x_update_judge();
	if (ret) {
		upd.status = 2;
		goto gt1x_update_exit;
	}
	upd.progress++;
	p = gt1x_get_fw_data(upd.firmware->subsystem[0].offset,
			     upd.firmware->subsystem[0].length);
	if (p == NULL) {










		GTP_ERROR("get isp fail");
		upd.status = 2;
		ret = ERROR_FW;
		goto gt1x_update_exit;
	}
	upd.progress++;
	ret = gt1x_run_ss51_isp(p, upd.firmware->subsystem[0].length);
	if (ret) {

		GTP_ERROR("run isp fail");
		goto gt1x_update_exit;
	}
	upd.progress++;
	msleep(800);
	for (i = 1; i < upd.firmware->subsystem_count; i++) {

		GTP_INFO("subsystem: %d", upd.firmware->subsystem[i].type);
		GTP_INFO("Length: %d", upd.firmware->subsystem[i].length);
		GTP_INFO("Address: %d", upd.firmware->subsystem[i].address);
		ret = gt1x_burn_subsystem(&(upd.firmware->subsystem[i]));
		if (ret) {

			GTP_ERROR("burn subsystem fail!");
			goto gt1x_update_exit;
		}
		upd.progress++;
	}
gt1x_update_exit:
	gt1x_update_cleanup();
	gt1x_leave_update_mode();
	gt1x_read_version(NULL);
	if (ret) {
		upd.progress = 2 * upd.max_progress;


































		GTP_ERROR("Update firmware failed!");
	} else {
		if (gt1x_init_failed) {
			gt1x_read_version(&gt1x_version);
			gt1x_init_panel();
		}






		GTP_INFO("Update firmware succeefully!");
	}
	return ret;
}

/*
 * an update from a FILE, not from the header: see NOTE (5) for the proofs and
 *
 *
 * for the two factory defects reproduced in here.
 */
int gt1x_update_prepare(char *filename)
{
	int ret = 0;
	int retry = 5;
	if (filename == NULL) {
		GTP_ERROR("No Fw in .h file!");
		return ERROR_FW;
	}

	GTP_INFO("Firmware: %s", filename);
	upd.old_fs = get_fs();
	set_fs(KERNEL_DS);
	upd.fw_name = filename;
	upd.update_type = UPDATE_TYPE_FILE;
	upd.fw_file = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(upd.fw_file)) {
		GTP_ERROR("Open update file(%s) error!", upd.fw_name);
		set_fs(upd.old_fs);
		return ERROR_FILE;
	}
	upd.fw_file->f_op->llseek(upd.fw_file, 0, SEEK_SET);
	upd.fw_length =
		upd.fw_file->f_op->llseek(upd.fw_file, 0, SEEK_END);
	while (retry > 0) {
		retry--;
		upd.firmware = kzalloc(sizeof(struct fw_info), GFP_KERNEL);
		if (upd.firmware == NULL) {
			GTP_INFO("Alloc %zu bytes memory fail.",
				 sizeof(struct fw_info));
			continue;
		} else {
			GTP_INFO("Alloc %zu bytes memory success.", sizeof(struct fw_info));
			break;
		}
	}
	if (retry <= 0) {
		ret = ERROR_RETRY;
		goto gt1x_update_pre_fail1;
	}
	retry = 5;
	while (retry > 0) {
		upd.buffer = kzalloc(1024 * 4, GFP_KERNEL);
		if (upd.buffer == NULL) {

			GTP_ERROR("Alloc %d bytes memory fail.", 1024 * 4);
			continue;
		} else {
			GTP_INFO("Alloc %d bytes memory success.", 1024 * 4);
			break;
		}
	}
	return 0;
gt1x_update_pre_fail1:
	filp_close(upd.fw_file, NULL);
	return ret;
}

/*
 * gt1x_check_firmware() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_update.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int gt1x_check_firmware(void)
{
	u16 checksum;
	u16 checksum_in_header;
	u8 *p;
	struct fw_info *firmware;
	int i;
	int offset;
	if (upd.fw_length < FW_HEAD_SIZE) {
		GTP_ERROR("Bad firmware!(file length: %d)",
			  upd.fw_length);
		return ERROR_CHECK;
	}
	p = gt1x_get_fw_data(0, 6);
	if (p == NULL)
		return ERROR_FW;
	if (getU32(p) + 6 != upd.fw_length) {
		GTP_ERROR("Bad firmware!(file length: %d, header define: %d)",

			  upd.fw_length, getU32(p));
		return ERROR_CHECK;
	}
	checksum_in_header = getU16(&p[4]);
	checksum = 0;
	for (i = 6; i < upd.fw_length; i++) {
		p = gt1x_get_fw_data(i, 1);
		if (p == NULL)
			return ERROR_FW;
		checksum += p[0];
	}
	if (checksum != checksum_in_header) {
		GTP_ERROR(
			"Bad firmware!(checksum: 0x%04X, header define: 0x%04X)",

			checksum, checksum_in_header);
		return ERROR_CHECK;
	}
	p = gt1x_get_fw_data(0, FW_HEAD_SIZE);
	if (p == NULL)
		return ERROR_FW;
	memcpy((u8 *)upd.firmware, p, FW_HEAD_SIZE - 8 * 12);
	upd.firmware->pid[5] = 0;
	p = &p[FW_HEAD_OFFSET_SUBSYSTEM_INFO_BASE];
	firmware = upd.firmware;
	offset = FW_HEAD_SIZE;
	for (i = 0; i < firmware->subsystem_count; i++) {
		firmware->subsystem[i].type =
			p[i * FW_HEAD_SUBSYSTEM_INFO_SIZE];
		firmware->subsystem[i].length =
			getU16(&p[i * FW_HEAD_SUBSYSTEM_INFO_SIZE + 1]);
		firmware->subsystem[i].address =
			getU16(&p[i * FW_HEAD_SUBSYSTEM_INFO_SIZE + 3]) * 256;
		firmware->subsystem[i].offset = offset;
		offset += firmware->subsystem[i].length;
	}
	GTP_INFO("Update type: %s",
		 upd.update_type == UPDATE_TYPE_HEADER ? "Header"
							       : "File");
	GTP_INFO("Firmware length: %d", upd.fw_length);
	GTP_INFO("Firmware product: GT%s", upd.firmware->pid);
	GTP_INFO("Firmware patch: %02X%02X%02X", upd.firmware->version[0], upd.firmware->version[1], upd.firmware->version[2]);
	GTP_INFO("Firmware chip: 0x%02X", upd.firmware->chip_type);
	GTP_INFO("Subsystem count: %d", upd.firmware->subsystem_count);
	for (i = 0; i < upd.firmware->subsystem_count; i++) {
		GTP_DEBUG("------------------------------------------");
		GTP_DEBUG("Subsystem: %d", i);
		GTP_DEBUG("Type: %d", upd.firmware->subsystem[i].type);
		GTP_DEBUG("Length: %d", upd.firmware->subsystem[i].length);
		GTP_DEBUG("Address: 0x%08X", upd.firmware->subsystem[i].address);
		GTP_DEBUG("Offset: %d", upd.firmware->subsystem[i].offset);
	}
	return 0;
}

/* it reads from the file when update_type is 1.  See NOTE (6). */
u8 *gt1x_get_fw_data(u32 offset, int length)
{
	int ret;
	if (upd.update_type == UPDATE_TYPE_FILE) {
		upd.fw_file->f_op->llseek(upd.fw_file, offset, SEEK_SET);
		ret = upd.fw_file->f_op->read(upd.fw_file, (char *)upd.buffer,
					      length, &upd.fw_file->f_pos);
		if (ret < 0) {
			GTP_ERROR("Read data error!");
			return NULL;
		}
		return upd.buffer;
	}
	return &upd.fw_data[offset];
}







int gt1x_update_judge(void)
{
	int ret;
	u8 reg_val[2] = {0};
	int retry = 2;
	struct gt1x_version_info ver_info;
	struct gt1x_version_info fw_ver_info;
	fw_ver_info.mask_id =
		(upd.firmware->target_mask_version[0] << 16) |
		(upd.firmware->target_mask_version[1] << 8) |
		(upd.firmware->target_mask_version[2]);
	fw_ver_info.patch_id = (upd.firmware->version[0] << 16) |
			       (upd.firmware->version[1] << 8) |
			       (upd.firmware->version[2]);
	memcpy(fw_ver_info.product_id, upd.firmware->pid, 4);
	fw_ver_info.product_id[4] = 0;
	do {
		ret = gt1x_i2c_read_dbl_check(0x41E4, &reg_val[0], 1);
		if (ret < 0) {
			gt1x_reset_guitar();
			continue;
		}
		if (ret != 0)
			continue;
		ret = gt1x_i2c_read_dbl_check(0x5095, &reg_val[1], 1);
		if (ret < 0) {
			gt1x_reset_guitar();
			continue;
		}
		if (ret != 0)
			continue;
		break;
	} while (--retry);
	if (ret != 0) {
		GTP_INFO("Update abort because of i2c error.");
		return ERROR_CHECK;
	}
	if (reg_val[0] != 0xBE || reg_val[1] == 0xAA) {
		GTP_INFO(
			"Check fw status reg not pass,reg[0x814E]=0x%2X,reg[0x5095]=0x%2X!", reg_val[0], reg_val[1]);
		return 0;
	}
	ret = gt1x_read_version(&ver_info);
	if (ret < 0) {

		GTP_INFO("Get IC's version info failed, force update!");
		return 0;
	}
	if (memcmp(fw_ver_info.product_id, ver_info.product_id, 4)) {

		GTP_INFO("Product id is not match!");
		return ERROR_CHECK;
	}
	if ((fw_ver_info.mask_id & 0xFFFFFF00) != (ver_info.mask_id & 0xFFFFFF00)) {
		GTP_INFO("Mask id is not match!");
		return ERROR_CHECK;
	}
	if ((fw_ver_info.patch_id & 0xFF0000) != (ver_info.patch_id & 0xFF0000)) {
		GTP_INFO("CID is not equal, need update!");
		return 0;
	}
	if (upd.c16) {

		GTP_DEBUG("Debug mode, force update fw.");
		return 0;
	}
	if ((fw_ver_info.patch_id & 0xFFFF) <= (ver_info.patch_id & 0xFFFF)) {

		GTP_INFO("The version of the fw is not high than the IC's!");
		return ERROR_CHECK;
	}
	return 0;
}

/* the retry limit is 30, not 2000.  See NOTE (7). */
int __gt1x_hold_ss51_dsp_20(void)
{
	int ret = -1;
	int retry = 0;
	u8 buf[1];
	int hold_times = 0;
	while (retry++ < 30) {
		buf[0] = 0x0C;
		ret = gt1x_i2c_write(_rRW_MISCTL__SWRST_B0_, buf, 1);
		if (ret) {
			GTP_ERROR("Hold ss51 & dsp I2C error,retry:%d", retry);
			continue;
		}
		buf[0] = 0x00;
		ret = gt1x_i2c_read(_rRW_MISCTL__SWRST_B0_, buf, 1);
		if (ret) {

			GTP_ERROR("Hold ss51 & dsp I2C error,retry:%d", retry);
			continue;
		}
		if (buf[0] == 0x0C) {
			if (hold_times++ < 20)
				continue;
			else
				break;
		}
		GTP_ERROR("Hold ss51 & dsp confirm 0x4180 failed,value:%d",
			  buf[0]);
	}
	if (retry >= 30) {
		GTP_ERROR("Hold ss51&dsp failed!");
		return ERROR_RETRY;
	}

	GTP_INFO("Hold ss51&dsp successfully.");
	return 0;
}

/*
 * it starts with a read loop of 0x4220 that ALPS does not have, and has
 *
 *
 *
 * no msleep(20) at all.  See NOTE (8).
 */
int gt1x_hold_ss51_dsp(void)
{
	int ret = ERROR;
	u8 buffer[2];
	int retry = 5;
	do {
		gt1x_select_addr();
		ret = gt1x_i2c_read(0x4220, buffer, 1);
	} while (retry-- && ret < 0);
	if (ret < 0)
		return ERROR;
	ret = __gt1x_hold_ss51_dsp_20();
	if (ret)
		return ret;
	buffer[0] = 0x00;
	ret = gt1x_i2c_write_with_readback(_bRW_MISCTL__DSP_MCU_PWR_, buffer,
					   1);
	if (ret) {
		GTP_ERROR("enabel dsp & mcu power fail!");
		return ret;
	}
	buffer[0] = 0x00;
	ret = gt1x_i2c_write_with_readback(_bRW_MISCTL__TMR0_EN, buffer, 1);
	if (ret) {

		GTP_ERROR("disable wdt fail!");
		return ret;
	}
	buffer[0] = 0x00;
	ret = gt1x_i2c_write_with_readback(_bRW_MISCTL__CACHE_EN, buffer, 1);
	if (ret) {

		GTP_ERROR("clear cache fail!");
		return ret;
	}
	buffer[0] = 0x01;
	ret = gt1x_i2c_write(_bWO_MISCTL__CPU_SWRST_PULSE, buffer, 1);
	if (ret) {

		GTP_ERROR("software reset fail!");
		return ret;
	}
	buffer[0] = 0x00;
	ret = gt1x_i2c_write_with_readback(_rRW_MISCTL__BOOT_OPT_B0_, buffer,
					   1);
	if (ret) {
		GTP_ERROR("set scramble fail!");
		return ret;
	}
	return 0;
}

/* came across already identical from the ALPS copy. */
int gt1x_run_ss51_isp(u8 *ss51_isp, int length)
{
	int ret;
	u8 buffer[10];
	ret = gt1x_hold_ss51_dsp();
	if (ret)
		return ret;
	buffer[0] = 0x04;
	ret = gt1x_i2c_write_with_readback(_bRW_MISCTL__SRAM_BANK, buffer, 1);
	if (ret) {
		GTP_ERROR("select bank4 fail.");
		return ret;
	}
	buffer[0] = 0x01;
	ret = gt1x_i2c_write_with_readback(_bRW_MISCTL__PATCH_AREA_EN_, buffer,
					   1);
	if (ret) {
		GTP_ERROR("enable patch area access fail!");
		return ret;
	}
	GTP_INFO("ss51_isp length: %d, checksum: 0x%04X", length,
		 gt1x_calc_checksum(ss51_isp, length));
	ret = gt1x_i2c_write(0xC000, ss51_isp, length);
	if (ret) {

		GTP_ERROR("load ss51 isp fail!");
		return ret;
	}
	ret = gt1x_recall_check(ss51_isp, 0xC000, length);
	if (ret) {

		GTP_ERROR("recall check ss51 isp fail!");
		return ret;
	}
	memset(buffer, 0xAA, 10);
	ret = gt1x_i2c_write_with_readback(0x8140, buffer, 10);
	buffer[0] = 0x00;
	ret = gt1x_i2c_write_with_readback(_bRW_MISCTL__PATCH_AREA_EN_, buffer,
					   1);
	if (ret) {


		GTP_ERROR("disable patch area access fail!");
		return ret;
	}
	memset(buffer, 0x55, 8);
	ret = gt1x_i2c_write_with_readback(0x8006, buffer, 8);
	if (ret) {

		GTP_ERROR("set 0x8006[0~7] 0x55 fail!");
		return ret;
	}
	buffer[0] = 0x08;
	ret = gt1x_i2c_write_with_readback(_rRW_MISCTL__SWRST_B0_, buffer, 1);
	if (ret) {

		GTP_ERROR("release ss51 fail!");
		return ret;
	}
	msleep(100);
	ret = gt1x_i2c_read(0x8006, buffer, 2);
	if (ret) {


		GTP_ERROR("read 0x8006 fail!");
		return ret;
	}
	if (!(buffer[0] == 0xAA && buffer[1] == 0xBB)) {
		GTP_ERROR("ERROR: isp is not running! 0x8006: %02X %02X", buffer[0], buffer[1]);
		return ERROR_CHECK;
	}
	return 0;
}

/*
 * gt1x_calc_checksum and gt1x_update_cleanup print nothing, so the
 * binary does not say which line they are on: their POSITION in the file is a
 * choice, not a measurement, and it is the only thing in here that is.
 */
u16 gt1x_calc_checksum(u8 *fw, u32 length)
{
	u32 i = 0;
	u32 checksum = 0;
	for (i = 0; i < length; i += 2) {
		checksum += (((int)fw[i]) << 8);
		checksum += fw[i + 1];
	}
	checksum &= 0xFFFF;
	return checksum;
}
int gt1x_recall_check(u8 *chk_src, u16 start_addr, u16 chk_length)
{
	u8 rd_buf[PACK_SIZE];
	s32 ret = 0;
	u16 len = 0;
	u32 compared_length = 0;
	while (chk_length > 0) {
		len = (chk_length > PACK_SIZE ? PACK_SIZE : chk_length);
		ret = gt1x_i2c_read(start_addr + compared_length, rd_buf, len);
		if (ret) {
			GTP_ERROR("recall i2c error,exit!");
			return ret;
		}
		if (memcmp(rd_buf, &chk_src[compared_length], len)) {
			GTP_ERROR("Recall frame not equal(addr: 0x%04X)",
				  start_addr + compared_length);
			GTP_DEBUG("chk_src array:");
			GTP_DEBUG_ARRAY(&chk_src[compared_length], len);
			GTP_DEBUG("recall array:");
			GTP_DEBUG_ARRAY(rd_buf, len);
			return ERROR_CHECK;
		}
		chk_length -= len;
		compared_length += len;
	}
	GTP_DEBUG("Recall check %d bytes(address: 0x%04X) success.",

		  compared_length, start_addr);
	return 0;
}


int gt1x_burn_subsystem(struct fw_subsystem_info *subsystem)
{
	int block_len;
	u16 checksum;
	int burn_len = 0;
	u16 cur_addr;
	u32 length = subsystem->length;
	u8 buffer[10];
	int ret;
	int wait_time;
	int burn_state;
	int retry = 5;
	u8 *fw;
	GTP_INFO("Subsystem: %d", subsystem->type);
	GTP_INFO("Length: %d", subsystem->length);
	GTP_INFO("Address: 0x%08X", subsystem->address);
	while (length > 0 && retry > 0) {
		retry--;
		block_len = length > 1024 * 4 ? 1024 * 4 : length;
		GTP_INFO("Burn block ==> length: %d, address: 0x%08X",


			 block_len, subsystem->address + burn_len);
		fw = gt1x_get_fw_data(subsystem->offset + burn_len, block_len);
		if (fw == NULL)
			return ERROR_FW;
		cur_addr = ((subsystem->address + burn_len) >> 8);
		checksum = 0;
		checksum += block_len;
		checksum += cur_addr;
		checksum += gt1x_calc_checksum(fw, block_len);
		checksum = (0 - checksum);
		buffer[0] = ((block_len >> 8) & 0xFF);
		buffer[1] = (block_len & 0xFF);
		buffer[2] = ((cur_addr >> 8) & 0xFF);
		buffer[3] = (cur_addr & 0xFF);
		ret = gt1x_i2c_write_with_readback(0x8100, buffer, 4);
		if (ret) {





			GTP_ERROR("write length & address fail!");
			continue;
		}
		ret = gt1x_i2c_write(0x8100 + 4, fw, block_len);
		if (ret) {

			GTP_ERROR("write fw data fail!");
			continue;
		}
		buffer[0] = ((checksum >> 8) & 0xFF);
		buffer[1] = (checksum & 0xFF);
		ret = gt1x_i2c_write_with_readback(0x8100 + 4 + block_len,
						   buffer, 2);
		if (ret) {
			GTP_ERROR("write checksum fail!");
			continue;
		}
		buffer[0] = 0;
		ret = gt1x_i2c_write_with_readback(0x8022, buffer, 1);
		if (ret) {

			GTP_ERROR("clear control flag fail!");
			continue;
		}
		buffer[0] = subsystem->type;
		buffer[1] = subsystem->type;
		ret = gt1x_i2c_write_with_readback(0x8020, buffer, 2);
		if (ret) {

			GTP_ERROR("write subsystem type fail!");
			continue;
		}
		msleep(5);
		burn_state = ERROR;
		wait_time = 200;
		while (wait_time > 0) {
			u8 buf[10];
			wait_time--;
			buf[0] = 0x55;
			ret = gt1x_i2c_read(0x8022, buffer, 1);
			if (ret < 0)
				continue;
			msleep(5);
			ret = gt1x_i2c_read(0x8022, buf, 1);
			if (ret < 0)
				continue;
			if (buffer[0] != buf[0])
				continue;
			if (buffer[0] == 0xAA) {




				GTP_DEBUG("burning.....");
				continue;
			} else if (buffer[0] == 0xDD) {
				GTP_ERROR("checksum error!");
				break;
			} else if (buffer[0] == 0xBB) {
				GTP_INFO("burning success.");
				burn_state = 0;
				break;
			} else if (buffer[0] == 0xCC) {
				GTP_ERROR("burning failed!");
				break;
			}
			GTP_DEBUG("unknown state!(0x8022: 0x%02X)", buffer[0]);
		}
		if (!burn_state) {
			length -= block_len;
			burn_len += block_len;
			retry = 5;
		}
	}
	if (length == 0)
		return 0;
	else
		return ERROR_RETRY;
}

/* it matches from the moment gt1x_get_fw_data reads from the file. */
int gt1x_check_subsystem_in_flash(struct fw_subsystem_info *subsystem)
{
	int block_len;
	int checked_len = 0;
	u32 length = subsystem->length;
	int ret;
	int check_state = 0;
	int retry = 5;
	u8 *fw;
	GTP_INFO("Subsystem: %d", subsystem->type);
	GTP_INFO("Length: %d", subsystem->length);
	GTP_INFO("Address: 0x%08X", subsystem->address);
	while (length > 0) {
		block_len = length > 1024 * 4 ? 1024 * 4 : length;
		GTP_INFO("Check block ==> length: %d, address: 0x%08X",

			 block_len, subsystem->address + checked_len);
		fw = gt1x_get_fw_data(subsystem->offset + checked_len,
				      block_len);
		if (fw == NULL)
			return ERROR_FW;
		ret = gt1x_read_flash(subsystem->address + checked_len,
				      block_len);
		if (ret)
			check_state |= ret;
		ret = gt1x_recall_check(fw, 0x8100, block_len);
		if (ret) {

			GTP_ERROR("Block in flash is broken!");
			check_state |= ret;
		}
		length -= block_len;
		checked_len += block_len;
		retry = 5;
	}
	if (check_state)

		GTP_ERROR("Subsystem in flash is broken!");
	else
		GTP_INFO("Subsystem in flash is correct!");
	return check_state;
}

/* "Error occured." with a single r: see NOTE (9). */
int gt1x_read_flash(u32 addr, int length)
{
	int wait_time;
	int ret = 0;
	u8 buffer[4];
	u16 read_addr = (addr >> 8);
	GTP_INFO("Read flash: 0x%04X, length: %d", addr, length);
	buffer[0] = 0;
	ret = gt1x_i2c_write_with_readback(0x8022, buffer, 1);
	buffer[0] = ((length >> 8) & 0xFF);
	buffer[1] = (length & 0xFF);
	buffer[2] = ((read_addr >> 8) & 0xFF);
	buffer[3] = (read_addr & 0xFF);
	ret |= gt1x_i2c_write_with_readback(0x8100, buffer, 4);
	buffer[0] = 0xAA;
	buffer[1] = 0xAA;
	ret |= gt1x_i2c_write(0x8020, buffer, 2);
	if (ret) {



		GTP_ERROR("Error occured.");
		return ret;
	}
	wait_time = 200;
	while (wait_time > 0) {
		wait_time--;
		msleep(5);
		ret = gt1x_i2c_read(0x8022, buffer, 1);
		if (ret)
			continue;
		if (buffer[0] == 0xBB) {
			GTP_INFO("Read success(addr: 0x%04X, length: %d)", addr,

				 length);
			break;
		}
	}
	if (wait_time == 0) {
		GTP_ERROR("Read Flash FAIL!");
		return ERROR_RETRY;
	}
	return 0;
}

/* the function ALPS does not have.  See NOTE (10). */
int gt1x_error_erase(void)
{
	int block_len;
	u16 checksum;
	u16 cur_addr;
	u8 buffer[10];
	int ret;
	int wait_time;
	int burn_state = ERROR;
	int retry = 5;
	u8 *fw;
	GTP_INFO("Erase flash area of ss51.");
	gt1x_reset_guitar();
	fw = gt1x_get_fw_data(upd.firmware->subsystem[0].offset,
			      upd.firmware->subsystem[0].length);
	if (fw == NULL) {


		GTP_ERROR("get isp fail");
		return ERROR_FW;
	}
	ret = gt1x_run_ss51_isp(fw, upd.firmware->subsystem[0].length);
	if (ret) {
		GTP_ERROR("run isp fail");
		return _ERROR(13);
	}
	fw = kmalloc(1024 * 4, GFP_KERNEL);
	if (fw == NULL) {

		GTP_ERROR("error when alloc mem.");
		return ERROR_MEM;
	}
	memset(fw, 0xFF, 1024 * 4);
	block_len = 1024 * 4;
	cur_addr = 0;
	while (retry-- > 0) {
		checksum = 0;
		checksum += block_len;
		checksum += cur_addr;
		checksum += gt1x_calc_checksum(fw, block_len);
		checksum = (0 - checksum);
		buffer[0] = ((block_len >> 8) & 0xFF);
		buffer[1] = (block_len & 0xFF);
		buffer[2] = ((cur_addr >> 8) & 0xFF);
		buffer[3] = (cur_addr & 0xFF);
		ret = gt1x_i2c_write_with_readback(0x8100, buffer, 4);
		if (ret) {





			GTP_ERROR("write length & address fail!");
			continue;
		}
		ret = gt1x_i2c_write(0x8100 + 4, fw, block_len);
		if (ret) {

			GTP_ERROR("write fw data fail!");
			continue;
		}
		ret = gt1x_recall_check(fw, 0x8100 + 4, block_len);
		if (ret)
			continue;
		buffer[0] = ((checksum >> 8) & 0xFF);
		buffer[1] = (checksum & 0xFF);
		ret = gt1x_i2c_write_with_readback(0x8100 + 4 + block_len,
						   buffer, 2);
		if (ret) {


			GTP_ERROR("write checksum fail!");
			continue;
		}
		buffer[0] = 0;
		ret = gt1x_i2c_write_with_readback(0x8022, buffer, 1);
		if (ret) {

			GTP_ERROR("clear control flag fail!");
			continue;
		}
		buffer[0] = 0x02;
		buffer[1] = 0x02;
		ret = gt1x_i2c_write_with_readback(0x8020, buffer, 2);
		if (ret) {

			GTP_ERROR("write subsystem type fail!");
			continue;
		}
		burn_state = ERROR;
		wait_time = 200;
		while (wait_time > 0) {
			wait_time--;
			msleep(5);
			ret = gt1x_i2c_read_dbl_check(0x8022, buffer, 1);
			if (ret)
				continue;
			if (buffer[0] == 0xAA) {


				GTP_DEBUG("burning.....");
				continue;
			} else if (buffer[0] == 0xDD) {
				GTP_ERROR("checksum error!");
				break;
			} else if (buffer[0] == 0xBB) {
				GTP_INFO("burning success.");
				burn_state = 0;
				break;
			} else if (buffer[0] == 0xCC) {
				GTP_ERROR("burning failed!");
				break;
			}
			GTP_DEBUG("unknown state!(0x8022: 0x%02X)", buffer[0]);
		}
	}
	kfree(fw);
	if (!burn_state)
		return 0;
	return ERROR_RETRY;
}

/* the 2 in `status` skips the reset: see NOTE (11). */
void gt1x_leave_update_mode(void)
{
	GTP_DEBUG("Leave FW update mode.");
	if (upd.status != 2)
		gt1x_reset_guitar();
	upd.status = 0;
	gt1x_irq_enable();
}

void gt1x_update_cleanup(void)
{
	if (upd.update_type == UPDATE_TYPE_FILE) {
		if (upd.fw_file != NULL) {
			filp_close(upd.fw_file, NULL);
			upd.fw_file = NULL;
		}
		set_fs(upd.old_fs);
	}
	if (upd.buffer != NULL) {
		kfree(upd.buffer);
		upd.buffer = NULL;
	}
	if (upd.firmware != NULL) {
		kfree(upd.firmware);
		upd.firmware = NULL;
	}
}

/*
 * gt1x_hold_ss51_dsp_no_reset() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_update.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int gt1x_hold_ss51_dsp_no_reset(void)
{
	int ret = ERROR;
	u8 buffer[2];
	ret = __gt1x_hold_ss51_dsp_20();
	if (ret)
		return ret;
	buffer[0] = 0x00;
	ret = gt1x_i2c_write_with_readback(_bRW_MISCTL__DSP_MCU_PWR_, buffer,
					   1);
	if (ret) {
		GTP_ERROR("enabel dsp & mcu power fail!");
		return ret;
	}
	buffer[0] = 0x00;
	ret = gt1x_i2c_write_with_readback(_bRW_MISCTL__TMR0_EN, buffer, 1);
	if (ret) {

		GTP_ERROR("disable wdt fail!");
		return ret;
	}
	buffer[0] = 0x00;
	ret = gt1x_i2c_write_with_readback(_bRW_MISCTL__CACHE_EN, buffer, 1);
	if (ret) {

		GTP_ERROR("clear cache fail!");
		return ret;
	}
	buffer[0] = 0x01;
	ret = gt1x_i2c_write(_bWO_MISCTL__CPU_SWRST_PULSE, buffer, 1);
	if (ret) {

		GTP_ERROR("software reset fail!");
		return ret;
	}
	buffer[0] = 0x00;
	ret = gt1x_i2c_write_with_readback(_rRW_MISCTL__BOOT_OPT_B0_, buffer,
					   1);
	if (ret) {
		GTP_ERROR("set scramble fail!");
		return ret;
	}
	return 0;
}
#define GT1X_LOAD_PACKET_SIZE (1024 * 2)

int gt1x_load_patch(u8 *patch, u32 patch_size, int offset, int bank_size)
{
	s32 loaded_length = 0;
	s32 len = 0;
	s32 ret = 0;
	u8 bank = 0, tmp;
	u16 address;
	GTP_INFO(
		"Load patch code(size: %d, checksum: 0x%04X, position: 0x%04X, bank-size: %d",
		patch_size, gt1x_calc_checksum(patch, patch_size),
		0xC000 + offset, bank_size);
	while (loaded_length != patch_size) {
		if (loaded_length == 0 ||
		    (loaded_length + offset) % bank_size == 0) {
			bank = 0x04 + (loaded_length + offset) / bank_size;
			ret = gt1x_i2c_write(_bRW_MISCTL__SRAM_BANK, &bank, 1);
			if (ret) {
				GTP_ERROR("select bank%d fail!", bank);
				return ret;
			}
			GTP_INFO("Select bank%d success.", bank);
			tmp = 0x01;
			ret = gt1x_i2c_write_with_readback(
				_bRW_MISCTL__PATCH_AREA_EN_ + bank - 4, &tmp, 1);
			if (ret) {
				GTP_ERROR("enable patch area access fail!");
				return ret;
			}
		}
		len = patch_size - loaded_length > GT1X_LOAD_PACKET_SIZE
			      ? GT1X_LOAD_PACKET_SIZE
			      : patch_size - loaded_length;
		address = 0xC000 + (loaded_length + offset) % bank_size;
		ret = gt1x_i2c_write(address, &patch[loaded_length], len);
		if (ret) {
			GTP_ERROR("load 0x%04X, %dbytes fail!", address, len);
			return ret;
		}
		ret = gt1x_recall_check(&patch[loaded_length], address, len);
		if (ret) {
			GTP_ERROR("Recall check 0x%04X, %dbytes fail!", address, len);
			return ret;
		}
		GTP_INFO("load code 0x%04X, %dbytes success.", address, len);
		loaded_length += len;
	}
	return 0;
}

int gt1x_startup_patch(void)
{
	s32 ret = 0;
	u8 buffer[8] = {0x55};
	buffer[0] = 0x00;
	buffer[1] = 0x00;
	ret |= gt1x_i2c_write(_bRW_MISCTL__PATCH_AREA_EN_, buffer, 2);
	memset(buffer, 0x55, 8);
	ret |= gt1x_i2c_write(GTP_REG_FLASH_PASSBY, buffer, 8);
	ret |= gt1x_i2c_write(GTP_REG_VERSION, buffer, 5);
	buffer[0] = 0xAA;
	ret |= gt1x_i2c_write(GTP_REG_CMD, buffer, 1);
	ret |= gt1x_i2c_write(GTP_REG_ESD_CHECK, buffer, 1);
	buffer[0] = 0x00;
	ret |= gt1x_i2c_write(_rRW_MISCTL__SWRST_B0_, buffer, 1);
	msleep(200);
	return ret;
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998c700).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_update.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800924c6b6).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_update.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

