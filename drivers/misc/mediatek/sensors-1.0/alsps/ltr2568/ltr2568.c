// SPDX-License-Identifier: GPL-2.0
/*
 * ODM_LTR2568 -- luce ambientale e prossimita' (Lite-On LTR2568), Doogee S88 Pro.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA, non portato da un altro
 * telefono. Ogni costante porta accanto la riga di disassemblato da cui viene,
 * nella forma "<8 cifre esadecimali> <mnemonico>", e ogni letterale l'indirizzo
 * a cui sta in `oracolo/stock.elf` nella forma "testo"@0xINDIRIZZO.
 *
 * Il driver sibling `ltr303.c` (in ALPS,
 * drivers/misc/mediatek/sensors-1.0/alsps/ltr303/) e' servito SOLO a orientarsi
 * sulla forma del framework `alsps` di MediaTek -- nomi e firme delle ops,
 * ordine delle registrazioni. Ogni riga qui sotto e' comunque verificata contro
 * il disassemblato di fabbrica: un sibling e' un'analogia dichiarata, non una
 * prova.
 *
 * ---------------------------------------------------------------------------
 * IL CONFINE: 62 funzioni
 * ---------------------------------------------------------------------------
 * Corsa contigua `ltr2568_setup_eint` (0xffffff8008783730) ..
 * `ltr2568_i2c_resume` (0xffffff8008786b9c, 124 byte, fine 0xffffff8008786c18)
 * = 61 funzioni, piu' `ltr2568_init` (0xffffff8009373bd8) nella sezione __init
 * = 62. Riverificato in ENTRAMBE le direzioni contro `out/System.map` del
 * nostro build ALPS: 62 dentro il confine, 0 gia' presenti; 24 vicini fuori
 * (12 prima, 12 dopo), 24 presenti. Zero eccezioni da entrambe le parti.
 * PRECISAZIONE (revisione-ltr2568-completo.md §7.4): quella verifica e' reale
 * ma prova meno di quanto suoni. E' misurata contro `out/`, un build che NON
 * contiene questo sorgente (esattamente cio' che HANDOFF §4 invoca per
 * squalificare altri numeri) -- rifatta contro `out-ltr2568` da' comunque 62
 * su 62, senza discriminare piu' niente. E la direzione "0 gia' presenti" e'
 * garantita dalla `.config` (`# CONFIG_MTK_CM36558 is not set`,
 * `# CONFIG_MTK_LTR303 is not set`: nessun driver alsps di chip concorrente
 * e' acceso, quindi nomi generici come `als_get_data` non potevano esserci
 * comunque), non dal confine in se'. Non e' una falsificazione del confine --
 * regge -- ma il potere probante di quella riga e' minore di come e' scritta.
 *
 * ---------------------------------------------------------------------------
 * QUANTE UNITA' DI TRADUZIONE: UNA
 * ---------------------------------------------------------------------------
 *  1. `.ddebug`  -- TACE: zero descrittori con file o modulo 'ltr'.
 *  2. numeri di riga delle printk -- 55 siti, 36 `__func__` distinti,
 *     intervalli di riga TUTTI DISGIUNTI fra 303 e 2742. Un test che poteva
 *     fallire in 35 punti e non e' fallito.
 *     (I due numeri prima qui scritti -- 40 e 39 -- erano SBAGLIATI: la
 *     revisione indipendente ha rifatto il conto risolvendo per ogni
 *     `bl printk` la terna x0/x1/w2, e ne ha trovati 36 e 35, con una
 *     contro-prova indipendente contando a mano i nomi di funzione nella
 *     corsa di .rodata del driver. La conclusione non cambia: 35 punti in
 *     cui il test poteva fallire sono una prova forte quanto 39. Ma un
 *     numero pubblicato qui e' un impegno, e questi due erano da correggere.)
 *  3. `__func__` -- ha dato i nomi di sei funzioni incorporate e senza simbolo:
 *     ltr2568_i2c_write_block, ltr2568_ps_get_thres, ltr2568_ps_enable,
 *     ltr2568_get_als_value, ltr2568_init_client, ltr2568_create_attr.
 *  4. `__FILE__` dentro un ASSERT -- ASSENTE per questo driver. Il meccanismo
 *     qui funziona (i due file confinanti lasciano 'ps/aal_control.c' a
 *     0xffffff80091bde00 e '/home/jenkins/jks-WTK6739_Q0_MP1_AP/alps/
 *     kernel-4.14/drivers/misc/mediatek/sensors-1.0/accelerometer/accel.c' a
 *     0xffffff80091be8ab): ltr2568 semplicemente non usa asserzioni.
 *  5. `module_version_attribute` -- ASSENTE: in `stock.map` non c'e' nessun
 *     `__modver` di questo driver.
 *  6. prefisso comune dei letterali -- TUTTI i messaggi del driver portano
 *     '\x013[ALS/PS] %s %d : ' e occupano UNA corsa contigua di .rodata da
 *     0xffffff80091bdea3 a 0xffffff80091be827, delimitata prima dai letterali
 *     di aal_control.c e dopo da quelli di accel.c. Nessuna seconda famiglia di
 *     prefisso. Le DUE stringhe 'ltr2568' (0xffffff80091bdfb7 e
 *     0xffffff8008f53aa0) NON sono due unita' di traduzione: la prima e' il
 *     letterale mergeable usato da `.driver.name`, la seconda e' il campo
 *     `char name[I2C_NAME_SIZE]` dentro `struct i2c_device_id`.
 *
 * Il PERCORSO di fabbrica non e' misurato (il `.ddebug` tace). Per adiacenza di
 * indirizzo il blocco sta fra il framework `alsps` e il framework
 * `accelerometer`, quindi `drivers/misc/mediatek/sensors-1.0/alsps/ltr2568/`
 * e' l'ipotesi naturale -- ma resta un'ipotesi, non una misura.
 *
 * ---------------------------------------------------------------------------
 * LA MISURA DI DIMENSIONE: COM'E' CONFIGURATA, E QUANTO VALE
 * ---------------------------------------------------------------------------
 * Strumento: `tools/misuraisolata.py`, che compila una copia di questo file
 * fuori dall'albero con la riga di comando vera estratta da
 * `out-ltr2568/drivers/misc/mediatek/sensors-1.0/alsps/ltr2568/.ltr2568.o.cmd`
 * (non `.alsps.o.cmd` di un'altra unita' di traduzione, come scritto qui fino
 * al lotto precedente -- HANDOFF §4 avverte che sbagliare `KBUILD_MODNAME` e'
 * una divergenza reale; la revisione indipendente ha rieseguito con la riga
 * giusta e i numeri non sono cambiati di un byte, ma la citazione andava
 * corretta lo stesso: docs/bringup/rapporti/revisione-ltr2568-completo.md
 * §4.2), e confronta con `oracolo/stock.map`. La RADICE conta quanto il
 * sorgente, e va dichiarata insieme al numero:
 *
 *   misuraisolata.py ltr2568.c .ltr2568.o.cmd stock.map \
 *     --lavoro <dir> --build-out <out-ltr2568> \
 *     --intervallo 0xffffff8008783730:0xffffff8008786c18 \
 *     --intervallo 0xffffff8009373bd8:0xffffff8009373bf8 \
 *     --radice fabbrica --fuori-radice ltr2568_attr_list --elf stock.elf
 *
 * `--radice fabbrica` e `--fuori-radice` esistono da oggi, e sono la
 * correzione di due ARTEFATTI DELLO STRUMENTO che avevano falsato i numeri
 * pubblicati la prima volta (revisione-ltr2568.md, difetti S1 e S2):
 *
 *   - la radice ancorava anche le sei funzioni che di FABBRICA sono
 *     incorporate (ltr2568_create_attr, ltr2568_delete_attr,
 *     ltr2568_get_als_value, ltr2568_init_client, read_int_from_buf,
 *     ltr2568_exit), tenendole fuori linea e rendendo i loro chiamanti piu'
 *     piccoli del vero;
 *   - `&ltr2568_attr_list` in `__misura_radice_dati` faceva uscire
 *     l'indirizzo della tabella e impediva a clang di piegarne le letture,
 *     cosa che il build di fabbrica fa (di fabbrica le dieci
 *     driver_remove_file ricevono l'attributo come "d0008d41 adrp"+
 *     "911ce021 add", mentre la misura lo prendeva da una lettura
 *     dell'array -- ldr x1,[x21,#8], codifica f94006a1, che e' del NOSTRO
 *     oggetto e non di stock.elf, quindi non e' citata come tale).
 *
 * Con la SOLA radice corretta e questo sorgente immutato, quattro delle dodici
 * divergenze pubblicate sparivano da sole: als_get_data da -188 a 320/320,
 * ltr2568_als_factory_get_data da +44 a 68/68, ltr2568_i2c_remove da -156 a
 * 228/228, ltr2568_i2c_probe da -580 a -64. Una misura con un artefatto SI
 * RIFA', non si accorcia: togliere le funzioni artefatte dal denominatore
 * ("3 su 9") e' aritmetica sbagliata, ed e' il rimedio che era stato proposto.
 *
 * DA OGGI QUESTA NON E' PIU' LA MISURA BUONA, ed e' la cosa piu' importante
 * di questo lotto. Con le quattro funzioni scritte il driver ha una radice
 * vera (`ltr2568_init` -> `alsps_driver_add` -> `i2c_add_driver` ->
 * `ltr2568_i2c_probe`), si linka, e sta nel kernel: si puo' finalmente
 * misurarlo nel BUILD VERO, che e' quello che HANDOFF §0 chiede
 * ("misura sempre contro un build che contiene il tuo sorgente"). La misura
 * isolata resta qui perche' e' quella con cui i numeri precedenti sono stati
 * pubblicati, e perche' il confronto fra le due dice una cosa che nessuna
 * delle due da sola direbbe.
 *
 * `<le 62>` non e' un comando: era un segnaposto, ed e' un rilievo della
 * revisione indipendente (docs/bringup/rapporti/revisione-ltr2568-completo.md
 * §3) -- dodici delle 62 funzioni hanno nomi generici che nessun prefisso
 * comune raccoglie, quindi `tools/driverpresenti.py` (che riconosce un
 * driver per PREFISSO di nome) non basta. Provato per davvero, non solo
 * temuto: passare i 62 nomi veri come prefissi da' **64**, non 62, perche'
 * `als_flush`/`ps_flush` catturano anche `als_flush_report`/
 * `ps_flush_report`, due funzioni del driver `alspshub` vicino che non
 * c'entrano nulla. Il confine vero e' l'INTERVALLO DI INDIRIZZI dichiarato
 * sopra ("IL CONFINE"), non un prefisso, e per questo esiste
 * `tools/misurablocco.py` (generalizzazione testata dello script ad hoc
 * scritto per la revisione, `misura62.py`): fabbrica per indirizzo, nostro
 * per nome esatto.
 *
 *   tools/misurablocco.py oracolo/stock.map out-ltr2568/System.map \
 *     --inizio 0xffffff8008783730 --fine 0xffffff8008786c18 \
 *     --extra ltr2568_init=0xffffff8009373bd8
 *
 * I numeri DI QUESTO LOTTO, con TUTTE E 62 LE FUNZIONI SCRITTE e i due
 * difetti di `ltr2568_master_recv` chiusi (vedi D2 e la sezione sui difetti,
 * piu' avanti), calcolati con `tools/intervallo.py` (Clopper-Pearson):
 *
 *   BUILD VERO (out-ltr2568/System.map, il kernel linkato, questo lotto):
 *     totale                              55 su 62 = 88,7%  IC95% [78,1%; 95,3%]
 *     ristretto alle funzioni > 256 byte   8 su 15 = 53,3%  IC95% [26,6%; 78,7%]
 *
 *   MISURA ISOLATA (misuraisolata.py, radice sintetica -- INVARIATA da
 *   questo lotto: rieseguita sul sorgente corretto, da' lo STESSO 51/62 di
 *   prima. `ltr2568_master_recv` era gia' 320/320 anche col difetto `int len`
 *   -- una coincidenza del compilatore sulla radice sintetica, non una prova
 *   che il difetto non ci fosse -- e resta 320/320 ora che il difetto e'
 *   chiuso, per la ragione vera):
 *     totale                              51 su 62 = 82,3%  IC95% [70,5%; 90,8%]
 *     ristretto alle funzioni > 256 byte   5 su 15 = 33,3%  IC95% [11,8%; 61,6%]
 *
 * SUL TOTALE LA MISURA ORA DISCRIMINA, e per la prima volta nella direzione
 * OPPOSTA a quella temuta finora: 88,7% `[78,1%; 95,3%]` NON contiene il
 * 77,10% del ramo ALPS -- il limite inferiore dell'intervallo sta SOPRA il
 * valore di riferimento, cioe' su questo driver la convergenza misurata e'
 * risultata piu' alta di quella del ramo, non piu' bassa. Non e' un
 * artefatto: chiudere `ltr2568_master_recv` (320 byte, sopra la soglia dei
 * 256) sposta di un'unita' SIA il totale SIA il ristretto, ed e' l'unico
 * cambiamento fra questo lotto e il precedente -- nessun'altra funzione ha
 * mosso la sua dimensione.
 *
 * RISTRETTO, IL MARGINE SI E' RIBALTATO: era 46,7% `[21,3%; 73,4%]`, che
 * ESCLUDEVA il 77,10% per quasi quattro punti; con l'ottava chiusura --
 * proprio quella che il lotto precedente aveva calcolato come sufficiente,
 * senza sapere che sarebbe stata `master_recv` -- e' 53,3% `[26,6%; 78,7%]`,
 * che CONTIENE il 77,10% e quindi non discrimina piu'. Il verdetto
 * "convergenza sul filo di una funzione" era vero nella lettura letterale:
 * la funzione era proprio quella.
 *
 * LA MISURA ISOLATA NON HA PIU' NIENTE DA DIRE SU QUESTO DRIVER, ed e' la
 * lezione che resta per gli altri: due divergenze diverse (`i2c_resume` e
 * `i2c_probe`, non `master_recv`) separano ancora isolata e build vero, ma
 * nessuna delle due e' un difetto -- `i2c_resume` e' un artefatto del
 * linker (sotto), `i2c_probe` e' aperta in entrambe le misure. Il disaccordo
 * che SEMBRAVA il piu' grosso (`master_recv`, -20 nel build vero contro =
 * nell'isolata) non era affatto un disaccordo fra strumenti: era un difetto
 * vero che l'isolata, per una coincidenza del compilatore sulla sua radice
 * sintetica, non vedeva. La revisione indipendente lo ha misurato con un
 * terzo oggetto che nessuna delle due misure guardava -- il `.o` vero del
 * build -- e ha trovato che coincide col kernel linkato su tutte e dodici le
 * funzioni contese: **il disaccordo fra le due misure e' quindi tutto dentro
 * il compilatore** (radice sintetica contro radice vera), il linker non
 * c'entra, e il numero difendibile e' **5 ribaltamenti su 62 = 8,1%
 * `[2,7%; 17,8%]`, non 7 su 62 = 11%** come scritto qui fino al lotto
 * precedente. La frase "cioe' quanto il segnale cercato" (11% contro il
 * 77,10%) era una coincidenza numerica scambiata per un argomento -- sono
 * due grandezze diverse, e va ritirata (revisione-ltr2568-completo.md §4.4,
 * §4.5).
 *
 * `ltr2568_i2c_resume` NON e' una divergenza del driver e va tolta dal
 * conto: nel `.o` vero del build (`objdump -t .../ltr2568.o`, non
 * `System.map`) e' 124/124 identica come nell'isolata. Il +4 che compariva
 * in alcuni build linkati e' il rammendo del linker per l'errata ARM 843419
 * caduto subito dopo la funzione -- si vede o non si vede a seconda di quale
 * simbolo il linker mette li' accanto, ed e' un artefatto di disposizione,
 * non codice del driver (revisione-ltr2568-completo.md §3, §4.3). In questo
 * lotto NON compare (misurablocco.py sopra non la elenca fra le diverse):
 * la prova che e' un artefatto di build e non del sorgente e' che lo stesso
 * `ltr2568.c`, immutato su questa funzione, da' esiti diversi a seconda del
 * build.
 *
 * ---------------------------------------------------------------------------
 * LE SEDICI DIVERGENZE DI DIMENSIONE, UNA PER UNA
 * ---------------------------------------------------------------------------
 * Erano dodici quando quattro funzioni erano fuori; ora sono sedici -- OTTO
 * chiuse e OTTO aperte, non piu' cinque/undici. Dove la causa e' localizzata
 * la scrivo; dove non lo e', lo dico invece di inventarla.
 *
 * QUESTO LOTTO chiude un difetto vero (D2, due cause distinte, sotto) e
 * RIPASSA l'intero elenco contro la misura che questo file dichiara
 * autorevole dal lotto precedente -- il build vero, non piu' l'isolata. Il
 * ripasso non e' stato fatto quando la misura autorevole e' cambiata, ed e'
 * il rilievo di processo piu' grave della revisione indipendente
 * (docs/bringup/rapporti/revisione-ltr2568-completo.md §7.3): D10, D11 e D12
 * erano scritte "aperte"/"non indagato" mentre la tabella del build vero, tre
 * pagine sopra nello stesso file, le mostrava gia' chiuse; D2 era sotto
 * "CHIUSE" mentre il testo appena sopra spiegava perche' non lo era piu'; D6
 * portava il numero della misura isolata (-64) invece di quello del build
 * vero (-16). Corretto qui, una voce alla volta, con la causa quando la causa
 * e' cambiata da quando e' stata scritta.
 *
 * Le quattro nuove (D13..D16) restano in fondo. I numeri delle 58 funzioni
 * gia' scritte nei lotti precedenti sono IDENTICI, funzione per funzione, a
 * quelli di questo lotto -- l'unica che ha cambiato dimensione e' D2.
 *
 *  CHIUSE
 *
 *   D1 ltr2568_eint_handler  -4 -> 88/88. DIFETTO VERO, non solo di
 *      dimensione: il .c metteva ltr2568_obj in una locale, mentre di
 *      fabbrica il globale viene RILETTO dopo disable_irq_nosync()
 *      ("f9405a68 ldr" x8,[x19,#176] a 0xffffff8008783904 e di nuovo a
 *      0xffffff8008783910, poi "b40000c8 cbz" x8). E' una differenza di
 *      COMPORTAMENTO: la rilettura avviene dopo una chiamata che puo'
 *      dormire. Corretto.
 *   D2 ltr2568_master_recv -- CHIUSA PER DAVVERO SOLO ORA, con DUE difetti
 *      distinti, entrambi trovati dalla revisione indipendente
 *      (revisione-ltr2568-completo.md §7.1, §7.2) e corretti in questo lotto.
 *      Una revisione precedente aveva gia' corretto un primo scarto -- la
 *      lettura volatile scartata di obj->trace ("b9405d1f ldr" wzr,[x8,#92],
 *      due sole occorrenze nel blocco), dichiarata senza forma sorgente
 *      finche' non si e' vista che la forma esiste e non inventa niente:
 *      `atomic_read(&ltr2568_obj->trace);` -- atomic_read e' READ_ONCE, cioe'
 *      volatile, e il compilatore deve emetterla anche se il valore non
 *      serve -- e quella correzione era ed e' giusta. Ma bastava a chiudere
 *      SOLO la misura isolata (320/320), non il kernel vero, dove la
 *      funzione restava a 300/320 (-20): la dichiarazione "chiusa a 320/320"
 *      di una revisione precedente era una chiusura della radice sintetica,
 *      non del driver.
 *      (a) COMPORTAMENTO, non dimensione: sul cammino in cui `i2c_transfer`
 *      non ritorna 2 messaggi, il .c restituiva `-EREMOTEIO` (-121) mentre
 *      la citazione di disassemblato accanto a quella riga -- "321d7bf8
 *      orr" w24, wzr, #0xfffffffb -- vale **-5 = -EIO**, non -EREMOTEIO.
 *      La citazione era giusta; era il codice C sbagliato, la
 *      forma speculare del dato inventato trovato altrove nel file (li' la
 *      citazione sbagliata provava la costante giusta, qui la citazione
 *      giusta prova che il codice e' sbagliato). Confermato sul binario di
 *      fabbrica per confronto diretto: `mov w24, #0xfffffffb` (fabbrica,
 *      -EIO) contro `mov w24, #0xffffff87` (nostro, -EREMOTEIO), stesso
 *      punto. Perche' nessuno l'aveva visto: nessuna misura di dimensione la
 *      tocca (un'istruzione contro un'istruzione, stesso numero di byte), e
 *      `tools/verificaistruzioni.py` conferma che la codifica `321d7bf8`
 *      esiste nel blocco col mnemonico `orr` ma NON CONFRONTA GLI OPERANDI,
 *      quindi non si accorge che l'immediato citato contraddice il codice
 *      accanto. Corretto: `err = -EIO;`.
 *      (b) IL QUARTO PARAMETRO e' `u8 len` di fabbrica, `int len` da noi.
 *      Di fabbrica il valore e' troncato a 8 bit DUE volte
 *      ("12001c79 and" w8, w19, #0xff prima del confronto con 9, e di nuovo
 *      prima di scriverlo in `msgs[1].len` -- AAPCS64: il chiamato non puo'
 *      assumere puliti i bit alti di un parametro piu' stretto di un
 *      registro). Da noi nessuno dei due `and`. Chiuso con un esperimento
 *      appaiato: copia del sorgente, UN SOLO TOKEN cambiato (`int len` ->
 *      `u8 len`), compilata con la riga di comando vera del build --
 *      300 -> **320/320 esatti**, e l'unica riga che cambia nell'intero
 *      oggetto e' quella (incluso `ltr2568_master_send`, che resta a 288: la
 *      sua -20 residua e' D9, causa diversa e non condivisa). Corretto: il
 *      parametro e' ora `u8 len`.
 *      Nel BUILD VERO, con entrambe le correzioni: **320/320 esatta**, e la
 *      citazione di errno torna a dire il vero. Vedi anche `ltr2568.c` alla
 *      funzione stessa per il codice corretto.
 *   D3 als_get_data  -188 -> 320/320. ARTEFATTO DELLA RADICE: di fabbrica
 *      ltr2568_get_als_value (224 byte qui, nome noto solo dal `__func__`
 *      perche' un simbolo non ce l'ha) e' incorporata qui dentro, e la radice
 *      la teneva fuori linea. Il codice non e' cambiato di una riga.
 *   D4 ltr2568_i2c_remove  -156 -> 228/228. ARTEFATTO DELLA RADICE in due
 *      pezzi, misurati separatamente: 132 byte tornano mettendo
 *      ltr2568_delete_attr dove la fabbrica la mette (incorporata qui
 *      dentro), gli ultimi 24 togliendo &ltr2568_attr_list dalla radice dei
 *      dati, che rende di nuovo piegabili le letture della tabella.
 *   D5 ltr2568_als_factory_get_data  +44 -> 68/68. ARTEFATTO DELLA RADICE
 *      (als_get_data non incorporata qui dentro come di fabbrica). Non era
 *      stata rivendicata come artefatto nella prima pubblicazione: e' una
 *      divergenza che spariva da sola.
 *   D10 ltr2568_local_init -- CHIUSA nel build vero: **88/88 esatta**
 *      (`misurablocco.py` sopra). Era scritta "aperta", ma quel -4 era
 *      SOLO della misura isolata: la tabella del build vero, in cima a
 *      questo file, lo mostra chiuso da quando esiste (il ripasso mancato e'
 *      il difetto di processo di cui sopra, non un difetto di codice). La
 *      differenza di codifica descritta in una revisione precedente --
 *      fabbrica "ldrb w8,[x8,#188]; mvn w8,w8; sbfx w0,w8,#0,#1" (tre
 *      istruzioni) contro il nostro "ldrsb w8,[x8]; sub w0,w8,#1" (due) --
 *      era quindi fra fabbrica e la RADICE SINTETICA di misuraisolata.py, non
 *      fra fabbrica e il kernel vero: nel kernel vero il codice coincide
 *      byte per byte. La deduzione che ltr2568_init_flag sia un `bool`
 *      resta comunque corretta (un `int = -1` darebbe una lettura a 32 bit e
 *      un `cmn`, che non compaiono da nessuna parte).
 *   D11 ltr2568_als_enable -- CHIUSA nel build vero: **392/392 esatta**.
 *      Stessa storia di D10: la "fusione di condizioni di ramo" descritta in
 *      una revisione precedente (otto istruzioni di fabbrica contro sei
 *      nostre) era una differenza fra fabbrica e la radice sintetica, non fra
 *      fabbrica e il kernel vero.
 *   D12 ltr2568_ps_set_thres -- CHIUSA nel build vero: **492/492 esatta**.
 *      Era scritta "NON INDAGATO, e lo dichiaro: nessuno ci ha guardato, ne'
 *      l'autore ne' la revisione" -- e la tabella dello stesso file, tre
 *      pagine sopra, la smentiva gia': `ltr2568_ps_set_thres 492 488 (-4)
 *      492 (=)`. Non serviva una nuova indagine, serviva rileggere la
 *      propria tabella (revisione-ltr2568-completo.md §7.3, il rilievo che
 *      pesa di piu' nel lotto).
 *
 *  APERTE
 *
 *   D6 ltr2568_i2c_probe  -16 NEL BUILD VERO (non -64: quel numero era della
 *      misura isolata, scritto qui per un lotto intero senza essere
 *      aggiornato -- lo stesso difetto di processo di D10/D11/D12). Delta
 *      dei mnemonici fra fabbrica e kernel vero: `ldr` -4, `strb` -2,
 *      `strh` -1, `stur` +2, `sturb` +1, `sturh` +1, `and` -1, `lsr` -1,
 *      `ubfx` +1 -- netto -4 istruzioni
 *      (revisione-ltr2568-completo.md §5.5). Il grosso e' scelta di modo
 *      d'indirizzamento (`str`/`stur`) e un'estrazione di campo scritta in
 *      due modi (`and`+`lsr` contro `ubfx`). NON HO UNA CAUSA SORGENTE, e non
 *      ne invento una: la firma e' quella del rumore di versione gia' noto
 *      altrove nel file, ma nessuna delle due classi nominate in D7/D8 e
 *      D13..D16 la spiega per intero. Resta aperta, ora con le istruzioni
 *      contate invece che con un numero solo.
 *   D7 ltr2568_store_alslv +8 e D8 ltr2568_store_alsval +8 -- UN FENOMENO
 *      SOLO, non due. Sono LE STESSE DUE ISTRUZIONI NEGLI STESSI PUNTI di
 *      entrambe le funzioni, dentro read_int_from_buf incorporata: dove la
 *      fabbrica esce dal ciclo degli spazi cadendo direttamente sul blocco
 *      della sscanf ("7100291f cmp" w8,#0xa ; "54ffff20 b.eq" ;
 *      "8b180b28 add" x8,x25,x24,lsl #2), noi emettiamo in mezzo un salto e
 *      una rimaterializzazione di cur = end -- b, codifica 14000002, e
 *      mov x22,x26, codifica aa1a03f6. La seconda e' davvero assente dal
 *      blocco di fabbrica. La prima no -- correzione della revisione
 *      indipendente (revisione-ltr2568-completo.md §7.6): `14000002` e' il
 *      riempitivo generico `b .+8` e compare 10 volte nel blocco di
 *      fabbrica (8764 in tutto il kernel), quindi l'affermazione "e' del
 *      nostro oggetto, non di stock.elf" e' vera del PUNTO (a QUELL'indirizzo
 *      la fabbrica non ha questo salto) e falsa della CODIFICA (quella
 *      sequenza di bit esiste altrove in stock.elf). Per questo nessuna delle
 *      due e' scritta come citazione: non perche' la codifica sia assente in
 *      assoluto, ma perche' non identificherebbe il punto.
 *      Quattro forme sorgente di read_int_from_buf sono state
 *      provate e misurate (due puntatori cur/end come qui; `const char
 *      *cur, *end`; `for (; cur < end && ...; cur++)`; `char *cur` con
 *      contatore): tutte e quattro danno +8/+8. L'UNICA che muove il numero
 *      e' la forma a indice (`size_t i` su buf[i], +4/+4), e IL
 *      DISASSEMBLATO LA ESCLUDE: di fabbrica "add x26, x20, x19" costruisce
 *      il puntatore di fine e "ldrb w8,[x22]" legge dal puntatore che
 *      avanza. Scriverla a indice per far scendere il numero sarebbe
 *      aggiustare qualcosa perche' un numero torni, che e' vietato.
 *      Ipotesi con la causa nominata, non un fatto: rumore di versione del
 *      compilatore, della stessa famiglia degli `orr wN,wzr,#imm` contro
 *      `movz` gia' noti -- un indizio a favore e' che nella stessa funzione
 *      il prologo di fabbrica ordina il frame in modo diverso dal nostro
 *      ("stp x26,x25,[sp,#-80]!" ... "stp x29,x30,[sp,#64]" contro
 *      "stp x29,x30,[sp,#-80]!" ... "mov x29,sp") a parita' di dimensione.
 *      IL CONTROLLO DECISIVO NON ESISTE: i due sibling che conterrebbero lo
 *      stesso read_int_from_buf (cm36558, ltr303) sono spenti in ENTRAMBI i
 *      kernel (`# CONFIG_MTK_CM36558 is not set`), quindi non c'e' una
 *      funzione a sorgente certamente identico su cui calibrare. Resta
 *      aperto, e resta scritto qui perche' il prossimo non ripercorra le
 *      quattro forme gia' provate.
 *   D9 ltr2568_master_send  -20 residuo, dopo D2. Pista, non causa: di
 *      fabbrica questa funzione contiene INCORPORATA ltr2568_i2c_write_block
 *      (la printk alla riga 303 porta il `__func__` di quella, non di
 *      master_send), e qui quel livello di annidamento non c'e'. E' la stessa
 *      forma di difetto che su aw22127 spiego' il +108 di hwen_store.
 *      CONFERMATO NON CONDIVISO con la causa di D2: l'esperimento appaiato
 *      `u8 len` di D2 lascia `ltr2568_master_send` esattamente a 288, invariata
 *      (revisione-ltr2568-completo.md §7.2).
 *
 *   (D10, D11 e D12 erano qui, "aperte"/"non indagato": sono spostate sopra,
 *   fra le CHIUSE, perche' il build vero le mostra gia' chiuse -- vedi il
 *   preambolo di questa sezione.)
 *
 *   NOTA DI METODO PER D13..D16, da leggere prima dei quattro. La
 *   localizzazione istruzione per istruzione qui sotto e' stata fatta sul
 *   `.o` della MISURA ISOLATA, che e' l'unico posto in cui i due flussi si
 *   allineano bene: nel kernel linkato i bersagli dei salti sono indirizzi
 *   veri e l'allineamento automatico si sfalsa. Nel BUILD VERO tre delle
 *   quattro hanno uno scarto DIVERSO, e va detto quale analisi vale dove:
 *
 *     funzione            isolata   build vero   la localizzazione qui sotto
 *     ltr2568_eint_work    -8        -8          vale per entrambi (i tre
 *                                                blocchi si ritrovano
 *                                                identici anche nel linkato,
 *                                                confermato -- D13)
 *     ltr2568_ps_read     -12         0          la DIMENSIONE non serve
 *                                                piu' (1168/1168 nel kernel
 *                                                linkato), ma "292 istruzioni
 *                                                su 292" scritto qui fino al
 *                                                lotto precedente e' FALSO:
 *                                                stessa dimensione, flussi
 *                                                diversi -- 107 istruzioni
 *                                                solo di fabbrica e 107 solo
 *                                                nostre (cbz/csel/salti
 *                                                condizionati scambiati),
 *                                                vedi D14
 *     ltr2568_als_read    -20       -16          vale per l'isolata; nel
 *                                                linkato sono 313 contro 317
 *                                                e la revisione indipendente
 *                                                ha CONFERMATO la stessa
 *                                                causa (bfi/ldrb -> ldrh),
 *                                                vedi D15
 *     ps_enable_nodata    +16       +48          la spiegazione data qui
 *                                                (D16, fusione di condizioni)
 *                                                vale SOLO per l'isolata: la
 *                                                revisione indipendente ha
 *                                                rifatto la localizzazione nel
 *                                                linkato e trovato una causa
 *                                                DIVERSA -- D16 e' riscritta
 *                                                sotto
 *
 *   D13 ltr2568_eint_work  -8, cioe' DUE ISTRUZIONI su 136, ed e' l'unica
 *      delle quattro nuove che si chiude per intero. Confronto istruzione
 *      per istruzione dei due flussi (`cmp.py`, allineamento con difflib
 *      sul testo normalizzato): 134 contro 136, e le uniche due posizioni
 *      non riconducibili a rumore gia' noto sono
 *        (a) la fabbrica emette DUE volte l'`adrp` di ltr2568_intr_flag
 *            (0xffffff8008784198 e 0xffffff80087841f8), noi una sola e la
 *            riusiamo -- eliminazione di sottoespressione comune;
 *        (b) sul ramo `obj->als > 50000` la fabbrica salta
 *            ("54000049 b.ls" piu' "320003f3 orr" w19, wzr, #0x1), noi
 *            convertiamo in predicato -- un csinc w19,w9,wzr,ls di codifica
 *            1a9f9533, che e' del NOSTRO oggetto e non di stock.elf, e per
 *            questo NON e' scritto come citazione (stessa regola di D7/D8).
 *      Tutto il resto del disallineamento e' `orr wN,wzr,#imm` contro `mov`
 *      (la classe gia' misurata su mir3da) e l'ordine del telaio nel
 *      prologo, che e' la stessa firma di versione notata in D7/D8.
 *   D14 ltr2568_ps_read  -12, cioe' TRE ISTRUZIONI su 292, tutte
 *      localizzate:
 *        (a) TRE volte la fabbrica confronta un globale `bool` con 1 in due
 *            istruzioni ("7100051f cmp" w8, #0x1 piu' "b.ne") dove il
 *            nostro clang usa un `cbz` solo -- su ltr2568_als_on
 *            (0xffffff8008784b84), ltr2568_ps_on (0xffffff8008784bb8) e
 *            ltr2568_ps_cali_on (0xffffff8008784cac). Vale -3;
 *        (b) noi emettiamo un salto all'istruzione successiva (+1);
 *        (c) la fabbrica tiene un `cbz w20` MORTO a 0xffffff8008784cc8 (il
 *            valore e' gia' stato provato non nullo a 0xffffff8008784c98),
 *            noi lo togliamo (-1).
 *      Il punto (a) e' stato MESSO ALLA PROVA e non solo affermato:
 *      dichiarando i tre globali `u8` invece di `bool` il nostro clang
 *      emette davvero `cmp #1` e ps_read passa da -12 a -8, ma
 *      ps_enable_nodata peggiora da +16 a +36 e ltr2568_als_enable da -8 a
 *      +8, perche' negli altri punti la fabbrica usa la forma `bool`
 *      ("37001c48 tbnz" w8, #0). Cioe': lo stesso globale ha entrambe le
 *      forme nel binario di fabbrica, il che e' compatibile solo con "e' un
 *      bool, e il clang di fabbrica non ripiega `bool == 1` sul test di
 *      verita'". `bool` resta, e il -12 resta aperto come rumore di
 *      versione. Nel BUILD VERO la dimensione non discrimina piu' (0,
 *      1168/1168), ma la revisione indipendente ha verificato che UGUALE
 *      DIMENSIONE NON VUOL DIRE STESSE ISTRUZIONI: 107 istruzioni presenti
 *      solo di fabbrica e 107 solo nostre nello stesso confronto (delta dei
 *      mnemonici: `ldrb` -4, `ldurb` +4, `add` -3, `sub` +4, `cbz` -2,
 *      `cmp` +1, `csel` -1, `b.le` +2, ...). Parte e' modo d'indirizzamento
 *      innocuo, ma `cbz`/`csel`/i salti scambiati sono differenze di
 *      struttura vere. La frase "292 istruzioni su 292", scritta qui fino al
 *      lotto precedente, era FALSA e va ritirata: la dimensione uguale non
 *      prova il codice identico, ed e' lo stesso strumento
 *      (`driverpresenti.py`) a dirlo nella sua docstring.
 *   D15 ltr2568_als_read  -20, cioe' CINQUE ISTRUZIONI su 317. Quattro
 *      sono UNA cosa sola: la fabbrica assembla i due valori a 16 bit dei
 *      canali con quattro letture di byte piu' due `bfi`
 *      ("394057e8 ldrb" ... "33181d15 bfi" w21, w8, #8, #8 a
 *      0xffffff8008785a00), il nostro clang fonde ogni coppia in una
 *      lettura di mezza parola (`ldrh`). E' l'allargamento delle letture
 *      adiacenti, un'ottimizzazione del compilatore su un buffer di pila
 *      contiguo: nessuna forma sorgente ragionevole la impedisce, e
 *      cercarne una per far tornare il numero sarebbe la cosa vietata. La
 *      quinta istruzione NON e' localizzata, e lo dichiaro.
 *      Era -72 prima che il filtro di assestamento fosse riscritto coi due
 *      rami espliciti invece del ternario -- vedi il commento su
 *      ltr2568_als_filter: quei 52 byte erano una parafrasi mia, non una
 *      differenza del compilatore, e la loro chiusura e' l'unico punto di
 *      questo lotto in cui il disassemblato ha corretto il sorgente.
 *      CONFERMATO nel build vero (-16 anziche' -20, una istruzione in meno
 *      per l'isolata: la stessa causa, `bfi`+`ldrb` di fabbrica contro
 *      `ldrh` nostro, regge nel kernel linkato -- delta dei mnemonici:
 *      `bfi` -3, `ldrb` -4, `ldrh` +2 -- ed e' esattamente il fenomeno
 *      descritto sopra. revisione-ltr2568-completo.md §5.2.
 *   D16 ps_enable_nodata  -- RISCRITTA: LA SPIEGAZIONE DATA (fusione di
 *      condizioni, "+16, quattro istruzioni su 291") VALEVA SOLO PER IL `.o`
 *      SINTETICO. Nel build vero lo scarto e' +48, cioe' +12 istruzioni, non
 *      +4, e la causa e' un'altra (revisione-ltr2568-completo.md §5.1):
 *      **duplicazione della coda con lock**. Il delta dei mnemonici mostra
 *      `bl` +2 -- due CHIAMATE in piu' della fabbrica -- cosa che una
 *      fusione di condizioni non produce. Le chiamate in piu' sono
 *      `mutex_lock`: la fabbrica ne emette UNA sola copia della coda
 *      `lock; if (en) set_bit else clear_bit; unlock` (confermato: `bl
 *      mutex_lock`/`bl mutex_unlock` compaiono una volta ciascuno nel blocco
 *      di fabbrica, agli indirizzi 0xffffff8008784944/0xffffff800878496c);
 *      il nostro clang CLONA quella coda in tre predecessori diversi,
 *      condividendo solo l'uscita -- confermato sul nostro kernel linkato:
 *      **tre** `bl mutex_lock` (non uno) contro **un solo** `bl
 *      mutex_unlock`. NON E' UNO SBILANCIAMENTO DEL LOCK: nel sorgente c'e'
 *      un solo `mutex_lock`/`mutex_unlock` (riga 2687 e dintorni), il lock e'
 *      bilanciato su ogni cammino d'esecuzione, ed e' il compilatore a
 *      duplicare la coda condivisa tre volte invece di saltarci una sola
 *      volta. Le dodici istruzioni non localizzate del lotto precedente
 *      erano in realta' questo: undici hanno ora un nome (i tre `mutex_lock`
 *      in piu', i `csel`/`cset`/`cmp` che li accompagnano), una resta
 *      residua e non localizzata. Aperto, con la causa vera al posto di
 *      quella sbagliata.
 *
 *   D17 LA DISPOSIZIONE DEI DATI IN `.bss` NON E' QUELLA DI FABBRICA, e
 *      questa e' la prima volta che su questo progetto la si puo' guardare:
 *      l'oracolo non ha simboli di dati (HANDOFF §8), ma il NOSTRO build
 *      linkato ce li ha, e gli spiazzamenti di fabbrica si leggono dalle
 *      istruzioni. Prendendo `ltr2568_obj` come zero da entrambe le parti
 *      (`grep " b .*ltr2568" out-ltr2568/System.map`):
 *
 *        variabile            fabbrica   nostro
 *        ltr2568_obj                +0       +0
 *        ltr2568_irq_enabled        +8       +8
 *        ltr2568_init_flag         +12      +12
 *        ltr2568_i2c_client        +16      +16
 *        ltr2568_intr_flag         +24      +24
 *        ltr2568_als_on            +28      +25
 *        ltr2568_ps_on             +32      +26
 *        ltr2568_ps_zero_cnt       +36      +28
 *        ltr2568_ps_cali_on        +40      +32
 *        ltr2568_ps_base           +44      +44
 *        ltr2568_ps_idx            +48      +36
 *        ltr2568_ps_full           +50      +38
 *        ltr2568_ps_stable         +52      +40
 *        ltr2568_ps_buf            +56      +48
 *        ltr2568_meas_rate         +72      +76
 *        ltr2568_als_last          +76      +92
 *        ltr2568_als_first         +80      +80
 *        ltr2568_als_prev1         +84      +84
 *        ltr2568_als_prev0         +88      +88
 *        ltr2568_thres_high        +92      +72
 *        ltr2568_thres_low         +96      +68
 *        ltr2568_thres_override   +100      +64
 *
 *      Le prime cinque coincidono, poi diverge. Due fatti: (1) di fabbrica
 *      ogni variabile da un byte occupa uno spiazzamento allineato a
 *      quattro, da noi le tre bool consecutive sono impacchettate in tre
 *      byte adiacenti; (2) di fabbrica l'ordine degli spiazzamenti coincide
 *      con l'ordine in cui le variabili sono dichiarate qui sopra (con
 *      un'eccezione: `ltr2568_init_flag`, fabbrica +12, e' dichiarato PRIMA
 *      di `ltr2568_irq_enabled`, fabbrica +8 -- diciotto coppie su
 *      diciannove coincidono, la prima no, e l'eccezione non era dichiarata
 *      finche' la revisione indipendente non l'ha trovata), da noi no --
 *      clang le riordina.
 *
 *      LA CAUSA E' ORA ACCERTATA, non piu' ignota (revisione-ltr2568-completo.md
 *      §6.1): il nostro clang RAGGRUPPA per allineamento -- gli oggetti da
 *      otto byte, poi i byte adiacenti, poi i quattro byte, spostando
 *      `ltr2568_ps_base` dopo `ps_stable` -- la fabbrica INTERLACCIA
 *      larghezze diverse senza raggrupparle (200 u8, 204 bool, 208 bool,
 *      212 int, 216 bool, 220 int, 224 u16, 226 bool, 228 u16). **Un
 *      allocatore che raggruppa per allineamento non puo' produrre una
 *      disposizione interlacciata sotto NESSUN ordine di dichiarazione**:
 *      qualunque fosse l'ordine del sorgente di fabbrica, il nostro clang le
 *      avrebbe comunque raggruppate. Quindi l'alternativa che questo
 *      commento si asteneva dallo scegliere -- "il sorgente di fabbrica le
 *      dichiarava in un altro ordine ancora" -- non salva l'ipotesi: la
 *      differenza e' nell'ALLOCATORE (il compilatore di fabbrica dispone
 *      `.bss` nell'ordine di dichiarazione, il nostro no), non in un ordine
 *      di dichiarazione diverso che nessuno conosce. Riordinare le
 *      dichiarazioni per far combaciare `.bss` sarebbe comunque aggiustare
 *      qualcosa perche' un numero torni, e NON VA FATTO: la causa si scrive,
 *      il codice no. Le due misure di questo file -- dimensione delle
 *      funzioni e citazioni -- sono cieche su tutto questo, che e'
 *      esattamente il motivo per cui vale la pena scriverlo.
 *
 * Una nota che vale per D13..D16 e che il prossimo non deve rifare: la
 * conversione in predicato (`csel`/`csinc`/`cneg` al posto di un salto) e
 * l'allargamento delle letture sono le DUE classi che separano il nostro
 * clang r383902 da quello di fabbrica r353983c su questo file, e compaiono
 * in tre delle quattro funzioni. Sono la stessa famiglia degli
 * `orr wN,wzr,#imm` contro `movz` gia' nominati in HANDOFF §5. Il controllo
 * decisivo -- una funzione a sorgente certamente identico su cui calibrarle
 * -- qui non esiste, per la stessa ragione di D7/D8: i due sibling che
 * conterrebbero lo stesso codice (cm36558, ltr303) sono spenti in ENTRAMBI i
 * kernel. Resta un'ipotesi con la causa nominata, non un fatto.
 *
 * Una funzione in piu' e' segnalata dallo strumento come NON IN FABBRICA e
 * non e' una divergenza: ltr2568_exit (4 byte), che di fabbrica non ha
 * simbolo. **La ragione scritta qui prima era sbagliata** («per un driver
 * built-in .exit.text viene scartata»): su arm64 `.exit.text` non viene MAI
 * scartata (arch/arm64/kernel/vmlinux.lds.S righe 18-19,
 * `#define ARM_EXIT_KEEP(x) x`, con il commento «.exit.text needed in case of
 * alternative patching»). A mancare non e' il codice, e' il SIMBOLO:
 * `oracolo/stock.map` e' `nm -n` di uno stock.elf la cui symtab viene dalla
 * tabella kallsyms dell'immagine, e kallsyms con
 * `# CONFIG_KALLSYMS_ALL is not set` emette solo i simboli in [_stext,_etext]
 * e [_sinittext,_einittext] (alps-mtkwatch/scripts/kallsyms.c righe 48-51).
 * `.exit.text` comincia esattamente a `_einittext` = 0xffffff80093a8518, cioe'
 * dove la mappa finisce -- vedi il commento sulla funzione in fondo al file.
 *
 * ---------------------------------------------------------------------------
 * IL DRIVER E' INNESTATO NEL KERNEL -- come, e come si verifica
 * ---------------------------------------------------------------------------
 * Fino a ieri questo file non era agganciato a niente: senza le quattro
 * funzioni non si linkava, e `grep ltr2568 System.map` dava ZERO. Adesso
 * l'innesto e' fatto e si compone di tre pezzi, tutti necessari:
 *
 *  1. `drivers/misc/mediatek/sensors-1.0/alsps/ltr2568/Kconfig`, che
 *     definisce `ODM_LTR2568`. E' il pezzo che si dimentica: il config di
 *     fabbrica ha `CONFIG_ODM_LTR2568=y`, ma se il simbolo non esiste
 *     nell'albero `olddefconfig` scarta quella riga SENZA DIRE NIENTE, e il
 *     driver non compare mai nel build. Era uno dei ventuno `=y` in quella
 *     condizione; ora sono venti (`scripts/build-kernel-stock.sh config`
 *     stampa "=y di fabbrica non presenti da noi: 20", ed era 21).
 *  2. il `Makefile` della stessa cartella, piu' le due righe che la
 *     agganciano al padre (`source .../ltr2568/Kconfig` e
 *     `obj-$(CONFIG_ODM_LTR2568) += ltr2568/`).
 *  3. `patches/kernel-stock/ltr2568-dal-disassemblato.patch`, generata e
 *     non scritta a mano, che porta i tre file su un albero pristino.
 *     Verificata con `tools/verificapatch.py`, che la applica a un
 *     `git archive 952d88e94` e confronta il risultato col sorgente
 *     committato: esito `OK` al commit di questo lotto. In questo progetto
 *     la correzione che non arriva nell'artefatto derivato e' mancata 3
 *     volte su 3, ed e' la ragione per cui quello strumento esiste.
 *
 * La prova che l'innesto funziona non e' che il build esce con zero: e'
 * `grep` sulla `System.map` del build vero.
 *
 *   awk '{print $3}' out-ltr2568/System.map | grep -c ltr2568   ->  87
 *
 * Ottantasette simboli il cui nome contiene `ltr2568` dove prima ce n'erano
 * zero: 52 funzioni (le 62 meno le DODICI che di fabbrica hanno nomi
 * generici -- i sei `als_*` e i sei `ps_*`, non dieci come scritto qui fino
 * al lotto precedente -- piu' `ltr2568_exit` che di fabbrica non ha
 * simbolo), cinque tabelle `r`, OTTO dati `d` (non sette: `ltr2568_init_info`,
 * `ltr2568_i2c_driver`, `ltr2568_factory_device`, `ltr2568_als_gain`,
 * `ltr2568_ps_offset`, `ltr2568_mutex`, `ltr2568_als_gainrange`,
 * `ltr2568_factory_fops` -- verificato con
 * `grep ltr2568 out-ltr2568/System.map | awk '{print $2}' | sort | uniq -c`),
 * ventidue `b`, e -- quello che conta per chi lo flashera' --
 * `__initcall_ltr2568_init6`, cioe' il driver ha davvero il suo initcall e la
 * sonda verra' chiamata. Il totale 87 era gia' giusto; la sua scomposizione
 * no, in due punti (revisione-ltr2568-completo.md §7.6).
 *
 * ---------------------------------------------------------------------------
 * UN DIFETTO DELLO STRUMENTO DELLE CITAZIONI, CHE NON E' PIU' UN DIFETTO
 * ---------------------------------------------------------------------------
 * QUESTA SEZIONE E' UN REGISTRO: il difetto qui sotto e' stato corretto
 * nello strumento, e rigirandolo oggi il verdetto e' l'opposto. Al commit di
 * questo lotto:
 *
 *   verificacitazioni.py ltr2568.c oracolo/stock.elf
 *   letterali: 64   citati: 65   verificati: 65   probanti: 52   deboli: 13
 *   di cui verificate come messaggio assemblato dalla macro di log: 36
 *   di cui verificate come nome di funzione da __func__: 6
 *   soglia imposta: 64 citati richiesti (64 letterali - 0 eccezioni)
 *   $? = 0
 *
 * Zero `NON_ANCORATA`, non 37: lo strumento ora riconosce il messaggio
 * assemblato dalla macro (causa (a) qui sotto) e i `__func__` (causa (b)), e
 * non conta piu' i percorsi delle `#include` fra i letterali (causa (c)).
 * Le tre cause erano state descritte giuste; il rimedio e' arrivato.
 *
 * E le citazioni di istruzione, che sono un'altra rete, ne hanno trovato uno
 * VERO in questo lotto: `verificaistruzioni.py` da' **374** citazioni (non
 * 375 -- quel numero era il totale della stesura PRECEDENTE, in cui una
 * delle 375 era prosa mal riconosciuta e risultava ASSENTE, cioe' 374 su
 * 375; sommare il totale vecchio col verdetto nuovo dava 375, ed era un
 * errore aritmetico, non una nuova citazione trovata -- revisione-ltr2568-
 * completo.md §7.6), e la prima stesura ne aveva quattro ASSENTI -- tre
 * erano le codifiche del NOSTRO oggetto scritte al posto di quelle di
 * fabbrica (stesse istruzioni, registri diversi) sotto le costanti 14560 e
 * 1000 della conversione in lux. Vedi la sezione sul dato inventato piu'
 * avanti. Corrette: **374 su 374**, riprodotto indipendentemente:
 *
 *   verificaistruzioni.py ltr2568.c stock.elf --mappa stock.map \
 *     --intervallo 0xffffff8008783730:0xffffff8008786c18 \
 *     --intervallo 0xffffff8009373bd8:0xffffff8009373bf8
 *   citazioni di istruzione trovate nel sorgente: 374 (374 a codifica, 0 ad indirizzo)
 *   confermate: 374   assenti: 0   mnemonico diverso: 0   controfattuali: 0
 *
 * Il testo storico delle tre cause, che resta utile perche' vale per tutta
 * la famiglia `sensors-1.0`:
 *
 * `tools/verificacitazioni.py` su questo file esce con 37 `NON_ANCORATA` e
 * codice 1. Nessuna delle 37 e' un difetto di questo driver, e la revisione
 * indipendente lo ha verificato A MACCHINA, non a occhio: tutte e 34 le
 * citazioni composte dalla macro APS_ERR hanno la coda presente come
 * letterale vero nel codice (34 su 34), e tutte e 60 le citazioni di stringa
 * del file trovano all'indirizzo citato in stock.elf esattamente il testo
 * citato, byte per byte (60 su 60). Le cause sono tre e distinte:
 *
 *   (a) 34 letterali composti dalla macro APS_ERR, che antepone KERN_ERR, il
 *       tag e "%s %d : ": nel codice il letterale e' solo la CODA, e lo
 *       strumento cerca il testo citato intero. E' lo stampo di OGNI driver
 *       `sensors-1.0` di MediaTek, quindi la rete e' cieca su tutta la
 *       famiglia -- mt5725, FT8719, GT917S, ILITEK la incontreranno tutti;
 *   (b) 3 sono `__func__` ('als_enable_nodata', 'als_get_data',
 *       'ps_get_data'): array generati dal compilatore, NON ANCORABILI PER
 *       COSTRUZIONE. Una correzione della causa (a) lascerebbe aperti questi;
 *   (c) i due percorsi di #include ('inc/alsps.h', 'inc/cust_alsps.h') sono
 *       contati fra i letterali del codice e gonfiano la soglia imposta da
 *       62 a 64.
 *
 * Non e' corretto qui perche' quello strumento e' in mano a un altro agente:
 * e' segnalato, non aggiustato. Le citazioni di ISTRUZIONE, che sono un'altra
 * rete, passano invece per intero: 172 su 172 confermate.
 *
 * [FINE DEL TESTO STORICO. Tutte e tre le cause sono state chiuse nello
 * strumento da chi lo ha in mano, e il conteggio delle citazioni di
 * istruzione e' salito da 172 a 374 (non 375: vedi sopra) con quel lotto.]
 *
 * ---------------------------------------------------------------------------
 * LE ULTIME QUATTRO: COSA ERA STATO DICHIARATO E COSA E' RISULTATO
 * ---------------------------------------------------------------------------
 * Fino a questo lotto qui c'era scritto che ltr2568_ps_read (1168 byte),
 * ltr2568_als_read (1268), ps_enable_nodata (1164) e ltr2568_eint_work (544)
 * erano fuori perche' "una dozzina di variabili globali il cui significato non
 * si ricava dal blocco". Adesso sono scritte tutte e quattro, e vale la pena
 * dire COSA ha spostato la conclusione, perche' non e' stata la bravura di
 * nessuno: e' stato un solo cambio di finestra.
 *
 * Il significato dei globali non si ricava dalla funzione che li usa, ma si
 * ricava dall'INSIEME degli accessi. La prima misura pubblicata qui era un
 * `grep` sulla sola finestra del blocco, e sbagliava su tre punti (contava
 * nove globali invece di otto, tre lettori/tre scrittori invece di due/due,
 * e includeva `ltr2568_eint_work` fra le funzioni che li usano quando non ne
 * tocca nessuno): la versione corretta, misurata su TUTTO il disassemblato
 * di fabbrica e non solo sulla finestra, e' scritta per esteso sopra la
 * dichiarazione di ltr2568_ps_on. La conclusione operativa sopravvive per una
 * ragione piu' debole di quella scritta all'origine: gli otto globali sono
 * toccati solo da `ltr2568_ps_read` e `ps_enable_nodata` (due funzioni, non
 * tre) e nessun quarto lettore esiste in tutto il kernel. Con la finestra su
 * una funzione sola quei globali sono un'incognita ciascuno; con la finestra
 * sul blocco intero sono lo stato di UNA macchina, e i nomi seguono
 * dall'uso.
 *
 * Le fonti che erano state indicate e che cosa ha dato ciascuna, per non
 * lasciarlo dedurre a chi verra' dopo:
 *
 *   - `tools/relocazioni.py` -- NON SERVITA qui. Questi globali non sono
 *     puntatori ne' tabelle: sono contatori e flag in `.bss`, e `.rela.dyn`
 *     non ha niente da dire su di loro. Lo scrivo perche' la sua utilita' e'
 *     stata dichiarata a priori, e su questo file non si e' realizzata.
 *   - `__func__` -- DECISIVA, e due volte. Ha dato il nome e i confini di
 *     ltr2568_ps_get_thres (riga 642, incorporata in DUE chiamanti) e di
 *     ltr2568_ps_enable (riga 833). Senza, i due blocchi sarebbero stati
 *     scritti in linea e i loro `__func__` nelle printk sarebbero risultati
 *     sbagliati -- una divergenza che nessuna misura di dimensione vede.
 *   - `tools/ddebug.py` -- VERIFICATA, non ereditata: continua a dare ZERO
 *     descrittori per questo driver (`ddebug.py stock.elf --tutte |
 *     grep -ci "ltr2568\|alsps/ltr"` -> 0). Lo zero e' vero.
 *   - `__FILE__` negli ASSERT e `module_version_attribute` -- assenti come
 *     gia' scritto sopra.
 *   - `tools/misuraisolata.py` con `--radice fabbrica --fuori-radice
 *     ltr2568_attr_list` -- e' cio' che ha reso il lavoro possibile: le
 *     quattro funzioni sono statiche e senza radice non sarebbero
 *     misurabili, e senza misura non ci sarebbe stato modo di sapere se una
 *     forma sorgente era giusta.
 *
 * IL DATO INVENTATO E' STATO CERCATO, e in un punto lo strumento me lo ha
 * trovato addosso. La conversione in lux (D15) contiene un moltiplicatore
 * 14560 e un divisore 1000 che sarebbero stati facilissimi da "arrotondare" a
 * numeri belli, e vengono da "52871c0b mov" w11, #0x38e0 e dalla costante
 * magica "5289ba6c mov" w12, #0x4dd3 + "72a20c4c movk" w12, #0x1062, lsl #16
 * con "9366fd29 asr" x9, x9, #38, che e' la divisione per 1000.
 *
 * La prima stesura di questo commento citava pero' tre codifiche diverse --
 * 52871c09 / 5289ba6b / 72a20c4b, scritte qui separate da barre e non da
 * virgole apposta, perche' "<otto esadecimali> <parola>" e' la forma che
 * verificaistruzioni.py legge come citazione e la parola "e" gli era
 * bastata: sono le stesse istruzioni con i registri del NOSTRO oggetto
 * (w9/w11 invece di w11/w12), copiate dal disassemblato sbagliato.
 * `tools/verificaistruzioni.py` le ha date ASSENTI -- "chiave mai presente
 * nel blocco" -- ed e' l'unica cosa che le ha viste: i valori 14560 e 1000
 * erano giusti, quindi nessuna misura di dimensione e nessuna rilettura del
 * senso avrebbe segnalato niente. Vale la pena scriverlo perche' e' la
 * classe "dato inventato" nella sua forma piu' subdola: una citazione che
 * PROVA la costante giusta con l'istruzione sbagliata.
 *
 * Dove invece NON ho una prova -- il nome di ltr2568_als_filter, che nel
 * binario non compare perche' dentro non c'e' nessuna printk -- e' scritto
 * sulla funzione che il nome e' mio.
 *
 * Restano invece fuori, e lo dichiaro: nessuna di queste quattro e' stata
 * confrontata istruzione per istruzione OLTRE le posizioni elencate in
 * D13..D16. Su ps_enable_nodata tre istruzioni su 291 e su als_read una su
 * 317 non sono localizzate.
 */

