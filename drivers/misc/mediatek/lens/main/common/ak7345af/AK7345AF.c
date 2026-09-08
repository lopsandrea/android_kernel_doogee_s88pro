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
 * AK7345AF voice coil motor driver
 *
 *
 */

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>

#include "lens_info.h"

#define AF_DRVNAME "AK7345AF_DRV"
#define AF_I2C_SLAVE_ADDR 0x18

#define AF_DEBUG
#ifdef AF_DEBUG
/*
 * pr_info, non pr_debug -- e non e' una preferenza, e' una misura.
 *
 * Tutte e cinque le funzioni di fabbrica di questo file chiamano `printk`:
 *
 *   AK7345AF_Ioctl_Main        printk i2c_master_send _raw_spin_lock ...
 *   AK7345AF_Release_Main      printk msleep _raw_spin_lock _raw_spin_unlock
 *   AK7345AF_PowerDown_Main    printk i2c_master_send i2c_master_recv ...
 *   AK7345AF_SetI2Cclient_Main printk i2c_master_send _raw_spin_lock ...
 *   AK7345AF_GetFileName_Main  strrchr strncpy printk __stack_chk_fail
 *
 * pr_debug con CONFIG_DYNAMIC_DEBUG acceso -- e da noi lo e' -- diventa
 * __dynamic_pr_debug, non printk: ogni sito porta con se' un descrittore e il
 * controllo che lo legge, e sono 24 byte a funzione.
 *
 * IL CONFRONTO CHE LO DIMOSTRA e' il gemello ak7371af, stesso mestiere e
 * stesso autore: li' la fabbrica chiama __dynamic_pr_debug, come noi. Quindi
 * non e' una scelta di progetto ne' un effetto della configurazione -- e' una
 * differenza di QUESTO FILE, e va riprodotta qui e non altrove.
 */
