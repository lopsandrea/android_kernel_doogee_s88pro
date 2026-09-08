/* SPDX-License-Identifier: GPL-2.0 */
/*
 * WTK_ILITEK_E977 -- pannello touch Ilitek TDDI, Doogee S88 Pro.
 * L'interfaccia comune alle OTTO unita' di traduzione del driver.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA. Nessun sorgente
 * pubblico e' stato letto per scrivere questo file. I driver Ilitek TDDI
 * circolano pubblicamente e ALPS contiene driver touch della stessa
 * famiglia (GT1151/, GT5688/): non sono stati usati, ne' per orientarsi ne'
 * per i nomi. Se un lotto futuro ne usasse uno, dovra' dichiararlo e
 * verificare ogni riga contro QUESTO binario.
 *
 * Ogni riga di questo header porta accanto la riga di disassemblato da cui
 * viene, nella forma "<8 cifre esadecimali> <mnemonico>"@0xINDIRIZZO.
 *
 * La fonte primaria e' `docs/bringup/rapporti/rapporto-ilitek-struttura.md`
 * (134 offset provati, 897 accessi a offset costante, 20 indicizzati, 78
 * prese d'indirizzo), che va letto insieme a questo file: qui c'e' la
 * struttura, li' c'e' la prova di ciascun offset e la prova che i buchi sono
 * buchi.
 *
 * ===========================================================================
 * COME SI CHIAMANO I CAMPI: LA CONVENZIONE, E PERCHE' E' QUESTA
 * ===========================================================================
 * Di 134 offset distinti, il binario ne nomina CINQUE. Non ce ne sono altri:
 * ne' `.ddebug`, ne' `module_version_attribute`, ne' i `__func__` dicono come
 * si chiami un campo. I cinque vengono da due macro che *stringano* il
 * proprio argomento e lasciano il testo nel binario come dato --
 * `mutex_init(m)` -> `__mutex_init((m), #m, &__key)` e `init_waitqueue_head(q)`
 * -> `__init_waitqueue_head((q), #q, &__key)`:
 *
 *   0xffffff800923ed67  "&idev->touch_mutex"        -> +112
 *   0xffffff800923ed7a  "&idev->debug_mutex"        -> +144
 *   0xffffff800923ed8d  "&idev->debug_read_mutex"   -> +176
 *   0xffffff800923eda5  "&(idev->inq)"              -> +568
 *   e il nome del puntatore stesso, `idev`.
 *
 * Per TUTTI GLI ALTRI il nome sarebbe inventato. La scelta fatta qui e' di
 * NON inventarlo: ogni campo senza nome dal binario si chiama `c<offset>`,
 * e il commento accanto dice cio' che del campo E' MISURATO -- larghezza,
 * segno, ruolo quando il codice lo prova. Un nome descrittivo sarebbe piu'
 * leggibile e sarebbe indistinguibile, per chi legge, da un fatto: in questo
 * progetto la classe di difetto «dati inventati» e' gia' stata trovata piu'
 * volte, e un nome e' il posto piu' facile in cui farla entrare senza che
 * nessuna misura la denunci. Il costo -- codice meno leggibile -- e'
 * dichiarato e accettato; e' reversibile in un lotto futuro, quando i ruoli
 * saranno misurati per tutte e otto le unita'.
 *
 * I sei OGGETTI DEL KERNEL (tre mutex, uno spinlock, una waitqueue, una
 * completion) fanno eccezione perche' il loro tipo e' misurato, non scelto:
 * li' il nome del tipo e' un fatto.
 *
 * ===========================================================================
 * COSA NON E' DECIDIBILE DA QUESTO BINARIO, E VA LETTO COME TALE
 * ===========================================================================
 * 1. `bool` contro `u8`. Su tutti e 50 i campi a un byte il primo uso del
 *    valore caricato e' stato classificato meccanicamente e `tbz`/`tbnz #0`
 *    -- il marcatore che avrebbe deciso -- NON COMPARE MAI. Restano quattro
 *    campi (527, 530, 532, 556) su cui clang emette `eor #1` su un byte
 *    appena letto e riscritto: prova che il compilatore *sapeva* che il byte
 *    vale 0 o 1, cioe' `bool` con `x = !x` oppure `u8` con `x ^= 1`. Le due
 *    restano indistinguibili. Qui sono tutti `u8`, e questa e' una SCELTA:
 *    `u8` e `bool` hanno lo stesso sizeof (misurato: sonda_sizeof_bool = 1)
 *    e la stessa disposizione, quindi la scelta non sposta nessun offset.
 * 2. Nessun campo e' `atomic_t` ne' `volatile`, e questo e' MISURATO: in
 *    tutta la struttura i mnemonici sono soltanto ldp/ldr/ldrb/ldrh/ldrsw/
 *    stp/str/strb/strh -- zero ldxr/stlxr/ldar/stlr, zero atomiche RMW -- e
 *    sette store larghi fondono due campi adiacenti (clang non fonde accessi
 *    volatili). Per le coppie fuse e' una prova; per i campi mai fusi e'
 *    un'assenza di indizi contrari, che e' meno.
 * 3. Il numero di elementi dell'array a +848 e' una SCELTA, non una misura:
 *    l'indice non e' confrontato con nessun limite. Vedi il commento li'.
 * 4. I sei buchi non riducibili a riempimento. Vedi `__ignoto_*` piu' sotto.
 *
 * ===========================================================================
 * LA TRAPPOLA DA NON RIPETERE: +452..+526
 * ===========================================================================
 * Dieci store larghi coprono +452..+526 e SEMBRANO campi a 8 byte. Non lo
 * sono: sono le istruzioni di un `memcpy` di 75 byte da un buffer di
 * firmware ("f900018b str"@0xffffff8008a69800 e seguenti, con la coda
 * sovrapposta tipica -- +516 copre 516..523 e +523 copre 523..526).
 * Prenderle per larghezze di campo darebbe una struttura sbagliata CHE
 * NESSUNA MISURA DI DIMENSIONE DENUNCEREBBE. Qui il blocco e' un array di
 * byte, ed e' l'unica lettura che il binario sostiene.
 */
#ifndef __ILITEK_H
#define __ILITEK_H

#include <linux/completion.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

/*
 * Dichiarazione incompleta, e resta tale in questo lotto. Di questa
 * struttura il gruppo A vede UN SOLO byte, quello a +0, che
 * `ilitek_tddi_dev_init` confronta con 0x18 per scegliere fra "I2C" e "SPI"
 * ("39400008 ldrb"@0xffffff8008a55310, "7100611f cmp"@0xffffff8008a55328).
 * Tutto il resto -- `name`, `owner`, `of_match_table`, il `driver` a +48 --
 * si legge nel gruppo B (`ilitek_tddi_interface_dev_init`), che questo lotto
 * non ha scritto. Definirla qui su un byte solo sarebbe la classe di difetto
 * «un buco nella struttura» applicata a una seconda struttura.
 */
/*
 * ===========================================================================
 * struct ilitek_hwif_info -- COMPLETATA DAL GRUPPO B, DEFINITA DAL GRUPPO C
 * ===========================================================================
 * Fino al lotto del gruppo A questa struttura era una dichiarazione
 * incompleta, e il motivo era scritto qui: il gruppo A ne vede UN SOLO byte,
 * quello a +0. Il gruppo B (`ilitek_tddi_interface_dev_init`) ne vede quattro
 * campi e li ha chiusi; il gruppo C (`tpd_local_init`) ne mostra l'OGGETTO,
 * che e' un globale statico in `.data` a 0xffffff8009987370, e da li' si
 * leggono anche i due campi che nessuna funzione tocca.
 *
 * I QUATTRO CAMPI CON UN RUOLO MISURATO. Nessuno di essi e' nominato dal
 * binario, ma di ciascuno e' misurata la DESTINAZIONE, che e' un campo di
 * una struttura del kernel con un nome vero:
 *
 *   +8  -> drv->driver.name          ("f9002008 str"@0xffffff8008a556dc)
 *   +16 -> drv->driver.owner         ("f9002808 str"@0xffffff8008a556f0)
 *   +24 -> drv->driver.of_match_table("f9003408 str"@0xffffff8008a55708)
 *
 * Il nome scelto qui e' quello della destinazione, non un'invenzione: e'
 * cio' che `ilitek.h` aveva gia' previsto («name, owner, of_match_table, il
 * driver a +48 -- si legge nel gruppo B»). Il TIPO viene dalla destinazione
 * per la stessa ragione.
 *
 * I BUCHI RESTANO BUCHI. +1..7, +32..39 e +40..47 non sono toccati da
 * nessuna delle 154 funzioni: non vanno riempiti con campi plausibili,
 * perche' sposterebbero i tre offset misurati.
 */
