// SPDX-License-Identifier: GPL-2.0
/*
 * WTK_FT8719_E977 -- FocalTech FT8719, Doogee S88 Pro (MT6771).
 * L'UNITA' DI TRADUZIONE DEL BUS I2C (unita' E), e SOLO quella.
 * Lotto PARZIALE dichiarato.
 *
 * RICOSTRUITO DAL DISASSEMBLATO DEL KERNEL DI FABBRICA.  Nessun sorgente
 * pubblico e' stato letto.  L'albero ALPS su cui questo file compila
 * CONTIENE un driver FocalTech pubblico
 * (drivers/input/touchscreen/mediatek/focaltech_fhd_touch/ e
 * .../focaltech_touch/): NON sono stati aperti.
 *
 * ===========================================================================
 * 1. CHE COSA C'E' QUI DENTRO, E QUAL E' IL CONFINE
 * ===========================================================================
 * Il blocco WTK_FT8719_E977 e' 67 funzioni nella mappa piu' una in
 * `.exit.text` che la mappa non vede (68 in tutto, 22092 byte), ripartite in
 * SEI unita' di traduzione.  La partizione e' MISURATA -- l'ordine del testo,
 * quello di `.data` e quello di `.bss` danno la stessa -- ed e' la tabella §5
 * di /mnt/s88pro/kernel-stock/scout-ft8719/RIASSUNTO.md.
 *
 * Questo file e' l'unita' E:
 *   testo   0xffffff8008a802ac .. 0xffffff8008a807e0   7 funzioni, 1332 byte
 *   .data   0xffffff800998cbe0 .. 0xffffff800998cc00   32 byte, una sola
 *                                                      `struct mutex`
 *   .bss    -- (nessuna)
 *
 * Che sia un file oggetto A SE' -- e non un pezzo di focaltech_core.c -- lo
 * prova il numero di riga che `FTS_FUNC_EXIT` lascia nel binario:
 *   "52802e42 mov"@0xffffff8008a8077c   w2 = 0x172 = 370   (fts_i2c_init)
 *   "52803042 mov"@0xffffff8008a807c4   w2 = 0x182 = 386   (fts_i2c_exit)
 * mentre nell'unita' C le righe sono 270, 290, 314, 337, 451, 852, 1089,
 * 1104, 1160, 1182, 1207, 1275, 1381, 1396.  Le righe 370 e 386 cadrebbero
 * dentro `fts_release_all_finger` (451) e `fts_irq_enable` (337), cioe' in
 * mezzo a funzioni che stanno 0x1500 byte piu' INDIETRO nel testo: due
 * numerazioni diverse, quindi due file.
 *
 * PERCHE' UN FILE NUOVO E NON UN'AGGIUNTA A focaltech_core.c.  E' una
 * DECISIONE, e la ragione e' misurabile: mettere le due unita' nello stesso
 * `.c` darebbe a clang la facolta' di INCORPORARE `fts_i2c_read` e
 * `fts_i2c_write` nei loro chiamanti dell'unita' C, cosa che la fabbrica non
 * ha potuto fare perche' li' sono due file oggetto distinti (nessun LTO: il
 * `bl` c'e', "94002b32 bl"@0xffffff8008a80338 e' un `bl` vero e non una
 * fusione).  Che clang lo faccia davvero e' provato dentro l'unita' C, dove
 * `fts_reset_proc` -- funzione con simbolo proprio -- e' incorporata in
 * `tpd_probe` E in `tpd_resume`.  Un file solo cambierebbe la dimensione di
 * funzioni gia' misurate.
 * Il NOME `focaltech_i2c.c` e' anch'esso una scelta: il binario non contiene
 * i nomi dei file di fabbrica (il `.ddebug` non nomina nessun file
 * focaltech -- `ddebug.py --file focaltech` -> "dopo il filtro: 0").
 *
 * ===========================================================================
 * 2. COME SI RIVERIFICA
 * ===========================================================================
 * Sul PC di build, in /mnt/s88pro/kernel-stock:
 *
 *   ./venv/bin/python3 verificacitazioni.py focaltech_i2c.c oracolo/stock.elf \
 *       --eccezione "[FTS]" --eccezione "[FTS][Error]" --eccezione "\n" \
 *       --eccezione "GPL"
 *
 *   ./venv/bin/python3 verificaistruzioni.py focaltech_i2c.c oracolo/stock.elf \
 *       --intervallo 0xffffff8008a802ac:0xffffff8008a807e0 \
 *       --intervallo 0xffffff8008a7b600:0xffffff8008a80b54
 *
 * IL SECONDO INTERVALLO E' TUTTO IL BLOCCO, e serve per SEI citazioni che
 * stanno FUORI da questa unita' apposta: sono i due siti di chiamata di
 * fts_i2c_hid2std -- "94000ca4 bl"@0xffffff8008a7d3d8 in
 * fts_fwupg_get_boot_state (unita' B) e "9400041d bl"@0xffffff8008a7f5f4 in
 * tpd_probe (unita' C), con le "52954aa8 mov" che li seguono.  Sono la prova
 * che quei chiamanti IGNORANO il valore di ritorno, e quella prova per forza
 * non e' dentro l'unita' che definisce la funzione.  Con il solo primo
 * intervallo lo strumento le segnala ASSENTE, e ha ragione: e' la forma
 * corretta di dichiararle, non un'eccezione.
 * Questa unita' non ha `.init.text` ne' `.exit.text`.
 * Nessuna `--controfattuale`: questo file non cita nessuna istruzione che NON
 * debba stare nel binario.
 *
 * ESITO ATTESO, al 2026-08-21:
 *   citazioni di istruzione trovate nel sorgente: 98 (98 a codifica, 0 ad indirizzo)
 *   confermate: 98   assenti: 0   mnemonico diverso: 0   controfattuali: 0
 *   operandi -- registro diverso: 0   immediato diverso: 0   entrambi/non confrontabili: 0
 * e per le citazioni di letterale:
 *   letterali: 12   citati: 13   verificati: 12   probanti: 12   deboli: 0
 *   di cui verificate come messaggio assemblato dalla macro di log: 6
 *   di cui verificate come nome di funzione da __func__: 4
 *   soglia imposta: 9 citati richiesti (12 letterali - 3 eccezioni)
 * piu' UNA `NON_ANCORATA`, dichiarata in coda a questo file.
 *
 * La misura di dimensione, dal `.o` VERO (non da misuraisolata.py):
 *
 *   make -C albero-ft8719 O=out-ft8719 ARCH=arm64 -j6 CC=clang HOSTCC=clang \
 *        CLANG_TRIPLE=aarch64-linux-gnu- CROSS_COMPILE=aarch64-linux-android- \
 *        KCFLAGS=-Werror \
 *        drivers/input/touchscreen/mediatek/focaltech_ft8719/
 *   aarch64-linux-android-nm --print-size \
 *        out-ft8719/drivers/input/touchscreen/mediatek/focaltech_ft8719/focaltech_i2c.o
 *
 * ===========================================================================
 * 3. LA FORMA DELLE CITAZIONI
 * ===========================================================================
 * istruzione:  "<codifica8hex> <mnemonico>"@0xINDIRIZZO
 * letterale:   "testo"@0xINDIRIZZO
 * Sono le due forme che verificaistruzioni.py e verificacitazioni.py sanno
 * confrontare col binario.  Un numero di riga dentro un listato effimero NON
 * e' una prova (classe B7).  Le codifiche qui sotto NON sono trascritte a
 * mano: escono da uno script che legge il disassemblato e stampa la coppia
 * (codifica, indirizzo) insieme (classe B8, la citazione che porta la
 * codifica giusta all'indirizzo sbagliato).
 *
 * ATTENZIONE nel leggere gli scarti: il kernel di fabbrica e' CONFIG_RELOCATABLE=y
 * e i nostri indirizzi non sono i suoi; ogni `adrp`/`add #lo12` che
 * materializza un simbolo ha per forza una codifica diversa dalla nostra.  La
 * misura che discrimina e' la DIMENSIONE.
 * E il compilatore non e' lo stesso: fabbrica clang 9.0.3 (r353983c), noi
 * clang 11.0.1 (r383902).
 *
 * ===========================================================================
 * 4. LE DIVERGENZE APERTE
 * ===========================================================================
 * Elencate e attribuite nel blocco in CODA a questo file.
 *
 * ===========================================================================
 * 5. CIO' CHE NON E' SCRITTO QUI, E PERCHE'
 * ===========================================================================
 * Le altre cinque unita' del blocco.  L'unita' C sta gia' in
 * focaltech_core.c (16 funzioni); restano A (22 funzioni, il canale /proc e
 * i nodi sysfs), B (17, l'aggiornamento del firmware), D (3, glove/cover/
 * charger) e F (3 piu' il blob del pramboot, l'aggiornamento FT8719).
 * Le funzioni che questo file CHIAMA e che non stanno qui sono API del
 * kernel: `mutex_lock`, `mutex_unlock`, `i2c_transfer`, `msleep`, `printk`.
 * Nessuna dipendenza di confine, quindi questa unita' e' completa.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mutex.h>

/* ==========================================================================
 * LE MACRO DI STAMPA -- lette dalle stringhe, non supposte
 * ==========================================================================
 * Il binario porta i formati interi, prefisso e livello KERN compresi:
 *   "[FTS]%s: Enter\n"@0xffffff800924f634                 (nessun livello)
 *   "[FTS]%s: Exit(%d)\n"@0xffffff800924f65a              (nessun livello)
 *   "[FTS]hidi2c change to stdi2c successful\n"@0xffffff8009250048
 *   "\x013[FTS][Error]hid2std cmd write fail\n"@0xffffff800924fffd  (KERN_ERR)
 * cioe' due macro distinte: una SENZA livello con prefisso "[FTS]", una con
 * KERN_ERR e prefisso "[FTS][Error]".  Questa unita' non usa FTS_INFO.
 *
 * x0 e' il FORMATO e x1 e' il __func__ -- classe B1, il difetto preso al
 * contrario da un lotto precedente:
 *   "9118d000 add"@0xffffff8008a80768   x0 = 0x924f000+0x634 = "[FTS]%s: Enter\n"
 *   "91027673 add"@0xffffff8008a80764   x19 = 0x9250000+0x9d = "fts_i2c_init"
 *   "aa1303e1 mov"@0xffffff8008a8076c   x1 = x19
 * e il __func__ resta in x19 attraverso le DUE printk di fts_i2c_init, che e'
 * il motivo per cui quella funzione salva un registro callee-saved.
 */
