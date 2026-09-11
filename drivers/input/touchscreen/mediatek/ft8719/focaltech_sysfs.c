// SPDX-License-Identifier: GPL-2.0
/*
 * FocalTech FT8719 touch panel, Doogee S88 Pro (MT6771) -- the /proc channel
 * and sysfs nodes translation unit. Complete: 22 functions out of 22, 5708
 * bytes out of 5708.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read; the two FocalTech drivers already in the ALPS tree were not
 * opened.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "tpd.h"

/*
 * The three macros, read from the whole formats -- the KERN level is in the binary:
 *   "\x016[FTS][Info]Create proc entry success!\n"@0xffffff800924db58   (KERN_INFO)
 *   "\x013[FTS][Error]create proc entry fail\n"@0xffffff800924db32      (KERN_ERR)
 *   "[FTS]cmd len: %d, buf: %s\n"@0xffffff800924de6e                    (no level)
 */
#define FTS_DEBUG(fmt, args...)		printk("[FTS]" fmt "\n", ##args)
#define FTS_INFO(fmt, args...)		printk(KERN_INFO "[FTS][Info]" fmt "\n", ##args)
#define FTS_ERROR(fmt, args...)		printk(KERN_ERR "[FTS][Error]" fmt "\n", ##args)

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7bb44).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define FTS_REG_POWER_MODE		0xa5	/* "Power Mode:0x%02x\n"@0xffffff800924e01c */
#define FTS_REG_FW_VER			0xa6	/* "FW Ver:0x%02x\n"@0xffffff800924e02f */
#define FTS_REG_LIC_VER			0xe4	/* "LCD Initcode Ver:0x%02x\n"@0xffffff800924e03e */
#define FTS_REG_PARAM_VER		0xb5	/* "Param Ver:0x%02x\n"@0xffffff800924e057 */
#define FTS_REG_PARAM_STATUS		0xb6	/* "Param status:0x%02x\n"@0xffffff800924e069 */
#define FTS_REG_VENDOR_ID		0xa8	/* "Vendor ID:0x%02x\n"@0xffffff800924e07e */
#define FTS_REG_LCD_BUSY_NUM		0xab	/* "LCD Busy Number:0x%02x\n"@0xffffff800924e090 */
#define FTS_REG_GESTURE_EN		0xd0	/* "Gesture Mode:0x%02x\n"@0xffffff800924e0a8 */
#define FTS_REG_CHARGER_MODE_EN		0x8b	/* "charge stat:0x%02x\n"@0xffffff800924e0bd */
#define FTS_REG_INT_CNT			0x8f	/* "INT count:0x%02x\n"@0xffffff800924e0d1 */
#define FTS_REG_ESD_SATURATE		0x91	/* "ESD count:0x%02x\n"@0xffffff800924e0e3 */

/*
 * "Focaltech V2.2 20180321"@0xffffff800924e1c9, the second argument of the
 * snprintf in fts_driverinfo_show: "91072463 add"@0xffffff8008a7ca90 puts
 * 0x924e000 + 0x1c9 into x3.  It is the SAME string tpd_driver_init prints
 * from unit C.
 */
#define FTS_DRIVER_VERSION		"Focaltech V2.2 20180321"

/*
 * "320023e1 orr"@0xffffff8008a7b620 -> w1 = 0x1ff = 0777, the mode of the
 * /proc node.  It is NOT 0644 like the eight device_attribute entries: the binary
 * tells them apart.
 */
#define FTS_PROC_MODE			0777

/*
 * THE SIZE IS MEASURED, THE NAME IS A CHOICE (rule 4).
 * 256 is the bound of the range check in both halves of the
 * /proc node -- "f104013f cmp"@0xffffff8008a7b72c on read and
 * "f104013f cmp"@0xffffff8008a7b8d4 on write -- and it is also the size
 * of the buffer on the stack: sixteen "stp xzr,xzr" for 256 bytes on read
 * (from 0xffffff8008a7b734 to 0xffffff8008a7b770).
 */
