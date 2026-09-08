// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_GT917S_E977 -- il sotto-lotto DICHIARATO del blocco Goodix `gt1x_*`.
 * Doogee S88 Pro (MediaTek MT6771).
 *
 * QUATTRO funzioni su 87, 620 byte su 38.000. Non e' il blocco: e' la parte
 * del blocco che NON si puo' ottenere compilando ALPS, piu' la funzione che
 * nessuno strumento del progetto poteva vedere. Il resto del lotto e' una
 * MISURA, non una scrittura: vedi «COSA QUESTO FILE NON CONTIENE» in fondo.
 *
 * TERZA STESURA. La prima era misurata con clang-r383902; il compilatore
 * VERO della fabbrica e' clang-r353983c, e la seconda stesura ha rifatto con
 * lui ogni numero (vedi «IL METRO»). La terza aggiunge `dump_to_file`, la
 * quarta funzione, e la misura della VARIANTE ALPS -- GT5688 contro GT1151 --
 * che nessuno aveva mai fatto per dimensione.
 *
 *   gt1x_gesture_debug   0xffffff8008a72a38   132 byte   assente da ALPS
 *   get_module_name      0xffffff8008a76ca8    76 byte   assente da ALPS
 *   dump_to_file         0xffffff8008a7aed8   360 byte   assente da ALPS
 *   tpd_driver_exit      0xffffff80093acaf8    52 byte   FUORI da stock.map
 *
 * Tutte e quattro misurano ESATTAMENTE come di fabbrica. Su QUATTRO unita'
 * la misura non discrimina in nessuna direzione, e va detto prima del numero:
 *   $ ./venv/bin/python3 intervallo.py 4 4
 *   4 su 4 = 100.0%   IC95% Clopper-Pearson [39.8%; 100.0%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *
 * RICOSTRUITE DAL DISASSEMBLATO DEL KERNEL DI FABBRICA. Nessun sorgente
 * pubblico Goodix e' stato letto. L'albero ALPS (GT1151/, GT5688/) E' stato
 * letto e compilato -- e' il mandato di questo lotto -- ma NON per copiarne
 * il codice: le quattro funzioni qui sotto in ALPS non esistono. Dove un
 * NOME viene da ALPS e non dal binario, e' detto riga per riga.
 *
 * Ogni riga derivata porta accanto la riga di disassemblato che la
 * giustifica, nella forma "<codifica8hex> <mnemonico>"@0xINDIRIZZO.
 *
 * ===========================================================================
 * COME SI RIVERIFICA (i comandi esatti)
 * ===========================================================================
 *
 *   cd /mnt/s88pro/kernel-stock
 *
 *   # 1. le due verifiche di citazione
 *   ./venv/bin/python3 verificacitazioni.py \
 *       kernel/doogee/S88Pro/driver/gt917s/gt1x_wtk.c oracolo/stock.elf
 *   #   ATTENZIONE: questa ESCE 1, e non per un difetto di questo file.
 *   #   Vedi «LE DIECI CITAZIONI NON ANCORATE» in fondo: il perche' e'
 *   #   scritto riga per riga, e dieci sono dieci, non «quelle delle
 *   #   funzioni altrui». L'ALTRA, verificaistruzioni.py, ESCE 0.
 *   ./venv/bin/python3 verificaistruzioni.py \
 *       kernel/doogee/S88Pro/driver/gt917s/gt1x_wtk.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a72a38:0xffffff8008a72abc \
 *       --intervallo 0xffffff8008a76ca8:0xffffff8008a76cf4 \
 *       --intervallo 0xffffff80093acaf8:0xffffff80093acb2c \
 *       --intervallo 0xffffff8008a7aed8:0xffffff8008a7b040 \
 *       --intervallo 0xffffff8008a7221c:0xffffff8008a72308 \
 *       --intervallo 0xffffff8008a777f8:0xffffff8008a77ab4 \
 *       --controfattuale '9ba92008 umaddl' \
 *       --controfattuale '39400108 ldrb'  --controfattuale '9b092900 madd' \
 *       --controfattuale 'd28dcea8 mov'   --controfattuale 'f2adcd68 movk' \
 *       --controfattuale 'f2ceede8 movk'  --controfattuale 'f2e00dc8 movk' \
 *       --controfattuale 'f9000028 str'
 *   #   ESITO=0. I sette `--controfattuale` in coda al primo non sono forme
 *   #   di sorgente scartate: sono le istruzioni dei NOSTRI oggetti di prova
 *   #   (le due controprove `con_int`/`con_unsigned` e la variante
 *   #   "unknown"), incollate qui come listati di objdump. Nel binario di
 *   #   FABBRICA non ci sono -- e' il punto -- e senza la dichiarazione lo
 *   #   strumento le segnala ASSENTE e ha ragione. La prima stesura le
 *   #   lasciava non dichiarate e usciva 1 con dodici ASSENTE.
 *
 *   Il TERZO intervallo sta OLTRE `_einittext` (0xffffff80093a8518): non
 *   compare in `oracolo/stock.map`, quindi `disassembla.py` non lo trova per
 *   nome. Si legge solo per indirizzo:
 *       aarch64-linux-android-objdump -d oracolo/stock.elf \
 *           --start-address=0xffffff80093acaf8 --stop-address=0xffffff80093acb2c
 *   Il QUARTO intervallo e' `gesture_enter_doze` (la prova della forma di
 *   GTP_DEBUG) e il QUINTO e' `tpd_i2c_probe`, dove `get_module_name` e'
 *   INCORPORATA. ATTENZIONE: il quinto NON e' «la seconda prova indipendente
 *   della firma», come diceva la prima stesura -- la' l'indice arriva da un
 *   `ldrb` e la forma della moltiplicazione non discrimina il segno. Vedi il
 *   commento di `get_module_name`. Serve a mostrare l'incorporamento, non il
 *   tipo.
 *
 *   # 2. la compilazione e la misura, dal .o vero.
 *   #    IL PATH PORTA IL COMPILATORE DI FABBRICA, non r383902: e' la
 *   #    differenza fra 25 e 34 funzioni identiche su 80. Vedi «IL METRO».
 *   docker run --rm --user $(id -u):$(id -g) \
 *     -v /home/alops/lineage:/srv/twrp -v /mnt/s88pro:/srv/archive -e HOME=/tmp \
 *     -e PATH=/srv/archive/kernel-stock/clang-r353983c/bin:/srv/twrp/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin:/usr/bin:/bin:/usr/sbin:/sbin \
 *     s88pro-build bash -lc "make -C /srv/archive/kernel-stock/albero-gt917s \
 *       O=/srv/archive/kernel-stock/out-gt917s ARCH=arm64 -j6 CC=clang HOSTCC=clang \
 *       CLANG_TRIPLE=aarch64-linux-gnu- CROSS_COMPILE=aarch64-linux-android- \
 *       KCFLAGS=-Werror drivers/input/touchscreen/mediatek/GT917S/"
 *   aarch64-linux-android-nm --print-size \
 *     out-gt917s/drivers/input/touchscreen/mediatek/GT917S/gt1x_wtk.o | grep -iE " [tT] "
 *
 *   # 3. i 38.000 byte del blocco, che il cappello dichiara
 *   awk -F'\t' '{s+=$4} END {print s"  +52 (.exit) = "s+52}' scout-gt917s/elenco.txt
 *   -> 37948  +52 (.exit) = 38000
 *
 * ===========================================================================
 * IL METRO: IL COMPILATORE, E PERCHE' CAMBIA NOVE NUMERI SU OTTANTA
 * ===========================================================================
 * Il kernel di fabbrica dice DA SOLO con che compilatore e' stato costruito:
 *
 *   $ strings oracolo/stock.elf | grep -m1 'Linux version'
 *   Linux version 4.14.141 (nobody@android-build) (Android (5484270 based on
 *   r353983c) clang version 9.0.3 (...) (based on LLVM 9.0.3svn)) #1 SMP
 *   PREEMPT Tue Jun 23 09:55:04 CST 2020
 *
 * Quel compilatore -- `clang-r353983c` -- e' stato recuperato il 2026-08-21
 * alle 10:05 (commit 540e404, «il compilatore di fabbrica e' stato
 * recuperato, e cambia il metro di misura») ed e' collegato in
 * `/mnt/s88pro/kernel-stock/clang-r353983c`. La sua stringa di versione,
 * letta dentro il container, coincide con quella dell'oracolo:
 *
 *   $ clang-r353983c/bin/clang --version | head -1
 *   Android (5484270 based on r353983c) clang version 9.0.3 (...)
 *
 * La PRIMA stesura di questo file fu misurata con `clang-r383902`, cioe' con
 * il metro sbagliato, e il numero-bandiera del lotto («la convergenza vera e'
 * 25 su 80») era il numero di quel metro. Rifatta la misura con kbuild vero,
 * stesso albero, stessa riga di comando, cambiato SOLO il PATH:
 *
 *   funzioni identiche/80          r383902 (nostro)   r353983c (FABBRICA)
 *   ALPS puro (sola typedef D2)          20                  25
 *   ALPS + D1 + D5 (questo albero)       25                  34
 *
 *   $ ./corr-gt917s/matrice.sh <clang> <D1> <D5> <etichetta>
 *   ALPS puro (sola typedef D2)  compilatore=clang-r383902  totale=87 presenti=80 identiche=20 assenti=7
 *   ALPS + D1 + D5               compilatore=clang-r383902  totale=87 presenti=80 identiche=25 assenti=7
 *   ALPS puro (sola typedef D2)  compilatore=clang-r353983c totale=87 presenti=80 identiche=25 assenti=7
 *   ALPS + D1 + D5               compilatore=clang-r353983c totale=87 presenti=80 identiche=34 assenti=7
 *
 * Tutti i numeri di questo file sono, da adesso, quelli della colonna di
 * destra. Le NOVE funzioni che passano a identiche cambiando solo il
 * compilatore sono: `__gt1x_hold_ss51_dsp_20`, `_do_i2c_read`,
 * `_do_i2c_write`, `comfirm`, `gt1x_gesture_data_write`, `gt1x_get_info`,
 * `gt1x_load_patch`, `gt1x_power_reset`, `gt1x_read_version`. La prima
 * stesura ne attribuiva quattro al sorgente («una `kfree` in piu' della
 * fabbrica») e due le dichiarava residuo inspiegabile: non erano ne' l'uno
 * ne' l'altro, erano il metro.
 *
 * Cosa e' caduto insieme al metro sbagliato:
 *  - la divergenza G7 di `get_module_name` (80 contro 76) NON ESISTE: col
 *    compilatore di fabbrica misura 0x4c = 76 e il confronto istruzione per
 *    istruzione da' 19 su 19;
 *  - il «caso da manuale della regola 3» costruito sulla stringa "unknow"
 *    insegnava l'esatto contrario del vero: vedi il commento di
 *    `get_module_name`;
 *  - l'affermazione «su questo blocco la dimensione e' l'UNICO criterio
 *    disponibile, l'identita' istruzione per istruzione non e' raggiungibile
 *    con un compilatore diverso» era una istruzione operativa DANNOSA: il
 *    compilatore giusto c'e', e con lui l'identita' istruzione per
 *    istruzione si raggiunge. Su `gt1x_send_cmd` (scarto 0 con entrambi i
 *    compilatori) le divergenze normalizzate passano da 11 a 2, e le due che
 *    restano sono rilocazioni non risolte nel `.o`, non codice diverso.
 *  - le funzioni scritte qui passavano da 2 su 3 a 3 su 3 identiche al byte;
 *    con `dump_to_file`, aggiunta dalla terza stesura, sono 4 su 4. Su
 *    QUATTRO unita' la misura non discrimina in nessuna direzione, e va detto
 *    prima del numero:
 *      $ ./venv/bin/python3 intervallo.py 4 4
 *      4 su 4 = 100.0%   IC95% Clopper-Pearson [39.8%; 100.0%]
 *      contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *
 * ===========================================================================
 * IL CONFINE, E PERCHE' LA TERZA FUNZIONE ERA INVISIBILE
 * ===========================================================================
 * `oracolo/stock.map` finisce a `_einittext` (0xffffff80093a8518, riga 53710,
 * l'ultima del file). Tutto cio' che segue -- la `.exit.text` -- e' fuori
 * dalla portata di ogni strumento del progetto che parte dalla mappa
 * (clusterdriver.py, cluster_finale.py, classificasimboli.py, disassembla.py,
 * misurablocco.py). `tpd_driver_exit` sta a 0xffffff80093acaf8, cioe' 17.888
 * byte OLTRE quella fine: nessun rapporto l'aveva mai contata, e la
 * ricognizione `scout-gt917s/RIASSUNTO.md` -- che pure ha contato 86 funzioni
 * con la somma dei byte che chiude esatta -- non poteva vederla.
 *
 *   $ python3 -c "a=0xffffff80093acaf8; e=0xffffff80093a8518; print(a-e, hex(a-e))"
 *   17888 0x45e0
 *
 * (La prima stesura scriveva 17.376, sbagliato di 512 byte. Lo scarto giusto
 * lo stampa objdump da solo, come etichetta della funzione, perche' oltre
 * `_einittext` non c'e' piu' nessun simbolo a cui agganciarsi:
 *   ffffff80093acaf8 <_einittext+0x45e0>:
 * e 0x45e0 = 17888.)
 *
 * Si trova cosi', e la prova e' l'indirizzo della struttura passata:
 *   nella .exit.text ci sono TRE chiamate a `tpd_driver_remove`, una per
 *   ciascuno dei tre driver touch di questo esemplare, e si distinguono
 *   SOLO per l'argomento:
 *     0xffffff80093acaec -> tpd_driver_remove(0xffffff8009987338)  ilitek
 *     0xffffff80093acb20 -> tpd_driver_remove(0xffffff800998a578)  goodix
 *     0xffffff80093acb58 -> tpd_driver_remove(0xffffff800998c918)  focaltech
 *   e 0xffffff800998a578 e' la `tpd_driver_t` del Goodix, gia' letta dalla
 *   ricognizione (§6.1: `+0x00 -> 'gt1x'`, `+0x08 <tpd_local_init>`).
 *
 * CORREZIONE a `scout-gt917s/RIASSUNTO.md` §6.1, misurata qui: quel rapporto
 * scrive che «quello aperto e' la `tpd_driver_t` del focaltech a
 * 0xffffff8009987338». NON e' del focaltech: e' dell'ILITEK. Lo prova la
 * funzione che la de-registra, che stampa
 * "\x016ILITEK: (%s, %d): ilitek driver has been removed\n"@0xffffff800923f7b9
 * con `__func__` = "ilitek_plat_dev_exit"@0xffffff800923f7ed. La
 * `tpd_driver_t` del focaltech e' 0xffffff800998c918, quella la cui `.exit`
 * stampa "[FTS]%s: Enter\n"@0xffffff800924f634.
 *
 * ===========================================================================
 * LE DIVERGENZE APERTE DI QUESTO FILE
 * ===========================================================================
 * G1. I `__LINE__`, ed e' L'UNICA divergenza aperta rimasta in questo file.
 *     Il binario li porta e sono noti:
 *       gt1x_gesture_debug  431   ("528035e2 mov"@0xffffff8008a72aac)
 *       dump_to_file       1331   ("5280a662 mov"@0xffffff8008a7af1c)
 *                          1334   ("5280a6c2 mov"@0xffffff8008a7af58)
 *       tpd_driver_exit    1051   ("52808362 mov"@0xffffff80093acb10)
 *       get_module_name    -- non ne ha, non stampa niente
 *     Questo file NON li riproduce: imbottire di righe vuote un file che
 *     verra' ancora toccato sposta tutto al primo cambio. E' una divergenza
 *     VERA nel binario e INVISIBILE alla misura di dimensione, perche'
 *     `mov w2,#imm16` e' una istruzione per qualunque riga.
 *     COL COMPILATORE DI FABBRICA il confronto istruzione per istruzione
 *     dice esattamente quanto resta -- e non e' un residuo inspiegato:
 *     La terza stesura lo rifa' su tutte e quattro con un confronto che
 *     CLASSIFICA ogni divergenza invece di normalizzarla via
 *     (`corr-gt917s/cmp.py`: una divergenza e' RILOC se l'istruzione nostra
 *     porta una rilocazione, `__LINE__` se e' il `mov w2,#imm` del numero di
 *     riga, ALTRO altrimenti):
 *       $ ./venv/bin/python3 corr-gt917s/cmp.py <inizio> <fine> <o> \
 *             <off> <fine_off> <nome> <sezione> | tail -1
 *       gt1x_gesture_debug  divergenti: 19 su 33 -- RILOC=18, __LINE__=1
 *       get_module_name     divergenti:  3 su 19 -- RILOC=3
 *       dump_to_file        divergenti: 22 su 90 -- RILOC=20, __LINE__=2
 *       tpd_driver_exit     divergenti:  9 su 13 -- RILOC=8,  __LINE__=1
 *     155 istruzioni di fabbrica, 53 divergenti, 49 rilocazioni + 4
 *     `__LINE__`, e ZERO nella classe ALTRO: non resta nessun residuo
 *     inspiegato. Le 49 rilocazioni non sono «ignorate»: `objdump -r` sul
 *     nostro `.o` mostra che ognuna nomina il simbolo giusto, e per le
 *     stringhe gli SCARTI fra un letterale e il successivo nella nostra
 *     `.rodata.str1.1` sono gli stessi che ha la fabbrica (p.es. per
 *     `dump_to_file`: 0x7d-0x45 = 0x38 da noi e 0x8bd-0x885 = 0x38 di
 *     fabbrica, 0x8a-0x7d = 0xd e 0x8ca-0x8bd = 0xd).
 *     L'immediato del NOSTRO `.o` non e' un numero da inseguire: e' il
 *     numero di riga della `GTP_*` dentro QUESTO file, e cambia a ogni
 *     modifica del file, quindi qui NON e' scritto nessun valore: si rilegge
 *     con il comando del punto 1 e non va copiato a mano -- la prima stesura
 *     aveva
 *     incollato «#0x183 // #387», un valore rimasto da una versione
 *     precedente del sorgente, ed e' la ragione per cui adesso c'e' scritto
 *     COME lo si rilegge invece del solo numero.
 * G2. Il nome della tabella di 6x40 byte a 0xffffff800998a468. Il binario
 *     NON la nomina, e ALPS non ha ne' la tabella ne' la funzione che la
 *     legge. Qui si chiama `d998a468`, il suo indirizzo, e il commento dice
 *     solo cio' che e' MISURATO: sei righe da 40 byte, contenuto letto,
 *     ruolo provato dal formato "TP MODULE:%s" del chiamante. Un nome
 *     descrittivo sarebbe indistinguibile, per chi legge, da un fatto.
 * G3. `gesture_enabled`, `gestures_flag`, `gesture_doze_status`: questi tre
 *     nomi vengono da ALPS `GT5688/gt1x_extents.c` (righe 69, 72, 70), NON
 *     dal binario. Del binario e' misurato: l'indirizzo, la larghezza
 *     dell'accesso e la dimensione. Sono dichiarati qui perche' il file che
 *     li definisce e' `gt1x_extents.c`, che questo lotto non riscrive.
 * G4. `tpd_driver_exit` NON e' assente da ALPS -- ALPS ce l'ha, e la misura
 *     dice 52 byte contro 52. Sta qui lo stesso perche' e' la funzione che
 *     nessuno aveva contata e perche' il file che la contiene
 *     (`GT5688/gt1x_tpd.c`) non e' nostro. Se il lotto di merge decide di
 *     tenere il `gt1x_tpd.c` di ALPS, questa copia va TOLTA: sarebbe una
 *     doppia definizione. E' una decisione del merge, non una misura.
 * G5. `GTP_INFO` e `GTP_DEBUG` sono ridefinite QUI, localmente, invece che
 *     corrette in `GT5688/include/gt1x_tpd_common.h`. Vedi «DELTA DI HEADER».
 * G6. `gestures_flag` e' DICHIARATA E NON DEFINITA qui (`extern`), e ALPS la
 *     definisce `static` in `gt1x_extents.c:72`. NON e' una misura: e' il
 *     confine di unita', ed e' una voce che BLOCCA il merge -- vedi il
 *     commento sulla variabile e il delta D8. La prima stesura la DEFINIVA
 *     non-`static`, e quella forma si linkava in silenzio con l'array
 *     `static` di ALPS lasciando DUE array da 32 byte distinti.
 * G7. RITIRATA. Diceva: «`get_module_name` misura 80 byte contro 76, ed e'
 *     l'unica divergenza di dimensione di questo file; non e' il sorgente,
 *     e' il compilatore». La seconda meta' era giusta -- ed e' proprio per
 *     questo che la prima e' caduta: era il compilatore SBAGLIATO. Con
 *     `clang-r353983c` lo stesso identico sorgente, non una riga toccata,
 *     misura 0x4c = 76:
 *       $ aarch64-linux-android-nm --print-size gt1x_wtk.o | grep -iE " [tT] "
 *       0000000000000084 000000000000004c T get_module_name
 *       0000000000000000 0000000000000084 T gt1x_gesture_debug
 *       0000000000000000 0000000000000034 T tpd_driver_exit
 *     0x4c=76, 0x84=132, 0x34=52: le tre dimensioni della fabbrica. Questo
 *     file non ha piu' nessuna divergenza di dimensione.
 *     Il meccanismo descritto (il prologo dentro il ramo che chiama
 *     `strcpy`, cioe' shrink-wrapping) era letto bene, e adesso lo fa anche
 *     il nostro `.o`:
 *       "b4000108 cbz"@0xffffff8008a76cb8   salta al ramo "unknow"
 *       "a9bf7bfd stp"@0xffffff8008a76cbc   il prologo sta DOPO la cbz
 *       "d65f03c0 ret"@0xffffff8008a76cf0   il ramo "unknow" finisce senza ldp
 *     E cade con G7 anche la lezione che ne era stata tratta sulla stringa
 *     "unknow": col metro giusto la dimensione DENUNCIA il refuso. Vedi il
 *     commento di `get_module_name`.
 */

#include <linux/err.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/types.h>

/*
 * ===========================================================================
 * LE MACRO DI LOG, NELLA FORMA CHE IL BINARIO PROVA
 * ===========================================================================
 * ALPS definisce (GT5688/include/gt1x_tpd_common.h:207-216):
 *     GTP_INFO  -> pr_info("<<GTP-INF>>[%s:%d] " fmt "\n", ...)
 *     GTP_DEBUG -> do { if (tpd_em_log) pr_debug("<<GTP-DBG>>[%s:%d]" ...); } while (0)
 *
 * Il binario di fabbrica dice DUE cose diverse, e le dice da solo:
 *
 *  (a) NESSUN LIVELLO KERN. Il formato di fabbrica comincia con '<':
 *      "<<GTP-DBG>>[%s:%d]Entering doze mode...\n"@0xffffff800924a225
 *      "<<GTP-INF>>[%s:%d] MediaTek GT1x touch panel driver exit.\n"@0xffffff800924c05c
 *      Un `pr_info` avrebbe anteposto "\001" "6", come fa l'ilitek accanto:
 *      "\x016ILITEK: (%s, %d): ilitek driver has been removed\n"@0xffffff800923f7b9
 *      Quindi di fabbrica e' `printk(...)` nudo, non `pr_info(...)`.
 *
 *  (b) NESSUNA GUARDIA su GTP_DEBUG. In `gesture_enter_doze` la prima
 *      istruzione dopo il prologo carica il formato e chiama subito:
 *      "97db04a7 bl"@0xffffff8008a72238 e' un `bl printk` incondizionato,
 *      senza nessun `ldrb`+`tbz` su `tpd_em_log` davanti, e senza
 *      `__dynamic_pr_debug`. Con la forma ALPS la stessa funzione misura
 *      308 byte contro i 236 di fabbrica; con questa forma, 236 contro 236.
 *      Il conto e' nel rapporto: una sola macro ha portato le funzioni
 *      identiche da 20 a 25 su 80.
 */
#define GTP_INFO(fmt, arg...) \
	printk("<<GTP-INF>>[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)
#define GTP_DEBUG(fmt, arg...) \
	printk("<<GTP-DBG>>[%s:%d]" fmt "\n", __func__, __LINE__, ##arg)
/*
 * GTP_ERROR: stessa forma di GTP_INFO col tag ERR. Provata dai byte del
 * messaggio di `dump_to_file`, che porta lo spazio dopo la parentesi quadra
 * come GTP_INFO e a differenza di GTP_DEBUG:
 *   "<<GTP-ERR>>[%s:%d] can not open file: %s\n\n"@0xffffff800924d8ca
 */
#define GTP_ERROR(fmt, arg...) \
	printk("<<GTP-ERR>>[%s:%d] " fmt "\n", __func__, __LINE__, ##arg)

/*
 * ===========================================================================
 * CIO' CHE STA IN ALTRE UNITA' -- DICHIARATO, NON DEFINITO
 * ===========================================================================
 * Nessuno stub. Se il link fallisce e' l'esito onesto: dice che l'unita' che
 * definisce il simbolo non e' stata ancora scritta. Uno stub la farebbe
 * passare per scritta.
 */

/* Da `drivers/input/touchscreen/mediatek/tpd.h`: la struttura che
 * `tpd_driver_add`/`tpd_driver_remove` prendono. Qui serve solo il
 * puntatore, quindi il tipo resta incompleto. */
struct tpd_driver_t;
extern int tpd_driver_remove(struct tpd_driver_t *tpd_drv);

/*
 * L'istanza del Goodix: 0xffffff800998a578, letta dalla ricognizione (§6.1)
 * e confermata qui dall'argomento della de-registrazione,
 * "d0002ee0 adrp"@0xffffff80093acb18 + "9115e000 add"@0xffffff80093acb1c
 * (pagina 0xffffff800998a000 + 0x578). E' definita in `gt1x_tpd.c`.
 */
extern struct tpd_driver_t gt1x_tpd_driver;

/*
 * `gesture_enabled` -- 0xffffff800998a2d0. NOME DA ALPS (G3).
 * Del binario e' misurato: accesso a LARGHEZZA PIENA in scrittura,
 * "b902d109 str"@0xffffff8008a72a50 (`str w9,[x8,#720]`), quindi NON e'
 * una `static` a pochi valori ristretta a `strb` dal compilatore (classe
 * A1). ALPS la dichiara `int gesture_enabled;` senza `static`, e la misura
 * non la smentisce.
 */
extern int gesture_enabled;

/*
 * `gestures_flag` -- 0xffffff800a10087d, 32 byte. NOME DA ALPS (G3).
 * Del binario e' misurato:
 *  - la DIMENSIONE: quattro `stp` di coppie da 8 byte coprono +0 e +16,
 *    "a9012d4b stp"@0xffffff8008a72a54 e "a9002d4b stp"@0xffffff8008a72a58;
 *  - l'ALLINEAMENTO A 1: l'indirizzo finisce in 0xd
 *    ("9121f54a add"@0xffffff8008a72a48, pagina + 0x87d). Un `stp` a 64 bit
 *    su un indirizzo non allineato a 8 il compilatore lo emette solo se
 *    l'oggetto ha allineamento 1, cioe' e' un array di byte. E' la prova
 *    che il tipo e' `u8[32]` e non, per dire, `u32[8]`.
 *
 * NELLA FABBRICA L'ARRAY E' UNO SOLO, e lo dicono i tre siti che lo usano:
 * lo stesso spiazzamento 0x87d compare nella funzione che lo SCRIVE e nelle
 * due che lo LEGGONO, che di fabbrica stanno in unita' diverse.
 *   $ objdump -d stock.elf --start-address=... | grep -nE "#0x87d"
 *   ffffff8008a72a48:	9121f54a 	add	x10, x10, #0x87d   gt1x_gesture_debug
 *   ffffff8008a72a64:	9121f529 	add	x9, x9, #0x87d     gt1x_gesture_debug
 *   ffffff8008a724c0:	9121f529 	add	x9, x9, #0x87d     gesture_event_handler
 *   ffffff8008a730f4:	9121f529 	add	x9, x9, #0x87d     gt1x_ioctl
 *   ffffff8008a73130:	9121f529 	add	x9, x9, #0x87d     gt1x_ioctl
 *
 * PERCHE' QUI E' `extern`, cioe' DICHIARATA E NON DEFINITA (G6, delta D8).
 * ALPS la dichiara `static u8 gestures_flag[32];` in `gt1x_extents.c:72`, e
 * i suoi lettori (SETBIT/QUERYBIT dentro `gesture_event_handler` e
 * `gt1x_ioctl`) stanno nella STESSA unita' di traduzione. Qui la funzione e'
 * scritta da sola, e le tre forme possibili sono state MISURATE tutte e tre:
 *
 *   `static u8 gestures_flag[32];`   gt1x_gesture_debug = 0x60 = 96 byte.
 *       Nessun lettore nell'unita': clang elimina le quattro `stp` come
 *       scritture morte. Sbagliata: 96 contro 132.
 *   `u8 gestures_flag[32];`          gt1x_gesture_debug = 0x84 = 132 byte.
 *       La misura torna, MA il simbolo diventa una definizione globale che
 *       NON collide con la `static` di ALPS: ELF ammette un simbolo locale e
 *       uno globale con lo stesso nome, il link riesce senza un avviso, e
 *       restano DUE array da 32 byte. Chi scrive non e' chi legge.
 *         $ ld -r -o merge.o GT917S/gt1x_wtk.o GT5688/gt1x_extents.o
 *         LINK OK (nessun errore di doppia definizione)
 *         $ aarch64-linux-android-readelf -sW merge.o \
 *               | grep -E "gestures_flag|gesture_enabled"
 *             25: 0000000000000034    32 OBJECT  LOCAL  DEFAULT   10 gestures_flag
 *             69: 0000000000000028     4 OBJECT  GLOBAL DEFAULT   10 gesture_enabled
 *             79: 0000000000000000    32 OBJECT  GLOBAL DEFAULT   10 gestures_flag
 *       Due OBJECT da 32 byte, uno LOCAL (la `static` di ALPS) e uno GLOBAL
 *       (la nostra), e il link non protesta. La controprova che la fusione
 *       funziona quando i due lati concordano e' la riga di mezzo:
 *       `gesture_enabled`, non-`static` da entrambe le parti, da' UN simbolo
 *       GLOBAL solo.
 *       QUESTA PROVA ERA SCRITTA CON `nm --print-size`, e la forma e' stata
 *       cambiata per una ragione misurata, non estetica: le colonne di `nm`
 *       -- `<16 cifre esadecimali> <lettera di tipo> <nome>` -- combaciano
 *       con la forma "citazione di istruzione ad indirizzo" di
 *       `confinecitazioni.descrive_istruzione` quando la lettera di tipo e'
 *       `b`/`B`, perche' `b` E' un mnemonico di salto e per i salti la
 *       grammatica ammette un nome di simbolo come operando. Le tre righe
 *       incollate qui producevano tre citazioni FANTASMA
 *       (`0x20 b`, `0x20 b`, `0x4 b`) che `verificaistruzioni.py` segnalava
 *       ASSENTE, e che non si possono nemmeno dichiarare `--controfattuale`
 *       perche' la chiave `0x20` non ha ne' 8 ne' 9-16 cifre. Vedi il delta
 *       D11: la correzione vera sta nello strumento, non qui.
 *   `extern u8 gestures_flag[32];`   gt1x_gesture_debug = 0x84 = 132 byte,
 *       e nessuna definizione emessa:
 *         $ nm gt1x_wtk.o | grep -i gestures_flag
 *                          U gestures_flag
 *       La misura torna com'e' e il conflitto diventa RUMOROSO: il link vero
 *       si ferma invece di produrre due array.
 *         $ ld -o /dev/null gt1x_wtk.o
 *         gt1x_wtk.c: undefined reference to `gestures_flag'
 *         (quattro volte: due rilocazioni -- `adrp`+`add` -- per ciascuna
 *         delle due `memset`)
 *
 * E' la terza, ed e' la regola 6 applicata a una variabile: cio' che questo
 * file non definisce si dichiara e si lascia indefinito, e il link che
 * fallisce e' l'esito onesto. Quale delle due unita' debba DEFINIRLA -- e
 * se di fabbrica fosse `static` -- questa funzione da sola non lo decide:
 * lo decidera' chi scrive `gt1x_extents.c`. Vedi D8.
 */
extern u8 gestures_flag[32];

/*
 * `gesture_doze_status` -- 0xffffff800a1006f0. NOME DA ALPS (G3).
 * Del binario e' misurato: scrittura a larghezza piena azzerata,
 * "b906f15f str"@0xffffff8008a72a78 (`str wzr,[x10,#1776]`), e SOLO sul
 * ramo di spegnimento. Il valore 0 e' `DOZE_DISABLED` in ALPS; qui il tipo
 * resta `int` perche' il binario misura la larghezza, non l'enum.
 */
extern int gesture_doze_status;

/*
 * ===========================================================================
 * gt1x_gesture_debug -- 0xffffff8008a72a38, 132 byte, 33 istruzioni
 * ===========================================================================
 * Firma: un solo argomento in w0, testato due volte
 * ("34000120 cbz"@0xffffff8008a72a3c e "7100001f cmp"@0xffffff8008a72a94)
 * e mai esteso ne' con segno ne' senza: `int`. Nessun valore di ritorno --
 * "d65f03c0 ret"@0xffffff8008a72ab8 arriva subito dopo la `printk` e w0
 * non viene toccato.
 *
 * Il ramo ACCESO ("d000b46a adrp"@0xffffff8008a72a40 in poi):
 *   gesture_enabled = 1     "320003e9 orr"@0xffffff8008a72a44 (w9 = 1)
 *                           "b902d109 str"@0xffffff8008a72a50
 *   gestures_flag tutto a 0xff:
 *                           "9280000b mov"@0xffffff8008a72a4c (x11 = -1,
 *                           cioe' otto byte 0xff)
 *                           "a9012d4b stp"@0xffffff8008a72a54  (+16)
 *                           "a9002d4b stp"@0xffffff8008a72a58  (+0)
 *   e NON tocca gesture_doze_status: sul ramo acceso non c'e' nessun
 *   accesso a 0xffffff800a1006f0. E' un'asimmetria della fabbrica e si
 *   riproduce com'e'.
 *
 * Il ramo SPENTO ("d000b469 adrp"@0xffffff8008a72a60 in poi):
 *   gesture_enabled = 0     "b902d11f str"@0xffffff8008a72a6c
 *   gestures_flag azzerato  "a9017d3f stp"@0xffffff8008a72a70
 *                           "a9007d3f stp"@0xffffff8008a72a74
 *   gesture_doze_status = 0 "b906f15f str"@0xffffff8008a72a78
 *
 * Il messaggio, uno solo, comune ai due rami (il salto
 * "14000008 b"@0xffffff8008a72a5c porta esattamente li'):
 *   formato   "<<GTP-DBG>>[%s:%d]Gesture debug %s\n"@0xffffff800924a472
 *             di cui la coda, cioe' l'argomento della macro, sta a
 *             "Gesture debug %s\n"@0xffffff800924a484 (0x472 + 18, la
 *             lunghezza del prefisso "<<GTP-DBG>>[%s:%d]")
 *   __func__  "gt1x_gesture_debug"@0xffffff800924a496
 *   __LINE__  431, "528035e2 mov"@0xffffff8008a72aac  (vedi G1)
 *   il terzo argomento e' scelto con una `csel`, non con un ramo:
 *   "9a881123 csel"@0xffffff8008a72aa0 -> x3 = (w0 != 0) ? x9 : x8, con
 *   x9 = "on"@0xffffff8009108b48 e x8 = "off"@0xffffff8009193462.
 *   x0 e' il FORMATO e x1 e' il `__func__`: lo dicono le due stringhe
 *   stesse, quella con %s e %d e' il formato (classe B1).
 */
void gt1x_gesture_debug(int on)
{
	if (on) {
		gesture_enabled = 1;
		memset(gestures_flag, 0xff, sizeof(gestures_flag));
	} else {
		gesture_enabled = 0;
		memset(gestures_flag, 0x00, sizeof(gestures_flag));
		gesture_doze_status = 0;
	}
#line 431
	GTP_DEBUG("Gesture debug %s", on ? "on" : "off");
}

/*
 * ===========================================================================
 * La tabella a 0xffffff800998a468 -- sei righe da 40 byte
 * ===========================================================================
 * Il passo e' 40 e lo dice il binario due volte, in due funzioni diverse:
 *   "52800509 mov"@0xffffff8008a76cb0  (w9 = 0x28 = 40) in get_module_name
 *   "52800508 mov"@0xffffff8008a77990                     in tpd_i2c_probe
 * Il numero di righe e' SEI, ed e' un'INFERENZA, non una misura: nel codice
 * non c'e' nessun limite sull'indice (`get_module_name` non lo confronta con
 * niente) e nessuna dimensione dell'array e' leggibile dal `.text`. Cio' che
 * e' MISURATO e' che il contenuto occupa
 * 0xffffff800998a468..0xffffff800998a557 e che a 0xffffff800998a558 comincia
 * un altro oggetto; i primi byte di quell'oggetto sono `32 00 00 00`, e se
 * sia un `int` che vale 50 o il letterale "2" i byte da soli non lo dicono.
 * SEI righe e' la scelta che riempie esattamente lo spazio misurato.
 *
 *   [0] "xinpengda"@0xffffff800998a468
 *   [1] "none_1"@0xffffff800998a490
 *   [2] "hongzhan"@0xffffff800998a4b8
 *   [3] "none_3"@0xffffff800998a4e0
 *   [4] "none_4"@0xffffff800998a508
 *   [5] "none_5"@0xffffff800998a530
 * Gli indirizzi distano 40 byte l'uno dall'altro: e' il passo, riletto dai
 * byte invece che dedotto dalla moltiplicazione, ed e' la seconda prova.
 *
 * Il RUOLO e' provato dal chiamante, non indovinato: `tpd_i2c_probe` copia
 * la riga scelta e poi la stampa con
 * "TP IC:GT%s,TP MODULE:%s,TP I2C ADR:0x%x,SW FirmWare:0x%06X,Sample FirmWare:0x%06X"@0xffffff800924c27f.
 * E' il nome del modulo del pannello. La citazione e' PER INTERO: la prima
 * stesura la troncava a 39 caratteri su 81 mettendo i puntini di sospensione
 * DENTRO le virgolette, cioe' in una forma che nessuno strumento puo'
 * confrontare col binario (classe B7). Letta dai byte:
 *   $ ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf 0xffffff800924c27f
 *   0xffffff800924c27f: b'TP IC:GT%s,TP MODULE:%s,TP I2C ADR:0x%x,SW FirmWare:0x%06X,Sample FirmWare:0x%06X'  (len=81, ...)
 * Il NOME della variabile resta l'indirizzo: vedi G2.
 */
static char d998a468[6][40] = {
	"xinpengda", "none_1", "hongzhan", "none_3", "none_4", "none_5",
};

/*
 * ===========================================================================
 * get_module_name -- 0xffffff8008a76ca8, 76 byte, 19 istruzioni
 * ===========================================================================
 * Firma: due argomenti.
 *  - w0 e' l'INDICE, ed e' un `int` CON SEGNO. Lo dice la moltiplicazione:
 *    "9b292008 smaddl"@0xffffff8008a76cb4 e' `smaddl x8, w0, w9, x8`, cioe'
 *    w0 esteso CON SEGNO a 64 bit. La prova che la forma discrimina e' la
 *    controprova compilata: cambiando SOLO il tipo del parametro in
 *    `unsigned int`, lo stesso file col compilatore di fabbrica emette
 *    `9ba92008 umaddl` allo stesso spiazzamento -- e misura gli stessi 76
 *    byte, quindi il segno e' una divergenza che la DIMENSIONE NON DENUNCIA
 *    e che solo il confronto istruzione per istruzione trova:
 *      $ objdump -d orig.o | grep -E "maddl"
 *        90:	9b292008 	smaddl	x8, w0, w9, x8      (index: int)
 *      $ objdump -d uns.o  | grep -E "maddl"
 *        90:	9ba92008 	umaddl	x8, w0, w9, x8      (index: unsigned int)
 *      $ nm --print-size uns.o | grep -i " T get_module_name"
 *      0000000000000084 000000000000004c T get_module_name
 *    (`9ba92008 umaddl` NON sta nel binario di fabbrica: e' dichiarata
 *    `--controfattuale` nel comando del punto 1.)
 *
 *    NON e' una prova, invece, il sito dove questa funzione e' INCORPORATA
 *    dentro `tpd_i2c_probe`. La prima stesura lo chiamava «la seconda prova
 *    indipendente del segno»: la' c'e' "9b0824c1 madd"@0xffffff8008a77998 e
 *    non una `smaddl`, ma la ragione non e' il tipo -- e' che li' l'indice
 *    arriva da "3966c686 ldrb"@0xffffff8008a77988, quindi vale 0..255 ed e'
 *    gia' esteso a 64 bit: non c'e' nessuna estensione da emettere. Provato
 *    compilando le due forme fianco a fianco:
 *      static char tab[6][40];  extern unsigned char idx8;  extern void usa(char *);
 *      void con_int(void)      { int i = idx8;          usa(tab[i]); }
 *      void con_unsigned(void) { unsigned int i = idx8; usa(tab[i]); }
 *      0000000000000000 <con_int>:
 *         4:	39400108 	ldrb	w8, [x8]
 *        14:	9b092900 	madd	x0, x8, x9, x10
 *      000000000000001c <con_unsigned>:
 *        20:	39400108 	ldrb	w8, [x8]
 *        30:	9b092900 	madd	x0, x8, x9, x10
 *    Identiche. Il segno lo prova la sola `smaddl` di `get_module_name`,
 *    dove l'indice e' un parametro a 32 bit che va esteso.
 *  - x1 e' la DESTINAZIONE: e' l'argomento di `strcpy`
 *    ("aa0103e0 mov"@0xffffff8008a76cc4 sposta x1 in x0) ed e' la base dei
 *    due store del ramo alternativo.
 * Nessun valore di ritorno: "d65f03c0 ret"@0xffffff8008a76cd4 non passa da w0.
 *
 * Il TEST DI NULLITA' SUL PUNTATORE CALCOLATO -- "b4000108 cbz"@0xffffff8008a76cb8
 * -- e' nel binario e va riprodotto: `&d998a468[index]` non e' mai NULL, ma
 * la fabbrica lo controlla lo stesso e su questo esemplare il compilatore
 * non lo ha ripiegato. E' un controllo inutile della fabbrica, non un
 * difetto nostro, e si riproduce (classe B6, la stessa regola per cui una
 * `kfree` mancante si riproduce).
 *
 * LA STRINGA E' "unknow", SEI LETTERE, E NON E' UN ERRORE DI LETTURA.
 * Il ramo alternativo scrive con DUE store sovrapposti, che e' come clang
 * incorpora una `strcpy` di una costante di SETTE byte:
 *   "528dcea9 mov"@0xffffff8008a76cdc  w9 = 0x00006e75
 *   "72adcd69 movk"@0xffffff8008a76ce4 w9 = 0x6e6b6e75
 *   "b9000029 str"@0xffffff8008a76cec  [x1+0..3] = 75 6e 6b 6e = 'u','n','k','n'
 *   "528dedc8 mov"@0xffffff8008a76cd8  w8 = 0x00006f6e
 *   "72a00ee8 movk"@0xffffff8008a76ce0 w8 = 0x00776f6e
 *   "b8003028 stur"@0xffffff8008a76ce8 [x1+3..6] = 6e 6f 77 00 = 'n','o','w',NUL
 * Sovrapponendoli: [0]='u' [1]='n' [2]='k' [3]='n' [4]='o' [5]='w' [6]=NUL,
 * cioe' SETTE byte in tutto e la stringa "unknow". La 'n' finale non c'e'.
 *
 * E' un refuso della fabbrica, e si riproduce (regola 7).
 *
 * QUI LA PRIMA STESURA INSEGNAVA IL CONTRARIO DEL VERO, e va detto per
 * intero perche' era stato scritto come «il caso da manuale della regola 3».
 * Diceva: «con "unknown" la funzione misura comunque 76 byte, quindi LA
 * DIMENSIONE NON DENUNCIA IL REFUSO». Era vero solo con `clang-r383902`.
 * Col compilatore di fabbrica accade l'opposto esatto -- stesso file, unica
 * modifica `sed 's/"unknow"/"unknown"/'`, stessa riga di comando:
 *
 *   $ nm --print-size orig.o    | grep -i " T get_module_name"
 *   0000000000000084 000000000000004c T get_module_name     (0x4c = 76 = fabbrica)
 *   $ nm --print-size unknown.o | grep -i " T get_module_name"
 *   0000000000000084 0000000000000048 T get_module_name     (0x48 = 72)
 *
 * e il perche' si legge nelle istruzioni: SETTE byte non entrano in un
 * registro da 64 bit e costano due store sovrapposti (quattro istruzioni per
 * caricare le due meta' piu' due store, 19 istruzioni in tutto); OTTO byte
 * ci entrano e costano un `mov` + tre `movk` + un `str x8`, cioe' UNA
 * istruzione in meno:
 *
 *   "unknow"                          "unknown"
 *   b4:	528dedc8 	mov	w8, #0x6f6e     b4:	d28dcea8 	mov	x8, #0x6e75
 *   b8:	528dcea9 	mov	w9, #0x6e75     b8:	f2adcd68 	movk	x8, #0x6e6b, lsl #16
 *   bc:	72a00ee8 	movk	w8, #0x77, lsl #16   bc:	f2ceede8 	movk	x8, #0x776f, lsl #32
 *   c0:	72adcd69 	movk	w9, #0x6e6b, lsl #16 c0:	f2e00dc8 	movk	x8, #0x6e, lsl #48
 *   c4:	b8003028 	stur	w8, [x1,#3]     c4:	f9000028 	str	x8, [x1]
 *   c8:	b9000029 	str	w9, [x1]        c8:	d65f03c0 	ret
 *   cc:	d65f03c0 	ret
 *
 * Quindi SU QUESTA FUNZIONE LA DIMENSIONE DENUNCIA IL REFUSO: scrivere
 * "unknown" avrebbe fatto 72 contro 76 e il numero non sarebbe tornato. La
 * DECISIONE -- tenere "unknow" -- resta quella di prima ed era gia' giusta
 * per la ragione giusta (i byte del binario dicono sette byte in tutto); a
 * essere sbagliato era il ragionamento pubblicato accanto.
 *
 * Cosa resta vero della lezione, e va tenuto: che il confronto istruzione
 * per istruzione va fatto ANCHE quando lo scarto di dimensione e' zero. In
 * questa stessa funzione ce n'e' l'esempio due paragrafi sopra -- il segno
 * dell'indice, `smaddl` contro `umaddl`, 76 byte in entrambi i casi.
 */
void get_module_name(int index, char *name)
{
	if (d998a468[index])
		strcpy(name, d998a468[index]);
	else
		strcpy(name, "unknow");
}

/*
 * `gt1x_i2c_read` -- 0xffffff8008a76cf4, definita in `gt1x_tpd.c`, che questo
 * lotto non riscrive. Dichiarata e non definita (regola 6).
 * La FIRMA e' letta dal sito di chiamata dentro `dump_to_file`:
 *  - w0 e' l'indirizzo del registro: ci arriva "0b140320 add"@0xffffff8008a7afb0
 *    (w0 = w25 + w20, cioe' base + scorrimento) SENZA nessun mascheramento a
 *    16 bit, mentre lo stesso valore mascherato -- "12003c03 and"@0xffffff8008a7af08
 *    (`and w3, w0, #0xffff`) -- serve solo alla `printk`. Un argomento
 *    stretto che il chiamato tronca da se' non va troncato dal chiamante;
 *    uno promosso a `int` per la lista variadica di `printk` si': le due
 *    istruzioni insieme dicono che il parametro e' a 16 bit.
 *  - x1 e' il buffer: "910023e1 add"@0xffffff8008a7afb4 (sp + 8).
 *  - w2 e' la lunghezza a 32 bit: "2a1803e2 mov"@0xffffff8008a7afb8.
 *  - il ritorno e' provato zero/non-zero: "340000a0 cbz"@0xffffff8008a7afc0.
 */
extern s32 gt1x_i2c_read(u16 addr, u8 *buffer, s32 len);

/*
 * ===========================================================================
 * dump_to_file -- 0xffffff8008a7aed8, 360 byte, 90 istruzioni
 * ===========================================================================
 * E' una delle SETTE che ALPS non ha (RIASSUNTO.md §8): non esiste ne' in
 * `GT1151/` ne' in `GT5688/`, quindi qui non c'e' niente da misurare, c'e' da
 * scrivere. Simbolo `T` nella mappa, cioe' GLOBALE, non `static`:
 *   $ grep -n " dump_to_file" oracolo/stock.map
 *   ffffff8008a7aed8 T dump_to_file
 * NESSUNO LA CHIAMA, e il controllo e' su TUTTA l'immagine e non su una
 * finestra (classe C3): sulle 5.018.179 righe del disassemblato completo
 * l'indirizzo 0xffffff8008a7aed8 compare DUE volte, e sono l'etichetta e la
 * sua prima istruzione.
 *   $ objdump -d oracolo/stock.elf > /tmp/tutto.asm; wc -l /tmp/tutto.asm
 *   5018179 /tmp/tutto.asm
 *   $ grep -cE '\bbl\s+ffffff8008a7aed8' /tmp/tutto.asm
 *   0
 *   $ grep -nE 'ffffff8008a7aed8' /tmp/tutto.asm
 *   2669274:ffffff8008a7aed8 <dump_to_file>:
 *   2669275:ffffff8008a7aed8:	d10383ff 	sub	sp, sp, #0xe0
 *   $ grep -cE '\bbl\s+ffffff8008a76cf4' /tmp/tutto.asm    # controprova
 *   55                                                    # gt1x_i2c_read
 * E' emessa perche' e' globale, non perche' qualcuno la usi. Si riproduce
 * com'e' (regola 7).
 *
 * FIRMA, tre argomenti, e ognuno ha la sua prova:
 *  - w0, l'indirizzo di partenza. E' a 16 BIT: alla `printk` arriva
 *    mascherato, "12003c03 and"@0xffffff8008a7af08 (`and w3, w0, #0xffff`,
 *    la promozione a `int` di un `u16` per una lista variadica), mentre alla
 *    `gt1x_i2c_read` arriva NON mascherato, "0b140320 add"@0xffffff8008a7afb0.
 *    Con un parametro a 32 bit la `and` non ci sarebbe; con uno a 16 bit
 *    troncato dal chiamante ci sarebbe anche sull'altro ramo.
 *  - w1, la lunghezza. E' CON SEGNO: "540003eb b.lt"@0xffffff8008a7af84
 *    (`b.lt`, non `b.ls`) e "54fffccc b.gt"@0xffffff8008a7affc chiudono il
 *    ciclo con confronti con segno. Un `s32`, non un `u32`.
 *  - x2, il nome del file, passato intatto a `filp_open`
 *    ("aa1603e0 mov"@0xffffff8008a7af38) e alla `printk`
 *    ("aa1603e5 mov"@0xffffff8008a7af24).
 * Nessun valore di ritorno: "d65f03c0 ret"@0xffffff8008a7b038 arriva
 * dall'epilogo e w0 non viene impostato su nessuno dei due rami.
 *
 * IL MESSAGGIO D'INGRESSO. Il formato porta DUE `\n`:
 *   "<<GTP-INF>>[%s:%d] Dump(0x%04X, %d bytes) to file: %s\n\n"@0xffffff800924d885
 * uno e' quello che la macro concatena, l'altro sta nell'argomento della
 * macro, che e' quindi "Dump(0x%04X, %d bytes) to file: %s\n" -- con il suo
 * `\n` scritto a mano. La stessa cosa nel messaggio d'errore,
 *   "<<GTP-ERR>>[%s:%d] can not open file: %s\n\n"@0xffffff800924d8ca
 * e questa e' la prova che le due macro sono davvero GTP_INFO e GTP_ERROR e
 * non due `printk` scritte a mano: il `__func__` e il `__LINE__` che le
 * seguono ("f0003e81 adrp"@0xffffff8008a7af10 per x1, e
 * "5280a662 mov"@0xffffff8008a7af1c per w2 = 0x533 = 1331) sono gli stessi
 * due argomenti che la macro mette, nello stesso ordine.
 *   __func__ "dump_to_file"@0xffffff800924d8bd
 *   __LINE__ 1331 sul messaggio d'ingresso, 1334 sull'errore
 *            ("5280a6c2 mov"@0xffffff8008a7af58, 0x536 = 1334) -- vedi G1:
 *            questo file non riproduce i numeri di riga.
 *
 * L'APERTURA. "52800841 mov"@0xffffff8008a7af30 mette w1 = 0x42 = 66 =
 * O_CREAT|O_RDWR (0100|0002 in ottale), e "528036c2 mov"@0xffffff8008a7af34
 * mette w2 = 0x1b6 = 438 = 0666. Il controllo dell'errore e' la forma
 * incorporata di IS_ERR: "b13ffc1f cmn"@0xffffff8008a7af40 confronta con
 * -4095 e "54000123 b.cc"@0xffffff8008a7af44 salta al corpo quando NON e'
 * un errore.
 *
 * IL RIPOSIZIONAMENTO, attraverso il puntatore a funzione e non con
 * `vfs_llseek`: "f9401408 ldr"@0xffffff8008a7af68 legge `flp->f_op`
 * (spiazzamento 40 in `struct file`), "f9400508 ldr"@0xffffff8008a7af78
 * legge il campo a +8 di `struct file_operations` -- che dopo `owner` e'
 * `llseek` -- e "d63f0100 blr"@0xffffff8008a7af7c lo chiama con
 * "aa1f03e1 mov"@0xffffff8008a7af6c (x1 = 0) e
 * "2a1f03e2 mov"@0xffffff8008a7af70 (w2 = 0 = SEEK_SET).
 *
 * IL CICLO, e i tre numeri che lo fissano:
 *  - il blocco e' 128: "321903fa orr"@0xffffff8008a7af90 mette w26 = 0x80 e
 *    "1a9ab278 csel"@0xffffff8008a7af98 sceglie `(len < 128) ? len : 128`.
 *    Il buffer e' quindi 128 byte, e lo conferma la pila: il canarino sta a
 *    x29-72 ("f81b83a8 stur"@0xffffff8008a7af28) cioe' a sp+136, e il buffer
 *    comincia a sp+8 ("910023e0 add"@0xffffff8008a7afa0): 136-8 = 128.
 *  - il riempimento e' 0x33: "52800661 mov"@0xffffff8008a7afa4 (w1 = 0x33)
 *    prima di "940f7415 bl"@0xffffff8008a7afac verso `__memset`. Due volte:
 *    una prima della lettura e una DOPO, se la lettura fallisce
 *    ("52800661 mov"@0xffffff8008a7afc8, "940f740c bl"@0xffffff8008a7afd0).
 *    Riempire due volte lo stesso buffer con lo stesso byte e' inutile e la
 *    fabbrica lo fa: si riproduce (regola 7).
 *  - la scrittura passa da `flp->f_op->write` -- il campo a +24 di
 *    `struct file_operations`, "f9400d08 ldr"@0xffffff8008a7afe4 -- e il
 *    quarto argomento e' `&flp->f_pos`, calcolato UNA VOLTA fuori dal ciclo:
 *    "9101a2b6 add"@0xffffff8008a7af8c (x22 = x21 + 0x68) e poi
 *    "aa1603e3 mov"@0xffffff8008a7afe8 a ogni giro. 0x68 = 104 e' lo
 *    spiazzamento di `f_pos` in `struct file` di questo sublevel.
 * Il valore di ritorno della `write` NON viene guardato -- dopo il `blr` la
 * prima istruzione e' "4b170273 sub"@0xffffff8008a7aff0 -- e nemmeno quello
 * della `llseek`. Anche questo si riproduce.
 *
 * LA CHIUSURA: "97dfabd6 bl"@0xffffff8008a7b008 verso `filp_close`, con
 * "aa1f03e1 mov"@0xffffff8008a7b004 (x1 = NULL). Sul ramo in cui l'apertura
 * fallisce NON si chiude niente e si salta direttamente al controllo del
 * canarino: "1400002a b"@0xffffff8008a7af64.
 */
void dump_to_file(u16 addr, s32 len, char *filename)
{
	struct file *flp = NULL;
	u8 buf[128];
	s32 ret = 0;
	s32 cnt = 0;
	s32 pos = 0;

#line 1331
	GTP_INFO("Dump(0x%04X, %d bytes) to file: %s\n", addr, len, filename);

	flp = filp_open(filename, O_CREAT | O_RDWR, 0666);
	if (IS_ERR(flp)) {
#line 1334
		GTP_ERROR("can not open file: %s\n", filename);
		return;
	}

	flp->f_op->llseek(flp, 0, SEEK_SET);

	while (len > 0) {
		cnt = (len < 128) ? len : 128;
		memset(buf, 0x33, cnt);
		ret = gt1x_i2c_read(addr + pos, buf, cnt);
		if (ret)
			memset(buf, 0x33, cnt);
		flp->f_op->write(flp, buf, cnt, &flp->f_pos);
		len -= cnt;
		pos += cnt;
	}

	filp_close(flp, NULL);
}

/*
 * ===========================================================================
 * tpd_driver_exit -- 0xffffff80093acaf8, 52 byte, 13 istruzioni, .exit.text
 * ===========================================================================
 * NON e' nella mappa: vedi il cappello. Le 13 istruzioni per intero, perche'
 * e' la prima volta che qualcuno le scrive:
 *
 *   "a9bf7bfd stp"@0xffffff80093acaf8   prologo
 *   "910003fd mov"@0xffffff80093acafc
 *   "90fff500 adrp"@0xffffff80093acb00  x0 <- pagina 0xffffff800924c000
 *   "90fff501 adrp"@0xffffff80093acb04  x1 <- stessa pagina
 *   "91017000 add"@0xffffff80093acb08   x0 = +0x5c  -> il FORMATO
 *   "91025c21 add"@0xffffff80093acb0c   x1 = +0x97  -> il `__func__`
 *   "52808362 mov"@0xffffff80093acb10   w2 = 0x41b = 1051 = __LINE__ (G1)
 *   "97b61a70 bl"@0xffffff80093acb14    printk
 *   "d0002ee0 adrp"@0xffffff80093acb18  x0 <- pagina 0xffffff800998a000
 *   "9115e000 add"@0xffffff80093acb1c   x0 = +0x578 -> gt1x_tpd_driver
 *   "97da9262 bl"@0xffffff80093acb20    tpd_driver_remove
 *   "a8c17bfd ldp"@0xffffff80093acb24   epilogo
 *   "d65f03c0 ret"@0xffffff80093acb28
 *
 * Le due stringhe, lette dai byte:
 *   formato  "<<GTP-INF>>[%s:%d] MediaTek GT1x touch panel driver exit.\n"@0xffffff800924c05c
 *            di cui la coda e' "MediaTek GT1x touch panel driver exit.\n"@0xffffff800924c06f
 *   __func__ "tpd_driver_exit"@0xffffff800924c097
 * ed e' il `__func__` a dare il NOME della funzione, che la mappa non da':
 * e' l'eccezione dichiarata alla regola dei nomi -- il binario la nomina.
 * Subito dopo, a 0xffffff800924c0a7, c'e' "gt1x", il `.name` della
 * `tpd_driver_t` e dell'`i2c_driver`.
 *
 * L'ORDINE conta e il binario lo fissa: il messaggio PRIMA della
 * de-registrazione. Il contrario darebbe le stesse due `bl` invertite, e
 * la stessa dimensione: e' una divergenza che la misura NON denuncia, ed e'
 * per questo che sta scritta qui.
 */
void __exit tpd_driver_exit(void)
{
	GTP_INFO("MediaTek GT1x touch panel driver exit.");
	tpd_driver_remove(&gt1x_tpd_driver);
}

/*
 * ===========================================================================
 * QUALE DELLE DUE VARIANTI ALPS -- MISURATO, NON PIU' SOLO DEDOTTO
 * ===========================================================================
 * Il rapporto del 2026-08-15 aveva stabilito che la variante da cui partire
 * e' GT5688 e non GT1151, contando i LETTERALI (4 a 0). La ricognizione
 * `scout-gt917s/RIASSUNTO.md` §8 aggiungeva che il confronto PER NOME non
 * discrimina: 79 funzioni definite in GT1151 e 79 in GT5688, zero esclusive
 * da una parte o dall'altra. La DIMENSIONE non era mai stata provata.
 * Provata qui, accendendo anche CONFIG_TOUCHSCREEN_MTK_GT1151 e applicando a
 * `GT1151/include/gt1x_tpd_common.h` la STESSA D1 gia' applicata a GT5688
 * (senza, GT1151 fa 20/80 e il confronto misurerebbe D1, non la variante):
 *
 *   variante          identiche/80   funzioni identiche che l'altra non ha
 *   GT5688 + D1 + D5       34        6
 *   GT1151 + D1 + D5       28        0
 *
 * Le 28 di GT1151 sono un SOTTOINSIEME PROPRIO delle 34 di GT5688 -- 28
 * comuni, 6 solo GT5688, 0 solo GT1151 -- e le sei stanno tutte in
 * `gt1x_update.c`, il file dell'aggiornamento firmware:
 *   gt1x_hold_ss51_dsp_no_reset  gt1x_i2c_write_with_readback
 *   gt1x_load_patch  gt1x_read_flash  gt1x_recall_check  gt1x_run_ss51_isp
 * Nessuna funzione e' piu' vicina alla fabbrica in GT1151 che in GT5688:
 * per la dimensione GT5688 DOMINA, e la conclusione dei letterali e' adesso
 * confermata da una misura indipendente e di verso opposto (i letterali
 * guardano le stringhe, questa guarda il codice generato).
 *
 * ===========================================================================
 * COSA QUESTO FILE NON CONTIENE, E PERCHE'
 * ===========================================================================
 * Il blocco e' 87 funzioni e 38.000 byte. Qui ce ne sono QUATTRO, per 620
 * byte: l'1,6% dei byte del blocco. Le altre 83 non sono state scritte, e la
 * ragione non e' la stessa per tutte:
 *
 *  - 79 ESISTONO GIA' in ALPS (GT1151/ e GT5688/) e questo lotto le ha
 *    MISURATE invece di riscriverle. E' il mandato, ed e' la misura che non
 *    era mai stata fatta:
 *      $ ./venv/bin/python3 intervallo.py 34 80
 *      34 su 80 = 42.5%   IC95% Clopper-Pearson [31.5%; 54.1%]
 *      contiene 77.10%: NO -- la misura discrimina
 *    cioe' 46 funzioni su 80 divergono, il 57,5%. Sul blocco intero, con le
 *    quattro scritte qui contate insieme alle 79 di ALPS:
 *      $ ./venv/bin/python3 intervallo.py 37 87
 *      37 su 87 = 42.5%   IC95% Clopper-Pearson [32.0%; 53.6%]
 *      contiene 77.10%: NO -- la misura discrimina
 *    (le 87 comprendono le 4 che nessuna delle due parti produce: se si
 *    conta solo cio' che produce un simbolo, 37 su 83 = 44,6%, IC95%
 *    [33,7%; 55,9%], e la conclusione non cambia.)
 *    LA PRIMA STESURA PUBBLICAVA 25 SU 80 con l'intervallo «[16,0%; 35,9%]»,
 *    e sbagliava due volte: il numeratore era quello del compilatore
 *    sbagliato (vedi «IL METRO»), e quell'intervallo non era nemmeno di
 *    25/80 -- e' quello di 20/80, rimasto dal giro precedente con il solo
 *    numeratore aggiornato:
 *      $ ./venv/bin/python3 intervallo.py 25 80
 *      25 su 80 = 31.2%   IC95% Clopper-Pearson [21.3%; 42.6%]
 *      $ ./venv/bin/python3 intervallo.py 20 80
 *      20 su 80 = 25.0%   IC95% Clopper-Pearson [16.0%; 35.9%]
 *    Il VERSO della conclusione non cambia: anche 34/80 esclude il 77,10%
 *    del ramo, quindi l'ipotesi «79 esistono gia', il blocco costa poco»
 *    resta falsa. Cambia il prezzo: il 57,5% di riscrittura, non il 69%.
 *    La tabella completa e l'attribuzione di ogni divergenza stanno nel
 *    rapporto del lotto.
 *  - 4 delle 7 assenti da ALPS NON sono scritte: `gt1x_parse_config` (920
 *    byte), `gt1x_suspend` (732), `gt1x_resume` (816), `gt1x_error_erase`
 *    (1172). Sono 3.640 byte, e ognuna dipende da qualcosa che questo lotto
 *    non ha letto: `gt1x_parse_config` dall'array `gt1x_config` in .bss, che
 *    nell'immagine non c'e' (RIASSUNTO.md §10); `gt1x_suspend` e
 *    `gt1x_resume` dalle quattro INCORPORATE che contengono
 *    (`gt1x_enter_sleep`, `gt1x_wakeup_sleep`, `gt1x_i2c_test`,
 *    `gesture_enter_doze`); `gt1x_error_erase` dalla macchina a stati
 *    dell'aggiornamento firmware. La settima, `dump_to_file`, e' scritta
 *    qui dalla terza stesura, e misura 360 contro 360.
 *  - 11 funzioni INCORPORATE (RIASSUNTO.md §5) non hanno ne' simbolo ne'
 *    dimensione: non si possono misurare una per una, solo insieme al
 *    chiamante.
 *  - 1 dipendenza fuori dal blocco, `fix_tp_proc_info` a 0xffffff8008a5151c,
 *    non esiste in ALPS e appartiene a un gruppo che nessuno ha aperto.
 *    Finche' non e' scritta, `tpd_i2c_probe` diverge di sicuro.
 *
 * Un lotto parziale dichiarato vale piu' di uno completo inventato.
 *
 * ===========================================================================
 * E COSA IL BLOCCO CONTIENE DI TROPPO: I 15 SIMBOLI SENZA OMOLOGO
 * ===========================================================================
 * Compilando ALPS escono 15 simboli di codice che nella mappa di fabbrica
 * NON esistono, per 3.380 byte (misurati dal `.o` col compilatore di
 * fabbrica). La prima stesura scriveva, nel rapporto, che «quei 17 NON sono
 * codice in piu' di ALPS»: l'affermazione copriva 17 simboli e la prova ne
 * copriva 5. Rifatta, la ripartizione e' questa e ogni voce ha la sua prova.
 * ERANO 17 e 4.728 byte finche' `CONFIG_HOTKNOT_BLOCK_RW` e' rimasta accesa:
 * il binario dice che di fabbrica e' SPENTA e spegnendola i due simboli che
 * dipendono da lei -- `hotknot_block_rw` 592 e `hotknot_event_handler` 756,
 * 1.348 byte -- spariscono. Vedi D12.
 *
 *  CINQUE sono INCORPORATE nella fabbrica, provate da RIASSUNTO.md §5 (il
 *  loro `__func__` compare nel binario di fabbrica dentro il chiamante):
 *    gt1x_enter_sleep 716, gt1x_wakeup_sleep 464, gt1x_i2c_test 368,
 *    gt1x_init_debug_node 136, tpd_irq_registration 484  = 2.168 byte
 *      $ for s in ...; do strings oracolo/stock.elf | grep -cx "$s"; done
 *      gt1x_enter_sleep 1 / gt1x_wakeup_sleep 1 / gt1x_i2c_test 1 /
 *      gt1x_init_debug_node 1 / tpd_irq_registration 1
 *
 *  UNO e' un artefatto della mappa e non una funzione:
 *    __initcall_tpd_driver_init6, 8 byte, che nel `.o` sta in
 *    `.initcall6.init` come OGGETTO (`l O .initcall6.init`).
 *    `oracolo/stock.map` contiene solo simboli di codice -- 30.100 T,
 *    23.372 t, 237 W, 1 A, e zero `__initcall` -- quindi non poteva averlo.
 *
 *  UNO ha il corpo NELLA FABBRICA, sotto altra forma, e la prima stesura
 *  aveva ragione su questo:
 *    gt1x_power_reset2, 136 byte -- il suo letterale c'e':
 *      $ strings oracolo/stock.elf | grep -cF 'force_reset_guitar'
 *      1
 *
 *  QUATTRO sono CODICE IN PIU' DI ALPS, provato: il letterale che stampano
 *  non compare DA NESSUNA PARTE nell'immagine di fabbrica, quindi quel corpo
 *  nel binario di fabbrica non c'e' -- ne' separato ne' incorporato.
 *    tpd_enter_tui 60          'enter tui'                            0
 *    tpd_exit_tui 248          'exit TUI+'                            0
 *    gt1x_pm_notifier 272      'PM_RESTORE_PREPARE enter'             0
 *    read_reg 304              'Read address: 0x%04X, Length: %d'     0
 *                                                            = 884 byte
 *  ALTRI DUE stavano in questa lista e adesso non ci sono piu', perche' la
 *  loro assenza dall'immagine non era «codice in piu' di ALPS» ma una
 *  CONFIG spenta di fabbrica, e spegnendola spariscono dal `.o`:
 *    hotknot_block_rw 592      'Goodix tool received wait polling state'  0
 *    hotknot_event_handler 756 'HotKnot paired!'                      0
 *  Vedi D12: e' la differenza fra «ALPS ha del codice che la fabbrica non
 *  ha» e «noi lo stavamo compilando e la fabbrica no», e solo la seconda si
 *  chiude cambiando una riga di `.config`.
 *    (controprova nella stessa scansione: 'Goodix touch panel driver init.'
 *    da' 1, 'MediaTek GT1x' da' 1.)
 *    `tpd_enter_tui`, `tpd_exit_tui` e `tpd_reregister_from_tui` sono per
 *    giunta funzioni GLOBALI senza nessun `#ifdef` attorno
 *    (`GT5688/gt1x_tpd.c:1248, 1257, 583`): il compilatore DEVE emetterle e
 *    nessun chiamante interno puo' assorbirle.
 *
 *  QUATTRO restano APERTI, 184 byte, e non si chiudono cosi' perche' non
 *  stampano niente: gt1x_auto_update_done 52, gt1x_deinit_debug_node 44,
 *  gt1x_is_tpd_halt 12, tpd_reregister_from_tui 76.
 *
 *  2.168 + 8 + 136 + 884 + 184 = 3.380, e i 3.380 sono misurati:
 *      $ ./venv/bin/python3 corr-gt917s/extra.py | tail -1
 *      totale=3380 byte su 15 simboli
 *
 * ===========================================================================
 * DELTA DI HEADER -- da fare al MERGE, non qui
 * ===========================================================================
 * D1. `GT5688/include/gt1x_tpd_common.h`, righe 207-216. Le tre macro di
 *     log vanno portate alla forma che il binario prova:
 *         GTP_INFO/GTP_ERROR:  pr_info(...)   ->  printk(...)   [via il livello]
 *         GTP_DEBUG:  do { if (tpd_em_log) pr_debug(...); } while (0)
 *                     ->  printk("<<GTP-DBG>>[%s:%d]" fmt "\n", ...)
 *     MISURATA COL COMPILATORE DI FABBRICA: porta le funzioni identiche da
 *     25 a 34 su 80, cioe' +9. (La prima stesura diceva +5, da 20 a 25: era
 *     il guadagno misurato col compilatore sbagliato, e SOTTOSTIMAVA la
 *     propria scoperta migliore.) Le nove sono, scarto prima -> dopo:
 *       gesture_enter_doze          +72 ->  0
 *       comfirm                    +100 ->  0
 *       __gt1x_hold_ss51_dsp_20    +100 ->  0
 *       gt1x_recall_check          +112 ->  0
 *       gt1x_gesture_data_write     +36 ->  0
 *       gt1x_read_version           +36 ->  0
 *       gt1x_request_event_handler  +36 ->  0
 *       hotknot_open                +36 ->  0
 *       hotknot_release             +36 ->  0
 *     E' UNA MODIFICA APPLICATA nel mio albero (`albero-gt917s`), non nel
 *     ramo principale, e la misura 34/80 e' fatta con lei applicata: senza
 *     di lei sarebbe 25/80. Il `.orig-alps` sta accanto al file.
 *     LA TERZA STESURA L'HA APPLICATA ANCHE A
 *     `GT1151/include/gt1x_tpd_common.h`, righe 204-213, dove il testo e'
 *     lo stesso a meno della spaziatura, e per la stessa ragione: senza,
 *     il confronto fra le due varianti ALPS avrebbe misurato D1 e non la
 *     variante (GT1151 fa 20/80 senza e 28/80 con). Anche li' resta un
 *     `.orig-alps` accanto al file, e anche li' e' solo nel mio albero.
 * D2. `GT5688/gt1x_extents.c:57`. `struct { ... } st_gesture_data;` manca la
 *     parola `typedef`, e la riga 73 usa `st_gesture_data` come TIPO. Con
 *     CONFIG_GTP_GESTURE_WAKEUP=y il file NON COMPILA. Lo stesso difetto e'
 *     in `GT1151/gt1x_extents.c:57`. E' la prova che nessuno ha mai
 *     compilato quei due driver con il gesture acceso.
 * D3. `GT5688/include/gt1x_tpd_common.h:83-86`. `GTP_INT_TRIGGER` e
 *     `GTP_WAKEUP_LEVEL` sono definite solo sotto CONFIG_GTP_CUSTOM_CFG, ma
 *     `gt1x_generic.c:1767-1768` le usa SENZA guardia. Con CUSTOM_CFG spenta
 *     il file non compila. Il binario forza la combinazione
 *     CUSTOM_CFG=y + DRIVER_SEND_CFG=n, che e' esattamente quella che
 *     scopre il difetto.
 * D4. `tpd_driver_exit` (questo file) e `gt1x_tpd_driver` si sovrappongono a
 *     `GT5688/gt1x_tpd.c`. Il merge deve scegliere UNA delle due
 *     definizioni. Vedi G4.
 * D5. `drivers/input/touchscreen/mediatek/tpd.h:35` definisce
 *     `TPD_DEBUG_CODE` INCONDIZIONATAMENTE, e con lui `TPD_EM_PRINT`,
 *     `TPD_DEBUG_SET_TIME`, `TPD_DEBUG_PRINT_INT`, che chiamano
 *     `tpd_em_log_output`, `tpd_em_log_store`, `tpd_em_log_release`,
 *     `MET_touch`, `tpd_debug_set_time`. Nel kernel di FABBRICA quei
 *     cinque simboli esistono e NESSUNO LI CHIAMA: zero `bl` su
 *     5.018.179 righe di disassemblato dell'intera immagine (il controllo
 *     e' sano: nella stessa scansione i `bl printk` sono 24.886). Quindi
 *     di fabbrica `TPD_DEBUG_CODE` e' SPENTA. Misurato nel mio albero:
 *     spegnendola, `gt1x_touch_up` passa da +132 a -32 e
 *     `gt1x_touch_down` da +344 a +204.
 *     ATTENZIONE, e' un header CONDIVISO con l'ilitek e il focaltech: la
 *     modifica non e' locale al Goodix. E ALPS non compila con quella
 *     macro spenta, perche' i ripieghi vuoti (`#ifndef TPD_EM_PRINT`)
 *     stanno DENTRO `tpd_debug.h`, che con TPD_DEBUG_CODE spenta non viene
 *     nemmeno incluso: servono cinque righe in piu' in `tpd.h`.
 *     ANCHE QUESTA E' APPLICATA nel mio albero, e va detto perche' la prima
 *     stesura la annunciava nel rapporto come «NON L'HO LASCIATO APPLICATO»
 *     mentre lo era: chi leggeva concludeva che il 34/80 fosse misurato su
 *     ALPS con TPD_DEBUG_CODE accesa, e non lo e'.
 *     QUANTO VALE, misurato col compilatore di fabbrica: ZERO sul
 *     numeratore. Sposta tre funzioni e nessuna delle tre arriva a scarto 0:
 *       gt1x_touch_down            +344 -> +204
 *       gt1x_touch_up              +140 ->  -32
 *       tpd_eint_interrupt_handler  +56 ->  +32
 *     cioe' 34/80 con D5 e 34/80 senza. E' la conferma che il numero-bandiera
 *     non dipende da questo delta -- ma la misura resta fatta su un ALPS
 *     modificato in due punti, e i due punti sono D1 e D5.
 * D6. `GT5688/gt1x_generic.c:489`, `gt1x_sensor_id_check` e' una `static`
 *     il cui unico chiamante sta dentro `#ifdef CONFIG_GTP_DRIVER_SEND_CFG`.
 *     Con DRIVER_SEND_CFG SPENTA -- che e' la configurazione che il binario
 *     IMPONE, perche' `gt1x_send_cfg` di fabbrica e' `mov w0,wzr; ret` in
 *     otto byte -- la funzione resta inutilizzata e con `-Werror` il file
 *     NON COMPILA:
 *       error: unused function 'gt1x_sensor_id_check' [-Werror,-Wunused-function]
 *     Nel build per sottodirectory il `subdir-ccflags-y += -Werror` del
 *     Makefile padre NON arriva (kbuild non attraversa il padre), quindi il
 *     difetto si vede solo forzando `KCFLAGS=-Werror` o compilando tutto
 *     `drivers/input/touchscreen/mediatek/`. In un build completo del
 *     kernel accendere CONFIG_TOUCHSCREEN_MTK_GT5688 lo farebbe FALLIRE.
 * D7. LA DECISIONE PIU' GRANDE DI QUESTO LOTTO, E NON E' UNA MISURA:
 *     `CONFIG_TOUCHSCREEN_MTK_GT5688=y` e `CONFIG_TOUCHSCREEN_MTK_GT1151=y`.
 *     Nel config di FABBRICA sono ENTRAMBE SPENTE e al loro posto c'e'
 *     `CONFIG_WTK_GT917S_E977=y`:
 *       $ grep -nE "GT1151|GT5688|GT917S" \
 *             /mnt/s88pro/kernel-work/e977_dg_m13_71_q0.config
 *       2210:# CONFIG_TOUCHSCREEN_MTK_GT1151 is not set
 *       2216:# CONFIG_TOUCHSCREEN_MTK_GT5688 is not set
 *       2220:CONFIG_WTK_GT917S_E977=y
 *     Le ho accese SOLO NEL MIO ALBERO (`albero-gt917s`), e solo perche'
 *     senza di loro il codice ALPS non si compila e quindi non si misura:
 *     e' lo STRUMENTO della misura, non il suo risultato. Accenderle nel ramo
 *     principale sarebbe una cosa diversa e piu' grande -- vorrebbe dire
 *     compilare il Goodix DA UNA DIRECTORY CHE LA FABBRICA NON USA, con un
 *     altro `KBUILD_MODNAME` e altri `__FILE__`, e vorrebbe dire far entrare
 *     nel kernel i 15 simboli senza omologo elencati sopra. NON E' PRESA da
 *     questo lotto, e nemmeno raccomandata: e' una scelta di perimetro che
 *     spetta al merge. Con `-Werror` acceso, per giunta, oggi non
 *     compilerebbe (vedi D6).
 *     Cio' che il ramo principale puo' prendere da qui senza decidere niente
 *     e' la MISURA: quanto di quel codice, cosi' com'e', combacia con la
 *     fabbrica. La risposta e' 34 su 80 (GT5688) e 28 su 80 (GT1151).
 * D12. `CONFIG_HOTKNOT_BLOCK_RW` DEVE ESSERE SPENTA, e non e' una scelta:
 *      lo decide il binario. Le due funzioni che quella CONFIG accende
 *      stampano letterali che nell'INTERA immagine di fabbrica non ci sono:
 *        $ strings oracolo/stock.elf | grep -cF 'Goodix tool received wait polling state'
 *        0
 *        $ strings oracolo/stock.elf | grep -cF 'HotKnot paired!'
 *        0
 *      (controprova nella stessa scansione, due letterali dello stesso
 *      driver che DEVONO esserci: 'Goodix touch panel driver init.' -> 1,
 *      'MediaTek GT1x' -> 1.) Il `.config` che questo albero eredita dalle
 *      preimpostazioni ALPS la teneva ACCESA. Spegnendola, misurato:
 *        i simboli senza omologo passano da 17 a 15 e da 4.728 a 3.380 byte;
 *        `gt1x_ioctl` passa da +1144 a +936 di scarto;
 *        `tpd_event_handler` passa da -316 a -412;
 *        le funzioni identiche restano 34 su 80.
 *      Il numero-bandiera NON si muove -- va detto, perche' e' la ragione
 *      per cui questa leva era passata inosservata: si vede sul CONTORNO
 *      (quanto codice in piu' compileremmo) e non sul numeratore.
 *      Le altre `CONFIG_GTP_*` del nostro `.config` concordano gia' con cio'
 *      che il binario impone (RIASSUNTO.md §6.7): DRIVER_SEND_CFG spenta,
 *      GESTURE_WAKEUP/HOTKNOT/AUTO_UPDATE/CREATE_WR_NODE accese,
 *      ESD_PROTECT/PROXIMITY/WITH_STYLUS spente.
 * D8. IL CONFLITTO CHE BLOCCA IL MERGE: `gestures_flag`. Questo file la
 *     DICHIARA (`extern u8 gestures_flag[32];`), `GT5688/gt1x_extents.c:72`
 *     la DEFINISCE `static u8 gestures_flag[32];`. Cosi' com'e' il link
 *     vero si ferma con `undefined reference`, ed e' voluto: e' l'unico modo
 *     perche' il conflitto sia RUMOROSO. Il merge deve scegliere:
 *       (a) togliere lo `static` in `gt1x_extents.c` -- e allora il binario
 *           ha UN array, come la fabbrica (vedi i cinque `#0x87d` citati sul
 *           commento della variabile); oppure
 *       (b) tenere lo `static` e spostare `gt1x_gesture_debug` DENTRO
 *           `gt1x_extents.c`, che e' dove di fabbrica sta.
 *     Cosa NON si deve fare: definirla non-`static` anche qui. Il link
 *     riesce, non c'e' nessun avviso, e restano DUE array da 32 byte --
 *     `gt1x_gesture_debug` scrive il proprio e `gesture_event_handler` /
 *     `gt1x_ioctl` leggono l'altro. La prima stesura faceva esattamente
 *     questo. La prova sta sul commento della variabile.
 *     D4 elenca la sovrapposizione RUMOROSA (`tpd_driver_exit`); questa e'
 *     quella silenziosa, ed e' l'unica delle due che si rompe da sola.
 * D9. NON un header: `tools/classificasimboli.py`. Il prefisso `gt1x` c'e'
 *     gia' (riga 72, dal commit del 2026-08-15) e conta 62 simboli, ma il
 *     blocco ne ha 86 nella mappa piu' la `.exit` fuori mappa. I 24 che i
 *     quattro prefissi (`gt1x`,`gt917`,`goodix`,`gtp_`) NON prendono sono:
 *     gesture_enter_doze gesture_event_handler gesture_clear_wakeup_data
 *     hotknot_open hotknot_release _do_i2c_read _do_i2c_write comfirm
 *     get_module_name tpd_off tpd_on tpd_local_init tpd_suspend tpd_resume
 *     tpd_i2c_probe tpd_i2c_remove tpd_i2c_detect tpd_registration
 *     tpd_event_handler tpd_eint_interrupt_handler getUint
 *     __gt1x_hold_ss51_dsp_20 dump_to_file tpd_driver_init.
 *     I nomi `tpd_*` sono in comune con l'ilitek e il focaltech: aggiungerli
 *     al prefisso del Goodix darebbe falsi positivi. E' una decisione, non
 *     una misura, e questo lotto NON la prende.
 *
 * D11. NON un header: `confinecitazioni.py` / `verificaistruzioni.py`.
 *      Le colonne di `nm --print-size` -- `<16 cifre esadecimali> <lettera di
 *      tipo> <nome>` -- passano per una CITAZIONE DI ISTRUZIONE ad indirizzo
 *      quando la lettera di tipo e' `b` o `B`: `b` e' un mnemonico di salto
 *      vero, e per i salti `descrive_istruzione` ammette un nome di simbolo
 *      come operando (confinecitazioni.py, RE_SIMBOLO). Incollare l'output di
 *      `nm --print-size` in un commento -- che e' il modo normale di provare
 *      una dimensione in questo progetto -- fabbrica quindi citazioni
 *      FANTASMA che `verificaistruzioni.py` segnala ASSENTE e che non si
 *      possono dichiarare `--controfattuale`, perche' la chiave estratta
 *      (`0x20`, `0x4`) non ha ne' 8 ne' 9-16 cifre e lo strumento la rifiuta
 *      come malformata:
 *        $ ./venv/bin/python3 verificaistruzioni.py ... --controfattuale "0x20 b"
 *        CONTROFATTUALE MALFORMATA: --controfattuale: lunghezza esadecimale
 *        inattesa (8 cifre per un encoding, 9-16 per un indirizzo): '0x20'
 *      Misurato qui: tre righe di `nm` incollate davano tre ASSENTE
 *      (`0x20 b`, `0x20 b`, `0x4 b`) e nessun modo di dichiararle. Il
 *      ripiego adottato in QUESTO file e' usare `readelf -sW`, il cui
 *      formato non combacia (dopo le 16 cifre viene un numero, non un
 *      mnemonico) -- ma e' un ripiego, non la correzione. La correzione sta
 *      nello strumento -- p.es. rifiutare la forma ad indirizzo quando la
 *      riga ha due campi esadecimali a 16 cifre in fila, che e' la firma di
 *      `nm --print-size` e non di `objdump -d` -- e questo lotto NON la
 *      prende, per la stessa ragione di D10: allarga o restringe CHI PASSA
 *      su tutti i driver, non solo su questo.
 *
 * ===========================================================================
 * LE DIECI CITAZIONI NON ANCORATE, E PERCHE' verificacitazioni.py ESCE 1
 * ===========================================================================
 * Lo strumento, su questo file, ESCE 1. Non e' un dettaglio da omettere: la
 * prima stesura concludeva «Soglia rispettata: 19 citati >= 14 richiesti» e
 * non diceva che lo strumento fallisce. La soglia E' rispettata; l'esito e'
 * 1 lo stesso, perche' `decidi_esito` fallisce gia' su `tutte_verificate`:
 *
 *   $ ./venv/bin/python3 verificacitazioni.py gt1x_wtk.c oracolo/stock.elf \
 *       >/dev/null 2>&1; echo "ESITO=$?"
 *   ESITO=1
 *   $ ./venv/bin/python3 verificacitazioni.py gt1x_wtk.c oracolo/stock.elf | tail -5
 *   letterali: 17   citati: 22   verificati: 12   probanti: 6   deboli: 6
 *   di cui verificate come messaggio assemblato dalla macro di log (coda + binario, non un singolo letterale C): 1
 *   di cui verificate come nome di funzione da __func__ (non un letterale scritto a mano): 3
 *   rinviate a verificaistruzioni.py (descrivono un'istruzione, non un letterale): 98 -- non contate qui, ne' come verificate ne' come mancanti
 *   soglia imposta: 17 citati richiesti (17 letterali - 0 eccezioni)
 *
 * Le NON_ANCORATE sono DIECI (erano otto prima che la terza stesura
 * aggiungesse `dump_to_file`, che ne porta due), e la prima stesura le
 * spiegava tutte con «sono citazioni di stringhe che stanno in funzioni che
 * questo file NON scrive». Vale per quattro. Le altre SEI sono i messaggi
 * delle TRE FUNZIONI CHE QUESTO FILE SCRIVE, e non sono ancorate per un
 * limite dello strumento.
 *
 *   (a) QUATTRO stanno in funzioni che questo file non scrive, e per
 *       costruzione non possono essere ancorate a un letterale di qui:
 *         0xffffff800924a225  <<GTP-DBG>>[%s:%d]Entering doze mode...\n
 *                             (gesture_enter_doze, la prova della forma di
 *                             GTP_DEBUG)
 *         0xffffff800924c27f  TP IC:GT%s,... (tpd_i2c_probe, il RUOLO della
 *                             tabella d998a468)
 *         0xffffff800924f634  [FTS]%s: Enter\n (la `.exit` del focaltech)
 *         0xffffff800923f7ed  ilitek_plat_dev_exit (il `__func__` che prova
 *                             la correzione a RIASSUNTO §6.1)
 *
 *   (b) SEI sono i messaggi di `gt1x_gesture_debug`, `dump_to_file` e
 *       `tpd_driver_exit`, che questo file scrive:
 *         0xffffff800924a472  <<GTP-DBG>>[%s:%d]Gesture debug %s\n
 *         0xffffff800924a484  Gesture debug %s\n
 *         0xffffff800924c05c  <<GTP-INF>>[%s:%d] MediaTek GT1x touch panel driver exit.\n
 *         0xffffff800924c06f  MediaTek GT1x touch panel driver exit.\n
 *         0xffffff800924d885  <<GTP-INF>>[%s:%d] Dump(0x%04X, %d bytes) to file: %s\n\n
 *         0xffffff800924d8ca  <<GTP-ERR>>[%s:%d] can not open file: %s\n\n
 *       Per le due di `dump_to_file` c'e' una ragione IN PIU', e va detta
 *       perche' non si provi a «sistemarle»: il loro testo di fabbrica
 *       finisce con DUE `\n` -- uno scritto a mano nell'argomento della
 *       macro, uno concatenato dalla macro -- e la coda che sarebbe il
 *       letterale del codice, "...%s\n", NON e' un suffisso dell'intero
 *       messaggio a un `\n` di distanza dalla fine. Citarla al proprio
 *       indirizzo la farebbe DIVERGENTE invece che non ancorata, che e'
 *       peggio: i byte che seguono nel binario sono `\n\0` e non `\0`.
 *       Lo strumento HA il dominio giusto per questi -- «messaggio assemblato
 *       dalla macro di log», quello che verifica la CODA contro un letterale
 *       del codice -- ma lo riconosce solo dalla testa KERN:
 *         verificacitazioni.py:262
 *         RE_TESTA_ASSEMBLATA = re.compile(r'\A\\x01[0-7]')
 *       e la macro di FABBRICA di questo driver NON ha il livello KERN: e'
 *       `printk("<<GTP-INF>>...")` nudo, ed e' la cosa che questo file
 *       dimostra al punto (a) delle macro di log. La stessa citazione
 *       dell'ilitek, che il livello ce l'ha, passa infatti:
 *         VERIFICATA_ASSEMBLATA 0xffffff800923f7b9: \x016ILITEK: (%s, %d): ...
 *       Quindi la ragione e' il limite del riconoscitore, non la provenienza
 *       della stringa. Vedi D10.
 *
 * D10. NON un header: `verificacitazioni.py`. `RE_TESTA_ASSEMBLATA` (riga
 *      262) riconosce solo i messaggi assemblati che cominciano per SOH +
 *      cifra di livello. I driver che di fabbrica usano `printk` nudo -- il
 *      Goodix e' uno -- restano fuori dal dominio e le loro citazioni
 *      risultano NON_ANCORATE anche quando la coda E' un letterale del
 *      codice. E' una decisione sullo strumento e questo lotto NON la
 *      prende: modificare il riconoscitore allarga CHI PASSA, e va fatto da
 *      chi puo' misurarne l'effetto su tutti i driver, non da chi ne ha uno
 *      che non passa. Registrato qui perche' non si perda: senza questa
 *      voce, il prossimo che legge «ESITO=1» crede di avere un difetto.
 */
