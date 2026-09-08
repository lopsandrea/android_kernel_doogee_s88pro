// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_ILITEK_E977 -- il GRUPPO C, la piattaforma MTK. Doogee S88 Pro.
 *
 * Tredici funzioni in `.text` (0xffffff8008a55d0c..0xffffff8008a5692c, 3104
 * byte) piu' `ilitek_plat_dev_init` in `.init.text` (0xffffff80093833a8, 112
 * byte). Il confine e' quello di
 * `docs/bringup/rapporti/rapporto-ilitek-confine-e-unita.md` §2.3.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA. Nessun sorgente
 * pubblico e' stato letto. Ogni riga derivata porta accanto la riga di
 * disassemblato che la giustifica, nella forma
 * "<codifica> <mnemonico>"@0xINDIRIZZO.
 *
 * Riverifica:
 *
 *   ./venv/bin/python3 verificacitazioni.py ilitek_plat.c oracolo/stock.elf \
 *       --eccezione ""
 *   ./venv/bin/python3 verificaistruzioni.py ilitek_plat.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a55d0c:0xffffff8008a5692c \
 *       --intervallo 0xffffff8009383388:0xffffff8009383418
 *
 * Il secondo intervallo e' la `.init.text`, dove sta `ilitek_plat_dev_init`:
 * senza, le sue citazioni escono ASSENTI ed e' un falso allarme.
 *
 * ===========================================================================
 * LE TRE `tpd_*` SONO OMONIME, E LA CHIAVE E' L'INDIRIZZO
 * ===========================================================================
 * `tpd_local_init`, `tpd_suspend` e `tpd_resume` hanno **tre definizioni
 * ciascuna** nella mappa dell'oracolo: il config di fabbrica accende tutti e
 * tre i driver touch (ILITEK, GT917S, FT8719) e ognuno le definisce.
 *
 *   ffffff8008a56590 t tpd_local_init   <- ILITEK (dentro il confine)
 *   ffffff8008a775bc t tpd_local_init
 *   ffffff8008a7ef40 t tpd_local_init
 *
 * Quelle di questo file sono le PRIME, le sole che cadono nell'intervallo del
 * gruppo C. `disassembla.py` si rifiuta di sceglierne una per nome, e ha
 * ragione: vanno lette con `--indirizzo`.
 *
 * ===========================================================================
 * LA `struct ilitek_hwif_info` E' UN GLOBALE, E SI LEGGE
 * ===========================================================================
 * `tpd_local_init` passa a `ilitek_tddi_dev_init` l'indirizzo
 * 0xffffff8009987370 ("b0007980 adrp"@0xffffff8008a565b0 piu'
 * "910dc000 add"@0xffffff8008a565b4): la `hwif` non e' costruita a runtime,
 * e' un oggetto statico in `.data`.
 *
 * I suoi puntatori valgono ZERO nell'immagine -- il kernel e'
 * `CONFIG_RELOCATABLE=y` -- e il valore vero sta nell'addend della
 * rilocazione. `tools/relocazioni.py` li legge:
 *
 *   0xffffff8009987370: non relocato            -> type = 0x18 (I2C)
 *   0xffffff8009987378: addend 0xffffff800923f97f -> "ILITEK_TDDI"
 *   0xffffff8009987380: non relocato            -> owner = NULL
 *   0xffffff8009987388: addend 0xffffff8008f7ea98 -> of_match_table
 *   0xffffff8009987390: addend 0xffffff8008a566c4 -> ilitek_plat_probe
 *   0xffffff8009987398: addend 0xffffff8008a568fc -> ilitek_plat_remove
 *   0xffffff80099873a0: non relocato            -> driver = NULL (a runtime)
 *
 * **Questo CHIUDE la divergenza B5 di `ilitek_bus.c`**: il campo a +32,
 * che il gruppo B poteva solo tipare dal sito di chiamata, e'
 * `ilitek_plat_probe`. E riempie il buco a +40, che era
 * `ilitek_plat_remove`.
 *
 * L'`of_match_table` a 0xffffff8008f7ea98 si legge dai byte:
 * `sizeof(struct of_device_id)` = 200 con `compatible` a +64, e li' c'e'
 * "mediatek,cap_touch"@0xffffff8008f7ead8. Il resto della voce e' zero, e la
 * seconda voce e' tutta zero: e' il terminatore.
 *
 * ===========================================================================
 * SETTE CAMPI CHE IL BINARIO NOMINA, E CHE `ilitek.h` CHIAMA ANCORA c<offset>
 * ===========================================================================
 * `ilitek.h` dice che di 134 offset il binario ne nomina CINQUE, tutti da
 * macro che stringano il proprio argomento. **Ne nomina altri sette**, e la
 * fonte e' un formato di `printk` in `ilitek_plat_isr_top_half`:
 *
 *   "\x016ILITEK: (%s, %d): report: %d, rst: %d, fw: %d, switch: %d, mp: %d,
 *    sleep: %d, esd: %d\n"@0xffffff800923f8b5
 *
 * Gli argomenti sono caricati in ordine e l'ordine e' l'abbinamento:
 *
 *   | nome nel formato | registro | campo | riga di disassemblato |
 *   |---|---|---|---|
 *   | report | w3    | +531 | "39484d03 ldrb"@0xffffff8008a5646c |
 *   | rst    | w4    | +708 | "b942c504 ldr"@0xffffff8008a56470  |
 *   | fw     | w5    | +716 | "b942cd05 ldr"@0xffffff8008a56474  |
 *   | switch | w6    | +728 | "b942d906 ldr"@0xffffff8008a56478  |
 *   | mp     | w7    | +720 | "b942d107 ldr"@0xffffff8008a5647c  |
 *   | sleep  | [sp]  | +724 | "b942d509 ldr"@0xffffff8008a56480  |
 *   | esd    | [sp+8]| +736 | "b942e108 ldr"@0xffffff8008a56484  |
 *
 * Il `rst` a +708 e' una CONFERMA INDIPENDENTE: `ilitek.h` gia' scriveva
 * «messo a 1 all'ingresso di `ilitek_tddi_reset_ctrl` e azzerato in coda»,
 * letto dal grafo dei salti. Il formato lo chiama `rst`.
 *
 * **QUESTO LOTTO NON RINOMINA I CAMPI**, e la scelta va dichiarata:
 * rinominarli tocca `ilitek.h`, che ha 118 asserzioni verificate, e
 * `ilitek_main.c`, che li usa. Farlo qui vorrebbe dire cambiare due file per
 * una ragione che non riguarda questo gruppo. Va fatto in un lotto suo, che
 * aggiorni tutti gli usi insieme.
 *
 * ===========================================================================
 * LE DIVERGENZE APERTE DI QUESTO FILE
 * ===========================================================================
 * C1. I `__LINE__`, come in `ilitek_main.c` (D1) e `ilitek_bus.c` (B1).
 *     Annotati sopra ogni funzione, non riprodotti: imbottire di righe vuote
 *     un file che verra' ancora toccato sposta tutto al primo cambio.
 * C2. `ilitek_plat_gpio_register` e' INCORPORATA in `ilitek_plat_probe`, e
 *     lo prova il suo `__func__` ("ilitek_plat_gpio_register"@0xffffff800923fade,
 *     righe 176..206, contro le righe 365/372/375 di `probe`). E' riprodotta
 *     come funzione separata, come `core_i2c_write` nel gruppo B.
 * C3. I globali di modulo fuori dalla struttura. Il binario non ne nomina
 *     nessuno, e i nomi qui sono una comodita' di lettura: il commento di
 *     ciascuno dice l'indirizzo, la larghezza, chi legge e chi scrive.
 *     DUE DI ESSI PERO' NON SONO ARBITRARI NELLA FORMA:
 *       - `ilitek_probe_riuscito` NON puo' essere `static`, e lo dimostra la
 *         misura di dimensione (vedi il suo commento);
 *       - lo store a 0xffffff800a0fc938 e' a `tpd_load_status`, il globale
 *         che ALPS dichiara in `tpd.h`, e anche qui e' la misura a reggere
 *         la lettura.
 *     Il flag di debug e' `ilitek_dbg_en`, che `ilitek.h` gia' dichiarava.
 * C5. `ilitek_plat_tp_reset` misura 164 byte contro 168, e la differenza e'
 *     UNA istruzione: la forma del ciclo di `mdelay`. La fabbrica conta
 *     ALL'INSU' da -N ("cb0803f3 neg"@0xffffff8008a55d90,
 *     "b1000673 adds"@0xffffff8008a55da0, "54ffff83 b.cc"@0xffffff8008a55da4);
 *     il nostro clang conta all'ingiu' con `sub`/`cbnz`. E' lo stesso
 *     `while (n--) udelay(1000)` che la macro espande, reso in due modi
 *     equivalenti. Nessun residuo semantico.
 * C6. `ilitek_plat_probe` misura 560 byte contro 568, e la differenza sono
 *     DUE istruzioni: la fabbrica ha due `b` in piu' e un `add` in piu'
 *     perche' NON fonde una coda di `printk` che noi fondiamo. E' la stessa
 *     classe di divergenza gia' vista su `ilitek_i2c_probe` nel gruppo B.
 *     Nessun residuo semantico.
 * C4. Il secondo blocco di `__set_bit` su `keybit` e' RIDONDANTE con le
 *     `input_set_capability` che lo precedono -- quella funzione fa gia'
 *     `__set_bit(code, dev->keybit)`. Di fabbrica ci sono entrambi, e
 *     entrambi si riproducono: il compilatore non puo' fondere una chiamata
 *     fuori linea con un `orr` in linea, quindi le due forme sono
 *     distinguibili nel binario e la ridondanza e' del sorgente, non del
 *     codegen.
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

