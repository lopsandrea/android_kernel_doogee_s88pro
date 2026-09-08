// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_ILITEK_E977 -- il GRUPPO B, il bus I2C. Doogee S88 Pro.
 *
 * Sette funzioni, 1704 byte, 0xffffff8008a55664..0xffffff8008a55d0c. Il
 * confine e' quello di `docs/bringup/rapporti/rapporto-ilitek-confine-e-unita.md`
 * §2.3, che ha retto la rilettura di §9 di HANDOFF.md senza una modifica.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA. Nessun sorgente
 * pubblico e' stato letto. Ogni riga derivata porta accanto la riga di
 * disassemblato che la giustifica, nella forma "<codifica> <mnemonico>"@0xINDIRIZZO
 * -- la stessa di `ilitek.h`, quella che `tools/verificaistruzioni.py`
 * sa confrontare col binario.
 *
 * COME SI RIVERIFICA, e perche' servono DUE intervalli:
 *
 *   ./venv/bin/python3 verificacitazioni.py ilitek_bus.c oracolo/stock.elf \
 *       --eccezione ""
 *   ./venv/bin/python3 verificaistruzioni.py ilitek_bus.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a55664:0xffffff8008a55d0c \
 *       --intervallo 0xffffff8008a536f8:0xffffff8008a55664 \
 *       --controfattuale "38334a97 strb"
 *
 * Esito atteso, al 2026-08-21: 20 letterali, 27 citati, 26 verificati, 25
 * probanti, 1 debole; 228 citazioni di istruzione, 227 confermate, 0
 * assenti, 1 controfattuale dichiarata.
 *
 * La `--controfattuale` e' la citazione di B7: "38334a97 strb" e' cio' che
 * il NOSTRO compilatore produceva quando `len` era `u32`, e NON sta nel
 * binario di fabbrica. E' citata apposta -- e' la prova della correzione --
 * quindi va dichiarata, non tolta: senza la dichiarazione lo strumento la
 * segnala ASSENTE, e ha ragione.
 *
 * Il secondo intervallo e' il GRUPPO A. Una sola citazione di questo file
 * ci cade dentro -- "39400008 ldrb"@0xffffff8008a55310, il confronto con
 * 0x18 che `ilitek_tddi_dev_init` fa sullo stesso campo -- ed e' citata
 * apposta, perche' e' la seconda prova indipendente che `hwif+0` e' il tipo
 * di interfaccia. Col solo intervallo del gruppo B esce ASSENTE, e sarebbe
 * un falso allarme.
 *
 * L'`--eccezione ""` copre il formato a 0xffffff800923e290, il messaggio
 * VUOTO di `ilitek_i2c_remove` (vedi B4): lo strumento ancora le citazioni
 * a un letterale del codice, e il letterale qui e' la stringa vuota, che
 * non puo' ancorare niente. E' un limite dichiarato dello strumento su un
 * caso vero, non una citazione mancante.
 *
 * ===========================================================================
 * PERCHE' QUESTO FILE E' STATO RISCRITTO DA CAPO
 * ===========================================================================
 * Un lotto del 2026-08-18 (ramo `wip/ilitek-2026-08-18`) aveva gia' scritto
 * queste sette funzioni. Quel lotto NON e' stato ripreso, ed e' stato usato
 * solo come mappa di dove guardare. Le ragioni sono misurate, non di gusto:
 *
 *   - `tools/verificacitazioni.py` su quel file: «letterali: 18  citati: 0».
 *     Le citazioni erano nella forma "/ * 33: adrp x8, ... * /", un numero di
 *     riga dentro un listato effimero: nessuno strumento del progetto le
 *     puo' confrontare col binario, quindi nessuna di esse e' una prova.
 *   - `tools/verificaistruzioni.py` sullo stesso file, con il confine del
 *     gruppo B: «citazioni di istruzione trovate nel sorgente: 0».
 *   - Gli argomenti di `printk` erano INVERTITI. Quel lotto leggeva x0 come
 *     il nome della funzione e x1 come il messaggio; il binario dice il
 *     contrario, e le stringhe lo dimostrano da sole:
 *       0xffffff800923f209 = "\x013ILITEK: (%s, %d): Not support this interface\n"
 *       0xffffff800923f239 = "core_spi_setup"
 *     cioe' x0 e' il FORMATO e x1 e' il `__func__`. E' la macro `ILI_ERR`
 *     che `ilitek.h` aveva gia' letto e definito.
 *   - I tre `struct kmem_cache *` dichiarati come globali del driver non
 *     esistono: sono `kmalloc_caches[N]`, l'indirizzo che il compilatore
 *     mette al posto di `kzalloc(costante, ...)`. Il passo lo prova --
 *     1672, 1688, 1712 distano 16 e 24 byte, cioe' 2 e 3 puntatori, ed e'
 *     esattamente la distanza fra gli indici 6, 8 e 11 dell'array, che sono
 *     le taglie 64, 256 e 2048 richieste dalle tre allocazioni (56, 240 e
 *     2048 byte).
 *   - `ilitek_tddi_dev_init` vi era ridefinita, mentre sta gia' in
 *     `ilitek_main.c` alla riga 1051.
 *
 * `struct ilitek_hwif_info` sta in `ilitek.h`: il gruppo B la COMPLETA (era
 * una dichiarazione incompleta), il gruppo C la DEFINISCE come oggetto, e
 * servono entrambi.
 *
 * ===========================================================================
 * LA STRUTTURA CONTENITORE DA 240 BYTE
 * ===========================================================================
 * `ilitek_tddi_interface_dev_init` alloca 240 byte
 * ("321c0fe2 orr"@0xffffff8008a556b0, 0xf0 = 240) e li registra come
 * `i2c_driver`, ma `sizeof(struct i2c_driver)` in questo albero e' 232 --
 * misurato con una sonda compilata con la riga di comando vera, vedi
 * `rapporto-ilitek-confine-e-unita.md` §3. Gli 8 byte in piu' sono usati:
 * "f9007413 str"@0xffffff8008a5570c scrive la `hwif` a +232, e
 * `ilitek_i2c_probe` la rilegge passando per `client->dev.driver`
 * ("f9405415 ldr"@0xffffff8008a55774 e "f94056ab ldr"@0xffffff8008a558c4).
 * E' un `container_of` scritto a mano, e qui e' reso come tale invece che
 * con una scrittura a offset grezzo: la forma con i campi nominati e' la
 * sola che il compilatore possa ricontrollare.
 *
 * Gli offset dentro `struct i2c_driver` non sono attesi ma misurati dalla
 * stessa sonda: probe +16, remove +24, driver +64, id_table +184, e
 * `offsetof(i2c_client, dev) + offsetof(device, driver)` = 168.
 *
 * ===========================================================================
 * LE DIVERGENZE APERTE DI QUESTO FILE
 * ===========================================================================
 * B1. I `__LINE__`, come in `ilitek_main.c` (divergenza D1 di quel file).
 *     I numeri di fabbrica sono noti uno per uno e annotati sopra ogni
 *     funzione; questo file NON li riproduce, perche' imbottire di righe
 *     vuote un file che verra' ancora toccato sposta tutto al primo cambio.
 *     E' una divergenza reale nel binario e INVISIBILE alla misura di
 *     dimensione: `mov w2,#imm16` e' una istruzione per qualunque riga.
 * B2. La lunghezza della `memcpy` di `ilitek_i2c_write`. Il binario emette
 *     un minimo CON SEGNO fra `msg.len` (u16, riletto dalla pila) e `len`
 *     ("6b13011f cmp"@0xffffff8008a55b08 piu'
 *     "1a93b108 csel"@0xffffff8008a55b0c). Qui e' reso con
 *     `min_t(int, msg.len, len)`, che riproduce le tre istruzioni; ma
 *     QUALE forma sorgente la fabbrica avesse NON e' deciso dal binario --
 *     un `min()` esplicito e un `memcpy(..., msgs[0].len)` con un altro
 *     controllo davanti danno lo stesso codice. Riprodotto il
 *     comportamento, dichiarata la non-unicita'.
 *     Da notare che senza il minimo il codice e' PIU' CORTO: la prima
 *     stesura scriveva `memcpy(mpbuf, buf, msg.len)` e misurava 460 byte
 *     contro i 476 di fabbrica. Le quattro istruzioni mancanti erano
 *     esattamente queste tre piu' la `sxtw` di B7.
 * B3. Il campo `c56`. `ilitek_i2c_write` ne legge il primo `u32` e lo
 *     confronta dopo uno scorrimento di 10
 *     ("530a7d29 lsr"@0xffffff8008a55aa0, "7105053f cmp"@0xffffff8008a55aa4).
 *     Cosa sia quel valore NON e' deciso da questo gruppo: `ilitek.h` lo
 *     tiene `void *`, e qui e' letto attraverso un cast dichiarato invece
 *     che dandogli un tipo che nessuna misura sostiene.
 * B4. `ilitek_i2c_remove` stampa un messaggio VUOTO: il formato a
 *     0xffffff800923e290 e' "\x016ILITEK: (%s, %d): " e finisce li'. Non e'
 *     una lettura parziale -- il NUL e' subito dopo. E' `ILI_INFO("")`, e
 *     si riporta com'e'.
 * B5. Il campo `hwif->c32`. Il sito di chiamata ne decide la firma per
 *     intero -- un argomento, la `hwif` stessa, e un `int` di ritorno usato
 *     come ritorno della `probe` -- ma NESSUNA delle 154 funzioni lo
 *     scrive: chi lo riempie sta fuori dal confine, ed e' la piattaforma
 *     (gruppo C) a costruire la `hwif`. Il campo ha quindi un tipo
 *     misurato e nessun nome. Si chiudera' scrivendo il gruppo C.
 * B7. `len` E' `int`, NON `u32`, e questo file lo misura mentre `ilitek.h`
 *     lo aveva SCELTO. L'header tipa i campi +776 e +784 come
 *     `int (*)(void *buf, u32 len)` dichiarando il limite: la firma veniva
 *     dall'unico sito di chiamata letto, che passa `w1 = 2` e non dice
 *     nulla sul segno. Il corpo lo dice: `mpbuf[len]` compila in
 *     "3833ca97 strb"@0xffffff8008a55b30, cioe' `[x20, w19, sxtw]` --
 *     indice ESTESO CON SEGNO. Con `u32` lo stesso sorgente da' `uxtw`,
 *     verificato: la prima stesura di questo file produceva
 *     "38334a97 strb", che differisce di un bit dal binario di fabbrica.
 *     Stessa prova sulle due "93407d16 sxtw"@0xffffff8008a55ad4 e
 *     "93407d02 sxtw"@0xffffff8008a55b10.
 *     `ilitek.h` e' stato corretto di conseguenza. Per +784
 *     (`ilitek_i2c_read`) la misura NON esiste -- il corpo non indicizza
 *     niente -- e resta una scelta, ma ora e' una scelta simmetrica a un
 *     campo MISURATO invece che a un campo scelto.
 * B6. IL `%ld` SENZA ARGOMENTO. Il formato a 0xffffff800923f3d6 dichiara
 *     tre conversioni (%s, %d, %ld) e il binario passa due argomenti:
 *     fra "910f5800 add"@0xffffff8008a55980 e "97db76d2 bl"@0xffffff8008a5598c
 *     nessuna istruzione tocca x3 o w3. La fabbrica stampa spazzatura al
 *     posto del terzo valore. E' riprodotto com'e'; se il compilatore di
 *     questo albero rifiuta la chiamata, la correzione minima e' passare
 *     `PTR_ERR(idev)` -- UNA istruzione in piu' del binario, e va
 *     dichiarata qui invece che nascosta.
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include "ilitek.h"

