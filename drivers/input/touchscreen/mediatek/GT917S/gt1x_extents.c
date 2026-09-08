/* drivers/input/touchscreen/gt1x_extents.c
 *
 * 2010 - 2014 Goodix Technology.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be a reference
 * to you, when you are integrating the GOODiX's CTP IC into your system,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * Version: 1.0
 * Revision Record:
 *      V1.0:  first release. 2014/09/28.
 *
 */

/* ======================================================================
 * LOTTO "extents" -- 2026-08-22.  Correzione di gt1x_extents.c contro
 * l'oracolo, NON riscrittura: il file di partenza era la copia dell'albero
 * ALPS (drivers/input/touchscreen/mediatek/GT5688/), portata dentro da un
 * lotto di ricognizione.
 *
 * Blocco di fabbrica: [0xffffff8008a7221c, 0xffffff8008a73854) meno
 * gt1x_gesture_debug (che sta in gt1x_wtk.c) = 5556 byte, 11 funzioni.
 * Misurato col compilatore DI FABBRICA (clang r353983c, LLVM 9.0.3):
 * 11 funzioni su 11 identiche al byte, 5556 su 5556.
 *
 * QUEL CHE LA MISURA DI DIMENSIONE NON DENUNCIAVA, e che il confronto per
 * codifica ha trovato: gesture_enter_doze misurava 236 su 236 fin dal
 * principio, ma faceva `msleep(20)` dove la fabbrica fa `msleep(10)`
 * ("52800140 mov"@0xffffff8008a7224c, w0=10; il ciclo e' srotolato cinque
 * volte, cinque occorrenze). Stessa dimensione, costante sbagliata.
 * Nella stessa funzione tre letterali erano diversi ("entering doze
 * mode...", "GTP has been working in doze mode!", "GTP send doze cmd
 * failed.") e in hotknot_open/hotknot_release altri due; anche loro non
 * cambiano un byte di codice.
 *
 * ---------------------------------------------------------------------
 * 1. LE DIVERGENZE CORRETTE, E LA PROVA DI CIASCUNA
 * ---------------------------------------------------------------------
 *
 * gt1x_compat_ioctl  408 -> 56.  Vedi il cappello sulla funzione: di
 *   fabbrica e' un rinvio secco, senza switch e senza log.
 *
 * gt1x_gesture_data_read  248 -> 244.  Il formato di fabbrica NOMINA la
 *   funzione dentro il letterale invece di passarla come %s:
 *   "<<GTP-DBG>>[%s:%d]visit gt1x_gesture_data_read. ppos:%d\n"@0xffffff800924a5b9 -- un
 *   argomento in meno, una `mov` in meno.
 *   La dimensione della struttura la da' "52802ba4 mov"@0xffffff8008a72c9c
 *   (w4 = 349 = 6 + 4 + (3 + 64*4 + 80)): il `#pragma pack(1)` e il
 *   GESTURE_MAX_POINT_COUNT = 64 della copia sono giusti.
 *
 * gt1x_init_node  240 -> 272.  Tre correzioni:
 *   - gestures_flag e' azzerato a 0xFF, non a 0:
 *     "92800009 mov"@0xffffff8008a72aec  (x9 = -1) seguito da
 *     "a9012509 stp"@0xffffff8008a72afc e "a9002509 stp"@0xffffff8008a72b00
 *     (32 byte di 0xFF). Con memset(...,0,...) le due `stp` sarebbero di
 *     `xzr`, come in gt1x_gesture_debug.
 *   - i permessi del nodo proc sono 0666, non 0644:
 *     "528036c1 mov"@0xffffff8008a72b18  (w1 = 438 = 0666).
 *   - i quattro messaggi sono quelli piu' vecchi di Goodix e portano
 *     GESTURE_NODE come argomento %s:
 *     "<<GTP-ERR>>[%s:%d] CAN't create proc entry /proc/%s.\n"@0xffffff800924a4cc
 *     "<<GTP-INF>>[%s:%d] Created proc entry /proc/%s.\n"@0xffffff800924a511
 *     "<<GTP-ERR>>[%s:%d] CAN't create misc device in /dev/hotknot.\n"@0xffffff800924a542
 *     "<<GTP-INF>>[%s:%d] Created misc device in /dev/hotknot.\n"@0xffffff800924a580
 *   Il nome del mutex lo stringa la macro `mutex_init` (eccezione alla
 *   regola dei nomi inventati): "&gesture_data_mutex"@0xffffff800924a4a9.
 *
 * gt1x_ioctl  3460 -> 2524.  La copia ALPS aveva, e la fabbrica non ha:
 *   - GTP_DEBUG("IOCTL CMD:%x", cmd) in testa: nel blocco non esiste ne'
 *     il letterale ne' la printk fra il ciclo di attesa e la `and` della
 *     maschera ("120246a8 and"@0xffffff8008a72ec8, w8 = cmd & 0xc000ffff);
 *   - la `static struct ratelimit_state ratelimit` e le due
 *     `gt1x_is_tpd_halt()` che la usavano: nessuna `bl` a __ratelimit nel
 *     blocco, e il .data dell'oggetto di fabbrica non ha spazio per essa
 *     (vedi punto 4);
 *   - i controlli `if (data != NULL)` su IO_IIC_READ/IO_IIC_WRITE: i due
 *     rami entrano dritti nel corpo incorporato;
 *   - il confronto `data_length + CMD_HEAD_LENGTH > buf_size` dentro
 *     io_iic_read e le due GTP_DEBUG finali di io_iic_read/io_iic_write;
 *   - i due `clamp` su gesture_data.data[1] e [3] in GESTURE_DATA_OBTAIN:
 *     al loro posto c'e' solo il controllo che il compilatore genera da
 *     solo per copy_to_user ("f10552bf cmp"@0xffffff8008a73348, x21 contro
 *     340, cioe' > 339 = sizeof(gesture_data.data), e la chiamata a
 *     warn_slowpath_fmt con "Buffer overflow detected (%d < %lu)!\n").
 *   La copia ALPS non aveva, e la fabbrica ha:
 *   - un comando che ALPS non conosce, _IOR('G',28) & NEGLECT_SIZE_MASK =
 *     0x8000471c ("5288e389 mov"@0xffffff8008a72f50 +
 *     "72b00009 movk"@0xffffff8008a72f54), che risponde con
 *     "GOODIX,GT1X"@0xffffff800924a838, 12 byte NUL compreso
 *     ("321e07f5 orr"@0xffffff8008a72f6c). Il NUMERO e' misurato, il NOME
 *     di fabbrica no: qui si chiama IO_NR28 e il commento dice perche'.
 *   - gt1x_irq_disable()/gt1x_irq_enable() intorno a gt1x_reset_guitar()
 *     su IO_RESET_GUITAR;
 *   - mutex_unlock DOPO l'if/else di GESTURE_DATA_OBTAIN, non prima.
 *   Cambia inoltre IO_VERSION: "V1.3-20150420"@0xffffff800924a6db, 14 byte
 *   ("321f0bf5 orr"@0xffffff8008a73030), non "V1.0-20140709".
 *   E i quattro messaggi dei gesti: "<<GTP-DBG>>[%s:%d]Gesture switch ON.\n"@0xffffff800924a700,
 *   "<<GTP-DBG>>[%s:%d]Gesture switch OFF.\n"@0xffffff800924a726,
 *   "<<GTP-DBG>>[%s:%d]Gesture flag: 0x%02X enabled.\n"@0xffffff800924a74d,
 *   "<<GTP-DBG>>[%s:%d]Gesture flag: 0x%02X disabled.\n"@0xffffff800924a77e.
 *   GESTURE_ENABLE_PARTLY di fabbrica NON scrive gesture_enabled, e
 *   GESTURE_DISABLE_PARTLY non fa ne' QUERYBIT ne' is_all_dead: c'e' solo
 *   SETBIT ("d3431c68 ubfx"@0xffffff8008a730f0, indice = (u8)value >> 3) e
 *   CLEARBIT ("0a2b014a bic"@0xffffff8008a73144).
 *   La tabella di salto e' a BYTE ("3869690b ldrb"@0xffffff8008a72ef4) e
 *   copre nr 1..104; nr 23..27 e 29 vi puntano al ramo `default`, che e'
 *   la prova che CONFIG_HOTKNOT_BLOCK_RW e' SPENTO di fabbrica.
 *   `cnt = 30` e `ssleep(1)` restano: "321b73f6 orr"@0xffffff8008a72e10
 *   (w22 parte da -29 e sale a zero: trenta giri) e
 *   "52807d00 mov"@0xffffff8008a72e14 (msleep di 1000 ms).
 *
 * gesture_event_handler  712 -> 1780.  E' una revisione piu' recente del
 *   codice Goodix, non quella di ALPS, e la fabbrica ci ha aggiunto la
 *   traduzione gesto -> tasto. Quel che il binario prova:
 *   - il registro dell'intestazione e' 0x814C
 *     ("52902980 mov"@0xffffff8008a72350), i dati extra stanno a 0x8150
 *     ("52902a00 mov"@0xffffff8008a72408), le coordinate a 0xA2A0
 *     ("52945400 mov"@0xffffff8008a724fc) e due byte in coda a 0x819F
 *     ("529033e0 mov"@0xffffff8008a72514);
 *   - i due limiti sono 64 e 32, non 64 e 80:
 *     "710106df cmp"@0xffffff8008a7239c (w22 contro 0x41) e
 *     "710086ff cmp"@0xffffff8008a723c4 (w23 contro 0x21);
 *   - il buffer dei dati extra e' un array a lunghezza variabile di
 *     extra_len+1 byte: "110042e8 add"@0xffffff8008a723ec e
 *     "927c1108 and"@0xffffff8008a723f0 arrotondano a multipli di 16, e
 *     "9100033f mov"@0xffffff8008a72404 sposta sp;
 *   - il bit 7 di doze_buf[2] accende il controllo di checksum
 *     ("363802b8 tbz"@0xffffff8008a72420), la somma e' a otto bit
 *     ("12001d08 and"@0xffffff8008a72460);
 *   - gesture_data.data[2] prende doze_buf[2] mascherato con 0x7f
 *     ("12001908 and"@0xffffff8008a72578);
 *   - lo `switch` finale sottrae 0x5e e ha 111 voci
 *     ("51017908 sub"@0xffffff8008a725b4 e
 *     "7101b91f cmp"@0xffffff8008a725b8), con quattordici casi vivi. Il
 *     primo e' 0x5e -> KEY_POWER ("52800e82 mov"@0xffffff8008a725dc,
 *     w2 = 116);
 *   - a fine funzione la fabbrica NON manda KEY_GESTURE e NON ritorna 1:
 *     ritorna `ret`, che li' vale zero.
 *
 * ---------------------------------------------------------------------
 * 2. LA VARIABILE CHE IL BINARIO NON NOMINA
 * ---------------------------------------------------------------------
 * `c6fc` -- il nome viene dall'indirizzo, 0xffffff800a1006fc, secondo la
 * convenzione del progetto per cio' che il binario non nomina. Di essa e'
 * MISURATO: che e' larga un byte, che viene letta con
 * "395bf109 ldrb"@0xffffff8008a729d4 e provata con
 * "360000a9 tbz"@0xffffff8008a729d8 (solo il bit 0), che viene scritta con
 * 0 ("391bf11f strb"@0xffffff8008a729dc, e ancora sul cammino in cui il
 * checksum torna) e con 1 ("391bf109 strb"@0xffffff8008a729f0); e che il
 * suo RUOLO, che il codice prova, e' "il checksum del gesto e' gia'
 * fallito una volta": al primo fallimento si mette a 1 e si esce senza
 * toccare il registro, al secondo si rimette a 0 e il registro viene
 * pulito.  Non e' misurato il suo nome, e non e' misurato se sia `static`:
 * `ldrb` + `tbz #0` esce da una `static u8` (clang conosce tutti gli
 * scritti); una `u8` non statica darebbe `cbz`, una `bool` non statica pure,
 * una `static bool` darebbe `cmp #1`. Vedi il residuo al punto 5.
 *
 * ---------------------------------------------------------------------
 * 3. LA MAPPA DELLE RIGHE, CHE E' UNA MISURA
 * ---------------------------------------------------------------------
 * Le macro di log incollano __LINE__ nella printk, quindi il binario
 * misura la POSIZIONE di ogni chiamata nel file di fabbrica. Le 46
 * costanti emesse danno 45 distanze fra ancore consecutive; 27 di esse
 * combaciano gia'. Le distanze sono servite da prova, non da decorazione:
 *   - fra "hotknot load jump code." (riga 504) e "Load jump code fail!"
 *     (507) di fabbrica ci sono TRE righe. Nella copia ALPS ce n'erano 19,
 *     per via del blocco #ifdef CONFIG_GTP_REQUEST_FW_UPDATE. Tolto quel
 *     blocco (e i suoi due gemelli), le otto distanze da 499 a 536 tornano
 *     tutte esatte.
 *   - fra "ERASE_GESTURE_DATA" (883) e "Unknown cmd." (944) di fabbrica ci
 *     sono 61 righe. Avendo io tolto i casi HOTKNOT_DEVICES_PAIRED..
 *     HOTKNOT_WAKEUP_BLOCK (che stanno sotto #ifdef CONFIG_HOTKNOT_BLOCK_RW
 *     e quindi non entrano nel binario), ne restavano 20: rimessi, sono 52.
 *     Il rimetterli e' quindi sostenuto dalla misura, non dal gusto.
 *   - fra "Can't access the memory." di gt1x_ioctl (795) e
 *     "Obtain gesture data." (870) le sette distanze sono tutte esatte: la
 *     fabbrica NON ha, li' dentro, i blocchi #ifdef CONFIG_GTP_ESD_PROTECT
 *     su IO_DISABLE_IRQ e IO_ENABLE_IRQ.
 * Le distanze che NON tornano dicono dove manca del sorgente di fabbrica
 * che questo lotto non ha recuperato; l'elenco sta nel rapporto.
 *
 * ---------------------------------------------------------------------
 * 4. IL .data E IL .bss, CHE SONO UNA PROVA A PARTE
 * ---------------------------------------------------------------------
 * `gesture_enabled` di fabbrica sta in .kernel2 (PROGBITS) a
 * 0xffffff800998a2d0 e i suoi quattro byte valgono 01 00 00 00: e'
 * `int gesture_enabled = 1;`, non la definizione senza inizializzatore di
 * ALPS. Lo scrive "b902d109 str"@0xffffff8008a72f24 (w9 = 1) su
 * GESTURE_ENABLE_TOTALLY e lo azzera "b902d11f str"@0xffffff8008a730e0.
 * Subito dopo, a 0xffffff800998a2d8, comincia `hotknot_misc_device`
 * (il primo campo vale 0xff = MISC_DYNAMIC_MINOR) e il suo campo `name`
 * punta a "hotknot"@0xffffff800924aaab. Con questo, il .data dell'oggetto
 * di fabbrica misura 0x58 byte: 4 di gesture_enabled, 4 di riempimento e
 * 0x50 di hotknot_misc_device. Non ci sta la
 * `static struct ratelimit_state ratelimit` (0x28 byte) della copia ALPS.
 * Il nostro .data misura esattamente lo stesso: gesture_enabled a 0,
 * hotknot_misc_device a 8.
 *
 * ---------------------------------------------------------------------
 * 5. QUEL CHE NON TORNA, DICHIARATO
 * ---------------------------------------------------------------------
 * (a) La forma del ciclo di checksum in gesture_event_handler. La fabbrica
 *     cammina con un puntatore e conta all'indietro
 *     ("3840152a ldrb"@0xffffff8008a72454, post-incremento, e
 *     "f100075a subs"@0xffffff8008a72458); il nostro clang -- lo STESSO
 *     clang r353983c -- sceglie un ciclo indicizzato. Sono state provate
 *     dodici scritture diverse del ciclo (indicizzata, con puntatore,
 *     do/while, contatore separato, sizeof del VLA, accumulatore int con
 *     maschera esplicita): la dimensione della funzione resta 1780 in
 *     quattro di esse e cresce nelle altre, ma nessuna riproduce la forma.
 *     La divergenza e' TUTTA li' dentro: 31 posizioni di codifica, di cui
 *     11 nel ciclo, 8 di numerazione dei registri temporanei e 12 di
 *     spiazzamento di salti interni che dipendono dalla disposizione dei
 *     blocchi.
 * (b) L'indirizzo di `c6fc`. Il binario lo mette a 0xffffff800a1006fc,
 *     cioe' quattro byte dopo hotknot_transfer_mode: un allineamento a
 *     quattro, che nel nostro oggetto tocca ai globali NON statici. La
 *     nostra `static u8` finisce a +0x09, impacchettata. Le due prove si
 *     contraddicono e la contraddizione e' dichiarata, non risolta: il
 *     codice (`tbz #0`) chiede `static u8`, l'indirizzo chiede un globale.
 *     Ho scelto di riprodurre il CODICE.
 * (c) Le costanti __LINE__ che non combaciano: 35 su 46. Non ho riempito
 *     il file di righe vuote per farle tornare -- sarebbe stato aggiustare
 *     un numero invece di misurarlo.
 *
 * ---------------------------------------------------------------------
 * 6. LETTERALI DEL BLOCCO NON CITATI ALTROVE
 * ---------------------------------------------------------------------
 * "<<GTP-DBG>>[%s:%d]Entering doze mode...\n"@0xffffff800924a225
 * "<<GTP-DBG>>[%s:%d]Working in doze mode!\n"@0xffffff800924a261
 * "<<GTP-ERR>>[%s:%d] Send doze cmd failed.\n"@0xffffff800924a28a
 * "<<GTP-DBG>>[%s:%d]0x%x = 0x%02X,0x%02X,0x%02X,0x%02X\n"@0xffffff800924a2b4
 * "<<GTP-ERR>>[%s:%d] Gesture contain too many points!(%d)\n"@0xffffff800924a300
 * "<<GTP-ERR>>[%s:%d] Gesture contain too many extra data!(%d)\n"@0xffffff800924a339
 * "<<GTP-ERR>>[%s:%d] Read extra gesture data failed.\n"@0xffffff800924a376
 * "<<GTP-ERR>>[%s:%d] Gesture checksum error.\n"@0xffffff800924a3aa
 * "<<GTP-INF>>[%s:%d] Gesture[0x%02X] has been disabled.\n"@0xffffff800924a3d6
 * "<<GTP-ERR>>[%s:%d] Read gesture data failed.\n"@0xffffff800924a40d
 * "<<GTP-DBG>>[%s:%d]--lan-- Gesture: 0x%02X, points: %d\n"@0xffffff800924a43b
 * "goodix_gesture"@0xffffff800924a4bd
 * "<<GTP-DBG>>[%s:%d]Got the gesture data.\n"@0xffffff800924a609
 * "<<GTP-ERR>>[%s:%d] copy_from_user failed.\n"@0xffffff800924a632
 * "<<GTP-DBG>>[%s:%d]gesture enabled:%x, ret:%d\n"@0xffffff800924a675
 * "<<GTP-ERR>>[%s:%d] Can't access the memory.\n"@0xffffff800924a6a3
 * "GT1X"@0xffffff800924a83f
 * "<<GTP-DBG>>[%s:%d]Obtain gesture data.\n"@0xffffff800924a7b0
 * "<<GTP-ERR>>[%s:%d] ERROR when copy gesture data to user.\n"@0xffffff800924a7d8
 * "<<GTP-DBG>>[%s:%d]ERASE_GESTURE_DATA\n"@0xffffff800924a812
 * "<<GTP-INF>>[%s:%d] Unknown cmd.\n"@0xffffff800924a844
 * "<<GTP-ERR>>[%s:%d] ERROR when copy to user.[addr: %04x], [read length:%d]\n"@0xffffff800924a871
 * "<<GTP-DBG>>[%s:%d]enter transfer mode: %s \n"@0xffffff800924a8bc
 * "GHot"@0xffffff800924a904
 * "<<GTP-ERR>>[%s:%d] Hold ss51 fail!\n"@0xffffff800924a909
 * "<<GTP-INF>>[%s:%d] hotknot load jump code.\n"@0xffffff800924a953
 * "<<GTP-ERR>>[%s:%d] Load jump code fail!\n"@0xffffff800924a97f
 * "<<GTP-INF>>[%s:%d] hotknot load auth code.\n"@0xffffff800924a9a8
 * "<<GTP-ERR>>[%s:%d] Load auth system fail!\n"@0xffffff800924a9d4
 * "<<GTP-ERR>>[%s:%d] load auth system fail!\n"@0xffffff800924a9ff
 * "<<GTP-ERR>>[%s:%d] Startup auth system fail!\n"@0xffffff800924aa2a
 * "<<GTP-ERR>>[%s:%d] i2c read error!\n"@0xffffff800924aa58
 * "<<GTP-INF>>[%s:%d] Current System version: %s\n"@0xffffff800924aa7c
 * "hotknot"@0xffffff800924aaab
 * "<<GTP-DBG>>[%s:%d]Hotknot is enabled.\n"@0xffffff800924aab3
 * "<<GTP-DBG>>[%s:%d]Hotknot is disabled.\n"@0xffffff800924aae7
 * Le due GTP_INFO("%s", ...) di gt1x_ioctl non hanno un letterale "%s"
 * separato: la macro lo salda dentro il formato, che sta a
 * "<<GTP-INF>>[%s:%d] %s\n"@0xffffff800924a6e9.
 *
 * NOTA SULLO STRUMENTO (delta riportato, non applicato): le citazioni qui
 * sopra escono da verificacitazioni.py come NON_ANCORATA, non come
 * verificate. Non e' un difetto delle citazioni -- i byte a quegli
 * indirizzi sono esattamente quelli scritti, terminatore NUL compreso --
 * ma un limite del ramo "messaggio assemblato dalla macro di log", che
 * RE_TESTA_ASSEMBLATA riconosce solo dal prefisso KERN_SOH (\x01 + cifra)
 * che printk("<<GTP-DBG>>...") non ha. La famiglia di macro Goodix
 * incolla "<<GTP-xxx>>[%s:%d]" e "\n" attorno al formato, quindi nessun
 * letterale del codice esiste nel binario come stringa a se'. E' la
 * stessa forma che gt1x_wtk.c gia' usa per gli stessi messaggi.
 * ====================================================================== */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/wait.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/proc_fs.h> /*proc */
