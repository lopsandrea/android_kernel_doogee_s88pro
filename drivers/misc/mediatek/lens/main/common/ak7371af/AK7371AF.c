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
 * AK7371AF voice coil motor driver
 *
 *
 */

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>

#include "lens_info.h"

#define AF_DRVNAME "AK7371AF_DRV"
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
/*
 * IL CONTATORE DEI TENTATIVI I2C, che ALPS non ha.
 *
 * Sta a 0xffffff8009928f88, in .kernel2 subito dopo g_u4AF_MACRO, e i suoi
 * quattro byte nel binario valgono 05 00 00 00: e' INIZIALIZZATO a 5, non
 * azzerato. Per questo va in .data accanto a g_u4AF_MACRO e non in .bss.
 *
 * Quattro byte, non otto: "str w12"@0xffffff800874beb8 e' una store a 32
 * bit, mentre g_u4AF_MACRO li' accanto e' un unsigned long.
 *
 * Chi lo azzera e' il ramo d'errore di s4AF_WriteReg; chi lo rimette a 5 e'
 * SetI2Cclient. Quando arriva a zero le scritture non partono piu': e' un
 * modo per smettere di parlare a un chip che non risponde, e non c'e' in
 * nessun altro driver di lente di questo albero.
 */
static int g9928f88 = 5;
static unsigned long g_u4CurrPosition;
/*
 * LA POSIZIONE CHIESTA, tenuta a parte da quella corrente. Sta a
 * 0xffffff8009c96d20, otto byte subito dopo g_u4CurrPosition, e in ALPS non
 * c'e': moveAF passa direttamente il suo argomento a setVCMPos.
 *
 * Di fabbrica l'argomento viene prima depositato qui sotto spin_lock
 * ("f9000ee13 str"), e da qui rileggono sia le due scritture I2C sia
 * l'assegnazione finale a g_u4CurrPosition. Il perche' non si legge nel
 * binario; quel che si legge e' che il valore passa da un globale.
 */
static unsigned long g9c96d20;

static int s4AF_ReadReg(u8 a_uAddr, u16 *a_pu2Result)
{
	int i4RetValue = 0;
	char pBuff;
	char puSendCmd[1];

	puSendCmd[0] = a_uAddr;

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 1);

	if (i4RetValue < 0) {
		LOG_INF("I2C read - send failed!!\n");
		return -1;
	}

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, &pBuff, 1);

	if (i4RetValue < 0) {
		LOG_INF("I2C read - recv failed!!\n");
		return -1;
	}
	*a_pu2Result = pBuff;

	return 0;
}

static int s4AF_WriteReg(u16 a_u2Addr, u16 a_u2Data)
{
	int i4RetValue = 0;

	char puSendCmd[2] = {(char)a_u2Addr, (char)a_u2Data};

	if (g9928f88 == 0)
		return -1;

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 2);

	if (i4RetValue < 0) {
		if (g9928f88)
			g9928f88--;
		LOG_INF("I2C write failed!!\n");
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

		unsigned short data = 0;

		/*
		 * L'ESITO SI PROPAGA. ALPS scarta il valore di ritorno di
		 * s4AF_WriteReg; qui un fallimento esce subito, e il
		 * chiamante se ne accorge. Il messaggio e' in chiaro:
		 * "AK7371AF_DRV [%s] InitDrv Fail!! I2C error occurred"@0xffffff80091aecec
		 */
		/* 00:active mode , 10:Standby mode , x1:Sleep mode */
		if (s4AF_WriteReg(0x02, 0x00) != 0) {
			LOG_INF("InitDrv Fail!! I2C error occurred\n");
			return -1;
		}

		msleep(20);

		/*
		 * IL 2 SI SCRIVE SOLO SE IL CHIP HA RISPOSTO. ALPS lo mette
		 * comunque; qui ci vogliono due condizioni -- la lettura
		 * deve tornare 0 E il dato deve essere 0 -- e sono due salti
		 * distinti nel binario, non un && ripiegato in uno.
		 */
		if (s4AF_ReadReg(0x02, &data) != 0 || data != 0) {
			LOG_INF("InitDrv Fail!! I2C error occurred\n");
			return -1;
		}

		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 2;
		spin_unlock(g_pAF_SpinLock);
	}

	LOG_INF("-\n");

	return 0;
}

static inline int setVCMPos(unsigned long a_u4Position)
{
	int i4RetValue = 0;

	i4RetValue = s4AF_WriteReg(0x0, (u16)((a_u4Position >> 2) & 0xff));

	if (i4RetValue < 0)
		return -1;

	i4RetValue = s4AF_WriteReg(0x1, (u16)((a_u4Position & 0x3) << 6));

	return i4RetValue;
}

