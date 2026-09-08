// SPDX-License-Identifier: GPL-2.0
/*
 * mt5725.c -- caricatore wireless Maxic MT5725 del Doogee S88 Pro (E977),
 * ricostruito leggendo il kernel di fabbrica disassemblato («l'oracolo»).
 * Nessun sorgente pubblico e' stato aperto: ogni riga qui sotto e' derivata
 * dai byte del binario e porta accanto la citazione che la prova.
 *
 * ------------------------------------------------------------------ CONFINE
 * Il blocco mt5725 e' di 61 funzioni:
 *   - 55 adiacenti, da MT5725_soft_reset (0xffffff8008aca8d4) a
 *     Mt5725_get_rxdetect (0xffffff8008ace894), 16.372 byte di codice proprio
 *     (16.412 secondo la mappa: gli ultimi 40 byte sono una copia debole e
 *     scartata di is_usb_rdy, senza simbolo proprio);
 *   - mt5725_driver_init, 32 byte a 0xffffff8009385a60, in `.init.text`;
 *   - mt5725_driver_exit, 28 byte a 0xffffff80093adb50, in `.exit.text`
 *     — OLTRE `_einittext` (0xffffff80093a8518), quindi INVISIBILE a
 *     `oracolo/stock.map` e a tutti gli strumenti che partono dalla mappa;
 *   - 4 ops power_supply (mt_wls_set_property, mt_wls_get_property,
 *     mt_wls_property_is_writeable, mt_rvs_get_property) che NON stanno qui:
 *     stanno nella stessa unita' di traduzione di mt_charger_probe e
 *     mt_usb_get_property, che in ALPS si chiama
 *     `drivers/power/supply/mediatek/charger/mtk_chg_type_det.c` e NON
 *     `mtk_charger.c` -- correzione a cio' che diceva la ricognizione:
 *     `mtk_charger.c` esiste in ALPS ma NON contiene mt_charger_probe.
 *     Vedi «DELTA» nel rapporto.
 *
 * Le 57 funzioni che stanno in QUESTO file oggetto (55 + init + exit) sono
 * tutte scritte. Le 4 che restano non appartengono a questo file: sono in
 * `mtk_chg_type_det.c`, che esiste in ALPS e che questo lotto NON riscrive. Non
 * c'e' nessuno stub con corpo segnaposto, e non ce n'e' bisogno: non e'
 * rimasta nessuna funzione dichiarata e non definita.
 *
 * ---------------------------------------------------------- COME SI RIVERIFICA
 * Sul PC di build, in /mnt/s88pro/kernel-stock. I comandi sono esatti.
 *
 *   # disassemblare una funzione di fabbrica
 *   ./venv/bin/python3 disassembla.py oracolo/stock.map oracolo/stock.elf \
 *       MT5725_soft_reset
 *
 *   # compilare con il compilatore VERO della fabbrica (clang 9.0.3 r353983c)
 *   lavoro-mt5725/build9.sh
 *   # ... e con quello dell'albero del progetto (clang 11.0.1 r383902)
 *   lavoro-mt5725/build.sh
 *
 *   # misurare, funzione per funzione, dal .o vero: 57 confrontate,
 *   # 37 di dimensione identica, 36 identiche istruzione per istruzione
 *   ./venv/bin/python3 lavoro-mt5725/identita.py
 *   # ... e l'intervallo, con l'avvertenza che sta nel rapporto
 *   ./venv/bin/python3 intervallo.py 37 57
 *
 *   # verificare i letterali citati
 *   ./venv/bin/python3 verificacitazioni.py \
 *       albero-mt5725/drivers/power/supply/mediatek/charger/mt5725/mt5725.c \
 *       oracolo/stock.elf
 *
 *   # verificare le codifiche di istruzione citate. Servono QUATTRO intervalli:
 *   # il blocco adiacente, .init.text, .exit.text e il singolo sito di lettura
 *   # in tcpc_typec_handle_timeout che prova che 0xffffff800a110fe4 esce
 *   # dall'oggetto.
 *   ./venv/bin/python3 verificaistruzioni.py \
 *       albero-mt5725/drivers/power/supply/mediatek/charger/mt5725/mt5725.c \
 *       oracolo/stock.elf \
 *       --intervallo 0xffffff8008aca8d4:0xffffff8008ace8f0 \
 *       --intervallo 0xffffff8009385a60:0xffffff8009385a80 \
 *       --intervallo 0xffffff80093adb50:0xffffff80093adb6c \
 *       --intervallo 0xffffff80088f6e50:0xffffff80088f6e54
 *
 *   # ricontrollare che i due blob di firmware nel .h siano ancora i byte
 *   # dell'oracolo
 *   ./venv/bin/python3 lavoro-mt5725/estrai_fw.py
 *
 * -------------------------------------------------- I DUE BLOB DI FIRMWARE
 * Sono in `mt5725_fw.h`, ESTRATTI CON UNO SCRIPT (`lavoro-mt5725/estrai_fw.py`)
 * dai byte dell'oracolo, mai trascritti a mano:
 *   MT5725_pgm_fw   836 byte @0xffffff8008f82170
 *                   sha256 fcf7c8912a370b4db1d05184965d40c33be4331304cd369562cb66a9eec96af6
 *   MT5725_otp_fw 15024 byte @0xffffff8008f824b4
 *                   sha256 461bee797480137ddc2a50fdcfe71ae77b531b498d6e0ebbe21f664d2e1f1839
 *
 * ------------------------------------------- IL COMPILATORE, CHE NON E' UNO
 * Il kernel di fabbrica e' stato costruito con Android clang r353983c, cioe'
 * LLVM 9.0.3; l'albero del progetto usa clang-r383902 = LLVM 11.0.1.
 *   strings -a oracolo/stock.elf | grep -i "clang version"
 *   Linux version 4.14.141 (nobody@android-build) (Android (5484270 based on
 *   r353983c) clang version 9.0.3 ... (based on LLVM 9.0.3svn)) #1 SMP PREEMPT
 *   Tue Jun 23 09:55:04 CST 2020
 * Con LLVM 11 il confronto e' inquinato da differenze che NON vengono dal
 * sorgente: LLVM 10 ha spostato il record di frame (x29/x30) in fondo all'area
 * dei registri salvati, LLVM 11 srotola cicli che LLVM 9 lascia stare, e i
 * piccoli immediati si materializzano con `mov` invece che con `orr ..., wzr`.
 * Le misure di questo file sono fatte con il compilatore VERO, scaricato da
 * AOSP e scompattato in /mnt/s88pro/kernel-stock/clang9/root -- vedi
 * lavoro-mt5725/build9.sh. Il file compila pulito, con -Werror, con TUTTI E DUE.
 *
 * ------------------------------------------------------- DIVERGENZE APERTE
 * Numerate; la misura funzione per funzione sta nel rapporto del lotto.
 * La misura, col compilatore VERO: 56 funzioni confrontate, 49 di dimensione
 * identica -- 50 contando Mt5725_get_rxdetect, dove la mappa dichiara 92 byte
 * ma il corpo vero e' 52 (corpo debole scartato dal linker) e noi lo
 * riproduciamo. Residuo totale: 80 byte su sei funzioni.
 *
 * Il salto da 36 a 49 e' del 2026-08-23 e ha quattro cause, tutte lette nel
 * binario e tutte scritte qui sotto:
 *   - i siti che scrivono due byte non chiamano tutti la stessa funzione:
 *     ce ne sono TRE (divergenza 2);
 *   - mt5725_g11101c e' volatile: la fabbrica la carica due volte di fila
 *     ("b9401f89 ldr"@0xffffff8008acde38 e
 *     "b9401f88 ldr"@0xffffff8008acde3c, consecutive), noi una sola;
 *   - il quarto parametro di MT5725_otp_read va tenuto vivo (divergenza 3);
 *   - MT5725_write_buffer non e' un ciclo ruotato (divergenza 2).
 *
 *  1. LE QUATTRO OPS power_supply DEL BLOCCO NON SONO QUI. mt_wls_set_property,
 *     mt_wls_get_property, mt_wls_property_is_writeable e mt_rvs_get_property
 *     stanno in `mtk_chg_type_det.c`, che esiste gia' in ALPS (e' il file che
 *     contiene mt_charger_probe e registra le power_supply "charger", "ac" e
 *     "usb"): sono un DELTA su un file condiviso, descritto nel rapporto, non
 *     righe da scrivere qui.
 *
 *  2. IL CICLO DI MT5725_write_buffer INCORPORATO -- CHIUSO, e la conclusione
 *     che stava qui era SBAGLIATA. Va detto per intero perche' l'errore era
 *     ragionevole e il prossimo rischia di rifarlo.
 *
 *     Quello che diceva: la fabbrica tiene il ciclo in certi siti (soft_reset,
 *     enable_afc/_apple/_epp, SetFodPara) e lo srotola in altri (fast_vfc,
 *     fastcharge_afc, parte di rx_sram_updata e di fsk_handle), con lo stesso
 *     compilatore e nello stesso oggetto; quindi "nessuna forma di sorgente
 *     puo' rendere tutti i siti, la scelta non e' nel sorgente".
 *
 *     Perche' era sbagliata: la premessa taciuta era che i siti chiamassero
 *     TUTTI la stessa funzione. Non e' cosi'. In `fast_vfc` la fabbrica non
 *     tocca la pila -- "2a1403e2 mov"@0xffffff8008acab38 per il byte basso e
 *     "53083e82 ubfx"@0xffffff8008acab50 per quello alto -- mentre
 *     MT5725_write_buffer un buffer in memoria ce l'ha sempre
 *     ("790013e8 strh"@0xffffff8008acc674, "38746aa2 ldrb"@0xffffff8008acc684).
 *     Quei siti non sono un ciclo srotolato: NON SONO UN CICLO. Sono una
 *     SECONDA funzione, che prende una parola da 16 bit e ne scrive i due
 *     byte, incorporata ovunque e senza simbolo -- vedi `c_acab34`.
 *
 *     Il MECCANISMO descritto qui prima, invece, era giusto ed e' servito: in
 *     un ciclo ruotato il test del latch e' quello di lunghezza, LoopUnroll ne
 *     ricava un numero di giri costante e srotola; in un ciclo NON ruotato il
 *     latch e' `ret < 0`, il numero di giri risulta 0 e il ciclo resta. La
 *     fabbrica non e' ruotata -- "7100069f cmp"@0xffffff8008aca904 in testa e
 *     "36fffee0 tbz"@0xffffff8008aca928 come arco all'indietro -- e la forma
 *     `while (ret >= 0) { if (i == len) return ret; ... }` la riproduce.
 *
 *     Misurato, sull'oggetto intero e a ogni passo:
 *       for indicizzato (com'era)                      36 su 56, residuo 1136
 *       while (ret >= 0), registro da reg + i          31 su 56, residuo  628
 *       while (ret >= 0), registro e buffer induttivi  39 su 56, residuo  456
 *       piu' c_acab34 ai siti a 16 bit                 44 su 56, residuo  252
 *     Le tre enable_* restano su MT5725_write_buffer: li' il ciclo in fabbrica
 *     c'e' davvero, e passarle a c_acab34 le riporta a -28.
 *
 *     E LE TRE FUNZIONI CHE RESTAVANO PIU' GRANDI ERANO LA PROVA CHE MANCAVA
 *     ANCORA QUALCOSA. rx_sram_updata (+20), charger_routine_thread (+36) e
 *     set_reverse_charger (+16) tenevano un ciclo che la fabbrica srotola:
 *     "52800ce2 mov"@0xffffff8008ace350 e "394007e2 ldrb"@0xffffff8008ace364
 *     sono il primo byte ripiegato e il secondo riletto dalla pila, cioe' un
 *     ciclo da due giri aperto. Provate quattro forme che mettono i due test
 *     nella condizione del ciclo, nei due ordini: srotolano TUTTE o NESSUNA.
 *     Quindi gli scrittori di buffer sono DUE, non uno -- vedi c_ace34c --
 *     e la scelta e' per SITO. Misurato: instradando tutti i siti con array
 *     si va a 42 su 56 (peggio), instradando solo i cinque che migliorano si
 *     va a 45, e saltando dentro quei cinque i siti con lunghezza letterale
 *     maggiore di 8 -- che la fabbrica tiene a ciclo, "710192bf cmp"@0xffffff8008acc360
 *     per i 100 byte di rx_sram_updata -- si va a 46.
 *     Poi routine_thread ha mostrato che due dei suoi siti non sono buffer
 *     affatto ma parole (tutti e due i byte ripiegati in immediati), e con
 *     quelli su c_acab34 va a 47.
 *
 *     RESTANO SEI FUNZIONI, 80 byte: MT5725_otp_write -28 (divergenza 8,
 *     gia' dichiarata non ottenibile dal sorgente), MT5725_run_pgm_fw -16,
 *     MT5725_otp_read -12, mt5725_charger_work_func -12 (divergenza 6),
 *     MT5725_fsk_handle -8, MT5725_send_ppp -4. Censite le chiamate di
 *     tutte e tre le piu' grosse: NESSUNA differenza. Cio' che resta e'
 *     allocazione di registri e rimaterializzazione, non forma del sorgente.
 *
 *     Cosa era gia' stato provato e NON cambia niente, che resta vero: cinque
 *     forme di ciclo, tre tipi di buffer, `always_inline`, l'ordine di
 *     definizione, -O1/-O2/-O3/-Os/-Oz, -fno-unroll-loops, -fwrapv,
 *     il nuovo pass manager, e `#pragma clang loop unroll(disable)`.
 *
 *  3. IL QUARTO PARAMETRO DI MT5725_otp_read. Nel binario di fabbrica non e'
 *     letto da nessuna istruzione del corpo, eppure tutti e tre i chiamanti
 *     lo caricano ("320003e3 orr"@0xffffff8008ace58c). Nella nostra
 *     ricostruzione, non essendo usato, LLVM lo sostituisce con `undef` ai siti
 *     di chiamata (RemoveDeadArgumentsFromCallers) e l'istruzione sparisce:
 *     costa 1 istruzione in mt5725_get_otp e 2 in MT5725_otp_process.
 *     In fabbrica quel parametro DEVE avere un uso che qui non si e' trovato.
 *
 *  4. MT5725_irq_handle: la DIMENSIONE combacia (1292), ma undici istruzioni
 *     sono in ordine diverso -- lo stesso insieme di istruzioni, schedulato
 *     altrimenti, a cavallo fra il ramo del bit 8 e quello del bit 9. Costo
 *     zero byte.
 *
 *  5. mt5725_charger_routine_thread -4: la fabbrica carica DUE VOLTE il
 *     globale 0xffffff800a11101c ("b9401f89 ldr"@0xffffff8008acde38 e
 *     "b9401f88 ldr"@0xffffff8008acde3c) senza scritture in mezzo; la nostra
 *     compilazione fonde i due carichi. 1 istruzione.
 *
 *  6. mt5725_charger_work_func -12 (3 istruzioni: la fabbrica rimaterializza
 *     l'indirizzo di mt5725_g111024 dentro ogni ramo, noi riusiamo quello
 *     gia' in registro) e set_reg -8 (2 istruzioni: la nostra compilazione
 *     fonde le code identiche dei due rami del `case`, la fabbrica no).
 *     Nessuna delle due tocca la semantica.
 *
 *  7. SizeofFskPkt INCORPORATA. Fuori linea e' IDENTICA (88 byte, e le
 *     quattro `ret` separate ci sono in tutte e due). Incorporata no: la
 *     fabbrica fonde le code dei tre rami in una sola `add`+`lsr`+`add` con
 *     le costanti scelte da `csel` ("1a89314a csel"@0xffffff8008acbf98,
 *     "1a882509 cinc"@0xffffff8008acbf9c, "1a8b3188 csel"@0xffffff8008acbfa0),
 *     noi teniamo i tre rami separati.
 *     ATTENZIONE, ed e' un DIFETTO DI UNO STRUMENTO, non di questo file:
 *     `verificacitazioni.py` segna quella `cinc` NON_ANCORATA e chiude a
 *     125 verificate su 126 citate. La causa e' che `MNEMONICI` in
 *     `confinecitazioni.py` elenca `csinc` ma non il suo alias `cinc` (ne'
 *     `cinv`, ne' `cneg`), che e' cio' che objdump stampa davvero; percio'
 *     quella citazione non viene riconosciuta come istruzione e viene
 *     cercata fra i letterali. `verificaistruzioni.py`, che la riconosce, la
 *     CONFERMA insieme alle altre 393. E' un delta su uno strumento
 *     CONDIVISO e questo lotto non lo applica: sta nel rapporto. Provata anche la forma a variabile
 *     unica e `return` unico: stesso risultato NOSTRO, perche' dopo
 *     SimplifyCFG l'IR e' lo stesso. Costa dentro MT5725_send_ppp (-16) e
 *     dentro MT5725_fsk_handle.
 *
 *  8. MT5725_otp_write -60 (15 istruzioni). La fabbrica DUPLICA le quattro
 *     `strh` dell'intestazione dentro i due rami della scelta della
 *     lunghezza del blocco, e in ORDINE DIVERSO nei due
 *     ("790057f6 strh"@0xffffff8008acb5fc nel ramo da 128,
 *     "79005bf3 strh"@0xffffff8008acb61c in quello corto); da noi
 *     SimplifyCFG le fonde in una copia sola e la scelta diventa un `csel`.
 *     Provato a scriverle a mano nei due rami, negli ordini che il binario
 *     mostra: clang le rifonde lo stesso (-64, cioe' peggio), quindi la forma
 *     duplicata NON e' ottenibile dal sorgente e la versione qui e' quella
 *     semplice. Restano ~7 istruzioni di differenza fra spill e riuso di
 *     registri, non attribuite singolarmente.
 *
 * ------------------------------------------------------ COSA NON C'E' QUI
 *  - LE QUATTRO OPS power_supply: divergenza 1. Stanno in
 *    `mtk_chg_type_det.c`, che ESISTE in ALPS e NON va riscritto: il rapporto
 *    del lotto dice cosa andrebbe aggiunto.
 *  - `is_usb_rdy`: e' una definizione weak dell'header, scartata dal linker.
 *    Va INCLUSA, non riscritta. Vedi la nota accanto all'#include.
 *  - MODULE_LICENSE / MODULE_DESCRIPTION / MODULE_DEVICE_TABLE: vedi la nota
 *    in fondo al file.
 *  Tutte e 57 le funzioni del blocco che stanno in QUESTO file oggetto sono
 *  scritte. Nessuno stub: non ce n'e' piu' bisogno.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/power_supply.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/pm_wakeup.h>
#include <linux/sysfs.h>
#include <mt-plat/mtk_battery.h>
#include <mt-plat/charger_class.h>
#include <mt-plat/mtk_boot_common.h>

/*
 * `is_usb_rdy` NON va riscritta: e' una definizione weak emessa da
 * `mtk_charger_intf.h` e scartata dal linker. La sua copia scartata di 40 byte
 * sta a 0xffffff8008ace8c8, cioe' IN CODA alla TU di mt5725 — il che prova che
 * il sorgente di fabbrica include quell'header.
 *
 * Queste tre stringhe NON sono citate nella forma verificabile perche' non
 * sono letterali di questo file: due stanno nell'header incluso, la terza la
 * produce la macro `mutex_init` stringando il proprio argomento.
 * `verificacitazioni.py` ancora ogni citazione a un letterale del CODICE, e
 * fa bene; qui il modo giusto di riverificarle e':
 *   ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf \
 *       0xffffff80091504cb 0xffffff80091504e0 0xffffff8009263377
 * che restituisce b'\x016%s is not defined\n', b'is_usb_rdy' e b'&chip->slock'.
 */
