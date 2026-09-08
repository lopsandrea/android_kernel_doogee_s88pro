/*
 * Copyright (C) 2017 MediaTek Inc.
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

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#include "imgsensor.h"
#include "imgsensor_proc.h"

char mtk_ccm_name[camera_info_size] = { 0 };
char mtk_i2c_dump[camera_info_size] = { 0 };

static int pdaf_type_info_read(struct seq_file *m, void *v)
{
#define bufsz 512

	unsigned int len = bufsz;
	char pdaf_type_info[bufsz];

	struct SENSOR_FUNCTION_STRUCT *psensor_func =
	    gimgsensor.sensor[IMGSENSOR_SENSOR_IDX_MAIN].pfunc;

	memset(pdaf_type_info, 0, 512);

	if (psensor_func == NULL)
		return 0;

	psensor_func->SensorFeatureControl(
	    SENSOR_FEATURE_GET_PDAF_TYPE,
	    pdaf_type_info,
	    &len);

	seq_printf(m, "%s\n", pdaf_type_info);
	return 0;
};

static int proc_SensorType_open(struct inode *inode, struct file *file)
{
	return single_open(file, pdaf_type_info_read, NULL);
};

static ssize_t proc_SensorType_write(struct file *file,
					const char *buffer, size_t count,
					loff_t *data)
{
	char regBuf[64] = { '\0' };
	u32 u4CopyBufSize =
		(count < (sizeof(regBuf) - 1)) ? (count) : (sizeof(regBuf) - 1);

	struct SENSOR_FUNCTION_STRUCT *psensor_func =
	    gimgsensor.sensor[IMGSENSOR_SENSOR_IDX_MAIN].pfunc;

	if (copy_from_user(regBuf, buffer, u4CopyBufSize))
		return -EFAULT;

	if (psensor_func)
		psensor_func->SensorFeatureControl(
		    SENSOR_FEATURE_SET_PDAF_TYPE,
		    regBuf,
		   &u4CopyBufSize);

	return count;
};

/*******************************************************************************
 * CAMERA_HW_Reg_Debug()
 * Used for sensor register read/write by proc file
 *****************************************************************************/
static ssize_t CAMERA_HW_Reg_Debug(struct file *file,
				const char *buffer, size_t count,
				loff_t *data)
{
	char regBuf[64] = { '\0' };
	u32 u4CopyBufSize =
		(count < (sizeof(regBuf) - 1)) ? (count) : (sizeof(regBuf) - 1);
	struct IMGSENSOR_SENSOR *psensor =
		&gimgsensor.sensor[IMGSENSOR_SENSOR_IDX_MAIN];

	MSDK_SENSOR_REG_INFO_STRUCT sensorReg;

	memset(&sensorReg, 0, sizeof(MSDK_SENSOR_REG_INFO_STRUCT));

	if (psensor == NULL || copy_from_user(regBuf, buffer, u4CopyBufSize))
		return -EFAULT;

	if (sscanf(
		regBuf, "%x %x", &sensorReg.RegAddr, &sensorReg.RegData) == 2) {
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_SET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_GET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));

		PK_DBG("write addr = 0x%08x, data = 0x%08x\n",
			sensorReg.RegAddr,
			sensorReg.RegData);
		snprintf(mtk_i2c_dump, sizeof(mtk_i2c_dump),
			"addr = 0x%08x, data = 0x%08x\n",
			 sensorReg.RegAddr, sensorReg.RegData);

	} else if (kstrtouint(regBuf, 16, &sensorReg.RegAddr) == 0) {
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_GET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		PK_DBG("read addr = 0x%08x, data = 0x%08x\n",
				sensorReg.RegAddr, sensorReg.RegData);
		snprintf(mtk_i2c_dump, sizeof(mtk_i2c_dump),
			"addr = 0x%08x, data = 0x%08x\n",
			 sensorReg.RegAddr, sensorReg.RegData);
	}
	return count;
}