#include <linux/ratelimit.h>
#include <linux/uaccess.h>
#ifdef CONFIG_GTP_REQUEST_FW_UPDATE
#include <linux/firmware.h>
#endif
#include "include/gt1x_tpd_common.h"
#include <asm/ioctl.h>

#define GT1151_FW_SIZE 5000
#ifdef CONFIG_GTP_REQUEST_FW_UPDATE
#define GT1151_PATCH_JUMP_FW "gt1151_patch_jump_"
#define HOTKNOT_AUTH_FW "gt1151_hotknot_auth_"
#endif
/*
 * FUORI DALL'#ifdef, e non e' una scelta: hotknot_load_hotknot_system li usa
 * SENZA guardia (gt1x_load_patch(gt1x_patch_jump_fw, ...) e
 * gt1x_load_patch(hotknot_auth_fw, ...)), e quella funzione e' esatta al byte
 * contro la fabbrica -- l'unita' e' 11 su 11.  Tenerli sotto
 * CONFIG_GTP_REQUEST_FW_UPDATE, come fa la copia di GT5688 da cui questo file
 * viene, li lascia INDEFINITI: e' cio' che il primo link di GT917S ha detto,
 * quattro volte.  Nella copia di GT5688 anche gli USI sono guardati; qui no,
 * perche' la fabbrica non li guarda.
 */
