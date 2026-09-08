// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_FT8719_E977 -- FocalTech FT8719, Doogee S88 Pro (MT6771).
 * L'UNITA' DI TRADUZIONE DEL FLASH E DELLO STATO DI BOOT (unita' B).
 * UNITA' COMPLETA: DICIASSETTE funzioni su diciassette.
 * Lotto B1: sei funzioni.  Lotto B2: otto.  Lotto B3: tre, 2026-08-22.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA.  Nessun sorgente
 * pubblico e' stato letto.  L'albero ALPS su cui questo file compila
 * CONTIENE un driver FocalTech pubblico
 * (drivers/input/touchscreen/mediatek/focaltech_fhd_touch/ e
 * .../focaltech_touch/): NON sono stati aperti.
 *
 * ===========================================================================
 * 0. IL NOME DI QUESTO FILE E' UNA SCELTA, NON UNA MISURA
 * ===========================================================================
 * Il binario NON contiene i nomi dei file sorgente di FocalTech.  Le due
 * strade per ricavarli sono chiuse tutte e due, e la chiusura e' misurata:
 *
 *   - i descrittori `_ddebug` (che portano `.filename`) esistono solo per
 *     pr_debug/dev_dbg, e FocalTech non ne usa:
 *       $ ./venv/bin/python3 ddebug.py oracolo/stock.elf --file focaltech
 *       dopo il filtro: 0
 *   - non c'e' nessuna stringa di percorso `.c` del driver nel `.rodata`.
 *
 * `focaltech_flash.c` e' quindi una SCELTA di questo lotto, presa sul
 * contenuto MISURATO dell'unita' (lettura e scrittura del flash, stato di
 * boot, pramboot, aggiornamento generico) e sullo schema di nome gia' usato
 * dalle altre cinque unita' di questo driver.  I lotti successivi dell'unita'
 * B scrivono in QUESTO file.
 *
 * ===========================================================================
 * 1. CHE COSA C'E' QUI DENTRO, E QUAL E' IL CONFINE
 * ===========================================================================
 * Il blocco WTK_FT8719_E977 e' 67 funzioni nella mappa piu' una in
 * `.exit.text` che la mappa non vede (68 in tutto, 22092 byte), ripartite in
 * SEI unita' di traduzione.  La partizione e' MISURATA -- l'ordine del testo,
 * quello di `.data` e quello di `.bss` danno la stessa.
 *
 * Questo file e' l'unita' B:
 *   testo   0xffffff8008a7cc4c .. 0xffffff8008a7eac0   17 funzioni, 7796 byte
 *   .bss    0xffffff800a100ac8 .. 0xffffff800a100ad0   8 byte, un solo
 *                                                      puntatore
 *   .data   -- (nessuna)
 *
 * Il confine `.bss` e' misurato per differenza: l'unita' A (focaltech_sysfs.c)
 * finisce a 0xffffff800a100ac8 e l'unita' C (focaltech_core.c) comincia a
 * 0xffffff800a100ad0, e in mezzo c'e' esattamente un puntatore.
 *
 * LE DICIASSETTE FUNZIONI DELL'UNITA', IN ORDINE DI INDIRIZZO.  Questa
 * tabella e' il contratto fra i lotti B1, B2 e B3: clang emette le funzioni
 * nell'ordine in cui il sorgente le definisce, quindi ognuna va scritta al
 * suo posto, non in coda.
 *
 *   indirizzo           byte  vis  nome                        lotto
 *   ------------------  ----  ---  --------------------------  -----
 *   0xffffff8008a7cc4c  1792   T   fts_pram_write_init          B2
 *   0xffffff8008a7d34c   504   T   fts_fwupg_get_boot_state     B1
 *   0xffffff8008a7d544   224   t   fts_pram_init                B2
 *   0xffffff8008a7d624   320   T   fts_fwupg_reset_to_romboot   B2
 *   0xffffff8008a7d764   148   T   fts_fwupg_check_state        B1
 *   0xffffff8008a7d7f8    64   T   fts_fwupg_check_fw_valid     B1
 *   0xffffff8008a7d838   148   T   fts_fwupg_reset_in_boot      B2
 *   0xffffff8008a7d8cc   140   T   fts_fwupg_reset_to_boot      B2
 *   0xffffff8008a7d958   348   T   fts_fwupg_enter_into_boot    B2
 *   0xffffff8008a7dab4   268   T   fts_fwupg_erase              B2
 *   0xffffff8008a7dbc0   540   T   fts_fwupg_ecc_cal            B2
 *   0xffffff8008a7dddc   584   T   fts_flash_write_buf          B1
 *   0xffffff8008a7e024   352   T   fts_flash_read_buf           B1
 *   0xffffff8008a7e184   280   T   fts_flash_read               B1
 *   0xffffff8008a7e29c   456   T   fts_read_file                B3
 *   0xffffff8008a7e464   412   T   fts_upgrade_bin              B3
 *   0xffffff8008a7e600  1216   T   fts_fwupg_upgrade            B3
 *   ------------------  ----  ---  --------------------------  -----
 *                       7796         = 0xa7eac0 - 0xa7cc4c
 *
 * La colonna `vis` viene dalla mappa: `T` globale, `t` static.  Una sola
 * funzione dell'unita' e' static, ed e' fts_pram_init, ed e' `static` anche
 * qui.  Le altre sedici sono tutte `T`, quindi NON possono esserlo.
 *
 * OLTRE ALLE DICIASSETTE DELLA MAPPA, QUESTO FILE DEFINISCE QUATTORDICI
 * FUNZIONI `static` CHE LA MAPPA NON HA, perche' di fabbrica sono INCORPORATE
 * nei loro chiamanti e non lasciano simbolo.  Non sono un'aggiunta: senza di
 * loro le funzioni che le contengono non misurano come di fabbrica.  Sono
 * fts_fwupg_check_flash_status (dentro fts_fwupg_erase e fts_fwupg_ecc_cal),
 * le sette di fts_pram_write_init -- fts_crc16_calc_host, fts_pram_write_buf,
 * fts_pram_ecc_cal_algo, fts_pram_ecc_cal_xor, fts_pram_ecc_cal,
 * fts_pram_start, fts_pram_write_remap -- e le sei di fts_fwupg_upgrade
 * (lotto B3) -- fts_fwupg_get_ver_in_host, fts_fwupg_get_ver_in_tp,
 * fts_fwupg_need_upgrade, fts_param_get_ver_in_host, fts_param_get_ver_in_tp,
 * fts_param_need_upgrade.  Ognuna ha nel suo cappello la prova che esiste e la
 * dichiarazione che il suo NOME e' una scelta.  La misura che lo verifica e'
 * negativa e sta nell'elenco dei simboli in coda al file: nel nostro oggetto
 * ci sono DICIASSETTE simboli di funzione, non trentuno.
 *
 * E DUE FUNZIONI `T` DI QUESTA STESSA UNITA' SONO INCORPORATE OLTRE CHE
 * EMESSE FUORI LINEA, il che non e' una contraddizione (senza LTO clang puo'
 * fare tutte e due le cose): fts_fwupg_reset_in_boot ha simbolo proprio a
 * 0xffffff8008a7d838 ed e' incorporata TRE volte (in fts_flash_read, in
 * fts_upgrade_bin e due volte in fts_fwupg_upgrade -- quattro copie in tutto),
 * e fts_fwupg_check_fw_valid ha simbolo a 0xffffff8008a7d7f8 ed e' incorporata
 * DUE volte, tutte e due dentro fts_fwupg_upgrade.
 *
 * PERCHE' UN FILE NUOVO E NON UN'AGGIUNTA A UNO ESISTENTE.  E' una DECISIONE,
 * e la ragione e' misurabile: mettere due unita' nello stesso `.c` darebbe a
 * clang la facolta' di incorporare fra loro funzioni che di fabbrica stanno in
 * file oggetto distinti (nessun LTO), e cambierebbe la dimensione di funzioni
 * gia' misurate.  Che clang lo faccia davvero e' provato DENTRO questa stessa
 * unita': fts_fwupg_reset_in_boot ha simbolo proprio a 0xffffff8008a7d838 ed
 * e' contemporaneamente INCORPORATA dentro fts_flash_read (vedi il cappello di
 * quella funzione).
 *
 * ===========================================================================
 * 2. COME SI RIVERIFICA
 * ===========================================================================
 * Sul PC di build, in /mnt/s88pro/kernel-stock:
 *
 *   ./venv/bin/python3 verificacitazioni.py \
 *       albero-ft8719/drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_flash.c \
 *       oracolo/stock.elf --eccezione "[FTS][Info]" --eccezione "[FTS][Error]" \
 *       --eccezione "\n" --eccezione "[FTS]"
 *
 *   ./venv/bin/python3 verificaistruzioni.py \
 *       .../focaltech_flash.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a7cc4c:0xffffff8008a7eac0 \
 *       --controfattuale '1a9f1502 csinc' --controfattuale '71000eff cmp'
 *
 * L'intervallo e' TUTTA l'unita' B e non le sole sei funzioni: alcune
 * citazioni di questo file stanno nelle funzioni VICINE apposta -- per
 * esempio quelle di fts_fwupg_reset_in_boot, che servono a provare che il
 * blocco finale di fts_flash_read e' una copia incorporata di quella
 * funzione e non codice suo.
 *
 * LE `--controfattuale` SONO DUE, e tutte e due sono forme SBAGLIATE che un
 * lotto ha scritto e poi scartato; nel binario di fabbrica non c'e' ne' l'una
 * ne' l'altra, e senza la dichiarazione lo strumento le segna ASSENTE e ha
 * ragione.
 *   "1a9f1502 csinc"  il `csinc ..., ne` del NOSTRO oggetto prima della
 *                     correzione del ternario in fts_fwupg_get_boot_state
 *                     (lotto B1);
 *   "71000eff cmp"    il `cmp w23, #0x3` della seconda stesura del ciclo di
 *                     fts_fwupg_upgrade, prima che diventasse un `do/while`
 *                     (lotto B3).
 *
 * ESITO ATTESO dopo il lotto B3: i numeri stanno nel blocco in coda al file,
 * sotto "LE CITAZIONI", e vengono dalla misura fatta PER ULTIMA COSA sul file
 * consegnato.
 *
 * La misura di dimensione, dal `.o` VERO (non da misuraisolata.py), con TUTTI
 * E DUE i compilatori:
 *
 *   ./compila-ft8719-b1.sh nostro werror     # clang r383902 (11.0.1)
 *   ./compila-ft8719-b1.sh fab - drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_flash.o
 *   ./misura-b1.sh nostro focaltech_flash.o
 *   ./misura-b1.sh fab    focaltech_flash.o
 *
 * ===========================================================================
 * 3. LA FORMA DELLE CITAZIONI
 * ===========================================================================
 * istruzione:  "<codifica8hex> <mnemonico>"@0xINDIRIZZO
 * letterale:   "testo"@0xINDIRIZZO
 * Sono le due forme che verificaistruzioni.py e verificacitazioni.py sanno
 * confrontare col binario.  Un numero di riga dentro un listato effimero NON
 * e' una prova (classe B7).
 *
 * ATTENZIONE nel leggere gli scarti: il kernel di fabbrica e'
 * CONFIG_RELOCATABLE=y e i nostri indirizzi non sono i suoi; ogni
 * `adrp`/`add #lo12`/`bl` che materializza un simbolo ha per forza una
 * codifica diversa dalla nostra.  Il confronto per CODIFICA maschera quei
 * campi e conta il residuo.
 *
 * ===========================================================================
 * 4. LE DIVERGENZE APERTE
 * ===========================================================================
 * Elencate e attribuite nel blocco in CODA a questo file.
 *
 * ===========================================================================
 * 5. CIO' CHE NON E' SCRITTO QUI, E PERCHE'
 * ===========================================================================
 * NIENTE: l'unita' B e' COMPLETA.  Tutte e diciassette le funzioni della
 * tabella al §1 sono scritte, e la colonna `lotto` non ha piu' nessun `--`.
 *
 * Cio' che resta indefinito in questo file sono le SEI funzioni di confine,
 * dichiarate e non definite qui perche' stanno in altre unita' di traduzione
 * (fts_i2c_read, fts_i2c_write, fts_i2c_hid2std, fts_i2c_write_reg,
 * fts_i2c_read_reg dall'unita' E, fts_wait_tp_to_valid dall'unita' C).  Non
 * sono stub: le definisce un altro oggetto della stessa directory, e il conto
 * degli irrisolti in coda al file lo prova.
 *
 * QUELLO CHE MANCA AL DRIVER non e' piu' in questa unita': sono le QUATTRO
 * funzioni dell'unita' A (fts_debug_read, fts_debug_write, fts_tprwreg_show,
 * fts_tprwreg_store), e sono le sole quattro funzioni di FT8719 che restano
 * irrisolte.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
/* I QUATTRO INCLUDE AGGIUNTI DAL LOTTO B3, e ognuno per un simbolo che il
 * binario nomina in fts_read_file:
 *   linux/fs.h       filp_open, vfs_read, filp_close, struct file, struct inode
 *   linux/uaccess.h  set_fs/get_fs/KERNEL_DS -- la coppia
 *                    "d503379f dsb"@0xffffff8008a7e3a8 +
 *                    "d5033fdf isb"@0xffffff8008a7e3ac e la
 *                    "940f62cd bl"@0xffffff8008a7e3b8 verso set_bit
 *   linux/vmalloc.h  vmalloc, vfree
 *   linux/slab.h     nessun simbolo nuovo; ci sta per uniformita' con le
 *                    altre unita' di questo driver */
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

/* ==========================================================================
 * LE MACRO DI STAMPA -- lette dalle stringhe, non supposte
 * ==========================================================================
 * Il binario porta i formati interi, prefisso e livello KERN compresi:
 *   "\x016[FTS][Info]**********read boot id**********\n"@0xffffff800924e4d6
 *   "\x013[FTS][Error]upgrade/func/fw_sts is null\n"@0xffffff800924e505
 * cioe' due macro: una con KERN_INFO e prefisso "[FTS][Info]", una con
 * KERN_ERR e prefisso "[FTS][Error]".  Le sei funzioni del lotto B1 non
 * usano ne' la macro senza livello ne' FTS_FUNC_ENTER/FTS_FUNC_EXIT: nessuna
 * di loro emette un "%s: Enter" o un "%s: Exit(%d)"; e nessuna delle
 * quattordici scritte lo fa -- FTS_FUNC_ENTER/EXIT non compaiono in questa
 * unita'.
 *
 * x0 e' il FORMATO e x1 e' il primo argomento -- classe B1, il difetto preso
 * al contrario da un lotto precedente.  Qui lo prova la coppia di
 * fts_fwupg_get_boot_state:
 *   "9115d400 add"@0xffffff8008a7d440   x0 = 0x924e000+0x575, il FORMATO
 *                                       "read boot id:0x%02x%02x"
 *   "394003e1 ldrb"@0xffffff8008a7d434  w1 = id[0]
 *   "394007e2 ldrb"@0xffffff8008a7d438  w2 = id[1]
 */
#define FTS_INFO(fmt, args...)		printk(KERN_INFO "[FTS][Info]" fmt "\n", ##args)
#define FTS_ERROR(fmt, args...)		printk(KERN_ERR "[FTS][Error]" fmt "\n", ##args)

/* LA TERZA MACRO -- SENZA LIVELLO KERN E CON PREFISSO "[FTS]" NUDO.  Il lotto
 * B1 aveva scritto che nessuna delle sue sei funzioni la usa, ed e' vero; due
 * delle otto di questo lotto la usano, e il binario la distingue dalle altre
 * due perche' il testo assemblato NON comincia con un byte di livello:
 *   "[FTS]pram not supported, confirm in bootloader\n"@0xffffff800924e7b4
 *   "[FTS]ecc calc startaddr:0x%04x, len:%d\n"@0xffffff800924e90a
 * contro
 *   "\x016[FTS][Info]pramboot initialization\n"@0xffffff800924ef46
 *   "\x013[FTS][Error]read flash type fail\n"@0xffffff800924ef6c
 *
 * IL NOME `FTS_DEBUG` E' UNA SCELTA, NON UNA MISURA: il binario non nomina le
 * macro.  Cio' che e' MISURATO e' il testo che assembla -- prefisso "[FTS]",
 * nessun livello, "\n" in coda -- e questo e' quello che la macro fa.
 */
#define FTS_DEBUG(fmt, args...)		printk("[FTS]" fmt "\n", ##args)

/* ==========================================================================
 * LE COSTANTI, lette dagli immediati
 * ==========================================================================
 * LA TAGLIA DEL PACCHETTO, 32.  Non e' scritta come 32 in nessun posto solo:
 * compare come `lsr #5` per la divisione, `#0x1f` per il resto e `#0x20` per
 * il passo, e le tre cose stanno insieme in fts_flash_read_buf:
 *   "53057c68 lsr"@0xffffff8008a7e058   packet_number = len >> 5
 *   "7200107f tst"@0xffffff8008a7e05c   len & 0x1f
 *   "12001076 and"@0xffffff8008a7e064   remainder = len & 0x1f
 *   "321b03f8 orr"@0xffffff8008a7e09c   w24 = 0x20
 *   "11008294 add"@0xffffff8008a7e0fc   w20 += 0x20  (l'indirizzo)
 *   "91008273 add"@0xffffff8008a7e104   x19 += 0x20  (il buffer)
 * `lsr` e non `asr`, e `and` senza correzione di segno: `len` e' UNSIGNED
 * (classe A3).
 */
#define FTS_FLASH_PACKET_LENGTH		32

/* Il comando di lettura del flash e la sua lunghezza:
 *   "320007e8 orr"@0xffffff8008a7e088   w8 = 3, poi
 *   "390013e8 strb"@0xffffff8008a7e08c  packet_buf[0] = 3
 *   "321e03e2 orr"@0xffffff8008a7e0b4   w2 = 4, la writelen della i2c_write
 *   "320003e0 orr"@0xffffff8008a7e0d4   w0 = 1, l'argomento di msleep
 */
#define FTS_CMD_READ			0x03
#define FTS_CMD_READ_LEN		4
#define FTS_CMD_READ_DELAY		1

/* Il comando di scrittura del flash, la sua intestazione di sei byte e il
 * comando di stato:
 *   "528017e8 mov"@0xffffff8008a7de78   w8 = 0xbf, poi
 *   "3900c3e8 strb"@0xffffff8008a7de7c  packet_buf[0] = 0xbf
 *   "1100190b add"@0xffffff8008a7defc   l'indice del dato e' j + 6
 *   "11001aa2 add"@0xffffff8008a7df14   la writelen e' packet_len + 6
 *   "52800d58 mov"@0xffffff8008a7dea8   w24 = 0x6a, poi
 *   "3900b3f8 strb"@0xffffff8008a7df6c  cmd = 0x6a
 *   "7101933f cmp"@0xffffff8008a7df98   il tentativo numero 100
 */
#define FTS_CMD_WRITE			0xBF
#define FTS_CMD_WRITE_LEN		6
#define FTS_CMD_FLASH_STATUS		0x6A
#define FTS_RETRIES_WRITE		100

/* I due byte del comando di ingresso in boot, e il ritardo che li segue:
 *   "52954aa8 mov"@0xffffff8008a7d3dc   w8 = 0xaa55, poi
 *   "79000be8 strh"@0xffffff8008a7d3ec  cmd[0]=0x55, cmd[1]=0xaa
 *   "52800140 mov"@0xffffff8008a7d3f8   w0 = 10, l'argomento di msleep
 *   "52801209 mov"@0xffffff8008a7d404   w9 = 0x90, poi
 *   "b90007e9 str"@0xffffff8008a7d408   cmd[0..3] = 0x90,0,0,0 -- una `str`
 *                                       a 32 bit, non una `strb`
 */
#define FTS_CMD_START1			0x55
#define FTS_CMD_START2			0xAA
#define FTS_CMD_START_DELAY		10
#define FTS_CMD_READ_ID			0x90

/* Il ciclo di attesa dello stato di boot, in fts_fwupg_check_state:
 *   "52800280 mov"@0xffffff8008a7d7ac   w0 = 20, l'argomento di msleep
 *   "71007abf cmp"@0xffffff8008a7d7b8   il giro numero 30
 */
#define FTS_UPGRADE_LOOP		30
#define FTS_DELAY_UPGRADE_LOOP		20

/* Il comando di reset e il ritardo che lo segue, in fts_fwupg_reset_in_boot
 * e nella sua copia incorporata dentro fts_flash_read:
 *   "32000be9 orr"@0xffffff8008a7d858   w9 = 7   (fuori linea)
 *   "32000be8 orr"@0xffffff8008a7e21c   w8 = 7   (incorporata)
 *   "52800a00 mov"@0xffffff8008a7d880   w0 = 0x50 = 80  (fuori linea)
 *   "52800a00 mov"@0xffffff8008a7e240   w0 = 0x50 = 80  (incorporata)
 */
#define FTS_REG_RESET_FW		0x07
#define FTS_DELAY_UPGRADE_RESET		80

/* I TRE REGISTRI DI VERSIONE LETTI DA fts_fwupg_upgrade.  I loro numeri
 * stanno negli immediati:
 *   "528014c1 mov"@0xffffff8008a7e914   w1 = 0xa6
 *   "528016a1 mov"@0xffffff8008a7e838   w1 = 0xb5
 *   "528016c1 mov"@0xffffff8008a7e804   w1 = 0xb6
 * I LORO NOMI NON SONO INVENTATI, ed e' l'eccezione della regola 5: il binario
 * li NOMINA, nelle stringhe di formato con cui l'unita' A ne stampa il valore
 * uno per uno:
 *   "FW Ver:0x%02x\n"@0xffffff800924e02f        -> 0xa6
 *   "Param Ver:0x%02x\n"@0xffffff800924e057     -> 0xb5
 *   "Param status:0x%02x\n"@0xffffff800924e069  -> 0xb6
 * e focaltech_sysfs.c li dichiara gia' con questi tre nomi (righe 174, 176,
 * 177), leggendoli dalle stesse stringhe.  Qui sono ripetuti perche' non c'e'
 * un header condiviso fra le unita' di questo driver. */
#define FTS_REG_FW_VER			0xa6
#define FTS_REG_PARAM_VER		0xb5
#define FTS_REG_PARAM_STATUS		0xb6

/* I TRE STATI DI BOOT CHE QUESTA UNITA' SCRIVE in *fw_sts.  I valori sono
 * misurati, e cosi' il ruolo di ognuno: ciascuno e' preceduto dal messaggio
 * che lo nomina.
 *   "321f03e8 orr"@0xffffff8008a7d470   w8 = 2, dopo
 *      "\x016[FTS][Info]tp run in romboot\n"@0xffffff800924e59b
 *   "320007e8 orr"@0xffffff8008a7d508   w8 = 3, dopo
 *      "\x016[FTS][Info]tp run in pramboot\n"@0xffffff800924e5bb
 *   "321e03e8 orr"@0xffffff8008a7d534   w8 = 4, dopo
 *      "\x016[FTS][Info]tp run in bootloader\n"@0xffffff800924e5dc
 * e tutti e tre finiscono nella stessa "b9000268 str"@0xffffff8008a7d538.
 * I valori 0 e 1 NON compaiono in nessuna delle quattordici funzioni scritte
 * (il lotto B2 ha aggiunto altri due confronti su questo stesso stato,
 * "7100091f cmp"@0xffffff8008a7ccc4 e "71000d1f cmp"@0xffffff8008a7cccc, e
 * sono ancora 2 e 3): non li dichiaro, perche' un nome per un valore non
 * misurato sarebbe indistinguibile da un fatto (regola 5).
 */
#define FTS_RUN_IN_ROM			2
#define FTS_RUN_IN_PRAM			3
#define FTS_RUN_IN_BOOTLOADER		4

/* ==========================================================================
 * LE STRUTTURE
 * ==========================================================================
 * `struct fts_ts_data` -- la taglia totale, 384 byte, e' misurata
 * dall'unita' C (la devm_kmalloc di tpd_probe).  Qui sono nominati i SOLI
 * campi che le funzioni scritte finora toccano; il resto e' riempimento
 * esplicito, perche' un nome inventato sarebbe indistinguibile da un fatto
 * (classe B2).
 *
 *   +16   u8   "39404108 ldrb"@0xffffff8008a7d410, letto e confrontato con 0
 *              da "7100011f cmp"@0xffffff8008a7d420 per scegliere fra
 *              writelen 4 e writelen 1
 *   +34   u8   "3940891b ldrb"@0xffffff8008a7d394  \
 *   +35   u8   "39408d1a ldrb"@0xffffff8008a7d398  / confrontati con l'id
 *              letto, e il ramo che combacia stampa "tp run in romboot"
 *   +36   u8   "39409119 ldrb"@0xffffff8008a7d39c  \
 *   +37   u8   "39409518 ldrb"@0xffffff8008a7d3a0  / -> "tp run in pramboot"
 *   +38   u8   "39409917 ldrb"@0xffffff8008a7d3a4  \
 *   +39   u8   "39409d16 ldrb"@0xffffff8008a7d3a8  / -> "tp run in bootloader"
 */
struct fts_ts_data {
	u8 c0[16];		/* non toccati da queste sei funzioni */
	u8 c16;
	u8 c17[17];		/* non toccati da queste sei funzioni */
	u8 c34;
	u8 c35;
	u8 c36;
	u8 c37;
	u8 c38;
	u8 c39;
	u8 c40[344];		/* fino a 384 = la taglia misurata dall'unita' C */
};

/* `fts_data` e' l'UNICO globale che il binario nomina, e lo nomina in chiaro
 * nel messaggio dell'unita' C:
 *   "\x013[FTS][Error]Failed to allocate memory for fts_data\n"@0xffffff800924f718
 * Sta a 0xffffff800a100ad8, lo definisce l'unita' C (focaltech_core.c) e qui
 * e' `extern`.  Che non sia `static` la' e' PROVATO: diciannove funzioni
 * sparse su quattro unita' di traduzione lo leggono, fra cui le due di questo
 * file. */
extern struct fts_ts_data *fts_data;

/* ==========================================================================
 * IL PUNTATORE DI STATO DELL'AGGIORNAMENTO, a 0xffffff800a100ac8
 * ==========================================================================
 * IL NOME E' UNA SCELTA, NON UNA MISURA (regola 5): il binario non lo
 * contiene -- la mappa di fabbrica ha solo simboli di testo,
 *   $ awk '{print $2}' oracolo/stock.map | sort | uniq -c
 *         1 A
 *     30100 T
 *       237 W
 *     23372 t
 * cioe' nessun `b`/`B`/`d`/`D` -- quindi la variabile si chiama come il suo
 * indirizzo di fabbrica.
 *
 * CHE NON SIA `static` E' MISURATO, ed e' la classe di difetto A1 usata al
 * contrario.  La scansione dell'intera immagine disassemblata trova SETTE
 * siti, tutti di LETTURA e tutti dentro l'unita' B:
 *   $ ./venv/bin/python3 corr-ft8719/scan.py 0xffffff800a100ac8
 *   riferimenti a 0xffffff800a100ac8:
 *     fts_fwupg_enter_into_boot          ldr    x1
 *     fts_fwupg_get_boot_state           ldr    x1
 *     fts_fwupg_upgrade                  ldr    x2
 *     fts_pram_write_init                ldr    x2
 *     fts_upgrade_bin                    ldr    x1
 *   TOTALE siti: 7
 * NESSUNO ci scrive.  Un `static` mai assegnato varrebbe NULL e clang
 * (GlobalOpt) lo avrebbe piegato a costante, cancellando la
 * "b40005d4 cbz"@0xffffff8008a7d3c0 e tutto il seguito; il binario invece lo
 * carica a larghezza piena, "f9456534 ldr"@0xffffff8008a7d390, e poi ci
 * dirama sopra.  Quindi NON e' static.
 *
 * CONSEGUENZA DI FABBRICA, che si riproduce (regola 7): siccome nessuno lo
 * scrive, a tempo di esecuzione vale sempre NULL, e fts_fwupg_get_boot_state
 * prende SEMPRE il ramo "upgrade/func/fw_sts is null".  Non e' un errore di
 * lettura: e' cio' che il binario fa.
 */