#include "tpd.h"
#include "ilitek.h"

/*
 * ===========================================================================
 * I QUATTRO GLOBALI DI MODULO -- vedi C3
 * ===========================================================================
 */

/*
 * 0xffffff800a0fc958, un byte. La guardia di `ILI_DBG`, gia' dichiarata in
 * `ilitek.h`. Qui e' DEFINITA, perche' e' questo il gruppo che la usa di
 * piu': "39656108 ldrb"@0xffffff8008a56134 (irq_disable),
 * "39656108 ldrb"@0xffffff8008a561dc (irq_enable),
 * "39656129 ldrb"@0xffffff8008a563dc e "39656289 ldrb"@0xffffff8008a56464
 * (isr_top_half), "39656108 ldrb"@0xffffff8008a56560 (isr_bottom_half).
 * Il VALORE INIZIALE non e' misurato: sta in `.bss` o in `.data` a zero, e
 * l'oracolo non distingue.
 */
u8 ilitek_dbg_en;

/*
 * 0xffffff800a0fca48, un byte. Vale 1 dopo che l'IRQ e' stato mappato una
 * volta, e serve a non rifare `of_find_matching_node_and_match` a ogni
 * registrazione: letto da "39692289 ldrb"@0xffffff8008a56284 e scritto da
 * "39292289 strb"@0xffffff8008a562e4, entrambi in `ilitek_plat_irq_register`.
 * Nessuno lo nomina.
 */
static bool ilitek_irq_mappato;

/*
 * 0xffffff800a0fc954, 32 bit. Messo a 1 in coda a `ilitek_plat_probe`
 * ("b9095509 str"@0xffffff8008a568ac) e letto da `tpd_local_init`
 * ("b9495508 ldr"@0xffffff8008a565c4), che fallisce se e' zero. E' il
 * «probe e' andato a buon fine». Nessuno lo nomina.
 *
 * NON E' `static`, E LA MISURA LO DIMOSTRA. Con `static` clang vede tutti
 * gli usi, sa che vale 0 o 1, e restringe: la lettura diventa
 * "ldrb"+"tbz #0" invece di "b9495508 ldr"+"cbz", e la scrittura diventa
 * una `strb` invece di "b9095509 str". Con quella forma
 * `ilitek_plat_probe` misurava 560 byte contro 568 e `tpd_local_init` 180
 * contro 188; togliendo `static` tornano esatte entrambe. E' la stessa
 * classe di prova del `sxtw` del gruppo B: una differenza di
 * DICHIARAZIONE che il binario denuncia.
 */
int ilitek_probe_riuscito;

/*
 * L'`of_match_table` della `hwif`, letta dai byte a 0xffffff8008f7ea98:
 * `compatible` = "mediatek,cap_touch"@0xffffff8008f7ead8 (offset +64 dentro
 * la voce, con `sizeof(struct of_device_id)` = 200), tutto il resto zero, e
 * la seconda voce tutta zero.
 */
static const struct of_device_id ilitek_of_match[] = {
	{ .compatible = "mediatek,cap_touch" },
	{ },
};

/*
 * LA SECONDA TABELLA, E PERCHE' NON PUO' ESSERE LA PRIMA.
 *
 * Il binario ha DUE tabelle di corrispondenza, non una, e i due indirizzi si
 * leggono a poche righe di distanza in questo stesso file:
 *
 *   hwif.of_match_table          addend 0xffffff8008f7ea98  "mediatek,cap_touch"
 *   ilitek_plat_irq_register     add    0xffffff8008f7e080  "mediatek,touch"
 *
 * Servono a due cose diverse, e il device tree ha davvero due nodi:
 *
 *   /i2c@11007000/cap_touch@5d   compatible "mediatek,cap_touch"  -- il
 *       dispositivo sul bus, che fa scattare il bind i2c e NON ha `interrupts`
 *   /touch                       compatible "mediatek,touch"      -- il nodo
 *       del pannello, che ha `interrupts` e `interrupt-parent`
 *
 * Usare la prima tabella anche qui trova il nodo i2c, che non ha interrupt:
 * `irq_of_parse_and_map` restituisce 0 e `devm_request_threaded_irq` fallisce
 * con -EINVAL. E' quel che succedeva, e il telefono si avviava con lo schermo
 * che non rispondeva al tocco:
 *
 *   ilitek_plat_irq_register, 344: Failed to register irq handler,
 *                                  irq = 0, ret = -22
 *
 * Tutto il resto del driver funzionava: il chip rispondeva (PID 78071001),
 * la flash era letta (MID c8), il firmware riconosciuto (8.0.6.0) e le misure
 * del pannello giuste (1080x2340). Mancava solo l'interrupt, e senza quello
 * un touchscreen e' muto.
 */
static const struct of_device_id ilitek_of_match_irq[] = {
	{ .compatible = "mediatek,touch" },
	{ },
};

/*
 * QUALI SONO `static` LO DICE LA MAPPA, non una scelta: sei delle quattordici
 * sono `T` (`tp_reset`, `input_register`, le quattro `irq_*` tranne le ISR) e
 * otto sono `t` -- le due ISR, le tre `tpd_*`, `plat_probe`, `plat_remove` e
 * `plat_dev_init`.
 */
static int ilitek_plat_probe(void);
static int ilitek_plat_remove(void);
static irqreturn_t ilitek_plat_isr_top_half(int irq, void *dev_id);
static irqreturn_t ilitek_plat_isr_bottom_half(int irq, void *dev_id);

/*
 * L'oggetto che `tpd_local_init` passa a `ilitek_tddi_dev_init`, letto voce
 * per voce dalle rilocazioni (vedi il cappello). NON e' costruito a runtime.
 */
static struct ilitek_hwif_info hwif = {
	.type = 0x18,				/* 0xffffff8009987370, non relocato */
	.name = "ILITEK_TDDI",			/* addend 0xffffff800923f97f */
	.owner = NULL,				/* 0xffffff8009987380, non relocato */
	.of_match_table = ilitek_of_match,	/* addend 0xffffff8008f7ea98 */
	.c32 = ilitek_plat_probe,		/* addend 0xffffff8008a566c4 */
	.c40 = ilitek_plat_remove,		/* addend 0xffffff8008a568fc */
	.driver = NULL,				/* 0xffffff80099873a0, non relocato */
};

/*
 * ===========================================================================
 * 1) ilitek_plat_tp_reset -- 0xffffff8008a55d0c, 168 byte
 * fabbrica: riga 36
 * ===========================================================================
 */
void ilitek_plat_tp_reset(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): edge delay = %d\n"@0xffffff800923f5d3,
	 * __func__ = "ilitek_plat_tp_reset"@0xffffff800923f5f8,
	 * __LINE__ = 36 ("52800482 mov"@0xffffff8008a55d34),
	 * argomento = idev+616 ("b9426903 ldr"@0xffffff8008a55d2c).
	 */