unsigned char gt1x_patch_jump_fw[GT1151_FW_SIZE];
unsigned char hotknot_auth_fw[GT1151_FW_SIZE];
#ifdef CONFIG_GTP_GESTURE_WAKEUP

#define GESTURE_NODE "goodix_gesture"
#define GESTURE_MAX_POINT_COUNT 64

#pragma pack(1)
typedef struct {
	u8 ic_msg[6]; /*from the first byte */
	u8 gestures[4];
	u8 data[3 + GESTURE_MAX_POINT_COUNT * 4 +
		80]; /*80 bytes for extra data */
} st_gesture_data;
#pragma pack()

#define SETBIT(longlong, bit) (longlong[bit / 8] |= (1 << bit % 8))
#define CLEARBIT(longlong, bit) (longlong[bit / 8] &= (~(1 << bit % 8)))
#define QUERYBIT(longlong, bit) (!!(longlong[bit / 8] & (1 << bit % 8)))

int gesture_enabled = 1;
enum DOZE_T gesture_doze_status = DOZE_DISABLED;

/*
 * NON : gt1x_wtk.c -- un'altra unita' di traduzione -- lo legge e lo
 * scrive (righe 509 e 512), quindi di fabbrica e' globale.  La copia di GT5688
 * da cui questo file viene lo teneva statico, perche' li' nessun altro file lo
 * tocca.  Anche questa e' una differenza che nessuna misura di DIMENSIONE puo'
 * vedere -- il codice generato e' lo stesso -- e che il link trova subito.
 */