struct fts_upgrade_func {
	u8 c0[32];		/* non toccati dalle funzioni scritte finora */
	u32 c32;		/* QUATTRO byte: "b940218c ldr"@0xffffff8008a7cee0
				 * e "b9402129 ldr"@0xffffff8008a7cf2c sono
				 * `ldr w`, non `ldrb`.  Confrontato con zero da
				 * "7100019f cmp"@0xffffff8008a7cee4: se e' zero
				 * la somma di controllo del pramboot e' uno XOR
				 * di byte, se non lo e' e' un CRC-16 con
				 * polinomio 0x8408 */
	u32 c36;		/* QUATTRO byte: "b9402502 ldr"@0xffffff8008a7e68c e
				 * "b9402508 ldr"@0xffffff8008a7e918 sono `ldr w`.
				 * E' lo scostamento, dentro il firmware, del byte
				 * di versione: "b9400aa1 ldr"@0xffffff8008a7e688
				 * carica la lunghezza e
				 * "6b02003f cmp"@0xffffff8008a7e690 la confronta
				 * con questo campo prima di indicizzare con
				 * "38686935 ldrb"@0xffffff8008a7e920 */
	u8 c40[16];		/* non toccati, fino a +56 */
	u32 c56;		/* QUATTRO byte: "b9403902 ldr"@0xffffff8008a7e7c8
				 * e "b9403908 ldr"@0xffffff8008a7e7ec sono `ldr w`.
				 * Stesso ruolo di c36 ma per il ramo `param`:
				 * "6b02003f cmp"@0xffffff8008a7e7cc e poi
				 * "3868693a ldrb"@0xffffff8008a7e7f0 */
	u8 c60[4];		/* non toccati, fino a +64 */
	u8 c64;			/* "39410108 ldrb"@0xffffff8008a7d3cc, poi
				 * "34000068 cbz"@0xffffff8008a7d3d0: se e'
				 * non-zero si chiama fts_i2c_hid2std */
	u8 c65;			/* "39410508 ldrb"@0xffffff8008a7d9f8, poi
				 * "34000188 cbz"@0xffffff8008a7d9fc: se e'
				 * non-zero si scrive il pramboot, se e' zero
				 * si va a "confirm in bootloader".  UN BYTE:
				 * la lettura e' `ldrb`, non `ldr` */
	u8 c66[6];		/* non toccati, fino a +72 */
	u8 *c72;		/* OTTO byte: "f9402517 ldr"@0xffffff8008a7cd20
				 * e' `ldr x`; il valore e' usato come base di
				 * "386b4aeb ldrb"@0xffffff8008a7cecc, cioe' e'
				 * un puntatore a byte */
	u32 c80;		/* QUATTRO byte: "b9405114 ldr"@0xffffff8008a7cd28
				 * e' `ldr w`; e' la lunghezza, confrontata
				 * SENZA segno da "71047e9f cmp"@0xffffff8008a7cd2c
				 * + "540005a8 b.hi"@0xffffff8008a7cd30 */
	u8 c84[12];		/* non toccati, fino a +96 */
	int (*c96)(struct i2c_client *client, u8 *buf, u32 len);
				/* OTTO byte: "f9403108 ldr"@0xffffff8008a7e740 e
				 * "f9403108 ldr"@0xffffff8008a7e534 sono `ldr x`,
				 * e il valore finisce in una chiamata indiretta,
				 * "d63f0100 blr"@0xffffff8008a7e754.  I TRE
				 * ARGOMENTI sono letti dai registri del sito:
				 * "aa1303e0 mov"@0xffffff8008a7e750 (x0=client),
				 * "f9400281 ldr"@0xffffff8008a7e748 (x1=fw) e
				 * "b9400a82 ldr"@0xffffff8008a7e74c (w2=len, a 32
				 * bit) */
	u8 c104[16];		/* non toccati, fino a +120 */
	int (*c120)(struct i2c_client *client, u8 *buf, u32 len);
				/* OTTO byte: "f9403d08 ldr"@0xffffff8008a7e780 e
				 * "f9403d08 ldr"@0xffffff8008a7e894, `ldr x`,
				 * chiamato da "d63f0100 blr"@0xffffff8008a7e898
				 * con gli stessi tre argomenti di c96 */
	int (*c128)(struct i2c_client *client, u8 *buf, u32 len);
				/* OTTO byte: "f9404108 ldr"@0xffffff8008a7e520,
				 * `ldr x`, chiamato dallo stesso
				 * "d63f0100 blr"@0xffffff8008a7e548 di c96 --
				 * i due rami di fts_upgrade_bin convergono sulla
				 * chiamata */
};

/*
 * LE TRE POSIZIONI c96/c120/c128 SONO CONFERMATE DA UN'ALTRA UNITA', ed e'
 * una verifica indipendente dal codice: l'unita' F (focaltech_ft8719_upgrade.c)
 * ha l'ISTANZA di questa struttura, a 0xffffff800998d7e0, e la legge dalle
 * rilocazioni invece che dalle istruzioni.  La' la stessa struttura si chiama
 * `struct fts_g_998d7e0_t` e i campi hanno l'offset in ESADECIMALE:
 *   +0x48 -> il pramboot                (qui c72)
 *   +0x50 -> la sua lunghezza, 0xbe0    (qui c80)
 *   +0x60 -> fts_ft8719_upgrade         (qui c96)
 *   +0x78 -> fts_ft8719_param_upgrade   (qui c120)
 *   +0x80 -> 0, NON relocato            (qui c128)
 * e i quattro campi che questa unita' legge come numeri valgono
 *   +0x20 = 1        (qui c32, il bivio della somma di controllo)
 *   +0x24 = 0x10e    (qui c36, lo scostamento della versione fw)
 *   +0x38 = 0x10004  (qui c56, lo scostamento della versione param)
 *   +0x40 = 0x100    (qui c64 = 0 e c65 = 1, i due byte)
 * Le due letture concordano su tutti e otto gli offset.
 *
 * CONSEGUENZA DI FABBRICA, che si riproduce (regola 7): c128 vale ZERO e non
 * e' relocato, quindi fts_upgrade_bin con `force` vero stampa SEMPRE
 * "force_upgrade function is null, no upgrade" e non aggiorna niente.  Non e'
 * un errore di lettura: e' cio' che la tabella di fabbrica contiene.
 */

struct fts_upgrade {
	u8 *c0;			/* OTTO byte: "f94002a9 ldr"@0xffffff8008a7e680 e
				 * "f9400349 ldr"@0xffffff8008a7e7bc sono `ldr x`,
				 * e il valore e' usato come base di
				 * "38686935 ldrb"@0xffffff8008a7e920, cioe' e' un
				 * puntatore a byte.  E' anche il secondo
				 * argomento delle due chiamate indirette,
				 * "f9400281 ldr"@0xffffff8008a7e748 */
	u32 c8;			/* QUATTRO byte: "b9400aa1 ldr"@0xffffff8008a7e688
				 * e' `ldr w`; e' la lunghezza, confrontata SENZA
				 * segno con c36 da "6b02003f cmp"@0xffffff8008a7e690
				 * + "54001322 b.cs"@0xffffff8008a7e694 */
	u8 c12[20];		/* non toccati, fino a +32 */
	struct fts_upgrade_func *c32;
				/* "f9401288 ldr"@0xffffff8008a7d3c4 */
};

struct fts_upgrade *fts_g_a100ac8;

/* ==========================================================================
 * IL CONFINE CON LE ALTRE UNITA' -- DICHIARATE, NON DEFINITE QUI
 * ==========================================================================
 * Le firme vengono dai registri dei siti di chiamata.
 */
int fts_i2c_read(struct i2c_client *client, char *writebuf, int writelen,
		 char *readbuf, int readlen);	/* unita' E, 0xffffff8008a802ac */
int fts_i2c_write(struct i2c_client *client, char *writebuf, int writelen);
						/* unita' E, 0xffffff8008a80494 */
void fts_i2c_hid2std(struct i2c_client *client);
						/* unita' E, 0xffffff8008a80668 */
int fts_i2c_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue);
						/* unita' E, 0xffffff8008a805c4 */
int fts_i2c_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue);
						/* unita' E, 0xffffff8008a80614 */
int fts_wait_tp_to_valid(struct i2c_client *client);
						/* unita' C, 0xffffff8008a7eac0 */

/* ==========================================================================
 * LE TRE FUNZIONI DI QUESTA UNITA' CHE ANCORA NON SONO SCRITTE
 * ==========================================================================
 * fts_read_file (0xffffff8008a7e29c), fts_upgrade_bin (0xffffff8008a7e464) e
 * fts_fwupg_upgrade (0xffffff8008a7e600).  Sono DICHIARATE E LASCIATE
 * INDEFINITE (regola 6) solo dove servono: nessuna riga di questo file le
 * nomina ancora, quindi qui non compare nessuna dichiarazione.  Il link che
 * fallisce e' l'esito onesto; uno stub le farebbe passare per scritte.
 *
 * ==========================================================================
 * LE FUNZIONI DI QUESTA UNITA' DEFINITE PIU' AVANTI E USATE PIU' SU
 * ==========================================================================
 * L'ordine del testo di fabbrica e' un VINCOLO (clang emette le funzioni
 * nell'ordine in cui il sorgente le definisce), e fts_pram_write_init e' la
 * PRIMA dell'unita' pur chiamando quattro funzioni che stanno dopo di lei.
 * Le quattro vanno quindi dichiarate qui e definite al loro posto.
 */
static int fts_pram_init(struct i2c_client *client);
						/* 0xffffff8008a7d544, `t` nella
						 * mappa: static.  Due siti di
						 * chiamata dentro
						 * fts_pram_write_init,
						 * "94000218 bl"@0xffffff8008a7cce4
						 * e "94000087 bl"@0xffffff8008a7d328 */
int fts_fwupg_get_boot_state(struct i2c_client *client, int *fw_sts);
						/* 0xffffff8008a7d34c */
int fts_fwupg_reset_to_romboot(struct i2c_client *client);
						/* 0xffffff8008a7d624 */
bool fts_fwupg_check_state(struct i2c_client *client, int rstate);
						/* 0xffffff8008a7d764 */

/*
 * ===========================================================================
 * LE SETTE FUNZIONI `static` CHE LA MAPPA NON HA, PERCHE' DI FABBRICA SONO
 * TUTTE INCORPORATE DENTRO fts_pram_write_init
 * ===========================================================================
 * fts_pram_write_init misura 1792 byte e la mappa non ha NESSUN altro simbolo
 * fra 0xffffff8008a7cc4c e 0xffffff8008a7d34c.  Dentro quei 1792 byte ci sono
 * pero' TRE LIVELLI di messaggi d'errore incatenati, e la catena e' la prova
 * che le funzioni ci sono:
 *
 *   "\x013[FTS][Error]fwupgrade/func is null\n"@0xffffff800924f105    (livello 3)
 *   "\x013[FTS][Error]write pramboot fail\n"@0xffffff800924f028       (livello 2)
 *   "\x013[FTS][Error]pram write fail, ret=%d\n"@0xffffff800924e404   (livello 1)
 *
 * e i tre sono stampati UNO DOPO L'ALTRO sulla stessa caduta:
 *   "97dad915 bl"@0xffffff8008a7d080   printk del livello 3
 *   "97dad911 bl"@0xffffff8008a7d090   printk del livello 2
 *   "97dad9c7 bl"@0xffffff8008a7cdb8   printk del livello 1
 * Un solo corpo di funzione non stampa tre messaggi incatenati cosi': ognuno
 * e' l'`if (ret < 0) FTS_ERROR(...)` di un chiamante diverso.  La stessa cosa
 * succede sull'altro ramo, con
 * "\x013[FTS][Error]read pramboot ecc fail\n"@0xffffff800924f04b al posto di
 * "write pramboot fail", e su un terzo con
 * "\x013[FTS][Error]pram start fail\n"@0xffffff800924f0c1.
 *
 * LA SECONDA PROVA E' UN CONTROLLO DI LUNGHEZZA FATTO DUE VOLTE, con lo STESSO
 * messaggio e in due punti diversi:
 *   "71047e9f cmp"@0xffffff8008a7cd2c   pb_len contro 0x11f, poi
 *   "97dad9e5 bl"@0xffffff8008a7cd40    stampa "pramboot length(%d) fail"
 *   "51048288 sub"@0xffffff8008a7ce20   pb_len - 0x120, poi
 *   "6b09011f cmp"@0xffffff8008a7ce28   contro 0xfee1 (cioe' <= 0x10000), poi
 *   "97dad9a6 bl"@0xffffff8008a7ce3c    stampa LA STESSA stringa
 * Due controlli con lo stesso messaggio in una funzione sola non hanno senso;
 * in due funzioni, uno e' del chiamante e uno del chiamato.
 *
 * I NOMI DI QUESTE SETTE FUNZIONI E DEI LORO PARAMETRI SONO UNA SCELTA, NON
 * UNA MISURA (regola 4): una funzione incorporata non lascia simbolo.  Cio'
 * che e' MISURATO e' che esistono (la catena dei messaggi, i controlli
 * ripetuti, i punti di azzeramento delle variabili locali) e quali sono i
 * loro confini.  La misura che lo verifica e' che NON SI VEDONO: se clang non
 * le incorporasse, comparirebbero simboli `t` in piu' nell'oggetto e
 * fts_pram_write_init misurerebbe molto meno di 1792.
 */

/*
 * ---------------------------------------------------------------------------
 * fts_crc16_calc_host -- incorporata dentro fts_pram_write_init
 * ---------------------------------------------------------------------------
 * IL POLINOMIO E' 0x8408 e il verso e' verso destra:
 *   "5290810a mov"@0xffffff8008a7cf48   w10 = 0x8408, fuori dal ciclo
 *   "7200019f tst"@0xffffff8008a7cf6c   il bit 0
 *   "4a0a01ac eor"@0xffffff8008a7cf70   ^ 0x8408
 *   "1a8c01ac csel"@0xffffff8008a7cf74  scelta senza salto
 *
 * IL CICLO INTERNO E' SROTOLATO SEDICI VOLTE, e le sedici copie si alternano
 * fra `ubfx w13,w12,#1,#15` e `lsr w13,w12,#1`: e' la stessa espressione
 * `ecc >> 1` vista dal compilatore ora su un valore che sa a 16 bit e ora su
 * uno che sa gia' ridotto.  La prima e l'ultima:
 *   "53013d8d ubfx"@0xffffff8008a7cf68
 *   "53017d8d lsr"@0xffffff8008a7d058
 * Sedici e' il numero di bit di un `u16`: il ciclo interno e' `for (j = 0;
 * j < 16; j++)` con estremi costanti, ed e' per questo che clang lo srotola
 * per intero.
 *
 * DUE BYTE PER GIRO, IN ORDINE BIG-ENDIAN:
 *   "386c6aeb ldrb"@0xffffff8008a7cf50   pbuf[i]
 *   "b240018c orr"@0xffffff8008a7cf54    i | 1, cioe' i+1 perche' i e' PARI
 *   "386c6aec ldrb"@0xffffff8008a7cf58   pbuf[i+1]
 *   "33181d6c bfi"@0xffffff8008a7cf60    (pbuf[i] << 8) | pbuf[i+1]
 *   "11000929 add"@0xffffff8008a7cf5c    i += 2
 * L'`orr` al posto dell'`add` prova che clang sa che `i` e' pari, cioe' che
 * il passo del ciclo e' 2 e l'inizio e' 0.
 *
 * L'INDICE E LA LUNGHEZZA SONO A 16 BIT:
 *   "92403d2c and"@0xffffff8008a7cf4c    x12 = i & 0xffff
 *   "12003d2b and"@0xffffff8008a7d05c    w11 = i & 0xffff
 *   "6b34217f cmp"@0xffffff8008a7d06c    confronto con `length` esteso `uxth`
 *   "72003e9f tst"@0xffffff8008a7cf38    e la guardia d'ingresso e' su 16 bit
 * cioe' `u16 i` e `u16 length` (classe A4): con un `u32` non ci sarebbe ne'
 * la `and` ne' l'`uxth`.
 */
static u16 fts_crc16_calc_host(u8 *pbuf, u16 length)
{
	u16 ecc = 0;
	u16 i = 0;
	u16 j = 0;

	for (i = 0; i < length; i += 2) {
		ecc ^= ((pbuf[i] << 8) | (pbuf[i + 1]));
		for (j = 0; j < 16; j++) {
			if (ecc & 0x01)
				ecc = (u16)((ecc >> 1) ^ 0x8408);
			else
				ecc >>= 1;
		}
	}

	return ecc;
}

/*
 * ---------------------------------------------------------------------------
 * fts_pram_write_buf -- incorporata dentro fts_pram_write_init
 * ---------------------------------------------------------------------------
 * IL CONTROLLO DI NULLITA' E' SCRITTO CON `&&` DOVE VOLEVA `||`, ED E' UN
 * DIFETTO DELLA FABBRICA CHE SI RIPRODUCE (regola 7).  Il binario lo dice
 * senza ambiguita':
 *   "b5000088 cbnz"@0xffffff8008a7ce00   se il globale NON e' nullo, si SALTA
 *                                        il secondo controllo e si tira
 *                                        dritto
 *   "321b03e8 orr"@0xffffff8008a7ce04    altrimenti w8 = 0x20 ...
 *   "f9400108 ldr"@0xffffff8008a7ce08    ... e si legge da 0x20, cioe' da
 *                                        `((struct fts_upgrade *)NULL)->c32`
 *   "b4001368 cbz"@0xffffff8008a7ce0c    e solo QUEL valore decide l'errore
 * Con `||` il codice sarebbe `cbz globale -> errore` seguito da `ldr` e da un
 * secondo `cbz`: due salti all'errore, non uno solo sul ramo nullo.  La
 * lettura all'indirizzo assoluto 0x20 e' la firma di
 * `-fno-delete-null-pointer-checks`, che il kernel passa di serie: clang non
 * puo' cancellare il ramo, e lo compila come un accesso vero.
 *
 * LO STESSO DIFETTO C'E' UNA SECONDA VOLTA, in fts_pram_ecc_cal:
 *   "b5000088 cbnz"@0xffffff8008a7d0cc
 *   "321b03e9 orr"@0xffffff8008a7d0d0
 *   "f9400129 ldr"@0xffffff8008a7d0d4
 *   "b4000d69 cbz"@0xffffff8008a7d0d8
 *
 * IL COMANDO DI SCRITTURA IN PRAM E' 0xAE e l'intestazione e' di sei byte,
 * come per il flash:
 *   "528015c8 mov"@0xffffff8008a7ce50   w8 = 0xae
 *   "390083e8 strb"@0xffffff8008a7ce54  packet_buf[0] = 0xae
 *   "1100190c add"@0xffffff8008a7ced0   l'indice del dato e' j + 6
 *   "11001a82 add"@0xffffff8008a7cef8   la writelen e' packet_len + 6
 *
 * L'INDIRIZZO SCRITTO NELL'INTESTAZIONE E' L'OFFSET, NON UN INDIRIZZO BASE +
 * OFFSET: non c'e' nessun `saddr` da sommare, e le tre estrazioni partono
 * direttamente da `i`:
 *   "530b7ea9 lsr"@0xffffff8008a7ce88   i >> 11  = (i*32) >> 16
 *   "53037eaa lsr"@0xffffff8008a7ce8c   i >> 3   = (i*32) >> 8
 *   "531b6aa8 lsl"@0xffffff8008a7ce9c   i << 5   =  i*32
 *
 * LA SOMMA DI CONTROLLO E' DECISA DENTRO IL CICLO, NON FUORI, e questo e'
 * misurato (classe A2 al contrario): le due letture del campo stanno DENTRO
 * il ciclo dei byte e si ripetono a ogni giro,
 *   "f940112c ldr"@0xffffff8008a7cedc   upg->c32
 *   "b940218c ldr"@0xffffff8008a7cee0   upg->c32->c32
 * mentre il GLOBALE e' letto una volta sola per ogni pacchetto,
 *   "f9456709 ldr"@0xffffff8008a7cebc
 * ed e' fuori dal ciclo dei byte.  La differenza non e' casuale: il globale
 * non ha mai l'indirizzo preso, quindi il compilatore sa che la `strb` nel
 * buffer non lo puo' toccare e lo tira fuori dal ciclo; i due campi si
 * leggono attraverso un puntatore, e quella prova non la puo' fare.  Se il
 * sorgente avesse messo il campo in un locale prima del ciclo, le due `ldr`
 * sarebbero fuori come il globale.
 *
 * LO XOR E' SENZA SALTO, ma resta un `if`:
 *   "1a9f016b csel"@0xffffff8008a7cee8   il valore oppure zero
 *   "4a190179 eor"@0xffffff8008a7cef0    e poi lo xor incondizionato
 *
 * IL VALORE DI RITORNO E' LA SOMMA DI CONTROLLO, NON UN CODICE D'ESITO, e il
 * chiamante lo sa: dopo l'innesto non c'e' nessun `tbz w,#31` sul risultato
 * buono, perche' clang ha dimostrato che non e' mai negativo.
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]write pramboot to pram\n"@0xffffff800924f0e0
 *   "\x013[FTS][Error]fwupgrade/func is null\n"@0xffffff800924f105
 *   "\x016[FTS][Info]pramboot len=%d\n"@0xffffff800924f12b
 *   "\x013[FTS][Error]pramboot length(%d) fail\n"@0xffffff800924f000
 *   "\x013[FTS][Error]pramboot write data(%d) fail\n"@0xffffff800924f149
 */
static int fts_pram_write_buf(struct i2c_client *client, u8 *buf, u32 len)
{
	int ret = 0;
	u32 i = 0;
	u32 j = 0;
	u32 offset = 0;
	u32 remainder = 0;
	u32 packet_number = 0;
	u32 packet_len = 0;
	u8 packet_buf[FTS_FLASH_PACKET_LENGTH + FTS_CMD_WRITE_LEN] = { 0 };
	u8 ecc_tmp = 0;
	int ecc_in_host = 0;

	FTS_INFO("write pramboot to pram");
	if ((NULL == fts_g_a100ac8) && (NULL == fts_g_a100ac8->c32)) {
		FTS_ERROR("fwupgrade/func is null");
		return -EINVAL;
	}

	FTS_INFO("pramboot len=%d", len);
	if ((len < 0x120) || (len > 0x10000)) {
		FTS_ERROR("pramboot length(%d) fail", len);
		return -EINVAL;
	}

	packet_number = len / FTS_FLASH_PACKET_LENGTH;
	remainder = len % FTS_FLASH_PACKET_LENGTH;
	if (remainder > 0)
		packet_number++;
	packet_len = FTS_FLASH_PACKET_LENGTH;

	packet_buf[0] = 0xAE;
	for (i = 0; i < packet_number; i++) {
		offset = i * FTS_FLASH_PACKET_LENGTH;
		packet_buf[1] = (u8)((offset >> 16) & 0xFF);
		packet_buf[2] = (u8)((offset >> 8) & 0xFF);
		packet_buf[3] = (u8)(offset & 0xFF);

		if ((i == (packet_number - 1)) && remainder)
			packet_len = remainder;

		packet_buf[4] = (u8)((packet_len >> 8) & 0xFF);
		packet_buf[5] = (u8)(packet_len & 0xFF);

		for (j = 0; j < packet_len; j++) {
			packet_buf[FTS_CMD_WRITE_LEN + j] = buf[offset + j];
			if (0 == fts_g_a100ac8->c32->c32)
				ecc_tmp ^= packet_buf[FTS_CMD_WRITE_LEN + j];
		}

		ret = fts_i2c_write(client, packet_buf,
				    packet_len + FTS_CMD_WRITE_LEN);
		if (ret < 0) {
			FTS_ERROR("pramboot write data(%d) fail", i);
			return ret;
		}
	}

	if (0 == fts_g_a100ac8->c32->c32)
		ecc_in_host = (int)ecc_tmp;
	else
		ecc_in_host = (int)fts_crc16_calc_host(buf, len);

	return ecc_in_host;
}

/*
 * ---------------------------------------------------------------------------
 * fts_pram_ecc_cal_xor / fts_pram_ecc_cal_algo / fts_pram_ecc_cal
 * ---------------------------------------------------------------------------
 * IL MESSAGGIO "read out pramboot checksum" E' STAMPATO DA DUE SITI DIVERSI,
 * "97dad8f8 bl"@0xffffff8008a7d0f4 e "97dad8d1 bl"@0xffffff8008a7d190, che
 * caricano tutti e due 0x175 sulla stessa pagina
 * ("9105d400 add"@0xffffff8008a7d0ec e @0xffffff8008a7d188).  Una sola
 * stampa prima del bivio darebbe UN solo sito: sono due rami, e ognuno ha la
 * sua.
 *
 * IL BIVIO E' LO STESSO CAMPO CHE DECIDE LO XOR DEL BUFFER:
 *   "b9402108 ldr"@0xffffff8008a7d0e0   upg->c32->c32
 *   "34000508 cbz"@0xffffff8008a7d0e4   se e' zero -> il ramo `xor`
 *
 * IL RAMO SEMPLICE legge un registro solo:
 *   "52801981 mov"@0xffffff8008a7d198   w1 = 0xcc, l'indirizzo di registro
 *   "94000d1d bl"@0xffffff8008a7d1a0    fts_i2c_read_reg
 *   "394083f6 ldrb"@0xffffff8008a7d1a8  e il valore letto E' il risultato
 *
 * IL RAMO CON L'ALGORITMO manda un comando di sette byte, aspetta e legge due
 * byte:
 *   "52801988 mov"@0xffffff8008a7d0f8   w8 = 0xcc, il comando di calcolo
 *   "b90023e8 str"@0xffffff8008a7d110   cmd[0..3] = 0xcc,0,0,0 -- una `str` a
 *                                       32 bit, cioe' i tre byte
 *                                       dell'indirizzo sono ZERO nel sorgente
 *   "32000be2 orr"@0xffffff8008a7d108   la writelen e' 7
 *   "528019c8 mov"@0xffffff8008a7d128   w8 = 0xce, il comando di attesa
 *   "71018e9f cmp"@0xffffff8008a7d164   il giro numero 0x63 = 99
 *   "528019a8 mov"@0xffffff8008a7d2b8   w8 = 0xcd, il comando di lettura
 *   "321f03e4 orr"@0xffffff8008a7d2c8   e la readlen e' 2
 *   "33181d16 bfi"@0xffffff8008a7d2e4   il risultato e' (val[0]<<8)|val[1]
 *
 * NEL CICLO D'ATTESA LA `msleep` VIENE PRIMA DELLA LETTURA, non dopo: il
 * bersaglio del salto "54fffe69 b.ls"@0xffffff8008a7d168 e'
 * 0xffffff8008a7d134, cioe' la "320003e0 orr"@0xffffff8008a7d134 che prepara
 * l'argomento della msleep.
 *
 * IL RITORNO NEGATIVO DELLA LETTURA FINALE NON E' STAMPATO QUI: i due rami
 * d'errore, "37f80a60 tbnz"@0xffffff8008a7d1a4 e
 * "37f800c0 tbnz"@0xffffff8008a7d2d8, finiscono tutti e due sullo stesso
 * blocco 0xffffff8008a7d2f0 che stampa "read pramboot ecc fail", ed e' il
 * messaggio del CHIAMANTE (fts_pram_write_remap), non di questa funzione.
 *
 * I MESSAGGI DI QUESTE FUNZIONI:
 *   "\x013[FTS][Error]fwupgrade/func is null\n"@0xffffff800924f105
 *   "\x016[FTS][Info]read out pramboot checksum\n"@0xffffff800924f175
 *   "\x013[FTS][Error]write pramboot ecc cal cmd fail\n"@0xffffff800924f19e
 *   "\x013[FTS][Error]ecc_finish read cmd fail\n"@0xffffff800924f1cd
 *   "\x013[FTS][Error]wait ecc finish fail\n"@0xffffff800924f1f5
 */
static int fts_pram_ecc_cal_algo(struct i2c_client *client, u32 saddr, u32 len)
{
	int ret = 0;
	u32 i = 0;
	u8 val[2] = { 0 };
	u8 cmd[7] = { 0 };

	FTS_INFO("read out pramboot checksum");

	cmd[0] = 0xCC;
	cmd[1] = (u8)((saddr >> 16) & 0xFF);
	cmd[2] = (u8)((saddr >> 8) & 0xFF);
	cmd[3] = (u8)(saddr & 0xFF);
	cmd[4] = (u8)((len >> 16) & 0xFF);
	cmd[5] = (u8)((len >> 8) & 0xFF);
	cmd[6] = (u8)(len & 0xFF);
	ret = fts_i2c_write(client, cmd, 7);
	if (ret < 0) {
		FTS_ERROR("write pramboot ecc cal cmd fail");
		return ret;
	}

	cmd[0] = 0xCE;
	for (i = 0; i < 100; i++) {
		msleep(1);
		ret = fts_i2c_read(client, cmd, 1, val, 1);
		if (ret < 0) {
			FTS_ERROR("ecc_finish read cmd fail");
			return ret;
		}

		if (0 == val[0])
			break;
	}
	if (i >= 100) {
		FTS_ERROR("wait ecc finish fail");
		return -EIO;
	}

	cmd[0] = 0xCD;
	ret = fts_i2c_read(client, cmd, 1, val, 2);
	if (ret < 0) {
		FTS_ERROR("read pramboot ecc fail");
		return ret;
	}

	return (int)((val[0] << 8) + val[1]);
}

static int fts_pram_ecc_cal_xor(struct i2c_client *client)
{
	int ret = 0;
	u8 reg_val = 0;

	FTS_INFO("read out pramboot checksum");

	ret = fts_i2c_read_reg(client, 0xCC, &reg_val);
	if (ret < 0) {
		FTS_ERROR("read pramboot ecc fail");
		return ret;
	}

	return (int)reg_val;
}

static int fts_pram_ecc_cal(struct i2c_client *client, u32 saddr, u32 len)
{
	int ret = 0;

	if ((NULL == fts_g_a100ac8) && (NULL == fts_g_a100ac8->c32)) {
		FTS_ERROR("fwupgrade/func is null");
		return -EINVAL;
	}

	if (0 != fts_g_a100ac8->c32->c32)
		ret = fts_pram_ecc_cal_algo(client, saddr, len);
	else
		ret = fts_pram_ecc_cal_xor(client);

	return ret;
}

