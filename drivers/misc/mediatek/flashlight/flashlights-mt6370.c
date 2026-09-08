/*
 * flashlights-mt6370.c -- ALPS con TRE innesti letti dal binario di fabbrica
 * ============================================================================
 *
 * COSA CONTIENE
 *   Il file `drivers/misc/mediatek/flashlight/flashlights-mt6370.c` dell'albero
 *   ALPS, invariato, PIU' tre modifiche derivate dal disassemblato del kernel
 *   di fabbrica del Doogee S88 Pro.  Non e' un driver riscritto: e' un driver
 *   FATTO COMBACIARE.  Ogni innesto porta accanto le citazioni verificabili.
 *
 * IL CONFINE
 *   Le tre modifiche stanno in `mt6370_enable`, in `mt6370_disable_ch1` (che
 *   nel binario esiste solo dentro `mt6370_disable`) e in `mt6370_operate`.
 *   Il resto del file non e' stato toccato.  Il conteggio, contato e non
 *   stimato:
 *     - dentro il blocco della mappa (0xffffff800875b5f8..0xffffff8008760ce4)
 *       questo file mette 17 funzioni;
 *     - fuori dalla mappa, in `.exit.text` oltre `_einittext`, ce n'e' una
 *       diciottesima: `flashlight_mt6370_exit`, 0xffffff80093aa294, 120 byte;
 *     - tolte le tre toccate restano 15 funzioni non toccate.  NON 12.
 *
 *   E NON erano «gia' identiche»: e' un'affermazione che il lotto stesso
 *   smentisce.  Con l'albero ALPS INTATTO e il compilatore della fabbrica, le
 *   gia' identiche fra le 17 del blocco erano 13:
 *     - `mt6370_disable` e `mt6370_enable` non esistevano nemmeno come simbolo
 *       (clang le incorporava, perche' il codice ALPS e' piu' piccolo);
 *     - `mt6370_operate` misurava 1148 byte contro i 944 di fabbrica;
 *     - `mt6370_work_disable_ch1` misurava 168 byte contro 68, ed e' diventata
 *       identica come EFFETTO dell'innesto in `mt6370_disable_ch1`, non
 *       perche' fosse gia' a posto.
 *   Uscita letterale (ALPS intatto + clang r353983c, contro l'oracolo):
 *     dimensione UGUALE           : 87
 *     dimensione DIVERSA          : 2
 *     MANCANTI                    : 2  ['mt6370_disable', 'mt6370_enable']
 *     mt6370_operate                                  944     1148     +204
 *     mt6370_work_disable_ch1                          68      168     +100
 *
 * PERCHE' SERVIVANO
 *   Compilando l'albero ALPS con il compilatore VERO della fabbrica
 *   (clang r353983c / LLVM 9.0.3, lo stesso della `linux_banner` dell'oracolo)
 *   87 funzioni su 89 del blocco flashlight risultavano gia' identiche
 *   istruzione per istruzione.  Le due che restavano erano `mt6370_operate` e
 *   `mt6370_work_disable_ch1`, e non erano rumore del compilatore: erano
 *   `mt6370_disable` e `mt6370_enable`, che di fabbrica hanno un simbolo
 *   proprio (472 e 208 byte) e da noi venivano INCORPORATE perche' il nostro
 *   codice era piu' piccolo.  Dopo i tre innesti sono 91 su 91 nel blocco
 *   della mappa e 95 su 95 contando le quattro `.exit` -- ma solo CON IL
 *   COMPILATORE DELLA FABBRICA, e il numero non si scrive senza dirlo.  Con il
 *   compilatore del progetto (clang r383902) sono 70 su 95 = 73,7%, IC95%
 *   Clopper-Pearson [63.6%; 82.2%], che contiene il 77,10% del ramo e quindi
 *   NON discrimina.  91 su 91 = 100%, IC95% [96.0%; 100.0%]: quello discrimina.
 *
 * COME SI RIVERIFICA (sul PC di build)
 *   # 1. compila con il compilatore della fabbrica (il percorso e' quello vero:
 *   #    lo script sta in kernel-stock/lotto-flash, non in /tmp)
 *   sh lotto-flash/b9.sh
 *   # 2. misura contro l'oracolo, per dimensione
 *   ./venv/bin/python3 lotto-flash/misura.py oracolo/stock.map <nostro.txt> \
 *       --inizio 0xffffff800875b5f8 --fine 0xffffff8008760ce4 \
 *       --extra flashlight_init=0xffffff8009373088 \
 *       --extra flashlight_mt6370_init=0xffffff800937315c \
 *       --extra flashlight_class_init=0xffffff8009373200
 *   # 3. e istruzione per istruzione (maschera solo i campi rilocati)
 *   ./venv/bin/python3 lotto-flash/identita2.py <dir del .o>
 *   # 4. i descrittori pr_debug di fabbrica di questo file (27, non 28)
 *   ./venv/bin/python3 ddebug.py oracolo/stock.elf \
 *       --file flashlights-mt6370 --tutte
 *   # 5. le citazioni di istruzione: 32 trovate, 32 confermate, 0 assenti
 *   ./venv/bin/python3 verificaistruzioni.py <questo file> oracolo/stock.elf \
 *       --intervallo 0xffffff800875b5f8:0xffffff8008760ce4
 *   # 6. le quattro `.exit`, che la mappa NON contiene: confini e dimensioni
 *   #    ricavati dai prologhi, non dalla mappa
 *   ./venv/bin/python3 lotto-flash/exit2.py <dir del .o>
 *
 * DIVERGENZE APERTE
 *   1. Il codice qui e' scritto INCONDIZIONATO.  Di fabbrica quasi certamente
 *      sta dentro `#ifdef CONFIG_WTK_MAIN_FLASHLIGHT_CH0`, che nell'albero ALPS
 *      non esiste (serve una voce `Kconfig`: e' il "delta di header" del
 *      rapporto, e NON e' stato fatto qui).  L'aritmetica delle righe del
 *      `.ddebug` e' compatibile con quella forma -- +15 e +9 righe sui due
 *      blocchi SOSTITUITI se il codice ALPS resta in un `#else`, +18 sul terzo
 *      che e' una pura AGGIUNTA -- ma dipende dalla formattazione, quindi e'
 *      una corroborazione, non una prova.  Il binario non registra il
 *      preprocessore.
 *   2. `mt6370_disable_ch1` usa `flashlight_dev_ch2` senza controllo NULL.
 *      E' un difetto della fabbrica, riprodotto (regola 7), non un errore qui.
 *   3. Con IL NOSTRO compilatore (clang r383902 / LLVM 11.0.1) le dimensioni
 *      NON tornano.  I numeri sono due coppie, di due artefatti diversi, e
 *      vanno tenuti separati:
 *        - albero ALPS INTATTO (l'esperimento di controllo): 64 su 89 in
 *          dimensione e 15 su 89 in codifica, contro 87 e 87 con il
 *          compilatore della fabbrica;
 *        - QUESTO albero, cioe' con i tre innesti: 66 su 91 in dimensione e
 *          16 su 91 in codifica, contro 91 e 91 con quello della fabbrica.
 *      Non e' un difetto di questo file -- e' la deriva fra due versioni di
 *      LLVM: le 25 funzioni che divergono con clang 11 tornano TUTTE identiche
 *      con clang 9, e questo prova la causa COLLETTIVAMENTE.  Il meccanismo
 *      funzione per funzione sta su un grado di prova piu' basso: quattro
 *      funzioni sono state lette affiancate, per le altre 21 l'evidenza e' il
 *      solo istogramma dei mnemonici.  Vedi il rapporto.
 *   4. `verificacitazioni.py` segnala NON_ANCORATA la citazione
 *      "1a8a1553 cinc"@0xffffff8008760150.  E' un FALSO POSITIVO dello
 *      strumento, non un difetto di questo file: `cinc` (come `cinv` e `cneg`)
 *      manca dall'insieme MNEMONICI di `confinecitazioni.py`, che ha invece le
 *      forme non-alias `csinc`/`csinv`/`csneg`; objdump stampa l'alias, quindi
 *      la citazione finisce nel dominio dei LETTERALI invece che in quello
 *      delle istruzioni.  `verificaistruzioni.py` la accetta e la conferma: su
 *      questo file lo strumento trova 32 citazioni di istruzione e le conferma
 *      tutte e 32, 0 assenti.  DUE delle 32 stanno in questo cappello -- la
 *      citazione qui sopra e la riga di objdump qui sotto, che lo strumento
 *      legge come citazione con operandi -- e sono contate anch'esse: 32 e' la
 *      misura sul FILE, non sul solo corpo.  Verifica a mano:
 *        $OD -d --start-address 0xffffff8008760150 \
 *            --stop-address 0xffffff8008760154 oracolo/stock.elf
 *        ffffff8008760150:  1a8a1553  cinc  w19, w10, eq
 *      `confinecitazioni.py` e' uno strumento CONDIVISO: la correzione e'
 *      descritta nel rapporto come delta, e non e' stata fatta qui.  Finche'
 *      non e' fatta, `verificacitazioni.py` su questo file esce con 1 anche
 *      dichiarando come eccezione tutti e 61 i letterali: la NON_ANCORATA
 *      resta, ed e' il falso positivo, non una citazione sbagliata.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": %s: " fmt, __func__

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/list.h>
#include <linux/delay.h>

#include "richtek/rt-flashlight.h"
#include "mtk_charger.h"

#include "flashlight-core.h"
#include "flashlight-dt.h"

/* device tree should be defined in flashlight-dt.h */
#ifndef MT6370_DTNAME
#define MT6370_DTNAME "mediatek,flashlights_mt6370"
#endif