u8 gestures_flag[32];
static st_gesture_data gesture_data;
static struct mutex gesture_data_mutex;
static u8 c6fc;

static ssize_t gt1x_gesture_data_read(struct file *file, char __user *page,
				      size_t size, loff_t *ppos)
{
	s32 ret = -1;

#line 69
	GTP_DEBUG("visit gt1x_gesture_data_read. ppos:%d", (int)*ppos);
	if (*ppos)
		return 0;
	if (size == 4) {
		ret = copy_to_user(((u8 __user *)page), "GT1X", 4);
		return 4;
	}
	ret = simple_read_from_buffer(page, size, ppos, &gesture_data,
				      sizeof(gesture_data));

	GTP_DEBUG("Got the gesture data.");
	return ret;
}

static ssize_t gt1x_gesture_data_write(struct file *filp,
				       const char __user *buff, size_t len,
				       loff_t *off)
{
	s32 ret = 0;

	GTP_DEBUG_FUNC();

	ret = copy_from_user(&gesture_enabled, buff, 1);
	if (ret) {
#line 91
		GTP_ERROR("copy_from_user failed.");
		return -EPERM;
	}

	GTP_DEBUG("gesture enabled:%x, ret:%d", gesture_enabled, ret);

	return len;
}

int gesture_enter_doze(void)
{
	int retry = 0;

	GTP_DEBUG_FUNC();
#line 143
	GTP_DEBUG("Entering doze mode...");
	while (retry++ < 5) {
		if (!gt1x_send_cmd(0x08, 0)) {
			gesture_doze_status = DOZE_ENABLED;
			GTP_DEBUG("Working in doze mode!");
			return 0;
		}
		msleep(10);
	}
	GTP_ERROR("Send doze cmd failed.");
	return -1;
}

s32 gesture_event_handler(struct input_dev *dev)
{
	u8 doze_buf[4] = {0};
	s32 ret = -1;
	int len, extra_len;
	u8 gesture, flag;
	u8 chksum;
	int i;

	if (gesture_doze_status == DOZE_ENABLED) {
		ret = gt1x_i2c_read(GTP_REG_WAKEUP_GESTURE, doze_buf, 4);
		if (ret < 0)
			return 0;
		gesture = doze_buf[0];
		len = doze_buf[1];
		flag = doze_buf[2];
		extra_len = doze_buf[3];
		GTP_DEBUG("0x%x = 0x%02X,0x%02X,0x%02X,0x%02X",
#line 186
			  GTP_REG_WAKEUP_GESTURE, gesture, len, flag, extra_len);
		if (len > GESTURE_MAX_POINT_COUNT) {
#line 189
			GTP_ERROR("Gesture contain too many points!(%d)", len);
			len = GESTURE_MAX_POINT_COUNT;
		}
		if (extra_len > 32) {
			GTP_ERROR("Gesture contain too many extra data!(%d)",
				  extra_len);
			extra_len = 32;
		}
		{
			u8 buf[extra_len + 1];

			ret = gt1x_i2c_read(GTP_REG_WAKEUP_GESTURE + 4, buf,
					    extra_len + 1);
			if (ret < 0) {
#line 206
				GTP_ERROR("Read extra gesture data failed.");
				return 0;
			}
			if (flag & 0x80) {
				buf[extra_len] += doze_buf[0] + doze_buf[1] +
						  doze_buf[2] + doze_buf[3];
				chksum = 0;
				for (i = 0; i < extra_len + 1; i++)
					chksum += buf[i];
				if (chksum) {
#line 218
					GTP_ERROR("Gesture checksum error.");
					if (c6fc) {
						c6fc = 0;
						ret = 0;
						goto clear_gesture_reg;
					}
					c6fc = 1;
					return 0;
				}
				c6fc = 0;
			}
			mutex_lock(&gesture_data_mutex);
			memcpy(&gesture_data.data[4 + len * 4], buf, extra_len);
			mutex_unlock(&gesture_data_mutex);
		}
		if (!gesture || !QUERYBIT(gestures_flag, gesture)) {
			GTP_INFO("Gesture[0x%02X] has been disabled.",
#line 242
				 doze_buf[0]);
			doze_buf[0] = 0;
			gt1x_i2c_write(GTP_REG_WAKEUP_GESTURE, doze_buf, 1);
			gesture_enter_doze();
			return 0;
		}
		if (len) {
			u8 coor_buf[len * 4 + 2];

			ret = gt1x_i2c_read(GTP_REG_WAKEUP_GESTURE_DETAIL,
					    coor_buf, len * 4);
			if (ret < 0) {
#line 255
				GTP_ERROR("Read gesture data failed.");
				return 0;
			}
			ret = gt1x_i2c_read(0x819F, &coor_buf[len * 4], 2);
			if (ret < 0) {
#line 262
				GTP_ERROR("Read gesture data failed.");
				return 0;
			}
			mutex_lock(&gesture_data_mutex);
			memcpy(&gesture_data.data[4], coor_buf, len * 4);
			mutex_unlock(&gesture_data_mutex);
		}
		mutex_lock(&gesture_data_mutex);
		gesture_data.data[0] = gesture; /*gesture type*/
		gesture_data.data[1] = len; /*gesture points number*/
		gesture_data.data[2] = doze_buf[2] & 0x7f;
		gesture_data.data[3] = extra_len;
		mutex_unlock(&gesture_data_mutex);

		GTP_DEBUG("--lan-- Gesture: 0x%02X, points: %d", doze_buf[0],
#line 296
			  doze_buf[1]);

		switch (gesture_data.data[0]) {
		case 0x5e:
			input_report_key(dev, KEY_POWER, 1);
			input_sync(dev);
			input_report_key(dev, KEY_POWER, 0);
			input_sync(dev);
			break;
		case 0x63:
			input_report_key(dev, KEY_C, 1);
			input_sync(dev);
			input_report_key(dev, KEY_C, 0);
			input_sync(dev);
			break;
		case 0x65:
			input_report_key(dev, KEY_E, 1);
			input_sync(dev);
			input_report_key(dev, KEY_E, 0);
			input_sync(dev);
			break;
		case 0x6d:
			input_report_key(dev, KEY_M, 1);
			input_sync(dev);
			input_report_key(dev, KEY_M, 0);
			input_sync(dev);
			break;
		case 0x6f:
			input_report_key(dev, KEY_O, 1);
			input_sync(dev);
			input_report_key(dev, KEY_O, 0);
			input_sync(dev);
			break;
		case 0x73:
			input_report_key(dev, KEY_S, 1);
			input_sync(dev);
			input_report_key(dev, KEY_S, 0);
			input_sync(dev);
			break;
		case 0x76:
			input_report_key(dev, KEY_V, 1);
			input_sync(dev);
			input_report_key(dev, KEY_V, 0);
			input_sync(dev);
			break;
		case 0x77:
			input_report_key(dev, KEY_W, 1);
			input_sync(dev);
			input_report_key(dev, KEY_W, 0);
			input_sync(dev);
			break;
		case 0x7a:
			input_report_key(dev, KEY_Z, 1);
			input_sync(dev);
			input_report_key(dev, KEY_Z, 0);
			input_sync(dev);
			break;
		case 0xaa:
			input_report_key(dev, KEY_RIGHT, 1);
			input_sync(dev);
			input_report_key(dev, KEY_RIGHT, 0);
			input_sync(dev);
			break;
		case 0xab:
			input_report_key(dev, KEY_DOWN, 1);
			input_sync(dev);
			input_report_key(dev, KEY_DOWN, 0);
			input_sync(dev);
			break;
		case 0xba:
			input_report_key(dev, KEY_UP, 1);
			input_sync(dev);
			input_report_key(dev, KEY_UP, 0);
			input_sync(dev);
			break;
		case 0xbb:
			input_report_key(dev, KEY_LEFT, 1);
			input_sync(dev);
			input_report_key(dev, KEY_LEFT, 0);
			input_sync(dev);
			break;
		case 0xcc:
			input_report_key(dev, KEY_F, 1);
			input_sync(dev);
			input_report_key(dev, KEY_F, 0);
			input_sync(dev);
			break;
		default:
			break;
		}

clear_gesture_reg:
		doze_buf[0] = 0;
		gt1x_i2c_write(GTP_REG_WAKEUP_GESTURE, doze_buf, 1);
		return ret;
	}
	return -1;
}

