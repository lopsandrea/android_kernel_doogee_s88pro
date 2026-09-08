// SPDX-License-Identifier: GPL-2.0
/*
 * sf_hw.c -- la parte hardware: GPIO, pinctrl, regolatore, clock SPI, reset.
 *
 * Lettore di impronte SPI Sunwave/TrustKernel (WTK_SUNWAVE_TRUSTKERNEL_TEE)
 * del Doogee S88 Pro, ricostruito leggendo il kernel di fabbrica
 * disassemblato. Ogni costante ha la sua riga di disassemblato citata qui
 * accanto nella forma "testo"@0xINDIRIZZO: se un valore non ce l'ha, non e'
 * stato letto ed e' un difetto.
 *
 * DUE UNITA' DI TRADUZIONE, e non e' una scelta di stile. Il lotto 3 aveva
 * scritto le 18 funzioni in un file solo, e quattro dei cinque scarti di
 * dimensione avevano quella causa. La prova che di fabbrica sono due file
 * non viene dai numeri:
 *
 *   - i numeri di riga che le printk portano come immediati si SOVRAPPONGONO
 *     (sf_ctl_device_power stampa 236, 239, 247, 262; sf_ctl_report_key_event
 *     stampa 259 e 308), il che in un file solo e' impossibile;
 *   - i due tag di log NON si spartiscono i due intervalli di riga: si
 *     SOVRAPPONGONO, quattro volte fra i tag (259..262, 241, 327..363,
 *     374..444) e zero volte dentro ciascun tag -- gli intervalli di
 *     "sunwave-sf_hw-%d" (162..196, 236..262, 322..363, 374..509, 517..531,
 *     537..550) e quelli di "sunwave-sf_ctl-%d" (233, 241, 259..308,
 *     327..444, 653..1196, 1001..1202, 1209..1264) sono ciascuno disgiunto
 *     e crescente per proprio conto. E' proprio questa sovrapposizione FRA
 *     tag -- non una spartizione -- a provare i due file: la partizione per
 *     tag di log e' esattamente quella che rende disgiunti tutti gli
 *     intervalli di riga (corretto dalla revisione indipendente, difetto
 *     D5: "si spartiscono" affermava il contrario del fatto su cui la tesi
 *     poggia);
 *   - le uniche due funzioni con linkage globale del blocco (`T` in
 *     oracolo/stock.map: sf_platform_init a 0xffffff8008a83994,
 *     sf_platform_exit a 0xffffff8008a8405c) sono esattamente quelle chiamate
 *     attraverso il confine fra i due tag. Le altre sedici sono `t`, e le
 *     cinque statiche di sf_hw prendono il proprio indirizzo dentro
 *     sf_platform_init (adrp+add di init_gpio_pins/power/spi_clock_enable/
 *     reset/free_gpio), non dentro sf_probe.
 *
 * Precisazione (revisione indipendente, HANDOFF SS4 "non affermare piu' di
 * quanto l'evidenza permetta): i soli numeri di riga provano che il
 * sorgente di fabbrica sta in PIU' DI UN FILE, non che sono due UNITA' DI
 * TRADUZIONE. Un .c che ne #include un altro darebbe la stessa
 * sovrapposizione, perche' __LINE__ segue il file fisico anche sotto
 * #include. A distinguere sono gli altri due fatti sopra: il linkage (le
 * due sole funzioni `T` di frontiera) e la mancata incorporazione, sotto.
 * Con tutte e tre le prove insieme l'inferenza a due unita' di traduzione
 * e' solida; con la sola prima non lo sarebbe.
 *
 * Conseguenza misurabile: di fabbrica sf_probe e sf_remove NON incorporano
 * sf_platform_init/sf_platform_exit, perche' stanno in un'altra unita' di
 * traduzione e non c'e' LTO -- sette `bl` vere nel blocco (una a
 * sf_platform_init, sei a sf_platform_exit), mai una copia incorporata. In
 * un file solo clang le incorpora, e il lotto 3 misurava sf_probe 2340
 * contro 2048 e sf_remove 216 contro 172.
 *
 * I numeri di riga citati accanto a ogni printk sono quelli del binario, non
 * di questi file: qui non coincidono, e non e' un difetto.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/spi/spi.h>

/*
 * Dichiarata `extern` e non da un header: nel kernel di fabbrica queste due
 * funzioni sono globali (`ffffff8008991ce8 T mt_spi_enable_master_clk`,
 * `ffffff8008991cb4 T mt_spi_disable_master_clk` in oracolo/stock.map) ma
 * l'albero ALPS non le esporta in nessun header -- l'altro driver di
 * impronte dell'albero fa la stessa cosa
 * (drivers/input/fingerprint/goodix/gf_spi_tee.h:296).
 */
