// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_FT8719_E977 -- FocalTech FT8719, Doogee S88 Pro (MT6771).
 * L'UNITA' DI TRADUZIONE DEL CORE, e SOLO quella.  Lotto PARZIALE dichiarato.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA.  Nessun sorgente
 * pubblico e' stato letto.  In particolare l'albero ALPS su cui questo file
 * compila CONTIENE un driver FocalTech pubblico,
 * drivers/input/touchscreen/mediatek/focaltech_fhd_touch/focaltech_core.c:
 * NON e' stato aperto.  Se un lotto futuro lo aprisse dovrebbe dichiararlo e
 * riverificare ogni riga contro il binario.
 *
 * ===========================================================================
 * 1. CHE COSA C'E' QUI DENTRO, E QUAL E' IL CONFINE
 * ===========================================================================
 * Il blocco WTK_FT8719_E977 e' 67 funzioni nella mappa + 1 in `.exit.text`
 * che la mappa non vede = 68 funzioni, 22092 byte, in SEI unita' di
 * traduzione (partizione misurata da testo/.data/.bss, ricognizione
 * /mnt/s88pro/kernel-stock/scout-ft8719/RIASSUNTO.md §5).
 *
 * CORREZIONE del 2026-08-21 (revisione indipendente).  Fin qui questo
 * cappello -- e il rapporto, e il mandato -- scrivevano «68, 22012 byte»: il
 * conteggio delle funzioni era 68 ma i byte erano quelli di SESSANTASETTE.
 *   21844 byte = 0xa80b54 - 0xa7b600, le 66 funzioni di `.text`
 *     + 168 byte = tpd_driver_init, `.init.text`   -> 22012 su 67 funzioni
 *     +  80 byte = tpd_driver_exit, `.exit.text`   -> 22092 su 68 funzioni
 *   $ python3 -c "print(0x80b54 - 0x7b600, 21844+168, 21844+168+80)"
 *   21844 22012 22092
 * La 68esima sfuggiva al totale perche' `oracolo/stock.map` finisce a
 * `_einittext` (0xffffff80093a8518) e nessuno strumento che parta dalla
 * mappa vede la `.exit.text`.  Il conteggio delle funzioni della mappa si
 * riproduce cosi':
 *   $ wc -l scout-ft8719/blocco.map
 *   67 scout-ft8719/blocco.map
 *
 * Questo file e' l'unita' C, cioe' 0xffffff8008a7eac0..0xffffff8008a8020c
 * (14 funzioni, 5964 byte) + `tpd_driver_init` in `.init.text`
 * (0xffffff8009383470, 168 byte) + `tpd_driver_exit` in `.exit.text`
 * (0xffffff80093acb2c, 80 byte) = 16 funzioni, 6212 byte.
 *
 * Piu' CINQUE funzioni che nel binario non hanno simbolo perche' il
 * compilatore le ha incorporate nel chiamante, e che qui sono scritte come
 * funzioni vere.  Sono la classe di difetto B5.  DUE le denuncia il
 * `__func__` che stampano, che non e' quello del chiamante:
 *   - `fts_input_init`   (dentro `tpd_probe`;  "fts_input_init"@0xffffff800924f9d4)
 *   - `fts_release_all_finger` (dentro `tpd_resume`; "fts_release_all_finger"@0xffffff800924fe55)
 * le altre TRE non stampano nessun `__func__` e sono denunciate da un locale
 * doppio o da un ricarico che un corpo solo non avrebbe -- l'argomento sta
 * sopra ognuna, alla sua definizione:
 *   - `fts_read_parse_touchdata` (dentro `touch_event_handler`)
 *   - `fts_input_report_b`       (dentro `touch_event_handler`)
 *   - `fts_get_ic_information`   (dentro `tpd_probe`)
 *
 * CORREZIONE del 2026-08-21: questo capoverso diceva DUE, il §5 diceva TRE e
 * il rapporto CINQUE, per la stessa quantita' (classe C1: la correzione non
 * aveva raggiunto tutte le copie).  Il numero e' CINQUE, e si misura: il
 * file definisce 21 funzioni, il `.o` ne porta 16 con simbolo.
 *   $ aarch64-linux-android-nm --print-size focaltech_core.o | grep -c " [tT] "
 *   17
 * dove 17 = le 16 funzioni piu' `__initcall_tpd_driver_init6`, che e' un
 * dato di 8 byte in `.initcall6.init` e non una funzione.
 *
 * Che questo sia UN file oggetto e non un pezzo di un altro lo provano i
 * numeri di riga che `FTS_FUNC_EXIT` lascia nel binario: 270, 290, 314, 337,
 * 451, 845, 852, 892, 898, 1089, 1104, 1160, 1182, 1207, 1275, 1341, 1381,
 * 1396 stanno tutti in una sola numerazione crescente e coerente, mentre
 * `fts_i2c_init`/`fts_i2c_exit` (unita' E) stampano 370 e 386, che in quella
 * numerazione cadrebbero dentro `fts_release_all_finger`.
 *
 * ===========================================================================
 * 2. COME SI RIVERIFICA
 * ===========================================================================
 * Sul PC di build, in /mnt/s88pro/kernel-stock:
 *
 *   ./venv/bin/python3 verificacitazioni.py focaltech_core.c oracolo/stock.elf \
 *       --eccezione "[FTS]" --eccezione "[FTS][Info]" \
 *       --eccezione "[FTS][Error]" --eccezione "\n" \
 *       --eccezione "GPL" --eccezione "unknown"
 *
 * Esito atteso, al 2026-08-21:
 *   letterali: 75   citati: 73   verificati: 59   probanti: 51   deboli: 8
 *   di cui verificate come messaggio assemblato dalla macro di log: 35
 *   di cui verificate come nome di funzione da __func__: 3
 *   soglia imposta: 69 citati richiesti (75 letterali - 6 eccezioni)
 *   e QUATTORDICI `NON_ANCORATA`, tutte di due classi dichiarate: vedi il
 *   blocco in coda al file.  Lo strumento esce con 1, ed e' giusto cosi':
 *   una NON_ANCORATA e' una divergenza, non un "non trovato" da ignorare.
 *
 * Le sei `--eccezione` sono i letterali delle MACRO, non messaggi: "[FTS]",
 * "[FTS][Info]", "[FTS][Error]" e "\n" sono i pezzi con cui le macro
 * assemblano i messaggi e da soli non esistono NUL-terminati nel binario;
 * "GPL" e' l'argomento di MODULE_LICENSE (finisce in .modinfo, che
 * l'immagine di fabbrica non porta); "unknown" e' costruita a registri da
 * tpd_probe -- "528eede8 mov"+"528dcea9 mov"+"72a00dc8 movk"+"72aded69 movk"
 * @0xffffff8008a7f9e8/ec/f0/f4, cioe' DUE `mov` e DUE `movk`, versati con
 * "b801b3e8 stur"+"b9001be9 str"@0xffffff8008a7f9fc/fc a sp+27 e sp+24
 * (i due store si sovrappongono: 8 byte scritti per 7 di "unknown\0") --
 * quindi non ha un indirizzo in .rodata da citare.
 * CORREZIONE del 2026-08-21: qui c'era scritto «e i tre movk che seguono».
 * I movk che seguono sono DUE.  La frase era stata copiata da quella di
 * tpd_i2c_detect, dove i movk sono davvero tre
 * ("d28e8da8 mov"+"f2a5ad68 movk"+"f2ce0e88 movk"+"f2e00c88 movk"
 * @0xffffff8008a7fb2c/30/34/38), senza ricontarli.
 *
 *   ./venv/bin/python3 verificaistruzioni.py focaltech_core.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a7eac0:0xffffff8008a8020c \
 *       --intervallo 0xffffff8009383470:0xffffff8009383518 \
 *       --intervallo 0xffffff80093acb2c:0xffffff80093acb7c
 *
 * Esito atteso, al 2026-08-21:
 *   citazioni di istruzione trovate nel sorgente: 240 (240 a codifica)
 *   confermate: 240   assenti: 0   mnemonico diverso: 0   controfattuali: 0
 *   operandi -- registro diverso: 0   immediato diverso: 0
 *
 * Nessuna `--controfattuale`: questo file non cita nessuna istruzione che
 * NON debba stare nel binario.
 *
 * I tre intervalli sono le tre sezioni in cui l'unita' e' finita: `.text`,
 * `.init.text`, `.exit.text`.  Il terzo NON e' raggiungibile da nessuno
 * strumento che parta da `oracolo/stock.map`: la mappa finisce a
 * `_einittext = 0xffffff80093a8518`.
 *
 * La misura di dimensione, dal `.o` vero (non da misuraisolata.py):
 *
 *   make -C albero-ft8719 O=out-ft8719 ARCH=arm64 -j6 CC=clang HOSTCC=clang \
 *        CLANG_TRIPLE=aarch64-linux-gnu- CROSS_COMPILE=aarch64-linux-android- \
 *        drivers/input/touchscreen/mediatek/focaltech_ft8719/
 *   aarch64-linux-android-nm --print-size \
 *        out-ft8719/drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_core.o
 *
 * ===========================================================================
 * 3. LA FORMA DELLE CITAZIONI
 * ===========================================================================
 * istruzione:  "<codifica8hex> <mnemonico>"@0xINDIRIZZO
 * letterale:   "testo"@0xINDIRIZZO
 * Sono le due forme che verificaistruzioni.py e verificacitazioni.py sanno
 * confrontare col binario di fabbrica.  Un numero di riga dentro un listato
 * effimero NON e' una prova (classe B7).
 *
 * ATTENZIONE nel leggere gli scarti: il kernel di fabbrica e' costruito con
 * CONFIG_RELOCATABLE=y e i nostri indirizzi non sono i suoi.  OGNI `adrp`,
 * ogni `add #lo12`, ogni `ldr [xN,#pageoff]` che materializza un simbolo ha
 * per forza una codifica diversa dalla nostra.  Le citazioni qui sotto sono
 * quelle di FABBRICA (e' quello che gli strumenti confrontano); la misura che
 * discrimina il nostro lavoro e' la DIMENSIONE, non la codifica.
 *
 * ===========================================================================
 * 4. LE DIVERGENZE APERTE
 * ===========================================================================
 * Sei, tutte elencate e attribuite nel blocco in CODA a questo file, con
 * l'istogramma dei mnemonici e -- per due di esse -- con l'esperimento che le
 * prova.  In sintesi:
 *   1. get_module_id, +68 byte: srotolamento di un ciclo (PROVATA).
 *   2. tpd_probe, +64 byte = +148 di srotolamento (PROVATA) e -84 di
 *      fusione delle code e disposizione della cornice.
 *   3. touch_event_handler, -104 byte: fusione di una coda, allocazione dei
 *      registri, materializzazione degli indirizzi.
 *   4. tpd_local_init, -12 byte: fusione delle due code di uscita.
 *   5. fts_tp_state_recovery, -4 byte: forma del contatore del ciclo
 *      incorporato.
 *   6. i numeri di riga di FTS_FUNC_EXIT, ZERO byte: dichiarata e non
 *      inseguita, con la tabella completa in coda.
 * Causa comune alle prime cinque, letta dal binario e non supposta: la
 * fabbrica ha compilato con clang 9.0.3 (r353983c), noi con clang 11.0.1
 * (r383902).
 *
 * ===========================================================================
 * 5. CIO' CHE NON E' SCRITTO, E PERCHE'
 * ===========================================================================
 * Delle 68 funzioni del blocco, qui ce ne sono 16 (piu' CINQUE incorporate
 * che nel binario non hanno simbolo: fts_release_all_finger, fts_input_init,
 * fts_read_parse_touchdata, fts_input_report_b e fts_get_ic_information --
 * qui c'era scritto «tre» e al §1 «DUE», corretti il 2026-08-21, vedi §1).
 * Le 52 che MANCANO sono le altre CINQUE unita' di traduzione, e sono un
 * lotto a se':
 *   A  0xffffff8008a7b600..0xa7cc4c  22 funzioni  canale /proc + i nodi sysfs
 *   B  0xffffff8008a7cc4c..0xa7eac0  17 funzioni  aggiornamento del firmware
 *   D  0xffffff8008a8020c..0xa802ac   3 funzioni  glove/cover/charger
 *   E  0xffffff8008a802ac..0xa807e0   7 funzioni  il bus i2c
 *   F  0xffffff8008a807e0..0xa80b54   3 funzioni  l'aggiornamento FT8719
 *
 * CORREZIONE del 2026-08-21: il confine di A era scritto 0xa7cbd0, e 0xa7cbd0
 * cade DENTRO l'ultima funzione dell'unita'.  fts_irq_store comincia a
 * 0xffffff8008a7cbc8 e la successiva, fts_pram_write_init, a
 * 0xffffff8008a7cc4c: sono 132 byte, e 0xa7cbd0 e' il suo terzo.
 *   $ sed -n '22,23p' scout-ft8719/blocco.map
 *   ffffff8008a7cbc8 t fts_irq_store
 *   ffffff8008a7cc4c T fts_pram_write_init
 * Cosi' com'era scritto lasciava 124 byte di buco fra la fine dichiarata di
 * A e l'inizio di B, mentre la partizione in sei unita' non ha un byte di
 * buco.  E' la classe C5, un confine sotto-esteso.  L'errore e' ereditato
 * dalla tabella §5 di scout-ft8719/RIASSUNTO.md e va corretto anche la':
 * non l'ho fatto, e' un file della ricognizione e non di questo lotto.
 * Le quattordici funzioni di confine che questo file CHIAMA e che stanno la'
 * dentro sono DICHIARATE E LASCIATE INDEFINITE, con l'indirizzo di fabbrica
 * accanto: questo file NON si linka, ed e' l'esito onesto (regola 6).  Uno
 * stub le farebbe passare per scritte.
 *
 * Non e' scritto nemmeno il blob del pramboot (3040 byte a
 * 0xffffff800998cc00), che appartiene all'unita' F.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <uapi/linux/sched/types.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/string.h>
#include "tpd.h"

/* ==========================================================================
 * LE MACRO DI STAMPA -- lette dalle stringhe, non supposte
 * ==========================================================================
 * Il binario porta i formati interi, prefisso e livello compresi:
 *   "[FTS]%s: Enter\n"@0xffffff800924f634
 *   "[FTS]%s: Exit(%d)\n"@0xffffff800924f65a
 *   "[FTS]TP Not Ready, ReadData = 0x%x\n"@0xffffff800924f5e7      (nessun livello)
 *   "\x016[FTS][Info]TP Ready, Device ID = 0x%x\n"@0xffffff800924f60b   (KERN_INFO)
 *   "\x013[FTS][Error]request irq failed\n"@0xffffff800924f927     (KERN_ERR)
 * cioe' tre macro distinte: una SENZA livello con prefisso "[FTS]", una con
 * KERN_INFO e prefisso "[FTS][Info]", una con KERN_ERR e "[FTS][Error]".
 * FTS_FUNC_EXIT porta __LINE__: "52802442 mov"@0xffffff8008a7ecd8 e'
 * `mov w2, #0x122` = 290, la riga di `fts_reset_proc` nel file di fabbrica.
 *
 * x0 e' il FORMATO e x1 e' il __func__ -- classe B1, il difetto che un lotto
 * precedente aveva preso al contrario:
 *   "9118d000 add"@0xffffff8008a7ec8c  -> x0 = 0x924f634 = "[FTS]%s: Enter\n"
 *   "9119b421 add"@0xffffff8008a7ec90  -> x1 = 0x924f66d = "fts_reset_proc"
 */
#define FTS_DEBUG(fmt, args...)		printk("[FTS]" fmt "\n", ##args)
#define FTS_INFO(fmt, args...)		printk(KERN_INFO "[FTS][Info]" fmt "\n", ##args)
#define FTS_ERROR(fmt, args...)		printk(KERN_ERR "[FTS][Error]" fmt "\n", ##args)
#define FTS_FUNC_ENTER()		printk("[FTS]%s: Enter\n", __func__)
#define FTS_FUNC_EXIT()			printk("[FTS]%s: Exit(%d)\n", __func__, __LINE__)

/* "Focaltech V2.2 20180321"@0xffffff800924e1c9, secondo argomento della
 * FTS_INFO di tpd_driver_init: "91072421 add"@0xffffff8009383498 mette in x1
 * 0x924e000 + 0x1c9. */
#define FTS_DRIVER_VERSION		"Focaltech V2.2 20180321"

/* Costanti lette dagli immediati, non dai nomi.
 * "52801461 mov"@0xffffff8008a7eb04   -> w1 = 0xa3, registro chip id
 * "528013e1 mov"@0xffffff8008a7f554   -> w1 = 0x9f, secondo byte del chip id
 * "52801501 mov"@0xffffff8008a7ee14   -> w1 = 0xa8, vendor id
 * "528014a1 mov"@0xffffff8008a7f0b8   -> w1 = 0xa5, power mode
 * "528014c1 mov"@0xffffff8008a7fa10   -> w1 = 0xa6, fw version
 * "52800c80 mov"@0xffffff8008a7eb28   -> 100 ms fra due letture
 * "710fa2df cmp"@0xffffff8008a7eb34   -> 1000 ms di attesa massima
 */
#define FTS_REG_CHIP_ID			0xa3
#define FTS_REG_CHIP_ID2		0x9f
#define FTS_REG_MODULE_ID		0xa8
#define FTS_REG_POWER_MODE		0xa5
#define FTS_REG_FW_VER			0xa6
#define INTERVAL_READ_REG		100
#define TIMEOUT_READ_REG		1000

/* "d10943ff sub"@0xffffff8008a7f320 -- la cornice di tpd_probe e' 0x250 byte
 * oltre i 96 dei registri salvati; il buffer della sprintf sta a sp+0x40.
 * Vedi DIVERGENZA 1 in coda. */
#define FTS_TP_INFO_LEN			512
/* "9128e000 add"@0xffffff8008a801e8 + il passo 40 della tabella:
 * "52800509 mov"@0xffffff8008a7f8cc -> w9 = 40, e
 * "9b292261 smaddl"@0xffffff8008a7f8d0 -> x1 = tabella + id*40. */
#define FTS_MODULE_NAME_LEN		40
#define FTS_MODULE_NAME_NUM		9

/* kfree() da sola tratta gia' NULL; il binario mette lo STESSO il confronto:
 *   "b4000060 cbz"@0xffffff8008a7f6e0   x0 == 0 -> salta la kfree
 *   "f900aa9f str"@0xffffff8008a7f6e8   e poi azzera il campo
 * quindi nel sorgente c'e' una macro che fa le due cose. */
#define kfree_safe(pbuf) do { \
	if (pbuf) { \
		kfree(pbuf); \
		pbuf = NULL; \
	} \
} while (0)

/* ==========================================================================
 * IL CONTESTO DEL DRIVER
 * ==========================================================================
 * La taglia della struttura e' letta da
 *   "321907e1 orr"@0xffffff8008a7f354  -> w1 = 0x180 = 384, secondo argomento
 * di devm_kmalloc in tpd_probe.  Gli offset qui sotto sono TUTTI misurati,
 * uno per uno, dalle istruzioni citate accanto.
 *
 * IL NOME DEI CAMPI.  Il binario nomina UN campo solo, e lo nomina davvero:
 *   "&ts_data->report_mutex"@0xffffff800924f7d5
 * e' il secondo argomento di __mutex_init ("97dab438 bl"@0xffffff8008a7f41c),
 * il cui primo argomento e' "91048280 add"@0xffffff8008a7f40c, cioe'
 * ts_data + 0x120.  Da li' vengono DUE nomi e non uno: il campo si chiama
 * `report_mutex` e la variabile locale del probe si chiama `ts_data`.
 * Tutti gli altri campi si chiamano c<offset>, con accanto cio' che di loro
 * e' MISURATO -- non un nome descrittivo, che per chi legge sarebbe
 * indistinguibile da un fatto (classe B2).
 *
 * Il NOME DEL TIPO e' una SCELTA: il binario non lo contiene.
 */

/* I 16 byte a 0xffffff800924fee8, copiati in blocco dentro il contesto:
 *   "a9402508 ldp"@0xffffff8008a7f728   x8,x9 <- 0x924fee8
 *   "a901a688 stp"@0xffffff8008a7f72c   -> ts_data + 24
 * Contenuto letto dall'immagine (objdump -s):
 *   ffffff800924fee8  0d000000 00000000 87198719 87a987b9
 * I byte 8 e 9 sono i due che il binario poi confronta e stampa:
 *   "71021f9f cmp"@0xffffff8008a7f588  -> 0x87
 *   "7100677f cmp"@0xffffff8008a7f590  -> 0x19
 */
struct fts_ic_ids {
	u64 c0;			/* = 0x0d */
	u8 c8;			/* = 0x87; letto da "39408281 ldrb"@0xffffff8008a7f738 */
	u8 c9;			/* = 0x19; letto da "39408682 ldrb"@0xffffff8008a7f73c */
	u8 c10;			/* = 0x87 */
	u8 c11;			/* = 0x19 */
	u8 c12;			/* = 0x87 */
	u8 c13;			/* = 0xa9; cfr "7102a6df cmp"@0xffffff8008a7f688 */
	u8 c14;			/* = 0x87 */
	u8 c15;			/* = 0xb9; cfr "7102e6df cmp"@0xffffff8008a7f680 */
};

struct fts_event {
	int c0;			/* x;  "b94002e3 ldr"@0xffffff8008a7ffcc -> ABS_MT_POSITION_X */
	int c4;			/* y;  "b94006e3 ldr"@0xffffff8008a7ffe4 -> ABS_MT_POSITION_Y */
	int c8;			/* "b9400ac3 ldr"@0xffffff8008a7ff74 -> ABS_MT_PRESSURE */
	int c12;		/* "b9400ea8 ldr"@0xffffff8008a7fef0, bit 1 = giu'/su' */
	int c16;		/* "b94012a3 ldr"@0xffffff8008a7ff28, indice dello slot */
	int c20;		/* "b94016c3 ldr"@0xffffff8008a7ff9c -> ABS_MT_TOUCH_MAJOR */
};				/* 24 byte: "8b130668 add"+"d37df100 lsl"@0xffffff8008a7f484/84 */

struct fts_ts_data {
	struct i2c_client *c0;	/* "f9000015 str"@0xffffff8008a7f374 = il client del probe */
	struct input_dev *c8;	/* "f9000408 str"@0xffffff8008a7f38c = tpd->dev */
	u8 c16;			/* "79002298 strh"@0xffffff8008a7f514 scrive c16=1 e c17=0 */
	u8 c17;			/* letto da "39404688 ldrb"@0xffffff8008a7f5e0 */
	u8 c18[6];		/* mai toccati; riempimento fino all'allineamento di c24 */
	struct fts_ic_ids c24;	/* "a901a688 stp"@0xffffff8008a7f72c */
	struct workqueue_struct *c40;	/* "f9001680 str"@0xffffff8008a7f3f0 */
	u8 c48[224];		/* mai toccati da questa unita' */
	struct task_struct *c272;	/* "f9008a97 str"@0xffffff8008a7f7d4 */
	spinlock_t c280;	/* "b9011a9f str"@0xffffff8008a7f418, poi
				 * "91046100 add"@0xffffff8008a7ed18 -> +0x118 */
	struct mutex report_mutex;	/* +288; NOMINATO dal binario, vedi sopra */
	int c320;		/* irq: "b9014280 str"@0xffffff8008a7f840 */
	u8 c324;		/* "39051288 strb"@0xffffff8008a7f0f0 = 1 in tpd_suspend */
	u8 c325;		/* "39451688 ldrb"@0xffffff8008a7f034 */
	u8 c326;		/* "39451909 ldrb"@0xffffff8008a7ed28 sotto lo spinlock */
	u8 c327;		/* mai toccato; riempimento */
	struct fts_event *c328;	/* "f900a680 str"@0xffffff8008a7f490 */
	u8 *c336;		/* "f900aa80 str"@0xffffff8008a7f474 */
	int c344;		/* "b9015a88 str"@0xffffff8008a7f46c, il numero di byte */
	int c348;		/* "b9415f29 ldr"@0xffffff8008a7fffc, maschera di bit */
	u8 c352;		/* "39058328 strb"@0xffffff8008a7ff04 */
	u8 c353[3];		/* riempimento */
	int c356;		/* "b901672c str"@0xffffff8008a7fdfc, contatore dei punti */
	int c360;		/* "b9016b21 str"@0xffffff8008a7fd30 */
	u8 c364[20];		/* fino a 384 = la taglia misurata */
};

/* La costante da cui vengono i 16 byte di c24. */
static const struct fts_ic_ids fts_ctype = {
	0x0dULL, 0x87, 0x19, 0x87, 0x19, 0x87, 0xa9, 0x87, 0xb9,
};

/* ==========================================================================
 * I GLOBALI
 * ==========================================================================
 * `fts_data` e' l'UNICO che il binario nomina, e lo nomina in chiaro:
 *   "\x013[FTS][Error]Failed to allocate memory for fts_data\n"@0xffffff800924f718
 * e' il messaggio del ramo in cui la devm_kmalloc che lo riempie fallisce
 * ("b4001a40 cbz"@0xffffff8008a7f364).  Sta a 0xffffff800a100ad8, lo scrive
 * solo tpd_probe ("f9056d20 str"@0xffffff8008a7f378) e lo leggono 19
 * funzioni, anche di altre unita' di traduzione: non e' static.
 *
 * Gli altri quattro il binario NON li nomina.  Si chiamano fts_g_<indirizzo
 * di fabbrica>, e il commento dice cio' che di loro e' MISURATO.
 */
struct fts_ts_data *fts_data;

/* DELTA DI HEADER (non fatto qui: e' un file CONDIVISO).  `tpd_type_cap` e'
 * definito in drivers/input/touchscreen/mediatek/tpd_setting.c e dichiarato
 * in tpd_debug.h, che tpd.h include SOLO sotto TPD_DEBUG_CODE.  Qui e'
 * dichiarato localmente, come fa gia' ilitek_bus.c per le funzioni del
 * gruppo E; la sede giusta sarebbe tpd.h. */
extern int tpd_type_cap;

/* 0xffffff800a100ae0: scritto da tpd_probe ("f9057135 str"@0xffffff8008a7f394)
 * con lo stesso i2c_client, letto solo da get_module_id
 * ("f9457280 ldr"@0xffffff8008a7ee1c).
 *
 * NON static E' UNA SCELTA, NON UNA MISURA (regola 4).  CORREZIONE del
 * 2026-08-21: qui c'era, come giustificazione, l'argomento «`ldr x` a
 * larghezza piena», cioe' l'argomento della classe A1 che uso qui sotto per
 * fts_g_a100ad0 e fts_g_998c910.  Su quei due discrimina, perche' hanno un
 * valore costante che clang avrebbe potuto piegare o restringere; su un
 * PUNTATORE scritto a tempo di esecuzione non discrimina affatto: un
 * `static struct i2c_client *`, scritto da tpd_probe e letto da
 * get_module_id -- due funzioni della STESSA unita' -- darebbe la medesima
 * `ldr x` a 64 bit, perche' non c'e' nessuna costante da piegare.
 * La scansione dell'INTERA immagine disassemblata lo conferma: fuori da
 * questa unita' nessuno tocca quell'indirizzo, quindi il binario non ha modo
 * di distinguere `static` da non static.
 *   $ ./venv/bin/python3 corr-ft8719/scan.py 0xffffff800a100ae0
 *   riferimenti a 0xffffff800a100ae0:
 *     get_module_id                      ldr    x2
 *     tpd_probe                          str    x1
 *     TOTALE siti: 3
 * Per contrasto la stessa scansione su fts_data (0xffffff800a100ad8) trova
 * 19 funzioni lettrici, fra cui fts_debug_read, fts_tprwreg_show e
 * fts_fwupg_get_boot_state, che stanno in ALTRE unita' di traduzione: la',
 * il non-static e' PROVATO.  Qui e' una scelta, e questa e' l'etichetta. */
struct i2c_client *fts_g_a100ae0;

/* 0xffffff800a100ad0, in .bss (vale 0), passato come PRIMO argomento a
 * tpd_gpio_output ("97ff4902 bl"@0xffffff8008a7eca4) da fts_reset_proc,
 * tpd_probe e tpd_resume.  NESSUNA istruzione in tutta l'immagine ci scrive.
 * Non e' static: un `static int` mai assegnato varrebbe 0 e clang lo avrebbe
 * piegato a `mov w0, wzr`; qui invece c'e' un carico a larghezza piena,
 * "b94ad280 ldr"@0xffffff8008a7ec9c.  E' la classe di difetto A1 usata al
 * contrario. */
int fts_g_a100ad0;

/* 0xffffff800998c910, in .data (vale 1), passato a tpd_gpio_as_int
 * ("97ff460a bl"@0xffffff8008a7f818).  Nessuno lo scrive; stesso ragionamento
 * di sopra: "b9491100 ldr"@0xffffff8008a7f814 e' un carico a larghezza piena,
 * quindi non e' static. */
int fts_g_998c910 = 1;

/* 0xffffff800a100aec: scritto a 1 da tpd_eint_interrupt_handler
 * ("392bb109 strb"@0xffffff8008a801f8), azzerato e atteso da
 * touch_event_handler ("396bb388 ldrb"+"370002c8 tbnz"@0xffffff8008a7fc68/68).
 * Gli accessi sono RISTRETTI (ldrb/strb + tbnz #0), cioe' compatibili con un
 * `static int` che clang ha ristretto a due valori. */
static int fts_g_a100aec;

/* 0xffffff800998ca38: gli offset +0x08 e +0x10 puntano a +0x08, cioe' lista
 * vuota, cioe' inizializzazione STATICA (nessun __init_waitqueue_head in
 * tpd_probe).  __wake_up da "97da7461 bl"@0xffffff8008a801fc. */
static DECLARE_WAIT_QUEUE_HEAD(fts_g_998ca38);

/* 0xffffff800998ca50, 360 byte in .data, indicizzata con moltiplicatore 40
 * dal valore che get_module_id restituisce ("9b292261 smaddl"@0xffffff8008a7f8d0).
 * Contenuto letto dall'immagine, non supposto (objdump -s):
 *   ffffff800998ca50  6e6f6e65 5f300000 ...  "none_0"
 *   ffffff800998ca78  6e6f6e65 5f310000 ...  "none_1"   (passo 40)
 *   ... fino a "none_8" a 0xffffff800998cb90.
 * Non e' const: sta in .data e non in .rodata. */
static char fts_g_998ca50[FTS_MODULE_NAME_NUM][FTS_MODULE_NAME_LEN] = {
	"none_0", "none_1", "none_2", "none_3", "none_4",
	"none_5", "none_6", "none_7", "none_8",
};

/* ==========================================================================
 * IL CONFINE CON LE ALTRE CINQUE UNITA' DI TRADUZIONE
 * ==========================================================================
 * Queste funzioni NON sono scritte.  Sono dichiarate e lasciate indefinite:
 * il link che fallisce e' l'esito onesto, uno stub le farebbe passare per
 * scritte (regola 6, classe B4).  L'indirizzo di fabbrica di ognuna e'
 * accanto, cosi' il prossimo lotto sa dove guardare.
 */
int fts_i2c_read(struct i2c_client *client, char *writebuf, int writelen,
		 char *readbuf, int readlen);		/* 0xffffff8008a802ac */
int fts_i2c_write(struct i2c_client *client, char *writebuf, int writelen);
							/* 0xffffff8008a80494 */
int fts_i2c_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue);
							/* 0xffffff8008a805c4 */
int fts_i2c_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue);
							/* 0xffffff8008a80614 */