static ssize_t CAMERA_HW_Reg_Debug2(struct file *file, const char *buffer,
					size_t count, loff_t *data)
{
	char regBuf[64] = { '\0' };
	u32 u4CopyBufSize =
		(count < (sizeof(regBuf) - 1)) ? (count) : (sizeof(regBuf) - 1);
	struct IMGSENSOR_SENSOR *psensor =
		&gimgsensor.sensor[IMGSENSOR_SENSOR_IDX_SUB];

	MSDK_SENSOR_REG_INFO_STRUCT sensorReg;

	memset(&sensorReg, 0, sizeof(MSDK_SENSOR_REG_INFO_STRUCT));

	if (psensor == NULL || copy_from_user(regBuf, buffer, u4CopyBufSize))
		return -EFAULT;

	if (sscanf(regBuf, "%x %x",
			&sensorReg.RegAddr, &sensorReg.RegData) == 2) {
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_SET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_GET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		PK_DBG("write addr = 0x%08x, data = 0x%08x\n",
			sensorReg.RegAddr,
			sensorReg.RegData);
		snprintf(mtk_i2c_dump, sizeof(mtk_i2c_dump),
			"addr = 0x%08x, data = 0x%08x\n",
			sensorReg.RegAddr, sensorReg.RegData);

	} else if (kstrtouint(regBuf, 16, &sensorReg.RegAddr) == 0) {
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_GET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		PK_DBG("read addr = 0x%08x, data = 0x%08x\n",
				sensorReg.RegAddr, sensorReg.RegData);
		snprintf(mtk_i2c_dump, sizeof(mtk_i2c_dump),
			"addr = 0x%08x, data = 0x%08x\n",
			sensorReg.RegAddr, sensorReg.RegData);
	}

	return count;
}

static ssize_t CAMERA_HW_Reg_Debug3(struct file *file, const char *buffer,
					size_t count, loff_t *data)
{
	char regBuf[64] = { '\0' };
	u32 u4CopyBufSize =
		(count < (sizeof(regBuf) - 1)) ? (count) : (sizeof(regBuf) - 1);
	struct IMGSENSOR_SENSOR *psensor =
		&gimgsensor.sensor[IMGSENSOR_SENSOR_IDX_MAIN2];

	MSDK_SENSOR_REG_INFO_STRUCT sensorReg;

	memset(&sensorReg, 0, sizeof(MSDK_SENSOR_REG_INFO_STRUCT));

	if (psensor == NULL || copy_from_user(regBuf, buffer, u4CopyBufSize))
		return -EFAULT;

	if (sscanf(regBuf, "%x %x",
			&sensorReg.RegAddr, &sensorReg.RegData) == 2) {
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_SET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_GET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		PK_DBG("write addr = 0x%08x, data = 0x%08x\n",
			sensorReg.RegAddr,
			sensorReg.RegData);
		snprintf(mtk_i2c_dump, sizeof(mtk_i2c_dump),
			"addr = 0x%08x, data = 0x%08x\n",
			 sensorReg.RegAddr, sensorReg.RegData);
	} else if (kstrtouint(regBuf, 16, &sensorReg.RegAddr) == 0) {
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_GET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		PK_DBG("read addr = 0x%08x, data = 0x%08x\n",
				sensorReg.RegAddr, sensorReg.RegData);
		snprintf(mtk_i2c_dump, sizeof(mtk_i2c_dump),
			"addr = 0x%08x, data = 0x%08x\n",
			 sensorReg.RegAddr, sensorReg.RegData);
	}

	return count;
}