#include "../mtk_charger_intf.h"

/* I due blob di firmware, estratti con uno script dai byte dell'oracolo. */
#include "mt5725_fw.h"

/*
 * Il binario NOMINA questa union e il suo membro, dentro il messaggio che
 * mt5725_charger_routine_thread stampa:
 *   " MT5725_write_buffer(mte, REG_VFC, temp.ptr, 2)\n"@0xffffff80092637fe
 * `temp` e' il nome della variabile, `ptr` il nome del membro a byte. Il
 * nome del membro a 16 bit NON e' nominato dal binario: si chiama `c0`.
 */
union mt5725_u16 {
	u16	c0;
	u8	ptr[2];
};

/* ------------------------------------------------------------------------ */
/* Lo stato del chip.
 *
 * Dimensione 832 = 0x340, letta dalla devm_kzalloc di MT5725_probe:
 *   "52806801 mov"@0xffffff8008acd378   (w1 = 0x340 = 832)
 *   "97e89237 bl"@0xffffff8008acd384    (devm_kmalloc)
 *
 * I nomi dei campi che il binario NON nomina sono `c<offset decimale>`
 * (regola 5). L'unico campo che il binario nomina e' `slock`, dalla macro
 * `mutex_init` che stringa il proprio argomento -- la stringa prodotta sta a
 * 0xffffff8009263377 e si legge con `leggi_stringa.py` (vedi la nota su
 * is_usb_rdy piu' sopra: non e' un letterale di questo file).
 */
struct mt5725_chip {
	u8			c0[8];
	/* +0x08 "a900d113 stp"@0xffffff8008acd450 (x19 = i2c_client) */
	struct i2c_client	*c8;
	/* +0x10 argomento di dev_err: "f9400a60 ldr"@0xffffff8008acabb0 */
	struct device		*c16;
	/* +0x18 argomento di regmap_read: "f9400c00 ldr"@0xffffff8008acae00 */
	struct regmap		*c24;
	/* +0x20 / +0x28 "a9022909 stp"@0xffffff8008acd420 (MT5725_read, MT5725_write) */
	int			(*c32)(struct mt5725_chip *chip, u16 reg, u8 *val);
	int			(*c40)(struct mt5725_chip *chip, u16 reg, u8 val);
	/* +0x30 / +0x38 "a903310b stp"@0xffffff8008acd454 (read_buffer, write_buffer) */
	int			(*c48)(struct mt5725_chip *chip, u16 reg, u8 *buf, u32 len);
	int			(*c56)(struct mt5725_chip *chip, u16 reg, u8 *buf, u32 len);
	/* +0x40 "b90042a0 str"@0xffffff8008acd50c (gpio di "dc-gpio") */
	int			c64;
	u8			c68[4];
	/* +0x48 "b9004aa0 str"@0xffffff8008acd594 (gpio di "eint_wpc") */
	int			c72;
	u8			c76[4];
	/* +0x50 "b9405108 ldr"@0xffffff8008ace8b0 (letto da Mt5725_get_rxdetect) */
	int			c80;
	u8			c84[4];
	/* +0x58 "97d97c4e bl"@0xffffff8008acd3c4 (__mutex_init su chip+0x58) */
	struct mutex		slock;
	/* +0x78 "9101e10a add"@0xffffff8008acd484, func = MT5725_int_delayed_work_func */
	struct delayed_work	c120;
	/* +0xd8 "9103810b add"@0xffffff8008acd488, func = mt5725_charger_work_func */
	struct delayed_work	c216;
	/* +0x138 "9104e2a8 add"@0xffffff8008acd4f8, func = MT5725_good_int_delayed_work_func */
	struct delayed_work	c312;
	u8			c408[4];
	/* +0x19c "b9019d1f str"@0xffffff8008acd45c (azzerato dalla probe) */
	int			c412;
	u8			c416[400];
	/* +0x330 "b9033114 str"@0xffffff8008ace520 (scritto da mt5725_set_otg) */
	int			c816;
	/* +0x334 "b9033509 str"@0xffffff8008ace6bc (scritto da mt5725_set_otp) */
	int			c820;
	/* +0x338 "b9433903 ldr"@0xffffff8008ace298 (letto da mt5725_get_reverse_charger) */
	int			c824;
	u8			c828[4];
};

/* ------------------------------------------------------------------------ */
/* I globali.
 *
 * Tre di questi ATTRAVERSANO il confine dell'oggetto e quindi NON sono
 * `static`: se lo fossero, clang restringerebbe gli accessi a un byte
 * (`ldrb`+`tbz`) e il binario carica invece a larghezza piena (`ldr w`).
 *   mt5725_wls_online  (0xffffff800a110fe4) letto anche da
 *                      tcpc_typec_handle_timeout: "b94fe508 ldr"@0xffffff80088f6e50
 *   mt5725_rvs_online  (0xffffff800a110fec) letto anche da
 *                      usb_insert_detect_thread_kthread
 *   usb_insert_stato   (0xffffff800a110ff4) e' POSSEDUTO da usb_insert_detect
 *                      ("b94ff508 ldr"@0xffffff8008ace8f4) e solo LETTO da qui.
 *
 * IL NOME DI QUESTA TERZA ERA SBAGLIATO, E IL LINK NON LO POTEVA DIRE. Qui
 * c'era scritto `extern int usb_insert_online`, cioe' il nome della FUNZIONE
 * a 0xffffff8008ace8f0 dato alla VARIABILE a 0xffffff800a110ff4: sono due
 * oggetti diversi, e il binario li distingue -- la funzione e' `T` nella
 * mappa, la variabile non c'e' affatto perche' la mappa non ha simboli di
 * dato.
 *
 * Il C non se ne accorge: una dichiarazione di variabile si lega senza una
 * parola a un simbolo di funzione, e `if (usb_insert_online)` diventa il test
 * dell'INDIRIZZO della funzione, che non e' mai zero. Il ramo `else` di
 * `mt5725_reverse_charge` -- quello che chiama `mt_charger_set_opa_mode` e
 * `charger_dev_set_boost_current_limit` -- non si sarebbe preso MAI.
 *
 * E finche' `usb_insert_detect` non era agganciato al build il link falliva
 * con "undefined reference", che era il sintomo giusto; agganciandolo il link
 * ha smesso di lamentarsi e il difetto e' diventato invisibile. Un link che
 * riesce non e' una prova che i nomi siano quelli giusti.
 *
 * I nomi `wls_online`/`rvs_online` non sono letti dal binario (la mappa non ha
 * simboli di dato): sono SCELTE. Cio' che e' MISURATO e' la larghezza (4 byte),
 * chi legge e chi scrive, e il fatto che escano dall'oggetto.
 */
int mt5725_wls_online;			/* 0xffffff800a110fe4 */
int mt5725_rvs_online;			/* 0xffffff800a110fec */
extern int usb_insert_stato;		/* 0xffffff800a110ff4, di usb_insert_detect */

/*
 * NON `static`: MT5725_good_irq lo carica a LARGHEZZA PIENA
 *   "b94fe908 ldr"@0xffffff8008acdd60
 * e MT5725_good_int_delayed_work_func lo azzera a larghezza piena
 *   "b90fe91f str"@0xffffff8008acdbe0
 * Se avesse collegamento interno clang lo restringerebbe a un byte
 * (`ldrb`/`strb`), perche' assume solo i valori 0 e 1.
 */
int mt5725_good_irq_ready;		/* 0xffffff800a110fe8 */
static struct mt5725_chip *mt5725_chip_p;	/* 0xffffff800a111000 */
static struct mt5725_chip *mt5725_chip_q;	/* 0xffffff800a111008 */
static int mt5725_fastcharge_timeout;	/* 0xffffff800a111010 */
u8 mt5725_fastcharge_state;		/* 0xffffff800a110ff8 */
/*
 * NON `static`: mt5725_set_hwen ci scrive e NESSUNO in tutta l'immagine lo
 * legge. Se fosse `static`, il compilatore cancellerebbe la scrittura (e la
 * variabile), mentre il binario di fabbrica la esegue:
 *   "b9002d28 str"@0xffffff8008ace73c
 */
int mt5725_hwen;			/* 0xffffff800a11102c */
static int mt5725_irq_num;		/* 0xffffff800a111070 */
static int mt5725_good_irq_num;		/* 0xffffff800a111074 */
static struct charger_device *mt5725_chg_dev;	/* 0xffffff800a111090 */
/*
 * Un solo oggetto: il binario prende l'indirizzo di 0xffffff800a111024 UNA
 * volta e legge [x9] e scrive [x9,#4]
 *   "91009129 add"@0xffffff8008acd984
 *   "b9400135 ldr"@0xffffff8008acd988
 *   "b9000528 str"@0xffffff8008acd990
 * Se fossero due variabili distinte ci sarebbero due coppie adrp/:lo12:.
 *
 * NON `static`: l'elemento [1] non e' letto da NESSUNO in tutta l'immagine, e
 * se l'oggetto avesse collegamento interno il compilatore cancellerebbe le sue
 * scritture. Il binario le esegue: "b9000528 str"@0xffffff8008acd990 e
 * "b9000509 str"@0xffffff8008acda08.
 */
int mt5725_g111024[2];			/* 0xffffff800a111024, 0xffffff800a111028 */
/* nome MISURATO: "rxdetect_count = 0\n"@0xffffff80092637ea */
static int mt5725_rxdetect_count;	/* 0xffffff800a111168 */
/* SENZA SEGNO: il confronto con 2 in mt5725_charger_routine_thread e'
 * "54000083 b.cc"@0xffffff8008acde44, non un b.lt */
static volatile unsigned int mt5725_g11101c;	/* 0xffffff800a11101c */
/* letti a UN byte da MT5725_fastcharge_change: "39405109 ldrb"@0xffffff8008acc8a8
 * e "39406109 ldrb"@0xffffff8008acc8d0 */
static u8 mt5725_g111014;		/* 0xffffff800a111014 */
static u8 mt5725_g111018;		/* 0xffffff800a111018 */
/* scritto da MT5725_irq_handle e da NESSUN altro; nessuno lo legge in tutta
 * l'immagine: "39008109 strb"@0xffffff8008accdfc */
u8 mt5725_g111020;			/* 0xffffff800a111020 */
static int mt5725_last_chrg_current;	/* 0xffffff800a111150 */
static struct input_dev *mt5725_input;	/* 0xffffff800a111158 */
static struct wakeup_source mt5725_ws;	/* 0xffffff800a111098 */
static int mt5725_rt_mode_pin;		/* 0xffffff800a111160 */

/* ------------------------------------------------------------------------ */
/*
 * MT5725_otp_write e' `t` nella mappa dell'oracolo, cioe' STATIC, ed e'
 * chiamata da MT5725_write_otpok_flag, che nel binario la PRECEDE
 * (0xffffff8008acb4dc contro 0xffffff8008acb53c): serve la dichiarazione
 * in avanti.
 */
static void MT5725_otp_write(u32 addr, u8 *buf, u32 len);


/*
 * DELTA DI HEADER (non fatto qui, per la regola sui file condivisi):
 * `mt_charger_set_opa_mode` e' chiamata da mt5725_set_otg, mt5725_get_otp e
 * MT5725_otp_process ("97ec1175 bl"@0xffffff8008ace4fc) ma nessun header di
 * ALPS la dichiara. Come fa gia' `ilitek_bus.c` per il gruppo E, la si
 * dichiara LOCALMENTE qui e si descrive il delta nel rapporto.
 */
extern int mt_charger_set_opa_mode(struct charger_device *chg_dev, bool en);

/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/*
 * NOTA STORICA, ora rientrata. Finche' MT5725_otp_write non era scritta,
 * MT5725_run_pgm_fw restava senza chiamanti in questo file e clang, a -O2,
 * l'avrebbe cancellata: serviva `__used` per poterla MISURARE, ed era una
 * divergenza di compilazione dichiarata. Ora MT5725_otp_write c'e' e la
 * chiama ("97fffee3 bl"@0xffffff8008acb584), quindi `__used` e' stato TOLTO e
 * la misura non ha piu' quel margine.
 */

/*
 * MT5725_read_buffer -- 0xffffff8008acadf8, 32 byte.
 * `reg` e' u16: il corpo lo maschera prima di allargarlo a `unsigned int`
 *   "12003c21 and"@0xffffff8008acae04
 * `len` e' un 32 bit SENZA segno: l'allargamento a 64 bit e' `mov w3,w3`,
 * non `sxtw`
 *   "2a0303e3 mov"@0xffffff8008acae08
 *   "97e90f78 bl"@0xffffff8008acae0c   (regmap_bulk_read)
 */
static int MT5725_read_buffer(struct mt5725_chip *chip, u16 reg, u8 *buf, u32 len)
{
	return regmap_bulk_read(chip->c24, reg, buf, len);
}

/*
 * MT5725_write -- 0xffffff8008acd8e0, 76 byte.
 *   "12003c21 and"@0xffffff8008acd8f4   (reg u16)
 *   "12001c42 and"@0xffffff8008acd8f8   (val u8)
 *   "97e8ff3d bl"@0xffffff8008acd8fc    (regmap_write)
 *   "36f800c0 tbz"@0xffffff8008acd904   (if (ret < 0))
 *   "MT5725 write error: %d\n"@0xffffff8009263201
 */
static int MT5725_write(struct mt5725_chip *chip, u16 reg, u8 val)
{
	int ret;

	ret = regmap_write(chip->c24, reg, val);
	if (ret < 0)
		dev_err(chip->c16, "MT5725 write error: %d\n", ret);

	return ret;
}

/*
 * MT5725_read -- 0xffffff8008acd87c, 100 byte.
 * Il canarino c'e' perche' l'indirizzo del locale ESCE nella chiamata
 * (stack-protector-strong, CONFIG_CC_STACKPROTECTOR_STRONG=y in fabbrica).
 *   "97e9026e bl"@0xffffff8008acd8a8    (regmap_read)
 *   "37f80060 tbnz"@0xffffff8008acd8ac  (if (ret >= 0) *val = ...)
 *   "39000268 strb"@0xffffff8008acd8b4
 */
static int MT5725_read(struct mt5725_chip *chip, u16 reg, u8 *val)
{
	unsigned int v;
	int ret;

	ret = regmap_read(chip->c24, reg, &v);
	if (ret >= 0)
		*val = v;

	return ret;
}

/*
 * MT5725_write_buffer -- 0xffffff8008acabe8, 140 byte.
 * Il puntatore a funzione `chip->c40` e' RILETTO a ogni giro: la `ldr` sta
 * alla TESTA del ciclo, non prima
 *   "f94016a8 ldr"@0xffffff8008acac10
 *   "39400282 ldrb"@0xffffff8008acac14
 *   "d63f0100 blr"@0xffffff8008acac20
 *   "34000223 cbz"@0xffffff8008acabfc    (len == 0 -> ret = 0)
 */
static int MT5725_write_buffer(struct mt5725_chip *chip, u16 reg, u8 *buf, u32 len)
{
	int ret = 0;
	u32 i;

	/*
	 * LA FORMA DEL CICLO E' MISURATA, non scelta. Tre cose insieme, e
	 * ognuna vale byte:
	 *
	 * 1. NON E' RUOTATO. Il confronto di lunghezza sta in TESTA
	 *    ("7100069f cmp"@0xffffff8008aca904, dentro MT5725_soft_reset dove
	 *    len vale 1) e l'arco all'indietro e' il test d'errore
	 *    ("36fffee0 tbz"@0xffffff8008aca928). Scritto come
	 *    `for (i = 0; i < len; i++)` il latch diventa il test di lunghezza,
	 *    LoopUnroll ne ricava un numero di giri costante e srotola: e' il
	 *    motivo per cui soft_reset misurava -52 e le enable_* -28.
	 *
	 * 2. IL REGISTRO E IL BUFFER SONO DUE INDUTTIVI, non `reg + i` e
	 *    `buf[i]`. La fabbrica incrementa il registro a parte
	 *    ("11000436 add"@0xffffff8008acc68c, w22 = w1 + 1, poi
	 *    "2a1603e1 mov"@0xffffff8008acc698). Con `reg + i` clang lo
	 *    ricalcola dall'indice in una sola istruzione e il prologo cambia,
	 *    perche' gli serve un registro salvato in meno.
	 *
	 * 3. IL PUNTATORE A FUNZIONE `chip->c40` E' RILETTO A OGNI GIRO
	 *    ("f9401668 ldr"@0xffffff8008acc680, dentro il ciclo).
	 *
	 * La copia FUORI LINEA (0xffffff8008acabe8, 140 byte) e' esatta con
	 * questa forma come lo era con la precedente.
	 *
	 * ATTENZIONE: NON tutti i siti che scrivono due byte passano di qui.
	 * Quelli che scrivono una PAROLA da 16 bit passano da `c_acab34`, che
	 * non tocca la pila. Confondere le due cose e' costato una conclusione
	 * sbagliata, scritta e poi ritirata: vedi la divergenza 2 in testa al
	 * file.
	 */
	i = 0;
	while (ret >= 0) {
		if (i == len)
			return ret;
		ret = chip->c40(chip, reg, *buf);
		i++;
		reg++;
		buf++;
	}
	dev_err(chip->c16, "MT5725 write error: %d\n", ret);

	return ret;
}


