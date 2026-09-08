// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_FT8719_E977 -- FocalTech FT8719, Doogee S88 Pro (MT6771).
 * L'UNITA' DI TRADUZIONE DEI "MODI" (unita' D), e SOLO quella.
 * Lotto PARZIALE dichiarato.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA.  Nessun sorgente
 * pubblico e' stato letto; in particolare i due driver FocalTech che l'albero
 * ALPS contiene gia' (focaltech_touch/, focaltech_fhd_touch/) NON sono stati
 * aperti.
 *
 * ===========================================================================
 * 1. CHE COSA C'E' QUI DENTRO, E QUAL E' IL CONFINE
 * ===========================================================================
 *   testo   0xffffff8008a8020c .. 0xffffff8008a802ac   3 funzioni, 160 byte
 *   .data   0xffffff800998cbb8 .. 0xffffff800998cbe0   40 byte, un solo
 *                                                      `attribute_group`
 *   .bss    0xffffff800a100af0 .. 0xffffff800a100b08   24 byte
 *
 * I confini vengono dalla partizione in sei unita' di traduzione misurata
 * dalla ricognizione (scout-ft8719/RIASSUNTO.md §5): l'ordine del testo,
 * quello di `.data` e quello di `.bss` danno la stessa partizione.  Qui i due
 * confini si toccano da soli: 0x998cbb8 + 40 = 0x998cbe0, che e' il mutex
 * dell'unita' E; e 0xa100af0 + 24 = 0xa100b08, dove finisce il `.bss` del
 * blocco.
 *
 * IL NOME DEL FILE E' UNA SCELTA (il binario non porta i nomi dei file di
 * fabbrica; `ddebug.py --file focaltech` -> "dopo il filtro: 0").  E' un file
 * a se' e non un pezzo di un altro per la stessa ragione registrata nel
 * Makefile: unire due unita' darebbe a clang la facolta' di incorporare
 * funzioni che di fabbrica stanno in file oggetto distinti.
 *
 * ===========================================================================
 * 2. COME SI RIVERIFICA
 * ===========================================================================
 *   ./venv/bin/python3 verificacitazioni.py focaltech_ex_mode.c oracolo/stock.elf \
 *       --eccezione "[FTS]" --eccezione "[FTS][Error]" --eccezione "\n"
 *
 *   ./venv/bin/python3 verificaistruzioni.py focaltech_ex_mode.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a8020c:0xffffff8008a802ac
 *
 * Nessuna `--controfattuale`.  Le codifiche non sono trascritte a mano: escono
 * da uno script che legge il disassemblato e stampa la coppia (codifica,
 * indirizzo) insieme (classe B8).
 *
 * ESITO ATTESO, al 2026-08-21:
 *   citazioni di istruzione trovate nel sorgente: 24 (24 a codifica, 0 ad indirizzo)
 *   confermate: 24   assenti: 0   mnemonico diverso: 0   controfattuali: 0
 *   operandi -- registro diverso: 0   immediato diverso: 0   entrambi/non confrontabili: 0
 * e per le citazioni di letterale:
 *   letterali: 5   citati: 2   verificati: 1   probanti: 1   deboli: 0
 *   soglia imposta: 2 citati richiesti (5 letterali - 3 eccezioni)
 * piu' UNA `NON_ANCORATA`: "[FTS][Mode]create sysfs succeeded\n"
 * @0xffffff800924ff22, che e' un messaggio della macro SENZA livello KERN.
 * Lo strumento riconosce i messaggi assemblati dalla testa `\x01` + cifra di
 * livello; FTS_DEBUG non ne emette nessuna, e il binario lo prova da solo --
 * quel letterale comincia con '[' mentre
 * "\x013[FTS][Error][Mode]create sysfs failed.\n"@0xffffff800924fef8 comincia
 * col SOH.  E' un DELTA DI STRUMENTO gia' registrato in coda a
 * focaltech_core.c, e non l'ho fatto: verificacitazioni.py e' condiviso.
 *
 * ===========================================================================
 * 3. LE DIVERGENZE APERTE
 * ===========================================================================
 * Nessuna: le tre funzioni misurano come di fabbrica.  La tabella e' in coda.
 * Ce n'era UNA, -16 byte su fts_ex_mode_init, ed e' stata trovata e chiusa:
 * era una `static` di troppo (classe A1).  L'esperimento che l'ha chiusa e'
 * scritto accanto alla variabile, perche' e' quello il posto dove serve.
 *
 * ===========================================================================
 * 4. CIO' CHE NON E' SCRITTO QUI
 * ===========================================================================
 * Le altre cinque unita' del blocco (C sta in focaltech_core.c, E in
 * focaltech_i2c.c; restano A, B, F).  Questa unita' non chiama nessuna
 * funzione del blocco -- solo `sysfs_create_group`, `sysfs_remove_group` e
 * `printk` -- quindi e' completa e si linka da sola.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/sysfs.h>

