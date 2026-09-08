/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <linux/seq_file.h>
#include <linux/power_supply.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/reboot.h>

#include <mt-plat/upmu_common.h>
#include <mach/upmu_sw.h>
#include <mach/upmu_hw.h>
#include <mt-plat/mtk_boot.h>
#include <mt-plat/charger_type.h>
#include <pmic.h>
#include <tcpm.h>

#include "mtk_charger_intf.h"


void __attribute__((weak)) fg_charger_in_handler(void)
{
	pr_notice("%s not defined\n", __func__);
}

struct chg_type_info {
	struct device *dev;
	struct charger_consumer *chg_consumer;
	struct tcpc_device *tcpc_dev;
	struct notifier_block pd_nb;
	bool tcpc_kpoc;
	/* Charger Detection */
	struct mutex chgdet_lock;
	bool chgdet_en;
	atomic_t chgdet_cnt;
	wait_queue_head_t waitq;
	struct task_struct *chgdet_task;
	struct workqueue_struct *pwr_off_wq;
	struct work_struct pwr_off_work;
	struct workqueue_struct *chg_in_wq;
	struct work_struct chg_in_work;
	bool ignore_usb;
	bool plugin;
};

#ifdef CONFIG_FPGA_EARLY_PORTING
/*  FPGA */
int hw_charging_get_charger_type(void)
{
	return STANDARD_HOST;
}

#else

/* EVB / Phone */
static const char * const mtk_chg_type_name[] = {
	"Charger Unknown",
	"Standard USB Host",
	"Charging USB Host",
	"Non-standard Charger",
	"Standard Charger",
	"Apple 2.1A Charger",
	"Apple 1.0A Charger",
	"Apple 0.5A Charger",
	"Wireless Charger",
	/* La DECIMA voce, aggiunta da Wingtech: di fabbrica
	 * "5265766572736520436861726765722e" a 0xffffff80092614f0, e il
	 * confronto e' "7100245f cmp"@0xffffff8008ac26e4 con 9, non con 8. */
	"Reverse Charger",
};

static void dump_charger_name(enum charger_type type)
{
	switch (type) {
	case CHARGER_UNKNOWN:
	case STANDARD_HOST:
	case CHARGING_HOST:
	case NONSTANDARD_CHARGER:
	case STANDARD_CHARGER:
	case APPLE_2_1A_CHARGER:
	case APPLE_1_0A_CHARGER:
	case APPLE_0_5A_CHARGER:
	/* I DUE CASI CHE ALPS NON ELENCA. Che ci siano e' misurato: il
	 * confronto di fabbrica e' "7100245f cmp"@0xffffff8008ac26e4, cioe'
	 * con 9; senza questi due casi ALPS confronta con 7. */
	case WIRELESS_CHARGER:
	case REVERSE_CHARGER:
		pr_info("%s: charger type: %d, %s\n", __func__, type,
			mtk_chg_type_name[type]);
		break;
	default:
		pr_info("%s: charger type: %d, Not Defined!!!\n", __func__,
			type);
		break;
	}
}

/* Power Supply */
struct mt_charger {
	struct device *dev;
	struct power_supply_desc chg_desc;
	struct power_supply_config chg_cfg;
	struct power_supply *chg_psy;
	struct power_supply_desc ac_desc;
	struct power_supply_config ac_cfg;
	struct power_supply *ac_psy;
	struct power_supply_desc usb_desc;
	struct power_supply_config usb_cfg;
	struct power_supply *usb_psy;
	/* --- L'AGGIUNTA WINGTECH: 264 byte fra `usb_psy` (+360) e `cti`.
	 * La misura sta nel cappello delle quattro funzioni piu' sotto: di
	 * fabbrica `chg_type` sta a +644 e in ALPS a +380, e 644-380 = 264. --- */
	struct power_supply_desc wls_desc;      /* +368 */
	struct power_supply_config wls_cfg;     /* +448 */
	struct power_supply *wls_psy;           /* +480 */
	int c488;   /* +488: scritto dalla `set` sul caso ONLINE */
	int c492;   /* +492: NON osservato, dedotto dallo spazio */
	int c496;   /* +496: la proprieta' CURRENT_NOW */
	int c500;   /* +500: la proprieta' CURRENT_MAX */
	int c504;   /* +504: la proprieta' CHARGE_ENABLED */
	int c508;   /* +508: NON osservato, dedotto dallo spazio */
	struct power_supply_desc rvs_desc;      /* +512 */
	struct power_supply_config rvs_cfg;     /* +592 */
	struct power_supply *rvs_psy;           /* +624 */
	struct chg_type_info *cti;
	bool chg_online; /* Has charger in or not */
	enum charger_type chg_type;
};