static ssize_t CAMERA_HW_Reg_Debug4(struct file *file, const char *buffer,
					size_t count, loff_t *data)
{
	char regBuf[64] = { '\0' };
	u32 u4CopyBufSize =
		(count < (sizeof(regBuf) - 1)) ? (count) : (sizeof(regBuf) - 1);
	struct IMGSENSOR_SENSOR *psensor =
		&gimgsensor.sensor[IMGSENSOR_SENSOR_IDX_SUB2];

	MSDK_SENSOR_REG_INFO_STRUCT sensorReg;

	memset(&sensorReg, 0, sizeof(MSDK_SENSOR_REG_INFO_STRUCT));

	if (psensor == NULL || copy_from_user(regBuf, buffer, u4CopyBufSize))
		return -EFAULT;

	if (sscanf(regBuf, "%x %x",
			&sensorReg.RegAddr, &sensorReg.RegData) == 2) {
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_SET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_GET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		PK_DBG("write addr = 0x%08x, data = 0x%08x\n",
			sensorReg.RegAddr,
			sensorReg.RegData);
		snprintf(mtk_i2c_dump, sizeof(mtk_i2c_dump),
			"addr = 0x%08x, data = 0x%08x\n",
			 sensorReg.RegAddr, sensorReg.RegData);
	} else if (kstrtouint(regBuf, 16, &sensorReg.RegAddr) == 0) {
		imgsensor_sensor_feature_control(psensor,
						SENSOR_FEATURE_GET_REGISTER,
						(MUINT8 *) &sensorReg,
			(MUINT32 *) sizeof(MSDK_SENSOR_REG_INFO_STRUCT));
		PK_DBG("read addr = 0x%08x, data = 0x%08x\n",
					sensorReg.RegAddr, sensorReg.RegData);
		snprintf(mtk_i2c_dump, sizeof(mtk_i2c_dump),
			"addr = 0x%08x, data = 0x%08x\n",
			sensorReg.RegAddr, sensorReg.RegData);
	}

	return count;
}


/* Camera information */
static int subsys_camera_info_read(struct seq_file *m, void *v)
{
	PK_DBG("%s %s\n", __func__, mtk_ccm_name);
	seq_printf(m, "%s\n", mtk_ccm_name);
	return 0;
};

static int subsys_camsensor_read(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", mtk_i2c_dump);
	return 0;
};

static int proc_camera_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, subsys_camera_info_read, NULL);
};

static int proc_camsensor_open(struct inode *inode, struct file *file)
{
	return single_open(file, subsys_camsensor_read, NULL);
};

static int imgsensor_proc_status_info_read(struct seq_file *m, void *v)
{
	char status_info[IMGSENSOR_STATUS_INFO_LENGTH];

	snprintf(status_info, sizeof(status_info), "ERR_L0, %x\n",
			*((uint32_t *)(&gimgsensor.status)));
	seq_printf(m, "%s\n", status_info);
	return 0;
};

static int imgsensor_proc_status_info_open(struct inode *inode,
						struct file *file)
{
	return single_open(file, imgsensor_proc_status_info_read, NULL);
};

static const struct file_operations fcamera_proc_fops1 = {
	.owner = THIS_MODULE,
	.open = proc_camera_info_open,
	.read = seq_read,
};

static const struct file_operations fcamera_proc_fops = {
	.owner = THIS_MODULE,
	.read = seq_read,
	.open = proc_camsensor_open,
	.write = CAMERA_HW_Reg_Debug
};

static const struct file_operations fcamera_proc_fops2 = {
	.owner = THIS_MODULE,
	.read = seq_read,
	.open = proc_camsensor_open,
	.write = CAMERA_HW_Reg_Debug2
};

static const struct file_operations fcamera_proc_fops3 = {
	.owner = THIS_MODULE,
	.read = seq_read,
	.open = proc_camsensor_open,
	.write = CAMERA_HW_Reg_Debug3
};

static const struct file_operations fcamera_proc_fops4 = {
	.owner = THIS_MODULE,
	.read = seq_read,
	.open = proc_camsensor_open,
	.write = CAMERA_HW_Reg_Debug4
};


static const struct file_operations fcamera_proc_fops_set_pdaf_type = {
	.owner = THIS_MODULE,
	.open = proc_SensorType_open,
	.read = seq_read,
	.write = proc_SensorType_write
};

static const struct file_operations fcamera_proc_fops_status_info = {
	.owner = THIS_MODULE,
	.open  = imgsensor_proc_status_info_open,
	.read = seq_read,
};