void gesture_clear_wakeup_data(void)
{
	mutex_lock(&gesture_data_mutex);
	memset(gesture_data.data, 0, 4);
	mutex_unlock(&gesture_data_mutex);
}
#endif /*CONFIG_GTP_GESTURE_WAKEUP*/

/*HotKnot module*/
#ifdef CONFIG_GTP_HOTKNOT

#define HOTKNOT_NODE "hotknot"

u8 hotknot_enabled;
u8 hotknot_transfer_mode;

static int hotknot_open(struct inode *node, struct file *flip)
{
#line 445
	GTP_DEBUG("Hotknot is enabled.");
	hotknot_enabled = 1;
	return 0;
}

static int hotknot_release(struct inode *node, struct file *filp)
{
	GTP_DEBUG("Hotknot is disabled.");
	hotknot_enabled = 0;
	return 0;
}

static s32 hotknot_enter_transfer_mode(void)
{
	int ret = 0;
	u8 buffer[5] = {0};

	hotknot_transfer_mode = 1;
#ifdef CONFIG_GTP_ESD_PROTECT
	gt1x_esd_switch(SWITCH_OFF);
#endif

	gt1x_irq_disable();
	gt1x_send_cmd(GTP_CMD_HN_TRANSFER, 0);
	msleep(100);
	gt1x_irq_enable();

	ret = gt1x_i2c_read(0x8140, buffer, sizeof(buffer));
	if (ret) {
		hotknot_transfer_mode = 0;
		return ret;
	}

	buffer[4] = 0;
	GTP_DEBUG("enter transfer mode: %s ", buffer);
	if (strcmp(buffer, "GHot")) {
		hotknot_transfer_mode = 0;
		return ERROR_HN_VER;
	}

	return 0;
}

static s32 hotknot_load_hotknot_subsystem(void)
{
	return hotknot_enter_transfer_mode();
}

static s32 hotknot_load_authentication_subsystem(void)
{
	s32 ret = 0;
	u8 buffer[5] = {0};

	ret = gt1x_hold_ss51_dsp_no_reset();
	if (ret < 0) {
#line 499
		GTP_ERROR("Hold ss51 fail!");
		return ERROR;
	}

	if (gt1x_chip_type == CHIP_TYPE_GT1X) {
		GTP_INFO("hotknot load jump code.");
		ret = gt1x_load_patch(gt1x_patch_jump_fw, 4096, 0, 1024 * 8);
		if (ret < 0) {
			GTP_ERROR("Load jump code fail!");
			return ret;
		}
		GTP_INFO("hotknot load auth code.");
		ret = gt1x_load_patch(hotknot_auth_fw, 4096, 4096, 1024 * 8);
		if (ret < 0) {
			GTP_ERROR("Load auth system fail!");
			return ret;
		}
	} else {
		GTP_INFO("hotknot load auth code.");
		ret = gt1x_load_patch(hotknot_auth_fw, 4096, 0, 1024 * 6);
		if (ret < 0) {
			GTP_ERROR("load auth system fail!");
			return ret;
		}
	}

	ret = gt1x_startup_patch();
	if (ret < 0) {
		GTP_ERROR("Startup auth system fail!");
		return ret;
	}
	ret = gt1x_i2c_read(GTP_REG_VERSION, buffer, 4);
	if (ret < 0) {
		GTP_ERROR("i2c read error!");
		return ERROR_IIC;
	}
	buffer[4] = 0;
	GTP_INFO("Current System version: %s", buffer);
	return 0;
}

static s32 hotknot_recovery_main_system(void)
{
	gt1x_irq_disable();
	gt1x_reset_guitar();
	gt1x_irq_enable();
#ifdef CONFIG_GTP_ESD_PROTECT
	gt1x_esd_switch(SWITCH_ON);
#endif
	hotknot_transfer_mode = 0;
	return 0;
}

#ifdef CONFIG_HOTKNOT_BLOCK_RW
DECLARE_WAIT_QUEUE_HEAD(bp_waiter);
static u8 got_hotknot_state;
static u8 got_hotknot_extra_state;
static u8 wait_hotknot_state;
static u8 force_wake_flag;
static u8 block_enable;
s32 hotknot_paired_flag;