#define FTS_DEBUG(fmt, args...)		printk("[FTS]" fmt "\n", ##args)
#define FTS_ERROR(fmt, args...)		printk(KERN_ERR "[FTS][Error]" fmt "\n", ##args)
#define FTS_FUNC_ENTER()		printk("[FTS]%s: Enter\n", __func__)
#define FTS_FUNC_EXIT()			printk("[FTS]%s: Exit(%d)\n", __func__, __LINE__)

/*
 * IL NUMERO DI TENTATIVI.  Non e' scritto in nessun immediato: e' il numero
 * di copie del corpo che clang ha lasciato dopo lo srotolamento completo.
 * In fts_i2c_read, ramo a due messaggi, le `bl i2c_transfer` sono TRE e non
 * hanno arco all'indietro:
 *   "94002b32 bl"@0xffffff8008a80338
 *   "94002b28 bl"@0xffffff8008a80360
 *   "94002b1e bl"@0xffffff8008a80388
 * ognuna seguita da "36f808a0 tbz"@0xffffff8008a80340 /
 * "36f80760 tbz"@0xffffff8008a80368 / "36f80620 tbz"@0xffffff8008a80390, cioe'
 * `if (ret >= 0) break;`.  Un ciclo a conteggio costante 3 srotolato: nessun
 * registro porta un contatore e la terza copia cade nel seguito invece di
 * tornare indietro.
 */
