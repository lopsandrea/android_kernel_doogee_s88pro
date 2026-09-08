// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_FT8719_E977 -- FocalTech FT8719, Doogee S88 Pro (MT6771).
 * L'UNITA' DI TRADUZIONE DEL CANALE /proc E DEI NODI sysfs (unita' A).
 * L'UNITA' E' COMPLETA: 22 funzioni su 22, 5708 byte su 5708.  Le prime
 * diciotto sono del lotto del 2026-08-21; le ultime QUATTRO -- fts_debug_read,
 * fts_debug_write, fts_tprwreg_show, fts_tprwreg_store, 3508 byte -- le ha
 * aggiunte il lotto A4 il 2026-08-22, e il §5 dice come sono misurate.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA.  Nessun sorgente
 * pubblico e' stato letto; i due driver FocalTech gia' presenti nell'albero
 * ALPS (focaltech_touch/, focaltech_fhd_touch/) NON sono stati aperti.
 *
 * ===========================================================================
 * 1. CHE COSA C'E' QUI DENTRO, E QUAL E' IL CONFINE
 * ===========================================================================
 *   testo   0xffffff8008a7b600 .. 0xffffff8008a7cc4c   22 funzioni, 5708 byte
 *   .data   0xffffff800998c750 .. 0xffffff800998c8c0   368 byte
 *   .bss    0xffffff800a100a98 .. 0xffffff800a100ac8   48 byte
 *
 * 0xffffff8008a7b600 e' anche l'inizio del blocco: subito prima finisce
 * `gt1x_startup_patch`, che appartiene al Goodix (WTK_GT917S_E977).  Il
 * confine e' quindi verificato dal lato basso senza dipendere
 * dall'adiacenza.
 *
 * IL CONFINE DI QUESTA UNITA' VERSO L'UNITA' B e' 0xffffff8008a7cc4c, dove
 * comincia `fts_pram_write_init`.  Attenzione: la tabella §5 di
 * scout-ft8719/RIASSUNTO.md scrive 0xa7cbd0, che cade DENTRO l'ultima
 * funzione dell'unita' -- `fts_irq_store` va da 0xffffff8008a7cbc8 a
 * 0xffffff8008a7cc4c, e 0xa7cbd0 e' il suo terzo byte.  L'errore e' gia'
 * registrato e corretto nel cappello di focaltech_core.c §5; lo ripeto qui
 * perche' una correzione che non raggiunge tutte le copie e' la classe C1.
 *   $ sed -n '22,23p' scout-ft8719/blocco.map
 *   ffffff8008a7cbc8 t fts_irq_store
 *   ffffff8008a7cc4c T fts_pram_write_init
 *
 * IL NOME DEL FILE E' UNA SCELTA: il binario non porta i nomi dei file di
 * fabbrica (`ddebug.py --file focaltech` -> "dopo il filtro: 0").
 *
 * ===========================================================================
 * 2. COME SI RIVERIFICA
 * ===========================================================================
 *   ./venv/bin/python3 verificacitazioni.py focaltech_sysfs.c oracolo/stock.elf \
 *       --eccezione "[FTS]" --eccezione "[FTS][Info]" \
 *       --eccezione "[FTS][Error]" --eccezione "\n"
 *
 *   ./venv/bin/python3 verificaistruzioni.py focaltech_sysfs.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a7b600:0xffffff8008a7cc4c \
 *       --intervallo 0xffffff8008a8020c:0xffffff8008a802ac \
 *       --controfattuale '12800000 mov'
 *
 * IL SECONDO INTERVALLO e' l'unita' D (focaltech_ex_mode.c), e serve per UNA
 * citazione sola: "321d7be0 orr"@0xffffff8008a8025c, il -EIO di
 * fts_ex_mode_init, citato per CONTRASTO col -ENOMEM di fts_create_sysfs.
 * Il confronto fra le due e' il punto di quel commento, e sta per forza a
 * cavallo di due unita'.
 *
 * LA `--controfattuale` e' l'istruzione che NON deve stare nel binario:
 * "12800000 mov" e' `mov w0, #-1` a 32 bit, cioe' la forma che i sette stub
 * avrebbero se il tipo restituito fosse `int`.  Il binario ha invece
 * "92800000 mov"@0xffffff8008a7bbf0, `mov x0, #-1` a 64 bit.  Senza il flag lo
 * strumento la segnala ASSENTE, e HA RAGIONE: e' proprio cio' che deve fare.
 * Il flag e' il modo di dire "so che non c'e', l'ho scartata".
 *
 * Le codifiche non sono trascritte a mano: escono da uno script che legge il
 * disassemblato e stampa la coppia (codifica, indirizzo) insieme (classe B8).
 *
 * ESITO ATTESO, al 2026-08-22 (unita' completa):
 *   citazioni di istruzione trovate nel sorgente: 271 (271 a codifica, 0 ad indirizzo)
 *   confermate: 268   assenti: 0   mnemonico diverso: 0   controfattuali: 3
 *   operandi -- registro diverso: 0   immediato diverso: 0   entrambi/non confrontabili: 0
 * e per le citazioni di letterale:
 *   letterali: 73   citati: 76   verificati: 62   probanti: 58   deboli: 4
 *   di cui verificate come messaggio assemblato dalla macro di log: 25
 *   soglia imposta: 69 citati richiesti (73 letterali - 4 eccezioni)
 * piu' QUATTORDICI `NON_ANCORATA` e QUATTRO `DEBOLE`, di tre classi
 * dichiarate: vedi il blocco in coda al file.  Erano dieci e tre quando
 * l'unita' era a diciotto funzioni; le quattro NON_ANCORATA in piu' sono
 * tutte della classe 2 (FTS_DEBUG non emette livello KERN, e lo strumento
 * riconosce solo i messaggi assemblati che cominciano con \x01 + cifra), la
 * DEBOLE in piu' e' "failed, ret: %d\n"@0xffffff800924de4c, che e' il
 * suffisso di un'altra stringa dello stesso file.
 *
 * ===========================================================================
 * 3. LE DIVERGENZE APERTE
 * ===========================================================================
 * Elencate e attribuite nel blocco in CODA a questo file.
 *
 * ===========================================================================
 * 4. IL DELTA DI FILE CONDIVISO -- DESCRITTO, NON FATTO
 * ===========================================================================
 * `struct fts_ts_data` e' definita in focaltech_core.c (unita' C, di un altro
 * lotto) e SERVE anche qui.  Non esiste un header comune, e crearne uno e'
 * una modifica a un file che non e' mio: qui la struttura e' RIDICHIARATA
 * LOCALMENTE con i soli campi che QUESTA unita' misura, come fa gia'
 * ilitek_bus.c per le funzioni del gruppo E.  Il delta e' in coda.
 *
 * ===========================================================================
 * 5. LE QUATTRO DEL LOTTO A4, E COME SONO MISURATE
 * ===========================================================================
 *   fts_debug_read     0xffffff8008a7b708   416 byte   t
 *   fts_debug_write    0xffffff8008a7b8a8   600 byte   t
 *   fts_tprwreg_show   0xffffff8008a7bbf8   756 byte   t
 *   fts_tprwreg_store  0xffffff8008a7beec  1736 byte   t
 * Tutte e quattro sono `static`, come dice la mappa (`t`), e tutte e quattro
 * misurano la dimensione ESATTA di fabbrica col compilatore di fabbrica.  La
 * DIVERGENZA 1 del lotto precedente -- le quattro dichiarate non-`static`
 * perche' non erano definite -- e' CHIUSA da questo lotto.
 *
 * IL CONFRONTO PER CODIFICA, che vale piu' della dimensione:
 *   fts_debug_read     104 istruzioni su 104 con lo stesso mnemonico, residuo 0
 *   fts_debug_write    150 su 150, residuo 0
 *   fts_tprwreg_show   185 su 189, residuo 4
 *   fts_tprwreg_store  270 su 434, residuo 104
 * I residui delle ultime due sono in coda, nelle DIVERGENZE 4 e 5.
 *
 * ANCHE LA `.bss` DELL'UNITA' E' ORA SCRITTA
 * (0xffffff800a100a98..0xffffff800a100ac8, 48 byte, sei simboli): li scrive
 * fts_tprwreg_store e li legge fts_tprwreg_show, e nessun'altra funzione del
 * blocco li tocca.  La loro spaziatura di otto byte NON e' riprodotta, ed e'
 * la DIVERGENZA 6 in coda.
 *
 * Le altre unita' del blocco: B sta in focaltech_flash.c, C in
 * focaltech_core.c, D in focaltech_ex_mode.c, E in focaltech_i2c.c, F in
 * focaltech_ft8719_upgrade.c.  Con questo lotto TUTTE E SEI sono scritte.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "tpd.h"

/* Le tre macro, lette dai formati interi -- il livello KERN e' nel binario:
 *   "\x016[FTS][Info]Create proc entry success!\n"@0xffffff800924db58   (KERN_INFO)
 *   "\x013[FTS][Error]create proc entry fail\n"@0xffffff800924db32      (KERN_ERR)
 *   "[FTS]cmd len: %d, buf: %s\n"@0xffffff800924de6e                    (nessun livello)
 */
#define FTS_DEBUG(fmt, args...)		printk("[FTS]" fmt "\n", ##args)
#define FTS_INFO(fmt, args...)		printk(KERN_INFO "[FTS][Info]" fmt "\n", ##args)
#define FTS_ERROR(fmt, args...)		printk(KERN_ERR "[FTS][Error]" fmt "\n", ##args)

/* Costanti lette dagli immediati, non dai nomi.
 *   "528014c1 mov"@0xffffff8008a7bb44   w1 = 0xa6, versione del firmware
 *   "528014a1 mov"@0xffffff8008a7c5fc   w1 = 0xa5, modo di alimentazione
 *   "52801c81 mov"@0xffffff8008a7c654   w1 = 0xe4
 *   "528016a1 mov"@0xffffff8008a7c680   w1 = 0xb5
 *   "528016c1 mov"@0xffffff8008a7c6ac   w1 = 0xb6
 *   "52801501 mov"@0xffffff8008a7c6d8   w1 = 0xa8, identificativo del venditore
 *   "52801561 mov"@0xffffff8008a7c704   w1 = 0xab
 *   "52801a01 mov"@0xffffff8008a7c730   w1 = 0xd0
 *   "52801161 mov"@0xffffff8008a7c75c   w1 = 0x8b
 *   "528011e1 mov"@0xffffff8008a7c788   w1 = 0x8f
 *   "52801221 mov"@0xffffff8008a7c7b4   w1 = 0x91
 * I NOMI sono scelti dal messaggio che fts_dumpreg_show stampa accanto a
 * ognuno, che il binario porta per intero: sono l'unica fonte che li nomina,
 * e quindi non sono inventati (l'eccezione della regola 5).
 */
#define FTS_REG_POWER_MODE		0xa5	/* "Power Mode:0x%02x\n"@0xffffff800924e01c */
#define FTS_REG_FW_VER			0xa6	/* "FW Ver:0x%02x\n"@0xffffff800924e02f */
#define FTS_REG_LIC_VER			0xe4	/* "LCD Initcode Ver:0x%02x\n"@0xffffff800924e03e */
#define FTS_REG_PARAM_VER		0xb5	/* "Param Ver:0x%02x\n"@0xffffff800924e057 */
#define FTS_REG_PARAM_STATUS		0xb6	/* "Param status:0x%02x\n"@0xffffff800924e069 */
#define FTS_REG_VENDOR_ID		0xa8	/* "Vendor ID:0x%02x\n"@0xffffff800924e07e */
#define FTS_REG_LCD_BUSY_NUM		0xab	/* "LCD Busy Number:0x%02x\n"@0xffffff800924e090 */
#define FTS_REG_GESTURE_EN		0xd0	/* "Gesture Mode:0x%02x\n"@0xffffff800924e0a8 */
#define FTS_REG_CHARGER_MODE_EN		0x8b	/* "charge stat:0x%02x\n"@0xffffff800924e0bd */
#define FTS_REG_INT_CNT			0x8f	/* "INT count:0x%02x\n"@0xffffff800924e0d1 */
#define FTS_REG_ESD_SATURATE		0x91	/* "ESD count:0x%02x\n"@0xffffff800924e0e3 */

/* "Focaltech V2.2 20180321"@0xffffff800924e1c9, secondo argomento della
 * snprintf di fts_driverinfo_show: "91072463 add"@0xffffff8008a7ca90 mette in
 * x3 0x924e000 + 0x1c9.  E' la STESSA stringa che tpd_driver_init stampa
 * dall'unita' C. */
#define FTS_DRIVER_VERSION		"Focaltech V2.2 20180321"

/* "320023e1 orr"@0xffffff8008a7b620 -> w1 = 0x1ff = 0777, il modo del nodo
 * /proc.  NON e' 0644 come le otto device_attribute: il binario le distingue. */
#define FTS_PROC_MODE			0777

/* LA DIMENSIONE E' MISURATA, IL NOME E' UNA SCELTA (regola 4).
 * 256 e' l'estremo del confronto d'intervallo di tutte e due le meta' del
 * nodo /proc -- "f104013f cmp"@0xffffff8008a7b72c in lettura e
 * "f104013f cmp"@0xffffff8008a7b8d4 in scrittura -- ed e' anche la taglia
 * del buffer sulla pila: sedici "stp xzr,xzr" per 256 byte in lettura
 * (da 0xffffff8008a7b734 a 0xffffff8008a7b770). */
#define PROC_BUF_SIZE			256

/* ==========================================================================
 * IL CONTESTO DEL DRIVER, RIDICHIARATO LOCALMENTE
 * ==========================================================================
 * La taglia -- 384 byte -- e la maggior parte dei campi sono misurati
 * dall'unita' C (focaltech_core.c).  QUI sono dichiarati SOLO i campi che
 * QUESTA unita' tocca; il resto e' riempimento esplicito, perche' un nome
 * inventato sarebbe indistinguibile da un fatto (classe B2).
 *
 *   +0    struct i2c_client *
 *         "a9402515 ldp"@0xffffff8008a7bb2c   ldp x21,x9,[x8] -> c0 e c8
 *   +8    struct input_dev *
 *         "9107e133 add"@0xffffff8008a7bb34   x19 = c8 + 0x1f8, cioe'
 *         &input_dev->mutex: e' l'argomento di mutex_lock in NOVE funzioni
 *         di questa unita'.  0x1f8 e' l'offset di `mutex` in
 *         `struct input_dev` in QUESTO albero, ed e' un fatto dell'header.
 *   +32   u8   "394082c3 ldrb"@0xffffff8008a7cb0c  primo byte dell'identificativo
 *   +33   u8   "394086c4 ldrb"@0xffffff8008a7cb10  secondo byte
 *   +320  int  "b94142c3 ldr"@0xffffff8008a7caf0   il numero di interruzione
 *   +325  u8   "390516c8 strb"@0xffffff8008a7c8e0 = 1 prima dell'aggiornamento,
 *              "390516df strb"@0xffffff8008a7c900 = 0 dopo
 *   +368  struct proc_dir_entry *
 *         "f900ba60 str"@0xffffff8008a7b62c e "f940b800 ldr"@0xffffff8008a7b664
 *   +376  u8   scritto da fts_debug_write, letto da fts_debug_read -- le due
 *              funzioni NON scritte da questo lotto; il campo e' qui perche'
 *              senza di lui la taglia non torna
 */
struct fts_ts_data {
	struct i2c_client *c0;
	struct input_dev *c8;
	u8 c16[16];		/* non toccati da questa unita' */
	u8 c32;
	u8 c33;
	u8 c34[286];		/* non toccati da questa unita' */
	int c320;
	u8 c324;		/* non toccato da questa unita' */
	u8 c325;
	u8 c326[42];		/* non toccati da questa unita' */
	struct proc_dir_entry *c368;
	u8 c376;
	u8 c377[7];		/* fino a 384 = la taglia misurata dall'unita' C */
};

/* 0xffffff800a100ad8, letto da OGNI funzione di questa unita' con
 * "f9456d08 ldr"@0xffffff8008a7bb28 (ldr x8,[x8,#2776]).  E' definito
 * dall'unita' C, che lo scrive in tpd_probe; qui e' `extern`, e il
 * non-static la' e' PROVATO dal fatto che diciannove funzioni sparse su
 * quattro unita' di traduzione lo leggono. */
extern struct fts_ts_data *fts_data;

/* ==========================================================================
 * IL CONFINE CON LE ALTRE UNITA' -- DICHIARATE, NON DEFINITE QUI
 * ==========================================================================
 * Le firme vengono dai registri dei siti di chiamata.
 */
int fts_i2c_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue);
					/* unita' E, 0xffffff8008a80614 */