struct ilitek_hwif_info {
	/*
	 * +0  8 bit. Il tipo di interfaccia. Confrontato con 0x18 sia qui
	 * ("39400268 ldrb"@0xffffff8008a556c0, "7100611f cmp"@0xffffff8008a556c4)
	 * sia in `ilitek_tddi_dev_init` del gruppo A
	 * ("39400008 ldrb"@0xffffff8008a55310).
	 */
	u8 type;
	u8 __ignoto_1[7];
	/* +8   -> `drv->driver.name`. "f9400668 ldr"@0xffffff8008a556cc */
	const char *name;
	/* +16  -> `drv->driver.owner`. "f9400a68 ldr"@0xffffff8008a556e0 */
	struct module *owner;
	/* +24  -> `drv->driver.of_match_table`. "f9400e68 ldr"@0xffffff8008a556f4 */
	const struct of_device_id *of_match_table;
	/*
	 * +32  Puntatore a funzione, CHIAMATO in coda a `ilitek_i2c_probe`:
	 * "f9401108 ldr"@0xffffff8008a5596c lo carica e
	 * "d63f0100 blr"@0xffffff8008a55970 lo chiama, col valore di ritorno
	 * usato come ritorno della `probe`.
	 *
	 * NON RICEVE ARGOMENTI, ed e' una misura: fra l'ultima chiamata
	 * (`kmem_cache_alloc_trace` a "97dfe507 bl"@0xffffff8008a55890) e il
	 * `blr` nessuna istruzione scrive x0 -- le trentadue in mezzo sono
	 * tutte store su `idev` o materializzazioni di costanti. Se un
	 * argomento ci fosse, x0 andrebbe caricato, e non lo e'.
	 *
	 * IL BERSAGLIO E' `ilitek_plat_probe`, e non e' dedotto: l'oggetto
	 * `hwif` e' un globale in `.data` a 0xffffff8009987370, i suoi
	 * puntatori valgono zero nell'immagine (`CONFIG_RELOCATABLE=y`) e la
	 * rilocazione a 0xffffff8009987390 porta l'addend 0xffffff8008a566c4.
	 * `tools/relocazioni.py` lo legge.
	 *
	 * Il gruppo B poteva solo tipare questo campo dal sito di chiamata e
	 * lasciava la cosa aperta come divergenza B5: il gruppo C la CHIUDE.
	 */
	int (*c32)(void);
	/*
	 * +40  Secondo puntatore a funzione. NESSUNA delle 154 lo chiama, e
	 * il gruppo B lo teneva per un buco. Non lo e': l'oggetto `hwif` e'
	 * un globale in `.data` a 0xffffff8009987370, e la rilocazione a
	 * 0xffffff8009987398 porta l'addend 0xffffff8008a568fc, cioe'
	 * `ilitek_plat_remove`. La FIRMA e' quella della definizione, non di
	 * un sito di chiamata -- qui il sito di chiamata non esiste.
	 */
	int (*c40)(void);
	/*
	 * +48  La `i2c_driver` allocata. Scritta qui
	 * ("f9001a60 str"@0xffffff8008a556d0) e riletta da
	 * `ilitek_tddi_interface_dev_exit` ("f9401913 ldr"@0xffffff8008a55a18)
	 * per `i2c_del_driver` e `kfree`.
	 */
	struct i2c_driver *driver;
};



/*
 * Solo il tipo del puntatore a +32 serve qui. `linux/pm_wakeup.h` NON viene
 * incluso: in questo albero non e' autonomo (usa `struct device` senza
 * includerlo) e tirarselo dentro obbligherebbe ogni unita' a includere
 * `linux/device.h` per una cosa che riguarda una sola funzione. Chi chiama
 * `wakeup_source_unregister` include `linux/device.h` da se'.
 */
struct wakeup_source;

/*
 * Solo il tipo del puntatore a +16 serve qui, e `linux/input.h` e' un
 * incluso pesante che tirerebbe dentro mezzo sottosistema per un puntatore.
 * Chi ne dereferenzia i campi -- il solo `ilitek_plat.c` -- lo include da se'.
 */
struct input_dev;

/*
 * ===========================================================================
 * LE DIMENSIONI DEGLI OGGETTI DEL KERNEL: MISURATE, NON ATTESE
 * ===========================================================================
 * Ciascuna decide dove comincia il campo successivo, quindi nessuna e' data
 * per buona. Sonda compilata con la RIGA DI COMANDO VERA estratta da
 * `out/drivers/input/touchscreen/mediatek/.tpd_debug.o.cmd` -- la riga del
 * sottoalbero del driver, non una qualsiasi -- col clang del progetto
 * (clang-r383902) nel container `s88pro-build`, scrivendo solo dentro
 * `out-ilitek/`:
 *
 *   sizeof(struct mutex)          = 32
 *   sizeof(spinlock_t)            = 4
 *   sizeof(wait_queue_head_t)     = 24
 *   sizeof(struct completion)     = 32, con `wait` a +8
 *   sizeof(bool)                  = 1
 *   sizeof(atomic_t)              = 4
 *
 * Tre conferme incrociate che potevano fallire e non falliscono:
 *  - i tre `mutex_init` cadono a 112, 144, 176 -- PASSO 32, esattamente il
 *    sizeof misurato -- e 176+32 = 208, dove cade lo `spin_lock_init`
 *    ("91034100 add"@0xffffff8008a56104);
 *  - `__init_waitqueue_head(idev+568)` piu' 24 = 592, il campo successivo;
 *  - `init_completion` scrive `done` a 744 ("b902e91f str"@0xffffff8008a54f0c)
 *    e chiama `__init_waitqueue_head(idev+752)`: 752-744 = 8 =
 *    offsetof(completion, wait) misurato, e 744+32 = 776, il campo successivo.
 */

