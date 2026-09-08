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


/* Define According To Usage */



/**************** Select Gyro Sensor **************/
/* #define       USE_3WIRE_DGYRO    //for D-Gyro SPI interface */

#define USE_INVENSENSE /* INVENSENSE */
#ifdef USE_INVENSENSE

#define FS_SEL 3 /* Å}32.8LSB/?/s  */

#endif

/**************** Model name *****************/
#define MN_3BSD05P1

/**************** FW version *****************/
#ifdef MN_3BSD05P1
#define MDL_VER 0x06
/*
 * FW_VER e' 0x1E, e lo dice RdFwVr, che non fa altro che restituirlo:
 * "321f0fe0 orr"@0xffffff8008748ea0 e' l'intera funzione, piu' il ret.
 *
 * MDL_VER resta a 0x06 ma NON E' PIU' USATO DA NESSUNO: la fabbrica ha
 * tolto sia la scrittura di MDLREG in ChkCvr sia la composizione in RdFwVr.
 * Si lascia dov'e' perche' toglierlo non e' misurabile.
 */
#define FW_VER 0x1E
#endif

/**************** Select Mode **************/
#define STANDBY_MODE /* STANDBY Mode */
#define GAIN_CONT    /* Gain Control Mode */

#define PWM_BREAK /* PWM mode select (disable zero cross) */

#ifdef MN_3BSD05P1
/*
 * IL RAMO ATTIVO E' IL 6.5 OHM, e a dirlo e' A1_IEXP1: IniSrv scrive
 * 0x3F180130 ("72a7e301 movk"@0xffffff80087497a0 e i tre gemelli), che e'
 * il valore del blocco ACTREG_6P5OHM. Il 10.2 ohm avrebbe 0x3F0CCCCD, ed e'
 * quello che il nostro build emetteva.
 *
 * A3_IEXP3 vale 0x3EC0017F in tutti e due i blocchi e non distingue niente:
 * la sola costante che li separa e' A1_IEXP1.
 */
#define ACTREG_6P5OHM /* Use 6.5ohm */
#endif

#define DEF_SET /* default value re-setting */


#define NEUTRAL_CENTER /* Upper Position Current 0mA Measurement */
#define H1COEF_CHANGER /* H1 coef lvl chage */
#define MONITOR_OFF    /* default Monitor output */


/* Command Status */
#define EXE_END 0x02    /* Execute End (Adjust OK) */
#define EXE_HXADJ 0x06  /* Adjust NG : X Hall NG (Gain or Offset) */
#define EXE_HYADJ 0x0A  /* Adjust NG : Y Hall NG (Gain or Offset) */
#define EXE_LXADJ 0x12  /* Adjust NG : X Loop NG (Gain) */
#define EXE_LYADJ 0x22  /* Adjust NG : Y Loop NG (Gain) */
#define EXE_GXADJ 0x42  /* Adjust NG : X Gyro NG (offset) */
#define EXE_GYADJ 0x82  /* Adjust NG : Y Gyro NG (offset) */
#define EXE_OCADJ 0x402 /* Adjust NG : OSC Clock NG */
#define EXE_ERR 0x99    /* Execute Error End */

/* Common Define */
#define SUCCESS 0x00 /* Success */
#define FAILURE 0x01 /* Failure */

#ifndef ON
#define ON 0x01  /* ON */
#define OFF 0x00 /* OFF */
#endif
#define SPC 0x02 /* Special Mode */

#define X_DIR 0x00  /* X Direction */
#define Y_DIR 0x01  /* Y Direction */
#define X2_DIR 0x10 /* X Direction */
#define Y2_DIR 0x11 /* Y Direction */

#define NOP_TIME 0.00004166F

#ifdef STANDBY_MODE
/* Standby mode */
#define STB1_ON 0x00     /* Standby1 ON */
#define STB1_OFF 0x01    /* Standby1 OFF */
#define STB2_ON 0x02     /* Standby2 ON */
#define STB2_OFF 0x03    /* Standby2 OFF */
#define STB3_ON 0x04     /* Standby3 ON */
#define STB3_OFF 0x05    /* Standby3 OFF */
#define STB4_ON 0x06     /* Standby4 ON  for Digital Gyro Read */
#define STB4_OFF 0x07    /* Standby4 OFF */
#define STB2_OISON 0x08  /* Standby2 ON (only OIS) */
#define STB2_OISOFF 0x09 /* Standby2 OFF(only OIS) */
#define STB2_AFON 0x0A   /* Standby2 ON (only AF) */
#define STB2_AFOFF 0x0B  /* Standby2 OFF(only AF) */
#endif

