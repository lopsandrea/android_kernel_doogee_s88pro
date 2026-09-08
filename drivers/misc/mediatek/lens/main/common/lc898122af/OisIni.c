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

/* ************************** */
/* Include Header File */
/* ************************** */
/* #define		OISINI */

/* #include      "Main.h" */
/* #include      "Cmd.h" */
#include "Ois.h"
#include "OisDef.h"
#include "OisFil.h"

unsigned short UsCntXof; /* OPTICAL Center Xvalue */
unsigned short UsCntYof; /* OPTICAL Center Yvalue */
unsigned char UcPwmMod;  /* PWM MODE */
unsigned char UcCvrCod;  /* CverCode */

/*
 * DUE COPPIE DI TABELLE: la seconda e' per l'altro modulo.
 *
 * IniFil sceglie con due csel sullo stesso g9c96cbc
 * ("3972f108 ldrb"@0xffffff800874a0e0): a 0xffffff8008f4c6e8 e
 * 0xffffff8008f4c708 le tabelle del modulo 2, a 0xffffff8008f4d388 e
 * 0xffffff8008f4d3a8 le altre.
 *
 * LE DUE TABELLE DI REGISTRI SONO IDENTICHE FRA LORO E A QUELLA DI ALPS --
 * lette dal binario, non supposte. Restano due perche' il csel sceglie fra
 * due INDIRIZZI: se nel sorgente ce ne fosse una sola non ci sarebbe niente
 * da scegliere, e il csel non ci sarebbe.
 *
 * QUELLE DELLA RAM sono 199 voci piu' il terminatore, contro le 322 di
 * ALPS. Dentro ci sono 20 indirizzi che ALPS non ha, 11 valori ritarati
 * rispetto ad ALPS, e fra le due di fabbrica 39 valori diversi.
 *
 * IniFil accende WC_RAMACCXY prima del ciclo della RAM e lo spegne dopo. Il
 * nome del registro dice "accesso a X e Y", e una tabella piu' corta e'
 * coerente con lo scrivere due assi in un colpo -- ma quella e' una lettura
 * del nome, non una misura: 37 indirizzi oltre 0x1100 ci sono ancora.
 */
const struct STFILREG g8f4c6e8[] = {{0x0111, 0x00},
				    {0x0113, 0x00},
				    {0x0114, 0x00},
				    {0x0172, 0x00},
				    {0x01E3, 0x00},
				    {0x01E4, 0x00},
				    {0xFFFF, 0xFF} };

const struct STFILRAM g8f4c708[] = {
	{0x1000, 0x3F800000},
	{0x1001, 0x3F800000},
	{0x1002, 0x00000000},
	{0x1003, 0x3F800000},
	{0x1004, 0x38A8A540},
	{0x1005, 0x38A8A540},
	{0x1006, 0x3F7FF580},
	{0x1007, 0x3F800000},
	{0x1008, 0xBF800000},
	{0x1009, 0x00000000},
	{0x100A, 0x3F800000},
	{0x100B, 0x3F800000},
	{0x100C, 0x3F800000},
	{0x100E, 0x3F800000},
	{0x1010, 0x3DA2AD80},
	{0x1011, 0x00000000},
	{0x1012, 0x3F7FFD00},
	{0x1013, 0x3FC83380},
	{0x1014, 0xBFC58900},
	{0x1015, 0x3F75E8C0},
	{0x1016, 0x3F06BD80},
	{0x1017, 0xBF06BA00},
	{0x1018, 0x3F7FFC80},
	{0x1019, 0x3F800000},
	{0x101A, 0x00000000},
	{0x101B, 0x00000000},
	{0x101C, 0x3F800000},
	{0x101D, 0x00000000},
	{0x101E, 0x00000000},
	{0x1020, 0x3F800000},
	{0x1021, 0x3F800000},
	{0x1022, 0x3F800000},
	{0x1023, 0x3F800000},
	{0x1024, 0x00000000},
	{0x1025, 0x00000000},
	{0x1026, 0x00000000},
	{0x1027, 0x00000000},
	{0x1030, 0x3F800000},
	{0x1031, 0x00000000},
	{0x1032, 0x00000000},
	{0x1033, 0x3F800000},
	{0x1034, 0x00000000},
	{0x1035, 0x00000000},
	{0x1036, 0x3F800000},
	{0x1037, 0x00000000},
	{0x1038, 0x00000000},
	{0x1039, 0x3F800000},
	{0x103A, 0x00000000},
	{0x103B, 0x00000000},
	{0x103C, 0x3F800000},
	{0x103D, 0x00000000},
	{0x103E, 0x00000000},
	{0x1043, 0x39D2BD40},
	{0x1044, 0x39D2BD40},
	{0x1045, 0x3F7FCB40},
	{0x1046, 0x38A8A540},
	{0x1047, 0x38A8A540},
	{0x1048, 0x3F7FF580},
	{0x1049, 0x390C87C0},
	{0x104A, 0x390C87C0},
	{0x104B, 0x3F7FEE80},
	{0x104C, 0x398C8300},
	{0x104D, 0x398C8300},
	{0x104E, 0x3F7FDCC0},
	{0x1053, 0x3F800000},
	{0x1054, 0x00000000},
	{0x1055, 0x00000000},
	{0x1056, 0x3F800000},
	{0x1057, 0x00000000},
	{0x1058, 0x00000000},
	{0x1059, 0x3F800000},
	{0x105A, 0x00000000},
	{0x105B, 0x00000000},
	{0x105C, 0x3F800000},
	{0x105D, 0x00000000},
	{0x105E, 0x00000000},
	{0x1063, 0x3F800000},
	{0x1066, 0x3F800000},
	{0x1069, 0x3F800000},
	{0x106C, 0x3F800000},
	{0x1073, 0x00000000},
	{0x1076, 0x3F800000},
	{0x1079, 0x3F800000},
	{0x107C, 0x3F800000},
	{0x1083, 0x38D1B700},
	{0x1086, 0x00000000},
	{0x1089, 0x00000000},
	{0x108C, 0x00000000},
	{0x1093, 0x00000000},
	{0x1098, 0x3F800000},
	{0x1099, 0x3F800000},
	{0x109A, 0x3F800000},
	{0x10A1, 0x3C58B440},
	{0x10A2, 0x3C58B440},
	{0x10A3, 0x3F793A40},
	{0x10A4, 0x3C58B440},
	{0x10A5, 0x3C58B440},
	{0x10A6, 0x3F793A40},
	{0x10A7, 0x3F800000},
	{0x10A8, 0x00000000},
	{0x10A9, 0x00000000},
	{0x10AA, 0x00000000},
	{0x10AB, 0x3BDA2580},
	{0x10AC, 0x3BDA2580},
	{0x10AD, 0x3F7C9780},
	{0x10B0, 0x3E0DE280},
	{0x10B1, 0x3E0DE280},
	{0x10B2, 0x3F390EC0},
	{0x10B3, 0x3F800000},
	{0x10B4, 0x00000000},
	{0x10B5, 0x00000000},
	{0x10B6, 0x3F353C00},
	{0x10B8, 0x3F800000},
	{0x10B9, 0x00000000},
	{0x10C0, 0x3FE304C0},
	{0x10C1, 0xBFDF6540},
	{0x10C2, 0x3F437BC0},
	{0x10C3, 0x3F7F7D40},
	{0x10C4, 0xBF7C6D00},
	{0x10C5, 0x3F7BEA40},
	{0x10C6, 0x3D506F00},
	{0x10C7, 0x3D506F00},
	{0x10C8, 0x3F65F240},
	{0x10C9, 0x3BAED500},
	{0x10CA, 0x3BAED500},
	{0x10CB, 0x3F7FEE80},
	{0x10CC, 0x3E0FC5C0},
	{0x10CD, 0xBE0ED000},
	{0x10CE, 0x3F7FC280},
	{0x10D0, 0x3FFF64C0},
	{0x10D1, 0x00000000},
	{0x10D2, 0x3F800000},
	{0x10D3, 0x3F800000},
	{0x10D4, 0x3F800000},
	{0x10D5, 0x3F800000},
	{0x10D7, 0x3F8EF7C0},
	{0x10D8, 0x3F8EF7C0},
	{0x10D9, 0x3F5FD780},
	{0x10DA, 0x3F7B0A00},
	{0x10DB, 0xBFE0EDC0},
	{0x10DC, 0x3FE0EDC0},
	{0x10DD, 0x3F62FDC0},
	{0x10DE, 0xBF5E07C0},
	{0x10E0, 0x3F7C5880},
	{0x10E1, 0xBFEFD200},
	{0x10E2, 0x3FEFD200},
	{0x10E3, 0x3F6AA180},
	{0x10E4, 0xBF66FA40},
	{0x10E5, 0x3F800000},
	{0x10E8, 0x3F800000},
	{0x10E9, 0x00000000},
	{0x10EA, 0x00000000},
	{0x10EB, 0x00000000},
	{0x10F0, 0x3F800000},
	{0x10F1, 0x00000000},
	{0x10F2, 0x00000000},
	{0x10F3, 0x00000000},
	{0x10F4, 0x00000000},
	{0x10F5, 0x3F800000},
	{0x10F6, 0x00000000},
	{0x10F7, 0x00000000},
	{0x10F8, 0x00000000},
	{0x10F9, 0x00000000},
	{0x1200, 0x00000000},
	{0x1201, 0x3F800000},
	{0x1202, 0x3F800000},
	{0x1203, 0x3F800000},
	{0x1204, 0x3F800000},
	{0x1205, 0x00000000},
	{0x1206, 0x00000000},
	{0x1207, 0x3F800000},
	{0x1208, 0x00000000},
	{0x1209, 0x00000000},
	{0x120A, 0x3F800000},
	{0x120B, 0x00000000},
	{0x120C, 0x00000000},
	{0x120D, 0x3F800000},
	{0x120E, 0x00000000},
	{0x120F, 0x00000000},
	{0x1210, 0x3F800000},
	{0x1211, 0x00000000},
	{0x1212, 0x00000000},
	{0x1213, 0x3F800000},
	{0x1214, 0x3F800000},
	{0x1215, 0x3F800000},
	{0x1216, 0x3F800000},
	{0x1217, 0x3F800000},
	{0x1218, 0x00000000},
	{0x1219, 0x00000000},
	{0x121A, 0x00000000},
	{0x121B, 0x00000000},
	{0x121C, 0x00000000},
	{0x121D, 0x3F800000},
	{0x121E, 0x3F800000},
	{0x121F, 0x3F800000},
	{0x1235, 0x3F800000},
	{0x1236, 0x3F800000},
	{0x1237, 0x3F800000},
	{0x1238, 0x3F800000},
	{0xFFFF, 0xFFFFFFFF}
};

const struct STFILREG CsFilReg[] = {{0x0111, 0x00}, /*00,0111 */
				   {0x0113, 0x00}, /*00,0113 */
				   {0x0114, 0x00}, /*00,0114 */
				   {0x0172, 0x00}, /*00,0172 */
				   {0x01E3, 0x00}, /*00,01E3 */
				   {0x01E4, 0x00}, /*00,01E4 */
				   {0xFFFF, 0xFF} };