#define MT6370_NAME "flashlights-mt6370"

/* define channel, level */
#define MT6370_CHANNEL_NUM 2
#define MT6370_CHANNEL_CH1 0
#define MT6370_CHANNEL_CH2 1
#define MT6370_CHANNEL_ALL 2

#define MT6370_NONE (-1)
#define MT6370_DISABLE 0
#define MT6370_ENABLE 1
#define MT6370_ENABLE_TORCH 1
#define MT6370_ENABLE_FLASH 2

#define MT6370_LEVEL_NUM 32
#define MT6370_LEVEL_TORCH 16
#define MT6370_LEVEL_FLASH MT6370_LEVEL_NUM
#define MT6370_WDT_TIMEOUT 1248 /* ms */
#define MT6370_HW_TIMEOUT 400 /* ms */

/* define mutex, work queue and timer */
static DEFINE_MUTEX(mt6370_mutex);
static struct work_struct mt6370_work_ch1;
static struct work_struct mt6370_work_ch2;
static struct hrtimer mt6370_timer_ch1;
static struct hrtimer mt6370_timer_ch2;
static unsigned int mt6370_timeout_ms[MT6370_CHANNEL_NUM];

/* define usage count */
static int use_count;
static int fd_use_count;

/* define RTK flashlight device */
static struct flashlight_device *flashlight_dev_ch1;
static struct flashlight_device *flashlight_dev_ch2;
#define RT_FLED_DEVICE_CH1  "mt-flash-led1"
#define RT_FLED_DEVICE_CH2  "mt-flash-led2"

/* define charger consumer */
static struct charger_consumer *flashlight_charger_consumer;
#define CHARGER_SUPPLY_NAME "charger_port1"

/* is decrease voltage */
static int is_decrease_voltage;

/* platform data */
struct mt6370_platform_data {
	int channel_num;
	struct flashlight_device_id *dev_id;
};


