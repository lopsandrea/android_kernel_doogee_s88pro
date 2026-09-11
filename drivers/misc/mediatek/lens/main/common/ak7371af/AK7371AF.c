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
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009928f88).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_ak7371af_AK7371AF.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int g9928f88 = 5;
static unsigned long g_u4CurrPosition;
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009c96d20).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_ak7371af_AK7371AF.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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
		 * THE OUTCOME PROPAGATES. ALPS discards the return value of
		 * s4AF_WriteReg; here a failure returns at once, and the
		 * caller notices. The message is in plain sight:
		 * "AK7371AF_DRV [%s] InitDrv Fail!! I2C error occurred"@0xffffff80091aecec
		 */
		/* 00:active mode , 10:Standby mode , x1:Sleep mode */
		if (s4AF_WriteReg(0x02, 0x00) != 0) {
			LOG_INF("InitDrv Fail!! I2C error occurred\n");
			return -1;
		}

		msleep(20);

		/*
		 * THE 2 IS WRITTEN ONLY IF THE CHIP ANSWERED. ALPS writes it
		 * regardless; here two conditions are required -- the read
		 * must return 0 AND the datum must be 0 -- and they are two
		 * distinct jumps in the binary, not an && folded into one.
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
	 * THE RANGE CHECK, which ALPS does not have in this driver but does have
	 * in DW9714AF. Two unsigned comparisons against the two globals:
	 * "eb13011f cmp"@0xffffff800874b53c against g_u4AF_MACRO and its twin
	 * against g_u4AF_INF, both before any call.
	 * The error outcome is -22, that is -EINVAL.
	 */
	if (a_u4Position > g_u4AF_MACRO || a_u4Position < g_u4AF_INF) {
		LOG_INF("out of range\n");
		return -EINVAL;
	}

	/*
	 * THE POWER-UP IS HERE, not in SetI2Cclient. In the factory build the initAF
	 * block sits inside moveAF, which in turn is inlined into the Ioctl: in the
	 * Ioctl there is no `bl` towards initAF, and SetI2Cclient is 44 bytes with no
	 * calls.
	 */
	if (initAF() != 0)
		return -1;

	/*
	 * THE POSITION IS READ BACK FROM THE CHIP before moving, and it is not a
	 * check: this is precisely where g_u4CurrPosition gets its value. Two reads
	 * and a two-bit graft ("d37e1d15 ubfiz"@0xffffff800874b774 followed by
	 * "b3461d35 bfxil"@0xffffff800874b778, which is a field insert and not an
	 * or). If the second read fails it is zeroed.
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

	/* If the motor is already there, nothing moves. */
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
	 * LOG_INF() was reconstructed from the factory kernel disassembly (0xffffff800874bbc4).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_ak7371af_AK7371AF.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	LOG_INF("+\n");
	/*
	 * ONE MORE BRANCH, for the "already open" case. ALPS looks only at
	 * zero; the factory tests 2 first and in that case brings the state
	 * back to 1 without powering anything down. The message is in plain sight
	 * in the binary:
	 * "AK7371AF_DRV [%s] reopen driver init"@0xffffff80091aec10.
	 *
	 * The 1 is written with a bare `str`, without a spin_lock -- unlike
	 * the zero in Release, which does have the lock.
	 */
	if (*g_pAF_Opened == 2) {
		*g_pAF_Opened = 1;
		LOG_INF("reopen driver init\n");
	} else if (*g_pAF_Opened == 0) {
		unsigned short data = 0;
		/*
		 * THE COUNTER COUNTS DOWN, not up: it starts at 1
		 * ("320003fa orr"@0xffffff800874bbe0) and the exit condition
		 * is tested BEFORE the comparison on the datum
		 * ("cbz w26"@0xffffff800874bc60 comes before
		 * "cmp w25, #0x20"). The number of rounds is the same as ALPS's
		 * -- at most two -- but the order of the two checks is not, and with
		 * `data == 0x20 || cnt == 1` the code does not match.
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
	 * NO initAF HERE. In the factory build this function is 44 bytes: three
	 * pointer stores, the counter set to 5, and `orr w0, wzr, #0x1` for the
	 * return value. No bl. It is the same pattern as LC898122AF -- the power-up
	 * has moved into the Ioctl, which is indeed twice the size.
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