/* 32bit */
const struct STFILRAM CsFilRam[] = {
	{0x1000, 0x3F800000},
	{0x1001, 0x3F800000},
	{0x1002, 0x00000000},
	{0x1003, 0x3F800000},
	{0x1004, 0x3828A700},
	{0x1005, 0x3828A700},
	{0x1006, 0x3F7FFAC0},
	{0x1007, 0x3F800000},
	{0x1008, 0xBF800000},
	{0x1009, 0x00000000},
	{0x100A, 0x3F800000},
	{0x100B, 0x3F800000},
	{0x100C, 0x3F800000},
	{0x100E, 0x3F800000},
	{0x1010, 0x3DA2AD80},
	{0x1011, 0x00000000},
	{0x1012, 0x3F7FFE00},
	{0x1013, 0x3FB26DC0},
	{0x1014, 0xBFB00DC0},
	{0x1015, 0x3F75E8C0},
	{0x1016, 0x3F1B2780},
	{0x1017, 0xBF1B2400},
	{0x1018, 0x3F7FFC80},
	{0x1019, 0x3F800000},
	{0x101A, 0x00000000},
	{0x101B, 0x00000000},
	{0x101C, 0x3F800000},
	{0x101D, 0x00000000},
	{0x101E, 0x00000000},
	{0x1020, 0x3F800000},
	{0x1021, 0x3F800000},
	{0x1022, 0x3F800000},
	{0x1023, 0x3F800000},
	{0x1024, 0x00000000},
	{0x1025, 0x00000000},
	{0x1026, 0x00000000},
	{0x1027, 0x00000000},
	{0x1030, 0x3F800000},
	{0x1031, 0x00000000},
	{0x1032, 0x00000000},
	{0x1033, 0x3F800000},
	{0x1034, 0x00000000},
	{0x1035, 0x00000000},
	{0x1036, 0x3F800000},
	{0x1037, 0x00000000},
	{0x1038, 0x00000000},
	{0x1039, 0x3F800000},
	{0x103A, 0x00000000},
	{0x103B, 0x00000000},
	{0x103C, 0x3F800000},
	{0x103D, 0x00000000},
	{0x103E, 0x00000000},
	{0x1043, 0x39D2BD40},
	{0x1044, 0x39D2BD40},
	{0x1045, 0x3F7FCB40},
	{0x1046, 0x388C8A40},
	{0x1047, 0x388C8A40},
	{0x1048, 0x3F7FF740},
	{0x1049, 0x390C87C0},
	{0x104A, 0x390C87C0},
	{0x104B, 0x3F7FEE80},
	{0x104C, 0x398C8300},
	{0x104D, 0x398C8300},
	{0x104E, 0x3F7FDCC0},
	{0x1053, 0x3F800000},
	{0x1054, 0x00000000},
	{0x1055, 0x00000000},
	{0x1056, 0x3F800000},
	{0x1057, 0x00000000},
	{0x1058, 0x00000000},
	{0x1059, 0x3F800000},
	{0x105A, 0x00000000},
	{0x105B, 0x00000000},
	{0x105C, 0x3F800000},
	{0x105D, 0x00000000},
	{0x105E, 0x00000000},
	{0x1063, 0x3F800000},
	{0x1066, 0x3F800000},
	{0x1069, 0x3F800000},
	{0x106C, 0x3F800000},
	{0x1073, 0x00000000},
	{0x1076, 0x3F800000},
	{0x1079, 0x3F800000},
	{0x107C, 0x3F800000},
	{0x1083, 0x38D1B700},
	{0x1086, 0x00000000},
	{0x1089, 0x00000000},
	{0x108C, 0x00000000},
	{0x1093, 0x00000000},
	{0x1098, 0x3F800000},
	{0x1099, 0x3F800000},
	{0x109A, 0x3F800000},
	{0x10A1, 0x3C58B440},
	{0x10A2, 0x3C58B440},
	{0x10A3, 0x3F793A40},
	{0x10A4, 0x3C58B440},
	{0x10A5, 0x3C58B440},
	{0x10A6, 0x3F793A40},
	{0x10A7, 0x3F800000},
	{0x10A8, 0x00000000},
	{0x10A9, 0x00000000},
	{0x10AA, 0x00000000},
	{0x10AB, 0x3BDA2580},
	{0x10AC, 0x3BDA2580},
	{0x10AD, 0x3F7C9780},
	{0x10B0, 0x3F800000},
	{0x10B1, 0x00000000},
	{0x10B2, 0x00000000},
	{0x10B3, 0x3F800000},
	{0x10B4, 0x00000000},
	{0x10B5, 0x00000000},
	{0x10B6, 0x3F353C00},
	{0x10B8, 0x3F800000},
	{0x10B9, 0x00000000},
	{0x10C0, 0x3F944EC0},
	{0x10C1, 0xBF925540},
	{0x10C2, 0x3F5414C0},
	{0x10C3, 0x3F800000},
	{0x10C4, 0x00000000},
	{0x10C5, 0x00000000},
	{0x10C6, 0x3D506F00},
	{0x10C7, 0x3D506F00},
	{0x10C8, 0x3F65F240},
	{0x10C9, 0x3C208400},
	{0x10CA, 0x3C208400},
	{0x10CB, 0x3F7FE940},
	{0x10CC, 0x3E1D2100},
	{0x10CD, 0xBE1C0980},
	{0x10CE, 0x3F7F5080},
	{0x10D0, 0x3FFF64C0},
	{0x10D1, 0x00000000},
	{0x10D2, 0x3F800000},
	{0x10D3, 0x3F800000},
	{0x10D4, 0x3F800000},
	{0x10D5, 0x3F800000},
	{0x10D7, 0x41FCFB80},
	{0x10D8, 0x00000000},
	{0x10D9, 0x00000000},
	{0x10DA, 0x3F649140},
	{0x10DB, 0xBFD21D40},
	{0x10DC, 0x3FD21D40},
	{0x10DD, 0x3F4F0940},
	{0x10DE, 0xBF339A80},
	{0x10E0, 0x3DD17800},
	{0x10E1, 0x3DD17800},
	{0x10E2, 0x3F4BA200},
	{0x10E3, 0x00000000},
	{0x10E4, 0x00000000},
	{0x10E5, 0x3F800000},
	{0x10E8, 0x3F800000},
	{0x10E9, 0x00000000},
	{0x10EA, 0x00000000},
	{0x10EB, 0x00000000},
	{0x10F0, 0x3F800000},
	{0x10F1, 0x00000000},
	{0x10F2, 0x00000000},
	{0x10F3, 0x00000000},
	{0x10F4, 0x00000000},
	{0x10F5, 0x3F800000},
	{0x10F6, 0x00000000},
	{0x10F7, 0x00000000},
	{0x10F8, 0x00000000},
	{0x10F9, 0x00000000},
	{0x1200, 0x00000000},
	{0x1201, 0x3F800000},
	{0x1202, 0x3F800000},
	{0x1203, 0x3F800000},
	{0x1204, 0x3F800000},
	{0x1205, 0x00000000},
	{0x1206, 0x00000000},
	{0x1207, 0x3F800000},
	{0x1208, 0x00000000},
	{0x1209, 0x00000000},
	{0x120A, 0x3F800000},
	{0x120B, 0x00000000},
	{0x120C, 0x00000000},
	{0x120D, 0x3F800000},
	{0x120E, 0x00000000},
	{0x120F, 0x00000000},
	{0x1210, 0x3F800000},
	{0x1211, 0x00000000},
	{0x1212, 0x00000000},
	{0x1213, 0x3F800000},
	{0x1214, 0x3F800000},
	{0x1215, 0x3F800000},
	{0x1216, 0x3F800000},
	{0x1217, 0x3F800000},
	{0x1218, 0x00000000},
	{0x1219, 0x00000000},
	{0x121A, 0x00000000},
	{0x121B, 0x00000000},
	{0x121C, 0x00000000},
	{0x121D, 0x3F800000},
	{0x121E, 0x3F800000},
	{0x121F, 0x3F800000},
	{0x1235, 0x3F800000},
	{0x1236, 0x3F800000},
	{0x1237, 0x3F800000},
	{0x1238, 0x3F800000},
	{0xFFFF, 0xFFFFFFFF}
};

void IniSet(void)
{
	/* Command Execute Process Initial */
	IniCmd();
	/* Clock Setting */
	IniClk();
	/* I/O Port Initial Setting */
	IniIop();
	/* DigitalGyro Initial Setting */
	IniDgy();
	/* Monitor & Other Initial Setting */
	IniMon();
	/* Servo Initial Setting */
	IniSrv();
	/* Gyro Filter Initial Setting */
	IniGyr();
	/* Gyro Filter Initial Setting */
	IniFil();
	/* Adjust Fix Value Setting */
	IniAdj();
}

void IniSetAf(void)
{
	/* Command Execute Process Initial */
	IniCmd();
	/* Clock Setting */
	IniClk();
	/* AF Initial Setting */
	IniAf();
}

void IniClk(void)
{
	ChkCvr(); /* Read Cver */

	/*OSC Enables */
	UcOscAdjFlg = 0; /* Osc adj flag */

#ifdef DEF_SET
	/*OSC ENABLE */
	RegWriteA_LC898122AF(OSCSTOP, 0x00);  /* 0x0256 */
	RegWriteA_LC898122AF(OSCSET, 0x90);   /* 0x0257       OSC ini */
	RegWriteA_LC898122AF(OSCCNTEN, 0x00); /* 0x0258       OSC Cnt disable */
#endif
	/*Clock Enables */
	RegWriteA_LC898122AF(CLKON, 0x1F); /* 0x020B */

#ifdef USE_EXTCLK_ALL
	RegWriteA_LC898122AF(CLKSEL, 0x07); /* 0x020C       All */
#else
#ifdef USE_EXTCLK_PWM
	RegWriteA_LC898122AF(CLKSEL, 0x01); /* 0x020C       only PWM */
#else
#ifdef DEF_SET
	RegWriteA_LC898122AF(CLKSEL, 0x00); /* 0x020C */
#endif
#endif
#endif

#ifdef USE_EXTCLK_ALL			     /* 24MHz */
	RegWriteA_LC898122AF(PWMDIV, 0x00);   /* 0x0210       24MHz/1 */
	RegWriteA_LC898122AF(SRVDIV, 0x00);   /* 0x0211       24MHz/1 */
	RegWriteA_LC898122AF(GIFDIV, 0x02);   /* 0x0212       24MHz/2 = 12MHz */
	RegWriteA_LC898122AF(AFPWMDIV, 0x00); /* 0x0213       24MHz/1 = 24MHz */
	RegWriteA_LC898122AF(OPAFDIV, 0x02);  /* 0x0214       24MHz/2 = 12MHz */
#else
#ifdef DEF_SET
	RegWriteA_LC898122AF(PWMDIV, 0x00); /* 0x0210       48MHz/1 */
	RegWriteA_LC898122AF(SRVDIV, 0x00); /* 0x0211       48MHz/1 */
	RegWriteA_LC898122AF(GIFDIV, 0x03); /* 0x0212       48MHz/3 = 16MHz */
#ifdef AF_PWMMODE
	RegWriteA_LC898122AF(AFPWMDIV, 0x00); /* 0x0213       48MHz/1 */
#else
	RegWriteA_LC898122AF(AFPWMDIV, 0x02); /* 0x0213       48MHz/2 = 24MHz */
#endif
	/*
	 * OPAFDIV dipende dal modulo: 0x06 (48MHz/6 = 8MHz) contro lo 0x04 di
	 * ALPS. Due chiamate distinte e non un ternario, perche' la fabbrica
	 * dirama -- "54000061 b.ne"@0xffffff80087491fc -- invece di fare un
	 * csel, ed e' quel che clang produce da un if/else con due chiamate
	 * di cui poi fonde la coda.
	 */
	if (g9c96cbc == 2)
		RegWriteA_LC898122AF(OPAFDIV, 0x06);
	else
		RegWriteA_LC898122AF(OPAFDIV, 0x04);
#endif
#endif
}