/******************************************************************************
 * mt6370 operations
 *****************************************************************************/
static const int mt6370_current[MT6370_LEVEL_NUM] = {
	  25,   50,  75, 100, 125, 150, 175,  200,  225,  250,
	 275,  300, 325, 350, 375, 400, 450,  500,  550,  600,
	 650,  700, 750, 800, 850, 900, 950, 1000, 1050, 1100,
	1150, 1200
};

static const unsigned char mt6370_torch_level[MT6370_LEVEL_TORCH] = {
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12,
	0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E
};

static const unsigned char mt6370_strobe_level[MT6370_LEVEL_FLASH] = {
	0x80, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10,
	0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x20, 0x24, 0x28, 0x2C,
	0x30, 0x34, 0x38, 0x3C, 0x40, 0x44, 0x48, 0x4C, 0x50, 0x54,
	0x58, 0x5C
};

static int mt6370_decouple_mode;
static int mt6370_en_ch1;
static int mt6370_en_ch2;
static int mt6370_level_ch1;
static int mt6370_level_ch2;

static int mt6370_is_charger_ready(void)
{
	if (flashlight_is_ready(flashlight_dev_ch1) &&
			flashlight_is_ready(flashlight_dev_ch2))
		return FLASHLIGHT_CHARGER_READY;
	else
		return FLASHLIGHT_CHARGER_NOT_READY;
}

static int mt6370_is_torch(int level)
{
	if (level >= MT6370_LEVEL_TORCH)
		return -1;

	return 0;
}

#if 0
static int mt6370_is_torch_by_timeout(int timeout)
{
	if (!timeout)
		return 0;

	if (timeout >= MT6370_WDT_TIMEOUT)
		return 0;

	return -1;
}
#endif

static int mt6370_verify_level(int level)
{
	if (level < 0)
		level = 0;
	else if (level >= MT6370_LEVEL_NUM)
		level = MT6370_LEVEL_NUM - 1;

	return level;
}

/* flashlight enable function */
static int mt6370_enable(void)
{
	int ret = 0;
	enum flashlight_mode mode = FLASHLIGHT_MODE_TORCH;

	if (!flashlight_dev_ch1 || !flashlight_dev_ch2) {
		pr_err("Failed to enable since no flashlight device.\n");
		return -1;
	}

	/* set flash mode if any channel is flash mode */
	if ((mt6370_en_ch1 == MT6370_ENABLE_FLASH)
			|| (mt6370_en_ch2 == MT6370_ENABLE_FLASH))
		mode = FLASHLIGHT_MODE_FLASH;

	/*
	 * DAL BINARIO -- `mt6370_enable` (0xffffff800876010c, 208 byte).
	 * ALPS ha qui, in piu': una `pr_debug("enable(%d,%d), mode:%d.\n", ...)`
	 * e un ramo "dual flash mode" con tre letture di `mt6370_decouple_mode`.
	 * Nella fabbrica NON ci sono, e la prova non e' solo la dimensione:
	 *  - `mt6370_decouple_mode` (0x9c98ad8) non e' MAI letto nei 208 byte;
	 *  - il `.ddebug` di fabbrica ha 27 descrittori per questo file e NESSUNO
	 *    e' in `mt6370_enable` (ddebug.py oracolo/stock.elf
	 *    --file flashlights-mt6370 --tutte).  ALPS ne ha 28.
	 * Il `mode` e il triplice test per canale:
	 *   "7100093f cmp"@0xffffff8008760148   cmp w9,#2   (en_ch2 == FLASH?)
	 *   "7a421904 ccmp"@0xffffff800876014c  ccmp w8,#2,#4,ne (en_ch1 == FLASH?)
	 *   "1a8a1553 cinc"@0xffffff8008760150  mode = 1 + (uno dei due e' FLASH)
	 *   "7100011f cmp"@0xffffff8008760154   cmp w8,#0     su en_ch1
	 *   "540000ed b.le"@0xffffff800876015c  > 0  -> set_mode(dev_ch1, mode)
	 *   "34ffff08 cbz"@0xffffff8008760180   == 0 -> set_mode(dev_ch1, OFF)
	 *                                       < 0  -> NIENTE (MT6370_NONE = -1)
	 *   "7100011f cmp"@0xffffff8008760188   lo stesso su en_ch2
	 *   "34000168 cbz"@0xffffff80087601a8
	 *   "2a1f03f3 mov"@0xffffff80087601d4   mode = 0 = FLASHLIGHT_MODE_OFF
	 * ALPS scrive `if (mt6370_en_ch1)`, che per un valore NEGATIVO chiamerebbe
	 * con `mode`: il `b.le` della fabbrica lo esclude.  Il confine e' `> 0`,
	 * non `!= 0`.
	 */
	/* enable channel 1 and channel 2 */
	if (mt6370_en_ch1 > MT6370_DISABLE)
		ret |= flashlight_set_mode(
			flashlight_dev_ch1, mode);
	else if (mt6370_en_ch1 == MT6370_DISABLE)
		ret |= flashlight_set_mode(
			flashlight_dev_ch1, FLASHLIGHT_MODE_OFF);
	if (mt6370_en_ch2 > MT6370_DISABLE)
		ret |= flashlight_set_mode(
			flashlight_dev_ch2, mode);
	else if (mt6370_en_ch2 == MT6370_DISABLE)
		ret |= flashlight_set_mode(
			flashlight_dev_ch2, FLASHLIGHT_MODE_OFF);

	if (ret < 0)
		pr_err("Failed to enable.\n");

	return ret;
}