/* OIS Adjust Parameter */
#define DAHLXO_INI 0x0000
/*
 * I VALORI CHE LA FABBRICA HA RITARATO. Sei costanti che ALPS lascia com'e'
 * e che qui hanno un altro valore; nessuna cambia la DIMENSIONE del codice,
 * si vedono solo confrontando le istruzioni.
 *
 * DAHLXB_INI e DAHLYB_INI: "321207e1 orr"@0xffffff800874a27c scrive 0xC000
 * dove ALPS ha 0xE000.
 * SXGAIN_INI e SYGAIN_INI: "321303e1 orr"@0xffffff800874a2b8, 0x2000 contro
 * 0x3000.
 * GXGAIN_INI e GYGAIN_INI: 0x3F333333 e 0xBF333333, e NON sono piu' uguali
 * fra loro -- ALPS le ha tutte e due a 0xBF147AE1, la fabbrica cambia anche
 * il segno della X ("72a7e661 movk"@0xffffff800874a310 mette 0x3F33, non
 * 0xBF33).
 */
#define DAHLXB_INI 0xC000
#define DAHLYO_INI 0x0000
#define DAHLYB_INI 0xC000
#define SXGAIN_INI 0x2000
#define SYGAIN_INI 0x2000
#define HXOFF0Z_INI 0x0000
#define HYOFF1Z_INI 0x0000

#ifdef ACTREG_6P5OHM      /* MTM 9.5 Actuator *************************** */
#define BIAS_CUR_OIS 0x33 /* 2.0mA/2.0mA */
#define AMP_GAIN_X 0x05   /* x150 */
#define AMP_GAIN_Y 0x05   /* x150 */

/*
 * 0x2C, non 0x2E: lo scrive IniAdj in OSCSET
 * ("52800581 mov"@0xffffff800874a228), l'unico posto in cui OSC_INI compaia.
 * Come RdFwVr, e' una differenza che la misura in byte non vede.
 */
#define OSC_INI 0x2C

/* AF Open para */
#define RWEXD1_L_AF 0x7FFF /*  */
#define RWEXD2_L_AF 0x1094 /*  */
#define RWEXD3_L_AF 0x72BA /*  */
#define FSTCTIME_AF 0xED   /*  */
#define FSTMODE_AF 0x02    /*  */

/* (0.3750114X^3+0.5937681X)*(0.3750114X^3+0.5937681X) 6.5ohm */
#define A3_IEXP3 0x3EC0017F
#define A1_IEXP1 0x3F180130

#endif
#ifdef ACTREG_10P2OHM     /* MTM 10.2 Actuator *************************** */
#define BIAS_CUR_OIS 0x33 /* 2.0mA/2.0mA */
#define AMP_GAIN_X 0x05   /* x150 */
#define AMP_GAIN_Y 0x05   /* x150 */

#define OSC_INI 0x2E /* VDD=2.8V */

/* AF Open para */
#define RWEXD1_L_AF 0x7FFF /*  */
#define RWEXD2_L_AF 0x75FE /*  */
#define RWEXD3_L_AF 0x7F32 /*  */
#define FSTCTIME_AF 0xF1   /*  */
#define FSTMODE_AF 0x00    /*  */

/* (0.3750114X^3+0.55X)*(0.3750114X^3+0.55X) 10.2ohm */
#define A3_IEXP3 0x3EC0017F
#define A1_IEXP1 0x3F0CCCCD

#endif
#ifdef ACTREG_15OHM       /* TDK 10.5 Actuator *************************** */
#define BIAS_CUR_OIS 0x22 /* 1.0mA/1.0mA */
#define AMP_GAIN_X 0x04   /* x100 */
#define AMP_GAIN_Y 0x04   /* x100 */

/* OSC Init */
#define OSC_INI 0x2E /* VDD=2.8V */

/* AF Open para */
#define RWEXD1_L_AF 0x7FFF /*  */
#define RWEXD2_L_AF 0x5A00 /*  */
#define RWEXD3_L_AF 0x7000 /*  */
#define FSTCTIME_AF 0x5F   /*  */
#define FSTMODE_AF 0x00    /*  */