/*
 * ===========================================================================
 * struct ilitek_tddi_dev -- 888 byte
 * ===========================================================================
 * Allocata UNA volta in `ilitek_i2c_probe` e raggiunta da ogni funzione del
 * blocco attraverso il globale `idev`:
 *
 *   ffffff8008a55820:	91008274 	add	x20, x19, #0x20
 *   ffffff8008a55824:	52901802 	mov	w2, #0x80c0
 *   ffffff8008a55828:	52806f01 	mov	w1, #0x378                 // #888
 *   ffffff8008a5582c:	72a02802 	movk	w2, #0x140, lsl #16
 *   ffffff8008a55830:	aa1403e0 	mov	x0, x20
 *   ffffff8008a55834:	97ea710b 	bl	<devm_kmalloc>
 *
 * 888 = 0x378 ("52806f01 mov"@0xffffff8008a55828).
 *
 * ED E' `devm_kzalloc`, NON `devm_kmalloc`: il terzo argomento vale
 * 0x80c0 | (0x140 << 16) = 0x14080c0 ("52901802 mov"@0xffffff8008a55824 piu'
 * "72a02802 movk"@0xffffff8008a5582c), cioe' GFP_KERNEL (0x14000c0) PIU'
 * __GFP_ZERO (0x8000). Conta per due motivi: la struttura arriva azzerata,
 * quindi ogni store esplicito di zero che si vede in `probe` e in
 * `ilitek_tddi_init` E' NEL SORGENTE (clang non puo' eliderlo, `devm_kmalloc`
 * e' opaca) -- ed e' cosi' che si legge la larghezza di parecchi campi; e una
 * riscrittura con `devm_kmalloc` sarebbe DELLA STESSA DIMENSIONE DI FUNZIONE
 * e diversa nel comportamento.
 *
 * `x0` e' `&client->dev`, cioe' `client + 32` ("91008274 add"@0xffffff8008a55820),
 * e offsetof(struct i2c_client, dev) MISURATO e' 32.
 *
 * I BUCHI: 114 byte su 888 (12,8%) che nessuna delle 154 funzioni tocca, in
 * tredici intervalli. Sette sono esattamente il riempimento che
 * l'allineamento impone, e dichiararli campi sarebbe inventare: 212..215,
 * 225..227, 336..339, 533..535, 557..559, 652..655, 740..743. Qui sono resi
 * dal riempimento implicito del compilatore, che produce gli stessi byte.
 * I SEI RESTANTI non sono spiegabili come riempimento e sono dichiarati
 * ignoti: 72..111 (40 byte), 252..263 (12), 272..287 (16), 304..319 (16),
 * 440..443 (4), 633 (1). La prova che nessuno li tocca e' costruita al
 * contrario e sulla variante *pessimistica* dell'analisi -- si prendono tutte
 * le istruzioni del blocco il cui offset immediato cade dentro un intervallo
 * dichiarato vuoto, con qualunque registro base, risolto o no:
 *
 *   totale candidati 94, di cui su idev 0
 *
 * Nove buchi su tredici hanno ZERO istruzioni in tutto il blocco con quel
 * offset immediato. Per i quattro rimanenti i 94 candidati sono esclusi uno
 * per uno (rapporto §5).
 *
 * `L` e `S` nei commenti sono quante letture e quante scritture in tutto il
 * blocco delle 154 funzioni: non provano il tipo, dicono dove guardare per
 * primo.
 */