#define I2C_RETRY_NUMBER		3

/*
 * IL MUTEX DEL BUS, a 0xffffff800998cbe0.
 * E' l'argomento in x0 di mutex_lock e mutex_unlock nelle due funzioni
 * grandi: "912f8000 add"@0xffffff8008a802dc mette in x0 0x998c000 + 0xbe0.
 * Occupa 32 byte esatti (0x998cbe0..0x998cc00, dove comincia il blob del
 * pramboot dell'unita' F), cioe' e' una `struct mutex` SENZA i campi di
 * CONFIG_DEBUG_MUTEXES e senza `struct lockdep_map`: se ci fossero, il nome
 * della variabile sarebbe nel binario.  Non c'e'.
 *
 * IL NOME E' UNA SCELTA, NON UNA MISURA (regola 5): il binario non lo
 * contiene, quindi la variabile si chiama come il suo indirizzo.  Anche
 * `static` e' una scelta: un `struct mutex` e' comunque preso per indirizzo,
 * quindi il collegamento non cambia una sola istruzione.  Cio' che E'
 * misurato e' che nessuna funzione fuori da questa unita' lo tocca
 * (scout-ft8719/tocca.py su tutto il kernel).
 */
static DEFINE_MUTEX(fts_g_998cbe0);

/*
 * ===========================================================================
 * fts_i2c_read @ 0xffffff8008a802ac, 488 byte
 * ===========================================================================
 * FIRMA, letta dai registri d'ingresso:
 *   x0 client   -- "b4000680 cbz"@0xffffff8008a802d0 lo confronta con NULL
 *   x1 writebuf -- "aa0103f6 mov"@0xffffff8008a802ec  -> x22, poi
 *                  "f9000bf6 str"@0xffffff8008a80314 in msgs[0].buf
 *   w2 writelen -- "2a0203f7 mov"@0xffffff8008a802e8  -> w23, confrontato con
 *                  1 da "710006ff cmp"@0xffffff8008a802fc e versato con
 *                  "79001bf7 strh"@0xffffff8008a80310 in msgs[0].len
 *   x3 readbuf  -- "aa0303f4 mov"@0xffffff8008a802e4  -> x20
 *   w4 readlen  -- "2a0403f5 mov"@0xffffff8008a802e0  -> w21
 * writelen e readlen sono `int` e non `u32`: il confronto e'
 * "710006bf cmp"@0xffffff8008a802f4 + "5400062b b.lt"@0xffffff8008a802f8,
 * cioe' un `b.lt` CON SEGNO.  Con un unsigned clang avrebbe emesso `cbz`
 * (== 0) oppure `b.lo`.  Classi A3/A4.
 *
 * IL CAMPO +2 e IL CAMPO +24 di i2c_client.  "79400668 ldrh"@0xffffff8008a80304
 * legge due byte a client+2 e li versa in msgs[].addr; "f9400e60 ldr"
 * @0xffffff8008a8032c legge otto byte a client+24 e li passa in x0 a
 * i2c_transfer.  Nel `struct i2c_client` dell'albero sono `addr` e `adapter`,
 * ed e' un fatto dell'header del kernel, non una scelta di questo file.
 *
 * LA `ldr x0,[x19,#24]` STA DENTRO OGNI COPIA DEL CICLO
 * ("f9400e60 ldr"@0xffffff8008a8032c, @0xffffff8008a80354, @0xffffff8008a8037c):
 * il sorgente RILEGGE `client->adapter` a ogni giro invece di estrarlo in un
 * locale prima del for.  E' la classe A2 al contrario -- qui il binario
 * rilegge, quindi il sorgente non estrae.
 *
 * IL MESSAGGIO DEL RAMO A DUE MESSAGGI DICE "(write)", NON "(read)".  E' un
 * difetto della fabbrica e si riproduce (regola 7):
 *   ramo writelen>0 -> "\x013[FTS][Error][IIC]: i2c_transfer(write) error, ret=%d!!\n"@0xffffff800924ff7c
 *   ramo writelen<=0 -> "\x013[FTS][Error][IIC]: i2c_transfer(read) error, ret=%d!!\n"@0xffffff800924ffb6
 * Le due `adrp`+`add` sono distinte e stanno su rami distinti
 * ("913df000 add"@0xffffff8008a80348 contro
 *  "913ed800 add"@0xffffff8008a803f8), quindi non e' una fusione del linker.
 *
 * IL MESSAGGIO DEL RAMO NULL PORTA IL __func__:
 *   "\x013[FTS][Error][IIC][%s]i2c_client==NULL!\n"@0xffffff800924ff45
 * e' il formato (x0), e "fts_i2c_read"@0xffffff800924ff6f e' il secondo
 * argomento (x1):
 *   "913d1400 add"@0xffffff8008a803a8   x0 = 0x924f000 + 0xf45, il FORMATO
 *   "913dbc21 add"@0xffffff8008a803ac   x1 = 0x924f000 + 0xf6f, il __func__
 * L'ordine e' quello, e non l'inverso: e' la classe B1, il difetto che un
 * lotto precedente aveva preso al contrario.  Che il __func__ combaci col
 * nome della funzione prova che questa NON e' incorporata da nessuna parte
 * (classe B5).
 *
 * IL RAMO NULL NON SBLOCCA IL MUTEX, e non e' un difetto: il `cbz`
 * @0xffffff8008a802d0 precede la `bl mutex_lock`@0xffffff8008a802f0, quindi
 * il lock non e' mai stato preso.  "128002b4 mov"@0xffffff8008a803b4 mette
 * -22 = -EINVAL e "1400002a b"@0xffffff8008a803b8 salta OLTRE la
 * mutex_unlock, direttamente all'epilogo.
 */