/*
 * ---------------------------------------------------------------------------
 * fts_pram_start -- incorporata dentro fts_pram_write_init
 * ---------------------------------------------------------------------------
 * IL COMANDO E' 8 ED E' ASSEGNATO PRIMA DELLA STAMPA:
 *   "321d03e8 orr"@0xffffff8008a7d1d0   w8 = 8
 *   "390083e8 strb"@0xffffff8008a7d1d8  cmd = 8
 *   "97dad8be bl"@0xffffff8008a7d1dc    e SOLO DOPO il printk
 * cioe' e' l'inizializzatore di una dichiarazione, non un'assegnazione.
 *
 * IL RITARDO E' 10, con msleep e non mdelay:
 *   "52800140 mov"@0xffffff8008a7d1f4   w0 = 0xa
 *   "97db5687 bl"@0xffffff8008a7d1f8    msleep
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]remap to start pramboot\n"@0xffffff800924f219
 *   "\x013[FTS][Error]write start pram cmd fail\n"@0xffffff800924f23f
 */
static int fts_pram_start(struct i2c_client *client)
{
	u8 cmd = 0x08;
	int ret = 0;

	FTS_INFO("remap to start pramboot");

	ret = fts_i2c_write(client, &cmd, 1);
	if (ret < 0) {
		FTS_ERROR("write start pram cmd fail");
		return ret;
	}

	msleep(10);

	return 0;
}

/*
 * ---------------------------------------------------------------------------
 * fts_pram_write_remap -- incorporata dentro fts_pram_write_init
 * ---------------------------------------------------------------------------
 * IL GLOBALE E' RILETTO QUI, e non e' quello che il chiamante ha gia' in
 * mano: la "f9456714 ldr"@0xffffff8008a7cd04 sta PRIMA della
 * "97dad9f1 bl"@0xffffff8008a7cd10 (il printk di "write pram and remap"), cioe'
 * e' l'inizializzatore di una dichiarazione di questa funzione.  Se il valore
 * fosse arrivato dal chiamante non ci sarebbe nessuna `ldr` li'.
 *
 * QUI IL CONTROLLO DI NULLITA' E' SCRITTO BENE, con `||`, e sono TRE:
 *   "b4000454 cbz"@0xffffff8008a7cd14   upg
 *   "b4000408 cbz"@0xffffff8008a7cd1c   upg->c32
 *   "b40003d7 cbz"@0xffffff8008a7cd24   upg->c32->c72
 * Tre salti allo stesso errore, uno per condizione: e' la forma di `||`.  E'
 * il confronto con fts_pram_write_buf, dove il salto e' UNO SOLO, che rende
 * misurabile il difetto dell'altra (vedi il cappello di quella).
 *
 * IL CONFRONTO FRA LE DUE SOMME NON E' `<` NE' `>`, E' UNA DISUGUAGLIANZA:
 *   "6b1602bf cmp"@0xffffff8008a7d1c4
 *   "54000441 b.ne"@0xffffff8008a7d1c8
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]write pram and remap\n"@0xffffff800924efb5
 *   "\x013[FTS][Error]upgrade/pramboot is null\n"@0xffffff800924efd8
 *   "\x013[FTS][Error]pramboot length(%d) fail\n"@0xffffff800924f000
 *   "\x013[FTS][Error]write pramboot fail\n"@0xffffff800924f028
 *   "\x013[FTS][Error]read pramboot ecc fail\n"@0xffffff800924f04b
 *   "\x016[FTS][Info]pram ecc in tp:%x, host:%x\n"@0xffffff800924f071
 *   "\x013[FTS][Error]pramboot ecc check fail\n"@0xffffff800924f09a
 *   "\x013[FTS][Error]pram start fail\n"@0xffffff800924f0c1
 */
static int fts_pram_write_remap(struct i2c_client *client)
{
	int ret = 0;
	int ecc_in_host = 0;
	int ecc_in_tp = 0;
	u8 *pb_buf = NULL;
	u32 pb_len = 0;
	struct fts_upgrade *upg = fts_g_a100ac8;

	FTS_INFO("write pram and remap");
	if ((NULL == upg) || (NULL == upg->c32) || (NULL == upg->c32->c72)) {
		FTS_ERROR("upgrade/pramboot is null");
		return -EINVAL;
	}

	pb_buf = upg->c32->c72;
	pb_len = upg->c32->c80;
	if (pb_len < 0x120) {
		FTS_ERROR("pramboot length(%d) fail", pb_len);
		return -EINVAL;
	}

	ecc_in_host = fts_pram_write_buf(client, pb_buf, pb_len);
	if (ecc_in_host < 0) {
		FTS_ERROR("write pramboot fail");
		return ecc_in_host;
	}

	ecc_in_tp = fts_pram_ecc_cal(client, 0, pb_len);
	if (ecc_in_tp < 0) {
		FTS_ERROR("read pramboot ecc fail");
		return ecc_in_tp;
	}

	FTS_INFO("pram ecc in tp:%x, host:%x", ecc_in_tp, ecc_in_host);
	if (ecc_in_host != ecc_in_tp) {
		FTS_ERROR("pramboot ecc check fail");
		return -EIO;
	}

	ret = fts_pram_start(client);
	if (ret < 0) {
		FTS_ERROR("pram start fail");
		return ret;
	}

	return 0;
}

/*
 * ===========================================================================
 * fts_pram_write_init @ 0xffffff8008a7cc4c, 1792 byte -- LA PRIMA FUNZIONE
 * DELL'UNITA'
 * ===========================================================================
 * FIRMA: un solo parametro, x0 -> x19 con
 * "aa0003f3 mov"@0xffffff8008a7cc78.
 *
 * IL GLOBALE E LO STATO SONO INIZIALIZZATI PRIMA DEL BANNER:
 *   "f9456714 ldr"@0xffffff8008a7cc84   x20 = fts_g_a100ac8
 *   "b9001bff str"@0xffffff8008a7cc8c   fw_sts = 0, QUATTRO byte a sp+24
 *   "97dada11 bl"@0xffffff8008a7cc90    e solo dopo il printk
 *
 * QUI IL CONTROLLO E' `||` -- due salti allo stesso errore:
 *   "b40005b4 cbz"@0xffffff8008a7cc94   upg
 *   "b4000568 cbz"@0xffffff8008a7cc9c   upg->c32
 * e il messaggio e' "upgrade/func is null", lo STESSO di
 * fts_fwupg_enter_into_boot (una sola stringa deduplicata, 0x2bf).
 *
 * IL TERZO CONTROLLO E' SUL CAMPO +65, cioe' lo stesso che in
 * fts_fwupg_enter_into_boot sceglie fra pramboot e bootloader:
 *   "39410508 ldrb"@0xffffff8008a7cca0
 *   "34000768 cbz"@0xffffff8008a7cca4   se e' zero -> "ic not support pram"
 *
 * I TRE ESITI DELLO STATO DI BOOT SONO UN `if` ANNIDATO, e l'ordine dei due
 * confronti e' 2 prima e 3 poi:
 *   "7100091f cmp"@0xffffff8008a7ccc4   fw_sts == 2 (romboot) -> non fa NIENTE
 *   "71000d1f cmp"@0xffffff8008a7cccc   fw_sts == 3 (pramboot) -> fts_pram_init
 *   "540000e1 b.ne"@0xffffff8008a7ccd0  qualunque altro -> solo il reset
 * cioe' `if (2 != fw_sts) { if (3 == fw_sts) {...} ...reset... }`.  IL VALORE
 * DI RITORNO DI fts_fwupg_get_boot_state E' IGNORATO: dopo la
 * "940001a4 bl"@0xffffff8008a7ccbc si rilegge subito la locale con
 * "b9401be8 ldr"@0xffffff8008a7ccc0.
 *
 * fts_pram_init HA DUE SITI DI CHIAMATA, ed e' per questo che la si vede
 * ancora come simbolo `t` invece che incorporata:
 *   "94000218 bl"@0xffffff8008a7cce4   il ramo "tp is in pramboot"
 *   "94000087 bl"@0xffffff8008a7d328   la coda, dopo il remap
 *
 * LA CODA E' fts_fwupg_check_state(client, 3) INCORPORATA -- le stesse cinque
 * costanti di quella funzione, col `rstate` piegato a 3:
 *   "b90023ff str"@0xffffff8008a7d20c   fw_sts = 0, FUORI dal ciclo (il
 *                                       bersaglio e' 0xffffff8008a7d210)
 *   "9400004d bl"@0xffffff8008a7d218    fts_fwupg_get_boot_state
 *   "71000d1f cmp"@0xffffff8008a7d220   fw_sts == 3
 *   "52800280 mov"@0xffffff8008a7d228   msleep(20)
 *   "71007a9f cmp"@0xffffff8008a7d234   il giro numero 30
 *   "54fffec3 b.cc"@0xffffff8008a7d238  contatore unsigned
 * e l'elenco delle chiamate conferma che il simbolo non c'e':
 *   chiama : printk fts_fwupg_get_boot_state fts_pram_init
 *            fts_fwupg_reset_to_romboot fts_i2c_write msleep fts_i2c_read
 *            fts_i2c_read_reg __stack_chk_fail
 * -- niente fts_fwupg_check_state, niente fts_pram_write_remap.
 *
 * I DUE CODICI D'ERRORE:
 *   "128002b6 mov"@0xffffff8008a7cd54   w22 = -22 = -EINVAL
 *   "321d7bf6 orr"@0xffffff8008a7d248   w22 = -5  = -EIO
 *
 * I MESSAGGI DI QUESTA FUNZIONE, in ordine di indirizzo nel `.rodata` -- ed e'
 * ESATTAMENTE l'ordine in cui il sorgente li scrive:
 *   "\x016[FTS][Info]**********pram write and init**********\n"@0xffffff800924e289
 *   "\x013[FTS][Error]upgrade/func is null\n"@0xffffff800924e2bf
 *   "\x013[FTS][Error]ic not support pram\n"@0xffffff800924e2e3
 *   "[FTS]check whether tp is in romboot or not \n"@0xffffff800924e306
 *   "\x016[FTS][Info]tp is in pramboot, need send reset cmd before upgrade\n"@0xffffff800924e333
 *   "\x013[FTS][Error]pramboot(before) init fail\n"@0xffffff800924e377
 *   "\x016[FTS][Info]tp isn't in romboot, need send reset to romboot\n"@0xffffff800924e3a1
 *   "\x013[FTS][Error]reset to romboot fail\n"@0xffffff800924e3df
 *   "\x013[FTS][Error]pram write fail, ret=%d\n"@0xffffff800924e404
 *   "[FTS]after write pramboot, confirm run in pramboot\n"@0xffffff800924e42b
 *   "\x013[FTS][Error]not in pramboot\n"@0xffffff800924e45f
 *   "\x013[FTS][Error]pramboot init fail\n"@0xffffff800924e47e
 * IL QUARTO E IL DECIMO SONO SENZA LIVELLO KERN e col prefisso "[FTS]" nudo:
 * sono la terza macro.  E il quarto ha uno SPAZIO in coda, prima del "\n",
 * che si riproduce (regola 7).
 */
int fts_pram_write_init(struct i2c_client *client)
{
	int ret = 0;
	bool state = false;
	int fw_sts = 0;
	struct fts_upgrade *upg = fts_g_a100ac8;

	FTS_INFO("**********pram write and init**********");
	if ((NULL == upg) || (NULL == upg->c32)) {
		FTS_ERROR("upgrade/func is null");
		return -EINVAL;
	}

	if (!upg->c32->c65) {
		FTS_ERROR("ic not support pram");
		return -EINVAL;
	}

	FTS_DEBUG("check whether tp is in romboot or not ");
	fts_fwupg_get_boot_state(client, &fw_sts);
	if (FTS_RUN_IN_ROM != fw_sts) {
		if (FTS_RUN_IN_PRAM == fw_sts) {
			FTS_INFO("tp is in pramboot, need send reset cmd before upgrade");
			ret = fts_pram_init(client);
			if (ret < 0) {
				FTS_ERROR("pramboot(before) init fail");
				return ret;
			}
		}

		FTS_INFO("tp isn't in romboot, need send reset to romboot");
		ret = fts_fwupg_reset_to_romboot(client);
		if (ret < 0) {
			FTS_ERROR("reset to romboot fail");
			return ret;
		}
	}

	ret = fts_pram_write_remap(client);
	if (ret < 0) {
		FTS_ERROR("pram write fail, ret=%d", ret);
		return ret;
	}

	FTS_DEBUG("after write pramboot, confirm run in pramboot");
	state = fts_fwupg_check_state(client, FTS_RUN_IN_PRAM);
	if (!state) {
		FTS_ERROR("not in pramboot");
		return -EIO;
	}

	ret = fts_pram_init(client);
	if (ret < 0) {
		FTS_ERROR("pramboot init fail");
		return ret;
	}

	return 0;
}

/*
 * ===========================================================================
 * fts_fwupg_get_boot_state @ 0xffffff8008a7d34c, 504 byte
 * ===========================================================================
 * FIRMA, letta dai registri d'ingresso:
 *   x0 client -- "aa0003f5 mov"@0xffffff8008a7d37c -> x21, passato a
 *                fts_i2c_hid2std, fts_i2c_write e fts_i2c_read
 *   x1 fw_sts -- "aa0103f3 mov"@0xffffff8008a7d3b4 -> x19, confrontato con
 *                NULL da "b40005f3 cbz"@0xffffff8008a7d3bc e scritto a 32 bit
 *                da "b9000268 str"@0xffffff8008a7d538: e' un `int *`
 *
 * I SEI BYTE DI fts_data SONO COPIATI IN LOCALI PRIMA DELLA PRIMA STAMPA.
 * Non e' una scelta di stile: e' l'unica lettura compatibile col binario.
 * Le sei "ldrb" stanno a 0xffffff8008a7d394..0xffffff8008a7d3a8, cioe' PRIMA
 * della "97dad847 bl"@0xffffff8008a7d3b8 (printk), e i valori restano nei
 * registri callee-saved w22..w27 attraverso QUATTRO chiamate fino ai confronti
 * di 0xffffff8008a7d450 in poi.  Che clang non li tenga da solo lo prova la
 * riga accanto: `fts_data` stesso viene RILETTO dopo la msleep,
 *   "f9456f88 ldr"@0xffffff8008a7d384   (prima)
 *   "f9456f88 ldr"@0xffffff8008a7d400   (dopo la msleep)
 * perche' printk e msleep sono chiamate opache e potrebbero cambiare un
 * globale.  Se anche i sei byte fossero stati letti come `fts_data->c34` al
 * momento del confronto, sarebbero stati riletti allo stesso modo.  E' la
 * classe A2/A5 al contrario.
 *
 * L'ORDINE DEI TRE CONTROLLI NULL e' quello del binario e non quello del
 * messaggio: fw_sts, poi upgrade, poi upgrade->func.
 *   "b40005f3 cbz"@0xffffff8008a7d3bc   fw_sts
 *   "b40005d4 cbz"@0xffffff8008a7d3c0   upgrade
 *   "b4000588 cbz"@0xffffff8008a7d3c8   upgrade->c32
 * Il messaggio li elenca nell'ordine inverso
 * ("upgrade/func/fw_sts is null"), ed e' un difetto di fabbrica che si
 * riproduce: il messaggio, non l'ordine.
 *
 * LA SCELTA FRA writelen 4 E writelen 1 e' un `csinc`, cioe' una selezione
 * senza salto, e IL VERSO NON E' QUELLO CHE SEMBRA:
 *   "7100011f cmp"@0xffffff8008a7d420   fts_data->c16 == 0 ?
 *   "321e03e8 orr"@0xffffff8008a7d424   w8 = 4
 *   "1a9f0502 csinc"@0xffffff8008a7d428 csinc w2, w8, wzr, eq
 * `csinc Rd,Rn,Rm,cond` vale Rn se la condizione e' VERA e Rm+1 se e' falsa.
 * Con cond = `eq`, cioe' campo == 0, si ha w2 = w8 = 4; con campo != 0 si ha
 * w2 = wzr+1 = 1.  Quindi la writelen e' 1 quando il campo e' NON-zero, e 4
 * quando e' zero: `fts_data->c16 ? 1 : 4`.
 *
 * QUESTO E' UN DIFETTO CHE LA DIMENSIONE NON DENUNCIA, e questo lotto ce
 * l'aveva.  La prima stesura scriveva `? 4 : 1`, misurava 504 byte esatti con
 * TUTTI E DUE i compilatori e 126 mnemonici su 126 -- e sbagliava.  L'ha
 * trovato il confronto per CODIFICA, che e' l'unica misura che lo vede:
 *   $ ./venv/bin/python3 f3cbis_codifica.py fts_fwupg_get_boot_state \
 *       0xffffff8008a7d34c out-ft8719-fab/.../focaltech_flash.o
 *   istruzioni: fabbrica 126, nostro 126
 *   stesso mnemonico in posizione: 126 su 126
 *   codifiche diverse: 39
 *     di cui siti di rilocazione o chiamata locale: 38
 *     RESIDUO: 1
 *        55  F ffffff8008a7d428 1a9f0502 csinc  w2, w8, wzr, eq          | N 1a9f1502 csinc  w2, w8, wzr, ne
 * Un solo bit di condizione, dimensione identica, mnemonico identico.
 *
 * IL VALORE DI RITORNO DI fts_i2c_hid2std E' IGNORATO: la
 * "94000ca4 bl"@0xffffff8008a7d3d8 e' seguita subito da
 * "52954aa8 mov"@0xffffff8008a7d3dc, che sovrascrive w0 senza leggerlo.
 *
 * I MESSAGGI DI QUESTA FUNZIONE, per intero come stanno nel binario --
 * livello KERN, prefisso e ritorno a capo compresi:
 *   "\x016[FTS][Info]**********read boot id**********\n"@0xffffff800924e4d6
 *   "\x013[FTS][Error]upgrade/func/fw_sts is null\n"@0xffffff800924e505
 *   "\x013[FTS][Error]write 55 aa cmd fail\n"@0xffffff800924e530
 *   "\x013[FTS][Error]write 90 cmd fail\n"@0xffffff800924e554
 *   "\x016[FTS][Info]read boot id:0x%02x%02x\n"@0xffffff800924e575
 *   "\x016[FTS][Info]tp run in romboot\n"@0xffffff800924e59b
 *   "\x016[FTS][Info]tp run in pramboot\n"@0xffffff800924e5bb
 *   "\x016[FTS][Info]tp run in bootloader\n"@0xffffff800924e5dc
 * Stanno tutti e otto di seguito nel `.rodata`, in quest'ordine, e l'ordine
 * dei loro indirizzi e' l'ordine in cui il sorgente li scrive.
 *
 * L'ULTIMO RAMO NON SCRIVE NIENTE in *fw_sts: se l'id non combacia con
 * nessuno dei tre, "2a1f03f4 mov"@0xffffff8008a7d514 mette ret = 0 e
 * "54fffb81 b.ne"@0xffffff8008a7d518 salta direttamente all'epilogo, oltre la
 * "b9000268 str"@0xffffff8008a7d538.  Si ritorna 0 lasciando *fw_sts com'era:
 * e' un difetto di fabbrica e si riproduce.
 */
int fts_fwupg_get_boot_state(struct i2c_client *client, int *fw_sts)
{
	int ret = 0;
	u8 cmd[4] = { 0 };
	u8 id[2] = { 0 };
	struct fts_upgrade *upg = fts_g_a100ac8;
	u8 rom_h = fts_data->c34;
	u8 rom_l = fts_data->c35;
	u8 pram_h = fts_data->c36;
	u8 pram_l = fts_data->c37;
	u8 boot_h = fts_data->c38;
	u8 boot_l = fts_data->c39;

	FTS_INFO("**********read boot id**********");
	if ((NULL == fw_sts) || (NULL == upg) || (NULL == upg->c32)) {
		FTS_ERROR("upgrade/func/fw_sts is null");
		return -EINVAL;
	}

	if (upg->c32->c64)
		fts_i2c_hid2std(client);

	cmd[0] = FTS_CMD_START1;
	cmd[1] = FTS_CMD_START2;
	ret = fts_i2c_write(client, cmd, 2);
	if (ret < 0) {
		FTS_ERROR("write 55 aa cmd fail");
		return ret;
	}

	msleep(FTS_CMD_START_DELAY);

	cmd[0] = FTS_CMD_READ_ID;
	cmd[1] = 0x00;
	cmd[2] = 0x00;
	cmd[3] = 0x00;
	ret = fts_i2c_read(client, cmd, fts_data->c16 ? 1 : 4, id, 2);
	if (ret < 0) {
		FTS_ERROR("write 90 cmd fail");
		return ret;
	}

	FTS_INFO("read boot id:0x%02x%02x", id[0], id[1]);
	if ((id[0] == rom_h) && (id[1] == rom_l)) {
		FTS_INFO("tp run in romboot");
		*fw_sts = FTS_RUN_IN_ROM;
	} else if ((id[0] == pram_h) && (id[1] == pram_l)) {
		FTS_INFO("tp run in pramboot");
		*fw_sts = FTS_RUN_IN_PRAM;
	} else if ((id[0] == boot_h) && (id[1] == boot_l)) {
		FTS_INFO("tp run in bootloader");
		*fw_sts = FTS_RUN_IN_BOOTLOADER;
	}

	return 0;
}

/*
 * ===========================================================================
 * fts_pram_init @ 0xffffff8008a7d544, 224 byte -- L'UNICA `static` DELL'UNITA'
 * ===========================================================================
 * NELLA MAPPA E' `t`, NON `T`, ed e' l'unica delle diciassette:
 *   $ grep -n fts_pram_init oracolo/stock.map
 *   38223:ffffff8008a7d544 t fts_pram_init
 * quindi di fabbrica e' `static`.  Ha UN SOLO chiamante, fts_pram_write_init,
 * e nonostante questo clang NON l'ha incorporata: il simbolo c'e' e il corpo
 * e' fuori linea.  Non so dire perche' (il compilatore di solito incorpora
 * l'ultima chiamata a una static), e non lo invento: e' un FATTO misurato,
 * non spiegato.  Se il nostro oggetto la incorporasse, il simbolo sparirebbe
 * e la misura lo direbbe.
 *
 * FIRMA: un solo parametro, x0 -> x19 con
 * "aa0003f3 mov"@0xffffff8008a7d560.
 *
 * DUE OGGETTI IN PILA, e le tre istruzioni di azzeramento li separano:
 *   "390003ff strb"@0xffffff8008a7d570   UN byte a sp+0     -> reg_val = 0
 *   "39001bff strb"@0xffffff8008a7d574   UN byte a sp+6   \
 *   "79000bff strh"@0xffffff8008a7d578   DUE byte a sp+4  / -> wbuf[3] = {0}
 * cioe' un byte da solo e tre byte contigui.
 *
 * IL REGISTRO 0x05 E' SCRITTO DUE VOLTE, e la seconda non e' ridondante per
 * il compilatore perche' in mezzo c'e' una chiamata che ha visto
 * l'indirizzo:
 *   "528000b5 mov"@0xffffff8008a7d580   w21 = 5, calcolato una volta sola
 *   "390013f5 strb"@0xffffff8008a7d598   wbuf[0] = 5, prima della lettura
 *   "390013f5 strb"@0xffffff8008a7d5b4   wbuf[0] = 5, prima della scrittura
 * Lo stesso vale per il terzo byte: la "39001bff strb"@0xffffff8008a7d5bc
 * riazzera sp+6 che era gia' azzerato dalla dichiarazione.  Sono DUE
 * assegnazioni del sorgente, non una.
 *
 * LA LETTURA E LA SCRITTURA, dai registri dei due siti di chiamata:
 *   "320003e2 orr"@0xffffff8008a7d58c   writelen 1  \  fts_i2c_read(client,
 *   "320003e4 orr"@0xffffff8008a7d590   readlen  1  /   wbuf,1, &reg_val,1)
 *   "910003e3 mov"@0xffffff8008a7d588   x3 = &reg_val (sp+0)
 *   "320007e2 orr"@0xffffff8008a7d5ac   writelen 3 -> fts_i2c_write(...,3)
 * e il byte letto finisce in wbuf[1]:
 *   "394003e8 ldrb"@0xffffff8008a7d5a4   w8 = reg_val
 *   "390017e8 strb"@0xffffff8008a7d5b8   wbuf[1] = w8
 *
 * NON DO' UN NOME AL 5 (regola 5): il binario lo scrive come numero e il
 * messaggio lo chiama "flash type" senza dire che quello sia il suo indirizzo
 * di registro.  Resta un 5.
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]pramboot initialization\n"@0xffffff800924ef46
 *   "\x013[FTS][Error]read flash type fail\n"@0xffffff800924ef6c
 *   "\x013[FTS][Error]write flash type fail\n"@0xffffff800924ef90
 */
static int fts_pram_init(struct i2c_client *client)
{
	int ret = 0;
	u8 reg_val = 0;
	u8 wbuf[3] = { 0 };

	FTS_INFO("pramboot initialization");

	wbuf[0] = 0x05;
	ret = fts_i2c_read(client, wbuf, 1, &reg_val, 1);
	if (ret < 0) {
		FTS_ERROR("read flash type fail");
		return ret;
	}

	wbuf[0] = 0x05;
	wbuf[1] = reg_val;
	wbuf[2] = 0x00;
	ret = fts_i2c_write(client, wbuf, 3);
	if (ret < 0) {
		FTS_ERROR("write flash type fail");
		return ret;
	}

	return 0;
}

/*
 * ===========================================================================
 * fts_fwupg_reset_to_romboot @ 0xffffff8008a7d624, 320 byte
 * ===========================================================================
 * FIRMA: un solo parametro, x0 -> x19 con
 * "aa0003f3 mov"@0xffffff8008a7d648.
 *
 * NESSUN MESSAGGIO IN TESTA: fra il prologo e la
 * "94000b8f bl"@0xffffff8008a7d658 verso fts_i2c_write non c'e' nessuna
 * chiamata a printk.  E' il contrario di fts_fwupg_reset_in_boot, che il
 * banner ce l'ha.
 *
 * IL COMANDO E' LO STESSO 7 DI fts_fwupg_reset_in_boot, e anche qui e'
 * assegnato dalla dichiarazione, prima di tutto:
 *   "32000be9 orr"@0xffffff8008a7d63c   w9 = 7
 *   "390013e9 strb"@0xffffff8008a7d650  cmd = 7 a sp+4
 *   "b90003ff str"@0xffffff8008a7d654   state = 0 a sp+0, QUATTRO byte
 * La `str` a 32 bit dice che lo stato e' un `int`, non un `u8`.
 *
 * I DUE RITARDI SONO `mdelay`, NON `msleep`, E LE DUE FORME SONO DIVERSE
 * PERCHE' LA MACRO DEL KERNEL SI SPACCA SU MAX_UDELAY_MS = 5:
 *
 *   mdelay(10) -- 10 > 5, quindi la macro diventa
 *   `{unsigned long __ms=10; while (__ms--) udelay(1000);}` e clang SROTOLA i
 *   dieci giri.  Nel binario ci sono DIECI chiamate identiche di fila:
 *     "52912b00 mov"@0xffffff8008a7d660 + "72a00820 movk"@0xffffff8008a7d664
 *     "940f6903 bl"@0xffffff8008a7d668   __const_udelay
 *     [... altre nove coppie identiche, fino a ...]
 *     "52912b00 mov"@0xffffff8008a7d6cc + "72a00820 movk"@0xffffff8008a7d6d0
 *     "940f68e8 bl"@0xffffff8008a7d6d4
 *   e 0x418958 = 4295000 = 1000 * 0x10c7, cioe' udelay(1000).
 *
 *   mdelay(5) -- 5 <= 5, quindi la macro diventa `udelay(5000)` e resta UNA
 *   chiamata sola con l'immediato moltiplicato:
 *     "5295d700 mov"@0xffffff8008a7d6f4 + "72a028e0 movk"@0xffffff8008a7d6f8
 *     "940f68de bl"@0xffffff8008a7d6fc
 *   e 0x147aeb8 = 21475000 = 5000 * 0x10c7.
 *
 *   E' la prova che il sorgente scrive `mdelay` e non un ciclo a mano: due
 *   forme diverse per lo stesso nome, decise dal valore della costante.
 *
 * IL CONTATORE E' UNSIGNED (classe A3):
 *   "71007a9f cmp"@0xffffff8008a7d704   il giro numero 0x1e = 30
 *   "54fffea3 b.cc"@0xffffff8008a7d708  salto SENZA segno
 * Con un `int i` clang emette `b.lt`.
 *
 * LO STATO ATTESO E' 2, cioe' il valore che fts_fwupg_get_boot_state scrive
 * dopo "tp run in romboot":
 *   "7100091f cmp"@0xffffff8008a7d6ec
 * e IL VALORE DI RITORNO DI fts_fwupg_get_boot_state E' IGNORATO: dopo la
 * "97ffff1a bl"@0xffffff8008a7d6e4 la prima istruzione e' la
 * "b94003e8 ldr"@0xffffff8008a7d6e8, che rilegge la locale senza guardare w0.
 *
 * IL CODICE D'ERRORE E' -EIO:
 *   "321d7bf4 orr"@0xffffff8008a7d718   w20 = 0xfffffffb = -5
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x013[FTS][Error]pram/rom/bootloader reset cmd write fail\n"@0xffffff800924e627
 *   "\x013[FTS][Error]reset to romboot fail\n"@0xffffff800924e3df
 * Il primo e' lo STESSO di fts_fwupg_reset_in_boot: una sola stringa
 * deduplicata, non due -- tutte e due le funzioni caricano 0x627 sulla stessa
 * pagina ("91189c00 add"@0xffffff8008a7d898 e @0xffffff8008a7d728).
 */