struct ilitek_tddi_dev {
	/* +0   puntatore. L=4 S=1. "f9000113 str"@0xffffff8008a558a8 */
	void *c0;
	/*
	 * +8   64 bit. L=3 S=1. "f900051f str"@0xffffff8008a558bc -- lo store
	 * di `probe` e' `str wzr`, cioe' NULL esplicito.
	 * E' il primo argomento passato alla funzione a +792, che nessuna
	 * delle 154 scrive: vedi il commento a +792.
	 */
	void *c8;
	/*
	 * +16  puntatore a `struct input_dev`. L=69 S=1.
	 * "f9000920 str"@0xffffff8008a55de0 -- l'UNICO store, ed e' il gruppo
	 * C a farlo.
	 *
	 * IL TIPO E' MISURATO, e il lotto del gruppo A poteva solo scrivere
	 * `void *`: `ilitek_plat_input_register` ci mette il campo a +24 del
	 * globale `tpd` di `mtk_tpd` ("f9400d00 ldr"@0xffffff8008a55dd4), che
	 * in `struct tpd_device` e' l'`input_dev`, e poi lo usa agli offset
	 * dell'ABI di questa struttura: `propbit` a +32
	 * ("91008101 add"@0xffffff8008a55e94), `evbit` a +40
	 * ("9100a001 add"@0xffffff8008a55e2c), `keybit` a +48
	 * ("9100c101 add"@0xffffff8008a55e6c). Tre offset che combaciano non
	 * sono un caso.
	 */
	struct input_dev *c16;
	/*
	 * +24  64 bit. L=5 S=1. "f9000d14 str"@0xffffff8008a558c0
	 */
	void *c24;
	/*
	 * +32  puntatore a `struct wakeup_source`. L=3 S=1.
	 * "f9001128 str"@0xffffff8008a551b0 lo scrive in `ilitek_tddi_init`;
	 * il TIPO e' misurato in `ilitek_tddi_dev_remove`, che lo passa a
	 * `wakeup_source_unregister` ("f9401100 ldr"@0xffffff8008a552cc,
	 * "97eaae70 bl"@0xffffff8008a552d4 verso <wakeup_source_unregister>).
	 */
	struct wakeup_source *c32;
	/*
	 * +40  puntatore. L=1 S=1. "f900150b str"@0xffffff8008a558ec
	 * `ilitek_i2c_probe` ci scrive il puntatore riletto da
	 * `client->dev.driver` + 232, cioe' la `struct ilitek_hwif_info *`
	 * agganciata alla `i2c_driver` da 240 byte.
	 */
	void *c40;
	/* +48  puntatore. L=57 S=1. "a9032149 stp"@0xffffff8008a5aad0 */
	void *c48;
	/* +56  puntatore. L=13 S=3. "f9001d0a str"@0xffffff8008a5a814 */
	void *c56;
	/*
	 * +64  puntatore a un blocco di 56 byte: `kzalloc(56, GFP_KERNEL)` in
	 * `probe` -- gfp 0x14080c0, size 0x38 ("321d0be2 orr"@0xffffff8008a5588c,
	 * "f9002100 str"@0xffffff8008a5589c) -- e `kfree` in
	 * `ilitek_tddi_dev_remove` ("f9402100 ldr"@0xffffff8008a552e8).
	 * L=2 S=1.
	 */
	void *c64;
	/*
	 * +72..111, 40 byte: BUCO DICHIARATO IGNOTO. Non e' riempimento --
	 * l'allineamento successivo (mutex, 8) non ne richiede nemmeno uno.
	 * 80 istruzioni del blocco portano un offset immediato in questo
	 * intervallo, ZERO su `idev`: sei funzioni coinvolte non caricano mai
	 * il globale (`ilitek_node_compat_ioctl`, `ilitek_tddi_mp_test_main`,
	 * `open_test_sp`, `ilitek_tddi_fw_upgrade`,
	 * `ilitek_tddi_interface_dev_init`), e dove l'analisi risolve la base
	 * la base e' un altro globale.
	 * NON RIEMPIRE con campi plausibili: sposterebbe tutto cio' che segue.
	 */
	u8 __ignoto_72[40];
	/* +112 `struct mutex`. Nome DAL BINARIO: "&idev->touch_mutex"
	 * @0xffffff800923ed67. "9101c100 add"@0xffffff8008a53f44 */
	struct mutex touch_mutex;
	/* +144 `struct mutex`. Nome DAL BINARIO: "&idev->debug_mutex"
	 * @0xffffff800923ed7a. "91024100 add"@0xffffff8008a54eac */
	struct mutex debug_mutex;
	/* +176 `struct mutex`. Nome DAL BINARIO: "&idev->debug_read_mutex"
	 * @0xffffff800923ed8d. "9102c100 add"@0xffffff8008a54ec8 */
	struct mutex debug_read_mutex;
	/* +208 `spinlock_t`, da `spin_lock_init`.
	 * "91034100 add"@0xffffff8008a56104. Riempimento 212..215 fino a 216. */
	spinlock_t c208;
	/*
	 * +216 `const char *`. L=0 S=1. "f9006d09 str"@0xffffff8008a558c8:
	 * `probe` ci scrive 0xffffff80091aa860, che contiene "I2C".
	 * `ilitek_tddi_dev_init` sceglie fra quel letterale e "SPI"
	 * (0xffffff800923ef05) con una `csel` in base al primo byte della
	 * `hwif`. Sono due letterali di dati veri, non nomi di campo.
	 */
	const char *c216;
	/*
	 * +224 un byte. L=2 S=3. "39038109 strb"@0xffffff8008a53b0c.
	 * In `ilitek_tddi_fw_upgrade_handler` e' la guardia che fa registrare
	 * l'input UNA volta sola: se e' zero si stampa "Registre touch to
	 * input subsystem", lo si mette a 1 e si registra. Riempimento
	 * 225..227 fino a 228.
	 */
	u8 c224;
	/* +228 32 bit. L=1 S=2. "291ca504 stp"@0xffffff8008a57e04 (fuso col
	 * campo a +232 da una sola `stp`) */
	u32 c228;
	/* +232 32 bit. L=3 S=2. "291ca504 stp"@0xffffff8008a57e04 */
	u32 c232;
	/* +236 16 bit. L=0 S=1. "7901d905 strh"@0xffffff8008a5a58c */
	u16 c236;
	/* +238 16 bit. L=0 S=1. "7901dd06 strh"@0xffffff8008a5a59c */
	u16 c238;
	/* +240 16 bit. L=0 S=1. "7901e103 strh"@0xffffff8008a5a568 */
	u16 c240;
	/* +242 16 bit. L=0 S=1. "7901e504 strh"@0xffffff8008a5a57c */
	u16 c242;
	/* +244 16 bit. L=9 S=3. "7901e948 strh"@0xffffff8008a5a258 (fuso con
	 * +246) */
	u16 c244;
	/* +246 16 bit. L=9 S=2. "7901ed49 strh"@0xffffff8008a5a25c */
	u16 c246;
	/*
	 * +248..+251 quattro byte. Entrano tutti e quattro, e SOLO li',
	 * nella lunghezza del pacchetto calcolata da `ilitek_set_tp_data_len`
	 * per i formati 1 e 7:
	 *   len = (c249 * c248 + c250 + c251) * 2 + 56
	 * ("3943e109 ldrb"@0xffffff8008a53ce4 .. "3943ed0c ldrb"@0xffffff8008a53cf0,
	 *  "1b092d49 madd"@0xffffff8008a53cf8, "531f7929 lsl"@0xffffff8008a53d00,
	 *  "1100e135 add"@0xffffff8008a53d04).
	 * Sono scritti in `ilitek_tddi_ic_get_tp_info` (gruppo D):
	 * "3903e109 strb"@0xffffff8008a5a5a4 e seguenti.
	 */
	u8 c248;	/* L=7 S=1 */
	u8 c249;	/* L=6 S=1 */
	u8 c250;	/* L=1 S=1 */
	u8 c251;	/* L=1 S=1 */
	/*
	 * +252..263, 12 byte: BUCO DICHIARATO IGNOTO. Non e' riempimento: il
	 * campo successivo e' allineato a 8 e 252 -> 256 basterebbero quattro
	 * byte. Zero istruzioni del blocco portano un offset immediato qui.
	 */
	u8 __ignoto_252[12];
	/*
	 * +264 puntatore a un blocco di 2048 byte: `kzalloc(2048, GFP_ATOMIC)`
	 * in `probe` -- gfp 0x1088020, size 0x800 ("321503e2 orr"@0xffffff8008a55860,
	 * "f9008500 str"@0xffffff8008a55870) -- e `kfree` in
	 * `ilitek_tddi_dev_remove` ("f9408500 ldr"@0xffffff8008a552dc).
	 * L=8 S=1.
	 */
	void *c264;
	/*
	 * +272..287, 16 byte: BUCO DICHIARATO IGNOTO. Zero istruzioni del
	 * blocco con questo offset immediato.
	 */
	u8 __ignoto_272[16];
	/* +288 64 bit. L=1 S=4. "f900911f str"@0xffffff8008a68680 (store di
	 * zero esplicito) */
	u64 c288;
	/* +296 64 bit. L=4 S=6. "f9009500 str"@0xffffff8008a68690 */
	u64 c296;
	/*
	 * +304..319, 16 byte: BUCO DICHIARATO IGNOTO. Zero istruzioni del
	 * blocco con questo offset immediato.
	 */
	u8 __ignoto_304[16];
	/*
	 * +320 32 bit. L=10 S=10. "b9014109 str"@0xffffff8008a5376c.
	 * E' il modo TP corrente: `ilitek_tddi_switch_tp_mode` ci scrive il
	 * proprio argomento ("b9014113 str"@0xffffff8008a538b4) e lo rilegge
	 * per stamparlo ("b9414103 ldr"@0xffffff8008a539bc, formato
	 * "Actual TP mode = %d"@0xffffff800923e1dd).
	 */
	int c320;
	/*
	 * +324 32 bit. L=1 S=2. "b901451f str"@0xffffff8008a5503c (fuso con
	 * +328 da uno store largo). E' il formato dei dati del pacchetto:
	 * `ilitek_set_tp_data_len` ci scrive il proprio argomento
	 * ("b9014513 str"@0xffffff8008a53df8).
	 */
	int c324;
	/*
	 * +328 32 bit. L=1 S=1. "b901490a str"@0xffffff8008a558f4. E' la
	 * lunghezza del pacchetto calcolata da `ilitek_set_tp_data_len`
	 * ("b9014915 str"@0xffffff8008a53dfc).
	 */
	int c328;
	/* +332 32 bit. L=10 S=1. "b9014d00 str"@0xffffff8008a562b4 */
	int c332;
	/*
	 * +336..339, 4 byte: riempimento d'allineamento (il campo a +340 e' a
	 * 4 byte, quindi qui il compilatore non ne produrrebbe -- ma
	 * l'intervallo e' vuoto e va dichiarato). Reso esplicito perche' NON
	 * e' aritmetica d'allineamento: nessuno lo tocca e nessuno sa cosa sia.
	 */
	u8 __ignoto_336[4];
	/* +340 32 bit. L=2 S=1. "b901550b str"@0xffffff8008a55960 */
	int c340;
	/*
	 * +344 32 bit. L=9 S=1. "b9415900 ldr"@0xffffff8008a55270 (fuso con
	 * +348). E' un numero di GPIO: `ilitek_tddi_dev_remove` lo passa a
	 * `gpio_free` ("97e88c56 bl"@0xffffff8008a55274).
	 */
	int c344;
	/*
	 * +348 32 bit. L=5 S=0. "b9415d00 ldr"@0xffffff8008a55264. Anche
	 * questo un numero di GPIO, liberato PRIMA di +344
	 * ("97e88c59 bl"@0xffffff8008a55268). Quale sia il reset e quale
	 * l'interrupt non e' deciso da questo lotto: lo dira' il gruppo C.
	 * S=0 in tutto il blocco: e' scritto solo dal `devm_kzalloc` o da
	 * codice fuori dal confine.
	 */
	int c348;
	/* +352 32 bit CON SEGNO (`ldrsw`). L=20 S=4.
	 * "b901611f str"@0xffffff8008a5c6e4 */
	s32 c352;
	/* +356 array di 10 `int`, indicizzato. "b9016509 str"@0xffffff8008a5c7d8 */
	int c356[10];
	/* +396 array di 10 `int`, indicizzato. "b9418d49 ldr"@0xffffff8008a5c8a4 */
	int c396[10];
	/* +436 32 bit. L=4 S=2. "b901b509 str"@0xffffff8008a5ca48 */
	int c436;
	/*
	 * +440..443, 4 byte: BUCO DICHIARATO IGNOTO. Non e' riempimento: +436
	 * e +444 sono entrambi a 4 byte e contigui lo sarebbero senza. Zero
	 * istruzioni del blocco con questo offset immediato.
	 */
	u8 __ignoto_440[4];
	/*
	 * +444 32 bit. L=2 S=9. "b901bd1f str"@0xffffff8008a53a98.
	 * E' l'avanzamento dell'aggiornamento firmware in percento:
	 * `ilitek_tddi_fw_upgrade_handler` lo azzera all'inizio
	 * ("b901bd1f str"@0xffffff8008a53a98) e ci mette 100 (0x64) o -1 alla
	 * fine ("52800c89 mov"@0xffffff8008a53ae0,
	 * "b901bd09 str"@0xffffff8008a53aec).
	 */
	int c444;
	/*
	 * +448 32 bit. L=1 S=1. "b901c10a str"@0xffffff8008a55944.
	 * E' l'unico argomento di `ilitek_tddi_fw_upgrade`
	 * ("b941c100 ldr"@0xffffff8008a53a94).
	 */
	int c448;
	/*
	 * +452..526, 75 byte. NON SONO CAMPI A 8 BYTE: sono il bersaglio di un
	 * `memcpy` di 75 byte da un buffer di firmware
	 * ("f900018b str"@0xffffff8008a69800 e seguenti, con la coda
	 * sovrapposta tipica -- +516 copre 516..523 e +523 copre 523..526).
	 * I soli byte riletti singolarmente, e quindi le sole larghezze che il
	 * binario dichiara, sono: 452, 457, 459, 460, 461, 462, 463, 464, 466,
	 * 468, 469, 470, 471, 500, 501, 502, 503, 520, 521, 522, 524, 525, 526.
	 * Finche' non sara' letto il gruppo G (firmware e flash) resta un
	 * blocco di byte, che e' l'unica lettura che il binario sostiene.
	 */
	u8 c452[75];
	/*
	 * +527 un byte, 0/1 (clang emette `eor #1` su un byte appena letto e
	 * riscritto: `bool` con `x = !x` oppure `u8` con `x ^= 1`, e le due
	 * restano indistinguibili). L=7 S=1.
	 * "39083d03 strb"@0xffffff8008a6d28c.
	 * E' la guardia comune di ENTRAMBI i rami di `ilitek_tddi_wq_ctrl`:
	 * "39483d28 ldrb"@0xffffff8008a5408c (ramo ESD) e
	 * "39483d28 ldrb"@0xffffff8008a54124 (ramo batteria).
	 */
	u8 c527;
	/*
	 * +528 un byte. L=5 S=3. "3908412a strb"@0xffffff8008a540ac:
	 * `ilitek_tddi_wq_ctrl` ci scrive `(ctrl != 0)` sul ramo ESD.
	 */
	u8 c528;
	/*
	 * +529 un byte. L=5 S=3. "3908452a strb"@0xffffff8008a54144: lo stesso
	 * sul ramo batteria.
	 */
	u8 c529;
	/* +530 un byte, 0/1 (`eor #1`). L=5 S=3.
	 * "3908491f strb"@0xffffff8008a55958 */
	u8 c530;
	/* +531 un byte. L=3 S=1. "39084d0a strb"@0xffffff8008a55954 */
	u8 c531;
	/* +532 un byte, 0/1 (`eor #1`). L=5 S=1.
	 * "39085103 strb"@0xffffff8008a6d318. Riempimento 533..535 fino a 536. */
	u8 c532;
	/* +536 32 bit. L=5 S=2. "b9021909 str"@0xffffff8008a6d400 (fuso con
	 * +540). Letto da `ilitek_set_tp_data_len` sul formato 3
	 * ("b9421908 ldr"@0xffffff8008a53d3c) e passato alla funzione a +816
	 * da `ilitek_tddi_switch_tp_mode` ("b9421900 ldr"@0xffffff8008a53914). */
	int c536;
	/*
	 * +540 32 bit. L=0 S=1. "b9021d28 str"@0xffffff8008a6d06c.
	 * `ilitek_set_tp_data_len` lo legge sul formato 3
	 * ("b9421d03 ldr"@0xffffff8008a53d38) e lo passa come valore a
	 * `ilitek_tddi_ic_func_ctrl("gesture_demo_en", ...)`
	 * ("b9421d01 ldr"@0xffffff8008a53d8c).
	 */
	int c540;
	/* +544 16 bit. L=2 S=2. "79044143 strh"@0xffffff8008a6adc8 */
	u16 c544;
	/* +546 16 bit. L=2 S=1. "79044544 strh"@0xffffff8008a6addc */
	u16 c546;
	/* +548 32 bit. L=5 S=1. "b9022549 str"@0xffffff8008a6adf0 */
	int c548;
	/* +552 32 bit. L=2 S=1. "b9022948 str"@0xffffff8008a6ae08 */
	int c552;
	/* +556 un byte, 0/1 (`eor #1`). L=4 S=5.
	 * "3908b11f strb"@0xffffff8008a5595c. Riempimento 557..559 fino a 560. */
	u8 c556;
	/* +560 32 bit. L=3 S=3. "b902312a str"@0xffffff8008a5cd68 (fuso con
	 * +564) */
	int c560;
	/* +564 32 bit CON SEGNO (`ldrsw`). L=13 S=2.
	 * "b9023509 str"@0xffffff8008a6c5d4 */
	s32 c564;
	/* +568 `wait_queue_head_t`. Nome DAL BINARIO: "&(idev->inq)"
	 * @0xffffff800923eda5 -- e il sorgente scrive le parentesi.
	 * "9108e100 add"@0xffffff8008a54ee4 */
	wait_queue_head_t inq;
	/* +592 puntatore. L=25 S=5. "f9012a7f str"@0xffffff8008a718c8 */
	void *c592;
	/* +600 32 bit. L=3 S=1. "b90259c3 str"@0xffffff8008a71378 */
	int c600;
	/* +604 32 bit. L=3 S=1. "b9025dc4 str"@0xffffff8008a7137c */
	int c604;
	/* +608 32 bit. L=0 S=1. "b90261c5 str"@0xffffff8008a71380 */
	int c608;
	/*
	 * +612 32 bit. L=8 S=1. "b9426500 ldr"@0xffffff8008a53824 (fuso con
	 * +616: "f9000169 str"@0xffffff8008a55904 scrive 0x0000006400000002,
	 * cioe' 612 = 2 e 616 = 100 in un colpo solo).
	 * E' il modo di reset passato a `ilitek_tddi_reset_ctrl`
	 * ("b9426500 ldr"@0xffffff8008a53824 e "b9426500 ldr"@0xffffff8008a539e8).
	 */
	int c612;
	/* +616 32 bit CON SEGNO (`ldrsw`). L=4 S=0.
	 * "b9426903 ldr"@0xffffff8008a55d2c. Vale 100 dopo `ilitek_tddi_init`. */
	s32 c616;
	/*
	 * +620 32 bit. L=3 S=1. "b9026d1f str"@0xffffff8008a55948.
	 * E' la scelta fra riaggiornare il firmware e ripartire dal reset:
	 * `ilitek_tddi_mp_test_handler` e `ilitek_tddi_switch_tp_mode` lo
	 * confrontano con 1 ("b9426d09 ldr"@0xffffff8008a537f4,
	 * "b9426d09 ldr"@0xffffff8008a538f4).
	 */
	int c620;
	/*
	 * +624 32 bit. L=2 S=3. "b9027113 str"@0xffffff8008a60888.
	 * Il gruppo A non lo tocca.
	 *
	 * TRAPPOLA GIA' CADUTA UNA VOLTA IN QUESTO FILE, e vale la pena
	 * lasciarla scritta: in `ilitek_tddi_gesture_recovery` e in
	 * `ilitek_tddi_spi_recovery` compare "b9027013 str"@0xffffff8008a53efc,
	 * che objdump stampa come `str w19, [x0,#624]` -- ma quel `x0` e' gia'
	 * `idev + 112` (`ldr x21, [x0,#112]!`, pre-indice con riscrittura), e
	 * l'offset vero e' quindi 112 + 624 = 736, cioe' `c736`, non questo
	 * campo. E' esattamente la classe «un offset sbagliato di uno» a
	 * dimensione identica: nessuna misura la denuncerebbe.
	 */
	int c624;
	/* +628 un byte. L=1 S=1. "3909d11f strb"@0xffffff8008a55950 */
	u8 c628;
	/* +629 un byte. L=1 S=1. "3949d508 ldrb"@0xffffff8008a58b8c (fuso con
	 * +630 da "79000169 strh"@0xffffff8008a55020) */
	u8 c629;
	/* +630 un byte. L=1 S=2. "3909d91f strb"@0xffffff8008a53c64: azzerato
	 * in coda a `ilitek_tddi_reset_ctrl`. */
	u8 c630;
	/*
	 * +631 un byte. L=1 S=9. "3909dd13 strb"@0xffffff8008a53f28.
	 * Messo a 1 mentre `ilitek_tddi_gesture_recovery` e
	 * `ilitek_tddi_spi_recovery` girano, azzerato subito dopo
	 * ("3909dd1f strb"@0xffffff8008a53f3c).
	 */
	u8 c631;
	/* +632 un byte. L=1 S=2. "3909e11f strb"@0xffffff8008a547c8 */
	u8 c632;
	/*
	 * +633, 1 byte: BUCO DICHIARATO IGNOTO, incastrato fra due `u8`
	 * toccati. Zero istruzioni del blocco con questo offset immediato.
	 * NON e' riempimento: fra due byte non ce n'e' bisogno.
	 */
	u8 __ignoto_633;
	/* +634 un byte. L=1 S=0. "3949e908 ldrb"@0xffffff8008a544dc */
	u8 c634;
	/* +635 un byte. L=0 S=2. "3909ed09 strb"@0xffffff8008a5469c */
	u8 c635;
	/* +636 un byte. L=7 S=8. "3909f11f strb"@0xffffff8008a55138 */
	u8 c636;
	/* +637 un byte. L=3 S=2. "3909f509 strb"@0xffffff8008a5b8ac */
	u8 c637;
	/* +638 un byte. L=1 S=2. "3909f906 strb"@0xffffff8008a69444 */
	u8 c638;
	/* +639 un byte. L=1 S=2. "3909fd1f strb"@0xffffff8008a55024 */
	u8 c639;
	/*
	 * +640 un byte. L=0 S=6. "390a011f strb"@0xffffff8008a53bb0.
	 * `ilitek_tddi_reset_ctrl` lo azzera prima del reset e lo rimette a 1
	 * alla fine ("390a010a strb"@0xffffff8008a53c70), ma solo sul ramo che
	 * NON e' il code-reset.
	 */
	u8 c640;
	/* +641 un byte. L=3 S=4. "390a052a strb"@0xffffff8008a5a348 */
	u8 c641;
	/* +642 un byte. L=1 S=1. "390a0928 strb"@0xffffff8008a6d4f4 */
	u8 c642;
	/* +643 un byte. L=1 S=2. "390a0d09 strb"@0xffffff8008a6f914 */
	u8 c643;
	/* +644 32 bit. L=0 S=1. "b902851f str"@0xffffff8008a55100 */
	int c644;
	/* +648 32 bit. L=1 S=1. "b902890b str"@0xffffff8008a550f4.
	 * Riempimento 652..655 fino al campo a 8 byte a 656. */
	int c648;
	/* +656 64 bit. L=2 S=1. "f9014903 str"@0xffffff8008a550dc */
	u64 c656;
	/* +664 64 bit. L=1 S=1. "f9014d05 str"@0xffffff8008a550e0 */
	u64 c664;
	/* +672 64 bit. L=2 S=1. "f9015106 str"@0xffffff8008a550e4 */
	u64 c672;
	/* +680 64 bit. L=1 S=1. "f9015504 str"@0xffffff8008a550e8 */
	u64 c680;
	/* +688 64 bit. L=1 S=1. "f9015909 str"@0xffffff8008a550ec */
	u64 c688;
	/* +696 64 bit. L=2 S=1. "f9015d0a str"@0xffffff8008a550f0 */
	u64 c696;
	/* +704 32 bit. L=2 S=5. "b902c11f str"@0xffffff8008a54f18 */
	int c704;
	/*
	 * +708 32 bit. L=5 S=3. "b902c509 str"@0xffffff8008a53b64.
	 * Messo a 1 all'ingresso di `ilitek_tddi_reset_ctrl` e azzerato in
	 * coda ("b902c51f str"@0xffffff8008a53c6c).
	 */
	int c708;
	/*
	 * +712 32 bit. L=51 S=6 -- il campo piu' letto della struttura.
	 * "b902c91f str"@0xffffff8008a53c4c: `ilitek_tddi_reset_ctrl` lo
	 * azzera, ma SOLO sul ramo che non e' il code-reset.
	 */
	int c712;
	/*
	 * +716 32 bit. L=5 S=3. "b902cd09 str"@0xffffff8008a53a90.
	 * E' il "sto aggiornando il firmware": `ilitek_tddi_fw_upgrade_handler`
	 * lo mette a 1 all'ingresso e lo azzera in uscita
	 * ("b902cd1f str"@0xffffff8008a53b34), e
	 * `ilitek_tddi_mp_test_handler` rifiuta di partire se e' acceso
	 * ("b942cd09 ldr"@0xffffff8008a53710).
	 */
	int c716;
	/*
	 * +720 32 bit. L=4 S=3. "b902d109 str"@0xffffff8008a5373c.
	 * Il gemello del precedente per il collaudo MP: 1 all'ingresso di
	 * `ilitek_tddi_mp_test_handler`, 0 in uscita
	 * ("b902d11f str"@0xffffff8008a5384c).
	 */
	int c720;
	/* +724 32 bit. L=3 S=4. "b902d509 str"@0xffffff8008a542b4 */
	int c724;
	/*
	 * +728 32 bit. L=3 S=4. "b902d909 str"@0xffffff8008a53768.
	 * 1 all'ingresso di `ilitek_tddi_switch_tp_mode`, 0 in uscita
	 * ("b902d91f str"@0xffffff8008a539d4).
	 */
	int c728;
	/* +732 32 bit. L=4 S=17. "b902dd1f str"@0xffffff8008a54f30 */
	int c732;
	/*
	 * +736 32 bit. L=2 S=7. "b9027013 str"@0xffffff8008a53efc -- e
	 * l'indirizzo e' `x0 + 624` con `x0` gia' avanzato a `idev + 112`
	 * ("f8470c15 ldr"@0xffffff8008a53ef4, pre-indice con riscrittura):
	 * 112 + 624 = 736. Vedi la nota a `c624`.
	 * 1 all'ingresso di `ilitek_tddi_gesture_recovery` e
	 * `ilitek_tddi_spi_recovery`, 0 in uscita
	 * ("b902e11f str"@0xffffff8008a53f50).
	 * Riempimento 740..743 fino alla `completion`, allineata a 8.
	 */
	int c736;
	/*
	 * +744 `struct completion`, 32 byte: `done` a +744
	 * ("b902e91f str"@0xffffff8008a54f0c) e `wait` a +752, dove cade
	 * `__init_waitqueue_head` ("910ba100 add"@0xffffff8008a5539c).
	 *
	 * DIVERGENZA APERTA: e' inizializzata e SEGNALATA, mai attesa.
	 * `init_completion` in `ilitek_tddi_init`, `complete_all` una volta
	 * sola -- in `ilitek_tddi_wq_esd_check`
	 * ("910ba100 add"@0xffffff8008a5539c, "97db23e1 bl"@0xffffff8008a553a0
	 * verso <complete_all>) -- e `grep -c wait_for_completion` su tutto il
	 * blocco disassemblato da 0. O e' codice morto, o l'attesa sta in una
	 * funzione fuori dal confine delle 154. Questo lotto NON lo chiude: ha
	 * trovato chi la segnala, non chi la aspetta.
	 */
	struct completion c744;
	/*
	 * +776 puntatore a funzione = `ilitek_i2c_write`. L=88 S=1.
	 * "f901850a str"@0xffffff8008a558e0 (`probe`).
	 * La firma viene dai siti di chiamata: `ilitek_set_tp_data_len` lo
	 * chiama con x0 = buffer sulla pila e w1 = 2
	 * ("910013e0 add"@0xffffff8008a53e64, "321f03e1 orr"@0xffffff8008a53e68,
	 * "d63f0100 blr"@0xffffff8008a53e6c) e ne legge il risultato come
	 * `int` negativo in caso d'errore ("36f80180 tbz"@0xffffff8008a53e74).
	 * LIMITE DICHIARATO: la firma viene dall'UNICO sito di chiamata letto
	 * in questo lotto. Gli altri 87 non sono stati letti; se uno di essi
	 * passasse un terzo argomento, questa firma andrebbe corretta.
	 *
	 * CORREZIONE DEL LOTTO DEL GRUPPO B: il secondo argomento e' `int`,
	 * non `u32`. Il sito di chiamata non decide il segno -- passa la
	 * costante 2 -- mentre il CORPO si': `ilitek_i2c_write` indicizza con
	 * esso e l'indice e' esteso CON SEGNO
	 * ("3833ca97 strb"@0xffffff8008a55b30, cioe' `[x20, w19, sxtw]`), e
	 * cosi' le due estensioni a 64 bit
	 * ("93407d16 sxtw"@0xffffff8008a55ad4,
	 * "93407d02 sxtw"@0xffffff8008a55b10). Con `u32` lo stesso sorgente
	 * produce `uxtw`, verificato compilando: vedi B7 in `ilitek_bus.c`.
	 */
	int (*c776)(void *buf, int len);
	/*
	 * +784 puntatore a funzione = `ilitek_i2c_read`. L=21 S=1.
	 * "f9018909 str"@0xffffff8008a558d8. Nessuno dei 21 siti di chiamata
	 * cade nel gruppo A: la firma qui e' assunta simmetrica a quella di
	 * +776, cioe' e' UNA SCELTA e non una misura.
	 *
	 * Resta una scelta anche dopo il gruppo B: `ilitek_i2c_read` non
	 * indicizza niente col secondo argomento, quindi il suo corpo non ne
	 * decide il segno. Cio' che e' cambiato e' il termine di paragone --
	 * ora la simmetria e' con un campo MISURATO (+776, vedi sopra).
	 */
	int (*c784)(void *buf, int len);
	/*
	 * +792 puntatore a funzione. L=3 S=0.
	 * DIVERGENZA APERTA, E QUESTO LOTTO NON LA CHIUDE. Nessuna delle 154
	 * funzioni lo scrive: `ilitek_node_ioctl_write` (gruppo H) fa
	 * `ldr x9,[idev,#792]; ldr x0,[idev,#8]; blr x9`
	 * ("f9418d09 ldr"@0xffffff8008a6d934), e dopo il `devm_kzalloc` vale
	 * NULL. Anche il primo argomento (+8) e' azzerato esplicitamente in
	 * `probe` ("f900051f str"@0xffffff8008a558bc).
	 * La lettura piu' semplice e' che siano la coppia
	 * `spi` / `spi_write_then_read` di un trasporto SPI che questo binario
	 * compila e non innesta -- `core_spi_setup` esiste (gruppo B) e non li
	 * scrive. SE E' COSI', di fabbrica quel nodo /proc dereferenzia NULL.
	 * Non e' verificato sul telefono e non e' affermato qui: il gruppo H
	 * non e' stato letto in questo lotto.
	 *
	 * La FIRMA non e' scritta: al solo sito noto il `blr` e' preceduto da
	 * un unico `ldr x0`, il che dice che almeno un argomento c'e' e non
	 * dice quanti ce ne siano. Un prototipo inventato qui sarebbe «dati
	 * inventati» in un campo che nessuna misura di dimensione denuncia,
	 * quindi il campo e' un puntatore opaco finche' il gruppo H non sara'
	 * letto.
	 */
	void *c792;
	/* +800 puntatore a funzione. L=4 S=1. "f901911f str"@0xffffff8008a558fc
	 * (`probe` ci scrive `wzr`, NULL esplicito).
	 * `ilitek_tddi_wq_esd_spi_check` lo chiama senza argomenti e confronta
	 * il risultato con 0xa3 ("f9419108 ldr"@0xffffff8008a54018). */
	int (*c800)(void);
	/* +808 puntatore a funzione = `ilitek_tddi_move_mp_code_flash`. L=2 S=1.
	 * "f9019509 str"@0xffffff8008a55918. Chiamata senza argomenti da
	 * `ilitek_tddi_switch_tp_mode` ("f9419508 ldr"@0xffffff8008a53950). */
	int (*c808)(void);
	/* +816 puntatore a funzione = `ilitek_tddi_move_gesture_code_flash`.
	 * L=2 S=1. "f901990b str"@0xffffff8008a55920. Chiamata con un
	 * argomento, `c536` ("f941990b ldr"@0xffffff8008a53910,
	 * "b9421900 ldr"@0xffffff8008a53914). */
	int (*c816)(int mode);
	/* +824 puntatore a funzione = `ilitek_tddi_wq_esd_i2c_check`. L=1 S=1.
	 * "f9019d09 str"@0xffffff8008a55934. Chiamata senza argomenti da
	 * `ilitek_tddi_wq_esd_check` ("f9419d08 ldr"@0xffffff8008a55370). */
	int (*c824)(void);
	/*
	 * +832 64 bit. L=4 S=1. "f901a11f str"@0xffffff8008a558f8: l'unico
	 * store e' `wzr`, cioe' NULL, e tutte e quattro le letture sono
	 * protette da `cbz`. Il campo e' semplicemente sempre spento in questa
	 * configurazione -- a differenza di +792, qui la guardia c'e'.
	 */
	void *c832;
	/* +840 puntatore a funzione = `ilitek_tddi_touch_esd_gesture_flash`.
	 * L=2 S=1. "f901a50b str"@0xffffff8008a5593c. Chiamata senza argomenti
	 * da `ilitek_tddi_gesture_recovery` ("f941a509 ldr"@0xffffff8008a53f24). */
	int (*c840)(void);
	/*
	 * +848 array di puntatori a funzione, passo 8.
	 * "f941a908 ldr"@0xffffff8008a5c668.
	 *
	 * IL NUMERO DI ELEMENTI E' UNA SCELTA, NON UNA MISURA. L'indice e' un
	 * byte del pacchetto di rapporto (`ldrb w20,[x19,#44]!` in
	 * `demo_debug_info_mode`) e NON e' confrontato con nessun limite: il
	 * binario non dice quanti elementi ci siano. CINQUE e' il solo valore
	 * compatibile con sizeof = 888 *se* questo campo e' l'ultimo -- e «e'
	 * l'ultimo» e' a sua volta un'inferenza dal fatto che 848 e' l'offset
	 * piu' alto toccato dalle 154. Va letto come scelta.
	 *
	 * La FIRMA degli elementi non e' misurata in questo lotto (l'unico sito
	 * di chiamata sta nel gruppo E), quindi sono puntatori opachi: cinque
	 * per otto = quaranta byte, che e' cio' che la dimensione richiede.
	 */
	void *c848[5];
};