static s32 hotknot_block_rw(u8 rqst_hotknot_state, s32 wait_hotknot_timeout)
{
	s32 ret = 0;

	wait_hotknot_state |= rqst_hotknot_state;
	GTP_DEBUG(
		"Goodix tool received wait polling state:0x%x,timeout:%d, all wait state:0x%x",
		rqst_hotknot_state, wait_hotknot_timeout, wait_hotknot_state);
	got_hotknot_state &= (~rqst_hotknot_state);

	set_current_state(TASK_INTERRUPTIBLE);
	if (wait_hotknot_timeout <= 0) {
		wait_event_interruptible(bp_waiter,
					 force_wake_flag ||
						 rqst_hotknot_state ==
							 (got_hotknot_state &
							  rqst_hotknot_state));
	} else {
		wait_event_interruptible_timeout(
			bp_waiter,
			force_wake_flag ||
				rqst_hotknot_state == (got_hotknot_state &
						       rqst_hotknot_state),
			wait_hotknot_timeout);
	}

	wait_hotknot_state &= (~rqst_hotknot_state);

	if (rqst_hotknot_state != (got_hotknot_state & rqst_hotknot_state)) {
		GTP_ERROR("Wait 0x%x block polling waiter failed.",
			  rqst_hotknot_state);
		ret = -1;
	}

	force_wake_flag = 0;
	return ret;
}

static void hotknot_wakeup_block(void)
{
	GTP_DEBUG("Manual wakeup all block polling waiter!");
	got_hotknot_state = 0;
	wait_hotknot_state = 0;
	force_wake_flag = 1;
	hotknot_paired_flag = 0;
	wake_up_interruptible(&bp_waiter);
}

s32 hotknot_event_handler(u8 *data)
{
	u8 hn_pxy_state = 0;
	u8 hn_pxy_state_bak = 0;
	static u8 hn_paired_cnt;
	u8 hn_state_buf[10] = {0};
	u8 finger = data[0];
	u8 id = 0;

	if (block_enable && !hotknot_paired_flag && (finger & 0x0F)) {
		id = data[1];
		hn_pxy_state = data[2] & 0x80;
		hn_pxy_state_bak = data[3] & 0x80;
		if ((id == 32) && (hn_pxy_state == 0x80) &&
		    (hn_pxy_state_bak == 0x80)) {
#ifdef HN_DBLCFM_PAIRED
			if (hn_paired_cnt++ < 2)
				return 0;
#endif
			GTP_DEBUG("HotKnot paired!");
			if (wait_hotknot_state & HN_DEVICE_PAIRED) {
				GTP_DEBUG(
					"INT wakeup HN_DEVICE_PAIRED block polling waiter");
				got_hotknot_state |= HN_DEVICE_PAIRED;
				wake_up_interruptible(&bp_waiter);
			}
			block_enable = 0;
			hotknot_paired_flag = 1;
			return 0;
		}
		got_hotknot_state &= (~HN_DEVICE_PAIRED);
		hn_paired_cnt = 0;
	}

	if (hotknot_paired_flag) {
		s32 ret = -1;

		ret = gt1x_i2c_read(GTP_REG_HN_STATE, hn_state_buf, 6);
		if (ret < 0) {
			GTP_ERROR("I2C transfer error. errno:%d\n ", ret);
			return 0;
		}

		got_hotknot_state = 0;

		GTP_DEBUG("wait_hotknot_state:%x", wait_hotknot_state);
		GTP_DEBUG("[0x8800~0x8803]=0x%x,0x%x,0x%x,0x%x",
			  hn_state_buf[0], hn_state_buf[1], hn_state_buf[2],
			  hn_state_buf[3]);

		if (wait_hotknot_state & HN_MASTER_SEND) {
			if ((hn_state_buf[0] == 0x03) ||
			    (hn_state_buf[0] == 0x04) ||
			    (hn_state_buf[0] == 0x07)) {
				GTP_DEBUG(
					"Wakeup HN_MASTER_SEND block polling waiter");
				got_hotknot_state |= HN_MASTER_SEND;
				got_hotknot_extra_state = hn_state_buf[0];
				wake_up_interruptible(&bp_waiter);
			}
		} else if (wait_hotknot_state & HN_SLAVE_RECEIVED) {
			if ((hn_state_buf[1] == 0x03) ||
			    (hn_state_buf[1] == 0x04) ||
			    (hn_state_buf[1] == 0x07)) {
				GTP_DEBUG(
					"Wakeup HN_SLAVE_RECEIVED block polling waiter:0x%x",
					hn_state_buf[1]);
				got_hotknot_state |= HN_SLAVE_RECEIVED;
				got_hotknot_extra_state = hn_state_buf[1];
				wake_up_interruptible(&bp_waiter);
			}
		} else if (wait_hotknot_state & HN_MASTER_DEPARTED) {
			if (hn_state_buf[0] == 0x07) {
				GTP_DEBUG(
					"Wakeup HN_MASTER_DEPARTED block polling waiter");
				got_hotknot_state |= HN_MASTER_DEPARTED;
				wake_up_interruptible(&bp_waiter);
			}
		} else if (wait_hotknot_state & HN_SLAVE_DEPARTED) {
			if (hn_state_buf[1] == 0x07) {
				GTP_DEBUG(
					"Wakeup HN_SLAVE_DEPARTED block polling waiter");
				got_hotknot_state |= HN_SLAVE_DEPARTED;
				wake_up_interruptible(&bp_waiter);
			}
		}
		return 0;
	}

	return -1;
}
#endif /*CONFIG_HOTKNOT_BLOCK_RW*/
#endif /*CONFIG_GTP_HOTKNOT*/

#define GOODIX_MAGIC_NUMBER 'G'
#define NEGLECT_SIZE_MASK (~(_IOC_SIZEMASK << _IOC_SIZESHIFT))

#define GESTURE_ENABLE_TOTALLY _IO(GOODIX_MAGIC_NUMBER, 1) /* 1*/
#define GESTURE_DISABLE_TOTALLY _IO(GOODIX_MAGIC_NUMBER, 2)
#define GESTURE_ENABLE_PARTLY _IO(GOODIX_MAGIC_NUMBER, 3)
#define GESTURE_DISABLE_PARTLY _IO(GOODIX_MAGIC_NUMBER, 4)
/*#define SET_ENABLED_GESTURE         */
/* (_IOW(GOODIX_MAGIC_NUMBER, 5, u8) & NEGLECT_SIZE_MASK)*/
#define GESTURE_DATA_OBTAIN                                                    \
	(_IOR(GOODIX_MAGIC_NUMBER, 6, u8) & NEGLECT_SIZE_MASK)
#define GESTURE_DATA_ERASE _IO(GOODIX_MAGIC_NUMBER, 7)

/*#define HOTKNOT_LOAD_SUBSYSTEM (_IOW(GOODIX_MAGIC_NUMBER, 6, u8) & */
/* NEGLECT_SIZE_MASK)*/
#define HOTKNOT_LOAD_HOTKNOT _IO(GOODIX_MAGIC_NUMBER, 20)
#define HOTKNOT_LOAD_AUTHENTICATION _IO(GOODIX_MAGIC_NUMBER, 21)
#define HOTKNOT_RECOVERY_MAIN _IO(GOODIX_MAGIC_NUMBER, 22)
/*#define HOTKNOT_BLOCK_RW      (_IOW(GOODIX_MAGIC_NUMBER, 6, u8) & */
/* NEGLECT_SIZE_MASK)*/
#define HOTKNOT_DEVICES_PAIRED _IO(GOODIX_MAGIC_NUMBER, 23)
#define HOTKNOT_MASTER_SEND _IO(GOODIX_MAGIC_NUMBER, 24)
#define HOTKNOT_SLAVE_RECEIVE _IO(GOODIX_MAGIC_NUMBER, 25)
/*#define HOTKNOT_DEVICES_COMMUNICATION*/
#define HOTKNOT_MASTER_DEPARTED _IO(GOODIX_MAGIC_NUMBER, 26)
#define HOTKNOT_SLAVE_DEPARTED _IO(GOODIX_MAGIC_NUMBER, 27)
#define HOTKNOT_WAKEUP_BLOCK _IO(GOODIX_MAGIC_NUMBER, 29)

