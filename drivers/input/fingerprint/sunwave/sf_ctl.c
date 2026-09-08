// SPDX-License-Identifier: GPL-2.0
/*
 * sf_ctl.c -- la parte di controllo: probe/remove, il nodo misc, ioctl, sysfs, input.
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
#include <linux/fb.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/pm_wakeup.h>
#include <linux/spi/spi.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>

/*
 * Stessa situazione: `ffffff80087adb34 T tee_spi_transfer` nell'oracolo, il
 * sorgente c'e' nell'albero
 * (drivers/misc/mediatek/tkcore/core/tee_fp.c:90, EXPORT_SYMBOL a riga 166)
 * ma nessun header di include/ lo dichiara.
 */
extern int tee_spi_transfer(void *conf, uint32_t conf_size,
			    void *inbuf, void *outbuf, uint32_t size);

#include "sunwave.h"

static long sf_ctl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static long sf_ctl_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static ssize_t sf_show_version(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t sunwave_chip_info_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t sunwave_version_show(struct device *dev, struct device_attribute *attr, char *buf);
static int sf_probe(struct spi_device *spi);
static int sf_remove(struct spi_device *spi);
static void sf_ctl_device_event(struct work_struct *ws);
static int sf_fb_notifier_callback(struct notifier_block *self,
				   unsigned long event, void *data);
static irqreturn_t sf_ctl_device_irq(int irq, void *dev_id);

/*
 * I tre attributi sysfs, letti da .rela.dyn con tools/relocazioni.py
 * --sysfs-group 0xffffff800998dde0 (l'indirizzo passato a sysfs_create_group
 * da sf_probe: "91378021  add x1, x1, #0xde0"):
 *
 *   0xffffff800998df68  nome=tee_version mode=0o644 show=sf_show_version         store=None
 *   0xffffff800998df88  nome=chip_info   mode=0o644 show=sunwave_chip_info_show  store=None
 *   0xffffff800998dfa8  nome=version     mode=0o644 show=sunwave_version_show    store=None
 *
 * mode 0644 con store nullo e' quello che ha la fabbrica, non un refuso da
 * "correggere" in 0444: e' letto come u16 dal campo `mode` di struct
 * attribute, non dedotto (relocazioni.py legge `mode` come umode_t, 16 bit).
 *
 * Il campo `name` dell'attribute_group (spiazzamento 0) NON e' relocato:
 * il gruppo non ha sottodirectory.
 *
 * Nessuno di questi oggetti e' `const`: stanno tutti nella fascia 0x998dxxx,
 * insieme a sf_ctl_dev e a sf_spi_conf che sono scritti a runtime, mentre la
 * .rodata di questo kernel sta a 0x92xxxxx (i letterali) e 0x8f7fxxx (le
 * tabelle di salto). Un oggetto `const` sarebbe finito li'.
 */
static DEVICE_ATTR(tee_version, 0644, sf_show_version, NULL);
static DEVICE_ATTR(chip_info, 0644, sunwave_chip_info_show, NULL);
static DEVICE_ATTR(version, 0644, sunwave_version_show, NULL);

static struct attribute *sf_ctl_attrs[] = {
	&dev_attr_tee_version.attr,
	&dev_attr_chip_info.attr,
	&dev_attr_version.attr,
	NULL,
};

static struct attribute_group sf_ctl_attr_group = {
	.attrs = sf_ctl_attrs,
};

/*
 * struct file_operations a 0xffffff800998de08 (l'indirizzo del campo `fops`
 * di sf_ctl_dev, spiazzamento 16). Nei 128 byte letti dal PT_LOAD ci sono
 * esattamente DUE rilocazioni, e nient'altro:
 *   +72 (0x48) -> 0xffffff8008a832a8 <sf_ctl_ioctl>
 *   +80 (0x50) -> 0xffffff8008a8379c <sf_ctl_compat_ioctl>
 * che sono unlocked_ioctl e compat_ioctl nell'ABI di questo sublevel.
 *
 * Il campo `owner` (spiazzamento 0) e' NULLO e senza rilocazione: e'
 * THIS_MODULE compilato dentro il kernel, dove la macro vale
 * ((struct module *)0). Scriverlo o ometterlo da lo stesso binario; qui e'
 * scritto perche' e' cio' che rende NULL quel campo in un builtin.
 */
static struct file_operations sf_ctl_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = sf_ctl_ioctl,
	.compat_ioctl = sf_ctl_compat_ioctl,
};

/*
 * L'istanza statica del driver, a 0xffffff800998dbf0.
 *
 * L'INIZIALIZZATORE (rilievo R7 della revisione del lotto 2, che lo aveva
 * letto per intero e osservato che due campi su tre non dipendevano da
 * codice non scritto). Nei 416 byte on-disk c'e' UN SOLO byte non nullo, il
 * primo, e due rilocazioni:
 *   +  0: ff 00 00 00 ...   -> minor = 255 = MISC_DYNAMIC_MINOR
 *   +  8: addend 0xffffff800925065b -> "sunwave_fp"@0x925065b
 *   + 16: addend 0xffffff800998de08 -> &sf_ctl_fops
 * e nient'altro. E' per questo che di fabbrica l'oggetto sta in una sezione
 * con contenuto e non in .bss: adesso ce lo sta anche da noi.
 */
static struct sf_ctl_device sf_ctl_dev = {
	.miscdev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "sunwave_fp",
		.fops = &sf_ctl_fops,
	},
};

/*
 * La struttura "versione", a 0xffffff800a100b0c, lunga 0x160 = 352 byte.
 *
 * La lunghezza non e' dedotta: sf_ctl_ioctl la copia da e verso lo spazio
 * utente con "52802c02  mov w2, #0x160" e base "912c3000/912c3021 add
 * x0/x1, x0/x1, #0xb0c" -- 352 byte a partire da 0xa100b0c. E il
 * confine superiore e' confermato da un secondo lato: subito dopo, a
 * 0xa100c6c = 0xa100b0c + 0x160 esatto, comincia il buffer da 32 byte qui
 * sotto.
 *
 * I nove campi e le loro dimensioni vengono dagli spiazzamenti (da x20, che
 * punta a 0xa100b0c) delle nove sprintf di sf_show_version:
 * 0x0 0x20 0x60 0xa0 0xc0 0xe0 0x100 0x120 0x140, piu' la lunghezza totale
 * 0x160 che delimita l'ultimo campo.
 *
 * Le altre due sysfs "show" leggono la STESSA struttura, non strutture
 * proprie -- questo il lotto 1 lo aveva sbagliato, inventando due variabili
 * separate:
 *   - sunwave_chip_info_show usa base "912cb063  add x3, x3, #0xb2c" =
 *     0xa100b2c = 0xa100b0c + 0x20, cioe' &g_tee_version.ca;
 *   - sunwave_version_show usa "912fb042  add x2, x2, #0xbec" =
 *     0xa100bec = 0xa100b0c + 0xe0, cioe' &g_tee_version.driver.
 * Entrambi gli indirizzi cadono dentro i 352 byte della struttura.
 *
 * NOTA SUL NOME: il tipo di fabbrica si chiama con ogni probabilita'
 * sf_hw_info_t, non sf_tee_version -- i due messaggi di errore che
 * sf_ctl_ioctl stampa quando la copia di questi 352 byte fallisce sono
 * "sunwave-sf_ctl-%d: sf_hw_info_t copy_from_user(..) failed.\n"@0x9250709 e
 * "sunwave-sf_ctl-%d: sf_hw_info_t copy_to_user(..) failed.\n"@0x9250747.
 * Il nome scelto qui resta quello del lotto
 * 1 perche' cambiarlo non cambia un byte del binario; l'evidenza e'
 * registrata qui invece di essere taciuta.
 */
struct sf_tee_version {
	char solution[0x20];		/* 0x00 */
	char ca[0x60 - 0x20];		/* 0x20 */
	char ta[0xa0 - 0x60];		/* 0x60 */
	char alg[0xc0 - 0xa0];		/* 0xa0 */
	char nav[0xe0 - 0xc0];		/* 0xc0 */
	char driver[0x100 - 0xe0];	/* 0xe0 */
	char firmware[0x120 - 0x100];	/* 0x100 */
	char sensor[0x140 - 0x120];	/* 0x120 */
	char vendor[0x160 - 0x140];	/* 0x140 */
};

/*
 * L'ORDINE DI QUESTE DICHIARAZIONI E' L'ORDINE DI .bss DI FABBRICA, e adesso
 * si puo' riprodurre (rilievo R6 della revisione del lotto 2: il lotto 2
 * aveva g_tee_version in coda invece che in testa, e non diceva perche').
 * La ragione era il terzo oggetto che il lotto 2 non aveva: il buffer da 32
 * byte a 0xa100c6c, che sta FRA g_tee_version e g_ctl_dev. Scritto quello,
 * l'ordine torna:
 *
 *   0xa100b0c  g_tee_version      352 byte
 *   0xa100c6c  g_driver_version    32 byte  (0xa100b0c + 0x160)
 *   0xa100c90  g_ctl_dev                    (allineamento a 8 dopo 0xa100c8c)
 *   0xa100c98  g_pinctrl
 *   0xa100ca0  g_clk_enabled       1 byte
 *   0xa100ca8  g_state_rst_low
 *   0xa100cb0  g_state_rst_high
 *   0xa100cb8  g_state_eint_init
 *   0xa100cc0  g_state_cs_init
 *   0xa100cc8  g_state_ck_init
 *   0xa100cd0  g_state_miso_init
 *   0xa100cd8  g_state_mosi_init
 *   0xa100ce0  g_probe_done        4 byte
 */
