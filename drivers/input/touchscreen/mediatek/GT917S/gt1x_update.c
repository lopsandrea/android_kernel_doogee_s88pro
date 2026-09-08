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

/* DELTA DI HEADER: di fabbrica `struct fw_update_info` ha un `int` in piu' a
 * offset 16, e da `firmware` in poi tutti i campi slittano di otto byte.  I
 * dodici offset sono MISURATI uno per uno -- citazioni in coda, NOTE (1).
 * La riga da aggiungere all'header e' `int c16;`, e la fa il lotto di merge.
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


/* Dichiarazioni in avanti: l'ordine qui e' quello dei __LINE__, NOTE (2). */
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

/* gt1x_search_update_files -- di fabbrica non ha simbolo proprio: e'
 *
 * incorporata in gt1x_auto_update_proc, e a dirlo e' il __func__ delle sue
 * tre printk, "gt1x_search_update_files"@0xffffff800924da9d, diverso da
 * quello del contenitore, "gt1x_auto_update_proc"@0xffffff800924c6a0.
 * E' la classe B5 vista dal lato buono.  Vedi NOTE (3).
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

/* nessun gt1x_esd_switch: la fabbrica chiama solo printk e
 * gt1x_irq_disable ("97fff88c bl"@0xffffff8008a78c14).
 */
void gt1x_enter_update_mode(void)
{
	GTP_DEBUG("Enter FW update mode.");
	gt1x_irq_disable();
}

/* la fabbrica NON ricontrolla il flash: dopo il ciclo di gt1x_burn_subsystem
 *
 *
 * si va dritti all'uscita ("17ffff55 b"@0xffffff8008a78bcc verso
 * 0xffffff8008a78920), senza seconda gt1x_run_ss51_isp e senza il ciclo di
 * gt1x_check_subsystem_in_flash.  Vedi NOTE (4).
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

/* aggiornamento da FILE, non dall'header: vedi NOTE (5) per le prove e per
 *
 *
 * i due difetti di fabbrica riprodotti qui dentro.
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

/* gt1x_check_firmware combacia da quando gt1x_get_fw_data legge dal file:
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * era -336 byte, e non aveva un difetto suo -- il -336 era tutto della
 * gt1x_get_fw_data che ALPS riduce a un `return &fw_data[offset];`.
 * Le sei GTP_DEBUG del blocco di stampa in fondo sono DEBUG e non INFO:
 * i letterali di fabbrica portano il prefisso <<GTP-DBG>>, per esempio
 * "<<GTP-DBG>>[%s:%d]Type: %d\n"@0xffffff800924cca9, e la copia ALPS
 * usava GTP_INFO.  E' una divergenza che la DIMENSIONE non denuncia:
 * cambia solo quale stringa di .rodata l'istruzione indirizza.
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

/* legge dal file quando update_type vale 1.  Vedi NOTE (6).
 *
 *
 */
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

/* il limite di ritentativi e' 30, non 2000.  Vedi NOTE (7).
 *
 */
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

/* comincia con un ciclo di lettura di 0x4220 che ALPS non ha, e non ha
 *
 *
 *
 * nessuna msleep(20).  Vedi NOTE (8).
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

/* arrivata gia' identica dalla copia ALPS.
 *
 *
 */
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

/* gt1x_calc_checksum e gt1x_update_cleanup non stampano niente, quindi il
 * binario non dice a che riga stanno: la loro POSIZIONE nel file e' una
 * scelta, non una misura, ed e' l'unica cosa qui dentro che lo sia.
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

/* combacia da quando gt1x_get_fw_data legge dal file.
 *
 *
 */
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

/* "Error occured." con una sola r: vedi NOTE (9).
 */
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

/* la funzione che ALPS non ha.  Vedi NOTE (10).
 */
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

/* il 2 di `status` salta il reset: vedi NOTE (11).
 *
 *
 */
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