void IniIop(void)
{
#ifdef DEF_SET
	/*set IOP direction */
	RegWriteA_LC898122AF(P0LEV, 0x00);
	RegWriteA_LC898122AF(P0DIR, 0x00);
	/*set pull up/down */
	RegWriteA_LC898122AF(P0PON, 0x0F);
	RegWriteA_LC898122AF(P0PUD, 0x0F);
#endif
/*select IOP signal */
#ifdef USE_3WIRE_DGYRO
	RegWriteA_LC898122AF(IOP1SEL, 0x02); /* 0x0231       IOP1 : IOP1 */
#else
	RegWriteA_LC898122AF(
		IOP1SEL,
		0x00); /* 0x0231       IOP1 : DGDATAIN (ATT:0236h[0]=1) */
#endif
#ifdef DEF_SET
	RegWriteA_LC898122AF(IOP0SEL, 0x02); /* 0x0230       IOP0 : IOP0 */
	RegWriteA_LC898122AF(IOP2SEL, 0x02); /* 0x0232       IOP2 : IOP2 */
	RegWriteA_LC898122AF(IOP3SEL, 0x00); /* 0x0233       IOP3 : DGDATAOUT */
	RegWriteA_LC898122AF(IOP4SEL, 0x00); /* 0x0234       IOP4 : DGSCLK */
	RegWriteA_LC898122AF(IOP5SEL, 0x00); /* 0x0235       IOP5 : DGSSB */
	RegWriteA_LC898122AF(DGINSEL,
			    0x00); /* 0x0236       DGDATAIN 0:IOP1 1:IOP2 */
	RegWriteA_LC898122AF(I2CSEL,
			    0x00); /* 0x0248       I2C noise reduction ON */
	RegWriteA_LC898122AF(DLMODE, 0x00); /* 0x0249       Download OFF */
#endif
}

void IniDgy(void)
{
#ifdef USE_INVENSENSE
	unsigned char UcGrini;
#endif

/*************/
/*For ST gyro */
/*************/

/*Set SPI Type */
#ifdef USE_3WIRE_DGYRO
	RegWriteA_LC898122AF(SPIM, 0x00);
#else
	RegWriteA_LC898122AF(SPIM, 0x01);
#endif
	/* DGSPI4  0: 3-wire SPI, 1: 4-wire SPI */

	/*Set to Command Mode */
	RegWriteA_LC898122AF(GRSEL, 0x01);

	/*Digital Gyro Read settings */
	RegWriteA_LC898122AF(GRINI, 0x80);

#ifdef USE_INVENSENSE

	RegReadA_LC898122AF(GRINI, &UcGrini);
	RegWriteA_LC898122AF(GRINI, (UcGrini | SLOWMODE));

	RegWriteA_LC898122AF(GRADR0, 0x6A); /* 0x0283       Set I2C_DIS */
	RegWriteA_LC898122AF(GSETDT, 0x10); /* 0x028A       Set Write Data */
	RegWriteA_LC898122AF(GRACC, 0x10); /* 0x0282       Set Trigger ON */
	AccWit(0x10);

	RegWriteA_LC898122AF(GRADR0, 0x1B); /* 0x0283       Set GYRO_CONFIG */
	RegWriteA_LC898122AF(GSETDT,
			    (FS_SEL << 3)); /* 0x028A       Set Write Data */
	RegWriteA_LC898122AF(GRACC, 0x10); /* 0x0282       Set Trigger ON */
	AccWit(0x10); /* Digital Gyro busy wait                               */

	RegReadA_LC898122AF(GRINI, &UcGrini);
	RegWriteA_LC898122AF(GRINI, (UcGrini & ~SLOWMODE));
/* 0x0281       [ PARA_REG | AXIS7EN | AXIS4EN | - ][ - | SLOWMODE | - | - ] */

#endif

	RegWriteA_LC898122AF(
		RDSEL,
		0x7C); /* 0x028B       RDSEL(Data1 and 2 for continuos mode) */

	GyOutSignal();
}

void IniMon(void)
{
	RegWriteA_LC898122AF(PWMMONA, 0x00); /* 0x0030       0:off */

	RegWriteA_LC898122AF(MONSELA, 0x5C); /* 0x0270       DLYMON1 */
	RegWriteA_LC898122AF(MONSELB, 0x5D); /* 0x0271       DLYMON2 */
	RegWriteA_LC898122AF(MONSELC, 0x00); /* 0x0272 */
	RegWriteA_LC898122AF(MONSELD, 0x00); /* 0x0273 */

	/* Monitor Circuit */
	RegWriteA_LC898122AF(WC_PINMON1,
			    0x00); /* 0x01C0               Filter Monitor */
	RegWriteA_LC898122AF(WC_PINMON2, 0x00); /* 0x01C1 */
	RegWriteA_LC898122AF(WC_PINMON3, 0x00); /* 0x01C2 */
	RegWriteA_LC898122AF(WC_PINMON4, 0x00); /* 0x01C3 */
	/* Delay Monitor */
	RegWriteA_LC898122AF(WC_DLYMON11, 0x04); /* 0x01C5 DlyMonAdd1[10:8] */
	RegWriteA_LC898122AF(WC_DLYMON10, 0x40); /* 0x01C4 DlyMonAdd1[ 7:0] */
	RegWriteA_LC898122AF(WC_DLYMON21, 0x04); /* 0x01C7 DlyMonAdd2[10:8] */
	RegWriteA_LC898122AF(WC_DLYMON20, 0xC0); /* 0x01C6 DlyMonAdd2[ 7:0] */
	RegWriteA_LC898122AF(WC_DLYMON31, 0x00); /* 0x01C9 DlyMonAdd3[10:8] */
	RegWriteA_LC898122AF(WC_DLYMON30, 0x00); /* 0x01C8 DlyMonAdd3[ 7:0] */
	RegWriteA_LC898122AF(WC_DLYMON41, 0x00); /* 0x01CB DlyMonAdd4[10:8] */
	RegWriteA_LC898122AF(WC_DLYMON40, 0x00); /* 0x01CA DlyMonAdd4[ 7:0] */

	/* Monitor */
	RegWriteA_LC898122AF(PWMMONA, 0x80); /* 0x0030       1:on */
 /**/}