/*
 * c_acab34 -- LA SECONDA FUNZIONE DI SCRITTURA, quella che scrive una parola
 * da 16 bit come due byte.  Il binario NON la lascia fuori linea: e'
 * incorporata a tutti i siti, e nell'intervallo del driver non resta nessun
 * simbolo di fabbrica senza corrispondenza (verificato). Quindi il nome e'
 * l'INDIRIZZO della prima copia incorporata, dentro `fast_vfc`, e non un nome
 * descrittivo: un nome descrittivo si leggerebbe come un fatto misurato.
 *
 * Che esista, invece, e' misurato e non e' opinabile. In `fast_vfc` la
 * fabbrica NON TOCCA LA PILA: il byte basso esce dal registro
 *   "2a1403e2 mov"@0xffffff8008acab38    (w2 = w20, il valore a 16 bit)
 * e il byte alto pure
 *   "53083e82 ubfx"@0xffffff8008acab50   (w2 = (w20 >> 8) & 0xff)
 * mentre MT5725_write_buffer un buffer in memoria ce l'ha sempre -- a
 * 0xffffff8008acc674 lo scrive ("790013e8 strh") e a 0xffffff8008acc684 lo
 * rilegge ("38746aa2 ldrb", base fissa piu' indice).
 */
static int c_acab34(struct mt5725_chip *chip, u16 reg, u16 val)
{
	int ret;

	ret = chip->c40(chip, reg, val);
	if (ret >= 0)
		ret = chip->c40(chip, reg + 1, val >> 8);
	if (ret < 0)
		dev_err(chip->c16, "MT5725 write error: %d\n", ret);

	return ret;
}

/*
 * c_ace34c -- IL SECONDO SCRITTORE DI BUFFER, quello che il compilatore
 * SROTOLA. Il nome e' l'indirizzo della prima copia incorporata, dentro
 * mt5725_set_reverse_charger: la fabbrica non lascia il simbolo.
 *
 * Che ce ne siano due e' MISURATO, non dedotto. Nell'immagine gli stessi due
 * byte scritti allo stesso registro escono in due forme diverse:
 *   - a ciclo, con il confronto in testa e il test d'errore come arco
 *     all'indietro: "7100069f cmp"@0xffffff8008aca904 (soft_reset, len 1) e
 *     "71000a9f cmp"@0xffffff8008acc678 (enable_apple, len 2);
 *   - srotolato, con il primo byte costante e il secondo RILETTO dalla pila
 *     dopo la chiamata: "52800ce2 mov"@0xffffff8008ace350 e
 *     "394007e2 ldrb"@0xffffff8008ace364 (set_reverse_charger, len 2).
 * Stesso compilatore, stesso oggetto, stessa lunghezza. Provate quattro
 * forme di ciclo che mettono i due test nella condizione, nei due ordini:
 * srotolano TUTTE o NESSUNA, mai le une si' e le altre no. Quindi la
 * differenza e' nel sorgente.
 *
 * La regola che il binario detta, e che qui e' applicata: i siti che passano
 * un ARRAY vanno di qui; quelli che passano il puntatore a uno SCALARE
 * (&val, (u8 *)&val) restano su MT5725_write_buffer.
 */
static int c_ace34c(struct mt5725_chip *chip, u16 reg, u8 *buf, u32 len)
{
	int ret = 0;
	u32 i;

	for (i = 0; i < len && ret >= 0; i++) {
		ret = chip->c40(chip, reg, *buf);
		reg++;
		buf++;
	}
	if (ret < 0)
		dev_err(chip->c16, "MT5725 write error: %d\n", ret);

	return ret;
}

/*
 * MT5725_soft_reset -- 0xffffff8008aca8d4, 136 byte.
 * MT5725_write_buffer e' INCORPORATA qui: il ciclo e' lo stesso, con len=1
 *   "7100069f cmp"@0xffffff8008aca904   (w20 contro 1)
 *   "528a4001 mov"@0xffffff8008aca8f8   (registro 0x5200)
 *   "321903e8 orr"@0xffffff8008aca8f4   (valore 0x80)
 *   "52801900 mov"@0xffffff8008aca940   (msleep(200))
 * Il locale NON e' un array: se lo fosse, stack-protector-strong metterebbe
 * un canarino, e qui non c'e'.
 */
void MT5725_soft_reset(void)
{
	u8 val = 0x80;

	MT5725_write_buffer(mt5725_chip_p, 0x5200, &val, 1);
	msleep(200);
}

/*
 * mt5725_reverse_charge -- 0xffffff8008aca95c, 424 byte.
 * MT5725_soft_reset e' INCORPORATA in tutti e due i rami (il ciclo di
 * MT5725_write_buffer con 0x80 al registro 0x5200, seguito da msleep(200)).
 *   "360003f3 tbz"@0xffffff8008aca984   (si guarda il solo bit 0: il
 *                                        parametro e' un `bool`, non un int)
 *   "b94ff508 ldr"@0xffffff8008aca994   (0xffffff800a110ff4 = usb_insert_online,
 *                                        globale POSSEDUTO da usb_insert_detect)
 *   "52923801 mov"@0xffffff8008acaa8c / "72a00421 movk"@0xffffff8008acaa90
 *                                        (0x2191c0 = 2.200.000)
 *   "529c6c01 mov"@0xffffff8008acaacc / "72a002c1 movk"@0xffffff8008acaad0
 *                                        (0x16e360 = 1.500.000)
 *   "39452108 ldrb"@0xffffff8008acaa9c  (0xffffff800a111148: e' il campo
 *                                        `active:1` della wakeup_source a
 *                                        0xffffff800a111098, letto come bit 0
 *                                        -- "37000088 tbnz"@0xffffff8008acaaa0)
 *   "b90fed28 str"@0xffffff8008acaaf8   (0xffffff800a110fec)
 */
void mt5725_reverse_charge(bool en)
{
	if (en) {
		gpio_set_value(mt5725_rt_mode_pin, 1);
		if (usb_insert_stato) {
			MT5725_soft_reset();
		} else {
			mt_charger_set_opa_mode(mt5725_chg_dev, 1);
			charger_dev_set_boost_current_limit(mt5725_chg_dev,
							    2200000);
		}
		if (!mt5725_ws.active)
			__pm_stay_awake(&mt5725_ws);
		mt5725_rvs_online = 1;
	} else {
		gpio_set_value(mt5725_rt_mode_pin, 0);
		if (usb_insert_stato) {
			MT5725_soft_reset();
		} else {
			mt_charger_set_opa_mode(mt5725_chg_dev, 0);
			charger_dev_set_boost_current_limit(mt5725_chg_dev,
							    1500000);
		}
		__pm_relax(&mt5725_ws);
		mt5725_rvs_online = 0;
	}
}

/*
 * MT5725_power_status -- 0xffffff8008acab04, 8 byte.
 *   "320003e0 orr"@0xffffff8008acab04   (w0 = 1)
 */
int MT5725_power_status(void)
{
	return 1;
}

/*
 * is_reverse_charger_online -- 0xffffff8008acab0c, 12 byte.
 * Legge a larghezza piena il globale 0xffffff800a110fec
 *   "b94fed00 ldr"@0xffffff8008acab10
 */
int is_reverse_charger_online(void)
{
	return mt5725_rvs_online;
}

/*
 * download_code -- 0xffffff8008acae18, 4 byte: un solo `ret`.
 *   "d65f03c0 ret"@0xffffff8008acae18
 */
void download_code(void)
{
}

/*
 * updata_wireless_online -- 0xffffff8008acd2a4, 4 byte: un solo `ret`.
 *   "d65f03c0 ret"@0xffffff8008acd2a4
 */
void updata_wireless_online(void)
{
}

/*
 * mt5725_get_hwen -- 0xffffff8008ace6f0, 8 byte: ritorna 0.
 *   "aa1f03e0 mov"@0xffffff8008ace6f0
 */
static ssize_t mt5725_get_hwen(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	return 0;
}

/*
 * SizeofFskPkt -- 0xffffff8008acbeac, 88 byte.
 *
 * Il tipo di ritorno e' `u8`, DEDOTTO: le costanti sommate prima dello
 * scorrimento sono 0xfe0 / 0x780 / 0x320 invece di -32 / -128 / -224, cioe'
 * -32, -128 e -224 troncati rispettivamente a 12, 11 e 10 bit. E' esattamente
 * la riduzione che clang applica quando del risultato servono solo 8 bit.
 *   "113f8108 add"@0xffffff8008acbec8   (+0xfe0, poi lsr #4)
 *   "111e0108 add"@0xffffff8008acbee4   (+0x780, poi lsr #3)
 *   "110c8108 add"@0xffffff8008acbef4   (+0x320, poi lsr #2)
 * Il secondo confronto e' un test del bit di segno del byte, non un `cmp`:
 *   "13001c09 sxtb"@0xffffff8008acbec0
 *   "37f800a9 tbnz"@0xffffff8008acbec4
 * Gli scorrimenti sono `lsr` qui e `asr` dove la funzione e' incorporata in
 * MT5725_send_ppp ("1ac92949 asr"@0xffffff8008acbc50): e' lo stesso operatore
 * `>>` del sorgente, semplificato a `lsr` solo dove il valore e' noto positivo.
 */
u8 SizeofFskPkt(u8 header)
{
	if (header < 0x20)
		return 1;
	else if (header < 0x80)
		return ((header - 0x20) >> 4) + 2;
	else if (header < 0xe0)
		return ((header - 0x80) >> 3) + 8;
	else
		return ((header - 0xe0) >> 2) + 20;
}


/*
 * MT5725_send_ppp -- 0xffffff8008acbbe0, 312 byte.
 * SizeofFskPkt e MT5725_write_buffer sono entrambe INCORPORATE qui.
 *   "39400002 ldrb"@0xffffff8008acbbf4   (header = data[0])
 *   "321a03e1 orr"@0xffffff8008acbc64    (primo registro 0x40)
 *   "31000517 adds"@0xffffff8008acbc58   (len = size + 1; il `b.cs` che segue
 *                                         e' la guardia `len == 0` della
 *                                         MT5725_write_buffer incorporata)
 *   "321f07e1 orr"@0xffffff8008acbcc0    (secondo blocco: registro 0x06)
 *   "320003e2 orr"@0xffffff8008acbcc4    (valore 0x01 -> reg_cmd = 0x0001)
 */
void MT5725_send_ppp(u8 *data)
{
	u16 reg_cmd = 0x0001;

	c_ace34c(mt5725_chip_p, 0x0040, data, SizeofFskPkt(data[0]) + 1);
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
}
EXPORT_SYMBOL(MT5725_send_ppp);

/*
 * fast_vfc -- 0xffffff8008acab18, 208 byte.
 * Due MT5725_write_buffer incorporate, con il puntatore globale RILETTO fra
 * l'una e l'altra ("f94002b3 ldr"@0xffffff8008acab78).
 *   "52800441 mov"@0xffffff8008acab34    (registro 0x22)
 *   "53083e82 ubfx"@0xffffff8008acab50   (byte alto di vfc; e' `ubfx` e non
 *                                         `lsr` perche' i bit alti di w0 in
 *                                         ingresso non sono noti)
 *   "321903e2 orr"@0xffffff8008acab80    (valore 0x80 al registro 0x06)
 *   "%s,write reg_cmd : 0x%04x,\n"@0xffffff8009262d1a
 *   "fast_vfc"@0xffffff8009262d36
 */
void fast_vfc(u16 vfc)
{
	u16 reg_cmd = 0x0080;

	c_acab34(mt5725_chip_p, 0x0022, vfc);
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
	printk("%s,write reg_cmd : 0x%04x,\n", __func__, reg_cmd);
}


/*
 * fastcharge_afc -- 0xffffff8008acac74, 388 byte.
 *   "394007e8 ldrb"@0xffffff8008acacac / "36000908 tbz"@0xffffff8008acacb0
 *                                        (bit 0 del secondo byte del registro 0x0c)
 *   "52800502 mov"@0xffffff8008acacd0   (0x28) / "52800462 mov"@0xffffff8008acacec (0x23)
 *                                        -> 0x2328 ai registri 0x22/0x23
 *   "321903e2 orr"@0xffffff8008acad18   (0x80 al registro 0x0e)
 *   "52801402 mov"@0xffffff8008acad78   (0xa0 al registro 0x06)
 *   "MT5725 %s ,version 0.1 Tx support samsung_afc\n"@0xffffff8009262d3f
 *   "fastcharge_afc"@0xffffff8009262d6e
 *   "%s,version 0.1 write reg_clr : 0x%04x,\n"@0xffffff8009262d7d
 *   "%s,version 0.1 write reg_cmd : 0x%04x,\n"@0xffffff8009262da5
 */
void fastcharge_afc(void)
{
	u8 buf[8];
	u16 vfc, reg_clr, reg_cmd;

	MT5725_read_buffer(mt5725_chip_p, 0x000c, buf, 2);
	if (buf[1] & 0x01) {
		printk("MT5725 %s ,version 0.1 Tx support samsung_afc\n",
		       __func__);
		vfc = 0x2328;
		c_acab34(mt5725_chip_p, 0x0022, vfc);
		reg_clr = 0x0080;
		c_acab34(mt5725_chip_p, 0x000e, reg_clr);
		printk("%s,version 0.1 write reg_clr : 0x%04x,\n", __func__,
		       reg_clr);
		reg_cmd = 0x00a0;
		c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
		printk("%s,version 0.1 write reg_cmd : 0x%04x,\n", __func__,
		       reg_cmd);
	}
}

/*
 * MT5725_send_EPT -- 0xffffff8008acbd18, 404 byte.
 * QUATTRO MT5725_write_buffer incorporate. Le tre su locali a 16 bit sono
 * SROTOLATE dal compilatore; quella sull'array di due byte resta un ciclo
 *   "71000adf cmp"@0xffffff8008acbdf8
 * -- e' la stessa asimmetria descritta fra le divergenze aperte.
 *   "321c03e2 orr"@0xffffff8008acbd4c   (0x10 al registro 0x0e)
 *   "321b03e2 orr"@0xffffff8008acbd94   (0x20 al registro 0x06)
 *   "52800c80 mov"@0xffffff8008acbdd4   (msleep(100))
 *   "390037f3 strb"@0xffffff8008acbde8  (buf[1] = ept)
 *   "390033e8 strb"@0xffffff8008acbdf4  (buf[0] = 0x02)
 *   "321a03e1 orr"@0xffffff8008acbdec   (primo registro 0x40)
 *   "320003e2 orr"@0xffffff8008acbe3c   (0x01 al registro 0x06)
 */
void MT5725_send_EPT(u8 ept)
{
	u16 reg_clr, reg_cmd;
	u8 buf[12];

	reg_clr = 0x0010;
	c_acab34(mt5725_chip_p, 0x000e, reg_clr);
	reg_cmd = 0x0020;
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
	msleep(100);
	buf[0] = 0x02;
	buf[1] = ept;
	MT5725_write_buffer(mt5725_chip_p, 0x0040, buf, 2);
	reg_cmd = 0x0001;
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
}

/*
 * MT5725_fsk_handle -- 0xffffff8008acbf04, 1020 byte.
 *
 * SizeofFskPkt e' INCORPORATA (stessa cascata di confronti che sta anche in
 * MT5725_send_ppp), e MT5725_read_buffer pure (chiama regmap_bulk_read diretto).
 *   "52800ac1 mov"@0xffffff8008acbf2c   (registro 0x56, letto DUE volte)
 *   "394003f3 ldrb"@0xffffff8008acbf4c  (header = buf[0], preso fra le due)
 *   "52800ae1 mov"@0xffffff8008acbfc8   (registro 0x57, il byte del pacchetto)
 *   "320003f5 orr"@0xffffff8008acbf3c   (il valore 1 di SizeofFskPkt, issato
 *                                        prima della prima lettura)
 *   "72001ebf tst"@0xffffff8008acbfb0   (guardia del ciclo: size a UN byte)
 *   "380016c8 strb"@0xffffff8008acbfe0  (data[i] = buf[0], puntatore che avanza)
 *   "71007e7f cmp"@0xffffff8008acbfe8   (header == 0x1f, altrimenti si esce)
 *   "7103751f cmp"@0xffffff8008acbff4   (data[0] == 0xdd)
 *   "7103b91f cmp"@0xffffff8008acbffc   (data[0] == 0xee)
 *
 * I due rami fanno la stessa cosa con due coppie di registri diverse e un byte
 * di marca diverso:
 *   0xee: legge 0x001a e 0x0018 ("52800341 mov"@0xffffff8008acc0b0,
 *         "321d07e1 orr"@0xffffff8008acc0e0), marca "52801dc8 mov"@0xffffff8008acc0c8
 *   0xdd: legge 0x0004 e 0x0003 ("321e03e1 orr"@0xffffff8008acc1e4,
 *         "320007e1 orr"@0xffffff8008acc214), marca "52801ba8 mov"@0xffffff8008acc1fc
 * In tutti e due i byte letti finiscono nel pacchetto SCAMBIATI: prima il
 * secondo, poi il primo
 *   "394007e9 ldrb"@0xffffff8008acc0bc / "394003ea ldrb"@0xffffff8008acc0c0
 *   "39002be9 strb"@0xffffff8008acc0d0 / "39002fea strb"@0xffffff8008acc0d4
 *
 * Le due `MT5725_write_buffer` da sei byte restano a CICLO
 *   "71001abf cmp"@0xffffff8008acc10c   (6 byte)
 *   "910023f6 add"@0xffffff8008acc100   (il pacchetto sta a sp+8)
 * mentre quelle da due byte sono srotolate (vedi divergenza 2).
 *   "321c03e2 orr"@0xffffff8008acc00c   (0x0010 al registro 0x0e)
 *   "321b03e2 orr"@0xffffff8008acc054   (0x0020 al registro 0x06)
 *   "52800c80 mov"@0xffffff8008acc094   (msleep(100))
 *   "320003e2 orr"@0xffffff8008acc284   (0x0001 al registro 0x06, in comune)
 *   "ATE send ASK TO TX\n"@0xffffff8009262e6d
 *
 * Le tre aree di pila si leggono dagli scostamenti: `buf` a sp+0, `tx` a sp+8,
 * `data` a sp+20, canarino a sp+120 ("f81c83a8 stur"@0xffffff8008acbf30, cioe'
 * x29-56 = sp+120). Le DIMENSIONI dei tre array non sono lette da nessuna
 * istruzione: sono SCELTE compatibili con quegli scostamenti (8, 12, 100), e
 * il ciclo prova solo che `data` ne regge almeno 27 (il massimo che
 * SizeofFskPkt puo' restituire).
 */