int fts_i2c_read(struct i2c_client *client, char *writebuf, int writelen,
		 char *readbuf, int readlen)
{
	int ret = 0;
	int i = 0;

	if (client == NULL) {
		FTS_ERROR("[IIC][%s]i2c_client==NULL!", __func__);
		return -EINVAL;
	}

	mutex_lock(&fts_g_998cbe0);
	if (readlen > 0) {
		if (writelen > 0) {
			struct i2c_msg msgs[] = {
				{
					.addr = client->addr,
					.flags = 0,
					.len = writelen,
					.buf = writebuf,
				},
				{
					.addr = client->addr,
					.flags = I2C_M_RD,
					.len = readlen,
					.buf = readbuf,
				},
			};
			for (i = 0; i < I2C_RETRY_NUMBER; i++) {
				ret = i2c_transfer(client->adapter, msgs, 2);
				if (ret < 0)
					FTS_ERROR("[IIC]: i2c_transfer(write) error, ret=%d!!", ret);
				else
					break;
			}
		} else {
			struct i2c_msg msgs[] = {
				{
					.addr = client->addr,
					.flags = I2C_M_RD,
					.len = readlen,
					.buf = readbuf,
				},
			};
			for (i = 0; i < I2C_RETRY_NUMBER; i++) {
				ret = i2c_transfer(client->adapter, msgs, 1);
				if (ret < 0)
					FTS_ERROR("[IIC]: i2c_transfer(read) error, ret=%d!!", ret);
				else
					break;
			}
		}
	}
	mutex_unlock(&fts_g_998cbe0);

	return ret;
}

/*
 * ===========================================================================
 * fts_i2c_write @ 0xffffff8008a80494, 304 byte
 * ===========================================================================
 * Stessa struttura di fts_i2c_read senza il ramo di lettura:
 *   "b40005a0 cbz"@0xffffff8008a804b4    client == NULL
 *   "710006bf cmp"@0xffffff8008a804d0 + "5400058b b.lt"@0xffffff8008a804d4
 *                                        writelen < 1  (con segno)
 *   "790017ff strh"@0xffffff8008a804dc    msgs[0].flags = 0
 *   "79001bf5 strh"@0xffffff8008a804e0    msgs[0].len = writelen
 *   "f9000bf3 str"@0xffffff8008a804e4     msgs[0].buf = writebuf
 *   "320003e2 orr"@0xffffff8008a804f4     w2 = 1, un solo messaggio
 * e le TRE `bl i2c_transfer` dello srotolamento:
 *   "94002ac2 bl"@0xffffff8008a804f8
 *   "94002ab8 bl"@0xffffff8008a80520
 *   "94002aae bl"@0xffffff8008a80548
 * Il __func__ del ramo NULL e' "fts_i2c_write"@0xffffff800924ffef, cioe' la
 * funzione NON e' incorporata da nessuna parte in questa unita' (classe B5:
 * se il __func__ non combaciasse col nome, lo sarebbe).
 */
int fts_i2c_write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret = 0;
	int i = 0;

	if (client == NULL) {
		FTS_ERROR("[IIC][%s]i2c_client==NULL!", __func__);
		return -EINVAL;
	}

	mutex_lock(&fts_g_998cbe0);
	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
				.addr = client->addr,
				.flags = 0,
				.len = writelen,
				.buf = writebuf,
			},
		};
		for (i = 0; i < I2C_RETRY_NUMBER; i++) {
			ret = i2c_transfer(client->adapter, msgs, 1);
			if (ret < 0)
				FTS_ERROR("[IIC]: i2c_transfer(write) error, ret=%d!!", ret);
			else
				break;
		}
	}
	mutex_unlock(&fts_g_998cbe0);

	return ret;
}