static struct sf_tee_version g_tee_version;

/*
 * Buffer di 32 byte a 0xffffff800a100c6c. Lo scrivono DUE funzioni con lo
 * stesso letterale e la stessa lunghezza:
 *   sf_ctl_driver_init  "9131b294  add x20, x20, #0xc6c" +
 *                       "911f8c21  add x1, x1, #0x7e3" +
 *                       "321b03e2  orr w2, wzr, #0x20" + bl <strncpy> +
 *                       "39007e9f  strb wzr, [x20,#31]"
 *   sf_ctl_ioctl        "9131b000  add x0, x0, #0xc6c" + stesse tre righe +
 *                       "39007c1f  strb wzr, [x0,#31]"
 * con "v2.2.36-2018-11-14"@0x92507e3. sf_ctl_ioctl poi lo copia all'utente
 * con "321b03e2 orr w2, wzr, #0x20" + bl <__arch_copy_to_user>: 32 byte.
 */
static char g_driver_version[32];

/*
 * La configurazione SPI passata a tee_spi_transfer: 80 byte a
 * 0xffffff800998def8. Che siano 80 non e' dedotto -- e' il secondo argomento
 * di ogni chiamata, "52800a01  mov w1, #0x50", con
 * "913be294  add x20, x20, #0xef8" come primo.
 *
 * I VALORI sono letti dal PT_LOAD, venti u32 consecutivi:
 *   +0  0a000000 0a000000   10 10
 *   +8  0f0000000f000000    15 15
 *   +16 0a00000000000000    10  0
 *   +24 0000000000000000     0  0
 *   +32 0000000000000000     0  0
 *   +40 0100000001000000     1  1
 *   +48 0000000000000000     0  0
 *   +56 0000000000000000     0  0
 *   +64 0500000000000000     5  0
 *   +72 0000000000000000     0  0
 *
 * I NOMI DEI CAMPI NON SONO LETTI, e per questo non sono scritti. Nessun
 * header dell'albero ALPS definisce questa struttura: `struct mt_chip_conf`
 * e' dichiarata in "mtk_spi.h", che il driver goodix include solo sotto
 * #ifndef CONFIG_SPI_MT65XX e che in questo albero non esiste (il config di
 * fabbrica ha CONFIG_SPI_MT65XX=y, riga 2468). I primi sei valori
 * coinciderebbero con setuptime/holdtime/high_time/low_time/cs_idletime/
 * ulthgh_thrsh dell'inizializzatore `spi_ctrldata` di
 * drivers/input/fingerprint/goodix/gf_spi_tee.c:143-149, che ha 10/10/50/50/
 * 10/0 dove qui c'e' 10/10/15/15/10/0 -- ma il 5 allo spiazzamento 64
 * cadrebbe su `finish_intr`, che in quell'inizializzatore vale 1 ed e' un
 * enum a due valori. L'identificazione non e' quindi stabilita, e venti u32
 * con i valori letti sono cio' che il binario dice davvero.
 *
 * sf_read_sensor_id ne riscrive cinque a runtime, con gli indici che il
 * disassemblato da' come spiazzamenti da 0x998def8:
 *   "b21e0fe9  mov x9, #0x3c0000003c" + "f9000109  str x9, [x8]"
 *      con x8 = 0x998df00 = conf+8   -> [2] = [3] = 60
 *   "f9000d1f  str xzr, [x8,#24]"    -> conf+32   -> [8] = [9] = 0
 *   "b900311f  str wzr, [x8,#48]"    -> conf+56   -> [14] = 0
 */
static u32 sf_spi_conf[20] = {
	10, 10, 15, 15, 10, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 0, 0, 5, 0, 0, 0,
};

/*
 * of_match_table dello spi_driver, a 0xffffff800998d998 (rilocazione allo
 * spiazzamento 72 di sf_spi_driver). struct of_device_id e' 200 byte
 * (name[32] type[32] compatible[128] data), e le stringhe stanno INLINE:
 * non hanno rilocazione, si leggono dai byte al loro indirizzo, che e'
 * l'inizio della voce piu' 64 (lo spiazzamento del campo compatible).
 *   "mediatek,spi-fingerprint"@0x998d9d8   (voce 0, 0x998d998 + 64)
 *   "mediatek,goodix-fp"@0x998daa0         (voce 1, 0x998da60 + 64)
 *   "goodix,goodix-fp"@0x998db68           (voce 2, 0x998db28 + 64)
 * Queste due copie in .data sono oggetti DISTINTI dai letterali di .rodata
 * che sf_ctl_device_init_gpio_pins (sf_hw.c) passa a of_find_compatible_node
 * e che quel file cita ai propri indirizzi (0x9250d19 e 0x9250d63): stesso
 * testo, quattro indirizzi, due per unita' di traduzione.
 *
 * TRE VOCI, SENZA SENTINELLA, e non e' una svista di lettura: la terza voce
 * finisce a 0x998d998 + 3*200 = 0x998dbf0, che e' esattamente l'indirizzo di
 * sf_ctl_dev (i cui primi byte sono ff 00 00 00, cioe' minor = 255, non zeri).
 * Una quarta voce azzerata occuperebbe 200 byte che sono gia' occupati.
 * Riprodotto com'e' e non "corretto": il device di questo driver nasce da
 * spi_register_board_info e non ha of_node, quindi of_driver_match_device
 * esce prima di scorrere la tabella e la sentinella mancante non morde mai.
 */
static struct of_device_id sf_of_match[] = {
	{ .compatible = "mediatek,spi-fingerprint", },
	{ .compatible = "mediatek,goodix-fp", },
	{ .compatible = "goodix,goodix-fp", },
};

/*
 * Lo spi_driver a 0xffffff800998d900, letto da .rela.dyn:
 *   +  8 -> 0xffffff8008a828f0 <sf_probe>          (probe)
 *   + 16 -> 0xffffff8008a830f0 <sf_remove>         (remove)
 *   + 32 -> 0xffffff80092504b7 -> "sunwave-fp"@0x92504b7  (driver.name)
 *          (il testo compare due volte nel binario -- qui in .rodata e come
 *          modalias dentro sf_spi_board_info -- ma la citazione non e' debole
 *          per questo: cio' che la ancora non e' il testo, e' la rilocazione,
 *          che punta a questo indirizzo e a nessun altro)
 *   + 40 -> 0xffffff800995ae20                     (driver.bus)
 *   + 72 -> 0xffffff800998d998                     (driver.of_match_table)
 * e nient'altro: id_table (+0), shutdown (+24), driver.owner (+48),
 * driver.mod_name (+56) e acpi_match_table (+80) sono nulli.
 *
 * 0x995ae20 e' spi_bus_type: il suo primo campo e' relocato a
 * 0xffffff8008f74640, che contiene la stringa "spi" -- cioe' bus_type.name.
 * La rilocazione e' STATICA, quindi il campo sta nell'inizializzatore e non
 * e' quello che __spi_register_driver assegna a runtime.
 *
 * driver.owner nullo e' THIS_MODULE in un builtin, coerente con
 * "aa1f03e0  mov x0, xzr" come primo argomento di __spi_register_driver:
 * spi_register_driver(&sf_spi_driver) espande a
 * __spi_register_driver(THIS_MODULE, ...).
 */
static struct spi_driver sf_spi_driver = {
	.driver = {
		.name = "sunwave-fp",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
		.of_match_table = sf_of_match,
	},
	.probe = sf_probe,
	.remove = sf_remove,
};

/*
 * L'array passato a spi_register_board_info, a 0xffffff8009411a38 (pagina
 * 0x9411000 = _einittext+0x68ae8, cioe' la zona __init: __initdata) con
 * "320003e1  orr w1, wzr, #0x1" come conteggio. I 72 byte on-disk, che sono
 * esattamente sizeof(struct spi_board_info) su questo sublevel:
 *   +0  73 75 6e 77 61 76 65 2d 66 70 00 ...   modalias = "sunwave-fp"
 *   +32 .. +63 zeri                             platform_data/properties/
 *                                               controller_data
 *   +56 00000000 00000000                       irq = 0, max_speed_hz = 0
 *   +64 00 00 | 02 00 | 00 00                   bus_num = 0, chip_select = 2,
 *                                               mode = 0
 * (l'oggetto successivo comincia a +72: la sua prima rilocazione punta alla
 * stringa "thermal", quindi appartiene a un altro sottosistema.)
 */
static struct spi_board_info sf_spi_board_info[] __initdata = {
	{
		.modalias = "sunwave-fp",
		.bus_num = 0,
		.chip_select = 2,
		.mode = 0,
	},
};