/*
 * La struttura contenitore di 240 byte: la `i2c_driver` e, subito dopo, il
 * puntatore alla `hwif`. Vedi il cappello.
 */
struct ilitek_i2c_drv {
	struct i2c_driver drv;			/* +0,   232 byte misurati */
	struct ilitek_hwif_info *hwif;		/* +232 */
};

/*
 * ===========================================================================
 * LE FUNZIONI DEGLI ALTRI GRUPPI CHE QUESTO FILE CHIAMA
 * ===========================================================================
 * Stanno in unita' non ancora scritte, quindi qui ci sono solo i prototipi.
 * NON SONO STUB: dichiararle e lasciarle indefinite fa fallire il LINK, che
 * e' l'esito onesto; darne un corpo «segnaposto» le farebbe passare per
 * scritte. `rapporto-ilitek-lotto1.md` §3.2 lo dice gia' per tutto il
 * driver: nessuno degli otto gruppi linka da solo.
 *
 * Le firme NON sono inventate. Tre vengono dai tipi che `ilitek.h` ha gia'
 * misurato sui campi a cui `probe` le assegna (+808, +816, +840); le altre
 * due dal solo sito di chiamata che questo file contiene, e il commento
 * dice quanto quel sito decide davvero.
 */
/* +808, "int (*c808)(void)" in ilitek.h. Gruppo E. */
int ilitek_tddi_move_mp_code_flash(void);
/* +816, "int (*c816)(int mode)" in ilitek.h. Gruppo E. */
int ilitek_tddi_move_gesture_code_flash(int mode);
/* +840, "int (*c840)(void)" in ilitek.h. Gruppo E. */
int ilitek_tddi_touch_esd_gesture_flash(void);
/*
 * Gruppo E. "940018b2 bl"@0xffffff8008a55acc: due argomenti, il buffer e la
 * lunghezza. Del RITORNO il sito decide solo che il byte basso serve
 * ("2a0003f7 mov"@0xffffff8008a55adc poi "3833ca97 strb"@0xffffff8008a55b30):
 * `u8` e' la lettura piu' stretta compatibile, e non e' l'unica.
 */