#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/sched/clock.h>

/*
 * Gli include sono nella forma che compila con la riga di comando vera del
 * sottoalbero `alsps` (`-I.../sensors-1.0/alsps`), la stessa che usa `alsps.c`.
 * Di fabbrica il Makefile del sottodirectory aggiungerebbe `-I.../alsps/inc` e
 * la forma sarebbe `#include "cust_alsps.h"`: e' una differenza di forma del
 * sorgente che non cambia una sola istruzione.
 */
#include "inc/cust_alsps.h"
#include "inc/alsps.h"

/*
 * Il tag e la forma dei messaggi vengono dai letterali: ogni printk del driver
 * comincia per '\x013[ALS/PS] %s %d : '. \x01 seguito da '3' e' KERN_ERR,
 * quindi la fabbrica usa KERN_ERR anche per i messaggi informativi. Nella corsa
 * 0xffffff80091bdea3..0xffffff80091be827 non c'e' UN SOLO letterale con
 * prefisso \x016 (KERN_INFO).
 */
#define LTR2568_DEV_NAME	"ltr2568"	/* "ltr2568"@0xffffff80091bdfb7 */
#define APS_TAG			"[ALS/PS] "
#define APS_ERR(fmt, args...)	printk(KERN_ERR APS_TAG "%s %d : " fmt, \
					__func__, __LINE__, ##args)