/*
 * Posizione 1. sf_probe(struct spi_device *spi), 2048 byte, il cuore del
 * driver. Letterali, tutti verificati byte per byte all'indirizzo citato:
 *   "sunwave-sf_ctl-%d: sunwave %s enter\n"@0x92504c2                 riga 653
 *   "sf_probe"@0x92504e9                                              (funcname)
 *   "sunwave-sf_ctl-%d: sf_platform_init failed with %d.\n"@0x92504f2 riga 670
 *   "sf_wakelock"@0x9250529
 *   "sunwave-sf_ctl-%d: gpio_init failed with %d.\n"@0x9250535        riga 686
 *   "sunwave-sf_ctl-%d: sunwave probe read chip id is failed\n"@0x9250565  riga 732
 *   "sunwave-sf_ctl-%d: sf_ctl_init_input failed with %d.\n"@0x92505a0    riga 761
 *   "sunwave-sf_ctl-%d: misc_register(..) = %d.\n"@0x92505d8          riga 771
 *   "sunwave-sf_ctl-%d: sf_ctl_init_irq failed with %d.\n"@0x9250606  riga 790
 *   "sunwave-sf_ctl-%d: %s leave\n"@0x925063c                         riga 831
 *
 * I tre spiazzamenti di struct spi_device usati qui (728 max_speed_hz,
 * 733 bits_per_word, 734 mode) vengono da
 *   "b902da93  str w19, [x20,#728]"   con w19 = 0xf4240 = 1000000, cioe'
 *                                     "52884813 mov w19,#0x4240" +
 *                                     "72a001f3 movk w19,#0xf, lsl #16"
 *   "390b7688  strb w8, [x20,#733]"   con w8 = 8
 *   "7905be9f  strh wzr, [x20,#734]"
 * e coincidono con l'ABI di struct spi_device di questo sublevel
 * (sizeof(struct device) = 712, poi controller/master a 712/720).
 *
 * SF_PLATFORM_EXIT HA CINQUE CHIAMATE QUI DENTRO (rilievo R1 della revisione
 * del lotto 2, che aveva corretto un "quattro" del messaggio di commit del
 * lotto 2): 0xa829e8, 0xa82d00, 0xa82f10, 0xa82f68, 0xa83084. Con quella di
 * sf_remove fanno SEI chiamate in tutto nel blocco.
 */
static int sf_read_sensor_id(void);
static int sf_ctl_init_input(struct sf_ctl_device *ctl);
static int sf_ctl_init_irq(struct sf_ctl_device *ctl);

static int sf_probe(struct spi_device *spi)
{
	struct sf_ctl_device *ctl = &sf_ctl_dev;
	int err;

	printk(KERN_INFO "sunwave-sf_ctl-%d: sunwave %s enter\n", 653, "sf_probe");
	ctl->spi = spi;
	spi->mode = 0;
	spi->bits_per_word = 8;
	spi->max_speed_hz = 1000000;
	spi_setup(spi);

	err = sf_platform_init(ctl);
	if (err) {
		ctl->spi = NULL;
		printk(KERN_ERR "sunwave-sf_ctl-%d: sf_platform_init failed with %d.\n", 670, err);
		return err;
	}

	wakeup_source_prepare(&ctl->ws, "sf_wakelock");
	wakeup_source_add(&ctl->ws);

	/* "f9404aa8  ldr x8, [x21,#144]" + "d63f0100  blr x8", x0 = base. */
	err = ctl->init_gpio_pins(ctl);
	if (err) {
		printk(KERN_ERR "sunwave-sf_ctl-%d: gpio_init failed with %d.\n", 686, err);
		ctl->free_gpio(ctl);
		sf_platform_exit(ctl);
		ctl->spi = NULL;
		return err;
	}

	if (sf_read_sensor_id()) {
		printk(KERN_ERR "sunwave-sf_ctl-%d: sunwave probe read chip id is failed\n", 732);
		ctl->free_gpio(ctl);
		sf_platform_exit(ctl);
		ctl->spi = NULL;
		/* "12800014  mov w20, #0xffffffff": -1, non un -Exxx. */
		return -1;
	}

	err = sf_ctl_init_input(ctl);
	if (err) {
		ctl->free_gpio(ctl);
		sf_platform_exit(ctl);
		ctl->spi = NULL;
		printk(KERN_ERR "sunwave-sf_ctl-%d: sf_ctl_init_input failed with %d.\n", 761, err);
		return err;
	}

	err = misc_register(&ctl->miscdev);
	if (err) {
		ctl->free_gpio(ctl);
		sf_platform_exit(ctl);
		printk(KERN_ERR "sunwave-sf_ctl-%d: misc_register(..) = %d.\n", 771, err);
		input_unregister_device(ctl->input);
		ctl->spi = NULL;
		return err;
	}

	/*
	 * "f9400288  ldr x8, [x20]" con x20 = 0x998dc20 = base+48
	 * (miscdev.this_device), "91004100  add x0, x8, #0x10" (&dev->kobj,
	 * spiazzamento 0x10 di struct device su questo sublevel) e
	 * "91378021  add x1, x1, #0xde0" = 0x998dde0 = l'attribute_group.
	 * L'esito non viene guardato: w0 e' sovrascritto subito dopo.
	 */
	sysfs_create_group(&ctl->miscdev.this_device->kobj, &sf_ctl_attr_group);
	INIT_WORK(&ctl->work, sf_ctl_device_event);
	g_probe_done = 1;

	err = sf_ctl_init_irq(ctl);
	if (err) {
		printk(KERN_ERR "sunwave-sf_ctl-%d: sf_ctl_init_irq failed with %d.\n", 790, err);
		input_unregister_device(ctl->input);
		misc_deregister(&ctl->miscdev);
		ctl->free_gpio(ctl);
		sf_platform_exit(ctl);
		ctl->spi = NULL;
		return err;
	}

	ctl->notifier.notifier_call = sf_fb_notifier_callback;
	fb_register_client(&ctl->notifier);
	printk(KERN_ERR "sunwave-sf_ctl-%d: %s leave\n", 831, "sf_probe");
	return err;
}

/*
 * Incorporata in sf_probe: "sf_read_sensor_id"@0x92507f6 esiste come
 * letterale passato a %s ma non come simbolo nella mappa.
 *   "sunwave-sf_ctl-%d: %s(..) enter.\n"@0x9250783                    riga 1001
 *   "sunwave-sf_ctl-%d: SPI transfer failed\n"@0x9250808              righe 1013, 1034, 1053
 *   "sunwave-sf_ctl-%d: read chip is ok\n"@0x9250832                  righe 1019, 1039, 1058
 *   "sunwave-sf_ctl-%d: SPI transfer failed step 1\n"@0x9250858       riga 1081
 *   "sunwave-sf_ctl-%d: SPI transfer failed step 2\n"@0x9250889       riga 1096
 *   "sunwave-sf_ctl-%d: SPI transfer failed step 3\n"@0x92508ba       riga 1114
 *   "sunwave-sf_ctl-%d: read chip is ok: 0x%02x%02x\n"@0x92508eb      riga 1119
 *   "sunwave-sf_ctl-%d: read chip: 0x%02x%02x\n"@0x925091d            riga 1131
 *
 * I due buffer sono 16 byte ciascuno: lo stack e' azzerato con
 * "a900ffff  stp xzr, xzr, [sp,#8]" (tx) e "a901ffff  stp xzr, xzr, [sp,#24]"
 * (rx), e ogni tee_spi_transfer riceve "x2 = sp+8" e "x3 = sp+24".
 *
 * I byte scritti in tx, letti dagli immediati:
 *   "528bf41b  mov w27, #0x5fa0"  + "790013fb  strh w27, [sp,#8]"   -> a0 5f
 *   "5293ec08  mov w8, #0x9f60" + "72a04508  movk w8, #0x228, lsl #16"
 *                                + "b9000be8  str w8, [sp,#8]"      -> 60 9f 28 02
 *   "52850c08  mov w8, #0x2860" + "72a00048  movk w8, #0x2, lsl #16"
 *                                + "b9000be8  str w8, [sp,#8]"      -> 60 28 02 00
 *   "5299a656  mov w22, #0xcd32" + "b9000bf6  str w22, [sp,#8]"     -> 32 cd 00 00
 *   "5280109c  mov w28, #0x84"   + "390033fc  strb w28, [sp,#12]"   -> tx[4] = 0x84
 *   "52801028  mov w8, #0x81"    + "390033e8  strb w8, [sp,#12]"    -> tx[4] = 0x81
 *   "52801069  mov w9, #0x83"    + "390033e9  strb w9, [sp,#12]"    -> tx[4] = 0x83
 *   "d28fd028  mov x8, #0x7e81" + "f2c00428  movk x8, #0x21, lsl #32"
 *                                + "a900ffe8  stp x8, xzr, [sp,#8]" -> 81 7e 00 00 21 00 ...
 * e le lunghezze passate a tee_spi_transfer, dall'immediato in w4:
 *   6 ("321f07e4 orr w4, wzr, #0x6"), 7 ("32000be4 orr w4, wzr, #0x7"),
 *   5 ("528000a4 mov w4, #0x5"), 10 ("52800144 mov w4, #0xa").
 *
 * I confronti sulla risposta: rx[2..5] contro 'S' 'u' 'n' 'W'
 * ("71014d1f cmp w8,#0x53", "7101d51f cmp w8,#0x75", "7101b91f cmp w8,#0x6e",
 * "71015d1f cmp w8,#0x57"), e rx[5]/rx[4]/rx[8] contro 0x82
 * ("7102091f cmp w8, #0x82").
 *
 * I DUE CICLI, e il dettaglio da non lisciare. Quello esterno parte da 3 e
 * scende: "320007e8 orr w8, wzr, #0x3" prima del ciclo, "2a0803f7 mov w23,w8"
 * in testa al corpo, "510006e8 sub w8, w23, #0x1" + "35fff877 cbnz w23" in
 * coda -- quattro giri, con la prova in coda. Quello interno conta 0..10:
 * "2a1f03f8 mov w24, wzr" nel preambolo, "11000678 add w24, w19, #0x1" +
 * "71002a7f cmp w19, #0xa" + "54fff76b b.lt" in coda -- undici giri, sempre
 * con la prova in coda, e il ritardo "331f7b1a bfi w26, w24, #1, #31" su un
 * w26 che vale 1, cioe' 2*i+1, incrementato di 2 a ogni giro
 * ("11000b5a add w26, w26, #0x2").
 * IL CONTATORE INTERNO NON VIENE AZZERATO A OGNI GIRO ESTERNO: w24 e'
 * scritto solo nel preambolo e in coda al ciclo interno, mai in testa al
 * ciclo esterno. E' per questo che `i` e' dichiarata fuori da entrambi.
 * Se fosse azzerata, il preambolo l'avrebbe azzerata dentro il ciclo.
 */