/* (0.4531388X^3+0.4531388X)*(0.4531388X^3+0.4531388X) 15ohm */
#define A3_IEXP3 0x3EE801CF
#define A1_IEXP1 0x3EE801CF

#endif

/* AF adjust parameter */
#define DAHLZB_INI 0x9000
#define DAHLZO_INI 0x0000
#define BIAS_CUR_AF 0x00 /* 0.25mA */
#define AMP_GAIN_AF 0x00 /* x6 */

/* Digital Gyro offset Initial value */
#define DGYRO_OFST_XH 0x00
#define DGYRO_OFST_XL 0x00
#define DGYRO_OFST_YH 0x00
#define DGYRO_OFST_YL 0x00

#define SXGAIN_LOP 0x3000
#define SYGAIN_LOP 0x3000

#define TCODEH_ADJ 0x0000

/*
 * 0x3DCCCCC0, e i quattro bit bassi contano: la fabbrica tiene questo
 * valore in un registro e ci somma 13 (`add x21, x23, #0xd`) per costruire
 * GYRA34_MID, che invece resta 0x3DCCCCCD. Con GYRLMT1H uguale a
 * GYRA34_MID quel trucco non avrebbe motivo di esistere -- e infatti nel
 * nostro build le due costanti venivano caricate due volte per intero.
 */
#define GYRLMT1H 0x3DCCCCC0

#ifdef CORRECT_1DEG
#define GYRLMT3_S1 0x3F19999A /* 0.60F */
#define GYRLMT3_S2 0x3F19999A /* 0.60F */

#define GYRLMT4_S1 0x40400000 /* 3.0F */
#define GYRLMT4_S2 0x40400000 /* 3.0F */

#define GYRA12_HGH 0x40000000 /* 2.00F */
#define GYRA12_MID 0x3F800000 /* 1.0F */
#define GYRA34_HGH 0x3F000000 /* 0.5F */
#define GYRA34_MID 0x3DCCCCCD /* 0.1F */

#define GYRB12_HGH 0x3E4CCCCD /* 0.20F */
#define GYRB12_MID 0x3CA3D70A /* 0.02F */
#define GYRB34_HGH 0x3CA3D70A /* 0.02F */
#define GYRB34_MID 0x3C23D70A /* 0.001F */

#else
#define GYRLMT3_S1 0x3ECCCCCD /* 0.40F */
#define GYRLMT3_S2 0x3ECCCCCD /* 0.40F */

#define GYRLMT4_S1 0x40000000 /* 2.0F */
#define GYRLMT4_S2 0x40000000 /* 2.0F */

#define GYRA12_HGH 0x3FC00000 /* 1.50F */
#define GYRA12_MID 0x3F800000 /* 1.0F */
#define GYRA34_HGH 0x3F000000 /* 0.5F */
#define GYRA34_MID 0x3DCCCCCD /* 0.1F */

#define GYRB12_HGH 0x3E4CCCCD /* 0.20F */
#define GYRB12_MID 0x3CA3D70A /* 0.02F */
#define GYRB34_HGH 0x3CA3D70A /* 0.02F */
#define GYRB34_MID 0x3C23D70A /* 0.001F */

#endif

/* #define               OPTCEN_X                0x0000 */
/* #define               OPTCEN_Y                0x0000 */

#ifdef USE_INVENSENSE
#ifdef MN_3BSD05P1
#define SXQ_INI 0x3F800000
#define SYQ_INI 0xBF800000

#define GXGAIN_INI 0x3F333333
#define GYGAIN_INI 0xBF333333

#define GYROX_INI 0x45
#define GYROY_INI 0x43

/*
 * ZERO, e le quattro scritture che protegge spariscono: la IniAdj di
 * fabbrica non tocca 0x10B8, 0x10B9, 0x11B8 ne' 0x11B9.
 */
#define GXHY_GYHX 0
#endif
#endif

/* Optical Center & Gyro Gain for Mode */
#define VAL_SET 0x00 /* Setting mode */
#define VAL_FIX 0x01 /* Fix Set value */
#define VAL_SPC 0x02 /* Special mode */

struct STFILREG {
	unsigned short UsRegAdd;
	unsigned char UcRegDat;
}; /* Register Data Table */