u8 ilitek_calc_packet_checksum(void *buf, u32 len);
/*
 * Gruppo E. "940013e8 bl"@0xffffff8008a55b38: cinque argomenti, dei quali
 * il sito fissa solo le CLASSI -- x0 e x4 puntatori, w1/w2/w3 interi a 32
 * bit -- e il ritorno non e' usato. I nomi dei parametri sono posizionali
 * apposta: nessuna misura dice cosa siano il 8, il len+1 e lo 0.
 */
void ilitek_dump_data(void *dato, int a, int b, int c, const char *etichetta);

/*
 * QUATTRO DELLE SETTE SONO `static`, E LO DICE LA MAPPA, non una scelta:
 * `oracolo/stock.map` le marca `t` minuscolo -- `ilitek_i2c_probe`,
 * `ilitek_i2c_remove`, `ilitek_i2c_write`, `ilitek_i2c_read` -- mentre
 * `core_spi_setup`, `ilitek_tddi_interface_dev_init` e
 * `ilitek_tddi_interface_dev_exit` sono `T`.
 */
static int ilitek_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id);
static int ilitek_i2c_remove(struct i2c_client *client);
static int ilitek_i2c_write(void *buf, int len);
static int ilitek_i2c_read(void *buf, int len);

/*
 * La tabella `id_table`, in `.rodata` a 0xffffff8008f7ea58
 * ("9129616b add"@0xffffff8008a556f8 su "b000294b adrp"@0xffffff8008a556e4).
 * Il CONTENUTO e' letto dai byte, non dedotto: a quell'indirizzo ci sono
 * "ILITEK_TDDI"@0xffffff8008f7ea58 seguito da zeri fino a +64, cioe' un
 * `name` di 11 caratteri, `driver_data` a 0, e la voce terminatrice tutta
 * a zero (sizeof(struct i2c_device_id) = 32: 20 di nome, 4 di riempimento,
 * 8 di dato).
 */
static const struct i2c_device_id ilitek_i2c_id[] = {
	{ "ILITEK_TDDI", 0 },
	{ },
};

/*
 * ===========================================================================
 * 1) core_spi_setup -- 0xffffff8008a55664, 44 byte
 * fabbrica: riga 197
 * ===========================================================================
 * Il ramo SPI non e' implementato di fabbrica: stampa e restituisce 0.
 */
int core_spi_setup(void)
{
	/*
	 * "\x013ILITEK: (%s, %d): Not support this interface\n"@0xffffff800923f209,
	 * __func__ = "core_spi_setup"@0xffffff800923f239,
	 * __LINE__ = 197 ("528018a2 mov"@0xffffff8008a5567c).
	 */
#line 197
	ILI_ERR("Not support this interface\n");

	/* "2a1f03e0 mov"@0xffffff8008a55684 -- w0 = wzr, cioe' zero. */
	return 0;
}

/*
 * ===========================================================================
 * 2) ilitek_tddi_interface_dev_init -- 0xffffff8008a55690, 212 byte
 * fabbrica: righe 306, 311
 * ===========================================================================
 */
