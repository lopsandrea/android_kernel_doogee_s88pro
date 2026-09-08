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
#include "tpd.h"
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fb.h>
/* per proc_create/single_open/seq_printf delle quattro funzioni
 * Wingtech aggiunte piu' sotto -- FUORI dai #if condizionali che
 * questo file usa piu' avanti per <linux/input/mt.h> */
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#ifdef CONFIG_MTK_MT6306_GPIO_SUPPORT
#include <mtk_6306_gpio.h>
#endif

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#if defined(CONFIG_MTK_S3320) || defined(CONFIG_MTK_S3320_50) \
	|| defined(CONFIG_MTK_S3320_47) || defined(CONFIG_MTK_MIT200) \
	|| defined(CONFIG_TOUCHSCREEN_SYNAPTICS_S3528) \
	|| defined(CONFIG_MTK_S7020) \
	|| defined(CONFIG_TOUCHSCREEN_MTK_SYNAPTICS_3320_50)
#include <linux/input/mt.h>
#endif /* CONFIG_MTK_S3320 */
/* for magnify velocity******************************************** */
#define TOUCH_IOC_MAGIC 'A'

#define TPD_GET_VELOCITY_CUSTOM_X _IO(TOUCH_IOC_MAGIC, 0)
#define TPD_GET_VELOCITY_CUSTOM_Y _IO(TOUCH_IOC_MAGIC, 1)
#define TPD_GET_FILTER_PARA _IOWR(TOUCH_IOC_MAGIC, 2, struct tpd_filter_t)
#ifdef CONFIG_COMPAT
#define COMPAT_TPD_GET_FILTER_PARA _IOWR(TOUCH_IOC_MAGIC, \
						2, struct tpd_filter_t)
#endif
struct tpd_filter_t tpd_filter;
struct tpd_dts_info tpd_dts_data;
struct pinctrl *pinctrl1;
struct pinctrl_state *pins_default;
struct pinctrl_state *eint_as_int, *eint_output0,
		*eint_output1, *rst_output0, *rst_output1;
const struct of_device_id touch_of_match[] = {
	{ .compatible = "mediatek,touch", },
	{ .compatible = "mediatek,mt8167-touch", },
	{},
};

void tpd_get_dts_info(void)
{
	struct device_node *node1 = NULL;
	int key_dim_local[16] = {0}, i = 0;

	node1 = of_find_matching_node(node1, touch_of_match);
	if (node1) {
		of_property_read_u32(node1,
			"tpd-max-touch-num", &tpd_dts_data.touch_max_num);
		of_property_read_u32(node1,
			"use-tpd-button", &tpd_dts_data.use_tpd_button);
		pr_debug("[tpd]use-tpd-button = %d\n",
			tpd_dts_data.use_tpd_button);
		if (of_property_read_u32_array(node1, "tpd-resolution",
			tpd_dts_data.tpd_resolution,
			ARRAY_SIZE(tpd_dts_data.tpd_resolution))) {
			pr_debug("[tpd] resulution is %d %d",
				tpd_dts_data.tpd_resolution[0],
				tpd_dts_data.tpd_resolution[1]);
		}
		if (tpd_dts_data.use_tpd_button) {
			of_property_read_u32(node1,
				"tpd-key-num", &tpd_dts_data.tpd_key_num);
			if (of_property_read_u32_array(node1, "tpd-key-local",
				tpd_dts_data.tpd_key_local,
				ARRAY_SIZE(tpd_dts_data.tpd_key_local)))
				pr_debug("tpd-key-local: %d %d %d %d",
					tpd_dts_data.tpd_key_local[0],
					tpd_dts_data.tpd_key_local[1],
					tpd_dts_data.tpd_key_local[2],
					tpd_dts_data.tpd_key_local[3]);
			if (of_property_read_u32_array(node1,
				"tpd-key-dim-local",
				key_dim_local, ARRAY_SIZE(key_dim_local))) {
				memcpy(tpd_dts_data.tpd_key_dim_local,
					key_dim_local, sizeof(key_dim_local));
				for (i = 0; i < 4; i++) {
					pr_debug("[tpd]key[%d].key_x = %d\n", i,
						tpd_dts_data
							.tpd_key_dim_local[i]
							.key_x);
					pr_debug("[tpd]key[%d].key_y = %d\n", i,
						tpd_dts_data
							.tpd_key_dim_local[i]
							.key_y);
					pr_debug("[tpd]key[%d].key_W = %d\n", i,
						tpd_dts_data
							.tpd_key_dim_local[i]
							.key_width);
					pr_debug("[tpd]key[%d].key_H = %d\n", i,
						tpd_dts_data
							.tpd_key_dim_local[i]
							.key_height);
				}
			}
		}
		of_property_read_u32(node1, "tpd-filter-enable",
			&tpd_dts_data.touch_filter.enable);
		if (tpd_dts_data.touch_filter.enable) {
			of_property_read_u32(node1,
				"tpd-filter-pixel-density",
				&tpd_dts_data.touch_filter.pixel_density);
			if (of_property_read_u32_array(node1,
				"tpd-filter-custom-prameters",
				(u32 *)tpd_dts_data.touch_filter.W_W,
				ARRAY_SIZE(tpd_dts_data.touch_filter.W_W)))
				pr_debug("get tpd-filter-custom-parameters");
			if (of_property_read_u32_array(node1,
				"tpd-filter-custom-speed",
				tpd_dts_data.touch_filter.VECLOCITY_THRESHOLD,
				ARRAY_SIZE(tpd_dts_data
						.touch_filter
						.VECLOCITY_THRESHOLD)))
				pr_debug("get tpd-filter-custom-speed");
		}
		memcpy(&tpd_filter,
			&tpd_dts_data.touch_filter, sizeof(tpd_filter));
		pr_debug("[tpd]tpd-filter-enable = %d, pixel_density = %d\n",
				tpd_filter.enable, tpd_filter.pixel_density);
		tpd_dts_data.tpd_use_ext_gpio =
			of_property_read_bool(node1, "tpd-use-ext-gpio");
		of_property_read_u32(node1,
			"tpd-rst-ext-gpio-num",
			&tpd_dts_data.rst_ext_gpio_num);

	} else {
		TPD_DMESG("can't find touch compatible custom node\n");
	}
}