int fts_fwupg_reset_to_romboot(struct i2c_client *client)
{
	int ret = 0;
	u32 i = 0;
	u8 cmd = FTS_REG_RESET_FW;
	int state = 0;

	ret = fts_i2c_write(client, &cmd, 1);
	if (ret < 0) {
		FTS_ERROR("pram/rom/bootloader reset cmd write fail");
		return ret;
	}
	mdelay(10);

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		fts_fwupg_get_boot_state(client, &state);
		if (FTS_RUN_IN_ROM == state)
			break;

		mdelay(5);
	}
	if (i >= FTS_UPGRADE_LOOP) {
		FTS_ERROR("reset to romboot fail");
		return -EIO;
	}

	return 0;
}

/*
 * ===========================================================================
 * fts_fwupg_check_state @ 0xffffff8008a7d764, 148 byte
 * ===========================================================================
 * FIRMA:
 *   x0 client -- "aa0003f4 mov"@0xffffff8008a7d784 -> x20, passato tale e
 *                quale a fts_fwupg_get_boot_state
 *   w1 rstate -- "2a0103f3 mov"@0xffffff8008a7d780 -> w19, confrontato a 32
 *                bit con il valore letto: "6b13011f cmp"@0xffffff8008a7d7a4
 *
 * IL VALORE DI RITORNO DI fts_fwupg_get_boot_state E' IGNORATO: dopo la
 * "97fffeec bl"@0xffffff8008a7d79c non c'e' nessuna lettura di w0, solo la
 * "b94007e8 ldr"@0xffffff8008a7d7a0 che rilegge la variabile locale.
 *
 * LA LOCALE E' AZZERATA UNA VOLTA SOLA, FUORI DAL CICLO.  La
 * "b90007ff str"@0xffffff8008a7d790 sta PRIMA dell'etichetta del ciclo, che
 * e' 0xffffff8008a7d794 (il bersaglio della
 * "54fffec3 b.cc"@0xffffff8008a7d7bc).  Se l'azzeramento fosse dentro il
 * ciclo, la `str` starebbe dopo quel bersaglio.
 *
 * IL RITORNO E' 1 o 0, non il codice d'errore:
 *   "320003e0 orr"@0xffffff8008a7d7c8   w0 = 1  (stato raggiunto)
 *   "2a1f03e0 mov"@0xffffff8008a7d7c0   w0 = 0  (trenta giri a vuoto)
 * Il TIPO `bool` e' una SCELTA, non una misura: un `int` che ritorna 1 e 0
 * darebbe le stesse due istruzioni.  Cio' che e' misurato sono i due valori.
 *
 * IL CONTATORE E' UNSIGNED: "71007abf cmp"@0xffffff8008a7d7b8 e'
 * seguito da "54fffec3 b.cc"@0xffffff8008a7d7bc, cioe' un salto SENZA segno.
 * Con un `int i` clang emette `b.lt`.  E' la classe A3.
 */
bool fts_fwupg_check_state(struct i2c_client *client, int rstate)
{
	u32 i = 0;
	int fw_sts = 0;

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		fts_fwupg_get_boot_state(client, &fw_sts);
		if (rstate == fw_sts)
			return true;

		msleep(FTS_DELAY_UPGRADE_LOOP);
	}

	return false;
}

/*
 * ===========================================================================
 * fts_fwupg_check_fw_valid @ 0xffffff8008a7d7f8, 64 byte
 * ===========================================================================
 * FIRMA: un solo parametro, x0, che NON viene mosso -- la
 * "940004b0 bl"@0xffffff8008a7d800 verso fts_wait_tp_to_valid e' la PRIMA
 * istruzione dopo il prologo, e x0 arriva com'e'.
 *
 * IL VERSO DEL CONTROLLO:
 *   "37f800e0 tbnz"@0xffffff8008a7d804   se w0 ha il bit 31 -> ramo a
 *                                        0xffffff8008a7d820
 * cioe' `if (ret < 0)` va al ramo che stampa
 * "\x016[FTS][Info]tp fw invaild\n"@0xffffff800924e4a0 e ritorna
 * "2a1f03e0 mov"@0xffffff8008a7d82c, w0 = 0; il seguito lineare stampa
 * "\x016[FTS][Info]tp fw vaild\n"@0xffffff800924e4bc e ritorna
 * "320003e0 orr"@0xffffff8008a7d814, w0 = 1.
 *
 * I DUE REFUSI SONO DELLA FABBRICA e si riproducono (regola 7): "invaild" e
 * "vaild" stanno cosi' nel binario, e sono le stringhe citate qui sopra.
 * Anche il livello e' misurato: tutte e due cominciano con `\x016`, cioe'
 * KERN_INFO -- il ramo d'errore NON usa FTS_ERROR.
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]tp fw invaild\n"@0xffffff800924e4a0
 *   "\x016[FTS][Info]tp fw vaild\n"@0xffffff800924e4bc
 *
 * NESSUN CANARINO: la funzione non ha buffer locali, quindi non c'e'
 * `__stack_chk_fail` e il prologo e' la sola
 * "a9bf7bfd stp"@0xffffff8008a7d7f8.
 */
bool fts_fwupg_check_fw_valid(struct i2c_client *client)
{
	int ret = 0;

	ret = fts_wait_tp_to_valid(client);
	if (ret < 0) {
		FTS_INFO("tp fw invaild");
		return false;
	}

	FTS_INFO("tp fw vaild");
	return true;
}

/*
 * ===========================================================================
 * fts_fwupg_reset_in_boot @ 0xffffff8008a7d838, 148 byte
 * ===========================================================================
 * FIRMA: un solo parametro, x0, salvato in x19 da
 * "aa0003f3 mov"@0xffffff8008a7d850 e ripassato tale e quale a fts_i2c_write
 * da "aa1303e0 mov"@0xffffff8008a7d874.
 *
 * IL BYTE DEL COMANDO E' ASSEGNATO PRIMA DELLA STAMPA, non dopo: la
 * "390013e9 strb"@0xffffff8008a7d864 sta PRIMA della
 * "97dad71b bl"@0xffffff8008a7d868 verso printk.  E' l'ordine di un
 * inizializzatore di dichiarazione, non di un'assegnazione dopo il messaggio.
 *
 * IL VALORE E' 7, e la larghezza dell'argomento e' 1:
 *   "32000be9 orr"@0xffffff8008a7d858   w9 = 0x7
 *   "320003e2 orr"@0xffffff8008a7d870   w2 = 1, la writelen
 *   "910013e1 add"@0xffffff8008a7d86c   x1 = &cmd
 *
 * IL RITORNO E' ZERO SUL RAMO BUONO, non `ret`:
 *   "2a1f03f3 mov"@0xffffff8008a7d888   w19 = 0 dopo la msleep
 *   "2a0003f3 mov"@0xffffff8008a7d890   w19 = w0 sul ramo d'errore
 *
 * IL CANARINO C'E' perche' c'e' un buffer locale il cui indirizzo esce dalla
 * funzione: "f9454508 ldr"@0xffffff8008a7d84c lo carica e
 * "eb08013f cmp"@0xffffff8008a7d8ac lo ricontrolla.
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]reset in boot environment\n"@0xffffff800924e5ff
 *   "\x013[FTS][Error]pram/rom/bootloader reset cmd write fail\n"@0xffffff800924e627
 *
 * QUESTA FUNZIONE CHIUDE LA DIVERGENZA 1 DEL LOTTO B1: fts_flash_read la
 * contiene INCORPORATA (vedi il cappello di quella funzione), e finche' non
 * era definita in questo file clang non aveva niente da incorporare.
 */
int fts_fwupg_reset_in_boot(struct i2c_client *client)
{
	int ret = 0;
	u8 cmd = FTS_REG_RESET_FW;

	FTS_INFO("reset in boot environment");
	ret = fts_i2c_write(client, &cmd, 1);
	if (ret < 0) {
		FTS_ERROR("pram/rom/bootloader reset cmd write fail");
		return ret;
	}

	msleep(FTS_DELAY_UPGRADE_RESET);

	return 0;
}

/*
 * ===========================================================================
 * fts_fwupg_reset_to_boot @ 0xffffff8008a7d8cc, 140 byte
 * ===========================================================================
 * FIRMA: un solo parametro, x0 -> x19 con
 * "aa0003f3 mov"@0xffffff8008a7d8d8.
 *
 * NESSUN BUFFER LOCALE, QUINDI NESSUN CANARINO: il prologo e' la sola coppia
 * "a9be4ff4 stp"@0xffffff8008a7d8cc + "a9017bfd stp"@0xffffff8008a7d8d0, e
 * l'epilogo non ricontrolla niente.  E' la conferma che i due valori vanno a
 * un REGISTRO e non a un buffer: si passa per fts_i2c_write_reg, non per
 * fts_i2c_write.
 *
 * IL REGISTRO E I DUE VALORI SONO NOMINATI DAL BINARIO nei messaggi d'errore
 * ("write FC=0xAA fail" e "write FC=0x55 fail"), e stanno negli immediati:
 *   "321e17e1 orr"@0xffffff8008a7d8e8   w1 = 0xfc   (l'indirizzo di registro)
 *   "52801542 mov"@0xffffff8008a7d8ec   w2 = 0xaa   (il primo valore)
 *   "321e17e1 orr"@0xffffff8008a7d904   w1 = 0xfc
 *   "52800aa2 mov"@0xffffff8008a7d908   w2 = 0x55   (il secondo valore)
 * NON DO' UN NOME NE' AL REGISTRO NE' AI DUE VALORI (regola 5): il binario li
 * scrive come numeri e li nomina come numeri, e un nome descrittivo sarebbe
 * indistinguibile da un fatto.  Restano numeri anche qui.
 *
 * I DUE RITARDI SONO DIVERSI, e sono misurati:
 *   "52800140 mov"@0xffffff8008a7d8fc   w0 = 10  fra il 0xAA e il 0x55
 *   "52800a00 mov"@0xffffff8008a7d918   w0 = 0x50 = 80  dopo il 0x55
 * Il secondo e' lo stesso 80 di fts_fwupg_reset_in_boot e lo scrivo con lo
 * stesso nome; il primo e' un 10 nudo e resta un 10.
 *
 * I DUE RAMI D'ERRORE SONO FUSI SULLA STAMPA (`tail merge`): tutti e due
 * arrivano alla "97dad6e4 bl"@0xffffff8008a7d944 dopo aver messo in x0 la
 * propria stringa, "911a8400 add"@0xffffff8008a7d930 e
 * "911b0c00 add"@0xffffff8008a7d940.  E' una fusione del compilatore, non due
 * `goto` del sorgente: la prova e' che ognuno dei due mette PRIMA il proprio
 * `ret` in w20 ("2a0003f4 mov"@0xffffff8008a7d928 e @0xffffff8008a7d938).
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]send 0xAA and 0x55 to FW, reset to boot environment\n"@0xffffff800924e65f
 *   "\x013[FTS][Error]write FC=0xAA fail\n"@0xffffff800924e6a1
 *   "\x013[FTS][Error]write FC=0x55 fail\n"@0xffffff800924e6c3
 */
int fts_fwupg_reset_to_boot(struct i2c_client *client)
{
	int ret = 0;

	FTS_INFO("send 0xAA and 0x55 to FW, reset to boot environment");
	ret = fts_i2c_write_reg(client, 0xFC, 0xAA);
	if (ret < 0) {
		FTS_ERROR("write FC=0xAA fail");
		return ret;
	}

	msleep(10);

	ret = fts_i2c_write_reg(client, 0xFC, 0x55);
	if (ret < 0) {
		FTS_ERROR("write FC=0x55 fail");
		return ret;
	}

	msleep(FTS_DELAY_UPGRADE_RESET);

	return 0;
}

/*
 * ===========================================================================
 * fts_fwupg_enter_into_boot @ 0xffffff8008a7d958, 348 byte
 * ===========================================================================
 * FIRMA: un solo parametro, x0 -> x19 con
 * "aa0003f3 mov"@0xffffff8008a7d974.
 *
 * IL GLOBALE E' LETTO UNA VOLTA SOLA, IN UN LOCALE, E LO STESSO x21 SOPRAVVIVE
 * A CINQUE CHIAMATE.  La "f9456515 ldr"@0xffffff8008a7d988 sta PRIMA della
 * "97dad6d2 bl"@0xffffff8008a7d98c (printk) e il registro non e' piu'
 * ricaricato; il CAMPO, invece, e' riletto:
 *   "f94012a8 ldr"@0xffffff8008a7d994   upg->c32, prima delle chiamate
 *   "f94012a8 ldr"@0xffffff8008a7d9f4   upg->c32, di nuovo dopo
 * cioe' il puntatore sta in un locale e il campo si scrive `upg->c32`.  E' la
 * stessa forma di fts_fwupg_get_boot_state.
 *
 * DUE FUNZIONI DI QUESTA STESSA UNITA' SONO INCORPORATE QUI DENTRO, e non e'
 * una deduzione: l'elenco delle chiamate non le contiene.
 *   $ ./venv/bin/python3 disassembla.py oracolo/stock.map oracolo/stock.elf \
 *         fts_fwupg_enter_into_boot
 *   fts_fwupg_enter_into_boot: 0xffffff8008a7d958, 348 byte
 *   istruzioni: 87
 *   chiama    : printk fts_wait_tp_to_valid fts_fwupg_reset_to_boot
 *               fts_pram_write_init fts_fwupg_get_boot_state msleep
 *               __stack_chk_fail
 * C'e' `fts_wait_tp_to_valid` ma NON `fts_fwupg_check_fw_valid`; c'e'
 * `fts_fwupg_get_boot_state` ma NON `fts_fwupg_check_state`.  Tutte e due
 * hanno simbolo proprio nella mappa (0xffffff8008a7d7f8 e 0xffffff8008a7d764)
 * e tutte e due sono definite PRIMA di questa in questo file, quindi clang le
 * incorpora qui come le incorpora di fabbrica.
 *
 * (a) fts_fwupg_check_fw_valid, incorporata a 0xffffff8008a7d99c..d9f0:
 *       "94000448 bl"@0xffffff8008a7d9a0    fts_wait_tp_to_valid
 *       "37f80220 tbnz"@0xffffff8008a7d9a4  se ret<0 -> il ramo "invaild"
 *       "9112f000 add"@0xffffff8008a7d9ac   x0 = "tp fw vaild"
 *       "91128000 add"@0xffffff8008a7d9ec   x0 = "tp fw invaild"
 *     Le due stringhe sono le STESSE di quella funzione,
 *     "\x016[FTS][Info]tp fw vaild\n"@0xffffff800924e4bc e
 *     "\x016[FTS][Info]tp fw invaild\n"@0xffffff800924e4a0, refusi compresi.
 *     Il ramo "vaild" (valida) chiama fts_fwupg_reset_to_boot, il ramo
 *     "invaild" lo SALTA: e' `if (fts_fwupg_check_fw_valid(client)) {...}`.
 *
 * (b) fts_fwupg_check_state(client, 4), incorporata a
 *     0xffffff8008a7da3c..da78, con l'argomento costante piegato dentro:
 *       "b90007ff str"@0xffffff8008a7da3c   fw_sts = 0, FUORI dal ciclo (il
 *                                           bersaglio del ciclo e' d a40)
 *       "97fffe41 bl"@0xffffff8008a7da48    fts_fwupg_get_boot_state
 *       "7100111f cmp"@0xffffff8008a7da50   fw_sts == 4 ?
 *       "52800280 mov"@0xffffff8008a7da58   msleep(20)
 *       "71007a9f cmp"@0xffffff8008a7da64   il giro numero 30
 *       "54fffec3 b.cc"@0xffffff8008a7da68  salto SENZA segno: contatore u32
 *     Sono le stesse cinque costanti di fts_fwupg_check_state -- 30, 20 e il
 *     confronto a 32 bit -- col `rstate` sostituito dalla costante 4.
 *
 * L'ORDINE DEI DUE CONTROLLI NULL e' `upg` poi `upg->c32`:
 *   "b4000235 cbz"@0xffffff8008a7d990   upg
 *   "b40001e8 cbz"@0xffffff8008a7d998   upg->c32
 * e il messaggio li nomina in quest'ordine
 * ("upgrade/func is null"): qui, a differenza di fts_fwupg_get_boot_state,
 * ordine e messaggio coincidono.
 *
 * I DUE CODICI D'ERRORE SONO DIVERSI E SONO MISURATI:
 *   "128002b4 mov"@0xffffff8008a7d9e0   w20 = -22 = -EINVAL (upgrade nullo)
 *   "321d7bf4 orr"@0xffffff8008a7da78   w20 = -5  = -EIO    (mai in bootloader)
 *
 * I MESSAGGI DI QUESTA FUNZIONE, in ordine di indirizzo nel `.rodata`:
 *   "\x013[FTS][Error]upgrade/func is null\n"@0xffffff800924e2bf
 *   "\x016[FTS][Info]tp fw invaild\n"@0xffffff800924e4a0
 *   "\x016[FTS][Info]tp fw vaild\n"@0xffffff800924e4bc
 *   "\x016[FTS][Info]***********enter into pramboot/bootloader***********\n"@0xffffff800924e6e5
 *   "\x013[FTS][Error]enter into romboot/bootloader fail\n"@0xffffff800924e728
 *   "\x016[FTS][Info]pram supported, write pramboot and init\n"@0xffffff800924e75a
 *   "\x013[FTS][Error]pram write_init fail\n"@0xffffff800924e790
 *   "[FTS]pram not supported, confirm in bootloader\n"@0xffffff800924e7b4
 *   "\x013[FTS][Error]fw not in bootloader, fail\n"@0xffffff800924e7e4
 * I primi tre sono CONDIVISI con le funzioni che li stampano fuori linea
 * (0x2bf sta anche in fts_pram_write_init, 0x4a0 e 0x4bc in
 * fts_fwupg_check_fw_valid): sono stringhe deduplicate, non copie.
 */
int fts_fwupg_enter_into_boot(struct i2c_client *client)
{
	int ret = 0;
	struct fts_upgrade *upg = fts_g_a100ac8;

	FTS_INFO("***********enter into pramboot/bootloader***********");
	if ((NULL == upg) || (NULL == upg->c32)) {
		FTS_ERROR("upgrade/func is null");
		return -EINVAL;
	}

	if (fts_fwupg_check_fw_valid(client)) {
		ret = fts_fwupg_reset_to_boot(client);
		if (ret < 0) {
			FTS_ERROR("enter into romboot/bootloader fail");
			return ret;
		}
	}

	if (upg->c32->c65) {
		FTS_INFO("pram supported, write pramboot and init");
		ret = fts_pram_write_init(client);
		if (ret < 0) {
			FTS_ERROR("pram write_init fail");
			return ret;
		}
	} else {
		FTS_DEBUG("pram not supported, confirm in bootloader");
		if (!fts_fwupg_check_state(client, FTS_RUN_IN_BOOTLOADER)) {
			FTS_ERROR("fw not in bootloader, fail");
			return -EIO;
		}
	}

	return 0;
}

/*
 * ===========================================================================
 * IL CICLO DI ATTESA DELLO STATO DEL FLASH: UNA FUNZIONE `static` CHE NELLA
 * MAPPA NON C'E', PERCHE' DI FABBRICA E' INCORPORATA IN TUTTI E DUE I SUOI
 * SITI DI CHIAMATA
 * ===========================================================================
 * QUESTA E' UNA DEDUZIONE, NON UNA LETTURA DIRETTA, e la prova sta nel numero
 * di CELLE DI PILA, non nel testo.  In fts_fwupg_erase il byte del comando
 * mandato per la cancellazione e quello mandato per leggere lo stato NON
 * stanno nella stessa cella:
 *   "390033e8 strb"@0xffffff8008a7daf8   0x61 -> sp+12
 *   "910033e1 add"@0xffffff8008a7daec    x1 = sp+12, la writelen e' 1
 *   "390053f5 strb"@0xffffff8008a7db34   0x6a -> sp+20
 *   "910053e1 add"@0xffffff8008a7db20    x1 = sp+20, la writelen e' 1
 * Sono due indirizzi diversi passati a due chiamate diverse.  Una sola
 * variabile `u8 cmd` darebbe una sola cella: l'indirizzo esce dalla funzione
 * alla prima fts_i2c_write, quindi clang non puo' ne' spezzarla ne'
 * duplicarla.  Due celle vogliono dire DUE variabili, e la seconda e' la
 * variabile locale di un'altra funzione, incorporata qui.
 *
 * LA SECONDA PROVA E' DOVE VENGONO AZZERATE.  In fts_fwupg_erase le due
 * "390053ff strb"@0xffffff8008a7db18 (cmd = 0) e
 * "790023ff strh"@0xffffff8008a7db1c (value[2] = {0}) stanno DOPO la
 * msleep(delay) e appena prima del ciclo, non in testa alla funzione; in
 * fts_fwupg_ecc_cal le stesse due, "390033ff strb"@0xffffff8008a7dcec e
 * "790013ff strh"@0xffffff8008a7dcf0, stanno DENTRO il ciclo esterno.  Due
 * punti di azzeramento diversi per lo stesso paio di variabili sono due
 * punti di INNESTO diversi della stessa funzione.
 *
 * LA TERZA PROVA E' CHE I DUE CICLI HANNO LA STESSA FORMA con quattro
 * costanti diverse, e nient'altro di diverso:
 *   in fts_fwupg_erase       in fts_fwupg_ecc_cal
 *   0x6a  il comando         0x6a  lo stesso comando
 *   0xf0aa lo stato atteso    0xf055 lo stato atteso
 *   0x32 = 50 i giri          0xa  = 10 i giri
 *   0xc8 = 200 il ritardo     0x32 = 50 il ritardo
 * cioe' quattro parametri.  Le istruzioni che compongono la halfword sono le
 * stesse due, identiche anche nella codifica:
 *   "33181d09 bfi"@0xffffff8008a7db44   (in fts_fwupg_erase)
 *   "33181d09 bfi"@0xffffff8008a7dd18   (in fts_fwupg_ecc_cal)
 *
 * IL NOME `fts_fwupg_check_flash_status` E I NOMI DEI SUOI PARAMETRI SONO UNA
 * SCELTA, NON UNA MISURA: una funzione incorporata non lascia simbolo, e il
 * binario non la nomina in nessun messaggio.  Cio' che e' misurato e' che
 * ESISTE (le due celle di pila) e che ha quattro parametri (le quattro
 * costanti che cambiano fra i due siti).  Se un lotto futuro trovasse una
 * lettura migliore, e' questo il pezzo da rimettere in discussione.
 *
 * LA MISURA E' CHE NON SI VEDE: se clang NON la incorporasse, comparirebbe un
 * simbolo `t` in piu' nell'oggetto e le due funzioni misurerebbero meno.
 * Il conto dei simboli e le due dimensioni sono nel blocco in coda.
 *
 * IL VALORE DI RITORNO DI fts_i2c_read E' IGNORATO -- e' un difetto di
 * fabbrica che si riproduce (regola 7): dopo la
 * "940009dd bl"@0xffffff8008a7db38 la prima istruzione e' la
 * "394043e8 ldrb"@0xffffff8008a7db3c, che sovrascrive w8 senza guardare w0.
 *
 * IL CONTATORE E' UNSIGNED: "7100ca9f cmp"@0xffffff8008a7db5c e' seguito da
 * "54fffe03 b.cc"@0xffffff8008a7db60, cioe' un salto SENZA segno (classe A3).
 */
static int fts_fwupg_check_flash_status(struct i2c_client *client, u16 expect,
					u32 retries, u32 retries_delay)
{
	u32 i = 0;
	u8 cmd = 0;
	u8 value[2] = { 0 };
	u16 read_status = 0;

	for (i = 0; i < retries; i++) {
		cmd = FTS_CMD_FLASH_STATUS;
		fts_i2c_read(client, &cmd, 1, value, 2);
		read_status = (((u16)value[0]) << 8) + value[1];
		if (expect == read_status)
			return 0;

		msleep(retries_delay);
	}

	return -EIO;
}

/*
 * ===========================================================================
 * fts_fwupg_erase @ 0xffffff8008a7dab4, 268 byte
 * ===========================================================================
 * FIRMA:
 *   x0 client -- "aa0003f3 mov"@0xffffff8008a7dad0 -> x19
 *   w1 delay  -- "2a0103f5 mov"@0xffffff8008a7dadc -> w21, e da li' passa
 *                intatto a msleep con "2a1503e0 mov"@0xffffff8008a7db04.
 *                LA SUA LARGHEZZA E IL SUO SEGNO NON SONO MISURATI: msleep
 *                prende un `unsigned int` e un `int` darebbe lo stesso `mov`.
 *                `u32` e' una SCELTA.
 *
 * IL BYTE DEL COMANDO NON E' AZZERATO NEL BINARIO, e non e' un caso: la sola
 * scrittura nella cella sp+12 e' la "390033e8 strb"@0xffffff8008a7daf8, che
 * ci mette 0x61.  L'azzeramento della dichiarazione c'e' nel sorgente ma e'
 * MORTO -- l'indirizzo della variabile non e' ancora uscito dalla funzione
 * quando arriva l'assegnazione -- e clang lo cancella.  Che sia cosi' lo
 * prova il confronto con la cella sp+20, che INVECE e' azzerata
 * ("390053ff strb"@0xffffff8008a7db18) perche' li' in mezzo c'e' gia' stata
 * una chiamata che ha visto l'indirizzo.
 *
 * LE COSTANTI, dagli immediati:
 *   "52800c28 mov"@0xffffff8008a7dae8   w8 = 0x61, il comando di cancellazione
 *   "52800d55 mov"@0xffffff8008a7db10   w21 = 0x6a, il comando di stato
 *   "529e1556 mov"@0xffffff8008a7db14   w22 = 0xf0aa, lo stato atteso
 *   "7100ca9f cmp"@0xffffff8008a7db5c   il giro numero 0x32 = 50
 *   "52801900 mov"@0xffffff8008a7db50   w0 = 0xc8 = 200, il ritardo
 * NON DO' UN NOME a 0x61, 0xf0aa, 50 e 200 (regola 5): il binario li scrive
 * come numeri e non li nomina in nessun messaggio.
 *
 * IL CODICE D'ERRORE DELL'ATTESA E' -EIO:
 *   "321d7bf4 orr"@0xffffff8008a7db70   w20 = 0xfffffffb = -5
 * ed e' materializzato DOPO la stampa, cioe' e' una costante che il
 * compilatore conosce, non il valore ritornato da una chiamata.
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]**********erase now**********\n"@0xffffff800924e80e
 *   "\x013[FTS][Error]erase cmd fail\n"@0xffffff800924e83a
 *   "\x013[FTS][Error]ecc flash status check fail\n"@0xffffff800924e858
 * Il terzo dice "ecc" ma sta nella funzione di CANCELLAZIONE: e' un refuso
 * della fabbrica e si riproduce (regola 7).
 */
int fts_fwupg_erase(struct i2c_client *client, u32 delay)
{
	int ret = 0;
	u8 cmd = 0;

	FTS_INFO("**********erase now**********");

	cmd = 0x61;
	ret = fts_i2c_write(client, &cmd, 1);
	if (ret < 0) {
		FTS_ERROR("erase cmd fail");
		return ret;
	}

	msleep(delay);

	ret = fts_fwupg_check_flash_status(client, 0xF0AA, 50, 200);
	if (ret < 0) {
		FTS_ERROR("ecc flash status check fail");
		return ret;
	}

	return 0;
}