int fts_i2c_hid2std(struct i2c_client *client);		/* 0xffffff8008a80668 */
int fts_i2c_init(void);					/* 0xffffff8008a80750 */
int fts_i2c_exit(void);					/* 0xffffff8008a80798 */
int fts_ex_mode_init(struct i2c_client *client);	/* 0xffffff8008a8020c */
int fts_ex_mode_exit(struct i2c_client *client);	/* 0xffffff8008a80280 */
int fts_ex_mode_recovery(struct i2c_client *client);	/* 0xffffff8008a802a4 */
int fts_create_apk_debug_channel(struct fts_ts_data *ts_data);
							/* 0xffffff8008a7b600 */
void fts_release_apk_debug_channel(struct fts_ts_data *ts_data);
							/* 0xffffff8008a7b664 */
int fts_create_sysfs(struct i2c_client *client);	/* 0xffffff8008a7b680 */
int fts_remove_sysfs(struct i2c_client *client);	/* 0xffffff8008a7b6e4 */

/* Non appartiene al blocco: e' una delle QUATTRO funzioni Wingtech che la
 * patch di fabbrica aggiunge a mtk_tpd.c (0xffffff8008a5151c..0xffffff8008a515e8)
 * e che il nostro albero ALPS non ha.  Chiamata da tpd_probe:
 * "97ff46ac bl"@0xffffff8008a7fa6c. */