int ilitek_tddi_interface_dev_init(struct ilitek_hwif_info *hwif)
{
	struct ilitek_i2c_drv *d;

	/*
	 * 240 byte azzerati, GFP_KERNEL. Il compilatore risolve la taglia
	 * costante in `kmalloc_caches[8]` (256) e chiama
	 * `kmem_cache_alloc_trace`: "d00084c8 adrp"@0xffffff8008a5569c,
	 * "f9434d08 ldr"@0xffffff8008a556a0 (+1688),
	 * "321c0fe2 orr"@0xffffff8008a556b0 (0xf0 = 240),
	 * "97dfe57d bl"@0xffffff8008a556b8.
	 * Il gfp e' 0x014080c0 ("52901801 mov"@0xffffff8008a556a4 piu'
	 * "72a02801 movk"@0xffffff8008a556ac), cioe' GFP_KERNEL | __GFP_ZERO.
	 */
	d = kzalloc(sizeof(*d), GFP_KERNEL);
	/* "b4000300 cbz"@0xffffff8008a556bc */
	if (!d) {
		/*
		 * "\x013ILITEK: (%s, %d): faied to allocate i2c_driver\n"@0xffffff800923f248
		 * -- il refuso «faied» e' di fabbrica e si riporta com'e'.
		 * __func__ = "ilitek_tddi_interface_dev_init"@0xffffff800923f27a,
		 * __LINE__ = 306 ("52802642 mov"@0xffffff8008a5572c).
		 */
#line 306
		ILI_ERR("faied to allocate i2c_driver\n");
		/* "12800160 mov"@0xffffff8008a55734 -- -12 = -ENOMEM */
		return -ENOMEM;
	}

	/*
	 * "39400268 ldrb"@0xffffff8008a556c0, "7100611f cmp"@0xffffff8008a556c4,
	 * "540003a1 b.ne"@0xffffff8008a556c8. 0x18 e' I2C.
	 */
	if (hwif->type != 0x18) {
		/*
		 * "\x013ILITEK: (%s, %d): Not I2C dev\n"@0xffffff800923f299,
		 * __LINE__ = 311 ("528026e2 mov"@0xffffff8008a5574c).
		 *
		 * NESSUNA `kfree` SU QUESTO RAMO. Fra il `b.ne` e il `ret`
		 * il binario esegue solo adrp/add/mov/bl printk e
		 * "128002a0 mov"@0xffffff8008a55754: la memoria appena
		 * allocata E' PERSA di fabbrica. E' una perdita vera, e si
		 * riproduce -- aggiungere qui la `kfree` che manca sarebbe
		 * codice che nel binario non c'e'.
		 */
#line 311
		ILI_ERR("Not I2C dev\n");
		/* "128002a0 mov"@0xffffff8008a55754 -- -22 = -EINVAL */
		return -EINVAL;
	}

	/* "f9001a60 str"@0xffffff8008a556d0 -- hwif+48 */
	hwif->driver = &d->drv;

	/* "f9400668 ldr"@0xffffff8008a556cc + "f9002008 str"@0xffffff8008a556dc */
	d->drv.driver.name = hwif->name;
	/* "f9400a68 ldr"@0xffffff8008a556e0 + "f9002808 str"@0xffffff8008a556f0 */
	d->drv.driver.owner = hwif->owner;
	/* "f9400e68 ldr"@0xffffff8008a556f4 + "f9003408 str"@0xffffff8008a55708 */
	d->drv.driver.of_match_table = hwif->of_match_table;

	/*
	 * I due puntatori a funzione escono da un unico `stp`:
	 * "a9012809 stp"@0xffffff8008a55700 su +16 e +24. Gli indirizzi sono
	 * "911d9129 add"@0xffffff8008a556e8 -> 0xffffff8008a55764 = ilitek_i2c_probe
	 * e "9127414a add"@0xffffff8008a556ec -> 0xffffff8008a559d0 = ilitek_i2c_remove.
	 */
	d->drv.probe = ilitek_i2c_probe;
	d->drv.remove = ilitek_i2c_remove;

	/* "f9005c0b str"@0xffffff8008a55704 -- +184 */
	d->drv.id_table = ilitek_i2c_id;

	/* "f9007413 str"@0xffffff8008a5570c -- +232, il campo in piu' */
	d->hwif = hwif;

	/*
	 * "aa1f03e0 mov"@0xffffff8008a55710 mette x0 a zero (l'`owner` NULL)
	 * e "9400d3a9 bl"@0xffffff8008a55714 chiama i2c_register_driver.
	 * IL VALORE DI RITORNO E' QUELLO DELLA CHIAMATA: dopo la `bl` c'e'
	 * "14000010 b"@0xffffff8008a55718 verso l'epilogo, che non tocca w0.
	 */
	return i2c_register_driver(NULL, &d->drv);
}

/*
 * ===========================================================================
 * 3) ilitek_i2c_probe -- 0xffffff8008a55764, 620 byte
 * fabbrica: righe 207..239
 * ===========================================================================
 */
static int ilitek_i2c_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	struct device_driver *drv;

	/*
	 * "f9405415 ldr"@0xffffff8008a55774 -- client+168, cioe'
	 * `client->dev.driver` (168 = offsetof(i2c_client, dev) +
	 * offsetof(device, driver), misurato). Letto PRIMA di ogni controllo
	 * su `client`, ed e' cosi' anche qui.
	 *
	 * QUI SI FERMA. La `hwif` NON viene calcolata adesso: il binario la
	 * ricava DUE VOLTE piu' avanti, a "f94056ab ldr"@0xffffff8008a558c4 e
	 * a "f94056a8 ldr"@0xffffff8008a55968, entrambe da `drv+168`. Una
	 * variabile locale calcolata qui produrrebbe una lettura sola e
	 * verrebbe riusata, che e' esattamente cio' che il binario NON fa --
	 * ed e' misurabile: la prima stesura la teneva in un locale e il
	 * confronto delle dimensioni dava 608 contro 620.
	 *
	 * Da `drv` si risale alla struttura contenitore: il `device_driver`
	 * sta a +64 della `i2c_driver`, quindi `drv - 64` e' la base dei 240
	 * byte e `+232` e' la `hwif`.
	 */
	drv = client->dev.driver;

	/*
	 * "\x016ILITEK: (%s, %d): ilitek i2c probe\n"@0xffffff800923f2fd,
	 * __func__ = "ilitek_i2c_probe"@0xffffff800923f323,
	 * __LINE__ = 207 ("528019e2 mov"@0xffffff8008a5578c).
	 */
#line 207
	ILI_INFO("ilitek i2c probe\n");

	/* "b4000313 cbz"@0xffffff8008a55794 */
	if (!client) {
		/*
		 * "\x013ILITEK: (%s, %d): i2c client is NULL\n"@0xffffff800923f334,
		 * __LINE__ = 210 ("52801a42 mov"@0xffffff8008a55804).
		 */
#line 210
		ILI_ERR("i2c client is NULL\n");
		/* "12800240 mov"@0xffffff8008a5580c -- -19 = -ENODEV */
		return -ENODEV;
	}

	/*
	 * "79400668 ldrh"@0xffffff8008a55798 -- client+2 = `client->addr`,
	 * "7101051f cmp"@0xffffff8008a5579c contro 0x41 = 65.
	 */
	if (client->addr != 0x41) {
		/*
		 * "52800828 mov"@0xffffff8008a557ac carica 0x41 e
		 * "79000668 strh"@0xffffff8008a557c0 lo scrive in client+2:
		 * l'indirizzo e' CORRETTO, non solo segnalato. Lo store
		 * avviene PRIMA della printk.
		 */
		client->addr = 0x41;
		/*
		 * "\x016ILITEK: (%s, %d): i2c addr doesn't be set up, use default : 0x%x\n"@0xffffff800923f35c,
		 * __LINE__ = 216 ("52801b02 mov"@0xffffff8008a557b8),
		 * e il quarto argomento e' di nuovo 0x41
		 * ("52800823 mov"@0xffffff8008a557bc).
		 */
#line 215
		ILI_INFO("i2c addr doesn't be set up, use default : 0x%x\n",
			 0x41);
	}

	/*
	 * `i2c_check_functionality` e' `static inline` e viene incorporata:
	 * "f9400e60 ldr"@0xffffff8008a557c8 (client+24 = `adapter`),
	 * "f9400808 ldr"@0xffffff8008a557cc (adapter+16 = `algo`),
	 * "f9400908 ldr"@0xffffff8008a557d0 (algo+16 = `functionality`),
	 * "d63f0100 blr"@0xffffff8008a557d4, e il bit provato e' il numero 0
	 * ("37000240 tbnz"@0xffffff8008a557d8), che e' I2C_FUNC_I2C.
	 */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		/*
		 * "\x013ILITEK: (%s, %d): i2c functions are not supported!\n"@0xffffff800923f3a0,
		 * __LINE__ = 220 ("52801b82 mov"@0xffffff8008a557ec).
		 */