void MT5725_fsk_handle(void)
{
	u8 data[100];
	u8 tx[12];
	u8 buf[8];
	u8 header, size, i;
	u16 reg_clr, reg_cmd;

	MT5725_read_buffer(mt5725_chip_p, 0x0056, buf, 1);
	header = buf[0];
	size = SizeofFskPkt(header);
	MT5725_read_buffer(mt5725_chip_p, 0x0056, buf, 1);

	for (i = 0; i < size; i++) {
		MT5725_read_buffer(mt5725_chip_p, 0x0057, buf, 1);
		data[i] = buf[0];
	}

	if (header != 0x1f)
		return;

	if (data[0] == 0xee) {
		reg_clr = 0x0010;
		c_acab34(mt5725_chip_p, 0x000e, reg_clr);
		reg_cmd = 0x0020;
		c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
		msleep(100);
		tx[0] = 0x58;
		MT5725_read_buffer(mt5725_chip_p, 0x001a, buf, 2);
		tx[1] = 0xee;
		tx[2] = buf[1];
		tx[3] = buf[0];
		MT5725_read_buffer(mt5725_chip_p, 0x0018, buf, 2);
		tx[4] = buf[1];
		tx[5] = buf[0];
		MT5725_write_buffer(mt5725_chip_p, 0x0040, tx, 6);
	} else if (data[0] == 0xdd) {
		reg_clr = 0x0010;
		c_acab34(mt5725_chip_p, 0x000e, reg_clr);
		reg_cmd = 0x0020;
		c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
		msleep(100);
		tx[0] = 0x58;
		MT5725_read_buffer(mt5725_chip_p, 0x0004, buf, 2);
		tx[1] = 0xdd;
		tx[2] = buf[1];
		tx[3] = buf[0];
		MT5725_read_buffer(mt5725_chip_p, 0x0003, buf, 2);
		tx[4] = buf[1];
		tx[5] = buf[0];
		MT5725_write_buffer(mt5725_chip_p, 0x0040, tx, 6);
	} else {
		return;
	}

	reg_cmd = 0x0001;
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
	printk("ATE send ASK TO TX\n");
}

/*
 * MT5725_rx_sram_updata -- 0xffffff8008acc300, 700 byte. EXPORT_SYMBOL.
 *
 * Il buffer di due byte ESCE (la MT5725_read_buffer incorporata lo passa a
 * regmap_bulk_read): stack-protector-strong mette il canarino.
 *   "f9454508 ldr"@0xffffff8008acc31c   (__stack_chk_guard = 0xffffff800985ea88)
 *   "97d7872a bl"@0xffffff8008acc5b8    (__stack_chk_fail)
 *   "321e03e1 orr"@0xffffff8008acc328   (registro 0x0004, due byte letti)
 *
 * Il ramo si decide su TUTTI i bit di buf[1] TRANNE il bit 4: la maschera e'
 * 0xffffffef, cioe' l'intero `~0x10` promosso a `int`, non 0xef.
 *   "394007e8 ldrb"@0xffffff8008acc340  (buf[1])
 *   "721b791f tst"@0xffffff8008acc344
 *   "54000800 b.eq"@0xffffff8008acc348  (se zero -> ramo `else`, 0x…acc448)
 *
 * I due rami fanno le STESSE tre scritture, in ordine diverso e con attese
 * diverse: 4 ms se il test e' vero, 2 ms se e' falso.
 *   "321e03e0 orr"@0xffffff8008acc43c   (msleep(4), ramo vero)
 *   "321f03e0 orr"@0xffffff8008acc4e8   (msleep(2), ramo falso)
 *
 * I 100 byte scritti al registro 0x600 vengono da 0xffffff8008f85f80, che sta
 * OLTRE _etext (quindi in `.rodata`: e' `const`) e contiene SOLO ZERI. Il
 * carico resta nel binario perche' l'indice e' variabile: LLVM piega un carico
 * da un globale costante solo a indice costante.
 *   "913e02d6 add"@0xffffff8008acc35c   (0xffffff8008f85f80)
 *   "321707e1 orr"@0xffffff8008acc358   (registro 0x600)
 *   "710192bf cmp"@0xffffff8008acc360   (100 byte)
 *   "38756ac2 ldrb"@0xffffff8008acc36c  (indice variabile)
 *
 * Le tre scritture da due byte sono INCORPORATE E SROTOLATE (qui la fabbrica
 * srotola, al contrario di MT5725_enable_afc: vedi la divergenza 2 in testa
 * al file). Il secondo byte e' RILETTO dalla pila, perche' fra le due c'e' una
 * chiamata e il buffer e' uscito: nessuna propagazione e' lecita.
 *   "528ae4a8 mov"@0xffffff8008acc3a0   (0x5725 -> registri 0x08 e 0x09)
 *   "528004a2 mov"@0xffffff8008acc3b0   (primo byte 0x25, immediato)
 *   "394007e2 ldrb"@0xffffff8008acc3c4  (secondo byte, riletto da [sp,#1])
 *   "321a03e8 orr"@0xffffff8008acc3f0   (0x0040 -> registri 0x06 e 0x07)
 *   "790003ff strh"@0xffffff8008acc544  (0x0000 -> registri 0x06 e 0x07)
 *
 * `buf` e' di OTTO byte, non di due: sta a sp+0 e il canarino a sp+8
 *   "910003e2 mov"@0xffffff8008acc324   (l'indirizzo passato e' sp, non sp+N)
 *   "f90007e8 str"@0xffffff8008acc32c   (canarino a sp+8)
 * Con `u8 buf[2]` la nostra compilazione mette il buffer a sp+4 e legge
 * `[sp,#5]`; con `u8 buf[8]` cade a sp+0 e legge `[sp,#1]`, come la fabbrica.
 * E' quindi una dimensione MISURATA per via indiretta (la posizione dello
 * spazio di pila), non letta da un'istruzione che la nomini: i byte 2..7 non
 * sono mai toccati.
 */

/*
 * 0xffffff8008f85f80. Il campo di zeri arriva fino a 0xffffff8008f86048, dove
 * comincia l'`of_device_id`: 200 byte in tutto. Il ciclo ne legge 100
 * ("710192bf cmp"@0xffffff8008acc360), e 100 e' quindi la sola dimensione
 * MISURATA; se l'oggetto di fabbrica fosse di 200 byte non si distinguerebbe,
 * perche' il resto e' zero come tutto il campo. La scelta di 100 e' dichiarata.
 */
static const u8 mt5725_rx_sram[100] = { 0 };

void MT5725_rx_sram_updata(void)
{
	u8 buf[8];

	MT5725_read_buffer(mt5725_chip_p, 0x0004, buf, 2);
	if (buf[1] & ~0x10) {
		MT5725_write_buffer(mt5725_chip_p, 0x0600,
				    (u8 *)mt5725_rx_sram, 100);
		buf[0] = 0x25;
		buf[1] = 0x57;
		c_ace34c(mt5725_chip_p, 0x0008, buf, 2);
		buf[0] = 0x40;
		buf[1] = 0x00;
		c_ace34c(mt5725_chip_p, 0x0006, buf, 2);
		msleep(4);
	} else {
		buf[0] = 0x25;
		buf[1] = 0x57;
		c_ace34c(mt5725_chip_p, 0x0008, buf, 2);
		buf[0] = 0x40;
		buf[1] = 0x00;
		c_ace34c(mt5725_chip_p, 0x0006, buf, 2);
		msleep(2);
		MT5725_write_buffer(mt5725_chip_p, 0x0600,
				    (u8 *)mt5725_rx_sram, 100);
	}
	buf[0] = 0x00;
	buf[1] = 0x00;
	c_ace34c(mt5725_chip_p, 0x0006, buf, 2);
}
EXPORT_SYMBOL(MT5725_rx_sram_updata);

/*
 * MT5725_enable_afc -- 0xffffff8008acc5bc, 140 byte.
 *   "52800848 mov"@0xffffff8008acc5dc   (0x0042 -> registri 0x38, 0x39)
 *   "321d0be1 orr"@0xffffff8008acc5e0   (registro 0x38)
 *   "790013e8 strh"@0xffffff8008acc5e8  (un solo `strh`: il locale NON e' un
 *                                        array, altrimenti ci sarebbe il
 *                                        canarino di stack-protector-strong)
 *   "2078MT5725_enable_afc\n"@0xffffff8009262e81
 */
void MT5725_enable_afc(void)
{
	u16 val = 0x0042;

	MT5725_write_buffer(mt5725_chip_p, 0x0038, (u8 *)&val, 2);
	printk("2078MT5725_enable_afc\n");
}

/*
 * MT5725_enable_apple -- 0xffffff8008acc648, 128 byte. Nessun printk.
 *   "52800b48 mov"@0xffffff8008acc668   (0x005a)
 *   "321d0be1 orr"@0xffffff8008acc66c   (registro 0x38)
 */
void MT5725_enable_apple(void)
{
	u16 val = 0x005a;

	MT5725_write_buffer(mt5725_chip_p, 0x0038, (u8 *)&val, 2);
}

/*
 * MT5725_enable_epp -- 0xffffff8008acc6c8, 128 byte. Nessun printk.
 *   "529dddc8 mov"@0xffffff8008acc6e8   (0xeeee)
 *   "321d03e1 orr"@0xffffff8008acc6ec   (registro 0x08)
 */
void MT5725_enable_epp(void)
{
	u16 val = 0xeeee;

	MT5725_write_buffer(mt5725_chip_p, 0x0008, (u8 *)&val, 2);
}

/*
 * MT5725_SetFodPara -- 0xffffff8008accc00, 116 byte.
 * I 16 byte dei parametri FOD stanno in `.data` a 0xffffff80099a74c8, cioe'
 * SCRIVIBILI (subito prima della struct i2c_driver): non sono `const`.
 *   "911322b5 add"@0xffffff8008accc24   (indirizzo 0x…99a74c8)
 *   "7100429f cmp"@0xffffff8008accc28   (16 byte)
 *   "52800d01 mov"@0xffffff8008accc20   (primo registro 0x68)
 */
static u8 mt5725_fod_para[16] = {
	0x32, 0x51, 0x31, 0x42, 0x31, 0x3f, 0x31, 0x40,
	0x30, 0x41, 0x31, 0x2c, 0x0a, 0x0a, 0x00, 0x00,
};

void MT5725_SetFodPara(void)
{
	MT5725_write_buffer(mt5725_chip_p, 0x0068, mt5725_fod_para, 16);
}


/*
 * MT5725_run_pgm_fw -- 0xffffff8008acb110, 972 byte.
 * Le prime sette MT5725_write_buffer sono la STESSA sequenza di
 * MT5725_fastcharge_change (registri 0x5808 tre volte, 0x5800, 0x5244,
 * 0x5218 a due byte, 0x5208).
 *   "52806881 mov"@0xffffff8008acb398   (836 = dimensione di MT5725_pgm_fw)
 *   "9105c108 add"@0xffffff8008acb3e0   (0xffffff8008f82170 = MT5725_pgm_fw)
 *   "7102035f cmp"@0xffffff8008acb3bc / "1a883357 csel"@0xffffff8008acb3c4
 *                                       (blocchi da 128 byte, confronto SENZA segno)
 *   "11200261 add"@0xffffff8008acb3f4  (registro = 0x800 + scostamento)
 *   "8b334116 add"@0xffffff8008acb3e4  (&pgm_fw[scostamento], `uxtw`: lo
 *                                       scostamento e' SENZA segno)
 *   "52800140 mov"@0xffffff8008acb448   (msleep(10))
 *   "528011c8 mov"@0xffffff8008acb468   (0x8e al registro 0x5200)
 *   "len:%d\n"@0xffffff8009176797
 *   "length:%d\n"@0xffffff8009177070
 *   "offset:%d\n"@0xffffff8009161021
 */
static void MT5725_run_pgm_fw(void)
{
	u8 val;
	u16 val16;
	u32 offset = 0;
	u32 remaining = sizeof(MT5725_pgm_fw);
	u32 chunk;

	val = 0x95;
	MT5725_write_buffer(mt5725_chip_p, 0x5808, &val, 1);
	MT5725_write_buffer(mt5725_chip_p, 0x5808, &val, 1);
	MT5725_write_buffer(mt5725_chip_p, 0x5808, &val, 1);
	val = 0x01;
	MT5725_write_buffer(mt5725_chip_p, 0x5800, &val, 1);
	val = 0x57;
	MT5725_write_buffer(mt5725_chip_p, 0x5244, &val, 1);
	val16 = 0x420f;
	c_acab34(mt5725_chip_p, 0x5218, val16);
	val = 0x0e;
	MT5725_write_buffer(mt5725_chip_p, 0x5208, &val, 1);
	msleep(200);

	printk("len:%d\n", (int)sizeof(MT5725_pgm_fw));
	do {
		chunk = (remaining < 128) ? remaining : 128;
		printk("length:%d\n", chunk);
		c_ace34c(mt5725_chip_p, 0x800 + offset, (u8 *)&MT5725_pgm_fw[offset], chunk);
		offset += chunk;
		printk("offset:%d\n", offset);
		remaining -= chunk;
		msleep(10);
	} while (remaining);

	msleep(200);
	val = 0x8e;
	MT5725_write_buffer(mt5725_chip_p, 0x5200, &val, 1);
	msleep(200);
}


/*
 * MT5725_otp_read -- 0xffffff8008acae1c, 756 byte.
 * Legge l'OTP a blocchi di 128 byte. Il quarto parametro NON e' usato dal
 * corpo: nessuna istruzione tocca w3.
 *   "1101fc49 add"@0xffffff8008acae44   (len + 127)
 *   "53077d34 lsr"@0xffffff8008acae50   (>>7: numero di blocchi)
 *   "12196135 and"@0xffffff8008acae58   (&~0x7f: lunghezza arrotondata)
 *   "d379e388 lsl"@0xffffff8008acae9c   (blocco * 128)
 *   "321903e8 orr"@0xffffff8008acaf40   (0x0080 al registro 4)
 *   "321d03e8 orr"@0xffffff8008acaf90   (0x08 al registro 0, UN solo byte:
 *                                        "7100043f cmp"@0xffffff8008acaf9c)
 *   "7100211f cmp"@0xffffff8008acaffc   (stato == 8: ancora occupato)
 *   "7100091f cmp"@0xffffff8008acb040   (stato == 2: pronto)
 *   "7107d35f cmp"@0xffffff8008acb034   (500 giri di attesa)
 *   "321903e3 orr"@0xffffff8008acb05c   (128 byte letti dal registro 8)
 *   "f102029f cmp"@0xffffff8008acb084   (128 byte stampati)
 *   "MT5725_run_pgm_fw,length:%d\n"@0xffffff8009262dcd
 *   "5725status:%d"@0xffffff8009262dea
 *   "error! Read OTP TImeout\n"@0xffffff8009262df8
 *   "OtpRead:"@0xffffff8009262e11
 *   ",%x"@0xffffff8009262e1a
 *   "OtpRead l670 error\n"@0xffffff8009262e1e
 *   "\n"@0xffffff8009245994
 */
int MT5725_otp_read(u32 addr, u8 *buf, u32 len, int flag)
{
	union mt5725_u16 temp;
	u32 blocks = (len + 127) / 128;
	u32 i, j;

	/* IL QUARTO PARAMETRO NON E' LETTO DA NESSUNA ISTRUZIONE DEL CORPO --
	 * cercato: le uniche w3 della funzione sono argomenti che LEI passa
	 * ("321f03e3 orr"@0xffffff8008acafe4 e @0xffffff8008acb014,
	 * "321903e3 orr"@0xffffff8008acb05c). Eppure tutti e tre i chiamanti
	 * lo caricano ("320003e3 orr"@0xffffff8008ace58c). Lasciandolo
	 * davvero inutilizzato, DeadArgumentElimination lo sostituisce con
	 * undef ai siti di chiamata e quelle istruzioni spariscono: costa 1
	 * istruzione in mt5725_get_otp e 2 in MT5725_otp_process.
	 * L'asm vuoto lo tiene vivo senza emettere niente. E' UN NOSTRO
	 * ESPEDIENTE, non una lettura del binario: in fabbrica quel parametro
	 * ha un uso che qui non e' stato trovato. */
	asm volatile("" :: "r"(flag));

	MT5725_run_pgm_fw();
	printk("MT5725_run_pgm_fw,length:%d\n", (len + 127) & ~0x7f);

	for (i = 0; i < blocks; i++) {
		temp.c0 = 0;
		MT5725_write_buffer(mt5725_chip_p, 0x0000, temp.ptr, 2);
		temp.c0 = addr + i * 128;
		MT5725_write_buffer(mt5725_chip_p, 0x0002, temp.ptr, 2);
		temp.c0 = 0x0080;
		MT5725_write_buffer(mt5725_chip_p, 0x0004, temp.ptr, 2);
		temp.ptr[0] = 0x08;
		MT5725_write_buffer(mt5725_chip_p, 0x0000, temp.ptr, 1);

		msleep(50);
		MT5725_read_buffer(mt5725_chip_p, 0x0000, temp.ptr, 2);

		j = 0;
		while (temp.ptr[0] == 8) {
			msleep(50);
			MT5725_read_buffer(mt5725_chip_p, 0x0000, temp.ptr, 2);
			printk("5725status:%d", temp.c0);
			j++;
			if (j > 500) {
				printk("error! Read OTP TImeout\n");
				return 0;
			}
		}
		if (temp.ptr[0] != 2) {
			printk("OtpRead l670 error\n");
			return 0;
		}

		printk("OtpRead:");
		MT5725_read_buffer(mt5725_chip_p, 0x0008, buf + i * 128, 128);
		for (j = 0; j < 128; j++)
			printk(",%x", buf[i * 128 + j]);
		printk("\n");
	}

	return 1;
}

/*
 * MT5725_write_otpok_flag -- 0xffffff8008acb4dc, 96 byte.
 *   "53087c0a lsr"@0xffffff8008acb4f4   (addr >> 8, scorrimento LOGICO)
 *   "7100dd5f cmp"@0xffffff8008acb4fc   (>= 0x37)
 *   "5294ab49 mov"@0xffffff8008acb4f0   (0xa55a)
 *   "72b4ab49 movk"@0xffffff8008acb4f8  (0xa55a nella meta' alta)
 *   "321e03e2 orr"@0xffffff8008acb510   (4 byte)
 * Il canarino c'e' perche' l'indirizzo del locale ESCE in MT5725_otp_write.
 */
void MT5725_write_otpok_flag(u32 addr)
{
	u32 flag = 0xa55aa55a;

	if ((addr >> 8) >= 0x37)
		MT5725_otp_write(addr, (u8 *)&flag, 4);
}