int fix_tp_proc_info(char *buf, int len);		/* 0xffffff8008a5151c */

/* Dichiarazioni in avanti dell'unita': l'ordine del testo di fabbrica non e'
 * l'ordine del sorgente (i numeri di riga di FTS_FUNC_EXIT lo provano), e le
 * strutture dati vengono prima. */
static int tpd_local_init(void);
static void tpd_suspend(struct device *h);
static void tpd_resume(struct device *h);
static int tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_remove(struct i2c_client *client);
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
static int touch_event_handler(void *unused);

/* ==========================================================================
 * LE STRUTTURE DI REGISTRAZIONE
 * ==========================================================================
 * Lette dalle RILOCAZIONI (CONFIG_RELOCATABLE=y: nell'immagine i puntatori
 * valgono zero e il valore vero sta nell'addend di una R_AARCH64_RELATIVE).
 *
 * tpd_driver_t @0xffffff800998c918, 56 byte (fino a 0xffffff800998c950):
 *   +0x00 -> "fts_ts"      +0x08 tpd_local_init
 *   +0x10 tpd_suspend      +0x18 tpd_resume
 * Che sia LUI quello del focaltech e non del goodix e' misurato due volte:
 * tpd_driver_init gli passa proprio 0x998c918 ("91246000 add"@0xffffff80093834dc,
 * "97db37a9 bl"@0xffffff80093834e0 -> tpd_driver_add) e nella tabella degli
 * initcall l'ordine e' quello del link del testo.
 *
 * i2c_driver @0xffffff800998c950:
 *   +0x10 probe = tpd_probe        +0x18 remove = tpd_remove
 *   +0x40 driver.name = "fts_ts"   +0x68 driver.of_match_table -> 0x8f7f460
 *   +0xb8 id_table -> 0x8f7f5f0    +0xc0 detect = tpd_i2c_detect
 * Gli offset 0x40/0x68 (e non 0x38/0x60) confermano che questo albero ha il
 * campo `probe_new` in struct i2c_driver, come il nostro.
 */
static const struct of_device_id fts_dt_match[] = {
	/* "mediatek,cap_touch" e' il `compatible` a 0xffffff8008f7f460 + 0x40. */
	{ .compatible = "mediatek,cap_touch" },
	{},
};
MODULE_DEVICE_TABLE(of, fts_dt_match);

static const struct i2c_device_id fts_tpd_id[] = {
	/* 0xffffff8008f7f5f0: name = "fts_ts", driver_data = 0, poi il
	 * terminatore a zero. */
	{ "fts_ts", 0 },
	{},
};

static struct i2c_driver tpd_i2c_driver = {
	.driver = {
		.name = "fts_ts",
		.of_match_table = of_match_ptr(fts_dt_match),
	},
	.probe = tpd_probe,
	.remove = tpd_remove,
	.id_table = fts_tpd_id,
	.detect = tpd_i2c_detect,
};

static struct tpd_driver_t tpd_device_driver = {
	.tpd_device_name = "fts_ts",
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
};

/* ==========================================================================
 * LE FUNZIONI
 * ==========================================================================
 * L'ordine e' quello del SORGENTE di fabbrica, ricostruito dai numeri di riga
 * che FTS_FUNC_EXIT lascia nel binario, NON quello del testo: nel testo
 * tpd_probe (riga 1089) sta dopo tpd_resume (riga 1341).
 */

/*
 * fts_wait_tp_to_valid @0xffffff8008a7eac0, 196 byte, visibilita' T.
 */
int fts_wait_tp_to_valid(struct i2c_client *client)
{
	int ret = 0;
	int cnt = 0;
	u8 reg_value = 0;
	/* Il chip id e' letto UNA volta, PRIMA del ciclo:
	 * "39408114 ldrb"@0xffffff8008a7eafc sta a 0xffffff8008a7eafc e il ciclo
	 * comincia a 0xffffff8008a7eb00 ("910013e2 add"). Se il sorgente lo
	 * rileggesse dentro il confronto, fts_i2c_read_reg lo invaliderebbe e
	 * la ldrb starebbe dentro il ciclo -- e' la classe A2 al contrario. */
	u8 chip_id = fts_data->c24.c8;

	do {
		ret = fts_i2c_read_reg(client, FTS_REG_CHIP_ID, &reg_value);
		if ((ret < 0) || (reg_value != chip_id)) {
			/* "37f80060 tbnz"@0xffffff8008a7eb14 e' il test di ret<0;
			 * "6b14003f cmp"@0xffffff8008a7eb18 quello di reg==chip. */
			FTS_DEBUG("TP Not Ready, ReadData = 0x%x", reg_value);
		} else {
			FTS_INFO("TP Ready, Device ID = 0x%x", reg_value);
			return 0;
		}
		cnt++;
		msleep(INTERVAL_READ_REG);
		/* clang riduce il contatore al passo 100:
		 * "110192d6 add"@0xffffff8008a7eb30 e
		 * "710fa2df cmp"@0xffffff8008a7eb34 (0x3e8 = 1000). */
	} while ((cnt * INTERVAL_READ_REG) < TIMEOUT_READ_REG);

	/* "321d7be0 orr"@0xffffff8008a7eb3c -> w0 = 0xfffffffb = -5 = -EIO */
	return -EIO;
}

/*
 * fts_tp_state_recovery @0xffffff8008a7eb84, 240 byte, visibilita' T.
 * Il corpo di fts_wait_tp_to_valid ci e' INCORPORATO (stesse istruzioni,
 * "9400068c bl"@0xffffff8008a7ebe4 chiama fts_i2c_read_reg direttamente) e il
 * valore di ritorno e' buttato: dopo il ramo "TP Ready" si prosegue a
 * 0xffffff8008a7ec28 come dopo il timeout ("14000005 b"@0xffffff8008a7ec14).
 */
void fts_tp_state_recovery(struct i2c_client *client)
{
	FTS_FUNC_ENTER();
	fts_wait_tp_to_valid(client);
	/* "9400059e bl"@0xffffff8008a7ec2c -> fts_ex_mode_recovery (unita' D) */
	fts_ex_mode_recovery(client);
	FTS_FUNC_EXIT();
}

/*
 * fts_reset_proc @0xffffff8008a7ec74, 124 byte, visibilita' T.
 * Anche questa e' incorporata in tpd_probe e in tpd_resume, ma ha simbolo
 * proprio: clang ha emesso sia la copia fuori linea sia quella incorporata.
 */
int fts_reset_proc(int hdelayms)
{
	FTS_FUNC_ENTER();
	/* "b94ad280 ldr"@0xffffff8008a7ec9c carica il pin a larghezza piena,
	 * "2a1f03e1 mov"@0xffffff8008a7eca0 mette il livello a 0. */
	tpd_gpio_output(fts_g_a100ad0, 0);
	/* "52800280 mov"@0xffffff8008a7eca8 -> w0 = 20 */
	msleep(20);
	/* "320003e1 orr"@0xffffff8008a7ecb4 -> livello 1 */
	tpd_gpio_output(fts_g_a100ad0, 1);
	/* "34000073 cbz"@0xffffff8008a7ecbc: la msleep e' condizionata */
	if (hdelayms)
		msleep(hdelayms);
	FTS_FUNC_EXIT();
	return 0;
}

/*
 * fts_irq_disable @0xffffff8008a7ecf0, 132 byte, visibilita' T.
 * Il campo c326 e' letto e scritto SOTTO lo spinlock c280, e fts_data e'
 * riletto dal globale dopo la _raw_spin_lock_irqsave
 * ("f9456e88 ldr"@0xffffff8008a7ed20): il sorgente usa `fts_data->...` e non
 * un locale, altrimenti il puntatore starebbe in un registro salvato.
 */
void fts_irq_disable(void)
{
	unsigned long irqflags;

	FTS_FUNC_ENTER();
	spin_lock_irqsave(&fts_data->c280, irqflags);
	/* "350000c9 cbnz"@0xffffff8008a7ed2c: si disabilita solo se non lo e' gia' */
	if (!fts_data->c326) {
		/* "b9414100 ldr"@0xffffff8008a7ed30 -> irq a +320 */
		disable_irq_nosync(fts_data->c320);
		fts_data->c326 = 1;
	}
	spin_unlock_irqrestore(&fts_data->c280, irqflags);
	FTS_FUNC_EXIT();
}

/*
 * fts_irq_enable @0xffffff8008a7ed74, 128 byte, visibilita' T.
 * Quattro byte in meno di fts_irq_disable: la scrittura del flag e' `strb wzr`
 * ("3905191f strb"@0xffffff8008a7edc0) invece di `orr w9,wzr,#1` + `strb`.
 */
void fts_irq_enable(void)
{
	unsigned long irqflags;

	FTS_FUNC_ENTER();
	spin_lock_irqsave(&fts_data->c280, irqflags);
	/* "340000a9 cbz"@0xffffff8008a7edb0 -- il test e' invertito rispetto a
	 * fts_irq_disable */
	if (fts_data->c326) {
		/* "97daefff bl"@0xffffff8008a7edb8 -> enable_irq */
		enable_irq(fts_data->c320);
		fts_data->c326 = 0;
	}
	spin_unlock_irqrestore(&fts_data->c280, irqflags);
	FTS_FUNC_EXIT();
}

/*
 * fts_release_all_finger -- INCORPORATA in tpd_resume, senza simbolo proprio.
 * Il "__func__" che stampa la denuncia (classe B5):
 *   "fts_release_all_finger"@0xffffff800924fe55 e' l'argomento x1 della
 *   FTS_FUNC_ENTER a "97dad0df bl"@0xffffff8008a7f158, dentro tpd_resume.
 * La FTS_FUNC_EXIT porta la riga 1c3 = 451 ("52803862 mov"@0xffffff8008a7f1f0).
 */
static void fts_release_all_finger(void)
{
	struct input_dev *input_dev = fts_data->c8;
	u32 finger_count = 0;

	FTS_FUNC_ENTER();
	mutex_lock(&fts_data->report_mutex);
	/* Il limite e' RILETTO a ogni giro: "b94f92e8 ldr"@0xffffff8008a7f19c sta
	 * dentro il ciclo, non davanti.  Il confronto e' senza segno
	 * ("6b08029f cmp"+"54fffe83 b.cc"@0xffffff8008a7f1a8/a8), quindi il
	 * contatore e' un u32. */
	for (finger_count = 0; finger_count < tpd_dts_data.touch_max_num; finger_count++) {
		/* "528005e2 mov"@0xffffff8008a7f17c -> ABS_MT_SLOT = 0x2f */
		input_mt_slot(input_dev, finger_count);
		input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, false);
	}
	/* "52802942 mov"@0xffffff8008a7f1b0 -> BTN_TOUCH = 0x14a */
	input_report_key(input_dev, BTN_TOUCH, 0);
	input_sync(input_dev);
	mutex_unlock(&fts_data->report_mutex);
	FTS_FUNC_EXIT();
}

/*
 * fts_input_init -- INCORPORATA in tpd_probe, senza simbolo proprio.
 *   "fts_input_init"@0xffffff800924f9d4 e' l'x1 della FTS_FUNC_ENTER a
 *   "97dad028 bl"@0xffffff8008a7f434, dentro tpd_probe.
 * Ha DUE uscite: riga 845 sul successo ("528069a2 mov"@0xffffff8008a7f4a8) e
 * riga 852 sull'errore ("52806a82 mov"@0xffffff8008a7f6fc).
 */
static int fts_input_init(struct i2c_client *client)
{
	int point_num = 0;

	FTS_FUNC_ENTER();
	/* "321f03e2 orr"@0xffffff8008a7f444 -> w2 = 2 = INPUT_MT_DIRECT */
	input_mt_init_slots(tpd->dev, tpd_dts_data.touch_max_num, INPUT_MT_DIRECT);

	/* Il secondo carico e' una ldrsw ("b98f9273 ldrsw"@0xffffff8008a7f450) e
	 * il suo valore sopravvive alla __kmalloc: se il sorgente rileggesse
	 * tpd_dts_data.touch_max_num per la seconda kzalloc, ci sarebbe un terzo
	 * carico dopo la chiamata.  C'e' quindi un locale, letto una volta. */
	point_num = tpd_dts_data.touch_max_num;
	/* "1b082668 madd"@0xffffff8008a7f460 -> w8 = point_num*6 + 3 */
	fts_data->c344 = point_num * 6 + 3;
	fts_data->c336 = kzalloc(fts_data->c344, GFP_KERNEL);
	if (!fts_data->c336) {
		FTS_ERROR("failed to alloc memory for point buf!");
		goto err_point_buf;
	}

	/* "8b130668 add"+"d37df100 lsl"@0xffffff8008a7f484/84 -> point_num*24 */
	fts_data->c328 = kzalloc(point_num * sizeof(struct fts_event), GFP_KERNEL);
	if (!fts_data->c328) {
		FTS_ERROR("failed to alloc memory for point events!");
		goto err_event_buf;
	}

	FTS_FUNC_EXIT();
	return 0;

err_event_buf:
	kfree_safe(fts_data->c336);
err_point_buf:
	FTS_FUNC_EXIT();
	/* "12800177 mov"@0xffffff8008a7f710 -> w23 = 0xfffffff4 = -12 = -ENOMEM,
	 * messo dal CHIAMANTE dopo la FTS_ERROR di "fts input initialize fail". */
	return -ENOMEM;
}