#line 220
		ILI_ERR("i2c functions are not supported!\n");
		/* stesso epilogo del ramo precedente: -19 = -ENODEV */
		return -ENODEV;
	}

	/*
	 * 888 byte azzerati sul `device` del client:
	 * "91008274 add"@0xffffff8008a55820 calcola client+0x20 = &client->dev
	 * (32 = offsetof(i2c_client, dev), misurato),
	 * "52806f01 mov"@0xffffff8008a55828 e' 0x378 = 888 =
	 * sizeof(struct ilitek_tddi_dev), e il gfp in w2 e' di nuovo
	 * GFP_KERNEL | __GFP_ZERO ("52901802 mov"@0xffffff8008a55824 piu'
	 * "72a02802 movk"@0xffffff8008a5582c).
	 * "f90522c0 str"@0xffffff8008a55844 lo scrive nel globale `idev`.
	 */
	idev = devm_kzalloc(&client->dev, sizeof(struct ilitek_tddi_dev),
			    GFP_KERNEL);
	/*
	 * Il controllo e' DUE prove e non una, in quest'ordine:
	 * "b140041f cmn"@0xffffff8008a55840 con "54000988 b.hi"@0xffffff8008a55848
	 * e' `IS_ERR` (puntatore nell'ultima pagina), e
	 * "b4000963 cbz"@0xffffff8008a5584c e' il NULL.
	 */
	if (IS_ERR(idev) || !idev) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to allocate idev memory, %ld\n"@0xffffff800923f3d6,
		 * __LINE__ = 226 ("52801c42 mov"@0xffffff8008a55988).
		 *
		 * IL TERZO ARGOMENTO NON E' PASSATO. Il formato dichiara un
		 * %ld ma fra la costruzione degli argomenti e la `bl` non c'e'
		 * nessuno store in w3/x3: la fabbrica stampa spazzatura. Si
		 * riproduce l'omissione -- aggiungere `PTR_ERR(idev)` qui
		 * sarebbe un'istruzione che nel binario non c'e'.
		 */
