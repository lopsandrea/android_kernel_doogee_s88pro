/* SPDX-License-Identifier: GPL-2.0 */
/*
 * MiraMEMS mir3da (DA218-B) — interfaccia del core, ricostruita dal
 * disassemblato del kernel di fabbrica del Doogee S88 Pro.
 *
 * ATTENZIONE: questo header dichiara SOLO cio' che `mir3da_cust.c` usa
 * davvero, e ogni firma qui sotto e' dedotta dai punti di chiamata
 * (registri usati, larghezza dei load/store sui puntatori passati), non da
 * un sorgente. Il blocco del core, da `squareRoot` a `NSA_get_reg_data`,
 * comprende 29 funzioni stock: 19 sono ora definite in `mir3da_core.c`,
 * mentre 10 globali senza chiamanti diretti restano omesse. Il nome
 * "mir3da_core.c" e' la convenzione del driver MiraMEMS a monte, NON un
 * nome provato dal binario: la stringa "mir3da_core" non esiste in
 * stock.elf. Censimento e limiti della prova sono documentati in
 * docs/bringup/rapporti/rapporto-mir3da-core.md.
 *
 * Il nome del file di fabbrica e' provato, non dedotto: l'ASSERT di
 * `mir3da_reg_data_store` porta il proprio __FILE__ come dato,
 *   "/home/jenkins/jks-WTK6739_Q0_MP1_AP/alps/kernel-4.14/drivers/misc/
 *    mediatek/sensors-1.0/accelerometer/da218-B/mir3da_cust.c"
 *      @0xffffff80091bfea6
 * e la MODULE_VERSION built-in porta il proprio KBUILD_MODNAME,
 *   "mir3da_cust"@0xffffff80091c0452  (slot .module_name della
 *   struct module_version_attribute a 0xffffff800992eb18).
 */
#ifndef __MIR3DA_CORE_H__
#define __MIR3DA_CORE_H__

#include <linux/types.h>

/*
 * Il livello di log e' UNA sola variabile condivisa dalle due unita' di
 * traduzione, non una per file: sta a 0xffffff800992ed98 e in tutto il
 * kernel di fabbrica la toccano 34 funzioni, di cui `mir3da_log_level_show`
 * e `mir3da_log_level_store` (mir3da_cust.c) e `mir3da_temp_calibrate`,
 * `mir3da_core_init`, `mir3da_module_detect`, `mir3da_read_raw_data`,
 * `NSA_interrupt_ops` (il core).  Una variabile `static` non potrebbe
 * essere letta da entrambe: fino al 2026-08-16 mir3da_cust.c la dichiarava
 * `static int Log_level`, ed era una divergenza vera — il conto dei
 * lettori e' in docs/bringup/rapporti/rapporto-mir3da-core.md.
 *
 * La DEFINIZIONE sta nel core (mir3da_core.c) per adiacenza: 0x992ed98 e'
 * il primo dei quattro oggetti .data contigui del core
 * (0x992ed98 Log_level, 0x992ed9c indice di chip, 0x992eda0 esito di
 * mir3da_parse_chip_info, 0x992eda8 la tabella dei descrittori).
 * L'adiacenza SUGGERISCE l'unita' di traduzione, non la dimostra.
 *
 * I bit, dal disassemblato:
 *   bit 0  "36000088 tbz w8, #0, ..."   messaggi d'errore
 *   bit 2  "361000a8 tbz w8, #2, ..."   messaggi informativi
 *   bit 3  "361800e8 tbz w8, #3, ..."   traccia di funzione (MI_FUN)
 *   bit 4  "36200108 tbz w8, #4, ..."   dump dei dati
 * Il bit 1 non e' testato da nessuna delle 56 funzioni scritte.
 * Il valore iniziale e' 1: la .data di fabbrica a 0xffffff800992ed98
 * contiene 01 00 00 00.
 */
#define MIR_ERR		(1 << 0)
#define MIR_WARN	(1 << 1)
#define MIR_MSG		(1 << 2)
#define MIR_FUN		(1 << 3)
#define MIR_DATA	(1 << 4)

extern int Log_level;