/*
 * get_module_id @0xffffff8008a7edf4, 332 byte, visibilita' T.
 * Le due stampe sono TPD_DMESG di tpd.h, non FTS_*: il formato di fabbrica e'
 *   "\x016mtk-tpd:[%s:%d] fts ret = %d ,verdor_id = 0x%x.\n"@0xffffff800924f69b
 * che e' esattamente `pr_info(TPD_DEVICE ":[%s:%d] " a, __func__, __LINE__, ...)`
 * con TPD_DEVICE = "mtk-tpd".  Le righe sono 892 ("52806f82 mov"@0xffffff8008a7ee40)
 * e 898 ("52807042 mov"@0xffffff8008a7eea0).  "verdor_id" e' l'ortografia di
 * fabbrica e si riproduce (regola 7).
 */
int get_module_id(void)
{
	int ret = 0;
	u8 vendor_id = 0;
	/* "12001e68 and"@0xffffff8008a7ee74 tronca il contatore a 8 bit prima
	 * del confronto: e' un u8, non un int (classe A4). */
	u8 i = 0;

	ret = fts_i2c_read_reg(fts_g_a100ae0, FTS_REG_MODULE_ID, &vendor_id);
	TPD_DMESG("fts ret = %d ,verdor_id = 0x%x.\n", ret, vendor_id);
	/* "7100067f cmp"@0xffffff8008a7ee4c -> ret == 1 */
	if (ret != 1) {
		/* "7100111f cmp"@0xffffff8008a7ee78 + "54000068 b.hi" -> (u8)i > 4
		 *
		 * LO SROTOLAMENTO E' L'UNICA COSA CHE SEPARAVA QUESTA FUNZIONE
		 * DALLA FABBRICA, e la pragma e' un'impalcatura dichiarata --
		 * non emette codice. Il binario ha un CICLO VERO con contatore:
		 * "11000673 add"@0xffffff8008a7ee70 incrementa w19 e
		 * "12001e68 and"@0xffffff8008a7ee74 lo tronca a otto bit prima
		 * del confronto. Da noi clang srotolava i cinque giri e la
		 * funzione misurava 400 byte contro 332. Come per `squareRoot`
		 * di mir3da, PERCHE' LA FABBRICA NON SROTOLI NON E' SPIEGATO:
		 * e' la stessa versione di clang e le stesse opzioni.
		 */
#pragma clang loop unroll(disable)
		for (i = 0; i < 5; i++) {
			ret = fts_i2c_read_reg(fts_g_a100ae0, FTS_REG_MODULE_ID,
					       &vendor_id);
			if (ret == 1)
				break;
		}
	}
	TPD_DMESG("fts ret = %d ,verdor_id = 0x%x.\n", ret, vendor_id);

	switch (vendor_id) {
	case 0x25:		/* "7100951f cmp"@0xffffff8008a7eec4 */
		return 0;	/* "2a1f03e0 mov"@0xffffff8008a7eecc */
	case 0x82:		/* "7102091f cmp"@0xffffff8008a7eee4 */
		return 1;	/* "320003e0 orr"@0xffffff8008a7eeec */
	case 0x67:		/* "71019d1f cmp"@0xffffff8008a7eed4 */
		return 2;	/* "321f03e0 orr"@0xffffff8008a7ef04 */
	case 0x11:		/* "7100451f cmp"@0xffffff8008a7eeb4 */
		return 3;	/* "320007e0 orr"@0xffffff8008a7eef4 */
	case 0xd2:		/* "7103491f cmp"@0xffffff8008a7eedc */
		return 4;	/* "321e03e0 orr"@0xffffff8008a7ef0c */
	case 0x12:		/* "7100491f cmp"@0xffffff8008a7eebc */
		return 5;	/* "528000a0 mov"@0xffffff8008a7eefc */
	default:
		return 8;	/* "321d03e0 orr"@0xffffff8008a7ef14 */
	}
}

/*
 * fts_read_parse_touchdata -- INCORPORATA in touch_event_handler.
 * Non ha ne' simbolo ne' FTS_FUNC_ENTER: quello che la denuncia come funzione
 * separata e' il DOPPIO carico di tpd_dts_data.touch_max_num.
 * "b98f9113 ldrsw"@0xffffff8008a7fce0 lo mette in x19 e x19 sopravvive alla
 * chiamata a fts_i2c_read; poi, dopo mutex_lock,
 * "b94f9138 ldr"@0xffffff8008a7fe9c lo RICARICA in w24.  Due locali distinti
 * in due corpi distinti; un corpo solo avrebbe un locale solo.
 */
static int fts_read_parse_touchdata(struct fts_ts_data *data)
{
	int i = 0;
	int ret = 0;
	int base = 0;
	int pointid = 0;
	int max_touch_num = tpd_dts_data.touch_max_num;
	u8 *buf = data->c336;
	struct fts_event *events = data->c328;

	/* "f900011f str"@0xffffff8008a7fcf0 azzera OTTO byte a +356: clang ha
	 * fuso i due azzeramenti di c356 e c360, che sono adiacenti. */
	data->c356 = 0;
	data->c360 = 0;

	/* "32001fe1 orr"@0xffffff8008a7fcf8 -> w1 = 0xff;
	 * "b9815b22 ldrsw"@0xffffff8008a7fcf4 -> la lunghezza e' c344, con segno */
	memset(buf, 0xff, data->c344);
	buf[0] = 0x00;

	/* "320003e2 orr"@0xffffff8008a7fd10 -> writelen = 1;
	 * lo stesso buf e' sorgente e destinazione ("aa1a03e1 mov"/"aa1a03e3 mov"
	 * @0xffffff8008a7fd14/18). */
	ret = fts_i2c_read(data->c0, buf, 1, buf, data->c344);
	if (ret < 0) {
		FTS_ERROR("read touchdata failed, ret:%d", ret);
		return ret;
	}

	/* "12000d01 and"@0xffffff8008a7fd28 -> point_num = buf[2] & 0x0f */
	data->c360 = buf[2] & 0x0f;

	/* "71003c3f cmp"@0xffffff8008a7fd2c -> 0x0f, e poi SEI confronti con 0xff
	 * su buf[1]..buf[6] ("7103fd1f cmp"@0xffffff8008a7fd44 e i cinque
	 * successivi). */
	if ((data->c360 == 0x0f) && data->c16 &&
	    (buf[1] == 0xff) && (buf[2] == 0xff) && (buf[3] == 0xff) &&
	    (buf[4] == 0xff) && (buf[5] == 0xff) && (buf[6] == 0xff)) {
		FTS_INFO("touch buff is 0xff, need recovery state");
		fts_tp_state_recovery(data->c0);
		return -EIO;
	}

	if (data->c360 > max_touch_num) {
		FTS_INFO("invalid point_num(%d)", data->c360);
		return -EIO;
	}

	for (i = 0; i < max_touch_num; i++) {
		base = 8 + 6 * i;
		/* "71027d9f cmp"@0xffffff8008a7fdd4 + "54000568 b.hi": clang ha
		 * riscritto `(buf[base-3] >> 4) >= 10` come `buf[base-3] > 0x9f`. */
		pointid = buf[base - 3] >> 4;
		if (pointid >= 10)
			break;
		else if (pointid >= max_touch_num) {
			FTS_ERROR("ID(%d) beyond max_touch_number", pointid);
			return -EINVAL;
		}

		data->c356++;
		/* "33180d8e bfi"@0xffffff8008a7fe0c: (buf[base-5]&0x0f)<<8 | buf[base-4] */
		events[i].c0 = ((buf[base - 5] & 0x0f) << 8) + buf[base - 4];
		events[i].c4 = ((buf[base - 3] & 0x0f) << 8) + buf[base - 2];
		/* "53067d8c lsr"@0xffffff8008a7fe30 -> >> 6 */
		events[i].c12 = buf[base - 5] >> 6;
		events[i].c16 = buf[base - 3] >> 4;
		events[i].c20 = buf[base] >> 4;
		events[i].c8 = buf[base - 1];

		/* "321f018c orr"+"7100099f cmp"@0xffffff8008a7fe40/40: (flag|2)==2,
		 * cioe' flag e' 0 o 2. */
		if (((events[i].c12 == 0) || (events[i].c12 == 2)) && (data->c360 == 0)) {
			FTS_INFO("abnormal touch data from fw");
			return -EIO;
		}
	}

	if (data->c356 == 0) {
		FTS_INFO("no touch point information");
		return -EIO;
	}

	return 0;
}

/*
 * fts_input_report_b -- INCORPORATA in touch_event_handler, come sopra.
 * TPD_RES_Y e' NOMINATA dal binario: e' il globale a 0xffffff800996b2b8 che
 * tpd_probe stampa con "[FTS]TPD_RES_Y:%d\n"@0xffffff800924f949.
 * Il confronto e' SENZA SEGNO a 32 bit ("6b09011f cmp"+"54000209 b.ls"
 * @0xffffff8008a7fee0/e4): in questa unita' TPD_RES_Y vale 32 bit senza segno,
 * mentre tpd.h la dichiara `unsigned long`.  Vedi DIVERGENZA 2.
 */
static int fts_input_report_b(struct fts_ts_data *data)
{
	int i = 0;
	int touchs = 0;
	bool va_reported = false;
	u32 max_touch_num = tpd_dts_data.touch_max_num;
	u32 res_y = (u32)TPD_RES_Y;
	struct fts_event *events = data->c328;

	/* Il limite del ciclo e' RILETTO a ogni giro:
	 * "b9816728 ldrsw"@0xffffff8008a80094 sta in coda al corpo. */
	for (i = 0; i < data->c356; i++) {
		if (tpd_dts_data.use_tpd_button && (events[i].c4 > res_y)) {
			if ((events[i].c12 == 0) || (events[i].c12 == 2)) {
				data->c352 = 1;
				/* "97ff48db bl"@0xffffff8008a7ff10 -> tpd_button */
				tpd_button(events[i].c0, events[i].c4, 1);
				FTS_DEBUG("Key(%d, %d) DOWN", events[i].c0, events[i].c4);
			} else {
				data->c352 = 0;
				tpd_button(events[i].c0, events[i].c4, 0);
				FTS_DEBUG("Key(%d, %d) UP", events[i].c0, events[i].c4);
			}
			continue;
		}

		/* "6b18007f cmp"+"54000bc2 b.cs"@0xffffff8008a7ff30/30: il confronto
		 * con il massimo e' SENZA segno, quindi max_touch_num e' u32. */
		if (events[i].c16 >= max_touch_num)
			break;

		input_mt_slot(tpd->dev, events[i].c16);

		if ((events[i].c12 == 0) || (events[i].c12 == 2)) {
			input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, true);
			/* "320017e3 orr"@0xffffff8008a7ff80 -> 0x3f */
			if (events[i].c8 <= 0)
				events[i].c8 = 0x3f;
			/* "52800742 mov"@0xffffff8008a7ff90 -> ABS_MT_PRESSURE = 0x3a */
			input_report_abs(tpd->dev, ABS_MT_PRESSURE, events[i].c8);
			/* "52800123 mov"@0xffffff8008a7ffa8 -> 9 */
			if (events[i].c20 <= 0)
				events[i].c20 = 9;
			/* "321c07e2 orr"@0xffffff8008a7ffb8 -> ABS_MT_TOUCH_MAJOR = 0x30 */
			input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, events[i].c20);
			input_report_abs(tpd->dev, ABS_MT_POSITION_X, events[i].c0);
			input_report_abs(tpd->dev, ABS_MT_POSITION_Y, events[i].c4);

			/* "9ac82368 lsl"@0xffffff8008a80008 -> 1 << id, a 64 bit */
			data->c348 |= BIT(events[i].c16);
			touchs |= BIT(events[i].c16);
			va_reported = true;
			FTS_DEBUG("[B]P%d(%d, %d)[p:%d,tm:%d] DOWN!",
				  events[i].c16, events[i].c0, events[i].c4,
				  events[i].c8, events[i].c20);
		} else {
			va_reported = true;
			input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, false);
			/* "0a280128 bic"@0xffffff8008a8005c */
			data->c348 &= ~BIT(events[i].c16);
			FTS_DEBUG("[B]P%d UP!", events[i].c16);
		}
	}

	/* "6b1a011f cmp"+"54000260 b.eq"@0xffffff8008a800c0/c0 salta tutto il
	 * ciclo quando i due valori coincidono; il "cbz w24" di
	 * 0xffffff8008a800a8 e' la guardia del ciclo, che clang ha issato sopra. */
	if (data->c348 ^ touchs) {
		for (i = 0; i < max_touch_num; i++) {
			/* "93407d08 sxtw"@0xffffff8008a7fbdc: lo xor a 32 bit e'
			 * esteso CON SEGNO a 64 prima del tst, cioe' BIT() e' a
			 * 64 bit e l'altro operando e' un int. */
			if (BIT(i) & (data->c348 ^ touchs)) {
				va_reported = true;
				FTS_DEBUG("[B]P%d UP!", i);
				input_mt_slot(tpd->dev, i);
				input_mt_report_slot_state(tpd->dev, MT_TOOL_FINGER, false);
			}
		}
	}
	data->c348 = touchs;

	if (va_reported) {
		if (!touchs || !data->c360) {
			FTS_DEBUG("[B]Points All Up!");
			input_report_key(tpd->dev, BTN_TOUCH, 0);
		} else {
			input_report_key(tpd->dev, BTN_TOUCH, 1);
		}
		input_sync(tpd->dev);
	}

	return 0;
}

/*
 * touch_event_handler @0xffffff8008a7fb48, 1676 byte, visibilita' t.
 * Il thread creato da tpd_probe ("912d2000 add"@0xffffff8008a7f7a8 mette in x0
 * proprio 0xffffff8008a7fb48).
 */
static int touch_event_handler(void *unused)
{
	/* "321e03e8 orr"@0xffffff8008a7fb88 -> w8 = 4, scritto a sp+0x40;
	 * "321f03e1 orr"@0xffffff8008a7fb78 -> w1 = 2 = SCHED_RR. */
	struct sched_param param = { .sched_priority = 4 };
	struct fts_ts_data *ts_data = fts_data;
	int ret = 0;

	sched_setscheduler(current, SCHED_RR, &param);

	do {
		/* "d5033bbf dmb"@0xffffff8008a7fc60 -- set_current_state e' una
		 * smp_store_mb, cioe' WRITE_ONCE + barriera. */
		set_current_state(TASK_INTERRUPTIBLE);
		wait_event_interruptible(fts_g_998ca38, fts_g_a100aec != 0);
		fts_g_a100aec = 0;
		set_current_state(TASK_RUNNING);

		FTS_DEBUG("touch_event_handler start");

		ret = fts_read_parse_touchdata(ts_data);
		if (ret == 0) {
			mutex_lock(&ts_data->report_mutex);
			fts_input_report_b(ts_data);
			mutex_unlock(&ts_data->report_mutex);
		}
		/* "97d961bc bl"@0xffffff8008a80188 -> kthread_should_stop */
	} while (!kthread_should_stop());

	return 0;
}