/*
 * struct ltr2568_priv -- gli offset vengono tutti dal disassemblato, non da
 * ltr303/ltr559. La dimensione totale e' quella dell'allocazione:
 *   "52802502 mov" w2, #0x128 (296) in ltr2568_i2c_probe, argomento di
 *   kmem_cache_alloc_trace.
 * I flag dell'allocazione sono "52901801 mov" w1, #0x80c0 piu'
 * "72a02801 movk" w1, #0x140, lsl #16 -> 0x014080c0, cioe' GFP_KERNEL con
 * __GFP_ZERO: e' kzalloc, non kmalloc. Subito dopo il sorgente azzera di nuovo
 * tutta la struttura a mano (la sfilza di "a910fc1f stp" xzr,xzr,[x0,#264] ...
 * "a902fc1f stp" xzr,xzr,[x0,#40]), cioe' un memset esplicito che si somma al
 * kzalloc -- come fa anche ltr303.
 */
struct ltr2568_priv {
	struct alsps_hw *hw;			/* +0x00 "a9005015 stp" x21,x20,[x0] */
	struct i2c_client *client;		/* +0x08 idem */
	struct work_struct eint_work;		/* +0x10 "f9000809 str" x9,[x0,#16] */

	/* misc */
	u16 als_modulus;			/* +0x30 "7900600a strh" w10,[x0,#48], w10 = 0x10 */
	atomic_t i2c_retry;			/* +0x34 "b940351a ldr" w26,[x8,#52] */
	atomic_t als_suspend;			/* +0x38 "b900381f str" wzr,[x0,#56] */
	atomic_t als_debounce;			/* +0x3c "b9003c0b str" w11,[x0,#60], w11 = 300 */
	atomic_t als_deb_on;			/* +0x40 "b900401f str" wzr,[x0,#64] */
	atomic_t als_deb_end;			/* +0x44 "b900441f str" wzr,[x0,#68] */
	atomic_t ps_mask;			/* +0x48 "b900481f str" wzr,[x0,#72] */
	atomic_t ps_debounce;			/* +0x4c "b9004c0b str" w11,[x0,#76], w11 = 300 */
	atomic_t ps_deb_on;			/* +0x50 "b900501f str" wzr,[x0,#80] */
	atomic_t ps_deb_end;			/* +0x54 "b900541f str" wzr,[x0,#84] */
	atomic_t ps_suspend;			/* +0x58 "b9405904 ldr" w4,[x8,#88] in show_status */
	atomic_t trace;				/* +0x5c "b9405d03 ldr" w3,[x8,#92] in show_trace */

	struct device_node *irq_node;		/* +0x60 "f9003008 str" x8,[x0,#96] */
	int irq;				/* +0x68 "b9006900 str" w0,[x8,#104] in setup_eint */

	/* dati */
	u16 als;				/* +0x6c "9101b101 add" x1, x8, #0x6c */
	u16 ps;					/* +0x6e "9101b901 add" x1, x8, #0x6e */
	u8 _align;				/* +0x70 */
	u16 als_level_num;			/* +0x72 "b8072009 stur" w9,[x0,#114], w9 = 0x0010000f */
	u16 als_value_num;			/* +0x74 meta' alta dello stesso stur: 16 */
	u32 als_level[C_CUST_ALS_LEVEL - 1];	/* +0x78 store_alslv "def" scrive 120..179 */
	u32 als_value[C_CUST_ALS_LEVEL];	/* +0xb4 store_alsval "def" scrive 180..243 */
	int ps_cali;				/* +0xf4 "b900f51f str" wzr,[x8,#244] */

	/*
	 * +0xf8 e +0xfc: OTTO BYTE di cui non so il nome ne' il tipo. Ci sono
	 * perche' senza di loro ps_thd_val_high cadrebbe a +0xf8 invece che a
	 * +0x100, e +0x100 e' misurato: "b9010109 str" w9,[x8,#256] in
	 * ltr2568_ps_factory_set_threshold. E' la classe di difetto "un buco
	 * nella struttura" di HANDOFF.md.
	 *
	 * La formulazione precedente -- "otto byte che NESSUNA delle 62 funzioni
	 * tocca" -- era un'affermazione assoluta e falsa alla lettera, smentita
	 * da una riga di grep sul blocco:
	 *
	 *   objdump -d --start-address=0xffffff8008783730 \
	 *     --stop-address=0xffffff8008786c18 stock.elf | grep -E ",#(248|252)\]"
	 *
	 * da' cinque riscontri. Quattro sono relativi alla pagina dei globali
	 * (0xffffff8009cba000) e sono variabili gia' dichiarate qui sotto, non
	 * campi di obj; il quinto, "a90ffc1f stp" xzr,xzr,[x0,#248] dentro
	 * ltr2568_i2c_probe, e' relativo a obj ed e' una delle coppie del memset
	 * esplicito. La formulazione giusta e': nessuna funzione legge o scrive
	 * INDIVIDUALMENTE questi due campi; l'unica istruzione che li tocca e'
	 * il memset di i2c_probe, che li copre perche' stanno dentro la
	 * struttura. La conclusione (il buco esiste) non cambia.
	 */
	u32 _sconosciuto_248;
	u32 _sconosciuto_252;

	atomic_t ps_thd_val_high;		/* +0x100 "b9010109 str" w9,[x8,#256] */
	atomic_t ps_thd_val_low;		/* +0x104 "b9010509 str" w9,[x8,#260] */
	atomic_t als_thd_val_high;		/* +0x108 "b9010808 str" w8,[x0,#264] <- hw+168 */
	atomic_t als_thd_val_low;		/* +0x10c "b9010c08 str" w8,[x0,#268] <- hw+172 */
	u32 ps_threshold;			/* +0x110 "b9011008 str" w8,[x0,#272] <- hw+152 */

	ulong enable;				/* +0x118 "91046016 add" x22, x0, #0x118 */
	/*
	 * +0x120. La "a911fc1f stp" xzr,xzr,[x0,#280] copre GLI OTTO BYTE DI
	 * ENTRAMBI i campi (uno stp scrive due word da 8 byte, #280 e #288):
	 * prova che l'ottetto esiste, non che sia UN campo suo. Nel blocco
	 * non c'e' nessun accesso INDIVIDUALE a +288 -- a differenza di
	 * `enable`, che ha la sua "add x22,x0,#0x118" propria. Nome e tipo
	 * vengono dal sibling `ltr303_priv` (`ulong pending_intr;`), come
	 * dichiarato nell'intestazione della struttura: e' la stessa
	 * condizione probatoria di `_sconosciuto_248/252` qui sopra, dove
	 * pero' la ricerca allargata non ha trovato un nome e si e' scritta
	 * l'ignoranza. Qui produce un nome per analogia, non per misura
	 * diretta (revisione-ltr2568-completo.md §7.5).
	 */
	ulong pending_intr;
};						/* fine a 296 = 0x128 */

enum CMC_BIT {
	CMC_BIT_ALS = 1,	/* "320003e0 orr" w0, wzr, #0x1 (clear_bit in probe) */
	CMC_BIT_PS = 2,		/* "321f03e0 orr" w0, wzr, #0x2 (clear_bit in probe) */
};