static int sf_read_sensor_id(void)
{
	struct sf_ctl_device *ctl = &sf_ctl_dev;
	u8 tx[16];
	u8 rx[16];
	int ret = -1;
	int retry;
	int i = 0;

	/* "f9400688  ldr x8, [x20,#8]" con x20 = base+168, cioe' base+176. */
	ctl->reset();
	/* "f9400288  ldr x8, [x20]" + "320003e0 orr w0, wzr, #0x1". */
	ctl->spi_clock_enable(true);

	sf_spi_conf[2] = 60;
	sf_spi_conf[3] = 60;
	sf_spi_conf[8] = 0;
	sf_spi_conf[9] = 0;
	sf_spi_conf[14] = 0;

	ctl->spi->max_speed_hz = 1000000;
	ctl->spi->bits_per_word = 8;
	ctl->spi->mode = 0;
	spi_setup(ctl->spi);

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) enter.\n", 1001, "sf_read_sensor_id");
	msleep(10);

	retry = 3;
	do {
		memset(tx, 0, sizeof(tx));
		memset(rx, 0, sizeof(rx));
		tx[0] = 0xa0;
		tx[1] = 0x5f;
		if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 6)) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed\n", 1013);
			continue;
		}
		if (rx[2] == 'S' && rx[3] == 'u' && rx[4] == 'n' && rx[5] == 'W') {
			printk(KERN_INFO "sunwave-sf_ctl-%d: read chip is ok\n", 1019);
			ret = 0;
			goto done;
		}

		memset(tx, 0, sizeof(tx));
		memset(rx, 0, sizeof(rx));
		tx[0] = 0x60;
		tx[1] = 0x9f;
		tx[2] = 0x28;
		tx[3] = 0x02;
		tx[4] = 0x00;
		if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 7)) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed\n", 1034);
			continue;
		}
		if (rx[5] == 0x82) {
			printk(KERN_INFO "sunwave-sf_ctl-%d: read chip is ok\n", 1039);
			ret = 0;
			goto done;
		}

		memset(tx, 0, sizeof(tx));
		memset(rx, 0, sizeof(rx));
		tx[0] = 0x60;
		tx[1] = 0x28;
		tx[2] = 0x02;
		if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 6)) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed\n", 1053);
			continue;
		}
		if (rx[4] == 0x82) {
			printk(KERN_INFO "sunwave-sf_ctl-%d: read chip is ok\n", 1058);
			ret = 0;
			goto done;
		}

		do {
			/* "f94652a8  ldr x8, [x21,#3232]" = 0x998dca0 = base+176. */
			ctl->reset();
			msleep(2 * i + 1);

			memset(tx, 0, sizeof(tx));
			memset(rx, 0, sizeof(rx));
			tx[0] = 0x32;
			tx[1] = 0xcd;
			tx[4] = 0x84;
			if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 5)) {
				printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed step 1\n", 1081);
				continue;
			}
			msleep(1);

			memset(tx, 0, sizeof(tx));
			memset(rx, 0, sizeof(rx));
			tx[0] = 0x32;
			tx[1] = 0xcd;
			tx[4] = 0x81;
			if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 5)) {
				printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed step 2\n", 1096);
				continue;
			}
			msleep(5);

			memset(tx, 0, sizeof(tx));
			memset(rx, 0, sizeof(rx));
			tx[0] = 0x81;
			tx[1] = 0x7e;
			tx[4] = 0x21;
			if (tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 10)) {
				printk(KERN_ERR "sunwave-sf_ctl-%d: SPI transfer failed step 3\n", 1114);
				continue;
			}
			if (rx[8] == 0x82) {
				printk(KERN_INFO "sunwave-sf_ctl-%d: read chip is ok: 0x%02x%02x\n",
				       1119, rx[8], rx[9]);
				memset(tx, 0, sizeof(tx));
				memset(rx, 0, sizeof(rx));
				tx[0] = 0x32;
				tx[1] = 0xcd;
				tx[4] = 0x83;
				tee_spi_transfer(sf_spi_conf, sizeof(sf_spi_conf), tx, rx, 5);
				ret = 0;
				goto done;
			}
			printk(KERN_ERR "sunwave-sf_ctl-%d: read chip: 0x%02x%02x\n",
			       1131, rx[8], rx[9]);
		} while (i++ < 10);
	} while (retry-- != 0);

done:
	/* "f9464d08 ldr x8,[x8,#3224]" + "2a1f03e0 mov w0, wzr": on = false. */
	ctl->spi_clock_enable(false);
	return ret;
}

/*
 * Incorporata in sf_probe: "sf_ctl_init_input"@0x9250949.
 *   "sunwave-sf_ctl-%d: %s(..) enter.\n"@0x9250783                       riga 1173
 *   "sunwave-sf_ctl-%d: input_allocate_device(..) failed.\n"@0x925095b   riga 1177
 *   "sf-keys"@0x9250993
 *   "sunwave-sf_ctl-%d: input_register_device(..) = %d.\n"@0x925099b     riga 1196
 *   "sunwave-sf_ctl-%d: %s(..) leave.\n"@0x92507bf                       riga 1202
 *
 * Gli undici bit accesi si leggono da undici "orr x9, x9, #imm" su tre
 * spiazzamenti di struct input_dev (40 evbit, 48/56/64 keybit[0..2]):
 *   [40] |= 0x2                -> bit 1  di evbit   = EV_KEY
 *   [56] |= 0x4000000000       -> bit 38 di keybit[1] = 64+38 = 102 KEY_HOME
 *   [64] |= 0x800              -> bit 11 di keybit[2] = 128+11 = 139 KEY_MENU
 *   [64] |= 0x40000000         -> bit 30 di keybit[2] = 128+30 = 158 KEY_BACK
 *   [56] |= 0x10               -> bit  4 di keybit[1] = 64+4  =  68 KEY_F10
 *   [48] |= 0x10000000         -> bit 28 di keybit[0] =         28 KEY_ENTER
 *   [56] |= 0x8000000000       -> bit 39 di keybit[1] = 64+39 = 103 KEY_UP
 *   [56] |= 0x20000000000      -> bit 41 di keybit[1] = 64+41 = 105 KEY_LEFT
 *   [56] |= 0x40000000000      -> bit 42 di keybit[1] = 64+42 = 106 KEY_RIGHT
 *   [56] |= 0x100000000000     -> bit 44 di keybit[1] = 64+44 = 108 KEY_DOWN
 *   [64] |= 0x8000             -> bit 15 di keybit[2] = 128+15 = 143 KEY_WAKEUP
 * Sono ldr/orr/str semplici, non ldxr/stxr: __set_bit, non set_bit. E il
 * puntatore e' riletto ogni volta da 0x998dc70 ("f9463a68 ldr x8, [x19,#3184]"),
 * cioe' l'operando e' ctl->input e non una copia locale.
 *
 * I due errori sono "12800174 mov w20, #0xfffffff4" = -12 = -ENOMEM e
 * "12800254 mov w20, #0xffffffed" = -19 = -ENODEV.
 */
static int sf_ctl_init_input(struct sf_ctl_device *ctl)
{
	int err;

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) enter.\n", 1173, "sf_ctl_init_input");
	ctl->input = input_allocate_device();
	if (!ctl->input) {
		printk(KERN_ERR "sunwave-sf_ctl-%d: input_allocate_device(..) failed.\n", 1177);
		return -ENOMEM;
	}

	ctl->input->name = "sf-keys";
	__set_bit(EV_KEY, ctl->input->evbit);
	__set_bit(KEY_HOME, ctl->input->keybit);
	__set_bit(KEY_MENU, ctl->input->keybit);
	__set_bit(KEY_BACK, ctl->input->keybit);
	__set_bit(KEY_F10, ctl->input->keybit);
	__set_bit(KEY_ENTER, ctl->input->keybit);
	__set_bit(KEY_UP, ctl->input->keybit);
	__set_bit(KEY_LEFT, ctl->input->keybit);
	__set_bit(KEY_RIGHT, ctl->input->keybit);
	__set_bit(KEY_DOWN, ctl->input->keybit);
	__set_bit(KEY_WAKEUP, ctl->input->keybit);

	err = input_register_device(ctl->input);
	if (err) {
		printk(KERN_ERR "sunwave-sf_ctl-%d: input_register_device(..) = %d.\n", 1196, err);
		input_free_device(ctl->input);
		ctl->input = NULL;
		return -ENODEV;
	}
	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) leave.\n", 1202, "sf_ctl_init_input");
	return 0;
}

