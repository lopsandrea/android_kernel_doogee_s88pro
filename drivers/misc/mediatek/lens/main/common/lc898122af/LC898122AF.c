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
 * LC898122AF voice coil motor driver
 *
 *
 */

#include "Ois.h"
#include "OisDef.h"
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>

#include "lens_info.h"

#define AF_DRVNAME "LC898122AF_DRV"
#define AF_I2C_SLAVE_ADDR 0x48

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

void RegWriteA_LC898122AF(unsigned short RegAddr, unsigned char RegData)
{
	int i4RetValue = 0;
	char puSendCmd[3] = {(char)((RegAddr >> 8) & 0xFF),
			     (char)(RegAddr & 0xFF), RegData};
	/* LOG_INF("I2C w (%x %x)\n", RegAddr, RegData); */

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 3);
	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return;
	}
}

void RegReadA_LC898122AF(unsigned short RegAddr, unsigned char *RegData)
{
	int i4RetValue = 0;
	char pBuff[2] = {(char)(RegAddr >> 8), (char)(RegAddr & 0xFF)};

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, pBuff, 2);
	if (i4RetValue < 0) {
		LOG_INF("[CAMERA SENSOR] read I2C send failed!!\n");
		return;
	}

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, (u8 *)RegData, 1);

	/* LOG_INF("I2C r (%x %x)\n", RegAddr, *RegData); */
	if (i4RetValue != 1) {
		LOG_INF("[CAMERA SENSOR] I2C read failed!!\n");
		return;
	}
}

void RamWriteA_LC898122AF(unsigned short RamAddr, unsigned short RamData)
{
	int i4RetValue = 0;
	char puSendCmd[4] = {
		(char)((RamAddr >> 8) & 0xFF), (char)(RamAddr & 0xFF),
		(char)((RamData >> 8) & 0xFF), (char)(RamData & 0xFF)};
	/* LOG_INF("I2C w2 (%x %x)\n", RamAddr, RamData); */

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 4);
	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return;
	}
}

void RamReadA_LC898122AF(unsigned short RamAddr, void *ReadData)
{
	int i4RetValue = 0;
	char pBuff[2] = {(char)(RamAddr >> 8), (char)(RamAddr & 0xFF)};
	unsigned short vRcvBuff = 0;
	unsigned long *pRcvBuff;

	pRcvBuff = (unsigned long *)ReadData;

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff8008744830).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_lc898122af_LC898122AF.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, pBuff, 2);
	if (i4RetValue < 0) {
		LOG_INF("[CAMERA SENSOR] read I2C send failed!!\n");
		return;
	}

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, (u8 *)&vRcvBuff, 2);
	if (i4RetValue != 2) {
		LOG_INF("[CAMERA SENSOR] I2C read failed!!\n");
		return;
	}
	*pRcvBuff = ((vRcvBuff & 0xFF) << 8) + ((vRcvBuff >> 8) & 0xFF);

	/* LOG_INF("I2C r2 (%x %x)\n", RamAddr, (unsigned int)*pRcvBuff); */
}

void RamWrite32A_LC898122AF(unsigned short RamAddr, unsigned long RamData)
{
	int i4RetValue = 0;
	char puSendCmd[6] = {
		(char)((RamAddr >> 8) & 0xFF),  (char)(RamAddr & 0xFF),
		(char)((RamData >> 24) & 0xFF), (char)((RamData >> 16) & 0xFF),
		(char)((RamData >> 8) & 0xFF),  (char)(RamData & 0xFF)};
	/* LOG_INF("I2C w4 (%x %x)\n", RamAddr, (unsigned int)RamData); */

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 6);
	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return;
	}
}

void RamRead32A_LC898122AF(unsigned short RamAddr, void *ReadData)
{
	int i4RetValue = 0;
	char pBuff[2] = {(char)(RamAddr >> 8), (char)(RamAddr & 0xFF)};
	unsigned long *pRcvBuff, vRcvBuff = 0;

	pRcvBuff = (unsigned long *)ReadData;

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);

	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, pBuff, 2);
	if (i4RetValue < 0) {
		LOG_INF("[CAMERA SENSOR] read I2C send failed!!\n");
		return;
	}

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, (u8 *)&vRcvBuff, 4);
	if (i4RetValue != 4) {
		LOG_INF("[CAMERA SENSOR] I2C read failed!!\n");
		return;
	}
	*pRcvBuff =
		((vRcvBuff & 0xFF) << 24) + (((vRcvBuff >> 8) & 0xFF) << 16) +
		(((vRcvBuff >> 16) & 0xFF) << 8) + (((vRcvBuff >> 24) & 0xFF));

	/* LOG_INF("I2C r4 (%x %x)\n", RamAddr, (unsigned int)*pRcvBuff); */
}

void WitTim_LC898122AF(unsigned short UsWitTim)
{
	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff800874aa8c).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_lc898122af_LC898122AF.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
}

void LC898prtvalue(unsigned short prtvalue)
{
	LOG_INF("printvalue ======%x\n", prtvalue);
}
/*
 * s4AF_Write_Word_Byte() was reconstructed from the factory kernel disassembly (0xffffff8008744cb8, 156 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_lc898122af_LC898122AF.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void s4AF_Write_Word_Byte(unsigned short a_u2Addr, unsigned char a_u1Data)
{
	char puSendCmd[3] = {(char)(a_u2Addr >> 8), (char)(a_u2Addr & 0xFF),
			     (char)a_u1Data};

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);

	if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 3) < 0)
		LOG_INF("I2C send failed!!\n");
}

void s4AF_Write_Word_Word(unsigned short a_u2Addr, unsigned short a_u2Data)
{
	char puSendCmd[4] = {(char)(a_u2Addr >> 8), (char)(a_u2Addr & 0xFF),
			     (char)(a_u2Data >> 8), (char)(a_u2Data & 0xFF)};

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);

	if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 4) < 0)
		LOG_INF("I2C send failed!!\n");
}

void s4AF_Write_Word_DWord(unsigned short a_u2Addr, unsigned long a_u4Data)
{
	char puSendCmd[6] = {(char)(a_u2Addr >> 8), (char)(a_u2Addr & 0xFF),
			     (char)(a_u4Data >> 24), (char)(a_u4Data >> 16),
			     (char)(a_u4Data >> 8), (char)(a_u4Data & 0xFF)};

	g_pstAF_I2Cclient->addr = (AF_I2C_SLAVE_ADDR >> 1);

	if (i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 6) < 0)
		LOG_INF("I2C send failed!!\n");
}

/*
 * TURNED ON. In ALPS this function sits inside an `#if 0`, and with it the whole
 * initAF block that uses it. The factory has turned both on, and it
 * shows: "94000488 bl"@0xffffff8008745084 and the second at 0xffffff800874509c
 * call exactly this one, inside LC898122AF_Ioctl_Main.
 */
static unsigned char s4LC898OTP_ReadReg(unsigned short RegAddr)
{
	int i4RetValue = 0;
	unsigned char pBuff = (unsigned char)RegAddr;
	unsigned char RegData = 0xFF;

	g_pstAF_I2Cclient->addr = (0xA0 >> 1);
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, &pBuff, 1);
	if (i4RetValue < 0) {
		LOG_INF("[CAMERA SENSOR] read I2C send failed!!\n");
		return 0xff;
	}

	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, &RegData, 1);

	LOG_INF("OTPI2C r (%x %x)\n", RegAddr, RegData);
	if (i4RetValue != 1) {
		LOG_INF("[CAMERA SENSOR] I2C read failed!!\n");
		return 0xff;
	}
	return RegData;
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

/* moveAF only use to control moving the motor */
struct lc898122_setting {
	unsigned short addr;
	unsigned int c4;
	unsigned long data;
	unsigned int width;
	unsigned short delay;
};