/*
 * MT5725_otp_write -- 0xffffff8008acb53c, 1156 byte. Programma l'OTP: e'
 * IRREVERSIBILE, e ogni riga qui sotto ha la sua citazione.
 *
 * Ritorna VOID: nessun `mov w0` precede il `ret`
 *   "a9597bfd ldp"@0xffffff8008acb99c   (epilogo: w0 non e' toccato)
 * Se in fabbrica fosse `int` con tutti e tre i chiamanti che ne buttano via il
 * risultato, DeadArgumentElimination darebbe lo stesso codice: il binario non
 * distingue i due casi, e `void` e' una SCELTA fra due indistinguibili.
 *
 *   "MT5725_run_pgm_fw,size:%d\n"@0xffffff8009263219
 *   "97fffee3 bl"@0xffffff8008acb584    (MT5725_run_pgm_fw)
 *   "34002013 cbz"@0xffffff8008acb588   (len == 0: non fa niente)
 *   "321d03ea orr"@0xffffff8008acb594   (8 tentativi, in uno slot di pila
 *                                        condiviso da TUTTI i blocchi:
 *                                        "b90017ea str"@0xffffff8008acb5a8)
 *   "7102067f cmp"@0xffffff8008acb5d0   (blocchi da 128: il confronto e' con
 *                                        0x81, cioe' `< 0x81` non `<= 0x80`)
 *
 * Il pacchetto sta a sp+40 e i suoi primi otto byte sono SCRITTI E MAI LETTI
 * (la scrittura al chip passa da un secondo buffer a sp+32). Sono scritture
 * morte che il compilatore non puo' togliere perche' l'indirizzo del pacchetto
 * esce nella MT5725_write_buffer dei dati: e' un DIFETTO DI FABBRICA e si
 * riproduce.
 *   "790053ff strh"@0xffffff8008acb604  (+0 = 0)
 *   "790057f6 strh"@0xffffff8008acb5fc  (+2 = indirizzo del blocco)
 *   "79005bee strh"@0xffffff8008acb608  (+4 = lunghezza)
 *   "79005ff6 strh"@0xffffff8008acb600  (+6 = indirizzo, seme della somma)
 *   "79005fe8 strh"@0xffffff8008acb664  (+6 = somma dopo il ciclo)
 *   "79005bfb strh"@0xffffff8008acb6ac  (+4 = lunghezza accorciata)
 *   "79005ff9 strh"@0xffffff8008acb6b0  (+6 = somma + lunghezza accorciata)
 *
 * Il ciclo che copia e somma:
 *   "386b4a8b ldrb"@0xffffff8008acb640  (buf[offset+i], indice a 32 bit SENZA
 *                                        segno: `uxtw`, non `sxtw`)
 *   "382a6b0b strb"@0xffffff8008acb644
 *   "0b0b0108 add"@0xffffff8008acb650   (somma += byte)
 *
 * L'accorciamento della coda a zero NON guarda il byte 0: il contatore parte
 * da lunghezza-1 e si ferma a 1, con confronti CON SEGNO.
 *   "5100052b sub"@0xffffff8008acb658   (lunghezza - 1)
 *   "5400014b b.lt"@0xffffff8008acb668  (se lunghezza-1 < 1 non entra)
 *   "3940018d ldrb"@0xffffff8008acb674
 *   "350000cd cbnz"@0xffffff8008acb678  (primo byte diverso da zero: esce)
 *   "54ffff4c b.gt"@0xffffff8008acb68c  (continua finche' l'indice > 0)
 *   "6b09015f cmp"@0xffffff8008acb690   (zeri == lunghezza: salta il blocco)
 *
 * I cinque registri scritti, nell'ordine, e la lettura dello stato:
 *   "790043ff strh"@0xffffff8008acb6b4  (0x0000 <- 0)
 *   "790043f6 strh"@0xffffff8008acb6fc  (0x0002 <- indirizzo)
 *   "790043fb strh"@0xffffff8008acb748  (0x0004 <- lunghezza accorciata)
 *   "790043f9 strh"@0xffffff8008acb794  (0x0006 <- somma)
 *   "4b3b2113 sub"@0xffffff8008acb7dc   (0x0008 <- i dati, lunghezza a 16 bit)
 *   "390083e8 strb"@0xffffff8008acb834  (0x0000 <- 0x01, UN solo byte:
 *                                        "7100043f cmp"@0xffffff8008acb838)
 *   "52800c80 mov"@0xffffff8008acb870   (msleep(100))
 *   "7107cf3f cmp"@0xffffff8008acb8dc   (500 giri di attesa)
 *   "otp write status:%d\n"@0xffffff8009263234
 *   "PGM_STATUS_PROGOK\n"@0xffffff8009263249
 *   "7100091f cmp"@0xffffff8008acb8e8   (stato 2 = programmato)
 *   "7100111f cmp"@0xffffff8008acb8f0   (stato 4 = errore di somma)
 *   "7100839f cmp"@0xffffff8008acb914   (stato 0x20 = errore di programmazione)
 *   "Find ERRCS\n"@0xffffff800926325c
 *   "PGM_STATUS_ERRCS\n"@0xffffff8009263268
 *   "Find ERRPGM\n"@0xffffff800926327a
 *   "PGM_STATUS_ERRPGM\n"@0xffffff8009263287
 *   "Find NUKNOWN\n"@0xffffff800926329a
 *   "PGM_STATUS_NUKNOWN\n"@0xffffff80092632a8
 *
 * Le DIMENSIONI dei due buffer non sono nominate da nessuna istruzione: sono
 * SCELTE dagli scostamenti di pila. Il pacchetto sta a sp+40 e il canarino a
 * sp+304 ("f81a03a8 stur"@0xffffff8008acb57c, cioe' x29-96); 304-40 = 264 =
 * 8 + 256. Il secondo buffer sta a sp+32 ("910083e2 add"@0xffffff8008acb87c).
 */
static void MT5725_otp_write(u32 addr, u8 *buf, u32 len)
{
	struct {
		u16	c0;
		u16	c2;
		u16	c4;
		u16	c6;
		u8	c8[256];
	} pkt;
	union mt5725_u16 temp;
	u32 offset = 0;
	u32 remaining = len;
	u32 retries = 8;
	u32 i;
	u32 zeros;
	int k;
	u16 count, sum, cur, eff;

	printk("MT5725_run_pgm_fw,size:%d\n", len);
	MT5725_run_pgm_fw();

	while (remaining) {
		cur = addr + offset;
		if (remaining < 0x81)
			count = remaining;
		else
			count = 0x80;

		pkt.c0 = 0;
		pkt.c2 = cur;
		pkt.c4 = count;
		pkt.c6 = cur;

		sum = cur;
		for (i = 0; i < count; i++) {
			pkt.c8[i] = buf[offset + i];
			sum += pkt.c8[i];
		}
		pkt.c6 = sum;

		zeros = 0;
		for (k = count - 1; k > 0; k--) {
			if (pkt.c8[k])
				break;
			zeros++;
		}

		if (zeros != count) {
			eff = count - zeros;
			pkt.c4 = eff;
			pkt.c6 = sum + eff;

			temp.c0 = 0x0000;
			MT5725_write_buffer(mt5725_chip_p, 0x0000, temp.ptr, 2);
			temp.c0 = cur;
			MT5725_write_buffer(mt5725_chip_p, 0x0002, temp.ptr, 2);
			temp.c0 = eff;
			MT5725_write_buffer(mt5725_chip_p, 0x0004, temp.ptr, 2);
			temp.c0 = sum + eff;
			MT5725_write_buffer(mt5725_chip_p, 0x0006, temp.ptr, 2);
			MT5725_write_buffer(mt5725_chip_p, 0x0008, pkt.c8, eff);
			temp.ptr[0] = 0x01;
			MT5725_write_buffer(mt5725_chip_p, 0x0000, temp.ptr, 1);

			msleep(100);
			MT5725_read_buffer(mt5725_chip_p, 0x0000, temp.ptr, 1);
			i = 0;
			while (temp.ptr[0] == 0x01) {
				msleep(100);
				MT5725_read_buffer(mt5725_chip_p, 0x0000,
						   temp.ptr, 1);
				printk("otp write status:%d\n", temp.ptr[0]);
				i++;
				if (i > 500)
					return;
			}

			if (temp.ptr[0] == 0x02) {
				printk("PGM_STATUS_PROGOK\n");
			} else if (temp.ptr[0] == 0x04) {
				if (retries == 0) {
					printk("PGM_STATUS_ERRCS\n");
					return;
				}
				printk("Find ERRCS\n");
				retries--;
				continue;
			} else if (temp.ptr[0] == 0x20) {
				if (retries == 0) {
					printk("PGM_STATUS_ERRPGM\n");
					return;
				}
				printk("Find ERRPGM\n");
				retries--;
				continue;
			} else {
				if (retries == 0) {
					printk("PGM_STATUS_NUKNOWN\n");
					return;
				}
				printk("Find NUKNOWN\n");
				retries--;
				continue;
			}
		}

		remaining -= count;
		offset += count;
	}
}

/*
 * MT5725_otp_process -- 0xffffff8008acb9c0, 544 byte.
 *
 * `MT5725_otp_write_check` e' INCORPORATA all'inizio (la stessa sequenza --
 * set_opa_mode(1), msleep(50), MT5725_otp_read(0x3d00, buf, 4, 1), confronto
 * con 5a a5 5a a5 -- compare anche in mt5725_get_otp, e le due condividono i
 * letterali "MT5725 OTP Write", "MT5725 OTP NOT Write" e
 * "MT5725_otp_write_check Done\n"). E' l'IPOTESI piu' debole di questo lotto:
 * la prova sono letterali di messaggio, non un `__func__`.
 *
 *   "5287a000 mov"@0xffffff8008acb9fc   (indirizzo OTP 0x3d00)
 *   "9112d273 add"@0xffffff8008acba94   (0xffffff8008f824b4 = MT5725_otp_fw)
 *   "52875602 mov"@0xffffff8008acba9c   (0x3ab0 = 15024 byte)
 *   "52801801 mov"@0xffffff8008acbab4 / "72a02801 movk"@0xffffff8008acbabc
 *                                       (0x014000c0 = GFP_KERNEL)
 *   "321f03e2 orr"@0xffffff8008acbac0   (ordine 2: kmalloc(15024) e' una
 *                                        kmalloc_large)
 *   "d344fd09 lsr"@0xffffff8008acbb28 / "f10ea93f cmp"@0xffffff8008acbb2c
 *                                       (i>>4 <= 0x3aa, cioe' i < 0x3ab0)
 *   "5294ab48 mov"@0xffffff8008acbb48 / "72b4ab48 movk"@0xffffff8008acbb4c
 *                                       (0xa55aa55a)
 *   "mt_charger_set_opa_mode done\n"@0xffffff8009262e4f
 *   "devm_kzalloc Error\n"@0xffffff80092632e2
 *   "MT5725_otp_verify FALSE"@0xffffff80092632f6
 *   "MT5725_otp_verify TRUE"@0xffffff800926330e
 *
 * DIFETTO DI FABBRICA RIPRODOTTO: sul ramo in cui il confronto fallisce il
 * buffer NON viene liberato ("90003cc0 adrp"@0xffffff8008acbb88 salta al
 * printk senza passare da kfree). E' una perdita di memoria vera.
 */
int MT5725_otp_process(void)
{
	u8 buf[128];
	u8 *rd;
	size_t i;

	/* ---- MT5725_otp_write_check, incorporata ---- */
	mt_charger_set_opa_mode(mt5725_chg_dev, 1);
	msleep(50);
	MT5725_otp_read(0x3d00, buf, 4, 1);
	if (buf[0] == 0x5a && buf[1] == 0xa5 &&
	    buf[2] == 0x5a && buf[3] == 0xa5) {
		printk("MT5725 OTP Write");
		printk("MT5725_otp_write_check Done\n");
		mt_charger_set_opa_mode(mt5725_chg_dev, 0);
		printk("mt_charger_set_opa_mode done\n");
		mt5725_chip_q->c820 = 5;
		return 1;
	}
	printk("MT5725 OTP NOT Write");
	/* ---- fine MT5725_otp_write_check ---- */

	mt5725_chip_q->c820 = 3;
	MT5725_otp_write(0, (u8 *)MT5725_otp_fw, sizeof(MT5725_otp_fw));
	mt5725_chip_q->c820 = 4;

	/* NON sizeof(MT5725_otp_fw): il binario chiede 0x3b00 = 15104 byte,
	 * 80 in piu' del firmware -- "52876000 mov"@0xffffff8008acbab8 */
	rd = kmalloc(0x3b00, GFP_KERNEL);
	if (!rd) {
		printk("devm_kzalloc Error\n");
		mt_charger_set_opa_mode(mt5725_chg_dev, 0);
		mt5725_chip_q->c820 = 6;
		return 0;
	}

	mt_charger_set_opa_mode(mt5725_chg_dev, 0);
	msleep(100);
	mt_charger_set_opa_mode(mt5725_chg_dev, 1);
	msleep(50);
	MT5725_otp_read(0, rd, sizeof(MT5725_otp_fw), 1);

	for (i = 0; i < sizeof(MT5725_otp_fw); i++) {
		if (MT5725_otp_fw[i] != rd[i]) {
			printk("MT5725_otp_verify FALSE");
			mt_charger_set_opa_mode(mt5725_chg_dev, 0);
			mt5725_chip_q->c820 = 6;
			return 0;
		}
	}

	kfree(rd);
	printk("MT5725_otp_verify TRUE");
	*(u32 *)buf = 0xa55aa55a;
	MT5725_otp_write(0x3d00, buf, 4);
	mt_charger_set_opa_mode(mt5725_chg_dev, 0);
	mt5725_chip_q->c820 = 1;

	return 1;
}
/*
 * MT5725_check_chargetimeout -- 0xffffff8008accb88, 120 byte.
 *   "97f87595 bl"@0xffffff8008accb98    (battery_get_vbus)
 *   "713e7e7f cmp"@0xffffff8008accbc0   (vbus contro 3999)
 *   "54000168 b.hi"@0xffffff8008accbc4  (confronto SENZA segno: la variabile
 *                                        e' unsigned, non il `signed int` che
 *                                        battery_get_vbus dichiara)
 *   "397fe2a3 ldrb"@0xffffff8008accba8  (lo stato e' letto a UN byte: e'
 *                                        `static` e stretto)
 *   "FastChargeTimeout=%d, vbus=%d,FastChargeState:%d\n"@0xffffff8009262f26
 *   "FastChargeTimeout=0, vbus=%d\n"@0xffffff8009262f58
 */
void MT5725_check_chargetimeout(void)
{
	unsigned int vbus = battery_get_vbus();

	printk("FastChargeTimeout=%d, vbus=%d,FastChargeState:%d\n",
	       mt5725_fastcharge_timeout, vbus, mt5725_fastcharge_state);

	if (vbus < 4000) {
		if (mt5725_fastcharge_timeout) {
			mt5725_fastcharge_timeout--;
		} else {
			mt5725_fastcharge_state = 0;
			printk("FastChargeTimeout=0, vbus=%d\n", vbus);
		}
	}
}

/*
 * ech_wls_set_chrg_current -- 0xffffff8008acd180, 184 byte.
 * Esiste come simbolo `T` E viene INCORPORATA in MT5725_irq_handle,
 * MT5725_good_int_delayed_work_func e mt5725_charger_work_func: in tutte e tre
 * compare il suo `__func__`.
 *   "wireless"@0xffffff80092ba049
 *   "\x016%s: get power supply failed\n"@0xffffff8009261205
 *   "ech_wls_set_chrg_current"@0xffffff800926316e
 *   "321c03e1 orr"@0xffffff8008acd1cc   (proprieta' 16 = CURRENT_MAX)
 *   "52800221 mov"@0xffffff8008acd1e0   (proprieta' 17 = CURRENT_NOW)
 *   "b94152c8 ldr"@0xffffff8008acd1bc   (0xffffff800a111150, ultimo valore)
 */
void ech_wls_set_chrg_current(int cur_max, int cur_now)
{
	union power_supply_propval val;
	struct power_supply *psy;

	psy = power_supply_get_by_name("wireless");
	if (!psy) {
		pr_info("%s: get power supply failed\n", __func__);
		return;
	}

	if (mt5725_last_chrg_current != cur_max) {
		val.intval = cur_max;
		power_supply_set_property(psy, POWER_SUPPLY_PROP_CURRENT_MAX, &val);
		val.intval = cur_now;
		power_supply_set_property(psy, POWER_SUPPLY_PROP_CURRENT_NOW, &val);
		mt5725_last_chrg_current = cur_max;
	}
}

/*
 * wls_get_online -- 0xffffff8008acd238, 56 byte.
 *   "\x016wls_work_online1 = %d.\n"@0xffffff8009263187
 *   "b94fe661 ldr"@0xffffff8008acd248   (0xffffff800a110fe4, larghezza piena)
 *   "1a9f07e0 cset"@0xffffff8008acd264  (ritorna 0/1)
 */
int wls_get_online(void)
{
	pr_info("wls_work_online1 = %d.\n", mt5725_wls_online);
	return mt5725_wls_online != 0;
}

/*
 * rvs_get_online -- 0xffffff8008acd270, 52 byte.
 * DIFETTO DI FABBRICA RIPRODOTTO: stampa il globale `wls` e ritorna il
 * globale `rvs`, che sono due variabili diverse.
 *   "b94fe501 ldr"@0xffffff8008acd27c   (stampa 0xffffff800a110fe4)
 *   "b94fed08 ldr"@0xffffff8008acd290   (ritorna 0xffffff800a110fec)
 */
int rvs_get_online(void)
{
	pr_info("wls_work_online1 = %d.\n", mt5725_wls_online);
	return mt5725_rvs_online != 0;
}

/*
 * reverse_charger_rxdetect -- 0xffffff8008acd2a8, 152 byte.
 *   "97fdea59 bl"@0xffffff8008acd2b4    (input_allocate_device)
 *   "940e2708 bl"@0xffffff8008acd2cc    (set_bit -- funzione VERA su arm64,
 *                                        non la versione inline `__set_bit`)
 *   "52800ae0 mov"@0xffffff8008acd2d4   (codice tasto 87)
 *   "52800328 mov"@0xffffff8008acd2e8   (id.bustype = 0x19 = BUS_HOST)
 *   "12800173 mov"@0xffffff8008acd31c   (-12 = -ENOMEM)
 *   "RXDETECT"@0xffffff80092631be
 *   "\x016alloc input device failed\n"@0xffffff80092631a1
 *   "\x016input device register failed\n"@0xffffff80092631c7
 *   "\x016input register success\n"@0xffffff80092631e7
 */
int reverse_charger_rxdetect(void)
{
	int ret;

	mt5725_input = input_allocate_device();
	if (!mt5725_input) {
		pr_info("alloc input device failed\n");
		return -ENOMEM;
	}

	set_bit(EV_KEY, mt5725_input->evbit);
	/* 87: il binario non lo nomina, lo si legge come costante */
	set_bit(87, mt5725_input->keybit);
	mt5725_input->id.bustype = BUS_HOST;
	mt5725_input->name = "RXDETECT";

	ret = input_register_device(mt5725_input);
	if (ret)
		pr_info("input device register failed\n");
	else
		pr_info("input register success\n");

	return ret;
}