extern void mt_spi_enable_master_clk(struct spi_device *spidev);
extern void mt_spi_disable_master_clk(struct spi_device *spidev);

#include "sunwave.h"

static int sf_ctl_device_init_gpio_pins(struct sf_ctl_device *ctl);
static int sf_ctl_device_power(bool on);
static int sf_spi_clock_enable(bool on);
static int sf_ctl_device_reset(void);
static int sf_ctl_device_free_gpio(struct sf_ctl_device *ctl);

/*
 * Puntatore globale alla struttura sopra, a 0xffffff800a100c90 (.bss).
 * Lo popola sf_ctl_device_init_gpio_pins ("f9064900  str x0, [x8,#3216]"
 * con x8 = adrp 0xffffff800a100000, cioe' 0xa100c90); lo rileggono
 * sf_ctl_device_power e sf_spi_clock_enable con lo stesso spiazzamento
 * ("f9464908  ldr x8, [x8,#3216]" in entrambe).
 */
static struct sf_ctl_device *g_ctl_dev;

/*
 * Le variabili statiche del blocco pinctrl, tutte sulla stessa pagina
 * 0xffffff800a100000 e tutte scritte da sf_ctl_device_init_gpio_pins
 * con gli spiazzamenti citati accanto.
 *
 *   3224 (0xc98) g_pinctrl        -- "f9064d00 str x0, [x8,#3224]" dopo devm_pinctrl_get
 *   3232 (0xca0) g_clk_enabled    -- un byte: "39728268 ldrb w8, [x19,#3232]"
 *   3240 (0xca8) g_state_rst_low  -- "f9065500 str x0, [x8,#3240]" dopo il lookup di
 *                                    "state_spi_rst_low"@0x9250fa2
 *   3248 (0xcb0) g_state_rst_high -- "f9065900 str x0, [x8,#3248]", "state_spi_rst_high"@0x9250fb4
 *   3256 (0xcb8) g_state_eint_init -- "f9065d00 str x0, [x8,#3256]", "state_spi_eint_init"@0x9250fc7
 *   3264 (0xcc0) g_state_cs_init  -- "f9066100 str x0, [x8,#3264]", "state_spi_cs_init"@0x9250fdb
 *   3272 (0xcc8) g_state_ck_init  -- "f9066500 str x0, [x8,#3272]", "state_spi_ck_init"@0x9250fed
 *   3280 (0xcd0) g_state_miso_init -- "f9066900 str x0, [x8,#3280]", "state_spi_miso_init"@0x9250fff
 *   3288 (0xcd8) g_state_mosi_init -- "f9066d00 str x0, [x8,#3288]", "state_spi_mosi_init"@0x9251013
 * I due stati di reset sono anche letti da sf_ctl_device_reset
 * ("f9465a81 ldr x1, [x20,#3248]" alto, "f9465501 ldr x1, [x8,#3240]" basso).
 */
static struct pinctrl *g_pinctrl;
static bool g_clk_enabled;
static struct pinctrl_state *g_state_rst_low;
static struct pinctrl_state *g_state_rst_high;
static struct pinctrl_state *g_state_eint_init;
static struct pinctrl_state *g_state_cs_init;
static struct pinctrl_state *g_state_ck_init;
static struct pinctrl_state *g_state_miso_init;
static struct pinctrl_state *g_state_mosi_init;

