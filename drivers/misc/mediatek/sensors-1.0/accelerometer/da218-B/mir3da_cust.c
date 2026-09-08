// SPDX-License-Identifier: GPL-2.0
/*
 * Accelerometro MiraMEMS mir3da (DA218-B) del Doogee S88 Pro —
 * l'unita' di traduzione "cust", ricostruita leggendo il kernel di
 * fabbrica disassemblato.  Nessuna riga viene da un altro telefono:
 * il sorgente mir3da non esiste ne' nell'albero ALPS ne' nell'albero
 * .186 dell'altro progetto.
 *
 * Il nome di questo file e la sua collocazione sono PROVATI, non dedotti
 * per adiacenza:
 *
 *  1. l'ASSERT di mir3da_reg_data_store porta il proprio __FILE__ come
 *     dato nel binario —
 *     "/home/jenkins/jks-WTK6739_Q0_MP1_AP/alps/kernel-4.14/drivers/misc/
 *      mediatek/sensors-1.0/accelerometer/da218-B/mir3da_cust.c"
 *     @0xffffff80091bfea6, caricata da "913a9821 add x1, x1, #0xea6";
 *  2. MODULE_VERSION, compilata built-in, lascia una
 *     struct module_version_attribute a 0xffffff800992eb18 il cui campo
 *     .module_name (+56) e' 'mir3da_cust' a 0xffffff80091c0452, cioe' il
 *     KBUILD_MODNAME vero di questo file oggetto.
 *
 * Le 37 funzioni di questo file sono le prime 36 del blocco contiguo
 * 0xffffff8008788658..0xffffff800878c2c0 (da i2c_smbus_read a
 * mir3da_resume) piu' mir3da_init, che sta in .init.text a
 * 0xffffff80093740fc.  Le altre 29 del blocco (da squareRoot a
 * NSA_get_reg_data) sono una seconda unita' di traduzione: 19 sono ora
 * ricostruite nel core e 10 restano omesse. Il nome "mir3da_core.c" per
 * riferirsi ad essa e' la convenzione del driver MiraMEMS a monte, NON
 * una misura: la stringa "mir3da_core" non esiste in nessun punto di
 * stock.elf (revisione indipendente, docs/bringup/rapporti/
 * revisione-mir3da.md, rilievo R6b).
 *
 * Prova che sono due unita' e non una:
 *  - linkage: le 36 di qui sono `t` (statiche) tranne i2c_smbus_read/
 *    _read_block/_write e msdelay, che il core prende per indirizzo dalla
 *    general_op_s; le 29 del core sono quasi tutte `T` — funzioni globali
 *    in un kernel senza moduli hanno senso solo se un ALTRO file oggetto
 *    le chiama (la prova piu' forte delle quattro, revisione §1.5);
 *  - i messaggi: TUTTI i letterali DI LOG di questo file portano il
 *    prefisso "\x013[MIR3DA] " (KERN_ERR piu' il tag della macro),
 *    NESSUNO di quelli del core lo porta — non "tutti i letterali" senza
 *    qualificazione: "MiraMEMS", "%s_%s\n", "x= %d;y=%d;z=%d\n",
 *    "reading failed!" e il formato dell'ASSERT restano senza prefisso
 *    perche' non sono messaggi di log (revisione indipendente, rilievo
 *    R6).  La ragione strutturale e' che il core non chiama mai printk:
 *    logga attraverso general_op_s.myprintk (+64, vedi mir3da_core.h),
 *    quindi i suoi letterali non passano mai dal preprocessore di questo
 *    file.  Il linker ha comunque fuso le due forme dove coincidono:
 *    "\x013[MIR3DA] chip resume fail!!\n\n" sta a 0xffffff80091c041e,
 *    puntata da mir3da_resume; la variante del core, "[MIR3DA] chip
 *    resume fail!!\n\n", e' la sua coda a 0xffffff80091c0420, puntata da
 *    mir3da_core_init — risolto sui riferimenti reali di stock.elf, non
 *    per sola adiacenza degli indirizzi;
 *  - MODULE_VERSION nomina "mir3da_cust" e non "mir3da_core" — prova
 *    che QUESTO file e' un oggetto a se', non che il core sia un secondo
 *    oggetto chiamato cosi'.
 *
 * I numeri di riga del sorgente di fabbrica sono noti, perche' MI_FUN
 * porta __LINE__ come immediato: 119 (mir3da_resetCalibration), 131
 * (mir3da_readCalibration), 146 (mir3da_writeCalibration), 307
 * (mir3da_enable_show), 376 (mir3da_reg_data_store), 538 (mir3da_suspend),
 * 566 (mir3da_resume), 725/745/759/769/778/793 (le sei mir3da_factory_*),
 * 872 (mir3da_probe), 1045 (mir3da_local_init), 1063 (mir3da_local_remove),
 * 1074 (mir3da_init).  Non si sovrappongono: e' UN file solo, di circa
 * 1080 righe.  L'ordine del sorgente non e' l'ordine del binario
 * (mir3da_probe e' alla riga 872 ed e' emessa per ottava; mir3da_enable_show
 * e' alla riga 307 ed e' emessa dopo le mir3da_factory_*): l'ordine di
 * definizione qui sotto segue i numeri di riga, non gli indirizzi.
 *
 * Le tre funzioni alle righe 119/131/146 non hanno un simbolo proprio nel
 * kernel di fabbrica — clang le ha incorporate nei loro chiamanti — ma i
 * loro __LINE__ e i loro __func__ sono nel binario, quindi esistono nel
 * sorgente e sono riscritte qui come statiche.
 *
 * Stato del core: la decisione originaria di non scriverlo, presa perche'
 * le cinque fonti sulla forma del sorgente (.ddebug, numeri di riga di
 * MI_FUN, __func__, ASSERT con __FILE__, module_version_attribute)
 * tacciono, e' stata superata il 2026-08-16 dalla rilettura dell'intero
 * blocco. La tabella a 0xffffff800992eda8 e' stata letta byte per byte:
 * una voce di 184 byte, nome "NSA_NTO", 144 byte fino ai descrittori e
 * quattro puntatori alle callback NSA_*. Delle 29 funzioni stock del core,
 * 19 sono ora ricostruite: le 9 API chiamate direttamente da questo file
 * piu' 10 dipendenze interne e callback necessarie a chiudere il link.
 * Restano omesse 10 globali senza chiamanti diretti osservati; le loro
 * relocation uniformi appartengono a kallsyms, non a chiamate operative.
 * La ricostruzione non pretende di conoscere la forma sorgente perduta:
 * conserva esplicitamente le ambiguita' non osservabili e confronta il
 * comportamento col disassemblato stock. Il censimento, la rilettura
 * indipendente delle 19 funzioni e i limiti delle reti automatiche sono in
 * docs/bringup/rapporti/rapporto-mir3da-core.md. Questa nota sostituisce
 * la decisione storica del lotto cust-only, conservata con le sue prove in
 * docs/bringup/rapporti/revisione-mir3da.md (rilievi R2/R8), senza
 * reinterpretarla come se allora il core fosse gia' stato disponibile.
 * Il nome mir3da_core.c resta convenzionale, non misurato dal binario.
 *
 * Misura (misuraisolata.py --radice nessuna, build indipendente in
 * out-rev-mir3da): 37/37 presenti, 29/37 alla dimensione di fabbrica.
 * Ristretta alle 7 funzioni sopra i 256 byte, dove la misura discrimina
 * davvero: 3/7 = 42,9% — su n=7 l'IC95% di Clopper-Pearson e' [9,9%;
 * 81,6%] e CONTIENE il 77,10% del ramo ALPS .141: non e' "sotto il
 * rumore di fondo", NON DISCRIMINA, ne' in un senso ne' nell'altro
 * (revisione indipendente, rilievo R4).  Le otto divergenze, enumerate
 * per intero — nessuna lo era da nessuna parte prima (rilievo R9):
 *   mir3da_probe              -8  (vedi il commento sulla forma del
 *                                  ciclo, sopra mir3da_probe)
 *   mir3da_enable_nodata     -12
 *   mir3da_get_data           +8
 *   mir3da_factory_set_cali  +20  (138 istruzioni di fabbrica contro 143
 *                                  nostre: e' una differenza di FORMA dei
 *                                  rami, non rumore di layout; resta
 *                                  APERTA, non indagata)
 *   msdelay                   -4
 *   get_address                -4
 *   mir3da_enable_show        -4
 *   mir3da_enable_store       -4
 * I quattro -4 isolati non sono, da soli, un difetto: perfino
 * acc_factory_unlocked_ioctl, stesso sorgente ALPS su entrambi i lati,
 * diverge di 4 byte su 1728 per la sola sostituzione di compilatore
 * (clang-r353983c -> clang-r383902: orr wN,wzr,#imm -> movz, 21 siti
 * contro 0 in questo blocco) — non un difetto del driver.
 */
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include <accel.h>
#include <cust_acc.h>
#include <hwmsen_helper.h>
#include <hwmsensor.h>
#include <sensors_io.h>

#include "mir3da_core.h"

/* ------------------------------------------------------------------ */
/* Log                                                                 */
/* ------------------------------------------------------------------ */

/*
 * Il livello e' una maschera di bit, letta con `ldrb` e testata con `tbz`
 * bit per bit.  Che la variabile sia un `int` a 32 bit e non un byte lo
 * prova mir3da_log_level_show, che la legge intera —
 *   "b94d9902 ldr w2, [x8,#3480]"
 * — e mir3da_log_level_store, che la scrive intera —
 *   "b90d9900 str w0, [x8,#3480]".
 * Le `ldrb` dei test sono il restringimento che clang fa quando servono
 * solo i bit bassi.
 *
 * I bit usati, dal disassemblato:
 *   bit 0  "36000088 tbz w8, #0, ..."   messaggi d'errore
 *   bit 2  "361000a8 tbz w8, #2, ..."   messaggi informativi
 *   bit 3  "361800e8 tbz w8, #3, ..."   traccia di funzione (MI_FUN)
 *   bit 4  "36200108 tbz w8, #4, ..."   dump dei dati
 * Il bit 1 non compare in questo file.
 *
 * Il valore iniziale e' 1: la .data di fabbrica a 0xffffff800992ed98
 * contiene 01 00 00 00.
 *
 * CORRETTO il 2026-08-16, scrivendo il core.  Fino a quel giorno qui c'era
 *   static int Log_level = MIR_ERR;
 * e NON puo' essere cosi': la stessa variabile a 0xffffff800992ed98 e'
 * letta anche da mir3da_temp_calibrate, mir3da_core_init,
 * mir3da_module_detect, mir3da_read_raw_data, mir3da_chip_resume,
 * mir3da_get_primary_offset e NSA_interrupt_ops, che stanno nell'ALTRA
 * unita' di traduzione — una `static` sarebbe invisibile da li'.  La
 * scansione che lo prova (34 funzioni su tutto il kernel di fabbrica, di
 * entrambe le unita') e' in docs/bringup/rapporti/rapporto-mir3da-core.md.
 * Definizione e maschere di bit stanno ora in mir3da_core.h / mir3da_core.c.
 */