/*
 * ===========================================================================
 * I GLOBALI DEL DRIVER CHE NON STANNO NELLA STRUTTURA
 * ===========================================================================
 * La struttura non e' tutto lo stato: la scansione trova 170 indirizzi
 * globali distinti toccati fuori da essa. Qui sono dichiarati SOLO i sei che
 * il gruppo A tocca; gli altri sono un lotto successivo, non ancora fatto.
 *
 * Il puntatore alla struttura sta a 0xffffff800a0fca40, raggiunto ovunque
 * come `adrp xN, 0xffffff800a0fc000` piu' `ldr [xN,#2624]`
 * (2624 = 0xa40): "f94522c8 ldr"@0xffffff8008a5370c. Il NOME `idev` viene
 * dal binario, non da una convenzione scelta qui.
 *
 * LA DISPOSIZIONE DEI DUE BLOCCHI DI LAVORO E' MISURATA, E LA MISURA
 * CORREGGE IL RAPPORTO PRECEDENTE. `rapporto-ilitek-struttura.md` §9.5
 * chiama `timer_list` gli oggetti a 0xffffff800a0fc968 e 0xffffff800a0fc9d0.
 * Sono `struct delayed_work`, e sono due prove indipendenti:
 *  - `ilitek_tddi_wq_ctrl` li passa a `cancel_delayed_work_sync`, che prende
 *    un `struct delayed_work *` ("9125a000 add"@0xffffff8008a54214 +
 *    "97d9f281 bl"@0xffffff8008a54218; "91274000 add"@0xffffff8008a54250 +
 *    "97d9f272 bl"@0xffffff8008a54254);
 *  - il PASSO fra i due blocchi e' 0xa0fc9c8 - 0xa0fc968 = 96, ed e'
 *    esattamente `sizeof(struct delayed_work)` MISURATO con una sonda
 *    compilata con la riga di comando vera del sottoalbero:
 *      sonda_sizeof_delayed_work = 0x60 = 96
 *      sonda_sizeof_timer_list   = 0x30 = 48
 *    Con `timer_list` il conto non tornerebbe. E' un controllo che poteva
 *    fallire e non e' fallito.
 */