/*
 * La variabile della guardia di sf_ctl_driver_init sta in QUESTA unita' di
 * traduzione, non in sf_ctl.c che la usa, e le ragioni sono due, entrambe
 * lette dal binario.
 *
 * La prima e' la posizione in .bss: di fabbrica sta a 0xa100ce0, cioe' DOPO
 * i sette pinctrl_state (0xa100ca8..0xa100cd8) che appartengono a questo
 * file, mentre gli oggetti di sf_ctl.c (g_tee_version 0xa100b0c,
 * g_driver_version 0xa100c6c) stanno tutti PRIMA di 0xa100c90. Una variabile
 * di sf_ctl.c non potrebbe cadere fra i due gruppi.
 *
 * La seconda e' come sf_ctl_driver_init la legge:
 *   "b94ce108  ldr w8, [x8,#3296]" + "7100051f  cmp w8, #0x1"
 * cioe' una lettura a 32 bit su un intero. Dichiarandola `static` in un file
 * che la scrive con un solo valore, GlobalOpt di LLVM la restringe a un byte
 * e il confronto diventa "ldrb" + "tbz w8, #0" -- verificato: nel lotto 3,
 * con la variabile statica, il nostro .o le dava 1 byte e la funzione
 * misurava 212 invece di 216. Con linkage esterno l'ottimizzazione non e'
 * lecita, ed e' il tipo di accesso che la fabbrica ha.
 */
int g_probe_done;

/*
 * Posizione 11. sf_platform_init(struct sf_ctl_device *ctl): scrive la
 * tabella dei cinque puntatori a funzione. E' globale (`T sf_platform_init`
 * in oracolo/stock.map, riga 38294), come sf_platform_exit.
 *
 *   "sunwave-sf_hw-%d: %s(..) enter.\n"@0x9250c69            riga 517
 *   "sf_platform_init"@0x9250c8c                             (funcname)
 *   "sunwave-sf_hw-%d: %s() ctl_dev is NULL.\n"@0x9250c9d    riga 527
 *   "sunwave-sf_hw-%d: %s(..) leave.\n"@0x9250cc8            riga 531
 *
 * I cinque indirizzi vengono da adrp x8..x12 sulla pagina
 * 0xffffff8008a83000 piu' i cinque add:
 *   x8  #0xa40 -> 0xffffff8008a83a40  sf_ctl_device_init_gpio_pins
 *   x9  #0xe18 -> 0xffffff8008a83e18  sf_ctl_device_power
 *   x10 #0xec4 -> 0xffffff8008a83ec4  sf_spi_clock_enable
 *   x11 #0xf2c -> 0xffffff8008a83f2c  sf_ctl_device_reset
 *   x12 #0xfd8 -> 0xffffff8008a83fd8  sf_ctl_device_free_gpio
 * e i tre store che li piazzano:
 *   "a90a2a89  stp x9, x10, [x20,#160]"   power(160), spi_clock_enable(168)
 *   "f9005a8b  str x11, [x20,#176]"       reset(176)
 *   "a9093288  stp x8, x12, [x20,#144]"   init_gpio_pins(144), free_gpio(152)
 *
 * L'esito e' "2a1f03f3  mov w19, wzr" nel ramo buono e
 * "12800013  mov w19, #0xffffffff" (-1, non un -Exxx) in quello nullo.
 */
int sf_platform_init(struct sf_ctl_device *ctl)
{
	int err = 0;

	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) enter.\n", 517, "sf_platform_init");
	if (ctl) {
		ctl->init_gpio_pins = sf_ctl_device_init_gpio_pins;
		ctl->free_gpio = sf_ctl_device_free_gpio;
		ctl->power = sf_ctl_device_power;
		ctl->spi_clock_enable = sf_spi_clock_enable;
		ctl->reset = sf_ctl_device_reset;
	} else {
		printk(KERN_ERR "sunwave-sf_hw-%d: %s() ctl_dev is NULL.\n", 527, "sf_platform_init");
		err = -1;
	}
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) leave.\n", 531, "sf_platform_init");
	return err;
}