int fts_i2c_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue);
					/* unita' E, 0xffffff8008a805c4 */
int fts_i2c_read(struct i2c_client *client, char *writebuf, int writelen,
		 char *readbuf, int readlen);
					/* unita' E, 0xffffff8008a802ac */
int fts_i2c_write(struct i2c_client *client, char *writebuf, int writelen);
					/* unita' E, 0xffffff8008a80494 */
void fts_irq_disable(void);		/* unita' C, 0xffffff8008a7ecf0 */
void fts_irq_enable(void);		/* unita' C, 0xffffff8008a7ed74 */
int fts_reset_proc(int hdelayms);	/* unita' C, 0xffffff8008a7ec74 */
int fts_upgrade_bin(struct i2c_client *client, char *fw_name, bool force);
					/* unita' B, 0xffffff8008a7e464 -- NON SCRITTA */

/* ==========================================================================
 * LE QUATTRO FUNZIONI DEL LOTTO A4 -- DICHIARATE QUI, DEFINITE PIU' SOTTO
 * ==========================================================================
 * Servono in avanti perche' le tabelle di dati (le device_attribute e le
 * file_operations del nodo /proc) le nominano prima che siano definite.
 * Sono `static`, come nella mappa di fabbrica (`t`): il lotto precedente
 * dovette dichiararle non-`static` perche' non erano definite, ed era la sua
 * DIVERGENZA 1.  Ora sono definite, e la divergenza e' chiusa.
 */
static ssize_t fts_debug_read(struct file *filp, char __user *buff,
			      size_t count, loff_t *ppos);
							/* 0xffffff8008a7b708 */
static ssize_t fts_debug_write(struct file *filp, const char __user *buff,
			       size_t count, loff_t *ppos);
							/* 0xffffff8008a7b8a8 */
static ssize_t fts_tprwreg_show(struct device *dev,
				struct device_attribute *attr, char *buf);
							/* 0xffffff8008a7bbf8 */
static ssize_t fts_tprwreg_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count);
							/* 0xffffff8008a7beec */

/* Dichiarazioni in avanti: le tabelle di dati vengono prima delle funzioni,
 * come nel `.data` di fabbrica. */
static ssize_t fts_tpfwver_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t fts_tpfwver_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count);
static ssize_t fts_dumpreg_show(struct device *dev,
				struct device_attribute *attr, char *buf);
static ssize_t fts_dumpreg_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count);
static ssize_t fts_fwupgradebin_show(struct device *dev,
				     struct device_attribute *attr, char *buf);
static ssize_t fts_fwupgradebin_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count);
static ssize_t fts_fwforceupg_show(struct device *dev,
				   struct device_attribute *attr, char *buf);
static ssize_t fts_fwforceupg_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count);
static ssize_t fts_driverinfo_show(struct device *dev,
				   struct device_attribute *attr, char *buf);
static ssize_t fts_driverinfo_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count);
static ssize_t fts_hw_reset_show(struct device *dev,
				 struct device_attribute *attr, char *buf);
static ssize_t fts_hw_reset_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count);
static ssize_t fts_irq_show(struct device *dev,
			    struct device_attribute *attr, char *buf);
static ssize_t fts_irq_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count);

/*
 * ==========================================================================
 * LE OTTO device_attribute -- LETTE DALLE RILOCAZIONI, NON SUPPOSTE
 * ==========================================================================
 * HANDOFF registra che QUATTRO difetti su cinque di aw87329 vennero dall'aver
 * letto i permessi sysfs a occhio.  Qui i permessi sono letti dai byte.
 *
 * Ogni `struct device_attribute` e' 32 byte: `attr.name` a +0, `attr.mode`
 * (umode_t, due byte) a +8, `show` a +0x10, `store` a +0x18.  I byte a +8
 * di TUTTE E OTTO sono "a4010000", cioe' 0x1a4 = 420 decimale = 0644 ottale:
 *
 *   $ aarch64-linux-android-objdump -s --start-address=0xffffff800998c7c0 \
 *       --stop-address=0xffffff800998c8c0 oracolo/stock.elf
 *    ffffff800998c7c0 00000000 00000000 a4010000 00000000
 *    ffffff800998c7d0 00000000 00000000 00000000 00000000
 *    ffffff800998c7e0 00000000 00000000 a4010000 00000000
 *    [... le altre sei, tutte con "a4010000" a +8 ...]
 *    ffffff800998c8a0 00000000 00000000 a4010000 00000000
 *    ffffff800998c8b0 00000000 00000000 00000000 00000000
 *   $ python3 -c "print(oct(0x1a4))"
 *   0o644
 *
 * I puntatori valgono zero nell'immagine perche' il kernel di fabbrica e'
 * CONFIG_RELOCATABLE=y; il valore vero e' l'addend della rilocazione:
 *
 *   indirizzo             addend                 significato
 *   0xffffff800998c7c0 -> 0xffffff800924dd48     "fts_fw_version"
 *   0xffffff800998c7d0 -> 0xffffff8008a7bb00     fts_tpfwver_show
 *   0xffffff800998c7d8 -> 0xffffff8008a7bbf0     fts_tpfwver_store
 *   0xffffff800998c7e0 -> 0xffffff800924dd85     "fts_rw_reg"
 *   0xffffff800998c7f0 -> 0xffffff8008a7bbf8     fts_tprwreg_show
 *   0xffffff800998c7f8 -> 0xffffff8008a7beec     fts_tprwreg_store
 *   0xffffff800998c800 -> 0xffffff800924e00f     "fts_dump_reg"
 *   0xffffff800998c810 -> 0xffffff8008a7c5b4     fts_dumpreg_show
 *   0xffffff800998c818 -> 0xffffff8008a7c818     fts_dumpreg_store
 *   0xffffff800998c820 -> 0xffffff800924e0f5     "fts_upgrade_bin"
 *   0xffffff800998c830 -> 0xffffff8008a7c820     fts_fwupgradebin_show
 *   0xffffff800998c838 -> 0xffffff8008a7c828     fts_fwupgradebin_store
 *   0xffffff800998c840 -> 0xffffff800924e169     "fts_force_upgrade"
 *   0xffffff800998c850 -> 0xffffff8008a7c93c     fts_fwforceupg_show
 *   0xffffff800998c858 -> 0xffffff8008a7c944     fts_fwforceupg_store
 *   0xffffff800998c860 -> 0xffffff800924e1aa     "fts_driver_info"
 *   0xffffff800998c870 -> 0xffffff8008a7ca58     fts_driverinfo_show
 *   0xffffff800998c878 -> 0xffffff8008a7cb4c     fts_driverinfo_store
 *   0xffffff800998c880 -> 0xffffff800924e21e     "fts_hw_reset"
 *   0xffffff800998c890 -> 0xffffff8008a7cb54     fts_hw_reset_show
 *   0xffffff800998c898 -> 0xffffff8008a7cbb8     fts_hw_reset_store
 *   0xffffff800998c8a0 -> 0xffffff800924e23e     "fts_irq"
 *   0xffffff800998c8b0 -> 0xffffff8008a7cbc0     fts_irq_show
 *   0xffffff800998c8b8 -> 0xffffff8008a7cbc8     fts_irq_store
 *
 * e gli otto nomi, letti dai byte a quegli indirizzi:
 *   "fts_fw_version"@0xffffff800924dd48
 *   "fts_rw_reg"@0xffffff800924dd85
 *   "fts_dump_reg"@0xffffff800924e00f
 *   "fts_upgrade_bin"@0xffffff800924e0f5
 *   "fts_force_upgrade"@0xffffff800924e169
 *   "fts_driver_info"@0xffffff800924e1aa
 *   "fts_hw_reset"@0xffffff800924e21e
 *   "fts_irq"@0xffffff800924e23e
 *
 * DEVICE_ATTR(nome, 0644, show, store) genera esattamente questa forma: il
 * simbolo che ne esce si chiama `dev_attr_<nome>`, ed e' per questo che le
 * otto variabili qui sotto NON hanno un nome inventato -- lo stringa la macro
 * dal suo primo argomento, che il binario porta.
 */
/* ==========================================================================
 * I SEI SIMBOLI .bss DELL'UNITA', A 0xffffff800a100a98..0xffffff800a100ac8
 * ==========================================================================
 * Sono lo STATO CONDIVISO fra fts_tprwreg_store (che lo scrive) e
 * fts_tprwreg_show (che lo stampa): nessuna altra funzione del blocco li
 * tocca.  I NOMI SONO PRESI DALL'INDIRIZZO (regola 5): il binario non li
 * nomina, non c'e' nessuna macro che li stringhi e nessun formato che li
 * chiami per nome, quindi un nome descrittivo sarebbe indistinguibile da un
 * fatto.  Ciascuno porta accanto CIO' CHE E' MISURATO -- larghezza, segno,
 * e il ruolo quando una stringa di formato lo prova.
 *
 *   a98  UN BYTE, e solo il bit 0:
 *          "396a6108 ldrb"@0xffffff8008a7bc3c   lettura a un byte
 *          "36000968 tbz"@0xffffff8008a7bc58    prova il solo bit 0
 *          "392a611f strb"@0xffffff8008a7bf84   = 0
 *          "392a6109 strb"@0xffffff8008a7bfb0   = 1
 *        E' la forma con cui clang tratta un `bool`.  Il RUOLO e' provato
 *        dalle stringhe: quando vale 1 fts_tprwreg_show stampa "Write ...",
 *        quando vale 0 stampa "Read ...".
 *   aa0  QUATTRO BYTE: "b94aa143 ldr"@0xffffff8008a7bc54 (ldr w).
 *        E' il primo %02X di "Write Reg: [%02X]-[%02X]\n".
 *   aa8  QUATTRO BYTE CON SEGNO: "b94aaae9 ldr"@0xffffff8008a7bc30 e
 *        "37f801c9 tbnz"@0xffffff8008a7bc34 (il bit 31 e' il segno), e
 *        "b98aaae8 ldrsw"@0xffffff8008a7bd38 lo estende con segno per usarlo
 *        come limite di un ciclo a 64 bit: e' un `int`, non un `unsigned`.
 *   ab0  QUATTRO BYTE: "b94ab104 ldr"@0xffffff8008a7bdf0.
 *        E' il secondo %02X di "Read %02X: %02X\n".
 *   ab8  QUATTRO BYTE: "b94ab924 ldr"@0xffffff8008a7bc50.
 *        E' il %d di "Result: failed, ret: %d\n".
 *   ac0  OTTO BYTE, un puntatore: "f9456308 ldr"@0xffffff8008a7bcd4 (ldr x),
 *        "b4000408 cbz"@0xffffff8008a7bcdc lo confronta con NULL, e
 *        "38796903 ldrb"@0xffffff8008a7bd30 legge un BYTE all'indice: punta
 *        a byte.  fts_tprwreg_store lo alloca con __kmalloc e lo libera con
 *        kfree.
 *
 * LA SPAZIATURA DI OTTO BYTE FRA I SEI NON E' RIPRODOTTA DA QUESTE
 * DICHIARAZIONI, ed e' dichiarata come divergenza in coda al file: di
 * fabbrica ogni oggetto comincia a un multiplo di 8 (a98, aa0, aa8, ab0,
 * ab8, ac0 = 48 byte in tutto), mentre un `bool` seguito da quattro `int` e
 * da un puntatore, dichiarati cosi', ne occupa 32.  Costa ZERO byte di
 * testo -- gli spiazzamenti stanno tutti in campi rilocati -- e nessuna
 * misura di dimensione lo denuncia.
 */
u8 fts_g_a100a98;
int fts_g_a100aa0;
int fts_g_a100aa8;
int fts_g_a100ab0;
int fts_g_a100ab8;
char *fts_g_a100ac0;

static DEVICE_ATTR(fts_fw_version, 0644, fts_tpfwver_show, fts_tpfwver_store);
static DEVICE_ATTR(fts_rw_reg, 0644, fts_tprwreg_show, fts_tprwreg_store);
static DEVICE_ATTR(fts_dump_reg, 0644, fts_dumpreg_show, fts_dumpreg_store);
static DEVICE_ATTR(fts_upgrade_bin, 0644, fts_fwupgradebin_show, fts_fwupgradebin_store);
static DEVICE_ATTR(fts_force_upgrade, 0644, fts_fwforceupg_show, fts_fwforceupg_store);
static DEVICE_ATTR(fts_driver_info, 0644, fts_driverinfo_show, fts_driverinfo_store);
static DEVICE_ATTR(fts_hw_reset, 0644, fts_hw_reset_show, fts_hw_reset_store);
static DEVICE_ATTR(fts_irq, 0644, fts_irq_show, fts_irq_store);

/*
 * L'array a 0xffffff800998c778, NOVE slot: otto puntatori nell'ordine qui
 * sopra piu' il terminatore.
 *   0xffffff800998c7b8: non relocato   <- il NULL finale
 * e il gruppo a 0xffffff800998c750, che punta all'array:
 *   0xffffff800998c750: non relocato               .name
 *   0xffffff800998c758: non relocato               .is_visible
 *   0xffffff800998c760: non relocato               .is_bin_visible
 *   0xffffff800998c768: relocato, addend 0xffffff800998c778   .attrs
 *   0xffffff800998c770: non relocato               .bin_attrs
 * Il NOME del gruppo e' una scelta (regola 5): il binario non lo contiene.
 */
static struct attribute *fts_g_998c778[] = {
	&dev_attr_fts_fw_version.attr,
	&dev_attr_fts_rw_reg.attr,
	&dev_attr_fts_dump_reg.attr,
	&dev_attr_fts_upgrade_bin.attr,
	&dev_attr_fts_force_upgrade.attr,
	&dev_attr_fts_driver_info.attr,
	&dev_attr_fts_hw_reset.attr,
	&dev_attr_fts_irq.attr,
	NULL,
};

static struct attribute_group fts_g_998c750 = {
	.attrs = fts_g_998c778,
};

/*
 * IL NODO /proc, a 0xffffff8008f7f370.
 *   +0x10: relocato, addend 0xffffff8008a7b708   .read  = fts_debug_read
 *   +0x18: relocato, addend 0xffffff8008a7b8a8   .write = fts_debug_write
 *   tutti gli altri slot: non relocati, cioe' NULL -- in particolare NON c'e'
 *   `.owner` (+0), che THIS_MODULE riempirebbe.
 * Sta in `.rodata` (0x8f7f370 e' nella stessa corsa degli `of_device_id` del
 * blocco, 0x8f7f460 e 0x8f7f5f0), quindi e' `const`.
 */
static const struct file_operations fts_g_8f7f370 = {
	.read = fts_debug_read,
	.write = fts_debug_write,
};

/*
 * ===========================================================================
 * fts_create_apk_debug_channel @ 0xffffff8008a7b600, 100 byte, visibilita' T
 * ===========================================================================
 *   "912c9400 add"@0xffffff8008a7b618   x0 = "ftxxxx-debug"@0xffffff800924db25
 *   "320023e1 orr"@0xffffff8008a7b620   w1 = 0x1ff = 0777
 *   "aa1f03e2 mov"@0xffffff8008a7b624   x2 = NULL, cioe' la radice di /proc
 *   "910dc063 add"@0xffffff8008a7b61c   x3 = 0x8f7f370, le file_operations
 *   "97e1e78b bl"@0xffffff8008a7b628    proc_create
 *   "f900ba60 str"@0xffffff8008a7b62c   ts_data->c368 = il risultato
 *   "b40000c0 cbz"@0xffffff8008a7b630   e poi il confronto con NULL
 * Il risultato e' SCRITTO NELLA STRUTTURA PRIMA di essere controllato: la
 * `str` precede la `cbz`.  Il codice d'errore e' -12:
 *   "12800160 mov"@0xffffff8008a7b654   mov w0, #0xfffffff4 = -ENOMEM
 */