#line 36
	ILI_INFO("edge delay = %d\n", idev->c616);

	/* "b9415900 ldr"@0xffffff8008a55d44 + "97ffecd9 bl"@0xffffff8008a55d48 */
	tpd_gpio_output(idev->c344, 1);
	/*
	 * "52912b00 mov"@0xffffff8008a55d4c piu'
	 * "72a00820 movk"@0xffffff8008a55d50 danno 0x418958 = 4.295.000, e
	 * `udelay(n)` compila in `__const_udelay(n * 0x10c7)` con
	 * 0x10c7 = 4295: 4.295.000 / 4295 = 1000 esatto.
	 */
	udelay(1000);

	/* "2a1f03e1 mov"@0xffffff8008a55d5c + "97ffecd2 bl"@0xffffff8008a55d64 */
	tpd_gpio_output(idev->c344, 0);
	/*
	 * "5295d700 mov"@0xffffff8008a55d68 piu'
	 * "72a028e0 movk"@0xffffff8008a55d6c danno 0x147aeb8 = 21.475.000,
	 * cioe' 5000 * 4295.
	 */
	udelay(5000);

	/* "320003e1 orr"@0xffffff8008a55d78 + "97ffeccb bl"@0xffffff8008a55d80 */
	tpd_gpio_output(idev->c344, 1);

	/*
	 * "b9826908 ldrsw"@0xffffff8008a55d88 legge idev+616 CON SEGNO, e
	 * quello che segue e' un ciclo che chiama `udelay(1000)` esattamente
	 * quel numero di volte: "cb0803f3 neg"@0xffffff8008a55d90 parte da
	 * -N, "b1000673 adds"@0xffffff8008a55da0 incrementa e
	 * "54ffff83 b.cc"@0xffffff8008a55da4 ripete finche' non va in
	 * riporto. E' `mdelay`, che il kernel definisce proprio cosi' quando
	 * l'argomento non e' costante.
	 *
	 * Il nostro clang lo rende contando ALL'INGIU' con `sub`/`cbnz`
	 * invece che all'insu' con `neg`/`adds`/`b.cc`: una istruzione in
	 * meno, stesso ciclo. Vedi C5.
	 */
	mdelay(idev->c616);
}

/*
 * ===========================================================================
 * 2) ilitek_plat_input_register -- 0xffffff8008a55db4, 828 byte
 * ===========================================================================
 * Nessuna `printk`: questa funzione non ha nemmeno una riga di fabbrica da
 * annotare, e non contribuisce alla divergenza C1.
 */
void ilitek_plat_input_register(void)
{
	int i;

	/*
	 * "f9416108 ldr"@0xffffff8008a55dc8 legge il globale `tpd` di
	 * `mtk_tpd` (0xffffff800a0fc2c0), "f9400d00 ldr"@0xffffff8008a55dd4
	 * ne prende il campo a +24 -- l'`input_dev` -- e
	 * "f9000920 str"@0xffffff8008a55de0 lo scrive in idev+16. E' ESATTAMENTE
	 * la citazione che `ilitek.h` porta accanto al campo `c16`.
	 */
	idev->c16 = tpd->dev;

	/*
	 * I tasti fisici, se il device tree ne dichiara:
	 * "b9400109 ldr"@0xffffff8008a55de4 e' `use_tpd_button`
	 * (0xffffff800a0fbf94 = &tpd_dts_data + 12) e
	 * "b9400508 ldr"@0xffffff8008a55dec e' `tpd_key_num` (+16), provato
	 * contro 1 con un confronto CON SEGNO
	 * ("540001cb b.lt"@0xffffff8008a55df4).
	 * Il ciclo indicizza `tpd_key_local` (+20) con
	 * "b9401502 ldr"@0xffffff8008a55e08.
	 */
	if (tpd_dts_data.use_tpd_button && tpd_dts_data.tpd_key_num >= 1) {
		for (i = 0; i < tpd_dts_data.tpd_key_num; i++)
			/* "97ffc814 bl"@0xffffff8008a55e10 */
			input_set_capability(idev->c16, EV_KEY,
					     tpd_dts_data.tpd_key_local[i]);
	}

	/*
	 * I bit di capacita'. Gli offset dentro `struct input_dev` sono quelli
	 * dell'ABI di questo albero: `propbit` a +32, `evbit` a +40,
	 * `keybit` a +48.
	 *   "9100a001 add"@0xffffff8008a55e2c -> evbit, EV_ABS (3)
	 *   "9100a101 add"@0xffffff8008a55e44 -> evbit, EV_SYN (0)
	 *   "9100a101 add"@0xffffff8008a55e58 -> evbit, EV_KEY (1)
	 *   "9100c101 add"@0xffffff8008a55e6c -> keybit, 330 = BTN_TOUCH
	 *   "9100c101 add"@0xffffff8008a55e80 -> keybit, 325 = BTN_TOOL_FINGER
	 *   "91008101 add"@0xffffff8008a55e94 -> propbit, INPUT_PROP_DIRECT (1)
	 */
	set_bit(EV_ABS, idev->c16->evbit);
	set_bit(EV_SYN, idev->c16->evbit);
	set_bit(EV_KEY, idev->c16->evbit);
	set_bit(BTN_TOUCH, idev->c16->keybit);
	set_bit(BTN_TOOL_FINGER, idev->c16->keybit);
	set_bit(INPUT_PROP_DIRECT, idev->c16->propbit);

	/*
	 * "52800741 mov"@0xffffff8008a55ea0 e' 58 = ABS_MT_PRESSURE,
	 * "32001fe3 orr"@0xffffff8008a55ea4 e' 255, e min/fuzz/flat sono zero
	 * ("2a1f03e2 mov"@0xffffff8008a55ea8, "2a1f03e4 mov"@0xffffff8008a55eb0,
	 * "2a1f03e5 mov"@0xffffff8008a55eb4).
	 */
	input_set_abs_params(idev->c16, ABS_MT_PRESSURE, 0, 255, 0, 0);
	/*
	 * "52800141 mov"@0xffffff8008a55ec0 e' 10 slot,
	 * "321f03e2 orr"@0xffffff8008a55ec4 e' 2 = INPUT_MT_DIRECT.
	 */
	input_mt_init_slots(idev->c16, 10, INPUT_MT_DIRECT);

	/*
	 * I quattordici codici di tasto, ciascuno con la sua
	 * "97ffc7xx bl"@... verso `input_set_capability`. I valori sono gli
	 * immediati, nell'ordine in cui il binario li materializza.
	 */
	/* "52800e82 mov"@0xffffff8008a55ed8 -- 116 */
	input_set_capability(idev->c16, EV_KEY, KEY_POWER);
	/* "52800ce2 mov"@0xffffff8008a55eec -- 103 */
	input_set_capability(idev->c16, EV_KEY, KEY_UP);
	/* "52800d82 mov"@0xffffff8008a55f00 -- 108 */
	input_set_capability(idev->c16, EV_KEY, KEY_DOWN);
	/* "52800d22 mov"@0xffffff8008a55f14 -- 105 */
	input_set_capability(idev->c16, EV_KEY, KEY_LEFT);
	/* "52800d42 mov"@0xffffff8008a55f28 -- 106 */
	input_set_capability(idev->c16, EV_KEY, KEY_RIGHT);
	/* "321d07e2 orr"@0xffffff8008a55f3c -- 24 */
	input_set_capability(idev->c16, EV_KEY, KEY_O);
	/* "52800242 mov"@0xffffff8008a55f50 -- 18 */
	input_set_capability(idev->c16, EV_KEY, KEY_E);
	/* "52800642 mov"@0xffffff8008a55f64 -- 50 */
	input_set_capability(idev->c16, EV_KEY, KEY_M);
	/* "52800222 mov"@0xffffff8008a55f78 -- 17 */
	input_set_capability(idev->c16, EV_KEY, KEY_W);
	/* "320013e2 orr"@0xffffff8008a55f8c -- 31 */
	input_set_capability(idev->c16, EV_KEY, KEY_S);
	/* "528005e2 mov"@0xffffff8008a55fa0 -- 47 */
	input_set_capability(idev->c16, EV_KEY, KEY_V);
	/* "52800582 mov"@0xffffff8008a55fb4 -- 44 */
	input_set_capability(idev->c16, EV_KEY, KEY_Z);
	/* "528005c2 mov"@0xffffff8008a55fc8 -- 46 */
	input_set_capability(idev->c16, EV_KEY, KEY_C);
	/* "52800422 mov"@0xffffff8008a55fdc -- 33 */
	input_set_capability(idev->c16, EV_KEY, KEY_F);

	/*
	 * Gli stessi quattordici tasti, una seconda volta e in linea. Vedi C4:
	 * e' ridondante e di fabbrica c'e'. Le prime cinque cadono in
	 * `keybit[1]` (+56), le altre nove in `keybit[0]` (+48), e la maschera
	 * dice quale bit: 0x10000000000000 e' il bit 52, cioe' il tasto
	 * 64+52 = 116.
	 */
	/* "b24c0129 orr"@0xffffff8008a55ff4 -- bit 52 di keybit[1] = 116 */
	__set_bit(KEY_POWER, idev->c16->keybit);
	/* "b2590129 orr"@0xffffff8008a56008 -- bit 39 = 103 */
	__set_bit(KEY_UP, idev->c16->keybit);
	/* "b2540129 orr"@0xffffff8008a5601c -- bit 44 = 108 */
	__set_bit(KEY_DOWN, idev->c16->keybit);
	/* "b2570129 orr"@0xffffff8008a56030 -- bit 41 = 105 */
	__set_bit(KEY_LEFT, idev->c16->keybit);
	/* "b2560129 orr"@0xffffff8008a56044 -- bit 42 = 106 */
	__set_bit(KEY_RIGHT, idev->c16->keybit);
	/* "b268014a orr"@0xffffff8008a56058 -- bit 24 di keybit[0] */
	__set_bit(KEY_O, idev->c16->keybit);
	/* "b26e014a orr"@0xffffff8008a56068 -- bit 18 */
	__set_bit(KEY_E, idev->c16->keybit);
	/* "b24e014a orr"@0xffffff8008a56078 -- bit 50 */
	__set_bit(KEY_M, idev->c16->keybit);
	/* "b26f014a orr"@0xffffff8008a56088 -- bit 17 */
	__set_bit(KEY_W, idev->c16->keybit);
	/* "b261014a orr"@0xffffff8008a56098 -- bit 31 */
	__set_bit(KEY_S, idev->c16->keybit);
	/* "b251014a orr"@0xffffff8008a560a8 -- bit 47 */
	__set_bit(KEY_V, idev->c16->keybit);
	/* "b254014a orr"@0xffffff8008a560b8 -- bit 44 */
	__set_bit(KEY_Z, idev->c16->keybit);
	/* "b252014a orr"@0xffffff8008a560c8 -- bit 46 */
	__set_bit(KEY_C, idev->c16->keybit);
	/* "b25f0129 orr"@0xffffff8008a560d8 -- bit 33 */
	__set_bit(KEY_F, idev->c16->keybit);
}