/* Le due macro che questa unita' usa, lette dai formati interi:
 *   "\x013[FTS][Error][Mode]create sysfs failed.\n"@0xffffff800924fef8  (KERN_ERR)
 *   "[FTS][Mode]create sysfs succeeded\n"@0xffffff800924ff22            (nessun livello)
 * Il SOH+'3' della prima e la '[' della seconda provano che sono due macro
 * diverse; "[Mode]" e' invece parte del messaggio, non del prefisso, perche'
 * compare DOPO "[FTS][Error]".
 */
#define FTS_DEBUG(fmt, args...)		printk("[FTS]" fmt "\n", ##args)
#define FTS_ERROR(fmt, args...)		printk(KERN_ERR "[FTS][Error]" fmt "\n", ##args)

/*
 * IL GRUPPO SYSFS A 0xffffff800998cbb8 -- LETTO DALLE RILOCAZIONI, NON SUPPOSTO
 * ==========================================================================
 * Il kernel di fabbrica e' CONFIG_RELOCATABLE=y: i puntatori nelle sezioni
 * dati valgono ZERO nell'immagine e il valore vero sta nell'addend di una
 * R_AARCH64_RELATIVE.  I cinque slot di `struct attribute_group`:
 *
 *   $ for a in 0xffffff800998cbb8 0xffffff800998cbc0 0xffffff800998cbc8 \
 *              0xffffff800998cbd0 0xffffff800998cbd8; do
 *         ./venv/bin/python3 relocazioni.py oracolo/stock.elf oracolo/stock.map --indirizzo $a
 *     done
 *   0xffffff800998cbb8: non relocato                                  .name
 *   0xffffff800998cbc0: non relocato                                  .is_visible
 *   0xffffff800998cbc8: non relocato                                  .is_bin_visible
 *   0xffffff800998cbd0: relocato, addend 0xffffff800a100b00           .attrs
 *   0xffffff800998cbd8: non relocato                                  .bin_attrs
 *
 * IL GRUPPO NON REGISTRA NESSUN ATTRIBUTO.  `.attrs` punta a 0xffffff800a100b00,
 * che sta in `.bss` (azzerata dal boot) ed e' OTTO byte, cioe' un array di UN
 * solo elemento: il terminatore NULL.  Se ci fosse anche un solo attributo,
 * quel puntatore starebbe in `.data` con una sua rilocazione, come le OTTO
 * `device_attribute` del gruppo dell'unita' A a 0xffffff800998c750.
 * Riprodurlo CON degli attributi sembrerebbe piu' corretto e sarebbe una
 * divergenza: e' esattamente la classe B6, codice aggiunto che nel binario non
 * c'e'.
 */
static struct attribute *fts_g_a100b00[] = {
	NULL,
};

static struct attribute_group fts_g_998cbb8 = {
	.attrs = fts_g_a100b00,
};