int fts_create_apk_debug_channel(struct fts_ts_data *ts_data)
{
	ts_data->c368 = proc_create("ftxxxx-debug", FTS_PROC_MODE, NULL,
				    &fts_g_8f7f370);
	if (NULL == ts_data->c368) {
		FTS_ERROR("create proc entry fail");
		return -ENOMEM;
	}

	FTS_INFO("Create proc entry success!");

	return 0;
}

/*
 * ===========================================================================
 * fts_release_apk_debug_channel @ 0xffffff8008a7b664, 28 byte, visibilita' T
 * ===========================================================================
 * SETTE istruzioni, e la cornice viene creata SOLO nel ramo che chiama:
 *   "f940b800 ldr"@0xffffff8008a7b664   x0 = ts_data->c368  (prima di tutto)
 *   "b40000a0 cbz"@0xffffff8008a7b668   se e' NULL, salta direttamente al `ret`
 *   "a9bf7bfd stp"@0xffffff8008a7b66c   la cornice, DOPO il confronto
 *   "97e1e8db bl"@0xffffff8008a7b674    proc_remove
 * Questa e' la forma che clang da' a `if (p) f(p);` in una funzione che non
 * fa altro: la funzione e' `void`, perche' nessun registro di ritorno viene
 * impostato in nessuno dei due rami.
 */
void fts_release_apk_debug_channel(struct fts_ts_data *ts_data)
{
	if (ts_data->c368)
		proc_remove(ts_data->c368);
}

/*
 * ===========================================================================
 * fts_create_sysfs @ 0xffffff8008a7b680, 100 byte, visibilita' T
 * ===========================================================================
 * Stessa forma di fts_ex_mode_init dell'unita' D, con lo STESSO difetto:
 *   "9100c013 add"@0xffffff8008a7b68c   x19 = client + 0x30 = &client->dev.kobj
 *   "911d4021 add"@0xffffff8008a7b694   x1 = 0x998c000 + 0x750, il gruppo
 *   "34000140 cbz"@0xffffff8008a7b6a0   se il risultato e' 0, salta al successo
 *   "97e2220d bl"@0xffffff8008a7b6bc    ... altrimenti sysfs_remove_group, cioe'
 *                                       toglie un gruppo che non e' stato creato
 *   "12800160 mov"@0xffffff8008a7b6c0   e restituisce -12 = -ENOMEM
 * NOTA: qui il codice d'errore e' -ENOMEM, mentre in fts_ex_mode_init
 * (unita' D) lo stesso ramo restituisce -EIO ("321d7be0 orr"@0xffffff8008a8025c).
 * Sono due funzioni diverse con lo stesso difetto e codici diversi, e si
 * riproducono tutti e due (regola 7).
 */
int fts_create_sysfs(struct i2c_client *client)
{
	int ret = 0;

	ret = sysfs_create_group(&client->dev.kobj, &fts_g_998c750);
	if (ret != 0) {
		FTS_ERROR("[EX]: sysfs_create_group() failed!!");
		sysfs_remove_group(&client->dev.kobj, &fts_g_998c750);
		return -ENOMEM;
	}

	FTS_INFO("[EX]: sysfs_create_group() succeeded!!");

	return 0;
}

/*
 * ===========================================================================
 * fts_remove_sysfs @ 0xffffff8008a7b6e4, 36 byte, visibilita' T
 * ===========================================================================
 *   "9100c000 add"@0xffffff8008a7b6f0   x0 = client + 0x30
 *   "97e221fe bl"@0xffffff8008a7b6f8    sysfs_remove_group
 *   "2a1f03e0 mov"@0xffffff8008a7b6fc   w0 = 0, cioe' `return 0`
 */
int fts_remove_sysfs(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &fts_g_998c750);

	return 0;
}

/*
 * ===========================================================================
 * fts_debug_read @ 0xffffff8008a7b708, 416 byte, visibilita' t
 * ===========================================================================
 * LA META' IN LETTURA DEL NODO /proc "ftxxxx-debug".
 *
 * IL BUFFER E' 256 BYTE ED E' AZZERATO, e tutti e due i fatti sono misurati:
 * SEDICI "stp xzr,xzr" consecutive da "a90fffff stp"@0xffffff8008a7b734 a
 * "a900ffff stp"@0xffffff8008a7b770 riempiono di zeri 256 byte a partire da
 * sp+8; il canarino sta subito sopra, a sp+264
 * ("f81d83a8 stur"@0xffffff8008a7b730, x29-40 = sp+264).
 * IL NOME `PROC_BUF_SIZE` E' UNA SCELTA, il valore 256 e' una misura.
 *
 * IL CONTROLLO SULLA LUNGHEZZA E' UN INTERVALLO, non due confronti:
 *   "d1000449 sub"@0xffffff8008a7b724   x9 = count - 1
 *   "f104013f cmp"@0xffffff8008a7b72c   x9 contro 0x100
 *   "540000e3 b.cc"@0xffffff8008a7b774  prosegue se count-1 < 256
 * cioe' `(count == 0) || (count > 256)` ripiegato da clang in un solo
 * confronto senza segno.  Il `sub` a SESSANTAQUATTRO bit dice che `count` e'
 * largo 64 bit (size_t), e la "2a1403e1 mov"@0xffffff8008a7b780 (w1, non x1)
 * dice che l'argomento della printk e' invece largo 32: e' un `(int)count`,
 * come in fts_fwupgradebin_store.
 *
 * IL CONTESTO ARRIVA DAL GLOBALE, NON DAL `filp`:
 *   "f9456d09 ldr"@0xffffff8008a7b794   x9 = fts_data
 *   "f9400120 ldr"@0xffffff8008a7b7a0   x0 = fts_data->c0, il client
 *   "3945e128 ldrb"@0xffffff8008a7b79c  w8 = fts_data->c376, UN BYTE
 * La `ldrb` a larghezza di byte e' cio' che prova che c376 e' un `u8`
 * (classe A4); fts_debug_write lo scrive con "3905e2e9 strb"@0xffffff8008a7b9b8.
 *
 * I DUE SOLI CASI SERVITI SONO 1 E 7, e sono confronti, non una tabella:
 *   "71001d1f cmp"@0xffffff8008a7b7a4   contro 7
 *   "7100051f cmp"@0xffffff8008a7b7ac   contro 1
 * (fts_debug_write serve undici casi e usa una tabella di salto).  I DUE
 * NOMI simbolici non ci sono nel binario e non li invento: restano 1 e 7,
 * col commento di cio' che il codice ne fa.
 *
 * LE LUNGHEZZE PASSATE A fts_i2c_read SONO DIVERSE NEI DUE CASI:
 *   "320003e4 orr"@0xffffff8008a7b7b8   w4 = 1        (caso 1)
 *   "2a1403e4 mov"@0xffffff8008a7b7e0   w4 = (int)count (caso 7)
 * e in tutti e due i casi writebuf e writelen sono nulli:
 *   "aa1f03e1 mov"@0xffffff8008a7b7bc   x1 = NULL
 *   "2a1f03e2 mov"@0xffffff8008a7b7c0   w2 = 0
 *
 * IL VALORE DI RITORNO E' ESTESO CON SEGNO DA 32 BIT IN TUTTI I RAMI:
 *   "93407e94 sxtw"@0xffffff8008a7b7ec  num_read_chars = buflen  (caso 7)
 *   "93407eb4 sxtw"@0xffffff8008a7b874  num_read_chars = ret     (errore)
 *   "aa1f03f4 mov"@0xffffff8008a7b7f4   num_read_chars = 0       (default)
 * cioe' num_read_chars e' un `int` usato poi in contesto a 64 bit; con un
 * `ssize_t` la sxtw del caso 7 non ci sarebbe.
 *
 * IL RAMO D'ERRORE DELLA LETTURA SALVA `ret` PRIMA DELLA printk:
 *   "2a0003f5 mov"@0xffffff8008a7b864   w21 = w0
 *   "97dadf19 bl"@0xffffff8008a7b870    printk
 *   "93407eb4 sxtw"@0xffffff8008a7b874  e solo dopo lo estende
 *
 * LE TRE STRINGHE, LETTE DAI BYTE, con l'istruzione che le materializza:
 *   FTS_ERROR "\x013[FTS][Error]apk proc read count(%d) fail\n"@0xffffff800924dbe9
 *             "912fa400 add"@0xffffff8008a7b77c   x0 = 0x924d000 + 0xbe9
 *   FTS_ERROR "\x013[FTS][Error][APK]: read iic error!!\n"@0xffffff800924dc15
 *             "91305400 add"@0xffffff8008a7b86c   x0 = 0x924d000 + 0xc15
 *   FTS_ERROR "\x013[FTS][Error][APK]: copy to user error!!\n"@0xffffff800924dc3c
 *             "9130f000 add"@0xffffff8008a7b854   x0 = 0x924d000 + 0xc3c
 * ed e' UNO SOLO per i due casi: clang ha fuso le due code identiche.
 */
static ssize_t fts_debug_read(struct file *filp, char __user *buff,
			      size_t count, loff_t *ppos)
{
	int ret = 0;
	int num_read_chars = 0;
	int buflen = count;
	char buf[PROC_BUF_SIZE] = { 0 };
	struct fts_ts_data *ts_data = fts_data;
	struct i2c_client *client = ts_data->c0;

	if ((count == 0) || (count > PROC_BUF_SIZE)) {
		FTS_ERROR("apk proc read count(%d) fail", (int)count);
		return -EINVAL;
	}

	switch (ts_data->c376) {
	case 1:
		ret = fts_i2c_read(client, NULL, 0, buf, 1);
		if (ret < 0) {
			FTS_ERROR("[APK]: read iic error!!");
			return ret;
		}
		num_read_chars = 1;
		break;

	case 7:
		ret = fts_i2c_read(client, NULL, 0, buf, buflen);
		if (ret < 0) {
			FTS_ERROR("[APK]: read iic error!!");
			return ret;
		}
		num_read_chars = buflen;
		break;

	default:
		break;
	}

	if (copy_to_user(buff, buf, num_read_chars)) {
		FTS_ERROR("[APK]: copy to user error!!");
		return -EFAULT;
	}

	return num_read_chars;
}

/*
 * ===========================================================================
 * fts_debug_write @ 0xffffff8008a7b8a8, 600 byte, visibilita' t
 * ===========================================================================
 * L'ALTRA META' DEL NODO /proc.  Stesso controllo d'intervallo della lettura
 * ("d1000449 sub"@0xffffff8008a7b8cc + "f104013f cmp"@0xffffff8008a7b8d4) e
 * stesso buffer da 256 byte, ma azzerato in DUE gruppi perche' il secondo
 * buffer glielo spezza: "f90036df str"@0xffffff8008a7b904 azzera sp+144 e
 * "f90017ff str"@0xffffff8008a7b920 azzera sp+40.
 *
 * IL PRIMO BYTE SCRITTO E' IL COMANDO, ED E' ANCHE CIO' CHE FINISCE IN c376:
 *   "3940a3e9 ldrb"@0xffffff8008a7b9ac   w9 = writebuf[0]
 *   "3905e2e9 strb"@0xffffff8008a7b9b8   ts_data->c376 = w9, UN BYTE
 * ed e' il valore che fts_debug_read rilegge.
 *
 * UNDICI CASI SERVITI DA UNA TABELLA DI SALTO, non da confronti:
 *   "51000528 sub"@0xffffff8008a7b9b0   w8 = writebuf[0] - 1
 *   "7100291f cmp"@0xffffff8008a7b9b4   contro 10
 *   "54000728 b.hi"@0xffffff8008a7b9bc  fuori intervallo -> default
 *   "1000008a adr"@0xffffff8008a7b9c8   x10 = 0xffffff8008a7b9d8, la base
 *   "3868692b ldrb"@0xffffff8008a7b9cc  w11 = tabella[w8]
 *   "8b0b094a add"@0xffffff8008a7b9d0   x10 = base + w11*4
 *   "d61f0140 br"@0xffffff8008a7b9d4
 * La tabella e' undici byte a 0xffffff8008f7f360 e li ho LETTI, non supposti:
 *   $ aarch64-linux-android-objdump -s --start-address=0xffffff8008f7f360 \
 *       --stop-address=0xffffff8008f7f380 oracolo/stock.elf
 *    ffffff8008f7f360 060a3232 32000016 32321b00 00000000  ..222...22......
 * cioe', con base 0xffffff8008a7b9d8 e passo 4:
 *   comando  1 -> 0x06 -> 0xffffff8008a7b9f0   writelen = 1
 *   comando  2 -> 0x0a -> 0xffffff8008a7ba00   writelen = 2
 *   comando  3 -> 0x32 -> 0xffffff8008a7baa0   uscita (default)
 *   comando  4 -> 0x32 -> uscita
 *   comando  5 -> 0x32 -> uscita
 *   comando  6 -> 0x00 -> 0xffffff8008a7b9d8   writelen = buflen - 1
 *   comando  7 -> 0x00 -> 0xffffff8008a7b9d8   idem (stesso ramo)
 *   comando  8 -> 0x16 -> 0xffffff8008a7ba30   stampa writebuf[1]
 *   comando  9 -> 0x32 -> uscita
 *   comando 10 -> 0x32 -> uscita
 *   comando 11 -> 0x1b -> 0xffffff8008a7ba44   confronto e reset
 * I cinque che vanno all'uscita sono INDISTINGUIBILI da `case` con corpo
 * vuoto: non li scrivo, perche' scriverli sarebbe affermare piu' di quanto
 * la misura permetta (regola 4).  I NOMI dei comandi non sono nel binario e
 * non li invento.
 *
 * IL RAMO 6/7 CONTROLLA LA LUNGHEZZA, I RAMI 1 E 2 NO:
 *   "51000662 sub"@0xffffff8008a7b9d8   w2 = buflen - 1
 *   "7100045f cmp"@0xffffff8008a7b9dc   contro 1
 *   "5400060b b.lt"@0xffffff8008a7b9e0  salta la scrittura se <= 0
 * ed e' un confronto a 32 bit CON SEGNO su `count-1`: con un `size_t` sarebbe
 * senza segno e a 64 bit.  E' la prova che esiste un `int` intermedio.
 *
 * LA COPIA DALL'UTENTE USA INVECE `count` A 64 BIT, non l'intero:
 *   "aa1303e2 mov"@0xffffff8008a7b9a0   x2 = count, larghezza piena
 * (nessuna sxtw): le due larghezze convivono nella stessa funzione e si
 * riproducono tutte e due.
 *
 * IL RAMO 11 SCRIVE IL TERMINATORE CON UN INDICE A 32 BIT ESTESO CON SEGNO:
 *   "b2607fe8 mov"@0xffffff8008a7ba64   x8 = 0xffffffff00000000
 *   "8b138108 add"@0xffffff8008a7ba68   x8 += count << 32
 *   "9360fd08 asr"@0xffffff8008a7ba70   x8 >>= 32 con segno
 *   "38286a9f strb"@0xffffff8008a7ba80  tmpbuf[x8] = 0
 * cioe' esattamente `sext32(buflen - 1)`.  Con `count - 1` a 64 bit clang
 * avrebbe emesso un solo `sub`: e' la seconda prova dell'intero intermedio.
 *
 * IL SECONDO BUFFER STA A sp+12 ("910033e0 add"@0xffffff8008a7ba54) e il
 * primo a sp+40: fra i due ci sono 28 byte.  LA TAGLIA 28 E' UNA SCELTA
 * VINCOLATA, NON UNA MISURA: il binario fissa lo SPIAZZAMENTO, e 28 e' la
 * taglia che lo riproduce se l'allineamento dell'oggetto e' 1.  Con
 * allineamento 4 andrebbe bene qualunque taglia fra 25 e 28.  Il confronto
 * per codifica di questa funzione e' il controllo che la tiene onesta.
 *
 * IL CONFRONTO E' UNA memcmp DI DODICI BYTE, non una strcmp:
 *   "940f7028 bl"@0xffffff8008a7ba84   __pi_memcmp
 *   "321e07e2 orr"@0xffffff8008a7ba7c  w2 = 12 = strlen("focal_driver")
 * e il terzo argomento della snprintf che riempie tmpbuf e' PAGE_SIZE:
 *   "321403e1 orr"@0xffffff8008a7ba58  w1 = 0x1000
 * su un buffer di 28 byte.  E' LO STESSO DIFETTO DI fts_fwupgradebin_store,
 * che la regola 7 dice di riprodurre, e per la stessa ragione l'avviso e'
 * spento qui sotto invece che aggirato scrivendo `sizeof(tmpbuf)`.
 *
 * LA copy_from_user FALLITA NON TORNA SUBITO: il ramo a 0xffffff8008a7bad4
 * azzera la CODA del buffer con "940f7147 bl"@0xffffff8008a7bae4 (__memset)
 * prima di stampare -- e' il corpo di `copy_from_user` stessa, che il kernel
 *
 * LE SETTE STRINGHE, LETTE DAI BYTE:
 *   FTS_ERROR "\x013[FTS][Error]apk proc wirte count(%d) fail\n"@0xffffff800924dc67
 *             ("wirte" e' un refuso della fabbrica, e si riproduce)
 *   FTS_ERROR "\x013[FTS][Error][APK]: copy from user error!!\n"@0xffffff800924dc94
 *   FTS_INFO  "\x016[FTS][Info][APK]: PROC_SET_TEST_FLAG = %x!!\n"@0xffffff800924dcc1
 *   FTS_ERROR "\x013[FTS][Error][APK]: write iic error!!\n"@0xffffff800924dcf0
 *   snprintf  "%s"@0xffffff8009100e35
 *   memcmp    "focal_driver"@0xffffff800924dd18
 *   FTS_INFO  "\x016[FTS][Info]APK execute HW Reset\n"@0xffffff800924dd25
 * "%s"@0xffffff8009100e35 e' un letterale cortissimo che il linker ha unito
 * con quelli di altre unita' (297 occorrenze nell'immagine): da solo NON
 * prova l'indirizzo, e cio' che lo ancora e' la coppia che lo materializza
 * dentro QUESTA funzione -- "b0003422 adrp"@0xffffff8008a7ba48 +
 * "9138d442 add"@0xffffff8008a7ba50.  E' lo stesso letterale che
 * fts_fwupgradebin_store cita con un'altra coppia.
 * incorpora, non codice del driver.
 */
