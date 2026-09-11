/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "["KBUILD_MODNAME"] " fmt
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/kfifo.h>

#include <linux/firmware.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/printk.h>

#include <asm/setup.h>
#include "mtk_chip_common.h"

struct mt_chip_drv g_chip_drv = {
	.info_bit_mask = CHIP_INFO_BIT(CHIP_INFO_ALL)
};

struct mt_chip_drv *get_mt_chip_drv(void)
{
	return &g_chip_drv;
}

/*
 * wtk_create_proc_mem_info() was reconstructed from the factory kernel disassembly (0xffffff80087a24ec, 68 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_chip_mtk_chip_common.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
extern char wtk_mem_name[100];

static int wtk_mem_info_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", wtk_mem_name);
	return 0;
}

static int wtk_mem_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, wtk_mem_info_show, NULL);
}

static const struct file_operations wtk_mem_info_fops = {
	.open = wtk_mem_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

void wtk_create_proc_mem_info(void)
{
	if (!proc_create("wtk_memInfo", 0444, NULL, &wtk_mem_info_fops))
		printk("create /proc/mem_info_entry fail\n");
}

struct chip_inf_entry {
	const char *name;
	unsigned int id;
	int (*to_str)(char *buf, size_t len, int val);
};

static int hex2str(char *buf, size_t len, int val)
{
	return snprintf(buf, len, "%04X", val);
}

static int dec2str(char *buf, size_t len, int val)
{
	return snprintf(buf, len, "%04d", val);
}

static int date2str(char *buf, size_t len, int val)
{
	unsigned int year = ((val & 0x3C0) >> 6) + 2012;
	unsigned int week = (val & 0x03F);

	return snprintf(buf, len, "%04d%02d", year, week);
}

#define __chip_info(id) \
	((g_chip_drv.get_chip_info) ? (g_chip_drv.get_chip_info(id)) : (0x0000))

static struct proc_dir_entry *chip_proc;
static struct chip_inf_entry chip_ent[] = {
	{"hw_code", CHIP_INFO_HW_CODE, hex2str},
	{"hw_subcode", CHIP_INFO_HW_SUBCODE, hex2str},
	{"hw_ver", CHIP_INFO_HW_VER, hex2str},
	{"sw_ver", CHIP_INFO_SW_VER, hex2str},
	{"code_func", CHIP_INFO_FUNCTION_CODE, hex2str},
	{"code_date", CHIP_INFO_DATE_CODE, date2str},
	{"code_proj", CHIP_INFO_PROJECT_CODE, dec2str},
	{"code_fab", CHIP_INFO_FAB_CODE, hex2str},
	{"wafer_big_ver", CHIP_INFO_WAFER_BIG_VER, hex2str},
	{"info", CHIP_INFO_ALL, NULL}
};

static int chip_proc_show(struct seq_file *s, void *v)
{
	struct chip_inf_entry *ent = s->private;

	if ((ent->id > CHIP_INFO_NONE) && (ent->id < CHIP_INFO_MAX)) {
		seq_printf(s, "%04X\n", __chip_info(ent->id));
	} else {
		int idx = 0;
		char buf[16];

		for (idx = 0; idx < ARRAY_SIZE(chip_ent); idx++) {
			struct chip_inf_entry *ent = &chip_ent[idx];
			unsigned int val = __chip_info(ent->id);

			if (!CHIP_INFO_SUP(g_chip_drv.info_bit_mask, ent->id))
				continue;
			else if (!ent->to_str)
				continue;
			else if (ent->to_str(buf, sizeof(buf), val) > 0)
				seq_printf(s, "%-16s:%s (%04x)\n",
					ent->name, buf, val);
			else
				seq_printf(s, "%-16s:%s (%04x)\n",
					ent->name, "NULL", val);
		}
		seq_printf(s, "%-16s:%04X %04X %04X %04X\n", "reg",
			   __chip_info(CHIP_INFO_REG_HW_CODE),
			   __chip_info(CHIP_INFO_REG_HW_SUBCODE),
			   __chip_info(CHIP_INFO_REG_HW_VER),
			   __chip_info(CHIP_INFO_REG_SW_VER));
	}
	return 0;
}

static int chip_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, chip_proc_show, PDE_DATA(file_inode(file)));
}

static const struct file_operations chip_proc_fops = {
	.open = chip_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static void __init create_procfs(void)
{
	int idx;

	chip_proc = proc_mkdir_data("chip", 0, NULL, NULL);
	if (chip_proc == NULL) {
		pr_err("create /proc/chip fails\n");
		return;
	}

	pr_debug("create /proc/chip(%x)\n", g_chip_drv.info_bit_mask);

	for (idx = 0; idx < ARRAY_SIZE(chip_ent); idx++) {
		struct chip_inf_entry *ent = &chip_ent[idx];

		if (!CHIP_INFO_SUP(g_chip_drv.info_bit_mask, ent->id))
			continue;
		if (proc_create_data(ent->name, 0444, chip_proc,
			&chip_proc_fops, ent) == NULL) {
			pr_err("create /proc/chip/%s fail\n", ent->name);
			return;
		}
	}
}

/*
 * init_wtk_mem_name() was reconstructed from the factory kernel disassembly (0xffffff80093769a8, 56 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_chip_mtk_chip_common.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
char wtk_mem_name[100];

static int __init init_wtk_mem_name(char *str)
{
	strncpy(wtk_mem_name, str, 99);
	wtk_mem_name[99] = 0;

	return 1;
}

__setup("wtk_memname=", init_wtk_mem_name);

static int __init chip_common_init(void)
{
	/*
	 * BEFORE create_procfs: in the binary "97bdfa92 bl"@0xffffff8009376a0c
	 * (proc_create, inlined by wtk_create_proc_mem_info) precedes
	 * "97bdf98d bl"@0xffffff8009376a34 (the proc_mkdir_data of /proc/chip).
	 */
	wtk_create_proc_mem_info();
	create_procfs();
	return 0;
}

arch_initcall(chip_common_init);
MODULE_DESCRIPTION("MTK Chip Common");
MODULE_LICENSE("GPL");