#define PROC_BUF_SIZE			256

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7bb2c, 384 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct fts_ts_data {
	struct i2c_client *c0;
	struct input_dev *c8;
	u8 c16[16];		/* not touched by this unit */
	u8 c32;
	u8 c33;
	u8 c34[286];		/* not touched by this unit */
	int c320;
	u8 c324;		/* not touched by this unit */
	u8 c325;
	u8 c326[42];		/* not touched by this unit */
	struct proc_dir_entry *c368;
	u8 c376;
	u8 c377[7];		/* up to 384 = the size measured by unit C */
};

/*
 * 0xffffff800a100ad8, read by EVERY function of this unit with
 * "f9456d08 ldr"@0xffffff8008a7bb28 (ldr x8,[x8,#2776]).  It is defined
 * by unit C, which writes it in tpd_probe; here it is `extern`, and the
 * non-static there is PROVEN by the fact that nineteen functions spread over
 * four translation units read it.
 */
extern struct fts_ts_data *fts_data;

/*
 * ==========================================================================
 * THE BOUNDARY WITH THE OTHER UNITS -- DECLARED, NOT DEFINED HERE
 * ==========================================================================
 * The signatures come from the registers at the call sites.
 */
int fts_i2c_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue);
					/* unita' E, 0xffffff8008a80614 */
int fts_i2c_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue);
					/* unita' E, 0xffffff8008a805c4 */
int fts_i2c_read(struct i2c_client *client, char *writebuf, int writelen,
		 char *readbuf, int readlen);
					/* unita' E, 0xffffff8008a802ac */
int fts_i2c_write(struct i2c_client *client, char *writebuf, int writelen);
					/* unita' E, 0xffffff8008a80494 */
void fts_irq_disable(void);		/* unita' C, 0xffffff8008a7ecf0 */
void fts_irq_enable(void);		/* unita' C, 0xffffff8008a7ed74 */
int fts_reset_proc(int hdelayms);	/* unita' C, 0xffffff8008a7ec74 */
int fts_upgrade_bin(struct i2c_client *client, char *fw_name, bool force);
					/* unit B, 0xffffff8008a7e464 -- NOT WRITTEN */

/*
 * ==========================================================================
 * THE FOUR FUNCTIONS OF BATCH A4 -- DECLARED HERE, DEFINED FURTHER DOWN
 * ==========================================================================
 * They are needed ahead of time because the data tables (the device_attribute
 * entries and the file_operations of the /proc node) name them before they are
 * defined. They are `static`, as in the factory map (`t`): the previous batch
 * had to declare them non-`static` because they were not defined, and that was
 * its DIVERGENCE 1.  Now they are defined, and the divergence is closed.
 */
static ssize_t fts_debug_read(struct file *filp, char __user *buff,
			      size_t count, loff_t *ppos);
							/* 0xffffff8008a7b708 */
static ssize_t fts_debug_write(struct file *filp, const char __user *buff,
			       size_t count, loff_t *ppos);
							/* 0xffffff8008a7b8a8 */
static ssize_t fts_tprwreg_show(struct device *dev,
				struct device_attribute *attr, char *buf);
							/* 0xffffff8008a7bbf8 */
static ssize_t fts_tprwreg_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count);
							/* 0xffffff8008a7beec */

/*
 * Forward declarations: the data tables come before the functions,
 * as in the factory `.data`.
 */
static ssize_t fts_tpfwver_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t fts_tpfwver_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count);
static ssize_t fts_dumpreg_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t fts_dumpreg_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count);
static ssize_t fts_fwupgradebin_show(struct device *dev,
				     struct device_attribute *attr, char *buf);
static ssize_t fts_fwupgradebin_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count);
static ssize_t fts_fwforceupg_show(struct device *dev,
				   struct device_attribute *attr, char *buf);
static ssize_t fts_fwforceupg_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count);
static ssize_t fts_driverinfo_show(struct device *dev,
				   struct device_attribute *attr, char *buf);
static ssize_t fts_driverinfo_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count);
static ssize_t fts_hw_reset_show(struct device *dev,
				 struct device_attribute *attr, char *buf);
static ssize_t fts_hw_reset_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count);
static ssize_t fts_irq_show(struct device *dev,
			    struct device_attribute *attr, char *buf);
static ssize_t fts_irq_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count);