/* 0xffffff800a0fc958 (pagina + 2392). Un byte: `ldrb`, mai letto piu' largo.
 * E' la guardia dei messaggi di livello KERN_INFO che il driver stampa solo
 * quando il diagnostico e' acceso: "39656108 ldrb"@0xffffff8008a537a8,
 * "39656269 ldrb"@0xffffff8008a540b8, e altri 130 siti nel blocco.
 * NOME SCELTO -- il binario non lo da'. */
extern u8 ilitek_dbg_en;

/* 0xffffff800a0fc960 (pagina + 2400): `struct workqueue_struct *`.
 * "f944b288 ldr"@0xffffff8008a54098, passato a `flush_workqueue`
 * ("97d9ee2d bl"@0xffffff8008a54220) e a `destroy_workqueue`
 * ("97d9f2b5 bl"@0xffffff8008a5529c). NOME SCELTO. */
extern struct workqueue_struct *ilitek_wq_esd;

/* 0xffffff800a0fc968 (pagina + 2408): `struct delayed_work`, 96 byte.
 * NOME SCELTO. */
extern struct delayed_work ilitek_esd_work;

/* 0xffffff800a0fc9c8 (pagina + 2504): `struct workqueue_struct *`.
 * "f944e688 ldr"@0xffffff8008a54130. NOME SCELTO. */