/*
 * Incorporata in sf_probe: "sf_ctl_init_irq"@0x9250ac9.
 *   "sunwave-sf_ctl-%d: %s(..) enter.\n"@0x9250783                  riga 1156
 *   "sunwave-sf_ctl-%d: request_irq(..) = %d.\n"@0x9250ae0          riga 1162
 *   "sf-irq"@0x9250ad9
 *   "sunwave-sf_ctl-%d: %s(..) leave.\n"@0x92507bf                  riga 1166
 *
 * La chiamata e' "97dae3c5  bl <request_threaded_irq>" con
 *   x0 = "b9402a80  ldr w0, [x20,#40]"  = 0x998dc48 = base+88 = ctl->irq
 *   x1 = "91248021  add x1, x1, #0x920" = 0xffffff8008a83920 = sf_ctl_device_irq
 *   x2 = "aa1f03e2  mov x2, xzr"        = thread_fn nullo
 *   w3 = "321f03e3  orr w3, wzr, #0x2"  = IRQF_TRIGGER_FALLING
 *   x4 = "912b6484  add x4, x4, #0xad9" = "sf-irq"
 *   x5 = "aa1f03e5  mov x5, xzr"        = dev_id nullo
 * cioe' request_irq(). Il thread_fn nullo e' quello che distingue
 * request_irq da request_threaded_irq.
 *
 * irq_set_irq_wake(irq, 1) e' fuori dall'if: il disassemblato lo esegue sia
 * sul ramo di errore (0xa8302c) sia su quello buono (0xa83098), e cosi' la
 * printk "leave".
 */
static int sf_ctl_init_irq(struct sf_ctl_device *ctl)
{
	int err;

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) enter.\n", 1156, "sf_ctl_init_irq");
	err = request_irq(ctl->irq, sf_ctl_device_irq, IRQF_TRIGGER_FALLING, "sf-irq", NULL);
	if (err)
		printk(KERN_ERR "sunwave-sf_ctl-%d: request_irq(..) = %d.\n", 1162, err);
	irq_set_irq_wake(ctl->irq, 1);
	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) leave.\n", 1166, "sf_ctl_init_irq");
	return err;
}

/*
 * Posizione 2. sf_remove(struct spi_device *spi): non ha nessuna printk (nel
 * disassemblato non c'e' una sola "bl <printk>"), quindi non ha letterali da
 * citare; tutto quello che dice e' nella sequenza di rilasci e negli
 * spiazzamenti, gia' citati sulla struttura.
 *
 * La guardia iniziale legge due campi prima di aprire il frame:
 *   "f9408509  ldr x9, [x8,#264]"  con x8 = base+144, cioe' base+408 (spi)
 *   "b40004c9  cbz x9, ...+0xa4"
 *   "f9400108  ldr x8, [x8]"       cioe' base+144 (init_gpio_pins)
 *   "b4000488  cbz x8, ...+0xa4"
 * e 0xa4 e' il "mov w0, wzr; ret" finale.
 *
 * "b9005a7f  str wzr, [x19,#88]" azzera irq DOPO free_irq, e
 * "f900ce7f  str xzr, [x19,#408]" azzera spi in fondo: sono i due soli
 * campi che la funzione scrive.
 *
 * LO SCARTO +44 (rilievo R2 della revisione del lotto 2). Di fabbrica questa
 * funzione e' 172 byte e chiama sf_platform_exit; da noi e' 216 perche'
 * clang la incorpora (un solo punto di chiamata nel lotto 2). Contabilita'
 * istruzione per istruzione: 43 istruzioni di fabbrica, 54 nostre, +11 =
 * +44 byte; l'incorporazione mette 15 istruzioni al posto di 2, cioe' +13.
 * Le altre DUE mancano perche' la fabbrica ri-materializza "adrp x19" +
 * "add x19, #0xbf0" una seconda volta dopo il ramo di free_irq
 * (0xffffff8008a83150 / 0xffffff8008a83154) e noi no: allocazione di
 * registri, rumore noto. Cioe' "tolta l'incorporazione coincidono istruzione
 * per istruzione" e' falso di ESATTAMENTE DUE ISTRUZIONI, ed e' il modo
 * giusto di dirlo.
 */
static int sf_remove(struct spi_device *spi)
{
	if (sf_ctl_dev.spi && sf_ctl_dev.init_gpio_pins) {
		fb_unregister_client(&sf_ctl_dev.notifier);
		if (sf_ctl_dev.input)
			input_unregister_device(sf_ctl_dev.input);
		/* "37f800c0  tbnz w0, #31, ...+0x60": salta se irq < 0. */
		if (sf_ctl_dev.irq >= 0) {
			free_irq(sf_ctl_dev.irq, &sf_ctl_dev);
			sf_ctl_dev.irq = 0;
		}
		misc_deregister(&sf_ctl_dev.miscdev);
		wakeup_source_remove(&sf_ctl_dev.ws);
		wakeup_source_drop(&sf_ctl_dev.ws);
		/* "f9404e68  ldr x8, [x19,#152]" + "d63f0100  blr x8", x0 = base. */
		sf_ctl_dev.free_gpio(&sf_ctl_dev);
		sf_platform_exit(&sf_ctl_dev);
		sf_ctl_dev.spi = NULL;
	}
	return 0;
}

/*
 * Posizione 3. sf_ctl_device_event(struct work_struct *ws): il gestore della
 * work_struct che sf_ctl_device_irq accoda. Non usa il parametro, non
 * restituisce niente (nessun "mov w0,..." prima del ret).
 *
 *   "sunwave-sf_ctl-%d: %s(..) enter.\n"@0x9250783   riga 233
 *   "sf_ctl_device_event"@0x9250ab5                  (funcname)
 *
 * L'array di ambiente e' locale, non una statica: il disassemblato copia i
 * suoi 16 byte da un modello in sola lettura sullo stack
 * ("a9402929  ldp x9, x10, [x9]" con x9 = 0xffffff8008f7f7a8, poi
 * "a900abe9  stp x9, x10, [sp,#8]"). Il modello e', letto con
 * tools/relocazioni.py:
 *   0xffffff8008f7f7a8: relocato, addend 0xffffff8009250aa4  -> "SPI_STATE=finger"@0x9250aa4
 *   0xffffff8008f7f7b0: non relocato                         -> NULL
 *
 * kobject_uevent_env riceve x0 = *(0x998dc20) + 0x10: il campo a
 * spiazzamento 48 e' miscdev.this_device (struct device *), e in questo
 * sublevel struct device ha kobj a spiazzamento 0x10 (dopo parent e p).
 * KOBJ_CHANGE = 2 e' "321f03e1  orr w1, wzr, #0x2".
 */
static void sf_ctl_device_event(struct work_struct *ws)
{
	char *envp[2] = { "SPI_STATE=finger", NULL };

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) enter.\n", 233, "sf_ctl_device_event");
	kobject_uevent_env(&sf_ctl_dev.miscdev.this_device->kobj, KOBJ_CHANGE, envp);
}

/*
 * Posizione 4. sf_fb_notifier_callback: nessuna printk, nessun letterale di
 * formato. I due modelli di ambiente, letti con tools/relocazioni.py:
 *   0xffffff8008f7f7b8: relocato, addend 0xffffff8009250b4e -> "SCREEN_STATUS=ON"@0x9250b4e
 *   0xffffff8008f7f7c0: non relocato                        -> NULL
 *   0xffffff8008f7f7c8: relocato, addend 0xffffff8009250b5f -> "SCREEN_STATUS=OFF"@0x9250b5f
 *   0xffffff8008f7f7d0: non relocato                        -> NULL
 *
 * Il confronto sull'evento e' "f100243f  cmp x1, #0x9" = FB_EVENT_BLANK
 * (include/linux/fb.h:148); quello sul valore, "7100111f  cmp w8, #0x4"
 * (FB_BLANK_POWERDOWN) e "350001c8  cbnz w8, ...+0x68" (cioe' l'altro ramo
 * e' == 0, FB_BLANK_UNBLANK). Il ramo == 4 sceglie il modello 0x8f7f7c8
 * (OFF), il ramo == 0 sceglie 0x8f7f7b8 (ON).
 *
 * "f9400448  ldr x8, [x2,#8]" e' evdata->data (struct fb_event ha info a 0 e
 * data a 8); "b9400108  ldr w8, [x8]" e' il *(int *) che ne segue.
 *
 * I due rami hanno la coda in comune nel binario (una sola stp + una sola
 * bl): e' il compilatore che ha fuso due blocchi identici, come si vede dal
 * fatto che l'unica cosa che cambia fra i due e' l'indirizzo del modello.
 */
static int sf_fb_notifier_callback(struct notifier_block *self,
				   unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int *blank;

	if (event == FB_EVENT_BLANK) {
		blank = evdata->data;
		if (*blank == FB_BLANK_POWERDOWN) {
			char *envp[2] = { "SCREEN_STATUS=OFF", NULL };

			kobject_uevent_env(&sf_ctl_dev.miscdev.this_device->kobj,
					   KOBJ_CHANGE, envp);
		} else if (*blank == FB_BLANK_UNBLANK) {
			char *envp[2] = { "SCREEN_STATUS=ON", NULL };

			kobject_uevent_env(&sf_ctl_dev.miscdev.this_device->kobj,
					   KOBJ_CHANGE, envp);
		}
	}
	return 0;
}