/*
 * ===========================================================================
 * 3) ilitek_plat_irq_disable -- 0xffffff8008a560f0, 160 byte
 * fabbrica: righe 226, 232
 * ===========================================================================
 */
void ilitek_plat_irq_disable(void)
{
	unsigned long flag;

	/* "91034100 add"@0xffffff8008a56104 -- idev+208 = &idev->c208 */
	spin_lock_irqsave(&idev->c208, flag);

	/* "b942c109 ldr"@0xffffff8008a56114 + "340002e9 cbz"@0xffffff8008a56118 */
	if (!idev->c704)
		goto fine;

	/* "b9414d00 ldr"@0xffffff8008a5611c + "340001c0 cbz"@0xffffff8008a56120 */
	if (!idev->c332) {
		/*
		 * "\x013ILITEK: (%s, %d): gpio_to_irq (%d) is incorrect\n"@0xffffff800923f60d,
		 * __func__ = "ilitek_plat_irq_disable"@0xffffff800923f640,
		 * __LINE__ = 226 ("52801c42 mov"@0xffffff8008a56168).
		 * Il terzo argomento e' ZERO fisso
		 * ("2a1f03e3 mov"@0xffffff8008a5616c): il compilatore sa che
		 * su questo ramo `idev->c332` vale zero e lo sostituisce.
		 */
#line 226
		ILI_ERR("gpio_to_irq (%d) is incorrect\n", idev->c332);
		goto fine;
	}

	/* "97db9286 bl"@0xffffff8008a56124 */
	disable_irq_nosync(idev->c332);
	/* "b902c11f str"@0xffffff8008a5612c */
	idev->c704 = 0;
	/*
	 * "\x016ILITEK: (%s, %d): Disable irq success\n"@0xffffff800923f658,
	 * __LINE__ = 232 ("52801d02 mov"@0xffffff8008a5614c).
	 * E' `ILI_DBG`, non `ILI_INFO`: la guardia e'
	 * "39656108 ldrb"@0xffffff8008a56134 piu'
	 * "340001e8 cbz"@0xffffff8008a56138, e salta la costruzione degli
	 * argomenti, non solo la `printk`.
	 */
#line 232
	ILI_DBG("Disable irq success\n");

fine:
	/* "91034100 add"@0xffffff8008a5617c + "94108339 bl"@0xffffff8008a56180 */
	spin_unlock_irqrestore(&idev->c208, flag);
}

/*
 * ===========================================================================
 * 4) ilitek_plat_irq_enable -- 0xffffff8008a56190, 168 byte
 * fabbrica: righe 248, 254
 * ===========================================================================
 */
void ilitek_plat_irq_enable(void)
{
	unsigned long flag;

	/* "91034100 add"@0xffffff8008a561a4 */
	spin_lock_irqsave(&idev->c208, flag);

	/*
	 * "b942c109 ldr"@0xffffff8008a561b4, "7100053f cmp"@0xffffff8008a561b8:
	 * il confronto e' con 1, non con zero -- simmetrico ma non identico a
	 * quello di `irq_disable`.
	 */
	if (idev->c704 == 1)
		goto fine;

	/* "b9414d00 ldr"@0xffffff8008a561c0 + "340001e0 cbz"@0xffffff8008a561c4 */
	if (!idev->c332) {
		/*
		 * Stesso formato di `irq_disable`:
		 * "\x013ILITEK: (%s, %d): gpio_to_irq (%d) is incorrect\n"@0xffffff800923f60d,
		 * __func__ = "ilitek_plat_irq_enable"@0xffffff800923f681,
		 * __LINE__ = 248 ("321d13e2 orr"@0xffffff8008a56210).
		 */
#line 248
		ILI_ERR("gpio_to_irq (%d) is incorrect\n", idev->c332);
		goto fine;
	}

	/* "97db92fb bl"@0xffffff8008a561c8 */
	enable_irq(idev->c332);
	/* "b902c109 str"@0xffffff8008a561d4 */
	idev->c704 = 1;
	/*
	 * "\x016ILITEK: (%s, %d): Enable irq success\n"@0xffffff800923f698,
	 * __LINE__ = 254 ("321f1be2 orr"@0xffffff8008a561f4),
	 * guardia "39656108 ldrb"@0xffffff8008a561dc.
	 */
#line 254
	ILI_DBG("Enable irq success\n");

fine:
	/* "9410830f bl"@0xffffff8008a56228 */
	spin_unlock_irqrestore(&idev->c208, flag);
}

/*
 * ===========================================================================
 * 5) ilitek_plat_irq_unregister -- 0xffffff8008a56238, 40 byte
 * ===========================================================================
 * Nessuna `printk`.
 */
void ilitek_plat_irq_unregister(void)
{
	/*
	 * "f9400d00 ldr"@0xffffff8008a5624c prende idev+24 (il `device` del
	 * client, scritto dal gruppo B), "b9414d01 ldr"@0xffffff8008a56250
	 * l'IRQ, e "aa1f03e2 mov"@0xffffff8008a56248 mette a NULL il terzo
	 * argomento.
	 */
	devm_free_irq(idev->c24, idev->c332, NULL);
}

/*
 * ===========================================================================
 * 6) ilitek_plat_irq_register -- 0xffffff8008a56260, 324 byte
 * fabbrica: righe 329, 339, 341, 344
 * ===========================================================================
 */