#define IO_IIC_READ (_IOR(GOODIX_MAGIC_NUMBER, 100, u8) & NEGLECT_SIZE_MASK)
#define IO_IIC_WRITE (_IOW(GOODIX_MAGIC_NUMBER, 101, u8) & NEGLECT_SIZE_MASK)
#define IO_RESET_GUITAR _IO(GOODIX_MAGIC_NUMBER, 102)
#define IO_DISABLE_IRQ _IO(GOODIX_MAGIC_NUMBER, 103)
#define IO_ENABLE_IRQ _IO(GOODIX_MAGIC_NUMBER, 104)
#define IO_GET_VERSION (_IOR(GOODIX_MAGIC_NUMBER, 110, u8) & NEGLECT_SIZE_MASK)
#define IO_PRINT (_IOW(GOODIX_MAGIC_NUMBER, 111, u8) & NEGLECT_SIZE_MASK)
#define IO_VERSION "V1.3-20150420"
#define IO_NR28 (_IOR(GOODIX_MAGIC_NUMBER, 28, u8) & NEGLECT_SIZE_MASK)
#ifdef CONFIG_COMPAT
#define COMPAT_GESTURE_ENABLE_TOTALLY _IO(GOODIX_MAGIC_NUMBER, 1) /*1*/
#define COMPAT_GESTURE_DISABLE_TOTALLY _IO(GOODIX_MAGIC_NUMBER, 2)
#define COMPAT_GESTURE_ENABLE_PARTLY _IO(GOODIX_MAGIC_NUMBER, 3)
#define COMPAT_GESTURE_DISABLE_PARTLY _IO(GOODIX_MAGIC_NUMBER, 4)
/*#define SET_ENABLED_GESTURE  (_IOW(GOODIX_MAGIC_NUMBER, 5, u8) & */
/* NEGLECT_SIZE_MASK)*/
#define COMPAT_GESTURE_DATA_OBTAIN                                             \
	(_IOR(GOODIX_MAGIC_NUMBER, 6, u8) & NEGLECT_SIZE_MASK)
#define COMPAT_GESTURE_DATA_ERASE _IO(GOODIX_MAGIC_NUMBER, 7)

/*#define HOTKNOT_LOAD_SUBSYSTEM  (_IOW(GOODIX_MAGIC_NUMBER, 6, u8) & */
/* NEGLECT_SIZE_MASK)*/
#define COMPAT_HOTKNOT_LOAD_HOTKNOT _IO(GOODIX_MAGIC_NUMBER, 20)
#define COMPAT_HOTKNOT_LOAD_AUTHENTICATION _IO(GOODIX_MAGIC_NUMBER, 21)
#define COMPAT_HOTKNOT_RECOVERY_MAIN _IO(GOODIX_MAGIC_NUMBER, 22)
/*#define HOTKNOT_BLOCK_RW   (_IOW(GOODIX_MAGIC_NUMBER, 6, u8) & */
/* NEGLECT_SIZE_MASK)*/
#define COMPAT_HOTKNOT_DEVICES_PAIRED _IO(GOODIX_MAGIC_NUMBER, 23)
#define COMPAT_HOTKNOT_MASTER_SEND _IO(GOODIX_MAGIC_NUMBER, 24)
#define COMPAT_HOTKNOT_SLAVE_RECEIVE _IO(GOODIX_MAGIC_NUMBER, 25)
/*#define HOTKNOT_DEVICES_COMMUNICATION*/
#define COMPAT_HOTKNOT_MASTER_DEPARTED _IO(GOODIX_MAGIC_NUMBER, 26)
#define COMPAT_HOTKNOT_SLAVE_DEPARTED _IO(GOODIX_MAGIC_NUMBER, 27)
#define COMPAT_HOTKNOT_WAKEUP_BLOCK _IO(GOODIX_MAGIC_NUMBER, 29)

#define COMPAT_IO_IIC_READ                                                     \
	(_IOR(GOODIX_MAGIC_NUMBER, 100, u8) & NEGLECT_SIZE_MASK)
#define COMPAT_IO_IIC_WRITE                                                    \
	(_IOW(GOODIX_MAGIC_NUMBER, 101, u8) & NEGLECT_SIZE_MASK)
#define COMPAT_IO_RESET_GUITAR _IO(GOODIX_MAGIC_NUMBER, 102)
#define COMPAT_IO_DISABLE_IRQ _IO(GOODIX_MAGIC_NUMBER, 103)
#define COMPAT_IO_ENABLE_IRQ _IO(GOODIX_MAGIC_NUMBER, 104)
#define COMPAT_IO_GET_VERSION                                                  \
	(_IOR(GOODIX_MAGIC_NUMBER, 110, u8) & NEGLECT_SIZE_MASK)
#define COMPAT_IO_PRINT (_IOW(GOODIX_MAGIC_NUMBER, 111, u8) & NEGLECT_SIZE_MASK)
#endif
#define CMD_HEAD_LENGTH 20
static s32 io_iic_read(u8 *data, int buf_size, void __user *arg)
{
	s32 err = ERROR;
	s32 data_length = 0;
	u16 addr = 0;

	err = copy_from_user(data, arg, CMD_HEAD_LENGTH);
	if (err) {
		GTP_ERROR("Can't access the memory.");
		return ERROR_MEM;
	}

	addr = data[0] << 8 | data[1];
	data_length = data[2] << 8 | data[3];

	err = gt1x_i2c_read(addr, &data[CMD_HEAD_LENGTH], data_length);
	if (!err) {
		err = copy_to_user(&((u8 __user *)arg)[CMD_HEAD_LENGTH],
				   &data[CMD_HEAD_LENGTH], data_length);
		if (err) {
			GTP_ERROR(
				"ERROR when copy to user.[addr: %04x], [read length:%d]",
#line 734
				addr, data_length);
			return ERROR_MEM;
		}
		err = CMD_HEAD_LENGTH + data_length;
	}

	return err;
}

static s32 io_iic_write(u8 *data)
{
	s32 err = ERROR;
	s32 data_length = 0;
	u16 addr = 0;

	addr = data[0] << 8 | data[1];
	data_length = data[2] << 8 | data[3];

	err = gt1x_i2c_write(addr, &data[CMD_HEAD_LENGTH], data_length);
	if (!err)
		err = CMD_HEAD_LENGTH + data_length;

	return err;
}

/*@return, 0:operate successfully
 *         > 0: the length of memory size ioctl has accessed,
 *         error otherwise.
 */