#line 226
		ILI_ERR("Failed to allocate idev memory, %ld\n");
		/* "12800160 mov"@0xffffff8008a55990 -- -12 = -ENOMEM */
		return -ENOMEM;
	}

	/*
	 * 2048 byte, GFP_ATOMIC | __GFP_ZERO -- il gfp e' 0x01088020
	 * ("52900401 mov"@0xffffff8008a55858 piu'
	 * "72a02101 movk"@0xffffff8008a5585c), DIVERSO dagli altri tre, e la
	 * taglia 0x800 e' "321503e2 orr"@0xffffff8008a55860. Il compilatore
	 * la risolve in `kmalloc_caches[11]` ("f9435900 ldr"@0xffffff8008a55854,
	 * +1712). "f9008500 str"@0xffffff8008a55870 la mette a idev+264.
	 */
	idev->c264 = kzalloc(0x800, GFP_ATOMIC);
	/* stessa doppia prova: "b140041f cmn"@0xffffff8008a5586c, "b4000900 cbz"@0xffffff8008a55878 */
	if (IS_ERR(idev->c264) || !idev->c264) {
		/*
		 * "\x013ILITEK: (%s, %d): failed to allocate touch report buffer\n"@0xffffff800923f40f,
		 * __LINE__ = 233 ("52801d22 mov"@0xffffff8008a559a8).
		 */
#line 233
		ILI_ERR("failed to allocate touch report buffer\n");
		/* "12800160 mov"@0xffffff8008a559c8 -- -12 = -ENOMEM */
		return -ENOMEM;
	}

	/*
	 * 56 byte, GFP_KERNEL | __GFP_ZERO: "321d0be2 orr"@0xffffff8008a5588c
	 * (0x38 = 56), `kmalloc_caches[6]` ("f9434500 ldr"@0xffffff8008a55880,
	 * +1672), "f9002100 str"@0xffffff8008a5589c su idev+64.
	 */
	idev->c64 = kzalloc(56, GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a55898, "b4000860 cbz"@0xffffff8008a558a4 */
	if (IS_ERR(idev->c64) || !idev->c64) {
		/*
		 * "\x013ILITEK: (%s, %d): Failed to allocate gresture coordinate buffer\n"@0xffffff800923f44b
		 * -- il refuso «gresture» e' di fabbrica.
		 * __LINE__ = 239 ("52801de2 mov"@0xffffff8008a559c0).
		 */
#line 239
		ILI_ERR("Failed to allocate gresture coordinate buffer\n");
		/* "12800160 mov"@0xffffff8008a559c8 -- -12 = -ENOMEM */
		return -ENOMEM;
	}

	/* "f9000113 str"@0xffffff8008a558a8 -- idev+0 = client */
	idev->c0 = client;
	/* "f900051f str"@0xffffff8008a558bc -- idev+8 = NULL esplicito */
	idev->c8 = NULL;
	/* "f9000d14 str"@0xffffff8008a558c0 -- idev+24 = &client->dev (x20) */
	idev->c24 = &client->dev;
	/*
	 * "b0003aa9 adrp"@0xffffff8008a558b0 piu' "91218129 add"@0xffffff8008a558b4
	 * -> 0xffffff80091aa860 = "I2C", scritta a idev+216 da
	 * "f9006d09 str"@0xffffff8008a558c8.
	 */
	idev->c216 = "I2C";
	/*
	 * "f94056ab ldr"@0xffffff8008a558c4 (la prima delle due derivazioni)
	 * e "f900150b str"@0xffffff8008a558ec -- idev+40 = hwif.
	 */
	idev->c40 = container_of(drv, struct ilitek_i2c_drv, drv.driver)->hwif;

	/*
	 * I due puntatori di trasporto:
	 * "9129214a add"@0xffffff8008a558d4 -> 0xffffff8008a55a48 = ilitek_i2c_write,
	 * scritto a +776 da "f901850a str"@0xffffff8008a558e0;
	 * "91309129 add"@0xffffff8008a558d0 -> 0xffffff8008a55c24 = ilitek_i2c_read,
	 * scritto a +784 da "f9018909 str"@0xffffff8008a558d8.
	 */
	idev->c776 = ilitek_i2c_write;
	idev->c784 = ilitek_i2c_read;

	/*
	 * "b901490a str"@0xffffff8008a558f4 -- idev+328 = 43
	 * ("5280056a mov"@0xffffff8008a558e4).
	 */
	idev->c328 = 43;

	/*
	 * Un solo `str` da 64 bit riempie DUE campi a 32:
	 * "9109910b add"@0xffffff8008a558f0 calcola idev+0x264 = idev+612 e
	 * "f9000169 str"@0xffffff8008a55904 vi scrive x9, costruito come
	 * 2 ("d2800049 mov"@0xffffff8008a558dc) con 0x64 nella meta' alta
	 * ("f2c00c89 movk"@0xffffff8008a558e8): c612 = 2 e c616 = 100.
	 */
	idev->c612 = 2;
	idev->c616 = 100;

	/* "f901a11f str"@0xffffff8008a558f8 -- idev+832 = NULL */
	idev->c832 = NULL;
	/* "f901911f str"@0xffffff8008a558fc -- idev+800 = NULL */
	idev->c800 = NULL;
	/* "b901411f str"@0xffffff8008a55900 -- idev+320 = 0 */
	idev->c320 = 0;

	/*
	 * "91319129 add"@0xffffff8008a55910 -> 0xffffff8008a5ac64 =
	 * ilitek_tddi_move_mp_code_flash, a +808
	 * ("f9019509 str"@0xffffff8008a55918).
	 */
	idev->c808 = ilitek_tddi_move_mp_code_flash;
	/*
	 * "912a016b add"@0xffffff8008a55914 -> 0xffffff8008a5ba80 =
	 * ilitek_tddi_move_gesture_code_flash, a +816
	 * ("f901990b str"@0xffffff8008a55920).
	 */
	idev->c816 = ilitek_tddi_move_gesture_code_flash;
	/*
	 * "9100c129 add"@0xffffff8008a55928 -> 0xffffff8008a54030 =
	 * ilitek_tddi_wq_esd_i2c_check, a +824
	 * ("f9019d09 str"@0xffffff8008a55934).
	 */
	idev->c824 = ilitek_tddi_wq_esd_i2c_check;
	/*
	 * "9137116b add"@0xffffff8008a5592c -> 0xffffff8008a5bdc4 =
	 * ilitek_tddi_touch_esd_gesture_flash, a +840
	 * ("f901a50b str"@0xffffff8008a5593c).
	 */
	idev->c840 = ilitek_tddi_touch_esd_gesture_flash;

	/* "b901c10a str"@0xffffff8008a55944 -- idev+448 = 1 */
	idev->c448 = 1;
	/* "b9026d1f str"@0xffffff8008a55948 -- idev+620 = 0 */
	idev->c620 = 0;
	/* "f9010d09 str"@0xffffff8008a5594c -- idev+536 = 4 ("321e03e9 orr"@0xffffff8008a55938) */
	idev->c536 = 4;
	/* "3909d11f strb"@0xffffff8008a55950 -- idev+628 = 0 */
	idev->c628 = 0;
	/* "39084d0a strb"@0xffffff8008a55954 -- idev+531 = 1 */
	idev->c531 = 1;
	/* "3908491f strb"@0xffffff8008a55958 -- idev+530 = 0 */
	idev->c530 = 0;
	/* "3908b11f strb"@0xffffff8008a5595c -- idev+556 = 0 */
	idev->c556 = 0;
	/* "b901550b str"@0xffffff8008a55960 -- idev+340 = 2 ("321f03eb orr"@0xffffff8008a55940) */
	idev->c340 = 2;
	/* "3909f10a strb"@0xffffff8008a55964 -- idev+636 = 1 */
	idev->c636 = 1;

	/*
	 * L'ultima azione e' una chiamata indiretta attraverso la `hwif`:
	 * "f94056a8 ldr"@0xffffff8008a55968 la ricava una SECONDA volta da
	 * `drv+168`, "f9401108 ldr"@0xffffff8008a5596c prende hwif+32 e
	 * "d63f0100 blr"@0xffffff8008a55970 la chiama SENZA argomenti (vedi
	 * la nota su `c32`). Subito dopo, "17ffffa7 b"@0xffffff8008a55974
	 * salta all'epilogo SENZA toccare w0: il valore di ritorno e' quello
	 * della chiamata.
	 *
	 * La seconda derivazione da `drv`, e non da `idev->c40` appena
	 * scritto, e' cio' che il binario fa.
	 */
	return container_of(drv, struct ilitek_i2c_drv, drv.driver)->hwif->c32();
}

/*
 * ===========================================================================
 * 4) ilitek_i2c_remove -- 0xffffff8008a559d0, 44 byte
 * fabbrica: riga 291
 * ===========================================================================
 */
static int ilitek_i2c_remove(struct i2c_client *client)
{
	/*
	 * Il formato e' "\x016ILITEK: (%s, %d): "@0xffffff800923e290 e finisce
	 * li' -- messaggio vuoto, vedi B4 nel cappello.
	 * __func__ = "ilitek_i2c_remove"@0xffffff800923f5c1,
	 * __LINE__ = 291 ("52802462 mov"@0xffffff8008a559e8).
	 */
#line 291
	ILI_INFO("");

	/* "2a1f03e0 mov"@0xffffff8008a559f0 */
	return 0;
}

/*
 * ===========================================================================
 * 5) ilitek_tddi_interface_dev_exit -- 0xffffff8008a559fc, 76 byte
 * fabbrica: riga 333
 * ===========================================================================
 * L'ARGOMENTO NON E' LA `hwif`. "f9401408 ldr"@0xffffff8008a55a08 legge
 * arg+40 e solo dopo "f9401913 ldr"@0xffffff8008a55a18 legge +48 di QUEL
 * puntatore: e' una doppia indirezione, e +40 e' il campo `c40` di
 * `struct ilitek_tddi_dev` -- quello che `probe` riempie con la `hwif`.
 * L'argomento e' quindi `idev`.
 */