/* flashlight disable function */
static int mt6370_disable_ch1(void)
{
	int ret = 0;

	pr_debug("disable_ch1.\n");

	if (!flashlight_dev_ch1) {
		pr_info("Failed to disable since no flashlight device.\n");
		return -1;
	}

	/*
	 * DAL BINARIO -- il ramo `channel == MT6370_CHANNEL_CH1` di
	 * `mt6370_disable` (0xffffff800875f144, 472 byte).  ALPS qui ha UNA riga
	 * sola, `ret |= flashlight_set_mode(flashlight_dev_ch1, OFF)`, senza
	 * guardie.  La fabbrica ne ha due, sulle due variabili di canale:
	 *   "b94ac908 ldr"@0xffffff800875f180   ldr w8,[x8,#2760] = mt6370_en_ch1
	 *   "34000788 cbz"@0xffffff800875f184   -> chiama solo se e' ZERO
	 *   "2a1f03f3 mov"@0xffffff800875f188   ret = 0 quando NON chiama
	 *   "2a1f03e1 mov"@0xffffff800875f274   w1 = 0 = FLASHLIGHT_MODE_OFF
	 *   "940004f7 bl"@0xffffff800875f278    bl flashlight_set_mode
	 *   "2a0003f3 mov"@0xffffff800875f27c   ret = w0   (il primo `|=` su 0)
	 *   "b94acd08 ldr"@0xffffff800875f284   ldr w8,[x8,#2764] = mt6370_en_ch2
	 *   "350000c8 cbnz"@0xffffff800875f288  -> salta se NON e' zero
	 *   "f9455d00 ldr"@0xffffff800875f290   ldr x0,[x8,#2744] = dev_ch2
	 *   "2a130013 orr"@0xffffff800875f29c   ret |= w0
	 * Le due variabili si riconoscono da `mt6370_enable`, che le legge in
	 * quest'ordine per decidere il `mode` (vedi la citazione la' sopra):
	 * 0x9c98ac8 comanda `flashlight_dev_ch1`, 0x9c98acc `flashlight_dev_ch2`.
	 *
	 * DIFETTO DELLA FABBRICA, RIPRODOTTO: il secondo ramo usa
	 * `flashlight_dev_ch2` SENZA controllarlo per NULL, mentre il primo ha
	 * il controllo su `flashlight_dev_ch1` qui sopra.  Nel binario e' cosi'
	 * (`ldr x0,[x8,#2744]` seguito subito dal `bl`, nessun `cbz`), e regola 7
	 * dice di riprodurlo, non di correggerlo.
	 */
	if (!mt6370_en_ch1)
		ret |= flashlight_set_mode(flashlight_dev_ch1,
					   FLASHLIGHT_MODE_OFF);
	if (!mt6370_en_ch2)
		ret |= flashlight_set_mode(flashlight_dev_ch2,
					   FLASHLIGHT_MODE_OFF);

	if (ret < 0)
		pr_info("Failed to disable.\n");

	return ret;
}

static int mt6370_disable_ch2(void)
{
	int ret = 0;

	pr_debug("disable_ch2.\n");

	if (!flashlight_dev_ch2) {
		pr_info("Failed to disable since no flashlight device.\n");
		return -1;
	}

	ret |= flashlight_set_mode(flashlight_dev_ch2, FLASHLIGHT_MODE_OFF);

	if (ret < 0)
		pr_info("Failed to disable.\n");

	return ret;
}

static int mt6370_disable_all(void)
{
	int ret = 0;

	pr_debug("disable_ch1.\n");

	if (!flashlight_dev_ch1) {
		pr_info("Failed to disable since no flashlight device.\n");
		return -1;
	}

	ret |= flashlight_set_mode(flashlight_dev_ch1,
				   FLASHLIGHT_MODE_DUAL_OFF);

	if (ret < 0)
		pr_info("Failed to disable.\n");

	return ret;
}

static int mt6370_disable(int channel)
{
	int ret = 0;

	if (channel == MT6370_CHANNEL_CH1)
		ret = mt6370_disable_ch1();
	else if (channel == MT6370_CHANNEL_CH2)
		ret = mt6370_disable_ch2();
	else if (channel == MT6370_CHANNEL_ALL)
		ret = mt6370_disable_all();
	else {
		pr_info("Error channel\n");
		return -1;
	}

	return ret;
}

/* set flashlight level */
static int mt6370_set_level_ch1(int level)
{
	level = mt6370_verify_level(level);
	mt6370_level_ch1 = level;

	if (!flashlight_dev_ch1) {
		pr_err("Failed to set ht level since no flashlight device.\n");
		return -1;
	}

	/* set brightness level */
	if (!mt6370_is_torch(level))
		flashlight_set_torch_brightness(
				flashlight_dev_ch1, mt6370_torch_level[level]);
	flashlight_set_strobe_brightness(
			flashlight_dev_ch1, mt6370_strobe_level[level]);

	return 0;
}

static int mt6370_set_level_ch2(int level)
{
	level = mt6370_verify_level(level);
	mt6370_level_ch2 = level;

	if (!flashlight_dev_ch2) {
		pr_err("Failed to set lt level since no flashlight device.\n");
		return -1;
	}

	/* set brightness level */
	if (!mt6370_is_torch(level))
		flashlight_set_torch_brightness(
				flashlight_dev_ch2, mt6370_torch_level[level]);
	flashlight_set_strobe_brightness(
			flashlight_dev_ch2, mt6370_strobe_level[level]);

	return 0;
}

static int mt6370_set_level(int channel, int level)
{
	if (channel == MT6370_CHANNEL_CH1)
		mt6370_set_level_ch1(level);
	else if (channel == MT6370_CHANNEL_CH2)
		mt6370_set_level_ch2(level);
	else {
		pr_err("Error channel\n");
		return -1;
	}

	return 0;
}

