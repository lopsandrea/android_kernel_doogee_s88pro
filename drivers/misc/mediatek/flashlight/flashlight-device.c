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
 * flashlight-device.c -- la TABELLA DEI DISPOSITIVI, letta dal binario di fabbrica
 * ============================================================================
 *
 * COSA CONTIENE
 *   Il file ALPS, invariato, PIU' un ramo `mt6771` che riproduce byte per byte
 *   la `flashlight_id[]` del kernel di fabbrica del Doogee S88 Pro.  Nessuna
 *   funzione: questo file non compare in `stock.map`, non compare in `.ddebug`,
 *   e nessuna misura sulle FUNZIONI del blocco flashlight lo puo' vedere.  E'
 *   per questo che la divergenza era rimasta invisibile fino a oggi.
 *
 * IL CONFINE
 *   Di questo file: solo `flashlight_id[]` e `flashlight_device_num`.  Il
 *   CODICE del blocco e' gia' nell'albero ALPS: vedi
 *   `patches/kernel-stock/flashlight-dal-disassemblato.patch` per le tre
 *   divergenze, che stanno tutte in `flashlights-mt6370.c`.
 *
 *   Quante siano le funzioni del blocco dipende da DOVE si mette il confine, e
 *   il «95» che girava nel lotto ne mescolava due.  I tre confini, contati:
 *     91   il blocco della mappa, 0xffffff800875b5f8..0xffffff8008760ce4:
 *          43 da flashlight-core.o, 17 da flashlights-mt6370.o, 31 da
 *          richtek/rt-flashlight.o, e ZERO da richtek/rtfled.o.
 *     94   le 91 piu' le tre `.exit` delle stesse tre unita' di traduzione.
 *          Stanno OLTRE `_einittext` (0xffffff80093a8518), dove `stock.map`
 *          finisce, quindi nessuno strumento che parta dalla mappa le vede:
 *            flashlight_exit          0xffffff80093aa21c   120 byte
 *            flashlight_mt6370_exit   0xffffff80093aa294   120 byte
 *            flashlight_class_exit    0xffffff80093aa30c    28 byte
 *     95   le 94 piu' `rtfled_exit` (0xffffff80093aa328, 28 byte).  Questa
 *          quarta `.exit` e' di `richtek/rtfled.c`, le cui altre 27 funzioni
 *          stanno FUORI dal blocco: cominciano esattamente a
 *          0xffffff8008760ce4, cioe' dove il blocco finisce.  Contare lei
 *          dentro e le sue 27 sorelle fuori e' un confine che cambia
 *          definizione a meta' del conteggio: se si scrive 95 va detto.
 *          Includendo tutto rtfled.c il denominatore sarebbe 122.
 *   Che le `.exit` siano quattro e non di piu' non e' un'ispezione a occhio:
 *   le cinque unita' di traduzione hanno in tutto quattro sezioni `.exit.text`,
 *   di 0x78, 0x78, 0x1c e 0x1c byte (`objdump -h` sui cinque `.o`), e i loro
 *   confini si ricavano dai prologhi, non dalla mappa.
 *
 *   E il numero di IDENTICHE non si scrive senza il compilatore accanto:
 *     clang r353983c (la FABBRICA)   91/91   94/94   95/95   (27/27 su rtfled.c)
 *     clang r383902  (il PROGETTO)   66/91   69/94   70/95   (24/27 su rtfled.c)
 *   Le quattro `.exit` sono identiche con ENTRAMBI i compilatori (0 istruzioni
 *   divergenti su tutte e quattro): la differenza sta tutta nelle 91.
 *   70 su 95 = 73,7%, IC95% Clopper-Pearson [63.6%; 82.2%]: contiene il 77,10%
 *   del ramo, quindi NON discrimina.  91 su 91 = 100%, IC95% [96.0%; 100.0%]:
 *   quello discrimina.
 *
 * COSA E' MISURATO E COSA E' SCELTO
 *   MISURATO  il CONTENUTO della tabella: una voce sola, `{0, 0, 0,
 *             "flashlights-mt6370", 0, 0}`, e `flashlight_device_num == 1`.
 *             I byte sono citati qui sotto uno per uno.
 *   SCELTA    la GUARDIA `#elif defined(mt6771)` -- ed e' una scelta PIU'
 *             LARGA del necessario, che va presa sapendo che ne esisteva una
 *             piu' stretta a costo zero.
 *             Il config di fabbrica ha `CONFIG_WTK_MAIN_FLASHLIGHT_CH0=y`
 *             (`e977_dg_m13_71_q0.config:1790`), che in tutto l'albero ALPS non
 *             esiste; il binario NON registra quale `#if` la fabbrica abbia
 *             usato, perche' il preprocessore non lascia tracce.  I candidati
 *             gia' disponibili sono DUE, non uno: il Makefile della directory
 *             passa `-D$(MTK_PLATFORM) -D$(MTK_PROJECT)`, e sulla riga di
 *             comando ci sono percio' sia `-Dmt6771` sia `-De977_dg_m13_71_q0`:
 *               grep -o -- "-D[a-z0-9_]*" \
 *                 out-flash/.../.flashlight-device.o.cmd | grep -E "mt6771|e977"
 *               -Dmt6771
 *               -De977_dg_m13_71_q0
 *             `defined(mt6771)` e' la forma della maggior parte degli altri
 *             rami di questo file (`mt6739`, `mt6757`, `mt6758`, `mt6759`,
 *             `mt6763`, `mt6799`, `mt8167`: sette rami, non cinque), ma il ramo
 *             `mt6757` usa gia' guardie a livello di PROGETTO
 *             (`defined(evb6757p_dm_64) || ...`): il precedente per la forma
 *             stretta e' gia' in ALPS, in questo stesso file.
 *             CONSEGUENZA della forma larga: la tabella dei dispositivi cambia
 *             per QUALUNQUE scheda mt6771 che compili questo driver, non solo
 *             per l'S88 Pro.  Con `defined(e977_dg_m13_71_q0)` le altre schede
 *             mt6771 ricadrebbero nel `#else`, cioe' esattamente dove ALPS le
 *             lasciava.
 *             MISURATO: la forma stretta produce un oggetto IDENTICO -- stessi
 *             simboli, stessa `.rodata` byte per byte -- perche' su questo
 *             albero i due nomi sono definiti entrambi.  Il cambio e' quindi
 *             gratuito per la misura; ma e' una DECISIONE sul comportamento di
 *             altre schede, e questo lotto non la prende da solo: resta al
 *             lotto di merge, insieme al "delta di header" che aggiunge la
 *             voce `Kconfig` per il nome di fabbrica.
 *
 * COME SI RIVERIFICA
 *   # i byte della tabella di fabbrica (52 = 0x34 byte, poi il contatore)
 *   OD=/mnt/s88pro/lineage21/prebuilts/gcc/linux-x86/aarch64/\
 *      aarch64-linux-android-4.9/bin/aarch64-linux-android-objdump
 *   $OD -s --start-address 0xffffff8008f51170 --stop-address 0xffffff8008f511a8 \
 *       oracolo/stock.elf
 *   ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf 0xffffff8008f5117c
 *   ./venv/bin/python3 relocazioni.py oracolo/stock.elf oracolo/stock.map \
 *       --indirizzo 0xffffff8008f51170        # "non relocato": nessun puntatore
 *
 *   # gli stessi byte nel NOSTRO .o, dopo questo file
 *   $OD -s -j .rodata out-flash/drivers/misc/mediatek/flashlight/flashlight-device.o
 *
 * DA DOVE VENGONO I NUMERI (tutte le citazioni sono di fabbrica)
 *   `flashlight_dev_register` legge il contatore e scandisce la tabella:
 *     "d0003fa8 adrp"@0xffffff800875b918   adrp x8, 0xffffff8008f51000
 *     "b981a519 ldrsw"@0xffffff800875b91c  ldrsw x25,[x8,#420]
 *                                          0xf51000+420 = 0xf511a4 = il contatore,
 *                                          letto con SEGNO -> e' un `int`
 *     "7100073f cmp"@0xffffff800875b924    cmp w25, #1  (il ciclo non parte se <1)
 *     "9105c108 add"@0xffffff800875b930    add x8, x8, #0x170
 *                                          0xf51000+0x170 = 0xf51170 = la tabella
 *     "9100d37b add"@0xffffff800875ba94    add x27, x27, #0x34
 *                                          passo 52 = sizeof(struct flashlight_device_id)
 *     "321b03e2 orr"@0xffffff800875ba04    w2 = 0x20 = FLASHLIGHT_NAME_SIZE
 *     "941bf25d bl"@0xffffff800875ba10     bl strncmp(nome, voce+0x0c, 32)
 *                                          -> il nome sta a +0x0c ed e' un char[32]
 *     "b85f4363 ldur"@0xffffff800875ba18   ldur w3,[x27,#-12] = il campo a +0x00
 *   e il nome, dai byte:
 *     "flashlights-mt6370"@0xffffff8008f5117c
 *
 *   I 52 byte, letti a mano (0xffffff8008f51170).  I valori sono scritti in
 *   decimale di proposito: la forma <otto cifre esadecimali> <parola> e' la
 *   forma di una CITAZIONE DI ISTRUZIONE, e usarla qui per dei byte di dati
 *   la farebbe cercare, giustamente, dentro il disassemblato.
 *     +0x00  campo type      = 0
 *     +0x04  campo ct        = 0
 *     +0x08  campo part      = 0
 *     +0x0c  campo name[32]  = "flashlights-mt6370" e 14 byte a zero
 *     +0x2c  campo channel   = 0
 *     +0x30  campo decouple  = 0
 *     +0x34  flashlight_device_num = 1   (fuori dalla voce: e' il contatore)
 *
 * DIVERGENZE APERTE
 *   1. La guardia del `#elif` e' una scelta, non una misura (vedi sopra), ed e'
 *      lasciata APERTA fra tre forme:
 *        `defined(mt6771)`                       larga, ed e' quella scritta;
 *        `defined(e977_dg_m13_71_q0)`            stretta, oggetto identico;
 *        `defined(CONFIG_WTK_MAIN_FLASHLIGHT_CH0)`  il nome di fabbrica, che
 *                                                pero' vuole prima la voce
 *                                                `Kconfig` (delta di header).
 *      Il nome `CONFIG_WTK_MAIN_FLASHLIGHT_CH0` e' l'unico indizio, e non e'
 *      nel binario: e' nel `.config` di fabbrica.
 *   2. Il ramo `#else` di ALPS (otto voci "flashlights-none" con canale -1) resta
 *      dov'e': non e' stato tolto, perche' e' il ramo di ripiego delle altre
 *      piattaforme e il binario non dice nulla su di esse.
 */

#include "flashlight-core.h"

#if defined(mt6739)
#if defined(CONFIG_MTK_FLASHLIGHT_LED191)
	const struct flashlight_device_id flashlight_id[] = {
		/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
		{0, 0, 0, "flashlights_led191", 0, 0},
		{1, 0, 0, "flashlights_led191", 1, 0},
	};
#else
	const struct flashlight_device_id flashlight_id[] = {
		/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
		{0, 0, 0, "flashlights-rt4505", 0, 0},
	};
#endif
#elif defined(mt6757)
	#if defined(evb6757p_dm_64) || defined(k57pv1_dm_64) || \
	defined(k57pv1_64_baymo) || defined(k57pv1_dm_64_bif) || \
	defined(k57pv1_dm_64_baymo) || defined(k57pv1_dm_teei_2g) || \
	defined(k57pv1_dm_64_zoom)
	const struct flashlight_device_id flashlight_id[] = {
		/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
		{0, 0, 0, "flashlights-rt5081", 0, 0},
		{0, 1, 0, "flashlights-rt5081", 1, 0},
	};
	#elif defined(CONFIG_MTK_FLASHLIGHT_RT5081)
	const struct flashlight_device_id flashlight_id[] = {
		/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
		{0, 0, 0, "flashlights-rt5081", 0, 0},
		{0, 1, 0, "flashlights-rt5081", 1, 0},
	};
	#else
	const struct flashlight_device_id flashlight_id[] = {
		/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
		{0, 0, 0, "flashlights-lm3643", 0, 0},
		{0, 1, 0, "flashlights-lm3643", 1, 0},
	};
	#endif
#elif defined(mt6758)
const struct flashlight_device_id flashlight_id[] = {
	/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
	{0, 0, 0, "flashlights-mt6370", 0, 0},
	{0, 1, 0, "flashlights-mt6370", 1, 0},
};
#elif defined(mt6759)
const struct flashlight_device_id flashlight_id[] = {
	/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
	{0, 0, 0, "flashlights-rt5081", 0, 0},
	{0, 1, 0, "flashlights-rt5081", 1, 0},
};
#elif defined(mt6763)
const struct flashlight_device_id flashlight_id[] = {
	/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
	{0, 0, 0, "flashlights-mt6370", 0, 0},
	{0, 1, 0, "flashlights-mt6370", 1, 0},
};
#elif defined(mt6799)
const struct flashlight_device_id flashlight_id[] = {
	/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
	{0, 0, 0, "flashlights-mt6336", 0, 0},
	{0, 1, 0, "flashlights-mt6336", 1, 0},
};
#elif defined(mt8167)
const struct flashlight_device_id flashlight_id[] = {
	/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
	{0, 0, 0, "flashlights-lm3642", 0, 0},
};
#elif defined(mt6771)
/*
 * Doogee S88 Pro (e977_dg_m13_71_q0): UNA voce sola, sul canale 0.
 * Byte per byte da 0xffffff8008f51170; il contatore a 0xffffff8008f511a4 vale 1.
 * "flashlights-mt6370"@0xffffff8008f5117c
 */
const struct flashlight_device_id flashlight_id[] = {
	/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
	{0, 0, 0, "flashlights-mt6370", 0, 0},
};
#else
const struct flashlight_device_id flashlight_id[] = {
	/* {TYPE, CT, PART, "NAME", CHANNEL, DECOUPLE} */
	{0, 0, 0, "flashlights-none", -1, 0},
	{0, 1, 0, "flashlights-none", -1, 0},
	{1, 0, 0, "flashlights-none", -1, 0},
	{1, 1, 0, "flashlights-none", -1, 0},
	{0, 0, 1, "flashlights-none", -1, 0},
	{0, 1, 1, "flashlights-none", -1, 0},
	{1, 0, 1, "flashlights-none", -1, 0},
	{1, 1, 1, "flashlights-none", -1, 0},
};
#endif

const int flashlight_device_num =
	sizeof(flashlight_id) / sizeof(struct flashlight_device_id);