/* arrivata gia' identica dalla copia ALPS.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
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

/* ==========================================================================
 * NOTE -- le prove, nella forma verificabile.  Stanno in coda e non dentro le
 * funzioni perche' l'ordine e la SPAZIATURA di questo file non sono liberi:
 * ognuna delle 124 macro GTP_* emette il proprio __LINE__ nel binario, e i
 * 124 valori di fabbrica fissano la riga di ogni chiamata.  Dalla riga 1467
 * in giu' non c'e' piu' niente da fissare.
 *
 * (1) DELTA DI HEADER, il campo `c16` e i sette che gli slittano dietro.
 *     Offset misurati uno per uno su update_info (base 0xffffff800998c700):
 *       +0  update_type  "b9470108 ldr"@0xffffff8008a7a024, poi `cmp w8,#0x1`
 *       +4  status       "b9470708 ldr"@0xffffff8008a78870 (cbz all'ingresso
 *                        di gt1x_update_firmware) e
 *                        "b907071f str"@0xffffff8008a7894c
 *       +8  progress     "b940092a ldr"@0xffffff8008a789f0 (x9 = base+0x700)
 *       +12 max_progress "29002329 stp"@0xffffff8008a78908, che scrive +8 e
 *                        +12 con una sola istruzione
 *       +16 c16          il campo NUOVO: lo scrive gt1x_debug_write_proc
 *                        ("b9071128 str"@0xffffff8008a75f30) e lo legge
 *                        gt1x_update_judge ("b9471129 ldr"@0xffffff8008a79638)
 *       +24 firmware     "f9438f88 ldr"@0xffffff8008a78b58, e il kfree di
 *                        gt1x_update_cleanup "f9438e60 ldr"@0xffffff8008a7a094
 *       +32 fw_length    "b9000a80 str"@0xffffff8008a78d68 (x20 = base+0x718,
 *                        quindi +8 = base+32) subito dopo la seconda llseek
 *       +40 fw_name      "f90016b4 str"@0xffffff8008a78c98 (x21 = base+0x700)
 *       +48 buffer       "f9039ac0 str"@0xffffff8008a78e68 e il kfree
 *                        "f9439a60 ldr"@0xffffff8008a7a080
 *       +56 old_fs       "f9001ea8 str"@0xffffff8008a78c68 (get_fs salvato)
 *       +64 fw_file      "f90022a0 str"@0xffffff8008a78ca8 (esito di filp_open)
 *       +72 fw_data      "f943a508 ldr"@0xffffff8008a7971c in gt1x_get_fw_data
 *     CONSEGUENZA DICHIARATA, e non e' una misura: finche' l'header non
 *     cambia, `update_info` occupa 72 byte e non 80, e il campo a offset 72
 *     (`fw_data`) cade oltre la fine dell'oggetto.  Il `.text` che l'oracolo
 *     giudica e' corretto; la RISERVA dei byte no, e si chiude con quella
 *     riga sola.  gt1x_generic.c fa gia' la stessa cosa per il solo `c16`.
 *
 * (2) L'ORDINE DELLE DEFINIZIONI.  Non e' una scelta: i 124 __LINE__ dicono
 *     dove ogni funzione sta.  Due sole funzioni non stampano niente --
 *     gt1x_calc_checksum e gt1x_update_cleanup -- e per quelle due la
 *     posizione E' una scelta, dichiarata qui e da nessun'altra parte.
 *     Nota: l'ordine delle definizioni non e' l'ordine del `.text`; clang
 *     riordina, e infatti nel nostro `.o` gt1x_update_firmware sta al quinto
 *     posto pur essendo definita per settima, esattamente come di fabbrica.
 *
 * (3) gt1x_search_update_files, incorporata in gt1x_auto_update_proc.
 *     La prova e' il __func__ delle sue printk, diverso da quello del
 *     contenitore: "gt1x_search_update_files"@0xffffff800924da9d contro
 *     "gt1x_auto_update_proc"@0xffffff800924c6a0.  E' incorporata anche
 *     gt1x_check_fs_mounted, che pero' ha ANCHE un corpo suo a
 *     0xffffff8008a7839c: le due kern_path, il confronto dei due `mnt_sb`
 *     ("f9400514 ldr"@0xffffff8008a78704 e "f9400536 ldr"@0xffffff8008a78708,
 *     offset 8 di struct vfsmount) e le due path_put sono il suo corpo, e il
 *     valore che ne esce e' ERROR_PATH ("52840014 mov"@0xffffff8008a786ec con
 *     "72b00014 movk"@0xffffff8008a786f0, cioe' 0x80002000).
 *     Il ciclo e' un CONTO ALLA ROVESCIA da 40: "128004f5 mov"@0xffffff8008a785e0
 *     lo parte a -40 e "310006b5 adds"@0xffffff8008a7873c con
 *     "54fffc03 b.cc"@0xffffff8008a78740 lo chiude sul riporto, senza nessun
 *     `cmp`.  Misurato: `for (i = 0; i < 40; i++)` da' 840 byte con add+cmp;
 *     la forma discendente da' gli 836 esatti.  Quale delle due scritture ci
 *     fosse non lo dice il binario -- dice quale RIPRODUCE.
 *     La msleep e' di 500 ms: "52803e80 mov"@0xffffff8008a786c0.
 *
 * (4) gt1x_update_firmware NON ricontrolla il flash.  Dopo il ciclo di
 *     gt1x_burn_subsystem si va dritti all'uscita
 *     ("17ffff55 b"@0xffffff8008a78bcc verso 0xffffff8008a78920): niente
 *     seconda gt1x_run_ss51_isp, niente ciclo di
 *     gt1x_check_subsystem_in_flash, che ALPS invece ha.  `max_progress` non
 *     e' la costante 9 di ALPS ma `subsystem_count + 3`
 *     ("11000d08 add"@0xffffff8008a78904), e sul ramo d'errore `progress`
 *     diventa il doppio di `max_progress` ("531f7929 lsl"@0xffffff8008a78978).
 *
 * (5) gt1x_update_prepare apre un FILE.  "97dfb36f bl"@0xffffff8008a78ca0
 *     chiama filp_open, e le due llseek che seguono
 *     ("d63f0100 blr"@0xffffff8008a78d38 e @0xffffff8008a78d58) danno
 *     `fw_length`.  DUE DIFETTI DI FABBRICA, riprodotti:
 *     - il primo ciclo di allocazione va a fondo anche quando l'ultimo
 *       tentativo RIESCE: dopo la quinta kzalloc si passa comunque per
 *       filp_close e ERROR_RETRY ("51401d14 sub"@0xffffff8008a78f6c);
 *     - il secondo ciclo non decrementa `retry`, quindi non ha fondo:
 *       "b4fffea0 cbz"@0xffffff8008a78eac torna alla printk d'errore senza
 *       toccare nessun contatore, e dopo il ciclo non c'e' nessun controllo
 *       `retry <= 0` ("2a1f03f4 mov"@0xffffff8008a78ecc mette w20 a zero e si
 *       esce con 0).  Non c'e' quindi nemmeno il ramo che libera
 *       `upd.firmware`: l'etichetta gt1x_update_pre_fail0 di ALPS qui non c'e'.
 *     Il ramo d'errore non rimette a posto `old_fs`: "f943a100 ldr"
 *     @0xffffff8008a78f5c e' seguita subito da filp_close e il ritorno non
 *     passa da nessun `mrs sp_el0`.  Anche questo e' della fabbrica.
 *
 * (6) gt1x_get_fw_data legge dal file quando `update_type` vale 1
 *     ("7100051f cmp"@0xffffff8008a796bc).  La lunghezza e' un `int` e ci
 *     arriva con estensione di SEGNO: "93407e62 sxtw"@0xffffff8008a796f4.
 *     L'offset invece e' senza segno: "8b204100 add"@0xffffff8008a79720 lo
 *     estende con `uxtw`.  E' la classe A3 del criterio di revisione.
 *
 * (7) __gt1x_hold_ss51_dsp_20 ritenta 30 volte, non 2000.  Lo dicono
 *     "710076ff cmp"@0xffffff8008a7a148 (`cmp w23,#0x1d`) e
 *     "71007aff cmp"@0xffffff8008a7a1a4 (`cmp w23,#0x1e`).  Questa funzione
 *     misurava 356 byte su 356 fin dal principio: e' un difetto che la
 *     DIMENSIONE non denuncia e che ha denunciato il confronto per codifica.
 *
 * (8) gt1x_hold_ss51_dsp comincia con un ciclo che ALPS non ha: legge 0x4220
 *     ("52884400 mov"@0xffffff8008a7a288) finche' riesce, e il contatore parte
 *     da -5 ("321d7bf3 orr"@0xffffff8008a7a278), esce a zero
 *     ("34000073 cbz"@0xffffff8008a7a294) e viene incrementato DOPO il
 *     confronto ("11000673 add"@0xffffff8008a7a298), cioe' `retry--` valutato
 *     per primo.  Non c'e' nessuna msleep(20).
 *
 * (9) "Error occured." con una sola `r`.  Il letterale di fabbrica e'
 *     "<<GTP-ERR>>[%s:%d] Error occured.\n"@0xffffff800924d759; ALPS scrive
 *     "occurred".  E' un difetto della fabbrica e si riproduce (regola 7).
 *     Nella stessa funzione la msleep del ciclo d'attesa e' di 5 ms e non di
 *     20: "528000a0 mov"@0xffffff8008a7a998.  Anche questo la dimensione non
 *     lo denunciava -- gt1x_read_flash misurava 480 su 480 dal principio.
 *
 * (10) gt1x_error_erase, la funzione che ALPS non ha: 1172 byte, 293
 *      istruzioni.  Scrive 4096 byte di 0xFF nell'area ss51 e li ribatte
 *      cinque volte, sempre e comunque -- anche dopo un "burning success.",
 *      perche' il ramo di successo salta al FONDO del ciclo
 *      ("14000032 b"@0xffffff8008a7adc8 verso 0xffffff8008a7ae90) e non fuori.
 *      Il buffer e' kmalloc e non kzalloc -- le flag sono 0x14000c0
 *      ("52801801 mov"@0xffffff8008a7ab48 con
 *      "72a02801 movk"@0xffffff8008a7ab50), senza il bit 0x8000 di __GFP_ZERO
 *      che la kzalloc di gt1x_update_prepare invece ha ("52901801 mov"
 *      @0xffffff8008a78d64) -- e viene riempito a mano con
 *      "940f7524 bl"@0xffffff8008a7ab70 verso __memset, con
 *      "32001fe1 orr"@0xffffff8008a7ab64 che mette 0xff in w1.
 *      La lunghezza e' 4096: "f140051f cmp"@0xffffff8008a7ae14 chiude il ciclo
 *      di checksum a `#0x1, lsl #12`, e i quattro byte di testata sono
 *      0x10,0,0,0 ("321c03f9 orr"@0xffffff8008a7ab8c mette 0x10 in w25 e
 *      "b9000bf9 str"@0xffffff8008a7ae30 lo scrive come parola).
 *      Il tipo scritto in 0x8020 e' 0x02 due volte:
 *      "52804048 mov"@0xffffff8008a7acc4 mette 0x202 e
 *      "790013e8 strh"@0xffffff8008a7acd4 lo scrive a mezza parola.
 *      L'attesa usa gt1x_i2c_read_dbl_check ("97ffe446 bl"@0xffffff8008a7ad3c)
 *      e non due gt1x_i2c_read come gt1x_burn_subsystem.
 *      I valori d'errore: ERROR_FW sul "get isp fail"
 *      ("11400913 add"@0xffffff8008a7abe0 = 0x80002000+0x2000), ERROR_MEM
 *      sull'alloc ("11001af3 add"@0xffffff8008a7aecc = 0x80000002+6),
 *      ERROR_RETRY in coda ("113ffae8 add"@0xffffff8008a7aea4), e sul
 *      "run isp fail" un 0x80002000 che e' _ERROR(13)
 *      ("52840013 mov"@0xffffff8008a7ab24 con
 *      "72b00013 movk"@0xffffff8008a7ab34): il bit e' misurato, il nome che
 *      l'header di fabbrica gli dava no.
 *
 * (11) gt1x_leave_update_mode salta il reset quando `status` vale 2:
 *      "b9470668 ldr"@0xffffff8008a7a0d8 e "7100091f cmp"@0xffffff8008a7a0dc.
 *      Il 2 lo scrivono i rami d'errore di gt1x_update_firmware
 *      ("321f03e8 orr"@0xffffff8008a78914).  Il binario non lo nomina.
 *
 * (12) gt1x_burn_subsystem legge 0x8022 DUE volte, con cinque millisecondi in
 *      mezzo, e va avanti solo se le due letture concordano: la seconda
 *      gt1x_i2c_read punta a un secondo vettore
 *      ("910053e1 add"@0xffffff8008a79e64, x1 = sp+0x14 contro il sp+0x8 della
 *      prima) e il confronto e' "6b08007f cmp"@0xffffff8008a79e80 seguito da
 *      "54000301 b.ne"@0xffffff8008a79e84.  Il seme 0x55 del secondo vettore e'
 *      "52800aa8 mov"@0xffffff8008a79e40 con "390053e8 strb"@0xffffff8008a79e50.
 *      La fabbrica NON fa la gt1x_recall_check dopo aver scritto i dati (ALPS
 *      si': erano 64 byte in piu'), e la lettura del ciclo d'attesa prova il
 *      SEGNO e non lo zero -- "37f80460 tbnz"@0xffffff8008a79e58.
 *
 * (13) read_reg, TOLTA.  Nel binario di fabbrica non c'e' ne' come funzione ne'
 *      incorporata: il suo unico letterale non compare in tutta l'immagine.
 *        $ strings oracolo/stock.elf | grep -cF 'Read address: 0x%04X, Length: %d'
 *        0
 *      (controprova sulla stessa scansione: 'Enter FW update mode.' da' 1 e
 *      'Erase flash area of ss51.' da' 1.)  Il cappello di gt1x_wtk.c la
 *      contava gia' fra le quattro funzioni di ALPS che la fabbrica non ha.
 * ==========================================================================
 */