extern struct workqueue_struct *ilitek_wq_bat;

/* 0xffffff800a0fc9d0 (pagina + 2512): `struct delayed_work`, 96 byte.
 * NOME SCELTO. */
extern struct delayed_work ilitek_bat_work;

/* 0xffffff800a0fca40 (pagina + 2624). NOME DAL BINARIO. */
extern struct ilitek_tddi_dev *idev;

/*
 * ===========================================================================
 * LA MACRO DI LOG: TRE FORME, TUTTE E TRE LETTE DAL BINARIO
 * ===========================================================================
 * Ogni `printk` del blocco porta tre argomenti risolvibili: x0 il formato,
 * x1 il `__func__`, w2 il `__LINE__`, e il formato ha sempre la testa
 * "ILITEK: (%s, %d): " preceduta dal livello. Esempio letterale:
 *
 *   ffffff8008a53864:	d0003f40 	adrp	x0, ffffff800923d000
 *   ffffff8008a53868:	d0003f41 	adrp	x1, ffffff800923d000
 *   ffffff8008a5386c:	913f6800 	add	x0, x0, #0xfda
 *   ffffff8008a53870:	913ef821 	add	x1, x1, #0xfbe
 *   ffffff8008a53874:	52800cc2 	mov	w2, #0x66                  // #102
 *   ffffff8008a53878:	97db7f17 	bl	<printk>
 *
 * -> "\x013ILITEK: (%s, %d): Switch MP mode failed\n"@0xffffff800923dfda,
 *    __func__ = "ilitek_tddi_mp_test_handler"@0xffffff800923dfbe,
 *    __LINE__ = 102 ("52800cc2 mov"@0xffffff8008a53874).
 *
 * Le tre forme si distinguono per il livello e per la guardia:
 *   \x013 = KERN_ERR,  mai protetto dal byte diagnostico   -> ILI_ERR
 *   \x016 = KERN_INFO, non protetto                        -> ILI_INFO
 *   \x016 = KERN_INFO, protetto da `ilitek_dbg_en`         -> ILI_DBG
 * (esiste anche \x01c = KERN_CONT, in 42 siti, tutti fuori dal gruppo A:
 * sono le stampe di dump grezzo, che non usano questa macro -- vedi i 26
 * siti su 1095 che lo scanner delle printk non risolve, e che sono
 * esattamente quelli.)
 *
 * La ripartizione fra ILI_INFO e ILI_DBG e' stata letta sito per sito nel
 * grafo dei salti delle funzioni scritte, NON con una finestra di
 * istruzioni: una finestra sbaglia -- provata, da' 27 falsi positivi -- e
 * un falso positivo qui e' una differenza di comportamento invisibile alla
 * misura di dimensione.
 *
 * `ILI_DBG` e' scritta con la guardia FUORI dalla chiamata perche' e' cosi'
 * che il binario la porta: il `cbz` salta la costruzione degli argomenti,
 * non solo la `printk`.
 */
