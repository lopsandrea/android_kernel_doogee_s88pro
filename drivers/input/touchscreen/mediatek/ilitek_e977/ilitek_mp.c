// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_ILITEK_E977 -- GRUPPO F, il collaudo MP. Doogee S88 Pro, MediaTek
 * MT6771. Sotto-lotto F1 (il parser del file .ini) CHIUSO, piu' DIECI
 * funzioni dei sotto-lotti F2/F3/F4/F5 -- l'ultima delle quali, aggiunta dal
 * lotto F5, e' `ilitek_tddi_mp_test_main`, che porta dentro di se' DODICI
 * funzioni INCORPORATE piu' una tredicesima (`parser_get_tdf`) che nessun
 * messaggio nomina -- e OTTO aggiunte dai lotti F3a, F3b, F3c, F3c-bis e F6.
 * TUTTE E VENTIQUATTRO le funzioni emesse del gruppo sono scritte, e gli
 * irrisolti ILITEK sono ZERO.
 *
 * ===========================================================================
 * IL LOTTO F6 -- 2026-08-22 -- mutual_test (4584), l'ultima del gruppo
 * ===========================================================================
 * QUESTA SEZIONE E' LA PIU' RECENTE E VA LETTA PER PRIMA: le sezioni che
 * seguono sono la storia dei lotti F1..F5 e F3a/F3b/F3c/F3c-bis e restano
 * com'erano, tranne dove dicono che `mutual_test` non e' scritta.
 *
 * SCRITTE CINQUE FUNZIONI, UNA EMESSA E QUATTRO INCORPORATE:
 *
 *   mutual_test              0xffffff8008a620d4   4584 byte   1146 istruzioni
 *   codeToOhm                riga 1369, "codeToOhm"@0xffffff8009244147
 *   mp_cdc_init_cmd_common   riga 1568, "mp_cdc_init_cmd_common"@0xffffff8009244083
 *   allnode_mutual_cdc_data  righe 1713..1795, "allnode_mutual_cdc_data"@0xffffff8009243e85
 *   ipio_strcmp              NESSUN messaggio la nomina: il nome e' SCELTO
 *
 * Le tre col `__func__` sono scritte come funzioni SEPARATE perche' clang le
 * rincorpori, ed e' cosi' che nell'oggetto non compaiono: `nm` sull'oggetto
 * costruito col compilatore di fabbrica stampa VENTIQUATTRO simboli `t`/`T`,
 * esattamente le ventiquattro emesse del gruppo, e nessuno dei quattro nomi
 * qui sopra.
 *
 * IL BILANCIO DEL GRUPPO, CHE TORNA AL BYTE:
 *   34088 (fino al lotto F3c-bis) + 4584 (`mutual_test`) = 38672, che e' la
 *   lunghezza dell'intero gruppo F, [0xffffff8008a5da3c, 0xffffff8008a6714c).
 *
 * ---------------------------------------------------------------------------
 * LA MISURA COL COMPILATORE DI FABBRICA (clang-r353983c, LLVM 9.0.3)
 * ---------------------------------------------------------------------------
 *   cd /mnt/s88pro/kernel-stock
 *   NM=/mnt/s88pro/lineage21/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-nm
 *   $NM --print-size out-ilitek-f9/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o \
 *       | grep -iE " [tT] " | sort -k3
 *
 *   0000000000006314 00000000000003d0 T allnode_open_cdc_result
 *   0000000000000ab0 00000000000000f0 T check_int_level
 *   00000000000009a0 00000000000000c0 T compare_charge
 *   000000000000012c 00000000000000b4 T dump_node_type_buffer
 *   0000000000000a60 0000000000000050 T full_open_rate_compare
 *   0000000000001f20 00000000000039f0 T ilitek_tddi_mp_test_main
 *   0000000000000520 000000000000035c T parser_ini_benchmark
 *   000000000000032c 00000000000001f4 T parser_ini_nodetype
 *   0000000000005e7c 0000000000000498 t allnode_open_cdc_data
 *   0000000000000ba0 0000000000000168 t compare_MaxMin_result
 *   0000000000005910 000000000000056c t create_mp_test_frame_buffer
 *   0000000000000000 000000000000012c t dump_benchmark_data
 *   00000000000078ac 0000000000000558 t key_test
 *   0000000000000dfc 0000000000000cf8 t mp_comp_result_before_retry
 *   0000000000001af4 000000000000042c t mp_compare_cdc_show_result
 *   0000000000000d60 000000000000009c t mp_do_retry
 *   00000000000066e4 00000000000011c8 t mutual_test
 *   0000000000007e04 0000000000000724 t open_test_cap
 *   0000000000008528 0000000000000f5c t open_test_sp
 *   00000000000001e0 000000000000014c t parser_get_ini_key_value
 *   000000000000087c 0000000000000124 t parser_get_u8_array
 *   0000000000009484 00000000000002e4 t pin_test
 *   0000000000000d08 000000000000002c t self_test
 *   0000000000000d34 000000000000002c t st_test
 *
 * VENTUNO delle ventiquattro combaciano al byte. Le tre che non combaciano
 * sono due divergenze gia' dichiarate da altri lotti -- `allnode_open_cdc_result`
 * (0x3d0 = 976 contro 984, F3b-D1) e `ilitek_tddi_mp_test_main`
 * (0x39f0 = 14832 contro 14704, lotto F5) -- piu' `mutual_test` di questo
 * lotto: 0x11c8 = 4552 contro 4584, cioe' TRENTADUE BYTE IN MENO, otto
 * istruzioni. La divergenza F6-D5 qui sotto dice quello che se ne sa.
 *
 *   ./venv/bin/python3 intervallo.py 21 24
 *   21 su 24 = 87.5%   IC95% Clopper-Pearson [67.6%; 97.3%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *
 * AVVERTENZA PRIMA DELLA PERCENTUALE: su ventiquattro unita' la misura NON
 * discrimina rispetto al 77,10% del ramo, nemmeno con 21 su 24.
 *
 * ---------------------------------------------------------------------------
 * LA MISURA COL NOSTRO COMPILATORE (clang-r383902, LLVM 11.0.1)
 * ---------------------------------------------------------------------------
 *   $NM --print-size out-ilitek-f/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o \
 *       | grep -w mutual_test
 *
 *   0000000000006898 00000000000011d8 t mutual_test
 *
 * 0x11d8 = 4568, sedici byte in meno. Con questo compilatore le funzioni
 * esatte del file scendono a OTTO su ventiquattro (allnode_open_cdc_data,
 * check_int_level, compare_charge, create_mp_test_frame_buffer,
 * full_open_rate_compare, mp_do_retry, self_test, st_test):
 *
 *   ./venv/bin/python3 intervallo.py 8 24
 *   8 su 24 = 33.3%   IC95% Clopper-Pearson [15.6%; 55.3%]
 *   contiene 77.10%: NO -- la misura discrimina
 *
 * E' lo stesso fenomeno gia' misurato dai lotti F3c, F3c-bis, F4 e F5: col
 * compilatore SBAGLIATO questo file sembrerebbe molto peggiore del ramo, col
 * GIUSTO la misura non discrimina in nessuna direzione.
 *
 * ---------------------------------------------------------------------------
 * IL CONFRONTO PER CODIFICA -- E UNO STRUMENTO NUOVO, E PERCHE' SERVE
 * ---------------------------------------------------------------------------
 * `f3cbis_codifica.py` confronta le due successioni POSIZIONE PER POSIZIONE e
 * si ferma quando i due conteggi non combaciano. Su `mutual_test` non
 * combaciano, e lo strumento lo dice:
 *
 *   ./venv/bin/python3 f3cbis_codifica.py mutual_test 0xffffff8008a620d4 \
 *       out-ilitek-f9/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o
 *
 *   === mutual_test ===
 *   istruzioni: fabbrica 1146, nostro 1138
 *   NUMERO DI ISTRUZIONI DIVERSO: confronto posizionale sospeso
 *   stesso mnemonico in posizione: 281 su 1138
 *   codifiche diverse: 1068
 *     di cui siti di rilocazione o chiamata locale: 301
 *     RESIDUO: 767
 *
 * QUEL "281 su 1138" NON E' UNA MISURA DELLA SOMIGLIANZA, e va detto prima
 * del numero: la PRIMA divergenza cade alla decima istruzione
 * ("d000b4da adrp"@0xffffff8008a620fc, che la fabbrica emette e noi no) e da
 * li' in avanti tutto il resto e' spostato di una posizione, quindi il
 * confronto posizionale conta come diverse anche le istruzioni identiche.
 *
 * LO STRUMENTO NUOVO E' `f6_allinea.py` (scritto da questo lotto, sul PC di
 * build in /mnt/s88pro/kernel-stock): allinea prima le due successioni di
 * mnemonici con `difflib` e poi conta.
 *
 *   ./venv/bin/python3 f6_allinea.py mutual_test 0xffffff8008a620d4 \
 *       out-ilitek-f9/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o | head -5
 *
 *   === mutual_test ===
 *   istruzioni: fabbrica 1146, nostro 1138
 *   mnemonici uguali dopo allineamento: 963 su 1146
 *   blocchi non allineati: 107
 *
 * 963 su 1146 e' l'84,0%. E' molto meno del 100% che i lotti F3c, F3c-bis e
 * F4 hanno raggiunto sulle loro funzioni, e la ragione sta tutta in due
 * scelte del compilatore che questa funzione fa scattare e le altre no --
 * vedi F6-D5.
 *
 * ---------------------------------------------------------------------------
 * F6-D5 -- LA DIVERGENZA APERTA: TRENTADUE BYTE, E DOVE STANNO
 * ---------------------------------------------------------------------------
 * Con `f6_allinea.py` i blocchi non allineati si contano per regione, e la
 * somma dei loro scarti e' -8 istruzioni:
 *
 *   regione        uguali    F-div    N-div    netto
 *   preambolo         499       48       49       +1
 *   case4              45        5        3       -2
 *   case5              42        4        9       +5
 *   case6              89       55       45      -10
 *   case7 (int)        29        1        1       +0
 *   default            15        1        1       +0
 *   case9              86       11       10       -1
 *   case7 (cap)        81       51       51       +0
 *   coda               77        7        6       -1
 *
 * LA REGIONE CHE PESA E' IL `case 6`, ed e' -10. Il nostro clang, sullo
 * stesso sorgente, fa DUE cose che il clang di fabbrica non fa:
 *  - allarga a 64 bit la variabile del ciclo interno: nel nostro oggetto la
 *    prova a zero e l'incremento sono a 64 bit (`cbz x11` e
 *    `add x11, x11, #0x1`, scritti SENZA indirizzo perche' sono del NOSTRO
 *    oggetto e non della fabbrica), mentre di fabbrica sono a 32 bit
 *    ("3500026a cbnz"@0xffffff8008a62c3c e "1100054a add"@0xffffff8008a62ce8),
 *    e con essa il limite del ciclo e' letto con `ldrsw` invece che con la
 *    `ldr` di fabbrica ("b94beb6e ldr"@0xffffff8008a62ce4);
 *  - da quella variabile a 64 bit ricava un PUNTATORE d'induzione e fonde due
 *    letture adiacenti in una coppia `ldp`/`ldur`, che di fabbrica sono tre
 *    `ldr` separate con indice calcolato ("b86ed90e ldr"@0xffffff8008a62b70,
 *    "b86fd90f ldr"@0xffffff8008a62b78 e "b86bd90d ldr"@0xffffff8008a62c24).
 * Le due cose insieme tolgono istruzioni: e' il compilatore che ottimizza di
 * piu', non codice mancante. QUESTO LOTTO NON HA ISOLATO QUALE OPZIONE DELLA
 * RIGA DI COMANDO LO DECIDA -- e' la stessa domanda che il lotto F3c-bis ha
 * lasciato aperta, e resta aperta.
 *
 * UNA FORMA PROVATA E SCARTATA, E IL NUMERO CHE L'HA SCARTATA. Una stesura
 * intermedia di questo lotto misurava 0x11e8 = 4584, cioe' ESATTA al byte.
 * Aveva pero' i due confronti sul nome del `case 7` scritti come due `if`
 * INDIPENDENTI, e in quella forma il compilatore chiama `strlen` DUE volte;
 * il binario di fabbrica la chiama UNA SOLA:
 *
 *   grep -c "__pi_strlen" (disassemblato di mutual_test, righe 795 e 942)
 *   ffffff8008a62d1c:	940fd579 	bl	ffffff8008e58300 <__pi_strlen>
 *   ffffff8008a62d48:	940fd58f 	bl	ffffff8008e58384 <__pi_strncmp>
 *   ffffff8008a62f68:	940fd507 	bl	ffffff8008e58384 <__pi_strncmp>
 *
 * -- due `strncmp` e una sola `strlen`. Scritti come `else if` (che e' cio'
 * che il binario misura, perche' solo sul ramo in cui il primo confronto
 * fallisce non c'e' ancora nessuna scrittura in memoria a invalidare la
 * `strlen`) la funzione scende a 4552. LA FORMA SCRITTA QUI E' QUELLA CHE
 * IL BINARIO MISURA, NON QUELLA CHE FA TORNARE LA DIMENSIONE: aggiustare il
 * sorgente perche' un numero torni e' esattamente cio' che la regola 3
 * vieta.
 *
 * ---------------------------------------------------------------------------
 * QUATTRO DIFETTI DELLA FABBRICA, RIPRODOTTI (regola 7)
 * ---------------------------------------------------------------------------
 * F6-D1. L'AZZERAMENTO DEL BUFFER DI TRAMA COPRE UN QUARTO DI CIO' CHE SERVE.
 *   "b98c0322 ldrsw"@0xffffff8008a626b4 passa `core_mp.c256` -- il NUMERO di
 *   nodi -- come terzo argomento di `__memset`
 *   ("940fd651 bl"@0xffffff8008a626bc), mentre l'allocazione dello stesso
 *   vettore moltiplica per quattro ("d37ef500 lsl"@0xffffff8008a628a4). E' lo
 *   stesso difetto che il lotto F3c-bis ha trovato su `mp_frame_buf1` dentro
 *   `allnode_key_cdc_data`: sono due copie della stessa riga sbagliata.
 *
 * F6-D2. IL CICLO CHE INIZIALIZZA MASSIMI E MINIMI LEGGE `mp_items[0].c16`,
 *   NON `mp_items[index].c16`. E' il fatto che la ricognizione del lotto F3a
 *   segnalava come «sorprendente, da verificare»: E' VERO.
 *   "b946e152 ldr"@0xffffff8008a621e0 legge l'indirizzo COSTANTE
 *   0xffffff80099876e0 = `mp_items` + 16, e il registro di base non e' toccato
 *   fra "b000792a adrp"@0xffffff8008a621bc e la lettura -- l'indice non
 *   compare affatto in quell'accesso. `mp_items[0]` e' "baseline data(bg)",
 *   che nella tavola ha `c16 = 0`: il ramo `== 4` di quel ciclo, di fabbrica,
 *   non parte mai, e i quattro vettori di `core_mp` restano com'erano anche
 *   per l'elemento "tx/rx delta", che e' l'unico con `c16 == 4`.
 *
 * F6-D3. NEL `case 4` I DUE `delta` SONO CALCOLATI SULLA STESSA DIFFERENZA.
 *   "1100054a add"@0xffffff8008a629a0 e "1100054a add"@0xffffff8008a629d8
 *   calcolano tutt'e due `x * c232 + y + 1`, cioe' il vicino a DESTRA, mentre
 *   le due GUARDIE sono diverse: "6b0d011f cmp"@0xffffff8008a6298c prova
 *   `x != c236 - 1` (il vicino di SOTTO) e "6b0d019f cmp"@0xffffff8008a629c8
 *   prova `y != c232 - 1` (il vicino a destra). `tx_delta_buf` riceve quindi
 *   la differenza orizzontale con la guardia verticale.
 *
 * F6-D4. NEL `case 6` TUTTI E QUATTRO I VICINI SONO `- 1` E `+ 1`. Le nove
 *   vie del blocco caricano SOLO due indirizzi, `x*c232+y-1` e `x*c232+y+1`:
 *   non c'e' nessuna moltiplicazione per `core_mp.c232` fra le due, quindi il
 *   vicino "di sopra" e quello "di sotto" sono in realta' quelli a sinistra e
 *   a destra. Negli angoli le due `ILI_ABS` che il sorgente scrive si
 *   riducono a UNA SOLA lettura perche' sono identiche: per esempio
 *   "1100060e add"@0xffffff8008a62c70 e' l'unico accesso del ramo
 *   `x == 0 && y == 0`.
 *
 * ---------------------------------------------------------------------------
 * LE SCOPERTE DI QUESTO LOTTO
 * ---------------------------------------------------------------------------
 * F6-S1. `core_mp` HA UN CAMPO A +227 CHE ERA DICHIARATO RIEMPIMENTO.
 *   "396f8d4a ldrb"@0xffffff8008a62e48 lo legge a UN byte e
 *   "9a96032a csel"@0xffffff8008a62e54 lo usa per scegliere quale delle due
 *   coppie di parametri passare a `codeToOhm`. Stava dentro `u8 c217[11]`
 *   perche' nessuna funzione scritta prima lo toccava. Nessun messaggio lo
 *   nomina: resta `c227` (regola 5).
 *
 * F6-S2. `ipio_strcmp` E' UNA FUNZIONE, NON DUE CONTROLLI IN FILA, E LO DICE
 *   UN -1. Sette siti di `mutual_test` portano lo stesso schema -- una
 *   `strlen`, il confronto della lunghezza con una costante, e solo se
 *   combacia una `strncmp` con quella stessa costante. In uno di essi il
 *   valore -1 e' MATERIALIZZATO ("12800000 mov"@0xffffff8008a6270c) e
 *   confluisce con il risultato della `strncmp` in un solo registro, che poi
 *   "34000300 cbz"@0xffffff8008a6271c prova contro zero. Due controlli
 *   scritti in fila (`strlen(a) == strlen(b) && strncmp(...) == 0`, che e' la
 *   forma che il lotto F5 usa in `ilitek_tddi_mp_init_item`) darebbero due
 *   salti allo stesso bersaglio e nessun -1.
 *
 * F6-S3. IL PUNTATORE `mp_frame_buf0` E' COPIATO IN UN LOCALE NEI CASI 4, 5,
 *   6 E 7, E NON NEI CASI 9 E `default`. E' una MISURA, non una lettura: nel
 *   caso 5 il puntatore e' letto una volta sola
 *   ("f9421108 ldr"@0xffffff8008a62a34) e non viene piu' riletto benche' fra
 *   un uso e l'altro ci sia una `str` che potrebbe scrivergli sopra, mentre
 *   nello stesso ciclo `mp_items[index].max_buf` E' riletto
 *   ("f940028d ldr"@0xffffff8008a62a4c) a ogni giro. Nel `default`, invece,
 *   il puntatore e' riletto a ogni giro
 *   ("f942114a ldr"@0xffffff8008a62da8, dentro il corpo del ciclo). Le due
 *   cose non possono venire dalla stessa forma di sorgente.
 *
 * F6-S4. IL `case 7` E' UN `else if`, E LO DICE IL CONTO DELLE `strlen`.
 *   Vedi F6-D5 qui sopra: due `strncmp` e una sola `strlen`.
 *
 * F6-S5. L'ORDINE DEI DUE TERMINI DENTRO UNA CONGIUNZIONE VALE OTTO BYTE.
 *   Nel `case 6` il secondo ramo della catena e' `y == 0 && x == c236 - 1` e
 *   non il contrario, e la prova e' l'ordine dei due confronti nel binario:
 *   "3500026a cbnz"@0xffffff8008a62c3c (che prova `y`) precede
 *   "6b0c013f cmp"@0xffffff8008a62c40 (che prova `x`). Scritto con `x` per
 *   primo la funzione misura otto byte in meno. Lo stesso vale per il quarto
 *   ramo, dove "6b0f015f cmp"@0xffffff8008a62b08 (che prova `y`) precede
 *   "6b0c013f cmp"@0xffffff8008a62b10.
 *
 * ---------------------------------------------------------------------------
 * IL LINK -- GLI IRRISOLTI ILITEK PASSANO DA UNO A ZERO
 * ---------------------------------------------------------------------------
 * Misurato sul `built-in.o` di TUTTI E OTTO gli oggetti ILITEK
 * (albero-ilitek-b / out-ilitek-b), come DIFFERENZA FRA INSIEMI su un
 * ARCHIVIO, con la classe dei definiti che include anche i `W`/`w`.
 *
 *   PRIMA del lotto:  definiti: 229   usati(U): 173   IRRISOLTI: 102
 *   DOPO  il lotto:   definiti: 230   usati(U): 172   IRRISOLTI: 101
 *
 *   diff <(tail -n +2 /tmp/irr_prima.txt) <(tail -n +2 /tmp/irr_dopo.txt)
 *   65d64
 *   < mutual_test
 *
 *   ./irrisolti.sh | tail -n +2 | grep -xE "mutual_test|open_test_sp|open_test_cap|key_test|pin_test|fix_tp_proc_info|str2hex|katoi|codeToOhm|mp_cdc_init_cmd_common|allnode_mutual_cdc_data|ipio_strcmp"
 *   fix_tp_proc_info
 *
 * DEI 101, cento sono simboli del nucleo e del quadro `tpd` MediaTek che il
 * link del kernel risolve. L'unico che un driver di touch dovrebbe definire
 * e' `fix_tp_proc_info` (0xffffff8008a5151c), che NON e' di ILITEK: sta nel
 * nucleo `tpd` MediaTek e nel nostro albero ALPS non esiste. E' un debito
 * gia' dichiarato in `ilitek_ic.c`, non un effetto di questo lotto.
 * **GLI IRRISOLTI ILITEK SONO ZERO: il driver si linka.**
 *
 * CONTANDO ANCHE CIO' CHE IL CODICE NUOVO CITA -- che e' la lezione del lotto
 * F5 -- `mutual_test` chiama `create_mp_test_frame_buffer`,
 * `parser_ini_benchmark`, `dump_benchmark_data`, `parser_get_ini_key_value`,
 * `parser_get_u8_array`, `compare_MaxMin_result`, `ilitek_dump_data`,
 * `ilitek_tddi_ic_check_busy` e `ilitek_tddi_ic_check_int_stat`: le prime sei
 * sono definite in questo file, le ultime tre negli altri oggetti ILITEK.
 * Nessun simbolo NUOVO compare fra gli irrisolti, ed e' per questo che il
 * conto scende invece di salire.
 *
 * ---------------------------------------------------------------------------
 * I DUE VERIFICATORI DI CITAZIONE, DOPO IL LOTTO
 * ---------------------------------------------------------------------------
 *   ./venv/bin/python3 verificacitazioni.py ilitek_mp.c oracolo/stock.elf
 *
 *     letterali: 340   citati: 292   verificati: 291   probanti: 276   deboli: 15
 *     di cui assemblate dalla macro di log: 128
 *     di cui nome di funzione da __func__: 34
 *     rinviate a verificaistruzioni.py: 2441
 *
 *   L'UNICA che non verifica e' `INDIRIZZO_AMBIGUO 0x0: %s -- citata con
 *   indirizzi diversi: 0xffffff8009100e35, 0xffffff8009101762`, ed e'
 *   PRECEDENTE a questo lotto (la stessa riga esce sul file di HEAD).
 *
 *   TRE CITAZIONI CHE ERANO `NON_ANCORATA` SONO DIVENTATE `VERIFICATA_FUNC`,
 *   ed e' esattamente cio' che il lotto F3a aveva previsto: i tre nomi
 *   `codeToOhm`, `mp_cdc_init_cmd_common` e `allnode_mutual_cdc_data` adesso
 *   esistono nel sorgente. Sul file di HEAD lo strumento stampava
 *   `letterali: 333   citati: 282   verificati: 278` e tre righe
 *   `NON_ANCORATA`; adesso non ne stampa nessuna.
 *
 *   ./venv/bin/python3 verificaistruzioni.py ilitek_mp.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a5da3c:0xffffff8008a6714c
 *
 *     citazioni di istruzione trovate nel sorgente: 2530 (2530 a codifica)
 *     confermate: 2498   assenti: 32   mnemonico diverso: 0   controfattuali: 0
 *     operandi -- registro diverso: 0   immediato diverso: 0
 *
 *   LE TRENTADUE ASSENTI SONO LE STESSE TRENTADUE DI PRIMA DEL LOTTO: sullo
 *   stesso comando il file di HEAD stampa `citazioni: 2114   confermate: 2082
 *   assenti: 32`. Le 416 citazioni nuove di questo lotto sono confermate
 *   TUTTE. Le 32 stanno nei cappelli dei lotti precedenti e citano codifiche
 *   del NOSTRO oggetto (i listati diagnostici di F3c-bis) o istruzioni fuori
 *   dal gruppo F (la `.exit` del driver), non del blocco.
 *
 * ---------------------------------------------------------------------------
 * COSA QUESTO LOTTO HA CREATO SUL PC DI BUILD
 * ---------------------------------------------------------------------------
 *   /mnt/s88pro/kernel-stock/f6_allinea.py   -- il confronto per codifica con
 *       allineamento, descritto sopra. Non tocca niente di condiviso.
 * Nessun altro file e nessun altro albero: la compilazione usa
 * `albero-ilitek-f` / `out-ilitek-f` (nostro compilatore),
 * `out-ilitek-f9` (compilatore di fabbrica) e `albero-ilitek-b` /
 * `out-ilitek-b` (gli otto oggetti insieme), tutti gia' esistenti.
 *
 * ---------------------------------------------------------------------------
 * QUANTO E' SCELTO, IN QUESTO LOTTO, E NON MISURATO
 * ---------------------------------------------------------------------------
 *  - il NOME `ipio_strcmp` e i nomi `ILI_ABS` e `ILI_MAX`: nessuna stringa
 *    del binario li porta. La FORMA e' misurata, il nome no;
 *  - il valore reso da `ipio_strcmp` quando le lunghezze combaciano: il -1
 *    del caso opposto e' misurato, questo no -- nessun sito ne prova un
 *    valore diverso da zero;
 *  - il tipo di ritorno di `codeToOhm` (`int`) e il fatto che il suo secondo
 *    parametro sia un `u16 *` e non due `u16`: che il chiamante SCELGA fra
 *    due coppie in memoria e' misurato ("9a96032a csel"@0xffffff8008a62e54),
 *    la firma esatta no;
 *  - i nomi delle variabili locali: nessuno sta nel binario;
 *  - la partizione del corpo fra `mutual_test` e le tre incorporate: le tre
 *    ESISTONO (lo dice il `__func__`), ma dove esattamente cominciano e
 *    finiscono e' deciso dal messaggio piu' esterno che porta ciascun nome,
 *    non da un confine che il binario segni.
 *
 * ===========================================================================
 * IL LOTTO F3c-bis -- 2026-08-22 -- key_test (1368) e pin_test (740)
 * ===========================================================================
 * QUESTA SEZIONE E' LA PIU' RECENTE E VA LETTA PER PRIMA: le sezioni che
 * seguono sono la storia dei lotti F1..F5 e F3a/F3b/F3c e restano com'erano.
 *
 * SCRITTE TRE FUNZIONI, DUE EMESSE E UNA INCORPORATA:
 *
 *   key_test   0xffffff8008a6493c   1368 byte   342 istruzioni   `t`
 *   pin_test   0xffffff8008a64eec    740 byte   185 istruzioni   `t`
 *   allnode_key_cdc_data -- NON e' un simbolo: e' incorporata dentro
 *       `key_test` e la sua esistenza e' provata dal `__func__`
 *       "allnode_key_cdc_data"@0xffffff8009244769, che sette `printk` del
 *       blocco portano mentre le tre che le circondano portano
 *       "key_test"@0xffffff80092446f1 (righe 1410..1484).
 *
 * IL BILANCIO DEL GRUPPO, CHE TORNA AL BYTE:
 *   22672 (fino al lotto F5) + 3548 (i tre di F3a/F3b) + 5760 (open_test_sp e
 *   open_test_cap di F3c) + 2108 (key_test e pin_test di questo lotto)
 *   = 34088 scritti, su 24 funzioni emesse ne restano 23 scritte e UNA no.
 *   34088 + 4584 (`mutual_test`) = 38672.
 *
 * ---------------------------------------------------------------------------
 * LA MISURA COL COMPILATORE DI FABBRICA (clang-r353983c, LLVM 9.0.3)
 * ---------------------------------------------------------------------------
 *   cd /mnt/s88pro/kernel-stock
 *   NM=/mnt/s88pro/lineage21/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-nm
 *   $NM --print-size out-ilitek-f9/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o \
 *       | grep -iE " [tT] " | sort -k3
 *
 *   0000000000006314 00000000000003d0 T allnode_open_cdc_result
 *   0000000000000ab0 00000000000000f0 T check_int_level
 *   00000000000009a0 00000000000000c0 T compare_charge
 *   000000000000012c 00000000000000b4 T dump_node_type_buffer
 *   0000000000000a60 0000000000000050 T full_open_rate_compare
 *   0000000000001f20 00000000000039f0 T ilitek_tddi_mp_test_main
 *   0000000000000520 000000000000035c T parser_ini_benchmark
 *   000000000000032c 00000000000001f4 T parser_ini_nodetype
 *   0000000000005e7c 0000000000000498 t allnode_open_cdc_data
 *   0000000000000ba0 0000000000000168 t compare_MaxMin_result
 *   0000000000005910 000000000000056c t create_mp_test_frame_buffer
 *   0000000000000000 000000000000012c t dump_benchmark_data
 *   00000000000066e4 0000000000000558 t key_test
 *   0000000000000dfc 0000000000000cf8 t mp_comp_result_before_retry
 *   0000000000001af4 000000000000042c t mp_compare_cdc_show_result
 *   0000000000000d60 000000000000009c t mp_do_retry
 *   0000000000006c3c 0000000000000724 t open_test_cap
 *   0000000000007360 0000000000000f5c t open_test_sp
 *   00000000000001e0 000000000000014c t parser_get_ini_key_value
 *   000000000000087c 0000000000000124 t parser_get_u8_array
 *   00000000000082bc 00000000000002e4 t pin_test
 *   0000000000000d08 000000000000002c t self_test
 *   0000000000000d34 000000000000002c t st_test
 *
 * LE DUE FUNZIONI DI QUESTO LOTTO SONO ESATTE AL BYTE, tutt'e due alla PRIMA
 * compilazione: 0x558 = 1368 contro 1368 e 0x2e4 = 740 contro 740.
 *
 * UNA TRAPPOLA MISURATA, PER CHI VERRA' DOPO: il md5 del `.o` NON e' un'identita'
 * stabile del codice emesso. Modificando SOLO commenti il md5 dell'oggetto
 * cambia (f707ad7679ee066d3c74f719c57fa1b9 -> 52370f09254ff66c529d3ec0ae7dd17e)
 * mentre tutte e ventitre le dimensioni e tutti gli scostamenti restano
 * identici: l'oggetto porta `.debug_str`, `.debug_loc`, `.debug_line` e
 * compagne (`objdump -h` le elenca), e le tabelle di riga cambiano con i
 * commenti. L'identita' stabile e' il md5 della sola `.text`:
 *
 *   $OBJCOPY -O binary --only-section=.text \
 *       out-ilitek-f9/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o /tmp/text_f9.bin
 *   md5sum /tmp/text_f9.bin
 *   bb6e2801da897a0bce5bbd903ee0fb5d  /tmp/text_f9.bin
 *
 * ventitre funzioni emesse, VENTUNO combaciano al byte. Le due che non
 * combaciano sono divergenze gia' dichiarate da altri lotti:
 * `allnode_open_cdc_result` (0x3d0 = 976 contro 984, F3b-D1) e
 * `ilitek_tddi_mp_test_main` (0x39f0 = 14832 contro 14704, lotto F5).
 *
 *   ./venv/bin/python3 intervallo.py 21 23
 *   21 su 23 = 91.3%   IC95% Clopper-Pearson [72.0%; 98.9%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *
 * AVVERTENZA PRIMA DELLA PERCENTUALE: su ventitre unita' la misura NON
 * discrimina rispetto al 77,10% del ramo, nemmeno con 21 su 23.
 *
 * ---------------------------------------------------------------------------
 * LA MISURA COL NOSTRO COMPILATORE (clang-r383902, LLVM 11.0.1)
 * ---------------------------------------------------------------------------
 *   $NM --print-size out-ilitek-f/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o \
 *       | grep -iE " [tT] " | grep -wE "key_test|pin_test"
 *
 *   0000000000006898 000000000000055c t key_test
 *   00000000000084b8 00000000000002dc t pin_test
 *
 *   key_test 0x55c = 1372   +4
 *   pin_test 0x2dc =  732   -8
 *
 * Con questo compilatore le funzioni esatte scendono a OTTO su ventitre:
 *
 *   ./venv/bin/python3 intervallo.py 8 23
 *   8 su 23 = 34.8%   IC95% Clopper-Pearson [16.4%; 57.3%]
 *   contiene 77.10%: NO -- la misura discrimina
 *
 * E' lo stesso fenomeno gia' misurato dai lotti F4 e F3c: col compilatore
 * SBAGLIATO questo file sembrerebbe molto peggiore del ramo, col GIUSTO la
 * misura non discrimina in nessuna direzione. Il giudice e' il compilatore di
 * fabbrica.
 *
 * ---------------------------------------------------------------------------
 * IL CONFRONTO PER CODIFICA -- la misura che conta piu' della dimensione
 * ---------------------------------------------------------------------------
 * Lo strumento e' `f3cbis_codifica.py` (scritto da questo lotto, sul PC di
 * build in /mnt/s88pro/kernel-stock): confronta parola per parola il .o
 * costruito col compilatore di FABBRICA contro il disassemblato dell'oracolo,
 * e conta come "sito di rilocazione o chiamata locale" ogni istruzione che nel
 * nostro oggetto porta una R_AARCH64_* e ogni `bl`/`b` il cui bersaglio cade
 * fuori dalla funzione.
 *
 *   ./venv/bin/python3 f3cbis_codifica.py key_test 0xffffff8008a6493c \
 *       out-ilitek-f9/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o
 *
 *   === key_test ===
 *   istruzioni: fabbrica 342, nostro 342
 *   stesso mnemonico in posizione: 336 su 342
 *   codifiche diverse: 133
 *     di cui siti di rilocazione o chiamata locale: 127
 *     RESIDUO: 6
 *      88  F ffffff8008a64a9c 320003e8 orr    w8, wzr, #0x1            | N 39009bff strb   wzr, [sp,#38]
 *      89  F ffffff8008a64aa0 39009bff strb   wzr, [sp,#38]            | N f9418528 ldr    x8, [x9,#776]
 *      90  F ffffff8008a64aa4 b902dd28 str    w8, [x9,#732]            | N 320003ea orr    w10, wzr, #0x1
 *      91  F ffffff8008a64aa8 f9418528 ldr    x8, [x9,#776]            | N 910093e0 add    x0, sp, #0x24
 *      92  F ffffff8008a64aac 910093e0 add    x0, sp, #0x24            | N 320007e1 orr    w1, wzr, #0x3
 *      93  F ffffff8008a64ab0 320007e1 orr    w1, wzr, #0x3            | N b902dd2a str    w10, [x9,#732]
 *
 *   ./venv/bin/python3 f3cbis_codifica.py pin_test 0xffffff8008a64eec \
 *       out-ilitek-f9/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o
 *
 *   === pin_test ===
 *   istruzioni: fabbrica 185, nostro 185
 *   stesso mnemonico in posizione: 181 su 185
 *   codifiche diverse: 78
 *     di cui siti di rilocazione o chiamata locale: 71
 *     RESIDUO: 7
 *      98  F ffffff8008a65074 b902dd14 str    w20, [x8,#732]           | N f9418509 ldr    x9, [x8,#776]
 *      99  F ffffff8008a65078 f9418508 ldr    x8, [x8,#776]            | N b902dd14 str    w20, [x8,#732]
 *     100  F ffffff8008a6507c d63f0100 blr    x8                       | N d63f0120 blr    x9
 *     115  F ffffff8008a650b8 320003e9 orr    w9, wzr, #0x1            | N 320003ea orr    w10, wzr, #0x1
 *     118  F ffffff8008a650c4 b902dd09 str    w9, [x8,#732]            | N f9418509 ldr    x9, [x8,#776]
 *     119  F ffffff8008a650c8 f9418508 ldr    x8, [x8,#776]            | N b902dd0a str    w10, [x8,#732]
 *     120  F ffffff8008a650cc d63f0100 blr    x8                       | N d63f0120 blr    x9
 *
 * ---------------------------------------------------------------------------
 * LA DIVERGENZA F3c-bis-D1 -- UNA SOLA, E HA UNA CAUSA SOLA
 * ---------------------------------------------------------------------------
 * I TREDICI RESIDUI (6 + 7) SONO TUTTI LO STESSO IDIOMA, in tre siti:
 *
 *     idev->c732 = 1;
 *     ret = idev->c776(cmd, N);
 *
 * Di fabbrica l'ordine e' `str` del campo a +732, poi `ldr` del puntatore a
 * funzione a +776, poi `blr`; il nostro clang-r353983c sposta la `ldr` PRIMA
 * della `str` e per questo assegna un registro diverso alla costante 1. Non
 * c'e' nessuna istruzione in piu' ne' in meno -- 342 contro 342 e 185 contro
 * 185 -- e la dimensione e' esatta in tutt'e due le funzioni: e' una
 * RIORDINATURA, non una differenza di sorgente.
 *
 * NON E' UNA NOVITA' DI QUESTO LOTTO, ED E' QUESTA LA PROVA CHE LA CAUSA E'
 * UNA SOLA: lo stesso idioma sta in `allnode_open_cdc_data`, scritta dal
 * lotto F3a, ESATTA al byte, e anche li' il residuo cade sulla stessa `str`:
 *
 *   ./venv/bin/python3 f3cbis_codifica.py allnode_open_cdc_data 0xffffff8008a65af4 \
 *       out-ilitek-f9/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o
 *
 *   === allnode_open_cdc_data ===
 *   istruzioni: fabbrica 294, nostro 294
 *   stesso mnemonico in posizione: 291 su 294
 *   codifiche diverse: 111
 *     di cui siti di rilocazione o chiamata locale: 109
 *     RESIDUO: 2
 *      99  F ffffff8008a65c80 f000b4aa adrp   x10, ffffff800a0fc000 <_einittext+0xd53ae8> | N 9102e3e0 add    x0, sp, #0xb8
 *     100  F ffffff8008a65c84 9102e3e0 add    x0, sp, #0xb8            | N b902dd09 str    w9, [x8,#732]
 *
 * UNA FORMA PROVATA E SCARTATA, e il numero che l'ha scartata: spostando
 * `idev->c732 = 1;` PRIMA delle tre assegnazioni a `cmd` -- l'unica
 * riscrittura possibile che non aggiunga codice -- `key_test` resta 0x558 =
 * 1368 ma il residuo SALE da 6 a 9 e i mnemonici uguali scendono da 336 a 333.
 * La forma scritta qui e' quella misurata migliore delle due.
 *
 * CIO' CHE QUESTO LOTTO NON SA: perche' lo stesso clang-r353983c pianifichi
 * diversamente da come ha pianificato per la fabbrica. Le due compilazioni
 * differiscono per la riga di comando (l'albero ALPS non e' quello Wingtech) e
 * questo lotto non ha isolato quale opzione lo decida.
 *
 * ---------------------------------------------------------------------------
 * IL LINK -- GLI IRRISOLTI ILITEK PASSANO DA DUE A UNO
 * ---------------------------------------------------------------------------
 * Misurato sul `built-in.o` di TUTTI E OTTO gli oggetti ILITEK
 * (albero-ilitek-b / out-ilitek-b), come DIFFERENZA FRA INSIEMI su un
 * ARCHIVIO, con la classe dei definiti che include anche i `W`/`w`
 * (`memset` e `memcpy` sono alias deboli di `__memset`/`__memcpy`).
 *
 *   cat /mnt/s88pro/kernel-stock/irrisolti.sh
 *   NM=.../aarch64-linux-android-nm
 *   B=out-ilitek-b/drivers/input/touchscreen/mediatek/ilitek_e977/built-in.o
 *   $NM $B | awk '$2 ~ /^[TtWwDdBbRrVvSs]$/ {print $3}' | sort -u > /tmp/definiti.txt
 *   $NM $B | awk '$1 == "U" {print $2}' | sort -u > /tmp/usati.txt
 *   comm -23 /tmp/usati.txt /tmp/definiti.txt > /tmp/irrisolti.txt
 *
 *   PRIMA del lotto:  definiti: 227   usati(U): 173   IRRISOLTI: 104
 *   DOPO  il lotto:   definiti: 229   usati(U): 173   IRRISOLTI: 102
 *
 *   diff <(tail -n +2 /tmp/irr_prima.txt) <(tail -n +2 /tmp/irr_dopo.txt)
 *   54d53
 *   < key_test
 *   69d67
 *   < pin_test
 *
 * DEI 102, cento sono simboli del nucleo e del quadro `tpd` MediaTek che il
 * link del kernel risolve. Quelli che questo driver dovrebbe definire e non
 * definisce sono DUE, e uno solo e' ILITEK:
 *
 *   ./irrisolti.sh | tail -n +2 | grep -xE "mutual_test|open_test_sp|open_test_cap|key_test|pin_test|fix_tp_proc_info|str2hex|katoi"
 *   fix_tp_proc_info
 *   mutual_test
 *
 * `fix_tp_proc_info` (0xffffff8008a5151c) NON e' del driver: sta nel nucleo
 * `tpd` MediaTek e nel nostro albero ALPS non esiste. E' un debito gia'
 * dichiarato in `ilitek_ic.c`, non un effetto di questo lotto.
 *
 * CONTANDO ANCHE CIO' CHE IL CODICE NUOVO CITA -- che e' la lezione del lotto
 * F5, dove scrivere una funzione fece SALIRE gli irrisolti da uno a cinque:
 * `key_test` chiama `create_mp_test_frame_buffer`, `compare_MaxMin_result`,
 * `ilitek_dump_data`, `ilitek_tddi_ic_check_busy`,
 * `ilitek_tddi_ic_check_int_stat`, `printk`, `kfree`, `__kmalloc` e
 * `__memset`; `pin_test` chiama `check_int_level`,
 * `ilitek_plat_irq_register`, `ilitek_plat_irq_unregister`,
 * `ilitek_tddi_ic_check_int_stat` e `printk`. TUTTE sono gia' definite in
 * questo oggetto o negli altri sette, o sono del nucleo: il conto dei citati
 * `U` resta fermo a 173 e NESSUN irrisolto nuovo compare.
 *
 * ---------------------------------------------------------------------------
 * I TRE VERIFICATORI DI CITAZIONE, DOPO QUESTO LOTTO
 * ---------------------------------------------------------------------------
 *   MP=albero-ilitek-f/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.c
 *   ./venv/bin/python3 verificacitazioni.py $MP oracolo/stock.elf | tail -5
 *
 *   letterali: 333   citati: 286   verificati: 278   probanti: 263   deboli: 15
 *   di cui verificate come messaggio assemblato dalla macro di log (coda + binario, non un singolo letterale C): 128
 *   di cui verificate come nome di funzione da __func__ (non un letterale scritto a mano): 30
 *   rinviate a verificaistruzioni.py (descrivono un'istruzione, non un letterale): 2029 -- non contate qui, ne' come verificate ne' come mancanti
 *   soglia imposta: 333 citati richiesti (333 letterali - 0 eccezioni)
 *
 * ISTOGRAMMA COMPLETO DELLE ETICHETTE, perche' la coda da sola non lo dice:
 *
 *   ./venv/bin/python3 verificacitazioni.py $MP oracolo/stock.elf | grep -oE "^  [A-Z_]+" | sort | uniq -c
 *        15   DEBOLE
 *         1   INDIRIZZO_AMBIGUO
 *         7   NON_ANCORATA
 *       128   VERIFICATA_ASSEMBLATA
 *        30   VERIFICATA_FUNC
 *
 * ZERO `DIVERGENTE`. Le `NON_ANCORATA` sono SETTE come dopo F3c, ma la
 * COMPOSIZIONE e' cambiata e va detta:
 *   - sparite le TRE dei formati di `pin_test` ("delay_time = 0x%x\n",
 *     "test_int_pin = 0x%x\n", "int_pulse_test = 0x%x\n"): adesso quei
 *     letterali esistono nel codice, e le tre citazioni del cappello di
 *     `struct ilitek_mp_item` sono passate da NON_ANCORATA a DIVERGENTE
 *     perche' citavano la CODA del messaggio all'indirizzo del messaggio
 *     INTERO. Sono state riscritte per intero col prefisso
 *     "\x016ILITEK: (%s, %d): " e ora verificano come VERIFICATA_ASSEMBLATA.
 *     E' ESATTAMENTE la stessa correzione che il lotto F3c dovette fare sulle
 *     sei "Failed to allocate <campo> mem", e la classe di difetto e' C1: la
 *     correzione che deve raggiungere l'artefatto derivato;
 *   - comparse TRE nuove, "1a8ba56b cinc"@0xffffff8008a64c24,
 *     "5a8b056b cneg"@0xffffff8008a64c14 e "5a8d05ad cneg"@0xffffff8008a64c0c.
 *     Sono citazioni di ISTRUZIONE, non di letterale, e sono un DELTA DI
 *     STRUMENTO gia' dichiarato dal lotto F3c per la sua gemella
 *     "5a90060e cneg"@0xffffff8008a65ddc: il dizionario di
 *     `confinecitazioni.py` non conosce i due mnemonici. Rimisurato oggi:
 *
 *       ./venv/bin/python3 -c "import confinecitazioni as c; print(c.mnemonico_valido('cneg'), c.mnemonico_valido('cinc'), c.mnemonico_valido('csel'))"
 *       False False True
 *
 *     Aggiungerli al dizionario resta il delta; NON e' fatto qui perche'
 *     `confinecitazioni.py` e' uno strumento condiviso;
 *   - restano le TRE sui nomi `codeToOhm`, `mp_cdc_init_cmd_common` e
 *     `allnode_mutual_cdc_data`, che sono le incorporate di `mutual_test`.
 *
 * IL COMANDO HA SEI `--controfattuale`, NON DUE, e le quattro nuove sono di
 * questo lotto: sono le istruzioni del NOSTRO oggetto incollate nella colonna
 * `N` dei confronti per codifica qui sopra, cioe' quattro codifiche che nel
 * binario di fabbrica NON devono esserci. Non dichiararle le farebbe contare
 * fra le `assenti`, e lo strumento avrebbe ragione.
 *
 *   ./venv/bin/python3 verificaistruzioni.py $MP oracolo/stock.elf \
 *       --intervallo 0xffffff8008a5da3c:0xffffff8008a6714c \
 *       --controfattuale "71002c5f cmp" --controfattuale "7100645f cmp" \
 *       --controfattuale "b902dd2a str" --controfattuale "b902dd0a str" \
 *       --controfattuale "f9418509 ldr" --controfattuale "d63f0120 blr" \
 *       | tail -3
 *
 *   citazioni di istruzione trovate nel sorgente: 2114 (2114 a codifica, 0 ad indirizzo)
 *   confermate: 2082   assenti: 16   mnemonico diverso: 0   controfattuali: 16
 *   operandi -- registro diverso: 0   immediato diverso: 0   entrambi/non confrontabili: 0
 *
 * TUTTE LE CITAZIONI DI ISTRUZIONE NUOVE DI QUESTO LOTTO SONO CONFERMATE. Le
 * `assenti` restano SEDICI, che sono le stesse righe di `ilitek_plat_dev_exit`
 * e `ilitek_plat_dev_init` fuori dall'intervallo del gruppo F gia' dichiarate
 * dal lotto F3c (il lotto F3c ne contava 1799 in tutto, con 16 assenti).
 *
 *   python3 lavoro-ilitek-f3cbis/ancoraggio.py $MP \
 *       lavoro-ilitek-f3cbis/gruppoF.asm lavoro-ilitek-f3cbis/oltre.asm
 *
 *   citazioni "<codifica> <mnemonico>"@0xINDIRIZZO: 2033
 *   ancorate (codifica e mnemonico all'indirizzo citato): 2033
 *   difformi: 0   indirizzo fuori dai disassemblati forniti: 0
 *
 * IL TERZO CONTROLLO HA TROVATO UN DIFETTO VERO, ed e' la ragione per cui va
 * eseguito sempre: alla prima esecuzione rispondeva
 *
 *   DIFFORME @0xffffff8008a64f98: citato 5280fee2 mov -- nel binario aa1403e1 mov
 *   difformi: 1
 *
 * cioe' la citazione del `__LINE__ = 2039` di `pin_test` portava l'indirizzo
 * dell'istruzione SEGUENTE (`aa1403e1 mov x1, x20`) invece del proprio. La
 * codifica era giusta, il mnemonico era giusto, e `verificaistruzioni.py` --
 * che cerca la codifica NEL BLOCCO, non a un indirizzo -- la confermava. E' la
 * classe di difetto B8, «una citazione che prova la costante giusta con
 * l'istruzione sbagliata», e nessuno degli altri due strumenti la vede.
 * Corretta in 0xffffff8008a64f94, difformi torna a 0.
 * `lavoro-ilitek-f3cbis/ancoraggio.py` e' la SESTA copia dello stesso script,
 * copiata da `lavoro-ilitek-f3c` e verificata byte per byte
 * (md5 64b6f177df668b1a0d26d99241ce7ceb).
 *
 * ---------------------------------------------------------------------------
 * LE SCOPERTE DI QUESTO LOTTO
 * ---------------------------------------------------------------------------
 * F3cbis-S1. `allnode_key_cdc_data` NON E' UNA COPIA DI `allnode_open_cdc_data`.
 *   Le due hanno la stessa ossatura ma quattro differenze misurate: il comando
 *   e' costruito in codice invece che letto dal .ini; i codici d'errore sono
 *   il valore RESO dalla chiamata invece dei numeri fissi -100/-107/-108/-110;
 *   la lunghezza viene da `core_mp.key_len * 2` invece che dal prodotto
 *   righe per colonne; il buffer di trama e' allocato dentro la funzione
 *   invece che da `create_mp_test_frame_buffer`. Scriverla per analogia
 *   avrebbe dato quattro difetti che la dimensione non denuncia.
 *
 * F3cbis-S2. TRE DIFETTI DELLA FABBRICA IN `allnode_key_cdc_data`, tutti
 *   riprodotti (regola 7): il ramo di allocazione fallita di `ori` non tocca
 *   `ret` e il collaudo prosegue come se la lettura fosse riuscita;
 *   l'azzeramento del buffer copre `key_len` byte invece di `key_len * 4`; il
 *   ciclo di combinazione conta fino a `core_mp.c256` mentre il vettore e'
 *   dimensionato su `core_mp.key_len`. I dettagli e le citazioni stanno sopra
 *   la funzione.
 *
 * F3cbis-S3. DUE DIFETTI DELLA FABBRICA IN `key_test`: la copia nel buffer
 *   dell'elemento non e' indicizzata per trama (ogni giro riscrive gli stessi
 *   elementi, "b82a6968 str"@0xffffff8008a64d44 senza il contatore di trama) e
 *   il controllo sul numero di trame e' `== 0` e non `< 1`
 *   ("34001e61 cbz"@0xffffff8008a649c4), quindi un conteggio negativo passa.
 *
 * F3cbis-S4. TRE DIFETTI DELLA FABBRICA IN `pin_test`: il primo messaggio non
 *   ha il fine riga ("PIN test start", 34 byte contati dai byte a
 *   0xffffff8009244890); il messaggio finale scrive "defualt" per "default"
 *   (0xffffff80092449c2); e `cmd[2]` -- il ritardo -- e' scritto una volta
 *   sola, cosi' che le due prove "a fronte di salita" e "a fronte di discesa"
 *   mandano al chip byte identici.
 *
 * F3cbis-S5. LE TRE `printk` DI `pin_test` CHE NOMINANO I CAMPI SONO
 *   `ILI_INFO`, NON `ILI_DBG`: in tutta la funzione non compare nemmeno una
 *   `ldrb` di `ilitek_dbg_en`. E' la stessa distinzione che il gruppo A aveva
 *   dovuto leggere sito per sito, ed e' invisibile alla misura di dimensione
 *   solo finche' la guardia non c'e'.
 *
 * ---------------------------------------------------------------------------
 * COSA QUESTO LOTTO NON HA SCRITTO -- e resta UNA funzione sola
 * ---------------------------------------------------------------------------
 *   mutual_test   0xffffff8008a620d4   4584 byte   `t`
 *
 * Porta dentro di se' `allnode_mutual_cdc_data`, `codeToOhm` e
 * `mp_cdc_init_cmd_common` (i tre `__func__` gia' citati nel file). NON e'
 * scritta, e' DICHIARATA E LASCIATA INDEFINITA (regola 6), e il link degli
 * otto oggetti fallisce su di lei: e' l'esito onesto.
 *
 * QUESTO LOTTO NON HA TOCCATO `ilitek.h`: nessun delta di header nuovo. Le
 * due funzioni usano `ilitek_plat_irq_register` e `ilitek_plat_irq_unregister`,
 * che `ilitek.h` gia' dichiara alle righe 1042 e 1043.
 *
 * ===========================================================================
 *
 * ATTENZIONE, IL FATTO PIU' IMPORTANTE DEL LOTTO F5 E' UNA NOTIZIA CATTIVA E
 * VA LETTA PER PRIMA: scrivere `ilitek_tddi_mp_test_main` NON fa linkare il
 * driver. Quella funzione INSTALLA nei cinquanta elementi di collaudo i
 * puntatori alle sette funzioni di prova, e cinque di esse -- `mutual_test`,
 * `open_test_sp`, `open_test_cap`, `key_test`, `pin_test`, 12452 byte in
 * tutto -- non sono scritte da nessuno. Il conto degli irrisolti ILITEK passa
 * da UNO a CINQUE. La misura, con i due insiemi a confronto, sta nella
 * sezione "IL LINK" piu' sotto.
 * [AGGIORNAMENTO: erano cinque al lotto F5; F3b ne ha scritta una
 * (`open_test_cap`), F3c una (`open_test_sp`) e F3c-bis due (`key_test` e
 * `pin_test`). Gli irrisolti ILITEK sono ora UNO, `mutual_test`, e la misura
 * corrente sta nella sezione "IL LINK -- GLI IRRISOLTI ILITEK PASSANO DA DUE
 * A UNO" in cima a questo cappello.] La consegna del lotto F5 diceva «mancano due
 * soli simboli perche' il driver si linchi»: contava i simboli citati dal
 * codice GIA' SCRITTO, non quelli che il codice DA SCRIVERE avrebbe citato.
 *
 * ===========================================================================
 * LA REVISIONE DEL LOTTO F5 -- 2026-08-22, COSA E' CAMBIATO
 * ===========================================================================
 * Una revisione indipendente ha esaminato i lotti F4 e F5 (F4: accettabile;
 * F5: da correggere) e ha dichiarato QUINDICI difetti. Sono stati rifatti
 * TUTTI E QUINDICI da zero su questo albero, con i comandi e le uscite
 * letterali che stanno nelle sezioni qui sotto: QUINDICI SU QUINDICI SI
 * RIPRODUCONO, nessuno e' respinto. Rifacendo le prove ne sono emersi altri
 * TRE che la revisione non aveva visto, due dei quali di sostanza. Elenco.
 *
 * DI SOSTANZA -- CAMBIANO IL CODICE EMESSO (cinque):
 *
 *  S1. `mp_print_csv_cdc_cmd` usciva dalla funzione con `return` dove la
 *      fabbrica CONTINUA il ciclo. La prova sta nel bersaglio del salto DOPO
 *      la `printk`: "17fffe38 b"@0xffffff8008a613e4 porta a
 *      0xffffff8008a60cc4, che e' il `memset` dell'ITERAZIONE SEGUENTE, non
 *      l'uscita; e "14000031 b"@0xffffff8008a60d30 porta a
 *      0xffffff8008a60df4, idem. Con `continue` i blocchi di messaggio
 *      srotolati diventano QUATTRO per la riga 1124 e DUE per la 1133,
 *      esattamente come di fabbrica. E' anche una divergenza di
 *      COMPORTAMENTO: con `return` un solo comando CDC mancante dal file .ini
 *      faceva saltare tutto il resto del campo CSV.
 *      [Nota sulla prova della revisione, che era imprecisa in due punti pur
 *      avendo ragione nella sostanza: 0xffffff8008a613dc e
 *      0xffffff8008a60d2c, che il rapporto indica come sorgenti dei salti,
 *      sono le due `bl <printk>`; i salti stanno quattro e otto byte dopo.]
 *
 *  S2. Nel secondo ciclo il messaggio d'errore nominava `open_c_cmd[i]`,
 *      mentre la fabbrica stampa `open_sp_cmd[i]`: il messaggio dice
 *      "open dac"@0xffffff800924446b e "open raw1"@0xffffff8009244474 mentre
 *      la chiave interrogata e' "open cap1 dac"@0xffffff8009244492 e
 *      "open cap1 raw"@0xffffff80092444a0. E' un errore di copia-incolla
 *      DELLA FABBRICA e va riprodotto (regola 7); il lotto F5 lo aveva
 *      tacitamente corretto.
 *
 *  S3. [TROVATO DALLA REVISIONE DELLA REVISIONE] Due dei tre `ILI_ERR` di
 *      `parser_get_ini_phy_data` stampavano "MAX_KEY_VALUE_LEN: Out Of
 *      Length" dove la fabbrica stampa "MAX_KEY_NAME_LEN: Out Of Length". I
 *      tre siti, per numero di riga e per stringa caricata in x0:
 *
 *        ffffff8008a61f50:	91337800 	add	x0, x0, #0xcde   __LINE__ 711
 *        ffffff8008a61f38:	91337800 	add	x0, x0, #0xcde   __LINE__ 776
 *        ffffff8008a60b6c:	91364400 	add	x0, x0, #0xd91   __LINE__ 787
 *
 *      e 0xffffff8009244cde e' "MAX_KEY_NAME_LEN: Out Of Length\n",
 *      0xffffff8009244d91 e' "MAX_KEY_VALUE_LEN: Out Of Length\n" (lette con
 *      `leggi_stringa.py`). L'oggetto consegnato da F5 non conteneva affatto
 *      la stringa "MAX_KEY_NAME_LEN": `strings ilitek_mp.o | grep MAX_KEY`
 *      dava due righe invece di tre. Il difetto non cambia un byte di
 *      `.text`, quindi nessuna misura di dimensione lo denunciava: e' la
 *      classe B1/B6, "quello che la dimensione NON denuncia".
 *
 *  S4. [TROVATO DALLA REVISIONE DELLA REVISIONE] In `mp_show_result`, prima
 *      della riga "Result_Summary", il sorgente scriveva
 *      `memset(str, 0, sizeof(str))` -- un vettore di PILA -- dove la
 *      fabbrica azzera i 128 byte del GLOBALE `csv_path` con otto
 *      `stp xzr, xzr` srotolate a partire da "f000b4c8 adrp"@0xffffff8008a61b34
 *      + "9131c108 add"@0xffffff8008a61b38 (0xffffff800a0fcc70 = `csv_path`).
 *      Il nostro oggetto emetteva SEDICI `str xzr, [sp,#...]`. Corretto: la
 *      funzione passa da 14860 a 14836 byte.
 *
 *  S5. [TROVATO DALLO STRUMENTO, appena scritto] Una riga di `objdump`
 *      incollata a mano in un commento di questa revisione portava la
 *      codifica `d0003902` invece di `f000b4c8`, e `verificaistruzioni.py` ha
 *      risposto "REGISTRO DIVERSO ... citati ['x8'], nel blocco ['x2']".
 *      Corretta prima della consegna. E' la ragione per cui gli strumenti si
 *      rilanciano dopo OGNI modifica, anche a un commento.
 *
 * DI CITAZIONE -- LA PROVA NON STAVA DOVE DICEVA (sette):
 *
 *  C1. "Result_Summary\t\t\t\n" era citata a 0xffffff8009245985, due byte
 *      oltre l'inizio: la stringa comincia a 0xffffff8009245983, ed e' 0x983
 *      il valore che "91260c42 add"@0xffffff8008a61b90 materializza.
 *  C2. "open raw3" era citata a 0xffffff8009244486, due byte PRIMA: a
 *      quell'indirizzo c'e' il byte '2' (la coda di "open raw2"); la stringa
 *      comincia a 0xffffff8009244488, ed e' 0x488 il valore che
 *      "91122021 add"@0xffffff8008a60ed0 materializza.
 *  C3..C7. CINQUE citazioni di letterale avevano l'indirizzo TRONCATO con i
 *      puntini (`@0xffffff8009245...`). Un indirizzo con l'ellissi non e' un
 *      indirizzo: `verificacitazioni.py` leggeva il prefisso come numero e le
 *      segnalava DIVERGENTE, e `ancoraggio.py` non le vedeva affatto perche'
 *      pretende sedici cifre. I cinque indirizzi veri, ritrovati dai siti di
 *      chiamata e letti dai byte, sono 0xffffff8009244d13 (seq_item is over
 *      than its define), 0xffffff800924563a (Open CSV : %s),
 *      0xffffff800924565d (Failed to open CSV file), 0xffffff8009245689
 *      (Open CSV succeed) e 0xffffff80092456fc (Writing Data into CSV
 *      succeed).
 *
 *  C8. Le due `ldrsw` citate come prova del SEGNO dei campi di
 *      `struct ilitek_ini_item` erano attaccate al campo sbagliato -- erano
 *      spostate di un campo. Conseguenza: il segno di `c2200` non era provato
 *      da niente, e adesso e' dichiarato per quello che e' (una scelta). La
 *      nota per esteso sta sulla struttura.
 *
 * DI RENDICONTO -- IL NUMERO O LA FRASE NON REGGEVANO (sei):
 *
 *  R1. Il cappello dichiarava di `verificacitazioni.py` solo le nove
 *      `NON_ANCORATA` e taceva SETTE `DIVERGENTE` e una `INDIRIZZO_AMBIGUO`.
 *      Adesso c'e' l'istogramma completo delle etichette, e il conto chiude.
 *  R2. «piu' 597 di istruzione: da 797 a 1396»: 1396 - 797 = 599.
 *  R3. «cinque funzioni di collaudo piu' `str2hex` ... il link fallisce su
 *      quei nomi»: `str2hex` e' definita in `ilitek_node.o`. Gli irrisolti
 *      ILITEK sono cinque.
 *  R4. «scrivendo quei `case` per primi la disposizione della tabella di
 *      salto combacia»: combacia la STRUTTURA, non la disposizione. Di
 *      fabbrica il gruppo di sei e' la voce 0x00 (primo blocco), da noi 0x3b
 *      (ultimo).
 *  R5. «per `ipio_vfree` la sequenza compare cinquanta volte»: compare DUE
 *      volte; i `bl <vfree>` nella funzione sono sei. (La frase gemella su
 *      `parser_get_tdf`, «sei volte», si riproduce esatta: 6 di fabbrica, 6
 *      nostre, tutte `7100b99f cmp`.)
 *  R6. «il resto delle dodici istruzioni non e' localizzato», con lo scarto
 *      netto di -48 byte a fare da vetrina. Il netto nascondeva un divario
 *      per mnemonico molto piu' grande. La sezione "COSA RESTA DI DIVERSO" e'
 *      stata riscritta da capo, e adesso dice anche cio' che non si sa.
 *
 * DI FORMA -- LA PROSA CONTRADDETTA DAL DISASSEMBLATO (tre):
 *
 *  F1. «le tre `stp` che la precedono»: la seguono.
 *  F2. «fra le due ci sono SOLO due `add`»: ce ne sono tre, la prima e' una
 *      `tbnz` (che non scrive w2, quindi la conclusione regge).
 *  F3. Il cappello portava DUE uscite dello stesso `ancoraggio.py`, una
 *      aggiornata e una ferma al lotto F2, e indicava lo script in TRE
 *      directory diverse. Le quattro copie sono byte per byte la stessa
 *      (md5 64b6f177df668b1a0d26d99241ce7ceb); l'uscita vecchia e' stata
 *      tolta.
 *
 * IL PREZZO, DETTO SUBITO: le correzioni S1 e S4 cambiano la dimensione, e
 * NON in meglio. `ilitek_tddi_mp_test_main` passa da 14656 byte (-48 dalla
 * fabbrica) a 14836 (+132). E' un fatto e sta scritto anche nella tabella
 * delle misure; una correzione giusta nel merito non si annulla perche' un
 * numero peggiora (regola 3).
 *
 * ===========================================================================
 * COSA C'E' QUI DENTRO, E COSA NO
 * ===========================================================================
 * Il gruppo F e' l'intero collaudo MP: 24 funzioni emesse, 38672 byte,
 * 0xffffff8008a5da3c..0xffffff8008a6714c (estremo superiore escluso). E' il
 * blocco piu' grande del progetto, e la ricognizione
 * (`scout-ilitek-F/RIASSUNTO.md`, sul PC di build) proponeva di spezzarlo in
 * cinque lotti. QUESTO FILE NE CHIUDE DUE E MEZZO: F1 e F5 per intero, piu'
 * NOVE funzioni prese da F2, F3 e F4. SEDICI su 24, 22672 byte su 38672
 * (58,6%).
 *
 *   lotto  funzione                    indirizzo             byte  vis.
 *   F1     dump_benchmark_data         0xffffff8008a6573c     300   t
 *   F1     dump_node_type_buffer       0xffffff8008a5da3c     180   T
 *   F1     parser_get_ini_key_value    0xffffff8008a61f88     332   t
 *   F1     parser_ini_nodetype         0xffffff8008a5daf0     500   T
 *   F1     parser_ini_benchmark        0xffffff8008a5dce4     860   T
 *   F1     parser_get_u8_array         0xffffff8008a659d0     292   t
 *   F2     compare_charge              0xffffff8008a5e040     192   T
 *   F2     full_open_rate_compare      0xffffff8008a5e100      80   T
 *   F2     check_int_level             0xffffff8008a5e528     240   T
 *   F2     compare_MaxMin_result       0xffffff8008a65868     360   t
 *   F2     mp_compare_cdc_show_result  0xffffff8008a66d20    1068   t
 *   F3     self_test                   0xffffff8008a64e94      44   t
 *   F3     st_test                     0xffffff8008a64ec0      44   t
 *   F4     mp_do_retry                 0xffffff8008a66c84     156   t
 *   F4     mp_comp_result_before_retry 0xffffff8008a65f8c    3320   t
 *   F5     ilitek_tddi_mp_test_main    0xffffff8008a5e618   14704   T
 *                                             somma: 22672
 *
 * QUESTA TABELLA E' FERMA AL LOTTO F5 E NON E' PIU' IL CONTO CORRENTE. Dopo
 * F3a, F3b, F3c e F3c-bis le funzioni emesse scritte sono VENTITRE su 24 e i
 * byte 34088 su 38672 (88,1%); le SETTE aggiunte dopo questa tabella sono
 * `create_mp_test_frame_buffer` (1388), `allnode_open_cdc_data` (1176),
 * `allnode_open_cdc_result` (984), `open_test_sp` (3932), `open_test_cap`
 * (1828), `key_test` (1368) e `pin_test` (740). Il conto sta per esteso nella
 * sezione "IL LOTTO F3c-bis" in cima a questo cappello.
 *
 * L'ultima riga vale per TRE funzioni del sorgente di fabbrica, non per una:
 * `mp_comp_result_before_retry` e' un simbolo solo, ma dentro ci sono i corpi
 * incorporati di `mp_compare_cdc_result` (sette siti) e di
 * `mp_test_data_sort_average` (un sito), piu' `ipio_kfree` (due siti). I tre
 * `__func__` distinti che il blocco contiene sono la prova, ed e' la classe di
 * difetto B5.
 *
 * IL CRITERIO DEL TAGLIO, dichiarato: sono state scritte tutte e sole le
 * funzioni che questo lotto ha potuto DETERMINARE dal binario senza
 * inventare. Le tre che restano di F2 -- `allnode_open_cdc_data`,
 * `allnode_open_cdc_result`, `create_mp_test_frame_buffer` -- non sono state
 * scritte, e per la prima il motivo NON e' la fatica ma un fatto misurato che
 * non tornava: vedi "LA DOMANDA (NON PIU') APERTA SU parser_get_int_data"
 * in fondo -- il lotto F5 l'ha chiusa.
 *
 * Le altre NOVE funzioni emesse del gruppo F (30704 byte) NON sono qui, e
 * NON sono state scritte: l'elenco esatto sta in fondo a questo cappello,
 * sezione "IL RESIDUO DICHIARATO". Un lotto parziale dichiarato vale piu' di
 * uno completo inventato.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA. Nessun sorgente
 * pubblico e' stato letto. Driver Ilitek TDDI circolano su GitHub e ALPS
 * contiene driver touch della stessa famiglia: non sono stati aperti. Ogni
 * riga derivata porta accanto la riga di disassemblato che la giustifica,
 * nella forma "<8 cifre esadecimali> <mnemonico>"@0xINDIRIZZO -- la sola che
 * `verificaistruzioni.py` sappia confrontare col binario.
 *
 * SU QUELL'AFFERMAZIONE PENDE UN SOSPETTO, e sta agli atti invece che taciuto:
 * un locale si chiamava `pToken`, l'unico camelCase del file, e una revisione
 * indipendente ha fatto notare che il driver pubblico usa quel nome nello
 * stesso punto. Il nome non viene dal binario, e' stato rinominato in `tok`, e
 * il sospetto NON e' chiuso: la nota per esteso e' sopra
 * `parser_get_u8_array`.
 *
 * ===========================================================================
 * LA SCOPERTA DI QUESTO LOTTO: IL COMPILATORE DI FABBRICA, E DOVE STA
 * ===========================================================================
 * Il kernel di fabbrica dice da solo con che cosa e' stato costruito. In
 * `oracolo/stock.elf`, nella riga di banner:
 *
 *   Linux version 4.14.141 (nobody@android-build) (Android (5484270 based on
 *   r353983c) clang version 9.0.3 (...745b335211bb9eadfa6aa6301f84715cee4b37c5)
 *   (...60cf23e54e46c807513f7a36d0a7b777920b5881) (based on LLVM 9.0.3svn))
 *   #1 SMP PREEMPT Tue Jun 23 09:55:04 CST 2020
 *
 * L'albero di build del progetto usava invece `clang-r383902` (LLVM 11). Due
 * versioni maggiori di scarto, e si vedono:
 *
 *   - LLVM 9 mette FP/LR in CIMA all'area dei registri salvati
 *     ("a9037bfd stp"@0xffffff8008a5da48, x29/x30 a [sp,#48], poi
 *     "9100c3fd add"@0xffffff8008a5da4c, x29 = sp+0x30); LLVM 11 li mette in
 *     fondo (`stp x29, x30, [sp,#-64]!` e `mov x29, sp`). Costo: zero byte.
 *   - LLVM 11 dispone i blocchi dei cicli in un altro ordine (mette la coda
 *     del ciclo PRIMA del corpo e paga due salti in piu' per ciclo) e
 *     duplica le code di ritorno. Costo: da +4 a +12 byte per funzione.
 *
 * Il toolchain di fabbrica esiste ancora ed e' stato scaricato:
 * `platform/prebuilts/clang/host/linux-x86`, ramo `refs/heads/android10-release`,
 * directory `clang-r353983c`. Sul PC di build sta in
 * `/mnt/s88pro/kernel-stock/clang-fabbrica-f/`. Il `manifest_5484270.xml`
 * che l'archivio porta con se' ha lo STESSO numero di build (5484270) del
 * banner, e `bin/clang --version` stampa la stringa identica, byte per byte,
 * a quella dentro `stock.elf`.
 *
 * Con quel compilatore, e senza toccare una riga di questo file, tutte e sei
 * le funzioni misurano ESATTAMENTE come di fabbrica. Con clang-r383902
 * cinque su sei sbagliavano da 4 a 12 byte. La differenza NON era nel
 * sorgente. E' un fatto che vale per ogni lotto del progetto, non solo per
 * questo, e va detto: una misura fatta con LLVM 11 puo' assolvere un
 * sorgente sbagliato (qui `parser_ini_nodetype` misurava 500 su 500 con LLVM
 * 11 ED ERA SBAGLIATA: con LLVM 9 misurava 492 su 500, e la correzione --
 * vedi la nota nel corpo -- l'ha riportata a 500 su 500).
 *
 * ===========================================================================
 * COME SI RICOMPILA E SI RIMISURA -- I COMANDI ESATTI
 * ===========================================================================
 * Compilazione con il compilatore DI FABBRICA (quella che conta):
 *
 *   ssh alops@95.239.75.21 'docker run --rm --user $(id -u):$(id -g) \
 *     -v /home/alops/lineage:/srv/twrp -v /mnt/s88pro:/srv/archive -e HOME=/tmp \
 *     -e PATH=/srv/archive/kernel-stock/clang-fabbrica-f/bin:/srv/twrp/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin:/usr/bin:/bin:/usr/sbin:/sbin \
 *     s88pro-build bash -lc "make -C /srv/archive/kernel-stock/albero-ilitek-f \
 *       O=/srv/archive/kernel-stock/out-ilitek-f9 ARCH=arm64 -j6 CC=clang \
 *       HOSTCC=clang CLANG_TRIPLE=aarch64-linux-gnu- \
 *       CROSS_COMPILE=aarch64-linux-android- \
 *       drivers/input/touchscreen/mediatek/ilitek_e977/"'
 *
 * Misura dal `.o` vero:
 *
 *   ssh alops@95.239.75.21 '/mnt/s88pro/lineage21/prebuilts/gcc/linux-x86/\
 *     aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android-nm \
 *     --print-size /mnt/s88pro/kernel-stock/out-ilitek-f9/drivers/input/\
 *     touchscreen/mediatek/ilitek_e977/ilitek_mp.o | grep -iE " [tT] "'
 *
 * Le due verifiche di citazione (dal PC di build, in /mnt/s88pro/kernel-stock).
 * Le tre `--eccezione` sono i letterali KERN_CONT: il perche' sta nella
 * sezione "I TRE LETTERALI KERN_CONT" in fondo a questo cappello.
 *
 *   ./venv/bin/python3 verificacitazioni.py ilitek_mp.c oracolo/stock.elf \
 *       --eccezione '%d, ' --eccezione '\n' \
 *       --eccezione 'Dump Denchmark Min\n'
 *   ./venv/bin/python3 verificaistruzioni.py ilitek_mp.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a5da3c:0xffffff8008a6714c \
 *       --intervallo 0xffffff80093833c8:0xffffff80093833d8 \
 *       --intervallo 0xffffff80093acac4:0xffffff80093acb5c
 *
 * AGGIORNATO ALLA REVISIONE DEL LOTTO F5, 2026-08-22. Uscita LETTERALE delle
 * ultime righe, sul file di OGGI:
 *
 *   letterali: 259   citati: 223   verificati: 213   probanti: 199   deboli: 14
 *   di cui verificate come messaggio assemblato dalla macro di log (coda + binario, non un singolo letterale C): 81
 *   di cui verificate come nome di funzione da __func__ (non un letterale scritto a mano): 25
 *   rinviate a verificaistruzioni.py (descrivono un'istruzione, non un letterale): 1402 -- non contate qui, ne' come verificate ne' come mancanti
 *   soglia imposta: 256 citati richiesti (259 letterali - 3 eccezioni)
 *
 *   citazioni di istruzione trovate nel sorgente: 1430 (1430 a codifica, 0 ad indirizzo)
 *   confermate: 1430   assenti: 0   mnemonico diverso: 0   controfattuali: 0
 *   operandi -- registro diverso: 0   immediato diverso: 0   entrambi/non confrontabili: 0
 *
 * ESITO=1 per `verificacitazioni.py`, ESITO=0 per `verificaistruzioni.py`.
 * Attenzione a come si legge quell'esito: se lo si prende in fondo a una pipe
 * (`| tail -5; echo $?`) si legge il codice di `tail`, non quello dello
 * strumento -- errore fatto e corretto qui, e rifatto e ricorretto in F4.
 *
 * PERCHE' L'ESITO E' 1 -- E TUTTO QUELLO CHE LO STRUMENTO DICE, NON UNA PARTE.
 * Questo paragrafo, nella stesura del lotto F5, dichiarava SOLO le nove
 * `NON_ANCORATA` e spiegava l'esito con la sola soglia di copertura. Era
 * falso per omissione (classe C1): lo strumento emetteva anche SETTE
 * `DIVERGENTE` e una `INDIRIZZO_AMBIGUO`, e il conto della riga di riassunto
 * lo denunciava -- 223 citati - 206 verificati = 17 = 9 + 7 + 1 -- e nessuno
 * l'aveva fatto. Le sette `DIVERGENTE` erano difetti veri, e sono corrette:
 * due indirizzi di letterale sbagliati di due byte e cinque indirizzi
 * TRONCATI con i puntini. L'elenco sta nella sezione "LA REVISIONE DEL LOTTO
 * F5" qui sotto.
 *
 * L'istogramma COMPLETO delle etichette, uscita letterale di
 *
 *   ./venv/bin/python3 verificacitazioni.py \
 *       albero-ilitek-f/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.c \
 *       oracolo/stock.elf --eccezione '%d, ' --eccezione '\n' \
 *       --eccezione 'Dump Denchmark Min\n' > /tmp/c.txt 2>&1; echo "ESITO=$?"
 *   grep -oE '^  [A-Z_]+' /tmp/c.txt | sort | uniq -c
 *
 *   ESITO=1
 *        14   DEBOLE
 *         1   INDIRIZZO_AMBIGUO
 *         9   NON_ANCORATA
 *        81   VERIFICATA_ASSEMBLATA
 *        25   VERIFICATA_FUNC
 *
 * e adesso il conto chiude: 223 - 213 = 10 = 9 NON_ANCORATA + 1
 * INDIRIZZO_AMBIGUO, con ZERO `DIVERGENTE`. L'esito resta 1 per la sola
 * ragione di copertura: lo strumento pretende che OGNI letterale del codice
 * abbia una citazione, e ne mancano 36 su 259.
 *
 * LE NOVE `NON_ANCORATA` sono LE STESSE NOVE di prima del lotto F5: i sei
 * messaggi "Failed to allocate <campo> mem" e i tre formati
 * "delay_time = 0x%x", "test_int_pin = 0x%x", "int_pulse_test = 0x%x" citati
 * nel cappello di `struct ilitek_mp_item`, che nominano campi letti da
 * `create_mp_test_frame_buffer` e da `pin_test` -- due funzioni che NON sono
 * scritte, quindi quei letterali non compaiono nel codice e la citazione non
 * si puo' ancorare.
 *
 * L'UNICA `INDIRIZZO_AMBIGUO` e' il letterale di formato di due caratteri
 * `percento-esse`, che questo file cita a DUE indirizzi diversi e veri:
 *
 *   INDIRIZZO_AMBIGUO 0x0: %s -- citata con indirizzi diversi: 0xffffff8009100e35, 0xffffff8009101762
 *
 *   $ ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf \
 *       0xffffff8009100e35 0xffffff8009101762
 *   0xffffff8009100e35: b'%s'  (len=2, hex=2573)
 *   0xffffff8009101762: b'%s'  (len=2, hex=2573)
 *
 * Il primo e' il formato della `snprintf` dentro `parser_get_int_data`, il
 * secondo quello della `ILI_CONT` che stampa "BYPASS,". E' lo stesso limite
 * dello strumento gia' dichiarato per il letterale di ritorno a capo: non sa
 * ancorare due volte lo stesso testo. NON e' un difetto del sorgente -- ma
 * taciuto sarebbe diventato uno, ed e' esattamente quello che era successo.
 *
 * Il confronto misurato, tre stadi:
 *
 *   prima di F5:       letterali: 103  citati:  71  soglia: 100  NON_ANCORATA: 9  DIVERGENTE: 0
 *   dopo  il lotto F5: letterali: 258  citati: 223  soglia: 255  NON_ANCORATA: 9  DIVERGENTE: 7
 *   dopo la revisione: letterali: 259  citati: 223  soglia: 256  NON_ANCORATA: 9  DIVERGENTE: 0
 *
 * cioe' F5 aggiunge 155 letterali e 152 citazioni di letterale, piu' 599
 * citazioni di istruzione: da 797 a 1396, e 1396 - 797 = 599. La stesura del
 * lotto F5 scriveva qui "597", e il rapporto scriveva "599": il cappello e il
 * rapporto si contraddicevano ed era il cappello a sbagliare. Aritmetica,
 * trovata dalla revisione e corretta. La revisione porta poi i letterali a
 * 259 (aggiunge "MAX_KEY_NAME_LEN: Out Of Length\n", che mancava del tutto --
 * vedi i difetti di sostanza) e le citazioni di istruzione a 1430.
 *
 * Durante il lavoro il lotto F5 aveva aggiunto TRE `NON_ANCORATA`, tutte e
 * tre trovate dallo strumento e corrette: un formato citato senza il suo "\n"
 * finale, il letterale KERN_CONT "KEY_%02d " citato a 0xffffff8009245534
 * (che e' una virgola, non quella stringa: l'indirizzo giusto della coda e'
 * 0xffffff8009245522) e una citazione di istruzione con il mnemonico `cinc`,
 * che `confinecitazioni.py` non ha in tavola e che lo strumento rileggeva
 * come letterale.
 *
 * IL TERZO CONTROLLO, `ancoraggio.py`, va eseguito SEMPRE: e' l'unico che
 * verifica che ogni citazione stia all'indirizzo che dichiara. Il motivo e'
 * un limite di `verificaistruzioni.py` (classe B8): per la forma a codifica
 * cerca la codifica OVUNQUE nel blocco e l'indirizzo dopo `@` NON viene
 * guardato. Con `--intervallo` largo come questo, ESITO=0 da solo NON prova
 * che una citazione sia ancorata all'indirizzo giusto.
 *
 * DOVE STA LO SCRIPT: ne esistono quattro copie, in `lavoro-ilitek-f2`,
 * `lavoro-ilitek-f4`, `lavoro-ilitek-f-fix` e `lavoro-ilitek-f5`, e sono
 * BYTE PER BYTE LA STESSA (md5 64b6f177df668b1a0d26d99241ce7ceb, verificato
 * su tutte e quattro). Il cappello del lotto F5 ne indicava tre diverse in
 * tre punti diversi e riportava DUE uscite dello stesso strumento, una
 * aggiornata e una ferma al lotto F2 (422 citazioni) senza dire che era
 * vecchia: chi rilanciava il controllo con il numero sbagliato lo credeva
 * fallito. Quella seconda uscita e' stata tolta. Comando e uscita CORRENTI,
 * uno solo:
 *
 *   cd /mnt/s88pro/kernel-stock
 *   OBJ=/mnt/s88pro/lineage21/prebuilts/gcc/linux-x86/aarch64/\
 *       aarch64-linux-android-4.9/bin/aarch64-linux-android-objdump
 *   $OBJ -d --start-address=0xffffff8008a5da3c \
 *        --stop-address=0xffffff8008a6714c oracolo/stock.elf \
 *        > lavoro-ilitek-f6/gruppoF.asm
 *   $OBJ -d --start-address=0xffffff8009380000 \
 *        --stop-address=0xffffff80093c0000 oracolo/stock.elf \
 *        > lavoro-ilitek-f6/oltre.asm
 *   python3 lavoro-ilitek-f6/ancoraggio.py \
 *       albero-ilitek-f/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.c \
 *       lavoro-ilitek-f6/gruppoF.asm lavoro-ilitek-f6/oltre.asm
 *
 *   citazioni "<codifica> <mnemonico>"@0xINDIRIZZO: 1398
 *   ancorate (codifica e mnemonico all'indirizzo citato): 1398
 *   difformi: 0   indirizzo fuori dai disassemblati forniti: 0
 *
 * ESITO=0. Le 32 citazioni che `verificaistruzioni.py` conta in piu' (1430
 * contro 1398) sono le righe in formato `objdump` incollate nei commenti, che
 * `ancoraggio.py` non legge perche' pretende la forma con le virgolette.
 *
 * Nessuna `--controfattuale`: questo lotto non cita nessuna istruzione che
 * NON debba stare nel binario.
 *
 * ---------------------------------------------------------------------------
 * AGGIORNAMENTO DEL LOTTO F3c (open_test_sp scritta, open_test_cap corretta)
 * ---------------------------------------------------------------------------
 * I numeri qui sopra sono quelli del lotto F5 e restano com'erano: sono la
 * storia di quel lotto. QUESTI sono i numeri del lotto F3c.
 *
 * NON SONO PIU' I NUMERI CORRENTI: il lotto F3c-bis li ha superati e i numeri
 * da confrontare rilanciando gli strumenti stanno nella sezione "I TRE
 * VERIFICATORI DI CITAZIONE, DOPO QUESTO LOTTO", in cima a questo cappello.
 *
 *   cd /mnt/s88pro/kernel-stock
 *   MP=albero-ilitek-f/drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.c
 *   ./venv/bin/python3 verificacitazioni.py $MP oracolo/stock.elf | tail -5
 *
 *   letterali: 316   citati: 268   verificati: 260   probanti: 245   deboli: 15
 *   di cui verificate come messaggio assemblato dalla macro di log (coda + binario, non un singolo letterale C): 115
 *   di cui verificate come nome di funzione da __func__ (non un letterale scritto a mano): 27
 *   rinviate a verificaistruzioni.py (descrivono un'istruzione, non un letterale): 1760 -- non contate qui, ne' come verificate ne' come mancanti
 *   soglia imposta: 316 citati richiesti (316 letterali - 0 eccezioni)
 *
 * ZERO `DIVERGENTE`. Le `NON_ANCORATA` sono SETTE, non piu' nove: le due che
 * sono sparite sono i due messaggi "Failed to allocate cap_dac buffer" e
 * "Failed to allocate cap_raw buffer", che ora SONO letterali del codice
 * perche' `open_test_cap` e' scritta. Ne e' comparsa una nuova,
 * "allnode_mutual_cdc_data"@0xffffff8009243e85, citata nel cappello dei sette
 * puntatori di trama: e' il `__func__` di una funzione incorporata in
 * `mutual_test`, che NON e' scritta, quindi il testo non compare nel codice.
 * E' la stessa classe delle altre sei e non e' un difetto nuovo.
 *
 *   ./venv/bin/python3 verificaistruzioni.py $MP oracolo/stock.elf \
 *       --intervallo 0xffffff8008a5da3c:0xffffff8008a6714c \
 *       --controfattuale "71002c5f cmp" --controfattuale "7100645f cmp" | tail -3
 *
 *   citazioni di istruzione trovate nel sorgente: 1799 (1799 a codifica, 0 ad indirizzo)
 *   confermate: 1779   assenti: 16   mnemonico diverso: 0   controfattuali: 4
 *   operandi -- registro diverso: 0   immediato diverso: 0   entrambi/non confrontabili: 0
 *
 * LE `--controfattuale` SONO NUOVE, e sono un DEBITO CHIUSO, non uno aperto:
 * sono le due `cmp` a 32 bit del NOSTRO oggetto citate nella divergenza
 * F3b-D1, cioe' due istruzioni che nel binario di fabbrica NON devono
 * esserci. Il lotto F3b le aveva scritte senza dichiararle e lo strumento le
 * contava fra le `assenti`; dichiarate, le assenti scendono da 18 a 16. Lo
 * strumento ne conta QUATTRO e non due perche' la riga di comando qui sopra
 * le ripete, e anche quella e' dentro il file: due nel testo della divergenza
 * e due nel comando che le dichiara.
 *
 * LE SEDICI `assenti` CHE RESTANO sono TUTTE righe di disassemblato di
 * `ilitek_plat_dev_exit` e `ilitek_plat_dev_init` (0xffffff80093acac4 e
 * 0xffffff80093833c8), incollate nella sezione sulla `.exit` alle righe
 * 1307..1345: stanno FUORI dall'intervallo del gruppo F, e con l'intervallo
 * giusto si confermano. Erano sedici anche prima di questo lotto.
 *
 *   python3 lavoro-ilitek-f3c/ancoraggio.py $MP \
 *       lavoro-ilitek-f3c/gruppoF.asm lavoro-ilitek-f3c/oltre.asm
 *
 *   citazioni "<codifica> <mnemonico>"@0xINDIRIZZO: 1757
 *   ancorate (codifica e mnemonico all'indirizzo citato): 1757
 *   difformi: 0   indirizzo fuori dai disassemblati forniti: 0
 *
 * `lavoro-ilitek-f3c/ancoraggio.py` e' la QUINTA copia dello stesso script,
 * copiata da `lavoro-ilitek-f6` e verificata byte per byte
 * (md5 64b6f177df668b1a0d26d99241ce7ceb per tutte e due).
 *
 * IL BILANCIO DEL LOTTO F3c sulle citazioni: da 1662 a 1799 citazioni di
 * istruzione (+137) e da 258 a 268 citazioni di letterale (+10), con
 * `DIVERGENTE` fermo a zero, `difformi` fermo a zero e nessuna `assente`
 * nuova.
 *
 * TRE intervalli, non uno. Il primo e' il gruppo F INTERO e non solo le sei
 * funzioni, perche' alcune citazioni stanno nei SITI DI CHIAMATA (dentro
 * `mutual_test`, `open_test_sp`, `key_test`, `ilitek_tddi_mp_test_main`) e
 * sono loro a decidere i tipi degli argomenti. Gli altri due coprono le venti
 * righe di disassemblato incollate nella sezione sulla `.exit`, che stanno
 * OLTRE `_einittext` e che lo strumento legge come citazioni -- ed e' giusto
 * cosi': anche quelle vanno confrontate col binario. Con F2 il conto e'
 * passato da 211 a 442 citazioni.
 *
 * LE TRE ECCEZIONE SONO ANCORA TRE, e non e' un caso: il lotto F2 aggiunge
 * OTTO letterali sotto `KERN_CONT`, che con la scrittura di F1 avrebbero
 * richiesto otto eccezioni in piu'. La macro locale `ILI_CONT` li rende
 * ancorabili alla CODA della stringa nel binario -- il perche' sta nel suo
 * cappello, sopra le dichiarazioni. Restano scusati solo i tre letterali di
 * F1, piu' `\n`, che questo file usa a DUE indirizzi diversi
 * (0xffffff80090ddcf3 e 0xffffff8009245994) e che lo strumento non sa
 * ancorare due volte: la nota per esteso e' dentro
 * `mp_compare_cdc_show_result`.
 *
 * E `ancoraggio.py` HA TROVATO UN DIFETTO VERO IN QUESTO LOTTO, prima che
 * fosse corretto: la citazione della `bl` a `strlen` dentro il cappello di
 * `struct ilitek_mp_item` portava la codifica `94003484` (che nel gruppo F
 * esiste davvero, ma e' la `bl katoi` a 0xffffff8008a5e3a8) invece di
 * `940fe858`. `verificaistruzioni.py` la dava CONFERMATA -- e' la classe di
 * difetto B8. Senza `ancoraggio.py` sarebbe passata. E in questa revisione
 * `verificaistruzioni.py` ne ha trovato un altro, appena scritto: una riga di
 * `objdump` incollata a mano nel commento di `memset(csv_path, ...)` portava
 * la codifica `d0003902` invece di `f000b4c8`, e lo strumento ha risposto
 * "REGISTRO DIVERSO ... citati ['x8'], nel blocco ['x2']".
 *
 * ===========================================================================
 * COSA QUESTO LOTTO HA CREATO SUL PC DI BUILD -- L'ELENCO INTERO
 * ===========================================================================
 * La prima stesura dichiarava "le due cose nuove sul PC sono mie e portano il
 * mio suffisso". Erano tre, e la terza non aveva suffisso. Elenco completo,
 * sotto `/mnt/s88pro/kernel-stock/`:
 *
 *   out-ilitek-f9/           uscita di build con il clang DI FABBRICA
 *   out-ilitek-f/            uscita di build con clang-r383902 (LLVM 11)
 *   clang-fabbrica-f/        il toolchain di fabbrica, scaricato e scompattato
 *   verifica-f1-ilitek_mp.c  copia di lavoro di questo file, lasciata nella
 *                            RADICE condivisa invece che in una directory con
 *                            suffisso (md5 uguale alla copia del repository al
 *                            momento in cui fu scritta). Innocua, ma non era
 *                            stata dichiarata: la regola del progetto e' che
 *                            chi dichiara cosa ha toccato lo dichiari per
 *                            intero, ed e' la stessa disciplina che ha reso
 *                            trovabile l'errore sulla `.exit`.
 *   lavoro-ilitek-f-fix/     la directory di lavoro di QUESTA correzione: gli
 *                            script `posizioni.py`, `istogramma.py`,
 *                            `ancoraggio.py`, `pip.sh`; i disassemblati
 *                            `fabbricaF.asm`, `nostroF.asm`, `gruppoF.asm`,
 *                            `oltre.asm`; le copie del sorgente prima e dopo
 *                            (`base.c`, `dopo.c`) e i due `.text` estratti
 *                            (`prima.bin`, `dopo.bin`) che provano le sette
 *                            parole cambiate da `#line`.
 *   lavoro-ilitek-f5/        la directory di lavoro del lotto F5: gli script
 *                            `fai.sh` (compila con i due compilatori e misura),
 *                            `cmp.sh` (rifa' il disassemblato del nostro
 *                            oggetto e lo confronta con quello di fabbrica),
 *                            `link.sh` (compila tutti e otto gli oggetti in
 *                            albero-ilitek-b e conta gli irrisolti), le copie
 *                            di `ancoraggio.py`, `dif2.py`, `cfr.py`,
 *                            `cita.py` prese da `lavoro-ilitek-f4/`; i
 *                            disassemblati `f5.asm` (la sola
 *                            `ilitek_tddi_mp_test_main` di fabbrica),
 *                            `gruppoF.asm`, `oltre.asm`, `oltre2.asm`,
 *                            `nostro.asm`, `nostro_rel.asm`; `str.txt` e
 *                            `str2.txt` (le stringhe lette dai byte),
 *                            `irr_dopo.txt` e la copia di lavoro
 *                            `ilitek_mp.c`.
 *   lavoro-ilitek-f6/        la directory di lavoro della REVISIONE del
 *                            lotto F5 (2026-08-22): `build.sh` (compila con
 *                            l'uno o l'altro compilatore), `link.sh` (conta
 *                            gli irrisolti per differenza fra insiemi),
 *                            `irrisolti.sh`, gli script di misura `isto.py`
 *                            (istogramma dei mnemonici), `chiamate.py`
 *                            (bersagli di chiamata con gli alias deboli),
 *                            `linee.py` (insiemi dei `__LINE__`), `msg2.py`
 *                            (inventario dei messaggi per formato),
 *                            `allinea.py`, `formati.py`, `printk.py`; la
 *                            copia di `ancoraggio.py` presa da
 *                            `lavoro-ilitek-f4/` (md5 identico); i
 *                            disassemblati `fabbrica.asm`, `nostro_f9.asm`,
 *                            `gruppoF.asm`, `oltre.asm`, RIGENERATI da questa
 *                            revisione e non ereditati.
 *   lavoro-ilitek-f2/        la directory di lavoro del lotto F2: gli script
 *                            `fai.sh` (rifa' i due disassemblati),
 *                            `posizioni2.py`, `residuo.py`, e la copia di
 *                            `ancoraggio.py` presa da `lavoro-ilitek-f-fix/`;
 *                            i disassemblati `fabbricaF2.asm`, `nostroF2.asm`,
 *                            `gruppoF.asm`, `oltre2.asm`; e la copia di
 *                            lavoro `ilitek_mp.c` su cui girano i due
 *                            strumenti di verifica.
 *
 * `albero-ilitek-f/` (l'albero sorgente) esisteva gia' ed e' l'unico toccato:
 * il file compilato e' `drivers/input/touchscreen/mediatek/ilitek_e977/
 * ilitek_mp.c` dentro di esso, che e' una copia byte per byte di questo.
 * Nessun file condiviso e' stato toccato: `ilitek.h` e i tre strumenti di
 * verifica sono quelli di prima. NON e' stato creato nessun albero nuovo e
 * non e' stata toccata nessuna directory di altri agenti.
 *
 * ===========================================================================
 * LA MISURA, DAL `.o` VERO -- AGGIORNATA ALLA REVISIONE DEL LOTTO F5
 * ===========================================================================
 * `nm --print-size` sull'oggetto vero, con i DUE compilatori, stesso
 * sorgente. F9 = clang-r353983c (LLVM 9.0.3), quello di FABBRICA;
 * L11 = clang-r383902 (LLVM 11.0.1), quello dell'albero.
 *
 *   funzione                    fabbrica       F9  scarto       L11  scarto
 *   check_int_level                  240      240       0       240       0
 *   compare_MaxMin_result            360      360       0       364      +4
 *   compare_charge                   192      192       0       192       0
 *   dump_benchmark_data              300      300       0       312     +12
 *   dump_node_type_buffer            180      180       0       188      +8
 *   full_open_rate_compare            80       80       0        80       0
 *   ilitek_tddi_mp_test_main       14704    14836    +132     14912    +208
 *   mp_comp_result_before_retry     3320     3320       0      3404     +84
 *   mp_compare_cdc_show_result      1068     1068       0      1276    +208
 *   mp_do_retry                      156      156       0       156       0
 *   parser_get_ini_key_value         332      332       0       344     +12
 *   parser_get_u8_array              292      292       0       300      +8
 *   parser_ini_benchmark             860      860       0       864      +4
 *   parser_ini_nodetype              500      500       0       508      +8
 *   self_test                         44       44       0        44       0
 *   st_test                           44       44       0        44       0
 *   somma                          22672    22804    +132     23228    +556
 *   identiche                                 15/16              6/16
 *
 * QUINDICI su SEDICI con il compilatore di fabbrica. AVVERTENZA PRIMA DELLA
 * PERCENTUALE: su sedici unita' la misura NON discrimina in nessuna
 * direzione. `intervallo.py 15 16` dice
 *
 *   15 su 16 = 93.8%   IC95% Clopper-Pearson [69.8%; 99.8%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *
 * ed e' un PASSO INDIETRO rispetto al lotto F4, che con 15 su 15 aveva
 * l'estremo inferiore a 78,2% e quindi discriminava: la sedicesima funzione
 * non e' esatta e l'intervallo si riapre. Va detto, non nascosto.
 *
 * Con il compilatore dell'albero, `intervallo.py 6 16`:
 *
 *   6 su 16 = 37.5%   IC95% Clopper-Pearson [15.2%; 64.6%]
 *   contiene 77.10%: NO -- la misura discrimina
 *
 * cioe' misurato con LLVM 11 questo file sembrerebbe PEGGIORE del ramo, e
 * sarebbe una conclusione sbagliata tratta dal compilatore sbagliato.
 *
 * E PER LA SOLA FUNZIONE DEL LOTTO F5, `intervallo.py 0 1`:
 *
 *   0 su 1 = 0.0%   IC95% Clopper-Pearson [0.0%; 97.5%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *
 * Lo scarto e' +132 byte su 14704, cioe' +0,90%: TRENTATRE istruzioni in piu'
 * su 3676 (fabbrica 3676, nostre 3709). Non e' esatta, e questo file non la
 * spaccia per esatta: la sezione "COSA RESTA DI DIVERSO" qui sotto dice
 * quanto se ne sa e quanto no.
 *
 * IL NUMERO E' PEGGIORATO CON LA REVISIONE, E VA DETTO PRIMA DI TUTTO IL
 * RESTO. Il lotto F5 consegnava -48 byte (14656). La revisione ha trovato TRE
 * differenze di sorgente vere e le ha corrette; una delle tre allunga la
 * funzione di 156 byte e un'altra la accorcia di 24:
 *
 *   consegna del lotto F5                              14656    -48
 *   + `continue` invece di `return` nei due cicli
 *     di `mp_print_csv_cdc_cmd`                         14860   +156
 *   + messaggio "MAX_KEY_NAME_LEN" dove F5 aveva
 *     scritto "MAX_KEY_VALUE_LEN" (due siti su tre)     14860   +156
 *   + `memset` del GLOBALE `csv_path` invece del
 *     vettore di pila `str`                             14836   +132
 *
 * (il secondo passo non cambia un byte di `.text`: cambia quale stringa di
 * `.rodata` viene puntata). Una correzione giusta nel merito che PEGGIORA la
 * misura si tiene lo stesso: e' il caso che la regola 3 prevede -- «non
 * aggiustare mai qualcosa perche' un numero torni».
 *
 * ===========================================================================
 * COSA RESTA DI DIVERSO -- LE TRENTATRE ISTRUZIONI, E QUANTO SE NE SA
 * ===========================================================================
 * QUATTRO MISURE DI FORMA COMBACIANO, e prima della revisione due di esse no.
 *
 * 1. I NUMERI DI RIGA EMESSI. Sessantotto valori distinti di `__LINE__` con
 *    il filtro naturale (valori >= 500), INSIEMI UGUALI; e sull'insieme
 *    INTERO dei `mov w2, #imm` (72 valori, insiemi uguali) c'e' UN SOLO
 *    conteggio diverso, il 100 -- 4 di fabbrica contro 3 nostri -- che non e'
 *    un numero di riga ma la lunghezza delle due `strncpy` che clang fonde
 *    (vedi in fondo). Prima della revisione erano diversi anche 1124 (4
 *    contro 1) e 1133 (2 contro 1): sono i due cicli di
 *    `mp_print_csv_cdc_cmd`, e sono tornati a combaciare con `continue`.
 *
 * 2. I BERSAGLI DI CHIAMATA. Ventinove bersagli distinti nella fabbrica,
 *    ventinove nel nostro, INSIEMI IDENTICI, con la mappatura degli alias
 *    deboli (__memset->memset, __memcpy->memcpy, __pi_strlen->strlen,
 *    __pi_strncmp->strncmp). Ventisette conteggi su ventinove identici, fra
 *    cui kfree 23=23, vfree 6=6, memset 35=35, snprintf 75=75, katoi 23=23,
 *    parser_get_ini_key_value 34=34, mp_compare_cdc_show_result 14=14,
 *    vfs_read 1=1, vfs_write 1=1, __stack_chk_fail 1=1.
 *
 * 3. I SITI LOGICI DI MESSAGGIO. I `bl printk` FISICI sono 101 contro 99, e
 *    NON e' codice in piu': la fabbrica FONDE le code di piu' rami d'errore
 *    in una sola `bl` (tre rami di "MAX_KEY_*: Out Of Length" finiscono tutti
 *    sulla `bl` a 0xffffff8008a60b78), e il nostro oggetto ne fonde meno. Il
 *    conto che tiene conto della fusione torna esatto:
 *
 *      fabbrica: 99 `bl printk`, di cui QUATTRO sono bersaglio di SEI salti
 *                diretti (0xffffff8008a5efd8 x1, 0xffffff8008a60658 x1,
 *                0xffffff8008a60b78 x2, 0xffffff8008a614a4 x2)  -> 99+6 = 105
 *      nostro  : 101 `bl printk`, di cui QUATTRO sono bersaglio di QUATTRO
 *                salti diretti                                  -> 101+4 = 105
 *
 *    e i `mov w2, #imm` totali sono 110 di fabbrica contro 109 nostri: la
 *    differenza e' l'unico 100 del punto 1.
 *
 * 4. GLI ACCESSI A MEMORIA VERA (non pila). Store 140 di fabbrica contro 136
 *    nostri; load 277 contro 274. Prima della correzione del `memset` di
 *    `csv_path` il nostro oggetto scriveva 128 byte sulla PILA invece che sul
 *    globale, e questa riga era molto peggiore.
 *
 * COSA NON COMBACIA, E CHE QUESTO FILE NON SA LOCALIZZARE. Il resto e' quasi
 * tutto TRAFFICO DI PILA, cioe' spill:
 *
 *      store verso la pila:  fabbrica 139   nostro 172   (+33)
 *      load  dalla pila:     fabbrica  89   nostro 109   (+20)
 *
 * e l'istogramma per mnemonico lo conferma: str +27, add -13, ldr +12,
 * adrp -10, ldp +8, b.lt +7, b.eq -5, b.ne +5. E' una differenza di
 * ALLOCAZIONE DEI REGISTRI, non di codice scritto -- nessuna chiamata in piu'
 * o in meno, nessun messaggio in piu' o in meno, nessun numero di riga in
 * piu' o in meno. Ma resta un residuo NON LOCALIZZATO, e va dichiarato per
 * quello che e': quattro store e tre load verso oggetti veri che la fabbrica
 * fa e noi no. Il lotto F5 scriveva che la differenza «non e' una differenza
 * di sorgente che questo lotto sappia nominare»; tre lo erano, e sono
 * corrette. Questa parte, per ora, davvero non lo e'.
 *
 * DUE DIFFERENZE MINORI, NOMINABILI, CHE F5 NON DICHIARAVA:
 *
 *   - `strncpy` 7 di fabbrica contro 6 nostre. Le due `strncpy` verso `c100`
 *     dentro `parser_get_ini_phy_data` hanno la STESSA destinazione e la
 *     STESSA lunghezza (100) e sorgenti diverse ("benchmark_data" e
 *     "node type"): clang le fonde in una sola, la fabbrica ne tiene due,
 *     "94102029 bl"@0xffffff8008a60448 e "94102014 bl"@0xffffff8008a6049c.
 *     E' la stessa fusione di code del punto 3, vista dall'altra parte.
 *   - i due letterali KERN_CONT "KEY_%02d " e " %3d   ": di fabbrica
 *     l'indirizzo e' RIMATERIALIZZATO dentro il ciclo,
 *     "90003f20 adrp"@0xffffff8008a6111c + "9114d800 add"@0xffffff8008a61120,
 *     mentre il nostro oggetto lo issa fuori dal ciclo in un registro
 *     salvato. Due istruzioni per giro in meno da noi: e' parte del -10 sugli
 *     `adrp`.
 *
 * ===========================================================================
 * IL LINK -- IL CONTO CHE PEGGIORA, E PERCHE' E' GIUSTO CHE PEGGIORI
 * ===========================================================================
 * Il conto e' la DIFFERENZA FRA INSIEMI sui simboli di `built-in.o`, che e'
 * un ARCHIVIO (lo stesso simbolo vi compare sia `T` sia `U`); un `nm -u`
 * letto a occhio da' numeri falsi.
 *
 *   prima del lotto F5:  definiti 208   irrisolti 98   ILITEK irrisolti: 1
 *   dopo  il lotto F5:   definiti 215   irrisolti 106  ILITEK irrisolti: 5
 *   dopo la revisione:   definiti 215   irrisolti 106  ILITEK irrisolti: 5
 *
 * La differenza fra i due insiemi degli irrisolti e' esattamente:
 *   tolto:    ilitek_tddi_mp_test_main
 *   aggiunti: _ctype, getnstimeofday64, key_test, mutual_test,
 *             open_test_cap, open_test_sp, pin_test, rtc_time64_to_tm,
 *             strncpy
 * cioe' 98 - 1 + 9 = 106. Quattro dei nove sono simboli del KERNEL (`_ctype`,
 * `getnstimeofday64`, `rtc_time64_to_tm`, `strncpy`) e cinque sono le
 * funzioni di collaudo del sotto-lotto F3, che nessuno ha ancora scritto e
 * che questo file DICHIARA E NON DEFINISCE come vuole la regola 6.
 *
 * SONO CINQUE, NON SEI: il rapporto del lotto F5 scriveva «cinque funzioni di
 * collaudo piu' `str2hex` dichiarate e non definite; il link fallisce su quei
 * nomi». `str2hex` E' definita, in `ilitek_node.o`, e NON e' fra gli
 * irrisolti; la `extern` che questo file ne dichiara e' corretta e innocua.
 * Corretto dalla revisione.
 *
 * IL COMANDO E LA SUA USCITA LETTERALE, rifatti il 2026-08-22 su tutti e otto
 * gli oggetti ricompilati con il clang DI FABBRICA (script
 * `lavoro-ilitek-f6/link.sh`):
 *
 *   $ ./lavoro-ilitek-f6/build.sh albero-ilitek-b out-ilitek-b \
 *         /srv/archive/kernel-stock/clang-r353983c/bin \
 *         drivers/input/touchscreen/mediatek/ilitek_e977/
 *     CC      drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_main.o
 *     CC      drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_bus.o
 *     CC      drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_plat.o
 *     CC      drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_ic.o
 *     CC      drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_touch.o
 *     CC      drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_fw.o
 *     CC      drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_node.o
 *     CC      drivers/input/touchscreen/mediatek/ilitek_e977/ilitek_mp.o
 *
 *   $ ./lavoro-ilitek-f6/link.sh
 *   definiti (ogni classe tranne U, quindi anche i W):  215
 *   citati (U):                                         174
 *   IRRISOLTI = citati meno definiti:                   106
 *   di cui presenti in oracolo/stock.map:               99
 *   di cui assenti da stock.map (sono dati del kernel):  7
 *   IRRISOLTI CHE SONO ILITEK, da scrivere:
 *      key_test
 *      mutual_test
 *      open_test_cap
 *      open_test_sp
 *      pin_test
 *
 * Il conto chiude: 106 = 5 (ILITEK, da scrivere) + 94 (testo del kernel, che
 * stanno in `stock.map`: 99 meno i cinque ILITEK, che nella mappa ci sono
 * perche' la fabbrica li ha) + 7 (dati del kernel: `__stack_chk_guard`,
 * `_ctype`, `init_net`, `kmalloc_caches`, `tpd`, `tpd_dts_data`,
 * `tpd_load_status`, che in `stock.map` non compaiono perche' quella mappa
 * non ha simboli di dato).
 *
 * QUINDI: ILITEK NON SI LINKA, e la revisione lo conferma. Mancano le cinque
 * funzioni di collaudo del sotto-lotto F3.
 *
 * I sette definiti in piu' sono `ilitek_tddi_mp_test_main` e i sei oggetti di
 * dato che il lotto F5 apre: `ilitek_mp_ddi_mode`, `mp_run`, `seq_item`,
 * `csv_path`, `csv_time`, `mp_frame_buf`.
 *
 * Con il compilatore dell'albero (clang-r383902, LLVM 11) le sei funzioni di
 * F1 misuravano 312, 188, 344, 508, 864, 300 -- zero su sei. Non era il
 * sorgente: era il compilatore (vedi la sezione qui sopra). Le otto funzioni
 * di F2 sono state scritte fin dall'inizio con il compilatore di fabbrica e
 * non sono mai state misurate con LLVM 11.
 *
 * ===========================================================================
 * LE POSIZIONI CHE DIFFERISCONO -- IL COMANDO, E LA SUA USCITA LETTERALE
 * ===========================================================================
 * CORREZIONE del 2026-08-21. La prima stesura di questo paragrafo dichiarava
 * "236 posizioni divergenti su 616", scomposte in 99 + 7 + 82 + 12 + 36, e
 * concludeva "Nessun residuo inspiegato". Era l'UNICO paragrafo del lotto
 * senza un comando e senza un'uscita letterale, ed era anche l'unico i cui
 * numeri non si riproducono: rifatto il confronto, le posizioni sono 280 e
 * gli immediati di rilocazione 147, non 236 e 99. Regola 1 e regola 2 hanno
 * indicato lo stesso punto. Qui sotto c'e' il comando.
 *
 * I due disassemblati (`fabbricaF.asm` = fabbrica, sei intervalli nell'ordine
 * in cui l'oggetto emette le funzioni; `nostroF.asm` = il nostro `.o` con le
 * rilocazioni):
 *
 *   OBJ=/mnt/s88pro/lineage21/prebuilts/gcc/linux-x86/aarch64/\
 *       aarch64-linux-android-4.9/bin/aarch64-linux-android-objdump
 *   O=/mnt/s88pro/kernel-stock/out-ilitek-f9/drivers/input/touchscreen/\
 *       mediatek/ilitek_e977/ilitek_mp.o
 *   > fabbricaF.asm
 *   for r in 0xffffff8008a6573c:0xffffff8008a65868 \
 *            0xffffff8008a5da3c:0xffffff8008a5daf0 \
 *            0xffffff8008a61f88:0xffffff8008a620d4 \
 *            0xffffff8008a5daf0:0xffffff8008a5dce4 \
 *            0xffffff8008a5dce4:0xffffff8008a5e040 \
 *            0xffffff8008a659d0:0xffffff8008a65af4 \
 *            0xffffff8008a5e040:0xffffff8008a5e100 \
 *            0xffffff8008a5e100:0xffffff8008a5e150 \
 *            0xffffff8008a5e528:0xffffff8008a5e618 \
 *            0xffffff8008a65868:0xffffff8008a659d0 \
 *            0xffffff8008a64e94:0xffffff8008a64ec0 \
 *            0xffffff8008a64ec0:0xffffff8008a64eec \
 *            0xffffff8008a66c84:0xffffff8008a66d20 \
 *            0xffffff8008a66d20:0xffffff8008a6714c; do
 *     $OBJ -d --start-address=${r%%:*} --stop-address=${r##*:} \
 *          /mnt/s88pro/kernel-stock/oracolo/stock.elf >> fabbricaF.asm
 *   done
 *   $OBJ -d -r $O > nostroF.asm
 *
 * I TOTALI, con una pipeline che non usa nessuno script del progetto. La
 * `gawk` forza il confronto a STRINGA (`($1"")!=($2"")`): senza quelle due
 * virgolette vuote `121e7508` e `121e7529` sono due notazioni scientifiche,
 * valgono tutte e due `inf` e risultano UGUALI -- la pipeline dava 279 invece
 * di 280, e la posizione persa era "121e7508 and"@0xffffff8008a5dedc.
 *
 *   paste <(grep -oP '^\s*[0-9a-f]+:\t\K[0-9a-f]{8}' fabbricaF.asm) \
 *         <(gawk 'match($0,/^ *[0-9a-f]+:\t([0-9a-f]{8}) \t/,m){
 *                   if(k)print e,r; e=m[1];r=0;k=1;next}
 *                 /R_AARCH64/{r=1} END{if(k)print e,r}' nostroF.asm) \
 *   | gawk '{n++; if(($1"")!=($2"")){d++; if($3)ril++; else nor++}}
 *           END{print "istruzioni confrontate:", n
 *               print "posizioni con codifica diversa:", d
 *               print "  con rilocazione nel nostro .o:", ril
 *               print "  SENZA rilocazione:", nor}'
 *
 *   istruzioni confrontate: 1162
 *   posizioni con codifica diversa: 491
 *     con rilocazione nel nostro .o: 326
 *     SENZA rilocazione: 165
 *
 * La ripartizione per funzione, con `posizioni2.py` (37 righe, in
 * `/mnt/s88pro/kernel-stock/lavoro-ilitek-f2/`, stessi due file in ingresso;
 * e' `posizioni.py` di F1 con in piu' la colonna dei mnemonici e senza quella
 * `__LINE__`, che da quando c'e' `#line` e' sempre a zero). Uscita LETTERALE:
 *
 *   python3 posizioni2.py fabbricaF2.asm nostroF2.asm \
 *       dump_benchmark_data dump_node_type_buffer parser_get_ini_key_value \
 *       parser_ini_nodetype parser_ini_benchmark parser_get_u8_array \
 *       compare_charge full_open_rate_compare check_int_level \
 *       compare_MaxMin_result self_test st_test mp_do_retry \
 *       mp_compare_cdc_show_result
 *
 *   funzione                      istr  diverse     ril.  residuo mnem.div
 *   dump_benchmark_data             75       31       31        0        0
 *   dump_node_type_buffer           45       18       18        0        0
 *   parser_get_ini_key_value        83       43       20       23       10
 *   parser_ini_nodetype            125       37       25       12        7
 *   parser_ini_benchmark           215      113       34       79       51
 *   parser_get_u8_array             73       31       19       12       11
 *   compare_charge                  48        8        8        0        0
 *   full_open_rate_compare          20        2        2        0        0
 *   check_int_level                 60       26       26        0        0
 *   compare_MaxMin_result           90       20       20        0        0
 *   self_test                       11        5        5        0        0
 *   st_test                         11        5        5        0        0
 *   mp_do_retry                     39       13       13        0        0
 *   mp_compare_cdc_show_result     267      139      100       39        5
 *   TOTALE                        1162      491      326      165       84
 *
 * Le 326 posizioni con rilocazione (adrp, `add` di :lo12:, offset di `ldr` su
 * globali, destinazioni di `bl`) NON sono divergenze: nel nostro oggetto
 * l'immediato vale zero perche' il linker non e' ancora passato, e la
 * rilocazione accanto dice che cosa ci andra'. Restano 165 posizioni di
 * residuo vero, tutte attribuite:
 *
 *    79  `parser_ini_benchmark`, disposizione delle locali sulla pila (F1-D4)
 *    23  `parser_get_ini_key_value`, registri di scratch e ordine (F1-D7)
 *    12  `parser_ini_nodetype`, ordine (F1-D7)
 *    12  `parser_get_u8_array`, ordine dei blocchi (F1-D3)
 *    33  `mp_compare_cdc_show_result`, RINUMERAZIONE DEI REGISTRI (F2-D2)
 *     6  `mp_compare_cdc_show_result`, ordine dei due operandi di un `&&`
 *        (F2-D1)
 *
 * 79 + 23 + 12 + 12 + 33 + 6 = 165, e 326 + 165 = 491.
 *
 * SETTE DELLE OTTO FUNZIONI NUOVE HANNO RESIDUO ZERO: ogni posizione in cui
 * la codifica differisce e' un sito di rilocazione. L'unica con residuo e'
 * `mp_compare_cdc_show_result`, e le sue 39 posizioni si scompongono in due
 * cause sole, elencate una per una da `residuo.py` (stessa directory):
 *
 *   python3 residuo.py fabbricaF2.asm nostroF2.asm mp_compare_cdc_show_result
 *
 * Delle 39 righe che stampa, 33 hanno lo STESSO mnemonico e gli stessi
 * operandi a meno del NUMERO del registro, e i numeri seguono una sola
 * permutazione dei cinque registri salvati: 25->28, 28->26, 26->25 (un ciclo
 * di tre) e 27->19, 19->27 (uno scambio). Per esempio la riga 26 e'
 * `mov w26, wzr` di fabbrica contro `mov w25, wzr` nostro, e la riga 86 e'
 * `mov w25, wzr` contro `mov w28, wzr`. Le altre 6 (righe 135..140) sono lo
 * scambio dei due blocchi di prova descritto in F2-D1.
 *
 * ISTOGRAMMI DEI MNEMONICI. Per le otto funzioni nuove la colonna `mnem.div`
 * della tabella qui sopra e' l'istogramma fatto meglio: conta le posizioni in
 * cui il MNEMONICO differisce, non solo il totale per mnemonico. Vale zero in
 * sette funzioni su otto, e vale 5 in `mp_compare_cdc_show_result` (le righe
 * 135..140, sei posizioni, di cui una -- la 139 -- ha per caso lo stesso
 * mnemonico `cmp` da tutte e due le parti).
 *
 * Per le sei funzioni di F1 gli istogrammi (`istogramma.py`, in
 * `lavoro-ilitek-f-fix/`) sono IDENTICI in quattro su sei; le due che
 * differiscono lo fanno a numero totale di istruzioni invariato. Uscita
 * LETTERALE:
 *
 *   for f in dump_benchmark_data dump_node_type_buffer \
 *            parser_get_ini_key_value parser_ini_nodetype \
 *            parser_ini_benchmark parser_get_u8_array; do
 *     echo "=== $f"; python3 istogramma.py fabbricaF.asm nostroF.asm $f; done
 *
 *   === dump_benchmark_data
 *   totale fabbrica=75 nostro=75
 *   === dump_node_type_buffer
 *   totale fabbrica=45 nostro=45
 *   === parser_get_ini_key_value
 *   totale fabbrica=83 nostro=83
 *   === parser_ini_nodetype
 *   totale fabbrica=125 nostro=125
 *   === parser_ini_benchmark
 *   add      fabbrica=31 nostro=30
 *   ldp      fabbrica=6 nostro=7
 *   ldr      fabbrica=18 nostro=13
 *   ldur     fabbrica=1 nostro=4
 *   sub      fabbrica=5 nostro=7
 *   totale fabbrica=215 nostro=215
 *   === parser_get_u8_array
 *   cbnz     fabbrica=0 nostro=1
 *   cbz      fabbrica=3 nostro=2
 *   totale fabbrica=73 nostro=73
 *
 * `dump_node_type_buffer` e `dump_benchmark_data` sono ora identiche alla
 * fabbrica istruzione per istruzione e operando per operando, con l'unica
 * eccezione degli immediati di rilocazione: zero nella colonna `residuo`.
 *
 * ===========================================================================
 * IL LOTTO F4 -- LA MISURA, LE DIVERGENZE, LE SCOPERTE
 * ===========================================================================
 * Il lotto F4 aggiunge UNA funzione emessa, `mp_comp_result_before_retry`, e
 * con essa i corpi di tre funzioni del sorgente di fabbrica che il
 * compilatore incorpora (`mp_compare_cdc_result`, `mp_test_data_sort_average`,
 * `ipio_kfree`).
 *
 * LA MISURA CON IL COMPILATORE DI FABBRICA (clang-r353983c, LLVM 9.0.3),
 * installato in /mnt/s88pro/kernel-stock/clang-r353983c. Uscita LETTERALE di
 * `nm --print-size` sull'oggetto, filtrata sulle sole funzioni:
 *
 *   0000000000000ab0 00000000000000f0 T check_int_level
 *   0000000000000ba0 0000000000000168 t compare_MaxMin_result
 *   00000000000009a0 00000000000000c0 T compare_charge
 *   0000000000000000 000000000000012c t dump_benchmark_data
 *   000000000000012c 00000000000000b4 T dump_node_type_buffer
 *   0000000000000a60 0000000000000050 T full_open_rate_compare
 *   0000000000000dfc 0000000000000cf8 t mp_comp_result_before_retry
 *   0000000000001af4 000000000000042c t mp_compare_cdc_show_result
 *   0000000000000d60 000000000000009c t mp_do_retry
 *   00000000000001e0 000000000000014c t parser_get_ini_key_value
 *   000000000000087c 0000000000000124 t parser_get_u8_array
 *   0000000000000520 000000000000035c T parser_ini_benchmark
 *   000000000000032c 00000000000001f4 T parser_ini_nodetype
 *   0000000000000d08 000000000000002c t self_test
 *   0000000000000d34 000000000000002c t st_test
 *
 * 0xcf8 = 3320, ESATTAMENTE la dimensione di fabbrica. Nessun simbolo per le
 * tre incorporate: sono sparite dentro il chiamante, come di fabbrica.
 *
 *   funzione                    fabbrica  nostro  scarto   istruzioni F/N
 *   ...le quattordici di F1/F2       4648    4648       0     1162 / 1162
 *   mp_comp_result_before_retry      3320    3320       0      830 /  830
 *   somma                            7968    7968       0     1992 / 1992
 *
 * QUINDICI su QUINDICI identiche. AVVERTENZA PRIMA DELLA PERCENTUALE: su
 * quindici unita' la misura comincia appena a discriminare, e per il solo
 * lotto F4 -- una funzione -- NON discrimina affatto.
 * `./venv/bin/python3 intervallo.py 15 15` e `intervallo.py 1 1`:
 *
 *   15 su 15 = 100.0%   IC95% Clopper-Pearson [78.2%; 100.0%]
 *   contiene 77.10%: NO -- la misura discrimina
 *
 *   1 su 1 = 100.0%   IC95% Clopper-Pearson [2.5%; 100.0%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *
 * Con quattordici funzioni l'estremo inferiore stava a 76,8%, SOTTO il 77,10%
 * del ramo; con la quindicesima sale a 78,2% e lo supera. E' la prima volta
 * che questo file, da solo, e' una prova che il metodo faccia meglio del ramo
 * -- e vale per il FILE, non per il lotto F4, che da solo non prova niente.
 *
 * LA MISURA CON IL COMPILATORE DELL'ALBERO (clang-r383902, LLVM 11.0.1), sullo
 * STESSO identico sorgente. Uscita LETTERALE:
 *
 *   0000000000000ae4 00000000000000f0 T check_int_level
 *   0000000000000bd4 000000000000016c t compare_MaxMin_result
 *   00000000000009d4 00000000000000c0 T compare_charge
 *   0000000000000000 0000000000000138 t dump_benchmark_data
 *   0000000000000138 00000000000000bc T dump_node_type_buffer
 *   0000000000000a94 0000000000000050 T full_open_rate_compare
 *   0000000000000e34 0000000000000d4c t mp_comp_result_before_retry
 *   0000000000001b80 00000000000004fc t mp_compare_cdc_show_result
 *   0000000000000d98 000000000000009c t mp_do_retry
 *   00000000000001f4 0000000000000158 t parser_get_ini_key_value
 *   00000000000008a8 000000000000012c t parser_get_u8_array
 *   0000000000000548 0000000000000360 T parser_ini_benchmark
 *   000000000000034c 00000000000001fc T parser_ini_nodetype
 *   0000000000000d40 000000000000002c t self_test
 *   0000000000000d6c 000000000000002c t st_test
 *
 * cioe' SEI su quindici identiche: `check_int_level` 240, `compare_charge`
 * 192, `full_open_rate_compare` 80, `mp_do_retry` 156, `self_test` 44,
 * `st_test` 44. Le altre nove sbagliano da 4 a 208 byte, e
 * `mp_comp_result_before_retry` misura 0xd4c = 3404 contro 3320: NON
 * identica, 84 byte di troppo. `intervallo.py 6 15`:
 *
 *   6 su 15 = 40.0%   IC95% Clopper-Pearson [16.3%; 67.7%]
 *   contiene 77.10%: NO -- la misura discrimina
 *
 * ed e' un intervallo che sta tutto SOTTO il 77,10%: misurato con il
 * compilatore sbagliato, questo file sembrerebbe PEGGIORE del ramo. E' la
 * ragione per cui il numero che conta e' quello con clang-r353983c.
 *
 * IL CONFRONTO POSIZIONE PER POSIZIONE, per la sola funzione nuova. I due
 * disassemblati e la pipeline (nessuno script del progetto, `gawk` che forza
 * il confronto a STRINGA):
 *
 *   OBJ=/mnt/s88pro/lineage21/prebuilts/gcc/linux-x86/aarch64/\
 *       aarch64-linux-android-4.9/bin/aarch64-linux-android-objdump
 *   O=/mnt/s88pro/kernel-stock/out-ilitek-f9/drivers/input/touchscreen/\
 *       mediatek/ilitek_e977/ilitek_mp.o
 *   $OBJ -d --start-address=0xffffff8008a65f8c --stop-address=0xffffff8008a66c84 \
 *        /mnt/s88pro/kernel-stock/oracolo/stock.elf > f4.asm
 *   $OBJ -d -r --start-address=0xdfc --stop-address=$((0xdfc+0xcf8)) $O > nostro_rel.asm
 *   paste <(grep -oP '^\s*[0-9a-f]+:\t\K[0-9a-f]{8}' f4.asm) \
 *         <(gawk 'match($0,/^ *[0-9a-f]+:\t([0-9a-f]{8}) \t/,m){
 *                   if(k)print e,r; e=m[1];r=0;k=1;next}
 *                 /R_AARCH64/{r=1} END{if(k)print e,r}' nostro_rel.asm) \
 *   | gawk '{n++; if(($1"")!=($2"")){d++; if($3)ril++; else nor++}}
 *           END{print "istruzioni confrontate:", n
 *               print "posizioni con codifica diversa:", d
 *               print "  con rilocazione nel nostro .o:", ril
 *               print "  SENZA rilocazione:", nor}'
 *
 *   istruzioni confrontate: 830
 *   posizioni con codifica diversa: 161
 *     con rilocazione nel nostro .o: 161
 *     SENZA rilocazione:
 *
 * RESIDUO ZERO: tutte e 161 le posizioni in cui la codifica differisce sono
 * siti di rilocazione, dove nel nostro oggetto l'immediato vale zero perche'
 * il linker non e' ancora passato. L'ultima riga e' vuota perche' `gawk` non
 * stampa uno zero mai incrementato: il conto delle SENZA rilocazione e'
 * 161 - 161 = 0. Anche i MNEMONICI combaciano tutti e 830 su 830, verificato
 * con un secondo confronto indipendente (`dif2.py` in
 * `/mnt/s88pro/kernel-stock/lavoro-ilitek-f4/`, che allinea le due sequenze
 * con `difflib` invece che per posizione).
 *
 * QUANTI SIMBOLI ILITEK RESTANO IRRISOLTI. Gli otto oggetti del gruppo,
 * compilati insieme in `albero-ilitek-b`, danno un `built-in.o` che e' un
 * ARCHIVIO: lo stesso simbolo vi compare sia `T` sia `U`, e un `nm -u` letto
 * a occhio da' numeri falsi. Il conto giusto e' la DIFFERENZA FRA INSIEMI:
 *
 *   $NM built-in.o > bi.txt
 *   awk '$2 ~ /^[TtDdBbRrWwVv]$/ {print $3}' bi.txt | sort -u > def.txt
 *   awk '$1 == "U" {print $2}'                 bi.txt | sort -u > undef.txt
 *   comm -23 undef.txt def.txt > irrisolti.txt
 *
 *   definiti: 208  irrisolti: 98
 *   ILITEK ancora irrisolti:
 *   ilitek_tddi_mp_test_main
 *
 * Prima del lotto F4 gli irrisolti erano NOVANTANOVE, non 98 come diceva la
 * consegna: il conto e' stato rifatto ricompilando la versione precedente di
 * questo file nello stesso albero, e la differenza fra i due insiemi e' UN
 * SOLO nome, `mp_comp_result_before_retry`. Nessun simbolo NUOVO e' comparso:
 * `__kmalloc`, `kfree` e `printk` erano gia' referenziati dagli altri oggetti.
 * Dei 98 che restano, UNO SOLO e' del driver ILITEK
 * (`ilitek_tddi_mp_test_main`, il lotto gemello); gli altri 97 sono simboli
 * del kernel e del core touch MediaTek (`tpd`, `tpd_dts_data`,
 * `tpd_driver_add`, ...) che si risolvono al link del kernel.
 *
 * ===========================================================================
 * LE DUE DIVERGENZE DEL LOTTO F4
 * ===========================================================================
 * F4-D1. `noinline` su `mp_comp_result_before_retry`. Di fabbrica la funzione
 * ha DUE chiamanti ("940017e0 bl"@0xffffff8008a6000c dentro
 * `ilitek_tddi_mp_test_main` e "97fffcab bl"@0xffffff8008a66ce0 dentro
 * `mp_do_retry`) e clang non la incorpora. Qui il primo chiamante non e'
 * scritto, il sito di chiamata e' uno solo e clang la incorpora dentro
 * `mp_do_retry`: misurato, `mp_do_retry` passa da 0x9c (156) a 0xd78 (3448) e
 * il simbolo `mp_comp_result_before_retry` sparisce dall'oggetto. `noinline`
 * e' l'artificio che tiene il simbolo, della stessa natura del `__used` gia'
 * usato in questo file, e VA TOLTO quando `ilitek_tddi_mp_test_main` sara'
 * scritta. Il codice emesso dentro la funzione non cambia.
 *
 * F4-D2. L'ordine dei due controlli su `core_mp.rx_delta_buf` e
 * `core_mp.tx_delta_buf` nel ramo `c16 == 4`. Il binario legge PRIMA `rx`
 * ("f9461d08 ldr"@0xffffff8008a66044) e POI `tx`
 * ("f9461908 ldr"@0xffffff8008a66058), ed e' quello che il sorgente scrive.
 * Ma i due sono carichi PURI da globali e clang puo' scambiarli senza
 * cambiare una virgola del codice emesso: l'ordine e' SUGGERITO dal binario,
 * non MISURATO da esso. Se il sorgente di fabbrica avesse l'ordine opposto,
 * nessuna misura di questo lotto se ne accorgerebbe.
 *
 * ===========================================================================
 * LE SCOPERTE DEL LOTTO F4
 * ===========================================================================
 * F4-S1. L'ORDINE DI DICHIARAZIONE DI DUE `int` VALE QUATTRO BYTE.
 * `mp_comp_result_before_retry` ha due variabili d'esito, quella che torna
 * (`ret`) e quella che finisce in `mp_items[index].c36` (`res`). Dichiarando
 * `ret` per primo, il blocco d'errore della riga 2764 esce nell'ordine
 * (`ret`, `res`) e clang riesce a fondere la coda del blocco della riga 2796
 * con la seconda delle due istruzioni: 3316 byte. Dichiarando `res` per
 * primo l'ordine si inverte, la fusione non e' piu' possibile e la funzione
 * misura 3320 ESATTI. Provato anche lo scambio delle ISTRUZIONI nel sorgente
 * (`res = -1; ret = -107;` invece di `ret = -107; res = -1;`, in tutti e due
 * i rami): NON cambia niente, 3316 in entrambi i casi. E' l'ordine delle
 * DICHIARAZIONI, non quello delle assegnazioni. E' una classe di difetto
 * nuova, parente di A2 e di F2-S1: la posizione di una RIGA del sorgente
 * misurabile dalla dimensione.
 *
 * F4-S2. UN PUNTATORE PASSATO PER INDIRIZZO VALE SEDICI BYTE. Le due `kfree`
 * finali sono guardate da un `if` sul puntatore. Scritte come
 * `if (thr_max) kfree(thr_max);` su una variabile locale, clang propaga la
 * non-nullita' gia' stabilita dal controllo dell'allocazione, le tre `cbnz`
 * delle uscite normali ("b5ffa7d3 cbnz"@0xffffff8008a66c6c,
 * "b5ffaa73 cbnz"@0xffffff8008a66c18, "b5ffa753 cbnz"@0xffffff8008a66c7c)
 * spariscono e i tre blocchi di uscita si fondono: 3304 byte, dodici
 * istruzioni in meno. Passando l'INDIRIZZO della variabile a una funzione che
 * rilegge `*mem`, la propagazione non avviene e le tre `cbnz` tornano con i
 * loro blocchi: 3320. E' la stessa classe -- il binario dice se un valore
 * passa per un registro o per la memoria.
 *
 * F4-S3. CONTARE I BLOCCHI DI MESSAGGIO SOTTOSTIMA I SITI DI INCORPORAZIONE.
 * `mp_compare_cdc_result` ha UN solo `__func__` e CINQUE blocchi di `printk`
 * che lo usano, ma i siti in cui il suo corpo e' incorporato sono SETTE: tre
 * di essi condividono lo stesso blocco d'errore (0xffffff8008a66bf8) perche'
 * clang ne ha fuso le code. Il conto giusto si fa sui caricamenti distinti
 * del dato confrontato, elencati nel cappello della funzione. E' la classe
 * C5, "un confine assolto per nome".
 *
 * F4-S4. `kcalloc` SI DISTINGUE DA `kmalloc` SENZA GUARDARE LE BANDIERE. La
 * prova di traboccamento `n > SIZE_MAX / size` che `kmalloc_array` fa, con
 * `size` costante 4 e `n` un `int` esteso con segno, si riduce a una sola
 * istruzione: `tbnz w, #31` ("37f80848 tbnz"@0xffffff8008a65fe8,
 * "37f808a9 tbnz"@0xffffff8008a66638). `kmalloc` non la genera. Le bandiere
 * 0x14080c0 ("52901801 mov"@0xffffff8008a65fec +
 * "72a02801 movk"@0xffffff8008a65ff4) lo confermano per la seconda via:
 * GFP_KERNEL vale 0x14000c0 nell'albero (___GFP_IO 0x40 | ___GFP_FS 0x80 |
 * ___GFP_DIRECT_RECLAIM 0x400000 | ___GFP_KSWAPD_RECLAIM 0x1000000) e
 * ___GFP_ZERO vale 0x8000.
 *
 * ===========================================================================
 * TRE DIFETTI TROVATI NEL LAVORO GIA' FATTO, E CORRETTI QUI
 * ===========================================================================
 * 1. UNA CITAZIONE ANCORATA ALL'INDIRIZZO SBAGLIATO (classe B8). Il cappello
 *    del campo `result_buf` di `struct ilitek_mp_item` dava la codifica
 *    f9000260 (`str`) all'indirizzo 0xffffff8008a6530c -- scritto qui FUORI
 *    dalla forma verificabile, apposta, perche' altrimenti i due strumenti lo
 *    rileggerebbero come una citazione vera e tornerebbero a segnalarlo. A
 *    quell'indirizzo il binario ha invece la codifica b140041f (`cmn`): la
 *    `str` sta a 0xffffff8008a65310, quattro byte piu' avanti. `verificaistruzioni.py` la dava CONFERMATA (cerca la codifica
 *    OVUNQUE nel blocco, non all'indirizzo citato) ed e' `ancoraggio.py` a
 *    trovarla -- esattamente il difetto contro cui il cappello di F1
 *    avvertiva. Corretto l'indirizzo; `ancoraggio.py`, ALLORA (lotto F4,
 *    quando le citazioni erano 764), diceva 764 su 764. Il numero di OGGI
 *    e' 1398 su 1398 ed e' quello della sezione "IL TERZO CONTROLLO": questa
 *    riga e' storia e va letta come tale -- lasciare un'uscita vecchia senza
 *    dire che e' vecchia e' proprio il difetto C1 che la revisione ha
 *    trovato altrove in questo cappello.
 *
 * 2. IL BLOCCO "Esito atteso" DEL CAPPELLO ERA FERMO AL LOTTO F2 (classe C1,
 *    la correzione che non raggiunge l'artefatto derivato). Dichiarava
 *    "ESITO=0 in tutti e due i casi"; ricompilato oggi sulla versione
 *    PRECEDENTE a questo lotto, `verificacitazioni.py` esce con ESITO=1 gia'
 *    da prima di F4. E' aggiornato piu' sopra, con i numeri misurati.
 *
 * 3. IL NUMERO DEI SITI DI INCORPORAZIONE (scoperta F4-S3): la consegna del
 *    lotto diceva CINQUE, e sono SETTE. Non e' un errore di chi l'ha scritta
 *    ma del modo di contare -- per blocchi di messaggio invece che per siti.
 *
 * ===========================================================================
 * COSA IL LOTTO F4 HA CREATO SUL PC DI BUILD
 * ===========================================================================
 * Sotto `/mnt/s88pro/kernel-stock/`:
 *
 *   lavoro-ilitek-f4/    la directory di lavoro: `fai.sh` (ricompila e misura
 *                        con l'uno o l'altro compilatore), `cfr.py` (confronto
 *                        posizionale dei mnemonici), `dif2.py` (confronto con
 *                        `difflib`, che regge gli spostamenti di blocco),
 *                        `cita.py` (produce le citazioni nella forma
 *                        verificabile a partire dagli indirizzi), la copia di
 *                        `ancoraggio.py` presa da `lavoro-ilitek-f2/`, i
 *                        disassemblati `f4.asm` (fabbrica), `nostro.asm` e
 *                        `nostro_rel.asm` (nostri), `gruppoF.asm`,
 *                        `oltre.asm`, `oltre2.asm`, l'elenco `irr_dopo.txt`
 *                        degli irrisolti, e le copie di lavoro
 *                        `ilitek_mp.c` e `ilitek_mp_HEAD.c` su cui girano i
 *                        tre strumenti di verifica.
 *
 * Sono stati RIUSATI, non creati: `albero-ilitek-f` (dove e' stato compilato
 * il solo `ilitek_mp.c`), `out-ilitek-f9` e `out-ilitek-f` (le due uscite di
 * build, con il compilatore di fabbrica e con quello dell'albero),
 * `albero-ilitek-b` e `out-ilitek-b` (dove sono stati compilati tutti e otto
 * gli oggetti insieme, come la consegna chiedeva), e `clang-r353983c`.
 * NESSUN albero nuovo, nessuna directory di altri agenti toccata, nessun file
 * condiviso modificato: `ilitek.h` e i tre strumenti di verifica sono quelli
 * di prima.
 *
 * ===========================================================================
 * COSA IL LOTTO F4 NON HA SCRITTO
 * ===========================================================================
 * Delle 24 funzioni emesse del gruppo F ne restano NOVE, 30704 byte. La
 * ripartizione e' cambiata rispetto a quella del lotto F2 solo per una voce:
 * `mp_comp_result_before_retry` era fra le "bloccate da funzioni incorporate"
 * e ora e' scritta -- il che dice che quel blocco NON e' un ostacolo di
 * principio, ma solo lavoro. Restano:
 *
 *   - `ilitek_tddi_mp_test_main` (14704 byte), l'ULTIMO simbolo che manca al
 *     link del driver, e il secondo chiamante che permettera' di togliere il
 *     `noinline` (divergenza F4-D1);
 *   - `mutual_test`, `open_test_sp`, `open_test_cap`, `key_test`,
 *     `allnode_open_cdc_data`, `allnode_open_cdc_result` -- tutte con
 *     funzioni incorporate dentro, `parser_get_int_data` in testa;
 *   - `pin_test` (740) e `create_mp_test_frame_buffer` (1388), che non sono
 *     bloccate da niente: sono lavoro rimandato, non un ostacolo.
 *
 * NON e' stato scritto nessuno stub (regola 6): `ilitek_tddi_mp_test_main`
 * non e' dichiarata ne' definita in questo file, e il link del gruppo
 * fallisce su quel nome. E' l'esito onesto.
 *
 * ===========================================================================
 * IL CONFINE, E PERCHE' REGGE
 * ===========================================================================
 * Il confine del gruppo F e' quello della ricognizione, verificato su quattro
 * prove indipendenti (adiacenza nella mappa; i numeri di riga del sorgente,
 * che coprono 348..3416 di un unico file mentre i due vicini ripartono da
 * altre numerazioni; la corsa densa di `.rodata` [0x92430a3, 0x9245ad8] con
 * il byte successivo che apre la prima stringa del vicino; il grafo delle
 * chiamate su tutto il kernel, con una sola freccia entrante). La somma delle
 * 24 dimensioni fa 38672 = 0xa6714c - 0xa5da3c: nessun buco, nessun
 * riempimento. Non l'ho rifatto: l'ho usato.
 *
 * Il taglio F1 dentro il gruppo NON e' un confine del binario: e' una scelta
 * mia, e si vede -- le sei funzioni non sono contigue in memoria. Il criterio
 * e' il tema (leggono il file .ini) piu' il fatto che nessuna di loro contiene
 * funzioni incorporate. `parser_get_int_data` (riga 566), che la ricognizione
 * assegna a questo tema, e' incorporata in CINQUE funzioni e i suoi byte
 * stanno tutti in F2/F3/F5: qui non c'e'.
 *
 * IL TAGLIO DEL LOTTO F2 E' ANCORA MENO UN CONFINE DEL BINARIO, e va detto
 * chiaro: le otto funzioni aggiunte non sono contigue e non stanno tutte nello
 * stesso sotto-lotto della ricognizione (cinque da F2, due da F3, una da F4).
 * Il criterio e' la DETERMINABILITA': nessuna delle quattordici funzioni
 * scritte qui contiene una funzione INCORPORATA. Il controllo e' quello della
 * ricognizione -- il `__func__` che ogni `printk` porta con se' -- e per le
 * UNDICI che stampano corrisponde sempre al nome della funzione che la
 * contiene. Per le tre che non stampano (`compare_charge`,
 * `full_open_rate_compare`, `compare_MaxMin_result`) quel controllo e' MUTO,
 * e va detto; al suo posto ce n'e' uno piu' forte: le tre non contengono
 * NESSUNA `bl`, quindi non c'e' niente che possa esservi stato incorporato.
 * Non e' una comodita': una funzione incorporata va riscritta come funzione C
 * separata, altrimenti il compilatore non la rincorpora nello stesso modo, e
 * riscriverla richiede di determinarne il corpo.
 *
 * DELLE DIECI CHE RESTANO, OTTO SONO BLOCCATE DALLO STESSO PROBLEMA. Sette
 * contengono funzioni incorporate secondo la tabella della ricognizione
 * (`ilitek_tddi_mp_test_main`, `mutual_test`, `open_test_sp`,
 * `open_test_cap`, `key_test`, `mp_comp_result_before_retry`,
 * `allnode_open_cdc_data`), e l'ottava e' `allnode_open_cdc_result`, che la
 * ricognizione classifica come "nessuna printk" e che invece CONTIENE
 * `parser_get_int_data` incorporata -- solo, senza il suo messaggio, perche'
 * i due puntatori che riceve sono costanti e il compilatore ha eliminato il
 * controllo di nullita'. Si riconosce dalla sequenza:
 * "94000f00 bl"@0xffffff8008a5e388 (parser_get_ini_key_value),
 * "321b03e1 orr"@0xffffff8008a5e398 (w1 = 0x20) e
 * "94103088 bl"@0xffffff8008a5e3a0 (snprintf), senza nessuna prova sui due
 * valori di ritorno. E' la stessa forma dei siti descritti in "LA DOMANDA
 * APERTA", con una TERZA dimensione ancora (32, dopo 128 e 512).
 *
 * LE DUE CHE NON SONO BLOCCATE DA NIENTE sono `pin_test` (740 byte) e
 * `create_mp_test_frame_buffer` (1388): tutti i loro `__func__` sono i propri
 * (quattro per `pin_test`, nove per `create_mp_test_frame_buffer`, contati in
 * `scout-ilitek-F/printk.txt`). Non sono state scritte per mancanza di tempo,
 * non per un ostacolo, e sono il primo lavoro del lotto successivo. Dirlo e'
 * il punto: 14 scritte + 2 rimandate + 8 bloccate = 24.
 *
 * ===========================================================================
 * LA FUNZIONE `.exit`: IL GRUPPO F NON NE HA UNA -- IL DRIVER SI'
 * ===========================================================================
 * CORREZIONE del 2026-08-21. La prima stesura di questa sezione concludeva
 * che "ne' il gruppo F ne' il driver ilitek nel suo complesso" avessero una
 * `.exit`, e chiamava la seconda meta' "un fatto misurato". Quella meta' era
 * FALSA, e la scansione che la sosteneva era cieca per costruzione in due
 * punti: cercava le `bl` solo dentro 0xffffff8008a53000..a72000, mentre
 * `tpd_driver_remove` sta a 0xffffff8008a514a8, SOTTO quell'intervallo; e
 * delle quattro funzioni di de-registrazione che il perimetro nominava ne
 * guardava una sola -- `i2c_del_driver` -- dichiarandola per giunta "l'unica"
 * quando nella zona ce ne sono OTTO. La conclusione ristretta al gruppo F
 * regge; quella estesa al driver no. E' la classe C3: una prova costruita su
 * una finestra ed estesa a un dominio che la finestra non copriva.
 *
 * PERCHE' SERVE objdump. `oracolo/stock.map` finisce a `_einittext`
 * (0xffffff80093a8518) e non vede la `.exit.text`: `clusterdriver.py`,
 * `cluster_finale.py`, `classificasimboli.py`, `disassembla.py` e
 * `misurablocco.py` partono tutti dalla mappa e sono ciechi oltre
 * quell'indirizzo. La zona e' 0xffffff80093a8518..0xffffff800942e6d8, dove
 * l'estremo alto e' la fine esatta della sezione `.kernel`
 * (0xffffff8008080000 + 0x13ae6d8):
 *
 *   OBJ=/mnt/s88pro/lineage21/prebuilts/gcc/linux-x86/aarch64/\
 *       aarch64-linux-android-4.9/bin/aarch64-linux-android-objdump
 *   $OBJ -d --start-address=0xffffff80093a8518 \
 *        --stop-address=0xffffff800942e6d8 \
 *        /mnt/s88pro/kernel-stock/oracolo/stock.elf > oltre.asm
 *   wc -l oltre.asm
 *   for f in i2c_del_driver tpd_driver_remove platform_driver_unregister \
 *            class_destroy; do
 *     echo -n "$f: "; grep -c "bl.*<$f>" oltre.asm
 *   done
 *
 *   105103 oltre.asm
 *   i2c_del_driver: 8
 *   tpd_driver_remove: 3
 *   platform_driver_unregister: 118
 *   class_destroy: 26
 *
 * `tpd_driver_remove` e' quella che usa un driver touch MediaTek, ed e'
 * quella che smentiva il lotto. Tre siti:
 *
 *   grep -n "bl.*<tpd_driver_remove>" oltre.asm
 *
 *   4477:ffffff80093acaec:	97da926f 	bl	ffffff8008a514a8 <tpd_driver_remove>
 *   4490:ffffff80093acb20:	97da9262 	bl	ffffff8008a514a8 <tpd_driver_remove>
 *   4504:ffffff80093acb58:	97da9254 	bl	ffffff8008a514a8 <tpd_driver_remove>
 *
 * Il primo e' ilitek. La funzione che lo contiene e' `ilitek_plat_dev_exit`,
 * 0xffffff80093acac4, 52 byte (0xffffff80093acaf8 - 0xffffff80093acac4 =
 * 0x34), 13 istruzioni:
 *
 *   $OBJ -d --start-address=0xffffff80093acac4 \
 *        --stop-address=0xffffff80093acaf8 \
 *        /mnt/s88pro/kernel-stock/oracolo/stock.elf | tail -13
 *
 *   ffffff80093acac4:	a9bf7bfd 	stp	x29, x30, [sp,#-16]!
 *   ffffff80093acac8:	910003fd 	mov	x29, sp
 *   ffffff80093acacc:	f0fff480 	adrp	x0, ffffff800923f000 <_etext+0x3af000>
 *   ffffff80093acad0:	f0fff481 	adrp	x1, ffffff800923f000 <_etext+0x3af000>
 *   ffffff80093acad4:	911ee400 	add	x0, x0, #0x7b9
 *   ffffff80093acad8:	911fb421 	add	x1, x1, #0x7ed
 *   ffffff80093acadc:	52803822 	mov	w2, #0x1c1                 	// #449
 *   ffffff80093acae0:	97b61a7d 	bl	ffffff80081334d4 <printk>
 *   ffffff80093acae4:	f0002ec0 	adrp	x0, ffffff8009987000 <_einittext+0x5deae8>
 *   ffffff80093acae8:	910ce000 	add	x0, x0, #0x338
 *   ffffff80093acaec:	97da926f 	bl	ffffff8008a514a8 <tpd_driver_remove>
 *   ffffff80093acaf0:	a8c17bfd 	ldp	x29, x30, [sp],#16
 *   ffffff80093acaf4:	d65f03c0 	ret
 *
 * L'argomento e' 0xffffff8009987338, in `.kernel2` e non in `.bss` -- ecco
 * perche' la scansione che cercava `adrp` solo verso le due pagine di dati
 * del gruppo F non poteva vederlo. E' la STESSA `tpd_driver_t` che
 * `ilitek_plat_dev_init` (0xffffff80093833a8, questa si' nella mappa) passa a
 * `tpd_driver_add`:
 *
 *   $OBJ -d --start-address=0xffffff80093833c8 \
 *        --stop-address=0xffffff80093833d8 \
 *        /mnt/s88pro/kernel-stock/oracolo/stock.elf | tail -4
 *
 *   ffffff80093833c8:	97db35fa 	bl	ffffff8008a50bb0 <tpd_get_dts_info>
 *   ffffff80093833cc:	90003020 	adrp	x0, ffffff8009987000 <_einittext+0x5deae8>
 *   ffffff80093833d0:	910ce000 	add	x0, x0, #0x338
 *   ffffff80093833d4:	97db37ec 	bl	ffffff8008a51384 <tpd_driver_add>
 *
 * E le due stringhe che la funzione stampa:
 *
 *   ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf \
 *       0xffffff800923f7b9 0xffffff800923f7ed
 *
 *   0xffffff800923f7b9: b'\x016ILITEK: (%s, %d): ilitek driver has been removed\n'  (len=51, hex=0136494c4954454b3a202825732c202564293a20696c6974656b2064726976657220686173206265656e2072656d6f7665640a)
 *   0xffffff800923f7ed: b'ilitek_plat_dev_exit'  (len=20, hex=696c6974656b5f706c61745f6465765f65786974)
 *
 * Quella funzione NON va scritta qui: appartiene al GRUPPO C, ed e' gia' nel
 * repository -- `ilitek_plat.c`, `static void __exit ilitek_plat_dev_exit`
 * piu' `module_exit(ilitek_plat_dev_exit)`, commit ee7669c del 2026-08-21,
 * cioe' quarantotto minuti PRIMA che questo lotto chiudesse. Il fatto era
 * gia' misurato e commesso mentre questo file scriveva il contrario: e' la
 * classe C1, la correzione che non raggiunge l'artefatto derivato.
 *
 * ED E' L'UNICA. Quelle 13 istruzioni chiamano solo `printk` e
 * `tpd_driver_remove`: non c'e' nessun'altra funzione ilitek da scrivere
 * dietro di lei. In tutta la zona oltre `_einittext` non compare nemmeno un
 * simbolo ilitek --
 *
 *   grep -c ilitek oltre.asm
 *
 *   0
 *
 * -- e `ilitek_plat_dev_exit` non e' un'eccezione a questo conto: objdump la
 * etichetta `<_einittext+0x45ac>` proprio perche' `stock.map`, da cui viene
 * la symtab ricostruita, si ferma prima.
 *
 * QUELLO CHE RESTA VERO, ed e' l'unica conclusione che questo lotto puo'
 * trarre: IL GRUPPO F, DA SOLO, NON HA UNA `.exit`. Nessuna delle 1502
 * `b`/`bl` della zona entra nei suoi byte (il confronto e' fra stringhe
 * esadecimali di 16 cifre, che per indirizzi della stessa larghezza ordina
 * come i numeri; `strtonum` su 64 bit perderebbe precisione):
 *
 *   gawk 'match($0, /\t(bl?)\t([0-9a-f]{16}) /, m){
 *           n++
 *           if(m[2]>="ffffff8008a5da3c" && m[2]<"ffffff8008a6714c") k++}
 *         END{print "b/bl nella zona:", n, "  che entrano nel gruppo F:", k+0}
 *        ' oltre.asm
 *
 *   b/bl nella zona: 1502   che entrano nel gruppo F: 0
 *
 * e nessuna `adrp` della zona materializza le sue due pagine di dati:
 *
 *   for p in ffffff800a0fc000 ffffff800a0ff000; do
 *     echo -n "adrp a $p: "; grep -c "adrp.*$p" oltre.asm
 *   done
 *
 *   adrp a ffffff800a0fc000: 0
 *   adrp a ffffff800a0ff000: 0
 *
 * Era prevedibile -- il collaudo MP non registra niente -- ma vale per il
 * gruppo F, non per il driver.
 *
 * IL LOTTO F2 HA RIFATTO QUESTO CONTROLLO DA CAPO, con un disassemblato piu'
 * largo e una scansione diversa, perche' un secondo parere su una conclusione
 * negativa costa poco. Il disassemblato e' `oltre2.asm`
 * (0xffffff80093a8518..0xffffff80093c0000, 24257 righe, che comprende tutta la
 * `.exit.text`: la prima `platform_driver_unregister` sta a
 * 0xffffff80093a8ac0 e l'ultima zona di codice finisce ben prima
 * dell'estremo). La scansione, in Python, ha cercato DUE cose e non una:
 *
 *   - ogni `b`/`bl` con destinazione dentro [0xffffff8008a5da3c,
 *     0xffffff8008a6714c);
 *   - ogni coppia `adrp`+`add` che materializzi un indirizzo dentro la corsa
 *     di `.rodata` del gruppo F, [0xffffff80092430a3, 0xffffff8009245ad8] --
 *     che il primo controllo, fatto solo sulle pagine di `.bss`, non
 *     guardava.
 *
 *   chiamate dalla .exit dentro il gruppo F: 0
 *   citazioni del .rodata del gruppo F dalla .exit: 0
 *
 * Lo script sta in `/mnt/s88pro/kernel-stock/lavoro-ilitek-f2/` insieme al
 * resto. Conclusione confermata, per una via indipendente dalla prima.
 *
 * ===========================================================================
 * LE DIVERGENZE APERTE, NUMERATE
 * ===========================================================================
 * F1-D1 -- CHIUSA il 2026-08-21. I `__LINE__`. Il binario porta i numeri di
 *     riga del file di fabbrica: 348 ("52802b82 mov"@0xffffff8008a65778), 371
 *     ("52802e62 mov"@0xffffff8008a5da70), 395
 *     ("52803162 mov"@0xffffff8008a620bc), 424
 *     ("52803502 mov"@0xffffff8008a5dc88), 464
 *     ("52803a02 mov"@0xffffff8008a5dfe8), 532
 *     ("52804282 mov"@0xffffff8008a65a44), 549
 *     ("528044a2 mov"@0xffffff8008a65ac4).
 *
 *     La prima stesura non li riproduceva e dichiarava la cosa IMPOSSIBILE:
 *     "allineare i `__LINE__` e documentare la derivazione sono requisiti
 *     incompatibili in questo file". L'aritmetica portata a sostegno e'
 *     giusta (424 - 395 = 29 righe di fabbrica fra le due `printk`, e
 *     464 - 424 = 40 fra le due successive, contro le 39 e 51 che il codice C
 *     annotato occupa qui), ma la conclusione no: la direttiva `#line` del C
 *     standard rimappa `__LINE__` senza toccare il numero di righe fisiche,
 *     il kernel la accetta e il clang di fabbrica la compila con `-Werror`
 *     senza un avviso. Era una scelta non tentata, non un vincolo -- ed e'
 *     esattamente la categoria che questo progetto chiede di non dichiarare
 *     impossibile quando e' solo non provata.
 *
 *     Ora ci sono SETTE `#line`, una per sito, ciascuna scritta subito sopra
 *     la sua `ILI_*`: restano attaccate alla chiamata anche se il file viene
 *     modificato piu' in alto. Le sei dimensioni non cambiano (300, 180, 332,
 *     500, 860, 292 prima e dopo, sei su sei), ma le posizioni divergenti
 *     scendono da 280 a 273 e la colonna `__LINE__` della tabella qui sopra
 *     va da 7 a 0: `dump_node_type_buffer` e `dump_benchmark_data` diventano
 *     identiche alla fabbrica in TUTTE le posizioni che non portano una
 *     rilocazione. In piu' i messaggi che il driver stampa a runtime portano
 *     ora gli stessi numeri di riga del kernel di fabbrica, che e' l'uso vero
 *     di quei sette immediati.
 *
 *     LA PROVA CHE NON TOCCA NULLA D'ALTRO. Compilato il file PRIMA e DOPO
 *     con lo stesso clang di fabbrica, estratto il `.text` con
 *     `objcopy -O binary --only-section=.text` e confrontato parola per
 *     parola, cambiano SETTE parole di quattro byte e nessun'altra -- e i
 *     valori nuovi sono esattamente quelli di fabbrica (little-endian:
 *     822b8052 = 0x52802b82 = `mov w2,#0x15c` = 348, e cosi' via). Uscita
 *     LETTERALE:
 *
 *       byte totali: 2464 e 2464
 *       parole di 4 byte diverse: 7
 *         +0x003c  02448052 -> 822b8052
 *         +0x0160  a24a8052 -> 622e8052
 *         +0x0314  c2548052 -> 62318052
 *         +0x04c4  62618052 -> 02358052
 *         +0x0824  42718052 -> 023a8052
 *         +0x08f0  227b8052 -> 82428052
 *         +0x0970  227f8052 -> a2448052
 *
 *     Nella stessa passata e' stato rinominato `pToken` in `tok` (vedi la
 *     nota su `parser_get_u8_array`): quel cambio non compare qui, perche' un
 *     nome di locale non arriva al codice emesso.
 *
 *     IL COSTO, dichiarato: dopo la prima `#line` le diagnostiche del
 *     compilatore riportano la numerazione DI FABBRICA e non quella fisica
 *     del file. Chi legge un errore deve contare le righe, o togliere
 *     temporaneamente le sette direttive.
 *
 * F1-D2. Il `__used` su tre funzioni, e il `static` mancante su tre oggetti
 *     DATI. La prima stesura intitolava questa voce "`static` mancante su
 *     cinque oggetti" e poi ne elencava sei, tre dei quali il `static` ce
 *     l'hanno: ne' 3 ne' 6 fa cinque, ed e' la classe di numero che questo
 *     progetto ha imparato a non lasciar passare.
 *
 *     LE TRE FUNZIONI. `stock.map` marca `t` (locali) `dump_benchmark_data`,
 *     `parser_get_ini_key_value` e `parser_get_u8_array`, e qui sono
 *     `static`: su questo NON c'e' divergenza. Diverge il `__used`, che nel
 *     sorgente di fabbrica non c'e': li' quelle tre hanno chiamanti nello
 *     stesso file (in F2/F3/F5), qui no, e senza `__used` clang le cancella e
 *     le segnala con `-Wunused-function` (che con `-Werror` e' fatale).
 *     Verificato: con `__used` il corpo emesso e' identico byte per byte a
 *     quello senza.
 *
 *     I TRE OGGETTI DATI. `core_mp`, `ilitek_ini_num` e `ilitek_ini_data`
 *     sono dichiarati `extern` e NON definiti qui: sono gli unici tre a cui
 *     il `static` manca davvero. Nel binario stanno in `.bss` e nessuno fuori
 *     dal gruppo F li tocca (quindi di fabbrica sono quasi certamente
 *     `static` al file del collaudo MP). Definirli `static` qui sarebbe
 *     PEGGIO di una divergenza cosmetica: mai scritti in questo lotto, il
 *     compilatore li proverebbe costanti a zero e cancellerebbe tutti i
 *     cicli. Chi chiudera' F5 li definisca, e li renda `static`.
 *
 *     Il quarto `extern` del file, `katoi`, non e' un caso di `static`
 *     mancante: e' una funzione di un altro gruppo, dichiarata e lasciata
 *     indefinita come vuole la regola 6.
 *
 *       grep -c '^static __used ' ilitek_mp.c
 *       grep -c '^extern ' ilitek_mp.c
 *
 *       3
 *       4
 *
 * F1-D3. `parser_get_u8_array`, ordine dei blocchi. Stesse 73 istruzioni,
 *     stessi 292 byte, ma la fabbrica prova `key[0] == 0` subito dopo la
 *     `ldrb` ("34000188 cbz"@0xffffff8008a65a04) mentre clang, con questo
 *     sorgente, sposta quella prova DOPO il test sugli spazi, sfruttando che
 *     0 sta comunque sotto 0x20 e non e' nella maschera. Dodici posizioni su
 *     73 permutate, zero byte di differenza. Sono state provate tre forme
 *     sorgente (catena di `||`, `switch`, `goto` sul ramo d'errore): tutte e
 *     tre danno lo stesso risultato. La forma che riproduce l'ordine di
 *     fabbrica NON e' stata trovata. E' un residuo dichiarato, non spiegato.
 *
 * F1-D4. `parser_ini_benchmark`, disposizione delle variabili locali sulla
 *     pila. Stesse 215 istruzioni, stessi 860 byte. La fabbrica mette
 *     `tmp[4]` a sp+0x120 e il buffer da 512 a sp+0x130
 *     ("9104c3e0 add"@0xffffff8008a5dd14); qui il buffer sta a sp+0x120 e
 *     `tmp[4]` in cima. Provato a scambiare l'ordine di dichiarazione: PEGGIORA
 *     (864 byte, una `sub` in piu'). Lasciato com'e'. Costo: zero byte,
 *     cambia solo l'immediato di alcune `add`/`ldr` e la forma di
 *     indirizzamento. Sono le 79 posizioni della colonna `residuo` di
 *     `parser_ini_benchmark` nella tabella in cima, ed e' il residuo piu'
 *     grosso del lotto: 79 su 126.
 *
 * F1-D5. Il difetto di fabbrica sul formato di `parser_ini_benchmark`, riga
 *     464: `"count (%d) is larger than frame length, break\n"` ha un `%d` e
 *     la `printk` riceve TRE argomenti (formato, `__func__`, `__LINE__`) e
 *     basta -- nessun `w3`. E' un difetto vero della fabbrica e si riproduce
 *     (regola 7). Passa in compilazione senza un avviso perche' la riga di
 *     comando del kernel porta `-Wno-format` (verificato nel `.cmd`
 *     dell'oggetto): NON e' stato aggirato con un pragma, non c'e' stato
 *     bisogno. La funzione gemella `parser_ini_nodetype`, riga 424, passa
 *     invece il suo `%d` ("2a1603e3 mov"@0xffffff8008a5dc94, w3 = count):
 *     due messaggi quasi uguali, uno rotto e uno no.
 *
 * F1-D6. La base di `core_mp` e' una SCELTA, non una misura. Della struttura
 *     e' misurata la DISTANZA fra i due campi usati qui: 3072-3048 = 24 byte
 *     ("b94c02c8 ldr"@0xffffff8008a5da7c contro
 *     "b94beb08 ldr"@0xffffff8008a5daac). La base 0xffffff800a0fcb00 e' il
 *     piu' basso degli indirizzi che il gruppo F materializza con `adrp+add`
 *     dentro quell'area, secondo la ricognizione; se fosse sbagliata,
 *     cambierebbero i nomi `c232`/`c256` ma non una sola istruzione emessa.
 *
 * F1-D7. Ordine delle istruzioni e numerazione dei registri di scratch, in
 *     DUE funzioni: `parser_get_ini_key_value` (23 posizioni, scambio
 *     x25<->x26 e x23<->x24) e `parser_ini_nodetype` (12 posizioni). In tutte
 *     e due l'istogramma dei mnemonici e' IDENTICO a quello di fabbrica --
 *     stesse istruzioni, stesso numero, ordine o registro diversi -- ed e'
 *     rumore dello schedulatore e dell'allocatore, non una differenza di
 *     sorgente. Non tocca la dimensione. I due numeri sono le colonne
 *     `residuo` della tabella in cima; la prima stesura scriveva "10
 *     posizioni" per `parser_ini_nodetype` e attribuiva a questa voce anche
 *     `parser_ini_benchmark`, che invece sta tutta in F1-D4: 23 + 12 = 35, e
 *     con i 79 di F1-D4 e i 12 di F1-D3 fanno i 126 del totale di F1.
 *
 * ---------------------------------------------------------------------------
 * LE DIVERGENZE DEL LOTTO F2
 * ---------------------------------------------------------------------------
 * F2-D1. `mp_compare_cdc_show_result`: l'ordine dei due operandi di `&&` nel
 *     secondo e nel terzo ramo del confronto per nodo. SEI posizioni (le
 *     135..140 della sequenza). Il testo per esteso, con i due esiti misurati
 *     (1088 byte scrivendolo come la fabbrica lo emette, 1068 scrivendolo al
 *     contrario) sta nel cappello della funzione. NESSUNA DELLE DUE SCRITTURE
 *     riproduce la fabbrica esattamente: qui e' scelta quella che riproduce il
 *     CONTEGGIO. APERTA.
 *
 * F2-D2. `mp_compare_cdc_show_result`: rinumerazione di cinque registri
 *     salvati. TRENTATRE posizioni, tutte con lo stesso mnemonico e gli stessi
 *     operandi a meno del numero di registro, secondo una sola permutazione
 *     (25->28->26->25 e 27<->19). E' rumore dell'allocatore, non una
 *     differenza di sorgente, e non tocca la dimensione. E' la stessa classe
 *     di F1-D7. APERTA, e probabilmente non chiudibile.
 *
 * F2-D3. Il `__used` su NOVE funzioni. `compare_MaxMin_result`, `self_test`,
 *     `st_test`, `mp_do_retry` e `mp_compare_cdc_show_result` sono `t` nella
 *     mappa, cioe' `static` di fabbrica, e in questo lotto NESSUNO le chiama:
 *     senza `__used` clang le eliminerebbe e non ci sarebbe niente da
 *     misurare. `__used` NON e' nel binario di fabbrica -- non lascia traccia,
 *     essendo solo un attributo -- ma e' codice che il sorgente di fabbrica
 *     quasi certamente non aveva. E' la stessa scelta gia' dichiarata in
 *     F1-D2, e va tolta dal lotto che scrive i loro chiamanti. APERTA.
 *
 * F2-D4. La macro locale `ILI_CONT`. Il testo preprocessato e' identico a
 *     `printk(KERN_CONT ...)` e la misura lo conferma (zero residuo nelle
 *     funzioni che la usano, a parte le due cause gia' contate), ma nel
 *     sorgente di fabbrica quella macro quasi certamente non c'e': e' un
 *     artificio per rendere ancorabili otto letterali che
 *     `verificacitazioni.py` non sa trattare. Il perche' sta nel suo cappello.
 *     Va sostituita con `printk(KERN_CONT ...)` -- o meglio, va reso capace lo
 *     strumento -- nel lotto di merge. APERTA, ed e' una scelta, non una
 *     misura.
 *
 * F2-D5. I nomi. `mp_items` (la tavola a 0xffffff80099876d0),
 *     `mp_nb_dx`/`mp_nb_dy` (i due vettori di scostamento) e TUTTI i nomi di
 *     parametro delle funzioni senza `printk` sono SCELTI: il binario non li
 *     nomina. Il caso di `mp_items` e' il piu' delicato ed e' documentato sul
 *     posto, perche' la ricognizione lo chiama `tItems` come se fosse un
 *     fatto, e quel nome nel binario NON C'E'. APERTA per costruzione: nessun
 *     lotto potra' chiuderla, perche' l'informazione non e' nel binario.
 *
 * ===========================================================================
 * LA NOTA CHE VALE PIU' DI TUTTE: PERCHE' `parser_ini_nodetype` MISURAVA MALE
 * ===========================================================================
 * Prima stesura, con clang 9: 492 byte contro 500. Due istruzioni in meno.
 * La fabbrica legge il carattere `c200[j]` INCONDIZIONATAMENTE in cima al
 * ciclo interno ("3943218c ldrb"@0xffffff8008a5dbc0) e solo DOPO confronta
 * `j` con la lunghezza ("6b0b015f cmp"@0xffffff8008a5dbc4) e `c` con ';'
 * ("7100ed9f cmp"@0xffffff8008a5dbcc); sul ramo che salta il campo,
 * `pre = c200[j]` diventa allora un semplice "2a0c03e9 mov"@0xffffff8008a5dc68
 * (il valore e' gia' in registro), mentre sul ramo che lo converte, dopo le
 * chiamate, il valore va RILETTO ("3943210c ldrb"@0xffffff8008a5dc5c).
 * Le due letture della stessa espressione, una eliminata per CSE e una no,
 * sono la firma di un sorgente che legge `ini[i].c200[j]` due volte e che
 * mette il confronto sul CARATTERE per PRIMO nel `||`: scritto
 * `if (j == len || c200[j] == ';')` clang corto-circuita e la lettura sparisce
 * dal ramo `j == len`. Scritto `if (c == ';' || j == len)` con `c` letto in
 * cima, torna esatto. Otto byte, e dietro c'e' l'ordine dei due operandi di
 * un `||`.
 *
 * ===========================================================================
 * IL RESIDUO DICHIARATO -- LE OTTO FUNZIONI CHE NON SONO SCRITTE
 * ===========================================================================
 * QUESTA SEZIONE E' STORIA. AGGIORNAMENTO DEL LOTTO F6: IL RESIDUO E' ZERO.
 * Tutte e ventiquattro le funzioni emesse del gruppo F sono scritte, e con
 * `mutual_test` gli irrisolti ILITEK passano da UNO a ZERO. L'elenco che
 * segue e' quello del lotto F5 e resta agli atti perche' l'ordine di lavoro
 * che propone e' stato seguito.
 *
 * AGGIORNATO AL LOTTO F5. Delle ventiquattro funzioni emesse del gruppo F ne
 * restano OTTO, per 16000 byte:
 *
 *   mutual_test                  0xffffff8008a620d4   4584   t
 *   open_test_sp                 0xffffff8008a632bc   3932   t
 *   open_test_cap                0xffffff8008a64218   1828   t
 *   key_test                     0xffffff8008a6493c   1368   t
 *   create_mp_test_frame_buffer  0xffffff8008a651d0   1388   t
 *   allnode_open_cdc_data        0xffffff8008a65af4   1176   t
 *   allnode_open_cdc_result      0xffffff8008a5e150    984   T
 *   pin_test                     0xffffff8008a64eec    740   t
 *                                       somma:      16000
 *
 * 22672 + 16000 = 38672, e il conto del confine chiude.
 *
 * CINQUE DELLE OTTO SONO ADESSO ANCHE CINQUE SIMBOLI IRRISOLTI AL LINK, e
 * sono le cinque che `ilitek_tddi_mp_init_item` installa nella tavola degli
 * elementi: `mutual_test`, `open_test_sp`, `open_test_cap`, `key_test`,
 * `pin_test`, 12452 byte. Prima del lotto F5 nessuna di esse era citata da
 * codice scritto, e per questo il conto degli irrisolti ILITEK era UNO.
 *
 * LA DOMANDA CHE LE BLOCCAVA E' RISOLTA. Il lotto F1 aveva lasciato agli atti
 * che `parser_get_int_data` (riga 566) non fosse determinabile, e con essa
 * cinque funzioni. La forma che soddisfa tutti e tre i fatti che F1
 * elencava e' scritta in questo file -- il cappello di `parser_get_int_data`
 * spiega quale dei tre fatti non era una proprieta' di quella funzione ma del
 * suo chiamante. Delle otto che restano, quindi, NESSUNA e' bloccata da una
 * domanda aperta: sono lavoro.
 *
 * ORDINE CONSIGLIATO PER CHI PROSEGUE, e il perche':
 *   1. `pin_test` (740) e `create_mp_test_frame_buffer` (1388): non hanno
 *      funzioni incorporate dentro, e chiudono le NOVE citazioni
 *      `NON_ANCORATA` che tengono `verificacitazioni.py` a ESITO=1 -- sei
 *      messaggi della seconda e tre della prima, citati nel cappello di
 *      `struct ilitek_mp_item` per nominarne i campi.
 *   2. `key_test` (1368), che porta dentro `allnode_key_cdc_data`
 *      (1410..1484): con essa e con `pin_test` gli irrisolti ILITEK
 *      scendono da cinque a tre.
 *   3. `mutual_test` (4584), che porta dentro `codeToOhm` (1369),
 *      `mp_cdc_init_cmd_common` (1568) e `allnode_mutual_cdc_data`
 *      (1713..1795).
 *   4. `open_test_cap` (1828) e `open_test_sp` (3932).
 *   5. `allnode_open_cdc_data` (1176) e `allnode_open_cdc_result` (984), che
 *      non sono citate da nessuna funzione scritta e quindi non spostano il
 *      link.
 *
 * ===========================================================================
 * LA DOMANDA (NON PIU') APERTA SU `parser_get_int_data` -- CHIUSA DA F5
 * ===========================================================================
 * QUESTA SEZIONE E' STORIA, E RESTA AGLI ATTI PERCHE' LA RISPOSTA SI CAPISCE
 * SOLO CON LA DOMANDA DAVANTI. Il lotto F1 la scrisse come domanda aperta e
 * ci blocco' cinque funzioni; il lotto F5 l'ha chiusa, e la forma che
 * soddisfa tutti e tre i fatti sta nel cappello di `parser_get_int_data`, in
 * fondo a questo file. IN DUE RIGHE: i fatti 1 e 2 sono proprieta' della
 * funzione e li produce `return (ret < 0) ? 0 : snprintf(...)` con la
 * `snprintf` chiamata SEMPRE; il fatto 3 -- l'argomento `%d` costante zero
 * sul ramo d'errore -- NON e' una proprieta' di `parser_get_int_data` ma del
 * suo chiamante, che stampa una PROPRIA variabile, e per questo nessuna forma
 * della funzione poteva produrlo. Cercare la spiegazione dentro la funzione
 * sbagliata e' costato un lotto: e' la stessa classe di errore di «un
 * confine assolto per nome».
 *
 * IL TESTO ORIGINALE DELLA DOMANDA, non toccato:
 *
 * `parser_get_int_data` (riga 566) e' incorporata in CINQUE funzioni del
 * gruppo F. Il suo corpo si legge quasi tutto, ma UN pezzo non torna, e la
 * regola 2 dice che un conto che non torna e' un sintomo. Ecco l'evidenza,
 * presa da DUE siti di incorporazione diversi.
 *
 * In `allnode_open_cdc_data`:
 *
 *   "97fff0da bl"@0xffffff8008a65c20   -> parser_get_ini_key_value
 *   "2a0003f5 mov"@0xffffff8008a65c28  -> w21 = il suo valore di ritorno
 *   "94101261 bl"@0xffffff8008a65c3c   -> snprintf(rv, 0x80, "%s", value)
 *   "37f80055 tbnz"@0xffffff8008a65c40 -> se w21 < 0 SALTA la prova seguente
 *   "37f81000 tbnz"@0xffffff8008a65c44 -> se w0 < 0 va al ramo d'errore
 *
 * In `mutual_test`, le stesse cinque istruzioni con gli stessi ruoli:
 *
 *   "97fffedf bl"@0xffffff8008a6240c, "2a0003f5 mov"@0xffffff8008a62414,
 *   "94102066 bl"@0xffffff8008a62428, "37f80055 tbnz"@0xffffff8008a6242c,
 *   "37f86e20 tbnz"@0xffffff8008a62430.
 *
 * Se ne leggono TRE fatti, e il terzo li contraddice:
 *
 *   1. la `snprintf` e' chiamata INCONDIZIONATAMENTE dopo
 *      `parser_get_ini_key_value` -- non c'e' nessun ritorno anticipato in
 *      mezzo, e una `snprintf` non e' speculabile dal compilatore, quindi il
 *      sorgente la chiama sempre;
 *   2. il chiamante entra nel ramo d'errore SOLO SE il ritorno del parser e'
 *      >= 0 E quello della `snprintf` e' < 0. Se il parser fallisce, il
 *      collaudo PROSEGUE;
 *   3. sul ramo d'errore l'argomento `%d` del messaggio
 *      "Failed to parse PV54 command, ret = %d" e' la COSTANTE ZERO
 *      ("2a1f03e3 mov"@0xffffff8008a65e58, w3 = wzr) su TUTTI i cammini che
 *      ci arrivano, compreso quello che passa per il messaggio
 *      "Parameters are invalid" (riga 566) -- che e' il ramo in cui il
 *      puntatore e' nullo, "b4001115 cbz"@0xffffff8008a65c0c.
 *
 * Il punto 3 dice che la variabile stampata NON e' il valore di ritorno della
 * funzione incorporata (quel valore e' negativo su quel cammino, altrimenti
 * il ramo d'errore non si prenderebbe), ma un'altra variabile che il
 * compilatore sa valere zero. Il punto 2 dice che la funzione incorporata
 * torna un valore NON negativo quando il parser fallisce. Nessuna forma
 * naturale di `parser_get_int_data` -- e ne sono state provate sei sulla
 * carta -- produce tutti e tre i fatti insieme.
 *
 * QUESTO E' UN RISULTATO, non una resa: e' un vincolo preciso che il lotto
 * successivo deve soddisfare, e va soddisfatto con la misura (scrivere una
 * forma, compilare, contare) e non con la plausibilita'. Finche' non e'
 * risolto, `parser_get_int_data` non va scritta, e con lei non vanno scritte
 * le cinque funzioni che la contengono.
 *
 * [FINE DEL TESTO DEL LOTTO F1. La forma trovata dal lotto F5 e' scritta nel
 * cappello di `parser_get_int_data`, e il fatto 3 e' spiegato li'.]
 *
 * DI SICURO SI SA GIA' QUESTO, ed e' misurato: la funzione prende almeno
 * quattro argomenti, perche' la DIMENSIONE della `snprintf` cambia da sito a
 * sito -- 0x80 = 128 in `allnode_open_cdc_data`
 * ("321903e1 orr"@0xffffff8008a65c38) e in `mutual_test`
 * ("321903e1 orr"@0xffffff8008a62420), 0x200 = 512 in
 * `ilitek_tddi_mp_test_main` ("321703e1 orr"@0xffffff8008a5f258), 0x20 = 32
 * in `allnode_open_cdc_result` ("321b03e1 orr"@0xffffff8008a5e398). TRE
 * valori diversi in QUATTRO siti di incorporazione: e' un ARGOMENTO, non una
 * costante del corpo.
 *
 * E SI SA ANCHE CHE IL BUFFER INTERMEDIO E' DI 512 BYTE, sempre: la `memset`
 * che lo azzera prima dell'uso passa 0x200 in tutti i siti --
 * "321703e2 orr"@0xffffff8008a65c00 in `allnode_open_cdc_data`,
 * "321703e2 orr"@0xffffff8008a5f298 in `ilitek_tddi_mp_test_main`,
 * "321703e2 orr"@0xffffff8008a5e35c in `allnode_open_cdc_result`. Quello, a
 * differenza della dimensione dell'uscita, e' una costante del corpo.
 *
 * ===========================================================================
 * IL DELTA DI HEADER -- DA FARE NEL LOTTO DI MERGE, NON QUI
 * ===========================================================================
 * `ilitek.h` non e' stato toccato. Servirebbero, e vanno messe li' dal lotto
 * che unisce i gruppi:
 *
 *   1. `struct ilitek_ini_item` (2212 byte, passo provato da
 *      "9122935a add"@0xffffff8008a62040 e "5281149c mov"@0xffffff8008a5db54)
 *      e i due globali `ilitek_ini_num` / `ilitek_ini_data`, che qui sono
 *      duplicati come dichiarazioni locali;
 *   2. `struct ilitek_core_mp` e l'oggetto `core_mp` -- il nome viene DAL
 *      BINARIO, dal formato
 *      il messaggio a 0xffffff800924477e (nota 1),
 *      la cui `w3` e' "b94bfaa3 ldr"@0xffffff8008a64a64, cioe' l'indirizzo
 *      0xffffff800a0fcbf8: quindi `key_len` e' il nome di fabbrica del campo
 *      a +248 rispetto alla base scelta qui. E' l'eccezione alla regola 5
 *      (una stringa di formato che nomina il campo), e va usata;
 *   3. il prototipo di `katoi` (0xffffff8008a6b5b8), che sta oltre il gruppo
 *      del flash e che il gruppo F non puo' compilare senza. Qui e' dichiarata
 *      e NON definita, come vuole la regola 6: il link fallisce, ed e' l'esito
 *      onesto.
 *
 * Il lotto F2 ne aggiunge quattro, e nessuno di essi e' stato fatto qui:
 *
 *   4. `struct ilitek_core_mp` va ALLARGATA. Il lotto F1 la fermava al campo
 *      a +256; F2 ne misura altri otto: `c16` (u32, confrontato SENZA segno
 *      con 0x10401), `c228`, `c236`, `c352`, `c356` e i SEI puntatori
 *      consecutivi da +304 a +344. La definizione locale qui sotto e' quella
 *      da portare in `ilitek.h`, con l'avvertenza che la BASE 0xffffff800a0fcb00
 *      resta una SCELTA (divergenza F1-D6) e che tutti i riempimenti fra i
 *      campi misurati sono scelte obbligate dagli offset, non misure.
 *      ATTENZIONE: `c228`, `c352` e `c356` NON sono usati da nessuna delle
 *      quattordici funzioni scritte qui -- compaiono solo nell'analisi di
 *      `allnode_open_cdc_data`, che non e' stata scritta -- e percio' NON
 *      sono nella definizione qui sotto: sarebbe stato dichiarare un campo su
 *      un'evidenza che il file non porta.
 *   5. `struct ilitek_mp_item` e la tavola `mp_items[50]` a
 *      0xffffff80099876d0 (passo 152, provato da
 *      "5280130f mov"@0xffffff8008a65888 con
 *      "9b2f3810 smaddl"@0xffffff8008a65890). Il NOME e' scelto, e la
 *      ricognizione usa `tItems`, che nel binario NON esiste: se il lotto di
 *      merge sceglie un nome, lo dichiari scelto.
 *   6. i prototipi di `mp_comp_result_before_retry` (0xffffff8008a65f8c) e
 *      delle altre funzioni del gruppo F non ancora scritte. Qui
 *      `mp_comp_result_before_retry` e' dichiarata e NON definita (regola 6).
 *   7. `verificacitazioni.py` -- che NON e' un header ma e' comunque uno
 *      strumento condiviso: aggiungere `"KERN_CONT": "c"` a `LIVELLO_KERN` e
 *      allargare `RE_TESTA_ASSEMBLATA` a `\A\\x01[0-7c]`, e ammettere che
 *      uno stesso letterale sia citato a piu' indirizzi. Le due mancanze
 *      hanno costretto questo lotto alla macro `ILI_CONT` (divergenza F2-D4)
 *      e a lasciare `\n` fra le `--eccezione`.
 *
 * Finche' quel lotto non esiste, le dichiarazioni stanno qui sotto, come gia'
 * fa `ilitek_bus.c` per le funzioni del gruppo E.
 *
 * ===========================================================================
 * I TRE LETTERALI KERN_CONT DI F1, E UN LIMITE DI `verificacitazioni.py`
 * ===========================================================================
 * NOTA DEL LOTTO F2: questa sezione descrive la situazione dei tre letterali
 * KERN_CONT del lotto F1, che restano `--eccezione`. Gli OTTO letterali
 * KERN_CONT che F2 aggiunge NON sono eccezioni: passano per la macro locale
 * `ILI_CONT` e sono citati alla coda, quindi verificati. Il perche' e il costo
 * stanno nel cappello di quella macro, sopra le dichiarazioni, e la scelta e'
 * dichiarata come divergenza F2-D4. I tre di F1 non sono stati riscritti:
 * "le citazioni gia' scritte non si riscrivono".
 *
 * Tre letterali di questo file -- "%d, ", "\n" e "Dump Denchmark Min\n" --
 * sono passati a `printk` con `KERN_CONT`, cioe' con il prefisso SOH + 'c'
 * (linux/kern_levels.h: `#define KERN_CONT KERN_SOH "c"`). Nel binario sono
 * a 0xffffff80092430dc, 0xffffff80090ddcf1 e 0xffffff8009243e3d.
 *
 * `verificacitazioni.py` NON li sa ancorare, e non e' un difetto di questo
 * file: la sua tabella `LIVELLO_KERN` ha le otto macro da KERN_EMERG a
 * KERN_DEBUG e NON ha KERN_CONT, e `RE_TESTA_ASSEMBLATA` e' `\A\\x01[0-7]`,
 * cioe' SOH seguito da una CIFRA -- 'c' non e' una cifra. Le conseguenze
 * sono due, e vanno dette invece che aggirate:
 *
 *   - una citazione nella forma "\x01c...."@0xINDIRIZZO esce NON_ANCORATA;
 *   - una citazione ancorata alla CODA (cioe' al testo che comincia due byte
 *     dopo, saltando SOH e 'c') farebbe sollevare allo strumento un
 *     `KeyError: macro di livello sconosciuta: 'KERN_CONT'`, perche'
 *     `bytes_attesi` cerca 'KERN_CONT' dentro `LIVELLO_KERN` e non lo trova.
 *     Provato: lo strumento termina con una traccia di errore, non con un
 *     esito. (E provato due volte, la seconda per sbaglio: la citazione
 *     d'esempio scritta QUI DENTRO, in un commento, bastava a farlo cadere.
 *     `verificacitazioni.py` legge le citazioni anche dai commenti, ed e'
 *     giusto cosi'.)
 *
 * Quindi i tre non sono citati nella forma verificabile, sono dichiarati con
 * `--eccezione` e si verificano a mano, con l'uscita LETTERALE qui sotto:
 *
 *   ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf \
 *       0xffffff80092430dc 0xffffff80090ddcf1 0xffffff8009243e3d
 *
 *   0xffffff80092430dc: b'\x01c%d, '  (len=6, hex=016325642c20)
 *   0xffffff80090ddcf1: b'\x01c\n'  (len=3, hex=01630a)
 *   0xffffff8009243e3d: b'\x01cDump Denchmark Min\n'  (len=21,
 *       hex=016344756d702044656e63686d61726b204d696e0a)
 *
 * Il comando completo della prima verifica e' quindi:
 *
 *   ./venv/bin/python3 verificacitazioni.py ilitek_mp.c oracolo/stock.elf \
 *       --eccezione '%d, ' --eccezione '\n' \
 *       --eccezione 'Dump Denchmark Min\n'
 *
 * Il DELTA DI STRUMENTO -- aggiungere "KERN_CONT": "c" a `LIVELLO_KERN` e
 * allargare `RE_TESTA_ASSEMBLATA` a `\A\\x01[0-7c]` -- NON e' stato fatto:
 * `verificacitazioni.py` e' uno strumento condiviso, e questo lotto non tocca
 * file condivisi. La ricognizione conta 42 siti KERN_CONT nel solo driver
 * ilitek, quindi il limite si ripresentera'.
 *
 * NOTA 1 -- stessa ragione, caso diverso. Il messaggio che NOMINA `core_mp`
 * non e' citato nella forma verificabile perche' la sua coda ancorabile
 * sarebbe il letterale "\n", che in questo file compare anche sotto
 * KERN_CONT e farebbe sollevare allo strumento lo stesso `KeyError`. Si
 * verifica a mano, uscita LETTERALE:
 *
 *   ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf 0xffffff800924477e
 *
 *   0xffffff800924477e: b'\x016ILITEK: (%s, %d): core_mp.key_len = %d\n'
 *       (len=41, hex=0136494c4954454b3a202825732c202564293a20636f72655f6d70
 *       2e6b65795f6c656e203d2025640a)
 */

#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/firmware.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/timekeeping.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#include "ilitek.h"

/*
 * ===========================================================================
 * `ILI_CONT` -- PERCHE' ESISTE, E PERCHE' NON E' UN AGGIRAMENTO
 * ===========================================================================
 * Il gruppo F stampa moltissimo con `KERN_CONT`, cioe' con il prefisso
 * SOH + 'c' (linux/kern_levels.h: `#define KERN_CONT KERN_SOH "c"`).
 * `verificacitazioni.py` NON sa trattare quel livello: la sua tabella
 * `LIVELLO_KERN` ha le otto macro da KERN_EMERG a KERN_DEBUG, e
 * `bytes_attesi` solleva `KeyError: macro di livello sconosciuta:
 * 'KERN_CONT'` appena la sua `RE_LETTERALE_CODICE`
 * (`(?:(KERN_\w+)\s+)?"..."`) vede un letterale ADIACENTE a `KERN_CONT` nel
 * sorgente. Il lotto F1 se ne accorse e dichiaro' i suoi tre letterali con
 * `--eccezione`, cioe' li tolse dalla verifica.
 *
 * Qui i letterali KERN_CONT sono OTTO, e toglierne otto dalla verifica
 * sarebbe una perdita vera. La soluzione e' una macro locale: nel testo del
 * sorgente il letterale non e' piu' adiacente a `KERN_CONT` -- lo diventa
 * solo dopo la sostituzione del preprocessore -- quindi lo strumento lo legge
 * come letterale semplice e lo puo' ancorare alla CODA della stringa nel
 * binario, cioe' all'indirizzo del testo due byte dopo SOH+'c'. Le otto
 * citazioni tornano verificabili invece che scusate.
 *
 * IL TESTO PREPROCESSATO E' IDENTICO a `printk(KERN_CONT "...", ...)`: la
 * macro non aggiunge ne' toglie una istruzione, e la misura lo conferma.
 * E' un artificio di SORGENTE dichiarato, della stessa natura delle direttive
 * `#line` (divergenza F1-D1), non una modifica al codice emesso. Il DELTA DI
 * STRUMENTO che chiuderebbe la questione per tutti -- aggiungere
 * `"KERN_CONT": "c"` a `LIVELLO_KERN` e allargare `RE_TESTA_ASSEMBLATA` a
 * `\A\\x01[0-7c]` -- resta NON FATTO, perche' `verificacitazioni.py` e' uno
 * strumento condiviso.
 */
#define ILI_CONT(fmt, ...)	printk(KERN_CONT fmt, ##__VA_ARGS__)

/*
 * ===========================================================================
 * `ILI_KFREE` -- IL NOME E' SCELTO, LA FORMA E' MISURATA
 * ===========================================================================
 * `create_mp_test_frame_buffer` chiude DODICI rami d'errore con la stessa
 * sequenza di quattro istruzioni, sempre identica a meno dell'indirizzo:
 *
 *   "f9461a60 ldr"@0xffffff8008a654d8    x0 = *p
 *   "b4001260 cbz"@0xffffff8008a654dc    se x0 == 0 salta
 *   "97dfacb4 bl"@0xffffff8008a654e0     kfree(x0)
 *   "f9061a7f str"@0xffffff8008a654e4    *p = NULL
 *
 * cioe' `if (p) { kfree(p); p = NULL; }`. QUESTO E' MISURATO.
 *
 * CHE NEL SORGENTE DI FABBRICA SIA UNA RIGA SOLA lo dicono i numeri di riga,
 * ed e' un argomento indipendente dal codice emesso: i dodici messaggi
 * d'errore stanno alle righe 1891, 1900, 1909, 1918, 1927, 1936, 1945, 1954,
 * 1963, 1972, 1982, 1990, e le prime sette differenze consecutive valgono
 * NOVE righe esatte. Nove righe sono esattamente: il messaggio, la
 * liberazione, il `return`, due parentesi chiuse, una riga vuota, il
 * `if (!p)`, l'allocazione, il `if (errore)`. Se la liberazione occupasse
 * quattro righe (la forma espansa) il passo sarebbe dodici, non nove.
 *
 * IL NOME `ILI_KFREE` E' SCELTO DA QUESTO LOTTO: il binario da' la forma, non
 * il nome. Una macro e la forma espansa producono lo stesso codice emesso, e
 * la misura non le distingue.
 */
#define ILI_KFREE(p)					\
	do {						\
		if (p) {				\
			kfree(p);			\
			(p) = NULL;			\
		}					\
	} while (0)

/*
 * ===========================================================================
 * LE DICHIARAZIONI LOCALI (vedi "IL DELTA DI HEADER" nel cappello)
 * ===========================================================================
 */

/*
 * Una voce del file .ini letto dal collaudo. PASSO 2212 = 0x8a4, misurato in
 * due punti indipendenti: "5281149c mov"@0xffffff8008a5db54 (w28 = 0x8a4,
 * moltiplicatore delle `madd` di `parser_ini_nodetype`) e
 * "9122935a add"@0xffffff8008a62040 (l'avanzamento del cursore in
 * `parser_get_ini_key_value`). Gli offset dei campi vengono dagli accessi:
 * +0 e' il primo argomento di `strstr` ("94102c01 bl"@0xffffff8008a5db64),
 * +100 = 0x64 e' l'argomento di `strlen`/`strncmp`
 * ("91019117 add"@0xffffff8008a62008), +200 = 0xc8 e' l'array dei caratteri
 * ("91032101 add"@0xffffff8008a62070, "3943218c ldrb"@0xffffff8008a5dbc0),
 * +2208 = 0x8a0 e' un `int` con segno ("b948a109 ldr"@0xffffff8008a6206c,
 * e "37f807cb tbnz"@0xffffff8008a5dba4 lo prova con segno perche' ne prova
 * il bit 31).
 *
 * ATTENZIONE, correzione del 2026-08-21: 100 + 100 + 2008 + 4 = 2212 chiude
 * il conto, ma la larghezza 2008 di `c200` e' una SCELTA, non una misura. Il
 * binario misura il PASSO (2212) e QUATTRO offset (+0, +100, +200, +2208);
 * non misura dove finisce `c200`. Qualunque partizione dell'intervallo
 * [200, 2208) in "array di N caratteri piu' riempimento" produce esattamente
 * lo stesso codice emesso. Nessuna istruzione del gruppo F cita un limite
 * superiore di `c200`: l'unico 2008 che compare in tutto il gruppo e' un
 * `__LINE__`, "5280fb02 mov"@0xffffff8008a5e564, dentro `check_int_level`, e
 * non ha niente a che vedere con questa struttura. La prima stesura scriveva
 * che il passo "fissa la larghezza dei tre array": presentava una scelta come
 * una determinazione, ed e' corretta qui prima che il lotto di merge la
 * erediti come misura.
 *
 * I NOMI DEI CAMPI NON SONO NEL BINARIO: `c<offset>` come vuole la regola 5.
 */
struct ilitek_ini_item {
	char c0[100];		/* nome di sezione */
	char c100[100];		/* nome di chiave */
	char c200[2000];	/* testo del valore */
	/*
	 * +2200 e +2204: DUE `int` che il lotto F5 trova dentro quello che F1
	 * dichiarava riempimento. Sono la MISURA che mancava alla larghezza di
	 * `c200`: "b9089900 str"@0xffffff8008a603bc scrive a +2200 e
	 * "b9089d09 str"@0xffffff8008a60428 a +2204, quindi `c200` finisce al
	 * piu' a 2200 -- ed e' esattamente il limite che
	 * "711f445f cmp"@0xffffff8008a604c4 (#0x7d1 = 2001) impone al suo
	 * contenuto. Il conto chiude: 100 + 100 + 2000 + 4 + 4 + 4 = 2212.
	 * La nota di F1 diceva, con onesta', «la larghezza 2008 di `c200` e'
	 * una SCELTA, non una misura»: adesso la misura c'e' e la scelta era
	 * sbagliata di otto byte.
	 */
	/*
	 * CORREZIONE del 2026-08-22 (revisione del lotto F5): le DUE `ldrsw`
	 * citate qui erano attaccate al campo SBAGLIATO -- erano spostate di
	 * un campo. Le tre letture di fabbrica, per offset, sono queste e sono
	 * tutte le letture che esistono:
	 *
	 *   ffffff8008a605a0:	b9489908	ldr	w8, [x8,#2200]
	 *   ffffff8008a60558:	b9889d42	ldrsw	x2, [x10,#2204]
	 *   ffffff8008a604c0:	b988a142	ldrsw	x2, [x10,#2208]
	 *
	 * cioe' +2200 e' letto SOLO a 32 bit, e del suo segno il binario non
	 * dice niente.
	 */
	int c2200;		/* lunghezza del nome di sezione. RUOLO misurato:
				 * "b9489908 ldr"@0xffffff8008a605a0 lo rilegge e
				 * "7101951f cmp"@0xffffff8008a605a4 lo confronta
				 * con 0x65 = 101, il limite del nome (100).
				 * IL SEGNO E' UNA SCELTA, NON UNA MISURA: quella
				 * `ldr` e' a 32 bit e nessun'altra istruzione del
				 * gruppo F legge +2200. `int` per simmetria con
				 * gli altri due, che invece sono provati */
	int c2204;		/* lunghezza del nome di chiave, con segno:
				 * "b9889d42 ldrsw"@0xffffff8008a60558 legge +2204
				 * e lo estende CON SEGNO; il ruolo lo prova cio'
				 * che segue -- "7101945f cmp"@0xffffff8008a6055c
				 * (0x65 = 101) e la copia a +100
				 * ("91019100 add"@0xffffff8008a60568) */
	int c2208;		/* lunghezza del valore, con segno:
				 * "b988a142 ldrsw"@0xffffff8008a604c0 legge +2208
				 * e lo estende CON SEGNO; il ruolo lo prova
				 * "711f445f cmp"@0xffffff8008a604c4 (0x7d1 = 2001)
				 * e la copia a +200
				 * ("91032100 add"@0xffffff8008a604d0) */
};

/*
 * `core_mp`: IL NOME VIENE DAL BINARIO (eccezione alla regola 5) -- il
 * formato il messaggio a 0xffffff800924477e (nota 1)
 * lo nomina, e il suo argomento e' "b94bfaa3 ldr"@0xffffff8008a64a64, cioe'
 * l'indirizzo 0xffffff800a0fcbf8.
 *
 * ATTENZIONE (divergenza F1-D6): la BASE 0xffffff800a0fcb00 e' una scelta.
 * Di misurato c'e' solo che i due campi usati qui stanno a 24 byte l'uno
 * dall'altro: "b94beb08 ldr"@0xffffff8008a5daac legge pagina+3048
 * (0xffffff800a0fcbe8) e "b94c02c8 ldr"@0xffffff8008a5da7c legge pagina+3072
 * (0xffffff800a0fcc00). La struttura vera e' molto piu' grande (la
 * ricognizione la vede fino a 0xffffff800a0fcc70): qui c'e' solo quanto serve
 * a F1.
 */
struct ilitek_core_mp {
	/*
	 * I PRIMI 232 BYTE, aperti dal lotto F5. Il lotto F1 li dichiarava
	 * riempimento (`u8 c0[232]`) perche' nessuna sua funzione li toccava:
	 * li scrive `ilitek_tddi_mp_init_item`, che e' incorporata dentro
	 * `ilitek_tddi_mp_test_main` e che il lotto F5 ha letto.
	 * IL PRIMO ARGOMENTO DI OGNI `printk` DI QUEL BLOCCO E' IL FORMATO E
	 * IL SECONDO IL `__func__` (classe B1): verificato leggendo le due
	 * stringhe, "ilitek_tddi_mp_init_item"@0xffffff800924384f e' il nome.
	 */
	int c0;		/* +0: "b90002ea str"@0xffffff8008a5e6ec, quattro byte;
			 * stampato da "\x016ILITEK: (%s, %d): CHIP = 0x%x\n"
			 * @0xffffff80092438b5 */
	u16 c4;		/* +4: "79000aea strh"@0xffffff8008a5e6f4, DUE byte */
	u8 c6;		/* +6: "39001aea strb"@0xffffff8008a5e6fc */
	u8 c7;		/* +7: "39001eea strb"@0xffffff8008a5e704 */
	int c8;		/* +8: "b9000aea str"@0xffffff8008a5e70c; stampato da
			 * "\x016ILITEK: (%s, %d): Firmware version = %x\n"
			 * @0xffffff80092438d6 */
	int c12;	/* +12: prima parola di "2901a6eb stp"@0xffffff8008a5e718;
			 * "\x016ILITEK: (%s, %d): Protocol version = %x\n"
			 * @0xffffff8009243901. E' anche il campo che il
			 * controllo di versione del collaudo confronta con
			 * `str2hex` del valore .ini "protocol", spostato di 8
			 * bit: "b94b0d08 ldr"@0xffffff8008a5ec6c +
			 * "6b48201f cmp"@0xffffff8008a5ec70 (`lsr #8`) */
	int c16;	/* +16: seconda parola della stessa `stp`;
			 * "\x016ILITEK: (%s, %d): Core version = %x\n"
			 * @0xffffff800924392c */
	/*
	 * +20 e +148: DUE BUFFER DI TESTO. Che siano due buffer distinti e'
	 * misurato -- `parser_get_ini_key_value` ci scrive dentro con x2 =
	 * 0xffffff800a0fcb14 ("912c5273 add"@0xffffff8008a608f8) e x2 =
	 * 0xffffff800a0fcb94 ("912e5273 add"@0xffffff8008a60934). Le due
	 * LARGHEZZE (128 e 64) sono una SCELTA obbligata dagli offset agli
	 * estremi, non una misura.
	 *
	 * DIFETTO DELLA FABBRICA, RIPRODOTTO (regola 7): nel buffer a +20 si
	 * legge la chiave "date"@0xffffff80091c8500 e lo si stampa sotto
	 * l'etichetta "INI Release Version ,%s\n"@0xffffff800924581f; nel
	 * buffer a +148 si legge "version"@0xffffff800926923d e lo si stampa
	 * sotto "INI Release Date ,%s\n"@0xffffff8009245838. I due sono
	 * SCAMBIATI, e si riproducono scambiati.
	 */
	char c20[128];
	char c148[64];
	int c212;	/* +212: "b900d6e9 str"@0xffffff8008a5e730, quattro byte */
	u8 c216;	/* +216: "390362ff strb"@0xffffff8008a5e744 lo azzera; e'
			 * la guardia del ritentativo --
			 * "396f6108 ldrb"@0xffffff8008a60018 +
			 * "34000128 cbz"@0xffffff8008a6001c saltano insieme il
			 * messaggio "MP failed, doing retry" e la chiamata a
			 * `mp_do_retry`. NESSUNA delle 24 funzioni del gruppo F
			 * lo SCRIVE con un valore diverso da zero: il
			 * ritentativo, di fabbrica, non parte mai */
	u8 c217[10];	/* +217..226: riempimento dichiarato */
	/*
	 * +227 = 0xffffff800a0fcbe3. APERTO DAL LOTTO F6: il lotto F5 lo
	 * dichiarava riempimento dentro `c217[11]`, perche' nessuna
	 * funzione scritta lo toccava. Lo legge `mutual_test`, nel caso
	 * `c16 == 9`: "396f8d4a ldrb"@0xffffff8008a62e48 lo legge a UN
	 * byte e "7100015f cmp"@0xffffff8008a62e50 +
	 * "9a96032a csel"@0xffffff8008a62e54 lo usa per scegliere quale
	 * delle due coppie di parametri passare a `codeToOhm`. NESSUN
	 * messaggio lo nomina: `c<offset>` (regola 5).
	 */
	u8 c227;
	int c228;	/* +228: "b900e6e9 str"@0xffffff8008a5e724;
			 * "\x016ILITEK: (%s, %d): Read CDC Length = %d\n"
			 * @0xffffff8009243953 */
	int c232;	/* 0xffffff800a0fcbe8 -- `int` con segno: e' il divisore
			 * di "1ac80ee9 sdiv"@0xffffff8008a5dab0 (divisione CON
			 * SEGNO) e il numero di colonne per riga della stampa */
	int c236;	/* 0xffffff800a0fcbec -- `int` con segno, il limite
			 * dell'indice di RIGA: "b94bed6b ldr"@0xffffff8008a5e078
			 * lo legge e "6b0b01ff cmp"@0xffffff8008a5e0b4 con
			 * "5400018a b.ge"@0xffffff8008a5e0b8 lo usa come
			 * estremo superiore con confronto CON SEGNO. Nella
			 * prima stesura di F1 questo campo non esisteva: i
			 * quattro byte stavano dentro un riempimento `u8
			 * c236[20]`, perche' nessuna funzione di F1 li tocca */
	int c240;	/* +240: azzerato da "a90f7eff stp"@0xffffff8008a5e73c,
			 * che copre SEDICI byte, 240..255 */
	int c244;	/* +244: seconda parola della stessa `stp` */
	/*
	 * +248: IL NOME VIENE DAL BINARIO (eccezione alla regola 5). Il
	 * formato a 0xffffff800924477e si legge
	 * "ILITEK: (%s, %d): core_mp.key_len = %d" e il suo argomento e'
	 * "b94bfaa3 ldr"@0xffffff8008a64a64, cioe' 0xffffff800a0fcbf8 =
	 * questo campo. Il lotto F5 lo legge in tre punti del dump CSV dei
	 * tasti: "b94bfaaa ldr"@0xffffff8008a610f4,
	 * "b98bfaaa ldrsw"@0xffffff8008a61158 (estensione CON SEGNO, quindi
	 * `int`) e "b94bfaa9 ldr"@0xffffff8008a61a4c.
	 */
	int key_len;
	int c252;	/* +252: seconda meta' della `stp` che azzera 248..255 */
	int c256;	/* 0xffffff800a0fcc00 -- `int` con segno: letto sia
			 * "b94c02c8 ldr"@0xffffff8008a5da7c (guardia del ciclo)
			 * sia "b98c02c8 ldrsw"@0xffffff8008a5dacc (estensione
			 * CON SEGNO all'indice). E' il numero di nodi */
	int c260;	/* +260: nessuna funzione scritta lo tocca */
	/*
	 * +264: L'ESITO COMPLESSIVO DEL COLLAUDO. Tre scritture misurate:
	 * "b9010af4 str"@0xffffff8008a5e750 lo mette a -1 all'inizializzazione
	 * (w20 = -1 da "12800014 mov"@0xffffff8008a5e6d8),
	 * "b900011f str"@0xffffff8008a61c70 lo azzera quando tutti gli
	 * elementi eseguiti hanno esito >= 0, e
	 * "b90c0909 str"@0xffffff8008a61cac lo rimette a -1 altrimenti.
	 * `mp_test_free` lo riporta a -1: "b90c0913 str"@0xffffff8008a5f044.
	 */
	int c264;
	u8 c268[20];	/* +268..287: riempimento dichiarato */
	/*
	 * QUATTRO `int` CONSECUTIVI, 0xffffff800a0fcc20..0xffffff800a0fcc2f,
	 * aggiunti dal lotto F4: prima stavano dentro il riempimento
	 * `c260[44]`, perche' nessuna funzione di F1/F2/F3 li tocca.
	 * `mp_comp_result_before_retry` li legge a COPPIE, con due `ldp` di
	 * due parole ciascuna, e le coppie sono adiacenti: e' quello che prova
	 * che sono quattro campi a 4 byte e non due a 8.
	 */
	int c288;	/* 0xffffff800a0fcc20: prima parola della coppia
			 * "2944354c ldp"@0xffffff8008a66084 (x10 = pagina+3072,
			 * scostamento 32) */
	int c292;	/* 0xffffff800a0fcc24: seconda parola della stessa `ldp` */
	int c296;	/* 0xffffff800a0fcc28: prima parola della coppia
			 * "2945354c ldp"@0xffffff8008a663a0 (stesso x10,
			 * scostamento 40) */
	int c300;	/* 0xffffff800a0fcc2c: seconda parola della stessa `ldp`.
			 * IL SEGNO NON E' MISURATO: i quattro valori vengono
			 * solo copiati in un array di `int`, mai confrontati */
	/*
	 * SEI PUNTATORI CONSECUTIVI, 0xffffff800a0fcc30..0xffffff800a0fcc58.
	 * Che siano puntatori e' misurato: sono letti con `ldr x` (otto byte)
	 * e il valore letto viene subito usato come base di un `ldr w`/`str w`
	 * scalato di 4. Che siano SEI e consecutivi lo dice
	 * `compare_MaxMin_result`, che li tocca tutti in ventiquattro
	 * istruzioni.
	 */
	int *tx_delta_buf;	/* "f9461964 ldr"@0xffffff8008a65924 (pagina+3120) */
	int *rx_delta_buf;	/* "f9461e25 ldr"@0xffffff8008a65974 (pagina+3128) */
	int *tx_max_buf;	/* "f9400985 ldr"@0xffffff8008a65944 ([x12,#16] con
			 * x12 = 0xffffff800a0fcc30) */
	int *tx_min_buf;	/* "f94625a5 ldr"@0xffffff8008a65964 (pagina+3144) */
	int *rx_max_buf;	/* "f9400e45 ldr"@0xffffff8008a65984 ([x18,#24] con
			 * x18 = 0xffffff800a0fcc38) */
	int *rx_min_buf;	/* "f9462c05 ldr"@0xffffff8008a659a4 (pagina+3160) */
	/*
	 * +352 e +356: DUE `int`, non un `u64`. Che siano due lo prova la
	 * lettura a QUATTRO byte del secondo -- "b94166e8 ldr"@0xffffff8008a5e840
	 * -- mentre l'inizializzazione li scrive insieme con
	 * "f900b2ea str"@0xffffff8008a5e740 (otto byte, x10 = 240 da
	 * "321c0fea orr"@0xffffff8008a5e720): 352 = 240, 356 = 0. Sono due
	 * store adiacenti fusi dal compilatore.
	 */
	int c352;
	int c356;	/* +356: sceglie fra "Polling"@0xffffff800923427d e
			 * "Interrupt"@0xffffff8009234cfb --
			 * "7100011f cmp"@0xffffff8008a5e854 +
			 * "9a890143 csel"@0xffffff8008a5e85c, `eq` sceglie
			 * "Interrupt" */
	u8 c360;	/* +360: nessuna funzione scritta lo tocca */
	u8 c361;	/* +361: "3905a6ff strb"@0xffffff8008a5e758 lo azzera e
			 * "3931a509 strb"@0xffffff8008a5f748 lo mette a 1
			 * quando la chiave .ini "goldenmode" non combacia con
			 * "spec option". Comanda il messaggio
			 * "WARNING! Golden and SPEC in ini file aren't
			 * matched!!" e la scelta del nome del file CSV */
	u8 c362[6];	/* +362..367: riempimento d'allineamento. LA LARGHEZZA
			 * TOTALE E' MISURATA: "52802e02 mov"@0xffffff8008a5e6a8
			 * passa 368 a `__memset` sulla base della struttura
			 * ("aa1703e0 mov"@0xffffff8008a5e6ac, x0 = x23 =
			 * 0xffffff800a0fcb00) */
};

/*
 * ===========================================================================
 * LA TAVOLA DEGLI ELEMENTI DI COLLAUDO -- 0xffffff80099876d0, 50 x 152 byte
 * ===========================================================================
 * IL NOME `mp_items` E' SCELTO, E VA DETTO PERCHE'. La ricognizione
 * (`scout-ilitek-F/RIASSUNTO.md`) chiama questa tavola `tItems` in tutte le
 * sue tabelle. QUEL NOME NON STA NEL BINARIO: `strings oracolo/stock.elf |
 * grep -c "tItems"` stampa `0`, e `grep -c tItems
 * scout-ilitek-F/stringhe_grezze.txt` -- cioe' l'uscita di `leggi_stringa.py`
 * su tutti e 382 gli indirizzi materializzati dal blocco -- stampa anch'esso
 * `0`. Un nome che non e' nel binario e' indistinguibile, per chi legge, da un
 * fatto (classe B2), quindi qui e' rinominato e dichiarato SCELTO.
 *
 * QUELLO CHE E' MISURATO:
 *   - la BASE 0xffffff80099876d0: "d000790e adrp"@0xffffff8008a6587c piu'
 *     "911b41ce add"@0xffffff8008a65884 (#0x6d0);
 *   - il PASSO 152: "5280130f mov"@0xffffff8008a65888 (w15 = 0x98) e
 *     "9b2f3810 smaddl"@0xffffff8008a65890, che moltiplica il primo argomento
 *     per 152 e somma la base -- e' anche la prova che il primo argomento
 *     delle funzioni indicizzate e' l'INDICE dell'elemento, non un puntatore;
 *   - il SEGNO dell'indice: `smaddl` estende w0 CON SEGNO, quindi `int`;
 *   - il numero di elementi, 50, che la ricognizione ha contato in due modi
 *     indipendenti (50 rilocazioni `R_AARCH64_RELATIVE` consecutive a passo
 *     0x98 e il contatore `mov w28, #0x32` del ciclo di inizializzazione).
 *
 * QUANTI CAMPI SONO MISURATI. Il lotto F1/F2 ne usava TRE su 152 byte (+0x10,
 * +0x68, +0x70). Il lotto F3 ne aggiunge NOVE: +0x14, +0x15, +0x24, +0x54,
 * +0x55, +0x56, +0x58, +0x60, +0x78, +0x80 -- e di quattro di essi legge il
 * NOME dal binario (`delay_time`, `test_int_pin`, `int_pulse_test`, piu' i sei
 * `*_buf` che i messaggi d'errore delle allocazioni nominano). Tutto cio' che
 * resta e' riempimento DICHIARATO, e la sua LARGHEZZA e' una scelta obbligata
 * dagli offset misurati agli estremi, non una misura.
 */
struct ilitek_mp_item {
	char *c0;	/* +0x00, otto byte: "f94002a4 ldr"@0xffffff8008a66ca8 lo
			 * legge e "940fe858 bl"@0xffffff8008a5e1a0 lo passa a
			 * `strlen`; in `mp_do_retry` finisce nell'argomento `%s`
			 * del messaggio a 0xffffff800924539e (citato per esteso
			 * sopra `mp_do_retry`). E' un
			 * `char *`, ed e' il nome dell'elemento */
	char *c8;	/* +0x08, otto byte: e' il buffer di ESITO dell'elemento.
			 * `ilitek_tddi_mp_init_item` ci mette il risultato di
			 * `kmalloc(16, GFP_KERNEL)`
			 * ("97dfc0a0 bl"@0xffffff8008a5ea2c verso
			 * <kmem_cache_alloc_trace>, "321c03e2 orr"@0xffffff8008a5ea24
			 * = 16) con "f8178360 stur"@0xffffff8008a5ea30, e vi
			 * scrive subito i cinque byte "FAIL\0" --
			 * "b9000019 str"@0xffffff8008a5ea40 (w25 = 0x4c494146 =
			 * 'F','A','I','L' da "528828d9 mov"@0xffffff8008a5e88c
			 * piu' "72a98939 movk"@0xffffff8008a5e8ac) e
			 * "3900101f strb"@0xffffff8008a5ea44 (il NUL).
			 * `mp_test_free` lo libera: "f85b0280 ldur"@0xffffff8008a5f100
			 * + "97dfc5aa bl"@0xffffff8008a5f108 verso <kfree>.
			 * IL TIPO `char *` E' MISURATO dai cinque byte di
			 * testo; il NOME no */
	int c16;	/* +0x10, `int`: "b94001c4 ldr"@0xffffff8008a658cc,
			 * quattro byte, confrontato con 4 e con 5
			 * ("7100109f cmp"@0xffffff8008a658d0 e
			 * "7100149f cmp"@0xffffff8008a658d8) */
	u8 c20;		/* +0x14, un byte: e' il PRIMO byte del comando che
			 * `pin_test` scrive sul chip
			 * ("39405108 ldrb"@0xffffff8008a64fb8, poi
			 * "390003e8 strb"@0xffffff8008a64fc8 su `cmd[0]`).
			 * NOME NON NEL BINARIO: `c<offset>` (regola 5) */
	u8 c21;		/* +0x15, un byte confrontato con 1:
			 * "39405508 ldrb"@0xffffff8008a6539c +
			 * "7100051f cmp"@0xffffff8008a653a0. Comanda i due
			 * `bench_mark_*` in `create_mp_test_frame_buffer`.
			 * NOME NON NEL BINARIO */
	u8 c22;		/* +0x16 = 22, UN byte. NON e' riempimento: la chiave .ini
			 * "type option"@0xffffff8009244f28 ci finisce dentro --
			 * "39005900 strb"@0xffffff8008a5f510, un solo byte.
			 * `mp_show_result` non lo rilegge; a rileggerlo e'
			 * `ilitek_tddi_mp_init_item`, che lo azzera insieme a
			 * +0x15 con "7818537f sturh"@0xffffff8008a5e954 (DUE
			 * byte a partire da 0x15). NOME NON NEL BINARIO */
	u8 c23;		/* +0x17 = 23, UN byte. NON e' riempimento: e' la
			 * bandiera "l'elemento va eseguito", scritta da
			 * `mp_sort_item` con "381ff2a8 sturb"@0xffffff8008a5eeb8
			 * come `(katoi(valore di "enable") != 0)`
			 * ("1a9f07e8 cset"@0xffffff8008a5eeb4, condizione `ne`),
			 * riletta da "39405d04 ldrb"@0xffffff8008a5ff90 per il
			 * campo `run` del messaggio
			 * "\x016ILITEK: (%s, %d): %s: run = %d, max = %d, min = %d, frame_count = %d\n"
			 * @0xffffff8009245045 e da
			 * "385f314b ldurb"@0xffffff8008a61c3c nel conteggio
			 * finale. `mp_test_free` la azzera
			 * ("381bf29f sturb"@0xffffff8008a5f04c).
			 * IL NOME `run` VIENE DAL FORMATO, ma il formato non lo
			 * scrive come nome di CAMPO: resta `c23` (regola 5) */
	/*
	 * +0x18 = 24. NON E' RIEMPIMENTO, e il lotto F1 lo dichiarava tale
	 * dentro `c22[14]`. Nei byte di `.data` della tavola vale 1 in
	 * QUARANTAQUATTRO voci su cinquanta e 0 nelle altre sei, e non ne
	 * assume nessun altro valore -- misurato scandendo tutti e 152 i byte
	 * di ogni voce:
	 *
	 *   offset  quante voci  valori visti
	 *     +16    35/50        [1..10]
	 *     +20    36/50        [1..30, 97]
	 *     +24    44/50        [1]
	 *
	 * LE SEI CHE VALGONO ZERO SONO ESATTAMENTE LE SEI IL CUI NOME FINISCE
	 * PER "(lcm off)": gli indici 44..49, da "raw data(have bk) (lcm off)"
	 * a "peak to peak_td (lcm off)". Nessuna delle altre quarantaquattro lo
	 * ha a zero e nessuna delle sei lo ha a uno: la corrispondenza e' esatta
	 * in tutte e due le direzioni.
	 *
	 * Il SIGNIFICATO resta una lettura, non una misura: nessuna delle
	 * quattordici funzioni scritte legge questo campo, quindi il binario non
	 * mostra cosa se ne faccia. Cio' che e' misurato e' la corrispondenza col
	 * nome, e il nome viene dal binario.
	 */
	u8 c24;
	u8 c25[3];	/* +0x19..0x1b: riempimento, sempre zero nelle 50 voci */
	int c28;	/* +0x1c = 28, `int`: "b940018e ldr"@0xffffff8008a6655c
			 * lo legge a QUATTRO byte (x12 = elemento + 0x1c,
			 * "9100710c add"@0xffffff8008a66550) e il valore letto
			 * viene copiato in un array di `int`. Il SEGNO non e'
			 * misurato: nessun confronto lo prova. E' zero in tutte
			 * e 50 le voci di `.data`, quindi ci si scrive a tempo
			 * d'esecuzione. NOME NON NEL BINARIO */
	int c32;	/* +0x20 = 32, QUATTRO byte. NON e' riempimento:
			 * `ilitek_tddi_mp_init_item` lo mette a -1 insieme a
			 * `c28` con "f818c377 stur"@0xffffff8008a5e95c (otto
			 * byte a partire da 0x1c, x23 = 0xffffffff00000000 da
			 * "b2607ff7 mov"@0xffffff8008a5e8a4: la meta' bassa --
			 * `c28` -- vale 0 e quella alta -- questo campo -- -1),
			 * e `mp_test_free` lo rimette a -1 insieme a `c36` con
			 * "f81c8293 stur"@0xffffff8008a5f050.
			 * NOME NON NEL BINARIO */
	int c36;	/* +0x24, `int`: `pin_test` ci scrive 0 oppure -1
			 * ("b9002528 str"@0xffffff8008a65164, quattro byte,
			 * con "12800008 mov"@0xffffff8008a65144 = -1 su un
			 * ramo e "2a1f03e8 mov"@0xffffff8008a650e0 = 0
			 * sull'altro). NOME NON NEL BINARIO */
	int c40;	/* +0x28 = 40, `int`: "b94001b0 ldr"@0xffffff8008a66560
			 * (x13 = elemento + 0x28, "9100a10d add"@0xffffff8008a66554).
			 * E' il compagno di `c28`: i due sono copiati nello
			 * stesso ciclo, l'uno nel vettore delle soglie
			 * superiori e l'altro in quello delle inferiori.
			 * NOME NON NEL BINARIO */
	int c44;	/* +0x2c = 44, QUATTRO byte. NON e' riempimento:
			 * "b819c374 stur"@0xffffff8008a5e968 lo mette a -1
			 * all'inizializzazione e
			 * "b81d4293 stur"@0xffffff8008a5f054 lo rimette a -1 in
			 * `mp_test_free`. Nessuna funzione scritta lo LEGGE.
			 * NOME NON NEL BINARIO */
	int c48;	/* +0x30 = 48, `int`: "b8430f6a ldr"@0xffffff8008a66614
			 * (pre-indice, x27 = elemento + 48) e
			 * "7100095f cmp"@0xffffff8008a66618 lo confronta con 2
			 * con "54002c4b b.lt"@0xffffff8008a6661c, cioe' CON
			 * SEGNO. E' il numero di ripetizioni della misura: e'
			 * il fattore per cui si moltiplica `core_mp.c256` per
			 * dimensionare il vettore di lavoro
			 * ("1b0a7d09 mul"@0xffffff8008a66630) ed e' il limite
			 * del ciclo interno di ordinamento.
			 * NOME NON NEL BINARIO */
	int c52;	/* +0x34 = 52, `int`: "b940356b ldr"@0xffffff8008a66584,
			 * quattro byte, provato solo contro zero
			 * ("3400008b cbz"@0xffffff8008a66588).
			 * NOME NON NEL BINARIO */
	int c56;	/* +0x38 = 56: prima parola della coppia
			 * "29472929 ldp"@0xffffff8008a667c8 */
	int c60;	/* +0x3c = 60: seconda parola della stessa `ldp`.
			 * SONO `int` CON SEGNO: il prodotto per `c48` finisce
			 * in una divisione per 100 fatta con
			 * "9b2b7d4a smull"@0xffffff8008a667e4 +
			 * "9365fd4a asr"@0xffffff8008a667f0, cioe' la sequenza
			 * di clang per una divisione CON SEGNO */
	/*
	 * +0x40..+0x53: CINQUE `int`, aperti dal lotto F5. Il lotto F3 li
	 * dichiarava riempimento (`u8 c64[20]`). I quattro `tdf` sono letti a
	 * COPPIE da due `ldp` adiacenti -- "29481103 ldp"@0xffffff8008a5ffc4
	 * (+64,+68) e "29491905 ldp"@0xffffff8008a5ffc8 (+72,+76) -- ed e'
	 * quello che prova che sono quattro campi da quattro byte.
	 */
	int c64;	/* +0x40 = 64: "b9004100 str"@0xffffff8008a5fa74 */
	int c68;	/* +0x44 = 68: "b9004500 str"@0xffffff8008a5fc64 */
	int c72;	/* +0x48 = 72: "b9004900 str"@0xffffff8008a5fd5c */
	int c76;	/* +0x4c = 76: "b9004d00 str"@0xffffff8008a5fe70 */
	int c80;	/* +0x50 = 80: "b9005100 str"@0xffffff8008a5f72c, il
			 * valore della chiave .ini "goldenmode"
			 * @0xffffff8009244f72; e' subito confrontato con `c21`
			 * ("6b08001f cmp"@0xffffff8008a5f738) */
	/*
	 * TRE BYTE CHE IL BINARIO NOMINA -- e' l'eccezione alla regola 5, la
	 * stessa di `core_mp.key_len`: tre `printk` di `pin_test` leggono
	 * questi tre byte e li stampano con un formato che ne scrive il nome.
	 */
	/*
	 * CORREZIONE DEL LOTTO F3c-bis: le tre citazioni qui sotto portavano la
	 * CODA del messaggio all'indirizzo del messaggio INTERO. Finche'
	 * `pin_test` non era scritta lo strumento le dava NON_ANCORATA
	 * (nessun letterale con quel testo nel codice) e la cosa passava;
	 * scritta la funzione, il letterale esiste, `verificacitazioni.py`
	 * confronta i byte e le tre diventano DIVERGENTE. Sono ora scritte per
	 * intero, col prefisso "\x016ILITEK: (%s, %d): ", ed e' esattamente la
	 * stessa correzione che il lotto F3c aveva dovuto fare sulle sei
	 * "Failed to allocate <campo> mem".
	 */
	u8 delay_time;		/* +0x54 = 84: "38454ee3 ldrb"@0xffffff8008a64f88
				 * (pre-indice, x23 = elemento + 84) e il
				 * formato
				 * "\x016ILITEK: (%s, %d): delay_time = 0x%x\n"@0xffffff8009244910 */
	u8 test_int_pin;	/* +0x55 = 85: "39415723 ldrb"@0xffffff8008a64f4c
				 * e il formato
				 * "\x016ILITEK: (%s, %d): test_int_pin = 0x%x\n"@0xffffff80092448bc */
	u8 int_pulse_test;	/* +0x56 = 86: "38456f03 ldrb"@0xffffff8008a64f6c
				 * (pre-indice, x24 = elemento + 86) e il
				 * formato
				 * "\x016ILITEK: (%s, %d): int_pulse_test = 0x%x\n"@0xffffff80092448e5 */
	u8 c87;			/* +0x57: riempimento d'allineamento fino al
				 * puntatore a +88 */
	/*
	 * SEI PUNTATORI, +0x58..+0x80. I NOMI VENGONO DAL BINARIO: ognuno e'
	 * scritto da una allocazione di `create_mp_test_frame_buffer` il cui
	 * ramo d'errore stampa "Failed to allocate <nome> mem". Il passo di
	 * inferenza e' UNO SOLO -- il messaggio nomina cio' che si stava
	 * allocando, e la `str` che lo precede dice dove -- e per `max_buf` e
	 * `min_buf` c'e' anche una conferma indipendente: sono i due vettori
	 * che `compare_MaxMin_result` aggiorna con il massimo e il minimo.
	 */
	int *result_buf;	/* +0x58 = 88: "f8458e68 ldr"@0xffffff8008a652e8,
				 * "f9000260 str"@0xffffff8008a65310;
				 * "\x013ILITEK: (%s, %d): Failed to allocate result_buf mem\n"
				 * @0xffffff8009243cec */
	int *buf;		/* +0x60 = 96: "f8460ec8 ldr"@0xffffff8008a652b4,
				 * "f90002c0 str"@0xffffff8008a652d4;
				 * "\x013ILITEK: (%s, %d): Failed to allocate buf mem\n"
				 * @0xffffff8009243cbc. E' l'UNICO allocato
				 * con `vmalloc` */
	int *max_buf;		/* +0x68 = 104: "f94001e5 ldr"@0xffffff8008a658e0
				 * (otto byte), e il valore letto e' subito
				 * base di un `ldr w` scalato 4
				 * ("b86368a6 ldr"@0xffffff8008a658f0);
				 * "\x013ILITEK: (%s, %d): Failed to allocate max_buf mem\n"
				 * @0xffffff8009243d23 */
	int *min_buf;		/* +0x70 = 112: "f9400205 ldr"@0xffffff8008a6590c;
				 * "\x013ILITEK: (%s, %d): Failed to allocate min_buf mem\n"
				 * @0xffffff8009243d57 */
	int *bench_mark_max;	/* +0x78 = 120: "f8478e68 ldr"@0xffffff8008a653b0;
				 * "\x013ILITEK: (%s, %d): Failed to allocate bench_mark_max mem\n"
				 * @0xffffff8009243d8b */
	int *bench_mark_min;	/* +0x80 = 128: "f8480e68 ldr"@0xffffff8008a653c0;
				 * "\x013ILITEK: (%s, %d): Failed to allocate bench_mark_min mem\n"
				 * @0xffffff8009243dc6 */
	int *c136;	/* +0x88 = 136, OTTO byte. NON e' riempimento: e' un
			 * puntatore che `mp_test_free` libera --
			 * "f9401a80 ldr"@0xffffff8008a5f0f0,
			 * "97dfc5ae bl"@0xffffff8008a5f0f8 verso <kfree>,
			 * "f9001a9f str"@0xffffff8008a5f0fc (rimesso a NULL).
			 * IL TIPO E' ORA MISURATO (lotto F3c, era `void *`):
			 * `open_test_sp` lo alloca con
			 * "97dfb3de bl"@0xffffff8008a63410 verso <__kmalloc>
			 * dopo "d37ef500 lsl"@0xffffff8008a63408 (conteggio per
			 * QUATTRO) e "f8088f80 str"@0xffffff8008a63420 lo
			 * scrive a +136; poi lo scorre a passo 4 con
			 * "b86bd98c ldr"@0xffffff8008a63f58 e
			 * "b871d9b2 ldr"@0xffffff8008a640a4. E' un `int *`, ed
			 * e' il vettore del TIPO DI NODO che
			 * `parser_ini_nodetype` riempie
			 * ("97ffe97e bl"@0xffffff8008a634f8) */
	int (*c144)(int);
			/* +0x90, PUNTATORE A FUNZIONE:
			 * "f9404aa8 ldr"@0xffffff8008a66cd0 lo legge e
			 * "d63f0100 blr"@0xffffff8008a66cd8 lo chiama con
			 * w0 = l'indice dell'elemento
			 * ("2a1303e0 mov"@0xffffff8008a66cd4). Il tipo del
			 * ritorno NON e' provato da questo lotto: `mp_do_retry`
			 * scarta w0 subito dopo. `int` e' una SCELTA */
};

/*
 * LA TAVOLA, LETTA DAI BYTE E DALLE RILOCAZIONI -- non piu' `extern`.
 *
 * Base 0xffffff80099876d0, passo 152 ("5280130f mov"@0xffffff8008a65888 con
 * "9b2f3810 smaddl"@0xffffff8008a65890), 50 voci: la fine cade a
 * 0xffffff8009989480, che e' l'inizio della tavola del flash del gruppo G --
 * le due si toccano senza un byte di gioco, ed e' la prova che il conteggio
 * di 50 e' giusto.
 *
 * I NOMI vengono dalle 50 rilocazioni R_AARCH64_RELATIVE a passo 152, una per
 * voce sul campo a +0: nell'immagine il puntatore vale zero, e il valore vero
 * sta nell'addend. Nessun nome e' inventato.
 *
 * Dei 152 byte di ciascuna voce, SOLO TRE campi sono inizializzati oltre al
 * nome -- +16, +20 e +24 -- e tutti gli altri 148 byte sono zero in tutte e
 * cinquanta. Scritti qui per nome di campo, non per posizione.
 */
struct ilitek_mp_item mp_items[50] = {
	{ .c0 = "baseline data(bg)", .c16 = 0, .c20 = 2, .c24 = 1 },
	{ .c0 = "untouch signal data(bg-raw-4096) - mutual", .c16 = 0, .c20 = 3, .c24 = 1 },
	{ .c0 = "manual bk data(mutual)", .c16 = 0, .c20 = 16, .c24 = 1 },
	{ .c0 = "calibration data(dac) - self", .c16 = 1, .c20 = 12, .c24 = 1 },
	{ .c0 = "baselin data(bg,self_tx,self_r)", .c16 = 1, .c20 = 15, .c24 = 1 },
	{ .c0 = "untouch signal data(bg-raw-4096) - self", .c16 = 1, .c20 = 13, .c24 = 1 },
	{ .c0 = "raw data(no bk) - self", .c16 = 1, .c20 = 14, .c24 = 1 },
	{ .c0 = "raw data(have bk) - self", .c16 = 1, .c20 = 11, .c24 = 1 },
	{ .c0 = "manual bk dac data(self_tx,self_rx)", .c16 = 1, .c20 = 17, .c24 = 1 },
	{ .c0 = "calibration data(dac/icon)", .c16 = 2, .c20 = 20, .c24 = 1 },
	{ .c0 = "key baseline data", .c16 = 2, .c20 = 22, .c24 = 1 },
	{ .c0 = "key raw data", .c16 = 2, .c20 = 7, .c24 = 1 },
	{ .c0 = "key raw bk dac", .c16 = 2, .c20 = 21, .c24 = 1 },
	{ .c0 = "key raw open test", .c16 = 2, .c20 = 18, .c24 = 1 },
	{ .c0 = "key raw short test", .c16 = 2, .c20 = 19, .c24 = 1 },
	{ .c0 = "st calibration data(dac)", .c16 = 3, .c20 = 26, .c24 = 1 },
	{ .c0 = "st baseline data(bg)", .c16 = 3, .c20 = 28, .c24 = 1 },
	{ .c0 = "st raw data(no bk)", .c16 = 3, .c20 = 23, .c24 = 1 },
	{ .c0 = "st raw(have bk)", .c16 = 3, .c20 = 27, .c24 = 1 },
	{ .c0 = "st open data", .c16 = 3, .c20 = 24, .c24 = 1 },
	{ .c0 = "tx short test", .c16 = 0, .c20 = 25, .c24 = 1 },
	{ .c0 = "rx open", .c16 = 0, .c20 = 6, .c24 = 1 },
	{ .c0 = "untouch cm data", .c16 = 0, .c20 = 9, .c24 = 1 },
	{ .c0 = "untouch cs data", .c16 = 0, .c20 = 10, .c24 = 1 },
	{ .c0 = "tx/rx delta", .c16 = 4, .c20 = 30, .c24 = 1 },
	{ .c0 = "untouch peak to peak", .c16 = 5, .c20 = 3, .c24 = 1 },
	{ .c0 = "pixel raw (no bk)", .c16 = 6, .c20 = 5, .c24 = 1 },
	{ .c0 = "pixel raw (have bk)", .c16 = 6, .c20 = 8, .c24 = 1 },
	{ .c0 = "noise peak to peak(cut panel)", .c16 = 8, .c20 = 0, .c24 = 1 },
	{ .c0 = "open test(integration)", .c16 = 7, .c20 = 4, .c24 = 1 },
	{ .c0 = "open test(cap)", .c16 = 7, .c20 = 4, .c24 = 1 },
	{ .c0 = "pin test ( int and rst )", .c16 = 10, .c20 = 97, .c24 = 1 },
	{ .c0 = "noise peak to peak(with panel)", .c16 = 8, .c20 = 0, .c24 = 1 },
	{ .c0 = "noise peak to peak(ic only)", .c16 = 8, .c20 = 29, .c24 = 1 },
	{ .c0 = "open test(integration)_sp", .c16 = 7, .c20 = 0, .c24 = 1 },
	{ .c0 = "raw data(no bk)", .c16 = 0, .c20 = 5, .c24 = 1 },
	{ .c0 = "raw data(have bk)", .c16 = 0, .c20 = 8, .c24 = 1 },
	{ .c0 = "calibration data(dac)", .c16 = 0, .c20 = 1, .c24 = 1 },
	{ .c0 = "short test -ili9881", .c16 = 9, .c20 = 4, .c24 = 1 },
	{ .c0 = "short test", .c16 = 9, .c20 = 0, .c24 = 1 },
	{ .c0 = "doze raw data", .c16 = 0, .c20 = 0, .c24 = 1 },
	{ .c0 = "doze peak to peak", .c16 = 8, .c20 = 0, .c24 = 1 },
	{ .c0 = "open test_c", .c16 = 7, .c20 = 0, .c24 = 1 },
	{ .c0 = "touch deltac", .c16 = 0, .c20 = 0, .c24 = 1 },
	{ .c0 = "raw data(have bk) (lcm off)", .c16 = 0, .c20 = 0, .c24 = 0 },
	{ .c0 = "raw data(no bk) (lcm off)", .c16 = 0, .c20 = 0, .c24 = 0 },
	{ .c0 = "noise peak to peak(with panel) (lcm off)", .c16 = 8, .c20 = 0, .c24 = 0 },
	{ .c0 = "noise peak to peak(ic only) (lcm off)", .c16 = 8, .c20 = 0, .c24 = 0 },
	{ .c0 = "raw data_td (lcm off)", .c16 = 0, .c20 = 0, .c24 = 0 },
	{ .c0 = "peak to peak_td (lcm off)", .c16 = 8, .c20 = 0, .c24 = 0 },
};

/*
 * Definiti altrove (divergenza F1-D2): nel binario stanno in `.bss` e nessuna
 * funzione fuori dal gruppo F li tocca, quindi di fabbrica sono quasi
 * certamente `static` al file del collaudo MP. Qui NON possono esserlo: mai
 * scritti in questo lotto, il compilatore li proverebbe costanti a zero.
 */
/*
 * DEFINITO QUI, non piu' `extern`. Sta in `.bss`: nessuna rilocazione lo
 * tocca e nessun byte di `.data` gli appartiene, quindi la definizione e'
 * l'oggetto azzerato -- che e' esattamente cio' che il binario contiene.
 *
 * Il lotto F1 lo lasciava `extern` di proposito (regola 6: il link che
 * fallisce e' l'esito onesto). Qui viene definito, e con esso il gruppo F
 * passa da SEI simboli mancanti al link a DUE.
 *
 * Resta la divergenza F1-D6: la BASE 0xffffff800a0fcb00 e' una SCELTA. Cio'
 * che e' misurato sono gli OFFSET dei campi rispetto a quella base, e il nome
 * `core_mp`, che viene dal binario -- dal formato a 0xffffff800924477e, che
 * si legge "ILITEK: (%s, %d): core_mp.key_len = %d" col prefisso KERN_INFO.
 */
struct ilitek_core_mp core_mp;

/* 0xffffff800a0ff400: `int` con segno, il numero di voci .ini caricate.
 * "b984011b ldrsw"@0xffffff8008a61fa8 (estensione con segno) e
 * "b9440328 ldr"@0xffffff8008a5db3c. NOME SCELTO. */
int ilitek_ini_num;

/* 0xffffff800a0ff408: puntatore alla corsa delle voci.
 * "f942051c ldr"@0xffffff8008a61fb8 e "f942071b ldr"@0xffffff8008a5db58.
 * Che 0xa0ff400 e 0xa0ff408 siano due simboli distinti o due campi di una
 * struttura NON e' deciso dal binario: clang rifa' l'`adrp` per ogni offset
 * in tutti e due i casi. La misura non cambia. NOME SCELTO. */
struct ilitek_ini_item *ilitek_ini_data;

/* 0xffffff8008a6b5b8, oltre il gruppo del flash: dichiarata e NON definita
 * (regola 6). "94003660 bl"@0xffffff8008a5dc38. Prende un `char *` e torna un
 * `int`: il risultato finisce in un array di `int` con
 * "b836d900 str"@0xffffff8008a5dc48 (`str w0`, quattro byte). */
extern int katoi(char *str);

void parser_ini_nodetype(int *type_ptr, char *desp, int frame_len);
void parser_ini_benchmark(int *max_ptr, int *min_ptr, u8 pct, char *desp,
			  int frame_len);
void dump_node_type_buffer(int *ptr);

/*
 * 0xffffff8008a593c8, gruppo D: dichiarata e NON definita (regola 6).
 * `pin_test` la chiama due volte, "97ffd0d1 bl"@0xffffff8008a65084 e
 * "97ffd0bd bl"@0xffffff8008a650d4, e in tutti e due i siti NON prepara
 * nessun registro d'argomento e prova il risultato come `int` negativo
 * ("37f80300 tbnz"@0xffffff8008a65088, bit 31): quindi `int (void)`.
 * `ilitek.h` non la dichiara -- e' nel delta di header (voce 8).
 */
int ilitek_tddi_ic_check_int_stat(void);

/*
 * ===========================================================================
 * dump_benchmark_data -- 0xffffff8008a6573c, 300 byte, `t` (riga 348)
 * ===========================================================================
 * Due parametri, tutti e due `int *`: il sito di chiamata li prende in coppia
 * dalla voce di collaudo, "a94786a0 ldp"@0xffffff8008a622cc (x0 e x1 da
 * tItems[i]+120 e +128), e li ripassa tali e quali.
 *
 * I NOMI `max_ptr`/`min_ptr` SONO SCELTI, ma non a caso: il messaggio stampato
 * subito prima di ciascun ciclo li nomina --
 * "\x016ILITEK: (%s, %d): Dump Benchmark Max\n"@0xffffff8009243e01 per il
 * primo, e per il secondo la stringa KERN_CONT a 0xffffff8009243e3d.
 *
 * "Denchmark" NON e' un errore di battitura di chi scrive: e' cosi' nel
 * binario, ed e' un difetto della fabbrica che si riproduce (regola 7).
 * A 0xffffff8009243e3d ci sono i byte
 *   01 63 44 75 6d 70 20 44 65 6e 63 68 6d 61 72 6b 20 4d 69 6e 0a
 * cioe' SOH + 'c' (= KERN_CONT) seguito da "Dump Denchmark Min\n". Il
 * comando che lo stampa e' in fondo al cappello, sezione "I TRE LETTERALI
 * KERN_CONT".
 *
 * La guardia diagnostica copre TUTTO il corpo, non solo la prima `printk`:
 * "39656108 ldrb"@0xffffff8008a65758 legge `ilitek_dbg_en` una volta sola e
 * "340007a8 cbz"@0xffffff8008a6575c salta all'epilogo. Una sola lettura in
 * tutta la funzione lo prova: con due `if` distinti il compilatore dovrebbe
 * rileggere la variabile dopo la `printk` di mezzo, perche' e' un globale.
 */
static __used void dump_benchmark_data(int *max_ptr, int *min_ptr)
{
	int i;

	if (!ilitek_dbg_en)
		return;

	/* "52802b82 mov"@0xffffff8008a65778 -- __LINE__ = 348 (F1-D1) */
	/* F1-D1: `#line` riporta il `__LINE__` alla riga 348 di fabbrica */
#line 348
	ILI_INFO("Dump Benchmark Max\n");
	/* "b94c02e8 ldr"@0xffffff8008a65784 e "7100051f cmp"@0xffffff8008a65788 */
	for (i = 0; i < core_mp.c256; i++) {
		/* "b8787a81 ldr"@0xffffff8008a657a8 -- ldr w1, [x20,x24,lsl #2]:
		 * quattro byte per elemento, quindi `int *` */
		printk(KERN_CONT "%d, ", max_ptr[i]);
		/* "1ac80f09 sdiv"@0xffffff8008a657b8 e
		 * "1b08e129 msub"@0xffffff8008a657bc sono il resto CON SEGNO;
		 * "51000508 sub"@0xffffff8008a657c0 e' il -1. Il campo e'
		 * caricato una volta sola ("b94beb28 ldr"@0xffffff8008a657b4)
		 * e usato per tutti e due gli usi: e' la CSE, non due letture */
		if (i % core_mp.c232 == core_mp.c232 - 1)
			printk(KERN_CONT "\n");
	}

	/* KERN_CONT, tre argomenti in meno: "9138f400 add"@0xffffff8008a657e8
	 * carica il formato in x0 e non c'e' nessun x1/w2 -- niente `__func__`,
	 * niente `__LINE__`. Non e' ILI_INFO, e' una `printk` nuda */
	printk(KERN_CONT "Dump Denchmark Min\n");
	for (i = 0; i < core_mp.c256; i++) {
		/* "b8767a61 ldr"@0xffffff8008a65814 */
		printk(KERN_CONT "%d, ", min_ptr[i]);
		if (i % core_mp.c232 == core_mp.c232 - 1)
			printk(KERN_CONT "\n");
	}
}

/*
 * ===========================================================================
 * dump_node_type_buffer -- 0xffffff8008a5da3c, 180 byte, `T` (riga 371)
 * ===========================================================================
 * NON e' chiamata da nessuno in tutto il kernel: il compilatore l'ha emessa
 * come funzione E l'ha anche incorporata dentro `open_test_sp`, dove si
 * riconosce dal `__func__` -- "39656108 ldrb"@0xffffff8008a63500 e' la stessa
 * guardia, e da li' si stampa lo stesso formato
 * "\x016ILITEK: (%s, %d): Dump NodeType\n"@0xffffff80092430a3 con
 * __func__ = "dump_node_type_buffer"@0xffffff80092430c6. E' `T` in
 * `stock.map`, quindi non `static`, ed e' per questo che sopravvive.
 *
 * UN SOLO parametro e' MISURATO. Un secondo parametro inutilizzato sarebbe
 * invisibile qui e nel sito incorporato: non lo si puo' escludere, e non lo si
 * scrive.
 */
void dump_node_type_buffer(int *ptr)
{
	int i;

	/* "39656108 ldrb"@0xffffff8008a5da54 -- pagina+2392 = 0xffffff800a0fc958,
	 * un byte solo; "34000428 cbz"@0xffffff8008a5da58 salta all'epilogo */
	if (!ilitek_dbg_en)
		return;

	/* "52802e62 mov"@0xffffff8008a5da70 -- __LINE__ = 371 (F1-D1) */
	/* F1-D1: `#line` riporta il `__LINE__` alla riga 371 di fabbrica */
#line 371
	ILI_INFO("Dump NodeType\n");
	/* "b94c02c8 ldr"@0xffffff8008a5da7c, "7100051f cmp"@0xffffff8008a5da80,
	 * "540002cb b.lt"@0xffffff8008a5da84: la guardia `c256 < 1` */
	for (i = 0; i < core_mp.c256; i++) {
		/* "b8777a61 ldr"@0xffffff8008a5daa0 */
		printk(KERN_CONT "%d, ", ptr[i]);
		/* "b94beb08 ldr"@0xffffff8008a5daac (pagina+3048),
		 * "1ac80ee9 sdiv"@0xffffff8008a5dab0,
		 * "1b08dd29 msub"@0xffffff8008a5dab4,
		 * "51000508 sub"@0xffffff8008a5dab8,
		 * "6b08013f cmp"@0xffffff8008a5dabc */
		if (i % core_mp.c232 == core_mp.c232 - 1)
			/* "aa1503e0 mov"@0xffffff8008a5dac4 carica
			 * 0xffffff80090ddcf1, che porta i byte 01 63 0a:
			 * KERN_CONT + "\n". E' condivisa dal linker con altre
			 * 170 funzioni del kernel */
			printk(KERN_CONT "\n");
		/* "b98c02c8 ldrsw"@0xffffff8008a5dacc -- il limite e' RILETTO a
		 * ogni giro: e' un globale e la `printk` potrebbe cambiarlo */
	}
}

/*
 * ===========================================================================
 * parser_get_ini_key_value -- 0xffffff8008a61f88, 332 byte, `t` (riga 395)
 * ===========================================================================
 * Cerca la voce (sezione, chiave) e ne copia il valore. Ritorna 0 se trovata
 * ("2a1f03e0 mov"@0xffffff8008a620cc) e -2 se no
 * ("321f7be0 orr"@0xffffff8008a62048, 0xfffffffe).
 *
 * DUE dei tre nomi vengono dal binario: il formato
 * "\x016ILITEK: (%s, %d): (key: %s, value:%s) => (ini key: %s, val: %s)\n"
 * @0xffffff8009244e2a chiama "key" il suo x3 ("aa1403e3 mov"@0xffffff8008a620c0,
 * cioe' il secondo parametro) e "value" il suo x4
 * ("aa1303e4 mov"@0xffffff8008a620c4, il terzo). Il primo parametro NON e'
 * nominato: `section` e' un NOME SCELTO, giustificato solo dal fatto che viene
 * confrontato col campo +0, quello che `parser_ini_nodetype` passa a `strstr`.
 */
static __used int parser_get_ini_key_value(char *section, char *key, char *value)
{
	int i;
	int len;

	/* "b984011b ldrsw"@0xffffff8008a61fa8 e
	 * "7100077f cmp"@0xffffff8008a61fac: il conteggio e' letto UNA volta
	 * sola e non piu' riletto -- `strlen`/`strncmp` sono `readonly` per il
	 * compilatore, `memcpy` no (e infatti dopo la `memcpy` il puntatore
	 * viene riletto: "f9420508 ldr"@0xffffff8008a6209c) */
	for (i = 0; i < ilitek_ini_num; i++) {
		/* "eb0002df cmp"@0xffffff8008a61fe4 -- confronto a 64 bit fra
		 * due `size_t`. `strlen(section)` e' calcolata una volta sola
		 * fuori dal ciclo ("940fd8ce bl"@0xffffff8008a61fc8) */
		if (strlen(section) != strlen(ilitek_ini_data[i].c0))
			continue;
		/* "940fd8e3 bl"@0xffffff8008a61ff8 con x2 = strlen(section) */
		if (strncmp(section, ilitek_ini_data[i].c0, strlen(section)) != 0)
			continue;
		/* "940fd8bd bl"@0xffffff8008a6200c -- `strlen(key)` e' invece
		 * DENTRO il ciclo, e questo prova che nel sorgente sta dopo il
		 * confronto sulla sezione: fuori dal ramo non e' garantita e il
		 * compilatore non la puo' sollevare */
		if (strlen(key) != strlen(ilitek_ini_data[i].c100))
			continue;
		if (strncmp(key, ilitek_ini_data[i].c100, strlen(key)) != 0)
			continue;

		/* "5280fa08 mov"@0xffffff8008a62074 -- 0x7d0 = 2000, e
		 * "1a88b128 csel"@0xffffff8008a62080 con condizione `lt`:
		 * minimo CON SEGNO, quindi i due operandi sono `int`.
		 * "93407d02 sxtw"@0xffffff8008a62084 lo estende all'argomento
		 * di `memcpy`. QUALE forma sorgente la fabbrica avesse non e'
		 * deciso dal binario: `min_t` e il ternario danno lo stesso */
		len = min_t(int, ilitek_ini_data[i].c2208, 2000);
		memcpy(value, ilitek_ini_data[i].c200, len);
		/* "39656108 ldrb"@0xffffff8008a62090 e
		 * "340001c8 cbz"@0xffffff8008a62094: la guardia salta la
		 * costruzione di TUTTI gli argomenti, non solo la chiamata --
		 * e' la forma di ILI_DBG.
		 * "52803162 mov"@0xffffff8008a620bc -- __LINE__ = 395 (F1-D1).
		 * x5 = "91019105 add"@0xffffff8008a620b0 (+0x64) e
		 * x6 = "91032106 add"@0xffffff8008a620b4 (+0xc8) */
		/* F1-D1: `#line` riporta il `__LINE__` alla riga 395 di fabbrica */
#line 395
		ILI_DBG("(key: %s, value:%s) => (ini key: %s, val: %s)\n", key, value, ilitek_ini_data[i].c100, ilitek_ini_data[i].c200);
		return 0;
	}

	return -2;
}

/*
 * ===========================================================================
 * parser_ini_nodetype -- 0xffffff8008a5daf0, 500 byte, `T` (riga 424)
 * ===========================================================================
 * Il sito di chiamata sta in `open_test_sp`:
 * "97ffe97e bl"@0xffffff8008a634f8, con x0 = un puntatore letto da memoria,
 * x1 = "node type"@0xffffff80092430e3 ("91038c21 add"@0xffffff8008a634f4) e
 * w2 = `core_mp` pagina+3072 ("b94c02a2 ldr"@0xffffff8008a634ec), cioe' lo
 * stesso `c256` che i due `dump_*` usano come lunghezza. Da qui il terzo
 * parametro: e' un numero di nodi, e i NOMI `type_ptr`, `desp`, `frame_len`
 * SONO SCELTI (`desp` e `Frame Count` compaiono nei formati del blocco, ma
 * non in uno che nomini QUESTI parametri).
 *
 * Curiosita' misurata, non spiegata: la stessa stringa "node type" e' sia
 * l'ago che si cerca dentro il nome di SEZIONE (`strstr`, x1 = il parametro),
 * sia il nome di CHIAVE che si confronta esattamente.
 */
void parser_ini_nodetype(int *type_ptr, char *desp, int frame_len)
{
	int i, j;
	int count = 0;		/* "2a1f03f6 mov"@0xffffff8008a5db4c: azzerato
				 * PRIMA del ciclo esterno, quindi vive per
				 * tutte le sezioni */
	char str[512];		/* "321703e2 orr"@0xffffff8008a5db28: 0x200 */

	memset(str, 0, sizeof(str));

	/* "b9440328 ldr"@0xffffff8008a5db3c e "7100051f cmp"@0xffffff8008a5db40 */
	for (i = 0; i < ilitek_ini_num; i++) {
		int index;
		char pre;

		/* "94102c01 bl"@0xffffff8008a5db64 e
		 * "b40009a0 cbz"@0xffffff8008a5db68 */
		if (strstr(ilitek_ini_data[i].c0, desp) == NULL)
			continue;
		/* "f100241f cmp"@0xffffff8008a5db7c (9) e
		 * "52800122 mov"@0xffffff8008a5db88 (9): sono
		 * `strlen("node type")` ripiegata dal compilatore */
		if (strlen(ilitek_ini_data[i].c100) != strlen("node type") ||
		    strncmp(ilitek_ini_data[i].c100, "node type", strlen("node type")) != 0)
			continue;

		index = 0;
		/* "52800589 mov"@0xffffff8008a5dbb0 -- 0x2c = ',' */
		pre = ',';
		/* "37f807cb tbnz"@0xffffff8008a5dba4: se il bit 31 della
		 * lunghezza e' acceso si salta tutto -- e' la guardia di un
		 * ciclo `j <= len` con `len` con SEGNO.
		 * "54fff9ed b.le"@0xffffff8008a5dc78 chiude con `<=`, non `<` */
		for (j = 0; j <= ilitek_ini_data[i].c2208; j++) {
			/* "3943218c ldrb"@0xffffff8008a5dbc0 -- la lettura sta
			 * in CIMA, prima di tutti e due i confronti: e' quello
			 * che costringe a mettere il test sul carattere per
			 * primo nel `||` (vedi la nota nel cappello) */
			char c = ilitek_ini_data[i].c200[j];

			/* "7100ed9f cmp"@0xffffff8008a5dbcc -- 0x3b = ';';
			 * "6b0b015f cmp"@0xffffff8008a5dbc4 -- j contro len */
			if (c == ';' || j == ilitek_ini_data[i].c2208) {
				/* "12001d29 and"@0xffffff8008a5dbdc:
				 * mascheratura a 8 bit prima del confronto --
				 * `pre` e' largo un byte, non un `int`;
				 * "7100b93f cmp"@0xffffff8008a5dbe0 -- 0x2e='.' */
				if (pre != '.') {
					/* "7108015f cmp"@0xffffff8008a5dbf4 (0x200)
					 * e "1a89b148 csel"@0xffffff8008a5dc00
					 * (`lt`, con segno) */
					int len = min_t(int, j - index, 512);
					int value;

					/* la coppia memset-coda + memcpy e'
					 * quello che clang produce da
					 * `memset(str,0,512); memcpy(str,...)`:
					 * "940fe8f8 bl"@0xffffff8008a5dc20 e
					 * "940fe834 bl"@0xffffff8008a5dc30 */
					memset(str, 0, sizeof(str));
					memcpy(str, &ilitek_ini_data[i].c200[index], len);
					/* "94003660 bl"@0xffffff8008a5dc38 */
					value = katoi(str);
					/* il confronto viene DOPO la chiamata:
					 * "6b1302df cmp"@0xffffff8008a5dc3c e
					 * "5400020a b.ge"@0xffffff8008a5dc40.
					 * Clang non specula le chiamate, quindi
					 * nel sorgente `katoi` sta prima --
					 * quale forma esatta avesse la fabbrica
					 * non e' deciso dal binario */
					if (count >= frame_len) {
						/* "52803502 mov"@0xffffff8008a5dc88
						 * -- __LINE__ = 424 (F1-D1);
						 * "2a1603e3 mov"@0xffffff8008a5dc94
						 * -- w3 = count, il `%d` C'E' */
						/* "\x013ILITEK: (%s, %d): count(%d) is larger than frame length, break\n"@0xffffff80092430ed,
						 * __func__ = "parser_ini_nodetype"@0xffffff800924312f */
						/* F1-D1: `#line` riporta il `__LINE__` alla riga 424 di fabbrica */
#line 424
						ILI_ERR("count(%d) is larger than frame length, break\n", count);
						break;
					}
					/* "b836d900 str"@0xffffff8008a5dc48 --
					 * `str w0, [x8,w22,sxtw #2]`: indice
					 * esteso CON SEGNO, quattro byte */
					type_ptr[count] = value;
					count++;
				}
				index = j + 1;
				/* rilettura, non riuso: sul ramo che salta il
				 * campo il compilatore la fonde nella lettura
				 * in cima ("2a0c03e9 mov"@0xffffff8008a5dc68),
				 * su quello che converte deve rileggere dopo le
				 * chiamate ("3943210c ldrb"@0xffffff8008a5dc5c) */
				pre = ilitek_ini_data[i].c200[j];
			}
		}
	}
}

/*
 * ===========================================================================
 * parser_ini_benchmark -- 0xffffff8008a5dce4, 860 byte, `T` (riga 464)
 * ===========================================================================
 * Il sito di chiamata in `mutual_test` fissa i cinque tipi:
 * "97ffee87 bl"@0xffffff8008a622c8, preceduto da
 * "a94786a0 ldp"@0xffffff8008a622bc (x0, x1 = due `int *` da tItems[i]+120 e
 * +128), "39405aa2 ldrb"@0xffffff8008a622c4 (w2 letto UN BYTE dalla voce di
 * collaudo: il terzo parametro e' largo 8 bit -- classe A4),
 * "f94002a3 ldr"@0xffffff8008a622c0 (x3 = tItems[i]+0, cioe' il nome
 * dell'elemento) e "b94c0124 ldr"@0xffffff8008a622b8 (w4 = core_mp+3072).
 *
 * Che il terzo sia `u8` e non `bool` lo dice il corpo:
 * "72001d5f tst"@0xffffff8008a5df14 prova gli otto bit bassi (#0xff). Con un
 * `_Bool` clang proverebbe il solo bit 0.
 *
 * `pct` e' un NOME SCELTO, ma il significato e' MISURATO: quando e' diverso
 * da zero i due scarti vengono divisi per 100 e moltiplicati per il valore
 * centrale, quando e' zero vengono sommati e sottratti tali e quali.
 */
void parser_ini_benchmark(int *max_ptr, int *min_ptr, u8 pct, char *desp,
			  int frame_len)
{
	int i, j;
	int k = 0;		/* "2a1f03f4 mov"@0xffffff8008a5ddb0: fuori dal
				 * ciclo esterno, come `count` in nodetype */
	int tmp[4];
	char str[512];
	char section[256] = {0};	/* i sedici "stp xzr, xzr" da
					 * 0xffffff8008a5dd54 a
					 * 0xffffff8008a5dd94: 256 byte */

	memset(str, 0, sizeof(str));
	/* "9410320b bl"@0xffffff8008a5dd94, con
	 * x2 = "%s%s%s"@0xffffff8009179329,
	 * x4 = "_"@0xffffff80090f3fa2,
	 * x5 = "benchmark_data"@0xffffff8009243e2e,
	 * w1 = "321803e1 orr"@0xffffff8008a5dd4c (0x100 = 256) */
	snprintf(section, sizeof(section), "%s%s%s", desp, "_", "benchmark_data");

	for (i = 0; i < ilitek_ini_num; i++) {
		int index;
		char pre;

		/* "eb00037f cmp"@0xffffff8008a5dddc; la `strncmp` prende come
		 * lunghezza `strlen(rec.c0)`, non `strlen(section)`:
		 * "aa1b03e2 mov"@0xffffff8008a5ddec */
		if (strlen(ilitek_ini_data[i].c0) != strlen(section) ||
		    strncmp(ilitek_ini_data[i].c0, section, strlen(ilitek_ini_data[i].c0)) != 0)
			continue;
		/* "f100381f cmp"@0xffffff8008a5de08 (0xe = 14) e
		 * "321f0be2 orr"@0xffffff8008a5de14 (14): e'
		 * `strlen("benchmark_data")` ripiegata */
		if (strlen(ilitek_ini_data[i].c100) != strlen("benchmark_data") ||
		    strncmp(ilitek_ini_data[i].c100, "benchmark_data", strlen("benchmark_data")) != 0)
			continue;

		index = 0;
		/* "52800589 mov"@0xffffff8008a5de3c -- ',' */
		pre = ',';
		/* "37f80e4b tbnz"@0xffffff8008a5de30 e
		 * "54fff34d b.le"@0xffffff8008a5dfd8: stesso ciclo `<=` */
		for (j = 0; j <= ilitek_ini_data[i].c2208; j++) {
			/* qui i delimitatori sono TRE e non uno, e clang li
			 * riduce a una maschera di bit:
			 * "7100ed9f cmp"@0xffffff8008a5de50 (c <= 0x3b),
			 * "d2ca001c mov"@0xffffff8008a5dda8 e
			 * "f2e1001c movk"@0xffffff8008a5ddbc costruiscono
			 * 0x0800500000000000, che ha accesi esattamente i bit
			 * 44 (','), 46 ('.') e 59 (';'), e
			 * "ea1c015f tst"@0xffffff8008a5de60 lo prova */
			if (ilitek_ini_data[i].c200[j] == ',' ||
			    ilitek_ini_data[i].c200[j] == '.' ||
			    ilitek_ini_data[i].c200[j] == ';' ||
			    j == ilitek_ini_data[i].c2208) {
				/* "12001d29 and"@0xffffff8008a5de68 e
				 * "7100b93f cmp"@0xffffff8008a5de6c */
				if (pre != '.') {
					int len = min_t(int, j - index, 512);
					int row;

					memset(str, 0, sizeof(str));
					memcpy(str, &ilitek_ini_data[i].c200[index], len);
					/* "940035bd bl"@0xffffff8008a5dec4 e
					 * "b828d920 str"@0xffffff8008a5deec:
					 * la scrittura in `tmp` avviene PRIMA
					 * del controllo sul limite -- il
					 * contrario di quel che fa nodetype */
					tmp[k % 4] = katoi(str);
					/* "11000e88 add"@0xffffff8008a5decc,
					 * "1a94b108 csel"@0xffffff8008a5ded4 e
					 * "13027d03 asr"@0xffffff8008a5ded8:
					 * divisione per 4 con arrotondamento
					 * verso zero, cioe' `k` e' un `int` con
					 * segno; "121e7508 and"@0xffffff8008a5dedc
					 * e "4b080288 sub"@0xffffff8008a5dee0
					 * sono il resto */
					row = k / 4;
					if (row >= frame_len) {
						/* "52803a02 mov"@0xffffff8008a5dfe8
						 * -- __LINE__ = 464. NESSUN w3:
						 * il `%d` del formato non ha
						 * argomento. DIFETTO DELLA
						 * FABBRICA, riprodotto (F1-D5) */
						/* "\x013ILITEK: (%s, %d): count (%d) is larger than frame length, break\n"@0xffffff8009243143,
						 * __func__ = "parser_ini_benchmark"@0xffffff8009243186 */
						/* F1-D1: `#line` riporta il `__LINE__` alla riga 464 di fabbrica */
#line 464
						ILI_ERR("count (%d) is larger than frame length, break\n");
						break;
					}
					/* "71000d1f cmp"@0xffffff8008a5def4 */
					if (k % 4 == 3) {
						/* "b94123e8 ldr"@0xffffff8008a5defc
						 * (tmp[0]) e
						 * "7100051f cmp"@0xffffff8008a5df00 */
						if (tmp[0] == 1) {
							/* "72001d5f tst"@0xffffff8008a5df14 */
							if (pct) {
								/* "1b087d29 mul"@0xffffff8008a5df20,
								 * "5290a3eb mov"@0xffffff8008a5df1c +
								 * "72aa3d6b movk"@0xffffff8008a5df24
								 * (0x51eb851f) e
								 * "9365fd29 asr"@0xffffff8008a5df30
								 * (>>37): e' la divisione per 100 */
								max_ptr[row] = tmp[1] + tmp[1] * tmp[2] / 100;
								min_ptr[row] = tmp[1] - tmp[1] * tmp[3] / 100;
							} else {
								/* "0b080129 add"@0xffffff8008a5df90 e
								 * "4b0a0108 sub"@0xffffff8008a5dfa0 */
								max_ptr[row] = tmp[1] + tmp[2];
								min_ptr[row] = tmp[1] - tmp[3];
							}
						} else {
							/* "32007bea orr"@0xffffff8008a5df78
							 * -- 0x7fffffff;
							 * "320103e8 orr"@0xffffff8008a5df84
							 * -- 0x80000000 */
							max_ptr[row] = INT_MAX;
							min_ptr[row] = INT_MIN;
						}
					}
					k++;
				}
				index = j + 1;
				/* "3943210c ldrb"@0xffffff8008a5dfbc e
				 * "2a0c03e9 mov"@0xffffff8008a5dfc8 */
				pre = ilitek_ini_data[i].c200[j];
			}
		}
	}
}

/*
 * ===========================================================================
 * parser_get_u8_array -- 0xffffff8008a659d0, 292 byte, `t` (righe 532 e 549)
 * ===========================================================================
 * Il sito di chiamata sta in `ilitek_tddi_mp_test_main`:
 * "94001b16 bl"@0xffffff8008a5ed78. Il secondo parametro e' un `u8 *` --
 * "38346a68 strb"@0xffffff8008a65adc scrive UN byte per elemento -- e il terzo
 * un `int` con segno, esteso con "93407c58 sxtw"@0xffffff8008a65a90.
 *
 * Il valore di ritorno e' il CONTATORE del ciclo, non il numero di conversioni
 * riuscite: "2a1403e0 mov"@0xffffff8008a65a64 restituisce w20, che avanza a
 * ogni giro ("91000694 add"@0xffffff8008a65ae0) anche quando `kstrtoll`
 * fallisce e si stampa l'errore. E' un comportamento della fabbrica, non un
 * refuso: si riproduce.
 *
 * SOSPETTO DICHIARATO E NON CHIUSO (2026-08-21). Il locale che raccoglie il
 * risultato di `strsep` si chiamava `pToken`: era l'unico identificatore in
 * stile ungherese/camelCase di tutto il file (ogni altro locale e'
 * lower_snake: `s`, `lval`, `ret`, `str`, `pre`, `index`, `count`, `row`,
 * `tmp`), e una revisione indipendente ha fatto notare che il driver Ilitek
 * TDDI che circola pubblicamente usa quel nome nello stesso punto. Il nome
 * NON viene dal binario -- `strings -a oracolo/stock.elf | grep -c pToken`
 * da' 0, e i nomi dei locali non finiscono nel codice emesso -- quindi non e'
 * derivato da una misura. E' stato rinominato in `tok`, che e' la convenzione
 * del resto del file; il cambio non tocca un byte emesso (verificato: `.text`
 * identico prima e dopo). Il sospetto in se' NON si puo' chiudere da questa
 * parte: non ho modo di provare che il sorgente pubblico non sia stato
 * aperto, e resta agli atti che il cappello di questo file afferma "Nessun
 * sorgente pubblico e' stato letto". Nessun altro identificatore del file ha
 * quella forma.
 */
static __used int parser_get_u8_array(char *key, u8 *buf, int len)
{
	char *s = key;
	char *tok;
	long long lval = 0;	/* "a90083ff stp"@0xffffff8008a659f8 azzera lo
				 * slot a [sp,#8], che
				 * "910023e2 add"@0xffffff8008a65ab0 passa a
				 * `kstrtoll`: otto byte, quindi `long long` */
	int ret;
	int i;

	/* "7100045f cmp"@0xffffff8008a659f0 (len contro 1) e
	 * "39400008 ldrb"@0xffffff8008a65a00 + "34000188 cbz"@0xffffff8008a65a04
	 * (`strlen(key) == 0` ripiegata in `key[0] == 0`).
	 * "52804282 mov"@0xffffff8008a65a44 -- __LINE__ = 532 (F1-D1);
	 * "12800014 mov"@0xffffff8008a65a4c -- il ritorno e' -1, non -EINVAL
	 *
	 * F1-D3 E' CHIUSA, e la correzione e' L'ORDINE DEI DUE TERMINI DI
	 * QUESTA `||`. Il lotto F1 scriveva `len < 1 || strlen(key) == 0` e
	 * dichiarava una divergenza aperta: 73 istruzioni su 73, stessa
	 * dimensione, ma 31 identiche su 73 e undici posizioni di «ordine dei
	 * blocchi». Scritta come qui -- `strlen(key) == 0` PRIMA e `len < 1`
	 * DOPO -- la funzione diventa identica alla fabbrica: 73 mnemonici su
	 * 73 e RESIDUO ZERO sulle codifiche (19 differenze, tutte e 19 siti di
	 * rilocazione). Il codice EMESSO non cambia d'ordine perche' il
	 * compilatore riordina i due controlli a suo piacimento -- `strlen` e'
	 * `readonly` e non ha effetti collaterali; cio' che cambia e' la forma
	 * intermedia, ed e' quella che il confronto per codifica misura.
	 *
	 * LA CONSEGUENZA STA ALTROVE, E QUESTA E' LA PARTE CHE CONTA. Con la
	 * forma di F1 il costo che clang attribuisce all'INCORPORAZIONE di
	 * questa funzione dentro `allnode_open_cdc_data` e' 220, sotto la
	 * soglia di 225, e clang la incorpora: `allnode_open_cdc_data` misura
	 * 1360 byte invece di 1176. Con la forma di qui il costo e' 230, sopra
	 * la soglia, la chiamata resta una `bl` come di fabbrica e
	 * `allnode_open_cdc_data` misura esattamente 1176. I due numeri sono
	 * letti dal compilatore stesso, con `-Rpass=inline`, e stanno nel
	 * rapporto del lotto F3a.
	 */
	if (strlen(key) == 0 || len < 1) {
		/* "\x013ILITEK: (%s, %d): Can't find any characters inside buffer\n"@0xffffff800924409a,
		 * __func__ = "parser_get_u8_array"@0xffffff80092440d7 */
		/* F1-D1: `#line` riporta il `__LINE__` alla riga 532 di fabbrica */
#line 532
		ILI_ERR("Can't find any characters inside buffer\n");
		return -1;
	}

	/* "7100811f cmp"@0xffffff8008a65a0c (0x20) piu'
	 * "d286e009 mov"@0xffffff8008a65a1c e "f2c00029 movk"@0xffffff8008a65a20:
	 * la maschera e' 0x0000000100003700, cioe' i bit 8, 9, 10, 12, 13 e 32.
	 * Sono '\b', '\t', '\n', '\f', '\r' e ' ': NON e' l'insieme di
	 * `isspace` del C -- manca '\v' (11) e c'e' '\b' (8) -- ed e' per
	 * questo che qui c'e' la catena esplicita e non una macro.
	 * "2a1f03f4 mov"@0xffffff8008a65a2c: il ritorno e' 0, non un errore.
	 * (la divergenza F1-D3 -- la posizione di questo blocco rispetto alla
	 * `cbz` qui sopra -- e' CHIUSA dal lotto F3a: vedi la nota sull'ordine
	 * dei due termini della `||`) */
	if (key[0] == ' ' || key[0] == '\t' || key[0] == '\n' ||
	    key[0] == '\r' || key[0] == '\f' || key[0] == '\b')
		return 0;

	for (i = 0; i < len; i++) {
		/* "94100b8b bl"@0xffffff8008a65aa8, con x1 =
		 * ","@0xffffff8009245907 e x0 = &s ([sp,#0x10]) */
		tok = strsep(&s, ",");
		/* "b4fffd20 cbz"@0xffffff8008a65aac */
		if (tok == NULL)
			break;
		/* "321c03e1 orr"@0xffffff8008a65ab4 -- base 0x10 = 16;
		 * "97e779fb bl"@0xffffff8008a65ab8 */
		ret = kstrtoll(tok, 16, &lval);
		/* "340000e0 cbz"@0xffffff8008a65abc */
		if (ret)
			/* "528044a2 mov"@0xffffff8008a65ac4 -- __LINE__ = 549;
			 * "2a0003e3 mov"@0xffffff8008a65ac0 -- w3 = ret */
			/* "\x016ILITEK: (%s, %d): convert string too long, ret = %d\n"@0xffffff80092440eb */
			/* F1-D1: `#line` riporta il `__LINE__` alla riga 549 di fabbrica */
#line 549
			ILI_INFO("convert string too long, ret = %d\n", ret);
		else
			/* "f94007e8 ldr"@0xffffff8008a65ad8 (otto byte) e
			 * "38346a68 strb"@0xffffff8008a65adc (uno): il
			 * troncamento e' della fabbrica */
			buf[i] = lval;
	}

	return i;
}

/*
 * ===========================================================================
 * I DUE VETTORI DI SCOSTAMENTO -- 0xffffff8009245a9c e 0xffffff8009245abc
 * ===========================================================================
 * Due corse di otto `int` contigue in `.rodata`, in fondo al blocco di
 * `.rodata` del gruppo F (il byte dopo la seconda, 0xffffff8009245adc, e' gia'
 * la prima stringa del gruppo successivo). Uscita LETTERALE di objdump:
 *
 *   $OBJ -s -j .kernel --start-address=0xffffff8009245a90 \
 *        --stop-address=0xffffff8009245ae0 oracolo/stock.elf
 *
 *    ffffff8009245a90 705f7465 73745f66 72656500 ffffffff  p_test_free.....
 *    ffffff8009245aa0 00000000 01000000 ffffffff 01000000  ................
 *    ffffff8009245ab0 ffffffff 00000000 01000000 ffffffff  ................
 *    ffffff8009245ac0 ffffffff ffffffff 00000000 00000000  ................
 *    ffffff8009245ad0 01000000 01000000 01000000 0133494c  .............3IL
 *
 * cioe' {-1, 0, 1, -1, 1, -1, 0, 1} a 0xffffff8009245a9c e
 * {-1, -1, -1, 0, 0, 1, 1, 1} a 0xffffff8009245abc.
 *
 * Che stiano in `.rodata` e non sulla pila e' MISURATO: `compare_charge` non
 * ha nessun prologo -- la sua prima istruzione e'
 * "d000b4e9 adrp"@0xffffff8008a5e040, non uno `stp` -- quindi non c'e' nessuna
 * copia locale e i due vettori sono oggetti statici.
 *
 * I NOMI SONO SCELTI. Che le due corse siano gli scostamenti degli otto
 * vicini di un nodo e' un'INTERPRETAZIONE (le coppie (dx,dy) sono le otto
 * combinazioni non nulle di {-1,0,1}); i VALORI sono misurati.
 */
static const int mp_nb_dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
static const int mp_nb_dy[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

/*
 * ===========================================================================
 * compare_charge -- 0xffffff8008a5e040, 192 byte, `T` (nessuna printk)
 * ===========================================================================
 * NON E' CHIAMATA DA NESSUNO in tutto il kernel: la ricognizione l'ha cercata
 * come destinazione di `bl`/`b`, come coppia `adrp`+`add` e come addend di
 * rilocazione, e non compare mai. E' codice emesso e morto. Deve percio'
 * restare NON `static`: dichiararla `static` la farebbe sparire e il confronto
 * non tornerebbe (questa NON e' una misura sul binario ma una proprieta' del
 * compilatore, ed e' verificabile cambiando la riga e rimisurando).
 *
 * Senza `printk` non c'e' nessun numero di riga e nessun nome: la funzione e'
 * collocata qui solo per adiacenza di indirizzo, e i nomi dei parametri sono
 * SCELTI.
 *
 * I SETTE PARAMETRI, e come il corpo ne decide il tipo:
 *   x0  `int *`   -- "b86ad800 ldr"@0xffffff8008a5e050: carico a 4 byte con
 *                    indice esteso CON SEGNO (`sxtw`), quindi array di `int`
 *                    indicizzato da un `int`;
 *   w1  `int`     -- "2b0101ce adds"@0xffffff8008a5e098 somma lo scostamento e
 *                    "54000264 b.mi"@0xffffff8008a5e09c prova il risultato
 *                    NEGATIVO: con segno;
 *   w2  `int`     -- stessa prova, "37f801cf tbnz"@0xffffff8008a5e0b0 (bit 31);
 *   x3  `int *`   -- "b86ed86f ldr"@0xffffff8008a5e0c0, 4 byte, indice `sxtw`;
 *   w4, w5, w6 `int` -- "6b0401df cmp"@0xffffff8008a5e0e0 e
 *                    "540000ac b.gt"@0xffffff8008a5e0e4: confronto CON SEGNO.
 *
 * L'INDICE: "1b02052a madd"@0xffffff8008a5e04c calcola `w9*w2 + w1` con
 * w9 = `core_mp.c232` ("b94be929 ldr"@0xffffff8008a5e044, 0xffffff800a0fcbe8).
 * Nel ciclo, "1b0939ee madd"@0xffffff8008a5e0bc rifa' lo stesso conto sui
 * vicini: percio' `c232` e' il passo di riga, w1 la colonna e w2 la riga.
 * Il limite della RIGA e' un campo DIVERSO, `core_mp.c236`
 * ("b94bed6b ldr"@0xffffff8008a5e078, 0xffffff800a0fcbec): la colonna si
 * confronta con c232 ("6b0901df cmp"@0xffffff8008a5e0a0) e la riga con c236
 * ("6b0b01ff cmp"@0xffffff8008a5e0b4).
 *
 * IL VALORE DI RITORNO, letto dalle due uscite:
 *   - "d65f03c0 ret"@0xffffff8008a5e0f4 torna w0, che dal
 *     "b86ad800 ldr"@0xffffff8008a5e050 in poi non e' piu' toccato: e' il
 *     valore del nodo. Ci arrivano il ramo `x0[idx] == 0`
 *     ("34000500 cbz"@0xffffff8008a5e054), il ramo "nessuno dei tre bit"
 *     ("3610042a tbz"@0xffffff8008a5e070) e la fine del ciclo;
 *   - "d65f03c0 ret"@0xffffff8008a5e0fc torna w4, cioe' la SOGLIA scelta
 *     ("2a0403e0 mov"@0xffffff8008a5e0f8).
 * Che la funzione torni la soglia e non un codice d'esito e' quello che il
 * binario fa: non e' stato "sistemato".
 *
 * LA CATENA SUI TRE BIT: "370000aa tbnz"@0xffffff8008a5e060 (bit 0, tiene w4),
 * "2a0503e4 mov"@0xffffff8008a5e064 (w4 = w5), "3708006a
 * tbnz"@0xffffff8008a5e068 (bit 1), "2a0603e4 mov"@0xffffff8008a5e06c
 * (w4 = w6), "3610042a tbz"@0xffffff8008a5e070 (bit 2 spento: esce).
 *
 * IL CICLO: otto giri, "f100215f cmp"@0xffffff8008a5e0ec contro 8 e
 * "54fffd03 b.cc"@0xffffff8008a5e0f0 (confronto SENZA segno su registro a 64
 * bit: e' l'allargamento che clang fa di un `int` che ha provato non negativo,
 * non un indice `unsigned` nel sorgente -- il sorgente lo dichiara `int`).
 *
 * LE DUE PROVE SUL VICINO: "3400012f cbz"@0xffffff8008a5e0c4 (tipo nullo) e
 * "121d05ef and"@0xffffff8008a5e0c8 con
 * "350000ef cbnz"@0xffffff8008a5e0cc (maschera 0x18, cioe' i bit 3 e 4).
 *
 * QUATTRO BYTE HANNO DETTO DOVE STA UNA RIGA (scoperta F2-S1). La prima
 * stesura calcolava `ny` DOPO la prova su `nx`, cioe' nel punto in cui il
 * valore serve, e misurava 188 su 192: mancava esattamente
 * "d37ef54f lsl"@0xffffff8008a5e090. Con `ny` calcolato INSIEME a `nx`, in
 * cima al corpo del ciclo, la funzione misura 192 e le 48 codifiche
 * combaciano tutte tranne le otto di rilocazione. La ragione e' visibile nel
 * binario: di fabbrica il secondo carico e'
 * "b86f69af ldr"@0xffffff8008a5e0a8, cioe' `ldr w15, [x13, x15]`, e scrive
 * nello STESSO registro che porta l'indice scalato -- percio' l'indice va
 * materializzato prima, e costa una `lsl`. Calcolando `ny` sul posto, clang
 * ha un registro libero, ripiega la scala nell'indirizzo
 * (`ldr w15, [x13, x10, lsl #2]`) e la `lsl` sparisce. La differenza di
 * quattro byte e' quindi una misura sull'ORDINE delle righe del sorgente, ed
 * e' la stessa classe di difetto A2 (un valore estratto o no rispetto al
 * ciclo) vista al contrario.
 */
int compare_charge(int *p, int x, int y, int *type, int t0, int t1, int t2)
{
	int idx = core_mp.c232 * y + x;
	int ret = p[idx];
	int thr;
	int i;

	/* "34000500 cbz"@0xffffff8008a5e054 */
	if (ret == 0)
		return ret;

	/* "b86a786a ldr"@0xffffff8008a5e05c -- una sola lettura di `type[idx]`,
	 * e i tre `tbnz`/`tbz` provano i bit dello stesso registro */
	if (type[idx] & 0x01)
		thr = t0;
	else if (type[idx] & 0x02)
		thr = t1;
	else if (type[idx] & 0x04)
		thr = t2;
	else
		return ret;

	for (i = 0; i < 8; i++) {
		int nx = mp_nb_dx[i] + x;
		int ny = mp_nb_dy[i] + y;
		int nidx;

		/* "2b0101ce adds"@0xffffff8008a5e098 +
		 * "54000264 b.mi"@0xffffff8008a5e09c */
		if (nx < 0 || nx >= core_mp.c232)
			continue;
		/* "37f801cf tbnz"@0xffffff8008a5e0b0 (bit 31) */
		if (ny < 0 || ny >= core_mp.c236)
			continue;
		nidx = core_mp.c232 * ny + nx;
		if (type[nidx] == 0)
			continue;
		if (type[nidx] & 0x18)
			continue;
		/* "3400008e cbz"@0xffffff8008a5e0d8 */
		if (p[nidx] == 0)
			continue;
		/* "4b0001ce sub"@0xffffff8008a5e0dc */
		if (p[nidx] - ret > thr)
			return thr;
	}

	return ret;
}

/*
 * ===========================================================================
 * full_open_rate_compare -- 0xffffff8008a5e100, 80 byte, `T` (nessuna printk)
 * ===========================================================================
 * Anche questa NON E' CHIAMATA DA NESSUNO, per le stesse tre ricerche: resta
 * non `static` per la stessa ragione di `compare_charge`.
 *
 * SEI PARAMETRI: x0 e x1 sono due array di `int` letti allo STESSO indice
 * ("b8686829 ldr"@0xffffff8008a5e124 e "b8686808 ldr"@0xffffff8008a5e128,
 * tutti e due con l'offset x8 calcolato una volta sola da
 * "937e7d08 sbfiz"@0xffffff8008a5e120 -- estensione CON SEGNO poi scalata di
 * 4). w2 e w3 sono colonna e riga ("1b030908 madd"@0xffffff8008a5e11c,
 * `c232*w3 + w2`, con "b94be908 ldr"@0xffffff8008a5e110 =
 * `core_mp.c232`). w4 e' un insieme di bit, w5 un moltiplicatore.
 *
 * LA DIVISIONE PER 100 E' MISURATA, non scelta: "5290a3ea
 * mov"@0xffffff8008a5e114 e "72aa3d6a movk"@0xffffff8008a5e118 formano
 * w10 = 0x51eb851f; poi "9b2a7d29 smull"@0xffffff8008a5e130,
 * "d37ffd2a lsr"@0xffffff8008a5e134 (bit di segno),
 * "9365fd29 asr"@0xffffff8008a5e138 (37) e
 * "0b0a0129 add"@0xffffff8008a5e13c. E' la forma con cui clang emette una
 * divisione CON SEGNO per 100: 0x51eb851f * 2^-37 = 1/100.
 *
 * IL VALORE DI RITORNO: "320003e8 orr"@0xffffff8008a5e100 mette w8 = 1 PRIMA
 * di ogni prova, "34000224 cbz"@0xffffff8008a5e104 e
 * "37180204 tbnz"@0xffffff8008a5e108 (bit 3) saltano all'uscita comune
 * "2a0803e0 mov"@0xffffff8008a5e148, e "1a9fb7e8 cset"@0xffffff8008a5e144
 * mette w8 = (w8 >= w9), cioe' `ge`: quindi il caso normale torna
 * `p1[idx] >= p2[idx] * rate / 100`.
 */
int full_open_rate_compare(int *p1, int *p2, int x, int y, int type, int rate)
{
	int ret = 1;
	int idx;

	/* "34000224 cbz"@0xffffff8008a5e104 e
	 * "37180204 tbnz"@0xffffff8008a5e108 */
	if (type == 0 || (type & 0x08))
		return ret;

	idx = core_mp.c232 * y + x;
	/* "6b09011f cmp"@0xffffff8008a5e140 + "1a9fb7e8 cset"@0xffffff8008a5e144 */
	if (p1[idx] < (p2[idx] * rate) / 100)
		ret = 0;

	return ret;
}

/*
 * ===========================================================================
 * check_int_level -- 0xffffff8008a5e528, 240 byte, `T` (righe 2008..2026)
 * ===========================================================================
 * IL PARAMETRO E' UN `bool`, non un `int`: "36000073 tbz"@0xffffff8008a5e578
 * e "7200027f tst"@0xffffff8008a5e5a8 provano il solo BIT 0. Un `int` usato
 * come condizione darebbe `cbz`/`cbnz` sull'intero registro. I due siti di
 * chiamata, tutti e due dentro `pin_test`, passano proprio 1 e 0:
 * "320003e0 orr"@0xffffff8008a64fe4 e "2a1f03e0 mov"@0xffffff8008a6500c.
 *
 * IL RITORNO E' UN `int` CON SEGNO: i due chiamanti lo provano con
 * "37f807e0 tbnz"@0xffffff8008a64fec e "37f80680 tbnz"@0xffffff8008a65018,
 * cioe' il bit 31. Vale -1 ("12800000 mov"@0xffffff8008a5e5c8) o 0
 * ("2a1f03e0 mov"@0xffffff8008a5e600).
 *
 * IL PIEDINO E' IL 66, ED E' UNA COSTANTE NEL BINARIO:
 * "52800840 mov"@0xffffff8008a5e554 mette w0 = 0x42 = 66 e subito dopo
 * "97e85235 bl"@0xffffff8008a5e558 chiama `gpio_to_desc`, seguita da
 * "97e8594d bl"@0xffffff8008a5e55c che chiama `gpiod_get_raw_value`. Le due
 * chiamate in fila, la seconda senza toccare x0, sono l'espansione di
 * `__gpio_get_value` (asm-generic/gpio.h), cioe' di `gpio_get_value`. Il
 * numero 66 e' scritto qui come numero perche' nel binario e' un immediato:
 * se di fabbrica era una macro, il nome della macro non e' recuperabile.
 *
 * IL CICLO E' DI 500 GIRI, e il conto e' MISURATO: "12803e77
 * mov"@0xffffff8008a5e548 mette w23 = -500, "310006f7 adds"@0xffffff8008a5e590
 * incrementa di 1 e "54fffe03 b.cc"@0xffffff8008a5e594 rientra finche' la
 * somma non trabocca, cioe' finche' w23 non arriva a 0.
 *
 * TRE ISTRUZIONI, NON QUATTRO -- e questo dice COME e' scritto il ciclo
 * (scoperta F2-S2). Con `for (i = 0; i < 500; i++)` clang emette QUATTRO
 * istruzioni di controllo (`mov w23, wzr`, `add w23, w23, #1`,
 * `cmp w23, #0x1f4`, `b.cc`) e la funzione misura 244 su 240. Con
 * `while (timer--)`, e `timer` inizializzato a 500, ne emette TRE -- l'unica
 * forma in cui la `cmp` sparisce, perche' la condizione d'uscita diventa il
 * traboccamento della `adds` -- e la funzione misura 240 con tutte e sessanta
 * le codifiche uguali tranne le ventisei di rilocazione. Il post-decremento
 * NON e' una scelta di stile: e' misurato, e vale quattro byte.
 *
 * L'ATTESA E' `udelay(2)`, ED E' DEDOTTA DALLA MOLTIPLICAZIONE DELLA MACRO:
 * "528431c0 mov"@0xffffff8008a5e588 mette w0 = 0x218e = 8590 e
 * "940fe53a bl"@0xffffff8008a5e58c chiama `__const_udelay`. La macro
 * `udelay(n)` di asm-generic/delay.h, quando n e' costante, chiama
 * `__const_udelay(n * 0x10c7)`; 8590 / 0x10c7 = 8590 / 4295 = 2 esatti.
 *
 * `sucess` NON E' UN ERRORE DI CHI SCRIVE: e' cosi' nel binario, in tutte e
 * due le stringhe, ed e' un difetto della fabbrica che si riproduce
 * (regola 7). Anche lo spazio prima di `\n` in tutte e tre le stringhe e'
 * della fabbrica.
 *
 * LA SCELTA FRA "High" E "Low" E' UNA `csel`, non due rami:
 * "9a881123 csel"@0xffffff8008a5e5b4 con
 * "7200027f tst"@0xffffff8008a5e5a8 -- x9 e' "High"@0xffffff8009243279 e x8
 * e' "Low"@0xffffff800924327e, e la condizione `ne` sceglie x9. Nel sorgente
 * e' l'operatore ternario dentro la lista degli argomenti.
 */
int check_int_level(bool high)
{
	int timer = 500;
	int ret;

	while (timer--) {
		ret = gpio_get_value(66);
		/* "\x016ILITEK: (%s, %d): int GPIO level = %d\n"@0xffffff80092431c1,
		 * __func__ = "check_int_level"@0xffffff80092431ea;
		 * "5280fb02 mov"@0xffffff8008a5e564 -- __LINE__ = 2008 */
		/* F1-D1: `#line` riporta il `__LINE__` alla riga 2008 di fabbrica */
#line 2008
		ILI_INFO("int GPIO level = %d\n", ret);

		if (high) {
			if (ret) {
				/* "\x016ILITEK: (%s, %d): check int high sucess \n"@0xffffff80092431fa;
				 * "5280fb62 mov"@0xffffff8008a5e5e0 -- __LINE__ = 2011 */
#line 2011
				ILI_INFO("check int high sucess \n");
				return 0;
			}
		} else {
			if (!ret) {
				/* "\x016ILITEK: (%s, %d): check int low sucess \n"@0xffffff8009243226;
				 * "5280fc22 mov"@0xffffff8008a5e5f8 -- __LINE__ = 2017 */
#line 2017
				ILI_INFO("check int low sucess \n");
				return 0;
			}
		}

		/* "528431c0 mov"@0xffffff8008a5e588 = 8590 = 2 * 0x10c7 */
		udelay(2);
		/* il decremento sta nella condizione: vedi la scoperta F2-S2
		 * nel cappello di questa funzione */
	}

	/* "\x016ILITEK: (%s, %d): check int %s fail \n"@0xffffff8009243251;
	 * "5280fd42 mov"@0xffffff8008a5e5c0 -- __LINE__ = 2026 */
#line 2026
	ILI_INFO("check int %s fail \n", high ? "High" : "Low");
	return -1;
}

/*
 * ===========================================================================
 * compare_MaxMin_result -- 0xffffff8008a65868, 360 byte, `t` (nessuna printk)
 * ===========================================================================
 * Nessun messaggio, quindi nessun numero di riga e nessun nome: i nomi dei due
 * parametri sono SCELTI. Il primo e' l'INDICE di un elemento della tavola
 * ("9b2f3810 smaddl"@0xffffff8008a65890, w0 * 152 + base), il secondo un
 * `int *` ("b8636823 ldr"@0xffffff8008a658f4, quattro byte, indice scalato 4).
 *
 * I DUE CICLI: quello esterno conta fino a `core_mp.c236`
 * ("b94bed02 ldr"@0xffffff8008a6586c e "6b02013f cmp"@0xffffff8008a659c4),
 * quello interno fino a `core_mp.c232` ("b94be943 ldr"@0xffffff8008a65880 e
 * "6b03005f cmp"@0xffffff8008a659b4). E' la stessa coppia riga/colonna di
 * `compare_charge`, e la conferma sta nell'indice:
 * "1b030924 madd"@0xffffff8008a658e4 calcola `esterno * c232 + interno`.
 *
 * I DUE GLOBALI SONO RILETTI A OGNI GIRO -- e non e' un dettaglio di stile.
 * "b94be943 ldr"@0xffffff8008a659ac rilegge `core_mp.c232` in fondo al ciclo
 * interno e "b94bed02 ldr"@0xffffff8008a659bc rilegge `core_mp.c236` in fondo
 * a quello esterno. Il compilatore DEVE rileggerli perche' il corpo scrive in
 * memoria attraverso puntatori che potrebbero aliasare i due `int`: scriverli
 * nel sorgente come guardia del `for` (invece di estrarli in un locale) e' la
 * forma che riproduce il binario, ed e' la classe A2 al contrario.
 *
 * IL CASO 5 ESCE DALLA FUNZIONE, NON DAL CICLO:
 * "54000780 b.eq"@0xffffff8008a658dc salta a
 * "d65f03c0 ret"@0xffffff8008a659cc, che e' la stessa istruzione di ritorno
 * usata dal ramo `core_mp.c236 < 1` ("54000acb b.lt"@0xffffff8008a65874).
 *
 * L'ASIMMETRIA DEL CASO 4 E' DELLA FABBRICA, e si riproduce (regola 7): le
 * quattro prove leggono `core_mp.tx_delta_buf` e `core_mp.rx_delta_buf` ma le quattro
 * scritture vanno in QUATTRO ALTRI vettori (tx_max_buf, tx_min_buf, rx_max_buf, rx_min_buf). Che non
 * sia una lettura sbagliata lo prova il codice stesso: dopo la scrittura in
 * tx_max_buf il compilatore RILEGGE sia il puntatore tx_delta_buf
 * ("f9400184 ldr"@0xffffff8008a65950) sia tx_delta_buf[idx]
 * ("b8666885 ldr"@0xffffff8008a65954) sia buf[idx]
 * ("b8666824 ldr"@0xffffff8008a65958) -- cioe' non riesce a escludere che la
 * scrittura in tx_max_buf abbia toccato tx_delta_buf, il che ha senso solo se sono due
 * oggetti diversi. Il caso `default`, invece, e' il massimo/minimo simmetrico
 * che ci si aspetta: legge e scrive lo STESSO vettore.
 *
 * OTTO BYTE HANNO DETTO DOVE STA UN'ALTRA RIGA (scoperta F2-S3). Scrivendo
 * `int idx = i * core_mp.c232 + j;` UNA volta, in cima al corpo del ciclo
 * interno, la funzione misura 352 su 360: clang emette una sola `madd`, e per
 * di piu' fonde la `str` finale del ramo `default` con quella del caso 4,
 * risparmiando una seconda istruzione. Di fabbrica le `madd` sono DUE --
 * "1b030924 madd"@0xffffff8008a658e4 nel ramo `default` e
 * "1b030923 madd"@0xffffff8008a65928 nel caso 4 -- e le due `str` NON sono
 * fuse, come si vede dai registri: il `default` fa
 * "b82478a3 str"@0xffffff8008a6591c (valore in w3, indice in x4) e il caso 4
 * fa "b82378a4 str"@0xffffff8008a659a8 (valore in w4, indice in x3), scambiati.
 * Calcolando l'indice DENTRO ciascun ramo la funzione misura 360 e tutte e
 * novanta le codifiche combaciano tranne le venti di rilocazione. Anche qui
 * la differenza di dimensione ha misurato la posizione di una riga.
 *
 * `static`: nella mappa e' `t`. Al lotto F2 non era chiamata da nessuna
 * funzione scritta, e da li' viene il `__used`. AGGIORNAMENTO DEL LOTTO
 * F3c-bis: adesso la chiamano `open_test_sp`, `open_test_cap` e `key_test`,
 * quindi il `__used` non serve piu' a tenerla viva; e' rimasto perche' su una
 * funzione che E' usata l'attributo non cambia un byte del codice emesso.
 */
static __used void compare_MaxMin_result(int index, int *buf)
{
	int i;
	int j;

	/* "7100045f cmp"@0xffffff8008a65870 + "54000acb b.lt"@0xffffff8008a65874 */
	for (i = 0; i < core_mp.c236; i++) {
		/* "7100047f cmp"@0xffffff8008a658c0 +
		 * "540007eb b.lt"@0xffffff8008a658c4 */
		for (j = 0; j < core_mp.c232; j++) {
			int idx;

			/* "b94001c4 ldr"@0xffffff8008a658cc: il campo e'
			 * riletto a ogni giro, per la stessa ragione di alias */
			switch (mp_items[index].c16) {
			case 4:
				idx = i * core_mp.c232 + j;
				/* "6b0400bf cmp"@0xffffff8008a6593c +
				 * "540000ea b.ge"@0xffffff8008a65940 */
				if (core_mp.tx_delta_buf[idx] < buf[idx])
					core_mp.tx_max_buf[idx] = buf[idx];
				/* "6b0400bf cmp"@0xffffff8008a6595c +
				 * "540000ad b.le"@0xffffff8008a65960 */
				if (core_mp.tx_delta_buf[idx] > buf[idx])
					core_mp.tx_min_buf[idx] = buf[idx];
				/* "6b0400bf cmp"@0xffffff8008a6597c +
				 * "540000ea b.ge"@0xffffff8008a65980 */
				if (core_mp.rx_delta_buf[idx] < buf[idx])
					core_mp.rx_max_buf[idx] = buf[idx];
				/* "6b0400bf cmp"@0xffffff8008a6599c +
				 * "5400006d b.le"@0xffffff8008a659a0 */
				if (core_mp.rx_delta_buf[idx] > buf[idx])
					core_mp.rx_min_buf[idx] = buf[idx];
				break;
			case 5:
				return;
			default:
				idx = i * core_mp.c232 + j;
				/* "6b0300df cmp"@0xffffff8008a658f8 +
				 * "5400008a b.ge"@0xffffff8008a658fc */
				if (mp_items[index].max_buf[idx] < buf[idx])
					mp_items[index].max_buf[idx] = buf[idx];
				/* "6b0300df cmp"@0xffffff8008a65914 +
				 * "540004ad b.le"@0xffffff8008a65918 */
				if (mp_items[index].min_buf[idx] > buf[idx])
					mp_items[index].min_buf[idx] = buf[idx];
				break;
			}
		}
	}
}

/*
 * ===========================================================================
 * self_test -- 0xffffff8008a64e94, 44 byte, `t` (riga 2592)
 * st_test   -- 0xffffff8008a64ec0, 44 byte, `t` (riga 2598)
 * ===========================================================================
 * Due delle SETTE funzioni installate nel campo `c144` degli elementi di
 * collaudo, e le due che non fanno niente: stampano un messaggio d'errore e
 * tornano -1 ("12800000 mov"@0xffffff8008a64eb4 e
 * "12800000 mov"@0xffffff8008a64ee0).
 *
 * IL PARAMETRO NON E' USATO, ma c'e': tutte e sette le funzioni di collaudo
 * sono chiamate attraverso `c144` con l'indice in w0
 * ("d63f0100 blr"@0xffffff8008a66cd8 preceduta da
 * "2a1303e0 mov"@0xffffff8008a66cd4), e le altre cinque lo usano
 * ("9b2f3810 smaddl"@0xffffff8008a65890 e simili). Qui il corpo non lo tocca
 * e il binario, da solo, non potrebbe dire se il parametro esiste: la firma e'
 * fissata dal TIPO DEL CAMPO che le contiene, non da queste due funzioni.
 *
 * Il livello e' KERN_ERR in tutte e due ("\x013"), quindi `ILI_ERR`.
 */
static __used int self_test(int index)
{
	/* "\x013ILITEK: (%s, %d): TDDI has no self to be tested currently\n"@0xffffff8009244805,
	 * __func__ = "self_test"@0xffffff8009244842;
	 * "52814402 mov"@0xffffff8008a64eac -- __LINE__ = 2592 */
#line 2592
	ILI_ERR("TDDI has no self to be tested currently\n");
	return -1;
}

static __used int st_test(int index)
{
	/* "\x013ILITEK: (%s, %d): ST Test is not supported by the driver\n"@0xffffff800924484c,
	 * __func__ = "st_test"@0xffffff8009244888;
	 * "528144c2 mov"@0xffffff8008a64ed8 -- __LINE__ = 2598 */
#line 2598
	ILI_ERR("ST Test is not supported by the driver\n");
	return -1;
}

/*
 * ===========================================================================
 * mp_do_retry -- 0xffffff8008a66c84, 156 byte, `t` (righe 2833..2837)
 * ===========================================================================
 * SI CHIAMA DA SOLA: "97ffffe5 bl"@0xffffff8008a66cf0 ha per destinazione
 * 0xffffff8008a66c84, cioe' il proprio ingresso. La ricorsione NON e' stata
 * trasformata in ciclo dal compilatore, e nel sorgente e' scritta come
 * ricorsione.
 *
 * DUE PARAMETRI `int`: w0 e' l'indice dell'elemento
 * ("9b292015 smaddl"@0xffffff8008a66ca4, w0 * 152 + base) e w1 e' un
 * contatore, provato con "34000321 cbz"@0xffffff8008a66c94 e decrementato con
 * "51000681 sub"@0xffffff8008a66ce8 prima della chiamata ricorsiva. Il
 * messaggio lo chiama `retry`.
 *
 * NON TORNA NIENTE: nessuno dei due rami mette un valore in w0 prima
 * dell'epilogo -- l'ultima cosa che tocca w0 e' la `printk`. E' `void`.
 *
 * IL SECONDO ARGOMENTO DEL MESSAGGIO E' IL NOME DELL'ELEMENTO:
 * "f94002a4 ldr"@0xffffff8008a66ca8 legge il campo a +0 e lo passa in x4,
 * cioe' come quinto argomento della `printk`, che per il formato
 * "retry = %d, item = %s" e' il `%s`.
 *
 * L'ORDINE E' QUELLO DEL BINARIO: prima la chiamata indiretta
 * ("d63f0100 blr"@0xffffff8008a66cd8), poi
 * `mp_comp_result_before_retry` ("97fffcab bl"@0xffffff8008a66ce0, che punta
 * a 0xffffff8008a65f8c) e infine la prova del segno
 * ("36f80160 tbz"@0xffffff8008a66ce4, bit 31).
 *
 * `mp_comp_result_before_retry` NON E' SCRITTA IN QUESTO LOTTO: e' dichiarata
 * e lasciata indefinita (regola 6). Il link fallisce, ed e' l'esito onesto.
 */
static int mp_comp_result_before_retry(int index);

static __used void mp_do_retry(int index, int count)
{
	/* "34000321 cbz"@0xffffff8008a66c94 */
	if (count == 0) {
		/* "\x016ILITEK: (%s, %d): Finish retry action\n"@0xffffff8009245369,
		 * __func__ = "mp_do_retry"@0xffffff8009245392;
		 * "52816222 mov"@0xffffff8008a66d08 -- __LINE__ = 2833 */
#line 2833
		ILI_INFO("Finish retry action\n");
		return;
	}

	/* "\x016ILITEK: (%s, %d): retry = %d, item = %s\n"@0xffffff800924539e;
	 * "528162a2 mov"@0xffffff8008a66cc4 -- __LINE__ = 2837 */
#line 2837
	ILI_INFO("retry = %d, item = %s\n", count, mp_items[index].c0);

	mp_items[index].c144(index);

	if (mp_comp_result_before_retry(index) < 0)
		mp_do_retry(index, count - 1);
}

/*
 * ===========================================================================
 * mp_compare_cdc_show_result -- 0xffffff8008a66d20, 1068 byte, `t` (riga 1155)
 * ===========================================================================
 * OTTO PARAMETRI, e il binario li tipa tutti:
 *   w0  `int`    -- indice dell'elemento: "9b292148 smaddl"@0xffffff8008a66e70
 *                   (w10 * 152 + base della tavola), con w10 riletto da
 *                   [sp,#12] dove w0 era stato messo da
 *                   "b9000fe0 str"@0xffffff8008a66d6c;
 *   x1  `int *`  -- i dati: "b868dac3 ldr"@0xffffff8008a66ee0, quattro byte,
 *                   indice esteso CON SEGNO;
 *   x2  `char *` -- il buffer di uscita: "8b38c280 add"@0xffffff8008a66dbc
 *                   somma `len` esteso con segno SENZA scala, quindi passo 1;
 *   x3  `int *`  -- lunghezza in ingresso E in uscita:
 *                   "b9400078 ldr"@0xffffff8008a66d44 la legge all'ingresso e
 *                   "b9000118 str"@0xffffff8008a67128 la riscrive alla fine;
 *   w4  `int`    -- confrontato con 2 ("71000abf cmp"@0xffffff8008a66f48) e
 *                   con 0 ("35000255 cbnz"@0xffffff8008a66fc0);
 *   x5, x6 `int *` -- "a9011be5 stp"@0xffffff8008a66d40 li salva in coppia a
 *                   [sp,#16] e [sp,#24]; x5 e' l'estremo SUPERIORE
 *                   ("6b08007f cmp"@0xffffff8008a66f40 con
 *                   "5400026d b.le"@0xffffff8008a66f44) e x6 l'INFERIORE
 *                   ("6b09007f cmp"@0xffffff8008a66f00 con
 *                   "5400082a b.ge"@0xffffff8008a66f04);
 *   x7  `char *` -- passato come `%s` a "\n %s "@0xffffff80092458ee.
 *
 * NON TORNA NIENTE. L'epilogo (da "a9477bfd ldp"@0xffffff8008a6712c a
 * "d65f03c0 ret"@0xffffff8008a67148) non tocca w0: l'ultima istruzione che lo
 * scrive e' una `snprintf`. La variabile che nel corpo vale 0, -1 o -105 non
 * e' un valore di ritorno ma un locale, letto solo da
 * "340002dc cbz"@0xffffff8008a670a0 per scegliere fra PASS e FAIL. E' `void`.
 *
 * IL CONTROLLO SUL PUNTATORE E' `IS_ERR` PIU' `NULL`, IN QUEST'ORDINE:
 * "b140043f cmn"@0xffffff8008a66d54 somma 4096 a x1 e
 * "54001b08 b.hi"@0xffffff8008a66d5c prende il ramo quando il riporto e'
 * acceso e il risultato non e' zero, cioe' quando x1 sta in [-4095, -1] --
 * che e' esattamente `IS_ERR_VALUE`. Solo DOPO viene il confronto con zero,
 * "b4001af6 cbz"@0xffffff8008a66d60. L'ordine e' misurato.
 *
 * IL TETTO DEL BUFFER E' 0x100000 = 1048576: "320c03fc orr"@0xffffff8008a66d8c
 * e "320c03fb orr"@0xffffff8008a66e30. Che nel sorgente sia scritto
 * `1024 * 1024`, `0x100000` o `SZ_1M` non e' deciso dal binario: qui e'
 * `1024 * 1024`, ed e' una SCELTA.
 *
 * LE COSTANTI DEL RAMO "BYPASS" SONO INT_MAX E INT_MIN, e la prova e' la forma
 * dell'intervallo: "320107e9 mov"@0xffffff8008a66fc4 mette w9 = 0x80000001,
 * "0b090069 add"@0xffffff8008a66fc8 lo somma al dato e
 * "7100053f cmp"@0xffffff8008a66fcc con
 * "540001c8 b.hi"@0xffffff8008a66fd0 prende il ramo normale se il risultato
 * supera 1 SENZA segno. Restano due soli valori possibili: 0x7fffffff e
 * 0x80000000.
 *
 * LE STAMPE DI DEBUG NON SONO `ILI_DBG`: la guardia e' la stessa
 * ("39656108 ldrb"@0xffffff8008a66e1c legge `ilitek_dbg_en`), ma il letterale
 * comincia con SOH + 'c' (KERN_CONT) e NON con "ILITEK: (%s, %d): ". Sono
 * `printk(KERN_CONT ...)` sotto un `if`, e qui passano per `ILI_CONT` (vedi il
 * cappello della macro).
 *
 * IL DATO E' RILETTO DOPO OGNI `printk`: per esempio
 * "b8777ac3 ldr"@0xffffff8008a66f1c ricarica `data[idx]` subito dopo
 * "97db316f bl"@0xffffff8008a66f18. E' quello che il compilatore deve fare se
 * il sorgente scrive `data[idx]` una seconda volta e in mezzo c'e' una
 * chiamata che puo' scrivere in memoria: la forma con un locale estratto NON
 * lo produrrebbe (classe A2).
 *
 * DIVERGENZA F2-D1, DICHIARATA E NON AGGIRATA -- L'ORDINE DEI DUE OPERANDI
 * DEL SECONDO E DEL TERZO RAMO. Di fabbrica il ramo "sopra il massimo" prova
 * PRIMA il valore ("6b08007f cmp"@0xffffff8008a66f40 con
 * "5400026d b.le"@0xffffff8008a66f44) e POI il tipo
 * ("71000abf cmp"@0xffffff8008a66f48 con
 * "54000361 b.ne"@0xffffff8008a66f4c), mentre il ramo "sotto il minimo" fa il
 * contrario ("71000abf cmp"@0xffffff8008a66f90 prima,
 * "6b08007f cmp"@0xffffff8008a66fa0 dopo). E' un'asimmetria della fabbrica.
 *
 * Scrivendola alla lettera -- `data[idx] > max[idx] && type == 2` nel secondo
 * ramo e `type == 2 && data[idx] < min[idx]` nel terzo -- la funzione misura
 * 1088 su 1068: cinque istruzioni in piu' (+1 `adrp`, +1 `ldrb`, +1 `cbnz`,
 * +2 `b`), perche' clang inoltra ("jump threading") l'arco `type != 2` del
 * terzo ramo direttamente al ramo normale e deve poi ricaricare
 * `ilitek_dbg_en` in un blocco a parte. Scrivendo il terzo ramo
 * `data[idx] < min[idx] && type == 2` la funzione misura 1068 ESATTI, 267
 * istruzioni su 267, e restano CINQUE posizioni in cui il mnemonico differisce
 * -- le posizioni 135..140 della sequenza, dove le due coppie
 * (`ldr`,`ldr`,`cmp`) e (`cmp`,`b.ne`) sono scambiate fra loro.
 *
 * NESSUNA DELLE DUE SCRITTURE RIPRODUCE LA FABBRICA ESATTAMENTE, e va detto
 * invece che nascosto: qui e' scelta quella che riproduce il CONTEGGIO
 * (1068 = 1068) invece di quella che riproduce l'ORDINE di un ramo e sbaglia
 * il conteggio di venti byte. Il residuo di cinque posizioni resta aperto.
 *
 * `static`: nella mappa e' `t`; `__used` perche' in questo lotto nessuno la
 * chiama.
 */
static __used void mp_compare_cdc_show_result(int index, int *data, char *buf,
					      int *plen, int type, int *max,
					      int *min, char *desp)
{
	int len = *plen;
	int ret = 0;
	int i;
	int j;

	/* "b140043f cmn"@0xffffff8008a66d54 + "b4001af6 cbz"@0xffffff8008a66d60 */
	if (IS_ERR(data) || data == NULL) {
		/* "\x013ILITEK: (%s, %d): The data of test item is null (%p)\n"@0xffffff80092451d1,
		 * __func__ = "mp_compare_cdc_show_result"@0xffffff80092458d1;
		 * "52809062 mov"@0xffffff8008a670cc -- __LINE__ = 1155 */
		/* F1-D1: `#line` riporta il `__LINE__` alla riga 1155 di fabbrica */
#line 1155
		ILI_ERR("The data of test item is null (%p)\n", data);
		/* "12800d1c mov"@0xffffff8008a670d8 -- w28 = 0xffffff97 = -105 */
		ret = -105;
		goto out;
	}

	/* "7100051f cmp"@0xffffff8008a66d70 + "5400052b b.lt"@0xffffff8008a66d74 */
	for (i = 0; i < core_mp.c232; i++) {
		/* "350001fa cbnz"@0xffffff8008a66d98: la prova e' sul valore
		 * PRIMA dell'incremento, cioe' su `i` */
		if (i == 0) {
			if (ilitek_dbg_en)
				/* "\n %s "@0xffffff80092458ee (la coda della
				 * stringa KERN_CONT che comincia a
				 * 0xffffff80092458ec) */
				ILI_CONT("\n %s ", desp);
			/* "\n\t   %s ,"@0xffffff80092458f4 */
			len += snprintf(buf + len, 1024 * 1024 - len,
					"\n\t   %s ,", desp);
		}
		if (ilitek_dbg_en)
			/* "  X_%d\t,"@0xffffff8009245900 */
			ILI_CONT("  X_%d\t,", i + 1);
		/* "\t X_%d  ,"@0xffffff8009245909 */
		len += snprintf(buf + len, 1024 * 1024 - len,
				"\t X_%d  ,", i + 1);
	}

	if (ilitek_dbg_en)
		/* il letterale e' il solo `"\n"`, che questo file usa in DUE
		 * posti con DUE indirizzi diversi -- 0xffffff80090ddcf3 (la
		 * coda della stringa KERN_CONT che comincia a
		 * 0xffffff80090ddcf1) qui, e 0xffffff8009245994 nella
		 * `snprintf` qui sotto. `verificacitazioni.py` ancora un
		 * letterale a UN indirizzo solo e risponde INDIRIZZO_AMBIGUO
		 * se ne trova due: percio' i due indirizzi stanno qui in
		 * prosa e `\n` resta fra le `--eccezione`. Il DELTA DI
		 * STRUMENTO che servirebbe e' ammettere piu' indirizzi per lo
		 * stesso letterale; non e' stato fatto (strumento condiviso) */
		ILI_CONT("\n");
	len += snprintf(buf + len, 1024 * 1024 - len, "\n");

	/* "7100051f cmp"@0xffffff8008a66e58 + "5400146b b.lt"@0xffffff8008a66e5c */
	for (i = 0; i < core_mp.c236; i++) {
		if (ilitek_dbg_en)
			/* "  Y_%d\t,"@0xffffff8009245915 */
			ILI_CONT("  Y_%d\t,", i + 1);
		/* "\t Y_%d  ,"@0xffffff800924591e */
		len += snprintf(buf + len, 1024 * 1024 - len,
				"\t Y_%d  ,", i + 1);

		for (j = 0; j < core_mp.c232; j++) {
			int idx = i * core_mp.c232 + j;

			/* "7100253f cmp"@0xffffff8008a66ee8 (con 9) */
			if (mp_items[index].c16 == 9) {
				if (data[idx] < min[idx]) {
					if (ilitek_dbg_en)
						/* " #%7d "@0xffffff800924592a */
						ILI_CONT(" #%7d ", data[idx]);
					/* "#%7d,"@0xffffff8009245931 */
					len += snprintf(buf + len,
							1024 * 1024 - len,
							"#%7d,", data[idx]);
					ret = -1;
				} else {
					if (ilitek_dbg_en)
						/* " %7d "@0xffffff8009245939 */
						ILI_CONT(" %7d ", data[idx]);
					/* " %7d, "@0xffffff800924593f */
					len += snprintf(buf + len,
							1024 * 1024 - len,
							" %7d, ", data[idx]);
				}
			} else if (data[idx] > max[idx] && type == 2) {
				if (ilitek_dbg_en)
					/* " *%7d "@0xffffff8009245950 */
					ILI_CONT(" *%7d ", data[idx]);
				/* "*%7d,"@0xffffff8009245957 */
				len += snprintf(buf + len, 1024 * 1024 - len,
						"*%7d,", data[idx]);
				ret = -1;
			} else if (data[idx] < min[idx] && type == 2) {
				if (ilitek_dbg_en)
					ILI_CONT(" #%7d ", data[idx]);
				len += snprintf(buf + len, 1024 * 1024 - len,
						"#%7d,", data[idx]);
				ret = -1;
			} else if (type == 0 && (data[idx] == INT_MAX ||
						 data[idx] == INT_MIN)) {
				if (ilitek_dbg_en)
					/* "%s"@0xffffff8009101762 */
					ILI_CONT("%s", "BYPASS,");
				/* "BYPASS,"@0xffffff8009245946 -- lo stesso
				 * indirizzo serve da argomento della `printk` e
				 * da formato della `snprintf`:
				 * "f0003ef7 adrp"@0xffffff8008a66fd4 con
				 * "91251af7 add"@0xffffff8008a66fd8 lo
				 * materializza una volta sola in x23 */
				len += snprintf(buf + len, 1024 * 1024 - len,
						"BYPASS,");
			} else {
				if (ilitek_dbg_en)
					ILI_CONT(" %7d ", data[idx]);
				len += snprintf(buf + len, 1024 * 1024 - len,
						" %7d, ", data[idx]);
			}
		}

		if (ilitek_dbg_en)
			ILI_CONT("\n");
		len += snprintf(buf + len, 1024 * 1024 - len, "\n");
	}

out:
	/* "71000abf cmp"@0xffffff8008a67098 + "54000441 b.ne"@0xffffff8008a6709c */
	if (type == 2) {
		/* "340002dc cbz"@0xffffff8008a670a0 */
		if (ret == 0) {
			/* "\x016\n Result : PASS\n"@0xffffff800924595d */
			printk(KERN_INFO "\n Result : PASS\n");
			/* "Result : PASS\n"@0xffffff8009245961 */
			len += snprintf(buf + len, 1024 * 1024 - len,
					"Result : PASS\n");
		} else {
			/* "\x016\n Result : FAIL\n"@0xffffff8009245970 */
			printk(KERN_INFO "\n Result : FAIL\n");
			/* "Result : FAIL\n"@0xffffff8009245974 */
			len += snprintf(buf + len, 1024 * 1024 - len,
					"Result : FAIL\n");
		}
	}

	/* "b9000118 str"@0xffffff8008a67128 */
	*plen = len;
}
/*
 * ===========================================================================
 * IL SOTTO-LOTTO F4 -- mp_comp_result_before_retry, E LE DUE INCORPORATE
 * ===========================================================================
 * `mp_comp_result_before_retry` sta a 0xffffff8008a65f8c ed e' lunga 3320
 * byte, 830 istruzioni. E' un simbolo SOLO: nel disassemblato non c'e' nessuna
 * `bl` verso `mp_compare_cdc_result` ne' verso `mp_test_data_sort_average`, e
 * nemmeno verso `ipio_kfree`. Eppure il blocco contiene TRE `__func__`
 * distinti, e i due che non sono il suo denunciano due funzioni INCORPORATE
 * (classe di difetto B5):
 *
 *   "mp_comp_result_before_retry"@0xffffff800924516e   righe 2756 2764 2773 2796
 *   "mp_compare_cdc_result"@0xffffff8009245209         riga 2724
 *   "mp_test_data_sort_average"@0xffffff8009245248     righe 2638 2645 2656
 *
 * CHE NON SIANO SIMBOLI E' MISURATO, non supposto:
 *
 *   $ grep -cE " (ipio_kfree|mp_compare_cdc_result|mp_test_data_sort_average)$" \
 *         oracolo/stock.map
 *   0
 *
 * Qui sono scritte come tre funzioni `static` separate, e il compilatore di
 * fabbrica le incorpora da solo: nell'oggetto che ne esce non compaiono, e
 * `mp_comp_result_before_retry` misura 3320 su 3320.
 *
 * QUANTE VOLTE E' INCORPORATA `mp_compare_cdc_result`. La consegna di questo
 * lotto diceva CINQUE, contando i cinque blocchi di `printk` che portano la
 * riga 2724 (0xffffff8008a662b4, 0xffffff8008a66368, 0xffffff8008a6640c,
 * 0xffffff8008a665e4, 0xffffff8008a66bf8). I SITI DI INCORPORAZIONE SONO
 * SETTE, e tre di essi condividono lo stesso blocco d'errore
 * (0xffffff8008a66bf8) perche' clang ne ha fuso le code. Si contano dai sette
 * caricamenti distinti del dato da confrontare:
 *
 *   "f9462123 ldr"@0xffffff8008a660a4   core_mp.tx_max_buf   (pagina+3136)
 *   "f9462503 ldr"@0xffffff8008a66318   core_mp.tx_min_buf   (pagina+3144)
 *   "f9462923 ldr"@0xffffff8008a663c0   core_mp.rx_max_buf   (pagina+3152)
 *   "f9462d03 ldr"@0xffffff8008a6642c   core_mp.rx_min_buf   (pagina+3160)
 *   "f9400143 ldr"@0xffffff8008a66598   mp_items[index].max_buf
 *   "f9400323 ldr"@0xffffff8008a666a0   mp_items[index].min_buf
 *   "f9400303 ldr"@0xffffff8008a66ba4   mp_items[index].result_buf
 *
 * Contare i blocchi di messaggio invece dei siti di chiamata SOTTOSTIMA: e' la
 * classe C5, "un confine assolto per nome".
 */

/*
 * ===========================================================================
 * ipio_kfree -- INCORPORATA, e la sua forma vale QUATTRO byte
 * ===========================================================================
 * La coda di `mp_comp_result_before_retry` libera i due vettori di soglia:
 *
 *   "aa1303e0 mov"@0xffffff8008a66164   x0 = thr_max
 *   "97dfa992 bl"@0xffffff8008a66168    kfree(x0)
 *   "b4000074 cbz"@0xffffff8008a6616c   se thr_min == 0 salta
 *
 * cioe' la stessa forma di `ILI_KFREE` gia' definita in questo file. MA I SITI
 * DI USCITA DENUNCIANO CHE IL PUNTATORE PASSA PER LA MEMORIA. Alle tre uscite
 * normali il binario RIPROVA `thr_max` contro zero:
 *
 *   "b5ffa7d3 cbnz"@0xffffff8008a66c6c
 *   "b5ffaa73 cbnz"@0xffffff8008a66c18
 *   "b5ffa753 cbnz"@0xffffff8008a66c7c
 *
 * e su quelle uscite `thr_max` E' GIA' STATO PROVATO non nullo dal controllo
 * dell'allocazione ("b4000873 cbz"@0xffffff8008a66008). Scritta con
 * `if (thr_max) kfree(thr_max);` su una variabile locale, clang propaga la non
 * nullita' e quelle tre `cbnz` SPARISCONO: la funzione misura 3304 invece di
 * 3320, e le tre uscite si fondono in una sola. Passando invece l'INDIRIZZO
 * della variabile a una funzione che rilegge `*mem`, la propagazione non
 * avviene e le tre `cbnz` tornano, con i loro blocchi separati: 3320 su 3320.
 * SEDICI BYTE hanno detto che il puntatore e' passato per indirizzo.
 *
 * IL NOME `ipio_kfree` E' SCELTO -- il binario non lo da', perche' la funzione
 * non e' un simbolo e nessun messaggio la nomina. Cio' che e' MISURATO e' la
 * forma: prova su `*mem`, `kfree(*mem)`, e il fatto che l'argomento sia
 * l'indirizzo della variabile e non il suo valore. La scrittura `*mem = NULL`
 * non e' misurabile qui (la variabile e' morta subito dopo e clang toglie la
 * scrittura) ed e' una SCELTA, presa per coerenza con `ILI_KFREE`, che il
 * lotto F1 aveva gia' misurato in quella forma su
 * `create_mp_test_frame_buffer`.
 */
static void ipio_kfree(void **mem)
{
	if (*mem != NULL) {
		kfree(*mem);
		*mem = NULL;
	}
}

/*
 * ===========================================================================
 * mp_test_data_sort_average -- INCORPORATA una volta, righe 2638/2645/2656
 * ===========================================================================
 * Prende le `c48` ripetizioni della misura di un elemento di collaudo, le
 * ordina nodo per nodo, scarta le `up` piu' alte e le `down` piu' basse e
 * scrive la media di quel che resta in `avg_result`.
 *
 * I TRE PARAMETRI. Sono i tre valori che il chiamante prepara subito prima:
 *   `data`       -- "f9400139 ldr"@0xffffff8008a6660c legge
 *                   mp_items[index].buf (il campo a +96, individuato da
 *                   "f8460d28 ldr"@0xffffff8008a661b0); e' un `int *` perche'
 *                   "b840472a ldr"@0xffffff8008a667a0 lo scorre a passo 4;
 *   `index`      -- il corpo legge `mp_items[index].c48`, `.c56`, `.c60` e
 *                   `.c0`, quindi l'indice serve dentro;
 *   `avg_result` -- "f9400309 ldr"@0xffffff8008a66610 legge
 *                   mp_items[index].result_buf (+88, individuato da
 *                   "f8458f08 ldr"@0xffffff8008a661f8); ci si scrive con
 *                   "b82a792c str"@0xffffff8008a66acc, quattro byte.
 * L'ORDINE dei tre parametri NON e' deciso dal binario: la funzione e'
 * incorporata e nessun registro d'argomento viene preparato. E' una SCELTA.
 *
 * NON TORNA NIENTE: dopo il punto in cui rientra nel chiamante
 * ("f9400303 ldr"@0xffffff8008a66ba4) nessun registro portato fuori dal corpo
 * viene provato. E' `void`.
 *
 * LE DUE USCITE ANTICIPATE, nell'ordine del binario:
 *   1. "b8430f6a ldr"@0xffffff8008a66614 legge `c48` (pre-indice, x27 =
 *      elemento + 48), "7100095f cmp"@0xffffff8008a66618 lo confronta con 2 e
 *      "54002c4b b.lt"@0xffffff8008a6661c esce se e' minore. Confronto CON
 *      SEGNO;
 *   2. "b140073f cmn"@0xffffff8008a66620 e "b4000859 cbz"@0xffffff8008a66628
 *      sono `IS_ERR` seguito dal confronto con zero, nello STESSO ordine
 *      misurato in `mp_compare_cdc_show_result`.
 *
 * LE DUE ALLOCAZIONI SONO `kcalloc`, E LO PROVANO DUE COSE INDIPENDENTI.
 *   - Le bandiere: "52901801 mov"@0xffffff8008a66640 mette w1 = 0x80c0 e
 *     "72a02801 movk"@0xffffff8008a66648 ci sovrappone 0x140 a partire dal bit
 *     16, cioe' 0x14080c0. Dal `gfp.h` dell'albero:
 *     ___GFP_IO 0x40, ___GFP_FS 0x80, ___GFP_DIRECT_RECLAIM 0x400000,
 *     ___GFP_KSWAPD_RECLAIM 0x1000000 -- la loro somma e' GFP_KERNEL =
 *     0x14000c0 -- piu' ___GFP_ZERO 0x8000. 0x14000c0 | 0x8000 = 0x14080c0
 *     ESATTO. Non e' `kmalloc`: e' `GFP_KERNEL | __GFP_ZERO`.
 *   - La prova di traboccamento: "37f808a9 tbnz"@0xffffff8008a66638 salta al
 *     ramo "allocazione fallita" quando il BIT 31 del conteggio e' acceso.
 *     Quella prova e' esattamente `n > SIZE_MAX / size` di `kmalloc_array`
 *     quando `size` vale 4 e `n` e' un `int` esteso con segno: per un `int`
 *     negativo l'estensione supera 0x3fffffffffffffff, per uno non negativo
 *     no. `kmalloc` non la genera, `kcalloc` si'.
 * La moltiplicazione del primo conteggio e' "1b0a7d09 mul"@0xffffff8008a66630
 * (core_mp.c256 * mp_items[index].c48); la scala per 4 e'
 * "d37ef500 lsl"@0xffffff8008a66644, e la chiamata
 * "97dfa74f bl"@0xffffff8008a6664c va a `__kmalloc`.
 *
 * L'ORDINE DEI DUE CONTROLLI E' MISURATO, E VALE DODICI BYTE. Il binario prova
 * PRIMA il secondo puntatore ("b140041f cmn"@0xffffff8008a66768 e
 * "b4002060 cbz"@0xffffff8008a66770, su x0 appena tornato da `__kmalloc`) e
 * POI il primo ("b14006bf cmn"@0xffffff8008a66774 e
 * "b4002015 cbz"@0xffffff8008a6677c, su x21). Scritto nell'ordine opposto --
 * cioe' l'ordine "naturale", prima quello allocato per primo -- il blocco
 * della seconda allocazione viene duplicato invece che condiviso e la funzione
 * misura 3304 invece di 3316. E' una misura sull'ORDINE DEGLI OPERANDI di un
 * `||`, la stessa classe della divergenza F2-D1.
 *
 * NESSUNA `kfree` SUL RAMO D'ERRORE: dal messaggio della riga 2645
 * ("52814aa2 mov"@0xffffff8008a66b8c) il codice salta direttamente al rientro
 * nel chiamante, e i due `bl kfree` (0xffffff8008a66b6c e 0xffffff8008a66b74)
 * stanno solo sul cammino riuscito. Se una delle due allocazioni riesce e
 * l'altra no, la riuscita NON viene liberata: e' una perdita di memoria della
 * FABBRICA e si riproduce (regola 7).
 *
 * IL LIMITE DELLA COPIA E' UN LOCALE, E QUATTRO ISTRUZIONI LO DICONO. Il
 * binario calcola `core_mp.c256 * mp_items[index].c48` UNA VOLTA
 * ("1b097d09 mul"@0xffffff8008a6678c, con
 * "7100053f cmp"@0xffffff8008a66790 subito dopo) e poi scorre con un
 * contatore all'indietro e due puntatori post-incrementati
 * ("b840472a ldr"@0xffffff8008a667a0, "f1000508 subs"@0xffffff8008a667a4,
 * "b800452a str"@0xffffff8008a667a8). Scrivendo il limite dentro la condizione
 * del `for`, clang rilegge i due globali A OGNI GIRO e il ciclo diventa di
 * dieci istruzioni invece di sette. E' la classe A2 letta al contrario: il
 * binario NON rilegge, quindi il sorgente ha estratto il locale.
 *
 * `up` E `down`: DUE PRODOTTI DIVISI PER CENTO. "29472929 ldp"@0xffffff8008a667c8
 * legge in coppia i campi a +56 e +60 dell'elemento; "1b087d4a mul"@0xffffff8008a667d8
 * moltiplica il SECONDO (c60) per `c48` e "1b087d28 mul"@0xffffff8008a667dc il
 * PRIMO (c56). La divisione per 100 e' la sequenza di clang con la costante
 * magica 0x51eb851f ("5290a3eb mov"@0xffffff8008a667cc +
 * "72aa3d6b movk"@0xffffff8008a667d0), "9b2b7d4a smull"@0xffffff8008a667e4,
 * "9365fd4a asr"@0xffffff8008a667f0 (37 bit) e la correzione del segno
 * "d37ffd4b lsr"@0xffffff8008a667ec + "0b0b0156 add"@0xffffff8008a667f4:
 * e' una divisione CON SEGNO, quindi `c56` e `c60` sono `int`.
 * QUALE DEI DUE E' `up` lo dice il messaggio: "2a1603e3 mov"@0xffffff8008a6683c
 * mette il prodotto di `c60` nel TERZO argomento di `printk`, che per il
 * formato "Up=%d, Down=%d -%s" e' il primo `%d`, cioe' `Up`; il prodotto di
 * `c56` va nel quarto ("b9402fe4 ldr"@0xffffff8008a66814), cioe' `Down`.
 *
 * LE TRE STAMPE DI DUMP SONO `KERN_CONT` SOTTO `ilitek_dbg_en`
 * ("39656189 ldrb"@0xffffff8008a667e0 legge il byte a pagina+2392). La prima
 * guardia salta OLTRE anche il blocco successivo perche' clang inoltra l'arco
 * ("jump threading"): sul cammino in cui il byte vale zero non c'e' nessuna
 * chiamata fra i due carichi, quindi il secondo carico e' lo stesso valore.
 * I due `5` sono immediati: "528000a1 mov"@0xffffff8008a66858 e
 * "528000a2 mov"@0xffffff8008a6685c.
 *
 * LA CONDIZIONE `i < 5 || i >= core_mp.c256 - 5` compare tre volte e il
 * binario la calcola in due modi diversi, e la differenza NON e' casuale:
 *   - nei primi due cicli e' un salto: "7100179f cmp"@0xffffff8008a668d0 +
 *     "540000a3 b.cc"@0xffffff8008a668d4, e dentro il ciclo interno diventa
 *     "7100139f cmp"@0xffffff8008a66888 + "1a9f97f7 cset"@0xffffff8008a66890
 *     ("i > 4", SENZA segno) messo in `and`
 *     ("0a17014a and"@0xffffff8008a668a8) con
 *     "1a9fa7ea cset"@0xffffff8008a668a4 ("i < c256-5", CON segno) e provato
 *     da "370000ca tbnz"@0xffffff8008a668ac;
 *   - nel TERZO ciclo il valore booleano sopravvive al ciclo
 *     ("1a9f97ea cset"@0xffffff8008a66b40, "1a9fa7e9 cset"@0xffffff8008a66b48,
 *     "0a090149 and"@0xffffff8008a66b50) e viene riprovato DOPO l'uscita
 *     ("37000089 tbnz"@0xffffff8008a66b58) per decidere se stampare l'ultimo
 *     "\n". Un valore vivo dopo il ciclo si ha solo se il sorgente riusa `i`
 *     DOPO il `for`: e' questo che ha fatto scrivere l'ultimo `if` fuori dal
 *     ciclo invece che dentro. Con l'`if` dentro, la funzione misurava 3264
 *     invece di 3276 (al netto delle altre correzioni) e il "\n" non veniva
 *     stampato quando `core_mp.c256 < 1`, mentre il binario in quel caso lo
 *     stampa ("540002ab b.lt"@0xffffff8008a66b08 va dritto al `printk`).
 *
 * IL RIORDINO E' UNA BOLLA A TRE CICLI, e gli indici sono LOCALI. Il ciclo di
 * mezzo va fino a `c48 - 1` ("5100054b sub"@0xffffff8008a66918 +
 * "7100057f cmp"@0xffffff8008a6691c), quello interno fino a `c48 - j - 1`
 * ("2a2803eb mvn"@0xffffff8008a66928 calcola -j-1 e
 * "0b0b014c add"@0xffffff8008a6692c lo somma a `c48`). I due indici sono
 * calcolati insieme, PRIMA del confronto ("1b0c25b0 madd"@0xffffff8008a66940 e
 * "1b0c25af madd"@0xffffff8008a66948) e RIUSATI dalle due `str` dello scambio
 * ("b82a7aae str"@0xffffff8008a66964 e "b82f7aad str"@0xffffff8008a66968).
 * Se il sorgente riscrivesse l'espressione d'indice dentro lo scambio, clang
 * dovrebbe rileggere `core_mp.c256` dopo la prima scrittura: sono quattro byte
 * in meno e due istruzioni diverse. Anche questa e' la classe A2 al contrario.
 * Il confronto e' CON SEGNO: "6b0e01bf cmp"@0xffffff8008a66954 +
 * "540000cd b.le"@0xffffff8008a66958.
 *
 * LA MEDIA. `sum_buf[i]` e' azzerato a ogni giro
 * ("b82a791f str"@0xffffff8008a66a84), accumulato DENTRO il ciclo interno
 * ("0b0c01ec add"@0xffffff8008a66ab8 + "b82a790c str"@0xffffff8008a66abc: la
 * scrittura sta nel ciclo, quindi il sorgente scrive `sum_buf[i] +=` e non un
 * accumulatore locale) e diviso alla fine
 * ("1acd0d8c sdiv"@0xffffff8008a66ac8, divisione CON SEGNO) per
 * `c48 - (down + up)` ("0b1602eb add"@0xffffff8008a66a7c calcola `down + up`
 * FUORI dal ciclo, "4b0b01ad sub"@0xffffff8008a66ac4 lo toglie da `c48`).
 * Il limite del ciclo interno e' `c48 - up`
 * ("4b1601ac sub"@0xffffff8008a66a88 + "6b0c02ff cmp"@0xffffff8008a66a8c +
 * "5400018a b.ge"@0xffffff8008a66a94) e parte da `down`.
 */
static void mp_test_data_sort_average(int *data, int index, int *avg_result)
{
	int i, j, k, tmp;
	int idx0, idx1;
	int len;
	int up, down;
	int *u32sum_raw_data = NULL;
	int *sum_buf = NULL;

	/* "b8430f6a ldr"@0xffffff8008a66614 + "7100095f cmp"@0xffffff8008a66618
	 * + "54002c4b b.lt"@0xffffff8008a6661c */
	if (mp_items[index].c48 < 2)
		return;

	/* "b140073f cmn"@0xffffff8008a66620 + "b4000859 cbz"@0xffffff8008a66628 */
	if (IS_ERR(data) || data == NULL) {
		/* "\x013ILITEK: (%s, %d): Input wrong address\n"@0xffffff800924521f,
		 * __func__ = "mp_test_data_sort_average"@0xffffff8009245248;
		 * "528149c2 mov"@0xffffff8008a66740 -- __LINE__ = 2638 */
#line 2638
		ILI_ERR("Input wrong address\n");
		return;
	}

	/* "1b0a7d09 mul"@0xffffff8008a66630, "37f808a9 tbnz"@0xffffff8008a66638,
	 * "97dfa74f bl"@0xffffff8008a6664c */
	u32sum_raw_data = kcalloc(core_mp.c256 * mp_items[index].c48,
				  sizeof(int), GFP_KERNEL);
	/* "36f807e8 tbz"@0xffffff8008a66658 e "37f82168 tbnz"@0xffffff8008a66750
	 * sono la stessa prova sul secondo conteggio: la seconda allocazione si
	 * fa ANCHE quando la prima e' traboccata
	 * ("aa1f03f5 mov"@0xffffff8008a6674c azzera il primo puntatore e prosegue);
	 * "97dfa709 bl"@0xffffff8008a66764 */
	sum_buf = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	/* L'ORDINE E' QUELLO DEL BINARIO, e vale dodici byte (vedi il cappello):
	 * "b140041f cmn"@0xffffff8008a66768 + "b4002060 cbz"@0xffffff8008a66770
	 * provano `sum_buf`, "b14006bf cmn"@0xffffff8008a66774 +
	 * "b4002015 cbz"@0xffffff8008a6677c provano `u32sum_raw_data` */
	if (IS_ERR(sum_buf) || sum_buf == NULL ||
	    IS_ERR(u32sum_raw_data) || u32sum_raw_data == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate u32sum_raw_data FRAME buffer\n"@0xffffff8009245262;
		 * "52814aa2 mov"@0xffffff8008a66b8c -- __LINE__ = 2645.
		 * NESSUNA `kfree` QUI: e' la perdita di memoria della fabbrica */
#line 2645
		ILI_ERR("Failed to allocate u32sum_raw_data FRAME buffer\n");
		return;
	}

	/* IL LOCALE E' MISURATO: "1b097d09 mul"@0xffffff8008a6678c una volta
	 * sola, e il ciclo scorre con "b840472a ldr"@0xffffff8008a667a0,
	 * "f1000508 subs"@0xffffff8008a667a4, "b800452a str"@0xffffff8008a667a8 */
	len = core_mp.c256 * mp_items[index].c48;
	for (i = 0; i < len; i++)
		u32sum_raw_data[i] = data[i];

	/* "29472929 ldp"@0xffffff8008a667c8 legge i due campi in coppia;
	 * "1b087d4a mul"@0xffffff8008a667d8 e "1b087d28 mul"@0xffffff8008a667dc;
	 * la divisione per 100 e' "9b2b7d4a smull"@0xffffff8008a667e4 +
	 * "9365fd4a asr"@0xffffff8008a667f0 */
	up = mp_items[index].c60 * mp_items[index].c48 / 100;
	down = mp_items[index].c56 * mp_items[index].c48 / 100;

	/* "\x016ILITEK: (%s, %d): Up=%d, Down=%d -%s\n"@0xffffff80092452a7;
	 * la guardia e' "39656189 ldrb"@0xffffff8008a667e0 (ilitek_dbg_en),
	 * quindi ILI_DBG e non ILI_INFO;
	 * "52814c02 mov"@0xffffff8008a66838 -- __LINE__ = 2656;
	 * "f8696905 ldr"@0xffffff8008a66828 mette il nome dell'elemento nel
	 * quinto argomento, cioe' nel `%s` */
#line 2656
	ILI_DBG("Up=%d, Down=%d -%s\n", up, down, mp_items[index].c0);

	if (ilitek_dbg_en) {
		/* "\n[Show Original frist%d and last%d node data]\n"@0xffffff80092452d1 (la
		 * coda della stringa KERN_CONT che comincia a 0xffffff80092452cf)
		 * -- il refuso «frist» per «first» e' della fabbrica e si
		 * riproduce (regola 7). I due 5 sono
		 * "528000a1 mov"@0xffffff8008a66858 e
		 * "528000a2 mov"@0xffffff8008a6685c */
		ILI_CONT("\n[Show Original frist%d and last%d node data]\n", 5, 5);
		for (i = 0; i < core_mp.c256; i++) {
			for (j = 0; j < mp_items[index].c48; j++) {
				/* "7100139f cmp"@0xffffff8008a66888 +
				 * "1a9f97f7 cset"@0xffffff8008a66890 +
				 * "1a9fa7ea cset"@0xffffff8008a668a4 +
				 * "0a17014a and"@0xffffff8008a668a8 +
				 * "370000ca tbnz"@0xffffff8008a668ac */
				if (i < 5 || i >= core_mp.c256 - 5)
					/* "%d,"@0xffffff8009245302 (coda di
					 * 0xffffff8009245300);
					 * l'indice e'
					 * "1b197108 madd"@0xffffff8008a668b0
					 * e il dato
					 * "b868daa1 ldr"@0xffffff8008a668b4 */
					ILI_CONT("%d,",
						 u32sum_raw_data[core_mp.c256 * j + i]);
			}
			/* "7100179f cmp"@0xffffff8008a668d0 +
			 * "540000a3 b.cc"@0xffffff8008a668d4 */
			if (i < 5 || i >= core_mp.c256 - 5)
				/* il letterale e' il solo "\n", a
				 * 0xffffff80090ddcf1 ("f00033a0
				 * adrp"@0xffffff8008a668e8): e' lo stesso che
				 * `mp_compare_cdc_show_result` usa, ed e' il
				 * motivo dell'unica `--eccezione` che resta */
				ILI_CONT("\n");
		}
	}

	for (i = 0; i < core_mp.c256; i++) {
		/* "5100054b sub"@0xffffff8008a66918 +
		 * "7100057f cmp"@0xffffff8008a6691c */
		for (j = 0; j < mp_items[index].c48 - 1; j++) {
			/* "2a2803eb mvn"@0xffffff8008a66928 +
			 * "0b0b014c add"@0xffffff8008a6692c +
			 * "7100059f cmp"@0xffffff8008a66930 */
			for (k = 0; k < mp_items[index].c48 - j - 1; k++) {
				/* I DUE INDICI SONO LOCALI, non espressioni
				 * riscritte: "1b0c25b0 madd"@0xffffff8008a66940
				 * e "1b0c25af madd"@0xffffff8008a66948 li
				 * calcolano PRIMA del confronto, e le due `str`
				 * dello scambio li riusano invece di rileggere
				 * `core_mp.c256` */
				idx0 = core_mp.c256 * k + i;
				idx1 = core_mp.c256 * (k + 1) + i;

				/* "b870daad ldr"@0xffffff8008a6694c +
				 * "b86fdaae ldr"@0xffffff8008a66950 +
				 * "6b0e01bf cmp"@0xffffff8008a66954 +
				 * "540000cd b.le"@0xffffff8008a66958 */
				if (u32sum_raw_data[idx0] > u32sum_raw_data[idx1]) {
					/* "b82a7aae str"@0xffffff8008a66964 +
					 * "b82f7aad str"@0xffffff8008a66968 */
					tmp = u32sum_raw_data[idx0];
					u32sum_raw_data[idx0] = u32sum_raw_data[idx1];
					u32sum_raw_data[idx1] = tmp;
				}
			}
		}
	}

	if (ilitek_dbg_en) {
		/* "\n[After sorting frist%d and last%d node data]\n"@0xffffff8009245308
		 * (coda di 0xffffff8009245306);
		 * i due 5 sono "528000a1 mov"@0xffffff8008a669b4 e
		 * "528000a2 mov"@0xffffff8008a669b8 */
		ILI_CONT("\n[After sorting frist%d and last%d node data]\n", 5, 5);
		for (i = 0; i < core_mp.c256; i++) {
			/* "b9402ff9 ldr"@0xffffff8008a669e8 rilegge `down` come
			 * valore iniziale; il limite e' `c48 - up`,
			 * "4b16012a sub"@0xffffff8008a669dc +
			 * "6b0a02ff cmp"@0xffffff8008a669e0 +
			 * "5400028a b.ge"@0xffffff8008a669e4 */
			for (j = down; j < mp_items[index].c48 - up; j++) {
				if (i < 5 || i >= core_mp.c256 - 5)
					ILI_CONT("%d,",
						 u32sum_raw_data[core_mp.c256 * j + i]);
			}
			if (i < 5 || i >= core_mp.c256 - 5)
				ILI_CONT("\n");
		}
	}

	for (i = 0; i < core_mp.c256; i++) {
		/* "b82a791f str"@0xffffff8008a66a84 */
		sum_buf[i] = 0;
		/* "4b1601ac sub"@0xffffff8008a66a88 +
		 * "6b0c02ff cmp"@0xffffff8008a66a8c +
		 * "5400018a b.ge"@0xffffff8008a66a94;
		 * l'indice e' "1b0e29ef madd"@0xffffff8008a66aa4 e il dato
		 * "b86fdaaf ldr"@0xffffff8008a66aa8 */
		for (j = down; j < mp_items[index].c48 - up; j++)
			/* "0b0c01ec add"@0xffffff8008a66ab8 +
			 * "b82a790c str"@0xffffff8008a66abc: la scrittura sta
			 * DENTRO il ciclo, quindi non c'e' accumulatore locale */
			sum_buf[i] += u32sum_raw_data[core_mp.c256 * j + i];
		/* "0b1602eb add"@0xffffff8008a66a7c (down+up, fuori dal ciclo),
		 * "4b0b01ad sub"@0xffffff8008a66ac4,
		 * "1acd0d8c sdiv"@0xffffff8008a66ac8 (CON SEGNO),
		 * "b82a792c str"@0xffffff8008a66acc */
		avg_result[i] = sum_buf[i] / (mp_items[index].c48 - (down + up));
	}

	if (ilitek_dbg_en) {
		/* "\n[Average result frist%d and last%d node data]\n"@0xffffff8009245339
		 * (coda di 0xffffff8009245337);
		 * i due 5 sono "528000a1 mov"@0xffffff8008a66af4 e
		 * "528000a2 mov"@0xffffff8008a66af8 */
		ILI_CONT("\n[Average result frist%d and last%d node data]\n", 5, 5);
		/* "370000a9 tbnz"@0xffffff8008a66b20 prova in cima al giro il
		 * booleano calcolato nella coda del giro PRECEDENTE
		 * ("f1000eff cmp"@0xffffff8008a66b34,
		 * "1a9f97ea cset"@0xffffff8008a66b40,
		 * "eb29c2ff cmp"@0xffffff8008a66b44,
		 * "1a9fa7e9 cset"@0xffffff8008a66b48,
		 * "0a090149 and"@0xffffff8008a66b50) */
		for (i = 0; i < core_mp.c256; i++) {
			if (i < 5 || i >= core_mp.c256 - 5)
				ILI_CONT("%d,", avg_result[i]);
		}
		/* FUORI DAL CICLO, e su `i` che vale ormai `core_mp.c256`:
		 * "37000089 tbnz"@0xffffff8008a66b58 riprova lo stesso booleano
		 * DOPO l'uscita. Vedi il cappello: e' l'unica scrittura che
		 * spiega sia quella prova sia il fatto che con
		 * `core_mp.c256 < 1` il "\n" venga stampato lo stesso
		 * ("540002ab b.lt"@0xffffff8008a66b08) */
		if (i < 5 || i >= core_mp.c256 - 5)
			ILI_CONT("\n");
	}

	/* "aa1503e0 mov"@0xffffff8008a66b68 + "97dfa711 bl"@0xffffff8008a66b6c,
	 * poi "f94003e0 ldr"@0xffffff8008a66b70 +
	 * "97dfa70f bl"@0xffffff8008a66b74. NESSUNA guardia: qui clang sa che i
	 * due puntatori non sono nulli */
	kfree(u32sum_raw_data);
	kfree(sum_buf);
}

/*
 * ===========================================================================
 * I NOMI DELLE VARIABILI LOCALI DEL LOTTO F4 -- COSA E' MISURATO E COSA NO
 * ===========================================================================
 * Nessun nome di variabile LOCALE sta nel binario, e nessuno dei nomi usati
 * qui e' quindi un fatto. Cio' che e' misurato e' il RUOLO. L'elenco, per non
 * lasciare l'ambiguita' al lettore:
 *
 *   `u32sum_raw_data`  -- il nome viene dal messaggio
 *                         "Failed to allocate u32sum_raw_data FRAME buffer";
 *                         QUALE DEI DUE puntatori allocati lo porti NON e'
 *                         deciso dal binario, perche' il messaggio copre tutte
 *                         e due le allocazioni. E' SCELTO il grande (c256*c48
 *                         elementi), perche' e' il vettore per FRAME e il
 *                         messaggio dice "FRAME buffer";
 *   `sum_buf`          -- SCELTO. Misurato: c256 elementi, azzerato e poi
 *                         sommato dentro il ciclo interno;
 *   `avg_result`       -- SCELTO. Misurato: e' `mp_items[index].result_buf` e
 *                         ci finisce il quoziente della `sdiv`;
 *   `thr_max`,`thr_min`-- SCELTI. Misurato il ruolo: il primo e' l'estremo
 *                         SUPERIORE (il confronto che fallisce e'
 *                         "54fffeec b.gt"@0xffffff8008a662f0) e il secondo
 *                         l'INFERIORE ("54fffe6b b.lt"@0xffffff8008a66300);
 *   `up`, `down`       -- i due NOMI vengono dal formato
 *                         "Up=%d, Down=%d -%s", ed e' l'eccezione alla regola
 *                         5: il binario li nomina;
 *   `len`,`idx0`,`idx1`,`tmp`,`res`,`ret`,`i`,`j`,`k`
 *                      -- SCELTI. Di `res` e `ret` e' misurata la DISTINZIONE
 *                         (uno finisce in `c36`, l'altro in w0) e persino
 *                         l'ordine di DICHIARAZIONE (scoperta F4-S1); di
 *                         `len`, `idx0` e `idx1` e' misurata l'ESISTENZA come
 *                         locali, non il nome.
 */

/*
 * ===========================================================================
 * mp_compare_cdc_result -- INCORPORATA SETTE VOLTE, riga 2724
 * ===========================================================================
 * Confronta un vettore di dati contro due vettori di soglia, nodo per nodo, e
 * torna 0 oppure -1.
 *
 * I QUATTRO PARAMETRI. `data` e' l'unico che il binario tipa direttamente:
 * "b86b686a ldr"@0xffffff8008a662e4 lo legge a quattro byte con indice esteso
 * CON SEGNO, quindi `int *` indicizzato da un `int`. `max` e `min` sono i due
 * vettori allocati dal chiamante (x19 e x20). `index` serve perche' il corpo
 * legge `mp_items[index].c16` ("b94002a9 ldr"@0xffffff8008a660b4). L'ORDINE
 * dei quattro NON e' deciso dal binario -- la funzione e' incorporata -- ed e'
 * una SCELTA.
 *
 * IL VALORE DI RITORNO E' 0 oppure -1: "2a1f03f8 mov"@0xffffff8008a66310 e
 * "12800018 mov"@0xffffff8008a662cc. Che sia un `int` e non un `bool` lo dice
 * il chiamante, che ne prova il bit 31.
 *
 * IL CONTROLLO SUL PUNTATORE E' `IS_ERR` PIU' `NULL`, IN QUEST'ORDINE:
 * "b140047f cmn"@0xffffff8008a660a8 poi
 * "b4001023 cbz"@0xffffff8008a660b0. E' lo stesso ordine misurato in
 * `mp_compare_cdc_show_result` e in `mp_test_data_sort_average`.
 *
 * IL `%p` E' IL PUNTATORE STESSO: nel blocco a 0xffffff8008a662b4 la `printk`
 * non prepara x3, perche' x3 GIA' contiene `data`.
 *
 * DUE RAMI, SCELTI DA `mp_items[index].c16 == 9`
 * ("7100253f cmp"@0xffffff8008a660b8 + "540010c1 b.ne"@0xffffff8008a660bc):
 *   - con 9 si prova SOLO il minimo: "b86a686b ldr"@0xffffff8008a660d0 (dato),
 *     "b86a6a8a ldr"@0xffffff8008a660d4 (min), "6b0a017f cmp"@0xffffff8008a660d8
 *     e "54000f8b b.lt"@0xffffff8008a660dc;
 *   - altrimenti si provano tutti e due, PRIMA il massimo:
 *     "b86b686a ldr"@0xffffff8008a662e4, "b86b6a6b ldr"@0xffffff8008a662e8,
 *     "6b0b015f cmp"@0xffffff8008a662ec, "54fffeec b.gt"@0xffffff8008a662f0,
 *     poi "b8697a8b ldr"@0xffffff8008a662f8,
 *     "6b0b015f cmp"@0xffffff8008a662fc, "54fffe6b b.lt"@0xffffff8008a66300.
 * Il limite dei due cicli e' `core_mp.c256`, riletto a ogni giro.
 *
 * NOTA SUL LETTERALE. "\x013ILITEK: (%s, %d): The data of test item is null (%p)\n"
 * @0xffffff80092451d1 e' lo STESSO letterale che `mp_compare_cdc_show_result`
 * usa: nel binario esiste una copia sola, e i due siti di chiamata la
 * condividono. Il `__func__` invece e' diverso
 * ("mp_compare_cdc_result"@0xffffff8009245209 contro
 * "mp_compare_cdc_show_result"@0xffffff80092458d1) ed e' quello che prova che
 * si tratta di due funzioni distinte.
 */
static int mp_compare_cdc_result(int index, int *data, int *max, int *min)
{
	int i;

	/* "b140047f cmn"@0xffffff8008a660a8 + "b4001023 cbz"@0xffffff8008a660b0 */
	if (IS_ERR(data) || data == NULL) {
		/* "\x013ILITEK: (%s, %d): The data of test item is null (%p)\n"@0xffffff80092451d1,
		 * __func__ = "mp_compare_cdc_result"@0xffffff8009245209;
		 * "52815482 mov"@0xffffff8008a662c4 -- __LINE__ = 2724 */
#line 2724
		ILI_ERR("The data of test item is null (%p)\n", data);
		/* "12800018 mov"@0xffffff8008a662cc */
		return -1;
	}

	/* "b94002a9 ldr"@0xffffff8008a660b4 + "7100253f cmp"@0xffffff8008a660b8
	 * + "540010c1 b.ne"@0xffffff8008a660bc */
	if (mp_items[index].c16 == 9) {
		for (i = 0; i < core_mp.c256; i++) {
			/* "b86a686b ldr"@0xffffff8008a660d0 +
			 * "b86a6a8a ldr"@0xffffff8008a660d4 +
			 * "6b0a017f cmp"@0xffffff8008a660d8 +
			 * "54000f8b b.lt"@0xffffff8008a660dc */
			if (data[i] < min[i])
				return -1;
		}
	} else {
		for (i = 0; i < core_mp.c256; i++) {
			/* "b86b686a ldr"@0xffffff8008a662e4 +
			 * "b86b6a6b ldr"@0xffffff8008a662e8 +
			 * "6b0b015f cmp"@0xffffff8008a662ec +
			 * "54fffeec b.gt"@0xffffff8008a662f0, poi
			 * "b8697a8b ldr"@0xffffff8008a662f8 +
			 * "6b0b015f cmp"@0xffffff8008a662fc +
			 * "54fffe6b b.lt"@0xffffff8008a66300 */
			if (data[i] > max[i] || data[i] < min[i])
				return -1;
		}
	}

	/* "2a1f03f8 mov"@0xffffff8008a66310 */
	return 0;
}

/*
 * ===========================================================================
 * mp_comp_result_before_retry -- 0xffffff8008a65f8c, 3320 byte, `t`
 * ===========================================================================
 * IL PARAMETRO E' UN `int`, l'indice dell'elemento di collaudo:
 * "9b28581b smaddl"@0xffffff8008a65fb8 moltiplica w0 per 152 CON SEGNO e ci
 * somma la base della tavola, e "93407c17 sxtw"@0xffffff8008a65fc4 lo estende
 * con segno per riusarlo. IL VALORE DI RITORNO E' UN `int`:
 * "2a1503e0 mov"@0xffffff8008a66184 e il chiamante ne prova il bit 31
 * ("36f80160 tbz"@0xffffff8008a66ce4, dentro `mp_do_retry`).
 *
 * `noinline`, ED E' UNA DIVERGENZA DICHIARATA (F4-D1). Di fabbrica la funzione
 * ha DUE chiamanti -- "940017e0 bl"@0xffffff8008a6000c, dentro
 * `ilitek_tddi_mp_test_main`, e "97fffcab bl"@0xffffff8008a66ce0, dentro
 * `mp_do_retry` -- e con due siti di chiamata clang non la incorpora. Qui
 * `ilitek_tddi_mp_test_main` NON e' scritta (e' l'ultimo simbolo che manca al
 * gruppo), quindi il sito di chiamata e' uno solo e clang la incorpora dentro
 * `mp_do_retry`, che passa da 156 a 3448 byte e fa sparire il simbolo. E' lo
 * stesso artificio di `__used`, gia' usato in questo file per le funzioni che
 * nessuno chiama: si toglie quando il secondo chiamante sara' scritto.
 *
 * L'USCITA ANTICIPATA: "7100291f cmp"@0xffffff8008a65fc8 confronta
 * `mp_items[index].c16` (letto da "b8410ea8 ldr"@0xffffff8008a65fc0, che
 * lascia anche in x21 l'indirizzo del campo) con 10, e in quel caso la
 * funzione torna `mp_items[index].c36` ("b9402515 ldr"@0xffffff8008a65fd8)
 * senza allocare niente e SENZA riscrivere `c36`.
 *
 * I DUE VETTORI DI SOGLIA SONO `kcalloc`, per gli stessi due argomenti
 * misurati in `mp_test_data_sort_average`: le bandiere 0x14080c0
 * ("52901801 mov"@0xffffff8008a65fec + "72a02801 movk"@0xffffff8008a65ff4) e
 * la prova di traboccamento "37f80848 tbnz"@0xffffff8008a65fe8 sul bit 31 del
 * conteggio, che e' `core_mp.c256` letto CON SEGNO
 * ("b98c0348 ldrsw"@0xffffff8008a65fe4). La chiamata e'
 * "97dfa8e4 bl"@0xffffff8008a65ff8 e il controllo
 * "b140041f cmn"@0xffffff8008a66000 + "b4000873 cbz"@0xffffff8008a66008.
 *
 * I DUE PUNTATORI PARTONO DA `NULL`, e lo prova il ramo d'errore della PRIMA
 * allocazione: "aa1f03f4 mov"@0xffffff8008a66138 azzera esplicitamente il
 * secondo prima di andare a liberare. Senza inizializzatore quel valore
 * sarebbe indefinito e clang non emetterebbe la scrittura.
 *
 * IL CODICE DI RITORNO -107 (0xffffff95, "12800d55 mov"@0xffffff8008a66108).
 * E' scritto come numero e non come nome: 107 e' `ENOTCONN` in Linux, ma il
 * binario da' il NUMERO, non il nome, e chiamarlo `-ENOTCONN` sarebbe
 * un'affermazione in piu' di quanto l'evidenza permetta (regola 4).
 *
 * DUE RAMI D'ERRORE DIVERSI PER LO STESSO MESSAGGIO, ED E' DELLA FABBRICA.
 * Il messaggio "This test item (%s) has no data inside its buffer" compare a
 * due righe: alla 2773 ("52815aa2 mov"@0xffffff8008a662a8), dentro il ramo
 * `c16 == 4`, e alla 2796 ("52815d82 mov"@0xffffff8008a66280), nell'altro
 * ramo. I due NON tornano lo stesso valore: il primo salta a
 * 0xffffff8008a6615c, che mette `ret = -107`, il secondo mette
 * `ret = 0` ("2a1f03f5 mov"@0xffffff8008a66288). E' un'asimmetria della
 * fabbrica e si riproduce.
 *
 * IL VALORE DI RITORNO NORMALE E' ZERO. Su ogni cammino riuscito
 * `ret` vale 0, e `mp_do_retry` riprova solo se il ritorno e' negativo: di
 * fabbrica, quindi, la ripetizione scatta SOLO quando un'allocazione fallisce
 * o quando il ramo `c16 == 4` non trova i dati. Che questo sia il
 * comportamento voluto non e' deciso dal binario; che sia il comportamento
 * EMESSO si', ed e' quello che qui e' riprodotto (regola 7).
 *
 * IL SECONDO ESITO, `res`, non e' il valore di ritorno ma quello che finisce
 * in `mp_items[index].c36` ("b9002518 str"@0xffffff8008a66180, quattro byte,
 * scostamento 36).
 *
 * IL RAMO `c16 == 4` ("b94002a8 ldr"@0xffffff8008a66034 +
 * "7100111f cmp"@0xffffff8008a66038) lavora sui sei puntatori di `core_mp`:
 * prova `rx_delta_buf` ("f9461d08 ldr"@0xffffff8008a66044, pagina+3128) e
 * `tx_delta_buf` ("f9461908 ldr"@0xffffff8008a66058, pagina+3120) --
 * IN QUEST'ORDINE, che e' quello del binario e non e' distinguibile dalla
 * misura di dimensione (i due sono carichi puri e clang puo' scambiarli): e'
 * una DIVERGENZA POTENZIALE dichiarata, F4-D2 -- poi riempie le soglie con
 * `core_mp.c288`/`c292` ("2944354c ldp"@0xffffff8008a66084) e confronta
 * `tx_max_buf` e `tx_min_buf`; poi le RIRIEMPIE con `c296`/`c300`
 * ("2945354c ldp"@0xffffff8008a663a0) e confronta `rx_max_buf` e `rx_min_buf`.
 *
 * L'ALTRO RAMO prova i quattro puntatori dell'elemento, nell'ordine
 * `buf` ("f8460d28 ldr"@0xffffff8008a661b0, +96),
 * `max_buf` ("f8468d48 ldr"@0xffffff8008a661c8, +104),
 * `min_buf` ("f8470f28 ldr"@0xffffff8008a661e0, +112),
 * `result_buf` ("f8458f08 ldr"@0xffffff8008a661f8, +88); poi riempie le soglie
 * dai due vettori di riferimento se `mp_items[index].c21 == 1`
 * ("3940550b ldrb"@0xffffff8008a66210 + "7100057f cmp"@0xffffff8008a66218,
 * con "9101e10c add"@0xffffff8008a66234 e
 * "9102010d add"@0xffffff8008a66238 che indirizzano +0x78 e +0x80) oppure dai
 * due `int` a +28 e +40 ("9100710c add"@0xffffff8008a66550 e
 * "9100a10d add"@0xffffff8008a66554).
 *
 * LA SCELTA FRA ORDINAMENTO E CONFRONTO DIRETTO:
 * "b940356b ldr"@0xffffff8008a66584 legge `c52`,
 * "3400008b cbz"@0xffffff8008a66588 lo prova contro zero e
 * "7100217f cmp"@0xffffff8008a66590 prova `c16` contro 8. Si ordina solo
 * quando `c52` non e' zero E `c16` non e' 8.
 *
 * COME SI COMPONE `res`. Il primo confronto di ciascuna catena ASSEGNA
 * ("2a1f03f8 mov"@0xffffff8008a66310 mette 0 sul cammino riuscito), i
 * successivi assegnano -1 SOLO se falliscono (sul cammino riuscito il registro
 * resta com'e'). E' la differenza fra `res = ...` e `if (... < 0) res = -1;`,
 * e la si legge dalla presenza o dall'assenza del `mov wzr` sul ramo riuscito.
 *
 * L'ORDINE DI DICHIARAZIONE DI `res` E `ret` VALE QUATTRO BYTE (scoperta
 * F4-S1). Dichiarando `ret` prima di `res`, il blocco d'errore della riga 2764
 * esce nell'ordine (`ret`, `res`) e clang riesce a fondere la coda del blocco
 * della riga 2796 con la seconda delle due istruzioni: la funzione misura
 * 3316. Dichiarando `res` prima di `ret` l'ordine si inverte, la fusione non
 * e' piu' possibile e la funzione misura 3320 ESATTI. Non e' l'ordine delle
 * ISTRUZIONI nel sorgente -- provato a scambiarle, non cambia niente -- ma
 * quello delle DICHIARAZIONI.
 */
static int mp_comp_result_before_retry(int index)
{
	int i;
	int res = 0;
	int ret = 0;
	int *thr_max = NULL;
	int *thr_min = NULL;

	/* "b8410ea8 ldr"@0xffffff8008a65fc0 + "7100291f cmp"@0xffffff8008a65fc8;
	 * "b9402515 ldr"@0xffffff8008a65fd8 */
	if (mp_items[index].c16 == 10)
		return mp_items[index].c36;

	/* "b98c0348 ldrsw"@0xffffff8008a65fe4 +
	 * "37f80848 tbnz"@0xffffff8008a65fe8 (la prova di traboccamento di
	 * `kmalloc_array`) + "97dfa8e4 bl"@0xffffff8008a65ff8 */
	thr_max = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a66000 + "b4000873 cbz"@0xffffff8008a66008 */
	if (IS_ERR(thr_max) || thr_max == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate threshold FRAME buffer\n"@0xffffff800924512f,
		 * __func__ = "mp_comp_result_before_retry"@0xffffff800924516e;
		 * "52815882 mov"@0xffffff8008a66100 -- __LINE__ = 2756 */
#line 2756
		ILI_ERR("Failed to allocate threshold FRAME buffer\n");
		/* "12800d55 mov"@0xffffff8008a66108 (-107) +
		 * "12800018 mov"@0xffffff8008a6610c (-1) */
		ret = -107;
		res = -1;
		goto out;
	}

	thr_min = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	if (IS_ERR(thr_min) || thr_min == NULL) {
		/* stesso letterale, riga diversa:
		 * "52815982 mov"@0xffffff8008a66154 -- __LINE__ = 2764 */
#line 2764
		ILI_ERR("Failed to allocate threshold FRAME buffer\n");
		ret = -107;
		res = -1;
		goto out;
	}

	/* "b94002a8 ldr"@0xffffff8008a66034 + "7100111f cmp"@0xffffff8008a66038 */
	if (mp_items[index].c16 == 4) {
		/* L'ORDINE E' QUELLO DEL BINARIO (divergenza F4-D2):
		 * "f9461d08 ldr"@0xffffff8008a66044 (rx, pagina+3128) prima di
		 * "f9461908 ldr"@0xffffff8008a66058 (tx, pagina+3120) */
		if (IS_ERR(core_mp.rx_delta_buf) || core_mp.rx_delta_buf == NULL ||
		    IS_ERR(core_mp.tx_delta_buf) || core_mp.tx_delta_buf == NULL) {
			/* "\x013ILITEK: (%s, %d): This test item (%s) has no data inside its buffer\n"@0xffffff800924518a;
			 * "52815aa2 mov"@0xffffff8008a662a8 -- __LINE__ = 2773.
			 * Il nome dell'elemento e' il quarto argomento,
			 * "f9400363 ldr"@0xffffff8008a6626c nell'altro ramo */
#line 2773
			ILI_ERR("This test item (%s) has no data inside its buffer\n", mp_items[index].c0);
			ret = -107;
			res = -1;
			goto out;
		}

		/* "2944354c ldp"@0xffffff8008a66084: i due campi sono letti in
		 * coppia e RILETTI a ogni giro, perche' le due scritture
		 * possono aliasarli */
		for (i = 0; i < core_mp.c256; i++) {
			thr_max[i] = core_mp.c288;
			thr_min[i] = core_mp.c292;
		}

		/* "f9462123 ldr"@0xffffff8008a660a4 (pagina+3136) */
		res = mp_compare_cdc_result(index, core_mp.tx_max_buf,
					    thr_max, thr_min);
		/* "f9462503 ldr"@0xffffff8008a66318 (pagina+3144): qui NON c'e'
		 * nessun `mov wzr` sul ramo riuscito, quindi `res` non viene
		 * riassegnato quando il confronto passa */
		if (mp_compare_cdc_result(index, core_mp.tx_min_buf,
					  thr_max, thr_min) < 0)
			res = -1;

		/* "2945354c ldp"@0xffffff8008a663a0: la SECONDA coppia di campi,
		 * a +296 e +300 */
		for (i = 0; i < core_mp.c256; i++) {
			thr_max[i] = core_mp.c296;
			thr_min[i] = core_mp.c300;
		}

		/* "f9462923 ldr"@0xffffff8008a663c0 (pagina+3152) */
		if (mp_compare_cdc_result(index, core_mp.rx_max_buf,
					  thr_max, thr_min) < 0)
			res = -1;
		/* "f9462d03 ldr"@0xffffff8008a6642c (pagina+3160) */
		if (mp_compare_cdc_result(index, core_mp.rx_min_buf,
					  thr_max, thr_min) < 0)
			res = -1;
	} else {
		/* "f8460d28 ldr"@0xffffff8008a661b0 (+96),
		 * "f8468d48 ldr"@0xffffff8008a661c8 (+104),
		 * "f8470f28 ldr"@0xffffff8008a661e0 (+112),
		 * "f8458f08 ldr"@0xffffff8008a661f8 (+88): quattro prove
		 * `IS_ERR` piu' zero, in quest'ordine */
		if (IS_ERR(mp_items[index].buf) || mp_items[index].buf == NULL ||
		    IS_ERR(mp_items[index].max_buf) || mp_items[index].max_buf == NULL ||
		    IS_ERR(mp_items[index].min_buf) || mp_items[index].min_buf == NULL ||
		    IS_ERR(mp_items[index].result_buf) || mp_items[index].result_buf == NULL) {
			/* stesso letterale della riga 2773, riga diversa:
			 * "52815d82 mov"@0xffffff8008a66280 -- __LINE__ = 2796.
			 * QUI `ret` RESTA ZERO ("2a1f03f5 mov"@0xffffff8008a66288),
			 * mentre alla riga 2773 vale -107: e' l'asimmetria della
			 * fabbrica */
#line 2796
			ILI_ERR("This test item (%s) has no data inside its buffer\n", mp_items[index].c0);
			res = -1;
			goto out;
		}

		/* "3940550b ldrb"@0xffffff8008a66210 +
		 * "7100057f cmp"@0xffffff8008a66218 */
		if (mp_items[index].c21 == 1) {
			/* "9101e10c add"@0xffffff8008a66234 (+0x78) e
			 * "9102010d add"@0xffffff8008a66238 (+0x80): i due
			 * puntatori sono RILETTI a ogni giro */
			for (i = 0; i < core_mp.c256; i++) {
				thr_max[i] = mp_items[index].bench_mark_max[i];
				thr_min[i] = mp_items[index].bench_mark_min[i];
			}
		} else {
			/* "9100710c add"@0xffffff8008a66550 (+0x1c) e
			 * "9100a10d add"@0xffffff8008a66554 (+0x28) */
			for (i = 0; i < core_mp.c256; i++) {
				thr_max[i] = mp_items[index].c28;
				thr_min[i] = mp_items[index].c40;
			}
		}

		/* "b940356b ldr"@0xffffff8008a66584 +
		 * "3400008b cbz"@0xffffff8008a66588 +
		 * "7100217f cmp"@0xffffff8008a66590 */
		if (mp_items[index].c52 != 0 && mp_items[index].c16 != 8) {
			/* "f9400139 ldr"@0xffffff8008a6660c (buf) e
			 * "f9400309 ldr"@0xffffff8008a66610 (result_buf) sono i
			 * due argomenti preparati subito prima del corpo
			 * incorporato */
			mp_test_data_sort_average(mp_items[index].buf, index,
						  mp_items[index].result_buf);
			/* "f9400303 ldr"@0xffffff8008a66ba4 */
			res = mp_compare_cdc_result(index,
						    mp_items[index].result_buf,
						    thr_max, thr_min);
		} else {
			/* "f9400143 ldr"@0xffffff8008a66598 */
			res = mp_compare_cdc_result(index,
						    mp_items[index].max_buf,
						    thr_max, thr_min);
			/* "f9400323 ldr"@0xffffff8008a666a0 */
			if (mp_compare_cdc_result(index,
						  mp_items[index].min_buf,
						  thr_max, thr_min) < 0)
				res = -1;
		}
	}

out:
	/* PER INDIRIZZO, non per valore: vedi il cappello di `ipio_kfree`.
	 * "aa1303e0 mov"@0xffffff8008a66164 + "97dfa992 bl"@0xffffff8008a66168,
	 * poi "b4000074 cbz"@0xffffff8008a6616c per il secondo */
	ipio_kfree((void **)&thr_max);
	ipio_kfree((void **)&thr_min);

	/* "b9002518 str"@0xffffff8008a66180 (quattro byte, scostamento 36) +
	 * "2a1503e0 mov"@0xffffff8008a66184 */
	mp_items[index].c36 = res;
	return ret;
}

/*
 * ===========================================================================
 * LE COSTANTI DEL LOTTO F5 -- i VALORI sono misurati, i NOMI sono SCELTI
 * ===========================================================================
 */
/* "5280065c mov"@0xffffff8008a5e8a0 e "f100cb9f cmp"@0xffffff8008a5ef38 */
#define MP_TEST_ITEM			50
/* "5280e113 mov"@0xffffff8008a600d4 e "711c211f cmp"@0xffffff8008a601a4 */
#define PARSER_MAX_KEY_NUM		1800
/* "f118013f cmp"@0xffffff8008a601ec */
#define PARSER_MAX_CFG_BUF		1536
/* "7101951f cmp"@0xffffff8008a605a4 (#0x65 come limite ESCLUSIVO) */
#define PARSER_MAX_KEY_NAME_LEN		100
/* "711f445f cmp"@0xffffff8008a604c4 (#0x7d1 come limite ESCLUSIVO) */
#define PARSER_MAX_KEY_VALUE_LEN	2000
/* "f10192ff cmp"@0xffffff8008a5ef48 */
#define PARSER_MAX_SECTION_NUM		100
/* "320c03e0 orr"@0xffffff8008a60054 */
#define CSV_FILE_SIZE			(1024 * 1024)
/* "321403fb orr"@0xffffff8008a607e8 (0x1000) e
 * "321f2be1 orr"@0xffffff8008a607ac (0xffe = 0x1000 - 2) */
#define APK_BUF_SIZE			4096

/*
 * "2.0.6.0.191122"@0xffffff800923ed58, citata due volte -- da
 * "91356063 add"@0xffffff8008a5e76c e da "91356063 add"@0xffffff8008a609b0.
 * Il NOME della macro e' SCELTO.
 */
#define DRIVER_VERSION			"2.0.6.0.191122"

/*
 * 0xffffff800a0fc963, cioe' undici byte dopo `ilitek_dbg_en`. Un byte:
 * "392f8e88 strb"@0xffffff8008a5edb4 lo scrive e
 * "392f8e9f strb"@0xffffff8008a5ed28 lo azzera. NESSUNA delle funzioni
 * scritte lo LEGGE: sta qui perche' `mp_get_timing_info` lo scrive, e chi lo
 * legge sta in un altro gruppo. NOME SCELTO; il lotto di merge dovra'
 * decidere in quale file definirlo (voce del delta di header).
 */
u8 ilitek_mp_ddi_mode;

/*
 * `ipio_vfree`: la gemella di `ipio_kfree` (lotto F4) per la memoria
 * virtuale. La FORMA e' misurata -- "f9400680 ldr"@0xffffff8008a5f140,
 * "b4000060 cbz"@0xffffff8008a5f144, "97df8c3c bl"@0xffffff8008a5f148 verso
 * <vfree>, "f900069f str"@0xffffff8008a5f14c -- il NOME e' scelto, per
 * simmetria con quello gia' in questo file.
 *
 * QUANTE VOLTE, MISURATO. Il rapporto del lotto F5 scriveva che questa
 * sequenza «compare cinquanta volte»: non si riproduce, e cinquanta e' il
 * numero di giri del ciclo in cui una delle due sta, non un conto di
 * occorrenze. Dentro `ilitek_tddi_mp_test_main` (0xffffff8008a5e618, 14704
 * byte) i `bl <vfree>` sono SEI e la forma completa `ldr`+`cbz`+`bl`+`str
 * xzr` compare DUE volte, a 0xffffff8008a5f148 e a 0xffffff8008a5f1e8. Gli
 * altri quattro siti -- 0xffffff8008a5ec14, 0xffffff8008a5f1cc,
 * 0xffffff8008a5f1d8, 0xffffff8008a60778 -- hanno la forma
 * `cbz`+`mov x0,`+`bl` SENZA lo `str xzr`, cioe' sono `vfree` dirette e non
 * questa funzione. Il conto sta in fondo, ed e' quello che giustifica la
 * scelta di scrivere `ipio_vfree` come funzione separata: DUE siti, non
 * cinquanta. Che due bastino a giustificarla e' una SCELTA, non una misura.
 */
static void ipio_vfree(void **mem)
{
	if (*mem != NULL) {
		vfree(*mem);
		*mem = NULL;
	}
}

/*
 * ===========================================================================
 * LOTTO F5 -- `ilitek_tddi_mp_test_main`, 0xffffff8008a5e618, 14704 byte, `T`
 * ===========================================================================
 * UN SOLO SIMBOLO, TREDICI FUNZIONI C. Il simbolo emesso e' uno; dentro ci
 * sono, incorporate dal compilatore, DODICI funzioni C separate che la
 * ricognizione (`scout-ilitek-F/RIASSUNTO.md` §3) elenca con le loro righe di
 * fabbrica, lette dal terzo argomento delle `printk`:
 *
 *   ilitek_tddi_mp_init_item   3113..3124
 *   ilitek_tddi_mp_ini_parser   816..883
 *   parser_get_ini_phy_data     663..796
 *   parser_ini_str_trim_r       625
 *   parser_get_int_data         566
 *   mp_get_timing_info         2620
 *   mp_sort_item               3363
 *   mp_test_run                3252..3272
 *   mp_print_csv_cdc_cmd       1124..1140
 *   mp_show_result             2858..3076
 *   mp_copy_ret_to_apk         3326..3337
 *   mp_test_free               3282
 *
 * piu' `parser_get_tdf`, che NESSUN messaggio nomina: la sua esistenza come
 * funzione separata e' una SCELTA di questo lotto, la sua FORMA e' misurata
 * (vedi il suo cappello).
 *
 * ===========================================================================
 * IL FATTO CHE CAMBIA IL PIANO DEL PROGETTO -- MISURATO, NON SUPPOSTO
 * ===========================================================================
 * La consegna di questo lotto diceva: «MANCANO DUE SOLI SIMBOLI PERCHE' IL
 * DRIVER SI LINCHI», cioe' `ilitek_tddi_mp_test_main` e
 * `mp_comp_result_before_retry`. Il lotto F4 ha chiuso il secondo; questo
 * lotto chiude il primo. MA IL DRIVER NON SI LINCA LO STESSO, e il motivo e'
 * dentro questa funzione: `ilitek_tddi_mp_init_item` INSTALLA nei cinquanta
 * elementi di collaudo i puntatori alle SETTE funzioni di prova, e CINQUE di
 * esse non sono scritte da nessuno. Sono i cinque `adrp`+`add` dello `switch`
 * a undici vie:
 *
 *   "90000033 adrp"@0xffffff8008a5e894 + "91035273 add"@0xffffff8008a5e8b4
 *       -> 0xffffff8008a620d4 = mutual_test        (4584 byte)
 *   "b0000028 adrp"@0xffffff8008a5ea08 + "910af108 add"@0xffffff8008a5ea0c
 *       -> 0xffffff8008a632bc = open_test_sp       (3932 byte)
 *   "d0000028 adrp"@0xffffff8008a5e938 + "91086108 add"@0xffffff8008a5e93c
 *       -> 0xffffff8008a64218 = open_test_cap      (1828 byte)
 *   "d0000028 adrp"@0xffffff8008a5e9b0 + "9124f108 add"@0xffffff8008a5e9b4
 *       -> 0xffffff8008a6493c = key_test           (1368 byte)
 *   "d0000028 adrp"@0xffffff8008a5ea14 + "913bb108 add"@0xffffff8008a5ea18
 *       -> 0xffffff8008a64eec = pin_test           ( 740 byte)
 *
 * Le altre due, `self_test` (0xffffff8008a64e94) e `st_test`
 * (0xffffff8008a64ec0), sono scritte dal lotto F3 e stanno in questo file.
 *
 * Le cinque mancanti sono DICHIARATE E NON DEFINITE qui sotto, come vuole la
 * regola 6: il link che fallisce e' l'esito onesto, e uno stub le farebbe
 * passare per scritte. Il conto degli irrisolti passa quindi da UNO
 * (`ilitek_tddi_mp_test_main`) a CINQUE, ed e' un progresso, non un
 * peggioramento: prima ne mancava uno da 14704 byte, adesso ne mancano cinque
 * per 12452 byte in tutto.
 *
 * ===========================================================================
 * LA RISPOSTA ALLA «DOMANDA APERTA SU parser_get_int_data» DEL LOTTO F1
 * ===========================================================================
 * Il lotto F1 lascio' agli atti tre fatti che diceva incompatibili fra loro e
 * concluse: «Nessuna forma naturale di `parser_get_int_data` -- e ne sono
 * state provate sei sulla carta -- produce tutti e tre i fatti insieme»,
 * bloccando cinque funzioni. La forma che li produce tutti e tre e' scritta
 * qui sotto, e il terzo fatto non e' una proprieta' di `parser_get_int_data`
 * ma del suo CHIAMANTE. Il dettaglio sta nel cappello della funzione.
 *
 * ===========================================================================
 * QUANTO E' MISURATO E QUANTO E' SCELTO -- LEGGERE PRIMA DEL CODICE
 * ===========================================================================
 * Questa e' la funzione piu' grande del progetto: 3676 istruzioni, il 38% del
 * gruppo F, con DUE salti indiretti e uno `switch` a undici vie su tabella di
 * salto. La ricostruzione NON e' esatta al byte e la sezione «LA MISURA» in
 * fondo dice di quanto sbaglia, funzione per funzione. Le divergenze
 * dichiarate stanno nel rapporto del lotto; qui, nel codice, ogni riga
 * derivata porta accanto la riga di disassemblato che la giustifica, e dove
 * il binario NON decide il commento lo dice con la parola SCELTA.
 */

/*
 * ===========================================================================
 * LE FUNZIONI DI COLLAUDO CHE NON SONO SCRITTE (regola 6) -- ORA UNA SOLA
 * ===========================================================================
 * Erano cinque; `open_test_cap` e' scritta dal lotto F3b, `open_test_sp` dal
 * lotto F3c, e `key_test` e `pin_test` dal lotto F3c-bis. Tutte e quattro
 * sono `static` perche' nella mappa sono `t`
 * ("ffffff8008a64218 t open_test_cap", "ffffff8008a632bc t open_test_sp",
 * "ffffff8008a6493c t key_test", "ffffff8008a64eec t pin_test").
 * AGGIORNAMENTO DEL LOTTO F6: NON NE RESTA NESSUNA. `mutual_test` e' scritta
 * in fondo a questo file ed e' `static` come le altre quattro, perche' nella
 * mappa e' `t` ("ffffff8008a620d4 t mutual_test"). La divergenza di legamento
 * che questa sezione dichiarava -- una `static` dichiarata, usata e mai
 * definita e' un errore di compilazione, quindi finche' non era scritta
 * doveva essere non-`static` -- E' CHIUSA.
 * ===========================================================================
 * Dichiarata qui e DEFINITA IN FONDO. La firma viene dal sito di chiamata,
 * che e' UNO SOLO e sta in `mp_do_retry` (gruppo F4):
 * "f9404aa8 ldr"@0xffffff8008a66cd0 legge il campo a +0x90 e
 * "d63f0100 blr"@0xffffff8008a66cd8 lo chiama con w0 = l'indice dell'elemento
 * ("2a1303e0 mov"@0xffffff8008a66cd4). Il tipo del ritorno NON e' provato:
 * `int` e' la stessa SCELTA gia' fatta per il campo `c144`.
 */
static int mutual_test(int index);
static int open_test_sp(int index);
static int open_test_cap(int index);
static int key_test(int index);
static int pin_test(int index);

/*
 * `str2hex`: 0xffffff8008a6b528, oltre il gruppo del flash. Dichiarata qui e
 * definita ALTROVE -- non e' un buco del progetto. CORREZIONE del 2026-08-22:
 * il rapporto del lotto F5 la elencava fra i simboli su cui il link fallisce
 * ("cinque funzioni di collaudo piu' `str2hex`"), e non e' vero: `str2hex` e'
 * definita in `ilitek_node.o` e NON compare fra gli irrisolti. Gli irrisolti
 * ILITEK sono CINQUE, non sei -- la misura sta nella sezione "IL LINK".
 * "94003231 bl"@0xffffff8008a5ec64 e
 * "94003169 bl"@0xffffff8008a5ef84 la chiamano con x0 = un `char *` e il
 * risultato e' confrontato a 32 bit con "6b48201f cmp"@0xffffff8008a5ec70.
 */
extern int str2hex(char *str);

/*
 * ===========================================================================
 * LE DUE STRUTTURE PUNTATE DA `idev` -- offset e larghezze MISURATI
 * ===========================================================================
 * `ilitek_tddi_mp_init_item` copia dentro `core_mp` undici valori presi da
 * due strutture che `idev` raggiunge con `idev->c48`
 * ("f9401909 ldr"@0xffffff8008a5e6d4) e `idev->c56`
 * ("f9401d0a ldr"@0xffffff8008a5e710). `ilitek.h` le dichiara `void *` e non
 * si tocca (e' un file condiviso): qui ci sono due strutture LOCALI, con i
 * soli campi che questo lotto legge. I NOMI SONO SCELTI e prendono l'offset
 * del campo di `idev` che le punta; le LARGHEZZE sono misurate una per una.
 */
struct ilitek_c48 {
	u8 c0;		/* "3940012a ldrb"@0xffffff8008a5e6f8 */
	u8 c1;		/* "3940052a ldrb"@0xffffff8008a5e700 */
	u16 c2;		/* "7940052a ldrh"@0xffffff8008a5e6f0, DUE byte */
	u8 c4[4];	/* riempimento dichiarato */
	int c8;		/* "b940092a ldr"@0xffffff8008a5e6e4 */
	u8 c12[36];	/* riempimento dichiarato */
	int c48;	/* prima parola di "2946252a ldp"@0xffffff8008a5e708 */
	int c52;	/* seconda parola della stessa `ldp` */
	u8 c56[8];	/* riempimento dichiarato */
	int c64;	/* "b9404129 ldr"@0xffffff8008a5e72c */
};

struct ilitek_c56 {
	int c0;		/* "b940014b ldr"@0xffffff8008a5e714 */
	u8 c4[32];	/* riempimento dichiarato */
	int c36;	/* "b9402549 ldr"@0xffffff8008a5e71c */
};

/*
 * ===========================================================================
 * I GLOBALI NUOVI DEL LOTTO F5 -- indirizzi e larghezze MISURATI
 * ===========================================================================
 * I NOMI SONO SCELTI (regola 5): nessuna stringa del binario li nomina.
 *
 * `mp_run`: 0xffffff800a0ff458 (il contatore) e 0xffffff800a0ff45c (gli
 * indici). Che il contatore stia PRIMA e' misurato -- lo scrive
 * "b90c0909 str"... no: lo legge "b9845903 ldrsw"@0xffffff8008a5eed0 e la
 * voce nuova va a "b900051c str"@0xffffff8008a5eeec, cioe' base + 4 +
 * 4*contatore ("8b030908 add"@0xffffff8008a5eee8). Il numero di voci, 50, e'
 * MISURATO dal confine: 0xffffff800a0ff45c + 50*4 = 0xffffff800a0ff524, che e'
 * esattamente l'indirizzo del buffer dell'ora ("91149000 add"@0xffffff8008a61cf4).
 *
 * DIFETTO DELLA FABBRICA, RIPRODOTTO (regola 7): il controllo e'
 * "7100c87f cmp"@0xffffff8008a5eed4 + "5400be0c b.gt"@0xffffff8008a5eed8,
 * cioe' «errore se contatore > 50». Con contatore == 50 la scrittura va a
 * `idx[50]`, che e' UN ELEMENTO OLTRE la fine e cade dentro il buffer
 * dell'ora. E' uno scarto di uno vero, e si riproduce.
 */
struct ilitek_mp_run {
	int num;
	int idx[50];
};
struct ilitek_mp_run mp_run;

/*
 * 0xffffff800a0fccf0. Il PASSO 100 e' misurato
 * ("52800c88 mov"@0xffffff8008a5ee3c con
 * "9b0826fa madd"@0xffffff8008a5ee4c, e "52800c95 mov"@0xffffff8008a605e4 con
 * "9b1522e0 madd"@0xffffff8008a605e8). Il NUMERO DI RIGHE, 100, e' misurato
 * dal confine: 0xffffff800a0fccf0 + 100*100 = 0xffffff800a0ff400, che e'
 * esattamente `ilitek_ini_num`. E' anche il limite del ciclo di ricerca
 * ("f10192ff cmp"@0xffffff8008a5ef48, #0x64).
 *
 * DUE DIFETTI DELLA FABBRICA, RIPRODOTTI (regola 7):
 *  - l'azzeramento copre 5000 byte su 10000
 *    ("52827102 mov"@0xffffff8008a60190 = 0x1388);
 *  - il controllo di capienza e'
 *    "f10196ff cmp"@0xffffff8008a605b0 + "5400cd62 b.cs"@0xffffff8008a605b4,
 *    cioe' «errore se indice >= 101»: con indice 100 la `strncpy` scrive
 *    cento byte a partire da 0xffffff800a0ff400, cioe' sopra
 *    `ilitek_ini_num` e `ilitek_ini_data`.
 */
char seq_item[100][100];

/*
 * 0xffffff800a0fcc70, subito DOPO `core_mp` (che finisce a
 * 0xffffff800a0fcc70 perche' e' lunga 368 byte) e subito PRIMA di `seq_item`:
 * la larghezza 128 e' quindi MISURATA dai due confini.
 * "9131c063 add"@0xffffff8008a607a8 lo passa a `snprintf` come argomento `%s`
 * e "9131c000 add"@0xffffff8008a61d10 come destinazione.
 */
char csv_path[128];

/*
 * 0xffffff800a0ff524. La larghezza 128 e' MISURATA dal secondo argomento
 * della `snprintf` che lo riempie: "321903e1 orr"@0xffffff8008a61cfc.
 */
char csv_time[128];

/*
 * ===========================================================================
 * SETTE PUNTATORI CONSECUTIVI, 0xffffff800a0ff420..0xffffff800a0ff450
 * ===========================================================================
 * SONO SETTE GLOBALI DISTINTI, NON UN VETTORE -- scoperta F3c-S1, e il lotto
 * F3 li dichiarava `int *mp_frame_buf[7]`. La prova e' una misura, non una
 * lettura: dove il binario legge DUE O PIU' di questi puntatori nello STESSO
 * blocco base, di fabbrica ogni lettura porta il suo `adrp` con lo
 * spiazzamento ripiegato dentro la `ldr`, e non c'e' nessuna base comune.
 *
 *   ffffff8008a63d78:	9000b4e9 	adrp	x9, ffffff800a0ff000
 *   ffffff8008a63d7c:	f9421920 	ldr	x0, [x9,#1072]
 *   ffffff8008a63d98:	9000b4e9 	adrp	x9, ffffff800a0ff000
 *   ffffff8008a63d9c:	f9421d20 	ldr	x0, [x9,#1080]
 *   ffffff8008a63db8:	9000b4e9 	adrp	x9, ffffff800a0ff000
 *   ffffff8008a63dbc:	f9422120 	ldr	x0, [x9,#1088]
 *
 * cioe' "9000b4e9 adrp"@0xffffff8008a63d78 + "f9421920 ldr"@0xffffff8008a63d7c,
 * "9000b4e9 adrp"@0xffffff8008a63d98 + "f9421d20 ldr"@0xffffff8008a63d9c e
 * "9000b4e9 adrp"@0xffffff8008a63db8 + "f9422120 ldr"@0xffffff8008a63dbc: SEI
 * istruzioni per tre letture. Con un VETTORE, lo stesso clang di fabbrica
 * (r353983c) emette invece UNA base `adrp`+`add` rimaterializzata e tre
 * `ldr` a spiazzamento 0/8/16 -- SETTE istruzioni per tre letture -- perche'
 * gli indici costanti di uno stesso oggetto si riducono a `base + k`, e
 * AArch64 non ammette un globale come base di modo d'indirizzamento
 * (`isLegalAddressingMode` rifiuta `BaseGV`), quindi la base viene
 * materializzata a parte.
 *
 * LA MISURA CHE LO DECIDE, sullo stesso identico file e con il compilatore di
 * fabbrica, cambiando SOLO la dichiarazione:
 *
 *   vettore `mp_frame_buf[7]`   open_test_sp 0xf6c = 3948   open_test_cap 0x728 = 1832
 *   sette globali distinti      open_test_sp 0xf5c = 3932   open_test_cap 0x724 = 1828
 *   fabbrica                                        3932                    1828
 *
 * Le due funzioni passano DA NON ESATTE A ESATTE AL BYTE nello stesso colpo, e
 * `ilitek_tddi_mp_test_main` cala di quattro byte (0x39f4 -> 0x39f0), cioe'
 * migliora anche lei. Le stesse tre letture in fila stanno anche in
 * `open_test_cap` ("f9422520 ldr"@0xffffff8008a646f4 e
 * "f9422920 ldr"@0xffffff8008a64710, ognuna col suo `adrp`).
 *
 * I NOMI. Cinque vengono dal binario, due no. Ogni nome preso dal binario e'
 * il nome che il ramo d'errore della SUA allocazione stampa, ed e' lo stesso
 * passo d'inferenza gia' accettato per i sei puntatori di
 * `struct ilitek_mp_item`.
 *
 *   0xffffff800a0ff420  NOME SCELTO (`mp_frame_buf0`). Riempito da
 *       `allnode_mutual_cdc_data`, incorporata in `mutual_test`
 *       ("f9021100 str"@0xffffff8008a628bc); il suo ramo d'errore stampa
 *       "\x013ILITEK: (%s, %d): Failed to allocate FrameBuffer mem (%ld)\n"@0xffffff8009244004,
 *       che e' CONDIVISO con quello di 0xffffff800a0ff428 e quindi non
 *       distingue i due. Nessuna funzione scritta lo legge.
 *   0xffffff800a0ff428  NOME SCELTO (`mp_frame_buf1`). Riempito da `key_test`
 *       ("f9021720 str"@0xffffff8008a64cd8), e il suo ramo d'errore porta LA
 *       STESSA stringa. Nessuna funzione scritta lo legge.
 *   0xffffff800a0ff430  NOME DAL BINARIO (`frame1_cbk700`): l'allocazione e'
 *       "f9021aa0 str"@0xffffff8008a636e4 e il ramo d'errore stampa
 *       "\x013ILITEK: (%s, %d): Failed to allocate frame1_cbk700 buffer\n"@0xffffff800924415e.
 *   0xffffff800a0ff438  NOME DAL BINARIO (`frame1_cbk250`): allocazione
 *       "f9021ee0 str"@0xffffff8008a63714, errore
 *       "\x013ILITEK: (%s, %d): Failed to allocate frame1_cbk250 buffer\n"@0xffffff800924419b.
 *   0xffffff800a0ff440  NOME IN PARTE SCELTO (`frame1_cbk200`): l'allocazione
 *       e' "f9022380 str"@0xffffff8008a63744, ma il suo ramo d'errore dice
 *       solo "\x013ILITEK: (%s, %d): Failed to allocate cbk buffer\n"@0xffffff80092441d8.
 *       Il "200" e il prefisso "frame1" vengono dalle due ETICHETTE che lo
 *       accompagnano -- "cbk 200"@0xffffff8009244408 nel dump grezzo e
 *       "frame1 cbk200"@0xffffff8009245502 nel CSV -- non dal nome della
 *       variabile: e' un passo d'inferenza in piu' degli altri quattro.
 *   0xffffff800a0ff448  NOME DAL BINARIO (`cap_dac`): allocazione
 *       "f9022680 str"@0xffffff8008a64434, errore
 *       "\x013ILITEK: (%s, %d): Failed to allocate cap_dac buffer\n"@0xffffff8009244585.
 *   0xffffff800a0ff450  NOME DAL BINARIO (`cap_raw`): allocazione
 *       "f9022aa0 str"@0xffffff8008a64464, errore
 *       "\x013ILITEK: (%s, %d): Failed to allocate cap_raw buffer\n"@0xffffff80092445bc.
 *
 * L'ORDINE in cui sono dichiarati qui e' l'ordine degli INDIRIZZI, che e'
 * misurato; che il compilatore li disponga poi in `.bss` in quest'ordine e'
 * una conseguenza, non una misura sul nostro oggetto.
 */
int *mp_frame_buf0;
int *mp_frame_buf1;
int *frame1_cbk700;
int *frame1_cbk250;
int *frame1_cbk200;
int *cap_dac;
int *cap_raw;

/*
 * ===========================================================================
 * parser_get_int_data -- riga 566, incorporata in CINQUE funzioni
 * ===========================================================================
 * QUESTA E' LA RISPOSTA ALLA DOMANDA APERTA DEL LOTTO F1. I tre fatti che il
 * lotto F1 dichiarava incompatibili sono:
 *
 *   1. la `snprintf` e' chiamata INCONDIZIONATAMENTE dopo
 *      `parser_get_ini_key_value`;
 *   2. il chiamante entra nel ramo d'errore SOLO SE il ritorno del parser e'
 *      >= 0 E quello della `snprintf` e' < 0;
 *   3. sul ramo d'errore l'argomento `%d` del messaggio del CHIAMANTE e' la
 *      costante ZERO su tutti i cammini.
 *
 * I fatti 1 e 2 sono proprieta' di QUESTA funzione e li produce la forma
 * scritta qui sotto: il valore reso e' `(ret < 0) ? 0 : len`, e in questo
 * lotto il compilatore lo emette come un solo `csel` --
 * "7100027f cmp"@0xffffff8008a5ed64 + "1a80b3e2 csel"@0xffffff8008a5ed68
 * (w2 = (w19 < 0) ? wzr : w0) + "37f813c2 tbnz"@0xffffff8008a5ed6c. Negli
 * altri quattro siti di incorporazione lo stesso valore e' emesso come i due
 * `tbnz` consecutivi che il lotto F1 aveva letto. Sono la stessa espressione:
 * il `csel` e' la forma senza salto, i due `tbnz` la forma con.
 *
 * IL FATTO 3 NON E' UNA PROPRIETA' DI QUESTA FUNZIONE, ed e' questo che
 * bloccava il conto. Il messaggio «Failed to parse PV54 command, ret = %d»
 * sta in `allnode_open_cdc_data`, non qui, e il suo argomento e' una VARIABILE
 * DEL CHIAMANTE che su quel cammino vale zero -- non il valore reso da
 * `parser_get_int_data`. Il compilatore lo sa perche' vede il chiamante per
 * intero. Che sia cosi' lo prova questo lotto per via indipendente: il sito
 * di incorporazione di riga 1124 dentro `mp_print_csv_cdc_cmd` stampa
 * `%s`, non `%d`, e non ha nessuna variabile a zero da esibire.
 *
 * QUATTRO ARGOMENTI, e il quarto e' MISURATO: la dimensione passata a
 * `snprintf` cambia da sito a sito -- 0x200 = 512 in `mp_test_run`
 * ("321703e1 orr"@0xffffff8008a5f258), 0x100 = 256 in `mp_get_timing_info`
 * ("321803e1 orr"@0xffffff8008a5ed5c), 0x80 = 128 in `mp_print_csv_cdc_cmd`
 * ("321903e1 orr"@0xffffff8008a60c5c). Il buffer intermedio e' invece SEMPRE
 * di 512 byte ("321703e2 orr"@0xffffff8008a5f298 e gli altri siti), e quello
 * e' una costante del corpo.
 *
 * IL CONTROLLO SUI PUNTATORI e' scritto a tre termini perche' a tre termini e'
 * il solo modo di NON affermare piu' del misurato: nei siti di questo lotto
 * il compilatore ne prova UNO solo ("b4000193 cbz"@0xffffff8008a5f238), ma e'
 * l'unico dei tre che possa essere nullo -- gli altri due sono un letterale e
 * un vettore di pila. Con due o con tre termini il codice emesso e' lo stesso.
 *
 * `static`: nella mappa NON esiste (`grep -c " parser_get_int_data$"
 * oracolo/stock.map` stampa 0), quindi di fabbrica e' incorporata.
 */
static int parser_get_int_data(char *section, char *keyname, char *rv, int len)
{
	int ret = 0;
	int size = 0;
	/* "321703e2 orr"@0xffffff8008a5f298 -- 512 byte azzerati all'ingresso,
	 * PRIMA del controllo sui puntatori: e' un inizializzatore di
	 * dichiarazione, non una `memset` scritta a mano
	 * ("940fe358 bl"@0xffffff8008a5f2a0 precede
	 * "b4000193 cbz"@0xffffff8008a5f2a4) */
	char value[512] = {0};

	if (rv == NULL || section == NULL || keyname == NULL) {
		/* "\x013ILITEK: (%s, %d): Parameters are invalid\n"@0xffffff80092437ca,
		 * __func__ = "parser_get_int_data"@0xffffff80092437f6;
		 * "528046c2 mov"@0xffffff8008a5f270 -- __LINE__ = 566 */
#line 566
		ILI_ERR("Parameters are invalid\n");
		return -EINVAL;
	}

	/* "94000b4f bl"@0xffffff8008a5f24c verso <parser_get_ini_key_value> */
	ret = parser_get_ini_key_value(section, keyname, value);

	/* "94102cd8 bl"@0xffffff8008a5f260 verso <snprintf>, con
	 * x2 = "%s"@0xffffff8009100e35 ("9138d442 add"@0xffffff8008a5ed50) */
	size = snprintf(rv, len, "%s", value);

	if (ret < 0)
		return 0;

	return size;
}

/*
 * ===========================================================================
 * parser_ini_str_trim_r -- riga 625, incorporata in ilitek_tddi_mp_test_main
 * ===========================================================================
 * Toglie gli spazi in testa alla riga. La soglia 512 e' misurata
 * ("f10802bf cmp"@0xffffff8008a60268 + "540000a2 b.cs"@0xffffff8008a6026c):
 * sotto la soglia lavora nel vettore di pila, sopra alloca.
 *
 * IL RITORNO SUL RAMO D'ERRORE E' IL LETTERALE VUOTO, ed e' misurato:
 * "90003480 adrp"@0xffffff8008a602e4 + "91069c00 add"@0xffffff8008a602e8
 * mettono in x0 l'indirizzo 0xffffff80090f01a7, che `leggi_stringa.py` legge
 * come `b''` (len=0), e il salto "1400000e b"@0xffffff8008a602ec porta
 * direttamente alla `strlen` del chiamante. NON e' NULL: il chiamante
 * chiamerebbe `strlen(NULL)`.
 */
static char *parser_ini_str_trim_r(char *buf)
{
	int i, len;
	char tmp[512] = {0};
	char *p = tmp;
	char *alloc = NULL;

	/* "940fe029 bl"@0xffffff8008a6025c verso <__pi_strlen> */
	len = strlen(buf);

	if (len >= 512) {
		/* "97dfc03f bl"@0xffffff8008a6028c verso <__kmalloc>, bandiere
		 * 0x14080c0 = GFP_KERNEL | __GFP_ZERO
		 * ("52901801 mov"@0xffffff8008a60280 +
		 * "72a02801 movk"@0xffffff8008a60284) */
		alloc = kzalloc(len, GFP_KERNEL);
		p = alloc;
		if (IS_ERR(p) || p == NULL) {
			/* "\x013ILITEK: (%s, %d): Failed to allocate tmp buf\n"
			 * @0xffffff8009244de4,
			 * __func__ = "parser_ini_str_trim_r"@0xffffff8009244e14;
			 * "52804e22 mov"@0xffffff8008a602d4 -- __LINE__ = 625 */
#line 625
			ILI_ERR("Failed to allocate tmp buf\n");
			/* ""@0xffffff80090f01a7 */
			return "";
		}
	}

	/* "38686b49 ldrb"@0xffffff8008a602b0 + "7100813f cmp"@0xffffff8008a602b4
	 * (0x20 = lo spazio) */
	for (i = 0; i < len; i++) {
		if (buf[i] != ' ')
			break;
	}

	if (i < len) {
		/* "9410207b bl"@0xffffff8008a60300 verso <strncpy>, con
		 * x2 = sxtw(len - i) ("4b080288 sub"@0xffffff8008a602f4 +
		 * "93407d02 sxtw"@0xffffff8008a602f8) */
		strncpy(p, buf + i, len - i);
	}

	/* "94102077 bl"@0xffffff8008a60310, x2 = x21 = sxtw(len) */
	strncpy(buf, p, len);

	/* "b400009c cbz"@0xffffff8008a60314 + "97dfc125 bl"@0xffffff8008a6031c */
	if (alloc)
		kfree(alloc);

	return buf;
}

/*
 * ===========================================================================
 * parser_get_ini_phy_data -- righe 663..796, incorporata in questa funzione
 * ===========================================================================
 * Spezza il testo del file .ini in voci `struct ilitek_ini_item`. E' la piu'
 * lunga delle dodici incorporate.
 *
 * LE COSTANTI SONO MISURATE, I LORO NOMI SONO SCELTI:
 *   1536 = la riga piu' lunga  ("f118013f cmp"@0xffffff8008a601ec, #0x600);
 *          il buffer allocato e' 1537 ("5280c022 mov"@0xffffff8008a6014c);
 *   1800 = il numero massimo di voci ("711c211f cmp"@0xffffff8008a601a4,
 *          #0x708) -- lo stesso 1800 con cui la corsa viene azzerata
 *          ("5280e113 mov"@0xffffff8008a600d4);
 *    100 = la lunghezza massima di un nome di sezione o di chiave
 *          ("7101951f cmp"@0xffffff8008a605a4 e
 *          "7101945f cmp"@0xffffff8008a6055c, #0x65 = 101 come limite
 *          ESCLUSIVO, quindi 100 come massimo);
 *   2000 = la lunghezza massima di un valore
 *          ("711f445f cmp"@0xffffff8008a604c4, #0x7d1 = 2001).
 *
 * IL TERZO E IL QUARTO CAMPO DI `struct ilitek_ini_item` LI TROVA QUESTO
 * LOTTO. Il lotto F1 dichiarava `char c200[2008]; int c2208;` e scriveva, con
 * onesta', che la larghezza 2008 era «una SCELTA, non una misura» perche'
 * nessuna istruzione del gruppo F citava un limite superiore di `c200`.
 * Adesso ci sono le istruzioni: "b9089900 str"@0xffffff8008a603bc scrive a
 * +2200 e "b9089d09 str"@0xffffff8008a60428 scrive a +2204. Sono due `int`
 * dentro quel presunto riempimento, quindi `c200` e' largo AL PIU' 2000 --
 * ed e' esattamente il limite che "711f445f cmp"@0xffffff8008a604c4 impone al
 * suo contenuto. Il conto torna: 100 + 100 + 2000 + 4 + 4 + 4 = 2212.
 */
static int parser_get_ini_phy_data(char *data, int size)
{
	int i, n = 0, offset = 0, seq = 0, ret = 0, len = 0;
	char *ini_buf = NULL, *tmp_sec = NULL;

	/* "97dfbad7 bl"@0xffffff8008a60150 verso <kmem_cache_alloc_trace>,
	 * bandiere 0x14080c0 = GFP_KERNEL | __GFP_ZERO
	 * ("52901801 mov"@0xffffff8008a60144 +
	 * "72a02801 movk"@0xffffff8008a60148), dimensione 1537
	 * ("5280c022 mov"@0xffffff8008a6014c) */
	ini_buf = kzalloc(PARSER_MAX_CFG_BUF + 1, GFP_KERNEL);
	if (IS_ERR(ini_buf) || ini_buf == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate ini_buf memory, %ld\n"
		 * @0xffffff8009244be9,
		 * __func__ = "parser_get_ini_phy_data"@0xffffff8009244bd1;
		 * "528052e2 mov"@0xffffff8008a6067c -- __LINE__ = 663.
		 * La chiamata occupa DUE righe e `__LINE__` e' la riga della
		 * parentesi CHIUSA: la direttiva porta quindi 662 */
#line 662
		ILI_ERR("Failed to allocate ini_buf memory, %ld\n",
			PTR_ERR(ini_buf));
		ret = -ENOMEM;
		goto out;
	}

	/* "97dfbace bl"@0xffffff8008a60174, stessa dimensione */
	tmp_sec = kzalloc(PARSER_MAX_CFG_BUF + 1, GFP_KERNEL);
	if (IS_ERR(tmp_sec) || tmp_sec == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate tmpSectionName memory, %ld\n"
		 * @0xffffff8009244c25;
		 * "528053c2 mov"@0xffffff8008a606d8 -- __LINE__ = 670; la
		 * chiamata occupa due righe, quindi la direttiva porta 669 */
#line 669
		ILI_ERR("Failed to allocate tmpSectionName memory, %ld\n",
			PTR_ERR(tmp_sec));
		ret = -ENOMEM;
		goto out;
	}

	/* "52827102 mov"@0xffffff8008a60190 -- 5000, cioe' META' del vettore.
	 * DIFETTO DELLA FABBRICA, riprodotto (regola 7) */
	memset(seq_item, 0, 5000);

	while (1) {
		/* "711c211f cmp"@0xffffff8008a601a4 +
		 * "540024ec b.gt"@0xffffff8008a601a8 */
		if (ilitek_ini_num > PARSER_MAX_KEY_NUM) {
			/* "\x013ILITEK: (%s, %d): MAX_KEY_NUM: Out of length\n"
			 * @0xffffff8009244c68;
			 * "52805542 mov"@0xffffff8008a60654 -- __LINE__ = 682 */
#line 682
			ILI_ERR("MAX_KEY_NUM: Out of length\n");
			goto out;
		}

		/* "6b1b029f cmp"@0xffffff8008a601c0 +
		 * "540024ca b.ge"@0xffffff8008a601c4 */
		if (offset >= size)
			goto out;

		/*
		 * LA LETTURA DI UNA RIGA. Il carattere si legge dal testo con
		 * base e indice ("3868694b ldrb"@0xffffff8008a601d0); 0x0d e
		 * 0x0a sono provati in quest'ordine
		 * ("7100357f cmp"@0xffffff8008a601d8 e
		 * "7100297f cmp"@0xffffff8008a601e0), e il NUL prima di tutti
		 * ("340002ab cbz"@0xffffff8008a601d4).
		 */
		for (i = 0; i < PARSER_MAX_CFG_BUF; i++) {
			if (data[offset + i] == 0) {
				n = i + 1;
				break;
			}
			if (data[offset + i] == 0x0d || data[offset + i] == 0x0a) {
				/* "39400529 ldrb"@0xffffff8008a6020c -- il
				 * carattere SEGUENTE */
				if (data[offset + i + 1] == 0x0a ||
				    data[offset + i + 1] == 0x0d)
					n = i + 2;
				else
					n = i + 1;
				break;
			}
			/* "38286b4b strb"@0xffffff8008a601f0 */
			ini_buf[i] = data[offset + i];
		}

		/* "38286b5f strb"@0xffffff8008a60234 e
		 * "38296b5f strb"@0xffffff8008a601fc */
		ini_buf[i] = 0;

		/*
		 * "36f801c9 tbz"@0xffffff8008a60200 prova IL BIT 31 di `n`,
		 * cioe' il segno. Il ramo e' IRRAGGIUNGIBILE (`n` non e' mai
		 * negativo) ma il codice c'e' e va riprodotto (regola 7).
		 */
		if (n < 0) {
			/* "\x013ILITEK: (%s, %d): End of Line\n"@0xffffff8009244c98;
			 * "52805662 mov"@0xffffff8008a608c8 -- __LINE__ = 691 */
#line 691
			ILI_ERR("End of Line\n");
			goto out;
		}

		/* "0b140134 add"@0xffffff8008a60238 */
		offset += n;

		/* "940fdff7 bl"@0xffffff8008a60324 verso <__pi_strlen> sul
		 * valore reso da `parser_ini_str_trim_r` */
		len = strlen(parser_ini_str_trim_r(ini_buf));

		/* "34000095 cbz"@0xffffff8008a60330 e
		 * "71008d1f cmp"@0xffffff8008a60338 (0x23 = '#') */
		if (len == 0 || ini_buf[0] == '#')
			continue;

		/* "71000ebf cmp"@0xffffff8008a60354 (3),
		 * "71016d1f cmp"@0xffffff8008a6035c (0x5b = '['),
		 * "7101753f cmp"@0xffffff8008a60374 (0x5d = ']') */
		if (len >= 3 && ini_buf[0] == '[' && ini_buf[len - 1] != ']') {
			/* "\x013ILITEK: (%s, %d): Bad Section: %s\n"
			 * @0xffffff8009244cb9;
			 * "52805802 mov"@0xffffff8008a60b94 -- __LINE__ = 704 */
#line 704
			ILI_ERR("Bad Section: %s\n", ini_buf);
			ret = -EINVAL;
			goto out;
		}

		/* "71016d1f cmp"@0xffffff8008a6037c +
		 * "54000fe0 b.eq"@0xffffff8008a60380 */
		if (ini_buf[0] == '[') {
			/* "51000ab3 sub"@0xffffff8008a60588 --
			 * "b9089913 str"@0xffffff8008a60590 scrive a +2200 */
			ilitek_ini_data[ilitek_ini_num].c2200 = len - 2;
			if (ilitek_ini_data[ilitek_ini_num].c2200 >
			    PARSER_MAX_KEY_NAME_LEN) {
				/* "\x013ILITEK: (%s, %d): MAX_KEY_NAME_LEN: Out Of Length\n"
				 * @0xffffff8009244cde ("91337800 add"@0xffffff8008a61f50);
				 * "528058e2 mov"@0xffffff8008a61f58 -- __LINE__ = 711 */
#line 711
				ILI_ERR("MAX_KEY_NAME_LEN: Out Of Length\n");
				ret = -1;
				goto out;
			}
			/* "f10196ff cmp"@0xffffff8008a605b0 (0x65 = 101) +
			 * "5400cd62 b.cs"@0xffffff8008a605b4 */
			if (seq >= PARSER_MAX_SECTION_NUM + 1) {
				/* "\x013ILITEK: (%s, %d): seq_item is over than its define (%d), abort\n"
				 * @0xffffff8009244d13;
				 * "528059a2 mov"@0xffffff8008a61f70 --
				 * __LINE__ = 717; la chiamata occupa due righe,
				 * quindi la direttiva porta 716 */
#line 716
				ILI_ERR("seq_item is over than its define (%d), abort\n",
					seq);
				ret = -1;
				goto out;
			}
			/* "38286b5f strb"@0xffffff8008a605d0 -- il ']' finale
			 * sostituito dal NUL */
			ini_buf[len - 1] = 0;
			/* "94101fc6 bl"@0xffffff8008a605d4, x2 = 1537 */
			strncpy(tmp_sec, ini_buf + 1, PARSER_MAX_CFG_BUF + 1);
			/* "94101fbe bl"@0xffffff8008a605f4, x2 = 100 */
			strncpy(seq_item[seq], tmp_sec, PARSER_MAX_KEY_NAME_LEN);
			/* "910006f7 add"@0xffffff8008a60604 -- l'incremento sta
			 * PRIMA del messaggio, e il messaggio ricalcola
			 * l'indirizzo con il valore NUOVO
			 * ("9b1522e3 madd"@0xffffff8008a6061c): stampa la
			 * voce SEGUENTE, che e' vuota. DIFETTO DELLA FABBRICA,
			 * riprodotto (regola 7) */
			seq++;
			/* "\x016ILITEK: (%s, %d): Section Name: %s, Len: %d, offset = %d\n"
			 * @0xffffff8009244d55;
			 * "52805b02 mov"@0xffffff8008a60620 -- __LINE__ = 728 */
#line 728
			ILI_DBG("Section Name: %s, Len: %d, offset = %d\n", seq_item[seq], len - 2, offset);
			continue;
		}

		/* "94102053 bl"@0xffffff8008a603a0 verso <strncpy>, x2 = 100 */
		strncpy(ilitek_ini_data[ilitek_ini_num].c0, tmp_sec,
			PARSER_MAX_KEY_NAME_LEN);
		/* "940fdfd6 bl"@0xffffff8008a603a8 +
		 * "b9089900 str"@0xffffff8008a603bc */
		ilitek_ini_data[ilitek_ini_num].c2200 = strlen(tmp_sec);

		/*
		 * LA RICERCA DEL SEPARATORE. "7100f53f cmp"@0xffffff8008a603d0
		 * (0x3d = '='), "71016d3f cmp"@0xffffff8008a603d8 ('[') e
		 * "7101753f cmp"@0xffffff8008a603e0 (']') dentro lo stesso
		 * ciclo: se compare una parentesi la riga si butta.
		 */
		n = -1;
		for (i = 0; i < len; i++) {
			if (ini_buf[i] == '=') {
				n = i;
				break;
			}
			if (ini_buf[i] == '[' || ini_buf[i] == ']')
				break;
		}

		if (n > 0) {
			/* "b9089d13 str"@0xffffff8008a60548 -- a +2204 */
			ilitek_ini_data[ilitek_ini_num].c2204 = n;
			if (ilitek_ini_data[ilitek_ini_num].c2204 >
			    PARSER_MAX_KEY_NAME_LEN) {
				/* "\x013ILITEK: (%s, %d): MAX_KEY_NAME_LEN: Out Of Length\n"
				 * @0xffffff8009244cde ("91337800 add"@0xffffff8008a61f38);
				 * "52806102 mov"@0xffffff8008a61f40 -- __LINE__ = 776 */
#line 776
				ILI_ERR("MAX_KEY_NAME_LEN: Out Of Length\n");
				ret = -1;
				goto out;
			}
			/* "940fdde4 bl"@0xffffff8008a60570 verso <__memcpy>,
			 * destinazione elemento + 0x64 = 100
			 * ("91019100 add"@0xffffff8008a60568) */
			memcpy(ilitek_ini_data[ilitek_ini_num].c100, ini_buf,
			       ilitek_ini_data[ilitek_ini_num].c2204);
			/* "4b130295 sub"@0xffffff8008a60574 */
			len = len - 1 - n;
		} else {
			/* "941021d5 bl"@0xffffff8008a60414 verso <strstr> con
			 * "benchmark_data"@0xffffff8009243e2e */
			if (strstr(&ilitek_ini_data[ilitek_ini_num].c0[0],
				   "benchmark_data") != NULL) {
				/* "321f0be9 orr"@0xffffff8008a60424 -- 14 */
				ilitek_ini_data[ilitek_ini_num].c2204 = 14;
				strncpy(ilitek_ini_data[ilitek_ini_num].c100,
					"benchmark_data",
					PARSER_MAX_KEY_NAME_LEN);
			/* "941021c1 bl"@0xffffff8008a60464 con
			 * "node type"@0xffffff80092430e3 */
			} else if (strstr(&ilitek_ini_data[ilitek_ini_num].c0[0],
					  "node type") != NULL) {
				/* "52800129 mov"@0xffffff8008a60474 -- 9 */
				ilitek_ini_data[ilitek_ini_num].c2204 = 9;
				strncpy(ilitek_ini_data[ilitek_ini_num].c100,
					"node type", PARSER_MAX_KEY_NAME_LEN);
			} else {
				continue;
			}
		}

		/* "b908a115 str"@0xffffff8008a604b0 -- a +2208 */
		ilitek_ini_data[ilitek_ini_num].c2208 = len;
		if (ilitek_ini_data[ilitek_ini_num].c2208 >
		    PARSER_MAX_KEY_VALUE_LEN) {
			/* "52806262 mov"@0xffffff8008a60b74 -- __LINE__ = 787 */
#line 787
			ILI_ERR("MAX_KEY_VALUE_LEN: Out Of Length\n");
			ret = -1;
			goto out;
		}

		/* "940fde09 bl"@0xffffff8008a604dc verso <__memcpy>,
		 * destinazione elemento + 0xc8 = 200
		 * ("91032100 add"@0xffffff8008a604d0) */
		memcpy(ilitek_ini_data[ilitek_ini_num].c200, ini_buf + n + 1,
		       ilitek_ini_data[ilitek_ini_num].c2208);

		/* "\x016ILITEK: (%s, %d): %s = %s\n"@0xffffff8009244dc7;
		 * "52806382 mov"@0xffffff8008a604fc -- __LINE__ = 796 */
#line 796
		ILI_DBG("%s = %s\n", ilitek_ini_data[ilitek_ini_num].c100, ilitek_ini_data[ilitek_ini_num].c200);

		/* "11000508 add"@0xffffff8008a60520 */
		ilitek_ini_num++;
	}

out:
	/* "b500043a cbnz"@0xffffff8008a60664 +
	 * "97dfc031 bl"@0xffffff8008a606ec verso <kfree> */
	if (ini_buf)
		kfree(ini_buf);
	/* "b4000073 cbz"@0xffffff8008a606f0 +
	 * "97dfc02e bl"@0xffffff8008a606f8 */
	if (tmp_sec)
		kfree(tmp_sec);
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_mp_ini_parser -- righe 816..883
 * ===========================================================================
 * Apre il file .ini, lo legge in memoria e lo passa a
 * `parser_get_ini_phy_data`.
 *
 * DUE DIFETTI DELLA FABBRICA, RIPRODOTTI (regola 7):
 *  1. quando il file non si apre e si ripiega su `request_firmware`, il
 *     descrittore resta NULLO e la `vfs_read` viene chiamata LO STESSO --
 *     "aa1f03f8 mov"@0xffffff8008a5eb10 azzera x24, e
 *     "97e02048 bl"@0xffffff8008a5eba0 lo passa a `vfs_read` come primo
 *     argomento senza nessun controllo in mezzo. Il contenuto letto da
 *     `request_firmware` non viene MAI copiato: della `struct firmware` si
 *     usa solo il campo a offset 0 ("f940011b ldr"@0xffffff8008a5eb14),
 *     cioe' la dimensione;
 *  2. `%ld` riceve un puntatore, non un `long`
 *     ("aa1803e3 mov"@0xffffff8008a5eac8 e "aa1603e3 mov"@0xffffff8008a5ec00).
 */
static int ilitek_tddi_mp_ini_parser(void)
{
	int ret = 0, fsize = 0, n = 0;
	char *tmp = NULL;
	struct file *f = NULL;
	const struct firmware *fw = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;

	/* "\x016ILITEK: (%s, %d): ini file path = %s\n"@0xffffff80092449fa,
	 * __func__ = "ilitek_tddi_mp_ini_parser"@0xffffff8009244a22;
	 * "52806602 mov"@0xffffff8008a5ea68 -- __LINE__ = 816.
	 * "f9415513 ldr"@0xffffff8008a5ea60 legge `idev->c680` */
#line 816
	ILI_INFO("ini file path = %s\n", (char *)idev->c680);

	/* "97e01bf5 bl"@0xffffff8008a5ea88 verso <filp_open>, w1 = 0
	 * ("2a1f03e1 mov"@0xffffff8008a5ea84) e w2 = 644
	 * ("52805082 mov"@0xffffff8008a5ea7c) */
	f = filp_open((char *)idev->c680, O_RDONLY, 644);
	if (IS_ERR(f) || f == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to open ini file at %ld, trying to request\n"
		 * @0xffffff8009244a3c;
		 * "52806682 mov"@0xffffff8008a5eac0 -- __LINE__ = 820 */
#line 820
		ILI_ERR("Failed to open ini file at %ld, trying to request\n", PTR_ERR(f));

		/* "\x016ILITEK: (%s, %d): request path = %s\n"@0xffffff8009244a83;
		 * "528066e2 mov"@0xffffff8008a5eadc -- __LINE__ = 823;
		 * "f9415914 ldr"@0xffffff8008a5eae0 legge `idev->c688` */
#line 823
		ILI_INFO("request path = %s\n", (char *)idev->c688);

		/* "97eaa39f bl"@0xffffff8008a5eb00 verso <request_firmware>,
		 * x2 = `idev->c24` ("f9400d02 ldr"@0xffffff8008a5eafc) */
		if (request_firmware(&fw, (char *)idev->c688,
				     (struct device *)idev->c24) < 0) {
			/* "\x013ILITEK: (%s, %d): Request ini file failed\n"
			 * @0xffffff8009244aaa;
			 * "52806722 mov"@0xffffff8008a5efbc -- __LINE__ = 825 */
#line 825
			ILI_ERR("Request ini file failed\n");
			return -1;
		}
		/* "f9414be8 ldr"@0xffffff8008a5eb0c + "f940011b ldr"@0xffffff8008a5eb14 */
		fsize = fw->size;
		f = NULL;
	} else {
		/* "f9400f08 ldr"@0xffffff8008a5ea9c (f_path.dentry, +24),
		 * "f9401908 ldr"@0xffffff8008a5eaa4 (d_inode, +48),
		 * "91014108 add"@0xffffff8008a5eaa8 (+0x50 = i_size) */
		fsize = f->f_path.dentry->d_inode->i_size;
	}

	/* "\x016ILITEK: (%s, %d): ini file size = %d\n"@0xffffff8009244ad7;
	 * "52806922 mov"@0xffffff8008a5eb28 -- __LINE__ = 841 */
#line 841
	ILI_INFO("ini file size = %d\n", fsize);

	/* "7100037f cmp"@0xffffff8008a5eb34 + "540004ad b.le"@0xffffff8008a5eb38 */
	if (fsize <= 0) {
		/* "\x013ILITEK: (%s, %d): The size of file is invaild\n"
		 * @0xffffff8009244aff;
		 * "52806962 mov"@0xffffff8008a5ebdc -- __LINE__ = 843 */
#line 843
		ILI_ERR("The size of file is invaild\n");
		ret = -EINVAL;
		goto out;
	}

	/* "97df8f48 bl"@0xffffff8008a5eb50 verso <vmalloc>, x0 = fsize + 1
	 * ("8b080268 add"@0xffffff8008a5eb48 con x8 = 0x100000000 e
	 * "9360fd00 asr"@0xffffff8008a5eb4c, cioe' l'aritmetica con segno a
	 * 32 bit) */
	tmp = vmalloc(fsize + 1);
	if (IS_ERR(tmp) || tmp == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate tmp memory, %ld\n"
		 * @0xffffff8009244b30;
		 * "52806a42 mov"@0xffffff8008a5ebfc -- __LINE__ = 850 */
#line 850
		ILI_ERR("Failed to allocate tmp memory, %ld\n", PTR_ERR(tmp));
		ret = -ENOMEM;
		goto out;
	}

	/* "d5384114 mrs"@0xffffff8008a5eb68 (sp_el0),
	 * "f9400697 ldr"@0xffffff8008a5eb6c (addr_limit a +8),
	 * "940fe0da bl"@0xffffff8008a5eb84 verso <set_bit> con w0 = 5 */
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	/* "97e02048 bl"@0xffffff8008a5eba0 verso <vfs_read> */
	vfs_read(f, tmp, fsize, &pos);
	set_fs(old_fs);

	/* "38336adf strb"@0xffffff8008a600cc */
	tmp[fsize] = 0;

	/* "b904039f str"@0xffffff8008a600dc -- 0xffffff800a0ff400 azzerato */
	ilitek_ini_num = 0;

	/*
	 * "5280e113 mov"@0xffffff8008a600d4 -- 1800 giri;
	 * "52811402 mov"@0xffffff8008a600e4 -- 2208 byte azzerati per giro con
	 * una `__memset` VERA ("940fdfc3 bl"@0xffffff8008a600f4), piu'
	 * "b90002ff str"@0xffffff8008a60100 che azzera a parte i quattro byte
	 * a +2208.
	 *
	 * IL TAGLIO 2208 + 4 E' MISURATO, e 2208 non e' `sizeof` di niente:
	 * e' 100 + 100 + 2000 + 4 + 4, cioe' TUTTA la voce meno l'ultimo
	 * campo. La forma qui sotto lo produce perche' il compilatore fonde
	 * le tre `memset` contigue con le due scritture a quattro byte che le
	 * seguono e lascia fuori la terza. Con un solo
	 * `memset(&ilitek_ini_data[n], 0, 2208)` NON si ottiene: il
	 * compilatore riconosce che il ciclo copre la corsa intera e lo
	 * sostituisce con UNA `memset` da 3981600 byte -- misurato: nel nostro
	 * oggetto compariva l'immediato 0x3cc120 al posto di 0x8a0, e il ciclo
	 * da 1800 giri non c'era piu'.
	 * Che la corsa venga RILETTA dal globale a ogni giro
	 * ("f9420728 ldr"@0xffffff8008a600e0) e' l'altra faccia dello stesso
	 * fatto: se il compilatore potesse tenerla in un registro potrebbe
	 * anche fondere il ciclo.
	 */
	for (n = 0; n < PARSER_MAX_KEY_NUM; n++) {
		memset(ilitek_ini_data[n].c0, 0, PARSER_MAX_KEY_NAME_LEN);
		memset(ilitek_ini_data[n].c100, 0, PARSER_MAX_KEY_NAME_LEN);
		memset(ilitek_ini_data[n].c200, 0, PARSER_MAX_KEY_VALUE_LEN);
		ilitek_ini_data[n].c2200 = 0;
		ilitek_ini_data[n].c2204 = 0;
		ilitek_ini_data[n].c2208 = 0;
	}

	/* "7100077f cmp"@0xffffff8008a60108 + il ciclo di
	 * "386a690b ldrb"@0xffffff8008a60120 su `_ctype` a
	 * 0xffffff8008fc9748: `tst w11,#1` e' `isupper`, `+0x20` e' la
	 * conversione. E' `tolower` del kernel, non una tabella del driver */
	for (n = 0; n < fsize; n++)
		tmp[n] = tolower(tmp[n]);

	ret = parser_get_ini_phy_data(tmp, fsize);
	if (ret < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to get physical ini data, ret = %d\n"
		 * @0xffffff8009244b68;
		 * "52806de2 mov"@0xffffff8008a60734 -- __LINE__ = 879 */
#line 879
		ILI_ERR("Failed to get physical ini data, ret = %d\n", ret);
		goto out;
	}

	/* "\x016ILITEK: (%s, %d): Parsed ini file done\n"@0xffffff8009244ba7;
	 * "52806e62 mov"@0xffffff8008a60710 -- __LINE__ = 883 */
#line 883
	ILI_INFO("Parsed ini file done\n");
	ret = 0;

out:
	/* "b5ff27b6 cbnz"@0xffffff8008a6071c + "97df8d89 bl"@0xffffff8008a5ec14 */
	if (tmp)
		vfree(tmp);
	/* "97e01cd0 bl"@0xffffff8008a5ec20 verso <filp_close>, x1 = NULL */
	filp_close(f, NULL);
	return ret;
}

/*
 * ===========================================================================
 * mp_get_timing_info -- riga 2620
 * ===========================================================================
 * Legge dalla sezione "pv5_4 command" la chiave "timing_info_raw" e la chiave
 * "enable" del DDI. Il valore letto finisce in `ilitek_dbg_en`? NO: finisce a
 * 0xffffff800a0fc963, cioe' `ilitek_dbg_en + 11`
 * ("392f8e88 strb"@0xffffff8008a5edb4, x20 = 0xffffff800a0fc000, offset
 * 3043). E' un globale del driver che questo lotto NON dichiara -- lo
 * dichiarera' il lotto di merge, che vede tutti i gruppi (voce del delta di
 * header). Qui e' `ilitek_mp_ddi_mode`, definito in questo file.
 */
static int mp_get_timing_info(void)
{
	int ret = 0, len = 0;
	char str[256] = {0};
	u8 timing_info_raw[16] = {0};

	/* "392f8e9f strb"@0xffffff8008a5ed28 -- azzerato PRIMA della
	 * `parser_get_ini_key_value` */
	ilitek_mp_ddi_mode = 0;

	/* "94000c91 bl"@0xffffff8008a5ed44, sezione
	 * "pv5_4 command"@0xffffff800924332d, chiave
	 * "timing_info_raw"@0xffffff8009244e86; la dimensione dell'uscita e'
	 * 0x100 = 256 ("321803e1 orr"@0xffffff8008a5ed5c) */
	len = parser_get_int_data("pv5_4 command", "timing_info_raw", str, 256);
	/* "37f813c2 tbnz"@0xffffff8008a5ed6c -- il bit 31 di `len` */
	if (len < 0) {
		ret = -1;
		goto out;
	}

	/*
	 * "94001b16 bl"@0xffffff8008a5ed78 verso <parser_get_u8_array>. IL
	 * TERZO ARGOMENTO E' `len`, e questa e' la prova che il valore reso da
	 * `parser_get_int_data` viene USATO e non solo provato: w2 non e'
	 * ricaricato fra "1a80b3e2 csel"@0xffffff8008a5ed68 e la chiamata.
	 * Fra le due ci sono TRE istruzioni, non due (la prima stesura scriveva
	 * "solo" e ne elencava due): "37f813c2 tbnz"@0xffffff8008a5ed6c, che
	 * prova il bit 31 di w2 e NON lo scrive, poi
	 * "910a43e0 add"@0xffffff8008a5ed70 (x0) e
	 * "910943e1 add"@0xffffff8008a5ed74 (x1). Nessuna delle tre scrive w2,
	 * quindi la conclusione regge; era la parola "solo" a essere falsa.
	 */
	ret = parser_get_u8_array(str, timing_info_raw, len);
	/* "37f81340 tbnz"@0xffffff8008a5ed7c */
	if (ret < 0)
		goto out;

	/* "39495be8 ldrb"@0xffffff8008a5ed80 legge il SETTIMO byte del
	 * vettore (sp+598 con base sp+592) e
	 * "1a9f07e8 cset"@0xffffff8008a5eda0 ne fa uno 0/1 */
	ilitek_mp_ddi_mode = (timing_info_raw[6] != 0);

	/* "\x016ILITEK: (%s, %d): DDI Mode = %s\n"@0xffffff8009244e96,
	 * __func__ = "mp_get_timing_info"@0xffffff8009244eb9;
	 * "52814782 mov"@0xffffff8008a5edb0 -- __LINE__ = 2620.
	 * "9a891143 csel"@0xffffff8008a5eda4 sceglie fra
	 * "Long V"@0xffffff8009244ecc (condizione `ne`) e
	 * "Long H"@0xffffff8009244ed3 */
#line 2620
	ILI_INFO("DDI Mode = %s\n", ilitek_mp_ddi_mode ? "Long V" : "Long H");
out:
	return ret;
}

/*
 * ===========================================================================
 * ilitek_tddi_mp_init_item -- righe 3113..3124
 * ===========================================================================
 * Azzera `core_mp`, ci copia undici valori presi dalle due strutture puntate
 * da `idev`, li stampa, e poi prepara i cinquanta elementi di collaudo.
 *
 * LO `switch` A UNDICI VIE E' UNA TABELLA DI SALTO, e la tabella sta nel
 * binario a 0xffffff8008f7ec40, undici byte:
 *
 *   00 02 05 08 00 00 00 0b 00 00 1e
 *
 * letti con "38686b0a ldrb"@0xffffff8008a5e990 dopo
 * "7100291f cmp"@0xffffff8008a5e960 (#0xa) e
 * "540004c8 b.hi"@0xffffff8008a5e988, e usati come scostamento in PAROLE da
 * "10000089 adr"@0xffffff8008a5e98c ("8b0a0929 add"@0xffffff8008a5e994,
 * `lsl #2`, e "d61f0120 br"@0xffffff8008a5e998).
 *
 * SEI INDICI SU UNDICI PORTANO ALLA STESSA VIA (la voce 0x00: 0, 4, 5, 6, 8,
 * 9), ed e' quello che rende la tabella una tabella e non una catena di
 * confronti. PERCHE' IL COMPILATORE LA RIPRODUCA il sorgente deve avere
 * esattamente questi casi, in questo ordine, senza `default`.
 */
static void ilitek_tddi_mp_init_item(void)
{
	int i;

	/* "52802e02 mov"@0xffffff8008a5e6a8 -- 368 = sizeof(core_mp) */
	memset(&core_mp, 0, sizeof(core_mp));

	/* "f9401909 ldr"@0xffffff8008a5e6d4 e "f9401d0a ldr"@0xffffff8008a5e710 */
	core_mp.c0 = ((struct ilitek_c48 *)idev->c48)->c8;
	core_mp.c4 = ((struct ilitek_c48 *)idev->c48)->c2;
	core_mp.c6 = ((struct ilitek_c48 *)idev->c48)->c0;
	core_mp.c7 = ((struct ilitek_c48 *)idev->c48)->c1;
	core_mp.c8 = ((struct ilitek_c48 *)idev->c48)->c48;
	core_mp.c12 = ((struct ilitek_c56 *)idev->c56)->c0;
	core_mp.c16 = ((struct ilitek_c48 *)idev->c48)->c52;
	core_mp.c228 = ((struct ilitek_c56 *)idev->c56)->c36;
	core_mp.c212 = ((struct ilitek_c48 *)idev->c48)->c64;
	/* "291d22e9 stp"@0xffffff8008a5e748 -- i due byte di `idev` a +248 e
	 * +249 finiscono in due `int` adiacenti */
	core_mp.c232 = idev->c248;
	core_mp.c236 = idev->c249;
	/* "1b097d08 mul"@0xffffff8008a5e74c */
	core_mp.c256 = core_mp.c232 * core_mp.c236;
	/* "b9010af4 str"@0xffffff8008a5e750 -- w20 = -1 */
	core_mp.c264 = -1;
	/* "f900b2ea str"@0xffffff8008a5e740 -- otto byte, 352 = 240 e 356 = 0 */
	core_mp.c352 = 240;
	core_mp.c356 = 0;
	/*
	 * QUATTRO AZZERAMENTI CHE LA `memset` HA GIA' FATTO, e che pero' nel
	 * binario CI SONO: "a90f7eff stp"@0xffffff8008a5e73c (sedici byte a
	 * 240), "390362ff strb"@0xffffff8008a5e744 (216) e
	 * "3905a6ff strb"@0xffffff8008a5e758 (361). Non sono ridondanza del
	 * compilatore: `__memset` e' una CHIAMATA, quindi opaca, e il
	 * compilatore non puo' sapere che quei byte sono gia' a zero. Sono
	 * quindi assegnamenti del sorgente, e vanno riprodotti.
	 */
	core_mp.c240 = 0;
	core_mp.c244 = 0;
	core_mp.key_len = 0;
	core_mp.c252 = 0;
	core_mp.c216 = 0;
	core_mp.c361 = 0;

	/* "\x016ILITEK: (%s, %d): ============== TP & Panel info ================\n"
	 * @0xffffff800924380a, __func__ =
	 * "ilitek_tddi_mp_init_item"@0xffffff800924384f;
	 * "52818522 mov"@0xffffff8008a5e6e0 -- __LINE__ = 3113 */
#line 3113
	ILI_INFO("============== TP & Panel info ================\n");
	/* "\x016ILITEK: (%s, %d): Driver version = %s\n"@0xffffff8009243868,
	 * x3 = "2.0.6.0.191122"@0xffffff800923ed58 */
#line 3114
	ILI_INFO("Driver version = %s\n", DRIVER_VERSION);
	/* "\x016ILITEK: (%s, %d): TP Module = %s\n"@0xffffff8009243891,
	 * "f9414903 ldr"@0xffffff8008a5e78c legge `idev->c656` */
#line 3115
	ILI_INFO("TP Module = %s\n", (char *)idev->c656);
	/* "\x016ILITEK: (%s, %d): CHIP = 0x%x\n"@0xffffff80092438b5 */
#line 3116
	ILI_INFO("CHIP = 0x%x\n", core_mp.c0);
	/* "\x016ILITEK: (%s, %d): Firmware version = %x\n"@0xffffff80092438d6 */
#line 3117
	ILI_INFO("Firmware version = %x\n", core_mp.c8);
	/* "\x016ILITEK: (%s, %d): Protocol version = %x\n"@0xffffff8009243901 */
#line 3118
	ILI_INFO("Protocol version = %x\n", core_mp.c12);
	/* "\x016ILITEK: (%s, %d): Core version = %x\n"@0xffffff800924392c */
#line 3119
	ILI_INFO("Core version = %x\n", core_mp.c16);
	/* "\x016ILITEK: (%s, %d): Read CDC Length = %d\n"@0xffffff8009243953 */
#line 3120
	ILI_INFO("Read CDC Length = %d\n", core_mp.c228);
	/* "\x016ILITEK: (%s, %d): X length = %d, Y length = %d\n"
	 * @0xffffff800924397d -- "295d12e3 ldp"@0xffffff8008a5e810 legge i due
	 * campi INSIEME, ed e' la prova che sono adiacenti */
#line 3121
	ILI_INFO("X length = %d, Y length = %d\n", core_mp.c232, core_mp.c236);
	/* "\x016ILITEK: (%s, %d): Frame length = %d\n"@0xffffff80092439af */
#line 3122
	ILI_INFO("Frame length = %d\n", core_mp.c256);
	/* "\x016ILITEK: (%s, %d): Check busy method = %s\n"@0xffffff80092439d6 */
#line 3123
	ILI_INFO("Check busy method = %s\n", core_mp.c356 ? "Polling" : "Interrupt");
	/* "\x016ILITEK: (%s, %d): ===============================================\n"
	 * @0xffffff8009243a02 */
#line 3124
	ILI_INFO("===============================================\n");

	/* "5280065c mov"@0xffffff8008a5e8a0 -- 50 giri */
	for (i = 0; i < MP_TEST_ITEM; i++) {
		/* "7818537f sturh"@0xffffff8008a5e954 -- DUE byte, 0x15 e 0x16 */
		mp_items[i].c21 = 0;
		mp_items[i].c22 = 0;
		/* "3818737f sturb"@0xffffff8008a5e958 */
		mp_items[i].c23 = 0;
		/* "f818c377 stur"@0xffffff8008a5e95c -- otto byte a 0x1c:
		 * la meta' bassa vale 0 e quella alta -1 */
		mp_items[i].c28 = 0;
		mp_items[i].c32 = -1;
		/* "f819437f stur"@0xffffff8008a5e964 -- otto byte a 0x24 */
		mp_items[i].c36 = 0;
		mp_items[i].c40 = 0;
		/* "b819c374 stur"@0xffffff8008a5e968 */
		mp_items[i].c44 = -1;
		/* "a93a7f7f stp"@0xffffff8008a5e974 e le altre: 0x30..0x56 */
		mp_items[i].c48 = 0;
		mp_items[i].c52 = 0;
		mp_items[i].c56 = 0;
		mp_items[i].c60 = 0;
		mp_items[i].c64 = 0;
		mp_items[i].c68 = 0;
		mp_items[i].c72 = 0;
		mp_items[i].c76 = 0;
		mp_items[i].c80 = 0;
		mp_items[i].delay_time = 0;
		mp_items[i].test_int_pin = 0;
		mp_items[i].int_pulse_test = 0;
		/* "f81c837f stur"@0xffffff8008a5e984 e le tre `stp` a 0x60,
		 * 0x70, 0x80 */
		mp_items[i].result_buf = NULL;
		mp_items[i].buf = NULL;
		mp_items[i].max_buf = NULL;
		mp_items[i].min_buf = NULL;
		mp_items[i].bench_mark_max = NULL;
		mp_items[i].bench_mark_min = NULL;
		mp_items[i].c136 = NULL;

		/*
		 * LO SWITCH. Le sei vie che portano a `mutual_test` sono
		 * scritte come sei `case` senza corpo che cadono sull'ultimo.
		 *
		 * CIO' CHE E' RIPRODOTTO E CIO' CHE NON LO E' -- CORREZIONE del
		 * 2026-08-22. La prima stesura diceva «e' la sola forma che dia
		 * la tabella di salto con la voce 0x00 ripetuta sei volte»:
		 * afferma piu' di quanto la misura sostenga. Riprodotta e' la
		 * STRUTTURA -- undici voci e SEI indici che condividono la stessa
		 * voce; la DISPOSIZIONE no. I due gruppi di undici byte, letti:
		 *
		 *   fabbrica, dai byte dell'ELF a 0xffffff8008f7ec40:
		 *     00 02 05 08 00 00 00 0b 00 00 1e
		 *   nostro, `objdump -s -j .rodata ilitek_mp.o` (clang di fabbrica):
		 *     0000 3b000306 3b3b3b09 3b3b19             ;...;;;.;;.
		 *
		 * cioe' di fabbrica il gruppo di sei e' la voce 0x00, il PRIMO
		 * blocco dopo la base 0xffffff8008a5e99c; da noi e' la voce 0x3b,
		 * l'ULTIMO. Combaciano il numero di voci, lo schema di
		 * condivisione e l'ordine relativo degli altri cinque blocchi.
		 */
		switch (mp_items[i].c16) {
		case 0:
		case 4:
		case 5:
		case 6:
		case 8:
		case 9:
			/* voce 0x00 -> "f9000373 str"@0xffffff8008a5e99c */
			mp_items[i].c144 = mutual_test;
			break;
		case 1:
			/* voce 0x02 -> "d0000028 adrp"@0xffffff8008a5e9a4 +
			 * "913a5108 add"@0xffffff8008a5e9a8 =
			 * 0xffffff8008a64e94 */
			mp_items[i].c144 = self_test;
			break;
		case 2:
			/* voce 0x05 -> 0xffffff8008a6493c */
			mp_items[i].c144 = key_test;
			break;
		case 3:
			/* voce 0x08 -> 0xffffff8008a64ec0 */
			mp_items[i].c144 = st_test;
			break;
		case 7:
			/*
			 * voce 0x0b. Il nome dell'elemento decide fra tre
			 * funzioni: "f1002c1f cmp"@0xffffff8008a5e9d8 (11) e
			 * "f100641f cmp"@0xffffff8008a5e9e0 (25) confrontano
			 * PRIMA la lunghezza, e solo se combacia si chiama
			 * `strncmp` -- e' la forma che il binario porta, non
			 * una `strcmp`.
			 */
			if (strlen(mp_items[i].c0) == strlen("open test_c") &&
			    strncmp(mp_items[i].c0, "open test_c",
				    strlen("open test_c")) == 0)
				mp_items[i].c144 = open_test_cap;
			else if (strlen(mp_items[i].c0) ==
					 strlen("open test(integration)_sp") &&
				 strncmp(mp_items[i].c0,
					 "open test(integration)_sp",
					 strlen("open test(integration)_sp")) == 0)
				mp_items[i].c144 = open_test_sp;
			else
				mp_items[i].c144 = mutual_test;
			break;
		case 10:
			/* voce 0x1e -> 0xffffff8008a64eec */
			mp_items[i].c144 = pin_test;
			break;
		}

		/* "97dfc0a0 bl"@0xffffff8008a5ea2c verso
		 * <kmem_cache_alloc_trace> con 16 byte
		 * ("321c03e2 orr"@0xffffff8008a5ea24) e bandiere 0x14000c0 =
		 * GFP_KERNEL SENZA __GFP_ZERO
		 * ("51402275 sub"@0xffffff8008a5e888 toglie 0x8000 alla
		 * costante 0x14080c0 che serve altrove) */
		mp_items[i].c8 = kmalloc(16, GFP_KERNEL);
		/* "b9000019 str"@0xffffff8008a5ea40 (w25 = 0x4c494146 = "FAIL")
		 * piu' "3900101f strb"@0xffffff8008a5ea44 (il NUL): cinque
		 * byte, cioe' una copia di "FAIL" TERMINATORE COMPRESO */
		memcpy(mp_items[i].c8, "FAIL", sizeof("FAIL"));
	}
}

/*
 * ===========================================================================
 * mp_sort_item -- riga 3363
 * ===========================================================================
 * Scorre le sezioni lette dal file .ini nell'ordine in cui compaiono e, per
 * ognuna, cerca l'elemento di collaudo con lo stesso nome; se lo trova e la
 * chiave "enable" e' non nulla, e se il campo `c24` dell'elemento combacia con
 * il modo (schermo acceso o spento), lo mette in coda alla lista da eseguire.
 *
 * IL CONFRONTO E' A DUE PASSI, e li porta il binario: prima le due lunghezze
 * ("940fe4fe bl"@0xffffff8008a5ef08 e "940fe4fb bl"@0xffffff8008a5ef14,
 * "eb00029f cmp"@0xffffff8008a5ef18), poi la `strncmp`
 * ("940fe516 bl"@0xffffff8008a5ef2c).
 */
static int mp_sort_item(bool lcm_on)
{
	int i, j, len = 0;
	char str[128] = {0};

	/* "f802e11f stur"@0xffffff8008a5ee28 e le tre `stp` che la SEGUONO
	 * ("a9027d1f stp"@0xffffff8008a5ee2c, "a9017d1f stp"@0xffffff8008a5ee30,
	 * "a9007d1f stp"@0xffffff8008a5ee34 -- la prima stesura scriveva "che la
	 * precedono", ed e' il disassemblato a smentirla):
	 * 54 byte azzerati a partire da 0xffffff800a0ff458.
	 * 54 = 4 + 50 = `sizeof(mp_run.num)` piu' il NUMERO DI ELEMENTI dove
	 * serviva il numero di BYTE (50 * 4 = 200). E' un difetto della
	 * fabbrica e si riproduce (regola 7): degli indici restano azzerati
	 * solo i primi dodici e mezzo */
	memset(&mp_run, 0, sizeof(mp_run.num) + MP_TEST_ITEM);

	/* "f10192ff cmp"@0xffffff8008a5ef48 (#0x64) */
	for (i = 0; i < PARSER_MAX_SECTION_NUM; i++) {
		/* "f100cb9f cmp"@0xffffff8008a5ef38 (#0x32) */
		for (j = 0; j < MP_TEST_ITEM; j++) {
			if (strlen(seq_item[i]) != strlen(mp_items[j].c0))
				continue;
			if (strncmp(seq_item[i], mp_items[j].c0,
				    strlen(seq_item[i])) != 0)
				continue;

			/* "94000c44 bl"@0xffffff8008a5ee78, chiave
			 * "enable"@0xffffff800924755d, uscita di 128 byte
			 * ("321903e1 orr"@0xffffff8008a5ee84) */
			len = parser_get_int_data(mp_items[j].c0, "enable",
						  str, 128);
			/* "940031c3 bl"@0xffffff8008a5eeac verso <katoi>;
			 * "1a9f07e8 cset"@0xffffff8008a5eeb4 con `ne` e
			 * "381ff2a8 sturb"@0xffffff8008a5eeb8 */
			mp_items[j].c23 = (katoi(str) != 0);
			/* "340003c0 cbz"@0xffffff8008a5eebc */
			if (!mp_items[j].c23)
				continue;
			/* "394002a8 ldrb"@0xffffff8008a5eec0 +
			 * "6b1b011f cmp"@0xffffff8008a5eec4 -- `c24` contro il
			 * modo, letto a UN byte */
			if (mp_items[j].c24 != lcm_on)
				continue;
			/* "7100c87f cmp"@0xffffff8008a5eed4 +
			 * "5400be0c b.gt"@0xffffff8008a5eed8 */
			if (mp_run.num > MP_TEST_ITEM) {
				/* "\x013ILITEK: (%s, %d): Test item(%d) is invaild, abort\n"
				 * @0xffffff8009244eda,
				 * __func__ = "mp_sort_item"@0xffffff8009244f0f;
				 * "5281a462 mov"@0xffffff8008a606a8 --
				 * __LINE__ = 3363 */
#line 3363
				ILI_ERR("Test item(%d) is invaild, abort\n", mp_run.num);
				return -1;
			}
			/* "b900051c str"@0xffffff8008a5eeec e
			 * "11000508 add"@0xffffff8008a5eef4 */
			mp_run.idx[mp_run.num] = j;
			mp_run.num++;
		}
	}
	/*
	 * "b9445908 ldr"@0xffffff8008a5ef54 + "7100051f cmp"@0xffffff8008a5ef60
	 * + "5400878b b.lt"@0xffffff8008a5ef64 NON appartengono a questa
	 * funzione: sono la guardia del ciclo `for` del CHIAMANTE, che salta
	 * direttamente a `mp_show_result` quando non c'e' niente da eseguire.
	 * Una prima stesura di questo lotto ci aveva messo qui un
	 * `if (mp_run.num < 1) return 0;` -- codice che nel binario non c'e'
	 * (classe B6) -- ed e' stato tolto.
	 */
	return 0;
}

/*
 * ===========================================================================
 * parser_get_tdf -- IL NOME E' SCELTO, LA FORMA E' MISURATA
 * ===========================================================================
 * NESSUN messaggio la nomina, quindi non e' fra le diciotto incorporate che
 * la ricognizione ha trovato con il `__func__`: che sia una funzione separata
 * e' una SCELTA di questo lotto. Cio' che e' misurato e' la FORMA, e lo e'
 * perche' il compilatore la emette SEI volte, sempre identica, ai sei siti
 * delle chiavi `v_tdf*` e `h_tdf*`.
 *
 * Legge un numero decimale con al piu' due cifre dopo il punto e lo rende
 * intero moltiplicato per cento: il punto non viene copiato
 * ("7100b99f cmp"@0xffffff8008a5f970, 0x2e = '.'), le cifre dopo il punto si
 * contano con un incremento CONDIZIONATO al fatto che il punto sia gia'
 * passato -- la sua guardia e' "7100013f cmp"@0xffffff8008a5f980, e
 * l'incremento e' la `cinc` a 0xffffff8008a5f98c, che NON e' citata nella
 * forma verificabile perche' `cinc` non sta nella tavola `MNEMONICI` di
 * `confinecitazioni.py` e `verificacitazioni.py` la rileggerebbe come
 * letterale (vedi il DELTA DI STRUMENTO nel rapporto) -- e alla fine si
 * moltiplica per dieci
 * se le cifre dopo il punto sono una ("0b000808 add"@0xffffff8008a5fa60 +
 * "531f7900 lsl"@0xffffff8008a5fa64, cioe' `x*5*2`) o per cento se sono zero
 * ("52800c88 mov"@0xffffff8008a5f9b8 + "1b087c00 mul"@0xffffff8008a5f9bc).
 *
 * LA CORREZIONE SI APPLICA SOLO SE `type` VALE 9
 * ("7100267f cmp"@0xffffff8008a5f9a4 + "54000601 b.ne"@0xffffff8008a5f9a8):
 * per tutti gli altri elementi il valore passa tale e quale.
 */
static int parser_get_tdf(char *str, int type)
{
	int i, j = 0, k = 0, dot = 0, len, ret;
	char tmp[10] = {0};

	/* "940fe26c bl"@0xffffff8008a5f950 verso <__pi_strlen> */
	len = strlen(str);

	for (i = 0; i < len; i++) {
		if (str[i] == '.') {
			dot = 1;
			continue;
		}
		/* "382b4acc strb"@0xffffff8008a5f984 -- indice esteso SENZA
		 * segno (`uxtw`), quindi `j` non e' un `int` con segno usato
		 * come indice: e' il compilatore che sa che e' non negativo */
		tmp[j++] = str[i];
		if (dot)
			k++;
	}

	/* "94002f06 bl"@0xffffff8008a5f9a0 verso <katoi> */
	ret = katoi(tmp);

	if (type == 9) {
		if (k == 1)
			ret = ret * 10;
		else if (k == 0)
			ret = ret * 100;
	}

	return ret;
}

/*
 * ===========================================================================
 * mp_test_run -- righe 3252..3272
 * ===========================================================================
 * Legge dal file .ini tutti i parametri dell'elemento, lo esegue chiamando il
 * puntatore a funzione installato da `ilitek_tddi_mp_init_item`, e -- se
 * l'esito e' negativo e il ritentativo e' acceso -- ritenta.
 *
 * L'ASIMMETRIA DEI `tdf` E' MISURATA, non e' una svista di chi scrive: il
 * ramo `c16 == 9` legge QUATTRO chiavi (`v_tdf_1`, `v_tdf_2`, `h_tdf_1`,
 * `h_tdf_2`) e l'altro DUE (`v_tdf`, `h_tdf`). La prova sta nel numero di
 * scritture che il compilatore emette per ciascun campo: DUE a +64
 * ("b9004100 str"@0xffffff8008a5fa74 e "b9004100 str"@0xffffff8008a5fb6c),
 * UNA a +68 ("b9004500 str"@0xffffff8008a5fc64), DUE a +72
 * ("b9004900 str"@0xffffff8008a5fd5c e "b9004900 str"@0xffffff8008a5fd78) e
 * UNA a +76 ("b9004d00 str"@0xffffff8008a5fe70). Con un ramo simmetrico i
 * conti sarebbero 2/2/2/2; con questo sono 2/1/2/1, ed e' quello che c'e'.
 * La conferma indipendente e' il salto
 * "14000045 b"@0xffffff8008a5fd60, che dal ramo corto salta OLTRE la lettura
 * di `h_tdf_2`.
 */
static void mp_test_run(int index)
{
	int ret = 0;
	char str[512] = {0};

	/* "94000ac9 bl"@0xffffff8008a5f464, chiave
	 * "spec option"@0xffffff8009244f1c; "38015f20 strb"@0xffffff8008a5f4a4
	 * scrive UN byte a +0x15 */
	parser_get_int_data(mp_items[index].c0, "spec option", str, 512);
	mp_items[index].c21 = katoi(str);

	/* chiave "type option"@0xffffff8009244f28;
	 * "39005900 strb"@0xffffff8008a5f510, UN byte a +0x16 */
	parser_get_int_data(mp_items[index].c0, "type option", str, 512);
	mp_items[index].c22 = katoi(str);

	/* chiave "frame count"@0xffffff8009244f34;
	 * "b8030f00 str"@0xffffff8008a5f57c, QUATTRO byte a +0x30 */
	parser_get_int_data(mp_items[index].c0, "frame count", str, 512);
	mp_items[index].c48 = katoi(str);

	/* chiave "trimmed mean"@0xffffff8009244f40;
	 * "b9003500 str"@0xffffff8008a5f5e8 */
	parser_get_int_data(mp_items[index].c0, "trimmed mean", str, 512);
	mp_items[index].c52 = katoi(str);

	/* chiave "lowest percentage"@0xffffff8009244f4d;
	 * "b9003900 str"@0xffffff8008a5f654 */
	parser_get_int_data(mp_items[index].c0, "lowest percentage", str, 512);
	mp_items[index].c56 = katoi(str);

	/* chiave "highest percentage"@0xffffff8009244f5f;
	 * "b9003d00 str"@0xffffff8008a5f6c0 */
	parser_get_int_data(mp_items[index].c0, "highest percentage", str, 512);
	mp_items[index].c60 = katoi(str);

	/* chiave "goldenmode"@0xffffff8009244f72;
	 * "b9005100 str"@0xffffff8008a5f72c */
	parser_get_int_data(mp_items[index].c0, "goldenmode", str, 512);
	mp_items[index].c80 = katoi(str);
	/* "340000e0 cbz"@0xffffff8008a5f730,
	 * "39400328 ldrb"@0xffffff8008a5f734 (`c21`, UN byte) +
	 * "6b08001f cmp"@0xffffff8008a5f738 +
	 * "54000080 b.eq"@0xffffff8008a5f73c */
	if (mp_items[index].c80 != 0 &&
	    mp_items[index].c80 != mp_items[index].c21)
		core_mp.c361 = 1;

	/* "b8410f37 ldr"@0xffffff8008a5f750 (pre-indice su +0x10) +
	 * "71002aff cmp"@0xffffff8008a5f754 (#0xa) */
	if (mp_items[index].c16 == 10) {
		/* chiave "test int pin"@0xffffff8009244f7d;
		 * "39015500 strb"@0xffffff8008a5f7cc, UN byte a +0x55 */
		parser_get_int_data(mp_items[index].c0, "test int pin", str, 512);
		mp_items[index].test_int_pin = katoi(str);
		/* chiave "int pulse test"@0xffffff8009244f8a;
		 * "39015900 strb"@0xffffff8008a5f838, +0x56 */
		parser_get_int_data(mp_items[index].c0, "int pulse test", str, 512);
		mp_items[index].int_pulse_test = katoi(str);
		/* chiave "delay time"@0xffffff8009244f99;
		 * "39015100 strb"@0xffffff8008a5f8a4, +0x54 */
		parser_get_int_data(mp_items[index].c0, "delay time", str, 512);
		mp_items[index].delay_time = katoi(str);
	}

	/* "710026ff cmp"@0xffffff8008a5f8c0 (#0x9) +
	 * "540001a1 b.ne"@0xffffff8008a5f8c4 */
	if (mp_items[index].c16 == 9) {
		/* chiave "v_tdf_1"@0xffffff8009244fa4 */
		parser_get_int_data(mp_items[index].c0, "v_tdf_1", str, 512);
		mp_items[index].c64 = parser_get_tdf(str, mp_items[index].c16);
		/* chiave "v_tdf_2"@0xffffff8009244fac */
		parser_get_int_data(mp_items[index].c0, "v_tdf_2", str, 512);
		mp_items[index].c68 = parser_get_tdf(str, mp_items[index].c16);
		/* chiave "h_tdf_1"@0xffffff8009244fb4 */
		parser_get_int_data(mp_items[index].c0, "h_tdf_1", str, 512);
		mp_items[index].c72 = parser_get_tdf(str, mp_items[index].c16);
		/* chiave "h_tdf_2"@0xffffff8009244fbc */
		parser_get_int_data(mp_items[index].c0, "h_tdf_2", str, 512);
		mp_items[index].c76 = parser_get_tdf(str, mp_items[index].c16);
	} else {
		/* chiave "v_tdf"@0xffffff8009244fc4 */
		parser_get_int_data(mp_items[index].c0, "v_tdf", str, 512);
		mp_items[index].c64 = parser_get_tdf(str, mp_items[index].c16);
		/* chiave "h_tdf"@0xffffff8009244fca */
		parser_get_int_data(mp_items[index].c0, "h_tdf", str, 512);
		mp_items[index].c72 = parser_get_tdf(str, mp_items[index].c16);
	}

	/* "f1002c1f cmp"@0xffffff8008a5fe80 (11) e
	 * "940fe13b bl"@0xffffff8008a5fe98 con
	 * "tx/rx delta"@0xffffff80092435e3 */
	if (strlen(mp_items[index].c0) == strlen("tx/rx delta") &&
	    strncmp(mp_items[index].c0, "tx/rx delta",
		    strlen("tx/rx delta")) == 0) {
		/* chiave "tx max"@0xffffff8009244fd0;
		 * "b90c2100 str"@0xffffff8008a5f290 -- 0xffffff800a0fcc20 */
		parser_get_int_data(mp_items[index].c0, "tx max", str, 512);
		core_mp.c288 = katoi(str);
		/* chiave "tx min"@0xffffff8009244fd7 */
		parser_get_int_data(mp_items[index].c0, "tx min", str, 512);
		core_mp.c292 = katoi(str);
		/* chiave "rx max"@0xffffff8009244fde */
		parser_get_int_data(mp_items[index].c0, "rx max", str, 512);
		core_mp.c296 = katoi(str);
		/* chiave "rx min"@0xffffff8009244fe5 */
		parser_get_int_data(mp_items[index].c0, "rx min", str, 512);
		core_mp.c300 = katoi(str);

		/* "\x016ILITEK: (%s, %d): %s: Tx Max = %d, Tx Min = %d, Rx Max = %d,  Rx Min = %d\n"
		 * @0xffffff8009244fec, __func__ =
		 * "mp_test_run"@0xffffff8009245039;
		 * "52819682 mov"@0xffffff8008a5f404 -- __LINE__ = 3252.
		 * "29401504 ldp"@0xffffff8008a5f3f4 legge `c288` e `c292`
		 * INSIEME */
#line 3252
		ILI_DBG("%s: Tx Max = %d, Tx Min = %d, Rx Max = %d,  Rx Min = %d\n", mp_items[index].c0, core_mp.c288, core_mp.c292, core_mp.c296, core_mp.c300);
	} else {
		/* chiave "max"@0xffffff8009244fe1;
		 * "b9001d00 str"@0xffffff8008a5ff04, +0x1c */
		parser_get_int_data(mp_items[index].c0, "max", str, 512);
		mp_items[index].c28 = katoi(str);
		/* chiave "min"@0xffffff8009244fe8;
		 * "b9002900 str"@0xffffff8008a5ff78, +0x28 */
		parser_get_int_data(mp_items[index].c0, "min", str, 512);
		mp_items[index].c40 = katoi(str);
	}

	/* "\x016ILITEK: (%s, %d): %s: run = %d, max = %d, min = %d, frame_count = %d\n"
	 * @0xffffff8009245045; "528197a2 mov"@0xffffff8008a5ffa4 --
	 * __LINE__ = 3261. Il campo `run` e' letto a UN byte
	 * ("39405d04 ldrb"@0xffffff8008a5ff90) */
#line 3261
	ILI_DBG("%s: run = %d, max = %d, min = %d, frame_count = %d\n", mp_items[index].c0, mp_items[index].c23, mp_items[index].c28, mp_items[index].c40, mp_items[index].c48);

	/* "\x016ILITEK: (%s, %d): v_tdf_1 = %d, v_tdf_2 = %d, h_tdf_1 = %d, h_tdf_2 = %d"
	 * @0xffffff800924508d -- SENZA "\n" finale, ed e' cosi' nel binario;
	 * "52819802 mov"@0xffffff8008a5ffd0 -- __LINE__ = 3264 */
#line 3264
	ILI_DBG("v_tdf_1 = %d, v_tdf_2 = %d, h_tdf_1 = %d, h_tdf_2 = %d", mp_items[index].c64, mp_items[index].c68, mp_items[index].c72, mp_items[index].c76);

	/* "\x016ILITEK: (%s, %d): Run MP Test Item : %s\n"@0xffffff80092450d8;
	 * "52819842 mov"@0xffffff8008a5ffe8 -- __LINE__ = 3266 */
#line 3266
	ILI_INFO("Run MP Test Item : %s\n", mp_items[index].c0);

	/* "f9404908 ldr"@0xffffff8008a5fffc (+0x90) +
	 * "d63f0100 blr"@0xffffff8008a60004 con w0 = l'indice */
	mp_items[index].c144(index);

	/* "940017e0 bl"@0xffffff8008a6000c verso
	 * <mp_comp_result_before_retry>; "36f80180 tbz"@0xffffff8008a60010
	 * prova il bit 31 */
	ret = mp_comp_result_before_retry(index);
	/* "396f6108 ldrb"@0xffffff8008a60018 + "34000128 cbz"@0xffffff8008a6001c */
	if (ret < 0 && core_mp.c216) {
		/* "\x016ILITEK: (%s, %d): MP failed, doing retry\n"
		 * @0xffffff8009245103; "52819902 mov"@0xffffff8008a60024 --
		 * __LINE__ = 3272 */
#line 3272
		ILI_INFO("MP failed, doing retry\n");
		/* "320007e1 orr"@0xffffff8008a60034 -- w1 = 3 */
		mp_do_retry(index, 3);
	}
}

/*
 * ===========================================================================
 * mp_print_csv_cdc_cmd -- righe 1124..1140
 * ===========================================================================
 * Scrive nel CSV il comando CDC dell'elemento, preso dalla sezione
 * "pv5_4 command" del file .ini. TRE casi, e i primi due sono cicli su una
 * tavola di nomi che il compilatore ha srotolato: il primo ha QUATTRO nomi
 * (le quattro `parser_get_ini_key_value` con "open dac"@0xffffff800924446b,
 * "open raw1"@0xffffff8009244474, "open raw2"@0xffffff800924447e e
 * "open raw3"@0xffffff8009244488), il secondo DUE
 * ("open cap1 dac"@0xffffff8009244492 e
 * "open cap1 raw"@0xffffff80092444a0).
 */
static void mp_print_csv_cdc_cmd(char *csv, int *csv_len, int index)
{
	int i, len = *csv_len;
	char str[128] = {0};
	char *open_sp_cmd[] = {"open dac", "open raw1", "open raw2", "open raw3"};
	char *open_c_cmd[] = {"open cap1 dac", "open cap1 raw"};
	char *name = mp_items[index].c0;

	/* "940fdbef bl"@0xffffff8008a61344 (strlen) +
	 * "f100641f cmp"@0xffffff8008a61354 (25) +
	 * "940fdc06 bl"@0xffffff8008a6136c (strncmp) con
	 * "open test(integration)_sp"@0xffffff800924319b */
	if (strlen(name) == strlen("open test(integration)_sp") &&
	    strncmp(name, "open test(integration)_sp",
		    strlen("open test(integration)_sp")) == 0) {
		for (i = 0; i < 4; i++) {
			if (parser_get_int_data("pv5_4 command", open_sp_cmd[i],
						str, 128) < 0) {
				/* "\x013ILITEK: (%s, %d): Failed to get CDC command %s from ini\n"
				 * @0xffffff8009245864, __func__ =
				 * "mp_print_csv_cdc_cmd"@0xffffff800924589f;
				 * "52808c82 mov"@0xffffff8008a613cc --
				 * __LINE__ = 1124 */
#line 1124
				ILI_ERR("Failed to get CDC command %s from ini\n", open_sp_cmd[i]);
				continue;
			}
			/* "%s = ,%s\n"@0xffffff80092458b4 */
			len += snprintf(csv + len, (CSV_FILE_SIZE - len),
					"%s = ,%s\n", open_sp_cmd[i], str);
		}
	/* "f1002c1f cmp"@0xffffff8008a61348 (11) +
	 * "940fdbe3 bl"@0xffffff8008a613f8 con
	 * "open test_c"@0xffffff80092431b5 */
	} else if (strlen(name) == strlen("open test_c") &&
		   strncmp(name, "open test_c", strlen("open test_c")) == 0) {
		for (i = 0; i < 2; i++) {
			if (parser_get_int_data("pv5_4 command", open_c_cmd[i],
						str, 128) < 0) {
				/* "52808da2 mov"@0xffffff8008a60c7c --
				 * __LINE__ = 1133 */
#line 1133
				ILI_ERR("Failed to get CDC command %s from ini\n", open_sp_cmd[i]);
				continue;
			}
			len += snprintf(csv + len, (CSV_FILE_SIZE - len),
					"%s = ,%s\n", open_c_cmd[i], str);
		}
	} else {
		if (parser_get_int_data("pv5_4 command", name, str, 128) < 0) {
			/* "52808e82 mov"@0xffffff8008a61494 -- __LINE__ = 1140 */
#line 1140
			ILI_ERR("Failed to get CDC command %s from ini\n", name);
			return;
		}
		/* "CDC command = ,%s\n"@0xffffff80092458be */
		len += snprintf(csv + len, (CSV_FILE_SIZE - len),
				"CDC command = ,%s\n", str);
	}

	*csv_len = len;
}

/*
 * ===========================================================================
 * mp_show_result -- righe 2858..3076
 * ===========================================================================
 * Costruisce il file CSV con l'esito di tutti gli elementi eseguiti e lo
 * scrive su /sdcard. E' la seconda funzione per lunghezza del blocco.
 *
 * DIFETTO DELLA FABBRICA, RIPRODOTTO (regola 7): il secondo argomento della
 * `snprintf` che compone il NOME DEL FILE non e' la capienza del nome ma la
 * lunghezza che RESTA nel buffer del CSV --
 * "320c03e8 orr"@0xffffff8008a61d58 (0x100000) meno la lunghezza corrente
 * ("4b130108 sub"@0xffffff8008a61d5c), passata poi in x1
 * ("aa1303e1 mov"@0xffffff8008a61d40). Il buffer del nome e' di 128 byte.
 */
static int mp_show_result(bool lcm_on)
{
	int i, j, ret = 0, len = 0, pass = 0, frame_cnt = 0, index = 0;
	char *csv = NULL, *csv_name = NULL;
	char str[128] = {0};
	int *max_threshold = NULL, *min_threshold = NULL;
	struct file *f = NULL;
	struct timespec64 ts;
	struct rtc_time tm;
	mm_segment_t old_fs;
	loff_t pos = 0;

	/* "320c03e0 orr"@0xffffff8008a60054 -- 0x100000 = 1 MiB;
	 * "97df8a05 bl"@0xffffff8008a6005c verso <vmalloc> */
	csv = vmalloc(CSV_FILE_SIZE);
	if (IS_ERR(csv) || csv == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate CSV mem\n"
		 * @0xffffff80092453c9, __func__ =
		 * "mp_show_result"@0xffffff80092453f9;
		 * "52816542 mov"@0xffffff8008a60758 -- __LINE__ = 2858 */
#line 2858
		ILI_ERR("Failed to allocate CSV mem\n");
		/* "12800d56 mov"@0xffffff8008a60768 -- -107 */
		ret = -107;
		goto out;
	}

	/*
	 * DUE `kcalloc`, non due `kmalloc`: la prova e'
	 * "37f84148 tbnz"@0xffffff8008a60084 (e la gemella
	 * "37f84168 tbnz"@0xffffff8008a600a4), che e' cio' in cui si riduce il
	 * controllo di traboccamento `n > SIZE_MAX / size` quando `size` e' la
	 * costante 4 e `n` un `int` esteso con segno. `kmalloc` non la genera.
	 * Le bandiere 0x14080c0 confermano: GFP_KERNEL | __GFP_ZERO.
	 */
	max_threshold = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	min_threshold = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	if (IS_ERR(max_threshold) || max_threshold == NULL ||
	    IS_ERR(min_threshold) || min_threshold == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate threshold FRAME buffer\n"
		 * @0xffffff800924512f; "52816642 mov"@0xffffff8008a60b50 --
		 * __LINE__ = 2866 */
#line 2866
		ILI_ERR("Failed to allocate threshold FRAME buffer\n");
		/* "12800d56 mov"@0xffffff8008a60b58 -- -107 */
		ret = -107;
		goto out;
	}

	/*
	 * "940005a0 bl"@0xffffff8008a60908 con "date"@0xffffff80091c8500 e
	 * "94000591 bl"@0xffffff8008a60944 con "version"@0xffffff800926923d.
	 * Sul ramo d'errore il compilatore scrive SETTE byte, non otto --
	 * "b9000269 str"@0xffffff8008a60924 (quattro) piu'
	 * "b8003268 stur"@0xffffff8008a60920 a +3 (altri quattro, con tre
	 * sovrapposti): e' una copia di lunghezza 7, cioe' SENZA il
	 * terminatore. Il buffer e' pero' gia' azzerato da `memset(&core_mp)`.
	 */
	if (parser_get_ini_key_value("pv5_4 command", "date", core_mp.c20) < 0)
		memcpy(core_mp.c20, "Unknown", strlen("Unknown"));

	if (parser_get_ini_key_value("pv5_4 command", "version", core_mp.c148) < 0)
		memcpy(core_mp.c148, "Unknown", strlen("Unknown"));

	/* "9410270f bl"@0xffffff8008a60984 e le dieci che seguono. Il secondo
	 * argomento e' sempre "0x100000 - len"
	 * ("4b130288 sub"@0xffffff8008a60974) */
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "==============================================================================\n");
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "ILITek C-TP Utility V%s\t%x : Driver Sensor Test\n", DRIVER_VERSION, core_mp.c0);
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Confidentiality Notice:\n");
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Any information of this tool is confidential and privileged.\n");
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "@ ILI TECHNOLOGY CORP. All Rights Reserved.\n");
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "==============================================================================\n");
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Firmware Version ,0x%x\n", core_mp.c8);
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Panel information ,XCH=%d, YCH=%d\n", core_mp.c232, core_mp.c236);
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "INI Release Version ,%s\n", core_mp.c20);
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "INI Release Date ,%s\n", core_mp.c148);
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Test Item:\n");

	/* "\t  ---%s\n"@0xffffff800924585a */
	for (i = 0; i < mp_run.num; i++)
		len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\t  ---%s\n", mp_items[mp_run.idx[i]].c0);

	/* "91002ec8 add"@0xffffff8008a60b24 (+11) +
	 * "7100491f cmp"@0xffffff8008a60b30 (#0x12 = 18) +
	 * "5400040d b.le"@0xffffff8008a60b38: il ciclo riempie la testata fino
	 * a diciannove righe */
	for (i = mp_run.num + 11; i < 19; i++)
		len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\n");

	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "==============================================================================\n");

	for (i = 0; i < mp_run.num; i++) {
		index = mp_run.idx[i];

		/* "b9402768 ldr"@0xffffff8008a611bc (+36) +
		 * "34000128 cbz"@0xffffff8008a611c4. I due messaggi NON hanno
		 * il prefisso "ILITEK: (%s, %d): ": sono `printk` nude con il
		 * livello KERN_INFO ("\x016\n[%s],OK \n"@0xffffff8009245408 e
		 * "\x016\n[%s],NG \n"@0xffffff800924541f), e le due stringhe
		 * scritte nel CSV sono altre due
		 * ("\n[%s],OK\n"@0xffffff8009245415 e
		 * "\n[%s],NG\n"@0xffffff800924542c) -- differiscono per lo
		 * SPAZIO prima dell'a-capo */
		if (mp_items[index].c36 == 0) {
			printk(KERN_INFO "\n[%s],OK \n", mp_items[index].c0);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\n[%s],OK\n", mp_items[index].c0);
		} else {
			printk(KERN_INFO "\n[%s],NG \n", mp_items[index].c0);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\n[%s],NG\n", mp_items[index].c0);
		}

		/* "b8410d48 ldr"@0xffffff8008a6122c (pre-indice su +0x10) +
		 * "7100291f cmp"@0xffffff8008a61230 (#0xa) */
		if (mp_items[index].c16 == 10) {
			printk(KERN_INFO "Test INT Pin = %d\n", mp_items[index].test_int_pin);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Test INT Pin = %d\n", mp_items[index].test_int_pin);
			printk(KERN_INFO "Pulse Test = %d\n", mp_items[index].int_pulse_test);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Pulse Test = %d\n", mp_items[index].int_pulse_test);
			printk(KERN_INFO "Delay Time = %d\n", mp_items[index].delay_time);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Delay Time = %d\n", mp_items[index].delay_time);
			continue;
		}

		mp_print_csv_cdc_cmd(csv, &len, index);

		/* "b8430e81 ldr"@0xffffff8008a614c8 (pre-indice su +0x30) */
		printk(KERN_INFO "Frame count = %d\n", mp_items[index].c48);
		len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Frame count = %d\n", mp_items[index].c48);

		/* "b8434f08 ldr"@0xffffff8008a61508 (pre-indice su +0x34) +
		 * "34000508 cbz"@0xffffff8008a6150c, e
		 * "7100211f cmp"@0xffffff8008a61518 (#0x8) */
		if (mp_items[index].c52 != 0 && mp_items[index].c16 != 8) {
			printk(KERN_INFO "lowest percentage = %d\n", mp_items[index].c56);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "lowest percentage = %d\n", mp_items[index].c56);
			printk(KERN_INFO "highest percentage = %d\n", mp_items[index].c60);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "highest percentage = %d\n", mp_items[index].c60);
		}

		/* "39405508 ldrb"@0xffffff8008a615b0 (+0x15) +
		 * "7100051f cmp"@0xffffff8008a615c0 */
		if (mp_items[index].c21 == 1) {
			/* "f8478d01 ldr"@0xffffff8008a615cc (+120) e
			 * "f940014d ldr"@0xffffff8008a615ec (+128): i due
			 * puntatori sono RILETTI a ogni giro */
			for (j = 0; j < core_mp.c256; j++) {
				max_threshold[j] = mp_items[index].bench_mark_max[j];
				min_threshold[j] = mp_items[index].bench_mark_min[j];
			}
			/* "940015bb bl"@0xffffff8008a61634, w4 = 0
			 * ("2a1f03e4 mov"@0xffffff8008a61624),
			 * x7 = "Max_Bench"@0xffffff80092454ba */
			mp_compare_cdc_show_result(index, mp_items[index].bench_mark_max, csv, &len, 0, max_threshold, min_threshold, "Max_Bench");
			/* x7 = "Min_Bench"@0xffffff80092454c4 */
			mp_compare_cdc_show_result(index, mp_items[index].bench_mark_min, csv, &len, 0, max_threshold, min_threshold, "Min_Bench");
		} else {
			/* "b841ce61 ldr"@0xffffff8008a6167c (+28) e
			 * "b940012a ldr"@0xffffff8008a61698 (+40) */
			for (j = 0; j < core_mp.c256; j++) {
				max_threshold[j] = mp_items[index].c28;
				min_threshold[j] = mp_items[index].c40;
			}
			printk(KERN_INFO "Max = %d\n", mp_items[index].c28);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Max = %d\n", mp_items[index].c28);
			printk(KERN_INFO "Min = %d\n", mp_items[index].c40);
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Min = %d\n", mp_items[index].c40);
		}

		/* "f100641f cmp"@0xffffff8008a6175c (25) +
		 * "940fdb04 bl"@0xffffff8008a61774 */
		if (strlen(mp_items[index].c0) == strlen("open test(integration)_sp") &&
		    strncmp(mp_items[index].c0, "open test(integration)_sp",
			    strlen("open test(integration)_sp")) == 0) {
			/* "f9421901 ldr"@0xffffff8008a61784 (0xffffff800a0ff430),
			 * x7 = "frame1 cbk700"@0xffffff80092454e6 */
			mp_compare_cdc_show_result(index, frame1_cbk700, csv, &len, 1, max_threshold, min_threshold, "frame1 cbk700");
			/* "f9421d01 ldr"@0xffffff8008a617b0,
			 * x7 = "frame1 cbk250"@0xffffff80092454f4 */
			mp_compare_cdc_show_result(index, frame1_cbk250, csv, &len, 1, max_threshold, min_threshold, "frame1 cbk250");
			/* "f9422101 ldr"@0xffffff8008a617dc,
			 * x7 = "frame1 cbk200"@0xffffff8009245502 */
			mp_compare_cdc_show_result(index, frame1_cbk200, csv, &len, 1, max_threshold, min_threshold, "frame1 cbk200");
		}

		/* "f1002c1f cmp"@0xffffff8008a61810 (11) +
		 * "940fdad7 bl"@0xffffff8008a61828 */
		if (strlen(mp_items[index].c0) == strlen("open test_c") &&
		    strncmp(mp_items[index].c0, "open test_c",
			    strlen("open test_c")) == 0) {
			/* "f9422501 ldr"@0xffffff8008a61838,
			 * x7 = "CAP_DAC"@0xffffff8009245510 */
			mp_compare_cdc_show_result(index, cap_dac, csv, &len, 1, max_threshold, min_threshold, "CAP_DAC");
			/* "f9422901 ldr"@0xffffff8008a61864,
			 * x7 = "CAP_RAW"@0xffffff8009245518 */
			mp_compare_cdc_show_result(index, cap_raw, csv, &len, 1, max_threshold, min_threshold, "CAP_RAW");
		}

		/* "7100111f cmp"@0xffffff8008a61894 (#0x4) */
		if (mp_items[index].c16 == 4) {
			/* "f9461d08 ldr"@0xffffff8008a618a0 legge PRIMA
			 * `rx_delta_buf` e "f9461908 ldr"@0xffffff8008a618b8
			 * POI `tx_delta_buf` -- l'ordine e' SUGGERITO dal
			 * binario, non misurato: sono due carichi puri dentro
			 * un `||` e il compilatore puo' scambiarli */
			if (IS_ERR(core_mp.rx_delta_buf) || core_mp.rx_delta_buf == NULL ||
			    IS_ERR(core_mp.tx_delta_buf) || core_mp.tx_delta_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): This test item (%s) has no data inside its buffer\n"
				 * @0xffffff800924518a;
				 * "52817062 mov"@0xffffff8008a61afc --
				 * __LINE__ = 2947 */
#line 2947
				ILI_ERR("This test item (%s) has no data inside its buffer\n", mp_items[index].c0);
				continue;
			}
			/* "29442a69 ldp"@0xffffff8008a618e4 legge `c288` e
			 * `c292` INSIEME */
			for (j = 0; j < core_mp.c256; j++) {
				max_threshold[j] = core_mp.c288;
				min_threshold[j] = core_mp.c292;
			}
			/* x7 = "TX Max Hold"@0xffffff8009245547 e
			 * "TX Min Hold"@0xffffff8009245553, w4 = 2 */
			mp_compare_cdc_show_result(index, core_mp.tx_max_buf, csv, &len, 2, max_threshold, min_threshold, "TX Max Hold");
			mp_compare_cdc_show_result(index, core_mp.tx_min_buf, csv, &len, 2, max_threshold, min_threshold, "TX Min Hold");
			/* "29452a69 ldp"@0xffffff8008a61964 -- `c296` e `c300` */
			for (j = 0; j < core_mp.c256; j++) {
				max_threshold[j] = core_mp.c296;
				min_threshold[j] = core_mp.c300;
			}
			/* x7 = "RX Max Hold"@0xffffff800924555f e
			 * "RX Min Hold"@0xffffff800924556b */
			mp_compare_cdc_show_result(index, core_mp.rx_max_buf, csv, &len, 2, max_threshold, min_threshold, "RX Max Hold");
			mp_compare_cdc_show_result(index, core_mp.rx_min_buf, csv, &len, 2, max_threshold, min_threshold, "RX Min Hold");
			continue;
		}

		/* "f8460f89 ldr"@0xffffff8008a619f0 (+96, pre-indice),
		 * "f9403521 ldr"@0xffffff8008a61a0c (+104),
		 * "f8470e69 ldr"@0xffffff8008a61a28 (+112, pre-indice) */
		if (IS_ERR(mp_items[index].buf) || mp_items[index].buf == NULL ||
		    IS_ERR(mp_items[index].max_buf) || mp_items[index].max_buf == NULL ||
		    IS_ERR(mp_items[index].min_buf) || mp_items[index].min_buf == NULL) {
			/* "52817122 mov"@0xffffff8008a61ad8 -- __LINE__ = 2953 */
#line 2953
			ILI_ERR("This test item (%s) has no data inside its buffer\n", mp_items[index].c0);
			continue;
		}

		/* "7100091f cmp"@0xffffff8008a61a38 (#0x2) */
		if (mp_items[index].c16 == 2) {
			/*
			 * I DUE LETTERALI SONO DIVERSI, e la differenza e' la
			 * virgola: quello stampato sul terminale e'
			 * "KEY_%02d "@0xffffff8009245522 (la CODA della stringa
			 * KERN_CONT che comincia a 0xffffff8009245520, due byte
			 * prima), quello scritto nel CSV e'
			 * "KEY_%02d,"@0xffffff800924552c. Il conto dei giri e'
			 * "b94bfaa9 ldr"@0xffffff8008a61a4c, cioe'
			 * `core_mp.key_len`.
			 */
			for (j = 0; j < core_mp.key_len; j++) {
				ILI_CONT("KEY_%02d ", j);
				len += snprintf(csv + len, (CSV_FILE_SIZE - len), "KEY_%02d,", j);
			}
			ILI_CONT("\n");
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\n");
			/* " %3d   "@0xffffff8009245538 (coda di
			 * 0xffffff8009245536) e " %3d, "@0xffffff8009245540 */
			for (j = 0; j < core_mp.key_len; j++) {
				ILI_CONT(" %3d   ", mp_items[index].buf[j]);
				len += snprintf(csv + len, (CSV_FILE_SIZE - len), " %3d, ", mp_items[index].buf[j]);
			}
			ILI_CONT("\n");
			len += snprintf(csv + len, (CSV_FILE_SIZE - len), "\n");
			continue;
		}

		/* "7100211f cmp"@0xffffff8008a60f48 (#0x8) +
		 * "b9400308 ldr"@0xffffff8008a60f50 (`c52`) */
		if (mp_items[index].c16 != 8 && mp_items[index].c52 != 0) {
			/* "f9402d01 ldr"@0xffffff8008a60f68 (+88),
			 * x7 = "Mean result"@0xffffff8009245577 */
			mp_compare_cdc_show_result(index, mp_items[index].result_buf, csv, &len, 2, max_threshold, min_threshold, "Mean result");
		} else {
			/* x7 = "Max Hold"@0xffffff8009245562 e
			 * "Min Hold"@0xffffff800924556e */
			mp_compare_cdc_show_result(index, mp_items[index].max_buf, csv, &len, 2, max_threshold, min_threshold, "Max Hold");
			mp_compare_cdc_show_result(index, mp_items[index].min_buf, csv, &len, 2, max_threshold, min_threshold, "Min Hold");
		}

		/* "320003f4 orr"@0xffffff8008a60ff0 -- 1 */
		if (mp_items[index].c16 == 8)
			frame_cnt = 1;
		else
			frame_cnt = mp_items[index].c48;

		/* "Frame %d"@0xffffff8009245583;
		 * "1b157d08 mul"@0xffffff8008a61088 (core_mp.c256 * j) +
		 * "8b28c921 add"@0xffffff8008a6108c (`lsl #2`) */
		for (j = 0; j < frame_cnt; j++) {
			memset(str, 0, sizeof(str));
			snprintf(str, (CSV_FILE_SIZE - len), "Frame %d", j + 1);
			mp_compare_cdc_show_result(index, &mp_items[index].buf[core_mp.c256 * j], csv, &len, 1, max_threshold, min_threshold, str);
		}
	}

	/*
	 * NON e' `str` che viene azzerato qui, ed e' MISURATO: la fabbrica
	 * azzera i 128 byte del GLOBALE `csv_path` (0xffffff800a0fcc70) con
	 * otto `stp xzr, xzr` srotolate --
	 *
	 *   ffffff8008a61b34:	f000b4c8 	adrp	x8, ffffff800a0fc000
	 *   ffffff8008a61b38:	9131c108 	add	x8, x8, #0xc70
	 *   ffffff8008a61b40:	a9077d1f 	stp	xzr, xzr, [x8,#112]
	 *   ffffff8008a61b5c:	a9007d1f 	stp	xzr, xzr, [x8]
	 *
	 * e 0xffffff800a0fcc70 e' `csv_path`, non un vettore di pila: lo stesso
	 * indirizzo che "9131c000 add"@0xffffff8008a61d10 usa come destinazione
	 * della `snprintf` che compone il nome del file. La prima stesura del
	 * lotto F5 scriveva `memset(str, 0, sizeof(str))` e il nostro oggetto
	 * emetteva SEDICI "str xzr, [sp,#...]" invece delle otto `stp` verso il
	 * globale: 128 byte azzerati nel posto sbagliato. CORRETTO il
	 * 2026-08-22.
	 */
	memset(csv_path, 0, sizeof(csv_path));
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "==============================================================================\n");
	/* "Result_Summary\t\t\t\n"@0xffffff8009245983 */
	len += snprintf(csv + len, (CSV_FILE_SIZE - len), "Result_Summary\t\t\t\n");

	/* "9a970362 csel"@0xffffff8008a61c00 sceglie fra
	 * "\t  {%s}\t   ,OK\n"@0xffffff8009245996 (condizione `eq`, cioe'
	 * `c36 == 0`) e "\t  {%s}\t   ,NG\n"@0xffffff80092459a6 */
	for (i = 0; i < mp_run.num; i++) {
		index = mp_run.idx[i];
		len += snprintf(csv + len, (CSV_FILE_SIZE - len), (mp_items[index].c36 == 0) ? "\t  {%s}\t   ,OK\n" : "\t  {%s}\t   ,NG\n", mp_items[index].c0);
	}

	/*
	 * IL CONTEGGIO FINALE. "385f314b ldurb"@0xffffff8008a61c3c legge `c23`
	 * (UN byte) e "b940014b ldr"@0xffffff8008a61c44 legge `c36` (quattro),
	 * con "37f802cb tbnz"@0xffffff8008a61c48 sul bit 31. Il giro e' su
	 * TUTTI e cinquanta gli elementi ("f100c93f cmp"@0xffffff8008a61c54),
	 * non sui soli eseguiti.
	 */
	for (i = 0; i < MP_TEST_ITEM; i++) {
		if (!mp_items[i].c23)
			continue;
		if (mp_items[i].c36 < 0)
			goto fail;
		pass++;
	}

	/* "34000208 cbz"@0xffffff8008a61c60 */
	if (pass == 0)
		goto fail;

	/* "b900011f str"@0xffffff8008a61c70 */
	core_mp.c264 = 0;
	/* "39418509 ldrb"@0xffffff8008a61c6c (+97 su 0xffffff800a0fcc08, cioe'
	 * `core_mp` + 361) + "340006e9 cbz"@0xffffff8008a61c74 */
	if (core_mp.c361) {
		/* "\x013ILITEK: (%s, %d): WARNING! Golden and SPEC in ini file aren't matched!!\n"
		 * @0xffffff80092455ef; "52817c22 mov"@0xffffff8008a61c88 --
		 * __LINE__ = 3041 */
#line 3041
		ILI_ERR("WARNING! Golden and SPEC in ini file aren't matched!!\n");
		/* "mp_warning"@0xffffff80092455e4 */
		csv_name = "mp_warning";
	} else {
		/* "mp_pass"@0xffffff800924558c */
		csv_name = "mp_pass";
	}

	/*
	 * IL BLOCCO DELL'ORA E DEL NOME E' SCRITTO DUE VOLTE, ed e' il
	 * sorgente a scriverlo due volte, non il compilatore a duplicarlo. LA
	 * PROVA e' il quinto argomento della `snprintf`: sul ramo di guasto e'
	 * una COSTANTE ("911650a5 add"@0xffffff8008a61d3c materializza
	 * "mp_fail"@0xffffff8009245594), su quello di esito buono e' una
	 * VARIABILE ("aa1503e5 mov"@0xffffff8008a61de8, x5 = x21). Se il
	 * sorgente avesse una sola `snprintf` con un puntatore scelto prima,
	 * anche il ramo di guasto passerebbe un registro.
	 */
	getnstimeofday64(&ts);
	rtc_time64_to_tm(ts.tv_sec, &tm);
	/* "%04d%02d%02d-%02d%02d%02d"@0xffffff80092459b6, 128 byte
	 * ("321903e1 orr"@0xffffff8008a61da8) */
	snprintf(csv_time, 128, "%04d%02d%02d-%02d%02d%02d",
		 (int)tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		 tm.tm_hour, tm.tm_min, tm.tm_sec);
	/* "36000096 tbz"@0xffffff8008a61dc4 prova il BIT 0 di `lcm_on`;
	 * "%s/%s_%s.csv"@0xffffff800924559c */
	snprintf(csv_path, (CSV_FILE_SIZE - len), "%s/%s_%s.csv",
		 lcm_on ? "/sdcard/ilitek_mp_lcm_on_log" :
			  "/sdcard/ilitek_mp_lcm_off_log",
		 csv_name, csv_time);
	/* "2a1f03f6 mov"@0xffffff8008a61df0 */
	ret = 0;
	goto write;

fail:
	/* "b90c0909 str"@0xffffff8008a61cac -- w9 = -1 */
	core_mp.c264 = -1;

	/* "97dbcdd9 bl"@0xffffff8008a61cbc verso <getnstimeofday64> e
	 * "94008933 bl"@0xffffff8008a61cc8 verso <rtc_time64_to_tm>;
	 * "111db103 add"@0xffffff8008a61cec (+1900) e
	 * "11000524 add"@0xffffff8008a61cf0 (+1) */
	getnstimeofday64(&ts);
	rtc_time64_to_tm(ts.tv_sec, &tm);
	snprintf(csv_time, 128, "%04d%02d%02d-%02d%02d%02d",
		 (int)tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		 tm.tm_hour, tm.tm_min, tm.tm_sec);
	/* "36000096 tbz"@0xffffff8008a61d18; "mp_fail"@0xffffff8009245594 */
	snprintf(csv_path, (CSV_FILE_SIZE - len), "%s/%s_%s.csv",
		 lcm_on ? "/sdcard/ilitek_mp_lcm_on_log" :
			  "/sdcard/ilitek_mp_lcm_off_log",
		 "mp_fail", csv_time);
	/* "12800016 mov"@0xffffff8008a61d48 */
	ret = -1;

write:
	/* "\x016ILITEK: (%s, %d): Open CSV : %s\n"@0xffffff800924563a;
	 * "52817d42 mov"@0xffffff8008a61e0c -- __LINE__ = 3050 */
#line 3050
	ILI_INFO("Open CSV : %s\n", csv_path);

	/* "97e00f0e bl"@0xffffff8008a61e24 verso <filp_open>, w1 = 577
	 * ("52804821 mov"@0xffffff8008a61e18) e w2 = 644 */
	f = filp_open(csv_path, O_WRONLY | O_CREAT | O_TRUNC, 644);
	if (IS_ERR(f) || f == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to open CSV file"
		 * @0xffffff800924565d; "52817e02 mov"@0xffffff8008a61e90 --
		 * __LINE__ = 3056 */
#line 3056
		ILI_ERR("Failed to open CSV file");
		/* Il ramo non ha una `mov` propria: "17fffa34 b"@0xffffff8008a61e98
		 * salta a "12800d56 mov"@0xffffff8008a60768, cioe' -107 */
		ret = -107;
		goto out;
	}

	/* "\x016ILITEK: (%s, %d): Open CSV succeed, its length = %d\n "
	 * @0xffffff8009245689; "52817ea2 mov"@0xffffff8008a61e4c --
	 * __LINE__ = 3061 */
#line 3061
	ILI_INFO("Open CSV succeed, its length = %d\n ", len);

	/* "7144011f cmp"@0xffffff8008a61e58 -- 0x100 << 12 = 0x100000 */
	if (len >= CSV_FILE_SIZE) {
		/* "52817f02 mov"@0xffffff8008a61e70 -- __LINE__ = 3064 */
#line 3064
		ILI_ERR("The length saved to CSV is too long !\n");
		/* "12800d16 mov"@0xffffff8008a61e78 -- -105 */
		ret = -105;
		goto out;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	/* "97e0148f bl"@0xffffff8008a61ed8 verso <vfs_write> */
	vfs_write(f, csv, len, &pos);
	set_fs(old_fs);
	/* "97e01014 bl"@0xffffff8008a61f10 verso <filp_close> */
	filp_close(f, NULL);

	/* "\x016ILITEK: (%s, %d): Writing Data into CSV succeed\n"
	 * @0xffffff80092456fc; "52818082 mov"@0xffffff8008a61f24 --
	 * __LINE__ = 3076 */
#line 3076
	ILI_INFO("Writing Data into CSV succeed\n");

out:
	/* "b4000078 cbz"@0xffffff8008a60770 + "97df86b0 bl"@0xffffff8008a60778 */
	if (csv)
		vfree(csv);
	if (max_threshold)
		kfree(max_threshold);
	if (min_threshold)
		kfree(min_threshold);
	return ret;
}

/*
 * ===========================================================================
 * mp_copy_ret_to_apk -- righe 3326..3337
 * ===========================================================================
 * Ricopia nel buffer dell'applicazione il percorso del CSV e l'esito di ogni
 * elemento eseguito.
 *
 * IL DUE INIZIALE E' MISURATO: "91000a60 add"@0xffffff8008a607a0 scrive a
 * partire da `buf + 2` e "11000813 add"@0xffffff8008a607bc riporta la
 * lunghezza a `len + 2`; il secondo argomento della `snprintf` e' 0xffe =
 * 4094 ("321f2be1 orr"@0xffffff8008a607ac), cioe' 4096 - 2. I due byte
 * saltati NON vengono mai scritti da questa funzione.
 */
static void mp_copy_ret_to_apk(char *buf)
{
	int i, len = 0, index = 0;

	/* "b40007f3 cbz"@0xffffff8008a60794 */
	if (buf == NULL) {
		/* "\x013ILITEK: (%s, %d): apk buffer is null\n"
		 * @0xffffff80092459d0, __func__ =
		 * "mp_copy_ret_to_apk"@0xffffff80092459f8;
		 * "52819fc2 mov"@0xffffff8008a608a0 -- __LINE__ = 3326 */
#line 3326
		ILI_ERR("apk buffer is null\n");
		return;
	}

	/* "CSV path: %s\n\n"@0xffffff8009245a0b */
	len = snprintf(buf + 2, (APK_BUF_SIZE - 2), "CSV path: %s\n\n", csv_path);
	len += 2;

	for (i = 0; i < mp_run.num; i++) {
		index = mp_run.idx[i];
		/* "3100051f cmn"@0xffffff8008a60818 -- il confronto con -1 */
		if (mp_items[index].c36 == -1) {
			/* "\x013ILITEK: (%s, %d): [%s] = FAIL\n"
			 * @0xffffff8009245a1a; "5281a0c2 mov"@0xffffff8008a6083c
			 * -- __LINE__ = 3334 */
#line 3334
			ILI_ERR("[%s] = FAIL\n", mp_items[index].c0);
			/* "[%s] = FAIL\n"@0xffffff8009245a2e */
			len += snprintf(buf + len, (APK_BUF_SIZE - len), "[%s] = FAIL\n", mp_items[index].c0);
		} else {
			/* "\x016ILITEK: (%s, %d): [%s] = PASS\n"
			 * @0xffffff8009245a3b; "5281a122 mov"@0xffffff8008a60820
			 * -- __LINE__ = 3337 */
#line 3337
			ILI_INFO("[%s] = PASS\n", mp_items[index].c0);
			/* "[%s] = PASS\n"@0xffffff8009245a4f */
			len += snprintf(buf + len, (APK_BUF_SIZE - len), "[%s] = PASS\n", mp_items[index].c0);
		}
	}

	/* "b9027113 str"@0xffffff8008a60888 -- `idev` + 624 */
	idev->c624 = len;
}

/*
 * ===========================================================================
 * mp_test_free -- riga 3282
 * ===========================================================================
 * L'ordine delle liberazioni e' quello del binario, e non e' l'ordine delle
 * allocazioni: prima i tre vettori di `mp_frame_buf` che il collaudo "open
 * test sp" riempie (indici 2, 3, 4), poi i due che nessuna funzione scritta
 * legge (indici 0 e 1).
 */
static void mp_test_free(void)
{
	int i;

	/* "\x016ILITEK: (%s, %d): Free all allocated mem for MP\n"
	 * @0xffffff8009245a5c, __func__ = "mp_test_free"@0xffffff8009245a8f;
	 * "52819a42 mov"@0xffffff8008a5f010 -- __LINE__ = 3282.
	 * NON e' guardata da `ilitek_dbg_en`: nessun `ldrb` la precede */
#line 3282
	ILI_INFO("Free all allocated mem for MP\n");

	/* "b90c0913 str"@0xffffff8008a5f044 -- w19 = -1 */
	core_mp.c264 = -1;

	for (i = 0; i < MP_TEST_ITEM; i++) {
		/* "381bf29f sturb"@0xffffff8008a5f04c */
		mp_items[i].c23 = 0;
		/* "f81c8293 stur"@0xffffff8008a5f050 -- OTTO byte a +32, cioe'
		 * `c32` e `c36` insieme, tutti e due a -1 */
		mp_items[i].c32 = -1;
		mp_items[i].c36 = -1;
		/* "b81d4293 stur"@0xffffff8008a5f054 */
		mp_items[i].c44 = -1;

		/* "b85b8288 ldur"@0xffffff8008a5f048 (+16) +
		 * "7100111f cmp"@0xffffff8008a5f058 (#0x4) */
		if (mp_items[i].c16 == 4) {
			/* "f9461ee0 ldr"@0xffffff8008a5f060 -- 0xffffff800a0fcc38 */
			ipio_kfree((void **)&core_mp.rx_delta_buf);
			/* "f9461b00 ldr"@0xffffff8008a5f070 -- 0xffffff800a0fcc30 */
			ipio_kfree((void **)&core_mp.tx_delta_buf);
			ipio_kfree((void **)&core_mp.tx_max_buf);
			ipio_kfree((void **)&core_mp.tx_min_buf);
			ipio_kfree((void **)&core_mp.rx_max_buf);
			ipio_kfree((void **)&core_mp.rx_min_buf);
			continue;
		}

		/* "385bd288 ldurb"@0xffffff8008a5f0c4 (+21) +
		 * "7100051f cmp"@0xffffff8008a5f0c8 */
		if (mp_items[i].c21 == 1) {
			/* "f9401280 ldr"@0xffffff8008a5f0d0 (+120) e
			 * "f9401680 ldr"@0xffffff8008a5f0e0 (+128) */
			ipio_kfree((void **)&mp_items[i].bench_mark_max);
			ipio_kfree((void **)&mp_items[i].bench_mark_min);
		}
		/* "f9401a80 ldr"@0xffffff8008a5f0f0 (+136) */
		ipio_kfree((void **)&mp_items[i].c136);
		/* "f85b0280 ldur"@0xffffff8008a5f100 (+8) */
		ipio_kfree((void **)&mp_items[i].c8);
		/* "f9400280 ldr"@0xffffff8008a5f110 (+88) */
		ipio_kfree((void **)&mp_items[i].result_buf);
		/* "f9400a80 ldr"@0xffffff8008a5f120 (+104) */
		ipio_kfree((void **)&mp_items[i].max_buf);
		/* "f9400e80 ldr"@0xffffff8008a5f130 (+112) */
		ipio_kfree((void **)&mp_items[i].min_buf);
		/* "f9400680 ldr"@0xffffff8008a5f140 (+96) +
		 * "97df8c3c bl"@0xffffff8008a5f148 verso <vfree> */
		ipio_vfree((void **)&mp_items[i].buf);
	}

	/* "f9421a60 ldr"@0xffffff8008a5f160 -- 0xffffff800a0ff430 */
	ipio_kfree((void **)&frame1_cbk700);
	/* "f9421e60 ldr"@0xffffff8008a5f174 -- 0xffffff800a0ff438 */
	ipio_kfree((void **)&frame1_cbk250);
	/* "f9422260 ldr"@0xffffff8008a5f18c -- 0xffffff800a0ff440 */
	ipio_kfree((void **)&frame1_cbk200);
	/* "f9421260 ldr"@0xffffff8008a5f1a0 -- 0xffffff800a0ff420 */
	ipio_kfree((void **)&mp_frame_buf0);
	/* "f9421660 ldr"@0xffffff8008a5f1b4 -- 0xffffff800a0ff428 */
	ipio_kfree((void **)&mp_frame_buf1);
}

/*
 * ===========================================================================
 * ilitek_tddi_mp_test_main -- 0xffffff8008a5e618, 14704 byte, `T`
 * ===========================================================================
 * IL SOLO CHIAMANTE IN TUTTO IL KERNEL e' `ilitek_tddi_mp_test_handler`
 * (ricognizione §1, prova 4). La firma viene da li' e dall'uso: x0 e' il
 * buffer dell'applicazione (`mp_copy_ret_to_apk` lo prova contro NULL,
 * "b40007f3 cbz"@0xffffff8008a60794) e w1 e' un valore provato al BIT 0
 * ("36000096 tbz"@0xffffff8008a61d18), cioe' un `bool`.
 * `ilitek.h` la dichiara gia': `int ilitek_tddi_mp_test_handler(char *apk,
 * bool lcm_on)`; questa ha la stessa forma.
 */
int ilitek_tddi_mp_test_main(char *apk, bool lcm_on)
{
	int i, ret = 0;
	char *val = NULL, *hex = NULL;

	/* "321903e0 orr"@0xffffff8008a5e644 (128) +
	 * "97df9088 bl"@0xffffff8008a5e650, e la coppia gemella a
	 * 0xffffff8008a5e658/0xffffff8008a5e65c. I NOMI SONO SCELTI */
	val = vmalloc(128);
	hex = vmalloc(128);

	/* "3943e103 ldrb"@0xffffff8008a5e668 e
	 * "3943e504 ldrb"@0xffffff8008a5e66c leggono `idev` a +248 e +249, UN
	 * byte ciascuno; "34001243 cbz"@0xffffff8008a5e674 e
	 * "34001224 cbz"@0xffffff8008a5e678 */
	if (idev->c248 == 0 || idev->c249 == 0) {
		/* "\x013ILITEK: (%s, %d): Invalid frame length (%d, %d)\n"
		 * @0xffffff8009243282, __func__ =
		 * "ilitek_tddi_mp_test_main"@0xffffff80092432b5;
		 * "5281a702 mov"@0xffffff8008a5e8cc -- __LINE__ = 3384 */
#line 3384
		ILI_ERR("Invalid frame length (%d, %d)\n", idev->c248, idev->c249);
		/* "12800d16 mov"@0xffffff8008a5e8d4 -- -105 */
		ret = -105;
		goto out;
	}

	/* "52982400 mov"@0xffffff8008a5e67c + "72a00780 movk"@0xffffff8008a5e680
	 * -- 0x3cc120 = 3981600 = 2212 * 1800, cioe'
	 * sizeof(struct ilitek_ini_item) * PARSER_MAX_KEY_NUM.
	 * "f9020500 str"@0xffffff8008a5e690 -- 0xffffff800a0ff408 */
	ilitek_ini_data = vmalloc(sizeof(struct ilitek_ini_item) * PARSER_MAX_KEY_NUM);
	if (IS_ERR(ilitek_ini_data) || ilitek_ini_data == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to malloc ini_info\n"
		 * @0xffffff80092432ce; "5281a7e2 mov"@0xffffff8008a5e8ec --
		 * __LINE__ = 3391 */
#line 3391
		ILI_ERR("Failed to malloc ini_info\n");
		/* "12800d56 mov"@0xffffff8008a5e8f4 -- -107 */
		ret = -107;
		goto out;
	}

	ilitek_tddi_mp_init_item();

	if (ilitek_tddi_mp_ini_parser() < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to parsing INI file\n"
		 * @0xffffff80092432fd; "5281a902 mov"@0xffffff8008a5efd4 --
		 * __LINE__ = 3400 */
#line 3400
		ILI_ERR("Failed to parsing INI file\n");
		/* "12800cd6 mov"@0xffffff8008a5efdc -- -103 */
		ret = -103;
		goto out;
	}

	/* "94000cd2 bl"@0xffffff8008a5ec40 con
	 * "pv5_4 command"@0xffffff800924332d e
	 * "protocol"@0xffffff80090e4a28 */
	parser_get_ini_key_value("pv5_4 command", "protocol", val);
	/* "94102e59 bl"@0xffffff8008a5ec5c, w1 = 8
	 * ("321d03e1 orr"@0xffffff8008a5ec50), x2 = "0x%s"@0xffffff800924333b */
	snprintf(hex, 8, "0x%s", val);
	/* "94003231 bl"@0xffffff8008a5ec64 verso <str2hex> e
	 * "6b48201f cmp"@0xffffff8008a5ec70 -- `core_mp.c12` spostato di OTTO
	 * bit a destra */
	if (str2hex(hex) != (core_mp.c12 >> 8)) {
		/* "\x013ILITEK: (%s, %d): ERROR! MP Protocol version is invaild, 0x%x\n"
		 * @0xffffff8009243340; "5281aa22 mov"@0xffffff8008a5ef9c --
		 * __LINE__ = 3409. `str2hex` E' CHIAMATA DUE VOLTE: la seconda
		 * e' "94003169 bl"@0xffffff8008a5ef84, e il suo risultato e'
		 * l'argomento `%x` */
#line 3409
		ILI_ERR("ERROR! MP Protocol version is invaild, 0x%x\n", str2hex(hex));
		/* "12800c96 mov"@0xffffff8008a5efa4 -- -101 */
		ret = -101;
		goto out;
	}

	if (mp_get_timing_info() < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to get timing info from ini\n"
		 * @0xffffff8009243381; "5281ab02 mov"@0xffffff8008a5eff4 --
		 * __LINE__ = 3416 */
#line 3416
		ILI_ERR("Failed to get timing info from ini\n");
		/* "12800cf6 mov"@0xffffff8008a5effc -- -104 */
		ret = -104;
		goto out;
	}

	if (mp_sort_item(lcm_on) < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to sort test item\n"
		 * @0xffffff80092433b9; "5281abe2 mov"@0xffffff8008a606c0 --
		 * __LINE__ = 3423. Il salto
		 * "17fffa45 b"@0xffffff8008a606c4 riusa la `printk` del ramo
		 * di riga 3400 */
#line 3423
		ILI_ERR("Failed to sort test item\n");
		/* Il ramo riusa la `printk` di riga 3400 e con essa la sua
		 * costante: "12800cd6 mov"@0xffffff8008a5efdc -- -103 */
		ret = -103;
		goto out;
	}

	/* "f000b4e8 adrp"@0xffffff8008a60040 +
	 * "b9845908 ldrsw"@0xffffff8008a60044 -- il limite e' RILETTO a ogni
	 * giro, quindi nel sorgente sta dentro la condizione del `for` e non
	 * in un locale estratto (classe A2, letta al contrario) */
	for (i = 0; i < mp_run.num; i++)
		mp_test_run(mp_run.idx[i]);

	ret = mp_show_result(lcm_on);
	mp_copy_ret_to_apk(apk);

out:
	mp_test_free();

	/* "a94453f5 ldp"@0xffffff8008a5f178 rilegge i due puntatori dalla
	 * pila; "b4000074 cbz"@0xffffff8008a5f1c4 e
	 * "b4000075 cbz"@0xffffff8008a5f1d0. NON viene rimesso a NULL nessuno
	 * dei due: sono locali */
	if (val)
		vfree(val);
	if (hex)
		vfree(hex);

	/* "f9420660 ldr"@0xffffff8008a5f1e0 + "f902067f str"@0xffffff8008a5f1ec
	 * -- questo INVECE viene rimesso a NULL, ed e' un globale */
	ipio_vfree((void **)&ilitek_ini_data);

	return ret;
}

/*
 * ===========================================================================
 * LOTTO F3a -- 2026-08-22: DUE FUNZIONI SU TRE, E UNA DIVERGENZA DI F1 CHIUSA
 * ===========================================================================
 * La consegna del lotto era TRE funzioni, 7148 byte:
 *
 *   mutual_test                 4584   NON SCRITTA (vedi in fondo)
 *   create_mp_test_frame_buffer 1388   scritta, 1388 su 1388, RESIDUO ZERO
 *   allnode_open_cdc_data       1176   scritta, 1176 su 1176, residuo 2
 *
 * Scritti 2564 byte su 7148. `mutual_test` NON e' scritta e resta dichiarata
 * e non definita (regola 6): il perche' e la mappa di cio' che di essa e'
 * stato letto stanno nella sezione "MUTUAL_TEST, LETTA E NON SCRITTA".
 *
 * LA MISURA, COL COMPILATORE DI FABBRICA (clang-r353983c):
 *
 *   create_mp_test_frame_buffer  1388 = 1388   347 istruzioni su 347,
 *                                             347 mnemonici uguali su 347,
 *                                             145 codifiche diverse, TUTTE
 *                                             siti di rilocazione: RESIDUO 0
 *   allnode_open_cdc_data        1176 = 1176   294 istruzioni su 294,
 *                                             291 mnemonici uguali su 294,
 *                                             111 codifiche diverse = 107
 *                                             rilocazioni + 2 salti +
 *                                             RESIDUO 2
 *
 * Il residuo 2 di `allnode_open_cdc_data` e' UNA SOLA cosa: tre istruzioni --
 * "f000b4aa adrp"@0xffffff8008a65c80, "9102e3e0 add"@0xffffff8008a65c84 e
 * "b902dd09 str"@0xffffff8008a65c88 -- che il nostro compilatore emette nello
 * stesso numero e con lo stesso mnemonico ma in ordine diverso (add, str,
 * adrp invece di adrp, add, str). Non c'e' nessuna istruzione in piu' e
 * nessuna in meno. E' una divergenza di ORDINAMENTO, dichiarata e non chiusa.
 *
 * ===========================================================================
 * F1-D3 E' CHIUSA, E CHIUDERLA E' CIO' CHE HA FATTO TORNARE IL CONTO
 * ===========================================================================
 * Questa e' la cosa piu' importante del lotto, ed e' un caso di scuola di
 * "un numero che non torna e' un sintomo".
 *
 * `allnode_open_cdc_data` scritta come qui misurava 1360 byte invece di 1176,
 * con TUTTI E DUE i compilatori. La differenza non era nel sorgente della
 * funzione: era che clang INCORPORAVA `parser_get_u8_array` dentro di essa,
 * mentre di fabbrica quella chiamata e' una `bl`
 * ("97ffff5f bl"@0xffffff8008a65c54). Il compilatore stesso dice perche',
 * se glielo si chiede con `-Rpass=inline`:
 *
 *   remark: parser_get_u8_array inlined into allnode_open_cdc_data
 *           with (cost=220, threshold=225) [-Rpass=inline]
 *
 * 220 contro 225: cinque punti. Quattro sonde, compilate e misurate, hanno
 * isolato da dove vengono i cinque punti -- il costo di incorporazione con la
 * stessa chiamata e argomenti diversi:
 *
 *   sonda_A(char *s, u8 *b)  parser_get_u8_array(s, b, 15)      cost=225
 *   sonda_B(char *s)         u8 buf[15]; (s, buf, 15)           cost=225
 *   sonda_C(u8 *b)           char k[128]; (k, b, 15)            cost=220
 *   sonda_D(char *s,u8 *b,int n) (s, b, n)                      cost=240
 *
 * cioe' lo sconto viene dal PRIMO argomento quando e' un vettore di pila, e
 * dal terzo quando e' una costante. Nel nostro sorgente lo sono tutti e due,
 * e di fabbrica pure: il primo argomento e' `add x0, sp, #0x38` e il terzo
 * "32000fe2 orr"@0xffffff8008a65c50, cioe' 15. Quindi la differenza NON
 * poteva stare nel chiamante: doveva stare nel CORPO di
 * `parser_get_u8_array`, che il lotto F1 aveva lasciato con una divergenza
 * aperta e dichiarata (F1-D3: 73 istruzioni su 73, stessa dimensione, ma solo
 * 62 mnemonici uguali su 73 e RESIDUO 12).
 *
 * La correzione e' l'ORDINE DEI DUE TERMINI di una `||`:
 * `strlen(key) == 0 || len < 1` invece di `len < 1 || strlen(key) == 0`.
 * Con essa:
 *
 *   parser_get_u8_array   PRIMA: 62/73 mnemonici, residuo 12
 *                          DOPO: 73/73 mnemonici, residuo  0
 *   costo di incorporazione dentro allnode_open_cdc_data: 220 -> 230
 *   allnode_open_cdc_data           1360 byte -> 1176 byte
 *
 * La dimensione di `parser_get_u8_array` non cambia (292 = 292 prima e dopo):
 * era per questo che F1-D3 era rimasta aperta -- nessuna misura di dimensione
 * la denunciava, e la sua conseguenza cadeva su una funzione che allora non
 * era ancora scritta.
 *
 * ===========================================================================
 * LE COSE MISURATE CHE HANNO CAMBIATO IL CODICE
 * ===========================================================================
 * F3a-1. IN `allnode_open_cdc_data` I DUE INDICI DEL CICLO SONO DUE CATENE
 *   INDIPENDENTI. Il binario porta DUE variabili d'induzione a 64 bit --
 *   "b26003ec orr"@0xffffff8008a65da0 (0x100000000) e
 *   "b25f03ed orr"@0xffffff8008a65da4 (0x200000000) -- e ne ricava i due
 *   indici con due `asr #32`. Scritti come `ori[2*i+1]` e `ori[2*i+2]` clang
 *   mette in comune il `2*i`: 1184 byte invece di 1176. Scritto
 *   `idx = 2*i+1` e poi `ori[idx]` / `ori[idx+1]`, il conto torna al byte.
 *
 * F3a-2. LA PROVA DEL SEGNO E' SUL BYTE ALTO GREZZO, non sul bit 15 del
 *   valore fuso: "721901df tst"@0xffffff8008a65dbc mette alla prova
 *   `ori[idx]` e sta PRIMA della `bfi`. Con `tmp >= 0x8000` clang emette
 *   `lsr`+`cmp`+`csel` (una istruzione in piu'); con `ori[idx] & 0x80`
 *   emette `tst`+`orr`+`csel` come di fabbrica.
 *
 * F3a-3. IL VALORE PASSA PER UNA TEMPORANEA, non per `buf[i]`: di fabbrica
 *   c'e' UNA SOLA `str` ("b8297a6e str"@0xffffff8008a65df0) per tutti e due i
 *   rami. Scritto con `buf[i]` come accumulatore le `str` diventano DUE,
 *   perche' il compilatore non puo' escludere che `buf` punti dentro
 *   `core_mp`.
 *
 * F3a-4. L'ORDINE DI DICHIARAZIONE dei due vettori di pila decide la
 *   disposizione del frame, e la disposizione si misura: di fabbrica `str[128]`
 *   sta a sp+56 e `cmd[15]` a sp+184 ("790173e9 strh"@0xffffff8008a65ce0).
 *   clang li dispone in ordine INVERSO rispetto alla dichiarazione, quindi
 *   `cmd` va dichiarato PRIMA di `str`. Con l'ordine sbagliato la dimensione
 *   e' identica e undici codifiche divergono.
 *
 * F3a-5. LE ALLOCAZIONI DI `create_mp_test_frame_buffer` SONO `kcalloc`, non
 *   `kzalloc`. La prova e' il controllo di traboccamento di `kmalloc_array`:
 *   "b98c0108 ldrsw"@0xffffff8008a6527c legge `core_mp.c256` CON SEGNO e
 *   "37f82108 tbnz"@0xffffff8008a65280 salta a un ramo che scrive NULL nel
 *   puntatore quando il valore e' negativo. Con `kzalloc(n * 4, ...)` quel
 *   ramo non esisterebbe.
 *
 * ===========================================================================
 * I DUE VERIFICATORI DI CITAZIONE, DOPO IL LOTTO
 * ===========================================================================
 *   ./venv/bin/python3 verificacitazioni.py ilitek_mp.c oracolo/stock.elf
 *
 *     letterali: 283   citati: 255   verificati: 247   probanti: 233   deboli: 14
 *     di cui assemblate dalla macro di log: 110
 *     di cui nome di funzione da __func__: 27
 *     rinviate a verificaistruzioni.py: 1572
 *
 *   istogramma delle etichette:
 *     14 DEBOLE   1 INDIRIZZO_AMBIGUO   7 NON_ANCORATA
 *    110 VERIFICATA_ASSEMBLATA        27 VERIFICATA_FUNC
 *
 *   Le otto che non verificano, una per una:
 *    - INDIRIZZO_AMBIGUO "%s": citata a 0xffffff8009100e35 e a
 *      0xffffff8009101762. E' PRECEDENTE a questo lotto e non e' toccata.
 *    - NON_ANCORATA "delay_time = 0x%x\n", "test_int_pin = 0x%x\n",
 *      "int_pulse_test = 0x%x\n": sono i tre messaggi di `pin_test`, che non
 *      e' scritta. PRECEDENTI a questo lotto.
 *      [SUPERATA DAL LOTTO F3c-bis: `pin_test` e' scritta, i tre letterali
 *      esistono, le tre citazioni sono passate a DIVERGENTE e sono state
 *      riscritte per intero col prefisso KERN_INFO. Ora verificano.]
 *    - NON_ANCORATA "5a90060e cneg"@0xffffff8008a65ddc: E' DI QUESTO LOTTO ed
 *      e' un DELTA DI STRUMENTO, non un difetto della citazione. Il
 *      dizionario di `confinecitazioni.py` non conosce i mnemonici `cneg` e
 *      `cinc` (verificato: `mnemonico_valido("cneg")` torna False,
 *      `mnemonico_valido("csel")` torna True), quindi la citazione non viene
 *      rinviata a `verificaistruzioni.py` e cade fra i letterali. Aggiungere
 *      i due mnemonici al dizionario e' il delta; NON e' fatto qui perche'
 *      `confinecitazioni.py` e' uno strumento condiviso.
 *    - NON_ANCORATA sui tre nomi `codeToOhm`, `mp_cdc_init_cmd_common` e
 *      `allnode_mutual_cdc_data`: SONO DI QUESTO LOTTO, e sono corrette
 *      cosi'. Lo strumento ancora un nome di funzione solo se quella
 *      funzione e' DEFINITA nel file (`VERIFICATA_FUNC`); quelle tre sono le
 *      incorporate di `mutual_test`, che questo lotto NON scrive, quindi non
 *      esistono nel sorgente e non possono essere ancorate. Il giorno in cui
 *      `mutual_test` sara' scritta, le tre passeranno a VERIFICATA_FUNC senza
 *      che nessuno tocchi le citazioni.
 *
 *   SEI CITAZIONI PRECEDENTI SONO STATE CORRETTE, e la correzione e' un
 *   sottoprodotto di questo lotto. Le sei che nominano i buffer di
 *   `struct ilitek_mp_item` ("Failed to allocate result_buf mem\n" e le
 *   altre cinque) citavano la CODA del messaggio all'indirizzo del messaggio
 *   INTERO: finche' `create_mp_test_frame_buffer` non era scritta, lo
 *   strumento le dava NON_ANCORATA (nessun letterale con quel testo nel
 *   codice) e la cosa passava. Scritta la funzione, il letterale esiste, lo
 *   strumento confronta i byte e le sei diventano DIVERGENTE. Sono ora
 *   scritte per intero, col prefisso "\x013ILITEK: (%s, %d): ", e verificano.
 *
 *   ./venv/bin/python3 verificaistruzioni.py ilitek_mp.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a5da3c:0xffffff8008a6714c
 *
 *     citazioni di istruzione trovate nel sorgente: 1602 (1602 a codifica)
 *     confermate: 1586   assenti: 16   mnemonico diverso: 0
 *     operandi -- registro diverso: 0   immediato diverso: 0
 *
 *   Le 16 ASSENTE sono PRECEDENTI (il file prima del lotto ne aveva 16 su
 *   1430, con 1414 confermate) e sono le righe di `oltre.asm` citate nel
 *   cappello sulla `exit`: stanno a 0xffffff80093ac..., cioe' FUORI
 *   dall'intervallo del gruppo F, e lo strumento lo dice cosi'. Le 172
 *   citazioni di istruzione nuove di questo lotto sono confermate tutte e
 *   172 (1586 - 1414).
 *
 * ===========================================================================
 * IL LINK: GLI IRRISOLTI ILITEK RESTANO CINQUE
 * ===========================================================================
 * Misurato sull'archivio di tutti e otto gli oggetti, PRIMA e DOPO:
 *
 *   $NM built-in.o | awk 'NF==3 {print $3}' | sort -u > def
 *   $NM built-in.o | awk '$1=="U" {print $2}' | sort -u > und
 *   comm -13 def und
 *
 *   PRIMA:  definiti 215   citati U 173   irrisolti 105
 *   DOPO:   definiti 217   citati U 174   irrisolti 105
 *
 * I due definiti in piu' sono le due funzioni di questo lotto. L'unico
 * simbolo `U` nuovo e' `ilitek_tddi_ic_check_int_stat`, che `ilitek_ic.o`
 * definisce: per questo il conto degli irrisolti non si muove. Gli irrisolti
 * che sono funzioni del driver restano gli stessi CINQUE:
 *
 *   key_test  mutual_test  open_test_cap  open_test_sp  pin_test
 *
 * Questo lotto NON fa comparire irrisolti nuovi: `create_mp_test_frame_buffer`
 * e `allnode_open_cdc_data` sono `static` e non chiamano niente che non sia
 * gia' definito o gia' citato.
 *
 * ===========================================================================
 * DIFETTO TROVATO E NON CORRETTO -- I CODICI D'ERRORE DI
 * `ilitek_tddi_mp_test_main` (lotto F5)
 * ===========================================================================
 * NON E' UN DIFETTO DI QUESTO LOTTO e questo lotto NON lo corregge: la
 * funzione e' di F5 e ha gia' una divergenza di dimensione dichiarata (+132).
 * Va pero' scritto, perche' e' esattamente la classe "che la dimensione NON
 * denuncia" (una `mov` di costante resta una `mov` di costante).
 *
 * Il sorgente scrive `ret = -ENOMEM;` e `ret = -EBADRQC;` dove i commenti
 * accanto dicono, correttamente, -107 e -105. Ma `-ENOMEM` vale -12 e
 * `-EBADRQC` vale -56. L'istogramma delle costanti negative emesse dentro
 * `ilitek_tddi_mp_test_main` lo mostra:
 *
 *   grep -oE "// #-[0-9]+" <disassemblato> | sort | uniq -c | sort -rn
 *
 *   FABBRICA                    NOSTRO
 *     12 #-1                      10 #-1
 *      3 #-4294967296              6 #-12
 *      3 #-12                      3 #-4294967296
 *      3 #-107                     3 #-22
 *      2 #-22                      1 #-56
 *      2 #-105                     1 #-14
 *      1 #-104                     1 #-107
 *      1 #-103                     1 #-101
 *      1 #-101
 *
 * cioe': di fabbrica -107 compare TRE volte e -105 DUE, da noi -107 compare
 * UNA volta sola e -105 nessuna; al loro posto ci sono tre -12 in piu' e un
 * -56 che di fabbrica non esiste. Mancano anche -104 e -103. I siti da
 * correggere sono le righe che scrivono `-ENOMEM`/`-EBADRQC` in
 * `parser_get_ini_phy_data` e in `ilitek_tddi_mp_test_main`.
 *
 * ===========================================================================
 * MUTUAL_TEST, LETTA E NON SCRITTA -- LA MAPPA PER IL LOTTO CHE LA FARA'
 * ===========================================================================
 * ATTENZIONE: QUESTA SEZIONE E' STORIA. Il lotto F6 ha SCRITTO `mutual_test`,
 * e il codice sta in fondo al file col suo cappello. Cio' che segue e' la
 * ricognizione del lotto F3a, lasciata agli atti perche' e' cio' su cui il
 * lotto F6 ha lavorato; dove F6 ha misurato qualcosa di diverso, lo dice il
 * cappello di `mutual_test`.
 *
 * 0xffffff8008a620d4, 4584 byte, 1146 istruzioni, `t`. NON E' SCRITTA: resta
 * dichiarata e non definita (regola 6), e il link continua a fallire su di
 * essa. La ragione e' che il suo `switch` centrale ha SEI rami, tre dei quali
 * sono algoritmi di confronto fra vicini con quattro casi di bordo annidati,
 * e scriverli "plausibilmente" sarebbe esattamente cio' che la regola 6
 * vieta. Cio' che di essa e' stato LETTO e verificato sta qui, perche' non
 * vada perduto.
 *
 * TRE FUNZIONI INCORPORATE, coi loro `__func__` e le loro righe:
 *   codeToOhm                riga 1369, "codeToOhm"@0xffffff8009244147
 *   mp_cdc_init_cmd_common   riga 1568, "mp_cdc_init_cmd_common"@0xffffff8009244083
 *   allnode_mutual_cdc_data  righe 1713..1795, "allnode_mutual_cdc_data"@0xffffff8009243e85
 * piu' `parser_get_int_data` (riga 566), che e' gia' scritta in questo file.
 *
 * LE RIGHE DI FABBRICA DEI MESSAGGI, lette una per una:
 *   2143 "\x016ILITEK: (%s, %d): index = %d, desp = %s, Frame Count = %d\n"@0xffffff8009243a47
 *   2150 "\x013ILITEK: (%s, %d): Frame count is zero, which is at least set as 1\n"@0xffffff8009243a90
 *   2187 "\x013ILITEK: (%s, %d): Failed to initialise CDC data, %d\n"@0xffffff8009243ad5
 *   1713 "\x016ILITEK: (%s, %d): Read X/Y Channel length = %d\n"@0xffffff8009243e53
 *   1716 "\x013ILITEK: (%s, %d): Length is invalid\n"@0xffffff8009243e9d
 *   1725 "\x013ILITEK: (%s, %d): Failed to get cdc command\n"@0xffffff8009243ec4
 *   1735 "\x013ILITEK: (%s, %d): Write CDC command failed\n"@0xffffff8009243f06
 *   1757 "\x013ILITEK: (%s, %d): Write (0x%x, 0x%x) error\n"@0xffffff8009243f34
 *   1766 "\x013ILITEK: (%s, %d): Write (0x%x) error\n"@0xffffff8009243f62
 *   1778 "\x013ILITEK: (%s, %d): Failed to allocate ori, (%ld)\n"@0xffffff8009243f8a
 *   1785 "\x013ILITEK: (%s, %d): Read cdc data error, len = %d\n"@0xffffff8009243fbd
 *   1795 "\x013ILITEK: (%s, %d): Failed to allocate FrameBuffer mem (%ld)\n"@0xffffff8009244004
 *   1568 "\x016ILITEK: (%s, %d): P2P CMD: %d,%d,%d,%d,%d\n"@0xffffff8009244056
 *   1369 "\x013ILITEK: (%s, %d): code is invalid\n"@0xffffff8009244122
 *
 * IL `switch` CENTRALE e' su `mp_items[index].c16` e la sua TAVOLA DI SALTO
 * sta a 0xffffff8008f7ec4c, sei mezze parole lette dai byte:
 *
 *   ffffff8008f7ec4c  00003200 6000ed00 0e011f01
 *
 * cioe' 0x0000, 0x0032, 0x0060, 0x00ed, 0x010e, 0x011f, ognuna moltiplicata
 * per 4 e sommata alla base "10000089 adr"@0xffffff8008a62944, che vale
 * 0xffffff8008a62954. Il `sub w8, w8, #0x4` piu' `cmp w8, #0x5` che
 * precedono ("51001108 sub"@0xffffff8008a62930,
 * "7100151f cmp"@0xffffff8008a62934) dicono che i casi vanno da 4 a 9:
 *
 *   c16 == 4 -> 0xffffff8008a62954   tx/rx delta
 *   c16 == 5 -> 0xffffff8008a62a1c   massimo/minimo e differenza
 *   c16 == 6 -> 0xffffff8008a62ad4   il confronto coi vicini (il piu' lungo)
 *   c16 == 7 -> 0xffffff8008a62d08   open test
 *   c16 == 8 -> 0xffffff8008a62d8c   LO STESSO del ramo `default`
 *   c16 == 9 -> 0xffffff8008a62dd0   short test, con `codeToOhm`
 *   default  -> 0xffffff8008a62d8c
 *
 * Che il caso 8 cada sul `default` significa che nel sorgente `case 8` NON
 * c'e': clang riempie i buchi dell'intervallo 4..9 con l'etichetta di
 * `default`.
 *
 * UN FATTO CHE VA VERIFICATO DA CHI LA SCRIVERA', perche' e' sorprendente:
 * il ciclo che inizializza i buffer di massimo e minimo legge
 * "b946e152 ldr"@0xffffff8008a621e0, cioe' l'indirizzo COSTANTE
 * 0xffffff80099876e0 = `mp_items` + 16 = `mp_items[0].c16` -- NON
 * `mp_items[index].c16`. L'indice non compare in quell'accesso, e il
 * registro di base (x10) non viene toccato fra
 * "b000792a adrp"@0xffffff8008a621bc e la lettura. Se e' cosi', il ramo
 * `== 4` di quel ciclo di fabbrica non parte mai (mp_items[0].c16 vale 0
 * nella tavola) ed e' un difetto della fabbrica da riprodurre.
 *
 * ALTRI DUE DIFETTI DELLA FABBRICA GIA' LOCALIZZATI dentro
 * `allnode_mutual_cdc_data`:
 *  - l'azzeramento del buffer di trama e' `memset(mp_frame_buf0, 0,
 *    core_mp.c256)` -- "b98c0322 ldrsw"@0xffffff8008a626b4 passa il NUMERO di
 *    nodi come numero di BYTE, cioe' un quarto di quel che serve;
 *  - nel ramo `c16 == 4` del caso 4 i due `delta` sono calcolati tutti e due
 *    su `frame[idx] - frame[idx + 1]` ("1100054a add"@0xffffff8008a629a0 e
 *    "1100054a add"@0xffffff8008a629d8), mentre le due GUARDIE sono diverse
 *    (`i != c236-1` per il primo, `j != c232-1` per il secondo).
 */

/*
 * DUE DICHIARAZIONI LOCALI (voce nuova del DELTA DI HEADER): `ilitek.h` non
 * le porta, e i due gruppi che le definiscono sono altri file dello stesso
 * oggetto. `ilitek_touch.c` definisce `ilitek_dump_data` (0xffffff8008a5aad8)
 * e `ilitek_ic.c` definisce `ilitek_tddi_ic_check_busy`
 * (0xffffff8008a594d4, 560 byte). Le firme sono COPIATE da quei due file,
 * non inventate qui.
 */
void ilitek_dump_data(void *dato, int larghezza, int numero, int per_riga,
		      const char *etichetta);
int ilitek_tddi_ic_check_busy(int count, int delay);

/*
 * ===========================================================================
 * create_mp_test_frame_buffer -- 0xffffff8008a651d0, 1388 byte, `t`
 * ===========================================================================
 * DUE PARAMETRI, tutti e due `int`. Il primo e' l'indice dell'elemento e il
 * suo segno e' MISURATO: "9b285688 smaddl"@0xffffff8008a65220 estende w20 CON
 * SEGNO prima di moltiplicarlo per 152, e "93407e94 sxtw"@0xffffff8008a652a8
 * fa lo stesso per gli accessi che seguono. Il secondo compare in un solo
 * punto, "1b137d08 mul"@0xffffff8008a652c4, dove moltiplica `core_mp.c256`.
 *
 * QUATTRO CHIAMANTI, nessuno dei quali fuori dal gruppo F:
 * "94000c12 bl"@0xffffff8008a62188 (`mutual_test`),
 * "9400078d bl"@0xffffff8008a6339c (`open_test_sp`),
 * "940003c3 bl"@0xffffff8008a642c4 (`open_test_cap`) e
 * "94000201 bl"@0xffffff8008a649cc (`key_test`).
 *
 * AGGIORNAMENTO DEL LOTTO F3c-bis: TRE DEI QUATTRO CHIAMANTI SONO ORA SCRITTI
 * (`open_test_sp`, `open_test_cap`, `key_test`); manca solo `mutual_test`. Il
 * `__used` non serve piu' a tenere viva la funzione ed e' rimasto solo per non
 * cambiare niente: su una funzione che E' usata l'attributo non cambia un byte
 * del codice emesso, e toglierlo e' un compito del lotto che scrivera'
 * `mutual_test`. La frase originale -- «NESSUNO DEI QUATTRO E' SCRITTO, e da
 * qui `__used`» -- valeva al lotto F1/F2 e oggi sarebbe falsa.
 *
 * LE ALLOCAZIONI SONO `kcalloc`, NON `kzalloc`, ed e' MISURATO. Ogni sito ha
 * la stessa forma di quattro istruzioni:
 *
 *   "b98c0108 ldrsw"@0xffffff8008a6527c   x8 = (long)core_mp.c256, CON SEGNO
 *   "37f82108 tbnz"@0xffffff8008a65280    se x8 < 0 -> il puntatore resta NULL
 *   "d37ef500 lsl"@0xffffff8008a65288     x0 = x8 * 4
 *   "97dfac3e bl"@0xffffff8008a65290      <__kmalloc>
 *
 * Il `tbnz` sul bit 31 e' il controllo di traboccamento di `kmalloc_array`
 * (`n > SIZE_MAX / size`): con `n` esteso CON SEGNO, «negativo» e «piu' grande
 * di SIZE_MAX/4» sono la stessa cosa, e il ramo scrive NULL nel puntatore
 * ("f9061a7f str"@0xffffff8008a654bc e le sue gemelle). Con `kzalloc(n * 4, ...)`
 * quel controllo non ci sarebbe. La bandiera e' 0x14080c0
 * ("52901801 mov"@0xffffff8008a65284 + "72a02801 movk"@0xffffff8008a6528c),
 * cioe' GFP_KERNEL | __GFP_ZERO, che e' quello che `kcalloc` aggiunge.
 *
 * `buf` E' L'UNICO ALLOCATO CON `vmalloc`, e la sua dimensione e' l'unica che
 * usa il secondo parametro: "b94c0108 ldr"@0xffffff8008a652c0 (quattro byte,
 * SENZA estensione di segno), "1b137d08 mul"@0xffffff8008a652c4 e
 * "937e7d00 sbfiz"@0xffffff8008a652c8, cioe' `(long)(c256 * frame_count) * 4`.
 *
 * DIFETTO DELLA FABBRICA, RIPRODOTTO (regola 7): il ramo d'errore di `buf`
 * libera con `kfree` ("97dfaccc bl"@0xffffff8008a65480) una memoria ottenuta
 * da `vmalloc` ("97df7569 bl"@0xffffff8008a652cc). E' la stessa `ILI_KFREE`
 * degli altri undici rami, applicata al puntatore sbagliato.
 *
 * IL CODICE DI RITORNO D'ERRORE E' -12: "12800160 mov"@0xffffff8008a65728,
 * cioe' -ENOMEM. E' l'unico dei codici di questo lotto che coincida con un
 * errno di Linux, e resta scritto come numero perche' il binario non nomina
 * niente.
 */
#line 1881
static __used int create_mp_test_frame_buffer(int index, int frame_count)
{
	/* "\x016ILITEK: (%s, %d): Create MP frame buffers (index = %d), count = %d\n"
	 * @0xffffff8009243b0c, __func__ =
	 * "create_mp_test_frame_buffer"@0xffffff8009243b52; la guardia e'
	 * "39656108 ldrb"@0xffffff8008a651e4 + "34000128 cbz"@0xffffff8008a651f0;
	 * "5280eba2 mov"@0xffffff8008a65204 -- __LINE__ = 1885. La chiamata
	 * occupa DUE righe e `__LINE__` e' la riga della parentesi CHIUSA:
	 * la direttiva porta quindi 1884 */
#line 1884
	ILI_DBG("Create MP frame buffers (index = %d), count = %d\n",
		index, frame_count);

	/* "b9401108 ldr"@0xffffff8008a65224 (+0x10) +
	 * "7100111f cmp"@0xffffff8008a65228 + "540003e1 b.ne"@0xffffff8008a6522c */
	if (mp_items[index].c16 == 4) {
		/* "f9461a68 ldr"@0xffffff8008a65234 (0xffffff800a0fcc30) +
		 * "b4000cc8 cbz"@0xffffff8008a65238 */
		if (!core_mp.tx_delta_buf) {
			core_mp.tx_delta_buf = kcalloc(core_mp.c256,
						       sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.tx_delta_buf) ||
			    core_mp.tx_delta_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate tx_delta_buf mem\n"
				 * @0xffffff8009243b6e;
				 * "5280ec62 mov"@0xffffff8008a654d0 -- __LINE__ = 1891 */
#line 1891
				ILI_ERR("Failed to allocate tx_delta_buf mem\n");
				ILI_KFREE(core_mp.tx_delta_buf);
				return -ENOMEM;
			}
		}

		/* "f9461e68 ldr"@0xffffff8008a65240 (0xffffff800a0fcc38) */
		if (!core_mp.rx_delta_buf) {
			core_mp.rx_delta_buf = kcalloc(core_mp.c256,
						       sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.rx_delta_buf) ||
			    core_mp.rx_delta_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate rx_delta_buf mem\n"
				 * @0xffffff8009243ba7;
				 * "5280ed82 mov"@0xffffff8008a65554 -- __LINE__ = 1900 */
#line 1900
				ILI_ERR("Failed to allocate rx_delta_buf mem\n");
				ILI_KFREE(core_mp.rx_delta_buf);
				return -ENOMEM;
			}
		}

		/* "f9462268 ldr"@0xffffff8008a6524c (0xffffff800a0fcc40) */
		if (!core_mp.tx_max_buf) {
			core_mp.tx_max_buf = kcalloc(core_mp.c256,
						     sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.tx_max_buf) ||
			    core_mp.tx_max_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate tx_max_buf mem\n"
				 * @0xffffff8009243be0;
				 * "5280eea2 mov"@0xffffff8008a655a8 -- __LINE__ = 1909 */
#line 1909
				ILI_ERR("Failed to allocate tx_max_buf mem\n");
				ILI_KFREE(core_mp.tx_max_buf);
				return -ENOMEM;
			}
		}

		/* "f9462668 ldr"@0xffffff8008a65258 (0xffffff800a0fcc48) */
		if (!core_mp.tx_min_buf) {
			core_mp.tx_min_buf = kcalloc(core_mp.c256,
						     sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.tx_min_buf) ||
			    core_mp.tx_min_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate tx_min_buf mem\n"
				 * @0xffffff8009243c17;
				 * "5280efc2 mov"@0xffffff8008a65654 -- __LINE__ = 1918 */
#line 1918
				ILI_ERR("Failed to allocate tx_min_buf mem\n");
				ILI_KFREE(core_mp.tx_min_buf);
				return -ENOMEM;
			}
		}

		/* "f9462a68 ldr"@0xffffff8008a65264 (0xffffff800a0fcc50) */
		if (!core_mp.rx_max_buf) {
			core_mp.rx_max_buf = kcalloc(core_mp.c256,
						     sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.rx_max_buf) ||
			    core_mp.rx_max_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate rx_max_buf mem\n"
				 * @0xffffff8009243c4e;
				 * "5280f0e2 mov"@0xffffff8008a65684 -- __LINE__ = 1927 */
#line 1927
				ILI_ERR("Failed to allocate rx_max_buf mem\n");
				ILI_KFREE(core_mp.rx_max_buf);
				return -ENOMEM;
			}
		}

		/* "f9462e68 ldr"@0xffffff8008a65270 (0xffffff800a0fcc58) */
		if (!core_mp.rx_min_buf) {
			core_mp.rx_min_buf = kcalloc(core_mp.c256,
						     sizeof(int), GFP_KERNEL);
			if (IS_ERR(core_mp.rx_min_buf) ||
			    core_mp.rx_min_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate rx_min_buf mem\n"
				 * @0xffffff8009243c85;
				 * "5280f202 mov"@0xffffff8008a656b4 -- __LINE__ = 1936 */
#line 1936
				ILI_ERR("Failed to allocate rx_min_buf mem\n");
				ILI_KFREE(core_mp.rx_min_buf);
				return -ENOMEM;
			}
		}
	} else {
		/* "f8460ec8 ldr"@0xffffff8008a652b4 -- pre-indice, +0x60 = 96 */
		if (!mp_items[index].buf) {
			mp_items[index].buf = vmalloc(core_mp.c256 *
						      frame_count * sizeof(int));
			if (IS_ERR(mp_items[index].buf) ||
			    mp_items[index].buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate buf mem\n"
				 * @0xffffff8009243cbc;
				 * "5280f322 mov"@0xffffff8008a65470 -- __LINE__ = 1945 */
#line 1945
				ILI_ERR("Failed to allocate buf mem\n");
				ILI_KFREE(mp_items[index].buf);
				return -ENOMEM;
			}
		}

		/* "f8458e68 ldr"@0xffffff8008a652e8 -- pre-indice, +0x58 = 88 */
		if (!mp_items[index].result_buf) {
			mp_items[index].result_buf = kcalloc(core_mp.c256,
							     sizeof(int), GFP_KERNEL);
			if (IS_ERR(mp_items[index].result_buf) ||
			    mp_items[index].result_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate result_buf mem\n"
				 * @0xffffff8009243cec;
				 * "5280f442 mov"@0xffffff8008a65538 -- __LINE__ = 1954 */
#line 1954
				ILI_ERR("Failed to allocate result_buf mem\n");
				ILI_KFREE(mp_items[index].result_buf);
				return -ENOMEM;
			}
		}

		/* "f8468e68 ldr"@0xffffff8008a65324 -- pre-indice, +0x68 = 104 */
		if (!mp_items[index].max_buf) {
			mp_items[index].max_buf = kcalloc(core_mp.c256,
							  sizeof(int), GFP_KERNEL);
			if (IS_ERR(mp_items[index].max_buf) ||
			    mp_items[index].max_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate max_buf mem\n"
				 * @0xffffff8009243d23;
				 * "5280f562 mov"@0xffffff8008a6558c -- __LINE__ = 1963 */
#line 1963
				ILI_ERR("Failed to allocate max_buf mem\n");
				ILI_KFREE(mp_items[index].max_buf);
				return -ENOMEM;
			}
		}

		/* "f8470e68 ldr"@0xffffff8008a65360 -- pre-indice, +0x70 = 112 */
		if (!mp_items[index].min_buf) {
			mp_items[index].min_buf = kcalloc(core_mp.c256,
							  sizeof(int), GFP_KERNEL);
			if (IS_ERR(mp_items[index].min_buf) ||
			    mp_items[index].min_buf == NULL) {
				/* "\x013ILITEK: (%s, %d): Failed to allocate min_buf mem\n"
				 * @0xffffff8009243d57;
				 * "5280f682 mov"@0xffffff8008a6560c -- __LINE__ = 1972 */
#line 1972
				ILI_ERR("Failed to allocate min_buf mem\n");
				ILI_KFREE(mp_items[index].min_buf);
				return -ENOMEM;
			}
		}

		/* "39405508 ldrb"@0xffffff8008a6539c + "7100051f cmp"@0xffffff8008a653a0 */
		if (mp_items[index].c21 == 1) {
			/* "f8478e68 ldr"@0xffffff8008a653b0 -- +0x78 = 120 */
			if (!mp_items[index].bench_mark_max) {
				mp_items[index].bench_mark_max = kcalloc(core_mp.c256,
									 sizeof(int),
									 GFP_KERNEL);
				if (IS_ERR(mp_items[index].bench_mark_max) ||
				    mp_items[index].bench_mark_max == NULL) {
					/* "\x013ILITEK: (%s, %d): Failed to allocate bench_mark_max mem\n"
					 * @0xffffff8009243d8b;
					 * "5280f7c2 mov"@0xffffff8008a656ec -- __LINE__ = 1982 */
#line 1982
					ILI_ERR("Failed to allocate bench_mark_max mem\n");
					ILI_KFREE(mp_items[index].bench_mark_max);
					return -ENOMEM;
				}
			}

			/* "f8480e68 ldr"@0xffffff8008a653c0 -- +0x80 = 128 */
			if (!mp_items[index].bench_mark_min) {
				mp_items[index].bench_mark_min = kcalloc(core_mp.c256,
									 sizeof(int),
									 GFP_KERNEL);
				if (IS_ERR(mp_items[index].bench_mark_min) ||
				    mp_items[index].bench_mark_min == NULL) {
					/* "\x013ILITEK: (%s, %d): Failed to allocate bench_mark_min mem\n"
					 * @0xffffff8009243dc6;
					 * "5280f8c2 mov"@0xffffff8008a65710 -- __LINE__ = 1990 */
#line 1990
					ILI_ERR("Failed to allocate bench_mark_min mem\n");
					ILI_KFREE(mp_items[index].bench_mark_min);
					return -ENOMEM;
				}
			}
		}
	}

	/* "2a1f03e0 mov"@0xffffff8008a653c8 */
	return 0;
}

/*
 * ===========================================================================
 * allnode_open_cdc_data -- 0xffffff8008a65af4, 1176 byte, `t`
 * ===========================================================================
 * DUE PARAMETRI. Il primo e' un `int` con segno: e' l'indice dentro il
 * vettore di sei nomi di chiave .ini e l'indicizzazione lo estende CON SEGNO
 * ("f874d915 ldr"@0xffffff8008a65bf8, cioe' `[x8, w20, sxtw #3]`). Il secondo
 * e' un `int *`: la sola scrittura che lo tocca e'
 * "b8297a6e str"@0xffffff8008a65df0, quattro byte scalati di 4.
 *
 * `static`: nella mappa e' `t` (0xffffff8008a65af4). I DUE CHIAMANTI sono
 * `open_test_sp` (quattro siti, il primo "94000790 bl"@0xffffff8008a63cb4) e
 * `open_test_cap` (due siti, "94000514 bl"@0xffffff8008a646a4), che questo
 * lotto NON scrive: da qui `__used`, altrimenti clang la eliminerebbe e la
 * misura non esisterebbe.
 *
 * IL VETTORE DI SEI NOMI sta sulla PILA, non in `.rodata`: sei `add` di
 * indirizzo e tre `stp` a [sp,#8], [sp,#24], [sp,#40]
 * ("a900abe9 stp"@0xffffff8008a65b2c, "a901b3eb stp"@0xffffff8008a65b50,
 * "a902afe8 stp"@0xffffff8008a65b60). I sei letterali, letti dai byte:
 *   "open dac"@0xffffff800924446b        "open raw1"@0xffffff8009244474
 *   "open raw2"@0xffffff800924447e       "open raw3"@0xffffff8009244488
 *   "open cap1 dac"@0xffffff8009244492   "open cap1 raw"@0xffffff80092444a0
 *
 * LA LUNGHEZZA. "2940252a ldp"@0xffffff8008a65b40 legge in UNA volta i due
 * campi adiacenti `core_mp.c232` e `core_mp.c236` (x9 = 0xffffff800a0fcbe8),
 * "1b097d48 mul"@0xffffff8008a65b64 li moltiplica,
 * "531f7908 lsl"@0xffffff8008a65b70 raddoppia e
 * "11000916 add"@0xffffff8008a65b7c aggiunge 2.
 *
 * IL CONTROLLO DI VERSIONE E' SENZA SEGNO, ed e' una MISURA:
 * "6b09011f cmp"@0xffffff8008a65cd0 e' seguito da
 * "540002e8 b.hi"@0xffffff8008a65cd4, non da `b.gt`. Con `core_mp.c16`
 * dichiarato `int` e la costante scritta `0x10401` il confronto sarebbe con
 * segno; la costante e' quindi scritta `0x10401U`, che e' cio' che forza la
 * conversione. QUALE DELLE DUE FORME stia nel sorgente di fabbrica -- campo
 * senza segno oppure costante senza segno -- il binario NON lo dice: e' una
 * SCELTA, presa cosi' per non toccare `struct ilitek_core_mp`, che altri
 * lotti hanno gia' misurato.
 *
 * I SEI CODICI DI RITORNO sono letti uno per uno dalle `mov` che li
 * materializzano e NON hanno un nome nel binario:
 *   "12800d15 mov"@0xffffff8008a65bec  = -105
 *   "12800d35 mov"@0xffffff8008a65e60  = -106
 *   "12800c75 mov"@0xffffff8008a65ebc  = -100
 *   "12800db5 mov"@0xffffff8008a65ed0  = -110
 *   "12800d55 mov"@0xffffff8008a65ef4  = -107
 *   "12800d75 mov"@0xffffff8008a65f50  = -108
 *
 * IL `%d` DEL MESSAGGIO «Failed to parse PV54 command» E' LA COSTANTE ZERO
 * ("2a1f03e3 mov"@0xffffff8008a65e58), non il valore reso dalla funzione
 * incorporata: e' la variabile `ret` del CHIAMANTE, che su quel cammino vale
 * ancora il suo inizializzatore. E' un difetto della fabbrica e si riproduce
 * (regola 7). La spiegazione per esteso sta sopra `parser_get_int_data`.
 */
#line 1581
static __used int allnode_open_cdc_data(int mode, int *buf)
{
	int i = 0, idx = 0, ret = 0, len = 0, tmp = 0;
	u8 cmd[15] = {0};
	char str[128] = {0};
	u8 *ori = NULL;
	char *key[] = {"open dac", "open raw1", "open raw2", "open raw3",
		       "open cap1 dac", "open cap1 raw"};

	len = core_mp.c232 * core_mp.c236 * 2 + 2;

	/* "\x016ILITEK: (%s, %d): Read X/Y Channel length = %d, mode = %d\n"
	 * @0xffffff80092444ae, __func__ =
	 * "allnode_open_cdc_data"@0xffffff80092444eb; la guardia e'
	 * "39656129 ldrb"@0xffffff8008a65b6c + "34000129 cbz"@0xffffff8008a65ba8;
	 * "5280c662 mov"@0xffffff8008a65bbc -- __LINE__ = 1587 */
#line 1587
	ILI_DBG("Read X/Y Channel length = %d, mode = %d\n", len, mode);

	/* "71000adf cmp"@0xffffff8008a65bcc + "5400012c b.gt"@0xffffff8008a65bd0 */
	if (len <= 2) {
		/* "\x013ILITEK: (%s, %d): Length is invalid\n"@0xffffff8009243e9d;
		 * "5280c6c2 mov"@0xffffff8008a65be4 -- __LINE__ = 1590 */
#line 1590
		ILI_ERR("Length is invalid\n");
		ret = -105;
		goto out;
	}

	/* "97fff0da bl"@0xffffff8008a65c20 verso <parser_get_ini_key_value> e
	 * "94101261 bl"@0xffffff8008a65c3c verso <snprintf>: e' l'INCORPORAZIONE
	 * di `parser_get_int_data`, e i suoi due `tbnz` consecutivi --
	 * "37f80055 tbnz"@0xffffff8008a65c40 e "37f81000 tbnz"@0xffffff8008a65c44
	 * -- sono l'espressione `(ret < 0) ? 0 : size` provata contro zero.
	 * "9138d442 add"@0xffffff8008a65c2c mette "%s"@0xffffff8009100e35 in x2,
	 * "910cb400 add"@0xffffff8008a65c14 "pv5_4 command"@0xffffff800924332d in
	 * x0 e "321903e1 orr"@0xffffff8008a65c38 la dimensione 128 in w1 */
	if (parser_get_int_data("pv5_4 command", key[mode], str, sizeof(str)) < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to parse PV54 command, ret = %d\n"
		 * @0xffffff8009244501;
		 * "5280c7a2 mov"@0xffffff8008a65e54 -- __LINE__ = 1597 */
#line 1597
		ILI_ERR("Failed to parse PV54 command, ret = %d\n", ret);
		ret = -106;
		goto out;
	}

	/* "97ffff5f bl"@0xffffff8008a65c54, w2 = 15
	 * ("32000fe2 orr"@0xffffff8008a65c50). Il valore reso NON e' provato */
	parser_get_u8_array(str, cmd, sizeof(cmd));

	/* "97ffd39a bl"@0xffffff8008a65c70 con
	 * "Open SP command"@0xffffff800924453d in x4 */
	ilitek_dump_data(cmd, 8, sizeof(cmd), 0, "Open SP command");

	/* "b902dd09 str"@0xffffff8008a65c88 -- `idev` + 732 = 1 */
	idev->c732 = 1;

	/* "b94be541 ldr"@0xffffff8008a65c90 legge 0xffffff800a0fcbe4, cioe'
	 * `core_mp.c228`; "f9418508 ldr"@0xffffff8008a65c8c prende `idev->c776`
	 * e "d63f0100 blr"@0xffffff8008a65c94 lo chiama */
	if (idev->c776(cmd, core_mp.c228) < 0) {
		/* "\x013ILITEK: (%s, %d): Write CDC command failed\n"
		 * @0xffffff8009243f06;
		 * "5280c922 mov"@0xffffff8008a65eb4 -- __LINE__ = 1609 */
#line 1609
		ILI_ERR("Write CDC command failed\n");
		ret = -100;
		goto out;
	}

	/* "b94c6508 ldr"@0xffffff8008a65ca0 legge 0xffffff800a0fcc64
	 * (`core_mp.c356`), "7100051f cmp"@0xffffff8008a65ca4 lo confronta con 1
	 * e "540010e1 b.ne"@0xffffff8008a65ca8 sceglie l'altro ramo */
	if (core_mp.c356 == 1)
		/* "97ffce08 bl"@0xffffff8008a65cb4, w0 = w1 = 50 */
		ret = ilitek_tddi_ic_check_busy(50, 50);
	else
		/* "97ffcd41 bl"@0xffffff8008a65ec4, nessun argomento */
		ret = ilitek_tddi_ic_check_int_stat();

	if (ret < 0) {
		ret = -110;
		goto out;
	}

	/* "b94b1108 ldr"@0xffffff8008a65cc4 legge 0xffffff800a0fcb10
	 * (`core_mp.c16`); la costante 0x10401 e'
	 * "52808029 mov"@0xffffff8008a65cc8 + "72a00029 movk"@0xffffff8008a65ccc */
	if ((unsigned int)core_mp.c16 <= 0x10401U) {
		/* "529e5ec9 mov"@0xffffff8008a65cdc (0xf2f6) +
		 * "790173e9 strh"@0xffffff8008a65ce0: DUE byte in una volta */
		cmd[0] = 0xF6;
		cmd[1] = 0xF2;
		/* "321f03e1 orr"@0xffffff8008a65cec -- lunghezza 2 */
		if (idev->c776(cmd, 2) < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x, 0x%x) error\n"
			 * @0xffffff8009243f34, con
			 * "3942e3e3 ldrb"@0xffffff8008a65f08 e
			 * "3942e7e4 ldrb"@0xffffff8008a65f0c;
			 * "5280cbe2 mov"@0xffffff8008a65f20 -- __LINE__ = 1631 */
#line 1631
			ILI_ERR("Write (0x%x, 0x%x) error\n", cmd[0], cmd[1]);
			ret = -100;
			goto out;
		}
		/* "52912b00 mov"@0xffffff8008a65cfc + "72a00820 movk"@0xffffff8008a65d00
		 * = 0x418958 = 4295000 = 1000 * 0x10c7, cioe' `udelay(1000)` */
		mdelay(1);

		/* "b24002f7 orr"@0xffffff8008a65d04 mette a uno il bit 0 del
		 * puntatore -- `sp+184` e' allineato a otto, quindi e'
		 * `&cmd[1]` */
		if (idev->c776(&cmd[1], 1) < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x) error\n"
			 * @0xffffff8009243f62, con
			 * "3942e7e3 ldrb"@0xffffff8008a65f64;
			 * "5280cd02 mov"@0xffffff8008a65f78 -- __LINE__ = 1640 */
#line 1640
			ILI_ERR("Write (0x%x) error\n", cmd[1]);
			ret = -108;
			goto out;
		}
		mdelay(1);
	}

	/* "52901801 mov"@0xffffff8008a65d30 + "72a02801 movk"@0xffffff8008a65d38
	 * = 0x14080c0, cioe' GFP_KERNEL | __GFP_ZERO: e' `kzalloc`, e la
	 * dimensione non e' costante, quindi la chiamata e' a <__kmalloc>
	 * ("97dfa993 bl"@0xffffff8008a65d3c) */
	ori = kzalloc(len, GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a65d44 + "54000c88 b.hi"@0xffffff8008a65d48
	 * (IS_ERR) e "b4000c77 cbz"@0xffffff8008a65d4c (NULL) */
	if (IS_ERR(ori) || ori == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate ori, (%ld)\n"
		 * @0xffffff8009243f8a, con "aa1703e3 mov"@0xffffff8008a65eec;
		 * "5280ce82 mov"@0xffffff8008a65ee8 -- __LINE__ = 1652 */
#line 1652
		ILI_ERR("Failed to allocate ori, (%ld)\n", PTR_ERR(ori));
		ret = -107;
		goto out;
	}

	/* "f9418908 ldr"@0xffffff8008a65d5c prende `idev->c784` e
	 * "d63f0100 blr"@0xffffff8008a65d60 lo chiama con w1 = len */
	if (idev->c784(ori, len) < 0) {
		/* "\x013ILITEK: (%s, %d): Read cdc data error, len = %d\n"
		 * @0xffffff8009243fbd;
		 * "5280cf62 mov"@0xffffff8008a65f40 -- __LINE__ = 1659 */
#line 1659
		ILI_ERR("Read cdc data error, len = %d\n", len);
		ret = -108;
		goto out;
	}

	/* "97ffd356 bl"@0xffffff8008a65d80 con
	 * "Open SP CDC original"@0xffffff800924454d */
	ilitek_dump_data(ori, 8, len, 0, "Open SP CDC original");

	/* "b98c0102 ldrsw"@0xffffff8008a65df4 rilegge `core_mp.c256` a OGNI
	 * giro: `buf` e' un puntatore che il compilatore non puo' escludere
	 * punti dentro `core_mp`.
	 *
	 * I DUE INDICI SONO DUE CATENE DISTINTE, ED E' UNA MISURA. Il binario
	 * porta DUE variabili d'induzione a 64 bit, non una: x12 parte da
	 * 0x100000000 ("b26003ec orr"@0xffffff8008a65da0) e x13 da
	 * 0x200000000 ("b25f03ed orr"@0xffffff8008a65da4), tutt'e due con
	 * passo 0x200000000 ("8b0b01ad add"@0xffffff8008a65dfc e
	 * "8b0b018c add"@0xffffff8008a65e00), e i due indici escono da
	 * "9360fd8e asr"@0xffffff8008a65da8 e "9360fdaf asr"@0xffffff8008a65db0.
	 * Scritti come `ori[2*i+1]` e `ori[2*i+2]` clang mette in comune il
	 * `2*i` e ne deriva il primo indice con un `orr ...,#1`: due istruzioni
	 * in piu' e 1184 byte invece di 1176. Scritto `idx = 2*i+1` e poi
	 * `ori[idx]` / `ori[idx + 1]`, le due catene restano separate e il
	 * conto torna al byte. Che nel sorgente di fabbrica ci sia proprio una
	 * variabile `idx` e' una SCELTA: cio' che e' misurato e' che le due
	 * catene sono indipendenti */
	for (i = 0; i < core_mp.c256; i++) {
		idx = (2 * i) + 1;

		/* "121d7a8a and"@0xffffff8008a65d98 (mode & ~4) +
		 * "340000ea cbz"@0xffffff8008a65db4: e' `mode == 0 || mode == 4`
		 * ripiegato in una sola prova */
		if (mode == 0 || mode == 4) {
			/* "120019d0 and"@0xffffff8008a65dd4 +
			 * "721901df tst"@0xffffff8008a65dd8 +
			 * "5a90060e cneg"@0xffffff8008a65ddc, e la coppia
			 * gemella per il secondo byte */
			tmp = ((ori[idx] & 0x80) ?
			       -(ori[idx] & 0x7F) :
			       (ori[idx] & 0x7F)) +
			      ((ori[idx + 1] & 0x80) ?
			       -(ori[idx + 1] & 0x7F) :
			       (ori[idx + 1] & 0x7F));
		} else {
			/* "33181dcf bfi"@0xffffff8008a65dc0 fonde i due byte */
			tmp = (ori[idx] << 8) + ori[idx + 1];
			/* "32103df0 orr"@0xffffff8008a65dc4 (| 0xffff0000) e
			 * "1a9001ee csel"@0xffffff8008a65dc8: la sottrazione di
			 * 65536 su un valore che sta in sedici bit E' un `orr`,
			 * la prova e' sul BYTE ALTO GREZZO, non sul bit 15 del
			 * valore fuso: "721901df tst"@0xffffff8008a65dbc mette
			 * alla prova w14 (cioe' `ori[2i+1]`) e sta PRIMA della
			 * `bfi`. Scritta come `tmp >= 0x8000` la stessa
			 * condizione diventa `lsr w16,w15,#15` + `cmp` + `csel`,
			 * cioe' una istruzione in piu' */
			if (ori[idx] & 0x80)
				tmp -= 0x10000;
		}
		/* UNA SOLA `str` per tutti e due i rami
		 * ("b8297a6e str"@0xffffff8008a65df0): il valore passa per una
		 * variabile temporanea, non per `buf[i]`. Scritto con
		 * `buf[i]` come accumulatore le `str` diventano DUE, perche' il
		 * compilatore non puo' escludere che `buf` punti dentro
		 * `core_mp` */
		buf[i] = tmp;
	}

	/* "97ffd32d bl"@0xffffff8008a65e24 con
	 * "Open SP CDC combined"@0xffffff8009244562, w1 = 10
	 * ("52800141 mov"@0xffffff8008a65e1c) e w3 = `core_mp.c232`
	 * ("b94be903 ldr"@0xffffff8008a65e10) */
	ilitek_dump_data(buf, 10, core_mp.c256, core_mp.c232,
			 "Open SP CDC combined");

out:
	/* "b902dd1f str"@0xffffff8008a65e6c e "b902dd1f str"@0xffffff8008a65efc */
	idev->c732 = 0;
	/* "b50002d7 cbnz"@0xffffff8008a65f00 + "97dfaa15 bl"@0xffffff8008a65f5c
	 * verso <kfree>. Sui sei cammini in cui `ori` vale ancora NULL la
	 * liberazione non compare affatto, ed e' per questo che le uscite sono
	 * DUE: 0xffffff8008a65e64 senza e 0xffffff8008a65ef8 con */
	ipio_kfree((void **)&ori);
	return ret;
}

/*
 * ===========================================================================
 * I TRE PARAMETRI DEL COLLAUDO OPEN -- 0xffffff800a0ff410, tre `int`
 * ===========================================================================
 * I NOMI DEI TRE CAMPI SONO NEL BINARIO, non scelti: il messaggio di
 * `open_test_cap` a 0xffffff80092445f3 li nomina uno per uno --
 * "\x016ILITEK: (%s, %d): open_test_c: frame_cont = %d, gain = %d, tvch = %d, tvcl = %d\n"
 * -- e i quattro argomenti di quella `printk` dicono quale indirizzo porta
 * quale nome:
 *
 *   w3 = "b9400343 ldr"@0xffffff8008a645a4   mp_items[index] + 48  frame_cont
 *   w4 = "b9400904 ldr"@0xffffff8008a645a8   base + 8 = 0xa0ff418  gain
 *   w5 = "b9400105 ldr"@0xffffff8008a645ac   base + 0 = 0xa0ff410  tvch
 *   w6 = "2a0003e6 mov"@0xffffff8008a645b0   il valore appena reso da
 *                                            `katoi` e scritto a 0xa0ff414   tvcl
 *
 * dove la base e' "91104108 add"@0xffffff8008a645a0 (pagina + 0x410). Le tre
 * scritture confermano l'accoppiamento con la chiave .ini interrogata:
 * "gain"@0xffffff800927b2b7 -> "b9041900 str"@0xffffff8008a6449c (+1048),
 * "tvch"@0xffffff800924426f -> "b9041100 str"@0xffffff8008a6450c (+1040),
 * "tvcl"@0xffffff8009244274 -> "b9041500 str"@0xffffff8008a64588 (+1044).
 *
 * CHE SIANO UN SOLO OGGETTO E' MISURATO: "29400d22 ldp"@0xffffff8008a5e234
 * legge 0xa0ff410 e 0xa0ff414 in UNA istruzione, e
 * "b9400b77 ldr"@0xffffff8008a5e308 legge 0xa0ff418 come [base + 8]. Due
 * globali distinti non si possono leggere con una `ldp`: o sono i campi di
 * una struttura, o sono tre `static` che GlobalMerge (acceso di suo su
 * AArch64) ha fuso. QUALE DELLE DUE il binario NON lo dice -- e' una SCELTA,
 * dichiarata qui (F3b-S1). Non c'e' nessun accesso a 0xa0ff41c in tutto il
 * gruppo F (`grep -c "#1052\]" gruppoF.asm` -> 0): il terzo campo e' l'ultimo.
 */
struct ilitek_open_para {
	int tvch;
	int tvcl;
	int gain;
};
struct ilitek_open_para open_para;

/*
 * ===========================================================================
 * allnode_open_cdc_result -- 0xffffff8008a5e150, 984 byte, `T`
 * ===========================================================================
 * NON E' `static`: nella mappa e' `T`
 * (`grep -w allnode_open_cdc_result oracolo/stock.map` ->
 * "ffffff8008a5e150 T allnode_open_cdc_result"). Per questo NON porta
 * `__used` -- a differenza delle due funzioni del lotto F3a, che sono `t`.
 *
 * NON RENDE NIENTE: sul cammino d'uscita (0xffffff8008a5e4f0 e seguenti) non
 * c'e' nessuna istruzione che scriva w0, e i due rami di collaudo lo
 * distruggono; anche il chiamante scritto qui accanto,
 * "97ffe69b bl"@0xffffff8008a646e4 dentro `open_test_cap`, non prova il
 * risultato. Quindi `void`.
 *
 * QUATTRO PARAMETRI. Il primo e' l'INDICE dell'elemento e il suo segno e'
 * misurato: "9b297c09 smull"@0xffffff8008a5e17c moltiplica w0 per 152 con
 * ESTENSIONE DI SEGNO, e "f8696916 ldr"@0xffffff8008a5e190 legge
 * `mp_items[index].c0` (la base e' "911b4108 add"@0xffffff8008a5e18c). Gli
 * altri tre sono `int *`: x1 e' la DESTINAZIONE
 * ("b8206aa1 str"@0xffffff8008a5e29c e
 * "b83a7aa8 str"@0xffffff8008a5e4dc), x2 e x3 sono letti allo stesso indice
 * ("b8606a85 ldr"@0xffffff8008a5e230 e "b8606a64 ldr"@0xffffff8008a5e22c).
 *
 * I NOMI `dac` e `raw` SONO SCELTI, ma non a caso: l'unico chiamante scritto
 * riempie x2 con `allnode_open_cdc_data(4, ...)` e x3 con
 * `allnode_open_cdc_data(5, ...)`, e i modi 4 e 5 sono le chiavi
 * "open cap1 dac"@0xffffff8009244492 e "open cap1 raw"@0xffffff80092444a0
 * della tavola `key[]` di quella funzione.
 *
 * IL NOME DELL'ELEMENTO SCEGLIE FRA DUE FORMULE, e il confronto e' a due
 * passi come in `ilitek_tddi_mp_init_item`: prima la lunghezza
 * ("f1002c1f cmp"@0xffffff8008a5e1a8 = 11 e
 * "f100645f cmp"@0xffffff8008a5e1b0 = 25), poi la `strncmp`
 * ("940fe870 bl"@0xffffff8008a5e1c4 e "940fe831 bl"@0xffffff8008a5e2c0). Le
 * due stringhe sono "open test_c"@0xffffff80092431b5 e
 * "open test(integration)_sp"@0xffffff800924319b.
 *
 * L'ORDINE DEI DUE RAMI NEL SORGENTE E' MISURATO, ed e' il ROVESCIO di quello
 * che il lotto F3b aveva scritto: il ramo "open test(integration)_sp" viene
 * PRIMA e "open test_c" dopo. La prova e' che il compilatore mette in linea il
 * SECONDO ramo del sorgente e manda fuori linea il primo. Con l'ordine
 * "test_c" prima, lo stesso clang (r353983c) emette
 * `cmp x0, #0x19; b.eq` come primo confronto; di fabbrica il primo confronto
 * e' "f1002c1f cmp"@0xffffff8008a5e1a8 = 11, cioe' "open test_c" e' quello
 * mandato fuori linea (a 0xffffff8008a5e2b4), quindi e' il SECONDO del
 * sorgente. Con l'ordine scritto qui il nostro oggetto riproduce
 * `cmp #0xb; b.eq; cmp #0x19; b.ne` nello stesso ordine.
 *
 * IL TERZO ARGOMENTO DELLE DUE `strncmp` E' IL VALORE RESO DA `strlen`, non
 * la costante: "aa0003e2 mov"@0xffffff8008a5e1a4 mette x2 = x0 PRIMA di
 * qualunque confronto e nessuna delle due `bl` ha una `mov w2` che la
 * preceda. Perche' clang non ripieghi x2 a costante dentro il ramo, il valore
 * CONFRONTATO e quello PASSATO devono essere due valori distinti: qui il
 * confronto e' su `len`, che e' un `int` (quindi un troncamento del `size_t`
 * reso da `strlen`), mentre la `strncmp` riceve `strlen(...)` per intero.
 * GVN propaga l'uguaglianza sul valore troncato e non su quello a 64 bit, e
 * x2 resta vivo. Misurato: con `size_t len` unico, o con due `strlen`
 * separate, o con un `switch` sulla lunghezza, lo stesso clang emette
 * `mov w2, #0xb` e `mov w2, #0x19` (tre forme provate, tutte e tre
 * ripiegate).
 *
 * LA DIVISIONE PER 100 e quella PER 10 sono le forme con cui clang emette una
 * divisione CON SEGNO per costante: 0x51eb851f con `asr #37`
 * ("9b2e7c21 smull"@0xffffff8008a5e260 + "9365fc21 asr"@0xffffff8008a5e26c)
 * e 0x66666667 con `asr #34` ("9b327c84 smull"@0xffffff8008a5e270 +
 * "9362fc84 asr"@0xffffff8008a5e27c).
 *
 * NEL RAMO "open test_c" I VALORI SONO COPIATI IN LOCALI IN TESTA AL CORPO
 * DEL CICLO, e questo e' MISURATO, non scelto: "29402b69 ldp"@0xffffff8008a5e300
 * (`open_para.tvch` e `open_para.tvcl`), "b9400b77 ldr"@0xffffff8008a5e308
 * (`open_para.gain`), "394009bc ldrb"@0xffffff8008a5e30c (`core_mp.c6`),
 * "b8686a98 ldr"@0xffffff8008a5e310 (`dac[i]`) e
 * "b8686a79 ldr"@0xffffff8008a5e314 (`raw[i]`) stanno tutte PRIMA della
 * "940fe724 bl"@0xffffff8008a5e370 (`__memset`) e delle tre chiamate che la
 * seguono. Una lettura di globale o di memoria puntata non si puo' spostare
 * sopra una chiamata che la potrebbe scrivere: se il binario la fa prima, il
 * sorgente la fa prima. E' la classe A5 dei criteri di revisione, e il lotto
 * F3b l'aveva sbagliata: senza i locali il nostro oggetto misurava 960 byte.
 * Anche "4b0a0136 sub"@0xffffff8008a5e318 (tvch - tvcl) e' prima del salto.
 *
 * IL TERMINE `dac` E' SCRITTO PRIMA DEL TERMINE `raw` nelle quattro
 * espressioni, e anche questo e' misurato per via indiretta. Di fabbrica NON
 * C'E' NESSUNA `sub` per `raw[i] - 8192` nel ramo "open test_c": la
 * sottrazione e' distribuita dentro la moltiplicazione e finisce
 * nell'addendo costante della `madd` condivisa
 * ("1b092b29 madd"@0xffffff8008a5e4b8, con
 * "52b11a4a mov"@0xffffff8008a5e350 = -1999503360 = -8192 * 244080 e
 * "52b08268 mov"@0xffffff8008a5e3e8 = -2079129600 = -8192 * 253800).
 * InstCombine distribuisce `(X - C1) * C2` solo se la `sub` ha UN SOLO uso;
 * scrivendo il termine `raw` per primo, la `sub` e' la prima istruzione di
 * tutti e quattro i blocchi e SimplifyCFG la solleva sopra i salti prima che
 * InstCombine la veda, la `sub` finisce con quattro usi e la `madd` non
 * nasce. Misurato: col termine `raw` per primo il nostro oggetto ha 13 `mul`
 * e 0 `madd`, col termine `dac` per primo ne ha 6 e 1, come la fabbrica.
 *
 * `str[32]` E' DICHIARATA DENTRO IL CICLO, ed e' misurato: le due
 * "a9027fff stp"@0xffffff8008a5e320 e "a9017fff stp"@0xffffff8008a5e324
 * azzerano 32 byte a ogni giro, in testa al corpo, PRIMA della prova su
 * `core_mp.c4` ("6b0e017f cmp"@0xffffff8008a5e31c).
 *
 * IL RITORNO DI `parser_get_int_data` NON E' PROVATO: dopo
 * "94103088 bl"@0xffffff8008a5e3a0 (la `snprintf` della funzione incorporata)
 * viene subito "94003484 bl"@0xffffff8008a5e3a8, cioe' `katoi`, senza nessun
 * confronto in mezzo. E il ramo d'errore di riga 566 non compare affatto,
 * perche' i tre puntatori passati sono due letterali e un vettore di pila.
 *
 * F3b-D1 -- QUASI CHIUSA: 976 byte contro 984, DUE istruzioni in meno (erano
 * SEI). Il lotto F3b/F3c ha trovato e corretto tre fatti di forma (l'ordine
 * dei due rami, i locali in testa al ciclo, il termine `dac` per primo) e la
 * forma che tiene x2 vivo; quello che resta e' UNO SOLO, ed e' isolato.
 *
 *   $NM --print-size ilitek_mp.o | grep -w allnode_open_cdc_result
 *   0000000000006318 00000000000003d0 T allnode_open_cdc_result
 *   grep -w allnode_open_cdc_result oracolo/stock.map
 *   ffffff8008a5e150 T allnode_open_cdc_result        (984 byte, 246 istr.)
 *
 * DOVE. L'INDIRIZZO DI `core_mp.c4`. Di fabbrica e' materializzato con
 * `adrp` + `add` ("d000b4ed adrp"@0xffffff8008a5e2dc +
 * "912c11ad add"@0xffffff8008a5e2ec, 0xb04 = lo12 di `core_mp`+4) e SERVE
 * DUE LETTURE: "794001ab ldrh"@0xffffff8008a5e304 (`c4`) e
 * "394009bc ldrb"@0xffffff8008a5e30c (`c6`, cioe' [base + 2]). La stessa
 * coppia `adrp`+`add` e' rimaterializzata in altri due punti
 * ("912c11ad add"@0xffffff8008a5e424 e "912c11ad add"@0xffffff8008a5e4b0):
 * in tutto TRE `add` che noi non abbiamo. Da noi la lettura di `c6` ha UN
 * SOLO uso e InstCombine la fa scendere nel ramo `else`, dove si porta
 * dietro un `adrp` suo; il `ldrh` di `c4` resta solo sulla sua pagina e
 * clang gli ripiega il lo12 dentro. Bilancio: la fabbrica ha +4 istruzioni
 * (tre `add` e il `ldrb` in testa al ciclo), noi ne abbiamo +2 (`adrp` e
 * `ldrb` nel ramo `else`), da cui le due di scarto. E' VERIFICATO che
 * impedire la discesa basta a far comparire l'`adrp` giusto: scrivendo il
 * collaudo con `accuracy` esterno e `c6` interno -- che da a `c6` DUE usi --
 * il conto degli `adrp` diventa 16 contro 16, ma la forma dei quattro rami
 * cambia e il totale peggiora (235 istruzioni). La forma di sorgente che
 * tiene la lettura in testa SENZA cambiare i quattro rami non e' stata
 * trovata: e' cio' che resta di F3b-D1.
 *
 * AGGIUNTA DEL LOTTO F3c -- IL MECCANISMO E' LO STESSO DELLA SCOPERTA F3c-S1,
 * VISTO DALL'ALTRA PARTE. La coppia `adrp`+`add` che qui manca e' esattamente
 * la forma che clang emette quando DUE O PIU' campi dello STESSO oggetto sono
 * letti nello STESSO blocco: la base viene materializzata a parte (AArch64 non
 * ammette un globale come base di modo d'indirizzamento) e i campi diventano
 * spiazzamenti. Di fabbrica accade per `core_mp.c4` e `core_mp.c6`
 * ("912c11ad add"@0xffffff8008a5e2ec, poi
 * "794001ab ldrh"@0xffffff8008a5e304 e "394009bc ldrb"@0xffffff8008a5e30c) e,
 * nella stessa funzione e nello stesso blocco, per i tre campi di `open_para`
 * ("9110437b add"@0xffffff8008a5e2e8, poi "29402b69 ldp"@0xffffff8008a5e300 e
 * "b9400b77 ldr"@0xffffff8008a5e308) -- e quella dei tre campi di `open_para`
 * il NOSTRO oggetto la riproduce. Il caso di `core_mp` no, e per un motivo
 * solo: la lettura di `c6` ha UN uso, scende nel ramo `else`, e la sua base
 * resta sola nel proprio blocco. La stessa forma che nei sette puntatori di
 * trama era il DIFETTO da togliere, qui e' la forma da OTTENERE.
 *
 * ALTRE DUE DIVERGENZE DI SOLA CODIFICA, dichiarate:
 *   - i due confronti di lunghezza sono a 32 bit da noi
 *     ("71002c5f cmp" e "7100645f cmp") e a 64 di fabbrica
 *     ("f1002c1f cmp"@0xffffff8008a5e1a8 e
 *     "f100645f cmp"@0xffffff8008a5e1b0). E' il prezzo del troncamento che
 *     tiene x2 vivo: nessuna forma provata da 64 bit E x2 vivo insieme;
 *   - l'ordine degli addendi dell'ultima somma: di fabbrica
 *     "0b0b0108 add"@0xffffff8008a5e4c0 (`raw` + `dac`), da noi
 *     `add w8, w11, w8` (`dac` + `raw`). Scambiarli riporta la `sub` in
 *     testa ai quattro blocchi e fa sparire la `madd`, che costa dodici
 *     istruzioni: la somma di dodici e uno non e' migliore di uno.
 */
void allnode_open_cdc_result(int index, int *buf, int *dac, int *raw)
{
	int i;
	int len = strlen(mp_items[index].c0);

	if (len == (int)strlen("open test(integration)_sp") &&
	    strncmp(mp_items[index].c0, "open test(integration)_sp",
		    strlen(mp_items[index].c0)) == 0) {
		for (i = 0; i < core_mp.c256; i++) {
			buf[i] = (dac[i] *
				  ((core_mp.c4 == 0x9881) ? 1610000 : 1310000) /
				  100 -
				  (8192 - raw[i]) * 140000 / 16384 * 36 / 10) /
				 (open_para.tvch - open_para.tvcl) / 2;
		}
	} else if (len == (int)strlen("open test_c") &&
		   strncmp(mp_items[index].c0, "open test_c",
			   strlen(mp_items[index].c0)) == 0) {
		for (i = 0; i < core_mp.c256; i++) {
			char str[32] = {0};
			int accuracy = 0;
			int rawv = raw[i];
			int dacv = dac[i];
			int vdiff = open_para.tvch - open_para.tvcl;
			int gain = open_para.gain;
			int c4 = core_mp.c4;
			int c6 = core_mp.c6;

			if (c4 == 0x9881) {
				buf[i] = (dacv * 16146 / 2 +
					  (rawv - 8192) * 244080 / 16384) /
					 vdiff / 100 / gain;
			} else {
				parser_get_int_data("open test_c", "accuracy",
						    str, sizeof(str));
				accuracy = katoi(str);

				if (c6 == 27) {
					if (accuracy)
						buf[i] = (dacv * 14664 / 2 *
							  10 +
							  (rawv - 8192) *
							  158625 / 1024) /
							 vdiff / 100 / gain;
					else
						buf[i] = (dacv * 14664 / 2 +
							  (rawv - 8192) *
							  253800 / 16384) /
							 vdiff / 100 / gain;
				} else {
					if (accuracy)
						buf[i] = (dacv * 13143 / 2 *
							  10 +
							  (rawv - 8192) *
							  152550 / 1024) /
							 vdiff / 100 / gain;
					else
						buf[i] = (dacv * 16146 / 2 +
							  (rawv - 8192) *
							  244080 / 16384) /
							 vdiff / 100 / gain;
				}
			}
		}
	}
}

/*
 * ===========================================================================
 * open_test_sp -- 0xffffff8008a632bc, 3932 byte, `t` (983 istruzioni)
 * ===========================================================================
 * ESATTA AL BYTE con tutti e due i compilatori, e per CODIFICA il residuo e'
 * SETTE su 983 -- le sette istruzioni di guardia di `compare_charge`
 * incorporata, che di fabbrica sono nello stesso insieme ma in ordine diverso
 * (divergenza F3c-D2, in fondo).
 *
 * `static`: nella mappa e' `t` (`grep -E " open_test_sp$" oracolo/stock.map`
 * -> "ffffff8008a632bc t open_test_sp").
 *
 * UN SOLO PARAMETRO, ED E' L'INDICE DELL'ELEMENTO, CON SEGNO:
 * "9b29501a smaddl"@0xffffff8008a632f4 moltiplica w0 per
 * "52801309 mov"@0xffffff8008a632f0 = 0x98 = 152 e somma la base della
 * tavola ("911b4294 add"@0xffffff8008a632ec, pagina + 0x6d0). `smaddl` e'
 * l'estensione CON SEGNO: un `int`, non un `unsigned`.
 *
 * IL VETTORE DI LAVORO E' `int *cdc[frame_count][9]`, E IL NOVE E' MISURATO:
 * "52800908 mov"@0xffffff8008a63304 mette w8 = 0x48 = 72 byte per riga e
 * "9b0826a8 madd"@0xffffff8008a63310 lo moltiplica per `c48`; 72 / 8 = 9
 * puntatori. Che siano NOVE e non uno da 72 byte lo prova il ciclo di
 * liberazione, che scorre nove `ldr`/`ldur` a passo 8 dentro la stessa riga --
 * da "f85e02a0 ldur"@0xffffff8008a63618 (riga + 0) a
 * "f94012a0 ldr"@0xffffff8008a63698 (riga + 64) -- e poi avanza di 0x48
 * ("910122b5 add"@0xffffff8008a636b0).
 *
 * A COSA SERVONO I NOVE, misurato dai modi passati a `allnode_open_cdc_data`
 * e dagli usi:
 *   [0] modo 1 ("320003e0 orr"@0xffffff8008a63cd0)      Raw1
 *   [1] modo 2 ("321f03e0 orr"@0xffffff8008a63ce8)      Raw2
 *   [2] modo 3 ("320007e0 orr"@0xffffff8008a63d04)      Raw3
 *   [3] risultato di [8] e [0] ("97ffe907 bl"@0xffffff8008a63d34)
 *   [4] risultato di [8] e [1] ("97ffe900 bl"@0xffffff8008a63d50)
 *   [5] risultato di [8] e [2] ("97ffe8fa bl"@0xffffff8008a63d68)
 *   [6] [4]*100/[3], il "charge rate" ("1b017d8c mul"@0xffffff8008a63e88 con
 *       "52800c81 mov"@0xffffff8008a63e58 = 100, e
 *       "1ace0d8c sdiv"@0xffffff8008a63e8c)
 *   [7] [3]-[5], il "full open" ("4b0e018c sub"@0xffffff8008a63ea8)
 *   [8] modo 0 ("2a1f03e0 mov"@0xffffff8008a63cb0)      DAC
 *
 * `str[512]` E' AZZERATA ALL'INGRESSO, PRIMA DI TUTTO IL RESTO:
 * "321703e2 orr"@0xffffff8008a63328 = 512 e
 * "940fd334 bl"@0xffffff8008a63330 verso <__memset>, subito dopo
 * "9100031f mov"@0xffffff8008a63320 che assesta lo stack del vettore
 * variabile. E' un inizializzatore di dichiarazione, come in
 * `parser_get_int_data`.
 *
 * `mp_items[index].c48` E' RILETTO DOPO LA `printk`, e questo decide la forma
 * del sorgente: "b9400375 ldr"@0xffffff8008a63364 ricarica il campo dopo
 * "97db405d bl"@0xffffff8008a63360, e solo allora
 * "710002bf cmp"@0xffffff8008a63368 lo confronta con zero. Una `printk` puo'
 * scrivere ovunque, quindi il confronto NON e' sul locale gia' letto per
 * dimensionare il vettore: il sorgente rilegge il campo. Con
 * `if (frame_count < 1)` la rilettura sparisce. (In `open_test_cap`, dove non
 * c'e' nessuna stampa prima del confronto, il locale basta ed e' quello che
 * quella funzione scrive.)
 *
 * L'ORDINE DI DICHIARAZIONE DEI CONTATORI E' MISURATO -- `i, j, k, x, y` --
 * e vale QUATTORDICI codifiche. Nei tre cicli doppi che portano un contatore
 * di riga e un indice progressivo, di fabbrica il contatore esterno sta in w8
 * e l'indice progressivo in w9 ("2a1f03e8 mov"@0xffffff8008a63e44 e
 * "2a1f03e9 mov"@0xffffff8008a63e48, poi
 * "0b0d012f add"@0xffffff8008a63e70 = w9 + w13 e
 * "11000508 add"@0xffffff8008a63ec4 = w8 + 1). Dichiarando `k` PRIMA di `j`
 * i due registri si scambiano e quattordici codifiche divergono senza che la
 * dimensione cambi: e' la classe di difetto che la misura di dimensione non
 * denuncia. `open_test_cap` vuole l'ordine OPPOSTO (`i, k, j`) ed e' misurato
 * allo stesso modo; non e' una regola generale ma una misura per funzione.
 *
 * L'INIZIALIZZAZIONE DI `max_buf`/`min_buf` e' la stessa di `open_test_cap`:
 * "320103ec orr"@0xffffff8008a63458 = 0x80000000 = INT_MIN e
 * "32007bed orr"@0xffffff8008a6345c = 0x7fffffff = INT_MAX, con i due limiti
 * RILETTI a ogni giro ("b94be92e ldr"@0xffffff8008a63478 e
 * "b94bee2f ldr"@0xffffff8008a63498).
 *
 * LE TRE ALLOCAZIONI DI TRAMA E QUELLA DEL TIPO DI NODO sono `kcalloc`, e la
 * prova e' il controllo di traboccamento che `kmalloc_array` genera:
 * "37f81aa8 tbnz"@0xffffff8008a63400 prova il BIT 31 del conteggio prima di
 * "d37ef500 lsl"@0xffffff8008a63408 (moltiplicazione per 4) e di
 * "97dfb3de bl"@0xffffff8008a63410 verso <__kmalloc>. Le bandiere sono
 * "52901801 mov"@0xffffff8008a63404 + "72a02801 movk"@0xffffff8008a6340c =
 * 0x140080c0, cioe' GFP_KERNEL con __GFP_ZERO. Il controllo sul risultato e'
 * a due termini -- "b140041f cmn"@0xffffff8008a6341c (IS_ERR) e
 * "b4001a00 cbz"@0xffffff8008a63428 (NULL) -- come in `open_test_cap`.
 *
 * LE TRE `memset` DI RIENTRO USANO `core_mp.c256` COME NUMERO DI BYTE, NON DI
 * INTERI: "b98c0102 ldrsw"@0xffffff8008a633b4 passa il conteggio tale e quale
 * a <__memset> senza nessuno spostamento a sinistra, mentre le tre `memcpy`
 * poco piu' avanti lo moltiplicano per quattro
 * ("531e7508 lsl"@0xffffff8008a63d84). E' un difetto della fabbrica -- azzera
 * un quarto del vettore -- e si riproduce (regola 7). Lo stesso difetto sta in
 * `open_test_cap`.
 *
 * I TRE RAMI D'ERRORE DELLE ALLOCAZIONI DI TRAMA TORNANO SUBITO, senza
 * passare per la liberazione del vettore di lavoro: "12800d40
 * mov"@0xffffff8008a6379c mette w0 = -107 e salta diritto all'epilogo
 * ("f0006fc9 adrp"@0xffffff8008a637a0), mentre il ramo d'errore di
 * `create_mp_test_frame_buffer` scrive -107 nella variabile di ritorno
 * ("12800d48 mov"@0xffffff8008a635d4 + "b9004e68 str"@0xffffff8008a635dc) e
 * passa per la liberazione. Sono DUE forme diverse nel sorgente -- `return
 * -107` e `goto out` -- ed e' misurato, non scelto. La perdita del vettore di
 * lavoro sui tre rami di trama e' quindi un difetto della fabbrica, ma NON e'
 * una perdita vera: quei tre rami stanno prima che il vettore sia riempito.
 *
 * QUANTO IL RAMO D'ERRORE LIBERA, in ordine: prima
 * `ipio_kfree(&mp_items[index].c136)` ("f8488e80 ldr"@0xffffff8008a635f4,
 * pre-indice a +136) e poi i nove puntatori di ogni riga.
 *
 * `dump_node_type_buffer` E' INCORPORATA, e si vede dal `__func__`: il
 * messaggio a 0xffffff8008a63520 porta
 * "dump_node_type_buffer"@0xffffff80092430c6 e
 * "52802e62 mov"@0xffffff8008a6351c = riga 371, cioe' il nome e la riga della
 * funzione CHIAMATA, non di questa. Stessa cosa per `parser_get_int_data`
 * (riga 566, "528046c2 mov"@0xffffff8008a63854) e per `compare_charge` e
 * `full_open_rate_compare`, che non hanno stampe ma si riconoscono dalle
 * costanti: "5290a3f1 mov"@0xffffff8008a63f30 +
 * "72aa3d71 movk"@0xffffff8008a63f40 = 0x51eb851f, la divisione per 100 di
 * `full_open_rate_compare`, e le due tavole di scostamento
 * "912a72d6 add"@0xffffff8008a63c98 (0xffffff8009245a9c = `mp_nb_dx`) e
 * "912af294 add"@0xffffff8008a63c9c (0xffffff8009245abc = `mp_nb_dy`) di
 * `compare_charge`.
 *
 * LE SEI CHIAVI .ini, nell'ordine in cui il binario le interroga:
 * "charge_aa"@0xffffff800924424a, "charge_border"@0xffffff8009244254,
 * "charge_notch"@0xffffff8009244262, "full open"@0xffffff800924442a,
 * "tvch"@0xffffff800924426f, "tvcl"@0xffffff8009244274. Solo l'ULTIMA porta
 * il valore reso: "1a80b3f7 csel"@0xffffff8008a63a6c e
 * "37f83657 tbnz"@0xffffff8008a63aa8 sono l'unico controllo, e questo dice
 * che nel sorgente solo la sesta chiamata assegna a `ret`. Le prime cinque
 * lasciano cadere il valore. "full open" e' la stessa stringa usata come
 * `desp` di `parser_ini_nodetype` ("91038c21 add"@0xffffff8008a634f4): un
 * solo letterale, due usi.
 *
 * IL CODICE D'ERRORE DELLA SESTA CHIAVE E' -106, non -107:
 * "12800d28 mov"@0xffffff8008a64188. `open_test_cap` usa lo stesso -106 sullo
 * stesso errore.
 *
 * I QUATTRO `allnode_open_cdc_data` SONO IN QUEST'ORDINE -- 0, 1, 2, 3 -- e
 * ognuno ha il suo messaggio: DAC ("52812562 mov"@0xffffff8008a641a4 =
 * riga 2347), Raw1 (2352), Raw2 (2357), Raw3 (2362). Il PRIMO e il QUARTO
 * differiscono nella forma: il quarto scrive il valore reso nella variabile
 * di ritorno PRIMA di provarlo ("b9004e60 str"@0xffffff8008a63d10 precede
 * "37f826e0 tbnz"@0xffffff8008a63d14) mentre il primo lo prova subito
 * ("37f826c0 tbnz"@0xffffff8008a63cb8): e' il compilatore che sceglie, non il
 * sorgente, e infatti il nostro oggetto riproduce tutte e due le forme.
 *
 * LA COPIA VERSO I TRE GLOBALI DI TRAMA E' SOLO AL PRIMO GIRO:
 * "b500033a cbnz"@0xffffff8008a63d6c salta i tre `memcpy` quando l'indice di
 * giro non e' zero.
 *
 * F3c-D2 -- DIVERGENZA DI SOLA CODIFICA, DICHIARATA. Le sette istruzioni di
 * guardia del vicino dentro `compare_charge` incorporata sono le STESSE ma in
 * ordine diverso. Di fabbrica:
 *   "b8726a92 ldr"@0xffffff8008a64084   (mp_nb_dy[n])
 *   "0b080252 add"@0xffffff8008a64088   (ny = dy + y)
 *   "6b0f025f cmp"@0xffffff8008a6408c   (ny contro c236)
 *   "540001ea b.ge"@0xffffff8008a64090
 *   "6b03023f cmp"@0xffffff8008a64094   (nx contro c232)
 *   "540001aa b.ge"@0xffffff8008a64098
 *   "37f80192 tbnz"@0xffffff8008a6409c  (ny < 0)
 * da noi le stesse sette nell'ordine nx-contro-c232, ny, ny<0, ny-contro-c236.
 * NON e' una differenza di sorgente: la `compare_charge` FUORI LINEA di
 * fabbrica porta l'ordine che il nostro sorgente scrive --
 * "2b0101ce adds"@0xffffff8008a5e098, "54000264 b.mi"@0xffffff8008a5e09c,
 * "6b0901df cmp"@0xffffff8008a5e0a0, "5400022a b.ge"@0xffffff8008a5e0a4,
 * "b86f69af ldr"@0xffffff8008a5e0a8, "0b0201ef add"@0xffffff8008a5e0ac,
 * "37f801cf tbnz"@0xffffff8008a5e0b0 -- e la nostra copia fuori linea misura
 * 192 byte come quella. E' quindi l'INCORPORAMENTO a riordinare, e le due
 * copie della stessa funzione nello stesso binario di fabbrica hanno gia'
 * ordini diversi fra loro. La dimensione non cambia: 983 istruzioni contro
 * 983.
 */
static int open_test_sp(int index)
{
	int i;
	int j;
	int k;
	int x;
	int y;
	int ret = 0;
	int full_open_rate = 0;
	int charge_aa = 0;
	int charge_border = 0;
	int charge_notch = 0;
	int frame_count = mp_items[index].c48;
	int *cdc[frame_count][9];
	char str[512] = {0};

#line 2226
	ILI_DBG("index = %d, desp = %s, Frame Count = %d\n", index, mp_items[index].c0, mp_items[index].c48);

	if (mp_items[index].c48 < 1) {
#line 2233
		ILI_ERR("Frame count is zero, which is at least set as 1\n");
		mp_items[index].c48 = 1;
	}

	if (create_mp_test_frame_buffer(index, mp_items[index].c48) < 0) {
		ret = -107;
		goto out;
	}

	if (frame1_cbk700 == NULL) {
		frame1_cbk700 = kcalloc(core_mp.c256, sizeof(int),
					  GFP_KERNEL);
		if (IS_ERR(frame1_cbk700) || frame1_cbk700 == NULL) {
#line 2246
			ILI_ERR("Failed to allocate frame1_cbk700 buffer\n");
			return -107;
		}
	} else {
		memset(frame1_cbk700, 0, core_mp.c256);
	}

	if (frame1_cbk250 == NULL) {
		frame1_cbk250 = kcalloc(core_mp.c256, sizeof(int),
					  GFP_KERNEL);
		if (IS_ERR(frame1_cbk250) || frame1_cbk250 == NULL) {
#line 2256
			ILI_ERR("Failed to allocate frame1_cbk250 buffer\n");
			ipio_kfree((void **)&frame1_cbk700);
			return -107;
		}
	} else {
		memset(frame1_cbk250, 0, core_mp.c256);
	}

	if (frame1_cbk200 == NULL) {
		frame1_cbk200 = kcalloc(core_mp.c256, sizeof(int),
					  GFP_KERNEL);
		if (IS_ERR(frame1_cbk200) || frame1_cbk200 == NULL) {
#line 2267
			ILI_ERR("Failed to allocate cbk buffer\n");
			ipio_kfree((void **)&frame1_cbk700);
			ipio_kfree((void **)&frame1_cbk250);
			return -107;
		}
	} else {
		memset(frame1_cbk200, 0, core_mp.c256);
	}

	mp_items[index].c136 = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	if (IS_ERR(mp_items[index].c136) || mp_items[index].c136 == NULL) {
#line 2278
		ILI_ERR("Failed to allocate node_type FRAME buffer\n");
		return -107;
	}

	for (i = 0; i < core_mp.c236; i++) {
		for (j = 0; j < core_mp.c232; j++) {
			mp_items[index].max_buf[i * core_mp.c232 + j] = INT_MIN;
			mp_items[index].min_buf[i * core_mp.c232 + j] = INT_MAX;
		}
	}

	if (mp_items[index].c21 == 1) {
		parser_ini_benchmark(mp_items[index].bench_mark_max,
				     mp_items[index].bench_mark_min,
				     mp_items[index].c22, mp_items[index].c0,
				     core_mp.c256);
		dump_benchmark_data(mp_items[index].bench_mark_max,
				    mp_items[index].bench_mark_min);
	}

	parser_ini_nodetype(mp_items[index].c136, "full open", core_mp.c256);
	dump_node_type_buffer(mp_items[index].c136);

	parser_get_int_data(mp_items[index].c0, "charge_aa", str, 512);
	charge_aa = katoi(str);
	parser_get_int_data(mp_items[index].c0, "charge_border", str, 512);
	charge_border = katoi(str);
	parser_get_int_data(mp_items[index].c0, "charge_notch", str, 512);
	charge_notch = katoi(str);
	parser_get_int_data(mp_items[index].c0, "full open", str, 512);
	full_open_rate = katoi(str);
	parser_get_int_data(mp_items[index].c0, "tvch", str, 512);
	open_para.tvch = katoi(str);
	ret = parser_get_int_data(mp_items[index].c0, "tvcl", str, 512);
	open_para.tvcl = katoi(str);

	if (ret < 0) {
#line 2324
		ILI_ERR("Failed to get parameters from ini file\n");
		ret = -106;
		goto out;
	}

#line 2330
	ILI_DBG("open_test_sp: frame_cont %d, AA %d, Border %d, Notch %d, full_open_rate %d\n", mp_items[index].c48, charge_aa, charge_border, charge_notch, full_open_rate);

	/* il limite e' RILETTO a ogni giro: "b9800368 ldrsw"@0xffffff8008a63c4c
	 * e "eb08029f cmp"@0xffffff8008a63c5c. Nessuna delle nove allocazioni
	 * e' controllata -- di fabbrica non c'e' nessun `cbz` fra una
	 * "97dfb21e bl"@0xffffff8008a63b10 e la successiva (regola 7) */
	for (i = 0; i < mp_items[index].c48; i++) {
		cdc[i][0] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][1] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][2] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][3] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][4] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][5] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][6] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][7] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][8] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	}

	/* "9b0b6355 madd"@0xffffff8008a63ca8 calcola la riga (indice * 0x48) e
	 * "f8440ea1 ldr"@0xffffff8008a63cac legge la colonna 8 col pre-indice */
	for (i = 0; i < mp_items[index].c48; i++) {
		ret = allnode_open_cdc_data(0, cdc[i][8]);
		if (ret < 0) {
#line 2347
			ILI_ERR("Failed to get Open SP DAC data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(1, cdc[i][0]);
		if (ret < 0) {
#line 2352
			ILI_ERR("Failed to get Open SP Raw1 data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(2, cdc[i][1]);
		if (ret < 0) {
#line 2357
			ILI_ERR("Failed to get Open SP Raw2 data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(3, cdc[i][2]);
		if (ret < 0) {
#line 2362
			ILI_ERR("Failed to get Open SP Raw3 data, %d\n", ret);
			goto out;
		}

		allnode_open_cdc_result(index, cdc[i][3], cdc[i][8], cdc[i][0]);
		allnode_open_cdc_result(index, cdc[i][4], cdc[i][8], cdc[i][1]);
		allnode_open_cdc_result(index, cdc[i][5], cdc[i][8], cdc[i][2]);

		if (i == 0) {
			memcpy(frame1_cbk700, cdc[i][3], core_mp.c256 * 4);
			memcpy(frame1_cbk250, cdc[i][4], core_mp.c256 * 4);
			memcpy(frame1_cbk200, cdc[i][5], core_mp.c256 * 4);
		}

		ilitek_dump_data(cdc[i][3], 10, core_mp.c256, core_mp.c232,
				 "cbk 700");
		ilitek_dump_data(cdc[i][4], 10, core_mp.c256, core_mp.c232,
				 "cbk 250");
		ilitek_dump_data(cdc[i][5], 10, core_mp.c256, core_mp.c232,
				 "cbk 200");

		/* "b94006ec ldr"@0xffffff8008a63e24 legge `core_mp.c236` e
		 * "5400054d b.le"@0xffffff8008a63e2c salta il ciclo: il
		 * confronto e' `<= 0`, cioe' CON SEGNO. I tre puntatori sono
		 * riletti a ogni giro ("f940030c ldr"@0xffffff8008a63e68 e
		 * "f94002ae ldr"@0xffffff8008a63e6c) */
		for (j = 0, k = 0; j < core_mp.c236; j++) {
			for (x = 0; x < core_mp.c232; x++) {
				cdc[i][6][k] = cdc[i][4][k] * 100 /
					       cdc[i][3][k];
				cdc[i][7][k] = cdc[i][3][k] - cdc[i][5][k];
				k++;
			}
		}

		ilitek_dump_data(cdc[i][6], 10, core_mp.c256, core_mp.c232,
				 "origin charge rate");
		ilitek_dump_data(cdc[i][7], 10, core_mp.c256, core_mp.c232,
				 "origin full open");

		/* `full_open_rate_compare` incorporata: il `type` passato e'
		 * letto con l'indice progressivo
		 * ("b86bd98c ldr"@0xffffff8008a63f58) mentre la funzione
		 * ricalcola il proprio indice con
		 * "1b03290e madd"@0xffffff8008a63f68 (c232 * y + x). Le due
		 * scritture del ramo a zero sono
		 * "b82cd9bf str"@0xffffff8008a63fac (buf) e
		 * "b82b799f str"@0xffffff8008a63fb4 (colonna 6) */
		for (y = 0, k = 0; y < core_mp.c236; y++) {
			for (x = 0; x < core_mp.c232; x++) {
				if (full_open_rate_compare(cdc[i][7],
							   cdc[i][3], x, y,
							   mp_items[index].c136[k],
							   full_open_rate) == 0) {
					mp_items[index].buf[i * core_mp.c256 + k] = 0;
					cdc[i][6][k] = 0;
				}
				k++;
			}
		}

		ilitek_dump_data(&mp_items[index].buf[i * core_mp.c256], 10,
				 core_mp.c256, core_mp.c232,
				 "after full_open_rate_compare");

		/* `compare_charge` incorporata: i tre `tbnz`/`tbz` sui bit 0, 1
		 * e 2 del tipo -- "370000af tbnz"@0xffffff8008a64058,
		 * "3708006f tbnz"@0xffffff8008a64060 e
		 * "3610038f tbz"@0xffffff8008a64068 -- scelgono fra i tre
		 * valori di soglia nell'ordine aa, border, notch, e
		 * "f100221f cmp"@0xffffff8008a640d0 chiude gli otto giri sui
		 * vicini */
		for (y = 0, k = 0; y < core_mp.c236; y++) {
			for (x = 0; x < core_mp.c232; x++) {
				mp_items[index].buf[i * core_mp.c256 + k] =
					compare_charge(cdc[i][6], x, y,
						       mp_items[index].c136,
						       charge_aa,
						       charge_border,
						       charge_notch);
				k++;
			}
		}

		ilitek_dump_data(&mp_items[index].buf[i * core_mp.c256], 10,
				 core_mp.c256, core_mp.c232,
				 "after compare charge rate");

		compare_MaxMin_result(index,
				      &mp_items[index].buf[i * core_mp.c256]);
	}

out:
	/* "f8488e80 ldr"@0xffffff8008a635f4 (pre-indice a +136),
	 * "97dfb46d bl"@0xffffff8008a635fc verso <kfree>,
	 * "f900029f str"@0xffffff8008a63600 */
	ipio_kfree((void **)&mp_items[index].c136);
	for (i = 0; i < mp_items[index].c48; i++) {
		ipio_kfree((void **)&cdc[i][0]);
		ipio_kfree((void **)&cdc[i][1]);
		ipio_kfree((void **)&cdc[i][2]);
		ipio_kfree((void **)&cdc[i][3]);
		ipio_kfree((void **)&cdc[i][4]);
		ipio_kfree((void **)&cdc[i][5]);
		ipio_kfree((void **)&cdc[i][6]);
		ipio_kfree((void **)&cdc[i][7]);
		ipio_kfree((void **)&cdc[i][8]);
	}
	return ret;
}

/*
 * ===========================================================================
 * open_test_cap -- 0xffffff8008a64218, 1828 byte, `t` (457 istruzioni)
 * ===========================================================================
 * ESATTA AL BYTE con tutti e due i compilatori, e IDENTICA PER CODIFICA:
 * 457 mnemonici su 457, 159 codifiche diverse e tutte e 159 sono siti di
 * rilocazione o chiamate locali -- RESIDUO ZERO. Prima del lotto F3c
 * misurava 1832 (una istruzione in piu'): la causa era la dichiarazione dei
 * sette puntatori di trama come VETTORE, ed e' spiegata per esteso nel
 * cappello di `frame1_cbk700` e compagni.
 *
 * IL VETTORE DI LAVORO E' `int *cdc[frame_count][3]`, e il TRE e' misurato:
 * "321d07e8 orr"@0xffffff8008a64260 mette w8 = 0x18 = 24 byte per riga, cioe'
 * tre puntatori, contro i 0x48 = 72 di `open_test_sp`.
 *
 * QUI NON C'E' NESSUNA STAMPA PRIMA DEL CONFRONTO su `c48`, e per questo il
 * confronto e' sul LOCALE gia' letto ("b8430f55 ldr"@0xffffff8008a6425c e
 * "710002bf cmp"@0xffffff8008a64294, senza rilettura in mezzo). In
 * `open_test_sp`, che stampa prima, il campo e' riletto: le due forme sono
 * diverse ed e' misurato in tutte e due.
 *
 * L'ORDINE DI DICHIARAZIONE DEI CONTATORI E' `i, k, j`, ed e' MISURATO: di
 * fabbrica il contatore di riga sta in w8 e l'indice progressivo in w9
 * ("2a1f03e8 mov"@0xffffff8008a64764, "2a1f03e9 mov"@0xffffff8008a64768,
 * "0b0c012a add"@0xffffff8008a64778 e "11000508 add"@0xffffff8008a647a8).
 * Con `i, j, k` i due registri si scambiano e quattro codifiche divergono
 * senza che la dimensione cambi. `open_test_sp` vuole l'ordine OPPOSTO: e'
 * una misura per funzione, non una regola.
 *
 * IL DIFETTO DELLA FABBRICA sulle due `memset` di rientro -- il conteggio e'
 * il NUMERO DI NODI passato come numero di BYTE
 * ("b98c0102 ldrsw"@0xffffff8008a642dc, nessuno spostamento) mentre le due
 * `memcpy` lo moltiplicano per quattro
 * ("531e7508 lsl"@0xffffff8008a646fc) -- e' lo stesso di `open_test_sp` e si
 * riproduce (regola 7).
 */
static int open_test_cap(int index)
{
	int i;
	int k;
	int j;
	int ret = 0;
	int frame_count = mp_items[index].c48;
	int *cdc[frame_count][3];
	char str[512] = {0};

	if (frame_count < 1) {
#line 2443
		ILI_ERR("Frame count is zero, which is at least set as 1\n");
		frame_count = 1;
		mp_items[index].c48 = frame_count;
	}

	if (create_mp_test_frame_buffer(index, frame_count) < 0) {
		ret = -107;
		goto out;
	}

	if (cap_dac == NULL) {
		cap_dac = kcalloc(core_mp.c256, sizeof(int),
					  GFP_KERNEL);
		if (IS_ERR(cap_dac) || cap_dac == NULL) {
#line 2455
			ILI_ERR("Failed to allocate cap_dac buffer\n");
			return -107;
		}
	} else {
		memset(cap_dac, 0, core_mp.c256);
	}

	if (cap_raw == NULL) {
		cap_raw = kcalloc(core_mp.c256, sizeof(int),
					  GFP_KERNEL);
		if (IS_ERR(cap_raw) || cap_raw == NULL) {
#line 2465
			ILI_ERR("Failed to allocate cap_raw buffer\n");
			ipio_kfree((void **)&cap_dac);
			return -107;
		}
	} else {
		memset(cap_raw, 0, core_mp.c256);
	}

	for (i = 0; i < core_mp.c236; i++) {
		for (j = 0; j < core_mp.c232; j++) {
			mp_items[index].max_buf[i * core_mp.c232 + j] = INT_MIN;
			mp_items[index].min_buf[i * core_mp.c232 + j] = INT_MAX;
		}
	}

	if (mp_items[index].c21 == 1) {
		parser_ini_benchmark(mp_items[index].bench_mark_max,
				     mp_items[index].bench_mark_min,
				     mp_items[index].c22, mp_items[index].c0,
				     core_mp.c256);
		dump_benchmark_data(mp_items[index].bench_mark_max,
				    mp_items[index].bench_mark_min);
	}

	parser_get_int_data(mp_items[index].c0, "gain", str, 512);
	open_para.gain = katoi(str);
	parser_get_int_data(mp_items[index].c0, "tvch", str, 512);
	open_para.tvch = katoi(str);
	ret = parser_get_int_data(mp_items[index].c0, "tvcl", str, 512);
	open_para.tvcl = katoi(str);
	if (ret < 0) {
#line 2500
		ILI_ERR("Failed to get parameters from ini file\n");
		ret = -106;
		goto out;
	}

#line 2506
	ILI_DBG("open_test_c: frame_cont = %d, gain = %d, tvch = %d, tvcl = %d\n", mp_items[index].c48, open_para.gain, open_para.tvch, open_para.tvcl);

	for (i = 0; i < mp_items[index].c48; i++) {
		cdc[i][0] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][1] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		cdc[i][2] = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
	}

	for (i = 0; i < mp_items[index].c48; i++) {
		ret = allnode_open_cdc_data(4, cdc[i][0]);
		if (ret < 0) {
#line 2517
			ILI_ERR("Failed to get Open CAP DAC data, %d\n", ret);
			goto out;
		}
		ret = allnode_open_cdc_data(5, cdc[i][1]);
		if (ret < 0) {
#line 2522
			ILI_ERR("Failed to get Open CAP RAW data, %d\n", ret);
			goto out;
		}
		allnode_open_cdc_result(index, cdc[i][2], cdc[i][0],
					cdc[i][1]);

		if (i == 0) {
			memcpy(cap_dac, cdc[i][0],
			       core_mp.c256 * 4);
			memcpy(cap_raw, cdc[i][1],
			       core_mp.c256 * 4);
		}

		ilitek_dump_data(cdc[i][2], 10, core_mp.c256, core_mp.c232,
				 "DCL_Cap");

		for (j = 0, k = 0; j < core_mp.c236; j++) {
			int m;

			for (m = 0; m < core_mp.c232; m++) {
				mp_items[index].buf[i * core_mp.c256 + k] =
					cdc[i][2][k];
				k++;
			}
		}

		compare_MaxMin_result(index,
				      &mp_items[index].buf[i * core_mp.c256]);
	}

out:
	for (i = 0; i < mp_items[index].c48; i++) {
		ipio_kfree((void **)&cdc[i][0]);
		ipio_kfree((void **)&cdc[i][1]);
		ipio_kfree((void **)&cdc[i][2]);
	}
	return ret;
}

/*
 * ===========================================================================
 * allnode_key_cdc_data -- INCORPORATA in `key_test`, righe 1410..1484
 * ===========================================================================
 * NON E' UN SIMBOLO: `grep -c " allnode_key_cdc_data$" oracolo/stock.map`
 * stampa 0. Che esista come funzione separata nel sorgente di fabbrica lo
 * dice il `__func__`: le sette `printk` di questo blocco portano
 * "allnode_key_cdc_data"@0xffffff8009244769 mentre le tre che le circondano
 * -- righe 2561, 2564 e 2576 -- portano "key_test"@0xffffff80092446f1. E' la
 * classe di difetto B5 usata al contrario: il `__func__` che NON corrisponde
 * al nome del simbolo e' la prova che la funzione c'e' ed e' stata
 * incorporata.
 *
 * UN SOLO PARAMETRO, `index`, ed e' misurato: dentro il blocco l'unico valore
 * che viene dal chiamante e' la base dell'elemento di collaudo
 * ("39400268 ldrb"@0xffffff8008a64a88, x19 = `&mp_items[index].c20`, che
 * "91005153 add"@0xffffff8008a64a24 calcola una volta sola fuori dal ciclo).
 * Il buffer di destinazione NON e' un parametro: e' il globale
 * `mp_frame_buf1`, che il blocco rilegge dalla memoria a ogni uso --
 * "f9421720 ldr"@0xffffff8008a64bb4, "f942172c ldr"@0xffffff8008a64c1c,
 * "f9421720 ldr"@0xffffff8008a64c4c -- e che il ramo d'allocazione SCRIVE
 * ("f9021720 str"@0xffffff8008a64cd8). Un parametro starebbe in un registro.
 *
 * IL VALORE RESO E' `int` ed e' provato come negativo dal chiamante
 * ("37f80936 tbnz"@0xffffff8008a64d18, bit 31).
 *
 * QUATTRO DIFFERENZE MISURATE RISPETTO ALLA GEMELLA `allnode_open_cdc_data`,
 * che ha la stessa ossatura ma non lo stesso codice:
 *
 *  1. il comando e' costruito qui, non letto dal file .ini: `cmd[0] = 0xF1`
 *     ("52801e2a mov"@0xffffff8008a64a90 + "390093ea strb"@0xffffff8008a64a94)
 *     e `cmd[1] = mp_items[index].c20`
 *     ("39400268 ldrb"@0xffffff8008a64a88 + "390097e8 strb"@0xffffff8008a64a98);
 *  2. i codici d'errore NON sono i numeri fissi della gemella: qui i tre
 *     rami che seguono una chiamata rendono il valore RESO DALLA CHIAMATA
 *     ("2a0003f6 mov"@0xffffff8008a64b90 dopo la lettura,
 *     "2a0003f6 mov"@0xffffff8008a64ad8 e "2a0003f6 mov"@0xffffff8008a64ae8
 *     dopo il controllo d'occupato). L'unico numero fisso e' il -1 del
 *     controllo di lunghezza ("12800016 mov"@0xffffff8008a64dc8);
 *  3. la lunghezza viene da `core_mp.key_len`, non dal prodotto righe per
 *     colonne: "b94bfaa8 ldr"@0xffffff8008a64a10 (0xffffff800a0fcbf8) e
 *     "531f791b lsl"@0xffffff8008a64a30, cioe' `key_len * 2`;
 *  4. il buffer di trama e' allocato QUI e non da
 *     `create_mp_test_frame_buffer`.
 *
 * TRE DIFETTI DELLA FABBRICA, RIPRODOTTI (regola 7):
 *
 *  D1. IL RAMO DI ALLOCAZIONE FALLITA DI `ori` NON TOCCA `ret`. Sul cammino
 *      "5280b782 mov"@0xffffff8008a64c70 (riga 1468) l'ultima scrittura di
 *      w22 e' quella del controllo d'occupato, che vale >= 0, e il chiamante
 *      alla prova "37f80936 tbnz"@0xffffff8008a64d18 NON entra nel ramo
 *      d'errore: il collaudo prosegue come se la lettura fosse riuscita. La
 *      gemella `allnode_open_cdc_data` in quel punto scrive -107.
 *  D2. L'AZZERAMENTO DEL BUFFER COPRE `key_len` BYTE, NON `key_len * 4`:
 *      "b98bfaa2 ldrsw"@0xffffff8008a64bbc passa il NUMERO DI ELEMENTI come
 *      terzo argomento di `__memset` ("940fcd0f bl"@0xffffff8008a64bc4),
 *      mentre l'allocazione dello stesso vettore moltiplica per quattro
 *      ("d37ef500 lsl"@0xffffff8008a64cc4). E' lo stesso difetto gia' trovato
 *      su `mp_frame_buf0` dentro `mutual_test`.
 *  D3. IL CICLO DI COMBINAZIONE CONTA FINO A `core_mp.c256` (il numero di
 *      nodi, "b94c0362 ldr"@0xffffff8008a64bcc) mentre il vettore e' stato
 *      dimensionato su `core_mp.key_len` ("b98bfaa8 ldrsw"@0xffffff8008a64cb4).
 *      I due non sono lo stesso numero, e il blocco scrive fuori dal vettore
 *      appena `c256 > key_len`.
 *
 * UN FATTO CHE VA DETTO PERCHE' SORPRENDE: la guardia del corpo del ciclo e'
 * `mp_items[index].c20 == 20` ("3940026b ldrb"@0xffffff8008a64be4 +
 * "7100517f cmp"@0xffffff8008a64be8, 0x14 = 20), cioe' lo STESSO byte che
 * finisce in `cmd[1]`. Nessuno dei cinque elementi di collaudo il cui nome
 * comincia per "key " ha `c20 = 20` nella tavola (valgono 22, 7, 21, 18, 19):
 * per tutti loro il corpo del ciclo non parte e `mp_frame_buf1` resta com'e'.
 * QUESTA E' UNA LETTURA DELLA TAVOLA, NON UNA MISURA DEL COMPORTAMENTO: il
 * campo e' scritto anche a tempo d'esecuzione da `ilitek_tddi_mp_init_item`,
 * e questo lotto non ha seguito quel cammino.
 *
 * `u8 cmd[3]`: la larghezza e' MISURATA dalle due istruzioni che lo azzerano
 * in cima a ogni chiamata -- "79004bff strh"@0xffffff8008a64a38 (due byte) e
 * "39009bff strb"@0xffffff8008a64a34 (il terzo) -- e dal terzo argomento
 * della prima scrittura sul chip ("320007e1 orr"@0xffffff8008a64ab0, w1 = 3).
 */
static int allnode_key_cdc_data(int index)
{
	int i = 0, idx = 0, ret = 0, len = 0;
	u8 cmd[3] = {0};
	u8 *ori = NULL;

	/* "b94bfaa8 ldr"@0xffffff8008a64a10 (core_mp.key_len, 0xffffff800a0fcbf8)
	 * e "531f791b lsl"@0xffffff8008a64a30 (per due). La lettura sta in cima
	 * al corpo del ciclo del chiamante -- "b94bfaa8 ldr"@0xffffff8008a64d1c
	 * la rifa' a ogni giro -- quindi nel sorgente e' dentro la funzione, non
	 * estratta prima del ciclo (classe A2 al contrario) */
	len = core_mp.key_len * 2;

	/* "\x016ILITEK: (%s, %d): Read key's length = %d\n"@0xffffff800924473d,
	 * __func__ = "allnode_key_cdc_data"@0xffffff8009244769; la guardia e'
	 * "396562e9 ldrb"@0xffffff8008a64a2c + "34000229 cbz"@0xffffff8008a64a3c;
	 * "5280b042 mov"@0xffffff8008a64a48 -- __LINE__ = 1410 */
#line 1410
	ILI_DBG("Read key's length = %d\n", len);
	/* "\x016ILITEK: (%s, %d): core_mp.key_len = %d\n"@0xffffff800924477e --
	 * e' il formato che NOMINA il campo, cioe' l'eccezione alla regola 5.
	 * L'argomento e' "b94bfaa3 ldr"@0xffffff8008a64a64, una RILETTURA del
	 * campo; la seconda guardia e' "396562e8 ldrb"@0xffffff8008a64a5c +
	 * "34000108 cbz"@0xffffff8008a64a60;
	 * "5280b062 mov"@0xffffff8008a64a70 -- __LINE__ = 1411 */
#line 1411
	ILI_DBG("core_mp.key_len = %d\n", core_mp.key_len);

	/* "7100037f cmp"@0xffffff8008a64a80 + "5400196d b.le"@0xffffff8008a64a84:
	 * il confronto e' CON SEGNO e con zero, non con 2 come nella gemella */
	if (len <= 0) {
		/* "\x013ILITEK: (%s, %d): Length is invalid\n"@0xffffff8009243e9d;
		 * "5280b0c2 mov"@0xffffff8008a64dc0 -- __LINE__ = 1414 */
#line 1414
		ILI_ERR("Length is invalid\n");
		/* "12800016 mov"@0xffffff8008a64dc8 */
		ret = -1;
		goto out;
	}

	/* "52801e2a mov"@0xffffff8008a64a90 (0xf1) +
	 * "390093ea strb"@0xffffff8008a64a94 */
	cmd[0] = 0xF1;
	/* "39400268 ldrb"@0xffffff8008a64a88 (mp_items[index].c20, un byte) +
	 * "390097e8 strb"@0xffffff8008a64a98 */
	cmd[1] = mp_items[index].c20;
	/* "39009bff strb"@0xffffff8008a64aa0 */
	cmd[2] = 0x00;

	/* "320003e8 orr"@0xffffff8008a64a9c + "b902dd28 str"@0xffffff8008a64aa4 */
	idev->c732 = 1;

	/* "f9418528 ldr"@0xffffff8008a64aa8 (idev->c776),
	 * "320007e1 orr"@0xffffff8008a64ab0 (w1 = 3) e
	 * "d63f0100 blr"@0xffffff8008a64ab4 */
	ret = idev->c776(cmd, sizeof(cmd));
	if (ret < 0) {
		/* "\x013ILITEK: (%s, %d): Write CDC command failed\n"
		 * @0xffffff8009243f06; "5280b282 mov"@0xffffff8008a64de4 --
		 * __LINE__ = 1428. Il valore reso e' quello della chiamata:
		 * "2a0003f6 mov"@0xffffff8008a64dd0 */
#line 1428
		ILI_ERR("Write CDC command failed\n");
		goto out;
	}

	/* "b94c6508 ldr"@0xffffff8008a64ac0 (core_mp.c356, 0xffffff800a0fcc64) +
	 * "7100051f cmp"@0xffffff8008a64ac4 + "540000e1 b.ne"@0xffffff8008a64ac8 */
	if (core_mp.c356 == 1)
		/* "52800640 mov"@0xffffff8008a64acc + "52800641 mov"@0xffffff8008a64ad0
		 * (w0 = w1 = 50) + "97ffd280 bl"@0xffffff8008a64ad4 */
		ret = ilitek_tddi_ic_check_busy(50, 50);
	else
		/* "97ffd239 bl"@0xffffff8008a64ae4, nessun argomento */
		ret = ilitek_tddi_ic_check_int_stat();

	/* "36f800a0 tbz"@0xffffff8008a64adc e "37f81a40 tbnz"@0xffffff8008a64aec.
	 * NON c'e' nessuna riscrittura di `ret` qui: la gemella
	 * `allnode_open_cdc_data` scrive -110, questa rende il valore ricevuto */
	if (ret < 0)
		goto out;

	/* "b94b1108 ldr"@0xffffff8008a64af4 (core_mp.c16, 0xffffff800a0fcb10),
	 * "52808029 mov"@0xffffff8008a64af8 + "72a00029 movk"@0xffffff8008a64afc
	 * (0x10401), "6b09011f cmp"@0xffffff8008a64b00 +
	 * "540002c8 b.hi"@0xffffff8008a64b04 (confronto SENZA segno) */
	if ((unsigned int)core_mp.c16 <= 0x10401U) {
		/* "529e5ec9 mov"@0xffffff8008a64b0c (0xf2f6) +
		 * "79004be9 strh"@0xffffff8008a64b10: DUE byte in una volta */
		cmd[0] = 0xF6;
		cmd[1] = 0xF2;
		/* "321f03e1 orr"@0xffffff8008a64b1c -- lunghezza 2 */
		ret = idev->c776(cmd, 2);
		if (ret < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x, 0x%x) error\n"
			 * @0xffffff8009243f34, con
			 * "394093e3 ldrb"@0xffffff8008a64df0 e
			 * "394097e4 ldrb"@0xffffff8008a64df4;
			 * "5280b502 mov"@0xffffff8008a64e0c -- __LINE__ = 1448 */
#line 1448
			ILI_ERR("Write (0x%x, 0x%x) error\n", cmd[0], cmd[1]);
			goto out;
		}
		/* "52912b00 mov"@0xffffff8008a64b28 +
		 * "72a00820 movk"@0xffffff8008a64b2c = 0x418958 = 1000 * 0x10c7 */
		mdelay(1);

		/* "b2400128 orr"@0xffffff8008a64a08 mette a uno il bit 0 del
		 * puntatore -- `sp+36` e' allineato a quattro, quindi e'
		 * `&cmd[1]` -- e "f9000be8 str"@0xffffff8008a64a0c lo salva
		 * fuori dal ciclo; "f9400be0 ldr"@0xffffff8008a64b38 lo rilegge */
		ret = idev->c776(&cmd[1], 1);
		if (ret < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x) error\n"
			 * @0xffffff8009243f62, con
			 * "394097e3 ldrb"@0xffffff8008a64e18;
			 * "5280b622 mov"@0xffffff8008a64e2c -- __LINE__ = 1457 */
#line 1457
			ILI_ERR("Write (0x%x) error\n", cmd[1]);
			goto out;
		}
		mdelay(1);
	}

	/* "93407f60 sxtw"@0xffffff8008a64b60 (la lunghezza e' un `int` con
	 * segno), "52901801 mov"@0xffffff8008a64b5c +
	 * "72a02801 movk"@0xffffff8008a64b64 = 0x14080c0 = GFP_KERNEL |
	 * __GFP_ZERO, "97dfae08 bl"@0xffffff8008a64b68 verso <__kmalloc> */
	ori = kzalloc(len, GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a64b70 + "540007a8 b.hi"@0xffffff8008a64b74
	 * (IS_ERR) e "b4000797 cbz"@0xffffff8008a64b78 (NULL) */
	if (IS_ERR(ori) || ori == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate ori mem (%ld)\n"
		 * @0xffffff80092447a8, con "aa1703e3 mov"@0xffffff8008a64c7c;
		 * "5280b782 mov"@0xffffff8008a64c70 -- __LINE__ = 1468.
		 * DIFETTO D1 DELLA FABBRICA: `ret` NON e' toccato qui */
#line 1468
		ILI_ERR("Failed to allocate ori mem (%ld)\n", PTR_ERR(ori));
		goto out;
	}

	/* "f9418908 ldr"@0xffffff8008a64b88 (idev->c784) +
	 * "d63f0100 blr"@0xffffff8008a64b8c, w1 = len
	 * ("2a1b03e1 mov"@0xffffff8008a64b84) */
	ret = idev->c784(ori, len);
	if (ret < 0) {
		/* "\x013ILITEK: (%s, %d): Read cdc data error, len = %d\n"
		 * @0xffffff8009243fbd; "5280b862 mov"@0xffffff8008a64c9c --
		 * __LINE__ = 1475 */
#line 1475
		ILI_ERR("Read cdc data error, len = %d\n", len);
		goto out;
	}

	/* "321d03e1 orr"@0xffffff8008a64b9c (w1 = 8),
	 * "911f7884 add"@0xffffff8008a64bac ("Key CDC original"
	 * @0xffffff80092447de) e "97ffd7ca bl"@0xffffff8008a64bb0 */
	ilitek_dump_data(ori, 8, len, 0, "Key CDC original");

	/* "f9421720 ldr"@0xffffff8008a64bb4 + "b40007e0 cbz"@0xffffff8008a64bb8 */
	if (mp_frame_buf1 == NULL) {
		/* la forma di `kcalloc`: "b98bfaa8 ldrsw"@0xffffff8008a64cb4
		 * (CON SEGNO), "37f80168 tbnz"@0xffffff8008a64cbc (il controllo
		 * di traboccamento di `kmalloc_array`),
		 * "d37ef500 lsl"@0xffffff8008a64cc4 (per quattro),
		 * "97dfadaf bl"@0xffffff8008a64ccc verso <__kmalloc> e
		 * "f9021720 str"@0xffffff8008a64cd8 */
		mp_frame_buf1 = kcalloc(core_mp.key_len, sizeof(int),
					GFP_KERNEL);
		/* "b140041f cmn"@0xffffff8008a64cd4 +
		 * "540000a8 b.hi"@0xffffff8008a64cdc e
		 * "b5fff763 cbnz"@0xffffff8008a64ce0 */
		if (IS_ERR(mp_frame_buf1) || mp_frame_buf1 == NULL) {
			/* "\x013ILITEK: (%s, %d): Failed to allocate FrameBuffer mem (%ld)\n"
			 * @0xffffff8009244004; "5280b982 mov"@0xffffff8008a64cf8
			 * -- __LINE__ = 1484. Anche qui `ret` NON e' toccato */
#line 1484
			ILI_ERR("Failed to allocate FrameBuffer mem (%ld)\n", PTR_ERR(mp_frame_buf1));
			goto out;
		}
	} else {
		/* DIFETTO D2: "b98bfaa2 ldrsw"@0xffffff8008a64bbc passa il
		 * numero di ELEMENTI come numero di BYTE */
		memset(mp_frame_buf1, 0, core_mp.key_len);
	}

	/* DIFETTO D3: la guardia e' `core_mp.c256`
	 * ("b94c0362 ldr"@0xffffff8008a64bcc, rilette in fondo al giro con
	 * "b94c0362 ldr"@0xffffff8008a64c30), non `core_mp.key_len`.
	 *
	 * LE DUE CATENE D'INDICE SONO DISTINTE, come nella gemella: x9 parte da
	 * 0x100000000 ("b26003e9 orr"@0xffffff8008a64bdc) e x10 da 0x200000000
	 * ("b25f03ea orr"@0xffffff8008a64be0), tutt'e due con passo 0x200000000
	 * ("8b140129 add"@0xffffff8008a64c38 e "8b14014a add"@0xffffff8008a64c40),
	 * e i due indici escono da "9360fd2b asr"@0xffffff8008a64bf0 e
	 * "9360fd4c asr"@0xffffff8008a64bf8 */
	for (i = 0; i < core_mp.c256; i++) {
		idx = (2 * i) + 1;

		/* "3940026b ldrb"@0xffffff8008a64be4 +
		 * "7100517f cmp"@0xffffff8008a64be8 (0x14 = 20) +
		 * "54000241 b.ne"@0xffffff8008a64bec */
		if (mp_items[index].c20 == 20)
			/* "1200196d and"@0xffffff8008a64c00 +
			 * "7219017f tst"@0xffffff8008a64c04 +
			 * "5a8d05ad cneg"@0xffffff8008a64c0c per il primo byte
			 * e "1200198b and"@0xffffff8008a64c08 +
			 * "7219019f tst"@0xffffff8008a64c10 +
			 * "5a8b056b cneg"@0xffffff8008a64c14 per il secondo;
			 * la divisione per due CON SEGNO e'
			 * "7100017f cmp"@0xffffff8008a64c20 +
			 * "1a8ba56b cinc"@0xffffff8008a64c24 +
			 * "13017d6b asr"@0xffffff8008a64c28, e la `str` finale
			 * e' "b828798b str"@0xffffff8008a64c2c */
			mp_frame_buf1[i] = (((ori[idx] & 0x80) ? -(ori[idx] & 0x7F) : (ori[idx] & 0x7F)) +
					    ((ori[idx + 1] & 0x80) ? -(ori[idx + 1] & 0x7F) : (ori[idx + 1] & 0x7F))) / 2;
	}

	/* "321b03e1 orr"@0xffffff8008a64c58 (w1 = 32),
	 * "b94be903 ldr"@0xffffff8008a64c50 (core_mp.c232) e
	 * "911fbc84 add"@0xffffff8008a64c5c ("Key CDC combined data"
	 * @0xffffff80092447ef) */
	ilitek_dump_data(mp_frame_buf1, 32, core_mp.c256, core_mp.c232,
			 "Key CDC combined data");

out:
	/* "b902dd1f str"@0xffffff8008a64c88, "b902dd1f str"@0xffffff8008a64d0c e
	 * "b902dd1f str"@0xffffff8008a64e38: TRE copie, una per gruppo di
	 * cammini */
	idev->c732 = 0;
	/* "b5000437 cbnz"@0xffffff8008a64c8c + "97dfaea7 bl"@0xffffff8008a64d14
	 * verso <kfree>. Sui cammini in cui `ori` vale ancora NULL la
	 * liberazione non compare affatto */
	ipio_kfree((void **)&ori);
	return ret;
}

/*
 * ===========================================================================
 * key_test -- 0xffffff8008a6493c, 1368 byte, `t` (342 istruzioni)
 * ===========================================================================
 * Un parametro `int`, l'indice dell'elemento di collaudo: il suo segno e'
 * MISURATO da "93407e74 sxtw"@0xffffff8008a6496c, che lo estende CON SEGNO
 * prima di moltiplicarlo per 152 ("9b082688 madd"@0xffffff8008a64988).
 *
 * `static`: nella mappa e' `t` ("ffffff8008a6493c t key_test"). L'indirizzo
 * arriva a `mp_items[i].c144` dal caso 2 di `ilitek_tddi_mp_init_item`
 * (voce 0x05 della tavola di salto).
 *
 * PORTA DENTRO DI SE' `allnode_key_cdc_data` (righe 1410..1484), incorporata:
 * la prova e' il `__func__` -- vedi il cappello di quella funzione.
 *
 * IL CICLO E' UNO SOLO, e non due come in `open_test_cap`: il corpo copia il
 * buffer di trama nel buffer dell'elemento e il confronto col massimo e col
 * minimo si fa UNA SOLA VOLTA, dopo il ciclo
 * ("940002b8 bl"@0xffffff8008a64d88 verso <compare_MaxMin_result>, fuori dal
 * ramo di ritorno del ciclo "54ffe60b b.lt"@0xffffff8008a64d6c).
 *
 * DUE DIFETTI DELLA FABBRICA, RIPRODOTTI (regola 7), oltre ai tre di
 * `allnode_key_cdc_data`:
 *
 *  D4. LA COPIA NON E' INDICIZZATA PER TRAMA. L'indice di destinazione e'
 *      "b82a6968 str"@0xffffff8008a64d44, cioe' `buf[j]` con x10 = j * 4
 *      ("d37ef52a lsl"@0xffffff8008a64d34) e nessun contributo del contatore
 *      di trama (che sta in w28, "1100079c add"@0xffffff8008a64d60): ogni
 *      giro riscrive gli stessi elementi. `open_test_cap` e `open_test_sp`
 *      scrivono invece a `i * core_mp.c256 + k`.
 *  D5. IL CONTROLLO SUL NUMERO DI TRAME E' `== 0`, NON `< 1`:
 *      "34001e61 cbz"@0xffffff8008a649c4. Con un conteggio NEGATIVO nel file
 *      .ini il controllo non scatta, `create_mp_test_frame_buffer` viene
 *      chiamata lo stesso e il ciclo non parte. `open_test_cap` in quel punto
 *      confronta con 1 e CORREGGE il campo a 1; qui non lo corregge.
 *
 * IL VALORE D'ERRORE DEL CONTEGGIO NULLO E' -22
 * ("128002b6 mov"@0xffffff8008a64da8), che coincide con -EINVAL; resta
 * scritto come numero perche' il binario non nomina niente (regola 5).
 */
static int key_test(int index)
{
	int i = 0, j = 0, ret = 0;

	/* "\x016ILITEK: (%s, %d): Item = %s, Frame Count = %d\n"
	 * @0xffffff80092446c0, __func__ = "key_test"@0xffffff80092446f1; la
	 * guardia e' "396562e8 ldrb"@0xffffff8008a64974 +
	 * "340001a8 cbz"@0xffffff8008a64978; i due argomenti sono
	 * "f9400103 ldr"@0xffffff8008a6498c (il nome, campo a +0) e
	 * "b9403104 ldr"@0xffffff8008a64990 (il conteggio, campo a +48);
	 * "52814022 mov"@0xffffff8008a649a4 -- __LINE__ = 2561 */
#line 2561
	ILI_DBG("Item = %s, Frame Count = %d\n", mp_items[index].c0, mp_items[index].c48);

	/* DIFETTO D5: "b8430d01 ldr"@0xffffff8008a649bc (pre-indice, x8 diventa
	 * `&mp_items[index].c48`) + "34001e61 cbz"@0xffffff8008a649c4 */
	if (mp_items[index].c48 == 0) {
		/* "\x013ILITEK: (%s, %d): Frame count is zero, which at least sets as 1\n"
		 * @0xffffff80092446fa; "52814082 mov"@0xffffff8008a64da0 --
		 * __LINE__ = 2564 */
#line 2564
		ILI_ERR("Frame count is zero, which at least sets as 1\n");
		/* "128002b6 mov"@0xffffff8008a64da8 */
		ret = -22;
		goto out;
	}

	/* "94000201 bl"@0xffffff8008a649cc. Il secondo argomento e' la STESSA
	 * lettura del campo a +48 che ha appena servito il `cbz`: w1 non e'
	 * ricaricato fra "b8430d01 ldr"@0xffffff8008a649bc e la chiamata.
	 * "2a0003f6 mov"@0xffffff8008a649d0 + "37f82420 tbnz"@0xffffff8008a649d4 */
	ret = create_mp_test_frame_buffer(index, mp_items[index].c48);
	if (ret < 0)
		goto out;

	/* "b9400108 ldr"@0xffffff8008a649e0 + "7100051f cmp"@0xffffff8008a649e4 +
	 * "54001c4b b.lt"@0xffffff8008a649e8 in testa, e in coda
	 * "f9400fe9 ldr"@0xffffff8008a64d5c + "b9400129 ldr"@0xffffff8008a64d64 +
	 * "6b09039f cmp"@0xffffff8008a64d68: il campo e' RILETTO a ogni giro */
	for (i = 0; i < mp_items[index].c48; i++) {
		ret = allnode_key_cdc_data(index);
		if (ret < 0) {
			/* "\x013ILITEK: (%s, %d): Failed to initialise CDC data, %d\n"
			 * @0xffffff8009243ad5, con
			 * "2a1603e3 mov"@0xffffff8008a64e50;
			 * "52814202 mov"@0xffffff8008a64e4c -- __LINE__ = 2576 */
#line 2576
			ILI_ERR("Failed to initialise CDC data, %d\n", ret);
			goto out;
		}

		/* DIFETTO D4: nessun contributo del contatore di trama.
		 * "b94bfaa8 ldr"@0xffffff8008a64d1c + "7100051f cmp"@0xffffff8008a64d20
		 * in testa e "b98bfaa8 ldrsw"@0xffffff8008a64d48 in coda: la
		 * guardia rilegge `core_mp.key_len`. I due puntatori sono
		 * riletti anch'essi a ogni giro --
		 * "f9421728 ldr"@0xffffff8008a64d30 (mp_frame_buf1) e
		 * "f940030b ldr"@0xffffff8008a64d38 (il campo a +96) */
		for (j = 0; j < core_mp.key_len; j++)
			mp_items[index].buf[j] = mp_frame_buf1[j];
	}

	/* "f9403101 ldr"@0xffffff8008a64d84 (il campo a +96) +
	 * "940002b8 bl"@0xffffff8008a64d88 */
	compare_MaxMin_result(index, mp_items[index].buf);

out:
	return ret;
}

/*
 * ===========================================================================
 * pin_test -- 0xffffff8008a64eec, 740 byte, `t` (185 istruzioni)
 * ===========================================================================
 * Un parametro `int`, l'indice dell'elemento: il segno e' MISURATO da
 * "9b285679 smaddl"@0xffffff8008a64f48, che estende w19 CON SEGNO prima di
 * moltiplicarlo per 152, e da "93407e76 sxtw"@0xffffff8008a64f60.
 *
 * `static`: nella mappa e' `t` ("ffffff8008a64eec t pin_test"). L'indirizzo
 * arriva a `mp_items[i].c144` dal caso 10 di `ilitek_tddi_mp_init_item`
 * (voce 0x1e della tavola di salto).
 *
 * NESSUNA FUNZIONE INCORPORATA: tutte e sette le `printk` del blocco portano
 * lo stesso `__func__`, "pin_test"@0xffffff80092448b3.
 *
 * E' LA FUNZIONE CHE NOMINA TRE CAMPI DI `struct ilitek_mp_item` -- e' su di
 * essa che poggia l'eccezione alla regola 5 gia' scritta sulla struttura:
 * "39415723 ldrb"@0xffffff8008a64f4c con il formato
 * "\x016ILITEK: (%s, %d): test_int_pin = 0x%x\n"@0xffffff80092448bc,
 * "38456f03 ldrb"@0xffffff8008a64f6c con
 * "\x016ILITEK: (%s, %d): int_pulse_test = 0x%x\n"@0xffffff80092448e5 e
 * "38454ee3 ldrb"@0xffffff8008a64f88 con
 * "\x016ILITEK: (%s, %d): delay_time = 0x%x\n"@0xffffff8009244910. Tutti e
 * tre sono letti con `ldrb`: sono `u8`.
 *
 * LE SETTE `printk` NON SONO PROTETTE dal byte diagnostico -- nessuna
 * `ldrb ilitek_dbg_en` compare in tutta la funzione -- e quattro di esse
 * portano il livello KERN_INFO: sono `ILI_INFO`, non `ILI_DBG`.
 *
 * TRE DIFETTI DELLA FABBRICA, RIPRODOTTI (regola 7):
 *
 *  D6. IL PRIMO MESSAGGIO NON HA IL FINE RIGA. La stringa a
 *      0xffffff8009244890 e' lunga 34 byte contati dai byte
 *      ("\x016ILITEK: (%s, %d): PIN test start", senza `\n`), e la riga
 *      seguente del registro le si attacca dietro.
 *  D7. "defualt" NEL MESSAGGIO FINALE, per "default":
 *      "\x016ILITEK: (%s, %d): Change to defualt IRQ trigger type\n"
 *      @0xffffff80092449c2. E' cosi' nei byte e si riproduce.
 *  D8. `cmd[2]` -- il ritardo -- E' SCRITTO SOLO NEL SECONDO BLOCCO
 *      ("39000be8 strb"@0xffffff8008a6504c), e le due scritture di TRE byte
 *      sul chip ("320007e1 orr"@0xffffff8008a65070 e
 *      "320007e1 orr"@0xffffff8008a650c0) leggono tutt'e due lo stesso
 *      valore: la prova "a impulso" e quella "a fronte di discesa" mandano
 *      byte identici. Il ramo del primo blocco, invece, scrive DUE byte
 *      ("321f03e1 orr"@0xffffff8008a64fd8 e "321f03e1 orr"@0xffffff8008a64ffc).
 *
 * IL VETTORE DI COMANDO E' LARGO CINQUE BYTE, ed e' MISURATO dalle due
 * istruzioni che lo azzerano: "b90003ff str"@0xffffff8008a64f34 (quattro
 * byte a `sp+0`) e "390013ff strb"@0xffffff8008a64f30 (il quinto, a `sp+4`).
 * Il canarino sta a `sp+8`, quindi il vettore non puo' essere piu' lungo di
 * otto; se fosse largo otto l'azzeramento sarebbe una sola `str xzr`, se sei
 * o sette ci sarebbe una `strh`. Sono cinque, e il codice ne usa tre.
 *
 * I TRE CODICI D'ERRORE. -100 per la scrittura sul chip fallita
 * ("12800c73 mov"@0xffffff8008a65140), -109 per tutto il resto
 * ("12800d93 mov"@0xffffff8008a650e8). Nessuno dei due coincide con un errno
 * di Linux e restano numeri (regola 5).
 *
 * L'ESITO NEL CAMPO `c36`. La `str` e' UNA SOLA
 * ("b9002528 str"@0xffffff8008a65164) e il valore le arriva da TRE
 * predecessori: "2a1f03e8 mov"@0xffffff8008a650e0 e
 * "2a1f03e8 mov"@0xffffff8008a650f0 (zero) e
 * "12800008 mov"@0xffffff8008a65144 (meno uno). Non c'e' nessuna `csel` e
 * nessuna `asr #31`: il compilatore ha propagato la costante lungo ciascun
 * cammino, perche' su ognuno il segno di `ret` e' gia' deciso. La forma
 * scritta qui e' il confronto, ed e' quella che riproduce le tre `mov`.
 *
 * IL TIPO DI INTERRUZIONE 1 E 2 sono numeri, non nomi: il binario passa
 * "320003e0 orr"@0xffffff8008a65054 e "321f03e0 orr"@0xffffff8008a650a8 a
 * `ilitek_plat_irq_register`, e alla fine rimette quello salvato in
 * `idev->c340` ("b9415500 ldr"@0xffffff8008a65178). Nessuna stringa li nomina.
 */
static int pin_test(int index)
{
	int ret = 0;
	u8 cmd[5] = {0};

	/* "\x016ILITEK: (%s, %d): PIN test start"@0xffffff8009244890 -- SENZA
	 * fine riga (difetto D6), __func__ = "pin_test"@0xffffff80092448b3;
	 * "5280fe82 mov"@0xffffff8008a64f24 -- __LINE__ = 2036. Nessuna
	 * guardia: e' `ILI_INFO` */
#line 2036
	ILI_INFO("PIN test start");
	/* "39415723 ldrb"@0xffffff8008a64f4c (+0x55) con
	 * "5280fea2 mov"@0xffffff8008a64f58 -- __LINE__ = 2037 */
#line 2037
	ILI_INFO("test_int_pin = 0x%x\n", mp_items[index].test_int_pin);
	/* "38456f03 ldrb"@0xffffff8008a64f6c (pre-indice, +0x56) con
	 * "5280fec2 mov"@0xffffff8008a64f78 -- __LINE__ = 2038 */
#line 2038
	ILI_INFO("int_pulse_test = 0x%x\n", mp_items[index].int_pulse_test);
	/* "38454ee3 ldrb"@0xffffff8008a64f88 (pre-indice, +0x54) con
	 * "5280fee2 mov"@0xffffff8008a64f94 -- __LINE__ = 2039 */
#line 2039
	ILI_INFO("delay_time = 0x%x\n", mp_items[index].delay_time);

	/* "39415728 ldrb"@0xffffff8008a64fa0 (una RILETTURA del campo) +
	 * "7100051f cmp"@0xffffff8008a64fa8 + "54000381 b.ne"@0xffffff8008a64fac */
	if (mp_items[index].test_int_pin == 1) {
		/* "39405108 ldrb"@0xffffff8008a64fb8 (+0x14) +
		 * "390003e8 strb"@0xffffff8008a64fc8 */
		cmd[0] = mp_items[index].c20;
		/* "320003ea orr"@0xffffff8008a64fc4 +
		 * "390007ea strb"@0xffffff8008a64fcc */
		cmd[1] = 0x1;

		/* "321f03e1 orr"@0xffffff8008a64fd8 (w1 = 2) +
		 * "d63f0100 blr"@0xffffff8008a64fdc +
		 * "37f808c0 tbnz"@0xffffff8008a64fe0 */
		if (idev->c776(cmd, 2) < 0) {
			/* "\x013ILITEK: (%s, %d): Write command failed\n"
			 * @0xffffff8009244937;
			 * "32002be2 orr"@0xffffff8008a65108 -- __LINE__ = 2047 */
#line 2047
			ILI_ERR("Write command failed\n");
			/* "12800c73 mov"@0xffffff8008a65140 */
			ret = -100;
			goto out;
		}

		/* "320003e0 orr"@0xffffff8008a64fe4 (w0 = 1) +
		 * "97ffe550 bl"@0xffffff8008a64fe8 +
		 * "37f807e0 tbnz"@0xffffff8008a64fec. L'assegnamento a `ret` e'
		 * MORTO su tutti i cammini (ogni uscita lo riscrive), e il
		 * compilatore lo toglie: qui non c'e' nessuna
		 * "mov w19, w0" */
		ret = check_int_level(true);
		if (ret < 0) {
			/* "12800d93 mov"@0xffffff8008a650e8 */
			ret = -109;
			goto out;
		}

		/* "390007ff strb"@0xffffff8008a64ff4 */
		cmd[1] = 0x0;

		/* "321f03e1 orr"@0xffffff8008a64ffc (w1 = 2) +
		 * "d63f0100 blr"@0xffffff8008a65004 +
		 * "37f80840 tbnz"@0xffffff8008a65008 */
		if (idev->c776(cmd, 2) < 0) {
			/* "528101c2 mov"@0xffffff8008a65120 -- __LINE__ = 2062 */
#line 2062
			ILI_ERR("Write command failed\n");
			ret = -100;
			goto out;
		}

		/* "2a1f03e0 mov"@0xffffff8008a6500c (w0 = 0) +
		 * "97ffe546 bl"@0xffffff8008a65010. QUI l'assegnamento e' VIVO
		 * -- "2a0003f3 mov"@0xffffff8008a65014 -- perche' il valore
		 * puo' arrivare fino al ritorno passando per il ramo
		 * `int_pulse_test != 1` */
		ret = check_int_level(false);
		if (ret < 0) {
			ret = -109;
			goto out;
		}
	}

	/* "39400308 ldrb"@0xffffff8008a6501c (+0x56, il registro pre-indicizzato
	 * di prima) + "7100051f cmp"@0xffffff8008a65020 +
	 * "54000661 b.ne"@0xffffff8008a65024 */
	if (mp_items[index].int_pulse_test == 1) {
		/* "\x016ILITEK: (%s, %d): MP IRQ Rising Trigger Test\n"
		 * @0xffffff8009244961;
		 * "52810382 mov"@0xffffff8008a65038 -- __LINE__ = 2076 */
#line 2076
		ILI_INFO("MP IRQ Rising Trigger Test\n");

		/* "321f03e9 orr"@0xffffff8008a65044 +
		 * "390007e9 strb"@0xffffff8008a65048 */
		cmd[1] = 0x2;
		/* "394002e8 ldrb"@0xffffff8008a65040 (+0x54) +
		 * "39000be8 strb"@0xffffff8008a6504c. E' l'UNICA scrittura di
		 * `cmd[2]` della funzione (difetto D8) */
		cmd[2] = mp_items[index].delay_time;

		/* "97ffc47a bl"@0xffffff8008a65050 */
		ilitek_plat_irq_unregister();
		/* "320003e0 orr"@0xffffff8008a65054 (w0 = 1) +
		 * "97ffc481 bl"@0xffffff8008a6505c +
		 * "37f80440 tbnz"@0xffffff8008a65060 */
		ret = ilitek_plat_irq_register(1);
		if (ret < 0) {
			ret = -109;
			goto out;
		}

		/* "320003f4 orr"@0xffffff8008a65058 +
		 * "b902dd14 str"@0xffffff8008a65074 */
		idev->c732 = 1;
		/* "320007e1 orr"@0xffffff8008a65070 (w1 = 3) +
		 * "d63f0100 blr"@0xffffff8008a6507c +
		 * "37f80540 tbnz"@0xffffff8008a65080 */
		if (idev->c776(cmd, 3) < 0) {
			/* "52810542 mov"@0xffffff8008a65138 -- __LINE__ = 2090 */
#line 2090
			ILI_ERR("Write command failed\n");
			ret = -100;
			goto out;
		}

		/* "97ffd0d1 bl"@0xffffff8008a65084 +
		 * "37f80300 tbnz"@0xffffff8008a65088 */
		ret = ilitek_tddi_ic_check_int_stat();
		if (ret < 0) {
			ret = -109;
			goto out;
		}

		/* "\x016ILITEK: (%s, %d): MP IRQ Falling Trigger Test\n"
		 * @0xffffff8009244991;
		 * "528106a2 mov"@0xffffff8008a6509c -- __LINE__ = 2101 */
#line 2101
		ILI_INFO("MP IRQ Falling Trigger Test\n");

		/* "97ffc465 bl"@0xffffff8008a650a4 */
		ilitek_plat_irq_unregister();
		/* "321f03e0 orr"@0xffffff8008a650a8 (w0 = 2) +
		 * "97ffc46d bl"@0xffffff8008a650ac +
		 * "37f801c0 tbnz"@0xffffff8008a650b0 */
		ret = ilitek_plat_irq_register(2);
		if (ret < 0) {
			ret = -109;
			goto out;
		}

		/* "320003e9 orr"@0xffffff8008a650b8 +
		 * "b902dd09 str"@0xffffff8008a650c4 */
		idev->c732 = 1;
		/* "320007e1 orr"@0xffffff8008a650c0 (w1 = 3) +
		 * "d63f0100 blr"@0xffffff8008a650cc +
		 * "37f80720 tbnz"@0xffffff8008a650d0 */
		if (idev->c776(cmd, 3) < 0) {
			/* "52810822 mov"@0xffffff8008a651c4 -- __LINE__ = 2113 */
#line 2113
			ILI_ERR("Write command failed\n");
			ret = -100;
			goto out;
		}

		/* "97ffd0bd bl"@0xffffff8008a650d4 +
		 * "37f80080 tbnz"@0xffffff8008a650d8 +
		 * "2a0003f3 mov"@0xffffff8008a650dc */
		ret = ilitek_tddi_ic_check_int_stat();
		if (ret < 0) {
			ret = -109;
			goto out;
		}
	}

out:
	/* "b9002528 str"@0xffffff8008a65164 con il valore propagato lungo i
	 * cammini: "2a1f03e8 mov"@0xffffff8008a650e0,
	 * "2a1f03e8 mov"@0xffffff8008a650f0 e
	 * "12800008 mov"@0xffffff8008a65144 */
	if (ret < 0)
		mp_items[index].c36 = -1;
	else
		mp_items[index].c36 = 0;

	/* "\x016ILITEK: (%s, %d): Change to defualt IRQ trigger type\n"
	 * @0xffffff80092449c2 (difetto D7);
	 * "52810a62 mov"@0xffffff8008a65160 -- __LINE__ = 2131 */
#line 2131
	ILI_INFO("Change to defualt IRQ trigger type\n");
	/* "97ffc433 bl"@0xffffff8008a6516c */
	ilitek_plat_irq_unregister();
	/* "b9415500 ldr"@0xffffff8008a65178 (idev + 340) +
	 * "97ffc439 bl"@0xffffff8008a6517c. IL RISULTATO NON E' USATO */
	ilitek_plat_irq_register(idev->c340);
	return ret;
}

/*
 * ===========================================================================
 * `ILI_ABS` e `ILI_MAX` -- I NOMI SONO SCELTI, LA FORMA E' MISURATA
 * ===========================================================================
 * Il valore assoluto di una differenza e' nel binario la coppia
 * "6b0a01ca subs"@0xffffff8008a629b0 + "5a8a554a cneg"@0xffffff8008a629b4, e
 * il massimo fra due interi e' la coppia "6b1001df cmp"@0xffffff8008a62cc4 +
 * "1a90c1cd csel"@0xffffff8008a62cc8 (condizione `gt`, quindi il PRIMO
 * operando vince a parita' rovesciata). Le due macro e la forma espansa
 * producono lo stesso codice emesso, e la misura non le distingue: i nomi
 * sono scelti da questo lotto, come `ILI_KFREE`.
 */
#define ILI_ABS(x)	((x) < 0 ? -(x) : (x))
#define ILI_MAX(a, b)	((a) > (b) ? (a) : (b))

/*
 * ===========================================================================
 * ipio_strcmp -- INCORPORATA, IL NOME E' SCELTO, LA FORMA E' MISURATA
 * ===========================================================================
 * NON E' UN SIMBOLO e nessun messaggio la nomina: e' incorporata dovunque.
 * CIO' CHE E' MISURATO e' la forma, ed e' misurata in SETTE siti di
 * `mutual_test` che portano tutti lo stesso schema -- una `strlen`, il
 * confronto della lunghezza con una costante, e solo se combacia una
 * `strncmp` con quella stessa costante:
 *
 *   ffffff8008a626e8:	940fd706 	bl	ffffff8008e58300 <__pi_strlen>
 *   ffffff8008a626ec:	f100541f 	cmp	x0, #0x15
 *   ffffff8008a626f0:	540000e1 	b.ne	ffffff8008a6270c <mutual_test+0x638>
 *   ffffff8008a626f4:	b0003f01 	adrp	x1, ffffff8009243000 <_etext+0x3b3000>
 *   ffffff8008a626f8:	528002a2 	mov	w2, #0x15                  	// #21
 *   ffffff8008a626fc:	aa1903e0 	mov	x0, x25
 *   ffffff8008a62700:	91154821 	add	x1, x1, #0x552
 *   ffffff8008a62704:	940fd720 	bl	ffffff8008e58384 <__pi_strncmp>
 *   ffffff8008a62708:	14000002 	b	ffffff8008a62710 <mutual_test+0x63c>
 *   ffffff8008a6270c:	12800000 	mov	w0, #0xffffffff            	// #-1
 *
 * CHE SIA UNA FUNZIONE CHE RENDE UN VALORE, e non due controlli scritti in
 * fila, lo dice "12800000 mov"@0xffffff8008a6270c: il -1 e' MATERIALIZZATO e
 * confluisce con il risultato della `strncmp` in un solo registro, che poi
 * "34000300 cbz"@0xffffff8008a6271c prova contro zero. Due controlli in fila
 * (`strlen(a) == strlen(b) && strncmp(...) == 0`) darebbero due salti allo
 * stesso bersaglio e nessun -1.
 *
 * IL VALORE -1 E' MISURATO; il valore reso quando le lunghezze combaciano e'
 * quello della `strncmp`, e nessun sito ne prova un valore diverso da zero:
 * che sia il ritorno della `strncmp` e non un 0/1 e' la SCELTA piu' piccola
 * che spiega il codice.
 */
static int ipio_strcmp(const char *s1, const char *s2)
{
	return (strlen(s1) != strlen(s2)) ? -1 : strncmp(s1, s2, strlen(s1));
}

/*
 * ===========================================================================
 * codeToOhm -- INCORPORATA in `mutual_test`, riga 1369
 * ===========================================================================
 * NON E' UN SIMBOLO: il nome viene dal `__func__` del suo unico messaggio,
 * "codeToOhm"@0xffffff8009244147, che accompagna
 * "\x013ILITEK: (%s, %d): code is invalid\n"@0xffffff8009244122 e
 * "5280ab22 mov"@0xffffff8008a62ed4 (__LINE__ = 1369). E' l'eccezione alla
 * regola 5: il nome sta nel binario.
 *
 * IL SECONDO PARAMETRO E' UN PUNTATORE, non due valori: il chiamante SCEGLIE
 * fra due coppie in memoria con "9a96032a csel"@0xffffff8008a62e54 e la
 * funzione legge le due mezze parole dal puntatore scelto
 * ("7940014d ldrh"@0xffffff8008a62e70 e "7940054a ldrh"@0xffffff8008a62e74).
 * Che siano `u16` e' MISURATO dalle due `ldrh`.
 *
 * LA COSTANTE 1500/930 E' SCELTA DA DUE CAMPI DI `core_mp`:
 * "7100699f cmp"@0xffffff8008a62e78 (core_mp.c6 contro 26) e
 * "7a4c0160 ccmp"@0xffffff8008a62e88 (core_mp.c4 contro 0x7807), con
 * "1a8b018b csel"@0xffffff8008a62e9c che sceglie 1500 quando TUTTI E DUE
 * combaciano e 930 altrimenti. I due numeri non hanno nome nel binario.
 *
 * LA FORMULA E' LETTA ISTRUZIONE PER ISTRUZIONE:
 *   "531a652e lsl"@0xffffff8008a62e80  + "4b0901c9 sub"@0xffffff8008a62e94
 *       -> code * 64 - code = code * 63
 *   "4b0a01aa sub"@0xffffff8008a62e84  -> v[0] - v[1]
 *   "530a254a lsl"@0xffffff8008a62e98  -> (v[0] - v[1]) << 22
 *   "1ac90d49 sdiv"@0xffffff8008a62ea0 -> divisione CON SEGNO
 *   "1b0b2929 madd"@0xffffff8008a62eac -> quoziente * 100 - costante
 *   "9b2a7d29 smull"@0xffffff8008a62eb8 + "9366fd29 asr"@0xffffff8008a62ec0
 *       (0x10624dd3, spostamento 38) -> divisione CON SEGNO per 1000
 *
 * IL VALORE RESO SUL RAMO D'ERRORE E' ZERO ("2a1f03e9 mov"@0xffffff8008a62ef4).
 */
static int codeToOhm(int code, u16 *v)
{
	int pull = 0;
	int ohm = 0;

	/* "340003a9 cbz"@0xffffff8008a62e58 */
	if (code == 0) {
		/* "\x013ILITEK: (%s, %d): code is invalid\n"@0xffffff8009244122,
		 * __func__ = "codeToOhm"@0xffffff8009244147;
		 * "5280ab22 mov"@0xffffff8008a62ed4 -- __LINE__ = 1369 */
#line 1369
		ILI_ERR("code is invalid\n");
		return 0;
	}

	pull = v[0] - v[1];
	ohm = ((pull << 22) / (code * 63) * 100 -
	       ((core_mp.c6 == 26 && core_mp.c4 == 0x7807) ? 1500 : 930)) / 1000;

	return ohm;
}

/*
 * ===========================================================================
 * mp_cdc_init_cmd_common -- INCORPORATA in `mutual_test`, riga 1568
 * ===========================================================================
 * NON E' UN SIMBOLO: il nome viene dal `__func__` del suo unico messaggio,
 * "mp_cdc_init_cmd_common"@0xffffff8009244083, che accompagna
 * "\x016ILITEK: (%s, %d): P2P CMD: %d,%d,%d,%d,%d\n"@0xffffff8009244056 e
 * "5280c402 mov"@0xffffff8008a62554 (__LINE__ = 1568). E' l'eccezione alla
 * regola 5.
 *
 * IL RIEMPIMENTO A 0xFF E' INCONDIZIONATO e precede la scelta di protocollo:
 * "9280000a mov"@0xffffff8008a623a0 (x10 = -1),
 * "f80673ea stur"@0xffffff8008a623a8 e "f90033ea str"@0xffffff8008a623ac
 * coprono i quindici byte del comando (96..110 sulla pila) prima di
 * "540004e9 b.ls"@0xffffff8008a623b0.
 *
 * LA SOGLIA DI PROTOCOLLO E' 0x503ff SENZA SEGNO: "6b09011f cmp"@0xffffff8008a623a4
 * e' seguito da `b.ls`, non da `b.le`. La costante e'
 * "52807fe9 mov"@0xffffff8008a62398 + "72a000a9 movk"@0xffffff8008a6239c.
 *
 * I TRE VALORI DI `core_mp.c228` SONO MISURATI: 15 sul ramo pv5_4
 * ("32000fe9 orr"@0xffffff8008a623b8), 3 sul ramo vecchio
 * ("320007e9 orr"@0xffffff8008a62450) e 5 quando l'elemento e' di tipo 8
 * ("528000a9 mov"@0xffffff8008a624ec). Tutte e tre finiscono a
 * 0xffffff800a0fcbe4 = `core_mp` + 228.
 */
static int mp_cdc_init_cmd_common(u8 *cmd, int len, int index)
{
	int ret = 0;
	/* "a90e7fff stp"@0xffffff8008a623d4 e le sette gemelle azzerano
	 * 128 byte (sp+112..sp+239): e' l'inizializzatore di dichiarazione */
	char str[128] = {0};

	memset(cmd, 0xFF, len);

	if ((unsigned int)core_mp.c12 > 0x503FFU) {
		core_mp.c228 = 15;
		/* "97fffedf bl"@0xffffff8008a6240c verso
		 * <parser_get_ini_key_value> e "94102066 bl"@0xffffff8008a62428
		 * verso <snprintf>: e' l'incorporazione di
		 * `parser_get_int_data`, e i suoi due `tbnz` consecutivi --
		 * "37f80055 tbnz"@0xffffff8008a6242c e
		 * "37f86e20 tbnz"@0xffffff8008a62430 -- sono l'espressione
		 * `(ret < 0) ? 0 : size` provata contro zero */
		ret = parser_get_int_data("pv5_4 command", mp_items[index].c0,
					  str, sizeof(str));
		if (ret < 0)
			return ret;

		/* "94000d64 bl"@0xffffff8008a62440, w2 = 15
		 * ("32000fe2 orr"@0xffffff8008a6243c) */
		return parser_get_u8_array(str, cmd, len);
	}

	/* "52801e28 mov"@0xffffff8008a6245c (0xf1) +
	 * "390183e8 strb"@0xffffff8008a62460 */
	cmd[0] = 0xF1;
	/* "39400115 ldrb"@0xffffff8008a62458 (mp_items[index].c20) +
	 * "390187f5 strb"@0xffffff8008a62468 */
	cmd[1] = mp_items[index].c20;
	/* "39018bff strb"@0xffffff8008a62454 */
	cmd[2] = 0x0;
	/* "b90be509 str"@0xffffff8008a62474 */
	core_mp.c228 = 3;

	/* "f100581f cmp"@0xffffff8008a62480 (0x16 = 22) e
	 * "open test(integration)"@0xffffff8009243648;
	 * "321f03e8 orr"@0xffffff8008a624a0 + "39018be8 strb"@0xffffff8008a624a4 */
	if (ipio_strcmp(mp_items[index].c0, "open test(integration)") == 0)
		cmd[2] = 0x2;

	/* "f100381f cmp"@0xffffff8008a624b0 (0xe = 14) e
	 * "open test(cap)"@0xffffff800924365f;
	 * "320007e8 orr"@0xffffff8008a624d0 + "39018be8 strb"@0xffffff8008a624d4 */
	if (ipio_strcmp(mp_items[index].c0, "open test(cap)") == 0)
		cmd[2] = 0x3;

	/* "b9400108 ldr"@0xffffff8008a624dc + "7100211f cmp"@0xffffff8008a624e0 */
	if (mp_items[index].c16 == 8) {
		/* "53087ec8 lsr"@0xffffff8008a624fc +
		 * "39018be8 strb"@0xffffff8008a62500 */
		cmd[2] = mp_items[index].c48 >> 8;
		/* "39018ff6 strb"@0xffffff8008a62508 */
		cmd[3] = mp_items[index].c48;
		/* "390193ff strb"@0xffffff8008a624f4 */
		cmd[4] = 0;
		/* "b90be509 str"@0xffffff8008a6250c */
		core_mp.c228 = 5;

		/* "f100741f cmp"@0xffffff8008a62514 (0x1d = 29) e
		 * "noise peak to peak(cut panel)"@0xffffff800924362a;
		 * "320003e7 orr"@0xffffff8008a628d0 +
		 * "390193e7 strb"@0xffffff8008a628d4 */
		if (ipio_strcmp(mp_items[index].c0,
				"noise peak to peak(cut panel)") == 0)
			cmd[4] = 1;

		/* "\x016ILITEK: (%s, %d): P2P CMD: %d,%d,%d,%d,%d\n"
		 * @0xffffff8009244056, __func__ =
		 * "mp_cdc_init_cmd_common"@0xffffff8009244083; la guardia e'
		 * "39656348 ldrb"@0xffffff8008a6253c +
		 * "34000168 cbz"@0xffffff8008a62540;
		 * "5280c402 mov"@0xffffff8008a62554 -- __LINE__ = 1568.
		 * I CINQUE ARGOMENTI vengono dai byte appena scritti, propagati
		 * dal compilatore: "52801e23 mov"@0xffffff8008a62558 (0xf1),
		 * "2a1503e4 mov"@0xffffff8008a62564,
		 * "53083ec5 ubfx"@0xffffff8008a6254c,
		 * "12001ec6 and"@0xffffff8008a62550 e w7 */
#line 1568
		ILI_DBG("P2P CMD: %d,%d,%d,%d,%d\n", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
	}

	return ret;
}

/*
 * ===========================================================================
 * allnode_mutual_cdc_data -- INCORPORATA in `mutual_test`, righe 1713..1795
 * ===========================================================================
 * NON E' UN SIMBOLO: il nome viene dal `__func__` che nove dei suoi messaggi
 * portano, "allnode_mutual_cdc_data"@0xffffff8009243e85, mentre i tre che la
 * circondano -- righe 2143, 2150 e 2187 -- portano
 * "mutual_test"@0xffffff8009243a84. E' la classe B5 usata al contrario.
 *
 * UN SOLO PARAMETRO, `index`: dentro il blocco i soli valori che vengono dal
 * chiamante sono i quattro campi di `mp_items[index]` che
 * `mp_cdc_init_cmd_common` legge, e il compilatore ne calcola gli indirizzi
 * UNA VOLTA SOLA fuori dal ciclo delle trame
 * ("f9002fe8 str"@0xffffff8008a62344 e le tre gemelle a [sp,#48], [sp,#32] e
 * [sp,#72]). Il buffer di destinazione NON e' un parametro: e' il globale
 * `mp_frame_buf0`, che il blocco rilegge dalla memoria a ogni uso
 * ("f9421100 ldr"@0xffffff8008a626a8, "f9421129 ldr"@0xffffff8008a627a0,
 * "f9421129 ldr"@0xffffff8008a627a0, "f9421108 ldr"@0xffffff8008a627d4) e
 * che il ramo d'allocazione SCRIVE ("f9021100 str"@0xffffff8008a628bc).
 *
 * I SEI CODICI DI RITORNO, letti dalle `mov` che li materializzano:
 *   "12800d15 mov"@0xffffff8008a631b0  = -105  (lunghezza non valida)
 *   "12800c75 mov"@0xffffff8008a6320c  = -100  (comando cdc non letto)
 *   "12800c75 mov"@0xffffff8008a63258  = -100  (scritture sul chip)
 *   "12800db5 mov"@0xffffff8008a631d4  = -110  (chip occupato)
 *   "12800d48 mov"@0xffffff8008a62858  = -107  (`ori` non allocato)
 *   "12800d68 mov"@0xffffff8008a6288c  = -108  (lettura cdc fallita)
 *   "12800d48 mov"@0xffffff8008a62908  = -107  (trama non allocata)
 * Nessuno di essi ha un nome nel binario e nessuno coincide con un errno.
 *
 * DIFETTO DELLA FABBRICA, RIPRODOTTO (regola 7) -- F6-D1:
 * L'AZZERAMENTO DEL BUFFER DI TRAMA COPRE `core_mp.c256` BYTE, NON
 * `core_mp.c256 * 4`: "b98c0322 ldrsw"@0xffffff8008a626b4 passa il NUMERO di
 * nodi come terzo argomento di `__memset` ("940fd651 bl"@0xffffff8008a626bc),
 * mentre l'allocazione dello stesso vettore moltiplica per quattro
 * ("d37ef500 lsl"@0xffffff8008a628a4). E' lo stesso difetto che il lotto
 * F3c-bis ha gia' trovato su `mp_frame_buf1` dentro `allnode_key_cdc_data`
 * (difetto D2 di quel blocco): sono due copie della stessa riga sbagliata.
 */
static int allnode_mutual_cdc_data(int index)
{
	int i = 0, idx = 0, ret = 0, len = 0, tmp = 0, val = 0;
	u8 cmd[15] = {0};
	u8 *ori = NULL;

	/* "294026e8 ldp"@0xffffff8008a62348 legge in UNA volta i due campi
	 * adiacenti `core_mp.c232` e `core_mp.c236`,
	 * "1b097d08 mul"@0xffffff8008a62358 li moltiplica,
	 * "531f7908 lsl"@0xffffff8008a6235c raddoppia e
	 * "11000919 add"@0xffffff8008a62360 aggiunge 2 */
	len = core_mp.c232 * core_mp.c236 * 2 + 2;

	/* "\x016ILITEK: (%s, %d): Read X/Y Channel length = %d\n"
	 * @0xffffff8009243e53; la guardia e'
	 * "3965634a ldrb"@0xffffff8008a62350 +
	 * "3400010a cbz"@0xffffff8008a62368;
	 * "5280d622 mov"@0xffffff8008a62374 -- __LINE__ = 1713 */
#line 1713
	ILI_DBG("Read X/Y Channel length = %d\n", len);

	/* "71000b3f cmp"@0xffffff8008a62388 + "5400706d b.le"@0xffffff8008a6238c */
	if (len <= 2) {
		/* "\x013ILITEK: (%s, %d): Length is invalid\n"@0xffffff8009243e9d;
		 * "5280d682 mov"@0xffffff8008a631a8 -- __LINE__ = 1716 */
#line 1716
		ILI_ERR("Length is invalid\n");
		ret = -105;
		goto out;
	}

	if (mp_cdc_init_cmd_common(cmd, sizeof(cmd), index) < 0) {
		/* "\x013ILITEK: (%s, %d): Failed to get cdc command\n"
		 * @0xffffff8009243ec4;
		 * "5280d7a2 mov"@0xffffff8008a63204 -- __LINE__ = 1725 */
#line 1725
		ILI_ERR("Failed to get cdc command\n");
		ret = -100;
		goto out;
	}

	/* "97ffe153 bl"@0xffffff8008a6258c con
	 * "Mutual CDC command"@0xffffff8009243ef3, w1 = 8
	 * ("321d03e1 orr"@0xffffff8008a6257c) e w2 = `core_mp.c228`
	 * ("b94be502 ldr"@0xffffff8008a62570) */
	ilitek_dump_data(cmd, 8, core_mp.c228, 0, "Mutual CDC command");

	/* "320003e9 orr"@0xffffff8008a62598 + "b902dd09 str"@0xffffff8008a625a0 */
	idev->c732 = 1;

	/* "f9418508 ldr"@0xffffff8008a625a4 (idev->c776),
	 * "b94be6a1 ldr"@0xffffff8008a625a8 (core_mp.c228) e
	 * "d63f0100 blr"@0xffffff8008a625ac */
	if (idev->c776(cmd, core_mp.c228) < 0) {
		/* "\x013ILITEK: (%s, %d): Write CDC command failed\n"
		 * @0xffffff8009243f06;
		 * "5280d8e2 mov"@0xffffff8008a631c8 -- __LINE__ = 1735 */
#line 1735
		ILI_ERR("Write CDC command failed\n");
		ret = -100;
		goto out;
	}

	/* "b94c6508 ldr"@0xffffff8008a625b8 (core_mp.c356) +
	 * "7100051f cmp"@0xffffff8008a625bc */
	if (core_mp.c356 == 1)
		/* "52800640 mov"@0xffffff8008a625c4 + "52800641 mov"@0xffffff8008a625c8 */
		ret = ilitek_tddi_ic_check_busy(50, 50);
	else
		/* "97ffdb7d bl"@0xffffff8008a625d4, nessun argomento */
		ret = ilitek_tddi_ic_check_int_stat();

	/* "37f85fa0 tbnz"@0xffffff8008a625e0 */
	if (ret < 0) {
		ret = -110;
		goto out;
	}

	/* "b94b1108 ldr"@0xffffff8008a625e8 (core_mp.c16), la costante 0x10401
	 * e' "52808029 mov"@0xffffff8008a625ec + "72a00029 movk"@0xffffff8008a625f0,
	 * e "540002a8 b.hi"@0xffffff8008a625f8 prova SENZA segno */
	if ((unsigned int)core_mp.c16 <= 0x10401U) {
		/* "529e5ec9 mov"@0xffffff8008a62600 (0xf2f6) +
		 * "7900c3e9 strh"@0xffffff8008a62604: DUE byte in una volta */
		cmd[0] = 0xF6;
		cmd[1] = 0xF2;
		/* "321f03e1 orr"@0xffffff8008a62610 -- lunghezza 2 */
		if (idev->c776(cmd, 2) < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x, 0x%x) error\n"
			 * @0xffffff8009243f34, con
			 * "394183e3 ldrb"@0xffffff8008a63218 e
			 * "394187e4 ldrb"@0xffffff8008a6321c;
			 * "5280dba2 mov"@0xffffff8008a63230 -- __LINE__ = 1757 */
#line 1757
			ILI_ERR("Write (0x%x, 0x%x) error\n", cmd[0], cmd[1]);
			ret = -100;
			goto out;
		}
		/* "52912b00 mov"@0xffffff8008a6261c +
		 * "72a00820 movk"@0xffffff8008a62620 = 0x418958 = 1000 * 0x10c7 */
		mdelay(1);

		/* "b2400129 orr"@0xffffff8008a622f0 mette a uno il bit 0 del
		 * puntatore -- `sp+96` e' allineato a otto, quindi e'
		 * `&cmd[1]` -- e "f90017e9 str"@0xffffff8008a622f4 lo salva
		 * fuori dal ciclo; "f94017e0 ldr"@0xffffff8008a6262c lo rilegge */
		if (idev->c776(&cmd[1], 1) < 0) {
			/* "\x013ILITEK: (%s, %d): Write (0x%x) error\n"
			 * @0xffffff8009243f62, con
			 * "394187e3 ldrb"@0xffffff8008a6323c;
			 * "5280dcc2 mov"@0xffffff8008a63250 -- __LINE__ = 1766 */
#line 1766
			ILI_ERR("Write (0x%x) error\n", cmd[1]);
			ret = -100;
			goto out;
		}
		mdelay(1);
	}

	/* "93407f20 sxtw"@0xffffff8008a62650 (la lunghezza e' un `int` con
	 * segno), "52901801 mov"@0xffffff8008a6264c +
	 * "72a02801 movk"@0xffffff8008a62654 = 0x14080c0 = GFP_KERNEL |
	 * __GFP_ZERO, "97dfb74b bl"@0xffffff8008a6265c verso <__kmalloc> */
	ori = kzalloc(len, GFP_KERNEL);
	/* "b140041f cmn"@0xffffff8008a62664 + "54000ea8 b.hi"@0xffffff8008a62668
	 * (IS_ERR) e "b4000e95 cbz"@0xffffff8008a6266c (NULL) */
	if (IS_ERR(ori) || ori == NULL) {
		/* "\x013ILITEK: (%s, %d): Failed to allocate ori, (%ld)\n"
		 * @0xffffff8009243f8a, con "aa1503e3 mov"@0xffffff8008a62850;
		 * "5280de42 mov"@0xffffff8008a62844 -- __LINE__ = 1778 */
#line 1778
		ILI_ERR("Failed to allocate ori, (%ld)\n", PTR_ERR(ori));
		ret = -107;
		goto out;
	}

	/* "f9418908 ldr"@0xffffff8008a6267c (idev->c784) +
	 * "d63f0100 blr"@0xffffff8008a62680, w1 = len */
	if (idev->c784(ori, len) < 0) {
		/* "\x013ILITEK: (%s, %d): Read cdc data error, len = %d\n"
		 * @0xffffff8009243fbd;
		 * "5280df22 mov"@0xffffff8008a62878 -- __LINE__ = 1785 */
#line 1785
		ILI_ERR("Read cdc data error, len = %d\n", len);
		ret = -108;
		goto out;
	}

	/* "97ffe10e bl"@0xffffff8008a626a0 con
	 * "Mutual CDC original"@0xffffff8009243ff0 */
	ilitek_dump_data(ori, 8, len, 0, "Mutual CDC original");

	/* "f9421100 ldr"@0xffffff8008a626a8 + "b4000f40 cbz"@0xffffff8008a626ac */
	if (mp_frame_buf0 == NULL) {
		/* la forma di `kcalloc`: "b98c0108 ldrsw"@0xffffff8008a62898
		 * (CON SEGNO), "37f80248 tbnz"@0xffffff8008a6289c (il controllo
		 * di traboccamento di `kmalloc_array`),
		 * "d37ef500 lsl"@0xffffff8008a628a4 (per quattro),
		 * "97dfb6b7 bl"@0xffffff8008a628ac verso <__kmalloc> e
		 * "f9021100 str"@0xffffff8008a628bc */
		mp_frame_buf0 = kcalloc(core_mp.c256, sizeof(int), GFP_KERNEL);
		/* "b140041f cmn"@0xffffff8008a628b4 +
		 * "54000188 b.hi"@0xffffff8008a628c0 e
		 * "b5ffefe3 cbnz"@0xffffff8008a628c8 */
		if (IS_ERR(mp_frame_buf0) || mp_frame_buf0 == NULL) {
			/* "\x013ILITEK: (%s, %d): Failed to allocate FrameBuffer mem (%ld)\n"
			 * @0xffffff8009244004;
			 * "5280e062 mov"@0xffffff8008a628f8 -- __LINE__ = 1795 */
#line 1795
			ILI_ERR("Failed to allocate FrameBuffer mem (%ld)\n", PTR_ERR(mp_frame_buf0));
			ret = -107;
			goto out;
		}
	} else {
		/* DIFETTO F6-D1: "b98c0322 ldrsw"@0xffffff8008a626b4 passa il
		 * numero di ELEMENTI come numero di BYTE */
		memset(mp_frame_buf0, 0, core_mp.c256);
	}

	/* LE DUE CATENE D'INDICE SONO DISTINTE, come nelle due gemelle: x26
	 * parte da 0x100000000 ("b26003fa orr"@0xffffff8008a626d4) e x22 da
	 * 0x200000000 ("b25f03f6 orr"@0xffffff8008a626d8), tutt'e due con passo
	 * 0x200000000 ("8b08035a add"@0xffffff8008a62800 e
	 * "8b0802d6 add"@0xffffff8008a62808), e i due indici escono da
	 * "9360ff48 asr"@0xffffff8008a62710 e "9360fec9 asr"@0xffffff8008a62718 */
	for (i = 0; i < core_mp.c256; i++) {
		idx = (2 * i) + 1;

		/* "calibration data(dac)"@0xffffff8009243552,
		 * "f100541f cmp"@0xffffff8008a626ec (0x15 = 21) */
		/* "38686aa8 ldrb"@0xffffff8008a62714 sta PRIMA di
		 * "34000300 cbz"@0xffffff8008a6271c: `ori[idx]` e' letto UNA
		 * VOLTA SOLA per tutt'e due i rami, mentre `ori[idx + 1]` e'
		 * letto DUE volte ("38696aa9 ldrb"@0xffffff8008a62720 e
		 * "38696aa9 ldrb"@0xffffff8008a6277c), una per ramo */
		val = ori[idx];

		if (ipio_strcmp(mp_items[index].c0, "calibration data(dac)") == 0) {
			/* "1200190a and"@0xffffff8008a62780 +
			 * "7219011f tst"@0xffffff8008a62784 +
			 * "5a8a0548 cneg"@0xffffff8008a62788 per il primo byte
			 * e la coppia gemella per il secondo; la divisione per
			 * due CON SEGNO e' "7100011f cmp"@0xffffff8008a627a4 +
			 * "1a88a508 cinc"@0xffffff8008a627a8 +
			 * "13017d08 asr"@0xffffff8008a627ac, e la `str` e'
			 * "b83c7928 str"@0xffffff8008a627b0 */
			mp_frame_buf0[i] = (((val & 0x80) ? -(val & 0x7F) : (val & 0x7F)) +
					    ((ori[idx + 1] & 0x80) ? -(ori[idx + 1] & 0x7F) : (ori[idx + 1] & 0x7F))) / 2;
		} else {
			/* "33181d09 bfi"@0xffffff8008a62730 fonde i due byte;
			 * "32103d2b orr"@0xffffff8008a62734 e
			 * "1a8b0128 csel"@0xffffff8008a62738 sono la
			 * sottrazione di 65536, e la prova
			 * "7219011f tst"@0xffffff8008a6272c e' sul BYTE ALTO
			 * GREZZO. UNA SOLA `str`
			 * ("b83c7948 str"@0xffffff8008a6273c) e UNA SOLA
			 * lettura del puntatore globale
			 * ("f942114a ldr"@0xffffff8008a62728): il valore passa
			 * per una variabile temporanea */
			tmp = (val << 8) + ori[idx + 1];
			if (val & 0x80)
				tmp -= 0x10000;
			mp_frame_buf0[i] = tmp;
		}

		/* "raw data(no bk)"@0xffffff8009243580 (15) e
		 * "raw data(no bk) (lcm off)"@0xffffff8009243731 (25): la
		 * `strlen` e' UNA SOLA ("940fd6ed bl"@0xffffff8008a6274c) e le
		 * due lunghezze sono provate una dopo l'altra
		 * ("f100641f cmp"@0xffffff8008a62750 e
		 * "f1003c1f cmp"@0xffffff8008a62758) -- il compilatore mette in
		 * comune la `strlen` delle due chiamate.
		 * QUI il valore e' letto e riscritto in memoria
		 * ("b86a690b ldr"@0xffffff8008a627e4 +
		 * "4b090169 sub"@0xffffff8008a627e8 +
		 * "b82a6909 str"@0xffffff8008a627ec), non tenuto in un
		 * registro: `core_mp.c212` e'
		 * "b94bd529 ldr"@0xffffff8008a627e0 */
		if (ipio_strcmp(mp_items[index].c0, "raw data(no bk)") == 0 ||
		    ipio_strcmp(mp_items[index].c0, "raw data(no bk) (lcm off)") == 0)
			mp_frame_buf0[i] -= core_mp.c212;
	}

	/* "97ffe0a9 bl"@0xffffff8008a62834 con
	 * "Mutual CDC combined"@0xffffff8009244042, w1 = 32
	 * ("321b03e1 orr"@0xffffff8008a6282c) e w3 = `core_mp.c232`
	 * ("b94beb63 ldr"@0xffffff8008a62824) */
	ilitek_dump_data(mp_frame_buf0, 32, core_mp.c256, core_mp.c232,
			 "Mutual CDC combined");

out:
	/* "b902dd1f str"@0xffffff8008a62864, "b902dd1f str"@0xffffff8008a62914
	 * e "b902dd1f str"@0xffffff8008a63260: TRE copie, una per gruppo di
	 * cammini */
	idev->c732 = 0;
	/* "b5000595 cbnz"@0xffffff8008a62868 + "97dfb7a5 bl"@0xffffff8008a6291c
	 * verso <kfree>. Sui cammini in cui `ori` vale ancora NULL la
	 * liberazione non compare affatto */
	ipio_kfree((void **)&ori);
	return ret;
}

/*
 * ===========================================================================
 * mutual_test -- 0xffffff8008a620d4, 4584 byte, `t` (1146 istruzioni)
 * ===========================================================================
 * Un parametro `int`, l'indice dell'elemento di collaudo: il segno e'
 * MISURATO da "93407ed3 sxtw"@0xffffff8008a62104, che lo estende CON SEGNO
 * prima di moltiplicarlo per 152 ("9b082668 madd"@0xffffff8008a62120).
 *
 * `static`: nella mappa e' `t` ("ffffff8008a620d4 t mutual_test"). L'indirizzo
 * arriva a `mp_items[i].c144` dal caso 7 di `ilitek_tddi_mp_init_item`, sul
 * ramo `else` dei due confronti sul nome.
 *
 * PORTA DENTRO DI SE' TRE FUNZIONI INCORPORATE -- `codeToOhm` (riga 1369),
 * `mp_cdc_init_cmd_common` (riga 1568) e `allnode_mutual_cdc_data`
 * (righe 1713..1795) -- piu' `parser_get_int_data` (riga 566) e
 * `ipio_strcmp`. Le prime tre sono provate dai loro `__func__`.
 *
 * LO `switch` CENTRALE HA SEI VIE e la sua TAVOLA DI SALTO sta a
 * 0xffffff8008f7ec4c, sei mezze parole:
 *
 *   ffffff8008f7ec4c  00003200 6000ed00 0e011f01
 *
 * cioe' 0x0000, 0x0032, 0x0060, 0x00ed, 0x010e, 0x011f, ognuna moltiplicata
 * per 4 e sommata alla base "10000089 adr"@0xffffff8008a62944, che vale
 * 0xffffff8008a62954. Il `sub w8, w8, #0x4` piu' il `cmp w8, #0x5` che
 * precedono ("51001108 sub"@0xffffff8008a62930,
 * "7100151f cmp"@0xffffff8008a62934) dicono che i casi vanno da 4 a 9:
 *
 *   c16 == 4 -> 0xffffff8008a62954   tx/rx delta
 *   c16 == 5 -> 0xffffff8008a62a1c   massimo/minimo e differenza
 *   c16 == 6 -> 0xffffff8008a62ad4   il confronto coi vicini
 *   c16 == 7 -> 0xffffff8008a62d08   open test
 *   c16 == 8 -> 0xffffff8008a62d8c   LO STESSO del ramo `default`
 *   c16 == 9 -> 0xffffff8008a62dd0   short test, con `codeToOhm`
 *   default  -> 0xffffff8008a62d8c
 *
 * Che il caso 8 cada sul `default` significa che nel sorgente `case 8` NON
 * c'e': clang riempie i buchi dell'intervallo 4..9 con l'etichetta di
 * `default`.
 *
 * IL PUNTATORE `mp_frame_buf0` E' COPIATO IN UN LOCALE nei casi 4, 5, 6 e 7,
 * e NON nei casi 9 e `default`. E' una MISURA, non una lettura: nel caso 5 il
 * puntatore e' letto una volta sola ("f9421108 ldr"@0xffffff8008a62a34) e non
 * viene piu' riletto benche' fra un uso e l'altro ci sia una `str` che
 * potrebbe scrivergli sopra, mentre nello stesso ciclo
 * `mp_items[index].max_buf` E' riletto ("f940028d ldr"@0xffffff8008a62a4c a
 * ogni giro). Nel `default`, invece, il puntatore e' riletto a ogni giro
 * ("f942114a ldr"@0xffffff8008a62da8, dentro il corpo del ciclo). Le due cose
 * non possono venire dalla stessa forma di sorgente.
 *
 * TRE DIFETTI DELLA FABBRICA, RIPRODOTTI (regola 7), oltre a F6-D1:
 *
 *  F6-D2. IL CICLO CHE INIZIALIZZA MASSIMI E MINIMI LEGGE `mp_items[0].c16`,
 *      NON `mp_items[index].c16`. "b946e152 ldr"@0xffffff8008a621e0 legge
 *      l'indirizzo COSTANTE 0xffffff80099876e0 = `mp_items` + 16, e il
 *      registro di base non e' toccato fra
 *      "b000792a adrp"@0xffffff8008a621bc e la lettura: l'indice non compare
 *      affatto in quell'accesso. `mp_items[0]` e' "baseline data(bg)", che
 *      nella tavola ha `c16 = 0`: il ramo `== 4` di quel ciclo, di fabbrica,
 *      non parte mai, e i quattro vettori di `core_mp` restano com'erano
 *      anche per l'elemento "tx/rx delta" che e' l'unico con `c16 == 4`.
 *
 *  F6-D3. NEL CASO 4 I DUE `delta` SONO CALCOLATI SULLA STESSA DIFFERENZA.
 *      "1100054a add"@0xffffff8008a629a0 e "1100054a add"@0xffffff8008a629d8
 *      calcolano tutt'e due `i * c232 + j + 1`, cioe' il vicino a DESTRA,
 *      mentre le due GUARDIE sono diverse -- "6b0d011f cmp"@0xffffff8008a6298c
 *      prova `x != c236 - 1` (il vicino di SOTTO) e
 *      "6b0d019f cmp"@0xffffff8008a629c8 prova `y != c232 - 1` (il vicino a
 *      destra). Il vettore `tx_delta_buf` riceve quindi la differenza
 *      orizzontale con la guardia verticale.
 *
 *  F6-D4. NEL CASO 6 TUTTI E QUATTRO I VICINI SONO `idx - 1` E `idx + 1`.
 *      Le nove vie del blocco caricano SOLO due indirizzi,
 *      `idx - 1` e `idx + 1`: non c'e' nessuna moltiplicazione per
 *      `core_mp.c232` fra le due, e nei quattro angoli le due `ILI_ABS` che
 *      il sorgente scrive si riducono a UNA SOLA lettura perche' sono
 *      identiche (per esempio "1100060e add"@0xffffff8008a62c70 e' l'unico
 *      accesso del ramo `x == 0 && y == 0`). La forma scritta qui riproduce
 *      la struttura -- quattro vicini per l'interno, tre per i bordi, due per
 *      gli angoli -- con gli indici che il binario porta.
 *
 * IL VALORE -107 DEL FALLIMENTO DI `create_mp_test_frame_buffer`
 * ("12800d55 mov"@0xffffff8008a62284) NON e' seguito dal messaggio
 * "Failed to initialise CDC data": quel cammino salta direttamente all'uscita
 * ("140003fe b"@0xffffff8008a62288 verso 0xffffff8008a63280).
 */
static int mutual_test(int index)
{
	int i = 0, j = 0, x = 0, y = 0, ret = 0;
	int frame_count = 0;

	/* "\x016ILITEK: (%s, %d): index = %d, desp = %s, Frame Count = %d\n"
	 * @0xffffff8009243a47, __func__ = "mutual_test"@0xffffff8009243a84; la
	 * guardia e' "39656348 ldrb"@0xffffff8008a6210c +
	 * "340001c8 cbz"@0xffffff8008a62110; i tre argomenti sono
	 * "2a1603e3 mov"@0xffffff8008a62140 (l'indice),
	 * "f9400104 ldr"@0xffffff8008a62124 (il nome, campo a +0) e
	 * "b9403105 ldr"@0xffffff8008a62128 (il conteggio, campo a +48);
	 * "52810be2 mov"@0xffffff8008a6213c -- __LINE__ = 2143 */
#line 2143
	ILI_DBG("index = %d, desp = %s, Frame Count = %d\n", index, mp_items[index].c0, mp_items[index].c48);

	/* "b8430ee1 ldr"@0xffffff8008a62158 (pre-indice, x23 diventa
	 * `&mp_items[index].c48`) + "7100003f cmp"@0xffffff8008a6215c +
	 * "5400012c b.gt"@0xffffff8008a62160: il confronto e' CON SEGNO e con
	 * zero, non `== 0` come in `key_test` */
	if (mp_items[index].c48 < 1) {
		/* "\x013ILITEK: (%s, %d): Frame count is zero, which is at least set as 1\n"
		 * @0xffffff8009243a90;
		 * "52810cc2 mov"@0xffffff8008a62174 -- __LINE__ = 2150 */
#line 2150
		ILI_ERR("Frame count is zero, which is at least set as 1\n");
		/* "320003e1 orr"@0xffffff8008a6217c +
		 * "b90002e1 str"@0xffffff8008a62180 */
		mp_items[index].c48 = 1;
	}

	/* "94000c12 bl"@0xffffff8008a62188. Il secondo argomento e' la STESSA
	 * lettura del campo a +48 che ha appena servito il confronto: w1 non e'
	 * ricaricato fra "b8430ee1 ldr"@0xffffff8008a62158 e la chiamata.
	 * "2a0003f5 mov"@0xffffff8008a62198 + "37f807c0 tbnz"@0xffffff8008a6218c */
	ret = create_mp_test_frame_buffer(index, mp_items[index].c48);
	if (ret < 0) {
		/* "12800d55 mov"@0xffffff8008a62284 */
		ret = -107;
		goto out;
	}

	/* DIFETTO F6-D2: la guardia legge `mp_items[0].c16`.
	 * "b94bec51 ldr"@0xffffff8008a62194 (core_mp.c236) e
	 * "b94be930 ldr"@0xffffff8008a621a8 (core_mp.c232) sono i due limiti;
	 * le quattro costanti sono "320103ee orr"@0xffffff8008a621cc
	 * (0x80000000) e "32007bef orr"@0xffffff8008a621d0 (0x7fffffff) */
	for (x = 0; x < core_mp.c236; x++) {
		for (y = 0; y < core_mp.c232; y++) {
			/* "b946e152 ldr"@0xffffff8008a621e0 +
			 * "7100125f cmp"@0xffffff8008a621e4 */
			if (mp_items[0].c16 == 4) {
				/* "f9402db2 ldr"@0xffffff8008a621ec (core_mp+320)
				 * + "b830da4e str"@0xffffff8008a621fc */
				core_mp.tx_max_buf[x * core_mp.c232 + y] = INT_MIN;
				/* "f94035b2 ldr"@0xffffff8008a62204 (core_mp+336)
				 * + "b830da4e str"@0xffffff8008a6220c */
				core_mp.rx_max_buf[x * core_mp.c232 + y] = INT_MIN;
				/* "910181a1 add"@0xffffff8008a621f8 (core_mp+328)
				 * e "912fa1ad add"@0xffffff8008a621c8 danno la
				 * base; la `str` e'
				 * "b830d832 str"@0xffffff8008a62230, fusa col
				 * ramo `else` */
				core_mp.tx_min_buf[x * core_mp.c232 + y] = INT_MAX;
				/* "9101c1a0 add"@0xffffff8008a621f4 (core_mp+344);
				 * la `str` e' "b830da4f str"@0xffffff8008a62240,
				 * fusa col ramo `else` */
				core_mp.rx_min_buf[x * core_mp.c232 + y] = INT_MAX;
			} else {
				/* "320103f2 orr"@0xffffff8008a6221c +
				 * "aa0b03e1 mov"@0xffffff8008a62220
				 * (&mp_items[index].max_buf, da
				 * "9101a18b add"@0xffffff8008a621c0) */
				mp_items[index].max_buf[x * core_mp.c232 + y] = INT_MIN;
				/* "aa0c03e0 mov"@0xffffff8008a62224
				 * (&mp_items[index].min_buf, da
				 * "9101c18c add"@0xffffff8008a621c4) */
				mp_items[index].min_buf[x * core_mp.c232 + y] = INT_MAX;
			}
		}
	}

	/* "b8410d28 ldr"@0xffffff8008a6226c (pre-indice, x9 diventa
	 * `&mp_items[index].c16`) + "7100211f cmp"@0xffffff8008a62270 +
	 * "540000a1 b.ne"@0xffffff8008a62278; "320003e8 orr"@0xffffff8008a6227c
	 * (uno) e "b94002e8 ldr"@0xffffff8008a6228c (il campo a +48);
	 * "b90047e8 str"@0xffffff8008a62290 lo salva a [sp,#68] */
	if (mp_items[index].c16 == 8)
		frame_count = 1;
	else
		frame_count = mp_items[index].c48;

	/* "39405508 ldrb"@0xffffff8008a6229c (mp_items[index].c21, UN byte) +
	 * "7100051f cmp"@0xffffff8008a622a0 */
	if (mp_items[index].c21 == 1) {
		/* "a94786a0 ldp"@0xffffff8008a622bc legge in UNA volta i due
		 * puntatori adiacenti a +120 e +128,
		 * "39405aa2 ldrb"@0xffffff8008a622c4 il byte a +22,
		 * "f94002a3 ldr"@0xffffff8008a622c0 il nome e
		 * "b94c0124 ldr"@0xffffff8008a622b8 `core_mp.c256`;
		 * "97ffee87 bl"@0xffffff8008a622c8 */
		parser_ini_benchmark(mp_items[index].bench_mark_max,
				     mp_items[index].bench_mark_min,
				     mp_items[index].c22, mp_items[index].c0,
				     core_mp.c256);
		/* "a94786a0 ldp"@0xffffff8008a622cc RILEGGE i due puntatori +
		 * "94000d1a bl"@0xffffff8008a622d4 */
		dump_benchmark_data(mp_items[index].bench_mark_max,
				    mp_items[index].bench_mark_min);
	}

	/* "b94047e8 ldr"@0xffffff8008a622d8 + "7100051f cmp"@0xffffff8008a622dc
	 * + "54007d0b b.lt"@0xffffff8008a622e0 in testa, e in coda
	 * "294a2bf5 ldp"@0xffffff8008a63180 (ret e i insieme) +
	 * "b94047e8 ldr"@0xffffff8008a63184 + "6b08015f cmp"@0xffffff8008a6318c */
	for (i = 0; i < frame_count; i++) {
		ret = allnode_mutual_cdc_data(index);
		if (ret < 0) {
			/* "\x013ILITEK: (%s, %d): Failed to initialise CDC data, %d\n"
			 * @0xffffff8009243ad5, con
			 * "2a1503e3 mov"@0xffffff8008a63278;
			 * "52811162 mov"@0xffffff8008a63274 -- __LINE__ = 2187 */
#line 2187
			ILI_ERR("Failed to initialise CDC data, %d\n", ret);
			goto out;
		}

		/* "b9400108 ldr"@0xffffff8008a6292c: il campo e' RILETTO dopo
		 * la lettura dei dati */
		switch (mp_items[index].c16) {
		case 4: {
			/* "f9421129 ldr"@0xffffff8008a62970: il puntatore e'
			 * letto UNA VOLTA SOLA, fuori dai due cicli */
			int *frame = mp_frame_buf0;

			for (x = 0; x < core_mp.c236; x++) {
				for (y = 0; y < core_mp.c232; y++) {
					/* "6b0d011f cmp"@0xffffff8008a6298c +
					 * "54000180 b.eq"@0xffffff8008a62994 */
					if (x != core_mp.c236 - 1)
						/* "f94026ef ldr"@0xffffff8008a629ac
						 * (core_mp+304, tx_delta_buf) +
						 * "6b0a01ca subs"@0xffffff8008a629b0
						 * + "5a8a554a cneg"@0xffffff8008a629b4
						 * + "b82d69ea str"@0xffffff8008a629b8.
						 * DIFETTO F6-D3: l'indice e'
						 * `+ 1`, il vicino a destra */
						core_mp.tx_delta_buf[x * core_mp.c232 + y] =
							ILI_ABS(frame[x * core_mp.c232 + y] -
								frame[x * core_mp.c232 + y + 1]);

					/* "6b0d019f cmp"@0xffffff8008a629c8 +
					 * "54000160 b.eq"@0xffffff8008a629cc */
					if (y != core_mp.c232 - 1)
						/* "f9402aee ldr"@0xffffff8008a629e4
						 * (core_mp+312, rx_delta_buf) +
						 * "b82b69ca str"@0xffffff8008a629f0 */
						core_mp.rx_delta_buf[x * core_mp.c232 + y] =
							ILI_ABS(frame[x * core_mp.c232 + y] -
								frame[x * core_mp.c232 + y + 1]);
				}
			}
			break;
		}
		case 5: {
			/* "f9421108 ldr"@0xffffff8008a62a34 */
			int *frame = mp_frame_buf0;

			for (x = 0; x < core_mp.c236; x++) {
				for (y = 0; y < core_mp.c232; y++) {
					/* "6b0e019f cmp"@0xffffff8008a62a64 +
					 * "5400008d b.le"@0xffffff8008a62a68 +
					 * "b82e69ac str"@0xffffff8008a62a70 */
					if (frame[x * core_mp.c232 + y] >
					    mp_items[index].max_buf[x * core_mp.c232 + y])
						mp_items[index].max_buf[x * core_mp.c232 + y] =
							frame[x * core_mp.c232 + y];

					/* "6b0d019f cmp"@0xffffff8008a62a80 +
					 * "540000aa b.ge"@0xffffff8008a62a84 +
					 * "b82d69cc str"@0xffffff8008a62a8c.
					 * "b86e690c ldr"@0xffffff8008a62a74
					 * RILEGGE `frame[idx]` dopo la `str`
					 * precedente */
					if (frame[x * core_mp.c232 + y] <
					    mp_items[index].min_buf[x * core_mp.c232 + y])
						mp_items[index].min_buf[x * core_mp.c232 + y] =
							frame[x * core_mp.c232 + y];

					/* "4b0d018c sub"@0xffffff8008a62aac +
					 * "b82b69cc str"@0xffffff8008a62ab0 */
					mp_items[index].buf[x * core_mp.c232 + y] =
						mp_items[index].max_buf[x * core_mp.c232 + y] -
						mp_items[index].min_buf[x * core_mp.c232 + y];
				}
			}
			break;
		}
		case 6: {
			/* "f9421108 ldr"@0xffffff8008a62aec */
			int *frame = mp_frame_buf0;

			for (x = 0; x < core_mp.c236; x++) {
				for (y = 0; y < core_mp.c232; y++) {
					int tmp[4] = {0};
					/* "b86bd90d ldr"@0xffffff8008a62c24 sta PRIMA
					 * di "340001ec cbz"@0xffffff8008a62c30: il
					 * nodo e' letto UNA VOLTA SOLA per tutte e nove
					 * le vie */
					int v = frame[x * core_mp.c232 + y];

					/* "2a09014c orr"@0xffffff8008a62c28 +
					 * "340001ec cbz"@0xffffff8008a62c30:
					 * `x == 0 && y == 0` ripiegato in una
					 * sola prova, e lo ZERO che ne esce e'
					 * riusato come valore di `tmp[3]` */
					if (x == 0 && y == 0) {
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else if (y == 0 && x == core_mp.c236 - 1) {
						/* "5100060e sub"@0xffffff8008a62c48 e
						 * "11000610 add"@0xffffff8008a62c50 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else if (x == 0 && y == core_mp.c232 - 1) {
						/* "110005d0 add"@0xffffff8008a62c9c e
						 * "510005ce sub"@0xffffff8008a62ca4 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
					} else if (y == core_mp.c232 - 1 && x == core_mp.c236 - 1) {
						/* "510005ce sub"@0xffffff8008a62b24:
						 * un solo accesso, il vicino a sinistra */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
					} else if (x == 0) {
						/* "110005cf add"@0xffffff8008a62b38 e
						 * "510005ce sub"@0xffffff8008a62b40 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[2] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else if (y == 0) {
						/* "5100060e sub"@0xffffff8008a62b6c e
						 * "1100060f add"@0xffffff8008a62b74 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
						tmp[2] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else if (x == core_mp.c236 - 1) {
						/* "510005cf sub"@0xffffff8008a62ba8 e
						 * "110005ce add"@0xffffff8008a62bb0 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[2] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else if (y == core_mp.c232 - 1) {
						/* fusa col ramo seguente dalle tre
						 * "1a8d13ec csel"@0xffffff8008a62c0c,
						 * "1a9011af csel"@0xffffff8008a62c10 e
						 * "1a8d120e csel"@0xffffff8008a62c14,
						 * comandate da
						 * "6a0e023f tst"@0xffffff8008a62c08 */
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[2] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					} else {
						tmp[0] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[1] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
						tmp[2] = ILI_ABS(v - frame[x * core_mp.c232 + y - 1]);
						tmp[3] = ILI_ABS(v - frame[x * core_mp.c232 + y + 1]);
					}

					/* "6b1001df cmp"@0xffffff8008a62cc4 +
					 * "1a90c1cd csel"@0xffffff8008a62cc8,
					 * "6b0d01ff cmp"@0xffffff8008a62ccc +
					 * "1a8dc1ed csel"@0xffffff8008a62cd4,
					 * "6b0d019f cmp"@0xffffff8008a62cd8 +
					 * "1a8dc18c csel"@0xffffff8008a62cdc, e
					 * "b82b79cc str"@0xffffff8008a62ce0 */
					mp_items[index].buf[x * core_mp.c232 + y] =
						ILI_MAX(tmp[3], ILI_MAX(tmp[2], ILI_MAX(tmp[1], tmp[0])));
				}
			}
			break;
		}
		case 7: {
			/* "f9421119 ldr"@0xffffff8008a62d0c: il puntatore e'
			 * letto in cima al caso, PRIMA dei due confronti sul
			 * nome */
			int *frame = mp_frame_buf0;

			/* "f100581f cmp"@0xffffff8008a62d2c (0x16 = 22) e
			 * "open test(integration)"@0xffffff8009243648.
			 * LA `strlen` E' UNA SOLA per i due confronti
			 * ("940fd579 bl"@0xffffff8008a62d1c): il compilatore la
			 * mette in comune */
			if (ipio_strcmp(mp_items[index].c0, "open test(integration)") == 0) {
				/* "b8696b2a ldr"@0xffffff8008a62d6c +
				 * "b829696a str"@0xffffff8008a62d78 */
				for (j = 0; j < core_mp.c256; j++)
					mp_items[index].buf[j] = frame[j];

			/* "f100381f cmp"@0xffffff8008a62d20 (0xe = 14) e
			 * "open test(cap)"@0xffffff800924365f.
			 * E' UN `else if`, ed e' MISURATO: la `strlen` e' UNA
			 * SOLA per i due confronti. Con due `if` indipendenti
			 * il ciclo del primo scrive in memoria e il
			 * compilatore DEVE richiamare `strlen` (il suo
			 * argomento potrebbe essere stato riscritto): due
			 * chiamate invece di una */
			} else if (ipio_strcmp(mp_items[index].c0, "open test(cap)") == 0) {
				for (x = 0; x < core_mp.c236; x++) {
					for (y = 0; y < core_mp.c232; y++) {
						int sum = 0;
						int cnt = 0;
						int avg = 0;
						/* "b860db2e ldr"@0xffffff8008a62fc4 */
						int v = frame[x * core_mp.c232 + y];

						/* "29402eea ldp"@0xffffff8008a6313c legge
						 * `core_mp.c232` e `core_mp.c236` in UNA
						 * volta, una volta per giro */
						if (x - 1 >= 0 && x - 1 < core_mp.c236 &&
						    y - 1 >= 0 && y - 1 < core_mp.c232) {
							/* "1b0c354f madd"@0xffffff8008a62ffc +
							 * "b86fdb2f ldr"@0xffffff8008a63000 */
							sum += frame[(x - 1) * core_mp.c232 + (y - 1)];
							cnt++;
						}
						if (x - 1 >= 0 && x - 1 < core_mp.c236 &&
						    y >= 0 && y < core_mp.c232) {
							/* "11000421 add"@0xffffff8008a63014 +
							 * "b861db21 ldr"@0xffffff8008a63018 */
							sum += frame[(x - 1) * core_mp.c232 + y];
							cnt++;
						}
						if (x - 1 >= 0 && x - 1 < core_mp.c236 &&
						    y + 1 >= 0 && y + 1 < core_mp.c232) {
							/* "11000821 add"@0xffffff8008a63034 +
							 * "b861db21 ldr"@0xffffff8008a63038 */
							sum += frame[(x - 1) * core_mp.c232 + (y + 1)];
							cnt++;
						}
						if (x >= 0 && x < core_mp.c236 &&
						    y + 1 >= 0 && y + 1 < core_mp.c232) {
							/* "0b1001a1 add"@0xffffff8008a63068 +
							 * "11000821 add"@0xffffff8008a6306c */
							sum += frame[x * core_mp.c232 + (y + 1)];
							cnt++;
						}
						if (x + 1 >= 0 && x + 1 < core_mp.c236 &&
						    y + 1 >= 0 && y + 1 < core_mp.c232) {
							/* "1b083541 madd"@0xffffff8008a63090 +
							 * "11000821 add"@0xffffff8008a63094 */
							sum += frame[(x + 1) * core_mp.c232 + (y + 1)];
							cnt++;
						}
						if (x + 1 >= 0 && x + 1 < core_mp.c236 &&
						    y >= 0 && y < core_mp.c232) {
							/* "1b083541 madd"@0xffffff8008a630d8 +
							 * "11000421 add"@0xffffff8008a630dc */
							sum += frame[(x + 1) * core_mp.c232 + y];
							cnt++;
						}
						if (x + 1 >= 0 && x + 1 < core_mp.c236 &&
						    y - 1 >= 0 && y - 1 < core_mp.c232) {
							/* "1b083541 madd"@0xffffff8008a630bc +
							 * "b861db21 ldr"@0xffffff8008a630c0 */
							sum += frame[(x + 1) * core_mp.c232 + (y - 1)];
							cnt++;
						}
						if (x >= 0 && x < core_mp.c236 &&
						    y - 1 >= 0 && y - 1 < core_mp.c232) {
							/* "0b1001aa add"@0xffffff8008a63110 +
							 * "b86adb2a ldr"@0xffffff8008a63114 */
							sum += frame[x * core_mp.c232 + (y - 1)];
							cnt++;
						}

						/* "0b0e01ea add"@0xffffff8008a63124 +
						 * "1100062f add"@0xffffff8008a63128 +
						 * "1acf0d4a sdiv"@0xffffff8008a6312c */
						avg = (sum + v) / (cnt + 1);
						/* "1b047dce mul"@0xffffff8008a63130 (100,
						 * da "52800c84 mov"@0xffffff8008a62fa0) +
						 * "1aca0dca sdiv"@0xffffff8008a63134 +
						 * "b82b7a0a str"@0xffffff8008a63138 */
						mp_items[index].buf[x * core_mp.c232 + y] = v * 100 / avg;
					}
				}
			}
			break;
		}
		case 9: {
			/* Le quattro mezze parole sono scritte
			 * INCONDIZIONATAMENTE, prima della scelta di
			 * protocollo: "7901e3e9 strh"@0xffffff8008a62de4,
			 * "7901e7ea strh"@0xffffff8008a62df8,
			 * "7900e3eb strh"@0xffffff8008a62e04 e
			 * "7900e7ec strh"@0xffffff8008a62e1c */
			u16 v1[2];
			u16 v2[2];

			v1[0] = mp_items[index].c64;
			v1[1] = mp_items[index].c68;
			v2[0] = mp_items[index].c72;
			v2[1] = mp_items[index].c76;

			/* "b940010d ldr"@0xffffff8008a62e14 (core_mp.c12) +
			 * "6b0901bf cmp"@0xffffff8008a62e20 +
			 * "540007a9 b.ls"@0xffffff8008a62e24 */
			if ((unsigned int)core_mp.c12 > 0x503FFU) {
				for (j = 0; j < core_mp.c256; j++)
					/* "396f8d4a ldrb"@0xffffff8008a62e48
					 * (core_mp.c227) +
					 * "7100015f cmp"@0xffffff8008a62e50 +
					 * "9a96032a csel"@0xffffff8008a62e54;
					 * "b8757929 ldr"@0xffffff8008a62e4c e
					 * "b828d949 str"@0xffffff8008a62f04 */
					mp_items[index].buf[i * core_mp.c256 + j] =
						codeToOhm(mp_frame_buf0[j],
							  (core_mp.c227 == 0) ? v2 : v1);
			} else {
				/* "b869794a ldr"@0xffffff8008a62f3c +
				 * "b828d96a str"@0xffffff8008a62f44 */
				for (j = 0; j < core_mp.c256; j++)
					mp_items[index].buf[i * core_mp.c256 + j] =
						mp_frame_buf0[j];
			}
			break;
		}
		default:
			/* "f942114a ldr"@0xffffff8008a62da8: il puntatore
			 * globale e' RILETTO a ogni giro, e questa e' la
			 * differenza misurata rispetto ai casi 4..7.
			 * "1b082448 madd"@0xffffff8008a62db0 +
			 * "b828d96a str"@0xffffff8008a62dbc */
			for (j = 0; j < core_mp.c256; j++)
				mp_items[index].buf[i * core_mp.c256 + j] =
					mp_frame_buf0[j];
			break;
		}

		/* "1b027d08 mul"@0xffffff8008a63170 +
		 * "8b28c921 add"@0xffffff8008a63174 +
		 * "940009bb bl"@0xffffff8008a6317c */
		compare_MaxMin_result(index, &mp_items[index].buf[core_mp.c256 * i]);
	}

out:
	return ret;
}