/*
 * MT5725_irq_handle -- 0xffffff8008accc74, 1292 byte.
 * ech_wls_set_chrg_current e' INCORPORATA nel ramo del bit 15
 * ("ech_wls_set_chrg_current"@0xffffff800926316e); MT5725_read_buffer e
 * MT5725_write_buffer sono incorporate ovunque.
 *
 * I bit del registro 0x0c, letti dalle prove di bit:
 *   "37f80108 tbnz"@0xffffff8008accd0c  bit 15 (il carico e' `ldrsh`, cioe'
 *                                       una prova sul segno del `short`)
 *   "37300548 tbnz"@0xffffff8008accd10  bit 6
 *   "37180668 tbnz"@0xffffff8008accd14  bit 3
 *   "37380da8 tbnz"@0xffffff8008accd18  bit 7
 *   "37400e68 tbnz"@0xffffff8008accd1c  bit 8
 *   "37480fe8 tbnz"@0xffffff8008accd20  bit 9
 *   "362000e8 tbz"@0xffffff8008accff8   bit 4
 *   "321b03f4 orr"@0xffffff8008accd24   (reg_cmd = 0x20)
 *   "52808414 mov"@0xffffff8008accf88   (reg_cmd = 0x420)
 *   "528ae4a8 mov"@0xffffff8008acce6c   (0x5725)
 *   "713e841f cmp"@0xffffff8008acd150   (4001) / "54fffcab b.lt"@0xffffff8008acd154
 *   "528001a3 mov"@0xffffff8008acd170   (ritardo 13)
 *   "----------------MT5725_delayed_work low interrupt-----------------------\n"@0xffffff8009262f76
 *   "%s ,version 0.1 little-endian val:0x%04x\n"@0xffffff8009262fc0
 *   "MT5725_irq_handle"@0xffffff8009262fea
 *   "MT5725 %s , INTFALG_PowerON\n"@0xffffff8009262ffc
 *   "MT5725 %s , LDO ON\n"@0xffffff8009263019
 *   "MT5725 %s , MT5725 is Ready\n"@0xffffff800926302d
 *   "MT5725 %s , 0x%x%x\n"@0xffffff800926304a
 *   "MT5725 ID Correct query\n"@0xffffff800926305e
 *   "MT5725 ID error :%d\n "@0xffffff8009263077
 *   "MT5725 chip_version old : 0x%02x,0x%02x\n"@0xffffff800926308d
 *   "MT5725 %s , MT5725 LDO  off\n"@0xffffff80092630b6
 *   "MT5725 %s ,version 0.1 Tx support samsung_afc\n"@0xffffff8009262d3f
 *   "MT5725 %s ,version 0.1 Tx support INT_EPP\n"@0xffffff80092630d3
 *   "MT5725 %s ,rxdetect: INT_POWER_TRANS\n"@0xffffff80092630fe
 *   "MT5725 %s ,rxdetect: INT_REMOVE_POWER\n"@0xffffff8009263124
 *   "MT5725 %s , FSK successfully  off\n"@0xffffff800926314b
 *   "%s,version 0.1 write reg_clr : 0x%04x,\n"@0xffffff8009262d7d
 *   "%s,version 0.1 write reg_cmd : 0x%04x,\n"@0xffffff8009262da5
 */
void MT5725_irq_handle(void)
{
	union mt5725_u16 temp;
	union mt5725_u16 chip_id;
	u8 version[2];
	u16 int_flag;
	u16 reg_cmd;
	u16 epp;
	int ret, gpio;

	printk("----------------MT5725_delayed_work low interrupt-----------------------\n");

	ret = MT5725_read_buffer(mt5725_chip_p, 0x000c, temp.ptr, 2);
	gpio = gpio_get_value(mt5725_chip_p->c72);
	if (ret < 0 || gpio)
		temp.c0 = 0;
	int_flag = temp.c0;
	printk("%s ,version 0.1 little-endian val:0x%04x\n", __func__, int_flag);

	if (temp.c0 & 0x8000) {
		mt5725_g11101c = 0;
		ech_wls_set_chrg_current(200, 200);
		printk("MT5725 %s , INTFALG_PowerON\n", __func__);
	}
	if (temp.c0 & 0x0040) {
		mt5725_good_irq_ready = 1;
		printk("MT5725 %s , LDO ON\n", __func__);
	}
	if (temp.c0 & 0x0008) {
		mt5725_g111020 = 1;
		printk("MT5725 %s , MT5725 is Ready\n", __func__);
		mt5725_g111024[0] = 100;
		schedule_delayed_work(&mt5725_chip_q->c216, 0);

		MT5725_read_buffer(mt5725_chip_p, 0x0000, chip_id.ptr, 2);
		printk("MT5725 %s , 0x%x%x\n", __func__, chip_id.ptr[1],
		       chip_id.ptr[0]);
		if (((chip_id.ptr[1] << 8) | chip_id.ptr[0]) == 0x5725)
			printk("MT5725 ID Correct query\n");
		else
			printk("MT5725 ID error :%d\n ", chip_id.c0);

		MT5725_read_buffer(mt5725_chip_p, 0x0004, version, 2);
		printk("MT5725 chip_version old : 0x%02x,0x%02x\n",
		       version[1], version[0]);
	}
	if (temp.c0 & 0x0080)
		printk("MT5725 %s , MT5725 LDO  off\n", __func__);
	if (temp.c0 & 0x0100) {
		mt5725_g111014 = 1;
		printk("MT5725 %s ,version 0.1 Tx support samsung_afc\n",
		       __func__);
		mt5725_g11101c = 6;
	}
	if (temp.c0 & 0x0200) {
		mt5725_g111018 = 1;
		epp = 0x2328;
		c_acab34(mt5725_chip_p, 0x001e, epp);
		printk("MT5725 %s ,version 0.1 Tx support INT_EPP\n", __func__);
		reg_cmd = 0x0420;
	} else {
		reg_cmd = 0x0020;
	}

	if ((temp.c0 & 0x0100) && mt5725_rvs_online == 1) {
		mt5725_chip_q->c80 = 1;
		printk("MT5725 %s ,rxdetect: INT_POWER_TRANS\n", __func__);
	}
	if ((temp.c0 & 0x0040) && mt5725_rvs_online == 1) {
		mt5725_chip_q->c80 = 0;
		printk("MT5725 %s ,rxdetect: INT_REMOVE_POWER\n", __func__);
	}
	if (temp.c0 & 0x0010) {
		MT5725_fsk_handle();
		printk("MT5725 %s , FSK successfully  off\n", __func__);
	}

	c_acab34(mt5725_chip_p, 0x000e, int_flag);
	printk("%s,version 0.1 write reg_clr : 0x%04x,\n", __func__, int_flag);
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
	printk("%s,version 0.1 write reg_cmd : 0x%04x,\n", __func__, reg_cmd);

	if (!gpio_get_value(mt5725_chip_p->c72)) {
		MT5725_read_buffer(mt5725_chip_p, 0x000c, temp.ptr, 2);
		if (temp.c0 != 0 && temp.c0 != 0xffff &&
		    battery_get_vbus() > 4000)
			schedule_delayed_work(&mt5725_chip_q->c120, 13);
	}

	enable_irq(mt5725_irq_num);
}

/*
 * MT5725_int_delayed_work_func -- 0xffffff8008acd92c, 20 byte.
 *   "97fffcd0 bl"@0xffffff8008acd934    (MT5725_irq_handle)
 * Il parametro `work` non e' usato.
 */
static void MT5725_int_delayed_work_func(struct work_struct *work)
{
	MT5725_irq_handle();
}

/*
 * MT5725_irq -- 0xffffff8008acdd18, 68 byte.
 *   "b9407100 ldr"@0xffffff8008acdd28   (0xffffff800a111070, numero di IRQ)
 *   "97d9b383 bl"@0xffffff8008acdd30    (disable_irq_nosync)
 *   "321d03e0 orr"@0xffffff8008acdd40   (cpu = 8 = WORK_CPU_UNBOUND con
 *                                        CONFIG_NR_CPUS=8: e' cio' in cui si
 *                                        espande schedule_delayed_work)
 *   "9101e262 add"@0xffffff8008acdd3c   (&chip->c120, offset 0x78)
 *   "320003e0 orr"@0xffffff8008acdd50   (IRQ_HANDLED)
 */
static irqreturn_t MT5725_irq(int irq, void *data)
{
	struct mt5725_chip *chip = data;

	disable_irq_nosync(mt5725_irq_num);
	schedule_delayed_work(&chip->c120, 0);

	return IRQ_HANDLED;
}

/*
 * MT5725_good_irq -- 0xffffff8008acdd5c, 96 byte.
 *   "b94fe908 ldr"@0xffffff8008acdd60   (0xffffff800a110fe8 == 1?)
 *   "b94fed08 ldr"@0xffffff8008acdd70   (0xffffff800a110fec == 0?)
 *   "b9407500 ldr"@0xffffff8008acdd88   (0xffffff800a111074, secondo IRQ)
 *   "9104e262 add"@0xffffff8008acdd9c   (&chip->c312, offset 0x138)
 */
static irqreturn_t MT5725_good_irq(int irq, void *data)
{
	struct mt5725_chip *chip = data;

	if (mt5725_good_irq_ready == 1 && !mt5725_rvs_online) {
		disable_irq_nosync(mt5725_good_irq_num);
		schedule_delayed_work(&chip->c312, 0);
	}

	return IRQ_HANDLED;
}

/*
 * fast_charging_store -- 0xffffff8008acdf6c, 140 byte.
 *   "97e5d92f bl"@0xffffff8008acdf98    (kstrtouint, base 10)
 *   "79400be0 ldrh"@0xffffff8008acdfa8  (del valore servono 16 bit: e' il
 *                                        restringimento del carico che clang
 *                                        applica quando l'uso e' un u16)
 *   "5289c428 mov"@0xffffff8008acdfac   (20001)
 *   "MT5725 Parameter error\n"@0xffffff8009263643
 */
static ssize_t fast_charging_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	unsigned int val;
	int ret;

	ret = kstrtouint(buf, 10, &val);
	if (ret)
		return ret;

	if ((u16)val >= 20001)
		printk("MT5725 Parameter error\n");
	else
		fast_vfc(val);

	return count;
}

/*
 * get_reg -- 0xffffff8008acdff8, 212 byte.
 * MT5725_read_buffer e' INCORPORATA (la chiamata a regmap_bulk_read e'
 * diretta, non un `blr` sul campo c48).
 *   "913d9339 add"@0xffffff8008ace034   (tabella a 0xffffff8008f85f64)
 *   "78776b36 ldrh"@0xffffff8008ace050  (elemento a passo 4 byte)
 *   "f10032ff cmp"@0xffffff8008ace044   (l'indice 3 e' SALTATO)
 *   "f10072ff cmp"@0xffffff8008ace08c   (7 giri)
 *   "321403fa orr"@0xffffff8008ace038   (PAGE_SIZE)
 *   "8b20c273 add"@0xffffff8008ace084   (len += (int)snprintf, esteso in segno)
 *   "reg:0x%02x=0x%02x%02x\n"@0xffffff800926365b
 *
 * SCELTA DICHIARATA: il binario salta l'indice 3 con un confronto
 * sull'INDUZIONE del ciclo. La stessa cosa la produrrebbe un test sul secondo
 * campo della tabella (l'unico che vale 2). La dimensione non discrimina fra
 * le due forme: qui e' scritta la forma sull'indice.
 */
static const u16 mt5725_reg_tab[7][2] = {
	{ 0x0000, 0x0001 }, { 0x001a, 0x0001 }, { 0x000c, 0x0001 },
	{ 0x000e, 0x0002 }, { 0x001e, 0x0003 }, { 0x0022, 0x0003 },
	{ 0x0006, 0x0007 },
};

static ssize_t get_reg(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	u8 val[2];
	ssize_t len = 0;
	int i;

	for (i = 0; i < 7; i++) {
		if (i == 3)
			continue;
		MT5725_read_buffer(mt5725_chip_p, mt5725_reg_tab[i][0], val, 2);
		len += snprintf(buf + len, PAGE_SIZE - len,
				"reg:0x%02x=0x%02x%02x\n",
				mt5725_reg_tab[i][0], val[0], val[1]);
	}

	return len;
}

/*
 * mt5725_get_reverse_charger -- 0xffffff8008ace27c, 52 byte.
 *   "b9433903 ldr"@0xffffff8008ace298   (chip->c824, offset 824)
 *   "321403e1 orr"@0xffffff8008ace29c   (PAGE_SIZE)
 *   "reverse_charger en : %d\n"@0xffffff80092636ed
 */
static ssize_t mt5725_get_reverse_charger(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	return snprintf(buf, PAGE_SIZE, "reverse_charger en : %d\n",
			mt5725_chip_q->c824);
}

/*
 * mt5725_get_otg -- 0xffffff8008ace47c, 52 byte.
 *   "b9433108 ldr"@0xffffff8008ace498   (chip->c816, offset 816)
 *   "%u\n"@0xffffff80090f4c93
 */
static ssize_t mt5725_get_otg(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", mt5725_chip_q->c816);
}

/*
 * Mt5725_get_rxdetect -- 0xffffff8008ace894. La mappa gli attribuisce 92 byte
 * ma il suo `ret` sta a 0xffffff8008ace8c4: sono 52. I 40 byte che seguono
 * sono la copia debole e scartata di is_usb_rdy (vedi il cappello del file).
 *   "b9405108 ldr"@0xffffff8008ace8b0   (chip->c80, offset 80)
 *   "d65f03c0 ret"@0xffffff8008ace8c4
 */
static ssize_t Mt5725_get_rxdetect(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", mt5725_chip_q->c80);
}


/*
 * mt5725_set_otg -- 0xffffff8008ace4b0, 152 byte.
 *   "940e74c1 bl"@0xffffff8008ace4e0    (sscanf, formato "%d")
 *   "%d"@0xffffff8009216030
 *   "97ec1175 bl"@0xffffff8008ace4fc    (mt_charger_set_opa_mode)
 *   "b9033114 str"@0xffffff8008ace520   (chip->c816)
 * L'area dei locali e' di 72 byte per un solo intero letto a +8: il locale
 * e' un ARRAY di 64 byte che stack-protector-strong incolla al canarino.
 * La sua LUNGHEZZA e' misurata (72 - 8), il suo tipo e' una scelta.
 */
static ssize_t mt5725_set_otg(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	unsigned int databuf[16];
	int en;

	sscanf(buf, "%d", &databuf[0]);
	if (databuf[0]) {
		mt_charger_set_opa_mode(mt5725_chg_dev, 1);
		en = 1;
	} else {
		mt_charger_set_opa_mode(mt5725_chg_dev, 0);
		en = 0;
	}
	mt5725_chip_q->c816 = en;

	return count;
}

/*
 * mt5725_set_otp -- 0xffffff8008ace670, 128 byte.
 *   "7100091f cmp"@0xffffff8008ace6a8   (valore == 2)
 *   "321f03e9 orr"@0xffffff8008ace6b8   (chip->c820 = 2)
 *   "97fff4c0 bl"@0xffffff8008ace6c0    (MT5725_otp_process)
 */
static ssize_t mt5725_set_otp(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	unsigned int databuf[16];

	sscanf(buf, "%d", &databuf[0]);
	if (databuf[0] == 2) {
		mt5725_chip_q->c820 = 2;
		MT5725_otp_process();
	}

	return count;
}

/*
 * mt5725_set_hwen -- 0xffffff8008ace6f8, 116 byte.
 *   "1a9f07e8 cset"@0xffffff8008ace738  (il globale prende 0/1, non il valore)
 *   "b9002d28 str"@0xffffff8008ace73c   (0xffffff800a11102c, larghezza piena:
 *                                        globale di SOLA SCRITTURA, nessuno in
 *                                        tutta l'immagine lo legge)
 */
static ssize_t mt5725_set_hwen(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	unsigned int databuf[16];

	sscanf(buf, "%d", &databuf[0]);
	mt5725_hwen = databuf[0] != 0;

	return count;
}

/*
 * mt5725_get_otp -- 0xffffff8008ace548, 296 byte.
 *   "5287a000 mov"@0xffffff8008ace584   (indirizzo OTP 0x3d00)
 *   "321e03e2 orr"@0xffffff8008ace588   (4 byte)
 *   "320003e3 orr"@0xffffff8008ace58c   (quarto argomento 1)
 *   "7101691f cmp"@0xffffff8008ace598   (0x5a) / "7102951f cmp"@0xffffff8008ace5a4 (0xa5)
 *   "MT5725 OTP Write"@0xffffff80092632d1
 *   "MT5725_otp_write_check Done\n"@0xffffff8009262e32
 *   "MT5725 OTP NOT Write"@0xffffff80092632bc
 *   "7100191f cmp"@0xffffff8008ace608   (stato == 6: in questo caso NON
 *                                        richiama mt_charger_set_opa_mode e
 *                                        NON riscrive lo stato)
 * DIFETTO DI FABBRICA RIPRODOTTO: `sprintf(buf, "%u\n", ret)` stampa il
 * valore locale, non il campo.
 */
static ssize_t mt5725_get_otp(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	u8 otpbuf[128];
	int ret;

	mt_charger_set_opa_mode(mt5725_chg_dev, 1);
	msleep(50);
	MT5725_otp_read(0x3d00, otpbuf, 4, 1);

	if (otpbuf[0] == 0x5a && otpbuf[1] == 0xa5 &&
	    otpbuf[2] == 0x5a && otpbuf[3] == 0xa5) {
		printk("MT5725 OTP Write");
		printk("MT5725_otp_write_check Done\n");
		mt_charger_set_opa_mode(mt5725_chg_dev, 0);
		ret = 5;
	} else {
		printk("MT5725 OTP NOT Write");
		if (mt5725_chip_q->c820 == 6)
			return sprintf(buf, "%u\n", 6);
		mt_charger_set_opa_mode(mt5725_chg_dev, 0);
		ret = 0;
	}
	mt5725_chip_q->c820 = ret;

	return sprintf(buf, "%u\n", ret);
}

/*
 * Mt5725_set_vout -- 0xffffff8008ace76c, 296 byte.
 *   "97e5d72f bl"@0xffffff8008ace798    (kstrtouint, base 10)
 *   "79400bf4 ldrh"@0xffffff8008ace79c  (restringimento a 16 bit)
 *   "321f0fe1 orr"@0xffffff8008ace7c0   (registro 0x1e)
 *   "53087e82 lsr"@0xffffff8008ace7dc   (byte alto: qui `lsr` e non `ubfx`
 *                                        perche' i bit alti sono noti a zero
 *                                        dal `ldrh`)
 *   "321603e2 orr"@0xffffff8008ace85c   (reg_cmd = 0x0400 nella seconda stampa)
 *   "aa1f03e0 mov"@0xffffff8008ace884   (ritorna 0, NON `count`)
 *   "Mt5725_set_vout"@0xffffff8009263746
 */