/*
 * tpd_eint_interrupt_handler @0xffffff8008a801d4, 56 byte, visibilita' t.
 * Passata a request_threaded_irq da tpd_probe ("91075021 add"@0xffffff8008a7f86c).
 */
static irqreturn_t tpd_eint_interrupt_handler(int irq, void *dev_id)
{
	/* "392bb109 strb"@0xffffff8008a801f8 -- scrittura RISTRETTA a un byte */
	fts_g_a100aec = 1;
	/* "320003e1 orr"+"320003e2 orr"+"aa1f03e3 mov"@0xffffff8008a801f4/f0/f4
	 * -> __wake_up(q, 1, 1, NULL) = wake_up_interruptible */
	wake_up_interruptible(&fts_g_998ca38);
	/* "320003e0 orr"@0xffffff8008a80200 -> IRQ_HANDLED */
	return IRQ_HANDLED;
}

/*
 * fts_get_ic_information -- INCORPORATA in tpd_probe.
 * Come le due di sopra non ha simbolo, e come quelle la denuncia il fatto che
 * ts_data->c0 sia RICARICATO in un registro salvato prima del ciclo
 * ("f9400296 ldr"@0xffffff8008a7f510) invece di riusare il parametro `client`
 * che tpd_probe tiene gia' in x21: e' il locale di un corpo separato.
 */
static int fts_get_ic_information(struct fts_ts_data *ts_data)
{
	int ret = 0;
	int cnt = 0;
	u8 chip_id[2] = { 0 };
	u8 id_cmd[4] = { 0 };
	u32 id_cmd_len = 0;
	struct i2c_client *client = ts_data->c0;

	/* "79002298 strh"@0xffffff8008a7f514 scrive DUE byte con il valore 1:
	 * c16 = 1 e c17 = 0, fusi da clang in una sola strh. */
	ts_data->c16 = 1;
	ts_data->c17 = 0;

	for (cnt = 0; cnt < TIMEOUT_READ_REG; cnt += INTERVAL_READ_REG) {
		ret = fts_i2c_read_reg(client, FTS_REG_CHIP_ID, &chip_id[0]);
		ret = fts_i2c_read_reg(client, FTS_REG_CHIP_ID2, &chip_id[1]);
		if ((ret < 0) || (!chip_id[0]) || (!chip_id[1])) {
			FTS_DEBUG("i2c read invalid, read:0x%02x%02x",
				  chip_id[0], chip_id[1]);
		} else {
			FTS_DEBUG("verify id:0x%02x%02x", chip_id[0], chip_id[1]);
			if ((chip_id[0] == 0x87) && (chip_id[1] == 0x19)) {
				ts_data->c24 = fts_ctype;
				break;
			}
			FTS_DEBUG("TP not ready, read:0x%02x%02x",
				  chip_id[0], chip_id[1]);
		}
		msleep(INTERVAL_READ_REG);
	}

	/* "710f9e7f cmp"@0xffffff8008a7f5cc -> 0x3e7 = 999, cioe' `cnt >= 1000` */
	if (cnt >= TIMEOUT_READ_REG) {
		FTS_INFO("fw is invalid, need read boot id");
		if (ts_data->c17)
			fts_i2c_hid2std(client);

		/* "52954aa8 mov"@0xffffff8008a7f5f8 -> 0xaa55, scritto con una
		 * sola strh: i due byte sono assegnati di seguito. */
		id_cmd[0] = 0x55;
		id_cmd[1] = 0xaa;
		ret = fts_i2c_write(client, id_cmd, 2);
		if (ret < 0) {
			FTS_ERROR("start cmd write fail");
			return ret;
		}
		msleep(10);

		/* "52801209 mov"+"b90043e9 str"@0xffffff8008a7f640/40: 0x90 e poi
		 * tre zeri, scritti come una `str w` da 4 byte. */
		id_cmd[0] = 0x90;
		id_cmd[1] = 0x00;
		id_cmd[2] = 0x00;
		id_cmd[3] = 0x00;
		/* LA CONDIZIONE E' DECODIFICATA, NON PARAFRASATA.
		 * "39404288 ldrb"+"7100011f cmp"+"321e03e8 orr"+"1a9f0502 csinc"
		 * @0xffffff8008a7f61c/2c/30/34, e la stessa csinc scritta anche
		 * nella forma che porta gli OPERANDI e si verifica ALL'INDIRIZZO:
		 * "csinc w2, w8, wzr, eq"@0xffffff8008a7f634.
		 *
		 * CSINC Wd,Wn,Wm,cond = cond ? Wn : Wm+1.  Qui Wn = w8 = 4 (lo
		 * mette l'`orr` immediatamente prima), Wm = wzr, e la condizione
		 * `eq` viene dal `cmp w8,#0x0` che confronta c16, appena caricato
		 * dalla `ldrb`.  La fabbrica calcola dunque
		 *     (c16 == 0) ? 4 : 1
		 * e NON (c16 != 0) ? 4 : 1.
		 *
		 * CORREZIONE del 2026-08-21 (revisione indipendente).  Il test era
		 * scritto invertito, e il commento accanto -- «-> c16 ? 4 : 1» --
		 * era la parafrasi sbagliata che l'aveva generato.  La citazione
		 * era GIUSTA e portava gia' la prova del contrario, ma era "nuda"
		 * (codifica + mnemonico, senza operandi), e verificaistruzioni.py
		 * non ha operandi da confrontare su una citazione nuda: la
		 * confermava.  L'inversione costa ZERO byte -- tpd_probe, che
		 * incorpora questa funzione, misura 2000 byte prima e 2000 dopo --
		 * quindi nessuna misura di dimensione poteva denunciarla.
		 * Con il test scritto al contrario il nostro clang emetteva
		 * "1a9f1502 csinc", cioe' la condizione `ne`, che nei tre
		 * intervalli di fabbrica non esiste: e' dichiarata `--controfattuale`
		 * nel §2, cosi' lo strumento verifica che sia ASSENTE.
		 *
		 * La differenza non e' teorica: fts_get_ic_information mette
		 * c16 = 1 in testa ("79002298 strh"@0xffffff8008a7f514), quindi con
		 * il test invertito la fts_i2c_read del boot id partiva con
		 * writelen 4 dove la fabbrica usa 1. */
		if (!ts_data->c16)
			id_cmd_len = 4;
		else
			id_cmd_len = 1;
		ret = fts_i2c_read(client, id_cmd, id_cmd_len, chip_id, 2);
		if ((ret < 0) || (!chip_id[0]) || (!chip_id[1])) {
			FTS_ERROR("read boot id fail");
			return -EIO;
		}
		FTS_DEBUG("verify id:0x%02x%02x", chip_id[0], chip_id[1]);
		if ((chip_id[0] == 0x87) &&
		    ((chip_id[1] == 0x19) || (chip_id[1] == 0xb9) ||
		     (chip_id[1] == 0xa9))) {
			ts_data->c24 = fts_ctype;
		} else {
			FTS_ERROR("can't get ic informaton");
			/* "321a6ff7 orr"@0xffffff8008a7f950 -> 0xffffffc3 = -61 */
			return -ENODATA;
		}
	}

	FTS_INFO("get ic information, chip id = 0x%02x%02x",
		 ts_data->c24.c8, ts_data->c24.c9);
	return 0;
}

/*
 * tpd_probe @0xffffff8008a7f304, 1936 byte, visibilita' t.
 * DUE definizioni di questo nome nella mappa: l'altra e' quella di mtk_tpd.c
 * a 0xffffff8008a515e8.  Questa e' la nostra, e non e' una deduzione per
 * vicinanza: stampa "[FTS][TPD]Touch Panel Device Probe %s!\n"@0xffffff800924f8ff,
 * scrive il puntatore fts_data e chiama fts_i2c_init.
 *
 * Incorpora fts_input_init, fts_reset_proc e fts_get_ic_information, in
 * quest'ordine.
 */
static int tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int retval = 0;
	int ret = 0;
	int i = 0;
	int sensor_id = 0;
	u8 fwver = 0;
	char *name = NULL;
	char module_name[FTS_MODULE_NAME_LEN];
	char tp_info[FTS_TP_INFO_LEN];
	struct device_node *node = NULL;
	struct fts_ts_data *ts_data = NULL;

	FTS_FUNC_ENTER();

	/* "321907e1 orr"@0xffffff8008a7f354 -> 0x180 = 384 byte;
	 * "52901802 mov"+"72a02802 movk"@0xffffff8008a7f358/58 -> 0x014080c0 =
	 * GFP_KERNEL | __GFP_ZERO, cioe' devm_kzalloc. */
	ts_data = devm_kzalloc(&client->dev, sizeof(*ts_data), GFP_KERNEL);
	if (!ts_data) {
		FTS_ERROR("Failed to allocate memory for fts_data");
		/* "12800177 mov"@0xffffff8008a7f6b8 -> -12 = -ENOMEM */
		return -ENOMEM;
	}

	ts_data->c0 = client;
	fts_data = ts_data;
	/* "f9400d08 ldr"@0xffffff8008a7f37c -> tpd->dev, offset 24 di tpd_device */
	ts_data->c8 = tpd->dev;
	/* "790006b6 strh"@0xffffff8008a7f390 -> client->addr = 0x38 */
	client->addr = 0x38;
	fts_g_a100ae0 = client;
	/* Il confronto e' fatto DOPO l'assegnazione, quindi non e' mai vero; il
	 * carico "794006a1 ldrh"@0xffffff8008a7f398 c'e' lo stesso perche' il
	 * kernel compila con -fno-strict-aliasing e la `str` del globale
	 * potrebbe, per il compilatore, sovrapporsi.  E' un difetto di
	 * fabbrica e si riproduce (regola 7). */
	if (client->addr != 0x38) {
		FTS_INFO("[TPD]Change i2c addr 0x%02x to %x", client->addr, 0x38);
		client->addr = 0x38;
		FTS_INFO("[TPD]i2c addr=0x%x\n", client->addr);
	}

	/* "97d9473d bl"@0xffffff8008a7f3ec ->
	 * __alloc_workqueue_key("%s", 0xe000a, 1, NULL, NULL, "fts_wq"), cioe'
	 * esattamente l'espansione di create_singlethread_workqueue.
	 * CORREZIONE del 2026-08-21: questa riga cominciava con un inizio di
	 * frase abortito, rimasto nel file consegnato, che citava la codifica
	 * `97e9ca40` e si interrompeva a meta' con un «... no:».  E' la classe B7
	 * (detrito editoriale).  Quella codifica non e' inventata -- ed e'
	 * proprio per questo che nessuno strumento protestava: e' la
	 * "97e9ca40 bl"@0xffffff8008a7f360, cioe' la `bl devm_kmalloc` del
	 * contesto, che con la create_singlethread_workqueue non c'entra
	 * niente. */
	ts_data->c40 = create_singlethread_workqueue("fts_wq");
	if (!ts_data->c40)
		FTS_ERROR("failed to create fts workqueue");

	spin_lock_init(&ts_data->c280);
	mutex_init(&ts_data->report_mutex);

	fts_i2c_init();

	ret = fts_input_init(client);
	if (ret) {
		FTS_ERROR("fts input initialize fail");
		retval = -ENOMEM;
		goto err_wq;
	}

	/* "52801900 mov"@0xffffff8008a7f4f8 -> 200 ms */
	fts_reset_proc(200);

	ret = fts_get_ic_information(ts_data);
	if (ret) {
		FTS_ERROR("not focal IC, unregister driver");
		retval = ret;
		goto err_wq;
	}

	ret = fts_create_apk_debug_channel(ts_data);
	if (ret)
		FTS_ERROR("create apk debug node fail");

	ret = fts_create_sysfs(client);
	if (ret)
		FTS_ERROR("create sysfs node fail");

	ret = fts_ex_mode_init(client);
	if (ret)
		FTS_ERROR("init glove/cover/charger fail");

	/* "97d964d3 bl"@0xffffff8008a7f7b8 kthread_create_on_node con nodo -1 e
	 * "97d9b159 bl"@0xffffff8008a7f7e4 wake_up_process solo se non e' un
	 * ERR_PTR: e' l'espansione di kthread_run. */
	ts_data->c272 = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	if (IS_ERR(ts_data->c272)) {
		/* "2a1703e1 mov"@0xffffff8008a7f7d0 passa i 32 bit BASSI del
		 * puntatore d'errore: nel sorgente c'e' un int, non un long. */
		retval = PTR_ERR(ts_data->c272);
		FTS_ERROR("[TPD]Failed to create kernel thread_tpd,ret:%d", retval);
		ts_data->c272 = NULL;
		goto err_wq;
	}

	/* "9a88b121 csel"@0xffffff8008a7f800 sceglie fra "FAIL"@0xffffff8009249687
	 * e "PASS"@0xffffff800924966f con la condizione `lt` su ret, che qui e'
	 * ancora quello di fts_ex_mode_init. */
	FTS_DEBUG("[TPD]Touch Panel Device Probe %s!", (ret < 0) ? "FAIL" : "PASS");

	tpd_gpio_as_int(fts_g_998c910);

	node = of_find_matching_node_and_match(NULL, touch_of_match, NULL);
	if (node) {
		ts_data->c320 = irq_of_parse_and_map(node, 0);
		client->irq = ts_data->c320;
		FTS_INFO("IRQ request succussfully, irq:%d client:%d",
			 ts_data->c320, client->irq);
		/* "321f03e3 orr"@0xffffff8008a7f874 -> flags = 2 =
		 * IRQF_TRIGGER_FALLING; il thread_fn e il dev_id sono NULL. */
		retval = request_threaded_irq(ts_data->c320,
					      tpd_eint_interrupt_handler, NULL,
					      IRQF_TRIGGER_FALLING,
					      "TOUCH_PANEL-eint", NULL);
	} else {
		FTS_ERROR("Can not find touch eint device node!");
		retval = -ENODATA;
	}
	if (retval != 0) {
		FTS_ERROR("request irq failed");
		goto err_irq;
	}

	/* 0xffffff800a0fc954: lo LEGGE il tpd_probe di mtk_tpd.c e lo scrivono i
	 * tre driver touch, cioe' e' tpd_load_status. */
	tpd_load_status = 1;
	/* TPD_RES_Y e' letta a 32 bit ("b942b901 ldr"@0xffffff8008a7f890)
	 * mentre tpd.h la dichiara `unsigned long`: vedi DIVERGENZA 2. */
	FTS_DEBUG("TPD_RES_Y:%d", (u32)TPD_RES_Y);

	sensor_id = get_module_id();
	/* Questo NON e' un FTS_*: il formato non ha ne' livello ne' prefisso.
	 * "yang.liu*****log***sensor_id=%d\n"@0xffffff800924f95c */
	printk("yang.liu*****log***sensor_id=%d\n", sensor_id);

	name = fts_g_998ca50[sensor_id];
	if (name)
		strcpy(module_name, name);
	else
		strcpy(module_name, "unknown");

	/* "128000b3 mov"@0xffffff8008a7fa00 parte da -6 e
	 * "31000673 adds"+"540001a2 b.cs"@0xffffff8008a7fa08/08 escono al
	 * riporto: il corpo gira CINQUE volte.
	 *
	 * E LA FABBRICA IL CICLO LO TIENE: la pragma serve a non farlo
	 * srotolare, come in `get_module_id` di questo stesso file e in
	 * `squareRoot` di mir3da. Senza, clang lo srotola -- lo dice lui:
	 *   <riga di comando vera> -Rpass=loop-unroll
	 *   focaltech_core.c:1363:2: remark: completely unrolled loop with 5
	 *       iterations
	 */