/*
 * IL NODO /proc/wtk_cameraInfo -- AGGIUNTA DI FABBRICA:
 *   wtk_creat_proc_camera_info @0xffffff8008704e24, 64 byte
 *   wtk_camera_info_open       @0xffffff8008704ffc, 36 byte
 *   wtk_camera_info_show       @0xffffff8008705020, 64 byte
 *
 * DUE DISALLINEAMENTI DI FABBRICA, riprodotti:
 *  - il nodo si chiama "wtk_cameraInfo"@0xffffff80091a5b22 ma il messaggio
 *    d'errore dice "/proc/camera_info_entry"@0xffffff80091a5b31;
 *  - il quarto campo e' etichettato "Main3Camera" ma il buffer che stampa e'
 *    quello che imgsensor_set_driver riempie nel caso SUB2. Il buffer di
 *    main3 (g9917b18) esiste e non viene mostrato.
 */
extern char g9917a98[32];
extern char g9917ab8[32];
extern char g9917ad8[32];
extern char g9917af8[32];

static int wtk_camera_info_show(struct seq_file *m, void *v)
{
	seq_printf(m,
		   "BackCamera:%s,FrontCamera:%s,Main2Camera:%s,Main3Camera:%s\n",
		   g9917a98, g9917ab8, g9917ad8, g9917af8);
	return 0;
}

static int wtk_camera_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, wtk_camera_info_show, NULL);
}