static long gt1x_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	u32 value = 0;
	s32 ret = 0; /*the initial value must be 0*/
	u8 *data = NULL;
	int cnt = 30;
	s32 data_length = 0;

	/* Blocking when firmwaer updating */
	while (cnt-- && update_info.status)
		ssleep(1);

	if (_IOC_DIR(cmd)) {
		s32 err = -1;

		data_length = _IOC_SIZE(cmd);
		data = kzalloc(data_length, GFP_KERNEL);
		memset(data, 0, data_length);

		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			err = copy_from_user(data, (void __user *)arg,
					     data_length);
			if (err) {
				GTP_ERROR("Can't access the memory.");
				kfree(data);
				return -1;
			}
		}
	} else {
		value = (u32)arg;
	}

	switch (cmd & NEGLECT_SIZE_MASK) {
	case IO_GET_VERSION:
		if ((u8 __user *)arg) {
			ret = copy_to_user(((u8 __user *)arg), IO_VERSION,
					   sizeof(IO_VERSION));
			if (!ret)
				ret = sizeof(IO_VERSION);
#line 811
			GTP_INFO("%s", IO_VERSION);
		}
		break;
	case IO_NR28:
		ret = copy_to_user(((u8 __user *)arg), "GOODIX,GT1X",
				   sizeof("GOODIX,GT1X"));
		if (!ret)
			ret = sizeof("GOODIX,GT1X");
		break;
	case IO_IIC_READ:
		ret = io_iic_read(data, data_length, (void __user *)arg);
		break;

	case IO_IIC_WRITE:
		ret = io_iic_write(data);
		break;

	case IO_RESET_GUITAR:
		gt1x_irq_disable();
		gt1x_reset_guitar();
		gt1x_irq_enable();
		break;

	case IO_DISABLE_IRQ:
		gt1x_irq_disable();
		break;

	case IO_ENABLE_IRQ:
		gt1x_irq_enable();
		break;

	/*print a string to syc log messages between application and kernel.*/
	case IO_PRINT:
		if (data)
			GTP_INFO("%s", (char *)data);
		break;

#ifdef CONFIG_GTP_GESTURE_WAKEUP
	case GESTURE_ENABLE_TOTALLY:
		GTP_DEBUG("Gesture switch ON.");
		gesture_enabled = 1;
		break;

	case GESTURE_DISABLE_TOTALLY:
		GTP_DEBUG("Gesture switch OFF.");
		gesture_enabled = 0;
		break;

	case GESTURE_ENABLE_PARTLY:
		SETBIT(gestures_flag, (u8)value);
		GTP_DEBUG("Gesture flag: 0x%02X enabled.", value);
		break;

	case GESTURE_DISABLE_PARTLY:
		CLEARBIT(gestures_flag, (u8)value);
		GTP_DEBUG("Gesture flag: 0x%02X disabled.", value);
		break;

	case GESTURE_DATA_OBTAIN:
		GTP_DEBUG("Obtain gesture data.");

		mutex_lock(&gesture_data_mutex);
		ret = copy_to_user(((u8 __user *)arg), &gesture_data.data,
				   4 + gesture_data.data[1] * 4 +
					   gesture_data.data[3]);
		if (ret) {
#line 874
			GTP_ERROR("ERROR when copy gesture data to user.");
			ret = ERROR_MEM;
		} else {
			ret = 4 + gesture_data.data[1] * 4 +
			      gesture_data.data[3];
		}
		mutex_unlock(&gesture_data_mutex);
		break;

	case GESTURE_DATA_ERASE:
#line 883
		GTP_DEBUG("ERASE_GESTURE_DATA");
		gesture_clear_wakeup_data();
		break;
#endif /*CONFIG_GTP_GESTURE_WAKEUP*/

#ifdef CONFIG_GTP_HOTKNOT
	case HOTKNOT_LOAD_HOTKNOT:
		ret = hotknot_load_hotknot_subsystem();
		break;

	case HOTKNOT_LOAD_AUTHENTICATION:
		ret = hotknot_load_authentication_subsystem();
		break;

	case HOTKNOT_RECOVERY_MAIN:
		ret = hotknot_recovery_main_system();
		break;
#ifdef CONFIG_HOTKNOT_BLOCK_RW
	case HOTKNOT_DEVICES_PAIRED:
		hotknot_paired_flag = 0;
		force_wake_flag = 0;
		block_enable = 1;
		ret = hotknot_block_rw(HN_DEVICE_PAIRED, (s32)value);
		break;

	case HOTKNOT_MASTER_SEND:
		ret = hotknot_block_rw(HN_MASTER_SEND, (s32)value);
		if (!ret)
			ret = got_hotknot_extra_state;
		break;

	case HOTKNOT_SLAVE_RECEIVE:
		ret = hotknot_block_rw(HN_SLAVE_RECEIVED, (s32)value);
		if (!ret)
			ret = got_hotknot_extra_state;
		break;

	case HOTKNOT_MASTER_DEPARTED:
		ret = hotknot_block_rw(HN_MASTER_DEPARTED, (s32)value);
		break;

	case HOTKNOT_SLAVE_DEPARTED:
		ret = hotknot_block_rw(HN_SLAVE_DEPARTED, (s32)value);
		break;

	case HOTKNOT_WAKEUP_BLOCK:
		hotknot_wakeup_block();
		break;
#endif /*CONFIG_HOTKNOT_BLOCK_RW*/
#endif /*CONFIG_GTP_HOTKNOT*/

	default:
#line 944
		GTP_INFO("Unknown cmd.");
		ret = -1;
		break;
	}

	if (data != NULL)
		kfree(data);
	return ret;
}
#ifdef CONFIG_GTP_HOTKNOT
#ifdef CONFIG_COMPAT
/* Di fabbrica NON c'e' switch: 56 byte in tutto, 14 istruzioni, e le sole
 * costanti sono 40, 72 e -25. Il corpo e' un rinvio secco.
 *   "f9401408 ldr"@0xffffff8008a737b0   ldr x8, [x0,#40]  -> file->f_op
 *   "f9402508 ldr"@0xffffff8008a737b8   ldr x8, [x8,#72]  -> ->unlocked_ioctl
 *   "b27bf7e0 orr"@0xffffff8008a737d8   x0 = -25 = -ENOTTY, su entrambi i cbz
 *   "92407c42 and"@0xffffff8008a737c8   x2 &= 0xffffffff  -> compat_ptr(arg)
 *   "d63f0100 blr"@0xffffff8008a737cc   chiamata indiretta, unica nel corpo
 * La `and` sta DOPO i due cbz e prima della blr: e' l'unico uso di arg32.
 */
static long gt1x_compat_ioctl(struct file *file, unsigned int cmd,
			      unsigned long arg)
{
	long ret;
	void __user *arg32 = compat_ptr(arg);

	if (!file->f_op || !file->f_op->unlocked_ioctl)
		return -ENOTTY;

	ret = file->f_op->unlocked_ioctl(file, cmd, (unsigned long)arg32);

	return ret;
}
#endif
#endif
static const struct file_operations gt1x_fops = {
	.owner = THIS_MODULE,
#ifdef CONFIG_GTP_GESTURE_WAKEUP
	.read = gt1x_gesture_data_read,
	.write = gt1x_gesture_data_write,
#endif
	.unlocked_ioctl = gt1x_ioctl,
};

#ifdef CONFIG_GTP_HOTKNOT
static const struct file_operations hotknot_fops = {
	.unlocked_ioctl = gt1x_ioctl,
	.open = hotknot_open,
	.release = hotknot_release,
#ifdef CONFIG_COMPAT
	.compat_ioctl = gt1x_compat_ioctl,
#endif
};

static struct miscdevice hotknot_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = HOTKNOT_NODE,
	.fops = &hotknot_fops,
};
#endif

s32 gt1x_init_node(void)
{
#ifdef CONFIG_GTP_GESTURE_WAKEUP
	struct proc_dir_entry *proc_entry = NULL;

	mutex_init(&gesture_data_mutex);
	memset(gestures_flag, 0xff, sizeof(gestures_flag));
	memset((u8 *)&gesture_data, 0, sizeof(st_gesture_data));

	proc_entry = proc_create(GESTURE_NODE, 0666, NULL, &gt1x_fops);
	if (proc_entry == NULL) {
#line 1007
		GTP_ERROR("CAN't create proc entry /proc/%s.", GESTURE_NODE);
		return -1;
	}
	GTP_INFO("Created proc entry /proc/%s.", GESTURE_NODE);
#endif

#ifdef CONFIG_GTP_HOTKNOT
	if (misc_register(&hotknot_misc_device)) {
#line 1016
		GTP_ERROR("CAN't create misc device in /dev/hotknot.");
		return -1;
	}
	GTP_INFO("Created misc device in /dev/hotknot.");
#endif
	return 0;
}

void gt1x_deinit_node(void)
{
#ifdef CONFIG_GTP_GESTURE_WAKEUP
	remove_proc_entry(GESTURE_NODE, NULL);
#endif

#ifdef CONFIG_GTP_HOTKNOT
	misc_deregister(&hotknot_misc_device);
#endif
}
