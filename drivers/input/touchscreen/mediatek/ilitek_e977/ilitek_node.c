// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_ILITEK_E977 -- il GRUPPO H: i nodi /proc, l'ioctl e il netlink.
 * Doogee S88 Pro (MediaTek MT6771). Il kernel di fabbrica e' un 4.14: il
 * percorso dei suoi sorgenti sta a 0xffffff80090e3c18 e finisce per
 * kernel-4.14/include/linux/thread_info.h (non e' un letterale di questo
 * file, quindi non e' citato nella forma verificabile).
 *
 * ===========================================================================
 * COSA CONTIENE, E DOVE FINISCE
 * ===========================================================================
 * Il gruppo H e' l'intervallo 0xffffff8008a6b528 .. 0xffffff8008a7221c
 * (estremo destro escluso): 28 funzioni, 27892 byte. QUESTO FILE LE SCRIVE
 * TUTTE E VENTOTTO. Le tre che il lotto precedente aveva lasciato dichiarate
 * e non definite -- `ilitek_node_ioctl_write` (4668 B),
 * `ilitek_node_ioctl` (5624 B) e `ilitek_node_compat_ioctl` (1840 B), 12132
 * byte, il 43,5% del blocco -- sono state scritte il 2026-08-21 e MISURANO
 * TUTTE E TRE ESATTAMENTE COME DI FABBRICA. Il file non ha piu' funzioni
 * dichiarate e non definite del gruppo H; cio' che resta indefinito sono le
 * funzioni di ALTRI gruppi che questo blocco chiama, ed e' l'esito onesto.
 * Vedi «COSA NON E' SCRITTO» in fondo.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA. Nessun sorgente
 * pubblico e' stato letto. Ogni riga derivata porta la citazione nella forma
 * "<codifica8hex> <mnemonico>"@0xINDIRIZZO per le istruzioni e
 * "testo"@0xINDIRIZZO per i letterali -- le due forme che
 * `tools/verificaistruzioni.py` e `tools/verificacitazioni.py` sanno
 * confrontare col binario.
 *
 * ===========================================================================
 * COME SI RIVERIFICA
 * ===========================================================================
 *   ./venv/bin/python3 verificacitazioni.py ilitek_node.c oracolo/stock.elf
 *   ./venv/bin/python3 verificaistruzioni.py ilitek_node.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a6b528:0xffffff8008a7221c \
 *       --intervallo 0xffffff80093aca94:0xffffff80093acc50
 *
 * Esito atteso al 2026-08-21 (dopo il lotto delle tre ioctl): 314
 * letterali, 316 citati, 316 verificati, 289 probanti, 27 deboli; 407
 * citazioni di istruzione, 407 confermate, 0 assenti, 0 controfattuali.
 * (I «citati» sono due piu' dei letterali perche' due sono nomi di funzione
 * riconosciuti dal `__func__`, non letterali scritti a mano.)
 *
 * IL SECONDO INTERVALLO SERVE, e non e' un dettaglio: e' la `.exit.text`,
 * che sta OLTRE `_einittext` (0xffffff80093a8518) e quindi oltre la fine di
 * `oracolo/stock.map`. Tre citazioni di questo file ci cadono dentro -- le
 * tre che PROVANO che il gruppo H non ha nessuna funzione `.exit` (vedi la
 * sezione apposita). Col solo intervallo del blocco escono ASSENTI, e
 * sarebbe un falso allarme.
 *
 * IL COMPILATORE CONTA: la misura va fatta con quello di FABBRICA.
 *   PATH=/srv/archive/kernel-stock/clang-r353983c/bin:... CC=clang
 * (con r383902 quattro funzioni divergono per sola deriva del compilatore:
 * vedi la sezione «IL METRO DI MISURA»).
 *
 * La misura di dimensione si fa dal `.o` VERO, non da una misura isolata:
 *   make -C <albero> O=<uscita> ARCH=arm64 CC=clang ... \
 *       drivers/input/touchscreen/mediatek/ilitek_e977/
 *   aarch64-linux-android-nm --print-size <uscita>/.../ilitek_node.o \
 *       | grep -iE " [tT] "
 *
 * ===========================================================================
 * IL METRO DI MISURA: IL COMPILATORE DI FABBRICA
 * ===========================================================================
 * Questo lotto e' misurato DUE volte, e i due numeri dicono cose diverse.
 * Il kernel di fabbrica e' stato compilato con clang r353983c (LLVM 9.0.3);
 * l'albero di questo progetto usa r383902 (LLVM 11.0.1). Il compilatore di
 * fabbrica e' stato recuperato il 2026-08-21 e sta in
 * /mnt/s88pro/kernel-stock/clang-r353983c.
 *
 *   col compilatore di FABBRICA (r353983c):  26 funzioni su 28 identiche
 *
 * La percentuale va letta con la numerosita' e l'intervallo, e L'AVVERTENZA
 * VIENE PRIMA DEL NUMERO: su 28 unita' la misura NON DISCRIMINA in nessuna
 * direzione rispetto al 77,10% del ramo.
 *   ./venv/bin/python3 intervallo.py 26 28
 *   26 su 28 = 92.9%   IC95% Clopper-Pearson [76.5%; 99.1%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 * Il denominatore e' ora quello del BLOCCO, perche' non ci sono piu'
 * funzioni non scritte. Sulle SOLE tre funzioni di questo lotto la misura
 * discrimina ancora meno:
 *   ./venv/bin/python3 intervallo.py 3 3
 *   3 su 3 = 100.0%   IC95% Clopper-Pearson [29.2%; 100.0%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 * Tre unita' non provano niente da sole: cio' che porta il peso non e' la
 * percentuale ma il confronto ISTRUZIONE PER ISTRUZIONE riportato sotto,
 * dove le tre funzioni hanno 1167, 1406 e 460 istruzioni e un istogramma
 * dei mnemonici a scarto ZERO su tutte e tre.
 *
 * QUATTRO divergenze che questo file aveva dichiarato come proprie SONO
 * SPARITE cambiando compilatore, cioe' non erano della ricostruzione:
 *   str2hex                          136 -> 144 (esatta)
 *   ilitek_debug_node_buff_control   456 -> 452 (esatta)
 *   debug_mode_get_data             1416 -> 1388 (esatta)
 *   ilitek_proc_rw_tp_reg_write      632 -> 636 (esatta)
 * Le cause misurate erano: la fusione dei due epiloghi `mov w0,#-1; ret` di
 * `str2hex` (r383902 li fonde, r353983c li duplica come la fabbrica --
 * "12800000 mov"@0xffffff8008a6b5a0 e "12800000 mov"@0xffffff8008a6b5a8) e
 * la rotazione dei cicli in `debug_mode_get_data` e
 * `ilitek_debug_node_buff_control` ("54fffe61 b.ne"@0xffffff8008a71c5c). Con
 * r353983c coincidono senza toccare una riga di sorgente.
 *
 * Sparisce anche la differenza di CODIFICA piu' pervasiva: r353983c emette
 * `orr wN, wzr, #imm` per gli immediati logici, esattamente come la fabbrica
 * ("320003e0 orr"@0xffffff8008a6d07c, "321f03e1 orr"@0xffffff8008a6b684),
 * mentre r383902 emette `movz`. Con r383902 ogni istogramma di questo lotto
 * mostrava `mov` in eccesso e `orr` in difetto della stessa quantita': era
 * deriva del compilatore, non un difetto.
 *
 * ===========================================================================
 * LE DIVERGENZE CHE RESTANO (col compilatore di fabbrica)
 * ===========================================================================
 * DUE funzioni su 28 non misurano come di fabbrica, ed entrambe erano gia'
 * cosi' prima di questo lotto (H1 e H2). Le tre funzioni scritte il
 * 2026-08-21 misurano tutte e tre ESATTE, e su tutte e tre l'istogramma dei
 * mnemonici ha scarto ZERO su ogni mnemonico -- il che e' piu' forte della
 * sola dimensione, perche' due funzioni della stessa lunghezza con
 * istruzioni diverse lo denuncerebbero:
 *   ilitek_node_ioctl_write   1167 istruzioni di fabbrica, 1167 nostre
 *   ilitek_node_ioctl         1406 istruzioni di fabbrica, 1406 nostre
 *   ilitek_node_compat_ioctl   460 istruzioni di fabbrica,  460 nostre
 * Cio' che RESTA diverso su quelle tre e' solo l'ORDINE (schedulazione) di
 * poche istruzioni e i valori dei `__LINE__` (divergenza H3), che nessuna
 * misura di dimensione vede:
 *   in `ilitek_node_ioctl_write` quattro istruzioni sono spostate --
 *   "f90017f5 str"@0xffffff8008a6ca68 (il cursore di `strsep`),
 *   "531e750a lsl"@0xffffff8008a6cae4 (dentro `str2hex` innestata) e
 *   "390093ff strb"@0xffffff8008a6d7ac (l'azzeramento del byte `ddi_data`);
 *   in `ilitek_node_ioctl` due -- "aa1403ea mov"@0xffffff8008a6df80 e
 *   "b9400909 ldr"@0xffffff8008a6df88 nel caso della `chip id`;
 *   in `ilitek_node_compat_ioctl` nessuna.
 * H1. `ilitek_proc_debug_message_read` misura 1408 byte contro 1404, cioe'
 *     UNA istruzione. L'istogramma dice `ldrb:+4 mul:+1 cmp:-2 cset:-1
 *     and:-1 tst:+1 lsl:+1`: la differenza sta tutta nell'abbassamento dello
 *     `switch (data[3] & 0x0F)`. La fabbrica prova l'appartenenza a {0,1,6}
 *     con una maschera ("5280086c mov"@0xffffff8008a6c3e4 carica 0x43,
 *     "6a0c017f tst"@0xffffff8008a6c3e8 la prova) e fa confluire i tre rami
 *     in UNA sola moltiplicazione-somma ("9b09291c madd"@0xffffff8008a6c3f4).
 *     Questo file scrive l'espressione per caso (`data[1]*data[2]*N + 6`) e
 *     ottiene la stessa `madd` condivisa piu' quattro `ldrb` in piu'.
 *     PROVATE anche le due forme alternative, e sono PEGGIO: col
 *     moltiplicatore in una variabile clang abbassa lo `switch` a una tabella
 *     di sette voci da 8 byte in `.rodata` (1356 byte, -12 istruzioni), col
 *     prodotto estratto prima dello `switch` sparisce la `madd` (1376 byte,
 *     -7). Attribuzione: 1 istruzione, abbassamento dello `switch`.
 * H2. `ilitek_proc_fw_get_raw_data_read` misura 1300 byte contro 1292, cioe'
 *     DUE istruzioni, ed e' tutta ALLOCAZIONE DEI REGISTRI: la fabbrica
 *     riversa un solo registro sulla pila, noi tre. L'istogramma dice
 *     `ldp:+2 ldr:-2 mov:+4 orr:-2` e il confronto istruzione per istruzione
 *     mostra solo tre `str`/`ldr` di riversamento in piu' e nessuna
 *     differenza di forma. La funzione GEMELLA
 *     (`ilitek_proc_get_delta_data_read`) misura esatta.
 *
 * H10. `ilitek_node_ioctl`, caso `chip id`: la fabbrica APPAIA le due
 *     letture agli scarti 40 e 44 e le due scritture sulla pila
 *     ("29452109 ldp"@0xffffff8008a6df90 e
 *     "2902a3e9 stp"@0xffffff8008a6df94). Scritti come tre assegnamenti
 *     consecutivi `buf[0]=..; buf[1]=..; buf[2]=..;` clang NON li appaia,
 *     perche' lo `str` intermedio sulla pila puo' aliasare il secondo `ldr`:
 *     misurato, 5632 byte invece di 5624. Letti prima in due locali,
 *     l'appaiamento torna e la funzione misura esatta. Il binario esclude
 *     la prima forma; NON dice quale delle altre stesse nel sorgente di
 *     fabbrica. Qui e' scritta la forma che riproduce.
 * H11. `ilitek_node_ioctl_write`, comando `rawdatarecore`: la fabbrica
 *     INNESTA i rami 0 e 2 di `get_tp_recore_ctrl` e CHIAMA la copia fuori
 *     linea per il ramo 1 ("97fff96b bl"@0xffffff8008a6d080). Scritto come
 *     `get_tp_recore_ctrl(data[1])` con l'argomento variabile clang non
 *     innesta niente: misurato, 4524 byte invece di 4668, cioe' 36
 *     istruzioni in meno, e la differenza sta TUTTA li'. Con lo `switch`
 *     sull'argomento e tre chiamate a costante l'innesto parziale torna
 *     esattamente com'e' di fabbrica, ramo per ramo. E' un fatto sul
 *     SORGENTE: al sito di chiamata l'argomento era una costante in due casi
 *     su tre, altrimenti il compilatore non avrebbe potuto propagarla.
 * H3. I `__LINE__`. Ogni `printk` del blocco porta il numero di riga del
 *     sorgente di fabbrica come terzo argomento; questo file NON li
 *     riproduce (come `ilitek_bus.c`, divergenza B1, e `ilitek_main.c`, D1).
 *     E' una divergenza reale nel binario e invisibile alla dimensione:
 *     `mov w2, #imm16` e' una istruzione per qualunque riga. I numeri di
 *     fabbrica sono noti uno per uno e stanno nel rapporto del lotto.
 * H4. L'IDENTITA' DI CODIFICA NON E' RAGGIUNTA, e va detto perche' la
 *     dimensione da sola non basta. Misurata su `str2hex`, l'unica funzione
 *     del lotto senza nessuna rilocazione (quindi l'unica su cui il confronto
 *     grezzo ha senso contro un `.o` non ancora collegato): 36 istruzioni,
 *     14 codifiche diverse, di cui ELEVEN sono solo nomi di registro
 *     (w9 contro w10, x10 contro x11) e TRE sono la stessa aritmetica
 *     riassociata -- la fabbrica calcola `(p - str) * 4 - 12`
 *     ("4b08014a sub"@0xffffff8008a6b55c poi
 *     "531e754a lsl"@0xffffff8008a6b560), noi `p*4 - 12 - str*4`. Sulle
 *     altre 22 funzioni il confronto grezzo non e' interpretabile: portano
 *     rilocazioni (adrp/add/bl), che in un `.o` valgono zero.
 * H5. `ilitek_debug_node_buff_control` e' marcata `static` (la mappa la da'
 *     `t`) e la fabbrica la chiama FUORI LINEA da
 *     `ilitek_proc_debug_switch_read` ("94001419 bl"@0xffffff8008a6c7f8)
 *     mentre la INNESTA due volte in `debug_mode_get_data` (dove l'argomento
 *     e' la costante `false`). Questo file riproduce quell'asimmetria senza
 *     nessun attributo: basta che i tre siti esistano. Se in futuro
 *     `debug_mode_get_data` venisse tolta, clang tornerebbe a innestarla
 *     nell'unico chiamante rimasto e `ilitek_proc_debug_switch_read`
 *     crescerebbe di 400 byte.
 * H6. `dev_mkdir` e `netlink_init` non esistono come simboli e sono scritte
 *     come funzioni `static` che il compilatore innesta. La prova che la
 *     scelta e' giusta e' indiretta ma forte: le tre funzioni ospiti
 *     (`ilitek_node_mp_lcm_on_test_read`, `..._off_...`,
 *     `ilitek_tddi_node_init`) misurano ESATTAMENTE come di fabbrica.
 * H7. `get_tp_recore_ctrl` e `str2hex` sono innestate dalla fabbrica dentro
 *     `ilitek_node_ioctl_write` e `ilitek_proc_get_debug_mode_data_write`
 *     (per `str2hex` il marcatore e' "7101819f cmp"@0xffffff8008a6cb08 e
 *     "7101819f cmp"@0xffffff8008a70c04). Qui `str2hex` risulta innestata
 *     dove la fabbrica la innesta; per `get_tp_recore_ctrl`
 *     l'innesto PARZIALE con propagazione di costante (rami 0 e 2 innestati,
 *     ramo 1 CHIAMATO -- "97fff96b bl"@0xffffff8008a6d080) E' RIPRODOTTO dal
 *     2026-08-21, e la forma sorgente che lo produce e' misurata: vedi H11.
 * H8. `katoi` NON risulta innestata da nessuna parte nel blocco: e' un
 *     risultato negativo confermato (nessuna `bl <katoi>` fra
 *     0xffffff8008a6b528 e 0xffffff8008a7221c, e le 35 che esistono nel
 *     binario stanno tutte nel gruppo del collaudo MP).
 * H9. `pr_cont` invece di `printk(KERN_CONT ...)` nelle due funzioni che
 *     stampano le righe di dati. Le due forme si espandono nella stessa
 *     cosa (`pr_cont(fmt, ...)` E' `printk(KERN_CONT fmt, ...)`) e la
 *     dimensione non cambia; la scrittura e' stata cambiata perche'
 *     `tools/verificacitazioni.py` va in eccezione su `KERN_CONT`, che manca
 *     dalla sua tabella `LIVELLO_KERN`. E' un difetto dello strumento,
 *     dichiarato qui e riportato nel rapporto, non aggirato in silenzio.
 *
 * ===========================================================================
 * I DIFETTI DELLA FABBRICA RIPRODOTTI (regola 7)
 * ===========================================================================
 * F1. `ilitek_proc_fw_process_read` copia in spazio utente `&idev->c444`
 *     invece del buffer che ha appena formattato: `__check_object_size` e
 *     `__arch_copy_to_user` ricevono `idev + 444`
 *     ("9106f116 add"@0xffffff8008a6fbb8, "aa1603e1 mov"@0xffffff8008a6fbfc)
 *     e non `g_user_buf`. Riprodotto.
 * F2. `get_tp_recore_data` esce con -ENODEV ("12800240 mov"@0xffffff8008a6b954)
 *     senza liberare il buffer da `kzalloc`: perdita di memoria vera.
 *     Riprodotta.
 * F3. `ilitek_proc_get_delta_data_read` esce con 0 sui due rami di
 *     allocazione fallita ("aa1f03e0 mov"@0xffffff8008a70030) SENZA
 *     `mutex_unlock`, senza riabilitare le due code di lavoro e senza
 *     `kfree` di quanto gia' allocato. Riprodotto.
 *     LA FUNZIONE GEMELLA NON HA QUESTO DIFETTO, e le due NON sono la stessa
 *     funzione con altre stringhe: in `ilitek_proc_fw_get_raw_data_read` gli
 *     stessi due rami saltano alla pulizia comune
 *     ("2a1f03f4 mov"@0xffffff8008a70590 azzera la lunghezza e cade in
 *     "f94522a8 ldr"@0xffffff8008a70594, che ricarica `idev` per lo
 *     `mutex_unlock`), e per questo le due `kfree` finali sono PROTETTE
 *     ("b4000079 cbz"@0xffffff8008a705b8) mentre nella gemella sono nude.
 *     E' una differenza DI SORGENTE fra due funzioni che tutti i rapporti
 *     precedenti trattavano come gemelle: scritte uguali, la seconda misura
 *     1280 byte invece di 1292.
 * F4. `ilitek_proc_rw_tp_reg_write` scrive `rw_reg[i]` senza confrontare `i`
 *     con nessun limite ("b8345b24 str"@0xffffff8008a70c34): con piu' di
 *     cinque campi separati da virgola scrive oltre l'array. Riprodotto.
 * F5. `file_write` passa 644 DECIMALE come modo a `filp_open`
 *     ("52805082 mov"@0xffffff8008a71a74, cioe' 0x284 = 644, non 0x1a4 =
 *     0644 ottale). Riprodotto.
 * F6. `ilitek_proc_debug_message_read` chiama `vfree` anche quando `vmalloc`
 *     ha restituito un ERR_PTR ("b4000075 cbz"@0xffffff8008a6c728 prova solo
 *     il NULL). Riprodotto.
 * F7. `ilitek_debug_node_buff_control(false)` ritorna -ENOMEM
 *     ("12800160 mov"@0xffffff8008a71a0c) anche quando non c'e' stato nessun
 *     errore: il ramo di spegnimento cade nello stesso epilogo. Riprodotto.
 * F8. `get_tp_recore_data` legge oltre il buffer letto: il ciclo di riversamento
 *     gira `row*col*2` volte leggendo due byte per giro da un buffer di
 *     `row*col*2+6` byte (vedi H4 e "f1000508 subs"@0xffffff8008a6fdc4).
 *     Riprodotto.
 * F9. `ilitek_node_ioctl_write` NON RILASCIA IL MUTEX sul ramo d'errore di
 *     `copy_from_user`. "17fffdb0 b"@0xffffff8008a6d2f4 salta alla `printk`
 *     condivisa con l'errore di lunghezza ("97db1ac8 bl"@0xffffff8008a6c9b4)
 *     e da li' "140001ea b"@0xffffff8008a6c9bc va DIRETTAMENTE al controllo
 *     del canarino: il `mutex_unlock` di "9410189b bl"@0xffffff8008a6d160
 *     non e' su quel percorso, e `idev->touch_mutex` resta preso per sempre.
 *     E' un blocco vero del dispositivo, non un dettaglio. Riprodotto.
 * F10. `ilitek_node_ioctl_write`, comando `spiw`: il byte di comando 0x82
 *     scritto in `temp[0]` ("52801048 mov"@0xffffff8008a6d8c0,
 *     "3900c3e8 strb"@0xffffff8008a6d8c4) e' SOVRASCRITTO dal primo giro del
 *     ciclo, che parte anch'esso dall'indice 0
 *     ("38366b68 strb"@0xffffff8008a6d90c). Sopravvive solo se la lunghezza
 *     e' zero. Riprodotto.
 * F11. `ilitek_node_ioctl_write`, comandi `spir` e `spirw`: la `kfree` del
 *     buffer di lettura sta anche sul ramo in cui l'allocazione e' FALLITA
 *     ("97db168a bl"@0xffffff8008a6daac cade su
 *     "97df8b3f bl"@0xffffff8008a6dab4, e "97db1692 bl"@0xffffff8008a6da8c
 *     salta a "97df8b1e bl"@0xffffff8008a6db38): con un ERR_PTR si libera un
 *     puntatore che non e' memoria. Riprodotto.
 * F12. `ilitek_node_ioctl`, comando a nr 8: legge QUATTRO byte dallo spazio
 *     utente ("321e03e2 orr"@0xffffff8008a6e810) e NON LI USA -- il livello
 *     diagnostico e' invertito senza guardarli
 *     ("52000103 eor"@0xffffff8008a6e830). Riprodotto.
 * F13. `ilitek_node_ioctl`, comando a nr 16: copia verso lo spazio utente
 *     QUATTRO byte ("321e03e2 orr"@0xffffff8008a6e0f0) a partire da un campo
 *     che ne occupa UNO ("39484903 ldrb"@0xffffff8008a6e0a4). Tre byte della
 *     struttura del driver finiscono nello spazio utente. Riprodotto.
 * F14. `ilitek_node_ioctl`, `default`: non stampa niente e non lascia
 *     traccia. Il blocco comune 0xffffff8008a6eeb4, raggiunto dai due `b.hi`
 *     ("54008808 b.hi"@0xffffff8008a6ddb4) e da tutte le voci mancanti delle
 *     due tabelle, comincia con "aa1303e0 mov"@0xffffff8008a6eeb4 e non
 *     contiene nessuna `bl <printk>`. Riprodotto.
 * F15. `ilitek_node_compat_ioctl`, `default`: il messaggio dice «return
 *     ilitek_node_ioctl» e il codice NON la chiama, ritorna -ENOIOCTLCMD
 *     ("92804040 mov"@0xffffff8008a6f228). Riprodotto com'e'.
 *
 * ===========================================================================
 * IL DELTA DI INTESTAZIONE (`ilitek.h`) -- MISURATO QUI, NON APPLICATO QUI
 * ===========================================================================
 * `ilitek.h` e' un file CONDIVISO: questo lotto non lo tocca. Cinque voci,
 * tutte misurate:
 *
 * D-H1. `+792` (`void *c792`). L'intestazione dice che la firma non e'
 *       scritta perche' "al solo sito noto il `blr` e' preceduto da un unico
 *       `ldr x0`". Quella lettura era incompleta: i TRE siti di lettura del
 *       campo stanno tutti in `ilitek_node_ioctl_write` e in tutti e tre
 *       preparano CINQUE argomenti, sempre nello stesso ordine --
 *       x0 = `idev->c8`, x1 = un buffer sulla pila, w2 = un conteggio a 32
 *       bit, x3 = un secondo puntatore, w4 = un secondo conteggio a 32 bit
 *       ("f9418d09 ldr"@0xffffff8008a6d934 col suo
 *       "d63f0120 blr"@0xffffff8008a6d940;
 *       "f9418d09 ldr"@0xffffff8008a6d984 col suo
 *       "d63f0120 blr"@0xffffff8008a6d998;
 *       "f9418d09 ldr"@0xffffff8008a6dad8 col suo
 *       "d63f0120 blr"@0xffffff8008a6dae8).
 *       Al primo sito x3 e w4 sono zero ("aa1f03e3 mov"@0xffffff8008a6d930 e
 *       "2a1f03e4 mov"@0xffffff8008a6d93c): solo scrittura.
 *       Il tipo di RITORNO resta non misurato: nessuno dei tre siti lo usa.
 *       La forma proposta e':
 *           int (*c792)(void *c8, const void *tx, u32 n_tx,
 *                       void *rx, u32 n_rx);
 *       con la nota che i cinque argomenti e le loro larghezze sono
 *       MISURATI, mentre il tipo di ritorno e i NOMI dei parametri sono una
 *       scelta. Che si tratti di `spi_write_then_read` e' un'inferenza dalla
 *       forma, non una misura: il binario non nomina la funzione, perche'
 *       nessuna delle 154 scrive il campo.
 * D-H2. `+568`: manca. Fra `c564` e `c592` c'e' un buco di 24 byte che e' una
 *       `wait_queue_head_t` ("9108e100 add"@0xffffff8008a71d64 e
 *       "9108e100 add"@0xffffff8008a6c310 la passano a
 *       `prepare_to_wait_event`). Qui e' raggiunta con un cast dichiarato
 *       (`IDEV_INQ`) invece che con un campo.
 * D-H3. `+592` (`void *c592`) SI CHIAMA `dbl`, e non e' un nome scelto: il
 *       formato a 0xffffff8009247ec4 e' "Failed to allocate idev->dbl mem".
 *       Punta a 1024 elementi di 16 byte con un byte a +0 e un puntatore a
 *       +8 (vedi `struct ilitek_dbl` qui sotto).
 * D-H4. `int ilitek_tddi_fw_upgrade_handler(void)` prende in realta' UN
 *       argomento, un puntatore, che l'unico sito noto mette a NULL
 *       ("aa1f03e0 mov"@0xffffff8008a6f90c). Qui e' aggirato con un cast
 *       dichiarato al sito di chiamata.
 * D-H6. `+527`, `+530`, `+532`, `+556`: quattro byte 0/1. `ilitek.h` gia'
 *       dice che `eor #1` su un byte appena letto e riscritto lascia
 *       indistinguibili `bool` con `x = !x` e `u8` con `x ^= 1`. Il gruppo H
 *       li tocca tutti e quattro nella stessa forma
 *       ("52000123 eor"@0xffffff8008a6d288 su +527,
 *       "52000123 eor"@0xffffff8008a6d430 su +530,
 *       "52000123 eor"@0xffffff8008a6d314 su +532,
 *       "52000123 eor"@0xffffff8008a6d48c su +556) e QUI sono scritti
 *       `^= 1`, che con `u8` produce esattamente quelle tre istruzioni. Se
 *       un lotto futuro decidesse di dichiararli `bool` in `ilitek.h`, le
 *       quattro righe di questo file andrebbero riscritte `= !...` nello
 *       stesso commit, o crescerebbero di una istruzione ciascuna.
 * D-H7. `+712` (`int c712`) NON e' un intero ordinario ad almeno un sito.
 *       In `ilitek_node_ioctl_write`, comando `iceflag`, la fabbrica SCRIVE
 *       il campo ("b902c928 str"@0xffffff8008a6d3a4) e SUBITO DOPO LO
 *       RILEGGE dalla stessa base e dallo stesso scarto
 *       ("b942c923 ldr"@0xffffff8008a6d3a8) per passarlo alla `printk`.
 *       Un accesso ordinario non sopravvive: misurate quattro forme sul `.o`
 *       vero, tre danno 4664 byte (una istruzione in meno) e solo la lettura
 *       VOLATILE dell'argomento ne da' 4668. Cio' che e' MISURATO e' che la
 *       rilettura non e' eliminabile; che il campo sia un `atomic_t` letto
 *       con `atomic_read` -- l'ipotesi piu' naturale, perche' `atomic_set`
 *       (cioe' `WRITE_ONCE`) sullo STORE NON riproduce il fatto, mentre
 *       `atomic_read` sul LOAD si' -- e' una LETTURA, non una misura.
 *       Qui e' aggirata con un `READ_ONCE` locale e dichiarato. Se
 *       `ilitek.h` cambiasse `+712` in `atomic_t`, tutti i suoi 51 siti di
 *       lettura e 6 di scrittura andrebbero riletti insieme: e' il genere di
 *       modifica che tocca al lotto di merge, non a questo.
 * D-H8. `+48` e `+56` (`void *c48`, `void *c56`) sono puntatori a due
 *       strutture di cui il gruppo H legge SEI scarti a 32 bit:
 *       +8, +40, +44, +48, +52 di `c48` e +0 di `c56`
 *       ("b9400909 ldr"@0xffffff8008a6df88, "29452109 ldp"@0xffffff8008a6df90,
 *       "b9403129 ldr"@0xffffff8008a6eb04, "b940352a ldr"@0xffffff8008a6ec78,
 *       "b9400129 ldr"@0xffffff8008a6ebc0). `ilitek.h` non le dichiara e
 *       questo file le raggiunge con due macro dichiarate
 *       (`IDEV_C48_AT`, `IDEV_C56_AT`) invece di inventare due `struct`.
 * D-H9. I VENTOTTO COMANDI IOCTL non hanno nome nel binario. Qui sono scritti
 *       con `_IOWR(0x64, nr, u32)` e `_IOWR(0x64, nr, unsigned long)`, che
 *       ricostruiscono esattamente i numeri confrontati. Se `ilitek.h`
 *       volesse dei nomi, sarebbero INVENTATI: il binario non ne porta
 *       nessuno, e il lotto di merge deve saperlo prima di aggiungerli.
 * D-H5. Sette prototipi che questo file dichiara localmente perche'
 *       `ilitek.h` non li ha: `str2hex`, `katoi`, `get_tp_recore_ctrl`,
 *       `get_tp_recore_data`, `gesture_fail_reason`, `netlink_reply_msg`,
 *       `ilitek_tddi_node_init` (tutti `T` nella mappa, quindi usati da
 *       fuori) piu' `ilitek_tddi_ic_func_ctrl`, `ilitek_ice_mode_ctrl`,
 *       `ilitek_ice_mode_read`, `ilitek_ice_mode_write`,
 *       `ilitek_tddi_ic_get_pc_counter` e `ilitek_dump_data` dei gruppi E/F.
 *
 * ===========================================================================
 * LA FUNZIONE `.exit` DEL GRUPPO H: NON ESISTE. E' UN RISULTATO, NON UNA
 * LACUNA
 * ===========================================================================
 * `oracolo/stock.map` finisce a `_einittext` (0xffffff80093a8518): tutto
 * cio' che sta oltre -- la `.exit.text` -- e' invisibile agli strumenti che
 * partono dalla mappa. Cercata a mano, e NON c'e':
 *  - in TUTTO il binario ci sono 9 `bl <netlink_kernel_release>`, tutte fra
 *    0xffffff8008e61c6c e 0xffffff8009398000, cioe' nel nucleo netlink e
 *    nell'avvio: NESSUNA nell'intervallo ILITEK;
 *  - oltre `_einittext` la pagina dei globali del gruppo H
 *    (0xffffff800a100000) e' toccata DUE volte sole, a scarto 3360 e 3376
 *    ("b94d2100 ldr"@0xffffff80093acbc8 e "f9469900 ldr"@0xffffff80093acc48),
 *    che NON sono gli scarti del gruppo H (1732..1768). Nessuna `.exit`
 *    libera il socket netlink ne' la directory /proc;
 *  - l'unica `.exit` ILITEK che esiste e' `ilitek_plat_dev_exit`
 *    (0xffffff80093acac4, 52 byte, `__func__` "ilitek_plat_dev_exit" a
 *    0xffffff800923f7ed, riga 449), che chiama `tpd_driver_remove`
 *    ("97da926f bl"@0xffffff80093acaec): appartiene al GRUPPO G ed e' gia'
 *    scritta in `ilitek_plat.c`.
 * Conseguenza misurata: di fabbrica i tredici nodi /proc e il socket netlink
 * non vengono mai smontati. Non c'e' niente da scrivere qui.
 *
 * RIVERIFICATO IL 2026-08-21, indipendentemente, e con un quasi-inciampo che
 * il primo controllo non aveva nominato: oltre `_einittext` ci sono DUE
 * chiamate a `tpd_driver_remove` ("97da926f bl"@0xffffff80093acaec e
 * "97da9262 bl"@0xffffff80093acb20), non una. La seconda e' preceduta da
 * "d0002ee0 adrp"@0xffffff80093acb18, cioe' dalla STESSA PAGINA
 * (0xffffff800998a000) su cui vive la tabella dei tredici nodi /proc del
 * gruppo H -- ma allo scarto +0x578, non +0x130, e con un `__func__` che sta
 * a 0xffffff800924c097, fuori dalle pagine delle stringhe ILITEK
 * (0xffffff8009247000..0xffffff800924a000). E' la `.exit` di UN ALTRO driver
 * touch. La coincidenza di pagina e' quella che, presa per buona, avrebbe
 * fatto attribuire al gruppo H una `.exit` che non ha.
 *
 * ===========================================================================
 * COSA NON E' SCRITTO, E PERCHE'
 * ===========================================================================
 * NESSUNA FUNZIONE DEL GRUPPO H. Tutte e ventotto sono definite; le tre che
 * il lotto precedente aveva lasciato dichiarate e non definite sono state
 * scritte il 2026-08-21 e misurano esatte. Il campo `+792` (divergenza D-H1)
 * e' ora anche ESERCITATO, ai suoi tre siti di chiamata veri.
 *
 * RESTA INDEFINITO cio' che sta in ALTRI gruppi e questo blocco chiama, e il
 * link continua a fallire per questo -- che e' l'esito onesto (regola 6) e
 * NON e' un difetto di questo file. Le funzioni sono dichiarate qui e non
 * definite da nessuna parte:
 *   dal gruppo A/C (`ilitek.h`): `ilitek_tddi_reset_ctrl`,
 *   `ilitek_set_tp_data_len`, `ilitek_tddi_wq_ctrl`,
 *   `ilitek_tddi_switch_tp_mode`, `ilitek_tddi_gesture_recovery`,
 *   `ilitek_tddi_spi_recovery`, `ilitek_tddi_fw_upgrade_handler`,
 *   `ilitek_plat_irq_enable`, `ilitek_plat_irq_disable`;
 *   dai gruppi D/E/F (dichiarate in questo file): `ilitek_tddi_ic_func_ctrl`,
 *   `ilitek_ice_mode_ctrl`, `ilitek_ice_mode_read`, `ilitek_ice_mode_write`,
 *   `ilitek_tddi_ic_get_pc_counter`, `ilitek_dump_data`,
 *   `ilitek_tddi_ic_get_info`, `ilitek_tddi_ic_get_protocl_ver`,
 *   `ilitek_tddi_ic_get_fw_ver`, `ilitek_tddi_ic_get_core_ver`,
 *   `ilitek_tddi_ic_get_tp_info`, `ilitek_tddi_ic_get_panel_info`,
 *   `core_spi_setup`, `ilitek_tddi_ic_get_ddi_reg_onepage`,
 *   `ilitek_tddi_ic_set_ddi_reg_onepage`, `ilitek_tddi_fw_dump_flash_data`,
 *   `ilitek_fw_dump_iram_data`, `ilitek_tddi_fw_uart_ctrl`,
 *   `ilitek_tddi_touch_esd_gesture_flash`.
 *
 * NON E' STATO FATTO, e va detto perche' nessuno lo scambi per fatto:
 *  - l'IDENTITA' DI CODIFICA non e' raggiunta e non e' stata cercata oltre
 *    l'istogramma dei mnemonici (divergenza H4). Sulle tre funzioni nuove il
 *    confronto grezzo non e' interpretabile: portano centinaia di
 *    rilocazioni (139 coppie `adrp`+`add` nella sola
 *    `ilitek_node_ioctl_write`), che in un `.o` non collegato valgono zero;
 *  - i `__LINE__` non sono riprodotti (divergenza H3). Sono noti uno per uno
 *    -- 1305..1545, 1560..1680, 1695..2058 -- e stanno nel rapporto;
 *  - le TRE TABELLE DI SALTO non sono state confrontate byte per byte con
 *    quelle che il nostro `.o` genera. Il confronto e' possibile e non e'
 *    stato fatto: le tabelle vivono in `.rodata` e in un `.o` non collegato
 *    gli offset sono relativi alla base del `br`, quindi confrontabili. Cio'
 *    che e' stato verificato e' che i CASI e i loro valori sono quelli
 *    giusti, per costruzione: senza di che le tabelle non tornerebbero e la
 *    dimensione non tornerebbe.
 *
 * ===========================================================================
 * LE CITAZIONI DEI LETTERALI
 * ===========================================================================
 * Ogni stringa che questo file contiene, con l'indirizzo a cui la fabbrica la
 * tiene. Le marcate DEBOLE compaiono piu' volte nel binario: l'indirizzo
 * citato e' quello che il codice del BLOCCO carica davvero (letto dalle
 * coppie `adrp`+`add` del disassemblato), ma la verifica byte-a-byte da sola
 * non lo distingue dalle altre occorrenze.
 *
 * (il "%5d" di KERN_CONT sta a 0xffffff80092499aa, dopo il byte di livello a
 * 0xffffff80092499a8: stesso testo, quindi una sola citazione; lo stesso vale
 * per il "\n" di KERN_CONT a 0xffffff80090ddcf3, dopo il byte a
 * 0xffffff80090ddcf1.)
 *
 * " %s debug level = %x\n"@0xffffff800924a01a
 * "%02X"@0xffffff80091bc2c7   (4 nel binario: DEBOLE)
 * "%02d\n"@0xffffff8009235f68   (14 nel binario: DEBOLE)
 * "%5d"@0xffffff800921ec89   (2 nel binario: DEBOLE)
 * "%d, "@0xffffff800913b1a2   (15 nel binario: DEBOLE)
 * "%s gesture fail reason\n"@0xffffff8009248097
 * "%s\n"@0xffffff8009226be1   (51 nel binario: DEBOLE)
 * ","@0xffffff8009245907   (51 nel binario: DEBOLE)
 * "/sdcard/ILITEK_log.csv"@0xffffff8009249ceb
 * "/sdcard/flash_dump"@0xffffff8009246288
 * "/sdcard/ilitek_mp_lcm_off_log"@0xffffff80092455c6
 * "/sdcard/ilitek_mp_lcm_on_log"@0xffffff80092455a9
 * "/sdcard/iram_dump"@0xffffff8009245b8a
 * "2.0.6.0.191122"@0xffffff800923ed58
 * "======== Deltadata ========\n"@0xffffff800924993b
 * "======== RawData ========\n"@0xffffff8009249a51
 * "CRC not matched, abort!"@0xffffff800924971c
 * "Core version = %d.%d.%d.%d\n"@0xffffff8009248a5d
 * "DISABLE"@0xffffff8009192e88
 * "Debug buf ctrl = %s\n"@0xffffff8009247e90
 * "Disable"@0xffffff80090df610
 * "Driver version = %s\n"@0xffffff800924387c
 * "ENABLE"@0xffffff80091e83db
 * "ERROR! input length is larger than local buffer\n"@0xffffff8009247f4c
 * "ERROR! read len is largn than ioctl buf (%d, %ld)\n"@0xffffff800924880e
 * "ERROR! write len is largn than ioctl buf (%d, %ld)\n"@0xffffff8009248739
 * "Enable"@0xffffff80091e0f72   (17 nel binario: DEBOLE)
 * "FAIL"@0xffffff8009249687   (26 nel binario: DEBOLE)
 * "FW still upgrading, abort!"@0xffffff8009247b70
 * "Failed to Read iram data\n"@0xffffff8009246793
 * "Failed to allocate CSV mem\n"@0xffffff80092453dd
 * "Failed to allocate data mem\n"@0xffffff800924982b
 * "Failed to allocate dbl[%d] mem, %ld\n"@0xffffff8009247f13
 * "Failed to allocate debug buf\n"@0xffffff8009249e88
 * "Failed to allocate delta mem\n"@0xffffff800924985c
 * "Failed to allocate idev->dbl mem, %ld\n"@0xffffff8009247ed8
 * "Failed to allocate mem\n"@0xffffff80092486e2
 * "Failed to allocate memory, abort!"@0xffffff80092496b3
 * "Failed to allocate new skb\n"@0xffffff80092479b6
 * "Failed to allocate packet memory, %ld\n"@0xffffff80092476d0
 * "Failed to allocate rawdata mem\n"@0xffffff80092499e3
 * "Failed to allocated memory, abort!"@0xffffff8009247bc1
 * "Failed to convert hex/ili file, abort!"@0xffffff800924968c
 * "Failed to copy data from user space\n"@0xffffff8009247fa9   (2 nel binario: DEBOLE)
 * "Failed to copy data to user space\n"@0xffffff8009247c41
 * "Failed to copy driver ver to user space\n"@0xffffff8009248abc
 * "Failed to create %s under /proc\n"@0xffffff8009247a1c
 * "Failed to create directory for mp_test\n"@0xffffff8009247aea
 * "Failed to create nelink socket\n"@0xffffff800924a0be
 * "Failed to disable ICE mode"@0xffffff8009249c55
 * "Failed to disable ICE mode, ret = %d\n"@0xffffff8009249c2f
 * "Failed to do reset, abort!"@0xffffff8009249734
 * "Failed to enter ICE mode"@0xffffff8009249ae2
 * "Failed to enter ICE mode, ret = %d\n"@0xffffff8009249abe
 * "Failed to erase flash, abort!"@0xffffff800924974f
 * "Failed to establish the channel between kernel and user space\n"@0xffffff800924a1e6
 * "Failed to get timing info, abort!"@0xffffff8009247c0b
 * "Failed to open %s file\n"@0xffffff8009249e5c
 * "Failed to operate ice mode, abort!"@0xffffff80092496d5
 * "Failed to operate watch dog, abort!"@0xffffff80092496f8
 * "Failed to program flash, abort!"@0xffffff800924976d
 * "Failed to read data\n"@0xffffff8009248855
 * "Failed to send data back to user\n"@0xffffff80092479e6
 * "Failed to set debug len from gesture mode\n"@0xffffff8009248cb0
 * "Failed to set debug len\n"@0xffffff8009248cef
 * "Failed to set demo len from gesture mode\n"@0xffffff8009248c12
 * "Failed to set tp data length\n"@0xffffff80092421f5
 * "Failed to switch MP mode, abort!"@0xffffff8009247b4f
 * "Failed to switch demo mode\n"@0xffffff8009248c50
 * "Failed to switch test mode\n"@0xffffff8009248c80
 * "Failed to write 0xB7,0x1 command, %d\n"@0xffffff800924988e
 * "Failed to write 0xB7,0x2 command, %d\n"@0xffffff8009249a17
 * "Failed to write 0xB7,0x3 command, %d\n"@0xffffff8009249901
 * "Failed to write data\n"@0xffffff80092487ba
 * "Failed to write iram data\n"@0xffffff8009246764
 * "Firmware version = %d.%d.%d.%d\n"@0xffffff80092414e5
 * "Gesture recovery failed\n"@0xffffff80092481da
 * "Get Delta data %d frame\n"@0xffffff8009249d66
 * "Get Raw data %d frame\n"@0xffffff8009249d3b
 * "Get data"@0xffffff800924758b
 * "Get ddi reg one page: page = %x, reg = %x\n"@0xffffff8009248453
 * "Get info fail\n"@0xffffff800924762d
 * "Header 0x%x ,Type %d, Length %d\n"@0xffffff800924996c
 * "Initialise Netlink and create its socket\n"@0xffffff800924a073
 * "MP TEST %s, Error code = %d\n"@0xffffff8009247b26
 * "MP TEST %s\n"@0xffffff8009247b43
 * "MP formula is null, abort!"@0xffffff8009247b8b
 * "Netlink = %d\n"@0xffffff800924a0f2
 * "Netlink is enable = %d\n"@0xffffff800924798a
 * "Not found ini file, abort!"@0xffffff8009247ba6
 * "Not implemented yet\n"@0xffffff80092488a3
 * "Not supported in this version\n"@0xffffff8009248ef4
 * "PASS"@0xffffff800924966f   (5 nel binario: DEBOLE)
 * "Preparing to upgarde firmware\n"@0xffffff8009249621
 * "Protocol version = %d.%d.%d\n"@0xffffff8009241869
 * "Protocol version isn't matched, abort!"@0xffffff8009247be4
 * "READ:addr = 0x%06x, read = 0x%08x\n"@0xffffff8009249b44
 * "Raw_count = %d, Delta_count = %d, BG_count = %d\n"@0xffffff8009249fd5
 * "Read %d bytes(s) from %ld\n"@0xffffff8009247e39
 * "Read data error"@0xffffff8009249afb
 * "Read data error\n"@0xffffff800923fddf
 * "Read debug packet header failed, %d\n"@0xffffff80092498c8
 * "Received a request from client: %s, %d\n"@0xffffff800924a125
 * "Run MP test with LCM off\n"@0xffffff8009247ca1
 * "Run MP test with LCM on\n"@0xffffff8009247a9d
 * "Saved to file length is too long !, %d\n"@0xffffff8009249e20
 * "Set ddi reg one page: page = %x, reg = %x, data = %x\n"@0xffffff80092484c3
 * "Start = 0x%x, End = 0x%x, Dump Hex path = %s\n"@0xffffff800924851b
 * "Start = 0x%x, End = 0x%x, Dump IRAM path = %s\n"@0xffffff800924856a
 * "Succeed to create %s under /proc\n"@0xffffff8009247a67
 * "TP module = %s\n"@0xffffff8009248118
 * "The Magic number doesn't match\n"@0xffffff8009248647
 * "The channel of Netlink has been established successfully !\n"@0xffffff800924a196
 * "The number of ioctl doesn't match\n"@0xffffff800924868d
 * "The size of data being sent to user = %d\n"@0xffffff800924791c
 * "There's no unlocked_ioctl defined in file\n"@0xffffff8009248fb8
 * "Unknown TP mode ctrl\n"@0xffffff8009248d1c
 * "Unknown command\n"@0xffffff8009248622
 * "Upgrade firmware = FAIL"@0xffffff8009249674
 * "Upgrade firmware = PASS"@0xffffff800924965c
 * "WARNING ! there's no data received.\n"@0xffffff8009247d47
 * "WRITE:addr = 0x%06x, write = 0x%08x, len =%d byte\n"@0xffffff8009249be8
 * "Write 0xFA,0x%x failed\n"@0xffffff8009249ece
 * "Write data error"@0xffffff8009249b8d
 * "Write data error\n"@0xffffff8009249b7b
 * "[%2d] "@0xffffff80092499a1
 * "[READ]:addr = 0x%06x, read = 0x%08x\n"@0xffffff8009249b1f
 * "[WRITE]:addr = 0x%06x, write = 0x%08x, len = %d byte\n"@0xffffff8009249bb2
 * "[X%d] ,"@0xffffff8009249f59
 * "\n"@0xffffff8009245994   (51 nel binario: DEBOLE)
 * "\n[X] ,"@0xffffff8009249f6a
 * "\n[Y%d] ,"@0xffffff8009249f61
 * "\n[Y] ,"@0xffffff8009249f71
 * "\n\n" -- INDIRIZZO NON DETERMINATO. Quello che stava qui,
 *     0xffffff8008090487, cade in .text, e un letterale di stringa in .text
 *     non c'e' mai: la scansione aveva preso il PRIMO byte uguale in tutta
 *     l'immagine, non il letterale. Da rifare cercando nella sola .rodata.
 * "\n\n=======Delta data======="@0xffffff8009249d98
 * "\n\n=======Raw data======="@0xffffff8009249d7f
 * "\n\nFrame%d,"@0xffffff8009249f4e
 * "addr = 0x%x, len = %d, lndex = 0x%x, fram num = %d, record_case = 0x%x\n"@0xffffff8009247663
 * "all record bytes"@0xffffff80092476ab
 * "app_an_stat = %d\n"@0xffffff8009247732
 * "app_check_abnor = %d\n"@0xffffff8009247758
 * "buffer vmalloc error\n"@0xffffff8009247d80
 * "cmd = %d\n"@0xffffff80092486c4
 * "cmd = %s\n"@0xffffff800924802e
 * "cmd fail\n"@0xffffff80092475a8   (7 nel binario: DEBOLE)
 * "compat_ioctl: convert chip id\n"@0xffffff80092492e9
 * "compat_ioctl: convert core version\n"@0xffffff8009249277
 * "compat_ioctl: convert ddi read\n"@0xffffff80092495b1
 * "compat_ioctl: convert ddi write\n"@0xffffff800924957c
 * "compat_ioctl: convert debug level\n"@0xffffff80092491d3
 * "compat_ioctl: convert driver version\n"@0xffffff80092492af
 * "compat_ioctl: convert dump flash\n"@0xffffff8009249474
 * "compat_ioctl: convert format mode\n"@0xffffff800924920a
 * "compat_ioctl: convert fw uart\n"@0xffffff80092494aa
 * "compat_ioctl: convert fw version\n"@0xffffff8009249241
 * "compat_ioctl: convert hw reset\n"@0xffffff80092490f8
 * "compat_ioctl: convert i2c/spi read\n"@0xffffff8009249049
 * "compat_ioctl: convert i2c/spi write\n"@0xffffff8009249010
 * "compat_ioctl: convert interface type\n"@0xffffff800924943a
 * "compat_ioctl: convert irq switch\n"@0xffffff800924919d
 * "compat_ioctl: convert netlink ctrl\n"@0xffffff800924931c
 * "compat_ioctl: convert netlink status\n"@0xffffff8009249354
 * "compat_ioctl: convert power switch\n"@0xffffff800924912c
 * "compat_ioctl: convert report switch\n"@0xffffff8009249164
 * "compat_ioctl: convert resolution\n"@0xffffff80092494dd
 * "compat_ioctl: convert set read length\n"@0xffffff80092490bd
 * "compat_ioctl: convert set write length\n"@0xffffff8009249081
 * "compat_ioctl: convert tp info\n"@0xffffff8009249513
 * "compat_ioctl: convert tp mode ctrl\n"@0xffffff800924938e
 * "compat_ioctl: convert tp mode status\n"@0xffffff80092493c6
 * "compat_ioctl: convert tp mode switch\n"@0xffffff8009249400
 * "compat_ioctl: convert wrapper rw\n"@0xffffff8009249546
 * "copy_to_user err\n"@0xffffff8009247e13
 * "data[%d] = %x\n"@0xffffff800924800b
 * "dbgflag"@0xffffff80092482e2
 * "ddi_data = %x\n"@0xffffff8009248492
 * "debug flag message = %d\n"@0xffffff80092482fe
 * "debug level : %s\n"@0xffffff800924a04d
 * "debug mode get data timeout!\n"@0xffffff8009249f30
 * "debug_level"@0xffffff8008ef4224   (5 nel binario: DEBOLE)
 * "debug_message"@0xffffff800924789b
 * "debug_message_switch"@0xffffff80092478a9
 * "deepsleepin"@0xffffff80092481fa
 * "disableicemode"@0xffffff8009248163
 * "disablewqbat"@0xffffff8009248197
 * "disablewqesd"@0xffffff800924818a
 * "dnp : %s\n"@0xffffff8009247e54
 * "dumpflashdata"@0xffffff80092484f9
 * "dumpiramdata"@0xffffff8009248549
 * "edge_palm"@0xffffff80092419d4
 * "edge_palm_ctrl"@0xffffff8009248599
 * "enable gesture fail reason failed"@0xffffff80092477df
 * "enableicemode"@0xffffff8009248128
 * "enablewqbat"@0xffffff800924817e
 * "enablewqesd"@0xffffff8009248172
 * "esdgesture"@0xffffff80092485bc
 * "esdspi"@0xffffff80092481f3
 * "f_count= %d, index = %d, mark = %d\n"@0xffffff8009247cf0
 * "file name is invaild\n"@0xffffff8009249df6
 * "flashesdgesture"@0xffffff80092485b7
 * "frame = %d,index = %d,count = %d\n"@0xffffff8009249efa
 * "fw_pc_counter"@0xffffff80092478be
 * "fw_process"@0xffffff8009247865
 * "fw_upgrade"@0xffffff8009246377
 * "gesture"@0xffffff800924b992
 * "gesture = %d\n"@0xffffff80092481b8
 * "gesture mode = %d\n"@0xffffff8009248258
 * "gesturedemoen"@0xffffff8009248066
 * "gesturefailrsn"@0xffffff8009248074
 * "gestureinfo"@0xffffff800924826b
 * "gesturenormal"@0xffffff8009248236
 * "get data fail\n"@0xffffff80092475c6   (2 nel binario: DEBOLE)
 * "get gesture parameters failed"@0xffffff8009247847
 * "get_debug_mode_data"@0xffffff80092478ea
 * "getddiregdata"@0xffffff8009248431
 * "getinfo"@0xffffff80092480fc
 * "hwreset"@0xffffff8009248038
 * "iccodereset"@0xffffff80092480bc
 * "ice mode flag = %d\n"@0xffffff8009248222
 * "iceflag"@0xffffff8009248206
 * "icwholereset"@0xffffff80092480af
 * "ilitek"@0xffffff800923f701
 * "info from hex = %d\n"@0xffffff80092480e8
 * "infofromhex"@0xffffff80092480c8
 * "ioctl"@0xffffff8009248673   (51 nel binario: DEBOLE)
 * "ioctl: Failed to dump flash data\n"@0xffffff8009248e3b
 * "ioctl: Netlink is disabled\n"@0xffffff8009248b7e
 * "ioctl: Netlink is enabled\n"@0xffffff8009248b4f
 * "ioctl: current firmware mode = %d"@0xffffff8009248d46
 * "ioctl: data = %x\n"@0xffffff8009248f92
 * "ioctl: dump flash data\n"@0xffffff8009248e0f
 * "ioctl: fw UART  = %d\n"@0xffffff8009248e71
 * "ioctl: get chip id\n"@0xffffff8009248af9
 * "ioctl: get core version\n"@0xffffff8009248a30
 * "ioctl: get driver version\n"@0xffffff8009248a8d
 * "ioctl: get fw version\n"@0xffffff80092489d5
 * "ioctl: get netlink stat = %d\n"@0xffffff8009248bae
 * "ioctl: get panel resolution\n"@0xffffff8009248e9b
 * "ioctl: get protocl version\n"@0xffffff8009248a00
 * "ioctl: get tp info\n"@0xffffff8009248ecc
 * "ioctl: hw reset\n"@0xffffff800924887e
 * "ioctl: irq switch = %d\n"@0xffffff800924894a
 * "ioctl: netlink ctrl = %d\n"@0xffffff8009248b21
 * "ioctl: page = %x, reg = %x, data = %x\n"@0xffffff8009248f27
 * "ioctl: page = %x, reg = %x\n"@0xffffff8009248f62
 * "ioctl: read len = %d\n"@0xffffff80092487e4
 * "ioctl: report switch = %d\n"@0xffffff80092488cc
 * "ioctl: set func mode = %x,%x,%x\n"@0xffffff80092489a0
 * "ioctl: set ice mode disabled\n"@0xffffff8009248ddd
 * "ioctl: set ice mode enabled\n"@0xffffff8009248dac
 * "ioctl: switch fw format = %d\n"@0xffffff8009248be0
 * "ioctl: switch ice mode = %d"@0xffffff8009248d7c
 * "ioctl: write len = %d\n"@0xffffff800924870e
 * "ior"@0xffffff80091d8867
 * "iow"@0xffffff8009248369
 * "iowr"@0xffffff80092483f4
 * "ipio_debug_level = %d"@0xffffff8009248976
 * "knock_en"@0xffffff800923e86d
 * "mkdir: %s\n"@0xffffff8009247c78
 * "mp_lcm_off_test"@0xffffff800924788b
 * "mp_lcm_on_test"@0xffffff800924787c
 * "netlink"@0xffffff80092a8ce3
 * "netlink flag= %d\n"@0xffffff800924828b
 * "no ioctl cmd, return ilitek_node_ioctl\n"@0xffffff80092495e5
 * "pc = 0x%x, latch = 0x%x\n"@0xffffff8009249cb3
 * "pid = %d\n"@0xffffff800924796c   (2 nel binario: DEBOLE)
 * "r_len = %d\n"@0xffffff80092483c5
 * "rawdatarecore"@0xffffff8009248040
 * "read length = %d\n"@0xffffff80092497e5
 * "read[%d] = %x\n"@0xffffff80092483e5
 * "recore disable"@0xffffff800924760a
 * "recore enable"@0xffffff8009247556
 * "recore reset"@0xffffff80092475e9
 * "recore_data"@0xffffff8009247643
 * "report is disabled\n"@0xffffff8009248922
 * "report is enabled\n"@0xffffff80092488fb
 * "rw_reg[%d] = 0x%x\n"@0xffffff8009249ca0
 * "rw_tp_reg"@0xffffff80092478fe
 * "send_data_len = %d set 4096 i = %d\n"@0xffffff8009247daa
 * "send_data_len = %d set 4096\n"@0xffffff8009247de2
 * "sense_stop = %d\n"@0xffffff8009248358
 * "set gesture parameters failed"@0xffffff8009247815
 * "set symbol failed"@0xffffff80092477a5
 * "setddiregdata"@0xffffff80092484a1
 * "show_delta_data"@0xffffff80092478cc
 * "show_raw_data"@0xffffff80092478dc
 * "size = %d, cmd = %s\n"@0xffffff8009247fe2
 * "sleep"@0xffffff800911c431
 * "sleepin"@0xffffff80092481fe
 * "spi clk num = %d\n"@0xffffff8009248332
 * "spiclk"@0xffffff8009248317
 * "spir"@0xffffff80092485cc
 * "spirw"@0xffffff8009248608
 * "spiw"@0xffffff80092485c7
 * "ss"@0xffffff800928d14b
 * "stop_mcu = %d\n"@0xffffff8009249a80
 * "str is invaild\n"@0xffffff8009249dc7
 * "switchdebugmode"@0xffffff80092482ac
 * "switchdemodebuginfomode"@0xffffff800924804e
 * "switchdemomode"@0xffffff80092482bc
 * "switchgesturedebugmode"@0xffffff80092482cb
 * "switchtestmode"@0xffffff800924829d
 * "temp[%d] = %d\n"@0xffffff8009249fb2
 * "the pid of sending process = %d\n"@0xffffff800924a161
 * "tp_palm_stat = %d\n"@0xffffff800924770b
 * "tp_recore"@0xffffff80092419e9
 * "uart_mode_ctrl"@0xffffff80092485a8
 * "update status = %d\n"@0xffffff80092497a1
 * "w_len = %d, r_len = %d, delay = %d\n"@0xffffff800924840d
 * "w_len = %d\n"@0xffffff8009248381
 * "wq_ctrl flag= %d\n"@0xffffff8009248151
 * "wqctrl"@0xffffff8009248136
 * "write[%d] = %x\n"@0xffffff80092483a1
 * "wrong_bg = %d\n"@0xffffff8009247782   (2 nel binario: DEBOLE)
 */

#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <net/net_namespace.h>
#include <net/netlink.h>
#include <net/sock.h>

#include "ilitek.h"

/* I globali del blocco. NOMI SCELTI: il binario non li nomina. */
/*
 * 0xffffff800a1006b0, elementi da 4 byte indicizzati con estensione ZERO
 * ("b8345b24 str"@0xffffff8008a70c34, `[x25, w20, uxtw #2]`). Il NUMERO di
 * elementi NON e' misurato -- il binario non confronta l'indice con nessun
 * limite -- ed e' dedotto dalla distanza dal globale successivo
 * (0xffffff800a1006c4 - 0xffffff800a1006b0 = 20 = 5 x 4). E' una SCELTA.
 * NOME SCELTO... no: il nome viene dal formato a 0xffffff8009249c8c,
 * "rw_reg[%d] = 0x%x\n".
 */
static u32 rw_reg[5];
static int netlink_pid;			/* 0xffffff800a1006c4, 4 byte */
static struct sk_buff *netlink_skb;	/* 0xffffff800a1006c8, 8 byte */
static struct nlmsghdr *netlink_nlh;	/* 0xffffff800a1006d0, 8 byte */
static struct sock *netlink_sk;		/* 0xffffff800a1006d8, 8 byte */
static struct proc_dir_entry *proc_dir;	/* 0xffffff800a1006e0, 8 byte */

/*
 * 0xffffff800a0ff6b0, 4096 byte. NOME SCELTO. La DIMENSIONE non e' scelta:
 * il controllo di `copy_to_user` in `ilitek_proc_debug_switch_read` confronta
 * la lunghezza con 4096 ("7140041f cmp"@0xffffff8008a6c834) e passa 4096 come
 * primo argomento della WARN ("321403e3 orr"@0xffffff8008a6c8ec), cioe'
 * `__builtin_object_size(g_user_buf, 0)` vale 4096.
 */
static unsigned char g_user_buf[4096];

/*
 * 0xffffff800a1006e8, DUE byte: il binario lo scrive con `strh`
 * ("790dd114 strh"@0xffffff8008a6ddd8) e lo legge con `ldrh`
 * ("794dd2d5 ldrh"@0xffffff8008a6dcf4). NOME SCELTO; la larghezza no.
 */
static u16 ioctl_len;

/*
 * I due puntatori opachi `idev->c48` e `idev->c56` letti a scarto costante
 * da `ilitek_node_ioctl`. `ilitek.h` li tiene `void *` e questo lotto non
 * lo tocca: gli scarti sono misurati ai siti citati caso per caso.
 */
#define IDEV_C48_AT(off)	(*(u32 *)((u8 *)idev->c48 + (off)))
#define IDEV_C56_AT(off)	(*(u32 *)((u8 *)idev->c56 + (off)))

/*
 * IL BUFFER DI DIAGNOSTICA `idev->c592`.
 *
 * `ilitek.h` lo tiene `void *`. Il binario dice che punta a 1024 elementi di
 * 16 byte -- la `kzalloc` chiede 0x4000 = 16384 byte
 * ("321203e0 orr"@0xffffff8008a718d0) e i due cicli contano fino a 1024
 * ("7110011f cmp"@0xffffff8008a71970) con passo 16
 * ("f140127f cmp"@0xffffff8008a719f0 su un indice incrementato di 0x10).
 * Di ciascun elemento sono misurati due campi: un byte a +0 azzerato
 * ("3829687f strb"@0xffffff8008a71904) e un puntatore a +8 che riceve una
 * `kzalloc(2048)` ("f9000500 str"@0xffffff8008a7194c).
 *
 * IL NOME DEL CAMPO E' DATO DAL BINARIO, non scelto: il formato a
 * 0xffffff8009247ec4 e' "Failed to allocate idev->dbl mem, %ld\n". E' il
 * campo che `ilitek.h` chiama `c592`. Vedi il delta di intestazione.
 */
struct ilitek_dbl {
	u8 c0;
	void *c8;
};

/*
 * La forma di liberazione della fabbrica: l'INDIRIZZO del puntatore e'
 * calcolato una volta e sopravvive alla `kfree`
 * ("f8408ec0 ldr"@0xffffff8008a71918, pre-indice con riscrittura, e
 * "f90002df str"@0xffffff8008a71924 che riusa lo stesso registro). Scrivere
 * invece `if (p->campo) { kfree(p->campo); p->campo = NULL; }` fa rileggere
 * il globale dopo la chiamata e costa una istruzione per sito.
 */
static inline void ipio_kfree(void **mem)
{
	if (*mem != NULL) {
		kfree(*mem);
		*mem = NULL;
	}
}

#define IDEV_DBL	((struct ilitek_dbl *)idev->c592)

/*
 * IL DESCRITTORE DI SCARICO SU FILE, 152 byte sulla pila di
 * `ilitek_proc_get_debug_mode_data_read`. Gli scarti sono misurati:
 *  +0    puntatore, riceve la `vmalloc` ("f90003e0 str"@0xffffff8008a70e94)
 *        ed e' il 2o argomento di `vfs_write` ("f9400261 ldr"@0xffffff8008a71abc);
 *  +8    128 byte di testo: la costante "/sdcard/ILITEK_log.csv" copiata a
 *        +8 e il riempimento a zero fino a +135
 *        ("39021fff strb"@0xffffff8008a70e40 azzera l'ultimo byte);
 *        e' il 1o argomento di `filp_open` ("aa1503e0 mov"@0xffffff8008a71a78
 *        con x21 = base+8, "b1002015 adds"@0xffffff8008a71a4c);
 *  +136  32 bit con segno: la lunghezza da scrivere
 *        ("b9808a62 ldrsw"@0xffffff8008a71ac0);
 *  +140  32 bit: accumulatore, +=  +136 ("b9008e68 str"@0xffffff8008a71b84);
 *  +144  32 bit: il tetto, 10240 ("52850009 mov"@0xffffff8008a70e48).
 */
struct ilitek_file_buffer {
	char *c0;
	char c8[128];
	int c136;
	int c140;
	int c144;
};

static void file_write(struct ilitek_file_buffer *file, bool new_open);
static int debug_mode_get_data(struct ilitek_file_buffer *file, u8 type,
			       u32 frame);

/*
 * +568 di `struct ilitek_tddi_dev`: una `wait_queue_head_t` che `ilitek.h`
 * NON dichiara (fra `c564` e `c592` l'intestazione ha un buco).
 * Misurata qui: "9108e100 add"@0xffffff8008a71d64 passa `idev + 0x238` a
 * `prepare_to_wait_event` con stato 1 (TASK_INTERRUPTIBLE), e
 * "9108e100 add"@0xffffff8008a71d9c la passa a `finish_wait`.
 * Vedi il delta di intestazione.
 */
#define IDEV_INQ	(*(wait_queue_head_t *)((char *)idev + 568))

static int ilitek_debug_node_buff_control(bool enable);

static ssize_t ilitek_node_ioctl_write(struct file *f, const char __user *buf,
				       size_t size, loff_t *pos);

static long ilitek_node_compat_ioctl(struct file *f, unsigned int cmd,
				     unsigned long arg);

static long ilitek_node_ioctl(struct file *f, unsigned int cmd,
			      unsigned long arg);
static ssize_t ilitek_node_mp_lcm_on_test_read(struct file *f, char __user *buf,
					       size_t size, loff_t *pos);
static ssize_t ilitek_node_mp_lcm_off_test_read(struct file *f,
						char __user *buf,
						size_t size, loff_t *pos);
static ssize_t ilitek_proc_debug_message_read(struct file *f, char __user *buf,
					      size_t size, loff_t *pos);
static ssize_t ilitek_proc_debug_switch_read(struct file *f, char __user *buf,
					     size_t size, loff_t *pos);
static ssize_t ilitek_node_fw_upgrade_read(struct file *f, char __user *buf,
					   size_t size, loff_t *pos);
static ssize_t ilitek_proc_fw_process_read(struct file *f, char __user *buf,
					   size_t size, loff_t *pos);
static ssize_t ilitek_proc_get_delta_data_read(struct file *f,
					       char __user *buf,
					       size_t size, loff_t *pos);
static ssize_t ilitek_proc_fw_get_raw_data_read(struct file *f,
						char __user *buf,
						size_t size, loff_t *pos);
static ssize_t ilitek_proc_rw_tp_reg_read(struct file *f, char __user *buf,
					  size_t size, loff_t *pos);
static ssize_t ilitek_proc_rw_tp_reg_write(struct file *f,
					   const char __user *buf,
					   size_t size, loff_t *pos);
static ssize_t ilitek_proc_fw_pc_counter_read(struct file *f, char __user *buf,
					      size_t size, loff_t *pos);
static ssize_t ilitek_proc_get_debug_mode_data_read(struct file *f,
						    char __user *buf,
						    size_t size, loff_t *pos);
static ssize_t ilitek_proc_get_debug_mode_data_write(struct file *f,
						     const char __user *buf,
						     size_t size, loff_t *pos);
static ssize_t ilitek_proc_debug_level_read(struct file *f, char __user *buf,
					    size_t size, loff_t *pos);

/* Gruppo F, 0xffffff8008a57e88. */
int ilitek_tddi_ic_func_ctrl(const char *name, int ctrl);

int get_tp_recore_data(void);

/* Gruppo F, 0xffffff8008a57d20. Nessun registro d'argomento e' impostato al
 * sito di chiamata di 0xffffff8008a70d10, e il corpo non legge x0. */
void ilitek_tddi_ic_get_pc_counter(void);
/* Gruppo F, 0xffffff8008a571a8. */
int ilitek_ice_mode_ctrl(int enable, int mcu);
/* Gruppo F, 0xffffff8008a57098 e 0xffffff8008a56ee0. Le firme vengono dai
 * siti di chiamata di `ilitek_proc_rw_tp_reg_read`. */
int ilitek_ice_mode_write(u32 addr, u32 data, int len);
int ilitek_ice_mode_read(u32 addr, void *data, int len);
/* Gruppo E, 0xffffff8008a5aad8. Stessa firma dichiarata in ilitek_bus.c. */
void ilitek_dump_data(void *dato, int a, int b, int c, const char *etichetta);

/* ==================================================================== */

/*
 * str2hex -- 0xffffff8008a6b528, 144 byte, `T` (serve fuori dal blocco: due
 * `bl <str2hex>` vengono dal gruppo del collaudo MP).
 * Il marcatore del corpo e' il confronto minuscolo/maiuscolo del nibble
 * ("7101819f cmp"@0xffffff8008a6b584, cioe' `(c & 0xf0 | 0x20) == 0x60`):
 * compare QUATTRO volte nel blocco -- una qui e tre nelle copie innestate
 * ("7101819f cmp"@0xffffff8008a70c04 in `ilitek_proc_rw_tp_reg_write`,
 * "710181bf cmp"@0xffffff8008a712bc in
 * `ilitek_proc_get_debug_mode_data_write`, "7101819f cmp"@0xffffff8008a6cb08
 * in `ilitek_node_ioctl_write`, che questo file ora scrive: il marcatore
 * torna anche li').
 * `str2hex` non contiene nessun `printk`, quindi nessun `__func__` la
 * denuncia: si riconosce solo dal corpo.
 * COL COMPILATORE DI FABBRICA misura 144 byte esatti. Col nostro r383902
 * misurava 136: le due istruzioni mancanti erano la copia DUPLICATA
 * dell'epilogo d'errore ("12800000 mov"@0xffffff8008a6b5a0 e
 * "12800000 mov"@0xffffff8008a6b5a8, bersagli dei due
 * "54000301 b.ne"@0xffffff8008a6b540 e "540002c1 b.ne"@0xffffff8008a6b550),
 * che r383902 fonde e r353983c duplica come la fabbrica. NON era un difetto
 * della ricostruzione, ed e' stato scoperto solo quando il compilatore di
 * fabbrica e' stato recuperato: dieci forme sorgente diverse erano state
 * provate invano prima.
 */
int str2hex(char *str)
{
	int strlen, result, intermed, intermedtop;
	char *s = str;

	while (*s != 0x0)
		s++;

	strlen = (int)(s - str);
	s = str;
	if (*s != 0x30)
		return -1;

	s++;
	if (*s != 'x' && *s != 'X')
		return -1;

	s++;
	result = 0;
	while (*s != 0x0) {
		intermed = *s & 0x0f;
		intermedtop = *s & 0xf0;
		if (intermedtop == 0x60 || intermedtop == 0x40)
			intermed += 0x09;
		intermed = intermed << (4 * (strlen - 3));
		result = result | intermed;
		strlen--;
		s++;
	}
	return result;
}

/*
 * katoi -- 0xffffff8008a6b5b8, 116 byte, `T` (35 `bl <katoi>` nel binario,
 * tutte dal gruppo del collaudo MP; ZERO dentro il gruppo H).
 * Due misure decidono la forma del corpo:
 *  - il segno e' provato PRIMA su '+' e poi su '-'
 *    ("7100ad1f cmp"@0xffffff8008a6b5bc confronta con 0x2b), il che significa
 *    che il SORGENTE prova prima '-': clang inverte le due prove;
 *  - la cifra e' portata avanti in una variabile, non ricavata di nuovo:
 *    la fabbrica fonde moltiplicazione e somma in una sola
 *    ("1b0c2929 madd"@0xffffff8008a6b610). Con `result*10 + (*str - '0')`
 *    scritto dentro la condizione del ciclo, clang riassocia in
 *    `(result*10 + c) - '0'` e servono TRE istruzioni invece di una: e' la
 *    differenza fra 124 e 116 byte, misurata.
 *  - il segno finale e' un `cneg` (codifica 5a890520, indirizzo
 *    0xffffff8008a6b624), preceduto da "7100011f cmp"@0xffffff8008a6b620.
 *    (`cneg` NON e' scritto nella forma citabile perche' manca dal
 *    dizionario `MNEMONICI` di `tools/confinecitazioni.py`, che i due
 *    strumenti importano: `verificaistruzioni.py` lo riconosce dalla
 *    codifica, `verificacitazioni.py` lo scambia per un letterale. E' un
 *    difetto dello strumento, non della citazione -- vedi il rapporto.)
 */
int katoi(char *str)
{
	int result = 0, signal = 0, d;

	if (*str == '-') {
		str++;
		signal = 1;
	} else if (*str == '+')
		str++;

	d = *str - '0';
	while (d >= 0 && d <= 9) {
		result = result * 10 + d;
		str++;
		d = *str - '0';
	}

	if (signal)
		result = -result;

	return result;
}

/*
 * get_tp_recore_ctrl -- 0xffffff8008a6b62c, 376 byte, `T`.
 * L'ordine dei casi nel sorgente e' quello dei `__LINE__`: 1147 (caso 0),
 * 1153..1167 (caso 1), 1172 (caso 2). Le due attese sono `mdelay`:
 * la costante 0x418958 = 1000 * 0x10c7 e' `udelay(1000)`
 * ("52912b00 mov"@0xffffff8008a6b650) ripetuto 50 volte (caso 1) o 200
 * (caso 0, "928018f4 mov"@0xffffff8008a6b724 carica -200).
 * Il quarto argomento di `ilitek_tddi_ic_func_ctrl` e' 1 nel caso 0
 * ("320003e1 orr"@0xffffff8008a6b718), 2 e poi 3 nel caso 1, 0 nel caso 2.
 * DIVERGENZA H7: la fabbrica innesta i rami 0 e 2 dentro
 * `ilitek_node_ioctl_write` e CHIAMA questa copia per il ramo 1
 * ("97fff96b bl"@0xffffff8008a6d080) E' RIPRODOTTO dal 2026-08-21: vedi la
 * divergenza H11 e il commento del comando `rawdatarecore`.
 */
int get_tp_recore_ctrl(int val)
{
	int ret = 0;

	switch (val) {
	case 0:
#line 1147
		ILI_INFO("recore enable");
		ret = ilitek_tddi_ic_func_ctrl("tp_recore", 1);
		mdelay(200);
		break;
	case 1:
		mdelay(50);
		ILI_INFO("Get data");
		ret = ilitek_tddi_ic_func_ctrl("tp_recore", 2);
		if (ret < 0) {
			ILI_ERR("cmd fail\n");
			break;
		}
		if (get_tp_recore_data() < 0)
#line 1162
			ILI_ERR("get data fail\n");
#line 1164
		ILI_INFO("recore reset");
		ret = ilitek_tddi_ic_func_ctrl("tp_recore", 3);
		if (ret < 0)
			ILI_ERR("cmd fail\n");
		break;
	case 2:
#line 1172
		ILI_INFO("recore disable");
		ret = ilitek_tddi_ic_func_ctrl("tp_recore", 0);
		break;
	default:
		break;
	}
	return ret;
}

/*
 * get_tp_recore_data -- 0xffffff8008a6b7a4, 724 byte, `T`.
 * Misure che decidono il sorgente:
 *  - `idev->c712` e' letto PRIMA della chiamata indiretta e tenuto in un
 *    registro salvato ("b942c91a ldr"@0xffffff8008a6b7e4): il sorgente lo
 *    copia in un locale, altrimenti il compilatore ricaricherebbe il globale
 *    `idev` dopo la `blr` (classe A5);
 *  - `buf[4]` e' letto CON SEGNO ("39c013f7 ldrsb"@0xffffff8008a6b804): e'
 *    un `s8`, e il ciclo lo decrementa con `sxtb` a ogni giro;
 *  - il buffer letto e' indicizzato a 16 bit
 *    ("8b28c660 add"@0xffffff8008a6b9c4, `[x19, w8, sxtw #1]`): il fattore 2
 *    e' nel MODO D'INDIRIZZAMENTO, cioe' il puntatore e' a elementi da due
 *    byte. Scriverlo come `u8 *` e moltiplicare per due a mano costa tre
 *    istruzioni in piu' (misurato: 736 byte contro 724);
 *  - le maschere di `buf[7]` sono `ubfx` a 2, 3, 1 e 1 bit, e i quattro nomi
 *    (tp_palm_stat, app_an_stat, app_check_abnor, wrong_bg) VENGONO DAI
 *    FORMATI, non sono scelti;
 *  - l'allocazione e' `kzalloc(len, GFP_ATOMIC)`: la maschera 0x1088020
 *    ("52900401 mov"@0xffffff8008a6b868 con
 *    "72a02101 movk"@0xffffff8008a6b86c) e'
 *    __GFP_HIGH|__GFP_ZERO|__GFP_ATOMIC|__GFP_KSWAPD_RECLAIM.
 * DIFETTO F2: i due rami che escono con -ENODEV
 * ("12800240 mov"@0xffffff8008a6b954) non liberano il buffer. Riprodotto.
 */
int get_tp_recore_data(void)
{
	u8 buf[8] = {0};
	u16 *data;
	u32 addr;
	int len, unit, i;
	s8 lndex;
	u8 fram_num, record_case, stat;
	int ice = idev->c712;

	if (idev->c784(buf, 8) < 0) {
#line 1192
		ILI_ERR("Get info fail\n");
		return -1;
	}

	addr = 0x20000 | (buf[0] << 8) | buf[1];
	len = (buf[2] << 8) | buf[3];
	lndex = (s8)buf[4];
	fram_num = buf[5];
	record_case = buf[6];
	stat = buf[7];

	ILI_INFO("addr = 0x%x, len = %d, lndex = 0x%x, fram num = %d, record_case = 0x%x\n",
#line 1203
		 addr, len, lndex, fram_num, record_case);
	ilitek_dump_data(buf, 8, 8, 0, "all record bytes");

	data = kzalloc(len, GFP_ATOMIC);
	if (IS_ERR(data) || data == NULL) {
		ILI_ERR("Failed to allocate packet memory, %ld\n", PTR_ERR(data));
		return -1;
	}

	if (!ice)
		ilitek_ice_mode_ctrl(1, 1);

	buf[0] = 0x25;
	buf[1] = (u8)((addr & 0x000000ff) >> 0);
	buf[2] = (u8)((addr & 0x0000ff00) >> 8);
	buf[3] = (u8)((addr & 0x00ff0000) >> 16);

	if (idev->c776(buf, 4)) {
#line 1222
		ILI_ERR("Failed to write iram data\n");
		return -ENODEV;
	}

	if (idev->c784(data, len)) {
		ILI_ERR("Failed to Read iram data\n");
		return -ENODEV;
	}

	unit = len / (fram_num * 2);
	for (i = 0; i < fram_num; i++) {
		ilitek_dump_data(&data[lndex * unit], 16, unit,
				 idev->c248, "recore_data");
		lndex--;
		if (lndex < 0)
			lndex = fram_num - 1;
	}

	if (!ice)
		ilitek_ice_mode_ctrl(0, 1);

	if (record_case == 2) {
#line 1245
		ILI_INFO("tp_palm_stat = %d\n", stat & 0x3);
		ILI_INFO("app_an_stat = %d\n", (stat >> 2) & 0x7);
		ILI_INFO("app_check_abnor = %d\n", (stat >> 5) & 0x1);
		ILI_INFO("wrong_bg = %d\n", (stat >> 6) & 0x1);
	}

	kfree(data);
	return 0;
}

/*
 * gesture_fail_reason -- 0xffffff8008a6ba78, 348 byte, `T`, ritorna void
 * (nessun ramo imposta w0 prima dell'epilogo).
 * L'argomento sopravvive alla prima chiamata in un registro salvato
 * ("2a0003f3 mov"@0xffffff8008a6ba90) e viene mascherato solo al momento di
 * scriverlo nel comando ("1200026a and"@0xffffff8008a6bad0): sono DUE
 * istruzioni, e servono entrambe. Scrivendo `bool enable` con
 * `cmd[3] = enable` clang fonde copia e maschera in una sola e la funzione
 * misura 344 byte invece di 348 -- misurato. TRE forme sorgente danno le
 * 348 giuste (`int` con `& 0x01`, `bool` col ternario, `u8` con `& 0x01`):
 * la non-unicita' e' dichiarata, non nascosta.
 */
void gesture_fail_reason(int enable)
{
	u8 cmd[24] = {0};

	if (ilitek_tddi_ic_func_ctrl("knock_en", 8) < 0)
#line 1262
		ILI_ERR("set symbol failed");

	cmd[0] = 0x01;
	cmd[1] = 0x0A;
	cmd[2] = 0x10;
	cmd[3] = enable & 0x01;
	cmd[4] = 0xFF;
	cmd[5] = 0xFF;
	if (idev->c776(cmd, 6) < 0)
#line 1275
		ILI_ERR("enable gesture fail reason failed");

	cmd[0] = 0x01;
	cmd[1] = 0x0A;
	cmd[2] = 0x12;
	cmd[3] = 0x01;
	memset(&cmd[4], 0xFF, 20);
	if (idev->c776(cmd, 24) < 0)
#line 1284
		ILI_ERR("set gesture parameters failed");

	cmd[0] = 0x01;
	cmd[1] = 0x0A;
	cmd[2] = 0x11;
	cmd[3] = 0x01;
	if (idev->c776(cmd, 4) < 0)
#line 1292
		ILI_ERR("get gesture parameters failed");
}

/*
 * ===========================================================================
 * IL CAMPO +792 DELLA STRUCT: LA DIVERGENZA D2 DI `ilitek.h`, CHIUSA QUI
 * ===========================================================================
 * `ilitek.h` dichiara `void *c792` con la motivazione «al solo sito noto il
 * `blr` e' preceduto da un unico `ldr x0`, il che dice che almeno un
 * argomento c'e' e non dice quanti ce ne siano». Quella lettura era
 * incompleta: i TRE siti di lettura del campo che esistono nel binario
 * stanno TUTTI in questa funzione, e in tutti e tre gli argomenti preparati
 * prima del `blr` sono CINQUE, sempre nello stesso ordine.
 *
 *   sito 1, comando "spiw"  ("f9418d09 ldr"@0xffffff8008a6d934, col suo
 *                            "d63f0120 blr"@0xffffff8008a6d940)
 *   sito 2, comando "spir"  ("f9418d09 ldr"@0xffffff8008a6d984, col suo
 *                            "d63f0120 blr"@0xffffff8008a6d998)
 *   sito 3, comando "spirw" ("f9418d09 ldr"@0xffffff8008a6dad8, col suo
 *                            "d63f0120 blr"@0xffffff8008a6dae8)
 *
 * CIO' CHE E' MISURATO:
 *  - x0 = `idev->c8` in tutti e tre ("f9400500 ldr"@0xffffff8008a6d938,
 *    "f9400500 ldr"@0xffffff8008a6d988, "f9400500 ldr"@0xffffff8008a6dadc);
 *  - x1 = un buffer sulla pila ("9100c3e1 add"@0xffffff8008a6d928);
 *  - w2 = un conteggio a 32 bit ("2a1503e2 mov"@0xffffff8008a6d92c al sito
 *    1, "320003e2 orr"@0xffffff8008a6d98c cioe' la costante 1 al sito 2);
 *  - x3 = un secondo puntatore, NULL al sito 1
 *    ("aa1f03e3 mov"@0xffffff8008a6d930);
 *  - w4 = un secondo conteggio a 32 bit, zero al sito 1
 *    ("2a1f03e4 mov"@0xffffff8008a6d93c).
 * Il valore di RITORNO non e' usato in nessuno dei tre siti: il tipo di
 * ritorno resta NON misurato, e `int` qui e' una scelta.
 *
 * CIO' CHE E' UNA LETTURA E NON UNA MISURA: la forma
 * `(dispositivo, tx, n_tx, rx, n_rx)` con questa disposizione dei registri
 * e' quella di `spi_write_then_read`, e i tre comandi che ci arrivano si
 * chiamano `spiw`, `spir`, `spirw`. Il binario NON nomina la funzione,
 * perche' nessuna delle 154 scrive il campo (`ilitek.h`: L=3 S=0) e non c'e'
 * nessuna guardia `cbz` prima dei tre `blr`: di fabbrica quei tre comandi
 * dereferenziano NULL. Non e' verificato sul telefono.
 *
 * `ilitek.h` e' CONDIVISO e questo lotto non lo tocca: la firma e' dichiarata
 * qui, come fa gia' `ilitek_bus.c` per le funzioni del gruppo E. Vedi il
 * delta di intestazione, voce D-H1.
 */
typedef int (*ilitek_c792_t)(void *c8, const void *tx, u32 n_tx,
			     void *rx, u32 n_rx);
#define IDEV_C792	((ilitek_c792_t)idev->c792)

/*
 * LE FUNZIONI DI ALTRI GRUPPI CHIAMATE DA `ilitek_node_ioctl_write`.
 * `ilitek.h` non le dichiara; le firme vengono dai siti di chiamata di
 * questa funzione, non da un'intuizione, e dove il binario non decide il
 * commento lo dice.
 */
/* 0xffffff8008a5a8bc, 0xffffff8008a5a61c, 0xffffff8008a5a004,
 * 0xffffff8008a59d74, 0xffffff8008a5a3e4, 0xffffff8008a5a1cc: sei chiamate
 * consecutive del comando "getinfo" (0xffffff8008a6d1e0 .. 0xffffff8008a6d200)
 * che non impostano nessun registro d'argomento e il cui risultato non e'
 * usato. */
int ilitek_tddi_ic_get_info(void);
int ilitek_tddi_ic_get_protocl_ver(void);
int ilitek_tddi_ic_get_fw_ver(void);
int ilitek_tddi_ic_get_core_ver(void);
int ilitek_tddi_ic_get_tp_info(void);
int ilitek_tddi_ic_get_panel_info(void);
/* 0xffffff8008a55664. Un solo argomento a 32 bit
 * ("b9400680 ldr"@0xffffff8008a6d4b4). */
int core_spi_setup(int clk);
/* 0xffffff8008a58704: due interi e un puntatore
 * ("29408680 ldp"@0xffffff8008a6d7b4, "910093e2 add"@0xffffff8008a6d7b8);
 * cio' che il terzo argomento punta e' riletto con `ldrb`
 * ("394093e3 ldrb"@0xffffff8008a6d7c0), quindi e' largo un byte. */
int ilitek_tddi_ic_get_ddi_reg_onepage(int page, int reg, u8 *data);
/* 0xffffff8008a5836c: tre interi ("29408680 ldp"@0xffffff8008a6d7f8 e
 * "b9400e82 ldr"@0xffffff8008a6d7fc). */
int ilitek_tddi_ic_set_ddi_reg_onepage(int page, int reg, int data);
/* 0xffffff8008a68280 e 0xffffff8008a6714c: due interi e un terzo argomento
 * che vale la costante 0 ("2a1f03e2 mov"@0xffffff8008a6d838) in un caso e la
 * costante 1 ("320003e2 orr"@0xffffff8008a6d874) nell'altro. Che sia un
 * `bool` e non un `int` NON e' deciso dal binario: e' una scelta. */
int ilitek_tddi_fw_dump_flash_data(int start, int end, bool user);
int ilitek_fw_dump_iram_data(int start, int end, bool user);
/* 0xffffff8008a59f0c. Un solo argomento a 32 bit
 * ("b9400680 ldr"@0xffffff8008a6d898). */
int ilitek_tddi_fw_uart_ctrl(int ctrl);
/* 0xffffff8008a5bdc4. Nessun registro d'argomento impostato al sito
 * ("97ffb946 bl"@0xffffff8008a6d8ac e' preceduta dal `cbz` della catena). */
int ilitek_tddi_touch_esd_gesture_flash(void);

/*
 * ilitek_node_ioctl_write -- 0xffffff8008a6c904, 4668 byte, `t` (`static`),
 * righe 1305..1545 del sorgente di fabbrica.
 *
 * E' il .write del nodo /proc "ioctl". La forma e' una CATENA LINEARE di
 * confronti, non una tabella: 45 confronti in tutto, di cui 40
 * `__pi_strncmp` e CINQUE `__pi_memcmp` con lunghezza costante. I cinque
 * memcmp non sono un'altra funzione di libreria: sono cio' in cui clang
 * trasforma `strcmp(x, "letterale")` quando il risultato serve solo a un
 * confronto con zero, e la lunghezza e' `strlen(letterale) + 1`
 * ("321f0be2 orr"@0xffffff8008a6cba0 carica 14 per "rawdatarecore", che ha
 * 13 caratteri; "321d07e2 orr"@0xffffff8008a6cbb8 carica 24 per
 * "switchdemodebuginfomode", che ne ha 23). Sono quindi CINQUE `strcmp` e
 * QUARANTA `strncmp` nel sorgente, e non e' una scelta di stile: scriverli
 * tutti nello stesso modo cambia il codice generato.
 *
 * CON UNA CATENA LINEARE L'ORDINE DEI CONFRONTI E' OSSERVABILE. L'ordine
 * qui sotto e' quello degli indirizzi crescenti dei `bl`, da
 * "940fadfe bl"@0xffffff8008a6cb8c (il primo, "hwreset") a
 * "940facf6 bl"@0xffffff8008a6cfac (l'ultimo, "spirw").
 *
 * DUE COMANDI NON CARICANO IL PRIMO ARGOMENTO perche' il compilatore sa che
 * vale gia' zero: `__pi_strncmp` ha appena restituito 0 in w0 e il `cbz` ci
 * ha portati qui. "enablewqesd" entra a
 * "320003e1 orr"@0xffffff8008a6d2a0 saltando l'`orr w0` che serve a
 * "enablewqbat", e "disablewqesd" entra a
 * "2a1f03e1 mov"@0xffffff8008a6d2b4 saltando quello di "disablewqbat":
 * il PRIMO argomento di `ilitek_tddi_wq_ctrl` vale dunque 0 per i due esd e
 * 1 per i due bat. Lo stesso vale per "disableicemode", che entra a
 * "2a1f03e1 mov"@0xffffff8008a6d100. Non e' una deduzione: e' l'unico modo
 * in cui quei tre blocchi possono avere w0 definito.
 *
 * TRE DIFETTI DELLA FABBRICA, riprodotti (regola 7): vedi F9, F10, F11 nel
 * cappello del file.
 */
static ssize_t ilitek_node_ioctl_write(struct file *filp,
				       const char __user *buff, size_t size,
				       loff_t *pos)
{
	int i;
	int *data = NULL;
	u8 *rxbuf = NULL;
	char cmd[512] = {0};
	char *token = NULL, *cur = NULL;
	u8 temp[256] = {0};
	u8 ddi_data = 0;
	int w_len = 0, r_len = 0, delay = 0;
	size_t len = 0;
	ssize_t count = size;

	/*
	 * "f10806bf cmp"@0xffffff8008a6c950 confronta `size - 1`
	 * ("d1000675 sub"@0xffffff8008a6c94c) con 513, e
	 * "54000123 b.cc"@0xffffff8008a6c99c prosegue solo se e' MINORE:
	 * l'errore scatta per `size - 1 > 512`, non per `size > 512`.
	 * Il buffer locale e' di 512 byte ("321703e2 orr"@0xffffff8008a6c93c
	 * e' il terzo argomento del `__memset` iniziale).
	 */
	if (size - 1 > 512) {
#line 1305
		ILI_ERR("ERROR! input length is larger than local buffer\n");
		return -1;
	}

	mutex_lock(&idev->touch_mutex);

	/*
	 * DIFETTO F9 DELLA FABBRICA: il ramo d'errore di `copy_from_user`
	 * NON rilascia il mutex. "17fffdb0 b"@0xffffff8008a6d2f4 salta alla
	 * `printk` condivisa con l'errore di lunghezza
	 * ("97db1ac8 bl"@0xffffff8008a6c9b4) e da li' a
	 * "140001ea b"@0xffffff8008a6c9bc, che va DIRETTAMENTE al controllo
	 * del canarino e all'epilogo: il `mutex_unlock` di
	 * "9410189b bl"@0xffffff8008a6d160 non e' sul percorso. Riprodotto.
	 */
	if (buff != NULL) {
		if (copy_from_user(cmd, buff, size - 1)) {
#line 1313
			ILI_ERR("Failed to copy data from user space\n");
			return -1;
		}
	}

	ILI_INFO("size = %d, cmd = %s\n", (int)size, cmd);

	/*
	 * 2048 byte = 512 elementi da 4: la dimensione e' il terzo argomento
	 * di `kmem_cache_alloc_trace` ("321503e2 orr"@0xffffff8008a6ca64) e
	 * la maschera GFP il secondo ("51000681 sub"@0xffffff8008a6ca60,
	 * cioe' 0x14080c1 - 1 = 0x14080c0 = GFP_KERNEL | __GFP_ZERO).
	 * Il `sub` e' un artefatto del compilatore: 0x14080c1 e' la maschera
	 * dei DUE `kzalloc` successivi (GFP_DMA in piu'), e clang materializza
	 * la piu' comune e ne ricava l'altra.
	 * Gli elementi sono INTERI CON SEGNO, non `u32`:
	 * "b9800696 ldrsw"@0xffffff8008a6d950 estende con il segno.
	 */
	data = kcalloc(512, sizeof(int), GFP_KERNEL);

	cur = cmd;
	i = 0;
	/*
	 * "940fef95 bl"@0xffffff8008a6ca80 verso <strsep>, con
	 * "91241c21 add"@0xffffff8008a6ca78 che carica il separatore ",".
	 * `str2hex` e' INNESTATA qui: il marcatore e'
	 * "7101819f cmp"@0xffffff8008a6cb08 (il confronto minuscolo/maiuscolo
	 * del nibble), e nel blocco non c'e' nessuna `bl <str2hex>`.
	 * L'indice e' un intero CON SEGNO:
	 * "b835da84 str"@0xffffff8008a6cb38 indicizza con `sxtw`.
	 */
	while ((token = strsep(&cur, ",")) != NULL) {
		data[i] = str2hex(token);
#line 1326
		ILI_INFO("data[%d] = %x\n", i, data[i]);
		i++;
	}

	ILI_INFO("cmd = %s\n", cmd);

	/* "940fade3 bl"@0xffffff8008a6cb74 verso <__pi_strlen>, UNA volta
	 * sola: il risultato serve a tutti e quaranta gli `strncmp`
	 * ("aa1503e2 mov"@0xffffff8008a6cb88 e i suoi trentanove gemelli). */
	len = strlen(cmd);

	if (!strncmp(cmd, "hwreset", len)) {
		/* "321f03e0 orr"@0xffffff8008a6cfdc */
		ilitek_tddi_reset_ctrl(2);
	} else if (!strcmp(cmd, "rawdatarecore")) {
		/*
		 * DIVERGENZA H7, INNESTO PARZIALE CON PROPAGAZIONE DI
		 * COSTANTE: la fabbrica innesta qui i rami 0 e 2 di
		 * `get_tp_recore_ctrl` -- il `__func__` delle due `printk` e'
		 * "get_tp_recore_ctrl"@0xffffff8009247564 dentro una funzione
		 * che si chiama altrimenti, e i `__LINE__` sono 1147
		 * ("52808f62 mov"@0xffffff8008a6d014) e 1172
		 * ("52809282 mov"@0xffffff8008a6d09c) -- e CHIAMA invece la
		 * copia fuori linea per il ramo 1
		 * ("97fff96b bl"@0xffffff8008a6d080, preceduta da
		 * "320003e0 orr"@0xffffff8008a6d07c che carica 1).
		 * Qui c'e' una sola chiamata: la scelta di innestare o no e'
		 * del compilatore, non del sorgente.
		 */
		switch (data[1]) {
		case 0:
			get_tp_recore_ctrl(0);
			break;
		case 1:
			get_tp_recore_ctrl(1);
			break;
		case 2:
			get_tp_recore_ctrl(2);
			break;
		}
	} else if (!strcmp(cmd, "switchdemodebuginfomode")) {
		/* "321f03e0 orr"@0xffffff8008a6d048 */
		ilitek_set_tp_data_len(2);
	} else if (!strcmp(cmd, "gesturedemoen")) {
		/* "1a9f07e8 cset"@0xffffff8008a6d068 (`ne`) e
		 * "b9021d28 str"@0xffffff8008a6d06c: e' una conversione a
		 * 0/1, non una copia. */
		idev->c540 = (data[1] != 0);
		/* "320007e0 orr"@0xffffff8008a6d060 */
		ilitek_set_tp_data_len(3);
	} else if (!strcmp(cmd, "gesturefailrsn")) {
		/* Due siti di chiamata distinti e non un `cset`:
		 * "320003e0 orr"@0xffffff8008a6d0c4 e
		 * "2a1f03e0 mov"@0xffffff8008a6d110 confluiscono in
		 * "97fffa59 bl"@0xffffff8008a6d114. `data[1]` e' RILETTO dopo
		 * la chiamata ("b9400688 ldr"@0xffffff8008a6d118). */
		if (data[1])
			gesture_fail_reason(1);
		else
			gesture_fail_reason(0);
		ILI_INFO("%s gesture fail reason\n",
#line 1354
			 data[1] ? "ENABLE" : "DISABLE");
	} else if (!strncmp(cmd, "icwholereset", len)) {
		/* "2a1f03e1 mov"@0xffffff8008a6d0d0 poi
		 * "2a1f03e0 mov"@0xffffff8008a6d0d8 */
		ilitek_ice_mode_ctrl(1, 0);
		ilitek_tddi_reset_ctrl(0);
	} else if (!strncmp(cmd, "iccodereset", len)) {
		/* "320003e0 orr"@0xffffff8008a6d0f4 e' il 1 di reset_ctrl */
		ilitek_ice_mode_ctrl(1, 0);
		ilitek_tddi_reset_ctrl(1);
		ilitek_ice_mode_ctrl(0, 0);
	} else if (!strcmp(cmd, "infofromhex")) {
		/* "1a9f07e8 cset"@0xffffff8008a6d1b0 e
		 * "3909f128 strb"@0xffffff8008a6d1b4 (un byte). */
		idev->c636 = (data[1] != 0);
		/* "b9400683 ldr"@0xffffff8008a6d1b8: `data[1]` e' RILETTO. */
#line 1364
		ILI_INFO("info from hex = %d\n", data[1]);
	} else if (!strncmp(cmd, "getinfo", len)) {
		ilitek_ice_mode_ctrl(1, 0);
		ilitek_tddi_ic_get_info();
		ilitek_ice_mode_ctrl(0, 0);
		ilitek_tddi_ic_get_protocl_ver();
		ilitek_tddi_ic_get_fw_ver();
		ilitek_tddi_ic_get_core_ver();
		ilitek_tddi_ic_get_tp_info();
		ilitek_tddi_ic_get_panel_info();
		/* "91356063 add"@0xffffff8008a6d218 carica
		 * "2.0.6.0.191122"@0xffffff800923ed58 */
#line 1374
		ILI_INFO("Driver version = %s\n", "2.0.6.0.191122");
		/* "f9414903 ldr"@0xffffff8008a6d238: il campo a +656 passato
		 * a un `%s`, quindi un puntatore a testo. `ilitek.h` lo tiene
		 * `u64`: il cast e' dichiarato, non nascosto. */
#line 1375
		ILI_INFO("TP module = %s\n", (char *)(uintptr_t)idev->c656);
	} else if (!strncmp(cmd, "enableicemode", len)) {
		/* "7100051f cmp"@0xffffff8008a6d254 con 1, e
		 * "54fff541 b.ne"@0xffffff8008a6d258 verso il sito condiviso
		 * con "disableicemode": sono DUE `bl` distinti, non un
		 * `cset`. */
		if (data[1] == 1)
			ilitek_ice_mode_ctrl(1, 1);
		else
			ilitek_ice_mode_ctrl(1, 0);
	} else if (!strncmp(cmd, "wqctrl", len)) {
		/* "52000123 eor"@0xffffff8008a6d288 su un byte appena letto
		 * ("39483d09 ldrb"@0xffffff8008a6d27c) e riscritto
		 * ("39083d03 strb"@0xffffff8008a6d28c). Con `u8 c527` la
		 * forma `x = !x` costerebbe un `cmp`+`cset` in piu': vedi il
		 * delta di intestazione, voce D-H6. */
		idev->c527 ^= 1;
#line 1383
		ILI_INFO("wq_ctrl flag= %d\n", idev->c527);
	} else if (!strncmp(cmd, "disableicemode", len)) {
		ilitek_ice_mode_ctrl(0, 0);
	} else if (!strncmp(cmd, "enablewqesd", len)) {
		ilitek_tddi_wq_ctrl(0, 1);
	} else if (!strncmp(cmd, "enablewqbat", len)) {
		ilitek_tddi_wq_ctrl(1, 1);
	} else if (!strncmp(cmd, "disablewqesd", len)) {
		ilitek_tddi_wq_ctrl(0, 0);
	} else if (!strncmp(cmd, "disablewqbat", len)) {
		ilitek_tddi_wq_ctrl(1, 0);
	} else if (!strncmp(cmd, "gesture", len)) {
		/* "52000123 eor"@0xffffff8008a6d314 sul byte a +532 */
		idev->c532 ^= 1;
#line 1396
		ILI_INFO("gesture = %d\n", idev->c532);
	} else if (!strncmp(cmd, "esdgesture", len)) {
		/* "36fff100 tbz"@0xffffff8008a6d32c prova il solo bit di
		 * segno: e' `< 0`, non `!= 0`. */
		if (ilitek_tddi_gesture_recovery() < 0) {
#line 1399
			ILI_ERR("Gesture recovery failed\n");
			count = -1;
		}
	} else if (!strncmp(cmd, "esdspi", len)) {
		ilitek_tddi_spi_recovery();
	} else if (!strncmp(cmd, "sleepin", len)) {
		/* "9110c400 add"@0xffffff8008a6d364 carica
		 * "sleep"@0xffffff800911c431 */
		ilitek_tddi_ic_func_ctrl("sleep", 0);
	} else if (!strncmp(cmd, "deepsleepin", len)) {
		/* "320007e1 orr"@0xffffff8008a6d378 */
		ilitek_tddi_ic_func_ctrl("sleep", 3);
	} else if (!strncmp(cmd, "iceflag", len)) {
		/*
		 * LA RILETTURA E' MISURATA E DISCRIMINA.
		 * "1a9f17e8 cset"@0xffffff8008a6d3a0 (`eq`) e
		 * "b902c928 str"@0xffffff8008a6d3a4 scrivono il campo;
		 * "b942c923 ldr"@0xffffff8008a6d3a8 LO RILEGGE SUBITO, dalla
		 * stessa base e dallo stesso scarto, per darlo alla `printk`.
		 * Nessun compilatore lascia in piedi quella lettura se il
		 * sorgente la scrive come accesso ordinario. Provate quattro
		 * forme, tutte misurate sul `.o` vero:
		 *   `idev->c712 = (data[1] == 1); ... idev->c712`  -> 4664 B
		 *   la stessa con `if`/`else` invece del confronto    -> 4664 B
		 *   `WRITE_ONCE` sullo STORE                          -> 4664 B
		 *   lettura volatile sull'argomento della printk      -> 4668 B
		 * Solo l'ultima torna. Cio' che e' MISURATO e' che di
		 * fabbrica quella lettura non e' eliminabile; che il sorgente
		 * di fabbrica scrivesse `READ_ONCE`, `atomic_read` su un
		 * `atomic_t` a +712, o un altro accesso volatile equivalente,
		 * il binario NON lo dice -- e' una LETTURA, non una misura, ed
		 * e' riportata come tale nel delta di intestazione (D-H7).
		 */
		idev->c712 = (data[1] == 1);
#line 1413
		ILI_INFO("ice mode flag = %d\n", READ_ONCE(idev->c712));
	} else if (!strncmp(cmd, "gesturenormal", len)) {
		/* "528000a9 mov"@0xffffff8008a6d3c8 (5, `movz`) e
		 * "b9021909 str"@0xffffff8008a6d400 */
		idev->c536 = 5;
		ILI_INFO("gesture mode = %d\n", idev->c536);
	} else if (!strncmp(cmd, "gestureinfo", len)) {
		/* "321e03e9 orr"@0xffffff8008a6d3ec (4, immediato logico) */
		idev->c536 = 4;
		ILI_INFO("gesture mode = %d\n", idev->c536);
	} else if (!strncmp(cmd, "netlink", len)) {
		/* "52000123 eor"@0xffffff8008a6d430 sul byte a +530 */
		idev->c530 ^= 1;
#line 1422
		ILI_INFO("netlink flag= %d\n", idev->c530);
	} else if (!strncmp(cmd, "switchtestmode", len)) {
		/* "320003e0 orr"@0xffffff8008a6d43c */
		ilitek_tddi_switch_tp_mode(1);
	} else if (!strncmp(cmd, "switchdebugmode", len)) {
		/* "320003e0 orr"@0xffffff8008a6d454 */
		ilitek_set_tp_data_len(1);
	} else if (!strncmp(cmd, "switchdemomode", len)) {
		/* Nessun `mov w0`: w0 vale gia' 0 dal `cbz` della catena
		 * ("34003240 cbz"@0xffffff8008a6ce18). */
		ilitek_set_tp_data_len(0);
	} else if (!strncmp(cmd, "switchgesturedebugmode", len)) {
		/* "32000be0 orr"@0xffffff8008a6d45c */
		ilitek_set_tp_data_len(7);
	} else if (!strncmp(cmd, "dbgflag", len)) {
		/* "52000123 eor"@0xffffff8008a6d48c sul byte a +556 */
		idev->c556 ^= 1;
#line 1433
		ILI_INFO("debug flag message = %d\n", idev->c556);
	} else if (!strncmp(cmd, "spiclk", len)) {
		/* `data[1]` letto DUE volte:
		 * "b9400683 ldr"@0xffffff8008a6d498 per la printk e
		 * "b9400680 ldr"@0xffffff8008a6d4b4 per la chiamata. */
#line 1435
		ILI_INFO("spi clk num = %d\n", data[1]);
		core_spi_setup(data[1]);
	} else if (!strncmp(cmd, "ss", len)) {
		ILI_INFO("sense_stop = %d\n", data[1]);
		/* "1a9f07e8 cset"@0xffffff8008a6d4f0 e
		 * "390a0928 strb"@0xffffff8008a6d4f4 (byte a +642) */
		idev->c642 = (data[1] != 0);
	} else if (!strncmp(cmd, "iow", len)) {
		w_len = data[1];
#line 1443
		ILI_INFO("w_len = %d\n", w_len);
		/*
		 * L'indice di `data` parte da 2: la base del ciclo e'
		 * "b25f03fa orr"@0xffffff8008a6d538 (0x200000000), che
		 * "935eff48 asr"@0xffffff8008a6d54c riduce a 8 = 2 * 4, e il
		 * passo e' "b26003fc orr"@0xffffff8008a6d548 (0x100000000),
		 * cioe' 4 dopo lo stesso `asr`.
		 */
		for (i = 0; i < w_len; i++) {
			temp[i] = data[i + 2];
#line 1447
			ILI_INFO("write[%d] = %x\n", i, temp[i]);
		}
		/* "f9418508 ldr"@0xffffff8008a6d58c: il campo a +776 */
		idev->c776(temp, w_len);
	} else if (!strncmp(cmd, "ior", len)) {
		r_len = data[1];
#line 1454
		ILI_INFO("r_len = %d\n", r_len);
		/* "f9418908 ldr"@0xffffff8008a6d5d0: il campo a +784 */
		idev->c784(temp, r_len);
		for (i = 0; i < r_len; i++)
#line 1457
			ILI_INFO("read[%d] = %x\n", i, temp[i]);
	} else if (!strncmp(cmd, "iowr", len)) {
		w_len = data[1];
		r_len = data[2];
		delay = data[3];
		ILI_INFO("w_len = %d, r_len = %d, delay = %d\n",
#line 1464
			 w_len, r_len, delay);
		/* Base 0x400000000 ("b25e03fc orr"@0xffffff8008a6d65c), cioe'
		 * indice 4 dopo "935eff88 asr"@0xffffff8008a6d670. */
		for (i = 0; i < w_len; i++) {
			temp[i] = data[i + 4];
#line 1468
			ILI_INFO("write[%d] = %x\n", i, temp[i]);
		}
		idev->c776(temp, w_len);
		/* Il riazzeramento e' abbassato a coppie di `stp xzr`:
		 * "a90f7d1f stp"@0xffffff8008a6d6bc .. "f9001bff
		 * str"@0xffffff8008a6d700, 256 byte esatti. */
		memset(temp, 0, sizeof(temp));
		/* La guardia e il ciclo sono quelli di `mdelay` su un valore
		 * non costante: "34000108 cbz"@0xffffff8008a6d704 e
		 * "cb0803f5 neg"@0xffffff8008a6d70c, con
		 * "52912b00 mov"@0xffffff8008a6d710 che e' `udelay(1000)`. */
		mdelay(delay);
		idev->c784(temp, r_len);
		for (i = 0; i < r_len; i++)
#line 1476
			ILI_INFO("read[%d] = %x\n", i, temp[i]);
	} else if (!strncmp(cmd, "getddiregdata", len)) {
		ILI_INFO("Get ddi reg one page: page = %x, reg = %x\n",
			 data[1], data[2]);
		/* "910093e2 add"@0xffffff8008a6d7b8: il terzo argomento e'
		 * l'indirizzo di un byte sulla pila, azzerato prima
		 * ("390093ff strb"@0xffffff8008a6d7ac). */
		ilitek_tddi_ic_get_ddi_reg_onepage(data[1], data[2],
						   &ddi_data);
#line 1481
		ILI_INFO("ddi_data = %x\n", ddi_data);
	} else if (!strncmp(cmd, "setddiregdata", len)) {
		ILI_INFO("Set ddi reg one page: page = %x, reg = %x, data = %x\n",
#line 1483
			 data[1], data[2], data[3]);
		ilitek_tddi_ic_set_ddi_reg_onepage(data[1], data[2], data[3]);
	} else if (!strncmp(cmd, "dumpflashdata", len)) {
		/* "910a20a5 add"@0xffffff8008a6d828 carica
		 * "/sdcard/flash_dump"@0xffffff8009246288 */
		ILI_INFO("Start = 0x%x, End = 0x%x, Dump Hex path = %s\n",
#line 1486
			 data[1], data[2], "/sdcard/flash_dump");
		ilitek_tddi_fw_dump_flash_data(data[1], data[2], false);
	} else if (!strncmp(cmd, "dumpiramdata", len)) {
		/* "912e28a5 add"@0xffffff8008a6d864 carica
		 * "/sdcard/iram_dump"@0xffffff8009245b8a */
		ILI_INFO("Start = 0x%x, End = 0x%x, Dump IRAM path = %s\n",
#line 1489
			 data[1], data[2], "/sdcard/iram_dump");
		ilitek_fw_dump_iram_data(data[1], data[2], true);
	} else if (!strncmp(cmd, "edge_palm_ctrl", len)) {
		/* "91275000 add"@0xffffff8008a6d890 carica
		 * "edge_palm"@0xffffff80092419d4, che NON e' il testo del
		 * comando: il comando e' "edge_palm_ctrl". */
		ilitek_tddi_ic_func_ctrl("edge_palm", data[1]);
	} else if (!strncmp(cmd, "uart_mode_ctrl", len)) {
		ilitek_tddi_fw_uart_ctrl(data[1]);
	} else if (!strncmp(cmd, "flashesdgesture", len)) {
		ilitek_tddi_touch_esd_gesture_flash();
	} else if (!strncmp(cmd, "spiw", len)) {
		w_len = data[1];
		/*
		 * DIFETTO F10 DELLA FABBRICA: il byte di comando 0x82 scritto
		 * in `temp[0]` ("52801048 mov"@0xffffff8008a6d8c0,
		 * "3900c3e8 strb"@0xffffff8008a6d8c4) viene SOVRASCRITTO dal
		 * primo giro del ciclo, che parte anch'esso dall'indice 0
		 * ("38366b68 strb"@0xffffff8008a6d90c indicizza con x22 = i,
		 * e x27 = sp+0x30 e' lo stesso `temp[0]`). Sopravvive solo se
		 * `w_len` e' zero. Riprodotto.
		 */
		temp[0] = 0x82;
		for (i = 0; i < w_len; i++) {
			temp[i] = data[i + 2];
#line 1503
			ILI_INFO("write[%d] = %x\n", i, temp[i]);
		}
		IDEV_C792(idev->c8, temp, w_len, NULL, 0);
	} else if (!strncmp(cmd, "spir", len)) {
		r_len = data[1];
		/* "52901821 mov"@0xffffff8008a6d954 con
		 * "72a02801 movk"@0xffffff8008a6d958: 0x14080c1, cioe'
		 * GFP_KERNEL | __GFP_ZERO | __GFP_DMA. */
		rxbuf = kzalloc(r_len, GFP_DMA | GFP_KERNEL);
		/* "b140041f cmn"@0xffffff8008a6d968 e' l'IS_ERR
		 * (confronto con -4096), "b4000935 cbz"@0xffffff8008a6d970 il
		 * NULL. */
		if (IS_ERR(rxbuf) || rxbuf == NULL) {
			ILI_ERR("Failed to allocate packet memory, %ld\n",
				PTR_ERR(rxbuf));
		} else {
			temp[0] = 0x83;
			IDEV_C792(idev->c8, temp, 1, rxbuf, r_len);
			for (i = 0; i < r_len; i++)
#line 1519
				ILI_INFO("read[%d] = %x\n", i, rxbuf[i]);
		}
		/*
		 * DIFETTO F11 DELLA FABBRICA: la `kfree` e' anche sul ramo
		 * d'errore -- "97db168a bl"@0xffffff8008a6daac (la printk)
		 * CADE su "aa1503e0 mov"@0xffffff8008a6dab0 e
		 * "97df8b3f bl"@0xffffff8008a6dab4 -- quindi con un ERR_PTR
		 * si libera un puntatore che non e' memoria. Riprodotto.
		 */
		kfree(rxbuf);
	} else if (!strncmp(cmd, "spirw", len)) {
		w_len = data[1];
		r_len = data[2];
		/* Base 0x300000000 ("b26007fb orr"@0xffffff8008a6da04), cioe'
		 * indice 3. */
		for (i = 0; i < w_len; i++) {
			temp[i] = data[i + 3];
#line 1528
			ILI_INFO("write[%d] = %x\n", i, temp[i]);
		}
		/* "340003a8 cbz"@0xffffff8008a6da50: senza lunghezza di
		 * lettura non si alloca e `rxbuf` resta NULL
		 * ("aa1f03f7 mov"@0xffffff8008a6dac4). */
		if (r_len) {
			rxbuf = kzalloc(r_len, GFP_DMA | GFP_KERNEL);
			if (IS_ERR(rxbuf) || rxbuf == NULL) {
				ILI_ERR("Failed to allocate packet memory, %ld\n",
					PTR_ERR(rxbuf));
				goto spirw_free;
			}
		}
		IDEV_C792(idev->c8, temp, w_len, rxbuf, r_len);
		for (i = 0; i < r_len; i++)
#line 1541
			ILI_INFO("read[%d] = %x\n", i, rxbuf[i]);
spirw_free:
		/* Anche qui la `kfree` e' sul ramo d'errore (difetto F11):
		 * "97db1692 bl"@0xffffff8008a6da8c salta a
		 * "aa1703e0 mov"@0xffffff8008a6db34. */
		kfree(rxbuf);
	} else {
#line 1545
		ILI_ERR("Unknown command\n");
		count = -1;
	}

	/* "b4000074 cbz"@0xffffff8008a6d14c protegge la `kfree`, e non c'e'
	 * nessuno store di NULL dopo. */
	if (data != NULL)
		kfree(data);

	mutex_unlock(&idev->touch_mutex);
	return count;
}

/*
 * ===========================================================================
 * I COMANDI IOCTL: I VALORI SONO MISURATI, I NOMI NON ESISTONO
 * ===========================================================================
 * Il binario porta i valori numerici dei comandi e NON li nomina: nessuna
 * stringa, nessuna `.ddebug`, nessun `__func__` dice come si chiamino le
 * ventotto costanti. Dare loro un nome descrittivo sarebbe inventare
 * (regola 5), e un nome inventato e' indistinguibile da un fatto per chi
 * legge. Qui sono scritte con le macro del kernel, che ricostruiscono
 * ESATTAMENTE il numero che il binario confronta:
 *
 *   _IOWR(0x64, nr, u32)           = 0xC00464nn
 *   _IOWR(0x64, nr, unsigned long) = 0xC00864nn
 *
 * e i due valori sono verificabili uno per uno contro il disassemblato:
 * "528c8001 mov"@0xffffff8008a6f1dc con "72b80101 movk"@0xffffff8008a6f1e4
 * costruiscono 0xC0086400, "528c8021 mov"@0xffffff8008a6f254 con
 * "72b80081 movk"@0xffffff8008a6f25c costruiscono 0xC0046401.
 *
 * L'INDICE DELLO `switch` e' misurato e vale per tutte e tre le tabelle di
 * salto del file: "52938008 mov"@0xffffff8008a6f18c con
 * "72a7ff68 movk"@0xffffff8008a6f190 caricano 0x3FFB9C00, che
 * "0b0802a8 add"@0xffffff8008a6f194 somma al comando -- cioe' `cmd`
 * MENO 0xC0046400, perche' 0xC0046400 + 0x3FFB9C00 = 2^32 -- e
 * "71006d1f cmp"@0xffffff8008a6f198 lo confronta con 27: i casi sono i
 * VENTOTTO valori 0xC0046400 .. 0xC004641B, densi e senza buchi.
 * La tabella e' a 0xffffff8008f7ecba, 28 mezze parole, base del `br`
 * 0xffffff8008a6f1b8 ("d61f0140 br"@0xffffff8008a6f1b4).
 */

/*
 * ilitek_node_compat_ioctl -- 0xffffff8008a6f138, 1840 byte, `t` (`static`),
 * righe 1560..1680 del sorgente di fabbrica.
 *
 * Non fa altro che RIMAPPARE il comando a 32 bit dello spazio utente a 32
 * bit sul comando nativo e richiamare la `.unlocked_ioctl` dello stesso
 * file. Ventotto casi; in tredici il comando nativo ha la stessa taglia
 * (`u32`) e resta identico, negli altri quindici la taglia diventa
 * `unsigned long` (0xC008...). La ripartizione e' letta caso per caso dai
 * "movk" della tabella qui sotto, non dedotta.
 *
 * Tutte le `printk` dei ventotto casi sono `ILI_DBG`: sono precedute dal
 * `ldrb` del byte diagnostico e da un `cbz` che salta anche la costruzione
 * degli argomenti ("396562c8 ldrb"@0xffffff8008a6f1b8 con
 * "340000e8 cbz"@0xffffff8008a6f1bc e' il primo dei ventinove).
 *
 * I due errori NON sono `ILI_DBG` ma `ILI_ERR` (livello \x013) e non hanno
 * guardia.
 */
static long ilitek_node_compat_ioctl(struct file *filp, unsigned int cmd,
				     unsigned long arg)
{
	long ret = 0;

	/*
	 * "f9401408 ldr"@0xffffff8008a6f148 legge `filp + 40` e
	 * "f9402508 ldr"@0xffffff8008a6f150 legge `+72` di cio' che ha
	 * trovato: sono `offsetof(struct file, f_op)` e
	 * `offsetof(struct file_operations, unlocked_ioctl)` del 4.14.
	 * "b27bf7e0 orr"@0xffffff8008a6f208 e' -25, cioe' -ENOTTY.
	 */
	if (!filp->f_op || !filp->f_op->unlocked_ioctl) {
#line 1560
		ILI_ERR("There's no unlocked_ioctl defined in file\n");
		return -ENOTTY;
	}

	/* "12001ea3 and"@0xffffff8008a6f178: il comando mascherato a un byte,
	 * cioe' `_IOC_NR`. */
#line 1564
	ILI_DBG("cmd = %d\n", _IOC_NR(cmd));

	switch (cmd) {
	case _IOWR(0x64, 0, u32):
		ILI_DBG("compat_ioctl: convert i2c/spi write\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 0, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 1, u32):
#line 1576
		ILI_DBG("compat_ioctl: convert set write length\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 1, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 2, u32):
#line 1572
		ILI_DBG("compat_ioctl: convert i2c/spi read\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 2, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 3, u32):
#line 1580
		ILI_DBG("compat_ioctl: convert set read length\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 3, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 4, u32):
#line 1584
		ILI_DBG("compat_ioctl: convert hw reset\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 4, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 5, u32):
#line 1588
		ILI_DBG("compat_ioctl: convert power switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 5, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 6, u32):
#line 1592
		ILI_DBG("compat_ioctl: convert report switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 6, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 7, u32):
#line 1596
		ILI_DBG("compat_ioctl: convert irq switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 7, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 8, u32):
#line 1600
		ILI_DBG("compat_ioctl: convert debug level\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 8, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 9, u32):
#line 1604
		ILI_DBG("compat_ioctl: convert format mode\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 9, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 10, u32):
		/* Il formato e' LO STESSO del caso 3 ("convert set read
		 * length"): "9102a400 add"@0xffffff8008a6f450 e
		 * "9102a400 add"@0xffffff8008a6f2ac caricano lo stesso
		 * indirizzo 0xffffff80092490a9. */
#line 1608
		ILI_DBG("compat_ioctl: convert set read length\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 10, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 11, u32):
#line 1612
		ILI_DBG("compat_ioctl: convert fw version\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 11, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 12, u32):
#line 1616
		ILI_DBG("compat_ioctl: convert core version\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 12, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 13, u32):
#line 1620
		ILI_DBG("compat_ioctl: convert driver version\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 13, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 14, u32):
#line 1624
		ILI_DBG("compat_ioctl: convert chip id\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 14, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 15, u32):
#line 1628
		ILI_DBG("compat_ioctl: convert netlink ctrl\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 15, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 16, u32):
#line 1632
		ILI_DBG("compat_ioctl: convert netlink status\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 16, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 17, u32):
#line 1636
		ILI_DBG("compat_ioctl: convert tp mode ctrl\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 17, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 18, u32):
#line 1640
		ILI_DBG("compat_ioctl: convert tp mode status\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 18, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 19, u32):
#line 1644
		ILI_DBG("compat_ioctl: convert tp mode switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 19, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 20, u32):
#line 1648
		ILI_DBG("compat_ioctl: convert interface type\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 20, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 21, u32):
#line 1652
		ILI_DBG("compat_ioctl: convert dump flash\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 21, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 22, u32):
#line 1656
		ILI_DBG("compat_ioctl: convert fw uart\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 22, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 23, u32):
#line 1660
		ILI_DBG("compat_ioctl: convert resolution\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 23, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 24, u32):
#line 1664
		ILI_DBG("compat_ioctl: convert tp info\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 24, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 25, u32):
#line 1668
		ILI_DBG("compat_ioctl: convert wrapper rw\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 25, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 26, u32):
#line 1672
		ILI_DBG("compat_ioctl: convert ddi write\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 26, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 27, u32):
#line 1676
		ILI_DBG("compat_ioctl: convert ddi read\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 27, u32),
			(unsigned long)compat_ptr(arg));
		break;
	default:
		/* "92804040 mov"@0xffffff8008a6f228 e' -515, cioe'
		 * -ENOIOCTLCMD. Il testo dice «return ilitek_node_ioctl» e il
		 * codice NON la chiama: e' il messaggio della fabbrica e si
		 * riporta com'e' (regola 7). */
#line 1680
		ILI_ERR("no ioctl cmd, return ilitek_node_ioctl\n");
		return -ENOIOCTLCMD;
	}

	return ret;
}

/*
 * ilitek_node_ioctl -- 0xffffff8008a6db40, 5624 byte, `t` (`static`),
 * righe 1695..2058 del sorgente di fabbrica. E' la funzione piu' grande del
 * gruppo H.
 *
 * DUE tabelle di salto, non una: clang ha spezzato lo `switch` in DUE
 * intervalli densi, perche' i ventotto comandi non sono contigui come
 * numero -- tredici hanno taglia `u32` (0xC004...) e quindici taglia
 * `unsigned long` (0xC008...).
 *   "72a7fee8 movk"@0xffffff8008a6dc9c costruisce 0x3FF79C00, cioe'
 *   -0xC0086400: 28 voci a 0xffffff8008f7ec82, base del `br`
 *   0xffffff8008a6dcc8 ("d61f0140 br"@0xffffff8008a6dcc4);
 *   "72a7ff68 movk"@0xffffff8008a6dda8 costruisce 0x3FFB9BFF, cioe'
 *   -0xC0046401: 21 voci a 0xffffff8008f7ec58, base del `br`
 *   0xffffff8008a6ddd0 ("d61f0140 br"@0xffffff8008a6ddcc).
 * Le voci mancanti delle due tabelle puntano tutte al blocco comune di
 * uscita 0xffffff8008a6eeb4, che e' il `default`.
 *
 * VERIFICA INCROCIATA CHE NON ERA NEL MANDATO: la ripartizione fra taglia 4
 * e taglia 8 letta qui combacia ESATTAMENTE, comando per comando, con quella
 * che `ilitek_node_compat_ioctl` produce come secondo argomento della
 * `.unlocked_ioctl`. Sono due letture indipendenti (una tabella di salto e
 * ventotto coppie mov/movk) e danno lo stesso insieme: nr 1, 3, 4, 5, 6, 7,
 * 8, 9, 19, 21 a taglia `u32`, gli altri diciotto a taglia `unsigned long`.
 *
 * IL VALORE DI RITORNO DI DEFAULT e' materializzato PRIMA della
 * distribuzione ("321b77f5 orr"@0xffffff8008a6dca8 carica -25 in w21): ogni
 * caso che riesce lo riazzera. `ret` e' un `int` e la funzione ritorna
 * `long`, come dice "93407ea0 sxtw"@0xffffff8008a6dd6c.
 *
 * LE DUE CODE DI LAVORO sono lette UNA volta sola, in ingresso
 * ("3948411a ldrb"@0xffffff8008a6dc20 e "39484519 ldrb"@0xffffff8008a6dc24),
 * PRIMA del `mutex_lock` e della `kzalloc`, e i valori sopravvivono in
 * registri salvati fino alla riabilitazione finale
 * ("3400009a cbz"@0xffffff8008a6dd40 e "34000099 cbz"@0xffffff8008a6dd50).
 * Rileggerli dopo la `kzalloc` non e' possibile: due chiamate opache stanno
 * in mezzo, e il compilatore non potrebbe sollevare le letture. Sono quindi
 * DUE LOCALI nel sorgente, non due accessi ripetuti.
 *
 * TRE DIFETTI DELLA FABBRICA, riprodotti (regola 7): F12, F13, F14 nel
 * cappello del file.
 */
static long ilitek_node_ioctl(struct file *filp, unsigned int cmd,
			      unsigned long arg)
{
	int ret = 0;
	u8 *szBuf = NULL;
	u8 if_to_user = 0;
	u32 buf[64] = {0};
	/*
	 * Lette una volta in ingresso: vedi il cappello. `c528` e `c529` sono
	 * i due byte che dicono se la coda ESD e quella della batteria erano
	 * in funzione; il primo argomento di `ilitek_tddi_wq_ctrl` e' 0 per
	 * l'ESD e 1 per la batteria ("2a1f03e0 mov"@0xffffff8008a6dc7c contro
	 * "320003e0 orr"@0xffffff8008a6dc8c).
	 */
	u8 c528 = idev->c528;
	u8 c529 = idev->c529;
	u32 c48_40, c48_44;

	/* "12181c29 and"@0xffffff8008a6db68 con
	 * "6b0a013f cmp"@0xffffff8008a6db70: il tipo del comando contro 0x64. */
	if (_IOC_TYPE(cmd) != 0x64) {
#line 1695
		ILI_ERR("The Magic number doesn't match\n");
		return -ENOTTY;
	}

	/* "7100707f cmp"@0xffffff8008a6dbcc con
	 * "540001e3 b.cc"@0xffffff8008a6dbd0: il numero contro 28. */
	if (_IOC_NR(cmd) >= 28) {
#line 1700
		ILI_ERR("The number of ioctl doesn't match\n");
		return -ENOTTY;
	}

	ILI_DBG("cmd = %d\n", _IOC_NR(cmd));

	mutex_lock(&idev->touch_mutex);

	/* "321403e2 orr"@0xffffff8008a6dc60: 4096 byte, e
	 * "72a02801 movk"@0xffffff8008a6dc5c con
	 * "52901801 mov"@0xffffff8008a6dc58: GFP_KERNEL | __GFP_ZERO, SENZA
	 * __GFP_DMA (qui la maschera e' 0x14080c0, non 0x14080c1). */
	szBuf = kzalloc(4096, GFP_KERNEL);
	if (IS_ERR(szBuf) || szBuf == NULL) {
#line 1710
		ILI_ERR("Failed to allocate mem\n");
		ret = -ENOMEM;
		goto out_no_free;
	}

	if (c528)
		ilitek_tddi_wq_ctrl(0, 0);
	if (c529)
		ilitek_tddi_wq_ctrl(1, 0);

	switch (cmd) {
	case _IOWR(0x64, 0, unsigned long):
		ILI_DBG("ioctl: write len = %d\n", ioctl_len);
		/* "794dd2d5 ldrh"@0xffffff8008a6dcf4 rilegge il globale dopo
		 * la printk, "714006bf cmp"@0xffffff8008a6dcf8 lo confronta
		 * con 4096 e "54006669 b.ls"@0xffffff8008a6dcfc prosegue: il
		 * ramo d'errore e' il MAGGIORE STRETTO. */
		if (ioctl_len > 4096) {
			ILI_ERR("ERROR! write len is largn than ioctl buf (%d, %ld)\n",
#line 1726
				ioctl_len, 4096L);
			ret = -ENOTTY;
			break;
		}
		if (copy_from_user(szBuf, (u8 __user *)arg, ioctl_len)) {
#line 1732
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		/* "f9418508 ldr"@0xffffff8008a6ea30: il campo a +776. */
		ret = idev->c776(szBuf, ioctl_len);
		if (ret < 0)
			ILI_ERR("Failed to write data\n");
		break;

	case _IOWR(0x64, 1, u32):
		/* "790dd114 strh"@0xffffff8008a6ddd8: due byte, non quattro. */
		ioctl_len = arg;
		ret = 0;
		break;

	case _IOWR(0x64, 2, unsigned long):
#line 1742
		ILI_DBG("ioctl: read len = %d\n", ioctl_len);
		if (ioctl_len > 4096) {
			ILI_ERR("ERROR! read len is largn than ioctl buf (%d, %ld)\n",
#line 1746
				ioctl_len, 4096L);
			ret = -ENOTTY;
			break;
		}
		/* "f9418908 ldr"@0xffffff8008a6ea68: il campo a +784. */
		ret = idev->c784(szBuf, ioctl_len);
		if (ret < 0) {
			ILI_ERR("Failed to read data\n");
			break;
		}
		if (copy_to_user((u8 __user *)arg, szBuf, ioctl_len)) {
#line 1758
			ILI_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 3, u32):
		ioctl_len = arg;
		ret = 0;
		break;

	case _IOWR(0x64, 4, u32):
#line 1767
		ILI_DBG("ioctl: hw reset\n");
		/* "b9426500 ldr"@0xffffff8008a6e694: il campo a +612, letto a
		 * 32 bit e passato direttamente. */
		ilitek_tddi_reset_ctrl(idev->c612);
		ret = 0;
		break;

	case _IOWR(0x64, 5, u32):
#line 1771
		ILI_DBG("Not implemented yet\n");
		ret = 0;
		break;

	case _IOWR(0x64, 6, u32):
		if (copy_from_user(szBuf, (u8 __user *)arg, 1)) {
#line 1775
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
#line 1780
		ILI_DBG("ioctl: report switch = %d\n", szBuf[0]);
		if (szBuf[0]) {
			idev->c531 = 1;
			ILI_DBG("report is enabled\n");
		} else {
			idev->c531 = 0;
			ILI_DBG("report is disabled\n");
		}
		ret = 0;
		break;

	case _IOWR(0x64, 7, u32):
		if (copy_from_user(szBuf, (u8 __user *)arg, 1)) {
#line 1791
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: irq switch = %d\n", szBuf[0]);
		if (szBuf[0])
			ilitek_plat_irq_enable();
		else
			ilitek_plat_irq_disable();
		ret = 0;
		break;

	case _IOWR(0x64, 8, u32):
		/*
		 * DIFETTO F12 DELLA FABBRICA: legge QUATTRO byte dallo spazio
		 * utente ("321e03e2 orr"@0xffffff8008a6e810) e poi NON LI USA:
		 * il livello diagnostico viene invertito
		 * ("52000103 eor"@0xffffff8008a6e830) senza guardare `szBuf`.
		 * Riprodotto.
		 * La `printk` di questo caso NON e' protetta dal byte
		 * diagnostico: "97db1325 bl"@0xffffff8008a6e840 e' raggiunta
		 * senza nessun `cbz`.
		 */
		if (copy_from_user(szBuf, (u8 __user *)arg, sizeof(u32))) {
#line 1803
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ilitek_dbg_en ^= 1;
#line 1809
		ILI_INFO("ipio_debug_level = %d", ilitek_dbg_en);
		ret = 0;
		break;

	case _IOWR(0x64, 9, u32):
		if (copy_from_user(szBuf, (u8 __user *)arg, 3)) {
#line 1813
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: set func mode = %x,%x,%x\n",
			szBuf[0], szBuf[1], szBuf[2]);
		idev->c776(szBuf, 3);
		ret = 0;
		break;

	case _IOWR(0x64, 10, unsigned long):
#line 1822
		ILI_DBG("ioctl: get fw version\n");
		/*
		 * QUATTRO letture distinte di `idev->c48` e del suo campo a
		 * +48 ("f9401989 ldr"@0xffffff8008a6eb00 seguita da
		 * "b9403129 ldr"@0xffffff8008a6eb04 e' la seconda delle
		 * quattro): ogni `strb` su `szBuf` puo' aliasare, e il
		 * compilatore ricarica. Il sorgente rilegge, non estrae un
		 * locale.
		 * Gli argomenti della `printk` vengono dai byte appena
		 * scritti: "ubfx w4, w11, #16, #8"@0xffffff8008a6eb40 e'
		 * esattamente `szBuf[1]` riletto.
		 */
		szBuf[3] = IDEV_C48_AT(48) & 0xFF;
		szBuf[2] = (IDEV_C48_AT(48) >> 8) & 0xFF;
		szBuf[1] = (IDEV_C48_AT(48) >> 16) & 0xFF;
		szBuf[0] = (IDEV_C48_AT(48) >> 24) & 0xFF;
		ILI_DBG("Firmware version = %d.%d.%d.%d\n",
#line 1828
			szBuf[0], szBuf[1], szBuf[2], szBuf[3]);
		if (copy_to_user((u8 __user *)arg, szBuf, sizeof(u32))) {
#line 1831
			ILI_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 11, unsigned long):
#line 1836
		ILI_DBG("ioctl: get protocl version\n");
		/* "f9401d69 ldr"@0xffffff8008a6ebbc legge il campo a +56 e
		 * "b9400129 ldr"@0xffffff8008a6ebc0 il valore al suo scarto 0.
		 * Tre byte, non quattro. */
		szBuf[2] = IDEV_C56_AT(0) & 0xFF;
		szBuf[1] = (IDEV_C56_AT(0) >> 8) & 0xFF;
		szBuf[0] = (IDEV_C56_AT(0) >> 16) & 0xFF;
		ILI_DBG("Protocol version = %d.%d.%d\n",
#line 1841
			szBuf[0], szBuf[1], szBuf[2]);
		if (copy_to_user((u8 __user *)arg, szBuf, 3)) {
#line 1844
			ILI_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 12, unsigned long):
#line 1849
		ILI_DBG("ioctl: get core version\n");
		/* Stesso schema del caso 10, ma allo scarto +52:
		 * "f9401989 ldr"@0xffffff8008a6ec74 poi
		 * "b940352a ldr"@0xffffff8008a6ec78. */
		szBuf[3] = IDEV_C48_AT(52) & 0xFF;
		szBuf[2] = (IDEV_C48_AT(52) >> 8) & 0xFF;
		szBuf[1] = (IDEV_C48_AT(52) >> 16) & 0xFF;
		szBuf[0] = (IDEV_C48_AT(52) >> 24) & 0xFF;
		ILI_DBG("Core version = %d.%d.%d.%d\n",
#line 1855
			szBuf[0], szBuf[1], szBuf[2], szBuf[3]);
		if (copy_to_user((u8 __user *)arg, szBuf, sizeof(u32))) {
#line 1858
			ILI_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 13, unsigned long):
#line 1863
		ILI_DBG("ioctl: get driver version\n");
		/* La copia e' di QUINDICI byte (8 + 7 sovrapposti,
		 * "f9400108 ldr"@0xffffff8008a6def4 e
		 * "f8407109 ldur"@0xffffff8008a6def0), cioe' testo PIU' il
		 * terminatore; la copia verso l'utente e' di QUATTORDICI
		 * ("321f0be2 orr"@0xffffff8008a6df34), cioe' senza. */
		strcpy(szBuf, "2.0.6.0.191122");
		if (copy_to_user((u8 __user *)arg, szBuf,
				 strlen("2.0.6.0.191122"))) {
#line 1867
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 14, unsigned long):
#line 1872
		ILI_DBG("ioctl: get chip id\n");
		/*
		 * UNA sola lettura di `idev->c48`
		 * ("f9401908 ldr"@0xffffff8008a6df84), poi
		 * "b9400909 ldr"@0xffffff8008a6df88 allo scarto 8 e i due
		 * scarti 40 e 44 letti APPAIATI
		 * ("29452109 ldp"@0xffffff8008a6df90) e scritti appaiati
		 * ("2902a3e9 stp"@0xffffff8008a6df94).
		 * L'APPAIAMENTO E' UNA MISURA, e discrimina: scritto come tre
		 * assegnamenti consecutivi `buf[0]=..; buf[1]=..; buf[2]=..;`
		 * clang NON appaia (misurato: 5632 byte invece di 5624, cioe'
		 * due istruzioni in piu'), perche' lo `str` intermedio sulla
		 * pila puo' aliasare il secondo `ldr`. Con i due valori letti
		 * PRIMA in due locali l'appaiamento torna. Quale delle due
		 * forme stesse nel sorgente di fabbrica il binario non lo
		 * dice; dice che una delle due e' esclusa.
		 */
		c48_40 = IDEV_C48_AT(40);
		c48_44 = IDEV_C48_AT(44);
		buf[0] = IDEV_C48_AT(8);
		buf[1] = c48_40;
		buf[2] = c48_44;
		if (copy_to_user((u32 __user *)arg, buf, 3 * sizeof(u32))) {
#line 1879
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 15, unsigned long):
		if (copy_from_user(szBuf, (u8 __user *)arg, 1)) {
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: netlink ctrl = %d\n", szBuf[0]);
		if (szBuf[0]) {
			idev->c530 = 1;
			ILI_DBG("ioctl: Netlink is enabled\n");
		} else {
			idev->c530 = 0;
			ILI_DBG("ioctl: Netlink is disabled\n");
		}
		ret = 0;
		break;

	case _IOWR(0x64, 16, unsigned long):
#line 1900
		ILI_DBG("ioctl: get netlink stat = %d\n", idev->c530);
		/*
		 * DIFETTO F13 DELLA FABBRICA: copia QUATTRO byte
		 * ("321e03e2 orr"@0xffffff8008a6e0f0) a partire da un campo
		 * che ne occupa UNO ("39484903 ldrb"@0xffffff8008a6e0a4 e
		 * "39084903 strb"@0xffffff8008a6d434 nel gruppo H lo trattano
		 * come byte): tre byte della struttura finiscono nello spazio
		 * utente. Riprodotto.
		 */
		if (copy_to_user((u8 __user *)arg, &idev->c530, 4)) {
#line 1902
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 17, unsigned long):
		if (copy_from_user(szBuf, (u8 __user *)arg, sizeof(u32))) {
#line 1908
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: switch fw format = %d\n", szBuf[0]);
		switch (szBuf[0]) {
		case 0:
			/* "3949ed08 ldrb"@0xffffff8008a6e198: il byte a
			 * +635. */
			if (idev->c635) {
				if (ilitek_set_tp_data_len(6) < 0) {
#line 1916
					ILI_ERR("Failed to set demo len from gesture mode\n");
					ret = -ENOTTY;
					break;
				}
			} else {
				if (ilitek_tddi_switch_tp_mode(0) < 0) {
#line 1921
					ILI_ERR("Failed to switch demo mode\n");
					ret = -ENOTTY;
					break;
				}
			}
			ret = 0;
			break;
		case 1:
			if (ilitek_tddi_switch_tp_mode(1) < 0) {
#line 1927
				ILI_ERR("Failed to switch test mode\n");
				ret = -ENOTTY;
				break;
			}
			ret = 0;
			break;
		case 2:
			/*
			 * `+635` e' letto DUE volte, e la seconda lettura sta
			 * DOPO `ilitek_tddi_switch_tp_mode`: e' il sorgente a
			 * rileggerlo ("3949ed08 ldrb"@0xffffff8008a6ee6c dopo
			 * "97ff928b bl"@0xffffff8008a6ee60).
			 */
			if (!idev->c635 && idev->c320) {
				if (ilitek_tddi_switch_tp_mode(0) < 0) {
#line 1933
					ILI_ERR("Failed to switch demo mode\n");
					ret = -ENOTTY;
					break;
				}
			}
			if (idev->c635) {
				if (ilitek_set_tp_data_len(7) < 0) {
#line 1941
					ILI_ERR("Failed to set debug len from gesture mode\n");
					ret = -ENOTTY;
					break;
				}
			} else {
				if (ilitek_set_tp_data_len(1) < 0) {
#line 1946
					ILI_ERR("Failed to set debug len\n");
					ret = -ENOTTY;
					break;
				}
			}
			ret = 0;
			break;
		default:
#line 1951
			ILI_ERR("Unknown TP mode ctrl\n");
			ret = -ENOTTY;
			break;
		}
		break;

	case _IOWR(0x64, 18, unsigned long):
#line 1956
		ILI_DBG("ioctl: current firmware mode = %d", idev->c320);
		if (copy_to_user((u8 __user *)arg, &idev->c320,
				 sizeof(idev->c320))) {
#line 1958
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 19, u32):
		if (copy_from_user(szBuf, (u8 __user *)arg, 1)) {
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: switch ice mode = %d", szBuf[0]);
		if (szBuf[0]) {
			idev->c712 = 1;
			ILI_DBG("ioctl: set ice mode enabled\n");
		} else {
			idev->c712 = 0;
			ILI_DBG("ioctl: set ice mode disabled\n");
		}
		ret = 0;
		break;

	case _IOWR(0x64, 20, unsigned long):
		/* NESSUNA `printk` qui: il byte a scarto 0 di `idev->c40`
		 * ("39400108 ldrb"@0xffffff8008a6e258) copiato in un byte
		 * della pila e poi verso l'utente. */
		if_to_user = *(u8 *)idev->c40;
		if (copy_to_user((u8 __user *)arg, &if_to_user, 1)) {
#line 1981
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 21, u32):
#line 1986
		ILI_DBG("ioctl: dump flash data\n");
		/* "320003e2 orr"@0xffffff8008a6e994 e' il terzo argomento a 1,
		 * i primi due sono zero. */
		ret = ilitek_tddi_fw_dump_flash_data(0, 0, true);
		if (ret < 0)
#line 1989
			ILI_ERR("ioctl: Failed to dump flash data\n");
		break;

	case _IOWR(0x64, 22, unsigned long):
		if (copy_from_user(szBuf, (u8 __user *)arg, 1)) {
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: fw UART  = %d\n", szBuf[0]);
		ilitek_tddi_fw_uart_ctrl(szBuf[0]);
		/* "3949d908 ldrb"@0xffffff8008a6e330: il byte a +630. */
		if_to_user = idev->c630;
		if (copy_to_user((u8 __user *)arg, &if_to_user, 1)) {
#line 2005
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 23, unsigned long):
#line 2010
		ILI_DBG("ioctl: get panel resolution\n");
		/* "7941e909 ldrh"@0xffffff8008a6e3b4 (+244) e
		 * "7941ed08 ldrh"@0xffffff8008a6e3b8 (+246). */
		buf[0] = idev->c244;
		buf[1] = idev->c246;
		if (copy_to_user((u32 __user *)arg, buf, 2 * sizeof(u32))) {
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 24, unsigned long):
#line 2021
		ILI_DBG("ioctl: get tp info\n");
		buf[0] = idev->c240;
		buf[1] = idev->c242;
		buf[2] = idev->c236;
		buf[3] = idev->c238;
		buf[4] = idev->c248;
		buf[5] = idev->c249;
		buf[6] = idev->c250;
		buf[7] = idev->c251;
		if (copy_to_user((u32 __user *)arg, buf, 8 * sizeof(u32))) {
#line 2033
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 25, unsigned long):
		/* Livello \x016 (KERN_INFO) e NESSUNA guardia diagnostica: e'
		 * una ILI_INFO, non una ILI_DBG. */
#line 2038
		ILI_INFO("Not supported in this version\n");
		ret = -ENOTTY;
		break;

	case _IOWR(0x64, 26, unsigned long):
		if (copy_from_user(szBuf, (u8 __user *)arg, 3)) {
#line 2043
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: page = %x, reg = %x, data = %x\n",
#line 2047
			szBuf[0], szBuf[1], szBuf[2]);
		ilitek_tddi_ic_set_ddi_reg_onepage(szBuf[0], szBuf[1],
						   szBuf[2]);
		ret = 0;
		break;

	case _IOWR(0x64, 27, unsigned long):
		if (copy_from_user(szBuf, (u8 __user *)arg, 2)) {
#line 2052
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: page = %x, reg = %x\n", szBuf[0], szBuf[1]);
		/* "91000a75 add"@0xffffff8008a6e5e8: il terzo argomento e'
		 * `szBuf + 2`, e il byte che ne esce e' riletto da li'
		 * ("394002a3 ldrb"@0xffffff8008a6e5fc). */
		ilitek_tddi_ic_get_ddi_reg_onepage(szBuf[0], szBuf[1],
						   szBuf + 2);
#line 2058
		ILI_DBG("ioctl: data = %x\n", szBuf[2]);
		if (copy_to_user((u8 __user *)arg, szBuf + 2, 1)) {
			ILI_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;

	default:
		/*
		 * DIFETTO F14 DELLA FABBRICA: il `default` non stampa niente.
		 * Il blocco 0xffffff8008a6eeb4 e' raggiunto direttamente dai
		 * due `b.hi` ("54008808 b.hi"@0xffffff8008a6ddb4) e da tutte
		 * le voci mancanti delle due tabelle, e non contiene nessuna
		 * `bl <printk>`. Riprodotto.
		 */
		ret = -ENOTTY;
		break;
	}

	kfree(szBuf);

out_no_free:
	if (c528)
		ilitek_tddi_wq_ctrl(0, 1);
	if (c529)
		ilitek_tddi_wq_ctrl(1, 1);

	mutex_unlock(&idev->touch_mutex);
	return ret;
}

/*
 * netlink_reply_msg -- 0xffffff8008a7156c, 380 byte, `T` (chiamata da
 * 0xffffff8008a5ccf8, fuori dal blocco).
 * La dimensione dello `skb` e' `NLMSG_SPACE(size)`: il binario calcola
 * `(size + 19) & ~3` ("11004e88 add"@0xffffff8008a715f0 e
 * "121e7515 and"@0xffffff8008a715f4), che e' ALIGN(16 + size, 4).
 * `alloc_skb` riceve priorita' ZERO ("2a1f03e1 mov"@0xffffff8008a71600):
 * nessuna maschera GFP. E' cosi' nel binario e si riporta com'e'.
 * Il tipo di messaggio e' 3 ("320007e3 orr"@0xffffff8008a71630) e la bandiera
 * di `netlink_unicast` e' MSG_DONTWAIT ("321a03e3 orr"@0xffffff8008a71680).
 * Lo slot azzerato a +56 dello `skb` ("b900391f str"@0xffffff8008a71668) e'
 * `NETLINK_CB(skb).dst_group` col layout 4.14 di `netlink_skb_parms`
 * (cb a +40, portid a +52, dst_group a +56): l'assegnazione al NOME e' una
 * lettura del layout noto, cio' che e' misurato e' "una scrittura di zero a
 * +56 dello skb".
 */
void netlink_reply_msg(void *raw, int size)
{
	int ret;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;

#line 2176
	ILI_INFO("The size of data being sent to user = %d\n", size);
	ILI_INFO("pid = %d\n", netlink_pid);
	ILI_INFO("Netlink is enable = %d\n", idev->c530);

	if (idev->c530) {
		skb = alloc_skb(NLMSG_SPACE(size), 0);
		netlink_skb = skb;
		if (!skb) {
			ILI_ERR("Failed to allocate new skb\n");
			return;
		}

		nlh = nlmsg_put(skb, 0, 0, 3, size, 0);
		netlink_nlh = nlh;
		NETLINK_CB(netlink_skb).dst_group = 0;
		memcpy(NLMSG_DATA(nlh), raw, size);
		ret = netlink_unicast(netlink_sk, netlink_skb, netlink_pid,
				      MSG_DONTWAIT);
		if (ret < 0)
#line 2196
			ILI_ERR("Failed to send data back to user\n");
	}
}

/*
 * netlink_recv_msg -- 0xffffff8008a72120, 252 byte, `t`.
 * E' il `.input` della configurazione netlink: l'UNICA rilocazione dei 48
 * byte a 0xffffff8008f7ed00 sta a +8 e punta qui.
 * Il pid arriva da `nlh->nlmsg_pid`, a scarto 12 dell'intestazione
 * ("b9400d03 ldr"@0xffffff8008a721ac), e finisce nel globale a
 * 0xffffff800a1006c4 ("b906c6c3 str"@0xffffff8008a721b4).
 */
static void netlink_recv_msg(struct sk_buff *skb)
{
	netlink_pid = 0;

#line 2204
	ILI_INFO("Netlink = %d\n", idev->c530);

	netlink_nlh = (struct nlmsghdr *)skb->data;
	ILI_INFO("Received a request from client: %s, %d\n",
		 (char *)NLMSG_DATA(netlink_nlh),
		 (int)strlen((char *)NLMSG_DATA(netlink_nlh)));

	netlink_pid = netlink_nlh->nlmsg_pid;
#line 2214
	ILI_INFO("the pid of sending process = %d\n", netlink_pid);

	if (netlink_pid != 0) {
#line 2218
		ILI_ERR("The channel of Netlink has been established successfully !\n");
		idev->c530 = 1;
	} else {
		ILI_ERR("Failed to establish the channel between kernel and user space\n");
		idev->c530 = 0;
	}
}

/*
 * LE TREDICI `file_operations`, nell'ordine in cui stanno in memoria
 * (0xffffff8009989500, passo 240 = sizeof(struct file_operations)), cioe'
 * nell'ordine del sorgente. Lo slot +0 (`.owner`) non e' relocato in
 * nessuna delle tredici.
 */
static struct file_operations proc_mp_lcm_on_test_fops = {
	.read = ilitek_node_mp_lcm_on_test_read,
};

static struct file_operations proc_mp_lcm_off_test_fops = {
	.read = ilitek_node_mp_lcm_off_test_read,
};

static struct file_operations proc_debug_message_fops = {
	.read = ilitek_proc_debug_message_read,
};

static struct file_operations proc_debug_message_switch_fops = {
	.read = ilitek_proc_debug_switch_read,
};

static struct file_operations proc_ioctl_fops = {
	.unlocked_ioctl = ilitek_node_ioctl,
	.compat_ioctl = ilitek_node_compat_ioctl,
	.write = ilitek_node_ioctl_write,
};

static struct file_operations proc_fw_upgrade_fops = {
	.read = ilitek_node_fw_upgrade_read,
};

static struct file_operations proc_fw_process_fops = {
	.read = ilitek_proc_fw_process_read,
};

static struct file_operations proc_get_delta_data_fops = {
	.read = ilitek_proc_get_delta_data_read,
};

static struct file_operations proc_fw_get_raw_data_fops = {
	.read = ilitek_proc_fw_get_raw_data_read,
};

static struct file_operations proc_rw_tp_reg_fops = {
	.read = ilitek_proc_rw_tp_reg_read,
	.write = ilitek_proc_rw_tp_reg_write,
};

static struct file_operations proc_fw_pc_counter_fops = {
	.read = ilitek_proc_fw_pc_counter_read,
};

static struct file_operations proc_get_debug_mode_data_fops = {
	.read = ilitek_proc_get_debug_mode_data_read,
	.write = ilitek_proc_get_debug_mode_data_write,
};

static struct file_operations proc_debug_level_fops = {
	.read = ilitek_proc_debug_level_read,
};

/*
 * LA TABELLA DEI TREDICI NODI, 13 x 32 byte a 0xffffff800998a130.
 * I nomi dei campi sono `c<scarto>` perche' il binario non li nomina; il
 * commento dice cio' che di ciascuno E' MISURATO al sito d'uso.
 */
struct ilitek_proc_node {
	/* +0  1o argomento di proc_create: il nome del nodo. */
	char *c0;
	/* +8  riceve il risultato di proc_create. */
	struct proc_dir_entry *c8;
	/* +16 4o argomento di proc_create. */
	struct file_operations *c16;
	/* +24 un byte: 1 se proc_create ha ritornato non-NULL, 0 altrimenti. */
	bool c24;
};

static struct ilitek_proc_node proc_table[13] = {
	{"ioctl", NULL, &proc_ioctl_fops, false},
	{"fw_process", NULL, &proc_fw_process_fops, false},
	{"fw_upgrade", NULL, &proc_fw_upgrade_fops, false},
	{"debug_level", NULL, &proc_debug_level_fops, false},
	{"mp_lcm_on_test", NULL, &proc_mp_lcm_on_test_fops, false},
	{"mp_lcm_off_test", NULL, &proc_mp_lcm_off_test_fops, false},
	{"debug_message", NULL, &proc_debug_message_fops, false},
	{"debug_message_switch", NULL, &proc_debug_message_switch_fops, false},
	{"fw_pc_counter", NULL, &proc_fw_pc_counter_fops, false},
	{"show_delta_data", NULL, &proc_get_delta_data_fops, false},
	{"show_raw_data", NULL, &proc_fw_get_raw_data_fops, false},
	{"get_debug_mode_data", NULL, &proc_get_debug_mode_data_fops, false},
	{"rw_tp_reg", NULL, &proc_rw_tp_reg_fops, false},
};

/*
 * netlink_init -- NON esiste come simbolo: la mappa non la contiene. Il
 * binario la denuncia col proprio `__func__` ("netlink_init" a
 * 0xffffff800924a09d) dentro `ilitek_tddi_node_init`, alle righe 2240 e
 * 2243. Va scritta come funzione separata e lasciata innestare.
 * L'unita' netlink e' 21 ("528002a1 mov"@0xffffff8008a717d0); la
 * configurazione e' 48 byte copiati dalla costante a 0xffffff8008f7ed00
 * ("a9402909 ldp"@0xffffff8008a717b8), con una sola rilocazione, a +8.
 */
static void netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = netlink_recv_msg,
	};

	netlink_sk = netlink_kernel_create(&init_net, 21, &cfg);

#line 2240
	ILI_INFO("Initialise Netlink and create its socket\n");

	if (!netlink_sk)
		ILI_ERR("Failed to create nelink socket\n");
}

/*
 * ilitek_tddi_node_init -- 0xffffff8008a716e8, 372 byte, `T` (chiamata da
 * 0xffffff8008a55104, l'area di `probe`).
 * La tabella dei nodi sta a 0xffffff800998a130
 * ("9104c318 add"@0xffffff8008a7173c), ha passo 32
 * ("910082d6 add"@0xffffff8008a7175c) e TREDICI voci: il confronto
 * ("f10602df cmp"@0xffffff8008a717a8) avviene PRIMA dell'incremento, quindi
 * il corpo gira per gli scarti 0x00..0x180 compresi. Confermato dalle
 * rilocazioni: l'ultimo slot relocato e' +0x190, il successivo non lo e'.
 * Il modo dei nodi e' 0644 ("52803481 mov"@0xffffff8008a7176c, 0x1a4).
 */
void ilitek_tddi_node_init(void)
{
	int i;

	proc_dir = proc_mkdir("ilitek", NULL);

	for (i = 0; i < 13; i++) {
		proc_table[i].c8 = proc_create(proc_table[i].c0, 0644,
					       proc_dir, proc_table[i].c16);
		if (proc_table[i].c8 == NULL) {
			proc_table[i].c24 = false;
			ILI_ERR("Failed to create %s under /proc\n",
#line 2261
				proc_table[i].c0);
		} else {
			proc_table[i].c24 = true;
			ILI_INFO("Succeed to create %s under /proc\n",
				 proc_table[i].c0);
		}
	}

	netlink_init();
}

/*
 * ilitek_proc_debug_switch_read -- 0xffffff8008a6c798, 364 byte, `t`.
 * Chiama `ilitek_debug_node_buff_control` FUORI LINEA
 * ("94001419 bl"@0xffffff8008a6c7f8) con l'argomento `!idev->c556`
 * ("1a9f17e0 cset"@0xffffff8008a6c7f4); e' l'unico sito di chiamata non
 * innestato, e la divergenza H5 spiega perche'.
 */
static ssize_t ilitek_proc_debug_switch_read(struct file *filp,
					     char __user *buff, size_t size,
					     loff_t *pos)
{
	int len = 0;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	mutex_lock(&idev->debug_mutex);

	ilitek_debug_node_buff_control(!idev->c556);

	len = snprintf(g_user_buf, 4096, "dnp : %s\n",
		       idev->c556 ? "Enable" : "Disable");

	*pos = len;

	if (copy_to_user(buff, g_user_buf, len))
#line 701
		ILI_ERR("Failed to copy data to user space\n");

	mutex_unlock(&idev->debug_mutex);
	return len;
}

/*
 * ilitek_proc_fw_process_read -- 0xffffff8008a6fb24, 280 byte, `t`.
 * DIFETTO F1: la sorgente di `copy_to_user` NON e' il buffer formattato ma
 * `&idev->c444` ("9106f116 add"@0xffffff8008a6fbb8 calcola idev+444, e
 * "aa1603e1 mov"@0xffffff8008a6fbfc lo passa come sorgente). Riprodotto.
 * La lunghezza e' ESTESA CON ZERI ("2a1503f5 mov"@0xffffff8008a6fbac, cioe'
 * `mov w21,w21`): e' un `u32`, non un `int`.
 */
static ssize_t ilitek_proc_fw_process_read(struct file *filp,
					   char __user *buff, size_t size,
					   loff_t *pos)
{
	u32 len = 0;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	len = snprintf(g_user_buf, 4096, "%02d\n", idev->c444);

#line 1042
	ILI_INFO("update status = %d\n", idev->c444);

	if (copy_to_user(buff, &idev->c444, len))
		ILI_ERR("Failed to copy data to user space\n");

	*pos = len;
	return len;
}

/*
 * ilitek_debug_node_buff_control -- 0xffffff8008a7185c, 452 byte, `t`.
 * L'allocazione grande e' 16384 byte ("321203e0 orr"@0xffffff8008a718d0),
 * i cicli contano fino a 1024 ("7110011f cmp"@0xffffff8008a71970), quindi
 * l'elemento e' di 16 byte; l'allocazione per elemento e' 2048
 * ("321503e2 orr"@0xffffff8008a71934).
 * La liberazione tiene l'INDIRIZZO del puntatore attraverso la `kfree`
 * ("f8408ec0 ldr"@0xffffff8008a71918, pre-indice con riscrittura): scrivere
 * `if (p->campo) { kfree(p->campo); p->campo = NULL; }` fa rileggere il
 * globale dopo la chiamata e costa una istruzione per sito -- misurato,
 * 464 byte contro 456.
 * L'uscita e' SEMPRE -ENOMEM ("12800160 mov"@0xffffff8008a71a0c), anche sul
 * ramo di spegnimento dove non c'e' stato nessun errore: difetto F7,
 * riprodotto.
 * COL COMPILATORE DI FABBRICA misura 452 byte esatti (con r383902
 * misurava 456: rotazione dei cicli di liberazione).
 */
static int ilitek_debug_node_buff_control(bool enable)
{
	int i;

	idev->c556 = enable;
#line 233
	ILI_INFO("Debug buf ctrl = %s\n", enable ? "Enable" : "Disable");

	if (!enable)
		goto out;

	idev->c560 = 0;
	idev->c564 = 0;
	ipio_kfree((void **)&idev->c592);

	idev->c592 = kzalloc(1024 * sizeof(struct ilitek_dbl), GFP_KERNEL);
	if (IS_ERR(idev->c592) || idev->c592 == NULL) {
		ILI_ERR("Failed to allocate idev->dbl mem, %ld\n",
#line 241
			PTR_ERR(idev->c592));
		goto out;
	}

	for (i = 0; i < 1024; i++) {
		IDEV_DBL[i].c0 = 0;
		ipio_kfree((void **)&IDEV_DBL[i].c8);
		IDEV_DBL[i].c8 = kzalloc(2048, GFP_KERNEL);
		if (IS_ERR(IDEV_DBL[i].c8) || IDEV_DBL[i].c8 == NULL) {
			ILI_ERR("Failed to allocate dbl[%d] mem, %ld\n", i,
				PTR_ERR(IDEV_DBL[i].c8));
			goto out;
		}
	}
	return 0;

out:
	for (i = 0; i < 1024; i++) {
		IDEV_DBL[i].c0 = 0;
		ipio_kfree((void **)&IDEV_DBL[i].c8);
	}
	ipio_kfree((void **)&idev->c592);
	return -ENOMEM;
}

/*
 * ilitek_proc_fw_pc_counter_read -- 0xffffff8008a70cd0, 308 byte, `t`.
 * I due valori stampati sono letti INSIEME
 * ("295c9103 ldp"@0xffffff8008a70d28): sono i campi a +228 e +232 di `idev`,
 * adiacenti e a 32 bit. `ilitek_tddi_ic_get_pc_counter` e' chiamata senza
 * preparare nessun registro d'argomento.
 */
static ssize_t ilitek_proc_fw_pc_counter_read(struct file *filp,
					      char __user *buff, size_t size,
					      loff_t *pos)
{
	int len = 0;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	ilitek_tddi_ic_get_pc_counter();

	len = snprintf(g_user_buf, 4096, "pc = 0x%x, latch = 0x%x\n",
		       idev->c228, idev->c232);

	if (copy_to_user(buff, g_user_buf, len))
#line 565
		ILI_ERR("Failed to copy data to user space\n");

	*pos += len;
	return len;
}

/*
 * ilitek_proc_debug_level_read -- 0xffffff8008a713f4, 376 byte, `t`.
 * Il diagnostico e' RIBALTATO con uno `eor #1`
 * ("52000104 eor"@0xffffff8008a7145c) e il valore nuovo e' anche il quarto
 * argomento del messaggio; la stringa scelta usa invece il valore VECCHIO
 * ("9a970303 csel"@0xffffff8008a71460), che dopo il ribaltamento equivale.
 * `*pos` e' INCREMENTATO, non assegnato ("8b130108 add"@0xffffff8008a714a4).
 */
static ssize_t ilitek_proc_debug_level_read(struct file *filp,
					    char __user *buff, size_t size,
					    loff_t *pos)
{
	int len = 0;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	/*
	 * `^= 1` e NON `= !ilitek_dbg_en`: la fabbrica ribalta il byte con
	 * uno EOR ("52000104 eor"@0xffffff8008a7145c), e su un `u8` la forma
	 * `!x` non lo produce -- clang non puo' provare che il valore stia
	 * in {0,1} e abbassa `!x` a `cmp`+`cset`. MISURATO sul `.o` vero: la
	 * dimensione non cambia (376 byte con entrambe le forme), cambia
	 * l'istogramma dei mnemonici (`cset:+1 eor:-1`). E' la stessa forma
	 * gia' usata sullo stesso globale in `ilitek_node_ioctl`.
	 */
	ilitek_dbg_en ^= 1;

	ILI_INFO(" %s debug level = %x\n",
#line 1128
		 ilitek_dbg_en ? "Enable" : "Disable", ilitek_dbg_en);

	len = snprintf(g_user_buf, 4096, "debug level : %s\n",
		       ilitek_dbg_en ? "Enable" : "Disable");

	*pos += len;

	if (copy_to_user(buff, g_user_buf, len))
#line 1135
		ILI_ERR("Failed to copy data to user space\n");

	return len;
}

/*
 * file_write -- 0xffffff8008a71a20, 404 byte, `t`.
 * Il controllo sul nome e' una PROVA DI PUNTATORE su un ARRAY
 * ("b1002015 adds"@0xffffff8008a71a4c somma 8 alla base e prova il
 * risultato): il campo a +8 e' un array, non un puntatore, e il confronto
 * con NULL non puo' mai essere vero. Riprodotto com'e'.
 * Le due maschere di `filp_open` sono 0x441 = O_WRONLY|O_CREAT|O_APPEND
 * ("52808828 mov"@0xffffff8008a71a68) e 0x241 = O_WRONLY|O_CREAT|O_TRUNC
 * ("52804829 mov"@0xffffff8008a71a6c); il modo e' 644 DECIMALE
 * ("52805082 mov"@0xffffff8008a71a74, 0x284) e non 0644 ottale -- difetto
 * F5, riprodotto.
 * La lunghezza scritta e' letta CON SEGNO ("b9808a62 ldrsw"@0xffffff8008a71ac0):
 * il campo a +136 e' un `int`.
 */
static void file_write(struct ilitek_file_buffer *file, bool new_open)
{
	struct file *f = NULL;
	mm_segment_t fs;
	loff_t pos;

	if (file->c0 == NULL) {
#line 193
		ILI_ERR("str is invaild\n");
		return;
	}

	if (file->c8 == NULL) {
		ILI_ERR("file name is invaild\n");
		return;
	}

	if (file->c136 >= file->c144) {
		ILI_ERR("Saved to file length is too long !, %d\n", file->c136);
		return;
	}

	if (new_open)
		f = filp_open(file->c8, O_WRONLY | O_CREAT | O_TRUNC, 644);
	else
		f = filp_open(file->c8, O_WRONLY | O_CREAT | O_APPEND, 644);

	if (IS_ERR(f) || f == NULL) {
		ILI_ERR("Failed to open %s file\n", file->c8);
		return;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_write(f, file->c0, file->c136, &pos);
	set_fs(fs);
	filp_close(f, NULL);

	file->c140 += file->c136;
}

/*
 * ilitek_proc_get_debug_mode_data_read -- 0xffffff8008a70e04, 700 byte, `t`.
 * Il descrittore di scarico e' inizializzato PEZZO PER PEZZO e non con un
 * inizializzatore d'aggregato: la fabbrica azzera i 105 byte di coda del
 * nome con tredici `stur xzr` disallineati
 * ("f807f3ff stur"@0xffffff8008a70e4c e seguenti) e copia i 23 byte del
 * percorso separatamente ("a900a3ea stp"@0xffffff8008a70e84). E' la forma
 * `memset(campo, 0, sizeof(campo)); strcpy(campo, "...")`. Con un
 * inizializzatore d'aggregato clang emette una `memcpy` di 152 byte da una
 * costante e la funzione misura 632 byte invece di 700 -- misurato.
 * Il tetto del file e' 10240 ("52850009 mov"@0xffffff8008a70e48).
 * Il ritorno e' SEMPRE zero ("aa1f03e0 mov"@0xffffff8008a710b0).
 */
static ssize_t ilitek_proc_get_debug_mode_data_read(struct file *filp,
						    char __user *buff,
						    size_t size, loff_t *pos)
{
	struct ilitek_file_buffer csv;

	if (*pos != 0)
		return 0;

	memset(csv.c8, 0, sizeof(csv.c8));
	strcpy(csv.c8, "/sdcard/ILITEK_log.csv");
	csv.c136 = 0;
	csv.c140 = 0;
	csv.c144 = 10240;

	csv.c0 = vmalloc(csv.c144);
	if (IS_ERR(csv.c0) || csv.c0 == NULL) {
#line 827
		ILI_ERR("Failed to allocate CSV mem\n");
		goto out;
	}

	ILI_INFO("Get Raw data %d frame\n", idev->c600);
	ILI_INFO("Get Delta data %d frame\n", idev->c604);

	csv.c136 += snprintf(csv.c0 + csv.c136, csv.c144 - csv.c136,
			     "Get Raw data %d frame\n", idev->c600);
	csv.c136 += snprintf(csv.c0 + csv.c136, csv.c144 - csv.c136,
			     "Get Delta data %d frame\n", idev->c604);
	file_write(&csv, true);

	if (ilitek_set_tp_data_len(1) < 0) {
		ILI_ERR("Failed to set tp data length\n");
		goto out;
	}

	csv.c136 = 0;
	memset(csv.c0, 0, csv.c144);
	csv.c136 += snprintf(csv.c0 + csv.c136, csv.c144 - csv.c136,
			     "\n\n=======Raw data=======");
	file_write(&csv, false);
	if (debug_mode_get_data(&csv, 8, idev->c600) < 0)
		goto out;

	csv.c136 = 0;
	memset(csv.c0, 0, csv.c144);
	csv.c136 += snprintf(csv.c0 + csv.c136, csv.c144 - csv.c136,
			     "\n\n=======Delta data=======");
	file_write(&csv, false);
	if (debug_mode_get_data(&csv, 3, idev->c604) < 0)
		goto out;

	if (ilitek_set_tp_data_len(0) < 0)
#line 867
		ILI_ERR("Failed to set tp data length\n");

out:
	if (csv.c0 != NULL) {
		vfree(csv.c0);
		csv.c0 = NULL;
	}
	return 0;
}

/*
 * `dev_mkdir` NON esiste come simbolo: la mappa non la contiene. Il binario
 * la denuncia col proprio `__func__` -- "dev_mkdir"@0xffffff8009247c83 con
 * `__LINE__` = 347 -- dentro le DUE funzioni qui sotto
 * ("91320c21 add"@0xffffff8008a6bc50 e "91320c21 add"@0xffffff8008a6bf74).
 * Va quindi scritta come funzione separata e lasciata incorporare.
 */
static int dev_mkdir(char *name, umode_t mode)
{
	int err;
	mm_segment_t fs;

#line 347
	ILI_INFO("mkdir: %s\n", name);

	fs = get_fs();
	set_fs(KERNEL_DS);
	err = sys_mkdir(name, mode);
	set_fs(fs);

	return err;
}

#define MP_TEST_TAIL(BUF, LEN)						\
	switch ((BUF)[1]) {						\
	case 0x65:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"Protocol version isn't matched, abort!"); \
		break;							\
	case 0x67:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"Not found ini file, abort!");		\
		break;							\
	case 0x68:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"Failed to get timing info, abort!");	\
		break;							\
	case 0x6b:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"Failed to allocated memory, abort!");	\
		break;							\
	case 0x6f:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"Failed to switch MP mode, abort!");	\
		break;							\
	case 0x70:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"FW still upgrading, abort!");		\
		break;							\
	case 0x71:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"MP formula is null, abort!");		\
		break;							\
	default:							\
		break;							\
	}

/*
 * ilitek_node_mp_lcm_on_test_read -- 0xffffff8008a6bbd4, 804 byte, `t`, e la
 * gemella `..._off_...` -- 0xffffff8008a6bef8, 804 byte.
 * `dev_mkdir` e' innestata qui (vedi il suo commento).
 * Il codice d'errore scritto nel secondo byte del buffer e' il VALORE
 * ASSOLUTO del ritorno (un `cneg`, codifica 5a80541c, indirizzo
 * 0xffffff8008a6bd40), preceduto da "7100001f cmp"@0xffffff8008a6bd30, e la
 * coda del
 * messaggio e' uno `switch` su sette codici abbassato a una tabella di
 * PUNTATORI a 0xffffff8008f7ed30, protetta da una maschera di validita'
 * costante ("528389a9 mov"@0xffffff8008a6bdac carica 0x1c4d, cioe' gli
 * indici 0,2,3,6,10,11,12 dell'intervallo 0x65..0x71).
 * Le due code di lavoro sono spente e riaccese solo se erano accese, e i due
 * flag sono letti PRIMA di tutto ("3948411a ldrb"@0xffffff8008a6bc14 e
 * "39484519 ldrb"@0xffffff8008a6bc18).
 */
static ssize_t ilitek_node_mp_lcm_on_test_read(struct file *filp,
					       char __user *buff, size_t size,
					       loff_t *pos)
{
	int ret, len = 0;
	u8 esd = idev->c528, bat = idev->c529;

	if (*pos != 0)
		return 0;

#line 916
	ILI_INFO("Run MP test with LCM on\n");

	mutex_lock(&idev->touch_mutex);

	if (dev_mkdir("/sdcard/ilitek_mp_lcm_on_log", 0644) != 0)
#line 922
		ILI_ERR("Failed to create directory for mp_test\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 0);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 0);

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	idev->c624 = 0;
	ret = ilitek_tddi_mp_test_handler(g_user_buf, true);

	ILI_INFO("MP TEST %s, Error code = %d\n",
#line 933
		 (ret < 0) ? "FAIL" : "PASS", ret);

	g_user_buf[0] = 3;
	g_user_buf[1] = (ret < 0) ? -ret : ret;

	len = idev->c624 + 2;
	len += snprintf(g_user_buf + len, 4096 - len, "MP TEST %s\n",
			(ret < 0) ? "FAIL" : "PASS");

	MP_TEST_TAIL(g_user_buf, len)

	if (copy_to_user(buff, g_user_buf, len))
#line 957
		ILI_ERR("Failed to copy data to user space\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 1);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 1);

	*pos += len;

	mutex_unlock(&idev->touch_mutex);
	return len;
}

static ssize_t ilitek_node_mp_lcm_off_test_read(struct file *filp,
						char __user *buff, size_t size,
						loff_t *pos)
{
	int ret, len = 0;
	u8 esd = idev->c528, bat = idev->c529;

	if (*pos != 0)
		return 0;

#line 977
	ILI_INFO("Run MP test with LCM off\n");

	mutex_lock(&idev->touch_mutex);

	if (dev_mkdir("/sdcard/ilitek_mp_lcm_off_log", 0644) != 0)
#line 984
		ILI_ERR("Failed to create directory for mp_test\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 0);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 0);

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	idev->c624 = 0;
	ret = ilitek_tddi_mp_test_handler(g_user_buf, false);

	ILI_INFO("MP TEST %s, Error code = %d\n",
#line 995
		 (ret < 0) ? "FAIL" : "PASS", ret);

	g_user_buf[0] = 3;
	g_user_buf[1] = (ret < 0) ? -ret : ret;

	len = idev->c624 + 2;
	len += snprintf(g_user_buf + len, 4096 - len, "MP TEST %s\n",
			(ret < 0) ? "FAIL" : "PASS");

	MP_TEST_TAIL(g_user_buf, len)

	if (copy_to_user(buff, g_user_buf, len))
#line 1019
		ILI_ERR("Failed to copy data to user space\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 1);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 1);

	*pos += len;

	mutex_unlock(&idev->touch_mutex);
	return len;
}

/*
 * ilitek_node_fw_upgrade_read -- 0xffffff8008a6f868, 700 byte, `t`.
 * DELTA DI INTESTAZIONE: `ilitek_tddi_fw_upgrade_handler` riceve UN
 * argomento, NULL ("aa1f03e0 mov"@0xffffff8008a6f90c), mentre `ilitek.h` la
 * dichiara `(void)`.
 * La coda del messaggio e' uno `switch` su otto codici abbassato a una
 * TABELLA DI SALTO A 8 BIT di sette voci a 0xffffff8008f7ecf2
 * ("3868692b ldrb"@0xffffff8008a6f984 la indicizza con `codice - 0x72`,
 * "d61f0140 br"@0xffffff8008a6f98c salta); l'ottavo codice (0x0c) sta fuori
 * dall'intervallo ed e' provato a parte
 * ("7100313f cmp"@0xffffff8008a6f9a8).
 * Il limite del primo `snprintf` e' 4094 ("321f2be1 orr"@0xffffff8008a6f94c,
 * 0xffe): e' `sizeof(g_user_buf) - len` con len=2 ripiegato. Scrivendo
 * `4096 - len` (aritmetica a 32 bit) la sottrazione resta a 32 bit e serve
 * una `sxtw` in piu' -- misurato, 704 byte contro 700.
 */
static ssize_t ilitek_node_fw_upgrade_read(struct file *filp,
					   char __user *buff, size_t size,
					   loff_t *pos)
{
	int ret, len = 0;
	u8 esd = idev->c528, bat = idev->c529;

	if (*pos != 0)
		return 0;

#line 1059
	ILI_INFO("Preparing to upgarde firmware\n");

	mutex_lock(&idev->touch_mutex);

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	if (esd)
		ilitek_tddi_wq_ctrl(0, 0);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 0);

	idev->c631 = 1;
	idev->c643 = 1;
	/*
	 * DELTA DI INTESTAZIONE: `ilitek.h` dichiara
	 * `int ilitek_tddi_fw_upgrade_handler(void)`, ma questo sito prepara
	 * x0 = NULL ("aa1f03e0 mov"@0xffffff8008a6f90c) subito prima della
	 * chiamata ("97ff9058 bl"@0xffffff8008a6f918): la funzione prende UN
	 * argomento, un puntatore, e qui vale NULL. Il cast riproduce il
	 * binario senza toccare l'intestazione condivisa.
	 */
	ret = ((int (*)(void *))ilitek_tddi_fw_upgrade_handler)(NULL);
	idev->c643 = 0;
	idev->c631 = 0;

	g_user_buf[0] = 0;
	g_user_buf[1] = (ret < 0) ? -ret : ret;

	len = 2;
	len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
			(g_user_buf[1] == 0) ? "Upgrade firmware = PASS" :
					       "Upgrade firmware = FAIL");

	switch (g_user_buf[1]) {
	case 0x0c:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to allocate memory, abort!");
		break;
	case 0x72:
		g_user_buf[0] = 0xff;
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to convert hex/ili file, abort!");
		break;
	case 0x73:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to operate ice mode, abort!");
		break;
	case 0x74:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to operate watch dog, abort!");
		break;
	case 0x75:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"CRC not matched, abort!");
		break;
	case 0x76:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to do reset, abort!");
		break;
	case 0x77:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to erase flash, abort!");
		break;
	case 0x78:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to program flash, abort!");
		break;
	default:
		break;
	}

	if (copy_to_user(buff, g_user_buf, len))
#line 1107
		ILI_ERR("Failed to copy data to user space\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 1);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 1);

	*pos += len;

	mutex_unlock(&idev->touch_mutex);
	return len;
}

/*
 * ilitek_proc_rw_tp_reg_write -- 0xffffff8008a70a54, 636 byte, `t`.
 * L'indice della scrittura in `rw_reg` e' esteso con ZERI
 * ("b8345b24 str"@0xffffff8008a70c34, `[x25, w20, uxtw #2]`): il contatore e'
 * UNSIGNED, non `int` (classe A3). Nessun confronto lo limita: difetto F4.
 * Il tetto sulla lunghezza d'ingresso e' 256, provato su `size - 1`
 * ("f10406bf cmp"@0xffffff8008a70a7c confronta con 0x101).
 * `str2hex` e' innestata: "7101819f cmp"@0xffffff8008a70c04.
 * COL COMPILATORE DI FABBRICA misura 636 byte esatti (con r383902
 * misurava 632).
 */
static ssize_t ilitek_proc_rw_tp_reg_write(struct file *filp,
					   const char __user *buff,
					   size_t size, loff_t *pos)
{
	char buf[256] = {0};
	char *token = NULL, *cur = NULL;
	u32 i = 0;
	ssize_t ret = size;

	if (size - 1 > 256) {
#line 657
		ILI_ERR("ERROR! input length is larger than local buffer\n");
		return -1;
	}

	mutex_lock(&idev->touch_mutex);

	if (buff != NULL) {
		if (copy_from_user(buf, buff, size - 1)) {
			ILI_INFO("Failed to copy data from user space\n");
			ret = -1;
			goto out;
		}
	}

	cur = buf;
	token = strsep(&cur, ",");
	while (token != NULL) {
		rw_reg[i] = str2hex(token);
#line 673
		ILI_INFO("rw_reg[%d] = 0x%x\n", i, rw_reg[i]);
		i++;
		token = strsep(&cur, ",");
	}

out:
	mutex_unlock(&idev->touch_mutex);
	return ret;
}

/*
 * ilitek_proc_rw_tp_reg_read -- 0xffffff8008a70640, 1044 byte, `t`.
 * QUI E' MISURATO CHE `rw_reg` HA CINQUE ELEMENTI: la funzione li legge
 * tutti, a coppie ("29405915 ldp"@0xffffff8008a7068c e
 * "29416117 ldp"@0xffffff8008a70690) piu' il quinto da solo
 * ("b9401119 ldr"@0xffffff8008a7069c). I nomi dei cinque vengono dai formati
 * ("stop_mcu = %d", "[WRITE]:addr = ..., write = ..., len = %d byte",
 * "[READ]:addr = ..., read = ...").
 * Il secondo argomento di `ilitek_ice_mode_ctrl` e' la NEGAZIONE di
 * rw_reg[0] ("1a9f17e1 cset"@0xffffff8008a70708, `cset w1, eq`).
 */
static ssize_t ilitek_proc_rw_tp_reg_read(struct file *filp, char __user *buff,
					  size_t size, loff_t *pos)
{
	int ret, len = 0;
	u32 type, stop_mcu, addr, write_data, write_len, read_data;
	u8 esd = idev->c528, bat = idev->c529;

	if (*pos != 0)
		return 0;

	stop_mcu = rw_reg[0];
	type = rw_reg[1];
	addr = rw_reg[2];
	write_data = rw_reg[3];
	write_len = rw_reg[4];

#line 588
	ILI_INFO("stop_mcu = %d\n", stop_mcu);

	if (esd)
		ilitek_tddi_wq_ctrl(0, 0);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 0);

	mutex_lock(&idev->touch_mutex);

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	ret = ilitek_ice_mode_ctrl(1, !stop_mcu);
	if (ret < 0) {
		ILI_ERR("Failed to enter ICE mode, ret = %d\n", ret);
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to enter ICE mode");
	}

	if (type) {
		ret = ilitek_ice_mode_write(addr, write_data, write_len);
		if (ret < 0) {
			ILI_ERR("Write data error\n");
			len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
					"Write data error");
		}
		ILI_INFO("[WRITE]:addr = 0x%06x, write = 0x%08x, len = %d byte\n",
#line 623
			 addr, write_data, write_len);
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
				"WRITE:addr = 0x%06x, write = 0x%08x, len =%d byte\n",
				addr, write_data, write_len);
	} else {
		ret = ilitek_ice_mode_read(addr, &read_data, sizeof(u32));
		if (ret < 0) {
			ILI_ERR("Read data error\n");
			len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
					"Read data error");
		}
		ILI_INFO("[READ]:addr = 0x%06x, read = 0x%08x\n", addr,
#line 615
			 read_data);
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
				"READ:addr = 0x%06x, read = 0x%08x\n", addr,
				read_data);
	}

	ret = ilitek_ice_mode_ctrl(0, !stop_mcu);
	if (ret < 0) {
		ILI_ERR("Failed to disable ICE mode, ret = %d\n", ret);
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to disable ICE mode");
	}

	if (copy_to_user(buff, g_user_buf, len))
#line 638
		ILI_ERR("Failed to copy data to user space\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 1);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 1);

	*pos += len;

	mutex_unlock(&idev->touch_mutex);
	return len;
}

/*
 * debug_mode_get_data -- 0xffffff8008a71bb4, 1388 byte, `t`.
 * DELTA DI INTESTAZIONE: la coda d'attesa sta a +568 di `idev`
 * ("9108e100 add"@0xffffff8008a71d64 la passa a `prepare_to_wait_event` con
 * stato 1, TASK_INTERRUPTIBLE), e `ilitek.h` non la dichiara.
 * L'attesa scade dopo 750 tick ("52805dd7 mov"@0xffffff8008a71d44), cioe'
 * `msecs_to_jiffies(3000)` a HZ=250.
 * Il primo ciclo di stampa divide l'indice per il numero di colonne SENZA
 * SEGNO ("1ad90b83 udiv"@0xffffff8008a71e68).
 * `ilitek_debug_node_buff_control` e' innestata DUE volte, con l'argomento
 * costante `false`: i due `__func__` a 0xffffff8008a71c04 e
 * 0xffffff8008a72040 lo denunciano.
 * COL COMPILATORE DI FABBRICA misura 1388 byte esatti (con r383902
 * misurava 1416: rotazione dei cicli di liberazione innestati).
 */
static int debug_mode_get_data(struct ilitek_file_buffer *csv, u8 type,
			       u32 frame_count)
{
	int ret = 0, j;
	s16 temp;
	u16 i;
	u8 cmd[2] = {0};
	u8 row, col;
	u8 *data;

	ilitek_debug_node_buff_control(false);

	row = idev->c249;
	col = idev->c248;
	idev->c560 = 0;

	mutex_lock(&idev->touch_mutex);

	cmd[0] = 0xFA;
	cmd[1] = type;
	ret = idev->c776(cmd, 2);

	if (ilitek_debug_node_buff_control(true) < 0) {
#line 289
		ILI_ERR("Failed to allocate debug buf\n");
		ret = -ENOMEM;
		goto out;
	}

	mutex_unlock(&idev->touch_mutex);

	if (ret < 0) {
		ILI_ERR("Write 0xFA,0x%x failed\n", type);
		return ret;
	}

	for (i = 0; i < frame_count; i++) {
		csv->c136 = 0;

		ILI_INFO("frame = %d,index = %d,count = %d\n", i, idev->c564,
#line 303
			 idev->c560);

		if (!wait_event_interruptible_timeout(IDEV_INQ,
						      IDEV_DBL[idev->c564].c0,
						      msecs_to_jiffies(3000))) {
#line 305
			ILI_ERR("debug mode get data timeout!\n");
			goto out;
		}

		mutex_lock(&idev->touch_mutex);

		memset(csv->c0, 0, csv->c144);

		csv->c136 += snprintf(csv->c0 + csv->c136,
				      csv->c144 - csv->c136, "\n\nFrame%d,", i);

		for (j = 0; j < col; j++)
			csv->c136 += snprintf(csv->c0 + csv->c136,
					      csv->c144 - csv->c136,
					      "[X%d] ,", j);

		data = (u8 *)IDEV_DBL[idev->c564].c8 + 35;

		for (j = 0; j < col * row; j++) {
			temp = (data[0] << 8) | data[1];
			if (j % col == 0)
				csv->c136 += snprintf(csv->c0 + csv->c136,
						      csv->c144 - csv->c136,
						      "\n[Y%d] ,", j / col);
			csv->c136 += snprintf(csv->c0 + csv->c136,
					      csv->c144 - csv->c136, "%d, ",
					      temp);
			data += 2;
		}

		csv->c136 += snprintf(csv->c0 + csv->c136,
				      csv->c144 - csv->c136, "\n[X] ,");

		for (j = 0; j < col + row; j++) {
			temp = (data[0] << 8) | data[1];
			if (j == col)
				csv->c136 += snprintf(csv->c0 + csv->c136,
						      csv->c144 - csv->c136,
						      "\n[Y] ,");
			csv->c136 += snprintf(csv->c0 + csv->c136,
					      csv->c144 - csv->c136, "%d, ",
					      temp);
			data += 2;
		}

		file_write(csv, false);

		mutex_unlock(&idev->touch_mutex);

		IDEV_DBL[idev->c564].c0 = 0;
		idev->c564 = (idev->c564 + 1) % 1024;
	}

out:
	ilitek_debug_node_buff_control(false);
	return ret;
}

/*
 * ilitek_proc_get_debug_mode_data_write -- 0xffffff8008a710c0, 820 byte, `t`.
 * L'indice dell'array temporaneo e' troncato a un byte
 * ("92401ee9 and"@0xffffff8008a712dc e "38296b08 strb"@0xffffff8008a712f4):
 * il contatore e' un `u8`, e il valore scritto e' un byte.
 * I tre campi ricomposti sono coppie big-endian di byte consecutivi e
 * finiscono a +600, +604 e +608 di `idev`
 * ("b90259c3 str"@0xffffff8008a71378 e le due seguenti).
 * `str2hex` e' innestata: "710181bf cmp"@0xffffff8008a712bc.
 */
static ssize_t ilitek_proc_get_debug_mode_data_write(struct file *filp,
						     const char __user *buff,
						     size_t size, loff_t *pos)
{
	u8 temp[256] = {0}, i = 0;
	char buf[256] = {0};
	char *token = NULL, *cur = NULL;
	ssize_t ret = size;

	if (size - 1 > 256) {
#line 881
		ILI_ERR("ERROR! input length is larger than local buffer\n");
		return -1;
	}

	if (buff != NULL) {
		if (copy_from_user(buf, buff, size - 1)) {
			ILI_INFO("Failed to copy data from user space\n");
			return -1;
		}
	}

	ILI_INFO("size = %d, cmd = %s\n", (int)size, buf);

	cur = buf;
	token = strsep(&cur, ",");
	while (token != NULL) {
		temp[i] = str2hex(token);
#line 896
		ILI_INFO("temp[%d] = %d\n", i, temp[i]);
		i++;
		token = strsep(&cur, ",");
	}

	idev->c600 = (temp[0] << 8) | temp[1];
	idev->c604 = (temp[2] << 8) | temp[3];
	idev->c608 = (temp[4] << 8) | temp[5];

	ILI_INFO("Raw_count = %d, Delta_count = %d, BG_count = %d\n",
#line 904
		 idev->c600, idev->c604, idev->c608);

	return ret;
}

/*
 * ilitek_proc_get_delta_data_read -- 0xffffff8008a6fc3c, 1272 byte, `t`, e la
 * gemella `ilitek_proc_fw_get_raw_data_read` -- 0xffffff8008a70134,
 * 1292 byte. Le due differiscono solo per le stringhe, i `__LINE__` e il
 * secondo byte del comando (0x1 contro 0x2).
 * La lunghezza da leggere e' `4 + righe*colonne*2 + 1`: il prodotto e'
 * calcolato a 64 bit senza segno ("9bb67d28 umull"@0xffffff8008a6fce8), il
 * `+5` e' "11001675 add"@0xffffff8008a6fcf0 e l'allocazione chiede uno in
 * piu' ("91001a60 add"@0xffffff8008a6fd00, cioe' `+6`).
 * Il secondo buffer e' 5120 byte ("52828002 mov"@0xffffff8008a6fd2c).
 * Il riversamento e' una lettura BIG-ENDIAN a 16 bit
 * ("5ac0096b rev"@0xffffff8008a6fdcc piu' "53107d6b lsr"@0xffffff8008a6fdd0)
 * e gira `righe*colonne*2` volte leggendo due byte per giro
 * ("f1000508 subs"@0xffffff8008a6fdc4): difetto F8, riprodotto.
 * DIFETTO F3: i due rami di allocazione fallita escono con 0
 * ("aa1f03e0 mov"@0xffffff8008a70030) saltando `mutex_unlock`, la riaccensione
 * delle code e ogni `kfree`. Riprodotto.
 */
static ssize_t ilitek_proc_get_delta_data_read(struct file *filp,
					       char __user *buff, size_t size,
					       loff_t *pos)
{
	s16 *delta = NULL;
	int row = 0, col = 0;
	int ret, i, j, read_length = 0, len = 0;
	u8 cmd[2] = {0};
	u8 *data = NULL;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	ilitek_tddi_wq_ctrl(0, 0);
	ilitek_tddi_wq_ctrl(1, 0);

	mutex_lock(&idev->touch_mutex);

	row = idev->c249;
	col = idev->c248;
	read_length = 4 + row * col * 2 + 1;

#line 378
	ILI_INFO("read length = %d\n", read_length);

	data = kzalloc(read_length + 1, GFP_KERNEL);
	if (IS_ERR(data) || data == NULL) {
		ILI_ERR("Failed to allocate data mem\n");
		return 0;
	}

	delta = kzalloc(5120, GFP_KERNEL);
	if (IS_ERR(delta) || delta == NULL) {
		ILI_ERR("Failed to allocate delta mem\n");
		return 0;
	}

	cmd[0] = 0xB7;
	cmd[1] = 0x1;
	ret = idev->c776(cmd, 2);
	if (ret < 0) {
		ILI_ERR("Failed to write 0xB7,0x1 command, %d\n", ret);
		goto out;
	}

	msleep(120);

	ret = idev->c784(data, read_length);
	if (ret < 0) {
#line 405
		ILI_ERR("Read debug packet header failed, %d\n", ret);
		goto out;
	}

	cmd[1] = 0x3;
	ret = idev->c776(cmd, 2);
	if (ret < 0) {
		ILI_ERR("Failed to write 0xB7,0x3 command, %d\n", ret);
		goto out;
	}

	for (i = 0; i < row * col * 2; i++)
		delta[i] = (data[i * 2 + 4] << 8) + data[i * 2 + 5];

	len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
			"======== Deltadata ========\n");
	ILI_INFO("======== Deltadata ========\n");
	len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
			"Header 0x%x ,Type %d, Length %d\n", data[0], data[1],
			(data[2] << 8) | data[3]);
	ILI_INFO("Header 0x%x ,Type %d, Length %d\n", data[0], data[1],
		 (data[2] << 8) | data[3]);

	for (i = 0; i < row; i++) {
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "[%2d] ", i + 1);
		ILI_INFO("[%2d] ", i + 1);
		for (j = 0; j < col; j++) {
			len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%5d",
					delta[i * col + j]);
			pr_cont("%5d", delta[i * col + j]);
		}
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "\n");
		pr_cont("\n");
	}

	if (copy_to_user(buff, g_user_buf, len))
		ILI_ERR("Failed to copy data to user space\n");

	*pos += len;

out:
	mutex_unlock(&idev->touch_mutex);
	ilitek_tddi_wq_ctrl(0, 1);
	ilitek_tddi_wq_ctrl(1, 1);
	kfree(data);
	kfree(delta);
	return len;
}
static ssize_t ilitek_proc_fw_get_raw_data_read(struct file *filp,
						char __user *buff, size_t size,
						loff_t *pos)
{
	s16 *rawdata = NULL;
	int row = 0, col = 0;
	int ret, i, j, read_length = 0, len = 0;
	u8 cmd[2] = {0};
	u8 *data = NULL;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	ilitek_tddi_wq_ctrl(0, 0);
	ilitek_tddi_wq_ctrl(1, 0);

	mutex_lock(&idev->touch_mutex);

	row = idev->c249;
	col = idev->c248;
	read_length = 4 + row * col * 2 + 1;

#line 476
	ILI_INFO("read length = %d\n", read_length);

	data = kzalloc(read_length + 1, GFP_KERNEL);
	if (IS_ERR(data) || data == NULL) {
		ILI_ERR("Failed to allocate data mem\n");
		goto out;
	}

	rawdata = kzalloc(5120, GFP_KERNEL);
	if (IS_ERR(rawdata) || rawdata == NULL) {
		ILI_ERR("Failed to allocate rawdata mem\n");
		goto out;
	}

	cmd[0] = 0xB7;
	cmd[1] = 0x2;
	ret = idev->c776(cmd, 2);
	if (ret < 0) {
		ILI_ERR("Failed to write 0xB7,0x2 command, %d\n", ret);
		goto out;
	}

	msleep(120);

	ret = idev->c784(data, read_length);
	if (ret < 0) {
#line 503
		ILI_ERR("Read debug packet header failed, %d\n", ret);
		goto out;
	}

	cmd[1] = 0x3;
	ret = idev->c776(cmd, 2);
	if (ret < 0) {
		ILI_ERR("Failed to write 0xB7,0x3 command, %d\n", ret);
		goto out;
	}

	for (i = 0; i < row * col * 2; i++)
		rawdata[i] = (data[i * 2 + 4] << 8) + data[i * 2 + 5];

	len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
			"======== RawData ========\n");
	ILI_INFO("======== RawData ========\n");
	len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
			"Header 0x%x ,Type %d, Length %d\n", data[0], data[1],
			(data[2] << 8) | data[3]);
	ILI_INFO("Header 0x%x ,Type %d, Length %d\n", data[0], data[1],
		 (data[2] << 8) | data[3]);

	for (i = 0; i < row; i++) {
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "[%2d] ", i + 1);
		ILI_INFO("[%2d] ", i + 1);
		for (j = 0; j < col; j++) {
			len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%5d",
					rawdata[i * col + j]);
			pr_cont("%5d", rawdata[i * col + j]);
		}
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "\n");
		pr_cont("\n");
	}

	if (copy_to_user(buff, g_user_buf, len))
		ILI_ERR("Failed to copy data to user space\n");

	*pos += len;

out:
	mutex_unlock(&idev->touch_mutex);
	ilitek_tddi_wq_ctrl(0, 1);
	ilitek_tddi_wq_ctrl(1, 1);
	if (data)
		kfree(data);
	if (rawdata)
		kfree(rawdata);
	return len;
}

/*
 * ilitek_proc_debug_message_read -- 0xffffff8008a6c21c, 1404 byte, `t`.
 * Il primo controllo e' su O_NONBLOCK: il byte a +65 di `struct file` e' il
 * secondo di `f_flags` e il bit provato e' il 3
 * ("39410408 ldrb"@0xffffff8008a6c26c e "371806c8 tbnz"@0xffffff8008a6c270),
 * cioe' il bit 11 = 0x800; l'uscita e' -EAGAIN
 * ("92800140 mov"@0xffffff8008a6c348).
 * La lunghezza attesa dipende dal primo byte del pacchetto (0x5A -> 43,
 * 0xA7 -> 2040, 0x7A -> un calcolo) e, per 0x7A, dai quattro bit bassi del
 * quarto byte: la fabbrica prova l'appartenenza a {0,1,6} con una maschera
 * ("5280086c mov"@0xffffff8008a6c3e4 carica 0x43,
 * "6a0c017f tst"@0xffffff8008a6c3e8 la prova) e fa confluire i tre rami in
 * UNA sola moltiplicazione-somma ("9b09291c madd"@0xffffff8008a6c3f4): e' la
 * prova che i tre rami confluiscono. Vedi la divergenza H1.
 * La coda "\n\n" e' scritta come due byte piu' il terminatore
 * ("52814149 mov"@0xffffff8008a6c578 carica 0x0a0a).
 * DIFETTO F6: `vfree` e' chiamata anche su un ERR_PTR, perche' la guardia
 * prova solo il NULL ("b4000075 cbz"@0xffffff8008a6c728). Riprodotto.
 */
static ssize_t ilitek_proc_debug_message_read(struct file *filp,
					      char __user *buff, size_t size,
					      loff_t *pos)
{
	unsigned long p = *pos;
	int i, ret = 0, send_data_len = 0, need = 0;
	unsigned char *tmp = NULL;
	unsigned char tmpbuf[128] = {0};
	u8 *data;

	if (filp->f_flags & O_NONBLOCK)
		return -EAGAIN;

	mutex_lock(&idev->debug_read_mutex);

	ILI_DBG("f_count= %d, index = %d, mark = %d\n", idev->c560,
#line 726
		idev->c564, IDEV_DBL[idev->c564].c0);

	if (!wait_event_interruptible_timeout(IDEV_INQ,
					      IDEV_DBL[idev->c564].c0,
					      msecs_to_jiffies(3000))) {
#line 728
		ILI_ERR("WARNING ! there's no data received.\n");
		mutex_unlock(&idev->debug_read_mutex);
		*pos = 0;
		return 0;
	}

	mutex_lock(&idev->debug_mutex);

	tmp = vmalloc(4096);
	if (IS_ERR(tmp) || tmp == NULL) {
		ILI_ERR("buffer vmalloc error\n");
		send_data_len += snprintf(tmpbuf, 128, "buffer vmalloc error\n");
		copy_to_user(buff, tmpbuf, send_data_len);
		goto out;
	}

	if (IDEV_DBL[idev->c564].c0) {
		data = (u8 *)IDEV_DBL[idev->c564].c8;

		switch (data[0]) {
		case 0x5A:
			need = 43;
			break;
		case 0xA7:
			need = 2040;
			break;
		case 0x7A:
			/*
			 * La fabbrica ha UNA sola `madd`
			 * ("9b09291c madd"@0xffffff8008a6c3f4) alimentata da un
			 * moltiplicatore che i tre rami calcolano
			 * (1, 2 e "4 o 0"): e' la forma con la variabile, non
			 * quella con l'espressione ripetuta caso per caso.
			 * Vedi la divergenza H5.
			 */
			switch (data[3] & 0x0F) {
			case 0:
			case 1:
			case 6:
				need = data[1] * data[2] * 1 + 6;
				break;
			case 2:
			case 3:
				need = data[1] * data[2] * 2 + 6;
				break;
			case 4:
			case 5:
				need = data[1] * data[2] * 4 + 6;
				break;
			default:
				need = data[1] * data[2] * 0 + 6;
				break;
			}
			break;
		default:
			break;
		}

		for (i = 0; i < need; i++) {
			send_data_len += snprintf(tmp + send_data_len, 128,
						  "%02X", data[i]);
			if (send_data_len > 4095) {
				ILI_ERR("send_data_len = %d set 4096 i = %d\n",
#line 767
					send_data_len, i);
				send_data_len = 4096;
				break;
			}
		}

		send_data_len += snprintf(tmp + send_data_len, 128, "\n\n");

		if (p == 5 || size == 4096 || size == 2048) {
			IDEV_DBL[idev->c564].c0 = 0;
			idev->c564 = (idev->c564 + 1) % 1024;
		}
	}

	if (size == 4096)
		ret = copy_to_user(buff, tmp, send_data_len);
	else
		ret = copy_to_user(buff, tmp + p, send_data_len - p);

	if (send_data_len < 1 || send_data_len > 4096) {
#line 790
		ILI_ERR("send_data_len = %d set 4096\n", send_data_len);
		send_data_len = 4096;
	}

	if (ret) {
		ILI_ERR("copy_to_user err\n");
		goto out;
	}

	*pos += send_data_len;

#line 800
	ILI_DBG("Read %d bytes(s) from %ld\n", send_data_len, p);

out:
	mutex_unlock(&idev->debug_mutex);
	mutex_unlock(&idev->debug_read_mutex);
	if (tmp)
		vfree(tmp);
	return send_data_len;
}