struct STFILRAM {
	unsigned short UsRamAdd;
	unsigned long UlRamDat;
}; /* Filter Coefficient Table */

struct STCMDTBL {
	unsigned short Cmd;
	unsigned int UiCmdStf;
	void (*UcCmdPtr)(void);
};

/*** caution [little-endian] ***/

/* Word Data Union */
union UnWrdVal {
	unsigned short UsWrdVal;
	unsigned char UcWrkVal[2];
	struct {
		unsigned char UcLowVal;
		unsigned char UcHigVal;
	} StWrdVal;
};

union UnDwdVal {
	unsigned long UlDwdVal;
	unsigned short UsDwdVal[2];
	struct {
		unsigned short UsLowVal;
		unsigned short UsHigVal;
	} StDwdVal;
	struct {
		unsigned char UcRamVa0;
		unsigned char UcRamVa1;
		unsigned char UcRamVa2;
		unsigned char UcRamVa3;
	} StCdwVal;
};

/* Float Data Union */
union UnFltVal {
	float SfFltVal;
	unsigned long UlLngVal;
	unsigned short UsDwdVal[2];
	struct {
		unsigned short UsLowVal;
		unsigned short UsHigVal;
	} StFltVal;
};

struct stAdjPar {
	struct {
		unsigned char UcAdjPhs; /* Hall Adjust Phase */

		unsigned short
			UsHlxCna; /* Hall Center Value after Hall Adjust */
		unsigned short UsHlxMax; /* Hall Max Value */
		unsigned short UsHlxMxa; /* Hall Max Value after Hall Adjust */
		unsigned short UsHlxMin; /* Hall Min Value */
		unsigned short UsHlxMna; /* Hall Min Value after Hall Adjust */
		unsigned short UsHlxGan; /* Hall Gain Value */
		unsigned short UsHlxOff; /* Hall Offset Value */
		unsigned short UsAdxOff; /* Hall A/D Offset Value */
		unsigned short UsHlxCen; /* Hall Center Value */

		unsigned short
			UsHlyCna; /* Hall Center Value after Hall Adjust */
		unsigned short UsHlyMax; /* Hall Max Value */
		unsigned short UsHlyMxa; /* Hall Max Value after Hall Adjust */
		unsigned short UsHlyMin; /* Hall Min Value */
		unsigned short UsHlyMna; /* Hall Min Value after Hall Adjust */
		unsigned short UsHlyGan; /* Hall Gain Value */
		unsigned short UsHlyOff; /* Hall Offset Value */
		unsigned short UsAdyOff; /* Hall A/D Offset Value */
		unsigned short UsHlyCen; /* Hall Center Value */
	} StHalAdj;

	struct {
		unsigned short UsLxgVal; /* Loop Gain X */
		unsigned short UsLygVal; /* Loop Gain Y */
		unsigned short UsLxgSts; /* Loop Gain X Status */
		unsigned short UsLygSts; /* Loop Gain Y Status */
	} StLopGan;

	struct {
		unsigned short UsGxoVal; /* Gyro A/D Offset X */
		unsigned short UsGyoVal; /* Gyro A/D Offset Y */
		unsigned short UsGxoSts; /* Gyro Offset X Status */
		unsigned short UsGyoSts; /* Gyro Offset Y Status */
	} StGvcOff;

	unsigned char UcOscVal; /* OSC value */
};

extern struct stAdjPar StAdjPar; /* Execute Command Parameter */

extern unsigned char UcOscAdjFlg; /* For Measure trigger */
#define MEASSTR 0x01
#define MEASCNT 0x08
#define MEASFIX 0x80

extern unsigned short UsCntXof; /* OPTICAL Center Xvalue */
extern unsigned short UsCntYof; /* OPTICAL Center Yvalue */

extern unsigned char UcPwmMod; /* PWM MODE */
#define PWMMOD_CVL 0x00	/* CVL PWM MODE */
#define PWMMOD_PWM 0x01	/* PWM MODE */

#define INIT_PWMMODE PWMMOD_CVL /* initial output mode */

extern unsigned char UcCvrCod; /* CverCode */
#define CVER122 0x93	   /* LC898122 */
#define CVER122A 0xA1	  /* LC898122A */

/* Prottype Declation */
extern void IniSet(void);   /* Initial Top Function */
extern void IniSetAf(void); /* Initial Top Function */

