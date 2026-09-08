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

/*
 * DW9714AF voice coil motor driver
 *
 *
 */

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>

#include "lens_info.h"

#define AF_DRVNAME "DW9714AF_DRV"
#define AF_I2C_SLAVE_ADDR 0x18

#define AF_DEBUG
#ifdef AF_DEBUG
#define LOG_INF(format, args...)                                               \
	pr_debug(AF_DRVNAME " [%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif

static struct i2c_client *g_pstAF_I2Cclient;
static int *g_pAF_Opened;
static spinlock_t *g_pAF_SpinLock;

static unsigned long g_u4AF_INF;
static unsigned long g_u4AF_MACRO = 1023;
static unsigned long g_u4CurrPosition;

#if 0
static int s4AF_ReadReg(unsigned short *a_pu2Result)
{
	int i4RetValue = 0;
	char pBuff[2];

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, pBuff, 2);

	if (i4RetValue < 0) {
		LOG_INF("I2C read failed!!\n");
		return -1;
	}

	*a_pu2Result = (((u16)pBuff[0]) << 4) + (pBuff[1] >> 4);

	return 0;
}
#endif

static int s4AF_WriteReg(u16 a_u2Data)
{
	int i4RetValue = 0;

	char puSendCmd[2] = {(char)(a_u2Data >> 4),
			     (char)((a_u2Data & 0xF) << 4)};

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 2);

	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return -1;
	}

	return 0;
}

static inline int getAFInfo(__user struct stAF_MotorInfo *pstMotorInfo)
{
	struct stAF_MotorInfo stMotorInfo;

	stMotorInfo.u4MacroPosition = g_u4AF_MACRO;
	stMotorInfo.u4InfPosition = g_u4AF_INF;
	stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
	stMotorInfo.bIsSupportSR = 1;

	stMotorInfo.bIsMotorMoving = 1;

	if (*g_pAF_Opened >= 1)
		stMotorInfo.bIsMotorOpen = 1;
	else
		stMotorInfo.bIsMotorOpen = 0;

	if (copy_to_user(pstMotorInfo, &stMotorInfo,
			 sizeof(struct stAF_MotorInfo)))
		LOG_INF("copy to user failed when getting motor information\n");

	return 0;
}

/* initAF include driver initialization and standby mode */
static int initAF(void)
{
	LOG_INF("+\n");

	if (*g_pAF_Opened == 1) {

		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 2;
		spin_unlock(g_pAF_SpinLock);
	}

	LOG_INF("-\n");

	return 0;
}

/* moveAF only use to control moving the motor */
static inline int moveAF(unsigned long a_u4Position)
{
	int ret = 0;

	if (s4AF_WriteReg((unsigned short)a_u4Position) == 0) {
		g_u4CurrPosition = a_u4Position;
		ret = 0;
	} else {
		LOG_INF("set I2C failed when moving the motor\n");
		ret = -1;
	}

	return ret;
}

static inline int setAFInf(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_INF = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

static inline int setAFMacro(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_MACRO = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	return 0;
}

/* ////////////////////////////////////////////////////////////// */
/*
 * DW9714AF_PowerDown, aggiunta di fabbrica -- e SENZA il suffisso _Main.
 *
 * stock.map: 0xffffff800873a460 DW9714AF_PowerDown, 148 byte, subito prima di
 * DW9714AF_Ioctl_Main: e' la prima funzione emessa di questo file.
 *
 * IL NOME NON E' UN ERRORE DI TRASCRIZIONE. Ogni altra funzione di ogni altro
 * driver di lente porta il suffisso -- AK7371AF_PowerDown_Main,
 * bu64748af_PowerDown_Main, DW9718SAF_PowerDown_Main, LC898217AF_PowerDown_Main
 * -- perche' lens_list.h le rinomina con una #define. Doogee ha aggiunto la
 * funzione a questo file e NON ha aggiunto la riga a lens_list.h, e il
 * binario lo dice: il simbolo e' nudo. Si riproduce (regola 7).
 *
 * COSA FA, letto dalle costanti:
 *
 *   "321903e8 orr"@0xffffff800873a488   w8 = 0x80
 *   "79000be8 strh"@0xffffff800873a48c  scritto come u16 a [sp,#4], cioe' i
 *                                       due byte {0x80, 0x00} di puSendCmd
 *   "321e07e8 orr"@0xffffff800873a490   w8 = 0xc, AF_I2C_SLAVE_ADDR >> 1
 *
 * s4AF_WriteReg compone puSendCmd come {a_u2Data >> 4, (a_u2Data & 0xF) << 4}:
 * {0x80, 0x00} si risolve all'indietro in a_u2Data = 0x800, unico valore
 * dentro l'intervallo di dieci bit della posizione.
 *
 * NESSUNO LA CHIAMA, nemmeno di fabbrica: cercando un `bl` verso
 * 0xffffff800873a460 in tutto stock.elf non se ne trova neanche uno, e
 * AF_PowerDown chiama LC898217AF_PowerDown_Main, AK7371AF_PowerDown_Main e
 * MAIN2AF_PowerDown ("94002a21 bl"@0xffffff8008737ce4 e le due dopo). Resta
 * perche' e' globale, non perche' serva.
 *
 * Per questo LA FIRMA NON E' MISURABILE: il corpo non legge nessun registro
 * d'argomento e nessun chiamante la vincola. Si scrive `void`, che e' la
 * forma minima compatibile con quel che il binario mostra; se un giorno
 * saltasse fuori un chiamante, sara' lui a dire la firma vera.
 */
int DW9714AF_PowerDown(void)
{
	s4AF_WriteReg(0x800);

	return 0;
}

long DW9714AF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command,
		    unsigned long a_u4Param)
{
	long i4RetValue = 0;

	switch (a_u4Command) {
	case AFIOC_G_MOTORINFO:
		i4RetValue =
			getAFInfo((__user struct stAF_MotorInfo *)(a_u4Param));
		break;

	case AFIOC_T_MOVETO:
		i4RetValue = moveAF(a_u4Param);
		break;

	case AFIOC_T_SETINFPOS:
		i4RetValue = setAFInf(a_u4Param);
		break;

	case AFIOC_T_SETMACROPOS:
		i4RetValue = setAFMacro(a_u4Param);
		break;

	default:
		LOG_INF("No CMD\n");
		i4RetValue = -EPERM;
		break;
	}

	return i4RetValue;
}