static int mt6370_set_scenario(int scenario)
{
	/* set decouple mode */
	mt6370_decouple_mode = scenario & FLASHLIGHT_SCENARIO_DECOUPLE_MASK;

	/* notify charger to increase or decrease voltage */
	if (!flashlight_charger_consumer) {
		pr_err("Failed with no charger consumer handler.\n");
		return -1;
	}

	mutex_lock(&mt6370_mutex);
	if (scenario & FLASHLIGHT_SCENARIO_CAMERA_MASK) {
		if (!is_decrease_voltage) {
			pr_info("Decrease voltage level.\n");
			charger_manager_enable_high_voltage_charging(
					flashlight_charger_consumer, false);
			is_decrease_voltage = 1;
		}
	} else {
		if (is_decrease_voltage) {
			pr_info("Increase voltage level.\n");
			charger_manager_enable_high_voltage_charging(
					flashlight_charger_consumer, true);
			is_decrease_voltage = 0;
		}
	}
	mutex_unlock(&mt6370_mutex);

	return 0;
}

/* flashlight init */
static int mt6370_init(void)
{
	/* clear flashlight state */
	mt6370_en_ch1 = MT6370_NONE;
	mt6370_en_ch2 = MT6370_NONE;

	/* clear decouple mode */
	mt6370_decouple_mode = FLASHLIGHT_SCENARIO_COUPLE;

	/* clear charger status */
	is_decrease_voltage = 0;

	return 0;
}

/* flashlight uninit */
static int mt6370_uninit(void)
{
	int ret;

	/* clear flashlight state */
	mt6370_en_ch1 = MT6370_NONE;
	mt6370_en_ch2 = MT6370_NONE;

	/* clear decouple mode */
	mt6370_decouple_mode = FLASHLIGHT_SCENARIO_COUPLE;

	/* clear charger status */
	is_decrease_voltage = 0;

	ret = mt6370_disable(MT6370_CHANNEL_ALL);

	return ret;
}


/******************************************************************************
 * Timer and work queue
 *****************************************************************************/
static void mt6370_work_disable_ch1(struct work_struct *data)
{
	pr_debug("ht work queue callback\n");
	mt6370_disable(MT6370_CHANNEL_CH1);
}

static void mt6370_work_disable_ch2(struct work_struct *data)
{
	pr_debug("lt work queue callback\n");
	mt6370_disable(MT6370_CHANNEL_CH2);
}