/*
 * Posizione 12. sf_ctl_device_init_gpio_pins(struct sf_ctl_device *ctl):
 * la piu' lunga del driver (984 byte). Letterali, tutti verificati byte per
 * byte all'indirizzo citato:
 *   "sunwave-sf_hw-%d: %s(..) enter.\n"@0x9250c69                     riga 374
 *   "sf_ctl_device_init_gpio_pins"@0x9250cfc                          (funcname)
 *   "goodix,goodix-fp"@0x9250d19
 *   "sunwave-sf_hw-%d: of_find_compatible_node(..) failed.\n"@0x9250d2a  riga 378
 *   "mediatek,goodix-fp"@0x9250d63
 *   "sunwave-sf_hw-%d: Cannot find eint node in dts file!!!!!!.\n"@0x9250d76  riga 386
 *   "sunwave-sf_hw-%d: irq number is %d.\n"@0x9250db4                 riga 391
 *   "sunwave-sf_hw-%d: pinctrl_get(..) failed.\n"@0x9250ddb           riga 397
 *   "sunwave-sf_hw-%d: Lookup pinctrl state NOW!!!.\n"@0x9250e08      riga 401
 *   "sunwave-sf_hw-%d: can't find '%s' pinctrl_state.\n"@0x9250e3a    riga 411
 *   "sunwave-sf_hw-%d: %s() failed.\n"@0x9250e6e                      riga 418
 *   "sunwave-sf_hw-%d: %s() pinctrl_select_state(%s) failed.\n"@0x9250e90  riga 426
 *   "sunwave-sf_hw-%d: pinctrl_select_state(%s) ok.\n"@0x9250ecb      riga 430
 *   "vfingerprint"@0x9250efd
 *   "sunwave-sf_hw-%d: Regulator get failed vdd err = %d\n"@0x9250f0a riga 493
 *   "sunwave-sf_hw-%d: Regulator set_vtg failed vdd err = %d\n"@0x9250f41 riga 502
 *   "sunwave-sf_hw-%d: %s(..) ok! exit.\n"@0x9250f7c                  riga 509
 * piu' i sette nomi di stato pinctrl, citati sulle statiche qui sopra.
 *
 * Le due tensioni sono "52973001 mov w1, #0xb980" + "72a00541 movk w1, #0x2a, lsl #16"
 * = 0x2ab980 = 2800000 (uguale per w2, minimo e massimo).
 *
 * &pdev->dev e' "91004280 add x0, x20, #0x10": in questo sublevel struct
 * platform_device ha name(0), id(8), id_auto(12), dev(16).
 *
 * IS_ERR e' "b13ffc1f cmn x0, #0xfff" + "54000103 b.cc": il ramo di errore
 * e' quello che NON salta.
 *
 * ATTENZIONE, il limite di questa ricostruzione: il binario determina il
 * FLUSSO, non la forma del sorgente. I sette blocchi di errore dei
 * pinctrl_lookup_state sono fusi in un blocco solo, e cosi' i quattro dei
 * pinctrl_select_state; qui sono resi con due goto verso un blocco comune,
 * che e' la trascrizione piu' diretta del flusso osservato, ma un sorgente
 * con if/else annidati che il compilatore avesse fuso darebbe lo stesso
 * binario. La differenza non e' osservabile, e non viene affermata.
 *
 * Da leggere e non "correggere": dopo un pinctrl_select_state fallito la
 * fabbrica NON esce -- stampa e prosegue con il regolatore, e l'errore
 * resta in "err" fino al ritorno ("2a1503e0 mov w0, w21" in coda). Allo
 * stesso modo, se of_find_device_by_node restituisce NULL la fabbrica salta
 * devm_pinctrl_get ma poi passa comunque pdev+0x10 a regulator_get.
 */