static int mt_charger_online(struct mt_charger *mtk_chg)
{
	int ret = 0;
	int boot_mode = 0;

	if (!mtk_chg->chg_online) {
		boot_mode = get_boot_mode();
		if (boot_mode == KERNEL_POWER_OFF_CHARGING_BOOT ||
		    boot_mode == LOW_POWER_OFF_CHARGING_BOOT) {
			pr_notice("%s: Unplug Charger/USB\n", __func__);
			pr_notice("%s: system_state=%d\n", __func__,
				system_state);
			if (system_state != SYSTEM_POWER_OFF)
				kernel_power_off();
		}
	}

	return ret;
}

/* Power Supply Functions */
static int mt_charger_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct mt_charger *mtk_chg = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		/* FORZATO A 1 SENZA GUARDARE chg_type, e il binario lo dice:
		 * "320003e8 orr"@0xffffff8008ac24ec mette 1 in w8 sul ramo di
		 * POWER_SUPPLY_PROP_ONLINE (psp == 4) e non c'e' nessun
		 * confronto prima. Il commento di ALPS -- "Force to 1 in all
		 * charger type" -- descriveva gia' questo, ma il codice teneva
		 * il controllo: valeva 24 byte. */
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		val->intval = mtk_chg->chg_type;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


static int mt_charger_set_property(struct power_supply *psy,
	enum power_supply_property psp, const union power_supply_propval *val)
{
	struct mt_charger *mtk_chg = power_supply_get_drvdata(psy);
	struct chg_type_info *cti;

	pr_info("%s\n", __func__);

	if (!mtk_chg) {
		pr_notice("%s: no mtk chg data\n", __func__);
		return -EINVAL;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		mtk_chg->chg_online = val->intval;
		mt_charger_online(mtk_chg);
		return 0;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		mtk_chg->chg_type = val->intval;
		break;
	default:
		return -EINVAL;
	}

	dump_charger_name(mtk_chg->chg_type);

	cti = mtk_chg->cti;
	if (!cti->ignore_usb) {
		/* usb */
		if ((mtk_chg->chg_type == STANDARD_HOST) ||
			(mtk_chg->chg_type == CHARGING_HOST) ||
			(mtk_chg->chg_type == NONSTANDARD_CHARGER))
			mt_usb_connect();
		else
			mt_usb_disconnect();
	}

	queue_work(cti->chg_in_wq, &cti->chg_in_work);

	power_supply_changed(mtk_chg->ac_psy);
	power_supply_changed(mtk_chg->usb_psy);
	/* Anche le due power_supply dell'aggiunta Wingtech, wls (+480) e
	 * rvs (+624): la fabbrica le tocca insieme alle altre. */
	power_supply_changed(mtk_chg->wls_psy);
	power_supply_changed(mtk_chg->rvs_psy);

	return 0;
}

static int mt_ac_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct mt_charger *mtk_chg = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = 0;
		/* Force to 1 in all charger type */
		if (mtk_chg->chg_type != CHARGER_UNKNOWN)
			val->intval = 1;
		/* Reset to 0 if charger type is USB -- E ANCHE SE E'
		 * WIRELESS_CHARGER, che e' l'aggiunta del blocco. La maschera
		 * di fabbrica e' "528020c9 mov"@0xffffff8008ac2558, cioe'
		 * 0x106: bit 1 (STANDARD_HOST), bit 2 (CHARGING_HOST) e bit 8
		 * (WIRELESS_CHARGER). ALPS ha solo i primi due. */
		if ((mtk_chg->chg_type == STANDARD_HOST) ||
			(mtk_chg->chg_type == CHARGING_HOST) ||
			(mtk_chg->chg_type == WIRELESS_CHARGER))
			val->intval = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int mt_usb_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct mt_charger *mtk_chg = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if ((mtk_chg->chg_type == STANDARD_HOST) ||
			(mtk_chg->chg_type == CHARGING_HOST))
			val->intval = 1;
		else
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		val->intval = 500000;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		val->intval = 5000000;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