/*
 * This section was reconstructed from the factory kernel disassembly (32 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800a100a98).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
u8 fts_g_a100a98;
int fts_g_a100aa0;
int fts_g_a100aa8;
int fts_g_a100ab0;
int fts_g_a100ab8;
char *fts_g_a100ac0;

static DEVICE_ATTR(fts_fw_version, 0644, fts_tpfwver_show, fts_tpfwver_store);
static DEVICE_ATTR(fts_rw_reg, 0644, fts_tprwreg_show, fts_tprwreg_store);
static DEVICE_ATTR(fts_dump_reg, 0644, fts_dumpreg_show, fts_dumpreg_store);
static DEVICE_ATTR(fts_upgrade_bin, 0644, fts_fwupgradebin_show, fts_fwupgradebin_store);
static DEVICE_ATTR(fts_force_upgrade, 0644, fts_fwforceupg_show, fts_fwforceupg_store);
static DEVICE_ATTR(fts_driver_info, 0644, fts_driverinfo_show, fts_driverinfo_store);
static DEVICE_ATTR(fts_hw_reset, 0644, fts_hw_reset_show, fts_hw_reset_store);
static DEVICE_ATTR(fts_irq, 0644, fts_irq_show, fts_irq_store);

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998c778).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct attribute *fts_g_998c778[] = {
	&dev_attr_fts_fw_version.attr,
	&dev_attr_fts_rw_reg.attr,
	&dev_attr_fts_dump_reg.attr,
	&dev_attr_fts_upgrade_bin.attr,
	&dev_attr_fts_force_upgrade.attr,
	&dev_attr_fts_driver_info.attr,
	&dev_attr_fts_hw_reset.attr,
	&dev_attr_fts_irq.attr,
	NULL,
};

static struct attribute_group fts_g_998c750 = {
	.attrs = fts_g_998c778,
};

/*
 * THE /proc NODE, at 0xffffff8008f7f370.
 *   +0x10: relocated, addend 0xffffff8008a7b708   .read  = fts_debug_read
 *   +0x18: relocated, addend 0xffffff8008a7b8a8   .write = fts_debug_write
 *   every other slot: not relocated, that is NULL -- in particular there is NO
 *   `.owner` (+0), which THIS_MODULE would fill in.
 * It lives in `.rodata` (0x8f7f370 is in the same run as the block's
 * `of_device_id`s, 0x8f7f460 and 0x8f7f5f0), so it is `const`.
 */
static const struct file_operations fts_g_8f7f370 = {
	.read = fts_debug_read,
	.write = fts_debug_write,
};

/*
 * fts_create_apk_debug_channel() was reconstructed from the factory kernel disassembly (0xffffff8008a7b600, 100 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_create_apk_debug_channel(struct fts_ts_data *ts_data)
{
	ts_data->c368 = proc_create("ftxxxx-debug", FTS_PROC_MODE, NULL,
				    &fts_g_8f7f370);
	if (NULL == ts_data->c368) {
		FTS_ERROR("create proc entry fail");
		return -ENOMEM;
	}

	FTS_INFO("Create proc entry success!");

	return 0;
}

/*
 * fts_release_apk_debug_channel() was reconstructed from the factory kernel disassembly (0xffffff8008a7b664, 28 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void fts_release_apk_debug_channel(struct fts_ts_data *ts_data)
{
	if (ts_data->c368)
		proc_remove(ts_data->c368);
}

/*
 * fts_create_sysfs() was reconstructed from the factory kernel disassembly (0xffffff8008a7b680, 100 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_create_sysfs(struct i2c_client *client)
{
	int ret = 0;

	ret = sysfs_create_group(&client->dev.kobj, &fts_g_998c750);
	if (ret != 0) {
		FTS_ERROR("[EX]: sysfs_create_group() failed!!");
		sysfs_remove_group(&client->dev.kobj, &fts_g_998c750);
		return -ENOMEM;
	}

	FTS_INFO("[EX]: sysfs_create_group() succeeded!!");

	return 0;
}

/*
 * ===========================================================================
 * fts_remove_sysfs @ 0xffffff8008a7b6e4, 36 bytes, visibility T
 * ===========================================================================
 *   "9100c000 add"@0xffffff8008a7b6f0   x0 = client + 0x30
 *   "97e221fe bl"@0xffffff8008a7b6f8    sysfs_remove_group
 *   "2a1f03e0 mov"@0xffffff8008a7b6fc   w0 = 0, that is `return 0`
 */
int fts_remove_sysfs(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &fts_g_998c750);

	return 0;
}

