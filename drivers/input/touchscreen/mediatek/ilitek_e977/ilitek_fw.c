// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_ILITEK_E977 -- il GRUPPO G, firmware e flash. Doogee S88 Pro.
 *
 * Confine del gruppo: 0xffffff8008a6714c..0xffffff8008a6b528, tredici
 * simboli di mappa, 17372 byte. Il confine e' quello di
 * `docs/bringup/rapporti/rapporto-ilitek-confine-e-unita.md` e la
 * ricognizione `scout-ilitek-G/RIASSUNTO.md` §1 lo ha rimisurato in modo
 * indipendente -- conteggio, somma dei byte, e la prova dei numeri di riga
 * su tutti e due i lati -- ottenendo lo stesso risultato.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA. Nessun sorgente
 * pubblico e' stato letto. Ogni riga derivata porta accanto la riga di
 * disassemblato che la giustifica, nella forma "<codifica> <mnemonico>"@0xINDIRIZZO
 * -- la stessa di `ilitek.h` e di `ilitek_bus.c`, quella che
 * `tools/verificaistruzioni.py` sa confrontare col binario.
 *
 * ===========================================================================
 * QUESTO E' UN LOTTO PARZIALE, E LO DICE PRIMA DI TUTTO IL RESTO
 * ===========================================================================
 * Delle tredici funzioni di mappa qui ce ne sono DODICI. Manca
 * `ilitek_tddi_fw_upgrade` (0xffffff8008a684e0, 8824 byte, il 50,8 % del
 * blocco) e con essa le NOVE funzioni che il compilatore le ha incorporato
 * dentro: `ilitek_tddi_fw_check_hex_hw_crc`, `ilitek_tddi_fw_check_ver`,
 * `ilitek_tddi_fw_flash_program`, `ilitek_tddi_fw_flash_upgrade`,
 * `ilitek_tddi_fw_update_block_info`, `ilitek_tddi_fw_ili_convert`,
 * `ilitek_tddi_fw_hex_convert`, `ilitek_tdd_fw_hex_open` (SI', "tdd": e' il
 * nome che sta nei byte a 0xffffff8009246... e una riscrittura "pulita" lo
 * normalizzerebbe in silenzio) e `ilitek_tddi_fw_upgrade` stessa.
 * Il residuo e' dichiarato per intero nel rapporto: 8824 byte, 2206
 * istruzioni, 60 `printk`.
 *
 * Cio' che c'e' e' un sotto-lotto COERENTE: i primitivi del flash (cs, key,
 * write-enable, poll-busy, clear-dma, int-flag, dma-write), le due letture
 * (dati e CRC hardware), i due scarichi su file (IRAM e flash), la
 * cancellazione, il CRC del file e la lettura delle informazioni del flash.
 * Nessuna di queste dodici chiama `ilitek_tddi_fw_upgrade`.
 *
 * ===========================================================================
 * COSA HA CAMBIATO LA REVISIONE DEL 2026-08-21, E COSA NON HA CAMBIATO
 * ===========================================================================
 * Una revisione indipendente ha trovato UN difetto di comportamento e
 * cinque di prova. Sono tutti verificati di nuovo e tutti corretti; le
 * correzioni sono firmate sul posto, ognuna accanto alla cosa che
 * correggeva, perche' un elenco di correzioni staccato dal punto che
 * correggono e' la classe C1.
 *
 * IL DIFETTO DI COMPORTAMENTO, uno solo: `kmalloc` dove la fabbrica ha
 * `kzalloc` in `ilitek_fw_dump_iram_data`. Le bandiere GFP erano l'unica
 * costante immediata non dichiarata di tutte e dodici le funzioni, ed era
 * invisibile a OGNI misura del lotto -- stessa dimensione, stessa
 * istruzione (`mov`+`movk`), e le due citazioni che stavano accanto
 * (la taglia e l'ordine) sono vere ma non riguardano le bandiere.
 * Corretto; la prova sta sul posto.
 *
 * I CINQUE DI PROVA: due citazioni su 270 portavano un indirizzo a cui
 * quella codifica non sta; il cappello di `ilitek_tddi_flash_poll_busy` era
 * un residuo di redazione che annunciava una forma di ciclo contraria al
 * codice; «i due siti di chiamata» di `ilitek_fw_dump_iram_data` erano uno;
 * «i sei globali» di `ilitek_tddi_fw_upgrade` erano sette; e
 * l'`--eccezione "Enable"` era motivata da un'affermazione falsa.
 * Piu' tre note di nome mancanti (`calc_crc32` e i suoi locali,
 * `ILI_DUMP_*_PATH`, `user` e `ice`) e due rinvii ciechi (G2b, G2c).
 *
 * UNA COSA DELLA REVISIONE E' STATA RESPINTA, con la prova: `pfw` NON e' un
 * nome inventato. Il binario lo scrive, a 0xffffff8009246333. Vedi il
 * cappello di `calc_crc32`.
 *
 * LA MISURA NON E' CAMBIATA DI UN BYTE, e va detto perche' e' il punto:
 * tutte e dodici le dimensioni sono identiche a prima e a dopo
 * (920/472/312/268/1092/220/1108/724/608/632/452/1788), cinque su dodici
 * combaciano ancora con la fabbrica, `intervallo.py 5 12` da' la stessa
 * riga. Il difetto vero era invisibile alla dimensione, ed e' esattamente
 * per questo che va corretto lo stesso. L'unica cosa che si e' mossa nel
 * `.o` sono DUE codifiche, che ora combaciano con la fabbrica:
 * "52901801 mov"@0xffffff8008a67260 e "72a02801 movk"@0xffffff8008a67268.
 *
 * ===========================================================================
 * COME SI RIVERIFICA
 * ===========================================================================
 *   ./venv/bin/python3 verificacitazioni.py ilitek_fw.c oracolo/stock.elf
 *   ./venv/bin/python3 verificaistruzioni.py ilitek_fw.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a6714c:0xffffff8008a6b528 \
 *       --intervallo 0xffffff8008a56d4c:0xffffff8008a57680 \
 *       --intervallo 0xffffff8008a550e0:0xffffff8008a55120 \
 *       --intervallo 0xffffff8008a5b5ec:0xffffff8008a5b630 \
 *       --intervallo 0xffffff8008a6d834:0xffffff8008a6d880 \
 *       --intervallo 0xffffff8008a55664:0xffffff8008a55d0c \
 *       --controfattuale "52801e02 mov"
 *
 * ESITO ATTESO, al 2026-08-21, dopo la revisione. I due comandi sopra sono
 * quelli, per intero: nessuna `--eccezione`, e non ne serve nessuna.
 *
 *   $ ./venv/bin/python3 verificacitazioni.py ilitek_fw.c oracolo/stock.elf
 *   [...]
 *     DEBOLE 0xffffff8009249687: FAIL -- verificata, ma occorre 26 volte nel binario: [...]
 *   letterali: 81   citati: 83   verificati: 83   probanti: 82   deboli: 1
 *   di cui verificate come messaggio assemblato dalla macro di log (coda + binario, non un singolo letterale C): 75
 *   di cui verificate come nome di funzione da __func__ (non un letterale scritto a mano): 3
 *   rinviate a verificaistruzioni.py (descrivono un'istruzione, non un letterale): 302 -- non contate qui, ne' come verificate ne' come mancanti
 *   soglia imposta: 81 citati richiesti (81 letterali - 0 eccezioni)
 *   exit=0
 *
 *   $ ./venv/bin/python3 verificaistruzioni.py ilitek_fw.c oracolo/stock.elf [...]
 *   [... una riga per citazione, piu' le tre righe `controfattuale` della
 *    forma dichiarata: NON sono ricopiate qui, perche' una riga di quel
 *    formato dentro il file si conta da se' e sposterebbe il totale ...]
 *   citazioni di istruzione trovate nel sorgente: 312 (312 a codifica, 0 ad indirizzo)
 *   confermate: 309   assenti: 0   mnemonico diverso: 0   controfattuali: 3
 *   operandi -- registro diverso: 0   immediato diverso: 0   entrambi/non confrontabili: 0
 *   exit=0
 *
 * (312 contro 302: le dieci di differenza sono citazioni NUDE, senza
 * `@0xINDIRIZZO` -- le tre occorrenze della controfattuale dichiarata e
 * sette codifiche nominate dentro i capoversi che spiegano un errore o un
 * alias. Solo le 302 ancorate hanno un indirizzo da controllare, ed e' cio'
 * che fa il terzo comando.)
 *
 * FINO ALLA REVISIONE DEL 2026-08-21 QUI C'ERA UN'AFFERMAZIONE FALSA, e va
 * scritto cosa era: diceva che `--eccezione "Enable"` copriva «l'unico
 * letterale del file che NON sta nel binario di fabbrica», e che questo era
 * «un fatto e non un limite dello strumento». La stringa "Enable\0" sta nel
 * binario di fabbrica 42 volte:
 *   python3 -c "d=open('oracolo/stock.elf','rb').read(); print(d.count(b'Enable\\x00'))"
 *   42
 * L'affermazione difendibile e' quella, piu' stretta, che sopravvive qui
 * sotto e nel commento di `ilitek_tddi_flash_protect`: NEL GRUPPO G non
 * compare nessun "Enable", perche' il ramo `enable == true` di
 * `ilitek_tddi_flash_protect` e' stato piegato via dal compilatore --
 * l'unico sito di chiamata passa una costante falsa. E l'opzione non
 * serviva comunque: `verificacitazioni.py` esce 0 anche senza, perche' la
 * soglia e' 81 letterali e le citazioni sono 83. Il file, per giunta,
 * prescriveva alla riga «COME SI RIVERIFICA» il comando SENZA
 * `--eccezione` e dichiarava l'esito CON: era anche una C1, la correzione
 * che non raggiunge l'artefatto derivato.
 *
 * La `--controfattuale "52801e02 mov"` e' la citazione della classe B8 al
 * contrario: e' la codifica che il NOSTRO compilatore produce dove la
 * fabbrica ha "321c0fe2 orr"@0xffffff8008a556b0, ed e' citata apposta in G8
 * come prova che i due compilatori non sono lo stesso. NON deve stare nel
 * binario di fabbrica, quindi va dichiarata.
 *
 * DOVE CADONO LE CITAZIONI, intervallo per intervallo (conteggio rifatto
 * dopo la revisione del 2026-08-21; somma 302, nessuna fuori):
 *   0xffffff8008a6714c..0xffffff8008a6b528  gruppo G                275
 *   0xffffff8008a56d4c..0xffffff8008a57680  gruppo D, ice_mode        7
 *   0xffffff8008a550e0..0xffffff8008a55120  ilitek_tddi_init          2
 *   0xffffff8008a5b5ec..0xffffff8008a5b630  dma_trigger_reg_setting   3
 *   0xffffff8008a6d834..0xffffff8008a6d880  ilitek_node_ioctl_write   4
 *   0xffffff8008a55664..0xffffff8008a55d0c  gruppo B                 11
 *
 * Il primo intervallo e' il gruppo G. Il secondo e' il gruppo D
 * (`ilitek_ice_mode_*`): sette citazioni di questo file ci cadono dentro, e
 * sono le prove delle firme. Il terzo, il quarto e il quinto sono SITI DI
 * CHIAMATA fuori dal gruppo, e sono prove che il corpo da solo non puo'
 * dare: il numero di parametri di `ilitek_tddi_flash_dma_write` e di
 * `ilitek_fw_dump_iram_data` (parametri che il corpo NON usa) e il fatto
 * che il ritorno di `ilitek_tddi_fw_read_flash_info` non e' letto.
 * Il sesto e' il GRUPPO B, `ilitek_tddi_interface_dev_init`: ci cade la
 * "321c0fe2 orr"@0xffffff8008a556b0 di G8 e, dalla revisione del
 * 2026-08-21, le dieci codifiche che provano due cose nuove -- che la
 * `struct i2c_driver` del driver e' allocata a runtime (e quindi che
 * nessuna `.exit` avrebbe un indirizzo statico da de-registrare) e che la
 * coppia "52901801 mov"+"72a02801 movk" e' la firma di `kzalloc` anche
 * fuori dal gruppo G.
 *
 * ===========================================================================
 * LE CITAZIONI: UN CONTROLLO CHE GLI STRUMENTI NON FANNO
 * ===========================================================================
 * `verificaistruzioni.py` cerca la CODIFICA dentro gli intervalli
 * dichiarati; l'`@0xINDIRIZZO` non lo confronta con niente. Lo dice da se',
 * ed e' la riga di riepilogo del comando qui sopra:
 *   citazioni di istruzione trovate nel sorgente: 312 (312 a codifica, 0 ad indirizzo)
 * Una citazione che porta la codifica giusta e l'indirizzo sbagliato passa
 * indenne, e piu' e' comune la codifica meno si nota. In questo file ne
 * erano passate DUE su 270 (revisione del 2026-08-21), e vanno scritte
 * SENZA la forma citabile, altrimenti il controllo qui sotto le ritrova:
 *   - 79400109 ldrh ancorata a 0xffffff8008a6ad60, dove sta invece la
 *     "6b09033f cmp"@0xffffff8008a6ad60 che la consuma; la `ldrh` vera e'
 *     quattro byte prima, "79400109 ldrh"@0xffffff8008a6ad5c, e quella
 *     codifica compare otto volte nella fascia;
 *   - 7100027f cmp ancorata a 0xffffff8008a6b488, dove sta invece la
 *     "71000673 subs"@0xffffff8008a6b488; la `cmp` vera e'
 *     "7100027f cmp"@0xffffff8008a6b4a8, e quella codifica compare quattro
 *     volte nella fascia.
 * Sono corrette nei rispettivi cappelli.
 * Il controllo mancante si fa cosi', ed e' un objdump della fascia intera
 * (0xffffff8008a536f8..0xffffff8008a7221c) confrontato riga per riga con le
 * citazioni del sorgente. NON e' uno strumento del progetto -- e' un
 * controllo ad hoc, e varrebbe la pena farlo diventare un'opzione di
 * `verificaistruzioni.py`:
 *   ./venv/bin/python3 /mnt/s88pro/kernel-stock/lavoro-rev-g/checkcit.py ilitek_fw.c
 * Esito, sul file com'e' adesso:
 *   citazioni di istruzione ancorate a un indirizzo: 302
 *   corrette (codifica E mnemonico all'indirizzo dichiarato): 302
 *   fuori dal disassemblato disponibile: 0
 *   SBAGLIATE: 0
 * Sul file com'era prima della revisione: 270 ancorate, 268 corrette,
 * 2 SBAGLIATE -- le due qui sopra.
 *
 * La misura si fa dal `.o` vero:
 *   aarch64-linux-android-nm --print-size out-ilitek-g/.../ilitek_fw.o | grep -iE " [tT] "
 *
 * ===========================================================================
 * LA MISURA, DAL `.o` VERO (2026-08-21)
 * ===========================================================================
 * `aarch64-linux-android-nm --print-size out-ilitek-g/.../ilitek_fw.o`,
 * compilato con `KCFLAGS=-Werror`, zero avvisi.
 *
 *   funzione                          fabbrica  nostro  scarto
 *   ilitek_tddi_fw_iram_read          incorp.   incorp.     --
 *   ilitek_fw_dump_iram_data              912      920      +8
 *   ilitek_tddi_flash_poll_busy           472      472       0
 *   ilitek_tddi_flash_clear_dma           312      312       0
 *   ilitek_tddi_flash_read_int_flag       264      268      +4
 *   ilitek_tddi_flash_dma_write          1092     1092       0
 *   ilitek_tddi_flash_write_enable        220      220       0
 *   ilitek_tddi_fw_read_hw_crc           1112     1108      -4
 *   ilitek_tddi_fw_read_flash_data        712      724     +12
 *   ilitek_tddi_fw_dump_flash_data        608      608       0
 *   ilitek_tddi_flash_protect         incorp.   incorp.     --
 *   ilitek_tddi_fw_flash_erase            612      632     +20
 *   calc_crc32                        incorp.   incorp.     --
 *   ilitek_fw_calc_file_crc               448      452      +4
 *   ilitek_tddi_fw_read_flash_info       1784     1788      +4
 *   TOTALE (12 simboli di mappa)         8548     8596     +48
 *
 * CINQUE SU DODICI IDENTICHE. `intervallo.py 5 12` dice
 * «5 su 12 = 41.7%   IC95% Clopper-Pearson [15.2%; 72.3%]   contiene
 * 77.10%: NO -- la misura discrimina»: su dodici unita' l'intervallo e'
 * largo quaranta punti, ma NON copre il 77,10 % del ramo, quindi questo
 * file sta sotto la media e va detto prima del numero, non dopo.
 *
 * La stessa misura in byte dice un'altra cosa, ed e' onesto darle
 * entrambe: 48 byte di scarto su 8548, cioe' 12 istruzioni su 2137
 * (0,56 %). Nessuna funzione sbaglia di piu' di cinque istruzioni.
 *
 * ===========================================================================
 * L'ATTRIBUZIONE DI OGNI DIVERGENZA -- 12 istruzioni, tutte spiegate
 * ===========================================================================
 * Fatta con l'istogramma dei mnemonici fabbrica-contro-nostro, funzione per
 * funzione, dopo aver normalizzato le coppie di alias che NON cambiano la
 * dimensione (vedi G8).
 *
 * ilitek_fw_dump_iram_data       +2 istr: due `b` incondizionati in piu' e
 *   la condizione di fine ciclo invertita (`b.lt` di fabbrica -> `b.ge`
 *   nostro). E' G9.
 * ilitek_tddi_flash_read_int_flag +1 istr: una `mov w3, wzr`. Il nostro
 *   clang dimostra che sul ramo di scadenza `flag` vale zero e la
 *   materializza; quello di fabbrica lascia w3 dalla `ldr` precedente e
 *   arriva alla stampa senza nulla in mezzo. Nessuna differenza di
 *   comportamento.
 * ilitek_tddi_fw_read_hw_crc     -1 istr: la riscrittura dell'induttore del
 *   ciclo di attesa. Di fabbrica `sub`+`cmp`+`b.gt` con l'induttore spostato
 *   di uno (parte da 501) e un `sub` fuori dal ciclo; da noi
 *   `sub`+`cmn #1`+`b.ne` e niente fuori. Stessa forma sorgente, due
 *   canonicalizzazioni diverse: la NOSTRA e' piu' corta di una istruzione.
 * ilitek_tddi_fw_read_flash_data +3 istr: due `b` di G9 (con `b.ls` ->
 *   `b.hi`) e una `str` di riversamento in piu'.
 * ilitek_tddi_fw_flash_erase     +5 istr: due `b` di G9 (con `b.cc` ->
 *   `b.eq`) e tre per il RICALCOLO di `&ilitek_fw_blk[i]` dopo la `printk`
 *   della riga 797 (`adrp`+`add`+`add`). Di fabbrica quell'indirizzo resta
 *   in x24, che e' un registro salvato dal chiamato, e la lettura successiva
 *   e' una `ldr [x24,#8]!` pre-indicizzata. E' allocazione dei registri:
 *   provato a estrarre `struct ilitek_fw_blk *b = &ilitek_fw_blk[i];` in un
 *   locale, la misura NON cambia (632 in entrambi i casi), quindi la forma
 *   sorgente non e' la causa.
 * ilitek_fw_calc_file_crc        +1 istr: un `b` di G9 (con `b.cc` ->
 *   `b.eq`).
 * ilitek_tddi_fw_read_flash_info +1 istr: una `and w3, w24, #0xff`. Di
 *   fabbrica il primo dei quattro byte letti e' caricato con `ldrb` --
 *   "394013f5 ldrb"@0xffffff8008a6ab18 -- e la larghezza e' decisa dalla
 *   lettura; da noi e' `ldr w` e la maschera arriva dopo, appena prima della
 *   catena di confronti. Il valore e' lo stesso.
 *
 * Non resta nessun residuo inspiegato: 2+1-1+3+5+1+1 = 12.
 *
 * ===========================================================================
 * IL CICLO SCRIVI -> MISURA -> CORREGGI: LE QUATTRO CORREZIONI CHE HA TROVATO
 * ===========================================================================
 * Sono qui perche' ognuna e' una misura che poteva fallire e non e' fallita.
 *
 * 1. `ilitek_fw_dump_iram_data`, 880 byte contro 912. Le 8 istruzioni
 *    mancanti erano la SECONDA COPIA della coda: il binario ha tre
 *    `bl ilitek_ice_mode_ctrl` e nove `bl printk`, la nostra prima stesura
 *    con una coda sola e un operatore ternario ne aveva due e sei. Con le
 *    due code separate e la sola scrittura di `idev->c444` condivisa: 920.
 * 2. `ilitek_tddi_flash_poll_busy`, 488 byte contro 472. `timer-- > 0`
 *    contro `--timer >= 0`: la stessa condizione, ma la prima costringe
 *    clang a tenere il vecchio e il nuovo valore in due registri, e costa
 *    una `mov`, un salto e una coppia `stp`/`ldp`. Con la seconda: 472,
 *    esatta.
 * 3. `ilitek_tddi_fw_read_flash_info`, 1728 byte contro 1784. Mancavano 14
 *    istruzioni perche' i quattro assegnamenti a `idev` erano DOPO il ciclo
 *    di ricerca invece che DENTRO: fuori, il compilatore ricalcola
 *    l'indirizzo dall'indice; dentro, tiene vivo `&tab[i].c2` dalla
 *    `ldrh` del confronto, che e' cio' che
 *    "79400109 ldrh"@0xffffff8008a6ad5c e la coda condivisa a
 *    0xffffff8008a6addc mostrano. Con gli assegnamenti dentro: 1788.
 *    (L'indirizzo era sbagliato di quattro byte fino alla revisione del
 *    2026-08-21: a 0xffffff8008a6ad60 c'e' la "6b09033f cmp" che CONSUMA
 *    la `ldrh`, non la `ldrh`. Quella codifica compare otto volte
 *    nella fascia, quindi `verificaistruzioni.py` -- che confronta la
 *    codifica e non l'indirizzo, «274 a codifica, 0 ad indirizzo» --
 *    la confermava lo stesso. Vedi la sezione COME SI RIVERIFICA.)
 * 4. `ilitek_tddi_fw_flash_erase`, 1052 byte contro 612, e
 *    `ilitek_tddi_flash_poll_busy` sparita dal `.o`. Con un solo sito di
 *    chiamata (l'altro e' in `ilitek_tddi_fw_upgrade`, che non c'e') clang
 *    la incorpora; di fabbrica non lo fa. Vedi G2c.
 *
 * ===========================================================================
 * LA `.exit`: NON C'E', E IL CONTROLLO E' NEGATIVO SU TUTTO IL DRIVER
 * ===========================================================================
 * `oracolo/stock.map` finisce a `_einittext` (0xffffff80093a8518): tutto
 * cio' che sta oltre -- la `.exit.text` -- e' invisibile a ogni strumento
 * che parta dalla mappa. La zona e' stata scandita due volte, e la seconda
 * (revisione del 2026-08-21) ha allargato il perimetro e aggiunto tre
 * controlli.
 *
 * PERIMETRO: `.kernel` (vaddr 0xffffff8008080000, off 0x240, size
 * 0x13ae6d8) PIU' `.kernel2` (vaddr 0xffffff8009843ae8, off 0x13ae918, size
 * 0x217d24), tutte le parole a 4 byte oltre `_einittext`: 686.008
 * istruzioni, 1664 `bl`. Su tutte:
 *   - ZERO `bl` nella fascia ILITEK 0xffffff8008a536f8..0xffffff8008a7221c;
 *   - ZERO `b` (chiamata di coda) nella stessa fascia;
 *   - ZERO `adrp` verso 0xffffff800a0fc000, 0xffffff800a0ff000 o
 *     0xffffff8009989000, cioe' verso una qualunque delle pagine DATI che il
 *     driver usa;
 *   - ZERO `adrp` verso una qualunque pagina di CODICE della fascia.
 * Delle 1664 `bl`, 211 vanno a una funzione di de-registrazione
 * (`i2c_del_driver`, `tpd_driver_remove`, `platform_driver_unregister`,
 * `class_destroy`, `misc_deregister`, `driver_unregister`, `free_irq`,
 * `remove_proc_entry`, ...): nessuna sta nella fascia, e nessuna riceve in
 * x0 un indirizzo che il codice della fascia registri.
 *
 * E QUI STA LA RAGIONE, che e' piu' forte del conteggio: NON C'E' NESSUNA
 * STRUTTURA STATICA DA DE-REGISTRARE. L'unica registrazione di tutta la
 * fascia e' "9400d3a9 bl"@0xffffff8008a55714 verso `i2c_register_driver`,
 * dentro `ilitek_tddi_interface_dev_init`, e la `struct i2c_driver` che le
 * passa in x1 e' ALLOCATA A RUNTIME:
 *   "321c0fe2 orr"@0xffffff8008a556b0    w2 = 0xf0 = 240 byte
 *   "97dfe57d bl"@0xffffff8008a556b8     kmem_cache_alloc_trace
 *   "aa0003e1 mov"@0xffffff8008a556fc    x1 = il blocco appena allocato
 *   "aa1f03e0 mov"@0xffffff8008a55710    x0 = NULL (l'owner)
 * Una `.exit` non avrebbe nessun indirizzo statico da passare a
 * `i2c_del_driver`, e infatti non ce n'e' nessuna.
 *
 * Quindi il driver ILITEK non ha nessuna funzione `.exit` -- non il gruppo
 * G, non gli altri. Il controllo era su tutto il driver e non solo su questo
 * blocco perche' il gruppo G non registra niente in proprio: non ha
 * `i2c_driver`, ne' `platform_driver`, ne' `class`.
 *
 * ===========================================================================
 * LE DIVERGENZE APERTE DI QUESTO FILE
 * ===========================================================================
 * G1. I `__LINE__`. I numeri di fabbrica sono noti uno per uno (sono
 *     annotati sopra ogni `printk` come "riga NNN di fabbrica") e questo
 *     file NON li riproduce, per la stessa ragione di `ilitek_bus.c` §B1:
 *     imbottire di righe vuote un file ancora da completare sposta tutto al
 *     primo cambio. E' una divergenza reale e INVISIBILE alla dimensione --
 *     `mov w2,#imm16` e' una istruzione per qualunque riga.
 * G2. `ilitek_fw_blk` e `ilitek_flashtab` NON sono `static` qui, e di
 *     fabbrica `ilitek_fw_blk` quasi certamente lo era. La ragione e'
 *     misurabile e viene dal lotto parziale: l'unico scrittore di
 *     `ilitek_fw_blk` e' `ilitek_tddi_fw_upgrade`, che non c'e'. Con
 *     `static`, clang dimostra che l'array e' tutto zero e CANCELLA i corpi
 *     di `ilitek_tddi_fw_flash_erase` e `ilitek_fw_calc_file_crc`
 *     (`ilitek_fw_blk[i].c12 == 0` sempre vero). Quando il gruppo G sara'
 *     completo va rimesso `static`.
 *     Per `ilitek_flashtab` invece la NON-staticita' e' MISURATA, non
 *     scelta: vedi il commento sopra la tabella.
 * G2b. `ilitek_tddi_fw_flash_erase` e `ilitek_fw_calc_file_crc` NON sono
 *     `static` qui, e di fabbrica lo sono: nella mappa hanno la `t`
 *     minuscola (0xffffff8008a6a758 e 0xffffff8008a6b0b4). La ragione e'
 *     la stessa del lotto parziale: tutti i loro siti di chiamata --
 *     tre per la prima (0xffffff8008a69cd4, 0xffffff8008a6a1b4,
 *     0xffffff8008a6a228) e due per la seconda (0xffffff8008a6969c,
 *     0xffffff8008a6a6c0) -- stanno dentro `ilitek_tddi_fw_upgrade`, che
 *     non c'e'. Con `static` e zero chiamanti clang le cancellerebbe. Va
 *     rimesso `static` a gruppo G completo.
 * G2c. `ilitek_tddi_flash_poll_busy` porta un `noinline` CHE DI FABBRICA
 *     NON C'E'. Di fabbrica ha DUE siti di chiamata
 *     ("9400050d bl"@0xffffff8008a69f1c in `ilitek_tddi_fw_upgrade` e
 *     "94000289 bl"@0xffffff8008a6a92c in `ilitek_tddi_fw_flash_erase`) e
 *     clang non la incorpora; con il solo secondo -- l'unico che resta in
 *     questo lotto -- la incorpora, e `ilitek_tddi_fw_flash_erase` passa
 *     da 612 a 1052 byte mentre questa sparisce dal `.o`. Il `noinline`
 *     ripristina la forma di fabbrica e va tolto a gruppo G completo.
 *     (G2b e G2c erano rinviate quattro volte dal corpo del file --
 *     righe del commento di `ilitek_tddi_flash_poll_busy`, di
 *     `ilitek_tddi_fw_flash_erase`, di `ilitek_fw_calc_file_crc` e della
 *     correzione 4 -- e non stavano in questo elenco: rinvii ciechi, e
 *     l'elenco e' esattamente cio' che si legge al merge per sapere che
 *     cosa annullare. Trovate dalla revisione del 2026-08-21.)
 * G3. `calc_crc32` non ha nessun `printk`, quindi non lascia ne' nome ne'
 *     numero di riga: la sua POSIZIONE nel file di fabbrica non e'
 *     osservabile, e qui e' messa subito prima del suo unico chiamante di
 *     questo lotto. Che sia una funzione separata e' invece misurato --
 *     il corpo e' srotolato otto volte identico in `ilitek_fw_calc_file_crc`
 *     e la stessa forma ricompare in `ilitek_tddi_fw_upgrade`.
 * G4. Le FIRME delle funzioni incorporate (`ilitek_tddi_fw_iram_read`,
 *     `ilitek_tddi_flash_protect`, `calc_crc32`) non sono osservabili: il
 *     compilatore le ha fuse e la loro ABI non compare nel binario. Cio'
 *     che e' misurato e' quali valori del chiamante il corpo usa; la
 *     ripartizione fra parametri e locali e' una SCELTA, segnata caso per
 *     caso.
 * G5. `mdelay(1)` contro `udelay(1000)`. Il binario porta
 *     "940fb188 bl"@0xffffff8008a6b454 verso `__const_udelay` con
 *     "52912b00 mov"@0xffffff8008a6b44c + "72a00820 movk"@0xffffff8008a6b450,
 *     cioe' 0x418958 = 1000 * 0x10c7. `mdelay(1)` e `udelay(1000)` danno la
 *     STESSA istruzione perche' `mdelay` con argomento costante <= 5 si
 *     espande in `udelay(n*1000)`. Quale delle due stesse nel sorgente di
 *     fabbrica il binario NON lo dice.
 * G6. Le due soglie di `ilitek_tddi_fw_flash_erase`
 *     ("530d7ca8 lsr"@0xffffff8008a6a7b8 con "7100391f cmp"@0xffffff8008a6a7bc,
 *     "530c7c88 lsr"@0xffffff8008a6a7c4 con "7100711f cmp"@0xffffff8008a6a7c8).
 *     Sono scorrimenti VERI del sorgente, non un'ottimizzazione di
 *     confronto: 15<<13 = 0x1E000 e 29<<12 = 0x1D000 sono entrambe immediate
 *     codificabili in una sola `cmp #imm12, lsl #12`, quindi il compilatore
 *     non avrebbe avuto ragione di emettere due istruzioni al posto di una.
 *     Qui sono trascritte come divisioni per 8192 e 4096, che e' la forma
 *     che le riproduce; che di fabbrica fossero `>>` o `/` non e' deciso.
 * G7. `%lu` con un valore a 32 bit. `ilitek_tddi_flash_clear_dma` e
 *     `ilitek_tddi_flash_dma_write` stampano "Write %lu at %x failed" e il
 *     binario carica il primo argomento con una istruzione a REGISTRO W
 *     "32101fe3 orr"@0xffffff8008a67520 e "32081fe3 orr"@0xffffff8008a67558.
 *     Su AArch64 una `orr wN, wzr, #imm` azzera i 32 bit alti di xN, quindi
 *     una costante `unsigned long` che sta in 32 bit produce esattamente
 *     quella codifica: la forma `0xff0000UL` riproduce il binario E passa
 *     `-Wformat`. Non e' quindi il difetto B6 di `ilitek_bus.c` (un
 *     argomento MANCANTE): l'argomento c'e' ed e' della larghezza giusta.
 * G8. IL NOSTRO CLANG NON E' QUELLO DELLA FABBRICA, e si vede in due modi
 *     SISTEMATICI e senza costo in byte. Vanno scritti perche' chiunque
 *     confronti le CODIFICHE (e non le dimensioni) del nostro `.o` con
 *     quelle del binario troverebbe centinaia di differenze e le
 *     scambierebbe per difetti.
 *     (a) Una costante a 32 bit che sia una immediata logica valida:
 *         la fabbrica emette `orr wN, wzr, #imm`, noi `movz wN, #imm`.
 *         In `ilitek_tddi_fw_read_flash_info` sono 33 istruzioni su 446.
 *         Non e' una novita' di questo file: `ilitek_bus.o`, gia' accettato,
 *         produce "52801e02 mov w2, #0xf0" dove il binario ha
 *         "321c0fe2 orr"@0xffffff8008a556b0 -- la stessa costante 240 che
 *         il cappello di `ilitek_bus.c` cita.
 *     (b) Il record di frame. La fabbrica mette `stp x29,x30` in CIMA al
 *         telaio e indirizza i locali con `str [sp,#N]`; noi lo mettiamo in
 *         basso e usiamo `stur [x29,#-N]`. Stesso numero di istruzioni.
 *     Le citazioni di questo file restano valide perche' citano il BINARIO
 *     DI FABBRICA, che e' cio' che `verificaistruzioni.py` confronta: e'
 *     esattamente la classe di difetto B8 al contrario -- il rischio sarebbe
 *     citare le codifiche del NOSTRO oggetto, e qui non se ne cita nessuna.
 * G9. IL PIAZZAMENTO DEI BLOCCHI DEI CICLI. Su quattro cicli su sei il
 *     nostro compilatore mette il blocco di incremento PRIMA della testa del
 *     ciclo, e paga un `b` incondizionato per entrarci (e a volte un secondo
 *     per il blocco che di fabbrica cadeva nell'incremento per contiguita').
 *     Costa 8 dei 48 byte di scarto. NON dipende dalla forma sorgente:
 *     provato a riscrivere il ciclo di `ilitek_tddi_fw_iram_read` da `for` a
 *     `while` con gli incrementi in coda al corpo, la misura non cambia di
 *     un byte (920 in entrambi i casi).
 *
 * ===========================================================================
 * IL DELTA DI HEADER (da NON fare qui -- lo faccia il lotto di merge)
 * ===========================================================================
 * H1. `struct ilitek_tddi_dev`: il campo `c452[75]` di `ilitek.h` dice
 *     "finche' non sara' letto il gruppo G resta un blocco di byte". Il
 *     gruppo G legge +500 come `u32` -- ma SOLO dentro
 *     `ilitek_tddi_fw_upgrade`, che questo lotto non scrive. Nessun delta
 *     da qui, quindi; la nota va tolta dall'header solo quando quella
 *     funzione ci sara'.
 * H2. `void *c48` va tipizzato: `ilitek_tddi_fw_read_hw_crc` ne legge un
 *     `u32` a +56 ("b9403904 ldr"@0xffffff8008a67b9c dopo
 *     "f9401908 ldr"@0xffffff8008a67b98) e lo usa come limite superiore
 *     della lunghezza. Qui e' letto attraverso un tipo LOCALE dichiarato
 *     apposta, come fa gia' `ilitek_bus.c` §B3 per lo stesso campo.
 * H3. `c544`/`c546`/`c548`/`c552` hanno un NOME dal binario -- "Flash MID",
 *     "Flash DEV_ID", "Flash program page", "Flash sector", dalle stringhe
 *     di formato a 0xffffff8009246666, 0xffffff800924669d e
 *     0xffffff80092466ca. `ilitek.h` li tiene `c544`... ed e' il lotto che
 *     rinomina l'header a doverli cambiare, non questo.
 * H4. Le funzioni del gruppo D che questo file chiama (`ilitek_ice_mode_*`)
 *     dovranno stare in `ilitek.h`; qui sono dichiarate localmente, come fa
 *     `ilitek_bus.c` per il gruppo E.
 * H5. `ilitek_flashtab` e `ilitek_fw_blk` sono globali del driver e
 *     dovranno essere dichiarate nell'header condiviso: `ilitek_fw_blk` e'
 *     scritta da `ilitek_tddi_fw_upgrade` e letta anche dal gruppo H
 *     (`ilitek_node_*`). Definite qui perche' qui stanno le funzioni che le
 *     leggono.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fcntl.h>
#include <linux/firmware.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#include "ilitek.h"

/*
 * ===========================================================================
 * LE FUNZIONI DEL GRUPPO D CHE QUESTO FILE CHIAMA
 * ===========================================================================
 * Sono in un'unita' non ancora scritta: qui ci sono solo i prototipi. NON
 * SONO STUB -- dichiararle e lasciarle indefinite fa fallire il LINK, che e'
 * l'esito onesto (regola 6 del progetto). Undici chiamate uscenti su undici
 * di questo blocco vanno al gruppo D o al gruppo A.
 *
 * Le firme vengono dai siti di chiamata E dal prologo del chiamato:
 *
 *  - `ilitek_ice_mode_write`: w0 indirizzo, w1 dato, w2 lunghezza. Il
 *    chiamato spezza w0 in tre byte ("53087c0a lsr"@0xffffff8008a570d8,
 *    "53107c0b lsr"@0xffffff8008a570dc) e confronta w2 con 1
 *    ("7100045f cmp"@0xffffff8008a570e0).
 *  - `ilitek_ice_mode_read`: w0 indirizzo, x1 puntatore a `u32`
 *    (il chiamato azzera il proprio temporaneo con
 *    "b90007ff str"@0xffffff8008a56f0c), w2 lunghezza.
 *  - `ilitek_ice_mode_bit_mask_write`: w0 indirizzo, w1 maschera, w2 valore.
 *    Il chiamato fa "0a15028a and"@0xffffff8008a56d98 (valore & maschera) e
 *    "0a350108 bic"@0xffffff8008a56d9c (letto &~ maschera), che decide quale
 *    argomento e' la maschera.
 *  - `ilitek_ice_mode_ctrl`: w0 e w1. Il chiamato prova il solo bit 0 di w0
 *    ("7200029f tst"@0xffffff8008a571fc), che e' la larghezza di un `bool`.
 *    Del secondo argomento questo blocco passa sempre zero: la sua
 *    larghezza NON e' misurata qui.
 */
int ilitek_ice_mode_write(u32 addr, u32 dato, int len);
int ilitek_ice_mode_read(u32 addr, u32 *dato, int len);
int ilitek_ice_mode_bit_mask_write(u32 addr, u32 maschera, u32 valore);
int ilitek_ice_mode_ctrl(bool enable, bool mcu);

/*
 * Il tipo puntato da `idev->c48`. `ilitek.h` lo tiene `void *` perche'
 * nessuno dei gruppi letti finora lo tipava; `ilitek_tddi_fw_read_hw_crc`
 * ne legge UN campo e uno solo:
 *   "f9401908 ldr"@0xffffff8008a67b98   x8 = idev->c48
 *   "b9403904 ldr"@0xffffff8008a67b9c   w4 = *(u32 *)(x8 + 56)
 * e lo confronta senza segno con la lunghezza
 * ("6b01009f cmp"@0xffffff8008a67ba0 + "54000122 b.cs"@0xffffff8008a67ba4).
 * Larghezza 4, senza segno: e' tutto cio' che il binario dice. Gli altri 56
 * byte sono un buco DICHIARATO ignoto, non un riempimento misurato: sono
 * l'offset, non una struttura letta.  Vedi H2.
 */
struct ilitek_g_c48 {
	u8 __ignoto_0[48];
	/* +48 32 bit senza segno. La versione corrente del firmware:
	 * "b9403104 ldr"@0xffffff8008a69ad8 la stampa accanto a quella nuova
	 * e "b9403108 ldr"@0xffffff8008a69af0 la confronta, nel formato
	 * "\x016ILITEK: (%s, %d): New FW ver = 0x%x, Current FW ver = 0x%x\n"@0xffffff8009246fae.
	 * AGGIUNTO DAL SECONDO LOTTO: prima nessuna funzione scritta lo
	 * leggeva. */
	u32 c48;
	/* +52 32 bit senza segno. Confrontato con 0x010402
	 * ("b9403529 ldr"@0xffffff8008a69864 + "6b0a013f cmp"@0xffffff8008a69868
	 * + "540003c9 b.ls"@0xffffff8008a6986c) per decidere `idev->c641`.
	 * AGGIUNTO DAL SECONDO LOTTO. */
	u32 c52;
	/* +56 32 bit senza segno. Limite massimo della lunghezza che
	 * `ilitek_tddi_fw_read_hw_crc` accetta; il formato a
	 * 0xffffff8009245e56 lo chiama "max count". */
	u32 c56;
};

/*
 * ===========================================================================
 * LA TABELLA DEL FLASH: 8 voci da 16 byte, di fabbrica a 0xffffff8009989480
 * ===========================================================================
 * Il PASSO e' nel binario, non dedotto:
 *   "f00078e8 adrp"@0xffffff8008a6add0  x8 = pagina 0xffffff8009989000
 *   "91120108 add"@0xffffff8008a6add4   x8 += 0x480
 *   "8b091108 add"@0xffffff8008a6add8   x8 += indice << 4     <<< 16 byte
 * e i confronti a cascata di `ilitek_tddi_fw_read_flash_info` pongono
 * l'indice a 0,1,...,7: OTTO voci. `.rela.dyn` non ha nessuna rilocazione
 * dentro la tabella, quindi qui non ci sono puntatori nascosti e i byte
 * valgono quello che si legge.
 *
 * I NOMI dei primi due campi vengono dal BINARIO e non da una scelta: il
 * formato "\x016ILITEK: (%s, %d): Flash MID = %x, Flash DEV_ID = %x\n"
 * @0xffffff8009246666 e' stampato con w3 e w4 che sono ESATTAMENTE le due
 * `ldrh` della tabella ("79490103 ldrh"@0xffffff8008a6ac68 per +0 e
 * "79400104 ldrh"@0xffffff8008a6adcc per +2). Per +8 e +12 il nome passa per
 * il campo del dispositivo in cui sono copiati ("\x016ILITEK: (%s, %d): Flash program page = %d\n"@0xffffff800924669d
 * e "\x016ILITEK: (%s, %d): Flash sector = %d\n"@0xffffff80092466ca): e' una
 * inferenza di UN passo, e va letta come tale. Gli identificatori restano
 * `c<offset>` perche' e' l'offset che le citazioni verificano.
 *
 * LA TABELLA NON E' `static`, ED E' MISURATO. `ilitek_tddi_fw_read_flash_info`
 * srotola per intero la ricerca sulle otto voci -- l'indice e' quindi noto
 * al compilatore in ogni copia -- eppure emette LETTURE VERE:
 *   "79490103 ldrh"@0xffffff8008a6ac68   voce 0, campo +0
 *   "b9400909 ldr"@0xffffff8008a6ade0    campo +8
 *   "b9400108 ldr"@0xffffff8008a6adf4    campo +12
 * Con una `static` inizializzata e mai scritta clang la marca costante e
 * piega quelle letture in immediate (i campi +8 e +12 valgono 256 e 4096 in
 * tutte e otto le voci). Non lo fa: la tabella ha collegamento esterno.
 */
struct ilitek_flash_tab {
	/* +0  16 bit. "Flash MID" (nome DAL BINARIO, vedi sopra). */
	u16 c0;
	/* +2  16 bit. "Flash DEV_ID" (nome DAL BINARIO). */
	u16 c2;
	/* +4  32 bit. MAI LETTO dal gruppo G: nessuna istruzione del blocco
	 * ha questo offset. Il valore e' quello dei byte, e cosa significhi
	 * non e' deciso da qui. */
	u32 c4;
	/* +8  32 bit. Copiato in `idev->c548` ("b9022549 str"@0xffffff8008a6adf0),
	 * che il formato a 0xffffff800924669d chiama "Flash program page". */
	u32 c8;
	/* +12 32 bit. Copiato in `idev->c552` ("b9022948 str"@0xffffff8008a6ae08),
	 * che il formato a 0xffffff80092466ca chiama "Flash sector". */
	u32 c12;
};

/*
 * I valori sono i byte letti a 0xffffff8009989480 (16 per riga, come il
 * dump della ricognizione §5.1). La voce 0 ha MID e DEV_ID a zero ed e' il
 * RIPIEGO: quando nessuna combina, il codice stampa "Not found flash id in
 * tab, use default" e rientra nella coda comune leggendo la voce 0
 * ("d00078e8 adrp"@0xffffff8008a6b094 + "91120108 add"@0xffffff8008a6b098).
 */
/* NOME SCELTO: il binario non nomina ne' la tabella ne' il suo tipo. Cio'
 * che nomina sono due dei cinque campi (vedi sopra). */
struct ilitek_flash_tab ilitek_flashtab[8] = {
	{ 0x0000, 0x0000, 0x00040000, 0x100, 0x1000 },
	{ 0x00ef, 0x6011, 0x00020000, 0x100, 0x1000 },
	{ 0x00ef, 0x6012, 0x00040000, 0x100, 0x1000 },
	{ 0x00c8, 0x6012, 0x00040000, 0x100, 0x1000 },
	{ 0x00c8, 0x6013, 0x00080000, 0x100, 0x1000 },
	{ 0x0085, 0x6013, 0x00400000, 0x100, 0x1000 },
	{ 0x00c2, 0x2812, 0x00040000, 0x100, 0x1000 },
	{ 0x001c, 0x3812, 0x00040000, 0x100, 0x1000 },
};

/*
 * ===========================================================================
 * LA TABELLA DEI BLOCCHI DI FIRMWARE: 7 voci da 32 byte, a 0xffffff800a0ff5b8
 * ===========================================================================
 * Base e passo dal binario:
 *   "d000b4a8 adrp"@0xffffff8008a69f90   pagina 0xffffff800a0ff000
 *   "9116e108 add"@0xffffff8008a69f94    + 0x5b8              <<< base
 *   "8b141509 add"@0xffffff8008a69f98    + indice << 5        <<< passo 32
 * L'ESTENSIONE la consegna l'azzeramento, senza doverla indovinare: 14
 * `stp xzr, xzr` a passo 16 da [x8] a [x8,#208]
 * (0xffffff8008a688d4..0xffffff8008a68908), cioe' 224 byte = 7 voci. E 224
 * byte dopo la base comincia esattamente il byte "Block Num" a
 * 0xffffff800a0ff698: due misure indipendenti che combaciano.
 *
 * Il conteggio 7 e' confermato una terza volta dai DUE cicli di questo
 * lotto: "f1001e7f cmp"@0xffffff8008a6a98c e "f1001e7f cmp"@0xffffff8008a6b20c
 * sono entrambi `cmp x19, #7`.
 *
 * NON E' `static` QUI, e non e' una misura: vedi la divergenza G2.
 */
struct ilitek_fw_blk {
	/* +0  8 byte, puntatore a stringa costante. Scritto solo da
	 * `ilitek_tddi_fw_update_block_info`, che non e' in questo lotto. */
	const char *c0;
	/* +8  32 bit. Indirizzo iniziale del blocco: e' l'estremo inferiore
	 * del ciclo di `ilitek_tddi_fw_flash_erase`
	 * ("b8408f04 ldr"@0xffffff8008a6a7b4) e il primo argomento del CRC in
	 * `ilitek_fw_calc_file_crc` ("b9400949 ldr"@0xffffff8008a6b100). */
	u32 c8;
	/* +12 32 bit. Estremo superiore. La GUARDIA di entrambi i cicli e'
	 * `c12 == 0` ("34000ec5 cbz"@0xffffff8008a6a7b0,
	 * "34000868 cbz"@0xffffff8008a6b0fc). */
	u32 c12;
	/* +16 32 bit. Lunghezza: `ilitek_fw_calc_file_crc` calcola il CRC su
	 * c8..c8+c16-4 ("b940114a ldr"@0xffffff8008a6b104,
	 * "5100116b sub"@0xffffff8008a6b10c). */
	u32 c16;
	/* +20 32 bit. MAI toccato dal gruppo G. */
	u32 c20;
	/* +24 32 bit. Scritto solo da `ilitek_tddi_fw_upgrade`. */
	u32 c24;
	/* +28 un byte. Scritto solo da `ilitek_tddi_fw_upgrade`. */
	u8 c28;
	/* +29..31 MAI toccati dal gruppo G. Sono riempimento fino a 32, che e'
	 * il passo misurato. */
	u8 __ignoto_29[3];
};

/* NOME SCELTO: il binario non nomina ne' l'array ne' il suo tipo. Nomina
 * pero' il byte che gli sta subito dopo, "Block Num": il formato a
 * 0xffffff8009246d95 -- che appartiene a `ilitek_tddi_fw_upgrade` e quindi
 * NON e' citabile da questo file, perche' nessun letterale di questo file
 * ne e' la coda -- lo nomina insieme a "star_addr" (scritto proprio cosi'
 * nel binario, senza la `t`) e a "end_addr". E' quel byte che fissa la fine
 * dell'array a 224 byte. */
static struct ilitek_fw_blk ilitek_fw_blk[7];

/*
 * ===========================================================================
 * COSA QUESTO FILE NON DICHIARA, E PERCHE'
 * ===========================================================================
 * I SETTE globali che `ilitek_tddi_fw_upgrade` usa e che nessuna delle
 * dodici funzioni scritte tocca -- 0xffffff800a0ff5a8 e 0xffffff800a0ff5b0
 * (i due puntatori ai buffer di firmware; il primo e' quello che il binario
 * chiama "pfw", vedi sotto), 0xffffff800a0ff698 ("Block Num"),
 * 0xffffff800a0ff69c ("star_addr", scritto proprio cosi' nel binario, senza
 * la `t`), 0xffffff800a0ff6a0 ("end_addr"), 0xffffff800a0ff6a4
 * ("New FW ver") e 0xffffff800a0ff6a8 -- NON sono dichiarati qui.
 * Dichiararli senza la funzione che li usa vorrebbe dire scegliere un tipo
 * che nessuna riga di questo lotto misura.
 *
 * FINO ALLA REVISIONE DEL 2026-08-21 QUI NE ERANO ELENCATI SEI, E IL
 * SETTIMO -- 0xffffff800a0ff6a8 -- E' PROPRIO QUELLO CHE PORTA
 * L'AVVERTENZA. `scout-ilitek-G/RIASSUNTO.md` §6.1 lo isola come «l'unico
 * caso che merita un dubbio»: i soli valori che il blocco gli assegna sono
 * 0 ("b906a91f str"@0xffffff8008a6a434), 174
 * ("528015c9 mov"@0xffffff8008a6a6a0 + "b906a909 str"@0xffffff8008a6a6a4) e
 * 175 ("7102bedf cmp"@0xffffff8008a68be0 + "b906a916 str"@0xffffff8008a68bec),
 * che stanno tutti in un byte -- ma la LETTURA e' a 32 bit pieni
 * ("b946a908 ldr"@0xffffff8008a68c48, seguita da
 * "7102bd1f cmp"@0xffffff8008a68c4c). Chi scrivera' il lotto successivo NON
 * lo dichiari `static u8`: o e' `int`, o e' scritto da un'altra unita' di
 * traduzione. Il conto sette e' rifatto qui con una scansione ADRP->pagina
 * 0xffffff800a0ff000 seguita da ADD/LDR/STR su tutta la sezione `.kernel`,
 * non su una finestra.
 * Lo stesso vale per il campo +4 della tabella flash e per gli offset +20 e
 * +29..31 della voce di blocco: sono buchi VERI, non riempimento, e restano
 * `c<offset>` con il commento che dice quel che si sa.
 */

/*
 * I due percorsi che il driver usa a runtime. I VALORI sono letterali del
 * binario, e le due citazioni qui sotto li ancorano. I due IDENTIFICATORI
 * `ILI_DUMP_IRAM_PATH` e `ILI_DUMP_FLASH_PATH` sono invece NOMI SCELTI: il
 * binario non li contiene in nessuna forma, e che di fabbrica i percorsi
 * fossero due macro, due costanti o due letterali scritti in linea il
 * binario non lo dice. (Fino alla revisione del 2026-08-21 questo commento
 * diceva «Sono letterali del binario, non nomi scelti», che letto come e'
 * scritto nega una scelta che invece c'e'.)
 */
/* "/sdcard/iram_dump"@0xffffff8009245b8a */
#define ILI_DUMP_IRAM_PATH	"/sdcard/iram_dump"
/* "/sdcard/flash_dump"@0xffffff8009246288 */
#define ILI_DUMP_FLASH_PATH	"/sdcard/flash_dump"

/*
 * ===========================================================================
 * ilitek_tddi_fw_iram_read -- INCORPORATA in ilitek_fw_dump_iram_data
 * ===========================================================================
 * Che sia una funzione separata e' MISURATO, non supposto: i suoi tre
 * `printk` portano un `__func__` che non e' quello del contenitore --
 * "ilitek_tddi_fw_iram_read"@0xffffff8009246737 contro
 * "ilitek_fw_dump_iram_data"@0xffffff8009245b08 -- e occupa le righe di
 * fabbrica 136..145, disgiunte dalle 161..206 del contenitore.
 *
 * G4: essendo incorporata, la sua ABI NON compare nel binario. Cio' che il
 * corpo usa e' misurato -- DUE valori distinti del chiamante, `end - start`
 * (estremo del ciclo, del modulo e del divisore della percentuale) e
 * `end - start + 1` (il limite del troncamento dell'ultima lettura) --
 * e questa e' la parametrizzazione minima che li fornisce entrambi. Che di
 * fabbrica fossero due parametri, o un parametro piu' un ricalcolo, il
 * binario non lo dice.
 *
 * TUTTI I CONFRONTI E LE DIVISIONI SONO CON SEGNO, e questo decide i tipi:
 *   "5400006d b.le"@0xffffff8008a672c0    troncamento
 *   "1ad50f48 sdiv"@0xffffff8008a672c4    modulo con segno
 *   "1ada0d23 sdiv"@0xffffff8008a6731c    percentuale con segno
 *   "54fffb2b b.lt"@0xffffff8008a67354    fine ciclo
 * Con `u32` verrebbero `b.ls`, `udiv`, `udiv`, `b.cc`.
 */
static int ilitek_tddi_fw_iram_read(u8 *buf, int start, int end, int len)
{
	int i, addr = 0, r_len = 0x1000;
	/* "b90017ff str"@0xffffff8008a6729c -- azzerati insieme, quattro
	 * byte in una `str wzr`: e' l'inizializzatore, ed e' FUORI dal ciclo */
	u8 cmd[4] = {0};

	for (i = start; i < end; i += r_len, addr += r_len) {
		/* "0b150288 add"@0xffffff8008a672b8 + "6b19011f cmp"@0xffffff8008a672bc */
		if ((i + r_len) > len)
			/* "1b15e915 msub"@0xffffff8008a672c8 */
			r_len = end % r_len;

		/* "528004bc mov"@0xffffff8008a672ac -- 0x25 e' il comando, e sta
		 * in cmd[0]: "390053fc strb"@0xffffff8008a672dc scrive a [sp,#20] */
		cmd[0] = 0x25;
		/* "53107e88 lsr"@0xffffff8008a672d0 + "39005fe8 strb"@0xffffff8008a672d4 */
		cmd[3] = (i & 0x00FF0000) >> 16;
		/* "53087e88 lsr"@0xffffff8008a672d8 + "39005be8 strb"@0xffffff8008a672e0 */
		cmd[2] = (i & 0x0000FF00) >> 8;
		/* "390057f4 strb"@0xffffff8008a672e4 */
		cmd[1] = (i & 0x000000FF);

		/* "f9418528 ldr"@0xffffff8008a672e8 = idev->c776, chiamato con
		 * "d63f0100 blr"@0xffffff8008a672f4; il ramo d'errore e'
		 * "37f80a20 tbnz"@0xffffff8008a672f8, cioe' il solo bit di segno */
		if (idev->c776(cmd, 4) < 0) {
			/* riga 136 di fabbrica;
			 * "\x013ILITEK: (%s, %d): Failed to write iram data\n"@0xffffff8009246750 */
#line 136
			ILI_ERR("Failed to write iram data\n");
			return -ENODEV;
		}

		/* "8b3bc260 add"@0xffffff8008a67300 -- buf + addr, indice ESTESO
		 * CON SEGNO (`sxtw`), che e' la seconda prova che `addr` e' `int`;
		 * "f9418908 ldr"@0xffffff8008a67308 = idev->c784 */
		if (idev->c784(buf + addr, r_len) < 0) {
			/* riga 141 di fabbrica;
			 * "\x013ILITEK: (%s, %d): Failed to Read iram data\n"@0xffffff800924677f */
#line 141
			ILI_ERR("Failed to Read iram data\n");
			return -ENODEV;
		}

		/* "1b167f69 mul"@0xffffff8008a67318 (per 100) +
		 * "1ada0d23 sdiv"@0xffffff8008a6731c +
		 * "b901bd03 str"@0xffffff8008a67320 */
		idev->c444 = (addr * 100) / end;
		/* riga 145 di fabbrica. Protetta da
		 * "396562e8 ldrb"@0xffffff8008a67324 + "34000108 cbz"@0xffffff8008a67328,
		 * quindi ILI_DBG e non ILI_INFO. Il '%' finale e' un ARGOMENTO,
		 * "528004a4 mov"@0xffffff8008a67338 = 0x25 in w4, e il formato
		 * @0xffffff80092467ad finisce con "%d%c" SENZA \n. */
		/* "\x016ILITEK: (%s, %d): Reading iram data .... %d%c"@0xffffff80092467ad */
#line 145
		ILI_DBG("Reading iram data .... %d%c", idev->c444, '%');
	}

	return 0;
}

/*
 * ===========================================================================
 * ilitek_fw_dump_iram_data -- 0xffffff8008a6714c, 912 byte
 * ===========================================================================
 * TRE parametri, e il terzo il corpo NON lo usa. Non e' una deduzione dal
 * corpo (che non lo puo' dire) ma dal SITO DI CHIAMATA del gruppo H:
 *   "29408680 ldp"@0xffffff8008a6d870    w0 e w1 dalla richiesta
 *   "320003e2 orr"@0xffffff8008a6d874    w2 = 1
 *   "97ffe635 bl"@0xffffff8008a6d878
 * Il valore passato e' 0 oppure 1, che e' la larghezza di un `bool`; il
 * nome no.
 *
 * IL RITORNO E' `void`: nessun percorso materializza w0. La coda comune
 * (0xffffff8008a67214) scrive -1 o 100 in `idev->c444` e poi va
 * all'epilogo senza toccare w0, e l'UNICO sito di chiamata non lo legge.
 *
 * IL SITO E' UNO SOLO, e il numero viene da una scansione di tutta la
 * sezione `.kernel` -- 0x13ae6d8 byte, ogni `bl` e ogni `b` decodificati
 * dai byte, nessuna finestra: "97ffe635 bl"@0xffffff8008a6d878, dentro
 * `ilitek_node_ioctl_write` (+0xf74). Sono DUE i siti della funzione
 * SORELLA `ilitek_tddi_fw_dump_flash_data` (0xffffff8008a6d83c in
 * `ilitek_node_ioctl_write` e 0xffffff8008a6e9a0 in `ilitek_node_ioctl`),
 * ed e' da li' che questo cappello, fino alla revisione del 2026-08-21,
 * aveva preso il numero: diceva «i due siti di chiamata del gruppo H».
 * La conclusione (`void`) non cambia -- il sito unico davvero non legge
 * w0 -- ma la prova, come era scritta, non reggeva.
 */
void ilitek_fw_dump_iram_data(u32 start, u32 end, bool mcu)
{
	struct file *f = NULL;
	u8 *fw_buf = NULL;
	int ret = 0, len, stato;
	loff_t pos = 0;
	mm_segment_t old_fs;

	/* "b901bd1f str"@0xffffff8008a67194 -- azzerato PRIMA di entrare in
	 * modalita' ICE */
	idev->c444 = 0;

	ret = ilitek_ice_mode_ctrl(true, false);
	if (ret < 0) {
		/* riga 161 di fabbrica;
		 * "\x013ILITEK: (%s, %d): Enable ice mode failed\n"@0xffffff8009245adc */
#line 161
		ILI_ERR("Enable ice mode failed\n");
		goto out;
	}

	/* "4b14027a sub"@0xffffff8008a671a0 + "11000759 add"@0xffffff8008a671a4 */
	len = end - start + 1;
	/* "7140a33f cmp"@0xffffff8008a671a8 -- 0x28 << 12 = 0x28000, e il
	 * confronto e' CON SEGNO ("5400058d b.le"@0xffffff8008a671ac) */
	if (len > 0x28000) {
		/* riga 169 di fabbrica;
		 * "\x013ILITEK: (%s, %d): len is larger than buffer, abort\n"@0xffffff8009245b21 */
#line 169
		ILI_ERR("len is larger than buffer, abort\n");
		ret = -ENOMEM;
		goto out;
	}

	/* "52900000 mov"@0xffffff8008a6725c + "72a00040 movk"@0xffffff8008a67264
	 * = 0x28000. La chiamata e' a `kmalloc_order_trace`
	 * ("97defe4e bl"@0xffffff8008a67270) con ordine 6
	 * ("321f07e2 orr"@0xffffff8008a6726c): e' cio' in cui `kmalloc` si
	 * espande quando la taglia supera KMALLOC_MAX_CACHE_SIZE, non una
	 * funzione chiamata a mano.
	 *
	 * E' `kzalloc`, NON `kmalloc`, e lo decidono LE BANDIERE -- l'unica
	 * istruzione di questa chiamata che nessuna misura di dimensione
	 * poteva denunciare, perche' e' `mov`+`movk` in tutti e due i casi:
	 *   "52901801 mov"@0xffffff8008a67260    w1 = 0x80c0
	 *   "72a02801 movk"@0xffffff8008a67268   w1 |= 0x140 << 16
	 * cioe' w1 = 0x14080c0. `GFP_KERNEL` da solo vale 0x14000c0
	 * (`include/linux/gfp.h`: `___GFP_DIRECT_RECLAIM 0x400000u`,
	 * `___GFP_KSWAPD_RECLAIM 0x1000000u`, `___GFP_IO 0x40u`,
	 * `___GFP_FS 0x80u`), e 0x14080c0 - 0x14000c0 = 0x8000 =
	 * `___GFP_ZERO`: esattamente cio' che
	 * `kzalloc(size, flags)` = `kmalloc(size, flags | __GFP_ZERO)` aggiunge.
	 * La stessa coppia di codifiche, con lo stesso valore e lo stesso
	 * significato, sta nel gruppo B davanti all'allocazione della
	 * `struct i2c_driver`: "52901801 mov"@0xffffff8008a556a4 +
	 * "72a02801 movk"@0xffffff8008a556ac prima di
	 * "97dfe57d bl"@0xffffff8008a556b8 verso `kmem_cache_alloc_trace`.
	 * Il `memset(fw_buf, 0xFF, 0x28000)` che segue rende i due
	 * comportamenti indistinguibili a runtime; il sorgente pero' deve
	 * riprodurre il binario, non il suo effetto. */
	fw_buf = kzalloc(0x28000, GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a67278 poi "b4000c13 cbz"@0xffffff8008a67280:
	 * IS_ERR PRIMA del confronto con NULL, in quest'ordine */
	if (IS_ERR(fw_buf) || fw_buf == NULL) {
		/* riga 176 di fabbrica;
		 * "\x013ILITEK: (%s, %d): Failed to allocate update_buf\n"@0xffffff8009245b57 */
#line 176
		ILI_ERR("Failed to allocate update_buf\n");
		ret = -ENOMEM;
		goto out;
	}

	/* "32001fe1 orr"@0xffffff8008a67288 -- 0xff, non 0 */
	memset(fw_buf, 0xFF, 0x28000);

	ret = ilitek_tddi_fw_iram_read(fw_buf, start, end - start, len);
	if (ret < 0)
		goto out;

	/* "52804821 mov"@0xffffff8008a67360 = 0x241 = O_WRONLY|O_CREAT|O_TRUNC
	 * e "52805082 mov"@0xffffff8008a67364 = 0x284 = 644 DECIMALE. Il modo
	 * e' scritto senza lo zero iniziale nel sorgente di fabbrica: 644
	 * decimale e' 01204 ottale, non 0644. E' un difetto della fabbrica e
	 * si riproduce (regola 7). */
	f = filp_open(ILI_DUMP_IRAM_PATH, O_WRONLY | O_CREAT | O_TRUNC, 644);
	if (IS_ERR(f) || f == NULL) {
		/* riga 190 di fabbrica. "%ld" con PTR_ERR: l'argomento c'e',
		 * "aa1403e3 mov"@0xffffff8008a67430 mette x20 (il puntatore) in
		 * x3, ed e' un valore a 64 bit. */
		/* "\x013ILITEK: (%s, %d): Failed to open the file at %ld.\n"@0xffffff8009245b9c */
#line 190
		ILI_ERR("Failed to open the file at %ld.\n", PTR_ERR(f));
		ret = -ENOENT;
		goto out;
	}

	/* Le DUE `set_fs` di fila sono nel binario, non un refuso:
	 * "f90006b6 str"@0xffffff8008a67388 e "f90006b6 str"@0xffffff8008a673a4
	 * scrivono lo stesso -1 in `current_thread_info()->addr_limit`, ognuna
	 * seguita dalla sua `set_bit(TIF_FSCHECK)`
	 * ("940fbed4 bl"@0xffffff8008a6739c, "940fbecd bl"@0xffffff8008a673b8).
	 * KERNEL_DS e get_ds() valgono la stessa cosa su arm64, e la costante
	 * -1 e' materializzata UNA volta ("92800016 mov"@0xffffff8008a6737c). */
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	set_fs(get_ds());
	pos = 0;
	/* "93407f22 sxtw"@0xffffff8008a673bc -- la lunghezza e' estesa CON
	 * SEGNO, terza prova che `len` e' `int`. Il ritorno non e' usato. */
	vfs_write(f, fw_buf, len, &pos);
	set_fs(old_fs);

	filp_close(f, NULL);
	/* riga 202 di fabbrica. NON protetta da `ilitek_dbg_en`, e il livello
	 * del formato @0xffffff8009245bd1 e' \x016 = KERN_INFO. */
	/* "\x016ILITEK: (%s, %d): Save iram data to %s\n"@0xffffff8009245bd1 */
#line 202
	ILI_INFO("Save iram data to %s\n", ILI_DUMP_IRAM_PATH);

	/*
	 * LE DUE CODE SONO DUE, NEL BINARIO. Non e' una scelta di stile: il
	 * binario contiene TRE `bl ilitek_ice_mode_ctrl`
	 * ("97ffbfef bl"@0xffffff8008a671ec sul ramo d'errore e
	 * "97ffbf3e bl"@0xffffff8008a674b0 su quello riuscito, oltre a quella
	 * d'ingresso) e DUE `bl printk` per la riga 206
	 * ("97db30b2 bl"@0xffffff8008a6720c e "97db3002 bl"@0xffffff8008a674cc),
	 * con la stringa scelta staticamente in ognuna delle due
	 * ("911a1c63 add"@0xffffff8008a67204 = "FAIL",
	 * "913a7463 add"@0xffffff8008a674c0 = "SUCCESS") e NESSUNA `csel`.
	 * Una coda sola con l'operatore ternario -- che e' quello che fa
	 * `ilitek_tddi_fw_dump_flash_data`, dove la `csel` C'E'
	 * ("9a88b123 csel"@0xffffff8008a68484) -- misura 880 byte contro 912.
	 * Le 8 istruzioni mancanti sono esattamente la seconda copia.
	 * Che di fabbrica ci fossero due code SCRITTE, o una sola che il
	 * compilatore ha duplicato, il binario non lo dice: e' misurato il
	 * codice, non il sorgente.
	 */
	ilitek_ice_mode_ctrl(false, false);
	/* riga 206 di fabbrica. "SUCCESS"@0xffffff80090f6e9d;
	 * "\x016ILITEK: (%s, %d): Dump IRAM %s\n"@0xffffff8009245bfb
	 * "SUCCESS"@0xffffff80090f6e9d */
	ILI_INFO("Dump IRAM %s\n", "SUCCESS");
	/* "52800c88 mov"@0xffffff8008a674d0 poi "17ffff50 b"@0xffffff8008a674d4:
	 * la costante 100 e' l'UNICA cosa che questo ramo calcola prima di
	 * saltare nella coda condivisa. */
	stato = 100;
	goto fine;

out:
	ilitek_ice_mode_ctrl(false, false);
	/* riga 206 di fabbrica. "FAIL"@0xffffff8009249687;
	 * "\x016ILITEK: (%s, %d): Dump IRAM %s\n"@0xffffff8009245bfb
	 * "FAIL"@0xffffff8009249687 */
	ILI_INFO("Dump IRAM %s\n", "FAIL");
	/* "12800008 mov"@0xffffff8008a67210 */
	stato = -1;

fine:
	/* "f9452309 ldr"@0xffffff8008a67214 + "b901bd28 str"@0xffffff8008a67218:
	 * la lettura di `idev` e la scrittura di +444 sono UNA sola nel
	 * binario, condivisa dai due rami. */
	idev->c444 = stato;
	/*
	 * Sul ramo in cui `kzalloc` ha fallito il puntatore NON e' azzerato:
	 * "17ffff73 b"@0xffffff8008a67418 salta DOPO la
	 * "aa1f03f3 mov"@0xffffff8008a671e0 che azzera x19. La fabbrica
	 * passerebbe quindi un ERR_PTR a `kfree`. Si riproduce com'e'
	 * (regola 7): in pratica `kzalloc` non restituisce mai un ERR_PTR,
	 * quindi il ramo e' morto, ma il codice c'e'.
	 */
	if (fw_buf != NULL)
		kfree(fw_buf);
}

/*
 * ===========================================================================
 * ilitek_tddi_flash_poll_busy -- 0xffffff8008a6b350, 472 byte
 * ===========================================================================
 * `t` nella mappa: `static` di fabbrica, e qui lo e'. Ma porta un
 * `noinline` CHE DI FABBRICA NON C'E' -- divergenza G2c. Di fabbrica ha DUE
 * siti di chiamata ("9400050d bl"@0xffffff8008a69f1c dentro
 * `ilitek_tddi_fw_upgrade` e "94000289 bl"@0xffffff8008a6a92c dentro
 * `ilitek_tddi_fw_flash_erase`) e clang non l'ha incorporata; con il solo
 * secondo -- che e' quello che resta in questo lotto parziale -- la
 * incorpora, e `ilitek_tddi_fw_flash_erase` passa da 612 a 1052 byte
 * mentre questa sparisce. Il `noinline` ripristina la forma di fabbrica e
 * va tolto quando il gruppo G sara' completo.
 *
 * IL CICLO E' `do { ... } while (--timer >= 0)`, e la forma e' misurata:
 *   "71000673 subs"@0xffffff8008a6b488 e "54fffce5 b.pl"@0xffffff8008a6b48c.
 * `subs` decrementa E mette a bandiera il risultato, `b.pl` risale se il
 * risultato e' >= 0: e' il decremento PREFISSO. La forma postfissa
 * `timer-- > 0` calcola la stessa condizione ma obbliga clang a tenere vivi
 * il vecchio e il nuovo valore, e costa 16 byte (488 contro 472): e' la
 * correzione 2 in cima al file. Il valore che resta dopo il ciclo e' quello
 * DECREMENTATO, ed e' quello che "7100027f cmp"@0xffffff8008a6b4a8
 * confronta (la stessa codifica ricompare a 0xffffff8008a6b4d0, dopo la
 * stampa d'errore).
 *
 * Fino alla revisione del 2026-08-21 questo capoverso era un residuo di
 * redazione mai ripulito: annunciava `timer-- > 0` -- contro il codice,
 * contro il binario e contro la correzione 2 dello stesso file -- ancorava
 * la "7100027f cmp" a 0xffffff8008a6b488, dove sta invece la
 * "71000673 subs", e dichiarava inesistente un'istruzione che tre righe
 * dopo citava come reale. Il CODICE era gia' quello giusto: e' la prova
 * che era da buttare.
 */
static int ilitek_tddi_flash_poll_busy(int timer)
{
	/* "b90007ff str"@0xffffff8008a6b390 -- azzerata prima del ciclo */
	u32 tmp = 0;

	/* "52820000 mov"@0xffffff8008a6b374 + "72a00080 movk"@0xffffff8008a6b37c
	 * = 0x41000 */
	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* riga 244;
		 * "\x013ILITEK: (%s, %d): Pull cs low failed\n"@0xffffff800924114c */
#line 244
		ILI_ERR("Pull cs low failed\n");

	/* "52954aa1 mov"@0xffffff8008a6b3b4 + "72a00cc1 movk"@0xffffff8008a6b3bc
	 * = 0x66aa55, e la lunghezza e' 3 ("320007e2 orr"@0xffffff8008a6b3c0) */
	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* riga 247;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 247
		ILI_ERR("Write key failed\n");

	/* "528000a1 mov"@0xffffff8008a6b3e8 = 5 */
	if (ilitek_ice_mode_write(0x41008, 0x5, 1) < 0)
		/* riga 250;
		 * "\x013ILITEK: (%s, %d): Write 0x5 cmd failed\n"@0xffffff800924724c */
#line 250
		ILI_ERR("Write 0x5 cmd failed\n");

	do {
		if (ilitek_ice_mode_write(0x41008, 0xFF, 1) < 0)
			/* riga 254;
			 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0 */
#line 254
			ILI_ERR("Write dummy failed\n");

		/* vedi G5 */
		mdelay(1);

		/* "910013e1 add"@0xffffff8008a6b45c -- &tmp, e la lunghezza e'
		 * 1 ("320003e2 orr"@0xffffff8008a6b460) */
		if (ilitek_ice_mode_read(0x41010, &tmp, sizeof(u8)) < 0)
			/* riga 259;
			 * "\x013ILITEK: (%s, %d): Read flash busy error\n"@0xffffff8009247276 */
#line 259
			ILI_ERR("Read flash busy error\n");

		/* "394013e8 ldrb"@0xffffff8008a6b47c + "7200051f tst"@0xffffff8008a6b480:
		 * la lettura per il CONFRONTO e' a un byte, quella per la stampa
		 * finale e' a 32 bit ("b94007e3 ldr"@0xffffff8008a6b4d8). */
		if ((tmp & 0x3) == 0)
			break;
	} while (--timer >= 0);

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* riga 266;
		 * "\x013ILITEK: (%s, %d): Pull cs high failed\n"@0xffffff80092472a1 */
#line 266
		ILI_ERR("Pull cs high failed\n");

	if (timer > 0)
		return 0;

	/* riga 269;
	 * "\x013ILITEK: (%s, %d): Flash polling busy timeout ! tmp = %x\n"@0xffffff80092472ca */
#line 269
	ILI_ERR("Flash polling busy timeout ! tmp = %x\n", tmp);
	return -1;
}

/*
 * ===========================================================================
 * ilitek_tddi_flash_clear_dma -- 0xffffff8008a674dc, 312 byte
 * ===========================================================================
 * Ritorno `void`: nessun percorso materializza w0 prima dell'epilogo
 * (0xffffff8008a67608), e le due chiamate note -- 0xffffff8008a59c58 e
 * 0xffffff8008a5b324 -- non lo leggono.
 *
 * Le tre `%lu`: vedi G7. Il valore stampato e' la MASCHERA, non il dato
 * scritto -- "32101fe3 orr"@0xffffff8008a67520 mette 0xff0000 in w3 mentre
 * il dato passato alla chiamata era 0x20000
 * ("320f03e2 orr"@0xffffff8008a674f8). Non e' una lettura sbagliata: e'
 * cosi' nel binario.
 */
void ilitek_tddi_flash_clear_dma(void)
{
	if (ilitek_ice_mode_bit_mask_write(0x41000, 0xFF0000, (2 << 16)) < 0)
		/* riga 279;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 279
		ILI_ERR("Write %lu at %x failed\n", 0xFF0000UL, 0x41000);

	/* "11004293 add"@0xffffff8008a6752c -- 0x41000 + 0x10 */
	if (ilitek_ice_mode_bit_mask_write(0x41010, 0xFF000000, 0) < 0)
		/* riga 282;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 282
		ILI_ERR("Write %lu at %x failed\n", 0xFF000000UL, 0x41010);

	if (ilitek_ice_mode_bit_mask_write(0x41000, 0x01000000, 0) < 0)
		/* riga 285;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 285
		ILI_ERR("Write %lu at %x failed\n", 0x01000000UL, 0x41000);

	/* "11003293 add"@0xffffff8008a675a0 -- 0x41000 + 0xc */
	if (ilitek_ice_mode_write(0x4100C, 0x00, 1) < 0)
		/* riga 288;
		 * "\x013ILITEK: (%s, %d): Write 0x0 at %x failed\n"@0xffffff8009242ece */
#line 288
		ILI_ERR("Write 0x0 at %x failed\n", 0x4100C);

	if (ilitek_ice_mode_write(0x41010, 0xFF, 1) < 0)
		/* riga 291;
		 * "\x013ILITEK: (%s, %d): Write 0xFF at %x failed\n"@0xffffff8009245c39 */
#line 291
		ILI_ERR("Write 0xFF at %x failed\n", 0x41010);
}

/*
 * ===========================================================================
 * ilitek_tddi_flash_read_int_flag -- 0xffffff8008a67614, 264 byte
 * ===========================================================================
 * L'INDIRIZZO LETTO E' ZERO. "2a1f03e0 mov"@0xffffff8008a67664 mette wzr in
 * w0 subito prima della chiamata: non e' un indirizzo del blocco flash. E'
 * quello che dice il binario e si riporta com'e'.
 *
 * La lunghezza e' 4 ("321e03e2 orr"@0xffffff8008a67660), non 1: questa
 * lettura e' a 32 bit.
 *
 * Il contatore parte da 500 ("52803e96 mov"@0xffffff8008a67640) ed e'
 * confrontato PRIMA di essere decrementato ("710002df cmp"@0xffffff8008a67694
 * poi "510006d6 sub"@0xffffff8008a67698), che e' `timer-- > 0`.
 */
int ilitek_tddi_flash_read_int_flag(void)
{
	int timer = 500;
	u32 flag = 0;

	do {
		if (ilitek_ice_mode_read(0x0, &flag, sizeof(u32)) < 0)
			/* riga 301;
			 * "\x013ILITEK: (%s, %d): Read flash int flag error\n"@0xffffff8009245c66 */
#line 301
			ILI_ERR("Read flash int flag error\n");

		/* riga 303. Protetta: "396562e8 ldrb"@0xffffff8008a67670 +
		 * "340000c8 cbz"@0xffffff8008a67674 */
		/* "\x016ILITEK: (%s, %d): int flag = %x\n"@0xffffff8009245cb5 */
#line 303
		ILI_DBG("int flag = %x\n", flag);

		/* "35000183 cbnz"@0xffffff8008a67690 -- il confronto e' con
		 * ZERO, non con un valore di bandiera particolare */
		if (flag)
			break;
	} while (timer-- > 0);

	if (timer > 0)
		return 0;

	/* riga 309. L'argomento e' ancora in w3 dalla `cbnz`: il binario NON
	 * ricarica, ma il valore stampato e' `flag`. */
	/* "\x013ILITEK: (%s, %d): Read Flash INT flag timeout !, flag = 0x%x\n"@0xffffff8009245cd8 */
#line 309
	ILI_ERR("Read Flash INT flag timeout !, flag = 0x%x\n", flag);
	return -1;
}

/*
 * ===========================================================================
 * ilitek_tddi_flash_dma_write -- 0xffffff8008a6771c, 1092 byte
 * ===========================================================================
 * TRE parametri, e il SECONDO il corpo non lo usa: "2a0203f3 mov"@0xffffff8008a6773c
 * salva w2 e "2a0003f5 mov"@0xffffff8008a67740 salva w0, mentre w1 e'
 * sovrascritto subito dopo da "32101fe1 orr"@0xffffff8008a67744. Che i
 * parametri siano tre e non due lo dice il SITO DI CHIAMATA di
 * `dma_trigger_reg_setting`:
 *   "0b140261 add"@0xffffff8008a5b61c   w1 = w19 + w20
 *   "2a1403e0 mov"@0xffffff8008a5b620   w0 = w20
 *   "2a1303e2 mov"@0xffffff8008a5b624   w2 = w19
 * cioe' (start, start+len, len). Il nome no, la posizione si'.
 *
 * ATTENZIONE ALL'ASIMMETRIA DELLE TRE STAMPE D'INDIRIZZO, che e' un fatto
 * del binario: la prima stampa il byte NON scorso
 * ("12101ea3 and"@0xffffff8008a67a54 = start & 0xff0000) mentre la seconda
 * e la terza stampano lo stesso valore che scrivono
 * ("2a1603e3 mov"@0xffffff8008a67aa8, "2a1503e3 mov"@0xffffff8008a67aec).
 */
void ilitek_tddi_flash_dma_write(u32 start, u32 end, u32 len)
{
	if (ilitek_ice_mode_bit_mask_write(0x41000, 0xFF0000, (1 << 16)) < 0)
		/* riga 318;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 318
		ILI_ERR("Write %lu at %x failed\n", 0xFF0000UL, 0x41000);

	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* riga 321;
		 * "\x013ILITEK: (%s, %d): Pull cs low failed\n"@0xffffff800924114c */
#line 321
		ILI_ERR("Pull cs low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* riga 324;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 324
		ILI_ERR("Write key failed\n");

	/* "52800161 mov"@0xffffff8008a677dc = 0x0b */
	if (ilitek_ice_mode_write(0x41008, 0x0b, 1) < 0)
		/* riga 327;
		 * "\x013ILITEK: (%s, %d): Write 0x0b at %x failed\n"@0xffffff8009245d34 */
#line 327
		ILI_ERR("Write 0x0b at %x failed\n", 0x41008);

	if (ilitek_tddi_flash_read_int_flag() < 0) {
		/* riga 330;
		 * "\x013ILITEK: (%s, %d): Write 0xb timeout \n"@0xffffff8009245d61 */
#line 330
		ILI_ERR("Write 0xb timeout \n");
		return;
	}

	/* "52900080 mov"@0xffffff8008a677f4 + "72a00080 movk"@0xffffff8008a677f8
	 * = 0x48004; la maschera e il valore sono lo stesso 1<<25
	 * ("320703e1 orr"@0xffffff8008a677fc, "320703e2 orr"@0xffffff8008a67800) */
	if (ilitek_ice_mode_bit_mask_write(0x48004, (1 << 25), (1 << 25)) < 0)
		/* riga 335;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 335
		ILI_ERR("Write %lu at %x failed\n", (1UL << 25), 0x48004);

	/* "53105ea1 ubfx"@0xffffff8008a67834 */
	if (ilitek_ice_mode_write(0x41008, (start & 0xFF0000) >> 16, 1) < 0)
		/* riga 338;
		 * "\x013ILITEK: (%s, %d): Write %x at %x failed\n"@0xffffff8009242f72 */
#line 338
		ILI_ERR("Write %x at %x failed\n", start & 0xFF0000, 0x41008);

	if (ilitek_tddi_flash_read_int_flag() < 0) {
		/* riga 341;
		 * "\x013ILITEK: (%s, %d): Write addr1 timeout\n"@0xffffff8009245d89 */
#line 341
		ILI_ERR("Write addr1 timeout\n");
		return;
	}

	if (ilitek_ice_mode_bit_mask_write(0x48004, (1 << 25), (1 << 25)) < 0)
		/* riga 346;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 346
		ILI_ERR("Write %lu at %x failed\n", (1UL << 25), 0x48004);

	/* "53083eb6 ubfx"@0xffffff8008a6788c -- il risultato resta in w22
	 * perche' serve DUE volte, alla scrittura e alla stampa */
	if (ilitek_ice_mode_write(0x41008, (start & 0x00FF00) >> 8, 1) < 0)
		/* riga 349;
		 * "\x013ILITEK: (%s, %d): Write %x at %x failed\n"@0xffffff8009242f72 */
#line 349
		ILI_ERR("Write %x at %x failed\n", (start & 0x00FF00) >> 8, 0x41008);

	if (ilitek_tddi_flash_read_int_flag() < 0) {
		/* riga 352;
		 * "\x013ILITEK: (%s, %d): Write addr2 timeout\n"@0xffffff8009245db2 */
#line 352
		ILI_ERR("Write addr2 timeout\n");
		return;
	}

	if (ilitek_ice_mode_bit_mask_write(0x48004, (1 << 25), (1 << 25)) < 0)
		/* riga 357;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 357
		ILI_ERR("Write %lu at %x failed\n", (1UL << 25), 0x48004);

	/* "12001eb5 and"@0xffffff8008a678ec */
	if (ilitek_ice_mode_write(0x41008, (start & 0x0000FF), 1) < 0)
		/* riga 360;
		 * "\x013ILITEK: (%s, %d): Write %x at %x failed\n"@0xffffff8009242f72 */
#line 360
		ILI_ERR("Write %x at %x failed\n", (start & 0x0000FF), 0x41008);

	if (ilitek_tddi_flash_read_int_flag() < 0) {
		/* riga 363;
		 * "\x013ILITEK: (%s, %d): Write addr3 timeout\n"@0xffffff8009245ddb */
#line 363
		ILI_ERR("Write addr3 timeout\n");
		return;
	}

	if (ilitek_ice_mode_bit_mask_write(0x48004, (1 << 25), (1 << 25)) < 0)
		/* riga 368;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 368
		ILI_ERR("Write %lu at %x failed\n", (1UL << 25), 0x48004);

	if (ilitek_ice_mode_bit_mask_write(0x41000, (1 << 24), 0) < 0)
		/* riga 371;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 371
		ILI_ERR("Write %lu at %x failed\n", (1UL << 24), 0x41000);

	if (ilitek_ice_mode_write(0x41008, 0x00, 1) < 0)
		/* riga 374;
		 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0 */
#line 374
		ILI_ERR("Write dummy failed\n");

	if (ilitek_tddi_flash_read_int_flag() < 0) {
		/* riga 377;
		 * "\x013ILITEK: (%s, %d): Write dummy timeout\n"@0xffffff8009245e04 */
#line 377
		ILI_ERR("Write dummy timeout\n");
		return;
	}

	if (ilitek_ice_mode_bit_mask_write(0x48004, (1 << 25), (1 << 25)) < 0)
		/* riga 382;
		 * "\x013ILITEK: (%s, %d): Write %lu at %x failed\n"@0xffffff8009242e59 */
#line 382
		ILI_ERR("Write %lu at %x failed\n", (1UL << 25), 0x48004);

	/* "110012e0 add"@0xffffff8008a679dc = 0x41008 + 4, lunghezza 4
	 * ("321e03e2 orr"@0xffffff8008a679e0) */
	if (ilitek_ice_mode_write(0x4100C, len, 4) < 0)
		/* riga 385;
		 * "\x013ILITEK: (%s, %d): Write length failed\n"@0xffffff8009245e2d */
#line 385
		ILI_ERR("Write length failed\n");
}

/*
 * ===========================================================================
 * ilitek_tddi_flash_write_enable -- 0xffffff8008a6b274, 220 byte
 * ===========================================================================
 * `t` nella mappa: `static`. Ritorno `void` -- l'ultima istruzione prima
 * dell'epilogo e' una `bl printk` e nessun percorso materializza w0.
 */
static void ilitek_tddi_flash_write_enable(void)
{
	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* riga 391. Il formato ha "CS" MAIUSCOLO qui
		 * (@0xffffff8009245ecb) e minuscolo altrove
		 * (@0xffffff800924114c): sono due letterali diversi. */
		/* "\x013ILITEK: (%s, %d): Pull CS low failed\n"@0xffffff8009245ecb */
#line 391
		ILI_ERR("Pull CS low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* riga 394;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 394
		ILI_ERR("Write key failed\n");

	/* "321f07e1 orr"@0xffffff8008a6b2ec = 6 */
	if (ilitek_ice_mode_write(0x41008, 0x6, 1) < 0)
		/* riga 397;
		 * "\x013ILITEK: (%s, %d): Write 0x6 failed\n"@0xffffff80092471e1 */
#line 397
		ILI_ERR("Write 0x6 failed\n");

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* riga 400;
		 * "\x013ILITEK: (%s, %d): Pull CS high failed\n"@0xffffff8009247207 */
#line 400
		ILI_ERR("Pull CS high failed\n");
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_read_hw_crc -- 0xffffff8008a67b60, 1112 byte
 * ===========================================================================
 * Il RITORNO e' il CRC letto ("b94003e0 ldr"@0xffffff8008a67f20) oppure -1
 * ("12800000 mov"@0xffffff8008a67f80): un `int`, non `void`.
 *
 * Il contatore parte da 501 nel binario ("52803eb9 mov"@0xffffff8008a67e44)
 * e la coda usa il valore MENO UNO ("51000735 sub"@0xffffff8008a67ecc).
 * E' la riscrittura che LLVM fa di `do {...} while (retry-- > 0)` quando il
 * valore iniziale e' una costante: l'induttore diventa retry+1 e il test di
 * uscita `--k > 0`. Con un contatore PARAMETRO -- `ilitek_tddi_flash_poll_busy`
 * -- la stessa forma sorgente da' invece `cmp`+`sub`+`b.pl`, perche' lo
 * spostamento non e' possibile. Due funzioni, una sola forma sorgente, due
 * codifiche: e' la prova che la forma e' quella.
 */
int ilitek_tddi_fw_read_hw_crc(u32 start, u32 end)
{
	int retry = 500;
	u32 busy = 0, crc = 0;

	/* "f9401908 ldr"@0xffffff8008a67b98 + "b9403904 ldr"@0xffffff8008a67b9c,
	 * confronto SENZA segno ("54000122 b.cs"@0xffffff8008a67ba4) */
	if (end > ((struct ilitek_g_c48 *)idev->c48)->c56) {
		/* riga 412. Il secondo argomento (w4) NON e' ricaricato: e'
		 * ancora quello della `ldr` del confronto. */
		/* "\x013ILITEK: (%s, %d): The length (%x) written into firmware is greater than max count (%x)\n"@0xffffff8009245e56 */
		ILI_ERR("The length (%x) written into firmware is greater than max count (%x)\n",
#line 412
			end, ((struct ilitek_g_c48 *)idev->c48)->c56);
		return -1;
	}

	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* riga 417;
		 * "\x013ILITEK: (%s, %d): Pull CS low failed\n"@0xffffff8009245ecb */
#line 417
		ILI_ERR("Pull CS low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* riga 420;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 420
		ILI_ERR("Write key failed\n");

	/* "52800761 mov"@0xffffff8008a67c3c = 0x3b */
	if (ilitek_ice_mode_write(0x41008, 0x3b, 1) < 0)
		/* riga 423;
		 * "\x013ILITEK: (%s, %d): Write 0x3b failed\n"@0xffffff8009245ef3 */
#line 423
		ILI_ERR("Write 0x3b failed\n");

	/* "53105e81 ubfx"@0xffffff8008a67c68 */
	if (ilitek_ice_mode_write(0x41008, (start & 0xFF0000) >> 16, 1) < 0)
		/* riga 426 -- lo STESSO formato @0xffffff80092411c6 per tutte e
		 * tre le scritture d'indirizzo */
		/* "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 426
		ILI_ERR("Write address failed\n");

	/* "53083e81 ubfx"@0xffffff8008a67c98 */
	if (ilitek_ice_mode_write(0x41008, (start & 0x00FF00) >> 8, 1) < 0)
		/* riga 429;
		 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 429
		ILI_ERR("Write address failed\n");

	/* "12001e81 and"@0xffffff8008a67cc8 */
	if (ilitek_ice_mode_write(0x41008, (start & 0x0000FF), 1) < 0)
		/* riga 432;
		 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 432
		ILI_ERR("Write address failed\n");

	/* "51001714 sub"@0xffffff8008a67cf4 -- 0x41008 - 5 = 0x41003 */
	if (ilitek_ice_mode_write(0x41003, 0x01, 1) < 0)
		/* riga 435;
		 * "\x013ILITEK: (%s, %d): Write enable Dio_Rx_dual failed\n"@0xffffff8009245f1a */
#line 435
		ILI_ERR("Write enable Dio_Rx_dual failed\n");

	if (ilitek_ice_mode_write(0x41008, 0xFF, 1) < 0)
		/* riga 438;
		 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0 */
#line 438
		ILI_ERR("Write dummy failed\n");

	/* "11001300 add"@0xffffff8008a67d54 -- 0x41008 + 4; lunghezza 3 */
	if (ilitek_ice_mode_write(0x4100C, end, 3) < 0)
		/* riga 441;
		 * "\x013ILITEK: (%s, %d): Write Set Receive count failed\n"@0xffffff8009245f4f */
#line 441
		ILI_ERR("Write Set Receive count failed\n");

	/* "529000e0 mov"@0xffffff8008a67d80 + "72a00080 movk"@0xffffff8008a67d84
	 * = 0x48007 */
	if (ilitek_ice_mode_write(0x48007, 0x02, 1) < 0)
		/* riga 444;
		 * "\x013ILITEK: (%s, %d): Write clearing int flag failed\n"@0xffffff8009245f83 */
#line 444
		ILI_ERR("Write clearing int flag failed\n");

	/* "11003b15 add"@0xffffff8008a67db0 -- 0x41008 + 0xe = 0x41016, ed e'
	 * lo stesso 041016 che il formato @0xffffff8009245fb7 nomina */
	if (ilitek_ice_mode_write(0x41016, 0x00, 1) < 0)
		/* riga 447;
		 * "\x013ILITEK: (%s, %d): Write 0x0 at 0x041016 failed\n"@0xffffff8009245fb7 */
#line 447
		ILI_ERR("Write 0x0 at 0x041016 failed\n");

	if (ilitek_ice_mode_write(0x41016, 0x01, 1) < 0)
		/* riga 450;
		 * "\x013ILITEK: (%s, %d): Write Checksum_En failed\n"@0xffffff8009245fe9 */
#line 450
		ILI_ERR("Write Checksum_En failed\n");

	/* "11002300 add"@0xffffff8008a67e0c -- 0x41008 + 8 = 0x41010 */
	if (ilitek_ice_mode_write(0x41010, 0xFF, 1) < 0)
		/* riga 453;
		 * "\x013ILITEK: (%s, %d): Write start to receive failed\n"@0xffffff8009246017 */
#line 453
		ILI_ERR("Write start to receive failed\n");

	do {
		if (ilitek_ice_mode_read(0x48007, &busy, sizeof(u8)) < 0)
			/* riga 457;
			 * "\x013ILITEK: (%s, %d): Read busy error\n"@0xffffff800924604a */
#line 457
			ILI_ERR("Read busy error\n");

		/* riga 459, protetta da "39656348 ldrb"@0xffffff8008a67e70;
		 * "\x016ILITEK: (%s, %d): busy = %x\n"@0xffffff800924606f */
#line 459
		ILI_DBG("busy = %x\n", busy);

		/* "394013e8 ldrb"@0xffffff8008a67e8c +
		 * "37080188 tbnz"@0xffffff8008a67e90: il solo bit 1 */
		if ((busy & 0x02) == 0x02)
			break;
	} while (retry-- > 0);

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* riga 465;
		 * "\x013ILITEK: (%s, %d): Write CS high failed\n"@0xffffff800924608e */
#line 465
		ILI_ERR("Write CS high failed\n");

	if (retry <= 0) {
		/* riga 468;
		 * "\x013ILITEK: (%s, %d): Read HW CRC timeout !, busy = 0x%x\n"@0xffffff80092460b8 */
#line 468
		ILI_ERR("Read HW CRC timeout !, busy = 0x%x\n", busy);
		return -1;
	}

	if (ilitek_ice_mode_write(0x41003, 0x00, 1) < 0)
		/* riga 473;
		 * "\x013ILITEK: (%s, %d): Write disable dio_Rx_dual failed\n"@0xffffff80092460f0 */
#line 473
		ILI_ERR("Write disable dio_Rx_dual failed\n");

	/* "11005300 add"@0xffffff8008a67f0c -- 0x41008 + 0x14 = 0x4101c;
	 * "910003e1 mov"@0xffffff8008a67f10 -- &crc e' in cima alla pila,
	 * lunghezza 4 */
	if (ilitek_ice_mode_read(0x4101C, &crc, sizeof(u32)) < 0) {
		/* riga 476;
		 * "\x013ILITEK: (%s, %d): Read hw crc error\n"@0xffffff8009246126 */
#line 476
		ILI_ERR("Read hw crc error\n");
		return -1;
	}

	return crc;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_read_flash_data -- 0xffffff8008a67fb8, 712 byte
 * ===========================================================================
 * QUATTRO parametri, e li fissa il sito di chiamata di
 * `ilitek_tddi_fw_dump_flash_data`:
 *   "2a1703e0 mov"@0xffffff8008a68348  w0 start
 *   "2a1403e1 mov"@0xffffff8008a6834c  w1 end
 *   "aa1503e2 mov"@0xffffff8008a68350  x2 puntatore
 *   "2a1603e3 mov"@0xffffff8008a68354  w3 lunghezza
 *
 * TUTTO E' SENZA SEGNO qui, ed e' l'opposto di `ilitek_tddi_fw_iram_read`:
 *   "54000129 b.ls"@0xffffff8008a67ff0    controllo di lunghezza
 *   "383c4a68 strb"@0xffffff8008a681dc    indice `uxtw`, non `sxtw`
 *   "1ad40b23 udiv"@0xffffff8008a681d8    percentuale senza segno
 *   "54fffb69 b.ls"@0xffffff8008a68214    fine ciclo
 */
int ilitek_tddi_fw_read_flash_data(u32 start, u32 end, u8 *data, u32 len)
{
	u32 i, index = 0;
	/*
	 * `tmp` NON azzerata: fra il prologo e il primo uso la fabbrica non
	 * scrive niente nel suo slot, e con `= 0` clang emetteva una
	 * "b90007ff str wzr, [sp,#4]" che nel binario non c'e' -- quattro byte,
	 * ed erano tutta la differenza.
	 */
	u32 tmp;

	/* "4b000023 sub"@0xffffff8008a67fe4 + "6b04007f cmp"@0xffffff8008a67fe8 */
	if ((end - start) > len) {
		/* riga 489;
		 * "\x013ILITEK: (%s, %d): the length (%d) reading crc is over than len(%d)\n"@0xffffff800924614d */
		ILI_ERR("the length (%d) reading crc is over than len(%d)\n",
#line 489
			end - start, len);
		return -1;
	}

	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* riga 494;
		 * "\x013ILITEK: (%s, %d): Write cs low failed\n"@0xffffff80092461b2 */
#line 494
		ILI_ERR("Write cs low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* riga 497;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 497
		ILI_ERR("Write key failed\n");

	/* "320007e1 orr"@0xffffff8008a68090 = 3 */
	if (ilitek_ice_mode_write(0x41008, 0x03, 1) < 0)
		/* riga 500;
		 * "\x013ILITEK: (%s, %d): Write 0x3 failed\n"@0xffffff80092461db */
#line 500
		ILI_ERR("Write 0x3 failed\n");

	if (ilitek_ice_mode_write(0x41008, (start & 0xFF0000) >> 16, 1) < 0)
		/* riga 503;
		 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 503
		ILI_ERR("Write address failed\n");

	if (ilitek_ice_mode_write(0x41008, (start & 0x00FF00) >> 8, 1) < 0)
		/* riga 506;
		 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 506
		ILI_ERR("Write address failed\n");

	if (ilitek_ice_mode_write(0x41008, (start & 0x0000FF), 1) < 0)
		/* riga 509;
		 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 509
		ILI_ERR("Write address failed\n");

	/* La guardia d'ingresso del ciclo e' "6b15029f cmp"@0xffffff8008a68130
	 * + "54000142 b.cs"@0xffffff8008a68134, cioe' `start <= end`. */
	for (i = start; i <= end; i++) {
		if (ilitek_ice_mode_write(0x41008, 0xFF, 1) < 0)
			/* riga 513;
			 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0 */
#line 513
			ILI_ERR("Write dummy failed\n");

		if (ilitek_ice_mode_read(0x41010, &tmp, sizeof(u8)) < 0)
			/* riga 516;
			 * "\x013ILITEK: (%s, %d): Read flash data error!\n"@0xffffff8009246201 */
#line 516
			ILI_ERR("Read flash data error!\n");

		data[index] = tmp;
		index++;

		/* "1b087eb9 mul"@0xffffff8008a68178 costruisce start*100 fuori
		 * dal ciclo e "11019339 add"@0xffffff8008a68210 lo incrementa di
		 * 100 a ogni giro: e' la riduzione di forza di `i * 100`. */
		idev->c444 = (i * 100) / end;

		/* riga 521, protetta da "396562e8 ldrb"@0xffffff8008a681ec.
		 * Il '%' e' l'argomento w4 ("528004a4 mov"@0xffffff8008a681f8) e
		 * il formato @0xffffff800924622d finisce SENZA \n. */
		/* "\x016ILITEK: (%s, %d): Reading flash data .... %d%c"@0xffffff800924622d */
#line 521
		ILI_DBG("Reading flash data .... %d%c", idev->c444, '%');
	}

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* riga 525;
		 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
#line 525
		ILI_ERR("Write cs high failed\n");

	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_dump_flash_data -- 0xffffff8008a68280, 608 byte
 * ===========================================================================
 * Qui il ritorno E' materializzato ("2a1403e0 mov"@0xffffff8008a684bc), a
 * differenza di `ilitek_fw_dump_iram_data`: `int`.
 *
 * E qui la coda e' UNA SOLA, con la scelta della stringa fatta a registro:
 * "9a88b123 csel"@0xffffff8008a68484 e "5a9fa113 csinv"@0xffffff8008a68498.
 * E' il contro-esempio che rende misurata l'osservazione su
 * `ilitek_fw_dump_iram_data`: due funzioni sorelle, due forme diverse, e la
 * differenza si vede.
 *
 * `user`, terzo parametro, E' UN NOME SCELTO. Del terzo parametro il
 * binario decide solo la LARGHEZZA -- "720002ff tst"@0xffffff8008a682f8
 * prova il solo bit 0, che e' la larghezza di un `bool` -- e nient'altro:
 * non c'e' nessuna stringa che lo nomini. Vale la stessa nota che il
 * cappello di `ilitek_fw_dump_iram_data` scrive per il proprio terzo
 * parametro («il valore passato e' 0 oppure 1 ... il nome no»); fino alla
 * revisione del 2026-08-21 era scritta li' e non qui.
 */
int ilitek_tddi_fw_dump_flash_data(u32 start, u32 end, bool user)
{
	struct file *f = NULL;
	u8 *hex_buffer = NULL;
	u32 start_addr, end_addr;
	int ret = 0, length;
	loff_t pos = 0;
	mm_segment_t old_fs;

	/* "b901bd1f str"@0xffffff8008a682d0 */
	idev->c444 = 0;

	f = filp_open(ILI_DUMP_FLASH_PATH, O_WRONLY | O_CREAT | O_TRUNC, 644);
	if (IS_ERR(f) || f == NULL) {
		/* riga 543 -- lo STESSO formato @0xffffff8009245b9c che usa
		 * `ilitek_fw_dump_iram_data` */
		/* "\x013ILITEK: (%s, %d): Failed to open the file at %ld.\n"@0xffffff8009245b9c */
#line 543
		ILI_ERR("Failed to open the file at %ld.\n", PTR_ERR(f));
		ret = -1;
		goto out;
	}

	ret = ilitek_ice_mode_ctrl(true, false);
	if (ret < 0)
		goto out;

	/* "720002ff tst"@0xffffff8008a682f8 -- il solo bit 0 del terzo
	 * parametro; "320043e8 orr"@0xffffff8008a682fc = 0x1ffff */
	if (user) {
		start_addr = 0x0;
		end_addr = 0x1FFFF;
	} else {
		start_addr = start;
		end_addr = end;
	}

	/* "4b170288 sub"@0xffffff8008a68308 + "11000515 add"@0xffffff8008a6830c */
	length = end_addr - start_addr + 1;
	/* riga 561. NON protetta: livello \x016 senza guardia;
	 * "\x016ILITEK: (%s, %d): len = %d\n"@0xffffff80092462ba */
#line 561
	ILI_INFO("len = %d\n", length);

	/* "93407eb6 sxtw"@0xffffff8008a6832c -- la lunghezza e' estesa CON
	 * SEGNO prima di `vmalloc`: `length` e' `int`. */
	hex_buffer = vmalloc(length);
	if (IS_ERR(hex_buffer) || hex_buffer == NULL) {
		/* riga 565;
		 * "\x013ILITEK: (%s, %d): Failed to allocate buf memory, %ld\n"@0xffffff80092462d8 */
#line 565
		ILI_ERR("Failed to allocate buf memory, %ld\n", PTR_ERR(hex_buffer));
		filp_close(f, NULL);
		ret = -1;
		goto out;
	}

	ret = ilitek_tddi_fw_read_flash_data(start_addr, end_addr, hex_buffer, length);
	if (ret < 0)
		/*
		 * DIFETTO DELLA FABBRICA, RIPRODOTTO (regola 7): questo ramo
		 * salta alla coda SENZA `filp_close` e SENZA `vfree`
		 * ("37f807e0 tbnz"@0xffffff8008a68360 va dritto a
		 * 0xffffff8008a6845c, dove la coda comincia con la
		 * `ilitek_ice_mode_ctrl`). Perde il file e il buffer.
		 */
		goto out;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	set_fs(get_ds());
	pos = 0;
	vfs_write(f, hex_buffer, length, &pos);
	set_fs(old_fs);

	filp_close(f, NULL);
	vfree(hex_buffer);

out:
	ilitek_ice_mode_ctrl(false, false);
	/* riga 586;
	 * "\x016ILITEK: (%s, %d): Dump flash %s\n"@0xffffff8009246310
	 * "FAIL"@0xffffff8009249687
	 * "SUCCESS"@0xffffff80090f6e9d */
#line 586
	ILI_INFO("Dump flash %s\n", (ret < 0) ? "FAIL" : "SUCCESS");
	idev->c444 = (ret < 0) ? -1 : 100;
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_flash_protect -- INCORPORATA in ilitek_tddi_fw_read_flash_info
 * ===========================================================================
 * `__func__` = "ilitek_tddi_flash_protect"@0xffffff800924749a, righe di
 * fabbrica 593..639, disgiunte dalle 1361..1416 del contenitore.
 *
 * IL RAMO `enable == true` NON E' NEL BINARIO. L'unico sito di chiamata
 * passa una costante falsa, quindi il compilatore ha piegato la scelta:
 * nel gruppo G compare solo "Disable"@0xffffff80090df610 -- che e' per di
 * piu' la CODA condivisa di una stringa piu' lunga, i quindici byte
 * " Bypass Disable" che stanno a 0xffffff80090df608 -- e nessun "Enable".
 * Di quel ramo non si puo' dire niente, quindi qui non c'e' niente: l'operatore ternario del messaggio e'
 * l'unico uso del parametro che il binario sostenga.
 * L'affermazione e' NEL GRUPPO G, e non oltre: fuori dal gruppo la stringa
 * "Enable\0" sta nel binario di fabbrica 42 volte, e "Disable\0" una.
 * Fino alla revisione del 2026-08-21 il cappello del file generalizzava al
 * binario intero -- e sbagliava. Vedi COME SI RIVERIFICA.
 */
static void ilitek_tddi_flash_protect(bool enable)
{
	/* riga 593. Livello \x016 e nessuna guardia;
	 * "\x016ILITEK: (%s, %d): %s flash protection\n"@0xffffff8009247471
	 * "Disable"@0xffffff80090df610 */
#line 593
	ILI_INFO("%s flash protection\n", enable ? "Enable" : "Disable");

	ilitek_tddi_flash_write_enable();

	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* riga 598;
		 * "\x013ILITEK: (%s, %d): Write cs low failed\n"@0xffffff80092461b2 */
#line 598
		ILI_ERR("Write cs low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* riga 601;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 601
		ILI_ERR("Write key failed\n");

	if (ilitek_ice_mode_write(0x41008, 0x1, 1) < 0)
		/* riga 604;
		 * "\x013ILITEK: (%s, %d): Write 0x1 failed\n"@0xffffff80092474b4 */
#line 604
		ILI_ERR("Write 0x1 failed\n");

	if (ilitek_ice_mode_write(0x41008, 0x0, 1) < 0)
		/* riga 607;
		 * "\x013ILITEK: (%s, %d): Write 0x0 failed\n"@0xffffff80092474da */
#line 607
		ILI_ERR("Write 0x0 failed\n");

	/* "79444103 ldrh"@0xffffff8008a6af2c -- la scelta e' sul MID, non
	 * sull'ID; "7103207f cmp"@0xffffff8008a6af30 (0xc8) e
	 * "7103bc7f cmp"@0xffffff8008a6af38 (0xef) */
	switch (idev->c544) {
	case 0xEF:
		/*
		 * "128c0209 mov"@0xffffff8008a6af44 = -24593 = -0x6011 e
		 * "7100051f cmp"@0xffffff8008a6af4c con 1: e' il controllo di
		 * INTERVALLO in cui clang piega `a == 0x6011 || a == 0x6012`.
		 */
		if (idev->c546 == 0x6011 || idev->c546 == 0x6012) {
			if (ilitek_ice_mode_write(0x41008, 0x0, 1) < 0)
				/* riga 617;
				 * "\x013ILITEK: (%s, %d): Write 0x0 at %x failed\n"@0xffffff8009242ece */
				ILI_ERR("Write 0x0 at %x failed\n", 0x41008);
		}
		break;
	case 0xC8:
		/*
		 * "52860129 mov"@0xffffff8008a6af90 = 0x3009 e
		 * "6b48053f cmp"@0xffffff8008a6af94 confronta con `dev_id >> 1`:
		 * e' la forma in cui clang piega `a == 0x6012 || a == 0x6013`,
		 * due costanti che differiscono nel solo bit 0.
		 */
		if (idev->c546 == 0x6012 || idev->c546 == 0x6013) {
			if (ilitek_ice_mode_write(0x41008, 0x0, 1) < 0)
				/* riga 628;
				 * "\x013ILITEK: (%s, %d): Write 0x0 at %x failed\n"@0xffffff8009242ece */
				ILI_ERR("Write 0x0 at %x failed\n", 0x41008);
		}
		break;
	default:
		/* riga 634. L'argomento e' ancora in w3 dal confronto;
		 * "\x013ILITEK: (%s, %d): Can't find flash id(0x%x), ignore protection\n"@0xffffff8009247500 */
		ILI_ERR("Can't find flash id(0x%x), ignore protection\n",
#line 634
			idev->c544);
		break;
	}

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* riga 639;
		 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
#line 639
		ILI_ERR("Write cs high failed\n");
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_flash_erase -- 0xffffff8008a6a758, 612 byte
 * ===========================================================================
 * `t` nella mappa: `static` di fabbrica. QUI NON LO E' -- divergenza G2b:
 * i suoi TRE soli siti di chiamata (0xffffff8008a69cd4, 0xffffff8008a6a1b4,
 * 0xffffff8008a6a228) stanno tutti dentro `ilitek_tddi_fw_upgrade`, che
 * questo lotto non scrive. Con `static` e zero chiamanti clang dichiara i
 * parametri indefiniti e cancella il corpo. Va rimessa `static` quando il
 * gruppo G sara' completo.
 *
 * IL SECONDO FILTRO, G6. "530d7ca8 lsr"@0xffffff8008a6a7b8 con
 * "7100391f cmp"@0xffffff8008a6a7bc, e "530c7c88 lsr"@0xffffff8008a6a7c4 con
 * "7100711f cmp"@0xffffff8008a6a7c8. Le soglie equivalenti sarebbero
 * 15<<13 = 0x1E000 e 29<<12 = 0x1D000, entrambe scrivibili come UNA `cmp
 * #imm12, lsl #12`: il compilatore non avrebbe emesso due istruzioni al
 * posto di una, quindi lo scorrimento sta nel sorgente. Qui e' reso come
 * divisione, che e' la forma che lo riproduce.
 */
static int ilitek_tddi_fw_flash_erase(void)
{
	int i;
	u32 addr;

	/* "f1001e7f cmp"@0xffffff8008a6a98c -- sette voci */
	for (i = 0; i < 7; i++) {
		/* "34000ec5 cbz"@0xffffff8008a6a7b0 */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		if (ilitek_fw_blk[i].c12 / 8192 <= 14 &&
		    ilitek_fw_blk[i].c8 / 4096 > 28)
			continue;

		/* riga 797. Livello \x016, nessuna guardia. I due indirizzi
		 * sono gia' in w4 e w5 dalle due `ldr` di sopra: il binario NON
		 * li ricarica, ed e' la prova che gli argomenti sono proprio
		 * questi due campi in quest'ordine. */
		/* "\x016ILITEK: (%s, %d): Block[%d]: Erasing from (0x%x) to (0x%x) \n"@0xffffff80092473be */
		ILI_INFO("Block[%d]: Erasing from (0x%x) to (0x%x) \n",
#line 797
			 i, ilitek_fw_blk[i].c8, ilitek_fw_blk[i].c12);

		/* "6b08033f cmp"@0xffffff8008a6a7f0 + "54000ca8 b.hi"@0xffffff8008a6a7f4
		 * -- guardia SENZA segno del ciclo interno */
		for (addr = ilitek_fw_blk[i].c8; addr <= ilitek_fw_blk[i].c12;
		     addr += idev->c552) {
			ilitek_tddi_flash_write_enable();

			if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
				/* riga 803;
				 * "\x013ILITEK: (%s, %d): Write cs low failed\n"@0xffffff80092461b2 */
#line 803
				ILI_ERR("Write cs low failed\n");

			if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
				/* riga 806;
				 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 806
				ILI_ERR("Write key failed\n");

			/* "b945e288 ldr"@0xffffff8008a6a850 -- 0xa0ff000+1504 =
			 * 0xa0ff5e0 = ilitek_fw_blk[1].c8, cioe' la voce che
			 * `ilitek_tddi_fw_update_block_info` chiama "AP" */
			if (addr == ilitek_fw_blk[1].c8) {
				/* "52801b01 mov"@0xffffff8008a6a864 = 0xd8 */
				if (ilitek_ice_mode_write(0x41008, 0xD8, 1) < 0)
					/* riga 810. Il formato dice "0xB" ma
					 * il comando scritto e' 0xD8: e' un
					 * difetto del messaggio di fabbrica e
					 * si riproduce. */
					/* "\x013ILITEK: (%s, %d): Write 0xB at %x failed\n"@0xffffff8009247418 */
#line 810
					ILI_ERR("Write 0xB at %x failed\n", 0x41008);
			} else {
				/* "321b03e1 orr"@0xffffff8008a6a88c = 0x20 */
				if (ilitek_ice_mode_write(0x41008, 0x20, 1) < 0)
					/* riga 813;
					 * "\x013ILITEK: (%s, %d): Write 0x20 at %x failed\n"@0xffffff8009247444 */
#line 813
					ILI_ERR("Write 0x20 at %x failed\n", 0x41008);
			}

			/* "12181f21 and"@0xffffff8008a6a8b8,
			 * "33105f21 bfxil"@0xffffff8008a6a8bc,
			 * "33101f21 bfi"@0xffffff8008a6a8c4:
			 * i tre byte dell'indirizzo scambiati e mandati in UNA
			 * sola scrittura da 3 byte ("320007e2 orr"@0xffffff8008a6a8cc) */
			if (ilitek_ice_mode_write(0x41008,
						  ((addr & 0xFF0000) >> 16) |
						  (addr & 0x00FF00) |
						  ((addr & 0x0000FF) << 16), 3) < 0)
				/* riga 818;
				 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 818
				ILI_ERR("Write address failed\n");

			if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
				/* riga 821;
				 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
#line 821
				ILI_ERR("Write cs high failed\n");

			mdelay(1);

			/* "5281b597 mov"@0xffffff8008a6a790 = 3500 e
			 * "52803e96 mov"@0xffffff8008a6a78c = 500, scelti con
			 * "1a9602e0 csel"@0xffffff8008a6a928 sul confronto
			 * RICARICATO dopo la `mdelay`
			 * ("b945e288 ldr"@0xffffff8008a6a920) */
			if (ilitek_tddi_flash_poll_busy(addr == ilitek_fw_blk[1].c8 ?
							3500 : 500) < 0)
				return -1;

			if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
				/* riga 835;
				 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
#line 835
				ILI_ERR("Write cs high failed\n");

			/* "b9400308 ldr"@0xffffff8008a6a95c -- il confronto e'
			 * su `c8` DEL BLOCCO, non su `addr` */
			if (ilitek_fw_blk[i].c8 == ilitek_fw_blk[1].c8)
				break;
		}
	}

	return 0;
}

/*
 * ===========================================================================
 * calc_crc32 -- INCORPORATA, e senza `printk` non lascia ne' nome ne' riga
 * ===========================================================================
 * G3. Che sia una funzione separata e non codice srotolato a mano lo dice la
 * forma: OTTO copie identiche di
 *   "531f796c lsl"@0xffffff8008a6b12c
 *   "7100017f cmp"@0xffffff8008a6b130
 *   "4a19018b eor"@0xffffff8008a6b134
 *   "1a8cb16b csel"@0xffffff8008a6b138
 * dentro un ciclo esterno che conta (c16 - 4) giri. Il polinomio e'
 * "5283b6f9 mov"@0xffffff8008a6b0cc + "72a09839 movk"@0xffffff8008a6b0e4 =
 * 0x04C11DB7, e il valore iniziale e' -1
 * ("12800017 mov"@0xffffff8008a6b11c).
 *
 * `calc_crc32` NON e' nella mappa: nessun simbolo `crc32` del kernel di
 * fabbrica cade nella fascia ILITEK. E' quindi incorporata OVUNQUE, ed e'
 * un ritrovamento che il censimento per `__func__` non poteva fare, perche'
 * quel censimento vede solo le funzioni che stampano.
 *
 * Il TIPO dell'indice e' misurato: "3869ca8b ldrb"@0xffffff8008a6b120 indicizza
 * con `sxtw`, quindi `i` e' `int`; il confronto di fine ciclo e' invece
 * SENZA segno ("54000529 b.ls"@0xffffff8008a6b114), che e' cio' che le
 * conversioni aritmetiche usuali fanno quando `int` incontra `u32`.
 *
 * ---------------------------------------------------------------------------
 * I NOMI DI QUESTA FUNZIONE SONO TUTTI SCELTI, E FINO AL 2026-08-21 NON LO
 * DICEVANO
 * ---------------------------------------------------------------------------
 * `calc_crc32`, `CRC_POLY`, `ReturnCRC`, `start_addr` e `end_addr` (i due
 * parametri, non i due globali omonimi di `ilitek_tddi_fw_upgrade`) e
 * `ex_addr` nel chiamante: NESSUNO di questi identificatori esiste nel
 * binario di fabbrica, in nessuna forma.
 *   python3 -c "d=open('oracolo/stock.elf','rb').read();
 *               print(d.count(b'calc_crc32'), d.count(b'CRC_POLY'),
 *                     d.count(b'ReturnCRC'), d.count(b'ex_addr'))"
 *   0 0 0 0
 * e nella mappa non c'e' nessun simbolo `calc_crc32` (la fascia ILITEK ha
 * quattro simboli con "crc" nel nome: `firmware_hd_dma_crc_off_ili9881`,
 * `firmware_hd_dma_crc_off_ili7807`, `ilitek_tddi_fw_read_hw_crc`,
 * `ilitek_fw_calc_file_crc`). Sono nomi SCELTI, non misurati -- la regola 5
 * del progetto: «un nome descrittivo e' indistinguibile, per chi legge, da
 * un fatto». Il file dichiarava «NOME SCELTO» sopra `ilitek_flashtab` e
 * `ilitek_fw_blk` e non qui, e la disparita' faceva leggere questi come
 * misurati.
 *
 * `pfw`, IL PARAMETRO DI `ilitek_fw_calc_file_crc`, E' INVECE UN NOME DEL
 * BINARIO, e va tenuto distinto dai precedenti. A 0xffffff8009246333 il
 * binario porta il formato
 *   \x013ILITEK: (%s, %d): Failed to allocate pfw memory, %ld\n
 * -- NON citabile nella forma verificabile, perche' appartiene a
 * `ilitek_tddi_fw_upgrade` e nessun letterale di questo file ne e' la coda;
 * si legge con `leggi_stringa.py oracolo/stock.elf 0xffffff8009246333`,
 * esattamente come il "Block Num" di 0xffffff8009246d95 piu' sopra. E' il
 * messaggio d'errore dell'allocazione del buffer 0xffffff800a0ff5a8
 * ("52900000 mov"@0xffffff8008a68558 + "72a00040 movk"@0xffffff8008a6855c =
 * 0x28000, "97df68c4 bl"@0xffffff8008a68560 verso `vmalloc`,
 * "f902d6a0 str"@0xffffff8008a68570 = [pagina+1448]). Che il parametro sia
 * proprio quel buffer e' misurato sul secondo sito di chiamata:
 * "f942d734 ldr"@0xffffff8008a6a268 carica x20 da [pagina+1448] e
 * "aa1403e0 mov"@0xffffff8008a6a6bc lo passa in x0 a
 * "9400027d bl"@0xffffff8008a6a6c0. E' un'inferenza di UN passo -- il nome
 * sta nel messaggio di fallimento dell'allocazione, non su una `printk` che
 * stampi il parametro -- e va letta come tale, ma non e' una invenzione.
 *
 * SOSPETTO APERTO, e resta aperto. La revisione del 2026-08-21 ha osservato
 * che `ReturnCRC` (PascalCase) e `CRC_POLY` (tutto maiuscolo per una
 * variabile LOCALE, non una macro) sono estranei sia al resto del file --
 * che usa `tmp`, `flag`, `busy`, `retry`, `crc`, `len`, `buf`, `addr` --
 * sia alla convenzione del kernel, e che una funzione ricostruita dai byte
 * non ha ragione di chiamare `ReturnCRC` un registro; e ha chiesto se sia
 * stato aperto un sorgente pubblico, cosa che la regola d'ingaggio del
 * progetto vieta e che, se fatta, andrebbe DICHIARATA. Chi corregge non e'
 * chi ha scritto e non puo' rispondere per lui: il sospetto e' registrato
 * qui, non chiuso. Cio' che si puo' dire e' verificabile e sta qui sopra:
 * i quattro identificatori non stanno nel binario, sono marcati SCELTI, e
 * ogni riga del corpo porta accanto la codifica di fabbrica che la
 * giustifica -- che e' esattamente cio' che la regola d'ingaggio chiede in
 * quel caso. Nessun sorgente pubblico e' stato aperto per questa
 * correzione.
 */
static u32 calc_crc32(u32 start_addr, u32 end_addr, u8 *data)
{
	int i, j;
	u32 CRC_POLY = 0x04C11DB7;
	u32 ReturnCRC = 0xFFFFFFFF;
	u32 len = start_addr + end_addr;

	for (i = start_addr; i < len; i++) {
		/* "4a0b62eb eor"@0xffffff8008a6b128 -- il byte entra dal
		 * bit 24 */
		ReturnCRC ^= (data[i] << 24);

		for (j = 0; j < 8; j++) {
			if ((ReturnCRC & 0x80000000) != 0)
				ReturnCRC = ReturnCRC << 1 ^ CRC_POLY;
			else
				ReturnCRC = ReturnCRC << 1;
		}
	}

	return ReturnCRC;
}

/*
 * ===========================================================================
 * ilitek_fw_calc_file_crc -- 0xffffff8008a6b0b4, 448 byte
 * ===========================================================================
 * `t` nella mappa. QUI NON E' `static`, e per la stessa ragione di
 * `ilitek_tddi_fw_flash_erase`: i due siti di chiamata (0xffffff8008a6969c e
 * 0xffffff8008a6a6c0) stanno dentro `ilitek_tddi_fw_upgrade`. Divergenza G2b.
 */
static int ilitek_fw_calc_file_crc(u8 *pfw)
{
	int i;
	u32 ex_addr, data_crc, file_crc;

	/* "f1001e7f cmp"@0xffffff8008a6b20c -- sette voci, come in
	 * `ilitek_tddi_fw_flash_erase` */
	for (i = 0; i < 7; i++) {
		/* "34000868 cbz"@0xffffff8008a6b0fc */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		ex_addr = ilitek_fw_blk[i].c12;
		/* "5100116b sub"@0xffffff8008a6b10c -- la lunghezza passata e'
		 * c16 MENO 4: gli ultimi quattro byte sono il CRC stesso */
		data_crc = calc_crc32(ilitek_fw_blk[i].c8,
				      ilitek_fw_blk[i].c16 - 4, pfw);

		/* "53081d29 lsl"@0xffffff8008a6b1d8,
		 * "33101d49 bfi"@0xffffff8008a6b1dc,
		 * "33181d69 bfi"@0xffffff8008a6b1e0,
		 * "2a080138 orr"@0xffffff8008a6b1e4:
		 * quattro byte in ordine BIG-ENDIAN, e i primi tre indicizzati
		 * `uxtw` ("38694a89 ldrb"@0xffffff8008a6b1c4) */
		file_crc = pfw[ex_addr - 3] << 24 | pfw[ex_addr - 2] << 16 |
			   pfw[ex_addr - 1] << 8 | pfw[ex_addr];

		/* riga 912. Livello \x016, nessuna guardia;
		 * "\x016ILITEK: (%s, %d): data crc = %x, file crc = %x\n"@0xffffff8009246b49 */
#line 912
		ILI_INFO("data crc = %x, file crc = %x\n", data_crc, file_crc);

		/* "6b1802ff cmp"@0xffffff8008a6b200 */
		if (data_crc != file_crc) {
			/* riga 915;
			 * "\x013ILITEK: (%s, %d): Content of fw file is broken. (%d, %x, %x)\n"@0xffffff8009246b93 */
			ILI_ERR("Content of fw file is broken. (%d, %x, %x)\n",
#line 915
				i, data_crc, file_crc);
			return -1;
		}
	}

	/* riga 920;
	 * "\x016ILITEK: (%s, %d): Content of fw file is correct\n"@0xffffff8009246bd3 */
#line 920
	ILI_INFO("Content of fw file is correct\n");
	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_read_flash_info -- 0xffffff8008a6a9bc, 1784 byte
 * ===========================================================================
 * `T` nella mappa, e il ritorno e' `void`: l'unico sito di chiamata
 * ("9400562d bl"@0xffffff8008a55108, in `ilitek_tddi_init`) prosegue con
 * "2a1f03e0 mov"@0xffffff8008a5510c che sovrascrive w0, e nessun percorso
 * della funzione lo materializza.
 *
 * `idev->c712` E' COPIATO IN UN LOCALE, e non e' una scelta: il binario lo
 * legge UNA volta all'ingresso ("b942c916 ldr"@0xffffff8008a6a9f0) e lo
 * rilegge dal registro salvato alla fine ("35000276 cbnz"@0xffffff8008a6b000),
 * attraverso diciassette chiamate. Un `idev->c712` scritto due volte nel
 * sorgente avrebbe dovuto essere ricaricato, perche' `printk` puo' scrivere
 * in memoria.
 * Cio' che e' misurato e' quindi CHE la copia esista, non come si chiami:
 * `ice` E' UN NOME SCELTO. Si appoggia a un dato del binario -- il valore
 * decide se entrare in modalita' ICE, e "ice mode" e' scritto nelle stringhe
 * di formato di questa stessa funzione
 * ("\x013ILITEK: (%s, %d): Enable ice mode failed while reading flash info\n"
 * @0xffffff8009246574) -- ma e' un'inferenza di un passo, non una lettura.
 * Lo stesso vale per `flash_id`, `flash_mid`, `buf` e `tmp`: di questi solo
 * "Flash MID" e "flash id" hanno un riscontro nel binario (i formati a
 * 0xffffff8009246666 e 0xffffff8009247500), e la forma esatta
 * dell'identificatore resta comunque una scelta.
 */
void ilitek_tddi_fw_read_flash_info(void)
{
	int i;
	int ice = idev->c712;
	u8 buf[4] = {0};
	u16 flash_id = 0, flash_mid = 0;
	u32 tmp = 0;

	if (!ice) {
		if (ilitek_ice_mode_ctrl(true, false) < 0)
			/* riga 1361;
			 * "\x013ILITEK: (%s, %d): Enable ice mode failed while reading flash info\n"@0xffffff8009246574 */
#line 1361
			ILI_ERR("Enable ice mode failed while reading flash info\n");
	}

	if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
		/* riga 1365;
		 * "\x013ILITEK: (%s, %d): Write cs low failed\n"@0xffffff80092461b2 */
#line 1365
		ILI_ERR("Write cs low failed\n");

	if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
		/* riga 1368;
		 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 1368
		ILI_ERR("Write key failed\n");

	/* "528013e1 mov"@0xffffff8008a6aa94 = 0x9f */
	if (ilitek_ice_mode_write(0x41008, 0x9F, 1) < 0)
		/* riga 1371;
		 * "\x013ILITEK: (%s, %d): Write 0x9F failed\n"@0xffffff80092465d8 */
#line 1371
		ILI_ERR("Write 0x9F failed\n");

	/*
	 * QUATTRO GIRI, e il binario li ha srotolati tutti e quattro con lo
	 * STESSO numero di riga (0x55f = 1375 e 0x562 = 1378 compaiono
	 * quattro volte ciascuno): e' un ciclo del sorgente, non quattro
	 * copie scritte a mano. Il quarto valore letto non e' mai usato --
	 * l'ultima lettura (0xffffff8008a6ac18) non e' seguita da nessuna
	 * `ldrb` -- ed e' cio' che ci si aspetta da `buf[3]` mai riletto.
	 */
	for (i = 0; i < 4; i++) {
		if (ilitek_ice_mode_write(0x41008, 0xFF, 1) < 0)
			/* riga 1375;
			 * "\x013ILITEK: (%s, %d): Write dummy failed\n"@0xffffff80092411f0 */
#line 1375
			ILI_ERR("Write dummy failed\n");

		if (ilitek_ice_mode_read(0x41010, &tmp, sizeof(u8)) < 0)
			/* riga 1378;
			 * "\x013ILITEK: (%s, %d): Read flash info error\n"@0xffffff80092465ff */
#line 1378
			ILI_ERR("Read flash info error\n");

		buf[i] = tmp;
	}

	if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
		/* riga 1384;
		 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
#line 1384
		ILI_ERR("Write cs high failed\n");

	/* "394013f5 ldrb"@0xffffff8008a6ab18 */
	flash_mid = buf[0];
	/* "33181f59 bfi"@0xffffff8008a6ac6c -- il byte alto e' il SECONDO
	 * letto e il basso il TERZO */
	flash_id = buf[1] << 8 | buf[2];

	/*
	 * La ricerca e' srotolata sulle otto voci: otto coppie
	 * `ldrh`+`cmp`+`b.ne`, l'ultima delle quali
	 * ("7949e103 ldrh"@0xffffff8008a6ada0) salta al ramo "non trovato".
	 */
	for (i = 0; i < ARRAY_SIZE(ilitek_flashtab); i++) {
		if (flash_mid == ilitek_flashtab[i].c0 &&
		    flash_id == ilitek_flashtab[i].c2) {
			idev->c544 = ilitek_flashtab[i].c0;
			idev->c546 = ilitek_flashtab[i].c2;
			idev->c548 = ilitek_flashtab[i].c8;
			idev->c552 = ilitek_flashtab[i].c12;
			break;
		}
	}

	if (i >= ARRAY_SIZE(ilitek_flashtab)) {
		/* riga 1401. Livello \x016, nessuna guardia;
		 * "\x016ILITEK: (%s, %d): Not found flash id in tab, use default\n"@0xffffff800924662a */
#line 1401
		ILI_INFO("Not found flash id in tab, use default\n");
		idev->c544 = ilitek_flashtab[0].c0;
		idev->c546 = ilitek_flashtab[0].c2;
		idev->c548 = ilitek_flashtab[0].c8;
		idev->c552 = ilitek_flashtab[0].c12;
	}

	/* riga 1408. I due argomenti NON sono ricaricati: sono gli stessi
	 * registri appena scritti. */
	/* "\x016ILITEK: (%s, %d): Flash MID = %x, Flash DEV_ID = %x\n"@0xffffff8009246666 */
#line 1408
	ILI_INFO("Flash MID = %x, Flash DEV_ID = %x\n", idev->c544, idev->c546);
	/* riga 1409. Questo INVECE e' ricaricato
	 * ("b9422503 ldr"@0xffffff8008a6ae20), perche' la `printk` di sopra
	 * puo' avere scritto in memoria. */
	/* "\x016ILITEK: (%s, %d): Flash program page = %d\n"@0xffffff800924669d */
#line 1409
	ILI_INFO("Flash program page = %d\n", idev->c548);
	/* riga 1410 ("b9422903 ldr"@0xffffff8008a6ae3c);
	 * "\x016ILITEK: (%s, %d): Flash sector = %d\n"@0xffffff80092466ca */
#line 1410
	ILI_INFO("Flash sector = %d\n", idev->c552);

	ilitek_tddi_flash_protect(false);

	if (!ice) {
		if (ilitek_ice_mode_ctrl(false, false) < 0)
			/* riga 1416;
			 * "\x013ILITEK: (%s, %d): Disable ice mode failed while reading flash info\n"@0xffffff80092466f1 */
#line 1416
			ILI_ERR("Disable ice mode failed while reading flash info\n");
	}
}

/*
 * ===========================================================================
 * IL SECONDO LOTTO: `ilitek_tddi_fw_upgrade` E LE OTTO INCORPORATE
 * ===========================================================================
 * Tutto cio' che segue e' la TREDICESIMA funzione di mappa del gruppo G,
 * `ilitek_tddi_fw_upgrade` (0xffffff8008a684e0, 8824 byte, 2206 istruzioni,
 * il 50,8 % del blocco), piu' le OTTO funzioni che il compilatore le ha
 * incorporato dentro. Che siano otto funzioni separate e non un corpo unico
 * lo dice il `__func__` di ogni `printk`: sessanta `printk` dentro un unico
 * simbolo di mappa, con NOVE `__func__` distinti.
 *
 *   __func__ (dal binario)                indirizzo del letterale   righe
 *   ilitek_tddi_fw_check_hex_hw_crc       0xffffff8009247340        225..233
 *   ilitek_tddi_fw_check_ver              0xffffff8009246f12        661..700
 *   ilitek_tddi_fw_flash_program          0xffffff8009247127        718..766
 *   ilitek_tddi_fw_flash_upgrade          0xffffff8009246e19        849..894
 *   ilitek_tddi_fw_update_block_info      0xffffff8009246d4d        941..953
 *   ilitek_tddi_fw_ili_convert            0xffffff8009246c37        969..1033
 *   ilitek_tddi_fw_hex_convert            0xffffff8009246a8b        1080..1115
 *   ilitek_tdd_fw_hex_open                0xffffff8009246813        1149..1243
 *   ilitek_tddi_fw_upgrade                0xffffff800924636b        1261..1311
 *
 * `ilitek_tdd_fw_hex_open` PORTA IL REFUSO DEL BINARIO -- "tdd", non "tddi".
 * Si legge coi byte, non con l'occhio:
 *   leggi_stringa.py oracolo/stock.elf 0xffffff8009246813
 *   0xffffff8009246813: b'ilitek_tdd_fw_hex_open'  (len=22, hex=696c6974656b5f7464645f66775f6865785f6f70656e)
 * Il quinto e il sesto byte del nome sono 74 64 64 5f = "tdd_". Una
 * riscrittura "pulita" lo normalizzerebbe in silenzio, e sarebbe un difetto.
 *
 * NOVE __func__ MA OTTO INCORPORATE: il nono e' `ilitek_tddi_fw_upgrade`
 * stessa. La ricognizione ne contava dieci in tutto il gruppo (nove piu'
 * `ilitek_tddi_flash_protect`); con `calc_crc32`, che non stampa e quindi
 * nessun censimento per `__func__` poteva vedere, sono UNDICI, di cui NOVE
 * dentro questa funzione (le otto piu' `calc_crc32`, che qui e' srotolata
 * DUE volte, in `ilitek_tddi_fw_check_ver` e in
 * `ilitek_tddi_fw_check_hex_hw_crc`).
 *
 * ---------------------------------------------------------------------------
 * COSA HA DECISO QUESTO LOTTO CHE IL PRECEDENTE AVEVA LASCIATO APERTO
 * ---------------------------------------------------------------------------
 * G2, G2b e G2c erano tre divergenze APERTE del lotto parziale, e la loro
 * chiusura era condizionata a questa funzione. Sono chiuse:
 *   - `ilitek_fw_blk` e' tornata `static` (G2);
 *   - `ilitek_tddi_fw_flash_erase` e `ilitek_fw_calc_file_crc` sono tornate
 *     `static`, che e' la `t` minuscola della mappa (G2b);
 *   - il `noinline` di `ilitek_tddi_flash_poll_busy` e' stato tolto (G2c).
 * E la condizione della prima e' MISURATA, non supposta: nessuna funzione
 * fuori dal gruppo G tocca nessuno dei globali fra 0xffffff800a0ff5a8 e
 * 0xffffff800a0ff6ab. La scansione e' su TUTTO il testo del kernel di
 * fabbrica, non su una finestra (classe C3):
 *   objdump -d --start-address 0xffffff8008080000 --stop-address 0xffffff80093a8518
 *   + tracciamento adrp->pagina per registro, azzerato a ogni simbolo
 *   0xffffff800a0ff5a8 {'ilitek_tddi_fw_upgrade': 11}
 *   0xffffff800a0ff5b0 {'ilitek_tddi_fw_upgrade': 8}
 *   0xffffff800a0ff5b8 {'ilitek_tddi_fw_upgrade': 8, 'ilitek_tddi_fw_flash_erase': 1, 'ilitek_fw_calc_file_crc': 1}
 *   0xffffff800a0ff5c4 {'ilitek_tddi_fw_upgrade': 1}
 *   0xffffff800a0ff5d8 {'ilitek_tddi_fw_upgrade': 1}
 *   0xffffff800a0ff5e0 {'ilitek_tddi_fw_flash_erase': 3}
 *   0xffffff800a0ff5f0 {'ilitek_tddi_fw_upgrade': 1}
 *   0xffffff800a0ff680 {'ilitek_tddi_fw_upgrade': 1}
 *   0xffffff800a0ff698 {'ilitek_tddi_fw_upgrade': 3}
 *   0xffffff800a0ff69c {'ilitek_tddi_fw_upgrade': 3}
 *   0xffffff800a0ff6a0 {'ilitek_tddi_fw_upgrade': 6}
 *   0xffffff800a0ff6a4 {'ilitek_tddi_fw_upgrade': 2}
 *   0xffffff800a0ff6a8 {'ilitek_tddi_fw_upgrade': 4}
 * TRE nomi in tutto, e sono le tre funzioni di questo file. H5 del delta di
 * header CADE: `ilitek_fw_blk` NON e' letta dal gruppo H.
 * Il LIMITE di questa scansione va detto: si ferma a `_einittext`, quindi
 * non copre la `.exit.text`. Quel pezzo e' coperto separatamente -- vedi la
 * sezione LA `.exit` piu' sopra, che scandisce `.kernel` e `.kernel2` per
 * intero e trova ZERO `adrp` verso 0xffffff800a0ff000 oltre `_einittext`.
 */

/*
 * ===========================================================================
 * LE FUNZIONI DEL GRUPPO D E DEL GRUPPO A CHE QUESTA PARTE CHIAMA
 * ===========================================================================
 * Come sopra: prototipi qui, definizioni in un'unita' non ancora scritta.
 * NON sono stub. `ilitek_tddi_reset_ctrl` sta gia' in `ilitek.h` (gruppo A).
 *
 *  - `ilitek_tddi_ic_watch_dog_ctrl`: due argomenti, e questo blocco passa
 *    sempre (1, 0) -- "320003e0 orr"@0xffffff8008a69a10 +
 *    "2a1f03e1 mov"@0xffffff8008a69a14 -- e prova il ritorno sul solo bit di
 *    segno ("37f81360 tbnz"@0xffffff8008a69a1c). La larghezza dei due
 *    argomenti NON e' misurata da qui.
 *  - `ilitek_tddi_ic_get_fw_ver`: nessun argomento (x0 non e' preparato
 *    prima di "97ffc18c bl"@0xffffff8008a699d4), ritorno provato sul bit di
 *    segno ("36f800a0 tbz"@0xffffff8008a699e0).
 *  - `ilitek_tddi_ic_get_core_ver`, `..._get_protocl_ver`, `..._get_tp_info`,
 *    `..._get_panel_info`: nessun argomento e ritorno IGNORATO -- cinque
 *    `bl` consecutive senza una sola istruzione in mezzo,
 *    "97ffbea9 bl"@0xffffff8008a6a2d0, "97ffc0d2 bl"@0xffffff8008a6a2d4,
 *    "97ffbf4b bl"@0xffffff8008a6a2d8, "97ffc042 bl"@0xffffff8008a6a2dc,
 *    "97ffbfbb bl"@0xffffff8008a6a2e0.
 *    (`protocl` e' scritto cosi' nella mappa dell'oracolo: e' il nome che
 *    sta nei byte, non un refuso di trascrizione.)
 */
int ilitek_tddi_ic_watch_dog_ctrl(bool write, bool enable);
int ilitek_tddi_ic_get_fw_ver(void);
int ilitek_tddi_ic_get_core_ver(void);
int ilitek_tddi_ic_get_protocl_ver(void);
int ilitek_tddi_ic_get_tp_info(void);
int ilitek_tddi_ic_get_panel_info(void);

/*
 * I codici d'errore. Sono le SETTE costanti negative che
 * `ilitek_tddi_fw_upgrade` mette in w20 prima di uscire, lette una per una
 * dalle `mov` che le materializzano. I VALORI sono misurati; gli
 * IDENTIFICATORI sono SCELTI -- il binario non contiene nessuna di queste
 * stringhe.
 *   "12800e34 mov"@0xffffff8008a698dc  w20 = 0xffffff8e = -114
 *   "12800e54 mov"@0xffffff8008a69c80  w20 = 0xffffff8d = -115
 *   "12800e74 mov"@0xffffff8008a69c88  w20 = 0xffffff8c = -116
 *   "12800e94 mov"@0xffffff8008a699b0  w20 = 0xffffff8b = -117
 *   "12800eb4 mov"@0xffffff8008a69c78  w20 = 0xffffff8a = -118
 *   "12800ed4 mov"@0xffffff8008a6a0e0  w20 = 0xffffff89 = -119
 *   "12800ef4 mov"@0xffffff8008a6a160  w20 = 0xffffff88 = -120
 * L'ottavo valore d'uscita e' -12 ("12800174 mov"@0xffffff8008a68734), che
 * e' `-ENOMEM` e non ha bisogno di un nome nuovo.
 */
#define EFW_CONVERT_FILE	114
#define EFW_ICE_MODE		115
#define EFW_WDT			116
#define EFW_CRC			117
#define EFW_REST		118
#define EFW_ERASE		119
#define EFW_PROGRAM		120

/*
 * `ERR_ALLOC_MEM` e `ipio_vfree` sono due MACRO, non due funzioni, e la
 * differenza e' visibile: il loro corpo compare in linea in undici punti
 * senza nessuna `bl` e senza nessun `__func__` proprio.
 *
 * `ERR_ALLOC_MEM` e' la coppia "b140041f cmn"@0xffffff8008a6856c +
 * "b4000ca3 cbz"@0xffffff8008a68578: `cmn x, #0x1, lsl #12` somma 4096 e il
 * `b.hi` che segue e' esattamente `IS_ERR_VALUE` (>= -MAX_ERRNO senza
 * segno), il `cbz` e' il test di NULL. Sono le due meta' di
 * `IS_ERR(m) || !m`, in quest'ordine.
 *
 * `ipio_vfree` e' la terna "b4000060 cbz"@0xffffff8008a68728 +
 * "97df66c3 bl"@0xffffff8008a6872c (verso `vfree`) +
 * "f902d6bf str"@0xffffff8008a68730 (azzeramento della variabile): salta
 * la `vfree` se il puntatore e' NULL e azzera SEMPRE dopo. NOMI SCELTI --
 * il binario non li contiene.
 */
#define ERR_ALLOC_MEM(m)	((IS_ERR(m) || (m) == NULL) ? 1 : 0)

#define ipio_vfree(v)				\
	do {					\
		if (*(v) != NULL) {		\
			vfree(*(v));		\
			*(v) = NULL;		\
		}				\
	} while (0)

/*
 * La taglia del buffer di firmware. "52900000 mov"@0xffffff8008a68558 +
 * "72a00040 movk"@0xffffff8008a6855c = 0x28000 = 163840, che e' sia la
 * taglia della `vmalloc` sia il limite superiore dell'indirizzo che
 * `ilitek_tddi_fw_hex_convert` accetta ("6b10007f cmp"@0xffffff8008a68e74).
 * NOME SCELTO.
 */
#define MAX_HEX_FILE_SIZE	0x28000

/*
 * Il numero di voci di `ilitek_fw_blk`. E' 7, e lo dicono cinque confronti
 * indipendenti: "f1001e9f cmp"@0xffffff8008a69aa8,
 * "f1001e9f cmp"@0xffffff8008a69bf4, "f1001e9f cmp"@0xffffff8008a69f78,
 * "f1001e9f cmp"@0xffffff8008a6a098 e "f1001f9f cmp"@0xffffff8008a6a550.
 * Il 6 che i due messaggi d'errore stampano e' 7-1
 * ("321f07e4 orr"@0xffffff8008a6a6f8, "321f07e4 orr"@0xffffff8008a6a720,
 * "321f07e4 orr"@0xffffff8008a6a744). NOME SCELTO.
 */
#define FW_BLOCK_INFO_NUM	7

/*
 * I due metodi di apertura del file. I VALORI sono misurati -- lo `switch`
 * di `ilitek_tdd_fw_hex_open` confronta con 1 ("7100051f cmp"@0xffffff8008a685f4)
 * e con 0 ("350014c8 cbnz"@0xffffff8008a685fc) -- e i NOMI vengono dalle due
 * stringhe che il messaggio della riga 1149 stampa:
 * "REQUEST_FIRMWARE"@0xffffff8009246834 e "FILP_OPEN"@0xffffff800924682a.
 * E' l'eccezione alla regola 5: qui il binario NOMINA i due valori.
 */
#define REQUEST_FIRMWARE	0
#define FILP_OPEN		1

/*
 * ===========================================================================
 * I GLOBALI DEL LOTTO -- adesso hanno la funzione che li usa, e un tipo
 * ===========================================================================
 * Il lotto precedente li aveva DICHIARATI MANCANTI apposta: «dichiararli
 * senza la funzione che li usa vorrebbe dire scegliere un tipo che nessuna
 * riga di questo lotto misura». Adesso la funzione c'e' e ogni tipo ha una
 * istruzione che lo decide.
 *
 * TUTTI E SETTE SONO `static`, e la condizione e' misurata sopra: nessun
 * simbolo fuori dal gruppo G li tocca.
 */

/*
 * 0xffffff800a0ff5a8. Otto byte: "f942d503 ldr"@0xffffff8008a6852c e
 * "f902d6a0 str"@0xffffff8008a68570 sono a larghezza piena, ed e' il
 * risultato di `vmalloc` ("97df68c4 bl"@0xffffff8008a68560).
 *
 * IL NOME VIENE DAL BINARIO, ed e' l'unico dei sette per cui sia cosi': il
 * formato "\x013ILITEK: (%s, %d): Failed to allocate pfw memory, %ld\n"
 * @0xffffff8009246333 e' il messaggio di fallimento PROPRIO di questa
 * `vmalloc` -- la `bl printk` che lo porta ("97db2b6d bl"@0xffffff8008a68720)
 * e' il ramo d'errore di quella `str`. E' un'inferenza di UN passo e va
 * letta come tale.
 */
u8 *pfw;

/*
 * 0xffffff800a0ff5b0. Otto byte ("f902dae8 str"@0xffffff8008a6a260,
 * "f942dae8 ldr"@0xffffff8008a6a480), copia di `idev->c696`
 * ("f9415d28 ldr"@0xffffff8008a6a24c) e controllato con `ERR_ALLOC_MEM`
 * ("b140051f cmn"@0xffffff8008a6a250). E' il buffer del file ILI: tutti i
 * suoi usi stanno in `ilitek_tddi_fw_ili_convert`. NOME SCELTO.
 */
static u8 *ilitek_ilifile;

/*
 * 0xffffff800a0ff698. UN byte: "391a6154 strb"@0xffffff8008a696c0 e
 * "395a6145 ldrb"@0xffffff8008a69948, mai letto piu' largo.
 * NOME dal formato
 * "\x016ILITEK: (%s, %d): star_addr = 0x%06X, end_addr = 0x%06X, Block Num = %d\n"@0xffffff8009246d95,
 * il cui terzo argomento e' proprio questo byte
 * ("395a6145 ldrb"@0xffffff8008a69948 -> w5). Inferenza di un passo.
 */
static u8 ilitek_blk_num;

/*
 * 0xffffff800a0ff69c e 0xffffff800a0ff6a0. Quattro byte ciascuno, letti a
 * larghezza piena ("b9469d03 ldr"@0xffffff8008a69940,
 * "b946a124 ldr"@0xffffff8008a69944) e scritti a larghezza piena
 * ("b9069d1c str"@0xffffff8008a696b8, "b906a136 str"@0xffffff8008a696bc).
 * I NOMI vengono dallo stesso formato di sopra: sono il primo e il secondo
 * argomento. `star_addr` E' SCRITTO COSI' NEL BINARIO, senza la `t`: si
 * legge coi byte,
 *   0xffffff8009246d95: b'\x016ILITEK: (%s, %d): star_addr = 0x%06X, end_addr = 0x%06X, Block Num = %d\n'
 * e va tenuto.
 * Il confronto di 0xffffff800a0ff6a0 dentro `ilitek_tddi_fw_flash_program`
 * e' SENZA segno ("6b09017f cmp"@0xffffff8008a69d88 +
 * "54000069 b.ls"@0xffffff8008a69d8c) e la divisione che lo usa e' `udiv`
 * ("1ac90949 udiv"@0xffffff8008a69f3c): e' quindi `u32`, non `int`.
 */
static u32 ilitek_star_addr;
static u32 ilitek_end_addr;

/*
 * 0xffffff800a0ff6a4. Quattro byte ("b906a503 str"@0xffffff8008a6992c,
 * "b946a523 ldr"@0xffffff8008a69ac0). NOME dal formato
 * "\x016ILITEK: (%s, %d): New FW ver = 0x%x\n"@0xffffff8009246d6e, il cui
 * unico argomento e' il valore appena scritto qui
 * ("5ac00903 rev"@0xffffff8008a69924 -> w3 -> str).
 */
static u32 ilitek_new_ver;

/*
 * 0xffffff800a0ff6a8. QUATTRO byte, e questa e' la parte che conta: i soli
 * valori che il blocco gli assegna sono 0 ("b906a91f str"@0xffffff8008a6a434),
 * 174 ("528015c9 mov"@0xffffff8008a6a6a0) e 175
 * ("528015e9 mov"@0xffffff8008a6a450), che stanno tutti in un byte, ma la
 * LETTURA e' a 32 bit pieni ("b946a908 ldr"@0xffffff8008a68c48) seguita da
 * "7102bd1f cmp"@0xffffff8008a68c4c. La ricognizione (§6.1) segnalava
 * questo come «l'unico caso che merita un dubbio» e avvertiva di non
 * dichiararlo `u8`. E' `u32`, e il tipo e' quello che riproduce la `ldr w`.
 * NOME SCELTO: il binario non lo nomina. Cio' che il codice ne fa e'
 * misurato -- vale 0xAF quando il file ha i marcatori di blocco e 0xAE
 * quando non li ha, ed e' la condizione che abilita il trattamento del
 * tag 0xB0 ("7102bd1f cmp"@0xffffff8008a68c4c).
 */
static u32 ilitek_hex_tag;

/*
 * ===========================================================================
 * hex_to_dec -- INCORPORATA e SROTOLATA, ventitre' copie
 * ===========================================================================
 * Non ha `printk`, quindi non lascia ne' nome ne' numero di riga: come
 * `calc_crc32`, e' un ritrovamento che il censimento per `__func__` non
 * poteva fare. Che sia una funzione separata lo dice la FORMA: la stessa
 * catena di quattro blocchi
 *   "5100c109 sub"@0xffffff8008a68948   c - '0'
 *   "7100293f cmp"@0xffffff8008a6894c   < 10
 *   "51018509 sub"@0xffffff8008a6895c   c - 'a'
 *   "7100193f cmp"@0xffffff8008a68960   < 6
 *   "51010509 sub"@0xffffff8008a68970   c - 'A'
 *   "7100153f cmp"@0xffffff8008a68974   <= 5
 *   "128006c9 mov"@0xffffff8008a6897c   offset -55
 * compare VENTITRE volte dentro `ilitek_tddi_fw_upgrade`, con `len`
 * costante 2, 4 o 6, e le tre costanti -48, -87, -55 sono sempre le stesse
 * ("128005e9 mov"@0xffffff8008a68954, "12800ac9 mov"@0xffffff8008a68968).
 * Il montaggio finale e' la piramide di scorrimenti a passo 4:
 *   len=2:  "531c6d08 lsl"@0xffffff8008a689c0        (<< 4)
 *   len=4:  "53144d08 lsl"@0xffffff8008a68ad4        (<< 12)
 *   len=6:  "530c2d08 lsl"@0xffffff8008a691ec        (<< 20)
 * cioe' `s = (len - 1) * 4` che scende di 4 a ogni cifra.
 *
 * IL RITORNO E' -1 SU CARATTERE NON VALIDO, e le due forme che il binario
 * porta sono la stessa cosa vista da due lati:
 *   "12800018 mov"@0xffffff8008a689d0   w24 = -1, dove il risultato e' un `int`
 *   "32001fec orr"@0xffffff8008a68f18   w12 = 0xff, dove il risultato e'
 *                                       subito troncato da uno `strb`
 *                                       ("38234a6c strb"@0xffffff8008a68f1c)
 * Il secondo e' il primo con la sola larghezza che serve: clang non
 * materializza i 32 bit di -1 quando ne memorizza 8.
 *
 * `phex` E' INDICIZZATO CON `ldrb`, non con `ldrsb`
 * ("39400148 ldrb"@0xffffff8008a68944): su AArch64 `char` e' SENZA segno,
 * quindi `char *` e `u8 *` producono lo stesso codice e il binario non
 * distingue. NOME SCELTO per la funzione e per i suoi locali: il binario
 * non li contiene.
 */
static int hex_to_dec(char *phex, s32 len)
{
	int ret = 0, i = 0, temp = 0, s = 0;

	for (i = 0, s = (len - 1) * 4; i < len; i++, s -= 4) {
		if ((phex[i] >= '0') && (phex[i] <= '9'))
			temp = phex[i] - '0';
		else if ((phex[i] >= 'a') && (phex[i] <= 'f'))
			temp = (phex[i] - 'a') + 10;
		else if ((phex[i] >= 'A') && (phex[i] <= 'F'))
			temp = (phex[i] - 'A') + 10;
		else
			return -1;

		ret |= (temp << s);
	}

	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_check_hex_hw_crc -- INCORPORATA, righe 225..233
 * ===========================================================================
 * `__func__` = "ilitek_tddi_fw_check_hex_hw_crc"@0xffffff8009247340, che non
 * e' quello del contenitore: e' una funzione separata.
 *
 * La `printk` della riga 225 e' PROTETTA dal byte diagnostico
 * ("39656108 ldrb"@0xffffff8008a6a058 + "34000148 cbz"@0xffffff8008a6a060),
 * quindi `ILI_DBG` e non `ILI_INFO`. Quella della riga 228 e' `\x013` e
 * quella della riga 233 e' `\x016` senza guardia.
 *
 * `calc_crc32` E' SROTOLATA QUI, ed e' la stessa di
 * `ilitek_fw_calc_file_crc`: otto copie di
 * "531f794b lsl"@0xffffff8008a69fc8 + "4a17016a eor"@0xffffff8008a69fd0 +
 * "1a8bb14a csel"@0xffffff8008a69fd4, con il polinomio in w23
 * ("5283b6f7 mov"@0xffffff8008a69960 + "72a09837 movk"@0xffffff8008a69968 =
 * 0x04C11DB7) tenuto vivo attraverso TUTTA la funzione, e il valore
 * iniziale -1 ("1280001b mov"@0xffffff8008a69fac).
 *
 * I DUE ARGOMENTI DI `ilitek_tddi_fw_read_hw_crc` SONO GLI STESSI DUE DEL
 * CRC, e non e' una scelta: il binario NON li ricalcola. w0 e w1 sono posti
 * prima del ciclo ("b9400920 ldr"@0xffffff8008a69fa4 e
 * "6b000101 subs"@0xffffff8008a69fb0) e la `bl`
 * ("97fff6c4 bl"@0xffffff8008a6a050) li trova ancora li'; il ciclo usa una
 * copia in w9 ("2a0003e9 mov"@0xffffff8008a69fb8) proprio per non
 * distruggerli.
 */
static int ilitek_tddi_fw_check_hex_hw_crc(u8 *pfw)
{
	u32 i, len = 0, hex_crc, hw_crc;

	/* "f1001e9f cmp"@0xffffff8008a6a098 */
	for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
		/* "340007a8 cbz"@0xffffff8008a69fa0 */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		/* "51000d08 sub"@0xffffff8008a69fa8 -- meno TRE, non meno
		 * quattro: gli ultimi tre byte del blocco non entrano nel CRC */
		len = ilitek_fw_blk[i].c12 - ilitek_fw_blk[i].c8 - 3;

		hex_crc = calc_crc32(ilitek_fw_blk[i].c8, len, pfw);
		hw_crc = ilitek_tddi_fw_read_hw_crc(ilitek_fw_blk[i].c8, len);

		/* riga 225;
		 * "\x016ILITEK: (%s, %d): Block = %d, Hex CRC = %x, HW CRC = %x\n"@0xffffff8009247305 */
		ILI_DBG("Block = %d, Hex CRC = %x, HW CRC = %x\n",
#line 225
			i, hex_crc, hw_crc);

		/* "6b15037f cmp"@0xffffff8008a6a088 */
		if (hex_crc != hw_crc) {
			/* riga 228;
			 * "\x013ILITEK: (%s, %d): Hex and HW CRC NO matched !!!\n"@0xffffff8009247360 */
#line 228
			ILI_ERR("Hex and HW CRC NO matched !!!\n");
			return -1;
		}
	}

	/* riga 233;
	 * "\x016ILITEK: (%s, %d): Hex and HW CRC match!\n"@0xffffff8009247393 */
#line 233
	ILI_INFO("Hex and HW CRC match!\n");
	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_check_ver -- INCORPORATA, righe 661..700
 * ===========================================================================
 * `__func__` = "ilitek_tddi_fw_check_ver"@0xffffff8009246f12.
 *
 * TRE CONTROLLI IN CASCATA, e il ritorno 0 significa «il firmware e' gia'
 * il piu' recente, non aggiornare»: e' l'unico percorso che porta alla riga
 * 700 e da li' alla disabilitazione della modalita' ICE.
 *
 * L'ARRAY DEI CRC HARDWARE e' vero: il primo ciclo scrive
 * "b8347b20 str"@0xffffff8008a69a74 con indice `x20 << 2` e il secondo
 * rilegge "b8747b35 ldr"@0xffffff8008a69bc0 con lo stesso indice. Un locale
 * scalare non basterebbe: i due cicli sono separati da due `printk` e da un
 * confronto di versione.
 *
 * IL CRC LETTO DAL FLASH E' A BYTE INVERTITI. "5ac0091c rev"@0xffffff8008a69a68
 * prende i quattro byte che `ilitek_tddi_fw_read_flash_data` ha appena
 * scritto nel locale a [sp,#40] ("b9402be8 ldr"@0xffffff8008a69a54) e li
 * rovescia. `rev` a 32 bit su little-endian e' esattamente `be32_to_cpu`.
 *
 * IL LOCALE E' AZZERATO PRIMA DEL CICLO ("b9002bff str"@0xffffff8008a69a2c)
 * E RIAZZERATO A OGNI GIRO RIUSCITO ("b9002bff str"@0xffffff8008a69aa0):
 * sono due istruzioni distinte a due indirizzi distinti, quindi due
 * assegnamenti distinti nel sorgente.
 */
static int ilitek_tddi_fw_check_ver(u8 *pfw)
{
	u32 i, crc;
	u32 flash_crc = 0;
	u32 hw_crc[FW_BLOCK_INFO_NUM];

	/* "f1001e9f cmp"@0xffffff8008a69aa8 */
	for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
		/* "34000395 cbz"@0xffffff8008a69a34 */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		/* "51000ea0 sub"@0xffffff8008a69a3c -- i tre byte finali del
		 * blocco piu' il quarto, letti come un intero da quattro:
		 * l'intervallo passato e' [c12-3, c12] e la lunghezza 4
		 * ("321e03e3 orr"@0xffffff8008a69a44) */
		if (ilitek_tddi_fw_read_flash_data(ilitek_fw_blk[i].c12 - 3,
						   ilitek_fw_blk[i].c12,
						   (u8 *)&flash_crc, 4) < 0) {
			/* riga 661;
			 * "\x013ILITEK: (%s, %d): Read Flash failed\n"@0xffffff8009246eeb */
#line 661
			ILI_ERR("Read Flash failed\n");
			return -1;
		}

		/* "321e7be9 orr"@0xffffff8008a69a58 = -3, poi
		 * "4b1b0129 sub"@0xffffff8008a69a5c e
		 * "0b150121 add"@0xffffff8008a69a60: la lunghezza e'
		 * c12 - c8 - 3, la stessa di `check_hex_hw_crc` */
		hw_crc[i] = ilitek_tddi_fw_read_hw_crc(ilitek_fw_blk[i].c8,
					ilitek_fw_blk[i].c12 -
					ilitek_fw_blk[i].c8 - 3);

		/* riga 669;
		 * "\x016ILITEK: (%s, %d): Block = %d, HW CRC = 0x%06x, Flash CRC = 0x%06x\n"@0xffffff8009246f2b */
		ILI_INFO("Block = %d, HW CRC = 0x%06x, Flash CRC = 0x%06x\n",
#line 669
			 i, hw_crc[i], be32_to_cpu(flash_crc));

		/* "6b15039f cmp"@0xffffff8008a69a98 */
		if (be32_to_cpu(flash_crc) != hw_crc[i]) {
			/* riga 673;
			 * "\x016ILITEK: (%s, %d): HW and Flash CRC not matched, do upgrade\n"@0xffffff8009246f70 */
#line 673
			ILI_INFO("HW and Flash CRC not matched, do upgrade\n");
			return -1;
		}

		flash_crc = 0;
	}

	/* riga 680. `idev->c48` e' letto DUE volte -- prima per la stampa
	 * ("f9401908 ldr"@0xffffff8008a69ac8 + "b9403104 ldr"@0xffffff8008a69ad8)
	 * e poi per il confronto ("f9401908 ldr"@0xffffff8008a69aec +
	 * "b9403108 ldr"@0xffffff8008a69af0) -- perche' `printk` puo' scrivere
	 * in memoria e il compilatore non puo' tenerlo in un registro;
	 * "\x016ILITEK: (%s, %d): New FW ver = 0x%x, Current FW ver = 0x%x\n"@0xffffff8009246fae */
	ILI_INFO("New FW ver = 0x%x, Current FW ver = 0x%x\n",
#line 680
		 ilitek_new_ver, ((struct ilitek_g_c48 *)idev->c48)->c48);

	/* "6b08013f cmp"@0xffffff8008a69af4 */
	if (ilitek_new_ver != ((struct ilitek_g_c48 *)idev->c48)->c48) {
		/* riga 682;
		 * "\x016ILITEK: (%s, %d): FW version not matched, do upgrade\n"@0xffffff8009246fec */
#line 682
		ILI_INFO("FW version not matched, do upgrade\n");
		return -1;
	}

	/* "f1001e9f cmp"@0xffffff8008a69bf4 */
	for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
		/* "340006e9 cbz"@0xffffff8008a69b14 */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		crc = calc_crc32(ilitek_fw_blk[i].c8,
				 ilitek_fw_blk[i].c12 -
				 ilitek_fw_blk[i].c8 - 3, pfw);

		/* riga 693. Il terzo argomento e' il CRC HARDWARE salvato dal
		 * primo ciclo, il quarto e' quello appena calcolato sul file:
		 * "2a1503e4 mov"@0xffffff8008a69bd8 (w4 = hw_crc[i]) e
		 * "2a1b03e5 mov"@0xffffff8008a69bdc (w5 = crc);
		 * "\x016ILITEK: (%s, %d): Block = %d, HW CRC = 0x%06x, Hex CRC = 0x%06x\n"@0xffffff8009247024 */
		ILI_INFO("Block = %d, HW CRC = 0x%06x, Hex CRC = 0x%06x\n",
#line 693
			 i, hw_crc[i], crc);

		/* "6b15037f cmp"@0xffffff8008a69be4 */
		if (crc != hw_crc[i]) {
			/* riga 695;
			 * "\x013ILITEK: (%s, %d): Hex and HW CRC not matched, do upgrade\n"@0xffffff8009247067 */
#line 695
			ILI_ERR("Hex and HW CRC not matched, do upgrade\n");
			return -1;
		}
	}

	/* riga 700;
	 * "\x016ILITEK: (%s, %d): Firmware is the newest version, upgrade abort\n"@0xffffff80092470a3 */
#line 700
	ILI_INFO("Firmware is the newest version, upgrade abort\n");
	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_flash_program -- INCORPORATA, righe 718..766
 * ===========================================================================
 * `__func__` = "ilitek_tddi_fw_flash_program"@0xffffff8009247127.
 *
 * I NOMI DEI TRE LOCALI VENGONO DAL BINARIO, ed e' l'eccezione alla regola
 * 5: il formato della riga 759
 * "\x013ILITEK: (%s, %d): Failed to program data at start_addr = 0x%X, k = 0x%X, addr = 0x%x\n"
 * @0xffffff800924716a li nomina uno per uno, e i tre argomenti sono
 * "2a1b03e3 mov"@0xffffff8008a6a124 (w3 = w27), "2a1c03e4 mov"@0xffffff8008a6a128
 * (w4 = w28) e "0b1b0385 add"@0xffffff8008a6a114 (w5 = w28 + w27). Quindi
 * `start_addr` e' l'indirizzo che scorre, `k` e' l'indice dentro la pagina,
 * e `addr` e' la loro somma -- una variabile che NON esiste come tale nel
 * codice, perche' e' calcolata sul posto.
 *
 * LE DUE SOGLIE SONO LE STESSE DI `ilitek_tddi_fw_flash_erase` (divergenza
 * G6): "530d7ca8 lsr"@0xffffff8008a69d14 con "7100391f cmp"@0xffffff8008a69d18
 * e "530c7c88 lsr"@0xffffff8008a69d20 con "7100711f cmp"@0xffffff8008a69d24.
 * Qui pero' il senso e' ROVESCIATO rispetto a `erase`: la` b.hi` salta AL
 * corpo ("54000088 b.hi"@0xffffff8008a69d1c), quindi il blocco si
 * programma quando la condizione NON vale.
 *
 * `skip` NON E' RIAZZERATO A OGNI PAGINA. E' un difetto della fabbrica e si
 * riproduce (regola 7): "320003f5 orr"@0xffffff8008a69cf4 lo mette a 1 UNA
 * volta sola, prima del ciclo sui blocchi, e "2a1f03f5 mov"@0xffffff8008a69f58
 * lo mette a 0 in coda a ogni pagina PROGRAMMATA -- non all'inizio della
 * successiva. Dopo la prima pagina non tutta-0xff, quindi, l'accumulatore
 * resta 0 per sempre e il ramo di uscita anticipata non scatta piu'.
 *
 * E QUEL RAMO RESTITUISCE UN ERRORE, non «salta la pagina». Il blocco
 * 0xffffff8008a6a0e8 ha due soli predecessori,
 * "140000c9 b"@0xffffff8008a69dc4 e "37001915 tbnz"@0xffffff8008a69dc8, e
 * finisce cosi':
 *   "97ffb3e8 bl"@0xffffff8008a6a0f8    -> ilitek_ice_mode_write
 *   "36f80300 tbz"@0xffffff8008a6a100   se ret >= 0 salta la stampa
 *   "52805c42 mov"@0xffffff8008a6a104   riga 738
 *   "12800ef4 mov"@0xffffff8008a6a160   w20 = -120 = -EFW_PROGRAM
 * Non c'e' nessun ritorno al ciclo: e' un `return`. Anche questo si
 * riproduce.
 *
 * LA SCRITTURA DEI DATI PASSA PER IL PUNTATORE A FUNZIONE `idev->c776`
 * ("f9418508 ldr"@0xffffff8008a69eb8 + "d63f0100 blr"@0xffffff8008a69ec0),
 * non per `ilitek_ice_mode_write`.
 */
static int ilitek_tddi_fw_flash_program(u8 *pfw)
{
	u8 buf[512];
	u32 i, addr, k;
	int percent;
	bool skip = true;

	/* "321703e2 orr"@0xffffff8008a69ce4 = 0x200 = 512, e
	 * "2a1f03e1 mov"@0xffffff8008a69ce8 = 0: e' un `memset` esplicito,
	 * non un inizializzatore -- l'array e' azzerato DENTRO il corpo, dopo
	 * la `bl ilitek_tddi_fw_flash_erase` del chiamante */
	memset(buf, 0, sizeof(buf));

	/* "f1001e9f cmp"@0xffffff8008a69f78 */
	for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
		/* "b840cf05 ldr"@0xffffff8008a69d08 -- pre-indicizzata: il
		 * binario tiene vivo &ilitek_fw_blk[i].c12 e ne ricava c8 con
		 * una seconda pre-indicizzata ("b8408f64 ldr"@0xffffff8008a69d10) */
		if (ilitek_fw_blk[i].c12 == 0)
			continue;

		if (ilitek_fw_blk[i].c12 / 8192 <= 14 &&
		    ilitek_fw_blk[i].c8 / 4096 > 28)
			continue;

		/* riga 718;
		 * "\x016ILITEK: (%s, %d): Block[%d]: Programing from (0x%x) to (0x%x)\n"@0xffffff80092470e6 */
		ILI_INFO("Block[%d]: Programing from (0x%x) to (0x%x)\n",
#line 718
			 i, ilitek_fw_blk[i].c8, ilitek_fw_blk[i].c12);

		/* "6b08037f cmp"@0xffffff8008a69d50 + "540010e2 b.cs"@0xffffff8008a69d54
		 * -- guardia SENZA segno */
		for (addr = ilitek_fw_blk[i].c8; addr < ilitek_fw_blk[i].c12;
		     addr += idev->c548) {
			/* "528104aa mov"@0xffffff8008a69d64 +
			 * "72a0820a movk"@0xffffff8008a69d68 +
			 * "b90033ea str"@0xffffff8008a69d6c: 0x04100825
			 * scritto in UNA `str` da quattro byte, che e' come
			 * clang fonde quattro `strb` consecutive di costante */
			buf[0] = 0x25;
			buf[1] = 0x08;
			buf[2] = 0x10;
			buf[3] = 0x04;

			/* "eb0c039f cmp"@0xffffff8008a69db4 -- il limite e'
			 * RILETTO da idev a ogni giro
			 * ("b942250c ldr"@0xffffff8008a69da4), non estratto in
			 * un locale: e' la classe di difetto A2 al contrario,
			 * e va lasciato com'e' */
			for (k = 0; k < idev->c548; k++) {
				/* "6b09017f cmp"@0xffffff8008a69d88 +
				 * "54000069 b.ls"@0xffffff8008a69d8c */
				if ((addr + k) <= ilitek_end_addr)
					buf[4 + k] = pfw[addr + k];
				else
					/* "32001feb orr"@0xffffff8008a69d90 */
					buf[4 + k] = 0xFF;

				/* "7103fd7f cmp"@0xffffff8008a69da8 +
				 * "1a9f17eb cset"@0xffffff8008a69dac +
				 * "0a0b02b5 and"@0xffffff8008a69db8 */
				if (buf[4 + k] != 0xFF)
					skip = false;
			}

			/* "36000095 tbz"@0xffffff8008a69dc0 */
			if (skip) {
				if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
					/* riga 738;
					 * "\x013ILITEK: (%s, %d): Write cs high failed\n"@0xffffff800924625e */
					ILI_ERR("Write cs high failed\n");
				return -EFW_PROGRAM;
			}

			ilitek_tddi_flash_write_enable();

			if (ilitek_ice_mode_write(0x41000, 0x0, 1) < 0)
				/* riga 745;
				 * "\x013ILITEK: (%s, %d): Write cs low failed\n"@0xffffff80092461b2 */
#line 745
				ILI_ERR("Write cs low failed\n");

			/* "52954aa1 mov"@0xffffff8008a69e0c +
			 * "72a00cc1 movk"@0xffffff8008a69e14 = 0x66AA55 */
			if (ilitek_ice_mode_write(0x41004, 0x66AA55, 3) < 0)
				/* riga 748;
				 * "\x013ILITEK: (%s, %d): Write key failed\n"@0xffffff8009241174 */
#line 748
				ILI_ERR("Write key failed\n");

			if (ilitek_ice_mode_write(0x41008, 0x2, 1) < 0)
				/* riga 751;
				 * "\x013ILITEK: (%s, %d): Write 0x2 failed\n"@0xffffff8009247144 */
#line 751
				ILI_ERR("Write 0x2 failed\n");

			/* "12181f61 and"@0xffffff8008a69e74,
			 * "33105f61 bfxil"@0xffffff8008a69e78,
			 * "33101f61 bfi"@0xffffff8008a69e7c -- gli stessi tre
			 * byte scambiati di `ilitek_tddi_fw_flash_erase` */
			if (ilitek_ice_mode_write(0x41008,
						  ((addr & 0xFF0000) >> 16) |
						  (addr & 0x00FF00) |
						  ((addr & 0x0000FF) << 16),
						  3) < 0)
				/* riga 755;
				 * "\x013ILITEK: (%s, %d): Write address failed\n"@0xffffff80092411c6 */
#line 755
				ILI_ERR("Write address failed\n");

			/* "11001121 add"@0xffffff8008a69ebc -- quattro byte di
			 * intestazione piu' la pagina */
			if (idev->c776(buf, idev->c548 + 4) < 0) {
				/* riga 759 */
				ILI_ERR("Failed to program data at start_addr = 0x%X, k = 0x%X, addr = 0x%x\n",
#line 759
					addr, k, addr + k);

				if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
					/* riga 761 */
					ILI_ERR("Write cs high failed\n");
				return -EFW_PROGRAM;
			}

			if (ilitek_ice_mode_write(0x41000, 0x1, 1) < 0)
				/* riga 766 */
#line 766
				ILI_ERR("Write cs high failed\n");

			/* "79444108 ldrh"@0xffffff8008a69efc + "7103bd1f cmp"@0xffffff8008a69f00
			 * -- il campo che il formato a 0xffffff8009246666 chiama
			 * "Flash MID", confrontato con 0xEF */
			if (idev->c544 == 0xEF)
				/* "52912b00 mov"@0xffffff8008a69f08 +
				 * "72a00820 movk"@0xffffff8008a69f0c = 0x418958
				 * = 1000 * 0x10c7. Divergenza G5: `mdelay(1)` e
				 * `udelay(1000)` danno la stessa istruzione */
				mdelay(1);
			else
				/* "52800140 mov"@0xffffff8008a69f18 = 10 */
				if (ilitek_tddi_flash_poll_busy(10) < 0)
					return -EFW_PROGRAM;

			/* "52800caa mov"@0xffffff8008a69f30 = 101,
			 * "1b0a7f6a mul"@0xffffff8008a69f34,
			 * "1ac90949 udiv"@0xffffff8008a69f3c: la divisione e'
			 * SENZA segno, ma il taglio a 90
			 * ("7101693f cmp"@0xffffff8008a69f40 +
			 * "1a8ab129 csel"@0xffffff8008a69f48) e' CON segno --
			 * che e' quello che si ottiene assegnando il quoziente
			 * a un `int` e confrontando quello */
			percent = (addr * 101) / ilitek_end_addr;
			idev->c444 = (percent < 90) ? percent : 90;

			/* "2a1f03f5 mov"@0xffffff8008a69f58 -- vedi il cappello:
			 * `skip` e' azzerato QUI, non all'inizio della pagina */
			skip = false;
		}
	}

	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_flash_upgrade -- INCORPORATA, righe 849..894
 * ===========================================================================
 * `__func__` = "ilitek_tddi_fw_flash_upgrade"@0xffffff8009246e19.
 *
 * E' il corpo del ciclo di ritentativi di `ilitek_tddi_fw_upgrade`: la sua
 * testa e' l'unica destinazione di "54ffc1a1 b.ne"@0xffffff8008a6a184.
 *
 * IL RITORNO E' 0 SU DUE PERCORSI DIVERSI, e tutti e due arrivano allo
 * stesso blocco 0xffffff8008a6a338: l'abbandono per «firmware gia' il piu'
 * recente" ("36f838a0 tbz"@0xffffff8008a69c24) e la programmazione riuscita
 * ("36f81360 tbz"@0xffffff8008a6a0cc). I sei ritorni negativi sono le sei
 * costanti EFW_*.
 *
 * `idev->c636` E' MESSO A 0 E POI A 1 ATTORNO A `ilitek_tddi_ic_get_fw_ver`
 * ("3909f11f strb"@0xffffff8008a699d0 e "3909f109 strb"@0xffffff8008a69a04):
 * sono due `strb` a due indirizzi diversi, quindi due assegnamenti.
 */
static int ilitek_tddi_fw_flash_upgrade(u8 *pfw)
{
	int ret = 0;

	/* "b9426500 ldr"@0xffffff8008a699c0 -- il campo +612, che `ilitek.h`
	 * documenta come il modo di reset */
	ret = ilitek_tddi_reset_ctrl(idev->c612);
	if (ret < 0) {
		/* riga 849;
		 * "\x013ILITEK: (%s, %d): TP reset failed during flash progam\n"@0xffffff8009246de0 */
#line 849
		ILI_ERR("TP reset failed during flash progam\n");
		return -EFW_REST;
	}

	idev->c636 = false;
	/* "36f800a0 tbz"@0xffffff8008a699e0 -- il ritorno e' provato ma
	 * l'errore NON interrompe: si stampa e si prosegue */
	if (ilitek_tddi_ic_get_fw_ver() < 0)
		/* riga 856;
		 * "\x013ILITEK: (%s, %d): Get firmware ver failed before upgrade\n"@0xffffff8009246e36 */
#line 856
		ILI_ERR("Get firmware ver failed before upgrade\n");
	idev->c636 = true;

	ret = ilitek_ice_mode_ctrl(true, false);
	if (ret < 0)
		/* "12800e54 mov"@0xffffff8008a69c80 = -115 */
		return -EFW_ICE_MODE;

	ret = ilitek_tddi_ic_watch_dog_ctrl(true, false);
	if (ret < 0)
		/* "12800e74 mov"@0xffffff8008a69c88 = -116 */
		return -EFW_WDT;

	/* "36f838a0 tbz"@0xffffff8008a69c24 -- se il firmware e' gia' il piu'
	 * recente si esce da qui, senza programmare niente */
	if (ilitek_tddi_fw_check_ver(pfw) == 0)
		goto out;

	/* "3100041f cmn"@0xffffff8008a69cd8 -- il confronto e' con -1 ESATTO,
	 * non con «minore di zero»: `ilitek_tddi_fw_flash_erase` restituisce
	 * solo 0 o -1 */
	if (ilitek_tddi_fw_flash_erase() == -1)
		/* "12800ed4 mov"@0xffffff8008a6a0e0 = -119 */
		return -EFW_ERASE;

	ret = ilitek_tddi_fw_flash_program(pfw);
	if (ret < 0)
		return ret;

	/* "54ffc841 b.ne"@0xffffff8008a6a090 -> "12800e94 mov"@0xffffff8008a699b0
	 * = -117 */
	if (ilitek_tddi_fw_check_hex_hw_crc(pfw) < 0)
		return -EFW_CRC;

	ret = ilitek_tddi_reset_ctrl(idev->c612);
	if (ret < 0) {
		/* riga 894;
		 * "\x013ILITEK: (%s, %d): TP reset failed after flash progam\n"@0xffffff8009246eb3 */
		ILI_ERR("TP reset failed after flash progam\n");
		return -EFW_REST;
	}

	return 0;

out:
	ret = ilitek_ice_mode_ctrl(false, false);
	if (ret < 0) {
		/* riga 870;
		 * "\x013ILITEK: (%s, %d): Disable ice mode failed, call reset instead\n"@0xffffff8009246e72 */
#line 870
		ILI_ERR("Disable ice mode failed, call reset instead\n");

		if (ilitek_tddi_reset_ctrl(idev->c612) < 0) {
			/* riga 872. IL MESSAGGIO E' QUELLO DELLA RIGA 849,
			 * "during" e non "after", ed e' un difetto della
			 * fabbrica che si riproduce (regola 7): le due
			 * "91378000 add"@0xffffff8008a69c54 e
			 * "91378000 add"@0xffffff8008a69c6c portano tutte e due
			 * a 0xffffff8009246de0, mentre la riga 894 usa un
			 * letterale DIVERSO, 0xffffff8009246eb3
			 * ("913acc00 add"@0xffffff8008a6a0d8), che dice
			 * "after". */
#line 872
			ILI_ERR("TP reset failed during flash progam\n");
			return -EFW_REST;
		}
	}

	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_update_block_info -- INCORPORATA, righe 941..953
 * ===========================================================================
 * `__func__` = "ilitek_tddi_fw_update_block_info"@0xffffff8009246d4d.
 *
 * I CINQUE NOMI DEI BLOCCHI SONO LETTERALI DEL BINARIO, e gli indici a cui
 * vanno sono misurati sull'offset della `str`, con base
 * 0xffffff800a0ff5d8 = &ilitek_fw_blk[1]
 * ("91176108 add"@0xffffff8008a69764):
 *   "f9000109 str"@0xffffff8008a69788   [x8,#0]   -> [1]  "AP"@0xffffff800920eb08
 *   "f900110a str"@0xffffff8008a6976c   [x8,#32]  -> [2]  "DATA"@0xffffff800910693a
 *   "f9002109 str"@0xffffff8008a697a4   [x8,#64]  -> [3]  "TUNING"@0xffffff8009246d06
 *   "f9003109 str"@0xffffff8008a697b4   [x8,#96]  -> [4]  "GESTURE"@0xffffff8009246d0d
 *   "f900410a str"@0xffffff8008a69780   [x8,#128] -> [5]  "MP"@0xffffff8009223d90
 * e i cinque byte a +28 di ciascuna voce:
 *   "3900710a strb"@0xffffff8008a69794  [x8,#28]  -> [1] = 1
 *   "3900f10a strb"@0xffffff8008a69798  [x8,#60]  -> [2] = 1
 *   "3901710a strb"@0xffffff8008a69790  [x8,#92]  -> [3] = 1
 *   "3901f109 strb"@0xffffff8008a697e0  [x8,#124] -> [4] = 4
 *   "39027109 strb"@0xffffff8008a697c8  [x8,#156] -> [5] = 5
 * (l'ordine delle istruzioni NON e' l'ordine del sorgente: clang le ha
 * mescolate. L'ordine scritto qui e' una SCELTA.)
 *
 * `AP` e `DATA` e `MP` stanno in pagine di `.rodata` diverse dalle altre
 * ("b00034ea adrp"@0xffffff8008a69760 verso 0xffffff8009106000,
 * "b0003d29 adrp"@0xffffff8008a69774 verso 0xffffff800920e000,
 * "d0003dca adrp"@0xffffff8008a69770 verso 0xffffff8009223000): sono code
 * condivise di stringhe di altri sottosistemi, cosa che il compilatore fa
 * da se' e che non dice niente sul sorgente.
 */
static void ilitek_tddi_fw_update_block_info(u8 *pfw)
{
	u32 index;

	ilitek_fw_blk[1].c0 = "AP";
	ilitek_fw_blk[2].c0 = "DATA";
	ilitek_fw_blk[3].c0 = "TUNING";
	ilitek_fw_blk[4].c0 = "GESTURE";
	ilitek_fw_blk[5].c0 = "MP";

	ilitek_fw_blk[1].c28 = 1;
	ilitek_fw_blk[2].c28 = 1;
	ilitek_fw_blk[3].c28 = 1;
	ilitek_fw_blk[4].c28 = 4;
	ilitek_fw_blk[5].c28 = 5;

	/* "b9400d0a ldr"@0xffffff8008a6979c legge ilitek_fw_blk[1].c12 e
	 * "51013d53 sub"@0xffffff8008a697bc gli toglie 0x4f = 79 */
	index = ilitek_fw_blk[1].c12 - 79;

	/* riga 941;
	 * "\x016ILITEK: (%s, %d): Parsing hex info start addr = 0x%x\n"@0xffffff8009246d15 */
#line 941
	ILI_INFO("Parsing hex info start addr = 0x%x\n", index);

	/* 75 byte, e l'estensione la consegnano le nove `str` da otto piu'
	 * la coda sovrapposta da quattro: "a940292b ldp"@0xffffff8008a697f0
	 * (offset 0), ..., "b844712a ldur"@0xffffff8008a6982c (offset 71).
	 * 71 + 4 = 75. Il bersaglio e' idev + 0x1c4 = 452
	 * ("9107110c add"@0xffffff8008a697f4). */
	memcpy(idev->c452, pfw + index, 75);

	/* "5280804a mov"@0xffffff8008a6985c + "72a0002a movk"@0xffffff8008a69860
	 * = 0x10402, e il confronto e' SENZA segno
	 * ("540003c9 b.ls"@0xffffff8008a6986c). Il campo e' a +52 del blocco
	 * puntato da `idev->c48` ("b9403529 ldr"@0xffffff8008a69864). */
	if (((struct ilitek_g_c48 *)idev->c48)->c52 > 0x010402)
		/* "1a9f07ea cset"@0xffffff8008a69878 -- il byte e' 0 o 1, non
		 * il valore letto */
		idev->c641 = (idev->c452[0] != 0);
	else
		/* "390a051f strb"@0xffffff8008a698e4 */
		idev->c641 = 0;

	/* riga 945. Le due stringhe sono "ON"@0xffffff8009250b5c e
	 * "OFF"@0xffffff8009250b6d, scelte con due `adrp`+`add` distinte e non
	 * con una `csel`, perche' il ramo era gia' separato;
	 * "\x016ILITEK: (%s, %d): Transfer touch coordinate = %s\n"@0xffffff8009241619 */
	ILI_INFO("Transfer touch coordinate = %s\n",
#line 945
		 idev->c641 ? "ON" : "OFF");

	/* "b941f508 ldr"@0xffffff8008a6991c legge i quattro byte a idev+500
	 * -- che stanno dentro `c452[75]`, all'indice 48 -- e
	 * "5ac00903 rev"@0xffffff8008a69924 li rovescia: e' un intero a 32 bit
	 * BIG-ENDIAN dentro il blocco appena copiato dal file */
	ilitek_new_ver = (idev->c452[48] << 24) | (idev->c452[49] << 16) |
			 (idev->c452[50] << 8) | idev->c452[51];

	/* riga 952;
	 * "\x016ILITEK: (%s, %d): New FW ver = 0x%x\n"@0xffffff8009246d6e */
#line 952
	ILI_INFO("New FW ver = 0x%x\n", ilitek_new_ver);

	/* riga 953. `star_addr` e' scritto cosi' NEL BINARIO, senza la `t`, e
	 * si riproduce;
	 * "\x016ILITEK: (%s, %d): star_addr = 0x%06X, end_addr = 0x%06X, Block Num = %d\n"@0xffffff8009246d95 */
	ILI_INFO("star_addr = 0x%06X, end_addr = 0x%06X, Block Num = %d\n",
#line 953
		 ilitek_star_addr, ilitek_end_addr, ilitek_blk_num);
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_ili_convert -- INCORPORATA, righe 969..1033
 * ===========================================================================
 * `__func__` = "ilitek_tddi_fw_ili_convert"@0xffffff8009246c37.
 *
 * IL PRIMO CONTROLLO NON HA MESSAGGIO PROPRIO: "b140051f cmn"@0xffffff8008a6a250
 * e "b4000208 cbz"@0xffffff8008a6a258 saltano direttamente al messaggio
 * della riga 1286, che appartiene al CHIAMANTE. E' un `return -1` muto.
 *
 * IL CONTROLLO DI FORMATO E' UNA CONGIUNZIONE DI QUATTRO DISUGUAGLIANZE, e
 * il senso e' quello che il binario porta, non quello che ci si aspetta:
 * "54000200 b.eq"@0xffffff8008a6a37c e le tre gemelle saltano AVANTI, cioe'
 * BASTA CHE UNO dei quattro byte valga 0xFF perche' il file sia accettato.
 * Solo se tutti e quattro sono diversi da 0xFF si stampa la riga 975.
 *
 * L'INDICE 39 E IL PASSO 6: "528004f3 mov"@0xffffff8008a6a458 = 39 e
 * "91001a73 add"@0xffffff8008a6a54c = +6. Le letture sono a
 * [indice-5 .. indice] ("385fb12a ldurb"@0xffffff8008a6a4c8 fino a
 * "386a6908 ldrb"@0xffffff8008a6a4f8), cioe' 34+6i .. 39+6i.
 *
 * IL CASO SPECIALE i == 5 legge la TESTA del file invece del proprio
 * gruppo: "f101167f cmp"@0xffffff8008a6a484 confronta l'indice con 69, che
 * e' 39 + 6*5, e il ramo che ne segue legge
 * "39400109 ldrb"@0xffffff8008a6a48c ... "3940110a ldrb"@0xffffff8008a6a4b0,
 * cioe' i byte 0..5. Il binario NON distingue `i == 5` da `indice == 69`:
 * vedi la divergenza G11.
 */
static int ilitek_tddi_fw_ili_convert(u8 *pfw)
{
	u32 i, addr, start, end, size, blk;
	u8 type;

	if (ERR_ALLOC_MEM((void *)idev->c696))
		return -1;

	ilitek_ilifile = (u8 *)idev->c696;

	/* "51010133 sub"@0xffffff8008a6a26c -- meno 0x40 = 64, che e'
	 * l'intestazione del file ILI, e
	 * "528ff809 mov"@0xffffff8008a6a270 + "72a00049 movk"@0xffffff8008a6a274
	 * = 0x27fc0 = MAX_HEX_FILE_SIZE - 64 */
	size = idev->c648 - 64;
	if (size > (MAX_HEX_FILE_SIZE - 64)) {
		/* riga 969;
		 * "\x013ILITEK: (%s, %d): size of ILI file is invalid\n"@0xffffff8009246c06 */
#line 969
		ILI_ERR("size of ILI file is invalid\n");
		return -1;
	}

	/* "7103fd3f cmp"@0xffffff8008a6a378 e le tre gemelle */
	if (ilitek_ilifile[22] != 0xFF && ilitek_ilifile[23] != 0xFF &&
	    ilitek_ilifile[24] != 0xFF && ilitek_ilifile[25] != 0xFF) {
		/* riga 975;
		 * "\x013ILITEK: (%s, %d): Invaild ILI format, abort!\n"@0xffffff8009246c52 */
#line 975
		ILI_ERR("Invaild ILI format, abort!\n");
		return -1;
	}

	/* riga 979. I due argomenti sono i byte 32 e 33
	 * ("39408103 ldrb"@0xffffff8008a6a3bc, "39408504 ldrb"@0xffffff8008a6a3c0);
	 * "\x016ILITEK: (%s, %d): Start to parse ILI file, type = %d, block_count = %d\n"@0xffffff8009246c82 */
	ILI_INFO("Start to parse ILI file, type = %d, block_count = %d\n",
#line 979
		 ilitek_ilifile[32], ilitek_ilifile[33]);

	/* "b9069d1f str"@0xffffff8008a6a3f0 */
	ilitek_star_addr = 0;
	memset(ilitek_fw_blk, 0x0, sizeof(ilitek_fw_blk));
	/* "b906a2df str"@0xffffff8008a6a430 */
	ilitek_end_addr = 0;
	/* "b906a91f str"@0xffffff8008a6a434 */
	ilitek_hex_tag = 0;

	/* "3940813b ldrb"@0xffffff8008a6a438 -- UNA sola lettura, e il valore
	 * resta in w27, un registro salvato dal chiamato, attraverso tutte le
	 * `printk` del ciclo ("6a1b011f tst"@0xffffff8008a6a470 e
	 * "36380a1b tbz"@0xffffff8008a6a568 lo riusano). Un `ilitek_ilifile[32]`
	 * riletto dentro il ciclo costringerebbe il compilatore a ricaricarlo
	 * dopo ogni `printk`: quindi nel sorgente di fabbrica e' un LOCALE.
	 * NOME dal formato della riga 979, che lo chiama "type". */
	type = ilitek_ilifile[32];

	/* "3400133b cbz"@0xffffff8008a6a43c */
	if (type) {
		/* "528015e9 mov"@0xffffff8008a6a450 = 0xAF, scritto PRIMA del
		 * ciclo ("b906a909 str"@0xffffff8008a6a468) */
		ilitek_hex_tag = 0xAF;

		/* "f1001f9f cmp"@0xffffff8008a6a550 */
		for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
			/* "1adc22c8 lsl"@0xffffff8008a6a46c +
			 * "6a1b011f tst"@0xffffff8008a6a470 */
			if (!((1 << i) & type))
				continue;

			/* "f1001b9f cmp"@0xffffff8008a6a478 +
			 * "54001322 b.cs"@0xffffff8008a6a47c */
			if (i + 1 > FW_BLOCK_INFO_NUM - 1) {
				/* riga 1001. Il secondo argomento e' la
				 * costante 6 ("321f07e4 orr"@0xffffff8008a6a6f8);
				 * "\x013ILITEK: (%s, %d): ERROR! block num is larger than its define (%d, %d)\n"@0xffffff8009246a42 */
				ILI_ERR("ERROR! block num is larger than its define (%d, %d)\n",
#line 1001
					i + 1, FW_BLOCK_INFO_NUM - 1);
				return -1;
			}

			if (i == 5) {
				start = (ilitek_ilifile[0] << 16) |
					(ilitek_ilifile[1] << 8) |
					 ilitek_ilifile[2];
				end   = (ilitek_ilifile[3] << 16) |
					(ilitek_ilifile[4] << 8) |
					 ilitek_ilifile[5];
			} else {
				start = (ilitek_ilifile[34 + i * 6] << 16) |
					(ilitek_ilifile[35 + i * 6] << 8) |
					 ilitek_ilifile[36 + i * 6];
				end   = (ilitek_ilifile[37 + i * 6] << 16) |
					(ilitek_ilifile[38 + i * 6] << 8) |
					 ilitek_ilifile[39 + i * 6];
			}

			/* "b81f0304 stur"@0xffffff8008a6a4e0 (c8),
			 * "b81f4305 stur"@0xffffff8008a6a508 (c12),
			 * "b9000314 str"@0xffffff8008a6a4fc (c24): tutte e tre
			 * PRIMA del confronto "6b0400bf cmp"@0xffffff8008a6a504 */
			ilitek_fw_blk[i + 1].c8 = start;
			ilitek_fw_blk[i + 1].c12 = end;
			ilitek_fw_blk[i + 1].c24 = 0x7FFFFFFF;

			/* "540001e0 b.eq"@0xffffff8008a6a50c */
			if (start == end)
				continue;

			/* "4b0402c8 sub"@0xffffff8008a6a510 +
			 * "0b050108 add"@0xffffff8008a6a518 = 1 - start + end,
			 * memorizzato DOPO il confronto
			 * ("b81f8308 stur"@0xffffff8008a6a52c) */
			ilitek_fw_blk[i + 1].c16 = end - start + 1;

			/* riga 1019;
			 * "\x016ILITEK: (%s, %d): Block[%d]: start_addr = %x, end = %x\n"@0xffffff8009246ccc */
			ILI_INFO("Block[%d]: start_addr = %x, end = %x\n",
#line 1019
				 i + 1, start, end);

			/* "f100e67f cmp"@0xffffff8008a6a534 confronta l'indice
			 * con 57 = 39 + 6*3, cioe' i == 3, e il blocco che ne
			 * risulta e' il quarto -- lo stesso che
			 * `ilitek_tddi_fw_hex_convert` marca con
			 * "710012ff cmp"@0xffffff8008a69434 */
			if (i == 3)
				/* "3909f916 strb"@0xffffff8008a6a544 */
				idev->c638 = 1;
		}

		/* "36380a1b tbz"@0xffffff8008a6a568 -- il bit 7 del byte 32,
		 * che il ciclo di sopra non ha usato */
		if (type & 0x80) {
			/*
			 * TRE COPIE, UNA SOLA RIGA DI SORGENTE. E' un CICLO
			 * srotolato, e la prova non e' la forma del codice ma
			 * il `__LINE__`: le tre `printk`
			 * "97db23c9 bl"@0xffffff8008a6a5b0,
			 * "97db23b7 bl"@0xffffff8008a6a5f8 e
			 * "97db23a5 bl"@0xffffff8008a6a640 portano TUTTE E TRE
			 * "52808122 mov"@0xffffff8008a6a5a8,
			 * "52808122 mov"@0xffffff8008a6a5f0 e
			 * "52808122 mov"@0xffffff8008a6a638, cioe' w2 = 0x409 =
			 * 1033. Un file sorgente non puo' avere tre istruzioni
			 * distinte sulla stessa riga se non sono la stessa
			 * istruzione ripetuta dal compilatore.
			 * Il passo e' 4 e la base 6: 6,7,8 con l'indice a 9;
			 * 10,11,12 con l'indice a 13; 14,15,16 con l'indice a
			 * 17 ("39401909 ldrb"@0xffffff8008a6a578,
			 * "39402909 ldrb"@0xffffff8008a6a5c0,
			 * "39403909 ldrb"@0xffffff8008a6a608).
			 */
			for (i = 0; i < 3; i++) {
				/* "34000223 cbz"@0xffffff8008a6a574 */
				blk = ilitek_ilifile[9 + i * 4];
				if (!blk)
					continue;

				addr = (ilitek_ilifile[6 + i * 4] << 16) |
				       (ilitek_ilifile[7 + i * 4] << 8) |
					ilitek_ilifile[8 + i * 4];

				/* "34000144 cbz"@0xffffff8008a6a590 */
				if (!addr)
					continue;

				/* "b9001904 str"@0xffffff8008a6a5ac */
				ilitek_fw_blk[blk].c24 = addr;

				/* riga 1033;
				 * "\x016ILITEK: (%s, %d): Tag 0xB0: change Block[%d] to addr = 0x%x\n"@0xffffff8009246adf */
				ILI_INFO("Tag 0xB0: change Block[%d] to addr = 0x%x\n",
					 blk, addr);
			}
		}
	} else {
		/* "528015c9 mov"@0xffffff8008a6a6a0 = 0xAE */
		ilitek_hex_tag = 0xAE;
	}

	/*
	 * "91010101 add"@0xffffff8008a6a6b4 -- la sorgente e' il file PIU' 64.
	 * LA LUNGHEZZA E' ESTESA CON SEGNO: "93407e62 sxtw"@0xffffff8008a6a6ac,
	 * non una `mov w2, w19`. E il confronto di sopra e' invece SENZA segno
	 * ("540007c9 b.ls"@0xffffff8008a6a27c). Le due cose insieme non si
	 * ottengono da una sola variabile: o il confronto ha un operando
	 * `unsigned` che non e' `size`, o la lunghezza passa per un `int`.
	 * PROVATO il cast `(s32)size`, che riproduce la `sxtw`: la funzione
	 * PERDE otto byte (0x2238 -> 0x2230), cioe' si allontana dalla
	 * fabbrica invece di avvicinarsi, perche' clang ne ricava un'altra
	 * semplificazione. Il cast NON e' quindi qui: la divergenza di UNA
	 * istruzione resta dichiarata e non aggiustata (divergenza G14).
	 * E' la regola 3 del progetto: non aggiustare qualcosa perche' un
	 * numero torni.
	 */
	memcpy(pfw, ilitek_ilifile + 64, size);

	if (ilitek_fw_calc_file_crc(pfw) < 0)
		return -1;

	/* "b906a2d3 str"@0xffffff8008a6a6d4 e
	 * "391a6128 strb"@0xffffff8008a6a6d8, con il byte 33 del file
	 * ("39408508 ldrb"@0xffffff8008a6a6d0). `ilitek_star_addr` NON e'
	 * riscritto qui: resta lo zero di sopra. */
	ilitek_end_addr = size;
	ilitek_blk_num = ilitek_ilifile[33];

	return 0;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_hex_convert -- INCORPORATA, righe 1080..1115
 * ===========================================================================
 * `__func__` = "ilitek_tddi_fw_hex_convert"@0xffffff8009246a8b.
 *
 * E' il lettore di Intel HEX, e le costanti del formato sono nel binario:
 *   "110006a8 add"@0xffffff8008a6893c    +1  la lunghezza (2 cifre)
 *   "11000ea8 add"@0xffffff8008a689d4    +3  l'indirizzo   (4 cifre)
 *   "11001ea8 add"@0xffffff8008a68af4    +7  il tipo       (2 cifre)
 *   "110026a8 add"@0xffffff8008a68b94    +9  il dato
 *   "11003ea8 add"@0xffffff8008a68c54   +15
 *   "110056a8 add"@0xffffff8008a68bf4   +21
 * e la lunghezza del record e' 11 + len*2 caratteri
 * ("531f7b0a lsl"@0xffffff8008a68e58 + "11002d08 add"@0xffffff8008a68e60),
 * piu' 1 o 2 secondo che il terminatore sia "\n" oppure "\r\n"
 * ("7100353f cmp"@0xffffff8008a68e6c con 0x0d, seguita da un `cinc` a
 * 0xffffff8008a68e70, codifica 1a8614c9).
 *
 * QUEL `cinc` NON E' SCRITTO NELLA FORMA CITABILE, e va detto perche' e'
 * l'unica citazione di tutto il file che non lo sia: `cinc` MANCA
 * dall'insieme `MNEMONICI` di `confinecitazioni.py` -- che elenca
 * "csel", "csinc", "csinv", "csneg", "cset", "csetm", "ccmp", "ccmn" e non
 * i tre alias `cinc`, `cinv`, `cneg` -- e quindi
 * `descrive_istruzione("1a8614c9 cinc")` restituisce False e
 * `verificacitazioni.py` la instrada fra i LETTERALI, dove giustamente non
 * la trova e risponde NON_ANCORATA. Non e' un difetto di questo file ed e'
 * un delta per lo strumento condiviso, non per il sorgente: vedi il
 * rapporto. Il fatto resta verificabile a mano:
 *   objdump -d --start-address 0xffffff8008a68e70 --stop-address 0xffffff8008a68e74 oracolo/stock.elf
 *   ffffff8008a68e70:	1a8614c9 	cinc	w9, w6, eq
 *
 * `start_addr` NON SI AGGIORNA MAI, ed e' un difetto della fabbrica che si
 * riproduce (regola 7). Il confronto e' SENZA segno
 * ("6b1c007f cmp"@0xffffff8008a68e80 + "1a9c307c csel"@0xffffff8008a68e88,
 * condizione `cc` = minore senza segno) e la variabile parte da zero
 * ("2a1f03fc mov"@0xffffff8008a68928): nessun indirizzo puo' essere minore
 * di zero senza segno, quindi la `csel` sceglie sempre il vecchio valore e
 * "star_addr" viene stampato 0. Con l'inizializzazione a 0xFFFFFFFF che il
 * codice avrebbe voluto, il ramo servirebbe -- ma il binario non ce l'ha.
 *
 * IL TIPO 0x02 E' DIVISO PER 4096, il tipo 0x04 no: "530c7d04 lsr"@0xffffff8008a68e50
 * e' uno scorrimento SENZA segno di 12, e l'indirizzo finale e'
 * "0b044363 add"@0xffffff8008a68e68 = addr + (ex_addr << 16). Per il tipo
 * 0x04 (Extended Linear Address) il valore e' gia' in unita' da 64K; per il
 * 0x02 (Extended Segment Address) e' in unita' da 16, e 16 >> 12 == 1/4096
 * lo riporta in unita' da 64K.
 *
 * IL VALORE DI RITORNO DI `hex_to_dec` FINISCE IN UN `strb`
 * ("38234a6c strb"@0xffffff8008a68f1c): il -1 dell'errore diventa 0xFF nel
 * buffer, e questo spiega perche' la copia srotolata di quel ramo
 * materializza 0xff ("32001fec orr"@0xffffff8008a68f18) invece di -1.
 */
static int ilitek_tddi_fw_hex_convert(u8 *phex, u32 size, u8 *pfw)
{
	u32 i = 0, j = 0, k = 0, num = 0;
	u32 len = 0, addr = 0, type = 0;
	u32 start_addr = 0, end_addr = 0, ex_addr = 0;
	u32 nowAddr, offset, block_count = 0;

	memset(ilitek_fw_blk, 0x0, sizeof(ilitek_fw_blk));

	/* "6b1702bf cmp"@0xffffff8008a68f34 + "54ffd023 b.cc"@0xffffff8008a68f38
	 * -- guardia SENZA segno, e il ciclo e' saltato per intero quando
	 * `size` e' zero ("34006bd7 cbz"@0xffffff8008a6890c).
	 *
	 * NON HA INCREMENTO PROPRIO, ed e' misurato: "0b080135 add"@0xffffff8008a68f30
	 * e' l'UNICA scrittura di w21 nel corpo, e somma al `w8` che vale gia'
	 * `i + 11 + len*2` il solo `w9` che vale 1 o 2. Un `for (; i < size; i++)`
	 * con `i = offset` in coda darebbe `+2` o `+3`, cioe' un carattere di
	 * troppo per ogni record. */
	while (i < size) {
		len = hex_to_dec(&phex[i + 1], 2);
		addr = hex_to_dec(&phex[i + 3], 4);
		type = hex_to_dec(&phex[i + 7], 2);

		/* "710012df cmp"@0xffffff8008a68b8c */
		if (type == 0x04) {
			ex_addr = hex_to_dec(&phex[i + 9], 4);
		/* "71000adf cmp"@0xffffff8008a68b84 */
		} else if (type == 0x02) {
			/* "530c7d04 lsr"@0xffffff8008a68e50 -- SENZA segno.
			 * Scritto come un'unica espressione a partire dal
			 * ritorno `int` di `hex_to_dec` verrebbe `asr`: e' il
			 * passaggio per la variabile `u32` a decidere il
			 * mnemonico, e il binario dice quale dei due e' */
			ex_addr = hex_to_dec(&phex[i + 9], 4);
			ex_addr = ex_addr >> 12;
		/* "320002c8 orr"@0xffffff8008a68bd4 + "7102bd1f cmp"@0xffffff8008a68bd8
		 * -- (type | 1) == 0xAF e' come clang scrive
		 * `type == 0xAE || type == 0xAF` */
		} else if (type == 0xAE || type == 0xAF) {
			/* "b906a916 str"@0xffffff8008a68bec */
			ilitek_hex_tag = type;

			/* "2a0203f7 mov"@0xffffff8008a68be8 -- l'assegnamento
			 * e' INCONDIZIONATO e precede il confronto
			 * "7102bedf cmp"@0xffffff8008a68be0 */
			num = block_count;
			if (type == 0xAF)
				num = hex_to_dec(&phex[i + 21], 2);

			/* "71001eff cmp"@0xffffff8008a69000 +
			 * "5400b962 b.cs"@0xffffff8008a69004 -- e' scritto
			 * `>= 7`, non `> 6`: il gemello della riga 1099 porta
			 * l'altra forma, e le due codifiche sono diverse */
			if (num >= FW_BLOCK_INFO_NUM) {
				/* riga 1080;
				 * "\x013ILITEK: (%s, %d): ERROR! block num is larger than its define (%d, %d)\n"@0xffffff8009246a42 */
				ILI_ERR("ERROR! block num is larger than its define (%d, %d)\n",
					num, FW_BLOCK_INFO_NUM - 1);
				return -1;
			}

			/* "b9000944 str"@0xffffff8008a69230 (c8),
			 * "2901a505 stp"@0xffffff8008a69428 (c12 e c16 in una
			 * sola coppia), "b900190a str"@0xffffff8008a69424 (c24) */
			ilitek_fw_blk[num].c8 = hex_to_dec(&phex[i + 9], 6);
			ilitek_fw_blk[num].c12 = hex_to_dec(&phex[i + 15], 6);
			ilitek_fw_blk[num].c16 = ilitek_fw_blk[num].c12 -
						 ilitek_fw_blk[num].c8 + 1;
			ilitek_fw_blk[num].c24 = 0x7FFFFFFF;

			/* riga 1088. IL FORMATO NON FINISCE CON `\n`, e non e'
			 * una svista di trascrizione: il letterale a
			 * 0xffffff8009246aa6 e' lungo 56 byte e l'ultimo e' 'x';
			 * "\x016ILITEK: (%s, %d): Block[%d]: start_addr = %x, end = %x"@0xffffff8009246aa6 */
			ILI_INFO("Block[%d]: start_addr = %x, end = %x",
				 num, ilitek_fw_blk[num].c8,
#line 1088
				 ilitek_fw_blk[num].c12);

			/* "710012ff cmp"@0xffffff8008a69434 +
			 * "3909f906 strb"@0xffffff8008a69444 */
			if (num == 4)
				idev->c638 = 1;

			/* "11000782 add"@0xffffff8008a6944c */
			block_count++;
		/* "7102c2df cmp"@0xffffff8008a68c3c + "b946a908 ldr"@0xffffff8008a68c48
		 * + "7102bd1f cmp"@0xffffff8008a68c4c */
		} else if (type == 0xB0 && ilitek_hex_tag == 0xAF) {
			num = hex_to_dec(&phex[i + 15], 2);

			/* "7100187f cmp"@0xffffff8008a69190 +
			 * "5400abc8 b.hi"@0xffffff8008a69194 -- `> 6`, la forma
			 * gemella di quella della riga 1080 */
			if (num > FW_BLOCK_INFO_NUM - 1) {
				/* riga 1099 */
				ILI_ERR("ERROR! block num is larger than its define (%d, %d)\n",
					num, FW_BLOCK_INFO_NUM - 1);
				return -1;
			}

			/* "b9001904 str"@0xffffff8008a69604 */
			ilitek_fw_blk[num].c24 = hex_to_dec(&phex[i + 9], 6);

			/* riga 1104;
			 * "\x016ILITEK: (%s, %d): Tag 0xB0: change Block[%d] to addr = 0x%x\n"@0xffffff8009246adf */
			ILI_INFO("Tag 0xB0: change Block[%d] to addr = 0x%x\n",
				 num, ilitek_fw_blk[num].c24);
		}

		/* "0b044363 add"@0xffffff8008a68e68 */
		nowAddr = addr + (ex_addr << 16);

		/* "38684b49 ldrb"@0xffffff8008a68e64 -- il carattere subito
		 * dopo il record, letto PRIMA del controllo di limite */
		if (phex[i + 1 + 2 + 4 + 2 + len * 2 + 2] == 0x0D)
			offset = i + 1 + 2 + 4 + 2 + len * 2 + 2 + 2;
		else
			offset = i + 1 + 2 + 4 + 2 + len * 2 + 2 + 1;

		/* "6b10007f cmp"@0xffffff8008a68e74 +
		 * "540043e8 b.hi"@0xffffff8008a68e78 -- SENZA segno */
		if (nowAddr > MAX_HEX_FILE_SIZE) {
			/* riga 1115;
			 * "\x013ILITEK: (%s, %d): Invalid hex format %d\n"@0xffffff8009246b1e */
#line 1115
			ILI_ERR("Invalid hex format %d\n", nowAddr);
			return -1;
		}

		/* "350005b6 cbnz"@0xffffff8008a68e7c */
		if (type == 0x00) {
			/* vedi il cappello: questa `if` non scatta mai, e si
			 * riproduce */
			if (nowAddr < start_addr)
				start_addr = nowAddr;

			/* "0b180074 add"@0xffffff8008a68e84 -- assegnamento
			 * secco, non un massimo */
			end_addr = nowAddr + len;

			/* "34000538 cbz"@0xffffff8008a68e8c -- il ciclo interno
			 * e' saltato per intero quando `len` e' zero, e il
			 * conteggio dei giri e' ((len*2 - 1) >> 1) + 1
			 * ("5100054a sub"@0xffffff8008a68e90 +
			 * "53017d4a lsr"@0xffffff8008a68e94 +
			 * "1100054a add"@0xffffff8008a68e98), che e' come clang
			 * conta un `for` a passo 2 */
			for (j = 0, k = 0; j < len * 2; j += 2, k++)
				pfw[nowAddr + k] = hex_to_dec(&phex[i + 9 + j], 2);
		}

		i = offset;
	}

	if (ilitek_fw_calc_file_crc(pfw) < 0)
		return -1;

	/* "b9069d1c str"@0xffffff8008a696b8, "b906a136 str"@0xffffff8008a696bc,
	 * "391a6154 strb"@0xffffff8008a696c0 */
	ilitek_star_addr = start_addr;
	ilitek_end_addr = end_addr;
	ilitek_blk_num = block_count;

	return 0;
}

/*
 * ===========================================================================
 * ilitek_tdd_fw_hex_open -- INCORPORATA, righe 1149..1243
 * ===========================================================================
 * IL NOME PORTA IL REFUSO DELLA FABBRICA: "tdd", non "tddi". Vedi il
 * cappello del lotto.
 *
 * IL CONFINE DI QUESTA FUNZIONE E' PIU' LARGO DI QUEL CHE SEMBRA: le righe
 * 1236 ("fw data/size is invaild") e 1243 ("Convert hex file failed")
 * portano il `__func__` "ilitek_tdd_fw_hex_open"@0xffffff8009246813
 * ("91204c21 add"@0xffffff8008a69634 e "91204c21 add"@0xffffff8008a6971c),
 * non quello di `ilitek_tddi_fw_upgrade`. La chiamata a
 * `ilitek_tddi_fw_hex_convert` sta quindi QUI, non nel chiamante.
 *
 * IL RAMO `REQUEST_FIRMWARE` NON LIBERA IL BUFFER PRECEDENTE e quello
 * `FILP_OPEN` si': "f900911f str"@0xffffff8008a68680 azzera `c288` senza
 * nessuna `vfree` prima, mentre "97df66a6 bl"@0xffffff8008a687a0 la fa. E'
 * un'asimmetria della fabbrica e si riproduce (regola 7).
 *
 * LE DUE `set_fs` CONSECUTIVE sono nel binario, non un errore di
 * trascrizione: tre `bl set_bit`
 * ("940fb9b1 bl"@0xffffff8008a68828, "940fb9aa bl"@0xffffff8008a68844,
 * "940fb99b bl"@0xffffff8008a68880) e tre scritture di `addr_limit`
 * ("f90006f6 str"@0xffffff8008a68814, "f90006f6 str"@0xffffff8008a68830,
 * "f90006f8 str"@0xffffff8008a6886c). Su arm64 `set_fs` si espande in
 * `addr_limit = x; dsb; isb; set_thread_flag(TIF_FSCHECK)` e TIF_FSCHECK
 * vale 5 ("528000a0 mov"@0xffffff8008a68820). Due prima della lettura con
 * lo stesso valore -1 ("92800016 mov"@0xffffff8008a68808 = KERNEL_DS) e una
 * dopo con il valore salvato.
 */
static int ilitek_tdd_fw_hex_open(u8 open_file_method, u8 *pfw)
{
	int ret = 0, fsize = 1;
	const struct firmware *fw = NULL;
	struct file *f = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	/* riga 1149. I due `%s` sono scelti con due `csel`
	 * ("9a891149 csel"@0xffffff8008a685b8 per il percorso e
	 * "9a881123 csel"@0xffffff8008a685dc per il nome del metodo), quindi
	 * sono due operatori ternari e non due rami separati;
	 * "\x016ILITEK: (%s, %d): Open file method = %s, path = %s\n"@0xffffff80092467dd,
	 * "FILP_OPEN"@0xffffff800924682a, "REQUEST_FIRMWARE"@0xffffff8009246834 */
	ILI_INFO("Open file method = %s, path = %s\n",
		 open_file_method ? "FILP_OPEN" : "REQUEST_FIRMWARE",
		 open_file_method ? (const char *)idev->c664 :
#line 1149
				    (const char *)idev->c672);

	switch (open_file_method) {
	case REQUEST_FIRMWARE:
		/* "f9415101 ldr"@0xffffff8008a68608 (+672, il nome) e
		 * "f9400d02 ldr"@0xffffff8008a6860c (+24, il dispositivo) */
		if (request_firmware(&fw, (const char *)idev->c672,
				     (struct device *)idev->c24) < 0) {
			/* riga 1154;
			 * "\x013ILITEK: (%s, %d): Request firmware failed, try again\n"@0xffffff8009246845 */
#line 1154
			ILI_ERR("Request firmware failed, try again\n");

			if (request_firmware(&fw, (const char *)idev->c672,
					     (struct device *)idev->c24) < 0) {
				/* riga 1156;
				 * "\x013ILITEK: (%s, %d): Request firmware failed after retry\n"@0xffffff800924687d */
#line 1156
				ILI_ERR("Request firmware failed after retry\n");
				ret = -1;
				goto out;
			}
		}

		/* "f9400114 ldr"@0xffffff8008a68658 */
		fsize = fw->size;
		/* riga 1163;
		 * "\x016ILITEK: (%s, %d): fsize = %d\n"@0xffffff80092468b6 */
#line 1163
		ILI_INFO("fsize = %d\n", fsize);

		/* "7100029f cmp"@0xffffff8008a6866c + "54007f8d b.le"@0xffffff8008a68670
		 * -- CON segno */
		if (fsize <= 0) {
			/* riga 1165;
			 * "\x013ILITEK: (%s, %d): The size of file is zero\n"@0xffffff80092468d6 */
#line 1165
			ILI_ERR("The size of file is zero\n");
			release_firmware(fw);
			ret = -1;
			goto out;
		}

		idev->c288 = 0;
		/* "93407e94 sxtw"@0xffffff8008a68678 -- `fsize` e' esteso CON
		 * segno prima di diventare la taglia della `vmalloc` */
		idev->c296 = (u64)vmalloc(fsize);
		if (ERR_ALLOC_MEM((void *)idev->c296)) {
			/* riga 1174;
			 * "\x013ILITEK: (%s, %d): Failed to allocate tp_fw by vmalloc, try again\n"@0xffffff8009246904 */
#line 1174
			ILI_ERR("Failed to allocate tp_fw by vmalloc, try again\n");
			idev->c296 = (u64)vmalloc(fsize);
			if (ERR_ALLOC_MEM((void *)idev->c296)) {
				/* riga 1177;
				 * "\x013ILITEK: (%s, %d): Failed to allocate tp_fw after retry\n"@0xffffff8009246948 */
#line 1177
				ILI_ERR("Failed to allocate tp_fw after retry\n");
				release_firmware(fw);
				ret = -ENOMEM;
				goto out;
			}
		}

		/* "f9400501 ldr"@0xffffff8008a686d8 -- il campo +8 di
		 * `struct firmware`, cioe' `data` */
		memcpy((u8 *)idev->c296, fw->data, fsize);
		/* "f9009114 str"@0xffffff8008a686e4 */
		idev->c288 = fsize;
		release_firmware(fw);
		break;
	case FILP_OPEN:
		/* "52803482 mov"@0xffffff8008a68744 = 0x1a4 = 420 decimale,
		 * che e' 644 in OTTALE scritto senza lo zero iniziale: il
		 * sorgente di fabbrica passa 644, e sono i permessi sbagliati
		 * per una `open` in lettura. Si riproduce. */
		f = filp_open((const char *)idev->c664, O_RDONLY, 644);
		if (ERR_ALLOC_MEM(f)) {
			/* riga 1192. L'argomento e' il puntatore stesso, non
			 * `PTR_ERR`: "aa1403e3 mov"@0xffffff8008a69654 passa x20
			 * tale e quale;
			 * "\x013ILITEK: (%s, %d): Failed to open the file at %ld\n"@0xffffff8009246982 */
#line 1192
			ILI_ERR("Failed to open the file at %ld\n", (long)f);
			ret = -1;
			goto out;
		}

		/* "f9401288 ldr"@0xffffff8008a68764 (+32 di `struct file` =
		 * `f_inode`) + "f9402915 ldr"@0xffffff8008a68774 (+80 di
		 * `struct inode` = `i_size`) */
		fsize = f->f_inode->i_size;
		/* riga 1198 */
#line 1198
		ILI_INFO("fsize = %d\n", fsize);

		if (fsize <= 0) {
			/* riga 1200;
			 * "\x013ILITEK: (%s, %d): The size of file is invaild\n"@0xffffff8009244aff */
#line 1200
			ILI_ERR("The size of file is invaild\n");
			filp_close(f, NULL);
			ret = -1;
			goto out;
		}

		ipio_vfree((void **)&idev->c296);
		idev->c288 = 0;
		idev->c296 = (u64)vmalloc(fsize);
		if (ERR_ALLOC_MEM((void *)idev->c296)) {
			/* riga 1210 */
#line 1210
			ILI_ERR("Failed to allocate tp_fw by vmalloc, try again\n");
			idev->c296 = (u64)vmalloc(fsize);
			if (ERR_ALLOC_MEM((void *)idev->c296)) {
				/* riga 1213 */
#line 1213
				ILI_ERR("Failed to allocate tp_fw after retry\n");
				filp_close(f, NULL);
				ret = -ENOMEM;
				goto out;
			}
		}

		old_fs = get_fs();
		set_fs(KERNEL_DS);
		set_fs(get_ds());
		/* "f90017ff str"@0xffffff8008a68854 */
		pos = 0;
		vfs_read(f, (char *)idev->c296, fsize, &pos);
		set_fs(old_fs);
		filp_close(f, NULL);
		/* "f9009115 str"@0xffffff8008a6a664 -- DOPO la `filp_close` */
		idev->c288 = fsize;
		break;
	default:
		/* riga 1231. L'argomento e' mascherato a un byte
		 * ("12001e83 and"@0xffffff8008a6889c), che e' la larghezza del
		 * parametro;
		 * "\x013ILITEK: (%s, %d): Unknown open file method, %d\n"@0xffffff80092469b6 */
#line 1231
		ILI_ERR("Unknown open file method, %d\n", open_file_method);
		break;
	}

	/* riga 1236. La condizione e' `ERR_ALLOC_MEM(c296) || !c288`, e il
	 * secondo test e' a 64 bit ("b4006b17 cbz"@0xffffff8008a688c8);
	 * "\x013ILITEK: (%s, %d): fw data/size is invaild\n"@0xffffff80092469e8 */
	if (ERR_ALLOC_MEM((void *)idev->c296) || !idev->c288) {
#line 1236
		ILI_ERR("fw data/size is invaild\n");
		ret = -1;
		goto out;
	}

	/* riga 1243;
	 * "\x013ILITEK: (%s, %d): Convert hex file failed\n"@0xffffff8009246a15 */
	if (ilitek_tddi_fw_hex_convert((u8 *)idev->c296, idev->c288, pfw) < 0) {
#line 1243
		ILI_ERR("Convert hex file failed\n");
		ret = -1;
	}

out:
	/* "b4000060 cbz"@0xffffff8008a6974c + "97df62ba bl"@0xffffff8008a69750
	 * + "f900969f str"@0xffffff8008a69754 -- la coda comune di TUTTI i
	 * percorsi, compresi quelli d'errore */
	ipio_vfree((void **)&idev->c296);
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_fw_upgrade -- 0xffffff8008a684e0, 8824 byte, righe 1261..1311
 * ===========================================================================
 * `T` nella mappa. L'unico sito di chiamata e'
 * `ilitek_tddi_fw_upgrade_handler`, nel gruppo A:
 *   "b941c100 ldr"@0xffffff8008a53a94    w0 = idev->c448
 *   "94005291 bl"@0xffffff8008a53a9c     -> 0xffffff8008a684e0
 *   "2a0003f3 mov"@0xffffff8008a53aa0    w19 = il ritorno
 * L'argomento e' quindi `idev->c448`, che `ilitek.h` documenta gia' come
 * «l'unico argomento di ilitek_tddi_fw_upgrade», ed e' un `int`; e il
 * ritorno E' USATO, quindi la funzione non e' `void`.
 *
 * LA CONDIZIONE D'INGRESSO HA TRE TERMINI IN OR, e l'ordine di valutazione
 * e' misurato dai due `cbz` in cascata:
 *   "34000069 cbz"@0xffffff8008a6851c   se `c224` e' zero -> si entra
 *   "34000e88 cbz"@0xffffff8008a68524   se `c631` e' zero -> si valuta pfw
 * e il ramo che valuta `pfw` ("b50092e3 cbnz"@0xffffff8008a68704) salta
 * OLTRE tutto il caricamento del file quando il buffer e' gia' buono. E'
 * esattamente `if (!c224 || c631 || ERR_ALLOC_MEM(pfw))`.
 *
 * IL RIEMPIMENTO A 0xFF E' UN CICLO, non una `memset`: nessuna `bl` verso
 * `__memset` e il globale e' RILETTO a ogni giro
 * ("f942d72a ldr"@0xffffff8008a68590 dentro il corpo), che e' quello che il
 * compilatore deve fare quando si scrive attraverso un `u8 *` che puo'
 * aliasare il puntatore stesso.
 *
 * IL CICLO DI RITENTATIVI CONTA ALL'INDIETRO DA TRE
 * ("320007f3 orr"@0xffffff8008a6996c = 3,
 * "71000673 subs"@0xffffff8008a6a180 + "54ffc1a1 b.ne"@0xffffff8008a6a184),
 * e il messaggio della riga 1303 stampa il contatore DOPO che e' arrivato a
 * zero ("2a1303e3 mov"@0xffffff8008a6a19c): stampa sempre 0. E' un difetto
 * della fabbrica e si riproduce.
 *
 * LE DUE USCITE. Il percorso di successo
 * ("f94522a8 ldr"@0xffffff8008a6a338) NON azzera `idev->c636`; tutti i
 * percorsi d'errore ci passano ("3909f11f strb"@0xffffff8008a6a2b8). Vedi la
 * divergenza G10 per la forma sorgente, che il binario non fissa.
 */
int ilitek_tddi_fw_upgrade(int open_file_method)
{
	int i, ret = 0, retry = 3;

	/* "39438109 ldrb"@0xffffff8008a68518 (+224),
	 * "3949dd08 ldrb"@0xffffff8008a68520 (+631) */
	if (!idev->c224 || idev->c631 || ERR_ALLOC_MEM(pfw)) {
		if (ERR_ALLOC_MEM(pfw)) {
			ipio_vfree((void **)&pfw);
			pfw = vmalloc(MAX_HEX_FILE_SIZE);
			if (ERR_ALLOC_MEM(pfw)) {
				/* riga 1261. L'argomento e' il puntatore
				 * stesso ("aa0303e0 mov" non c'e': x0 non e'
				 * preparato, la `printk` non ha il terzo
				 * argomento perche' il formato lo prende da
				 * w3 e w3 non e' scritto). Vedi G13;
				 * "\x013ILITEK: (%s, %d): Failed to allocate pfw memory, %ld\n"@0xffffff8009246333 */
#line 1261
				ILI_ERR("Failed to allocate pfw memory, %ld\n");
				ipio_vfree((void **)&pfw);
				ret = -ENOMEM;
				goto out;
			}
		}

		/* "32001fe8 orr"@0xffffff8008a68580 = 0xff,
		 * "eb13013f cmp"@0xffffff8008a6859c contro 0x28000 */
		for (i = 0; i < MAX_HEX_FILE_SIZE; i++)
			pfw[i] = 0xff;

		/* "37f809d3 tbnz"@0xffffff8008a69758 */
		if (ilitek_tdd_fw_hex_open(open_file_method, pfw) < 0) {
			/* riga 1272;
			 * "\x013ILITEK: (%s, %d): Open hex file fail, try upgrade from ILI file\n"@0xffffff8009246382 */
#line 1272
			ILI_ERR("Open hex file fail, try upgrade from ILI file\n");

			/* "394a0d28 ldrb"@0xffffff8008a698ac (+643) */
			if (!idev->c643) {
				/* riga 1280;
				 * "\x013ILITEK: (%s, %d): Ignore update from ILI file\n"@0xffffff80092463c5 */
#line 1280
				ILI_ERR("Ignore update from ILI file\n");
				ipio_vfree((void **)&pfw);
				/* "12800e34 mov"@0xffffff8008a698dc -- e il
				 * salto va a 0xffffff8008a6a2f8, cioe' SALTA
				 * anche l'azzeramento di `c636` e le cinque
				 * riletture: e' un `return`, non un `goto` */
				return -EFW_CONVERT_FILE;
			}

			if (ilitek_tddi_fw_ili_convert(pfw) < 0) {
				/* riga 1286;
				 * "\x013ILITEK: (%s, %d): Convert ILI file error\n"@0xffffff80092463f6 */
#line 1286
				ILI_ERR("Convert ILI file error\n");
				ret = -EFW_CONVERT_FILE;
				goto out;
			}
		}

		ilitek_tddi_fw_update_block_info(pfw);
	}

	do {
		ret = ilitek_tddi_fw_flash_upgrade(pfw);
		if (ret == 0)
			goto done;

		/* riga 1299;
		 * "\x013ILITEK: (%s, %d): Upgrade failed, do retry!\n"@0xffffff8009246422 */
#line 1299
		ILI_ERR("Upgrade failed, do retry!\n");
	} while (--retry);

	/* riga 1303;
	 * "\x013ILITEK: (%s, %d): Failed to upgrade fw %d times, erasing flash\n"@0xffffff8009246451 */
#line 1303
	ILI_ERR("Failed to upgrade fw %d times, erasing flash\n", retry);

	if (ilitek_ice_mode_ctrl(true, false) < 0)
		/* riga 1305;
		 * "\x013ILITEK: (%s, %d): Enable ice mode failed while erasing flash\n"@0xffffff8009246493 */
#line 1305
		ILI_ERR("Enable ice mode failed while erasing flash\n");

	if (ilitek_tddi_fw_flash_erase() < 0)
		/* riga 1307;
		 * "\x013ILITEK: (%s, %d): Failed to erase flash\n"@0xffffff80092464d3 */
#line 1307
		ILI_ERR("Failed to erase flash\n");

	if (ilitek_ice_mode_ctrl(false, false) < 0)
		/* riga 1309;
		 * "\x013ILITEK: (%s, %d): Disable ice mode failed after erase flash\n"@0xffffff80092464fe */
#line 1309
		ILI_ERR("Disable ice mode failed after erase flash\n");

	if (ilitek_tddi_reset_ctrl(idev->c612) < 0)
		/* riga 1311;
		 * "\x013ILITEK: (%s, %d): TP reset failed after erase flash\n"@0xffffff800924653d */
#line 1311
		ILI_ERR("TP reset failed after erase flash\n");

out:
	/* "3909f11f strb"@0xffffff8008a6a2b8 */
	idev->c636 = false;

done:
	/* "b942c908 ldr"@0xffffff8008a6a2bc (+712) */
	if (idev->c712)
		ilitek_ice_mode_ctrl(false, false);

	ilitek_tddi_ic_get_core_ver();
	ilitek_tddi_ic_get_protocl_ver();
	ilitek_tddi_ic_get_fw_ver();
	ilitek_tddi_ic_get_tp_info();
	ilitek_tddi_ic_get_panel_info();

	/* "3949f109 ldrb"@0xffffff8008a6a2e8 + "35000069 cbnz"@0xffffff8008a6a2ec */
	if (!idev->c636)
		idev->c636 = true;

	return ret;
}