/*
 * ===================================================================
 * L'AGGIUNTA WINGTECH: l'alimentazione WIRELESS e quella INVERSA
 * ===================================================================
 * Quattro funzioni ricostruite dal disassemblato del kernel di fabbrica,
 * piu' i campi che leggono. Non sono in nessun albero pubblico: l'albero ALPS
 * ha questo file ma non queste funzioni. E' lo stesso schema di
 * `mt_charger_set_opa_mode` dentro mt6370_pmu_charger.c e delle quattro
 * funzioni di /proc/wtk_tpInfo dentro mtk_tpd.c.
 *
 *   mt_wls_set_property           0xffffff8008ac260c   336   t
 *   mt_wls_get_property           0xffffff8008ac275c   204   t
 *   mt_wls_property_is_writeable  0xffffff8008ac2828    52   t
 *   mt_rvs_get_property           0xffffff8008ac285c    64   t
 *
 * CHE STIANO IN QUESTO FILE lo dice l'adiacenza: di fabbrica seguono
 * immediatamente `mt_charger_get_property` (0xffffff8008ac24bc),
 * `mt_ac_get_property` (0xffffff8008ac250c) e `mt_usb_get_property`
 * (0xffffff8008ac2580), che questo file gia' contiene.
 *
 * LA DISPOSIZIONE DELLA STRUTTURA E' MISURATA, NON SUPPOSTA. La prova sta in
 * `mt_usb_get_property`, che di fabbrica e' identica alla nostra istruzione
 * per istruzione TRANNE UNA:
 *
 *   fabbrica:  "b9428408 ldr"@0xffffff8008ac25b0   ldr w8, [x0,#644]
 *   ALPS:                                          ldr w8, [x0,#380]
 *
 * E' lo stesso campo, `chg_type`. 644 - 380 = 264: Wingtech ha INSERITO 264
 * byte prima di esso, non aggiunti in coda. Dove, lo dicono gli offset che le
 * funzioni di fabbrica toccano:
 *
 *   +360  usb_psy      invariato (probe, set_property, remove, resume)
 *   +368  wls_desc     probe scrive +368 +376 +384 +400 +416
 *   +448  wls_cfg      probe scrive +456, cioe' `cfg.drv_data`
 *   +480  wls_psy      "f940f260 ldr"@0xffffff8008ac2738 + power_supply_changed
 *   +488  int          "b901ea68 str"@0xffffff8008ac26b4
 *   +496  int          "b901f268 str"@0xffffff8008ac26dc
 *   +500  int          "b901f668 str"@0xffffff8008ac26d0
 *   +504  int          "b901fa68 str"@0xffffff8008ac26a8
 *   +512  rvs_desc     probe scrive +512 +520 +528 +536 +544
 *   +592  rvs_cfg      probe scrive +600, cioe' `cfg.drv_data`
 *   +624  rvs_psy      probe (x3), set_property, remove, resume
 *   +632  cti          era 368 -- "f9413e68 ldr"@0xffffff8008ac2724 (OTTO byte)
 *   +640  chg_online   era 376 -- "390a0268 strb"@0xffffff8008ac26c4 (UN byte)
 *   +644  chg_type     era 380
 *
 * 368 + 120 (wls) + 24 (sei interi) + 120 (rvs) = 632, e da li' cti, chg_online
 * e chg_type ripartono: il conto torna al byte, e i 264 sono esattamente questi.
 * Due dei sei interi (+492 e +508) NON sono osservati da nessuna funzione:
 * sono dedotti dallo spazio, e questo e' dichiarato.
 *
 * I NOMI c488..c508 NON SONO INVENTATI DESCRITTIVI: il binario non nomina
 * questi campi, quindi si chiamano `c<offset>` e il commento dice cio' che di
 * loro e' MISURATO -- quale proprieta' li legge e li scrive.
 */

/*
 * Il prototipo visibile QUI rende `bool`, e non e' una scelta: al sito di
 * chiamata la fabbrica emette "12000108 and"@0xffffff8008ac27fc, cioe' la
 * normalizzazione a un bit di un valore reso da una chiamata che il
 * compilatore non vede. Con `int` quell'istruzione non ci sarebbe.
 *
 * DIVERGENZA DICHIARATA: il nostro `mt5725.c` le definisce `int` (rendendo
 * `x != 0`, che emette lo stesso codice DENTRO mt5725.c ma non qui).
 * Allinearle e' un lotto suo, perche' tocca un file gia' misurato.
 */
extern bool wls_get_online(void);
extern bool rvs_get_online(void);

/*
 * "b9400283 ldr"@0xffffff8008ac2628 legge `val->intval` PRIMA della `printk`.
 * "\x016%s psp=%d val=%d \n"@0xffffff8009261500 -- lo spazio prima di \n e'
 * della fabbrica. `__func__` = "mt_wls_set_property"@0xffffff8009261515.
 */
static int mt_wls_set_property(struct power_supply *psy,
	enum power_supply_property psp, const union power_supply_propval *val)
{
	struct mt_charger *mtk_chg = power_supply_get_drvdata(psy);

	pr_info("%s psp=%d val=%d \n", __func__, psp, val->intval);

	/* "b40001d3 cbz"@0xffffff8008ac2648 ;
	 * "\x015%s: no mtk chg data\n"@0xffffff80092613b3 (KERN_NOTICE) */
	if (!mtk_chg) {
		pr_notice("%s: no mtk chg data\n", __func__);
		return -EINVAL;
	}