extern void ClrGyr(unsigned short D1, unsigned char D2);
#define CLR_FRAM0 0x01
#define CLR_FRAM1 0x02
#define CLR_ALL_RAM 0x03
extern void BsyWit(unsigned short D1, unsigned char D2);

extern void MemClr(unsigned char *D1, unsigned short D2);
extern void GyOutSignal(void);
extern void GyOutSignalCont(void);
#ifdef STANDBY_MODE
extern void AccWit(unsigned char D1);
extern void SelectGySleep(unsigned char D1);
#endif
#ifdef GAIN_CONT
extern void AutoGainControlSw(unsigned char D1);
#endif
extern void DrvSw(unsigned char UcDrvSw);
extern void AfDrvSw(unsigned char UcDrvSw);
extern void RamAccFixMod(unsigned char D1);
extern void IniPtMovMod(unsigned char D1);
extern void ChkCvr(void);   /* Check Function */

extern void SrvCon(unsigned char D1, unsigned char D2);
extern unsigned short TneRun(void);
extern unsigned char RtnCen(unsigned char D1);
extern void OisEna(void);
extern void OisEnaLin(void);
extern void TimPro(void);
extern void S2cPro(unsigned char D1);

#ifdef MN_3BSD05P1
/*
 * DIFIL_S2 DIPENDE DAL MODULO, e la prova e' un csel.
 *
 * "529fa00a mov"@0xffffff8008748464 carica 0x3F7FFD00, la add che segue ne
 * ricava 0x3F7FFE00 -- il valore di ALPS -- e "9a8b0153 csel"@0xffffff8008748480
 * sceglie fra i due sul confronto di g9c96cbc con 2.
 *
 * Il valore di ALPS e' quindi quello del ramo diverso da 2; l'altro modulo
 * usa 0x3F7FFD00. Vale solo per OisCmd.c, l'unico file che nomini DIFIL_S2.
 */
#define DIFIL_S2 ((g9c96cbc == 2) ? 0x3F7FFD00 : 0x3F7FFE00)
#endif
extern void SetSinWavePara(unsigned char D1,
			   unsigned char D2);
#define SINEWAVE 0
#define XHALWAVE 1
#define YHALWAVE 2
#define CIRCWAVE 255
extern unsigned char TneGvc(void); /* Gyro VC Offset Adjust */

extern void SetZsp(unsigned char D1);
extern void OptCen(unsigned char a, unsigned short b, unsigned short c);
extern void StbOnnN(unsigned char D1,
		    unsigned char D2);
#ifdef MODULE_CALIBRATION
extern unsigned char LopGan(unsigned char D1);
#endif
#ifdef STANDBY_MODE
extern void SetStandby(unsigned char D1);
#endif
#ifdef MODULE_CALIBRATION
extern unsigned short OscAdj(void); /* OSC clock adjustment */
#endif

#ifdef HALLADJ_HW
#ifdef MODULE_CALIBRATION
extern unsigned char LoopGainAdj(unsigned char D1);
#endif
extern unsigned char BiasOffsetAdj(unsigned char D1, unsigned char D2);
#endif
extern void GyrGan(unsigned char D1, unsigned long D2,
		   unsigned long D3);
extern void SetPanTiltMode(unsigned char D1);
extern void SelectModule(unsigned char UcSelPrm);
extern void SetDOFSTDAF(unsigned char UcSetDat);
extern void RemOff(unsigned char UcMod);
/*
 * I globali che SelectModule scrive e SetTregAf/SetDOFSTDAF leggono. Stanno
 * in OisCmd.c e servono anche a OisIni.c, dove SetTregAf e RemOff sono
 * dovute andare per non farsi incorporare (vedi il commento la' in fondo).
 * Il nome e' l'indirizzo: il binario non nomina i dati.
 */
extern unsigned char g9c96cb8;
extern unsigned char g9c96cbc;
/*
 * g9c96cc4 ERA UcCvrCod, e tenerne due era un difetto NOSTRO: OisIni.c
 * dichiara `unsigned char UcCvrCod` e ci legge il CverCode, OisCmd.c
 * dichiarava un secondo globale sullo stesso indirizzo di fabbrica. Due
 * oggetti dove la fabbrica ne ha uno: le scritture di ChkCvr non sarebbero
 * mai arrivate a chi le legge in OisCmd.c.
 *
 * La dimensione non poteva vederlo -- un ldrb resta un ldrb -- e nessuna
 * delle trentacinque funzioni di OisCmd.o e' cambiata di un byte.
 */