static int sf_ctl_device_init_gpio_pins(struct sf_ctl_device *ctl)
{
	struct device_node *node;
	struct platform_device *pdev;
	const char *name;
	const char *sel;
	int err;

	g_ctl_dev = ctl;
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) enter.\n", 374, "sf_ctl_device_init_gpio_pins");

	ctl->spi->dev.of_node = of_find_compatible_node(NULL, NULL, "goodix,goodix-fp");
	if (!ctl->spi->dev.of_node) {
		printk(KERN_ERR "sunwave-sf_hw-%d: of_find_compatible_node(..) failed.\n", 378);
		return -ENODEV;
	}

	node = of_find_compatible_node(NULL, NULL, "mediatek,goodix-fp");
	if (!node)
		printk(KERN_INFO "sunwave-sf_hw-%d: Cannot find eint node in dts file!!!!!!.\n", 386);

	ctl->irq = irq_of_parse_and_map(node, 0);
	printk(KERN_INFO "sunwave-sf_hw-%d: irq number is %d.\n", 391, ctl->irq);

	pdev = of_find_device_by_node(node);
	if (pdev) {
		g_pinctrl = devm_pinctrl_get(&pdev->dev);
		if (!g_pinctrl) {
			printk(KERN_ERR "sunwave-sf_hw-%d: pinctrl_get(..) failed.\n", 397);
			return -ENODEV;
		}
	}

	printk(KERN_ERR "sunwave-sf_hw-%d: Lookup pinctrl state NOW!!!.\n", 401);

	name = "state_spi_rst_low";
	g_state_rst_low = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_rst_low)
		goto lookup_failed;
	name = "state_spi_rst_high";
	g_state_rst_high = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_rst_high)
		goto lookup_failed;
	name = "state_spi_eint_init";
	g_state_eint_init = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_eint_init)
		goto lookup_failed;
	name = "state_spi_cs_init";
	g_state_cs_init = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_cs_init)
		goto lookup_failed;
	name = "state_spi_ck_init";
	g_state_ck_init = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_ck_init)
		goto lookup_failed;
	name = "state_spi_miso_init";
	g_state_miso_init = pinctrl_lookup_state(g_pinctrl, name);
	if (!g_state_miso_init)
		goto lookup_failed;
	name = "state_spi_mosi_init";
	g_state_mosi_init = pinctrl_lookup_state(g_pinctrl, name);
	if (g_state_mosi_init)
		goto lookup_done;
lookup_failed:
	printk(KERN_ERR "sunwave-sf_hw-%d: can't find '%s' pinctrl_state.\n", 411, name);
	printk(KERN_ERR "sunwave-sf_hw-%d: %s() failed.\n", 418, "sf_ctl_device_init_gpio_pins");
lookup_done:

	sel = "state_spi_cs_init";
	err = pinctrl_select_state(g_pinctrl, g_state_cs_init);
	if (err)
		goto select_failed;
	printk(KERN_INFO "sunwave-sf_hw-%d: pinctrl_select_state(%s) ok.\n", 430, sel);
	sel = "state_spi_ck_init";
	err = pinctrl_select_state(g_pinctrl, g_state_ck_init);
	if (err)
		goto select_failed;
	printk(KERN_INFO "sunwave-sf_hw-%d: pinctrl_select_state(%s) ok.\n", 430, sel);
	sel = "state_spi_miso_init";
	err = pinctrl_select_state(g_pinctrl, g_state_miso_init);
	if (err)
		goto select_failed;
	printk(KERN_INFO "sunwave-sf_hw-%d: pinctrl_select_state(%s) ok.\n", 430, sel);
	sel = "state_spi_mosi_init";
	err = pinctrl_select_state(g_pinctrl, g_state_mosi_init);
	if (err)
		goto select_failed;
	printk(KERN_INFO "sunwave-sf_hw-%d: pinctrl_select_state(%s) ok.\n", 430, sel);
	goto select_done;
select_failed:
	printk(KERN_ERR "sunwave-sf_hw-%d: %s() pinctrl_select_state(%s) failed.\n",
	       426, "sf_ctl_device_init_gpio_pins", sel);
select_done:

	ctl->vdd_reg = regulator_get(&pdev->dev, "vfingerprint");
	if (IS_ERR(ctl->vdd_reg)) {
		err = PTR_ERR(ctl->vdd_reg);
		printk(KERN_ERR "sunwave-sf_hw-%d: Regulator get failed vdd err = %d\n", 493, err);
		return err;
	}
	if (regulator_count_voltages(ctl->vdd_reg) > 0) {
		err = regulator_set_voltage(ctl->vdd_reg, 2800000, 2800000);
		if (err) {
			printk(KERN_ERR "sunwave-sf_hw-%d: Regulator set_vtg failed vdd err = %d\n", 502, err);
			return err;
		}
	}

	sf_ctl_device_power(true);
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) ok! exit.\n", 509, "sf_ctl_device_init_gpio_pins");
	return err;
}