/*
 * ===========================================================================
 * fts_fwupg_ecc_cal @ 0xffffff8008a7dbc0, 540 byte
 * ===========================================================================
 * FIRMA:
 *   x0 client -- "aa0003f3 mov"@0xffffff8008a7dbe8 -> x19
 *   w1 saddr  -- versato subito sulla pila da
 *                "b90003e1 str"@0xffffff8008a7dbf8 e riletto a ogni giro da
 *                "b94003e8 ldr"@0xffffff8008a7dc84
 *   w2 len    -- "2a0203f5 mov"@0xffffff8008a7dbf4 -> w21, e diviso SENZA
 *                segno (vedi qui sotto)
 *
 * LA DIVISIONE PER 0xFFFE E' UNA DIVISIONE SENZA SEGNO, ed e' la prova che
 * `len` e' unsigned (classe A3).  clang la fa col reciproco magico:
 *   "53017ea8 lsr"@0xffffff8008a7dc30    w8 = len >> 1     (LOGICO)
 *   "52800069 mov"@0xffffff8008a7dc2c  \
 *   "72b00029 movk"@0xffffff8008a7dc34 /  w9 = 0x80010003
 *   "9ba97d08 umull"@0xffffff8008a7dc38  x8 = w8 * w9      (UMULL, non SMULL)
 *   "d36efd08 lsr"@0xffffff8008a7dc3c    x8 >>= 46         => len / 0xFFFE
 *   "1b09d515 msub"@0xffffff8008a7dc44   w21 = len - w8*0xFFFE  => il resto
 *   "1a880516 cinc"@0xffffff8008a7dc4c   w22 = w8 + (resto != 0)
 * `umull` e il `lsr` iniziale (non `asr`) sono la firma della divisione
 * unsigned; l'ultima riga e' `if (remainder > 0) packet_number++;` scritto
 * senza salto.
 *
 * LE COSTANTI, dagli immediati:
 *   "52800c88 mov"@0xffffff8008a7dc10   w8 = 0x64, il comando di avvio
 *   "52800ca8 mov"@0xffffff8008a7dc64   w8 = 0x65, il comando di calcolo
 *   "52800cc8 mov"@0xffffff8008a7dd44   w8 = 0x66, il comando di lettura
 *   "321f3be9 orr"@0xffffff8008a7dc8c   w9 = 0xfffe, la taglia del pezzo
 *   "529e0ab4 mov"@0xffffff8008a7dc7c   w20 = 0xf055, lo stato atteso
 *   "71002b3f cmp"@0xffffff8008a7dd30   il giro numero 0xa = 10
 *   "52800640 mov"@0xffffff8008a7dd24   w0 = 0x32 = 50, il ritardo
 * NON DO' UN NOME a nessuna di queste (regola 5): il binario le scrive come
 * numeri e non le nomina in nessun messaggio.
 *
 * IL RITARDO FRA IL COMANDO E L'ATTESA E' `packet_len / 256`, E LA STESSA
 * ESPRESSIONE E' ANCHE cmd[4].  Il compilatore la calcola una volta sola:
 *   "53087f1a lsr"@0xffffff8008a7dca8   w26 = packet_len >> 8
 *   "390053fa strb"@0xffffff8008a7dcc0  cmd[4] = w26
 *   "2a1a03e0 mov"@0xffffff8008a7dce0   w0 = w26, l'argomento di msleep
 * Che sia una divisione SENZA segno lo dice ancora il `lsr`.  Non c'e'
 * nessuna `and #0xff` prima della msleep perche' clang sa gia' che il valore
 * sta in un byte: packet_len non supera 0xFFFE.
 *
 * IL CICLO DI ATTESA E' LA FUNZIONE INCORPORATA fts_fwupg_check_flash_status
 * (vedi il suo cappello sopra), qui con (0xf055, 10, 50), e IL SUO VALORE DI
 * RITORNO E' IGNORATO: quando il ciclo interno finisce senza mai vedere lo
 * stato buono, "54fffe03 b.cc"@0xffffff8008a7dd34 cade a
 * 0xffffff8008a7dd38, che e' l'incremento del ciclo esterno -- nessuna
 * stampa, nessun ritorno.  E' un difetto di fabbrica e si riproduce
 * (regola 7).
 *
 * LA LUNGHEZZA DELL'ULTIMO PEZZO E' UN `ccmp` + `csel`, cioe' la congiunzione
 * di due condizioni senza salti, e il valore vecchio e' l'ALTRO ramo del
 * `csel` (quindi `packet_len` e' una variabile viva fuori dal ciclo, non una
 * costante):
 *   "6b1b039f cmp"@0xffffff8008a7dc88    i == packet_number - 1 ?
 *   "7a400aa4 ccmp"@0xffffff8008a7dc90   se si', remainder == 0 ?
 *   "1a9812b8 csel"@0xffffff8008a7dc98   packet_len = ne ? remainder : packet_len
 *   "321f3bf8 orr"@0xffffff8008a7dc80    e fuori dal ciclo vale 0xfffe
 *
 * L'INDIRIZZO DEL PEZZO E' UN `madd`, cioe' `saddr + 0xFFFE * i` in una sola
 * istruzione: "1b092381 madd"@0xffffff8008a7dc94.
 *
 * IL VALORE LETTO ALLA FINE E' UN BYTE SOLO, MA LA CELLA NE VALE DUE:
 *   "79000bff strh"@0xffffff8008a7dc08   azzera sp+4 e sp+5 -- DUE byte
 *   "320003e4 orr"@0xffffff8008a7dd54    la readlen e' 1
 *   "394013f9 ldrb"@0xffffff8008a7dd68   si rilegge il solo sp+4
 * Un `u8` singolo darebbe una `strb`.  L'oggetto e' quindi un `u8 val[2]`
 * azzerato di cui si usa il solo elemento zero: e' cio' che il binario dice,
 * e non lo raddrizzo (regola 7).
 *
 * IL VALORE DI RITORNO E' IL BYTE LETTO, non zero:
 *   "394013f9 ldrb"@0xffffff8008a7dd68   w25 = val[0]
 *   "2a1903e0 mov"@0xffffff8008a7ddb4    w0 = w25
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]**********read out checksum**********\n"@0xffffff800924e883
 *   "\x013[FTS][Error]ecc init cmd write fail\n"@0xffffff800924e8b7
 *   "\x016[FTS][Info]ecc calc num:%d, remainder:%d\n"@0xffffff800924e8de
 *   "[FTS]ecc calc startaddr:0x%04x, len:%d\n"@0xffffff800924e90a
 *   "\x013[FTS][Error]ecc calc cmd write fail\n"@0xffffff800924e932
 *   "\x013[FTS][Error]ecc read cmd write fail\n"@0xffffff800924e959
 * IL QUARTO E' SENZA LIVELLO KERN e con prefisso "[FTS]" nudo: e' la terza
 * macro (vedi il cappello delle macro).  Gli altri cinque hanno il livello.
 */
int fts_fwupg_ecc_cal(struct i2c_client *client, u32 saddr, u32 len)
{
	int ret = 0;
	u32 i = 0;
	u32 packet_number = 0;
	u32 packet_len = 0;
	u32 remainder = 0;
	u32 addr = 0;
	u8 cmd[6] = { 0 };
	u8 val[2] = { 0 };

	FTS_INFO("**********read out checksum**********");

	cmd[0] = 0x64;
	ret = fts_i2c_write(client, cmd, 1);
	if (ret < 0) {
		FTS_ERROR("ecc init cmd write fail");
		return ret;
	}

	packet_number = len / 0xFFFE;
	remainder = len % 0xFFFE;
	if (remainder > 0)
		packet_number++;
	packet_len = 0xFFFE;
	FTS_INFO("ecc calc num:%d, remainder:%d", packet_number, remainder);

	cmd[0] = 0x65;
	for (i = 0; i < packet_number; i++) {
		addr = saddr + 0xFFFE * i;
		cmd[1] = (u8)((addr >> 16) & 0xFF);
		cmd[2] = (u8)((addr >> 8) & 0xFF);
		cmd[3] = (u8)(addr & 0xFF);

		if ((i == (packet_number - 1)) && remainder)
			packet_len = remainder;

		cmd[4] = (u8)((packet_len >> 8) & 0xFF);
		cmd[5] = (u8)(packet_len & 0xFF);

		FTS_DEBUG("ecc calc startaddr:0x%04x, len:%d", addr, packet_len);
		ret = fts_i2c_write(client, cmd, 6);
		if (ret < 0) {
			FTS_ERROR("ecc calc cmd write fail");
			return ret;
		}

		msleep(packet_len / 256);

		fts_fwupg_check_flash_status(client, 0xF055, 10, 50);
	}

	cmd[0] = 0x66;
	ret = fts_i2c_read(client, cmd, 1, val, 1);
	if (ret < 0) {
		FTS_ERROR("ecc read cmd write fail");
		return ret;
	}

	return (int)val[0];
}

/* Lo stato che il TP restituisce quando il pacchetto e' andato a buon fine:
 * l'indice del pacchetto piu' 0x1000.
 *   "1ad50ae8 udiv"@0xffffff8008a7df48   w8 = addr / packet_len
 *   "11400508 add"@0xffffff8008a7df4c    w8 += 0x1 << 12 = 0x1000
 *   "12003d17 and"@0xffffff8008a7df54    e il risultato e' troncato a 16 bit
 */
#define FTS_FLASH_STATUS_OK		0x1000

/*
 * ===========================================================================
 * fts_flash_write_buf @ 0xffffff8008a7dddc, 584 byte
 * ===========================================================================
 * FIRMA, letta dai registri d'ingresso e dal loro uso:
 *   x0 client -- "aa0003f6 mov"@0xffffff8008a7de04+0x28 -> x22
 *   w1 saddr  -- versato subito sullo stack insieme a w4:
 *                "290407e4 stp"@0xffffff8008a7de10  stp w4, w1, [sp,#32]
 *                e riletto a ogni giro da "b94027e8 ldr"@0xffffff8008a7deb8
 *   x2 buf    -- "aa0203f4 mov"@0xffffff8008a7de18 -> x20, indicizzato da
 *                "386a4a8a ldrb"@0xffffff8008a7def8 con `uxtw`: indice UNSIGNED
 *   w3 len    -- "2a0303f8 mov"@0xffffff8008a7de14 -> w24, diviso con
 *                "53057f08 lsr"@0xffffff8008a7de54 (LOGICO, non aritmetico)
 *   w4 retry  -- l'altra meta' della `stp` di sopra, riletto da
 *                "b94023e8 ldr"@0xffffff8008a7df28 e allargato a 64 bit con
 *                `uxtw` da "cb284368 sub"@0xffffff8008a7de9c
 * `lsr`, `and #0x1f` senza correzione di segno e `uxtw` negli indici: saddr,
 * len e retry sono UNSIGNED (classi A3/A4).
 *
 * I TRE BUFFER LOCALI E IL LORO AZZERAMENTO.  Il binario azzera 38 byte a
 * sp+48, 1 byte a sp+44 e 2 byte a sp+40:
 *   "a9037fff stp"@0xffffff8008a7de28   xzr,xzr -> sp+48..63
 *   "a9047fff stp"@0xffffff8008a7de24   xzr,xzr -> sp+64..79
 *   "f804e3ff stur"@0xffffff8008a7de20  xzr     -> sp+78..85
 *   "3900b3ff strb"@0xffffff8008a7de2c  wzr     -> sp+44
 *   "790053ff strh"@0xffffff8008a7de30  wzr     -> sp+40
 * cioe' 48..85 = 38 byte = 32 + 6, che e' esattamente la taglia del pacchetto
 * piu' la sua intestazione; poi un byte solo (il comando di stato) e due byte
 * (il valore letto).  Le tre `= { 0 }` sono MISURATE, non messe per abitudine.
 *
 * IL RITARDO E' `mdelay(retry)`, NON UN CICLO SCRITTO A MANO.  La prova e' la
 * forma canonica in cui clang riscrive il `while (__ms--)` della macro
 * mdelay() del kernel quando l'argomento NON e' costante:
 *   "cb284368 sub"@0xffffff8008a7de9c   x8 = 0 - (u64)retry, fuori dal ciclo
 *   "f90007e8 str"@0xffffff8008a7deac   e messo da parte
 *   "f94007f9 ldr"@0xffffff8008a7df30   x25 = -retry
 *   "52912b00 mov"@0xffffff8008a7df34 + "72a00820 movk"@0xffffff8008a7df38
 *                                       w0 = 0x418958
 *   "940f66ce bl"@0xffffff8008a7df3c    __const_udelay
 *   "b1000739 adds"@0xffffff8008a7df40  x25 += 1, con flag
 *   "54ffff83 b.cc"@0xffffff8008a7df44  finche' non arriva a zero
 * 0x418958 = 4295000 = 1000 * 0x10c7, cioe' `udelay(1000)`, cioe' `mdelay(1)`
 * per un compilatore che vede la costante: la macro mdelay del kernel espande
 * l'argomento non costante in `{__ms = n; while (__ms--) udelay(1000);}`.  Il
 * `uxtw` della `sub` dice che `retry` e' un `u32` allargato a `unsigned long`.
 * La guardia `if (retry != 0)` non e' scritta nel sorgente: e'
 * "340000e8 cbz"@0xffffff8008a7df2c, cioe' la rotazione del `while` fatta da
 * clang.
 *
 * L'ATTESA DELLO STATO usa la stessa mdelay(1), ma stavolta con la costante
 * gia' nel sorgente: "52912b00 mov"@0xffffff8008a7df88 +
 * "940f66b9 bl"@0xffffff8008a7df90 hanno lo stesso immediato, e non c'e'
 * nessun contatore a 64 bit -- il contatore e' w25 e si ferma a
 * "7101933f cmp"@0xffffff8008a7df98, cioe' 100.
 *
 * LO STATO LETTO E' UNA HALFWORD COMPOSTA A MANO:
 *   "3940a3e8 ldrb"@0xffffff8008a7df74   w8 = value[0]
 *   "3940a7e9 ldrb"@0xffffff8008a7df78   w9 = value[1]
 *   "33181d09 bfi"@0xffffff8008a7df7c    w9 = (value[0] << 8) | value[1]
 *   "6b0902ff cmp"@0xffffff8008a7df80    confrontata con wr_ok
 * e wr_ok e' troncato a 16 bit da "12003d17 and"@0xffffff8008a7df54: la
 * variabile e' un `u16`, non un `int` (con un `int` la `and` non ci sarebbe).
 *
 * IL VALORE DI RITORNO E' LA ECC, MASCHERATA A UN BYTE:
 *   "4a1a015a eor"@0xffffff8008a7df08   ecc ^= dato
 *   "12001f59 and"@0xffffff8008a7dfd0   w25 = ecc & 0xff
 * La `and` finale e' l'estensione a `int` di un `u8`: se `ecc` fosse un `int`
 * non ci sarebbe niente da mascherare.  E' la classe A4.
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]**********write data to flash**********\n"@0xffffff800924e980
 *   "\x013[FTS][Error]buf is NULL or len is 0\n"@0xffffff800924e9b6
 *   "\x016[FTS][Info]data buf start addr=0x%x, len=0x%x\n"@0xffffff800924e9dd
 *   "\x016[FTS][Info]write data, num:%d remainder:%d\n"@0xffffff800924ea0e
 *   "\x013[FTS][Error]app write fail\n"@0xffffff800924ea3c
 * Il secondo e' CONDIVISO con fts_flash_read_buf e con fts_flash_read: le tre
 * funzioni caricano tutte 0x9b6 sulla stessa pagina
 * ("9126d800 add"@0xffffff8008a7dfbc, @0xffffff8008a7e118 e
 *  @0xffffff8008a7e1fc), cioe' e' una sola stringa deduplicata dal
 * compilatore, non tre.
 *
 * IL VALORE DI RITORNO DI fts_i2c_read NELL'ATTESA E' IGNORATO: dopo la
 * "940008cf bl"@0xffffff8008a7df70 la prima istruzione e' la
 * "3940a3e8 ldrb"@0xffffff8008a7df74, che sovrascrive w8 senza guardare w0.
 * E' un difetto di fabbrica e si riproduce (regola 7).
 */
int fts_flash_write_buf(struct i2c_client *client, u32 saddr, u8 *buf,
			u32 len, u32 retry)
{
	int ret = 0;
	u32 i = 0;
	u32 j = 0;
	u32 packet_number = 0;
	u32 packet_len = 0;
	u32 addr = 0;
	u32 offset = 0;
	u32 remainder = 0;
	u8 packet_buf[FTS_FLASH_PACKET_LENGTH + FTS_CMD_WRITE_LEN] = { 0 };
	u8 ecc = 0;
	u8 cmd = 0;
	u8 value[2] = { 0 };
	u16 read_status = 0;
	u16 wr_ok = 0;

	FTS_INFO("**********write data to flash**********");
	if ((NULL == buf) || (0 == len)) {
		FTS_ERROR("buf is NULL or len is 0");
		return -EINVAL;
	}

	FTS_INFO("data buf start addr=0x%x, len=0x%x", saddr, len);
	packet_number = len / FTS_FLASH_PACKET_LENGTH;
	remainder = len % FTS_FLASH_PACKET_LENGTH;
	if (remainder > 0)
		packet_number++;
	packet_len = FTS_FLASH_PACKET_LENGTH;
	FTS_INFO("write data, num:%d remainder:%d", packet_number, remainder);

	packet_buf[0] = FTS_CMD_WRITE;
	for (i = 0; i < packet_number; i++) {
		offset = i * FTS_FLASH_PACKET_LENGTH;
		addr = saddr + offset;
		packet_buf[1] = (u8)((addr >> 16) & 0xFF);
		packet_buf[2] = (u8)((addr >> 8) & 0xFF);
		packet_buf[3] = (u8)(addr & 0xFF);

		if ((i == (packet_number - 1)) && remainder)
			packet_len = remainder;

		packet_buf[4] = (u8)((packet_len >> 8) & 0xFF);
		packet_buf[5] = (u8)(packet_len & 0xFF);

		for (j = 0; j < packet_len; j++) {
			packet_buf[FTS_CMD_WRITE_LEN + j] = buf[offset + j];
			ecc ^= packet_buf[FTS_CMD_WRITE_LEN + j];
		}

		ret = fts_i2c_write(client, packet_buf,
				    packet_len + FTS_CMD_WRITE_LEN);
		if (ret < 0) {
			FTS_ERROR("app write fail");
			return ret;
		}

		mdelay(retry);

		wr_ok = addr / packet_len + FTS_FLASH_STATUS_OK;
		for (j = 0; j < FTS_RETRIES_WRITE; j++) {
			cmd = FTS_CMD_FLASH_STATUS;
			ret = fts_i2c_read(client, &cmd, 1, value, 2);
			read_status = (((u16)value[0]) << 8) + value[1];
			if (wr_ok == read_status)
				break;

			mdelay(1);
		}
	}

	return (int)ecc;
}

/*
 * ===========================================================================
 * fts_flash_read_buf @ 0xffffff8008a7e024, 352 byte
 * ===========================================================================
 * FIRMA:
 *   x0 client -- "aa0003f5 mov"@0xffffff8008a7e060 -> x21
 *   w1 saddr  -- "2a0103f4 mov"@0xffffff8008a7e074 -> w20, incrementato di
 *                0x20 a ogni giro da "11008294 add"@0xffffff8008a7e0fc
 *   x2 buf    -- "aa0203f3 mov"@0xffffff8008a7e070 -> x19, confrontato con
 *                NULL da "b4000622 cbz"@0xffffff8008a7e050 e incrementato di
 *                0x20 da "91008273 add"@0xffffff8008a7e104
 *   w3 len    -- confrontato con zero da "34000603 cbz"@0xffffff8008a7e054 e
 *                diviso da "53057c68 lsr"@0xffffff8008a7e058
 *
 * NESSUNA INTESTAZIONE PRIMA DEL CONTROLLO.  A differenza di
 * fts_flash_write_buf e di fts_flash_read, qui i due `cbz` sono le PRIME due
 * istruzioni dopo il prologo e il canarino: la prima
 * "97dad514 bl"@0xffffff8008a7e084 verso printk e' gia' quella del messaggio
 * "read packet_number:%d, remainder:%d".  Non c'e' nessun banner
 * "**********...**********" in questa funzione.
 *
 * IL BUFFER DEL COMANDO NON E' AZZERATO, E QUESTO E' UNA MISURA.  Nel binario
 * di fabbrica non c'e' nessuna `str wzr,[sp,#4]` prima dei due `cbz`, mentre
 * in fts_flash_write_buf le cinque istruzioni di azzeramento ci sono.  Che la
 * differenza venga dal SORGENTE e non da un'ottimizzazione lo prova il
 * compilatore di fabbrica stesso: questo lotto aveva scritto
 * `u8 packet_buf[FTS_CMD_READ_LEN] = { 0 };` e clang r353983c NON ha
 * cancellato l'azzeramento --
 *   con `= { 0 }`:   0000000000000514 0000000000000164 T fts_flash_read_buf
 *   senza:           0000000000000514 0000000000000160 T fts_flash_read_buf
 * cioe' 356 byte contro 352, una istruzione in piu', e il confronto per
 * codifica la mostrava in testa:
 *      11  F ffffff8008a7e050 b4000622 cbz    x2, ...  | N b90007ff str    wzr, [sp,#4]
 * Quindi il sorgente di fabbrica dichiara il buffer SENZA inizializzatore.
 * In fts_flash_write_buf, invece, l'azzeramento resta perche' la coda del
 * pacchetto puo' non essere mai scritta (packet_len < 32).
 *
 * LA LUNGHEZZA DELL'ULTIMO PACCHETTO E' UN `ccmp` + `csel`, cioe' la
 * congiunzione di due condizioni senza salti:
 *   "eb1a037f cmp"@0xffffff8008a7e0a0    (packet_number - 1) == i ?
 *   "7a400ac4 ccmp"@0xffffff8008a7e0ac   se si', remainder == 0 ?
 *   "1a9812d8 csel"@0xffffff8008a7e0c8   packet_len = ne ? remainder : 0x20
 * cioe' `if ((i == packet_number - 1) && remainder) packet_len = remainder;`
 * con packet_len che vale 32 fuori dal ciclo.
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]read packet_number:%d, remainder:%d\n"@0xffffff800924ea5a
 *   "\x013[FTS][Error]buf is NULL or len is 0\n"@0xffffff800924e9b6
 *   "\x013[FTS][Error]pram/bootloader write 03 command fail\n"@0xffffff800924ea8c
 *   "\x013[FTS][Error]pram/bootloader read 03 command fail\n"@0xffffff800924eac1
 *
 * LA LETTURA NON MANDA NIENTE: "aa1f03e1 mov"@0xffffff8008a7e0e0 mette x1 =
 * NULL e "2a1f03e2 mov"@0xffffff8008a7e0e4 mette w2 = 0, cioe' writebuf NULL e
 * writelen 0.  E' la forma "sola lettura" di fts_i2c_read.
 */
int fts_flash_read_buf(struct i2c_client *client, u32 saddr, u8 *buf, u32 len)
{
	int ret = 0;
	u32 i = 0;
	u32 packet_number = 0;
	u32 packet_len = 0;
	u32 addr = 0;
	u32 offset = 0;
	u32 remainder = 0;
	u8 packet_buf[FTS_CMD_READ_LEN];

	if ((NULL == buf) || (0 == len)) {
		FTS_ERROR("buf is NULL or len is 0");
		return -EINVAL;
	}

	packet_number = len / FTS_FLASH_PACKET_LENGTH;
	remainder = len % FTS_FLASH_PACKET_LENGTH;
	if (remainder > 0)
		packet_number++;
	packet_len = FTS_FLASH_PACKET_LENGTH;
	FTS_INFO("read packet_number:%d, remainder:%d", packet_number, remainder);

	packet_buf[0] = FTS_CMD_READ;
	for (i = 0; i < packet_number; i++) {
		offset = i * FTS_FLASH_PACKET_LENGTH;
		addr = saddr + offset;
		packet_buf[1] = (u8)((addr >> 16) & 0xFF);
		packet_buf[2] = (u8)((addr >> 8) & 0xFF);
		packet_buf[3] = (u8)(addr & 0xFF);

		if ((i == (packet_number - 1)) && remainder)
			packet_len = remainder;

		ret = fts_i2c_write(client, packet_buf, FTS_CMD_READ_LEN);
		if (ret < 0) {
			FTS_ERROR("pram/bootloader write 03 command fail");
			return ret;
		}

		msleep(FTS_CMD_READ_DELAY);

		ret = fts_i2c_read(client, NULL, 0, buf + offset, packet_len);
		if (ret < 0) {
			FTS_ERROR("pram/bootloader read 03 command fail");
			return ret;
		}
	}

	return 0;
}

/*
 * ===========================================================================
 * fts_flash_read @ 0xffffff8008a7e184, 280 byte
 * ===========================================================================
 * FIRMA:
 *   x0 client -- "aa0003f3 mov"@0xffffff8008a7e1a0 -> x19
 *   w1 addr   -- "2a0103f6 mov"@0xffffff8008a7e1b4 -> w22, ripassato tale e
 *                quale a fts_flash_read_buf da "2a1603e1 mov"@0xffffff8008a7e1d8
 *   x2 buf    -- "aa0203f5 mov"@0xffffff8008a7e1b0 -> x21, `cbz` a
 *                "b40001d5 cbz"@0xffffff8008a7e1c0
 *   w3 len    -- "2a0303f4 mov"@0xffffff8008a7e1ac -> w20, `cbz` a
 *                "340001b4 cbz"@0xffffff8008a7e1c4
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x016[FTS][Info]***********read flash***********\n"@0xffffff800924eaf5
 *   "\x013[FTS][Error]buf is NULL or len is 0\n"@0xffffff800924e9b6
 *   "\x013[FTS][Error]enter into pramboot/bootloader fail\n"@0xffffff800924eb24
 *   "\x013[FTS][Error]read flash fail\n"@0xffffff800924eb57
 *   "\x013[FTS][Error]reset to normal boot fail\n"@0xffffff800924eb76
 * L'ultimo NON e' della copia incorporata: vedi il punto (3) qui sotto.
 *
 * IL BANNER PRECEDE IL CONTROLLO: la "97dad4c6 bl"@0xffffff8008a7e1bc verso
 * printk, col formato assemblato citato qui sopra a 0xffffff800924eaf5, sta
 * PRIMA dei due `cbz`.  E' il contrario di fts_flash_read_buf.
 *
 * ===========================================================================
 * LA CODA E' UNA COPIA INCORPORATA DI fts_fwupg_reset_in_boot
 * ===========================================================================
 * Questo e' il fatto che governa la misura di questa funzione, ed e' MISURATO,
 * non supposto.  Tre prove indipendenti:
 *
 * (1) L'ELENCO DELLE CHIAMATE non contiene quel simbolo:
 *       $ ./venv/bin/python3 disassembla.py oracolo/stock.map oracolo/stock.elf \
 *             fts_flash_read
 *       fts_flash_read: 0xffffff8008a7e184, 280 byte
 *       istruzioni: 70
 *       chiama    : printk fts_fwupg_enter_into_boot fts_flash_read_buf fts_i2c_write msleep __stack_chk_fail
 *     C'e' `fts_i2c_write` e c'e' `msleep`, ma NON c'e' fts_fwupg_reset_in_boot.
 *
 * (2) LE ISTRUZIONI COINCIDONO UNA A UNA con il corpo della funzione fuori
 *     linea, che ha simbolo proprio a 0xffffff8008a7d838:
 *       fuori linea                          incorporata
 *       "32000be9 orr"@0xffffff8008a7d858    "32000be8 orr"@0xffffff8008a7e21c  (w=7)
 *       "9117fc00 add"@0xffffff8008a7d85c    "9117fc00 add"@0xffffff8008a7e220  (0x5ff)
 *       "390013e9 strb"@0xffffff8008a7d864   "390013e8 strb"@0xffffff8008a7e224
 *       "910013e1 add"@0xffffff8008a7d86c    "910013e1 add"@0xffffff8008a7e22c
 *       "320003e2 orr"@0xffffff8008a7d870    "320003e2 orr"@0xffffff8008a7e230  (w2=1)
 *       "37f800a0 tbnz"@0xffffff8008a7d87c   "37f800a0 tbnz"@0xffffff8008a7e23c
 *       "52800a00 mov"@0xffffff8008a7d880    "52800a00 mov"@0xffffff8008a7e240  (80)
 *       "91189c00 add"@0xffffff8008a7d898    "91189c00 add"@0xffffff8008a7e258  (0x627)
 *     Stessi immediati, stesse stringhe
 *     ("\x016[FTS][Info]reset in boot environment\n"@0xffffff800924e5ff e
 *      "\x013[FTS][Error]pram/rom/bootloader reset cmd write fail\n"@0xffffff800924e627),
 *     stesso ordine.
 *
 * (3) IL MESSAGGIO CHE SEGUE non appartiene a quella funzione:
 *     "\x013[FTS][Error]reset to normal boot fail\n"@0xffffff800924eb76 e'
 *     stampato da "97dad49b bl"@0xffffff8008a7e268, subito DOPO il ramo
 *     d'errore della copia incorporata, e non compare fra le costanti di
 *     fts_fwupg_reset_in_boot (che sono 0x5ff e 0x627 e basta).  E' il
 *     messaggio del chiamante.
 *
 * CONSEGUENZA SULLA MISURA -- ERA LA DIVERGENZA 1 DEL LOTTO B1, ED E' CHIUSA.
 * Finche' fts_fwupg_reset_in_boot non era DEFINITA in questo file clang non
 * aveva niente da incorporare ed emetteva una `bl` verso un simbolo esterno:
 * questa funzione misurava 180 byte invece di 280, perche' mancavano il corpo
 * della copia e -- sparito l'unico buffer locale -- il canarino e mezzo
 * telaio.  Il lotto B2 ha definito fts_fwupg_reset_in_boot qui sopra e la
 * misura e' tornata 280 esatti con residuo di codifica ZERO (vedi il blocco
 * in coda).
 *
 * IL VALORE DI RITORNO PERDE L'ERRORE, ED E' UN DIFETTO DI FABBRICA CHE SI
 * RIPRODUCE (regola 7).  Se fts_fwupg_enter_into_boot o fts_flash_read_buf
 * falliscono, il salto va a 0xffffff8008a7e218 (la copia incorporata) e da
 * li', se la scrittura del comando di reset riesce, si passa da
 * "2a1f03f3 mov"@0xffffff8008a7e248, che mette w19 = 0, e si ritorna ZERO.
 * L'errore e' stato stampato ma non e' propagato.  Il `ret` del sorgente e'
 * riassegnato dal reset, e questo e' quello che il binario fa.
 */