void IniSrv(void)
{
	unsigned char UcStbb0;

	UcPwmMod = INIT_PWMMODE; /* Driver output mode */

	RegWriteA_LC898122AF(WC_EQON,
			     0x00); /* 0x0101               Filter Calcu */
	RegWriteA_LC898122AF(WC_RAMINITON, 0x00); /* 0x0102 */
	ClrGyr(0x0000, CLR_ALL_RAM);		  /* All Clear */

	RegWriteA_LC898122AF(WH_EQSWX, 0x02);
	RegWriteA_LC898122AF(WH_EQSWY, 0x02);

	RamAccFixMod(OFF); /* 32bit Float mode */

	/* Monitor Gain */
	RamWrite32A_LC898122AF(dm1g, 0x3F800000); /* 0x109A */
	RamWrite32A_LC898122AF(dm2g, 0x3F800000); /* 0x109B */
	RamWrite32A_LC898122AF(dm3g, 0x3F800000); /* 0x119A */
	RamWrite32A_LC898122AF(dm4g, 0x3F800000); /* 0x119B */

	/* Hall output limitter */
	RamWrite32A_LC898122AF(
		sxlmta1,
		0x3F800000); /* 0x10E6               Hall X output Limit */
	RamWrite32A_LC898122AF(
		sylmta1,
		0x3F800000); /* 0x11E6               Hall Y output Limit */

	/* Emargency Stop */
	RegWriteA_LC898122AF(
		WH_EMGSTPON,
		0x00); /* 0x0178               Emargency Stop OFF */
	RegWriteA_LC898122AF(WH_EMGSTPTMR,
			     0xFF); /* 0x017A 255*(16/23.4375kHz)=174ms */

	RamWrite32A_LC898122AF(sxemglev, 0x3F800000);
	RamWrite32A_LC898122AF(syemglev, 0x3F800000);

	/* Hall Servo smoothing */
	RegWriteA_LC898122AF(WH_SMTSRVON,
			     0x00); /* 0x017C               Smooth Servo OFF */
#ifdef USE_EXTCLK_ALL		    /* 24MHz */
	RegWriteA_LC898122AF(WH_SMTSRVSMP,
			     0x03); /* 0x017D 2.7ms=2^03/11.718kHz */
	RegWriteA_LC898122AF(WH_SMTTMR,
			     0x00); /* 0x017E 1.3ms=(0+1)*16/11.718kHz */
#else
	RegWriteA_LC898122AF(WH_SMTSRVSMP,
			     0x06); /* 0x017D 2.7ms=2^06/23.4375kHz */
	RegWriteA_LC898122AF(WH_SMTTMR,
			     0x0F); /* 0x017E */
#endif

	RamWrite32A_LC898122AF(sxsmtav, 0xBC800000);
	RamWrite32A_LC898122AF(sysmtav, 0xBC800000);
	RamWrite32A_LC898122AF(sxsmtstp, 0x3AE90466);
	RamWrite32A_LC898122AF(sysmtstp, 0x3AE90466);

	/* High-dimensional correction  */
	RegWriteA_LC898122AF(WH_HOFCON,
			     0x11); /* 0x0174               OUT 3x3 */

	/* Front */
	RamWrite32A_LC898122AF(sxiexp3, A3_IEXP3);   /* 0x10BA */
	RamWrite32A_LC898122AF(sxiexp2, 0x00000000); /* 0x10BB */
	RamWrite32A_LC898122AF(sxiexp1, A1_IEXP1);   /* 0x10BC */
	RamWrite32A_LC898122AF(sxiexp0, 0x00000000); /* 0x10BD */
	RamWrite32A_LC898122AF(sxiexp, 0x3F800000);  /* 0x10BE */

	RamWrite32A_LC898122AF(syiexp3, A3_IEXP3);   /* 0x11BA */
	RamWrite32A_LC898122AF(syiexp2, 0x00000000); /* 0x11BB */
	RamWrite32A_LC898122AF(syiexp1, A1_IEXP1);   /* 0x11BC */
	RamWrite32A_LC898122AF(syiexp0, 0x00000000); /* 0x11BD */
	RamWrite32A_LC898122AF(syiexp, 0x3F800000);  /* 0x11BE */

	/* Back */
	RamWrite32A_LC898122AF(sxoexp3, A3_IEXP3);   /* 0x10FA */
	RamWrite32A_LC898122AF(sxoexp2, 0x00000000); /* 0x10FB */
	RamWrite32A_LC898122AF(sxoexp1, A1_IEXP1);   /* 0x10FC */
	RamWrite32A_LC898122AF(sxoexp0, 0x00000000); /* 0x10FD */
	RamWrite32A_LC898122AF(sxoexp, 0x3F800000);  /* 0x10FE */

	RamWrite32A_LC898122AF(syoexp3, A3_IEXP3);   /* 0x11FA */
	RamWrite32A_LC898122AF(syoexp2, 0x00000000); /* 0x11FB */
	RamWrite32A_LC898122AF(syoexp1, A1_IEXP1);   /* 0x11FC */
	RamWrite32A_LC898122AF(syoexp0, 0x00000000); /* 0x11FD */
	RamWrite32A_LC898122AF(syoexp, 0x3F800000);  /* 0x11FE */

 /* Sine wave */
#ifdef DEF_SET
	RegWriteA_LC898122AF(WC_SINON,
			     0x00); /* 0x0180               Sin Wave off */
	RegWriteA_LC898122AF(WC_SINFRQ0, 0x00); /* 0x0181 */
	RegWriteA_LC898122AF(WC_SINFRQ1, 0x60); /* 0x0182 */
	RegWriteA_LC898122AF(WC_SINPHSX, 0x00); /* 0x0183 */
	RegWriteA_LC898122AF(WC_SINPHSY, 0x20); /* 0x0184 */

	/* AD over sampling */
	RegWriteA_LC898122AF(WC_ADMODE,
			     0x06); /* 0x0188               AD Over Sampling */

	/* Measure mode */
	RegWriteA_LC898122AF(WC_MESMODE, 0x00); /* 0x0190 Measurement Mode */
	RegWriteA_LC898122AF(WC_MESSINMODE, 0x00); /* 0x0191 */
	RegWriteA_LC898122AF(WC_MESLOOP0, 0x08);   /* 0x0192 */
	RegWriteA_LC898122AF(WC_MESLOOP1, 0x02);   /* 0x0193 */
	RegWriteA_LC898122AF(WC_MES1ADD0, 0x00);   /* 0x0194 */
	RegWriteA_LC898122AF(WC_MES1ADD1, 0x00);   /* 0x0195 */
	RegWriteA_LC898122AF(WC_MES2ADD0, 0x00);   /* 0x0196 */
	RegWriteA_LC898122AF(WC_MES2ADD1, 0x00);   /* 0x0197 */
	RegWriteA_LC898122AF(WC_MESABS, 0x00);     /* 0x0198 */
	RegWriteA_LC898122AF(WC_MESWAIT, 0x00);    /* 0x0199 */

	/* auto measure */
	RegWriteA_LC898122AF(
		WC_AMJMODE,
		0x00); /* 0x01A0               Automatic measurement mode */

	RegWriteA_LC898122AF(WC_AMJLOOP0, 0x08); /* 0x01A2 Self-Aadjustment */
	RegWriteA_LC898122AF(WC_AMJLOOP1, 0x02); /* 0x01A3 */
	RegWriteA_LC898122AF(WC_AMJIDL0, 0x02);  /* 0x01A4 */
	RegWriteA_LC898122AF(WC_AMJIDL1, 0x00);  /* 0x01A5 */
	RegWriteA_LC898122AF(WC_AMJ1ADD0, 0x00); /* 0x01A6 */
	RegWriteA_LC898122AF(WC_AMJ1ADD1, 0x00); /* 0x01A7 */
	RegWriteA_LC898122AF(WC_AMJ2ADD0, 0x00); /* 0x01A8 */
	RegWriteA_LC898122AF(WC_AMJ2ADD1, 0x00); /* 0x01A9 */

	/* Data Pass */
	RegWriteA_LC898122AF(WC_DPI1ADD0,
			     0x00); /* 0x01B0               Data Pass */
	RegWriteA_LC898122AF(WC_DPI1ADD1, 0x00); /* 0x01B1 */
	RegWriteA_LC898122AF(WC_DPI2ADD0, 0x00); /* 0x01B2 */
	RegWriteA_LC898122AF(WC_DPI2ADD1, 0x00); /* 0x01B3 */
	RegWriteA_LC898122AF(WC_DPI3ADD0, 0x00); /* 0x01B4 */
	RegWriteA_LC898122AF(WC_DPI3ADD1, 0x00); /* 0x01B5 */
	RegWriteA_LC898122AF(WC_DPI4ADD0, 0x00); /* 0x01B6 */
	RegWriteA_LC898122AF(WC_DPI4ADD1, 0x00); /* 0x01B7 */
	RegWriteA_LC898122AF(WC_DPO1ADD0,
			     0x00); /* 0x01B8               Data Pass */
	RegWriteA_LC898122AF(WC_DPO1ADD1, 0x00); /* 0x01B9 */
	RegWriteA_LC898122AF(WC_DPO2ADD0, 0x00); /* 0x01BA */
	RegWriteA_LC898122AF(WC_DPO2ADD1, 0x00); /* 0x01BB */
	RegWriteA_LC898122AF(WC_DPO3ADD0, 0x00); /* 0x01BC */
	RegWriteA_LC898122AF(WC_DPO3ADD1, 0x00); /* 0x01BD */
	RegWriteA_LC898122AF(WC_DPO4ADD0, 0x00); /* 0x01BE */
	RegWriteA_LC898122AF(WC_DPO4ADD1, 0x00); /* 0x01BF */
	RegWriteA_LC898122AF(WC_DPON,
			     0x00); /* 0x0105               Data pass OFF */

	/* Interrupt Flag */
	RegWriteA_LC898122AF(WC_INTMSK,
			     0xFF); /* 0x01CE               All Mask */

#endif

	/* Ram Access */
	RamAccFixMod(OFF); /* 32bit float mode */

	/* PWM Signal Generate */
	DrvSw(OFF); /* 0x0070       Drvier Block Ena=0 */
	RegWriteA_LC898122AF(
		DRVFC2, 0x90); /* 0x0002       Slope 3, Dead Time = 30 ns */
	RegWriteA_LC898122AF(
		DRVSELX,
		0xFF); /* 0x0003       PWM X drv max current  DRVSELX[7:0] */
	RegWriteA_LC898122AF(
		DRVSELY,
		0xFF); /* 0x0004       PWM Y drv max current  DRVSELY[7:0] */

#ifdef PWM_BREAK
#ifdef PWM_CAREER_TEST
	RegWriteA_LC898122AF(PWMFC, 0x7C);
#else  /* PWM_CAREER_TEST */
	if (UcCvrCod == CVER122)
		RegWriteA_LC898122AF(PWMFC, 0x2D);
	else
		RegWriteA_LC898122AF(PWMFC, 0x1D);
#endif /* PWM_CAREER_TEST */
#else
	RegWriteA_LC898122AF(PWMFC, 0x21);
#endif

#ifdef USE_VH_SYNC
	RegWriteA_LC898122AF(STROBEFC,
			     0x80); /* 0x001C       外?入力Strobe信?の有効 */
	RegWriteA_LC898122AF(STROBEDLYX, 0x00); /* 0x001D       Delay */
	RegWriteA_LC898122AF(STROBEDLYY, 0x00); /* 0x001E       Delay */
#endif						/* USE_VH_SYNC */

	RegWriteA_LC898122AF(PWMA, 0x00); /* 0x0010       PWM X/Y standby */
	RegWriteA_LC898122AF(PWMDLYX,
			     0x04); /* 0x0012       X Phase Delay Setting */
	RegWriteA_LC898122AF(PWMDLYY,
			     0x04); /* 0x0013       Y Phase Delay Setting */

#ifdef DEF_SET
	RegWriteA_LC898122AF(DRVCH1SEL,
			     0x00); /* 0x0005       OUT1/OUT2       X axis */
	RegWriteA_LC898122AF(DRVCH2SEL,
			     0x00); /* 0x0006       OUT3/OUT4       Y axis */

	RegWriteA_LC898122AF(PWMDLYTIMX,
			     0x00); /* 0x0014               PWM Timing */
	RegWriteA_LC898122AF(PWMDLYTIMY,
			     0x00); /* 0x0015               PWM Timing */
#endif

	if (UcCvrCod == CVER122) {
#ifdef PWM_CAREER_TEST
		RegWriteA_LC898122AF(PWMPERIODY, 0xD0);
		RegWriteA_LC898122AF(PWMPERIODY2, 0xD0);
#else /* PWM_CAREER_TEST */
		RegWriteA_LC898122AF(PWMPERIODY, 0x00);
		RegWriteA_LC898122AF(PWMPERIODY2, 0x00);
#endif
	} else {
#ifdef PWM_CAREER_TEST
		RegWriteA_LC898122AF(
			PWMPERIODX,
			0xF2); /* 0x0018               PWM Carrier Freq */
		RegWriteA_LC898122AF(
			PWMPERIODX2,
			0x00); /* 0x0019               PWM Carrier Freq */
		RegWriteA_LC898122AF(
			PWMPERIODY,
			0xF2); /* 0x001A               PWM Carrier Freq */
		RegWriteA_LC898122AF(
			PWMPERIODY2,
			0x00); /* 0x001B               PWM Carrier Freq */
#else				/* PWM_CAREER_TEST */
		RegWriteA_LC898122AF(
			PWMPERIODX,
			0x00); /* 0x0018               PWM Carrier Freq */
		RegWriteA_LC898122AF(
			PWMPERIODX2,
			0x00); /* 0x0019               PWM Carrier Freq */
		RegWriteA_LC898122AF(
			PWMPERIODY,
			0x00); /* 0x001A               PWM Carrier Freq */
		RegWriteA_LC898122AF(
			PWMPERIODY2,
			0x00); /* 0x001B               PWM Carrier Freq */
#endif
	}

	/* Linear PWM circuit setting */
	RegWriteA_LC898122AF(CVA,
			     0xC0); /* 0x0020       Linear PWM mode enable */

	if (UcCvrCod == CVER122)
		RegWriteA_LC898122AF(CVFC, 0x22); /* 0x0021 */

	RegWriteA_LC898122AF(CVFC2, 0x80); /* 0x0022 */
	if (UcCvrCod == CVER122) {
		RegWriteA_LC898122AF(CVSMTHX,
				     0x00); /* 0x0023       smooth off */
		RegWriteA_LC898122AF(CVSMTHY,
				     0x00); /* 0x0024       smooth off */
	}

	RegReadA_LC898122AF(STBB0, &UcStbb0);

	UcStbb0 &= 0x80;
	RegWriteA_LC898122AF(STBB0, UcStbb0); /* 0x0250       OIS standby */
}

#ifdef GAIN_CONT
#define TRI_LEVEL 0x3A83126F
#define TIMELOW 0x50	/* */
#define TIMEHGH 0x05	/* */
#ifdef USE_EXTCLK_ALL	/* 24MHz */
#define TIMEBSE 0x2F	/* 4.0ms */
#else
#define TIMEBSE 0x5D /* 3.96ms */
#endif
#define MONADR GXXFZ
#define GANADR gxadj
#define XMINGAIN 0x00000000
#define XMAXGAIN 0x3F800000
#define YMINGAIN 0x00000000
#define YMAXGAIN 0x3F800000
#define XSTEPUP 0x38D1B717 /* 0.0001        */
#define XSTEPDN 0xBD4CCCCD /* -0.05         */
#define YSTEPUP 0x38D1B717 /* 0.0001        */
#define YSTEPDN 0xBD4CCCCD /* -0.05         */
#endif