/*
 * I TRE INTERI AZZERATI DA fts_ex_mode_init, a 0xffffff800a100af0.
 *   "9000b408 adrp"@0xffffff8008a80218 + "912bc108 add"@0xffffff8008a80224
 *      -> x8 = 0xa100000 + 0xaf0 = 0xffffff800a100af0
 *   "f900011f str"@0xffffff8008a80230   str xzr, [x8]     -> 8 byte a +0
 *   "b900091f str"@0xffffff8008a80234   str wzr, [x8,#8]  -> 4 byte a +8
 * Dodici byte azzerati in due colpi: la larghezza dice TRE `int` (o due int e
 * un int, il che e' la stessa cosa), non un `long` piu' un `int` -- perche'
 * un `long` a +0 seguito da un `int` a +8 darebbe la stessa coppia di store,
 * e il binario qui NON discrimina.  Dichiarato come tale.
 * Il NOME dei tre campi il binario non lo dice: nessuna macro li stringa,
 * nessun formato li nomina.  Si chiamano come il loro offset (regola 5).
 *
 * NON E' `static`, ED E' LA DIMENSIONE A DIRLO -- classe A1, trovata qui per
 * la terza volta nel progetto (le prime due su ilitek_probe_riuscito e
 * mir3da_chip_index).  Scritta `static`, questa variabile non viene letta da
 * nessuna funzione di questa unita' -- fts_ex_mode_exit e fts_ex_mode_recovery
 * non la toccano -- e clang cancella i tre azzeramenti come scritture morte.
 * La misura dell'esperimento, sullo stesso albero e con lo stesso clang:
 *
 *   variabile          fts_ex_mode_init nel .o   scarto sulla fabbrica (116)
 *   -----------------  -----------------------   ---------------------------
 *   `static struct`    0x64 = 100 byte           -16   (4 istruzioni in meno)
 *   `struct`           0x74 = 116 byte             0
 *
 * Le quattro istruzioni che tornano sono esattamente
 *   "9000b408 adrp"@0xffffff8008a80218
 *   "912bc108 add"@0xffffff8008a80224
 *   "f900011f str"@0xffffff8008a80230
 *   "b900091f str"@0xffffff8008a80234
 * cioe' l'indirizzo della variabile e i due store.  Con collegamento esterno
 * clang non puo' provare che nessuno legga, e le scritture restano.
 *
 * NON DIMOSTRA che di fabbrica la variabile fosse `extern`-visibile per
 * scelta del programmatore: `volatile` darebbe lo stesso effetto.  Cio' che e'
 * misurato e' che `static` semplice NON riproduce il binario.  E cio' che si
 * sa dall'altra parte e' che NESSUNA istruzione in tutta l'immagine legge
 * quei dodici byte (scansione di tutto il kernel disassemblato,
 * scout-ft8719/tocca.py): sono scritti e mai riletti.
 */
struct {
	int c0;
	int c4;
	int c8;
} fts_g_a100af0;

/*
 * ===========================================================================
 * fts_ex_mode_init @ 0xffffff8008a8020c, 116 byte
 * ===========================================================================
 * IL PRIMO PARAMETRO E' UN `struct i2c_client *`, e il +0x30 lo prova:
 *   "9100c013 add"@0xffffff8008a8021c   x19 = x0 + 0x30
 *   "aa1303e0 mov"@0xffffff8008a8022c   x0 = x19, primo arg di
 *   "97e20e2e bl"@0xffffff8008a80238    sysfs_create_group, che vuole un
 *                                       `struct kobject *`
 * In questo albero `struct i2c_client` ha `dev` a +32 e `struct device` ha
 * `kobj` a +16 (il terzo campo, dopo `parent` e `p`): 32 + 16 = 48 = 0x30.
 * E' un fatto degli header del kernel, non una scelta di questo file, e la
 * COMPILAZIONE lo verifica -- se l'offset non fosse 0x30 la dimensione non
 * tornerebbe e il `.o` porterebbe un altro immediato.
 *
 * L'ORDINE: gli azzeramenti PRECEDONO la sysfs_create_group
 * ("f900011f str"@0xffffff8008a80230 e "b900091f str"@0xffffff8008a80234 stanno
 * prima di "97e20e2e bl"@0xffffff8008a80238), quindi nel sorgente stanno prima.
 *
 * IL RAMO D'ERRORE RIMUOVE UN GRUPPO CHE NON E' STATO CREATO.  Se
 * sysfs_create_group fallisce ("34000140 cbz"@0xffffff8008a8023c salta al ramo
 * buono quando w0 == 0, quindi il ramo che segue e' w0 != 0), la funzione
 * stampa l'errore e poi chiama LO STESSO sysfs_remove_group
 * ("97e20f26 bl"@0xffffff8008a80258) sullo stesso kobject e sullo stesso
 * gruppo.  E' un difetto della fabbrica e si riproduce (regola 7): toglierlo
 * sarebbe togliere codice che nel binario c'e'.
 *
 * IL CODICE D'ERRORE E' -5:
 *   "321d7be0 orr"@0xffffff8008a8025c   orr w0, wzr, #0xfffffffb = -5 = -EIO
 * e il ramo buono restituisce 0: "2a1f03e0 mov"@0xffffff8008a80270.
 */