#define LOG_INF(format, args...)                                               \
	pr_info(AF_DRVNAME " [%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif

static struct i2c_client *g_pstAF_I2Cclient;
static int *g_pAF_Opened;
static spinlock_t *g_pAF_SpinLock;

static unsigned long g_u4AF_INF;
static unsigned long g_u4AF_MACRO = 1023;
static unsigned long g_u4CurrPosition;

static int s4AF_ReadReg(u16 a_u2Addr, u16 *a_pu2Result)
{
	int i4RetValue = 0;
	char pBuff;
	/*
	 * IL BUFFER DI UN BYTE, e perche' e' un byte e non due.
	 *
	 * Questo file non e' mai stato compilato in ALPS -- non sta in nessun
	 * obj-y -- e la riga di sotto, com'era, non compila: passava `&a_u2Addr`
	 * (un `u16 *`) dove i2c_master_send vuole un `const char *`, ed e'
	 * -Werror. Qualcosa doveva cambiare comunque; QUALE cambiamento lo dice
	 * il binario, non il gusto.
	 *
	 * "390053f4 strb"@0xffffff800874c394 e' una scrittura di UN BYTE a
	 * [sp,#20]. Un `(char *)&a_u2Addr` avrebbe dovuto lasciare in pila la
	 * mezza parola intera -- il chiamato e' opaco e potrebbe leggere due
	 * byte -- e sarebbe stata una `strh`. Un buffer di un byte solo da'
	 * la `strb` che c'e'.
	 *
	 * E' la stessa forma del gemello ak7371af, che l'albero compila.
	 */
	char puSendCmd[1];

	puSendCmd[0] = a_u2Addr;

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

	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;

	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 2);

	if (i4RetValue < 0) {
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

		int ret = 0;

		/* 00:active mode        10:Standby mode    x1:Sleep mode */
		ret = s4AF_WriteReg(0x02, 0x00);

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

	/*
	 * LA RIGA DI DEBUG LASCIATA DENTRO, e i suoi quindici trattini.
	 *
	 * Dice "set I2C failed" e si stampa a OGNI movimento, riuscito o no:
	 * chi l'ha scritta ha copiato il messaggio dell'else, gli ha aggiunto
	 * i trattini per ritrovarlo a occhio nel log, e non l'ha piu' tolta.
	 * E' un difetto di fabbrica e si riproduce (regola 7).
	 *
	 * Che ci siano DUE stringhe distinte, e non una sola usata due volte,
	 * lo dice il binario: le due printk di moveAF dentro AK7345AF_Ioctl_Main
	 * puntano a due indirizzi diversi di .rodata --
	 * "97e79d33 bl"@0xffffff800874c008 va alla stringa coi trattini,
	 * "bl printk"@0xffffff800874c1e8 a quella senza -- e tutte e due
	 * portano __func__ = "moveAF".
	 *
	 * I trattini sono quindici, contati sui byte:
	 *   "he motor--------"@0xffffff80091af03c  (otto)
	 *   "-------\n\0"@0xffffff80091af04c       (altri sette, poi fine)
	 */
	LOG_INF("set I2C failed when moving the motor---------------\n");

	s4AF_WriteReg(0x02, 0x00);

	if (s4AF_WriteReg(0x0, (u16)((a_u4Position >> 2) & 0xff)) == 0 &&
	    s4AF_WriteReg(0x1, (u16)(((a_u4Position >> 1) & 1) << 7)) ==
		    0) {
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
long AK7345AF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command,
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
int AK7345AF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");

	if (*g_pAF_Opened == 2) {
		LOG_INF("Wait\n");
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

/*
 * AK7345AF_PowerDown, aggiunta di fabbrica.
 *
 * L'ordine e' quello del binario -- Ioctl, Release, PowerDown, SetI2Cclient,
 * GetFileName: stock.map: 0xffffff800874c2cc AK7345AF_PowerDown_Main, 384 byte, fra
 * AK7345AF_Release_Main e AK7345AF_SetI2Cclient_Main.
 *
 * Che sia questa forma e non un'altra lo dicono tre costanti immediate:
 *
 *   "321e07f3 orr"@0xffffff800874c340   w19 = 0xc, cioe' AF_I2C_SLAVE_ADDR
 *                                       (0x18) gia' diviso per due
 *   "52840059 mov"@0xffffff800874c33c   w25 = 0x2002, che scritto come u16
 *                                       little-endian a [sp,#16] e' la coppia
 *                                       di byte {0x02, 0x20} di puSendCmd --
 *                                       cioe' s4AF_WriteReg(0x02, 0x20)
 *   "390053f4 strb"@0xffffff800874c394  w20 = 2 a [sp,#20], il singolo byte
 *                                       che s4AF_ReadReg manda prima di
 *                                       ricevere: s4AF_ReadReg(0x02, &data)
 *
 * I due aiutanti sono incorporati (nessuna bl verso di loro), ma le loro
 * stringhe di errore restano e portano il proprio __func__: e' cosi' che si
 * vede che sono s4AF_ReadReg e non altro.
 *
 * Il ciclo, "3400009c cbz"@0xffffff800874c3ec e le due righe dopo, e' un
 * conto alla rovescia da 1: clang ha girato `cnt` in `1 - cnt`. Sono al
 * massimo due giri, e il secondo si fa solo se il registro non ha ancora
 * letto 0x20.
 */
int AK7345AF_PowerDown(struct i2c_client *pstAF_I2Cclient,
			int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_Opened = pAF_Opened;

	LOG_INF("+\n");
	if (*g_pAF_Opened == 0) {
		unsigned short data = 0;
		int cnt = 0;

		while (1) {
			data = 0;

			s4AF_WriteReg(0x02, 0x20);

			s4AF_ReadReg(0x02, &data);

			LOG_INF("Addr : 0x02 , Data : %x\n", data);

			if (data == 0x20 || cnt == 1)
				break;

			cnt++;
		}
	}
	LOG_INF("-\n");

	return 0;
}

int AK7345AF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient,
			  spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_SpinLock = pAF_SpinLock;
	g_pAF_Opened = pAF_Opened;

	initAF();

	return 1;
}

int AK7345AF_GetFileName(unsigned char *pFileName)
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