/*
 * ===========================================================================
 * fts_i2c_write_reg @ 0xffffff8008a805c4, 80 byte
 * ===========================================================================
 * Due byte sullo stack e una fts_i2c_write:
 *   "390013e1 strb"@0xffffff8008a805dc   buf[0] = w1   (UN byte: regaddr e' u8)
 *   "390017e2 strb"@0xffffff8008a805e0   buf[1] = w2   (UN byte: regvalue e' u8)
 *   "910013e1 add"@0xffffff8008a805e4    x1 = sp+4 = buf
 *   "321f03e2 orr"@0xffffff8008a805e8    w2 = 2 = sizeof(buf)
 *   "97ffffaa bl"@0xffffff8008a805ec     fts_i2c_write
 * La larghezza dei due parametri e' MISURATA dalle due `strb` (classe A4):
 * con un `int` clang avrebbe scritto comunque `strb` per riempire un buffer
 * di char, ma qui non c'e' nessun troncamento esplicito ne' `uxtb` prima,
 * e i due valori arrivano dai registri d'ingresso senza passare da nessuna
 * maschera -- il che e' compatibile con `u8` e basta.
 * Il valore di ritorno di fts_i2c_write NON viene ritoccato: la `bl` e'
 * seguita solo dal controllo del canarino e dall'epilogo, quindi w0 esce
 * com'e'.  E' un `return fts_i2c_write(...)`.
 */
int fts_i2c_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue)
{
	u8 buf[2] = { 0 };

	buf[0] = regaddr;
	buf[1] = regvalue;

	return fts_i2c_write(client, buf, sizeof(buf));
}

/*
 * ===========================================================================
 * fts_i2c_read_reg @ 0xffffff8008a80614, 84 byte
 * ===========================================================================
 *   "aa0203e3 mov"@0xffffff8008a80628   x3 = x2, cioe' il TERZO parametro
 *                                       diventa il readbuf: e' un puntatore
 *   "320003e2 orr"@0xffffff8008a8062c   w2 = 1  (writelen)
 *   "320003e4 orr"@0xffffff8008a80630   w4 = 1  (readlen)
 *   "390013e1 strb"@0xffffff8008a80638  UN byte a sp+4: regaddr e' u8
 *   "910013e1 add"@0xffffff8008a8063c   x1 = sp+4
 *   "97ffff1b bl"@0xffffff8008a80640    fts_i2c_read
 * Il parametro `regaddr` finisce sullo stack perche' se ne prende
 * l'indirizzo: e' `&regaddr`, non un buffer separato -- non c'e' nessun
 * secondo store, e il byte scritto e' esattamente w1 senza passare da un
 * temporaneo.
 */
int fts_i2c_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue)
{
	return fts_i2c_read(client, &regaddr, 1, regvalue, 1);
}