int fts_ex_mode_init(struct i2c_client *client)
{
	int ret = 0;

	fts_g_a100af0.c0 = 0;
	fts_g_a100af0.c4 = 0;
	fts_g_a100af0.c8 = 0;

	ret = sysfs_create_group(&client->dev.kobj, &fts_g_998cbb8);
	if (ret != 0) {
		FTS_ERROR("[Mode]create sysfs failed.");
		sysfs_remove_group(&client->dev.kobj, &fts_g_998cbb8);
		return -EIO;
	}

	FTS_DEBUG("[Mode]create sysfs succeeded");

	return 0;
}

/*
 * ===========================================================================
 * fts_ex_mode_exit @ 0xffffff8008a80280, 36 byte
 * ===========================================================================
 *   "9100c000 add"@0xffffff8008a8028c   x0 = client + 0x30
 *   "912ee021 add"@0xffffff8008a80290   x1 = 0x998c000 + 0xbb8, lo stesso gruppo
 *   "97e20f17 bl"@0xffffff8008a80294    sysfs_remove_group
 *   "2a1f03e0 mov"@0xffffff8008a80298   w0 = 0
 * Il valore di ritorno di sysfs_remove_group (che e' `void`) non c'entra: il
 * `mov w0, wzr` DOPO la `bl` e' un `return 0` esplicito.
 */
int fts_ex_mode_exit(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &fts_g_998cbb8);

	return 0;
}

/*
 * ===========================================================================
 * fts_ex_mode_recovery @ 0xffffff8008a802a4, 8 byte
 * ===========================================================================
 * DUE istruzioni in tutto:
 *   "2a1f03e0 mov"@0xffffff8008a802a4   w0 = 0
 *   "d65f03c0 ret"@0xffffff8008a802a8   ret
 * E' uno stub VERO della fabbrica, non una mia omissione: il parametro non
 * viene letto, non c'e' cornice, non c'e' nessuna chiamata.  E' una delle OTTO
 * funzioni da 8 byte del blocco.
 */
int fts_ex_mode_recovery(struct i2c_client *client)
{
	return 0;
}

/*
 * ===========================================================================
 * LA MISURA
 * ===========================================================================
 * Dal `.o` VERO, non da misuraisolata.py:
 *
 *   $ aarch64-linux-android-nm --print-size \
 *       out-ft8719/.../focaltech_ex_mode.o | grep -iE ' [tT] '
 *   0000000000000074 0000000000000024 T fts_ex_mode_exit
 *   0000000000000000 0000000000000074 T fts_ex_mode_init
 *   0000000000000098 0000000000000008 T fts_ex_mode_recovery
 *
 *   funzione              fabbrica  nostro  scarto
 *   --------------------  --------  ------  ------
 *   fts_ex_mode_init           116     116       0
 *   fts_ex_mode_exit            36      36       0
 *   fts_ex_mode_recovery         8       8       0
 *   --------------------  --------  ------  ------
 *   totale                     160     160       0
 *
 * TRE SU TRE, e tre e' una numerosita' minuscola: la misura NON DISCRIMINA
 * in nessuna direzione (classe C4).  L'intervallo di Clopper-Pearson al 95%
 * su 3/3 e' [29,2% ; 100%] e contiene il 77,10% del ramo insieme a quasi
 * tutto il resto.  L'avvertenza viene PRIMA della percentuale, che e' 100%.
 *
 * NESSUNA DIVERGENZA APERTA.  Ce n'era una, -16 byte su fts_ex_mode_init, ed
 * e' documentata accanto alla variabile che l'ha causata (classe A1: una
 * `static` di troppo, che faceva cancellare a clang i tre azzeramenti come
 * scritture morte).
 */