/*
 * Posizione 13. sf_ctl_device_power(bool on): accende/spegne il regolatore
 * di alimentazione. Tag di log "sunwave-sf_hw-%d: " (KERN_INFO="\x016",
 * KERN_ERR="\x013") + numero di riga della sorgente di fabbrica, letti
 * dagli immediati mov w1,#N accanto a ogni bl printk:
 *   "sunwave-sf_hw-%d: %s(..) enter.\n"@0x9250c69   riga 236
 *   "sf_ctl_device_power_by_regulator"@0x9251027    (funcname; vedi sotto)
 *   "sunwave-sf_hw-%d: ctl_dev->vdd_reg is NULL.\n"@0x9251048   riga 239
 *   "sunwave-sf_hw-%d: Regulator vdd enable failed err = %d\n"@0x9251077   riga 247
 *   "sunwave-sf_hw-%d: %s(..) leave.\n"@0x9250cc8   riga 262
 *
 * Il nome usato nell'argomento %s ("..._by_regulator") non e' il nome del
 * simbolo (sf_ctl_device_power): e' un letterale distinto, verificato
 * byte per byte a 0x9251027. Va scritto cosi' com'e', non sostituito con
 * __func__, altrimenti il binario prodotto non coinciderebbe.
 *
 * IL TIPO DEL PARAMETRO, nella formulazione che regge (rilievo R3 della
 * revisione del lotto 2, che aveva mostrato come il lotto 2 lo puntasse nel
 * posto sbagliato). L'evidenza sta nel CHIAMATO, non nel chiamante:
 *   ffffff8008a83e50: 36000253  tbz  w19, #0, ffffff8008a83e98
 * tbz prova UN SOLO BIT di w19, non l'intero registro; con "on" di tipo int
 * lo stesso "if (on)" produce cbz, verificato ricompilando col compilatore
 * del progetto (clang-r383902). Quindi il binario determina IL TEST (bit 0),
 * non il tipo C: `bool on` con `if (on)` e `int on` con `if (on & 1)` danno
 * entrambi questo tbz -- la revisione del lotto 1 aveva verificato lei
 * stessa la seconda forma -- e restano sorgenti osservazionalmente
 * equivalenti. `bool` e' la scelta fatta, e da' 172/172, ma non e' un tipo
 * dimostrato: non si puo' stabilire di piu' con questo binario.
 *
 * Il chiamante NON aggiunge niente a questa qualificazione. Il solo
 * chiamante diretto e' sf_ctl_device_init_gpio_pins, con
 *   ffffff8008a83dc8: 320003e0  orr  w0, wzr, #0x1
 *   ffffff8008a83dcc: 94000013  bl   ffffff8008a83e18 <sf_ctl_device_power>
 * e passare 1 e' compatibile sia con `bool true` sia con un int di cui e'
 * acceso il solo bit 0: NON esclude il campo di bit, che il lotto 2 diceva
 * di escludere. L'unica cosa che il chiamante dice e' che 1 e' l'unico
 * valore mai passato.
 *
 * Il puntatore ctl->power non e' invocato da nessuna delle 18 funzioni del
 * blocco: la tabella lo assegna (sf_platform_init) e lo azzera
 * (sf_platform_exit), e basta. Cio' vale per `power` e non per gli altri due
 * puntatori della tabella: sf_ctl_ioctl invoca ctl->spi_clock_enable
 * (spiazzamento 168, "#3224") e ctl->reset (176, "#3232") con 1 e con 0.
 */
