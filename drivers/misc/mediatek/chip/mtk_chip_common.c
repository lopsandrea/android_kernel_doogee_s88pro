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
 * IL NODO /proc/wtk_memInfo -- AGGIUNTA DI FABBRICA, tre funzioni contigue
 * subito dopo get_mt_chip_drv nell'immagine:
 *   wtk_create_proc_mem_info @0xffffff80087a24ec, 68 byte
 *   wtk_mem_info_open        @0xffffff80087a2530, 36 byte
 *   wtk_mem_info_show        @0xffffff80087a2554, 40 byte
 *
 * Il nome del nodo e il messaggio d'errore NON coincidono, ed e' cosi' nel
 * binario: il nodo si chiama "wtk_memInfo"@0xffffff80091c83c2 e il messaggio
 * dice "create /proc/mem_info_entry fail\n"@0xffffff80091c83cf. Difetto di
 * fabbrica, riprodotto.
 *
 * I permessi sono 0444: "52802481 mov"@0xffffff80087a2504 mette 0x124 in w1,
 * che e' 292, cioe' 0444 in ottale.
 *
 * La struttura delle operazioni sta a 0xffffff8008f55ee0 e ha seq_lseek a +8
 * e seq_read a +16 (readelf -r: gli addendi normalizzati danno
 * 0xffffff8008297dd8 e 0xffffff8008297730), cioe' e' la forma standard di
 * single_open.
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
 * init_wtk_mem_name @0xffffff80093769a8, 56 byte, in .init.text -- AGGIUNTA
 * WINGTECH a questo file di ALPS.
 *
 * E' un gestore `__setup` del parametro `wtk_memname=`, e il binario lo dice
 * tutto:
 *
 *   ffffff80093769b4: adrp x19, ...       il buffer a 0xffffff8009d060e8
 *   ffffff80093769b8: add  x19, x19, #0xe8
 *   ffffff80093769c0: mov  w2, #0x63      99 byte al massimo
 *   ffffff80093769c8: bl   <strncpy>
 *   ffffff80093769d0: orr  w0, wzr, #0x1  il ritorno e' 1
 *   ffffff80093769d4: strb wzr, [x19,#99] e il terminatore va a mano
 *
 * IL BUFFER E' DI CENTO BYTE, non di novantanove: `strncpy` ne copia 99 e il
 * centesimo e' lo zero scritto a mano -- che e' anche la prova che la
 * sorgente NON si fida di `strncpy` per terminare, ed e' giusto: se `str` e'
 * piu' lungo di 99 byte `strncpy` non termina.
 *
 * L'UNICO LETTORE, in tutta l'immagine, e' `wtk_mem_info_show` poche righe
 * piu' su -- 0xffffff80087a2554, che legge lo stesso 0xffffff8009d060e8. Il
 * parametro finisce in un nodo `/proc` e in nient'altro: senza questa
 * funzione quel nodo mostra una stringa vuota, e nessun'altra cosa cambia.
 *
 * LA CATENA DEL PARAMETRO e' letta con le rilocazioni, non a occhio: la
 * `struct obs_kernel_param` sta a 0xffffff800941dda8 e ha `str` =
 * "wtk_memname=" (0xffffff80094176b2) e `setup_func` = questa.
 *
 * IL NOME `wtk_mem_name` E' SCELTO: la mappa non ha simboli di dato, e il
 * binario quel buffer non lo nomina.
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
	/* PRIMA di create_procfs: nel binario "97bdfa92 bl"@0xffffff8009376a0c
	 * (proc_create, incorporata da wtk_create_proc_mem_info) precede
	 * "97bdf98d bl"@0xffffff8009376a34 (proc_mkdir_data di /proc/chip). */
	wtk_create_proc_mem_info();
	create_procfs();
	return 0;
}

arch_initcall(chip_common_init);
MODULE_DESCRIPTION("MTK Chip Common");
MODULE_LICENSE("GPL");