#pragma clang loop unroll(disable)
	for (i = 0; i < 5; i++) {
		fts_i2c_read_reg(client, FTS_REG_FW_VER, &fwver);
		/* "528000a0 mov"@0xffffff8008a7fa1c -> 5 ms */
		msleep(5);
		if ((fwver == 0x05) || (fwver == 0x03))
			break;
	}

	/* "940fab45 bl"@0xffffff8008a7fa60 sprintf, con SEI argomenti: il quinto
	 * e il sesto sono lo stesso fwver ("2a0503e6 mov"@0xffffff8008a7fa5c). */
	ret = sprintf(tp_info,
		      "TP IC :%s,TP module :%s,TP I2C adr : 0x%x,SW FirmWare:0x%x,Sample FirmWare:0x%x",
		      "ft8719", module_name, client->addr, fwver, fwver);
	fix_tp_proc_info(tp_info, ret);

	FTS_FUNC_EXIT();
	return 0;

err_irq:
	if (ts_data->c272) {
		kthread_stop(ts_data->c272);
		ts_data->c272 = NULL;
	}
err_wq:
	if (ts_data->c40)
		destroy_workqueue(ts_data->c40);
	kfree_safe(ts_data->c336);
	kfree_safe(ts_data->c328);
	devm_kfree(&client->dev, ts_data);
	FTS_FUNC_EXIT();
	return retval;
}

/*
 * tpd_remove @0xffffff8008a7fa94, 152 byte, visibilita' t.
 * "f9405c13 ldr"@0xffffff8008a7faa0 legge client + 184: e'
 * i2c_get_clientdata, cioe' client->dev.driver_data.  tpd_probe NON chiama
 * mai i2c_set_clientdata, quindi qui il puntatore e' quello che il core i2c
 * ci ha lasciato.  E' un difetto di fabbrica e si riproduce (regola 7).
 */
static int tpd_remove(struct i2c_client *client)
{
	struct fts_ts_data *ts_data = i2c_get_clientdata(client);

	FTS_FUNC_ENTER();
	fts_remove_sysfs(client);
	fts_ex_mode_exit(client);
	fts_release_apk_debug_channel(ts_data);
	fts_i2c_exit();
	kfree_safe(ts_data->c336);
	kfree_safe(ts_data->c328);
	if (ts_data->c40)
		destroy_workqueue(ts_data->c40);
	FTS_FUNC_EXIT();
	return 0;
}

/*
 * tpd_i2c_detect @0xffffff8008a7fb2c, 28 byte, visibilita' t.
 * Gli otto byte scritti in [x1] sono "mtk-tpd\0", materializzati con un
 * mov+tre movk: "d28e8da8 mov"@0xffffff8008a7fb2c = 0x746d = "mt".
 */
static int tpd_i2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	strcpy(info->type, TPD_DEVICE);
	return 0;
}

/*
 * tpd_local_init @0xffffff8008a7ef40, 176 byte, visibilita' t.
 * TRE definizioni di questo nome nella mappa (ilitek 0xffffff8008a56590,
 * goodix 0xffffff8008a775bc, focaltech 0xffffff8008a7ef40): questa e' la
 * terza, ed e' il campo +0x08 del tpd_driver_t "fts_ts".
 */
static int tpd_local_init(void)
{
	FTS_FUNC_ENTER();
	/* "aa1f03e0 mov"@0xffffff8008a7ef64 mette x0 = NULL (THIS_MODULE di un
	 * built-in) e "94002d94 bl"@0xffffff8008a7ef68 chiama
	 * i2c_register_driver: e' i2c_add_driver. */
	if (i2c_add_driver(&tpd_i2c_driver) != 0) {
		FTS_ERROR("[TPD]: Unable to add fts i2c driver!!");
		FTS_FUNC_EXIT();
		/* "12800000 mov"@0xffffff8008a7ef94 -> -1, non un -Exxx */
		return -1;
	}

	/* "b94f9508 ldr"@0xffffff8008a7efa4 legge tpd_dts_data + 12;
	 * "b8404420 ldr"@0xffffff8008a7efb8 legge +16 e post-incrementa,
	 * "91007022 add"@0xffffff8008a7efb4 fa +16+28 = +44.  Sono
	 * use_tpd_button, tpd_key_num e tpd_key_dim_local di struct
	 * tpd_dts_info, cioe' la base e' 0xffffff800a0fbf88. */
	if (tpd_dts_data.use_tpd_button) {
		tpd_button_setting(tpd_dts_data.tpd_key_num,
				   tpd_dts_data.tpd_key_local,
				   tpd_dts_data.tpd_key_dim_local);
	}

	/* 0xffffff800a0fc938.  Lo scrivono SOLO i tre tpd_local_init e nessuno
	 * lo legge in tutta l'immagine.  L'identificazione e' PLAUSIBILE, non
	 * certa: nel nostro System.map tpd_type_cap sta a 0x1c byte esatti prima
	 * di tpd_load_status, la stessa distanza che c'e' fra 0xa0fc938 e
	 * 0xa0fc954 di fabbrica, e le due variabili vengono dallo stesso file
	 * (tpd_setting.c), quindi la loro distanza e' una distanza INTERNA a un
	 * oggetto.
	 *
	 * CORREZIONE del 2026-08-21: qui seguiva «Controprova indipendente:
	 * `tpd` e `tpd_load_status` distano 0x694 in tutti e due i kernel»,
	 * presentata come invariante.  NON e' un'invariante, ed e' la classe C3
	 * (prova costruita su un campione invece che su tutti i casi
	 * disponibili).  Sui 26 System.map presenti sul PC di build, 25 danno
	 * tpd_type_cap 0x1c byte PRIMA di tpd_load_status e tpd->tpd_load_status
	 * = 0x694; uno, out-st54spi-impl, ha l'ordine ROVESCIATO e un'altra
	 * distanza:
	 *   out-avvio/System.map  tpd=0xffffff800a017360 type_cap=0xffffff800a0179d8 load_status=0xffffff800a0179f4  cap->load=+28 (0x1c)  tpd->load=1684 (0x694)
	 *   out-st54spi-impl/System.map  tpd=0xffffff800a0aced8 type_cap=0xffffff800a0ad668 load_status=0xffffff800a0ad648  cap->load=-32 (0x20)  tpd->load=1904 (0x770)
	 * La distanza dipende dall'ordine di link, che il .config cambia.
	 * L'identificazione resta PLAUSIBILE, e la regge l'argomento del file
	 * comune (le due variabili nascono in tpd_setting.c, quindi la loro
	 * distanza e' interna a un oggetto e sopravvive al link); la
	 * "controprova indipendente" invece non c'e', ed e' stata tolta. */
	tpd_type_cap = 1;
	FTS_FUNC_EXIT();
	return 0;
}

/*
 * tpd_suspend @0xffffff8008a7eff0, 280 byte, visibilita' t.
 * Incorpora fts_irq_disable (il __func__ "fts_irq_disable"@0xffffff800924f67c
 * compare a "97dad11e bl"@0xffffff8008a7f05c, dentro tpd_suspend).
 */
static void tpd_suspend(struct device *h)
{
	int ret = 0;
	struct fts_ts_data *ts_data = fts_data;

	FTS_FUNC_ENTER();
	/* "39451288 ldrb"@0xffffff8008a7f01c -> +324 */
	if (ts_data->c324) {
		FTS_INFO("Already in suspend state");
		return;
	}
	/* "39451688 ldrb"@0xffffff8008a7f034 -> +325 */
	if (ts_data->c325) {
		FTS_INFO("fw upgrade in process, can't suspend");
		return;
	}

	fts_irq_disable();

	/* "320007e2 orr"@0xffffff8008a7f0bc -> il valore 3 nel registro 0xa5 */
	ret = fts_i2c_write_reg(ts_data->c0, FTS_REG_POWER_MODE, 3);
	if (ret < 0)
		FTS_ERROR("Set TP to sleep mode fail, ret=%d!", ret);

	ts_data->c324 = 1;
	FTS_FUNC_EXIT();
}

/*
 * tpd_resume @0xffffff8008a7f108, 508 byte, visibilita' t.
 * Incorpora fts_release_all_finger, fts_reset_proc e fts_irq_enable, in
 * quest'ordine (i tre __func__ si susseguono a 0xffffff8008a7f158,
 * 0xffffff8008a7f214 e 0xffffff8008a7f270).
 */
static void tpd_resume(struct device *h)
{
	struct fts_ts_data *ts_data = fts_data;

	FTS_FUNC_ENTER();
	/* "34000d48 cbz"@0xffffff8008a7f13c: se non e' sospeso, esce subito */
	if (!ts_data->c324) {
		FTS_DEBUG("Already in awake state");
		return;
	}

	fts_release_all_finger();

	/* "394042a8 ldrb"+"350002e8 cbnz"@0xffffff8008a7f1fc/fc: il reset si fa
	 * solo se c16 e' ZERO, e tpd_probe lo mette a 1, quindi non si fa mai.
	 * Si riproduce com'e'. */
	if (!ts_data->c16)
		fts_reset_proc(200);

	fts_tp_state_recovery(ts_data->c0);
	fts_irq_enable();
	ts_data->c324 = 0;
	FTS_FUNC_EXIT();
}

/*
 * tpd_driver_init @0xffffff8009383470, 168 byte, in .init.text.
 * DUE definizioni: l'altra e' quella del goodix a 0xffffff8009383418.
 * Questa e' la nostra perche' stampa "Focaltech V2.2 20180321" e passa a
 * tpd_driver_add il tpd_driver_t "fts_ts" (0xffffff800998c918).
 */
static int __init tpd_driver_init(void)
{
	FTS_FUNC_ENTER();
	FTS_INFO("Driver version: %s", FTS_DRIVER_VERSION);
	/* "97db35c4 bl"@0xffffff80093834a0 */
	tpd_get_dts_info();
	/* "7100083f cmp"@0xffffff80093834ac -> 2;
	 * "71002c3f cmp"@0xffffff80093834bc -> 0xb, cioe' `> 10`.
	 * L'argomento della FTS_INFO che segue e' il registro w1 gia' caricato:
	 * "913a3400 add"@0xffffff80093834d0 mette solo il formato in x0. */
	if (tpd_dts_data.touch_max_num < 2)
		tpd_dts_data.touch_max_num = 2;
	else if (tpd_dts_data.touch_max_num > 10)
		tpd_dts_data.touch_max_num = 10;
	FTS_INFO("tpd max touch num:%d", tpd_dts_data.touch_max_num);

	/* "36f80080 tbz"@0xffffff80093834e4 -> il test e' `< 0` */
	if (tpd_driver_add(&tpd_device_driver) < 0)
		FTS_ERROR("[TPD]: Add FTS Touch driver failed!!");

	FTS_FUNC_EXIT();
	return 0;
}

/*
 * tpd_driver_exit @0xffffff80093acb2c, 80 byte, in .exit.text.
 * NON HA SIMBOLO nella mappa, e non e' una svista: `oracolo/stock.map`
 * finisce a `_einittext = 0xffffff80093a8518`, quindi tutta la .exit.text e'
 * invisibile a clusterdriver.py, cluster_finale.py, classificasimboli.py,
 * disassembla.py e misurablocco.py, che partono tutti dalla mappa.
 * Che sia questa e non la gemella goodix (subito prima, stesso __func__) lo
 * dice l'argomento di tpd_driver_remove: "91246000 add"@0xffffff80093acb54
 * mette in x0 0xffffff800998c918, il tpd_driver_t "fts_ts".
 * La riga e' 1396: "5280ae82 mov"@0xffffff80093acb64.
 */