/*
 * fts_debug_read() was reconstructed from the factory kernel disassembly (0xffffff8008a7b708, 416 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_debug_read(struct file *filp, char __user *buff,
			      size_t count, loff_t *ppos)
{
	int ret = 0;
	int num_read_chars = 0;
	int buflen = count;
	char buf[PROC_BUF_SIZE] = { 0 };
	struct fts_ts_data *ts_data = fts_data;
	struct i2c_client *client = ts_data->c0;

	if ((count == 0) || (count > PROC_BUF_SIZE)) {
		FTS_ERROR("apk proc read count(%d) fail", (int)count);
		return -EINVAL;
	}

	switch (ts_data->c376) {
	case 1:
		ret = fts_i2c_read(client, NULL, 0, buf, 1);
		if (ret < 0) {
			FTS_ERROR("[APK]: read iic error!!");
			return ret;
		}
		num_read_chars = 1;
		break;

	case 7:
		ret = fts_i2c_read(client, NULL, 0, buf, buflen);
		if (ret < 0) {
			FTS_ERROR("[APK]: read iic error!!");
			return ret;
		}
		num_read_chars = buflen;
		break;

	default:
		break;
	}

	if (copy_to_user(buff, buf, num_read_chars)) {
		FTS_ERROR("[APK]: copy to user error!!");
		return -EFAULT;
	}

	return num_read_chars;
}

/*
 * fts_debug_write() was reconstructed from the factory kernel disassembly (0xffffff8008a7b8a8, 600 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_debug_write(struct file *filp, const char __user *buff,
			       size_t count, loff_t *ppos)
{
	int buflen = count;
	int writelen = 0;
	int ret = 0;
	char writebuf[PROC_BUF_SIZE] = { 0 };
	char tmpbuf[28];
	struct fts_ts_data *ts_data = fts_data;
	struct i2c_client *client = ts_data->c0;

	if ((count == 0) || (count > PROC_BUF_SIZE)) {
		FTS_ERROR("apk proc wirte count(%d) fail", (int)count);
		return -EINVAL;
	}

	if (copy_from_user(writebuf, buff, count)) {
		FTS_ERROR("[APK]: copy from user error!!");
		return -EFAULT;
	}

	ts_data->c376 = writebuf[0];

	switch (writebuf[0]) {
	case 1:
		writelen = 1;
		ret = fts_i2c_write(client, writebuf + 1, writelen);
		if (ret < 0) {
			FTS_ERROR("[APK]: write iic error!!");
			return ret;
		}
		break;

	case 2:
		writelen = 2;
		ret = fts_i2c_write(client, writebuf + 1, writelen);
		if (ret < 0) {
			FTS_ERROR("[APK]: write iic error!!");
			return ret;
		}
		break;

	case 6:
	case 7:
		writelen = buflen - 1;
		if (writelen > 0) {
			ret = fts_i2c_write(client, writebuf + 1, writelen);
			if (ret < 0) {
				FTS_ERROR("[APK]: write iic error!!");
				return ret;
			}
		}
		break;

	case 8:
		FTS_INFO("[APK]: PROC_SET_TEST_FLAG = %x!!", writebuf[1]);
		break;

	case 11:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wfortify-source"
		snprintf(tmpbuf, PAGE_SIZE, "%s", writebuf + 1);
#pragma clang diagnostic pop
		tmpbuf[buflen - 1] = '\0';
		if (memcmp(tmpbuf, "focal_driver", 12) == 0) {
			FTS_INFO("APK execute HW Reset");
			fts_reset_proc(1);
		}
		break;

	default:
		break;
	}

	return count;
}

/*
 * fts_tpfwver_show() was reconstructed from the factory kernel disassembly (0xffffff8008a7bb00, 240 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_tpfwver_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = fts_data->c0;
	struct input_dev *input_dev = fts_data->c8;
	ssize_t num_read_chars = 0;
	u8 fwver = 0;

	mutex_lock(&input_dev->mutex);

	if (fts_i2c_read_reg(client, FTS_REG_FW_VER, &fwver) < 0)
		sprintf(buf, "I2c transfer error!\n");

	if ((fwver == 0xFF) || (fwver == 0x00))
		num_read_chars = snprintf(buf, PAGE_SIZE, "get tp fw version fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "%02x\n", fwver);

	mutex_unlock(&input_dev->mutex);

	return num_read_chars;
}

/*
 * fts_tpfwver_store() was reconstructed from the factory kernel disassembly (0xffffff8008a7bbf0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_tpfwver_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	return -1;
}

/*
 * fts_tprwreg_show() was reconstructed from the factory kernel disassembly (0xffffff8008a7bbf8, 756 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_tprwreg_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct input_dev *input_dev = fts_data->c8;
	int count;
	int i;

	mutex_lock(&input_dev->mutex);

	if (fts_g_a100aa8 < 0) {
		count = snprintf(buf, PAGE_SIZE, "Invalid cmd line\n");
	} else if (fts_g_a100aa8 == 1) {
		if (fts_g_a100a98 & 0x01) {
			if (fts_g_a100ab8)
				count = snprintf(buf, PAGE_SIZE,
						 "Write %02X failed, ret: %d\n",
						 fts_g_a100aa0, fts_g_a100ab8);
			else
				count = snprintf(buf, PAGE_SIZE,
						 "Write %02X, %02X success\n",
						 fts_g_a100aa0, fts_g_a100ab0);
		} else {
			if (fts_g_a100ab8)
				count = snprintf(buf, PAGE_SIZE,
						 "Read %02X failed, ret: %d\n",
						 fts_g_a100aa0, fts_g_a100ab8);
			else
				count = snprintf(buf, PAGE_SIZE,
						 "Read %02X: %02X\n",
						 fts_g_a100aa0, fts_g_a100ab0);
		}
	} else {
		if (fts_g_a100a98 & 0x01) {
			count = snprintf(buf, PAGE_SIZE,
					 "Write Reg: [%02X]-[%02X]\n",
					 fts_g_a100aa0,
					 fts_g_a100aa0 + fts_g_a100aa8 - 1);
			count += snprintf(buf + count, PAGE_SIZE, "Write Data: ");
			if (fts_g_a100ac0) {
				for (i = 1; i < fts_g_a100aa8; i++)
					count += snprintf(buf + count, PAGE_SIZE,
							  "%02X ",
							  fts_g_a100ac0[i]);
				count += snprintf(buf + count, PAGE_SIZE, "\n");
			}
			if (fts_g_a100ab8)
				count += snprintf(buf + count, PAGE_SIZE,
						  "Result: failed, ret: %d\n",
						  fts_g_a100ab8);
			else
				count += snprintf(buf + count, PAGE_SIZE,
						  "Result: success\n");
		} else {
			count = snprintf(buf, PAGE_SIZE,
					 "Read Reg: [%02X]-[%02X]\n",
					 fts_g_a100aa0,
					 fts_g_a100aa0 + fts_g_a100aa8);
			count += snprintf(buf + count, PAGE_SIZE, "Result: ");
			if (fts_g_a100ab8) {
				count += snprintf(buf + count, PAGE_SIZE,
						  "failed, ret: %d\n",
						  fts_g_a100ab8);
			} else {
				if (fts_g_a100ac0) {
					for (i = 0; i < fts_g_a100aa8; i++)
						count += snprintf(buf + count,
								  PAGE_SIZE,
								  "%02X ",
								  fts_g_a100ac0[i]);
					count += snprintf(buf + count, PAGE_SIZE,
							  "\n");
				}
			}
		}
	}

	mutex_unlock(&input_dev->mutex);

	return count;
}

/*
 * fts_hex_ubyte() was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static u8 fts_hex_ubyte(const char *s)
{
	u8 hi;
	u8 lo;
	char c;

	c = s[1];
	if ((c >= '0') && (c <= '9'))
		lo = c - '0';
	else if ((c >= 'a') && (c <= 'z'))
		lo = c - 'a' + 10;
	else if ((c >= 'A') && (c <= 'Z'))
		lo = c - 'A' + 10;
	else
		return -EINVAL;

	c = s[0];
	if ((c >= '0') && (c <= '9'))
		hi = c - '0';
	else if ((c >= 'a') && (c <= 'z'))
		hi = c - 'a' + 10;
	else if ((c >= 'A') && (c <= 'Z'))
		hi = c - 'A' + 10;
	else
		return -EINVAL;

	return lo + (hi << 4);
}

static int fts_hex_byte(const char *s)
{
	int hi;
	int lo;
	char c;

	c = s[1];
	if ((c >= '0') && (c <= '9'))
		lo = c - '0';
	else if ((c >= 'a') && (c <= 'z'))
		lo = c - 'a' + 10;
	else if ((c >= 'A') && (c <= 'Z'))
		lo = c - 'A' + 10;
	else
		return -EINVAL;

	c = s[0];
	if ((c >= '0') && (c <= '9'))
		hi = c - '0';
	else if ((c >= 'a') && (c <= 'z'))
		hi = c - 'a' + 10;
	else if ((c >= 'A') && (c <= 'Z'))
		hi = c - 'A' + 10;
	else
		return -EINVAL;

	return lo + (hi << 4);
}

/*
 * fts_tprwreg_store() was reconstructed from the factory kernel disassembly (0xffffff8008a7beec, 1736 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_tprwreg_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct input_dev *input_dev = fts_data->c8;
	ssize_t cmdlen = count - 1;
	char *rw_buf = NULL;
	char regaddr;
	u8 regvalue;
	int len = 0;
	int num = 0;
	int ret = 0;
	int i = 0;

	mutex_lock(&input_dev->mutex);

	if (fts_g_a100ac0) {
		kfree(fts_g_a100ac0);
		fts_g_a100ac0 = NULL;
	}

	FTS_DEBUG("cmd len: %d, buf: %s", (int)cmdlen, buf);

	if (count == 3) {
		fts_g_a100a98 = 0;
		fts_g_a100aa8 = 1;
		fts_g_a100aa0 = fts_hex_byte(&buf[0]);
	} else if (count == 5) {
		fts_g_a100a98 = 1;
		fts_g_a100aa8 = 1;
		fts_g_a100aa0 = fts_hex_byte(&buf[0]);
		fts_g_a100ab0 = fts_hex_byte(&buf[2]);
	} else {
		if (cmdlen <= 4) {
			FTS_ERROR("Invalid cmd buffer");
			mutex_unlock(&input_dev->mutex);
			return -EINVAL;
		}

		regaddr = fts_hex_byte(&buf[1]);
		len = fts_hex_byte(&buf[3]);
		fts_g_a100aa0 = regaddr;

		if (buf[0] == '1') {
			fts_g_a100aa8 = len;
			fts_g_a100a98 = 0;
			FTS_DEBUG("read %02X, %d bytes", regaddr, len);
		} else {
			if ((count - 1) < (len * 2 + 5)) {
				printk(KERN_ERR "data invalided!\n");
				num = -EINVAL;
				goto setlen;
			}
			FTS_DEBUG("write %02X, %d bytes", regaddr, len);
			fts_g_a100a98 = 1;
			fts_g_a100aa8 = len + 1;
		}

		num = fts_g_a100aa8;
		if (num > 0) {
			rw_buf = kzalloc(num, GFP_KERNEL);
			if (NULL == rw_buf) {
				FTS_ERROR("allocate memory failed!\n");
				num = -ENOMEM;
				goto setlen;
			}

			if (fts_g_a100a98 & 0x01) {
				rw_buf[0] = fts_g_a100aa0;
				FTS_DEBUG("write buffer: ");
				for (i = 1; i < fts_g_a100aa8; i++) {
					rw_buf[i] = fts_hex_ubyte(&buf[i * 2 + 3]);
					FTS_DEBUG("buf[%d]: %02X", i, rw_buf[i]);
				}
			}

			num = fts_g_a100aa8;
			fts_g_a100ac0 = rw_buf;
		}
setlen:
		fts_g_a100aa8 = num;
	}

	if (fts_g_a100aa8 < 0) {
		FTS_ERROR("cmd buffer error!");
	} else if (fts_g_a100a98 & 0x01) {
		if (fts_g_a100aa8 == 1)
			ret = fts_i2c_write_reg(client, fts_g_a100aa0,
						fts_g_a100ab0);
		else
			ret = fts_i2c_write(client, fts_g_a100ac0,
					    fts_g_a100aa8);
		fts_g_a100ab8 = ret;

		if (ret < 0) {
			FTS_ERROR("Could not write 0x%02x", fts_g_a100aa0);
		} else {
			FTS_INFO("Write 0x%02x, %d bytes successful",
				 fts_g_a100ab0, fts_g_a100aa8);
			fts_g_a100ab8 = 0;
		}
	} else {
		if (fts_g_a100aa8 == 1) {
			ret = fts_i2c_read_reg(client, fts_g_a100aa0, &regvalue);
			fts_g_a100ab8 = ret;
			fts_g_a100ab0 = regvalue;
		} else {
			regaddr = fts_g_a100aa0;
			ret = fts_i2c_read(client, &regaddr, 1,
					   fts_g_a100ac0, fts_g_a100aa8);
			fts_g_a100ab8 = ret;
		}

		if (ret < 0) {
			FTS_ERROR("Could not read 0x%02x", fts_g_a100aa0);
		} else {
			FTS_INFO("read 0x%02x, %d bytes successful",
				 fts_g_a100aa0, fts_g_a100aa8);
			fts_g_a100ab8 = 0;
		}
	}

	mutex_unlock(&input_dev->mutex);

	return count;
}

/*
 * fts_dumpreg_show() was reconstructed from the factory kernel disassembly (0xffffff8008a7c5b4, 612 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_dumpreg_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct input_dev *input_dev = fts_data->c8;
	int count = 0;
	u8 val = 0;

	mutex_lock(&input_dev->mutex);

	fts_i2c_read_reg(client, FTS_REG_POWER_MODE, &val);
	count = snprintf(buf, PAGE_SIZE, "Power Mode:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_FW_VER, &val);
	count += snprintf(buf + count, PAGE_SIZE, "FW Ver:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_LIC_VER, &val);
	count += snprintf(buf + count, PAGE_SIZE, "LCD Initcode Ver:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_PARAM_VER, &val);
	count += snprintf(buf + count, PAGE_SIZE, "Param Ver:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_PARAM_STATUS, &val);
	count += snprintf(buf + count, PAGE_SIZE, "Param status:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_VENDOR_ID, &val);
	count += snprintf(buf + count, PAGE_SIZE, "Vendor ID:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_LCD_BUSY_NUM, &val);
	count += snprintf(buf + count, PAGE_SIZE, "LCD Busy Number:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_GESTURE_EN, &val);
	count += snprintf(buf + count, PAGE_SIZE, "Gesture Mode:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_CHARGER_MODE_EN, &val);
	count += snprintf(buf + count, PAGE_SIZE, "charge stat:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_INT_CNT, &val);
	count += snprintf(buf + count, PAGE_SIZE, "INT count:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_ESD_SATURATE, &val);
	count += snprintf(buf + count, PAGE_SIZE, "ESD count:0x%02x\n", val);

	mutex_unlock(&input_dev->mutex);

	return count;
}

static ssize_t fts_dumpreg_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	return -1;
}

static ssize_t fts_fwupgradebin_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	return -1;
}

/*
 * fts_fwupgradebin_store() was reconstructed from the factory kernel disassembly (0xffffff8008a7c828, 276 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_fwupgradebin_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	char fwname[128] = { 0 };
	struct fts_ts_data *ts_data = fts_data;
	struct i2c_client *client = ts_data->c0;
	struct input_dev *input_dev = ts_data->c8;

	if ((count <= 1) || (count >= 128 - 32)) {
		FTS_ERROR("fw bin name's length(%d) fail", (int)count);
		return -EINVAL;
	}

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7c890, 128 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
#pragma clang diagnostic push
/*
 * THE -Wfortify-source GROUP DOES NOT EXIST IN THE FACTORY COMPILER (clang
 * 9.0.3, r353983c): there, the line below is itself an error, because the
 * kernel carries -Werror=unknown-warning-option as standard.  Turning that
 * group off FIRST makes the two lines harmless for clang 9 and effective for
 * clang 11, with no version numbers in the source.  Measured: without this
 * line `compila-a4.sh fab` gave
 *   error: unknown warning group '-Wfortify-source', ignored
 *          [-Werror,-Wunknown-warning-option]
 * and the file did NOT compile with the compiler that acts as judge.
 */
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wfortify-source"
	snprintf(fwname, PAGE_SIZE, "%s", buf);