void IniGyr(void)
{
	/*
	 * IL #ifdef CORRECT_1DEG E' DIVENTATO UNA SCELTA A ESECUZIONE.
	 *
	 * Quattro csel in cima -- "9a890156 csel"@0xffffff8008749bc8 e i tre
	 * che seguono -- sullo stesso g9c96cbc, tutti prima della prima
	 * chiamata. Quattro locali, per la stessa ragione di S2cPro: usarli
	 * come macro farebbe rileggere la variabile dopo ogni bl.
	 *
	 * E i valori del MODULO 2 sono quelli della colonna CORRECT_1DEG di
	 * ALPS: 0x40400000 per il limite 4, 0x40000000 e 0x3F800000 per la
	 * soglia alta e media. Lo stesso vale per MAXLMT in SetH1cMod. La
	 * fabbrica ha preso una scelta che in ALPS si fa a compilazione e
	 * l'ha messa sul modulo -- poi ha ritarato l'altra colonna, che
	 * infatti non coincide con nessuno dei due rami di ALPS.
	 *
	 * GYRLMT3 e' l'eccezione: 0x3EE66666 sul modulo 2 contro lo
	 * 0x3F19999A di CORRECT_1DEG. Ritarato anche quello.
	 *
	 * I registri si leggono da dove il valore va a finire, non
	 * dall'ordine dei csel: x22 in 0x1029/0x102A (limite 3), x21 in
	 * 0x102B/0x102C (limite 4), x20 in 0x104F, x19 in 0x105F. Attenzione
	 * che x21 e x22 vengono RIASSEGNATI a meta' funzione con due trucchi
	 * aritmetici (`add x21, x23, #0xd` e `sub x22, x22, #0x800, lsl #12`)
	 * per costruire altre costanti: le loro apparizioni dopo quel punto
	 * non c'entrano con la scelta di modulo.
	 */
	unsigned long UlGyrLmt3 = (g9c96cbc == 2) ? 0x3EE66666 : 0x3F0F5C29;
	unsigned long UlGyrLmt4 = (g9c96cbc == 2) ? 0x40400000 : 0x40333333;
	unsigned long UlGyrA12H = (g9c96cbc == 2) ? 0x40000000 : 0x401CCCCD;
	unsigned long UlGyrA12M = (g9c96cbc == 2) ? 0x3F800000 : 0x3FB33333;


	/*Gyro Filter Setting */
	RegWriteA_LC898122AF(WG_EQSW, 0x03);

	/*Gyro Filter Down Sampling */

	RegWriteA_LC898122AF(WG_SHTON, 0x10);
 /* CmShtOpe[1:0] 00: シ?ッターOFF, 01: シ?ッターON, 1x:外?制御 */

#ifdef DEF_SET
	RegWriteA_LC898122AF(WG_SHTDLYTMR, 0x00); /* 0x0117 Shutter Delay */
	RegWriteA_LC898122AF(WG_GADSMP,
			     0x00); /* 0x011C               Sampling timing */
	RegWriteA_LC898122AF(WG_HCHR, 0x00);
	RegWriteA_LC898122AF(WG_LMT3MOD, 0x00);
	/* CmLmt3Mod       0: 通常?ミッター動作, 1: 円の半径?ミッター動作 */
	RegWriteA_LC898122AF(WG_VREFADD, 0x12);
#endif
	RegWriteA_LC898122AF(WG_SHTMOD, 0x06);

	/* Limiter */
	RamWrite32A_LC898122AF(gxlmt1H, GYRLMT1H); /* 0x1028 */
	RamWrite32A_LC898122AF(gylmt1H, GYRLMT1H); /* 0x1128 */

	RamWrite32A_LC898122AF(gxlmt3HS0, UlGyrLmt3); /* 0x1029 */
	RamWrite32A_LC898122AF(gylmt3HS0, UlGyrLmt3); /* 0x1129 */

	RamWrite32A_LC898122AF(gxlmt3HS1, UlGyrLmt3); /* 0x102A */
	RamWrite32A_LC898122AF(gylmt3HS1, UlGyrLmt3); /* 0x112A */

	RamWrite32A_LC898122AF(
		gylmt4HS0,
		UlGyrLmt4); /* 0x112B        Y軸Limiter4 High?値0 */
	RamWrite32A_LC898122AF(
		gxlmt4HS0,
		UlGyrLmt4); /* 0x102B        X軸Limiter4 High?値0 */

	RamWrite32A_LC898122AF(
		gxlmt4HS1,
		UlGyrLmt4); /* 0x102C        X軸Limiter4 High?値1 */
	RamWrite32A_LC898122AF(
		gylmt4HS1,
		UlGyrLmt4); /* 0x112C        Y軸Limiter4 High?値1 */

	/* Pan/Tilt parameter */
	RegWriteA_LC898122AF(WG_PANADDA,
			     0x12); /* 0x0130       GXH1Z2/GYH1Z2 Select */
	RegWriteA_LC898122AF(WG_PANADDB,
			     0x09); /* 0x0131       GXIZ/GYIZ Select */

	/* Threshold */
	RamWrite32A_LC898122AF(SttxHis, 0x00000000);  /* 0x1226 */
	RamWrite32A_LC898122AF(SttxaL, 0x00000000);   /* 0x109D */
	RamWrite32A_LC898122AF(SttxbL, 0x00000000);   /* 0x109E */
	RamWrite32A_LC898122AF(Sttx12aM, UlGyrA12M); /* 0x104F */
	RamWrite32A_LC898122AF(Sttx12aH, UlGyrA12H); /* 0x105F */
	RamWrite32A_LC898122AF(Sttx12bM, GYRB12_MID); /* 0x106F */
	RamWrite32A_LC898122AF(Sttx12bH, GYRB12_HGH); /* 0x107F */
	RamWrite32A_LC898122AF(Sttx34aM, GYRA34_MID); /* 0x108F */
	RamWrite32A_LC898122AF(Sttx34aH, GYRA34_HGH); /* 0x109F */
	RamWrite32A_LC898122AF(Sttx34bM, GYRB34_MID); /* 0x10AF */
	RamWrite32A_LC898122AF(Sttx34bH, GYRB34_HGH); /* 0x10BF */
	RamWrite32A_LC898122AF(SttyaL, 0x00000000);   /* 0x119D */
	RamWrite32A_LC898122AF(SttybL, 0x00000000);   /* 0x119E */
	RamWrite32A_LC898122AF(Stty12aM, UlGyrA12M); /* 0x114F */
	RamWrite32A_LC898122AF(Stty12aH, UlGyrA12H); /* 0x115F */
	RamWrite32A_LC898122AF(Stty12bM, GYRB12_MID); /* 0x116F */
	RamWrite32A_LC898122AF(Stty12bH, GYRB12_HGH); /* 0x117F */
	RamWrite32A_LC898122AF(Stty34aM, GYRA34_MID); /* 0x118F */
	RamWrite32A_LC898122AF(Stty34aH, GYRA34_HGH); /* 0x119F */
	RamWrite32A_LC898122AF(Stty34bM, GYRB34_MID); /* 0x11AF */
	RamWrite32A_LC898122AF(Stty34bH, GYRB34_HGH); /* 0x11BF */

	/* Pan level */
	RegWriteA_LC898122AF(WG_PANLEVABS, 0x00); /* 0x0133 */

	/* Average parameter are set IniAdj */

	/* Phase Transition Setting */
	/* State 2 -> 1 */
	RegWriteA_LC898122AF(WG_PANSTT21JUG0, 0x00); /* 0x0140 */
	RegWriteA_LC898122AF(WG_PANSTT21JUG1, 0x00); /* 0x0141 */
	/* State 3 -> 1 */
	RegWriteA_LC898122AF(WG_PANSTT31JUG0, 0x00); /* 0x0142 */
	RegWriteA_LC898122AF(WG_PANSTT31JUG1, 0x00); /* 0x0143 */
	/* State 4 -> 1 */
	RegWriteA_LC898122AF(WG_PANSTT41JUG0, 0x01); /* 0x0144 */
	RegWriteA_LC898122AF(WG_PANSTT41JUG1, 0x00); /* 0x0145 */
	/* State 1 -> 2 */
	RegWriteA_LC898122AF(WG_PANSTT12JUG0, 0x00); /* 0x0146 */
	RegWriteA_LC898122AF(WG_PANSTT12JUG1, 0x07); /* 0x0147 */
	/* State 1 -> 3 */
	RegWriteA_LC898122AF(WG_PANSTT13JUG0, 0x00); /* 0x0148 */
	RegWriteA_LC898122AF(WG_PANSTT13JUG1, 0x00); /* 0x0149 */
	/* State 2 -> 3 */
	RegWriteA_LC898122AF(WG_PANSTT23JUG0, 0x11); /* 0x014A */
	RegWriteA_LC898122AF(WG_PANSTT23JUG1, 0x00); /* 0x014B */
	/* State 4 -> 3 */
	RegWriteA_LC898122AF(WG_PANSTT43JUG0, 0x00); /* 0x014C */
	RegWriteA_LC898122AF(WG_PANSTT43JUG1, 0x00); /* 0x014D */
	/* State 3 -> 4 */
	RegWriteA_LC898122AF(WG_PANSTT34JUG0, 0x01); /* 0x014E */
	RegWriteA_LC898122AF(WG_PANSTT34JUG1, 0x00); /* 0x014F */
	/* State 2 -> 4 */
	RegWriteA_LC898122AF(WG_PANSTT24JUG0, 0x00); /* 0x0150 */
	RegWriteA_LC898122AF(WG_PANSTT24JUG1, 0x00); /* 0x0151 */
	/* State 4 -> 2 */
	RegWriteA_LC898122AF(WG_PANSTT42JUG0, 0x44); /* 0x0152 */
	RegWriteA_LC898122AF(WG_PANSTT42JUG1, 0x04); /* 0x0153 */

	/* State Timer */
	RegWriteA_LC898122AF(WG_PANSTT1LEVTMR, 0x00); /* 0x015B */
	RegWriteA_LC898122AF(WG_PANSTT2LEVTMR, 0x00); /* 0x015C */
	RegWriteA_LC898122AF(WG_PANSTT3LEVTMR, 0x00); /* 0x015D */
	RegWriteA_LC898122AF(WG_PANSTT4LEVTMR, 0x03); /* 0x015E */

	/* Control filter */
	RegWriteA_LC898122AF(WG_PANTRSON0,
			     0x11); /* 0x0132       USE I12/iSTP/Gain-Filter */

	/* State Setting */
	IniPtMovMod(OFF); /* Pan/Tilt setting (Still) */

	/* Hold */
	RegWriteA_LC898122AF(WG_PANSTTSETILHLD, 0x00); /* 0x015F */

	/* State2,4 Step Time Setting */
	RegWriteA_LC898122AF(WG_PANSTT2TMR0, 0x01); /* 0x013C */
	RegWriteA_LC898122AF(WG_PANSTT2TMR1, 0x00); /* 0x013D */
	RegWriteA_LC898122AF(WG_PANSTT4TMR0, 0x02); /* 0x013E */
	RegWriteA_LC898122AF(WG_PANSTT4TMR1, 0x07); /* 0x013F */

	RegWriteA_LC898122AF(WG_PANSTTXXXTH, 0x00); /* 0x015A */

#ifdef GAIN_CONT
	RamWrite32A_LC898122AF(gxlevlow, TRI_LEVEL); /* 0x10AE       Low Th */
	RamWrite32A_LC898122AF(gylevlow, TRI_LEVEL); /* 0x11AE       Low Th */
	RamWrite32A_LC898122AF(gxadjmin, XMINGAIN); /* 0x1094       Low gain */
	RamWrite32A_LC898122AF(gxadjmax, XMAXGAIN); /* 0x1095       Hgh gain */
	RamWrite32A_LC898122AF(gxadjdn, XSTEPDN);   /* 0x1096       -step */
	RamWrite32A_LC898122AF(gxadjup, XSTEPUP);   /* 0x1097       +step */
	RamWrite32A_LC898122AF(gyadjmin, YMINGAIN); /* 0x1194       Low gain */
	RamWrite32A_LC898122AF(gyadjmax, YMAXGAIN); /* 0x1195       Hgh gain */
	RamWrite32A_LC898122AF(gyadjdn, YSTEPDN);   /* 0x1196       -step */
	RamWrite32A_LC898122AF(gyadjup, YSTEPUP);   /* 0x1197       +step */

	RegWriteA_LC898122AF(
		WG_LEVADD,
		(unsigned char)MONADR); /* 0x0120       Input signal */
	RegWriteA_LC898122AF(WG_LEVTMR, TIMEBSE); /* 0x0123       Base Time */
	RegWriteA_LC898122AF(WG_LEVTMRLOW,
			     TIMELOW); /* 0x0121       X Low Time */
	RegWriteA_LC898122AF(WG_LEVTMRHGH,
			     TIMEHGH); /* 0x0122       X Hgh Time */
	RegWriteA_LC898122AF(
		WG_ADJGANADD,
		(unsigned char)GANADR); /* 0x0128       control address */
	RegWriteA_LC898122AF(WG_ADJGANGO, 0x00); /* 0x0108       manual off */

	/*
	 * OFF, NON ON -- ed e' esattamente lo scambio che ALPS ha in
	 * commento. La fabbrica scrive 0xA0 in 0x0129 e 0x012A
	 * ("52801401 mov"@0xffffff8008749b28 e' 0xA0, non 0xA3), e subito
	 * dopo le due RamWrite32A del ramo OFF verso 0x100B e 0x110B, che
	 * sono GANADR e GANADR | 0x0100.
	 *
	 * Le avevo prese per due scritture nuove verso gxadj e gyadj: gli
	 * indirizzi e i valori tornavano, ma la spiegazione era sbagliata.
	 * Erano gia' dentro AutoGainControlSw, e bastava chiamarlo con
	 * l'altro argomento.
	 */
	AutoGainControlSw(OFF);		     /* Auto Gain Control Mode OFF */
	/* AutoGainControlSw( ON ) ;      */ /* Auto Gain Control Mode ON  */
#endif
}