static ssize_t fts_debug_write(struct file *filp, const char __user *buff,
			       size_t count, loff_t *ppos)
{
	int buflen = count;
	int writelen = 0;
	int ret = 0;
	char writebuf[PROC_BUF_SIZE] = { 0 };
	char tmpbuf[28];
	struct fts_ts_data *ts_data = fts_data;
	struct i2c_client *client = ts_data->c0;

	if ((count == 0) || (count > PROC_BUF_SIZE)) {
		FTS_ERROR("apk proc wirte count(%d) fail", (int)count);
		return -EINVAL;
	}

	if (copy_from_user(writebuf, buff, count)) {
		FTS_ERROR("[APK]: copy from user error!!");
		return -EFAULT;
	}

	ts_data->c376 = writebuf[0];

	switch (writebuf[0]) {
	case 1:
		writelen = 1;
		ret = fts_i2c_write(client, writebuf + 1, writelen);
		if (ret < 0) {
			FTS_ERROR("[APK]: write iic error!!");
			return ret;
		}
		break;

	case 2:
		writelen = 2;
		ret = fts_i2c_write(client, writebuf + 1, writelen);
		if (ret < 0) {
			FTS_ERROR("[APK]: write iic error!!");
			return ret;
		}
		break;

	case 6:
	case 7:
		writelen = buflen - 1;
		if (writelen > 0) {
			ret = fts_i2c_write(client, writebuf + 1, writelen);
			if (ret < 0) {
				FTS_ERROR("[APK]: write iic error!!");
				return ret;
			}
		}
		break;

	case 8:
		FTS_INFO("[APK]: PROC_SET_TEST_FLAG = %x!!", writebuf[1]);
		break;

	case 11:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wfortify-source"
		snprintf(tmpbuf, PAGE_SIZE, "%s", writebuf + 1);
#pragma clang diagnostic pop
		tmpbuf[buflen - 1] = '\0';
		if (memcmp(tmpbuf, "focal_driver", 12) == 0) {
			FTS_INFO("APK execute HW Reset");
			fts_reset_proc(1);
		}
		break;

	default:
		break;
	}

	return count;
}

/*
 * ===========================================================================
 * fts_tpfwver_show @ 0xffffff8008a7bb00, 240 byte
 * ===========================================================================
 * IL CLIENT ARRIVA DAL CONTESTO GLOBALE, NON DAL `dev`:
 *   "f9456d08 ldr"@0xffffff8008a7bb28   x8 = fts_data
 *   "a9402515 ldp"@0xffffff8008a7bb2c   x21 = fts_data->c0, x9 = fts_data->c8
 * mentre fts_dumpreg_show ricava il client dal `dev`
 * ("d1008015 sub"@0xffffff8008a7c5d0).  Sono due funzioni dello stesso file
 * che fanno la stessa cosa in due modi diversi, e si riproducono tutte e due.
 *
 * LE TRE snprintf CON FORMATO COSTANTE SONO STATE RIPIEGATE DAL COMPILATORE
 * in copie di byte piu' una costante.  Non e' una scelta di questo file: e'
 * cio' che clang fa a `snprintf(buf, PAGE_SIZE, "letterale")`.
 *   "a9402908 ldp"@0xffffff8008a7bb60 + "f840d109 ldur"@0xffffff8008a7bb5c
 *   + "a9002a88 stp"@0xffffff8008a7bb68 + "f800d289 stur"@0xffffff8008a7bb64
 *     -> 21 byte, cioe' "I2c transfer error!\n" (20) piu' il NUL
 *   "321d07f4 orr"@0xffffff8008a7bb98  -> w20 = 0x18 = 24, cioe'
 *     strlen("get tp fw version fail!\n"), il valore di ritorno ripiegato
 * La prima NON usa il valore di ritorno (nessun registro lo prende), la
 * seconda si': sono due statement diversi.
 *
 * IL CONFRONTO E' SU DUE VALORI:
 *   "7103fc7f cmp"@0xffffff8008a7bb70   fwver == 0xff
 *   "35000143 cbnz"@0xffffff8008a7bb78  fwver != 0 -> il ramo con "%02x\n"
 */
static ssize_t fts_tpfwver_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = fts_data->c0;
	struct input_dev *input_dev = fts_data->c8;
	ssize_t num_read_chars = 0;
	u8 fwver = 0;

	mutex_lock(&input_dev->mutex);

	if (fts_i2c_read_reg(client, FTS_REG_FW_VER, &fwver) < 0)
		sprintf(buf, "I2c transfer error!\n");

	if ((fwver == 0xFF) || (fwver == 0x00))
		num_read_chars = snprintf(buf, PAGE_SIZE, "get tp fw version fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "%02x\n", fwver);

	mutex_unlock(&input_dev->mutex);

	return num_read_chars;
}

/*
 * ===========================================================================
 * I SETTE STUB DA 8 BYTE
 * ===========================================================================
 * fts_tpfwver_store     @0xffffff8008a7bbf0
 * fts_dumpreg_store     @0xffffff8008a7c818
 * fts_fwupgradebin_show @0xffffff8008a7c820
 * fts_fwforceupg_show   @0xffffff8008a7c93c
 * fts_driverinfo_store  @0xffffff8008a7cb4c
 * fts_hw_reset_store    @0xffffff8008a7cbb8
 * fts_irq_show          @0xffffff8008a7cbc0
 *
 * DUE istruzioni ciascuno, identiche in tutti e sette:
 *   "92800000 mov"@0xffffff8008a7bbf0   mov x0, #-1   (SESSANTAQUATTRO bit)
 *   "d65f03c0 ret"@0xffffff8008a7bbf4
 * Sono stub VERI della fabbrica, non omissioni di questo lotto: non c'e'
 * cornice, nessun parametro viene letto, nessuna chiamata.
 * La `mov x0` a 64 bit e non `mov w0` dice che il tipo restituito e' largo
 * 64 bit, cioe' `ssize_t` e non `int`: con `int` clang avrebbe emesso
 * "12800000 mov w0, #-1".  E' la classe A4 applicata al valore di ritorno.
 * Insieme all'ottavo (fts_ex_mode_recovery, unita' D) fanno le OTTO funzioni
 * da 8 byte del blocco.
 */
static ssize_t fts_tpfwver_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	return -1;
}

/*
 * ===========================================================================
 * fts_tprwreg_show @ 0xffffff8008a7bbf8, 756 byte, visibilita' t
 * ===========================================================================
 * STAMPA LO STATO LASCIATO DA fts_tprwreg_store: non tocca l'i2c, non
 * chiama nulla oltre a mutex_lock/mutex_unlock e a snprintf.  Il MUTEX
 * viene dal contesto globale, come in fts_tpfwver_show:
 *   "f9400508 ldr"@0xffffff8008a7bc1c + "9107e113 add"@0xffffff8008a7bc20
 *
 * TRE RAMI, DECISI DAL SOLO aa8:
 *   "37f801c9 tbnz"@0xffffff8008a7bc34   aa8 < 0   -> "Invalid cmd line"
 *   "7100053f cmp"@0xffffff8008a7bc40    aa8 == 1  -> il registro singolo
 *   "54000241 b.ne"@0xffffff8008a7bc44   altrimenti -> il blocco
 * e dentro i primi due il ramo secondario e' su a98 (Write contro Read) e
 * poi su ab8 CON UN CONFRONTO A ZERO, non col segno:
 *   "34000c84 cbz"@0xffffff8008a7bc5c    ab8 == 0 -> "success"
 * (con `ab8 < 0` clang avrebbe emesso una tbnz sul bit 31, come fa per aa8
 * quattro istruzioni sopra: le due forme convivono e si distinguono).
 *
 * LE QUATTRO snprintf A FORMATO COSTANTE SONO RIPIEGATE IN COPIE DI BYTE:
 *   "a9402508 ldp"@0xffffff8008a7bc74 + "7900228a strh"@0xffffff8008a7bc7c
 *     -> 16 byte + "\n\0", cioe' "Invalid cmd line\n"; e il valore di
 *        ritorno e' la costante "52800235 mov"@0xffffff8008a7bc80, w21 = 17
 *   "f9400108 ldr"@0xffffff8008a7bcc0 + "f8405109 ldur"@0xffffff8008a7bcbc
 *     -> 13 byte, "Write Data: " piu' il NUL; ritorno 12
 *        ("11003016 add"@0xffffff8008a7bcd8, w22 = w0 + 12)
 *   "f9400108 ldr"@0xffffff8008a7bdb0 + "3900213f strb"@0xffffff8008a7bdb8
 *     -> 8 byte + NUL, "Result: "; ritorno 8
 *        ("11002015 add"@0xffffff8008a7bdbc)
 *   "a9402508 ldp"@0xffffff8008a7be08 + "3900401f strb"@0xffffff8008a7be10
 *     -> 16 byte + NUL, "Result: success\n"; ritorno 16
 *        ("110042d5 add"@0xffffff8008a7be0c)
 * e il solo "\n" diventa una half-word:
 *   "52800148 mov"@0xffffff8008a7bd50 + "79000008 strh"@0xffffff8008a7bd58
 *     -> 0x000a, cioe' '\n' e il NUL; e il contatore cresce di UNO
 *        ("110006d6 add"@0xffffff8008a7bd54).
 *
 * L'ACCUMULATORE E' UN `int`, ESTESO CON SEGNO A OGNI USO:
 *   "8b36c280 add"@0xffffff8008a7bce4   x0 = buf + w22, sxtw
 *   "93407ea0 sxtw"@0xffffff8008a7be44  e una volta sola all'uscita
 * esattamente come in fts_dumpreg_show.
 *
 * I DUE CICLI PARTONO DA INDICI DIVERSI, ed e' il punto:
 *   ramo Write: "7100093f cmp"@0xffffff8008a7bce8 (aa8 contro 2) e
 *               "39400503 ldrb"@0xffffff8008a7bcf0 legge ac0[1]
 *   ramo Read:  "7100053f cmp"@0xffffff8008a7be74 (aa8 contro 1) e
 *               "39400103 ldrb"@0xffffff8008a7be7c legge ac0[0]
 * cioe' la scrittura salta il primo byte (che e' l'indirizzo del registro)
 * e la lettura no.
 *
 * E I DUE ESTREMI STAMPATI SONO DIVERSI DI UNO:
 *   "51000484 sub"@0xffffff8008a7bca0   Write: aa0 + aa8 - 1
 *   (nessuna sub nel ramo Read)         Read:  aa0 + aa8
 * mentre "0b090064 add"@0xffffff8008a7bc94 e' comune ai due.  E' una
 *
 * LE TREDICI STRINGHE, LETTE DAI BYTE:
 *   "Invalid cmd line\n"@0xffffff800924dd90
 *   "Read %02X: %02X\n"@0xffffff800924dda2
 *   "Read %02X failed, ret: %d\n"@0xffffff800924ddb3
 *   "Write %02X, %02X success\n"@0xffffff800924ddce
 *   "Write %02X failed, ret: %d\n"@0xffffff800924dde8
 *   "Read Reg: [%02X]-[%02X]\n"@0xffffff800924de04
 *   "Write Reg: [%02X]-[%02X]\n"@0xffffff800924de1d
 *   "Write Data: "@0xffffff800924de37
 *   "Result: failed, ret: %d\n"@0xffffff800924de44
 *   "failed, ret: %d\n"@0xffffff800924de4c
 *   "Result: success\n"@0xffffff800924de5d
 *   "%02X "@0xffffff80091b570b
 *   "Result: "@0xffffff8009223ea5
 *
 * "failed, ret: %d\n"@0xffffff800924de4c E' LA CODA DI
 * "Result: failed, ret: %d\n"@0xffffff800924de44 -- otto byte piu' avanti
 * dentro la stessa stringa -- e il binario le usa TUTTE E DUE, in due rami
 * diversi ("940fba12 bl"@0xffffff8008a7bd78 e "940fb9f8 bl"@0xffffff8008a7bde0):
 * e' il linker che ha sovrapposto un letterale al suffisso di un altro, non
 * un errore di lettura.
 *
 * LE ULTIME DUE STANNO FUORI dalla corsa di stringhe del blocco
 * (0x924db25..0x9250380): letterali corti uniti dal linker con quelli di
 * altre unita'.  Da soli non provano l'indirizzo; cio' che li ancora e' la
 * coppia che li materializza dentro QUESTA funzione:
 *   "d00039c2 adrp"@0xffffff8008a7bcf4 + "911c2c42 add"@0xffffff8008a7bcf8
 *     -> x2 = 0x91b5000 + 0x70b = "%02X "
 *   "90003d48 adrp"@0xffffff8008a7bda8 + "913a9508 add"@0xffffff8008a7bdac
 *     -> x8 = 0x9223000 + 0xea5 = "Result: "
 * incoerenza della fabbrica -- lo stesso intervallo stampato in due modi --
 * e la regola 7 dice di riprodurla.
 */