	/* La tavola di salto sta a 0xffffff8008f81cc8, 17 byte indicizzati da
	 * `psp - 1` ("510006a8 sub"@0xffffff8008ac264c), e vale
	 *   0:0x00  3:0x0f  15:0x16  16:0x19  tutti gli altri 0x36
	 * cioe' psp = 1, 4, 16, 17; il caso 65 sta fuori dalla tavola
	 * ("710106bf cmp"@0xffffff8008ac269c). */
	switch (psp) {
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		/* "b9028668 str"@0xffffff8008ac2678 */
		mtk_chg->chg_type = val->intval;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		/* "b901ea68 str"@0xffffff8008ac26b4 poi
		 * "1a9f07e8 cset"@0xffffff8008ac26c0 +
		 * "390a0268 strb"@0xffffff8008ac26c4 */
		mtk_chg->c488 = val->intval;
		mtk_chg->chg_online = val->intval != 0;
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		/* "b901f668 str"@0xffffff8008ac26d0 */
		mtk_chg->c500 = val->intval;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		/* "b901f268 str"@0xffffff8008ac26dc */
		mtk_chg->c496 = val->intval;
		break;
	case POWER_SUPPLY_PROP_CHARGE_ENABLED:
		/* "b901fa68 str"@0xffffff8008ac26a8 */
		mtk_chg->c504 = val->intval;
		break;
	default:
		return -EINVAL;
	}

	/* "b9428662 ldr"@0xffffff8008ac26e0 rilegge `chg_type` DOPO lo switch */
	dump_charger_name(mtk_chg->chg_type);
	/* "97d83254 bl"@0xffffff8008ac2734 verso <queue_work_on> con
	 * "321d03e0 orr"@0xffffff8008ac2728 = 8 = WORK_CPU_UNBOUND, cioe'
	 * `queue_work`; x1 = cti+168, x2 = cti+176 */
	queue_work(mtk_chg->cti->chg_in_wq, &mtk_chg->cti->chg_in_work);
	/* "f940f260 ldr"@0xffffff8008ac2738 (+480) e
	 * "f9403e60 ldr"@0xffffff8008ac2740 (+120) */
	power_supply_changed(mtk_chg->wls_psy);
	power_supply_changed(mtk_chg->chg_psy);
	return 0;
}

/*
 * "\x016%s\n"@0xffffff800911cbac con
 * `__func__` = "mt_wls_get_property"@0xffffff8009261529.
 */
static int mt_wls_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct mt_charger *mtk_chg = power_supply_get_drvdata(psy);

	pr_info("%s\n", __func__);

	/* "b40001b4 cbz"@0xffffff8008ac2790 */
	if (!mtk_chg) {
		pr_notice("%s: no mtk chg data\n", __func__);
		return -EINVAL;
	}

	/* Tavola a 0xffffff8008f81cd9: 0:0x00 3:0x0d 15:0x12 16:0x14,
	 * gli altri 0x17 -- gli stessi cinque casi della `set`. */
	switch (psp) {
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		/* "b9428688 ldr"@0xffffff8008ac27bc */
		val->intval = mtk_chg->chg_type;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		/* "94002a92 bl"@0xffffff8008ac27f0 +
		 * "12000108 and"@0xffffff8008ac27fc */
		val->intval = wls_get_online();
		break;
	case POWER_SUPPLY_PROP_CURRENT_MAX:
		/* "b941f688 ldr"@0xffffff8008ac2804 */
		val->intval = mtk_chg->c500;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		/* "b941f288 ldr"@0xffffff8008ac280c */
		val->intval = mtk_chg->c496;
		break;
	case POWER_SUPPLY_PROP_CHARGE_ENABLED:
		/* "b941fa88 ldr"@0xffffff8008ac27e8 */
		val->intval = mtk_chg->c504;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/*
 * Nessuna `printk` e nessun accesso alla struttura: la funzione e' un puro
 * test sul solo `psp`. "51004028 sub"@0xffffff8008ac2828 toglie 16 e
 * "7100c51f cmp"@0xffffff8008ac282c confronta con 0x31; la maschera
 * "d2800069 mov"@0xffffff8008ac283c + "f2e00049 movk"@0xffffff8008ac2840 vale
 * 0x0002000000000003, cioe' i bit 0, 1 e 49 -- psp - 16 in {0, 1, 49}, cioe'
 * psp in {16, 17, 65}. Sono le tre proprieta' che la `set` sa scrivere e che
 * non sono `chg_type` ne' `online`.
 */
static int mt_wls_property_is_writeable(struct power_supply *psy,
	enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_CURRENT_MAX:
	case POWER_SUPPLY_PROP_CURRENT_NOW:
	case POWER_SUPPLY_PROP_CHARGE_ENABLED:
		return 1;
	default:
		return 0;
	}
}

/*
 * Un solo caso. "7100103f cmp"@0xffffff8008ac285c prova `psp == 4` PRIMA di
 * salvare qualunque registro: il ramo d'errore non tocca la pila
 * ("128002a0 mov"@0xffffff8008ac2894 = -22 e ritorno). `psy` non e' usata --
 * nessuna `power_supply_get_drvdata` -- ed e' un fatto, non una svista da
 * correggere: la funzione rende il globale, non un campo della struttura.
 */
static int mt_rvs_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	if (psp != POWER_SUPPLY_PROP_ONLINE)
		return -EINVAL;

	/* "94002a7f bl"@0xffffff8008ac2874 + "12000108 and"@0xffffff8008ac287c */
	val->intval = rvs_get_online();
	return 0;
}

