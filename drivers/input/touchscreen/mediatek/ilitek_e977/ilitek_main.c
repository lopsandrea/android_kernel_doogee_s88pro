// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_ILITEK_E977 -- `ilitek_main.c`, il gruppo A del pannello touch Ilitek
 * TDDI del Doogee S88 Pro.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA, non portato da un
 * altro telefono e non adattato da un sorgente pubblico. I driver Ilitek TDDI
 * circolano pubblicamente e ALPS contiene driver touch della stessa famiglia
 * (GT1151/, GT5688/): NESSUNO E' STATO LETTO per scrivere questo file, ne'
 * per il codice ne' per i nomi. Ogni costante porta accanto la riga di
 * disassemblato da cui viene, nella forma "<8 cifre esadecimali> <mnemonico>",
 * e ogni letterale l'indirizzo a cui sta in `oracolo/stock.elf` nella forma
 * "testo"@0xINDIRIZZO.
 *
 * ===========================================================================
 * PERCHE' QUESTO FILE SI CHIAMA COSI', E GLI ALTRI SETTE NO
 * ===========================================================================
 * `ilitek_main.c` E' L'UNICO NOME CHE IL BINARIO DA'. Viene dal `__FILE__`
 * che `WARN_ON()` lascia nel binario, chiamato due volte dentro
 * `ilitek_tddi_init` (righe 356 e 357):
 *
 *   ffffff8008a55208:	b0003f40 	adrp	x0, ffffff800923e000
 *   ffffff8008a5520c:	913c2400 	add	x0, x0, #0xf09
 *   ffffff8008a55210:	52802c81 	mov	w1, #0x164                 // #356
 *   ffffff8008a55214:	97d9640a 	bl	<warn_slowpath_null>
 *
 * e 0xffffff800923ef09 e'
 * "/home/jenkins/jks-WTK6739_Q0_MP1_AP/alps/kernel-4.14/drivers/input/
 * touchscreen/mediatek/ilitek_e977/ilitek_main.c"
 * ("913c2400 add"@0xffffff8008a5520c). Da li' vengono due cose: la directory
 * di fabbrica `drivers/input/touchscreen/mediatek/ilitek_e977/`, e il fatto
 * che il gruppo A e' questo file: LE DUE ISTRUZIONI CHE MATERIALIZZANO QUEL
 * `__FILE__` STANNO A 0xffffff8008a55208 E 0xffffff8008a55220, DENTRO
 * L'INTERVALLO DEL GRUPPO A [0xffffff8008a536f8, 0xffffff8008a55664). E'
 * l'indirizzo a decidere, e basta da solo.
 *
 * LA PROVA CHE STAVA QUI PRIMA NEGAVA SE STESSA, e va detto perche' non
 * torni: diceva che le righe 356/357 cadono nell'estensione di righe del
 * gruppo A (93-993) e FUORI da quella di B (93-333) e C (36-414). Ma
 * 36 <= 356 <= 414: cadono anche dentro quella di C. L'estensione di righe
 * NON separa A da C, e non poteva. Le tre estensioni sono misurate --
 * ricostruendo il `__LINE__` di ogni `printk` (l'immediato di `mov w2`) sui
 * tre intervalli di fabbrica -- e restano scritte qui perche' sono un fatto
 * utile, non perche' provino il nome:
 *   gruppo A  [0xffffff8008a536f8, 0xffffff8008a55664)   righe  93..993
 *   gruppo B  [0xffffff8008a55664, 0xffffff8008a55d0c)   righe  93..333
 *   gruppo C  [0xffffff8008a55d0c, 0xffffff8008a5692c)   righe  36..414
 * Il `__FILE__` prova inoltre che la riga 356 sta in `ilitek_main.c` e non in
 * un header: se la funzione che contiene i due `WARN_ON` fosse una
 * `static inline` di `ilitek.h`, il `__FILE__` direbbe `ilitek.h`.
 *
 * PER GLI ALTRI SETTE FILE IL BINARIO NON DA' IL NOME, e questo file non ne
 * nomina nessuno. Quando ci si arrivera', i nomi andranno scelti e la scelta
 * dichiarata come tale: senza `MODULE_VERSION` non c'e' `KBUILD_MODNAME` per
 * oggetto e senza `.ddebug` non c'e' `filename`, quindi il nome di un file
 * non lascia traccia nel binario -- sbagliarlo non produce una divergenza
 * misurabile, ed e' proprio per questo che va dichiarato.
 *
 * ===========================================================================
 * COSA C'E' IN QUESTO FILE: 17 SIMBOLI SU 17, PIU' TRE FUNZIONI SENZA SIMBOLO
 * ===========================================================================
 * Il gruppo A e' 17 simboli e 8044 byte, e ci sono tutti. Le prime dodici
 * (3256 byte) vengono dal lotto precedente; le cinque aggiunte da questo
 * lotto sono `ilitek_tddi_sleep_handler` (1188), `ilitek_tddi_report_handler`
 * (1820), `ilitek_tddi_init` (1004), `ilitek_tddi_wq_esd_check` (96, static)
 * e `ilitek_tddi_wq_bat_check` (680, static): 4788 byte, il 59,5% del gruppo.
 *
 * Le funzioni di SORGENTE sono VENTI, non diciassette e non diciannove.
 * Tre non hanno simbolo proprio:
 *
 *   $ for n in read_power_status ilitek_update_tp_module_info \
 *   >          ilitek_tddi_wq_init; do \
 *   >     printf "%-32s %s\n" "$n" "$(grep -c " $n\$" oracolo/stock.map)"; done
 *   read_power_status                0
 *   ilitek_update_tp_module_info     0
 *   ilitek_tddi_wq_init              0
 *
 * e sono di due specie diverse:
 *
 *  - `read_power_status` e `ilitek_update_tp_module_info` le NOMINA il
 *    binario, col `__func__` dei loro `printk`, e sono riprodotte come
 *    funzioni `static` separate perche' e' l'unico modo perche' clang emetta
 *    quel `__func__`. Scriverne il corpo in linea darebbe un binario della
 *    stessa dimensione e con i messaggi sbagliati;
 *
 *  - `ilitek_tddi_wq_init` NON la nomina niente -- non ha nessuna `printk` --
 *    e a denunciarla e' il `__LINE__`: i due `warn_slowpath_null` che il
 *    binario emette dentro il simbolo `ilitek_tddi_init` portano le righe
 *    356 e 357, mentre le sei `printk` proprie di `ilitek_tddi_init` portano
 *    870, 910, 915, 932, 948 e 957, e fra le due c'e' il corpo di
 *    `ilitek_tddi_sleep_handler` (379..468). Un corpo di funzione e'
 *    contiguo, quindi quelle due righe sono di un'ALTRA funzione, incorporata
 *    li' dal compilatore. IL SUO NOME E' SCELTO, non misurato: vedi D16 e il
 *    commento sopra la funzione. E' la classe B5 dei criteri nella forma in
 *    cui il `__func__` non puo' denunciarla, e la dimensione non la denuncia
 *    affatto -- `ilitek_tddi_init` misura 1004 byte col compilatore di
 *    fabbrica sia scrivendo quel corpo in linea sia estraendolo, e i due .o
 *    hanno la stessa tabella di diciassette dimensioni.
 *
 * ===========================================================================
 * IL CONFINE, E COME SI RIVERIFICA
 * ===========================================================================
 * Il gruppo A e' l'intervallo [0xffffff8008a536f8, 0xffffff8008a55664) della
 * mappa di fabbrica: 17 simboli contigui, somma delle dimensioni 8044 =
 * ampiezza dell'intervallo, differenza zero. Sotto sta `tpd_switch_normal_mode`
 * (`tpd_misc.c`, framework condiviso), sopra `core_spi_setup` (gruppo B).
 *
 * I comandi che rifanno il lavoro, sul PC di build, in
 * /mnt/s88pro/kernel-stock:
 *
 *   ./venv/bin/python3 disassembla.py oracolo/stock.map oracolo/stock.elf \
 *       ilitek_tddi_sleep_handler --grezzo
 *   ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf 0xffffff800923e443
 *   ./venv/bin/python3 verificacitazioni.py ilitek_main.c oracolo/stock.elf
 *   ./venv/bin/python3 verificaistruzioni.py ilitek_main.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a536f8:0xffffff8008a7221c \
 *       --intervallo 0xffffff80093acac4:0xffffff80093acaf8 \
 *       --controfattuale "51000409 sub"
 *
 * GLI ESITI VERI DI QUESTI DUE COMANDI, RIESEGUITI IL 2026-08-21 SUL FILE
 * COM'E' ORA: IL PRIMO ESCE 0, IL SECONDO ESCE 1, e il cappello dichiarava i
 * comandi senza dichiararne l'esito -- che e' la regola 1 mancata. Il perche'
 * del secondo sta in D17.
 *
 * Uscita del primo (le due righe di `controfattuale` sono ELISE di
 * proposito: incollarle qui dentro le farebbe contare dallo strumento stesso
 * -- misurato, vedi la nota subito sotto):
 *
 *   [...]
 *   citazioni di istruzione trovate nel sorgente: 620 (620 a codifica, 0 ad indirizzo)
 *   confermate: 618   assenti: 0   mnemonico diverso: 0   controfattuali: 2
 *   operandi -- registro diverso: 0   immediato diverso: 0   entrambi/non confrontabili: 0
 *   $ echo $?
 *   0
 *
 * LA NOTA CHE QUELL'ELISIONE MERITA, perche' e' un fatto misurato e non una
 * cautela: `verificaistruzioni.py` legge TUTTO il file, commenti compresi, e
 * non sa distinguere una citazione da un pezzo di output incollato che le
 * somiglia. Incollando le due righe di `controfattuale` per intero, piu' la
 * riga di comando che le produce, il conteggio passava da 621/619/2 a
 * 624/619/5: tre citazioni in piu' e tre controfattuali in piu', tutti
 * fantasmi del proprio rapporto. E' la ragione per cui qui c'e' `[...]`.
 *
 * Uscita del secondo, integrale:
 *
 *   $ ./venv/bin/python3 verificacitazioni.py <file> oracolo/stock.elf
 *   [...]
 *     NON_ANCORATA 0xffffff800923e290: \x016ILITEK: (%s, %d):  -- testa assemblata da una macro di log (SOH + cifra di livello), ma nessun letterale del codice e' la coda di questo testo: non e' ne' un letterale diretto ne' un messaggio assemblato riconoscibile
 *   letterali: 109   citati: 116   verificati: 115   probanti: 108   deboli: 7
 *   di cui verificate come messaggio assemblato dalla macro di log (coda + binario, non un singolo letterale C): 85
 *   di cui verificate come nome di funzione da __func__ (non un letterale scritto a mano): 6
 *   rinviate a verificaistruzioni.py (descrivono un'istruzione, non un letterale): 599 -- non contate qui, ne' come verificate ne' come mancanti
 *   soglia imposta: 109 citati richiesti (109 letterali - 0 eccezioni)
 *   $ echo $?
 *   1
 *
 * DUE COSE DI QUESTI COMANDI ERANO SCRITTE MALE, E NON RIPRODUCEVANO:
 *  - il prefisso `tools/`. `tools/verificacitazioni.py` e
 *    `tools/verificaistruzioni.py` NON PARTONO da
 *    /mnt/s88pro/kernel-stock: entrambi muoiono con
 *    `ModuleNotFoundError: No module named 'confinecitazioni'`, perche'
 *    quel modulo sta nella radice e non in `tools/`. Le copie che si eseguono
 *    sono quelle della radice;
 *  - UN SOLO `--intervallo`. Questo file cita, oltre al gruppo A, quattro
 *    istruzioni di `ilitek_tddi_ic_get_core_ver` (gruppo D, 0xffffff8008a59ed0
 *    e seguenti, citate due volte), due di `ilitek_tddi_touch_esd_gesture_flash`
 *    (gruppo H, 0xffffff8008a6d3a4) e tre della `.exit` del gruppo C
 *    (0xffffff80093acadc e seguenti). Col solo intervallo del gruppo A lo
 *    strumento dava `assenti: 14` ed usciva 1 -- e aveva ragione: quelle
 *    codifiche nel gruppo A non ci sono. Il primo intervallo qui sopra copre
 *    l'INTERO blocco Ilitek, il secondo la `.exit`; il `--controfattuale` e'
 *    la forma di sorgente scartata di `ilitek_set_tp_data_len`, dichiarata
 *    come tale nel commento di quella funzione.
 *
 * e la misura, dal `.o` vero dell'albero `albero-ilitek-a`:
 *
 *   make -C albero-ilitek-a O=out-ilitek-a ARCH=arm64 -j6 CC=clang \
 *       HOSTCC=clang CLANG_TRIPLE=aarch64-linux-gnu- \
 *       CROSS_COMPILE=aarch64-linux-android- \
 *       drivers/input/touchscreen/mediatek/ilitek_e977/
 *   aarch64-linux-android-nm --print-size --size-sort \
 *       out-ilitek-a/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_main.o \
 *       | grep -iE " [tT] "
 *
 * ===========================================================================
 * L'ORDINE DELLE FUNZIONI -- E' MISURATO, E ORA IL FILE LO SEGUE
 * ===========================================================================
 * L'ordine in cui la fabbrica definisce le funzioni NON e' ignoto: si legge
 * dai `__LINE__`, uno per funzione, e questo file lo annota gia' sopra
 * ciascuna come «fabbrica: righe N..M». Le righe note sono 86, e mettono in
 * fila venti corpi:
 *
 *   ilitek_tddi_mp_test_handler      93..127
 *   ilitek_tddi_switch_tp_mode      150..183
 *   ilitek_tddi_gesture_recovery    198
 *   ilitek_tddi_spi_recovery        219..222
 *   ilitek_tddi_wq_esd_spi_check    (nessun log: sta fra la 222 e la 238)
 *   ilitek_tddi_wq_esd_i2c_check    238
 *   ilitek_tddi_wq_esd_check        245
 *   read_power_status               263..270      (senza simbolo)
 *   ilitek_tddi_wq_bat_check        283..299
 *   ilitek_tddi_wq_ctrl             312..346
 *   ilitek_tddi_wq_init             356..357      (senza simbolo, nome scelto)
 *   ilitek_tddi_sleep_handler       379..468
 *   ilitek_tddi_fw_upgrade_handler  488..497
 *   ilitek_set_tp_data_len          551..580
 *   ilitek_tddi_report_handler      598..669
 *   ilitek_tddi_reset_ctrl          723..739
 *   ilitek_update_tp_module_info    844..859      (senza simbolo)
 *   ilitek_tddi_init                870..957
 *   ilitek_tddi_dev_remove          964
 *   ilitek_tddi_dev_init            993
 *
 * FINO A QUESTA REVISIONE IL FILE NON SEGUIVA QUELL'ORDINE E NON DICHIARAVA
 * DI NON SEGUIRLO: seguiva l'ordine degli INDIRIZZI, che e' un'altra cosa --
 * clang non emette le funzioni nell'ordine del sorgente, e infatti nella
 * mappa `ilitek_tddi_fw_upgrade_handler` (riga 488) sta a 0xffffff8008a53a78,
 * PRIMA di `ilitek_tddi_gesture_recovery` (riga 198) a 0xffffff8008a53ed8, e
 * le due `static` stanno in fondo. Ora il file segue l'ordine di SORGENTE. La
 * differenza non tocca una sola istruzione -- la tabella delle diciassette
 * dimensioni e' identica prima e dopo il riordino, con entrambi i
 * compilatori -- ma senza il riordino D1 non era chiudibile.
 *
 * UNA SOLA POSIZIONE FRA LE VENTI NON E' MISURATA:
 * `ilitek_tddi_wq_esd_spi_check` non ha nessuna `printk`, quindi non ha
 * nessun `__LINE__`, e il binario non dice dove stia nel sorgente. Sta qui
 * fra `ilitek_tddi_spi_recovery` (222) e `ilitek_tddi_wq_esd_i2c_check`
 * (238) perche' e' dove sta nell'ordine degli indirizzi e perche' e' l'unico
 * vuoto che la sua presenza spiega; e' una SCELTA vincolata, non una misura.
 *
 * ===========================================================================
 * LE DIVERGENZE APERTE DI QUESTO FILE
 * ===========================================================================
 * D1. I `__LINE__`. Ogni `printk` di fabbrica porta il proprio numero di riga
 *     come immediato (`mov w2, #NNN`), e ogni `WARN_ON` porta il proprio come
 *     secondo argomento di `warn_slowpath_null` (`mov w1, #NNN`). I numeri di
 *     fabbrica sono noti uno per uno, ricostruiti dal disassemblato, e sono
 *     annotati sopra ciascuna funzione come «fabbrica: righe N..M». Questo
 *     file NON li riproduce: sono 58 immediati diversi su 1197 istruzioni --
 *     56 `mov w2,#imm` piu' i due `mov w1,#imm` dei WARN. E' una divergenza
 *     REALE nel binario e INVISIBILE alla misura di dimensione:
 *     `mov w2,#imm16` e' una sola istruzione per qualunque riga sotto 65536.
 *
 *     COSA MANCA PER CHIUDERLA, ora che il file e' completo di simboli. Il
 *     residuo dichiarato dal lotto precedente («basta imbottire di righe
 *     vuote») era piu' piccolo del vero, perche' il file NON seguiva l'ordine
 *     di sorgente della fabbrica -- che non e' ignoto, e' MISURATO dagli
 *     stessi `__LINE__`. Questa revisione lo ha rimesso in ordine (vedi
 *     «L'ORDINE DELLE FUNZIONI» piu' sotto), e quel che resta e' davvero solo
 *     l'imbottitura. Resta pero' una cosa che l'imbottitura da sola non fa:
 *     la funzione senza simbolo `ilitek_tddi_wq_init` deve stare fra la riga
 *     347 e la 378 con i due `WARN_ON` esattamente sulla 356 e sulla 357, e
 *     `ilitek_tddi_wq_ctrl` deve chiudersi entro la 346. Finche' quella
 *     funzione era scritta in linea dentro `ilitek_tddi_init` (righe 870+),
 *     D1 non era chiudibile affatto: nessuna quantita' di righe vuote mette
 *     un `WARN_ON` a riga 356 dentro un corpo che va dalla 870 alla 957.
 * D2. `+792` e `+8`. Il campo a +792 e' chiamato e mai scritto da nessuna
 *     delle 154, e il suo primo argomento (+8) e' anch'esso azzerato. Questo
 *     lotto NON lo chiude: il solo sito di chiamata sta in
 *     `ilitek_node_ioctl_write` (gruppo H), che non e' stato letto. Vedi
 *     `ilitek.h`.
 * D3. La `completion` a +744. E' inizializzata (`ilitek_tddi_init`) e
 *     SEGNALATA -- il `complete_all` sta in `ilitek_tddi_wq_esd_check`,
 *     "910ba100 add"@0xffffff8008a5539c piu' "97db23e1 bl"@0xffffff8008a553a0
 *     -- e mai attesa: `wait_for_completion` non compare in tutto il blocco.
 *     Questo lotto trova CHI LA SEGNALA, non chi la aspetta: o e' codice
 *     morto, o l'attesa sta fuori dal confine delle 154. Resta aperta.
 * D4. Il numero di elementi dell'array a +848 e' una scelta, non una misura
 *     (vedi `ilitek.h`). Nessuna funzione di questo lotto lo tocca.
 * D5. La FORMA di `ilitek_tddi_reset_ctrl`. Il flusso e' riprodotto
 *     esattamente, ma la forma sorgente che lo produce non e' unica: il
 *     prologo condiviso (`c640 = 0` piu' `ilitek_tddi_ic_check_otp_prog_mode`)
 *     sta fuori dal ramo `mode == 1` e dentro tutti gli altri, e almeno due
 *     scritture C diverse danno lo stesso codice. Vedi il commento li'.
 * D6. Le firme dei puntatori a funzione `c776` e `c784`. Quella di `c776`
 *     viene da UN solo sito di chiamata su 88; quella di `c784` non viene da
 *     nessuno (zero siti nel gruppo A) ed e' assunta simmetrica. Sono
 *     dichiarate come tali in `ilitek.h`.
 * D7. `ilitek_tddi_mp_test_handler` restituisce -112 e -111, che NON sono
 *     errno del kernel con un significato plausibile qui (112 = EHOSTDOWN,
 *     111 = ECONNREFUSED). Sono scritti come valori grezzi con la loro
 *     citazione, invece di essere «tradotti» in un nome di errno che il
 *     binario non dice: sono con ogni probabilita' codici propri del driver,
 *     definiti in un header che non e' stato ricostruito.
 * D8. Il byte a +0 di `struct ilitek_hwif_info` e' letto con un cast e non
 *     attraverso un campo, perche' quella struttura NON e' ricostruita in
 *     questo lotto: il gruppo A ne vede un byte solo. Vedi il commento in
 *     `ilitek_tddi_dev_init`.
 * D9. I CAMPI A +704..+736 SONO ACCEDUTI IN MODO `volatile`, ed e' misurato
 *     in due modi indipendenti:
 *      (a) in `ilitek_tddi_init` gli otto azzeramenti a 32 bit NON sono
 *          fusi, mentre lo STESSO compilatore fonde altrove due store a 32
 *          bit adiacenti in uno a 64 ("f9000169 str"@0xffffff8008a55904
 *          scrive +612 = 2 e +616 = 100 in un colpo solo, dentro
 *          `ilitek_i2c_probe`). Se una delle tre coppie ALLINEATE
 *          {704,708} {712,716} {720,724} fosse fatta di due campi ordinari,
 *          sarebbe stata fusa;
 *      (b) a "b902c928 str"@0xffffff8008a6d3a4 (gruppo H) un valore e'
 *          scritto in +712 e SUBITO RILETTO da
 *          "b942c923 ldr"@0xffffff8008a6d3a8. Con un `int` ordinario clang
 *          inoltra il valore appena scritto e la lettura sparisce.
 *     QUANTO LA MISURA PINZA DAVVERO, provato togliendo il `WRITE_ONCE` un
 *     pezzo per volta e ricompilando col compilatore DI FABBRICA:
 *      - togliendolo a TUTTI E OTTO, `ilitek_tddi_init` scende da 1004 a 992;
 *      - togliendolo a +732 e +736 soltanto, resta 1004 -- la coppia
 *        {732,736} non e' allineata a 8 e quel compilatore non la fonde
 *        comunque, quindi di quei due la misura NON dice niente;
 *      - togliendolo anche a +708 (lasciando +704), resta 1004 -- dentro una
 *        coppia basta che UNO sia volatile, quindi QUALE dei due lo sia la
 *        misura non lo dice.
 *     Cio' che e' provato e' dunque: uno per ciascuna delle tre coppie
 *     allineate, piu' +712 per la via indipendente (b). Qui tutti e otto sono
 *     trattati allo stesso modo perche' sono lo stesso genere di campo e
 *     perche' sceglierne uno per coppia sarebbe una scelta arbitraria
 *     travestita da misura; il codice prodotto e' quello di fabbrica.
 *     Qui sono scritti con `READ_ONCE`/`WRITE_ONCE`. `atomic_t` con
 *     `atomic_set`/`atomic_read` produce ESATTAMENTE le stesse istruzioni e
 *     il binario non distingue le due letture -- non ci sono istruzioni
 *     esclusive: `grep -cE "ldxr|stxr|ldadd|swp|casal" ilitek_intero.asm`
 *     da' 0 su tutto il blocco Ilitek, il che NON esclude `atomic_t`, perche'
 *     `atomic_set`/`atomic_read` su arm64 sono `WRITE_ONCE`/`READ_ONCE` e non
 *     emettono niente di esclusivo. La scelta fra le due e' dichiarata, non
 *     misurata; il DELTA DI HEADER e' in fondo.
 *     QUESTO LOTTO HA APPLICATO `READ_ONCE`/`WRITE_ONCE` ANCHE ALLE DODICI
 *     RIGHE DELLE FUNZIONI DEL LOTTO PRECEDENTE che toccano quegli otto
 *     campi -- l'unico cambiamento fatto a codice non suo -- perche' lo
 *     stesso campo trattato in due modi nello stesso file sarebbe un difetto
 *     di per se'. IL CAMBIAMENTO E' A COSTO ZERO, MISURATO: la tabella delle
 *     diciassette dimensioni e' identica prima e dopo (`ilitek_tddi_reset_ctrl`
 *     312, `ilitek_tddi_fw_upgrade_handler` 208, `ilitek_tddi_gesture_recovery`
 *     144, `ilitek_tddi_spi_recovery` 160, `ilitek_tddi_mp_test_handler` 400
 *     in entrambe le versioni). NON risolve R5.
 * D10. `read_power_status` passa a `printk` un valore a 64 bit per un `%d`
 *     ("aa0003e3 mov"@0xffffff8008a55470 e' `mov x3, x0`, non `mov w3, w0`).
 *     E' un difetto della fabbrica e si riproduce: la variabile e' `ssize_t`
 *     e il formato resta `%d`. Il compilatore di questo albero non lo
 *     segnala.
 * D11. `read_power_status` sul ramo d'errore ritorna senza rimettere
 *     `set_fs()` e senza chiudere: "14000005 b"@0xffffff8008a554ec salta sia
 *     lo `str x21,[x19,#8]` sia la `filp_close`. Difetto della fabbrica,
 *     riprodotto.
 * D12. `ilitek_tddi_wq_bat_check` prova `strstr(str, "Full")` PRIMA di
 *     `strstr(str, "Fully charged")`: il terzo confronto e' irraggiungibile.
 *     Difetto della fabbrica, riprodotto.
 * D13. Il numero di elementi di +848 resta una scelta (D4), ma ora
 *     l'ELEMENTO 0 e' misurato: `ilitek_tddi_init` ci scrive
 *     `demo_debug_info_id0` ("910c614a add"@0xffffff8008a55014,
 *     "f901a90a str"@0xffffff8008a5502c). Di quel simbolo il gruppo A usa
 *     solo l'indirizzo, quindi la sua FIRMA non e' misurata ed e' dichiarato
 *     come oggetto opaco.
 * D14. Il confronto `[c48+52] < 0x10403` dentro `ilitek_tddi_sleep_handler`
 *     passa per un cast: la struttura puntata da `c48` non e' ricostruita.
 *     Del campo il binario dice larghezza (32 bit), segno (`cset cc`, cioe'
 *     senza segno) e ruolo (e' la versione del core composta da
 *     `ilitek_tddi_ic_get_core_ver`). Vedi il DELTA DI HEADER.
 * D15. IL COMPILATORE. Il kernel di fabbrica e' stato compilato con clang
 *     r353983c (LLVM 9.0.3); l'albero di questo lotto usa r383902
 *     (LLVM 11.0.1). La differenza NON e' rumore da dichiarare: e' deriva da
 *     togliere, e questo lotto la toglie misurando con ENTRAMBI. La tabella
 *     sta sotto «LA MISURA». Col compilatore di fabbrica le cinque funzioni
 *     di questo lotto sono TUTTE E CINQUE della dimensione di fabbrica e,
 *     istruzione per istruzione, l'unica differenza che resta oltre ai
 *     `__LINE__` e' una: un salto che entra nel blocco condiviso una
 *     istruzione prima, a costo zero (vedi R2).
 * D16. LA TERZA FUNZIONE DI SORGENTE SENZA SIMBOLO. `ilitek_tddi_wq_init`
 *     (nome SCELTO) non e' nella mappa, non ha `printk` e quindi non ha
 *     nemmeno un `__func__`: a denunciarla e' solo il `__LINE__` dei due
 *     `WARN_ON`, 356 e 357, che non possono stare nel corpo di
 *     `ilitek_tddi_init` (870..957) senza che quel corpo contenga anche le
 *     righe di `ilitek_tddi_sleep_handler` (379..468). Qui e' scritta come
 *     funzione `static` a se', e il compilatore la incorpora come fa la
 *     fabbrica. Cio' che resta APERTO e' quanto di quel blocco sia davvero
 *     suo: il binario fissa solo che le due righe 356/357 non sono di
 *     `ilitek_tddi_init`, non dove cominci e dove finisca il corpo. Vedi il
 *     commento sopra la funzione.
 *     RESTA APERTO ANCHE UN SOSPETTO CHE QUESTA REVISIONE NON CHIUDE. Le
 *     righe di fabbrica note sono 86, e sono i soli punti del file su cui il
 *     binario dica qualcosa; fra un corpo e il successivo restano DICIOTTO
 *     vuoti, in cui puo' stare tanto spazio bianco e commento quanto un'altra
 *     funzione incorporata e senza `printk`, che niente denuncerebbe:
 *
 *       128..149  22 righe | 184..197  14 | 199..218  20 | 223..237  15
 *       239..244   6       | 246..262  17 | 271..282  12 | 300..311  12
 *       347..355   9       | 358..378  21 | 469..487  19 | 498..550  53
 *       581..597  17       | 670..722  53 | 740..843 104 | 860..869  10
 *       958..963   6       | 965..992  28
 *
 *     Di questi diciotto UNO e' gia' spiegato -- 223..237 e' dove sta
 *     `ilitek_tddi_wq_esd_spi_check`, che ha simbolo ma nessuna `printk` -- e
 *     due sono occupati da `ilitek_tddi_wq_init` (347..355 e 358..378, che
 *     la circondano). IL PIU' GRANDE, 740..843, e' 104 righe senza un solo
 *     log fra `ilitek_tddi_reset_ctrl` (ultimo 739) e
 *     `ilitek_update_tp_module_info` (primo 844): nessuna misura di questo
 *     file dice se dentro ci sia una funzione o soltanto commento. E' un
 *     SOSPETTO, non un difetto: a differenza del vuoto attorno al 356, dove i
 *     due `WARN_ON` provano che qualcosa c'e', qui non c'e' niente nel
 *     binario che lo confermi ne' che lo smentisca.
 * D17. `verificacitazioni.py` ESCE 1 SU QUESTO FILE, E NON E' UN DIFETTO DEL
 *     FILE: e' il solo caso che lo strumento non sa ancorare. La citazione
 *     e' "\x016ILITEK: (%s, %d): "@0xffffff800923e290, il formato di
 *     `ilitek_tddi_wq_esd_i2c_check`, cioe' la sola testa della macro di log
 *     con la CODA VUOTA; il letterale corrispondente nel codice e'
 *     `ILI_DBG("")`, e lo strumento cerca «un letterale del codice che sia la
 *     coda di questo testo» -- la stringa vuota non lo e' per costruzione.
 *     CHE I BYTE CI SIANO E' PROVATO PER ALTRA VIA, e la prova e' questa:
 *
 *       $ ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf \
 *             0xffffff800923e280 0xffffff800923e290 0xffffff800923e2a0
 *       0xffffff800923e280: b'di_spi_recovery'  (len=15, hex=64695f7370695f7265636f76657279)
 *       0xffffff800923e290: b'\x016ILITEK: (%s, %d): '  (len=20, hex=0136494c4954454b3a202825732c202564293a20)
 *       0xffffff800923e2a0: b'd): '  (len=4, hex=64293a20)
 *
 *     `--eccezione ""` NON la chiude: abbassa la soglia di copertura da 109 a
 *     108 ma il verdetto `NON_ANCORATA` resta e l'uscita resta 1 (provato).
 *     E' un LIMITE DELLO STRUMENTO su un messaggio a coda vuota, non una
 *     citazione sbagliata, ed e' scritto qui perche' il cappello dichiarava
 *     il comando senza dichiararne l'esito -- che e' la regola 1 mancata.
 * D18. `intervallo.py` NON SA CALCOLARE l'intervallo sulla numerosita' 1197,
 *     quella del confronto istruzione per istruzione:
 *
 *       $ ./venv/bin/python3 intervallo.py 1138 1197
 *       [...]
 *         File "/mnt/s88pro/kernel-stock/intervallo.py", line 49, in <genexpr>
 *           return sum(math.comb(n, i) * p ** i * (1.0 - p) ** (n - i)
 *                      ~~~~~~~~~~~~~~~~^~~~~~~~
 *       OverflowError: int too large to convert to float
 *
 *     `math.comb(1197, i)` supera il massimo di un `float` e la somma
 *     esplode. Le due numerosita' piccole (17 e 5) passano. Quindi la
 *     numerosita' 1197 CITATA PIU' SOTTO NON HA UN INTERVALLO calcolato dallo
 *     strumento canonico del progetto, e questo file non ne inventa uno con
 *     un altro conto: dice che lo strumento non lo produce. Il difetto e' di
 *     `intervallo.py`, sta fuori da questo file e non e' stato corretto qui.
 * D19. NON TUTTE LE 58 DIFFERENZE DI `__LINE__` SONO «UN IMMEDIATO DIVERSO»:
 *     ALCUNE SONO UN MNEMONICO DIVERSO, e il cappello le chiamava tutte e 58
 *     immediati. L'istogramma dei mnemonici del residuo lo dice:
 *
 *       56 mov      (i `__LINE__` che restano `movz`)
 *        2 orr      (i `__LINE__` la cui riga NOSTRA e' una maschera di bit)
 *        1 tbz      (il salto a costo zero, R2)
 *
 *     La causa e' che il numero di riga NOSTRO, per quelle `printk`, e'
 *     esprimibile come maschera di bit logica (`0x600`, `0x700`, `0x780`
 *     e simili) mentre quello di fabbrica non lo e': clang preferisce
 *     `orr wN, wzr, #imm` a `movz` quando puo'.
 *     LA RIPARTIZIONE 56/2/1 NON E' STABILE E NON C'E' UN ESEMPIO, PERCHE'
 *     ANCHE L'ESEMPIO SI SPOSTA: dipende dai numeri di riga di questo file,
 *     che cambiano a ogni riga aggiunta o tolta, anche a un commento.
 *     Misurato scrivendo questo cappello: la ripartizione e' passata da
 *     57/1/1 a 56/2/1 aggiungendo commento, e l'`orr` ha cambiato tre volte
 *     istruzione. Cio' che NON si sposta e' il totale -- 58 + 1 = 59 -- e la
 *     causa. Chi tocchera' questo file rifaccia l'istogramma invece di
 *     fidarsi di questi tre numeri.
 *
 * ===========================================================================
 * LE DIVERGENZE RESIDUE, MISURATE CON DUE COMPILATORI
 * ===========================================================================
 * R1. NESSUNA DELLE CINQUE FUNZIONI DI QUESTO LOTTO DIVERGE IN DIMENSIONE dal
 *     binario di fabbrica, quando la si compila col compilatore di fabbrica:
 *     1188, 1820, 1004, 96, 680 -- gli stessi cinque numeri della mappa. Col
 *     nostro r383902 ne divergono TRE su cinque, ed e' la tabella qui sotto a
 *     dirlo: `ilitek_tddi_sleep_handler` -16, `ilitek_tddi_init` -4,
 *     `ilitek_tddi_wq_bat_check` +4; `ilitek_tddi_report_handler` e
 *     `ilitek_tddi_wq_esd_check` fanno +0. (Il «quattro» che stava scritto
 *     qui veniva dal conteggio 13/17 dell'intero gruppo, che conta anche
 *     `ilitek_tddi_mp_test_handler` -- del lotto precedente, vedi R3.) La
 *     causa dei tre scarti e' il compilatore, non il sorgente: e' esattamente
 *     cio' che il passaggio a r353983c rimuove.
 *
 * R2. L'UNICA DIFFERENZA DI ISTRUZIONE CHE RESTA, col compilatore di
 *     fabbrica e a parte i `__LINE__`, sta in `ilitek_tddi_report_handler` e
 *     costa ZERO byte: "36fff153 tbz"@0xffffff8008a54dcc salta a
 *     0xffffff8008a54bf4, cioe' SCAVALCA la ricarica di `idev`
 *     ("f94522a8 ldr"@0xffffff8008a54bf0), mentre da noi lo stesso `tbz`
 *     entra quattro byte prima e la ricarica la esegue. Stesso numero di
 *     istruzioni (455), stessa dimensione, un accesso a memoria in piu' su
 *     una via. Provato e SCARTATO che dipenda dal `WRITE_ONCE` su +736: senza
 *     quel `WRITE_ONCE` il salto resta dov'e'.
 *
 * R3. `ilitek_tddi_mp_test_handler`: 400 byte contro 404, una istruzione, ed
 *     E' DEL LOTTO PRECEDENTE, non di questo. Misurata qui per la prima volta
 *     DAL `.o` VERO e con ENTRAMBI i compilatori -- 400 con tutti e due,
 *     quindi NON e' il compilatore. La fabbrica tiene DUE copie del
 *     caricamento del globale `idev` sulle due vie che confluiscono
 *     sull'azzeramento di +720 ("f94522c8 ldr"@0xffffff8008a53848 e
 *     "f94522c8 ldr"@0xffffff8008a53880), il sorgente attuale ne fa emettere
 *     una sola. QUESTO LOTTO NON L'HA TOCCATA: e' una funzione di un altro
 *     lotto, e la correzione va fatta da chi l'ha scritta.
 *
 * R4. `memset` CONTRO `__memset`, ed e' una MISURA, non uno stile. Vedi il
 *     commento lungo dentro `ilitek_tddi_report_handler`: col builtin la
 *     funzione fa 1816 byte, con `__memset` ne fa 1820 come la fabbrica, e la
 *     differenza e' la ricarica di `idev` che il binario porta a
 *     "f94522a8 ldr"@0xffffff8008a54ae0. E' l'unico posto di questo lotto in
 *     cui una FORMA del sorgente e' stata scelta perche' e' l'unica che
 *     riproduce il binario, e il limite di quella lettura e' dichiarato li'.
 *
 * ===========================================================================
 * LA MISURA: COM'E' CONFIGURATA E COSA VALE
 * ===========================================================================
 * Il lotto precedente misurava fuori dall'albero, con
 * `tools/misuraisolata.py`. Questo lotto misura DAL `.o` VERO: il file e'
 * compilato dentro l'albero `albero-ilitek-a` col `Makefile` della sua
 * directory (che ripete i propri `-I`, perche' il `ccflags-y` del padre non
 * si propaga) e le dimensioni escono da `nm --print-size` sull'oggetto. E lo
 * fa DUE VOLTE, col nostro clang r383902 e con quello di fabbrica r353983c
 * (`/mnt/s88pro/kernel-stock/clang-r353983c`, uscita in `out-ilitek-a-fab`):
 *
 *   funzione                          fabb  r383902  r353983c
 *   ilitek_tddi_mp_test_handler        404   400 -4    400 -4   (lotto 1)
 *   ilitek_tddi_switch_tp_mode         492   492 +0    492 +0
 *   ilitek_tddi_fw_upgrade_handler     208   208 +0    208 +0
 *   ilitek_tddi_reset_ctrl             312   312 +0    312 +0
 *   ilitek_set_tp_data_len             600   600 +0    600 +0
 *   ilitek_tddi_gesture_recovery       144   144 +0    144 +0
 *   ilitek_tddi_spi_recovery           160   160 +0    160 +0
 *   ilitek_tddi_wq_esd_spi_check        40    40 +0     40 +0
 *   ilitek_tddi_wq_esd_i2c_check        56    56 +0     56 +0
 *   ilitek_tddi_wq_ctrl                544   544 +0    544 +0
 *   ilitek_tddi_sleep_handler         1188  1172 -16   1188 +0  <- questo lotto
 *   ilitek_tddi_report_handler        1820  1820 +0    1820 +0  <- questo lotto
 *   ilitek_tddi_init                  1004  1000 -4    1004 +0  <- questo lotto
 *   ilitek_tddi_dev_remove             208   208 +0    208 +0
 *   ilitek_tddi_dev_init                88    88 +0     88 +0
 *   ilitek_tddi_wq_esd_check            96    96 +0     96 +0   <- questo lotto
 *   ilitek_tddi_wq_bat_check           680   684 +4    680 +0   <- questo lotto
 *   TOTALE                            8044  8024 -20   8040 -4
 *   identiche                                 13/17      16/17
 *
 * IL `-Werror`: NON E' NEI FLAG DELL'ALBERO, E VA CHIESTO. Questo albero non
 * compila con un `-Werror` globale -- ne ha solo sei mirati, e il cappello
 * che diceva «il vincolo del -Werror» non diceva quali:
 *
 *   $ tr ' ' '\n' < out-ilitek-a/.../.ilitek_main.o.cmd | grep -i 'werror'
 *   -Werror-implicit-function-declaration
 *   -Werror=date-time
 *   -Werror=implicit-int
 *   -Werror=incompatible-pointer-types
 *   -Werror=strict-prototypes
 *   -Werror=unknown-warning-option
 *
 * Il `-Werror` VERO e' stato quindi imposto a mano, con `KCFLAGS=-Werror`
 * aggiunto alla riga di `make`, E CON ENTRAMBI I COMPILATORI. In tutt'e due
 * i casi l'unica riga che `grep -E "CC |error|warning"` stampa per questo
 * file e' la sua compilazione, e le diciassette dimensioni sono identiche a
 * quelle della tabella qui sopra: nessun avviso, e nessuna riga di questo
 * file e' stata scritta per aggirarne uno.
 *
 * Il `-Werror` non impedisce di misurare, ma il LINK si': questo file chiama
 * simboli che stanno negli altri sette gruppi e che nessuno ha scritto,
 * quindi il driver non va ancora acceso nel `Makefile` del kernel completo.
 * Il link che fallirebbe e' l'esito onesto: nessuna di quelle funzioni ha
 * uno stub qui.
 *
 * AVVERTENZA PRIMA DELLE PERCENTUALI: su queste numerosita' l'intervallo di
 * confidenza copre mezza scala, e una percentuale da sola non dice niente.
 *
 *   $ ./venv/bin/python3 intervallo.py 16 17
 *   16 su 17 = 94.1%   IC95% Clopper-Pearson [71.3%; 99.9%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *
 *   $ ./venv/bin/python3 intervallo.py 5 5
 *   5 su 5 = 100.0%   IC95% Clopper-Pearson [47.8%; 100.0%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *
 * Il secondo conto e' quello delle SOLE cinque funzioni di questo lotto,
 * misurate col compilatore di fabbrica. Cinque su cinque NON discrimina in
 * nessuna direzione, e va detto prima del 100%: su cinque unita' l'intervallo
 * arriva fino al 47,8%. Cio' che la numerosita' non copre lo copre il
 * confronto ISTRUZIONE PER ISTRUZIONE, che ha una numerosita' di 1197 -- e
 * per la quale `intervallo.py` non produce nessun intervallo, vedi D18.
 *
 * QUEL CONFRONTO ERA AFFERMATO E NON MOSTRATO. Ora e' fatto meccanicamente,
 * fra il disassemblato di fabbrica delle cinque funzioni e il `.o` prodotto
 * dal compilatore DI FABBRICA, e questi sono i suoi numeri:
 *
 *   funzione                       fab  nos  divergenti  rilocate  residuo
 *   ilitek_tddi_sleep_handler      297  297     186        165       21
 *   ilitek_tddi_report_handler     455  455     210        191       19
 *   ilitek_tddi_init               251  251     151        141       10
 *   ilitek_tddi_wq_esd_check        24   24      12         11        1
 *   ilitek_tddi_wq_bat_check       170  170      88         80        8
 *   TOTALE                        1197 1197     647        588       59
 *
 * «rilocate» sono le istruzioni che nel `.o` portano una rilocazione in
 * `.text` (967 in tutto il file): in un oggetto non linkato il loro campo
 * immediato vale zero per costruzione, quindi la codifica DEVE differire e
 * quella differenza non e' un fatto sul sorgente. Il RESIDUO -- le 59
 * istruzioni che divergono SENZA avere una rilocazione che lo spieghi -- e'
 * il numero che conta, ed e' interamente attribuito:
 *
 *   58  `mov w2,#imm` / `mov w1,#imm`  =  i `__LINE__` (D1), di cui due
 *                                        emessi come `orr` (D19)
 *    1  "36fff153 tbz"@0xffffff8008a54dcc  =  il salto a costo zero (R2)
 *
 * e non resta NIENTE di inspiegato: 58 + 1 = 59.
 *
 * LE ISTRUZIONI RILOCATE NON SONO STATE LASCIATE CADERE. Di quelle 588 si
 * verifica per altra via cio' che la codifica non puo' dire:
 *
 *  - i 175 `bl` delle cinque funzioni: per ognuno si confronta il NOME del
 *    chiamato di fabbrica (l'operando `<nome>` di objdump) col simbolo della
 *    rilocazione corrispondente del nostro `.o`. **175 su 175 coincidono, 0
 *    divergenti.** E' il controllo che denuncerebbe un chiamato sbagliato,
 *    che la dimensione non denuncia mai;
 *  - le 143 coppie `adrp`+`add` che materializzano un indirizzo: per ognuna
 *    si legge la stringa all'indirizzo di fabbrica e la stringa che la
 *    rilocazione del nostro `.o` nomina, e SI CONFRONTANO I BYTE. 141
 *    coincidono; le sole 2 che divergono sono il `__FILE__` dei due
 *    `WARN_ON`, e divergono per il percorso di build
 *    (`/home/jenkins/jks-WTK6739_Q0_MP1_AP/alps/kernel-4.14/...` contro
 *    `/srv/archive/kernel-stock/albero-ilitek-a/...`) mentre la coda
 *    `drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_main.c` e'
 *    identica. Delle 141 che coincidono, 56 sono formati di log, 55 sono
 *    `__func__`, 28 sono altre stringhe (i percorsi, `DEF`, `plug`,
 *    `sense`, `2.0.6.0.191122`, i nomi delle code di lavoro, i nomi di campo
 *    delle macro `mutex_init`/`init_waitqueue_head`) e 2 sono in realta'
 *    indirizzi di codice -- le due `static` -- che il confronto legge come
 *    byte e trova uguali perche' i due corpi lo sono.
 *    I 55 `__func__` che coincidono ALLO STESSO INDICE DI ISTRUZIONE sono la
 *    prova diretta contro due classi che la dimensione non denuncia: contro
 *    B5 (una funzione incorporata fusa nel chiamante darebbe un `__func__`
 *    diverso: qui `read_power_status` e `ilitek_update_tp_module_info`
 *    emettono il proprio) e contro B1 (formato e `__func__` invertiti
 *    darebbero due stringhe scambiate fra due indici adiacenti);
 *  - le 15 coppie `adrp`+`add` che NON materializzano una stringa sono
 *    elencate una per una e coincidono tutte: `ilitek_esd_work` a
 *    0xffffff800a0fc968 (x3), `ilitek_bat_work` a 0xffffff800a0fc9d0 (x3),
 *    i cinque `lock_class_key` a 0xffffff800a0fca30..a34 (nel nostro `.o`
 *    `.bss+0xe4`..`.bss+0xe8`, cinque byte consecutivi come di fabbrica),
 *    `delayed_work_timer_fn`, `demo_debug_info_id0`, e gli indirizzi delle
 *    due `static` (`.text+0x1c60` e `.text+0x1cc0`, che `nm` conferma essere
 *    proprio `ilitek_tddi_wq_esd_check` e `ilitek_tddi_wq_bat_check`).
 *
 * COSA QUESTO CONFRONTO NON COPRE, detto perche' non si legga per piu' di
 * quello che e': non tocca le DODICI funzioni del lotto precedente (solo le
 * cinque di questo), e non dice niente sulla semantica -- due sorgenti
 * diversi che producono gli stessi 1197 opcode restano indistinguibili, ed
 * e' esattamente il caso di D5.
 *
 * ===========================================================================
 * LA `.exit` DEL DRIVER: C'E', E NON E' DI QUESTO FILE
 * ===========================================================================
 * `oracolo/stock.map` FINISCE a `_einittext` (0xffffff80093a8518): tutto cio'
 * che sta oltre -- la `.exit.text` -- e' invisibile a ogni strumento che
 * parta dalla mappa. La ricognizione va quindi fatta sul disassemblato
 * grezzo, e su TUTTE le sezioni marcate CODE, non su una finestra:
 *
 *   $ aarch64-linux-android-objdump -h oracolo/stock.elf
 *     0 .kernel       013ae6d8  ffffff8008080000  ...  CONTENTS, ALLOC, LOAD, CODE
 *     1 .kernel2      00217d24  ffffff8009843ae8  ...  CONTENTS, ALLOC, LOAD, CODE
 *
 * cioe' la coda di `.kernel` da `_einittext` a 0xffffff800942e6d8 PIU' tutta
 * `.kernel2` (0xffffff8009843ae8..0xffffff8009a5b80c). Sono 264.885
 * istruzioni: 103.909 nella coda di `.kernel` e 160.976 in `.kernel2`.
 *
 *   $ objdump -d --start-address=0xffffff80093a8518 \
 *             --stop-address=0xffffff800942e6d8 oracolo/stock.elf > coda.asm
 *   $ grep -c "^ffffff[0-9a-f]*:" coda.asm
 *   103909
 *
 * ATTENZIONE AL 103.910 CHE STAVA SCRITTO QUI: e' cio' che da'
 * `grep -c "^ffffff" coda.asm`, che conta anche la riga di intestazione di
 * simbolo `ffffff80093a8518 <_einittext>:` -- l'unica di tutta la coda,
 * perche' oltre `_einittext` la mappa non ha simboli. Le istruzioni sono
 * 103.909.
 *
 * IL GRUPPO A NON HA `.exit`. La scansione e' fatta con un parser dei
 * BERSAGLI di salto (ogni operando di 16 cifre esadecimali che objdump
 * stampa) e non con un'espressione regolare sulle cifre dell'indirizzo: una
 * regex sulle cifre e' precisamente il modo in cui si perde l'INGRESSO del
 * blocco, 0xffffff8008a536f8, che e' l'indirizzo che una `.exit` avrebbe piu'
 * ragione di chiamare. Su tutte e 264.885 le istruzioni:
 *   riferimenti dentro il GRUPPO A [0xffffff8008a536f8, 0xffffff8008a55664): 0
 *   riferimenti dentro il resto del blocco Ilitek (fino a 0xffffff8008a7221c): 0
 *   `adrp` verso la pagina di `.bss` del driver (0xffffff800a0fc000):         0
 *
 * CONTROLLO POSITIVO DELLO STRUMENTO (senza il quale «zero» non prova
 * niente, criterio C2): lo stesso scandaglio, con l'intervallo cercato
 * spostato su `tpd_driver_remove` (0xffffff8008a514a8) e la pagina su
 * 0xffffff8009987000, trova 3 riferimenti e 1 `adrp` -- cioe' le tre `bl
 * tpd_driver_remove` della coda (0xffffff80093acaec, 0xffffff80093acb20,
 * 0xffffff80093acb58) e l'unica materializzazione della pagina delle
 * strutture statiche del driver, quella dell'Ilitek.
 *
 * LA `.exit` DEL DRIVER ESISTE, ed e' UNA SOLA: sta a 0xffffff80093acac4, e'
 * lunga 52 byte (tredici istruzioni), NON HA SIMBOLO
 * (`grep -c ilitek_plat_dev_exit oracolo/stock.map` da' 0) e appartiene al
 * GRUPPO C -- `ilitek_plat.c` -- non a questo file. Il suo `__func__` la
 * nomina: `leggi_stringa.py oracolo/stock.elf 0xffffff800923f7ed` da'
 * `ilitek_plat_dev_exit`, e il messaggio, a 0xffffff800923f7b9, e' quello di
 * livello KERN_INFO che dice «ilitek driver has been removed», riga 449
 * ("52803822 mov"@0xffffff80093acadc). Il corpo e' due sole cose: quella
 * `printk` e `tpd_driver_remove(<oggetto a 0xffffff8009987338>)`
 * ("910ce000 add"@0xffffff80093acae8, "97da926f bl"@0xffffff80093acaec).
 *
 * L'oggetto a 0xffffff8009987338 e' la `struct tpd_driver_t` del pannello, e
 * i suoi campi si leggono con `relocazioni.py` (nell'immagine valgono zero,
 * il valore vero e' l'addend della `R_AARCH64_RELATIVE`):
 *   +0  -> 0xffffff800923f97f, cioe' la stringa `ILITEK_TDDI`
 *   +8  -> 0xffffff8008a56590 = `tpd_local_init`  (gruppo C)
 *   +16 -> 0xffffff8008a5664c = `tpd_suspend`     (gruppo C)
 *   +24 -> 0xffffff8008a56688 = `tpd_resume`      (gruppo C)
 * e sta 0x38 byte PRIMA della `struct ilitek_hwif_info` (0xffffff8009987370)
 * che `ilitek.h` gia' documenta: due oggetti statici adiacenti della stessa
 * unita' di traduzione.
 *
 * Questo lotto NON scrive quella funzione: sta in un altro file, e scriverla
 * qui la metterebbe nell'unita' sbagliata. E' materia del lotto del gruppo C.
 *
 * ===========================================================================
 * IL DELTA DI HEADER -- COSA ANDREBBE CAMBIATO IN `ilitek.h`, E NON QUI
 * ===========================================================================
 * Questo lotto NON tocca `ilitek.h` ne' nessun altro file condiviso. Cio' che
 * ha misurato e che quell'header dovrebbe recepire, quando il lotto di merge
 * lo fara':
 *
 * H1. I sei campi a +656, +664, +672, +680, +688, +696 sono `const char *`,
 *     non `u64`: `ilitek_update_tp_module_info` ci scrive sei letterali e i
 *     primi quattro sono anche i quattro argomenti `%s` della riga 859.
 *     Finche' l'header dice `u64`, qui la scrittura passa per un cast.
 * H2. I campi a +704, +708, +712, +716, +720, +724 (piu' +732 e +736, per
 *     uniformita' e non per misura -- vedi D9) sono acceduti in modo
 *     `volatile`. L'header dovrebbe dirlo: o tipandoli `atomic_t`, o
 *     lasciandoli `int` e imponendo `READ_ONCE`/`WRITE_ONCE`. Le due forme
 *     sono indistinguibili nel binario e la scelta va dichiarata.
 * H3. Il campo a +848 e' un array di puntatori a funzione il cui elemento 0
 *     e' `demo_debug_info_id0` (D13). Il NUMERO di elementi resta la scelta
 *     gia' dichiarata in D4.
 * H4. La struttura puntata da `c48` ha un `u32` a +52: e' la versione del
 *     core, composta come (maggiore<<16)|(medio<<8)|minore da
 *     `ilitek_tddi_ic_get_core_ver` ("53103d29 lsl"@0xffffff8008a59ed0,
 *     "33181d49 bfi"@0xffffff8008a59ed4, "2a0b0129 orr"@0xffffff8008a59ed8,
 *     "b9003509 str"@0xffffff8008a59edc). `ilitek_tddi_sleep_handler` la
 *     confronta SENZA SEGNO con 0x10403. La struttura non e' ricostruita:
 *     e' materia del gruppo D.
 * H5. Le funzioni degli altri gruppi che questo file chiama sono dichiarate
 *     QUI, sopra le funzioni, come fa gia' `ilitek_bus.c`. Quando quei
 *     gruppi esisteranno, i prototipi vanno spostati in `ilitek.h` e
 *     CONFRONTATI: `ilitek_calc_packet_checksum` e `ilitek_dump_data` sono
 *     dichiarate qui con la stessa firma che `ilitek_bus.c` usa, apposta.
 * H6. Il globale a 0xffffff800a0fca38 (`ilitek_bat_status` qui, NOME SCELTO)
 *     e' definito in questo file e non e' in `ilitek.h`: lo tocca solo
 *     `ilitek_tddi_wq_bat_check`, quindi puo' anche restare qui, ma il lotto
 *     di merge deve saperlo.
 * H7. NON E' UN DELTA DI `ilitek.h` MA DELLO STESSO GENERE, e va scritto
 *     dove qualcuno lo legga: nel repository la directory
 *     `driver/ilitek_e977/` NON HA NE' `Makefile` NE' `Kconfig`, mentre
 *     `qmcx983`, `sunwave`, `ltr2568`, `ft8719`, `nfc`, `da218-B`, `mt5725`
 *     ce li hanno entrambi e `aw22127` ha il `Makefile`. Il `Makefile` che
 *     compila questo file esiste SOLTANTO nell'albero di build
 *     (`albero-ilitek-a/.../ilitek_e977/Makefile`, con i tre `-I` ripetuti
 *     perche' il `ccflags-y` del padre non si propaga) e dice
 *     `obj-y := ilitek_bus.o ilitek_plat.o ilitek_main.o`, cioe' TRE dei
 *     nove file: e' lo stato parziale di QUESTO albero, non la forma finale.
 *     Portarlo nel repository cosi' com'e' significherebbe scrivere nel
 *     repository che il driver e' fatto di tre file. Per questo NON e' stato
 *     portato: la voce `Kconfig` (`WTK_ILITEK_E977`, che nell'albero ALPS non
 *     esiste, mentre nel config di fabbrica e' `=y`) e l'elenco degli oggetti
 *     sono una decisione che riguarda tutti e otto i gruppi, e la prende il
 *     lotto di merge, non questo.
 *
 * ===========================================================================
 * COSA QUESTO LOTTO NON HA FATTO
 * ===========================================================================
 * - I `__LINE__` restano quelli del nostro sorgente, non quelli della
 *   fabbrica (D1). Ora che il file e' completo di simboli E NELL'ORDINE DI
 *   SORGENTE MISURATO la cosa si potrebbe chiudere, e quel che resta e'
 *   davvero solo imbottire di righe vuote perche' ogni `printk` cada sulla
 *   sua riga; prima del riordino non lo era. Col compilatore di fabbrica sono
 *   le UNICHE differenze rimaste sulle cinque funzioni nuove, 58 su 1197
 *   istruzioni.
 * - Non ha corretto `ilitek_tddi_mp_test_handler` (R3): e' del lotto
 *   precedente, la causa e' letta e scritta, la correzione tocca a chi
 *   l'ha scritta.
 * - Non ha scritto la `.exit` del driver: e' del gruppo C.
 * - Il file NON LINKA, e deve non linkare: chiama simboli che stanno negli
 *   altri sette gruppi e che nessuno ha ancora scritto. Sono dichiarati e
 *   lasciati indefiniti. Nessuno stub.
 * - Nessuna delle funzioni degli altri gruppi e' stata letta: le loro firme
 *   vengono dai siti di chiamata di QUESTO file, e dove il sito non decide
 *   (il segno di un parametro, il numero di elementi di un array) il
 *   commento lo dice.
 * - Non ha chiuso il sospetto del vuoto di righe 740..843 (D16): 104 righe
 *   senza un solo log, in cui un'altra funzione incorporata e senza `printk`
 *   ci starebbe, e che nessuna misura di questo file esclude.
 * - Non ha riletto le dodici funzioni del lotto precedente riga per riga: le
 *   ha misurate (la tabella delle diciassette dimensioni) e riordinate, non
 *   rilette.
 *
 * ===========================================================================
 * COSA HA CAMBIATO LA REVISIONE DEL 2026-08-21, E COSA NON HA CAMBIATO
 * ===========================================================================
 * Una revisione indipendente ha trovato otto cose. Sette erano vere e sono
 * corrette qui; una era vera a meta'. Nessuna di queste correzioni cambia una
 * sola istruzione del binario prodotto: LA TABELLA DELLE DICIASSETTE
 * DIMENSIONI E' IDENTICA PRIMA E DOPO, con entrambi i compilatori (col
 * compilatore di fabbrica 16/17 identiche, totale 8040 contro 8044; col
 * nostro r383902 13/17, totale 8024). Va detto, perche' un difetto vero puo'
 * essere invisibile alla misura di dimensione -- ed e' proprio il caso del
 * piu' importante degli otto.
 *
 *  1. LA TERZA FUNZIONE DI SORGENTE SENZA SIMBOLO (D16). I due `WARN_ON`
 *     portano `__LINE__` 356 e 357, che non possono stare nel corpo di
 *     `ilitek_tddi_init` (870..957). Estratta come `ilitek_tddi_wq_init`.
 *  2. «quattro delle cinque divergono» in R1: sono TRE, e lo dice la tabella
 *     dieci righe piu' sotto, nello stesso file.
 *  3. La prova del nome del file negava se stessa (356/357 cadono anche
 *     dentro l'estensione del gruppo C, 36-414). Rifatta sull'indirizzo.
 *  4. L'ordine delle funzioni era quello degli INDIRIZZI e non quello del
 *     SORGENTE, che e' misurato, e il file non lo dichiarava. Riordinato.
 *  5. La ricognizione della `.exit` copriva solo la coda di `.kernel` e non
 *     `.kernel2`, e la sua espressione regolare non copriva l'ingresso del
 *     gruppo A. Rifatta su entrambe le sezioni con un parser dei bersagli di
 *     salto, e con un controllo positivo. LA CONCLUSIONE NON CAMBIA: il
 *     gruppo A non ha `.exit`.
 *  6. «103.910 istruzioni» nella coda: sono 103.909, piu' una riga di
 *     intestazione di simbolo che il `grep` contava.
 *  7. Il listato incollato nel cappello di `ilitek_tddi_sleep_handler` non
 *     era letterale. Ripristinato.
 *  8. VERA A META': la stessa revisione diceva che quel listato non e' in una
 *     forma che gli strumenti sappiano confrontare. Lo e':
 *     `verificaistruzioni.py` ne estrae tutte e dieci le righe nella forma a
 *     codifica, con gli operandi. La meta' vera era la non letteralita'.
 *
 * E due difetti che la revisione non aveva visto, trovati rieseguendo i
 * comandi che questo cappello pubblica:
 *  9. `tools/verificacitazioni.py` e `tools/verificaistruzioni.py` NON
 *     partono (`ModuleNotFoundError: No module named 'confinecitazioni'`).
 * 10. Il comando di `verificaistruzioni.py` pubblicato qui aveva UN SOLO
 *     `--intervallo` e usciva 1 con `assenti: 14`. Ne servono due piu' un
 *     `--controfattuale`.
 *
 * ===========================================================================
 * LA VERIFICA INDIPENDENTE DEL 2026-08-21 (secondo passaggio)
 * ===========================================================================
 * Un secondo passaggio ha RIESEGUITO da zero, senza fidarsi di questo
 * cappello, tutto cio' che il cappello afferma di misurare. Non ha scritto
 * nessuna funzione: le cinque c'erano gia' e sono complete. Ha cambiato solo
 * commento -- LA TABELLA DELLE DICIASSETTE DIMENSIONI E' IDENTICA PRIMA E
 * DOPO, con entrambi i compilatori -- e ha trovato tre cose che il cappello
 * diceva e che non stavano in piedi, piu' due che il cappello non diceva.
 *
 * CIO' CHE HA RIPRODOTTO ESATTAMENTE, e che quindi ora e' verificato due
 * volte da due mani diverse:
 *  - le diciassette dimensioni con r383902 (13/17, totale 8024) e con
 *    r353983c (16/17, totale 8040);
 *  - le cinque funzioni di questo lotto: 5 su 5 della dimensione di fabbrica
 *    col compilatore di fabbrica;
 *  - le 1197 istruzioni, cinque funzioni su cinque con lo stesso numero di
 *    istruzioni della fabbrica;
 *  - il residuo di 59 istruzioni divergenti, tutte attribuite (58 `__LINE__`
 *    + 1 `tbz`), e nessun residuo inspiegato;
 *  - `verificaistruzioni.py`: 0 assenti, 0 mnemonici diversi, 0 operandi
 *    diversi, uscita 0;
 *  - la coda di `.kernel` (103.909 istruzioni), `.kernel2` (160.976), zero
 *    riferimenti al gruppo A, e il controllo positivo su `tpd_driver_remove`
 *    che ne trova 3 piu' un `adrp`.
 *
 * CIO' CHE HA TROVATO E CORRETTO (tre voci nuove nell'elenco delle
 * divergenze):
 *  D17. `verificacitazioni.py` esce 1, e il cappello pubblicava il comando
 *       senza pubblicarne l'esito.
 *  D18. `intervallo.py` non calcola l'intervallo sulla numerosita' 1197: va
 *       in `OverflowError`. La numerosita' grande, che era il pilastro del
 *       ragionamento «cio' che la numerosita' non copre lo copre il confronto
 *       istruzione per istruzione», NON ha un intervallo prodotto dallo
 *       strumento canonico.
 *  D19. delle 58 differenze di `__LINE__` due non sono un immediato diverso
 *       ma un mnemonico diverso, ed e' un conto che si sposta da solo.
 *
 * CIO' CHE HA MISURATO E CHE NESSUNO AVEVA MISURATO:
 *  - il `-Werror` NON e' nei flag di questo albero: ce ne sono sei mirati e
 *    basta. Imposto a mano con `KCFLAGS=-Werror`, il file compila pulito con
 *    ENTRAMBI i compilatori (vedi «LA MISURA»);
 *  - le 588 istruzioni che divergono per rilocazione non erano mai state
 *    guardate. Ora lo sono, per altra via: 175 `bl` su 175 chiamano lo stesso
 *    nome della fabbrica; 141 coppie `adrp`+`add` su 143 materializzano gli
 *    stessi byte di stringa (le 2 che divergono sono il `__FILE__`, per il
 *    percorso di build); le 15 che non sono stringhe coincidono una per una.
 *    Sono i controlli che denunciano un chiamato sbagliato, un `__func__`
 *    sbagliato (B5) e i due argomenti di `printk` invertiti (B1), che la
 *    misura di dimensione non denuncia MAI;
 *  - incollare l'uscita di `verificaistruzioni.py` dentro questo file
 *    CAMBIA IL RISULTATO DELLO STRUMENTO: le righe incollate vengono contate
 *    come citazioni. Misurato, 620/618/2 contro 623/618/5. Per questo
 *    l'uscita del primo comando e' elisa dov'e' elisa.
 *
 * CIO' CHE QUESTO SECONDO PASSAGGIO NON HA FATTO: non ha riletto la
 * SEMANTICA di nessuna funzione (due sorgenti diversi con gli stessi 1197
 * opcode restano indistinguibili, ed e' D5); non ha toccato le dodici
 * funzioni del lotto precedente ne' R3; non ha corretto `intervallo.py`, che
 * sta fuori da questo file; non ha chiuso D1, D16 ne' nessuna delle altre
 * divergenze aperte.
 */

#include <linux/device.h>	/* per wakeup_source_unregister, via pm_wakeup.h */
#include <linux/errno.h>
#include <linux/delay.h>	/* per mdelay */
#include <linux/err.h>		/* per IS_ERR */
#include <linux/fs.h>		/* per filp_open, filp_close, struct file */
#include <linux/gpio.h>
#include <linux/interrupt.h>	/* per irq_set_irq_wake */
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>	/* per __memset (via asm/string.h), strstr */
#include <linux/uaccess.h>	/* per get_fs, set_fs, KERNEL_DS */
#include <linux/workqueue.h>

#include "ilitek.h"

/*
 * I GLOBALI. Le definizioni stanno qui per scelta dichiarata: il binario non
 * dice in quale delle otto unita' siano definiti (l'oracolo non ha simboli di
 * dati -- 30.100 `T`, 237 `W`, 23.372 `t`, zero dati), quindi «in
 * ilitek_main.c» e' una scelta e non una misura. Cio' che il binario DICE e'
 * la loro disposizione in .bss, che e' verificata in `ilitek.h`.
 */
struct ilitek_tddi_dev *idev;
/*
 * `ilitek_dbg_en` NON E' QUI, ed e' il link ad averlo deciso.
 *
 * Era definita in due unita' -- qui e in `ilitek_plat.c:178` -- e ognuna delle
 * due note diceva che «in quale unita' stia e' una scelta, non una misura»,
 * perche' l'oracolo non ha simboli di dati (30.100 `T`, 237 `W`, 23.372 `t`,
 * ZERO dati). Due scelte indipendenti, e nessuna misura che le confrontasse:
 * il primo `LD vmlinux.o` mai tentato di questo progetto l'ha trovata subito.
 *
 *   drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_plat.o:(.bss+0x0):
 *   multiple definition of `ilitek_dbg_en'
 *
 * E IL BINARIO SA DIRE QUALE DELLE DUE, anche senza simboli di dati: basta
 * ricostruire la disposizione della `.bss` dagli ACCESSI. `ilitek_dbg_en` sta
 * a 0xffffff800a0fc958 ("d000b528 adrp"@0xffffff8008a56130 piu'
 * "39656108 ldrb"@0xffffff8008a56134, cioe' pagina 0xa0fc000 + 2392), e il suo
 * vicinato e' questo:
 *
 *   +2372, +2376  tpd_probe
 *   +2388         ilitek_plat_probe, tpd_local_init, tpd_probe   (4 byte)
 *   +2392         ilitek_dbg_en                                  (1 byte)
 *   +2400 in poi  ilitek_tddi_init, ilitek_tddi_dev_remove,
 *                 ilitek_tddi_report_handler                     (di QUESTO file)
 *
 * cioe' e' l'ULTIMO byte del gruppo di `ilitek_plat.c` -- 2388+4 = 2392 -- e i
 * globali di questa unita' cominciano dopo, a 2400, allineati a otto. Resta
 * definita li'; `ilitek.h` la dichiara `extern` per tutti.
 */
struct workqueue_struct *ilitek_wq_esd;
struct delayed_work ilitek_esd_work;
struct workqueue_struct *ilitek_wq_bat;
struct delayed_work ilitek_bat_work;

/*
 * Le funzioni degli altri sette gruppi che questo file chiama. Staranno in
 * `ilitek.h` quando quei gruppi saranno scritti; per ora sono dichiarate qui,
 * e ogni firma viene dal proprio sito di chiamata nel gruppo A.
 */
/* gruppo B (bus I2C) */
extern int ilitek_tddi_interface_dev_init(struct ilitek_hwif_info *hwif);
extern void ilitek_tddi_interface_dev_exit(struct ilitek_tddi_dev *dev);
/* gruppo C (piattaforma) */
extern void ilitek_plat_tp_reset(void);
extern void ilitek_plat_input_register(void);
/* gruppo D (registri del chip) */
extern int ilitek_tddi_ic_code_reset(void);
extern int ilitek_tddi_ic_whole_reset(void);
extern int ilitek_tddi_ic_check_otp_prog_mode(void);
extern int ilitek_tddi_ic_func_ctrl(const char *name, int ctrl);
/* "52800640 mov"@0xffffff8008a54468 (#50) e "52800281 mov"@0xffffff8008a5446c
 * (#20): due `int`, risultato provato sul solo bit di segno
 * ("36f800e0 tbz"@0xffffff8008a54474). */
extern int ilitek_tddi_ic_check_busy(int count, int delay);
/* gruppo D (registri del chip), letta dal solo sito di `report_handler`:
 * "940011b2 bl"@0xffffff8008a54bb0, nessun argomento, risultato non usato. */
extern void ilitek_tddi_ic_get_pc_counter_forwdt(void);
/*
 * gruppo E (rapporto del tocco). Le firme vengono dai siti di chiamata di
 * `ilitek_tddi_report_handler`: x0 e' sempre il puntatore al pacchetto e w1
 * la lunghezza, tranne per `demo_debug_info_mode`, dove il secondo argomento
 * e' a 64 bit ("93407e61 sxtw"@0xffffff8008a54dfc). L'`sxtw` dice che il
 * valore convertito e' un `int`; NON dice se il parametro sia con o senza
 * segno, quindi `long` e' una scelta fra due letture equivalenti.
 * `ilitek_calc_packet_checksum` e `ilitek_dump_data` sono dichiarate con la
 * stessa firma che `ilitek_bus.c` ha gia' misurato dai propri siti, perche'
 * due dichiarazioni diverse dello stesso simbolo sarebbero un difetto che
 * solo il lotto di merge vedrebbe.
 */
extern void ilitek_tddi_touch_release_all_point(void);
extern void ilitek_tddi_report_ap_mode(void *buf, int len);
extern void ilitek_tddi_report_i2cuart_mode(void *buf, int len);
extern void ilitek_tddi_report_debug_mode(void *buf, int len);
extern void ilitek_tddi_report_gesture_mode(void *buf, int len);
extern void demo_debug_info_mode(void *buf, long len);
extern u8 ilitek_calc_packet_checksum(void *buf, u32 len);
extern void ilitek_dump_data(void *dato, int a, int b, int c,
			     const char *etichetta);
/*
 * gruppo D (registri del chip), chiamate da `ilitek_tddi_init`. Nessuna
 * riceve argomenti tranne `ilitek_ice_mode_ctrl`, che ne riceve due
 * ("320003e0 orr"@0xffffff8008a55034 + "2a1f03e1 mov"@0xffffff8008a55038 la
 * prima volta, due zeri la seconda); di `ilitek_tddi_ic_get_info` e
 * `ilitek_ice_mode_ctrl` il sito prova il bit di segno del ritorno, delle
 * altre cinque il ritorno non e' usato. I NOMI DEI PARAMETRI sono
 * posizionali: il binario passa costanti e non dice cosa siano.
 */
extern void ilitek_tddi_ic_init(void);
extern int ilitek_ice_mode_ctrl(int a0, int a1);
extern int ilitek_tddi_ic_get_info(void);
extern void ilitek_tddi_ic_get_core_ver(void);
extern void ilitek_tddi_ic_get_protocl_ver(void);
extern void ilitek_tddi_ic_get_fw_ver(void);
extern void ilitek_tddi_ic_get_tp_info(void);
extern void ilitek_tddi_ic_get_panel_info(void);
/* gruppo G (firmware e flash). "9400562d bl"@0xffffff8008a55108 */
extern void ilitek_tddi_fw_read_flash_info(void);
/* gruppo H (nodi /proc). "94007179 bl"@0xffffff8008a55104 */
extern void ilitek_tddi_node_init(void);
/*
 * gruppo E. Di questo simbolo il gruppo A usa SOLO L'INDIRIZZO
 * ("910c614a add"@0xffffff8008a55014, poi "f901a90a str"@0xffffff8008a5502c
 * dentro `idev->c848[0]`): nessun sito di chiamata cade qui, quindi la firma
 * NON e' misurata. Dichiararlo come funzione vorrebbe dire inventarne una;
 * e' dichiarato come oggetto opaco, che produce lo stesso `adrp`+`add`.
 */
extern u8 demo_debug_info_id0[];
/* gruppo F (collaudo MP) */
extern int ilitek_tddi_mp_test_main(char *apk, bool lcm_on);
/* gruppo G (firmware e flash) */
extern int ilitek_tddi_fw_upgrade(int open_file_method);

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_mp_test_handler -- 0xffffff8008a536f8, 404 byte
 * fabbrica: righe 93..127
 * ---------------------------------------------------------------------------
 * Il secondo parametro e' `bool`: la fabbrica lo maschera con 1 prima di
 * passarlo ("12000261 and"@0xffffff8008a537dc) e lo prova con
 * "360002f3 tbz"@0xffffff8008a537f0, che e' come clang prova un `i1`.
 * Il primo e' un puntatore, girato senza toccarlo a `ilitek_tddi_mp_test_main`
 * ("aa0003f4 mov"@0xffffff8008a53748, "aa1403e0 mov"@0xffffff8008a537e0).
 *
 * ATTENZIONE ALLA FORMA: la fabbrica NON chiama `ilitek_tddi_switch_tp_mode`
 * qui -- ne ha incorporata la sola via `mode == 1`, come mostra il `__func__`
 * dei tre `printk` fra 0xffffff8008a53754 e 0xffffff8008a537cc, che vale
 * "ilitek_tddi_switch_tp_mode"@0xffffff800923e096 mentre l'indirizzo sta
 * dentro `ilitek_tddi_mp_test_handler`. E' clang che incorpora e specializza
 * su una costante, non il sorgente che duplica: qui c'e' la chiamata.
 */
int ilitek_tddi_mp_test_handler(char *apk, bool lcm_on)
{
	int ret = 0;

	/* "b942cd09 ldr"@0xffffff8008a53710, "34000129 cbz"@0xffffff8008a53714 */
	if (READ_ONCE(idev->c716)) {
		/* "\x013ILITEK: (%s, %d): fw upgrade processing, ignore\n"
		 * @0xffffff800923df8b */
#line 93
		ILI_ERR("fw upgrade processing, ignore\n");
		/* "12800df4 mov"@0xffffff8008a53730: #0xffffff90 = -112. Non e'
		 * un errno del kernel con senso qui: vedi D7 nel cappello. */
		return -112;
	}

	/* "b902d109 str"@0xffffff8008a5373c */
	WRITE_ONCE(idev->c720, 1);

	/* "7100055f cmp"@0xffffff8008a5374c, "54000460 b.eq"@0xffffff8008a53750 */
	if (idev->c320 != 1) {
		/* "320003e9 orr"@0xffffff8008a53738 mette il valore 1, che e'
		 * il modo passato; la chiamata e' incorporata, vedi sopra. */
		if (ilitek_tddi_switch_tp_mode(1) < 0) {
			/* "\x013ILITEK: (%s, %d): Switch MP mode failed\n"
			 * @0xffffff800923dfda,
			 * "52800cc2 mov"@0xffffff8008a53874 -> riga 102 */
#line 102
			ILI_ERR("Switch MP mode failed\n");
			/* "12800dd4 mov"@0xffffff8008a5387c: #0xffffff91 = -111 */
			ret = -111;
			goto out;
		}
	}

	/* "94002b8d bl"@0xffffff8008a537e4 -> <ilitek_tddi_mp_test_main> */
	ret = ilitek_tddi_mp_test_main(apk, lcm_on);

out:
	/* "360002f3 tbz"@0xffffff8008a537f0 e "3707fb93 tbnz"@0xffffff8008a53884:
	 * il ripristino gira solo se `lcm_on`, su ENTRAMBE le vie -- anche su
	 * quella che ha appena fallito lo switch. */
	if (lcm_on) {
		/* "b901411f str"@0xffffff8008a537f8 */
		idev->c320 = 0;
		/* "b9426d09 ldr"@0xffffff8008a537f4,
		 * "7100053f cmp"@0xffffff8008a537fc */
		if (idev->c620 == 1) {
			/* "9400009d bl"@0xffffff8008a53804 */
			if (ilitek_tddi_fw_upgrade_handler() < 0)
				/* "\x013ILITEK: (%s, %d): FW upgrade failed
				 * during mp test\n"@0xffffff800923e005,
				 * "321e13e2 orr"@0xffffff8008a5381c -> riga 124 */
#line 124
				ILI_ERR("FW upgrade failed during mp test\n");
		} else {
			/* "b9426500 ldr"@0xffffff8008a53824,
			 * "940000c8 bl"@0xffffff8008a53828 */
			if (ilitek_tddi_reset_ctrl(idev->c612) < 0)
				/* "\x013ILITEK: (%s, %d): TP Reset failed
				 * during mp test\n"@0xffffff800923e03b,
				 * "32001be2 orr"@0xffffff8008a53840 -> riga 127 */
#line 127
				ILI_ERR("TP Reset failed during mp test\n");
		}
	}

	/* "b902d11f str"@0xffffff8008a5384c */
	WRITE_ONCE(idev->c720, 0);
	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_switch_tp_mode -- 0xffffff8008a5388c, 492 byte
 * fabbrica: righe 150..183
 * ---------------------------------------------------------------------------
 * Il parametro e' largo un byte: "12001c13 and"@0xffffff8008a538a8 lo maschera
 * con 0xff prima di ogni uso.
 */
int ilitek_tddi_switch_tp_mode(u8 mode)
{
	int ret = 0;
	bool ges_dbg;

	/* "b902d909 str"@0xffffff8008a538ac */
	idev->c728 = 1;

	/*
	 * `c324` E' LETTO PRIMA DELLO SMISTAMENTO, e questo e' misurato, non
	 * scelto: "b9414509 ldr"@0xffffff8008a538b0 cade fra lo store a +728 e
	 * quello a +320, cioe' PRIMA della chiamata indiretta a +816 -- e
	 * clang non potrebbe risalire una lettura sopra una chiamata
	 * indiretta, che potrebbe scrivere quel campo. La condizione e'
	 * costruita come congiunzione di due `cset`:
	 *   "1a9f17e9 cset"@0xffffff8008a538bc  (c324 == 1)
	 *   "1a9f17ea cset"@0xffffff8008a538c4  (mode == 15)
	 *   "0a090156 and"@0xffffff8008a53918
	 * ed e' provata DUE volte dentro il ramo `mode == 15`
	 * ("35000856 cbnz"@0xffffff8008a53928 e
	 *  "34fff816 cbz"@0xffffff8008a53a2c).
	 * Che la congiunzione includa `mode == 15`, dentro un ramo dove `mode`
	 * vale gia' 15, e' cio' che il binario porta. La FORMA sorgente che lo
	 * produce e' una scelta -- la variabile poteva chiamarsi in altro modo
	 * ed essere scritta altrimenti -- ma il FATTO che `c324` sia letto
	 * prima dello smistamento e' una misura: scrivendola dentro il ramo, la
	 * funzione perde venti byte (misurato: 472 contro 492).
	 */
	ges_dbg = (mode == 15) & (idev->c324 == 1);

	/* "b9014113 str"@0xffffff8008a538b4 */
	idev->c320 = mode;

	switch (mode) {
	case 0:
		/* "\x016ILITEK: (%s, %d): Switch to AP mode\n"
		 * @0xffffff800923e06f, riga 150. NON protetto dal byte
		 * diagnostico: il `bl` a 0xffffff8008a538ec non ha davanti
		 * nessun `ldrb [.,#2392]` sulla propria via. */
#line 150
		ILI_INFO("Switch to AP mode\n");
		/* "b9426d09 ldr"@0xffffff8008a538f4 */
		if (idev->c620 == 1) {
			/* "9400005e bl"@0xffffff8008a53900 */
			if (ilitek_tddi_fw_upgrade_handler() < 0)
				/* "\x013ILITEK: (%s, %d): FW upgrade failed\n"
				 * @0xffffff800923e0b1, riga 153.
				 * `ret` RESTA ZERO: "2a1f03f4 mov"@0xffffff8008a53a70
				 * e "2a1f03f4 mov"@0xffffff8008a53908 azzerano w20
				 * su tutte e due le vie. */
#line 153
				ILI_ERR("FW upgrade failed\n");
		} else {
			/* "b9426500 ldr"@0xffffff8008a539e8,
			 * "94000057 bl"@0xffffff8008a539ec */
			ret = ilitek_tddi_reset_ctrl(idev->c612);
			if (ret < 0)
				/* "\x013ILITEK: (%s, %d): TP Reset failed\n"
				 * @0xffffff800923e0d8, riga 158 */
#line 158
				ILI_ERR("TP Reset failed\n");
		}
		break;

	case 1:
		/* "\x016ILITEK: (%s, %d): Switch to Test mode\n"
		 * @0xffffff800923e15a, riga 171 */
#line 171
		ILI_INFO("Switch to Test mode\n");
		/* "f9419508 ldr"@0xffffff8008a53950 (+808 =
		 * `ilitek_tddi_move_mp_code_flash`),
		 * "d63f0100 blr"@0xffffff8008a53954 */
		ret = idev->c808();
		break;

	case 15:
		/* "f941990b ldr"@0xffffff8008a53910 (+816 =
		 * `ilitek_tddi_move_gesture_code_flash`),
		 * "b9421900 ldr"@0xffffff8008a53914 (l'argomento e' c536),
		 * "d63f0160 blr"@0xffffff8008a5391c */
		ret = idev->c816(idev->c536);
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): Move gesture code failed\n"
			 * @0xffffff800923e0fd, riga 164 */
#line 164
			ILI_ERR("Move gesture code failed\n");
		if (ges_dbg) {
			/* "\x016ILITEK: (%s, %d): Enable gesture debug func\n"
			 * @0xffffff800923e12b, riga 166 */
#line 166
			ILI_INFO("Enable gesture debug func\n");
			/* "32000be0 orr"@0xffffff8008a53a48: #0x7,
			 * "9400008d bl"@0xffffff8008a53a4c */
			ilitek_set_tp_data_len(7);
		}
		break;

	default:
		/* "\x013ILITEK: (%s, %d): Unknown TP mode: %x\n"
		 * @0xffffff800923e183, riga 175 */
#line 175
		ILI_ERR("Unknown TP mode: %x\n", mode);
		/* "12800014 mov"@0xffffff8008a53980: #0xffffffff = -1 */
		ret = -1;
		break;
	}

	if (ret < 0)
		/* "\x013ILITEK: (%s, %d): Switch TP mode (%d) failed \n"
		 * @0xffffff800923e1ac, riga 181. Lo spazio prima del \n c'e'
		 * anche di fabbrica. */
#line 181
		ILI_ERR("Switch TP mode (%d) failed \n", mode);

	/* Protetta: "39656108 ldrb"@0xffffff8008a539a4 sul byte diagnostico,
	 * "34000128 cbz"@0xffffff8008a539a8 salta anche il caricamento
	 * dell'argomento. "\x016ILITEK: (%s, %d): Actual TP mode = %d\n"
	 * @0xffffff800923e1dd, riga 183. */
#line 183
	ILI_DBG("Actual TP mode = %d\n", idev->c320);

	/* "b902d91f str"@0xffffff8008a539d4 */
	idev->c728 = 0;
	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_gesture_recovery -- 0xffffff8008a53ed8, 144 byte
 * fabbrica: riga 198
 * ---------------------------------------------------------------------------
 * IL LUCCHETTO CONDIZIONATO NON E' UN'INVENZIONE. La fabbrica legge una volta
 * sola la prima parola del mutex e la confronta con 7, sia prima del lock sia
 * prima dell'unlock:
 *
 *   ffffff8008a53ef4:	f8470c15 	ldr	x21, [x0,#112]!
 *   ffffff8008a53ef8:	f1001ebf 	cmp	x21, #0x7
 *   ffffff8008a53f00:	54000048 	b.hi	ffffff8008a53f08
 *   ffffff8008a53f04:	94107d1e 	bl	<mutex_lock>
 *
 * `mutex_is_locked(lock)` in questo albero e' `__mutex_owner(lock) != NULL`
 * (include/linux/mutex.h riga 143) e `__mutex_owner` e'
 * `(struct task_struct *)(atomic_long_read(&lock->owner) & ~0x07)` (riga 128),
 * cioe' esattamente «owner senza segno maggiore di 7». Verificato nel
 * sorgente ALPS `952d88e94`, non dedotto. Il valore e' letto UNA volta e
 * riusato per la decisione di sblocco -- x21 e' preservato attraverso le
 * chiamate e riconfrontato a "f1001ebf cmp"@0xffffff8008a53f38 -- quindi il
 * sorgente lo salva in una variabile.
 *
 * (Che questo idioma sia una corsa critica e' vero e non e' affare di questa
 * riscrittura: l'obiettivo e' la fedelta' al kernel di fabbrica.)
 */
int ilitek_tddi_gesture_recovery(void)
{
	int ret;
	bool locked;

	/* "f8470c15 ldr"@0xffffff8008a53ef4 */
	locked = mutex_is_locked(&idev->touch_mutex);

	/* "b9027013 str"@0xffffff8008a53efc: base gia' avanzata a idev+112,
	 * quindi 112 + 624 = 736.
	 * L'ORDINE E' MISURATO: lo store cade PRIMA di
	 * "94107d1e bl"@0xffffff8008a53f04 verso <mutex_lock>, su entrambe le
	 * vie. Scrivendo il campo dopo il lucchetto clang deve ricaricare il
	 * globale, e la funzione cresce di otto byte (misurato: 152 contro
	 * 144). */
	WRITE_ONCE(idev->c736, 1);

	if (!locked)
		mutex_lock(&idev->touch_mutex);

	/* "\x016ILITEK: (%s, %d): Doing gesture recovery\n"
	 * @0xffffff800923e206, riga 198 */
#line 198
	ILI_INFO("Doing gesture recovery\n");

	/* "3909dd13 strb"@0xffffff8008a53f28 */
	idev->c631 = 1;
	/* "f941a509 ldr"@0xffffff8008a53f24 (+840 =
	 * `ilitek_tddi_touch_esd_gesture_flash`),
	 * "d63f0120 blr"@0xffffff8008a53f2c */
	ret = idev->c840();
	/* "3909dd1f strb"@0xffffff8008a53f3c */
	idev->c631 = 0;

	if (!locked)
		/* "9101c100 add"@0xffffff8008a53f44 (idev + 0x70 = 112),
		 * "94107d21 bl"@0xffffff8008a53f48 */
		mutex_unlock(&idev->touch_mutex);

	/* "b902e11f str"@0xffffff8008a53f50 */
	WRITE_ONCE(idev->c736, 0);
	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_spi_recovery -- 0xffffff8008a53f68, 160 byte
 * fabbrica: righe 219..222
 * ---------------------------------------------------------------------------
 * Restituisce `void`: l'epilogo a 0xffffff8008a53ff8 non scrive w0, a
 * differenza di quello di `ilitek_tddi_gesture_recovery`
 * ("2a1303e0 mov"@0xffffff8008a53f54), che e' la sua gemella per struttura.
 */
void ilitek_tddi_spi_recovery(void)
{
	int ret;
	bool locked;

	/* "f8470c14 ldr"@0xffffff8008a53f84, "f1001e9f cmp"@0xffffff8008a53f88 */
	locked = mutex_is_locked(&idev->touch_mutex);

	/* "b9027015 str"@0xffffff8008a53f8c: base gia' avanzata a idev+112, e
	 * come nella gemella lo store cade PRIMA di
	 * "94107cfa bl"@0xffffff8008a53f94 verso <mutex_lock>. */
	WRITE_ONCE(idev->c736, 1);

	if (!locked)
		mutex_lock(&idev->touch_mutex);

	/* "\x016ILITEK: (%s, %d): Doing spi recovery\n"@0xffffff800923e24f,
	 * riga 219 */
#line 219
	ILI_INFO("Doing spi recovery\n");

	/* "3909dd15 strb"@0xffffff8008a53fb4 */
	idev->c631 = 1;
	/* "97fffeb0 bl"@0xffffff8008a53fb8 */
	ret = ilitek_tddi_fw_upgrade_handler();
	if (ret < 0)
		/* "\x013ILITEK: (%s, %d): FW upgrade failed\n"
		 * @0xffffff800923e0b1, riga 222 -- lo stesso letterale che usa
		 * `ilitek_tddi_switch_tp_mode` alla riga 153. */
#line 222
		ILI_ERR("FW upgrade failed\n");
	/* "3909dd1f strb"@0xffffff8008a53fe0 */
	idev->c631 = 0;

	if (!locked)
		/* "9101c100 add"@0xffffff8008a53fe8,
		 * "94107cf8 bl"@0xffffff8008a53fec */
		mutex_unlock(&idev->touch_mutex);

	/* "b902e11f str"@0xffffff8008a53ff4 */
	WRITE_ONCE(idev->c736, 0);
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_wq_esd_spi_check -- 0xffffff8008a54008, 40 byte
 * fabbrica: riga non nota (nessuna printk; sta fra la 222 e la 238)
 * ---------------------------------------------------------------------------
 */
int ilitek_tddi_wq_esd_spi_check(void)
{
	/* "f9419108 ldr"@0xffffff8008a54018 (+800),
	 * "d63f0100 blr"@0xffffff8008a5401c */
	int ret = idev->c800();

	/* "71028c1f cmp"@0xffffff8008a54020 con #0xa3,
	 * "5a9f03e0 csetm"@0xffffff8008a54024: `csetm` mette -1, non 1. */
	return (ret != 0xa3) ? -1 : 0;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_wq_esd_i2c_check -- 0xffffff8008a54030, 56 byte
 * fabbrica: riga 238
 * ---------------------------------------------------------------------------
 * Il formato e' "\x016ILITEK: (%s, %d): "@0xffffff800923e290, cioe' la sola
 * testa della macro senza messaggio: di fabbrica questa funzione stampa il
 * proprio nome e la propria riga e nient'altro. Un `ILI_DBG("")` la riproduce
 * esattamente, perche' la concatenazione col prefisso da' lo stesso letterale.
 * Restituisce sempre 0 ("2a1f03e0 mov"@0xffffff8008a54060).
 */
int ilitek_tddi_wq_esd_i2c_check(void)
{
	/* Protetta: "39656108 ldrb"@0xffffff8008a54034,
	 * "34000148 cbz"@0xffffff8008a54038 -- e il salto scavalca perfino il
	 * prologo della funzione ("a9bf7bfd stp"@0xffffff8008a5403c), che
	 * clang mette dentro il ramo. */
#line 238
	ILI_DBG("");
	return 0;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_wq_esd_check -- 0xffffff8008a5535c, 96 byte
 * fabbrica: riga 245
 * ---------------------------------------------------------------------------
 * E' `static`, E LO DICE LA MAPPA: `oracolo/stock.map` la marca `t`
 * minuscolo, contro `T` per le altre quindici del gruppo. L'unico
 * riferimento in tutto il blocco e' l'`INIT_DELAYED_WORK` dentro
 * `ilitek_tddi_init` ("910d7108 add"@0xffffff8008a54fa0 materializza
 * 0xffffff8008a5535c), quindi la firma e' quella che `INIT_DELAYED_WORK`
 * impone: `void (*)(struct work_struct *)`. Il parametro non e' usato --
 * nessuna istruzione del corpo tocca x0 in ingresso.
 *
 * E' QUI CHE LA `completion` A +744 VIENE SEGNALATA, ed e' l'unico posto in
 * tutto il blocco: "910ba100 add"@0xffffff8008a5539c (idev + 744) piu'
 * "97db23e1 bl"@0xffffff8008a553a0 verso <complete_all>. La divergenza D3 del
 * cappello resta aperta -- nessuno la aspetta -- ma ora il CHI SEGNALA e'
 * scritto.
 */
static void ilitek_tddi_wq_esd_check(struct work_struct *work)
{
	/* "f9419d08 ldr"@0xffffff8008a55370 (+824),
	 * "d63f0100 blr"@0xffffff8008a55374,
	 * "36f80100 tbz"@0xffffff8008a55378 */
	if (idev->c824() < 0) {
		/* "\x013ILITEK: (%s, %d): SPI ACK failed, doing spi
		 * recovery\n"@0xffffff800923eabd, riga 245 -- lo stesso
		 * letterale che `ilitek_tddi_report_handler` usa alla riga
		 * 638. */
#line 245
		ILI_ERR("SPI ACK failed, doing spi recovery\n");
		/* "97fffaf5 bl"@0xffffff8008a55394 */
		ilitek_tddi_spi_recovery();
	}

	/* "910ba100 add"@0xffffff8008a5539c, "97db23e1 bl"@0xffffff8008a553a0 */
	complete_all(&idev->c744);
	/* "2a1f03e0 mov"@0xffffff8008a553a8 (tipo 0),
	 * "320003e1 orr"@0xffffff8008a553a4 (ctrl 1),
	 * "97fffb2f bl"@0xffffff8008a553ac */
	ilitek_tddi_wq_ctrl(0, 1);
}

/*
 * Il percorso che `read_power_status` apre. E' UN SOLO letterale usato DUE
 * volte -- come argomento di `filp_open` ("9102bc00 add"@0xffffff8008a5540c)
 * e come argomento `%s` del messaggio d'errore
 * ("9102bc63 add"@0xffffff8008a554c8) -- ed e' lo stesso indirizzo,
 * "/sys/class/power_supply/battery/status"@0xffffff800923f0af.
 */
#define ILITEK_POWER_STATUS_PATH	"/sys/class/power_supply/battery/status"

/*
 * ---------------------------------------------------------------------------
 * read_power_status -- NESSUN SIMBOLO PROPRIO, incorporata in
 * ilitek_tddi_wq_bat_check
 * fabbrica: righe 263 e 270
 * ---------------------------------------------------------------------------
 * `grep -c " read_power_status$" oracolo/stock.map` da' 0: la funzione non ha
 * simbolo. Che esista LO DICE IL `__func__`: i due `printk` a
 * 0xffffff8008a554d0 e 0xffffff8008a55488 portano
 * "read_power_status"@0xffffff800923f0fd ("9103f421 add"@0xffffff8008a55480 e
 * "9103f421 add"@0xffffff8008a554c4) mentre i loro indirizzi cadono dentro
 * `ilitek_tddi_wq_bat_check`. Riprodurla come funzione separata e' l'UNICO
 * modo perche' clang emetta quel `__func__`: scriverne il corpo in linea
 * darebbe "ilitek_tddi_wq_bat_check" e nessuna misura di dimensione lo
 * denuncerebbe.
 *
 * DIFETTO DELLA FABBRICA CHE SI RIPRODUCE: sul ramo d'errore la funzione
 * ritorna SENZA rimettere a posto `set_fs()` e senza chiudere niente -- da
 * 0xffffff8008a554b4 si va dritti a 0xffffff8008a55500 saltando sia lo
 * `str x21,[x19,#8]` sia la `filp_close`. Il processo resta con
 * `addr_limit = KERNEL_DS`. Non e' un errore di lettura: e' il salto
 * "14000005 b"@0xffffff8008a554ec.
 */
static int read_power_status(u8 *buf)
{
	struct file *f;
	mm_segment_t old_fs;
	ssize_t byte;

	/* "f9400675 ldr"@0xffffff8008a553f0 (current + 8),
	 * "f9000669 str"@0xffffff8008a553f4 con x9 = -1 = KERNEL_DS,
	 * "d503379f dsb"@0xffffff8008a553f8 + "d5033fdf isb"@0xffffff8008a553fc,
	 * "941006ba bl"@0xffffff8008a55404 verso <set_bit> con w0 = 5 =
	 * TIF_FSCHECK: e' l'espansione di `set_fs` di questo albero. */
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* "2a1f03e1 mov"@0xffffff8008a55410 (O_RDONLY = 0),
	 * "2a1f03e2 mov"@0xffffff8008a55414 (modo 0),
	 * "97e04190 bl"@0xffffff8008a5541c */
	f = filp_open(ILITEK_POWER_STATUS_PATH, O_RDONLY, 0);
	/* L'ORDINE DEI DUE TERMINI E' MISURATO: prima IS_ERR
	 * ("b140041f cmn"@0xffffff8008a55420 con #0x1,lsl#12 piu'
	 * "54000488 b.hi"@0xffffff8008a55424), poi il NULL
	 * ("b4000440 cbz"@0xffffff8008a5542c). `IS_ERR_OR_NULL` del kernel
	 * prova il NULL per PRIMO, quindi NON e' quella macro. */
	if (IS_ERR(f) || f == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to open %s\n"
		 * @0xffffff800923f0d6, riga 263 */
#line 263
		ILI_ERR("Failed to open %s\n", ILITEK_POWER_STATUS_PATH);
		return -1;
	}

	/* "f9401688 ldr"@0xffffff8008a55430 (f->f_op),
	 * "f9400508 ldr"@0xffffff8008a55440 (+8 = llseek),
	 * "aa1f03e1 mov"@0xffffff8008a55438 (offset 0),
	 * "2a1f03e2 mov"@0xffffff8008a5543c (whence 0 = SEEK_SET),
	 * "d63f0100 blr"@0xffffff8008a55444 */
	f->f_op->llseek(f, 0, SEEK_SET);
	/* "f9400908 ldr"@0xffffff8008a55458 (+16 = read),
	 * "9101a283 add"@0xffffff8008a5544c = f + 104, cioe' &f->f_pos,
	 * "52800282 mov"@0xffffff8008a55454 (20 byte),
	 * "d63f0100 blr"@0xffffff8008a55460 */
	byte = f->f_op->read(f, buf, 20, &f->f_pos);

	/* protetta: "39656108 ldrb"@0xffffff8008a55468.
	 * "\x016ILITEK: (%s, %d): Read %d bytes\n"@0xffffff800923f10f,
	 * riga 270. DIFETTO DELLA FABBRICA: l'argomento e' passato a 64 bit
	 * ("aa0003e3 mov"@0xffffff8008a55470 e' `mov x3, x0`, non `mov w3,w0`)
	 * mentre il formato dice `%d`. Vedi la divergenza D10 del cappello. */
#line 270
	ILI_DBG("Read %d bytes\n", byte);

	/* "f9000675 str"@0xffffff8008a5548c rimette il vecchio limite, poi
	 * "b10006bf cmn"@0xffffff8008a554a4: il confronto con -1 e' il ramo
	 * `fs == KERNEL_DS` che l'ALTERNATIVE di UAO porta con se'. */
	set_fs(old_fs);
	/* "aa1f03e1 mov"@0xffffff8008a554f8 (NULL),
	 * "97e04299 bl"@0xffffff8008a554fc */
	filp_close(f, NULL);
	return 0;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_wq_bat_check -- 0xffffff8008a553bc, 680 byte
 * fabbrica: righe 283, 285, 290, 292, 297, 299
 * ---------------------------------------------------------------------------
 * `static` per la stessa prova di `ilitek_tddi_wq_esd_check`: `t` minuscolo
 * in `oracolo/stock.map`, e l'unico riferimento e' l'`INIT_DELAYED_WORK` di
 * `ilitek_tddi_init` ("910ef108 add"@0xffffff8008a54fdc).
 *
 * IL GLOBALE A 0xffffff800a0fca38 E' TOCCATO SOLO DA QUI. In tutto il blocco
 * Ilitek ci sono TRE soli accessi a quell'offset, e sono le tre righe di
 * questa funzione: "b94a3a88 ldr"@0xffffff8008a55568,
 * "b90a3a88 str"@0xffffff8008a555c4, "b94a3a88 ldr"@0xffffff8008a55600.
 * E' letto a LARGHEZZA PIENA (`ldr w`), quindi e' a 32 bit; assume TRE valori
 * distinti (0 iniziale, 1, 2), il che spiega perche' clang non lo abbia
 * ristretto a un bit come fa con i globali booleani. La larghezza e' misurata;
 * il NOME e' scelto, come per gli altri cinque globali del gruppo.
 */
int ilitek_bat_status;

static void ilitek_tddi_wq_bat_check(struct work_struct *work)
{
	/* "a9007fff stp"@0xffffff8008a553ec (16 byte) piu'
	 * "b90013ff str"@0xffffff8008a553e8 (4): venti byte azzerati sulla
	 * pila. Il canarino a [sp,#24] dice che e' `-fstack-protector-strong`
	 * a vederci un array. */
	u8 str[20] = {0};

	/* "36f800e0 tbz" non c'e': il ritorno NON e' provato con un salto
	 * proprio -- il ramo d'errore e' il seguito della `filp_open` fallita,
	 * "54000488 b.hi"@0xffffff8008a55424, che dopo la riga 263 stampa
	 * anche questa e prosegue. */
	if (read_power_status(str) < 0)
		/* "\x013ILITEK: (%s, %d): Read power status failed\n"
		 * @0xffffff800923ef94, riga 283 */
#line 283
		ILI_ERR("Read power status failed\n");

	/* protetta: "39656268 ldrb"@0xffffff8008a55504.
	 * "\x016ILITEK: (%s, %d): Batter Status: %s\n"@0xffffff800923efdb,
	 * riga 285. Il refuso «Batter» e' della fabbrica. */
#line 285
	ILI_DBG("Batter Status: %s\n", str);

	/* "Charging"@0xffffff800925eb49 ("912d2421 add"@0xffffff8008a5552c),
	 * "Full"@0xffffff800927a583 ("91160c21 add"@0xffffff8008a55540),
	 * "Fully charged"@0xffffff800923f002 ("91000821 add"@0xffffff8008a55554).
	 * I primi due terminano con `cbnz` ("b5000160 cbnz"@0xffffff8008a55538,
	 * "b50000c0 cbnz"@0xffffff8008a5554c), il terzo con `cbz`
	 * ("b40004e0 cbz"@0xffffff8008a55560): e' un `||` a tre termini.
	 * Che "Full" renda irraggiungibile "Fully charged" e' un difetto della
	 * fabbrica e si riproduce. */
	if (strstr(str, "Charging") || strstr(str, "Full") ||
	    strstr(str, "Fully charged")) {
		/* "b94a3a88 ldr"@0xffffff8008a55568,
		 * "7100051f cmp"@0xffffff8008a5556c con #1,
		 * "540002c0 b.eq"@0xffffff8008a55570 */
		if (ilitek_bat_status != 1) {
			/* protetta: "39656268 ldrb"@0xffffff8008a55574.
			 * "\x016ILITEK: (%s, %d): Charging mode\n"
			 * @0xffffff800923f010, riga 290 */
#line 290
			ILI_DBG("Charging mode\n");
			/* "plug"@0xffffff80090fa501,
			 * "2a1f03e1 mov"@0xffffff8008a5559c (ctrl 0),
			 * "94000a3a bl"@0xffffff8008a555a0 */
			if (ilitek_tddi_ic_func_ctrl("plug", 0) < 0)
				/* "\x013ILITEK: (%s, %d): Write plug in
				 * failed\n"@0xffffff800923f033, riga 292 */
#line 292
				ILI_ERR("Write plug in failed\n");
			/* "320003e8 orr"@0xffffff8008a555c0 +
			 * "b90a3a88 str"@0xffffff8008a555c4 */
			ilitek_bat_status = 1;
		}
	} else {
		/* "7100091f cmp"@0xffffff8008a55604 con #2,
		 * "54fffe00 b.eq"@0xffffff8008a55608 */
		if (ilitek_bat_status != 2) {
			/* protetta: "39656268 ldrb"@0xffffff8008a5560c.
			 * "\x016ILITEK: (%s, %d): Not charging mode\n"
			 * @0xffffff800923f05d, riga 297 */
#line 297
			ILI_DBG("Not charging mode\n");
			/* "320003e1 orr"@0xffffff8008a55634 (ctrl 1),
			 * "94000a14 bl"@0xffffff8008a55638 */
			if (ilitek_tddi_ic_func_ctrl("plug", 1) < 0)
				/* "\x013ILITEK: (%s, %d): Write plug out
				 * failed\n"@0xffffff800923f084, riga 299 */
#line 299
				ILI_ERR("Write plug out failed\n");
			/* "321f03e8 orr"@0xffffff8008a55658 (#2) e lo STESSO
			 * "b90a3a88 str"@0xffffff8008a555c4 dell'altro ramo,
			 * raggiunto con "17ffffda b"@0xffffff8008a5565c. */
			ilitek_bat_status = 2;
		}
	}

	/* "320003e0 orr"@0xffffff8008a555c8, "320003e1 orr"@0xffffff8008a555cc,
	 * "97fffaa6 bl"@0xffffff8008a555d0 */
	ilitek_tddi_wq_ctrl(1, 1);
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_wq_ctrl -- 0xffffff8008a54068, 544 byte
 * fabbrica: righe 312..346
 * ---------------------------------------------------------------------------
 * Il ritardo passato a `queue_delayed_work` e' il valore GREZZO 1000 (ESD) e
 * 500 (batteria), non una conversione da millisecondi: "52807d03 mov"
 * @0xffffff8008a540e8 e "52803e83 mov"@0xffffff8008a54180 caricano
 * direttamente #0x3e8 e #0x1f4 nel quarto argomento.
 *
 * E il PRIMO argomento e' 8 ("321d03e0 orr"@0xffffff8008a540e4), cioe'
 * `WORK_CPU_UNBOUND`: nell'albero ALPS `WORK_CPU_UNBOUND = NR_CPUS`
 * (include/linux/workqueue.h riga 61) e il config di fabbrica ha
 * `CONFIG_NR_CPUS=8` (e977_dg_m13_71_q0.config riga 456). E' quindi la
 * `queue_delayed_work()` in linea, non una `queue_delayed_work_on()` con una
 * CPU scelta -- una differenza di comportamento che nessuna misura di
 * dimensione vedrebbe, perche' le due sono la stessa istruzione.
 *
 * La guardia `c527` e' letta DUE volte, una per ramo
 * ("39483d28 ldrb"@0xffffff8008a5408c e "39483d28 ldrb"@0xffffff8008a54124):
 * se stesse prima dello smistamento clang la leggerebbe una volta sola.
 */
void ilitek_tddi_wq_ctrl(int type, int ctrl)
{
	switch (type) {
	case 0:
		/* "340009e8 cbz"@0xffffff8008a54090 */
		if (!idev->c527)
			break;
		/* "f944b288 ldr"@0xffffff8008a54098 (0xffffff800a0fc960),
		 * "b40009e8 cbz"@0xffffff8008a5409c */
		if (!ilitek_wq_esd) {
			/* "\x013ILITEK: (%s, %d): wq esd is null\n"
			 * @0xffffff800923e2c2, riga 312 */
#line 312
			ILI_ERR("wq esd is null\n");
			break;
		}
		/* "1a9f07ea cset"@0xffffff8008a540a4 (ne),
		 * "3908412a strb"@0xffffff8008a540ac */
		idev->c528 = (ctrl != 0);
		/* "7100043f cmp"@0xffffff8008a540a8 con #0x1,
		 * "54000b01 b.ne"@0xffffff8008a540b0 */
		if (ctrl == 1) {
			/* protetta: "39656269 ldrb"@0xffffff8008a540b8.
			 * "\x016ILITEK: (%s, %d): execute esd check\n"
			 * @0xffffff800923e2fa, riga 317 */
#line 317
			ILI_DBG("execute esd check\n");
			/* "97d9ed7b bl"@0xffffff8008a540f0,
			 * "370006c0 tbnz"@0xffffff8008a540f4 sul bit 0 del
			 * ritorno */
			if (!queue_delayed_work(ilitek_wq_esd,
						&ilitek_esd_work, 1000))
				/* protetta: "39656268 ldrb"@0xffffff8008a540f8.
				 * "\x016ILITEK: (%s, %d): esd check was already
				 * on queue\n"@0xffffff800923e321, riga 319 */
#line 319
				ILI_DBG("esd check was already on queue\n");
		} else {
			/* "97d9f281 bl"@0xffffff8008a54218 */
			cancel_delayed_work_sync(&ilitek_esd_work);
			/* "97d9ee2d bl"@0xffffff8008a54220 */
			flush_workqueue(ilitek_wq_esd);
			/* protetta: "39656108 ldrb"@0xffffff8008a54228.
			 * "\x016ILITEK: (%s, %d): cancel esd wq\n"
			 * @0xffffff800923e355, riga 323 */
#line 323
			ILI_DBG("cancel esd wq\n");
		}
		break;

	case 1:
		/* "34000528 cbz"@0xffffff8008a54128 */
		if (!idev->c527)
			break;
		/* "f944e688 ldr"@0xffffff8008a54130 (0xffffff800a0fc9c8),
		 * "b4000608 cbz"@0xffffff8008a54134 */
		if (!ilitek_wq_bat) {
			/* "\x013ILITEK: (%s, %d): WQ BAT is null\n"
			 * @0xffffff800923e378, riga 330 */
#line 330
			ILI_ERR("WQ BAT is null\n");
			break;
		}
		/* "1a9f07ea cset"@0xffffff8008a5413c,
		 * "3908452a strb"@0xffffff8008a54144 */
		idev->c529 = (ctrl != 0);
		/* "7100043f cmp"@0xffffff8008a54140,
		 * "54000821 b.ne"@0xffffff8008a54148 */
		if (ctrl == 1) {
			/* protetta: "39656269 ldrb"@0xffffff8008a54150.
			 * "\x016ILITEK: (%s, %d): execute bat check\n"
			 * @0xffffff800923e39c, riga 335 */
#line 335
			ILI_DBG("execute bat check\n");
			/* "97d9ed55 bl"@0xffffff8008a54188,
			 * "37000200 tbnz"@0xffffff8008a5418c */
			if (!queue_delayed_work(ilitek_wq_bat,
						&ilitek_bat_work, 500))
				/* protetta: "39656268 ldrb"@0xffffff8008a54190.
				 * "\x016ILITEK: (%s, %d): bat check was already
				 * on queue\n"@0xffffff800923e3c3, riga 337 */
#line 337
				ILI_DBG("bat check was already on queue\n");
		} else {
			/* "97d9f272 bl"@0xffffff8008a54254 */
			cancel_delayed_work_sync(&ilitek_bat_work);
			/* "97d9ee1e bl"@0xffffff8008a5425c */
			flush_workqueue(ilitek_wq_bat);
			/* protetta: "39656108 ldrb"@0xffffff8008a54264.
			 * "\x016ILITEK: (%s, %d): cancel bat wq\n"
			 * @0xffffff800923e3f7, riga 341 */
#line 341
			ILI_DBG("cancel bat wq\n");
		}
		break;

	default:
		/* "\x013ILITEK: (%s, %d): Unknown WQ type, %d\n"
		 * @0xffffff800923e41a, riga 346. NON protetta: il
		 * "39656108 ldrb" piu' vicino sta su un'altra via. */
#line 346
		ILI_ERR("Unknown WQ type, %d\n", type);
		break;
	}
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_wq_init -- SENZA SIMBOLO, incorporata in `ilitek_tddi_init`
 * fabbrica: righe 356 e 357 (i due WARN_ON); il corpo cade fra la 347 e la
 * 378, cioe' fra `ilitek_tddi_wq_ctrl` (ultimo log 346) e
 * `ilitek_tddi_sleep_handler` (primo log 379)
 * ---------------------------------------------------------------------------
 * E' LA TERZA FUNZIONE DI SORGENTE SENZA SIMBOLO DI QUESTO FILE, e a
 * differenza di `read_power_status` e `ilitek_update_tp_module_info` NON la
 * denuncia il `__func__` -- non ha nessuna `printk` -- ma il `__LINE__`.
 *
 * COSA E' MISURATO. I due `warn_slowpath_null` di 0xffffff8008a55214 e
 * 0xffffff8008a5522c ricevono come secondo argomento 356 e 357
 * ("52802c81 mov"@0xffffff8008a55210, "52802ca1 mov"@0xffffff8008a55228) e
 * come primo il `__FILE__` di `ilitek_main.c`. Le sei `printk` proprie di
 * `ilitek_tddi_init` -- quelle il cui `__func__` e'
 * "ilitek_tddi_init"@0xffffff800923ed47 -- portano 870, 910, 915, 932, 948 e
 * 957 ("52806cc2 mov"@0xffffff8008a54e74 e le altre cinque). Le righe 356 e
 * 357 cadono nel VUOTO fra l'ultimo log di `ilitek_tddi_wq_ctrl` (346) e il
 * primo di `ilitek_tddi_sleep_handler` (379). Un corpo di funzione e'
 * contiguo: se `ilitek_tddi_init` contenesse la riga 356 conterrebbe anche
 * le righe 379..468, che sono di `ilitek_tddi_sleep_handler`. Quindi i due
 * `WARN_ON` non stanno nel corpo di sorgente di `ilitek_tddi_init`, e la
 * funzione che li contiene e' incorporata li' dal compilatore. E' la classe
 * B5 dei criteri (una funzione incorporata fusa nel chiamante), nella forma
 * in cui il `__func__` non puo' denunciarla.
 *
 * COSA NON E' MISURATO, e va letto come scelta:
 *  - IL NOME. Questa funzione non ha simbolo (la mappa non la elenca) e non
 *    ha `printk`, quindi il binario non la nomina in nessun modo:
 *    `ilitek_tddi_wq_init` e' un NOME SCELTO, come `ilitek_bat_status`
 *    (H6). Sbagliarlo non produce nessuna divergenza misurabile, ed e'
 *    proprio per questo che va dichiarato.
 *  - LA FIRMA. Nessun argomento e nessun ritorno: e' cio' che il codice
 *    incorporato permette, non cio' che il binario impone -- l'incorporamento
 *    cancella il confine.
 *  - I SUOI CONFINI ESATTI. Il binario fissa solo che le due righe 356 e 357
 *    (i due `WARN_ON`) stanno dentro una funzione che non e'
 *    `ilitek_tddi_init`. Che ci stiano dentro anche le due `alloc_workqueue`
 *    -- i cui risultati sono proprio cio' che i due `WARN_ON` provano
 *    ("b4001408 cbz"@0xffffff8008a54f88 e "b40014a0 cbz"@0xffffff8008a54f8c
 *    leggono le due variabili appena scritte) -- e i due
 *    `INIT_DELAYED_WORK` che le seguono senza altro codice in mezzo e' la
 *    lettura piu' semplice, non una misura. Il codice generato e' lo stesso
 *    in ogni caso, perche' la funzione viene comunque incorporata.
 *
 * CIO' CHE LA SCELTA E' COSTRETTA A RISPETTARE e' l'intervallo di righe: il
 * corpo, prototipo e graffe comprese, deve stare fra la 347 e la 378, e le
 * due `WARN_ON` sulle righe 356 e 357. Trentadue righe bastano.
 */
static void ilitek_tddi_wq_init(void)
{
	/* "esd_check"@0xffffff80091ff832, "321d03e1 orr"@0xffffff8008a54f44
	 * (flag = 8 = WQ_MEM_RECLAIM), "2a1f03e2 mov"@0xffffff8008a54f48
	 * (max_active = 0), "aa1f03e3 mov"@0xffffff8008a54f4c e
	 * "aa1f03e4 mov"@0xffffff8008a54f50 (key e lock_name NULL, cioe' il
	 * ramo senza LOCKDEP della macro), "97d9f063 bl"@0xffffff8008a54f54.
	 * Il primo argomento e' il letterale DIRETTO, non "%s": nessun
	 * `create_workqueue`/`create_singlethread_workqueue`, che passerebbero
	 * "%s" e flag diversi. */
	ilitek_wq_esd = alloc_workqueue("esd_check", WQ_MEM_RECLAIM, 0);
	/* "bat_check"@0xffffff800923efd1, "97d9f05a bl"@0xffffff8008a54f78 */
	ilitek_wq_bat = alloc_workqueue("bat_check", WQ_MEM_RECLAIM, 0);

	/* "b4001408 cbz"@0xffffff8008a54f88 verso il primo
	 * `warn_slowpath_null` (riga 356) e
	 * "b40014a0 cbz"@0xffffff8008a54f8c verso il secondo (riga 357).
	 * Dopo il WARN l'esecuzione PROSEGUE: "b5ffeba0 cbnz"@0xffffff8008a5521c
	 * e "17ffff58 b"@0xffffff8008a55230 rientrano entrambi a
	 * 0xffffff8008a54f90. Nessun ritorno d'errore. */
	WARN_ON(!ilitek_wq_esd);
	WARN_ON(!ilitek_wq_bat);

	/* "9125a273 add"@0xffffff8008a54f94 = 0xffffff800a0fc968,
	 * "910d7108 add"@0xffffff8008a54fa0 = ilitek_tddi_wq_esd_check,
	 * "f9000275 str"@0xffffff8008a54fb8 (work.data = 0xfffffffe0),
	 * "a9012269 stp"@0xffffff8008a54fc0 (entry.prev e work.func),
	 * "320b03e1 orr"@0xffffff8008a54fac (#0x200000 = TIMER_IRQSAFE),
	 * "97dbef6a bl"@0xffffff8008a54fc4 verso <init_timer_key>,
	 * "a903ce76 stp"@0xffffff8008a54ff4 (timer.function =
	 * 0xffffff80080cf6bc = delayed_work_timer_fn, timer.data = il blocco
	 * stesso). */
	INIT_DELAYED_WORK(&ilitek_esd_work, ilitek_tddi_wq_esd_check);
	/* "912742f7 add"@0xffffff8008a54fd0 = 0xffffff800a0fc9d0,
	 * "910ef108 add"@0xffffff8008a54fdc = ilitek_tddi_wq_bat_check */
	INIT_DELAYED_WORK(&ilitek_bat_work, ilitek_tddi_wq_bat_check);
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_sleep_handler -- 0xffffff8008a54288, 1188 byte
 * fabbrica: righe 379..468
 * ---------------------------------------------------------------------------
 * TRAPPOLA DI FORMA, MISURATA. `ilitek_tddi_wq_ctrl` compare qui QUATTRO
 * volte come corpo incorporato e DUE volte come chiamata vera:
 *   - incorporata: le vie a 0xffffff8008a54308 (tipo 0, ctrl 0) e
 *     0xffffff8008a54370 (tipo 1, ctrl 0), riconoscibili perche' il `__func__`
 *     dei loro `printk` vale "ilitek_tddi_wq_ctrl"@0xffffff800923e2e6 mentre
 *     l'indirizzo cade dentro `ilitek_tddi_sleep_handler`
 *     ("910b9821 add"@0xffffff8008a5434c e "910b9821 add"@0xffffff8008a543d0);
 *   - chiamata: "97fffe64 bl"@0xffffff8008a546d8 e
 *     "97fffe61 bl"@0xffffff8008a546e4, entrambe verso 0xffffff8008a54068.
 * E' clang che specializza sui due argomenti costanti, non il sorgente che
 * duplica: qui ci sono QUATTRO chiamate, non due chiamate e due copie.
 *
 * LA CONDIZIONE CONDIVISA. Il valore che decide se fermare la scansione e'
 * calcolato PRIMA dello smistamento su `mode` e usato in due rami su quattro.
 * Le righe qui sotto sono LETTERALI -- objdump -d sul gruppo A, colonne e
 * commenti di coda compresi -- e l'unica elisione e' marcata:
 *
 *   ffffff8008a54400:	394a0909 	ldrb	w9, [x8,#642]
 *   ffffff8008a54404:	340005c9 	cbz	w9, ffffff8008a544bc <ilitek_tddi_sleep_handler+0x234>
 *   ffffff8008a54408:	320003f5 	orr	w21, wzr, #0x1
 *   [...44 istruzioni, da 0xffffff8008a5440c a 0xffffff8008a544b8...]
 *   ffffff8008a544bc:	f9401909 	ldr	x9, [x8,#48]
 *   ffffff8008a544c0:	5280806a 	mov	w10, #0x403                 	// #1027
 *   ffffff8008a544c4:	72a0002a 	movk	w10, #0x1, lsl #16
 *   ffffff8008a544c8:	b9403529 	ldr	w9, [x9,#52]
 *   ffffff8008a544cc:	6b0a013f 	cmp	w9, w10
 *   ffffff8008a544d0:	1a9f27f5 	cset	w21, cc
 *
 * Prima della revisione mancavano il suffisso simbolico del `cbz` e il
 * commento di coda `// #1027` del `mov`, tolti senza marcare l'elisione: la
 * regola 1 del progetto chiede l'output letterale, e un listato ripulito e'
 * una bugia sulla forma anche quando le codifiche sono giuste. Le dieci
 * righe SONO invece verificabili cosi' come stanno -- `verificaistruzioni.py`
 * le estrae tutte e dieci nella forma a codifica e ne confronta anche gli
 * operandi; non sono una citazione fuori forma.
 * Il secondo termine dereferenzia `c48` sulla via in cui `c642` e' zero, e
 * quella dereferenza sta PRIMA del `cmp w19,#2` che smista: il sorgente
 * calcola la condizione fuori dallo `switch`. Se stesse dentro i due rami,
 * clang non potrebbe speculare la lettura di `[c48+52]` al di sopra dello
 * smistamento. E' una LETTURA DEL BINARIO, non una scelta di stile.
 *
 * `cset w21, cc` e' il confronto SENZA SEGNO ("1a9f27f5 cset"
 * @0xffffff8008a544d0): il campo a `c48+52` e' un `u32`. E' la versione del
 * core, composta da `ilitek_tddi_ic_get_core_ver` come
 * (maggiore<<16) | (medio<<8) | minore ("53103d29 lsl"@0xffffff8008a59ed0,
 * "33181d49 bfi"@0xffffff8008a59ed4, "2a0b0129 orr"@0xffffff8008a59ed8,
 * "b9003509 str"@0xffffff8008a59edc), quindi 0x10403 e' la 1.4.3.
 *
 * DELTA DI HEADER (non fatto qui): `c48` e' `void *` in `ilitek.h` e la
 * struttura a cui punta non e' ricostruita, quindi la lettura passa per un
 * cast esplicito, come fa gia' `ilitek_tddi_dev_init` col byte a +0 di
 * `struct ilitek_hwif_info`.
 */
int ilitek_tddi_sleep_handler(int mode)
{
	int ret = 0;
	bool stop_sense;

	/* "9101c100 add"@0xffffff8008a542a4, "94107c35 bl"@0xffffff8008a542a8 */
	mutex_lock(&idev->touch_mutex);
	/* "b902d509 str"@0xffffff8008a542b4 */
	WRITE_ONCE(idev->c724, 1);

	/* "35000069 cbnz"@0xffffff8008a542bc (c716),
	 * "34000229 cbz"@0xffffff8008a542c4 (c720) */
	if (READ_ONCE(idev->c716) || READ_ONCE(idev->c720)) {
		/* "\x016ILITEK: (%s, %d): fw upgrade or mp still running,
		 * ignore sleep requst\n"@0xffffff800923e443, riga 379.
		 * Il refuso «requst» e' della fabbrica e si riproduce. */
#line 379
		ILI_INFO("fw upgrade or mp still running, ignore sleep requst\n");
		/* "b902d51f str"@0xffffff8008a542e8 PRIMA dello sblocco */
		WRITE_ONCE(idev->c724, 0);
		/* "94107c38 bl"@0xffffff8008a542ec */
		mutex_unlock(&idev->touch_mutex);
		/* "2a1f03f3 mov"@0xffffff8008a542f0 */
		return 0;
	}

	/* incorporate, vedi il cappello */
	ilitek_tddi_wq_ctrl(0, 0);
	ilitek_tddi_wq_ctrl(1, 0);

	/* "94000745 bl"@0xffffff8008a543dc */
	ilitek_plat_irq_disable();

	/* "\x016ILITEK: (%s, %d): Sleep Mode = %d\n"@0xffffff800923e4a6,
	 * riga 389; l'argomento e' `mode` ("2a1303e3 mov"@0xffffff8008a543f4). */
#line 389
	ILI_INFO("Sleep Mode = %d\n", mode);

	stop_sense = idev->c642 ||
		     *(u32 *)((u8 *)idev->c48 + 52) < 0x10403;

	switch (mode) {
	case 0:
		/* "\x016ILITEK: (%s, %d): TP suspend start\n"
		 * @0xffffff800923e4cb, riga 400 */
#line 400
		ILI_INFO("TP suspend start\n");
		/* "340002d5 cbz"@0xffffff8008a54438 */
		if (stop_sense) {
			/* "sense"@0xffffff80092419c4,
			 * "91271000 add"@0xffffff8008a54440,
			 * "2a1f03e1 mov"@0xffffff8008a54444 (ctrl = 0) */
			if (ilitek_tddi_ic_func_ctrl("sense", 0) < 0)
				/* "\x013ILITEK: (%s, %d): Write sense stop cmd
				 * failed\n"@0xffffff800923e4f1, riga 403 */
#line 403
				ILI_ERR("Write sense stop cmd failed\n");
			/* "52800640 mov"@0xffffff8008a54468 (#50),
			 * "52800281 mov"@0xffffff8008a5446c (#20) */
			if (ilitek_tddi_ic_check_busy(50, 20) < 0)
				/* "\x013ILITEK: (%s, %d): Check busy timeout
				 * during suspend\n"@0xffffff800923e522, riga 406 */
#line 406
				ILI_ERR("Check busy timeout during suspend\n");
		}
		/* "39485108 ldrb"@0xffffff8008a54494,
		 * "34000b88 cbz"@0xffffff8008a54498 */
		if (idev->c532) {
			/* "32000fe0 orr"@0xffffff8008a5449c (#0xf) */
			ilitek_tddi_switch_tp_mode(0x0f);
			/* "b9414d00 ldr"@0xffffff8008a544ac (c332),
			 * "320003e1 orr"@0xffffff8008a544a8 (#1),
			 * "97db9a7d bl"@0xffffff8008a544b0 */
			irq_set_irq_wake(idev->c332, 1);
			/* "94000737 bl"@0xffffff8008a544b4 */
			ilitek_plat_irq_enable();
		} else {
			/* "sleep"@0xffffff800911c431,
			 * "320007e1 orr"@0xffffff8008a54610 (ctrl = 3) */
			if (ilitek_tddi_ic_func_ctrl("sleep", 3) < 0)
				/* "\x013ILITEK: (%s, %d): Write sleep in cmd
				 * failed\n"@0xffffff800923e559, riga 415 */
#line 415
				ILI_ERR("Write sleep in cmd failed\n");
		}
		/* "\x016ILITEK: (%s, %d): TP suspend end\n"
		 * @0xffffff800923e588, riga 417 */
#line 417
		ILI_INFO("TP suspend end\n");
		/* "3909ed09 strb"@0xffffff8008a5469c,
		 * "390a011f strb"@0xffffff8008a546a0 */
		idev->c635 = 1;
		idev->c640 = 0;
		break;

	case 1:
		/* "\x016ILITEK: (%s, %d): TP deep suspend start\n"
		 * @0xffffff800923e5ac, riga 422 */
#line 422
		ILI_INFO("TP deep suspend start\n");
		/* "340002d5 cbz"@0xffffff8008a54560 */
		if (stop_sense) {
			if (ilitek_tddi_ic_func_ctrl("sense", 0) < 0)
				/* stesso letterale della riga 403,
				 * @0xffffff800923e4f1, qui riga 425 */
#line 425
				ILI_ERR("Write sense stop cmd failed\n");
			if (ilitek_tddi_ic_check_busy(50, 20) < 0)
				/* "\x013ILITEK: (%s, %d): Check busy timeout
				 * during deep suspend\n"@0xffffff800923e5d7,
				 * riga 428 */
#line 428
				ILI_ERR("Check busy timeout during deep suspend\n");
		}
		/* "39485108 ldrb"@0xffffff8008a545bc,
		 * "34000468 cbz"@0xffffff8008a545c0 */
		if (idev->c532) {
			ilitek_tddi_switch_tp_mode(0x0f);
			irq_set_irq_wake(idev->c332, 1);
			ilitek_plat_irq_enable();
		} else {
			if (ilitek_tddi_ic_func_ctrl("sleep", 3) < 0)
				/* "\x013ILITEK: (%s, %d): Write deep sleep in
				 * cmd failed\n"@0xffffff800923e613, riga 437 */
#line 437
				ILI_ERR("Write deep sleep in cmd failed\n");
		}
		/* "\x016ILITEK: (%s, %d): TP deep suspend end\n"
		 * @0xffffff800923e647, riga 439 */
#line 439
		ILI_INFO("TP deep suspend end\n");
		idev->c635 = 1;
		idev->c640 = 0;
		break;

	case 2:
		/* "3949e908 ldrb"@0xffffff8008a544dc,
		 * "35001168 cbnz"@0xffffff8008a544e0: se il byte e' acceso si
		 * salta TUTTO il corpo del risveglio, ma NON la riabilitazione
		 * dell'interruzione. */
		if (!idev->c634) {
			/* "\x016ILITEK: (%s, %d): TP resume start\n"
			 * @0xffffff800923e670, riga 445 */
#line 445
			ILI_INFO("TP resume start\n");
			/* "39485109 ldrb"@0xffffff8008a54500,
			 * "340000a9 cbz"@0xffffff8008a54504 */
			if (idev->c532)
				/* "2a1f03e1 mov"@0xffffff8008a5450c (0) */
				irq_set_irq_wake(idev->c332, 0);
			/* "b901411f str"@0xffffff8008a5451c */
			idev->c320 = 0;
			/* "7100053f cmp"@0xffffff8008a54520 con #1 */
			if (idev->c620 == 1) {
				/* "97fffd54 bl"@0xffffff8008a54528 */
				if (ilitek_tddi_fw_upgrade_handler() < 0)
					/* "\x013ILITEK: (%s, %d): FW upgrade
					 * failed during resume\n"
					 * @0xffffff800923e695, riga 454 */
#line 454
					ILI_ERR("FW upgrade failed during resume\n");
			} else {
				/* "b9426500 ldr"@0xffffff8008a546a8 (c612),
				 * "97fffd27 bl"@0xffffff8008a546ac */
				if (ilitek_tddi_reset_ctrl(idev->c612) < 0)
					/* "\x013ILITEK: (%s, %d): TP Reset
					 * failed during resume\n"
					 * @0xffffff800923e6ca, riga 457 */
#line 457
					ILI_ERR("TP Reset failed during resume\n");
			}
			/* le DUE chiamate vere, non incorporate:
			 * "97fffe64 bl"@0xffffff8008a546d8 e
			 * "97fffe61 bl"@0xffffff8008a546e4 */
			ilitek_tddi_wq_ctrl(0, 1);
			ilitek_tddi_wq_ctrl(1, 1);
			/* "3909ed1f strb"@0xffffff8008a54700,
			 * "390a0113 strb"@0xffffff8008a54704: cadono PRIMA di
			 * "97db7b73 bl"@0xffffff8008a54708 verso <printk>. */
			idev->c635 = 0;
			idev->c640 = 1;
			/* "\x016ILITEK: (%s, %d): TP resume end\n"
			 * @0xffffff800923e6fd, riga 463 */
#line 463
			ILI_INFO("TP resume end\n");
		}
		/* "940006a1 bl"@0xffffff8008a5470c */
		ilitek_plat_irq_enable();
		break;

	default:
		/* "\x013ILITEK: (%s, %d): Unknown sleep mode, %d\n"
		 * @0xffffff800923e720, riga 468 */
#line 468
		ILI_ERR("Unknown sleep mode, %d\n", mode);
		/* "128002b3 mov"@0xffffff8008a54600 = -22 */
		ret = -EINVAL;
		break;
	}

	/* "94002130 bl"@0xffffff8008a54714 */
	ilitek_tddi_touch_release_all_point();
	/* "b902d51f str"@0xffffff8008a54720 */
	WRITE_ONCE(idev->c724, 0);
	/* "94107b2a bl"@0xffffff8008a54724 */
	mutex_unlock(&idev->touch_mutex);
	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_fw_upgrade_handler -- 0xffffff8008a53a78, 208 byte
 * fabbrica: righe 488..497
 * ---------------------------------------------------------------------------
 * Da notare, perche' una riscrittura «pulita» lo normalizzerebbe in silenzio:
 * "FW upgrade fail" e' di livello KERN_INFO e non KERN_ERR, benche' sia il
 * messaggio di fallimento. Il livello si legge dal primo byte del formato,
 * \x016 ("\x016ILITEK: (%s, %d): FW upgrade fail\n"@0xffffff800923e74c).
 */
int ilitek_tddi_fw_upgrade_handler(void)
{
	int ret;

	/* "b902cd09 str"@0xffffff8008a53a90 */
	WRITE_ONCE(idev->c716, 1);
	/* "b901bd1f str"@0xffffff8008a53a98 */
	idev->c444 = 0;

	/* "b941c100 ldr"@0xffffff8008a53a94, "94005291 bl"@0xffffff8008a53a9c */
	ret = ilitek_tddi_fw_upgrade(idev->c448);
	/* "34000120 cbz"@0xffffff8008a53aa4: il confronto e' con zero, non
	 * col segno. */
	if (ret) {
#line 488
		ILI_INFO("FW upgrade fail\n");	/* riga 488 */
		/* "12800009 mov"@0xffffff8008a53ac0: #0xffffffff = -1 */
		idev->c444 = -1;
	} else {
		/* "\x016ILITEK: (%s, %d): FW upgrade pass\n"
		 * @0xffffff800923e790, riga 491 */
#line 491
		ILI_INFO("FW upgrade pass\n");
		/* "52800c89 mov"@0xffffff8008a53ae0: #0x64 = 100 */
		idev->c444 = 100;
	}

	/* "3943810a ldrb"@0xffffff8008a53ae8, "3500022a cbnz"@0xffffff8008a53af0 */
	if (!idev->c224) {
		/* L'ORDINE E' MISURATO, non di comodo: "39038109 strb"
		 * @0xffffff8008a53b0c cade PRIMA di "97db7e71 bl"
		 * @0xffffff8008a53b10 verso <printk> e riusa la `x8` caricata a
		 * "f9452288 ldr"@0xffffff8008a53ae4. Scrivendo il campo dopo la
		 * `printk` clang deve ricaricare il globale, e la funzione
		 * cresce di quattro byte (misurato: 212 contro 208). */
		idev->c224 = 1;
		/* "\x016ILITEK: (%s, %d): Registre touch to input subsystem\n"
		 * @0xffffff800923e7b5, riga 497. Il refuso «Registre» e' di
		 * fabbrica. */
#line 497
		ILI_INFO("Registre touch to input subsystem\n");
		/* "940008a8 bl"@0xffffff8008a53b14 */
		ilitek_plat_input_register();
		/* "2a1f03e0 mov"@0xffffff8008a53b1c + "320003e1 orr"@0xffffff8008a53b18
		 * -> (0, 1); "94000152 bl"@0xffffff8008a53b20 */
		ilitek_tddi_wq_ctrl(0, 1);
		/* "320003e0 orr"@0xffffff8008a53b24 + "320003e1 orr"@0xffffff8008a53b28
		 * -> (1, 1); "9400014f bl"@0xffffff8008a53b2c */
		ilitek_tddi_wq_ctrl(1, 1);
	}

	/* "b902cd1f str"@0xffffff8008a53b34 */
	WRITE_ONCE(idev->c716, 0);
	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_set_tp_data_len -- 0xffffff8008a53c80, 600 byte
 * fabbrica: righe 551..580
 * ---------------------------------------------------------------------------
 * Lo smistamento e' una TABELLA DI SALTO in .rodata, non una catena di
 * confronti, e la tabella e' stata letta:
 *
 *   "f000294a adrp"@0xffffff8008a53cb8 + "9129394a add"@0xffffff8008a53cc0
 *      -> 0xffffff8008f7ea4e
 *   "100000eb adr"@0xffffff8008a53cc8   -> base dei bersagli 0xffffff8008a53ce4
 *   "3869694c ldrb"@0xffffff8008a53ccc  -> tabella[formato]
 *   "8b0c096b add"@0xffffff8008a53cd0   -> base + 4 * tabella[formato]
 *
 * e i byte a 0xffffff8008f7ea4e sono 3d 00 12 15 38 3b 3d 00, cioe':
 *
 *   formato 0 -> +244 = 0xffffff8008a53dd8   (coda, valori iniziali)
 *   formato 1 -> +0   = 0xffffff8008a53ce4   (lunghezza calcolata)
 *   formato 2 -> +72  = 0xffffff8008a53d2c
 *   formato 3 -> +84  = 0xffffff8008a53d38   (gesti)
 *   formato 4 -> +224 = 0xffffff8008a53dc4
 *   formato 5 -> +236 = 0xffffff8008a53dd0
 *   formato 6 -> +244 = 0xffffff8008a53dd8   (coda, come 0)
 *   formato 7 -> +0   = 0xffffff8008a53ce4   (come 1)
 *
 * I formati 0 e 6 cadono sulla coda senza toccare niente: restano i valori
 * iniziali, `len = 43` ("52800575 mov"@0xffffff8008a53cdc, #0x2b) e
 * `data_fmt = 0` ("2a1f03f6 mov"@0xffffff8008a53cd8) -- cioe' il ramo di
 * default. I formati 1 e 7 condividono il bersaglio.
 *
 * Il protettore di pila c'e' ("97d968e3 bl"@0xffffff8008a53ed4 verso
 * <__stack_chk_fail>) perche' il buffer di due byte sta sulla pila.
 */
int ilitek_set_tp_data_len(int format)
{
	u8 cmd[2] = {0};	/* "79000bff strh"@0xffffff8008a53ca8: azzerato */
	int mode, len = 43, data_fmt = 0, ret = 0;

	/* "b9414114 ldr"@0xffffff8008a53cc4 */
	mode = idev->c320;

	/*
	 * IL CONTROLLO DI INTERVALLO E' IL `default` DELLO SWITCH, non un `if`
	 * che lo precede, e la differenza e' misurata. La fabbrica ha
	 * "71001c1f cmp"@0xffffff8008a53ca0 con #0x7 piu'
	 * "54000308 b.hi"@0xffffff8008a53cac verso il messaggio d'errore, e poi
	 * una tabella di OTTO voci indicizzata da `format` grezzo
	 * ("3869694c ldrb"@0xffffff8008a53ccc, con x9 = w0 senza sottrazioni):
	 * e' la forma che clang da' a uno `switch` i cui casi coprono 0..7 e il
	 * cui `default` e' l'errore.
	 * Scritto invece come `if ((unsigned)format > 7) { ...; return -1; }`
	 * davanti allo switch, clang restringe l'intervallo a 1..7 e aggiunge
	 * un "51000409 sub" e un secondo confronto: 604 byte contro 600
	 * (misurato, in entrambe le varianti con i casi 0 e 6 elencati).
	 * Anche i numeri di riga lo dicono: il messaggio d'errore e' alla riga
	 * 557, DOPO le 551 e 553 del caso 3 e PRIMA della 564 che segue lo
	 * switch -- cioe' e' dentro lo switch, in coda.
	 */
	switch (format) {
	case 0:
	case 6:
		/* La tabella manda questi due direttamente alla coda: restano
		 * i valori iniziali, `len = 43` e `data_fmt = 0`. */
		break;

	case 1:
	case 7:
		/* "321f03f6 orr"@0xffffff8008a53cf4: #0x2 */
		data_fmt = 2;
		/* "3943e109 ldrb"@0xffffff8008a53ce4 (c248),
		 * "3943e50a ldrb"@0xffffff8008a53ce8 (c249),
		 * "3943e90b ldrb"@0xffffff8008a53cec (c250),
		 * "3943ed0c ldrb"@0xffffff8008a53cf0 (c251),
		 * "1b092d49 madd"@0xffffff8008a53cf8 -> c249*c248 + c250,
		 * "0b0c0129 add"@0xffffff8008a53cfc -> + c251,
		 * "531f7929 lsl"@0xffffff8008a53d00 -> * 2,
		 * "1100e135 add"@0xffffff8008a53d04 -> + 0x38 = 56 */
		len = (idev->c249 * idev->c248 + idev->c250 + idev->c251) * 2 + 56;
		break;

	case 2:
		/* "321e03f6 orr"@0xffffff8008a53d2c: #0x4 */
		data_fmt = 4;
		/* "321e0ff5 orr"@0xffffff8008a53d30: #0x3c = 60 */
		len = 60;
		break;

	case 3:
		/* "b9421d03 ldr"@0xffffff8008a53d38 (c540),
		 * "b9421908 ldr"@0xffffff8008a53d3c (c536),
		 * "7100111f cmp"@0xffffff8008a53d54 con #0x4,
		 * "1a890148 csel"@0xffffff8008a53d60 -> 174 : 47,
		 * "1a8b0189 csel"@0xffffff8008a53d64 -> 170 : 8,
		 * "7100047f cmp"@0xffffff8008a53d68 con #0x1,
		 * "1a890115 csel"@0xffffff8008a53d78 sceglie fra le due coppie.
		 * Le quattro costanti: "528005e9 mov"@0xffffff8008a53d44 (47),
		 * "528015ca mov"@0xffffff8008a53d48 (174),
		 * "321d03eb orr"@0xffffff8008a53d4c (8),
		 * "5280154c mov"@0xffffff8008a53d50 (170). */
		if (idev->c540 == 1)
			len = (idev->c536 == 4) ? 174 : 47;
		else
			len = (idev->c536 == 4) ? 170 : 8;
		/* "\x016ILITEK: (%s, %d): Gesture demo mode control = %d\n"
		 * @0xffffff800923e7ec, riga 551 */
#line 551
		ILI_INFO("Gesture demo mode control = %d\n", idev->c540);
		/* "gesture_demo_en"@0xffffff800923e837,
		 * "b9421d01 ldr"@0xffffff8008a53d8c (l'argomento e' c540),
		 * "9400103e bl"@0xffffff8008a53d90 */
		ilitek_tddi_ic_func_ctrl("gesture_demo_en", idev->c540);
		/* "\x016ILITEK: (%s, %d): knock_en setting\n"
		 * @0xffffff800923e847, riga 553 */
#line 553
		ILI_INFO("knock_en setting\n");
		/* "knock_en"@0xffffff800923e86d,
		 * "321d03e1 orr"@0xffffff8008a53db0: #0x8,
		 * "94001035 bl"@0xffffff8008a53db4 */
		ilitek_tddi_ic_func_ctrl("knock_en", 8);
		break;

	case 4:
		/* "321f03f6 orr"@0xffffff8008a53dc4: #0x2 */
		data_fmt = 2;
		/* "52801555 mov"@0xffffff8008a53dc8: #0xaa = 170 */
		len = 170;
		break;

	case 5:
		/* "320003f6 orr"@0xffffff8008a53dd0: #0x1 */
		data_fmt = 1;
		/* "321d03f5 orr"@0xffffff8008a53dd4: #0x8 */
		len = 8;
		break;

	default:
		/* "\x013ILITEK: (%s, %d): Unknow TP data format\n"
		 * @0xffffff800923e876, riga 557. Il refuso «Unknow» e' di
		 * fabbrica.
		 * Il confronto che ci porta e' SENZA SEGNO
		 * ("54000308 b.hi"@0xffffff8008a53cac), quindi un `format`
		 * negativo cade qui e non nella tabella. */
#line 557
		ILI_ERR("Unknow TP data format\n");
		/* "12800014 mov"@0xffffff8008a53d24: #0xffffffff = -1 */
		return -1;
	}

	/* L'ORDINE E' MISURATO: "b9014513 str"@0xffffff8008a53df8 e
	 * "b9014915 str"@0xffffff8008a53dfc cadono PRIMA di
	 * "97db7db5 bl"@0xffffff8008a53e00 verso <printk>, e clang non puo'
	 * spostare uno store attraverso una chiamata opaca in nessuno dei due
	 * versi -- l'ordine emesso e' quindi l'ordine del sorgente. Scritti
	 * dopo la `printk`, il globale resta vivo qui e va ricaricato piu'
	 * avanti, al sito di +776 dove la fabbrica ha
	 * "f94522e8 ldr"@0xffffff8008a53e50: 596 byte contro 600 (misurato). */
	idev->c324 = format;
	idev->c328 = len;

	/* "\x016ILITEK: (%s, %d): TP mode = %d, format = %d, len = %d\n"
	 * @0xffffff800923e8a1, riga 564 */
#line 564
	ILI_INFO("TP mode = %d, format = %d, len = %d\n", mode, format, len);

	/* "32000268 orr"@0xffffff8008a53e04 (format | 1),
	 * "71001d1f cmp"@0xffffff8008a53e08 con #0x7,
	 * "54000220 b.eq"@0xffffff8008a53e0c: clang ha fuso `format == 6 ||
	 * format == 7` in `(format | 1) == 7`.
	 * "34000214 cbz"@0xffffff8008a53e10 aggiunge `mode == 0`. */
	if (format == 6 || format == 7 || mode == 0) {
		/* "321c0fe9 orr"@0xffffff8008a53e54: #0xf0,
		 * "390013e9 strb"@0xffffff8008a53e58,
		 * "390017f6 strb"@0xffffff8008a53e5c */
		cmd[0] = 0xf0;
		cmd[1] = data_fmt;
		/* "f9418508 ldr"@0xffffff8008a53e60 (+776 = `ilitek_i2c_write`),
		 * "910013e0 add"@0xffffff8008a53e64 (x0 = cmd),
		 * "321f03e1 orr"@0xffffff8008a53e68 (#0x2),
		 * "d63f0100 blr"@0xffffff8008a53e6c */
		ret = idev->c776(cmd, 2);
		if (ret < 0) {
			/* "\x013ILITEK: (%s, %d): switch to format %d failed\n"
			 * @0xffffff800923e8da, riga 574 */
#line 574
			ILI_ERR("switch to format %d failed\n", format);
			/* "2a1f03e0 mov"@0xffffff8008a53e94 (#0),
			 * "97fffe7d bl"@0xffffff8008a53e98 */
			ilitek_tddi_switch_tp_mode(0);
		}
	/* "71003e9f cmp"@0xffffff8008a53e14 con #0xf,
	 * "54000441 b.ne"@0xffffff8008a53e18 */
	} else if (mode == 15) {
		/* "lpwg"@0xffffff800923e90a,
		 * "2a1603e1 mov"@0xffffff8008a53e24 (l'argomento e' data_fmt),
		 * "94001018 bl"@0xffffff8008a53e28 */
		ret = ilitek_tddi_ic_func_ctrl("lpwg", data_fmt);
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): write gesture mode failed\n"
			 * @0xffffff800923e90f, riga 580 */
#line 580
			ILI_ERR("write gesture mode failed\n");
	}
	/* "2a1f03f4 mov"@0xffffff8008a53ea0: negli altri casi il ritorno e'
	 * zero, non il valore precedente. */

	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_report_handler -- 0xffffff8008a5472c, 1820 byte
 * fabbrica: righe 598..696
 * ---------------------------------------------------------------------------
 * E' la piu' grande del gruppo, e contiene TRE cose che la dimensione da sola
 * non spiegherebbe.
 *
 * 1. QUARANTA `bl __const_udelay` CONSECUTIVI, da 0xffffff8008a548b4 a
 *    0xffffff8008a54a93: 480 byte, il 26,4% della funzione. Ognuno e'
 *    ("52912b00 mov"@0xffffff8008a548b4 + "72a00820 movk"@0xffffff8008a548b8 +
 *    "94100c6e bl"@0xffffff8008a548bc), cioe' w0 = 0x418958 = 4295000 =
 *    1000 * 0x10c7. In `arch/arm64/include/asm/delay.h` `udelay(n)` con `n`
 *    costante e' `__const_udelay((n) * 0x10c7ul)`, quindi ogni chiamata e'
 *    `udelay(1000)`; e `mdelay(n)` con n > MAX_UDELAY_MS e'
 *    `{unsigned long __ms = (n); while (__ms--) udelay(1000);}`
 *    (include/linux/delay.h), che clang srotola perche' il conteggio e'
 *    costante. QUARANTA giri = `mdelay(40)`. Cio' che il binario mostra sono
 *    le quaranta chiamate; la forma sorgente e' una sola riga.
 *
 * 2. `ilitek_tddi_wq_ctrl` INCORPORATA DUE VOLTE e CHIAMATA DUE VOLTE, come
 *    in `ilitek_tddi_sleep_handler`: le copie stanno a 0xffffff8008a547d0
 *    (tipo 0, ctrl 0) e 0xffffff8008a54834 (tipo 1, ctrl 0) e si riconoscono
 *    dal `__func__` "ilitek_tddi_wq_ctrl"@0xffffff800923e2e6
 *    ("910b9821 add"@0xffffff8008a54810); le chiamate vere sono
 *    "97fffd42 bl"@0xffffff8008a54b60 e "97fffd3f bl"@0xffffff8008a54b6c.
 *
 * 3. `ilitek_tddi_gesture_recovery` INCORPORATA: il blocco
 *    0xffffff8008a54d64..0xffffff8008a54dc8 ha il `__func__`
 *    "ilitek_tddi_gesture_recovery"@0xffffff800923e232
 *    ("9108c821 add"@0xffffff8008a54d8c) e la riga 198, cioe' quelle della
 *    funzione che sta a 0xffffff8008a53ed8. Anche qui: la CHIAMATA, non una
 *    copia del corpo.
 *
 * IL SALVATAGGIO DI `ilitek_dbg_en` E' LETTO IN CIMA. La sola scrittura di
 * ripristino e' "39256296 strb"@0xffffff8008a54d44, e il valore ripristinato
 * sta in w22, caricato da "39656276 ldrb"@0xffffff8008a54750 -- cioe' PRIMA
 * di qualunque chiamata, in mezzo alle condizioni della guardia d'ingresso.
 * Non e' un sollevamento che clang possa fare da solo dal punto d'uso: fra i
 * due punti ci sono decine di `bl printk`, e `ilitek_dbg_en` non e' `static`
 * (lo leggono tutti e sette i gruppi che hanno codice), quindi il compilatore
 * non puo' provare che nessuna di quelle chiamate lo cambi. La lettura sta
 * nel sorgente. Cio' che resta indeciso e' la RIGA esatta: fra la fine della
 * guardia e la prima chiamata non ci sono barriere, quindi clang puo'
 * spostarla in su fino a dove la si vede. E' una scelta, ed e' dichiarata.
 */
void ilitek_tddi_report_handler(void)
{
	int len, ret, pid;
	u8 checksum, dbg_en_prima;
	u8 *buf;

	/* "39484d09 ldrb"@0xffffff8008a54744 (c531),
	 * "340001a9 cbz"@0xffffff8008a54748, poi quattro `cbnz` e un `cbz`:
	 * "35000129 cbnz"@0xffffff8008a54758 (c708),
	 * "350000e9 cbnz"@0xffffff8008a54760 (c716),
	 * "350000a9 cbnz"@0xffffff8008a54768 (c728),
	 * "35000069 cbnz"@0xffffff8008a54770 (c720),
	 * "34000169 cbz"@0xffffff8008a54778 (c724). */
	/* "39656276 ldrb"@0xffffff8008a54750: la lettura cade FRA la prima e la
	 * seconda condizione della guardia qui sotto, dove clang l'ha
	 * sollevata; nel sorgente sta prima della guardia. */
	dbg_en_prima = ilitek_dbg_en;

	if (!idev->c531 || READ_ONCE(idev->c708) || READ_ONCE(idev->c716) ||
	    idev->c728 || READ_ONCE(idev->c720) || READ_ONCE(idev->c724)) {
		/* "\x016ILITEK: (%s, %d): ignore report request\n"
		 * @0xffffff800923e93e, riga 598 */
#line 598
		ILI_INFO("ignore report request\n");
		return;
	}

	/* "3949e109 ldrb"@0xffffff8008a547a4,
	 * "34000149 cbz"@0xffffff8008a547a8 */
	if (idev->c632) {
		/* "\x016ILITEK: (%s, %d): ignore int triggered by recovery\n"
		 * @0xffffff800923e984, riga 603 */
#line 603
		ILI_INFO("ignore int triggered by recovery\n");
		/* "3909e11f strb"@0xffffff8008a547c8 */
		idev->c632 = 0;
		return;
	}

	/* incorporate, vedi il cappello */
	ilitek_tddi_wq_ctrl(0, 0);
	ilitek_tddi_wq_ctrl(1, 0);

	/* "b9414109 ldr"@0xffffff8008a548a0 (c320),
	 * "71003d3f cmp"@0xffffff8008a548a4 con #0xf */
	if (idev->c320 == 0x0f) {
		/* "f9401100 ldr"@0xffffff8008a548ac (c32),
		 * "97eab22a bl"@0xffffff8008a548b0 */
		__pm_stay_awake(idev->c32);
		/* le quaranta `bl __const_udelay`, vedi il cappello */
		mdelay(40);
	}

	/* "b9414913 ldr"@0xffffff8008a54a9c */
	len = idev->c328;
	/* protetta: "39656269 ldrb"@0xffffff8008a54a98.
	 * "\x016ILITEK: (%s, %d): Packget length = %d\n"
	 * @0xffffff800923e9ba, riga 618. Il refuso «Packget» e' della
	 * fabbrica e si riproduce. */
#line 618
	ILI_DBG("Packget length = %d\n", len);

	/* "34000373 cbz"@0xffffff8008a54ac0 e
	 * "7120067f cmp"@0xffffff8008a54ac4 + "5400032a b.ge"@0xffffff8008a54ac8:
	 * il primo termine e' un confronto con ZERO, non con uno -- un `len`
	 * NEGATIVO passa entrambi i controlli. E' cio' che il binario fa. */
	if (len == 0 || len > 2048) {
		/* "\x013ILITEK: (%s, %d): Length of packet (%d) is invaild\n"
		 * @0xffffff800923e9e3, riga 621 */
#line 621
		ILI_ERR("Length of packet (%d) is invaild\n", len);
		goto out;
	}

	/*
	 * "321503e2 orr"@0xffffff8008a54ad0 (#0x800 = 2048),
	 * "94100d49 bl"@0xffffff8008a54adc verso 0xffffff8008e58000.
	 *
	 * E' `__memset`, NON `memset`, E LA DIFFERENZA E' MISURATA. I due nomi
	 * stanno allo STESSO indirizzo -- `oracolo/stock.map` ha
	 * `ffffff8008e58000 T __memset`, `T __pi_memset` e `W memset` -- quindi
	 * il bersaglio della `bl` non li distingue. Li distingue l'istruzione
	 * DOPO: "f94522a8 ldr"@0xffffff8008a54ae0 RICARICA il globale `idev`.
	 * Con `memset` clang lo modella come builtin, sa che scrive solo nella
	 * destinazione e la ricarica sparisce; con `__memset`, che
	 * `arch/arm64/include/asm/string.h` dichiara come una `extern` qualsiasi,
	 * la chiamata e' opaca e la ricarica c'e'. Misurato col compilatore DI
	 * FABBRICA (clang r353983c): con `memset` 1816 byte e 454 istruzioni,
	 * con `__memset` 1820 byte e 455 istruzioni, cioe' esattamente la
	 * fabbrica.
	 *
	 * LIMITE DICHIARATO: quello che la misura prova e' che la chiamata NON
	 * e' il builtin. Che il sorgente scriva `__memset` e' UNA delle due
	 * letture; l'altra e' che quel file di fabbrica fosse compilato con
	 * `-fno-builtin-memset`. Le due danno lo stesso codice e il binario non
	 * le separa.
	 */
	__memset(idev->c264, 0, 2048);

	/* "f9418909 ldr"@0xffffff8008a54ae8 (+784),
	 * "d63f0120 blr"@0xffffff8008a54af0 */
	ret = idev->c784(idev->c264, len);
	/* "37f804a0 tbnz"@0xffffff8008a54af8 */
	if (ret < 0) {
		/* "\x013ILITEK: (%s, %d): Read report packet failed, ret = %d\n"
		 * @0xffffff800923ea19, riga 629 */
#line 629
		ILI_ERR("Read report packet failed, ret = %d\n", ret);
		/* "31000a7f cmn"@0xffffff8008a54ba8: confronto con -2 */
		if (ret == -2) {
			/* "940011b2 bl"@0xffffff8008a54bb0 */
			ilitek_tddi_ic_get_pc_counter_forwdt();
			/* "71003d3f cmp"@0xffffff8008a54bbc (c320 con 0xf),
			 * "34000069 cbz"@0xffffff8008a54bc8 (c532),
			 * "34000be8 cbz"@0xffffff8008a54bd0 (c637): il terzo
			 * termine e' NEGATO -- il salto al ramo del gesto e'
			 * un `cbz`, quindi ci si va quando c637 vale ZERO. */
			if (idev->c320 == 0x0f && idev->c532 && !idev->c637) {
				/* "\x013ILITEK: (%s, %d): Gesture failed,
				 * doing gesture recovery\n"
				 * @0xffffff800923ea52, riga 633 */
#line 633
				ILI_ERR("Gesture failed, doing gesture recovery\n");
				/* incorporata, vedi il cappello */
				if (ilitek_tddi_gesture_recovery() < 0)
					/* "\x013ILITEK: (%s, %d): Failed to
					 * recover gesture\n"
					 * @0xffffff800923ea8e, riga 635 */
#line 635
					ILI_ERR("Failed to recover gesture\n");
				/* "3909e109 strb"@0xffffff8008a54bf8
				 *
				 * L'ASSEGNAMENTO E' DENTRO ENTRAMBI I RAMI, E
				 * IL BINARIO LO DISCRIMINA. La stesura
				 * precedente lo aveva UNA volta sola, dopo
				 * l'`if/else`: stessa dimensione (1820 byte)
				 * e stesso numero di istruzioni (455), ma un
				 * bersaglio di salto diverso.
				 * Il blocco comune di fabbrica e'
				 *   0x4c4  ldr  x8, [x21,#2624]   (ricarica idev)
				 *   0x4c8  orr  w9, wzr, #0x1
				 *   0x4cc  strb w9, [x8,#632]
				 *   0x4d0  b    0x41c
				 * e il salto che vi arriva dal ramo del gesto
				 * e' "36fff153 tbz"@0xffffff8008a54dcc, che
				 * punta a +0x4c8 -- cioe' SCAVALCA la
				 * ricarica, perche' su quel cammino x8 e' gia'
				 * idev (caricato da "f94522a8 ldr"@0xffffff8008a54dc4
				 * per lo `str wzr,[x8,#736]` della
				 * `ilitek_tddi_gesture_recovery` incorporata).
				 * Scritto una volta sola dopo l'`if/else` c'e'
				 * UN solo sito sorgente, quindi una sola
				 * lettura di `idev` al punto di confluenza, e
				 * clang faceva puntare quel `tbz` a +0x4c4:
				 * misurato, e' l'unica posizione che restava
				 * inspiegata delle 455. Scritto in entrambi i
				 * rami, i due sotto-alberi si fondono per la
				 * coda e il ramo del gesto entra a +0x4c8: la
				 * posizione sparisce. E' la classe B dei
				 * criteri -- una divergenza che la misura di
				 * dimensione NON denuncia. */
				idev->c632 = 1;
			} else {
				/* "\x013ILITEK: (%s, %d): SPI ACK failed,
				 * doing spi recovery\n"@0xffffff800923eabd,
				 * riga 638 */
#line 638
				ILI_ERR("SPI ACK failed, doing spi recovery\n");
				/* "97fffcdf bl"@0xffffff8008a54bec */
				ilitek_tddi_spi_recovery();
				idev->c632 = 1;
			}
		}
		goto out;
	}

	/* "32007be8 orr"@0xffffff8008a54afc + "6b08027f cmp"@0xffffff8008a54b00
	 * + "54000220 b.eq"@0xffffff8008a54b04: 0x7fffffff e' scritto come
	 * valore grezzo, non tradotto in un nome che il binario non da'. */
	if (ret == 0x7fffffff)
		goto out;

	/* "7120067f cmp"@0xffffff8008a54b08 + "540007ab b.lt"@0xffffff8008a54b0c */
	if (ret > 2048) {
		/* "\x013ILITEK: (%s, %d): Returned length (%d) is invaild\n"
		 * @0xffffff800923eaf5, riga 650 */
#line 650
		ILI_ERR("Returned length (%d) is invaild\n", ret);
		goto out;
	}

	/* "finger report"@0xffffff800923eb2a,
	 * "321d03e1 orr"@0xffffff8008a54c0c (#8),
	 * "2a1f03e3 mov"@0xffffff8008a54c18 (0),
	 * "940017af bl"@0xffffff8008a54c1c */
	ilitek_dump_data(idev->c264, 8, ret, 0, "finger report");

	/* "51000674 sub"@0xffffff8008a54c24 (ret - 1),
	 * "94001c59 bl"@0xffffff8008a54c30,
	 * "12001c03 and"@0xffffff8008a54c38: del ritorno serve il byte basso. */
	checksum = ilitek_calc_packet_checksum(idev->c264, ret - 1);

	/* "3874c904 ldrb"@0xffffff8008a54c40 = [x8, w20, sxtw]: l'indice e'
	 * esteso CON SEGNO, cioe' e' un `int`.
	 * "6b04007f cmp"@0xffffff8008a54c44, "54000060 b.eq"@0xffffff8008a54c48,
	 * poi "3949d929 ldrb"@0xffffff8008a54c4c (c630) e
	 * "34000549 cbz"@0xffffff8008a54c50: il secondo termine e' NEGATO. */
	if (checksum != ((u8 *)idev->c264)[ret - 1] && !idev->c630) {
		/* "\x013ILITEK: (%s, %d): Wrong checksum, checksum = %x,
		 * buf = %x, len = %d\n"@0xffffff800923eb38, riga 661 */
		ILI_ERR("Wrong checksum, checksum = %x, buf = %x, len = %d\n",
#line 661
			checksum, ((u8 *)idev->c264)[ret - 1], ret);
		/* "320003ea orr"@0xffffff8008a54d18 +
		 * "3925610a strb"@0xffffff8008a54d20 */
		ilitek_dbg_en = 1;
		/* "finger report with wrong"@0xffffff800923eb7f,
		 * "94001766 bl"@0xffffff8008a54d40 */
		ilitek_dump_data(idev->c264, 8, ret, 0,
				 "finger report with wrong");
		/* "39256296 strb"@0xffffff8008a54d44: rimette il valore letto
		 * in cima, non una costante. */
		ilitek_dbg_en = dbg_en_prima;
		goto out;
	}

	/* "39400114 ldrb"@0xffffff8008a54c5c */
	pid = ((u8 *)idev->c264)[0];
	/* protetta: "39656129 ldrb"@0xffffff8008a54c58.
	 * "\x016ILITEK: (%s, %d): Packet ID = %x\n"@0xffffff800923eb98,
	 * riga 669 */
#line 669
	ILI_DBG("Packet ID = %x\n", pid);

	/* Il puntatore e' RILETTO dal campo: "f9408508 ldr"@0xffffff8008a54c84
	 * ricarica `idev->c264` dopo la `printk`, cosa che clang non farebbe
	 * se il sorgente lo tenesse in un locale attraverso la chiamata. */
	buf = idev->c264;
	/* "7102de9f cmp"@0xffffff8008a54c88 con #0xb7,
	 * "38403d14 ldrb"@0xffffff8008a54c90 = [x8,#3]! (pre-indice con
	 * riscrittura: il puntatore avanza di 3 e resta avanzato). */
	if (pid == 0xb7) {
		buf += 3;
		pid = buf[0];
	}

	switch (pid) {
	case 0x5a:
		/* "94001e23 bl"@0xffffff8008a54df4 */
		ilitek_tddi_report_ap_mode(buf, ret);
		break;
	case 0x5c:
		/* "93407e61 sxtw"@0xffffff8008a54dfc: il secondo argomento e'
		 * a 64 bit, esteso con segno da un `int`. */
		demo_debug_info_mode(buf, ret);
		break;
	case 0x7a:
		/* "940022d2 bl"@0xffffff8008a54cbc */
		ilitek_tddi_report_i2cuart_mode(buf, ret);
		break;
	case 0xa7:
		/* "94001ff2 bl"@0xffffff8008a54e14 */
		ilitek_tddi_report_debug_mode(buf, ret);
		break;
	case 0xaa:
		/* "940020f2 bl"@0xffffff8008a54e24 */
		ilitek_tddi_report_gesture_mode(buf, ret);
		break;
	case 0xae:
		/* "39400503 ldrb"@0xffffff8008a54cdc = buf[1].
		 * "\x016ILITEK: (%s, %d): gesture fail reason code = 0x%02x"
		 * @0xffffff800923ebbc, riga 690: NON finisce con \n, ed e' un
		 * difetto della fabbrica che si riproduce. */
#line 690
		ILI_INFO("gesture fail reason code = 0x%02x", buf[1]);
		break;
	default:
		/* "\x013ILITEK: (%s, %d): Unknown packet id, %x\n"
		 * @0xffffff800923ebf2, riga 696 */
#line 696
		ILI_ERR("Unknown packet id, %x\n", pid);
		break;
	}

out:
	/* "71003d3f cmp"@0xffffff8008a54b50 (c320 con 0xf),
	 * "54000160 b.eq"@0xffffff8008a54b54 */
	if (idev->c320 != 0x0f) {
		ilitek_tddi_wq_ctrl(0, 1);
		ilitek_tddi_wq_ctrl(1, 1);
	}
	/* il campo e' RILETTO: "b9414109 ldr"@0xffffff8008a54b74 dopo un
	 * secondo "f94522a8 ldr"@0xffffff8008a54b70 su `idev`. Sono due `if`
	 * distinti nel sorgente, non uno con `else`. */
	if (idev->c320 == 0x0f)
		/* "97eaaf43 bl"@0xffffff8008a54b84 */
		__pm_relax(idev->c32);
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_reset_ctrl -- 0xffffff8008a53b48, 312 byte
 * fabbrica: righe 723..739
 * ---------------------------------------------------------------------------
 * D5 del cappello: il flusso e' riprodotto esattamente, la FORMA sorgente
 * no -- non e' unica. Cio' che il binario impone e' che sul ramo `mode == 1`
 * NON girino ne' `c640 = 0` ("390a011f strb"@0xffffff8008a53bb0) ne'
 * `ilitek_tddi_ic_check_otp_prog_mode` ("940013e9 bl"@0xffffff8008a53bb4)
 * ne' `c712 = 0` ("b902c91f str"@0xffffff8008a53c4c), mentre la coda a
 * 0xffffff8008a53c50 gira su tutti i rami. Almeno due scritture C diverse
 * danno questo; qui c'e' quella che salta.
 */
int ilitek_tddi_reset_ctrl(int mode)
{
	int ret = 0;

	/* "b902c509 str"@0xffffff8008a53b64 */
	WRITE_ONCE(idev->c708, 1);

	/* "7100041f cmp"@0xffffff8008a53b60, "54000221 b.ne"@0xffffff8008a53b68 */
	if (mode == 1) {
		/* "\x016ILITEK: (%s, %d): TP IC Code RST \n"
		 * @0xffffff800923ec1d, riga 723. Lo spazio prima del \n e' di
		 * fabbrica. */
#line 723
		ILI_INFO("TP IC Code RST \n");
		/* "94001133 bl"@0xffffff8008a53b84 */
		ret = ilitek_tddi_ic_code_reset();
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): IC Code reset failed\n"
			 * @0xffffff800923ec59, riga 726 */
#line 726
			ILI_ERR("IC Code reset failed\n");
		goto out;
	}

	/* "390a011f strb"@0xffffff8008a53bb0 */
	idev->c640 = 0;
	/* "940013e9 bl"@0xffffff8008a53bb4 */
	ilitek_tddi_ic_check_otp_prog_mode();

	switch (mode) {
	case 0:
		/* "\x016ILITEK: (%s, %d): TP IC whole RST\n"
		 * @0xffffff800923ec83, riga 729 */
#line 729
		ILI_INFO("TP IC whole RST\n");
		/* "94001166 bl"@0xffffff8008a53c00 */
		ret = ilitek_tddi_ic_whole_reset();
		if (ret < 0)
			/* "\x013ILITEK: (%s, %d): IC whole reset failed\n"
			 * @0xffffff800923eca8, riga 732 */
#line 732
			ILI_ERR("IC whole reset failed\n");
		break;
	case 2:
		/* "\x016ILITEK: (%s, %d): TP HW RST\n"@0xffffff800923ecd3,
		 * riga 735 */
#line 735
		ILI_INFO("TP HW RST\n");
		/* "9400084c bl"@0xffffff8008a53bdc */
		ilitek_plat_tp_reset();
		/* "2a1f03f3 mov"@0xffffff8008a53be0: il ritorno e' azzerato
		 * anche se `mode` era 2, non lasciato al valore precedente. */
		ret = 0;
		break;
	default:
		/* "\x013ILITEK: (%s, %d): Unknown reset mode, %d\n"
		 * @0xffffff800923ecf2, riga 739 */
#line 739
		ILI_ERR("Unknown reset mode, %d\n", mode);
		/* "128002b3 mov"@0xffffff8008a53c44: #0xffffffea = -22 */
		ret = -EINVAL;
		break;
	}

	/* "b902c91f str"@0xffffff8008a53c4c */
	WRITE_ONCE(idev->c712, 0);

out:
	/* La coda scrive cinque cose, e due di esse con UNA SOLA istruzione:
	 * "d2c00569 mov"@0xffffff8008a53c54 carica 0x2b00000000 e
	 * "f9000169 str"@0xffffff8008a53c68 lo scrive a `idev + 324`
	 * ("9105110b add"@0xffffff8008a53c60, #0x144 = 324), cioe' c324 = 0 e
	 * c328 = 43 in un colpo solo. Che clang possa fondere i due store e'
	 * anche la prova che nessuno dei due campi e' volatile. */
	idev->c324 = 0;
	idev->c328 = 43;
	/* "3909d91f strb"@0xffffff8008a53c64 */
	idev->c630 = 0;
	/* "b902c51f str"@0xffffff8008a53c6c */
	WRITE_ONCE(idev->c708, 0);
	/* "390a010a strb"@0xffffff8008a53c70 con w10 = 1
	 * ("320003ea orr"@0xffffff8008a53c58) */
	idev->c640 = 1;
	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_update_tp_module_info -- NESSUN SIMBOLO PROPRIO, incorporata in
 * ilitek_tddi_init
 * fabbrica: righe 844 e 859
 * ---------------------------------------------------------------------------
 * Come `read_power_status`: `grep -c " ilitek_update_tp_module_info$"
 * oracolo/stock.map` da' 0, e che esista lo dice il `__func__`
 * "ilitek_update_tp_module_info"@0xffffff800923f17f
 * ("9105fe73 add"@0xffffff8008a55074), portato da due `printk` che stanno
 * dentro `ilitek_tddi_init`.
 *
 * NON C'E' NESSUNA TABELLA DI MODULI, ed e' una MISURA che smentisce cio' che
 * il cappello di questo file temeva. Nell'intervallo del corpo incorporato
 * (0xffffff8008a5506c..0xffffff8008a550fc) non parte nessun salto, e l'unico
 * salto che vi arriva atterra sulla PRIMA istruzione
 * ("36f800e0 tbz"@0xffffff8008a55050, che scavalca la `printk` della riga
 * 915): e' un blocco lineare senza diramazioni. Un solo ramo, quello che
 * stampa «Couldn't find any tp modules» e riempie i campi con costanti.
 *
 * DELTA DI HEADER (non fatto qui): i sei campi a +656..+696 sono `u64` in
 * `ilitek.h` perche' il lotto precedente vedeva solo la larghezza degli
 * store. Qui si vede il CONTENUTO: sono `const char *`, e i primi quattro
 * sono anche i quattro argomenti `%s` della riga 859. Finche' `ilitek.h` non
 * lo dice, la scrittura passa per un cast, che produce gli stessi
 * `adrp`+`add`+`str`.
 */
static void ilitek_update_tp_module_info(void)
{
	/* "\x013ILITEK: (%s, %d): Couldn't find any tp modules, applying
	 * default settings\n"@0xffffff800923f132, riga 844 */
#line 844
	ILI_ERR("Couldn't find any tp modules, applying default settings\n");

	/* "DEF"@0xffffff80090de706, "f9014903 str"@0xffffff8008a550dc */
	idev->c656 = (u64)(uintptr_t)"DEF";
	/* "/sdcard/ILITEK_FW"@0xffffff800923f19c,
	 * "f9014d05 str"@0xffffff8008a550e0 */
	idev->c664 = (u64)(uintptr_t)"/sdcard/ILITEK_FW";
	/* "ILITEK_FW"@0xffffff800923f1a4, "f9015106 str"@0xffffff8008a550e4 */
	idev->c672 = (u64)(uintptr_t)"ILITEK_FW";
	/* "/sdcard/mp.ini"@0xffffff800923f1ae,
	 * "f9015504 str"@0xffffff8008a550e8 */
	idev->c680 = (u64)(uintptr_t)"/sdcard/mp.ini";
	/* "mp.ini"@0xffffff800923f1b6, "f9015909 str"@0xffffff8008a550ec */
	idev->c688 = (u64)(uintptr_t)"mp.ini";
	/* ""@0xffffff800996b2f8 -- venti byte di zeri, cioe' il letterale
	 * vuoto fuso dal linker. "f9015d0a str"@0xffffff8008a550f0 */
	idev->c696 = (u64)(uintptr_t)"";
	/* "5298080b mov"@0xffffff8008a550a4 piu'
	 * "72a0002b movk"@0xffffff8008a550c8 = 0xc040 | (1 << 16) = 0x1c040 =
	 * 114752. "b902890b str"@0xffffff8008a550f4 */
	idev->c648 = 0x1c040;

	/* "\x016ILITEK: (%s, %d): Found %s module: ini path = %s,
	 * fw path = (%s, %s, %d)\n"@0xffffff800923f1bd, riga 859.
	 * L'ORDINE DEGLI ARGOMENTI E' MISURATO, e NON e' quello degli store:
	 * x3 = +656, x4 = +680, x5 = +664, x6 = +672, w7 = +648
	 * ("911c1863 add"@0xffffff8008a550b0, "9106b884 add"@0xffffff8008a550bc,
	 * "910670a5 add"@0xffffff8008a550b4, "910690c6 add"@0xffffff8008a550b8,
	 * "52980807 mov"@0xffffff8008a550ac + "72a00027 movk"@0xffffff8008a550d4). */
	ILI_INFO("Found %s module: ini path = %s, fw path = (%s, %s, %d)\n",
		 (const char *)(uintptr_t)idev->c656,
		 (const char *)(uintptr_t)idev->c680,
		 (const char *)(uintptr_t)idev->c664,
		 (const char *)(uintptr_t)idev->c672,
#line 859
		 idev->c648);

	/* "b902851f str"@0xffffff8008a55100, con `idev` RICARICATO dopo la
	 * `printk` ("f9452288 ldr"@0xffffff8008a550fc): e' uno statement
	 * separato, dopo la stampa. Che stia dentro questa funzione o nel
	 * chiamante il binario non lo dice -- l'incorporamento cancella il
	 * confine -- ed e' una SCELTA. */
	idev->c644 = 0;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_init -- 0xffffff8008a54e48, 1004 byte
 * fabbrica: righe 870, 910, 915, 932, 948, 957 (il simbolo contiene anche le
 * righe 844 e 859 di `ilitek_update_tp_module_info` e le righe 356/357 di
 * `ilitek_tddi_wq_init`, che sono di ALTRE due funzioni di sorgente,
 * incorporate qui)
 * ---------------------------------------------------------------------------
 * E' IL SIMBOLO CHE DA' IL NOME AL FILE, ma non e' la funzione di sorgente
 * che lo fa: i due `warn_slowpath_null` di 0xffffff8008a55214 e
 * 0xffffff8008a5522c portano come `__FILE__` la stringa che sta a
 * 0xffffff800923ef09, cioe' quella riportata per intero in cima a questo file
 * (l'indirizzo e' materializzato da "913c2400 add"@0xffffff8008a5520c), e la
 * riga che portano -- 356 e 357 -- non e' di questo corpo ma di
 * `ilitek_tddi_wq_init` (vedi D16 e il commento sopra quella funzione). Il
 * `__FILE__` resta valido per il nome del file: il nome dell'unita' non
 * cambia per il fatto che il `WARN_ON` stia in un'altra funzione dello stesso
 * file.
 *
 * NON e' citato qui nella forma "testo"@0xINDIRIZZO perche' il testo e'
 * prodotto dal preprocessore e non compare come letterale nel codice:
 * `verificacitazioni.py` lo segnerebbe NON_ANCORATA, e avrebbe ragione. In
 * questo albero `__WARN()` e' ancora `warn_slowpath_null(__FILE__, __LINE__)`
 * -- `arch/arm64/include/asm/bug.h` definisce `__WARN_FLAGS` e poi lo
 * `#undef`, quindi `__WARN_TAINT` non esiste e `include/asm-generic/bug.h`
 * riga 89 vince.
 *
 * LE RIGHE 356 E 357 NON SONO «LE UNICHE DI CUI SI CONOSCA IL NUMERO VERO»,
 * come diceva questo commento prima della revisione: sono due delle 86 righe
 * di fabbrica note, ricostruite una per una dagli immediati `mov w2,#NNN`
 * (e `orr w2,wzr,#NNN`, che e' la stessa cosa scritta altrimenti) davanti a
 * ogni `printk`. Sono annotate sopra ciascuna funzione e messe in fila nella
 * tabella «L'ORDINE DELLE FUNZIONI» del cappello; e' D1 a dire che il file
 * non le riproduce, non che non le conosca.
 *
 * I CINQUE `struct lock_class_key` a 0xffffff800a0fca30..0xa34 NON vanno
 * dichiarati: sono cio' che `mutex_init` e `init_waitqueue_head` generano da
 * sole come terzo argomento ("9128c042 add"@0xffffff8008a54e94 e i quattro
 * successivi, con passo UNO -- un `struct lock_class_key` vuoto). Le stringhe
 * che quelle macro producono sono la prova che i nomi dei campi sono DEL
 * BINARIO, e si leggono con
 * `leggi_stringa.py oracolo/stock.elf 0xffffff800923ed67 0xffffff800923ed7a
 * 0xffffff800923ed8d 0xffffff800923eda5 0xffffff80090e2854`: rispettivamente
 * `&idev->touch_mutex`, `&idev->debug_mutex`, `&idev->debug_read_mutex`,
 * `&(idev->inq)` -- con le parentesi, che il sorgente scrive -- e
 * `&x->wait`, che viene da DENTRO `init_completion`. NON sono citate nella
 * forma "testo"@0xINDIRIZZO perche' quel testo lo produce il preprocessore e
 * non compare come letterale nel codice: `verificacitazioni.py` le
 * segnerebbe NON_ANCORATA, e avrebbe ragione.
 */
int ilitek_tddi_init(void)
{
	/* "\x016ILITEK: (%s, %d): driver version = %s\n"
	 * @0xffffff800923ed1e, riga 870; l'argomento e'
	 * "2.0.6.0.191122"@0xffffff800923ed58
	 * ("91356063 add"@0xffffff8008a54e70). */
#line 870
	ILI_INFO("driver version = %s\n", "2.0.6.0.191122");

	/* "9101c100 add"@0xffffff8008a54e90 (+112),
	 * "97db5d99 bl"@0xffffff8008a54e98 verso <__mutex_init> */
	mutex_init(&idev->touch_mutex);
	/* "91024100 add"@0xffffff8008a54eac (+144) */
	mutex_init(&idev->debug_mutex);
	/* "9102c100 add"@0xffffff8008a54ec8 (+176) */
	mutex_init(&idev->debug_read_mutex);
	/* "9108e100 add"@0xffffff8008a54ee4 (+568),
	 * "97db20e3 bl"@0xffffff8008a54eec verso <__init_waitqueue_head>.
	 * Le parentesi ci sono davvero: il letterale e' "&(idev->inq)". */
	init_waitqueue_head(&(idev->inq));
	/* "b900d11f str"@0xffffff8008a54f08: quattro byte a zero, che e'
	 * `spin_lock_init` senza lockdep su arm64. */
	spin_lock_init(&idev->c208);
	/* "b902e91f str"@0xffffff8008a54f0c (done a +744) piu'
	 * "910bc100 add"@0xffffff8008a54f00 (+752) e
	 * "97db20da bl"@0xffffff8008a54f10: e' `init_completion`, e lo prova
	 * il letterale a 0xffffff80090e2854, che `leggi_stringa.py` legge come
	 * `&x->wait` e che sta DENTRO la macro del kernel, non nel driver. */
	init_completion(&idev->c744);

	/* L'ORDINE E' MISURATO E NON E' CRESCENTE: 704, 712, 708, 716, 720,
	 * 724, 732, 736 -- "b902c11f str"@0xffffff8008a54f18,
	 * "b902c91f str"@0xffffff8008a54f1c, "b902c51f str"@0xffffff8008a54f20,
	 * "b902cd1f str"@0xffffff8008a54f24, "b902d11f str"@0xffffff8008a54f28,
	 * "b902d51f str"@0xffffff8008a54f2c, "b902dd1f str"@0xffffff8008a54f30,
	 * "b902e11f str"@0xffffff8008a54f34.
	 * +704 e +708 sono adiacenti e allineati a 8: se fossero contigui nel
	 * sorgente clang li fonderebbe in un solo `str xzr`. Non lo fa, quindi
	 * l'ordine del sorgente e' questo. */
	WRITE_ONCE(idev->c704, 0);
	WRITE_ONCE(idev->c712, 0);
	WRITE_ONCE(idev->c708, 0);
	WRITE_ONCE(idev->c716, 0);
	WRITE_ONCE(idev->c720, 0);
	WRITE_ONCE(idev->c724, 0);
	WRITE_ONCE(idev->c732, 0);
	WRITE_ONCE(idev->c736, 0);

	/* "940016ce bl"@0xffffff8008a54f38 */
	ilitek_tddi_ic_init();

	/*
	 * LA CHIAMATA ALLA FUNZIONE SENZA SIMBOLO. Il corpo che segue nel
	 * binario -- le due `alloc_workqueue`, i due `WARN_ON` e i due
	 * `INIT_DELAYED_WORK`, da 0xffffff8008a54f3c a 0xffffff8008a5501c con i
	 * due blocchi freddi del WARN in coda a 0xffffff8008a55208 e
	 * 0xffffff8008a55220 -- NON e' scritto qui, benche' il binario lo
	 * emetta dentro questo simbolo: i due `warn_slowpath_null` portano
	 * __LINE__ 356 e 357 ("52802c81 mov"@0xffffff8008a55210,
	 * "52802ca1 mov"@0xffffff8008a55228) e le sei `printk` di questa
	 * funzione portano 870, 910, 915, 932, 948 e 957. Un corpo di funzione
	 * e' contiguo nel sorgente: le righe 356/357 non possono stare dentro
	 * un corpo che contiene la 870 e la 957 senza contenere anche le righe
	 * 379..468, che sono di `ilitek_tddi_sleep_handler`. Vedi D16.
	 */
	ilitek_tddi_wq_init();

	/* "9109d50b add"@0xffffff8008a55018 (idev + 629) piu'
	 * "79000169 strh"@0xffffff8008a55020: uno store a 16 bit del valore 1
	 * copre +629 = 1 e +630 = 0. Sono due campi, non uno. */
	idev->c629 = 1;
	idev->c630 = 0;
	/* "3909fd1f strb"@0xffffff8008a55024 */
	idev->c639 = 0;
	/* "3909dd1f strb"@0xffffff8008a55028 */
	idev->c631 = 0;
	/* "910c614a add"@0xffffff8008a55014 = 0xffffff8008a5c318 =
	 * `demo_debug_info_id0` (gruppo E), "f901a90a str"@0xffffff8008a5502c.
	 * E' l'unica misura che il blocco da' sul contenuto di +848: dice che
	 * l'elemento 0 e' questo simbolo, NON quanti elementi ci siano
	 * (divergenza D4). */
	idev->c848[0] = demo_debug_info_id0;

	/* "b901451f str"@0xffffff8008a5503c */
	idev->c324 = 0;
	/* "3903811f strb"@0xffffff8008a55040 */
	idev->c224 = 0;

	/* "320003e0 orr"@0xffffff8008a55034 (1), "2a1f03e1 mov"@0xffffff8008a55038
	 * (0), "94000859 bl"@0xffffff8008a55044,
	 * "37f80c80 tbnz"@0xffffff8008a55048 */
	if (ilitek_ice_mode_ctrl(1, 0) < 0) {
		/* "\x013ILITEK: (%s, %d): Not found ilitek chips\n"
		 * @0xffffff800923edb2, riga 910 */
#line 910
		ILI_ERR("Not found ilitek chips\n");
		/* "12800240 mov"@0xffffff8008a551f0 = -19 */
		return -ENODEV;
	}

	/* "9400161c bl"@0xffffff8008a5504c,
	 * "36f800e0 tbz"@0xffffff8008a55050 */
	if (ilitek_tddi_ic_get_info() < 0)
		/* "\x013ILITEK: (%s, %d): Chip info is incorrect\n"
		 * @0xffffff800923edde, riga 915 */
#line 915
		ILI_ERR("Chip info is incorrect\n");

	/* incorporata, vedi sopra */
	ilitek_update_tp_module_info();

	/* "94007179 bl"@0xffffff8008a55104 */
	ilitek_tddi_node_init();
	/* "9400562d bl"@0xffffff8008a55108 */
	ilitek_tddi_fw_read_flash_info();

	/* "2a1f03e0 mov"@0xffffff8008a5510c, "2a1f03e1 mov"@0xffffff8008a55110,
	 * "94000825 bl"@0xffffff8008a55114 */
	if (ilitek_ice_mode_ctrl(0, 0) < 0)
		/* "\x013ILITEK: (%s, %d): Failed to disable ice mode failed
		 * during init\n"@0xffffff800923ee0a, riga 932. La ripetizione
		 * «failed ... failed» e' della fabbrica. */
#line 932
		ILI_ERR("Failed to disable ice mode failed during init\n");

	/* "3909f11f strb"@0xffffff8008a55138 */
	idev->c636 = 0;
	/* "9400130e bl"@0xffffff8008a5513c */
	ilitek_tddi_ic_get_core_ver();
	/* "94001537 bl"@0xffffff8008a55140 */
	ilitek_tddi_ic_get_protocl_ver();
	/* "940013b0 bl"@0xffffff8008a55144 */
	ilitek_tddi_ic_get_fw_ver();
	/* "940014a7 bl"@0xffffff8008a55148 */
	ilitek_tddi_ic_get_tp_info();
	/* "94001420 bl"@0xffffff8008a5514c */
	ilitek_tddi_ic_get_panel_info();
	/* "3909f113 strb"@0xffffff8008a5516c: cade PRIMA di
	 * "97db78d9 bl"@0xffffff8008a55170 verso <printk>. */
	idev->c636 = 1;

	/* "\x016ILITEK: (%s, %d): Registre touch to input subsystem\n"
	 * @0xffffff800923e7b5, riga 948. Il refuso «Registre» e' della
	 * fabbrica -- ed e' lo STESSO letterale che
	 * `ilitek_tddi_fw_upgrade_handler` usa alla riga 497. */
#line 948
	ILI_INFO("Registre touch to input subsystem\n");
	/* "94000310 bl"@0xffffff8008a55174 */
	ilitek_plat_input_register();

	/* le due chiamate vere: "97fffbba bl"@0xffffff8008a55180 e
	 * "97fffbb7 bl"@0xffffff8008a5518c */
	ilitek_tddi_wq_ctrl(0, 1);
	ilitek_tddi_wq_ctrl(1, 1);

	/* "39038113 strb"@0xffffff8008a5519c, PRIMA della chiamata seguente */
	idev->c224 = 1;

	/* "ili_wakelock"@0xffffff800923ee4d ("91393400 add"@0xffffff8008a55198),
	 * "97eaae77 bl"@0xffffff8008a551a0,
	 * "f9001128 str"@0xffffff8008a551b0 */
	idev->c32 = wakeup_source_register("ili_wakelock");
	/* "b5000208 cbnz"@0xffffff8008a551b4 */
	if (!idev->c32)
		/* "\x013ILITEK: (%s, %d): wakeup source request failed\n"
		 * @0xffffff800923ee5a, riga 957 */
#line 957
		ILI_ERR("wakeup source request failed\n");

	/* Il ritorno e' ZERO anche quando la wakeup source manca: sono DUE
	 * `mov w0, wzr`, "2a1f03e0 mov"@0xffffff8008a551ac sulla via che salta
	 * la stampa e "2a1f03e0 mov"@0xffffff8008a551d0 su quella che la fa.
	 * L'unico ritorno diverso da zero e' il -19 del ramo «Not found
	 * ilitek chips». */
	return 0;
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_dev_remove -- 0xffffff8008a55234, 208 byte
 * fabbrica: riga 964
 * ---------------------------------------------------------------------------
 * La prova che +32 e' un `struct wakeup_source *` sta qui, ed e' l'unica del
 * blocco: "f9401100 ldr"@0xffffff8008a552cc piu'
 * "97eaae70 bl"@0xffffff8008a552d4 verso <wakeup_source_unregister>.
 *
 * `ilitek_tddi_interface_dev_exit` e' l'UNICA delle 154 che riceve `idev`
 * come argomento invece di rileggerlo dal globale
 * ("f9452260 ldr"@0xffffff8008a552f0 in x0,
 * "940001c2 bl"@0xffffff8008a552f4).
 */
void ilitek_tddi_dev_remove(void)
{
	/* "\x016ILITEK: (%s, %d): remove ilitek dev\n"@0xffffff800923ee8c,
	 * riga 964. E' stampata PRIMA del controllo su `idev`. */
#line 964
	ILI_INFO("remove ilitek dev\n");

	/* "b40004c8 cbz"@0xffffff8008a55260 */
	if (!idev)
		return;

	/* "b9415d00 ldr"@0xffffff8008a55264 (c348) poi
	 * "b9415900 ldr"@0xffffff8008a55270 (c344): l'ordine e' questo, prima
	 * il campo piu' alto. */
	gpio_free(idev->c348);
	gpio_free(idev->c344);

	/* "f944b288 ldr"@0xffffff8008a5527c, "b4000108 cbz"@0xffffff8008a55280 */
	if (ilitek_wq_esd) {
		/* "97d9ee64 bl"@0xffffff8008a5528c */
		cancel_delayed_work_sync(&ilitek_esd_work);
		/* "97d9ea10 bl"@0xffffff8008a55294 */
		flush_workqueue(ilitek_wq_esd);
		/* "97d9f2b5 bl"@0xffffff8008a5529c */
		destroy_workqueue(ilitek_wq_esd);
	}

	/* "f944e688 ldr"@0xffffff8008a552a4, "b4000108 cbz"@0xffffff8008a552a8 */
	if (ilitek_wq_bat) {
		/* "97d9ee5a bl"@0xffffff8008a552b4 */
		cancel_delayed_work_sync(&ilitek_bat_work);
		/* "97d9ea06 bl"@0xffffff8008a552bc */
		flush_workqueue(ilitek_wq_bat);
		/* "97d9f2ab bl"@0xffffff8008a552c4 */
		destroy_workqueue(ilitek_wq_bat);
	}

	/* "b4000060 cbz"@0xffffff8008a552d0: la guardia c'e' anche se
	 * `wakeup_source_unregister` regge gia' il NULL. */
	if (idev->c32)
		wakeup_source_unregister(idev->c32);

	/* "f9408500 ldr"@0xffffff8008a552dc + "97dfed34 bl"@0xffffff8008a552e0,
	 * poi "f9402100 ldr"@0xffffff8008a552e8 +
	 * "97dfed31 bl"@0xffffff8008a552ec: entrambe `kfree`, senza guardia. */
	kfree(idev->c264);
	kfree(idev->c64);

	ilitek_tddi_interface_dev_exit(idev);
}

/*
 * ---------------------------------------------------------------------------
 * ilitek_tddi_dev_init -- 0xffffff8008a55304, 88 byte
 * fabbrica: riga 993
 * ---------------------------------------------------------------------------
 * D8 del cappello: il byte a +0 della `struct ilitek_hwif_info` e' letto con
 * un cast e non attraverso un campo, perche' quella struttura NON e'
 * ricostruita in questo lotto -- il gruppo A ne vede UN SOLO byte, e definirla
 * su un byte solo sarebbe la classe di difetto «un buco nella struttura»
 * applicata a una seconda struttura. La lettura e'
 * "39400008 ldrb"@0xffffff8008a55310, larga un byte, e il confronto e'
 * "7100611f cmp"@0xffffff8008a55328 con #0x18 = 24.
 *
 * I due letterali stanno in due corse di .rodata DIVERSE -- "I2C"@0xffffff80091aa860,
 * fuori dai letterali del driver, e "SPI"@0xffffff800923ef05, dentro -- e la
 * scelta e' una `csel` senza salto
 * ("9a890143 csel"@0xffffff8008a55334). Che "I2C" stia altrove e' fusione di
 * letterali fatta dal linker, non un dato del driver.
 */
int ilitek_tddi_dev_init(struct ilitek_hwif_info *hwif)
{
	/* "\x016ILITEK: (%s, %d): TP Interface: %s\n"@0xffffff800923eeca,
	 * riga 993 */
	ILI_INFO("TP Interface: %s\n",
#line 993
		 (*(const u8 *)hwif == 0x18) ? "I2C" : "SPI");

	/* "940000d1 bl"@0xffffff8008a5534c: l'argomento e' `hwif`, girato
	 * senza toccarlo ("aa1303e0 mov"@0xffffff8008a55348). */
	return ilitek_tddi_interface_dev_init(hwif);
}