static ssize_t fts_tprwreg_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct input_dev *input_dev = fts_data->c8;
	int count;
	int i;

	mutex_lock(&input_dev->mutex);

	if (fts_g_a100aa8 < 0) {
		count = snprintf(buf, PAGE_SIZE, "Invalid cmd line\n");
	} else if (fts_g_a100aa8 == 1) {
		if (fts_g_a100a98 & 0x01) {
			if (fts_g_a100ab8)
				count = snprintf(buf, PAGE_SIZE,
						 "Write %02X failed, ret: %d\n",
						 fts_g_a100aa0, fts_g_a100ab8);
			else
				count = snprintf(buf, PAGE_SIZE,
						 "Write %02X, %02X success\n",
						 fts_g_a100aa0, fts_g_a100ab0);
		} else {
			if (fts_g_a100ab8)
				count = snprintf(buf, PAGE_SIZE,
						 "Read %02X failed, ret: %d\n",
						 fts_g_a100aa0, fts_g_a100ab8);
			else
				count = snprintf(buf, PAGE_SIZE,
						 "Read %02X: %02X\n",
						 fts_g_a100aa0, fts_g_a100ab0);
		}
	} else {
		if (fts_g_a100a98 & 0x01) {
			count = snprintf(buf, PAGE_SIZE,
					 "Write Reg: [%02X]-[%02X]\n",
					 fts_g_a100aa0,
					 fts_g_a100aa0 + fts_g_a100aa8 - 1);
			count += snprintf(buf + count, PAGE_SIZE, "Write Data: ");
			if (fts_g_a100ac0) {
				for (i = 1; i < fts_g_a100aa8; i++)
					count += snprintf(buf + count, PAGE_SIZE,
							  "%02X ",
							  fts_g_a100ac0[i]);
				count += snprintf(buf + count, PAGE_SIZE, "\n");
			}
			if (fts_g_a100ab8)
				count += snprintf(buf + count, PAGE_SIZE,
						  "Result: failed, ret: %d\n",
						  fts_g_a100ab8);
			else
				count += snprintf(buf + count, PAGE_SIZE,
						  "Result: success\n");
		} else {
			count = snprintf(buf, PAGE_SIZE,
					 "Read Reg: [%02X]-[%02X]\n",
					 fts_g_a100aa0,
					 fts_g_a100aa0 + fts_g_a100aa8);
			count += snprintf(buf + count, PAGE_SIZE, "Result: ");
			if (fts_g_a100ab8) {
				count += snprintf(buf + count, PAGE_SIZE,
						  "failed, ret: %d\n",
						  fts_g_a100ab8);
			} else {
				if (fts_g_a100ac0) {
					for (i = 0; i < fts_g_a100aa8; i++)
						count += snprintf(buf + count,
								  PAGE_SIZE,
								  "%02X ",
								  fts_g_a100ac0[i]);
					count += snprintf(buf + count, PAGE_SIZE,
							  "\n");
				}
			}
		}
	}

	mutex_unlock(&input_dev->mutex);

	return count;
}

/*
 * ===========================================================================
 * LA CONVERSIONE DI UNA CIFRA ESADECIMALE -- INCORPORATA DIECI VOLTE
 * ===========================================================================
 * Nel binario NON esiste nessun simbolo per questa funzione: e' `static` ed
 * e' stata incorporata in tutti i suoi siti d'uso dentro fts_tprwreg_store.
 * I DUE NOMI (fts_hex_char, fts_hex_byte) SONO UNA SCELTA, non una misura.
 *
 * Che gli intervalli siano quelli e non altri e' misurato, e sono TRE, non
 * due, e il terzo arriva fino alla 'z' e alla 'Z' invece che alla 'f' e alla
 * 'F' -- e' un difetto della fabbrica ("zz" si converte in 245) che la
 * regola 7 dice di riprodurre:
 *   "5100c109 sub"@0xffffff8008a7bf90   c - '0'
 *   "7100293f cmp"@0xffffff8008a7bf94   contro 10   -> le cifre
 *   "51018509 sub"@0xffffff8008a7bff4   c - 'a'
 *   "7100693f cmp"@0xffffff8008a7bff8   contro 26   -> VENTISEI lettere
 *   "51010509 sub"@0xffffff8008a7c040   c - 'A'
 *   "7100653f cmp"@0xffffff8008a7c044   contro 25 con b.hi -> altre ventisei
 * e i tre spostamenti di ritorno sono costanti nel binario:
 *   "128005e9 mov"@0xffffff8008a7bf9c   -48  = -'0'
 *   "12800ac9 mov"@0xffffff8008a7c000   -87  = -'a' + 10
 *   "128006c9 mov"@0xffffff8008a7c04c   -55  = -'A' + 10
 *   "128002a8 mov"@0xffffff8008a7c0d8   -22  = -EINVAL, il caso non valido
 * Clang calcola lo SPOSTAMENTO e poi lo somma al carattere
 * ("0b080128 add"@0xffffff8008a7c0c8), invece di sottrarre subito: e' la
 * forma che prende quando il risultato resta un `int`.
 *
 * QUANDO IL RISULTATO VIENE TRONCATO A UN BYTE la stessa sorgente da' un
 * codice diverso, e la differenza e' la prova che la destinazione e' un `u8`:
 *   "5100c128 sub"@0xffffff8008a7c020   sottrae subito, niente spostamento
 *   "51015d28 sub"@0xffffff8008a7c038   c - 0x57 = c - 87
 *   "5100dd28 sub"@0xffffff8008a7c2c4   c - 0x37 = c - 55
 *   "52801d48 mov"@0xffffff8008a7c304   234 = (u8)(-22)
 * Le due forme convivono nella stessa funzione e si distinguono a occhio.
 *
 * IL BYTE ALTO E' IL PRIMO CARATTERE, IL BASSO IL SECONDO:
 *   "0b091108 add"@0xffffff8008a7c0d0   w8 = basso + (alto << 4)
 * e il carattere BASSO e' letto per PRIMO
 * ("394006c8 ldrb"@0xffffff8008a7bf8c legge s[1], "394002ca ldrb"
 * @0xffffff8008a7c050 legge s[0]): l'ordine e' lo stesso in tutte e cinque
 * le copie.
 */
static u8 fts_hex_ubyte(const char *s)
{
	u8 hi;
	u8 lo;
	char c;

	c = s[1];
	if ((c >= '0') && (c <= '9'))
		lo = c - '0';
	else if ((c >= 'a') && (c <= 'z'))
		lo = c - 'a' + 10;
	else if ((c >= 'A') && (c <= 'Z'))
		lo = c - 'A' + 10;
	else
		return -EINVAL;

	c = s[0];
	if ((c >= '0') && (c <= '9'))
		hi = c - '0';
	else if ((c >= 'a') && (c <= 'z'))
		hi = c - 'a' + 10;
	else if ((c >= 'A') && (c <= 'Z'))
		hi = c - 'A' + 10;
	else
		return -EINVAL;

	return lo + (hi << 4);
}

static int fts_hex_byte(const char *s)
{
	int hi;
	int lo;
	char c;

	c = s[1];
	if ((c >= '0') && (c <= '9'))
		lo = c - '0';
	else if ((c >= 'a') && (c <= 'z'))
		lo = c - 'a' + 10;
	else if ((c >= 'A') && (c <= 'Z'))
		lo = c - 'A' + 10;
	else
		return -EINVAL;

	c = s[0];
	if ((c >= '0') && (c <= '9'))
		hi = c - '0';
	else if ((c >= 'a') && (c <= 'z'))
		hi = c - 'a' + 10;
	else if ((c >= 'A') && (c <= 'Z'))
		hi = c - 'A' + 10;
	else
		return -EINVAL;

	return lo + (hi << 4);
}

/*
 * ===========================================================================
 * fts_tprwreg_store @ 0xffffff8008a7beec, 1736 byte, visibilita' t
 * ===========================================================================
 * L'UNICA SCRITTRICE DEI SEI SIMBOLI .bss.  Riceve dal nodo sysfs una riga
 * di cifre esadecimali, la interpreta e fa il trasferimento i2c; e'
 * fts_tprwreg_show a stampare poi cio' che questa lascia.
 *
 * IL CLIENT VIENE DAL `dev` E IL MUTEX DAL GLOBALE, come in fts_dumpreg_show:
 *   "d10082a0 sub"@0xffffff8008a7c1b4   x0 = dev - 0x20 = to_i2c_client(dev)
 *   "f9400508 ldr"@0xffffff8008a7bf2c + "9107e114 add"@0xffffff8008a7bf30
 *
 * IL BUFFER PRECEDENTE E' LIBERATO PRIMA DI TUTTO:
 *   "b4000060 cbz"@0xffffff8008a7bf48   se ac0 non e' NULL
 *   "97df5219 bl"@0xffffff8008a7bf4c    kfree
 *   "f905633f str"@0xffffff8008a7bf50   e ac0 = NULL
 *
 * LA LUNGHEZZA E' LETTA CON DUE SEGNI DIVERSI NELLA STESSA FUNZIONE, e non
 * e' un caso ne' un errore di lettura:
 *   "f10012ff cmp"@0xffffff8008a7bfd0 + "5400024c b.gt"@0xffffff8008a7bfd4
 *     confronto CON SEGNO a 64 bit: `cmdlen > 4` con `cmdlen` di tipo ssize_t
 *   "eb28c2ff cmp"@0xffffff8008a7c3e8 + "540000c2 b.cs"@0xffffff8008a7c3ec
 *     confronto SENZA SEGNO fra lo stesso valore e un `int` esteso con segno:
 *     e' `count - 1` (size_t) contro `len * 2 + 5` (int), cioe' la
 *     conversione abituale che porta l'int nel dominio senza segno.
 * Con una sola delle due forme una delle due istruzioni non torna.
 *
 * TRE FORME DI COMANDO, DECISE SULLA LUNGHEZZA:
 *   "f100167f cmp"@0xffffff8008a7bf68   count == 5  -> scrittura di un byte
 *   "f1000e7f cmp"@0xffffff8008a7bf70   count == 3  -> lettura di un byte
 *   "f10012ff cmp"@0xffffff8008a7bfd0   cmdlen > 4  -> il blocco
 * e sotto ai 3 byte si esce con -EINVAL e il proprio mutex_unlock:
 *   "928002b3 mov"@0xffffff8008a7bfec   x19 = -22
 *
 * NEL BLOCCO IL PRIMO CARATTERE DECIDE IL VERSO, ED E' IL CARATTERE '1':
 *   "7100c51f cmp"@0xffffff8008a7c3a8   buf[0] contro 0x31
 * (non il valore 1: e' il CARATTERE, perche' la riga e' testo).
 *
 * IL BUFFER E' AZZERATO DALL'ALLOCATORE, e il binario lo dice nel flag:
 *   "52901801 mov"@0xffffff8008a7c434 + "72a02801 movk"@0xffffff8008a7c43c
 *     -> w1 = 0x014080c0
 * mentre in questo albero
 *   GFP_KERNEL = ___GFP_DIRECT_RECLAIM|___GFP_KSWAPD_RECLAIM|___GFP_IO|___GFP_FS
 *              = 0x400000|0x1000000|0x40|0x80 = 0x014000c0
 * e la differenza e' esattamente ___GFP_ZERO (0x8000): e' una `kzalloc`,
 * non una `kmalloc`.  La chiamata e' `__kmalloc`
 * ("97df4fd2 bl"@0xffffff8008a7c440) perche' la taglia non e' costante.
 * La taglia e' estesa CON SEGNO ("93407c80 sxtw"@0xffffff8008a7c438): e'
 * un `int` che diventa `size_t`.
 *
 * L'INDICE DEL CICLO CRESCE DI DUE NEL TESTO E DI UNO NEL BUFFER:
 *   "8b3cc349 add"@0xffffff8008a7c48c   x9 = (buf+3) + w28, esteso con segno
 *   "11000b9c add"@0xffffff8008a7c520   w28 += 2
 *   "910006f7 add"@0xffffff8008a7c51c   i  += 1
 *   "38376ac8 strb"@0xffffff8008a7c510  rw_buf[i] = valore, UN BYTE
 * e il limite del ciclo e' RILETTO DALLA MEMORIA A OGNI GIRO:
 *   "b98aab64 ldrsw"@0xffffff8008a7c518
 * Questa rilettura e' la prova che i sei simboli NON sono `static`: se lo
 * fossero, e nessuno ne prendesse l'indirizzo, clang saprebbe che la printk
 * del giro precedente non puo' averli cambiati e terrebbe il limite in un
 * registro.  La stessa prova si ripete a "b94aaac4 ldr"@0xffffff8008a7c3d0,
 * subito dopo la printk del ramo di lettura.
 *
 * IL MESSAGGIO DI SUCCESSO DELLA SCRITTURA STAMPA IL DATO, NON IL REGISTRO:
 *   "b94ab101 ldr"@0xffffff8008a7c228   w1 = ab0, cioe' il valore scritto
 * mentre quello della lettura stampa il registro:
 *   "b94aa101 ldr"@0xffffff8008a7c268   w1 = aa0
 * pur avendo tutti e due il formato "0x%02x".  E' una incoerenza della
 * fabbrica e si riproduce (regola 7).
 *
 * LA printk DI "data invalided!" NON PASSA PER LA MACRO:
 *   "\x013data invalided!\n"@0xffffff800924df90
 * ha il livello KERN_ERR ma NON il prefisso "[FTS][Error]" che FTS_ERROR
 * aggiunge sempre, e non ha il "\n" della macro: e' una printk nuda.  Anche
 *
 * LE TREDICI STRINGHE, LETTE DAI BYTE:
 *   FTS_DEBUG "[FTS]cmd len: %d, buf: %s\n"@0xffffff800924de6e
 *   FTS_ERROR "\x013[FTS][Error]Invalid cmd buffer\n"@0xffffff800924de89
 *   FTS_ERROR "\x013[FTS][Error]cmd buffer error!\n"@0xffffff800924deab
 *   FTS_ERROR "\x013[FTS][Error]Could not read 0x%02x\n"@0xffffff800924decc
 *   FTS_INFO  "\x016[FTS][Info]read 0x%02x, %d bytes successful\n"@0xffffff800924def1
 *   FTS_ERROR "\x013[FTS][Error]Could not write 0x%02x\n"@0xffffff800924df20
 *   FTS_INFO  "\x016[FTS][Info]Write 0x%02x, %d bytes successful\n"@0xffffff800924df46
 *   FTS_DEBUG "[FTS]read %02X, %d bytes\n"@0xffffff800924df76
 *   printk    "\x013data invalided!\n"@0xffffff800924df90
 *   FTS_DEBUG "[FTS]write %02X, %d bytes\n"@0xffffff800924dfa3
 *   FTS_ERROR "\x013[FTS][Error]allocate memory failed!\n\n"@0xffffff800924dfbe
 *   FTS_DEBUG "[FTS]write buffer: \n"@0xffffff800924dfe6
 *   FTS_DEBUG "[FTS]buf[%d]: %02X\n"@0xffffff800924dffb
 *
 * DUE DI QUESTE TREDICI SONO DIFETTI DELLA FABBRICA E SI RIPRODUCONO:
 *   - "\x013data invalided!\n" ha il livello KERN_ERR ma NON il prefisso
 *     "[FTS][Error]" e NON il "\n" che la macro aggiunge sempre: e' una
 *     printk NUDA in mezzo a dodici chiamate di macro;
 *   - "allocate memory failed!\n\n" ha DUE ritorni a capo, perche' il "\n"
 *     e' scritto nel formato passato a FTS_ERROR e la macro ne aggiunge un
 *     altro.
 * questo e' un difetto della fabbrica e si riproduce.
 */