/* (14) I 106 LETTERALI DI QUESTA UNITA', uno per uno, con l'indirizzo dei
 *      byte che producono nel binario di fabbrica.  I messaggi non esistono
 *      nell'immagine come li scrive il codice: le macro Goodix li saldano al
 *      proprio prefisso (`<<GTP-ERR>>[%s:%d] `, `<<GTP-INF>>[%s:%d] `,
 *      `<<GTP-DBG>>[%s:%d]`) e alla `\n`, e la citazione porta il testo
 *      ASSEMBLATO, che e' quello che sta davvero a quell'indirizzo.
 *      I quattro letterali che non compaiono sono i tre percorsi di #include
 *      e "gt1151_default_", che sta sotto CONFIG_GTP_REQUEST_FW_UPDATE, spenta.
 *
 * "/data/_goodix_update_.bin"@0xffffff800924c6b6
 * "/sdcard/_goodix_update_.bin"@0xffffff800924c6d0
 * "/data/_gt1x_config_.cfg"@0xffffff800924c6ec
 * "/sdcard/_gt1x_config_.cfg"@0xffffff800924c704
 * "/" -- INDIRIZZO NON DETERMINATO. Quello che stava qui,
 *     0xffffff80080811d9, cade in .text, e un letterale di stringa in .text
 *     non c'e' mai: la scansione aveva preso il PRIMO byte uguale in tutta
 *     l'immagine (2833 occorrenze), non il letterale. Da rifare cercando
 *     nella sola .rodata -- oppure da togliere, perche' qui '/' e' una
 *     costante di carattere passata a strrchr, e una costante di carattere
 *     vive in un immediato d'istruzione, non ha un indirizzo suo.
 * "<<GTP-INF>>[%s:%d] Start auto update thread...\n"@0xffffff800924c670
 * "<<GTP-ERR>>[%s:%d] Update config failed!\n"@0xffffff800924c71e
 * "<<GTP-INF>>[%s:%d] Update config successfully!\n"@0xffffff800924c748
 * "<<GTP-INF>>[%s:%d] Search firmware file...\n"@0xffffff800924da71
 * "/data"@0xffffff800924dab6
 * "<<GTP-DBG>>[%s:%d]filesystem is not ready\n"@0xffffff800924dabc
 * "<<GTP-INF>>[%s:%d] Not found firmware or config file, retry.\n"@0xffffff800924dae7
 * "<<GTP-DBG>>[%s:%d]Enter FW update mode.\n"@0xffffff800924c778
 * "<<GTP-ERR>>[%s:%d] Update process is running!\n"@0xffffff800924c7b8
 * "<<GTP-ERR>>[%s:%d] get isp fail\n"@0xffffff800924c7fc
 * "<<GTP-ERR>>[%s:%d] run isp fail\n"@0xffffff800924c81d
 * "<<GTP-INF>>[%s:%d] subsystem: %d\n"@0xffffff800924c83e
 * "<<GTP-INF>>[%s:%d] Length: %d\n"@0xffffff800924c860
 * "<<GTP-INF>>[%s:%d] Address: %d\n"@0xffffff800924c87f
 * "<<GTP-ERR>>[%s:%d] burn subsystem fail!\n"@0xffffff800924c89f
 * "<<GTP-ERR>>[%s:%d] Update firmware failed!\n"@0xffffff800924c8c8
 * "<<GTP-INF>>[%s:%d] Update firmware succeefully!\n"@0xffffff800924c8f4
 * "<<GTP-ERR>>[%s:%d] No Fw in .h file!\n"@0xffffff800924c925
 * "<<GTP-INF>>[%s:%d] Firmware: %s\n"@0xffffff800924c95f
 * "<<GTP-ERR>>[%s:%d] Open update file(%s) error!\n"@0xffffff800924c980
 * "<<GTP-INF>>[%s:%d] Alloc %zu bytes memory fail.\n"@0xffffff800924c9b0
 * "<<GTP-INF>>[%s:%d] Alloc %zu bytes memory success.\n"@0xffffff800924c9e1
 * "<<GTP-ERR>>[%s:%d] Alloc %d bytes memory fail.\n"@0xffffff800924ca15
 * "<<GTP-INF>>[%s:%d] Alloc %d bytes memory success.\n"@0xffffff800924ca45
 * "<<GTP-ERR>>[%s:%d] Bad firmware!(file length: %d)\n"@0xffffff800924ca78
 * "<<GTP-ERR>>[%s:%d] Bad firmware!(file length: %d, header define: %d)\n"@0xffffff800924cabf
 * "<<GTP-ERR>>[%s:%d] Bad firmware!(checksum: 0x%04X, header define: 0x%04X)\n"@0xffffff800924cb05
 * "<<GTP-INF>>[%s:%d] Update type: %s\n"@0xffffff800924cb50
 * "Header"@0xffffff80091b53c0
 * "File"@0xffffff800927b2eb -- 2 occorrenze
 * "<<GTP-INF>>[%s:%d] Firmware length: %d\n"@0xffffff800924cb74
 * "<<GTP-INF>>[%s:%d] Firmware product: GT%s\n"@0xffffff800924cb9c
 * "<<GTP-INF>>[%s:%d] Firmware patch: %02X%02X%02X\n"@0xffffff800924cbc7
 * "<<GTP-INF>>[%s:%d] Firmware chip: 0x%02X\n"@0xffffff800924cbf8
 * "<<GTP-INF>>[%s:%d] Subsystem count: %d\n"@0xffffff800924cc22
 * "<<GTP-DBG>>[%s:%d]------------------------------------------\n"@0xffffff800924cc4a
 * "<<GTP-INF>>[%s:%d] Subsystem: %d\n"@0xffffff800924d3ef
 * "<<GTP-DBG>>[%s:%d]Type: %d\n"@0xffffff800924cca9
 * "<<GTP-INF>>[%s:%d] Address: 0x%08X\n"@0xffffff800924d425
 * "<<GTP-DBG>>[%s:%d]Offset: %d\n"@0xffffff800924cd06
 * "<<GTP-ERR>>[%s:%d] Read data error!\n"@0xffffff800924cd24
 * "<<GTP-INF>>[%s:%d] Update abort because of i2c error.\n"@0xffffff800924cd5a
 * "<<GTP-INF>>[%s:%d] Check fw status reg not pass,reg[0x814E]=0x%2X,reg[0x5095]=0x%2X!\n"@0xffffff800924cda3
 * "<<GTP-INF>>[%s:%d] Get IC's version info failed, force update!\n"@0xffffff800924cdf9
 * "<<GTP-INF>>[%s:%d] Product id is not match!\n"@0xffffff800924ce39
 * "<<GTP-INF>>[%s:%d] Mask id is not match!\n"@0xffffff800924ce66
 * "<<GTP-INF>>[%s:%d] CID is not equal, need update!\n"@0xffffff800924ce90
 * "<<GTP-DBG>>[%s:%d]Debug mode, force update fw.\n"@0xffffff800924cec3
 * "<<GTP-INF>>[%s:%d] The version of the fw is not high than the IC's!\n"@0xffffff800924cef3
 * "<<GTP-ERR>>[%s:%d] Hold ss51 & dsp I2C error,retry:%d\n"@0xffffff800924cf38
 * "<<GTP-ERR>>[%s:%d] Hold ss51 & dsp confirm 0x4180 failed,value:%d\n"@0xffffff800924cf87
 * "<<GTP-ERR>>[%s:%d] Hold ss51&dsp failed!\n"@0xffffff800924cfca
 * "<<GTP-INF>>[%s:%d] Hold ss51&dsp successfully.\n"@0xffffff800924cff4
 * "<<GTP-ERR>>[%s:%d] enabel dsp & mcu power fail!\n"@0xffffff800924d024
 * "<<GTP-ERR>>[%s:%d] disable wdt fail!\n"@0xffffff800924d068
 * "<<GTP-ERR>>[%s:%d] clear cache fail!\n"@0xffffff800924d08e
 * "<<GTP-ERR>>[%s:%d] software reset fail!\n"@0xffffff800924d0b4
 * "<<GTP-ERR>>[%s:%d] set scramble fail!\n"@0xffffff800924d0dd
 * "<<GTP-ERR>>[%s:%d] select bank4 fail.\n"@0xffffff800924d104
 * "<<GTP-ERR>>[%s:%d] enable patch area access fail!\n"@0xffffff800924d13d
 * "<<GTP-INF>>[%s:%d] ss51_isp length: %d, checksum: 0x%04X\n"@0xffffff800924d170
 * "<<GTP-ERR>>[%s:%d] load ss51 isp fail!\n"@0xffffff800924d1aa
 * "<<GTP-ERR>>[%s:%d] recall check ss51 isp fail!\n"@0xffffff800924d1d2
 * "<<GTP-ERR>>[%s:%d] disable patch area access fail!\n"@0xffffff800924d202
 * "<<GTP-ERR>>[%s:%d] set 0x8006[0~7] 0x55 fail!\n"@0xffffff800924d236
 * "<<GTP-ERR>>[%s:%d] release ss51 fail!\n"@0xffffff800924d265
 * "<<GTP-ERR>>[%s:%d] read 0x8006 fail!\n"@0xffffff800924d28c
 * "<<GTP-ERR>>[%s:%d] ERROR: isp is not running! 0x8006: %02X %02X\n"@0xffffff800924d2b2
 * "<<GTP-ERR>>[%s:%d] recall i2c error,exit!\n"@0xffffff800924d2f3
 * "<<GTP-ERR>>[%s:%d] Recall frame not equal(addr: 0x%04X)\n"@0xffffff800924d330
 * "<<GTP-DBG>>[%s:%d]chk_src array:\n"@0xffffff800924d369
 * "<<GTP-DBG>>[%s:%d]recall array:\n"@0xffffff800924d38b
 * "<<GTP-DBG>>[%s:%d]Recall check %d bytes(address: 0x%04X) success.\n"@0xffffff800924d3ac
 * "<<GTP-INF>>[%s:%d] Burn block ==> length: %d, address: 0x%08X\n"@0xffffff800924d449
 * "<<GTP-ERR>>[%s:%d] write length & address fail!\n"@0xffffff800924d488
 * "<<GTP-ERR>>[%s:%d] write fw data fail!\n"@0xffffff800924d4b9
 * "<<GTP-ERR>>[%s:%d] write checksum fail!\n"@0xffffff800924d4e1
 * "<<GTP-ERR>>[%s:%d] clear control flag fail!\n"@0xffffff800924d50a
 * "<<GTP-ERR>>[%s:%d] write subsystem type fail!\n"@0xffffff800924d537
 * "<<GTP-DBG>>[%s:%d]burning.....\n"@0xffffff800924d566
 * "<<GTP-ERR>>[%s:%d] checksum error!\n"@0xffffff800924d586
 * "<<GTP-INF>>[%s:%d] burning success.\n"@0xffffff800924d5aa
 * "<<GTP-ERR>>[%s:%d] burning failed!\n"@0xffffff800924d5cf
 * "<<GTP-DBG>>[%s:%d]unknown state!(0x8022: 0x%02X)\n"@0xffffff800924d5f3
 * "<<GTP-INF>>[%s:%d] Check block ==> length: %d, address: 0x%08X\n"@0xffffff800924d643
 * "<<GTP-ERR>>[%s:%d] Block in flash is broken!\n"@0xffffff800924d683
 * "<<GTP-ERR>>[%s:%d] Subsystem in flash is broken!\n"@0xffffff800924d6b1
 * "<<GTP-INF>>[%s:%d] Subsystem in flash is correct!\n"@0xffffff800924d6e3
 * "<<GTP-INF>>[%s:%d] Read flash: 0x%04X, length: %d\n"@0xffffff800924d716
 * "<<GTP-ERR>>[%s:%d] Error occured.\n"@0xffffff800924d759
 * "<<GTP-INF>>[%s:%d] Read success(addr: 0x%04X, length: %d)\n"@0xffffff800924d77c
 * "<<GTP-ERR>>[%s:%d] Read Flash FAIL!\n"@0xffffff800924d7b7
 * "<<GTP-INF>>[%s:%d] Erase flash area of ss51.\n"@0xffffff800924d7dc
 * "<<GTP-ERR>>[%s:%d] error when alloc mem.\n"@0xffffff800924d81b
 * "<<GTP-DBG>>[%s:%d]Leave FW update mode.\n"@0xffffff800924d845
 * "<<GTP-INF>>[%s:%d] Load patch code(size: %d, checksum: 0x%04X, position: 0x%04X, bank-size: %d\n"@0xffffff800924d911
 * "<<GTP-ERR>>[%s:%d] select bank%d fail!\n"@0xffffff800924d981
 * "<<GTP-INF>>[%s:%d] Select bank%d success.\n"@0xffffff800924d9a9
 * "<<GTP-ERR>>[%s:%d] load 0x%04X, %dbytes fail!\n"@0xffffff800924d9d4
 * "<<GTP-ERR>>[%s:%d] Recall check 0x%04X, %dbytes fail!\n"@0xffffff800924da03
 * "<<GTP-INF>>[%s:%d] load code 0x%04X, %dbytes success.\n"@0xffffff800924da3a
 */