/*
 * ===========================================================================
 * fts_i2c_hid2std @ 0xffffff8008a80668, 232 byte
 * ===========================================================================
 * I TRE BYTE DEL COMANDO, letti dagli immediati e non supposti:
 *   "52955d6a mov"@0xffffff8008a80684   w10 = 0xaaeb
 *   "79000bea strh"@0xffffff8008a8069c  strh w10, [sp,#4]  -> buf[0]=0xeb, buf[1]=0xaa
 *   "52800129 mov"@0xffffff8008a80680   w9 = 9
 *   "39001be9 strb"@0xffffff8008a80698  strb w9, [sp,#6]   -> buf[2]=0x09
 *   "320007e2 orr"@0xffffff8008a8068c   w2 = 3, la lunghezza
 * L'ordine dei byte dentro la halfword e' little-endian: 0xaaeb versato a
 * sp+4 mette 0xeb a sp+4 e 0xaa a sp+5.
 *
 * L'AZZERAMENTO PRIMA DELLA LETTURA:
 *   "79000bff strh"@0xffffff8008a806c8  buf[0] = buf[1] = 0
 *   "39001bff strb"@0xffffff8008a806c4  buf[2] = 0
 * e la lettura senza scrittura:
 *   "aa1f03e1 mov"@0xffffff8008a806bc   x1 = NULL   (writebuf)
 *   "2a1f03e2 mov"@0xffffff8008a806c0   w2 = 0      (writelen)
 *   "910013e3 add"@0xffffff8008a806b0   x3 = sp+4   (readbuf)
 *   "320007e4 orr"@0xffffff8008a806b4   w4 = 3      (readlen)
 * cioe' proprio il ramo `writelen <= 0` di fts_i2c_read.
 *
 * I QUATTRO MESSAGGI, uno per ramo, letti per intero dal binario:
 *   "\x013[FTS][Error]hid2std cmd write fail\n"@0xffffff800924fffd
 *   "\x013[FTS][Error]hid2std cmd read fail\n"@0xffffff8009250023
 *   "[FTS]hidi2c change to stdi2c successful\n"@0xffffff8009250048
 *   "\x013[FTS][Error]hidi2c change to stdi2c fail\n"@0xffffff8009250071
 * Il terzo comincia con '[' e gli altri col SOH: e' la prova che nello stesso
 * file convivono DUE macro di stampa, una senza livello KERN e una con.
 *
 * I TRE CONFRONTI DELLA RISPOSTA:
 *   "7103ad1f cmp"@0xffffff8008a806d8   buf[0] == 0xeb
 *   "7102a91f cmp"@0xffffff8008a806e4   buf[1] == 0xaa
 *   "7100211f cmp"@0xffffff8008a806f0   buf[2] == 0x08
 * Il terzo byte atteso e' 0x08 e non 0x09: il comando manda 9, la risposta
 * buona porta 8.  Letto, non dedotto.
 *
 * IL VALORE DI RITORNO NON C'E', ED E' MISURATO.  L'epilogo
 * (0xffffff8008a80728..0xffffff8008a80748) e' canarino + `ldp` + `ldr` +
 * `add sp` + `ret`, e NON contiene nessuna scrittura di w0: l'ultima cosa che
 * tocca w0 e' "97dacb6c bl"@0xffffff8008a80724, cioe' printk.  Di piu':
 * `ret` non sopravvive mai in un registro callee-saved -- dopo
 * "97ffff7d bl"@0xffffff8008a806a0 il risultato e' testato direttamente con
 * "37f80300 tbnz"@0xffffff8008a806a4, e dopo "97fffef8 bl"@0xffffff8008a806cc
 * con "37f80200 tbnz"@0xffffff8008a806d0.  Se il sorgente finisse con
 * `return ret;` quel valore sarebbe vivo.
 *
 * E I DUE CHIAMANTI NEL RESTO DEL BLOCCO LO IGNORANO:
 *   "94000ca4 bl"@0xffffff8008a7d3d8  (fts_fwupg_get_boot_state) seguito da
 *      "52954aa8 mov"@0xffffff8008a7d3dc, che sovrascrive w8 e non legge w0
 *   "9400041d bl"@0xffffff8008a7f5f4  (tpd_probe) seguito da
 *      "52954aa8 mov"@0xffffff8008a7f5f8, idem
 *
 * QUINDI DUE SORGENTI DIVERSI DANNO LO STESSO BINARIO, e l'ho MISURATO
 * invece di sceglierlo a occhio.  Tre varianti, stesso albero, stesso clang:
 *
 *   A  `int` senza `return` finale (il difetto della fabbrica riprodotto)
 *   B  `int` con `return ret;` aggiunto
 *   C  `void`
 *
 *   variante | fts_i2c_hid2std nel .o | -Werror
 *   ---------|-----------------------|---------------------------------------
 *   A        | 0xe4 = 228 byte       | ERRORE: non-void function does not
 *            |                       | return a value [-Wreturn-type]
 *   B        | 0xf0 = 240 byte       | pulito, ma +8 byte sulla fabbrica
 *   C        | 0xe4 = 228 byte       | pulito
 *
 *   $ aarch64-linux-android-objdump -d varC.o | tail -n +3 > C.txt
 *   $ aarch64-linux-android-objdump -d varA.o | tail -n +3 > A.txt
 *   $ diff -q C.txt A.txt && echo IDENTICO
 *   IDENTICO
 * A e C hanno il disassemblato IDENTICO riga per riga; i due `.o` differiscono
 * solo in `.debug_abbrev`/`.debug_info` (il tipo di ritorno nel DWARF) e negli
 * scorrimenti che ne conseguono.  Il binario NON discrimina `void` da un `int`
 * a cui manca il `return`.
 *
 * SCELTO `void`, e la ragione e' che non costa niente: produce il codice della
 * fabbrica byte per byte E passa `-Werror`.  La variante B, che sarebbe la
 * lettura "corretta", costerebbe 8 byte che nel binario non ci sono, cioe'
 * sarebbe codice aggiunto (classe B6).
 * CONSEGUENZA DA DICHIARARE: focaltech_core.c (unita' C, scritta da un altro
 * lotto) dichiara `int fts_i2c_hid2std(struct i2c_client *client);`.  E' una
 * discordanza di tipo fra due unita' di traduzione, che il C non segnala.
 * NON ho toccato quel file: e' il delta n.1 in coda.
 */
void fts_i2c_hid2std(struct i2c_client *client)
{
	int ret = 0;
	u8 buf[3] = { 0xeb, 0xaa, 0x09 };

	ret = fts_i2c_write(client, buf, 3);
	if (ret < 0) {
		FTS_ERROR("hid2std cmd write fail");
	} else {
		msleep(10);
		buf[0] = buf[1] = buf[2] = 0;
		ret = fts_i2c_read(client, NULL, 0, buf, 3);
		if (ret < 0) {
			FTS_ERROR("hid2std cmd read fail");
		} else if ((buf[0] == 0xeb) && (buf[1] == 0xaa) && (buf[2] == 0x08)) {
			FTS_DEBUG("hidi2c change to stdi2c successful");
		} else {
			FTS_ERROR("hidi2c change to stdi2c fail");
		}
	}
}

/*
 * ===========================================================================
 * fts_i2c_init @ 0xffffff8008a80750, 72 byte
 * fts_i2c_exit @ 0xffffff8008a80798, 72 byte
 * ===========================================================================
 * Due funzioni che stampano soltanto.  Il numero di riga di FTS_FUNC_EXIT e'
 * l'unico contenuto misurabile oltre le due printk:
 *   "52802e42 mov"@0xffffff8008a8077c   w2 = 0x172 = 370
 *   "52803042 mov"@0xffffff8008a807c4   w2 = 0x182 = 386
 * e i due __func__, che combaciano col nome della funzione che li contiene
 * (quindi nessuna delle due e' incorporata -- classe B5):
 *   "fts_i2c_init"@0xffffff800925009d
 *   "fts_i2c_exit"@0xffffff80092500aa
 * e "2a1f03e0 mov"@0xffffff8008a8078c / @0xffffff8008a807d4 mettono w0 = 0,
 * cioe' `return 0` -- anche in fts_i2c_exit, che quindi e' `int` e non `void`.
 *
 * I DUE NUMERI DI RIGA NON SI RIPRODUCONO in questo file, e non li ho
 * inseguiti: sono la posizione di quelle righe nel file DI FABBRICA, che ha
 * una intestazione e un ordine che il binario non porta.  Costo: ZERO byte
 * (`mov w2, #imm` e' una istruzione qualunque sia l'immediato).  Vedi
 * DIVERGENZA 3.
 */