static const struct file_operations wtk_camera_info_fops = {
	.open = wtk_camera_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int wtk_creat_proc_camera_info(void)
{
	if (!proc_create("wtk_cameraInfo", 0444, NULL, &wtk_camera_info_fops))
		printk("create /proc/camera_info_entry fail\n");

	return 0;
}


/*
 * LE DUE CLASSI /sys/class DI WINGTECH, e le tre funzioni che le servono:
 *   camera_main3_yuv_bv_show   @0xffffff8008705c18, 96 byte
 *   camera_yuv_bv_switch_show  @0xffffff8008705c78, 48 byte
 *   camera_yuv_bv_switch_store @0xffffff8008705ca8, 124 byte
 *
 * I permessi sono letti dal campo mode delle due struct class_attribute,
 * che non e' relocato e sta scritto in chiaro: 0x124 a 0xffffff8009917b40
 * (0444) e 0x1a4 a 0xffffff8009917b60 (0644).
 *
 * DUE DISALLINEAMENTI DI FABBRICA, riprodotti:
 *  - la classe si chiama "camera_main3_yuv_bv" ma il suo messaggio d'errore
 *    dice "camera_main2_yuv_bv_class"@0xffffff80091a5bf2;
 *  - la show di main3 legge gimgsensor.sensor[3].pfunc, e l'indice 3 e'
 *    SUB2, non MAIN3. Il calcolo: sensor sta a +336, sizeof e' 128 e pfunc
 *    e' l'ultimo campo (+120), quindi 336 + 128*3 + 120 = 840, che e'
 *    esattamente "f941a508 ldr"@0xffffff8008705c30.
 *
 * I nomi dei tre globali sono i loro indirizzi (regola 5).
 */
static int g9c95260;
static u8 g9c95264;

/*
 * IL PUNTATORE A gimgsensor, e perche' non basta la struttura.
 *
 * Di fabbrica camera_main3_yuv_bv_show fa DUE caricamenti prima della
 * guardia, non uno:
 *
 *   "f9433908 ldr"@0xffffff8008705c28   x8 = *(0xffffff8009917670)
 *   "f941a508 ldr"@0xffffff8008705c30   x8 = *(x8 + 840)
 *
 * Il primo legge un PUNTATORE da un globale in .data; il secondo prende
 * sensor[3].pfunc a partire da li'. Scrivendo `gimgsensor.sensor[3].pfunc`
 * il caricamento e' uno solo -- gimgsensor e' un indirizzo noto a
 * compilazione -- e mancano quattro byte, che erano esattamente lo scarto.
 *
 * Il puntatore non e' `static`: se lo fosse, clang saprebbe che nessuno lo
 * cambia e ripiegherebbe di nuovo sull'indirizzo diretto.
 *
 * Il nome e' l'indirizzo (regola 5): il binario non nomina i dati.
 */
struct IMGSENSOR *g9917670 = &gimgsensor;

static ssize_t camera_main3_yuv_bv_show(struct class *class,
					struct class_attribute *attr, char *buf)
{
	struct SENSOR_FUNCTION_STRUCT *pfunc = g9917670->sensor[3].pfunc;
	int bv;

	/* Il valore reso NON viene riletto dal globale sul ramo in cui la
	 * chiamata c'e' stata: "2a0003e2 mov"@0xffffff8008705c40 lo passa
	 * direttamente da w0, e solo il ramo senza pfunc lo rilegge con
	 * "b9426102 ldr"@0xffffff8008705c54. Scrivendo `g = f(); return
	 * sprintf(..., g)` clang rilegge in tutti e due, ed e' un'istruzione. */
	if (pfunc) {
		bv = pfunc->c48();
		g9c95260 = bv;
	} else {
		bv = g9c95260;
	}

	return sprintf(buf, "%d", bv);
}

static ssize_t camera_yuv_bv_switch_show(struct class *class,
					 struct class_attribute *attr,
					 char *buf)
{
	return sprintf(buf, "%d", g9c95264);
}

static ssize_t camera_yuv_bv_switch_store(struct class *class,
					  struct class_attribute *attr,
					  const char *buf, size_t count)
{
	unsigned long long val;

	if (kstrtoull(buf, 10, &val) < 0)
		return -ENOBUFS;

	g9c95264 = (val == 1);
	return count;
}

/* CLASS_ATTR_RO da' 0444 e CLASS_ATTR_RW 0644: sono esattamente i due modi
 * letti dal binario, quindi le macro giuste sono queste e non una CLASS_ATTR
 * col modo esplicito, che in 4.9 non esiste piu'. */
static CLASS_ATTR_RO(camera_main3_yuv_bv);
static CLASS_ATTR_RW(camera_yuv_bv_switch);

static struct class *g9c95268;
static struct class *g9c95278;

enum IMGSENSOR_RETURN imgsensor_proc_init(void)
{
	memset(mtk_ccm_name, 0, camera_info_size);

	proc_create("driver/camsensor", 0000, NULL, &fcamera_proc_fops);
	proc_create("driver/camsensor2", 0000, NULL, &fcamera_proc_fops2);
	proc_create("driver/camsensor3", 0000, NULL, &fcamera_proc_fops3);
	proc_create("driver/camsensor4", 0000, NULL, &fcamera_proc_fops4);
	proc_create("driver/pdaf_type", 0000, NULL,
				&fcamera_proc_fops_set_pdaf_type);
	proc_create("driver/imgsensor_status_info", 0000, NULL,
				&fcamera_proc_fops_status_info);

	/* Camera information */
	proc_create(PROC_CAMERA_INFO, 0000, NULL, &fcamera_proc_fops1);

	/* L'ULTIMA delle otto: "97efc13e bl"@0xffffff8008704f5c usa la stringa
	 * a 0xffffff80091a5b22, ed e' seguita dalla printk sull'errore. */
	wtk_creat_proc_camera_info();

	/* Le due classi, dopo il nodo: "97f7a5fa bl"@0xffffff8008704f84 e
	 * "97f7a5ea bl"@0xffffff8008704fc4 sono le due __class_create. */
	g9c95268 = class_create(THIS_MODULE, "camera_main3_yuv_bv");
	if (class_create_file(g9c95268, &class_attr_camera_main3_yuv_bv))
		printk("camera_main2_yuv_bv_class create sys interface ERROR\n");

	g9c95278 = class_create(THIS_MODULE, "camera_yuv_bv_switch");
	if (class_create_file(g9c95278, &class_attr_camera_yuv_bv_switch))
		printk("camera_yuv_bv_switch create sys interface ERROR\n");

	return IMGSENSOR_RETURN_SUCCESS;
}
