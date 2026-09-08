// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_ILITEK_E977 -- il GRUPPO E, tocco e rapporto. Doogee S88 Pro.
 *
 * Ventidue funzioni, 12132 byte, 0xffffff8008a5aad8..0xffffff8008a5da3c.
 * Il confine e' quello della ricognizione del 2026-08-21
 * (`scout-ilitek-E/RIASSUNTO.md` §1), chiuso con tre prove indipendenti:
 * adiacenza senza riempimento (la somma delle 22 dimensioni fa esattamente
 * l'ampiezza dell'intervallo), grappolo di `.rodata` disgiunto da quello dei
 * vicini, e SCONTRO dei `__LINE__` con i vicini (`ilitek_tddi_report_i2cuart_mode`
 * e `ilitek_tddi_ic_get_protocl_ver` hanno entrambe un messaggio a riga 1084,
 * quindi stanno in due file diversi).
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA. Nessun sorgente
 * pubblico e' stato letto. Ogni riga derivata porta accanto la riga di
 * disassemblato che la giustifica, nella forma
 * "<codifica8hex> <mnemonico>"@0xINDIRIZZO -- la sola che
 * `tools/verificaistruzioni.py` sappia confrontare col binario.
 *
 * COME SI RIVERIFICA:
 *
 *   ./venv/bin/python3 verificacitazioni.py \
 *       kernel/doogee/S88Pro/driver/ilitek_e977/ilitek_touch.c oracolo/stock.elf
 *   ./venv/bin/python3 verificaistruzioni.py \
 *       kernel/doogee/S88Pro/driver/ilitek_e977/ilitek_touch.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a5aad8:0xffffff8008a5da3c \
 *       --intervallo 0xffffff8008a53a78:0xffffff8008a53b48 \
 *       --intervallo 0xffffff8008a53ed8:0xffffff8008a54030 \
 *       --intervallo 0xffffff8008a5472c:0xffffff8008a55304 \
 *       --intervallo 0xffffff8008a55a48:0xffffff8008a55d0c \
 *       --intervallo 0xffffff80093acac4:0xffffff80093acaf8 \
 *       --controfattuale "d61f0100 br"
 *
 * Esito atteso, al 2026-08-21. ATTENZIONE:
 * `verificacitazioni.py` ESCE CON 1, e NON e' un difetto di questo file --
 * e' il conteggio delle sette NON_ANCORATA di "DELTA DI STRUMENTO", che
 * sono citazioni giuste che lo strumento non sa leggere. Fino a quando il
 * lotto che possiede gli strumenti non colma le due lacune di dizionario,
 * l'esito 1 e' quello atteso e va letto INSIEME al conteggio, non da solo.
 * `verificaistruzioni.py` esce con 0.
 *   verificacitazioni.py: 100 letterali, 112 citati, 105 verificati,
 *     104 probanti, 1 debole, 857 rinviate; 7 NON_ANCORATA, tutte e sette
 *     spiegate dalle due lacune di dizionario di "DELTA DI STRUMENTO".
 *   verificaistruzioni.py: 867 citazioni di istruzione, 864 confermate,
 *     0 assenti, 0 con mnemonico diverso, 3 controfattuali dichiarate
 *     (la stessa forma scartata, scritta in tre punti del file).
 *
 * I QUATTRO INTERVALLI IN PIU' NON SONO DEL GRUPPO E, e sono li' apposta:
 *   0xffffff8008a53a78.. `ilitek_tddi_fw_upgrade_handler` (gruppo A) e
 *   0xffffff8008a53ed8.. `ilitek_tddi_spi_recovery` (gruppo A) portano la
 *      prova della divergenza D1;
 *   0xffffff8008a5472c.. `ilitek_tddi_report_handler` (gruppo A) porta
 *      "93407e61 sxtw"@0xffffff8008a54dfc, la prova che il secondo
 *      parametro di `demo_debug_info_mode` e' a 64 bit;
 *   0xffffff8008a55a48.. `ilitek_i2c_write` (gruppo B) porta
 *      "3833ca97 strb"@0xffffff8008a55b30, la prova che il ritorno di
 *      `ilitek_calc_packet_checksum` viene troncato a byte dal chiamante;
 *   0xffffff80093acac4.. `ilitek_plat_dev_exit`, che sta nella `.exit.text`
 *      OLTRE `_einittext` e quindi FUORI da `oracolo/stock.map`: porta la
 *      prova della divergenza D7, cioe' dell'esito negativo della caccia
 *      alla `.exit` di questo blocco.
 * Col solo intervallo del gruppo E uscirebbero tutte ASSENTI, e sarebbero
 * cinque falsi allarmi.
 *
 * ===========================================================================
 * COSA C'E' IN QUESTO FILE E COSA NO
 * ===========================================================================
 * TUTTE E VENTIDUE le funzioni del blocco sono scritte. Nessuna e' stubata.
 *
 * QUESTO FILE NON LINKA DA SOLO, ed e' l'esito onesto: chiama quattordici
 * funzioni ILITEK che stanno fuori dal confine e che nessun lotto ha ancora
 * scritto -- `ilitek_ice_mode_ctrl`, `ilitek_ice_mode_read`,
 * `ilitek_ice_mode_write`, `ilitek_ice_mode_bit_mask_write`,
 * `ilitek_tddi_ic_check_busy`, `ilitek_tddi_ic_func_ctrl` (gruppo D),
 * `ilitek_tddi_reset_ctrl`, `ilitek_tddi_flash_clear_dma`,
 * `ilitek_tddi_flash_dma_write`, `ilitek_tddi_fw_upgrade_handler`
 * (gruppo G / A), `netlink_reply_msg` (gruppo H),
 * `ilitek_tddi_gesture_recovery`, `ilitek_tddi_switch_tp_mode`,
 * `ilitek_set_tp_data_len`. Sono DICHIARATE e non definite (regola 6).
 *
 * ===========================================================================
 * LA MISURA, dal `.o` vero, COL COMPILATORE DI FABBRICA
 * ===========================================================================
 * IL METRO DI MISURA E' CAMBIATO IL 2026-08-21, e la versione precedente di
 * questo cappello era basata su un fatto FALSO. Diceva: «`r353983c` non
 * esiste piu' negli archivi disponibili (verificato)», e attribuiva a quella
 * mancanza tutte e dieci le divergenze aperte. Il compilatore di fabbrica E'
 * STATO RECUPERATO (commit 540e404): sta in
 * /mnt/s88pro/kernel-stock/clang-r353983c, e la sua stringa di versione
 * coincide con quella dentro l'oracolo.
 *
 *   $ strings oracolo/stock.elf | grep -o "clang version [0-9.]*"
 *   clang version 9.0.3
 *   $ /mnt/s88pro/kernel-stock/clang-r353983c/bin/clang --version | head -1
 *   Android (5484270 based on r353983c) clang version 9.0.3 [...]
 *
 * Da qui in avanti la misura si fa DUE VOLTE: col compilatore di fabbrica
 * (r353983c, LLVM 9.0.3) e col nostro (r383902, LLVM 11.0.1). La prima dice
 * quanto il SORGENTE si discosta dalla fabbrica; la differenza fra le due
 * dice quanto se ne discosta il COMPILATORE, ed e' deriva da togliere, non
 * rumore da dichiarare.
 *
 * Comandi (l'ordine e' quello vero, l'uscita e' letterale):
 *
 *   # (a) la compilazione dentro kbuild, col NOSTRO compilatore, -Werror
 *   make -C albero-ilitek-e O=out-ilitek-e ARCH=arm64 -j6 CC=clang \
 *        HOSTCC=clang CLANG_TRIPLE=aarch64-linux-gnu- \
 *        CROSS_COMPILE=aarch64-linux-android- KCFLAGS=-Werror \
 *        drivers/input/touchscreen/mediatek/ilitek_e977/
 *     -> "  CC      drivers/.../ilitek_e977/ilitek_touch.o" e nient'altro:
 *        zero avvisi, zero errori.
 *
 *   # (b) la stessa riga di comando registrata da kbuild, col compilatore
 *   #     DI FABBRICA (/tmp/cc.sh fab ilitek_touch). Unica differenza:
 *   #     va tolto -Wno-int-in-bool-context, che clang 9 non conosce e che
 *   #     -Werror=unknown-warning-option rende fatale. Nel kernel vero quel
 *   #     flag passa da $(call cc-disable-warning,...), che con clang 9 non
 *   #     lo emette: toglierlo RIPRODUCE la riga di fabbrica, non la aggira.
 *   #     Esito: nessun avviso, nessun errore, con -Werror attivo.
 *
 *   # (c) la misura di dimensione dai due .o
 *   aarch64-linux-android-nm --print-size misura-E/ilitek_touch.{fab,noi}.o
 *
 * $ python3 /tmp/tab.py
 * funzione                               fabbrica r353983c  scarto  r383902  scarto
 * ilitek_tddi_move_mp_code_iram                48       48      +0       48      +0
 * ilitek_calc_packet_checksum                  48       48      +0       48      +0
 * ilitek_tddi_move_gesture_code_flash          72       72      +0       72      +0
 * demo_debug_info_mode                        112      112      +0      112      +0
 * ilitek_tddi_touch_release                   124      124      +0      124      +0
 * ilitek_tddi_proximity_near                  172      172      +0      172      +0
 * ilitek_tddi_touch_press                     228      228      +0      228      +0
 * ilitek_tddi_touch_release_all_point         232      232      +0      240      +8
 * ilitek_tddi_touch_send_debug_data           288      288      +0      288      +0
 * ilitek_tddi_proximity_far                   320      320      +0      320      +0
 * ilitek_dump_data                            396      396      +0      404      +8
 * dma_clear_reg_setting                       440      440      +0      440      +0
 * ilitek_tddi_report_i2cuart_mode             568      568      +0      564      -4
 * ilitek_tddi_touch_esd_gesture_flash         588      588      +0      588      +0
 * ilitek_tddi_move_gesture_code_iram          716      716      +0      712      -4
 * demo_debug_info_id0                         760      760      +0      760      +0
 * ilitek_tddi_touch_esd_gesture_iram          776      776      +0      780      +4
 * ilitek_tddi_report_ap_mode                 1012     1012      +0     1020      +8
 * ilitek_tddi_report_debug_mode              1040     1040      +0     1052     +12
 * ilitek_tddi_move_mp_code_flash             1304     1304      +0     1300      -4
 * dma_trigger_reg_setting                    1328     1328      +0     1324      -4
 * ilitek_tddi_report_gesture_mode            1560     1560      +0     1588     +28
 * TOTALE                                    12132    12132       0    12184      52
 * identiche col compilatore di fabbrica: 22/22
 * identiche col nostro compilatore:      12/22
 *
 * L'AVVERTENZA PRIMA DELLA PERCENTUALE: ventidue e' una numerosita' piccola,
 * e l'intervallo di confidenza va letto insieme al numero, non dopo.
 *   $ ./venv/bin/python3 intervallo.py 22 22
 *   22 su 22 = 100.0%   IC95% Clopper-Pearson [84.6%; 100.0%]
 *   contiene 77.10%: NO -- la misura discrimina
 *   $ ./venv/bin/python3 intervallo.py 12 22
 *   12 su 22 = 54.5%   IC95% Clopper-Pearson [32.2%; 75.6%]
 *   contiene 77.10%: NO -- la misura discrimina
 * DUE INTERVALLI CHE NON SI SOVRAPPONGONO SULLO STESSO SORGENTE: col
 * compilatore di fabbrica il blocco e' misurabilmente MEGLIO della media del
 * ramo, col nostro misurabilmente PEGGIO. La differenza fra i due numeri --
 * dieci funzioni su ventidue -- non e' della ricostruzione, e' del
 * compilatore, e leggere il 54,5% come «questo blocco va male» vorrebbe dire
 * misurare clang 11 invece del sorgente.
 * NOTA DI ONESTA': la colonna r383902 e' INSTABILE fra un giro e l'altro.
 * Le cinque correzioni C1..C5 qui sotto, che portano il residuo di codifica
 * di fabbrica a zero, PEGGIORANO la colonna del nostro clang (era 13/22 e
 * 12164 byte prima di C1 e C5, e' 12/22 e 12184 dopo). E' un'altra ragione
 * per non usarla come metro.
 *
 * ===========================================================================
 * LA MISURA FORTE: IL CONFRONTO PER CODIFICA, PAROLA PER PAROLA
 * ===========================================================================
 * La dimensione non basta, e lo dice il commit 540e404: sullo stesso
 * sorgente ALPS, col nostro clang 66 funzioni su 91 hanno la dimensione di
 * fabbrica ma solo 16 su 91 hanno le stesse istruzioni. Cinquanta passano la
 * misura di dimensione con codice diverso. Qui il confronto si fa quindi
 * anche PER CODIFICA A 32 BIT, funzione per funzione:
 *
 *   $ python3 /tmp/cmp4.py misura-E/ilitek_touch.fab.o
 *
 * Lo strumento maschera, su ENTRAMBI i lati, le sole parole che nel NOSTRO
 * .o portano una rilocazione (nell'oggetto valgono 0, nell'oracolo sono
 * risolte); confronta le `bl` per NOME del bersaglio e i salti locali per
 * SCOSTAMENTO dal principio della funzione; e conta a parte le differenze
 * fra due materializzazioni di costante nello stesso registro con entrambi i
 * valori in [1,6000], che sono i `__LINE__` (divergenza D2).
 *
 * ilitek_dump_data                         99 istr  __LINE__=1   residuo=0
 * ilitek_tddi_move_mp_code_flash          326 istr  __LINE__=9   residuo=0
 * dma_clear_reg_setting                   110 istr  __LINE__=7   residuo=0
 * dma_trigger_reg_setting                 332 istr  __LINE__=19  residuo=0
 * ilitek_tddi_move_mp_code_iram            12 istr  __LINE__=1   residuo=0
 * ilitek_tddi_proximity_near               43 istr  __LINE__=3   residuo=0
 * ilitek_tddi_proximity_far                80 istr  __LINE__=5   residuo=0
 * ilitek_tddi_move_gesture_code_flash      18 istr  __LINE__=1   residuo=0
 * ilitek_tddi_move_gesture_code_iram      179 istr  __LINE__=13  residuo=0
 * ilitek_calc_packet_checksum              12 istr  __LINE__=0   residuo=0
 * ilitek_tddi_touch_esd_gesture_flash     147 istr  __LINE__=10  residuo=0
 * ilitek_tddi_touch_esd_gesture_iram      194 istr  __LINE__=12  residuo=0
 * demo_debug_info_id0                     190 istr  __LINE__=23  residuo=0
 * demo_debug_info_mode                     28 istr  __LINE__=1   residuo=0
 * ilitek_tddi_report_ap_mode              253 istr  __LINE__=4   residuo=0
 * ilitek_tddi_touch_press                  57 istr  __LINE__=1   residuo=0
 * ilitek_tddi_touch_release                31 istr  __LINE__=1   residuo=0
 * ilitek_tddi_touch_release_all_point      58 istr  __LINE__=1   residuo=0
 * ilitek_tddi_touch_send_debug_data        72 istr  __LINE__=1   residuo=2
 * ilitek_tddi_report_debug_mode           260 istr  __LINE__=4   residuo=0
 * ilitek_tddi_report_gesture_mode         390 istr  __LINE__=5   residuo=0
 * ilitek_tddi_report_i2cuart_mode         142 istr  __LINE__=6   residuo=0
 * ---- identiche a meno di __LINE__: 21/22   residuo totale 2
 *
 *   $ ./venv/bin/python3 intervallo.py 21 22
 *   21 su 22 = 95.5%   IC95% Clopper-Pearson [77.2%; 99.9%]
 *   contiene 77.10%: NO -- la misura discrimina
 *
 * VENTUNO FUNZIONI SU VENTIDUE SONO IDENTICHE ISTRUZIONE PER ISTRUZIONE.
 * Il residuo TOTALE del blocco e' DUE parole su 3231.
 *
 * ===========================================================================
 * L'ATTRIBUZIONE DI OGNI DIVERGENZA
 * ===========================================================================
 * X1. IL RESIDUO CHE RESTA -- `ilitek_tddi_touch_send_debug_data`, 2 parole.
 *     Non e' una differenza di codice: sono DUE ISTRUZIONI ADIACENTI E
 *     INDIPENDENTI scambiate dentro lo stesso blocco.
 *       fabbrica: "4b0b014a sub"@0xffffff8008a5cd60 poi
 *                 "93407c76 sxtw"@0xffffff8008a5cd64
 *       nostro:   `sxtw x22, w3` poi `sub w10, w10, w11`
 *     `sub` scrive w10 e legge w10/w11; `sxtw` scrive x22 e legge w3: non
 *     hanno dipendenze, quindi l'ordine e' una scelta dello schedulatore e
 *     non cambia niente. NON SO quale forma del sorgente lo faccia
 *     ribaltare, e lo dico invece di inventarlo: e' il solo residuo del
 *     blocco che non ho attribuito a una causa nel sorgente.
 * X2. LE DIVERGENZE COL NOSTRO CLANG (nove funzioni, 32 byte netti) sono
 *     TUTTE del compilatore, e adesso e' MISURATO e non piu' asserito: lo
 *     stesso identico sorgente, compilato col compilatore di fabbrica, da'
 *     22/22 di dimensione e 21/22 di codifica. Il marcatore piu' visibile e'
 *     la disposizione del telaio: la fabbrica mette x29/x30 in CIMA all'area
 *     dei registri salvati ("a9047bfd stp"@0xffffff8008a5bdd4 a +64 di 80,
 *     con "910103fd add"@0xffffff8008a5bdd8), il nostro clang li mette in
 *     fondo -- stesso numero di istruzioni, quindi la dimensione non lo
 *     denuncia.
 *
 * ===========================================================================
 * LE CINQUE CORREZIONI DI SORGENTE DI QUESTO GIRO -- tutte trovate perche'
 * un numero non tornava, e tutte verificate col compilatore di fabbrica
 * ===========================================================================
 * C1. IL CICLO A CONTATORE E' DECRESCENTE, in `dma_trigger_reg_setting` (30
 *     giri) e in `ilitek_tddi_touch_esd_gesture_iram` (100 giri). Il binario
 *     parte da -N e sale: "128003ba mov"@0xffffff8008a5b6dc (-30) +
 *     "3100075a adds"@0xffffff8008a5b738 + "54fffda3 b.cc"@0xffffff8008a5b73c,
 *     e "12800c79 mov"@0xffffff8008a5c188 (-100) +
 *     "31000739 adds"@0xffffff8008a5c1f8 + "54fffce3 b.cc"@0xffffff8008a5c1fc.
 *     Il lotto precedente aveva scritto `for (i = 0; i != N; i++)`, che
 *     questo stesso compilatore rende con `mov #N` + `subs` + `b.ne`.
 *     Esperimento controllato (sei forme sorgente, misura-E/lab/loop.c,
 *     compilate con r353983c): SOLO `for (i = N; i > 0; i--)` produce
 *     `mov #-N` + `adds` + `b.cc`; `i < N` ne produce quattro invece di tre
 *     e cambia la dimensione. Corretto.
 * C2. IL SECONDO PARAMETRO DI `demo_debug_info_id0` E' A 64 BIT, non `int`.
 *     Vedi il cappello della funzione: e' una lettura che il lotto
 *     precedente aveva fatto al rovescio, e che rende `demo_debug_info_id0`
 *     e `demo_debug_info_mode` compatibili con la STESSA firma di tabella.
 * C3. IN `ilitek_tddi_report_i2cuart_mode` IL PRODOTTO DEI DUE BYTE E'
 *     CALCOLATO PRIMA DELLA CATENA DI `if`: "1b087d28 mul"@0xffffff8008a5d864
 *     sta nel blocco d'ingresso, prima di "54000928 b.hi"@0xffffff8008a5d868.
 *     Scritto dentro l'espressione finale, questo compilatore lo mette nel
 *     blocco di ricongiungimento: dimensione invariata, 29 parole diverse.
 * C4. IN `ilitek_tddi_move_mp_code_flash` LA BANDIERA `overlay` E' UNA
 *     CONGIUNZIONE IN DUE PASSI: "0a090108 and"@0xffffff8008a5ada4 fonde le
 *     due prove `!= 0` e "0a080138 and"@0xffffff8008a5adac vi somma
 *     `buf[0] == 0xfe` COME PRIMO OPERANDO. Un `a && b && c` in una sola
 *     espressione da' l'associazione a sinistra e 32 parole diverse (con la
 *     stessa dimensione, che quindi non lo denuncia); i due passi separati
 *     danno residuo zero, e sistemano da soli anche l'ordine delle otto
 *     `ldrb`/`bfi` che li precedono.
 * C5. IN `ilitek_tddi_report_gesture_mode`, DUE COSE.
 *     (a) I tre campi di `idev` si leggono PRIMA della copia del pacchetto
 *         ("f9452308 ldr"@0xffffff8008a5d220 sta prima di
 *         "940feaa3 bl"@0xffffff8008a5d274): letti dopo, il carico del
 *         globale resta dopo la copia.
 *     (b) I CASI 0x60..0x68 NON FANNO IL CALCOLO DEI QUATTRO PUNTI. Il lotto
 *         precedente lo dava a tutti e dieci i casi 0x60..0x69 e la MISURA
 *         DI DIMENSIONE NON LO DENUNCIAVA (1560 = 1560): e' la classe B6 di
 *         `criteri-revisione.md`, codice che nel binario non c'e'. La prova
 *         e' la tabella di salto letta dal binario -- vedi il commento sul
 *         posto. E' il difetto piu' grave trovato in questo giro, ed e' un
 *         difetto di COMPORTAMENTO: nove gesti su dieci scrivevano sei
 *         coordinate che la fabbrica non scrive.
 *
 * ===========================================================================
 * LE DIVERGENZE APERTE DI QUESTO FILE -- elenco numerato
 * ===========================================================================
 * D1. `ilitek_tddi_fw_upgrade_handler` HA UN PARAMETRO in questo file e NON
 *     ne ha nel gruppo A. Non e' un'ipotesi: e' misurato sui siti di
 *     chiamata. I quattro siti del gruppo A non toccano x0 --
 *     "97fffeb0 bl"@0xffffff8008a53fb8 in `ilitek_tddi_spi_recovery` viene
 *     subito dopo "97db7d4a bl"@0xffffff8008a53fac (una `printk`, che
 *     x0 lo sporca), quindi x0 NON e' noto-zero e la `mov` manca perche' il
 *     prototipo visibile li' non ha parametri. I siti del gruppo E invece
 *     azzerano x0 prima: "aa1f03e0 mov"@0xffffff8008a5b884,
 *     "aa1f03e0 mov"@0xffffff8008a5bcd4, "aa1f03e0 mov"@0xffffff8008a5c100,
 *     "aa1f03e0 mov"@0xffffff8008a5c274. La definizione
 *     ("b941c100 ldr"@0xffffff8008a53a94 sovrascrive x0 subito) non usa mai
 *     il parametro: e' un parametro ignorato.
 *     Nel sorgente di fabbrica i due file vedono quindi DUE PROTOTIPI
 *     INCOMPATIBILI della stessa funzione. `ilitek.h` porta gia' quello del
 *     gruppo A (`int ilitek_tddi_fw_upgrade_handler(void)`); questo file non
 *     puo' cambiarlo (e' un header CONDIVISO), e per riprodurre il `bl` DIRETTO
 *     con x0 azzerato usa un alias con etichetta di assemblatore. Vedi il
 *     "DELTA DI HEADER 1" piu' sotto.
 * D2. I `__LINE__`. I numeri di fabbrica sono noti uno per uno e annotati
 *     sopra ogni funzione, ma questo file NON li riproduce: imbottire di
 *     righe vuote un file che verra' ancora toccato sposta tutto al primo
 *     cambio. E' una divergenza reale nel binario e INVISIBILE alla misura
 *     di dimensione (`mov w2,#imm16` e' una istruzione per qualunque riga).
 * D3. `ilitek_calc_packet_checksum`: `ilitek_bus.c` la dichiara
 *     `u8 ilitek_calc_packet_checksum(void *buf, u32 len)`, deducendola dal
 *     solo sito di chiamata e dicendolo. La DEFINIZIONE la smentisce sul
 *     secondo parametro: "7100043f cmp"@0xffffff8008a5bd94 seguito da
 *     "5400012b b.lt"@0xffffff8008a5bd98 e' un confronto CON SEGNO. Con un
 *     `u32`, `len < 1` sarebbe `len == 0` e clang avrebbe emesso `cbz w1`.
 *     Il parametro e' un intero CON SEGNO. Il tipo di ritorno regge (il
 *     chiamante tronca a byte con "3833ca97 strb"@0xffffff8008a55b30).
 *     UNA DELLE DUE DICHIARAZIONI E' SBAGLIATA ED E' QUELLA DI `ilitek_bus.c`.
 *     Non la correggo io: `ilitek_bus.c` non e' mio.
 * D4. `ilitek_dump_data`: `ilitek_bus.c` chiama i tre interi `a`, `b`, `c`
 *     dicendo che «nessuna misura dice cosa siano». La definizione li decide
 *     (vedi il cappello della funzione); i nomi che uso qui descrivono il
 *     RUOLO MISURATO, e restano nomi miei -- il binario non nomina i
 *     parametri di nessuna funzione.
 * D5. `ilitek_tddi_touch_release` esiste come simbolo fuori linea
 *     (0xffffff8008a5cb58, `T`, 124 byte) ED e' incorporata in tre chiamanti.
 *     Qui e' scritta UNA volta e lasciata incorporare. RISOLTA e verificata:
 *     con entrambi i compilatori le tre incorporazioni avvengono, e
 *     `ilitek_tddi_report_ap_mode`, `ilitek_tddi_report_debug_mode` e
 *     `ilitek_tddi_touch_release_all_point` hanno residuo di codifica ZERO.
 *     Il controllo negativo regge nello stesso .o:
 *     `ilitek_tddi_touch_press` resta una `bl` vera.
 * D6. IL RESIDUO DI DUE PAROLE di `ilitek_tddi_touch_send_debug_data`
 *     ("4b0b014a sub"@0xffffff8008a5cd60 e "93407c76 sxtw"@0xffffff8008a5cd64
 *     nell'ordine opposto al nostro). E' il solo punto del blocco in cui non
 *     so quale forma del sorgente riproduca l'ordine di fabbrica. Le due
 *     istruzioni sono indipendenti, quindi la differenza non ha effetto, ma
 *     resta una divergenza APERTA e non un fatto spiegato.
 * D7. NESSUNA FUNZIONE `.exit` APPARTIENE A QUESTO BLOCCO, ed e' un esito
 *     NEGATIVO misurato, non un'omissione. `oracolo/stock.map` finisce a
 *     `_einittext` (0xffffff80093a8518) e non copre la `.exit.text`, per cui
 *     la zona 0xffffff80093a8518..0xffffff80093c0000 e' stata disassemblata a
 *     mano (24257 righe) e cercata in due modi:
 *       - `bl` verso un indirizzo dentro [0xffffff8008a5aad8, 0xffffff8008a5da3c):
 *         totale 0;
 *       - `adrp` verso la pagina dei globali ILITEK 0xffffff800a0fc000:
 *         totale 0.
 *     L'unica `.exit` del driver ILITEK e' `ilitek_plat_dev_exit`
 *     (0xffffff80093acac4, 52 byte, "97da926f bl"@0xffffff80093acaec verso
 *     `tpd_driver_remove`), che sta in `ilitek_plat.c` -- gia' scritta nel
 *     repository dal commit ee7669c, con il proprio `__func__` gia'
 *     verificato la'. Questo blocco non ne ha una propria.
 *
 * ===========================================================================
 * DELTA DI HEADER -- da fare al lotto di merge, NON qui
 * ===========================================================================
 * DELTA DI HEADER 1: `ilitek.h` riga 998 dichiara
 *   `int ilitek_tddi_fw_upgrade_handler(void);`
 * Serve una seconda forma per questo file (D1). La soluzione pulita e' una
 * macro o un secondo prototipo con nome diverso e alias, decisa dal merge:
 * questo file la realizza LOCALMENTE, senza toccare l'header.
 *
 * DELTA DI HEADER 2: le funzioni del gruppo D e del gruppo G che questo
 * blocco chiama (`ilitek_ice_mode_*`, `ilitek_tddi_ic_func_ctrl`, ...) non
 * sono in `ilitek.h`. Qui sono dichiarate LOCALMENTE, come fa gia'
 * `ilitek_bus.c` per le funzioni del gruppo E. Quando i gruppi D/G saranno
 * scritti le dichiarazioni vanno spostate nell'header e TOLTE da qui.
 *
 * ===========================================================================
 * DELTA DI STRUMENTO -- due lacune di dizionario, da NON tappare qui
 * ===========================================================================
 * Sono le sole sette citazioni che `verificacitazioni.py` marca
 * NON_ANCORATA. Nessuna delle sette e' sbagliata: sono corrette e lo
 * strumento non le sa leggere. E' la classe C1 di `criteri-revisione.md`
 * (il dizionario di uno strumento), e gli strumenti sono CONDIVISI: qui si
 * descrivono, non si toccano.
 *
 * S1. `verificacitazioni.py` NON CONOSCE `KERN_CONT`. La sua tabella
 *     `LIVELLO_KERN` ha solo le otto macro con un livello NUMERICO
 *     (`KERN_EMERG`..`KERN_DEBUG`); `KERN_CONT` e' `KERN_SOH "c"`, con una
 *     lettera al posto della cifra, e non c'e'. Le sei stampe di
 *     `ilitek_dump_data` -- "\x01c\n\n"@0xffffff8009241cc5,
 *     "\x01cILITEK: Dump %s data\n"@0xffffff8009241cca,
 *     "\x01cILITEK: "@0xffffff8009241ce2, "\x01c %4x "@0xffffff8009241ced,
 *     "\x01c %4d "@0xffffff8009241cf5, "\x01c\n"@0xffffff80090ddcf1 --
 *     sono quindi citate correttamente e non ancorabili.
 *     E' la prima volta che il problema si presenta perche' e' la prima
 *     volta che un lotto scrive una funzione che usa `KERN_CONT`: sono i
 *     42 siti che `ilitek.h` gia' segnalava come «le stampe di dump grezzo,
 *     tutte fuori dal gruppo A».
 * S2. `confinecitazioni.py` NON HA `cinc` nella lista `MNEMONICI`. Ha
 *     `csel`, `csinc`, `csinv`, `csneg`, `cset`, `csetm`, `ccmp`, `ccmn`,
 *     ma non gli alias a due operandi `cinc` / `cinv` / `cneg`. La
 *     citazione "1a93166a cinc"@0xffffff8008a5cf08 di
 *     `ilitek_tddi_report_debug_mode` finisce percio' nel dominio dei
 *     LETTERALI invece che in quello delle istruzioni, ed e' invisibile a
 *     `verificaistruzioni.py`. Verificata a mano:
 *       $ ./venv/bin/python3 disassembla.py oracolo/stock.map \
 *             oracolo/stock.elf ilitek_tddi_report_debug_mode --grezzo \
 *         | grep a5cf08
 *       ffffff8008a5cf08:  1a93166a  cinc  w10, w19, eq
 *
 * DELTA DI HEADER 4 (per `ilitek_bus.c`, NON per `ilitek.h`): quel file
 * dichiara localmente `u8 ilitek_calc_packet_checksum(void *buf, u32 len);`.
 * Il secondo parametro e' CON SEGNO, e la definizione lo prova:
 * "7100043f cmp"@0xffffff8008a5bd94 + "5400012b b.lt"@0xffffff8008a5bd98.
 * Con un `u32`, `len < 1` sarebbe `len == 0` e clang avrebbe emesso
 * `cbz w1`. Qui la definizione dice `int len`. Le due dichiarazioni stanno
 * in unita' di compilazione diverse, per cui il compilatore non se ne
 * accorge e il link riesce lo stesso: e' un difetto latente, e va corretto
 * in `ilitek_bus.c`, che non e' mio.
 * RITIRATO, invece, il «delta di header 4» del lotto precedente, che
 * chiedeva DUE firme diverse per la tabella `idev->c848[]` perche'
 * `demo_debug_info_id0` avrebbe avuto un secondo parametro `int` e
 * `demo_debug_info_mode` uno a 64 bit. La misura col compilatore di
 * fabbrica dice che sono ENTRAMBI a 64 bit (vedi il cappello di
 * `demo_debug_info_id0`): una firma sola basta, e il delta cade.
 *
 * DELTA DI HEADER 3: nessun campo nuovo di `struct ilitek_tddi_dev` serve a
 * questo sotto-lotto. I ventuno scostamenti che il blocco tocca
 * (`scout-ilitek-E/campi_idev.txt`) sono TUTTI gia' presenti in `ilitek.h`.
 * Tre di essi CONFERMANO cio' che il gruppo B aveva scritto e potevano
 * smentirlo: +776 e +784 letti e chiamati come `ilitek_i2c_write` /
 * `ilitek_i2c_read`, +816 letto e chiamato da
 * `ilitek_tddi_touch_esd_gesture_flash` -- che e' esattamente cio' che
 * `ilitek_i2c_probe` ci scrive.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include "ilitek.h"

/*
 * ===========================================================================
 * D1 / DELTA DI HEADER 1 -- l'alias a un parametro
 * ===========================================================================
 * `ilitek.h` dichiara `ilitek_tddi_fw_upgrade_handler` senza parametri,
 * perche' e' cosi' che la vede il gruppo A, che la DEFINISCE. Questo file la
 * vede con un parametro, e lo dimostrano quattro `mov x0, xzr` che il gruppo
 * A non ha. L'alias con etichetta di assemblatore produce lo stesso `bl`
 * DIRETTO verso lo stesso simbolo, con x0 azzerato: e' il modo di riprodurre
 * il binario senza modificare un header condiviso.
 * Il parametro e' ignorato dalla definizione
 * ("b941c100 ldr"@0xffffff8008a53a94 sovrascrive x0 al terzo passo), quindi
 * il suo TIPO non e' misurato: `void *` e' la lettura piu' larga compatibile
 * con l'unico valore mai passato, che e' zero.
 */
extern int ilitek_tddi_fw_upgrade_handler_arg(void *dato)
	__asm__("ilitek_tddi_fw_upgrade_handler");

/*
 * ===========================================================================
 * LE FUNZIONI ILITEK FUORI DAL BLOCCO -- dichiarate, non definite
 * ===========================================================================
 * DELTA DI HEADER 2. Le firme vengono dai siti di chiamata di QUESTO blocco,
 * e il commento dice quanto ogni sito decide davvero. Nessuna e' definita
 * qui: il link fallisce, ed e' l'esito onesto (regola 6).
 */

/*
 * Gruppo D. "97fff16f bl"@0xffffff8008a5b8cc: x0 e' un puntatore a stringa
 * ("sleep"@0xffffff800911c431, materializzata da
 * "9110c400 add"@0xffffff8008a5b8c4), w1 un intero azzerato
 * ("2a1f03e1 mov"@0xffffff8008a5b8c8), il ritorno provato sul solo bit di
 * segno ("36f802e0 tbz"@0xffffff8008a5b8d4).
 */
int ilitek_tddi_ic_func_ctrl(const char *nome, int ctrl);

/*
 * ===========================================================================
 * ilitek_calc_packet_checksum -- 0xffffff8008a5bd94, 48 byte
 * ===========================================================================
 * Riga di fabbrica: nessuna. Non stampa niente, quindi non ha `__LINE__`:
 * e' una delle due funzioni del blocco senza collocazione misurata nel file.
 *
 * Il secondo parametro e' CON SEGNO: vedi D3.
 * Il ciclo e' a rovescio su un contatore a 64 bit
 * ("2a0103e9 mov"@0xffffff8008a5bda0 zero-estende `len`,
 * "f1000529 subs"@0xffffff8008a5bda8), con il puntatore post-incrementato
 * ("3840140a ldrb"@0xffffff8008a5bda4) e la somma a 32 bit senza
 * troncamenti ("0b0a0108 add"@0xffffff8008a5bdac).
 * Il ritorno e' la negazione a 32 bit, NON troncata a byte
 * ("4b0803e0 neg"@0xffffff8008a5bdb4): il tronco lo fa il chiamante.
 */
u8 ilitek_calc_packet_checksum(u8 *buf, int len)
{
	int i;
	int somma = 0;

	for (i = 0; i < len; i++)
		somma += buf[i];

	/* "4b0803e0 neg"@0xffffff8008a5bdb4 */
	return (u8)-somma;
}

/*
 * ===========================================================================
 * ilitek_dump_data -- 0xffffff8008a5aad8, 396 byte
 * ===========================================================================
 * Riga di fabbrica del solo messaggio: 75
 * ("52800962 mov"@0xffffff8008a5ac3c).
 *
 * I NOMI DEI PARAMETRI DESCRIVONO IL RUOLO MISURATO (D4), non sono del
 * binario:
 *
 *  - `larghezza` (w1) sceglie insieme la larghezza dell'elemento e il
 *    formato, e i valori ammessi sono QUATTRO:
 *      8   -> "7100227f cmp"@0xffffff8008a5ab9c, `ldrb`
 *             ("387c6b21 ldrb"@0xffffff8008a5aba4), formato
 *             "\x01c %4x "@0xffffff8009241ced
 *      32  -> "7100827f cmp"@0xffffff8008a5abb0, `ldr w`
 *             ("b87c7b41 ldr"@0xffffff8008a5abb8), stesso "%4x"
 *      10  -> "71002a7f cmp"@0xffffff8008a5abc4, `ldr w`
 *             ("b87c7b41 ldr"@0xffffff8008a5abcc), formato
 *             "\x01c %4d "@0xffffff8009241cf5
 *      16  -> "7100427f cmp"@0xffffff8008a5abd4, `ldrsh` -- CON SEGNO
 *             ("78fc7901 ldrsh"@0xffffff8008a5abe0), stesso "%4d"
 *    Ogni altro valore non stampa niente
 *    ("540000c1 b.ne"@0xffffff8008a5abd8 salta la `printk`).
 *  - `numero` (w2) e' il NUMERO di elementi: la guardia e' CON SEGNO
 *    ("7100069f cmp"@0xffffff8008a5ab6c + "5400054b b.lt"@0xffffff8008a5ab74)
 *    e il limite del ciclo e' a 64 bit ("eb1c02ff cmp"@0xffffff8008a5ac14).
 *  - `per_riga` (w3): se e' <= 0 il valore usato e' TRENTUNO, non 32
 *    ("320013e8 orr"@0xffffff8008a5ab08 mette 0x1f,
 *    "1a88c078 csel"@0xffffff8008a5ab10 sceglie `gt`).
 *  - `etichetta` (x4) e' stampata da "\x01cILITEK: Dump %s data\n".
 *
 * L'a capo cade quando `i % per_riga == per_riga - 1`, calcolato con una
 * divisione CON SEGNO ("1ad80f88 sdiv"@0xffffff8008a5abf0 +
 * "1b18f108 msub"@0xffffff8008a5abf4 + "6b1b011f cmp"@0xffffff8008a5abf8,
 * dove w27 e' `per_riga - 1`, "5100071b sub"@0xffffff8008a5ab8c).
 *
 * Tutto il corpo, MESSAGGIO D'ERRORE COMPRESO, sta sotto la guardia del
 * diagnostico: "34000a28 cbz"@0xffffff8008a5ab00 salta all'epilogo.
 */
void ilitek_dump_data(void *dato, int larghezza, int numero, int per_riga,
		      const char *etichetta)
{
	int i;
	int colonne;
	u8 *p8;
	u32 *p32;
	s16 *p16;

	/* "39656108 ldrb"@0xffffff8008a5aafc, "34000a28 cbz"@0xffffff8008a5ab00 */
	if (!ilitek_dbg_en)
		return;

	/* "1a88c078 csel"@0xffffff8008a5ab10 -- `gt`, quindi 0 e i negativi
	 * prendono il 31 di "320013e8 orr"@0xffffff8008a5ab08 */
	colonne = (per_riga > 0) ? per_riga : 31;

	/* "b40008c0 cbz"@0xffffff8008a5ab14 */
	if (!dato) {
		/* "\x013ILITEK: (%s, %d): The data going to dump is NULL\n"@0xffffff8009241c80,
		 * __func__ "ilitek_dump_data"@0xffffff8009241cb4, riga 75 */
#line 75
		ILI_ERR("The data going to dump is NULL\n");
		return;
	}

	/* "\x01c\n\n"@0xffffff8009241cc5, "97db626a bl"@0xffffff8008a5ab2c */
	printk(KERN_CONT "\n\n");
	/* "\x01cILITEK: Dump %s data\n"@0xffffff8009241cca, x1 = etichetta
	 * ("aa1603e1 mov"@0xffffff8008a5ab38) */
	printk(KERN_CONT "ILITEK: Dump %s data\n", etichetta);
	/* "\x01cILITEK: "@0xffffff8009241ce2, "97db6263 bl"@0xffffff8008a5ab48 */
	printk(KERN_CONT "ILITEK: ");

	/*
	 * I TRE PUNTATORI SONO CALCOLATI PRIMA DEL CICLO, e non e' una scelta
	 * di stile: sono tre `csel` con `xzr` come alternativa, cioe' tre
	 * puntatori che valgono `dato` se la condizione tiene e NULL se no.
	 *   "9a9f02b9 csel"@0xffffff8008a5ab54 (larghezza == 8)
	 *   "9a9f02ba csel"@0xffffff8008a5ab60 (larghezza == 10 || == 32,
	 *      condizione composta da "71002a7f cmp"@0xffffff8008a5ab58 e
	 *      "7a481264 ccmp"@0xffffff8008a5ab5c, che e' la forma con cui
	 *      clang rende un `||` fra due confronti)
	 *   "9a9f02a8 csel"@0xffffff8008a5ab68 (larghezza == 16), riversato
	 *      subito sulla pila da "f90007e8 str"@0xffffff8008a5ab70 e
	 *      riletto dentro il ciclo da "f94007e8 ldr"@0xffffff8008a5abdc.
	 * Un ciclo che indicizzasse `dato` direttamente non produrrebbe
	 * nessuna `csel`: prima stesura, 356 byte contro 396, e clang fondeva
	 * i quattro confronti in una TABELLA DI SALTO -- "d61f0100 br" --, che
	 * nel binario di fabbrica NON c'e'. Quella citazione e' una
	 * CONTROFATTUALE dichiarata: descrive cio' che il NOSTRO compilatore
	 * produceva, e va passata a `verificaistruzioni.py` con
	 * `--controfattuale "d61f0100 br"`, altrimenti lo strumento la segnala
	 * ASSENTE e ha ragione.
	 * L'ORDINE dei tre e' quello del binario: 8, poi (10 || 32), poi 16 --
	 * diverso dall'ordine con cui il CICLO li riprova.
	 */
	p8 = (larghezza == 8) ? dato : NULL;
	p32 = (larghezza == 32 || larghezza == 10) ? dato : NULL;
	p16 = (larghezza == 16) ? dato : NULL;

	for (i = 0; i < numero; i++) {
		if (larghezza == 8)
			printk(KERN_CONT " %4x ", p8[i]);
		else if (larghezza == 32)
			printk(KERN_CONT " %4x ", p32[i]);
		else if (larghezza == 10)
			printk(KERN_CONT " %4d ", p32[i]);
		else if (larghezza == 16)
			printk(KERN_CONT " %4d ", p16[i]);

		if ((i % colonne) == (colonne - 1)) {
			/* "\x01c\n"@0xffffff80090ddcf1 -- NON e' nel grappolo
			 * di .rodata del blocco: e' una stringa fusa dal
			 * linker, "97db6234 bl"@0xffffff8008a5ac04 */
			printk(KERN_CONT "\n");
			printk(KERN_CONT "ILITEK: ");
		}
	}
	/* "97db622c bl"@0xffffff8008a5ac24 -- di nuovo "\x01c\n\n" */
	printk(KERN_CONT "\n\n");
}

/*
 * ===========================================================================
 * ilitek_tddi_touch_press -- 0xffffff8008a5ca74, 228 byte
 * ===========================================================================
 * Riga di fabbrica: 692 ("52805682 mov"@0xffffff8008a5cab4).
 *
 * TUTTI E QUATTRO i parametri sono a 16 BIT SENZA SEGNO, e lo dicono le
 * quattro maschere che la funzione fa DA SOLA sugli argomenti in ingresso --
 * cioe' esattamente cio' che l'ABI impone a un chiamato che riceve `u16`:
 *   "12003c76 and"@0xffffff8008a5ca90 (w3 -> id)
 *   "12003c15 and"@0xffffff8008a5ca94 (w0 -> x)
 *   "12003c34 and"@0xffffff8008a5ca98 (w1 -> y)
 *   "12003c53 and"@0xffffff8008a5ca9c (w2 -> p)
 * I NOMI x, y, p, id vengono dal binario: li nomina la stringa di formato
 * "Touch Press: id = %d, x = %d, y = %d, p = %d" e l'ordine degli argomenti
 * della `printk` li lega ai registri
 * ("2a1603e3 mov"@0xffffff8008a5cab8 = id in w3, poi x, y, p).
 *
 * Questa funzione NON E' INCORPORATA nei suoi due chiamanti: e' il controllo
 * negativo che regge, "94000089 bl"@0xffffff8008a5c850 in
 * `ilitek_tddi_report_ap_mode`.
 *
 * I codici sono quelli standard di `input.h` e sono MISURATI:
 * 0x2f = ABS_MT_SLOT ("528005e2 mov"@0xffffff8008a5cad8),
 * 0x35 = ABS_MT_POSITION_X ("528006a2 mov"@0xffffff8008a5cb04),
 * 0x36 = ABS_MT_POSITION_Y ("528006c2 mov"@0xffffff8008a5cb1c),
 * 0x3a = ABS_MT_PRESSURE ("52800742 mov"@0xffffff8008a5cb34),
 * 3 = EV_ABS ("320007e1 orr"@0xffffff8008a5cad4).
 */
void ilitek_tddi_touch_press(u16 x, u16 y, u16 p, u16 id)
{
	/* "\x016ILITEK: (%s, %d): Touch Press: id = %d, x = %d, y = %d, p = %d\n"@0xffffff8009242a67,
	 * __func__ "ilitek_tddi_touch_press"@0xffffff8009242aa9, riga 692,
	 * guardia "34000168 cbz"@0xffffff8008a5caa0 */
#line 692
	ILI_DBG("Touch Press: id = %d, x = %d, y = %d, p = %d\n", id, x, y, p);

	/* "97ffa747 bl"@0xffffff8008a5cae4, x0 = idev->c16
	 * ("f9400900 ldr"@0xffffff8008a5cae0) */
	input_report_abs(idev->c16, ABS_MT_SLOT, id);
	/* "97ffba4c bl"@0xffffff8008a5caf8, w1 = 0 = MT_TOOL_FINGER
	 * ("2a1f03e1 mov"@0xffffff8008a5caf0), w2 = 1
	 * ("320003e2 orr"@0xffffff8008a5caec) */
	input_mt_report_slot_state(idev->c16, MT_TOOL_FINGER, true);
	/* "97ffa73c bl"@0xffffff8008a5cb10 */
	input_report_abs(idev->c16, ABS_MT_POSITION_X, x);
	/* "97ffa736 bl"@0xffffff8008a5cb28 */
	input_report_abs(idev->c16, ABS_MT_POSITION_Y, y);
	/* "97ffa730 bl"@0xffffff8008a5cb40 */
	input_report_abs(idev->c16, ABS_MT_PRESSURE, p);
}

/*
 * ===========================================================================
 * ilitek_tddi_touch_release -- 0xffffff8008a5cb58, 124 byte
 * ===========================================================================
 * Riga di fabbrica: 717 ("528059a2 mov"@0xffffff8008a5cb8c).
 *
 * Tre parametri a 16 bit senza segno. La maschera di `id` sta FUORI dalla
 * guardia del diagnostico ("12003c53 and"@0xffffff8008a5cb6c, prima del
 * "34000148 cbz"@0xffffff8008a5cb70) perche' `id` serve anche a
 * `input_report_abs`; quelle di `x` e `y` stanno DENTRO
 * ("12003c04 and"@0xffffff8008a5cb74, "12003c25 and"@0xffffff8008a5cb78)
 * perche' li' servono solo alla `printk`.
 *
 * D5: questa funzione e' anche INCORPORATA in tre chiamanti. La prova e' il
 * suo `__func__`: "ilitek_tddi_touch_release"@0xffffff8009242afd viene
 * materializzato anche da `ilitek_tddi_touch_release_all_point`
 * ("912bf6b5 add"@0xffffff8008a5cc00), e nessuno dei tre chiamanti ha una
 * `bl` verso 0xffffff8008a5cb58.
 */
void ilitek_tddi_touch_release(u16 x, u16 y, u16 id)
{
	/* "\x016ILITEK: (%s, %d): Touch Release: id = %d, x = %d, y = %d\n"@0xffffff8009242ac1,
	 * riga 717, guardia "34000148 cbz"@0xffffff8008a5cb70 */
#line 717
	ILI_DBG("Touch Release: id = %d, x = %d, y = %d\n", id, x, y);

	/* "97ffa714 bl"@0xffffff8008a5cbb0 */
	input_report_abs(idev->c16, ABS_MT_SLOT, id);
	/* "97ffba19 bl"@0xffffff8008a5cbc4, w1 = 0 e w2 = 0
	 * ("2a1f03e1 mov"@0xffffff8008a5cbb8,
	 * "2a1f03e2 mov"@0xffffff8008a5cbbc) */
	input_mt_report_slot_state(idev->c16, MT_TOOL_FINGER, false);
}

/*
 * ===========================================================================
 * ilitek_tddi_touch_release_all_point -- 0xffffff8008a5cbd4, 232 byte
 * ===========================================================================
 * Riga di fabbrica: nessuna PROPRIA. L'unico messaggio che compare qui e'
 * quello di `ilitek_tddi_touch_release` (riga 717,
 * "528059a2 mov"@0xffffff8008a5cc0c), incorporata: e' la seconda delle due
 * funzioni del blocco senza collocazione misurata nel file.
 *
 * Il ciclo conta DIECI dita ("71002a7f cmp"@0xffffff8008a5cc58), e la
 * dimensione dell'array di §6.1 della ricognizione (80 byte / 8) concorda.
 *
 * In coda tre eventi: BTN_TOUCH = 0x14a ("52802942 mov"@0xffffff8008a5cc68)
 * e BTN_TOOL_FINGER = 0x145 ("528028a2 mov"@0xffffff8008a5cc80) di tipo
 * EV_KEY = 1 ("320003e1 orr"@0xffffff8008a5cc64), poi
 * EV_SYN/SYN_REPORT/0 -- cioe' `input_sync`, che e' una `static inline`
 * ("97ffa6d7 bl"@0xffffff8008a5cca4 con w1, w2, w3 tutti a zero:
 * "2a1f03e1 mov"@0xffffff8008a5cc94, "2a1f03e2 mov"@0xffffff8008a5cc98,
 * "2a1f03e3 mov"@0xffffff8008a5cc9c).
 */
void ilitek_tddi_touch_release_all_point(void)
{
	int i;

	/* "11000673 add"@0xffffff8008a5cc54 -- passo 1 */
	for (i = 0; i < 10; i++)
		ilitek_tddi_touch_release(0, 0, i);

	/* "97ffa6e3 bl"@0xffffff8008a5cc74 */
	input_report_key(idev->c16, BTN_TOUCH, 0);
	/* "97ffa6dd bl"@0xffffff8008a5cc8c */
	input_report_key(idev->c16, BTN_TOOL_FINGER, 0);
	/* "97ffa6d7 bl"@0xffffff8008a5cca4 */
	input_sync(idev->c16);
}

/*
 * ===========================================================================
 * ilitek_tddi_proximity_near -- 0xffffff8008a5b894, 172 byte
 * ===========================================================================
 * Righe di fabbrica: 340, 343, 346
 * ("52802a82 mov"@0xffffff8008a5b8e8, "52802ae2 mov"@0xffffff8008a5b904,
 * "52802b42 mov"@0xffffff8008a5b924). Il passo costante di TRE righe fra i
 * tre messaggi dice che i tre rami sono adiacenti nel sorgente e che il ramo
 * a riga 340 viene PER PRIMO; e' una lettura dei numeri, non una prova.
 *
 * Il campo `idev->c637` e' messo a 1 SEMPRE, prima di qualunque prova
 * ("320003e9 orr"@0xffffff8008a5b8a8 +
 * "3909f509 strb"@0xffffff8008a5b8ac).
 */
int ilitek_tddi_proximity_near(int mode)
{
	int ret = 0;

	/* "3909f509 strb"@0xffffff8008a5b8ac -- idev+637 = 1 */
	idev->c637 = 1;

	switch (mode) {
	case 1:
		/* "97fff16f bl"@0xffffff8008a5b8cc, x0 = "sleep"@0xffffff800911c431,
		 * w1 = 0 ("2a1f03e1 mov"@0xffffff8008a5b8c8) */
		ret = ilitek_tddi_ic_func_ctrl("sleep", 0);
		/* "36f802e0 tbz"@0xffffff8008a5b8d4 -- solo il bit di segno */
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): Write sleep in cmd failed\n"@0xffffff800923e559
			 * -- fuori dal grappolo del blocco: e' fusa col
			 * gruppo D. Riga 340. */
#line 340
			ILI_ERR("Write sleep in cmd failed\n");
		break;
	case 0:
		/* "\x016ILITEK: (%s, %d): DDI POWER OFF, do nothing\n"@0xffffff8009241f99,
		 * riga 343, NON guardata dal diagnostico */
#line 343
		ILI_INFO("DDI POWER OFF, do nothing\n");
		break;
	default:
		/* "\x013ILITEK: (%s, %d): Unknown mode (%d)\n"@0xffffff8009241fc8,
		 * riga 346, w3 = mode ("2a0003e3 mov"@0xffffff8008a5b8b4) */
#line 346
		ILI_ERR("Unknown mode (%d)\n", mode);
		/* "128002b3 mov"@0xffffff8008a5b92c -- -22 = -EINVAL */
		ret = -EINVAL;
		break;
	}

	/* "2a1303e0 mov"@0xffffff8008a5b934 */
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_proximity_far -- 0xffffff8008a5b940, 320 byte
 * ===========================================================================
 * Righe di fabbrica: 359, 379, 382, 388, 391
 * ("52802ce2 mov"@0xffffff8008a5b9a0, "52802f62 mov"@0xffffff8008a5b9c4,
 * "52802fc2 mov"@0xffffff8008a5ba44, "52803082 mov"@0xffffff8008a5ba08,
 * "528030e2 mov"@0xffffff8008a5ba20).
 *
 * IL COMANDO STA SULLA PILA E IL SUO INDIRIZZO E' PRESO: la funzione legge
 * la guardia in prologo ("f9454508 ldr"@0xffffff8008a5b954,
 * "f90007e8 str"@0xffffff8008a5b95c) e la riconfronta in epilogo
 * ("eb08013f cmp"@0xffffff8008a5ba60) saltando a `__stack_chk_fail`
 * ("97d949f9 bl"@0xffffff8008a5ba7c).
 * ATTENZIONE, E' UN'AFFERMAZIONE PIU' DEBOLE DI QUANTO SEMBRI:
 * `-fstack-protector-strong` accende quel codice sia per gli ARRAY su pila
 * sia per QUALUNQUE locale di cui si prenda l'indirizzo. Il protettore
 * quindi NON decide fra `u8 cmd[2]` e `u16 cmd` -- decide solo che
 * l'oggetto sta sulla pila e che il suo indirizzo esce dalla funzione.
 * Che siano DUE BYTE lo dice la lunghezza passata a `idev->c776`
 * ("321f03e1 orr"@0xffffff8008a5b9d8). La forma `u8[2]` e' una SCELTA fra
 * due che danno lo stesso codice. I due byte sono azzerati all'ingresso
 * ("79000bff strh"@0xffffff8008a5b964) e riempiti solo nel ramo `case 3`
 * ("52815ec8 mov"@0xffffff8008a5b9b8 mette 0x0af6,
 * "79000be8 strh"@0xffffff8008a5b9c8 lo scrive): 0xf6 al byte 0 e 0x0a al
 * byte 1, perche' l'AArch64 e' little-endian e il messaggio accanto dice
 * "0xF6 0x0A".
 * CHE la fabbrica avesse scritto due assegnazioni a byte o una sola a 16 bit
 * NON e' deciso dal binario: clang fonde le due in una `strh`. Qui e' resa
 * come due byte, che e' la forma coerente col buffer passato a
 * `idev->c776(buf, 2)`.
 *
 * `idev->c637` e' azzerato su TUTTI i rami tranne l'uscita anticipata
 * ("3909f51f strb"@0xffffff8008a5ba50, raggiunta da 0xffffff8008a5ba4c; il
 * ramo di riga 359 salta oltre, a 0xffffff8008a5ba54).
 */
int ilitek_tddi_proximity_far(int mode)
{
	int ret = 0;
	u8 cmd[2] = {0};

	/* "3949f508 ldrb"@0xffffff8008a5b968 + "34000128 cbz"@0xffffff8008a5b96c */
	if (!idev->c637) {
		/* "\x016ILITEK: (%s, %d): No proximity near event, break\n"@0xffffff8009241fef,
		 * riga 359 */
#line 359
		ILI_INFO("No proximity near event, break\n");
		return 0;
	}

	switch (mode) {
	case 2:
		/* "97ffe155 bl"@0xffffff8008a5b984 -- il ritorno NON e' usato:
		 * "2a1f03f3 mov"@0xffffff8008a5b988 azzera `ret` subito dopo */
		ilitek_tddi_gesture_recovery();
		break;
	case 3:
		/* "79000be8 strh"@0xffffff8008a5b9c8 -- 0x0af6 in due byte */
		cmd[0] = 0xf6;
		cmd[1] = 0x0a;
		/* "\x016ILITEK: (%s, %d): write prepare gesture command 0xF6 0x0A\n"@0xffffff800924203d,
		 * riga 379 */
#line 379
		ILI_INFO("write prepare gesture command 0xF6 0x0A\n");
		/* "f9418508 ldr"@0xffffff8008a5b9dc (idev+776) +
		 * "d63f0100 blr"@0xffffff8008a5b9e0, w1 = 2
		 * ("321f03e1 orr"@0xffffff8008a5b9d8) */
		ret = idev->c776(cmd, 2);
		/* "37f80260 tbnz"@0xffffff8008a5b9e4 */
		if (ret < 0) {
			/* "\x016ILITEK: (%s, %d): write prepare gesture command error\n"@0xffffff800924207a,
			 * riga 382 -- KERN_INFO, non KERN_ERR, anche se e' un
			 * ramo d'errore. E' cosi' nel binario. */
#line 382
			ILI_INFO("write prepare gesture command error\n");
			break;
		}
		/* "32000fe0 orr"@0xffffff8008a5b9e8 = 0xf, poi
		 * "97ffdfa8 bl"@0xffffff8008a5b9ec */
		ret = ilitek_tddi_switch_tp_mode(0x0f);
		/* "36f802c0 tbz"@0xffffff8008a5b9f4 */
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): Switch to gesture mode failed during proximity far\n"@0xffffff80092420b3,
			 * riga 388 */
#line 388
			ILI_ERR("Switch to gesture mode failed during proximity far\n");
		break;
	default:
		/* stesso letterale di `proximity_near`,
		 * "\x013ILITEK: (%s, %d): Unknown mode (%d)\n"@0xffffff8009241fc8,
		 * riga 391 */
#line 391
		ILI_ERR("Unknown mode (%d)\n", mode);
		/* "128002b3 mov"@0xffffff8008a5ba28 */
		ret = -EINVAL;
		break;
	}

	/* "3909f51f strb"@0xffffff8008a5ba50 -- idev+637 = 0 */
	idev->c637 = 0;
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_move_gesture_code_flash -- 0xffffff8008a5ba80, 72 byte
 * ===========================================================================
 * Riga di fabbrica: 405 ("528032a2 mov"@0xffffff8008a5baa8).
 *
 * IL PARAMETRO NON E' USATO. La funzione riceve w0 e non lo tocca mai:
 * il primo uso di un registro d'ingresso e' assente, e il valore stampato e
 * quello passato oltre vengono entrambi da `idev->c536`
 * ("b9421903 ldr"@0xffffff8008a5baa0 per la `printk`,
 * "b9421900 ldr"@0xffffff8008a5bab4 per la chiamata). Che il parametro
 * ESISTA lo dice il gruppo B, che assegna questa funzione a `idev->c816`,
 * dichiarato `int (*)(int mode)` in `ilitek.h`.
 *
 * `idev` e' RILETTO dopo la `printk` ("f9452268 ldr"@0xffffff8008a5bab0):
 * il puntatore non e' stato messo in un locale, altrimenti clang lo avrebbe
 * tenuto in un registro salvato. x19 tiene la PAGINA, non il puntatore.
 */
int ilitek_tddi_move_gesture_code_flash(int mode)
{
	/* "\x016ILITEK: (%s, %d): Switch to Gesture mode, lpwg cmd = %d\n"@0xffffff80092420fb,
	 * __func__ "ilitek_tddi_move_gesture_code_flash"@0xffffff8009242136,
	 * riga 405, w3 = idev->c536 */
#line 405
	ILI_INFO("Switch to Gesture mode, lpwg cmd = %d\n", idev->c536);

	/* "97ffe072 bl"@0xffffff8008a5bab8 */
	return ilitek_set_tp_data_len(idev->c536);
}

/*
 * ===========================================================================
 * ilitek_tddi_move_mp_code_iram -- 0xffffff8008a5b864, 48 byte
 * ===========================================================================
 * Riga di fabbrica: 322 ("52802842 mov"@0xffffff8008a5b87c).
 *
 * D1: "aa1f03e0 mov"@0xffffff8008a5b884 azzera x0 PRIMA del
 * "97ffe07c bl"@0xffffff8008a5b888. Il gruppo A non lo fa mai.
 */
int ilitek_tddi_move_mp_code_iram(void)
{
	/* "\x016ILITEK: (%s, %d): Download MP code to iram\n"@0xffffff8009241f32,
	 * __func__ "ilitek_tddi_move_mp_code_iram"@0xffffff8009241f60, riga 322 */
#line 322
	ILI_INFO("Download MP code to iram\n");

	/* "97ffe07c bl"@0xffffff8008a5b888 */
	return ilitek_tddi_fw_upgrade_handler_arg(NULL);
}

/*
 * Gruppo H. "9400521d bl"@0xffffff8008a5ccf8: x0 e' il buffer, w1 la
 * lunghezza, il ritorno non e' usato.
 */
void netlink_reply_msg(void *dato, int len);

/*
 * ===========================================================================
 * L'ARRAY STATICO DEI PUNTI -- 0xffffff800a0fcaa0, 10 elementi da 8 byte
 * ===========================================================================
 * NOME SCELTO: il binario non lo nomina. La `static`-ita' invece e' MISURATA,
 * non scelta: la scansione di tutto il testo del kernel
 * (`scout-ilitek-E/confine_rodata.txt`) trova DUE SOLE funzioni che tocchino
 * [0xffffff800a0fcaa0, 0xffffff800a0fcb00), e sono entrambe di questo blocco.
 *
 * DIECI elementi, e lo dicono due misure che concordano: l'azzeramento
 * copre 80 byte con cinque `stp xzr, xzr`
 * ("a9047f5f stp"@0xffffff8008a5c6d0 e le quattro seguenti fino a
 * "a9007f5f stp"@0xffffff8008a5c6e0), e il ciclo conta fino a dieci
 * ("f1002aff cmp"@0xffffff8008a5c7d0). 80 / 8 = 10.
 *
 * OTTO byte per elemento, e lo dice il passo dell'indice
 * ("8b090f49 add"@0xffffff8008a5c740, `lsl #3`).
 *
 * I NOMI DEI CAMPI restano `c<offset>`: il binario non li nomina
 * DIRETTAMENTE. Quello che il binario nomina sono gli ARGOMENTI di
 * `ilitek_tddi_touch_press`, e la catena che li lega ai quattro campi e'
 * misurata in due passi -- come si SCRIVE il campo qui e come si RILEGGE
 * per passarlo a `touch_press` -- ma resta una catena, non una stringa che
 * nomini il campo. Il commento accanto dice il nome che il formato usa.
 */
static struct {
	/* +0: scritto da "78296b57 strh"@0xffffff8008a5c78c (l'indice del
	 * ciclo), riletto da "785fc2c3 ldurh"@0xffffff8008a5c840 come QUARTO
	 * argomento di `touch_press`, che il formato chiama `id` */
	u16 c0;
	/* +2: scritto da "79000549 strh"@0xffffff8008a5c764, riletto da
	 * "785fe2c0 ldurh"@0xffffff8008a5c84c come PRIMO argomento, `x` */
	u16 c2;
	/* +4: scritto da "79000949 strh"@0xffffff8008a5c780, riletto da
	 * "794002c1 ldrh"@0xffffff8008a5c848 come SECONDO argomento, `y` */
	u16 c4;
	/* +6: scritto da "79000d49 strh"@0xffffff8008a5c79c, riletto da
	 * "794006c2 ldrh"@0xffffff8008a5c844 come TERZO argomento, `p` */
	u16 c6;
} punti[10];

/*
 * ===========================================================================
 * IL RECORD DELL'ANELLO DI `idev->c592` -- 16 byte
 * ===========================================================================
 * DELTA DI HEADER: `ilitek.h` dichiara `idev->c592` come `void *` perche' il
 * gruppo che l'ha scritto non aveva un sito che ne provasse la forma. Qui
 * il sito c'e', ed e' `ilitek_tddi_touch_send_debug_data`. Il tipo dovrebbe
 * finire in `ilitek.h`; io lo dichiaro localmente e non tocco l'header.
 *
 * SEDICI byte per record: "937c7c6a sbfiz"@0xffffff8008a5cd28 moltiplica
 * l'indice per 16 (`#4, #32`) e "8b161108 add"@0xffffff8008a5cd6c lo somma
 * con `lsl #4`.
 */
struct ilitek_record_debug {
	/* +0: un byte, letto da "386a690a ldrb"@0xffffff8008a5cd2c e messo a 1
	 * da "3829690a strb"@0xffffff8008a5cdac */
	u8 c0;
	/* +1..+7: nessuna istruzione del blocco li tocca. NON e' riempimento
	 * dichiarabile: e' un buco che il binario non descrive. */
	u8 __ignoto_1[7];
	/* +8: puntatore, "f9400500 ldr"@0xffffff8008a5cd70, destinazione di
	 * una `memcpy` */
	void *c8;
};

/*
 * ===========================================================================
 * ilitek_tddi_touch_send_debug_data -- 0xffffff8008a5ccbc, 288 byte, `static`
 * ===========================================================================
 * `static` DALLA MAPPA: `oracolo/stock.map` la marca `t` minuscolo. Non e'
 * una scelta.
 *
 * Riga di fabbrica: 677 ("528054a2 mov"@0xffffff8008a5cdd0).
 *
 * L'ANELLO E' LUNGO 1024, e lo dicono i due versi:
 *  - indietro: "320027ea orr"@0xffffff8008a5cd3c mette 0x3ff = 1023, e
 *    "1a890143 csel"@0xffffff8008a5cd40 lo sceglie quando l'indice e' zero;
 *  - avanti: "1110006b add"@0xffffff8008a5cd50 (indice + 1024),
 *    "1a83b56b csinc"@0xffffff8008a5cd58, "1216556b and"@0xffffff8008a5cd5c
 *    (`& ~1023`) e "4b0b014a sub"@0xffffff8008a5cd60. E' la forma con cui
 *    clang rende il RESTO CON SEGNO per una potenza di due: il bias 1023
 *    sommato solo quando il dividendo e' negativo.
 *
 * La copia e' limitata a 2048 byte, con confronto CON SEGNO
 * ("7120029f cmp"@0xffffff8008a5cd78 + "1a88b288 csel"@0xffffff8008a5cd80,
 * condizione `lt`), e la taglia passa a `memcpy` ESTESA CON SEGNO
 * ("93407d02 sxtw"@0xffffff8008a5cd84): il minimo e' fra due `int`.
 *
 * "3948b128 ldrb"@0xffffff8008a5cd18 legge `idev->c556` DOPO `idev->c530`
 * e sullo STESSO `x9` caricato una volta sola
 * ("f94522a9 ldr"@0xffffff8008a5cce4): `idev` non e' riletto fra i due.
 */
static void ilitek_tddi_touch_send_debug_data(void *dato, int len)
{
	struct ilitek_record_debug *anello;
	int i;

	/* "91024100 add"@0xffffff8008a5ccdc (idev + 0x90 = 144) +
	 * "941059a7 bl"@0xffffff8008a5cce0 */
	mutex_lock(&idev->debug_mutex);

	/* "39484928 ldrb"@0xffffff8008a5cce8 + "34000168 cbz"@0xffffff8008a5ccec */
	if (idev->c530) {
		/* "9400521d bl"@0xffffff8008a5ccf8 */
		netlink_reply_msg(dato, len);
		goto fine;
	}

	/* "3948b128 ldrb"@0xffffff8008a5cd18 + "34ffff08 cbz"@0xffffff8008a5cd1c */
	if (!idev->c556)
		goto fine;

	/* "b9423123 ldr"@0xffffff8008a5cd20 */
	i = idev->c560;
	/* "f9412928 ldr"@0xffffff8008a5cd24 */
	anello = idev->c592;

	/* "386a690a ldrb"@0xffffff8008a5cd2c + "340000ea cbz"@0xffffff8008a5cd30 */
	if (anello[i].c0) {
		/* "1a890143 csel"@0xffffff8008a5cd40 -- indietro, circolare */
		i = (i == 0) ? 1023 : i - 1;
	} else {
		/* "b902312a str"@0xffffff8008a5cd68 -- avanti, circolare;
		 * `i` NON cambia, e' solo l'indice memorizzato che avanza */
		idev->c560 = (i + 1) % 1024;
	}

	/* "f9400500 ldr"@0xffffff8008a5cd70 + "b4000260 cbz"@0xffffff8008a5cd74 */
	if (!anello[i].c8) {
		/* "\x016ILITEK: (%s, %d): BUFFER %d error\n"@0xffffff800924305c,
		 * __func__ "ilitek_tddi_touch_send_debug_data"@0xffffff8009243081,
		 * riga 677. L'argomento e' `i`: il binario NON lo ricarica in w3
		 * perche' w3 lo contiene gia' -- e' l'indice appena calcolato. */
#line 677
		ILI_INFO("BUFFER %d error\n", i);
		goto fine;
	}

	/* "940febdd bl"@0xffffff8008a5cd8c */
	memcpy(anello[i].c8, dato, (len < 2048) ? len : 2048);
	/* "f94522a8 ldr"@0xffffff8008a5cd90 (rilettura di `idev`),
	 * "f9412908 ldr"@0xffffff8008a5cda0 (rilettura di `idev->c592`) e
	 * "3829690a strb"@0xffffff8008a5cdac */
	((struct ilitek_record_debug *)idev->c592)[i].c0 = 1;
	/* "9108e100 add"@0xffffff8008a5cdb4 (idev + 0x238 = 568) +
	 * "97db0172 bl"@0xffffff8008a5cdb8, con w1 = 3 = TASK_NORMAL
	 * ("320007e1 orr"@0xffffff8008a5cd9c), w2 = 1
	 * ("320003e2 orr"@0xffffff8008a5cda4) e x3 = NULL
	 * ("aa1f03e3 mov"@0xffffff8008a5cda8): e' `wake_up`, che e' una macro */
	wake_up(&idev->inq);

fine:
	/* "941059b2 bl"@0xffffff8008a5cd04 */
	mutex_unlock(&idev->debug_mutex);
}

/*
 * ===========================================================================
 * ilitek_tddi_report_ap_mode -- 0xffffff8008a5c680, 1012 byte
 * ===========================================================================
 * Righe di fabbrica: 781 ("528061a2 mov"@0xffffff8008a5c7a4), 787
 * ("52806262 mov"@0xffffff8008a5c804) e la 717 della `touch_release`
 * incorporata ("528059a2 mov"@0xffffff8008a5c8b8 e
 * "528059a2 mov"@0xffffff8008a5c988 -- DUE volte, vedi sotto).
 *
 * IL FORMATO DEL PACCHETTO, misurato dagli scostamenti: il punto `i` occupa
 * QUATTRO byte, da `dato[4*i+1]` a `dato[4*i+4]`. Il puntatore di lavoro
 * parte da `dato + 2` ("9100081b add"@0xffffff8008a5c6c0) e avanza di 4
 * ("9100137b add"@0xffffff8008a5c7d4), e i quattro accessi sono a -1, 0,
 * +1, +2 ("385ff369 ldurb"@0xffffff8008a5c6f8,
 * "39400363 ldrb"@0xffffff8008a5c720, "39400764 ldrb"@0xffffff8008a5c724,
 * "39400b69 ldrb"@0xffffff8008a5c794).
 *
 * LE COORDINATE si compongono con due `bfi` di QUATTRO bit ciascuno:
 * "33180d63 bfi"@0xffffff8008a5c730 mette in x i bit alti presi da
 * "53047d2b lsr"@0xffffff8008a5c728 (il nibble ALTO di `dato[4*i+1]`), e
 * "33180d24 bfi"@0xffffff8008a5c734 mette in y il nibble BASSO dello stesso
 * byte. I nomi `x` e `y` vengono dal binario: li nomina il formato
 * "original x = %d, y = %d".
 *
 * LA SCALA e' condizionata da `idev->c641`
 * ("394a050a ldrb"@0xffffff8008a5c72c + "340000ca cbz"@0xffffff8008a5c738):
 * se e' zero le coordinate passano per `* idev->c244 >> 11` e
 * `* idev->c246 >> 11` ("1b097c69 mul"@0xffffff8008a5c758 +
 * "530b7d29 lsr"@0xffffff8008a5c75c, e la coppia gemella a
 * 0xffffff8008a5c76c/0xffffff8008a5c770); se non lo e', passano crude.
 * Lo scorrimento e' LOGICO (`lsr`, non `asr`).
 *
 * IL CICLO DI RILASCIO E' SCRITTO DUE VOLTE, ed e' cosi' nel binario: una
 * volta a 0xffffff8008a5c898 e una a 0xffffff8008a5c968, con le stesse
 * istruzioni e lo stesso `__func__` incorporato. Che la fabbrica avesse
 * duplicato il ciclo nel sorgente o l'avesse messo in una funzione `static`
 * poi incorporata due volte NON e' deciso dal binario: una `static`
 * completamente incorporata non lascia simbolo. Qui e' duplicato, che e' la
 * forma che riproduce il binario; la non-unicita' e' dichiarata.
 *
 * `idev->c396[i] = idev->c356[i]` vale su TUTTI i rami: il ramo che rilascia
 * ricarica `idev->c356[i]` dopo le due chiamate
 * ("b9416529 ldr"@0xffffff8008a5c908) e vi trova zero, perche' e' il ramo in
 * cui quel valore E' zero.
 */
void ilitek_tddi_report_ap_mode(u8 *dato, int len)
{
	int i;
	u16 x, y;

	/* "a9047f5f stp"@0xffffff8008a5c6d0 e le quattro seguenti: 80 byte */
	memset(punti, 0x0, sizeof(punti));
	/* "b901611f str"@0xffffff8008a5c6e4 */
	idev->c352 = 0;

	for (i = 0; i < 10; i++) {
		/* "7103fd3f cmp"@0xffffff8008a5c6fc,
		 * "7103fd1f cmp"@0xffffff8008a5c708,
		 * "7103fd1f cmp"@0xffffff8008a5c714 */
		if (dato[(4 * i) + 1] == 0xff && dato[(4 * i) + 2] == 0xff &&
		    dato[(4 * i) + 3] == 0xff) {
			/* "b9016509 str"@0xffffff8008a5c7d8 con w9 = 0
			 * ("2a1f03e9 mov"@0xffffff8008a5c6f0) */
			idev->c356[i] = 0;
			continue;
		}

		/* "33180d63 bfi"@0xffffff8008a5c730 */
		x = ((dato[(4 * i) + 1] & 0xf0) << 4) | dato[(4 * i) + 2];
		/* "33180d24 bfi"@0xffffff8008a5c734 */
		y = ((dato[(4 * i) + 1] & 0x0f) << 8) | dato[(4 * i) + 3];

		/* "340000ca cbz"@0xffffff8008a5c738 */
		if (idev->c641) {
			/* "79000523 strh"@0xffffff8008a5c744 */
			punti[idev->c352].c2 = x;
			punti[idev->c352].c4 = y;
		} else {
			/* "7941e909 ldrh"@0xffffff8008a5c750 (idev+244) */
			punti[idev->c352].c2 = x * idev->c244 >> 11;
			/* "7941ed09 ldrh"@0xffffff8008a5c768 (idev+246) */
			punti[idev->c352].c4 = y * idev->c246 >> 11;
		}
		/* "78296b57 strh"@0xffffff8008a5c78c */
		punti[idev->c352].c0 = i;
		/* "79000d49 strh"@0xffffff8008a5c79c */
		punti[idev->c352].c6 = dato[(4 * i) + 4];

		/* "\x016ILITEK: (%s, %d): original x = %d, y = %d\n"@0xffffff8009242b17,
		 * __func__ "ilitek_tddi_report_ap_mode"@0xffffff8009242b44,
		 * riga 781, guardia "340000cb cbz"@0xffffff8008a5c7a0 */
#line 781
		ILI_DBG("original x = %d, y = %d\n", x, y);

		/* "11000529 add"@0xffffff8008a5c7bc */
		idev->c352++;
		/* "b9016509 str"@0xffffff8008a5c7d8 con w9 = 1
		 * ("320003e9 orr"@0xffffff8008a5c7c4) */
		idev->c356[i] = 1;
	}

	/* "\x016ILITEK: (%s, %d): figner number = %d, LastTouch = %d\n"@0xffffff8009242b5f,
	 * riga 787, guardia "34000148 cbz"@0xffffff8008a5c7e4.
	 * "figner" e' un refuso DELLA FABBRICA e si riproduce (regola 7).
	 * Il formato NOMINA `idev->c436`: `LastTouch`. Vedi il delta di header. */
#line 787
	ILI_DBG("figner number = %d, LastTouch = %d\n", idev->c352, idev->c436);

	/* "340009c9 cbz"@0xffffff8008a5c814 */
	if (idev->c352) {
		/* "7100053f cmp"@0xffffff8008a5c818 +
		 * "5400034b b.lt"@0xffffff8008a5c81c -- la guardia del ciclo e'
		 * SEPARATA dal test qui sopra perche' `c352` e' con segno */
		for (i = 0; i < idev->c352; i++) {
			/* "97ffa7f1 bl"@0xffffff8008a5c83c */
			input_report_key(idev->c16, BTN_TOUCH, 1);
			/* "94000089 bl"@0xffffff8008a5c850 -- una `bl` VERA:
			 * `touch_press` NON e' incorporata. E' il controllo
			 * negativo di D5. */
			ilitek_tddi_touch_press(punti[i].c2, punti[i].c4,
						punti[i].c6, punti[i].c0);
			/* "97ffa7e6 bl"@0xffffff8008a5c868 */
			input_report_key(idev->c16, BTN_TOOL_FINGER, 1);
		}

		for (i = 0; i < 10; i++) {
			/* "b9416549 ldr"@0xffffff8008a5c89c +
			 * "350003a9 cbnz"@0xffffff8008a5c8a0, e
			 * "b9418d49 ldr"@0xffffff8008a5c8a4 +
			 * "7100053f cmp"@0xffffff8008a5c8a8 */
			if (idev->c356[i] == 0 && idev->c396[i] == 1)
				ilitek_tddi_touch_release(0, 0, i);
			/* "b9018d09 str"@0xffffff8008a5c918 */
			idev->c396[i] = idev->c356[i];
		}
		/* "97ffa7b1 bl"@0xffffff8008a5c93c */
		input_sync(idev->c16);
		/* "b901b509 str"@0xffffff8008a5ca48 con w9 = idev->c352
		 * ("b9416109 ldr"@0xffffff8008a5c944) */
		idev->c436 = idev->c352;
	} else if (idev->c436) {
		/* "b941b509 ldr"@0xffffff8008a5c94c +
		 * "340007e9 cbz"@0xffffff8008a5c950 */
		for (i = 0; i < 10; i++) {
			/* "b9416549 ldr"@0xffffff8008a5c96c +
			 * "350003a9 cbnz"@0xffffff8008a5c970 */
			if (idev->c356[i] == 0 && idev->c396[i] == 1)
				ilitek_tddi_touch_release(0, 0, i);
			/* "b9018d09 str"@0xffffff8008a5c9e8 */
			idev->c396[i] = idev->c356[i];
		}
		/* "97ffa77d bl"@0xffffff8008a5ca0c */
		input_report_key(idev->c16, BTN_TOUCH, 0);
		/* "97ffa777 bl"@0xffffff8008a5ca24 */
		input_report_key(idev->c16, BTN_TOOL_FINGER, 0);
		/* "97ffa771 bl"@0xffffff8008a5ca3c */
		input_sync(idev->c16);
		/* "b901b509 str"@0xffffff8008a5ca48 con w9 = 0
		 * ("2a1f03e9 mov"@0xffffff8008a5ca44) */
		idev->c436 = 0;
	}

	/* "9400009a bl"@0xffffff8008a5ca54 */
	ilitek_tddi_touch_send_debug_data(dato, len);
}

/*
 * ===========================================================================
 * demo_debug_info_mode -- 0xffffff8008a5c610, 112 byte
 * ===========================================================================
 * Riga di fabbrica: 646 ("528050c2 mov"@0xffffff8008a5c644).
 *
 * IL SECONDO PARAMETRO NON E' USATO: "52800561 mov"@0xffffff8008a5c620
 * scrive 43 in w1 prima di qualunque lettura. Che ESISTA lo dice il sito di
 * chiamata del gruppo A, "93407e61 sxtw"@0xffffff8008a54dfc, che lo passa
 * ESTESO CON SEGNO A 64 BIT -- e non e' come vengono passati gli altri tre
 * modi di rapporto, che ricevono "2a1303e1 mov"@0xffffff8008a54df0 (32 bit).
 * CORREZIONE DEL 2026-08-21, MISURATA: il lotto precedente aveva scritto qui
 * che il secondo parametro di `demo_debug_info_id0` e' invece un `int`,
 * deducendolo da "93407e89 sxtw"@0xffffff8008a5c334 («con un parametro gia'
 * a 64 bit l'estensione non servirebbe»). E' FALSO, e lo dice il compilatore
 * di fabbrica: con un parametro `int` r353983c emette `mov w20, w1` e mette
 * l'estensione DOPO la scelta (`csel w`, poi `sxtw x2, w`); la fabbrica ha
 * "aa0103f4 mov"@0xffffff8008a5c330, cioe' la copia INTERA di x1, e la
 * scelta a 64 bit "9a8ab122 csel"@0xffffff8008a5c344. Quella forma la
 * produce SOLO un parametro a 64 bit troncato a `int` nel corpo. Le due
 * funzioni hanno percio' la STESSA firma e possono stare nella stessa
 * tabella: il delta di header 4 cade.
 *
 * La lettura dei due byte avviene DOPO la chiamata a
 * `ilitek_tddi_report_ap_mode` ("3842ce74 ldrb"@0xffffff8008a5c62c e
 * "385ff275 ldurb"@0xffffff8008a5c63c stanno dopo
 * "94000016 bl"@0xffffff8008a5c628): il sorgente non li ha copiati prima.
 */
void demo_debug_info_mode(u8 *dato, long len)
{
	void (*f)(void *, unsigned long);
	u8 c43, c44;

	/* "52800561 mov"@0xffffff8008a5c620 (43) +
	 * "94000016 bl"@0xffffff8008a5c628 */
	ilitek_tddi_report_ap_mode(dato, 43);

	/*
	 * I DUE BYTE FINISCONO IN LOCALI, e non e' stile: il binario li tiene
	 * in x20 e x21 -- registri SALVATI DAL CHIAMATO -- attraverso la
	 * `printk` ("2a1403e4 mov"@0xffffff8008a5c648,
	 * "2a1503e3 mov"@0xffffff8008a5c64c per la stampa, e poi
	 * "aa1503e1 mov"@0xffffff8008a5c660 e
	 * "8b140d08 add"@0xffffff8008a5c664 per la chiamata indiretta) e non
	 * li rilegge mai. Rileggendoli da `dato` la funzione misura 100 byte
	 * contro 112: due `ldrb` in piu' e tre `mov`/`stp` in meno.
	 * E' la classe A5 di `criteri-revisione.md`, al rovescio.
	 */
	/* "3842ce74 ldrb"@0xffffff8008a5c62c -- pre-indice con riscrittura:
	 * x19 diventa `dato + 44` e w20 prende `dato[44]` */
	c44 = dato[44];
	/* "385ff275 ldurb"@0xffffff8008a5c63c -- `dato + 44 - 1` */
	c43 = dato[43];

	/* "\x016ILITEK: (%s, %d): info len = %d ,id = %d\n"@0xffffff8009242a26,
	 * __func__ "demo_debug_info_mode"@0xffffff8009242a52, riga 646.
	 * NON e' guardata dal diagnostico. */
#line 646
	ILI_INFO("info len = %d ,id = %d\n", c43, c44);

	/* "8b140d08 add"@0xffffff8008a5c664 (`lsl #3`: puntatori a 8 byte) +
	 * "f941a908 ldr"@0xffffff8008a5c668 (idev+848) +
	 * "d63f0100 blr"@0xffffff8008a5c66c */
	f = idev->c848[c44];
	f(&dato[44], c43);
}

/*
 * ===========================================================================
 * IL SECONDO ARRAY STATICO -- 0xffffff800a0fcaf0, byte, passo 1
 * ===========================================================================
 * Sta esattamente a `punti + 80`, cioe' subito dopo il primo, ed e' un
 * OGGETTO SEPARATO: il binario lo materializza con la sua `add`
 * ("912bc39c add"@0xffffff8008a5ce24) invece di indicizzare il primo.
 *
 * `static` MISURATO: solo `ilitek_tddi_report_debug_mode` lo tocca, in tutto
 * il testo del kernel.
 *
 * DEL NUMERO DI ELEMENTI il binario dice solo che l'indice arriva a 9 (il
 * ciclo conta dieci): ALMENO DIECI. Non c'e' nessun confronto con un limite.
 * Il 10 qui e' quindi la lettura piu' stretta compatibile, non una misura
 * dell'estensione.
 *
 * NON E' AZZERATO all'ingresso -- nessuna istruzione lo azzera -- quindi il
 * suo contenuto sopravvive fra una chiamata e l'altra. NOME SCELTO.
 */
static u8 alternanza[10];

/*
 * ===========================================================================
 * ilitek_tddi_report_debug_mode -- 0xffffff8008a5cddc, 1040 byte
 * ===========================================================================
 * Righe di fabbrica: 871 ("52806ce2 mov"@0xffffff8008a5cf18), 877
 * ("52806da2 mov"@0xffffff8008a5cf78), piu' la 717 della `touch_release`
 * incorporata due volte.
 *
 * E' `ilitek_tddi_report_ap_mode` con DUE differenze misurate, e nient'altro:
 *
 * 1. IL PASSO DEL PACCHETTO E' TRE, non quattro
 *    ("91000f7b add"@0xffffff8008a5cf48), e la base parte da `dato + 7`
 *    ("91001c1b add"@0xffffff8008a5ce20): il punto `i` sta in
 *    `dato[3*i+5]`, `dato[3*i+6]`, `dato[3*i+7]`
 *    ("385fe369 ldurb"@0xffffff8008a5ce64,
 *    "385ff363 ldurb"@0xffffff8008a5ce68,
 *    "39400368 ldrb"@0xffffff8008a5ce78). Non c'e' un quarto byte: la
 *    pressione viene dal punto 2.
 *    I primi DUE confronti con 0xff sono FUSI IN UNO da clang
 *    ("0a030128 and"@0xffffff8008a5ce6c + "7103fd1f cmp"@0xffffff8008a5ce70):
 *    per due byte, `(a & b) == 0xff` equivale a `a == 0xff && b == 0xff`.
 *    E "32001fe3 orr"@0xffffff8008a5ce84 rimette 0xff in w3 sul ramo in cui
 *    il compilatore SA che quel byte vale 0xff.
 *
 * 2. LA PRESSIONE NON VIENE DAL PACCHETTO ma dall'array `alternanza`, e
 *    alterna 1 e 2 per ciascun dito:
 *    "38696b8a ldrb"@0xffffff8008a5cefc legge, "7100055f cmp"@0xffffff8008a5cf04
 *    confronta con 1, "1a93166a cinc"@0xffffff8008a5cf08 produce `1 + (x==1)`
 *    (w19 vale 1, "320003f3 orr"@0xffffff8008a5ce28), e il risultato va sia
 *    nell'array ("38296b8a strb"@0xffffff8008a5cf0c) sia nel campo `c6`
 *    ("79000d8a strh"@0xffffff8008a5cf10). Una sola lettura, due scritture.
 *
 * `len` e' RIVERSATO SULLA PILA ("b9000fe1 str"@0xffffff8008a5ce38) e
 * riletto solo alla fine ("b9400fe1 ldr"@0xffffff8008a5d1c0): e' pressione
 * di registri, non una scelta del sorgente.
 */
void ilitek_tddi_report_debug_mode(u8 *dato, int len)
{
	int i;
	u16 x, y;
	u8 pr;

	/* "a9047f5f stp"@0xffffff8008a5ce3c e le quattro seguenti */
	memset(punti, 0x0, sizeof(punti));
	/* "b901611f str"@0xffffff8008a5ce50 */
	idev->c352 = 0;

	for (i = 0; i < 10; i++) {
		/* "7103fd1f cmp"@0xffffff8008a5ce70 (i primi due, fusi) e
		 * "7103fd1f cmp"@0xffffff8008a5ce7c (il terzo) */
		if (dato[(3 * i) + 5] == 0xff && dato[(3 * i) + 6] == 0xff &&
		    dato[(3 * i) + 7] == 0xff) {
			/* "b9016509 str"@0xffffff8008a5cf4c con w9 = 0
			 * ("2a1f03e9 mov"@0xffffff8008a5ce5c) */
			idev->c356[i] = 0;
			continue;
		}

		/* "33180d63 bfi"@0xffffff8008a5ce94 */
		x = ((dato[(3 * i) + 5] & 0xf0) << 4) | dato[(3 * i) + 6];
		/* "33180d24 bfi"@0xffffff8008a5ce9c */
		y = ((dato[(3 * i) + 5] & 0x0f) << 8) | dato[(3 * i) + 7];

		/* "340000ca cbz"@0xffffff8008a5cea0 */
		if (idev->c641) {
			/* "79000523 strh"@0xffffff8008a5ceac */
			punti[idev->c352].c2 = x;
			punti[idev->c352].c4 = y;
		} else {
			/* "7941e909 ldrh"@0xffffff8008a5ceb8 (idev+244) */
			punti[idev->c352].c2 = x * idev->c244 >> 11;
			/* "7941ed09 ldrh"@0xffffff8008a5ced0 (idev+246) */
			punti[idev->c352].c4 = y * idev->c246 >> 11;
		}
		/* "78296b57 strh"@0xffffff8008a5cef4 */
		punti[idev->c352].c0 = i;

		/* "1a93166a cinc"@0xffffff8008a5cf08 */
		pr = (alternanza[idev->c352] == 1) ? 2 : 1;
		/* "38296b8a strb"@0xffffff8008a5cf0c */
		alternanza[idev->c352] = pr;
		/* "79000d8a strh"@0xffffff8008a5cf10 */
		punti[idev->c352].c6 = pr;

		/* stesso letterale di `report_ap_mode`,
		 * "\x016ILITEK: (%s, %d): original x = %d, y = %d\n"@0xffffff8009242b17,
		 * __func__ "ilitek_tddi_report_debug_mode"@0xffffff8009242b97,
		 * riga 871, guardia "340000cb cbz"@0xffffff8008a5cf14 */
#line 871
		ILI_DBG("original x = %d, y = %d\n", x, y);

		/* "11000529 add"@0xffffff8008a5cf30 */
		idev->c352++;
		/* "b9016509 str"@0xffffff8008a5cf4c con w9 = 1
		 * ("320003e9 orr"@0xffffff8008a5cf38) */
		idev->c356[i] = 1;
	}

	/* riga 877, guardia "34000148 cbz"@0xffffff8008a5cf58 */
#line 877
	ILI_DBG("figner number = %d, LastTouch = %d\n", idev->c352, idev->c436);

	/* "340009c9 cbz"@0xffffff8008a5cf88 */
	if (idev->c352) {
		/* "5400034b b.lt"@0xffffff8008a5cf90 */
		for (i = 0; i < idev->c352; i++) {
			/* "97ffa614 bl"@0xffffff8008a5cfb0 */
			input_report_key(idev->c16, BTN_TOUCH, 1);
			/* "97fffeac bl"@0xffffff8008a5cfc4 -- `bl` VERA */
			ilitek_tddi_touch_press(punti[i].c2, punti[i].c4,
						punti[i].c6, punti[i].c0);
			/* "97ffa609 bl"@0xffffff8008a5cfdc */
			input_report_key(idev->c16, BTN_TOOL_FINGER, 1);
		}

		for (i = 0; i < 10; i++) {
			/* "350003a9 cbnz"@0xffffff8008a5d014 +
			 * "7100053f cmp"@0xffffff8008a5d01c */
			if (idev->c356[i] == 0 && idev->c396[i] == 1)
				ilitek_tddi_touch_release(0, 0, i);
			/* "b9018d09 str"@0xffffff8008a5d08c */
			idev->c396[i] = idev->c356[i];
		}
		/* "97ffa5d4 bl"@0xffffff8008a5d0b0 */
		input_sync(idev->c16);
		/* "b901b509 str"@0xffffff8008a5d1bc */
		idev->c436 = idev->c352;
	} else if (idev->c436) {
		/* "340007e9 cbz"@0xffffff8008a5d0c4 */
		for (i = 0; i < 10; i++) {
			/* "350003a9 cbnz"@0xffffff8008a5d0e4 */
			if (idev->c356[i] == 0 && idev->c396[i] == 1)
				ilitek_tddi_touch_release(0, 0, i);
			/* "b9018d09 str"@0xffffff8008a5d15c */
			idev->c396[i] = idev->c356[i];
		}
		/* "97ffa5a0 bl"@0xffffff8008a5d180 */
		input_report_key(idev->c16, BTN_TOUCH, 0);
		/* "97ffa59a bl"@0xffffff8008a5d198 */
		input_report_key(idev->c16, BTN_TOOL_FINGER, 0);
		/* "97ffa594 bl"@0xffffff8008a5d1b0 */
		input_sync(idev->c16);
		/* "b901b509 str"@0xffffff8008a5d1bc con w9 = 0
		 * ("2a1f03e9 mov"@0xffffff8008a5d1b8) */
		idev->c436 = 0;
	}

	/* "97fffebd bl"@0xffffff8008a5d1c8 */
	ilitek_tddi_touch_send_debug_data(dato, len);
}

/*
 * ===========================================================================
 * ilitek_tddi_report_i2cuart_mode -- 0xffffff8008a5d804, 568 byte
 * ===========================================================================
 * Righe di fabbrica: 1071, 1084, 1092, 1096, 1101, 1107
 * ("528085e2 mov"@0xffffff8008a5d848, "52808782 mov"@0xffffff8008a5d8a0,
 * "52808882 mov"@0xffffff8008a5d8dc, "52808902 mov"@0xffffff8008a5d9b0,
 * "528089a2 mov"@0xffffff8008a5d9e8, "52808a62 mov"@0xffffff8008a5da08).
 * La riga 1084 e' quella che SCONTRA con `ilitek_tddi_ic_get_protocl_ver`
 * del blocco vicino: e' la terza prova del confine.
 *
 * QUI I NOMI VENGONO DAL BINARIO. Sono le stringhe di formato a nominarli:
 * `type` e `actual_len` da "data[3] = %x, type = %x, actual_len = %d",
 * `need_read_len` e `one_data_bytes` da
 * "need_read_len = %d  one_data_bytes = %d" (due spazi, come in fabbrica),
 * `uart len` da "uart len = %d", `uart_buf` e `total_buf` dai due messaggi
 * di allocazione fallita.
 *
 * LA SCELTA DI `one_data_bytes` E' UNO `switch`, e lo prova la forma con cui
 * clang lo abbassa: "1ad62089 lsl"@0xffffff8008a5d870 costruisce `1 << type`
 * e "6a0a013f tst"@0xffffff8008a5d878 lo confronta con la maschera 0x43
 * ("5280086a mov"@0xffffff8008a5d874), cioe' i bit 0, 1 e 6. E' la prova a
 * bit che clang genera per un `switch` con casi sparsi, non per una catena
 * di `if`. Il controllo di intervallo che la precede e'
 * "71001adf cmp"@0xffffff8008a5d860 + "54000928 b.hi"@0xffffff8008a5d868.
 * Gli altri due gruppi sono "320002c9 orr"@0xffffff8008a5d98c
 * (`type | 1 == 3`, cioe' i casi 2 e 3) e
 * "7100153f cmp"@0xffffff8008a5d9c8 con
 * "1a9f17e9 cset"@0xffffff8008a5d9cc e "531e7524 lsl"@0xffffff8008a5d9d0
 * (`type | 1 == 5`, i casi 4 e 5, valore 4; zero altrimenti).
 *
 * IL DIFETTO DELLA FABBRICA CHE SI RIPRODUCE (regola 7): i due messaggi di
 * allocazione fallita stampano `%ld` e ricevono IL PUNTATORE
 * ("aa1503e3 mov"@0xffffff8008a5d9b4 e "aa1703e3 mov"@0xffffff8008a5da0c).
 * Non e' un errore di lettura mio: e' cosi' nel binario.
 *
 * IL CONTROLLO SUL PUNTATORE e' `IS_ERR` seguito da `== NULL`, in
 * quest'ordine: "b140041f cmn"@0xffffff8008a5d900 confronta con -4096 e
 * "540004e8 b.hi"@0xffffff8008a5d904 dirotta, poi
 * "b40004d5 cbz"@0xffffff8008a5d908. Sono due prove, non una: `IS_ERR_OR_NULL`
 * da solo produrrebbe le stesse due istruzioni, quindi la FORMA sorgente non
 * e' decisa dal binario -- il COMPORTAMENTO si'.
 */
void ilitek_tddi_report_i2cuart_mode(u8 *dato, int len)
{
	int type;
	int actual_len;
	int need_read_len;
	int one_data_bytes = 0;
	int uart_len;
	u8 *uart_buf = NULL;
	u8 *total_buf = NULL;
	int ret;

	/* "12000c76 and"@0xffffff8008a5d82c */
	type = dato[3] & 0x0f;
	/* "51001435 sub"@0xffffff8008a5d830 */
	actual_len = len - 5;

	/* "\x016ILITEK: (%s, %d): data[3] = %x, type = %x, actual_len = %d\n"@0xffffff8009242cf4,
	 * riga 1071, guardia "34000128 cbz"@0xffffff8008a5d834 */
	ILI_DBG("data[3] = %x, type = %x, actual_len = %d\n",
#line 1071
		dato[3], type, actual_len);

	/*
	 * IL PRODOTTO DEI DUE BYTE E' CALCOLATO PRIMA DELLA CATENA DI `if`,
	 * e non e' una scelta di stile: "39400668 ldrb"@0xffffff8008a5d858,
	 * "39400a69 ldrb"@0xffffff8008a5d85c e "1b087d28 mul"@0xffffff8008a5d864
	 * stanno nel blocco d'ingresso, PRIMA di "54000928 b.hi"@0xffffff8008a5d868.
	 * Scrivendo il prodotto dentro l'espressione finale
	 * (`dato[1] * dato[2] * one_data_bytes + 1`) questo stesso compilatore
	 * mette la `mul` nel blocco di ricongiungimento, dopo la catena: la
	 * dimensione resta 568 e cambiano ventinove parole. Misurato.
	 */
	need_read_len = dato[1] * dato[2];

	if (type == 0 || type == 1 || type == 6)
		/* "320003e4 orr"@0xffffff8008a5d86c */
		one_data_bytes = 1;
	else if (type == 2 || type == 3)
		/* "321f03e4 orr"@0xffffff8008a5d998 */
		one_data_bytes = 2;
	else if (type == 4 || type == 5)
		/* "531e7524 lsl"@0xffffff8008a5d9d0 */
		one_data_bytes = 4;

	/* "1b042916 madd"@0xffffff8008a5d888 */
	need_read_len = need_read_len * one_data_bytes + 1;

	/* "\x016ILITEK: (%s, %d): need_read_len = %d  one_data_bytes = %d\n"@0xffffff8009242d52,
	 * riga 1084, guardia "34000109 cbz"@0xffffff8008a5d88c */
	ILI_DBG("need_read_len = %d  one_data_bytes = %d\n",
#line 1084
		need_read_len, one_data_bytes);

	/* "6b1502d7 subs"@0xffffff8008a5d8ac + "540000aa b.ge"@0xffffff8008a5d8b0
	 * -- una sola istruzione per il confronto E la differenza */
	if (need_read_len < actual_len) {
		/* "97fffd00 bl"@0xffffff8008a5d8bc */
		ilitek_tddi_touch_send_debug_data(dato, len);
		return;
	}

	uart_len = need_read_len - actual_len;
	/* "\x016ILITEK: (%s, %d): uart len = %d\n"@0xffffff8009242d8f,
	 * riga 1092, guardia "34000108 cbz"@0xffffff8008a5d8c8 */
#line 1092
	ILI_DBG("uart len = %d\n", uart_len);

	/* "93407ef6 sxtw"@0xffffff8008a5d8e8 (la taglia e' un `int` esteso con
	 * segno) + "97dfcaa4 bl"@0xffffff8008a5d8f8 verso `__kmalloc`: la
	 * taglia NON e' costante, per cui `kmalloc` non si riduce a
	 * `kmem_cache_alloc`. Le bandiere sono
	 * "52901801 mov"@0xffffff8008a5d8ec + "72a02801 movk"@0xffffff8008a5d8f0,
	 * cioe' 0x14080c0. GFP_KERNEL in questo albero vale 0x14000c0 -- il
	 * bit in piu' e' 0x8000 = __GFP_ZERO, quindi la fabbrica chiama
	 * `kzalloc`, non `kmalloc`. E' una differenza di COMPORTAMENTO che la
	 * misura di dimensione NON denuncia: le due macro danno le stesse due
	 * istruzioni. */
	uart_buf = kzalloc(uart_len, GFP_KERNEL);
	if (IS_ERR(uart_buf) || uart_buf == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate uart_buf memory %ld\n"@0xffffff8009242db2,
		 * riga 1096, x3 = il puntatore
		 * ("aa1503e3 mov"@0xffffff8008a5d9b4) */
		ILI_ERR("Failed to allocate uart_buf memory %ld\n",
#line 1096
			PTR_ERR(uart_buf));
		/* "b4000375 cbz"@0xffffff8008a5d9bc */
		if (uart_buf)
			/* "97dfcb63 bl"@0xffffff8008a5da24 */
			kfree(uart_buf);
		return;
	}

	/* "f9418908 ldr"@0xffffff8008a5d91c (idev+784) +
	 * "d63f0100 blr"@0xffffff8008a5d920 */
	ret = idev->c784(uart_buf, uart_len);
	/* "37f805a0 tbnz"@0xffffff8008a5d924 */
	if (ret < 0) {
		/* "\x013ILITEK: (%s, %d): i2cuart read data failed\n"@0xffffff8009242dee,
		 * riga 1101 -- senza argomenti */
#line 1101
		ILI_ERR("i2cuart read data failed\n");
		/* "97dfcb63 bl"@0xffffff8008a5da24, senza prova di NULL */
		kfree(uart_buf);
		return;
	}

	/* "0b1402e8 add"@0xffffff8008a5d928 + "93407d18 sxtw"@0xffffff8008a5d92c
	 * + "97dfca93 bl"@0xffffff8008a5d93c, con le stesse bandiere
	 * 0x14080c0 ("52901801 mov"@0xffffff8008a5d930): di nuovo `kzalloc` */
	total_buf = kzalloc(uart_len + len, GFP_KERNEL);
	if (IS_ERR(total_buf) || total_buf == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate total_buf memory %ld\n"@0xffffff8009242e1c,
		 * riga 1107, x3 = il puntatore
		 * ("aa1703e3 mov"@0xffffff8008a5da0c) */
		ILI_ERR("Failed to allocate total_buf memory %ld\n",
#line 1107
			PTR_ERR(total_buf));
		/* "97dfcb66 bl"@0xffffff8008a5da18 -- `uart_buf` senza prova */
		kfree(uart_buf);
		/* "b4000077 cbz"@0xffffff8008a5da1c */
		if (total_buf)
			kfree(total_buf);
		return;
	}

	/* "93407e94 sxtw"@0xffffff8008a5d950 + "940fe8e8 bl"@0xffffff8008a5d960 */
	memcpy(total_buf, dato, len);
	/* "8b1402e0 add"@0xffffff8008a5d964 + "940fe8e4 bl"@0xffffff8008a5d970 */
	memcpy(total_buf + len, uart_buf, uart_len);
	/* "97fffcd0 bl"@0xffffff8008a5d97c, con la lunghezza TOTALE
	 * ("2a1803e1 mov"@0xffffff8008a5d978) */
	ilitek_tddi_touch_send_debug_data(total_buf, uart_len + len);
	/* "97dfcb8b bl"@0xffffff8008a5d984 */
	kfree(uart_buf);
	/* "97dfcb63 bl"@0xffffff8008a5da24 */
	kfree(total_buf);
}

/*
 * ===========================================================================
 * LA STRUTTURA DI `demo_debug_info_id0` -- 12 byte, IMPACCHETTATA
 * ===========================================================================
 * DODICI byte, e lo dicono due misure indipendenti: il limite della copia
 * ("7100329f cmp"@0xffffff8008a5c338, con segno) e il valore STAMPATO come
 * "struct size" ("321e07e4 orr"@0xffffff8008a5c368).
 *
 * IMPACCHETTATA, e lo dice la lettura NON ALLINEATA: dopo il primo byte
 * tutti i campi si leggono da `sp+9`, `sp+11`, `sp+12`, `sp+15`, `sp+16`,
 * `sp+17` -- cioe' dagli scostamenti 1, 3, 4, 7, 8, 9 della struttura --
 * con `ldur` ("b84093e8 ldur"@0xffffff8008a5c390,
 * "7840b3e8 ldurh"@0xffffff8008a5c454, "f84093e8 ldur"@0xffffff8008a5c4a8).
 * Un campo a 3 bit ATTRAVERSA il confine fra il byte 4 e il byte 5
 * (`algo_pt_status2`, "d35e8103 ubfx"@0xffffff8008a5c4b8, bit 38..40): con
 * campi di bit di tipo `u8` il compilatore NON li farebbe attraversare, e
 * quindi il tipo e' piu' largo e la struttura e' `packed`.
 *
 * I VENTIDUE NOMI VENGONO DAL BINARIO. Non sono scelti: ogni campo ha una
 * stringa di formato che lo nomina, ed e' l'eccezione prevista dalla regola
 * 5. Esempi: "\x016ILITEK: (%s, %d): app_sys_powr_state_e = %d\n"@0xffffff80092426af,
 * "\x016ILITEK: (%s, %d): status_of_dynamic_th_e = %d\n"@0xffffff80092427de,
 * "\x016ILITEK: (%s, %d): hopping_index = %d\n"@0xffffff80092429da.
 *
 * TRE BUCHI NON STAMPATI, e non hanno nome perche' il binario non ne dice
 * niente:
 *   - il bit 23 (fra `g_b_wrong_bg` e `status_of_dynamic_th_e`);
 *   - i bit 28..31 (fra `status_of_dynamic_th_e` e `algo_pt_status0`);
 *   - i bit 62..63 (fra `algo_pt_status9` e `hopping_flag`).
 * IL TERZO NON ERA STATO CONTATO dalla ricognizione (`RIASSUNTO.md` §6.3 ne
 * elenca due): la sua esistenza si deduce dal fatto che `algo_pt_status9`
 * finisce al bit 61 ("d373d503 ubfx"@0xffffff8008a5c57c, `#51, #3` sul
 * registro caricato dallo scostamento 1, cioe' bit 59..61) e `hopping_flag`
 * comincia al bit 64 ("b94013e8 ldr"@0xffffff8008a5c588 dallo scostamento 8,
 * "12000103 and"@0xffffff8008a5c598 sul bit 0).
 *
 * E DUE BYTE FINALI mai letti: i campi coprono 80 bit = 10 byte, la
 * struttura ne misura 12.
 *
 * `frequency` NON E' UN CAMPO DI BIT CONTIGUO. Il binario compone dieci bit
 * da due pezzi CHE NON SONO NELL'ORDINE NATURALE: i due bit ALTI stanno ai
 * bit 70..71 e gli otto bassi nel byte 9 (bit 72..79) --
 * "d376fd08 lsr"@0xffffff8008a5c5d0 piu' "12180508 and"@0xffffff8008a5c5d4
 * danno `((x >> 62) & 3) << 8`, e "2a090103 orr"@0xffffff8008a5c5d8 vi
 * somma "394047e9 ldrb"@0xffffff8008a5c5c4, il byte 9. Un campo di bit
 * unico a 10 bit darebbe l'ordine opposto. Qui e' reso come due campi con
 * la composizione esplicita, che e' la sola forma che riproduce le tre
 * istruzioni; il NOME `frequency` resta uno solo, dal formato.
 */
struct ilitek_info_id0 {
	/* bit 0..7: "394023e3 ldrb"@0xffffff8008a5c378, senza maschera */
	u8 id;
	/* bit 8..10: "12000903 and"@0xffffff8008a5c3a0 */
	u32 app_sys_powr_state_e:3;
	/* bit 11..13: "53031503 ubfx"@0xffffff8008a5c3bc */
	u32 app_sys_state_e:3;
	/* bit 14..15: "53061d03 ubfx"@0xffffff8008a5c3d8 */
	u32 tp_state_e:2;
	/* bit 16..17: "53082503 ubfx"@0xffffff8008a5c3f4 */
	u32 touch_palm_state_e:2;
	/* bit 18..20: "530a3103 ubfx"@0xffffff8008a5c410 */
	u32 app_an_statu_e:3;
	/* bit 21: "530d3503 ubfx"@0xffffff8008a5c42c */
	u32 app_sys_check_bg_abnormal:1;
	/* bit 22: "530e3903 ubfx"@0xffffff8008a5c448 */
	u32 g_b_wrong_bg:1;
	/* bit 23: BUCO. Nessuna istruzione del blocco lo legge. */
	u32 __ignoto_23:1;
	/* bit 24..27: "12000d03 and"@0xffffff8008a5c464 sul mezzo-parola letto
	 * dallo scostamento 3 */
	u32 status_of_dynamic_th_e:4;
	/* bit 28..31: BUCO di quattro bit. */
	u32 __ignoto_28:4;
	/* bit 32..34: "12000903 and"@0xffffff8008a5c480 sul byte 4 */
	u32 algo_pt_status0:3;
	/* bit 35..37: "531b7503 ubfx"@0xffffff8008a5c49c */
	u32 algo_pt_status1:3;
	/* bit 38..40: "d35e8103 ubfx"@0xffffff8008a5c4b8 -- attraversa il
	 * confine fra il byte 4 e il byte 5 */
	u32 algo_pt_status2:3;
	/* bit 41..43: "d3618d03 ubfx"@0xffffff8008a5c4d4 */
	u32 algo_pt_status3:3;
	/* bit 44..46: "d3649903 ubfx"@0xffffff8008a5c4f0 */
	u32 algo_pt_status4:3;
	/* bit 47..49: "d367a503 ubfx"@0xffffff8008a5c50c */
	u32 algo_pt_status5:3;
	/* bit 50..52: "d36ab103 ubfx"@0xffffff8008a5c528 */
	u32 algo_pt_status6:3;
	/* bit 53..55: "d36dbd03 ubfx"@0xffffff8008a5c544 */
	u32 algo_pt_status7:3;
	/* bit 56..58: "12000903 and"@0xffffff8008a5c560 sulla parola letta
	 * dallo scostamento 7 */
	u32 algo_pt_status8:3;
	/* bit 59..61: "d373d503 ubfx"@0xffffff8008a5c57c */
	u32 algo_pt_status9:3;
	/* bit 62..63: BUCO. Vedi il cappello. */
	u32 __ignoto_62:2;
	/* bit 64: "12000103 and"@0xffffff8008a5c598 sulla parola letta dallo
	 * scostamento 8 */
	u32 hopping_flag:1;
	/* bit 65..69: "d379f503 ubfx"@0xffffff8008a5c5b4 */
	u32 hopping_index:5;
	/* bit 70..71: i due bit ALTI di `frequency` */
	u32 frequency_alti:2;
	/*
	 * bit 72..79: gli otto bit BASSI di `frequency`,
	 * "394047e9 ldrb"@0xffffff8008a5c5c4.
	 * E' UN CAMPO DI BIT, non un `u8`, e lo dice la LARGHEZZA DEI DUE
	 * CARICAMENTI CHE LO PRECEDONO: la fabbrica legge una parola intera
	 * allo scostamento 7 ("b840f3e8 ldur"@0xffffff8008a5c550) e un'altra
	 * allo scostamento 8 ("b94013e8 ldr"@0xffffff8008a5c588). Con un `u8`
	 * qui la corsa di campi di bit finirebbe al bit 71 e clang
	 * restringerebbe quei due caricamenti a mezza parola e a un byte --
	 * misurato: `ldurh` e `ldrb`, stessa dimensione della funzione.
	 * La corsa unica arriva percio' fino in fondo alla struttura.
	 */
	u32 frequency_bassi:8;
	/* bit 80..95: mai letti da nessuna istruzione del blocco. Ci sono
	 * perche' la struttura misura 12 byte e i campi ne coprono 10. */
	u32 __ignoto_80:16;
} __packed;

/*
 * ===========================================================================
 * demo_debug_info_id0 -- 0xffffff8008a5c318, 760 byte
 * ===========================================================================
 * Righe di fabbrica: da 610 ("52804c42 mov"@0xffffff8008a5c364) a 633
 * ("52804f22 mov"@0xffffff8008a5c5dc), consecutive tranne la 611.
 *
 * IL SECONDO PARAMETRO E' UN `int`: "93407e89 sxtw"@0xffffff8008a5c334 lo
 * estende con segno a 64 bit DENTRO la funzione. Con un parametro gia' a 64
 * bit quella istruzione non servirebbe. E il confronto col 12 e' con segno
 * ("7100329f cmp"@0xffffff8008a5c338 + "9a8ab122 csel"@0xffffff8008a5c344,
 * condizione `lt`): con `sizeof` il confronto sarebbe SENZA segno.
 *
 * La copia locale sta sulla pila e per questo la funzione ha il protettore
 * ("f9454508 ldr"@0xffffff8008a5c32c, "eb08013f cmp"@0xffffff8008a5c5f4,
 * "97d94715 bl"@0xffffff8008a5c60c verso `__stack_chk_fail`).
 *
 * Nessuno dei ventiquattro messaggi e' guardato dal diagnostico: non c'e'
 * una sola lettura di `ilitek_dbg_en` in tutta la funzione.
 *
 * I VENTIDUE LETTERALI, uno per uno, con l'indirizzo di fabbrica:
 *   "\x016ILITEK: (%s, %d): id = %d\n"@0xffffff8009242692
 *   "\x016ILITEK: (%s, %d): app_sys_state_e = %d\n"@0xffffff80092426de
 *   "\x016ILITEK: (%s, %d): tp_state_e = %d\n"@0xffffff8009242708
 *   "\x016ILITEK: (%s, %d): touch_palm_state_e = %d\n"@0xffffff800924272d
 *   "\x016ILITEK: (%s, %d): app_an_statu_e = %d\n"@0xffffff800924275a
 *   "\x016ILITEK: (%s, %d): app_sys_check_bg_abnormal = %d\n"@0xffffff8009242783
 *   "\x016ILITEK: (%s, %d): g_b_wrong_bg = %d\n"@0xffffff80092427b7
 *   "\x016ILITEK: (%s, %d): algo_pt_status0 = %d\n"@0xffffff800924280f
 *   "\x016ILITEK: (%s, %d): algo_pt_status1 = %d\n"@0xffffff8009242839
 *   "\x016ILITEK: (%s, %d): algo_pt_status2 = %d\n"@0xffffff8009242863
 *   "\x016ILITEK: (%s, %d): algo_pt_status3 = %d\n"@0xffffff800924288d
 *   "\x016ILITEK: (%s, %d): algo_pt_status4 = %d\n"@0xffffff80092428b7
 *   "\x016ILITEK: (%s, %d): algo_pt_status5 = %d\n"@0xffffff80092428e1
 *   "\x016ILITEK: (%s, %d): algo_pt_status6 = %d\n"@0xffffff800924290b
 *   "\x016ILITEK: (%s, %d): algo_pt_status7 = %d\n"@0xffffff8009242935
 *   "\x016ILITEK: (%s, %d): algo_pt_status8 = %d\n"@0xffffff800924295f
 *   "\x016ILITEK: (%s, %d): algo_pt_status9 = %d\n"@0xffffff8009242989
 *   "\x016ILITEK: (%s, %d): hopping_flag = %d\n"@0xffffff80092429b3
 *   "\x016ILITEK: (%s, %d): frequency = %d\n"@0xffffff8009242a02
 */
void demo_debug_info_id0(void *dato, unsigned long len)
{
	struct ilitek_info_id0 s;
	/*
	 * IL SECONDO PARAMETRO E' A 64 BIT, E IL CORPO LO TRONCA A `int`.
	 * Lo dicono tre istruzioni che stanno insieme e solo insieme:
	 * "aa0103f4 mov"@0xffffff8008a5c330 copia x1 INTERO (con un
	 * parametro `int` clang emette `mov w20, w1`),
	 * "93407e89 sxtw"@0xffffff8008a5c334 riestende i 32 bit bassi, e
	 * "9a8ab122 csel"@0xffffff8008a5c344 sceglie A 64 BIT fra quel
	 * valore e 12 ("321e07ea orr"@0xffffff8008a5c33c).
	 * Con un parametro `int` questo compilatore accorcia la scelta a 32
	 * bit e mette l'estensione DOPO (`csel w`, poi `sxtw x2, w`): e' la
	 * canonicalizzazione `select(c, sext(x), K) -> sext(select(c, x, K))`,
	 * che con `sext(trunc(...))` invece NON scatta. Verificato col
	 * compilatore di fabbrica su otto forme sorgente (misura-E/lab):
	 * tutte quelle con parametro `int` danno la forma stretta, e solo
	 * quelle con parametro a 64 bit troncato danno le quattro istruzioni
	 * di fabbrica. Vedi la divergenza D5 del cappello.
	 */
	int l = (int)len;

	/* "940fee6c bl"@0xffffff8008a5c350 */
	memcpy(&s, dato, (l < (int)sizeof(s)) ? (size_t)l : sizeof(s));

	/* "\x016ILITEK: (%s, %d): id0 len = %d, struct size = %d\n"@0xffffff800924264a,
	 * __func__ "demo_debug_info_id0"@0xffffff800924267e, riga 610,
	 * w3 = len troncato ("2a1403e3 mov"@0xffffff8008a5c370), w4 = 12 */
#line 610
	ILI_INFO("id0 len = %d, struct size = %d\n", l, 12);
#line 612
	ILI_INFO("id = %d\n", s.id);
	ILI_INFO("app_sys_powr_state_e = %d\n", s.app_sys_powr_state_e);
	ILI_INFO("app_sys_state_e = %d\n", s.app_sys_state_e);
	ILI_INFO("tp_state_e = %d\n", s.tp_state_e);
	ILI_INFO("touch_palm_state_e = %d\n", s.touch_palm_state_e);
	ILI_INFO("app_an_statu_e = %d\n", s.app_an_statu_e);
	ILI_INFO("app_sys_check_bg_abnormal = %d\n", s.app_sys_check_bg_abnormal);
	ILI_INFO("g_b_wrong_bg = %d\n", s.g_b_wrong_bg);
	ILI_INFO("status_of_dynamic_th_e = %d\n", s.status_of_dynamic_th_e);
	ILI_INFO("algo_pt_status0 = %d\n", s.algo_pt_status0);
	ILI_INFO("algo_pt_status1 = %d\n", s.algo_pt_status1);
	ILI_INFO("algo_pt_status2 = %d\n", s.algo_pt_status2);
	ILI_INFO("algo_pt_status3 = %d\n", s.algo_pt_status3);
	ILI_INFO("algo_pt_status4 = %d\n", s.algo_pt_status4);
	ILI_INFO("algo_pt_status5 = %d\n", s.algo_pt_status5);
	ILI_INFO("algo_pt_status6 = %d\n", s.algo_pt_status6);
	ILI_INFO("algo_pt_status7 = %d\n", s.algo_pt_status7);
	ILI_INFO("algo_pt_status8 = %d\n", s.algo_pt_status8);
	ILI_INFO("algo_pt_status9 = %d\n", s.algo_pt_status9);
	ILI_INFO("hopping_flag = %d\n", s.hopping_flag);
	ILI_INFO("hopping_index = %d\n", s.hopping_index);
	/* "2a090103 orr"@0xffffff8008a5c5d8 */
#line 633
	ILI_INFO("frequency = %d\n", (s.frequency_alti << 8) | s.frequency_bassi);
}

/*
 * Gruppo D. Le firme vengono dai siti di chiamata di QUESTO blocco.
 * "97ffeced bl"@0xffffff8008a5bdf4: due interi, ritorno provato sul solo bit
 * di segno ("36f800e0 tbz"@0xffffff8008a5bdf8).
 */
int ilitek_ice_mode_ctrl(int enable, int mcu);
/*
 * Gruppo D. "97ffec90 bl"@0xffffff8008a5be58 (verso ilitek_ice_mode_write,
 * 0xffffff8008a57098): w0 un indirizzo a 32 bit, w1 un dato a 32 bit, w2 la
 * lunghezza in byte ("321e03e2 orr"@0xffffff8008a5be54, cioe' 4).
 */
int ilitek_ice_mode_write(u32 addr, u32 dato, int len);
/*
 * Gruppo D. "97ffebf9 bl"@0xffffff8008a5befc: w0 un indirizzo a 32 bit,
 * x1 un PUNTATORE ("910013e1 add"@0xffffff8008a5bef0 -- l'indirizzo di un
 * locale sulla pila), w2 la lunghezza.
 */
int ilitek_ice_mode_read(u32 addr, void *dato, int len);

/*
 * ===========================================================================
 * ilitek_tddi_touch_esd_gesture_flash -- 0xffffff8008a5bdc4, 588 byte
 * ===========================================================================
 * Righe di fabbrica: 490, 493, 497, 502, 505, 510, 512, 518, 521, 525
 * ("52803d42 mov"@0xffffff8008a5be0c, "52803da2 mov"@0xffffff8008a5be30,
 * "52803e22 mov"@0xffffff8008a5be70, "52803ec2 mov"@0xffffff8008a5bea0,
 * "52803f22 mov"@0xffffff8008a5bec8, "321f1fe2 orr"@0xffffff8008a5bf04,
 * "321703e2 orr"@0xffffff8008a5bf24, "528040c2 mov"@0xffffff8008a5bf78,
 * "52804122 mov"@0xffffff8008a5bf98, "528041a2 mov"@0xffffff8008a5bfc4).
 *
 * LE TRE COSTANTI a 32 bit sono materializzate in due pezzi ciascuna e sono
 * quindi lette senza ambiguita':
 *   indirizzo 0x40054 -- "52800a83 mov"@0xffffff8008a5be1c +
 *      "72a00083 movk"@0xffffff8008a5be34;
 *   risposta attesa 0xa67c9dfe -- "5293bfc4 mov"@0xffffff8008a5be20 +
 *      "72b4cf84 movk"@0xffffff8008a5be38 (e la copia in w23,
 *      "5293bfd7 mov"@0xffffff8008a5be24 + "72b4cf97 movk"@0xffffff8008a5be3c,
 *      che il compilatore tiene viva attraverso tutto il ciclo);
 *   parola d'ordine scritta 0xf38a94ef -- "52929de1 mov"@0xffffff8008a5be48 +
 *      "72be7141 movk"@0xffffff8008a5be50.
 * La parola d'ordine SCRITTA e quella ATTESA in lettura sono DIVERSE, e non
 * e' un refuso mio: sono due costanti distinte nel binario.
 *
 * IL RITARDO e' `mdelay(1)`: "52912b00 mov"@0xffffff8008a5bf38 +
 * "72a00820 movk"@0xffffff8008a5bf3c danno 0x418958 = 4295000 = 1000 * 0x10c7,
 * e 0x10c7 e' il fattore che `udelay` di questo albero moltiplica ai
 * microsecondi. `mdelay(1)` si espande in `udelay(1000)` perche' 1 <= 5.
 *
 * IL CONTATORE del ciclo vale 101 all'ingresso
 * ("52800cb8 mov"@0xffffff8008a5bedc) e il ciclo esce quando arriva a 2
 * ("71000b1f cmp"@0xffffff8008a5bf44): sono CENTO giri. Il registro tiene
 * `retry + 1`, non `retry` -- clang materializza il valore gia'
 * decrementato ("51000708 sub"@0xffffff8008a5bf48) e riusa lo stesso
 * confronto per la prova d'uscita che segue
 * ("7100051f cmp"@0xffffff8008a5bf60, `retry > 0`).
 */
int ilitek_tddi_touch_esd_gesture_flash(void)
{
	int ret = 0;
	int retry = 100;
	u32 answer = 0;

	/* "97ffeced bl"@0xffffff8008a5bdf4 */
	if (ilitek_ice_mode_ctrl(1, 0) < 0)
		/* "\x013ILITEK: (%s, %d): Enable ice mode failed during gesture recovery\n"@0xffffff80092423dd,
		 * __func__ "ilitek_tddi_touch_esd_gesture_flash"@0xffffff8009242421,
		 * riga 490 */
#line 490
		ILI_ERR("Enable ice mode failed during gesture recovery\n");

	/* "\x016ILITEK: (%s, %d): ESD Gesture PWD Addr = 0x%x, Answer = 0x%x\n"@0xffffff8009242445,
	 * riga 493, NON guardata */
	ILI_INFO("ESD Gesture PWD Addr = 0x%x, Answer = 0x%x\n",
#line 493
		 0x40054, 0xa67c9dfe);

	/* "97ffec90 bl"@0xffffff8008a5be58 */
	if (ilitek_ice_mode_write(0x40054, 0xf38a94ef, 4) < 0)
		/* "\x013ILITEK: (%s, %d): write password failed\n"@0xffffff8009242485,
		 * riga 497 */
#line 497
		ILI_ERR("write password failed\n");

	/* "b901411f str"@0xffffff8008a5be84 -- idev+320 = 0, PRIMA del reset */
	idev->c320 = 0;
	/* "b9426500 ldr"@0xffffff8008a5be80 (idev+612) +
	 * "97ffdf30 bl"@0xffffff8008a5be88 */
	if (ilitek_tddi_reset_ctrl(idev->c612) < 0)
		/* "\x013ILITEK: (%s, %d): TP Reset failed during gesture recovery\n"@0xffffff80092424b0,
		 * riga 502 */
#line 502
		ILI_ERR("TP Reset failed during gesture recovery\n");

	/* "97ffecbe bl"@0xffffff8008a5beb0, questa volta con (1, 1) */
	if (ilitek_ice_mode_ctrl(1, 1) < 0)
		/* stesso letterale della riga 490, riga 505 */
#line 505
		ILI_ERR("Enable ice mode failed during gesture recovery\n");

	do {
		/* "97ffebf9 bl"@0xffffff8008a5befc */
		if (ilitek_ice_mode_read(0x40054, &answer, 4) < 0)
			/* "\x013ILITEK: (%s, %d): Read gesture answer error\n"@0xffffff80092424ed,
			 * riga 510 */
#line 510
			ILI_ERR("Read gesture answer error\n");

		/* "6b17007f cmp"@0xffffff8008a5bf18 */
		if (answer != 0xa67c9dfe)
			/* "\x016ILITEK: (%s, %d): answer = 0x%x != (0x%x)\n"@0xffffff800924251c,
			 * riga 512, NON guardata */
#line 512
			ILI_INFO("answer = 0x%x != (0x%x)\n", answer, 0xa67c9dfe);

		/* "940feecd bl"@0xffffff8008a5bf40 */
		mdelay(1);
		retry--;
	} while (answer != 0xa67c9dfe && retry > 0);

	/* "5400012c b.gt"@0xffffff8008a5bf64 */
	if (retry <= 0) {
		/* "\x013ILITEK: (%s, %d): Enter gesture failed\n"@0xffffff8009242549,
		 * riga 518 */
#line 518
		ILI_ERR("Enter gesture failed\n");
		/* "12800013 mov"@0xffffff8008a5bf80 */
		ret = -1;
	} else {
		/* "\x016ILITEK: (%s, %d): Enter gesture successfully\n"@0xffffff8009242573,
		 * riga 521 */
#line 521
		ILI_INFO("Enter gesture successfully\n");
		/* "2a1f03f3 mov"@0xffffff8008a5bfa0 */
		ret = 0;
	}

	/* "97ffec7f bl"@0xffffff8008a5bfac, con (0, 1) */
	if (ilitek_ice_mode_ctrl(0, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Disable ice mode failed during gesture recovery\n"@0xffffff80092425a3,
		 * riga 525 */
#line 525
		ILI_ERR("Disable ice mode failed during gesture recovery\n");

	/* "f9419909 ldr"@0xffffff8008a5bfd0 (idev+816) +
	 * "b9421900 ldr"@0xffffff8008a5bfd4 (idev+536) +
	 * "d63f0120 blr"@0xffffff8008a5bfd8. Il ritorno NON e' usato: l'epilogo
	 * restituisce w19 ("2a1303e0 mov"@0xffffff8008a5bff0).
	 * Il campo +816 e' `ilitek_tddi_move_gesture_code_flash`, e questa e'
	 * una CONFERMA indipendente di cio' che il gruppo B aveva scritto. */
	idev->c816(idev->c536);

	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_move_gesture_code_iram -- 0xffffff8008a5bac8, 716 byte
 * ===========================================================================
 * Righe di fabbrica: 418, 420, 423, 427, 429, 431, 444, 447, 449, 451, 457,
 * 462, 469 ("52803442 mov"@0xffffff8008a5bc8c, "52803482 mov"@0xffffff8008a5bcb4,
 * "528034e2 mov"@0xffffff8008a5bb20, "52803562 mov"@0xffffff8008a5bb4c,
 * "528035a2 mov"@0xffffff8008a5bb64, "528035e2 mov"@0xffffff8008a5bb8c,
 * "52803782 mov"@0xffffff8008a5bc00, "528037e2 mov"@0xffffff8008a5bc60,
 * "52803822 mov"@0xffffff8008a5bc34, "52803862 mov"@0xffffff8008a5bccc,
 * "52803922 mov"@0xffffff8008a5bd50, "528039c2 mov"@0xffffff8008a5bcf0,
 * "52803aa2 mov"@0xffffff8008a5bd30).
 *
 * IL BUFFER E' DI TRE BYTE e sta sulla pila: azzerato in due pezzi
 * ("79000bff strh"@0xffffff8008a5bb04 e "39001bff strb"@0xffffff8008a5bb00),
 * e la sua presenza accende il protettore di pila
 * ("97d94934 bl"@0xffffff8008a5bd90).
 *
 * IL DIFETTO DELLA FABBRICA CHE SI RIPRODUCE (regola 7): due messaggi
 * d'errore NON hanno l'a capo finale --
 * "\x013ILITEK: (%s, %d): write 0x1,0xA,0x5 error"@0xffffff8009242273 e
 * "\x013ILITEK: (%s, %d): write 0x1,0xA,0x6 error"@0xffffff80092423b1.
 * Anche il messaggio di riga 418 ha un a capo IN MEZZO, non alla fine di una
 * frase: "...lpwg cmd = %d,\n no need to load gesture code by driver\n".
 *
 * IL RITORNO DELL'ULTIMO RAMO NON E' ZERO: dopo
 * "97ffe060 bl"@0xffffff8008a5bd58 verso `ilitek_tddi_gesture_recovery` non
 * c'e' nessuna scrittura di w0, e l'epilogo restituisce quello che la
 * chiamata ha lasciato. Sugli altri rami w0 e' azzerato esplicitamente
 * ("2a1f03e0 mov"@0xffffff8008a5bd38).
 *
 * IL PRIMO `idev->c776(cmd, 2)` NON E' CONTROLLATO: dopo
 * "d63f0100 blr"@0xffffff8008a5bbdc non c'e' nessun `tbz`/`tbnz`. Il
 * secondo invece si' ("36f800a0 tbz"@0xffffff8008a5bbfc).
 */
int ilitek_tddi_move_gesture_code_iram(int mode)
{
	int i;
	u8 cmd[3] = {0};

	/* "3949f908 ldrb"@0xffffff8008a5bb08 (idev+638) +
	 * "34000b88 cbz"@0xffffff8008a5bb0c */
	if (!idev->c638) {
		/* "\x016ILITEK: (%s, %d): Switch to Gesture mode, lpwg cmd = %d,\n no need to load gesture code by driver\n"@0xffffff800924215a,
		 * riga 418, w3 = mode ("2a1303e3 mov"@0xffffff8008a5bc90) */
		ILI_INFO("Switch to Gesture mode, lpwg cmd = %d,\n no need to load gesture code by driver\n",
#line 418
			 mode);
		/* "97ffdff9 bl"@0xffffff8008a5bc9c */
		if (ilitek_set_tp_data_len(mode) < 0)
			/* "\x013ILITEK: (%s, %d): Failed to set tp data length\n"@0xffffff80092421e1,
			 * riga 420 */
#line 420
			ILI_ERR("Failed to set tp data length\n");
		return 0;
	}

	/* "\x016ILITEK: (%s, %d): Load gesture code by driver\n"@0xffffff8009242213,
	 * riga 423 */
#line 423
	ILI_INFO("Load gesture code by driver\n");
	/* "97fff0d5 bl"@0xffffff8008a5bb34, x0 = "lpwg"@0xffffff800923e90a
	 * (fusa col gruppo D), w1 = 3 ("320007e1 orr"@0xffffff8008a5bb30) */
	if (ilitek_tddi_ic_func_ctrl("lpwg", 3) < 0)
		/* "\x013ILITEK: (%s, %d): write gesture flag failed\n"@0xffffff8009242244,
		 * riga 427 */
#line 427
		ILI_ERR("write gesture flag failed\n");

	/* stesso letterale di `move_gesture_code_flash`, riga 429 */
#line 429
	ILI_INFO("Switch to Gesture mode, lpwg cmd = %d\n", mode);
	/* "97ffe043 bl"@0xffffff8008a5bb74 */
	if (ilitek_set_tp_data_len(mode) < 0)
		/* riga 431 */
#line 431
		ILI_ERR("Failed to set tp data length\n");

	/* "7100271f cmp"@0xffffff8008a5bc54 + "54fffb89 b.ls"@0xffffff8008a5bc58
	 * -- confronto SENZA segno con 9, cioe' dieci giri */
	for (i = 0; i < 10; i++) {
		/* "79000bf9 strh"@0xffffff8008a5bbcc con w25 = 0xaf6
		 * ("52815ed9 mov"@0xffffff8008a5bba8) */
		cmd[0] = 0xf6;
		cmd[1] = 0x0a;
		/* "d63f0100 blr"@0xffffff8008a5bbdc, w1 = 2
		 * ("321f03e1 orr"@0xffffff8008a5bbd4). Il ritorno NON e' usato. */
		idev->c776(cmd, 2);

		/* "79000bfa strh"@0xffffff8008a5bbe4 con w26 = 0xa01
		 * ("5281403a mov"@0xffffff8008a5bbac) e
		 * "39001bfb strb"@0xffffff8008a5bbe8 con w27 = 5
		 * ("528000bb mov"@0xffffff8008a5bbb0) */
		cmd[0] = 0x01;
		cmd[1] = 0x0a;
		cmd[2] = 0x05;
		/* "d63f0100 blr"@0xffffff8008a5bbf8, w1 = 3 */
		if (idev->c776(cmd, 3) < 0)
			/* riga 444, SENZA a capo */
#line 444
			ILI_ERR("write 0x1,0xA,0x5 error");

		/* "f9418908 ldr"@0xffffff8008a5bc1c (idev+784) +
		 * "d63f0100 blr"@0xffffff8008a5bc20, w1 = 1 */
		if (idev->c784(cmd, 1) < 0)
			/* "\x013ILITEK: (%s, %d): read gesture ready byte error\n"@0xffffff800924229f,
			 * riga 447 */
#line 447
			ILI_ERR("read gesture ready byte error\n");

		/* "\x016ILITEK: (%s, %d): gesture ready byte = 0x%x\n"@0xffffff80092422d2,
		 * riga 449, guardia "340000c8 cbz"@0xffffff8008a5bc2c */
#line 449
		ILI_DBG("gesture ready byte = 0x%x\n", cmd[0]);

		/* "7102447f cmp"@0xffffff8008a5bc48 */
		if (cmd[0] == 0x91) {
			/* "\x016ILITEK: (%s, %d): Gesture check fw ready\n"@0xffffff8009242301,
			 * riga 451 */
#line 451
			ILI_INFO("Gesture check fw ready\n");
			/* D1: "aa1f03e0 mov"@0xffffff8008a5bcd4 +
			 * "97ffdf68 bl"@0xffffff8008a5bcd8 */
			if (ilitek_tddi_fw_upgrade_handler_arg(NULL) < 0)
				/* "\x013ILITEK: (%s, %d): FW upgrade failed during moving code\n"@0xffffff8009242377,
				 * riga 462 */
#line 462
				ILI_ERR("FW upgrade failed during moving code\n");

			/* "79000be8 strh"@0xffffff8008a5bd00 (0xa01) +
			 * "39001be8 strb"@0xffffff8008a5bd08 (6) */
			cmd[0] = 0x01;
			cmd[1] = 0x0a;
			cmd[2] = 0x06;
			/* "d63f0100 blr"@0xffffff8008a5bd18 */
			if (idev->c776(cmd, 3) < 0)
				/* riga 469, SENZA a capo */
#line 469
				ILI_ERR("write 0x1,0xA,0x6 error");
			return 0;
		}
	}

	/* "\x013ILITEK: (%s, %d): Gesture is not ready (0x%x), try to run its recovery\n"@0xffffff800924232d,
	 * riga 457. w3 contiene ancora `cmd[0]`
	 * ("394013e3 ldrb"@0xffffff8008a5bc44): il binario non lo ricarica. */
#line 457
	ILI_ERR("Gesture is not ready (0x%x), try to run its recovery\n", cmd[0]);
	/* "97ffe060 bl"@0xffffff8008a5bd58 -- il RITORNO e' quello della
	 * funzione, non zero */
	return ilitek_tddi_gesture_recovery();
}

/*
 * ===========================================================================
 * ilitek_tddi_touch_esd_gesture_iram -- 0xffffff8008a5c010, 776 byte
 * ===========================================================================
 * Righe di fabbrica: 539, 547, 551, 556, 565, 570, 573, 578, 581, 585, 590,
 * 597.
 *
 * E' la gemella di `..._esd_gesture_flash` con QUATTRO differenze misurate:
 *
 * 1. L'INDIRIZZO DELLA PAROLA D'ORDINE E' SCELTO A TEMPO DI ESECUZIONE:
 *    "1a898113 csel"@0xffffff8008a5c0ac fra 0x40054
 *    ("52800a88 mov"@0xffffff8008a5c0a4 + "72a00088 movk"@0xffffff8008a5c0a8)
 *    e 0x25ff8 ("528bff09 mov"@0xffffff8008a5c084 +
 *    "72a00049 movk"@0xffffff8008a5c088), sulla condizione `hi` -- confronto
 *    SENZA segno di `*(u32 *)(idev->c48 + 52)` con 0x10401
 *    ("1100068a add"@0xffffff8008a5c07c, che deriva 0x10401 dal 0x10400 gia'
 *    materializzato in w20 per il confronto piu' avanti).
 * 2. LA RISPOSTA ATTESA E' DIVERSA: 0xa67c9dfe nella variante flash,
 *    0x5b92e7f4 qui ("529cfe84 mov"@0xffffff8008a5c098 +
 *    "72ab7244 movk"@0xffffff8008a5c0bc).
 * 3. AL POSTO DEL RESET c'e' l'AGGIORNAMENTO DEL FIRMWARE
 *    ("97ffde5c bl"@0xffffff8008a5c108, con x0 azzerato -- D1).
 * 4. IL CICLO E' UN `for` a contatore, non un `do/while`: il contatore parte
 *    da -100 ("12800c79 mov"@0xffffff8008a5c188) e sale con
 *    "31000739 adds"@0xffffff8008a5c1f8 finche' non riporta
 *    ("54fffce3 b.cc"@0xffffff8008a5c1fc). Sono cento giri, e l'uscita per
 *    successo e' un `break` provato PRIMA dell'incremento
 *    ("6b18011f cmp"@0xffffff8008a5c1f0 + "54000160 b.eq"@0xffffff8008a5c1f4).
 *
 * Il valore restituito sul fallimento e' -62
 * ("128007a0 mov"@0xffffff8008a5c218), cioe' -ETIME in questo albero.
 */
int ilitek_tddi_touch_esd_gesture_iram(void)
{
	int i;
	u32 addr;
	u32 answer = 0;
	u8 cmd[3] = {0};

	/* "97ffec57 bl"@0xffffff8008a5c04c */
	if (ilitek_ice_mode_ctrl(1, 0) < 0)
		/* riga 539 */
#line 539
		ILI_ERR("Enable ice mode failed during gesture recovery\n");

	/*
	 * "f9401908 ldr"@0xffffff8008a5c080 (idev+48) +
	 * "b9403508 ldr"@0xffffff8008a5c090 (+52 dentro l'oggetto puntato).
	 * `ilitek.h` dichiara `idev->c48` come `void *` e non descrive cio'
	 * che punta: qui l'accesso e' scritto per scostamento esplicito, e la
	 * struttura vera e' un DELTA DI HEADER che il lotto di merge dovra'
	 * decidere quando il gruppo che la scrive l'avra' misurata.
	 */
	addr = (*(u32 *)((u8 *)idev->c48 + 52) > 0x10401) ? 0x40054 : 0x25ff8;

	/* stesso letterale della variante flash, riga 547 */
	ILI_INFO("ESD Gesture PWD Addr = 0x%x, Answer = 0x%x\n",
#line 547
		 addr, 0x5b92e7f4);

	/* "97ffebef bl"@0xffffff8008a5c0dc */
	if (ilitek_ice_mode_write(addr, 0xf38a94ef, 4) < 0)
		/* riga 551 */
#line 551
		ILI_ERR("write password failed\n");

	/* "b901411f str"@0xffffff8008a5c104 */
	idev->c320 = 0;
	/* D1: "97ffde5c bl"@0xffffff8008a5c108 */
	if (ilitek_tddi_fw_upgrade_handler_arg(NULL) < 0)
		/* "\x013ILITEK: (%s, %d): FW upgrade failed during gesture recovery\n"@0xffffff800924260b,
		 * riga 556 */
#line 556
		ILI_ERR("FW upgrade failed during gesture recovery\n");

	/* "390a010a strb"@0xffffff8008a5c134 -- idev+640 = 1 */
	idev->c640 = 1;
	/* "3949f109 ldrb"@0xffffff8008a5c130 (idev+636) +
	 * "350000a9 cbnz"@0xffffff8008a5c138, e in alternativa
	 * "6b14011f cmp"@0xffffff8008a5c144 + "54000069 b.ls"@0xffffff8008a5c148
	 * (senza segno, contro 0x10400) */
	if (idev->c636 || *(u32 *)((u8 *)idev->c48 + 52) > 0x10400)
		/* "97dbdab1 bl"@0xffffff8008a5c150, w0 = 50 */
		msleep(50);

	/* "97ffec13 bl"@0xffffff8008a5c15c */
	if (ilitek_ice_mode_ctrl(1, 1) < 0)
		/* riga 565 */
#line 565
		ILI_ERR("Enable ice mode failed during gesture recovery\n");

	for (i = 100; i > 0; i--) {
		/* "97ffeb4f bl"@0xffffff8008a5c1a4 */
		if (ilitek_ice_mode_read(addr, &answer, 4) < 0)
			/* riga 570 */
#line 570
			ILI_ERR("Read gesture answer error\n");

		/* "6b18007f cmp"@0xffffff8008a5c1c0 */
		if (answer != 0x5b92e7f4)
			/* riga 573 */
			ILI_INFO("answer = 0x%x != (0x%x)\n",
#line 573
				 answer, 0x5b92e7f4);

		/* "940fee23 bl"@0xffffff8008a5c1e8 */
		mdelay(1);

		/* "54000160 b.eq"@0xffffff8008a5c1f4 */
		if (answer == 0x5b92e7f4)
			break;
	}

	if (answer != 0x5b92e7f4) {
		/* riga 578 */
#line 578
		ILI_ERR("Enter gesture failed\n");
		/* "128007a0 mov"@0xffffff8008a5c218 */
		return -ETIME;
	}

	/* riga 581 */
#line 581
	ILI_INFO("Enter gesture successfully\n");

	/* "97ffebda bl"@0xffffff8008a5c240 */
	if (ilitek_ice_mode_ctrl(0, 1) < 0)
		/* riga 585 */
#line 585
		ILI_ERR("Disable ice mode failed during gesture recovery\n");

	/* "b9014109 str"@0xffffff8008a5c26c -- idev+320 = 0xf
	 * ("32000fe9 orr"@0xffffff8008a5c264) */
	idev->c320 = 0x0f;
	/* "97ffde84 bl"@0xffffff8008a5c270 */
	ilitek_set_tp_data_len(idev->c536);
	/* D1: "aa1f03e0 mov"@0xffffff8008a5c274 +
	 * "97ffde00 bl"@0xffffff8008a5c278 */
	if (ilitek_tddi_fw_upgrade_handler_arg(NULL) < 0)
		/* riga 590 */
#line 590
		ILI_ERR("FW upgrade failed during gesture recovery\n");

	/* "79000be8 strh"@0xffffff8008a5c2a0 (0xa01) +
	 * "39001be8 strb"@0xffffff8008a5c2a8 (6) */
	cmd[0] = 0x01;
	cmd[1] = 0x0a;
	cmd[2] = 0x06;
	/* "d63f0100 blr"@0xffffff8008a5c2b8 */
	if (idev->c776(cmd, 3) < 0)
		/* riga 597, SENZA a capo */
#line 597
		ILI_ERR("write 0x1,0xA,0x6 error");

	/* "390a011f strb"@0xffffff8008a5c2e0 -- idev+640 = 0 */
	idev->c640 = 0;
	return 0;
}

/*
 * ===========================================================================
 * LA STRUTTURA DEL GESTO -- l'oggetto da 56 byte puntato da `idev->c64`
 * ===========================================================================
 * `ilitek.h` dice gia' che `idev->c64` punta a un blocco di 56 byte allocato
 * da `probe` con `kzalloc(56, GFP_KERNEL)`. Questo blocco e' l'unico posto
 * che ne descrive la FORMA, e la descrive per intero:
 *
 *  - l'azzeramento in testa copre esattamente 56 byte, in quattro istruzioni
 *    ("f90002ff str"@0xffffff8008a5d284 per 0..7 e tre
 *    "a900feff stp"@0xffffff8008a5d280 / 0xffffff8008a5d27c /
 *    0xffffff8008a5d278 per 8..55);
 *  - i campi toccati sono +0 (u16), +2 (u8), +4 (int) e dodici `u16` agli
 *    scostamenti 10, 12, 18, 20, 26, 28, 34, 36, 42, 44, 50, 52 -- cioe' SEI
 *    elementi da OTTO byte a partire da +8, con due `u16` a +2 e +4 dentro
 *    ciascuno. La stessa disposizione dell'array `punti`.
 *
 * DUE NOMI VENGONO DAL BINARIO: il formato
 * "\x016ILITEK: (%s, %d): Transfer = %d, Type = %d, clockwise = %d\n"@0xffffff8009242c60 riceve
 * "b94006e4 ldr"@0xffffff8008a5d6d4 (il campo +4) come `Type` e
 * "39400ae5 ldrb"@0xffffff8008a5d6d8 (il campo +2) come `clockwise`; il
 * primo `%d` e' `idev->c641`, che il formato chiama `Transfer`.
 * Gli altri restano `c<offset>`.
 *
 * DELTA DI HEADER: il tipo dovrebbe stare in `ilitek.h` al posto del
 * `void *c64`. Qui e' dichiarato localmente.
 */
struct ilitek_gesto {
	/* +0 u16: "790002e3 strh"@0xffffff8008a5d298, e riletto come selettore
	 * dello `switch` ("794002ea ldrh"@0xffffff8008a5d388) */
	u16 c0;
	/* +2 un byte: "39000ae8 strb"@0xffffff8008a5d4c8. Il formato lo chiama
	 * `clockwise`. */
	u8 c2;
	/* +3: nessuna istruzione lo tocca. Serve comunque per allineare il
	 * campo a 4 byte che segue. */
	u8 __ignoto_3;
	/* +4 32 bit: "b90006e8 str"@0xffffff8008a5d544 e
	 * "b94006e4 ldr"@0xffffff8008a5d6d4. Il formato lo chiama `Type`. */
	int c4;
	/* +8: sei elementi da otto byte. Del primo e dell'ultimo `u16` di
	 * ciascun elemento (scostamenti +0 e +6) il binario non dice niente in
	 * questo blocco: sono azzerati e mai piu' toccati. */
	struct {
		u16 c0;
		/* "790016e9 strh"@0xffffff8008a5d2c0 per l'elemento 0 */
		u16 c2;
		/* "79001ae9 strh"@0xffffff8008a5d2d0 per l'elemento 0 */
		u16 c4;
		u16 c6;
	} pos[6];
};

/*
 * ===========================================================================
 * ilitek_tddi_report_gesture_mode -- 0xffffff8008a5d1ec, 1560 byte
 * ===========================================================================
 * Righe di fabbrica: 932, 950, 1031, 1050, 1057
 * ("52807482 mov"@0xffffff8008a5d2a4, "528076c2 mov"@0xffffff8008a5d3d4,
 * "528080e2 mov"@0xffffff8008a5d45c, "52808342 mov"@0xffffff8008a5d6ec,
 * "52808422 mov"@0xffffff8008a5d734).
 *
 * IL BUFFER LOCALE E' DI 170 BYTE, e lo dice l'azzeramento: uno `str xzr`
 * ("f9001fff str"@0xffffff8008a5d250), dieci `stp xzr, xzr`
 * (da "a9047fff stp"@0xffffff8008a5d24c a "a90d7fff stp"@0xffffff8008a5d228)
 * e una `strh wzr` ("7901c3ff strh"@0xffffff8008a5d224): 8 + 160 + 2 = 170.
 *
 * LA COPIA E' UN CICLO NEL SORGENTE, non una `memcpy`. Due indizi
 * concordano: la guardia `len >= 1` con confronto CON SEGNO
 * ("7100043f cmp"@0xffffff8008a5d218 + "540000ab b.lt"@0xffffff8008a5d264)
 * e la taglia passata ESTESA A ZERO ("2a1303e2 mov"@0xffffff8008a5d268), non
 * con segno. Una `memcpy(buf, dato, len)` con `len` intero darebbe una
 * `sxtw` e nessuna guardia; il riconoscimento di un ciclo da parte di clang
 * da' esattamente questa coppia.
 *
 * LA SCALA QUI E' UNA DIVISIONE, NON UNO SCORRIMENTO, e la differenza e'
 * misurata: "111ffd2a add"@0xffffff8008a5d560 somma 2047 e
 * "1a89b149 csel"@0xffffff8008a5d568 lo fa solo se il prodotto e' negativo.
 * E' la forma con cui clang rende `/ 2048` con segno. In
 * `ilitek_tddi_report_ap_mode` la stessa scala e' invece uno `>> 11` puro,
 * senza bias ("530b7d29 lsr"@0xffffff8008a5c75c): DUE ESPRESSIONI DIVERSE
 * nel sorgente di fabbrica, e la dimensione lo denuncia (tre istruzioni
 * contro una).
 *
 * LA TABELLA DI SALTO a 0xffffff8008f7ec28 NON e' un dato del driver: e' la
 * tabella compatta che clang genera per uno `switch` con indici 0x58..0x6f
 * ("51016148 sub"@0xffffff8008a5d398, "71005d1f cmp"@0xffffff8008a5d39c,
 * "3868692b ldrb"@0xffffff8008a5d3b8, "d61f0140 br"@0xffffff8008a5d3c0).
 * Si riproduce scrivendo lo `switch`, non la tabella.
 */
void ilitek_tddi_report_gesture_mode(u8 *dato, int len)
{
	struct ilitek_gesto *g;
	struct input_dev *in;
	int transfer;
	u8 buf[170];
	int i;
	u8 codice;
	u16 xa, ya, xb, yb;

	/* "a90d7fff stp"@0xffffff8008a5d228 e le altre */
	memset(buf, 0x0, sizeof(buf));

	/*
	 * I TRE CAMPI SI LEGGONO PRIMA DELLA COPIA, e non e' stile: nel
	 * binario "f9452308 ldr"@0xffffff8008a5d220 (il puntatore `idev`),
	 * "f9402117 ldr"@0xffffff8008a5d254, "f9400916 ldr"@0xffffff8008a5d258
	 * e "394a0515 ldrb"@0xffffff8008a5d25c stanno TUTTI PRIMA di
	 * "940feaa3 bl"@0xffffff8008a5d274, la copia. Mettendoli dopo, questo
	 * stesso compilatore lascia il carico di `idev` dopo la copia e cambia
	 * ottanta parole a dimensione invariata. Misurato.
	 *
	 * TRE CAMPI DI `idev` FINISCONO IN LOCALI, e non e' stile: il binario
	 * li legge TUTTI E TRE da un solo `idev`
	 * ("f9452308 ldr"@0xffffff8008a5d220) e li tiene in registri SALVATI
	 * DAL CHIAMATO attraverso tutte le chiamate --
	 * "f9402117 ldr"@0xffffff8008a5d254 (+64, in x23),
	 * "f9400916 ldr"@0xffffff8008a5d258 (+16, in x22),
	 * "394a0515 ldrb"@0xffffff8008a5d25c (+641, in w21). Le quattro
	 * `input_event` del caso 0x58 usano "aa1603e0 mov"@0xffffff8008a5d3e8
	 * e non rileggono niente, e le cinque prove di `c641` sono cinque
	 * `cbnz w21` ("35001495 cbnz"@0xffffff8008a5d444 e le altre).
	 * Il nome `transfer` NON e' scelto: lo da' il formato
	 * "Transfer = %d, Type = %d, clockwise = %d". Quello di `in` si'.
	 */
	g = idev->c64;
	in = idev->c16;
	transfer = idev->c641;

	for (i = 0; i < len; i++)
		buf[i] = dato[i];

	/* "f90002ff str"@0xffffff8008a5d284 e le tre `stp` che la precedono */
	memset(g, 0x0, sizeof(*g));

	/* "790002e3 strh"@0xffffff8008a5d298 */
	codice = buf[1];
	g->c0 = codice;
	/* "\x016ILITEK: (%s, %d): gesture code = 0x%x, score = %d\n"@0xffffff8009242bb5,
	 * __func__ "ilitek_tddi_report_gesture_mode"@0xffffff8009242bea,
	 * riga 932, w3 = buf[1] e w4 = buf[36]
	 * ("394173e4 ldrb"@0xffffff8008a5d29c) */
#line 932
	ILI_INFO("gesture code = 0x%x, score = %d\n", codice, buf[36]);

	/* "33180d49 bfi"@0xffffff8008a5d2bc e la coppia
	 * "121c0d0a and"@0xffffff8008a5d2b4 + "53047d4a lsr"@0xffffff8008a5d2b8 */
	g->pos[0].c2 = ((buf[4] & 0xf0) << 4) | buf[5];
	/* "12000d08 and"@0xffffff8008a5d2c8 + "33180d09 bfi"@0xffffff8008a5d2cc */
	g->pos[0].c4 = ((buf[4] & 0x0f) << 8) | buf[6];
	g->pos[1].c2 = ((buf[7] & 0xf0) << 4) | buf[8];
	g->pos[1].c4 = ((buf[7] & 0x0f) << 8) | buf[9];
	g->pos[2].c2 = ((buf[16] & 0xf0) << 4) | buf[17];
	g->pos[2].c4 = ((buf[16] & 0x0f) << 8) | buf[18];
	g->pos[3].c2 = ((buf[19] & 0xf0) << 4) | buf[20];
	g->pos[3].c4 = ((buf[19] & 0x0f) << 8) | buf[21];
	g->pos[4].c2 = ((buf[22] & 0xf0) << 4) | buf[23];
	g->pos[4].c4 = ((buf[22] & 0x0f) << 8) | buf[24];
	g->pos[5].c2 = ((buf[25] & 0xf0) << 4) | buf[26];
	g->pos[5].c4 = ((buf[25] & 0x0f) << 8) | buf[27];

	switch (g->c0) {
	case 0x58:
		/* "\x016ILITEK: (%s, %d): Double Click key event\n"@0xffffff8009242c0a,
		 * riga 950 */
#line 950
		ILI_INFO("Double Click key event\n");
		/* "97ffa504 bl"@0xffffff8008a5d3f0, w2 = 116 = KEY_POWER
		 * ("52800e82 mov"@0xffffff8008a5d3e0) */
		input_report_key(in, KEY_POWER, 1);
		/* "97ffa4ff bl"@0xffffff8008a5d404 */
		input_sync(in);
		/* "97ffa4fa bl"@0xffffff8008a5d418 */
		input_report_key(in, KEY_POWER, 0);
		/* "97ffa4f5 bl"@0xffffff8008a5d42c */
		input_sync(in);
		/* "b90006e9 str"@0xffffff8008a5d438 con w9 = 0x58 */
		g->c4 = 0x58;
		/* "39000af9 strb"@0xffffff8008a5d43c */
		g->c2 = 1;
		/* "b840a2e8 ldur"@0xffffff8008a5d430 +
		 * "b80122e8 stur"@0xffffff8008a5d440: clang fonde le due `u16`
		 * adiacenti in una parola sola */
		g->pos[1].c2 = g->pos[0].c2;
		g->pos[1].c4 = g->pos[0].c4;
		break;
	/*
	 * ===================================================================
	 * CORREZIONE DEL 2026-08-21: I CASI 0x60..0x68 NON FANNO IL CERCHIO
	 * ===================================================================
	 * Il lotto precedente dava a tutti e dieci i casi 0x60..0x69 lo stesso
	 * corpo (una macro `ILITEK_GESTO_CERCHIO`). E' SBAGLIATO, e la misura
	 * di DIMENSIONE non lo denunciava: il conto delle istruzioni e
	 * identico nei due modi, perche' in entrambi ci sono dieci trampolini
	 * `mov`+`b`, un corpo lungo e una coda corta.
	 *
	 * LA PROVA E' LA TABELLA DI SALTO, letta dal binario. Sta a
	 * 0xffffff8008f7ec28, la base dei rami e 0xffffff8008a5d3c4
	 * ("1000008a adr"@0xffffff8008a5d3b4) e l'indice e `codice - 0x58`
	 * ("51016148 sub"@0xffffff8008a5d398,
	 * "3868692b ldrb"@0xffffff8008a5d3b8,
	 * "8b0b094a add"@0xffffff8008a5d3bc, "d61f0140 br"@0xffffff8008a5d3c0):
	 *
	 *   $ ./venv/bin/python3 - <<EOF   # legge i 24 byte con pyelftools
	 *   tabella: 00222222222222222a2c2e30323436383a3c5c5e222222f5
	 *   caso 0x60 -> voce  42 -> 0xffffff8008a5d46c
	 *   caso 0x61 -> voce  44 -> 0xffffff8008a5d474
	 *   [...]
	 *   caso 0x68 -> voce  58 -> 0xffffff8008a5d4ac
	 *   caso 0x69 -> voce  60 -> 0xffffff8008a5d4b4
	 *   caso 0x6a -> voce  92 -> 0xffffff8008a5d534
	 *   caso 0x6b -> voce  94 -> 0xffffff8008a5d53c
	 *   caso 0x6f -> voce 245 -> 0xffffff8008a5d798
	 *   EOF
	 *
	 * e i nove rami 0x60..0x68 finiscono TUTTI con un salto alla coda
	 * corta ("14000034 b"@0xffffff8008a5d470 e le otto gemelle, bersaglio
	 * 0xffffff8008a5d540 = "320003e9 orr"+"b90006e8 str"+"39000ae9 strb",
	 * cioe' `c4 = codice; c2 = 1`), la STESSA coda dei casi 0x6a e 0x6b.
	 * SOLO il caso 0x69 ("52800d28 mov"@0xffffff8008a5d4b4) prosegue per
	 * caduta nel corpo lungo ("b90006e8 str"@0xffffff8008a5d4b8), che e'
	 * quello dei quattro punti.
	 *
	 * DIECI COSTANTI DIVERSE, non un caso solo che riusi `g->c0`: il
	 * binario ha dieci `mov w8, #imm` distinte -- "321b07e8 orr"@0xffffff8008a5d46c
	 * (0x60), "52800c28 mov"@0xffffff8008a5d474 (0x61), ...,
	 * "52800d28 mov"@0xffffff8008a5d4b4 (0x69). Con un solo caso che
	 * scriva `g->c0` quelle dieci `mov` non ci sarebbero.
	 * CHE il sorgente di fabbrica avesse nove corpi scritti per esteso o
	 * una macro NON e' deciso dal binario: la macro qui e' una scelta di
	 * scrittura, dichiarata, e produce lo stesso codice.
	 */
#define ILITEK_GESTO_SEMPLICE(codice)					\
	do {								\
		g->c4 = (codice);					\
		g->c2 = 1;						\
	} while (0)

	case 0x60:
		ILITEK_GESTO_SEMPLICE(0x60);
		break;
	case 0x61:
		ILITEK_GESTO_SEMPLICE(0x61);
		break;
	case 0x62:
		ILITEK_GESTO_SEMPLICE(0x62);
		break;
	case 0x63:
		ILITEK_GESTO_SEMPLICE(0x63);
		break;
	case 0x64:
		ILITEK_GESTO_SEMPLICE(0x64);
		break;
	case 0x65:
		ILITEK_GESTO_SEMPLICE(0x65);
		break;
	case 0x66:
		ILITEK_GESTO_SEMPLICE(0x66);
		break;
	case 0x67:
		ILITEK_GESTO_SEMPLICE(0x67);
		break;
	case 0x68:
		ILITEK_GESTO_SEMPLICE(0x68);
		break;
#undef ILITEK_GESTO_SEMPLICE
	case 0x69:
		/* "52800d28 mov"@0xffffff8008a5d4b4 +
		 * "b90006e8 str"@0xffffff8008a5d4b8 */
		g->c4 = 0x69;
		/* "1a8883e8 csel"@0xffffff8008a5d4c4, condizione `hi`:
		 * confronto SENZA segno con 1. `buf[34]` sta a sp+90
		 * ("39416be8 ldrb"@0xffffff8008a5d4bc) e il buffer parte da
		 * sp+56 ("9100e3e0 add"@0xffffff8008a5d26c): 90-56 = 34. */
		g->c2 = (buf[34] > 1) ? 0 : buf[34];
		/* "53047d08 lsr"@0xffffff8008a5d4e8 +
		 * "33180d09 bfi"@0xffffff8008a5d4ec */
		xa = ((buf[28] >> 4) << 8) | buf[29];
		/* "33180d0a bfi"@0xffffff8008a5d4e4 */
		ya = ((buf[28] & 0x0f) << 8) | buf[30];
		/* "53047d68 lsr"@0xffffff8008a5d4f0 +
		 * "33180d0c bfi"@0xffffff8008a5d4f8 */
		xb = ((buf[31] >> 4) << 8) | buf[32];
		/* "33180d6d bfi"@0xffffff8008a5d4f4 */
		yb = ((buf[31] & 0x0f) << 8) | buf[33];
		/* quattro punti costruiti da due, con due medie:
		 * "0b0a01a8 add"@0xffffff8008a5d500 +
		 * "53017d08 lsr"@0xffffff8008a5d50c e
		 * "0b09018a add"@0xffffff8008a5d504 +
		 * "53017d49 lsr"@0xffffff8008a5d510 */
		g->pos[2].c2 = (xb + xa) >> 1;
		g->pos[2].c4 = ya;
		g->pos[3].c2 = xa;
		g->pos[3].c4 = (yb + ya) >> 1;
		g->pos[4].c2 = (xb + xa) >> 1;
		g->pos[4].c4 = yb;
		g->pos[5].c2 = xb;
		g->pos[5].c4 = (yb + ya) >> 1;
		break;
	case 0x6a:
		/* "52800d48 mov"@0xffffff8008a5d534, poi la coda condivisa
		 * "b90006e8 str"@0xffffff8008a5d544 +
		 * "39000ae9 strb"@0xffffff8008a5d548 */
		g->c4 = 0x6a;
		g->c2 = 1;
		break;
	case 0x6b:
		/* "52800d68 mov"@0xffffff8008a5d53c */
		g->c4 = 0x6b;
		g->c2 = 1;
		break;
	case 0x6f:
		/* "52800de8 mov"@0xffffff8008a5d798 */
		g->c4 = 0x6f;
		g->c2 = 1;
		/* "33180d49 bfi"@0xffffff8008a5d7b8 e le tre seguenti */
		g->pos[2].c2 = ((buf[10] & 0xf0) << 4) | buf[11];
		g->pos[2].c4 = ((buf[10] & 0x0f) << 8) | buf[12];
		g->pos[3].c2 = ((buf[13] & 0xf0) << 4) | buf[14];
		g->pos[3].c4 = ((buf[13] & 0x0f) << 8) | buf[15];
		break;
	default:
		/* "\x013ILITEK: (%s, %d): Unknown gesture code\n"@0xffffff8009242c36,
		 * riga 1031 */
#line 1031
		ILI_ERR("Unknown gesture code\n");
		break;
	}

	/* "394a0515 ldrb"@0xffffff8008a5d25c (idev+641), letto UNA volta in
	 * testa alla funzione e riusato da tutti i rami
	 * ("35001495 cbnz"@0xffffff8008a5d444 e le sue quattro gemelle) */
	if (!transfer) {
		struct ilitek_tddi_dev *d = idev;

		for (i = 0; i < 6; i++) {
			/* "1b097d49 mul"@0xffffff8008a5d55c +
			 * "111ffd2a add"@0xffffff8008a5d560 +
			 * "1a89b149 csel"@0xffffff8008a5d568 +
			 * "530b7d29 lsr"@0xffffff8008a5d56c */
			g->pos[i].c2 = g->pos[i].c2 * d->c244 / 2048;
			g->pos[i].c4 = g->pos[i].c4 * d->c246 / 2048;
		}
	}

	/* "\x016ILITEK: (%s, %d): Transfer = %d, Type = %d, clockwise = %d\n"@0xffffff8009242c60,
	 * riga 1050 */
	ILI_INFO("Transfer = %d, Type = %d, clockwise = %d\n",
#line 1050
		 transfer, g->c4, g->c2);

	/* "\x016ILITEK: (%s, %d): Gesture Points: (%d, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)\n"@0xffffff8009242c9e,
	 * riga 1057. Dodici argomenti: i primi cinque nei registri, i sette
	 * restanti sulla pila ("b90003e8 str"@0xffffff8008a5d754 e le sei
	 * seguenti). */
	ILI_INFO("Gesture Points: (%d, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)(%d, %d)\n",
		 g->pos[0].c2, g->pos[0].c4, g->pos[1].c2, g->pos[1].c4,
		 g->pos[2].c2, g->pos[2].c4, g->pos[3].c2, g->pos[3].c4,
#line 1057
		 g->pos[4].c2, g->pos[4].c4, g->pos[5].c2, g->pos[5].c4);

	/* "97fffd56 bl"@0xffffff8008a5d764 */
	ilitek_tddi_touch_send_debug_data(dato, len);
}

/*
 * Gruppo D. "97ffeeeb bl"@0xffffff8008a5b1a0: w0 un indirizzo a 32 bit, w1
 * una maschera, w2 il valore; il ritorno e' provato sul solo bit di segno
 * ("36f80120 tbz"@0xffffff8008a5b1a4).
 */
int ilitek_ice_mode_bit_mask_write(u32 addr, u32 maschera, u32 valore);
/*
 * Gruppo G. "9400306e bl"@0xffffff8008a5b324: nessun argomento, ritorno non
 * usato.
 */
int ilitek_tddi_flash_clear_dma(void);

/*
 * ===========================================================================
 * dma_clear_reg_setting -- 0xffffff8008a5b17c, 440 byte, `static`
 * ===========================================================================
 * `static` DALLA MAPPA (`t` minuscolo), non una scelta.
 *
 * Righe di fabbrica: 112, 116, 120, 122, 124, 128, 130
 * ("321c0be2 orr"@0xffffff8008a5b1b8, "52800e82 mov"@0xffffff8008a5b1f0,
 * "321d0fe2 orr"@0xffffff8008a5b230, "52800f42 mov"@0xffffff8008a5b268,
 * "321e13e2 orr"@0xffffff8008a5b2a0, "321903e2 orr"@0xffffff8008a5b2d8,
 * "52801042 mov"@0xffffff8008a5b314).
 *
 * GLI INDIRIZZI SONO COSTANTI LETTERALI, non un base piu' scostamento: il
 * binario materializza 0x48004 ("52900094 mov"@0xffffff8008a5b188 +
 * "72a00094 movk"@0xffffff8008a5b18c) e ne DERIVA gli altri due con
 * "1101f293 add"@0xffffff8008a5b190 (+0x7c) e
 * "11001293 add"@0xffffff8008a5b1c8 (+4), e lo stesso fa con 0x720c8
 * ("52841914 mov"@0xffffff8008a5b200, poi
 * "51001293 sub"@0xffffff8008a5b208 e "51002293 sub"@0xffffff8008a5b2b0).
 * E' il modo in cui clang riusa una costante gia' in registro, non una
 * struttura del sorgente.
 *
 * IL DIFETTO DELLA FABBRICA CHE SI RIPRODUCE (regola 7): il formato
 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 usa `%lu` -- un `unsigned
 * long`, 64 bit -- e riceve una costante che il binario mette in un
 * registro a 32 BIT ("320807e3 orr"@0xffffff8008a5b1bc e le sue quattro
 * gemelle). Il valore stampato viene giusto lo stesso, perche' scrivere w3
 * azzera la meta' alta di x3; il TIPO no.
 * DIVERGENZA DI SORGENTE DICHIARATA: qui le costanti portano il suffisso
 * `UL` perche' altrimenti `-Wformat` (che `-Wall` accende e che questo lotto
 * compila con `-Werror`) rifiuta il file. Il codice macchina e' lo stesso --
 * un `unsigned long` il cui valore sta in 32 bit si materializza con la
 * stessa istruzione -- ma il sorgente NON e' necessariamente quello di
 * fabbrica: la fabbrica ha compilato con l'avviso acceso e senza `-Werror`.
 */
static void dma_clear_reg_setting(void)
{
	/* "97ffeeeb bl"@0xffffff8008a5b1a0 */
	if (ilitek_ice_mode_bit_mask_write(0x48080, 0x3000000, 0) < 0)
		/* riga 112 */
#line 112
		ILI_ERR("Write %lu at %x failed\n", 0x3000000UL, 0x48080);

	/* "97ffeedd bl"@0xffffff8008a5b1d8 */
	if (ilitek_ice_mode_bit_mask_write(0x48008, 0x40000, 0x40000) < 0)
		/* riga 116 */
#line 116
		ILI_ERR("Write %lu at %x failed\n", 0x40000UL, 0x48008);

	/* "97ffefa0 bl"@0xffffff8008a5b218, w2 = 4 */
	if (ilitek_ice_mode_write(0x720c4, 0x0, 4) < 0)
		/* "\x013ILITEK: (%s, %d): Write 0x00000000 at %x failed\n"@0xffffff8009242e9b,
		 * riga 120 */
#line 120
		ILI_ERR("Write 0x00000000 at %x failed\n", 0x720c4);

	/* "97ffef93 bl"@0xffffff8008a5b24c, w2 = 1 */
	if (ilitek_ice_mode_write(0x720c8, 0x0, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Write 0x0 at %x failed\n"@0xffffff8009242ece,
		 * riga 122 */
#line 122
		ILI_ERR("Write 0x0 at %x failed\n", 0x720c8);

	/* "97ffeeb2 bl"@0xffffff8008a5b284 */
	if (ilitek_ice_mode_bit_mask_write(0x720c8, 0x83000000, 0x80000000) < 0)
		/* riga 124 */
#line 124
		ILI_ERR("Write %lu at %x failed\n", 0x83000000UL, 0x720c8);

	/* "97ffeea3 bl"@0xffffff8008a5b2c0 */
	if (ilitek_ice_mode_bit_mask_write(0x720c0, 0xf0000, 0) < 0)
		/* riga 128 */
#line 128
		ILI_ERR("Write %lu at %x failed\n", 0xf0000UL, 0x720c0);

	/* "97ffee95 bl"@0xffffff8008a5b2f8 */
	if (ilitek_ice_mode_bit_mask_write(0x48004, 0x2000000, 0x2000000) < 0)
		/* riga 130 */
#line 130
		ILI_ERR("Write %lu at %x failed\n", 0x2000000UL, 0x48004);

	/* "9400306e bl"@0xffffff8008a5b324 */
	ilitek_tddi_flash_clear_dma();
}

/*
 * Gruppo G. "9400303d bl"@0xffffff8008a5b628: tre interi a 32 bit
 * ("0b140261 add"@0xffffff8008a5b61c calcola il secondo come somma degli
 * altri due), ritorno non usato.
 */
int ilitek_tddi_flash_dma_write(u32 inizio, u32 fine, u32 len);
/*
 * Gruppo D. "97fff9ba bl"@0xffffff8008a5adec: due interi
 * ("52800640 mov"@0xffffff8008a5ade0 e "52800641 mov"@0xffffff8008a5ade4,
 * cioe' 50 e 50; l'altro sito passa 300 e 50), ritorno provato sul bit di
 * segno.
 */
int ilitek_tddi_ic_check_busy(int conta, int ritardo);

/*
 * ===========================================================================
 * dma_trigger_reg_setting -- 0xffffff8008a5b334, 1328 byte, `static`
 * ===========================================================================
 * `static` DALLA MAPPA. Diciannove messaggi, righe 143, 147, 149, 151, 155,
 * 159, 161, 163, 167, 169, 173, 177, 184, 186, 190, 195, 200, 211, 215.
 *
 * I TRE PARAMETRI sono nominati dal formato di riga 211,
 * "DMA fail: Regsiter = 0x%x Flash = 0x%x, Size = %d" (il refuso `Regsiter`
 * e' della fabbrica), che riceve w3 = w21, w4 = w20, w5 = w19, cioe' i tre
 * argomenti nell'ordine in cui arrivano
 * ("2a0003f5 mov"@0xffffff8008a5b35c, "2a0103f4 mov"@0xffffff8008a5b368,
 * "2a0203f3 mov"@0xffffff8008a5b364).
 *
 * DUE DIFETTI DELLA FABBRICA CHE SI RIPRODUCONO (regola 7):
 *  1. a riga 163 la chiamata e' `ilitek_ice_mode_write`
 *     ("97ffeee3 bl"@0xffffff8008a5b50c) con 0x83000000 come DATO e
 *     0x80000000 come LUNGHEZZA ("320103e2 orr"@0xffffff8008a5b504). Tutte
 *     le altre coppie (0x83000000, 0x80000000) del file vanno a
 *     `ilitek_ice_mode_bit_mask_write`, dove sono maschera e valore:
 *     "97ffee46 bl"@0xffffff8008a5b434 lo fa quattordici istruzioni prima
 *     sullo stesso indirizzo. Una lunghezza di 0x80000000 byte non ha
 *     senso: e' una chiamata sbagliata, ed e' nel binario.
 *  2. il formato "Write %lu at %x failed" con argomenti a 32 bit -- vedi il
 *     cappello di `dma_clear_reg_setting` per la divergenza di sorgente
 *     dichiarata (`UL`).
 *
 * IL CICLO DI ATTESA fa TRENTA giri: il contatore parte da -30
 * ("128003ba mov"@0xffffff8008a5b6dc) e sale con
 * "3100075a adds"@0xffffff8008a5b738 finche' non riporta
 * ("54fffda3 b.cc"@0xffffff8008a5b73c). L'uscita per successo prova il BIT
 * 17 della parola letta -- "39401be8 ldrb"@0xffffff8008a5b724 legge il byte
 * di scostamento 2 e "37080308 tbnz"@0xffffff8008a5b728 ne prova il bit 1 --
 * cioe' `stato & 0x20000`.
 *
 * IL RITARDO FINALE e' `mdelay(10)`: DIECI `__const_udelay(0x418958)`
 * consecutive, da "940ff0ae bl"@0xffffff8008a5b7bc a
 * "940ff093 bl"@0xffffff8008a5b828. Non e' srotolamento mio: `mdelay(10)`
 * con 10 > 5 si espande in `while (__ms--) udelay(1000);`, e clang srotola
 * un ciclo a conteggio noto e corpo minuscolo.
 */
static void dma_trigger_reg_setting(u32 reg, u32 flash, u32 size)
{
	int i;
	u32 stato = 0;

	/* "97ffee71 bl"@0xffffff8008a5b388 */
	if (ilitek_ice_mode_bit_mask_write(0x720c0, 0x2000000, 0x2000000) < 0)
		/* riga 143 */
#line 143
		ILI_ERR("Write %lu at %x failed\n", 0x2000000UL, 0x720c0);

	/* "97ffef32 bl"@0xffffff8008a5b3d0, dato 0x41010
	 * ("52820201 mov"@0xffffff8008a5b3b8 + "72a00081 movk"@0xffffff8008a5b3c0) */
	if (ilitek_ice_mode_write(0x720c4, 0x41010, 4) < 0)
		/* "\x013ILITEK: (%s, %d): Write 0x00041010 at %x failed\n"@0xffffff8009242f12,
		 * riga 147 */
#line 147
		ILI_ERR("Write 0x00041010 at %x failed\n", 0x720c4);

	/* "97ffef25 bl"@0xffffff8008a5b404 */
	if (ilitek_ice_mode_write(0x720c8, 0x00, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Write 0x00 at %x failed\n"@0xffffff8009242f45,
		 * riga 149 */
#line 149
		ILI_ERR("Write 0x00 at %x failed\n", 0x720c8);

	/* "97ffee46 bl"@0xffffff8008a5b434 */
	if (ilitek_ice_mode_bit_mask_write(0x720c8, 0x83000000, 0x80000000) < 0)
		/* riga 151 */
#line 151
		ILI_ERR("Write %lu at %x failed\n", 0x83000000UL, 0x720c8);

	/* "97ffee38 bl"@0xffffff8008a5b46c */
	if (ilitek_ice_mode_bit_mask_write(0x720d0, 0x80000000, 0x00) < 0)
		/* riga 155 */
#line 155
		ILI_ERR("Write %lu at %x failed\n", 0x80000000UL, 0x720d0);

	/* "97ffeefd bl"@0xffffff8008a5b4a4, w2 = 3 */
	if (ilitek_ice_mode_write(0x720d4, reg, 3) < 0)
		/* "\x013ILITEK: (%s, %d): Write %x at %x failed\n"@0xffffff8009242f72,
		 * riga 159 */
#line 159
		ILI_ERR("Write %x at %x failed\n", reg, 0x720d4);

	/* "97ffeeef bl"@0xffffff8008a5b4dc */
	if (ilitek_ice_mode_write(0x720d8, 0x01, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Write 0x01 at %x failed\n"@0xffffff8009242f9d,
		 * riga 161 */
#line 161
		ILI_ERR("Write 0x01 at %x failed\n", 0x720d8);

	/* DIFETTO 1 DELLA FABBRICA: `write` invece di `bit_mask_write`.
	 * "97ffeee3 bl"@0xffffff8008a5b50c */
	if (ilitek_ice_mode_write(0x720d8, 0x83000000, 0x80000000) < 0)
		/* riga 163 */
#line 163
		ILI_ERR("Write %lu at %x failed\n", 0x83000000UL, 0x720d8);

	/* "97ffeed5 bl"@0xffffff8008a5b544 */
	if (ilitek_ice_mode_write(0x720dc, size, 4) < 0)
		/* riga 167 */
#line 167
		ILI_ERR("Write %x at %x failed\n", size, 0x720dc);

	/* "97ffedf5 bl"@0xffffff8008a5b578 */
	if (ilitek_ice_mode_bit_mask_write(0x720dc, 0xf000000, 0x00) < 0)
		/* riga 169 */
#line 169
		ILI_ERR("Write %lu at %x failed\n", 0xf000000UL, 0x720dc);

	/* "97ffede5 bl"@0xffffff8008a5b5b8, indirizzo derivato da 0x48004
	 * ("11020116 add"@0xffffff8008a5b5a8, +0x80) */
	if (ilitek_ice_mode_bit_mask_write(0x48084, 0x20000, 0x20000) < 0)
		/* riga 173 */
#line 173
		ILI_ERR("Write %lu at %x failed\n", 0x20000UL, 0x48084);

	/* "97ffedd7 bl"@0xffffff8008a5b5f0 */
	if (ilitek_ice_mode_bit_mask_write(0x720c0, 0xf0000, 0x10000) < 0)
		/* riga 177 */
#line 177
		ILI_ERR("Write %lu at %x failed\n", 0xf0000UL, 0x720c0);

	/* "9400303d bl"@0xffffff8008a5b628 */
	ilitek_tddi_flash_dma_write(flash, flash + size, size);

	/* "97ffedc4 bl"@0xffffff8008a5b63c */
	if (ilitek_ice_mode_bit_mask_write(0x48004, 0x2020000, 0x2020000) < 0)
		/* riga 184 */
#line 184
		ILI_ERR("Write %lu at %x failed\n", 0x2020000UL, 0x48004);

	/* "97ffedb5 bl"@0xffffff8008a5b678, indirizzo derivato da 0x41010
	 * ("11000f36 add"@0xffffff8008a5b668, +3) */
	if (ilitek_ice_mode_bit_mask_write(0x41013, 0x01, 0x01) < 0)
		/* riga 186 */
#line 186
		ILI_ERR("Write %lu at %x failed\n", 0x01UL, 0x41013);

	/* "97ffee7a bl"@0xffffff8008a5b6b0 */
	if (ilitek_ice_mode_write(0x41010, 0xff, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Trigger DMA failed\n"@0xffffff8009241e5f,
		 * riga 190 */
#line 190
		ILI_ERR("Trigger DMA failed\n");

	/* il contatore va da -30 a 0: trenta giri */
	for (i = 30; i > 0; i--) {
		/* "97ffedf8 bl"@0xffffff8008a5b700 */
		if (ilitek_ice_mode_read(0x48004, &stato, 4) < 0) {
			/* "\x013ILITEK: (%s, %d): Read 0x%x error\n"@0xffffff8009242fca,
			 * riga 195 */
#line 195
			ILI_ERR("Read 0x%x error\n", 0x48004);
			continue;
		}

		/* "\x016ILITEK: (%s, %d): fw dma stat = %x\n"@0xffffff8009242fef,
		 * riga 200, guardia "340000c8 cbz"@0xffffff8008a5b70c */
#line 200
		ILI_DBG("fw dma stat = %x\n", stato);

		/* "37080308 tbnz"@0xffffff8008a5b728 -- il bit 17 */
		if (stato & 0x20000)
			goto alto;

		/* "94106a28 bl"@0xffffff8008a5b734, 1000 e 1000 */
		usleep_range(1000, 1000);
	}

	/* "\x013ILITEK: (%s, %d): DMA fail: Regsiter = 0x%x Flash = 0x%x, Size = %d\n"@0xffffff8009243015,
	 * riga 211. `Regsiter` e' della fabbrica. */
	ILI_ERR("DMA fail: Regsiter = 0x%x Flash = 0x%x, Size = %d\n",
#line 211
		reg, flash, size);

alto:
	/* "51004320 sub"@0xffffff8008a5b788, cioe' 0x41010 - 0x10 = 0x41000 */
	if (ilitek_ice_mode_write(0x41000, 0x01, 1) < 0)
		/* "\x013ILITEK: (%s, %d): Pull CS High failed\n"@0xffffff8009241e87,
		 * riga 215 */
#line 215
		ILI_ERR("Pull CS High failed\n");

	/* dieci `__const_udelay(0x418958)`, da
	 * "940ff0ae bl"@0xffffff8008a5b7bc a "940ff093 bl"@0xffffff8008a5b828 */
	mdelay(10);
}

/*
 * ===========================================================================
 * ilitek_tddi_move_mp_code_flash -- 0xffffff8008a5ac64, 1304 byte
 * ===========================================================================
 * Righe di fabbrica: 241, 261, 276, 280, 285, 292, 298, 304, 314
 * ("52801e22 mov"@0xffffff8008a5acf0, "528020a2 mov"@0xffffff8008a5adc0,
 * "52802282 mov"@0xffffff8008a5ae20, "52802302 mov"@0xffffff8008a5ae40,
 * "528023a2 mov"@0xffffff8008a5ae68, "52802482 mov"@0xffffff8008a5aeb8,
 * "52802542 mov"@0xffffff8008a5b050, "52802602 mov"@0xffffff8008a5b0ec,
 * "52802742 mov"@0xffffff8008a5b134).
 *
 * IL BUFFER E' DI SEDICI BYTE: "a900ffff stp"@0xffffff8008a5ac9c azzera
 * sedici byte in un colpo, e la stessa istruzione ricompare a
 * 0xffffff8008a5acd8 per il secondo azzeramento.
 *
 * I QUATTRO CAMPI SI COMPONGONO DA TRE BYTE CIASCUNO, BIG-ENDIAN, e lo
 * dicono le otto `bfi` a passo 8 e 16 --
 * "33181d13 bfi"@0xffffff8008a5ad58 (byte 2 al posto 8) e
 * "33101d33 bfi"@0xffffff8008a5ad78 (byte 1 al posto 16) per il primo, e le
 * tre terne gemelle. I NOMI vengono dal formato di riga 261:
 * "MP info Overlay: Enable = %d, addr = 0x%x ~ 0x%x, flash addr = 0x%x, mp
 * size = 0x%x" riceve w3 = la bandiera, w4 e w5 i due estremi
 * dell'indirizzo, w6 l'indirizzo di flash, w7 la dimensione.
 * `addr_inizio` e `addr_fine` sono MIE letture dei due estremi: il formato
 * dice solo che sono un intervallo.
 *
 * LA BANDIERA E' UNA CONGIUNZIONE DI TRE PROVE, senza salti:
 * "1a9f07e8 cset"@0xffffff8008a5ad94 (addr_inizio != 0),
 * "1a9f07e9 cset"@0xffffff8008a5ad9c (addr_fine != 0),
 * "1a9f17e9 cset"@0xffffff8008a5ada8 (buf[0] == 0xfe), fuse da
 * "0a090108 and"@0xffffff8008a5ada4 e "0a080138 and"@0xffffff8008a5adac.
 * Un `&&` con corto circuito darebbe dei salti: qui e' l'operatore `&&` che
 * clang ha potuto rendere senza salti perche' nessun operando ha effetti.
 *
 * I DUE RITARDI del ramo senza sovrapposizione sono `mdelay(30)` e
 * `mdelay(10)`: TRENTA `__const_udelay(0x418958)` da
 * "940ff2eb bl"@0xffffff8008a5aec8 a "940ff294 bl"@0xffffff8008a5b024, e
 * DIECI da "940ff285 bl"@0xffffff8008a5b060 a
 * "940ff26a bl"@0xffffff8008a5b0cc.
 */
int ilitek_tddi_move_mp_code_flash(void)
{
	int ret = 0;
	u8 buf[16] = {0};
	u32 flash_addr, mp_size, addr_inizio, addr_fine;
	int overlay;

	/* "790013e9 strh"@0xffffff8008a5aca0 -- 0x1f0, cioe' 0xf0 e 0x01 */
	buf[0] = 0xf0;
	buf[1] = 0x01;
	/* "d63f0100 blr"@0xffffff8008a5acac, w1 = 2 */
	ret = idev->c776(buf, 2);
	/* "37f82480 tbnz"@0xffffff8008a5acb0 */
	if (ret < 0)
		goto fine;

	/* "390023e9 strb"@0xffffff8008a5acbc */
	buf[0] = 0xfe;
	/* "d63f0100 blr"@0xffffff8008a5accc, w1 = 1 */
	ret = idev->c776(buf, 1);
	if (ret < 0)
		goto fine;

	/* "a900ffff stp"@0xffffff8008a5acd8 */
	memset(buf, 0x0, sizeof(buf));

	/* "f9401d08 ldr"@0xffffff8008a5ace4 (idev+56) +
	 * "b9402903 ldr"@0xffffff8008a5acf4 (+40 dentro l'oggetto puntato).
	 * `ilitek.h` dichiara `idev->c56` come `void *`: DELTA DI HEADER, come
	 * per `c48`. Riga 241,
	 * "\x016ILITEK: (%s, %d): read mp info length = %d\n"@0xffffff8009241cfd */
	ILI_INFO("read mp info length = %d\n",
#line 241
		 *(int *)((u8 *)idev->c56 + 40));

	/* "f9418908 ldr"@0xffffff8008a5ad08 (idev+784) +
	 * "d63f0100 blr"@0xffffff8008a5ad10 */
	ret = idev->c784(buf, *(int *)((u8 *)idev->c56 + 40));
	if (ret < 0)
		goto fine;

	/* "97ffff68 bl"@0xffffff8008a5ad38, con "MP overlay info"@0xffffff8009241d4a
	 * in x4, 8 in w1 e 0 in w3 */
	ilitek_dump_data(buf, 8, *(int *)((u8 *)idev->c56 + 40), 0,
			 "MP overlay info");

	/* "33181d13 bfi"@0xffffff8008a5ad58 + "33101d33 bfi"@0xffffff8008a5ad78 */
	flash_addr = buf[3] | (buf[2] << 8) | (buf[1] << 16);
	/* "33181d36 bfi"@0xffffff8008a5ad60 + "33101d56 bfi"@0xffffff8008a5ad80 */
	mp_size = buf[6] | (buf[5] << 8) | (buf[4] << 16);
	/* "33181d54 bfi"@0xffffff8008a5ad68 + "33101d14 bfi"@0xffffff8008a5ad88 */
	addr_inizio = buf[9] | (buf[8] << 8) | (buf[7] << 16);
	/* "33181d15 bfi"@0xffffff8008a5ad70 + "33101d35 bfi"@0xffffff8008a5ad8c */
	addr_fine = buf[12] | (buf[11] << 8) | (buf[10] << 16);

	/* "0a080138 and"@0xffffff8008a5adac */
	overlay = (addr_inizio != 0) && (addr_fine != 0);
	overlay = (buf[0] == 0xfe) && overlay;

	/* "\x016ILITEK: (%s, %d): MP info Overlay: Enable = %d, addr = 0x%x ~ 0x%x, flash addr = 0x%x, mp size = 0x%x\n"@0xffffff8009241d5a,
	 * riga 261 */
	ILI_INFO("MP info Overlay: Enable = %d, addr = 0x%x ~ 0x%x, flash addr = 0x%x, mp size = 0x%x\n",
#line 261
		 overlay, addr_inizio, addr_fine, flash_addr, mp_size);

	/* "b901411f str"@0xffffff8008a5ade8 */
	idev->c320 = 0;
	/* "97fff9ba bl"@0xffffff8008a5adec */
	ret = ilitek_tddi_ic_check_busy(50, 50);
	/* "37f81a80 tbnz"@0xffffff8008a5adf0 */
	if (ret < 0)
		goto fine;

	/* "97fff0eb bl"@0xffffff8008a5adfc */
	ret = ilitek_ice_mode_ctrl(1, 0);
	if (ret < 0)
		goto fine;

	/* "34000458 cbz"@0xffffff8008a5ae04 */
	if (overlay) {
		/* "4b1502c8 sub"@0xffffff8008a5ae08 +
		 * "11000516 add"@0xffffff8008a5ae10 */
		mp_size = mp_size - addr_fine + 1;
		/* "\x016ILITEK: (%s, %d): MP andes init size = %d , MP text size = %d\n"@0xffffff8009241dc3,
		 * riga 276 */
		ILI_INFO("MP andes init size = %d , MP text size = %d\n",
#line 276
			 addr_inizio, mp_size);

		/* "940000d2 bl"@0xffffff8008a5ae34 */
		dma_clear_reg_setting();
		/* "\x016ILITEK: (%s, %d): [Move ANDES.INIT to DRAM]\n"@0xffffff8009241e04,
		 * riga 280 */
#line 280
		ILI_INFO("[Move ANDES.INIT to DRAM]\n");
		/* "94000137 bl"@0xffffff8008a5ae58 */
		dma_trigger_reg_setting(0, flash_addr, addr_inizio);
		/* "940000c8 bl"@0xffffff8008a5ae5c */
		dma_clear_reg_setting();

		/* "\x016ILITEK: (%s, %d): [Move MP.TEXT to DRAM]\n"@0xffffff8009241e33,
		 * riga 285 */
#line 285
		ILI_INFO("[Move MP.TEXT to DRAM]\n");
		/* "0b130281 add"@0xffffff8008a5ae74 +
		 * "9400012d bl"@0xffffff8008a5ae80 */
		dma_trigger_reg_setting(addr_fine, addr_inizio + flash_addr,
					mp_size);
		/* "940000be bl"@0xffffff8008a5ae84 */
		dma_clear_reg_setting();
	} else {
		/* "97fff07e bl"@0xffffff8008a5aea0, indirizzo derivato da
		 * 0x41000 ("11004100 add"@0xffffff8008a5ae94, +0x10) */
		if (ilitek_ice_mode_write(0x41010, 0xff, 1) < 0)
			/* riga 292 */
#line 292
			ILI_ERR("Trigger DMA failed\n");
		/* trenta `__const_udelay` */
		mdelay(30);
		/* "97fff018 bl"@0xffffff8008a5b038 */
		if (ilitek_ice_mode_write(0x41000, 0x01, 1) < 0)
			/* riga 298 */
			ILI_ERR("Pull CS High failed\n");
		/* dieci `__const_udelay` */
		mdelay(10);
	}

	/* "97ffe29d bl"@0xffffff8008a5b0d4, w0 = 1 */
	if (ilitek_tddi_reset_ctrl(1) < 0)
		/* "\x013ILITEK: (%s, %d): IC Code reset failed during moving mp code\n"@0xffffff8009241eb0,
		 * riga 304 */
#line 304
		ILI_ERR("IC Code reset failed during moving mp code\n");

	/* "97fff02b bl"@0xffffff8008a5b0fc */
	ret = ilitek_ice_mode_ctrl(0, 0);
	if (ret < 0)
		goto fine;

	/* "b9014109 str"@0xffffff8008a5b114 -- idev+320 = 1 */
	idev->c320 = 1;
	/* "97fff8ef bl"@0xffffff8008a5b118, 300 e 50 */
	ret = ilitek_tddi_ic_check_busy(300, 50);
	/* "36f80120 tbz"@0xffffff8008a5b120 */
	if (ret < 0)
		/* "\x013ILITEK: (%s, %d): Check cdc timeout failed after moved mp code\n"@0xffffff8009241ef0,
		 * riga 314 */
#line 314
		ILI_ERR("Check cdc timeout failed after moved mp code\n");

fine:
	/* "2a1703e0 mov"@0xffffff8008a5b158 */
	return ret;
}