int ilitek_plat_irq_register(int type)
{
	int ret;

	/* "b902c11f str"@0xffffff8008a56280 */
	idev->c704 = 0;

	/*
	 * La mappatura dell'IRQ si fa UNA VOLTA SOLA:
	 * "39692289 ldrb"@0xffffff8008a56284 legge il flag e
	 * "37000309 tbnz"@0xffffff8008a56288 salta tutto il blocco se e' gia'
	 * a uno.
	 */
	if (!ilitek_irq_mappato) {
		struct device_node *node;

		/*
		 * "91020021 add"@0xffffff8008a56290 -> 0xffffff8008f7e080,
		 * che e' la tabella "mediatek,touch" e NON quella di `hwif`
		 * (0xffffff8008f7ea98, "mediatek,cap_touch"): due indirizzi
		 * diversi, due tabelle diverse. Primo e terzo argomento NULL
		 * ("aa1f03e0 mov"@0xffffff8008a56294,
		 * "aa1f03e2 mov"@0xffffff8008a56298).
		 * "94043cc9 bl"@0xffffff8008a5629c
		 */
		node = of_find_matching_node_and_match(NULL,
						       ilitek_of_match_irq,
						       NULL);
		/* "b40000e0 cbz"@0xffffff8008a562a0 */
		if (node)
			/*
			 * "2a1f03e1 mov"@0xffffff8008a562a4 (indice 0) piu'
			 * "94045474 bl"@0xffffff8008a562a8, e il risultato va
			 * in idev+332 ("b9014d00 str"@0xffffff8008a562b4).
			 */
			idev->c332 = irq_of_parse_and_map(node, 0);

		/*
		 * "\x016ILITEK: (%s, %d): idev->irq_num = %d\n"@0xffffff800923f6c0,
		 * __func__ = "ilitek_plat_irq_register"@0xffffff800923f6e8,
		 * __LINE__ = 329 ("52802922 mov"@0xffffff8008a562d4).
		 * IL FORMATO NOMINA IL CAMPO: `idev->irq_num` e' idev+332.
		 */
#line 329
		ILI_INFO("idev->irq_num = %d\n", idev->c332);
		/* "39292289 strb"@0xffffff8008a562e4 */
		ilitek_irq_mappato = true;
	}

	/*
	 * "32130268 orr"@0xffffff8008a562f0 aggiunge 0x2000 = IRQF_ONESHOT al
	 * tipo ricevuto, e "93407d04 sxtw"@0xffffff8008a56300 lo estende CON
	 * SEGNO: il parametro e' `int`.
	 * Le due funzioni sono "910e9042 add"@0xffffff8008a56304 ->
	 * 0xffffff8008a563a4 = ilitek_plat_isr_top_half e
	 * "91149063 add"@0xffffff8008a56308 -> 0xffffff8008a56524 =
	 * ilitek_plat_isr_bottom_half; il nome e' "ilitek"@0xffffff800923f701
	 * ("911c04a5 add"@0xffffff8008a5630c) e l'ultimo argomento e' NULL
	 * ("aa1f03e6 mov"@0xffffff8008a56310).
	 */
	ret = devm_request_threaded_irq(idev->c24, idev->c332,
					ilitek_plat_isr_top_half,
					ilitek_plat_isr_bottom_half,
					type | IRQF_ONESHOT, "ilitek", NULL);

	/*
	 * "7100067f cmp"@0xffffff8008a56318 e "71000a7f cmp"@0xffffff8008a56324:
	 * il tipo si stampa solo se e' 1 o 2, e per nessun altro valore.
	 */
	if (type == 1)
		/*
		 * "\x016ILITEK: (%s, %d): IRQ TYPE = IRQF_TRIGGER_RISING\n"@0xffffff800923f73d,
		 * __LINE__ = 341 ("52802aa2 mov"@0xffffff8008a56354).
		 */
#line 341
		ILI_INFO("IRQ TYPE = IRQF_TRIGGER_RISING\n");
	else if (type == 2)
		/*
		 * "\x016ILITEK: (%s, %d): IRQ TYPE = IRQF_TRIGGER_FALLING\n"@0xffffff800923f708,
		 * __LINE__ = 339 ("52802a62 mov"@0xffffff8008a5633c).
		 */
#line 339
		ILI_INFO("IRQ TYPE = IRQF_TRIGGER_FALLING\n");

	/* "34000154 cbz"@0xffffff8008a5635c */
	if (ret)
		/*
		 * "\x013ILITEK: (%s, %d): Failed to register irq handler, irq = %d, ret = %d\n"@0xffffff800923f771,
		 * __LINE__ = 344 ("52802b02 mov"@0xffffff8008a56378),
		 * argomenti idev+332 ("b9414d03 ldr"@0xffffff8008a56370) e
		 * `ret` ("2a1403e4 mov"@0xffffff8008a5637c).
		 */
		ILI_ERR("Failed to register irq handler, irq = %d, ret = %d\n",
#line 344
			idev->c332, ret);

	/*
	 * "b902c109 str"@0xffffff8008a56390 -- il flag va a 1 SEMPRE, anche
	 * quando la registrazione e' fallita. E' quello che il binario fa.
	 */
	idev->c704 = 1;

	/* "2a1403e0 mov"@0xffffff8008a5638c */
	return ret;
}

/*
 * ===========================================================================
 * 7) ilitek_plat_isr_top_half -- 0xffffff8008a563a4, 384 byte
 * fabbrica: righe 263, 269, 275, 286, 292
 * ===========================================================================
 */
static irqreturn_t ilitek_plat_isr_top_half(int irq, void *dev_id)
{
	/* "b9414d09 ldr"@0xffffff8008a563bc + "6b00013f cmp"@0xffffff8008a563c0 */
	if (irq != idev->c332) {
		/*
		 * "\x013ILITEK: (%s, %d): Incorrect irq number (%d)\n"@0xffffff800923f802,
		 * __func__ = "ilitek_plat_isr_top_half"@0xffffff800923f831,
		 * __LINE__ = 263 ("528020e2 mov"@0xffffff8008a56430).
		 */
#line 263
		ILI_ERR("Incorrect irq number (%d)\n", irq);
		/* "2a1f03f3 mov"@0xffffff8008a56438 -- IRQ_NONE */
		return IRQ_NONE;
	}

	/* "b942dd09 ldr"@0xffffff8008a563c8 + "7100053f cmp"@0xffffff8008a563cc */
	if (idev->c732 == 1) {
		/* "b902dd1f str"@0xffffff8008a563d4 */
		idev->c732 = 0;
		/*
		 * "\x016ILITEK: (%s, %d): interrupt for mp test, ignore\n"@0xffffff800923f84a,
		 * __LINE__ = 269 ("528021a2 mov"@0xffffff8008a563f4),
		 * guardia "39656129 ldrb"@0xffffff8008a563dc.
		 */
#line 269
		ILI_DBG("interrupt for mp test, ignore\n");
		/*
		 * "9108e100 add"@0xffffff8008a56400 -- idev+568 = &idev->inq;
		 * "320007e1 orr"@0xffffff8008a56404 e' TASK_NORMAL (3),
		 * "320003e2 orr"@0xffffff8008a56408 e' 1, l'ultimo e' NULL:
		 * e' `wake_up(&idev->inq)`.
		 */
		wake_up(&idev->inq);
		/* "320003f3 orr"@0xffffff8008a56410 -- IRQ_HANDLED */
		return IRQ_HANDLED;
	}

	/* "3949f509 ldrb"@0xffffff8008a56440 + "340000e9 cbz"@0xffffff8008a56444 */
	if (idev->c637) {
		/*
		 * "\x016ILITEK: (%s, %d): Proximity event, ignore interrupt!\n"@0xffffff800923f87d,
		 * __LINE__ = 275 ("52802262 mov"@0xffffff8008a56458).
		 * NON e' protetta dal byte di debug: il salto a
		 * "14000029 b"@0xffffff8008a5645c va dritto alla `printk`.
		 */
#line 275
		ILI_INFO("Proximity event, ignore interrupt!\n");
		/* "320003f3 orr"@0xffffff8008a56504 -- IRQ_HANDLED */
		return IRQ_HANDLED;
	}

	/*
	 * Il formato che NOMINA SETTE CAMPI -- vedi il cappello.
	 * "\x016ILITEK: (%s, %d): report: %d, rst: %d, fw: %d, switch: %d, mp: %d, sleep: %d, esd: %d\n"@0xffffff800923f8b5,
	 * __LINE__ = 286 ("528023c2 mov"@0xffffff8008a56498),
	 * guardia "39656289 ldrb"@0xffffff8008a56464.
	 * Gli ultimi due argomenti passano dalla pila:
	 * "b9000be8 str"@0xffffff8008a5649c e "b90003e9 str"@0xffffff8008a564a0.
	 */
	ILI_DBG("report: %d, rst: %d, fw: %d, switch: %d, mp: %d, sleep: %d, esd: %d\n",
		idev->c531, idev->c708, idev->c716, idev->c728,
#line 286
		idev->c720, idev->c724, idev->c736);

	/*
	 * Sette prove in cascata, nell'ordine esatto del binario: la prima e'
	 * «e' acceso il rapporto?» e le altre sei «c'e' qualcosa in corso?».
	 *   "39484d09 ldrb"@0xffffff8008a564ac  c531 == 0 -> ignora
	 *   "b942c509 ldr"@0xffffff8008a564b4   c708 != 0 -> ignora
	 *   "b942cd09 ldr"@0xffffff8008a564bc   c716 != 0 -> ignora
	 *   "b942d909 ldr"@0xffffff8008a564c4   c728 != 0 -> ignora
	 *   "b942d109 ldr"@0xffffff8008a564cc   c720 != 0 -> ignora
	 *   "b942d509 ldr"@0xffffff8008a564d4   c724 != 0 -> ignora
	 *   "b942e108 ldr"@0xffffff8008a564dc   c736 != 0 -> ignora
	 */
	if (!idev->c531 || idev->c708 || idev->c716 || idev->c728 ||
	    idev->c720 || idev->c724 || idev->c736) {
		/*
		 * "\x016ILITEK: (%s, %d): ignore interrupt !\n"@0xffffff800923f90e,
		 * __LINE__ = 292 ("52802482 mov"@0xffffff8008a564fc),
		 * guardia "39656288 ldrb"@0xffffff8008a564e4.
		 */
#line 292
		ILI_DBG("ignore interrupt !\n");
		/* "320003f3 orr"@0xffffff8008a56504 */
		return IRQ_HANDLED;
	}

	/* "321f03f3 orr"@0xffffff8008a5651c -- IRQ_WAKE_THREAD (2) */
	return IRQ_WAKE_THREAD;
}