static void __exit tpd_driver_exit(void)
{
	FTS_FUNC_ENTER();
	tpd_driver_remove(&tpd_device_driver);
	FTS_FUNC_EXIT();
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

MODULE_LICENSE("GPL");

/*
 * ===========================================================================
 * LA MISURA, E L'ATTRIBUZIONE DI OGNI DIVERGENZA
 * ===========================================================================
 * Misurata dal `.o` VERO, con
 *   aarch64-linux-android-nm --print-size \
 *     out-ft8719/drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_core.o
 *
 *   funzione                     fabbrica  nostro  scarto
 *   fts_wait_tp_to_valid              196     196       0
 *   fts_tp_state_recovery             240     236      -4
 *   fts_reset_proc                    124     124       0
 *   fts_irq_disable                   132     132       0
 *   fts_irq_enable                    128     128       0
 *   get_module_id                     332     400     +68
 *   tpd_local_init                    176     164     -12
 *   tpd_suspend                       280     280       0
 *   tpd_resume                        508     508       0
 *   tpd_probe                        1936    2000     +64
 *   tpd_remove                        152     152       0
 *   tpd_i2c_detect                     28      28       0
 *   touch_event_handler              1676    1572    -104
 *   tpd_eint_interrupt_handler         56      56       0
 *   tpd_driver_init                   168     168       0
 *   tpd_driver_exit                    80      80       0
 *   TOTALE                           6212    6224     +12
 *
 * AVVERTENZA PRIMA DELLA PERCENTUALE: su sedici unita' la misura NON
 * discrimina.  `./venv/bin/python3 intervallo.py 11 16` dice
 *   11 su 16 = 68.8%   IC95% Clopper-Pearson [41.3%; 89.0%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *
 * ---------------------------------------------------------------------------
 * LA CAUSA COMUNE: IL COMPILATORE DI FABBRICA NON E' IL NOSTRO
 * ---------------------------------------------------------------------------
 * Il kernel di fabbrica lo dice da solo, nel proprio banner:
 *
 *   "Linux version 4.14.141 (nobody@android-build) (Android (5484270 based on
 *    r353983c) clang version 9.0.3 (...) (based on LLVM 9.0.3svn)) #1 SMP
 *    PREEMPT Tue Jun 23 09:55:04 CST 2020"
 *
 *   $ strings -a oracolo/stock.elf | grep -m1 "Linux version 4"
 *
 * Il nostro albero compila con clang-r383902, cioe':
 *
 *   $ clang --version
 *   Android (6443078 based on r383902) clang version 11.0.1
 *
 * Due versioni maggiori di LLVM di distanza.  Le cinque divergenze qui sotto
 * sono TUTTE di quella famiglia, e due di esse sono provate da un
 * esperimento, non asserite.
 *
 * DIVERGENZA 1 -- get_module_id, +68 byte (17 istruzioni).  PROVATA.
 *   Il nostro clang SROTOLA completamente il ciclo di ritentativo a cinque
 *   giri ("7100111f cmp"@0xffffff8008a7ee78 -- di fabbrica il ciclo resta
 *   arrotolato); il ciclo ha due uscite (il contatore e `ret == 1`) e LLVM 9
 *   non srotolava i cicli a uscita multipla.
 *   Prova: ricompilando lo stesso sorgente con -fno-unroll-loops,
 *   get_module_id misura 0x14c = 332 byte, cioe' ESATTAMENTE la fabbrica.
 *
 * DIVERGENZA 2 -- tpd_probe, +64 byte, che sono +148 e -84.  PROVATA a meta'.
 *   Con -fno-unroll-loops tpd_probe misura 0x73c = 1852: lo srotolamento del
 *   ciclo a cinque giri della versione firmware ("128000b3 mov"@0xffffff8008a7fa00)
 *   vale quindi ESATTAMENTE 148 byte, e il residuo e' -84 (21 istruzioni).
 *   Il residuo, dall'istogramma dei mnemonici (fabbrica contro nostro):
 *     - `bl printk` 34 -> 32.  Le altre TRENTA destinazioni di chiamata
 *       coincidono una per una, nomi e conteggi.  Il nostro clang fonde in un
 *       solo sito le tre stampe d'errore "read boot id fail" /
 *       "start cmd write fail" / "can't get ic informaton" e tre rami del
 *       ciclo del chip id; quello di fabbrica ne fonde due soli.
 *     - disposizione della cornice: LLVM 11 mette x29/x30 in FONDO all'area
 *       dei registri salvati ("mov x29, sp") e indirizza i locali con
 *       `sub`+`ldur` da x29; LLVM 9 li mette in cima
 *       ("910143fd add"@0xffffff8008a7f31c, cioe' `add x29, sp, #0x50`) e
 *       usa `add`+`ldrb` da sp.  (Fino al 2026-08-21 l'indirizzo dopo la @
 *       era scritto letteralmente `...`: classe B7.)
 *       Istogramma: ldrb 12->3, ldurb 0->8, ldur 0->1, sturh 0->1,
 *       sub 1->7, add 76->66.
 *     - `orr wN, wzr, #imm` contro `mov wN, #imm`: 17 occorrenze, stessa
 *       taglia, si annullano nel conto dei byte.
 *
 * DIVERGENZA 3 -- touch_event_handler, -104 byte (26 istruzioni).
 *   Anche qui le destinazioni di chiamata coincidono una per una (input_event
 *   8/8, input_mt_report_slot_state 3/3, tpd_button 2/2, ...), tranne:
 *     - `bl printk` 12 -> 11 (una fusione in piu');
 *     - `__memset` di fabbrica contro `memset` nostro: e' la stessa chiamata
 *       sotto due nomi (alias di arch/arm64), non una riga in meno.
 *   Il resto e' allocazione dei registri e materializzazione degli indirizzi:
 *     - adrp 30 -> 23: il nostro clang materializza meno spesso le pagine
 *       dei globali.  CORREZIONE del 2026-08-21: qui c'era scritto che la
 *       fabbrica «rimaterializza a ogni uso» la pagina di `tpd`, con una
 *       citazione che accoppiava le codifiche `9000b3e8` e `f9416108` agli
 *       indirizzi 0xffffff8008a7fbf8 e 0xffffff8008a7fbfc.  (Le codifiche
 *       sono scritte qui NUDE, senza il mnemonico accanto, apposta: scritte
 *       in forma di citazione rimetterebbero in circolo l'errore, e la
 *       forma a codifica non controlla l'indirizzo -- e' il punto.)
 *       Erano sbagliate tutte e due le cose, ed e' la classe B8.
 *       (a) LA CITAZIONE.  A 0xffffff8008a7fbf8 la fabbrica ha
 *           "b000b3e8 adrp"@0xffffff8008a7fbf8, non 9000b3e8; a
 *           0xffffff8008a7fbfc c'e' "aa0803f4 mov"@0xffffff8008a7fbfc, e la
 *           "f9416108 ldr"@0xffffff8008a7fc00 sta un'istruzione piu' in la'.
 *           La codifica `9000b3e8` esiste nel blocco (a 0xffffff8008a7fcdc e
 *           0xffffff8008a7fec8) ma e' l'adrp di un'ALTRA pagina,
 *           0xffffff800a0fb000: ecco perche' verificaistruzioni.py la
 *           confermava lo stesso.  La forma a codifica si cerca OVUNQUE nel
 *           blocco e l'indirizzo dopo la @ non viene mai confrontato -- e'
 *           un DELTA DI STRUMENTO, scritto in coda a questo file.
 *       (b) L'AFFERMAZIONE.  L'istruzione vera a 0xffffff8008a7fbfc dice il
 *           contrario di cio' che la citazione doveva sostenere:
 *           "aa0803f4 mov"@0xffffff8008a7fbfc e' `mov x20, x8`, cioe' la
 *           fabbrica SALVA quella pagina in x20, che e' callee-saved.  Le
 *           adrp della pagina 0xffffff800a0fc000 dentro touch_event_handler
 *           di fabbrica sono QUATTRO su 30, non «a ogni uso»:
 *             $ ./venv/bin/python3 corr-ft8719/adrp.py touch_event_handler
 *             istruzioni touch_event_handler: 419
 *             adrp totali: 30
 *               ffffff800924f000 14
 *               ffffff800a100000 4
 *               ffffff800a0fc000 4
 *               ffffff800a0fb000 3
 *               ffffff800985e000 2
 *               ffffff800998c000 2
 *               ffffff800996b000 1
 *           I -104 byte restano quelli misurati; e' la loro ATTRIBUZIONE a
 *           questa causa che era piu' forte dell'evidenza.
 *     - ldr 61 -> 56, str 18 -> 15: di fabbrica TPD_RES_Y viene versata sullo
 *       stack ("b90017e8 str"@0xffffff8008a7fec4) e riletta dentro il ciclo
 *       ("b94017e9 ldr"@0xffffff8008a7fedc); da noi resta in w23;
 *     - add+sub 45 -> 39: la riduzione di forza sceglie base = 6*i da noi e
 *       base = 8+6*i di fabbrica ("321d03ea orr"@0xffffff8008a7fdc4 mette
 *       w10 = 8 e "1100194a add"@0xffffff8008a7fe74 lo incrementa di 6).
 *     - orr -> mov: 27 occorrenze, stessa taglia.
 *
 * DIVERGENZA 4 -- tpd_local_init, -12 byte (3 istruzioni).
 *   Il nostro clang FONDE le due code `FTS_FUNC_EXIT(); return X;` in una
 *   sola (passa il valore di ritorno per w19, che costa il salvataggio di un
 *   registro ma risparmia adrp+add+bl+ldp+ret duplicati); quello di fabbrica
 *   le tiene separate, e infatti la fabbrica ha DUE `ret`
 *   ("d65f03c0 ret"@0xffffff8008a7ef9c e @0xffffff8008a7efec) mentre noi ne
 *   abbiamo uno.  Istogramma: ret 2->1, bl 6->5, ldp 2->1.
 *
 * DIVERGENZA 5 -- fts_tp_state_recovery, -4 byte (1 istruzione).
 *   Nella copia INCORPORATA di fts_wait_tp_to_valid il nostro clang converte
 *   il contatore in un contatore all'ingiu' (`subs`+`b.ne`, 2 istruzioni)
 *   mentre di fabbrica resta all'insu'
 *   ("110192d6 add"+"710fa2df cmp"+"54fffe43 b.cc"@0xffffff8008a7ec10/0c/10,
 *   3 istruzioni).  L'istogramma dei mnemonici lo mostra tutto:
 *     fts_tp_state_recovery  {'add': (10, 9), 'b.cc': (1, 0), 'b.ne': (1, 2),
 *                             'cmp': (3, 2), 'subs': (0, 1)}
 *
 *   CORREZIONE del 2026-08-21: qui seguiva «Che non sia il sorgente lo prova
 *   la copia FUORI LINEA, fts_wait_tp_to_valid, che dalle STESSE righe misura
 *   196 byte esatti».  Quell'argomento NON prova la tesi (classe C2: la prova
 *   offerta non prova cio' che dice).  La copia fuori linea porta LO STESSO
 *   contatore all'ingiu' della copia incorporata -- `mov w22, #0xa`, dieci
 *   giri, poi `subs`+`b.eq` -- mentre la fabbrica li' conta all'insu' con
 *   "110192d6 add"+"710fa2df cmp"+"54fffe43 b.cc"@0xffffff8008a7eb38/34/38.
 *   Le due 196 coincidono NONOSTANTE la trasformazione, per compensazione (un
 *   `b` in piu' da una parte, un'istruzione di ciclo in meno dall'altra), non
 *   perche' la trasformazione manchi:
 *     fts_wait_tp_to_valid   {'add': (6, 5), 'b': (1, 2), 'b.cc': (1, 0),
 *                             'b.ne': (1, 2), 'cmp': (3, 2), 'mov': (8, 9),
 *                             'orr': (1, 0), 'subs': (0, 1)}
 *   La tesi -- e' il compilatore, non il sorgente -- puo' restare: la
 *   sostiene il fatto che le due copie NASCONO DALLE STESSE RIGHE e subiscono
 *   la stessa trasformazione.  Ma l'uguaglianza di TAGLIA non la sostiene, e
 *   la frase e' stata tolta.
 *
 *   Corollario, dalla stessa misura: TAGLIA IDENTICA NON VUOL DIRE FORMA
 *   IDENTICA.  Delle 11 funzioni che misurano esattamente come la fabbrica,
 *   l'istogramma dei mnemonici coincide su UNA sola, tpd_i2c_detect.  Le
 *   altre dieci differiscono almeno per l'alias `orr wN, wzr, #imm` contro
 *   `mov wN, #imm` (tpd_eint_interrupt_handler: orr 4->0, mov 2->6;
 *   tpd_driver_init: orr 1->0, mov 4->5) e fts_wait_tp_to_valid differisce
 *   anche in struttura, come sopra.  Si riproduce con:
 *     $ ./venv/bin/python3 corr-ft8719/isto.py fts_wait_tp_to_valid \
 *           fts_tp_state_recovery tpd_i2c_detect tpd_eint_interrupt_handler \
 *           tpd_driver_init tpd_driver_exit
 *
 * DIVERGENZA 6 -- i numeri di riga di FTS_FUNC_EXIT.  ZERO byte.
 *   Il binario porta il __LINE__ di ogni FTS_FUNC_EXIT e di ogni TPD_DMESG,
 *   e sono numeri del file di FABBRICA.  Questo file non li riproduce, e la
 *   scelta e' dichiarata: per riprodurli bisognerebbe spostare ~340 righe di
 *   codice e tagliare ~214 righe di questo cappello, e il guadagno sarebbe
 *   ZERO byte (ogni divergenza e' un solo `mov w2, #imm`, stessa taglia).
 *   La tabella e' qui perche' l'informazione non vada persa -- dice al
 *   prossimo lotto dov'e' ogni funzione nel sorgente di fabbrica:
 *
 *     sito                                    fabbrica   qui
 *     fts_tp_state_recovery  EXIT               270   547
 *     fts_reset_proc         EXIT               290   568
 *     fts_irq_disable        EXIT               314   592
 *     fts_irq_enable         EXIT               337   614
 *     fts_release_all_finger EXIT               451   644
 *     fts_input_init         EXIT (successo)    845   682
 *     fts_input_init         EXIT (errore)      852   688
 *     get_module_id          TPD_DMESG          892   712
 *     get_module_id          TPD_DMESG          898   723
 *     tpd_probe              EXIT (successo)   1089  1253
 *     tpd_probe              EXIT (errore)     1104  1267
 *     tpd_remove             EXIT              1160  1291
 *     tpd_local_init         EXIT (errore)     1182  1320
 *     tpd_local_init         EXIT (successo)   1207  1345
 *     tpd_suspend            EXIT              1275  1379
 *     tpd_resume             EXIT              1341  1410
 *     tpd_driver_init        EXIT              1381  1439
 *     tpd_driver_exit        EXIT              1396  1458
 *
 *   Due cose che quella colonna dice e che nessun'altra misura dice:
 *   (a) il file di fabbrica e' lungo ALMENO 1396 righe;
 *   (b) fra fts_irq_enable (337) e fts_release_all_finger (451) ci sono 114
 *       righe, e fra questa e fts_input_init (845) altre 394, che NON
 *       corrispondono a nessuna funzione del blocco: sono codice di fabbrica
 *       che il config non compila.  Questa ricostruzione non le contiene e
 *       non pretende di contenerle.
 *
 * ===========================================================================
 * IL RESIDUO CHE NON SO SPIEGARE
 * ===========================================================================
 * Nessuno, alla risoluzione della singola istruzione, TRANNE questo: dentro
 * DIVERGENZA 2 e 3 ho attribuito i -84 e i -104 byte a classi (fusione delle
 * code, disposizione della cornice, rimaterializzazione degli indirizzi,
 * riduzione di forza) e ho contato le istruzioni di ogni classe
 * nell'istogramma, ma NON ho fatto corrispondere una per una tutte le 21 e 26
 * istruzioni a una singola causa.  L'istogramma quadra come somma; la
 * corrispondenza puntuale no.  E' un residuo di metodo, non un residuo
 * numerico, e va detto.
 *
 * ===========================================================================
 * DELTA DI HEADER -- descritto, NON fatto (sono file CONDIVISI)
 * ===========================================================================
 * 1. drivers/input/touchscreen/mediatek/tpd.h dichiara
 *      extern unsigned long TPD_RES_X;
 *      extern unsigned long TPD_RES_Y;
 *    ma questa unita' di traduzione le legge a 32 BIT SENZA SEGNO, due volte
 *    e in due modi indipendenti:
 *      "b942b901 ldr"@0xffffff8008a7f890   (tpd_probe, argomento di un %d)
 *      "6b09011f cmp"+"54000209 b.ls"@0xffffff8008a7fee4/e4  (confronto
 *      SENZA segno con events[i].y, che e' un int)
 *    Un `unsigned long` passato a un %d darebbe `ldr x1`, e un confronto fra
 *    `int` e `unsigned long` sarebbe a 64 bit.  Qui il cast a (u32) e' scritto
 *    nel sorgente; NON so dire se di fabbrica fosse un cast o una
 *    dichiarazione locale `extern unsigned int TPD_RES_Y;` in disaccordo con
 *    la definizione (che e' a 8 byte: nel nostro System.map TPD_RES_X e
 *    TPD_RES_Y distano 8).  Il binario non discrimina fra le due.
 *    Il delta sarebbe: decidere quale delle due, e nel secondo caso
 *    registrarlo come difetto di fabbrica riprodotto.
 *
 * 2. `tpd_type_cap` e' dichiarato in tpd_debug.h, che tpd.h include solo
 *    sotto TPD_DEBUG_CODE.  Qui e' dichiarato localmente.  La sede giusta
 *    sarebbe tpd.h, accanto a tpd_load_status.
 *
 * 3. Il Makefile padre (drivers/input/touchscreen/mediatek/Makefile) e il
 *    Kconfig padre non hanno la riga per questa directory.  Servono:
 *      obj-$(CONFIG_WTK_FT8719_E977) += focaltech_ft8719/
 *      source "drivers/input/touchscreen/mediatek/focaltech_ft8719/Kconfig"
 *    Non le ho aggiunte: sono file condivisi con gli altri due driver touch.
 *
 * 4. `fix_tp_proc_info` (e le altre tre Wingtech di mtk_tpd.c:
 *    wtk_creat_proc_tp_info, wtk_tp_info_open, wtk_tp_info_show,
 *    0xffffff8008a5151c..0xffffff8008a515e8) NON esistono nel nostro albero.
 *    Sono una patch a mtk_tpd.c, non a questo blocco; senza di loro il link
 *    non riesce.  Vanno scritte dal lotto che tocca mtk_tpd.c.
 */

/*
 * ===========================================================================
 * OGNI LETTERALE DI QUESTO FILE, E I BYTE DI FABBRICA DA CUI VIENE
 * ===========================================================================
 * Un messaggio per riga, nella forma verificabile "testo"@0xINDIRIZZO.  Il
 * testo citato e' il messaggio COME LO ASSEMBLA LA MACRO -- livello KERN,
 * prefisso e '\n' finale compresi -- perche' e' quello che il binario porta,
 * NUL-terminato, a quell'indirizzo.  Il letterale scritto nel codice e' la
 * parte in mezzo.  `n` dice quante volte quei byte compaiono in tutta
 * l'immagine: n=1 e' una citazione PROBANTE, n>1 e' DEBOLE.
 *
 * La colonna `macro` dice da quale macro viene la testa, e serve a leggere il
 * blocco successivo: le righe FTS_DEBUG sono quelle che verificacitazioni.py
 * non riesce ad ANCORARE, per una ragione precisa e scritta li'.
 *
 *   macro       citazione
 *   printk      "[FTS]%s: Enter\n"@0xffffff800924f634   n=1
 *   printk      "[FTS]%s: Exit(%d)\n"@0xffffff800924f65a   n=1
 *   FTS_DEBUG   "[FTS]TP Not Ready, ReadData = 0x%x\n"@0xffffff800924f5e7   n=1
 *   FTS_INFO    "\x016[FTS][Info]TP Ready, Device ID = 0x%x\n"@0xffffff800924f60b   n=1
 *   FTS_ERROR   "\x013[FTS][Error]failed to alloc memory for point buf!\n"@0xffffff800924f9e3   n=1
 *   FTS_ERROR   "\x013[FTS][Error]failed to alloc memory for point events!\n"@0xffffff800924fa18   n=1
 *   TPD_DMESG   "\x016mtk-tpd:[%s:%d] fts ret = %d ,verdor_id = 0x%x.\n"@0xffffff800924f69b   n=1
 *   FTS_ERROR   "\x013[FTS][Error]read touchdata failed, ret:%d\n"@0xffffff800924fba9   n=1
 *   FTS_INFO    "\x016[FTS][Info]touch buff is 0xff, need recovery state\n"@0xffffff800924fbd6   n=1
 *   FTS_INFO    "\x016[FTS][Info]invalid point_num(%d)\n"@0xffffff800924fc0c   n=1
 *   FTS_ERROR   "\x013[FTS][Error]ID(%d) beyond max_touch_number\n"@0xffffff800924fc30   n=1
 *   FTS_INFO    "\x016[FTS][Info]abnormal touch data from fw\n"@0xffffff800924fc5e   n=1
 *   FTS_INFO    "\x016[FTS][Info]no touch point information\n"@0xffffff800924fc88   n=1
 *   FTS_DEBUG   "[FTS]Key(%d, %d) DOWN\n"@0xffffff800924fd01   n=1
 *   FTS_DEBUG   "[FTS]Key(%d, %d) UP\n"@0xffffff800924fd18   n=1
 *   FTS_DEBUG   "[FTS][B]P%d(%d, %d)[p:%d,tm:%d] DOWN!\n"@0xffffff800924fcb1   n=1
 *   FTS_DEBUG   "[FTS][B]P%d UP!\n"@0xffffff800924fcd8   n=1
 *   FTS_DEBUG   "[FTS][B]Points All Up!\n"@0xffffff800924fce9   n=1
 *   FTS_DEBUG   "[FTS]touch_event_handler start\n"@0xffffff800924fb89   n=1
 *   FTS_DEBUG   "[FTS]i2c read invalid, read:0x%02x%02x\n"@0xffffff800924fa50   n=1
 *   FTS_DEBUG   "[FTS]verify id:0x%02x%02x\n"@0xffffff800924fb6e   n=1
 *   FTS_DEBUG   "[FTS]TP not ready, read:0x%02x%02x\n"@0xffffff800924fa78   n=1
 *   FTS_INFO    "\x016[FTS][Info]fw is invalid, need read boot id\n"@0xffffff800924fa9c   n=1
 *   FTS_ERROR   "\x013[FTS][Error]start cmd write fail\n"@0xffffff800924facb   n=1
 *   FTS_ERROR   "\x013[FTS][Error]read boot id fail\n"@0xffffff800924faef   n=1
 *   FTS_ERROR   "\x013[FTS][Error]can't get ic informaton\n"@0xffffff800924fb10   n=1
 *   FTS_INFO    "\x016[FTS][Info]get ic information, chip id = 0x%02x%02x\n"@0xffffff800924fb37   n=1
 *   FTS_ERROR   "\x013[FTS][Error]Failed to allocate memory for fts_data\n"@0xffffff800924f718   n=1
 *   FTS_INFO    "\x016[FTS][Info][TPD]Change i2c addr 0x%02x to %x\n"@0xffffff800924f74e   n=1
 *   FTS_INFO    "\x016[FTS][Info][TPD]i2c addr=0x%x\n\n"@0xffffff800924f77e   n=1
 *   FTS_ERROR   "\x013[FTS][Error]failed to create fts workqueue\n"@0xffffff800924f7a7   n=1
 *   FTS_ERROR   "\x013[FTS][Error]fts input initialize fail\n"@0xffffff800924f7ec   n=1
 *   FTS_ERROR   "\x013[FTS][Error]not focal IC, unregister driver\n"@0xffffff800924f815   n=1
 *   FTS_ERROR   "\x013[FTS][Error]create apk debug node fail\n"@0xffffff800924f844   n=1
 *   FTS_ERROR   "\x013[FTS][Error]create sysfs node fail\n"@0xffffff800924f86e   n=1
 *   FTS_ERROR   "\x013[FTS][Error]init glove/cover/charger fail\n"@0xffffff800924f894   n=1
 *   FTS_ERROR   "\x013[FTS][Error][TPD]Failed to create kernel thread_tpd,ret:%d\n"@0xffffff800924f8c1   n=1
 *   FTS_DEBUG   "[FTS][TPD]Touch Panel Device Probe %s!\n"@0xffffff800924f8ff   n=1
 *   FTS_INFO    "\x016[FTS][Info]IRQ request succussfully, irq:%d client:%d\n"@0xffffff800924fd61   n=1
 *   FTS_ERROR   "\x013[FTS][Error]Can not find touch eint device node!\n"@0xffffff800924fd2d   n=1
 *   FTS_ERROR   "\x013[FTS][Error]request irq failed\n"@0xffffff800924f927   n=1
 *   FTS_DEBUG   "[FTS]TPD_RES_Y:%d\n"@0xffffff800924f949   n=1
 *   printk      "yang.liu*****log***sensor_id=%d\n"@0xffffff800924f95c   n=1
 *   FTS_ERROR   "\x013[FTS][Error][TPD]: Unable to add fts i2c driver!!\n"@0xffffff800924f6e3   n=1
 *   FTS_INFO    "\x016[FTS][Info]Already in suspend state\n"@0xffffff800924fdac   n=1
 *   FTS_INFO    "\x016[FTS][Info]fw upgrade in process, can't suspend\n"@0xffffff800924fdd3   n=1
 *   FTS_ERROR   "\x013[FTS][Error]Set TP to sleep mode fail, ret=%d!\n"@0xffffff800924fe06   n=1
 *   FTS_DEBUG   "[FTS]Already in awake state\n"@0xffffff800924fe38   n=1
 *   FTS_INFO    "\x016[FTS][Info]Driver version: %s\n"@0xffffff800924fe6c   n=1
 *   FTS_INFO    "\x016[FTS][Info]tpd max touch num:%d\n"@0xffffff800924fe8d   n=1
 *   FTS_ERROR   "\x013[FTS][Error][TPD]: Add FTS Touch driver failed!!\n"@0xffffff800924feb0   n=1
 *   -           "Focaltech V2.2 20180321"@0xffffff800924e1c9   n=1
 *   -           "none_0"@0xffffff800998ca50   n=1
 *   -           "none_1"@0xffffff800998ca78   n=2
 *   -           "none_2"@0xffffff800998caa0   n=1
 *   -           "none_3"@0xffffff800998cac8   n=2
 *   -           "none_4"@0xffffff800998caf0   n=2
 *   -           "none_5"@0xffffff800998cb18   n=2
 *   -           "none_6"@0xffffff800998cb40   n=1
 *   -           "none_7"@0xffffff800998cb68   n=1
 *   -           "none_8"@0xffffff800998cb90   n=1
 *   -           "mediatek,cap_touch"@0xffffff8008f7f4a0   n=3
 *   -           "fts_ts"@0xffffff800924f6dc   n=2
 *   -           "fts_wq"@0xffffff800924f7a0   n=1
 *   -           "TOUCH_PANEL-eint"@0xffffff800924c50d   n=1
 *   -           "ft8719"@0xffffff800924f9cd   n=3
 *   -           "TP IC :%s,TP module :%s,TP I2C adr : 0x%x,SW FirmWare:0x%x,Sample FirmWare:0x%x"@0xffffff800924f97d   n=1
 */

/*
 * ===========================================================================
 * LE CITAZIONI CHE `verificacitazioni.py` SEGNA `NON_ANCORATA`, E PERCHE'
 * ===========================================================================
 * Non sono citazioni sbagliate: i byte ci sono, a quegli indirizzi, e sono
 * NUL-terminati -- il controllo di byte lo prova, e' l'ANCORAGGIO che manca.
 * Sono DUE classi, e vale la pena scriverle perche' la seconda e' nuova.
 *
 * 1. LA STRINGIFICAZIONE DI UNA MACRO.
 *      "&ts_data->report_mutex"@0xffffff800924f7d5
 *    Non e' un letterale scritto a mano: e' `#lock` dentro mutex_init, cioe'
 *    l'argomento stringato.  Lo strumento conosce il dominio __func__
 *    (RE_IDENTIFICATORE piu' i nomi delle funzioni del file) ma non quello
 *    di `#`.  Questa stringa e' anche l'UNICA prova che il campo a +288 si
 *    chiama davvero `report_mutex` e che il locale del probe si chiama
 *    `ts_data`: senza di lei quei due nomi sarebbero inventati (classe B2).
 *
 * 2. UNA MACRO DI LOG SENZA LIVELLO KERN -- tredici righe FTS_DEBUG del
 *    blocco qui sopra.
 *    Lo strumento sa gia' verificare un messaggio ASSEMBLATO da una macro
 *    (la "quarta convenzione"): controlla che la CODA della citazione sia un
 *    letterale vero del codice e poi confronta il testo intero col binario.
 *    Ma lo riconosce da
 *        RE_TESTA_ASSEMBLATA = re.compile(r'\A\\x01[0-7]')
 *    cioe' dal SOH piu' la cifra di livello.  FTS_DEBUG NON emette un
 *    livello, e il binario lo prova da solo: "[FTS]%s: Enter\n"
 *    @0xffffff800924f634 comincia con '[' e non con \x01, mentre
 *    "\x016[FTS][Info]TP Ready, Device ID = 0x%x\n"@0xffffff800924f60b
 *    comincia col SOH.  Sono due macro diverse nello stesso file, e il
 *    binario le distingue.
 *    I messaggi FTS_DEBUG cadono quindi fuori dalla quarta convenzione pur
 *    avendo la stessa struttura: testa costante + letterale del codice +
 *    "\n".  La loro coda ("\n") E' un letterale di questo file, cioe' il
 *    primo dei due controlli passerebbe.
 *    E' un DELTA DI STRUMENTO, e non l'ho fatto io: verificacitazioni.py e'
 *    condiviso.  La forma sarebbe accettare anche una testa che sia essa
 *    stessa un letterale del codice ("[FTS]"), non solo \x01 + cifra.
 */

/*
 * ===========================================================================
 * DELTA DI STRUMENTO PER `verificaistruzioni.py` -- IL BUCO CHE HA LASCIATO
 * PASSARE DUE CITAZIONI SBAGLIATE (aggiunto il 2026-08-21)
 * ===========================================================================
 * La citazione della classe B8 corretta qui sopra (DIVERGENZA 3, punto adrp)
 * non e' stata trovata da nessuno strumento, e non per caso: nella forma a
 * CODIFICA -- "<codifica8hex> <mnemonico>"@0xINDIRIZZO, quella che questo
 * file usa 240 volte -- lo strumento cerca la codifica OVUNQUE nel blocco e
 * NON confronta mai l'indirizzo scritto dopo la @.  Una citazione che porti
 * la codifica di un'altra istruzione dello stesso blocco risulta
 * "confermata".  Lo dice il codice stesso di verificaistruzioni.py, dove la
 * forma virgolettata@indirizzo salta la forma a codifica proprio perche'
 * "estrai_citazioni_istruzione la prende gia' correttamente" -- ma quella la
 * prende per codifica, senza indirizzo.
 *
 * Il controllo che manca e' di dieci righe e l'ho scritto per questa
 * revisione: espandere ogni gruppo di citazioni unite dal '+' e chiuse da un
 * solo @0xINDIRIZZO con i suffissi /xx/yy nelle sue coppie (codifica,
 * indirizzo) e confrontare ognuna contro l'indirizzo che le tocca.
 * Su questo file, prima della correzione:
 *
 *   $ ./venv/bin/python3 corr-ft8719/checkcit.py corr-ft8719/focaltech_core.c.PRIMA
 *   citazioni istruzione totali nel file: 240
 *   coppie (codifica,indirizzo) espanse: 231
 *   senza indirizzo esplicito nella coppia: 0
 *   DISCORDANTI: 2
 *   [... le due righe di dettaglio sono omesse QUI di proposito: riportano
 *    alla lettera le due citazioni sbagliate, e incollarle nel file corretto
 *    le rimetterebbe in circolo.  Le trovi, con l'istruzione VERA accanto, al
 *    punto (a) della DIVERGENZA 3 ...]
 *
 * Due su 231: il resto del file regge il controllo.  Lo strumento e'
 * CONDIVISO e non l'ho toccato (lo vieta il briefing): questo e' il delta,
 * scritto perche' il lotto che tocca gli strumenti lo trovi.
 *
 * NEL FRATTEMPO, la difesa che si puo' mettere in un driver senza toccare lo
 * strumento: dove la citazione deve provare qualcosa di piu' della presenza
 * dell'istruzione -- una CONDIZIONE, un immediato, un registro -- si scrive
 * ANCHE nella quarta forma del progetto, "mnemonico operandi"@0xINDIRIZZO,
 * che verificaistruzioni.py risolve ALL'INDIRIZZO e confronta con gli
 * operandi veri.  In questo file e' fatto per la csinc di
 * fts_get_ic_information, che e' esattamente il punto dove la citazione nuda
 * aveva lasciato passare un difetto di codice:
 *   "csinc w2, w8, wzr, eq"@0xffffff8008a7f634
 */