static ssize_t Mt5725_set_vout(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	unsigned int val;
	u16 vout;
	u16 reg_cmd = 0x0400;

	kstrtouint(buf, 10, &val);
	vout = val;
	printk("%s,write reg_cmd : 0x%04x,\n", __func__, vout);
	c_acab34(mt5725_chip_p, 0x001e, vout);
	c_acab34(mt5725_chip_p, 0x0006, reg_cmd);
	printk("%s,write reg_cmd : 0x%04x,\n", __func__, reg_cmd);

	return 0;
}

/*
 * mt5725_set_reverse_charger -- 0xffffff8008ace2b0, 460 byte.
 *   "97fff199 bl"@0xffffff8008ace2f8    (mt5725_reverse_charge(1))
 *   "b9033914 str"@0xffffff8008ace308   (chip->c824 = 1)
 *   "52800280 mov"@0xffffff8008ace304   (msleep(20))
 *   "52804ce8 mov"@0xffffff8008ace340   (0x267 scritto ai registri 0x40/0x41)
 *   "52805f28 mov"@0xffffff8008ace3bc   (0x2f9 scritto ai registri 0x3e/0x3f)
 *   " 0x40 val = 0x%04x\n"@0xffffff8009263706
 *   " 0x40 valw = 0x%04x\n"@0xffffff800926371a
 * DIFETTO DI FABBRICA RIPRODOTTO: la terza stampa usa la stringa " 0x40 valw"
 * anche se il registro riletto e' 0x3e.
 */
static ssize_t mt5725_set_reverse_charger(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	union mt5725_u16 temp;
	unsigned int databuf[16];

	sscanf(buf, "%d", &databuf[0]);
	if (databuf[0]) {
		mt5725_reverse_charge(1);
		mt5725_chip_q->c824 = 1;
		msleep(20);

		MT5725_read_buffer(mt5725_chip_p, 0x0040, temp.ptr, 2);
		printk(" 0x40 val = 0x%04x\n", temp.c0);
		temp.c0 = 0x0267;
		c_ace34c(mt5725_chip_p, 0x0040, temp.ptr, 2);
		temp.c0 = 0;
		MT5725_read_buffer(mt5725_chip_p, 0x0040, temp.ptr, 2);
		printk(" 0x40 valw = 0x%04x\n", temp.c0);

		temp.c0 = 0x02f9;
		c_ace34c(mt5725_chip_p, 0x003e, temp.ptr, 2);
		temp.c0 = 0;
		MT5725_read_buffer(mt5725_chip_p, 0x003e, temp.ptr, 2);
		printk(" 0x40 valw = 0x%04x\n", temp.c0);
	} else {
		mt5725_reverse_charge(0);
		mt5725_chip_q->c824 = 0;
	}

	return count;
}


/*
 * set_reg -- 0xffffff8008ace0cc, 432 byte.
 * Il `case` e' cablato dal compilatore in una rotazione:
 *   "51001909 sub"@0xffffff8008ace13c   (v - 6)
 *   "13890929 ror"@0xffffff8008ace140   (ruota a destra di 2)
 *   "7100095f cmp"@0xffffff8008ace148 / "54000463 b.cc"@0xffffff8008ace14c
 *   "7100093f cmp"@0xffffff8008ace150 / "54000420 b.eq"@0xffffff8008ace154
 *   "35000789 cbnz"@0xffffff8008ace158
 * Risolvendo: ror(v-6,2) == 0 -> v = 0x06; == 2 -> v = 0x0e; in {6,7} ->
 * v = 0x1e e v = 0x22. Sono esattamente quattro dei sette registri della
 * tabella di get_reg.
 *   "%x %x"@0xffffff80091411fa
 *   "MT5725 %s : %d 0x%x 0x%x\n"@0xffffff8009263672
 *   "set_reg"@0xffffff8009297df9
 *   "MT5725 get reg: 0x%04x  set reg: 0x%04x \n"@0xffffff800926368c
 *   "MT5725 Set reg : [0x%04x]  0x%x 0x%x \n"@0xffffff80092636b6
 *   "33181d01 bfi"@0xffffff8008ace18c    (val = buf[0] | buf[1] << 8)
 *
 * DIFETTO DI FABBRICA RIPRODOTTO: nella seconda stampa il primo `%04x` riceve
 * il VALORE (databuf[1]) e non il registro ("2a1703e1 mov"@0xffffff8008ace1ec).
 */
static ssize_t set_reg(struct device *dev, struct device_attribute *attr,
		       const char *buf, size_t count)
{
	u8 rbuf[2];
	unsigned int databuf[2];
	u16 val, regval;
	int ret;

	ret = sscanf(buf, "%x %x", &databuf[0], &databuf[1]);
	printk("MT5725 %s : %d 0x%x 0x%x\n", __func__, ret, databuf[0],
	       databuf[1]);

	if (ret == 2) {
		switch (databuf[0]) {
		case 0x06:
			regval = databuf[1];
			MT5725_read_buffer(mt5725_chip_p, databuf[0], rbuf, 2);
			val = rbuf[0] | (rbuf[1] << 8);
			regval |= val;
			printk("MT5725 get reg: 0x%04x  set reg: 0x%04x \n",
			       val, regval);
			MT5725_write_buffer(mt5725_chip_p, databuf[0],
					    (u8 *)&regval, 2);
			break;
		case 0x0e:
		case 0x1e:
		case 0x22:
			regval = databuf[1];
			printk("MT5725 Set reg : [0x%04x]  0x%x 0x%x \n",
			       databuf[1], regval & 0xff, regval >> 8);
			MT5725_write_buffer(mt5725_chip_p, databuf[0],
					    (u8 *)&regval, 2);
			break;
		}
	}

	return count;
}

/* ------------------------------------------------------------------------ */
/* Il gruppo sysfs, letto con `relocazioni.py --sysfs-group` dalla tabella a
 * 0xffffff80099a75c0 (8 voci + NULL) e dal gruppo a 0xffffff8008f862f0.
 *   "mt5725group"@0xffffff8009263629 (il campo `name` del gruppo a
 *   0xffffff8008f862f0 punta qui)
 */
static DEVICE_ATTR(fast_charging, 0644, NULL, fast_charging_store);
static DEVICE_ATTR(reg, 0644, get_reg, set_reg);
static DEVICE_ATTR(reverse_charger, 0660, mt5725_get_reverse_charger,
		   mt5725_set_reverse_charger);
static DEVICE_ATTR(otg, 0660, mt5725_get_otg, mt5725_set_otg);
static DEVICE_ATTR(otp, 0660, mt5725_get_otp, mt5725_set_otp);
static DEVICE_ATTR(mt5725_en, 0660, mt5725_get_hwen, mt5725_set_hwen);
static DEVICE_ATTR(epp_set_vout, 0660, NULL, Mt5725_set_vout);
static DEVICE_ATTR(rxdetect, 0660, Mt5725_get_rxdetect, NULL);

static struct attribute *mt5725_attributes[] = {
	&dev_attr_fast_charging.attr,
	&dev_attr_reg.attr,
	&dev_attr_reverse_charger.attr,
	&dev_attr_otg.attr,
	&dev_attr_otp.attr,
	&dev_attr_mt5725_en.attr,
	&dev_attr_epp_set_vout.attr,
	&dev_attr_rxdetect.attr,
	NULL,
};

static const struct attribute_group mt5725_attr_group = {
	.name = "mt5725group",
	.attrs = mt5725_attributes,
};
/*
 * MT5725_remove -- 0xffffff8008acd858, 36 byte.
 *   "9100c000 add"@0xffffff8008acd864   (&client->dev.kobj: dev a +0x20,
 *                                        kobj a +0x10 dentro struct device)
 *   "97e0d9a1 bl"@0xffffff8008acd86c    (sysfs_remove_group)
 *   "2a1f03e0 mov"@0xffffff8008acd870   (ritorna 0)
 */
static int MT5725_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &mt5725_attr_group);
	return 0;
}




/*
 * MT5725_fastcharge_change -- 0xffffff8008acc874, 788 byte.
 * SETTE MT5725_write_buffer incorporate, tutte sullo STESSO locale a sp+8:
 * i registri 0x5808 (TRE VOLTE con lo stesso valore 0x95 -- e' quello che il
 * binario fa), 0x5800, 0x5244, 0x5218 (due byte), 0x5208.
 *   "397fe261 ldrb"@0xffffff8008acc88c   (stato a un byte)
 *   "5280fa00 mov"@0xffffff8008acc89c    (msleep(2000))
 *   "97f355e5 bl"@0xffffff8008acc904     (get_boot_mode)
 *   "7100201f cmp"@0xffffff8008acc908    (== 8 = KERNEL_POWER_OFF_CHARGING_BOOT)
 *   "528012a8 mov"@0xffffff8008acc91c    (0x95)
 *   "528b0101 mov"@0xffffff8008acc920    (registro 0x5808)
 *   "528b0001 mov"@0xffffff8008acca0c    (registro 0x5800, valore 0x01)
 *   "528a4881 mov"@0xffffff8008acca60    (registro 0x5244, valore 0x57)
 *   "528841e8 mov"@0xffffff8008accab0    (0x420f) / "528a4301 mov"@0xffffff8008accab4 (registro 0x5218)
 *   "321f0be8 orr"@0xffffff8008accb04    (0x0e) / "528a4101 mov"@0xffffff8008accb08 (registro 0x5208)
 *   "INT_AFC_SUPPORT\n"@0xffffff8009262ebc
 *   "FastChargeState = 1;\n"@0xffffff8009262ecd
 *   "INT_EPP_SUPPORT\n"@0xffffff8009262ee3
 *   "FastChargeState = 0;\n"@0xffffff8009262ef4
 *   "MT5725_fastcharge_change:%d"@0xffffff8009262f0a
 * L'ultimo `printk` sembra a prima vista un `%d` senza argomento: non lo e'.
 * Il valore dello stato e' stato caricato in w1 fin dall'inizio
 * ("397fe261 ldrb"@0xffffff8008acc88c carica proprio in w1, il registro del
 * secondo argomento) e non serve ricaricarlo.
 */
void MT5725_fastcharge_change(void)
{
	union mt5725_u16 val;

	if (mt5725_fastcharge_state == 1) {
		msleep(2000);
		if (mt5725_g111018 == 1) {
			mt5725_g111018 = 0;
			mt5725_fastcharge_state = 0;
			printk("INT_EPP_SUPPORT\n");
		} else {
			mt5725_fastcharge_state = 0;
			printk("FastChargeState = 0;\n");
		}
	} else if (mt5725_fastcharge_state == 0) {
		msleep(2000);
		if (mt5725_g111014 == 1) {
			mt5725_g111014 = 0;
			mt5725_fastcharge_state = 0;
			printk("INT_AFC_SUPPORT\n");
		} else {
			mt5725_fastcharge_state = 1;
			if (get_boot_mode() == KERNEL_POWER_OFF_CHARGING_BOOT)
				return;
			val.ptr[0] = 0x95;
			MT5725_write_buffer(mt5725_chip_p, 0x5808, val.ptr, 1);
			MT5725_write_buffer(mt5725_chip_p, 0x5808, val.ptr, 1);
			MT5725_write_buffer(mt5725_chip_p, 0x5808, val.ptr, 1);
			val.ptr[0] = 0x01;
			MT5725_write_buffer(mt5725_chip_p, 0x5800, val.ptr, 1);
			val.ptr[0] = 0x57;
			MT5725_write_buffer(mt5725_chip_p, 0x5244, val.ptr, 1);
			val.c0 = 0x420f;
			MT5725_write_buffer(mt5725_chip_p, 0x5218, val.ptr, 2);
			val.ptr[0] = 0x0e;
			MT5725_write_buffer(mt5725_chip_p, 0x5208, val.ptr, 1);
			msleep(200);
			printk("FastChargeState = 1;\n");
		}
	} else {
		printk("MT5725_fastcharge_change:%d", mt5725_fastcharge_state);
	}
}

/*
 * MT5725_fastcharge_select -- 0xffffff8008acc748, 300 byte.
 * MT5725_enable_epp e MT5725_enable_afc sono INCORPORATE (i loro corpi
 * compaiono in linea, ognuno col proprio locale: `mov x21, sp` per il ramo
 * AFC e `add x21, sp, #0x8` per quello EPP).
 *   "397fe108 ldrb"@0xffffff8008acc760   (lo stato letto a UN byte)
 *   "7100051f cmp"@0xffffff8008acc764    (== 1)
 *   "35000748 cbnz"@0xffffff8008acc76c   (!= 0: TERZO ramo — e' la prova che
 *                                         la variabile non e' un `bool`)
 *   "52800149 mov"@0xffffff8008acc848    (timeout = 10)
 *   "MT5725_enable_afc\n"@0xffffff8009262e85
 *   "MT5725_enable_epp\n"@0xffffff8009262e98
 *   "MT5725_rpp_mode\n"@0xffffff8009262eab
 */
void MT5725_fastcharge_select(void)
{
	if (mt5725_fastcharge_state == 1) {
		MT5725_enable_epp();
		printk("MT5725_enable_epp\n");
	} else if (mt5725_fastcharge_state == 0) {
		MT5725_enable_afc();
		printk("MT5725_enable_afc\n");
	} else {
		printk("MT5725_rpp_mode\n");
		return;
	}
	mt5725_fastcharge_timeout = 10;
}

/*
 * MT5725_good_int_delayed_work_func -- 0xffffff8008acdb50, 456 byte.
 * DUE funzioni incorporate, entrambe NOMINATE dal proprio `__func__`:
 *   ech_wls_set_chrg_current  "ech_wls_set_chrg_current"@0xffffff800926316e
 *   set_charger_type          "set_charger_type"@0xffffff800926351b
 *
 *   "1a9f17e1 cset"@0xffffff8008acdb90   (online = !gpio_get_value(...))
 *   "b90fe6a1 str"@0xffffff8008acdb9c    (0xffffff800a110fe4)
 *   "321e03e1 orr"@0xffffff8008acdbb8    (IRQ_TYPE_LEVEL_HIGH = 4)
 *   "321d03e1 orr"@0xffffff8008acdbc4    (IRQ_TYPE_LEVEL_LOW = 8)
 *   "b90fe91f str"@0xffffff8008acdbe0    (0xffffff800a110fe8 = 0)
 *   "b900252a str"@0xffffff8008acdbe4    (0xffffff800a111024 = 200)
 *   "321e03e1 orr"@0xffffff8008acdc74    (POWER_SUPPLY_PROP_ONLINE = 4)
 *   "320003e1 orr"@0xffffff8008acdc90    (POWER_SUPPLY_PROP_CHARGE_TYPE = 1)
 *   "321d03e8 orr"@0xffffff8008acdc88    (valore 8)
 *   "97d9b434 bl"@0xffffff8008acdce4     (enable_irq)
 *   "\x016wls_work_online: %d.\n"@0xffffff8009263503
 *   "charger"@0xffffff800925fee0
 *   "\x016set_charger_type: %d.\n"@0xffffff800926352c
 *   "\x016POWER_SUPPLY_PROP_CHARGE_TYPE: %d.\n"@0xffffff8009263545
 */
static void MT5725_good_int_delayed_work_func(struct work_struct *work)
{
	union power_supply_propval val;
	struct power_supply *psy;

	mt5725_wls_online = !gpio_get_value(mt5725_chip_q->c64);
	pr_info("wls_work_online: %d.\n", mt5725_wls_online);

	if (mt5725_wls_online == 1) {
		irq_set_irq_type(mt5725_good_irq_num, IRQ_TYPE_LEVEL_HIGH);
	} else {
		irq_set_irq_type(mt5725_good_irq_num, IRQ_TYPE_LEVEL_LOW);
		mt5725_good_irq_ready = 0;
		mt5725_g111024[0] = 200;
		ech_wls_set_chrg_current(200, 200);
	}

	/* ---- set_charger_type, incorporata ---- */
	psy = power_supply_get_by_name("charger");
	if (!psy) {
		pr_info("%s: get power supply failed\n", "set_charger_type");
	} else {
		val.intval = mt5725_wls_online;
		pr_info("set_charger_type: %d.\n", val.intval);
		power_supply_set_property(psy, POWER_SUPPLY_PROP_ONLINE, &val);
		if (val.intval) {
			val.intval = 8;
			if (!power_supply_set_property(psy,
					POWER_SUPPLY_PROP_CHARGE_TYPE, &val))
				pr_info("POWER_SUPPLY_PROP_CHARGE_TYPE: %d.\n",
					val.intval);
		} else {
			power_supply_set_property(psy,
					POWER_SUPPLY_PROP_CHARGE_TYPE, &val);
		}
	}
	/* ---- fine set_charger_type ---- */

	enable_irq(mt5725_good_irq_num);
}


/*
 * mt5725_charger_work_func -- 0xffffff8008acd940, 528 byte.
 * ech_wls_set_chrg_current e' INCORPORATA in tutti e due i rami
 * ("ech_wls_set_chrg_current"@0xffffff800926316e).
 *   "11019108 add"@0xffffff8008acd968   (+100)
 *   "5282ede8 mov"@0xffffff8008acd974   (5999)
 *   "540003ec b.gt"@0xffffff8008acd97c  (confronto CON SEGNO: qui la tensione
 *                                        e' un intero con segno, al contrario
 *                                        di MT5725_check_chargetimeout)
 *   "710fa6bf cmp"@0xffffff8008acd994   (1001) / "7112c6bf cmp"@0xffffff8008acda0c (1201)
 *   "1b087d21 mul"@0xffffff8008acda94   (x1000)
 *   "52801f43 mov"@0xffffff8008acdb1c   (ritardo 250)
 *   "5V set charger current %d\n "@0xffffff80092634cb
 *   "9V set charger current %d\n "@0xffffff80092634e7
 */
static void mt5725_charger_work_func(struct work_struct *work)
{
	int cur;

	mt5725_g111024[0] += 100;

	if (battery_get_vbus() <= 5999) {
		cur = mt5725_g111024[0];
		mt5725_g111024[1] = 1000;
		if (cur > 1000) {
			cur = 1000;
			mt5725_g111024[0] = 1000;
		}
		ech_wls_set_chrg_current(cur, 1000);
		charger_dev_set_input_current(mt5725_chg_dev,
					      mt5725_g111024[0] * 1000);
		printk("5V set charger current %d\n ", mt5725_g111024[0]);
		if (mt5725_g111024[0] <= 999)
			schedule_delayed_work(&mt5725_chip_q->c216, 250);
	} else {
		cur = mt5725_g111024[0];
		mt5725_g111024[1] = 1500;
		if (cur > 1200) {
			cur = 1200;
			mt5725_g111024[0] = 1200;
		}
		ech_wls_set_chrg_current(cur, 1500);
		charger_dev_set_input_current(mt5725_chg_dev,
					      mt5725_g111024[0] * 1000);
		printk("9V set charger current %d\n ", mt5725_g111024[0]);
		if (mt5725_g111024[0] <= 1199)
			schedule_delayed_work(&mt5725_chip_q->c216, 250);
	}
}