/*
 * Incorporata in sf_ctl_ioctl: "sf_ctl_report_key_event"@0x92507a7.
 *   "sunwave-sf_ctl-%d: %s(..) enter.\n"@0x9250783   riga 259
 *   "sunwave-sf_ctl-%d: %s(..) leave.\n"@0x92507bf   riga 308
 *
 * LA SECONDA TABELLA NON DECODIFICATA DEL LOTTO 2, a 0x8f7f7d8, e' qui: non
 * e' una tabella di salto ma una tabella di VALORI, quella che clang genera
 * per uno switch che assegna una costante. Il codice e'
 *   "51000508  sub w8, w8, #0x1"        indice = key - 1
 *   "7100251f  cmp w8, #0x9" + "54000cc8 b.hi"   fuori intervallo -> default
 *   "911f6129  add x9, x9, #0x7d8"      base della tabella
 *   "b868d922  ldr w2, [x9,w8,sxtw #2]" dieci voci da 4 byte
 * e i dieci u32 letti dal PT_LOAD sono
 *   102 139 158 68 28 103 105 106 108 143
 * cioe' KEY_HOME KEY_MENU KEY_BACK KEY_F10 KEY_ENTER KEY_UP KEY_LEFT
 * KEY_RIGHT KEY_DOWN KEY_WAKEUP -- gli stessi dieci che sf_ctl_init_input
 * accende in keybit, letti li' da dieci "orr" indipendenti su tre parole
 * (undici contando anche l'"orr" di evbit -- vedi riga 683: corretto dalla
 * revisione indipendente, difetto D3, "otto" era sbagliato e questo stesso
 * file lo diceva gia' giusto piu' sotto).
 * Il default e' "321c0fe2  orr w2, wzr, #0xf0" = 240 = KEY_UNKNOWN.
 *
 * Il valore e' un booleano: "7100011f cmp w8, #0x0" + "1a9f07e3 cset w3, ne",
 * cioe' !!kevent->value, che e' esattamente cio' che fa input_report_key.
 * La seconda chiamata, "input_event(dev, 0, 0, 0)", e' input_sync.
 */
struct sf_key_event {
	int key;	/* "b94003e8  ldr w8, [sp]"    */
	int value;	/* "b94007e8  ldr w8, [sp,#4]" */
};

static int sf_ctl_report_key_event(struct input_dev *input, struct sf_key_event *kevent)
{
	int code;

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) enter.\n", 259, "sf_ctl_report_key_event");
	switch (kevent->key) {
	case 1:
		code = KEY_HOME;
		break;
	case 2:
		code = KEY_MENU;
		break;
	case 3:
		code = KEY_BACK;
		break;
	case 4:
		code = KEY_F10;
		break;
	case 5:
		code = KEY_ENTER;
		break;
	case 6:
		code = KEY_UP;
		break;
	case 7:
		code = KEY_LEFT;
		break;
	case 8:
		code = KEY_RIGHT;
		break;
	case 9:
		code = KEY_DOWN;
		break;
	case 10:
		code = KEY_WAKEUP;
		break;
	default:
		code = KEY_UNKNOWN;
		break;
	}
	input_report_key(input, code, kevent->value != 0);
	input_sync(input);
	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(..) leave.\n", 308, "sf_ctl_report_key_event");
	return 0;
}

/*
 * Posizione 5. sf_ctl_ioctl(struct file *, unsigned int cmd, unsigned long arg),
 * 1268 byte.
 *   "sunwave-sf_ctl-%d: %s(_IO(type,nr) nr= 0x%08x, ..)\n"@0x9250666      riga 327
 *   "sf_ctl_ioctl"@0x925069c                                             (funcname)
 *   "sunwave-sf_ctl-%d: copy_from_user(..) failed.\n"@0x92506a9           riga 389
 *   "sunwave-sf_ctl-%d: copy_to_user(..) failed.\n"@0x92506da             riga 424
 *   "sunwave-sf_ctl-%d: sf_hw_info_t copy_from_user(..) failed.\n"@0x9250709  riga 434
 *   "sunwave-sf_ctl-%d: sf_hw_info_t copy_to_user(..) failed.\n"@0x9250747    riga 444
 * Il terzo argomento della printk d'ingresso e' "12001c23 and w3, w1, #0xff",
 * cioe' _IOC_NR(cmd).
 *
 * LA TABELLA DI SALTO NON DECODIFICATA DEL LOTTO 2, a 0x8f7f740. Il codice
 * che la usa e'
 *   "128e5fe9  mov w9, #0xffff8d00"     -0x7300
 *   "0b090289  add w9, w20, w9"         indice = cmd - 0x7300
 *   "7100c53f  cmp w9, #0x31" + "540017c8 b.hi"   fuori intervallo -> default
 *   "911d014a  add x10, x10, #0x740"    base della tabella
 *   "1000008b  adr x11, 0xffffff8008a83330"       ancora dei bersagli
 *   "7869794c  ldrh w12, [x10,x9,lsl #1]"         cinquanta voci da 2 byte
 *   "8b0c096b  add x11, x11, x12, lsl #2"         bersaglio = ancora + 4*voce
 * Le cinquanta voci, risolte, danno nove bersagli distinti (corretto dalla
 * revisione indipendente, difetto D2 -- l'elenco sotto ne ha sempre
 * elencati nove, era la frase introduttiva a dire "sette"):
 *   0x7300 0x7301 0x7303 0x7304 0x7307 0x7308 0x7321 -> 0xa83704 (uscita, err = 0)
 *   0x7302                                           -> 0xa834b4 (reset)
 *   0x7305                                           -> 0xa83330 (clock on)
 *   0x7306                                           -> 0xa83544 (clock off)
 *   0x730b                                           -> 0xa834c8 (max_speed_hz)
 *   0x730d                                           -> 0xa834e0 (put_user)
 *   0x7330                                           -> 0xa8355c (copy_from_user 352)
 *   0x7331                                           -> 0xa835ac (copy_to_user 352)
 *   tutte le altre                                   -> 0xa8360c (err = -EINVAL)
 *
 * La tabella occupa 0x8f7f740..0x8f7f7a4 (50 voci da DUE byte). Il modello
 * envp di sf_ctl_device_event comincia a 0x8f7f7a8: i due dati si
 * incastrano senza sovrapporsi, il che conferma la larghezza delle voci --
 * con voci da quattro byte la tabella si sarebbe mangiata i modelli.
 *
 * Gli altri sette comandi non stanno nella tabella ma nell'albero di
 * confronti che la precede, e i loro valori si leggono dagli immediati:
 *   "528d60a8  mov w8, #0x6b05"                                   0x00006b05
 *   "528d6088  mov w8, #0x6b04" + "72a80088 movk w8,#0x4004,lsl#16" 0x40046b04
 *   "528d6228  mov w8, #0x6b11" + "72a80088 movk w8,#0x4004,lsl#16" 0x40046b11
 *   "528e6128  mov w8, #0x7309" + "72a80108 movk w8,#0x4008,lsl#16" 0x40087309
 *   "528d6089  mov w9, #0x6b04" + "72b00089 movk w9,#0x8004,lsl#16" 0x80046b04
 *   "528e6408  mov w8, #0x7320" + "72b00108 movk w8,#0x8008,lsl#16" 0x80087320
 *   "528e6149  mov w9, #0x730a" + "72b80109 movk w9,#0xc008,lsl#16" 0xc008730a
 * I due valori 0x40046b03 e 0xc0087309 che compaiono nello stesso albero NON
 * sono comandi: sono i perni del confronto binario, e valgono ciascuno il
 * comando successivo meno uno. Chi ci cade sopra finisce nel ramo -EINVAL,
 * verificato seguendo i salti.
 *
 * I NOMI dei comandi non sono leggibili dal binario -- nessun letterale li
 * nomina -- quindi qui ci sono i valori, composti con le macro _IO/_IOR/_IOW
 * che li riproducono esattamente, e non nomi inventati.
 *
 * I confronti dell'albero sono FIRMATI ("b.le", "b.gt") anche se `cmd` e'
 * unsigned: e' il modo in cui LLVM ordina i valori di uno switch quando ne
 * costruisce l'albero binario, non un indizio sul tipo del parametro. Lo si
 * vede dall'ordine dei perni, che e' l'ordine firmato
 * (0x80046b04 < 0x80087320 < 0xc008730a < 0x6b05 < 0x7300).
 *
 * Il ritorno e' "93407d00  sxtw x0, w8": un int esteso a long, cioe' una
 * variabile int in una funzione che ritorna long.
 */