static int sf_ctl_device_power(bool on)
{
	struct sf_ctl_device *ctl = g_ctl_dev;
	int ret;

	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) enter.\n", 236, "sf_ctl_device_power_by_regulator");
	if (!ctl->vdd_reg) {
		printk(KERN_ERR "sunwave-sf_hw-%d: ctl_dev->vdd_reg is NULL.\n", 239);
		return -ENODEV;
	}
	if (on) {
		ret = regulator_enable(ctl->vdd_reg);
		if (ret) {
			printk(KERN_ERR "sunwave-sf_hw-%d: Regulator vdd enable failed err = %d\n", 247, ret);
			return ret;
		}
		msleep(10);
	}
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) leave.\n", 262, "sf_ctl_device_power_by_regulator");
	return 0;
}

/*
 * Posizione 14. sf_spi_clock_enable(bool on): accende/spegne il clock del
 * master SPI. Nessuna printk, nessun letterale.
 *
 * Il parametro e' bool per la stessa ragione di sf_ctl_device_power, con la
 * stessa qualificazione:
 *   "360000e0  tbz w0, #0, ...+0x34"    prova un solo bit di w0
 * La variabile di stato e' un byte, non una parola:
 *   "39728268  ldrb w8, [x19,#3232]"    lettura  (0xa100ca0)
 *   "39328268  strb w8, [x19,#3232]"    scrittura
 *
 * La forma delle due condizioni si legge dalle prove nell'ordine in cui il
 * binario le fa:
 *   "37000108  tbnz w8, #0, ...+0x34"   se gia' acceso, salta il primo ramo
 *   "360000e0  tbz  w0, #0, ...+0x34"   se on == 0, salta il primo ramo
 *   -> primo ramo: !acceso && on -> mt_spi_enable_master_clk, stato = 1
 *   "52000108  eor w8, w8, #0x1"
 *   "2a000108  orr w8, w8, w0"
 *   "370000e8  tbnz w8, #0, ...+0x58"   esce se (!acceso) || on
 *   -> secondo ramo: acceso && !on -> mt_spi_disable_master_clk, stato = 0
 * cioe' l'ordine degli operandi e' "stato" prima e "on" dopo in entrambe le
 * congiunzioni.
 *
 * L'argomento delle due mt_spi_* e' "f9464908 ldr x8, [x8,#3216]" +
 * "f940cd00 ldr x0, [x8,#408]", cioe' g_ctl_dev->spi.
 *
 * Il ritorno e' sempre "2a1f03e0  mov w0, wzr".
 *
 * ATTENZIONE A COME SI MISURA QUESTA FUNZIONE (rilievo R5 della revisione
 * del lotto 2). tools/misuraisolata.py compila il file fuori dall'albero con
 * una radice sintetica, e quella radice CAMBIA UN'ISTRUZIONE proprio qui: la
 * prima condizione diventa "35000108 cbnz w8" invece del "37000108 tbnz
 * w8,#0" che ha la fabbrica, perche' la radice rende i globali mutabili e
 * toglie a clang l'informazione di intervallo sul bool. Il build vero
 * coincide con la fabbrica su quell'istruzione. La taglia non ne risente
 * (100 in entrambi i casi) e nessuna conclusione dipende da questo, ma il
 * TESTO istruzione per istruzione prodotto fuori dall'albero non va letto
 * come se fosse il driver: per quello serve --elf sul build vero.
 */
static int sf_spi_clock_enable(bool on)
{
	if (!g_clk_enabled && on) {
		mt_spi_enable_master_clk(g_ctl_dev->spi);
		g_clk_enabled = true;
	} else if (g_clk_enabled && !on) {
		mt_spi_disable_master_clk(g_ctl_dev->spi);
		g_clk_enabled = false;
	}
	return 0;
}

/*
 * Posizione 15. sf_ctl_device_reset(void): impulso di reset via pinctrl
 * (alto -> basso -> alto), non un GPIO diretto. Sequenza verificata dagli
 * argomenti di pinctrl_select_state nel disassemblato:
 * select(rst_high); msleep(1); select(rst_low); msleep(10);
 * ret = select(rst_high).
 *   "sunwave-sf_hw-%d: %s(..) enter.\n"@0x9250c69   riga 162
 *   "sf_ctl_device_reset"@0x92510b1   (funcname; qui SI coincide col nome del simbolo)
 *   "sunwave-sf_hw-%d: sf_pinctrl is NULL.\n"@0x92510c5   riga 173
 *   "sunwave-sf_hw-%d: %s(..) leave.\n"@0x9250cc8   riga 196
 */