/*
 * Il core prende un "handle" opaco. Nel driver di fabbrica lo stesso
 * puntatore viene passato ora come `struct i2c_client *` (mir3da_probe
 * chiama mir3da_core_init con mir3da_i2c_client) ora come il valore
 * restituito da mir3da_core_init (mir3da_handle): sono lo stesso oggetto,
 * ma il tipo che li accomuna nel sorgente non e' osservabile. `void *` e'
 * l'unica scelta compatibile con entrambi i siti di chiamata.
 */
typedef void *PLAT_HANDLE;

/*
 * La tabella di operazioni che il .cust installa nel core.  Il layout NON
 * e' inventato: e' letto slot per slot dalla struttura a
 * 0xffffff800992eac0, risolvendo `.rela.dyn`:
 *
 *   +0   -> i2c_smbus_read        (0xffffff8008788658)
 *   +8   -> i2c_smbus_write       (0xffffff8008788694)
 *   +16  -> i2c_smbus_read_block  (0xffffff8008788680)
 *   +24  (non relocato, zero)
 *   +32  (non relocato, zero)
 *   +40  (non relocato, zero)
 *   +48  -> get_address           (0xffffff80087886e0)
 *   +56  (non relocato, zero)
 *   +64  -> printk                (0xffffff80081334d4)
 *   +72  -> sprintf               (0xffffff8008e6a774)
 *   +80  -> msdelay               (0xffffff80087886a8)
 *
 * I quattro slot a zero (+24, +32, +40, +56) esistono nel binario di
 * fabbrica ma il driver non li riempie e il core non li chiama in nessuna
 * delle funzioni del blocco: il loro NOME non e' ricavabile, quindi qui
 * restano campi senza semantica invece di essere inventati.  La
 * dimensione della struct (88 byte) e' quella misurata: subito dopo, a
 * 0xffffff800992eb18, comincia la `struct module_version_attribute` di
 * MODULE_VERSION, che e' un altro oggetto.
 *
 * Gli offset usati dal core sono verificati sul disassemblato:
 *   mir3da_register_read   "f9400108 ldr x8, [x8]"        -> +0
 *   mir3da_register_write  "f9400508 ldr x8, [x8,#8]"     -> +8
 *   mir3da_read_raw_data   "f9400908 ldr x8, [x8,#16]"    -> +16
 *   mir3da_read_data       "f9402108 ldr x8, [x8,#64]"    -> +64
 */
struct general_op_s {
	int (*smi2c_read)(PLAT_HANDLE handle, u8 addr, u8 *data);
	int (*smi2c_write)(PLAT_HANDLE handle, u8 addr, u8 data);
	int (*smi2c_read_block)(PLAT_HANDLE handle, u8 addr, u8 len, u8 *buf);
	void *slot_24;
	void *slot_32;
	void *slot_40;
	int (*get_address)(PLAT_HANDLE handle);
	void *slot_56;
	int (*myprintk)(const char *fmt, ...);
	int (*mysprintf)(char *buf, const char *fmt, ...);
	void (*msdelay)(int msec);
};

/*
 * Le firme che seguono vengono dai siti di chiamata in mir3da_cust.c.
 * Qui sono dichiarazioni; le definizioni stanno nell'altra unita' di
 * traduzione, mir3da_core.c, ricostruita per 19 funzioni stock su 29.
 */
int mir3da_install_general_ops(struct general_op_s *ops);
PLAT_HANDLE mir3da_core_init(PLAT_HANDLE handle);
int mir3da_chip_resume(PLAT_HANDLE handle);
int mir3da_read_data(PLAT_HANDLE handle, short *x, short *y, short *z);
int mir3da_get_enable(PLAT_HANDLE handle, u8 *enable);
int mir3da_set_enable(PLAT_HANDLE handle, bool enable);
int mir3da_get_reg_data(PLAT_HANDLE handle, char *buf);
int mir3da_register_write(PLAT_HANDLE handle, u8 addr, u8 data);
int mir3da_get_primary_offset(PLAT_HANDLE handle, int *x, int *y, int *z);

/*
 * "1.0.0_2017-06-27-16:31:30_"@0xffffff80091bff95, secondo argomento della
 * sprintf di mir3da_version_show ("913e5463 add x3, x3, #0xf95").  Nel
 * driver di fabbrica questa stringa e' cablata nel core: qui sta
 * nell'header perche' e' l'unico punto in cui mir3da_cust.c la usa.
 */
#define MIR3DA_CORE_VERSION	"1.0.0_2017-06-27-16:31:30_"

#endif /* __MIR3DA_CORE_H__ */