static long sf_ctl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct sf_ctl_device *ctl = &sf_ctl_dev;
	struct sf_key_event kevent;
	int err = 0;

	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(_IO(type,nr) nr= 0x%08x, ..)\n",
	       327, "sf_ctl_ioctl", _IOC_NR(cmd));

	switch (cmd) {
	/* 0x00007302 e 0x00006b05: "f9465108 ldr x8,[x8,#3232]" + blr, senza argomenti. */
	case _IO('s', 0x02):
	case _IO('k', 0x05):
		ctl->reset();
		break;
	/* 0x00007305: "f9464d08 ldr x8,[x8,#3224]" + "320003e0 orr w0,wzr,#0x1" + blr. */
	case _IO('s', 0x05):
		ctl->spi_clock_enable(true);
		break;
	/* 0x00007306: stessa ldr, "2a1f03e0 mov w0, wzr". */
	case _IO('s', 0x06):
		ctl->spi_clock_enable(false);
		break;
	/*
	 * 0x0000730b e 0x40046b04: "f946c500 ldr x0,[x8,#3464]" (0x998dd88 =
	 * base+408 = ctl->spi), "b902d813 str w19,[x0,#728]" con x19 = arg,
	 * poi "97fc1e88 bl <spi_setup>".
	 */
	case _IO('s', 0x0b):
	case _IOW('k', 0x04, u32):
		ctl->spi->max_speed_hz = arg;
		spi_setup(ctl->spi);
		break;
	/*
	 * 0x0000730d e 0x40046b11: controllo di accesso su 4 byte
	 * ("b100114a adds x10, x10, #0x4"), "b94d818b ldr w11,[x12,#3456]"
	 * (0x998dd80 = base+400) e "b900014b str w11,[x10]". Nessuna printk
	 * sul fallimento: "128001a8 mov w8, #0xfffffff2" = -14 = -EFAULT.
	 */
	case _IO('s', 0x0d):
	case _IOW('k', 0x11, u32):
		if (put_user(ctl->field_400, (int __user *)arg))
			err = -EFAULT;
		break;
	/*
	 * 0x40087309: copy_from_user di 8 byte ("321d03e2 orr w2, wzr, #0x8")
	 * sullo stack, poi "f9463913 ldr x19,[x8,#3184]" = ctl->input e la
	 * sf_ctl_report_key_event incorporata. Il ramo di fallimento passa da
	 * "940f522b bl <__memset>" -- la coda non copiata azzerata -- che e'
	 * la firma di copy_from_user e non di __copy_from_user.
	 */
	case _IOW('s', 0x09, struct sf_key_event):
		if (copy_from_user(&kevent, (void __user *)arg, sizeof(kevent))) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: copy_from_user(..) failed.\n", 389);
			err = -EFAULT;
			break;
		}
		sf_ctl_report_key_event(ctl->input, &kevent);
		break;
	/*
	 * 0x80087320: strncpy del letterale di versione nel buffer da 32 byte
	 * a 0xa100c6c, terminatore forzato con "39007c1f strb wzr,[x0,#31]",
	 * poi __arch_copy_to_user di 32 byte verso arg.
	 */
	case _IOR('s', 0x20, unsigned long):
		strncpy(g_driver_version, "v2.2.36-2018-11-14", sizeof(g_driver_version));
		g_driver_version[sizeof(g_driver_version) - 1] = '\0';
		if (copy_to_user((void __user *)arg, g_driver_version, sizeof(g_driver_version))) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: copy_to_user(..) failed.\n", 424);
			err = -EFAULT;
		}
		break;
	/* 0x00007330: "52802c02 mov w2,#0x160" verso 0xa100b0c, con __memset in coda. */
	case _IO('s', 0x30):
		if (copy_from_user(&g_tee_version, (void __user *)arg, sizeof(g_tee_version))) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: sf_hw_info_t copy_from_user(..) failed.\n", 434);
			err = -EFAULT;
		}
		break;
	/* 0x00007331: stessa lunghezza, __arch_copy_to_user, senza __memset. */
	case _IO('s', 0x31):
		if (copy_to_user((void __user *)arg, &g_tee_version, sizeof(g_tee_version))) {
			printk(KERN_ERR "sunwave-sf_ctl-%d: sf_hw_info_t copy_to_user(..) failed.\n", 444);
			err = -EFAULT;
		}
		break;
	/*
	 * Nove comandi con corpo vuoto: la tabella di salto (o l'albero) li
	 * manda all'uscita comune 0xa83704 dove w8 vale ancora zero, non al
	 * ramo -EINVAL di 0xa8360c. Sono quindi case veri, non default.
	 */
	case _IO('s', 0x00):
	case _IO('s', 0x01):
	case _IO('s', 0x03):
	case _IO('s', 0x04):
	case _IO('s', 0x07):
	case _IO('s', 0x08):
	case _IO('s', 0x21):
	case _IOR('k', 0x04, u32):
	case _IOWR('s', 0x0a, u64):
		break;
	/* "128002a8  mov w8, #0xffffffea" = -22 = -EINVAL. */
	default:
		err = -EINVAL;
		break;
	}
	return err;
}

/*
 * Posizione 6. sf_ctl_compat_ioctl, 24 byte, sei istruzioni, l'intera
 * funzione:
 *   ffffff8008a8379c:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
 *   ffffff8008a837a0:	910003fd 	mov	x29, sp
 *   ffffff8008a837a4:	92407c42 	and	x2, x2, #0xffffffff
 *   ffffff8008a837a8:	97fffec0 	bl	ffffff8008a832a8 <sf_ctl_ioctl>
 *   ffffff8008a837ac:	a8c17bfd 	ldp	x29, x30, [sp],#16
 *   ffffff8008a837b0:	d65f03c0 	ret
 * Il lotto 2 la lascio' fuori dicendo che "e' un guscio su sf_ctl_ioctl, non
 * ha senso scriverlo prima". La formulazione precisa, che il rilievo C della
 * revisione chiedeva, e' che NON COMPILAVA prima: era un vincolo, non un
 * giudizio. Ora sf_ctl_ioctl c'e' e questa segue.
 */
static long sf_ctl_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return sf_ctl_ioctl(filp, cmd, (u32)arg);
}

/*
 * Posizione 7. sf_show_version: nove sprintf in sequenza, ognuna aggiunge
 * al buffer sysfs. Formati:
 *   "solution:%s\n"@0x92509dd   "ca      :%s\n"@0x92509ea
 *   "ta      :%s\n"@0x92509f7   "alg     :%s\n"@0x9250a04
 *   "nav     :%s\n"@0x9250a11   "driver  :%s\n"@0x9250a1e
 *   "firmware:%s\n"@0x9250a2b   "sensor  :%s\n"@0x9250a38
 *   "vendor  :%s\n"@0x9250a45
 *
 * L'accumulatore e' un int, non una ssize_t. La fabbrica lo tiene in w21
 * (32 bit) e lo estende UNA SOLA VOLTA, in fondo:
 *   "8b20c260  add x0, x19, w0, sxtw"   buf + (int)n, con estensione
 *   "0b150015  add w21, w0, w21"        somma a 32 bit
 *   "0b0002a8  add w8, w21, w0"
 *   "93407d00  sxtw x0, w8"             la sola estensione, sul ritorno
 * Con "ssize_t n" clang somma a 64 bit ("8b20c2b5 add x21, x21, w0, sxtw")
 * e la sxtw finale sparisce: 260 byte invece di 264. Vedi il rapporto.
 */
static ssize_t sf_show_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	int n = 0;

	n += sprintf(buf + n, "solution:%s\n", g_tee_version.solution);
	n += sprintf(buf + n, "ca      :%s\n", g_tee_version.ca);
	n += sprintf(buf + n, "ta      :%s\n", g_tee_version.ta);
	n += sprintf(buf + n, "alg     :%s\n", g_tee_version.alg);
	n += sprintf(buf + n, "nav     :%s\n", g_tee_version.nav);
	n += sprintf(buf + n, "driver  :%s\n", g_tee_version.driver);
	n += sprintf(buf + n, "firmware:%s\n", g_tee_version.firmware);
	n += sprintf(buf + n, "sensor  :%s\n", g_tee_version.sensor);
	n += sprintf(buf + n, "vendor  :%s\n", g_tee_version.vendor);
	return n;
}

/*
 * Posizione 8. sunwave_chip_info_show: un'unica sprintf. Formato:
 *   "chip   : %s %s\nid     : 0x0 lib:%s\nvendor : fw:%s\nmore   : fingerprint\n"@0x9250a5c
 * (il testo "id : 0x0" e' letterale nel binario, non un valore calcolato).
 *
 * Gli argomenti, nei registri x2..x5, sono tutti spiazzamenti della stessa
 * g_tee_version, calcolati a partire da x3 = 0xa100b2c:
 *   "91040062  add x2, x3, #0x100"  -> 0xa100c2c = base+0x120  sensor
 *   x3                              -> 0xa100b2c = base+0x20   ca
 *   "91020064  add x4, x3, #0x80"   -> 0xa100bac = base+0xa0   alg
 *   "91038065  add x5, x3, #0xe0"   -> 0xa100c0c = base+0x100  firmware
 */
static ssize_t sunwave_chip_info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "chip   : %s %s\nid     : 0x0 lib:%s\nvendor : fw:%s\nmore   : fingerprint\n",
		       g_tee_version.sensor, g_tee_version.ca,
		       g_tee_version.alg, g_tee_version.firmware);
}

/*
 * Posizione 9. sunwave_version_show: sprintf(buf, "%s\n", ...).
 * Formato "%s\n"@0x9226be1 -- ma quel testo compare 1357 volte nel binario,
 * quindi la citazione dell'indirizzo da sola non prova niente: cio' che lo
 * prova e' la coppia adrp/add che ci punta dentro questa funzione,
 *   "f0003d01  adrp x1, ffffff8009226000"
 *   "912f8421  add  x1, x1, #0xbe1"
 * (0x9226000 + 0xbe1 = 0x9226be1).
 * L'argomento e' "912fb042  add x2, x2, #0xbec" su pagina 0xa100000, cioe'
 * 0xa100bec = g_tee_version + 0xe0 = il campo driver.
 */
static ssize_t sunwave_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", g_tee_version.driver);
}