#pragma clang diagnostic pop
	fwname[count - 1] = '\0';

	FTS_INFO("upgrade with bin file through sysfs node");

	mutex_lock(&input_dev->mutex);
	ts_data->c325 = 1;
	fts_irq_disable();
	fts_upgrade_bin(client, fwname, 0);
	fts_irq_enable();
	ts_data->c325 = 0;
	mutex_unlock(&input_dev->mutex);

	return count;
}

static ssize_t fts_fwforceupg_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	return -1;
}

static ssize_t fts_fwforceupg_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	char fwname[128] = { 0 };
	struct fts_ts_data *ts_data = fts_data;
	struct i2c_client *client = ts_data->c0;
	struct input_dev *input_dev = ts_data->c8;

	if ((count <= 1) || (count >= 128 - 32)) {
		FTS_ERROR("fw bin name's length(%d) fail", (int)count);
		return -EINVAL;
	}

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7c890, 128 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
#pragma clang diagnostic push
/*
 * THE -Wfortify-source GROUP DOES NOT EXIST IN THE FACTORY COMPILER (clang
 * 9.0.3, r353983c): there, the line below is itself an error, because the
 * kernel carries -Werror=unknown-warning-option as standard.  Turning that
 * group off FIRST makes the two lines harmless for clang 9 and effective for
 * clang 11, with no version numbers in the source.  Measured: without this
 * line `compila-a4.sh fab` gave
 *   error: unknown warning group '-Wfortify-source', ignored
 *          [-Werror,-Wunknown-warning-option]
 * and the file did NOT compile with the compiler that acts as judge.
 */
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wfortify-source"
	snprintf(fwname, PAGE_SIZE, "%s", buf);