extern unsigned char UcCvrCod;

extern void SetTregAf(unsigned short UcTregAf);
extern unsigned long MesMSABS1AV(void);
extern unsigned char GetDOFSTDAF(void);
extern void SetDOFSTDAF_WT(unsigned char UcSetDat);
extern unsigned short AfMidOffAdj(void);
#ifndef HALLADJ_HW
extern unsigned long TnePtp(unsigned char D1,
			    unsigned char D2);
#ifdef MN_3BSD05P1
#define HALL_H_VAL 0x3F800000 /* 1.0 */
#endif
extern unsigned char TneCen(unsigned char D1,
			    union UnDwdVal D2);
#define PTP_BEFORE 0
#define PTP_AFTER 1
#endif
#ifdef GAIN_CONT
extern unsigned char TriSts(void);
#endif
extern unsigned char DrvPwmSw(unsigned char D1);
#define Mlnp 0
#define Mpwm 1
#ifdef NEUTRAL_CENTER
extern unsigned char TneHvc(void);
#endif
extern void SetGcf(unsigned char D1);
extern unsigned long UlH1Coefval;
#ifdef H1COEF_CHANGER
extern unsigned char UcH1LvlMod; /* H1 level coef mode */
extern void SetH1cMod(unsigned char D1);
#define S2MODE 0x40
#define ACTMODE 0x80
#define MOVMODE 0xFF
#endif
extern unsigned short RdFwVr(void);
void RegWriteA_LC898122AF(unsigned short RegAddr, unsigned char RegData);
void RegReadA_LC898122AF(unsigned short RegAddr, unsigned char *RegData);
void RamWriteA_LC898122AF(unsigned short RamAddr, unsigned short RamData);
void RamReadA_LC898122AF(unsigned short RamAddr, void *ReadData);
void RamWrite32A_LC898122AF(unsigned short RamAddr, unsigned long RamData);
void RamRead32A_LC898122AF(unsigned short RamAddr, void *ReadData);
void WitTim_LC898122AF(unsigned short UsWitTim);
void LC898prtvalue(unsigned short value);

/* ************************** */
/* Local Function Prottype */
/* ************************** */
extern void IniClk(void);   /* Clock Setting */
extern void IniIop(void);   /* I/O Port Initial Setting */
extern void IniMon(void);   /* Monitor & Other Initial Setting */
extern void IniSrv(void);   /* Servo Register Initial Setting */
extern void IniGyr(void);   /* Gyro Filter Register Initial Setting */
extern void IniFil(void);   /* Gyro Filter Initial Parameter Setting */
extern void IniAdj(void);   /* Adjust Fix Value Setting */
extern void IniCmd(void);   /* Command Execute Process Initial */
extern void IniDgy(void);   /* Digital Gyro Initial Setting */
extern void IniAf(void);    /* Open AF Initial Setting */
extern void IniPtAve(void); /* Average setting */

/* ************************** */
/* Local Function Prottype */
/* ************************** */
extern void MesFil(unsigned char D1);
#ifdef MODULE_CALIBRATION
#ifndef HALLADJ_HW
extern void LopIni(unsigned char D1);
#endif
extern void LopPar(unsigned char D1);
#ifndef HALLADJ_HW
extern void LopSin(unsigned char D1,
		   unsigned char D2);
extern unsigned char LopAdj(unsigned char D1);
extern void LopMes(void);
#endif
#endif
#ifndef HALLADJ_HW
extern unsigned long GinMes(unsigned char D1);
#endif
extern void GyrCon(unsigned char D1);
extern short GenMes(unsigned short D1, unsigned char D2);
#ifndef HALLADJ_HW

extern unsigned long TneOff(union UnDwdVal D1,
			    unsigned char D2);
extern unsigned long TneBia(union UnDwdVal D1,
			    unsigned char D2);
#endif

extern void StbOnn(void); /* Servo ON Slope mode */

extern void SetSineWave(unsigned char D1, unsigned char D2);
extern void StartSineWave(void);
extern void StopSineWave(void);

extern void SetMeasFil(unsigned char D1);
extern void ClrMeasFil(void);

/* Read Fw Version Function */