void IniFil(void)
{
	unsigned short UsAryId;
	/*
	 * Le due scelte si fanno in cima, una volta sola, come in S2cPro: la
	 * fabbrica fa i due csel prima del primo ciclo e poi non rilegge piu'.
	 */
	const struct STFILREG *CsFilRegSel =
		(g9c96cbc == 2) ? g8f4c6e8 : CsFilReg;
	const struct STFILRAM *CsFilRamSel =
		(g9c96cbc == 2) ? g8f4c708 : CsFilRam;

	/* Filter Registor Parameter Setting */
	UsAryId = 0;
	while (CsFilRegSel[UsAryId].UsRegAdd != 0xFFFF) {
		RegWriteA_LC898122AF(CsFilRegSel[UsAryId].UsRegAdd,
				     CsFilRegSel[UsAryId].UcRegDat);
		UsAryId++;
	}

	/* 0x018D */
	RegWriteA_LC898122AF(WC_RAMACCXY, 0x01);

	/* Filter Ram Parameter Setting */
	UsAryId = 0;
	while (CsFilRamSel[UsAryId].UsRamAdd != 0xFFFF) {
		RamWrite32A_LC898122AF(CsFilRamSel[UsAryId].UsRamAdd,
					CsFilRamSel[UsAryId].UlRamDat);
		UsAryId++;
	}

	RegWriteA_LC898122AF(WC_RAMACCXY, 0x00);
}

void IniAdj(void)
{
	/*
	 * Due valori per modulo, scelti una volta sola in cima come in
	 * S2cPro: "1a890153 csel"@0xffffff800874a1d8 per la corrente di
	 * polarizzazione e "1a8b0194 csel"@0xffffff800874a1dc per il
	 * guadagno, tutti e due prima della chiamata a IniPtAve.
	 *
	 * X e Y prendono lo stesso registro (w20) in tutti e due i rami:
	 * AMP_GAIN_X e AMP_GAIN_Y restano due macro in ALPS ma qui valgono
	 * sempre uguale, ed e' per questo che la locale e' una sola.
	 */
	unsigned char UcBiasCur = (g9c96cbc == 2) ? 0x44 : BIAS_CUR_OIS;
	unsigned char UcAmpGain = (g9c96cbc == 2) ? 0x03 : AMP_GAIN_X;

	RegWriteA_LC898122AF(WC_RAMACCXY,
			     0x00); /* 0x018D       Filter copy off */

	IniPtAve(); /* Average setting */

	/* OIS */
	RegWriteA_LC898122AF(CMSDAC0,
			     UcBiasCur); /* 0x0251       Hall Dac電流 */
	RegWriteA_LC898122AF(OPGSEL0,
			     UcAmpGain); /* 0x0253       Hall amp Gain X */
	RegWriteA_LC898122AF(OPGSEL1,
			     UcAmpGain); /* 0x0254       Hall amp Gain Y */
	/* AF */
	RegWriteA_LC898122AF(CMSDAC1,
			     BIAS_CUR_AF); /* 0x0252       Hall Dac電流 */
	RegWriteA_LC898122AF(OPGSEL2,
			     AMP_GAIN_AF); /* 0x0255       Hall amp Gain AF */

	RegWriteA_LC898122AF(OSCSET, OSC_INI); /* 0x0257       OSC ini */

	/* adjusted value */
	RegWriteA_LC898122AF(
		IZAH,
		DGYRO_OFST_XH); /* 0x02A0               Set Offset High byte */
	RegWriteA_LC898122AF(
		IZAL,
		DGYRO_OFST_XL); /* 0x02A1               Set Offset Low byte */
	RegWriteA_LC898122AF(
		IZBH,
		DGYRO_OFST_YH); /* 0x02A2               Set Offset High byte */
	RegWriteA_LC898122AF(
		IZBL,
		DGYRO_OFST_YL); /* 0x02A3               Set Offset Low byte */

	/* Ram Access */
	RamAccFixMod(ON); /* 16bit Fix mode */

	/* OIS adjusted parameter */
	RamWriteA_LC898122AF(DAXHLO, DAHLXO_INI); /* 0x1479 */
	RamWriteA_LC898122AF(DAXHLB, DAHLXB_INI); /* 0x147A */
	RamWriteA_LC898122AF(DAYHLO, DAHLYO_INI); /* 0x14F9 */
	RamWriteA_LC898122AF(DAYHLB, DAHLYB_INI); /* 0x14FA */
	RamWriteA_LC898122AF(OFF0Z, HXOFF0Z_INI); /* 0x1450 */
	RamWriteA_LC898122AF(OFF1Z, HYOFF1Z_INI); /* 0x14D0 */
	RamWriteA_LC898122AF(sxg, SXGAIN_INI);    /* 0x10D3 */
	RamWriteA_LC898122AF(syg, SYGAIN_INI);    /* 0x11D3 */
	/* UsCntXof = OPTCEN_X ;         */ /* Clear Optical center X value */
	/* UsCntYof = OPTCEN_Y ;         */ /* Clear Optical center Y value */
	/* RamWriteA_LC898122AF( SXOFFZ1,             UsCntXof ) ; // 0x1461 */
	/* RamWriteA_LC898122AF( SYOFFZ1,             UsCntYof ) ; // 0x14E1 */

	/* AF adjusted parameter */
	RamWriteA_LC898122AF(DAZHLO, DAHLZO_INI); /* 0x1529 */
	RamWriteA_LC898122AF(DAZHLB, DAHLZB_INI); /* 0x152A */

	/* Ram Access */
	RamAccFixMod(OFF); /* 32bit Float mode */

	/*
	 * L'unica chiamata a SetDOFSTDAF di tutto il binario che non venga da
	 * SetDOFSTDAF_WT: "97fffb4d bl"@0xffffff800874a304, sotto la guardia
	 * di g9c96cb8 -- l'altra variabile di modulo, non quella che sceglie
	 * le costanti qui sopra.
	 */
	if (g9c96cb8 == 2)
		SetDOFSTDAF(0x10);

	RamWrite32A_LC898122AF(
		gxzoom,
		GXGAIN_INI); /* 0x1020 Gyro X axis Gain adjusted value */
	RamWrite32A_LC898122AF(
		gyzoom,
		GYGAIN_INI); /* 0x1120 Gyro Y axis Gain adjusted value */

	RamWrite32A_LC898122AF(sxq, SXQ_INI);
	RamWrite32A_LC898122AF(syq, SYQ_INI);

	if (GXHY_GYHX) { /* GX -> HY , GY -> HX */
		RamWrite32A_LC898122AF(sxgx, 0x00000000); /* 0x10B8 */
		RamWrite32A_LC898122AF(sxgy, 0x3F800000); /* 0x10B9 */

		RamWrite32A_LC898122AF(sygy, 0x00000000); /* 0x11B8 */
		RamWrite32A_LC898122AF(sygx, 0x3F800000); /* 0x11B9 */
	}

	SetZsp(0); /* Zoom coefficient Initial Setting */

	RegWriteA_LC898122AF(PWMA, 0xC0); /* 0x0010               PWM enable */

	RegWriteA_LC898122AF(STBB0, 0xDF);

	RegWriteA_LC898122AF(WC_EQSW, 0x02);     /* 0x01E0 */
	RegWriteA_LC898122AF(WC_MESLOOP1, 0x02); /* 0x0193 */
	RegWriteA_LC898122AF(WC_MESLOOP0, 0x00); /* 0x0192 */
	RegWriteA_LC898122AF(WC_AMJLOOP1, 0x02); /* 0x01A3 */
	RegWriteA_LC898122AF(WC_AMJLOOP0, 0x00); /* 0x01A2 */

	SetPanTiltMode(OFF); /* Pan/Tilt OFF */

	SetGcf(0); /* DI initial value */
#ifdef H1COEF_CHANGER
	SetH1cMod(ACTMODE); /* Lvl Change Active mode */
#endif

	DrvSw(ON); /* 0x0001               Driver Mode setting */

	RegWriteA_LC898122AF(WC_EQON, 0x01); /* 0x0101       Filter ON */
}

void IniCmd(void)
{

	MemClr((unsigned char *)&StAdjPar,
		sizeof(struct stAdjPar)); /* Adjust Parameter Clear */
}

void BsyWit(unsigned short UsTrgAdr, unsigned char UcTrgDat)
{
	unsigned char UcFlgVal;
	unsigned char UcCntPla = 0;

	RegWriteA_LC898122AF(UsTrgAdr,
			     UcTrgDat); /* Trigger Register Setting */

	/*
	 * ATTESA LIMITATA A SESSANTA GIRI, come in StbOnn e StbOnnN.
	 * ALPS gira finche' il chip non risponde; la fabbrica conta e se ne va.
	 * Il contatore si prova in cima e il valore con un break: e' quella
	 * forma, e non la condizione doppia, a far combaciare il codice -- e a
	 * far morire il valore iniziale, che nel binario infatti non si scrive.
	 */
	while (UcCntPla < 60) {

		RegReadA_LC898122AF(UsTrgAdr, &UcFlgVal);
		UcFlgVal &= (UcTrgDat & 0x0F);
		if (!UcFlgVal)
			break;
		UcCntPla++;
	};
}

void MemClr(unsigned char *NcTgtPtr, unsigned short UsClrSiz)
{
	unsigned short UsClrIdx;

	for (UsClrIdx = 0; UsClrIdx < UsClrSiz; UsClrIdx++) {
		*NcTgtPtr = 0;
		NcTgtPtr++;
	}
}

void GyOutSignal(void)
{

	RegWriteA_LC898122AF(GRADR0, GYROX_INI);
	RegWriteA_LC898122AF(GRADR1, GYROY_INI);

	/*Start OIS Reading */
	RegWriteA_LC898122AF(GRSEL, 0x02);
}

void GyOutSignalCont(void)
{

	/*Start OIS Reading */
	RegWriteA_LC898122AF(GRSEL, 0x04);
}

#ifdef STANDBY_MODE

void AccWit(unsigned char UcTrgDat)
{
	unsigned char UcFlgVal;
	unsigned char UcCntPla = 0;

	/*
	 * ATTESA LIMITATA A SESSANTA GIRI, come in StbOnn e StbOnnN.
	 * ALPS gira finche' il chip non risponde; la fabbrica conta e se ne va.
	 * Il contatore si prova in cima e il valore con un break: e' quella
	 * forma, e non la condizione doppia, a far combaciare il codice -- e a
	 * far morire il valore iniziale, che nel binario infatti non si scrive.
	 */
	while (UcCntPla < 60) {
		RegReadA_LC898122AF(GRACC, &UcFlgVal); /* 0x0282 */
		UcFlgVal &= UcTrgDat;
		if (!UcFlgVal)
			break;
		UcCntPla++;
	};
}