static enum hrtimer_restart mt6370_timer_func_ch1(struct hrtimer *timer)
{
	schedule_work(&mt6370_work_ch1);
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart mt6370_timer_func_ch2(struct hrtimer *timer)
{
	schedule_work(&mt6370_work_ch2);
	return HRTIMER_NORESTART;
}

static int mt6370_timer_start(int channel, ktime_t ktime)
{
	if (channel == MT6370_CHANNEL_CH1)
		hrtimer_start(&mt6370_timer_ch1, ktime, HRTIMER_MODE_REL);
	else if (channel == MT6370_CHANNEL_CH2)
		hrtimer_start(&mt6370_timer_ch2, ktime, HRTIMER_MODE_REL);
	else {
		pr_err("Error channel\n");
		return -1;
	}

	return 0;
}

static int mt6370_timer_cancel(int channel)
{
	if (channel == MT6370_CHANNEL_CH1)
		hrtimer_cancel(&mt6370_timer_ch1);
	else if (channel == MT6370_CHANNEL_CH2)
		hrtimer_cancel(&mt6370_timer_ch2);
	else {
		pr_err("Error channel\n");
		return -1;
	}

	return 0;
}

/******************************************************************************
 * Flashlight operation wrapper function
 *****************************************************************************/
static int mt6370_operate(int channel, int enable)
{
	ktime_t ktime;
	unsigned int s;
	unsigned int ns;

	/* setup enable/disable */
	if (channel == MT6370_CHANNEL_CH1) {
		mt6370_en_ch1 = enable;
		if (mt6370_en_ch1)
			if (mt6370_is_torch(mt6370_level_ch1))
				mt6370_en_ch1 = MT6370_ENABLE_FLASH;
	} else if (channel == MT6370_CHANNEL_CH2) {
		mt6370_en_ch2 = enable;
		if (mt6370_en_ch2)
			if (mt6370_is_torch(mt6370_level_ch2))
				mt6370_en_ch2 = MT6370_ENABLE_FLASH;
	} else {
		pr_err("Error channel\n");
		return -1;
	}

	/* decouple mode */
	if (mt6370_decouple_mode) {
		if (channel == MT6370_CHANNEL_CH1) {
			mt6370_en_ch2 = MT6370_DISABLE;
			mt6370_timeout_ms[MT6370_CHANNEL_CH2] = 0;
		} else if (channel == MT6370_CHANNEL_CH2) {
			mt6370_en_ch1 = MT6370_DISABLE;
			mt6370_timeout_ms[MT6370_CHANNEL_CH1] = 0;
		}
	}

	pr_debug("en_ch(%d,%d), decouple:%d\n",
		mt6370_en_ch1, mt6370_en_ch2, mt6370_decouple_mode);

	/* operate flashlight and setup timer */
	if ((mt6370_en_ch1 != MT6370_NONE) && (mt6370_en_ch2 != MT6370_NONE)) {
		if ((mt6370_en_ch1 == MT6370_DISABLE) &&
				(mt6370_en_ch2 == MT6370_DISABLE)) {
			if (mt6370_decouple_mode) {
				if (channel == MT6370_CHANNEL_CH1) {
					mt6370_disable(MT6370_CHANNEL_CH1);
					mt6370_timer_cancel(MT6370_CHANNEL_CH1);
				} else if (channel == MT6370_CHANNEL_CH2) {
					mt6370_disable(MT6370_CHANNEL_CH2);
					mt6370_timer_cancel(MT6370_CHANNEL_CH2);
				}
			} else {
				mt6370_disable(MT6370_CHANNEL_ALL);
				mt6370_timer_cancel(MT6370_CHANNEL_CH1);
				mt6370_timer_cancel(MT6370_CHANNEL_CH2);
			}
		} else {
			if (mt6370_timeout_ms[MT6370_CHANNEL_CH1] &&
				mt6370_en_ch1 != MT6370_DISABLE) {
				s = mt6370_timeout_ms[MT6370_CHANNEL_CH1] /
					1000;
				ns = mt6370_timeout_ms[MT6370_CHANNEL_CH1] %
					1000 * 1000000;
				ktime = ktime_set(s, ns);
				mt6370_timer_start(MT6370_CHANNEL_CH1, ktime);
			}
			if (mt6370_timeout_ms[MT6370_CHANNEL_CH2] &&
				mt6370_en_ch2 != MT6370_DISABLE) {
				s = mt6370_timeout_ms[MT6370_CHANNEL_CH2] /
					1000;
				ns = mt6370_timeout_ms[MT6370_CHANNEL_CH2] %
					1000 * 1000000;
				ktime = ktime_set(s, ns);
				mt6370_timer_start(MT6370_CHANNEL_CH2, ktime);
			}
			mt6370_enable();
		}

		/* clear flashlight state */
		mt6370_en_ch1 = MT6370_NONE;
		mt6370_en_ch2 = MT6370_NONE;
	/*
	 * DAL BINARIO -- `mt6370_operate` (0xffffff800875fd5c, 944 byte).
	 * ALPS non fa NULLA quando uno dei due canali e' MT6370_NONE: il blocco
	 * qui sotto e' cio' che la fabbrica esegue quando il canale 2 non ha mai
	 * ricevuto un comando -- cioe' il caso di un flash principale SOLO sul
	 * canale 0, che e' quello che nomina `CONFIG_WTK_MAIN_FLASHLIGHT_CH0`.
	 *   "3100053f cmn"@0xffffff800875fec4   cmn w9,#1  -> en_ch1 == -1 ?
	 *   "54000d20 b.eq"@0xffffff800875fec8  se si', esce senza fare nulla
	 *   "34000589 cbz"@0xffffff800875fecc   en_ch1 == 0 -> il ramo "spegni"
	 *   "b94a9908 ldr"@0xffffff800875fed4   ldr w8,[x8,#2712]
	 *                                       0x9c98a98 = mt6370_timeout_ms[0]
	 *   "34000248 cbz"@0xffffff800875fed8   timeout 0 -> niente timer
	 *   "97e7cdcf bl"@0xffffff800875ff1c    bl hrtimer_start_range_ns
	 *   "9400007b bl"@0xffffff800875ff20    bl mt6370_enable
	 * il ramo "spegni" e' `mt6370_disable(MT6370_CHANNEL_ALL)` incorporato
	 * (il compilatore ne piega la costante) seguito da un solo hrtimer_cancel:
	 *   "321f07e1 orr"@0xffffff800875ff94   w1 = 6 = FLASHLIGHT_MODE_DUAL_OFF
	 *   "97e7cf4d bl"@0xffffff800876000c    bl hrtimer_cancel (solo il CH1)
	 * e in uscita si azzera un solo canale, non due:
	 *   "12800008 mov"@0xffffff8008760010   w8 = -1 = MT6370_NONE
	 *   "b90aca88 str"@0xffffff8008760014   str w8,[x20,#2760] = en_ch1
	 */
	} else if (mt6370_en_ch1 != MT6370_NONE) {
		if (mt6370_en_ch1 == MT6370_DISABLE) {
			mt6370_disable(MT6370_CHANNEL_ALL);
			mt6370_timer_cancel(MT6370_CHANNEL_CH1);
		} else {
			if (mt6370_timeout_ms[MT6370_CHANNEL_CH1]) {
				s = mt6370_timeout_ms[MT6370_CHANNEL_CH1] /
					1000;
				ns = mt6370_timeout_ms[MT6370_CHANNEL_CH1] %
					1000 * 1000000;
				ktime = ktime_set(s, ns);
				mt6370_timer_start(MT6370_CHANNEL_CH1, ktime);
			}
			mt6370_enable();
		}

		/* clear flashlight state */
		mt6370_en_ch1 = MT6370_NONE;
	}

	return 0;
}

/******************************************************************************
 * Flashlight operations
 *****************************************************************************/
static int mt6370_ioctl(unsigned int cmd, unsigned long arg)
{
	struct flashlight_dev_arg *fl_arg;
	int channel;

	fl_arg = (struct flashlight_dev_arg *)arg;
	channel = fl_arg->channel;

	/* verify channel */
	if (channel < 0 || channel >= MT6370_CHANNEL_NUM) {
		pr_err("Failed with error channel\n");
		return -EINVAL;
	}

	switch (cmd) {
	case FLASH_IOC_SET_TIME_OUT_TIME_MS:
		pr_debug("FLASH_IOC_SET_TIME_OUT_TIME_MS(%d): %d\n",
				channel, (int)fl_arg->arg);
		mt6370_timeout_ms[channel] = fl_arg->arg;
		break;

	case FLASH_IOC_SET_DUTY:
		pr_debug("FLASH_IOC_SET_DUTY(%d): %d\n",
				channel, (int)fl_arg->arg);
		mt6370_set_level(channel, fl_arg->arg);
		break;

	case FLASH_IOC_SET_SCENARIO:
		pr_debug("FLASH_IOC_SET_SCENARIO(%d): %d\n",
				channel, (int)fl_arg->arg);
		mt6370_set_scenario(fl_arg->arg);
		break;

	case FLASH_IOC_SET_ONOFF:
		pr_debug("FLASH_IOC_SET_ONOFF(%d): %d\n",
				channel, (int)fl_arg->arg);
		mt6370_operate(channel, fl_arg->arg);
		break;

	case FLASH_IOC_IS_CHARGER_READY:
		pr_debug("FLASH_IOC_IS_CHARGER_READY(%d)\n", channel);
		fl_arg->arg = mt6370_is_charger_ready();
		break;

	case FLASH_IOC_GET_DUTY_NUMBER:
		pr_debug("FLASH_IOC_GET_DUTY_NUMBER(%d)\n", channel);
		fl_arg->arg = MT6370_LEVEL_NUM;
		break;

	case FLASH_IOC_GET_MAX_TORCH_DUTY:
		pr_debug("FLASH_IOC_GET_MAX_TORCH_DUTY(%d)\n", channel);
		fl_arg->arg = MT6370_LEVEL_TORCH - 1;
		break;

	case FLASH_IOC_GET_DUTY_CURRENT:
		fl_arg->arg = mt6370_verify_level(fl_arg->arg);
		pr_debug("FLASH_IOC_GET_DUTY_CURRENT(%d): %d\n",
				channel, (int)fl_arg->arg);
		fl_arg->arg = mt6370_current[fl_arg->arg];
		break;

	case FLASH_IOC_GET_HW_TIMEOUT:
		pr_debug("FLASH_IOC_GET_HW_TIMEOUT(%d)\n", channel);
		fl_arg->arg = MT6370_HW_TIMEOUT;
		break;

	default:
		pr_info("No such command and arg(%d): (%d, %d)\n",
				channel, _IOC_NR(cmd), (int)fl_arg->arg);
		return -ENOTTY;
	}

	return 0;
}

static int mt6370_open(void)
{
	/* Move to set driver for saving power */
	mutex_lock(&mt6370_mutex);
	fd_use_count++;
	pr_debug("open driver: %d\n", fd_use_count);
	mutex_unlock(&mt6370_mutex);
	return 0;
}

static int mt6370_release(void)
{
	/* Move to set driver for saving power */
	mutex_lock(&mt6370_mutex);
	fd_use_count--;
	pr_debug("close driver: %d\n", fd_use_count);
	/* If camera NE, we need to enable pe by ourselves*/
	if (fd_use_count == 0 && is_decrease_voltage) {
#ifdef CONFIG_MTK_CHARGER
		pr_info("Increase voltage level.\n");
		charger_manager_enable_high_voltage_charging(
				flashlight_charger_consumer, true);
#endif
		is_decrease_voltage = 0;
	}
	mutex_unlock(&mt6370_mutex);
	return 0;
}

static int mt6370_set_driver(int set)
{
	int ret = 0;

	/* set chip and usage count */
	mutex_lock(&mt6370_mutex);
	if (set) {
		if (!use_count)
			ret = mt6370_init();
		use_count++;
		pr_debug("Set driver: %d\n", use_count);
	} else {
		use_count--;
		if (!use_count)
			ret = mt6370_uninit();
		if (use_count < 0)
			use_count = 0;
		pr_debug("Unset driver: %d\n", use_count);
	}
	mutex_unlock(&mt6370_mutex);

	return ret;
}

static ssize_t mt6370_strobe_store(struct flashlight_arg arg)
{
	mt6370_set_driver(1);
	if (arg.decouple)
		mt6370_set_scenario(
			FLASHLIGHT_SCENARIO_CAMERA |
			FLASHLIGHT_SCENARIO_DECOUPLE);
	else
		mt6370_set_scenario(
			FLASHLIGHT_SCENARIO_CAMERA |
			FLASHLIGHT_SCENARIO_COUPLE);
	mt6370_set_level(arg.channel, arg.level);
	mt6370_timeout_ms[arg.channel] = 0;

	if (arg.level < 0)
		mt6370_operate(arg.channel, MT6370_DISABLE);
	else
		mt6370_operate(arg.channel, MT6370_ENABLE);

	msleep(arg.dur);
	if (arg.decouple)
		mt6370_set_scenario(
			FLASHLIGHT_SCENARIO_FLASHLIGHT |
			FLASHLIGHT_SCENARIO_DECOUPLE);
	else
		mt6370_set_scenario(
			FLASHLIGHT_SCENARIO_FLASHLIGHT |
			FLASHLIGHT_SCENARIO_COUPLE);
	mt6370_operate(arg.channel, MT6370_DISABLE);
	mt6370_set_driver(0);

	return 0;
}

static struct flashlight_operations mt6370_ops = {
	mt6370_open,
	mt6370_release,
	mt6370_ioctl,
	mt6370_strobe_store,
	mt6370_set_driver
};


/******************************************************************************
 * Platform device and driver
 *****************************************************************************/
static int mt6370_parse_dt(struct device *dev,
		struct mt6370_platform_data *pdata)
{
	struct device_node *np, *cnp;
	u32 decouple = 0;
	int i = 0;