/*
 * ===========================================================================
 * 8) ilitek_plat_isr_bottom_half -- 0xffffff8008a56524, 108 byte
 * fabbrica: riga 302
 * ===========================================================================
 */
static irqreturn_t ilitek_plat_isr_bottom_half(int irq, void *dev_id)
{
	/*
	 * "f8470c08 ldr"@0xffffff8008a56538 e' un `ldr` PRE-INDICIZZATO:
	 * calcola idev+112 (il `touch_mutex`) e nello stesso colpo ne legge il
	 * primo campo, `owner`. "f100211f cmp"@0xffffff8008a5653c lo confronta
	 * con 8 SENZA segno, che e' come clang rende `(owner & ~7) != 0` --
	 * cioe' `mutex_is_locked()`.
	 */
	if (mutex_is_locked(&idev->touch_mutex)) {
		/*
		 * "\x016ILITEK: (%s, %d): touch is locked, ignore\n"@0xffffff800923f936,
		 * __func__ = "ilitek_plat_isr_bottom_half"@0xffffff800923f963,
		 * __LINE__ = 302 ("528025c2 mov"@0xffffff8008a56578),
		 * guardia "39656108 ldrb"@0xffffff8008a56560.
		 */
#line 302
		ILI_DBG("touch is locked, ignore\n");
		/* "320003e0 orr"@0xffffff8008a56584 -- IRQ_HANDLED */
		return IRQ_HANDLED;
	}

	/* "9410738e bl"@0xffffff8008a56544 */
	mutex_lock(&idev->touch_mutex);
	/* "97fff879 bl"@0xffffff8008a56548 */
	ilitek_tddi_report_handler();
	/* "9101c100 add"@0xffffff8008a56550 + "9410739e bl"@0xffffff8008a56554 */
	mutex_unlock(&idev->touch_mutex);

	return IRQ_HANDLED;
}

/*
 * ===========================================================================
 * 9) ilitek_plat_gpio_register -- INCORPORATA in ilitek_plat_probe
 * fabbrica: righe 176..206
 * ===========================================================================
 * Vedi C2: il `__func__` di tutte le sue `printk` e'
 * "ilitek_plat_gpio_register"@0xffffff800923fade, non
 * "ilitek_plat_probe"@0xffffff800923fa57.
 */
static int ilitek_plat_gpio_register(void)
{
	int ret = 0;

	/*
	 * "b26003e9 orr"@0xffffff8008a566fc costruisce 0x100000000 e
	 * "f900ad09 str"@0xffffff8008a56710 lo scrive a idev+344 con UNO STORE
	 * DA 64 BIT: riempie due campi a 32 in un colpo, c344 = 0 e c348 = 1.
	 */
	idev->c344 = 0;
	idev->c348 = 1;

	/*
	 * "\x016ILITEK: (%s, %d): TP INT: %d\n"@0xffffff800923fabe,
	 * __LINE__ = 176 ("52801602 mov"@0xffffff8008a56704).
	 * Il terzo argomento e' la COSTANTE 1
	 * ("320003e3 orr"@0xffffff8008a56708): il compilatore sa quanto vale
	 * `idev->c348`, l'ha appena scritto.
	 */
#line 176
	ILI_INFO("TP INT: %d\n", idev->c348);
	/*
	 * "\x016ILITEK: (%s, %d): TP RESET: %d\n"@0xffffff800923faf8,
	 * __LINE__ = 177 ("52801622 mov"@0xffffff8008a56724).
	 * Qui invece il valore si RILEGGE dalla memoria
	 * ("b9415903 ldr"@0xffffff8008a56728).
	 */
#line 177
	ILI_INFO("TP RESET: %d\n", idev->c344);

	/*
	 * `gpio_is_valid` con `ARCH_NR_GPIOS` = 512: il confronto e' senza
	 * segno contro 0x200 ("7108007f cmp"@0xffffff8008a5673c e
	 * "540006e2 b.cs"@0xffffff8008a56740).
	 */
	if (!gpio_is_valid(idev->c348)) {
		/*
		 * "\x013ILITEK: (%s, %d): Invalid INT gpio: %d\n"@0xffffff800923fb1a,
		 * __LINE__ = 180 ("52801682 mov"@0xffffff8008a5682c).
		 */
#line 180
		ILI_ERR("Invalid INT gpio: %d\n", idev->c348);
		return -EBADR;
	}

	/* "7108011f cmp"@0xffffff8008a56748 + "54000762 b.cs"@0xffffff8008a5674c */
	if (!gpio_is_valid(idev->c344)) {
		/*
		 * "\x013ILITEK: (%s, %d): Invalid RESET gpio: %d\n"@0xffffff800923fb44,
		 * __LINE__ = 185 ("52801722 mov"@0xffffff8008a56848).
		 */
#line 185
		ILI_ERR("Invalid RESET gpio: %d\n", idev->c344);
		return -EBADR;
	}

	/*
	 * "912dc021 add"@0xffffff8008a56754 -> "TP_INT"@0xffffff800923fb70,
	 * "97e8875a bl"@0xffffff8008a5675c.
	 */
	ret = gpio_request(idev->c348, "TP_INT");
	/* "36f80220 tbz"@0xffffff8008a56760 -- solo il bit di segno */
	if (ret < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): Request IRQ GPIO failed, ret = %d\n"@0xffffff800923fb77,
		 * __LINE__ = 191 ("528017e2 mov"@0xffffff8008a56778).
		 */
#line 191
		ILI_ERR("Request IRQ GPIO failed, ret = %d\n", ret);
		/* "97e88711 bl"@0xffffff8008a56788 */
		gpio_free(idev->c348);
		/* "97e8874a bl"@0xffffff8008a5679c -- si RIPROVA, stesso nome */
		ret = gpio_request(idev->c348, "TP_INT");
		/* "37f805c0 tbnz"@0xffffff8008a567a0 */
		if (ret < 0)
			/*
			 * "\x013ILITEK: (%s, %d): Retrying request INT GPIO still failed , ret = %d\n"@0xffffff800923fbae,
			 * __LINE__ = 195 ("52801862 mov"@0xffffff8008a5686c).
			 */
			ILI_ERR("Retrying request INT GPIO still failed , ret = %d\n",
#line 195
				ret);
	}

	/*
	 * "912fd421 add"@0xffffff8008a567ac -> "TP_RESET"@0xffffff800923fbf5,
	 * "97e88744 bl"@0xffffff8008a567b4.
	 */
	ret = gpio_request(idev->c344, "TP_RESET");
	/* "36f805e0 tbz"@0xffffff8008a567bc */
	if (ret < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): Request RESET GPIO failed, ret = %d\n"@0xffffff800923fbfe,
		 * __LINE__ = 202 ("52801942 mov"@0xffffff8008a567d0).
		 */
#line 202
		ILI_ERR("Request RESET GPIO failed, ret = %d\n", ret);
		/* "97e886fa bl"@0xffffff8008a567e4 */
		gpio_free(idev->c344);
		/* "97e88733 bl"@0xffffff8008a567f8 */
		ret = gpio_request(idev->c344, "TP_RESET");
		/* "36f803c0 tbz"@0xffffff8008a56800 */
		if (ret < 0)
			/*
			 * "\x013ILITEK: (%s, %d): Retrying request RESET GPIO still failed , ret = %d\n"@0xffffff800923fc37,
			 * __LINE__ = 206 ("528019c2 mov"@0xffffff8008a56814).
			 */
			ILI_ERR("Retrying request RESET GPIO still failed , ret = %d\n",
#line 206
				ret);
	}

	/*
	 * "97e8716b bl"@0xffffff8008a56880 verso `gpio_to_desc` seguito da
	 * "97e87686 bl"@0xffffff8008a56884 verso `gpiod_direction_input`: e'
	 * `gpio_direction_input`, che il kernel definisce cosi'.
	 * IL RISULTATO NON E' USATO.
	 */
	gpio_direction_input(idev->c348);

	return ret;
}

/*
 * ===========================================================================
 * 10) ilitek_plat_probe -- 0xffffff8008a566c4, 568 byte
 * fabbrica: righe 365, 372, 375
 * ===========================================================================
 * E' il campo `c32` della `hwif`, chiamato da `ilitek_i2c_probe` del gruppo
 * B: vedi il cappello.
 */