void SelectGySleep(unsigned char UcSelMode)
{
#ifdef USE_INVENSENSE
	unsigned char UcRamIni;
	unsigned char UcGrini;

	if (UcSelMode == ON) {
		RegWriteA_LC898122AF(WC_EQON,
				     0x00); /* 0x0101       Equalizer OFF */
		RegWriteA_LC898122AF(GRSEL,
				     0x01); /* 0x0280       Set Command Mode */

		RegReadA_LC898122AF(GRINI, &UcGrini);

		RegWriteA_LC898122AF(GRINI, (UcGrini | SLOWMODE));

		RegWriteA_LC898122AF(
			GRADR0, 0x6B); /* 0x0283       Set Write Command */
		RegWriteA_LC898122AF(GRACC, 0x01);
		AccWit(0x01); /* Digital Gyro busy wait */
		RegReadA_LC898122AF(GRDAT0H, &UcRamIni); /* 0x0290 */

		UcRamIni |= 0x40; /* Set Sleep bit */
#ifdef GYROSTBY
		UcRamIni &= ~0x01; /* Clear PLL bit(internal oscillator */
#endif

		RegWriteA_LC898122AF(
			GRADR0, 0x6B); /* 0x0283       Set Write Command */
		RegWriteA_LC898122AF(
			GSETDT,
			UcRamIni); /* 0x028A       Set Write Data(Sleep ON) */
		RegWriteA_LC898122AF(GRACC,
				     0x10); /* 0x0282       Set Trigger ON */
		AccWit(0x10); /* Digital Gyro busy wait */

#ifdef GYROSTBY
		RegWriteA_LC898122AF(
			GRADR0, 0x6C); /* 0x0283       Set Write Command */
		RegWriteA_LC898122AF(
			GSETDT,
			0x07); /* 0x028A       Set Write Data(STBY ON) */
		RegWriteA_LC898122AF(GRACC,
				     0x10); /* 0x0282       Set Trigger ON */
		AccWit(0x10); /* Digital Gyro busy wait */
#endif
	} else {
#ifdef GYROSTBY
		RegWriteA_LC898122AF(
			GRADR0, 0x6C); /* 0x0283       Set Write Command */
		RegWriteA_LC898122AF(
			GSETDT,
			0x00); /* 0x028A       Set Write Data(STBY OFF) */
		RegWriteA_LC898122AF(GRACC,
				     0x10); /* 0x0282       Set Trigger ON */
		AccWit(0x10); /* Digital Gyro busy wait */
#endif
		RegWriteA_LC898122AF(GRADR0, 0x6B);
		RegWriteA_LC898122AF(GRACC, 0x01);
		AccWit(0x01); /* Digital Gyro busy wait */
		RegReadA_LC898122AF(GRDAT0H, &UcRamIni); /* 0x0290 */

		UcRamIni &= ~0x40; /* Clear Sleep bit */
#ifdef GYROSTBY
		UcRamIni |= 0x01; /* Set PLL bit */
#endif

		RegWriteA_LC898122AF(
			GSETDT,
			UcRamIni); /* 0x028A       Set Write Data(Sleep OFF) */
		RegWriteA_LC898122AF(GRACC,
				     0x10); /* 0x0282       Set Trigger ON */
		AccWit(0x10); /* Digital Gyro busy wait */

		RegReadA_LC898122AF(GRINI, &UcGrini);

		RegWriteA_LC898122AF(GRINI, (UcGrini & ~SLOWMODE));

		GyOutSignal(); /* Select Gyro output signal */

		/*
		 * LA FABBRICA NON HA QUESTA ATTESA. Nessuna delle due: in
		 * tutto SelectGySleep non c'e' un solo `bl` verso WitTim.
		 * Il nostro build ne mostrava una sola in piu' perche' clang
		 * fonde le code dei due rami, ma le righe da togliere sono
		 * due.
		 *
		 * Coerente con WitTim svuotata: chi ha tolto il corpo ha
		 * tolto anche qualche chiamata.
		 */

		RegWriteA_LC898122AF(
			WC_EQON, 0x01); /* 0x0101       GYRO Equalizer ON */

		ClrGyr(0x007F, CLR_FRAM1); /* Gyro Delay RAM Clear */
	}
#else /* Panasonic */

	/* unsigned char   UcRamIni ; */

	if (UcSelMode == ON) {
		RegWriteA_LC898122AF(
			WC_EQON, 0x00); /* 0x0101       GYRO Equalizer OFF */
		RegWriteA_LC898122AF(GRSEL,
				     0x01); /* 0x0280       Set Command Mode */
		RegWriteA_LC898122AF(
			GRADR0, 0x4C); /* 0x0283       Set Write Command */
		RegWriteA_LC898122AF(
			GSETDT,
			0x02); /* 0x028A       Set Write Data(Sleep ON) */
		RegWriteA_LC898122AF(GRACC,
				     0x10); /* 0x0282       Set Trigger ON */
		AccWit(0x10); /* Digital Gyro busy wait */
	} else {
		RegWriteA_LC898122AF(
			GRADR0, 0x4C); /* 0x0283       Set Write Command */
		RegWriteA_LC898122AF(
			GSETDT,
			0x00); /* 0x028A       Set Write Data(Sleep OFF) */
		RegWriteA_LC898122AF(GRACC,
				     0x10); /* 0x0282       Set Trigger ON */
		AccWit(0x10); /* Digital Gyro busy wait */
		GyOutSignal(); /* Select Gyro output signal */

		/*
		 * LA FABBRICA NON HA QUESTA ATTESA. Nessuna delle due: in
		 * tutto SelectGySleep non c'e' un solo `bl` verso WitTim.
		 * Il nostro build ne mostrava una sola in piu' perche' clang
		 * fonde le code dei due rami, ma le righe da togliere sono
		 * due.
		 *
		 * Coerente con WitTim svuotata: chi ha tolto il corpo ha
		 * tolto anche qualche chiamata.
		 */

		RegWriteA_LC898122AF(
			WC_EQON, 0x01);    /* 0x0101       GYRO Equalizer ON */
		ClrGyr(0x007F, CLR_FRAM1); /* Gyro Delay RAM Clear */
	}
#endif
}
#endif

#ifdef GAIN_CONT

void AutoGainControlSw(unsigned char UcModeSw)
{

	if (UcModeSw == OFF) {
		RegWriteA_LC898122AF(WG_ADJGANGXATO,
				     0xA0); /* 0x0129       X exe off */
		RegWriteA_LC898122AF(WG_ADJGANGYATO,
				     0xA0); /* 0x012A       Y exe off */
		RamWrite32A_LC898122AF(GANADR, XMAXGAIN); /* Gain Through */
		RamWrite32A_LC898122AF(GANADR | 0x0100,
					YMAXGAIN); /* Gain Through */
	} else {
		RegWriteA_LC898122AF(WG_ADJGANGXATO,
				     0xA3); /* 0x0129       X exe on */
		RegWriteA_LC898122AF(WG_ADJGANGYATO,
				     0xA3); /* 0x012A       Y exe on */
	}
}
#endif

void ClrGyr(unsigned short UsClrFil, unsigned char UcClrMod)
{
	unsigned char UcRamClr;
	unsigned char UcCntPla = 0;

	/*Select Filter to clear */
	RegWriteA_LC898122AF(WC_RAMDLYMOD1, (unsigned char)(UsClrFil >> 8));
	RegWriteA_LC898122AF(
		WC_RAMDLYMOD0, (unsigned char)UsClrFil);

	/*Enable Clear */
	RegWriteA_LC898122AF(WC_RAMINITON,
			     UcClrMod);

	/*Check RAM Clear complete */
	/*
	 * ATTESA LIMITATA A SESSANTA GIRI, come in StbOnn e StbOnnN.
	 * ALPS gira finche' il chip non risponde; la fabbrica conta e se ne va.
	 * Il contatore si prova in cima e il valore con un break: e' quella
	 * forma, e non la condizione doppia, a far combaciare il codice -- e a
	 * far morire il valore iniziale, che nel binario infatti non si scrive.
	 */
	while (UcCntPla < 60) {
		RegReadA_LC898122AF(WC_RAMINITON, &UcRamClr);
		UcRamClr &= UcClrMod;
		if (UcRamClr == 0x00)
			break;
		UcCntPla++;
	}
}

void DrvSw(unsigned char UcDrvSw)
{
	if (UcDrvSw == ON) {
		if (UcPwmMod == PWMMOD_CVL) {
			RegWriteA_LC898122AF(
				DRVFC,
				0xF0);
		} else {
#ifdef PWM_BREAK
			RegWriteA_LC898122AF(
				DRVFC,
				0x00);
#else
			RegWriteA_LC898122AF(
				DRVFC,
				0xC0);
#endif
		}
	} else {
		if (UcPwmMod == PWMMOD_CVL) {
			RegWriteA_LC898122AF(
				DRVFC,
				0x30); /* 0x0001       Drvier Block Ena=0 */
		} else {
#ifdef PWM_BREAK
			RegWriteA_LC898122AF(
				DRVFC,
				0x00); /* 0x0001 Drv.MODE=0,Drv.BLK=0,MODE0B */
#else
			RegWriteA_LC898122AF(
				DRVFC,
				0x00); /* 0x0001       Drvier Block Ena=0 */
#endif
		}
	}
}

void AfDrvSw(unsigned char UcDrvSw)
{
	if (UcDrvSw == ON) {
		/*
		 * Il valore di DRVFCAF dipende dal modulo, come le costanti
		 * di SetH1cMod -- ma qui la variabile e' g9c96cb8
		 * ("3972e108 ldrb"@0xffffff800874ad44, spiazzamento 3256 =
		 * 0xCB8), non g9c96cbc. Sono due variabili distinte e
		 * SelectModule le scrive tutte e due.
		 *
		 * DUE CHIAMATE, NON UN TERNARIO: col ternario clang emette
		 * un csel e la funzione resta quattro byte corta. La
		 * fabbrica dirama e poi fonde la coda, che e' quel che
		 * produce un if/else con due chiamate distinte.
		 *
		 * 0x20 e' il valore di ALPS con AF_PWMMODE non definita, ed
		 * e' il ramo diverso da 2.
		 */
		if (g9c96cb8 == 2)
			RegWriteA_LC898122AF(DRVFCAF, 0x10);
		else
			RegWriteA_LC898122AF(DRVFCAF, 0x20);
		RegWriteA_LC898122AF(CCAAF, 0x80);
	} else {
		RegWriteA_LC898122AF(CCAAF, 0x00);
	}
}

void RamAccFixMod(unsigned char UcAccMod)
{
	switch (UcAccMod) {
	case OFF:
		RegWriteA_LC898122AF(WC_RAMACCMOD, 0x00);
		break;
	case ON:
		RegWriteA_LC898122AF(WC_RAMACCMOD, 0x31);
		break;
	}
}