/*
 * Tabella const a 0xffffff80091be828, otto voci, indicizzata dai bit 3..5 di un
 * registro:
 *   "d343fd08 lsr" x8, x8, #3 ; "927e0908 and" x8, x8, #0x1c ; ldr w8,[x9,x8]
 * in ltr2568_i2c_probe.
 */
static const int ltr2568_als_meas_rate[8] = {
	0, 20, 40, 60, 80, 100, 120, 140,
};

/*
 * I 34 registri letti da ltr2568_show_reg, a 0xffffff8008f53ae0. Il conteggio
 * viene dal limite del ciclo: "f10222ff cmp" x23, #0x88 con passo 4 -> 34.
 */
static const int ltr2568_reg_dump[34] = {
	0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
	0x89, 0x8a, 0x8b, 0x8c, 0x91, 0x92, 0x93, 0x94, 0x95, 0x98,
	0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa4, 0xb6, 0xb7,
	0xad, 0xb4, 0xba, 0xb9,
};

struct alsps_hw alsps_cust;			/* 0xffffff8009cb9fd8 */
static struct alsps_hw *hw = &alsps_cust;

static struct ltr2568_priv *ltr2568_obj;	/* 0xffffff8009cba0b0 */
static struct i2c_client *ltr2568_i2c_client;	/* 0xffffff8009cba0c0 */
static DEFINE_MUTEX(ltr2568_mutex);		/* 0xffffff800992e670 */

/*
 * 0xffffff8009cba0bc. Letto con `ldrb` e azzerato con `strb wzr`, quindi e' un
 * byte, non un int: in ltr2568_local_init la sequenza e'
 *   "3942f108 ldrb" w8,[x8,#188] ; "2a2803e8 mvn" w8, w8 ;
 *   "13000100 sbfx" w0, w8, #0, #1
 * cioe' `return flag ? 0 : -1` su un bool. Un `int init_flag = -1` avrebbe dato
 * una lettura a 32 bit e un `cmn`.
 */
static bool ltr2568_init_flag;

/*
 * 0xffffff8009cba0b8. Messo a 1 da setup_eint dopo request_irq, confrontato con
 * 2 da eint_handler. E' un `int`: "b900b913 str" w19,[x8,#184] (32 bit).
 */
static int ltr2568_irq_enabled;

/* 0xffffff8009cba0c8, scritto con `strb` da ps_get_data: un byte. */
static u8 ltr2568_intr_flag;

/* 0xffffff8009cba0cc, "394332c8 ldrb" w8,[x22,#204] in ltr2568_als_enable. */
static bool ltr2568_als_on;

/*
 * GLI OTTO GLOBALI DELLA CALIBRAZIONE DI PROSSIMITA', 0xffffff8009cba0d0..0x0f7.
 *
 * ERANO DICHIARATI NOVE, CON TRE LETTORI E TRE SCRITTORI CIASCUNO: FALSO SU
 * ENTRAMBI I CONTI. La revisione indipendente
 * (docs/bringup/rapporti/revisione-ltr2568-completo.md §1) ha riletto il
 * comando pubblicato riga per riga e trovato tre difetti, tutti nel comando
 * stesso, non nella conclusione che ne dipende:
 *
 *   - le dichiarazioni sotto l'intestazione "I NOVE GLOBALI" erano OTTO
 *     (`awk 'NR>=756 && NR<=845 && /^static /' ltr2568.c` ne conta otto:
 *     `ltr2568_ps_on` .. `ltr2568_ps_buf`); `ltr2568_meas_rate`, il nono
 *     nominato, sta a 0xf8, FUORI dall'intervallo dichiarato 0xd0..0xf7;
 *   - il comando NON guardava "tutto il kernel di fabbrica": guardava la
 *     finestra delle 62 funzioni di questo blocco, quindi non poteva
 *     sostenere un conteggio dichiarato su "tutto il kernel";
 *   - dentro quella finestra il comando aveva ANCHE falsi positivi (cattura
 *     ogni accesso a offset 200..229 da QUALSIASI registro, non solo dalla
 *     pagina dei globali -- prende `stp xzr,xzr,[x0,#216]` e altri accessi a
 *     campi di struttura che con questi globali non c'entrano) e falsi
 *     negativi (la finestra 200..229 esclude `ltr2568_ps_buf`, che occupa
 *     232..247, e non riconosce la forma indicizzata con cui l'array viene
 *     scritto, `strh w20,[x12,x10,lsl #1]`).
 *
 * LA MISURA VERA, che guarda per davvero tutto il kernel di fabbrica: si
 * disassembla l'intero `stock.elf` (5.018.179 righe) e si segue ogni coppia
 * `adrp`/accesso, invalidando il registro a ogni ridefinizione -- l'unico
 * modo di dire "in tutto il kernel" invece di "dentro la finestra che ho
 * scelto". Script `scanglobali.py` (scritto per la revisione indipendente,
 * riusato qui invariato: e' una misura sul kernel di fabbrica, indipendente
 * da qualunque riga di questo file):
 *
 *   objdump -d oracolo/stock.elf > stock.dis          # 5018179 righe
 *   python3 scanglobali.py stock.dis \
 *     0xffffff8009cba0b0 0xffffff8009cba118
 *
 *   offset  variabile              lettori                    scrittori
 *   +208    ltr2568_ps_on          2 (ps_read, ps_enable_nodata)  1 (ps_enable_nodata)
 *   +212    ltr2568_ps_zero_cnt    1 (ps_read)                    2 (ps_read, ps_enable_nodata)
 *   +216    ltr2568_ps_cali_on     1 (ps_read)                    2 (ps_read, ps_enable_nodata)
 *   +220    ltr2568_ps_base        1 (ps_read)                    2 (ps_read, ps_enable_nodata)
 *   +224    ltr2568_ps_idx         1 (ps_read)                    1 (ps_read)
 *   +226    ltr2568_ps_full        1 (ps_read)                    1 (ps_read)
 *   +228    ltr2568_ps_stable      1 (ps_read)                    1 (ps_read)
 *   +232..247 ltr2568_ps_buf       1 (ps_read)                    1 (ps_read)
 *
 * NESSUNO degli otto ha tre lettori o tre scrittori: il massimo e' DUE e
 * DUE. E `ltr2568_eint_work` NON compare in nessuna riga di questa tabella:
 * l'unica variabile della pagina che tocca e' +200 (`ltr2568_intr_flag`),
 * che non e' uno degli otto -- e' dichiarata qui sopra, prima di questo
 * blocco. Riga di controllo che prova che lo scanner vede davvero il blocco
 * intero: +176 (`ltr2568_obj`) torna con 33 lettori nominati, e +280 cade
 * gia' nel driver `accel` vicino e nomina 18 funzioni `acc_*` -- cioe' lo
 * strumento attraversa il confine del driver, che e' precisamente cio' che
 * serve per dire "tutto il kernel".
 *
 * LA CONCLUSIONE OPERATIVA REGGE COMUNQUE, per una ragione piu' debole di
 * quella scritta: questi otto sono toccati SOLO da `ltr2568_ps_read` e
 * `ps_enable_nodata` (piu' il memset di `ltr2568_i2c_probe`, che li copre
 * perche' stanno nella pagina, non perche' li nomini uno per uno) -- due
 * funzioni, non tre, e nessun quarto lettore esiste in tutto il kernel di
 * fabbrica. Sono lo stato interno di UNA macchina a stati, non variabili
 * indipendenti; il numero di funzioni che le toccano era sbagliato, il fatto
 * che siano un gruppo solo no. Ogni nome qui sotto ha accanto le istruzioni
 * che lo fissano.
 *
 * I nomi sono MIEI: la fabbrica non lascia nomi di dati (l'oracolo non ha
 * simboli di dati, HANDOFF §8). Cio' che e' misurato e' il tipo, la larghezza,
 * l'indirizzo e l'uso; il nome e' una descrizione dell'uso.
 */

/*
 * 0xffffff8009cba0d0. Byte: "39434288 ldrb" w8,[x20,#208] in ps_enable_nodata,
 * "39034297 strb" w23,[x20,#208] (1) e "3903429f strb" wzr,[x20,#208] (0) nei
 * due rami della stessa funzione, "39434108 ldrb" w8,[x8,#208] in
 * ltr2568_ps_read. E' lo stato "PS accesa": ps_enable_nodata lo confronta con
 * la richiesta e se coincide NON tocca il bus.
 */
static bool ltr2568_ps_on;

/*
 * 0xffffff8009cba0d4. Int a 32 bit ("b900d55f str" wzr,[x10,#212],
 * "b940d548 ldr" w8,[x10,#212], "b900d548 str" w8,[x10,#212]). Conta le
 * letture PS consecutive uguali a zero: in ltr2568_ps_read viene incrementato
 * e confrontato con 4 ("7100111f cmp" w8, #0x4), azzerato appena una lettura
 * non e' zero, e azzerato anche da ps_enable_nodata all'accensione.
 */
static int ltr2568_ps_zero_cnt;

/*
 * 0xffffff8009cba0d8. Byte ("39436109 ldrb" w9,[x8,#216],
 * "3903614b strb" w11,[x10,#216] con w11 = 1, "3903611f strb" wzr,[x8,#216]).
 * Dice quale soglia inferiore usa ltr2568_ps_read per decidere se un campione
 * entra nella finestra: se 1 obj->ps_thd_val_low, altrimenti
 * ltr2568_ps_base - 10. Messo a 1 quando quattro zeri di fila aprono una
 * ricalibrazione, azzerato quando la finestra si chiude.
 */
static bool ltr2568_ps_cali_on;

/*
 * 0xffffff8009cba0dc. Int ("b900dd14 str" w20,[x8,#220] in ltr2568_ps_read,
 * "b900dd35 str" w21,[x9,#220] in ps_enable_nodata, "b940dd29 ldr"
 * w9,[x9,#220]). E' il valore PS di riferimento: la media dei cinque campioni
 * alla calibrazione di accensione, o il campione che ha chiuso la finestra.
 */
static int ltr2568_ps_base;

/*
 * 0xffffff8009cba0e0. u16 -- "7941c12a ldrh" w10,[x9,#224] e
 * "7901c12a strh" w10,[x9,#224], sedici bit. E' l'indice nella finestra
 * scorrevole, confrontato con 8 ("f100215f cmp" x10, #0x8).
 */
static u16 ltr2568_ps_idx;

/*
 * 0xffffff8009cba0e2. Byte ("3943896b ldrb" w11,[x11,#226],
 * "3903896c strb" w12,[x11,#226]). Dice che la finestra si e' riempita almeno
 * una volta: da allora ltr2568_ps_read calcola la media invece di accumulare.
 */
static bool ltr2568_ps_full;

/*
 * 0xffffff8009cba0e4. u16 ("7941c9ca ldrh" w10,[x14,#228],
 * "7901c9ca strh" w10,[x14,#228]). Quanti campioni consecutivi della finestra
 * stanno entro +-10 dalla media; a 7 la calibrazione si chiude
 * ("71001d9f cmp" w12, #0x7).
 */
static u16 ltr2568_ps_stable;

/*
 * 0xffffff8009cba0e8..0x0f7. Otto u16: "9103a18c add" x12, x12, #0xe8 seguito
 * da "782a7994 strh" w20,[x12,x10,lsl #1] (passo 2) e dalle otto letture
 * ldrh [x12], [x12,#2] ... [x12,#14]. E' la finestra scorrevole dei campioni.
 */
static u16 ltr2568_ps_buf[8];

/* 0xffffff8009cba0f8, "b900f928 str" w8,[x9,#248] in ltr2568_i2c_probe. */
static int ltr2568_meas_rate;

/*
 * 0xffffff8009cba0fc. Int, letto e riscritto SOLO da ltr2568_als_read
 * ("b940fd15 ldr" w21,[x8,#252] e "b900fd15 str" w21,[x8,#252]). E' l'ultimo
 * valore di luce restituito: sulle vie d'errore la funzione ritorna questo
 * invece di un valore nuovo.
 */
static int ltr2568_als_last;

/* 0xffffff8009cba100, "3904011f strb" wzr,[x8,#256] in ltr2568_als_read. */
static bool ltr2568_als_first;

/*
 * 0xffffff8009cba104 e 0x0108. Due int, letti e scritti solo da
 * ltr2568_als_read ("b941054b ldr" w11,[x10,#260] / "b9010549 str"
 * w9,[x10,#260] ; "b941096b ldr" w11,[x11,#264] / "b90109ab str"
 * w11,[x13,#264]). Sono i valori filtrati precedenti dei due canali ALS --
 * 0x104 per la coppia di registri 0x8b/0x8c, 0x108 per 0x89/0x8a -- cioe' lo
 * stato del filtro di assestamento.
 */
static int ltr2568_als_prev1;
static int ltr2568_als_prev0;

/* 0xffffff8009cba10c / 0x110 / 0x114, usati da ltr2568_ps_set_thres. */
static int ltr2568_thres_high;
static int ltr2568_thres_low;
static bool ltr2568_thres_override;

/*
 * 0xffffff800992e668, valore iniziale 12 (letto nel .data di fabbrica:
 * "0c 00 00 00" a 0xffffff800992e668). Letto con `ldrb` da ltr2568_sensor_init
 * ("3959a108 ldrb" w8,[x8,#1640]).
 *
 * SI CHIAMAVA `ltr2568_ps_gain`, ed era un nome sbagliato -- scritto quando le
 * quattro funzioni grandi erano fuori. Non e' il guadagno del PS: e' il CAMPO
 * DI GUADAGNO ALS del registro 0x80, gia' spostato di due bit. Lo dice
 * ltr2568_als_read, che e' il solo posto che lo SCRIVE:
 *   "531e773a lsl" w26, w25, #2      (w25 = indice di guadagno)
 *   "b906693a str" w26, [x9,#1640]   (a 0xffffff8008785e48)
 * e i tre punti che lo LEGGONO lo mettono in bit 2..3 del registro 0x80
 * ("2a090108 orr" w8, w8, w9 con w9 = 0x21 ; "321b0108 orr" w8, w8, #0x20).
 * La controprova aritmetica: 12 = 3 << 2, cioe' indice 3, e la tabella dei
 * guadagni a 0xffffff80091be848 ha [3] = 64 -- esattamente il valore iniziale
 * di ltr2568_als_gainrange qui sotto. I due valori iniziali concordano solo
 * con questa lettura.
 */
static int ltr2568_als_gain = 12;

/*
 * 0xffffff800992e66c, valore iniziale 400 (nel .data di fabbrica:
 * "90 01 00 00"). Int: "b9466d09 ldr" w9,[x8,#1644] e "b9066d17 str"
 * w23,[x8,#1644]. E' l'offset di prossimita' scritto nei registri 0x9e/0x9f:
 * ps_enable_nodata lo ricalcola dalla media dei cinque campioni
 * ("5104b2b7 sub" w23, w21, #0x12c) e ltr2568_ps_read lo abbassa di 300 a
 * ogni quarta lettura nulla ("7104b129 subs" w9, w9, #0x12c ;
 * "1a9fc135 csel" w21, w9, wzr, gt).
 */
static int ltr2568_ps_offset = 400;

/*
 * 0xffffff800992e690, valore iniziale 64 ("40 00 00 00"). E' il terzo argomento
 * della printk "ALS sensor gainrange %d!" ("b9469103 ldr" w3,[x8,#1680]).
 *
 * ltr2568_als_read ne mostra il ruolo pieno: e' il guadagno ALS in unita'
 * naturali (1, 4, 16, 64), sempre riletto dalla tabella
 * ("b90692c8 str" w8,[x22,#1680]) e usato come divisore nella conversione in
 * lux ("1aca0d29 sdiv" w9, w9, w10).
 */
static int ltr2568_als_gainrange = 64;

/*
 * Tabella const a 0xffffff80091be848, QUATTRO voci, verificata byte per byte
 * contro .rodata ("01 00 00 00 04 00 00 00 10 00 00 00 40 00 00 00"): il
 * letterale successivo comincia a 0xffffff80091be858, quindi la corsa e' lunga
 * 16 byte e non 32.
 *
 * ltr2568_als_read la indicizza in tre punti, e in DUE dei tre l'indice puo'
 * uscire dai limiti. E' un difetto di fabbrica, deterministico, e si riproduce
 * di proposito -- non si "corregge":
 *
 *  - "d3431528 ubfx" x8, x9, #3, #3 estrae TRE bit (0..7) dal registro 0x88 e
 *    li usa come indice: per 4..7 la lettura cade oltre la tabella. La stessa
 *    maschera a tre bit e' giusta in ltr2568_init_client, dove indicizza
 *    ltr2568_als_meas_rate, che di voci ne ha otto -- il difetto ha la forma di
 *    una maschera copiata fra due tabelle di lunghezza diversa. Se
 *    l'hardware possa davvero alzare il bit 5 del registro 0x88 NON e'
 *    deducibile dal binario, e non lo affermo.
 *  - "b875dae9 ldr" w9, [x23, w21, sxtw #2] con w21 = -2
 *    ("321f7bf5 orr" w21, wzr, #0xfffffffe): la lettura cade a
 *    0xffffff80091be840, che contiene "78 00 00 00 8c 00 00 00", cioe' le
 *    ultime due voci di ltr2568_als_meas_rate. Il valore che ne esce e' 120, ed
 *    e' verificabile invece che solo dichiarato.
 */
static const int ltr2568_als_gain_table[4] = {
	1, 4, 16, 64,
};

static int ltr2568_local_init(void);
static int ltr2568_remove(void);

static struct alsps_init_info ltr2568_init_info = {	/* 0xffffff800992e550 */
	.name = LTR2568_DEV_NAME,			/* reloc +0x00 -> 0xffffff80091bdfb7 */
	.init = ltr2568_local_init,			/* reloc +0x08 */
	.uninit = ltr2568_remove,			/* reloc +0x10 */
};

static const struct i2c_device_id ltr2568_i2c_id[] = {	/* 0xffffff8008f53aa0 */
	{LTR2568_DEV_NAME, 0},
	{},
};

static const struct of_device_id alsps_of_match[] = {	/* 0xffffff8008f53858 */
	{.compatible = "mediatek,alsps"},	/* "mediatek,alsps"@0xffffff8008f53898 */
	{},
};

static int ltr2568_i2c_probe(struct i2c_client *client,
			     const struct i2c_device_id *id);
static int ltr2568_i2c_remove(struct i2c_client *client);
static int ltr2568_i2c_detect(struct i2c_client *client,
			      struct i2c_board_info *info);
static int ltr2568_i2c_suspend(struct device *dev);
static int ltr2568_i2c_resume(struct device *dev);

/*
 * 0xffffff8008f539e8: sei puntatori a +0x10..+0x38 (suspend, resume, freeze,
 * thaw, poweroff, restore), alternati fra ltr2568_i2c_suspend e
 * ltr2568_i2c_resume, e nient'altro. E' esattamente SET_SYSTEM_SLEEP_PM_OPS.
 */
static const struct dev_pm_ops ltr2568_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(ltr2568_i2c_suspend, ltr2568_i2c_resume)
};

static struct i2c_driver ltr2568_i2c_driver = {		/* 0xffffff800992e570 */
	.probe = ltr2568_i2c_probe,			/* reloc +0x10 */
	.remove = ltr2568_i2c_remove,			/* reloc +0x18 */
	.detect = ltr2568_i2c_detect,			/* reloc +0xc0 */
	.id_table = ltr2568_i2c_id,			/* reloc +0xb8 -> 0xffffff8008f53aa0 */
	.driver = {					/* 0xffffff800992e5b0 */
		.name = LTR2568_DEV_NAME,		/* reloc +0x00 */
		.of_match_table = alsps_of_match,	/* reloc +0x28 -> 0xffffff8008f53858 */
		.pm = &ltr2568_pm_ops,			/* reloc +0x68 -> 0xffffff8008f539e8 */
	},
};

/*
 * Tutte e quattro `static`, come di fabbrica (`t` in stock.map). Le
 * dichiarazioni anticipate servono perche' gli attributi di debug e le ops del
 * framework, che stanno prima nel file, le chiamano.
 */
static int ltr2568_ps_read(struct i2c_client *client, u16 *data);
static int ltr2568_als_read(struct i2c_client *client, u16 *data);
static int ps_enable_nodata(int en);
static void ltr2568_eint_work(struct work_struct *work);

/* ------------------------------------------------------------------ i2c -- */

/*
 * Riga 303. Questo simbolo di fabbrica si chiama `ltr2568_master_send`: e' il
 * nome che l'oracolo ha ricostruito. `ltr2568_i2c_write_block` -- il `__func__`
 * a 0xffffff80091be1bd, argomento x1 della printk a 0xffffff80087853d8 -- e'
 * una funzione DIVERSA, senza simbolo perche' di fabbrica e' incorporata qui
 * dentro: e' anche una pista sui -20 byte residui (vedi il cappello, D9).
 *
 * La lettura volatile scartata di obj->trace ("b9405d1f ldr" wzr,[x8,#92]) e'
 * un `atomic_read()`, cioe' un `READ_ONCE()`: il compilatore deve emetterla
 * anche quando il valore non serve. Sta PRIMA della lettura di i2c_retry
 * ("b940351a ldr" w26,[x8,#52] a 0xffffff8008785348), e per questo la
 * dichiarazione di max_try qui sotto e' separata dall'assegnamento: l'ordine
 * delle due letture e' quello del disassemblato, non una scelta di stile.
 */
static int ltr2568_master_send(struct i2c_client *client, u8 addr, u8 *data)
{
	int err = 0;
	int i;
	int max_try;
	u8 buf[2];

	atomic_read(&ltr2568_obj->trace);	/* "b9405d1f ldr" wzr,[x8,#92] */
	max_try = atomic_read(&ltr2568_obj->i2c_retry);

	for (i = 0; i < max_try; i++) {
		mutex_lock(&ltr2568_mutex);
		if (!client) {
			mutex_unlock(&ltr2568_mutex);
			err = -EINVAL;		/* "128002b9 mov" w25, #0xffffffea */
		} else {
			buf[0] = addr;
			buf[1] = data[0];
			err = i2c_master_send(client, buf, 2);
			if (err < 0) {
				/* "\x013[ALS/PS] %s %d : send command error!!\n"@0xffffff80091be1d5 */
#line 303
				APS_ERR("send command error!!\n");
				mutex_unlock(&ltr2568_mutex);
				err = -EFAULT;	/* "128001b9 mov" w25, #0xfffffff2 */
			} else {
				mutex_unlock(&ltr2568_mutex);
			}
		}
		if (err == 0)
			break;
		/*
		 * "5291b780 mov" w0, #0x8dbc con "72a000c0 movk" w0, #0x6, lsl #16
		 * -> __const_udelay(0x68dbc), cioe' udelay(100).
		 */
		udelay(100);
	}

	if (err == 0)
		err = 1;		/* "320003f9 orr" w25, wzr, #0x1 */

	return err;
}