	if (!dev || !dev->of_node || !pdata)
		return -ENODEV;

	np = dev->of_node;

	pdata->channel_num = of_get_child_count(np);
	if (!pdata->channel_num) {
		pr_info("Parse no dt, node.\n");
		return 0;
	}
	pr_info("Channel number(%d).\n", pdata->channel_num);

	if (of_property_read_u32(np, "decouple", &decouple))
		pr_info("Parse no dt, decouple.\n");

	pdata->dev_id = devm_kzalloc(dev,
			pdata->channel_num *
			sizeof(struct flashlight_device_id),
			GFP_KERNEL);
	if (!pdata->dev_id)
		return -ENOMEM;

	for_each_child_of_node(np, cnp) {
		if (of_property_read_u32(cnp, "type", &pdata->dev_id[i].type))
			goto err_node_put;
		if (of_property_read_u32(cnp, "ct", &pdata->dev_id[i].ct))
			goto err_node_put;
		if (of_property_read_u32(cnp, "part", &pdata->dev_id[i].part))
			goto err_node_put;
		snprintf(pdata->dev_id[i].name, FLASHLIGHT_NAME_SIZE,
				MT6370_NAME);
		pdata->dev_id[i].channel = i;
		pdata->dev_id[i].decouple = decouple;

		pr_info("Parse dt (type,ct,part,name,channel,decouple)=(%d,%d,%d,%s,%d,%d).\n",
				pdata->dev_id[i].type, pdata->dev_id[i].ct,
				pdata->dev_id[i].part, pdata->dev_id[i].name,
				pdata->dev_id[i].channel,
				pdata->dev_id[i].decouple);
		i++;
	}