void ilitek_tddi_interface_dev_exit(struct ilitek_tddi_dev *dev)
{
	struct i2c_driver *drv;

	/* "f9401408 ldr"@0xffffff8008a55a08 + "f9401913 ldr"@0xffffff8008a55a18 */
	drv = ((struct ilitek_hwif_info *)dev->c40)->driver;

	/*
	 * "\x016ILITEK: (%s, %d): remove i2c dev\n"@0xffffff800923f2ba,
	 * __func__ = "ilitek_tddi_interface_dev_exit"@0xffffff800923f2de,
	 * __LINE__ = 333 ("528029a2 mov"@0xffffff8008a55a20).
	 * La lettura dei due campi avviene PRIMA della printk.
	 */
#line 333
	ILI_INFO("remove i2c dev\n");

	/* "9400d325 bl"@0xffffff8008a55a2c */
	i2c_del_driver(drv);
	/*
	 * "b4000073 cbz"@0xffffff8008a55a30 salta la `kfree` se il puntatore
	 * e' nullo: e' la forma che `kfree(NULL)` NON produce da sola, quindi
	 * la guardia e' nel sorgente.
	 */
	if (drv)
		/* "97dfeb5e bl"@0xffffff8008a55a38 */
		kfree(drv);
}

/*
 * ===========================================================================
 * 6) ilitek_i2c_write -- 0xffffff8008a55a48, 476 byte
 * fabbrica: righe 93 (in `core_i2c_write`), 156, 166
 * ===========================================================================
 * SONO DUE FUNZIONI, NON UNA, e a dirlo e' il binario: il ramo di errore a
 * 0xffffff8008a55c00 passa a `printk` il `__func__`
 * "core_i2c_write"@0xffffff800923f52e, non "ilitek_i2c_write". Una
 * riscrittura che fondesse i due corpi perderebbe quella stringa -- ed e'
 * cosi' che la divergenza e' stata trovata: `tools/verificacitazioni.py`
 * segnalava "core_i2c_write" come NON_ANCORATA su una prima stesura fusa.
 *
 * L'Appendice B di `rapporto-ilitek-confine-e-unita.md` la conferma e la
 * colloca: «core_i2c_write, righe 93..93, incorporata in ilitek_i2c_write
 * (gruppo B)». La riga 93 e' PRIMA della 156, quindi `core_i2c_write` sta
 * piu' in alto nel file di fabbrica, ed e' qui riprodotta nello stesso
 * ordine.
 *
 * La spartizione dei corpi si legge dal grafo dei salti:
 *   - la costruzione del messaggio, il percorso col checksum, la
 *     `i2c_transfer`, la `kfree` e la riduzione a 0/-1 stanno tutte prima
 *     della lettura di `idev+708`;
 *   - il ramo -ENOMEM ("12800173 mov"@0xffffff8008a55c18) NON torna al
 *     chiamante ma salta a "f9452308 ldr"@0xffffff8008a55b7c, cioe' alla
 *     STESSA lettura di `idev+708` in cui finisce il -1. I due codici
 *     d'errore condividono la coda: e' il chiamante a trattarli, ed e'
 *     questa la prova che la coda appartiene a `ilitek_i2c_write`.
 */
static int core_i2c_write(void *buf, int len)
{
	struct i2c_msg msg;
	u8 *mpbuf = NULL;
	int ret;

	/*
	 * Il messaggio: "790013e9 strh"@0xffffff8008a55a94 mette l'indirizzo
	 * (letto da idev->c0->addr, "79400529 ldrh"@0xffffff8008a55a84),
	 * "790017ff strh"@0xffffff8008a55a88 azzera i flag (scrittura),
	 * "79001be1 strh"@0xffffff8008a55a8c mette la lunghezza e
	 * "f9000be0 str"@0xffffff8008a55a90 il buffer.
	 */
	msg.addr = ((struct i2c_client *)idev->c0)->addr;
	msg.flags = 0;
	msg.len = len;
	msg.buf = buf;

	/*
	 * Il percorso col checksum si prende solo se tutte e tre le prove
	 * passano, nell'ordine in cui il binario le mette:
	 *   "530a7d29 lsr"@0xffffff8008a55aa0 e "7105053f cmp"@0xffffff8008a55aa4
	 *      -- vedi B3: il primo u32 di c56, scorso di 10, contro 0x141;
	 *   "394002a9 ldrb"@0xffffff8008a55aac e "7103c53f cmp"@0xffffff8008a55ab0
	 *      -- il primo byte del comando contro 0xf1;
	 *   "b9414109 ldr"@0xffffff8008a55ab8 e "7100053f cmp"@0xffffff8008a55abc
	 *      -- il modo TP (idev+320) contro 1.
	 */
	if ((*(u32 *)idev->c56 >> 10) >= 0x141 &&
	    ((u8 *)buf)[0] == 0xf1 &&
	    idev->c320 == 1) {
		u8 checksum;

		/* "940018b2 bl"@0xffffff8008a55acc -> ilitek_calc_packet_checksum */
		checksum = ilitek_calc_packet_checksum(buf, len);

		/*
		 * `len + 1` ("11000668 add"@0xffffff8008a55ad0), azzerato,
		 * GFP_KERNEL: la taglia e' variabile, quindi il compilatore
		 * chiama `__kmalloc` e non un `kmem_cache_alloc_trace`
		 * ("97dfea28 bl"@0xffffff8008a55ae8).
		 */
		mpbuf = kzalloc(len + 1, GFP_KERNEL);
		/* "b140041f cmn"@0xffffff8008a55aec, "b4000840 cbz"@0xffffff8008a55af8 */
		if (IS_ERR(mpbuf) || !mpbuf) {
			/*
			 * "\x013ILITEK: (%s, %d): Failed to allocate mpbuf mem\n"@0xffffff800923f4fc,
			 * __func__ = "core_i2c_write"@0xffffff800923f52e,
			 * __LINE__ = 93 ("52800ba2 mov"@0xffffff8008a55c10).
			 */
#line 93
			ILI_ERR("Failed to allocate mpbuf mem\n");
			/* "12800173 mov"@0xffffff8008a55c18 -- -12 = -ENOMEM */
			return -ENOMEM;
		}

		/*
		 * "9410087b bl"@0xffffff8008a55b14 verso __memcpy. La
		 * lunghezza NON e' `len`: e' il minimo fra `msg.len` riletto
		 * dalla pila ("79401be8 ldrh"@0xffffff8008a55afc) e `len`,
		 * scelto con un confronto CON SEGNO
		 * ("6b13011f cmp"@0xffffff8008a55b08,
		 * "1a93b108 csel"@0xffffff8008a55b0c) e poi esteso a 64 bit
		 * sempre con segno ("93407d02 sxtw"@0xffffff8008a55b10).
		 * Vedi B2: la forma sorgente che lo produce non e' unica.
		 */
		memcpy(mpbuf, buf, min_t(int, msg.len, len));
		/* "3833ca97 strb"@0xffffff8008a55b30 -- mpbuf[len] = checksum */
		mpbuf[len] = checksum;

		/* "f9000bf4 str"@0xffffff8008a55b34 -- msg.buf prima della dump */
		msg.buf = mpbuf;
		/*
		 * "940013e8 bl"@0xffffff8008a55b38 verso ilitek_dump_data, con
		 * "mp cdc cmd with checksum"@0xffffff800923f53d in x4,
		 * 8 in w1 ("321d03e1 orr"@0xffffff8008a55b20), len+1 in w2 e
		 * 0 in w3 ("2a1f03e3 mov"@0xffffff8008a55b2c).
		 */
		ilitek_dump_data(mpbuf, 8, len + 1, 0,
				 "mp cdc cmd with checksum");
		/* "79001bf6 strh"@0xffffff8008a55b40 -- msg.len = len + 1 */
		msg.len = len + 1;
	}

	/*
	 * "f9400108 ldr"@0xffffff8008a55b4c (idev+0 = client) e
	 * "f9400d00 ldr"@0xffffff8008a55b58 (client+24 = adapter);
	 * "320003e2 orr"@0xffffff8008a55b54 e' il conteggio 1.
	 * "9400d529 bl"@0xffffff8008a55b5c
	 */
	ret = i2c_transfer(((struct i2c_client *)idev->c0)->adapter, &msg, 1);

	/*
	 * "b4000074 cbz"@0xffffff8008a55b64 piu' "97dfeb11 bl"@0xffffff8008a55b6c:
	 * la `kfree` e' guardata, ed e' DOPO il trasferimento.
	 */
	if (mpbuf)
		kfree(mpbuf);

	/*
	 * "7100067f cmp"@0xffffff8008a55b70 -- un solo messaggio trasferito.
	 * Il ramo che riesce va a "2a1f03f3 mov"@0xffffff8008a55b8c (zero),
	 * l'altro a "12800013 mov"@0xffffff8008a55b78 (meno uno).
	 */
	if (ret != 1)
		return -1;

	return 0;
}