static void LC898122_write_settings(unsigned short n,
				    struct lc898122_setting *tbl);

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800991d3f0, 15,248 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_lc898122af_LC898122AF.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct {
	unsigned short n;
	struct lc898122_setting e[635];
} g991d3f0 = {
	635,
	{
		{ 0x02d0, 2, 0x00000001, 1, 0 },
		{ 0x0256, 2, 0x00000000, 1, 0 },
		{ 0x0257, 2, 0x00000090, 1, 0 },
		{ 0x0258, 2, 0x00000000, 1, 0 },
		{ 0x020b, 2, 0x0000001f, 1, 0 },
		{ 0x020c, 2, 0x00000000, 1, 0 },
		{ 0x0210, 2, 0x00000000, 1, 0 },
		{ 0x0211, 2, 0x00000000, 1, 0 },
		{ 0x0212, 2, 0x00000003, 1, 0 },
		{ 0x0213, 2, 0x00000002, 1, 0 },
		{ 0x0214, 2, 0x00000006, 1, 0 },
		{ 0x00a0, 2, 0x00000000, 1, 0 },
		{ 0x0380, 2, 0x00008000, 2, 0 },
		{ 0x0081, 2, 0x00000010, 1, 0 },
		{ 0x0083, 2, 0x00000040, 1, 0 },
		{ 0x0084, 2, 0x00000080, 1, 0 },
		{ 0x0088, 2, 0x00000090, 1, 0 },
		{ 0x0082, 2, 0x00000000, 1, 0 },
		{ 0x0085, 2, 0x00000000, 1, 0 },
		{ 0x0091, 2, 0x00000001, 1, 0 },
		{ 0x0099, 2, 0x00000020, 1, 0 },
		{ 0x00a1, 2, 0x00000008, 1, 0 },
		{ 0x0250, 2, 0x00000000, 1, 0 },
		{ 0x0264, 2, 0x00000000, 1, 0 },
		{ 0x0302, 2, 0x00000000, 1, 0 },
		{ 0x0396, 2, 0x00007fff, 2, 0 },
		{ 0x0398, 2, 0x0000113e, 2, 0 },
		{ 0x039a, 2, 0x00007211, 2, 0 },
		{ 0x0303, 2, 0x000000a9, 1, 0 },
		{ 0x0304, 2, 0x00000004, 1, 0 },
		{ 0x0250, 2, 0x00000080, 1, 0 },
		{ 0x0264, 2, 0x00000005, 1, 0 },
		{ 0x0081, 2, 0x00000010, 1, 0 },
		{ 0x00a0, 2, 0x00000080, 1, 0 },
		{ 0x02d0, 2, 0x00000001, 1, 0 },
		{ 0x0256, 2, 0x00000000, 1, 0 },
		{ 0x0257, 2, 0x00000090, 1, 0 },
		{ 0x0258, 2, 0x00000000, 1, 0 },
		{ 0x020b, 2, 0x0000001f, 1, 0 },
		{ 0x020c, 2, 0x00000000, 1, 0 },
		{ 0x0210, 2, 0x00000000, 1, 0 },
		{ 0x0211, 2, 0x00000000, 1, 0 },
		{ 0x0212, 2, 0x00000003, 1, 0 },
		{ 0x0213, 2, 0x00000002, 1, 0 },
		{ 0x0214, 2, 0x00000006, 1, 0 },
		{ 0x0220, 2, 0x00000000, 1, 0 },
		{ 0x0221, 2, 0x00000000, 1, 0 },
		{ 0x0222, 2, 0x0000000f, 1, 0 },
		{ 0x0223, 2, 0x0000000f, 1, 0 },
		{ 0x0231, 2, 0x00000000, 1, 0 },
		{ 0x0230, 2, 0x00000002, 1, 0 },
		{ 0x0232, 2, 0x00000002, 1, 0 },
		{ 0x0233, 2, 0x00000000, 1, 0 },
		{ 0x0234, 2, 0x00000000, 1, 0 },
		{ 0x0235, 2, 0x00000000, 1, 0 },
		{ 0x0236, 2, 0x00000000, 1, 0 },
		{ 0x0248, 2, 0x00000000, 1, 0 },
		{ 0x0249, 2, 0x00000000, 1, 0 },
		{ 0x028f, 2, 0x00000001, 1, 0 },
		{ 0x0280, 2, 0x00000001, 1, 0 },
		{ 0x0281, 2, 0x00000080, 1, 0 },
		{ 0x0281, 2, 0x00000004, 1, 0 },
		{ 0x0283, 2, 0x0000006a, 1, 0 },
		{ 0x028a, 2, 0x00000010, 1, 0 },
		{ 0x0282, 2, 0x00000010, 1, 0 },
		{ 0x0282, 2, 0x00000000, 1, 2 },
		{ 0x0283, 2, 0x0000001b, 1, 0 },
		{ 0x028a, 2, 0x00000018, 1, 0 },
		{ 0x0282, 2, 0x00000010, 1, 0 },
		{ 0x0282, 2, 0x00000000, 1, 2 },
		{ 0x0281, 2, 0x00000000, 1, 0 },
		{ 0x028b, 2, 0x0000007c, 1, 0 },
		{ 0x0283, 2, 0x00000045, 1, 0 },
		{ 0x0284, 2, 0x00000043, 1, 0 },
		{ 0x0280, 2, 0x00000002, 1, 0 },
		{ 0x0030, 2, 0x00000000, 1, 0 },
		{ 0x0270, 2, 0x0000005c, 1, 0 },
		{ 0x0271, 2, 0x0000005d, 1, 0 },
		{ 0x0272, 2, 0x00000000, 1, 0 },
		{ 0x0273, 2, 0x00000000, 1, 0 },
		{ 0x01c0, 2, 0x00000000, 1, 0 },
		{ 0x01c1, 2, 0x00000000, 1, 0 },
		{ 0x01c2, 2, 0x00000000, 1, 0 },
		{ 0x01c3, 2, 0x00000000, 1, 0 },
		{ 0x01c5, 2, 0x00000004, 1, 0 },
		{ 0x01c4, 2, 0x00000040, 1, 0 },
		{ 0x01c7, 2, 0x00000004, 1, 0 },
		{ 0x01c6, 2, 0x000000c0, 1, 0 },
		{ 0x01c9, 2, 0x00000000, 1, 0 },
		{ 0x01c8, 2, 0x00000000, 1, 0 },
		{ 0x01cb, 2, 0x00000000, 1, 0 },
		{ 0x01ca, 2, 0x00000000, 1, 0 },
		{ 0x0030, 2, 0x00000080, 1, 0 },
		{ 0x0101, 2, 0x00000000, 1, 0 },
		{ 0x0102, 2, 0x00000000, 1, 0 },
		{ 0x018f, 2, 0x00000000, 1, 0 },
		{ 0x018e, 2, 0x00000000, 1, 0 },
		{ 0x0102, 2, 0x00000003, 1, 0 },
		{ 0x0102, 2, 0x00000000, 1, 2 },
		{ 0x0170, 2, 0x00000002, 1, 0 },
		{ 0x0171, 2, 0x00000002, 1, 0 },
		{ 0x018c, 2, 0x00000000, 1, 0 },
		{ 0x1239, 2, 0x3f800000, 4, 0 },
		{ 0x123a, 2, 0x3f800000, 4, 0 },
		{ 0x123b, 2, 0x3f800000, 4, 0 },
		{ 0x123c, 2, 0x3f800000, 4, 0 },
		{ 0x10e6, 2, 0x3f800000, 4, 0 },
		{ 0x11e6, 2, 0x3f800000, 4, 0 },
		{ 0x0178, 2, 0x00000000, 1, 0 },
		{ 0x017a, 2, 0x000000ff, 1, 0 },
		{ 0x10ec, 2, 0x3f800000, 4, 0 },
		{ 0x11ec, 2, 0x3f800000, 4, 0 },
		{ 0x017c, 2, 0x00000000, 1, 0 },
		{ 0x017d, 2, 0x00000006, 1, 0 },
		{ 0x017e, 2, 0x0000000f, 1, 0 },
		{ 0x10ed, 2, 0xbc800000, 4, 0 },
		{ 0x11ed, 2, 0xbc800000, 4, 0 },
		{ 0x10ee, 2, 0x3ae90466, 4, 0 },
		{ 0x11ee, 2, 0x3ae90466, 4, 0 },
		{ 0x0174, 2, 0x00000011, 1, 0 },
		{ 0x10ba, 2, 0x3ec0017f, 4, 0 },
		{ 0x10bb, 2, 0x00000000, 4, 0 },
		{ 0x10bc, 2, 0x3f180130, 4, 0 },
		{ 0x10bd, 2, 0x00000000, 4, 0 },
		{ 0x10be, 2, 0x3f800000, 4, 0 },
		{ 0x11ba, 2, 0x3ec0017f, 4, 0 },
		{ 0x11bb, 2, 0x00000000, 4, 0 },
		{ 0x11bc, 2, 0x3f180130, 4, 0 },
		{ 0x11bd, 2, 0x00000000, 4, 0 },
		{ 0x11be, 2, 0x3f800000, 4, 0 },
		{ 0x10fa, 2, 0x3ec0017f, 4, 0 },
		{ 0x10fb, 2, 0x00000000, 4, 0 },
		{ 0x10fc, 2, 0x3f180130, 4, 0 },
		{ 0x10fd, 2, 0x00000000, 4, 0 },
		{ 0x10fe, 2, 0x3f800000, 4, 0 },
		{ 0x11fa, 2, 0x3ec0017f, 4, 0 },
		{ 0x11fb, 2, 0x00000000, 4, 0 },
		{ 0x11fc, 2, 0x3f180130, 4, 0 },
		{ 0x11fd, 2, 0x00000000, 4, 0 },
		{ 0x11fe, 2, 0x3f800000, 4, 0 },
		{ 0x0180, 2, 0x00000000, 1, 0 },
		{ 0x0181, 2, 0x00000000, 1, 0 },
		{ 0x0182, 2, 0x00000060, 1, 0 },
		{ 0x0183, 2, 0x00000000, 1, 0 },
		{ 0x0184, 2, 0x00000020, 1, 0 },
		{ 0x0188, 2, 0x00000006, 1, 0 },
		{ 0x0190, 2, 0x00000000, 1, 0 },
		{ 0x0191, 2, 0x00000000, 1, 0 },
		{ 0x0192, 2, 0x00000008, 1, 0 },
		{ 0x0193, 2, 0x00000002, 1, 0 },
		{ 0x0194, 2, 0x00000000, 1, 0 },
		{ 0x0195, 2, 0x00000000, 1, 0 },
		{ 0x0196, 2, 0x00000000, 1, 0 },
		{ 0x0197, 2, 0x00000000, 1, 0 },
		{ 0x0198, 2, 0x00000000, 1, 0 },
		{ 0x0199, 2, 0x00000000, 1, 0 },
		{ 0x01a0, 2, 0x00000000, 1, 0 },
		{ 0x01a2, 2, 0x00000008, 1, 0 },
		{ 0x01a3, 2, 0x00000002, 1, 0 },
		{ 0x01a4, 2, 0x00000002, 1, 0 },
		{ 0x01a5, 2, 0x00000000, 1, 0 },
		{ 0x01a6, 2, 0x00000000, 1, 0 },
		{ 0x01a7, 2, 0x00000000, 1, 0 },
		{ 0x01a8, 2, 0x00000000, 1, 0 },
		{ 0x01a9, 2, 0x00000000, 1, 0 },
		{ 0x01b0, 2, 0x00000000, 1, 0 },
		{ 0x01b1, 2, 0x00000000, 1, 0 },
		{ 0x01b2, 2, 0x00000000, 1, 0 },
		{ 0x01b3, 2, 0x00000000, 1, 0 },
		{ 0x01b4, 2, 0x00000000, 1, 0 },
		{ 0x01b5, 2, 0x00000000, 1, 0 },
		{ 0x01b6, 2, 0x00000000, 1, 0 },
		{ 0x01b7, 2, 0x00000000, 1, 0 },
		{ 0x01b8, 2, 0x00000000, 1, 0 },
		{ 0x01b9, 2, 0x00000000, 1, 0 },
		{ 0x01ba, 2, 0x00000000, 1, 0 },
		{ 0x01bb, 2, 0x00000000, 1, 0 },
		{ 0x01bc, 2, 0x00000000, 1, 0 },
		{ 0x01bd, 2, 0x00000000, 1, 0 },
		{ 0x01be, 2, 0x00000000, 1, 0 },
		{ 0x01bf, 2, 0x00000000, 1, 0 },
		{ 0x0105, 2, 0x00000000, 1, 0 },
		{ 0x01ce, 2, 0x000000ff, 1, 0 },
		{ 0x018c, 2, 0x00000000, 1, 0 },
		{ 0x0001, 2, 0x00000030, 1, 0 },
		{ 0x0002, 2, 0x00000090, 1, 0 },
		{ 0x0003, 2, 0x000000ff, 1, 0 },
		{ 0x0004, 2, 0x000000ff, 1, 0 },
		{ 0x0011, 2, 0x0000003d, 1, 0 },
		{ 0x0010, 2, 0x00000000, 1, 0 },
		{ 0x0012, 2, 0x00000004, 1, 0 },
		{ 0x0013, 2, 0x00000004, 1, 0 },
		{ 0x0005, 2, 0x00000000, 1, 0 },
		{ 0x0006, 2, 0x00000000, 1, 0 },
		{ 0x0014, 2, 0x00000000, 1, 0 },
		{ 0x0015, 2, 0x00000000, 1, 0 },
		{ 0x0018, 2, 0x00000000, 1, 0 },
		{ 0x0019, 2, 0x00000000, 1, 0 },
		{ 0x001a, 2, 0x00000000, 1, 0 },
		{ 0x001b, 2, 0x00000000, 1, 0 },
		{ 0x0020, 2, 0x000000c0, 1, 0 },
		{ 0x0022, 2, 0x00000080, 1, 0 },
		{ 0x0250, 2, 0x00000080, 1, 0 },
		{ 0x0110, 2, 0x00000003, 1, 0 },
		{ 0x0107, 2, 0x00000010, 1, 0 },
		{ 0x0117, 2, 0x00000000, 1, 0 },
		{ 0x011c, 2, 0x00000000, 1, 0 },
		{ 0x011b, 2, 0x00000000, 1, 0 },
		{ 0x0118, 2, 0x00000000, 1, 0 },
		{ 0x0119, 2, 0x00000012, 1, 0 },
		{ 0x0116, 2, 0x00000006, 1, 0 },
		{ 0x1028, 2, 0x3dccccc0, 4, 0 },
		{ 0x1128, 2, 0x3dccccc0, 4, 0 },
		{ 0x1029, 2, 0x3ee66666, 4, 0 },
		{ 0x1129, 2, 0x3ee66666, 4, 0 },
		{ 0x102a, 2, 0x3ee66666, 4, 0 },
		{ 0x112a, 2, 0x3ee66666, 4, 0 },
		{ 0x112b, 2, 0x40400000, 4, 0 },
		{ 0x102b, 2, 0x40400000, 4, 0 },
		{ 0x102c, 2, 0x40400000, 4, 0 },
		{ 0x112c, 2, 0x40400000, 4, 0 },
		{ 0x0130, 2, 0x00000012, 1, 0 },
		{ 0x0131, 2, 0x00000009, 1, 0 },
		{ 0x1226, 2, 0x00000000, 4, 0 },
		{ 0x109d, 2, 0x00000000, 4, 0 },
		{ 0x109e, 2, 0x00000000, 4, 0 },
		{ 0x104f, 2, 0x3f800000, 4, 0 },
		{ 0x105f, 2, 0x40000000, 4, 0 },
		{ 0x106f, 2, 0x3ca3d70a, 4, 0 },
		{ 0x107f, 2, 0x3e4ccccd, 4, 0 },
		{ 0x108f, 2, 0x3dcccccd, 4, 0 },
		{ 0x109f, 2, 0x3f000000, 4, 0 },
		{ 0x10af, 2, 0x3c23d70a, 4, 0 },
		{ 0x10bf, 2, 0x3ca3d70a, 4, 0 },
		{ 0x119d, 2, 0x00000000, 4, 0 },
		{ 0x119e, 2, 0x00000000, 4, 0 },
		{ 0x114f, 2, 0x3f800000, 4, 0 },
		{ 0x115f, 2, 0x40000000, 4, 0 },
		{ 0x116f, 2, 0x3ca3d70a, 4, 0 },
		{ 0x117f, 2, 0x3e4ccccd, 4, 0 },
		{ 0x118f, 2, 0x3dcccccd, 4, 0 },
		{ 0x119f, 2, 0x3f000000, 4, 0 },
		{ 0x11af, 2, 0x3c23d70a, 4, 0 },
		{ 0x11bf, 2, 0x3ca3d70a, 4, 0 },
		{ 0x0133, 2, 0x00000000, 1, 0 },
		{ 0x0140, 2, 0x00000000, 1, 0 },
		{ 0x0141, 2, 0x00000000, 1, 0 },
		{ 0x0142, 2, 0x00000000, 1, 0 },
		{ 0x0143, 2, 0x00000000, 1, 0 },
		{ 0x0144, 2, 0x00000001, 1, 0 },
		{ 0x0145, 2, 0x00000000, 1, 0 },
		{ 0x0146, 2, 0x00000000, 1, 0 },
		{ 0x0147, 2, 0x00000007, 1, 0 },
		{ 0x0148, 2, 0x00000000, 1, 0 },
		{ 0x0149, 2, 0x00000000, 1, 0 },
		{ 0x014a, 2, 0x00000011, 1, 0 },
		{ 0x014b, 2, 0x00000000, 1, 0 },
		{ 0x014c, 2, 0x00000000, 1, 0 },
		{ 0x014d, 2, 0x00000000, 1, 0 },
		{ 0x014e, 2, 0x00000001, 1, 0 },
		{ 0x014f, 2, 0x00000000, 1, 0 },
		{ 0x0150, 2, 0x00000000, 1, 0 },
		{ 0x0151, 2, 0x00000000, 1, 0 },
		{ 0x0152, 2, 0x00000044, 1, 0 },
		{ 0x0153, 2, 0x00000004, 1, 0 },
		{ 0x015b, 2, 0x00000000, 1, 0 },
		{ 0x015c, 2, 0x00000000, 1, 0 },
		{ 0x015d, 2, 0x00000000, 1, 0 },
		{ 0x015e, 2, 0x00000003, 1, 0 },
		{ 0x0132, 2, 0x00000011, 1, 0 },
		{ 0x0154, 2, 0x00000000, 1, 0 },
		{ 0x0155, 2, 0x00000054, 1, 0 },
		{ 0x0156, 2, 0x00000014, 1, 0 },
		{ 0x0157, 2, 0x00000094, 1, 0 },
		{ 0x0158, 2, 0x00000000, 1, 0 },
		{ 0x015f, 2, 0x00000000, 1, 0 },
		{ 0x013c, 2, 0x00000001, 1, 0 },
		{ 0x013d, 2, 0x00000000, 1, 0 },
		{ 0x013e, 2, 0x00000002, 1, 0 },
		{ 0x013f, 2, 0x00000007, 1, 0 },
		{ 0x015a, 2, 0x00000000, 1, 0 },
		{ 0x10ae, 2, 0x3a83126f, 4, 0 },
		{ 0x11ae, 2, 0x3a83126f, 4, 0 },
		{ 0x1094, 2, 0x00000000, 4, 0 },
		{ 0x1095, 2, 0x3f800000, 4, 0 },
		{ 0x1096, 2, 0xbd4ccccd, 4, 0 },
		{ 0x1097, 2, 0x38d1b717, 4, 0 },
		{ 0x1194, 2, 0x00000000, 4, 0 },
		{ 0x1195, 2, 0x3f800000, 4, 0 },
		{ 0x1196, 2, 0xbd4ccccd, 4, 0 },
		{ 0x1197, 2, 0x38d1b717, 4, 0 },
		{ 0x0120, 2, 0x0000000a, 1, 0 },
		{ 0x0123, 2, 0x0000005d, 1, 0 },
		{ 0x0121, 2, 0x00000050, 1, 0 },
		{ 0x0122, 2, 0x00000005, 1, 0 },
		{ 0x0128, 2, 0x0000000b, 1, 0 },
		{ 0x0108, 2, 0x00000000, 1, 0 },
		{ 0x0129, 2, 0x000000a0, 1, 0 },
		{ 0x012a, 2, 0x000000a0, 1, 0 },
		{ 0x100b, 2, 0x3f800000, 4, 0 },
		{ 0x110b, 2, 0x3f800000, 4, 0 },
		{ 0x0111, 2, 0x00000000, 1, 0 },
		{ 0x0113, 2, 0x00000000, 1, 0 },
		{ 0x0114, 2, 0x00000000, 1, 0 },
		{ 0x0172, 2, 0x00000000, 1, 0 },
		{ 0x01e3, 2, 0x00000000, 1, 0 },
		{ 0x01e4, 2, 0x00000000, 1, 0 },
		{ 0x018d, 2, 0x00000001, 1, 0 },
		{ 0x1000, 2, 0x3f800000, 4, 0 },
		{ 0x1001, 2, 0x3f800000, 4, 0 },
		{ 0x1002, 2, 0x00000000, 4, 0 },
		{ 0x1003, 2, 0x3f800000, 4, 0 },
		{ 0x1004, 2, 0x38a8a540, 4, 0 },
		{ 0x1005, 2, 0x38a8a540, 4, 0 },
		{ 0x1006, 2, 0x3f7ff580, 4, 0 },
		{ 0x1007, 2, 0x3f800000, 4, 0 },
		{ 0x1008, 2, 0xbf800000, 4, 0 },
		{ 0x1009, 2, 0x00000000, 4, 0 },
		{ 0x100a, 2, 0x3f800000, 4, 0 },
		{ 0x100b, 2, 0x3f800000, 4, 0 },
		{ 0x100c, 2, 0x3f800000, 4, 0 },
		{ 0x100e, 2, 0x3f800000, 4, 0 },
		{ 0x1010, 2, 0x3da2ad80, 4, 0 },
		{ 0x1011, 2, 0x00000000, 4, 0 },
		{ 0x1012, 2, 0x3f7ffd00, 4, 0 },
		{ 0x1013, 2, 0x3fc83380, 4, 0 },
		{ 0x1014, 2, 0xbfc58900, 4, 0 },
		{ 0x1015, 2, 0x3f75e8c0, 4, 0 },
		{ 0x1016, 2, 0x3f06bd80, 4, 0 },
		{ 0x1017, 2, 0xbf06ba00, 4, 0 },
		{ 0x1018, 2, 0x3f7ffc80, 4, 0 },
		{ 0x1019, 2, 0x3f800000, 4, 0 },
		{ 0x101a, 2, 0x00000000, 4, 0 },
		{ 0x101b, 2, 0x00000000, 4, 0 },
		{ 0x101c, 2, 0x3f800000, 4, 0 },
		{ 0x101d, 2, 0x00000000, 4, 0 },
		{ 0x101e, 2, 0x00000000, 4, 0 },
		{ 0x1020, 2, 0x3f800000, 4, 0 },
		{ 0x1021, 2, 0x3f800000, 4, 0 },
		{ 0x1022, 2, 0x3f800000, 4, 0 },
		{ 0x1023, 2, 0x3f800000, 4, 0 },
		{ 0x1024, 2, 0x00000000, 4, 0 },
		{ 0x1025, 2, 0x00000000, 4, 0 },
		{ 0x1026, 2, 0x00000000, 4, 0 },
		{ 0x1027, 2, 0x00000000, 4, 0 },
		{ 0x1030, 2, 0x3f800000, 4, 0 },
		{ 0x1031, 2, 0x00000000, 4, 0 },
		{ 0x1032, 2, 0x00000000, 4, 0 },
		{ 0x1033, 2, 0x3f800000, 4, 0 },
		{ 0x1034, 2, 0x00000000, 4, 0 },
		{ 0x1035, 2, 0x00000000, 4, 0 },
		{ 0x1036, 2, 0x3f800000, 4, 0 },
		{ 0x1037, 2, 0x00000000, 4, 0 },
		{ 0x1038, 2, 0x00000000, 4, 0 },
		{ 0x1039, 2, 0x3f800000, 4, 0 },
		{ 0x103a, 2, 0x00000000, 4, 0 },
		{ 0x103b, 2, 0x00000000, 4, 0 },
		{ 0x103c, 2, 0x3f800000, 4, 0 },
		{ 0x103d, 2, 0x00000000, 4, 0 },
		{ 0x103e, 2, 0x00000000, 4, 0 },
		{ 0x1043, 2, 0x39d2bd40, 4, 0 },
		{ 0x1044, 2, 0x39d2bd40, 4, 0 },
		{ 0x1045, 2, 0x3f7fcb40, 4, 0 },
		{ 0x1046, 2, 0x38a8a540, 4, 0 },
		{ 0x1047, 2, 0x38a8a540, 4, 0 },
		{ 0x1048, 2, 0x3f7ff580, 4, 0 },
		{ 0x1049, 2, 0x390c87c0, 4, 0 },
		{ 0x104a, 2, 0x390c87c0, 4, 0 },
		{ 0x104b, 2, 0x3f7fee80, 4, 0 },
		{ 0x104c, 2, 0x398c8300, 4, 0 },
		{ 0x104d, 2, 0x398c8300, 4, 0 },
		{ 0x104e, 2, 0x3f7fdcc0, 4, 0 },
		{ 0x1053, 2, 0x3f800000, 4, 0 },
		{ 0x1054, 2, 0x00000000, 4, 0 },
		{ 0x1055, 2, 0x00000000, 4, 0 },
		{ 0x1056, 2, 0x3f800000, 4, 0 },
		{ 0x1057, 2, 0x00000000, 4, 0 },
		{ 0x1058, 2, 0x00000000, 4, 0 },
		{ 0x1059, 2, 0x3f800000, 4, 0 },
		{ 0x105a, 2, 0x00000000, 4, 0 },
		{ 0x105b, 2, 0x00000000, 4, 0 },
		{ 0x105c, 2, 0x3f800000, 4, 0 },
		{ 0x105d, 2, 0x00000000, 4, 0 },
		{ 0x105e, 2, 0x00000000, 4, 0 },
		{ 0x1063, 2, 0x3f800000, 4, 0 },
		{ 0x1066, 2, 0x3f800000, 4, 0 },
		{ 0x1069, 2, 0x3f800000, 4, 0 },
		{ 0x106c, 2, 0x3f800000, 4, 0 },
		{ 0x1073, 2, 0x00000000, 4, 0 },
		{ 0x1076, 2, 0x3f800000, 4, 0 },
		{ 0x1079, 2, 0x3f800000, 4, 0 },
		{ 0x107c, 2, 0x3f800000, 4, 0 },
		{ 0x1083, 2, 0x38d1b700, 4, 0 },
		{ 0x1086, 2, 0x00000000, 4, 0 },
		{ 0x1089, 2, 0x00000000, 4, 0 },
		{ 0x108c, 2, 0x00000000, 4, 0 },
		{ 0x1093, 2, 0x00000000, 4, 0 },
		{ 0x1098, 2, 0x3f800000, 4, 0 },
		{ 0x1099, 2, 0x3f800000, 4, 0 },
		{ 0x109a, 2, 0x3f800000, 4, 0 },
		{ 0x10a1, 2, 0x3c58b440, 4, 0 },
		{ 0x10a2, 2, 0x3c58b440, 4, 0 },
		{ 0x10a3, 2, 0x3f793a40, 4, 0 },
		{ 0x10a4, 2, 0x3c58b440, 4, 0 },
		{ 0x10a5, 2, 0x3c58b440, 4, 0 },
		{ 0x10a6, 2, 0x3f793a40, 4, 0 },
		{ 0x10a7, 2, 0x3f800000, 4, 0 },
		{ 0x10a8, 2, 0x00000000, 4, 0 },
		{ 0x10a9, 2, 0x00000000, 4, 0 },
		{ 0x10aa, 2, 0x00000000, 4, 0 },
		{ 0x10ab, 2, 0x3bda2580, 4, 0 },
		{ 0x10ac, 2, 0x3bda2580, 4, 0 },
		{ 0x10ad, 2, 0x3f7c9780, 4, 0 },
		{ 0x10b0, 2, 0x3e0de280, 4, 0 },
		{ 0x10b1, 2, 0x3e0de280, 4, 0 },
		{ 0x10b2, 2, 0x3f390ec0, 4, 0 },
		{ 0x10b3, 2, 0x3f800000, 4, 0 },
		{ 0x10b4, 2, 0x00000000, 4, 0 },
		{ 0x10b5, 2, 0x00000000, 4, 0 },
		{ 0x10b6, 2, 0x3f353c00, 4, 0 },
		{ 0x10b8, 2, 0x3f800000, 4, 0 },
		{ 0x10b9, 2, 0x00000000, 4, 0 },
		{ 0x10c0, 2, 0x3fe304c0, 4, 0 },
		{ 0x10c1, 2, 0xbfdf6540, 4, 0 },
		{ 0x10c2, 2, 0x3f437bc0, 4, 0 },
		{ 0x10c3, 2, 0x3f7f7d40, 4, 0 },
		{ 0x10c4, 2, 0xbf7c6d00, 4, 0 },
		{ 0x10c5, 2, 0x3f7bea40, 4, 0 },
		{ 0x10c6, 2, 0x3d506f00, 4, 0 },
		{ 0x10c7, 2, 0x3d506f00, 4, 0 },
		{ 0x10c8, 2, 0x3f65f240, 4, 0 },
		{ 0x10c9, 2, 0x3baed500, 4, 0 },
		{ 0x10ca, 2, 0x3baed500, 4, 0 },
		{ 0x10cb, 2, 0x3f7fee80, 4, 0 },
		{ 0x10cc, 2, 0x3e0fc5c0, 4, 0 },
		{ 0x10cd, 2, 0xbe0ed000, 4, 0 },
		{ 0x10ce, 2, 0x3f7fc280, 4, 0 },
		{ 0x10d0, 2, 0x3fff64c0, 4, 0 },
		{ 0x10d1, 2, 0x00000000, 4, 0 },
		{ 0x10d2, 2, 0x3f800000, 4, 0 },
		{ 0x10d3, 2, 0x3f800000, 4, 0 },
		{ 0x10d4, 2, 0x3f800000, 4, 0 },
		{ 0x10d5, 2, 0x3f800000, 4, 0 },
		{ 0x10d7, 2, 0x3f8ef7c0, 4, 0 },
		{ 0x10d8, 2, 0x3f8ef7c0, 4, 0 },
		{ 0x10d9, 2, 0x3f5fd780, 4, 0 },
		{ 0x10da, 2, 0x3f76ba40, 4, 0 },
		{ 0x10db, 2, 0xbfe16f80, 4, 0 },
		{ 0x10dc, 2, 0x3fe16f80, 4, 0 },
		{ 0x10dd, 2, 0x3f641800, 4, 0 },
		{ 0x10de, 2, 0xbf5ad200, 4, 0 },
		{ 0x10e0, 2, 0x3f7c5880, 4, 0 },
		{ 0x10e1, 2, 0xbfefd200, 4, 0 },
		{ 0x10e2, 2, 0x3fefd200, 4, 0 },
		{ 0x10e3, 2, 0x3f6aa180, 4, 0 },
		{ 0x10e4, 2, 0xbf66fa40, 4, 0 },
		{ 0x10e5, 2, 0x3f800000, 4, 0 },
		{ 0x10e8, 2, 0x3f800000, 4, 0 },
		{ 0x10e9, 2, 0x00000000, 4, 0 },
		{ 0x10ea, 2, 0x00000000, 4, 0 },
		{ 0x10eb, 2, 0x00000000, 4, 0 },
		{ 0x10f0, 2, 0x3f800000, 4, 0 },
		{ 0x10f1, 2, 0x00000000, 4, 0 },
		{ 0x10f2, 2, 0x00000000, 4, 0 },
		{ 0x10f3, 2, 0x00000000, 4, 0 },
		{ 0x10f4, 2, 0x00000000, 4, 0 },
		{ 0x10f5, 2, 0x3f800000, 4, 0 },
		{ 0x10f6, 2, 0x00000000, 4, 0 },
		{ 0x10f7, 2, 0x00000000, 4, 0 },
		{ 0x10f8, 2, 0x00000000, 4, 0 },
		{ 0x10f9, 2, 0x00000000, 4, 0 },
		{ 0x1200, 2, 0x00000000, 4, 0 },
		{ 0x1201, 2, 0x3f800000, 4, 0 },
		{ 0x1202, 2, 0x3f800000, 4, 0 },
		{ 0x1203, 2, 0x3f800000, 4, 0 },
		{ 0x1204, 2, 0x3f800000, 4, 0 },
		{ 0x1205, 2, 0x00000000, 4, 0 },
		{ 0x1206, 2, 0x00000000, 4, 0 },
		{ 0x1207, 2, 0x3f800000, 4, 0 },
		{ 0x1208, 2, 0x00000000, 4, 0 },
		{ 0x1209, 2, 0x00000000, 4, 0 },
		{ 0x120a, 2, 0x3f800000, 4, 0 },
		{ 0x120b, 2, 0x00000000, 4, 0 },
		{ 0x120c, 2, 0x00000000, 4, 0 },
		{ 0x120d, 2, 0x3f800000, 4, 0 },
		{ 0x120e, 2, 0x00000000, 4, 0 },
		{ 0x120f, 2, 0x00000000, 4, 0 },
		{ 0x1210, 2, 0x3f800000, 4, 0 },
		{ 0x1211, 2, 0x00000000, 4, 0 },
		{ 0x1212, 2, 0x00000000, 4, 0 },
		{ 0x1213, 2, 0x3f800000, 4, 0 },
		{ 0x1214, 2, 0x3f800000, 4, 0 },
		{ 0x1215, 2, 0x3f800000, 4, 0 },
		{ 0x1216, 2, 0x3f800000, 4, 0 },
		{ 0x1217, 2, 0x3f800000, 4, 0 },
		{ 0x1218, 2, 0x00000000, 4, 0 },
		{ 0x1219, 2, 0x00000000, 4, 0 },
		{ 0x121a, 2, 0x00000000, 4, 0 },
		{ 0x121b, 2, 0x00000000, 4, 0 },
		{ 0x121c, 2, 0x00000000, 4, 0 },
		{ 0x121d, 2, 0x3f800000, 4, 0 },
		{ 0x121e, 2, 0x3f800000, 4, 0 },
		{ 0x121f, 2, 0x3f800000, 4, 0 },
		{ 0x1235, 2, 0x3f800000, 4, 0 },
		{ 0x1236, 2, 0x3f800000, 4, 0 },
		{ 0x1237, 2, 0x3f800000, 4, 0 },
		{ 0x1238, 2, 0x3f800000, 4, 0 },
		{ 0x018d, 2, 0x00000000, 1, 0 },
		{ 0x018d, 2, 0x00000000, 1, 0 },
		{ 0x0134, 2, 0x00000000, 1, 0 },
		{ 0x0135, 2, 0x00000000, 1, 0 },
		{ 0x0136, 2, 0x00000090, 1, 0 },
		{ 0x0137, 2, 0x00000001, 1, 0 },
		{ 0x0138, 2, 0x00000064, 1, 0 },
		{ 0x0139, 2, 0x00000000, 1, 0 },
		{ 0x013a, 2, 0x00000000, 1, 0 },
		{ 0x013b, 2, 0x00000000, 1, 0 },
		{ 0x1235, 2, 0x3f800000, 4, 0 },
		{ 0x1236, 2, 0x3b23d700, 4, 0 },
		{ 0x1237, 2, 0x3c23d700, 4, 0 },
		{ 0x1238, 2, 0x3f800000, 4, 0 },
		{ 0x0251, 2, 0x00000044, 1, 0 },
		{ 0x0253, 2, 0x00000003, 1, 0 },
		{ 0x0254, 2, 0x00000003, 1, 0 },
		{ 0x0252, 2, 0x00000000, 1, 0 },
		{ 0x0255, 2, 0x00000000, 1, 0 },
		{ 0x0257, 2, 0x0000002c, 1, 0 },
		{ 0x02a0, 2, 0x00000000, 1, 0 },
		{ 0x02a1, 2, 0x00000000, 1, 0 },
		{ 0x02a2, 2, 0x00000000, 1, 0 },
		{ 0x02a3, 2, 0x00000000, 1, 0 },
		{ 0x018c, 2, 0x00000031, 1, 0 },
		{ 0x1479, 2, 0x00000000, 2, 0 },
		{ 0x147a, 2, 0x0000c000, 2, 0 },
		{ 0x14f9, 2, 0x00000000, 2, 0 },
		{ 0x14fa, 2, 0x0000c000, 2, 0 },
		{ 0x1450, 2, 0x00000000, 2, 0 },
		{ 0x14d0, 2, 0x00000000, 2, 0 },
		{ 0x10d3, 2, 0x00002000, 2, 0 },
		{ 0x11d3, 2, 0x00002000, 2, 0 },
		{ 0x1529, 2, 0x00000000, 2, 0 },
		{ 0x152a, 2, 0x00009000, 2, 0 },
		{ 0x018c, 2, 0x00000000, 1, 0 },
		{ 0x0084, 2, 0x00000080, 1, 0 },
		{ 0x1020, 2, 0x3f333333, 4, 0 },
		{ 0x1120, 2, 0xbf333333, 4, 0 },
		{ 0x10e5, 2, 0x3f800000, 4, 0 },
		{ 0x11e5, 2, 0xbf800000, 4, 0 },
		{ 0x1022, 2, 0x3f800000, 4, 0 },
		{ 0x1122, 2, 0x3f800000, 4, 0 },
		{ 0x0010, 2, 0x000000c0, 1, 0 },
		{ 0x0250, 2, 0x000000df, 1, 0 },
		{ 0x01e0, 2, 0x00000002, 1, 0 },
		{ 0x0193, 2, 0x00000002, 1, 0 },
		{ 0x0192, 2, 0x00000000, 1, 0 },
		{ 0x01a3, 2, 0x00000002, 1, 0 },
		{ 0x01a2, 2, 0x00000000, 1, 0 },
		{ 0x0109, 2, 0x00000000, 1, 0 },
		{ 0x1012, 2, 0x3f7ffd00, 4, 0 },
		{ 0x1112, 2, 0x3f7ffd00, 4, 0 },
		{ 0x0154, 2, 0x00000000, 1, 0 },
		{ 0x0155, 2, 0x00000054, 1, 0 },
		{ 0x0156, 2, 0x00000014, 1, 0 },
		{ 0x0157, 2, 0x00000094, 1, 0 },
		{ 0x0158, 2, 0x00000000, 1, 0 },
		{ 0x102d, 2, 0x40000000, 4, 0 },
		{ 0x112d, 2, 0x40000000, 4, 0 },
		{ 0x10aa, 2, 0xb9400000, 4, 0 },
		{ 0x11aa, 2, 0xb9400000, 4, 0 },
		{ 0x100e, 2, 0x3f7ffd00, 4, 0 },
		{ 0x110e, 2, 0x3f7ffd00, 4, 0 },
		{ 0x011b, 2, 0x00000012, 1, 0 },
		{ 0x0154, 2, 0x00000000, 1, 0 },
		{ 0x0155, 2, 0x00000054, 1, 0 },
		{ 0x0156, 2, 0x00000014, 1, 0 },
		{ 0x0157, 2, 0x00000094, 1, 0 },
		{ 0x0158, 2, 0x00000000, 1, 0 },
		{ 0x102d, 2, 0x40000000, 4, 0 },
		{ 0x102e, 2, 0x40400000, 4, 0 },
		{ 0x112d, 2, 0x40000000, 4, 0 },
		{ 0x112e, 2, 0x40400000, 4, 0 },
		{ 0x100e, 2, 0x3f7ffd00, 4, 0 },
		{ 0x10aa, 2, 0xb9400000, 4, 0 },
		{ 0x110e, 2, 0x3f7ffd00, 4, 0 },
		{ 0x11aa, 2, 0xb9400000, 4, 0 },
		{ 0x011b, 2, 0x00000012, 1, 0 },
		{ 0x0001, 2, 0x000000f0, 1, 0 },
		{ 0x0101, 2, 0x00000001, 1, 0 },
		{ 0x018c, 2, 0x00000031, 1, 0 },
		{ 0x1479, 2, 0x00001a00, 2, 0 },
		{ 0x14f9, 2, 0x00000b62, 2, 0 },
		{ 0x147a, 2, 0x0000d600, 2, 0 },
		{ 0x14fa, 2, 0x0000cf00, 2, 0 },
		{ 0x1450, 2, 0x0000f8f7, 2, 0 },
		{ 0x14d0, 2, 0x000005a2, 2, 0 },
		{ 0x10d3, 2, 0x0000248f, 2, 0 },
		{ 0x11d3, 2, 0x000022b9, 2, 0 },
		{ 0x018c, 2, 0x00000000, 1, 0 },
		{ 0x02a0, 2, 0x00000000, 1, 0 },
		{ 0x02a1, 2, 0x0000001f, 1, 0 },
		{ 0x02a2, 2, 0x000000ff, 1, 0 },
		{ 0x02a3, 2, 0x000000d0, 1, 0 },
		{ 0x1020, 2, 0x3f051ec0, 4, 0 },
		{ 0x1120, 2, 0xbf0a3d80, 4, 0 },
		{ 0x0257, 2, 0x0000008e, 1, 0 },
		{ 0x0084, 2, 0x00000050, 1, 0 },
		{ 0x018f, 2, 0x00000000, 1, 0 },
		{ 0x018e, 2, 0x0000007f, 1, 0 },
		{ 0x0102, 2, 0x00000002, 1, 0 },
		{ 0x0102, 2, 0x00000000, 1, 2 },
		{ 0x018d, 2, 0x00000001, 1, 0 },
		{ 0x1043, 2, 0x3b038040, 4, 0 },
		{ 0x1044, 2, 0x3b038040, 4, 0 },
		{ 0x1045, 2, 0x3f7ef900, 4, 0 },
		{ 0x018d, 2, 0x00000000, 1, 0 },
		{ 0x0109, 2, 0x00000001, 1, 0 },
		{ 0x010a, 2, 0x00000044, 1, 0 },
		{ 0x0380, 2, 0x00008000, 2, 0 },
		{ 0x0107, 2, 0x00000000, 1, 0 },
		{ 0x10b5, 2, 0x00000000, 4, 0 },
		{ 0x11b5, 2, 0x00000000, 4, 0 },
		{ 0x0129, 2, 0x000000a0, 1, 0 },
		{ 0x012a, 2, 0x000000a0, 1, 0 },
		{ 0x100b, 2, 0x3f800000, 4, 0 },
		{ 0x110b, 2, 0x3f800000, 4, 0 },
		{ 0x017c, 2, 0x00000001, 1, 0 },
		{ 0x0170, 2, 0x00000003, 1, 0 },
		{ 0x10b5, 2, 0x00000000, 4, 0 },
		{ 0x0171, 2, 0x00000003, 1, 0 },
		{ 0x11b5, 2, 0x00000000, 4, 0 },
		{ 0x01f8, 2, 0x00000066, 1, 2 },
		{ 0x017c, 2, 0x00000000, 1, 0 },
		{ 0x0302, 2, 0x00000001, 1, 10 },
		{ 0x0303, 2, 0x000000f1, 1, 10 },
		{ 0x0304, 2, 0x00000400, 2, 10 },
	},
};