	return 0;

err_node_put:
	of_node_put(cnp);
	return -EINVAL;
}

static int mt6370_probe(struct platform_device *pdev)
{
	struct mt6370_platform_data *pdata = dev_get_platdata(&pdev->dev);
	int ret;
	int i;

	pr_debug("Probe start.\n");

	/* parse dt */
	if (!pdata) {
		pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;
		pdev->dev.platform_data = pdata;
		ret = mt6370_parse_dt(&pdev->dev, pdata);
		if (ret)
			return ret;
	}

	/* init work queue */
	INIT_WORK(&mt6370_work_ch1, mt6370_work_disable_ch1);
	INIT_WORK(&mt6370_work_ch2, mt6370_work_disable_ch2);

	/* init timer */
	hrtimer_init(&mt6370_timer_ch1, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	mt6370_timer_ch1.function = mt6370_timer_func_ch1;
	hrtimer_init(&mt6370_timer_ch2, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	mt6370_timer_ch2.function = mt6370_timer_func_ch2;
	mt6370_timeout_ms[MT6370_CHANNEL_CH1] = 600;
	mt6370_timeout_ms[MT6370_CHANNEL_CH2] = 600;

	/* clear attributes */
	use_count = 0;
	fd_use_count = 0;
	is_decrease_voltage = 0;

	/* get RTK flashlight handler */
	flashlight_dev_ch1 = find_flashlight_by_name(RT_FLED_DEVICE_CH1);
	if (!flashlight_dev_ch1) {
		pr_err("Failed to get ht flashlight device.\n");
		return -EFAULT;
	}
	flashlight_dev_ch2 = find_flashlight_by_name(RT_FLED_DEVICE_CH2);
	if (!flashlight_dev_ch2) {
		pr_err("Failed to get lt flashlight device.\n");
		return -EFAULT;
	}

	/* setup strobe mode timeout */
	if (flashlight_set_strobe_timeout(flashlight_dev_ch1,
				MT6370_HW_TIMEOUT, MT6370_HW_TIMEOUT + 200) < 0)
		pr_err("Failed to set strobe timeout.\n");

	/* get charger consumer manager */
	flashlight_charger_consumer = charger_manager_get_by_name(
			&flashlight_dev_ch1->dev, CHARGER_SUPPLY_NAME);
	if (!flashlight_charger_consumer) {
		pr_err("Failed to get charger manager.\n");
		return -EFAULT;
	}

	/* register flashlight device */
	if (pdata->channel_num) {
		for (i = 0; i < pdata->channel_num; i++)
			if (flashlight_dev_register_by_device_id(
						&pdata->dev_id[i],
						&mt6370_ops))
				return -EFAULT;
	} else {
		if (flashlight_dev_register(MT6370_NAME, &mt6370_ops))
			return -EFAULT;
	}

	pr_debug("Probe done.\n");

	return 0;
}

static int mt6370_remove(struct platform_device *pdev)
{
	struct mt6370_platform_data *pdata = dev_get_platdata(&pdev->dev);
	int i;

	pr_debug("Remove start.\n");

	pdev->dev.platform_data = NULL;

	/* unregister flashlight device */
	if (pdata && pdata->channel_num)
		for (i = 0; i < pdata->channel_num; i++)
			flashlight_dev_unregister_by_device_id(
					&pdata->dev_id[i]);
	else
		flashlight_dev_unregister(MT6370_NAME);

	/* flush work queue */
	flush_work(&mt6370_work_ch1);
	flush_work(&mt6370_work_ch2);

	/* clear RTK flashlight device */
	flashlight_dev_ch1 = NULL;
	flashlight_dev_ch2 = NULL;

	pr_debug("Remove done.\n");

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id mt6370_of_match[] = {
	{.compatible = MT6370_DTNAME},
	{},
};
MODULE_DEVICE_TABLE(of, mt6370_of_match);
#else
static struct platform_device mt6370_platform_device[] = {
	{
		.name = MT6370_NAME,
		.id = 0,
		.dev = {}
	},
	{}
};
MODULE_DEVICE_TABLE(platform, mt6370_platform_device);
#endif

static struct platform_driver mt6370_platform_driver = {
	.probe = mt6370_probe,
	.remove = mt6370_remove,
	.driver = {
		.name = MT6370_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = mt6370_of_match,
#endif
	},
};

static int __init flashlight_mt6370_init(void)
{
	int ret;

	pr_debug("Init start.\n");

#ifndef CONFIG_OF
	ret = platform_device_register(&mt6370_platform_device);
	if (ret) {
		pr_err("Failed to register platform device\n");
		return ret;
	}
#endif

	ret = platform_driver_register(&mt6370_platform_driver);
	if (ret) {
		pr_err("Failed to register platform driver\n");
		return ret;
	}

	pr_debug("Init done.\n");

	return 0;
}

static void __exit flashlight_mt6370_exit(void)
{
	pr_debug("Exit start.\n");

	platform_driver_unregister(&mt6370_platform_driver);

	pr_debug("Exit done.\n");
}

/* replace module_init() since conflict in kernel init process */
late_initcall(flashlight_mt6370_init);
module_exit(flashlight_mt6370_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Simon Wang <Simon-TCH.Wang@mediatek.com>");
MODULE_DESCRIPTION("MTK Flashlight MT6370 Driver");