#pragma clang diagnostic pop
	fwname[count - 1] = '\0';

	FTS_INFO("force upgrade through sysfs node");

	mutex_lock(&input_dev->mutex);
	ts_data->c325 = 1;
	fts_irq_disable();
	fts_upgrade_bin(client, fwname, 1);
	fts_irq_enable();
	ts_data->c325 = 0;
	mutex_unlock(&input_dev->mutex);

	return count;
}

/*
 * fts_driverinfo_show() was reconstructed from the factory kernel disassembly (0xffffff8008a7ca58, 244 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_driverinfo_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct fts_ts_data *ts_data = fts_data;
	struct input_dev *input_dev = ts_data->c8;
	int count = 0;

	mutex_lock(&input_dev->mutex);

	count = snprintf(buf, PAGE_SIZE, "Driver Ver:%s\n", FTS_DRIVER_VERSION);

	count += snprintf(buf + count, PAGE_SIZE, "Resolution:(%d,%d)~(%d,%d)\n",
			  0, 0, (int)TPD_RES_X, (int)TPD_RES_Y);

	count += snprintf(buf + count, PAGE_SIZE, "Max Touchs:%d\n",
			  tpd_dts_data.touch_max_num);

	count += snprintf(buf + count, PAGE_SIZE, "irq:%d\n", ts_data->c320);

	count += snprintf(buf + count, PAGE_SIZE, "IC ID:0x%02x%02x\n",
			  ts_data->c32, ts_data->c33);

	mutex_unlock(&input_dev->mutex);

	return count;
}

static ssize_t fts_driverinfo_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return -1;
}

/*
 * fts_hw_reset_show() was reconstructed from the factory kernel disassembly (0xffffff8008a7cb54, 100 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_hw_reset_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct input_dev *input_dev = fts_data->c8;
	ssize_t count = 0;

	mutex_lock(&input_dev->mutex);
	fts_reset_proc(1);
	count = snprintf(buf, PAGE_SIZE, "hw reset executed\n");
	mutex_unlock(&input_dev->mutex);

	return count;
}

static ssize_t fts_hw_reset_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	return -1;
}

static ssize_t fts_irq_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	return -1;
}

/*
 * fts_irq_store() was reconstructed from the factory kernel disassembly (0xffffff8008a7cbc8, 132 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t fts_irq_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct input_dev *input_dev = fts_data->c8;

	mutex_lock(&input_dev->mutex);

	if (buf[0] == '1') {
		FTS_INFO("[EX-FUN]enable irq");
		fts_irq_enable();
	} else if (buf[0] == '0') {
		FTS_INFO("[EX-FUN]disable irq");
		fts_irq_disable();
	}

	mutex_unlock(&input_dev->mutex);

	return count;
}

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_sysfs.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