static int ilitek_plat_probe(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): platform probe\n"@0xffffff800923fa33,
	 * __func__ = "ilitek_plat_probe"@0xffffff800923fa57,
	 * __LINE__ = 365 ("52802da2 mov"@0xffffff8008a566e0).
	 */
#line 365
	ILI_INFO("platform probe\n");

	/* "37f80173 tbnz"@0xffffff8008a56888 sul risultato della incorporata */
	if (ilitek_plat_gpio_register() < 0)
		/*
		 * "\x013ILITEK: (%s, %d): Register gpio failed\n"@0xffffff800923fa69,
		 * __LINE__ = 372 ("52802e82 mov"@0xffffff8008a568c4).
		 * NON si esce: si continua lo stesso, e questo e' cio' che il
		 * binario fa -- dopo la `printk` a "97db7303 bl"@0xffffff8008a568c8
		 * viene "97fff95f bl"@0xffffff8008a568cc, la stessa
		 * `ilitek_tddi_init` del percorso che riesce.
		 */
#line 372
		ILI_ERR("Register gpio failed\n");

	/* "97fff96f bl"@0xffffff8008a5688c */
	if (ilitek_tddi_init() < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): platform probe failed\n"@0xffffff800923fa93,
		 * __LINE__ = 375 ("52802ee2 mov"@0xffffff8008a568e4).
		 */
#line 375
		ILI_ERR("platform probe failed\n");
		/* "12800240 mov"@0xffffff8008a568ec -- -19 = -ENODEV */
		return -ENODEV;
	}

	/*
	 * "b9415500 ldr"@0xffffff8008a56898 legge idev+340 -- il tipo di
	 * trigger, messo a 2 dal gruppo B -- e
	 * "97fffe71 bl"@0xffffff8008a5689c registra l'IRQ.
	 * IL RISULTATO NON E' USATO.
	 */
	ilitek_plat_irq_register(idev->c340);

	/* "b9095509 str"@0xffffff8008a568ac */
	ilitek_probe_riuscito = 1;

	/* "2a1f03e0 mov"@0xffffff8008a568a0 */
	return 0;
}

/*
 * ===========================================================================
 * 11) ilitek_plat_remove -- 0xffffff8008a568fc, 48 byte
 * fabbrica: riga 385
 * ===========================================================================
 */
static int ilitek_plat_remove(void)
{
	/*
	 * Il formato e' "\x016ILITEK: (%s, %d): "@0xffffff800923e290 e finisce
	 * li' -- messaggio VUOTO, lo stesso letterale che usa
	 * `ilitek_i2c_remove` del gruppo B.
	 * __func__ = "ilitek_plat_remove"@0xffffff800923fc80,
	 * __LINE__ = 385 ("52803022 mov"@0xffffff8008a56914).
	 */
#line 385
	ILI_INFO("");

	/* "97fffa46 bl"@0xffffff8008a5691c */
	ilitek_tddi_dev_remove();

	/* "2a1f03e0 mov"@0xffffff8008a56920 */
	return 0;
}

/*
 * ===========================================================================
 * 12) tpd_local_init -- 0xffffff8008a56590, 188 byte
 * fabbrica: righe 407, 410, 414
 * ===========================================================================
 * OMONIMA: tre definizioni nella mappa, questa e' la prima. Vedi il cappello.
 */
static int tpd_local_init(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): TPD init device driver\n"@0xffffff800923f98b,
	 * __func__ = "tpd_local_init"@0xffffff800923f9b7,
	 * __LINE__ = 407 ("528032e2 mov"@0xffffff8008a565a8).
	 */
	ILI_INFO("TPD init device driver\n");

	/*
	 * "910dc000 add"@0xffffff8008a565b4 -> 0xffffff8009987370, la `hwif`
	 * statica; "97fffb53 bl"@0xffffff8008a565b8.
	 * "37f80240 tbnz"@0xffffff8008a565bc prova solo il bit di segno.
	 */
	if (ilitek_tddi_dev_init(&hwif) < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to register i2c/spi bus driver\n"@0xffffff800923f9c6,
		 * __LINE__ = 410 ("52803342 mov"@0xffffff8008a56614).
		 */
		ILI_ERR("Failed to register i2c/spi bus driver\n");
		/* "12800240 mov"@0xffffff8008a5661c -- -19 = -ENODEV */
		return -ENODEV;
	}

	/* "b9495508 ldr"@0xffffff8008a565c4 + "34000308 cbz"@0xffffff8008a565c8 */
	if (!ilitek_probe_riuscito) {
		/*
		 * "\x013ILITEK: (%s, %d): Add error touch panel driver\n"@0xffffff800923fa01,
		 * __LINE__ = 414 ("528033c2 mov"@0xffffff8008a56638).
		 */
		ILI_ERR("Add error touch panel driver\n");
		/* "12800000 mov"@0xffffff8008a56640 -- meno uno, non -ENODEV */
		return -1;
	}

	/* "b94f9508 ldr"@0xffffff8008a565d0 + "340000c8 cbz"@0xffffff8008a565d4 */
	if (tpd_dts_data.use_tpd_button)
		/*
		 * "913e6021 add"@0xffffff8008a565dc calcola &tpd_key_num,
		 * "91007022 add"@0xffffff8008a565e0 il terzo argomento, e
		 * "b8404420 ldr"@0xffffff8008a565e4 e' un `ldr` POST-INDICIZZATO
		 * che carica `tpd_key_num` e lascia in x1 l'indirizzo del
		 * campo successivo, `tpd_key_local`.
		 * "97ffefa3 bl"@0xffffff8008a565e8
		 */
		tpd_button_setting(tpd_dts_data.tpd_key_num,
				   tpd_dts_data.tpd_key_local,
				   tpd_dts_data.tpd_key_dim_local);

	/*
	 * "b9093909 str"@0xffffff8008a565f8 -- l'indirizzo e'
	 * 0xffffff800a0fc938, scritto a 1 e mai letto dentro il confine delle
	 * 154.
	 *
	 * E' `tpd_load_status`, il globale che ALPS dichiara in `tpd.h`
	 * («extern int tpd_load_status;  / * 0: failed, 1: success * /») e
	 * con cui un driver touch dice a `mtk_tpd` di essersi caricato. La
	 * lettura non e' certificata da un simbolo -- l'oracolo non contiene
	 * simboli di dati -- ma tre cose la sostengono: e' un `int` scritto a
	 * 1 sul solo percorso che riesce, sta fuori da `struct
	 * ilitek_tddi_dev`, e con un `static` di questo file la scrittura
	 * SPARISCE (nessuno la legge, clang la elimina) e la funzione misura
	 * 180 byte invece di 188.
	 */
	tpd_load_status = 1;

	/* "2a1f03e0 mov"@0xffffff8008a565f4 */
	return 0;
}

/*
 * ===========================================================================
 * 13) tpd_suspend -- 0xffffff8008a5664c, 60 byte
 * fabbrica: riga 360
 * ===========================================================================
 */
static void tpd_suspend(struct device *dev)
{
	/*
	 * "2a1f03e0 mov"@0xffffff8008a56654 -- l'argomento e' ZERO, e
	 * "97fff70c bl"@0xffffff8008a56658 chiama
	 * `ilitek_tddi_sleep_handler`. L'argomento `dev` NON e' usato.
	 */
	if (ilitek_tddi_sleep_handler(0) < 0)
		/*
		 * "\x013ILITEK: (%s, %d): TP suspend failed\n"@0xffffff800923fc93,
		 * __func__ = "tpd_suspend"@0xffffff800923fcba,
		 * __LINE__ = 360 ("52802d02 mov"@0xffffff8008a56678).
		 */
		ILI_ERR("TP suspend failed\n");
}

/*
 * ===========================================================================
 * 14) tpd_resume -- 0xffffff8008a56688, 60 byte
 * fabbrica: riga 354
 * ===========================================================================
 * La riga 354 e' PRIMA della 360 di `tpd_suspend`, quindi di fabbrica
 * `tpd_resume` sta piu' in alto nel file. Qui l'ordine e' quello degli
 * INDIRIZZI, che e' l'opposto: e' una scelta, dichiarata, e riguarda solo
 * la divergenza C1.
 */
static void tpd_resume(struct device *dev)
{
	/*
	 * "321f03e0 orr"@0xffffff8008a56690 -- l'argomento e' 2, e
	 * "97fff6fd bl"@0xffffff8008a56694 chiama la stessa funzione di
	 * `tpd_suspend`.
	 */
	if (ilitek_tddi_sleep_handler(2) < 0)
		/*
		 * "\x013ILITEK: (%s, %d): TP resume failed\n"@0xffffff800923fcc6,
		 * __func__ = "tpd_resume"@0xffffff800923fcec,
		 * __LINE__ = 354 ("52802c42 mov"@0xffffff8008a566b4).
		 */
		ILI_ERR("TP resume failed\n");
}