int fts_flash_read(struct i2c_client *client, u32 addr, u8 *buf, u32 len)
{
	int ret = 0;

	FTS_INFO("***********read flash***********");
	if ((NULL == buf) || (0 == len)) {
		FTS_ERROR("buf is NULL or len is 0");
		return -EINVAL;
	}

	ret = fts_fwupg_enter_into_boot(client);
	if (ret < 0) {
		FTS_ERROR("enter into pramboot/bootloader fail");
		goto read_flash_err;
	}

	ret = fts_flash_read_buf(client, addr, buf, len);
	if (ret < 0) {
		FTS_ERROR("read flash fail");
		goto read_flash_err;
	}

read_flash_err:
	ret = fts_fwupg_reset_in_boot(client);
	if (ret < 0)
		FTS_ERROR("reset to normal boot fail");

	return ret;
}

/*
 * ===========================================================================
 * fts_read_file @ 0xffffff8008a7e29c, 456 byte
 * ===========================================================================
 * FIRMA, letta dai registri d'ingresso e dal loro uso:
 *   x0 file_name -- "b40002e0 cbz"@0xffffff8008a7e2e0 lo confronta con NULL e
 *                   "aa0003e4 mov"@0xffffff8008a7e2f4 lo passa come QUINTO
 *                   argomento (x4) della snprintf, cioe' come un `%s`
 *   x1 file_buf  -- "aa0103f3 mov"@0xffffff8008a7e2e4 -> x19,
 *                   "b40002a1 cbz"@0xffffff8008a7e2e8 lo confronta con NULL, e
 *                   "f9000260 str"@0xffffff8008a7e390 ci SCRIVE DENTRO il
 *                   risultato della vmalloc: e' un puntatore a puntatore, non
 *                   un buffer.  La rilettura "f9400261 ldr"@0xffffff8008a7e3c4
 *                   lo riprende per darlo alla vfs_read
 * Il valore di ritorno e' `int`: "2a1303e0 mov"@0xffffff8008a7e360 mette in w0
 * il w19 accumulato, e i tre codici d'errore sono a 32 bit.
 *
 * IL CONTROLLO E' `||`, DUE SALTI ALLO STESSO ERRORE, e in mezzo c'e' il `mov`
 * del secondo argomento:
 *   "b40002e0 cbz"@0xffffff8008a7e2e0   file_name
 *   "b40002a1 cbz"@0xffffff8008a7e2e8   file_buf
 * tutti e due verso 0xffffff8008a7e33c, che stampa
 * "\x013[FTS][Error]filename/filebuf is NULL\n"@0xffffff800924eb9f e mette
 * "128002b3 mov"@0xffffff8008a7e348, w19 = -22 = -EINVAL.
 *
 * IL PERCORSO E' COMPOSTO DA UNA snprintf CON DUE `%s`, e i quattro registri
 * dicono quale argomento e' quale:
 *   "910023e0 add"@0xffffff8008a7e300   x0 = &file_path, il buffer
 *   "321903e1 orr"@0xffffff8008a7e304   w1 = 0x80 = 128, la sua taglia
 *   "91029442 add"@0xffffff8008a7e2f8   x2 = "%s%s"@0xffffff80091760a5
 *   "912f1c63 add"@0xffffff8008a7e2fc   x3 = "/sdcard/"@0xffffff800924ebc7
 *   "aa0003e4 mov"@0xffffff8008a7e2f4   x4 = file_name
 * IL PREFISSO E' UN LETTERALE DEL BINARIO, non un nome inventato: la stringa
 * "/sdcard/" sta a 0xffffff800924ebc7 fra i messaggi di questa funzione, e il
 * formato "%s%s" sta in un'altra pagina di `.rodata` (0xffffff8009176000)
 * perche' e' deduplicato con tutto il kernel.
 *
 * IL BUFFER E' DI 128 BYTE AZZERATI, e le due cose sono misurate a parte: la
 * taglia dalla `w1 = 0x80` della snprintf, l'azzeramento dalle OTTO coppie
 *   "a900ffff stp"@0xffffff8008a7e2dc .. "a907ffff stp"@0xffffff8008a7e2c0
 * che coprono sp+8..sp+135, cioe' 16 registri per 8 byte = 128.  L'azzeramento
 * sta PRIMA dei due `cbz`, cioe' e' un inizializzatore di dichiarazione.
 *
 * IS_ERR, NON UN CONFRONTO CON NULL:
 *   "b13ffc1f cmn"@0xffffff8008a7e31c   filp + 0xfff
 *   "540002e3 b.cc"@0xffffff8008a7e320  riporto pulito -> NON e' un errore
 * `cmn x0,#0xfff` mette il riporto quando x0 >= -4095 senza segno, che e'
 * esattamente `IS_ERR(filp)`.  Un `NULL == filp` avrebbe dato un `cbz`.
 * Il ramo d'errore stampa
 * "\x013[FTS][Error]open %s file fail\n"@0xffffff800924ebd0 con
 * "910023e1 add"@0xffffff8008a7e32c, cioe' con file_path e non con file_name,
 * e ritorna "321f7bf3 orr"@0xffffff8008a7e334, w19 = -2 = -ENOENT.
 *
 * I DUE MODI DELLA filp_open SONO ENTRAMBI ZERO:
 *   "2a1f03e1 mov"@0xffffff8008a7e310   w1 = 0 = O_RDONLY
 *   "2a1f03e2 mov"@0xffffff8008a7e314   w2 = 0
 *
 * LA TAGLIA ARRIVA DALL'INODE, e i due scostamenti sono quelli del kernel:
 *   "f9401008 ldr"@0xffffff8008a7e37c   x8 = filp->f_inode   (+32)
 *   "f9402916 ldr"@0xffffff8008a7e384   x22 = inode->i_size  (+80)
 * Non do' un nome inventato a nessuno dei due: sono campi di due struct del
 * KERNEL, quindi il sorgente li nomina come li nomina il kernel e il
 * compilatore ricalcola gli scostamenti da solo.  CHE I DUE SCOSTAMENTI
 * TORNINO E' LA MISURA: se `f_inode` non stesse a +32 o `i_size` non stesse a
 * +80, le due `ldr` uscirebbero con un immediato diverso e il confronto per
 * codifica lo direbbe.
 *
 * LA TAGLIA E' A 64 BIT: "aa1603e0 mov"@0xffffff8008a7e388 la passa intera a
 * vmalloc e "aa1603e2 mov"@0xffffff8008a7e3d0 intera a vfs_read; sono `mov x`,
 * non `mov w`.  E' `loff_t`, non `int`.
 *
 * LA COPPIA get_fs/set_fs E' RICONOSCIBILE UNA A UNA, ed e' la ragione per cui
 * questa funzione chiama `set_bit`:
 *   "d5384115 mrs"@0xffffff8008a7e39c    x21 = sp_el0, cioe' `current`
 *   "f94006b7 ldr"@0xffffff8008a7e3a0    x23 = ti->addr_limit  (+8) = get_fs()
 *   "92800008 mov"@0xffffff8008a7e398    x8 = -1 = KERNEL_DS
 *   "f90006a8 str"@0xffffff8008a7e3a4    ti->addr_limit = KERNEL_DS
 *   "d503379f dsb"@0xffffff8008a7e3a8    \ spec_bar()
 *   "d5033fdf isb"@0xffffff8008a7e3ac    /
 *   "528000a0 mov"@0xffffff8008a7e3b0    w0 = 5 = TIF_FSCHECK
 *   "aa1503e1 mov"@0xffffff8008a7e3b4    x1 = &ti->flags  (+0)
 *   "940f62cd bl"@0xffffff8008a7e3b8     set_bit
 *   "d503201f nop"@0xffffff8008a7e3bc    l'ALTERNATIVE di UAO, gia' risolta
 * e la seconda copia, quella con l'argomento variabile, ha in piu' il
 * confronto che la prima non ha perche' KERNEL_DS e' una costante:
 *   "b10006ff cmn"@0xffffff8008a7e428    old_fs == -1 ?
 *   "54000160 b.eq"@0xffffff8008a7e42c   -> l'altro ramo dell'ALTERNATIVE
 *   "d503201f nop"@0xffffff8008a7e430  e "d503201f nop"@0xffffff8008a7e458
 * Sono le due espansioni di `set_fs()` dell'arm64 di questo albero: nessuna
 * riga di sorgente in piu' le produce.
 *
 * L'ORDINE DELLE TRE COSE ALLA FINE E' filp_close, POI set_fs, e si vede:
 *   "97df9ed5 bl"@0xffffff8008a7e40c    filp_close
 *   "f90006b7 str"@0xffffff8008a7e410   ti->addr_limit = old_fs
 * Sul ramo della vmalloc fallita, invece, il set_fs NON c'e' affatto -- il
 * blocco 0xffffff8008a7e438..0xffffff8008a7e454 e' stampa, filp_close, -12 --
 * perche' il set_fs(KERNEL_DS) viene DOPO la vmalloc.
 *
 * L'ERRORE DELLA vfs_read NON INTERROMPE NIENTE:
 *   "36f80093 tbz"@0xffffff8008a7e3dc   se ret >= 0 salta la stampa
 *   "97dad43b bl"@0xffffff8008a7e3e8    "read file fail"
 * e subito dopo si stampa comunque il riepilogo e si chiude il file.  E' un
 * difetto di fabbrica e si riproduce (regola 7): `ret` negativo viene
 * restituito, ma il buffer resta allocato e il chiamante non lo sa dal codice
 * di ritorno del solo `if`.
 *
 * I TRE ARGOMENTI DEL RIEPILOGO SONO TRONCATI A 32 BIT, e due dei tre sono
 * valori a 64 bit:
 *   "2a1603e1 mov"@0xffffff8008a7e3f8   w1 = (u32)file_len   (x22 e' loff_t)
 *   "2a1303e2 mov"@0xffffff8008a7e3fc   w2 = ret
 *   "b94003e3 ldr"@0xffffff8008a7e3ec   w3 = (u32)pos, `ldr w` e non `ldr x`
 * Con un `%d` su un `loff_t` il compilatore passerebbe il registro intero: la
 * `mov w`/`ldr w` prova che il sorgente TRONCA. Il verso del troncamento --
 * (u32) o (int) -- non e' distinguibile dal binario, ed e' una SCELTA
 * dichiarata: qui e' (u32), e le due forme danno la stessa codifica.
 *
 * IL CANARINO C'E' perche' c'e' il buffer `file_path`:
 *   "f81c83a8 stur"@0xffffff8008a7e2bc  lo deposita
 *   "eb08013f cmp"@0xffffff8008a7e358   lo ricontrolla
 *   "97d8bf80 bl"@0xffffff8008a7e460    __stack_chk_fail
 *
 * I MESSAGGI DI QUESTA FUNZIONE, in ordine di indirizzo nel `.rodata` -- ed e'
 * l'ordine in cui il sorgente li scrive:
 *   "\x013[FTS][Error]filename/filebuf is NULL\n"@0xffffff800924eb9f
 *   "/sdcard/"@0xffffff800924ebc7
 *   "\x013[FTS][Error]open %s file fail\n"@0xffffff800924ebd0
 *   "\x013[FTS][Error]file buf malloc fail\n"@0xffffff800924ebf1
 *   "\x013[FTS][Error]read file fail\n"@0xffffff800924ec15
 *   "\x016[FTS][Info]file len:%d read len:%d pos:%d\n"@0xffffff800924ec33
 */