static DEFINE_MUTEX(tpd_set_gpio_mutex);
void tpd_gpio_as_int(int pin)
{
	mutex_lock(&tpd_set_gpio_mutex);
	TPD_DEBUG("[tpd] %s\n", __func__);
	if (pin == 1)
		pinctrl_select_state(pinctrl1, eint_as_int);
	mutex_unlock(&tpd_set_gpio_mutex);
}

void tpd_gpio_output(int pin, int level)
{
	mutex_lock(&tpd_set_gpio_mutex);
	TPD_DEBUG("%s pin = %d, level = %d\n", __func__, pin, level);
	if (pin == 1) {
		if (level)
			pinctrl_select_state(pinctrl1, eint_output1);
		else
			pinctrl_select_state(pinctrl1, eint_output0);
	} else {
		if (tpd_dts_data.tpd_use_ext_gpio) {
#ifdef CONFIG_MTK_MT6306_GPIO_SUPPORT
			mt6306_set_gpio_dir(
				tpd_dts_data.rst_ext_gpio_num, 1);
			mt6306_set_gpio_out(
				tpd_dts_data.rst_ext_gpio_num, level);
#endif
		} else {
			if (level)
				pinctrl_select_state(pinctrl1, rst_output1);
			else
				pinctrl_select_state(pinctrl1, rst_output0);
		}
	}
	mutex_unlock(&tpd_set_gpio_mutex);
}
int tpd_get_gpio_info(struct platform_device *pdev)
{
	int ret;

	TPD_DEBUG("[tpd %d] mt_tpd_pinctrl+++++++++++++++++\n", pdev->id);
	pinctrl1 = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(pinctrl1)) {
		ret = PTR_ERR(pinctrl1);
		dev_info(&pdev->dev, "fwq Cannot find pinctrl1!\n");
		return ret;
	}
	pins_default = pinctrl_lookup_state(pinctrl1, "default");
	if (IS_ERR(pins_default)) {
		ret = PTR_ERR(pins_default);
		TPD_DMESG("Cannot find pinctrl default %d!\n", ret);
	}
	eint_as_int = pinctrl_lookup_state(pinctrl1, "state_eint_as_int");
	if (IS_ERR(eint_as_int)) {
		ret = PTR_ERR(eint_as_int);
		TPD_DMESG("Cannot find pinctrl state_eint_as_int!\n");
		return ret;
	}
	eint_output0 = pinctrl_lookup_state(pinctrl1, "state_eint_output0");
	if (IS_ERR(eint_output0)) {
		ret = PTR_ERR(eint_output0);
		TPD_DMESG("Cannot find pinctrl state_eint_output0!\n");
		return ret;
	}
	eint_output1 = pinctrl_lookup_state(pinctrl1, "state_eint_output1");
	if (IS_ERR(eint_output1)) {
		ret = PTR_ERR(eint_output1);
		TPD_DMESG("Cannot find pinctrl state_eint_output1!\n");
		return ret;
	}
	if (tpd_dts_data.tpd_use_ext_gpio == false) {
		rst_output0 =
			pinctrl_lookup_state(pinctrl1, "state_rst_output0");
		if (IS_ERR(rst_output0)) {
			ret = PTR_ERR(rst_output0);
			TPD_DMESG("Cannot find pinctrl state_rst_output0!\n");
			return ret;
		}
		rst_output1 =
			pinctrl_lookup_state(pinctrl1, "state_rst_output1");
		if (IS_ERR(rst_output1)) {
			ret = PTR_ERR(rst_output1);
			TPD_DMESG("Cannot find pinctrl state_rst_output1!\n");
			return ret;
		}
	}
	TPD_DEBUG("[tpd%d] mt_tpd_pinctrl----------\n", pdev->id);
	return 0;
}