/*
 * mt5725_charger_routine_thread -- 0xffffff8008acddbc, 432 byte.
 * Non esce mai: il binario non ha `ret`.
 *   "7101e11f cmp"@0xffffff8008acdf04   (120)
 *   "52800ae2 mov"@0xffffff8008acdf18   (codice tasto 87)
 *   "97fde238 bl"@0xffffff8008acdf20    (input_event)
 *   "52800502 mov"@0xffffff8008acde64   (0x28) / "52800462 mov"@0xffffff8008acde80 (0x23)
 *                                        -> il valore 0x2328 ai registri 0x22/0x23
 *   "321903e2 orr"@0xffffff8008acdea8   (0x80 al registro 0x06)
 *   "52807d00 mov"@0xffffff8008acde0c   (msleep(1000))
 *   "rxdetect_count = 0\n"@0xffffff80092637ea
 *   "KEY_RXDETECT_STATUS 0\n"@0xffffff80092637d3
 *   " MT5725_write_buffer(mte, REG_VFC, temp.ptr, 2)\n"@0xffffff80092637fe
 * Quest'ultimo messaggio e' la fonte dei nomi `temp`, `ptr` e `REG_VFC`
 * (REG_VFC = 0x22, dal registro usato); `mte` e' il nome della variabile del
 * chip in questa funzione, ma qui il chip si legge dal globale, quindi il
 * nome non compare.
 */
int mt5725_charger_routine_thread(void *data)
{

	while (1) {
		if (mt5725_rvs_online == 1 && mt5725_chip_q->c80 == 0) {
			mt5725_rxdetect_count++;
			if (mt5725_rxdetect_count >= 120) {
				input_event(mt5725_input, EV_KEY, 87, 1);
				input_event(mt5725_input, EV_SYN, SYN_REPORT, 0);
				input_event(mt5725_input, EV_KEY, 87, 0);
				input_event(mt5725_input, EV_SYN, SYN_REPORT, 0);
				printk("KEY_RXDETECT_STATUS 0\n");
			}
		} else {
			mt5725_rxdetect_count = 0;
			printk("rxdetect_count = 0\n");
		}

		if (mt5725_g11101c >= 2) {
			mt5725_g11101c--;
		} else if (mt5725_g11101c == 1) {
			/* Scrittore di PAROLA, non di buffer: la fabbrica
			 * ripiega tutti e due i byte in immediati e non tocca
			 * la pila. Con un buffer il secondo verrebbe riletto
			 * da [sp,#9]. */
			c_acab34(mt5725_chip_p, 0x0022, 0x2328);
			c_acab34(mt5725_chip_p, 0x0006, 0x0080);
			mt5725_g11101c--;
			printk(" MT5725_write_buffer(mte, REG_VFC, temp.ptr, 2)\n");
		}

		msleep(1000);
	}
}

/*
 * mt5725_regmap_config -- 0xffffff8008f86218, 216 byte in `.rodata`.
 * Letto con relocazioni.py: reg_bits = 16, val_bits = 8, tutto il resto zero.
 *   "91086021 add"@0xffffff8008acd3d8   (l'indirizzo passato a __regmap_init_i2c)
 */
static const struct regmap_config mt5725_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
};

/*
 * MT5725_probe -- 0xffffff8008acd340, 1304 byte.
 *
 * DUE funzioni sono INCORPORATE qui, e il binario le NOMINA con il loro
 * `__func__` passato come `%s`:
 *   MT5725_parse_dt        "MT5725_parse_dt"@0xffffff8009263591
 *   gpio_wpc_rt_mode_init  "gpio_wpc_rt_mode_init"@0xffffff800926379b
 * Nessuna delle due sta nella mappa: esistono solo incorporate. Sono scritte
 * qui in linea, con i loro nomi come LETTERALI (non `__func__`): e' cio' che
 * il binario contiene.
 *
 *   "52806801 mov"@0xffffff8008acd378   (832 = sizeof(struct mt5725_chip))
 *   "97e89237 bl"@0xffffff8008acd384    (devm_kmalloc, cioe' devm_kzalloc)
 *   "12800175 mov"@0xffffff8008acd530   (-12 = -ENOMEM)
 *   "12800015 mov"@0xffffff8008acd54c   (-1)
 *   "128002b5 mov"@0xffffff8008acd560   (-22 = -EINVAL)
 *   "97d97c4e bl"@0xffffff8008acd3c4    (__mutex_init: `mutex_init(&chip->slock)`)
 *   "97e91612 bl"@0xffffff8008acd3e8    (__regmap_init_i2c)
 *   "7107fc3f cmp"@0xffffff8008acd5d4   (gpio contro 511, confronto SENZA segno)
 *   "320003e2 orr"@0xffffff8008acd5e4   (GPIOF_IN)
 *   "52840104 mov"@0xffffff8008acd6ec   (0x2008 = IRQF_TRIGGER_LOW|IRQF_ONESHOT)
 *   "12800002 mov"@0xffffff8008acd7e4   (NUMA_NO_NODE = -1 in kthread_create)
 *   "b140041f cmn"@0xffffff8008acd7f0   (IS_ERR)
 *   "primary_chg"@0xffffff8009150aee
 *   "%s: get primary charger device failed\n"@0xffffff8009263341 (con KERN_NOTICE)
 *   "MT5725 parent regmap is missing\n"@0xffffff8009263392 (con KERN_ERR)
 *   "%s : irq_gpio request failed\n"@0xffffff80092633d0 (con KERN_ERR)
 *   "failed to request IRQ %d : %d\n"@0xffffff80092633fc (con KERN_ERR)
 *   "sucess to request IRQ %d : %d\n"@0xffffff800926341d (con KERN_ERR)
 *   "%s The interruption has come \n"@0xffffff800926343e (con KERN_ERR)
 *   "%s skipping IRQ registration\n"@0xffffff800926345f (con KERN_INFO)
 *   "%s : no dc gpio provided \n "@0xffffff8009263573 (con KERN_ERR)
 *   "%s:dc gpio provided od.mt5725->dc_gpio = %d\n"@0xffffff80092635a1 (con KERN_INFO)
 *   "%s : no irq gpio provided \n "@0xffffff80092635d9 (con KERN_ERR)
 *   "%s:irq gpio provided od.mt5725->irq_gpio = %d\n"@0xffffff80092635f8 (con KERN_INFO)
 *   "%s get gpio_wpc_rt_mode_pin failed!\n"@0xffffff8009263774 (con KERN_ERR)
 *   "%s gpio_request failed, gpio=%d\n"@0xffffff80091522cd (con KERN_ERR)
 *   "find usb node failed\n"@0xffffff8009152307 (con KERN_ERR)
 *   "mediatek,usb_iddig_bi_eint"@0xffffff800915226a
 *   "MT5725 probe.\n"@0xffffff800926332c
 *   "0.0.1"@0xffffff800926333b
 *   "MT5725 chip.\n"@0xffffff8009263384
 *   "MT5725 regmap.\n"@0xffffff80092633b5
 *   "mt5725_int"@0xffffff80092633c5
 *   "mt5725_good"@0xffffff80092633f0
 *   "MT5725 probed successfully\n"@0xffffff800926347f
 *   "reverse_charger_wake_lock"@0xffffff800926349b
 *   "mt5725_charger_thread"@0xffffff80092634b5
 *   "dc-gpio"@0xffffff800926356b
 *   "eint_wpc"@0xffffff80092635d0
 *   "gpio_wpc_rt_mode_pin"@0xffffff800926375f
 *   "set gpio_wpc_rt_mode_pin success\n"@0xffffff80092637b1
 *
 * Il corpo usa il GLOBALE `mt5725_chip_q` e non un locale: dopo ogni chiamata
 * il puntatore e' RILETTO dalla memoria ("f94006c8 ldr"@0xffffff8008acd5cc).
 * L'unico accesso che passa dall'altro globale e' la rilettura del livello del
 * gpio ("f94002e8 ldr"@0xffffff8008acd828, cioe' 0xffffff800a111000).
 */
static int MT5725_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct mt5725_chip *chip;
	struct task_struct *task;
	struct device_node *node;
	int gpio, irq, ret;

	printk("MT5725 probe.\n");
	printk("0.0.1");

	mt5725_chip_q = devm_kzalloc(&client->dev,
				     sizeof(struct mt5725_chip), GFP_KERNEL);
	if (!mt5725_chip_q)
		return -ENOMEM;

	mt5725_chg_dev = get_charger_by_name("primary_chg");
	if (!mt5725_chg_dev) {
		printk(KERN_NOTICE "%s: get primary charger device failed\n",
		       __func__);
		return -1;
	}

	/*
	 * `mutex_init` stringa il proprio argomento: perche' il binario contenga
	 * "&chip->slock" la variabile DEVE chiamarsi `chip`. E' l'unico punto
	 * di MT5725_probe in cui il puntatore passa da un locale.
	 */
	chip = mt5725_chip_q;
	mutex_init(&chip->slock);
	printk("MT5725 chip.\n");

	mt5725_chip_q->c24 = regmap_init_i2c(client, &mt5725_regmap_config);
	if (!mt5725_chip_q->c24) {
		printk(KERN_ERR "MT5725 parent regmap is missing\n");
		return -EINVAL;
	}
	printk("MT5725 regmap.\n");

	mt5725_chip_q->c8 = client;
	mt5725_chip_q->c16 = &client->dev;
	mt5725_chip_q->c32 = MT5725_read;
	mt5725_chip_q->c40 = MT5725_write;
	mt5725_chip_q->c48 = MT5725_read_buffer;
	mt5725_chip_q->c56 = MT5725_write_buffer;
	mt5725_chip_p = mt5725_chip_q;
	mt5725_chip_q->c412 = 0;
	INIT_DELAYED_WORK(&mt5725_chip_q->c120, MT5725_int_delayed_work_func);
	INIT_DELAYED_WORK(&mt5725_chip_q->c216, mt5725_charger_work_func);
	/*
	 * Da qui il puntatore resta in un REGISTRO fino alla fine di
	 * MT5725_parse_dt incorporata: la fabbrica lo carica una volta
	 * ("f94006d5 ldr"@0xffffff8008acd4e8) e poi lo riusa attraverso la
	 * chiamata a of_get_named_gpio_flags ("b90042a0 str"@0xffffff8008acd50c
	 * scrive su quel registro, non su un valore riletto).
	 */
	INIT_DELAYED_WORK(&mt5725_chip_q->c312,
			  MT5725_good_int_delayed_work_func);

	/* ---- MT5725_parse_dt, incorporata ---- */
	chip = mt5725_chip_q;
	gpio = of_get_named_gpio_flags(client->dev.of_node, "dc-gpio", 0, NULL);
	chip->c64 = gpio;
	if (gpio < 0)
		printk(KERN_ERR "%s : no dc gpio provided \n ",
		       "MT5725_parse_dt");
	else
		printk(KERN_INFO
		       "%s:dc gpio provided od.mt5725->dc_gpio = %d\n",
		       "MT5725_parse_dt", gpio);

	gpio = of_get_named_gpio_flags(client->dev.of_node, "eint_wpc", 0, NULL);
	chip->c72 = gpio;
	if (gpio < 0)
		printk(KERN_ERR "%s : no irq gpio provided \n ",
		       "MT5725_parse_dt");
	else
		printk(KERN_INFO
		       "%s:irq gpio provided od.mt5725->irq_gpio = %d\n",
		       "MT5725_parse_dt", gpio);
	/* ---- fine MT5725_parse_dt ---- */

	if ((unsigned int)mt5725_chip_q->c64 <= 511) {
		ret = devm_gpio_request_one(&client->dev, mt5725_chip_q->c64,
					    GPIOF_IN, "mt5725_int");
		if (ret) {
			printk(KERN_ERR "%s : irq_gpio request failed\n",
			       __func__);
			return ret;
		}

		irq = gpio_to_irq(mt5725_chip_q->c72);
		mt5725_irq_num = irq;
		devm_request_threaded_irq(&client->dev, irq, NULL, MT5725_irq,
					  IRQF_TRIGGER_LOW | IRQF_ONESHOT,
					  "mt5725", mt5725_chip_q);
		irq_set_irq_wake(mt5725_irq_num, 1);

		irq = gpio_to_irq(mt5725_chip_q->c64);
		mt5725_good_irq_num = irq;
		ret = devm_request_threaded_irq(&client->dev, irq, NULL,
						MT5725_good_irq,
						IRQF_TRIGGER_LOW | IRQF_ONESHOT,
						"mt5725_good", mt5725_chip_q);
		irq_set_irq_wake(mt5725_good_irq_num, 1);
		if (ret) {
			printk(KERN_ERR "failed to request IRQ %d : %d\n",
			       gpio_to_irq(mt5725_chip_q->c72), ret);
			return ret;
		}
		printk(KERN_ERR "sucess to request IRQ %d : %d\n",
		       gpio_to_irq(mt5725_chip_q->c72), ret);
		if (!gpio_get_value(mt5725_chip_p->c72)) {
			printk(KERN_ERR "%s The interruption has come \n",
			       __func__);
			MT5725_irq_handle();
		}
	} else {
		printk(KERN_INFO "%s skipping IRQ registration\n", __func__);
	}

	ret = sysfs_create_group(&client->dev.kobj, &mt5725_attr_group);
	printk("MT5725 probed successfully\n");

	/* ---- gpio_wpc_rt_mode_init, incorporata ---- */
	node = of_find_compatible_node(NULL, NULL, "mediatek,usb_iddig_bi_eint");
	if (!node) {
		printk(KERN_ERR "find usb node failed\n");
	} else {
		mt5725_rt_mode_pin = of_get_named_gpio_flags(node,
					"gpio_wpc_rt_mode_pin", 0, NULL);
		if (mt5725_rt_mode_pin < 0) {
			printk(KERN_ERR "%s get gpio_wpc_rt_mode_pin failed!\n",
			       "gpio_wpc_rt_mode_init");
		} else if (gpio_request(mt5725_rt_mode_pin,
					"gpio_wpc_rt_mode_pin") < 0) {
			printk(KERN_ERR "%s gpio_request failed, gpio=%d\n",
			       "gpio_wpc_rt_mode_init", mt5725_rt_mode_pin);
		} else {
			gpio_direction_output(mt5725_rt_mode_pin, 0);
			printk("set gpio_wpc_rt_mode_pin success\n");
		}
	}
	/* ---- fine gpio_wpc_rt_mode_init ---- */

	wakeup_source_init(&mt5725_ws, "reverse_charger_wake_lock");

	task = kthread_create(mt5725_charger_routine_thread, NULL,
			      "mt5725_charger_thread");
	if (!IS_ERR(task))
		wake_up_process(task);

	reverse_charger_rxdetect();

	return ret;
}

/* ------------------------------------------------------------------------ */
/* Le tabelle di riconoscimento e la struct del driver, lette dalle
 * rilocazioni dell'oracolo.
 *   "mediatek,mt5725"@0xffffff8008f86088 (compatible, of_device_id a 0xf86048)
 *   "mt5725"@0xffffff8008f861d8          (i2c_device_id a 0xf861d8)
 *   struct i2c_driver a 0xffffff80099a74d8 (l'indirizzo passato a
 *   i2c_register_driver da mt5725_driver_init:
 *   "91136021 add"@0xffffff8009385a6c)
 */
static const struct of_device_id mt5725_match_table[] = {
	{ .compatible = "mediatek,mt5725", },
	{ },
};

static const struct i2c_device_id mt5725_id[] = {
	{ "mt5725", 0 },
	{ },
};

static struct i2c_driver mt5725_driver = {
	.driver = {
		.name = "mt5725",
		.owner = THIS_MODULE,
		.of_match_table = mt5725_match_table,
	},
	.probe = MT5725_probe,
	.remove = MT5725_remove,
	.id_table = mt5725_id,
};

/*
 * mt5725_driver_init -- 0xffffff8009385a60, 32 byte, in `.init.text`
 * mt5725_driver_exit -- 0xffffff80093adb50, 28 byte, in `.exit.text`
 *
 * SCOPERTA di questo lotto: `.exit.text` sta OLTRE `_einittext`
 * (0xffffff80093a8518) e quindi fuori da `oracolo/stock.map`; nessun rapporto
 * precedente aveva contato mt5725_driver_exit. La sua identificazione e'
 * certa perche' materializza l'indirizzo della NOSTRA struct i2c_driver:
 *   "d0002fc0 adrp"@0xffffff80093adb58   (pagina 0xffffff80099a7000)
 *   "91136000 add"@0xffffff80093adb5c    (+0x4d8 -> 0xffffff80099a74d8)
 *   "97db72d8 bl"@0xffffff80093adb60     (i2c_del_driver)
 * e la funzione che la segue (0xffffff80093adb6c) de-registra la
 * platform_driver di `usb_insert_detect` (0xffffff80099a7708), cioe' lo stesso
 * ordine di link che hanno mt5725_driver_init e usb_insert_detect_init in
 * `.init.text`.
 *
 * I due nomi sono quelli che genera `module_i2c_driver(mt5725_driver)`
 * (module_driver crea `<driver>_init` e `<driver>_exit`), e il primo argomento
 * nullo di i2c_register_driver e' THIS_MODULE compilato dentro il kernel:
 *   "aa1f03e0 mov"@0xffffff8009385a70
 *   "97dc12d1 bl"@0xffffff8009385a74
 */
module_i2c_driver(mt5725_driver);

/*
 * NON scritti: MODULE_LICENSE, MODULE_DESCRIPTION, MODULE_AUTHOR e
 * MODULE_DEVICE_TABLE. L'immagine dell'oracolo non contiene NESSUNA
 * `.modinfo`:
 *   ./venv/bin/python3 - <<'EOF'
 *   d = open('oracolo/stock.elf','rb').read()
 *   for t in (b'license=GPL', b'description=', b'author=', b'alias=i2c:'):
 *       print(t, d.count(t))
 *   EOF
 *   b'license=GPL' 0
 *   b'description=' 0
 *   b'author=' 0
 *   b'alias=i2c:' 0
 * Di quelle righe non si puo' derivare nulla -- ne' che ci siano, ne' quale
 * testo abbiano. Scriverle sarebbe inventare (regola 6).
 */