/*
 * Posizione 10. sf_ctl_device_irq(int irq, void *dev_id).
 *   "sunwave-sf_ctl-%d: %s(irq = %d, ..) toggled.\n"@0x9250b0c   riga 241
 *   "sf_ctl_device_irq"@0x9250b3c                                (funcname)
 *
 * L'accodamento e' "321d03e0  orr w0, wzr, #0x8" +
 * "97d92dc7 bl <queue_work_on>": l'8 e' WORK_CPU_UNBOUND con
 * CONFIG_NR_CPUS=8, cioe' la forma espansa di schedule_work(). Il secondo
 * argomento e' "f9457501  ldr x1, [x8,#2792]" su pagina 0xffffff800985e000,
 * cioe' la variabile a 0xffffff800985eae8; nel nostro build ALPS
 * __stack_chk_guard sta a ...a88 e system_wq a ...ae8, gli stessi 12 bit
 * bassi e la stessa distanza di 0x60 che l'oracolo ha fra la guardia di
 * stack (0x985ea88, usata da sf_probe/sf_ctl_ioctl) e questa variabile:
 * e' system_wq.
 *
 * "97e9f723  bl <pm_wakeup_ws_event>" con w1 = 1250 e w2 = 0 e' la forma
 * espansa di __pm_wakeup_event(ws, 1250) (include/linux/pm_wakeup.h:206),
 * su x20+0x58 = base+184 = &ctl->ws.
 *
 * Il ritorno e' "320003e0  orr w0, wzr, #0x1" = IRQ_HANDLED.
 */
static irqreturn_t sf_ctl_device_irq(int irq, void *dev_id)
{
	disable_irq_nosync(irq);
	printk(KERN_INFO "sunwave-sf_ctl-%d: %s(irq = %d, ..) toggled.\n",
	       241, "sf_ctl_device_irq", irq);
	schedule_work(&sf_ctl_dev.work);
	__pm_wakeup_event(&sf_ctl_dev.ws, 1250);
	enable_irq(irq);
	return IRQ_HANDLED;
}

/*
 * Posizione 18. sf_ctl_driver_init(void), 216 byte, sezione __init
 * (`ffffff8009383534 t sf_ctl_driver_init` in oracolo/stock.map riga 52775:
 * fuori dall'intervallo del blocco per adiacenza, che si ferma a
 * 0xffffff8008a840d4).
 *   "sunwave-sf_ctl-%d: '%s' SW_BUS_NAME = %s\n"@0x9250b71                riga 1209
 *   "sf_ctl_driver_init"@0x9250b9d                                        (funcname)
 *   "spi bus"@0x9250bb0
 *   "sunwave-sf_ctl-%d: %s, Failed to register SPI driver.\n"@0x9250bb8   riga 1230
 *   "sunwave-sf_ctl-%d: sunwave fingerprint device control driver registered.\n"@0x9250bf1  riga 1263
 *   "v2.2.36-2018-11-14"@0x92507e3
 *   "sunwave-sf_ctl-%d: driver version: '%s'.\n"@0x9250c3d                riga 1264
 *
 * La guardia in testa e' "b94ce108 ldr w8, [x8,#3296]" (0xa100ce0) +
 * "7100051f cmp w8, #0x1" + "54000061 b.ne": se g_probe_done vale gia' 1 la
 * funzione mette "2a1f03f3 mov w19, wzr" e salta tutto il resto.
 *
 * La registrazione e' "9128e000 add x0, x0, #0xa38" (0x9411a38, l'array
 * __initdata) + "320003e1 orr w1, wzr, #0x1" + bl <spi_register_board_info>,
 * e poi "91240021 add x1, x1, #0x900" (0x998d900, lo spi_driver) +
 * "aa1f03e0 mov x0, xzr" + bl <__spi_register_driver>.
 *
 * Il livello di initcall NON e' leggibile da qui: l'indirizzo dice che sta
 * in .init.text, non in quale .initcallN.init sia il puntatore. module_init
 * (device_initcall) e' la scelta ordinaria e non e' una misura.
 */
static int __init sf_ctl_driver_init(void)
{
	int err;

	printk(KERN_INFO "sunwave-sf_ctl-%d: '%s' SW_BUS_NAME = %s\n",
	       1209, "sf_ctl_driver_init", "spi bus");
	if (g_probe_done == 1)
		return 0;

	spi_register_board_info(sf_spi_board_info, ARRAY_SIZE(sf_spi_board_info));
	err = spi_register_driver(&sf_spi_driver);
	/* "36f800e0  tbz w0, #31, ...+0x84": stampa solo se err < 0. */
	if (err < 0)
		printk(KERN_ERR "sunwave-sf_ctl-%d: %s, Failed to register SPI driver.\n",
		       1230, "sf_ctl_driver_init");

	printk(KERN_INFO "sunwave-sf_ctl-%d: sunwave fingerprint device control driver registered.\n", 1263);
	strncpy(g_driver_version, "v2.2.36-2018-11-14", sizeof(g_driver_version));
	g_driver_version[sizeof(g_driver_version) - 1] = '\0';
	printk(KERN_INFO "sunwave-sf_ctl-%d: driver version: '%s'.\n", 1264, g_driver_version);
	return err;
}
module_init(sf_ctl_driver_init);

/*
 * La funzione di uscita, trovata dal censimento della `.exit.text` del
 * 2026-08-21.  Non era mai stata cercata perche' nessuno strumento del
 * progetto la poteva vedere: `oracolo/stock.map` e' `nm -n` di uno stock.elf
 * la cui symtab viene dalla tabella kallsyms dell'immagine, e kallsyms --
 * con `# CONFIG_KALLSYMS_ALL is not set` nel config di fabbrica -- emette
 * solo i simboli in [_stext,_etext] e [_sinittext,_einittext]
 * (alps-mtkwatch/scripts/kallsyms.c righe 48-51).  `.exit.text` comincia
 * esattamente a `_einittext` = 0xffffff80093a8518, dove la mappa finisce.
 *
 * 0xffffff80093acb98, 44 byte:
 *   "a9bf7bfd stp"@0xffffff80093acb98   stp x29, x30, [sp,#-16]!
 *   "910003fd mov"@0xffffff80093acb9c   mov x29, sp
 *   "b0002f00 adrp"@0xffffff80093acba0  adrp x0, 0xffffff800998d000
 *   "91248000 add"@0xffffff80093acba4   add x0,x0,#0x920 -> 0xffffff800998d920
 *                                       = 0xffffff800998d900 + 0x20, cioe'
 *                                       &sf_spi_driver.driver: sf_spi_driver
 *                                       e' a 0xffffff800998d900, lo stesso
 *                                       indirizzo che sf_ctl_driver_init
 *                                       passa a __spi_register_driver
 *                                       ("91240021 add"@0xffffff800938358c),
 *                                       e `driver` sta a spiazzamento 0x20 di
 *                                       struct spi_driver
 *   "97c50651 bl"@0xffffff80093acba8    -> ffffff80084ee4ec <driver_unregister>
 *                                       cioe' l'inline spi_unregister_driver,
 *                                       il cui `if (sdrv)` sparisce perche'
 *                                       l'argomento e' un indirizzo costante
 *   "9111b400 add"@0xffffff80093acbb0   add x0,x0,#0x46d, cioe' il letterale
 *          "sunwave-sf_ctl-%d: sunwave fingerprint device control driver released.\n"@0x925046d
 *          (i byte veri all'indirizzo cominciano con \x016 = KERN_INFO)
 *   "5280a041 mov"@0xffffff80093acbb4   mov w1, #0x502 = 1282, il numero di
 *                                       riga che questo file stampa al posto
 *                                       di __LINE__, come in tutte le altre
 *                                       printk del driver
 *   "97b61a47 bl"@0xffffff80093acbb8    -> ffffff80081334d4 <printk>
 *   "d65f03c0 ret"@0xffffff80093acbc0
 *
 * L'ordine e' quello di fabbrica: **prima** si de-registra il driver SPI,
 * **poi** si stampa il messaggio.
 *
 * **IL NOME E' UNA SCELTA, NON UNA MISURA**: la printk di questa funzione non
 * passa alcun `__func__` (il formato ha un solo `%d`, e l'argomento e' 1282),
 * e la stringa "sf_ctl_driver_exit" non compare in nessun punto dell'immagine
 * (cercata su tutti i byte: 0 occorrenze).  `sf_ctl_driver_exit` e' il nome
 * simmetrico a `sf_ctl_driver_init`.
 *
 * Anche il `module_exit()` e' una scelta dichiarata: il puntatore che genera
 * sta in `.exitcall.exit`, che vmlinux.lds scarta (`EXIT_CALL` dentro
 * /DISCARD/, riga 98); la funzione sopravvive comunque perche' per un
 * built-in `__exit` implica `__used` (include/linux/init.h righe 78-85).
 * Dal binario «con module_exit()» e «senza» sono indistinguibili.
 *
 * MISURA DI DIMENSIONE NON FATTA: questo lotto non ha un albero di build per
 * sunwave.  I 44 byte di fabbrica sono letti dal disassemblato; il confronto
 * con il nostro `.o` va ancora fatto.
 */
static void __exit sf_ctl_driver_exit(void)
{
	spi_unregister_driver(&sf_spi_driver);
	printk(KERN_INFO "sunwave-sf_ctl-%d: sunwave fingerprint device control driver released.\n", 1282);
}
module_exit(sf_ctl_driver_exit);

MODULE_DESCRIPTION("Sunwave TrustKernel TEE fingerprint driver");
MODULE_LICENSE("GPL");