static ssize_t fts_tprwreg_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct input_dev *input_dev = fts_data->c8;
	ssize_t cmdlen = count - 1;
	char *rw_buf = NULL;
	char regaddr;
	u8 regvalue;
	int len = 0;
	int num = 0;
	int ret = 0;
	int i = 0;

	mutex_lock(&input_dev->mutex);

	if (fts_g_a100ac0) {
		kfree(fts_g_a100ac0);
		fts_g_a100ac0 = NULL;
	}

	FTS_DEBUG("cmd len: %d, buf: %s", (int)cmdlen, buf);

	if (count == 3) {
		fts_g_a100a98 = 0;
		fts_g_a100aa8 = 1;
		fts_g_a100aa0 = fts_hex_byte(&buf[0]);
	} else if (count == 5) {
		fts_g_a100a98 = 1;
		fts_g_a100aa8 = 1;
		fts_g_a100aa0 = fts_hex_byte(&buf[0]);
		fts_g_a100ab0 = fts_hex_byte(&buf[2]);
	} else {
		if (cmdlen <= 4) {
			FTS_ERROR("Invalid cmd buffer");
			mutex_unlock(&input_dev->mutex);
			return -EINVAL;
		}

		regaddr = fts_hex_byte(&buf[1]);
		len = fts_hex_byte(&buf[3]);
		fts_g_a100aa0 = regaddr;

		if (buf[0] == '1') {
			fts_g_a100aa8 = len;
			fts_g_a100a98 = 0;
			FTS_DEBUG("read %02X, %d bytes", regaddr, len);
		} else {
			if ((count - 1) < (len * 2 + 5)) {
				printk(KERN_ERR "data invalided!\n");
				num = -EINVAL;
				goto setlen;
			}
			FTS_DEBUG("write %02X, %d bytes", regaddr, len);
			fts_g_a100a98 = 1;
			fts_g_a100aa8 = len + 1;
		}

		num = fts_g_a100aa8;
		if (num > 0) {
			rw_buf = kzalloc(num, GFP_KERNEL);
			if (NULL == rw_buf) {
				FTS_ERROR("allocate memory failed!\n");
				num = -ENOMEM;
				goto setlen;
			}

			if (fts_g_a100a98 & 0x01) {
				rw_buf[0] = fts_g_a100aa0;
				FTS_DEBUG("write buffer: ");
				for (i = 1; i < fts_g_a100aa8; i++) {
					rw_buf[i] = fts_hex_ubyte(&buf[i * 2 + 3]);
					FTS_DEBUG("buf[%d]: %02X", i, rw_buf[i]);
				}
			}

			num = fts_g_a100aa8;
			fts_g_a100ac0 = rw_buf;
		}
setlen:
		fts_g_a100aa8 = num;
	}

	if (fts_g_a100aa8 < 0) {
		FTS_ERROR("cmd buffer error!");
	} else if (fts_g_a100a98 & 0x01) {
		if (fts_g_a100aa8 == 1)
			ret = fts_i2c_write_reg(client, fts_g_a100aa0,
						fts_g_a100ab0);
		else
			ret = fts_i2c_write(client, fts_g_a100ac0,
					    fts_g_a100aa8);
		fts_g_a100ab8 = ret;

		if (ret < 0) {
			FTS_ERROR("Could not write 0x%02x", fts_g_a100aa0);
		} else {
			FTS_INFO("Write 0x%02x, %d bytes successful",
				 fts_g_a100ab0, fts_g_a100aa8);
			fts_g_a100ab8 = 0;
		}
	} else {
		if (fts_g_a100aa8 == 1) {
			ret = fts_i2c_read_reg(client, fts_g_a100aa0, &regvalue);
			fts_g_a100ab8 = ret;
			fts_g_a100ab0 = regvalue;
		} else {
			regaddr = fts_g_a100aa0;
			ret = fts_i2c_read(client, &regaddr, 1,
					   fts_g_a100ac0, fts_g_a100aa8);
			fts_g_a100ab8 = ret;
		}

		if (ret < 0) {
			FTS_ERROR("Could not read 0x%02x", fts_g_a100aa0);
		} else {
			FTS_INFO("read 0x%02x, %d bytes successful",
				 fts_g_a100aa0, fts_g_a100aa8);
			fts_g_a100ab8 = 0;
		}
	}

	mutex_unlock(&input_dev->mutex);

	return count;
}

/*
 * ===========================================================================
 * fts_dumpreg_show @ 0xffffff8008a7c5b4, 612 byte
 * ===========================================================================
 * UNDICI registri letti e stampati uno dietro l'altro, senza controllare
 * nemmeno una volta il valore di ritorno di fts_i2c_read_reg: dopo ognuna
 * delle undici "bl fts_i2c_read_reg" il codice fa direttamente
 * "394013e3 ldrb"@0xffffff8008a7c608 e passa il byte a snprintf.  E' un
 * difetto della fabbrica e si riproduce.
 *
 * IL CLIENT VIENE DAL `dev`, non dal contesto globale:
 *   "d1008015 sub"@0xffffff8008a7c5d0   x21 = x0 - 0x20, cioe' to_i2c_client(dev)
 *   (in `struct i2c_client` il campo `dev` sta a +32)
 * mentre il MUTEX viene dal contesto globale:
 *   "f9400508 ldr"@0xffffff8008a7c5e8 + "9107e113 add"@0xffffff8008a7c5ec
 *   -> x19 = fts_data->c8 + 0x1f8
 *
 * L'ACCUMULO DELL'INDICE E' A 32 BIT ED E' ESTESO CON SEGNO A OGNI USO:
 *   "8b36c280 add"@0xffffff8008a7c63c   add x0, x20, w22, sxtw
 * cioe' `buf + count` dove `count` e' un `int`.  Con un `size_t` non ci
 * sarebbe la `sxtw`.
 *
 * IL VALORE DI RITORNO E' LA SOMMA DELLE ULTIME DUE, non l'accumulo:
 *   "2a0003f4 mov"@0xffffff8008a7c7d8   w20 = il risultato dell'undicesima
 *   "0b160288 add"@0xffffff8008a7c7f8   w8 = w20 + w22
 *   "93407d00 sxtw"@0xffffff8008a7c808  esteso con segno
 * cioe' il totale e' `count + snprintf(...)` con `count` che si ferma alla
 * decima.  E' la forma normale di `count += snprintf(...); return count;`.
 */
static ssize_t fts_dumpreg_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct input_dev *input_dev = fts_data->c8;
	int count = 0;
	u8 val = 0;

	mutex_lock(&input_dev->mutex);

	fts_i2c_read_reg(client, FTS_REG_POWER_MODE, &val);
	count = snprintf(buf, PAGE_SIZE, "Power Mode:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_FW_VER, &val);
	count += snprintf(buf + count, PAGE_SIZE, "FW Ver:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_LIC_VER, &val);
	count += snprintf(buf + count, PAGE_SIZE, "LCD Initcode Ver:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_PARAM_VER, &val);
	count += snprintf(buf + count, PAGE_SIZE, "Param Ver:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_PARAM_STATUS, &val);
	count += snprintf(buf + count, PAGE_SIZE, "Param status:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_VENDOR_ID, &val);
	count += snprintf(buf + count, PAGE_SIZE, "Vendor ID:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_LCD_BUSY_NUM, &val);
	count += snprintf(buf + count, PAGE_SIZE, "LCD Busy Number:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_GESTURE_EN, &val);
	count += snprintf(buf + count, PAGE_SIZE, "Gesture Mode:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_CHARGER_MODE_EN, &val);
	count += snprintf(buf + count, PAGE_SIZE, "charge stat:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_INT_CNT, &val);
	count += snprintf(buf + count, PAGE_SIZE, "INT count:0x%02x\n", val);

	fts_i2c_read_reg(client, FTS_REG_ESD_SATURATE, &val);
	count += snprintf(buf + count, PAGE_SIZE, "ESD count:0x%02x\n", val);

	mutex_unlock(&input_dev->mutex);

	return count;
}

static ssize_t fts_dumpreg_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	return -1;
}

static ssize_t fts_fwupgradebin_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	return -1;
}

/*
 * ===========================================================================
 * fts_fwupgradebin_store @ 0xffffff8008a7c828, 276 byte
 * fts_fwforceupg_store   @ 0xffffff8008a7c944, 276 byte
 * ===========================================================================
 * Le due funzioni sono la stessa cosa con DUE differenze misurate: il
 * messaggio e l'ultimo argomento di fts_upgrade_bin.
 *   "2a1f03e2 mov"@0xffffff8008a7c8f0   w2 = 0   (bin)
 *   "320003e2 orr"@0xffffff8008a7ca08   w2 = 1   (force)
 *
 * IL CONTROLLO DELLA LUNGHEZZA e' ancora l'idioma sottrazione+confronto
 * senza segno, ma su SESSANTAQUATTRO bit, cioe' su un `size_t`:
 *   "d1000869 sub"@0xffffff8008a7c848   x9 = count - 2
 *   "f101793f cmp"@0xffffff8008a7c850   cmp x9, #0x5e
 *   "540000e3 b.cc"@0xffffff8008a7c858
 * L'intervallo accettato e' [2, 0x5f] = [2, 95].  `sub x9`/`cmp x9` a 64 bit
 * (e non `sub w9`) dice che `count` e' `size_t` -- classe A4.
 *
 * IL BUFFER E' DI 128 BYTE MA LA snprintf NE DICHIARA 4096.  Il buffer viene
 * azzerato con OTTO `stp xzr,xzr` (0xffffff8008a7c894..0xffffff8008a7c8b0),
 * cioe' 128 byte, e poi:
 *   "321403e1 orr"@0xffffff8008a7c890   w1 = 0x1000 = 4096 = PAGE_SIZE
 * E' un difetto della fabbrica -- la lunghezza passata a snprintf non e'
 * quella del buffer -- e si riproduce (regola 7).  Il controllo di lunghezza
 * a monte lo rende innocuo di fatto, ma nel binario c'e' PAGE_SIZE.
 *
 * L'AZZERAMENTO DELL'ULTIMO BYTE usa `count`, non il risultato della snprintf:
 *   "8b1302e8 add"@0xffffff8008a7c8c0   x8 = fwname + count
 *   "381ff11f sturb"@0xffffff8008a7c8c8 sturb wzr, [x8,#-1]
 * cioe' `fwname[count - 1] = '\0'`.
 */
static ssize_t fts_fwupgradebin_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	char fwname[128] = { 0 };
	struct fts_ts_data *ts_data = fts_data;
	struct i2c_client *client = ts_data->c0;
	struct input_dev *input_dev = ts_data->c8;

	if ((count <= 1) || (count >= 128 - 32)) {
		FTS_ERROR("fw bin name's length(%d) fail", (int)count);
		return -EINVAL;
	}

	/* LA DEVIAZIONE E' DICHIARATA, NON AGGIRATA IN SILENZIO.  Il binario ha
	 * PAGE_SIZE ("321403e1 orr"@0xffffff8008a7c890, w1 = 0x1000) e un buffer
	 * di 128 byte (le otto "stp xzr,xzr" da 0xffffff8008a7c894 a
	 * 0xffffff8008a7c8b0): sono in contraddizione, ed e' un difetto della
	 * fabbrica che la regola 7 dice di riprodurre.  Il nostro clang lo
	 * TROVA -- e' la prova indipendente che il difetto e' reale:
	 *   error: 'snprintf' size argument is too large; destination buffer has
	 *   size 128, but size argument is 4096 [-Werror,-Wfortify-source]
	 * quindi -Werror rifiuta il file.  Scrivere `sizeof(fwname)` lo farebbe
	 * passare e costerebbe ZERO byte -- misurato: entrambe le forme danno
	 * 276 byte, perche' `orr w1,wzr,#0x1000` e `mov w1,#0x80` sono una
	 * istruzione ciascuna -- ma metterebbe nel sorgente un immediato che nel
	 * binario NON c'e', e nessuna misura di dimensione lo denuncerebbe
	 * (e' la classe B8 al livello dell'operando).  Fra le due, tengo il
	 * binario e spengo l'avviso QUI, su due righe, dicendo perche'. */
#pragma clang diagnostic push
/* IL GRUPPO -Wfortify-source NON ESISTE NEL COMPILATORE DI FABBRICA (clang
 * 9.0.3, r353983c): la` la riga sotto e` essa stessa un errore, perche' il
 * kernel porta -Werror=unknown-warning-option di serie.  Spegnere PRIMA
 * quel gruppo rende le due righe innocue per clang 9 e attive per clang 11,
 * senza numeri di versione nel sorgente.  Misurato: senza questa riga
 * `compila-a4.sh fab` dava
 *   error: unknown warning group '-Wfortify-source', ignored
 *          [-Werror,-Wunknown-warning-option]
 * e il file NON si compilava col compilatore che fa da giudice. */
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wfortify-source"
	snprintf(fwname, PAGE_SIZE, "%s", buf);
#pragma clang diagnostic pop
	fwname[count - 1] = '\0';

	FTS_INFO("upgrade with bin file through sysfs node");

	mutex_lock(&input_dev->mutex);
	ts_data->c325 = 1;
	fts_irq_disable();
	fts_upgrade_bin(client, fwname, 0);
	fts_irq_enable();
	ts_data->c325 = 0;
	mutex_unlock(&input_dev->mutex);

	return count;
}

static ssize_t fts_fwforceupg_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	return -1;
}

static ssize_t fts_fwforceupg_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	char fwname[128] = { 0 };
	struct fts_ts_data *ts_data = fts_data;
	struct i2c_client *client = ts_data->c0;
	struct input_dev *input_dev = ts_data->c8;

	if ((count <= 1) || (count >= 128 - 32)) {
		FTS_ERROR("fw bin name's length(%d) fail", (int)count);
		return -EINVAL;
	}

	/* LA DEVIAZIONE E' DICHIARATA, NON AGGIRATA IN SILENZIO.  Il binario ha
	 * PAGE_SIZE ("321403e1 orr"@0xffffff8008a7c890, w1 = 0x1000) e un buffer
	 * di 128 byte (le otto "stp xzr,xzr" da 0xffffff8008a7c894 a
	 * 0xffffff8008a7c8b0): sono in contraddizione, ed e' un difetto della
	 * fabbrica che la regola 7 dice di riprodurre.  Il nostro clang lo
	 * TROVA -- e' la prova indipendente che il difetto e' reale:
	 *   error: 'snprintf' size argument is too large; destination buffer has
	 *   size 128, but size argument is 4096 [-Werror,-Wfortify-source]
	 * quindi -Werror rifiuta il file.  Scrivere `sizeof(fwname)` lo farebbe
	 * passare e costerebbe ZERO byte -- misurato: entrambe le forme danno
	 * 276 byte, perche' `orr w1,wzr,#0x1000` e `mov w1,#0x80` sono una
	 * istruzione ciascuna -- ma metterebbe nel sorgente un immediato che nel
	 * binario NON c'e', e nessuna misura di dimensione lo denuncerebbe
	 * (e' la classe B8 al livello dell'operando).  Fra le due, tengo il
	 * binario e spengo l'avviso QUI, su due righe, dicendo perche'. */
#pragma clang diagnostic push
/* IL GRUPPO -Wfortify-source NON ESISTE NEL COMPILATORE DI FABBRICA (clang
 * 9.0.3, r353983c): la` la riga sotto e` essa stessa un errore, perche' il
 * kernel porta -Werror=unknown-warning-option di serie.  Spegnere PRIMA
 * quel gruppo rende le due righe innocue per clang 9 e attive per clang 11,
 * senza numeri di versione nel sorgente.  Misurato: senza questa riga
 * `compila-a4.sh fab` dava
 *   error: unknown warning group '-Wfortify-source', ignored
 *          [-Werror,-Wunknown-warning-option]
 * e il file NON si compilava col compilatore che fa da giudice. */
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wfortify-source"
	snprintf(fwname, PAGE_SIZE, "%s", buf);
#pragma clang diagnostic pop
	fwname[count - 1] = '\0';

	FTS_INFO("force upgrade through sysfs node");

	mutex_lock(&input_dev->mutex);
	ts_data->c325 = 1;
	fts_irq_disable();
	fts_upgrade_bin(client, fwname, 1);
	fts_irq_enable();
	ts_data->c325 = 0;
	mutex_unlock(&input_dev->mutex);

	return count;
}

/*
 * ===========================================================================
 * fts_driverinfo_show @ 0xffffff8008a7ca58, 244 byte
 * ===========================================================================
 * CINQUE snprintf.  I valori che stampa vengono da quattro posti diversi, e
 * ognuno e' letto e non supposto:
 *   "b942b105 ldr"@0xffffff8008a7caa8   w5 = [0x996b000 + 688] = TPD_RES_X
 *   "b942b926 ldr"@0xffffff8008a7caac   w6 = [0x996b000 + 696] = TPD_RES_Y
 *   "2a1f03e3 mov"@0xffffff8008a7cac4 + "2a1f03e4 mov"@0xffffff8008a7cac8
 *                                        w3 = w4 = 0, i due angoli in alto
 *   "b94f9103 ldr"@0xffffff8008a7cad4   w3 = [0xa0fb000 + 3984] =
 *                                        tpd_dts_data.touch_max_num
 *   "b94142c3 ldr"@0xffffff8008a7caf0   w3 = fts_data->c320, il numero di IRQ
 *   "394082c3 ldrb"@0xffffff8008a7cb0c + "394086c4 ldrb"@0xffffff8008a7cb10
 *                                        w3 = fts_data->c32, w4 = c33
 *
 * TPD_RES_X E TPD_RES_Y SONO LETTE A 32 BIT (`ldr w`), mentre in tpd.h di
 * questo albero sono `unsigned long`, cioe' 64 bit.  Il sorgente le tronca:
 * un argomento variadico a 64 bit avrebbe dato `ldr x5`.  Qui il troncamento
 * e' scritto con un cast esplicito, che e' anche cio' che serve perche' il
 * `%d` non dia un avviso di formato.  E' una LETTURA del binario, non una
 * comodita': se il cast non ci fosse, il codice non sarebbe questo.
 *
 * "irq:%d\n"@0xffffff8009283763 sta FUORI dalla corsa di stringhe del blocco
 * (0x924db25..0x9250380): e' un letterale che il linker ha unito con quello
 * di un'altra unita' del kernel.  Va citato all'indirizzo dove il linker l'ha
 * messo, non a uno immaginario dentro il blocco.
 */
static ssize_t fts_driverinfo_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct fts_ts_data *ts_data = fts_data;
	struct input_dev *input_dev = ts_data->c8;
	int count = 0;

	mutex_lock(&input_dev->mutex);

	count = snprintf(buf, PAGE_SIZE, "Driver Ver:%s\n", FTS_DRIVER_VERSION);

	count += snprintf(buf + count, PAGE_SIZE, "Resolution:(%d,%d)~(%d,%d)\n",
			  0, 0, (int)TPD_RES_X, (int)TPD_RES_Y);

	count += snprintf(buf + count, PAGE_SIZE, "Max Touchs:%d\n",
			  tpd_dts_data.touch_max_num);

	count += snprintf(buf + count, PAGE_SIZE, "irq:%d\n", ts_data->c320);

	count += snprintf(buf + count, PAGE_SIZE, "IC ID:0x%02x%02x\n",
			  ts_data->c32, ts_data->c33);

	mutex_unlock(&input_dev->mutex);

	return count;
}

static ssize_t fts_driverinfo_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return -1;
}

/*
 * ===========================================================================
 * fts_hw_reset_show @ 0xffffff8008a7cb54, 100 byte
 * ===========================================================================
 *   "320003e0 orr"@0xffffff8008a7cb7c   w0 = 1, l'argomento di fts_reset_proc
 *   "9400083d bl"@0xffffff8008a7cb80    fts_reset_proc(1)
 * poi la snprintf ripiegata dal compilatore: 19 byte di "hw reset executed\n"
 * piu' il NUL, in tre store,
 *   "a9402508 ldp"@0xffffff8008a7cb8c   x8,x9 = i primi 16 byte
 *   "528c8caa mov"@0xffffff8008a7cb90 + "72a0014a movk"@0xffffff8008a7cb94
 *                                       w10 = 0x000a6465 = 'e','d','\n','\0'
 *   "a9002668 stp"@0xffffff8008a7cba0   i 16 byte
 *   "b800f26a stur"@0xffffff8008a7cb9c  i 4 byte a buf+15
 * e il valore di ritorno ripiegato a costante:
 *   "52800240 mov"@0xffffff8008a7cbac   w0 = 0x12 = 18 = strlen("hw reset executed\n")
 * Il `mov w0` (32 bit) qui, contro il `mov x0` dei sette stub, non contraddice
 * `ssize_t`: e' un valore positivo noto, e clang lo estende a zero
 * gratuitamente scrivendo il registro a 32 bit.
 */
static ssize_t fts_hw_reset_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct input_dev *input_dev = fts_data->c8;
	ssize_t count = 0;

	mutex_lock(&input_dev->mutex);
	fts_reset_proc(1);
	count = snprintf(buf, PAGE_SIZE, "hw reset executed\n");
	mutex_unlock(&input_dev->mutex);

	return count;
}