static enum power_supply_property mt_charger_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property mt_ac_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property mt_usb_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
};

/*
 * LE DUE TAVOLE DELL'AGGIUNTA WINGTECH, lette dai byte del kernel di
 * fabbrica a 0xffffff80099a6c34 e 0xffffff80099a6c44:
 *
 *   ffffff80099a6c30  08000000 04000000 11000000 10000000
 *   ffffff80099a6c40  41000000 04000000 00000000 00000000
 *
 * cioe' 4, 17, 16, 65 a partire da 0xc34 e il solo 4 a 0xc44. Le due
 * numerosita' sono confermate dal codice: "321e03e8 orr"@0xffffff8008ac1e84
 * mette w8 = 4 e "a918200b stp"@0xffffff8008ac1ea0 lo scrive a +392, mentre
 * "320003ea orr"@0xffffff8008ac1e34 mette w10 = 1 e
 * "f9010c0a str"@0xffffff8008ac1f0c lo scrive a +536.
 *
 * Le tre scrivibili di `mt_wls_properties` sono esattamente quelle che
 * `mt_wls_property_is_writeable` accetta, e l'unica di `mt_rvs_properties`
 * e' l'unico caso che `mt_rvs_get_property` sa trattare: due conferme
 * indipendenti.
 */
static enum power_supply_property mt_wls_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_CHARGE_ENABLED,
};

static enum power_supply_property mt_rvs_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static void tcpc_power_off_work_handler(struct work_struct *work)
{
	pr_info("%s\n", __func__);
	kernel_power_off();
}

static void charger_in_work_handler(struct work_struct *work)
{
	mtk_charger_int_handler();
	fg_charger_in_handler();
}

#ifdef CONFIG_TCPC_CLASS
static void plug_in_out_handler(struct chg_type_info *cti, bool en, bool ignore)
{
	mutex_lock(&cti->chgdet_lock);
	cti->chgdet_en = en;
	cti->ignore_usb = ignore;
	cti->plugin = en;
	atomic_inc(&cti->chgdet_cnt);
	wake_up_interruptible(&cti->waitq);
	mutex_unlock(&cti->chgdet_lock);
}

static int pd_tcp_notifier_call(struct notifier_block *pnb,
				unsigned long event, void *data)
{
	struct tcp_notify *noti = data;
	struct chg_type_info *cti = container_of(pnb,
		struct chg_type_info, pd_nb);
	int vbus = 0;

	switch (event) {
	case TCP_NOTIFY_TYPEC_STATE:
		if (noti->typec_state.old_state == TYPEC_UNATTACHED &&
		    (noti->typec_state.new_state == TYPEC_ATTACHED_SNK ||
		    noti->typec_state.new_state == TYPEC_ATTACHED_CUSTOM_SRC ||
		    noti->typec_state.new_state == TYPEC_ATTACHED_NORP_SRC)) {
			pr_info("%s USB Plug in, pol = %d\n", __func__,
					noti->typec_state.polarity);
			plug_in_out_handler(cti, true, false);
		} else if ((noti->typec_state.old_state == TYPEC_ATTACHED_SNK ||
		    noti->typec_state.old_state == TYPEC_ATTACHED_CUSTOM_SRC ||
			noti->typec_state.old_state == TYPEC_ATTACHED_NORP_SRC)
			&& noti->typec_state.new_state == TYPEC_UNATTACHED) {
			if (cti->tcpc_kpoc) {
				vbus = battery_get_vbus();
				pr_info("%s KPOC Plug out, vbus = %d\n",
					__func__, vbus);
				queue_work_on(cpumask_first(cpu_online_mask),
					      cti->pwr_off_wq,
					      &cti->pwr_off_work);
				break;
			}
			pr_info("%s USB Plug out\n", __func__);
			plug_in_out_handler(cti, false, false);
		} else if (noti->typec_state.old_state == TYPEC_ATTACHED_SRC &&
			noti->typec_state.new_state == TYPEC_ATTACHED_SNK) {
			pr_info("%s Source_to_Sink\n", __func__);
			plug_in_out_handler(cti, true, true);
		}  else if (noti->typec_state.old_state == TYPEC_ATTACHED_SNK &&
			noti->typec_state.new_state == TYPEC_ATTACHED_SRC) {
			pr_info("%s Sink_to_Source\n", __func__);
			plug_in_out_handler(cti, false, true);
		}
		break;
	}
	return NOTIFY_OK;
}
#endif