static int tpd_misc_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static int tpd_misc_release(struct inode *inode, struct file *file)
{
	return 0;
}

#ifdef CONFIG_COMPAT
static long tpd_compat_ioctl(
			struct file *file, unsigned int cmd,
			unsigned long arg)
{
	long ret;
	void __user *arg32 = compat_ptr(arg);

	if (!file->f_op || !file->f_op->unlocked_ioctl)
		return -ENOTTY;
	switch (cmd) {
	case COMPAT_TPD_GET_FILTER_PARA:
		if (arg32 == NULL) {
			pr_info("invalid argument.");
			return -EINVAL;
		}
		ret = file->f_op->unlocked_ioctl(file, TPD_GET_FILTER_PARA,
					   (unsigned long)arg32);
		if (ret) {
			pr_info("TPD_GET_FILTER_PARA unlocked_ioctl failed.");
			return ret;
		}
		break;
	default:
		pr_info("tpd: unknown IOCTL: 0x%08x\n", cmd);
		ret = -ENOIOCTLCMD;
		break;
	}
	return ret;
}
#endif
static long tpd_unlocked_ioctl(struct file *file,
			unsigned int cmd, unsigned long arg)
{
	/* char strbuf[256]; */
	void __user *data;

	long err = 0;

	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE,
			(void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ,
			(void __user *)arg, _IOC_SIZE(cmd));
	if (err) {
		pr_info("tpd: access error: %08X, (%2d, %2d)\n",
			cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch (cmd) {
	case TPD_GET_VELOCITY_CUSTOM_X:
		data = (void __user *)arg;

		if (data == NULL) {
			err = -EINVAL;
			break;
		}

		if (copy_to_user(data,
			&tpd_v_magnify_x, sizeof(tpd_v_magnify_x))) {
			err = -EFAULT;
			break;
		}

		break;

	case TPD_GET_VELOCITY_CUSTOM_Y:
		data = (void __user *)arg;

		if (data == NULL) {
			err = -EINVAL;
			break;
		}

		if (copy_to_user(data,
			&tpd_v_magnify_y, sizeof(tpd_v_magnify_y))) {
			err = -EFAULT;
			break;
		}

		break;
	case TPD_GET_FILTER_PARA:
			data = (void __user *) arg;

			if (data == NULL) {
				err = -EINVAL;
				TPD_DMESG("GET_FILTER_PARA: data is null\n");
				break;
			}

			if (copy_to_user(data, &tpd_filter,
					sizeof(struct tpd_filter_t))) {
				TPD_DMESG("GET_FILTER_PARA: copy data error\n");
				err = -EFAULT;
				break;
			}
			break;
	default:
		pr_info("tpd: unknown IOCTL: 0x%08x\n", cmd);
		err = -ENOIOCTLCMD;
		break;

	}

	return err;
}
static struct work_struct touch_resume_work;
static struct workqueue_struct *touch_resume_workqueue;
static const struct file_operations tpd_fops = {
/* .owner = THIS_MODULE, */
	.open = tpd_misc_open,
	.release = tpd_misc_release,
	.unlocked_ioctl = tpd_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = tpd_compat_ioctl,
#endif
};

/*---------------------------------------------------------------------------*/
static struct miscdevice tpd_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "touch",
	.fops = &tpd_fops,
};

/* ********************************************** */
/* #endif */


/* function definitions */
static int __init tpd_device_init(void);
static void __exit tpd_device_exit(void);
static int tpd_probe(struct platform_device *pdev);
static int tpd_remove(struct platform_device *pdev);
static struct work_struct tpd_init_work;
static struct workqueue_struct *tpd_init_workqueue;
static int tpd_suspend_flag;
int tpd_register_flag;
/* global variable definitions */
struct tpd_device *tpd;
static struct tpd_driver_t tpd_driver_list[TP_DRV_MAX_COUNT];	/* = {0}; */

struct platform_device tpd_device = {
	.name		= TPD_DEVICE,
	.id			= -1,
};
const struct dev_pm_ops tpd_pm_ops = {
	.suspend = NULL,
	.resume = NULL,
};
static struct platform_driver tpd_driver = {
	.remove = tpd_remove,
	.shutdown = NULL,
	.probe = tpd_probe,
	.driver = {
			.name = TPD_DEVICE,
			.pm = &tpd_pm_ops,
			.owner = THIS_MODULE,
			.of_match_table = touch_of_match,
	},
};
static struct tpd_driver_t *g_tpd_drv;
/* hh: use fb_notifier */
static struct notifier_block tpd_fb_notifier;
/* use fb_notifier */
static void touch_resume_workqueue_callback(struct work_struct *work)
{
	TPD_DEBUG("GTP %s\n", __func__);
	g_tpd_drv->resume(NULL);
	tpd_suspend_flag = 0;
}
static int tpd_fb_notifier_callback(
			struct notifier_block *self,
			unsigned long event, void *data)
{
	struct fb_event *evdata = NULL;
	int blank;
	int err = 0;

	TPD_DEBUG("%s\n", __func__);

	evdata = data;
	/* If we aren't interested in this event, skip it immediately ... */
	if (event != FB_EVENT_BLANK)
		return 0;

	blank = *(int *)evdata->data;
	TPD_DMESG("fb_notify(blank=%d)\n", blank);
	switch (blank) {
	case FB_BLANK_UNBLANK:
		TPD_DMESG("LCD ON Notify\n");
		if (g_tpd_drv && tpd_suspend_flag) {
			err = queue_work(touch_resume_workqueue,
						&touch_resume_work);
			if (!err) {
				TPD_DMESG("start resume_workqueue failed\n");
				return err;
			}
		}
		break;
	case FB_BLANK_POWERDOWN:
		TPD_DMESG("LCD OFF Notify\n");
		if (g_tpd_drv && !tpd_suspend_flag) {
			err = cancel_work_sync(&touch_resume_work);
			if (!err)
				TPD_DMESG("cancel resume_workqueue failed\n");
			g_tpd_drv->suspend(NULL);
		}
		tpd_suspend_flag = 1;
		break;
	default:
		break;
	}
	return 0;
}
/* Add driver: if find TPD_TYPE_CAPACITIVE driver successfully, loading it */
int tpd_driver_add(struct tpd_driver_t *tpd_drv)
{
	int i;

	if (g_tpd_drv != NULL) {
		TPD_DMESG("touch driver exist\n");
		return -1;
	}
	/* check parameter */
	if (tpd_drv == NULL)
		return -1;
	tpd_drv->tpd_have_button = tpd_dts_data.use_tpd_button;
	/* R-touch */
	if (strcmp(tpd_drv->tpd_device_name, "generic") == 0) {
		tpd_driver_list[0].tpd_device_name = tpd_drv->tpd_device_name;
		tpd_driver_list[0].tpd_local_init = tpd_drv->tpd_local_init;
		tpd_driver_list[0].suspend = tpd_drv->suspend;
		tpd_driver_list[0].resume = tpd_drv->resume;
		tpd_driver_list[0].tpd_have_button = tpd_drv->tpd_have_button;
		return 0;
	}
	for (i = 1; i < TP_DRV_MAX_COUNT; i++) {
		/* add tpd driver into list */
		if (tpd_driver_list[i].tpd_device_name == NULL) {
			tpd_driver_list[i].tpd_device_name =
				tpd_drv->tpd_device_name;
			tpd_driver_list[i].tpd_local_init =
				tpd_drv->tpd_local_init;
			tpd_driver_list[i].suspend = tpd_drv->suspend;
			tpd_driver_list[i].resume = tpd_drv->resume;
			tpd_driver_list[i].tpd_have_button =
				tpd_drv->tpd_have_button;
			tpd_driver_list[i].attrs = tpd_drv->attrs;
			break;
		}
		if (strcmp(tpd_driver_list[i].tpd_device_name,
			tpd_drv->tpd_device_name) == 0)
			return 1;	/* driver exist */
	}

	return 0;
}

int tpd_driver_remove(struct tpd_driver_t *tpd_drv)
{
	int i = 0;
	/* check parameter */
	if (tpd_drv == NULL)
		return -1;
	for (i = 0; i < TP_DRV_MAX_COUNT; i++) {
		/* find it */
		if (strcmp(tpd_driver_list[i].tpd_device_name,
				tpd_drv->tpd_device_name) == 0) {
			memset(&tpd_driver_list[i], 0,
				sizeof(struct tpd_driver_t));
			break;
		}
	}
	return 0;
}

/*
 * ---------------------------------------------------------------------
 * L'AGGIUNTA WINGTECH DENTRO mtk_tpd.c -- quattro funzioni, 204 byte
 * ---------------------------------------------------------------------
 * Ricostruite dal disassemblato del kernel di fabbrica. Non sono in nessun
 * albero pubblico: l'albero ALPS ha questo file ma non queste funzioni, ed
 * e' lo stesso schema di `mt_charger_set_opa_mode` dentro
 * mt6370_pmu_charger.c e del flashlight.
 *
 * CHE STIANO IN QUESTO FILE lo dice l'adiacenza degli indirizzi: di fabbrica
 * occupano [0xffffff8008a5151c, 0xffffff8008a515e8), cioe' esattamente lo
 * spazio fra `tpd_driver_remove` (0xffffff8008a514a8, che finisce a
 * 0xffffff8008a5151c) e `tpd_probe` (0xffffff8008a515e8) -- due funzioni che
 * questo file gia' contiene.
 *
 *   fix_tp_proc_info        0xffffff8008a5151c    64   T
 *   wtk_creat_proc_tp_info  0xffffff8008a5155c    64   T
 *   wtk_tp_info_open        0xffffff8008a5159c    36   t
 *   wtk_tp_info_show        0xffffff8008a515c0    40   t
 *
 * PERCHE' SERVONO: `fix_tp_proc_info` e' chiamata da tre driver di tocco --
 * "97ffdce1 bl"@0xffffff8008a5a198 (ILITEK, dentro ilitek_ic.c),
 * "97ff66ae bl"@0xffffff8008a77a64 e "97ff46ac bl"@0xffffff8008a7fa6c --
 * e senza di essa ILITEK non si linka.
 *
 * QUESTA NOTA DICEVA IL CONTRARIO, ED ERA SBAGLIATA. Diceva:
 * «wtk_creat_proc_tp_info non e' chiamata da nessuno, non c'e' nessuna `bl`
 * verso 0xffffff8008a5155c in tutto il kernel; quindi /proc/wtk_tpInfo di
 * fabbrica non viene mai creato». La `bl` non c'e' davvero -- ma perche' la
 * funzione e' INCORPORATA dentro tpd_device_init, dove la fabbrica ha
 * "97bdc883 bl"@0xffffff8009383248 verso proc_create con la stessa stringa e
 * gli stessi permessi. Il nodo viene creato eccome. Costava 44 byte su
 * tpd_device_init, ed e' lo stesso errore che nascondeva il nodo
 * /proc/wtk_memInfo dentro chip_common_init.
 */

/*
 * LA DIMENSIONE DI QUESTO VETTORE E' UNA SCELTA, NON UNA MISURA. Il binario
 * non la dice: `fix_tp_proc_info` riceve la lunghezza come argomento e i tre
 * chiamanti passano il valore reso da una `sprintf`
 * ("2a0003e1 mov"@0xffffff8008a5a190 dopo "9410417a bl"@0xffffff8008a5a18c),
 * quindi non c'e' nessuna costante da leggere. Cio' che e' MISURATO e' il
 * limite superiore: il vettore comincia a 0xffffff800a0fc0b9 e il primo
 * offset piu' alto usato da qualunque codice nella stessa pagina di `.bss` e'
 * 0xe4, quindi il vettore sta in [0xb9, 0xe4) ed e' al piu' di 43 byte.
 * Scelti 43, il massimo compatibile con la misura. Il nome e' scelto anch'esso:
 * l'oracolo non contiene simboli di dati.
 */
/*
 * IL BYTE CHE PRECEDE `wtk_tp_info`, e sta in QUESTA unita' di traduzione.
 *
 * Il binario non lo nomina, quindi il nome viene dall'indirizzo -- lo stesso
 * che usa GT917S/gt1x_generic.c, che lo dichiara `extern u8 g0fc0b8` -- e non
 * e' un fatto. Cio' che E' misurato:
 *
 *   sta a 0xffffff800a0fc0b8, ed e' letto SEI volte in tutta l'immagine, tutte
 *   `ldrb` seguito da `cbz`/`cbnz`, mai una scrittura:
 *     "3942e108 ldrb"@0xffffff80087fca6c   (lcm_suspend)
 *     "3942e108 ldrb"@0xffffff80087fd858   (lcm_suspend, secondo ramo)
 *     "3942e108 ldrb"@0xffffff8008a74d74   (gt1x_suspend)
 *     "3942e108 ldrb"@0xffffff8008a75008   (gt1x_resume)
 *     "3942e108 ldrb"@0xffffff8008a75ef0   (gt1x_debug_write_proc)
 *     "3942e108 ldrb"@0xffffff8008a78028   (tpd_event_handler)
 *
 *   e il byte SUCCESSIVO, 0xffffff800a0fc0b9, e' `wtk_tp_info`, che
 *   `fix_tp_proc_info` riempie con __memcpy. Adiacenti in .bss vuol dire
 *   stessa unita' di traduzione, ed e' questa: e' la stessa tecnica che ha
 *   deciso a chi appartenesse `ilitek_dbg_en`.
 *
 * E' GLOBALE perche' lo leggono altre due unita' (il driver LCM e GT917S), e
 * il primo link con GT917S acceso si e' fermato proprio qui, cinque volte.
 *
 * NESSUNO LO SCRIVE, in tutta l'immagine: resta zero, e i sei rami che lo
 * guardano non si prendono mai. E' un difetto di fabbrica, ed e' riprodotto.
 */
u8 g0fc0b8;

static char wtk_tp_info[43];

/*
 * "92401c33 and"@0xffffff8008a51530 maschera il secondo argomento a OTTO bit
 * prima di usarlo: e' un `u8`, non un `int`. "941019ef bl"@0xffffff8008a51544
 * va a <__memcpy> con x0 = il vettore, x1 = l'argomento, x2 = la lunghezza; e
 * "38336a9f strb"@0xffffff8008a5154c scrive lo zero in coda a `[x20,x19]`,
 * cioe' `wtk_tp_info[len]`. "2a1f03e0 mov"@0xffffff8008a51550 rende 0.
 */
int fix_tp_proc_info(const char *buf, u8 len)
{
	memcpy(wtk_tp_info, buf, len);
	wtk_tp_info[len] = '\0';
	return 0;
}

/*
 * "97e11a92 bl"@0xffffff8008a515d8 va a <seq_printf> con
 * x1 = "%s\n"@0xffffff8009226be1 e x2 = il vettore
 * ("9102e442 add"@0xffffff8008a515d4 = pagina + 0xb9, lo stesso indirizzo che
 * `fix_tp_proc_info` scrive). "2a1f03e0 mov"@0xffffff8008a515dc rende 0.
 */
static int wtk_tp_info_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", wtk_tp_info);
	return 0;
}