/*
 * THE TARGET POSITION, distinct from g_u4CurrPosition.
 *
 * moveAF writes it under the lock, then reads it back to compose the value to
 * send, and only then copies it into g_u4CurrPosition -- three accesses to an
 * address that is not g_u4CurrPosition's ("f9062ad3 str"@0xffffff8008745544
 * against "f9062688 str"@0xffffff80087455b4). The name is the address (rule 5).
 */
static unsigned long g9c96c50;

/*
 * LC898122_init() was reconstructed from the factory kernel disassembly (0xffffff80087457e0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_lc898122af_LC898122AF.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void LC898122_init(void)
{
	unsigned char ucOtp[33] = {0};
	/*
	 * INITIALISED, and the clearing shows it: the factory wipes
	 * THIRTY-SIX bytes -- "b90033ff str"@0xffffff8008745074 is a word,
	 * not a byte -- while the array on its own wants thirty-three. The three
	 * extra cover this variable, which sits right after, and clang merges them
	 * into a single write only if it is initialised too.
	 */
	unsigned char ucAfOtp = 0;
	unsigned char UcRegDat;
	unsigned long i;

	LC898122_write_settings(g991d3f0.n, g991d3f0.e);

	LOG_INF("LC898122_init \n");

	RegReadA_LC898122AF(0x0250, &UcRegDat);
	UcRegDat &= 0x80;
	RegWriteA_LC898122AF(0x0250, UcRegDat);

	RegWriteA_LC898122AF(0x0010, 0x00);
	RegWriteA_LC898122AF(0x0020, 0x00);

	DrvSw(0);
	SelectGySleep(1);

	RegReadA_LC898122AF(0x020B, &UcRegDat);
	UcRegDat &= 0x1A;
	RegWriteA_LC898122AF(0x020B, UcRegDat);

	SelectModule(2);

	IniSetAf();
	IniSet();

	for (i = 0; i < 0x21; i++)
		ucOtp[i] = s4LC898OTP_ReadReg(i + 0x11);

	ucAfOtp = s4LC898OTP_ReadReg(0x38);

	RamAccFixMod(1);

	RamWriteA_LC898122AF(0x1479, (ucOtp[1] << 8) + ucOtp[0]);
	RamWriteA_LC898122AF(0x14F9, (ucOtp[3] << 8) + ucOtp[2]);
	RamWriteA_LC898122AF(0x147A, (ucOtp[5] << 8) + ucOtp[4]);
	RamWriteA_LC898122AF(0x14FA, (ucOtp[7] << 8) + ucOtp[6]);
	RamWriteA_LC898122AF(0x1450, (ucOtp[9] << 8) + ucOtp[8]);
	RamWriteA_LC898122AF(0x14D0, (ucOtp[11] << 8) + ucOtp[10]);
	RamWriteA_LC898122AF(0x10D3, (ucOtp[13] << 8) + ucOtp[12]);
	RamWriteA_LC898122AF(0x11D3, (ucOtp[15] << 8) + ucOtp[14]);

	RamAccFixMod(0);

	RegWriteA_LC898122AF(0x02A0, ucOtp[17]);
	RegWriteA_LC898122AF(0x02A1, ucOtp[16]);
	RegWriteA_LC898122AF(0x02A2, ucOtp[19]);
	RegWriteA_LC898122AF(0x02A3, ucOtp[18]);
	RegWriteA_LC898122AF(0x0257, ucOtp[20]);

	RamWrite32A_LC898122AF(0x1020,
			       ((unsigned long)ucOtp[24] << 24) +
			       (ucOtp[23] << 16) + (ucOtp[22] << 8) +
			       ucOtp[21]);
	RamWrite32A_LC898122AF(0x1120,
			       ((unsigned long)ucOtp[28] << 24) +
			       (ucOtp[27] << 16) + (ucOtp[26] << 8) +
			       ucOtp[25]);

	SetDOFSTDAF(ucAfOtp);

	RemOff(1);

	RegWriteA_LC898122AF(0x0304, 0x04);
	s4AF_Write_Word_Word(0x0380, 0xFA00);

	RtnCen(0);

	/*
	 * mdelay() was reconstructed from the factory kernel disassembly (0xffffff80087454d0).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_lc898122af_LC898122AF.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	mdelay(150);

	RemOff(0);
	OisEna();
}

/*
 * moveAF() was reconstructed from the factory kernel disassembly (0xffffff8008744f04).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_lc898122af_LC898122AF.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static inline int moveAF(unsigned long a_u4Position)
{
	if (a_u4Position > g_u4AF_MACRO || a_u4Position < g_u4AF_INF) {
		LOG_INF("out of range\n");
		return -EINVAL;
	}

	if (*g_pAF_Opened == 1) {
		LC898122_init();

		spin_lock(g_pAF_SpinLock);
		g_u4CurrPosition = 0;
		*g_pAF_Opened = 2;
		spin_unlock(g_pAF_SpinLock);
	}

	if (a_u4Position == 0) {
		LOG_INF("hct-drv s4AF_Write_Word_Word\n");
		s4AF_Write_Word_Word(0x0380, 0x7080);
		return 0;
	}

	if (g_u4CurrPosition == a_u4Position)
		return 0;

	spin_lock(g_pAF_SpinLock);
	g9c96c50 = a_u4Position;
	spin_unlock(g_pAF_SpinLock);

	LOG_INF("move [curr] %lu [target] %lu  DAC 0x%x\n",
		g_u4CurrPosition, g9c96c50,
		(unsigned int)(0x6400 + (g9c96c50 << 5)));

	s4AF_Write_Word_Word(0x0380, 0x6400 + (g9c96c50 << 5));

	spin_lock(g_pAF_SpinLock);
	g_u4CurrPosition = g9c96c50;
	spin_unlock(g_pAF_SpinLock);

	return 0;
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
long LC898122AF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command,
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
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008745afc, 844 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_lc898122af_LC898122AF.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff80099231b8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_lc898122af_LC898122AF.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct {
	unsigned short n;
	struct lc898122_setting e[2];
} g99231b8 = {
	2,
	{
		{ 0x10b5, 2, 0, 4, 0 },
		{ 0x11b5, 2, 0, 4, 0 },
	},
};

int LC898122_PowerDown(void)
{
	LC898122_write_settings(g99231b8.n, g99231b8.e);

	SetStandby(4);

	return 0;
}

static void LC898122_write_settings(unsigned short n,
				    struct lc898122_setting *tbl)
{
	unsigned long i;

	LOG_INF("Enter\n");

	for (i = 0; i < n; i++, tbl++) {
		switch (tbl->width) {
		case 1:
			s4AF_Write_Word_Byte(tbl->addr, tbl->data);
			LOG_INF("addr[%d] =  0x%04x, data = 0x%02x\n",
				(int)i, tbl->addr, (unsigned char)tbl->data);
			break;
		case 2:
			s4AF_Write_Word_Word(tbl->addr, tbl->data);
			LOG_INF("addr[%d] =  0x%04x, data = 0x%04x\n",
				(int)i, tbl->addr, (unsigned short)tbl->data);
			break;
		case 4:
			s4AF_Write_Word_DWord(tbl->addr, tbl->data);
			LOG_INF("addr[%d] =  0x%04x, data = 0x%08lx\n",
				(int)i, tbl->addr, tbl->data);
			break;
		default:
			LOG_INF("Unsupport data type: %d\n", tbl->width);
			break;
		}

		if (tbl->delay >= 21)
			msleep(tbl->delay);
		else if (tbl->delay)
			usleep_range(tbl->delay * 1000,
				     tbl->delay * 1000 + 1000);
	}

	LOG_INF("Exit\n");
}

int LC898122AF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");

	if (*g_pAF_Opened == 2) {
		LOG_INF("Wait\n");

		/*
		 * s4AF_Write_Word_Word() was reconstructed from the factory kernel disassembly.
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_misc_mediatek_lens_main_common_lc898122af_LC898122AF.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		s4AF_Write_Word_Word(0x0380, 0x7080);
		msleep(50);
		s4AF_Write_Word_Word(0x0380, 0x6a40);
		msleep(50);
		s4AF_Write_Word_Word(0x0380, 0x67c0);
		msleep(50);
		s4AF_Write_Word_Word(0x0380, 0x65e0);
		msleep(50);
		s4AF_Write_Word_Word(0x0380, 0x64a0);
		msleep(50);

		/*
		 * Inlined: in the binary there is a `bl LC898122_write_settings`
		 * followed by a `bl SetStandby` with w0 = 4
		 * ("321e03e0 orr"@0xffffff8008745ff8), which is exactly the
		 * body of LC898122_PowerDown. It is also why in
		 * the whole of stock.elf there is not a single `bl` towards
		 * LC898122_PowerDown: it is called, but always inlined.
		 */
		LC898122_PowerDown();
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

int LC898122AF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient,
			    spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_SpinLock = pAF_SpinLock;
	g_pAF_Opened = pAF_Opened;

	/*
	 * NO initAF HERE. In the factory build this function is thirty-two bytes:
	 * three `str`s and a `ret` ("f9061500 str"@0xffffff800874619c and the two
	 * after it). The power-up has moved inside moveAF, at the first move,
	 * and is guarded by *g_pAF_Opened == 1.
	 */
	return 1;
}

int LC898122AF_GetFileName(unsigned char *pFileName)
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