/*
 * La `struct tpd_driver_t` a 0xffffff8009987338, che
 * `ilitek_plat_dev_init` passa a `tpd_driver_add`
 * ("910ce000 add"@0xffffff80093833d0). Nell'immagine e' TUTTA A ZERO -- 56
 * byte -- perche' ogni suo campo e' un puntatore relocato; i valori si
 * leggerebbero con `tools/relocazioni.py` come per la `hwif`.
 *
 * QUESTO LOTTO NON LI HA LETTI. La forma qui sotto viene dai NOMI delle
 * funzioni che il gruppo definisce e che nessun altro chiama -- e' la
 * lettura piu' semplice, non una misura. Va chiusa leggendo le sette
 * rilocazioni a 0xffffff8009987338..0xffffff800998736f, esattamente come si
 * e' fatto per la `hwif`.
 */
static struct tpd_driver_t ilitek_device_driver = {
	.tpd_device_name = "ILITEK_TDDI",	/* DA VERIFICARE: non letto */
	.tpd_local_init = tpd_local_init,	/* DA VERIFICARE: non letto */
	.suspend = tpd_suspend,			/* DA VERIFICARE: non letto */
	.resume = tpd_resume,			/* DA VERIFICARE: non letto */
};

/*
 * ===========================================================================
 * 15) ilitek_plat_dev_init -- 0xffffff80093833a8, 112 byte, `.init.text`
 * fabbrica: righe 436, 440
 * ===========================================================================
 * E' l'unica funzione del blocco che NON sta in `.text`: la mappa la trova a
 * 0xffffff80093833a8, quasi 10 MB piu' avanti, ed e' il motivo per cui
 * `verificaistruzioni.py` ha bisogno del secondo intervallo.
 */
static int __init ilitek_plat_dev_init(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): ILITEK TP driver init for MTK\n"@0xffffff800923fcf7,
	 * __func__ = "ilitek_plat_dev_init"@0xffffff800923fd2a,
	 * __LINE__ = 436 ("52803682 mov"@0xffffff80093833c0).
	 */
#line 436
	ILI_INFO("ILITEK TP driver init for MTK\n");

	/* "97db35fa bl"@0xffffff80093833c8 -- nessun argomento */
	tpd_get_dts_info();

	/*
	 * "910ce000 add"@0xffffff80093833d0 -> 0xffffff8009987338 e
	 * "97db37ec bl"@0xffffff80093833d4; "37f80080 tbnz"@0xffffff80093833d8
	 * prova solo il bit di segno.
	 */
	if (tpd_driver_add(&ilitek_device_driver) < 0) {
		/*
		 * "\x013ILITEK: (%s, %d): ILITEK add TP driver failed\n"@0xffffff800923fd3f,
		 * __LINE__ = 440 ("52803702 mov"@0xffffff80093833f8).
		 */
#line 440
		ILI_ERR("ILITEK add TP driver failed\n");
		/* "97db3828 bl"@0xffffff8009383408 */
		tpd_driver_remove(&ilitek_device_driver);
		/* "12800240 mov"@0xffffff800938340c -- -19 = -ENODEV */
		return -ENODEV;
	}

	/* "2a1f03e0 mov"@0xffffff80093833dc */
	return 0;
}

/*
 * ===========================================================================
 * LA REGISTRAZIONE DELL'INITCALL -- MISURATA, NON DEDOTTA
 * ===========================================================================
 * `ilitek_plat_dev_init` e' `static`, sta in `.init.text` e NESSUNA delle 154
 * la chiama: l'unica cosa che la tiene in vita e' la tabella degli initcall.
 * Il puntatore c'e' davvero, e si trova cercando in `.rela.dyn` le
 * rilocazioni il cui addend e' il suo indirizzo -- ce ne sono due, e la
 * prima e' la tabella:
 *
 *   0xffffff800941f160 -> input_leds_init
 *   0xffffff800941f168 -> evdev_init
 *   0xffffff800941f170 -> kpd_pdrv_init
 *   0xffffff800941f178 -> tpd_log_init
 *   0xffffff800941f180 -> ilitek_plat_dev_init      <-- qui
 *   0xffffff800941f188 -> tpd_driver_init
 *   0xffffff800941f190 -> tpd_driver_init
 *   0xffffff800941f198 -> uinput_misc_init
 *
 * I vicini danno il LIVELLO senza doverlo indovinare: `evdev_init` e
 * `input_leds_init` sono `module_init` (livello 6) in ogni albero Linux. E i
 * due `tpd_driver_init` subito dopo sono gli altri due driver touch che il
 * config di fabbrica accende -- Goodix e Focaltech -- che non hanno
 * rinominato la propria init come ha fatto Ilitek.
 *
 * IL `module_exit` C'E', E QUESTA NOTA DICEVA IL CONTRARIO.
 *
 * La stesura del 2026-08-21 scriveva qui: «NESSUN module_exit. Non c'e' una
 * .exit.text con una funzione di questo blocco». **E' falso**, ed e' stato
 * smentito lo stesso giorno da una ricognizione che ha guardato dove nessuno
 * strumento del progetto guarda.
 *
 * LA CAUSA E' PIU' GRANDE DI QUESTO FILE: `oracolo/stock.map` FINISCE a
 * `_einittext`.
 *
 *   $ tail -1 oracolo/stock.map
 *   ffffff80093a8518 T _einittext
 *
 * Tutto cio' che sta oltre -- la `.exit.text` -- e' invisibile a
 * `clusterdriver.py`, `cluster_finale.py`, `classificasimboli.py`,
 * `disassembla.py` e `misurablocco.py`, perche' tutti partono dalla mappa.
 * La frase «non e' deciso, e non si aggiunge» era prudente nella forma e
 * sbagliata nella sostanza: non avevo guardato, e non l'avevo scritto.
 *
 * La funzione sta a 0xffffff80093acac4, 52 byte, e si legge con objdump
 * dando gli estremi a mano:
 *
 *   ffffff80093acac4:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
 *   ffffff80093acac8:	910003fd 	mov	x29, sp
 *   ffffff80093acacc:	f0fff480 	adrp	x0, ffffff800923f000
 *   ffffff80093acad0:	f0fff481 	adrp	x1, ffffff800923f000
 *   ffffff80093acad4:	911ee400 	add	x0, x0, #0x7b9
 *   ffffff80093acad8:	911fb421 	add	x1, x1, #0x7ed
 *   ffffff80093acadc:	52803822 	mov	w2, #0x1c1                 	// #449
 *   ffffff80093acae0:	97b61a7d 	bl	ffffff80081334d4 <printk>
 *   ffffff80093acae4:	f0002ec0 	adrp	x0, ffffff8009987000
 *   ffffff80093acae8:	910ce000 	add	x0, x0, #0x338
 *   ffffff80093acaec:	97da926f 	bl	ffffff8008a514a8 <tpd_driver_remove>
 *   ffffff80093acaf0:	a8c17bfd 	ldp	x29, x30, [sp],#16
 *   ffffff80093acaf4:	d65f03c0 	ret
 *
 * e 0xffffff8009987338 e' la `tpd_driver_t` di ILITEK, la stessa che
 * `ilitek_plat_dev_init` passa a `tpd_driver_add`.
 *
 * **Il gruppo C e' quindi 15 funzioni, non 14.**
 */
module_init(ilitek_plat_dev_init);

/*
 * ===========================================================================
 * 16) ilitek_plat_dev_exit -- 0xffffff80093acac4, 52 byte, `.exit.text`
 * fabbrica: riga 449
 * ===========================================================================
 */
static void __exit ilitek_plat_dev_exit(void)
{
	/*
	 * "\x016ILITEK: (%s, %d): ilitek driver has been removed\n"@0xffffff800923f7b9,
	 * __func__ = "ilitek_plat_dev_exit"@0xffffff800923f7ed,
	 * __LINE__ = 449 ("52803822 mov"@0xffffff80093acadc).
	 */
	ILI_INFO("ilitek driver has been removed\n");

	/*
	 * "910ce000 add"@0xffffff80093acae8 -> 0xffffff8009987338, la stessa
	 * `tpd_driver_t` che `ilitek_plat_dev_init` registra.
	 * "97da926f bl"@0xffffff80093acaec
	 */
	tpd_driver_remove(&ilitek_device_driver);
}

module_exit(ilitek_plat_dev_exit);