/*
 * "aa0103e0 mov"@0xffffff8008a515a4 passa il SECONDO argomento come primo a
 * <single_open> ("97e11c04 bl"@0xffffff8008a515b4): il primo argomento della
 * funzione non e' usato, com'e' per una `.open` di `file_operations`.
 * "91170021 add"@0xffffff8008a515ac mette x1 = 0xffffff8008a515c0, cioe'
 * `wtk_tp_info_show`, e "aa1f03e2 mov"@0xffffff8008a515b0 mette x2 = NULL.
 */
static int wtk_tp_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, wtk_tp_info_show, NULL);
}

/*
 * La struttura sta a 0xffffff8008f7e390 e si legge dalle rilocazioni, perche'
 * il kernel e' CONFIG_RELOCATABLE=y e i puntatori valgono zero nell'immagine:
 *
 *   +8   0xffffff8008297dd8  ->  seq_lseek       (.llseek)
 *   +16  0xffffff8008297730  ->  seq_read        (.read)
 *   +96  0xffffff8008a5159c  ->  wtk_tp_info_open (.open)
 *   +112 0xffffff8008298798  ->  single_release  (.release)
 *
 * e tutti gli altri campi non sono relocati, cioe' sono NULL -- compreso
 * `.owner` a +0.
 */