/* Main jobs: */
/* 1.Deallocate anything that "open" allocated in private_data. */
/* 2.Shut down the device on last close. */
/* 3.Only called once on last time. */
/* Q1 : Try release multiple times. */
int DW9714AF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");

	if (*g_pAF_Opened == 2) {
		LOG_INF("Wait\n");

		/*
		 * LA RAMPA DI RIENTRO, che ALPS non ha: cinque posizioni in
		 * discesa con quindici millisecondi in mezzo, invece di una
		 * scrittura sola.
		 *
		 * I valori si ricavano all'indietro dai byte mandati, perche'
		 * s4AF_WriteReg compone puSendCmd come
		 * {a_u2Data >> 4, (a_u2Data & 0xF) << 4}:
		 *
		 *   {0x12,0xc0} -> 0x12c    {0x0f,0xa0} -> 0x0fa
		 *   {0x0c,0x80} -> 0x0c8    {0x09,0x60} -> 0x096
		 *   {0x06,0x40} -> 0x064
		 *
		 * cioe' 300, 250, 200, 150, 100: passi da cinquanta.
		 */
		s4AF_WriteReg(0x12C);
		msleep(15);
		s4AF_WriteReg(0x0FA);
		msleep(15);
		s4AF_WriteReg(0x0C8);
		msleep(15);
		s4AF_WriteReg(0x096);
		msleep(15);
		s4AF_WriteReg(0x064);
		msleep(15);
	}

	if (*g_pAF_Opened) {
		LOG_INF("Free\n");

		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 0;
		spin_unlock(g_pAF_SpinLock);
	}

	/*
	 * FUORI DA TUTTI E DUE GLI `if`: "cbz"@0xffffff800873a8ac salta il
	 * secondo blocco e atterra qui lo stesso.
	 *
	 * E QUESTA E' LA CHIAMATA CHE MANCAVA A DW9714AF_PowerDown. Nella
	 * nota di quella funzione avevo scritto che non la chiama nessuno,
	 * "perche' non c'e' un `bl` verso 0xffffff800873a460" -- ed e'
	 * proprio la deduzione sbagliata che questo progetto si e' gia'
	 * annotata una volta: una chiamata incorporata non lascia nessuna
	 * `bl`. Il corpo che segue e' identico a quello di PowerDown, e
	 * scriverlo come chiamata e' l'unica lettura che spiega perche' quella
	 * funzione esista.
	 */
	DW9714AF_PowerDown();
	msleep(10);

	LOG_INF("End\n");

	return 0;
}

int DW9714AF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient,
			  spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_SpinLock = pAF_SpinLock;
	g_pAF_Opened = pAF_Opened;

	initAF();

	return 1;
}

int DW9714AF_GetFileName(unsigned char *pFileName)
{
	#if SUPPORT_GETTING_LENS_FOLDER_NAME
	char FilePath[256];
	char *FileString;

	sprintf(FilePath, "%s", __FILE__);
	FileString = strrchr(FilePath, '/');
	*FileString = '\0';
	FileString = (strrchr(FilePath, '/') + 1);
	strncpy(pFileName, FileString, AF_MOTOR_NAME);
	LOG_INF("FileName : %s\n", pFileName);
	#else
	pFileName[0] = '\0';
	#endif
	return 1;
}