/* moveAF only use to control moving the motor */
static inline int moveAF(unsigned long a_u4Position)
{
	int ret = 0;
	unsigned short alta = 0, bassa = 0;

	/*
	 * IL CONTROLLO D'INTERVALLO, che ALPS non ha in questo driver ma ha
	 * in DW9714AF. Due confronti senza segno contro i due globali:
	 * "eb13011f cmp"@0xffffff800874b53c contro g_u4AF_MACRO e il gemello
	 * contro g_u4AF_INF, tutti e due prima di qualunque chiamata.
	 * L'esito d'errore e' -22, cioe' -EINVAL.
	 */
	if (a_u4Position > g_u4AF_MACRO || a_u4Position < g_u4AF_INF) {
		LOG_INF("out of range\n");
		return -EINVAL;
	}

	/*
	 * L'ACCENSIONE E' QUI, non in SetI2Cclient. Di fabbrica il blocco di
	 * initAF sta dentro moveAF, che a sua volta viene incorporata
	 * nell'Ioctl: nell'Ioctl non c'e' nessun `bl` verso initAF, e
	 * SetI2Cclient e' 44 byte senza chiamate.
	 */
	if (initAF() != 0)
		return -1;

	/*
	 * LA POSIZIONE SI RILEGGE DAL CHIP prima di muovere, e non e' un
	 * controllo: e' proprio da qui che g_u4CurrPosition prende il suo
	 * valore. Due letture e un innesto di due bit
	 * ("d37e1d15 ubfiz"@0xffffff800874b774 seguito da
	 * "b3461d35 bfxil"@0xffffff800874b778, che e' un field insert e non
	 * una or). Se la seconda lettura fallisce si azzera.
	 */
	s4AF_ReadReg(0x00, &alta);
	ret = s4AF_ReadReg(0x01, &bassa);

	if (ret != 0) {
		spin_lock(g_pAF_SpinLock);
		g_u4CurrPosition = 0;
		spin_unlock(g_pAF_SpinLock);
	} else {
		spin_lock(g_pAF_SpinLock);
		g_u4CurrPosition = ((alta & 0xff) << 2) | ((bassa >> 6) & 0x3);
		spin_unlock(g_pAF_SpinLock);
	}

	/* Se il motore e' gia' li', non si muove niente. */
	if (g_u4CurrPosition == a_u4Position)
		return 0;

	spin_lock(g_pAF_SpinLock);
	g9c96d20 = a_u4Position;
	spin_unlock(g_pAF_SpinLock);

	if (setVCMPos(g9c96d20) == 0) {
		spin_lock(g_pAF_SpinLock);
		g_u4CurrPosition = g9c96d20;
		spin_unlock(g_pAF_SpinLock);
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
long AK7371AF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command,
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
int AK7371AF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");

	if (*g_pAF_Opened == 2) {
		LOG_INF("Wait\n");
		s4AF_WriteReg(0x02, 0x20);
		msleep(20);
	}

	if (*g_pAF_Opened) {
		LOG_INF("Free\n");

		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 0;
		spin_unlock(g_pAF_SpinLock);
	}

	LOG_INF("End\n");

	return 0;
}

int AK7371AF_PowerDown(struct i2c_client *pstAF_I2Cclient,
			int *pAF_Opened)
{
	/*
	 * I DUE PARAMETRI NON SI USANO, ed e' misurato: nel prologo di
	 * fabbrica non c'e' nessuna store verso g_pstAF_I2Cclient ne' verso
	 * g_pAF_Opened. La funzione legge il globale gia' impostato da
	 * SetI2Cclient ("f9467d08 ldr"@0xffffff800874bbc4 lo prende dal
	 * globale, non da x1).
	 *
	 * La firma resta con i due parametri perche' e' quella di
	 * lens_list.h, uguale per tutti i driver di lente.
	 */
	LOG_INF("+\n");
	/*
	 * UN RAMO IN PIU', per il caso "gia' aperto". ALPS guarda solo lo
	 * zero; la fabbrica prova prima il 2 e in quel caso riporta lo stato
	 * a 1 senza spegnere niente. Il messaggio e' in chiaro nel binario:
	 * "AK7371AF_DRV [%s] reopen driver init"@0xffffff80091aec10.
	 *
	 * Il 1 si scrive con una `str` nuda, senza spin_lock -- al contrario
	 * dello zero in Release, che il lucchetto ce l'ha.
	 */
	if (*g_pAF_Opened == 2) {
		*g_pAF_Opened = 1;
		LOG_INF("reopen driver init\n");
	} else if (*g_pAF_Opened == 0) {
		unsigned short data = 0;
		/*
		 * IL CONTATORE SCENDE, non sale: parte da 1
		 * ("320003fa orr"@0xffffff800874bbe0) e la condizione d'uscita
		 * si prova PRIMA del confronto sul dato
		 * ("cbz w26"@0xffffff800874bc60 viene prima di
		 * "cmp w25, #0x20"). Il numero di giri e' lo stesso di ALPS
		 * -- al piu' due -- ma l'ordine dei due controlli no, e con
		 * `data == 0x20 || cnt == 1` il codice non combacia.
		 */
		int cnt = 1;

		while (1) {
			data = 0;

			s4AF_WriteReg(0x02, 0x20);

			s4AF_ReadReg(0x02, &data);

			LOG_INF("Addr : 0x02 , Data : %x\n", data);

			if (cnt == 0 || data == 0x20)
				break;

			cnt--;
		}
	}
	LOG_INF("-\n");

	return 0;
}

int AK7371AF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient,
			  spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_SpinLock = pAF_SpinLock;
	g_pAF_Opened = pAF_Opened;

	/*
	 * NIENTE initAF QUI. Di fabbrica questa funzione e' 44 byte: tre
	 * store di puntatore, il contatore a 5, e `orr w0, wzr, #0x1` per il
	 * valore di ritorno. Nessun bl. E' lo stesso nodo di LC898122AF --
	 * l'accensione si e' spostata dentro l'Ioctl, che infatti e' il
	 * doppio.
	 */
	g9928f88 = 5;

	return 1;
}

int AK7371AF_GetFileName(unsigned char *pFileName)
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