#define ILI_ERR(fmt, ...) \
	printk(KERN_ERR "ILITEK: (%s, %d): " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define ILI_INFO(fmt, ...) \
	printk(KERN_INFO "ILITEK: (%s, %d): " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define ILI_DBG(fmt, ...)						\
	do {								\
		if (ilitek_dbg_en)					\
			printk(KERN_INFO "ILITEK: (%s, %d): " fmt,	\
			       __func__, __LINE__, ##__VA_ARGS__);	\
	} while (0)

/*
 * ===========================================================================
 * LE FUNZIONI GLOBALI DEL GRUPPO A (`ilitek_main.c`)
 * ===========================================================================
 * Sono globali quelle che `oracolo/stock.map` marca `T`: quindici su
 * diciassette. Le due `t` -- `ilitek_tddi_wq_esd_check` e
 * `ilitek_tddi_wq_bat_check` -- sono `static` nel sorgente e non compaiono
 * qui.
 *
 * Le firme vengono dai siti di chiamata, non da un'intuizione. Dove il
 * binario non decide -- per esempio se un parametro sia `int` o `u8` --
 * il commento lo dice.
 */
int ilitek_tddi_mp_test_handler(char *apk, bool lcm_on);
int ilitek_tddi_switch_tp_mode(u8 mode);
int ilitek_tddi_fw_upgrade_handler(void);
int ilitek_tddi_reset_ctrl(int mode);
int ilitek_set_tp_data_len(int format);
int ilitek_tddi_gesture_recovery(void);
void ilitek_tddi_spi_recovery(void);
int ilitek_tddi_wq_esd_spi_check(void);
int ilitek_tddi_wq_esd_i2c_check(void);
void ilitek_tddi_wq_ctrl(int type, int ctrl);
int ilitek_tddi_dev_init(struct ilitek_hwif_info *hwif);
void ilitek_tddi_dev_remove(void);

/*
 * NON ANCORA SCRITTE -- il motivo sta nel cappello di `ilitek_main.c` -- ma
 * DICHIARATE, perche' il gruppo C le chiama e un prototipo mancante e' un
 * errore di compilazione che nasconde il vero stato del lavoro. Le firme
 * vengono dai siti di chiamata del gruppo C:
 *   `ilitek_tddi_sleep_handler` e' chiamata con 0 da `tpd_suspend`
 *   ("2a1f03e0 mov"@0xffffff8008a56654) e con 2 da `tpd_resume`
 *   ("321f03e0 orr"@0xffffff8008a56690), e il risultato e' provato sul solo
 *   bit di segno ("37f80060 tbnz"@0xffffff8008a5665c).
 *   `ilitek_tddi_report_handler` e' chiamata senza argomenti da
 *   `ilitek_plat_isr_bottom_half` ("97fff879 bl"@0xffffff8008a56548) e il
 *   risultato non e' usato.
 *   `ilitek_tddi_init` e' chiamata senza argomenti da `ilitek_plat_probe`
 *   ("97fff96f bl"@0xffffff8008a5688c) e provata sul bit di segno.
 */
int ilitek_tddi_sleep_handler(int mode);
void ilitek_tddi_report_handler(void);
int ilitek_tddi_init(void);

/*
 * ===========================================================================
 * LE FUNZIONI GLOBALI DEL GRUPPO C (`ilitek_plat.c`)
 * ===========================================================================
 * Sei su quattordici sono `T` nella mappa; le altre otto sono `static` e non
 * compaiono qui. Nessuna riceve argomenti e nessuna restituisce niente --
 * lo dicono i loro prologhi, che non toccano x0 in ingresso, tranne
 * `ilitek_plat_irq_register`, che lo salva subito
 * ("2a0003f3 mov"@0xffffff8008a5627c).
 */
void ilitek_plat_tp_reset(void);
void ilitek_plat_input_register(void);
void ilitek_plat_irq_disable(void);
void ilitek_plat_irq_enable(void);
void ilitek_plat_irq_unregister(void);
int ilitek_plat_irq_register(int type);


#endif /* __ILITEK_H */