static int chgdet_task_threadfn(void *data)
{
	struct chg_type_info *cti = data;
	bool attach = false;
	int ret = 0;

	pr_info("%s: ++\n", __func__);
	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(cti->waitq,
					     atomic_read(&cti->chgdet_cnt) > 0);
		if (ret < 0) {
			pr_info("%s: wait event been interrupted(%d)\n",
				__func__, ret);
			continue;
		}

		pm_stay_awake(cti->dev);
		mutex_lock(&cti->chgdet_lock);
		atomic_set(&cti->chgdet_cnt, 0);
		attach = cti->chgdet_en;
		mutex_unlock(&cti->chgdet_lock);

#ifdef CONFIG_MTK_EXTERNAL_CHARGER_TYPE_DETECT
		if (cti->chg_consumer)
			charger_manager_enable_chg_type_det(cti->chg_consumer,
							attach);
#else
		mtk_pmic_enable_chr_type_det(attach);
#endif
		pm_relax(cti->dev);
	}
	pr_info("%s: --\n", __func__);
	return 0;
}

static int mt_charger_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct chg_type_info *cti = NULL;
	struct mt_charger *mt_chg = NULL;

	pr_info("%s\n", __func__);

	mt_chg = devm_kzalloc(&pdev->dev, sizeof(*mt_chg), GFP_KERNEL);
	if (!mt_chg)
		return -ENOMEM;

	mt_chg->dev = &pdev->dev;
	mt_chg->chg_online = false;
	mt_chg->chg_type = CHARGER_UNKNOWN;

	mt_chg->chg_desc.name = "charger";
	mt_chg->chg_desc.type = POWER_SUPPLY_TYPE_UNKNOWN;
	mt_chg->chg_desc.properties = mt_charger_properties;
	mt_chg->chg_desc.num_properties = ARRAY_SIZE(mt_charger_properties);
	mt_chg->chg_desc.set_property = mt_charger_set_property;
	mt_chg->chg_desc.get_property = mt_charger_get_property;
	mt_chg->chg_cfg.drv_data = mt_chg;

	mt_chg->ac_desc.name = "ac";
	mt_chg->ac_desc.type = POWER_SUPPLY_TYPE_MAINS;
	mt_chg->ac_desc.properties = mt_ac_properties;
	mt_chg->ac_desc.num_properties = ARRAY_SIZE(mt_ac_properties);
	mt_chg->ac_desc.get_property = mt_ac_get_property;
	mt_chg->ac_cfg.drv_data = mt_chg;

	mt_chg->usb_desc.name = "usb";
	mt_chg->usb_desc.type = POWER_SUPPLY_TYPE_USB;
	mt_chg->usb_desc.properties = mt_usb_properties;
	mt_chg->usb_desc.num_properties = ARRAY_SIZE(mt_usb_properties);
	mt_chg->usb_desc.get_property = mt_usb_get_property;
	mt_chg->usb_cfg.drv_data = mt_chg;

	/*
	 * I DUE DESCRITTORI DELL'AGGIUNTA WINGTECH.
	 * "9101258c add"@0xffffff8008ac1e98 mette "wireless"@0xffffff80092ba049
	 * e "f900b80c str"@0xffffff8008ac1eb4 lo scrive a +368;
	 * "910a6d08 add"@0xffffff8008ac1ebc mette "rvs"@0xffffff800926129b e
	 * "f9010008 str"@0xffffff8008ac1ed4 lo scrive a +512.
	 *
	 * IL TIPO DELL'ALIMENTAZIONE WIRELESS E' 8, NON 12. Il binario e'
	 * esplicito -- "321d03ed orr"@0xffffff8008ac1ea8 mette w13 = 8 e
	 * "b901780d str"@0xffffff8008ac1ec8 lo scrive a +376 -- e 8 e'
	 * POWER_SUPPLY_TYPE_USB_TYPE_C, non POWER_SUPPLY_TYPE_WIRELESS. Che la
	 * numerazione dell'enum sia la stessa di ALPS lo provano le tre righe
	 * gia' presenti: chg = 0 ("b900101f str"@0xffffff8008ac1e58),
	 * ac = 3 ("b900880e str"@0xffffff8008ac1e6c), usb = 4
	 * ("b9010008 str"@0xffffff8008ac1e9c). E' una stranezza della fabbrica
	 * e si riproduce.
	 */
	mt_chg->wls_desc.name = "wireless";
	mt_chg->wls_desc.type = POWER_SUPPLY_TYPE_USB_TYPE_C;
	mt_chg->wls_desc.properties = mt_wls_properties;
	mt_chg->wls_desc.num_properties = ARRAY_SIZE(mt_wls_properties);
	mt_chg->wls_desc.set_property = mt_wls_set_property;
	mt_chg->wls_desc.get_property = mt_wls_get_property;
	mt_chg->wls_desc.property_is_writeable = mt_wls_property_is_writeable;
	mt_chg->wls_cfg.drv_data = mt_chg;

	mt_chg->rvs_desc.name = "rvs";
	mt_chg->rvs_desc.type = POWER_SUPPLY_TYPE_REVERSE;
	mt_chg->rvs_desc.properties = mt_rvs_properties;
	mt_chg->rvs_desc.num_properties = ARRAY_SIZE(mt_rvs_properties);
	mt_chg->rvs_desc.get_property = mt_rvs_get_property;
	mt_chg->rvs_cfg.drv_data = mt_chg;

	mt_chg->chg_psy = power_supply_register(&pdev->dev,
		&mt_chg->chg_desc, &mt_chg->chg_cfg);
	if (IS_ERR(mt_chg->chg_psy)) {
		dev_notice(&pdev->dev, "Failed to register power supply: %ld\n",
			PTR_ERR(mt_chg->chg_psy));
		ret = PTR_ERR(mt_chg->chg_psy);
		return ret;
	}

	mt_chg->ac_psy = power_supply_register(&pdev->dev, &mt_chg->ac_desc,
		&mt_chg->ac_cfg);
	if (IS_ERR(mt_chg->ac_psy)) {
		dev_notice(&pdev->dev, "Failed to register power supply: %ld\n",
			PTR_ERR(mt_chg->ac_psy));
		ret = PTR_ERR(mt_chg->ac_psy);
		goto err_ac_psy;
	}

	mt_chg->usb_psy = power_supply_register(&pdev->dev, &mt_chg->usb_desc,
		&mt_chg->usb_cfg);
	if (IS_ERR(mt_chg->usb_psy)) {
		dev_notice(&pdev->dev, "Failed to register power supply: %ld\n",
			PTR_ERR(mt_chg->usb_psy));
		ret = PTR_ERR(mt_chg->usb_psy);
		goto err_usb_psy;
	}

	/* "9105c261 add"@0xffffff8008ac1fc0 mette x1 = mt_chg + 368 (wls_desc) e
	 * "91070262 add"@0xffffff8008ac1fc4 x2 = mt_chg + 448 (wls_cfg) */
	mt_chg->wls_psy = power_supply_register(&pdev->dev, &mt_chg->wls_desc,
		&mt_chg->wls_cfg);
	if (IS_ERR(mt_chg->wls_psy)) {
		dev_notice(&pdev->dev, "Failed to register power supply: %ld\n",
			PTR_ERR(mt_chg->wls_psy));
		ret = PTR_ERR(mt_chg->wls_psy);
		goto err_wls_psy;
	}

	mt_chg->rvs_psy = power_supply_register(&pdev->dev, &mt_chg->rvs_desc,
		&mt_chg->rvs_cfg);
	if (IS_ERR(mt_chg->rvs_psy)) {
		dev_notice(&pdev->dev, "Failed to register power supply: %ld\n",
			PTR_ERR(mt_chg->rvs_psy));
		ret = PTR_ERR(mt_chg->rvs_psy);
		goto err_rvs_psy;
	}

	cti = devm_kzalloc(&pdev->dev, sizeof(*cti), GFP_KERNEL);
	if (!cti) {
		ret = -ENOMEM;
		goto err_no_mem;
	}
	cti->dev = &pdev->dev;