static ssize_t fts_hw_reset_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	return -1;
}

static ssize_t fts_irq_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	return -1;
}

/*
 * ===========================================================================
 * fts_irq_store @ 0xffffff8008a7cbc8, 132 byte
 * ===========================================================================
 *   "394002a8 ldrb"@0xffffff8008a7cbf8   w8 = buf[0]   (UN byte)
 *   "7100c11f cmp"@0xffffff8008a7cbfc    == 0x30 = '0'  -> disable
 *   "7100c51f cmp"@0xffffff8008a7cc04    == 0x31 = '1'  -> enable
 * Il valore di ritorno e' il `count` d'ingresso, non toccato:
 *   "aa0303f3 mov"@0xffffff8008a7cbe0 (x19 = x3) e
 *   "aa1303e0 mov"@0xffffff8008a7cc38 (x0 = x19).
 * Il buffer NON e' letto oltre il primo byte, e non c'e' nessun controllo su
 * `count`: con `count == 0` la funzione legge comunque buf[0].  E' un difetto
 * della fabbrica e si riproduce.
 */
static ssize_t fts_irq_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct input_dev *input_dev = fts_data->c8;

	mutex_lock(&input_dev->mutex);

	if (buf[0] == '1') {
		FTS_INFO("[EX-FUN]enable irq");
		fts_irq_enable();
	} else if (buf[0] == '0') {
		FTS_INFO("[EX-FUN]disable irq");
		fts_irq_disable();
	}

	mutex_unlock(&input_dev->mutex);

	return count;
}

/*
 * ===========================================================================
 * LA MISURA, E L'ATTRIBUZIONE DI OGNI DIVERGENZA
 * ===========================================================================
 * DAL `.o` VERO, non da misuraisolata.py, E CON TUTTI E DUE I COMPILATORI.
 * Il giudice e' quello di FABBRICA: clang r353983c (LLVM 9.0.3), lo stesso
 * con cui e' compilato il kernel dell'apparecchio.
 *
 *   $ ./compila-a4.sh fab werror          # tutta la directory, sei obj-y
 *     CC drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_core.o
 *     CC drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_i2c.o
 *     CC drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_ex_mode.o
 *     CC drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_ft8719_upgrade.o
 *     CC drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_sysfs.o
 *     AR drivers/input/touchscreen/mediatek/focaltech_ft8719/built-in.o
 *   $ ./misura-a4.sh fab
 *   0000000000000108 00000000000001a0 t fts_debug_read
 *   00000000000002a8 0000000000000258 t fts_debug_write
 *   00000000000005f8 00000000000002f4 t fts_tprwreg_show
 *   00000000000008ec 00000000000006c8 t fts_tprwreg_store
 *   [... le altre diciotto, invariate ...]
 *
 *   funzione                        fabbrica  fab(9.0.3)  nostro(11.0.1)
 *   ------------------------------  --------  ----------  --------------
 *   fts_create_apk_debug_channel         100         100             100
 *   fts_release_apk_debug_channel         28          28              28
 *   fts_create_sysfs                     100         100             100
 *   fts_remove_sysfs                      36          36              36
 *   fts_debug_read                       416         416             408
 *   fts_debug_write                      600         600             608
 *   fts_tpfwver_show                     240         240             240
 *   fts_tpfwver_store                      8           8               8
 *   fts_tprwreg_show                     756         756             756
 *   fts_tprwreg_store                   1736        1736            1732
 *   fts_dumpreg_show                     612         612             612
 *   fts_dumpreg_store                      8           8               8
 *   fts_fwupgradebin_show                  8           8               8
 *   fts_fwupgradebin_store               276         276             276
 *   fts_fwforceupg_show                    8           8               8
 *   fts_fwforceupg_store                 276         276             276
 *   fts_driverinfo_show                  244         244             244
 *   fts_driverinfo_store                   8           8               8
 *   fts_hw_reset_show                    100         100             100
 *   fts_hw_reset_store                     8           8               8
 *   fts_irq_show                           8           8               8
 *   fts_irq_store                        132         132             132
 *   ------------------------------  --------  ----------  --------------
 *   totale (22 su 22)                   5708        5708            5704
 *
 * COL COMPILATORE DI FABBRICA: 22 SU 22 ESATTE, e l'unita' e' CHIUSA -- la
 * somma fa 5708 = 0xffffff8008a7cc4c - 0xffffff8008a7b600, esattamente
 * l'intervallo dell'unita' A.
 * COL NOSTRO: 19 su 22 (fts_debug_read -8, fts_debug_write +8,
 * fts_tprwreg_store -4).
 *
 * L'AVVERTENZA PRIMA DEL NUMERO (classe C4):
 *   $ ./venv/bin/python3 intervallo.py 22 22
 *   22 su 22 = 100.0%   IC95% Clopper-Pearson [84.6%; 100.0%]
 *   contiene 77.10%: NO -- la misura discrimina
 *   $ ./venv/bin/python3 intervallo.py 19 22
 *   19 su 22 = 86.4%   IC95% Clopper-Pearson [65.1%; 97.1%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *   $ ./venv/bin/python3 intervallo.py 4 4
 *   4 su 4 = 100.0%   IC95% Clopper-Pearson [39.8%; 100.0%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 * Cioe': sul lotto A4 da solo (quattro funzioni) la misura NON discrimina in
 * nessuna direzione; sull'unita' intera col compilatore giusto discrimina
 * sopra al ramo; col compilatore sbagliato NON discrimina.  I due intervalli
 * col compilatore giusto e con quello sbagliato si sovrappongono: la
 * differenza fra i due compilatori si VEDE nei numeri (22 contro 19) ma su
 * ventidue unita' non e' statisticamente separata.
 *
 * IL CONFRONTO PER CODIFICA, che conta piu' della dimensione (una dimensione
 * uguale non prova che il codice sia lo stesso), sulle QUATTRO nuove e col
 * compilatore di fabbrica:
 *
 *   $ ./cod-a4.sh fts_debug_read 0xffffff8008a7b708 fab
 *   istruzioni: fabbrica 104, nostro 104
 *   stesso mnemonico in posizione: 104 su 104
 *   codifiche diverse: 20
 *     di cui siti di rilocazione o chiamata locale: 20
 *     RESIDUO: 0
 *
 *   funzione            istr F=N  mnem. uguali  cod. diverse  reloc  RESIDUO
 *   ------------------  --------  ------------  ------------  -----  -------
 *   fts_debug_read           104       104/104            20     20        0
 *   fts_debug_write          150       150/150            35     35        0
 *   fts_tprwreg_show         189       185/189            79     75        4
 *   fts_tprwreg_store        434       270/434           238    134      104
 *   ------------------  --------  ------------  ------------  -----  -------
 *   totale                   877       709/877           372    264      108
 *
 * Col confronto STRETTO (rev_codifica.py, che maschera il SOLO campo rilocato
 * invece di tutta la parola) i primi due restano a ZERO, fts_tprwreg_show
 * resta a 4 e fts_tprwreg_store sale a 180.
 *
 * `.data` E `.rodata` COMBACIANO BYTE PER BYTE.
 *   $ aarch64-linux-android-objdump -h focaltech_sysfs.o
 *     1 .data         00000170   ...
 *     2 .bss          00000000   ...
 *   0x170 = 368 = 0xffffff800998c8c0 - 0xffffff800998c750, la corsa di
 *   `.data` dell'unita'.  Il contenuto, confrontato con l'immagine:
 *     nostro   0070 00000000 00000000 a4010000 00000000
 *     fabbrica ffffff800998c7c0 00000000 00000000 a4010000 00000000
 *   e cosi' per tutte e otto, agli stessi scostamenti relativi
 *   (0x78, 0x98, 0xb8, 0xd8, 0xf8, 0x118, 0x138, 0x158 contro
 *    0x998c7c8, 7e8, 808, 828, 848, 868, 888, 8a8 meno 0x998c750).
 *   Tutto il resto e' zero da tutte e due le parti, perche' i puntatori
 *   stanno nelle rilocazioni.
 *   `.bss` NON e' piu' vuota: il lotto A4 ci ha messo i sei simboli di stato
 *   di fts_tprwreg_show/store.  Di fabbrica occupa 48 byte
 *   (0xffffff800a100a98..0xffffff800a100ac8) con i sei oggetti distanziati di
 *   OTTO byte l'uno dall'altro; dichiarati come sono qui ne occupano 32.  E'
 *   la DIVERGENZA 6, costa zero byte di testo e nessuna misura di dimensione
 *   la denuncerebbe.
 *
 * ===========================================================================
 * LE DIVERGENZE
 * ===========================================================================
 * DIVERGENZA 1 -- CHIUSA dal lotto A4.  Le quattro funzioni erano dichiarate
 *   non-`static` perche' non erano definite; ora sono definite e sono
 *   `static`, come nella mappa di fabbrica.
 *
 * DIVERGENZA 2 -- `snprintf(fwname, PAGE_SIZE, ...)` in fts_fwupgradebin_store
 *   e fts_fwforceupg_store, ZERO byte, ma con un avviso spento a mano.
 *   Il commento accanto alle due chiamate porta la motivazione e la misura.
 *
 * DIVERGENZA 3 -- l'ALIAS scelto per l'immediato 0x1000, ZERO byte.
 *   La fabbrica scrive "321403e1 orr"@0xffffff8008a7c890 (`orr w1, wzr, #0x1000`),
 *   il nostro clang scrive `mov w1, #0x1000` (codifica 52820001).  Sono la
 *   stessa istruzione con due codifiche diverse: entrambe 4 byte, entrambe
 *   mettono 4096 in w1.  E' la scelta di alias di clang 11 contro clang 9.
 *   Compare in ognuna delle diciassette `orr wN, wzr, #imm` dell'unita' e non
 *   sposta un byte, ma va detta: chi confrontasse le CODIFICHE invece delle
 *   dimensioni la troverebbe diciassette volte.
 *
 * DIVERGENZA 4 -- fts_tprwreg_show, QUATTRO posizioni su 189, ZERO byte.
 *   Sono DUE COPPIE DI ISTRUZIONI ADIACENTI SCAMBIATE FRA LORO, e nient'altro:
 *     fabbrica "110042d5 add"@0xffffff8008a7be0c poi "3900401f strb"@0xffffff8008a7be10
 *     nostro   le stesse due nell'ordine opposto
 *     fabbrica "79000008 strh"@0xffffff8008a7bee0 poi "110006b5 add"@0xffffff8008a7bee4
 *     nostro   le stesse due nell'ordine opposto
 *   Le due istruzioni di ogni coppia non dipendono l'una dall'altra (una
 *   scrive in memoria, l'altra incrementa il contatore in un registro), e la
 *   fabbrica stessa le emette nei DUE ordini in due punti diversi della
 *   stessa funzione: e' l'ordinatore delle istruzioni, non il sorgente.
 *   Non ho trovato nessuna forma di sorgente che sposti l'ordine, e lo
 *   dichiaro come non risolto invece di far finta che non ci sia.
 *
 * DIVERGENZA 5 -- fts_tprwreg_store, 104 posizioni su 434, ZERO byte.
 *   La dimensione e' esatta (1736) e il NUMERO di istruzioni e' lo stesso
 *   (434 contro 434); cio' che differisce e' la DISPOSIZIONE dei blocchi
 *   nella seconda meta' della funzione.  La fabbrica mette per PRIMO il ramo
 *   di scrittura e per secondo quello di lettura
 *   ("36000108 tbz"@0xffffff8008a7c1bc salta AL ramo di lettura), noi il
 *   contrario; e la fabbrica issa "7100049f cmp"@0xffffff8008a7c1b8 sopra la
 *   tbz, usando le stesse bandierine nei due rami, mentre noi ripetiamo il
 *   confronto in ognuno.  Ho provato a scambiare i due rami nel sorgente
 *   (`if ((a98 & 0x01) == 0)` con la lettura per prima): l'uscita e'
 *   IDENTICA, byte per byte, perche' clang canonicalizza la condizione.  E'
 *   una scelta dell'ordinatore di blocchi che il sorgente non governa.
 *   Restano inoltre due differenze locali che si compensano nel conto:
 *     - la fabbrica materializza `buf + 3` in un registro
 *       ("91000eda add"@0xffffff8008a7c318) e lo riusa come base del ciclo;
 *       noi indirizziamo da `buf` con l'indice che parte da 5;
 *     - il nostro `regaddr` ha l'indirizzo preso (lo passiamo a fts_i2c_read)
 *       e quindi vive sulla pila: una `strb` in piu' che la fabbrica non ha.
 *   Una in piu' e una in meno: il totale resta 434.
 *
 * DIVERGENZA 6 -- la spaziatura dei sei simboli `.bss`, ZERO byte di testo.
 *   Di fabbrica cominciano a 0xffffff800a100a98, aa0, aa8, ab0, ab8, ac0,
 *   cioe' ognuno a un multiplo di otto, per 48 byte in tutto.  Un `u8`
 *   seguito da quattro `int` e da un puntatore, dichiarati cosi', ne occupa
 *   32.  Gli spiazzamenti stanno tutti in campi rilocati, quindi non entrano
 *   nel confronto per codifica ne' nella dimensione: e' proprio il tipo di
 *   divergenza che nessuna misura denuncia (classe B), e per questo e'
 *   scritta qui.
 *
 * NESSUN RESIDUO INSPIEGATO in fts_debug_read e fts_debug_write: 104 su 104
 * e 150 su 150, residuo ZERO anche col confronto stretto.
 *
 * LE TRE DIVERGENZE CHE C'ERANO E SONO STATE CHIUSE, tutte della stessa
 * classe (A5: un valore che il binario tiene in un registro salvato
 * attraverso le chiamate, cioe' un LOCALE nel sorgente; scritto come accesso
 * ripetuto, clang lo ricarica):
 *
 *   funzione                  prima  dopo  fabbrica  che cosa mancava
 *   ------------------------  -----  ----  --------  --------------------------
 *   fts_tpfwver_show            244   240       240  `struct input_dev *` locale
 *   fts_dumpreg_show            616   612       612  `struct input_dev *` locale
 *   fts_driverinfo_show         252   244       244  `struct fts_ts_data *` locale
 *   fts_fwupgradebin_store      284   276       276  `struct fts_ts_data *` locale
 *   fts_fwforceupg_store        284   276       276  `struct fts_ts_data *` locale
 *
 *   La prova sta nel binario: la fabbrica carica il puntatore UNA volta
 *   ("f9456d16 ldr"@0xffffff8008a7c878 mette fts_data in x22) e poi lo riusa
 *   attraverso mutex_lock, fts_irq_disable, fts_upgrade_bin e fts_irq_enable
 *   ("390516c8 strb"@0xffffff8008a7c8e0 e "390516df strb"@0xffffff8008a7c900
 *   scrivono a [x22,#325] DOPO tre chiamate).  Un `extern struct fts_ts_data *`
 *   riletto a ogni uso non potrebbe sopravvivere a quelle chiamate, e infatti
 *   il nostro `.o` prima della correzione aveva due `ldr x8,[x22]` in piu'.
 *   Due istruzioni x 4 byte = gli 8 byte di scarto.
 *
 * ===========================================================================
 * DELTA DI FILE CONDIVISO -- DESCRITTO, NON FATTO
 * ===========================================================================
 * 1. `struct fts_ts_data` e' definita in focaltech_core.c e ridichiarata qui
 *    con i soli campi che questa unita' misura.  Le due definizioni devono
 *    restare compatibili sugli offset, e oggi lo sono perche' entrambe
 *    arrivano a 384 byte.  La sede giusta sarebbe un header comune
 *    (focaltech_core.h), che NON ho creato: sarebbe un file nuovo condiviso
 *    fra il mio lotto e quello dell'unita' C.
 *    Nel farlo, due campi vanno ESPOSTI: +368 (`struct proc_dir_entry *`) e
 *    +376 (u8), che in focaltech_core.c cadono dentro `u8 c364[20]` perche'
 *    l'unita' C non li tocca.
 * 2. Il Makefile del padre (drivers/input/touchscreen/mediatek/Makefile) non
 *    ha ancora `obj-$(CONFIG_WTK_FT8719_E977) += focaltech_ft8719/` e il suo
 *    Kconfig non ha il `source` della directory.  E' lo stesso delta gia'
 *    registrato dai cappelli di focaltech_core.c e focaltech_i2c.c: lo
 *    ripeto perche' una correzione che non raggiunge tutte le copie e' la
 *    classe C1.
 * 3. `TPD_RES_X`/`TPD_RES_Y` sono `unsigned long` in tpd.h, e il binario le
 *    legge a 32 bit.  Qui il troncamento e' un cast esplicito nel sorgente.
 *    Se un giorno si scoprisse che la tpd.h di FABBRICA le dichiarava `int`,
 *    il cast andrebbe tolto -- e la dimensione non se ne accorgerebbe, perche'
 *    `ldr w5` e' `ldr w5` in tutti e due i casi.  Dichiarato come tale.
 */