int fts_read_file(char *file_name, u8 **file_buf)
{
	int ret = 0;
	char file_path[128] = { 0 };
	struct file *filp = NULL;
	struct inode *inode;
	mm_segment_t old_fs;
	loff_t pos;
	loff_t file_len = 0;

	if ((NULL == file_name) || (NULL == file_buf)) {
		FTS_ERROR("filename/filebuf is NULL");
		return -EINVAL;
	}

	snprintf(file_path, 128, "%s%s", "/sdcard/", file_name);
	filp = filp_open(file_path, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		FTS_ERROR("open %s file fail", file_path);
		return -ENOENT;
	}

	inode = filp->f_inode;
	file_len = inode->i_size;
	*file_buf = vmalloc(file_len);
	if (NULL == *file_buf) {
		FTS_ERROR("file buf malloc fail");
		filp_close(filp, NULL);
		return -ENOMEM;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	ret = vfs_read(filp, *file_buf, file_len, &pos);
	if (ret < 0)
		FTS_ERROR("read file fail");
	FTS_INFO("file len:%d read len:%d pos:%d", (u32)file_len, ret, (u32)pos);
	filp_close(filp, NULL);
	set_fs(old_fs);

	return ret;
}

/*
 * ===========================================================================
 * fts_upgrade_bin @ 0xffffff8008a7e464, 412 byte
 * ===========================================================================
 * FIRMA, dai registri d'ingresso:
 *   x0 client  -- "aa0003f4 mov"@0xffffff8008a7e480 -> x20, ripassato tale e
 *                 quale alla chiamata indiretta,
 *                 "aa1403e0 mov"@0xffffff8008a7e540
 *   x1 fw_name -- "aa0103f3 mov"@0xffffff8008a7e49c -> x19, primo argomento
 *                 di fts_read_file, "aa1303e0 mov"@0xffffff8008a7e4b8
 *   w2 force   -- "2a0203f5 mov"@0xffffff8008a7e498 -> w21, provato UN BIT
 *                 SOLO da "360000d5 tbz"@0xffffff8008a7e51c: e' un `bool`,
 *                 non un `int` (classe A4).  L'unita' A lo dichiara cosi'
 *                 anche dal suo lato -- focaltech_sysfs.c riga 254,
 *                 `int fts_upgrade_bin(struct i2c_client *, char *, bool)` --
 *                 e i due siti di chiamata passano 0 e 1
 *
 * IL GLOBALE E IL PUNTATORE LOCALE SONO INIZIALIZZATI PRIMA DEL BANNER, come
 * in fts_pram_write_init:
 *   "f9456516 ldr"@0xffffff8008a7e494   x22 = fts_g_a100ac8
 *   "f90007ff str"@0xffffff8008a7e4a0   fw_file_buf = NULL, a sp+8
 *   "97dad40c bl"@0xffffff8008a7e4a4    e SOLO DOPO la printk del banner
 *
 * IL CONTROLLO E' `||`, e il messaggio e' lo STESSO di fts_pram_write_init --
 * una sola stringa deduplicata a 0x2bf:
 *   "b4000276 cbz"@0xffffff8008a7e4a8   upg
 *   "b4000228 cbz"@0xffffff8008a7e4b0   upg->c32
 * e il ramo NON passa dalla coda che libera il buffer: mette
 * "128002b3 mov"@0xffffff8008a7e500, w19 = -22 = -EINVAL, e salta DRITTO
 * all'epilogo con "14000030 b"@0xffffff8008a7e504.  E' un `return`, non un
 * `goto`.
 *
 * IL CONTROLLO DI LUNGHEZZA E' UN INTERVALLO CHIUSO, riscritto dal
 * compilatore come un solo confronto senza segno:
 *   "51048008 sub"@0xffffff8008a7e4c4   w8 = ret - 0x120
 *   "529fdc29 mov"@0xffffff8008a7e4c0   \ w9 = 0x1fee1
 *   "72a00029 movk"@0xffffff8008a7e4c8  /
 *   "6b09011f cmp"@0xffffff8008a7e4d0   w8 contro w9
 *   "540001a3 b.cc"@0xffffff8008a7e4d4  minore SENZA segno -> lunghezza buona
 * 0x1fee1 = 0x20000 - 0x120 + 1, cioe' l'intervallo e' [0x120, 0x20000].
 * E' la stessa forma gia' vista in fts_pram_write_remap
 * ("51048288 sub"@0xffffff8008a7ce20 + "6b09011f cmp"@0xffffff8008a7ce28), la'
 * col limite alto 0x10000.  I DUE NUMERI RESTANO NUMERI qui, come alla riga
 * 663 di questo file; l'unita' F li nomina FTS_MIN_LEN e FTS_MAX_LEN_APP con
 * gli stessi valori, ma il nome e' suo e non del binario.
 *
 * UNA VARIABILE SOLA PORTA LA LUNGHEZZA E POI IL `ret`, e si vede da tre
 * punti che usano lo stesso w19:
 *   "2a0003f3 mov"@0xffffff8008a7e4cc   w19 = fts_read_file(...)
 *   "2a1303e1 mov"@0xffffff8008a7e510   w1 = w19, l'argomento di "fw bin
 *                                       file len:%d"
 *   "2a1303e2 mov"@0xffffff8008a7e544   w2 = w19, la LUNGHEZZA passata alla
 *                                       funzione di aggiornamento
 *   "2a0003f3 mov"@0xffffff8008a7e54c   w19 = il valore di ritorno di quella
 * Se le variabili fossero due, sul ramo "upgrade function is null" si
 * ritornerebbe lo zero dell'inizializzatore; il binario ritorna invece la
 * LUNGHEZZA, perche' w19 non e' stato toccato.
 *
 * IL BIVIO `force` E' ASIMMETRICO, ED E' UN DIFETTO DI FABBRICA CHE SI
 * RIPRODUCE (regola 7).  I due rami "funzione assente" non fanno la stessa
 * cosa:
 *   "b50000c8 cbnz"@0xffffff8008a7e524   force: c128 c'e' -> chiama
 *   "14000020 b"@0xffffff8008a7e530      force: c128 manca -> stampa e
 *                                        SALTA la stampa di successo
 *   "b4000328 cbz"@0xffffff8008a7e538    non-force: c96 manca -> 0xa7e59c
 * e il blocco 0xffffff8008a7e59c stampa
 * "\x016[FTS][Info]upgrade function is null, no upgrade\n"@0xffffff800924ed17
 * e poi CADE DENTRO 0xffffff8008a7e5a8, che carica
 * "\x016[FTS][Info]upgrade fw bin success\n"@0xffffff800924ed6f.  Senza `force`
 * e senza funzione di aggiornamento la fabbrica stampa TUTTI E DUE i
 * messaggi.  Che sia una caduta e non un salto lo prova l'assenza del
 * controllo `ret < 0` su quel percorso: il compilatore sa gia' che w19 sta in
 * [0x120, 0x20000] e lo ha eliminato.
 *
 * LA CODA E' UNA COPIA INCORPORATA DI fts_fwupg_reset_in_boot, la seconda
 * dell'unita' dopo quella di fts_flash_read.  Le costanti sono le stesse:
 *   "32000be8 orr"@0xffffff8008a7e564   w8 = 7
 *   "390053e8 strb"@0xffffff8008a7e56c  cmd = 7, a sp+20
 *   "910053e1 add"@0xffffff8008a7e574   x1 = &cmd
 *   "320003e2 orr"@0xffffff8008a7e578   w2 = 1, la writelen
 *   "52800a00 mov"@0xffffff8008a7e588   w0 = 0x50 = 80, la msleep
 * e le due stringhe sono quelle di quella funzione, non di questa:
 *   "\x016[FTS][Info]reset in boot environment\n"@0xffffff800924e5ff
 *   "\x013[FTS][Error]pram/rom/bootloader reset cmd write fail\n"@0xffffff800924e627
 * IL VALORE DI RITORNO DELLA COPIA E' IGNORATO: dopo la
 * "97db51a2 bl"@0xffffff8008a7e58c (msleep) si va dritti a rileggere il
 * buffer con "f94007e0 ldr"@0xffffff8008a7e590, e w19 resta quello negativo
 * dell'aggiornamento fallito.
 *
 * LA LIBERAZIONE E' CONDIZIONATA E AZZERA, e le due cose sono separate:
 *   "b4000060 cbz"@0xffffff8008a7e5b8   se il buffer e' NULL salta
 *   "97df0f1f bl"@0xffffff8008a7e5bc    vfree
 *   "f90007ff str"@0xffffff8008a7e5c0   fw_file_buf = NULL
 * L'azzeramento dopo la vfree e' morto (la variabile non e' piu' letta) e il
 * compilatore lo tiene lo stesso perche' l'indirizzo del locale e' uscito
 * dalla funzione: e' una riga del sorgente, non un artefatto.
 *
 * I MESSAGGI DI QUESTA FUNZIONE, in ordine di indirizzo nel `.rodata`:
 *   "\x013[FTS][Error]upgrade/func is null\n"@0xffffff800924e2bf   (deduplicato)
 *   "\x016[FTS][Info]start upgrade with fw bin\n"@0xffffff800924ec60
 *   "\x013[FTS][Error]read fw bin file(sdcard) fail, len:%d\n"@0xffffff800924ec88
 *   "\x016[FTS][Info]fw bin file len:%d\n"@0xffffff800924ecbd
 *   "\x016[FTS][Info]force_upgrade function is null, no upgrade\n"@0xffffff800924ecde
 *   "\x016[FTS][Info]upgrade function is null, no upgrade\n"@0xffffff800924ed17
 *   "\x013[FTS][Error]upgrade fw bin failed\n"@0xffffff800924ed4a
 *   "\x016[FTS][Info]upgrade fw bin success\n"@0xffffff800924ed6f
 * L'ordine dei sette non deduplicati e' l'ordine in cui il sorgente li scrive.
 */
int fts_upgrade_bin(struct i2c_client *client, char *fw_name, bool force)
{
	int ret = 0;
	u8 *fw_file_buf = NULL;
	struct fts_upgrade *upg = fts_g_a100ac8;

	FTS_INFO("start upgrade with fw bin");
	if ((NULL == upg) || (NULL == upg->c32)) {
		FTS_ERROR("upgrade/func is null");
		return -EINVAL;
	}

	ret = fts_read_file(fw_name, &fw_file_buf);
	if ((ret < 0x120) || (ret > 0x20000)) {
		FTS_ERROR("read fw bin file(sdcard) fail, len:%d", ret);
		goto err_bin;
	}

	FTS_INFO("fw bin file len:%d", ret);
	if (force) {
		if (upg->c32->c128) {
			ret = upg->c32->c128(client, fw_file_buf, ret);
		} else {
			FTS_INFO("force_upgrade function is null, no upgrade");
			goto err_bin;
		}
	} else {
		if (upg->c32->c96) {
			ret = upg->c32->c96(client, fw_file_buf, ret);
		} else {
			FTS_INFO("upgrade function is null, no upgrade");
		}
	}

	if (ret < 0) {
		FTS_ERROR("upgrade fw bin failed");
		fts_fwupg_reset_in_boot(client);
		goto err_bin;
	}

	FTS_INFO("upgrade fw bin success");

err_bin:
	if (fw_file_buf) {
		vfree(fw_file_buf);
		fw_file_buf = NULL;
	}

	return ret;
}

/*
 * ===========================================================================
 * LE SEI FUNZIONI `static` DI fts_fwupg_upgrade, CHE LA MAPPA NON HA PERCHE'
 * DI FABBRICA SONO TUTTE INCORPORATE
 * ===========================================================================
 * fts_fwupg_upgrade misura 1216 byte e la mappa non ha NESSUN altro simbolo
 * fra 0xffffff8008a7e600 e la fine dell'unita' a 0xffffff8008a7eac0.  Dentro
 * quei 1216 byte ci sono pero' DUE LIVELLI di messaggi incatenati -- il
 * messaggio del chiamato e subito dopo quello del chiamante -- e la catena e'
 * la prova che le funzioni ci sono.  Tre catene, tutte e tre di due `bl`
 * consecutive verso printk senza niente in mezzo se non il caricamento della
 * seconda stringa:
 *
 *   "97dad29b bl"@0xffffff8008a7ea68   "read fw ver from tp fail"
 *   "97dad2c1 bl"@0xffffff8008a7e9d0   "get fw ver in tp fail"
 *
 *   "97dad2c8 bl"@0xffffff8008a7e9b4   "fw len(%x) < paramcfg ver offset(%x)"
 *   "97dad2c1 bl"@0xffffff8008a7e9d0   "param version in host invalid" (ERR)
 *
 *   "97dad292 bl"@0xffffff8008a7ea8c   "param version in tp invalid"
 *   "97dad2c1 bl"@0xffffff8008a7e9d0   "get IDE param ver in tp fail"
 *
 * LA SECONDA PROVA E' UNA COPPIA DI MESSAGGI CON LO STESSO IDENTICO TESTO E
 * LIVELLO DIVERSO.  Il binario porta tutti e due:
 *   "\x016[FTS][Info]param version in host invalid\n"@0xffffff800924f563
 *   "\x013[FTS][Error]param version in host invalid\n"@0xffffff800924f3ee
 * Una funzione sola non stampa lo stesso testo due volte con due livelli
 * diversi; due funzioni si', quando il chiamato lo dice come informazione e
 * il chiamante come errore.  E infatti sono stampati UNO DOPO L'ALTRO sulla
 * stessa caduta: "97dad2c4 bl"@0xffffff8008a7e9c4 (INFO) e
 * "97dad2c1 bl"@0xffffff8008a7e9d0 (ERR).
 *
 * LA TERZA PROVA E' UN BLOCCO DUPLICATO: la lettura del registro 0xa6 con la
 * stampa "read fw ver from tp fail" compare DUE VOLTE, a
 * "9400073c bl"@0xffffff8008a7e924 e a "94000729 bl"@0xffffff8008a7e970, con
 * bersagli di stack diversi (sp+4 e sp+0).  Sono due incorporazioni della
 * stessa funzione in due punti, non due copie scritte a mano.  Lo stesso vale
 * per il registro 0xb5, "94000775 bl"@0xffffff8008a7e840 e
 * "940006f6 bl"@0xffffff8008a7ea3c.
 *
 * I NOMI DI QUESTE SEI FUNZIONI, DEI LORO PARAMETRI E IL LORO ORDINE NEL FILE
 * SONO UNA SCELTA, NON UNA MISURA (regola 4): una funzione incorporata non
 * lascia simbolo.  Cio' che e' MISURATO e' che esistono (le catene qui sopra),
 * quali sono i loro confini e che cosa fanno.  ANCHE I LORO CODICI D'ERRORE
 * SONO UNA SCELTA: il chiamante prova solo il segno (`ret < 0`), quindi il
 * binario non distingue -EINVAL da -EIO da qualunque altro negativo.  Sono
 * scritti come -EINVAL e -EIO e vanno letti come "un negativo".
 *
 * La misura che verifica l'incorporazione e' NEGATIVA e sta nell'elenco dei
 * simboli in coda al file: nel nostro oggetto ci sono DICIASSETTE simboli di
 * funzione, non ventinove.
 */

/*
 * ---------------------------------------------------------------------------
 * fts_fwupg_get_ver_in_host -- incorporata dentro fts_fwupg_upgrade
 * ---------------------------------------------------------------------------
 * I TRE CONTROLLI DI NULLITA' SONO UN `||` DI TRE, e l'ordine e' quello del
 * binario:
 *   "b4000255 cbz"@0xffffff8008a7e674   upg
 *   "b4000208 cbz"@0xffffff8008a7e67c   upg->c32
 *   "b40001c9 cbz"@0xffffff8008a7e684   upg->c0
 * Il messaggio ne elenca CINQUE ("fts_data/upgrade/func/fw/ver is NULL") e i
 * controlli sono tre: e' un difetto di fabbrica e si riproduce (regola 7).
 *
 * IL CONFRONTO DI LUNGHEZZA E' SENZA SEGNO e il verso e' quello che il
 * binario sceglie:
 *   "b9400aa1 ldr"@0xffffff8008a7e688   w1 = upg->c8
 *   "b9402502 ldr"@0xffffff8008a7e68c   w2 = upg->c32->c36
 *   "6b02003f cmp"@0xffffff8008a7e690
 *   "54001322 b.cs"@0xffffff8008a7e694  >= -> prosegue
 * I DUE ARGOMENTI DELLA STAMPA D'ERRORE SONO GIA' NEI REGISTRI: la
 * "97dad38d bl"@0xffffff8008a7e6a0 non ricarica niente, quindi il formato
 * "fw len(0x%0x) < fw ver offset(0x%x)" riceve (lunghezza, scostamento) in
 * quest'ordine.  Il `%0x` senza cifra di larghezza e' un refuso di fabbrica e
 * si riproduce.
 *
 * I CAMPI SONO RILETTI DOPO LA STAMPA, non tenuti in un locale:
 *   "f94012a8 ldr"@0xffffff8008a7e908   upg->c32, di nuovo
 *   "f94002a9 ldr"@0xffffff8008a7e90c   upg->c0, di nuovo
 *   "b9402508 ldr"@0xffffff8008a7e918   ->c36, di nuovo
 *   "38686935 ldrb"@0xffffff8008a7e920  upg->c0[scostamento]
 * Se lo scostamento fosse stato messo in una variabile locale, clang lo
 * avrebbe tenuto in un registro callee-saved attraverso la printk invece di
 * ricaricarlo tre volte (classe A2 al contrario).  Il sorgente rilegge il
 * campo ogni volta che lo nomina.
 *
 * I MESSAGGI DI QUESTA FUNZIONE, per intero come stanno nel binario:
 *   "\x013[FTS][Error]fts_data/upgrade/func/fw/ver is NULL\n"@0xffffff800924f309
 *   "\x013[FTS][Error]fw len(0x%0x) < fw ver offset(0x%x)\n"@0xffffff800924f33d
 *   "\x016[FTS][Info]fw version offset:0x%x\n"@0xffffff800924f370
 */
static int fts_fwupg_get_ver_in_host(u8 *ver)
{
	struct fts_upgrade *upg = fts_g_a100ac8;

	if ((NULL == upg) || (NULL == upg->c32) || (NULL == upg->c0)) {
		FTS_ERROR("fts_data/upgrade/func/fw/ver is NULL");
		return -EINVAL;
	}

	if (upg->c8 < upg->c32->c36) {
		FTS_ERROR("fw len(0x%0x) < fw ver offset(0x%x)", upg->c8,
			  upg->c32->c36);
		return -EINVAL;
	}

	FTS_INFO("fw version offset:0x%x", upg->c32->c36);
	*ver = upg->c0[upg->c32->c36];

	return 0;
}

/*
 * ---------------------------------------------------------------------------
 * fts_fwupg_get_ver_in_tp -- incorporata dentro fts_fwupg_upgrade, DUE VOLTE
 * ---------------------------------------------------------------------------
 * IL REGISTRO E' 0xa6 ED E' NOMINATO DAL BINARIO: l'unita' A stampa il suo
 * valore con "FW Ver:0x%02x\n"@0xffffff800924e02f, quindi il nome
 * FTS_REG_FW_VER non e' inventato (eccezione della regola 5).
 *   "528014c1 mov"@0xffffff8008a7e914   w1 = 0xa6   (prima copia)
 *   "528014c1 mov"@0xffffff8008a7e968   w1 = 0xa6   (seconda copia)
 * IL TERZO ARGOMENTO E' UN PUNTATORE A UN BYTE DEL CHIAMANTE, e le due copie
 * usano due slot diversi:
 *   "910013e2 add"@0xffffff8008a7e910   x2 = sp+4
 *   "910003e2 mov"@0xffffff8008a7e964   x2 = sp+0
 *
 * IL MESSAGGIO DI QUESTA FUNZIONE:
 *   "\x013[FTS][Error]read fw ver from tp fail\n"@0xffffff800924f395
 */
static int fts_fwupg_get_ver_in_tp(struct i2c_client *client, u8 *ver)
{
	int ret = 0;

	ret = fts_i2c_read_reg(client, FTS_REG_FW_VER, ver);
	if (ret < 0) {
		FTS_ERROR("read fw ver from tp fail");
		return ret;
	}

	return 0;
}

/*
 * ---------------------------------------------------------------------------
 * fts_fwupg_need_upgrade -- incorporata dentro fts_fwupg_upgrade
 * ---------------------------------------------------------------------------
 * LA TESTA E' fts_fwupg_check_fw_valid INCORPORATA: non c'e' nessuna `bl`
 * verso quel simbolo, c'e' la sua `bl` verso fts_wait_tp_to_valid e le sue due
 * stringhe.
 *   "9400011a bl"@0xffffff8008a7e658   fts_wait_tp_to_valid
 *   "37f80400 tbnz"@0xffffff8008a7e65c ret < 0 -> "tp fw invaild"
 *   "97dad39b bl"@0xffffff8008a7e668   "tp fw vaild"
 * L'elenco delle chiamate lo conferma: `chiama: printk fts_wait_tp_to_valid
 * fts_i2c_read_reg fts_i2c_write msleep __stack_chk_fail` -- niente
 * fts_fwupg_check_fw_valid, niente fts_fwupg_reset_in_boot.
 *
 * IL RISULTATO E' UNA SELEZIONE SENZA SALTO, e questo dice che l'ultimo
 * confronto e' l'espressione di ritorno e non un `if` che salta:
 *   "394013e8 ldrb"@0xffffff8008a7e940   la versione del TP
 *   "6b15011f cmp"@0xffffff8008a7e944    contro quella dell'host
 *   "1a9f27f6 cset"@0xffffff8008a7e948   w22 = (tp < host), SENZA segno
 * `cset ..., cc` e' l'unsigned lower: le due versioni sono `u8`.
 *
 * I DUE ALTRI ESITI SONO COSTANTI E TAIL-MERGED:
 *   "2a1f03f6 mov"@0xffffff8008a7e6d4   w22 = 0, la coda dei due fallimenti
 *   "320003f6 orr"@0xffffff8008a7e6f4   w22 = 1, il ramo "fw invalid"
 *
 * LA VERSIONE DELL'HOST NON HA UNO SLOT DI STACK -- "38686935 ldrb"
 * @0xffffff8008a7e920 la mette in w21 e la' resta -- mentre quella del TP ce
 * l'ha, "390013ff strb"@0xffffff8008a7e654 (l'azzeramento) e
 * "394013e1 ldrb"@0xffffff8008a7e92c: l'indirizzo della seconda esce dalla
 * funzione (va a fts_i2c_read_reg), quello della prima no.
 *
 * I MESSAGGI DI QUESTA FUNZIONE.  I primi due sono della copia incorporata di
 * fts_fwupg_check_fw_valid, non suoi, e sono gli stessi due che quella
 * funzione stampa fuori linea:
 *   "\x016[FTS][Info]tp fw invaild\n"@0xffffff800924e4a0
 *   "\x016[FTS][Info]tp fw vaild\n"@0xffffff800924e4bc
 *   "\x013[FTS][Error]get fw ver in host fail\n"@0xffffff800924f268
 *   "\x013[FTS][Error]get fw ver in tp fail\n"@0xffffff800924f28f
 *   "\x016[FTS][Info]fw version in tp:%x, host:%x\n"@0xffffff800924f2b4
 *   "\x016[FTS][Info]fw invalid, need upgrade fw\n"@0xffffff800924f2df
 */
static bool fts_fwupg_need_upgrade(struct i2c_client *client)
{
	int ret = 0;
	bool fwvalid = false;
	u8 fw_ver_in_host = 0;
	u8 fw_ver_in_tp = 0;

	fwvalid = fts_fwupg_check_fw_valid(client);
	if (fwvalid) {
		ret = fts_fwupg_get_ver_in_host(&fw_ver_in_host);
		if (ret < 0) {
			FTS_ERROR("get fw ver in host fail");
			return false;
		}

		ret = fts_fwupg_get_ver_in_tp(client, &fw_ver_in_tp);
		if (ret < 0) {
			FTS_ERROR("get fw ver in tp fail");
			return false;
		}

		FTS_INFO("fw version in tp:%x, host:%x", fw_ver_in_tp,
			 fw_ver_in_host);
		if (fw_ver_in_tp < fw_ver_in_host)
			return true;
	} else {
		FTS_INFO("fw invalid, need upgrade fw");
		return true;
	}

	return false;
}

/*
 * ---------------------------------------------------------------------------
 * fts_param_get_ver_in_host -- incorporata dentro fts_fwupg_upgrade
 * ---------------------------------------------------------------------------
 * E' la gemella di fts_fwupg_get_ver_in_host col campo c56 al posto di c36 e
 * un controllo in piu' sul valore letto.  I tre `cbz` sono gli stessi tre, e
 * il MESSAGGIO E' LA STESSA STRINGA DEDUPLICATA a 0xffffff800924f309:
 *   "b40009fa cbz"@0xffffff8008a7e7b0   upg
 *   "b40009a8 cbz"@0xffffff8008a7e7b8   upg->c32
 *   "b4000969 cbz"@0xffffff8008a7e7c0   upg->c0
 *
 * IL VERSO DEL CONFRONTO E' L'OPPOSTO di quello della gemella, e non e' una
 * scelta:
 *   "6b02003f cmp"@0xffffff8008a7e7cc
 *   "54000ee3 b.cc"@0xffffff8008a7e7d0   MINORE -> salta al ramo d'errore
 * mentre la gemella ha "54001322 b.cs"@0xffffff8008a7e694 che salta al ramo
 * BUONO.  Il testo dei due messaggi lo conferma: qui il formato e' senza gli
 * `0x` letterali, "fw len(%x) < paramcfg ver offset(%x)".
 *
 * IL CONTROLLO SUL VALORE LETTO E' UN `||` DI DUE, e i due estremi sono 0xff
 * e 0:
 *   "7103ff5f cmp"@0xffffff8008a7e7f4   == 0xff
 *   "54000e20 b.eq"@0xffffff8008a7e7f8
 *   "34000e1a cbz"@0xffffff8008a7e7fc   == 0
 * tutti e due verso 0xffffff8008a7e9bc, che stampa la versione INFO del
 * messaggio.
 *
 * I MESSAGGI DI QUESTA FUNZIONE.  Il primo e' la stessa stringa deduplicata
 * della gemella:
 *   "\x013[FTS][Error]fts_data/upgrade/func/fw/ver is NULL\n"@0xffffff800924f309
 *   "\x013[FTS][Error]fw len(%x) < paramcfg ver offset(%x)\n"@0xffffff800924f503
 *   "\x016[FTS][Info]fw paramcfg version offset:%x\n"@0xffffff800924f537
 *   "\x016[FTS][Info]param version in host invalid\n"@0xffffff800924f563
 */
static int fts_param_get_ver_in_host(u8 *ver)
{
	struct fts_upgrade *upg = fts_g_a100ac8;

	if ((NULL == upg) || (NULL == upg->c32) || (NULL == upg->c0)) {
		FTS_ERROR("fts_data/upgrade/func/fw/ver is NULL");
		return -EINVAL;
	}

	if (upg->c8 < upg->c32->c56) {
		FTS_ERROR("fw len(%x) < paramcfg ver offset(%x)", upg->c8,
			  upg->c32->c56);
		return -EINVAL;
	}

	FTS_INFO("fw paramcfg version offset:%x", upg->c32->c56);
	*ver = upg->c0[upg->c32->c56];
	if ((0xFF == *ver) || (0x00 == *ver)) {
		FTS_INFO("param version in host invalid");
		return -EINVAL;
	}

	return 0;
}

/*
 * ---------------------------------------------------------------------------
 * fts_param_get_ver_in_tp -- incorporata dentro fts_fwupg_upgrade, DUE VOLTE
 * ---------------------------------------------------------------------------
 * IL REGISTRO E' 0xb5 ED E' NOMINATO DAL BINARIO: l'unita' A stampa il suo
 * valore con "Param Ver:0x%02x\n"@0xffffff800924e057.
 *   "528016a1 mov"@0xffffff8008a7e838   w1 = 0xb5   (prima copia, sp+12)
 *   "528016a1 mov"@0xffffff8008a7ea34   w1 = 0xb5   (seconda copia, sp+0)
 * Ha lo stesso controllo 0xff/0 di fts_param_get_ver_in_host, ed e' su un
 * valore RILETTO dalla memoria dopo la chiamata:
 *   "394033e1 ldrb"@0xffffff8008a7e848   prima copia
 *   "394003e8 ldrb"@0xffffff8008a7ea44   seconda copia
 *
 * I MESSAGGI DI QUESTA FUNZIONE:
 *   "\x013[FTS][Error]read fw param ver from tp fail\n"@0xffffff800924f58f
 *   "\x016[FTS][Info]param version in tp invalid\n"@0xffffff800924f5bd
 */
static int fts_param_get_ver_in_tp(struct i2c_client *client, u8 *ver)
{
	int ret = 0;

	ret = fts_i2c_read_reg(client, FTS_REG_PARAM_VER, ver);
	if (ret < 0) {
		FTS_ERROR("read fw param ver from tp fail");
		return ret;
	}

	if ((0xFF == *ver) || (0x00 == *ver)) {
		FTS_INFO("param version in tp invalid");
		return -EIO;
	}

	return 0;
}

/*
 * ---------------------------------------------------------------------------
 * fts_param_need_upgrade -- incorporata dentro fts_fwupg_upgrade
 * ---------------------------------------------------------------------------
 * LA TESTA E' DI NUOVO fts_fwupg_check_fw_valid INCORPORATA, seconda copia:
 *   "940000cb bl"@0xffffff8008a7e794    fts_wait_tp_to_valid
 *   "37f80fe0 tbnz"@0xffffff8008a7e798  ret < 0 -> "tp fw invaild"
 *
 * IL REGISTRO DI STATO E' 0xb6 ED E' NOMINATO DAL BINARIO: l'unita' A stampa
 * il suo valore con "Param status:0x%02x\n"@0xffffff800924e069.
 *   "528016c1 mov"@0xffffff8008a7e804   w1 = 0xb6
 *
 * I DUE CONTROLLI SULLO STATO SONO UNO A BIT SINGOLO E UNO A MASCHERA, e le
 * due istruzioni sono diverse apposta:
 *   "36381068 tbz"@0xffffff8008a7e818   il bit 7 -> "no IDE VER in tp"
 *   "7200191f tst"@0xffffff8008a7e81c   i sette bit bassi
 *   "540000a0 b.eq"@0xffffff8008a7e820  se sono zero prosegue
 * Un `tbz` su un bit solo e un `tst #0x7f` su una maschera: sono due
 * espressioni distinte del sorgente, non una sola.  Il ramo con i sette bit
 * NON zero salta a 0xffffff8008a7e874, cioe' DIRETTAMENTE all'aggiornamento:
 * e' un `return true`.
 *
 * L'ULTIMO CONFRONTO E' UN SALTO E NON UNA SELEZIONE, al contrario di quello
 * della gemella fts_fwupg_need_upgrade:
 *   "6b1a011f cmp"@0xffffff8008a7e86c
 *   "54000b22 b.cs"@0xffffff8008a7e870   >= -> "param don't need upgrade"
 * ed e' la differenza di struttura fra i due chiamanti, non dei due chiamati:
 * qui il `true` porta a un blocco di codice, la' porta a una variabile.
 *
 * I MESSAGGI DI QUESTA FUNZIONE.  I primi due sono della copia incorporata di
 * fts_fwupg_check_fw_valid:
 *   "\x016[FTS][Info]tp fw invaild\n"@0xffffff800924e4a0
 *   "\x016[FTS][Info]tp fw vaild\n"@0xffffff800924e4bc
 *   "\x016[FTS][Info]fw is invalid, no upgrade paramcfg\n"@0xffffff800924f3bd
 *   "\x013[FTS][Error]param version in host invalid\n"@0xffffff800924f3ee
 *   "\x013[FTS][Error]read IDE PARAM STATUS in tp fail\n"@0xffffff800924f41b
 *   "\x016[FTS][Info]no IDE VER in tp\n"@0xffffff800924f44b
 *   "\x016[FTS][Info]IDE VER, param invalid, need upgrade param\n"@0xffffff800924f46a
 *   "\x013[FTS][Error]get IDE param ver in tp fail\n"@0xffffff800924f4a3
 *   "\x016[FTS][Info]fw paramcfg version in tp:%x, host:%x\n"@0xffffff800924f4cf
 * IL TESTO "param version in host invalid" COMPARE DUE VOLTE CON DUE LIVELLI
 * DIVERSI, 0x3ee con KERN_ERR e 0x563 con KERN_INFO: quello di qui e' l'ERR,
 * quello di fts_param_get_ver_in_host e' l'INFO.  E' la prova che le due
 * funzioni sono due (vedi il cappello del gruppo).
 */
static bool fts_param_need_upgrade(struct i2c_client *client)
{
	int ret = 0;
	bool fwvalid = false;
	u8 ide_para_status = 0;
	u8 param_ver_in_tp = 0;
	u8 param_ver_in_host = 0;

	fwvalid = fts_fwupg_check_fw_valid(client);
	if (!fwvalid) {
		FTS_INFO("fw is invalid, no upgrade paramcfg");
		return false;
	}

	ret = fts_param_get_ver_in_host(&param_ver_in_host);
	if (ret < 0) {
		FTS_ERROR("param version in host invalid");
		return false;
	}

	ret = fts_i2c_read_reg(client, FTS_REG_PARAM_STATUS, &ide_para_status);
	if (ret < 0) {
		FTS_ERROR("read IDE PARAM STATUS in tp fail");
		return false;
	}

	if (0 == (ide_para_status & 0x80)) {
		FTS_INFO("no IDE VER in tp");
		return false;
	}

	if (0 != (ide_para_status & 0x7F)) {
		FTS_INFO("IDE VER, param invalid, need upgrade param");
		return true;
	}

	ret = fts_param_get_ver_in_tp(client, &param_ver_in_tp);
	if (ret < 0) {
		FTS_ERROR("get IDE param ver in tp fail");
		return false;
	}

	FTS_INFO("fw paramcfg version in tp:%x, host:%x", param_ver_in_tp,
		 param_ver_in_host);
	if (param_ver_in_tp < param_ver_in_host)
		return true;

	return false;
}

/*
 * ===========================================================================
 * fts_fwupg_upgrade @ 0xffffff8008a7e600, 1216 byte -- L'ULTIMA FUNZIONE
 * DELL'UNITA'
 * ===========================================================================
 * FIRMA, dai registri d'ingresso:
 *   x0 client -- "aa0003f3 mov"@0xffffff8008a7e628 -> x19, passato a
 *                fts_wait_tp_to_valid, fts_i2c_read_reg, fts_i2c_write e alle
 *                due chiamate indirette
 *   x1 upg    -- "aa0103f4 mov"@0xffffff8008a7e634 -> x20, confrontato con
 *                NULL da "b4000334 cbz"@0xffffff8008a7e644 e usato come base
 *                di "f9401288 ldr"@0xffffff8008a7e648 (+32),
 *                "f9400281 ldr"@0xffffff8008a7e748 (+0) e
 *                "b9400a82 ldr"@0xffffff8008a7e74c (+8): e' una
 *                `struct fts_upgrade *`, la stessa del globale.
 * IL PARAMETRO NON E' IL GLOBALE: i due convivono nella stessa funzione.
 * x20 arriva da fuori, mentre il globale e' ricaricato due volte da dentro le
 * funzioni incorporate, "f9456515 ldr"@0xffffff8008a7e670 e
 * "f945651a ldr"@0xffffff8008a7e7ac.
 *
 * IL LOCALE E' INIZIALIZZATO PRIMA DEL BANNER:
 *   "390003ff strb"@0xffffff8008a7e63c   ver = 0, UN byte a sp+0
 *   "97dad3a5 bl"@0xffffff8008a7e640     e solo dopo la printk
 * ED E' UNO SOLO PER TUTTI E DUE I RAMI: lo stesso sp+0 e' il bersaglio della
 * lettura del registro 0xa6 ("910003e2 mov"@0xffffff8008a7e964) e di quella
 * del registro 0xb5 ("910003e2 mov"@0xffffff8008a7ea30).  Due variabili
 * distinte in due rami avrebbero avuto due azzeramenti distinti dentro i rami,
 * non uno solo in cima.
 *
 * IL CONTROLLO E' `||` DI DUE e il messaggio ne nomina due:
 *   "b4000334 cbz"@0xffffff8008a7e644   upg
 *   "b40002e8 cbz"@0xffffff8008a7e64c   upg->c32
 *   "128002b5 mov"@0xffffff8008a7e6b4   w21 = -22 = -EINVAL
 *
 * IL CICLO E' UN `do/while` DI DUE GIRI COL CONTATORE INCREMENTATO IN TESTA,
 * e le tre istruzioni del fondo lo dicono NELL'ORDINE:
 *   "320003f7 orr"@0xffffff8008a7e718   w23 = 1, gia' nel preambolo
 *   "71000aff cmp"@0xffffff8008a7e8dc   w23 contro 2, PRIMA dell'incremento
 *   "110006f7 add"@0xffffff8008a7e8e0   w23 += 1, DOPO il confronto
 *   "54fff243 b.cc"@0xffffff8008a7e8e4  minore SENZA segno -> altro giro
 * QUESTO E' UN DIFETTO CHE LA DIMENSIONE HA DENUNCIATO PER QUATTRO BYTE E CHE
 * IL CONFRONTO PER CODIFICA HA CHIUSO.  La prima stesura di questo lotto
 * scriveva `for (i = 0; i < 2; i++)` stampando `i + 1`, e clang 9 NON ha
 * riscritto l'induzione: metteva un `add w23,w23,#1` in TESTA a tutti e due i
 * rami del ciclo, 305 istruzioni invece di 304 e 1220 byte invece di 1216.
 * La seconda stesura, `for (i = 1; i <= 2; i++)` stampando `i`, tornava a 304
 * istruzioni e 1216 byte ESATTI e sbagliava lo stesso: il confronto per
 * codifica dava RESIDUO 2, ed erano proprio queste due posizioni scambiate --
 * `add w23,w23,#1` + `cmp w23,#3` invece di `cmp w23,#2` + `add w23,w23,#1`.
 * Solo il `do/while` con l'incremento in testa mette il confronto sul valore
 * VECCHIO e l'incremento dopo, e porta il residuo a zero.  Il `orr w23,wzr,#1`
 * del preambolo e' la stessa cosa vista dall'altro capo: e' il primo `++`
 * eseguito una volta sola fuori dal ciclo, che un `for` non produrrebbe.
 *
 * QUATTRO INVARIANTI SONO ESTRATTI FUORI DAL CICLO dal compilatore, e sono la
 * conferma che il ciclo c'e':
 *   "91380718 add"@0xffffff8008a7e71c   x24 = "upgrade fw app(times:%d)"
 *   "9117ff39 add"@0xffffff8008a7e724   x25 = "reset in boot environment"
 *   "913a677b add"@0xffffff8008a7e728   x27 = "fw don't need upgrade"
 *   "32000bfc orr"@0xffffff8008a7e720   w28 = 7, il byte del comando di reset
 * Il w28 e' l'indizio piu' netto: il 7 di fts_fwupg_reset_in_boot e' tenuto in
 * un registro callee-saved per tutti e due i giri.
 *
 * IL BIVIO DEL CICLO E' SUL FLAG CALCOLATO UNA VOLTA SOLA:
 *   "34000256 cbz"@0xffffff8008a7e72c   se il flag e' 0 -> il ramo `param`
 * Il flag e' w22 e viene da fts_fwupg_need_upgrade, chiamata PRIMA del ciclo
 * ("97dad374 bl"@0xffffff8008a7e704 stampa "fw upgrade flag:%d" con
 * "2a1603e1 mov"@0xffffff8008a7e700, w1 = w22).  Nel secondo giro NON viene
 * ricalcolato.
 *
 * LE DUE CHIAMATE INDIRETTE HANNO GLI STESSI TRE ARGOMENTI, e sono i campi del
 * PARAMETRO e non del globale:
 *   "f9400281 ldr"@0xffffff8008a7e748   x1 = upg->c0        (ramo fw)
 *   "b9400a82 ldr"@0xffffff8008a7e74c   w2 = upg->c8
 *   "d63f0100 blr"@0xffffff8008a7e754
 *   "f9400281 ldr"@0xffffff8008a7e888   x1 = upg->c0        (ramo param)
 *   "b9400a82 ldr"@0xffffff8008a7e88c   w2 = upg->c8
 *   "d63f0100 blr"@0xffffff8008a7e898
 *
 * IL CODICE D'ERRORE DELLA FUNZIONE ASSENTE E' -61:
 *   "321a6ff5 orr"@0xffffff8008a7e95c   w21 = 0xffffffc3 = -61 = -ENODATA
 *
 * SUL SUCCESSO SI ESCE DAL CICLO, sul fallimento no.  Il ramo fw:
 *   "36f81040 tbz"@0xffffff8008a7e75c   ret >= 0 -> 0xffffff8008a7e964, che
 *                                       rilegge la versione e ritorna
 *   ret < 0 -> la copia incorporata di fts_fwupg_reset_in_boot e poi il fondo
 *              del ciclo
 * IL VALORE DI RITORNO DEL RESET E' IGNORATO: dopo la
 * "97db50d3 bl"@0xffffff8008a7e8c8 (msleep) si va dritti a
 * "71000aff cmp"@0xffffff8008a7e8dc, e w21 resta il negativo
 * dell'aggiornamento fallito.
 *
 * LE DUE COPIE INCORPORATE DI fts_fwupg_reset_in_boot HANNO DUE BUFFER
 * DIVERSI, ed e' la prova che sono due incorporazioni e non un blocco condiviso
 * del sorgente:
 *   "390023fc strb"@0xffffff8008a7e764   cmd = 7 a sp+8    (ramo fw)
 *   "910023e1 add"@0xffffff8008a7e76c    x1 = sp+8
 *   "390053fc strb"@0xffffff8008a7e8a8   cmd = 7 a sp+20   (ramo param)
 *   "910053e1 add"@0xffffff8008a7e8b0    x1 = sp+20
 * Le due code convergono poi sulla stessa fts_i2c_write,
 * "940006f6 bl"@0xffffff8008a7e8bc: e' la fusione delle code fatta dal
 * compilatore, non una funzione sola.
 *
 * IL RAMO `param` NON HA UN MESSAGGIO PER LA FUNZIONE ASSENTE:
 *   "f9403d08 ldr"@0xffffff8008a7e780   upg->c32->c120
 *   "b40012e8 cbz"@0xffffff8008a7e784   se manca -> DRITTO all'epilogo
 * senza stampare niente e senza toccare w21.  E' asimmetrico rispetto al ramo
 * fw, che invece stampa e mette -61: e' cio' che il binario fa.
 *
 * I MESSAGGI DI QUESTA FUNZIONE (senza quelli delle sei incorporate e delle
 * due copie di fts_fwupg_check_fw_valid e di fts_fwupg_reset_in_boot), in
 * ordine di indirizzo nel `.rodata`:
 *   "\x016[FTS][Info]fw auto upgrade function\n"@0xffffff800924ed94
 *   "\x013[FTS][Error]upg/upg->func is null\n"@0xffffff800924edbb
 *   "\x016[FTS][Info]fw upgrade flag:%d\n"@0xffffff800924ede0
 *   "\x016[FTS][Info]upgrade fw app(times:%d)\n"@0xffffff800924ee01
 *   "\x016[FTS][Info]success upgrade to fw version %02x\n"@0xffffff800924ee28
 *   "\x013[FTS][Error]upgrade func/upgrade is null, return immediately\n"@0xffffff800924ee59
 *   "\x016[FTS][Info]fw don't need upgrade\n"@0xffffff800924ee99
 *   "\x016[FTS][Info]upgrade param area(times:%d)\n"@0xffffff800924eebd
 *   "\x016[FTS][Info]success upgrade to fw param version %02x\n"@0xffffff800924eee8
 *   "\x016[FTS][Info]param don't need upgrade\n"@0xffffff800924ef1f
 * L'ordine dei dieci ha guidato la stesura: "success upgrade to fw version"
 * sta PRIMA di "upgrade func/upgrade is null", quindi la stampa della funzione
 * assente e' l'`else` della chiamata e non un controllo che la precede.  E'
 * un indizio DEBOLE (l'ordine del `.rodata` di questa unita' non e' l'ordine
 * delle funzioni) e la misura che conta e' il confronto per codifica.
 */
int fts_fwupg_upgrade(struct i2c_client *client, struct fts_upgrade *upg)
{
	int ret = 0;
	int upgrade_count = 0;
	bool upgrade_flag = false;
	u8 ver = 0;

	FTS_INFO("fw auto upgrade function");
	if ((NULL == upg) || (NULL == upg->c32)) {
		FTS_ERROR("upg/upg->func is null");
		return -EINVAL;
	}

	upgrade_flag = fts_fwupg_need_upgrade(client);
	FTS_INFO("fw upgrade flag:%d", upgrade_flag);
	do {
		upgrade_count++;
		if (upgrade_flag) {
			FTS_INFO("upgrade fw app(times:%d)", upgrade_count);
			if (upg->c32->c96) {
				ret = upg->c32->c96(client, upg->c0, upg->c8);
				if (ret < 0) {
					fts_fwupg_reset_in_boot(client);
				} else {
					fts_fwupg_get_ver_in_tp(client, &ver);
					FTS_INFO("success upgrade to fw version %02x",
						 ver);
					break;
				}
			} else {
				FTS_ERROR("upgrade func/upgrade is null, return immediately");
				return -ENODATA;
			}
		} else {
			FTS_INFO("fw don't need upgrade");
			if (NULL == upg->c32->c120)
				break;

			if (fts_param_need_upgrade(client)) {
				FTS_INFO("upgrade param area(times:%d)",
					 upgrade_count);
				ret = upg->c32->c120(client, upg->c0, upg->c8);
				if (ret < 0) {
					fts_fwupg_reset_in_boot(client);
				} else {
					fts_param_get_ver_in_tp(client, &ver);
					FTS_INFO("success upgrade to fw param version %02x",
						 ver);
					break;
				}
			} else {
				FTS_INFO("param don't need upgrade");
				break;
			}
		}
	} while (upgrade_count < 2);

	return ret;
}


/*
 * ===========================================================================
 * LA MISURA, RIFATTA PER ULTIMA COSA (lotto B3, 2026-08-22)
 * ===========================================================================
 * I due compilatori, e sono due giudici diversi:
 *   fabbrica  Android (5484270 based on r353983c) clang version 9.0.3
 *   nostro    Android (6443078 based on r383902) clang version 11.0.1
 *
 * COL COMPILATORE DI FABBRICA, l'oggetto vero (non misuraisolata.py), con
 * `-Werror` imposto:
 *
 *   $ ./compila-ft8719-b1.sh fab werror \
 *       drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_flash.o
 *     CC      drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_flash.o
 *   $ aarch64-linux-android-nm --print-size \
 *       out-ft8719-fab/.../focaltech_flash.o | grep -iE " [tTwW] " | sort -k1
 *   0000000000000000 0000000000000700 T fts_pram_write_init
 *   0000000000000700 00000000000001f8 T fts_fwupg_get_boot_state
 *   00000000000008f8 00000000000000e0 t fts_pram_init
 *   00000000000009d8 0000000000000140 T fts_fwupg_reset_to_romboot
 *   0000000000000b18 0000000000000094 T fts_fwupg_check_state
 *   0000000000000bac 0000000000000040 T fts_fwupg_check_fw_valid
 *   0000000000000bec 0000000000000094 T fts_fwupg_reset_in_boot
 *   0000000000000c80 000000000000008c T fts_fwupg_reset_to_boot
 *   0000000000000d0c 000000000000015c T fts_fwupg_enter_into_boot
 *   0000000000000e68 000000000000010c T fts_fwupg_erase
 *   0000000000000f74 000000000000021c T fts_fwupg_ecc_cal
 *   0000000000001190 0000000000000248 T fts_flash_write_buf
 *   00000000000013d8 0000000000000160 T fts_flash_read_buf
 *   0000000000001538 0000000000000118 T fts_flash_read
 *   0000000000001650 00000000000001c8 T fts_read_file
 *   0000000000001818 000000000000019c T fts_upgrade_bin
 *   00000000000019b4 00000000000004c0 T fts_fwupg_upgrade
 *
 * QUELLO E' L'ELENCO COMPLETO DEI SIMBOLI DI FUNZIONE dell'oggetto:
 * DICIASSETTE, non trentuno.  E' la misura NEGATIVA che prova che tutte e
 * quattordici le `static` scritte in questo file per riprodurre le funzioni
 * incorporate della fabbrica sono state DAVVERO incorporate: se anche una
 * sola fosse rimasta fuori linea, qui ci sarebbe un simbolo `t` in piu' e la
 * funzione che la contiene misurerebbe meno.  L'unico `t` e' fts_pram_init,
 * che `t` lo e' anche nella mappa di fabbrica.
 *
 * E L'UNITA' E' CHIUSA, non solo completa: l'ultimo simbolo comincia a
 * 0x19b4 e misura 0x4c0, quindi l'oggetto finisce a 0x1e74 = 7796, ed e'
 * esattamente 0xffffff8008a7eac0 - 0xffffff8008a7cc4c, l'intervallo
 * dell'unita' B nella mappa.  Ogni indirizzo di partenza combacia con quello
 * di fabbrica meno la base: 0x1650 + 0xa7cc4c = 0xa7e29c, 0x1818 + 0xa7cc4c =
 * 0xa7e464, 0x19b4 + 0xa7cc4c = 0xa7e600.
 *
 * COL NOSTRO COMPILATORE, DIRECTORY INTERA (tutti e sei gli obj-y) con
 * KCFLAGS=-Werror:
 *
 *   $ ./compila-ft8719-b1.sh nostro werror
 *     CC      drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_flash.o
 *     AR      drivers/input/touchscreen/mediatek/focaltech_ft8719/built-in.o
 *   $ aarch64-linux-android-nm --print-size \
 *       out-ft8719/.../focaltech_flash.o | grep -iE " [tTwW] " | sort -k1
 *   0000000000000000 00000000000006dc T fts_pram_write_init
 *   00000000000006dc 00000000000001f8 T fts_fwupg_get_boot_state
 *   00000000000008d4 00000000000000dc t fts_pram_init
 *   00000000000009b0 0000000000000144 T fts_fwupg_reset_to_romboot
 *   0000000000000af4 0000000000000090 T fts_fwupg_check_state
 *   0000000000000b84 0000000000000044 T fts_fwupg_check_fw_valid
 *   0000000000000bc8 0000000000000094 T fts_fwupg_reset_in_boot
 *   0000000000000c5c 000000000000008c T fts_fwupg_reset_to_boot
 *   0000000000000ce8 0000000000000158 T fts_fwupg_enter_into_boot
 *   0000000000000e40 0000000000000108 T fts_fwupg_erase
 *   0000000000000f48 0000000000000220 T fts_fwupg_ecc_cal
 *   0000000000001168 0000000000000248 T fts_flash_write_buf
 *   00000000000013b0 0000000000000154 T fts_flash_read_buf
 *   0000000000001504 0000000000000118 T fts_flash_read
 *   000000000000161c 00000000000001c8 T fts_read_file
 *   00000000000017e4 000000000000019c T fts_upgrade_bin
 *   0000000000001980 00000000000004f8 T fts_fwupg_upgrade
 *
 * LA TABELLA, tutte e tre le colonne dalla stessa misura finale:
 *
 *   funzione                   fabbrica  fab(9.0.3)  scarto  nostro(11.0.1) scarto
 *   -------------------------  --------  ----------  ------  -------------- ------
 *   fts_pram_write_init            1792        1792       0            1756    -36
 *   fts_fwupg_get_boot_state        504         504       0             504      0
 *   fts_pram_init                   224         224       0             220     -4
 *   fts_fwupg_reset_to_romboot      320         320       0             324     +4
 *   fts_fwupg_check_state           148         148       0             144     -4
 *   fts_fwupg_check_fw_valid         64          64       0              68     +4
 *   fts_fwupg_reset_in_boot         148         148       0             148      0
 *   fts_fwupg_reset_to_boot         140         140       0             140      0
 *   fts_fwupg_enter_into_boot       348         348       0             344     -4
 *   fts_fwupg_erase                 268         268       0             264     -4
 *   fts_fwupg_ecc_cal               540         540       0             544     +4
 *   fts_flash_write_buf             584         584       0             584      0
 *   fts_flash_read_buf              352         352       0             340    -12
 *   fts_flash_read                  280         280       0             280      0
 *   fts_read_file                   456         456       0             456      0
 *   fts_upgrade_bin                 412         412       0             412      0
 *   fts_fwupg_upgrade              1216        1216       0            1272    +56
 *   -------------------------  --------  ----------  ------  -------------- ------
 *   totale                         7796        7796       0            7800     +4
 *
 * DICIASSETTE SU DICIASSETTE ESATTE col compilatore di fabbrica; SETTE SU
 * DICIASSETTE col nostro.  Le TRE di questo lotto sono tre su tre col
 * compilatore di fabbrica e due su tre col nostro.
 *
 * ===========================================================================
 * IL CONFRONTO PER CODIFICA -- la misura che conta piu' della dimensione
 * ===========================================================================
 *   $ ./tutte-b3.sh fab normale        # f3cbis_codifica.py su tutte e 17
 *
 *   funzione                   istr F=N  mnemonici  codifiche  di cui  residuo
 *                                        uguali     diverse    reloc
 *   -------------------------  --------  ---------  ---------  ------  -------
 *   fts_pram_write_init             448    448/448        122     122        0
 *   fts_fwupg_get_boot_state        126    126/126         38      38        0
 *   fts_pram_init                    56      56/56         16      16        0
 *   fts_fwupg_reset_to_romboot       80      80/80         24      24        0
 *   fts_fwupg_check_state            37      37/37          7       7        0
 *   fts_fwupg_check_fw_valid         16      16/16          7       7        0
 *   fts_fwupg_reset_in_boot          37      37/37         13      13        0
 *   fts_fwupg_reset_to_boot          35      35/35         12      12        0
 *   fts_fwupg_enter_into_boot        87      87/87         39      39        0
 *   fts_fwupg_erase                  67      67/67         18      18        0
 *   fts_fwupg_ecc_cal               135    135/135         27      27        0
 *   fts_flash_write_buf             146    146/146         24      24        0
 *   fts_flash_read_buf               88      88/88         19      19        0
 *   fts_flash_read                   70      70/70         29      29        0
 *   fts_read_file                   114    114/114         32      32        0
 *   fts_upgrade_bin                 103    103/103         39      39        0
 *   fts_fwupg_upgrade               304    304/304        130     130        0
 *   -------------------------  --------  ---------  ---------  ------  -------
 *   totale                         1949  1949/1949        596     596        0
 *
 * MILLENOVECENTOQUARANTANOVE ISTRUZIONI SU MILLENOVECENTOQUARANTANOVE con lo
 * stesso mnemonico nella stessa posizione, e tutte e 596 le posizioni in cui
 * la codifica differisce sono siti di rilocazione: RESIDUO ZERO.
 *
 * E LO STESSO CONTO COL CONFRONTO STRETTO (`./tutte-b3.sh fab stretto`, cioe'
 * rev_codifica.py, che maschera il SOLO campo rilocato invece di tutta la
 * parola, cosi' una differenza di REGISTRO su un'istruzione rilocata non
 * resta nascosta) da' "RESIDUO STRETTO: 0" su tutte e diciassette:
 *   $ ./tutte-b3.sh fab stretto | grep -c "RESIDUO STRETTO: 0"
 *   17
 *   $ ./tutte-b3.sh fab stretto | grep "RESIDUO STRETTO" | grep -v ": 0$"
 *   (nessuna riga)
 *
 * ===========================================================================
 * IL CONFRONTO PER CODIFICA HA TROVATO IL DIFETTO DI QUESTO LOTTO, E LA
 * DIMENSIONE NE AVEVA VISTO SOLO META'
 * ===========================================================================
 * E' il ciclo di fts_fwupg_upgrade, ed e' raccontato per esteso nel cappello
 * di quella funzione.  In breve, tre stesure:
 *   `for (i = 0; i < 2; i++)` stampando `i + 1`
 *        -> 305 istruzioni invece di 304, 1220 byte invece di 1216.  LA
 *           DIMENSIONE LO DENUNCIA: un `add w23,w23,#1` in testa a ognuno dei
 *           due rami invece di uno solo in fondo.
 *   `for (i = 1; i <= 2; i++)` stampando `i`
 *        -> 304 istruzioni e 1216 byte ESATTI, e sbagliato lo stesso.  LA
 *           DIMENSIONE NON LO DENUNCIA PIU'; il confronto per codifica da'
 *             stesso mnemonico in posizione: 302 su 304
 *             RESIDUO: 2
 *             183  F 71000aff cmp w23, #0x2       | N 110006f7 add w23, w23, #0x1
 *             184  F 110006f7 add w23, w23, #0x1  | N 71000eff cmp w23, #0x3
 *           cioe' il confronto e l'incremento SCAMBIATI.  La codifica
 *           "71000eff cmp" e' quella della forma SCARTATA e nel binario di
 *           fabbrica NON c'e': va dichiarata `--controfattuale`, altrimenti
 *           verificaistruzioni.py la segna ASSENTE e ha ragione.  E' la
 *           SECONDA controfattuale di questo file, dopo quella di B1.
 *   `do { upgrade_count++; ... } while (upgrade_count < 2)`
 *        -> 304 su 304, RESIDUO 0.
 * Due byte di codifica in due parole su 1216: e' esattamente la classe di
 * difetto contro cui il progetto avverte, e la dimensione da sola l'aveva
 * gia' assolto.
 *
 * ===========================================================================
 * COME SI RIPORTA LA PERCENTUALE
 * ===========================================================================
 * L'AVVERTENZA VIENE PRIMA DEL NUMERO (classe C4).  Diciassette e tre sono
 * numerosita' piccole, e i due compilatori NON si comportano allo stesso modo
 * rispetto al 77,10% del ramo:
 *   $ ./venv/bin/python3 intervallo.py 17 17
 *   17 su 17 = 100.0%   IC95% Clopper-Pearson [80.5%; 100.0%]
 *   contiene 77.10%: NO -- la misura discrimina
 *   $ ./venv/bin/python3 intervallo.py 7 17
 *   7 su 17 = 41.2%   IC95% Clopper-Pearson [18.4%; 67.1%]
 *   contiene 77.10%: NO -- la misura discrimina
 *   $ ./venv/bin/python3 intervallo.py 3 3
 *   3 su 3 = 100.0%   IC95% Clopper-Pearson [29.2%; 100.0%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 *   $ ./venv/bin/python3 intervallo.py 2 3
 *   2 su 3 = 66.7%   IC95% Clopper-Pearson [9.4%; 99.2%]
 *   contiene 77.10%: SI -- la misura NON discrimina rispetto a quel valore
 * QUESTO E' IL PUNTO IN CUI L'UNITA' HA SUPERATO LA SOGLIA: dopo il lotto B2,
 * 14 su 14 dava [76.8%; 100.0%] e NON discriminava rispetto al 77,10%; con la
 * diciassettesima funzione l'estremo basso e' salito a 80.5% e la misura
 * discrimina SOPRA il ramo.  Col compilatore SBAGLIATO lo stesso identico
 * file discrimina SOTTO.  E sulle SOLE TRE funzioni di questo lotto la misura
 * non discrimina in nessuna direzione, con nessuno dei due compilatori: tre e'
 * troppo poco, e dirlo prima del numero e' obbligo.
 *
 * ===========================================================================
 * LE DIVERGENZE
 * ===========================================================================
 * COL COMPILATORE DI FABBRICA: NESSUNA.  Diciassette funzioni su diciassette
 * misurano la dimensione esatta e hanno residuo di codifica zero, sia col
 * confronto normale sia con quello stretto.  Le divergenze aperte dal lotto B1
 * sono tutte chiuse: la 1 (fts_flash_read -100) l'ha chiusa B2 definendo
 * fts_fwupg_reset_in_boot, e le altre tre erano gia' dichiarate come esistenti
 * SOLO col nostro compilatore.
 *
 * COL NOSTRO COMPILATORE restano DIECI scarti su diciassette.  Nove sono
 * quelli gia' dichiarati dai lotti B1 e B2, tutti di quattro o dodici byte
 * tranne fts_pram_write_init (-36); il decimo e' NUOVO ed e' di questo lotto:
 *   fts_fwupg_upgrade   1216 di fabbrica, 1272 col nostro, +56
 * Non c'e' niente da correggere: correggere il sorgente per far tornare un
 * numero col compilatore sbagliato sarebbe la regola 3 violata.  La causa e'
 * la stessa gia' misurata dal lotto B1 e non dipende dal sorgente --
 * `orr wD,wzr,#imm` (clang 9) contro `mov wD,#imm` (clang 11) per le costanti
 * piccole, e una disposizione dei blocchi diversa che su una funzione con
 * DICIOTTO code d'errore fuse costa piu' che sulle altre.
 *
 * ===========================================================================
 * LE CITAZIONI
 * ===========================================================================
 *   $ ./venv/bin/python3 verificaistruzioni.py \
 *       albero-ft8719/.../focaltech_flash.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a7cc4c:0xffffff8008a7eac0 \
 *       --controfattuale "1a9f1502 csinc" --controfattuale "71000eff cmp"
 *   citazioni di istruzione trovate nel sorgente: 652 (652 a codifica, 0 ad indirizzo)
 *   confermate: 641   assenti: 0   mnemonico diverso: 0   controfattuali: 11
 *   operandi -- registro diverso: 0   immediato diverso: 0   entrambi/non confrontabili: 0
 *
 * LE CONTROFATTUALI SONO DUE, E LA SECONDA E' DI QUESTO LOTTO.
 *   "1a9f1502 csinc"  la forma SBAGLIATA del ternario di
 *                     fts_fwupg_get_boot_state, dichiarata dal lotto B1;
 *                     compare CINQUE volte.
 *   "71000eff cmp"    il `cmp w23, #0x3` della seconda stesura del ciclo di
 *                     fts_fwupg_upgrade, quella con `for (i = 1; i <= 2; i++)`;
 *                     compare SEI volte, fra il diff che racconta come e'
 *                     stato trovato il difetto e le righe di comando.
 * Nessuna delle due sta nel binario di fabbrica, ed e' per questo che vanno
 * dichiarate: senza la dichiarazione lo strumento le segna ASSENTE e ha
 * ragione.  L'altra codifica di quel diff, "110006f7 add", nel binario C'E'
 * (e' l'incremento vero, a 0xffffff8008a7e8e0) e infatti e' CONFERMATA.
 *
 *   $ ./venv/bin/python3 verificacitazioni.py \
 *       albero-ft8719/.../focaltech_flash.c oracolo/stock.elf \
 *       --eccezione "[FTS][Info]" --eccezione "[FTS][Error]" \
 *       --eccezione "\n" --eccezione "[FTS]"
 *   letterali: 122   citati: 123   verificati: 116   probanti: 115   deboli: 1
 *   di cui verificate come messaggio assemblato dalla macro di log (coda + binario, non un singolo letterale C): 114
 *   soglia imposta: 118 citati richiesti (122 letterali - 4 eccezioni)
 *
 * OGNI LETTERALE DEL CODICE E' CITATO: i citati (123) sono piu' dei letterali
 * (122) perche' tre citazioni riguardano letterali di UN'ALTRA unita'.
 *
 * UNA CITAZIONE E' `DEBOLE`, ED E' UN LIMITE VERO CHE VA DETTO:
 *   DEBOLE 0xffffff80091760a5: %s%s -- occorre 10 volte nel binario
 * "%s%s" e' il formato della snprintf di fts_read_file, e sta in una pagina di
 * `.rodata` condivisa con tutto il kernel: l'indirizzo citato non e'
 * distinguibile dalle altre nove occorrenze sul solo testo.  L'ANCORAGGIO LO
 * DA' L'ISTRUZIONE, non il letterale: "900037c2 adrp"@0xffffff8008a7e2ec e
 * "91029442 add"@0xffffff8008a7e2f8 formano esattamente
 * 0xffffff8009176000 + 0xa5, e verificaistruzioni.py le conferma tutte e due.
 *
 * SETTE CITAZIONI RESTANO `NON_ANCORATA`, E LE CAUSE SONO DUE, TUTTE E DUE
 * NOTE E VERIFICATE A PARTE.
 *
 * (a) QUATTRO SONO QUELLE GIA' DICHIARATE DAL LOTTO B2: i messaggi della terza
 *     macro, quella SENZA livello KERN.  verificacitazioni.py riconosce un
 *     "messaggio assemblato" solo se comincia con SOH piu' una cifra di
 *     livello, e questi non ce l'hanno.  E' un buco dello STRUMENTO, non del
 *     file.
 *
 * (b) TRE SONO DI QUESTO LOTTO E SONO DI UN GENERE NUOVO: sono letterali di
 *     UN'ALTRA UNITA' DI TRADUZIONE, citati qui come prova che i nomi
 *     FTS_REG_FW_VER, FTS_REG_PARAM_VER e FTS_REG_PARAM_STATUS non sono
 *     inventati (eccezione della regola 5).  Lo strumento cerca l'ancora fra i
 *     letterali DI QUESTO file e non la trova, ed ha ragione: quelle printk
 *     stanno in focaltech_sysfs.c.  Non e' un difetto ne' dello strumento ne'
 *     del file: e' il costo di provare un nome con una stringa che vive
 *     altrove.
 *
 * LA VERIFICA INDIPENDENTE DI TUTTE E SETTE, che lo strumento non sa fare:
 *   $ ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf \
 *       0xffffff800924e02f 0xffffff800924e057 0xffffff800924e069
 *   0xffffff800924e02f: b'FW Ver:0x%02x\n'  (len=14, [...])
 *   0xffffff800924e057: b'Param Ver:0x%02x\n'  (len=17, [...])
 *   0xffffff800924e069: b'Param status:0x%02x\n'  (len=20, [...])
 *   $ ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf \
 *       0xffffff800924e306 0xffffff800924e42b 0xffffff800924e7b4 0xffffff800924e90a
 *   0xffffff800924e306: b'[FTS]check whether tp is in romboot or not \n'  (len=44, [...])
 *   0xffffff800924e42b: b'[FTS]after write pramboot, confirm run in pramboot\n'  (len=51, [...])
 *   0xffffff800924e7b4: b'[FTS]pram not supported, confirm in bootloader\n'  (len=47, [...])
 *   0xffffff800924e90a: b'[FTS]ecc calc startaddr:0x%04x, len:%d\n'  (len=39, [...])
 *
 * ===========================================================================
 * IL CONTO DEGLI IRRISOLTI, PRIMA E DOPO QUESTO LOTTO
 * ===========================================================================
 * Differenza fra insiemi su `built-in.o`, che e' un ARCHIVIO: la classe dei
 * definiti include anche i `W` (memset e memcpy sono alias deboli).
 *
 *   $ ./irr2-ft8719.sh                       # i SEI oggetti, dopo B1+B2+B3
 *   escluso: 'nulla'  definiti: 97   usati(U): 94   IRRISOLTI: 69
 *   FT8719 irrisolti (4):
 *     fts_debug_read
 *     fts_debug_write
 *     fts_tprwreg_show
 *     fts_tprwreg_store
 *
 *   $ ./irr2-ft8719.sh focaltech_flash.o     # i CINQUE oggetti, senza unita' B
 *   escluso: 'focaltech_flash.o'  definiti: 79   usati(U): 86   IRRISOLTI: 68
 *   FT8719 irrisolti (10):
 *     fts_debug_read
 *     fts_debug_write
 *     fts_flash_write_buf
 *     fts_fwupg_ecc_cal
 *     fts_fwupg_enter_into_boot
 *     fts_fwupg_erase
 *     fts_fwupg_reset_in_boot
 *     fts_tprwreg_show
 *     fts_tprwreg_store
 *     fts_upgrade_bin
 *
 * L'UNITA' B HA CHIUSO SEI DELLE DIECI FUNZIONI FT8719 IRRISOLTE, e la sesta
 * -- fts_upgrade_bin -- l'ha chiusa QUESTO lotto.  LE QUATTRO CHE RESTANO NON
 * SONO PIU' DI QUESTA UNITA': sono le quattro dell'unita' A.
 *
 * E HO CONTATO ANCHE CIO' CHE IL MIO CODICE NUOVO CITA (la lezione del lotto
 * F5), per DIFFERENZA FRA INSIEMI e non per totale:
 *
 *   $ comm -13 /tmp/und-cinque.txt /tmp/und-dopo-b3.txt   # U nuovi dell'unita' B
 *   __const_udelay
 *   filp_close
 *   filp_open
 *   fts_wait_tp_to_valid
 *   set_bit
 *   vfree
 *   vfs_read
 *   vmalloc
 *   $ comm -13 /tmp/irr-cinque.txt /tmp/irr-dopo-b3.txt   # IRRISOLTI nuovi
 *   __const_udelay
 *   filp_close
 *   filp_open
 *   set_bit
 *   vfree
 *   vfs_read
 *   vmalloc
 *
 * SEI DEI SETTE IRRISOLTI NUOVI LI PORTA QUESTO LOTTO -- filp_open,
 * filp_close, vfs_read, vmalloc, vfree e set_bit, tutti da fts_read_file
 * tranne vfree che viene da fts_upgrade_bin -- e SONO TUTTI SIMBOLI DEL
 * KERNEL, non del driver: si risolvono al link finale come printk e msleep.
 * Il settimo, `__const_udelay`, e' quello che avevano gia' portato B1 e B2.
 * `fts_wait_tp_to_valid` e' un U nuovo ma NON un irrisolto: lo definisce
 * focaltech_core.o.
 *
 * Il totale passa da 64 (dichiarato dal lotto B2: definiti 94, usati 88) a 69:
 * sei irrisolti nuovi meno uno chiuso, e i definiti passano da 94 a 97, cioe'
 * esattamente le TRE funzioni di questo lotto -- nessuna delle sei `static` di
 * appoggio lascia simbolo, come deve.
 *
 * `snprintf` NON e' un irrisolto nuovo anche se questo lotto lo chiama per la
 * prima volta in questa unita': era gia' nell'insieme degli usati dei cinque
 * oggetti, perche' lo chiama l'unita' A.
 *   $ grep -c "^snprintf$" /tmp/und-cinque.txt
 *   1
 *
 * NESSUNO STUB.  Le sei funzioni di confine di questo file (fts_i2c_read,
 * fts_i2c_write, fts_i2c_hid2std, fts_i2c_write_reg, fts_i2c_read_reg,
 * fts_wait_tp_to_valid) sono dichiarate e lasciate indefinite: le definiscono
 * altri oggetti della stessa directory, e il conto qui sopra lo prova.
 *
 * ===========================================================================
 * COSA E' STATO CONTROLLATO E TROVATO A POSTO
 * ===========================================================================
 * Una revisione che non dice cosa NON ha guardato non serve.
 *   - dimensione: 17 su 17 esatte col compilatore di fabbrica (3 su 3 fra
 *     quelle di questo lotto), 7 su 17 col nostro (2 su 3);
 *   - chiusura dell'unita': l'oggetto misura 7796 byte, esattamente
 *     l'intervallo della mappa, e i tre indirizzi di partenza nuovi combaciano
 *     con quelli di fabbrica meno la base;
 *   - codifica: 1949 istruzioni su 1949 con lo stesso mnemonico nella stessa
 *     posizione, residuo 0 sia col confronto normale sia con quello stretto;
 *   - simboli: diciassette e non trentuno, cioe' tutte e quattordici le
 *     `static` di appoggio sono incorporate come di fabbrica; l'unica `t` e'
 *     fts_pram_init, ed e' `t` anche nella mappa;
 *   - istruzioni citate: 641 confermate, 0 assenti, 0 mnemonici diversi, 0
 *     operandi diversi, 11 controfattuali dichiarate -- cinque occorrenze
 *     della forma scartata di B1 e sei della forma scartata di B3;
 *   - letterali: ogni letterale del codice e' citato, 1 debole di causa nota
 *     e 7 NON_ANCORATA di due cause note, tutte verificate a parte con
 *     leggi_stringa.py;
 *   - `-Werror`: il mio oggetto passa con tutti e due i compilatori, zero
 *     avvisi;
 *   - struttura: i tre nuovi offset di `struct fts_upgrade_func` (96, 120,
 *     128) e i due di `struct fts_upgrade` (0, 8) sono confermati DUE VOLTE e
 *     in due modi indipendenti -- dalle istruzioni di questa unita' e dalle
 *     rilocazioni dell'istanza che sta nell'unita' F;
 *   - nomi: nessun campo inventato fuori dalla convenzione c<offset>, nessun
 *     globale inventato; i tre nomi di registro vengono dalle stringhe del
 *     binario; i nomi delle sei `static` nuove, dei loro parametri e dei loro
 *     codici d'errore sono DICHIARATI come scelta;
 *   - difetti di fabbrica riprodotti e non raddrizzati: il bivio asimmetrico
 *     di fts_upgrade_bin (con `force` non stampa "success", senza `force` lo
 *     stampa anche quando non ha aggiornato niente), il `%0x` senza larghezza,
 *     il messaggio che nomina cinque cose e ne controlla tre, il valore di
 *     ritorno di fts_read_file che resta negativo col buffer allocato, il
 *     ramo `param` senza messaggio per la funzione assente, i valori di
 *     ritorno delle due copie incorporate di fts_fwupg_reset_in_boot ignorati.
 *
 * COSA NON E' STATO GUARDATO: la DIRECTORY INTERA non compila col compilatore
 * di FABBRICA, e non per colpa di questo file (vedi il delta n.1 qui sotto),
 * quindi il giudizio col compilatore di fabbrica e' dato oggetto per oggetto e
 * non c'e' un `built-in.o` di fabbrica.  Il comportamento a tempo di
 * esecuzione non e' stato provato su nessun dispositivo.  Le quattro funzioni
 * dell'unita' A non sono state guardate.  E la partizione in unita' resta
 * quella misurata dal lotto di ricognizione, non riverificata qui -- ma la
 * chiusura esatta dei 7796 byte e' una conferma indipendente del confine
 * dell'unita' B.
 *
 * ===========================================================================
 * DELTA DI HEADER / DI FILE CONDIVISO -- DESCRITTI, NON FATTI
 * ===========================================================================
 * 1. LA DIRECTORY NON COMPILA COL COMPILATORE DI FABBRICA, e non per colpa di
 *    questo file: focaltech_sysfs.c (unita' A, di un altro lotto) contiene due
 *      #pragma clang diagnostic ignored "-Wfortify-source"
 *    e quel gruppo di avvisi NON esiste in clang 9.0.3.  Riverificato oggi:
 *      $ ./compila-ft8719-b1.sh fab -
 *        CC      .../focaltech_sysfs.o
 *        CC      .../focaltech_flash.o
 *      .../focaltech_sysfs.c:755:34: error: unknown warning group
 *          '-Wfortify-source', ignored [-Werror,-Wunknown-warning-option]
 *      .../focaltech_sysfs.c:809:34: error: unknown warning group
 *          '-Wfortify-source', ignored [-Werror,-Wunknown-warning-option]
 *    Sono DUE righe di errore e sono le sole; il mio oggetto compila accanto
 *    senza dire niente.  Non l'ho corretto: quel file e' l'artefatto misurato
 *    di un altro lotto.
 * 2. Il Makefile del padre (drivers/input/touchscreen/mediatek/Makefile) non
 *    ha ancora la riga
 *      obj-$(CONFIG_WTK_FT8719_E977) += focaltech_ft8719/
 *    e il suo Kconfig non ha il `source` della directory.
 * 3. Il Kconfig di QUESTA directory descrive ancora cinque unita' e dice che
 *    l'unita' B «MANCA per intero».  Adesso e' COMPLETA, diciassette funzioni
 *    su diciassette.  NON l'ho riscritto: e' un file condiviso e lo
 *    aggiornera' il lotto di merge.  Il Makefile della directory, che i lotti
 *    B1 e B2 avevano gia' toccato, l'ho aggiornato nel solo conto delle
 *    funzioni (14 -> 17), perche' quel conto e' la sola cosa che questo lotto
 *    cambia in quel file.
 */