#ifdef CONFIG_TCPC_CLASS
	cti->tcpc_dev = tcpc_dev_get_by_name("type_c_port0");
	if (cti->tcpc_dev == NULL) {
		pr_info("%s: tcpc device not ready, defer\n", __func__);
		ret = -EPROBE_DEFER;
		goto err_get_tcpc_dev;
	}
	cti->pd_nb.notifier_call = pd_tcp_notifier_call;
	ret = register_tcp_dev_notifier(cti->tcpc_dev,
		&cti->pd_nb, TCP_NOTIFY_TYPE_ALL);
	if (ret < 0) {
		pr_info("%s: register tcpc notifer fail\n", __func__);
		ret = -EINVAL;
		goto err_get_tcpc_dev;
	}
#endif

	cti->chg_consumer = charger_manager_get_by_name(cti->dev,
							"charger_port1");
	if (!cti->chg_consumer) {
		pr_info("%s: get charger consumer device failed\n", __func__);
		ret = -EINVAL;
		goto err_get_tcpc_dev;
	}

	ret = get_boot_mode();
	if (ret == KERNEL_POWER_OFF_CHARGING_BOOT ||
	    ret == LOW_POWER_OFF_CHARGING_BOOT)
		cti->tcpc_kpoc = true;
	pr_info("%s KPOC(%d)\n", __func__, cti->tcpc_kpoc);

	/* Init Charger Detection */
	mutex_init(&cti->chgdet_lock);
	atomic_set(&cti->chgdet_cnt, 0);

	init_waitqueue_head(&cti->waitq);
	cti->chgdet_task = kthread_run(
				chgdet_task_threadfn, cti, "chgdet_thread");
	ret = PTR_ERR_OR_ZERO(cti->chgdet_task);
	if (ret < 0) {
		pr_info("%s: create chg det work fail\n", __func__);
		return ret;
	}

	/* Init power off work */
	cti->pwr_off_wq = create_singlethread_workqueue("tcpc_power_off");
	INIT_WORK(&cti->pwr_off_work, tcpc_power_off_work_handler);

	cti->chg_in_wq = create_singlethread_workqueue("charger_in");
	INIT_WORK(&cti->chg_in_work, charger_in_work_handler);

	mt_chg->cti = cti;
	platform_set_drvdata(pdev, mt_chg);
	device_init_wakeup(&pdev->dev, true);

	pr_info("%s done\n", __func__);
	return 0;