static int sf_ctl_device_reset(void)
{
	int ret;

	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) enter.\n", 162, "sf_ctl_device_reset");
	if (!g_pinctrl) {
		printk(KERN_ERR "sunwave-sf_hw-%d: sf_pinctrl is NULL.\n", 173);
		return -1;
	}
	pinctrl_select_state(g_pinctrl, g_state_rst_high);
	msleep(1);
	pinctrl_select_state(g_pinctrl, g_state_rst_low);
	msleep(10);
	ret = pinctrl_select_state(g_pinctrl, g_state_rst_high);
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) leave.\n", 196, "sf_ctl_device_reset");
	return ret;
}

/*
 * Posizione 16. sf_ctl_device_free_gpio(struct sf_ctl_device *ctl): libera
 * pinctrl e regolatore, azzera il device_node cache in ctl->spi.
 *   "sunwave-sf_hw-%d: %s(..) enter, free resource.\n"@0x92510ee   riga 322
 *   "sf_ctl_device_free_gpio"@0x9251120   (funcname)
 *   "sunwave-sf_hw-%d: %s(..) leave.\n"@0x9250cc8   riga 363
 */
static int sf_ctl_device_free_gpio(struct sf_ctl_device *ctl)
{
	printk(KERN_ERR "sunwave-sf_hw-%d: %s(..) enter, free resource.\n", 322, "sf_ctl_device_free_gpio");
	if (ctl->spi->dev.of_node)
		ctl->spi->dev.of_node = NULL;
	if (g_pinctrl) {
		pinctrl_put(g_pinctrl);
		g_pinctrl = NULL;
	}
	if (ctl->vdd_reg) {
		regulator_put(ctl->vdd_reg);
		ctl->vdd_reg = NULL;
	}
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) leave.\n", 363, "sf_ctl_device_free_gpio");
	return 0;
}

/*
 * Posizione 17. sf_platform_exit(struct sf_ctl_device *ctl): azzera la
 * tabella dei cinque puntatori a funzione (spiazzamenti 144..183, tre store
 * che coprono esattamente reset/spi_clock_enable/free_gpio/power/init_gpio_pins).
 * Nessun `mov w0,...` nel disassemblato prima del ret: la funzione e' void.
 *   "sunwave-sf_hw-%d: %s(..) enter.\n"@0x9250c69   riga 537
 *   "sf_platform_exit"@0x9250ceb   (funcname)
 *   "sunwave-sf_hw-%d: %s() ctl_dev is NULL.\n"@0x9250c9d   riga 547
 *   "sunwave-sf_hw-%d: %s(..) leave.\n"@0x9250cc8   riga 550
 *
 * SEI CHIAMANTI IN TUTTO nel blocco (rilievo R1): cinque in sf_probe e una
 * in sf_remove (0xa829e8, 0xa82d00, 0xa82f10, 0xa82f68, 0xa83084, 0xa83184).
 * Il messaggio di commit del lotto 2 diceva "cinque chiamanti, quattro in
 * sf_probe": erano cinque in sf_probe e sei in tutto.
 *
 * L'ultima, 0xa83184, e' quella corretta dalla revisione indipendente
 * (difetto D1): il rilievo R1 stesso l'aveva data come 0xa83140, che nel
 * binario e' l'"add x19, x19, #0xbf0" due righe prima, non questa bl.
 */
void sf_platform_exit(struct sf_ctl_device *ctl)
{
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) enter.\n", 537, "sf_platform_exit");
	if (ctl) {
		ctl->spi_clock_enable = NULL;
		ctl->reset = NULL;
		ctl->free_gpio = NULL;
		ctl->power = NULL;
		ctl->init_gpio_pins = NULL;
	} else {
		printk(KERN_ERR "sunwave-sf_hw-%d: %s() ctl_dev is NULL.\n", 547, "sf_platform_exit");
	}
	printk(KERN_INFO "sunwave-sf_hw-%d: %s(..) leave.\n", 550, "sf_platform_exit");
}