/*
 * ===========================================================================
 * OGNI LETTERALE DELLE 18 FUNZIONI SCRITTE, LETTO PER INTERO DAL BINARIO
 * ===========================================================================
 * Il livello KERN e il prefisso fanno parte del letterale nell'immagine: e'
 * questo che dice, senza supporre niente, QUALE macro il sorgente usa in ogni
 * punto -- e che distingue i messaggi (con prefisso) dai testi versati nel
 * buffer sysfs (senza).
 *
 *   macro       letterale
 *   ----------  --------------------------------------------------------------
 *   -           "ftxxxx-debug"@0xffffff800924db25       il nome del nodo /proc
 *   FTS_ERROR   "\x013[FTS][Error]create proc entry fail\n"@0xffffff800924db32
 *   FTS_INFO    "\x016[FTS][Info]Create proc entry success!\n"@0xffffff800924db58
 *   FTS_ERROR   "\x013[FTS][Error][EX]: sysfs_create_group() failed!!\n"@0xffffff800924db81
 *   FTS_INFO    "\x016[FTS][Info][EX]: sysfs_create_group() succeeded!!\n"@0xffffff800924dbb4
 *   sprintf     "I2c transfer error!\n"@0xffffff800924dd57
 *   snprintf    "get tp fw version fail!\n"@0xffffff800924dd6c
 *   snprintf    "%02x\n"@0xffffff80091b65a4
 *   snprintf    "Power Mode:0x%02x\n"@0xffffff800924e01c
 *   snprintf    "FW Ver:0x%02x\n"@0xffffff800924e02f
 *   snprintf    "LCD Initcode Ver:0x%02x\n"@0xffffff800924e03e
 *   snprintf    "Param Ver:0x%02x\n"@0xffffff800924e057
 *   snprintf    "Param status:0x%02x\n"@0xffffff800924e069
 *   snprintf    "Vendor ID:0x%02x\n"@0xffffff800924e07e
 *   snprintf    "LCD Busy Number:0x%02x\n"@0xffffff800924e090
 *   snprintf    "Gesture Mode:0x%02x\n"@0xffffff800924e0a8
 *   snprintf    "charge stat:0x%02x\n"@0xffffff800924e0bd
 *   snprintf    "INT count:0x%02x\n"@0xffffff800924e0d1
 *   snprintf    "ESD count:0x%02x\n"@0xffffff800924e0e3
 *   FTS_ERROR   "\x013[FTS][Error]fw bin name's length(%d) fail\n"@0xffffff800924e105
 *   snprintf    "%s"@0xffffff8009100e35
 *   FTS_INFO    "\x016[FTS][Info]upgrade with bin file through sysfs node\n"@0xffffff800924e132
 *   FTS_INFO    "\x016[FTS][Info]force upgrade through sysfs node\n"@0xffffff800924e17b
 *   snprintf    "Driver Ver:%s\n"@0xffffff800924e1ba
 *   -           "Focaltech V2.2 20180321"@0xffffff800924e1c9
 *   snprintf    "Resolution:(%d,%d)~(%d,%d)\n"@0xffffff800924e1e1
 *   snprintf    "Max Touchs:%d\n"@0xffffff800924e1fd
 *   snprintf    "irq:%d\n"@0xffffff8009283763
 *   snprintf    "IC ID:0x%02x%02x\n"@0xffffff800924e20c
 *   snprintf    "hw reset executed\n"@0xffffff800924e22b
 *   FTS_INFO    "\x016[FTS][Info][EX-FUN]enable irq\n"@0xffffff800924e246
 *   FTS_INFO    "\x016[FTS][Info][EX-FUN]disable irq\n"@0xffffff800924e267
 *
 * TRE OSSERVAZIONI CHE NON SI VEDONO DALLA TABELLA.
 *
 * 1. "%02x\n"@0xffffff80091b65a4, "%s"@0xffffff8009100e35 e
 *    "irq:%d\n"@0xffffff8009283763 stanno FUORI dalla corsa di stringhe del
 *    blocco (0x924db25..0x9250380): sono letterali cortissimi che il linker
 *    ha unito con quelli di altre unita' del kernel.  Vanno citati
 *    all'indirizzo dove il linker li ha messi.
 *    SONO LORO TRE le `DEBOLE` di verificacitazioni.py, e lo strumento ha
 *    ragione a segnarle -- il numero di occorrenze e' la misura di quanto
 *    valgono da sole:
 *      "%s"@0xffffff8009100e35        297 occorrenze nell'immagine
 *      "%02x\n"@0xffffff80091b65a4     93 occorrenze
 *      "irq:%d\n"@0xffffff8009283763    5 occorrenze
 *    Cio' che le ancora e' l'istruzione che le materializza, e per ognuna sta
 *    dentro una funzione di questa unita':
 *      "f0004022 adrp"@0xffffff8008a7caf8 + "911d8c42 add"@0xffffff8008a7cb00
 *        -> x2 = 0x9283000 + 0x763 = "irq:%d\n"     (fts_driverinfo_show)
 *      "f00039c2 adrp"@0xffffff8008a7bba0 + "91169042 add"@0xffffff8008a7bba4
 *        -> x2 = 0x91b6000 + 0x5a4 = "%02x\n"       (fts_tpfwver_show)
 *      "90003422 adrp"@0xffffff8008a7c880 + "9138d442 add"@0xffffff8008a7c884
 *        -> x2 = 0x9100000 + 0xe35 = "%s"           (fts_fwupgradebin_store)
 *    La coppia (letterale, istruzione che lo materializza) e' una prova;
 *    il letterale da solo no.
 *
 * 2. "[EX]" e "[EX-FUN]" e "[Mode]" (quest'ultimo nell'unita' D) sono PARTE
 *    DEL MESSAGGIO e non del prefisso: compaiono DOPO "[FTS][Error]" o
 *    "[FTS][Info]" dentro lo stesso letterale.  Il binario lo dice da solo, e
 *    e' il motivo per cui le macro di questo file hanno tre prefissi e non
 *    cinque.
 *
 * 3. "I2c transfer error!\n"@0xffffff800924dd57 e
 *    "get tp fw version fail!\n"@0xffffff800924dd6c e
 *    "hw reset executed\n"@0xffffff800924e22b non compaiono MAI come argomento
 *    di una `bl printk`: il compilatore ha ripiegato le loro snprintf in copie
 *    di byte.  Si leggono dalle `ldp`/`ldur` che li caricano
 *    ("a9402908 ldp"@0xffffff8008a7bb60, "a940a909 ldp"@0xffffff8008a7bb84,
 *     "a9402508 ldp"@0xffffff8008a7cb8c), non dal formato di una printk.
 *
 * ===========================================================================
 * LE CITAZIONI CHE `verificacitazioni.py` SEGNA `NON_ANCORATA`, E PERCHE'
 * ===========================================================================
 * QUATTORDICI in tutto, di TRE classi, tutte gia' note al progetto e nessuna
 * un errore:
 * i byte CI SONO, a quegli indirizzi, NUL-terminati -- il controllo di byte lo
 * prova; e' l'ANCORAGGIO a un letterale del codice che manca.
 *
 * 1. LA STRINGIFICAZIONE DI UNA MACRO -- gli OTTO nomi sysfs.
 *      "fts_fw_version"@0xffffff800924dd48   "fts_rw_reg"@0xffffff800924dd85
 *      "fts_dump_reg"@0xffffff800924e00f     "fts_upgrade_bin"@0xffffff800924e0f5
 *      "fts_force_upgrade"@0xffffff800924e169 "fts_driver_info"@0xffffff800924e1aa
 *      "fts_hw_reset"@0xffffff800924e21e     "fts_irq"@0xffffff800924e23e
 *    Nel sorgente NON sono letterali: sono il primo argomento di DEVICE_ATTR,
 *    che la macro stringa con `#`.  Lo strumento conosce il dominio __func__
 *    ma non quello di `#`.  E' la stessa classe della citazione
 *    "&ts_data->report_mutex"@0xffffff800924f7d5 gia' registrata in coda a
 *    focaltech_core.c.  Anzi: proprio perche' il binario porta questi otto
 *    nomi, i nomi delle otto variabili NON sono inventati (regola 5,
 *    l'eccezione della macro che stringa il proprio argomento).
 *    La NONA di questa classe e' proprio
 *    "&ts_data->report_mutex"@0xffffff800924f7d5, che questo file cita per
 *    richiamare il precedente: e' `#lock` dentro mutex_init, non un letterale.
 *
 * 2. UNA MACRO DI LOG SENZA LIVELLO KERN -- CINQUE, tutte di
 *    fts_tprwreg_store:
 *      "[FTS]cmd len: %d, buf: %s\n"@0xffffff800924de6e
 *      "[FTS]read %02X, %d bytes\n"@0xffffff800924df76
 *      "[FTS]write %02X, %d bytes\n"@0xffffff800924dfa3
 *      "[FTS]write buffer: \n"@0xffffff800924dfe6
 *      "[FTS]buf[%d]: %02X\n"@0xffffff800924dffb
 *    Lo strumento riconosce un messaggio assemblato dalla testa `\x01` +
 *    cifra di livello (RE_TESTA_ASSEMBLATA); FTS_DEBUG non emette nessun
 *    livello, perche' il binario non ne porta.  E' un DELTA DI STRUMENTO
 *    gia' registrato in coda a focaltech_core.c, e non l'ho fatto io:
 *    verificacitazioni.py e' condiviso.  I byte CI SONO a quegli indirizzi,
 *    e il confronto di byte lo prova.
 *
 * 3. LA QUARTA `DEBOLE` E' UN SUFFISSO, NON UN LETTERALE A SE'.
 *      "failed, ret: %d\n"@0xffffff800924de4c
 *    e' la CODA di "Result: failed, ret: %d\n"@0xffffff800924de44, otto byte
 *    piu' avanti nella stessa stringa: il linker ha sovrapposto un letterale
 *    al suffisso di un altro.  Il binario USA tutti e due gli indirizzi, in
 *    due rami diversi di fts_tprwreg_show
 *    ("940fba12 bl"@0xffffff8008a7bd78 contro "940fb9f8 bl"@0xffffff8008a7bde0),
 *    quindi la sovrapposizione non e' un errore di lettura: e' cio' che il
 *    linker ha fatto, e le due citazioni sono tutte e due giuste.
 */