int fts_i2c_init(void)
{
	FTS_FUNC_ENTER();
	FTS_FUNC_EXIT();

	return 0;
}

int fts_i2c_exit(void)
{
	FTS_FUNC_ENTER();
	FTS_FUNC_EXIT();

	return 0;
}

/*
 * ===========================================================================
 * LA MISURA, E L'ATTRIBUZIONE DI OGNI DIVERGENZA
 * ===========================================================================
 * Dal `.o` VERO, non da misuraisolata.py:
 *
 *   $ aarch64-linux-android-nm --print-size \
 *       out-ft8719/.../focaltech_ft8719/focaltech_i2c.o | grep -iE ' [tT] '
 *   00000000000004e8 0000000000000048 T fts_i2c_exit
 *   00000000000003bc 00000000000000e4 T fts_i2c_hid2std
 *   00000000000004a0 0000000000000048 T fts_i2c_init
 *   0000000000000000 00000000000001e8 T fts_i2c_read
 *   0000000000000368 0000000000000054 T fts_i2c_read_reg
 *   00000000000001e8 0000000000000130 T fts_i2c_write
 *   0000000000000318 0000000000000050 T fts_i2c_write_reg
 *
 *   funzione            fabbrica  nostro  scarto
 *   ------------------  --------  ------  ------
 *   fts_i2c_read             488     488       0
 *   fts_i2c_write            304     304       0
 *   fts_i2c_write_reg         80      80       0
 *   fts_i2c_read_reg          84      84       0
 *   fts_i2c_hid2std          232     228      -4
 *   fts_i2c_init              72      72       0
 *   fts_i2c_exit              72      72       0
 *   ------------------  --------  ------  ------
 *   totale                  1332    1328      -4
 *
 * SEI FUNZIONI SU SETTE SONO IDENTICHE.  Sei su sette e' una numerosita'
 * piccola e la misura NON DISCRIMINA in nessuna direzione: l'intervallo di
 * Clopper-Pearson al 95% e' [42,1% ; 99,6%] e contiene sia il 77,10% del ramo
 * sia quasi tutto il resto.  L'avvertenza viene PRIMA della percentuale
 * (classe C4).  Il numero e' 85,7%.
 *
 * DIVERGENZA 1 -- fts_i2c_hid2std, -4 byte, UNA istruzione, causa TROVATA.
 *
 *   La fabbrica confronta i tre byte della risposta con tre `cmp` e tre
 *   `b.ne` che convergono su un blocco di fallimento separato:
 *     "394013e8 ldrb"@0xffffff8008a806d4  +  "7103ad1f cmp"@0xffffff8008a806d8
 *                                         +  "54000201 b.ne"@0xffffff8008a806dc
 *     "394017e8 ldrb"@0xffffff8008a806e0  +  "7102a91f cmp"@0xffffff8008a806e4
 *                                         +  "540001a1 b.ne"@0xffffff8008a806e8
 *     "39401be8 ldrb"@0xffffff8008a806ec  +  "7100211f cmp"@0xffffff8008a806f0
 *                                         +  "54000141 b.ne"@0xffffff8008a806f4
 *   piu' "90003e80 adrp"@0xffffff8008a806f8 + "91012000 add"@0xffffff8008a806fc
 *   + "14000009 b"@0xffffff8008a80700 per il successo, e
 *   "90003e80 adrp"@0xffffff8008a8071c + "9101c400 add"@0xffffff8008a80720 per
 *   il fallimento: VENTI istruzioni, da 0xffffff8008a806d4 a 0xffffff8008a80720.
 *
 *   Il nostro clang usa `ccmp` e sceglie la stringa con una `csel`:
 *     3f0:  ldrb w10,[sp,#6] / ldrb w9,[sp,#5] / ldrb w8,[sp,#4]
 *           mov w11,#0xaa / cmp w10,#0x8 / mov w12,#0xeb
 *           ccmp w9,w11,#0x0,eq / adrp x13 / ccmp w8,w12,#0x0,eq / adrp x8
 *           add x13 / add x8 / csel x0,x8,x13,eq / b
 *           adrp x0 / add x0 / b / adrp x0 / add x0
 *   DICIANNOVE istruzioni per lo stesso lavoro: il blocco di fallimento non
 *   esiste piu' perche' il suo indirizzo entra nella `csel`.
 *
 *   ISTOGRAMMA DEI MNEMONICI, fabbrica contro nostro, sulla sola regione che
 *   cambia (fabbrica 0xffffff8008a806d4..0xffffff8008a80720 = 20 istruzioni;
 *   nostro 0x428..0x470 = 19):
 *
 *     mnemonico   fabbrica   nostro   delta
 *     ---------   --------   ------   -----
 *     ldrb               3        3       0
 *     cmp                3        1      -2
 *     ccmp               0        2      +2
 *     mov                0        2      +2
 *     b.ne               3        0      -3
 *     csel               0        1      +1
 *     adrp               4        3      -1
 *     add                4        3      -1
 *     b                  3        2      -1
 *     ---------   --------   ------   -----
 *     totale            20       19      -1
 *
 *   -1 istruzione x 4 byte = -4 byte, cioe' TUTTO lo scarto.  Il resto della
 *   funzione (le 38 istruzioni di prologo, comando, msleep, lettura, printk
 *   ed epilogo) combacia una a una.
 *
 *   CAUSA: `ccmp` + `csel` e' una trasformazione che il nostro clang 11.0.1
 *   (r383902) applica e quello della fabbrica, clang 9.0.3 (r353983c), no.
 *   NON e' una differenza del sorgente: qualunque scrittura della catena
 *   `(a==0xeb) && (b==0xaa) && (c==0x08)` che dia i tre `ldrb` nell'ordine
 *   giusto passa per la stessa trasformazione.  E' la stessa classe di
 *   divergenza che il progetto ha gia' registrato: sullo STESSO sorgente ALPS
 *   il cambio di compilatore vale 64/89 contro 87/89.
 *
 * DIVERGENZA 2 -- il tipo di ritorno di fts_i2c_hid2std, ZERO byte.
 *   Il binario non discrimina `void` da `int` senza `return` (misurato: i due
 *   disassemblati sono identici, vedi il commento della funzione).  La scelta
 *   di `void` e' dichiarata come scelta, non come misura.  Costo: 0 byte.
 *
 * DIVERGENZA 3 -- i numeri di riga di FTS_FUNC_EXIT, ZERO byte.
 *   La fabbrica stampa 370 in fts_i2c_init e 386 in fts_i2c_exit
 *   ("52802e42 mov"@0xffffff8008a8077c, "52803042 mov"@0xffffff8008a807c4).
 *   In questo file le due `FTS_FUNC_EXIT()` cadono ad altre righe, perche' il
 *   file di fabbrica ha un'intestazione e un ordine che il binario non porta.
 *   Costo: 0 byte -- `mov w2, #imm` occupa 4 byte per ogni immediato a 16 bit.
 *   Dichiarata e non inseguita: inseguirla vorrebbe dire mettere righe vuote
 *   di riempimento, cioe' scrivere il file per far tornare un numero
 *   (regola 3).
 *
 * NESSUN RESIDUO INSPIEGATO.  Lo scarto totale e' -4 byte, e i -4 byte sono
 * l'unica istruzione della DIVERGENZA 1.
 *
 * ===========================================================================
 * DELTA DI HEADER / DI FILE CONDIVISO -- DESCRITTO, NON FATTO
 * ===========================================================================
 * 1. focaltech_core.c (unita' C, di un altro lotto) contiene
 *      int fts_i2c_hid2std(struct i2c_client *client);        [riga 482]
 *    mentre qui la definizione e' `void`.  Sono due unita' di traduzione
 *    diverse e il C non segnala la discordanza, ma resta una discordanza.
 *    Il delta: cambiare quella riga in
 *      void fts_i2c_hid2std(struct i2c_client *client);
 *    NON l'ho fatto -- quel file e' l'artefatto misurato di un altro lotto, e
 *    la riga sta in mezzo alle sue dichiarazioni di confine.  I due chiamanti
 *    di fabbrica ignorano il risultato ("52954aa8 mov"@0xffffff8008a7d3dc e
 *    "52954aa8 mov"@0xffffff8008a7f5f8 sovrascrivono senza leggere w0), quindi
 *    il cambio non dovrebbe muovere un byte di quel file: va MISURATO dal
 *    lotto che lo fa, non dato per buono.
 *
 * 2. Il Makefile del padre (drivers/input/touchscreen/mediatek/Makefile) non
 *    ha ancora la riga
 *      obj-$(CONFIG_WTK_FT8719_E977) += focaltech_ft8719/
 *    e il suo Kconfig non ha il `source` della directory.  E' lo stesso delta
 *    n.3 gia' registrato dal cappello di focaltech_core.c: lo ripeto qui
 *    perche' una correzione che non raggiunge tutte le copie e' la classe C1.
 *
 * ===========================================================================
 * LE CITAZIONI CHE `verificacitazioni.py` SEGNA `NON_ANCORATA`, E PERCHE'
 * ===========================================================================
 * Una sola classe in questo file, gia' descritta in coda a focaltech_core.c:
 * i messaggi della macro SENZA livello KERN.
 *   "[FTS]hidi2c change to stdi2c successful\n"@0xffffff8009250048
 *   "[FTS]%s: Enter\n"@0xffffff800924f634
 *   "[FTS]%s: Exit(%d)\n"@0xffffff800924f65a
 * Lo strumento riconosce un messaggio assemblato dalla testa `\x01` + cifra
 * di livello (RE_TESTA_ASSEMBLATA); FTS_DEBUG/FTS_FUNC_ENTER/FTS_FUNC_EXIT
 * non emettono nessun livello -- e il binario lo prova da solo, perche'
 * "[FTS]hidi2c..." comincia con '[' mentre
 * "\x013[FTS][Error]hidi2c change to stdi2c fail\n"@0xffffff8009250071
 * comincia col SOH.  Sono due macro diverse nello stesso file.
 * E' un DELTA DI STRUMENTO e non l'ho fatto: verificacitazioni.py e'
 * condiviso.
 */