void IniAf(void)
{
	unsigned char UcStbb0;
	/*
	 * QUI CONVIVONO LE DUE VARIABILI DI MODULO, e non e' un refuso.
	 * Questi quattro valori si scelgono su g9c96cbc
	 * ("3972f128 ldrb"@0xffffff800874a450, spiazzamento 3260 = 0xCBC),
	 * mentre i blocchi if/else piu' sotto si scelgono su g9c96cb8
	 * ("3972e2e8 ldrb"@0xffffff800874a48c, spiazzamento 3256 = 0xCB8).
	 * SelectModule le scrive tutte e due, e IniAf le legge tutte e due.
	 *
	 * Nessuno dei quattro valori coincide con quelli di ALPS in nessuno
	 * dei tre rami ACTREG_*: sono stati ritarati e si leggono solo qui.
	 */
	unsigned short UsRwexd2 = (g9c96cbc == 2) ? 0x113E : 0x4A02;
	unsigned short UsRwexd3 = (g9c96cbc == 2) ? 0x7211 : 0x7D62;
	unsigned char UcFstctime = (g9c96cbc == 2) ? 0xA9 : 0xF9;
	unsigned char UcFstmode = (g9c96cbc == 2) ? 0x00 : 0x02;

	AfDrvSw(OFF); /* AF Drvier Block Ena=0 */
	/*
	 * Il blocco intero e' duplicato, non e' un valore che cambia: sul
	 * modulo 2 c'e' una scrittura in piu' verso 0x0380 -- che in
	 * OisDef.h e' un #define commentato, quindi resta un numero -- e ne
	 * manca una verso PWMAAF.
	 */
	if (g9c96cb8 == 2) {
		RamWriteA_LC898122AF(0x0380, 0x8000);
		RegWriteA_LC898122AF(DRVFCAF, 0x10);
		RegWriteA_LC898122AF(DRVFC3AF, 0x40);
		RegWriteA_LC898122AF(DRVFC4AF, 0x80); /* 0x0084 DOFSTDAF */
		RegWriteA_LC898122AF(AFFC, 0x90);     /* 0x0088 */
	} else {
		RegWriteA_LC898122AF(DRVFCAF, 0x20);
		RegWriteA_LC898122AF(DRVFC3AF, 0x00);
		RegWriteA_LC898122AF(DRVFC4AF, 0x80); /* 0x0084 DOFSTDAF */
		RegWriteA_LC898122AF(PWMAAF, 0x00);   /* 0x0090 AF standby */
		RegWriteA_LC898122AF(AFFC, 0x80);     /* 0x0088 */
	}
#ifdef AF_PWMMODE
	RegWriteA_LC898122AF(DRVFC2AF, 0x82); /* 0x0082       AF slope3 */
	RegWriteA_LC898122AF(DRVCH3SEL,
			     0x02); /* 0x0085       AF only IN1 control */
	RegWriteA_LC898122AF(PWMFCAF,
			     0x89); /* 0x0091       AF GND , Carrier , MODE1 */
	RegWriteA_LC898122AF(PWMPERIODAF,
			     0xA0); /* 0x0099       AF none-synchronism */
#else
	RegWriteA_LC898122AF(DRVFC2AF, 0x00); /* 0x0082       AF slope0 */
	RegWriteA_LC898122AF(DRVCH3SEL,
			     0x00); /* 0x0085       AF H bridge control */
	RegWriteA_LC898122AF(
		PWMFCAF, 0x01); /* 0x0091       AF VREF , Carrier , MODE1 */
	RegWriteA_LC898122AF(PWMPERIODAF,
			     0x20); /* 0x0099       AF none-synchronism */
#endif
	if (g9c96cb8 == 2)
		RegWriteA_LC898122AF(CCFCAF, 0x08); /* 0x00A1 */
	else
		RegWriteA_LC898122AF(CCFCAF, 0x40); /* 0x00A1 GND/- */

	RegReadA_LC898122AF(STBB0, &UcStbb0);

	UcStbb0 &= 0x7F;
	RegWriteA_LC898122AF(STBB0, UcStbb0); /* 0x0250       OIS standby */
	RegWriteA_LC898122AF(STBB1, 0x00);    /* 0x0264       All standby */

	/* AF Initial setting */
	RegWriteA_LC898122AF(FSTMODE, UcFstmode); /* 0x0302 */
	RamWriteA_LC898122AF(
		RWEXD1_L,
		RWEXD1_L_AF); /* 0x0396 - 0x0397 (Register continuos write) */
	RamWriteA_LC898122AF(RWEXD2_L, UsRwexd2);   /* 0x0398 - 0x0399 */
	RamWriteA_LC898122AF(RWEXD3_L, UsRwexd3);   /* 0x039A - 0x039B */
	RegWriteA_LC898122AF(FSTCTIME, UcFstctime); /* 0x0303 */
	/*
	 * RegWriteA, non RamWriteA, e 0x04 al posto di 0x0000: la fabbrica
	 * chiama l'altra funzione con un altro valore
	 * ("321e03e1 orr"@0xffffff800874a5cc).
	 */
	RegWriteA_LC898122AF(TCODEH, 0x04); /* 0x0304 */

#ifdef AF_PWMMODE
	RegWriteA_LC898122AF(PWMAAF, 0x80); /* 0x0090       AF PWM enable */
#endif

	UcStbb0 |= 0x80;
	RegWriteA_LC898122AF(STBB0, UcStbb0); /* 0x0250 */
	RegWriteA_LC898122AF(STBB1,
			     0x05);
	AfDrvSw(ON); /* AF Drvier Block Ena=1 */
}

void IniPtAve(void)
{
	RegWriteA_LC898122AF(WG_PANSTT1DWNSMP0, 0x00); /* 0x0134 */
	RegWriteA_LC898122AF(WG_PANSTT1DWNSMP1, 0x00); /* 0x0135 */
	RegWriteA_LC898122AF(WG_PANSTT2DWNSMP0, 0x90); /* 0x0136 400 */
	RegWriteA_LC898122AF(WG_PANSTT2DWNSMP1, 0x01); /* 0x0137 */
	RegWriteA_LC898122AF(WG_PANSTT3DWNSMP0, 0x64); /* 0x0138 100 */
	RegWriteA_LC898122AF(WG_PANSTT3DWNSMP1, 0x00); /* 0x0139 */
	RegWriteA_LC898122AF(WG_PANSTT4DWNSMP0, 0x00); /* 0x013A */
	RegWriteA_LC898122AF(WG_PANSTT4DWNSMP1, 0x00); /* 0x013B */

	RamWrite32A_LC898122AF(st1mean, 0x3f800000); /* 0x1235 */
	RamWrite32A_LC898122AF(st2mean, 0x3B23D700); /* 0x1236       1/400 */
	RamWrite32A_LC898122AF(st3mean, 0x3C23D700); /* 0x1237       1/100 */
	RamWrite32A_LC898122AF(st4mean, 0x3f800000); /* 0x1238 */
}

void IniPtMovMod(unsigned char UcPtMod)
{
	switch (UcPtMod) {
	case OFF:
		RegWriteA_LC898122AF(WG_PANSTTSETGYRO, 0x00); /* 0x0154 */
		RegWriteA_LC898122AF(WG_PANSTTSETGAIN, 0x54); /* 0x0155 */
		RegWriteA_LC898122AF(WG_PANSTTSETISTP, 0x14); /* 0x0156 */
		RegWriteA_LC898122AF(WG_PANSTTSETIFTR, 0x94); /* 0x0157 */
		RegWriteA_LC898122AF(WG_PANSTTSETLFTR, 0x00); /* 0x0158 */

		break;
	case ON:
		RegWriteA_LC898122AF(WG_PANSTTSETGYRO, 0x00); /* 0x0154 */
		RegWriteA_LC898122AF(WG_PANSTTSETGAIN, 0x00); /* 0x0155 */
		RegWriteA_LC898122AF(WG_PANSTTSETISTP, 0x14); /* 0x0156 */
		RegWriteA_LC898122AF(WG_PANSTTSETIFTR, 0x94); /* 0x0157 */
		RegWriteA_LC898122AF(WG_PANSTTSETLFTR, 0x00); /* 0x0158 */
		break;
	}
}

void ChkCvr(void)
{
	RegReadA_LC898122AF(CVER, &UcCvrCod); /* 0x027E */
	/*
	 * LA SCRITTURA DI MDLREG NON C'E'. Nella ChkCvr di fabbrica ci sono
	 * due chiamate in tutto, la lettura di 0x027E e la scrittura di
	 * 0x02D0: verso 0x00FF non va niente.
	 */
	RegWriteA_LC898122AF(VRREG, FW_VER); /* 0x02D0       Version */
}

/*
 * SetTregAf e RemOff STANNO QUI E NON IN OisCmd.c, e la ragione e' misurabile.
 *
 * In stock.map il blocco delle OIS si spezza in due a IniSet
 * (0xffffff80087490f0), che e' la prima funzione di questo file. Tutto quel
 * che sta prima -- TneRun, AfMidOffAdj, GetDOFSTDAF, SetDOFSTDAF,
 * SelectModule -- e' OisCmd.c; tutto quel che sta dopo e' OisIni.c. E
 * SetTregAf (0xffffff800874ad84) e RemOff (0xffffff800874adb8) stanno DOPO.
 *
 * Non e' una questione di ordine estetico. Tenendole in OisCmd.c, clang le
 * vedeva dalla stessa unita' di traduzione e le INCORPORAVA dentro
 * AfMidOffAdj e dentro se stesse: AfMidOffAdj misurava +60 e RemOff +4, e in
 * tutti e due i casi la differenza era una `bl` di fabbrica diventata codice
 * srotolato da noi. Spostate qui, il compilatore non puo' piu' vederle e
 * chiama, come fa la fabbrica.
 *
 * E' il quarto caso in questo driver in cui la differenza non era il codice
 * ma CHI PUO' VEDERE CHE COSA.
 */
/*
 * SetTregAf legge il globale che SelectModule scrive
 * ("3972e108 ldrb"@0xffffff800874ad90 e' [0xffffff8009c96cb8]) e sceglie
 * quanto spostare: cinque bit se vale 2, sei altrimenti
 * ("531b6801 lsl"@0xffffff800874ad9c contro "531a6401 lsl"@0xffffff800874ada4).
 *
 * E' anche la prova che g9c96cb8 SI LEGGE: finora nessuna funzione lo
 * rileggeva, ed era il motivo per cui non poteva essere `static`.
 */
void SetTregAf(unsigned short UcTregAf)
{
	if (g9c96cb8 == 2)
		RamWriteA_LC898122AF(0x0380, UcTregAf << 5);
	else
		RamWriteA_LC898122AF(0x0380, UcTregAf << 6);
}

/*
 * Due rami e un'attesa. Il parametro vale 1 o 0; per qualunque altro valore
 * la funzione non fa niente ("350008a8 cbnz"@0xffffff800874ade0 salta
 * all'uscita).
 *
 * L'ATTESA SCRIVE INDIETRO. "390013e8 strb"@0xffffff800874ae8c rimette in
 * pila il valore mascherato, e questo dice che il sorgente assegna
 * (`UcRegDat &= 0x02;`) invece di mascherare dentro la condizione: un
 * `while (UcRegDat & 0x02)` non avrebbe nessuna scrittura.
 *
 * Il conteggio si ferma sopra 0x3b ("7100ed3f cmp"@0xffffff800874ae84,
 * `b.hi`), e il confronto e' su otto bit
 * ("12001e69 and"@0xffffff800874ae80): il contatore e' un `unsigned char`.
 */
void RemOff(unsigned char UcMod)
{
	unsigned char UcRegDat;
	unsigned char UcCnt;

	switch (UcMod) {
	case 1:
		RegWriteA_LC898122AF(0x018F, 0x00);
		RegWriteA_LC898122AF(0x018E, 0x7F);
		RegWriteA_LC898122AF(0x0102, 0x02);

		UcCnt = 0;
		do {
			RegReadA_LC898122AF(0x0102, &UcRegDat);
			UcCnt++;
			UcRegDat &= 0x02;
		} while (UcRegDat && UcCnt <= 0x3B);

		RegWriteA_LC898122AF(0x018D, 0x01);
		RamWrite32A_LC898122AF(0x1043, 0x3AAF73C0);
		RamWrite32A_LC898122AF(0x1044, 0x3AAF73C0);
		RamWrite32A_LC898122AF(0x1045, 0x3F7F5080);
		RegWriteA_LC898122AF(0x018D, 0x00);

		SetPanTiltMode(1);

		RegWriteA_LC898122AF(0x010A, 0x44);
		break;

	case 0:
		RegWriteA_LC898122AF(0x010A, 0x11);
		RegWriteA_LC898122AF(0x018D, 0x01);
		RamWrite32A_LC898122AF(0x1043, 0x39D2BD40);
		RamWrite32A_LC898122AF(0x1044, 0x39D2BD40);
		RamWrite32A_LC898122AF(0x1045, 0x3F7FCB40);
		RegWriteA_LC898122AF(0x018D, 0x00);
		RegWriteA_LC898122AF(0x010A, 0x00);
		break;
	}
}