err_get_tcpc_dev:
	devm_kfree(&pdev->dev, cti);
err_no_mem:
	power_supply_unregister(mt_chg->rvs_psy);
err_rvs_psy:
	power_supply_unregister(mt_chg->wls_psy);
err_wls_psy:
	power_supply_unregister(mt_chg->usb_psy);
err_usb_psy:
	power_supply_unregister(mt_chg->ac_psy);
err_ac_psy:
	power_supply_unregister(mt_chg->chg_psy);
	return ret;
}

static int mt_charger_remove(struct platform_device *pdev)
{
	struct mt_charger *mt_charger = platform_get_drvdata(pdev);
	struct chg_type_info *cti = mt_charger->cti;

	power_supply_unregister(mt_charger->chg_psy);
	power_supply_unregister(mt_charger->ac_psy);
	power_supply_unregister(mt_charger->usb_psy);
	/* Anche le due power_supply dell'aggiunta Wingtech, wls (+480) e
	 * rvs (+624): la fabbrica le tocca insieme alle altre. */
	power_supply_unregister(mt_charger->wls_psy);
	power_supply_unregister(mt_charger->rvs_psy);

	pr_info("%s\n", __func__);
	if (cti->chgdet_task) {
		kthread_stop(cti->chgdet_task);
		atomic_inc(&cti->chgdet_cnt);
		wake_up_interruptible(&cti->waitq);
	}

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int mt_charger_suspend(struct device *dev)
{
	/* struct mt_charger *mt_charger = dev_get_drvdata(dev); */
	return 0;
}

static int mt_charger_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct mt_charger *mt_charger = platform_get_drvdata(pdev);

	power_supply_changed(mt_charger->chg_psy);
	power_supply_changed(mt_charger->ac_psy);
	power_supply_changed(mt_charger->usb_psy);
	/* Anche le due power_supply dell'aggiunta Wingtech, wls (+480) e
	 * rvs (+624): la fabbrica le tocca insieme alle altre. */
	power_supply_changed(mt_charger->wls_psy);
	power_supply_changed(mt_charger->rvs_psy);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(mt_charger_pm_ops, mt_charger_suspend,
	mt_charger_resume);

static const struct of_device_id mt_charger_match[] = {
	{ .compatible = "mediatek,mt-charger", },
	{ },
};
static struct platform_driver mt_charger_driver = {
	.probe = mt_charger_probe,
	.remove = mt_charger_remove,
	.driver = {
		.name = "mt-charger-det",
		.owner = THIS_MODULE,
		.pm = &mt_charger_pm_ops,
		.of_match_table = mt_charger_match,
	},
};

/* Legacy api to prevent build error */
bool upmu_is_chr_det(void)
{
	struct mt_charger *mtk_chg;
	struct power_supply *psy = power_supply_get_by_name("charger");

	if (!psy) {
		pr_info("%s: get power supply failed\n", __func__);
		return -EINVAL;
	}
	mtk_chg = power_supply_get_drvdata(psy);
	return mtk_chg->chg_online;
}

/* Legacy api to prevent build error */
bool pmic_chrdet_status(void)
{
	if (upmu_is_chr_det())
		return true;

	pr_notice("%s: No charger\n", __func__);
	return false;
}

enum charger_type mt_get_charger_type(void)
{
	struct mt_charger *mtk_chg;
	struct power_supply *psy = power_supply_get_by_name("charger");

	if (!psy) {
		pr_info("%s: get power supply failed\n", __func__);
		return -EINVAL;
	}
	mtk_chg = power_supply_get_drvdata(psy);
	return mtk_chg->chg_type;
}

bool mt_charger_plugin(void)
{
	struct mt_charger *mtk_chg;
	struct power_supply *psy = power_supply_get_by_name("charger");
	struct chg_type_info *cti;

	if (!psy) {
		pr_info("%s: get power supply failed\n", __func__);
		return -EINVAL;
	}
	mtk_chg = power_supply_get_drvdata(psy);
	cti = mtk_chg->cti;
	pr_info("%s plugin:%d\n", __func__, cti->plugin);

	return cti->plugin;
}

static s32 __init mt_charger_det_init(void)
{
	return platform_driver_register(&mt_charger_driver);
}

static void __exit mt_charger_det_exit(void)
{
	platform_driver_unregister(&mt_charger_driver);
}

subsys_initcall(mt_charger_det_init);
module_exit(mt_charger_det_exit);

MODULE_DESCRIPTION("mt-charger-detection");
MODULE_AUTHOR("MediaTek");
MODULE_LICENSE("GPL v2");

#endif /* CONFIG_MTK_FPGA */
