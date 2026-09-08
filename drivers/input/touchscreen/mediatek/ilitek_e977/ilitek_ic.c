// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_ILITEK_E977 -- il GRUPPO D, i registri del chip. Doogee S88 Pro.
 *
 * VENTINOVE funzioni, 16812 byte, 0xffffff8008a5692c..0xffffff8008a5aad8.
 * E' il gruppo fondamentale: i gruppi E, F, G e H lo chiamano tutti.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA. Nessun sorgente
 * pubblico e' stato letto. Un driver Ilitek TDDI circola pubblicamente e
 * ALPS contiene driver touch della stessa famiglia (GT1151/, GT5688/): non
 * sono stati aperti. Ogni riga derivata porta accanto la riga di
 * disassemblato che la giustifica, nella forma
 * "<8 cifre esadecimali> <mnemonico>"@0xINDIRIZZO -- la stessa di
 * `ilitek.h` e di `ilitek_bus.c`, quella che `tools/verificaistruzioni.py`
 * sa confrontare col binario.
 *
 * ===========================================================================
 * IL CONFINE
 * ===========================================================================
 * [0xffffff8008a5692c, 0xffffff8008a5aad8), 29 funzioni (26 `T`, 3 `t`),
 * righe consecutive 37998..38026 di `oracolo/stock.map`. Somma delle
 * dimensioni 16812 = ampiezza dell'intervallo: ZERO buchi di riempimento su
 * 28 coppie consecutive. Zero omonimi su 29 nomi.
 * Il confine e' stato provato tre volte in modo indipendente -- indirizzi,
 * numeri di riga sorgente (i 28 intervalli di `__func__` sono disgiunti e
 * crescenti, e cinque funzioni del gruppo C e quattro del gruppo E
 * COLLIDONO con righe di D, quindi non possono stare nello stesso file), e
 * il blocco `.rodata` proprio (0x923fd70..0x9241c80, in mezzo fra l'ultimo
 * `__func__` di C e il primo di E, senza toccarli).
 *
 * IL BLOCCO `.rodata` NON CONTIENE TUTTI I LETTERALI CHE D USA, e va detto
 * perche' altrimenti il perimetro sembra piu' compatto di quanto sia: OTTO
 * dei 19 nomi di `func_ctrl` cadono FUORI da [0x923fd70, 0x9241c80) --
 * "sleep"@0xffffff800911c431, "lpwg"@0xffffff800923e90a,
 * "gesture"@0xffffff800924b992, "plug"@0xffffff80090fa501,
 * "active"@0xffffff80091ed66a, "idle"@0xffffff80091ed64d,
 * "gesture_demo_en"@0xffffff800923e837, "knock_en"@0xffffff800923e86d.
 * Gli indirizzi vengono dagli addend delle 19 rilocazioni di `func_ctrl`,
 * non da una ricerca di testo: per i da 0 a 18,
 *   ./venv/bin/python3 relocazioni.py oracolo/stock.elf oracolo/stock.map \
 *       --indirizzo $(python3 -c "print(hex(0xffffff80099873a8+I*24))")
 * Degli otto, sette stanno SOTTO il blocco e uno SOPRA. La causa e'
 * una DEDUZIONE, non una misura: `.rodata.str1.1` e' SHF_MERGE|SHF_STRINGS
 * e il linker unifica i letterali identici di unita' diverse, quindi il
 * blocco proprio di D tiene solo quelli che nessun'altra unita' condivide.
 *
 * ===========================================================================
 * LA `.exit.text`, CHE `oracolo/stock.map` NON VEDE
 * ===========================================================================
 * La mappa FINISCE a `_einittext` (0xffffff80093a8518): tutto cio' che sta
 * oltre -- la `.exit.text` -- e' invisibile agli strumenti che partono dalla
 * mappa, e la dimensione di una funzione la' si legge solo dalla distanza
 * fra due prologhi nel disassemblato.
 *
 * IL GRUPPO D NON HA FUNZIONE `.exit`, e il conto e' stato rifatto perche'
 * il numero che stava qui NON SI RIPRODUCE. Diceva «264885 istruzioni oltre
 * `_einittext`»: la sezione `.kernel` va da 0xffffff8008080000 per
 * 0x13ae6d8 byte, quindi finisce a 0xffffff800942e6d8, e
 * [_einittext, fine) sono 0x861c0 byte = 137328 slot da quattro byte, di cui
 * objdump ne decodifica 103909 come istruzioni (il resto e' riempimento a
 * zero, che objdump stampa come '...': 1189 righe). Il numero giusto e'
 * 103909, non 264885, e nessuno dei due e' 137328.
 *   aarch64-linux-android-objdump -d \
 *       --start-address=0xffffff80093a8518 --stop-address=0xffffff800942e6d8 \
 *       oracolo/stock.elf
 * Di quelle 103909, ZERO nominano un indirizzo di
 * [0xffffff8008a5692c, 0xffffff8008a5aad8): ne' con `b`/`bl`, ne' con la
 * coppia `adrp`+`add` (scansione con risoluzione della coppia, non con una
 * ricerca di testo). Le 159 `bl` verso
 * i2c_del_driver / platform_driver_unregister / class_destroy /
 * driver_unregister / tpd_driver_remove che stanno oltre `_einittext` sono
 * tutte di altri driver -- il 159 si riproduce.
 *
 * Quella del driver ilitek e' `ilitek_plat_dev_exit`, ed e' del GRUPPO C:
 * comincia a 0xffffff80093acac4, e la funzione successiva comincia a
 * 0xffffff80093acaf8, quindi misura 0x34 = 52 BYTE (tredici istruzioni), non
 * 48. Stampa la riga sorgente 449 ("52803822 mov"@0xffffff80093acadc) e
 * chiama `tpd_driver_remove` ("97da926f bl"@0xffffff80093acaec) con lo
 * stesso oggetto 0xffffff8009987338 di `ilitek_plat_dev_init`
 * ("910ce000 add"@0xffffff80093acae8, su
 * "f0002ec0 adrp"@0xffffff80093acae4). Il 52 e' scritto qui perche' il
 * lotto C lo erediti: nessuna mappa lo produce.
 *
 * ===========================================================================
 * COME SI RIVERIFICA -- i comandi esatti
 * ===========================================================================
 *   # 1. i letterali
 *   ./venv/bin/python3 verificacitazioni.py ilitek_ic.c oracolo/stock.elf
 *
 *   # 2. le istruzioni. SERVONO TRE INTERVALLI: il blocco D;
 *   #    `fix_tp_proc_info`, che sta nel nucleo `tpd` MediaTek, PRIMA della
 *   #    corsa ilitek, ed e' citata perche' `ilitek_tddi_ic_get_fw_ver` la
 *   #    chiama (vedi la DIVERGENZA 2); e `ilitek_plat_dev_exit`, che sta
 *   #    nella `.exit.text` oltre `_einittext` (vedi la sezione sopra).
 *   ./venv/bin/python3 verificaistruzioni.py ilitek_ic.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a5692c:0xffffff8008a5aad8 \
 *       --intervallo 0xffffff8008a5151c:0xffffff8008a5155c \
 *       --intervallo 0xffffff80093acac4:0xffffff80093acaf8
 *
 *   # 2-ter. IL CONTROLLO ALL'INDIRIZZO, che verificaistruzioni.py NON fa
 *   #  (vedi 2-bis). Si estraggono le citazioni, si disassemblano dall'ELF le
 *   #  sole finestre citate e si confronta l'istruzione VERA a ogni
 *   #  indirizzo:
 *   #    grep -oE '"[0-9a-f]{8} [a-z0-9.]+"@0x[0-9a-f]+' ilitek_ic.c | sort -u
 *   #    aarch64-linux-android-objdump -d --start-address=... --stop-address=...
 *   #        oracolo/stock.elf
 *
 *   # 3. la misura, dal .o VERO -- E CON IL COMPILATORE DI FABBRICA.
 *   #    Il kernel dell'oracolo e' stato compilato con clang r353983c
 *   #    (LLVM 9.0.3); il nostro albero usa r383902 (LLVM 11.0.1). Misurare
 *   #    con r383902 confonde due cose: quanto la ricostruzione si discosta
 *   #    dalla fabbrica e quanto se ne discosta il compilatore. Il compilatore
 *   #    di fabbrica e' in /mnt/s88pro/kernel-stock/clang-r353983c e basta
 *   #    metterlo in testa al PATH del container.
 *   make -C <albero> O=<uscita> ARCH=arm64 -j6 CC=clang HOSTCC=clang \
 *       CLANG_TRIPLE=aarch64-linux-gnu- CROSS_COMPILE=aarch64-linux-android- \
 *       KCFLAGS=-Werror drivers/input/touchscreen/mediatek/ilitek_e977/
 *   aarch64-linux-android-nm --print-size <uscita>/.../ilitek_ic.o | grep " [tT] "
 *
 * ESITO ATTESO, al 2026-08-21:
 *   verificacitazioni.py  -> letterali: 165  citati: 167  verificati: 167
 *                            probanti: 158  deboli: 9  (nessuna mancante,
 *                            nessuna non ancorata). Le 9 deboli sono le
 *                            stringhe corte ('sense', 'sleep', 'plug',
 *                            'idle', 'active', 'gesture', 'Enable', 'ON',
 *                            'OFF') che ricorrono decine di volte nel
 *                            binario: la citazione da sola non prova
 *                            l'indirizzo, e infatti l'indirizzo NON viene da
 *                            una ricerca di testo ma dalla rilocazione della
 *                            voce di `func_ctrl` o dal sito che le
 *                            materializza.
 *   verificaistruzioni.py -> 257 citazioni, 257 confermate, 0 assenti,
 *                            0 mnemonico diverso, 0 controfattuali.
 *   controllo all'indirizzo -> 230 citazioni distinte, 230 esatte
 *                            ALL'INDIRIZZO, 0 sbagliate, 0 fuori finestra.
 *
 *   # 2-bis. IL CONTROLLO CHE `verificaistruzioni.py` NON FA.
 *   #  Il suo stesso conteggio lo dichiara: 257 a codifica, 0 ad indirizzo.
 *   #  Per una citazione di istruzione lo strumento verifica che la CODIFICA
 *   #  esista da qualche parte negli intervalli, MAI che stia all'indirizzo
 *   #  scritto dopo la chiocciola. Un "257 confermate, 0 assenti" percio'
 *   #  NON prova che le citazioni siano al posto giusto: serve leggere
 *   #  l'istruzione vera a ogni indirizzo citato nel disassemblato completo
 *   #  (4.875.132 istruzioni indicizzate).
 *   #  Tre citazioni di questo file erano sbagliate proprio cosi' e il
 *   #  conteggio non le denunciava (classi B7 e B8). In due il commento
 *   #  portava la codifica di un `mov w3, w19` con l'indirizzo della `bl
 *   #  printk` che lo segue di quattro byte -- 0xffffff8008a56ac8 dato come
 *   #  0xffffff8008a56acc, 0xffffff8008a57e24 dato come 0xffffff8008a57e28.
 *   #  Nella terza la codifica 5292801c (che sta a 0xffffff8008a587e4 ed e'
 *   #  la base 0x1FFF9400 delle `wr_pack`, un'altra costante di un'altra
 *   #  riga) era appaiata all'indirizzo 0xffffff8008a58734, dove sta invece
 *   #  52802017: la conclusione era giusta, la prova no.
 *   #  Sono corrette al 2026-08-21, e ora le citazioni con indirizzo sono
 *   #  230 su 230, tutte esatte ALL'INDIRIZZO.
 *   dimensione, col clang DI FABBRICA (r353983c)
 *                         -> 26 funzioni su 29 identiche al byte.
 *                            AVVERTENZA PRIMA DELLA PERCENTUALE: su 29 unita'
 *                            la misura NON discrimina. 26/29 = 89,7%,
 *                            IC95% Clopper-Pearson [72,6%; 97,8%], che
 *                            CONTIENE il 77,10% del ramo.
 *   dimensione, col NOSTRO clang (r383902)
 *                         -> 21 su 29 = 72,4%, IC95% [52,8%; 87,3%], che
 *                            contiene anch'esso il 77,10%. LE DUE MISURE NON
 *                            HANNO LO STESSO INSIEME: con r383902
 *                            `ilitek_tddi_ic_watch_dog_ctrl` misura 1696,
 *                            cioe' ESATTAMENTE come di fabbrica, mentre col
 *                            compilatore giusto misura 1704. Era una
 *                            COINCIDENZA contata come convergenza, ed e' la
 *                            ragione per cui il compilatore sbagliato non e'
 *                            un margine da dichiarare ma una deriva da
 *                            togliere.
 *   CODIFICA (piu' esigente della dimensione), col clang di fabbrica
 *                         -> 16 funzioni su 29 hanno, istruzione per
 *                            istruzione, LA STESSA CODIFICA a 32 bit del
 *                            binario, mascherando i campi rilocati e il solo
 *                            immediato di `__LINE__` (vedi D1).
 *                            16/29 = 55,2%, IC95% [35,7%; 73,6%]: questa
 *                            misura DISCRIMINA rispetto al 77,10%, e dice
 *                            che dieci delle funzioni che passano la misura
 *                            di dimensione contengono codice diverso.
 *                            Col NOSTRO clang la stessa misura vale 0 su 29.
 *   MULTINSIEME delle istruzioni, col clang di fabbrica
 *                         -> per TUTTE E DIECI le funzioni che hanno la
 *                            dimensione di fabbrica ma una codifica diversa,
 *                            il multinsieme delle istruzioni e' identico a
 *                            meno di (a) quale registro e' scelto, (b) la
 *                            forma dell'operando rilocato e (c)
 *                            l'immediato di `__LINE__`. NESSUNA differenza di
 *                            offset, di costante o di operazione: cioe'
 *                            nessun difetto di quelli che la dimensione non
 *                            denuncia e' stato trovato in quelle dieci.
 *   .data                 -> 0x328 = 808 byte = 19x24 (func_ctrl) + 8x44
 *                            (protocol_info). ATTENZIONE: questa e'
 *                            un'IDENTITA' DEL NOSTRO SORGENTE, non una
 *                            conferma della disposizione DI FABBRICA --
 *                            puo' fallire solo se il sorgente contraddice
 *                            se stesso (classe C2). La disposizione di
 *                            fabbrica e' misurata altrove, e senza usare il
 *                            nostro oggetto: 19 rilocazioni `.rela.dyn` a
 *                            passo 24 da 0xffffff80099873a8 e ZERO dentro
 *                            [0xffffff8009987570, 0xffffff80099876d0)
 *                            (0x9987570 - 0x99873a8 = 456 = 19x24, cioe'
 *                            `func_ctrl` finisce esattamente dove
 *                            `protocol_info` comincia), piu' il ciclo
 *                            ("f1004f3f cmp"@0xffffff8008a57f04 = 19 voci,
 *                            "91006273 add"@0xffffff8008a57f08 = passo 24)
 *                            e i sette `add` srotolati 0x570..0x678 a
 *                            passo 44.
 *   .bss                  -> 0x50 = 80 byte. Anche questa e' un'identita'
 *                            del nostro sorgente. Il fatto DI FABBRICA e'
 *                            che l'ultimo campo toccato dalle 29 funzioni e'
 *                            +72, con una `str` da 8 byte
 *                            ("f900252a str"@0xffffff8008a569e0): da qui
 *                            sizeof >= 80. Nessun dato statico delimita la
 *                            fine (zero rilocazioni con addend
 *                            0xffffff800a0fca50 su 178389), quindi 80 e' il
 *                            MINIMO compatibile -- una SCELTA, non una
 *                            misura: 88 misurerebbe uguale.
 *
 * ===========================================================================
 * LA DIFFERENZA DI TOOLCHAIN, MISURATA E NON PIU' ARGOMENTATA
 * ===========================================================================
 * Il nostro clang (r383902) sceglie SEMPRE `mov wN, #imm` dove quello di
 * fabbrica sceglie `orr wN, wzr, #imm` per le costanti che sono immediati
 * logici. E' 1:1 e non sposta nessuna dimensione, ma cambia la CODIFICA.
 * Per questo tutte le citazioni di istruzione di questo file vengono dal
 * disassemblato DI FABBRICA e non dal nostro oggetto: e' la classe di
 * difetto B8 (una citazione che prova la costante giusta con l'istruzione
 * sbagliata), e qui e' evitata per costruzione.
 *
 * Fino al 2026-08-21 le divergenze di dimensione erano ATTRIBUITE al
 * compilatore con un argomento. Ora il compilatore di fabbrica (r353983c) e'
 * disponibile e l'attribuzione e' una MISURA: ricompilando questo stesso
 * file, senza cambiare una riga, con r353983c invece che con r383902,
 *
 *   ilitek_ice_mode_write                272 -> corretta   (era +4)
 *   ilitek_tddi_ic_whole_reset           468 -> corretta   (era -4)
 *   ilitek_tddi_ic_get_ddi_reg_onepage  1108 -> corretta   (era +8)
 *   ilitek_tddi_ic_spi_speed_ctrl        892 -> corretta   (era -8)
 *   ilitek_tddi_ic_get_project_id       1648 -> corretta   (era +8)
 *   ilitek_tddi_ic_watch_dog_ctrl       1696 -> 1704       (era GIUSTA per caso)
 *
 * cioe' cinque divergenze spariscono e una compare. Il saldo e' 21/29 col
 * compilatore sbagliato contro 25/29 con quello giusto, PRIMA delle due
 * correzioni di sorgente di questo lotto (vedi C1 e C2 sotto), 26/29 dopo.
 *
 * ===========================================================================
 * LE DIVERGENZE APERTE
 * ===========================================================================
 * D1. I `__LINE__`. Come in `ilitek_main.c` e `ilitek_bus.c`, questo file
 *     NON riproduce i numeri di riga di fabbrica: sono noti uno per uno
 *     (sono annotati sopra ogni funzione come "righe A..B") ma imbottire di
 *     righe vuote un file ancora in lavorazione sposta tutto al primo
 *     cambiamento. E' una divergenza REALE nel binario e INVISIBILE alla
 *     misura di dimensione: `mov w2,#imm16` e' una istruzione per qualunque
 *     riga.
 * D2. `fix_tp_proc_info` NON ESISTE nel nostro albero. E' verificato:
 *     `grep -rn fix_tp_proc_info <albero>/drivers/ --exclude=ilitek_ic.c`
 *     non trova niente (l'esclusione serve perche' QUESTO file la nomina, e
 *     senza di essa il grep trova se stesso: cosi' com'era scritto prima,
 *     il comando non era piu' vero). Sta a
 *     0xffffff8008a5151c, nel nucleo `tpd` MediaTek, ed e' un'aggiunta
 *     Wingtech. Qui e' DICHIARATA localmente perche' il gruppo D compili; il
 *     link fallira' finche' qualcuno non la scrive. Vedi il DELTA 2.
 * D3. TRE funzioni su 29 non misurano come di fabbrica quando si compila col
 *     compilatore di fabbrica. Per tutte e tre lo scarto e' attribuito
 *     istruzione per istruzione, sul multinsieme completo delle istruzioni,
 *     e per nessuna delle tre e' una differenza di comportamento:
 *
 *     D3a. `ilitek_tddi_ic_func_ctrl`  456 -> 424  (-32 B, -8 istruzioni).
 *          -7 istruzioni sono il TERZO confronto del ciclo,
 *          `strlen(name) == strlen(func_ctrl[i].name)`, che di fabbrica e'
 *          emesso ("94100106 bl"@0xffffff8008a57ee8,
 *          "aa0003f8 mov"@0xffffff8008a57eec,
 *          "aa1703e0 mov"@0xffffff8008a57ef0,
 *          "94100103 bl"@0xffffff8008a57ef4,
 *          "eb00031f cmp"@0xffffff8008a57ef8,
 *          "54000280 b.eq"@0xffffff8008a57efc, piu' il
 *          "aa1503e0 mov"@0xffffff8008a57ee4) e da noi e' eliminato come
 *          ridondante: dopo il primo confronto il compilatore SA gia' che le
 *          due lunghezze sono uguali. -2 istruzioni sono il nono registro
 *          da salvare che quel confronto costringe a tenere vivo
 *          ("f81b0ff9 str"@0xffffff8008a57e88 salva x25 e apre un frame di
 *          80 byte invece di 64); +1 e' la propagazione della costante nella
 *          lunghezza di `strncmp` (di fabbrica "aa0003e2 mov"@0xffffff8008a57f9c
 *          passa il risultato di `strlen`, da noi due `mov w2,#imm`).
 *          CONTROESPERIMENTO: lo STESSO ciclo, compilato con
 *          `-fno-builtin-strncmp`, emette tutte e quattro le `bl strlen`
 *          come la fabbrica (2 -> 4 chiamate) -- quindi la FORMA del
 *          sorgente e' compatibile col binario e l'eliminazione e' una
 *          decisione dell'ottimizzatore, non una riga mancante. Ma con quel
 *          flag la funzione misura 480 e non 456: il flag NON e' la
 *          condizione di fabbrica, e la causa esatta resta APERTA. Non e'
 *          stata «aggiustata» aggiungendo codice.
 *     D3b. `ilitek_ice_mode_ctrl`  1240 -> 1228  (-12 B, -3 istruzioni).
 *          Il multinsieme delle istruzioni e' identico tranne: due `b` in
 *          meno, due riletture di `idev` in meno e un `mov w19, wzr` in piu'.
 *          Sono i due blocchi di `printk` d'errore che la fabbrica colloca
 *          FUORI LINEA (ognuno costa un `b` di ritorno e una rilettura di
 *          `idev` dopo la chiamata) e che noi abbiamo in linea, piu' un
 *          `mov w19, wzr` che la fabbrica condivide e noi duplichiamo.
 *          Nessuna differenza di offset, costante od operazione.
 *          NOTA: -12 e' PEGGIO del -8 che questa funzione misurava prima:
 *          la correzione C2 qui sotto toglie un'istruzione che nel binario
 *          non c'e', e il numero peggiora. Il numero non e' stato riportato
 *          indietro.
 *     D3c. `ilitek_tddi_ic_watch_dog_ctrl`  1696 -> 1704  (+8 B,
 *          +2 istruzioni). Multinsieme identico tranne un `b` e un
 *          `mov w19, wzr` in piu' e una polarita' di salto invertita
 *          (`tbz w0,#31` -> `tbnz w0,#31`): dopo la
 *          `ilitek_ice_mode_write` che segue 'WDT turn off succeed' la
 *          fabbrica salta al `mov w19, wzr` gia' presente
 *          ("36ffd960 tbz"@0xffffff8008a57cc8 verso +0x174) mentre noi ne
 *          emettiamo una copia. E' collocazione dei blocchi: di fabbrica i
 *          TRE `return 0` della funzione finiscono in un solo
 *          `mov w19, wzr`, da noi in due.
 *          CONTROESPERIMENTO NEGATIVO: riscrivere il ciclo come
 *          `for (; timeout > 0; timeout--)` invece di
 *          `while (timeout > 0) { ...; timeout--; }` da' ESATTAMENTE 1704,
 *          quindi la forma del ciclo non e' la causa. La causa esatta resta
 *          APERTA e non e' stata mascherata.
 * D4. La larghezza di `cmd[]` nella tabella `func_ctrl` e' una SCELTA fra 6,
 *     7 e 8 (vedi il commento alla tabella). Le tre danno la stessa
 *     disposizione e lo stesso codice.
 * D5. Le firme di `ilitek_tddi_ic_spi_speed_ctrl` e
 *     `ilitek_tddi_ic_get_project_id` sono DEDOTTE DAL SOLO PROLOGO: le due
 *     funzioni non sono chiamate da nessuna parte del kernel di fabbrica, e
 *     non esiste un sito di chiamata da cui leggerle. Vedi i loro commenti.
 * D6. `idev->c630` e' `u8` in `ilitek.h` ma il binario ci scrive con una
 *     `cset`, che e' cio' che clang fa verso un `bool`. Qui c'e' un `!!`
 *     esplicito. Vedi il DELTA 3.
 * D7. Dieci funzioni hanno la dimensione di fabbrica ma NON la stessa
 *     codifica: `check_support` (41 istruzioni diverse su 107),
 *     `set_ddi_reg_onepage` (7/95), `get_ddi_reg_onepage` (7/277),
 *     `get_pc_counter_forwdt` (5/84), `check_busy` (23/140),
 *     `get_project_id` (244/412), `fw_uart_ctrl` (1/62),
 *     `get_panel_info` (9/134), `get_tp_info` (6/142), `init` (20/26).
 *     Per TUTTE E DIECI il multinsieme delle istruzioni e' identico a meno
 *     del registro scelto, della forma dell'operando rilocato e
 *     dell'immediato di `__LINE__`: nessun offset, nessuna costante e
 *     nessuna operazione differisce. E' l'allocazione dei registri e la
 *     schedulazione, non il sorgente -- ma NON e' provato che il sorgente
 *     sia identico a quello di fabbrica, solo che nessuna delle differenze
 *     osservabili e' semantica.
 *
 * ===========================================================================
 * DUE CONTROLLI CHE LA DIMENSIONE NON FA, E CHE QUI SONO FATTI
 * ===========================================================================
 * K1. L'INCORPORAZIONE, sito per sito. La ricognizione del blocco indicava
 *     come rischio numero uno che `ilitek_ice_mode_write` (che HA un simbolo
 *     proprio, `T` a 0xffffff8008a57098) e' incorporata in tutti e dodici i
 *     chiamanti interni: se il nostro build non la incorporasse, dodici
 *     funzioni divergerebbero insieme. Contando i `__func__` che ogni sito
 *     `printk` del NOSTRO oggetto passa in x1, il censimento e' identico a
 *     quello di fabbrica, voce per voce:
 *       ice_mode_write incorporata in  crc_off_ili7807 4, crc_off_ili9881 4,
 *         ice_mode_bit_mask_write 2, ice_mode_ctrl 2, check_otp_prog_mode 8,
 *         code_reset 2, get_ddi_reg_onepage 4, get_project_id 16,
 *         spi_speed_ctrl 10, watch_dog_ctrl 12, whole_reset 2, wr_pack 2
 *         = 68 siti in 12 funzioni
 *       rd_pack incorporata in get_ddi_reg_onepage           = 5 siti
 *       check_protocol_ver incorporata in get_protocl_ver    = 3 siti
 *     Totale 76 siti con un `__func__` altrui su 211 siti `printk`. I 211,
 *     i 68/12, i 5 e i 3 sono ESATTAMENTE i numeri letti dal binario di
 *     fabbrica. `ilitek_ice_mode_read` NON e' incorporata ne' di fabbrica ne'
 *     da noi.
 * K2. L'ORDINE DEGLI ARGOMENTI DI `printk` (classe di difetto B1: x0 e' il
 *     FORMATO, x1 e' il `__func__`; invertirli non sposta di un byte la
 *     dimensione e stampa spazzatura). Risolvendo x0 e x1 a ogni sito del
 *     NOSTRO oggetto attraverso le rilocazioni e leggendo la stringa vera:
 *     211 siti su 211 hanno in x0 un formato che contiene 'ILITEK: (%s, %d)'
 *     e in x1 il `__func__` della funzione che lo contiene o di una delle
 *     tre incorporate. Zero sospetti.
 *
 * ===========================================================================
 * LE CORREZIONI DI QUESTO LOTTO (2026-08-21), CON LA PROVA
 * ===========================================================================
 * C1. `ilitek_tddi_ic_get_tp_info`: 576 -> 568, ora esatta.
 *     Il binario legge +464 e +466 UNA VOLTA CIASCUNO e ne scrive DUE volte:
 *     "39474109 ldrb"@0xffffff8008a5a46c carica +464 in w9 e
 *     "39005fe9 strb"@0xffffff8008a5a470 e
 *     "39006fe9 strb"@0xffffff8008a5a478 lo scrivono in buf[7] e buf[11];
 *     "39474908 ldrb"@0xffffff8008a5a474 carica +466 in w8 e
 *     "390063e8 strb"@0xffffff8008a5a47c e
 *     "390073e8 strb"@0xffffff8008a5a480 lo scrivono in buf[8] e buf[12].
 *     Scritto come quattro letture distinte il compilatore ne emetteva
 *     QUATTRO (l'indirizzo di `buf` sfugge alla funzione, quindi la scrittura
 *     a `buf[8]` gli impedisce di riusare la lettura di +464); scritto come
 *     `buf[11] = buf[7] = ...` ne emette due, come il binario.
 *     Riordinare le quattro assegnazioni NON basta: provato, 576.
 * C2. `ilitek_ice_mode_ctrl`: tolto `= 0` da `u32 pid`.
 *     In tutta la funzione di fabbrica non c'e' NESSUNA scrittura a [sp,#12]:
 *     ci sono solo le tre letture "b9400fe0 ldr"@0xffffff8008a5732c,
 *     "b9400fe0 ldr"@0xffffff8008a573c0 e
 *     "b9400fe0 ldr"@0xffffff8008a57454, ognuna dopo la
 *     `ilitek_ice_mode_read` che riempie la variabile. Con `= 0` il nostro
 *     oggetto emetteva un `str wzr, [sp,#12]` che il binario non ha.
 *     La dimensione PEGGIORA (da -8 a -12) e la correzione resta.
 *
 * ===========================================================================
 * I LETTERALI, CON IL LORO INDIRIZZO DI FABBRICA
 * ===========================================================================
 * Centosessantacinque letterali, uno per riga, ordinati per indirizzo. Le
 * stringhe brevi ricorrono molte volte nel binario e la citazione da sola
 * non prova l'indirizzo: quelle sono marcate, e il loro indirizzo viene
 * dalla rilocazione della voce di `func_ctrl` (`relocazioni.py`) o dal sito
 * che le materializza, non da una ricerca di testo.
 *
 * "Disable"@0xffffff80090df610
 * "plug"@0xffffff80090fa501   -- 8 occorrenze nel binario, indirizzo dalla rilocazione o dal sito
 * "sleep"@0xffffff800911c431   -- 48 occorrenze nel binario, indirizzo dalla rilocazione o dal sito
 * "Enable"@0xffffff80091e0f72   -- 17 occorrenze nel binario, indirizzo dalla rilocazione o dal sito
 * "idle"@0xffffff80091ed64d   -- 55 occorrenze nel binario, indirizzo dalla rilocazione o dal sito
 * "active"@0xffffff80091ed66a   -- 35 occorrenze nel binario, indirizzo dalla rilocazione o dal sito
 * "gesture_demo_en"@0xffffff800923e837
 * "knock_en"@0xffffff800923e86d
 * "lpwg"@0xffffff800923e90a
 * "\x016ILITEK: (%s, %d): ERROR, ILITEK CHIP (%x, %x) Not found !!\n"@0xffffff800923fd70
 * "\x013ILITEK: (%s, %d): Read data error\n"@0xffffff800923fdcb
 * "\x016ILITEK: (%s, %d): mask value data = %x\n"@0xffffff800923fe0f
 * "\x013ILITEK: (%s, %d): Failed to re-write data in ICE mode, ret = %d\n"@0xffffff800923fe39
 * "\x013ILITEK: (%s, %d): ice mode not enabled\n"@0xffffff800923fe7c
 * "\x013ILITEK: (%s, %d): Failed to write data in ice mode, ret = %d\n"@0xffffff800923febc
 * "\x013ILITEK: (%s, %d): Failed to allocate rxbuf, %ld\n"@0xffffff800923ff11
 * "\x013ILITEK: (%s, %d): Failed to read data in ice mode, ret = %d\n"@0xffffff800923ff44
 * "\x016ILITEK: (%s, %d): %s ICE mode, mcu on = %d\n"@0xffffff800923ff83
 * "\x016ILITEK: (%s, %d): ice mode already enabled\n"@0xffffff800923ffc6
 * "\x013ILITEK: (%s, %d): write ice mode cmd error\n"@0xffffff800923fff4
 * "\x013ILITEK: (%s, %d): Read pid error\n"@0xffffff8009240022
 * "\x013ILITEK: (%s, %d): Enter to ICE Mode failed !!\n"@0xffffff8009240046
 * "\x013ILITEK: (%s, %d): Write 0x0 at 0x47002 failed\n"@0xffffff8009240077
 * "\x016ILITEK: (%s, %d): ice mode already disabled\n"@0xffffff80092400a8
 * "\x013ILITEK: (%s, %d): write ice mode disable failed\n"@0xffffff80092400d7
 * "\x013ILITEK: (%s, %d): Failed to exit ice mode\n"@0xffffff800924010a
 * "\x013ILITEK: (%s, %d): Exit to ICE Mode failed !!\n"@0xffffff8009240137
 * "\x013ILITEK: (%s, %d): ice mode wasn't enabled\n"@0xffffff8009240167
 * "\x013ILITEK: (%s, %d): WDT/CHIP ID is invalid\n"@0xffffff80092401b2
 * "\x016ILITEK: (%s, %d): WDT ctrl is off, do nothing\n"@0xffffff80092401de
 * "\x013ILITEK: (%s, %d): Read wdt error\n"@0xffffff800924020f
 * "\x016ILITEK: (%s, %d): Read WDT: %s\n"@0xffffff8009240233
 * "\x016ILITEK: (%s, %d): %s WDT, key = %x\n"@0xffffff8009240255
 * "\x013ILITEK: (%s, %d): Wrie WDT key failed\n"@0xffffff800924027b
 * "\x013ILITEK: (%s, %d): Write WDT key failed\n"@0xffffff80092402a4
 * "\x013ILITEK: (%s, %d): Read wdt active error\n"@0xffffff80092402ce
 * "\x016ILITEK: (%s, %d): ret = %x\n"@0xffffff80092402f9
 * "\x013ILITEK: (%s, %d): Write 0x0 at %x\n"@0xffffff8009240317
 * "\x013ILITEK: (%s, %d): Write 0x98 at %x\n"@0xffffff800924033c
 * "\x013ILITEK: (%s, %d): WDT turn on/off timeout !, ret = %x\n"@0xffffff8009240362
 * "\x016ILITEK: (%s, %d): WDT turn on succeed\n"@0xffffff800924039b
 * "\x016ILITEK: (%s, %d): WDT turn off succeed\n"@0xffffff80092403c4
 * "\x013ILITEK: (%s, %d): Write turn off cmd failed\n"@0xffffff80092403ee
 * "\x013ILITEK: (%s, %d): Not found function ctrl, %s\n"@0xffffff800924041d
 * "\x013ILITEK: (%s, %d): Non support function ctrl with protocol v5.0\n"@0xffffff8009240467
 * "phone_cover_window"@0xffffff80092404a9
 * "\x016ILITEK: (%s, %d): Non support %s function ctrl\n"@0xffffff80092404bc
 * "\x016ILITEK: (%s, %d): func = %s, len = %d, cmd = 0x%x, 0%x, 0x%x\n"@0xffffff80092404ee
 * "\x013ILITEK: (%s, %d): Write TP function failed\n"@0xffffff800924052e
 * "\x013ILITEK: (%s, %d): Enable ice mode failed before code reset\n"@0xffffff800924055c
 * "\x013ILITEK: (%s, %d): ic code reset failed\n"@0xffffff80092405b4
 * "\x013ILITEK: (%s, %d): Enable ice mode failed after code reset\n"@0xffffff80092405de
 * "\x013ILITEK: (%s, %d): Enable ice mode failed before chip reset\n"@0xffffff800924061b
 * "\x016ILITEK: (%s, %d): ic whole reset key = 0x%x, edge_delay = %d\n"@0xffffff8009240674
 * "\x013ILITEK: (%s, %d): ic whole reset failed\n"@0xffffff80092406b4
 * "\x013ILITEK: (%s, %d): Enable ice mode failed after chip reset\n"@0xffffff80092406df
 * "\x016ILITEK: (%s, %d): setpage =  0x%X setreg = 0x%X\n"@0xffffff800924071c
 * "\x013ILITEK: (%s, %d): Enable ice mode failed before writing ddi reg\n"@0xffffff8009240772
 * "\x013ILITEK: (%s, %d): Disable WDT failed before writing ddi reg\n"@0xffffff80092407b5
 * "\x013ILITEK: (%s, %d): Enable WDT failed after writing ddi reg\n"@0xffffff80092407f4
 * "\x013ILITEK: (%s, %d): Disable ice mode failed after writing ddi reg\n"@0xffffff8009240831
 * "\x016ILITEK: (%s, %d): setpage = 0x%X setreg = 0x%X\n"@0xffffff8009240874
 * "\x013ILITEK: (%s, %d): Enable ice mode failed before reading ddi reg\n"@0xffffff80092408c9
 * "\x013ILITEK: (%s, %d): Disable WDT failed before reading ddi reg\n"@0xffffff800924090c
 * "\x013ILITEK: (%s, %d): Write 0x2 at 0x4800A\n"@0xffffff800924094b
 * "\x016ILITEK: (%s, %d): check page = 0x%X, reg = 0x%X, read 0x%X\n"@0xffffff8009240975
 * "\x013ILITEK: (%s, %d): Enable WDT failed after reading ddi reg\n"@0xffffff80092409b3
 * "\x013ILITEK: (%s, %d): Disable ice mode failed after reading ddi reg\n"@0xffffff80092409f0
 * "\x013ILITEK: (%s, %d): enter ice mode failed in otp\n"@0xffffff8009240a33
 * "\x013ILITEK: (%s, %d): disable WDT failed in otp\n"@0xffffff8009240a88
 * "\x013ILITEK: (%s, %d): Write 0x80 at 0x43008 failed\n"@0xffffff8009240ab7
 * "\x013ILITEK: (%s, %d): Write 0x0 at 0x43030 failed\n"@0xffffff8009240ae9
 * "\x013ILITEK: (%s, %d): Write 0x4 at 0x4300C failed\n"@0xffffff8009240b1a
 * "\x013ILITEK: (%s, %d): Write 0x4 at 0x4300C\n"@0xffffff8009240b4b
 * "\x013ILITEK: (%s, %d): Read prog_done error\n"@0xffffff8009240b75
 * "\x013ILITEK: (%s, %d): Read prog_mode error\n"@0xffffff8009240b9f
 * "\x016ILITEK: (%s, %d): otp prog_mode = 0x%x, prog_done = 0x%x\n"@0xffffff8009240bc9
 * "\x013ILITEK: (%s, %d): OTP Program mode error!\n"@0xffffff8009240c05
 * "\x016ILITEK: (%s, %d): %s spi speed up\n"@0xffffff8009240c32
 * "\x013ILITEK: (%s, %d): Write 0x00000101 at 0x063820 failed\n"@0xffffff8009240c75
 * "\x013ILITEK: (%s, %d): Write 0x00000008 at 0x042c34 failed\n"@0xffffff8009240cae
 * "\x013ILITEK: (%s, %d): Write 0x00000000 at 0x063820 failed\n"@0xffffff8009240ce7
 * "\x013ILITEK: (%s, %d): Write 0x00000000 at 0x042c34 failed\n"@0xffffff8009240d20
 * "\x013ILITEK: (%s, %d): Enable ice mode failed while reading pc counter\n"@0xffffff8009240d59
 * "\x013ILITEK: (%s, %d): Read pc conter error\n"@0xffffff8009240dbc
 * "\x013ILITEK: (%s, %d): read pc (addr: 0x%x) = 0x%x\n"@0xffffff8009240de6
 * "\x013ILITEK: (%s, %d): read latch (addr: 0x%x) = 0x%x\n"@0xffffff8009240e17
 * "\x013ILITEK: (%s, %d): Disable ice mode failed while reading pc counter\n"@0xffffff8009240e4b
 * "\x013ILITEK: (%s, %d): force Disable ice mode\n"@0xffffff8009240eb6
 * "\x013ILITEK: (%s, %d): Error! Interrupt for MP isn't received\n"@0xffffff8009240ee2
 * "\x013ILITEK: (%s, %d): Unknown TP mode (0x%x)\n"@0xffffff8009240f3c
 * "\x016ILITEK: (%s, %d): read byte = %x, delay = %d\n"@0xffffff8009240f82
 * "\x013ILITEK: (%s, %d): Write %x,%x failed\n"@0xffffff8009240fb2
 * "\x013ILITEK: (%s, %d): Write %x failed\n"@0xffffff8009240fda
 * "\x013ILITEK: (%s, %d): Read check busy failed\n"@0xffffff8009240fff
 * "\x016ILITEK: (%s, %d): busy = 0x%x\n"@0xffffff800924102b
 * "\x016ILITEK: (%s, %d): Check busy free\n"@0xffffff800924104c
 * "\x013ILITEK: (%s, %d): Check busy (0x%x) timeout !\n"@0xffffff8009241071
 * "\x013ILITEK: (%s, %d): pdata is null\n"@0xffffff80092410a2
 * "\x016ILITEK: (%s, %d): Read size = %d\n"@0xffffff80092410e3
 * "\x013ILITEK: (%s, %d): Enable ice mode failed while reading project id\n"@0xffffff8009241107
 * "\x013ILITEK: (%s, %d): Pull cs low failed\n"@0xffffff800924114c
 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174
 * "\x013ILITEK: (%s, %d): Write 0x03 at 0x041008\n"@0xffffff800924119a
 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6
 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0
 * "\x013ILITEK: (%s, %d): Read project id error\n"@0xffffff8009241218
 * "\x016ILITEK: (%s, %d): project_id[%d] = 0x%x\n"@0xffffff8009241243
 * "\x013ILITEK: (%s, %d): Pull cs high\n"@0xffffff800924126e
 * "\x013ILITEK: (%s, %d): Disable ice mode failed while reading project id\n"@0xffffff8009241290
 * "\x013ILITEK: (%s, %d): write core ver err\n"@0xffffff80092412d6
 * "\x013ILITEK: (%s, %d): i2c/spi read core ver err\n"@0xffffff800924131a
 * "\x013ILITEK: (%s, %d): Invalid core ver\n"@0xffffff8009241349
 * "\x016ILITEK: (%s, %d): Core version = %d.%d.%d\n"@0xffffff800924136f
 * "\x016ILITEK: (%s, %d): Unknown cmd, ignore\n"@0xffffff800924139c
 * "\x016ILITEK: (%s, %d): %s UART mode\n"@0xffffff80092413de
 * "\x016ILITEK: (%s, %d): Write fw uart cmd failed\n"@0xffffff8009241400
 * "\x013ILITEK: (%s, %d): write firmware ver err\n"@0xffffff800924142e
 * "\x013ILITEK: (%s, %d): i2c/spi read firmware ver err\n"@0xffffff8009241474
 * "\x013ILITEK: (%s, %d): Invalid firmware ver\n"@0xffffff80092414a7
 * "\x016ILITEK: (%s, %d): Firmware version = %d.%d.%d.%d\n"@0xffffff80092414d1
 * "TP IC: ILITEK,TP MODULE: hongzhan,TP I2C ADR: 0x41,SW FirmWare: 0x%06X,Sample FirmWare: 0x%06X"@0xffffff8009241505
 * "\x013ILITEK: (%s, %d): Write panel info error\n"@0xffffff8009241564
 * "\x013ILITEK: (%s, %d): Read panel info error\n"@0xffffff80092415ae
 * "\x016ILITEK: (%s, %d): Invalid panel info, use default resolution\n"@0xffffff80092415d9
 * "\x016ILITEK: (%s, %d): Transfer touch coordinate = %s\n"@0xffffff8009241619
 * "\x016ILITEK: (%s, %d): Panel info: width = %d, height = %d\n"@0xffffff800924164d
 * "\x013ILITEK: (%s, %d): Write tp info error\n"@0xffffff8009241686
 * "\x013ILITEK: (%s, %d): Read tp info error\n"@0xffffff80092416ca
 * "\x013ILITEK: (%s, %d): Invalid tp info\n"@0xffffff80092416f2
 * "\x016ILITEK: (%s, %d): TP Info: min_x = %d, min_y = %d, max_x = %d, max_y = %d\n"@0xffffff8009241717
 * "\x016ILITEK: (%s, %d): TP Info: xch = %d, ych = %d, stx = %d, srx = %d\n"@0xffffff8009241764
 * "\x013ILITEK: (%s, %d): Write protocol version error\n"@0xffffff80092417a9
 * "\x013ILITEK: (%s, %d): Read protocol version error\n"@0xffffff80092417fa
 * "\x013ILITEK: (%s, %d): Invalid protocol ver\n"@0xffffff800924182b
 * "\x016ILITEK: (%s, %d): Protocol version = %d.%d.%d\n"@0xffffff8009241855
 * "\x013ILITEK: (%s, %d): ice mode doesn't enable\n"@0xffffff8009241886
 * "\x013ILITEK: (%s, %d): Read chip id error\n"@0xffffff80092418cb
 * "\x013ILITEK: (%s, %d): Read otp id error\n"@0xffffff80092418f3
 * "\x013ILITEK: (%s, %d): Read ana id error\n"@0xffffff800924191a
 * "\x016ILITEK: (%s, %d): CHIP: PID = %x, ID = %x, TYPE = %x, VER = %x, OTP = %x, ANA = %x\n"@0xffffff8009241941
 * "glove"@0xffffff8009241997
 * "stylus"@0xffffff800924199d
 * "tp_scan_mode"@0xffffff80092419a4
 * "phone_cover"@0xffffff80092419b1
 * "finger_sense"@0xffffff80092419bd
 * "sense"@0xffffff80092419c4   -- 9 occorrenze nel binario, indirizzo dalla rilocazione o dal sito
 * "proximity"@0xffffff80092419ca
 * "edge_palm"@0xffffff80092419d4
 * "lock_point"@0xffffff80092419de
 * "tp_recore"@0xffffff80092419e9
 * "\x013ILITEK: (%s, %d): Read 0x73010 error\n"@0xffffff80092419f3
 * "\x016ILITEK: (%s, %d): check ok 0x73010 read 0x%X retry = %d\n"@0xffffff8009241a32
 * "\x016ILITEK: (%s, %d): check 0x73010 error read 0x%X\n"@0xffffff8009241a6d
 * "\x013ILITEK: (%s, %d): Write %x at 0x73000\n"@0xffffff8009241aa0
 * "\x013ILITEK: (%s, %d): Read 0x4800A error\n"@0xffffff8009241ac9
 * "\x016ILITEK: (%s, %d): check  ok 0x4800A read 0x%X retry = %d\n"@0xffffff8009241b08
 * "\x016ILITEK: (%s, %d): check 0x4800A error read 0x%X\n"@0xffffff8009241b44
 * "\x013ILITEK: (%s, %d): Read 0x73016 error\n"@0xffffff8009241b77
 * "\x016ILITEK: (%s, %d): same procotol version, do nothing\n"@0xffffff8009241b9f
 * "\x016ILITEK: (%s, %d): update protocol version = %x\n"@0xffffff8009241bf8
 * "\x016ILITEK: (%s, %d): Not found a correct protocol version in list, use newest version\n"@0xffffff8009241c2a
 * "gesture"@0xffffff800924b992   -- 3 occorrenze nel binario, indirizzo dalla rilocazione o dal sito
 * "ON"@0xffffff8009250b5c   -- 67 occorrenze nel binario, indirizzo dalla rilocazione o dal sito
 * "OFF"@0xffffff8009250b6d   -- 13 occorrenze nel binario, indirizzo dalla rilocazione o dal sito
 */
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/wait.h>

#include "ilitek.h"

/*
 * ===========================================================================
 * LA STRUTTURA DEL CHIP -- `.bss` a 0xffffff800a0fca50, puntata da `idev->c48`
 * ===========================================================================
 * `ilitek.h` tipa `+48` come `void *` (lo scrisse il gruppo A, che ne vede
 * solo lo store "a9032149 stp"@0xffffff8008a5aad0). Il gruppo D ne legge e
 * scrive DICIOTTO offset, e questa e' la sola unita' che li tocca: la
 * struttura si definisce qui, e nel rapporto c'e' il DELTA DI HEADER che
 * cambierebbe `void *c48` in `struct ilitek_ic_c48 *`.
 *
 * La convenzione dei nomi e' quella di `ilitek.h`: `c<offset>`, e il
 * commento dice cio' che del campo E' MISURATO. Il binario NON nomina
 * nessuno di questi campi. Il formato
 * "\x016ILITEK: (%s, %d): CHIP: PID = %x, ID = %x, TYPE = %x, VER = %x, OTP = %x, ANA = %x\n"@0xffffff8009241941
 * stampa, in quest'ordine, +8 +2 +0 +1 +40 +44
 * ("b9400903 ldr"@0xffffff8008a5a9f8, "79400504 ldrh"@0xffffff8008a5a9fc,
 *  "39400105 ldrb"@0xffffff8008a5aa00, "39400506 ldrb"@0xffffff8008a5aa04,
 *  "29452107 ldp"@0xffffff8008a5aa08): la corrispondenza e' POSIZIONALE,
 * cioe' un'inferenza forte e non un nome dal binario, e resta nel commento.
 */
struct ilitek_ic_c48 {
	/* +0  8 bit. "39400105 ldrb"@0xffffff8008a5aa00 /
	 * "3900010a strb"@0xffffff8008a5a9c8 (= pid >> 8) */
	u8 c0;
	/* +1  8 bit. "39000509 strb"@0xffffff8008a5a9d4 (= pid) */
	u8 c1;
	/* +2  16 bit. "7900050a strh"@0xffffff8008a5a9b8 (= pid >> 16) */
	u16 c2;
	/* +4  16 bit. "79000909 strh"@0xffffff8008a569d4 = 0x9881 oppure
	 * "79000909 strh"@0xffffff8008a56a3c = 0x9878 */
	u16 c4;
	/*
	 * +6..7: BUCO. Nessuna delle 29 funzioni lo tocca; NON riempire, e'
	 * cio' che tiene +8 dov'e'.
	 */
	u8 __ignoto_6[2];
	/* +8  32 bit. "b9000953 str"@0xffffff8008a569b0 (= il primo argomento
	 * di `ilitek_tddi_ic_check_support`) */
	u32 c8;
	/* +12 32 bit. Scritto solo da `ilitek_tddi_ic_init`
	 * ("b9000d2a str"@0xffffff8008a5aaa4) col valore 0x0004009c; letto da
	 * `get_info` come indirizzo di lettura ("b9400d00 ldr"@0xffffff8008a5a8f8) */
	u32 c12;
	/* +16 32 bit = 0x0005100c. Meta' bassa di "f900092a str"@0xffffff8008a5aa94 */
	u32 c16;
	/* +20 32 bit = 0x00044008. Meta' alta della stessa `str` */
	u32 c20;
	/* +24 32 bit = 0x00051010. "2903212b stp"@0xffffff8008a5aac4 */
	u32 c24;
	/* +28 32 bit = 0x00040050. Stessa `stp`; letto da `whole_reset` come
	 * indirizzo ("b9401d2a ldr"@0xffffff8008a58220) */
	u32 c28;
	/* +32 32 bit = 0x000400a0. "2904292c stp"@0xffffff8008a5aab0 */
	u32 c32;
	/* +36 32 bit = 0x000400a4. Stessa `stp` */
	u32 c36;
	/* +40 32 bit. Scritto da `get_info` via `ilitek_ice_mode_read`
	 * ("9100a101 add"@0xffffff8008a5a934) e poi troncato a un byte
	 * ("3940a12a ldrb"@0xffffff8008a5a9e0 + "b900292a str"@0xffffff8008a5a9e4) */
	u32 c40;
	/* +44 32 bit. Idem ("9100b101 add"@0xffffff8008a5a968,
	 * "3940b12a ldrb"@0xffffff8008a5a9ec) */
	u32 c44;
	/* +48 32 bit. "b9003149 str"@0xffffff8008a5a17c: `get_fw_ver` ci mette
	 * i quattro byte di versione in ordine invertito (`rev`) */
	u32 c48;
	/* +52 32 bit. "b9003509 str"@0xffffff8008a59edc: `get_core_ver` ci mette
	 * (b1<<16)|(b2<<8)|b3 */
	u32 c52;
	/* +56 32 bit = 0x1ffff. "b9003909 str"@0xffffff8008a56aa4 */
	u32 c56;
	/*
	 * +60 32 bit = 0x19881 ("b9003d48 str"@0xffffff8008a569c0) oppure
	 * 0x19878 ("b9003d48 str"@0xffffff8008a56a28).
	 * `ilitek_tddi_ic_whole_reset` lo scrive nel registro +28 e lo stampa
	 * col formato
	 * "\x016ILITEK: (%s, %d): ic whole reset key = 0x%x, edge_delay = %d\n"@0xffffff8009240674
	 * : il binario chiama "key" IL VALORE STAMPATO,
	 * non il campo, quindi il nome resta `c60`.
	 */
	u32 c60;
	/* +64 32 bit = 0x1000 oppure 0x2000.
	 * "b900412a str"@0xffffff8008a56a94, "b900414b str"@0xffffff8008a56a68 */
	u32 c64;
	/* +68 8 bit. Messo a 1 da `ilitek_tddi_ic_init`
	 * ("3901112c strb"@0xffffff8008a5aa9c), azzerato da `check_support`
	 * ("3901113f strb"@0xffffff8008a56a08) */
	u8 c68;
	/* +69..71: riempimento fino all'allineamento a 8 di +72 */
	u8 __ignoto_69[3];
	/*
	 * +72 puntatore a funzione. `check_support` ci mette
	 * `firmware_hd_dma_crc_off_ili9881` ("f900252a str"@0xffffff8008a569e0)
	 * o `firmware_hd_dma_crc_off_ili7807`
	 * ("f900254b str"@0xffffff8008a56a5c). E' l'UNICO modo in cui quelle
	 * due funzioni sono raggiungibili: in tutto il kernel non c'e' nessuna
	 * `b`/`bl` verso di loro. Il TIPO -- nessun argomento, nessun ritorno --
	 * viene dal loro prologo, non dal sito di chiamata, che non esiste
	 * dentro il gruppo D.
	 */
	void (*c72)(void);
};

/* `idev->c48` e' `void *` in `ilitek.h`: qui serve tipato. Vedi il DELTA
 * DI HEADER 1 nel rapporto. Non e' un locale: e' un'espressione, cosi' che
 * ogni uso rilegga `idev` e `idev->c48` come fa il binario. */
#define IC	((struct ilitek_ic_c48 *)idev->c48)

/*
 * ===========================================================================
 * LA TABELLA DEI PROTOCOLLI -- `.data` a 0xffffff8009987570, 8 voci da 44
 * ===========================================================================
 * 44 byte = 11 `u32`. `.rela.dyn` non ha NESSUNA rilocazione in
 * [0x9987570, 0x99876a4+44): la tabella non contiene puntatori, e' tutta
 * interi. Il passo 44 e' misurato dagli otto `adrp`+`add` che
 * `ilitek_tddi_ic_get_protocl_ver` srotola:
 * 0x570 0x59c 0x5c8 0x5f4 0x620 0x64c 0x678 0x6a4 ("9115c14a add"@0xffffff8008a5a78c
 * ... "9119e14a add"@0xffffff8008a5a804), che distano 0x2c = 44.
 *
 * Che `c0` sia la versione e' MISURATO: `ilitek_tddi_ic_func_ctrl` fa
 * "f9401d08 ldr"@0xffffff8008a57f54 (idev->c56) poi
 * "b9400108 ldr"@0xffffff8008a57f58 e lo confronta con 0x050000
 * ("7141411f cmp"@0xffffff8008a57f5c).
 *
 * NON `const`: di fabbrica sta in `.data`, accanto a `func_ctrl`.
 * Il NOME della tabella e' SCELTO: `oracolo/stock.map` contiene solo
 * simboli di testo (30100 T, 23372 t, 237 W, 1 A -- zero D/B/R), quindi
 * nessun dato del kernel di fabbrica ha un nome da cui copiare.
 */
struct ilitek_protocol_info {
	u32 c0;			/* la versione: 0x050000 .. 0x050700 */
	u32 c4;
	u32 c8;			/* lunghezza della lettura in `get_protocl_ver` */
	u32 c12;
	u32 c16;
	u32 c20;
	u32 c24;		/* lunghezza della lettura in `get_core_ver` */
	u32 c28;
	u32 c32;
	u32 c36;
	u32 c40;
};

static struct ilitek_protocol_info protocol_info[] = {
	/* i byte vengono da `relocazioni.py`/`rodata.py`, non dedotti */
	{ 0x050000, 4, 4, 14, 30, 5, 5, 2, 8, 3,  8 },
	{ 0x050100, 4, 3, 14, 30, 5, 5, 3, 8, 3,  8 },
	{ 0x050200, 4, 4, 14, 30, 5, 5, 3, 8, 3,  8 },
	{ 0x050300, 9, 4, 14, 30, 5, 5, 3, 8, 3,  8 },
	{ 0x050400, 9, 4, 14, 30, 5, 5, 3, 8, 15, 8 },
	{ 0x050500, 9, 4, 14, 30, 5, 5, 3, 8, 15, 14 },
	{ 0x050600, 9, 4, 14, 30, 5, 5, 3, 8, 15, 14 },
	{ 0x050700, 9, 4, 14, 30, 5, 5, 3, 8, 15, 14 },
};

/*
 * ===========================================================================
 * LA TABELLA `func_ctrl` -- `.data` a 0xffffff80099873a8, 19 voci da 24
 * ===========================================================================
 * Il ciclo di `ilitek_tddi_ic_func_ctrl` la delimita da solo: 19 voci
 * ("f1004f3f cmp"@0xffffff8008a57f04) di 24 byte
 * ("91006273 add"@0xffffff8008a57f08).
 *
 * NON PUO' ESSERE `const`: "39000a74 strb"@0xffffff8008a57ff8 ci scrive
 * dentro. E' il vincolo piu' duro del blocco e nasce da una sola istruzione.
 *
 * LA LARGHEZZA DI `cmd` E' UNA SCELTA, e questa e' la sua misura esatta.
 * `len` sta a +16 ("b9400a64 ldr"@0xffffff8008a57fd0) e il passo e' 24: con
 * `u8 cmd[N]; u32 len;` l'offset di `len` e' 8+N arrotondato a 4, quindi
 * 8+N deve cadere in (12,16], cioe' N in {5,6,7,8}. Con N=4 `len`
 * cadrebbe a +12 e il passo sarebbe 16: N=4 E' ESCLUSO DAL BINARIO, e
 * questo restringe l'intervallo {4..8} che la ricognizione dichiarava.
 * La voce `knock_en` ha len=6 e un byte non nullo a cmd[3], quindi la
 * scrittura di 6 byte richiede N>=6 se il codice di fabbrica non e' fuori
 * limite. Resta {6,7,8}: qui e' 8, ed e' una SCELTA -- le tre danno la
 * stessa disposizione e lo stesso codice.
 *
 * I nomi dei campi vengono dal formato
 * "\x016ILITEK: (%s, %d): func = %s, len = %d, cmd = 0x%x, 0%x, 0x%x\n"@0xffffff80092404ee,
 * che li
 * nomina insieme ai valori che stampa: `name` (x3), `len` (w4, da +16),
 * e tre byte di `cmd` (w5 da +8, w6 da +9, w7 da +10).
 * Il NOME DELLA TABELLA e' scelto (vedi sopra: la mappa non ha simboli di
 * dato).
 */
struct ilitek_func_ctrl {
	const char *name;	/* +0,  risolto dagli addend di `.rela.dyn` */
	u8 cmd[8];		/* +8,  vedi sopra: 8 e' una SCELTA fra 6,7,8 */
	u32 len;		/* +16 */
};

static struct ilitek_func_ctrl func_ctrl[] = {
	{ "sense",			{0x01, 0x01, 0x00}, 3 },
	{ "sleep",			{0x01, 0x02, 0x00}, 3 },
	{ "glove",			{0x01, 0x06, 0x00}, 3 },
	{ "stylus",			{0x01, 0x07, 0x00}, 3 },
	{ "tp_scan_mode",		{0x01, 0x08, 0x00}, 3 },
	{ "lpwg",			{0x01, 0x0A, 0x00}, 3 },
	{ "gesture",			{0x01, 0x0B, 0x3F}, 3 },
	{ "phone_cover",		{0x01, 0x0C, 0x00}, 3 },
	{ "finger_sense",		{0x01, 0x0F, 0x00}, 3 },
	{ "phone_cover_window",		{0x0E, 0x00, 0x00}, 3 },
	{ "proximity",			{0x01, 0x10, 0x00}, 3 },
	{ "plug",			{0x01, 0x11, 0x00}, 3 },
	{ "edge_palm",			{0x01, 0x12, 0x00}, 3 },
	{ "lock_point",			{0x01, 0x13, 0x00}, 3 },
	{ "active",			{0x01, 0x14, 0x00}, 3 },
	{ "idle",			{0x01, 0x19, 0x00}, 3 },
	{ "gesture_demo_en",		{0x01, 0x16, 0x00}, 3 },
	{ "tp_recore",			{0x01, 0x18, 0x00}, 3 },
	{ "knock_en",			{0x01, 0x0A, 0x08, 0x03}, 6 },
};

/*
 * ===========================================================================
 * LA STRUTTURA DEL CHIP, L'OGGETTO
 * ===========================================================================
 * `.bss` a 0xffffff800a0fca50, azzerata; `.rela.dyn` non ha nessuna
 * rilocazione con quell'addend, quindi NESSUN dato statico la punta: e'
 * raggiungibile solo attraverso `idev->c48`, che `ilitek_tddi_ic_init`
 * riempie ("a9032149 stp"@0xffffff8008a5aad0).
 */
static struct ilitek_ic_c48 chip;

/*
 * ===========================================================================
 * LE FUNZIONI DEL GRUPPO D CHE QUESTO LOTTO NON HA SCRITTO
 * ===========================================================================
 * Dichiarate e NON definite: il link che fallisce e' l'esito onesto, uno
 * stub le farebbe passare per scritte (regola 6 del progetto).
 * Le firme vengono dai siti di chiamata che stanno in questo file.
 */

/*
 * `fix_tp_proc_info` sta a 0xffffff8008a5151c, nel nucleo `tpd` MediaTek,
 * PRIMA della corsa ilitek: NON e' del driver, ed e' un'aggiunta Wingtech.
 * Nel nostro albero NON ESISTE (verificato: `grep -rn fix_tp_proc_info
 * .../albero-ilitek-d/drivers/` non trova niente). Qui e' dichiarata
 * localmente perche' il gruppo D non compili grazie a un `#include` che
 * non abbiamo; il DELTA DI HEADER 2 del rapporto dice cosa servirebbe.
 * La firma viene dal suo corpo: "aa0803e1 mov"@0xffffff8008a5153c passa x0
 * a `__memcpy` come sorgente, "92401c33 and"@0xffffff8008a51530 tronca il
 * secondo argomento a un byte, e "2a1f03e0 mov"@0xffffff8008a51550
 * restituisce 0.
 */
int fix_tp_proc_info(char *buf, u8 len);

/*
 * Gruppo G, non scritto. L'unica chiamata del gruppo D verso fuori dal
 * gruppo e dentro il driver: "94003621 bl"@0xffffff8008a59c58 verso
 * <ilitek_tddi_flash_clear_dma> (0xffffff8008a674dc). Nessun argomento e
 * ritorno non usato: e' tutto cio' che il sito decide.
 */
void ilitek_tddi_flash_clear_dma(void);

static void ilitek_tddi_ic_wr_pack(int packet);
static void firmware_hd_dma_crc_off_ili9881(void);
static void firmware_hd_dma_crc_off_ili7807(void);

/*
 * Le funzioni DI QUESTO FILE usate prima di essere definite. L'ordine delle
 * definizioni qui sotto e' quello delle RIGHE SORGENTE di fabbrica (82, 129,
 * 151, 177, ...), letto dai `__LINE__` dei `printk`; l'ordine di emissione
 * nel binario e' un altro, e non e' un'informazione sul sorgente.
 * `ilitek.h` non le dichiara -- verificato: delle sei, `ilitek.h` nomina solo
 * `ilitek_tddi_ic_func_ctrl`, e lo fa dentro un commento (riga 571), non in
 * un prototipo. E' il DELTA DI HEADER 4.
 */
int ilitek_ice_mode_write(u32 addr, u32 data, int len);
int ilitek_ice_mode_read(u32 addr, u32 *data, int len);
int ilitek_tddi_ic_watch_dog_ctrl(bool write, bool on);
int ilitek_tddi_ic_check_support(u32 pid, u16 id);
void ilitek_tddi_ic_get_pc_counter(void);

/*
 * ===========================================================================
 * ilitek_tddi_ic_check_support -- 0xffffff8008a5692c, 428 byte, riga 82
 * ===========================================================================
 * I sette confronti sono letti uno per uno dal binario:
 *   id  == 0x9881   "6b08009f cmp"@0xffffff8008a56940
 *   pid in {0x98811103, 0x98811104}  "0b090269 add"@0xffffff8008a56954 +
 *                                    "7100093f cmp"@0xffffff8008a56958
 *                                    (0x98811103 + 0x677eeefd = 2^32)
 *   pid == 0x9881   "6b09027f cmp"@0xffffff8008a56964
 *   id  == 0x7807   "6b21213f cmp"@0xffffff8008a56970
 *   pid in {0x78071000, 0x78071001}  "0b090269 add"@0xffffff8008a56980
 *   pid == 0x7807   "6b09027f cmp"@0xffffff8008a56990
 *
 * Il messaggio d'errore ha DUE `%x` e il binario ne prepara uno solo,
 * "2a1303e3 mov"@0xffffff8008a56ac8: il secondo (w4) e' ancora quello
 * calcolato in testa, "12003c24 and"@0xffffff8008a56938, cioe' `id`. Non
 * e' un difetto: e' clang che ha programmato l'argomento in anticipo.
 */
int ilitek_tddi_ic_check_support(u32 pid, u16 id)
{
	if (id != 0x9881 && pid != 0x98811103 && pid != 0x98811104 &&
	    pid != 0x9881 && id != 0x7807 && pid != 0x78071000 &&
	    pid != 0x78071001 && pid != 0x7807) {
#line 82
		ILI_INFO("ERROR, ILITEK CHIP (%x, %x) Not found !!\n", pid, id);
		return -1;
	}

	IC->c8 = pid;

	if (id == 0x9881) {
		IC->c60 = 0x19881;
		IC->c4 = 0x9881;
		IC->c72 = firmware_hd_dma_crc_off_ili9881;
		/* "f941a109 ldr"@0xffffff8008a569e4 = idev->c832;
		 * "3941114a ldrb"@0xffffff8008a569f0 = IC->c68 */
		if (idev->c832 && IC->c68) {
			((void (*)(int))idev->c832)(0);
			IC->c68 = 0;
		}
		/* "5281e00a mov"@0xffffff8008a56a10 + "72b3102a movk"@0xffffff8008a56a14 */
		IC->c64 = (pid == 0x98810F00) ? 0x1000 : 0x2000;
	} else {
		IC->c60 = 0x19878;
		IC->c4 = 0x9878;
		IC->c72 = firmware_hd_dma_crc_off_ili7807;
		IC->c64 = 0x2000;
		if (pid != 0x78071000 && pid != 0x78071001)
			IC->c68 = 0;
		/* "121d726a and"@0xffffff8008a56a78 */
		if ((pid & 0xFFFFFFF8) == 0x78071000)
			idev->c639 = 1;
	}

	IC->c56 = 0x1FFFF;
	return 0;
}

/*
 * ===========================================================================
 * firmware_hd_dma_crc_off_ili9881 -- 0xffffff8008a56ad8, 312 byte
 * ===========================================================================
 * `t` nella mappa, quindi `static`. Nessuna `b`/`bl` in tutto il kernel la
 * raggiunge: si arriva qui solo dal puntatore a `IC->c72`.
 * Le due `ilitek_ice_mode_write` sono INCORPORATE (zero `bl` nel gruppo).
 * Gli indirizzi vengono dai byte del buffer:
 * "5282c4a9 mov"@0xffffff8008a56b14 + "72a08209 movk"@0xffffff8008a56b18
 * mettono 0x04101625 a [sp,#8], cioe' 0x25 0x16 0x10 0x04 -> 0x041016;
 * "d28904a9 mov"@0xffffff8008a56b8c ... "f2c00029 movk"@0xffffff8008a56b94
 * mettono 0x0000000104104825, cioe' addr 0x041048 e dato 1 su 4 byte
 * ("321d03e1 orr"@0xffffff8008a56ba4 = len+4 = 8).
 */
static void firmware_hd_dma_crc_off_ili9881(void)
{
	ilitek_ice_mode_write(0x041016, 0x00, 1);
	ilitek_ice_mode_write(0x041048, 0x01, 4);
}

/*
 * ===========================================================================
 * firmware_hd_dma_crc_off_ili7807 -- 0xffffff8008a56c10, 316 byte
 * ===========================================================================
 * "5282e4a9 mov"@0xffffff8008a56cc4 + "72a08209 movk"@0xffffff8008a56cc8
 * = 0x04101725 -> addr 0x041017; "320007ea orr"@0xffffff8008a56ccc = dato 3,
 * "528000a1 mov"@0xffffff8008a56ce0 = len+4 = 5 -> len 1.
 */
static void firmware_hd_dma_crc_off_ili7807(void)
{
	ilitek_ice_mode_write(0x041016, 0x00, 1);
	ilitek_ice_mode_write(0x041017, 0x03, 1);
}

/*
 * ===========================================================================
 * ilitek_ice_mode_bit_mask_write -- 0xffffff8008a56d4c, 404 byte, righe 129..140
 * ===========================================================================
 * "0a350108 bic"@0xffffff8008a56d9c = data & ~mask,
 * "0a15028a and"@0xffffff8008a56d98 = value & mask,
 * "2a0a0114 orr"@0xffffff8008a56da0 la fusione. L'ordine dei due argomenti
 * e' deciso da quale registro entra in quale operazione: w21 = x1 = mask,
 * w20 = x2 = value.
 */
int ilitek_ice_mode_bit_mask_write(u32 addr, u32 mask, u32 value)
{
	int ret = 0;
	u32 data = 0;

	ret = ilitek_ice_mode_read(addr, &data, sizeof(u32));
	if (ret < 0) {
#line 129
		ILI_ERR("Read data error\n");
		return -1;
	}

	data = (data & (~mask)) | (value & mask);

	/* \x016 + guardia "39656129 ldrb"@0xffffff8008a56d94 -> ILI_DBG */
	ILI_DBG("mask value data = %x\n", data);

	ret = ilitek_ice_mode_write(addr, data, 4);
	if (ret < 0)
		ILI_ERR("Failed to re-write data in ICE mode, ret = %d\n", ret);

	return ret;
}

/*
 * ===========================================================================
 * ilitek_ice_mode_write -- 0xffffff8008a57098, 272 byte, righe 151..165
 * ===========================================================================
 * `T` nella mappa E incorporata in tutti e dodici i chiamanti interni: nel
 * disassemblato del gruppo non c'e' NESSUNA `bl ilitek_ice_mode_write`.
 * I dodici sono le dodici funzioni del gruppo (oltre a `ice_mode_write`
 * stessa) che materializzano il suo sito d'errore incorporato,
 * "\x013ILITEK: (%s, %d): Failed to write data in ice mode, ret = %d\n"@0xffffff800923febc.
 * DEI DODICI, SETTE MISURANO ESATTAMENTE come la fabbrica
 * (firmware_hd_dma_crc_off_ili7807, firmware_hd_dma_crc_off_ili9881,
 * ilitek_ice_mode_bit_mask_write, ilitek_tddi_ic_check_otp_prog_mode,
 * ilitek_tddi_ic_code_reset, ilitek_tddi_ic_watch_dog_ctrl,
 * ilitek_tddi_ic_wr_pack) e CINQUE divergono (ice_mode_ctrl -8,
 * whole_reset -4, get_ddi_reg_onepage +8, spi_speed_ctrl -8,
 * get_project_id +8): sette su dodici, non undici. L'argomento regge lo
 * stesso -- se qui `ice_mode_write` fosse rimasta una chiamata i dodici
 * sarebbero divergiti TUTTI, e sette invece coincidono al byte -- ma il
 * numero e' 7/12.
 * NON va marcata `static inline`: 86 siti fuori dal gruppo la chiamano.
 *
 * `u8 txbuf[64]`: la dimensione e' MISURATA dalle quattro
 * "a903ffff stp"@0xffffff8008a570bc, "a902ffff stp"@0xffffff8008a570c0,
 * "a901ffff stp"@0xffffff8008a570c4, "a900ffff stp"@0xffffff8008a570c8 --
 * 4 x 16 = 64 byte azzerati a partire da [sp,#8].
 * L'indice del ciclo e' esteso CON SEGNO ("9360fd6f asr"@0xffffff8008a57110,
 * "382f698e strb"@0xffffff8008a57120): `i` e' `int`.
 */
int ilitek_ice_mode_write(u32 addr, u32 data, int len)
{
	int i, ret = 0;
	u8 txbuf[64] = {0};

	if (!idev->c712) {
#line 151
		ILI_ERR("ice mode not enabled\n");
		return -1;
	}

	txbuf[0] = 0x25;
	txbuf[1] = (char)((addr & 0x000000FF) >> 0);
	txbuf[2] = (char)((addr & 0x0000FF00) >> 8);
	txbuf[3] = (char)((addr & 0x00FF0000) >> 16);

	for (i = 0; i < len; i++)
		txbuf[i + 4] = (char)(data >> (8 * i));

	ret = idev->c776(txbuf, len + 4);
	if (ret < 0)
		ILI_ERR("Failed to write data in ice mode, ret = %d\n", ret);

	return ret;
}

/*
 * ===========================================================================
 * ilitek_ice_mode_read -- 0xffffff8008a56ee0, 440 byte, righe 177..208
 * ===========================================================================
 * NON incorporata: 19 `bl ilitek_ice_mode_read` nel gruppo. La differenza
 * con la gemella e' MISURATA, non scelta.
 *
 * `kzalloc`: "52901801 mov"@0xffffff8008a56f50 +
 * "72a02801 movk"@0xffffff8008a56f58 = 0x14080c0 =
 * GFP_KERNEL (0x14000c0) | __GFP_ZERO (0x8000), e la taglia non e'
 * costante, quindi `kmalloc` degrada a `__kmalloc`
 * ("97dfe50b bl"@0xffffff8008a56f5c).
 * Il controllo e' "b140041f cmn"@0xffffff8008a56f64 (IS_ERR) seguito da
 * "b4000493 cbz"@0xffffff8008a56f6c (NULL), in quest'ordine.
 * La `kfree` e' PROTETTA da un test di non-nullita'
 * ("b5000133 cbnz"@0xffffff8008a57038): sul ramo in cui `rxbuf` e'
 * dimostrabilmente NULL il compilatore salta la chiamata del tutto
 * ("1400000a b"@0xffffff8008a5703c).
 */
int ilitek_ice_mode_read(u32 addr, u32 *data, int len)
{
	int ret = 0;
	u8 *rxbuf = NULL;
	u8 txbuf[4] = {0};

	if (!idev->c712) {
#line 177
		ILI_ERR("ice mode not enabled\n");
		return -1;
	}

	txbuf[0] = 0x25;
	txbuf[1] = (char)((addr & 0x000000FF) >> 0);
	txbuf[2] = (char)((addr & 0x0000FF00) >> 8);
	txbuf[3] = (char)((addr & 0x00FF0000) >> 16);

	ret = idev->c776(txbuf, 4);
	if (ret < 0)
		goto out;

	rxbuf = kzalloc(len, GFP_KERNEL);
	if (IS_ERR(rxbuf) || rxbuf == NULL) {
		ILI_ERR("Failed to allocate rxbuf, %ld\n", PTR_ERR(rxbuf));
		ret = -ENOMEM;
		goto out;
	}

	ret = idev->c784(rxbuf, len);
	if (ret < 0)
		goto out;

	if (len == 1)
		*data = rxbuf[0];
	else
		*data = (rxbuf[0] | rxbuf[1] << 8 | rxbuf[2] << 16 |
			 rxbuf[3] << 24);

out:
	if (ret < 0)
#line 208
		ILI_ERR("Failed to read data in ice mode, ret = %d\n", ret);

	if (rxbuf)
		kfree(rxbuf);

	return ret;
}

/*
 * ===========================================================================
 * ilitek_ice_mode_ctrl -- 0xffffff8008a571a8, 1240 byte, righe 221..294
 * ===========================================================================
 * I DUE PARAMETRI SONO `bool`, e lo dice il binario: sono provati sul SOLO
 * bit 0 ("7200029f tst"@0xffffff8008a571fc) e mascherati con 1 prima di
 * essere stampati ("12000264 and"@0xffffff8008a5720c).
 *
 * I due comandi da 4 byte sono inizializzati da UNA sola `stp`
 * ("29022be9 stp"@0xffffff8008a571e4) perche' i due valori distano 10
 * ("1100292a add"@0xffffff8008a571e0): 0x1810621b -> {0x1b,0x62,0x10,0x18}
 * e 0x18106225 -> {0x25,0x62,0x10,0x18}.
 *
 * I DUE CICLI DI RITENTATIVO SONO SROTOLATI TRE VOLTE ciascuno (l'entrata a
 * 0xffffff8008a572ac / 0xffffff8008a5733c / 0xffffff8008a573d0, l'uscita a
 * 0xffffff8008a57254 / 0xffffff8008a57538 / 0xffffff8008a575c8): il limite 3
 * non compare come immediato da nessuna parte, e' il conteggio delle copie.
 *
 * La prima `write` del ramo di uscita e' UNA SOLA per entrambe le vie perche'
 * il compilatore ha sollevato l'istruzione comune sopra la diramazione:
 * "3949fd14 ldrb"@0xffffff8008a57250 legge `idev->c639` PRIMA della `write`
 * ma la prova solo dopo ("340014d4 cbz"@0xffffff8008a57268).
 */
int ilitek_ice_mode_ctrl(bool enable, bool mcu)
{
	int ret = 0, retry = 0;
	u8 cmd_open[4] = {0x25, 0x62, 0x10, 0x18};
	u8 cmd_close[4] = {0x1B, 0x62, 0x10, 0x18};
	/*
	 * `pid` NON e' inizializzato di fabbrica: in tutta la funzione non c'e'
	 * nessuna scrittura a [sp,#12] -- solo le tre letture
	 * "b9400fe0 ldr"@0xffffff8008a5732c,
	 * "b9400fe0 ldr"@0xffffff8008a573c0 e
	 * "b9400fe0 ldr"@0xffffff8008a57454, ognuna dopo la
	 * `ilitek_ice_mode_read` che riempie la variabile. Con `= 0` il nostro
	 * oggetto emetteva un `str wzr, [sp,#12]` in piu' che il binario non ha.
	 */
	u32 pid;

	ILI_DBG("%s ICE mode, mcu on = %d\n", (enable ? "Enable" : "Disable"),
#line 221
		mcu);

	if (enable) {
		if (idev->c712) {
			ILI_INFO("ice mode already enabled\n");
			return 0;
		}

		/* "320013e9 orr"@0xffffff8008a5729c = 0x1f */
		if (mcu)
			cmd_open[0] = 0x1F;

		/* "b902c909 str"@0xffffff8008a572a8 */
		idev->c712 = 1;

		for (retry = 0; retry < 3; retry++) {
			if (idev->c776(cmd_open, sizeof(cmd_open)) < 0)
#line 236
				ILI_ERR("write ice mode cmd error\n");

			if (idev->c832 && IC->c68)
				((void (*)(int))idev->c832)(1);

			if (ilitek_ice_mode_read(IC->c12, &pid,
						 sizeof(u32)) < 0)
				ILI_ERR("Read pid error\n");

			/* "53107c01 lsr"@0xffffff8008a57330 */
			if (!ilitek_tddi_ic_check_support(pid, pid >> 16))
				break;
		}

		if (retry >= 3) {
#line 250
			ILI_ERR("Enter to ICE Mode failed !!\n");
			ret = -1;
			goto out;
		}

		if (ilitek_ice_mode_write(0x47002, 0x00, 1) < 0)
#line 259
			ILI_ERR("Write 0x0 at 0x47002 failed\n");

		return 0;
	}

	if (!idev->c712) {
#line 263
		ILI_INFO("ice mode already disabled\n");
		return 0;
	}

	if (idev->c639) {
		for (retry = 0; retry < 3; retry++) {
			ret = idev->c776(cmd_close, sizeof(cmd_close));
			if (ret < 0)
#line 270
				ILI_ERR("write ice mode disable failed\n");

			/* "71028c1f cmp"@0xffffff8008a57280 = 0xa3 */
			if (!idev->c800 || idev->c800() == 0xA3) {
				ret = 0;
				break;
			}
			usleep_range(1000, 1000);
		}

		if (retry >= 3) {
#line 287
			ILI_ERR("Failed to exit ice mode\n");
			ret = -EIO;
		}
	} else {
		ret = idev->c776(cmd_close, sizeof(cmd_close));
		if (ret < 0)
#line 294
			ILI_ERR("Exit to ICE Mode failed !!\n");
	}

out:
	/* "b902c91f str"@0xffffff8008a57600 */
	idev->c712 = 0;
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_watch_dog_ctrl -- 0xffffff8008a57680, 1696 byte, righe 307..378
 * ===========================================================================
 * La funzione piu' grande del blocco insieme a `get_project_id`. Entrambi i
 * parametri sono `bool` (provati sul solo bit 0:
 * "360008e8 tbz"@0xffffff8008a576e0 e "7200003f tst"@0xffffff8008a576fc).
 *
 * IL RITORNO DEL RAMO DI LETTURA E' IL VALORE LETTO
 * ("b94007f3 ldr"@0xffffff8008a5780c mette `read` in w19, che e' il
 * registro del ritorno): e' cio' che permette a
 * `ilitek_tddi_ic_set_ddi_reg_onepage` di scrivere
 * `wdt = ilitek_tddi_ic_watch_dog_ctrl(0, 0)` e provarlo per zero.
 *
 * DUE MESSAGGI QUASI IDENTICI, e vanno tenuti distinti perche' il
 * compilatore fonde le stringhe uguali e ne separa una lettera:
 *   "\x013ILITEK: (%s, %d): Wrie WDT key failed\n"@0xffffff800924027b  (riga 335, ON)
 *   "\x013ILITEK: (%s, %d): Write WDT key failed\n"@0xffffff80092402a4 (righe 340, 342, OFF)
 *
 * I due ritardi sono `udelay`, non `mdelay`, e i numeri vengono dagli
 * immediati divisi per 0x10c7: "52952680 mov"@0xffffff8008a57878 +
 * "72a00260 movk"@0xffffff8008a5787c = 0x13a934 = 300 x 0x10c7 -> 300 us;
 * "5293e300 mov"@0xffffff8008a57a54 + "72a00040 movk"@0xffffff8008a57a58
 * = 0x29f18 = 40 x 0x10c7 -> 40 us.
 */
int ilitek_tddi_ic_watch_dog_ctrl(bool write, bool on)
{
	int timeout = 50;
	u32 read = 0;

	if (!idev->c712) {
#line 307
		ILI_ERR("ice mode wasn't enabled\n");
		return -1;
	}

	if (IC->c16 == 0 || IC->c2 == 0) {
		ILI_ERR("WDT/CHIP ID is invalid\n");
		return -EINVAL;
	}

	if (!idev->c628) {
#line 318
		ILI_INFO("WDT ctrl is off, do nothing\n");
		return 0;
	}

	if (!write) {
		if (ilitek_ice_mode_read(IC->c16, &read, sizeof(u8)) < 0) {
			ILI_ERR("Read wdt error\n");
			return -1;
		}
		ILI_INFO("Read WDT: %s\n", (read ? "ON" : "OFF"));
		return read;
	}

	ILI_INFO("%s WDT, key = %x\n", (on ? "Enable" : "Disable"), IC->c4);

	if (on) {
		if (ilitek_ice_mode_write(IC->c16, 0x1, 1) < 0)
			ILI_ERR("Wrie WDT key failed\n");
	} else {
		udelay(300);
		if (ilitek_ice_mode_write(IC->c16, IC->c4 & 0xFF, 1) < 0)
#line 340
			ILI_ERR("Write WDT key failed\n");
		if (ilitek_ice_mode_write(IC->c16, IC->c4 >> 8, 1) < 0)
			ILI_ERR("Write WDT key failed\n");
	}

	while (timeout > 0) {
		udelay(40);

		if (ilitek_ice_mode_read(0x51018, &read, sizeof(u8)) < 0)
#line 348
			ILI_ERR("Read wdt active error\n");

		ILI_DBG("ret = %x\n", read);

		if (on) {
			if (read == 0xA5) {
#line 374
				ILI_INFO("WDT turn on succeed\n");
				return 0;
			}
		} else {
			if (read == 0x5A) {
#line 376
				ILI_INFO("WDT turn off succeed\n");
				if (ilitek_ice_mode_write(IC->c16, 0x00, 1) < 0)
					ILI_ERR("Write turn off cmd failed\n");
				return 0;
			}

			if (ilitek_ice_mode_write(IC->c16, 0x00, 1) < 0)
#line 360
				ILI_ERR("Write 0x0 at %x\n", IC->c16);

			if (ilitek_ice_mode_write(IC->c16, 0x98, 1) < 0)
#line 362
				ILI_ERR("Write 0x98 at %x\n", IC->c16);
		}
		timeout--;
	}

#line 368
	ILI_ERR("WDT turn on/off timeout !, ret = %x\n", read);
	ilitek_tddi_ic_get_pc_counter();
	return -EINVAL;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_pc_counter -- 0xffffff8008a57d20, 360 byte, righe 693..708
 * ===========================================================================
 * I DUE INDIRIZZI SONO LOCALI, e la misura lo prova:
 * "2942cd14 ldp"@0xffffff8008a57d58 li legge UNA volta in testa, e le due
 * `printk` di coda usano ancora w20 e w19 ("2a1403e3 mov"@0xffffff8008a57e00,
 * "2a1303e3 mov"@0xffffff8008a57e24) senza rileggerli. Con `IC->c20` scritto
 * a ogni uso il compilatore ricaricherebbe dopo ogni chiamata opaca
 * (classe A2 alla rovescia).
 */
void ilitek_tddi_ic_get_pc_counter(void)
{
	int ice = idev->c712;
	u32 pc = 0, latch = 0;
	u32 pc_addr = IC->c20;
	u32 latch_addr = IC->c24;

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
#line 693
			ILI_ERR("Enable ice mode failed while reading pc counter\n");

	if (ilitek_ice_mode_read(pc_addr, &pc, sizeof(u32)) < 0)
		ILI_ERR("Read pc conter error\n");

	if (ilitek_ice_mode_read(latch_addr, &latch, sizeof(u32)) < 0)
		ILI_ERR("Read pc conter error\n");

	idev->c228 = pc;
	idev->c232 = latch;
	ILI_ERR("read pc (addr: 0x%x) = 0x%x\n", pc_addr, idev->c228);
	ILI_ERR("read latch (addr: 0x%x) = 0x%x\n", latch_addr, idev->c232);

	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Disable ice mode failed while reading pc counter\n");
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_func_ctrl -- 0xffffff8008a57e88, 456 byte, righe 396..423
 * ===========================================================================
 * Il ciclo confronta con la tabella; il rifiuto sopra la versione 0x0505ff
 * riguarda due nomi soli, e i due `strncmp` sono FUSI dal compilatore in
 * uno solo con due `add x1` diversi
 * ("91264821 add"@0xffffff8008a57fb4 = 'gesture',
 *  "9112a421 add"@0xffffff8008a57fc0 = 'phone_cover_window',
 *  "941000ef bl"@0xffffff8008a57fc8 la chiamata comune).
 * "39000a74 strb"@0xffffff8008a57ff8 e' la scrittura che vieta il `const`.
 */
int ilitek_tddi_ic_func_ctrl(const char *name, int ctrl)
{
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(func_ctrl); i++) {
		if (strlen(name) != strlen(func_ctrl[i].name))
			continue;
		if (strncmp(name, func_ctrl[i].name, strlen(name)))
			continue;
		if (strlen(name) == strlen(func_ctrl[i].name))
			break;
	}

	if (i >= ARRAY_SIZE(func_ctrl)) {
#line 396
		ILI_ERR("Not found function ctrl, %s\n", name);
		return -1;
	}

	/* "7141411f cmp"@0xffffff8008a57f5c */
	if (((struct ilitek_protocol_info *)idev->c56)->c0 == 0x050000) {
		ILI_ERR("Non support function ctrl with protocol v5.0\n");
		return -1;
	}

	/* "1117fd29 add"@0xffffff8008a57f88 = 0x50000 + 0x5ff */
	if (((struct ilitek_protocol_info *)idev->c56)->c0 > 0x0505FF) {
		if ((strlen(func_ctrl[i].name) == strlen("gesture") &&
		     !strncmp(func_ctrl[i].name, "gesture",
			      strlen(func_ctrl[i].name))) ||
		    (strlen(func_ctrl[i].name) == strlen("phone_cover_window") &&
		     !strncmp(func_ctrl[i].name, "phone_cover_window",
			      strlen(func_ctrl[i].name)))) {
			ILI_INFO("Non support %s function ctrl\n",
#line 410
				 func_ctrl[i].name);
			return -1;
		}
	}

	func_ctrl[i].cmd[2] = ctrl;

	ILI_INFO("func = %s, len = %d, cmd = 0x%x, 0%x, 0x%x\n",
		 func_ctrl[i].name, func_ctrl[i].len, func_ctrl[i].cmd[0],
		 func_ctrl[i].cmd[1], func_ctrl[i].cmd[2]);

	if (idev->c776(func_ctrl[i].cmd, func_ctrl[i].len) < 0) {
#line 423
		ILI_ERR("Write TP function failed\n");
		return -1;
	}

	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_code_reset -- 0xffffff8008a58050, 328 byte, righe 436..444
 * ===========================================================================
 * `ice` E' UN LOCALE, e lo prova la misura: "b942c914 ldr"@0xffffff8008a58074
 * legge +712 una volta sola e "35000174 cbnz"@0xffffff8008a58140 riusa lo
 * stesso w20 dopo tre chiamate opache.
 * L'indirizzo e il dato vengono dai byte:
 * "528804a9 mov"@0xffffff8008a580c0 + "72a08009 movk"@0xffffff8008a580c4
 * = 0x04004025 -> 0x25 0x40 0x00 0x04 -> addr 0x040040, e
 * "528015ca mov"@0xffffff8008a580c8 = 0xae con len+4 = 5.
 */
int ilitek_tddi_ic_code_reset(void)
{
	int ret = 0;
	int ice = idev->c712;

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
#line 436
			ILI_ERR("Enable ice mode failed before code reset\n");

	ret = ilitek_ice_mode_write(0x40040, 0xAE, 1);
	if (ret < 0)
		ILI_ERR("ic code reset failed\n");

	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Enable ice mode failed after code reset\n");

	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_whole_reset -- 0xffffff8008a58198, 468 byte, righe 455..472
 * ===========================================================================
 * `mdelay` con argomento NON costante: "b9826908 ldrsw"@0xffffff8008a58294
 * estende `idev->c616` CON SEGNO a 64 bit (`unsigned long __ms = (n)` con
 * `n` di tipo `int`), e il ciclo e' `__const_udelay(0x418958)`
 * ("52912b00 mov"@0xffffff8008a582a0 + "72a00820 movk"@0xffffff8008a582a4),
 * cioe' 1000 x 0x10c7 = udelay(1000).
 */
int ilitek_tddi_ic_whole_reset(void)
{
	int ret = 0;
	int ice = idev->c712;

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
#line 455
			ILI_ERR("Enable ice mode failed before chip reset\n");

	ILI_INFO("ic whole reset key = 0x%x, edge_delay = %d\n",
		 IC->c60, idev->c616);

	ret = ilitek_ice_mode_write(IC->c28, IC->c60, 4);
	if (ret < 0) {
		ILI_ERR("ic whole reset failed\n");
		goto out;
	}

	/* "940ffdf3 bl"@0xffffff8008a582a8 verso <__const_udelay> */
	mdelay(idev->c616);

out:
	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Enable ice mode failed after chip reset\n");

	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_wr_pack -- 0xffffff8008a584e8, 540 byte, righe 484..497
 * ===========================================================================
 * `t` nella mappa: `static`. Quattordici siti, tutti interni al gruppo.
 * `retry` parte da 100: "52800c74 mov"@0xffffff8008a58514 mette 99, cioe'
 * il valore GIA' decrementato dal `while (retry--)`, e l'uscita e'
 * "3100069f cmn"@0xffffff8008a585d4, cioe' `retry != -1`.
 * `mdelay(10)` e' srotolato in dieci `__const_udelay(0x418958)`
 * (0xffffff8008a58558 .. 0xffffff8008a585cc), che e' esattamente cio' che
 * la macro produce quando 10 > MAX_UDELAY_MS.
 * DUE SPAZI dopo "check" nel messaggio della gemella `rd_pack`; qui UNO.
 */
static void ilitek_tddi_ic_wr_pack(int packet)
{
	int retry = 100;
	u32 reg_data = 0;

	while (retry--) {
		if (ilitek_ice_mode_read(0x73010, &reg_data, sizeof(u8)) < 0)
#line 484
			ILI_ERR("Read 0x73010 error\n");

		if ((reg_data & 0x02) == 0) {
			ILI_INFO("check ok 0x73010 read 0x%X retry = %d\n",
#line 487
				 reg_data, retry);
			break;
		}
		mdelay(10);
	}

	if (retry <= 0)
		ILI_INFO("check 0x73010 error read 0x%X\n", reg_data);

	if (ilitek_ice_mode_write(0x73000, packet, 4) < 0)
		ILI_ERR("Write %x at 0x73000\n", packet);
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_set_ddi_reg_onepage -- 0xffffff8008a5836c, 380 byte, righe 536..562
 * ===========================================================================
 * "321853f3 mov"@0xffffff8008a5838c = 0x1fffff00 e
 * "33001c13 bfxil"@0xffffff8008a583a0 inseriscono i soli otto bit bassi di
 * `page`: e' un `u8`. Idem per `reg` ("33101d2a bfi"@0xffffff8008a58398)
 * e `data` ("12001c4a and"@0xffffff8008a58388).
 * Il valore di `ilitek_tddi_ic_watch_dog_ctrl(0, 0)` e' tenuto in un locale
 * e riusato: il compilatore ha DUPLICATO la coda invece di tenerlo in un
 * registro (le due copie dei quattro `wr_pack` a 0xffffff8008a58434 e
 * 0xffffff8008a58488).
 * DUE SPAZI dopo il primo `=` nel formato @0xffffff800924071c.
 */
void ilitek_tddi_ic_set_ddi_reg_onepage(u8 page, u8 reg, u8 data)
{
	int ice = idev->c712;
	int wdt;
	/* i nomi `setpage`/`setreg` vengono dal formato, non sono scelti */
	u32 setpage = 0x1FFFFF00 | page;
	u32 setreg = 0x1F000100 | (reg << 16) | data;

#line 536
	ILI_INFO("setpage =  0x%X setreg = 0x%X\n", setpage, setreg);

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
			ILI_ERR("Enable ice mode failed before writing ddi reg\n");

	wdt = ilitek_tddi_ic_watch_dog_ctrl(0, 0);
	if (wdt)
		if (ilitek_tddi_ic_watch_dog_ctrl(1, 0) < 0)
			ILI_ERR("Disable WDT failed before writing ddi reg\n");

	ilitek_tddi_ic_wr_pack(0x1FFF9527);
	ilitek_tddi_ic_wr_pack(setpage);
	ilitek_tddi_ic_wr_pack(setreg);
	ilitek_tddi_ic_wr_pack(0x1FFF9500);

	if (wdt)
		if (ilitek_tddi_ic_watch_dog_ctrl(1, 1) < 0)
#line 558
			ILI_ERR("Enable WDT failed after writing ddi reg\n");

	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Disable ice mode failed after writing ddi reg\n");
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_rd_pack -- INCORPORATA, righe 509..524, nessun simbolo
 * ===========================================================================
 * La gemella di `ilitek_tddi_ic_wr_pack`, e a differenza sua NON ha un
 * simbolo in `oracolo/stock.map`: i suoi CINQUE `__func__` -- uno per
 * ciascuna delle cinque `printk` che contiene -- sono
 * "ilitek_tddi_ic_rd_pack"@0xffffff8009241af1, materializzato da
 * "912bc75a add"@0xffffff8008a588b8, "912bc421 add"@0xffffff8008a58980,
 * "912bc421 add"@0xffffff8008a589a8, "912bc421 add"@0xffffff8008a58a40 e
 * "912bc421 add"@0xffffff8008a58a70. Tutti e cinque cadono in
 * [0xffffff8008a58704, 0xffffff8008a58b58), cioe' dentro
 * `ilitek_tddi_ic_get_ddi_reg_onepage`, che e' l'unico chiamante.
 * (Le cinque sono lo scandaglio che conta: `ilitek_tddi_ic_check_protocol_ver`,
 * l'altra incorporata di questo file, ne ha davvero tre.)
 *
 * DUE DIFFERENZE MISURATE rispetto alla gemella, non scelte:
 *   - la condizione e' INVERTITA: "37080478 tbnz"@0xffffff8008a588e8 esce
 *     quando il bit 1 e' ACCESO, mentre `wr_pack` esce quando e' spento
 *     ("36080475 tbz"@0xffffff8008a58554);
 *   - il messaggio ha DUE spazi dopo "check"
 *     ("\x016ILITEK: (%s, %d): check  ok 0x4800A read 0x%X retry = %d\n"@0xffffff8009241b08),
 *     la
 *     gemella uno solo.
 * Il valore letto E' IL RITORNO: il chiamante lo usa subito
 * ("b94017e8 ldr"@0xffffff8008a58a7c, "39000128 strb"@0xffffff8008a58aa4).
 */
static u32 ilitek_tddi_ic_rd_pack(int packet)
{
	int retry = 100;
	u32 reg_data = 0;

	while (retry--) {
		if (ilitek_ice_mode_read(0x4800A, &reg_data, sizeof(u8)) < 0)
#line 509
			ILI_ERR("Read 0x4800A error\n");

		if ((reg_data & 0x02) == 0x02) {
			ILI_INFO("check  ok 0x4800A read 0x%X retry = %d\n",
#line 512
				 reg_data, retry);
			break;
		}
		mdelay(10);
	}

	if (retry <= 0)
#line 518
		ILI_INFO("check 0x4800A error read 0x%X\n", reg_data);

	if (ilitek_ice_mode_write(0x4800A, packet, 1) < 0)
		ILI_ERR("Write 0x2 at 0x4800A\n");

	if (ilitek_ice_mode_read(0x73016, &reg_data, sizeof(u8)) < 0)
		ILI_ERR("Read 0x73016 error\n");

	return reg_data;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_ddi_reg_onepage -- 0xffffff8008a58704, 1108 byte, 572..608
 * ===========================================================================
 * `setreg` ha una base diversa dalla gemella che scrive: 0x2F000100
 * ("52802017 mov"@0xffffff8008a58734 + "72a5e017 movk"@0xffffff8008a58748)
 * contro 0x1F000100, e NON contiene il dato -- solo il registro
 * ("33101e97 bfi"@0xffffff8008a58758).
 * Il formato ha UNO spazio dopo il primo `=`
 * (@0xffffff8009240874), mentre `set_ddi_reg_onepage` ne ha DUE.
 * Le quattro `wr_pack` sono 0x1FFF9527, `setpage`, 0x1FFF9487 e, dopo la
 * lettura, 0x1FFF9400 e 0x1FFF9500. La loro base comune 0x1FFF9400 sta in
 * w28 ("5292801c mov"@0xffffff8008a587e4 + "72a3fffc movk"@0xffffff8008a587e8)
 * e i tre scarti sono "11049f80 add"@0xffffff8008a587ec (+0x127),
 * "11021f80 add"@0xffffff8008a58800 (+0x87) e
 * "11040380 add"@0xffffff8008a58ab8 (+0x100).
 */
void ilitek_tddi_ic_get_ddi_reg_onepage(u8 page, u8 reg, u8 *data)
{
	int ice = idev->c712;
	int wdt;
	u32 setpage = 0x1FFFFF00 | page;
	u32 setreg = 0x2F000100 | (reg << 16);

#line 572
	ILI_INFO("setpage = 0x%X setreg = 0x%X\n", setpage, setreg);

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
			ILI_ERR("Enable ice mode failed before reading ddi reg\n");

	wdt = ilitek_tddi_ic_watch_dog_ctrl(0, 0);
	if (wdt)
		if (ilitek_tddi_ic_watch_dog_ctrl(1, 0) < 0)
			ILI_ERR("Disable WDT failed before reading ddi reg\n");

	ilitek_tddi_ic_wr_pack(0x1FFF9527);
	ilitek_tddi_ic_wr_pack(setpage);
	ilitek_tddi_ic_wr_pack(0x1FFF9487);

	if (ilitek_ice_mode_write(0x4800A, 0x02, 1) < 0)
#line 592
		ILI_ERR("Write 0x2 at 0x4800A\n");

	ilitek_tddi_ic_wr_pack(setreg);
	*data = ilitek_tddi_ic_rd_pack(0x02);

	ILI_INFO("check page = 0x%X, reg = 0x%X, read 0x%X\n", page, reg,
#line 595
		 *data);

	ilitek_tddi_ic_wr_pack(0x1FFF9400);
	ilitek_tddi_ic_wr_pack(0x1FFF9500);

	if (wdt)
		if (ilitek_tddi_ic_watch_dog_ctrl(1, 1) < 0)
#line 604
			ILI_ERR("Enable WDT failed after reading ddi reg\n");

	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Disable ice mode failed after reading ddi reg\n");
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_check_otp_prog_mode -- 0xffffff8008a58b58, 932 byte, 620..657
 * ===========================================================================
 * Cinque giri ("528000b8 mov"@0xffffff8008a58bc8 = 5,
 * "71000718 subs"@0xffffff8008a58e68) su un `do..while`: il corpo si entra
 * per caduta, senza guardia davanti.
 * `mdelay(1)` NON e' un ciclo: l'argomento e' costante e <= MAX_UDELAY_MS,
 * quindi la macro produce UNA sola `__const_udelay`
 * ("940ffb43 bl"@0xffffff8008a58d68).
 * I due valori letti sono estratti da UNA `ldp`
 * ("294057fb ldp"@0xffffff8008a58e40): `prog_done` sta a [sp,#0] e
 * `prog_mode` a [sp,#4], e il formato li stampa nell'ordine INVERSO
 * ("2a1503e3 mov"@0xffffff8008a58e50 = prog_mode in w3).
 * Non riabilita il WDT e non esce dalla modalita' ice: e' cosi' di fabbrica.
 */
void ilitek_tddi_ic_check_otp_prog_mode(void)
{
	int retry = 5;
	u32 prog_mode, prog_done;

	if (!idev->c629)
		return;

	if (ilitek_ice_mode_ctrl(1, 0) < 0) {
#line 620
		ILI_ERR("enter ice mode failed in otp\n");
		return;
	}

	if (ilitek_tddi_ic_watch_dog_ctrl(1, 0) < 0) {
		ILI_ERR("disable WDT failed in otp\n");
		return;
	}

	do {
		if (ilitek_ice_mode_write(0x43008, 0x80, 1) < 0)
			ILI_ERR("Write 0x80 at 0x43008 failed\n");

		if (ilitek_ice_mode_write(0x43030, 0x00, 1) < 0)
			ILI_ERR("Write 0x0 at 0x43030 failed\n");

		if (ilitek_ice_mode_write(0x4300C, 0x04, 1) < 0)
			ILI_ERR("Write 0x4 at 0x4300C failed\n");

		mdelay(1);

		if (ilitek_ice_mode_write(0x4300C, 0x04, 1) < 0)
#line 643
			ILI_ERR("Write 0x4 at 0x4300C\n");

		if (ilitek_ice_mode_read(0x43030, &prog_done, sizeof(u8)) < 0)
			ILI_ERR("Read prog_done error\n");

		if (ilitek_ice_mode_read(0x43008, &prog_mode, sizeof(u8)) < 0)
			ILI_ERR("Read prog_mode error\n");

		ILI_INFO("otp prog_mode = 0x%x, prog_done = 0x%x\n", prog_mode,
#line 651
			 prog_done);

		if (prog_mode == 0x80 && prog_done == 0x00)
			break;
	} while (--retry > 0);

	if (retry <= 0)
#line 657
		ILI_ERR("OTP Program mode error!\n");
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_pc_counter_forwdt -- 0xffffff8008a59278, 336 byte, righe 720..732
 * ===========================================================================
 * La gemella della precedente, con due differenze MISURATE: `idev->c712`
 * non e' tenuto in un locale ("b942c909 ldr"@0xffffff8008a592a0, w9 e' un
 * temporaneo) e la coda non chiama `ilitek_ice_mode_ctrl` ma azzera il
 * campo direttamente ("b902c91f str"@0xffffff8008a593a0).
 */
void ilitek_tddi_ic_get_pc_counter_forwdt(void)
{
	u32 pc = 0, latch = 0;
	u32 pc_addr = IC->c20;
	u32 latch_addr = IC->c24;

	if (!idev->c712)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
#line 720
			ILI_ERR("Enable ice mode failed while reading pc counter\n");

	if (ilitek_ice_mode_read(pc_addr, &pc, sizeof(u32)) < 0)
		ILI_ERR("Read pc conter error\n");

	if (ilitek_ice_mode_read(latch_addr, &latch, sizeof(u32)) < 0)
		ILI_ERR("Read pc conter error\n");

	idev->c228 = pc;
	idev->c232 = latch;
	ILI_ERR("read pc (addr: 0x%x) = 0x%x\n", pc_addr, idev->c228);
	ILI_ERR("read latch (addr: 0x%x) = 0x%x\n", latch_addr, idev->c232);

#line 732
	ILI_ERR("force Disable ice mode\n");
	idev->c712 = 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_check_int_stat -- 0xffffff8008a593c8, 268 byte, riga 740
 * ===========================================================================
 * La macro e' `wait_event_interruptible_timeout`: si riconosce dai quattro
 * pezzi che il binario porta in chiaro -- `init_wait_entry`
 * ("97db1149 bl"@0xffffff8008a593fc), `prepare_to_wait_event` con
 * TASK_INTERRUPTIBLE ("320003e2 orr"@0xffffff8008a59408 = 1),
 * `schedule_timeout` ("94107160 bl"@0xffffff8008a5942c) e `finish_wait`
 * ("97db11bf bl"@0xffffff8008a5946c). La coda d'attesa e' `idev->inq`
 * a +568 ("9108e100 add"@0xffffff8008a5940c = 0x238 = 568).
 * Il timeout e' 750 ("52805dd3 mov"@0xffffff8008a59420), e con CONFIG_HZ=250
 * `msecs_to_jiffies(3000)` = (3000+3)/4 = 750.
 */
int ilitek_tddi_ic_check_int_stat(void)
{
	if (!wait_event_interruptible_timeout(idev->inq, !idev->c732,
					      msecs_to_jiffies(3000))) {
#line 740
		ILI_ERR("Error! Interrupt for MP isn't received\n");
		idev->c732 = 0;
		return -1;
	}

	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_check_busy -- 0xffffff8008a594d4, 560 byte, righe 760..784
 * ===========================================================================
 * Il byte atteso e' 0x41 o 0x51 secondo `idev->c320`
 * ("52800835 mov"@0xffffff8008a59534, "52800a35 mov"@0xffffff8008a5952c),
 * e ogni altro valore e' -EINVAL ("128002a0 mov"@0xffffff8008a596a0 = -22).
 * I due byte del comando sono una `strh` di 0xf3f6
 * ("529e7ec9 mov"@0xffffff8008a59500 + "79000be9 strh"@0xffffff8008a59510),
 * cioe' cmd[0]=0xf6 e cmd[1]=0xf3. La seconda scrittura parte da `cmd+1`
 * ("b2400116 orr"@0xffffff8008a5955c = sp+4 | 1).
 * Le costanti nei due messaggi d'errore sono IMMEDIATI
 * ("52801ec3 mov"@0xffffff8008a59598 = 0xf6,
 *  "52801e64 mov"@0xffffff8008a5959c = 0xf3): il sorgente NON vi rilegge
 * `cmd[]`, che il compilatore non potrebbe piegare a costante perche'
 * l'indirizzo del vettore e' passato a una funzione opaca.
 */
int ilitek_tddi_ic_check_busy(int count, int delay)
{
	u8 cmd[2] = {0};
	u8 busy = 0, rby = 0;

	cmd[0] = 0xF6;
	cmd[1] = 0xF3;

	if (idev->c320 == 0)
		rby = 0x41;
	else if (idev->c320 == 1)
		rby = 0x51;
	else {
#line 760
		ILI_ERR("Unknown TP mode (0x%x)\n", idev->c320);
		return -EINVAL;
	}

	ILI_INFO("read byte = %x, delay = %d\n", rby, delay);

	do {
		if (idev->c776(cmd, sizeof(cmd)) < 0)
			ILI_ERR("Write %x,%x failed\n", 0xF6, 0xF3);

		if (idev->c776(cmd + 1, 1) < 0)
#line 770
			ILI_ERR("Write %x failed\n", 0xF3);

		if (idev->c784(&busy, 1) < 0)
#line 772
			ILI_ERR("Read check busy failed\n");

		ILI_DBG("busy = 0x%x\n", busy);

		if (busy == rby) {
			ILI_INFO("Check busy free\n");
			return 0;
		}

		mdelay(delay);
	} while (--count > 0);

	ILI_ERR("Check busy (0x%x) timeout !\n", busy);
	ilitek_tddi_ic_get_pc_counter();
	return -1;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_spi_speed_ctrl -- 0xffffff8008a58efc, 892 byte, righe 662..681
 * ===========================================================================
 * FUNZIONE MORTA. In tutta la sezione `.kernel` (5.159.350 parole) NON
 * esiste nessuna `b`/`bl` verso 0xffffff8008a58efc, e nessuna rilocazione
 * di `.rela.dyn` ha quell'addend. Esiste solo perche' non e' `static`.
 * NON C'E' NESSUN SITO DI CHIAMATA DA CUI LEGGERE LA FIRMA: quella qui
 * sotto e' DEDOTTA DAL SOLO PROLOGO, ed e' una deduzione, non una misura --
 *   - il parametro e' provato sul SOLO bit 0
 *     ("7200001f tst"@0xffffff8008a58f2c e "36000513 tbz"@0xffffff8008a58f98),
 *     che e' cio' che clang fa con un `bool`; con un `int` il confronto
 *     sarebbe sull'intera parola;
 *   - l'epilogo NON scrive w0 ("a9477bfd ldp"@0xffffff8008a59260 e seguenti
 *     fino a "d65f03c0 ret"@0xffffff8008a59270): il ritorno e' `void`.
 * Un `int` al posto del `bool` e un `int` di ritorno mai assegnato darebbero
 * lo stesso codice per il ritorno ma NON per il parametro.
 *
 * LA PRIMA `ilitek_ice_mode_write` COMPARE UNA SOLA VOLTA NEL BINARIO ED E'
 * DUE VOLTE NEL SORGENTE: e' identica nei due rami e il compilatore l'ha
 * sollevata sopra la diramazione ("d63f0100 blr"@0xffffff8008a58f8c precede
 * "36000513 tbz"@0xffffff8008a58f98), ma le due `printk` d'errore restano
 * distinte perche' portano numeri di riga diversi (666 e 675).
 */
void ilitek_tddi_ic_spi_speed_ctrl(bool enable)
{
	int ret = 0;

#line 662
	ILI_INFO("%s spi speed up\n", (enable ? "Enable" : "Disable"));

	if (enable) {
		ret = ilitek_ice_mode_write(0x063820, 0x00000101, 4);
		if (ret < 0)
#line 666
			ILI_ERR("Write 0x00000101 at 0x063820 failed\n");
		ret = ilitek_ice_mode_write(0x042c34, 0x00000008, 4);
		if (ret < 0)
			ILI_ERR("Write 0x00000008 at 0x042c34 failed\n");
		ret = ilitek_ice_mode_write(0x063820, 0x00000000, 4);
		if (ret < 0)
			ILI_ERR("Write 0x00000000 at 0x063820 failed\n");
	} else {
		ret = ilitek_ice_mode_write(0x063820, 0x00000101, 4);
		if (ret < 0)
#line 675
			ILI_ERR("Write 0x00000101 at 0x063820 failed\n");
		ret = ilitek_ice_mode_write(0x042c34, 0x00000000, 4);
		if (ret < 0)
			ILI_ERR("Write 0x00000000 at 0x042c34 failed\n");
		ret = ilitek_ice_mode_write(0x063820, 0x00000000, 4);
		if (ret < 0)
			ILI_ERR("Write 0x00000000 at 0x063820 failed\n");
	}
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_project_id -- 0xffffff8008a59704, 1648 byte, righe 796..837
 * ===========================================================================
 * LA SECONDA FUNZIONE MORTA del blocco, e la piu' grande: nessuna `b`/`bl`
 * la raggiunge in tutto il kernel. Anche qui LA FIRMA E' DEDOTTA DAL SOLO
 * PROLOGO, e la deduzione e' piu' ricca della precedente perche' il corpo
 * usa i due parametri:
 *   - x0 e' provato per NULL ("b40005c0 cbz"@0xffffff8008a5973c) e indicizzato
 *     A BYTE ("38356a68 strb"@0xffffff8008a59c44, cioe' [x19, x21]): e' un
 *     puntatore a byte;
 *   - w1 e' il limite di un ciclo con confronto CON SEGNO
 *     ("7100069f cmp"@0xffffff8008a59b44 + "5400088b b.lt"@0xffffff8008a59b48)
 *     ed e' stampato con `%d`: e' un `int`;
 *   - il ritorno e' -12 sul ramo NULL ("12800160 mov"@0xffffff8008a5980c) e
 *     0 su tutti gli altri: e' un `int`.
 * Che `buf` sia `u8 *` e non `char *` NON e' deciso dal binario: l'indice
 * a byte vale per entrambi. E' una SCELTA.
 *
 * `project_id` e' un VETTORE, e lo dice il formato
 * "\x016ILITEK: (%s, %d): project_id[%d] = 0x%x\n"@0xffffff8009241243.
 */
int ilitek_tddi_ic_get_project_id(u8 *buf, int size)
{
	int i = 0;
	int ice = idev->c712;
	u32 tmp;

	if (!buf) {
#line 796
		ILI_ERR("pdata is null\n");
		return -ENOMEM;
	}

	ILI_INFO("Read size = %d\n", size);

	if (!ice)
		if (ilitek_ice_mode_ctrl(1, 0) < 0)
			ILI_ERR("Enable ice mode failed while reading project id\n");

	if (ilitek_ice_mode_write(0x041000, 0x0, 1) < 0)
		ILI_ERR("Pull cs low failed\n");

	/* "52954aaa mov"@0xffffff8008a5986c = 0xaa55, "52800ccb mov"
	 * @0xffffff8008a59870 = 0x66, len+4 = 7 -> 3 byte di dato */
	if (ilitek_ice_mode_write(0x041004, 0x66AA55, 3) < 0)
#line 809
		ILI_ERR("Write key failed\n");

	if (ilitek_ice_mode_write(0x041008, 0x03, 1) < 0)
		ILI_ERR("Write 0x03 at 0x041008\n");

	if (ilitek_ice_mode_write(0x041008, 0x01, 1) < 0)
		ILI_ERR("Write address failed\n");

	if (ilitek_ice_mode_write(0x041008, 0xD0, 1) < 0)
#line 817
		ILI_ERR("Write address failed\n");

	if (ilitek_ice_mode_write(0x041008, 0x00, 1) < 0)
#line 819
		ILI_ERR("Write address failed\n");

	for (i = 0; i < size; i++) {
		if (ilitek_ice_mode_write(0x041008, 0xFF, 1) < 0)
			ILI_ERR("Write dummy failed\n");

		if (ilitek_ice_mode_read(0x041010, &tmp, sizeof(u8)) < 0)
#line 825
			ILI_ERR("Read project id error\n");

		buf[i] = tmp;
#line 827
		ILI_INFO("project_id[%d] = 0x%x\n", i, buf[i]);
	}

	ilitek_tddi_flash_clear_dma();

	if (ilitek_ice_mode_write(0x041000, 0x1, 1) < 0)
		ILI_ERR("Pull cs high\n");

	if (!ice)
		if (ilitek_ice_mode_ctrl(0, 0) < 0)
			ILI_ERR("Disable ice mode failed while reading project id\n");

	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_core_ver -- 0xffffff8008a59d74, 408 byte, righe 877..882
 * ===========================================================================
 * `u8 buf[10]`: la dimensione e' MISURATA dalle due istruzioni di
 * azzeramento, "f90007ff str"@0xffffff8008a59da0 (8 byte da [sp,#8]) e
 * "790023ff strh"@0xffffff8008a59d9c (2 byte da [sp,#16]): esattamente 10.
 * Con 8 basterebbe la `str`, con 9 servirebbe una `strb`.
 * La lunghezza della lettura viene dalla tabella dei protocolli,
 * "b9401921 ldr"@0xffffff8008a59e14 = +24.
 */
int ilitek_tddi_ic_get_core_ver(void)
{
	int ret = 0;
	u8 cmd[2] = {0};
	u8 buf[10] = {0};

	if (idev->c636) {
		buf[1] = idev->c452[68];
		buf[2] = idev->c452[69];
		buf[3] = idev->c452[70];
		goto out;
	}

	cmd[0] = 0xF6;
	cmd[1] = 0x23;

	if (idev->c776(cmd, 2) < 0) {
#line 859
		ILI_ERR("write core ver err\n");
		ret = -1;
		goto out;
	}

	if (idev->c776(cmd + 1, 1) < 0) {
		ILI_ERR("write core ver err\n");
		ret = -1;
		goto out;
	}

	if (idev->c784(buf,
		       ((struct ilitek_protocol_info *)idev->c56)->c24) < 0) {
#line 871
		ILI_ERR("i2c/spi read core ver err\n");
		ret = -1;
		goto out;
	}

	if (buf[0] != 0x23) {
		ILI_ERR("Invalid core ver\n");
		ret = -1;
	}

out:
	ILI_INFO("Core version = %d.%d.%d\n", buf[1], buf[2], buf[3]);
	IC->c52 = buf[1] << 16 | buf[2] << 8 | buf[3];
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_uart_ctrl -- 0xffffff8008a59f0c, 248 byte, righe 892..904
 * ===========================================================================
 * "12001c09 and"@0xffffff8008a59f24 + "7100093f cmp"@0xffffff8008a59f28:
 * il parametro e' confrontato ESTESO DA UN BYTE, cioe' e' un `u8`.
 * "52806808 mov"@0xffffff8008a59f8c + "79000be8 strh"@0xffffff8008a59f90
 * mettono 0x0340, cioe' cmd[0]=0x40 e cmd[1]=0x03.
 * "1a9f07e9 cset"@0xffffff8008a59fb8 converte a 0/1 prima di scrivere in
 * `idev->c630`: e' cio' che clang produce quando la DESTINAZIONE e' un
 * `bool`. `ilitek.h` la tiene `u8`, quindi qui serve un `!!` esplicito --
 * vedi il DELTA DI HEADER 3.
 */
void ilitek_tddi_fw_uart_ctrl(u8 ctrl)
{
	u8 cmd[4] = {0};

	if (ctrl > 1) {
#line 892
		ILI_INFO("Unknown cmd, ignore\n");
		return;
	}

	ILI_INFO("%s UART mode\n", (ctrl ? "Enable" : "Disable"));

	cmd[0] = 0x40;
	cmd[1] = 0x03;
	cmd[2] = 0x00;
	cmd[3] = ctrl;

	if (idev->c776(cmd, 4) < 0) {
		ILI_INFO("Write fw uart cmd failed\n");
		return;
	}

	idev->c630 = !!ctrl;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_fw_ver -- 0xffffff8008a5a004, 456 byte, righe 934..957
 * ===========================================================================
 * `char str[512]`: la dimensione e' MISURATA dalla pila. La cornice e'
 * "d108c3ff sub"@0xffffff8008a5a014 = 0x230 = 560 byte sotto `x29-32`;
 * `str` comincia a sp+24 ("910063e0 add"@0xffffff8008a5a178) e `buf`
 * comincia a x29-56 ("d100e3b4 sub"@0xffffff8008a5a020), cioe' a sp+536:
 * 536 - 24 = 512, e non resta un byte in mezzo.
 *
 * `IC->c48` e' i quattro byte di versione in ordine INVERTITO:
 * "b8401289 ldur"@0xffffff8008a5a164 legge i 4 byte da `buf+1` in un colpo
 * e "5ac00929 rev"@0xffffff8008a5a174 li rovescia.
 *
 * `fix_tp_proc_info` NON e' del driver: vedi la dichiarazione locale sopra.
 * Il secondo argomento e' il valore di ritorno della `sprintf`
 * ("2a0003e1 mov"@0xffffff8008a5a190).
 */
int ilitek_tddi_ic_get_fw_ver(void)
{
	int ret = 0;
	u8 cmd[2] = {0};
	u8 buf[10] = {0};
	char str[512];

	if (idev->c636) {
		buf[1] = idev->c452[48];
		buf[2] = idev->c452[49];
		buf[3] = idev->c452[50];
		buf[4] = idev->c452[51];
		goto out;
	}

	cmd[0] = 0xF6;
	cmd[1] = 0x21;

	if (idev->c776(cmd, 2) < 0) {
#line 934
		ILI_ERR("write firmware ver err\n");
		ret = -1;
		goto out;
	}

	if (idev->c776(cmd + 1, 1) < 0) {
		ILI_ERR("write firmware ver err\n");
		ret = -1;
		goto out;
	}

	if (idev->c784(buf,
		       ((struct ilitek_protocol_info *)idev->c56)->c4) < 0) {
#line 946
		ILI_ERR("i2c/spi read firmware ver err\n");
		ret = -1;
		goto out;
	}

	if (buf[0] != 0x21) {
		ILI_ERR("Invalid firmware ver\n");
		ret = -1;
	}

out:
	ILI_INFO("Firmware version = %d.%d.%d.%d\n", buf[1], buf[2], buf[3],
#line 957
		 buf[4]);
	IC->c48 = buf[1] << 24 | buf[2] << 16 | buf[3] << 8 | buf[4];

	fix_tp_proc_info(str,
			 sprintf(str,
				 "TP IC: ILITEK,TP MODULE: hongzhan,TP I2C ADR: 0x41,SW FirmWare: 0x%06X,Sample FirmWare: 0x%06X",
				 IC->c48, IC->c48));
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_panel_info -- 0xffffff8008a5a1cc, 536 byte, righe 987..1006
 * ===========================================================================
 * DUE ORDINI DI BYTE DIVERSI PER LO STESSO CAMPO, ed e' misurato, non
 * scelto: sul ramo con i valori gia' in memoria il binario fa
 * "33185d68 bfi"@0xffffff8008a5a244, cioe' `b1 | (b2 << 8)`; sul ramo che
 * legge dal chip fa "5ac00908 rev"@0xffffff8008a5a2e8 +
 * "53107d08 lsr"@0xffffff8008a5a2ec, cioe' `(b1 << 8) | b2`. E' un difetto
 * della fabbrica e si riproduce (regola 7).
 *
 * La risoluzione di ripiego e' UNA sola `str` da 4 byte
 * ("b900f52a str"@0xffffff8008a5a388) che copre +244 e +246 con 0x0924_0438
 * = 1080 e 2340.
 */
int ilitek_tddi_ic_get_panel_info(void)
{
	int ret = 0;
	u8 cmd = 0x29;
	u8 buf[10] = {0};
	/*
	 * DUE LOCALI, e la misura li impone (classe A2 alla rovescia):
	 * "a9432149 ldp"@0xffffff8008a5a208 legge `idev->c48` e `idev->c56`
	 * in un colpo, "39405113 ldrb"@0xffffff8008a5a214 prende la LUNGHEZZA
	 * come UN BYTE -- e' il locale a essere `u8`, non il campo, che le
	 * altre quattro letture prendono a 32 bit -- e "b9403536 ldr"
	 * @0xffffff8008a5a218 la versione. Tutti e due sopravvivono a tre
	 * chiamate opache in registri salvati.
	 */
	u32 ver = IC->c52;
	u8 len = ((struct ilitek_protocol_info *)idev->c56)->c20;

	/* "51000aa8 sub"@0xffffff8008a5a220 = 0x10402 - 2 */
	if (idev->c636 && ver > 0x010400) {
		buf[1] = idev->c452[16];
		buf[2] = idev->c452[17];
		buf[3] = idev->c452[18];
		buf[4] = idev->c452[19];
		idev->c244 = buf[1] | (buf[2] << 8);
		idev->c246 = buf[3] | (buf[4] << 8);
		goto out;
	}

	if (idev->c776(&cmd, sizeof(cmd)) < 0)
#line 987
		ILI_ERR("Write panel info error\n");

	/* "1a938121 csel"@0xffffff8008a5a29c */
	ret = idev->c784(buf, (ver > 0x010402) ? 6 : len);
	if (ret < 0)
#line 991
		ILI_ERR("Read panel info error\n");

	if (buf[0] != 0x29) {
		ILI_INFO("Invalid panel info, use default resolution\n");
		idev->c244 = 1080;
		idev->c246 = 2340;
		idev->c641 = 0;
	} else {
		idev->c244 = buf[1] << 8 | buf[2];
		idev->c246 = buf[3] << 8 | buf[4];
		idev->c641 = (IC->c52 > 0x010402) && buf[5];
		ILI_INFO("Transfer touch coordinate = %s\n",
#line 1002
			 idev->c641 ? "ON" : "OFF");
	}

out:
	ILI_INFO("Panel info: width = %d, height = %d\n", idev->c244,
#line 1006
		 idev->c246);
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_tp_info -- 0xffffff8008a5a3e4, 568 byte, righe 1034..1068
 * ===========================================================================
 * `u8 buf[20]`: 4 byte a [sp,#32] ("b90023ff str"@0xffffff8008a5a410) piu'
 * 16 a [sp,#16] ("a9017fff stp"@0xffffff8008a5a414) = 20 esatti.
 *
 * DUE COPPIE DI CAMPI RICEVONO LO STESSO BYTE: `buf[7]` e `buf[11]` vengono
 * entrambi da +464 ("39474109 ldrb"@0xffffff8008a5a46c, poi
 * "39005fe9 strb"@0xffffff8008a5a470 e "39006fe9 strb"@0xffffff8008a5a478),
 * `buf[8]` e `buf[12]` entrambi da +466 ("39474908 ldrb"@0xffffff8008a5a474,
 * poi "390063e8 strb"@0xffffff8008a5a47c e
 * "390073e8 strb"@0xffffff8008a5a480). Riprodotto com'e'.
 *
 * LA LETTURA E' UNA SOLA PER COPPIA, e la forma del sorgente lo deve dire:
 * `buf[11] = buf[7] = ...` da' una `ldrb` sola come il binario, mentre due
 * assegnazioni separate ne danno DUE -- l'indirizzo di `buf` sfugge alla
 * funzione (finisce in `idev->c784`), quindi la `strb` intermedia impedisce
 * al compilatore di riusare la lettura. E' la sola differenza fra un
 * `get_tp_info` da 568 byte e uno da 576. Vedi la CORREZIONE C1.
 */
int ilitek_tddi_ic_get_tp_info(void)
{
	int ret = 0;
	u8 cmd[2] = {0};
	u8 buf[20] = {0};

	/* "5280802a mov"@0xffffff8008a5a424 + "72a0002a movk"@0xffffff8008a5a428 */
	if (idev->c636 && IC->c52 >= 0x010401) {
		buf[1] = idev->c452[5];
		buf[2] = idev->c452[7];
		buf[3] = idev->c452[8];
		buf[4] = idev->c452[9];
		buf[5] = idev->c452[10];
		buf[6] = idev->c452[11];
		buf[11] = buf[7] = idev->c452[12];
		buf[12] = buf[8] = idev->c452[14];
		goto out;
	}

	cmd[0] = 0xF6;
	cmd[1] = 0x20;

	if (idev->c776(cmd, 2) < 0) {
#line 1034
		ILI_ERR("Write tp info error\n");
		ret = -1;
		goto out;
	}

	if (idev->c776(cmd + 1, 1) < 0) {
		ILI_ERR("Write tp info error\n");
		ret = -1;
		goto out;
	}

	if (idev->c784(buf,
		       ((struct ilitek_protocol_info *)idev->c56)->c12) < 0) {
#line 1046
		ILI_ERR("Read tp info error\n");
		ret = -1;
		goto out;
	}

	if (buf[0] != 0x20) {
		ILI_ERR("Invalid tp info\n");
		ret = -1;
	}

out:
	idev->c240 = buf[1];
	idev->c242 = buf[2];
	idev->c236 = buf[3] | (buf[4] << 8);
	idev->c238 = buf[5] | (buf[6] << 8);
	idev->c248 = buf[7];
	idev->c249 = buf[8];
	idev->c250 = buf[11];
	idev->c251 = buf[12];

	ILI_INFO("TP Info: min_x = %d, min_y = %d, max_x = %d, max_y = %d\n",
		 idev->c240, idev->c242, idev->c236, idev->c238);
	ILI_INFO("TP Info: xch = %d, ych = %d, stx = %d, srx = %d\n",
#line 1068
		 idev->c248, idev->c249, idev->c250, idev->c251);
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_protocl_ver -- 0xffffff8008a5a61c, 672 byte, righe 1129..1140
 * ===========================================================================
 * Contiene INCORPORATA `ilitek_tddi_ic_check_protocol_ver` (righe
 * 1077..1089), che nella mappa NON ha un simbolo proprio: i suoi tre
 * `__func__` sono "ilitek_tddi_ic_check_protocol_ver"@0xffffff8009241bd6.
 * SETTE delle otto voci sono confrontate SROTOLATE, una `adrp`+`add`
 * ciascuna, a passo 44: da "9115c14a add"@0xffffff8008a5a78c (+0x570) a
 * "9119e14a add"@0xffffff8008a5a804 (+0x678). L'OTTAVA (+0x6a4) NON e' mai
 * confrontata: e' il ripiego preso quando l'ultima `cmp` fallisce
 * ("540003e1 b.ne"@0xffffff8008a5a810 verso 0xffffff8008a5a88c, che stampa
 * il messaggio 'Not found a correct protocol version in list, use newest
 * version' -- gia' citato con il suo indirizzo, 0xffffff8009241c2a -- e poi
 * scrive la voce con
 * "911a9129 add"@0xffffff8008a5a8ac). Per questo il ciclo qui sotto va fino
 * a ARRAY_SIZE-1 e il ripiego usa [ARRAY_SIZE-1].
 * "same procotol version" -- `procotol`, non `protocol`: refuso di
 * fabbrica, si riproduce.
 */
static void ilitek_tddi_ic_check_protocol_ver(u32 pver)
{
	int i = 0;

	if (((struct ilitek_protocol_info *)idev->c56)->c0 == pver) {
#line 1077
		ILI_INFO("same procotol version, do nothing\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(protocol_info) - 1; i++) {
		if (protocol_info[i].c0 == pver) {
			idev->c56 = &protocol_info[i];
			ILI_INFO("update protocol version = %x\n",
#line 1084
				 ((struct ilitek_protocol_info *)idev->c56)->c0);
			return;
		}
	}

	ILI_INFO("Not found a correct protocol version in list, use newest version\n");
	idev->c56 = &protocol_info[ARRAY_SIZE(protocol_info) - 1];
}

int ilitek_tddi_ic_get_protocl_ver(void)
{
	int ret = 0;
	u32 ver;
	u8 cmd[2] = {0};
	u8 buf[10] = {0};

	if (idev->c636) {
		buf[1] = idev->c452[72];
		buf[2] = idev->c452[73];
		buf[3] = idev->c452[74];
		goto out;
	}

	cmd[0] = 0xF6;
	cmd[1] = 0x22;

	if (idev->c776(cmd, 2) < 0) {
		ILI_ERR("Write protocol version error\n");
		ret = -1;
		goto out;
	}

	if (idev->c776(cmd + 1, 1) < 0) {
		ILI_ERR("Write protocol version error\n");
		ret = -1;
		goto out;
	}

	if (idev->c784(buf,
		       ((struct ilitek_protocol_info *)idev->c56)->c8) < 0) {
#line 1123
		ILI_ERR("Read protocol version error\n");
		ret = -1;
		goto out;
	}

	if (buf[0] != 0x22) {
		ILI_ERR("Invalid protocol ver\n");
		ret = -1;
	}

out:
	ver = buf[1] << 16 | buf[2] << 8 | buf[3];
	ilitek_tddi_ic_check_protocol_ver(ver);

	ILI_INFO("Protocol version = %d.%d.%d\n",
		 ((struct ilitek_protocol_info *)idev->c56)->c0 >> 16,
		 (((struct ilitek_protocol_info *)idev->c56)->c0 >> 8) & 0xFF,
		 ((struct ilitek_protocol_info *)idev->c56)->c0 & 0xFF);
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_get_info -- 0xffffff8008a5a8bc, 436 byte, righe 1150..1176
 * ===========================================================================
 * `pid` E' UN LOCALE, e lo prova "b90017e9 str"@0xffffff8008a5a8e8: il
 * campo +8 e' copiato in uno slot di pila PRIMA del controllo su +712, e
 * l'indirizzo di quello slot ("910053e1 add"@0xffffff8008a5a8fc) e' cio'
 * che va a `ilitek_ice_mode_read`. Con `&IC->c8` il binario passerebbe
 * l'indirizzo vero del campo.
 * Le due `ldrb` seguite da `str w` ("3940a12a ldrb"@0xffffff8008a5a9e0 +
 * "b900292a str"@0xffffff8008a5a9e4) sono un `&= 0xFF` su un campo a 32
 * bit: clang restringe la lettura, non il campo.
 */
int ilitek_tddi_ic_get_info(void)
{
	int ret = 0;
	u32 pid = IC->c8;

	if (!idev->c712) {
#line 1150
		ILI_ERR("ice mode doesn't enable\n");
		return -1;
	}

	if (!pid) {
		ret = ilitek_ice_mode_read(IC->c12, &pid, sizeof(u32));
		if (ret < 0)
#line 1156
			ILI_ERR("Read chip id error\n");
	}

	ret = ilitek_ice_mode_read(IC->c32, &IC->c40, sizeof(u32));
	if (ret < 0)
#line 1159
		ILI_ERR("Read otp id error\n");

	ret = ilitek_ice_mode_read(IC->c36, &IC->c44, sizeof(u32));
	if (ret < 0)
#line 1161
		ILI_ERR("Read ana id error\n");

	IC->c8 = pid;
	IC->c2 = pid >> 16;
	IC->c0 = pid >> 8;
	IC->c1 = pid;
	IC->c40 &= 0xFF;
	IC->c44 &= 0xFF;

	ILI_INFO("CHIP: PID = %x, ID = %x, TYPE = %x, VER = %x, OTP = %x, ANA = %x\n",
#line 1176
		 IC->c8, IC->c2, IC->c0, IC->c1, IC->c40, IC->c44);

	return ilitek_tddi_ic_check_support(IC->c8, IC->c2);
}

/*
 * ===========================================================================
 * ilitek_tddi_ic_init -- 0xffffff8008a5aa70, 104 byte, 26 istruzioni
 * ===========================================================================
 * Tutte e 26 le istruzioni servono a riempire la struttura del chip e a
 * agganciarla a `idev`. I valori sono ricostruiti dagli immediati:
 * "f900092a str"@0xffffff8008a5aa94 e' UNA `str x` che copre +16 e +20
 * (0x0005100c e 0x00044008), "2903212b stp"@0xffffff8008a5aac4 copre +24 e
 * +28, "2904292c stp"@0xffffff8008a5aab0 copre +32 e +36, e
 * "a9032149 stp"@0xffffff8008a5aad0 scrive in `idev` sia +48 (questa
 * struttura) sia +56 (l'ULTIMA voce della tabella dei protocolli,
 * "911a9108 add"@0xffffff8008a5aacc = 0x6a4).
 */
void ilitek_tddi_ic_init(void)
{
	chip.c12 = 0x4009C;
	chip.c16 = 0x5100C;
	chip.c20 = 0x44008;
	chip.c24 = 0x51010;
	chip.c28 = 0x40050;
	chip.c32 = 0x400A0;
	chip.c36 = 0x400A4;
	chip.c68 = 1;

	idev->c48 = &chip;
	idev->c56 = &protocol_info[ARRAY_SIZE(protocol_info) - 1];
}