static int ilitek_i2c_write(void *buf, int len)
{
	int ret;

	/* "34000941 cbz"@0xffffff8008a55a6c -- la sola prova e' su `len` */
	if (!len) {
		/*
		 * "\x013ILITEK: (%s, %d): i2c write len is invalid\n"@0xffffff800923f48e,
		 * __func__ = "ilitek_i2c_write"@0xffffff800923f4bc,
		 * __LINE__ = 156 ("52801382 mov"@0xffffff8008a55ba4).
		 */
#line 156
		ILI_ERR("i2c write len is invalid\n");
		/* "128002b3 mov"@0xffffff8008a55bac -- -22 = -EINVAL */
		return -EINVAL;
	}

	ret = core_i2c_write(buf, len);

	if (ret < 0) {
		/*
		 * "b942c508 ldr"@0xffffff8008a55b80 legge idev+708 e
		 * "7100051f cmp"@0xffffff8008a55b84 lo confronta con 1: se il
		 * reset e' in corso l'errore NON si stampa e la funzione
		 * riesce ("2a1f03f3 mov"@0xffffff8008a55b8c).
		 */
		if (idev->c708 == 1) {
			ret = 0;
		} else {
			/*
			 * "\x013ILITEK: (%s, %d): i2c write error, ret = %d\n"@0xffffff800923f4cd,
			 * __LINE__ = 166 ("528014c2 mov"@0xffffff8008a55bc4),
			 * e il quarto argomento e' `ret`
			 * ("2a1303e3 mov"@0xffffff8008a55bc8).
			 */
#line 166
			ILI_ERR("i2c write error, ret = %d\n", ret);
		}
	}

	return ret;
}

/*
 * ===========================================================================
 * 7) ilitek_i2c_read -- 0xffffff8008a55c24, 232 byte
 * fabbrica: righe 178, 188
 * ===========================================================================
 */
static int ilitek_i2c_read(void *buf, int len)
{
	struct i2c_msg msg;
	int ret;

	/* "34000321 cbz"@0xffffff8008a55c40 */
	if (!len) {
		/*
		 * "\x013ILITEK: (%s, %d): i2c read len is invalid\n"@0xffffff800923f556,
		 * __func__ = "ilitek_i2c_read"@0xffffff800923f583,
		 * __LINE__ = 178 ("52801642 mov"@0xffffff8008a55cb4).
		 */
#line 178
		ILI_ERR("i2c read len is invalid\n");
		/* "128002b3 mov"@0xffffff8008a55cbc -- -22 = -EINVAL */
		return -EINVAL;
	}

	/*
	 * Identico a `ilitek_i2c_write` tranne i flag:
	 * "790017ea strh"@0xffffff8008a55c60 scrive 1
	 * ("320003ea orr"@0xffffff8008a55c4c), cioe' I2C_M_RD.
	 */
	msg.addr = ((struct i2c_client *)idev->c0)->addr;
	msg.flags = I2C_M_RD;
	msg.len = len;
	msg.buf = buf;

	/* "9400d4e1 bl"@0xffffff8008a55c7c */
	ret = i2c_transfer(((struct i2c_client *)idev->c0)->adapter, &msg, 1);
	/*
	 * "7100041f cmp"@0xffffff8008a55c80 piu'
	 * "1a800273 csel"@0xffffff8008a55c84: se il trasferimento riesce il
	 * ritorno e' `len`, altrimenti l'errore di `i2c_transfer`.
	 */
	if (ret == 1)
		ret = len;

	/* "36f802d3 tbz"@0xffffff8008a55c88 -- solo il bit di segno */
	if (ret < 0) {
		/* "b942c508 ldr"@0xffffff8008a55c90, "7100051f cmp"@0xffffff8008a55c94 */
		if (idev->c708 == 1) {
			/* "2a1f03f3 mov"@0xffffff8008a55c9c */
			ret = 0;
		} else {
			/*
			 * "\x013ILITEK: (%s, %d): i2c read error, ret = %d\n"@0xffffff800923f593,
			 * __LINE__ = 188 ("52801782 mov"@0xffffff8008a55cd4),
			 * quarto argomento `ret` ("2a1303e3 mov"@0xffffff8008a55cd8).
			 */
#line 188
			ILI_ERR("i2c read error, ret = %d\n", ret);
		}
	}

	return ret;
}