/*
 * Nessuna printk, quindi nessun numero di riga. Usa i2c_transfer con due
 * messaggi ("321f03e2 orr" w2, wzr, #0x2): scrittura di un byte di registro
 * piu' lettura di `len` byte.
 *
 * Stessa lettura volatile scartata di obj->trace di ltr2568_master_send
 * ("b9405d1f ldr" wzr,[x8,#92] a 0xffffff8008784ffc), di nuovo prima della
 * lettura di i2c_retry ("b9403519 ldr" w25,[x8,#52]).
 */
static int ltr2568_master_recv(struct i2c_client *client, u8 addr, u8 *data,
			       u8 len)
{
	int err = 0;
	int i;
	int max_try;
	/*
	 * NON inizializzata qui: il ciclo le riassegna `addr` a ogni giro, e la
	 * fabbrica la scrive SOLO li'. Con `= addr` in dichiarazione clang emette
	 * una "strb w1, [sp,#4]" prima del ciclo che nel binario non c'e'.
	 */
	u8 beg;
	struct i2c_msg msgs[2];

	atomic_read(&ltr2568_obj->trace);	/* "b9405d1f ldr" wzr,[x8,#92] */
	max_try = atomic_read(&ltr2568_obj->i2c_retry);

	if (max_try < 1)	/* "7100073f cmp" w25, #0x1 ; "540005eb b.lt" */
		return len;

	for (i = 0; i < max_try; i++) {
		beg = addr;
		msgs[0].addr = client->addr;
		msgs[0].flags = 0;
		msgs[0].len = 1;	/* "790037fc strh" w28,[sp,#26], w28 = 1 */
		msgs[0].buf = &beg;
		msgs[1].addr = client->addr;
		msgs[1].flags = I2C_M_RD;	/* "321003e9 orr" w9, wzr, #0x10000 */
		msgs[1].len = len;
		msgs[1].buf = data;

		mutex_lock(&ltr2568_mutex);
		if (!client || len >= 9) {	/* "7100251f cmp" w8, #0x9 ; "540000a3 b.cc" */
			mutex_unlock(&ltr2568_mutex);
			err = -EINVAL;		/* "128002b8 mov" w24, #0xffffffea */
		} else {
			err = i2c_transfer(client->adapter, msgs, 2);
			mutex_unlock(&ltr2568_mutex);
			if (err == 2)		/* "71000b1f cmp" w24, #0x2 */
				return len;
			err = -EIO;	/* "321d7bf8 orr" w24, wzr, #0xfffffffb */
		}
		udelay(100);	/* "5291b780 mov" w0, #0x8dbc + "72a000c0 movk" #0x6, lsl #16 */
	}

	return err;
}

/* --------------------------------------------------------------- soglie -- */

/*
 * Riga 775. Due forme, scelte dal flag a 0xffffff8009cba114: se e' 1 usa la
 * coppia di globali 0xffffff8009cba110 / 0xffffff8009cba10c, altrimenti
 * obj->ps_thd_val_low / obj->ps_thd_val_high.
 *
 * Registri: 0x9c/0x9d (soglia bassa, byte basso e alto) e 0x9a/0x9b (soglia
 * alta) -- "52801389 mov" w9, #0x9c ; "528013a9 mov" w9, #0x9d ;
 * "52801349 mov" w9, #0x9a ; "52801369 mov" w9, #0x9b.
 *
 * Qui la fabbrica NON passa da ltr2568_master_send: chiama direttamente
 * i2c_master_send con un buffer di 2 byte ("940c1710 bl" -> i2c_master_send).
 */
static int ltr2568_ps_set_thres(void)
{
	struct ltr2568_priv *obj = ltr2568_obj;
	struct i2c_client *client = obj->client;
	u8 databuf[2];
	int res;

	databuf[0] = 0x9c;
	if (ltr2568_thres_override) {
		databuf[1] = (u8)ltr2568_thres_low;
		res = i2c_master_send(client, databuf, 2);
		if (res <= 0)
			goto EXIT_ERR;
		databuf[0] = 0x9d;
		databuf[1] = (u8)(ltr2568_thres_low >> 8);
		res = i2c_master_send(client, databuf, 2);
		if (res <= 0)
			goto EXIT_ERR;
		databuf[0] = 0x9a;
		databuf[1] = (u8)ltr2568_thres_high;
		res = i2c_master_send(client, databuf, 2);
		if (res <= 0)
			goto EXIT_ERR;
		databuf[0] = 0x9b;
		databuf[1] = (u8)(ltr2568_thres_high >> 8);
		res = i2c_master_send(client, databuf, 2);
		if (res <= 0)
			goto EXIT_ERR;
		ltr2568_thres_override = false;	/* "3904529f strb" wzr,[x20,#276] */
		return 0;
	}

	databuf[1] = (u8)atomic_read(&obj->ps_thd_val_low);
	res = i2c_master_send(client, databuf, 2);
	if (res <= 0)
		goto EXIT_ERR;
	databuf[0] = 0x9d;
	databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) >> 8);
	res = i2c_master_send(client, databuf, 2);
	if (res <= 0)
		goto EXIT_ERR;
	databuf[0] = 0x9a;
	databuf[1] = (u8)atomic_read(&obj->ps_thd_val_high);
	res = i2c_master_send(client, databuf, 2);
	if (res <= 0)
		goto EXIT_ERR;
	databuf[0] = 0x9b;
	databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) >> 8);
	res = i2c_master_send(client, databuf, 2);
	if (res <= 0)
		goto EXIT_ERR;
	return 0;

EXIT_ERR:
	/* "\x013[ALS/PS] %s %d : set thres: %d\n"@0xffffff80091be23d */
#line 775
	APS_ERR("set thres: %d\n", res);
	return -1;
}

/* -------------------------------------------------------- accensione ALS -- */

/*
 * Riga 1116. Legge il registro 0x80, forza il bit 0 a 1 o a 0 e lo riscrive;
 * poi mdelay(10). Il flag a 0xffffff8009cba0cc dice se l'ALS e' gia' nello
 * stato richiesto, e in quel caso la funzione non tocca il bus.
 */
static int ltr2568_als_enable(struct i2c_client *client, int enable)
{
	u8 databuf[1];
	int res;

	databuf[0] = 0;			/* "390013ff strb" wzr,[sp,#4] */
	if (enable && ltr2568_als_on)
		return 0;
	if (!enable && !ltr2568_als_on)
		return 0;

	/* registro 0x80: "321903e1 orr" w1, wzr, #0x80 */
	ltr2568_master_recv(client, 0x80, databuf, 1);
	if (enable) {
		databuf[0] |= 0x01;	/* "32000108 orr" w8, w8, #0x1 */
		ltr2568_als_on = true;
		ltr2568_als_first = false;
	} else {
		databuf[0] &= 0xfe;	/* "121f1908 and" w8, w8, #0xfe */
		ltr2568_als_on = false;
	}
	res = ltr2568_master_send(client, 0x80, databuf);
	if (res < 0) {
		/* "\x013[ALS/PS] %s %d : ALS: enable als err: %d en: %d \n"@0xffffff80091be64a */
#line 1116
		APS_ERR("ALS: enable als err: %d en: %d \n", res, enable);
		return res;
	}
	/* dieci __const_udelay(0x418958) consecutivi = mdelay(10) */
	mdelay(10);
	return 0;
}

/*
 * Riga 1295/1320, incorporata in als_get_data. Il nome viene dal `__func__` a
 * 0xffffff80091be6bf.
 *
 * NOTA: sulla via buona la fabbrica NON ritorna obj->hw->als_value[idx] ma il
 * valore grezzo `als` -- alla fine di als_get_data il registro che finisce in
 * *value e' lo stesso `and w21, w0, #0xffff` calcolato subito dopo
 * ltr2568_als_read, e nessuna istruzione ricarica als_value[idx] su quella
 * branca. als_value[idx] compare solo come argomento della printk di errore
 * ("b9405904 ldr" w4,[x8,#88]).
 */
static int ltr2568_get_als_value(struct ltr2568_priv *obj, u16 als)
{
	int idx;
	int invalid = 0;

	for (idx = 0; idx < obj->als_level_num; idx++) {
		if (als < obj->hw->als_level[idx])
			break;
	}
	if (idx >= obj->als_value_num) {
		/* "\x013[ALS/PS] %s %d : exceed range\n"@0xffffff80091be69e */
#line 1295
		APS_ERR("exceed range\n");
		idx = obj->als_value_num - 1;
	}

	if (atomic_read(&obj->als_deb_on) == 1) {
		unsigned long endt = atomic_read(&obj->als_deb_end);

		if (time_after(jiffies, endt))
			atomic_set(&obj->als_deb_on, 0);

		if (atomic_read(&obj->als_deb_on) == 1)
			invalid = 1;
	}

	if (!invalid)
		return als;

	/* "\x013[ALS/PS] %s %d : ALS: %05d => %05d (-1)\n"@0xffffff80091be6d5 */
#line 1320
	APS_ERR("ALS: %05d => %05d (-1)\n", als, obj->hw->als_value[idx]);
	return -1;
}

/* ----------------------------------------------------- attributi di debug -- */

/* Riga 1335. "(%d %d %d %d %d)\n"@0xffffff80091be4e6 */
static ssize_t ltr2568_show_config(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	if (!ltr2568_obj) {
		/* "\x013[ALS/PS] %s %d : ltr2568_obj is null!!\n"@0xffffff80091be1fe */
#line 1335
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	res = snprintf(buf, PAGE_SIZE, "(%d %d %d %d %d)\n",
		       atomic_read(&ltr2568_obj->i2c_retry),
		       atomic_read(&ltr2568_obj->als_debounce),
		       atomic_read(&ltr2568_obj->ps_mask),
		       ltr2568_obj->ps_threshold,
		       atomic_read(&ltr2568_obj->ps_debounce));
	return res;
}

/* Riga 1350/1364. "%d %d %d %d %d"@0xffffff800921596c */
static ssize_t ltr2568_store_config(struct device_driver *ddri, const char *buf,
				    size_t count)
{
	int retry, als_deb, mask, thres, ps_deb;

	if (!ltr2568_obj) {
#line 1350
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	if (sscanf(buf, "%d %d %d %d %d", &retry, &als_deb, &mask, &thres,
		   &ps_deb) == 5) {
		atomic_set(&ltr2568_obj->i2c_retry, retry);
		atomic_set(&ltr2568_obj->als_debounce, als_deb);
		atomic_set(&ltr2568_obj->ps_mask, mask);
		ltr2568_obj->ps_threshold = thres;
		atomic_set(&ltr2568_obj->ps_debounce, ps_deb);
	} else {
		/* "\x013[ALS/PS] %s %d : invalid content: '%s', length = %zu\n"@0xffffff80091be49a */
#line 1364
		APS_ERR("invalid content: '%s', length = %zu\n", buf, count);
	}

	return count;
}

/* Riga 1374. "0x%04X\n"@0xffffff80091be5fe */
static ssize_t ltr2568_show_trace(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	if (!ltr2568_obj) {
#line 1374
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}
	res = snprintf(buf, PAGE_SIZE, "0x%04X\n",
		       atomic_read(&ltr2568_obj->trace));
	return res;
}

/* Riga 1387/1397. "0x%x"@0xffffff8009142911 */
static ssize_t ltr2568_store_trace(struct device_driver *ddri, const char *buf,
				   size_t count)
{
	int trace;

	if (!ltr2568_obj) {
#line 1387
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	if (sscanf(buf, "0x%x", &trace) == 1)
		atomic_set(&ltr2568_obj->trace, trace);
	else
#line 1397
		APS_ERR("invalid content: '%s', length = %zu\n", buf, count);

	return count;
}

/* Riga 1408. "0x%04X(%d)\n"@0xffffff80091be457 */
static ssize_t ltr2568_show_als(struct device_driver *ddri, char *buf)
{
	int res;

	if (!ltr2568_obj) {
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	res = ltr2568_als_read(ltr2568_obj->client, &ltr2568_obj->als);
	return snprintf(buf, PAGE_SIZE, "0x%04X(%d)\n", res, res);
}

/* Riga 1421. Stesso formato di show_als. */
static ssize_t ltr2568_show_ps(struct device_driver *ddri, char *buf)
{
	int res;

	if (!ltr2568_obj) {
#line 1421
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	res = ltr2568_ps_read(ltr2568_obj->client, &ltr2568_obj->ps);
	return snprintf(buf, PAGE_SIZE, "0x%04X(%d)\n", res, res);
}

/* "aa1f03e0 mov" x0, xzr ; "d65f03c0 ret" -- otto byte. */
static ssize_t ltr2568_show_send(struct device_driver *ddri, char *buf)
{
	return 0;
}

/* Riga 1483/1488. "%x %x"@0xffffff80091411fa */
static ssize_t ltr2568_store_send(struct device_driver *ddri, const char *buf,
				  size_t count)
{
	int addr, cmd;

	if (!ltr2568_obj) {
#line 1483
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	} else if (sscanf(buf, "%x %x", &addr, &cmd) != 2) {
		/* "\x013[ALS/PS] %s %d : invalid format: '%s'\n"@0xffffff80091be534 */
#line 1488
		APS_ERR("invalid format: '%s'\n", buf);
		return 0;
	}

	return count;
}

/* "aa1f03e0 mov" x0, xzr ; "d65f03c0 ret" -- otto byte. */
static ssize_t ltr2568_show_recv(struct device_driver *ddri, char *buf)
{
	return 0;
}

/* Riga 1508/1513. "%x"@0xffffff800926f8f8 */
static ssize_t ltr2568_store_recv(struct device_driver *ddri, const char *buf,
				  size_t count)
{
	int addr;

	if (!ltr2568_obj) {
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	} else if (sscanf(buf, "%x", &addr) != 1) {
#line 1513
		APS_ERR("invalid format: '%s'\n", buf);
		return 0;
	}

	return count;
}

/*
 * Riga 1527. "CUST: %d, (%d %d)\n"@0xffffff80091be59a,
 * "CUST: NULL\n"@0xffffff80091be5ad (che il compilatore riduce a due store
 * immediati, "5289898a mov" w10, #0x4c4c e "52800174 mov" w20, #0xb),
 * "MISC: %d %d\n"@0xffffff80091be5b9.
 */
static ssize_t ltr2568_show_status(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;

	if (!ltr2568_obj) {
#line 1527
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	if (ltr2568_obj->hw) {
		len += snprintf(buf + len, PAGE_SIZE - len, "CUST: %d, (%d %d)\n",
				ltr2568_obj->hw->i2c_num,
				ltr2568_obj->hw->power_id,
				ltr2568_obj->hw->power_vol);
	} else {
		len += snprintf(buf + len, PAGE_SIZE - len, "CUST: NULL\n");
	}

	len += snprintf(buf + len, PAGE_SIZE - len, "MISC: %d %d\n",
			atomic_read(&ltr2568_obj->als_suspend),
			atomic_read(&ltr2568_obj->ps_suspend));
	return len;
}

/*
 * Incorporata in store_alslv e store_alsval. I due separatori sono
 * "7100811f cmp" w8, #0x20 (spazio) e "7100291f cmp" w8, #0xa (a capo);
 * il formato e' "%d"@0xffffff8009216030.
 */
#define IS_SPACE(CH) (((CH) == ' ') || ((CH) == '\n'))
static int read_int_from_buf(struct ltr2568_priv *obj, const char *buf,
			     size_t count, u32 data[], int len)
{
	int idx = 0;
	char *cur = (char *)buf, *end = (char *)(buf + count);

	while (idx < len) {
		while ((cur < end) && IS_SPACE(*cur))
			cur++;

		if (sscanf(cur, "%d", &data[idx]) != 1)
			break;

		idx++;
		while ((cur < end) && !IS_SPACE(*cur))
			cur++;
	}

	return idx;
}

/* Riga 1582. "%d "@0xffffff8009295b89 e "\n"@0xffffff8009245994 */
static ssize_t ltr2568_show_alslv(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	int idx;

	if (!ltr2568_obj) {
#line 1582
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	for (idx = 0; idx < ltr2568_obj->als_level_num; idx++)
		len += snprintf(buf + len, PAGE_SIZE - len, "%d ",
				ltr2568_obj->hw->als_level[idx]);

	len += snprintf(buf + len, PAGE_SIZE - len, "\n");
	return len;
}

/* Riga 1598/1608. "def"@0xffffff800920c7c5 */
static ssize_t ltr2568_store_alslv(struct device_driver *ddri, const char *buf,
				   size_t count)
{
	if (!ltr2568_obj) {
#line 1598
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	} else if (!strcmp(buf, "def"))
		memcpy(ltr2568_obj->als_level, ltr2568_obj->hw->als_level,
		       sizeof(ltr2568_obj->als_level));
	else if (ltr2568_obj->als_level_num !=
		 read_int_from_buf(ltr2568_obj, buf, count,
				   ltr2568_obj->hw->als_level,
				   ltr2568_obj->als_level_num))
#line 1608
		APS_ERR("invalid format: '%s'\n", buf);

	return count;
}

/* Riga 1619. */
static ssize_t ltr2568_show_alsval(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	int idx;

	if (!ltr2568_obj) {
#line 1619
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	}

	for (idx = 0; idx < ltr2568_obj->als_value_num; idx++)
		len += snprintf(buf + len, PAGE_SIZE - len, "%d ",
				ltr2568_obj->hw->als_value[idx]);

	len += snprintf(buf + len, PAGE_SIZE - len, "\n");
	return len;
}

/* Riga 1635/1645. */
static ssize_t ltr2568_store_alsval(struct device_driver *ddri, const char *buf,
				    size_t count)
{
	if (!ltr2568_obj) {
#line 1635
		APS_ERR("ltr2568_obj is null!!\n");
		return 0;
	} else if (!strcmp(buf, "def"))
		memcpy(ltr2568_obj->als_value, ltr2568_obj->hw->als_value,
		       sizeof(ltr2568_obj->als_value));
	else if (ltr2568_obj->als_value_num !=
		 read_int_from_buf(ltr2568_obj, buf, count,
				   ltr2568_obj->hw->als_value,
				   ltr2568_obj->als_value_num))
#line 1645
		APS_ERR("invalid format: '%s'\n", buf);

	return count;
}

/*
 * Legge i 34 registri di ltr2568_reg_dump uno per uno.
 * "reg:0x%04X value: 0x%04X\n"@0xffffff80091be5ec
 */
static ssize_t ltr2568_show_reg(struct device_driver *ddri, char *buf)
{
	int i, len = 0;
	u8 val;

	for (i = 0; i < 34; i++) {
		ltr2568_master_recv(ltr2568_obj->client, ltr2568_reg_dump[i],
				    &val, 1);
		len += snprintf(buf + len, PAGE_SIZE - len,
				"reg:0x%04X value: 0x%04X\n",
				ltr2568_reg_dump[i], val);
	}

	return len;
}

/*
 * I dieci attributi, nell'ordine in cui stanno in .data a partire da
 * 0xffffff800992e718 (passo 0x20). Il modo e' 0644 per tutti e dieci:
 * il campo `mode` vale 0x1a4 in ogni voce.
 */
static DRIVER_ATTR(als,    0644, ltr2568_show_als,    NULL);
static DRIVER_ATTR(ps,     0644, ltr2568_show_ps,     NULL);
static DRIVER_ATTR(trace,  0644, ltr2568_show_trace,  ltr2568_store_trace);
static DRIVER_ATTR(config, 0644, ltr2568_show_config, ltr2568_store_config);
static DRIVER_ATTR(alslv,  0644, ltr2568_show_alslv,  ltr2568_store_alslv);
static DRIVER_ATTR(alsval, 0644, ltr2568_show_alsval, ltr2568_store_alsval);
static DRIVER_ATTR(status, 0644, ltr2568_show_status, NULL);
static DRIVER_ATTR(send,   0644, ltr2568_show_send,   ltr2568_store_send);
static DRIVER_ATTR(recv,   0644, ltr2568_show_recv,   ltr2568_store_recv);
static DRIVER_ATTR(reg,    0644, ltr2568_show_reg,    NULL);

static struct driver_attribute *ltr2568_attr_list[] = {
	&driver_attr_als,	/* 0xffffff800992e718, name -> 'als' */
	&driver_attr_ps,	/* 0xffffff800992e738, name -> 'ps' */
	&driver_attr_trace,	/* 0xffffff800992e758, name -> 'trace' */
	&driver_attr_config,	/* 0xffffff800992e778, name -> 'config' */
	&driver_attr_alslv,	/* 0xffffff800992e798, name -> 'alslv' */
	&driver_attr_alsval,	/* 0xffffff800992e7b8, name -> 'alsval' */
	&driver_attr_status,	/* 0xffffff800992e7d8, name -> 'status' */
	&driver_attr_send,	/* 0xffffff800992e7f8, name -> 'send' */
	&driver_attr_recv,	/* 0xffffff800992e818, name -> 'recv' */
	&driver_attr_reg,	/* 0xffffff800992e838, name -> 'reg' */
};

/*
 * Riga 1689 -- il nome viene dal `__func__` a 0xffffff80091be432. Di fabbrica
 * e' incorporata in ltr2568_i2c_probe, che srotola le dieci driver_create_file.
 * "\x013[ALS/PS] %s %d : driver_create_file (%s) = %d\n"@0xffffff80091be401
 */
static int ltr2568_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(ltr2568_attr_list);

	if (driver == NULL)
		return -EINVAL;

	for (idx = 0; idx < num; idx++) {
		err = driver_create_file(driver, ltr2568_attr_list[idx]);
		if (err) {
			APS_ERR("driver_create_file (%s) = %d\n",
#line 1689
				ltr2568_attr_list[idx]->attr.name, err);
			break;
		}
	}

	return err;
}

/*
 * Incorporata in ltr2568_i2c_remove, che srotola le dieci driver_remove_file
 * senza controllarne l'esito.
 */
static int ltr2568_delete_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(ltr2568_attr_list);

	if (!driver)
		return -EINVAL;

	for (idx = 0; idx < num; idx++)
		driver_remove_file(driver, ltr2568_attr_list[idx]);

	return err;
}

/* ------------------------------------------------------------ interrupt -- */

/*
 * "b940b908 ldr" w8,[x8,#184] ; "7100091f cmp" w8, #0x2 -- il confronto con 2
 * (non con 1) e' quello che c'e'. La coda del lavoro passa da
 * queue_work_on(8, system_wq, &obj->eint_work): "321d03e0 orr" w0, wzr, #0x8.
 *
 * Il globale ltr2568_obj viene RILETTO dopo disable_irq_nosync(), e non messo
 * in una locale: "f9405a68 ldr" x8,[x19,#176] a 0xffffff8008783904, la chiamata,
 * e di nuovo "f9405a68 ldr" x8,[x19,#176] a 0xffffff8008783910 seguita da
 * "b40000c8 cbz" x8. Non e' solo una differenza di dimensione (88 contro 84):
 * una locale leggerebbe il puntatore UNA volta sola, mentre di fabbrica la
 * rilettura avviene dopo una chiamata che puo' dormire.
 */
static irqreturn_t ltr2568_eint_handler(int irq, void *desc)
{
	if (ltr2568_irq_enabled == 2) {
		disable_irq_nosync(ltr2568_obj->irq);
		if (ltr2568_obj)
			queue_work_on(8, system_wq, &ltr2568_obj->eint_work);
	}

	return IRQ_HANDLED;
}

/*
 * 544 byte, nessuna printk.
 *
 * `obj` viene da container_of: la fabbrica calcola "d1004274 sub" x20, x19,
 * #0x10 (eint_work sta a +0x10 nella struttura) e prende il client come
 * "f85f8000 ldur" x0,[x0,#-8], cioe' obj+8. Sono le due istruzioni che fissano
 * l'offset del campo, non un'ipotesi sul framework.
 *
 * La prima meta' e' la stessa macchina a soglie di ps_get_data, con lo stesso
 * ordine e le stesse costanti. La seconda riprogramma le soglie di interrupt:
 * quando l'oggetto e' lontano (ltr2568_intr_flag == 1) mette la soglia bassa a
 * obj->ps_thd_val_low e quella alta a 0xffff -- "529ff348 mov" w8, #0xff9a e
 * "529ff368 mov" w8, #0xff9b sono le due coppie <registro, 0xff> scritte in
 * un colpo solo; quando e' vicino mette la bassa a 1 e l'alta a
 * obj->ps_thd_val_high ("528013a8 mov" w8, #0x9d con "79000be8 strh" scrive
 * insieme il registro 0x9d e il valore 0).
 *
 * Come ltr2568_ps_set_thres, qui la fabbrica NON passa da ltr2568_master_send:
 * chiama i2c_master_send con due byte ("940c1baf bl" -> i2c_master_send).
 */
static void ltr2568_eint_work(struct work_struct *work)
{
	struct ltr2568_priv *obj =
		container_of(work, struct ltr2568_priv, eint_work);
	u8 databuf[2];
	int value;
	int res;

	obj->ps = ltr2568_ps_read(obj->client, &obj->ps);

	if (obj->ps > atomic_read(&obj->ps_thd_val_high)) {
		value = 0;
		ltr2568_intr_flag = 1;
	} else if (obj->ps < atomic_read(&obj->ps_thd_val_low)) {
		value = 1;
		ltr2568_intr_flag = 0;
	} else {
		value = 1;
	}

	if (atomic_read(&obj->ps_suspend)) {
		value = -1;	/* "12800013 mov" w19, #0xffffffff */
	} else if (atomic_read(&obj->ps_deb_on) == 1) {
		unsigned long endt = atomic_read(&obj->ps_deb_end);

		if (time_after(jiffies, endt))
			atomic_set(&obj->ps_deb_on, 0);

		if (atomic_read(&obj->ps_deb_on) == 1)
			value = -1;	/* "5a9f1273 csinv" w19, w19, wzr, ne */
	} else if (obj->als > 50000) {	/* "52986a09 mov" w9, #0xc350 */
		value = 1;
	}

	databuf[0] = 0x9c;		/* "52801389 mov" w9, #0x9c */
	if (ltr2568_intr_flag == 1) {
		databuf[1] = (u8)atomic_read(&obj->ps_thd_val_low);
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)		/* "7100041f cmp" w0, #0x1 ; "b.lt" */
			goto EXIT;
		databuf[0] = 0x9d;
		databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_low) >> 8);
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)
			goto EXIT;
		databuf[0] = 0x9a;
		databuf[1] = 0xff;
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)
			goto EXIT;
		databuf[0] = 0x9b;
		databuf[1] = 0xff;
	} else {
		databuf[1] = 0x01;
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)
			goto EXIT;
		databuf[0] = 0x9d;
		databuf[1] = 0x00;
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)
			goto EXIT;
		databuf[0] = 0x9a;	/* "52801348 mov" w8, #0x9a */
		databuf[1] = (u8)atomic_read(&obj->ps_thd_val_high);
		res = i2c_master_send(obj->client, databuf, 2);
		if (res <= 0)
			goto EXIT;
		databuf[0] = 0x9b;	/* "52801368 mov" w8, #0x9b */
		databuf[1] = (u8)(atomic_read(&obj->ps_thd_val_high) >> 8);
	}

	res = i2c_master_send(obj->client, databuf, 2);
	if (res <= 0)
		goto EXIT;

	ps_report_interrupt_data(value);