/*
 * Tutti i messaggi di questo file cominciano con "\x013", cioe'
 * KERN_SOH "3" = KERN_ERR, seguito da "[MIR3DA] ": lo si vede su ogni
 * stringa citata piu' sotto, e vale anche per i messaggi informativi e di
 * traccia.  Non e' una scelta: e' cio' che il binario contiene.
 * Il prefisso '[MIR3DA] ' non esiste come stringa a se' nel binario di
 * fabbrica — il preprocessore lo concatena a ogni messaggio, quindi non e'
 * mai terminato da NUL da solo e verificacitazioni.py non puo' ancorarlo:
 * e' una delle tre eccezioni nominate.  Lo provano indirettamente le
 * sessanta citazioni dei messaggi qui sotto, che cadono tutte esattamente
 * undici byte dopo l'inizio del letterale intero (due di KERN_SOH+'3' piu'
 * nove di '[MIR3DA] ').
 */
#define MI_ERR(fmt, arg...) \
	do { if (Log_level & MIR_ERR)  printk(KERN_ERR "[MIR3DA] " fmt, ##arg); } while (0)
#define MI_MSG(fmt, arg...) \
	do { if (Log_level & MIR_MSG)  printk(KERN_ERR "[MIR3DA] " fmt, ##arg); } while (0)
#define MI_DATA(fmt, arg...) \
	do { if (Log_level & MIR_DATA) printk(KERN_ERR "[MIR3DA] " fmt, ##arg); } while (0)
/* "%s is called, line: %d\n"@0xffffff80091bf916 */
#define MI_FUN \
	do { if (Log_level & MIR_FUN) \
		printk(KERN_ERR "[MIR3DA] " "%s is called, line: %d\n", __func__, __LINE__); \
	} while (0)

/*
 * "Assertion failed! %s,%d,%s,%s\n"@0xffffff80091bfe85 — questo messaggio
 * NON ha il prefisso "[MIR3DA] " (l'indirizzo citato punta gia' al testo
 * subito dopo "\x013"), quindi la macro non lo aggiunge.
 */
#define ASSERT(expr) \
	do { if (!(expr)) \
		printk(KERN_ERR "Assertion failed! %s,%d,%s,%s\n", \
		       __FILE__, __LINE__, __func__, #expr); \
	} while (0)

/* ------------------------------------------------------------------ */
/* Costanti e dati privati                                             */
/* ------------------------------------------------------------------ */

/* "mir3da"@0xffffff80091bf966: .driver.name della i2c_driver a
 * 0xffffff800992ebc0 e .name della acc_init_info a 0xffffff800992eb60. */
#define MIR3DA_DRV_NAME		"mir3da"

/* "1.0"@0xffffff80091bff91: e' insieme il .version della
 * module_version_attribute (0xffffff800992eb58) e il primo argomento
 * della sprintf di mir3da_version_show ("913e4442 add x2, x2, #0xf91"). */
#define MIR3DA_DRIVER_VERSION	"1.0"

/* "52800280 mov w0, #0x14" in mir3da_get_data: msleep(20). */
#define MIR3DA_POWERON_DELAY_MS	20

/* La dimensione del buffer di stringa e' misurata, non scelta: in
 * mir3da_get_data il buffer sta a sp+20 e l'array databuf a x29-0x6c =
 * sp+276, cioe' 256 byte dopo; in mir3da_factory_get_raw_data
 * l'azzeramento va da "a900ffff stp xzr, xzr, [sp,#8]" a
 * "a90fffff stp xzr, xzr, [sp,#248]", cioe' 256 byte esatti. */
#define MIR3DA_BUFSIZE		256

/* Gli indici degli assi. "3940f308 ldrb w8, [x24,#60]" e le due seguenti
 * leggono cvt.map[0..2]; "39c0e30d ldrsb w13, [x24,#56]" e le due
 * seguenti leggono cvt.sign[0..2]. */
#define MIR3DA_AXIS_X		0
#define MIR3DA_AXIS_Y		1
#define MIR3DA_AXIS_Z		2
#define MIR3DA_AXES_NUM		3

/* "5284c9e9 mov w9, #0x264f" in mir3da_get_data e "5284c9eb mov w11, #0x264f"
 * in mir3da_factory_get_cali: 9807, il valore di g in mg che MediaTek
 * chiama GRAVITY_EARTH_1000. */
#define GRAVITY_EARTH_1000	9807

/* "321603e2 orr w2, wzr, #0x400" (mir3da_probe) e
 * "321603eb orr w11, wzr, #0x400" (mir3da_factory_set_cali): 1024 LSB/g. */
#define MIR3DA_GAIN		1024

/* "7114551f cmp w8, #0x515" in mir3da_get_data: la soglia oltre la quale
 * l'offset software dell'asse Z fa sottrarre 1 g dal dato. Il confronto e'
 * `b.lt` sul valore assoluto, quindi la condizione del sorgente e' > 1300. */
#define MIR3DA_CALI_Z_LIMIT	1300

/* "5120014a sub w10, w10, #0x800": 2048 = 2 * MIR3DA_GAIN. */
#define MIR3DA_CALI_Z_ADJUST	2048

/* "71004d1f cmp w8, #0x13": l'ID che il chip deve rispondere al registro
 * 0x01 perche' l'indirizzo I2C 0x26 sia quello giusto. */
#define MIR3DA_REG_CHIP_ID	0x01
#define MIR3DA_CHIP_ID		0x13

/* "528004c8 mov w8, #0x26" e "528004e8 mov w8, #0x27": i due indirizzi
 * I2C possibili del chip. */
#define MIR3DA_I2C_ADDR_A	0x26
#define MIR3DA_I2C_ADDR_B	0x27

/* "5295d700 mov w0, #0xaeb8" + "72a028e0 movk w0, #0x147, lsl #16" =
 * 0x147aeb8 = 21475000 = 5000 * 0x10C7, cioe' udelay(5000) = mdelay(5). */
#define MIR3DA_CHIP_ID_RETRY_MS	5

/* Quante volte mir3da_probe rilegge il registro di identita' prima di
 * ripiegare sul secondo indirizzo: le `bl i2c_smbus_read_byte_data` a
 * 0x87889d4, 0x87889f8, 0x8788a1c, 0x8788a40, 0x8788a64, 0x8788a88 sono
 * sei, e i cinque `bl __const_udelay` stanno solo fra una e la seguente. */
#define MIR3DA_CHIP_ID_RETRY	6

/*
 * I dati privati per chip.  La dimensione, 104 byte, e' quella
 * dell'allocazione: "52800d02 mov w2, #0x68" passata a
 * kmem_cache_alloc_trace.  I flag di kzalloc sono
 * "52901801 mov w1, #0x80c0" + "72a02801 movk w1, #0x140, lsl #16" =
 * 0x14080c0 = GFP_KERNEL | __GFP_ZERO.
 *
 * Gli offset dei campi, tutti letti dal disassemblato:
 *   +0    client            "f9000274 str x20, [x19]"
 *   +8    hw                "91002261 add x1, x19, #0x8" -> get_accel_dts_func
 *   +12   hw.direction      "b9400e60 ldr w0, [x19,#12]"
 *   +52   hw.is_batch_supported  "3940d26d ldrb w13, [x19,#52]"  (8 + 44)
 *   +56   cvt               "9100e261 add x1, x19, #0x38" -> hwmsen_get_convert
 *   +64   flush             "3901011f strb wzr, [x8,#64]"
 *   +68   (azzerato)        "b900467f str wzr, [x19,#68]"
 *   +72   suspended         "b9004aa8 str w8, [x21,#72]" in mir3da_suspend
 *   +80   cali_sw           "a9057e7f stp xzr, xzr, [x19,#80]"
 *   +96   data              "7940c30a ldrh w10, [x24,#96]"
 *
 * Tre avvertenze oneste su questa struct:
 *  - il campo a +68 viene azzerato in mir3da_probe e mai letto in nessuna
 *    delle 66 funzioni del blocco: non ho un nome per lui;
 *  - i campi a +68 e +72 sono `atomic_t` e non due `int`: mir3da_probe li
 *    azzera con DUE istruzioni separate, "b900467f str wzr, [x19,#68]" e
 *    "b9004a7f str wzr, [x19,#72]".  Due `int` adiacenti azzerati di
 *    seguito clang li fonde in un unico `stur xzr, [x19,#68]` a 64 bit —
 *    e' quello che questo file produceva prima della correzione.  Il
 *    WRITE_ONCE dentro atomic_set() e' volatile e impedisce la fusione:
 *    la coppia di store separate e' la prova della QUALIFICAZIONE
 *    (accesso non fondibile / volatile), non del TIPO — un `volatile
 *    int` darebbe lo stesso codice.  `atomic_t` resta la forma idiomatica
 *    del kernel per questi campi, ma non e' l'unica compatibile col
 *    binario (revisione indipendente, rilievo R6c);
 *  - fra +72 e +80 c'e' un buco di quattro byte che nessuna delle 66
 *    funzioni tocca.  Non e' un'ipotesi di comodo: senza di esso
 *    `cali_sw` cadrebbe a +76, e a quel disallineamento clang non puo'
 *    piu' usare "a9057e7f stp xzr, xzr, [x19,#80]" per azzerarlo — ne
 *    emette due (`stur` a +76 e a +84), che e' esattamente lo scarto di
 *    +4 con cui mir3da_factory_clear_cali sbagliava prima;
 *  - a +80 la fabbrica azzera SEDICI byte in una sola istruzione, mentre
 *    legge e scrive soltanto le tre halfword a +80/+82/+84.  Un
 *    `s16 cali_sw[3]` non produrrebbe quello store.  `s16 cali_sw[8]` e'
 *    la dichiarazione piu' piccola compatibile con entrambe le
 *    osservazioni, non una lettura del sorgente di fabbrica.
 */
struct mir3da_data {
	struct i2c_client	*client;	/* +0   */
	struct acc_hw		hw;		/* +8   */
	struct hwmsen_convert	cvt;		/* +56  */
	bool			flush;		/* +64  */
	atomic_t		campo_0x44;	/* +68  */
	atomic_t		suspended;	/* +72  */
	int			campo_0x4c;	/* +76  */
	s16			cali_sw[8];	/* +80  */
	s16			data[MIR3DA_AXES_NUM];	/* +96 */
};

/*
 * Le variabili di modulo.  Gli offset stanno tutti nella stessa pagina
 * 0xffffff8009cba000 e vengono dal disassemblato:
 *   +344 (0x158) mir3da_init_flag   "39456108 ldrb w8, [x8,#344]"
 *   +352 (0x160) mir3da_obj         "f900b113 str x19, [x8,#352]"
 *   +360 (0x168) mir3da_i2c_client  "f900b6b4 str x20, [x21,#360]"
 *   +368 (0x170) mir3da_handle      "f900b900 str x0, [x8,#368]"
 *   +376 (0x178) mir3da_sem         "9105e129 add x9, x9, #0x178"
 *   +400 (0x190) mir3da_sensor_power "394642e8 ldrb w8, [x23,#400]"
 *   +402/+404/+406 i tre flag di guadagno
 *
 * mir3da_init_flag e' un `bool` e non un `int`: mir3da_local_init lo legge
 * con "39456108 ldrb" e lo converte con "13000100 sbfx w0, w8, #0, #1",
 * che e' l'estensione di segno di un i1 — la forma che clang usa per
 * `flag ? -1 : 0` quando `flag` e' un bool.  Un `int` avrebbe dato un
 * `cmp` piu' `csetm`.
 *
 * mir3da_sem e' inizializzata staticamente a 1 dal probe, non da un
 * DEFINE_SEMAPHORE: "b26003ea orr x10, xzr, #0x100000000" mette lock=0 e
 * count=1, e le due store seguenti chiudono la wait_list su se' stessa —
 * cioe' sema_init(&sem, 1).
 */
static bool mir3da_init_flag;
static struct mir3da_data *mir3da_obj;
static struct i2c_client *mir3da_i2c_client;
static PLAT_HANDLE mir3da_handle;
static struct semaphore mir3da_sem;
static bool mir3da_sensor_power;

/*
 * I tre flag di guadagno.  Sono byte (`strb`/`ldrb`) usati come booleani:
 * in mir3da_factory_set_cali il prodotto per 1024 esce da
 * "1a9f116e csel w14, w11, wzr, ne" dopo "7100015f cmp w10, #0", che e'
 * la forma di `bool * 1024`.
 *
 * Divergenza dichiarata: nella .bss di fabbrica stanno a +402, +404, +406,
 * cioe' a due byte di distanza l'uno dall'altro, con un byte inutilizzato
 * in mezzo.  Tre `bool` separati come questi finirebbero adiacenti.  Non
 * so spiegare quel passo di due, quindi il mio layout .bss diverge qui e
 * lo dichiaro invece di inventare una struct che lo giustifichi.
 */
static bool mir3da_gain_x;
static bool mir3da_gain_y;
static bool mir3da_gain_z;

static int mir3da_local_init(void);
static int mir3da_local_remove(void);
static int mir3da_setPowerMode(struct i2c_client *client, bool on);
static int mir3da_flush(void);

/*
 * "mir3da"@0xffffff80091bf966 — la stessa stringa serve a .name qui e a
 * .driver.name della i2c_driver: il linker ne tiene una copia sola.
 * Il campo .platform_diver_addr (+24, 0xffffff800992eb78) e' NULL nel
 * binario di fabbrica.
 */
static struct acc_init_info mir3da_init_info = {
	.name = MIR3DA_DRV_NAME,
	.init = mir3da_local_init,
	.uninit = mir3da_local_remove,
};

/* ------------------------------------------------------------------ */
/* La tabella di operazioni che il core si fa passare                  */
/* ------------------------------------------------------------------ */

/*
 * Le quattro funzioni che seguono sono globali (`T` in stock.map) perche'
 * il core le prende per indirizzo dalla general_op_s: nessuna funzione del
 * blocco le chiama con una `bl`.
 */
int i2c_smbus_read(PLAT_HANDLE handle, u8 addr, u8 *data)
{
	*data = i2c_smbus_read_byte_data((struct i2c_client *)handle, addr);
	return 0;
}

int i2c_smbus_read_block(PLAT_HANDLE handle, u8 addr, u8 len, u8 *buf)
{
	return i2c_smbus_read_i2c_block_data((struct i2c_client *)handle,
					     addr, len, buf);
}

int i2c_smbus_write(PLAT_HANDLE handle, u8 addr, u8 data)
{
	return i2c_smbus_write_byte_data((struct i2c_client *)handle,
					 addr, data);
}

/*
 * "52912b00 mov w0, #0x8958" + "72a00820 movk w0, #0x41, lsl #16" =
 * 0x418958 = 4295000 = 1000 * 0x10C7, cioe' udelay(1000), dentro un ciclo
 * che conta da -msec a 0 ("cb20c113 sub x19, x8, w0, sxtw",
 * "b1000673 adds x19, x19, #0x1", "54ffff83 b.cc"): e' esattamente
 * l'espansione di mdelay() per un argomento non costante.
 */
void msdelay(int msec)
{
	mdelay(msec);
}

/* "chip init failed !\n\n"@0xffffff80091bf945 */
static int get_address(PLAT_HANDLE handle)
{
	if (handle == NULL) {
		MI_ERR("chip init failed !\n\n");
		return -1;
	}
	/* "79400400 ldrh w0, [x0,#2]": i2c_client.addr sta a +2. */
	return ((struct i2c_client *)handle)->addr;
}

/*
 * La struttura a 0xffffff800992eac0, slot per slot da .rela.dyn.  I
 * quattro slot che la fabbrica lascia a zero restano a zero qui: non ho
 * un nome per loro e non invento una funzione da metterci.
 */
static struct general_op_s mir3da_general_ops = {
	.smi2c_read = i2c_smbus_read,
	.smi2c_write = i2c_smbus_write,
	.smi2c_read_block = i2c_smbus_read_block,
	.get_address = get_address,
	.myprintk = printk,
	.mysprintf = sprintf,
	.msdelay = msdelay,
};

/* ------------------------------------------------------------------ */
/* Calibrazione — le tre funzioni incorporate (righe 119, 131, 146)     */
/* ------------------------------------------------------------------ */

/*
 * Riga 119 di fabbrica: "52800ee2 mov w2, #0x77" nel MI_FUN incorporato
 * in mir3da_factory_clear_cali, con __func__ =
 * 'mir3da_resetCalibration' a 0xffffff80091bfc70.
 * L'azzeramento e' "a9057e7f stp xzr, xzr, [x19,#80]", sedici byte.
 */
static int mir3da_resetCalibration(PLAT_HANDLE handle)
{
	struct mir3da_data *obj = i2c_get_clientdata(mir3da_i2c_client);

#line 119
	MI_FUN;
	memset(obj->cali_sw, 0, sizeof(obj->cali_sw));

	return 0;
}

/*
 * Riga 131: "52801062 mov w2, #0x83", __func__ =
 * 'mir3da_readCalibration' a 0xffffff80091bfdd0.  Le tre letture sono
 * "79c0a2a8 ldrsh w8, [x21,#80]", "79c0a6a9 ldrsh w9, [x21,#82]",
 * "79c0aaaa ldrsh w10, [x21,#84]".
 */
static int mir3da_readCalibration(PLAT_HANDLE handle, int *dat)
{
	struct mir3da_data *obj = i2c_get_clientdata(mir3da_i2c_client);

#line 131
	MI_FUN;
	dat[MIR3DA_AXIS_X] = obj->cali_sw[MIR3DA_AXIS_X];
	dat[MIR3DA_AXIS_Y] = obj->cali_sw[MIR3DA_AXIS_Y];
	dat[MIR3DA_AXIS_Z] = obj->cali_sw[MIR3DA_AXIS_Z];

	return 0;
}

/*
 * Riga 146: "52801242 mov w2, #0x92", __func__ =
 * 'mir3da_writeCalibration' a 0xffffff80091bfd2f.
 *
 * "null ptr!!\n\n"@0xffffff80091bfd52
 * "write_cali  raw cali_sw[%d][%d][%d] dat[%d][%d][%d]\n"@0xffffff80091bfd6a
 * "write_cali  new cali_sw[%d][%d][%d] \n"@0xffffff80091bfdaa
 *
 * L'attesa finale e' "52912b00 mov w0, #0x8958" + "72a00820 movk w0,
 * #0x41, lsl #16" = udelay(1000) = mdelay(1).
 */
static int mir3da_writeCalibration(PLAT_HANDLE handle, int *dat)
{
	struct mir3da_data *obj = i2c_get_clientdata(mir3da_i2c_client);
	int cali[MIR3DA_AXES_NUM];

#line 146
	MI_FUN;
	if (obj == NULL) {
		MI_ERR("null ptr!!\n\n");
		return -1;
	}

	mir3da_readCalibration(handle, cali);
	MI_MSG("write_cali  raw cali_sw[%d][%d][%d] dat[%d][%d][%d]\n",
	       cali[MIR3DA_AXIS_X], cali[MIR3DA_AXIS_Y], cali[MIR3DA_AXIS_Z],
	       dat[MIR3DA_AXIS_X], dat[MIR3DA_AXIS_Y], dat[MIR3DA_AXIS_Z]);

	obj->cali_sw[MIR3DA_AXIS_X] = cali[MIR3DA_AXIS_X] + dat[MIR3DA_AXIS_X];
	obj->cali_sw[MIR3DA_AXIS_Y] = cali[MIR3DA_AXIS_Y] + dat[MIR3DA_AXIS_Y];
	obj->cali_sw[MIR3DA_AXIS_Z] = cali[MIR3DA_AXIS_Z] + dat[MIR3DA_AXIS_Z];

	MI_MSG("write_cali  new cali_sw[%d][%d][%d] \n",
	       obj->cali_sw[MIR3DA_AXIS_X], obj->cali_sw[MIR3DA_AXIS_Y],
	       obj->cali_sw[MIR3DA_AXIS_Z]);
	mdelay(1);

	return 0;
}

/* ------------------------------------------------------------------ */
/* Gli attributi sysfs (righe ~300-500 del sorgente di fabbrica)        */
/* ------------------------------------------------------------------ */

/* Riga 307: "52802662 mov w2, #0x133".  "%d\n"@0xffffff80092ae2ec.
 * L'`enable` letto e' un byte: "394013e2 ldrb w2, [sp,#4]". */
static ssize_t mir3da_enable_show(struct device_driver *ddri, char *buf)
{
	int result;
	u8 enable;
	/*
	 * IL MANIGLIO SI LEGGE PRIMA DELLA STAMPA, e il binario lo dice: la
	 * fabbrica salva x20 ("a9014ff4 stp x20, x19, [sp,#16]" contro il nostro
	 * "f9000bf3 str x19") e carica il globale a
	 * "f940b934 ldr"@0xffffff80087899e0, cioe' PRIMA della
	 * "361800e8 tbz"@0xffffff80087899e4 che salta la printk di MI_FUN. Un
	 * valore vivo attraverso una chiamata e' un valore che il sorgente ha
	 * gia' letto.
	 */
	PLAT_HANDLE h = mir3da_handle;

#line 307
	MI_FUN;
	result = mir3da_get_enable(h, &enable);
	if (result < 0)
		return -EINVAL;

	return sprintf(buf, "%d\n", enable);
}

/*
 * "b40002c1 cbz x1, ..." — il primo controllo e' su `buf`, e il valore
 * restituito in quel caso e' "92800000 mov x0, #0xffffffffffffffff", -1.
 * La base della conversione e' "52800142 mov w2, #0xa", decimale.
 * "f100001f cmp x0, #0x0" + "1a9f07e1 cset w1, ne" e' la conversione a
 * bool dell'argomento passato a mir3da_set_enable.
 */
static ssize_t mir3da_enable_store(struct device_driver *ddri,
				   const char *buf, size_t count)
{
	unsigned long enable;
	PLAT_HANDLE h;

	if (buf == NULL)
		return -1;

	/*
	 * COME IN `mir3da_enable_show`: il maniglio si legge PRIMA della
	 * chiamata, non al momento di usarlo. La fabbrica lo carica a
	 * "f940b914 ldr"@0xffffff8008789a6c, cioe' prima della
	 * "941b7d60 bl"@0xffffff8008789a80 verso <simple_strtoul>, e lo tiene in
	 * x20 attraverso quella chiamata -- che e' proprio perche' salva x20
	 * ("a9be4ff4 stp x20, x19, [sp,#-32]!").
	 */
	h = mir3da_handle;

	enable = simple_strtoul(buf, NULL, 10);
	if (mir3da_set_enable(h, enable != 0) < 0)
		return -EINVAL;

	/*
	 * IL VALORE DI RITORNO PASSA PER UN `int`, e il binario lo dice:
	 * "93407e68 sxtw"@0xffffff8008789a98 estende con segno i 32 bit bassi di
	 * w19 (dove sta `count`) prima di sceglierlo. Con un `size_t` restituito
	 * direttamente quell'estensione non serve e clang non la emette -- erano
	 * i quattro byte di differenza.
	 */
	return (int)count;
}

/*
 * "x= %d;y=%d;z=%d\n"@0xffffff80091bfe5b
 * "reading failed!"@0xffffff80091bfe6c — sedici byte costanti che clang
 * copia con "a9402508 ldp x8, x9, [x8]" + "a9002668 stp x8, x9, [x19]" e
 * una lunghezza costante "32000fe0 orr w0, wzr, #0xf": e' una sprintf
 * senza specificatori, ridotta a copia.
 */
static ssize_t mir3da_axis_data_show(struct device_driver *ddri, char *buf)
{
	int result;
	short x, y, z;

	result = mir3da_read_data(mir3da_handle, &x, &y, &z);
	if (result == 0)
		return sprintf(buf, "x= %d;y=%d;z=%d\n", x, y, z);

	return sprintf(buf, "reading failed!");
}

static ssize_t mir3da_reg_data_show(struct device_driver *ddri, char *buf)
{
	return mir3da_get_reg_data(mir3da_handle, buf);
}

/*
 * Riga 376: "52802f02 mov w2, #0x178".
 * "0x%x, 0x%x\n"@0xffffff800916770a
 * "set[0x%x]->[0x%x]\n\n"@0xffffff80091bff4c
 * Il testo dell'espressione asserita, 'result==0' a 0xffffff80091bff37, e'
 * lo stringify di #expr: non lo scrivo come letterale, lo produce ASSERT.
 */
static ssize_t mir3da_reg_data_store(struct device_driver *ddri,
				     const char *buf, size_t count)
{
	int addr, data;
	int result;

	sscanf(buf, "0x%x, 0x%x\n", &addr, &data);
	result = mir3da_register_write(mir3da_handle, addr, data);
	ASSERT(result == 0);
	MI_MSG("set[0x%x]->[0x%x]\n\n", addr, data);

	return count;
}

/* "%d\n"@0xffffff80092ae2ec */
static ssize_t mir3da_log_level_show(struct device_driver *ddri, char *buf)
{
	return sprintf(buf, "%d\n", Log_level);
}

static ssize_t mir3da_log_level_store(struct device_driver *ddri,
				      const char *buf, size_t count)
{
	Log_level = simple_strtoul(buf, NULL, 10);

	return count;
}

/*
 * "x=%d ,y=%d ,z=%d\n"@0xffffff80091bff6f.  I tre interi sono azzerati
 * prima della chiamata — "f9000bff str xzr, [sp,#16]" e
 * "b9000fff str wzr, [sp,#12]" — e sono `int`, non `short`: la rilettura
 * e' "29420be3 ldp w3, w2, [sp,#16]".
 */
static ssize_t mir3da_primary_offset_show(struct device_driver *ddri, char *buf)
{
	int x = 0, y = 0, z = 0;

	mir3da_get_primary_offset(mir3da_handle, &x, &y, &z);

	return sprintf(buf, "x=%d ,y=%d ,z=%d\n", x, y, z);
}

/* "%s\n"@0xffffff8009226be1, "MiraMEMS"@0xffffff80091bff81 */
static ssize_t mir3da_vendor_show(struct device_driver *ddri, char *buf)
{
	return sprintf(buf, "%s\n", "MiraMEMS");
}

/* "%s_%s\n"@0xffffff80091bff8a */
static ssize_t mir3da_version_show(struct device_driver *ddri, char *buf)
{
	return sprintf(buf, "%s_%s\n", MIR3DA_DRIVER_VERSION,
		       MIR3DA_CORE_VERSION);
}

/*
 * I sette attributi, nell'ordine in cui mir3da_probe li crea e in cui
 * stanno nella .data di fabbrica (0xffffff800992ecb8, passo 0x20).
 * I permessi sono letti dal binario, non a occhio:
 *   +8 di 0xffffff800992ecb8 = 0x1a4 = 0644   enable
 *   +8 di 0xffffff800992ecd8 = 0x124 = 0444   axis_data
 *   +8 di 0xffffff800992ecf8 = 0x1a4 = 0644   reg_data
 *   +8 di 0xffffff800992ed18 = 0x1a4 = 0644   log_level
 *   +8 di 0xffffff800992ed38 = 0x124 = 0444   primary_offset
 *   +8 di 0xffffff800992ed58 = 0x124 = 0444   vendor
 *   +8 di 0xffffff800992ed78 = 0x124 = 0444   version
 * I nomi vengono da .rela.dyn: "enable"@0xffffff800924755d,
 * "axis_data"@0xffffff80091bfe51, "reg_data"@0xffffff80091bfe7c,
 * "log_level"@0xffffff8009258d49, "primary_offset"@0xffffff80091bff60,
 * "vendor"@0xffffff800923922c, "version"@0xffffff800926923d.
 * Gli slot .store di axis_data, primary_offset, vendor e version non sono
 * relocati: sono NULL.
 */
static struct driver_attribute driver_attr_enable = {
	.attr = { .name = "enable", .mode = 0644 },
	.show = mir3da_enable_show,
	.store = mir3da_enable_store,
};
static struct driver_attribute driver_attr_axis_data = {
	.attr = { .name = "axis_data", .mode = 0444 },
	.show = mir3da_axis_data_show,
};
static struct driver_attribute driver_attr_reg_data = {
	.attr = { .name = "reg_data", .mode = 0644 },
	.show = mir3da_reg_data_show,
	.store = mir3da_reg_data_store,
};
static struct driver_attribute driver_attr_log_level = {
	.attr = { .name = "log_level", .mode = 0644 },
	.show = mir3da_log_level_show,
	.store = mir3da_log_level_store,
};
static struct driver_attribute driver_attr_primary_offset = {
	.attr = { .name = "primary_offset", .mode = 0444 },
	.show = mir3da_primary_offset_show,
};
static struct driver_attribute driver_attr_vendor = {
	.attr = { .name = "vendor", .mode = 0444 },
	.show = mir3da_vendor_show,
};
static struct driver_attribute driver_attr_version = {
	.attr = { .name = "version", .mode = 0444 },
	.show = mir3da_version_show,
};

static struct driver_attribute *mir3da_attr_list[] = {
	&driver_attr_enable,
	&driver_attr_axis_data,
	&driver_attr_reg_data,
	&driver_attr_log_level,
	&driver_attr_primary_offset,
	&driver_attr_vendor,
	&driver_attr_version,
};

/* "driver_create_file (%s) = %d\n\n"@0xffffff80091bfe1f */
static int mir3da_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(mir3da_attr_list);

	if (driver == NULL)
		return -EINVAL;

	for (idx = 0; idx < num; idx++) {
		err = driver_create_file(driver, mir3da_attr_list[idx]);
		if (err) {
			MI_MSG("driver_create_file (%s) = %d\n\n",
			       mir3da_attr_list[idx]->attr.name, err);
			break;
		}
	}

	return err;
}

static int mir3da_delete_attr(struct device_driver *driver)
{
	int idx;
	int num = ARRAY_SIZE(mir3da_attr_list);

	if (driver == NULL)
		return -EINVAL;

	for (idx = 0; idx < num; idx++)
		driver_remove_file(driver, mir3da_attr_list[idx]);

	return 0;
}

/* ------------------------------------------------------------------ */
/* Sospensione (righe 538 e 566)                                       */
/* ------------------------------------------------------------------ */

/*
 * Riga 538: "52804342 mov w2, #0x21a".
 * "null pointer!!\n\n"@0xffffff80091c03d7
 * "write power control fail!!\n\n"@0xffffff80091c03f3
 * "d1008273 sub x19, x19, #0x20" e' to_i2c_client(dev): `struct device`
 * sta a +32 dentro `struct i2c_client`.
 */
static int mir3da_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mir3da_data *obj = dev_get_drvdata(dev);

#line 538
	MI_FUN;
	if (obj == NULL) {
		MI_ERR("null pointer!!\n\n");
		return -EINVAL;
	}

	atomic_set(&obj->suspended, 1);
	down_interruptible(&mir3da_sem);
	if (mir3da_setPowerMode(client, false)) {
		up(&mir3da_sem);
		MI_ERR("write power control fail!!\n\n");
		return -1;
	}
	up(&mir3da_sem);

	return 0;
}

/*
 * Riga 566: "528046c2 mov w2, #0x236".
 * "chip resume fail!!\n\n"@0xffffff80091c0429
 *
 * Divergenza di fabbrica riprodotta di proposito: sul ramo in cui
 * mir3da_chip_resume fallisce la fabbrica NON rilascia il semaforo —
 * "14000019 b ..." salta diritto all'uscita senza la `bl up`.  Il
 * disassemblato lo dice, quindi il codice qui fa lo stesso.
 */
static int mir3da_resume(struct device *dev)
{
	struct mir3da_data *obj = dev_get_drvdata(dev);
	int err;

#line 566
	MI_FUN;
	if (obj == NULL) {
		MI_ERR("null pointer!!\n\n");
		return -EINVAL;
	}

	down_interruptible(&mir3da_sem);
	err = mir3da_chip_resume(obj->client);
	if (err) {
		MI_ERR("chip resume fail!!\n\n");
		return err;
	}

	err = mir3da_setPowerMode(obj->client, true);
	up(&mir3da_sem);
	if (err) {
		MI_ERR("write power control fail!!\n\n");
		return err;
	}
	atomic_set(&obj->suspended, 0);

	return err;
}

/* ------------------------------------------------------------------ */
/* Il percorso di controllo e di dato (righe ~600-720)                 */
/* ------------------------------------------------------------------ */

/*
 * "mir3da_setPowerMode(), enable = %d\n"@0xffffff80091c00bd
 * "Sensor power status should not be set again!!!\n\n"@0xffffff80091c00ec
 * "remain flush, will call mir3da_flush in setPowerMode\n\n"@0xffffff80091c0128
 */
static int mir3da_setPowerMode(struct i2c_client *client, bool on)
{
	MI_MSG("mir3da_setPowerMode(), enable = %d\n", on);

	if (on == mir3da_sensor_power)
		MI_ERR("Sensor power status should not be set again!!!\n\n");

	if (mir3da_set_enable(client, on))
		return -1;

	mir3da_sensor_power = on;

	if (mir3da_obj->flush) {
		if (on) {
			MI_MSG("remain flush, will call mir3da_flush in setPowerMode\n\n");
			mir3da_flush();
		} else {
			mir3da_obj->flush = false;
		}
	}

	return 0;
}

static int mir3da_open_report_data(int open)
{
	return 0;
}

/*
 * "mir3da_enable_nodata 0\n"@0xffffff80091bffbb
 * "mir3da_SetPowerMode fail\n\n"@0xffffff80091c0004
 * "mir3da_SetPowerMode done\n\n"@0xffffff80091bffde
 * "mir3da_enable_nodata 1\n"@0xffffff80091c002a
 * "mir3da_SetPowerMode fail!\n\n"@0xffffff80091c004d
 * "mir3da_enable_nodata OK!\n\n"@0xffffff80091c0074
 * "mir3da_enable_nodata 2\n"@0xffffff80091c009a
 *
 * Il ciclo e' srotolato tre volte nel binario (tre `bl mir3da_setPowerMode`
 * a 0x8788e18, 0x8788e40, 0x8788e68).  L'argomento e'
 * "7100067f cmp w19, #0x1" + "1a9f17e1 cset w1, eq", cioe' `en == 1`.
 */
static int mir3da_enable_nodata(int en)
{
	int res = 0;
	int retry;
	bool power = false;

	if (en == 1)
		power = true;

	MI_ERR("mir3da_enable_nodata 0\n");

	for (retry = 0; retry < 3; retry++) {
		res = mir3da_setPowerMode(mir3da_i2c_client, power);
		if (res == 0) {
			MI_ERR("mir3da_SetPowerMode done\n\n");
			break;
		}
		MI_ERR("mir3da_SetPowerMode fail\n\n");
	}

	MI_ERR("mir3da_enable_nodata 1\n");

	if (res != 0) {
		MI_ERR("mir3da_SetPowerMode fail!\n\n");
		return -1;
	}

	MI_MSG("mir3da_enable_nodata OK!\n\n");
	MI_ERR("mir3da_enable_nodata 2\n");

	return 0;
}

/* "mir3da_batch 0\n"@0xffffff80091c016a */
static int mir3da_batch(int flag, int64_t samplingPeriodNs,
			int64_t maxBatchReportLatencyNs)
{
	MI_ERR("mir3da_batch 0\n");

	return 0;
}

/*
 * "mir3da_flush 0\n"@0xffffff80091c0185
 * "mir3da_flush 1\n"@0xffffff80091c01a0
 * Il ramo a sensore spento esce prima del secondo messaggio: nel binario
 * "340001a8 cbz w8, ..." porta a 0x8788fb0, che salta direttamente
 * all'epilogo.
 */
static int mir3da_flush(void)
{
	int err;

	MI_ERR("mir3da_flush 0\n");

	if (!mir3da_sensor_power) {
		mir3da_obj->flush = true;
		return 0;
	}

	err = acc_flush_report();
	if (err >= 0)
		mir3da_obj->flush = false;

	MI_ERR("mir3da_flush 1\n");

	return err;
}

/*
 * "mir3da_set_delay \n"@0xffffff80091c01bb
 * "mir3daset_delay (%d), chip only use 1024HZ\n\n"@0xffffff80091c01d9
 * La divisione e' con segno e a 32 bit — "529bd068 mov w8, #0xde83" +
 * "72a86368 movk w8, #0x431b, lsl #16" + "9b287e68 smull x8, w19, w8" +
 * "9372fd08 asr x8, x8, #50" e' il reciproco magico di 1000000 per un
 * `int`, non per il `u64` del parametro.
 */
static int mir3da_set_delay(u64 ns)
{
	int value = (int)ns / 1000 / 1000;

	MI_ERR("mir3da_set_delay \n");
	MI_MSG("mir3daset_delay (%d), chip only use 1024HZ\n\n", value);

	return 0;
}

/*
 * "Power on mir3da error %d!\n\n"@0xffffff80091c0211
 * "I2C error: ret value=%d\n"@0xffffff80091bfc0c
 * "read_sensor_data map[%d][%d][%d] sign[%d][%d][%d]\n"@0xffffff80091c0238
 * "read_sensor_data xyz_0[%d][%d][%d] cali_sw[%d][%d][%d]\n"@0xffffff80091c0276
 * "read_sensor_data xyz_1[%d][%d][%d]\n"@0xffffff80091c02b9
 * "read_sensor_data xyz_2[%d][%d][%d]\n"@0xffffff80091c02e8
 * "mir3da data map: %d, %d, %d!\n\n"@0xffffff80091c0317
 * "read_sensor_data xyz_3[%d][%d][%d]\n"@0xffffff80091c0341
 * "%04x %04x %04x"@0xffffff80091bfc25
 * "mir3da data mg: x= %d, y=%d, z=%d\n\n"@0xffffff80091c0370
 *
 * Nel kernel di fabbrica questa funzione non ha un simbolo proprio: e'
 * incorporata in mir3da_get_data e in nessun altro punto.  Il valore che
 * restituisce non e' osservabile — mir3da_get_data lo ignora — quindi
 * `return res` qui e' una scelta mia, non una lettura.
 */
static int mir3da_ReadSensorData(struct i2c_client *client, char *buf,
				 int bufsize)
{
	struct mir3da_data *obj;
	int res;
	/*
	 * NON inizializzato: in mir3da_get_data l'array sta a x29-0x6c e il
	 * disassemblato non ha una sola store di zero verso quell'area.  Un
	 * `= {0}` produrrebbe l'azzeramento, perche' clang non puo' provare
	 * che obj->cvt.map[] sia una permutazione e quindi che tutte e tre
	 * le celle vengano scritte.
	 */
	int databuf[MIR3DA_AXES_NUM];

	if (client == NULL) {
		*buf = 0;
		return 0;
	}
	obj = i2c_get_clientdata(client);

	if (!mir3da_sensor_power) {
		res = mir3da_setPowerMode(client, true);
		if (res)
			MI_ERR("Power on mir3da error %d!\n\n", res);
		msleep(MIR3DA_POWERON_DELAY_MS);
	}

	res = mir3da_read_data(client, &obj->data[MIR3DA_AXIS_X],
			       &obj->data[MIR3DA_AXIS_Y],
			       &obj->data[MIR3DA_AXIS_Z]);
	if (res) {
		MI_ERR("I2C error: ret value=%d\n", res);
		return res;
	}

	MI_MSG("read_sensor_data map[%d][%d][%d] sign[%d][%d][%d]\n",
	       obj->cvt.map[MIR3DA_AXIS_X], obj->cvt.map[MIR3DA_AXIS_Y],
	       obj->cvt.map[MIR3DA_AXIS_Z], obj->cvt.sign[MIR3DA_AXIS_X],
	       obj->cvt.sign[MIR3DA_AXIS_Y], obj->cvt.sign[MIR3DA_AXIS_Z]);
	MI_MSG("read_sensor_data xyz_0[%d][%d][%d] cali_sw[%d][%d][%d]\n",
	       obj->data[MIR3DA_AXIS_X], obj->data[MIR3DA_AXIS_Y],
	       obj->data[MIR3DA_AXIS_Z], obj->cali_sw[MIR3DA_AXIS_X],
	       obj->cali_sw[MIR3DA_AXIS_Y], obj->cali_sw[MIR3DA_AXIS_Z]);

	obj->data[MIR3DA_AXIS_X] +=
		obj->cvt.sign[MIR3DA_AXIS_X] * obj->cali_sw[obj->cvt.map[MIR3DA_AXIS_X]];
	obj->data[MIR3DA_AXIS_Y] +=
		obj->cvt.sign[MIR3DA_AXIS_Y] * obj->cali_sw[obj->cvt.map[MIR3DA_AXIS_Y]];
	obj->data[MIR3DA_AXIS_Z] +=
		obj->cvt.sign[MIR3DA_AXIS_Z] * obj->cali_sw[obj->cvt.map[MIR3DA_AXIS_Z]];

	MI_MSG("read_sensor_data xyz_1[%d][%d][%d]\n",
	       obj->data[MIR3DA_AXIS_X], obj->data[MIR3DA_AXIS_Y],
	       obj->data[MIR3DA_AXIS_Z]);

	databuf[obj->cvt.map[MIR3DA_AXIS_X]] =
		obj->cvt.sign[MIR3DA_AXIS_X] * obj->data[MIR3DA_AXIS_X];
	databuf[obj->cvt.map[MIR3DA_AXIS_Y]] =
		obj->cvt.sign[MIR3DA_AXIS_Y] * obj->data[MIR3DA_AXIS_Y];
	databuf[obj->cvt.map[MIR3DA_AXIS_Z]] =
		obj->cvt.sign[MIR3DA_AXIS_Z] * obj->data[MIR3DA_AXIS_Z];

	MI_MSG("read_sensor_data xyz_2[%d][%d][%d]\n",
	       databuf[obj->cvt.map[MIR3DA_AXIS_X]],
	       databuf[obj->cvt.map[MIR3DA_AXIS_Y]],
	       databuf[obj->cvt.map[MIR3DA_AXIS_Z]]);

	if (abs(obj->cali_sw[MIR3DA_AXIS_Z]) > MIR3DA_CALI_Z_LIMIT)
		databuf[obj->cvt.map[MIR3DA_AXIS_Z]] -= MIR3DA_CALI_Z_ADJUST;

	MI_DATA("mir3da data map: %d, %d, %d!\n\n", databuf[MIR3DA_AXIS_X],
		databuf[MIR3DA_AXIS_Y], databuf[MIR3DA_AXIS_Z]);

	databuf[MIR3DA_AXIS_X] =
		databuf[MIR3DA_AXIS_X] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;
	databuf[MIR3DA_AXIS_Y] =
		databuf[MIR3DA_AXIS_Y] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;
	databuf[MIR3DA_AXIS_Z] =
		databuf[MIR3DA_AXIS_Z] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;

	MI_MSG("read_sensor_data xyz_3[%d][%d][%d]\n", databuf[MIR3DA_AXIS_X],
	       databuf[MIR3DA_AXIS_Y], databuf[MIR3DA_AXIS_Z]);

	sprintf(buf, "%04x %04x %04x", databuf[MIR3DA_AXIS_X],
		databuf[MIR3DA_AXIS_Y], databuf[MIR3DA_AXIS_Z]);

	MI_DATA("mir3da data mg: x= %d, y=%d, z=%d\n\n", databuf[MIR3DA_AXIS_X],
		databuf[MIR3DA_AXIS_Y], databuf[MIR3DA_AXIS_Z]);

	return res;
}

/*
 * "%x %x %x"@0xffffff80091411f7.
 * Lo `status` restituito e' "321f03e8 orr w8, wzr, #0x2", cioe' 2:
 * SENSOR_STATUS_ACCURACY_MEDIUM.
 */
static int mir3da_get_data(int *x, int *y, int *z, int *status)
{
	char buff[MIR3DA_BUFSIZE];

	down_interruptible(&mir3da_sem);
	mir3da_ReadSensorData(mir3da_i2c_client, buff, MIR3DA_BUFSIZE);
	up(&mir3da_sem);

	sscanf(buff, "%x %x %x", x, y, z);
	*status = SENSOR_STATUS_ACCURACY_MEDIUM;

	return 0;
}

/* ------------------------------------------------------------------ */
/* Il percorso di fabbrica (righe 725-793)                             */
/* ------------------------------------------------------------------ */

/*
 * Riga 725: "52805aa2 mov w2, #0x2d5".
 * "%s enable sensor failed!\n\n"@0xffffff80091bfb53
 * Il messaggio di mir3da_batch che si vede in coda a questa funzione e'
 * la mir3da_batch incorporata: il suo valore di ritorno e' costante 0,
 * quindi clang ne ha eliminato il controllo e nessun secondo messaggio
 * d'errore esiste nel binario.
 */
static int mir3da_factory_enable_sensor(bool enabledisable,
					int64_t sample_periods_ms)
{
	int err;

#line 725
	MI_FUN;
	err = mir3da_enable_nodata(enabledisable);
	if (err) {
		MI_ERR("%s enable sensor failed!\n\n", __func__);
		return -1;
	}

	mir3da_batch(0, sample_periods_ms, 0);

	return 0;
}

/*
 * Riga 745: "52805d22 mov w2, #0x2e9".
 * "mir3da_factory_get_data %d %d %d\n"@0xffffff80091bfb91
 */
static int mir3da_factory_get_data(int32_t data[3], int *status)
{
#line 745
	MI_FUN;
	mir3da_get_data(&data[0], &data[1], &data[2], status);
	MI_MSG("mir3da_factory_get_data %d %d %d\n", data[0], data[1], data[2]);

	return 0;
}

/*
 * Riga 759: "52805ee2 mov w2, #0x2f7".
 * "I2C error: ret value=%d\n"@0xffffff80091bfc0c
 * "support mir3da_factory_get_raw_data!\n\n"@0xffffff80091bfbda
 * "%04x %04x %04x"@0xffffff80091bfc25
 *
 * Difetto di fabbrica riprodotto: la funzione formatta i tre assi in un
 * buffer locale e non scrive mai in `data[3]`, che e' il parametro di
 * uscita.  Nel binario non esiste nessuna store su x0.
 */
static int mir3da_factory_get_raw_data(int32_t data[3])
{
	struct mir3da_data *obj;
	char strbuf[MIR3DA_BUFSIZE] = {0};
	int res;

#line 759
	MI_FUN;
	if (mir3da_i2c_client != NULL) {
		obj = i2c_get_clientdata(mir3da_i2c_client);
		res = mir3da_read_data(mir3da_i2c_client,
				       &obj->data[MIR3DA_AXIS_X],
				       &obj->data[MIR3DA_AXIS_Y],
				       &obj->data[MIR3DA_AXIS_Z]);
		if (res)
			MI_ERR("I2C error: ret value=%d\n", res);
		else
			sprintf(strbuf, "%04x %04x %04x",
				obj->data[MIR3DA_AXIS_X],
				obj->data[MIR3DA_AXIS_Y],
				obj->data[MIR3DA_AXIS_Z]);
	}

	MI_MSG("support mir3da_factory_get_raw_data!\n\n");

	return 0;
}

/* Riga 769: "52806022 mov w2, #0x301". */
static int mir3da_factory_enable_calibration(void)
{
#line 769
	MI_FUN;

	return 0;
}

/* Riga 778: "52806142 mov w2, #0x30a". */
static int mir3da_factory_clear_cali(void)
{
#line 778
	MI_FUN;
	mir3da_resetCalibration(mir3da_handle);

	return 0;
}

/*
 * Riga 793: "52806322 mov w2, #0x319".
 * "mir3da_factory_set_cali ori %d %d %d\n"@0xffffff80091bfcab
 * "mir3da_factory_set_cali new %d %d %d\n"@0xffffff80091bfcdc
 * "mir3da_WriteCalibration failed!\n\n"@0xffffff80091bfd0d
 *
 * La conversione da mg a LSB e' "529a4bed mov w13, #0xd25f" +
 * "72ad5d6d movk w13, #0x6aeb, lsl #16" + "9b2d7d4a smull x10, w10, w13" +
 * "936cfd4a asr x10, x10, #44": il reciproco magico di 9807, cioe'
 * la divisione per GRAVITY_EARTH_1000 del prodotto per il guadagno.
 */
static int mir3da_factory_set_cali(int32_t offset[3])
{
	int err;
	int cali[MIR3DA_AXES_NUM];

#line 793
	MI_FUN;
	MI_MSG("mir3da_factory_set_cali ori %d %d %d\n", offset[0], offset[1],
	       offset[2]);

	cali[MIR3DA_AXIS_X] = offset[MIR3DA_AXIS_X] *
			      (mir3da_gain_x * MIR3DA_GAIN) / GRAVITY_EARTH_1000;
	cali[MIR3DA_AXIS_Y] = offset[MIR3DA_AXIS_Y] *
			      (mir3da_gain_y * MIR3DA_GAIN) / GRAVITY_EARTH_1000;
	cali[MIR3DA_AXIS_Z] = offset[MIR3DA_AXIS_Z] *
			      (mir3da_gain_z * MIR3DA_GAIN) / GRAVITY_EARTH_1000;

	MI_MSG("mir3da_factory_set_cali new %d %d %d\n", cali[MIR3DA_AXIS_X],
	       cali[MIR3DA_AXIS_Y], cali[MIR3DA_AXIS_Z]);

	err = mir3da_writeCalibration(mir3da_handle, cali);
	if (err) {
		MI_ERR("mir3da_WriteCalibration failed!\n\n");
		return -1;
	}

	return 0;
}

/*
 * "mir3da_factory_get_cali %d %d %d\n"@0xffffff80091bfdf2.
 * La conversione inversa e' "5284c9eb mov w11, #0x264f" +
 * "110ffd0b add w11, w8, #0x3ff" + "130a7d01 asr w1, w8, #10":
 * moltiplicazione per 9807 e divisione con segno per 1024.
 */
static int mir3da_factory_get_cali(int32_t offset[3])
{
	int cali[MIR3DA_AXES_NUM];

	mir3da_readCalibration(mir3da_handle, cali);

	offset[MIR3DA_AXIS_X] =
		cali[MIR3DA_AXIS_X] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;
	offset[MIR3DA_AXIS_Y] =
		cali[MIR3DA_AXIS_Y] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;
	offset[MIR3DA_AXIS_Z] =
		cali[MIR3DA_AXIS_Z] * GRAVITY_EARTH_1000 / MIR3DA_GAIN;

	MI_MSG("mir3da_factory_get_cali %d %d %d\n", offset[MIR3DA_AXIS_X],
	       offset[MIR3DA_AXIS_Y], offset[MIR3DA_AXIS_Z]);

	return 0;
}

static int mir3da_factory_do_self_test(void)
{
	return 0;
}

/*
 * La tabella a 0xffffff800992ec78, otto puntatori nell'ordine in cui
 * .rela.dyn li elenca.  La struttura che la contiene sta a
 * 0xffffff800992ec68 e vale { gain = 1, sensitivity = 1, fops = &tabella }:
 * i due interi si leggono diretti nella .data, "01 00 00 00 01 00 00 00".
 */
static struct accel_factory_fops mir3da_factory_fops = {
	.enable_sensor = mir3da_factory_enable_sensor,
	.get_data = mir3da_factory_get_data,
	.get_raw_data = mir3da_factory_get_raw_data,
	.enable_calibration = mir3da_factory_enable_calibration,
	.clear_cali = mir3da_factory_clear_cali,
	.set_cali = mir3da_factory_set_cali,
	.get_cali = mir3da_factory_get_cali,
	.do_self_test = mir3da_factory_do_self_test,
};

static struct accel_factory_public mir3da_factory_device = {
	.gain = 1,
	.sensitivity = 1,
	.fops = &mir3da_factory_fops,
};

/* ------------------------------------------------------------------ */
/* probe / remove (riga 872) e registrazione (righe 1045-1074)          */
/* ------------------------------------------------------------------ */

/*
 * "[%s] gain: %d  %d  %d\n\n"@0xffffff80091bfb04 — il __func__ che la
 * accompagna e' 'mir3da_SetGain' a 0xffffff80091bfb1c, quindi nel sorgente
 * di fabbrica questa e' una funzione a se', incorporata in mir3da_probe.
 * I tre argomenti sono costanti nel binario ("321603e2 orr w2, wzr,
 * #0x400" e le due seguenti): 1024 tre volte, cioe' i tre flag appena
 * messi a vero moltiplicati per MIR3DA_GAIN.
 */
static int mir3da_SetGain(void)
{
	mir3da_gain_x = true;
	mir3da_gain_y = true;
	mir3da_gain_z = true;

	MI_MSG("[%s] gain: %d  %d  %d\n\n", __func__,
	       mir3da_gain_x * MIR3DA_GAIN, mir3da_gain_y * MIR3DA_GAIN,
	       mir3da_gain_z * MIR3DA_GAIN);

	return 0;
}

/*
 * Riga 872: "52806d02 mov w2, #0x368".
 * "kzalloc failed!\n"@0xffffff80091bf9b5
 * "get cust_baro dts info fail\n\n"@0xffffff80091bf9d1
 * "invalid direction: %d\n\n"@0xffffff80091bf9fa
 * "Install ops failed !\n\n"@0xffffff80091bfa1d
 * "chip init failed !\n\n"@0xffffff80091bf945
 * "acc_factory register failed.\n\n"@0xffffff80091bfa3f
 * "create attribute result = %d\n\n"@0xffffff80091bfa69
 * "register acc control path err\n\n"@0xffffff80091bfa93
 * "register acc data path err= %d\n\n"@0xffffff80091bfabe
 * "%s: err = %d\n\n"@0xffffff80091bfaea
 *
 * Due cose che il binario dice e che sembrano sbagliate ma sono quelle:
 *  - sul fallimento di mir3da_install_general_ops e su quello di
 *    mir3da_core_init il codice mette err a ZERO ("2a1f03f5 mov w21, wzr")
 *    prima di uscire: probe ritorna 0 pur avendo fallito;
 *  - mir3da_create_attr riceve
 *    &mir3da_init_info.platform_diver_addr->driver, cioe' un puntatore
 *    caricato da 0xffffff800992eb78 (che nel binario e' NULL) piu' 0x28:
 *    "f945bd08 ldr x8, [x8,#2936]" + "b100a116 adds x22, x8, #0x28".
 *    Il confronto con NULL che segue e' il `if (driver == NULL)` dentro
 *    mir3da_create_attr, ed e' vero solo se il puntatore vale -0x28.
 *  - il vender_div passato a acc_register_data_path e'
 *    "52807d09 mov w9, #0x3e8" = 1000.
 */
static int mir3da_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct mir3da_data *obj;
	struct acc_control_path ctl = {0};
	struct acc_data_path data = {0};
	int err = 0;
	int i;

#line 872
	MI_FUN;

	obj = kzalloc(sizeof(struct mir3da_data), GFP_KERNEL);
	if (obj == NULL) {
		MI_ERR("kzalloc failed!\n");
		err = -ENOMEM;
		goto exit;
	}

	err = get_accel_dts_func(client->dev.of_node, &obj->hw);
	if (err < 0) {
		MI_ERR("get cust_baro dts info fail\n\n");
		goto exit_kfree;
	}

	err = hwmsen_get_convert(obj->hw.direction, &obj->cvt);
	if (err) {
		MI_ERR("invalid direction: %d\n\n", obj->hw.direction);
		goto exit_kfree;
	}

	if (client->addr != MIR3DA_I2C_ADDR_A)
		client->addr = MIR3DA_I2C_ADDR_A;

	obj->client = client;
	i2c_set_clientdata(client, obj);
	atomic_set(&obj->campo_0x44, 0);
	atomic_set(&obj->suspended, 0);
	mir3da_obj = obj;
	mir3da_i2c_client = client;
	sema_init(&mir3da_sem, 1);

	if (mir3da_install_general_ops(&mir3da_general_ops)) {
		MI_ERR("Install ops failed !\n\n");
		err = 0;
		goto exit_kfree;
	}

	/*
	 * La forma di questo ciclo non e' indifferente, ed e' stata scelta
	 * misurando.  Di fabbrica il ciclo e' srotolato per intero — sei
	 * `bl i2c_smbus_read_byte_data` e cinque `bl __const_udelay`, da
	 * 0xffffff80087889cc a 0xffffff8008788a9c, 212 byte.  Due forme C
	 * semanticamente identiche danno risultati diversi con questo clang:
	 * mettendo la store dell'indirizzo di ripiego DENTRO il ciclo
	 * (`if (i == RETRY - 1) { client->addr = ...; break; }`) clang non
	 * srotola e produce 84 byte, con solo DUE `bl i2c_smbus_read_byte_data`
	 * (verificato compilando la variante); tenendola fuori, come qui,
	 * srotola e produce 212 byte identici a quelli di fabbrica in NUMERO,
	 * ORDINE e STRUTTURA delle istruzioni — NON negli encoding: 6 dei 53
	 * slot hanno qui "52800021 mov w1, #0x1" dove la fabbrica ha
	 * "320003e1 orr w1, wzr, #0x1" (la stessa sostituzione di compilatore
	 * clang-r353983c -> clang-r383902 documentata in HANDOFF.md §5 e qui
	 * sopra nella misura), confermato anche nell'oggetto kbuild vero e non
	 * solo nella misura isolata (revisione indipendente, rilievo R1).
	 * Non e' una scelta fatta per far tornare un numero — mir3da_probe
	 * resta a -8 byte anche cosi' —
	 * ma perche' il NUMERO di `bl i2c_smbus_read_byte_data` (sei contro
	 * due) e' il fatto piu' visibile che un disassemblato possa mostrare,
	 * e la forma srotolata e' quella che lo riproduce.
	 */
	for (i = 0; i < MIR3DA_CHIP_ID_RETRY; i++) {
		if ((i2c_smbus_read_byte_data(mir3da_i2c_client,
					      MIR3DA_REG_CHIP_ID) & 0xff)
		    == MIR3DA_CHIP_ID)
			break;
		if (i < MIR3DA_CHIP_ID_RETRY - 1)
			mdelay(MIR3DA_CHIP_ID_RETRY_MS);
	}
	if (i == MIR3DA_CHIP_ID_RETRY)
		client->addr = MIR3DA_I2C_ADDR_B;

	mir3da_handle = mir3da_core_init(mir3da_i2c_client);
	if (mir3da_handle == NULL) {
		MI_ERR("chip init failed !\n\n");
		err = 0;
		goto exit_kfree;
	}

	mir3da_SetGain();

	ctl.is_use_common_factory = false;
	err = accel_factory_device_register(&mir3da_factory_device);
	if (err) {
		MI_ERR("acc_factory register failed.\n\n");
		goto exit_kfree;
	}

	err = mir3da_create_attr(&mir3da_init_info.platform_diver_addr->driver);
	if (err) {
		MI_ERR("create attribute result = %d\n\n", err);
		err = -EINVAL;
		goto exit_kfree;
	}

	ctl.open_report_data = mir3da_open_report_data;
	ctl.enable_nodata = mir3da_enable_nodata;
	ctl.set_delay = mir3da_set_delay;
	ctl.batch = mir3da_batch;
	ctl.flush = mir3da_flush;
	ctl.is_report_input_direct = false;
	ctl.is_support_batch = obj->hw.is_batch_supported;

	err = acc_register_control_path(&ctl);
	if (err) {
		MI_ERR("register acc control path err\n\n");
		goto exit_kfree;
	}

	data.get_data = mir3da_get_data;
	data.vender_div = 1000;
	err = acc_register_data_path(&data);
	if (err) {
		MI_ERR("register acc data path err= %d\n\n", err);
		goto exit_kfree;
	}

	mir3da_init_flag = false;

	return err;

exit_kfree:
	kfree(obj);
exit:
	MI_ERR("%s: err = %d\n\n", __func__, err);
	mir3da_handle = NULL;
	mir3da_i2c_client = NULL;
	mir3da_obj = NULL;
	mir3da_init_flag = true;

	return err;
}

/* "mir3da_delete_attr fail: %d\n\n"@0xffffff80091c039f */
static int mir3da_remove(struct i2c_client *client)
{
	int err;

	err = mir3da_delete_attr(&mir3da_init_info.platform_diver_addr->driver);
	if (err)
		MI_ERR("mir3da_delete_attr fail: %d\n\n", err);

	mir3da_i2c_client = NULL;
	i2c_unregister_device(client);
	accel_factory_device_deregister(&mir3da_factory_device);
	kfree(i2c_get_clientdata(client));

	return 0;
}

/*
 * "mediatek,gsensor"@0xffffff8008f53f28 e' il compatible del nodo del
 * Device Tree: quell'indirizzo e' l'offset +64 (il campo `compatible`) della
 * struct of_device_id a 0xffffff8008f53ee8 puntata da
 * .driver.of_match_table (0xffffff800992ebe8).  La tabella ha due voci —
 * fra il suo inizio e la struct dev_pm_ops a 0xffffff8008f54078 ci sono
 * esattamente 400 byte, cioe' 2 * sizeof(struct of_device_id).
 */
static const struct of_device_id gsensor_of_match[] = {
	{ .compatible = "mediatek,gsensor" },
	{},
};

/*
 * "mir3da" — la copia dentro la i2c_device_id sta a 0xffffff8008f54130 e
 * la tabella occupa 64 byte (due voci da 32) fino a 0xffffff8008f54170.
 */
static const struct i2c_device_id mir3da_i2c_id[] = {
	{ MIR3DA_DRV_NAME, 0 },
	{},
};

/*
 * La struct dev_pm_ops a 0xffffff8008f54078 ha relocati sei slot:
 * +16 e +48 -> mir3da_suspend, +24 e +56 -> mir3da_resume, +32 ->
 * mir3da_suspend, +40 -> mir3da_resume, cioe' suspend/resume/freeze/thaw/
 * poweroff/restore.  E' esattamente cio' che produce SIMPLE_DEV_PM_OPS.
 */
static SIMPLE_DEV_PM_OPS(mir3da_pm_ops, mir3da_suspend, mir3da_resume);

/*
 * La i2c_driver a 0xffffff800992eb80: .probe a +16 e .remove a +24 (in
 * questo sublevel struct i2c_driver ha ancora attach_adapter a +8),
 * .driver a +64, .id_table a +184.  Il campo .class (+0) non e'
 * inizializzato.
 */
static struct i2c_driver mir3da_i2c_driver = {
	.probe = mir3da_probe,
	.remove = mir3da_remove,
	.id_table = mir3da_i2c_id,
	.driver = {
		.name = MIR3DA_DRV_NAME,
		.of_match_table = gsensor_of_match,
		.pm = &mir3da_pm_ops,
	},
};

/*
 * Riga 1045: "528082a2 mov w2, #0x415".
 * "add driver error\n\n"@0xffffff80091bf98a
 * Il primo argomento di i2c_register_driver e' "aa1f03e0 mov x0, xzr",
 * cioe' THIS_MODULE con il driver compilato dentro il kernel: e'
 * i2c_add_driver().
 */
static int mir3da_local_init(void)
{
#line 1045
	MI_FUN;

	if (i2c_add_driver(&mir3da_i2c_driver)) {
		MI_ERR("add driver error\n\n");
		return -1;
	}

	return mir3da_init_flag ? -1 : 0;
}

/* Riga 1063: "528084e2 mov w2, #0x427". */
static int mir3da_local_remove(void)
{
#line 1063
	MI_FUN;
	i2c_del_driver(&mir3da_i2c_driver);

	return 0;
}

/*
 * Riga 1074: "52808642 mov w2, #0x432".  La funzione sta in .init.text
 * (0xffffff80093740fc), quindi e' __init, e ignora il valore restituito
 * da acc_driver_add: "2a1f03e0 mov w0, wzr" e' l'unico valore che ritorna.
 *
 * **CORREZIONE del 2026-08-21.** Questo commento diceva: «Nel kernel di
 * fabbrica non esiste nessun simbolo mir3da_exit: per il codice compilato
 * dentro il kernel una module_exit finisce in .exit.text ed e' scartata dal
 * link». Meta' giusta e meta' sbagliata, e la meta' sbagliata e' quella che
 * contava. Giusto: nessun simbolo `mir3da_exit` e' in stock.map, e non ci
 * poteva essere -- stock.map e' `nm -n` di uno stock.elf la cui symtab viene
 * dalla tabella kallsyms dell'immagine, e kallsyms con
 * `# CONFIG_KALLSYMS_ALL is not set` emette solo i simboli in
 * [_stext,_etext] e [_sinittext,_einittext]
 * (alps-mtkwatch/scripts/kallsyms.c righe 48-51), mentre `.exit.text`
 * comincia esattamente a `_einittext` = 0xffffff80093a8518. Sbagliato: la
 * sezione NON e' scartata dal link. Su arm64
 * `arch/arm64/kernel/vmlinux.lds.S` righe 18-19 definiscono
 * `#define ARM_EXIT_KEEP(x) x` incondizionatamente, con il commento
 * «.exit.text needed in case of alternative patching».
 *
 * **La funzione c'e', e il binario la NOMINA da se'.** 0xffffff80093aa528,
 * 52 byte:
 *   "90002c28 adrp"@0xffffff80093aa528  adrp x8, 0xffffff800992e000
 *   "39766108 ldrb"@0xffffff80093aa52c  ldrb w8,[x8,#3480] -> Log_level a
 *                                       0xffffff800992ed98, la stessa cella e
 *                                       la stessa larghezza di lettura di
 *                                       mir3da_init ("39766108 ldrb"
 *                                       @0xffffff8009374108)
 *   "36180148 tbz"@0xffffff80093aa530   tbz w8, #3 -> il bit MIR_FUN
 *   "91242c00 add"@0xffffff80093aa544   add x0,x0,#0x90b -> 0xffffff80091bf90b
 *                        '\x013[MIR3DA] %s is called, line: %d\n'
 *   "9124b821 add"@0xffffff80093aa548   add x1,x1,#0x92e -> 0xffffff80091bf92e
 *                        'mir3da_exit'  <-- il __func__: **il nome e' misurato,
 *                                       non scelto.** La stringa esiste una
 *                                       volta sola in tutta l'immagine.
 *   "52808762 mov"@0xffffff80093aa54c   mov w2, #0x43b = 1083, il numero di
 *                                       riga di fabbrica (mir3da_init sta a
 *                                       1074: nove righe piu' su)
 *   "97b623e1 bl"@0xffffff80093aa550    -> ffffff80081334d4 <printk>
 *   "d65f03c0 ret"@0xffffff80093aa558
 * L'ordine e' quello giusto: x0 e' il formato, x1 il nome della funzione.
 *
 * Il corpo e' **solo** MI_FUN: nessuna acc_driver_del, nessuna
 * i2c_del_driver, nessun altro accesso a memoria. E' coerente con
 * mir3da_init, che pure si limita a MI_FUN + acc_driver_add: il framework
 * sensori MTK non espone alcuna `acc_driver_del` (cercata in stock.map: non
 * esiste), quindi non c'e' niente da disfare.
 *
 * Il `module_exit()` scritto qui sotto e' una SCELTA dichiarata: il puntatore
 * che genera finisce in `.exitcall.exit`, che vmlinux.lds scarta (`EXIT_CALL`
 * dentro /DISCARD/, riga 98), mentre la funzione sopravvive comunque perche'
 * per un built-in `__exit` implica `__used` (include/linux/init.h righe
 * 78-85). Dal binario «con module_exit()» e «senza» sono indistinguibili.
 * La funzione e il suo nome sono invece misurati.
 *
 * MISURA DI DIMENSIONE NON FATTA: questo lotto non ha un albero di build per
 * mir3da. I 52 byte di fabbrica sono letti dal disassemblato; il confronto
 * con il nostro `.o` va ancora fatto. Il numero di riga stampato sara' quello
 * di QUESTO file e non 1083, come gia' accade per mir3da_init.
 */
static int __init mir3da_init(void)
{
#line 1074
	MI_FUN;
	acc_driver_add(&mir3da_init_info);

	return 0;
}

module_init(mir3da_init);

static void __exit mir3da_exit(void)
{
	MI_FUN;
}

module_exit(mir3da_exit);

/*
 * MODULE_VERSION compilata built-in lascia la struct
 * module_version_attribute a 0xffffff800992eb18, con .attr.name
 * "version"@0xffffff800926923d, .attr.mode 0x124 = 0444, .show
 * __modver_version_show, .module_name 'mir3da_cust' a 0xffffff80091c0452 e
 * .version "1.0"@0xffffff80091bff91.  E' la prova diretta sia della
 * direttiva sia del nome del file oggetto.
 *
 * Di MODULE_LICENSE, MODULE_AUTHOR e MODULE_DESCRIPTION non c'e' traccia
 * osservabile nel binario di fabbrica (per il codice built-in finiscono in
 * .modinfo e non lasciano strutture), quindi qui c'e' solo la licenza, che
 * il kernel richiede.
 */
MODULE_VERSION(MIR3DA_DRIVER_VERSION);
MODULE_LICENSE("GPL");