static const struct file_operations wtk_tp_info_fops = {
	.open = wtk_tp_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/*
 * "52802481 mov"@0xffffff8008a51574 = 0x124 = 0444, "aa1f03e2 mov"@0xffffff8008a51578
 * mette NULL come genitore, e "910e5c00 add"@0xffffff8008a5156c mette
 * "wtk_tpInfo"@0xffffff800923d397. La prova del ritorno e' "b5000080 cbnz"
 * @0xffffff8008a51580, che SALTA la `printk` quando il puntatore non e' nullo:
 * il messaggio "create /proc/wtk_tpInfo fail\n"@0xffffff800923d3a2 e'
 * il ramo d'errore. "2a1f03e0 mov"@0xffffff8008a51590 rende 0 in tutti e due
 * i casi.
 */
int wtk_creat_proc_tp_info(void)
{
	if (!proc_create("wtk_tpInfo", 0444, NULL, &wtk_tp_info_fops))
		printk("create /proc/wtk_tpInfo fail\n");
	return 0;
}

static void tpd_create_attributes(struct device *dev, struct tpd_attrs *attrs)
{
	int num = attrs->num;

	for (; num > 0;) {
		if (device_create_file(dev, attrs->attr[--num]))
			pr_info("mtk_tpd: tpd create attributes file failed\n");
	}
}

/* touch panel probe */
static int tpd_probe(struct platform_device *pdev)
{
	int touch_type = 1;	/* 0:R-touch, 1: Cap-touch */
	int i = 0;
#ifndef CONFIG_CUSTOM_LCM_X
#ifdef CONFIG_LCM_WIDTH
	unsigned long tpd_res_x = 0, tpd_res_y = 0;
	int ret = 0;
#endif
#endif

	TPD_DMESG("enter %s, %d\n", __func__, __LINE__);

	if (misc_register(&tpd_misc_device))
		pr_info("mtk_tpd: tpd_misc_device register failed\n");
	tpd_get_gpio_info(pdev);
	tpd = kmalloc(sizeof(struct tpd_device), GFP_KERNEL);
	if (tpd == NULL)
		return -ENOMEM;
	memset(tpd, 0, sizeof(struct tpd_device));

	/* allocate input device */
	tpd->dev = input_allocate_device();
	if (tpd->dev == NULL) {
		kfree(tpd);
		return -ENOMEM;
	}
	/* TPD_RES_X = simple_strtoul(LCM_WIDTH, NULL, 0); */
	/* TPD_RES_Y = simple_strtoul(LCM_HEIGHT, NULL, 0); */

	#ifdef CONFIG_MTK_LCM_PHYSICAL_ROTATION
	if (strncmp(CONFIG_MTK_LCM_PHYSICAL_ROTATION, "90", 2) == 0
		|| strncmp(CONFIG_MTK_LCM_PHYSICAL_ROTATION, "270", 3) == 0) {
#ifdef CONFIG_MTK_FB
/*Fix build errors,as some projects  cannot support these apis while bring up*/
		TPD_RES_Y = DISP_GetScreenWidth();
		TPD_RES_X = DISP_GetScreenHeight();
#endif
	} else
    #endif
	{
#ifdef CONFIG_CUSTOM_LCM_X
#ifndef CONFIG_FPGA_EARLY_PORTING
#if defined(CONFIG_MTK_FB) && defined(CONFIG_MTK_LCM)
/*Fix build errors,as some projects  cannot support these apis while bring up*/
		TPD_RES_X = DISP_GetScreenWidth();
		TPD_RES_Y = DISP_GetScreenHeight();
#else
/*for some projects, we do not use mtk framebuffer*/
	TPD_RES_X = tpd_dts_data.tpd_resolution[0];
	TPD_RES_Y = tpd_dts_data.tpd_resolution[1];
#endif
#endif
#else
#ifdef CONFIG_LCM_WIDTH
		ret = kstrtoul(CONFIG_LCM_WIDTH, 0, &tpd_res_x);
		if (ret < 0) {
			pr_info("Touch down get lcm_x failed");
			return ret;
		}
		TPD_RES_X = tpd_res_x;
		ret = kstrtoul(CONFIG_LCM_HEIGHT, 0, &tpd_res_x);
		if (ret < 0) {
			pr_info("Touch down get lcm_y failed");
			return ret;
		}
		TPD_RES_Y = tpd_res_y;
#endif
#endif
	}

	if (2560 == TPD_RES_X)
		TPD_RES_X = 2048;
	if (1600 == TPD_RES_Y)
		TPD_RES_Y = 1536;
	pr_debug("mtk_tpd: TPD_RES_X = %lu, TPD_RES_Y = %lu\n",
		TPD_RES_X, TPD_RES_Y);

	tpd_mode = TPD_MODE_NORMAL;
	tpd_mode_axis = 0;
	tpd_mode_min = TPD_RES_Y / 2;
	tpd_mode_max = TPD_RES_Y;
	tpd_mode_keypad_tolerance = TPD_RES_X * TPD_RES_X / 1600;
	/* struct input_dev dev initialization and registration */
	tpd->dev->name = TPD_DEVICE;
	set_bit(EV_ABS, tpd->dev->evbit);
	set_bit(EV_KEY, tpd->dev->evbit);
	set_bit(ABS_X, tpd->dev->absbit);
	set_bit(ABS_Y, tpd->dev->absbit);
	set_bit(ABS_PRESSURE, tpd->dev->absbit);
#if !defined(CONFIG_MTK_S3320) && !defined(CONFIG_MTK_S3320_47)\
	&& !defined(CONFIG_MTK_S3320_50) && !defined(CONFIG_MTK_MIT200) \
	&& !defined(CONFIG_TOUCHSCREEN_SYNAPTICS_S3528) \
	&& !defined(CONFIG_MTK_S7020) \
	&& !defined(CONFIG_TOUCHSCREEN_MTK_SYNAPTICS_3320_50)
	set_bit(BTN_TOUCH, tpd->dev->keybit);
#endif /* CONFIG_MTK_S3320 */
	set_bit(INPUT_PROP_DIRECT, tpd->dev->propbit);

	/* save dev for regulator_get() before tpd_local_init() */
	tpd->tpd_dev = &pdev->dev;
	for (i = 1; i < TP_DRV_MAX_COUNT; i++) {
		/* add tpd driver into list */
		if (tpd_driver_list[i].tpd_device_name != NULL) {
			tpd_driver_list[i].tpd_local_init();
			/* msleep(1); */
			if (tpd_load_status == 1) {
				TPD_DMESG("%s, tpd_driver_name=%s\n", __func__,
					  tpd_driver_list[i].tpd_device_name);
				g_tpd_drv = &tpd_driver_list[i];
				break;
			}
		}
	}
	if (g_tpd_drv == NULL) {
		if (tpd_driver_list[0].tpd_device_name != NULL) {
			g_tpd_drv = &tpd_driver_list[0];
			/* touch_type:0: r-touch, 1: C-touch */
			touch_type = 0;
			g_tpd_drv->tpd_local_init();
			TPD_DMESG("Generic touch panel driver\n");
		} else {
			TPD_DMESG("no touch driver is loaded!!\n");
			return 0;
		}
	}
	touch_resume_workqueue = create_singlethread_workqueue("touch_resume");
	INIT_WORK(&touch_resume_work, touch_resume_workqueue_callback);
	/* use fb_notifier */
	tpd_fb_notifier.notifier_call = tpd_fb_notifier_callback;
	if (fb_register_client(&tpd_fb_notifier))
		TPD_DMESG("register fb_notifier fail!\n");
	/* TPD_TYPE_CAPACITIVE handle */
	if (touch_type == 1) {

		set_bit(ABS_MT_TRACKING_ID, tpd->dev->absbit);
		set_bit(ABS_MT_TOUCH_MAJOR, tpd->dev->absbit);
		set_bit(ABS_MT_TOUCH_MINOR, tpd->dev->absbit);
		set_bit(ABS_MT_POSITION_X, tpd->dev->absbit);
		set_bit(ABS_MT_POSITION_Y, tpd->dev->absbit);
		input_set_abs_params(tpd->dev,
			ABS_MT_POSITION_X, 0, TPD_RES_X, 0, 0);
		input_set_abs_params(tpd->dev,
			ABS_MT_POSITION_Y, 0, TPD_RES_Y, 0, 0);
#if defined(CONFIG_MTK_S3320) || defined(CONFIG_MTK_S3320_47) \
	|| defined(CONFIG_MTK_S3320_50) || defined(CONFIG_MTK_MIT200) \
	|| defined(CONFIG_TOUCHSCREEN_SYNAPTICS_S3528) \
	|| defined(CONFIG_MTK_S7020) \
	|| defined(CONFIG_TOUCHSCREEN_MTK_SYNAPTICS_3320_50)
		input_set_abs_params(tpd->dev,
		ABS_MT_PRESSURE, 0, 255, 0, 0);
		input_set_abs_params(tpd->dev,
			ABS_MT_WIDTH_MAJOR, 0, 15, 0, 0);
		input_set_abs_params(tpd->dev, ABS_MT_WIDTH_MINOR, 0, 15, 0, 0);
		input_mt_init_slots(tpd->dev, 10, 0);
#else
		input_set_abs_params(tpd->dev,
			ABS_MT_TOUCH_MAJOR, 0, 100, 0, 0);
		input_set_abs_params(tpd->dev,
			ABS_MT_TOUCH_MINOR, 0, 100, 0, 0);
#endif /* CONFIG_MTK_S3320 */
		TPD_DMESG("Cap touch panel driver\n");
	}
	input_set_abs_params(tpd->dev, ABS_X, 0, TPD_RES_X, 0, 0);
	input_set_abs_params(tpd->dev, ABS_Y, 0, TPD_RES_Y, 0, 0);
	input_abs_set_res(tpd->dev, ABS_X, TPD_RES_X);
	input_abs_set_res(tpd->dev, ABS_Y, TPD_RES_Y);
	input_set_abs_params(tpd->dev, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(tpd->dev, ABS_MT_TRACKING_ID, 0, 10, 0, 0);

	if (input_register_device(tpd->dev))
		TPD_DMESG("input_register_device failed.(tpd)\n");
	else
		tpd_register_flag = 1;
	if (g_tpd_drv->tpd_have_button)
		tpd_button_init();

	if (g_tpd_drv->attrs.num)
		tpd_create_attributes(&pdev->dev, &g_tpd_drv->attrs);

	return 0;
}
static int tpd_remove(struct platform_device *pdev)
{
	input_unregister_device(tpd->dev);
	return 0;
}

/* called when loaded into kernel */
static void tpd_init_work_callback(struct work_struct *work)
{
	TPD_DEBUG("MediaTek touch panel driver init\n");
	if (platform_driver_register(&tpd_driver) != 0)
		TPD_DMESG("unable to register touch panel driver.\n");
}
static int __init tpd_device_init(void)
{
	int res = 0;

	tpd_init_workqueue = create_singlethread_workqueue("mtk-tpd");
	INIT_WORK(&tpd_init_work, tpd_init_work_callback);

	res = queue_work(tpd_init_workqueue, &tpd_init_work);
	if (!res)
		pr_info("tpd : touch device init failed res:%d\n", res);
	/* RITRATTAZIONE. La nota qui sopra diceva che wtk_creat_proc_tp_info
	 * non e' chiamata da nessuno, perche' non c'e' nessuna `bl` verso
	 * 0xffffff8008a5155c. La `bl` non c'e' perche' e' INCORPORATA qui: la
	 * fabbrica ha "97bdc883 bl"@0xffffff8009383248 (proc_create) con la
	 * stessa stringa "wtk_tpInfo"@0xffffff800923d397 e gli stessi permessi
	 * ("52802481 mov"@0xffffff8009383240, 0x124 = 0444), seguita da
	 * "97b6c09f bl"@0xffffff8009383258 sul messaggio d'errore. Cercare una
	 * `bl` e non trovarla non dimostra che una funzione non sia chiamata:
	 * dimostra solo che non e' chiamata FUORI LINEA. */
	wtk_creat_proc_tp_info();

	return 0;
}
/* should never be called */
static void __exit tpd_device_exit(void)
{
	TPD_DMESG("MediaTek touch panel driver exit\n");
	/* input_unregister_device(tpd->dev); */
	platform_driver_unregister(&tpd_driver);
}

late_initcall(tpd_device_init);
module_exit(tpd_device_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek touch panel driver");
MODULE_AUTHOR("Kirby Wu<kirby.wu@mediatek.com>");