EXIT:
	enable_irq(obj->irq);
}

/*
 * Riga 1921/1927/1941/1946/1954. "pin_cfg"@0xffffff80091bdee5,
 * "debounce"@0xffffff800911984a, "p-sensor"@0xffffff80091bdf24,
 * "ALS-eint"@0xffffff80091bdf5d.
 */
int ltr2568_setup_eint(struct i2c_client *client)
{
	int ret;
	struct pinctrl *pinctrl;
	struct pinctrl_state *pins_cfg;
	u32 ints[2] = {0, 0};

	pinctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(pinctrl)) {
		ret = PTR_ERR(pinctrl);
		/* "\x013[ALS/PS] %s %d : Cannot find alsps pinctrl!\n"@0xffffff80091bdea3 */
#line 1921
		APS_ERR("Cannot find alsps pinctrl!\n");
	}
	pins_cfg = pinctrl_lookup_state(pinctrl, "pin_cfg");
	if (IS_ERR(pins_cfg)) {
		ret = PTR_ERR(pins_cfg);
		/* "\x013[ALS/PS] %s %d : Cannot find alsps pinctrl pin_cfg!\n"@0xffffff80091bdeed */
		APS_ERR("Cannot find alsps pinctrl pin_cfg!\n");
	}

	if (ltr2568_obj->irq_node) {
		of_property_read_u32_array(ltr2568_obj->irq_node, "debounce",
					   ints, ARRAY_SIZE(ints));
		gpio_request(ints[0], "p-sensor");
		gpio_set_debounce(ints[0], ints[1]);
		pinctrl_select_state(pinctrl, pins_cfg);
		ltr2568_obj->irq = irq_of_parse_and_map(ltr2568_obj->irq_node, 0);
		if (!ltr2568_obj->irq) {
			/* "\x013[ALS/PS] %s %d : irq_of_parse_and_map fail!!\n"@0xffffff80091bdf2d */
#line 1941
			APS_ERR("irq_of_parse_and_map fail!!\n");
			return -EINVAL;	/* "128002a0 mov" w0, #0xffffffea */
		}
		if (request_irq(ltr2568_obj->irq, ltr2568_eint_handler,
				IRQF_TRIGGER_NONE, "ALS-eint", NULL)) {
			/* "\x013[ALS/PS] %s %d : IRQ LINE NOT AVAILABLE!!\n"@0xffffff80091bdf66 */
#line 1946
			APS_ERR("IRQ LINE NOT AVAILABLE!!\n");
			return -EINVAL;
		}
		enable_irq_wake(ltr2568_obj->irq);
		ltr2568_irq_enabled = 1;	/* "320003f3 orr" w19, wzr, #0x1 */
	} else {
		/* "\x013[ALS/PS] %s %d : null irq node!!\n"@0xffffff80091bdf93 */
#line 1954
		APS_ERR("null irq node!!\n");
		return -EINVAL;
	}

	return 0;
}

/* ------------------------------------------------------- init del sensore -- */

/*
 * Riga 2060/2064/2068. Sequenza di scritture di registro, ognuna abortita al
 * primo errore. I registri e i valori vengono uno per uno dal disassemblato:
 *   0x7f <- 0x00   "32001be1 orr" w1, wzr, #0x7f
 *   0xb6 <- 0x00   "528016c1 mov" w1, #0xb6
 *   0xb7 <- 0x10   "528016e1 mov" w1, #0xb7 / "321c03e8 orr" w8, wzr, #0x10
 *   0xa4 <- 0x04   "52801481 mov" w1, #0xa4 / "321e03e8 orr" w8, wzr, #0x4
 *   0xad <- 0x18   "528015a1 mov" w1, #0xad / "321d07e8 orr" w8, wzr, #0x18
 *   0x83 <- 0xaf   "52801061 mov" w1, #0x83 / "528015f5 mov" w21, #0xaf
 *   0x82 <- 0xaf   "52801041 mov" w1, #0x82
 *   0x84 <- 0x03   "52801081 mov" w1, #0x84 / "320007e8 orr" w8, wzr, #0x3
 * poi, se hw->polling_mode_ps e' zero:
 *   0x98 <- 0x81   "52801301 mov" w1, #0x98 / "52801028 mov" w8, #0x81
 *   0x99 <- 0x10   "52801321 mov" w1, #0x99 / "321c03e8 orr" w8, wzr, #0x10
 * e in ogni caso:
 *   0x80 <- ltr2568_als_gain | 0x20  "321b0108 orr" w8, w8, #0x20
 *   0x85 <- 0xa5   "528010a1 mov" w1, #0x85 / "528014a8 mov" w8, #0xa5
 */
static int ltr2568_sensor_init(void)
{
	struct ltr2568_priv *obj = ltr2568_obj;
	struct i2c_client *client = obj->client;
	u8 databuf[1];
	int res;

	databuf[0] = 0x00;
	res = ltr2568_master_send(client, 0x7f, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0x00;
	res = ltr2568_master_send(client, 0xb6, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0x10;
	res = ltr2568_master_send(client, 0xb7, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0x04;
	res = ltr2568_master_send(client, 0xa4, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0x18;
	res = ltr2568_master_send(client, 0xad, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0xaf;
	res = ltr2568_master_send(client, 0x83, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0xaf;
	res = ltr2568_master_send(client, 0x82, databuf);
	if (res < 0)
		goto EXIT_ERR;
	databuf[0] = 0x03;
	res = ltr2568_master_send(client, 0x84, databuf);
	if (res < 0)
		goto EXIT_ERR;

	if (!obj->hw->polling_mode_ps) {
		databuf[0] = 0x81;
		res = ltr2568_master_send(client, 0x98, databuf);
		if (res < 0)
			goto EXIT_ERR;
		databuf[0] = 0x10;
		res = ltr2568_master_send(client, 0x99, databuf);
		if (res < 0)
			goto EXIT_ERR;
	}

	databuf[0] = ltr2568_als_gain | 0x20;
	ltr2568_master_send(client, 0x80, databuf);

	/* "\x013[ALS/PS] %s %d : ALS sensor gainrange %d!\n"@0xffffff80091be115 */
#line 2060
	APS_ERR("ALS sensor gainrange %d!\n", ltr2568_als_gainrange);

	databuf[0] = 0xa5;
	ltr2568_master_send(client, 0x85, databuf);

	/*
	 * Citazione su una riga sola, di proposito: spezzarla in due righe la
	 * rende invisibile a verificacitazioni.py, ed e' un difetto gia' visto.
	 * "\x013[ALS/PS] %s %d : ALS sensor integration & measurement rate: %d!\n"@0xffffff80091be156
	 * Il terzo argomento e' l'immediato "528000a3 mov" w3, #0x5.
	 */
#line 2064
	APS_ERR("ALS sensor integration & measurement rate: %d!\n", 5);
	return 0;

EXIT_ERR:
	/* "\x013[ALS/PS] %s %d : sensor init: %d\n"@0xffffff80091be199 */
#line 2068
	APS_ERR("sensor init: %d\n", res);
	return 1;	/* "320003e0 orr" w0, wzr, #0x1 */
}

/*
 * Riga 2105/2112 -- il nome viene dal `__func__` a 0xffffff80091be297. Di
 * fabbrica e' incorporata in ltr2568_i2c_probe.
 *
 * I due registri letti al centro sono 0xb4 (scritto due volte, con 0x1c e poi
 * 0x1d) e 0xb9 (letto): "52801681 mov" w1, #0xb4 ; "321e0be8 orr" w8, wzr,
 * #0x1c ; "528003a8 mov" w8, #0x1d ; "52801721 mov" w1, #0xb9.
 * Il byte letto indicizza ltr2568_als_meas_rate con i bit 3..5.
 */
static int ltr2568_init_client(void)
{
	struct i2c_client *client = ltr2568_obj->client;
	u8 databuf[1];
	int res;

	mdelay(200);	/* 200 giri di __const_udelay(0x418958), cioe' udelay(1000) */

	res = ltr2568_sensor_init();
	if (res)
		goto EXIT_ERR;

	databuf[0] = 0x1c;
	res = ltr2568_master_send(client, 0xb4, databuf);
	if (res < 0)
		goto SETUP;
	databuf[0] = 0x1d;
	res = ltr2568_master_send(client, 0xb4, databuf);
	if (res < 0)
		goto SETUP;
	res = ltr2568_master_recv(client, 0xb9, databuf, 1);
	if (res < 0)
		goto SETUP;
	ltr2568_meas_rate = ltr2568_als_meas_rate[(databuf[0] >> 3) & 0x7];

SETUP:
	res = ltr2568_setup_eint(client);
	if (res != 0) {
		/* "\x013[ALS/PS] %s %d : setup eint: %d\n"@0xffffff80091be274 */
#line 2105
		APS_ERR("setup eint: %d\n", res);
		goto EXIT_ERR;
	}
	return 0;

EXIT_ERR:
	/* "\x013[ALS/PS] %s %d : init dev: %d\n"@0xffffff80091be2ab */
	APS_ERR("init dev: %d\n", res);
	return 1;
}

/* --------------------------------------- prossimita': lettura e soglie -- */

/*
 * Riga 642, `__func__` "ltr2568_ps_get_thres"@0xffffff80091be228. Di fabbrica
 * NON ha un simbolo: e' incorporata in ltr2568_ps_read e in ps_enable_nodata,
 * e la prova che e' una funzione sola sono le DUE printk (0xffffff8008784f18 e
 * 0xffffff80087848a8) che portano lo stesso `__func__` e la stessa riga da
 * dentro due funzioni diverse -- la firma attesa di un'incorporazione in due
 * chiamanti (revisione-ltr2568.md §3.3).
 *
 * L'argomento e' un u16: dove il chiamante ha un int la fabbrica mette una
 * maschera esplicita ("72003eaa ands" w10, w21, #0xffff in ps_enable_nodata),
 * dove ha gia' un valore a 11 bit non la mette (ltr2568_ps_read).
 *
 * Le sei soglie e le sette costanti vengono tutte dal disassemblato, e tre
 * confronti consecutivi cadono sullo STESSO blocco -- e' scritto in tre rami
 * uguali qui sotto perche' e' cosi' che la fabbrica lo confronta, non per
 * ridondanza:
 *   "71018e9f cmp" w20, #0x63  ->  <100:   +100 / +50
 *   "71031e9f cmp" w20, #0xc7  ->  <200:   +150 / +60
 *   "7104ae9f cmp" w20, #0x12b ->  <300:   +150 / +60
 *   "71063e9f cmp" w20, #0x18f ->  <400:   +150 / +60
 *   "71095e9f cmp" w20, #0x257 ->  <600:   +180 / +90
 *   "710f9e9f cmp" w20, #0x3e7 -> <1000:   +300 / +180
 *   "5280c808 mov" w8, #0x640 / "5280af0a mov" w10, #0x578 -> 1600 / 1400
 */
static void ltr2568_ps_get_thres(u16 ps)
{
	struct ltr2568_priv *obj = ltr2568_obj;
	int high, low;

	if (!obj) {
		/* "\x013[ALS/PS] %s %d : ltr2568_obj is null!!\n"@0xffffff80091be1fe */
		APS_ERR("ltr2568_obj is null!!\n");
		return;
	}
	if (ps == 0)		/* "340004d4 cbz" w20, ... */
		return;

	if (ps < 100) {
		high = ps + 100;	/* "11019288 add" w8, w20, #0x64 */
		low = ps + 50;		/* "1100ca8a add" w10, w20, #0x32 */
	} else if (ps < 200) {
		high = ps + 150;	/* "11025a88 add" w8, w20, #0x96 */
		low = ps + 60;		/* "1100f28a add" w10, w20, #0x3c */
	} else if (ps < 300) {
		high = ps + 150;
		low = ps + 60;
	} else if (ps < 400) {
		high = ps + 150;
		low = ps + 60;
	} else if (ps < 600) {
		high = ps + 180;	/* "1102d288 add" w8, w20, #0xb4 */
		low = ps + 90;		/* "11016a8a add" w10, w20, #0x5a */
	} else if (ps < 1000) {
		high = ps + 300;	/* "1104b288 add" w8, w20, #0x12c */
		low = ps + 180;		/* "1102d28a add" w10, w20, #0xb4 */
	} else {
		high = 1600;
		low = 1400;
	}

	atomic_set(&obj->ps_thd_val_high, high);	/* "b9010128 str" w8,[x9,#256] */
	atomic_set(&obj->ps_thd_val_low, low);		/* "b901052a str" w10,[x9,#260] */
	ltr2568_thres_override = true;			/* "3904512b strb" w11,[x9,#276] */
	ltr2568_thres_low = low;			/* "b901118a str" w10,[x12,#272] */
	ltr2568_thres_high = high;			/* "b9010da8 str" w8,[x13,#268] */
}

/*
 * 1168 byte, nessuna printk propria (le due che si vedono nel disassemblato
 * sono di ltr2568_ps_get_thres incorporata).
 *
 * Tre cose che vale la pena sapere prima di leggerla:
 *
 *  - il primo blocco NON e' una lettura: e' un controllo di identita' del chip.
 *    Legge il registro 0x83 e, se non vale 0xaf ("7102bd1f cmp" w8, #0xaf),
 *    rifa' l'inizializzazione e riscrive i due registri di configurazione.
 *    In quel ramo `databuf` viene RILETTO dopo essere stato scritto solo nel
 *    ramo precedente ("121e0ea8 and" w8, w21, #0x3c usa il registro, non una
 *    rilettura dalla pila): di fabbrica il valore per il registro 0x81 e'
 *    derivato da quello appena costruito per lo 0x80, e se lo 0x80 non e'
 *    stato scritto quel valore e' indeterminato. E' quello che il binario fa;
 *    non lo "aggiusto".
 *  - il campione buono e' a 11 bit: "12000908 and" w8, w8, #0x7 sul byte alto
 *    piu' "33180914 bfi" w20, w8, #8, #3.
 *  - la finestra scorrevole di otto campioni sta tutta nei globali
 *    0xffffff8009cba0e0..0x0f7 documentati in testa al file.
 */
static int ltr2568_ps_read(struct i2c_client *client, u16 *data)
{
	u8 databuf[2];
	u8 cfgbuf[2];
	u8 offbuf[2];
	u8 status;
	u16 ps;
	u16 sum;
	u16 avg;
	u16 lo, hi;
	int idx;
	int i;
	int res;

	ltr2568_master_recv(client, 0x83, databuf, 1);
	if (databuf[0] != 0xaf) {
		ltr2568_sensor_init();
		if (ltr2568_als_on == 1) {
			/* "52800429 mov" w9, #0x21 ; "2a090115 orr" w21, w8, w9 */
			cfgbuf[0] = ltr2568_als_gain | 0x21;
			ltr2568_master_send(client, 0x80, cfgbuf);
		}
		if (ltr2568_ps_on == 1) {
			/* "121e0ea8 and" w8, w21, #0x3c ; "52800349 mov" w9, #0x1a */
			cfgbuf[0] = (cfgbuf[0] & 0x3c) | 0x1a;
			ltr2568_master_send(client, 0x81, cfgbuf);
		}
		mdelay(10);	/* dieci __const_udelay(0x418958) */
	}

	ltr2568_master_recv(client, 0x91, databuf, 1);
	status = databuf[0];
	ltr2568_master_recv(client, 0x92, databuf, 2);
	if (status & 0x04)	/* "371007f5 tbnz" w21, #2 */
		goto zero;

	ps = databuf[0] | ((databuf[1] & 0x07) << 8);

	if (ps == 0) {
		/*
		 * Quattro letture nulle di fila abbassano l'offset di 300 e
		 * riaprono la finestra di calibrazione.
		 */
		if (++ltr2568_ps_zero_cnt < 4)	/* "7100111f cmp" w8, #0x4 */
			goto zero;
		ltr2568_ps_zero_cnt = 0;
		if (ltr2568_ps_offset == 0)
			goto zero;

		ltr2568_ps_cali_on = true;
		ltr2568_ps_idx = 0;
		ltr2568_ps_full = false;
		ltr2568_ps_stable = 0;

		/* "7104b129 subs" w9, w9, #0x12c ; "1a9fc135 csel" w21, w9, wzr, gt */
		if (ltr2568_ps_offset - 300 > 0)
			ltr2568_ps_offset = ltr2568_ps_offset - 300;
		else
			ltr2568_ps_offset = 0;

		client = ltr2568_obj->client;	/* "f9400554 ldr" x20,[x10,#8] */
		offbuf[0] = (u8)ltr2568_ps_offset;
		res = ltr2568_master_send(client, 0x9e, offbuf);
		if (res >= 0) {
			offbuf[0] = (u8)(ltr2568_ps_offset >> 8);
			ltr2568_master_send(client, 0x9f, offbuf);
		}
		goto zero;
	}

	ltr2568_ps_zero_cnt = 0;
	if (ltr2568_ps_cali_on == 1) {
		if (ps >= atomic_read(&ltr2568_obj->ps_thd_val_low))
			goto out;
	} else {
		/* "51002929 sub" w9, w9, #0xa */
		if (ps >= ltr2568_ps_base - 10)
			goto out;
	}

	if (ltr2568_ps_idx >= 8)	/* "f100215f cmp" x10, #0x8 */
		ltr2568_ps_full = true;

	if (ltr2568_ps_full) {
		idx = ltr2568_ps_idx & 0x07;	/* "9240094a and" x10, x10, #0x7 */
		ltr2568_ps_buf[idx] = ps;

		sum = 0;
		for (i = 0; i < 8; i++)
			sum += ltr2568_ps_buf[i];
		/*
		 * "53033c63 ubfx" w3, w3, #3, #13: la somma e' troncata a
		 * sedici bit PRIMA della divisione, cioe' e' un u16 anche nel
		 * sorgente -- con un int il campo sarebbe stato piu' largo.
		 */
		avg = sum >> 3;

		/*
		 * "529ffecd mov" w13, #0xfff6 con "12003dad and" w13, w13,
		 * #0xffff: il limite basso e' calcolato a SEDICI bit e puo'
		 * quindi avvolgersi; quello alto no ("11002863 add" w3, w3,
		 * #0xa senza maschera, perche' avg sta in tredici bit).
		 */
		lo = avg - 10;
		hi = avg + 10;
		for (i = 0; i < 8; i++) {
			if (ltr2568_ps_buf[i] > lo && ltr2568_ps_buf[i] < hi)
				ltr2568_ps_stable++;
			else
				ltr2568_ps_stable = 0;
		}

		ltr2568_ps_idx = idx + 1;
		if (ltr2568_ps_stable < 7)	/* "71001d9f cmp" w12, #0x7 */
			goto out;
	} else {
		ltr2568_ps_buf[ltr2568_ps_idx] = ps;
		ltr2568_ps_stable = 0;
		ltr2568_ps_idx = ltr2568_ps_idx + 1;
		goto out;
	}

	ltr2568_ps_cali_on = false;	/* "3903611f strb" wzr,[x8,#216] */
	ltr2568_ps_base = ps;		/* "b900dd14 str" w20,[x8,#220] */
	ltr2568_ps_get_thres(ps);
	ltr2568_ps_set_thres();
	goto out;

zero:
	ps = 0;			/* "2a1f03f4 mov" w20, wzr */
out:
	*data = ps;		/* "79000274 strh" w20,[x19] */
	return ps;
}

/* ---------------------------------------------------- luce: assestamento -- */

/*
 * Il filtro di assestamento dei due canali ALS. Di fabbrica compare DUE volte
 * dentro ltr2568_als_read, con le stesse istruzioni negli stessi ordini
 * (0xffffff8008785b24..0x5c14 per un canale, 0xffffff8008785c3c..0x5e5c per
 * l'altro): e' una funzione incorporata in due punti, oppure lo stesso testo
 * scritto due volte -- non e' distinguibile, perche' dentro non c'e' nessuna
 * printk e quindi nessun `__func__` che la nomini.
 *
 * IL NOME E' MIO, non misurato: e' l'unica cosa qui dentro che non venga dal
 * disassemblato. Costanti e forma invece vengono tutte da li':
 *   "7107d1bf cmp" w13, #0x1f4       -> lo scarto oltre 500 non si filtra
 *   "531d71b0 lsl" w16, w13, #3 ; "4b0d020f sub" w15, w16, w13
 *   "110fa1ef add" w15, w15, #0x3e8  -> denominatore |d|*7 + 1000
 *   "528008cf mov" w15, #0x46        -> numeratore d*|d|*70
 *   "528cccee mov" w14, #0x6667 + "72acccce movk" w14, #0x6666, lsl #16
 *     con "9362fe10 asr" x16, x16, #34 -> divisione per 10
 *   "52800151 mov" w17, #0xa ; "1b11be0f msub" w15, w16, w17, w15 -> resto
 *   "710015ff cmp" w15, #0x5         -> arrotondamento a meta' per eccesso
 *   "71000dbf cmp" w13, #0x3         -> sotto 3 di scarto non si muove
 */
static int ltr2568_als_filter(int cur, int prev)
{
	int diff = cur - prev;
	int adiff = abs(diff);
	int step;
	int r;

	if (adiff > 500)
		return cur;

	step = diff * adiff * 70 / (adiff * 7 + 1000);
	/*
	 * I DUE RAMI SONO SCRITTI PER ESTESO, e non come `step / 10 + (step > 0
	 * ? 1 : -1)`, perche' e' quello che il disassemblato mostra: la
	 * divisione per dieci compare DUE VOLTE, una per ramo
	 * ("9b2e7d2e smull" x14, w9, w14 a 0xffffff8008785b8c e
	 * "9b2e7d29 smull" x9, w9, w14 a 0xffffff8008785bc8), e l'aggiustamento
	 * e' un "11000529 add" w9, w9, #0x1 e un "51000529 sub" w9, w9, #0x1 in
	 * due blocchi distinti raggiunti da salti. La forma col ternario e' una
	 * parafrasi e il nostro clang la converte in cneg+csel senza salti:
	 * -52 byte su ltr2568_als_read, cioe' proprio i due blocchi che qui
	 * tornano.
	 */
	if (abs(step) % 10 >= 5) {
		if (step > 0)
			r = step / 10 + 1;
		else
			r = step / 10 - 1;
	} else {
		r = step / 10;
	}

	if (abs(r) >= 1)
		return prev + r;
	if (adiff < 3)
		return cur;
	if (diff >= 1)		/* "7100059f cmp" w12, #0x1 */
		return prev + 2;
	return prev - 2;
}

/*
 * 1268 byte. Legge lo stato (0x88), i quattro byte dei due canali (0x89..0x8c),
 * assesta ciascun canale, converte in lux e sceglie il guadagno per la lettura
 * successiva.
 *
 * Sulle vie d'errore NON ritorna un codice: ritorna l'ULTIMO valore buono,
 * "b940fd15 ldr" w21,[x8,#252]. E' il motivo per cui ltr2568_als_last esiste.
 *
 * La conversione in lux, istruzione per istruzione:
 *   "1b0c7d4c mul" w12, w10, w12       gainrange * meas_rate
 *   "1107fd8e add" w14, w12, #0x1ff / "1a8cb1cc csel" / "13097d8c asr" w12, w12, #9
 *                                      divisione con segno per 512
 *   "6b0c0129 subs" w9, w9, w12        sottratta dal canale assestato
 *   "52871c0b mov" w11, #0x38e0        moltiplicazione per 14560
 *   "1aca0d29 sdiv" w9, w9, w10        divisione per il guadagno
 *   "5289ba6c mov" w12, #0x4dd3 + "72a20c4c movk" w12, #0x1062, lsl #16
 *     con "9366fd29 asr" x9, x9, #38   divisione per 1000
 *
 * I due cambi di guadagno sono due `switch` DIVERSI e vanno tenuti separati:
 * quello che scende (segnale troppo alto) manda 64->2, 16->1, 4->0, e su 1 non
 * fa niente; quello che sale manda 1->1, 4->2, 16->3, e su 64 non fa niente.
 * Il ramo `default` del primo vale -2 ("321f7bf5 orr" w21, wzr, #0xfffffffe),
 * quello del secondo 0 -- ed e' il -2 a produrre l'accesso fuori dai limiti
 * gia' di fabbrica documentato su ltr2568_als_gain_table.
 */
static int ltr2568_als_read(struct i2c_client *client, u16 *data)
{
	u8 databuf[4];
	u8 cfgbuf[2];
	int als0, als1;
	int val0, val1;
	int lux;
	int idx;
	int res;

	res = ltr2568_master_recv(client, 0x88, databuf, 1);
	if (res < 0)
		goto last;

	/* "d3431528 ubfx" x8, x9, #3, #3 -- tre bit su una tabella di quattro */
	ltr2568_als_gainrange = ltr2568_als_gain_table[(databuf[0] >> 3) & 0x07];

	if (databuf[0] & 0x40) {	/* "37300369 tbnz" w9, #6 */
		switch (ltr2568_als_gainrange) {
		case 64:
			idx = 2;
			break;
		case 16:
			idx = 1;
			break;
		case 4:
			idx = 0;
			break;
		case 1:
			goto last;
		default:
			idx = -2;
			break;
		}
		/* "531e76b8 lsl" w24, w21, #2 ; "52800428 mov" w8, #0x21 */
		cfgbuf[0] = (idx << 2) | 0x21;
		ltr2568_master_send(client, 0x80, cfgbuf);
		ltr2568_als_gain = idx << 2;
		ltr2568_als_first = false;
		ltr2568_als_gainrange = ltr2568_als_gain_table[idx];
		goto last;
	}

	res = ltr2568_master_recv(client, 0x89, databuf, 4);
	if (res < 0)
		goto last;
	/* "33181d15 bfi" w21, w8, #8, #8 e "33181d39 bfi" w25, w9, #8, #8 */
	als0 = databuf[0] | (databuf[1] << 8);
	als1 = databuf[2] | (databuf[3] << 8);

	if (ltr2568_als_gainrange == 1) {
		res = ltr2568_master_recv(client, 0x95, cfgbuf, 1);
		if (res < 0)
			cfgbuf[0] = 1;
		if (cfgbuf[0] < 1)	/* "1a9f8518 csinc" w24, w8, wzr, hi */
			cfgbuf[0] = 1;
		/* "0b183f28 add" w8, w25, w24, lsl #15 ; "51402119 sub" w25, w8, #0x8, lsl #12 */
		als1 = als1 + cfgbuf[0] * 32768 - 32768;
	}

	if (als1 == 0)		/* "1a9503e8 csel" w8, wzr, w21, eq */
		als0 = 0;

	if (ltr2568_als_first) {
		val1 = ltr2568_als_filter(als1, ltr2568_als_prev1);
		ltr2568_als_prev1 = val1;
		val0 = ltr2568_als_filter(als0, ltr2568_als_prev0);
	} else {
		ltr2568_als_first = true;
		ltr2568_als_prev1 = als1;
		val1 = als1;
		val0 = als0;
	}
	ltr2568_als_prev0 = val0;

	val1 -= (ltr2568_als_gainrange * ltr2568_meas_rate) / 512;
	if (val1 > 0)
		lux = (val1 * 14560 / ltr2568_als_gainrange) / 1000;
	else
		lux = 0;

	if (als1 < 50 || als0 < 50) {
		switch (ltr2568_als_gainrange) {
		case 1:
			idx = 1;
			break;
		case 4:
			idx = 2;
			break;
		case 16:
			idx = 3;
			break;
		case 64:
			goto out;
		default:
			idx = 0;
			break;
		}
		/* "331e0728 bfi" w8, w25, #2, #2 -- qui l'indice sta in due bit */
		cfgbuf[0] = (idx << 2) | 0x21;
		ltr2568_master_send(client, 0x80, cfgbuf);
		ltr2568_als_gain = idx << 2;
		ltr2568_als_first = false;
		ltr2568_als_gainrange = ltr2568_als_gain_table[idx];
	} else if (als1 > 50000 || als0 > 50000) {
		switch (ltr2568_als_gainrange) {
		case 64:
			idx = 2;
			break;
		case 16:
			idx = 1;
			break;
		case 4:
			idx = 0;
			break;
		case 1:
			goto out;
		default:
			idx = -2;
			break;
		}
		cfgbuf[0] = (idx << 2) | 0x21;
		ltr2568_master_send(client, 0x80, cfgbuf);
		ltr2568_als_gain = idx << 2;
		ltr2568_als_first = false;
		ltr2568_als_gainrange = ltr2568_als_gain_table[idx];
	}
	goto out;

last:
	lux = ltr2568_als_last;
out:
	*data = lux;			/* "79000275 strh" w21,[x19] */
	ltr2568_als_last = lux;		/* "b900fd15 str" w21,[x8,#252] */
	return lux;
}

/* --------------------------------------------- prossimita': accensione -- */

/*
 * Riga 833, `__func__` "ltr2568_ps_enable"@0xffffff80091be774. Di fabbrica non
 * ha un simbolo: e' incorporata in ps_enable_nodata, che e' il suo unico
 * chiamante.
 *
 * Le tre costanti del registro 0x81 sono tutte nel disassemblato, e le due che
 * non sono immediati logici validi arrivano da un registro -- il che conferma
 * che sono proprio 0x26 e 0x1a e non altro:
 *   "528004c9 mov" w9, #0x26 ; "0a090108 and" w8, w8, w9
 *   "321d0509 orr" w9, w8, #0x18
 *   ramo acceso:  "52800349 mov" w9, #0x1a ; "2a090108 orr" w8, w8, w9
 *   ramo spento:  "121e0d29 and" w9, w9, #0x3c
 */
static int ltr2568_ps_enable(struct i2c_client *client, int enable)
{
	u8 databuf[2];
	u8 gainbuf[2];
	int res;

	ltr2568_master_recv(client, 0x81, databuf, 1);
	databuf[0] = (databuf[0] & 0x26) | 0x18;

	if (enable) {
		databuf[0] |= 0x02;
		ltr2568_ps_on = true;
		ltr2568_ps_zero_cnt = 0;
		ltr2568_ps_cali_on = false;
	} else {
		databuf[0] &= 0x3c;
		ltr2568_ps_on = false;
		/* "321b0108 orr" w8, w8, #0x20 */
		gainbuf[0] = ltr2568_als_gain | 0x20;
		ltr2568_master_send(client, 0x80, gainbuf);
	}

	res = ltr2568_master_send(client, 0x81, databuf);
	if (res < 0)
		/* "\x013[ALS/PS] %s %d : PS: enable ps err: %d en: %d \n"@0xffffff80091be742 */
#line 833
		APS_ERR("PS: enable ps err: %d en: %d \n", res, enable);

	return res;
}

/* ------------------------------------------------ ops del framework alsps -- */

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int als_open_report_data(int open)
{
	return 0;
}

/* Riga 2133/2139. "als_enable_nodata"@0xffffff80091be606 */
static int als_enable_nodata(int en)
{
	int res = 0;

	if (!ltr2568_obj) {
		APS_ERR("ltr2568_obj is null!!\n");
		return -1;
	}

	res = ltr2568_als_enable(ltr2568_obj->client, en);
	if (res) {
		/* "\x013[ALS/PS] %s %d : als_enable_nodata is failed!!\n"@0xffffff80091be618 */
#line 2139
		APS_ERR("als_enable_nodata is failed!!\n");
		return -1;
	}

	mutex_lock(&ltr2568_mutex);
	if (en)
		set_bit(CMC_BIT_ALS, &ltr2568_obj->enable);
	else
		clear_bit(CMC_BIT_ALS, &ltr2568_obj->enable);
	mutex_unlock(&ltr2568_mutex);
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int als_set_delay(u64 ns)
{
	return 0;
}

/*
 * Otto byte: "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret". Di fabbrica als_batch NON
 * chiama als_set_delay (che pure esiste come simbolo separato): ritorna zero
 * direttamente.
 */
static int als_batch(int flag, int64_t samplingPeriodNs,
		     int64_t maxBatchReportLatencyNs)
{
	return 0;
}

static int als_flush(void)
{
	return als_flush_report();
}

/* Riga 2180. "als_get_data"@0xffffff80091be691 */
static int als_get_data(int *value, int *status)
{
	int err = 0;

	if (!ltr2568_obj) {
		APS_ERR("ltr2568_obj is null!!\n");
		return -1;
	}

	ltr2568_obj->als = ltr2568_als_read(ltr2568_obj->client,
					    &ltr2568_obj->als);
	*value = ltr2568_get_als_value(ltr2568_obj, ltr2568_obj->als);
	if (*value < 0)
		err = -1;
	/* "321f03e8 orr" w8, wzr, #0x2 -- SENSOR_STATUS_ACCURACY_MEDIUM */
	*status = SENSOR_STATUS_ACCURACY_MEDIUM;

	return err;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ps_open_report_data(int open)
{
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ps_set_delay(u64 ns)
{
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ps_batch(int flag, int64_t samplingPeriodNs,
		    int64_t maxBatchReportLatencyNs)
{
	return 0;
}

static int ps_flush(void)
{
	return ps_flush_report();
}

/*
 * 1164 byte, righe 2212 e 2218, "ps_enable_nodata"@0xffffff80091be700.
 *
 * Contiene incorporate ltr2568_ps_enable (riga 833) e ltr2568_ps_get_thres
 * (riga 642), ed e' anche l'unico posto dove la calibrazione di accensione
 * gira: cinque campioni a 30 ms di distanza, media, offset.
 *
 * Due dettagli che vengono dal binario e non da un modello:
 *  - il ciclo dei cinque campioni RIPETE l'iterazione fallita invece di
 *    contarla: "131f7c08 asr" w8, w0, #31 ; "0b0802a8 add" w8, w21, w8 ;
 *    "11000515 add" w21, w8, #0x1 -- cioe' l'indice avanza di 1 solo se la
 *    lettura e' andata bene. La somma usa "0a280009 bic" w9, w0, w8, che e'
 *    il valore letto oppure zero;
 *  - la soglia sulla media e' scritta come confronto sulla SOMMA:
 *    "7117869f cmp" w20, #0x5e1 e' 1505 = 301*5, cioe' `media > 300` che il
 *    compilatore ha riscritto. Qui e' scritto sulla media, che e' la forma di
 *    cui 1505 e' la conseguenza.
 */
static int ps_enable_nodata(int en)
{
	struct ltr2568_priv *obj = ltr2568_obj;
	struct i2c_client *client;
	u8 databuf[2];
	u8 offbuf[2];
	int res = 0;
	int sum = 0;
	int avg;
	int i;

	if (!obj) {
		/* "\x013[ALS/PS] %s %d : ltr2568_obj is null!!\n"@0xffffff80091be1fe */
#line 2212
		APS_ERR("ltr2568_obj is null!!\n");
		return -1;
	}
	client = obj->client;

	/*
	 * Se la PS e' gia' nello stato richiesto la fabbrica NON tocca il bus,
	 * ma il bit di abilitazione lo aggiorna lo stesso: i due salti
	 * ("37001c48 tbnz" w8, #0 e "37001be8 tbnz" w8, #0 dopo
	 * "52000108 eor" w8, w8, #0x1) puntano entrambi al blocco del mutex.
	 */
	if ((en && ltr2568_ps_on) || (!en && !ltr2568_ps_on))
		goto ENABLE_BIT;

	res = ltr2568_ps_enable(client, en);
	if (res < 0)
		goto EXIT_ERR;

	if (!en && ltr2568_als_on) {
		databuf[0] = ltr2568_als_gain | 0x21;
		ltr2568_master_send(client, 0x80, databuf);
	}
	mdelay(10);

	if (en && !ltr2568_obj->hw->polling_mode_ps) {
		client = ltr2568_obj->client;
		offbuf[0] = 0;
		res = ltr2568_master_send(client, 0x9e, offbuf);
		if (res >= 0) {
			offbuf[0] = 0;
			ltr2568_master_send(client, 0x9f, offbuf);
		}

		databuf[0] = 0x02;	/* "321f03e9 orr" w9, wzr, #0x2 */
		res = ltr2568_master_send(ltr2568_obj->client, 0x84, databuf);
		if (res < 0) {
			res = -1;
		} else {
			for (i = 0; i < 5; i++) {	/* "710016bf cmp" w21, #0x5 */
				msleep(30);	/* "321f0fe0 orr" w0, wzr, #0x1e */
				res = ltr2568_ps_read(ltr2568_obj->client,
						      &ltr2568_obj->ps);
				if (res < 0) {
					i--;
					continue;
				}
				sum += res;
			}

			avg = sum / 5;
			if (avg > 300) {
				/* "5104b2b7 sub" w23, w21, #0x12c */
				ltr2568_ps_offset = avg - 300;
				avg = 300;	/* "52802595 mov" w21, #0x12c */
			} else {
				ltr2568_ps_offset = 0;
			}
			ltr2568_ps_base = avg;

			client = ltr2568_obj->client;
			offbuf[0] = (u8)ltr2568_ps_offset;
			res = ltr2568_master_send(client, 0x9e, offbuf);
			if (res >= 0) {
				offbuf[0] = (u8)(ltr2568_ps_offset >> 8);
				ltr2568_master_send(client, 0x9f, offbuf);
			}

			ltr2568_ps_get_thres(avg);

			databuf[0] = 0x03;	/* "320007e9 orr" w9, wzr, #0x3 */
			res = ltr2568_master_send(ltr2568_obj->client, 0x84,
						  databuf);
			/* "131f7c14 asr" w20, w0, #31 */
			res = (res < 0) ? -1 : 0;
		}
		ltr2568_ps_set_thres();
	}

	/* "b940b909 ldr" w9,[x8,#184] ; "7100053f cmp" w9, #0x1 */
	if (en && ltr2568_irq_enabled == 1)
		ltr2568_irq_enabled = 2;

	if (res < 0)
		goto EXIT_ERR;

ENABLE_BIT:
	mutex_lock(&ltr2568_mutex);
	if (en)
		set_bit(CMC_BIT_PS, &ltr2568_obj->enable);
	else
		clear_bit(CMC_BIT_PS, &ltr2568_obj->enable);
	mutex_unlock(&ltr2568_mutex);
	return 0;

EXIT_ERR:
	/* "\x013[ALS/PS] %s %d : ps_enable_nodata is failed!!\n"@0xffffff80091be711 */
#line 2218
	APS_ERR("ps_enable_nodata is failed!!\n");
	return -1;
}

/*
 * Riga 2254. "ps_get_data"@0xffffff80091be786. La soglia di 50000 e'
 * "52986a0a mov" w10, #0xc350.
 */
static int ps_get_data(int *value, int *status)
{
	int err = 0;
	struct ltr2568_priv *obj = ltr2568_obj;

	if (!obj) {
#line 2254
		APS_ERR("ltr2568_obj is null!!\n");
		return -1;
	}

	obj->ps = ltr2568_ps_read(obj->client, &obj->ps);
	if (obj->ps > atomic_read(&obj->ps_thd_val_high)) {
		*value = 0;
		ltr2568_intr_flag = 1;
	} else if (obj->ps < atomic_read(&obj->ps_thd_val_low)) {
		*value = 1;
		ltr2568_intr_flag = 0;
	} else {
		*value = 1;
	}

	if (atomic_read(&obj->ps_suspend)) {
		*value = -1;
		err = -1;
	} else if (atomic_read(&obj->ps_deb_on) == 1) {
		unsigned long endt = atomic_read(&obj->ps_deb_end);

		if (time_after(jiffies, endt))
			atomic_set(&obj->ps_deb_on, 0);

		if (atomic_read(&obj->ps_deb_on) == 1) {
			*value = -1;
			err = -1;
		}
	} else if (obj->als > 50000) {
		*value = 1;
	}

	*status = SENSOR_STATUS_ACCURACY_MEDIUM;
	return err;
}

/* ------------------------------------------------- ops di fabbrica (factory) -- */

/*
 * Riga 2279. "\x013[ALS/PS] %s %d : %s:%s failed\n"@0xffffff80091be2cc,
 * "enable"@0xffffff800924755d, "disable"@0xffffff8009247611. La scelta fra i
 * due letterali e' "9a881124 csel" x4, x9, x8, ne.
 */
static int ltr2568_als_factory_enable_sensor(bool enable_disable,
					     int64_t sample_periods_ms)
{
	int err;

	err = als_enable_nodata(enable_disable ? 1 : 0);
	if (err) {
		APS_ERR("%s:%s failed\n", __func__,
#line 2279
			enable_disable ? "enable" : "disable");
		return -1;
	}
	return 0;
}

static int ltr2568_als_factory_get_data(int32_t *data)
{
	int status;

	return als_get_data(data, &status);
}

/* Riga 2301/2307. "\x013[ALS/PS] %s %d : obj is null!!\n"@0xffffff80091be30f */
static int ltr2568_als_factory_get_raw_data(int32_t *data)
{
	int err;

	if (!ltr2568_obj) {
#line 2301
		APS_ERR("obj is null!!\n");
		return -1;
	}
	err = ltr2568_als_read(ltr2568_obj->client, &ltr2568_obj->als);
	if (err < 0) {
		/* "\x013[ALS/PS] %s %d : %s failed\n"@0xffffff80091be352 */
		APS_ERR("%s failed\n", __func__);
		return -1;
	}
	*data = ltr2568_obj->als;
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ltr2568_als_factory_enable_calibration(void)
{
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ltr2568_als_factory_clear_cali(void)
{
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ltr2568_als_factory_set_cali(int32_t offset)
{
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ltr2568_als_factory_get_cali(int32_t *offset)
{
	return 0;
}

/* Riga 2338. */
static int ltr2568_ps_factory_enable_sensor(bool enable_disable,
					    int64_t sample_periods_ms)
{
	int err;

	err = ps_enable_nodata(enable_disable ? 1 : 0);
	if (err) {
		APS_ERR("%s:%s failed\n", __func__,
#line 2338
			enable_disable ? "enable" : "disable");
		return -1;
	}
	return 0;
}

/*
 * "b90007ff str" wzr,[sp,#4] azzera la variabile locale prima della chiamata, e
 * "131f7c00 asr" w0, w0, #31 trasforma il risultato di ps_get_data in 0 oppure
 * -1.
 */
static int ltr2568_ps_factory_get_data(int32_t *data)
{
	int status = 0;

	return ps_get_data(data, &status) < 0 ? -1 : 0;
}

/* Riga 2364. */
static int ltr2568_ps_factory_get_raw_data(int32_t *data)
{
	int err;

	err = ltr2568_ps_read(ltr2568_obj->client, &ltr2568_obj->ps);
	if (err < 0) {
#line 2364
		APS_ERR("%s failed\n", __func__);
		return -1;
	}
	*data = ltr2568_obj->ps;
	return 0;
}

/* "2a1f03e0 mov" w0, wzr ; "d65f03c0 ret" */
static int ltr2568_ps_factory_enable_calibration(void)
{
	return 0;
}

/* "b900f51f str" wzr,[x8,#244] */
static int ltr2568_ps_factory_clear_cali(void)
{
	ltr2568_obj->ps_cali = 0;
	return 0;
}

/* "b900f500 str" w0,[x8,#244] */
static int ltr2568_ps_factory_set_cali(int32_t offset)
{
	ltr2568_obj->ps_cali = offset;
	return 0;
}

/* "b940f508 ldr" w8,[x8,#244] ; "b9000008 str" w8,[x0] */
static int ltr2568_ps_factory_get_cali(int32_t *offset)
{
	*offset = ltr2568_obj->ps_cali;
	return 0;
}

/*
 * Riga 2407.
 * "\x013[ALS/PS] %s %d : set_psensor_threshold fail\n"@0xffffff80091be3b1
 */
static int ltr2568_ps_factory_set_threshold(int32_t threshold[2])
{
	struct ltr2568_priv *obj = ltr2568_obj;

	atomic_set(&obj->ps_thd_val_high, obj->ps_cali + threshold[0]);
	atomic_set(&obj->ps_thd_val_low, threshold[1] + obj->ps_cali);
	if (ltr2568_ps_set_thres() <= 0) {	/* "7100001f cmp" w0, #0x0 ; "5400008d b.le" */
#line 2407
		APS_ERR("set_psensor_threshold fail\n");
		return -1;
	}
	return 0;
}

/* "4b0a0129 sub" w9, w9, w10 -- le soglie tornano al netto della calibrazione. */
static int ltr2568_ps_factory_get_threshold(int32_t threshold[2])
{
	struct ltr2568_priv *obj = ltr2568_obj;

	threshold[0] = atomic_read(&obj->ps_thd_val_high) - obj->ps_cali;
	threshold[1] = atomic_read(&obj->ps_thd_val_low) - obj->ps_cali;
	return 0;
}

static struct alsps_factory_fops ltr2568_factory_fops = {	/* 0xffffff800992e698 */
	.als_enable_sensor = ltr2568_als_factory_enable_sensor,
	.als_get_data = ltr2568_als_factory_get_data,
	.als_get_raw_data = ltr2568_als_factory_get_raw_data,
	.als_enable_calibration = ltr2568_als_factory_enable_calibration,
	.als_clear_cali = ltr2568_als_factory_clear_cali,
	.als_set_cali = ltr2568_als_factory_set_cali,
	.als_get_cali = ltr2568_als_factory_get_cali,

	.ps_enable_sensor = ltr2568_ps_factory_enable_sensor,
	.ps_get_data = ltr2568_ps_factory_get_data,
	.ps_get_raw_data = ltr2568_ps_factory_get_raw_data,
	.ps_enable_calibration = ltr2568_ps_factory_enable_calibration,
	.ps_clear_cali = ltr2568_ps_factory_clear_cali,
	.ps_set_cali = ltr2568_ps_factory_set_cali,
	.ps_get_cali = ltr2568_ps_factory_get_cali,
	.ps_set_threshold = ltr2568_ps_factory_set_threshold,
	.ps_get_threshold = ltr2568_ps_factory_get_threshold,
};

/*
 * 0xffffff800992e658: due u32 a 1 e a 1 ("01 00 00 00 01 00 00 00") seguiti dal
 * puntatore alle fops.
 */
static struct alsps_factory_public ltr2568_factory_device = {
	.gain = 1,
	.sensitivity = 1,
	.fops = &ltr2568_factory_fops,
};

/* ------------------------------------------------------- i2c del driver -- */

/* Riga 2461/2538/2550/2566/2575/2591/2600/2616. */
static int ltr2568_i2c_probe(struct i2c_client *client,
			     const struct i2c_device_id *id)
{
	struct ltr2568_priv *obj = NULL;
	struct als_control_path als_ctl = {0};
	struct als_data_path als_data = {0};
	struct ps_control_path ps_ctl = {0};
	struct ps_data_path ps_data = {0};
	int err = 0;

	client->addr = 0x23;	/* "52800468 mov" w8, #0x23 ; "79000688 strh" w8,[x20,#2] */
	err = get_alsps_dts_func(client->dev.of_node, hw);
	if (err < 0) {
		/* "\x013[ALS/PS] %s %d : get customization info from dts failed\n"@0xffffff80091bdff7 */
#line 2461
		APS_ERR("get customization info from dts failed\n");
		err = -EFAULT;	/* "128001b4 mov" w20, #0xfffffff2 */
		goto exit;
	}

	obj = kzalloc(sizeof(*obj), GFP_KERNEL);
	if (!obj) {
		err = -ENOMEM;	/* "12800174 mov" w20, #0xfffffff4 */
		goto exit;
	}
	memset(obj, 0, sizeof(*obj));
	ltr2568_obj = obj;
	obj->hw = hw;
	INIT_WORK(&obj->eint_work, ltr2568_eint_work);
	obj->client = client;
	i2c_set_clientdata(client, obj);

	atomic_set(&obj->als_debounce, 300);	/* "5280258b mov" w11, #0x12c */
	atomic_set(&obj->als_deb_on, 0);
	atomic_set(&obj->als_deb_end, 0);
	atomic_set(&obj->ps_debounce, 300);
	atomic_set(&obj->ps_deb_on, 0);
	atomic_set(&obj->ps_deb_end, 0);
	atomic_set(&obj->ps_mask, 0);
	atomic_set(&obj->als_suspend, 0);
	atomic_set(&obj->ps_thd_val_high, obj->hw->ps_threshold_high);
	atomic_set(&obj->ps_thd_val_low, obj->hw->ps_threshold_low);
	obj->ps_threshold = obj->hw->ps_threshold;
	atomic_set(&obj->als_thd_val_high, obj->hw->als_threshold_high);
	atomic_set(&obj->als_thd_val_low, obj->hw->als_threshold_low);

	obj->irq_node = client->dev.of_node;
	obj->enable = 0;
	obj->pending_intr = 0;
	obj->als_level_num = ARRAY_SIZE(obj->hw->als_level);
	obj->als_value_num = ARRAY_SIZE(obj->hw->als_value);
	obj->als_modulus = (400 * 100) / (16 * 150);	/* = 16, "321c03ea orr" w10, wzr, #0x10 */

	memcpy(obj->als_level, obj->hw->als_level, sizeof(obj->als_level));
	memcpy(obj->als_value, obj->hw->als_value, sizeof(obj->als_value));
	obj->ps_cali = 0;
	atomic_set(&obj->i2c_retry, 3);		/* "320007ea orr" w10, wzr, #0x3 */
	clear_bit(CMC_BIT_ALS, &obj->enable);
	clear_bit(CMC_BIT_PS, &obj->enable);

	ltr2568_i2c_client = client;
	err = ltr2568_init_client();
	if (err)
		goto exit_init_failed;

	err = alsps_factory_device_register(&ltr2568_factory_device);
	if (err) {
		/* "\x013[ALS/PS] %s %d : ltr2568_device register failed\n"@0xffffff80091be044 */
#line 2538
		APS_ERR("ltr2568_device register failed\n");
		goto exit_init_failed;
	}

	als_ctl.is_use_common_factory = false;
	ps_ctl.is_use_common_factory = false;
	err = ltr2568_create_attr(&(ltr2568_i2c_driver.driver));
	if (err) {
		/* "\x013[ALS/PS] %s %d : create attribute err = %d\n"@0xffffff80091be077 */
#line 2550
		APS_ERR("create attribute err = %d\n", err);
		goto exit_init_failed;
	}

	als_ctl.open_report_data = als_open_report_data;
	als_ctl.enable_nodata = als_enable_nodata;
	als_ctl.set_delay = als_set_delay;
	als_ctl.batch = als_batch;
	als_ctl.flush = als_flush;
	als_ctl.is_report_input_direct = false;
	als_ctl.is_support_batch = false;
	err = als_register_control_path(&als_ctl);
	if (err) {
		/* "\x013[ALS/PS] %s %d : register fail = %d\n"@0xffffff80091be0a5 */
#line 2566
		APS_ERR("register fail = %d\n", err);
		goto exit_init_failed;
	}

	als_data.get_data = als_get_data;
	als_data.vender_div = 100;	/* "52800c89 mov" w9, #0x64 */
	err = als_register_data_path(&als_data);
	if (err) {
#line 2575
		APS_ERR("register fail = %d\n", err);
		goto exit_init_failed;
	}

	ps_ctl.open_report_data = ps_open_report_data;
	ps_ctl.enable_nodata = ps_enable_nodata;
	ps_ctl.set_delay = ps_set_delay;
	ps_ctl.batch = ps_batch;
	ps_ctl.flush = ps_flush;
	ps_ctl.is_report_input_direct = false;
	ps_ctl.is_support_batch = false;
	ps_ctl.is_polling_mode = obj->hw->polling_mode_ps;
	err = ps_register_control_path(&ps_ctl);
	if (err) {
#line 2591
		APS_ERR("register fail = %d\n", err);
		goto exit_init_failed;
	}

	ps_data.get_data = ps_get_data;
	ps_data.vender_div = 100;
	err = ps_register_data_path(&ps_data);
	if (err) {
		/* "\x013[ALS/PS] %s %d : tregister fail = %d\n"@0xffffff80091be0cc */
		APS_ERR("tregister fail = %d\n", err);
		goto exit_init_failed;
	}

	ltr2568_init_flag = true;	/* "3902f109 strb" w9,[x8,#188], w9 = 1 */
	return 0;

exit_init_failed:
	kfree(obj);
exit:
	/* "\x013[ALS/PS] %s %d : %s: err = %d\n"@0xffffff80091be0f4 */
#line 2616
	APS_ERR("%s: err = %d\n", __func__, err);
	ltr2568_init_flag = false;	/* "3902f11f strb" wzr,[x8,#188] */
	return err;
}

/*
 * Le dieci driver_remove_file sono srotolate: e' ltr2568_delete_attr
 * incorporata. Non c'e' nessun controllo del suo esito, e non c'e' nessun
 * azzeramento di ltr2568_i2c_client.
 */
static int ltr2568_i2c_remove(struct i2c_client *client)
{
	ltr2568_delete_attr(&(ltr2568_i2c_driver.driver));
	alsps_factory_device_deregister(&ltr2568_factory_device);
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}

/*
 * Otto byte di nome copiati con un solo store a 64 bit:
 * "d28e8d88 mov" x8, #0x746c ; "f2a64e48 movk" x8, #0x3272, lsl #16 ;
 * "f2c6c6a8 movk" x8, #0x3635, lsl #32 ; "f2e00708 movk" x8, #0x38, lsl #48
 * = 'ltr2568\0'.
 */
static int ltr2568_i2c_detect(struct i2c_client *client,
			      struct i2c_board_info *info)
{
	strcpy(info->type, LTR2568_DEV_NAME);
	return 0;
}

/*
 * Riga 2656/2664. "f9404c08 ldr" x8,[x0,#152] e' i2c_get_clientdata(dev) fatto
 * su `struct device *` (offset di dev.driver_data): la firma e' quella di
 * dev_pm_ops, non quella vecchia di i2c_driver.
 */
static int ltr2568_i2c_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ltr2568_priv *obj = i2c_get_clientdata(client);
	int err;

	if (!obj) {
		/* "\x013[ALS/PS] %s %d : null pointer!!\n"@0xffffff80091be792 */
#line 2656
		APS_ERR("null pointer!!\n");
		return -EINVAL;	/* "128002a0 mov" w0, #0xffffffea */
	}
	atomic_set(&obj->als_suspend, 1);
	err = ltr2568_als_enable(obj->client, 0);
	if (err < 0) {
		/* "\x013[ALS/PS] %s %d : disable als: %d\n"@0xffffff80091be7c9 */
#line 2664
		APS_ERR("disable als: %d\n", err);
		return err;
	}
	return 0;
}

/* Riga 2692/2714. */
static int ltr2568_i2c_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ltr2568_priv *obj = i2c_get_clientdata(client);
	int err;

	if (!obj) {
#line 2692
		APS_ERR("null pointer!!\n");
		return -EINVAL;
	}
	atomic_set(&obj->als_suspend, 0);

	if (test_bit(CMC_BIT_ALS, &obj->enable)) {
		err = ltr2568_als_enable(obj->client, 1);
		if (err < 0) {
			/* "\x013[ALS/PS] %s %d : enable als fail: %d\n"@0xffffff80091be800 */
#line 2714
			APS_ERR("enable als fail: %d\n", err);
		}
	}
	return 0;
}

/* "3902f11f strb" wzr,[x8,#188] -- azzera il flag, non lo mette a -1. */
static int ltr2568_remove(void)
{
	i2c_del_driver(&ltr2568_i2c_driver);
	ltr2568_init_flag = false;
	return 0;
}

/*
 * Riga 2742. La coda e'
 *   "3942f108 ldrb" w8,[x8,#188] ; "2a2803e8 mvn" w8, w8 ;
 *   "13000100 sbfx" w0, w8, #0, #1
 * cioe' `return ltr2568_init_flag ? 0 : -1` su un bool.
 */
static int ltr2568_local_init(void)
{
	if (i2c_add_driver(&ltr2568_i2c_driver)) {
		/* "\x013[ALS/PS] %s %d : add driver error\n"@0xffffff80091bdfbf */
#line 2742
		APS_ERR("add driver error\n");
		return -1;	/* "12800000 mov" w0, #0xffffffff */
	}

	if (!ltr2568_init_flag)
		return -1;

	return 0;
}

/*
 * 0xffffff8009373bd8, sezione __init, 32 byte: una sola chiamata a
 * alsps_driver_add(&ltr2568_init_info) e `return 0`.
 */
static int __init ltr2568_init(void)
{
	alsps_driver_add(&ltr2568_init_info);
	return 0;
}

/*
 * ===========================================================================
 * ltr2568_exit -- 0xffffff80093aa4bc, 4 byte, `.exit.text`, corpo VUOTO
 * ===========================================================================
 *
 * **Riscritto il 2026-08-21, secondo passaggio sulla `.exit.text`.** La
 * stesura precedente diceva due cose che non sono piu' vere: che la sezione
 * avesse 669 funzioni e 25 corpi vuoti da 4 byte, e che quei corpi fossero
 * «tutte indistinguibili fra loro e non attribuibili a nessun driver».
 *
 * I NUMERI. Il primo censimento spezzava le funzioni solo dopo un `ret` (e
 * dopo un `b` che uscisse dalla sezione), e quindi fondeva con la funzione
 * seguente ogni funzione che finisse con un `b` ALL'INDIETRO dentro se
 * stessa. Con la regola giusta -- ogni `b` incondizionato chiude un blocco;
 * se l'istruzione seguente non e' bersaglio di alcun salto, comincia una
 * funzione -- la sezione ha 692 funzioni e 26 corpi vuoti. La sezione e i
 * suoi estremi non cambiano: [0xffffff80093a8518, 0xffffff80093b06cc),
 * 33.204 byte.
 *
 * IL CORPO. Misurato, e non ammette alternative:
 *   "d65f03c0 ret"@0xffffff80093aa4bc
 * quattro byte, nessun accesso a memoria, nessuna chiamata. Coerente con
 * `ltr2568_init`, che si limita ad alsps_driver_add: `alsps_driver_del` non
 * esiste come simbolo in stock.map, quindi non c'e' niente da disfare.
 * Restano vere anche le altre negative gia' misurate: nessuna funzione della
 * sezione nomina `ltr2568_i2c_driver` (0xffffff800992e570) ne'
 * `ltr2568_init_info` (0xffffff800992e550); le OTTO funzioni della
 * `.exit.text` che chiamano `i2c_del_driver` passano otto indirizzi diversi e
 * nessuno e' 0xffffff800992e570; la stringa "ltr2568_exit" non compare in
 * nessun punto dell'immagine.
 *
 * L'ATTRIBUZIONE. E' un'inferenza dall'ORDINE DI LINK, non la lettura di un
 * nome: una funzione vuota non nomina niente, e va pesata per quello che e'.
 * Il vicino di sinistra ora si legge, ed e' questo che ha sbloccato il caso.
 * Tre sezioni diverse, disposte tutte in ordine di link, danno lo stesso
 * ordinamento degli oggetti, e i 4 byte cadono nella stessa lacuna in tutte
 * e tre:
 *
 *   oggetto        dato                .init.text             .exit.text
 *   core alsps     --                  alsps_init  0x9373794  0xffffff80093aa3e8
 *   AAL            0xffffff800992e500  AAL_init    0x9373b80  0xffffff80093aa4a0
 *   >>> ltr2568    0xffffff800992e550  ltr2568_init 0x9373bd8 >>> 0xffffff80093aa4bc
 *   core accel     0xffffff800992e878  acc_init    0x9373bf8  0xffffff80093aa4c0
 *
 *  - il dato: "91140000 add"@0xffffff80093aa4ac mette 0xffffff800992e500 in
 *    x0 per la misc_deregister di AAL, ed e' lo STESSO indirizzo che
 *    `AAL_init` passa a misc_register ("91140000 add"@0xffffff8009373b8c);
 *    "9121e000 add"@0xffffff80093aa514 mette 0xffffff800992e878 in x0 per la
 *    platform_driver_unregister del core accelerometro, indirizzo che nel
 *    resto dell'immagine e' usato solo da `acc_driver_add`; e
 *    `ltr2568_init_info` sta a 0xffffff800992e550, in mezzo ai due
 *    ("91154000 add"@0xffffff8009373be4);
 *  - la `.init.text`: in `oracolo/stock.map` le righe alsps_init / AAL_init /
 *    ltr2568_init / acc_init sono CONSECUTIVE, nessun altro simbolo fra loro;
 *  - la `.exit.text`: fra la fine di AAL ("d65f03c0 ret"@0xffffff80093aa4b8)
 *    e l'inizio del core accelerometro ("f81e0ff3 str"@0xffffff80093aa4c0)
 *    c'e' esattamente questa funzione e nient'altro. Nessun salto, in tutte
 *    le 5.018.179 righe del disassemblato, ha 0xffffff80093aa4bc come
 *    bersaglio.
 *
 * Il metodo e' TARATO su tre casi in cui la funzione si nomina da sola e cade
 * comunque nella lacuna che l'ordine di link prevede: `flashlight_exit`,
 * `flashlight_mt6370_exit` e `mir3da_exit`. Sulle 15 coppie init/exit dei
 * driver di questo progetto l'ordine e' monotono senza eccezioni.
 *
 * CIO' CHE RESTA NON PROVATO, e va detto: un oggetto che avesse una `__exit`
 * vuota e NESSUN simbolo in `.init.text` e NESSUN dato proprio sarebbe
 * invisibile a tutti e tre gli ordinamenti, e potrebbe occupare questa
 * lacuna al posto di ltr2568. Non c'e' modo di escluderlo dal binario.
 *
 * Il `module_exit()` e' una SCELTA dichiarata, non una misura: il puntatore
 * che genera finisce in `.exitcall.exit`, che vmlinux.lds scarta (`EXIT_CALL`
 * dentro /DISCARD/), mentre la funzione sopravvive comunque perche' per un
 * file built-in `__exit` implica `__used` (include/linux/init.h righe 78-85).
 *
 * Il riferimento a ltr303 in una versione ancora precedente di questo
 * commento va segnalato: ltr303 e' un driver che esiste come sorgente
 * pubblico, e questo progetto legge il binario, non i sorgenti pubblici. La
 * forma della funzione non ne dipende: i 4 byte la fissano da soli.
 *
 * MISURA DI DIMENSIONE NON FATTA: questo lotto non ha un albero di build per
 * ltr2568 e non ha ricompilato il file dopo questa rilettura. I 4 byte di
 * fabbrica sono letti dal disassemblato; il confronto con il nostro `.o` VA
 * ANCORA FATTO.
 */
static void __exit ltr2568_exit(void)
{
}

module_init(ltr2568_init);
module_exit(ltr2568_exit);

MODULE_AUTHOR("Liteon");
MODULE_DESCRIPTION("LTR-2568 ALS/PS Driver");
MODULE_LICENSE("GPL");
