// SPDX-License-Identifier: GPL-2.0
/*
 * Accelerometro MiraMEMS mir3da (DA218-B) del Doogee S88 Pro — l'unita' di
 * traduzione "core", ricostruita leggendo il kernel di fabbrica
 * disassemblato.  Nessuna riga viene da un altro telefono: il sorgente
 * mir3da non esiste ne' nell'albero ALPS ne' nell'albero .186 dell'altro
 * progetto.
 *
 * ATTENZIONE SUL NOME DEL FILE.  "mir3da_core.c" e' la convenzione con cui
 * mir3da_cust.c e mir3da_core.h chiamano questa seconda unita' di
 * traduzione, NON una misura: la stringa "mir3da_core" non esiste in
 * nessun punto di stock.elf (revisione indipendente,
 * docs/bringup/rapporti/revisione-mir3da.md, rilievo R6b).  Che siano DUE
 * unita' e non una lo provano il linkage (le 36 di mir3da_cust.c sono `t`
 * tranne le quattro che il core prende per indirizzo; queste sono quasi
 * tutte `T`), la mancata incorporazione, e il canale di log: il core non
 * chiama MAI printk, logga attraverso general_op_s.myprintk a +64, ed e'
 * per questo che i suoi letterali NON portano il prefisso di livello
 * "\x013" che portano tutti i messaggi di mir3da_cust.c.
 *
 *
 * 1. QUANTE FUNZIONI CI SONO QUI, E PERCHE' PROPRIO QUESTE
 * -------------------------------------------------------
 *
 * Il blocco contiguo 0xffffff800878a0a8..0xffffff800878c3d8 (da squareRoot
 * a NSA_get_reg_data) ha 29 funzioni.  Qui ne sono scritte 19.  Il criterio
 * non e' "le piu' facili": e' la CHIUSURA TRANSITIVA di cio' che serve a
 * far riuscire il link, misurata e non stimata.
 *
 * L'esperimento del 2026-08-16 (docs/bringup/rapporti/
 * rapporto-innesto-mir3da-solo-cust.md) aveva misurato che mir3da_cust.o
 * da solo lascia 9 simboli non risolti — le 9 dichiarate in mir3da_core.h.
 * Quelle 9 pero' NON bastano, e questo e' il fatto nuovo di questo lotto:
 *
 *   - quattro delle nove ne chiamano altre quattro del core
 *     (mir3da_read_data -> mir3da_read_raw_data, mir3da_temp_calibrate;
 *      mir3da_get_primary_offset -> mir3da_read_offset,
 *      mir3da_read_raw_data; mir3da_core_init e mir3da_chip_resume ->
 *      mir3da_parse_chip_info), e mir3da_temp_calibrate chiama squareRoot;
 *   - mir3da_core_init incorpora mir3da_module_detect (il corpo di
 *     module_detect compare due volte nel binario: una come funzione a
 *     0xffffff800878b204, una dentro core_init a 0xffffff800878b5c8);
 *   - e soprattutto la TABELLA dei descrittori di chip, che
 *     mir3da_core_init installa e che quattro delle nove dereferenziano,
 *     contiene QUATTRO PUNTATORI A FUNZIONE (NSA_NTO_calibrate,
 *     NSA_NTO_auto_calibrate, NSA_interrupt_ops, NSA_get_reg_data).
 *     Un puntatore in un inizializzatore e' un simbolo da risolvere
 *     esattamente come una chiamata: senza quelle quattro definizioni il
 *     link non riesce lo stesso.
 *
 * 9 + 4 + squareRoot + module_detect + 4 = 19.  Le 10 che restano fuori
 * (mir3da_register_read, mir3da_register_read_continuously,
 * mir3da_register_mask_write, mir3da_direction_remap, cycle_read_xyz,
 * mir3da_write_offset, mir3da_calibrate, mir3da_interrupt_ops,
 * mir3da_set_odr, mir3da_temp_calibrate_detect_static) sono `T` globali per
 * cui la scansione completa non trova alcun `bl` diretto.  Hanno soltanto
 * la relocation uniforme della tabella kallsyms, non una chiamata operativa;
 * per questo non compaiono negli errori del linker e ometterle non toglie
 * niente al link.  Non sono scritte perche' codice non necessario e' una
 * occasione in piu' di sbagliare, non perche' siano illeggibili.
 *
 *
 * 2. IL MODELLO DEI DATI, LETTO E NON DEDOTTO
 * -------------------------------------------
 *
 * La tabella dei descrittori sta a 0xffffff800992eda8, ha UNA voce di 184
 * byte, ed e' letta byte per byte dalla .data di stock.elf (il dump
 * completo e' nel rapporto).  Che sia una voce sola e non un array
 * indicizzato fra piu' famiglie di chip e' misurato: l'indice in .data a
 * 0xffffff800992ed9c vale -1 all'avvio e viene messo solo a 0
 * ("b90d9d1f str wzr, [x8,#3484]" in mir3da_module_detect), e la finestra
 * successiva di 184 byte non e' una seconda voce ma un altro oggetto.
 *
 * Il PASSO dei descrittori e' misurato, non supposto: mir3da_set_odr fa
 *   "8b0b0929 add x9, x9, x11, lsl #2"   (indice * 4)
 *   "7940d533 ldrh w19, [x9,#106]"       (addr, 16 bit)
 *   "3941b135 ldrb w21, [x9,#108]"       (mask)
 *   "3941b536 ldrb w22, [x9,#109]"       (value)
 * cioe' { short addr; u8 mask; u8 value; } di 4 byte, e i tre campi in
 * quest'ordine.  L'origine dell'array (offset 10) e' fissata da
 * mir3da_module_detect, che indirizza i primi due descrittori in modo
 * ASSOLUTO — "3976c921 ldrb w1, [x9,#3506]" = 0xffffff800992edb2 = base+10
 * per l'addr, e 0x992edb4/0x992edb5 per mask/value — e da mir3da_get_enable,
 * che legge addr a +22, mask a +24, value a +25.  Con passo 4 e origine 10
 * i due vincoli tornano insieme; con nessun'altra origine tornano.
 *
 * Che l'`addr` sia a 16 bit e con SEGNO lo prova mir3da_chip_resume:
 *   "79c03575 ldrsh w21, [x11,#26]"
 *   "37f80395 tbnz  w21, #31, ..."
 * cioe' il -1 (ff ff nella tabella) e' il terminatore dell'array init_regs.
 *
 * Due campi restano SENZA NOME e sono dichiarati come tali invece di essere
 * inventati:
 *   +8   due byte a zero, che nessuna delle 29 funzioni legge.  Non e'
 *        distinguibile da un `char asic[10]` invece di `char asic[8]`:
 *        i due byte in piu' sarebbero comunque zero.
 *   +144 otto byte a zero, senza rilocazione.  Sono allineati a 8 e
 *        precedono i quattro puntatori: potrebbero essere un QUINTO
 *        puntatore a NULL, o due interi, o altro.  Nessuna funzione li
 *        legge, quindi non c'e' modo di deciderlo.
 *
 * Le variabili .bss del core, tutte a 0xffffff8009cba000 + offset:
 *   +408  general_op_s *      installata da mir3da_install_general_ops
 *   +416  la tabella          installata da mir3da_core_init
 *   +424  chip_info, 16 byte  (u8 a +0, tre int a +4/+8/+12)
 *   +440..+460  lo stato di mir3da_temp_calibrate, sotto.
 *
 *
 * 3. I CINQUE GLOBALI DI mir3da_temp_calibrate SONO UNA MACCHINA SOLA
 * ------------------------------------------------------------------
 *
 * Cercare a chi servano funzione per funzione non dice niente; guardando
 * l'INTERO kernel di fabbrica in una volta invece si', ed e' cosi' che
 * questa funzione si e' aperta.  Scansione di tutte le 5.018.179 righe del
 * disassemblato (comando e output nel rapporto), accoppiando ogni
 * `adrp` alla pagina 0xffffff8009cba000 con l'accesso che la segue:
 *
 *   +440 (0x9cba1b8)  mir3da_temp_calibrate, mir3da_temp_calibrate_detect_static
 *   +444 (0x9cba1bc)  mir3da_temp_calibrate, mir3da_temp_calibrate_detect_static
 *   +448 (0x9cba1c0)  mir3da_temp_calibrate, mir3da_temp_calibrate_detect_static
 *   +452 (0x9cba1c4)  mir3da_temp_calibrate, mir3da_temp_calibrate_detect_static
 *   +456 (0x9cba1c8)  mir3da_temp_calibrate, mir3da_temp_calibrate_detect_static
 *   +460 (0x9cba1cc)  mir3da_temp_calibrate
 *
 * Due funzioni sole, in tutto il kernel, per cinque variabili: non sono
 * cinque incognite, sono lo stato di una macchina sola — l'ultimo campione
 * (x, y, z), un flag "ho gia' un campione", un contatore di quiete e un
 * offset di z.  E i nomi dei primi due glieli da' il binario, non io: la
 * printk di traccia e' "[MIR3DA] delta_sum=%d count_static=%d\n"
 * @0xffffff80091c05ae, con i due valori gia' in w1 e w2 al momento della
 * chiamata; e quella di uscita e'
 * "[MIR3DA] end mir3da_temp_calibrate z_offset=%d\n" @0xffffff80091c05fb.
 *
 * Un campo che invece NON ha lettori da nessuna parte: chip_info +4
 * (0xffffff8009cba1ac).  La stessa scansione trova un solo tocco in tutto
 * il kernel, ed e' una SCRITTURA, in mir3da_parse_chip_info.  E' scritto
 * qui perche' di fabbrica c'e', non perche' serva a qualcosa.
 *
 *
 * 4. QUELLO CHE NON SI SA, E CHE NON E' STATO INVENTATO
 * ----------------------------------------------------
 *
 *  a) Il nome del file (§ sopra).
 *  b) I due campi +8 e +144 della tabella (§2).
 *  c) La FIRMA dei due puntatori a +152 e +160.  Nessuna funzione del
 *     kernel di fabbrica li dereferenzia — mir3da_calibrate e' il corpo
 *     "2a1f03e0 mov w0, wzr" / "d65f03c0 ret" e non li tocca — e i due
 *     bersagli (NSA_NTO_calibrate @0xffffff800878be88 e
 *     NSA_NTO_auto_calibrate @0xffffff800878be90) sono le stesse due
 *     istruzioni.  Un puntatore mai chiamato e un corpo che ignora i
 *     parametri non permettono di ricavare quali fossero: qui i due slot
 *     sono `void *` e i due bersagli sono definiti senza argomenti.
 *     E' una dichiarazione di ignoranza, non una scelta di stile.
 *  d) La forma dello `struct` passato a NSA_interrupt_ops.  Il campo a +8
 *     e' letto come intero a 32 bit nel caso 2 ("b9400a88 ldr w8, [x20,#8]")
 *     e come byte nel caso 0 ("39402296 ldrb w22, [x20,#8]" e
 *     "39402696 ldrb w22, [x20,#9]").  Le due letture sono compatibili sia
 *     con un `int` a +8 che clang restringe, sia con byte distinti a +8 e
 *     +9: il binario non le distingue, e qui c'e' una union che dice
 *     esattamente questo.  Il tipo non e' osservabile da nessun chiamante,
 *     perche' in questo kernel NESSUNO chiama per quella via (l'unico
 *     ponte, mir3da_interrupt_ops, non e' chiamato da mir3da_cust.c).
 *  e) La forma di ciclo di mir3da_module_detect.  Il binario indirizza
 *     `mir3da_chip_ops_tbl[0]` in modo assoluto e poi scrive 0 nell'indice.
 *     Con una tabella di UNA voce un ciclo `for (i = 0; i < ARRAY_SIZE)` e
 *     una versione senza ciclo producono lo stesso codice: il binario non
 *     le distingue.  E' scritto col ciclo perche' e' l'unica forma che
 *     spiega insieme l'indice, il -1 come sentinella e la tabella
 *     indicizzata; non perche' sia misurato.
 *  f) mir3da_temp_calibrate: il VALORE e' ricostruito istruzione per
 *     istruzione, la FORMA del sorgente no.  clang ha fuso le code,
 *     convertito quasi ogni `if` in `csel`/`cneg`/`csinc` e incorporato
 *     squareRoot due volte; il nesting qui sotto e' la ricostruzione piu'
 *     semplice che riproduce tutti i rami, non una lettura del sorgente di
 *     fabbrica.  Ogni costante ha accanto la riga da cui viene.
 *
 *
 * 5. LE COSTANTI CHE SEMBRANO STRANE E NON LO SONO
 * ------------------------------------------------
 *
 * mir3da_temp_calibrate scala gli assi orizzontali con "* 130 / 200"
 * (= 0,65) e tira l'asse verticale verso 1024 con "* 70 / 200" (= 0,35).
 * I due pesi sommano a 1: e' un filtro lineare verso il valore di gravita'
 * atteso.  Che il divisore sia 200 e non 100 e' letto, non scelto:
 *   "5290a3eb mov w11, #0x851f" / "72aa3d6b movk w11, #0x51eb, lsl #16"
 *   "9b2b7d08 smull x8, w8, w11" / "9366fd08 asr x8, x8, #38"
 * e 0x51eb851f >> 38 e' 1/200 (per 1/100 lo scorrimento sarebbe 37); il
 * 130 si vede in "0b081908 add w8, w8, w8, lsl #6" (65x) seguito da
 * "531f7908 lsl w8, w8, #1".  Allo stesso modo la divisione per 5 in testa
 * e' "528ccce8 mov w8, #0x6667" + "72acccc8 movk w8, #0x6666, lsl #16" +
 * "9361fd08 asr x8, x8, #33" (0x66666667 >> 33 = 1/5), e quella per 20 di
 * mir3da_get_primary_offset e' lo stesso magico con "9363fd08 asr x8, x8,
 * #35".
 */
#include <linux/kernel.h>
#include <linux/types.h>

#include "mir3da_core.h"

/* ------------------------------------------------------------------ */
/* Log                                                                  */
/* ------------------------------------------------------------------ */

/*
 * "b94d9d08 ldr w8, [x8,#3484]" e le `ldrb` dei test danno la stessa
 * variabile di mir3da_cust.c: 0xffffff800992ed98.  Definita qui, dichiarata
 * `extern` in mir3da_core.h — vedi il commento la' sopra per la misura.
 */
int Log_level = MIR_ERR;

/*
 * Il core NON chiama printk: ogni messaggio passa da
 * general_op_s.myprintk (+64).  Lo si vede su tutti e dodici i siti, per
 * esempio in mir3da_read_data:
 *   "f940cd08 ldr x8, [x8,#408]"   la general_op_s
 *   "f9402108 ldr x8, [x8,#64]"    lo slot myprintk
 *   "d63f0100 blr x8"
 * E' la ragione per cui i letterali di questo file NON hanno il prefisso
 * "\x013" che hanno quelli di mir3da_cust.c: nessuna macro KERN_ li tocca.
 */
#define MI_ERR(fmt, ...)						\
	do {								\
		if (Log_level & MIR_ERR)				\
			p_mir3da_general_ops->myprintk("[MIR3DA] " fmt,	\
						       ##__VA_ARGS__);	\
	} while (0)

#define MI_MSG(fmt, ...)						\
	do {								\
		if (Log_level & MIR_MSG)				\
			p_mir3da_general_ops->myprintk("[MIR3DA] " fmt,	\
						       ##__VA_ARGS__);	\
	} while (0)

#define MI_DATA(fmt, ...)						\
	do {								\
		if (Log_level & MIR_DATA)				\
			p_mir3da_general_ops->myprintk("[MIR3DA] " fmt,	\
						       ##__VA_ARGS__);	\
	} while (0)

/* ------------------------------------------------------------------ */
/* Il modello dei dati                                                  */
/* ------------------------------------------------------------------ */

/*
 * { short addr; u8 mask; u8 value; } — passo 4, misurato su mir3da_set_odr
 * (vedi il cappello, §2).  `addr` e' con segno perche' -1 termina gli
 * array: "79c03575 ldrsh w21, [x11,#26]" seguito da
 * "37f80395 tbnz w21, #31, ...".
 */
struct mir_reg_obj {
	short		addr;
	unsigned char	mask;
	unsigned char	value;
};

/*
 * La voce della tabella dei chip: 184 byte, misurati sul moltiplicatore che
 * ogni funzione del core usa per indicizzarla —
 *   "5280170a mov w10, #0xb8"  (184)
 *   "9b0a2128 madd x8, x9, x10, x8"
 *
 * Gli offset dei campi non sono ripartiti a occhio: ognuno e' il numero che
 * compare in un accesso del disassemblato.
 */
struct mir_chip_ops {
	/* +0   il nome, primo argomento del "%s" di mir3da_core_init */
	char			asic[8];
	/* +8   due byte a zero, nessun lettore (vedi §2 e §4b) */
	unsigned char		sconosciuto_8[2];
	/* +10  "3976c921 ldrb w1, [x9,#3506]" (= 0x992edb2, base+10) */
	struct mir_reg_obj	chip_id;
	/* +14  "3976d921 ldrb w1, [x9,#3510]" (= 0x992edb6, base+14) */
	struct mir_reg_obj	mod_id;
	/* +18  "79402515 ldrh w21, [x8,#18]" in mir3da_chip_resume */
	struct mir_reg_obj	soft_reset;
	/* +22  "39405901 ldrb w1, [x8,#22]" in mir3da_get_enable */
	struct mir_reg_obj	power_mode;
	/*
	 * +26  "79c03575 ldrsh w21, [x11,#26]" con
	 *      "8b28096b add x11, x11, w8, uxtb #2" e il ciclo che finisce a
	 *      "7100293f cmp w9, #0xa" / "54fffc09 b.ls ..." — undici voci,
	 *      le ultime due a -1.
	 */
	struct mir_reg_obj	init_regs[11];
	/*
	 * +70  mir3da_read_offset le srotola tutte e nove:
	 *      "79c08d01 ldrsh w1, [x8,#70]" ... "79c0cd01 ldrsh w1, [x8,#102]"
	 */
	struct mir_reg_obj	offset_regs[9];
	/* +106 "7940d533 ldrh w19, [x9,#106]" in mir3da_set_odr, tre voci */
	struct mir_reg_obj	odr_regs[3];
	/*
	 * +118 "3941d921 ldrb w1, [x9,#118]" in mir3da_read_raw_data, che ne
	 *      legge sei di fila ("321f07e2 orr w2, wzr, #0x6").
	 */
	struct mir_reg_obj	data_regs[6];
	/* +142 "3942392a ldrb w10, [x9,#142]" — scorrimento del byte alto */
	unsigned char		data_msb_shift;
	/* +143 "39423d29 ldrb w9, [x9,#143]" — usato come "8 - questo" */
	unsigned char		data_shift_base;
	/* +144 otto byte a zero senza rilocazione, nessun lettore (§4b) */
	unsigned char		sconosciuto_144[8];
	/* +152 e +160: firma non osservabile (§4c) */
	void			*calibrate;
	void			*auto_calibrate;
	/* +168 "f9405508 ldr x8, [x8,#168]" in mir3da_interrupt_ops */
	int			(*interrupt_ops)(PLAT_HANDLE handle, void *ops);
	/* +176 "f9405908 ldr x8, [x8,#176]" in mir3da_get_reg_data */
	int			(*get_reg_data)(PLAT_HANDLE handle, char *buf);
};

/*
 * Lo `struct` che NSA_interrupt_ops riceve in x1.  Nessun chiamante nel
 * kernel di fabbrica, quindi i nomi dei campi non esistono: sono gli
 * offset.  Vedi §4d per l'ambiguita' del campo a +8.
 */
struct mir3da_int_ops_s {
	int	op;		/* +0  "b9400028 ldr w8, [x1]", 0..3 */
	int	campo_4;	/* +4  "b9400688 ldr w8, [x20,#4]" */
	union {
		int		campo_8;	/* "b9400a88 ldr w8, [x20,#8]" */
		unsigned char	campo_8b[4];	/* "39402296 ldrb w22, [x20,#8]" */
	};
	unsigned char	campo_12;	/* +12 "39403282 ldrb w2, [x20,#12]" */
	unsigned char	campo_13;	/* +13 "39403696 ldrb w22, [x20,#13]" */
	unsigned char	campo_14;	/* +14 mai letto */
	unsigned char	campo_15;	/* +15 "39403e97 ldrb w23, [x20,#15]" */
};

/* chip_info, .bss 0xffffff8009cba1a8, 16 byte */
struct mir3da_chip_info {
	/* +0  "3906a128 strb w8, [x9,#424]" — il registro 0xC0 grezzo */
	unsigned char	reg_c0;
	/* +4  "b900012b str w11, [x9]" con x9 = 0x9cba1ac. Nessun lettore. */
	int		campo_4;
	/* +8  "b941b108 ldr w8, [x8,#432]" — vale 2, 3 o 4 */
	int		campo_8;
	/* +12 "b901b509 str w9, [x8,#436]" — vale da 2 a 6 */
	int		campo_12;
};

/* ------------------------------------------------------------------ */
/* Prototipi                                                            */
/* ------------------------------------------------------------------ */

int squareRoot(int val);
int mir3da_read_offset(PLAT_HANDLE handle, unsigned char *offset);
int mir3da_temp_calibrate(int *x, int *y, int *z);
int mir3da_module_detect(PLAT_HANDLE handle);
int mir3da_parse_chip_info(PLAT_HANDLE handle);

static int mir3da_read_raw_data(PLAT_HANDLE handle, short *x, short *y,
				short *z);
static int NSA_interrupt_ops(PLAT_HANDLE handle, void *ops);
static int NSA_get_reg_data(PLAT_HANDLE handle, char *buf);
static int NSA_NTO_calibrate(void);
static int NSA_NTO_auto_calibrate(void);

/* ------------------------------------------------------------------ */
/* I dati di fabbrica                                                   */
/* ------------------------------------------------------------------ */

/*
 * La voce unica a 0xffffff800992eda8, byte per byte dalla .data di
 * stock.elf.  Il dump esadecimale completo e il comando che lo produce
 * sono in docs/bringup/rapporti/rapporto-mir3da-core.md; qui sotto ogni
 * riga e' la traduzione di quattro byte di quel dump.
 *
 *   +0   4e 53 41 5f 4e 54 4f 00   "NSA_NTO"
 *   +8   00 00                     sconosciuto
 *   +10  01 00 ff 13               chip_id     addr 0x01 mask 0xff val 0x13
 *   +14  14 00 ff 00               mod_id      addr 0x14 mask 0xff val 0x00
 *   +18  00 00 24 24               soft_reset  addr 0x00 mask 0x24 val 0x24
 *   +22  11 00 80 80               power_mode  addr 0x11 mask 0x80 val 0x80
 *   [...]
 *   +142 08                        data_msb_shift
 *   +143 04                        data_shift_base
 */
static struct mir_chip_ops mir3da_chip_ops_tbl[] = {
	{
		.asic		= "NSA_NTO",
		.chip_id	= { 0x01, 0xff, 0x13 },
		.mod_id		= { 0x14, 0xff, 0x00 },
		.soft_reset	= { 0x00, 0x24, 0x24 },
		.power_mode	= { 0x11, 0x80, 0x80 },
		.init_regs	= {
			{ 0x0f, 0x03, 0x00 },
			{ 0x11, 0xff, 0x3e },
			{ 0x10, 0xff, 0x07 },
			{ 0x17, 0xff, 0x00 },
			{ 0x1a, 0xff, 0x00 },
			{ 0x7f, 0xff, 0x83 },
			{ 0x7f, 0xff, 0x69 },
			{ 0x7f, 0xff, 0xbd },
			{ 0x20, 0x0f, 0x05 },
			{   -1, 0x00, 0x00 },
			{   -1, 0x00, 0x00 },
		},
		.offset_regs	= {
			{ 0x83, 0xff, 0x00 },
			{ 0x84, 0xff, 0x00 },
			{ 0x85, 0xff, 0x00 },
			{ 0x86, 0xff, 0x00 },
			{ 0x87, 0xff, 0x00 },
			{ 0x88, 0xff, 0x00 },
			{ 0x38, 0xff, 0x00 },
			{ 0x39, 0xff, 0x00 },
			{ 0x3a, 0xff, 0x00 },
		},
		.odr_regs	= {
			{ 0x10, 0x0f, 0x06 },
			{ 0x10, 0x0f, 0x07 },
			{ 0x10, 0x0f, 0x08 },
		},
		.data_regs	= {
			{ 0x02, 0xff, 0x00 },
			{ 0x03, 0xff, 0x00 },
			{ 0x04, 0xff, 0x00 },
			{ 0x05, 0xff, 0x00 },
			{ 0x06, 0xff, 0x00 },
			{ 0x07, 0xff, 0x00 },
		},
		.data_msb_shift	 = 8,
		.data_shift_base = 4,
		.calibrate	 = (void *)NSA_NTO_calibrate,
		.auto_calibrate	 = (void *)NSA_NTO_auto_calibrate,
		.interrupt_ops	 = NSA_interrupt_ops,
		.get_reg_data	 = NSA_get_reg_data,
	},
};

/*
 * .data 0xffffff800992ed9c = ff ff ff ff.  Messo a 0 solo da
 * mir3da_module_detect ("b90d9d1f str wzr, [x8,#3484]") e confrontato con
 * -1 da mir3da_parse_chip_info ("3100051f cmn w8, #0x1").
 */
/*
 * NON E' `static`, E LA MISURA LO DIMOSTRA. Il binario lo legge sempre a
 * larghezza piena e con segno -- "b98d9d29 ldrsw x9, [x9,#3484]" in
 * `mir3da_interrupt_ops`, "b98d9eca ldrsw x10, [x22,#3484]" in
 * `mir3da_write_offset`, e cosi' in tutte le altre. Dichiarandolo `static`,
 * clang vede che in questa unita' vale solo -1 o 0, lo restringe a UN BIT e
 * lo ricostruisce con "ldrb" + "mvn" + "sbfx x8, x8, #0, #1": tre
 * istruzioni al posto di una, in ogni funzione che indicizza la tabella.
 *
 * L'effetto sull'intero file, misurato:
 *   con `static`   9 funzioni su 29 di dimensione identica a fabbrica
 *   senza          13 su 29
 * e le quattro che si aggiungono sono `mir3da_get_enable`,
 * `mir3da_get_reg_data`, `mir3da_interrupt_ops` e `mir3da_set_enable` --
 * tre delle quali erano gia' scritte e riviste. E' la stessa classe di
 * prova di `ilitek_probe_riuscito` nel gruppo C di ILITEK: una differenza
 * di DICHIARAZIONE che nessuna lettura a occhio troverebbe e che la
 * dimensione denuncia.
 *
 * Il costo dichiarato: `mir3da_write_offset` torna a srotolarsi e passa da
 * 148 a 492 byte contro i 144 di fabbrica (vedi il suo commento). Il
 * bilancio resta positivo di quattro funzioni, e i due numeri sono
 * entrambi riportati invece di scegliere quello che conviene.
 */
int mir3da_chip_index = -1;

/* .data 0xffffff800992eda0 = ff ff ff ff; l'esito memorizzato del parse. */
static int mir3da_chip_info_res = -1;

/* .bss 0xffffff8009cba198 e 0xffffff8009cba1a0 */
static struct general_op_s *p_mir3da_general_ops;
static struct mir_chip_ops *p_mir3da_chip_ops;

/* .bss 0xffffff8009cba1a8 */
static struct mir3da_chip_info chip_info;

/* .bss 0xffffff8009cba1b8 .. 0xffffff8009cba1cc — la macchina del §3 */
static int		count_static;
static short		last_x;
static short		last_y;
static short		last_z;
static unsigned char	have_last;
static int		z_offset;

/*
 * Controllo di forma a tempo di compilazione: nessun codice generato, ma
 * se un giorno un campo si sposta il build si ferma qui invece di produrre
 * un driver che legge il registro sbagliato.  Gli offset sono quelli
 * citati uno per uno sopra.
 */
static inline void __maybe_unused mir3da_controlla_forma(void)
{
	BUILD_BUG_ON(sizeof(struct mir_reg_obj) != 4);
	BUILD_BUG_ON(sizeof(struct mir_chip_ops) != 184);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, chip_id) != 10);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, mod_id) != 14);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, soft_reset) != 18);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, power_mode) != 22);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, init_regs) != 26);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, offset_regs) != 70);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, odr_regs) != 106);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, data_regs) != 118);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, data_msb_shift) != 142);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, data_shift_base) != 143);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, calibrate) != 152);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, auto_calibrate) != 160);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, interrupt_ops) != 168);
	BUILD_BUG_ON(offsetof(struct mir_chip_ops, get_reg_data) != 176);
	BUILD_BUG_ON(offsetof(struct mir3da_chip_info, campo_4) != 4);
	BUILD_BUG_ON(offsetof(struct mir3da_chip_info, campo_8) != 8);
	BUILD_BUG_ON(offsetof(struct mir3da_chip_info, campo_12) != 12);
	BUILD_BUG_ON(offsetof(struct mir3da_int_ops_s, campo_15) != 15);
}

/* ------------------------------------------------------------------ */
/* squareRoot @0xffffff800878a0a8, 76 byte                              */
/* ------------------------------------------------------------------ */

/*
 * Radice quadrata intera per bisezione sui bit pari.  Algoritmo e semantica
 * vengono dal disassemblato stock.
 *
 * DIVERGENZA RESIDUA: -4 byte, cioe' UNA istruzione in dimensione -- ma la
 * frase che seguiva («Prologo ed epilogo combaciano istruzione per istruzione»
 * e «cio' che resta e' UNA SOLA ISTRUZIONE») era piu' forte della misura, e va
 * corretta.  L'EPILOGO combacia, come sequenza, sfalsato di una posizione.  IL
 * PROLOGO NO: le due `mov` di inizializzazione stanno in ordine invertito (la
 * fabbrica fa `mov x9, xzr` e poi `mov w0, wzr`, noi il contrario).  E per
 * ISTRUZIONE il conto e' 19 di fabbrica contro 18 nostre, con DIECI uguali
 * dopo la normalizzazione: le posizioni che non tornano sono sedici, non una.
 * Quattro byte di differenza e sedici posizioni non sono in contraddizione --
 * e' esattamente il motivo per cui questo progetto misura in tutti e due i
 * modi.
 *
 * In mezzo la fabbrica ha un salto vero
 * ("6b0c010c subs"@0xffffff800878a0c4 + "5400006a b.ge"@0xffffff800878a0c8 +
 * "13017c00 asr"@0xffffff800878a0cc + "14000003 b"@0xffffff800878a0d0) dove
 * noi emettiamo due `csel`.  E' una decisione del generatore di codice, non
 * della forma del sorgente: sono state provate tre forme -- differenza
 * calcolata prima e segno provato poi, uscita anticipata con `continue`, e
 * questa -- e tutte e tre danno 72 byte con gli stessi due `csel`.  Questa e'
 * quella che resta piu' vicina, perche' emette `add w12, w0, w11` seguito da
 * un confronto, come la fabbrica.
 *
 * Le citazioni che seguono sono della fabbrica:
 *   "37f80220 tbnz w0, #31, ..."      il negativo esce con 0
 *   "320203ea orr w10, wzr, #0x40000000"
 *   "9ac9254b lsr x11, x10, x9"       lo scorrimento e' a 64 bit: la
 *                                     variabile di ciclo e' un `unsigned
 *                                     long`, non un `int` (sarebbe `lsr w`)
 *   "0b0b000c add w12, w0, w11"
 *   "6b0c010c subs w12, w8, w12"
 *   "2a800560 orr w0, w11, w0, asr #1"   il ramo preso
 *   "13017c00 asr w0, w0, #1"            il ramo non preso
 *   "91000929 add x9, x9, #0x2" / "f100813f cmp x9, #0x20"
 */
int squareRoot(int val)
{
	unsigned long i;
	unsigned long b = 0x40000000UL;
	int r = 0;
	int t;

	if (val < 0)
		return 0;

	/*
	 * LO SROTOLAMENTO E' L'UNICA COSA CHE SEPARAVA QUESTA FUNZIONE DALLA
	 * FABBRICA, e la pragma e' un'impalcatura dichiarata -- non emette
	 * codice, come `__used` in ilitek_mp.c.
	 *
	 * MISURATO: senza di essa il compilatore di FABBRICA (clang-r353983c,
	 * lo stesso che ha prodotto il binario) srotola e la funzione misura
	 * 444 byte contro i 76 di fabbrica. E' lui a dirlo:
	 *   $ <riga di comando vera> -Rpass=loop-unroll
	 *   mir3da_core.c:542:2: remark: completely unrolled loop with 16
	 *       iterations [-Rpass=loop-unroll]
	 * Con la pragma: 72 byte, cioe' UNA istruzione in meno di fabbrica.
	 *
	 * PERCHE' LA FABBRICA NON SROTOLI NON E' SPIEGATO. Non e' il
	 * compilatore (e' lo stesso) ne' le opzioni (sedici funzioni di questo
	 * file combaciano al byte, quindi le opzioni sono quelle). E' una
	 * forma del sorgente che non abbiamo trovato, e la pragma la surroga.
	 */
#pragma clang loop unroll(disable)
	for (i = 0; i < 32; i += 2) {
		t = r + (int)(b >> i);
		if (val >= t) {
			r = (int)(b >> i) | (r >> 1);
			val = val - t;
		} else {
			r = r >> 1;
		}
	}

	return r;
}

/* ------------------------------------------------------------------ */
/* mir3da_register_write @0xffffff800878a148, 32 byte — CONFINE         */
/* ------------------------------------------------------------------ */

/*
 * Un solo inoltro:
 *   "f940cd08 ldr x8, [x8,#408]"   la general_op_s
 *   "f9400508 ldr x8, [x8,#8]"     lo slot smi2c_write
 *   "d63f0100 blr x8"
 * x0/x1/x2 non vengono toccati: gli argomenti passano cosi' come sono.
 */
int mir3da_register_write(PLAT_HANDLE handle, u8 addr, u8 data)
{
	return p_mir3da_general_ops->smi2c_write(handle, addr, data);
}

/* ------------------------------------------------------------------ */
/* mir3da_read_data @0xffffff800878a288, 276 byte — CONFINE             */
/* ------------------------------------------------------------------ */

/*
 * I tre interi di appoggio sono azzerati prima di tutto —
 *   "f9000bff str xzr, [sp,#16]" (due) e "b9000fff str wzr, [sp,#12]" —
 * e questo conta: nel ramo che salta la calibrazione, *z riceve quello
 * zero.  La condizione che sceglie il ramo e' letterale:
 *   "71012d3f cmp w9, #0x4b" / "7102313f cmp w9, #0x8c" /
 *   "7103293f cmp w9, #0xca" su chip_info.reg_c0, e
 *   "7100153f cmp w9, #0x5" su chip_info.campo_12.
 */
int mir3da_read_data(PLAT_HANDLE handle, short *x, short *y, short *z)
{
	int nx = 0, ny = 0, nz = 0;
	int res;

	res = mir3da_read_raw_data(handle, x, y, z);
	if (res) {
		/* "[MIR3DA] mir3da_read_raw_data failed, rst = %d\n"
		 * @0xffffff80091c045e, caricata da
		 * "91117800 add x0, x0, #0x45e"
		 */
		MI_ERR("mir3da_read_raw_data failed, rst = %d\n", res);
		return res;
	}

	if (chip_info.reg_c0 != 0x4B && chip_info.reg_c0 != 0x8C &&
	    chip_info.reg_c0 != 0xCA && chip_info.campo_12 != 5) {
		nx = *x;
		ny = *y;
		nz = *z;

		mir3da_temp_calibrate(&nx, &ny, &nz);

		*x = nx;
		*y = ny;
	}

	*z = nz;

	return res;
}

/* ------------------------------------------------------------------ */
/* mir3da_read_raw_data @0xffffff800878a39c, 412 byte (statica)         */
/* ------------------------------------------------------------------ */

/*
 * Sei byte in un colpo solo — "321f07e2 orr w2, wzr, #0x6" — e il
 * successo e' misurato sul CONTEGGIO restituito, non su zero:
 *   "7100181f cmp w0, #0x6" / "54000701 b.ne ..."
 *
 * La ricomposizione dei tre assi e' la stessa tre volte:
 *   "1aca216a lsl w10, w11, w10"     byte alto << data_msb_shift (8)
 *   "2a0d014a orr w10, w10, w13"     | byte basso
 *   "4b0901c9 sub w9, w14, w9"       8 - data_shift_base (4)
 *   "13003d4a sxth w10, w10"         estensione a 16 bit con segno
 *   "1ac92949 asr w9, w10, w9"       scorrimento aritmetico
 */
static int mir3da_read_raw_data(PLAT_HANDLE handle, short *x, short *y,
				short *z)
{
	unsigned char buf[6] = { 0 };
	int res;

	res = p_mir3da_general_ops->smi2c_read_block(handle,
			p_mir3da_chip_ops[mir3da_chip_index].data_regs[0].addr,
			6, buf);
	if (res != 6) {
		/* "[MIR3DA] i2c block read failed\n\n" @0xffffff80091c058d */
		MI_ERR("i2c block read failed\n\n");
		return -1;
	}

	/*
	 * GLI STESSI DUE GLOBALI RILETTI A OGNI USO, come in
	 * `mir3da_read_offset`: la `smi2c_read_block` e' una chiamata
	 * indiretta opaca e il compilatore non puo' tenerseli attraverso di
	 * essa. Misurato: 380 -> 404 byte, contro i 412 di fabbrica.
	 *
	 * RESIDUO -8, DICHIARATO. Sono due istruzioni e stanno
	 * nell'allocazione dei registri: la fabbrica emette UNDICI `adrp` in
	 * questa funzione, noi NOVE -- due in piu' delle nostre, ed e'
	 * esattamente lo scarto.
	 *
	 * LA PRIMA STESURA DI QUESTA NOTA SPIEGAVA IL RESIDUO AL CONTRARIO.
	 * Diceva che la fabbrica «carica i due globali con una SOLA
	 * "a9402528 ldp"@0xffffff800878a3c8 mentre noi emettiamo tre `adrp`
	 * separate»: la `ldp` c'e' davvero -- x9 viene da
	 * "91066129 add"@0xffffff800878a3bc e i due puntatori sono adiacenti --
	 * ma fa risparmiare un'istruzione ALLA FABBRICA, quindi non puo'
	 * spiegare la fabbrica piu' LUNGA.  La spinta e' nel verso opposto, e
	 * viene dalle due `adrp` in piu' che la fabbrica emette altrove:
	 * rimaterializza indirizzi di pagina dove il nostro build li tiene nei
	 * registri.
	 *
	 * (E la citazione portava la forma `@0x...-4`, che nessuno strumento di
	 * questo progetto sa verificare: era l'unica delle undici di questo file
	 * che il controllo per indirizzo non risolveva, e l'aritmetica era
	 * comunque sbagliata -- la `add` sta a -12 dalla `ldp`, non a -4.)
	 */
	*x = (short)((short)((buf[1] << p_mir3da_chip_ops[mir3da_chip_index].data_msb_shift) | buf[0]) >>
		     (8 - p_mir3da_chip_ops[mir3da_chip_index].data_shift_base));
	*y = (short)((short)((buf[3] << p_mir3da_chip_ops[mir3da_chip_index].data_msb_shift) | buf[2]) >>
		     (8 - p_mir3da_chip_ops[mir3da_chip_index].data_shift_base));
	*z = (short)((short)((buf[5] << p_mir3da_chip_ops[mir3da_chip_index].data_msb_shift) | buf[4]) >>
		     (8 - p_mir3da_chip_ops[mir3da_chip_index].data_shift_base));

	/* "[MIR3DA] mir3da_raw: x=%d, y=%d, z=%d\n" @0xffffff80091c062b */
	MI_DATA("mir3da_raw: x=%d, y=%d, z=%d\n", *x, *y, *z);

	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_temp_calibrate @0xffffff800878a538, 1656 byte                 */
/* ------------------------------------------------------------------ */

/*
 * Vedi il cappello, §3 (i cinque globali sono una macchina sola), §4f (la
 * forma non e' misurata) e §5 (le costanti).
 *
 * Lo schema: se il modulo di x e di y e' piccolo la gravita' e' su z e si
 * ricalcola z dalla sfera, tenendo un offset persistente che viene fissato
 * quando il dispositivo e' fermo da dieci campioni; se invece un asse
 * orizzontale e' vicino a 1024, e' lui il verticale e viene tirato verso
 * 1024.  Gli assi non verticali sono scalati di 130/200.
 */
int mir3da_temp_calibrate(int *x, int *y, int *z)
{
	int delta_x, delta_y, delta_z, delta_sum;
	int old_x, old_y, old_z;
	int is_static = 0;
	int tmp;
	/*
	 * "b9800057 ldrsw x23, [x2]" prima di ogni altra cosa, poi
	 * "9361fd08 asr x8, x8, #33" (/5) e
	 * "0b080918 add w24, w8, w8, lsl #2" (*5) e
	 * "4b1802f7 sub w23, w23, w24": il resto della divisione per 5.
	 */
	int rem = *z % 5;

	/* "[MIR3DA] start mir3da_temp_calibrate\n" @0xffffff80091c05d5 */
	MI_MSG("start mir3da_temp_calibrate\n");

	/* "71031d5f cmp w10, #0xc7" / "5400024c b.gt ..." — 199, cioe' < 200 */
	if (abs(*x) < 200 && abs(*y) < 200) {
		/* "7100059f cmp w12, #0x1" su have_last */
		if (have_last != 1) {
			last_x = (short)*x;
			last_y = (short)*y;
			last_z = (short)*z;
			have_last = 1;
		}

		old_x = last_x;
		old_y = last_y;
		old_z = last_z;

		last_x = (short)*x;
		last_y = (short)*y;
		last_z = (short)*z;

		delta_x = abs((short)*x - old_x);
		delta_y = abs((short)*y - old_y);
		delta_z = abs((short)*z - old_z);
		delta_sum = delta_x + delta_y + delta_z;

		/* "7100f03f cmp w1, #0x3c" / "1a8ba7e2 csinc w2, wzr, w11, ge" */
		if (delta_sum >= 60)
			count_static = 0;
		else
			count_static = count_static + 1;

		/* "[MIR3DA] delta_sum=%d count_static=%d\n" @0xffffff80091c05ae */
		MI_MSG("delta_sum=%d count_static=%d\n", delta_sum,
		       count_static);

		/* "7100285f cmp w2, #0xa" */
		if (count_static >= 10) {
			count_static = 10;
			is_static = 1;
		}

		/*
		 * "320c03eb orr w11, wzr, #0x100000" = 1024*1024, poi due
		 * "msub" e squareRoot incorporata.
		 */
		tmp = squareRoot(1024 * 1024 - (*x) * (*x) - (*y) * (*y)) + rem;

		if (is_static) {
			if (z_offset == 0) {
				/* "5a89a529 cneg w9, w9, lt" poi
				 * "4b080121 sub w1, w9, w8"
				 */
				z_offset = (*z < 0 ? -tmp : tmp) - *z;
			} else if (abs(abs(*z + z_offset) - 1024) >= 131) {
				/* "71020d7f cmp w11, #0x83" */
				*z = (*z < 0 ? -1 : 1) * tmp;
				z_offset = 0;
			} else {
				*z = (*z < 0 ? -1 : 1) * tmp - z_offset;
			}
		} else if (z_offset == 0) {
			*z = (*z < 0 ? -1 : 1) * tmp;
		}

		*x = *x * 130 / 200;
		*y = *y * 130 / 200;
	} else if (abs(abs(*x) - 1024) < 200 && z_offset != 0 &&
		   abs(*y) < 200) {
		/*
		 * "7110055f cmp w10, #0x401" separa i due casi su abs(*x),
		 * "7100051f cmp w8, #0x1" separa il segno di *x.
		 */
		if (abs(*x) > 1024) {
			if (*x > 0)
				*x = *x - (abs(*x) - 1024) * 70 / 200;
			else
				*x = *x + (abs(*x) - 1024) * 70 / 200;
		} else {
			if (*x > 0)
				*x = *x + (1024 - abs(*x)) * 70 / 200;
			else
				*x = *x - (1024 - abs(*x)) * 70 / 200;
		}
		*y = *y * 130 / 200;
	} else if (abs(*x) < 200 && z_offset != 0 &&
		   abs(abs(*y) - 1024) < 200) {
		if (abs(*y) > 1024) {
			if (*y > 0)
				*y = *y - (abs(*y) - 1024) * 70 / 200;
			else
				*y = *y + (abs(*y) - 1024) * 70 / 200;
		} else {
			if (*y > 0)
				*y = *y + (1024 - abs(*y)) * 70 / 200;
			else
				*y = *y - (1024 - abs(*y)) * 70 / 200;
		}
		*x = *x * 130 / 200;
	} else if (z_offset == 0) {
		if (abs(*x) < 200 && abs(abs(*y) - 1024) < 200)
			*z = (*z < 0 ? -1 : 1) * abs(*x) * 130 / 200;
		else if (abs(*y) < 200 && abs(abs(*x) - 1024) < 200)
			*z = (*z < 0 ? -1 : 1) * abs(*y) * 130 / 200;
		else
			*z = (*z < 0 ? -1 : 1) *
			     (squareRoot(1024 * 1024 - (*x) * (*x) -
					 (*y) * (*y)) + rem);
	}

	/* "[MIR3DA] end mir3da_temp_calibrate z_offset=%d\n" @0xffffff80091c05fb */
	MI_MSG("end mir3da_temp_calibrate z_offset=%d\n", z_offset);

	/* "35ffdc01 cbnz w1, ..." / "12800000 mov w0, #0xffffffff" */
	if (z_offset == 0)
		return -1;

	*z = *z + z_offset;

	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_read_offset @0xffffff800878acd0, 492 byte                     */
/* ------------------------------------------------------------------ */

/*
 * Nel binario il ciclo e' srotolato per intero — nove copie, con gli offset
 * 70, 74, 78, 82, 86, 90, 94, 98, 102 e i nove "add x2, x19, #N" da 0 a 8.
 * L'uscita anticipata su `addr` negativo restituisce 0 esplicitamente
 * ("2a1f03e0 mov w0, wzr" a 0xffffff800878aea4).
 */
/*
 * I DUE GLOBALI SONO RILETTI A OGNI GIRO, e non e' stile: e' l'unica lettura
 * compatibile col binario. Il ciclo e' srotolato in nove copie (nove `blr`)
 * e OGNI copia ricomincia da
 *   "f940d2a8 ldr"@0xffffff800878ad24    p_mir3da_chip_ops
 *   "b98d9ec9 ldrsw"@0xffffff800878ad28  mir3da_chip_index (CON SEGNO)
 * prima della "9b0a2128 madd"@0xffffff800878ad30 che moltiplica per 184.
 * Estrarre `&p_mir3da_chip_ops[mir3da_chip_index]` in un locale toglie due
 * istruzioni per copia: otto copie x due = sedici istruzioni = 64 byte, che
 * erano ESATTAMENTE lo scarto misurato (428 contro 492).
 *
 * Il compilatore non puo' tenerseli da solo perche' `smi2c_read` e' una
 * chiamata indiretta opaca: e' la stessa ragione per cui `fts_data` viene
 * riletto in `fts_fwupg_get_boot_state` di FT8719.
 *
 * "79c08d01 ldrsh"@0xffffff800878ad00 legge `addr` come HALFWORD CON SEGNO, e
 * "37f80d01 tbnz"@0xffffff800878ad04 ne prova il bit 31: e' un `s16`.
 */
int mir3da_read_offset(PLAT_HANDLE handle, unsigned char *offset)
{
	int res = 0;
	int i;

	for (i = 0; i < 9; i++) {
		if (p_mir3da_chip_ops[mir3da_chip_index].offset_regs[i].addr < 0)
			return 0;

		res = p_mir3da_general_ops->smi2c_read(handle,
			p_mir3da_chip_ops[mir3da_chip_index].offset_regs[i].addr,
			&offset[i]);
		if (res)
			return res;
	}

	return res;
}

/* ------------------------------------------------------------------ */
/* mir3da_get_enable @0xffffff800878af84, 164 byte — CONFINE            */
/* ------------------------------------------------------------------ */

/*
 * "390013ff strb wzr, [sp,#4]" azzera il byte prima della lettura, e
 * l'esito e' il complemento del bit:
 *   "6a0a011f tst w8, w10" / "1a9f17e8 cset w8, eq"
 */
int mir3da_get_enable(PLAT_HANDLE handle, u8 *enable)
{
	struct mir_chip_ops *chip = &p_mir3da_chip_ops[mir3da_chip_index];
	unsigned char reg_data = 0;
	int res;

	res = p_mir3da_general_ops->smi2c_read(handle, chip->power_mode.addr,
					       &reg_data);
	if (res)
		return res;

	*enable = ((chip->power_mode.mask & reg_data) == 0);

	return res;
}

/* ------------------------------------------------------------------ */
/* mir3da_set_enable @0xffffff800878b028, 220 byte — CONFINE            */
/* ------------------------------------------------------------------ */

/*
 * "72001c3f tst w1, #0xff" / "54000060 b.eq ...": e' lo SPEGNIMENTO a
 * scrivere il valore della tabella (0x80, il bit di sospensione), non
 * l'accensione.  Il byte di appoggio NON e' azzerato prima della lettura,
 * a differenza di mir3da_get_enable: nel binario manca la `strb wzr`.
 */
int mir3da_set_enable(PLAT_HANDLE handle, bool enable)
{
	struct mir_chip_ops *chip = &p_mir3da_chip_ops[mir3da_chip_index];
	unsigned char reg_data;
	unsigned char val;
	int res;

	if (enable)
		val = 0;
	else
		val = chip->power_mode.value;

	res = p_mir3da_general_ops->smi2c_read(handle, chip->power_mode.addr,
					       &reg_data);
	if (res)
		return res;

	/* "0a350108 bic w8, w8, w21" / "0a15028a and w10, w20, w21" */
	reg_data = (reg_data & ~chip->power_mode.mask) |
		   (val & chip->power_mode.mask);

	return p_mir3da_general_ops->smi2c_write(handle,
						 chip->power_mode.addr,
						 reg_data);
}

/* ------------------------------------------------------------------ */
/* mir3da_get_reg_data @0xffffff800878b104, 48 byte — CONFINE           */
/* ------------------------------------------------------------------ */

/*
 * Solo un inoltro attraverso la tabella:
 *   "f9405908 ldr x8, [x8,#176]" / "d63f0100 blr x8"
 */
int mir3da_get_reg_data(PLAT_HANDLE handle, char *buf)
{
	return p_mir3da_chip_ops[mir3da_chip_index].get_reg_data(handle, buf);
}

/* ------------------------------------------------------------------ */
/* mir3da_module_detect @0xffffff800878b204, 260 byte                   */
/* ------------------------------------------------------------------ */

/*
 * Il binario indirizza la voce 0 in modo assoluto (0x992edb2 = base+10 e
 * 0x992edb6 = base+14) e poi mette 0 nell'indice.  Sulla forma di ciclo
 * vedi il cappello, §4e.
 *
 * Il confronto e' fra il valore mascherato e quello atteso:
 *   "0a090149 and w9, w10, w9" / "6b29011f cmp w8, w9, uxtb"
 * e il valore mascherato viene RISCRITTO nella variabile locale
 * ("390013e9 strb w9, [sp,#4]") — inutile, ma di fabbrica c'e'.
 */
int mir3da_module_detect(PLAT_HANDLE handle)
{
	/*
	 * NON AZZERATE: le riempie `smi2c_read`, e la fabbrica non scrive niente
	 * nei loro due slot prima di chiamarla. Con `= 0` clang emetteva
	 * "390013ff strb wzr, [sp,#4]" e "390003ff strb wzr, [sp]", che nel
	 * binario non ci sono -- e sono anche in `mir3da_core_init`, dove questa
	 * funzione e' incorporata.
	 *
	 * DIFETTO DELLA FABBRICA, riprodotto: se la lettura fallisce senza
	 * scrivere, i due byte restano indefiniti e il confronto li usa lo
	 * stesso.
	 */
	unsigned char reg_data;
	unsigned char reg_data2;
	struct mir_chip_ops *chip;
	int res;
	int i;

	for (i = 0; i < ARRAY_SIZE(mir3da_chip_ops_tbl); i++) {
		chip = &mir3da_chip_ops_tbl[i];

		res = p_mir3da_general_ops->smi2c_read(handle,
						       chip->chip_id.addr,
						       &reg_data);
		if (res)
			return res;

		reg_data = reg_data & chip->chip_id.mask;
		if (reg_data != chip->chip_id.value)
			return -1;

		res = p_mir3da_general_ops->smi2c_read(handle,
						       chip->mod_id.addr,
						       &reg_data2);
		if (res)
			return res;

		reg_data2 = reg_data2 & chip->mod_id.mask;
		if (reg_data2 != chip->mod_id.value)
			return -1;

		/* "[MIR3DA] Found Gsensor MIR3DA !\n" @0xffffff80091c048e */
		MI_MSG("Found Gsensor MIR3DA !\n");

		mir3da_chip_index = i;

		return 0;
	}

	return -1;
}

/* ------------------------------------------------------------------ */
/* mir3da_parse_chip_info @0xffffff800878b308, 524 byte                 */
/* ------------------------------------------------------------------ */

/*
 * Le quattro variabili sono inizializzate tutte a 0xFF da un solo
 * "32001fe0 orr w0, wzr, #0xff" seguito da tre `strb`: la quarta resta in
 * w0 ed e' il valore restituito quando l'indice vale ancora -1.  Quel 255
 * e' POSITIVO, quindi mir3da_chip_resume lo prende per un successo — e' un
 * comportamento di fabbrica, riprodotto e non corretto.
 *
 * Che l'esito delle letture sia troncato a 8 bit lo dice
 * "72001c00 ands w0, w0, #0xff": la variabile e' un `unsigned char`.
 *
 * Lo switch e' una tabella di salto a 0xffffff8008f54170, byte
 * 00 06 1e 0a 16 per gli indici 0..4 ("3875690a ldrb w10, [x8,x21]" e
 * "8b0a0929 add x9, x9, x10, lsl #2" su base 0xffffff800878b470): il caso
 * 2 salta direttamente all'uscita e non fa niente.
 */
int mir3da_parse_chip_info(PLAT_HANDLE handle)
{
	unsigned char res = 0xFF;
	unsigned char reg_c0 = 0xFF;
	unsigned char reg_c1 = 0xFF;
	unsigned char reg_8f = 0xFF;
	unsigned int sel;
	int t;

	/* "3100051f cmn w8, #0x1" */
	if (mir3da_chip_index == -1)
		return res;

	res = p_mir3da_general_ops->smi2c_read(handle, 0xC0, &reg_c0);
	if (res)
		return res;

	chip_info.reg_c0 = reg_c0;

	/* "7101011f cmp w8, #0x40" / "54000062 b.cs ..." */
	if (reg_c0 < 0x40)
		return -1;

	/* "53067d0b lsr w11, w8, #6" */
	chip_info.campo_4 = reg_c0 >> 6;
	chip_info.campo_8 = 2;
	chip_info.campo_12 = 2;

	/* "53037d08 lsr w8, w8, #3" / "11001d08 add w8, w8, #0x7" /
	 * "12000908 and w8, w8, #0x7" / "7100091f cmp w8, #0x2"
	 */
	t = ((reg_c0 >> 3) + 7) & 7;
	if (t <= 2)
		chip_info.campo_8 = t + 2;

	res = p_mir3da_general_ops->smi2c_read(handle, 0xC1, &reg_c1);
	if (res)
		return res;

	if (chip_info.campo_8 == 2) {
		res = p_mir3da_general_ops->smi2c_read(handle, 0x8F, &reg_c0);
		if (res)
			return res;

		/* "53067d15 lsr w21, w8, #6" / "331e0135 bfi w21, w9, #2, #1" */
		sel = (reg_c1 >> 6);
		sel = (sel & ~0x4u) | ((reg_c0 & 1u) << 2);
	} else {
		/* "53057d15 lsr w21, w8, #5" */
		sel = reg_c1 >> 5;
	}

	res = p_mir3da_general_ops->smi2c_read(handle, 0x8F, &reg_8f);
	if (res)
		return res;

	/* "710012bf cmp w21, #0x4" / "540004a8 b.hi ..." */
	if (sel > 4)
		return res;

	switch (sel) {
	case 0:
		/* "39c033e8 ldrsb w8, [sp,#12]" / "37f80328 tbnz w8, #31" */
		if ((signed char)reg_8f < 0)
			chip_info.campo_12 = 3;
		else
			chip_info.campo_12 = 2;
		return 0;
	case 1:
		chip_info.campo_12 = 4;
		return 0;
	case 2:
		return res;
	case 3:
		/* "7100113f cmp w9, #0x4" / "7a4b0144 ccmp w10, w11, #0x4, eq"
		 * con w11 = 0x5a, poi "1a890529 cinc w9, w9, ne" su 5.
		 */
		if (chip_info.campo_8 == 4 && chip_info.reg_c0 == 0x5A)
			chip_info.campo_12 = 5;
		else
			chip_info.campo_12 = 6;
		return 0;
	case 4:
		chip_info.campo_12 = 6;
		return 0;
	}

	return res;
}

/* ------------------------------------------------------------------ */
/* mir3da_install_general_ops @0xffffff800878b514, 32 byte — CONFINE    */
/* ------------------------------------------------------------------ */

/*
 * "b40000c0 cbz x0, ..." / "12800000 mov w0, #0xffffffff" per il NULL,
 * "f900cd28 str x8, [x9,#408]" per il resto.
 */
int mir3da_install_general_ops(struct general_op_s *ops)
{
	if (!ops)
		return -1;

	p_mir3da_general_ops = ops;

	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_core_init @0xffffff800878b534, 416 byte — CONFINE             */
/* ------------------------------------------------------------------ */

/*
 * "9136a294 add x20, x20, #0xda8" / "f900d134 str x20, [x9,#416]": la
 * tabella viene installata SEMPRE, prima di qualunque prova, e viene
 * installata la BASE, non la voce.
 *
 * mir3da_module_detect e' incorporata (il suo corpo compare per la seconda
 * volta a 0xffffff800878b5c8) e il salto di ritorno
 * "17ffffbe b 0xffffff800878b574" e' la riconvergenza col ramo in cui
 * l'indice era gia' valido.
 */
PLAT_HANDLE mir3da_core_init(PLAT_HANDLE handle)
{
	int res;

	p_mir3da_chip_ops = mir3da_chip_ops_tbl;

	/* "37f802c8 tbnz w8, #31, ..." — l'indice e' -1 finche' non si sonda */
	if (mir3da_chip_index < 0) {
		if (mir3da_module_detect(handle)) {
			/* "[MIR3DA] Can't find Mir3da gsensor!!\n"
			 * @0xffffff80091c04af
			 */
			MI_ERR("Can't find Mir3da gsensor!!\n");
			return NULL;
		}
	}

	/* "[MIR3DA] Probe gsensor module: %s\n" @0xffffff80091c04d5, con
	 * "9b2a5101 smaddl x1, w8, w10, x20" come secondo argomento: e' la
	 * voce, cioe' il suo primo campo, il nome.
	 */
	MI_MSG("Probe gsensor module: %s\n",
	       p_mir3da_chip_ops[mir3da_chip_index].asic);

	res = mir3da_chip_resume(handle);
	if (res) {
		/* "[MIR3DA] chip resume fail!!\n\n" @0xffffff80091c0420 —
		 * e' la CODA del letterale di mir3da_cust.c a
		 * 0xffffff80091c041e, che il linker ha fuso.
		 */
		MI_ERR("chip resume fail!!\n\n");
		return NULL;
	}

	return handle;
}

/* ------------------------------------------------------------------ */
/* mir3da_chip_resume @0xffffff800878b6d4, 640 byte — CONFINE           */
/* ------------------------------------------------------------------ */

/*
 * Il ciclo di inizializzazione conta fino a undici e si ferma prima su
 * `addr` negativo:
 *   "11000748 add w8, w26, #0x1" / "12001d09 and w9, w8, #0xff" /
 *   "7100293f cmp w9, #0xa" / "54fffc09 b.ls ..."
 * cioe' l'indice e' a 8 bit e il confronto e' senza segno.
 */
int mir3da_chip_resume(PLAT_HANDLE handle)
{
	struct mir_chip_ops *chip = &p_mir3da_chip_ops[mir3da_chip_index];
	unsigned char reg_data;
	unsigned char addr;
	int res;
	unsigned char i;

	res = p_mir3da_general_ops->smi2c_read(handle, chip->soft_reset.addr,
					       &reg_data);
	if (res == 0) {
		reg_data = (reg_data & ~chip->soft_reset.mask) |
			   (chip->soft_reset.mask & chip->soft_reset.value);
		res = p_mir3da_general_ops->smi2c_write(handle,
						chip->soft_reset.addr,
						reg_data);
	}

	/* "528000a0 mov w0, #0x5" attraverso lo slot msdelay (+80) */
	p_mir3da_general_ops->msdelay(5);

	if (res) {
		/* "[MIR3DA] Do softreset failed !\n" @0xffffff80091c04f8 */
		MI_ERR("Do softreset failed !\n");
		return res;
	}

	for (i = 0; i < 11; i++) {
		if (chip->init_regs[i].addr < 0)
			break;

		res = p_mir3da_general_ops->smi2c_read(handle,
						chip->init_regs[i].addr,
						&reg_data);
		if (res)
			return res;

		reg_data = (reg_data & ~chip->init_regs[i].mask) |
			   (chip->init_regs[i].mask & chip->init_regs[i].value);

		res = p_mir3da_general_ops->smi2c_write(handle,
						chip->init_regs[i].addr,
						reg_data);
		if (res)
			return res;
	}

	/* "52800140 mov w0, #0xa" */
	p_mir3da_general_ops->msdelay(10);

	/* "36f800a8 tbz w8, #31, ..." sull'esito memorizzato */
	if (mir3da_chip_info_res < 0) {
		mir3da_chip_info_res = mir3da_parse_chip_info(handle);
		if (mir3da_chip_info_res < 0) {
			/* "[MIR3DA] Can't parse Mir3da gsensor chipinfo!!\n"
			 * @0xffffff80091c0518
			 */
			MI_ERR("Can't parse Mir3da gsensor chipinfo!!\n");
			return -1;
		}
	}

	if (chip_info.campo_8 == 2) {
		/* "f9401908 ldr x8, [x8,#48]" — lo slot get_address */
		addr = p_mir3da_general_ops->get_address(handle);

		/* "7101311f cmp w8, #0x4c" / "7100991f cmp w8, #0x26" */
		if (addr == 0x4C || addr == 0x26) {
			res = p_mir3da_general_ops->smi2c_read(handle, 0x8C,
							       &reg_data);
			if (res == 0) {
				/* "12001502 and w2, w8, #0x3f" */
				reg_data = reg_data & 0x3F;
				p_mir3da_general_ops->smi2c_write(handle, 0x8C,
								  reg_data);
			}
		}
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_get_primary_offset @0xffffff800878b954, 1052 byte — CONFINE   */
/* ------------------------------------------------------------------ */

/*
 * Nove byte di offset sul frame — "f9000fff str xzr, [sp,#24]" piu'
 * "390083ff strb wzr, [sp,#32]" — letti, poi rimessi a posto alla fine.
 * In mezzo: softreset, i registri di init, cento millisecondi
 * ("52800c80 mov w0, #0x64"), venti campioni ("52800297 mov w23, #0x14")
 * mediati per venti ("9363fd08 asr x8, x8, #35" col magico 0x66666667).
 *
 * Il secondo ciclo, quello che riscrive gli offset, parte da 1 e usa
 * `i - 1` come indice ("51000688 sub w8, w20, #0x1"), e la sua condizione
 * di uscita e' "710022bf cmp w21, #0x8" / "54000068 b.hi ...".
 */
int mir3da_get_primary_offset(PLAT_HANDLE handle, int *x, int *y, int *z)
{
	struct mir_chip_ops *chip = &p_mir3da_chip_ops[mir3da_chip_index];
	unsigned char offset[9] = { 0 };
	unsigned char reg_data;
	short sx, sy, sz;
	int res;
	unsigned char i;
	int j;

	res = mir3da_read_offset(handle, offset);
	if (res) {
		/* "[MIR3DA] Read offset failed !\n" @0xffffff80091c0548 */
		MI_ERR("Read offset failed !\n");
		return -1;
	}

	res = p_mir3da_general_ops->smi2c_read(handle, chip->soft_reset.addr,
					       &reg_data);
	if (res == 0) {
		reg_data = (reg_data & ~chip->soft_reset.mask) |
			   (chip->soft_reset.mask & chip->soft_reset.value);
		res = p_mir3da_general_ops->smi2c_write(handle,
						chip->soft_reset.addr,
						reg_data);
	}

	p_mir3da_general_ops->msdelay(5);

	if (res) {
		MI_ERR("Do softreset failed !\n");
		return -1;
	}

	for (i = 0; i < 11; i++) {
		if (chip->init_regs[i].addr < 0)
			break;

		res = p_mir3da_general_ops->smi2c_read(handle,
						chip->init_regs[i].addr,
						&reg_data);
		if (res == 0) {
			reg_data = (reg_data & ~chip->init_regs[i].mask) |
				   (chip->init_regs[i].mask &
				    chip->init_regs[i].value);
			res = p_mir3da_general_ops->smi2c_write(handle,
						chip->init_regs[i].addr,
						reg_data);
		}

		if (res) {
			/* "[MIR3DA] Write register[0x%x] error!\n"
			 * @0xffffff80091c0567, col registro come argomento:
			 * "79c03521 ldrsh w1, [x9,#26]"
			 */
			MI_ERR("Write register[0x%x] error!\n",
			       chip->init_regs[i].addr);

			for (j = 1; j <= 9; j++) {
				if (chip->offset_regs[j - 1].addr < 0)
					return -1;
				if (p_mir3da_general_ops->smi2c_write(handle,
						chip->offset_regs[j - 1].addr,
						offset[j - 1]))
					return -1;
			}

			return -1;
		}
	}

	p_mir3da_general_ops->msdelay(100);

	*z = 0;
	*y = 0;
	*x = 0;

	for (i = 0; i < 20; i++) {
		sx = 0;
		sy = 0;
		sz = 0;

		mir3da_read_raw_data(handle, &sx, &sy, &sz);

		*x = *x + sx;
		*y = *y + sy;
		*z = *z + sz;

		p_mir3da_general_ops->msdelay(5);
	}

	*x = *x / 20;
	*y = *y / 20;
	*z = *z / 20;

	for (j = 1; j <= 9; j++) {
		if (chip->offset_regs[j - 1].addr < 0)
			break;

		res = p_mir3da_general_ops->smi2c_write(handle,
						chip->offset_regs[j - 1].addr,
						offset[j - 1]);
		if (res)
			break;
	}

	if (chip_info.reg_c0 == 0x4B || chip_info.reg_c0 == 0x8C ||
	    chip_info.reg_c0 == 0xCA || chip_info.campo_12 == 5)
		*z = 0;

	return 0;
}

/* ------------------------------------------------------------------ */
/* NSA_NTO_calibrate    @0xffffff800878be88, 8 byte (statica)           */
/* NSA_NTO_auto_calibrate @0xffffff800878be90, 8 byte (statica)         */
/* ------------------------------------------------------------------ */

/*
 * Due istruzioni ciascuna, identiche:
 *   "2a1f03e0 mov w0, wzr"
 *   "d65f03c0 ret"
 * Nessuna funzione del kernel di fabbrica le chiama, ne' direttamente ne'
 * per gli slot +152/+160 della tabella.  La loro firma non e' osservabile:
 * vedi il cappello, §4c.  Non sono stub scritti per far linkare — sono la
 * riproduzione esatta di due corpi che di fabbrica sono cosi'.
 */
static int NSA_NTO_calibrate(void)
{
	return 0;
}

static int NSA_NTO_auto_calibrate(void)
{
	return 0;
}

/* ------------------------------------------------------------------ */
/* NSA_interrupt_ops @0xffffff800878be98, 1064 byte (statica)           */
/* ------------------------------------------------------------------ */

/*
 * Uno switch a quattro casi con tabella di salto a 0xffffff8008f54175
 * (byte 00 2e 3f 6a, base 0xffffff800878bee8), piu' il ramo di default:
 *   "71000d1f cmp w8, #0x3" / "540005a8 b.hi ..."
 *
 * In QUESTO kernel il percorso e' morto: l'unico ponte verso questo
 * puntatore e' mir3da_interrupt_ops, che nessuna funzione chiama.  E'
 * scritta perche' il suo indirizzo sta nella tabella, e senza la
 * definizione il link non riesce.
 */
static int NSA_interrupt_ops(PLAT_HANDLE handle, void *arg)
{
	struct mir3da_int_ops_s *ops = arg;
	unsigned char reg_data;
	unsigned char t;

	switch (ops->op) {
	case 0:
		if (p_mir3da_general_ops->smi2c_read(handle, 0x21,
						     &reg_data) == 0) {
			/* "33000ec2 bfxil w2, w22, #0, #4" */
			reg_data = (reg_data & ~0x0F) |
				   (ops->campo_8b[1] & 0x0F);
			p_mir3da_general_ops->smi2c_write(handle, 0x21,
							  reg_data);
		}

		if (p_mir3da_general_ops->smi2c_read(handle, 0x20, &reg_data))
			return 0;

		/* "2a1406c8 orr w8, w22, w20, lsl #1" /
		 * "2a160908 orr w8, w8, w22, lsl #2" /
		 * "2a140d08 orr w8, w8, w20, lsl #3"
		 */
		t = ops->campo_8b[0] | (ops->campo_4 << 1) |
		    (ops->campo_8b[0] << 2) | (ops->campo_4 << 3);
		reg_data = (reg_data & ~0x0F) | (t & 0x0F);
		p_mir3da_general_ops->smi2c_write(handle, 0x20, reg_data);
		break;

	case 1:
		if (ops->campo_4 == 2) {
			if (p_mir3da_general_ops->smi2c_read(handle, 0x16,
							     &reg_data))
				return 0;
			/* "321c0502 orr w2, w8, #0x30" */
			reg_data = reg_data | 0x30;
			p_mir3da_general_ops->smi2c_write(handle, 0x16,
							  reg_data);
		} else if (ops->campo_4 == 1) {
			if (p_mir3da_general_ops->smi2c_read(handle, 0x16,
							     &reg_data))
				return 0;
			/* "32000902 orr w2, w8, #0x7" */
			reg_data = reg_data | 0x07;
			p_mir3da_general_ops->smi2c_write(handle, 0x16,
							  reg_data);
		}
		break;

	case 2:
		if (ops->campo_8 == 2) {
			if (p_mir3da_general_ops->smi2c_read(handle, 0x2B,
							     &reg_data) == 0) {
				/* "330012c2 bfxil w2, w22, #0, #5" */
				reg_data = (reg_data & ~0x1F) |
					   (ops->campo_12 & 0x1F);
				p_mir3da_general_ops->smi2c_write(handle, 0x2B,
								  reg_data);
			}

			if (p_mir3da_general_ops->smi2c_read(handle, 0x2A,
							     &reg_data) == 0) {
				/* "2a161ae8 orr w8, w23, w22, lsl #6" e le
				 * due maschere "12800cea mov w10, #0xffffff98"
				 * e "52800ceb mov w11, #0x67"
				 */
				t = ops->campo_15 | (ops->campo_13 << 6);
				reg_data = (reg_data & ~0x67) | (t & 0x67);
				p_mir3da_general_ops->smi2c_write(handle, 0x2A,
								  reg_data);
			}

			if (ops->campo_4 == 1) {
				if (p_mir3da_general_ops->smi2c_read(handle,
							0x1B, &reg_data))
					return 0;
				reg_data = reg_data | 0x30;
				p_mir3da_general_ops->smi2c_write(handle, 0x1B,
								  reg_data);
			} else if (ops->campo_4 == 0) {
				if (p_mir3da_general_ops->smi2c_read(handle,
							0x19, &reg_data))
					return 0;
				reg_data = reg_data | 0x30;
				p_mir3da_general_ops->smi2c_write(handle, 0x19,
								  reg_data);
			}
		} else if (ops->campo_8 == 1) {
			p_mir3da_general_ops->smi2c_write(handle, 0x28,
							  ops->campo_12);

			if (p_mir3da_general_ops->smi2c_read(handle, 0x27,
							     &reg_data) == 0) {
				/* "330006c2 bfxil w2, w22, #0, #2" */
				reg_data = (reg_data & ~0x03) |
					   (ops->campo_13 & 0x03);
				p_mir3da_general_ops->smi2c_write(handle, 0x27,
								  reg_data);
			}

			if (ops->campo_4 == 1) {
				if (p_mir3da_general_ops->smi2c_read(handle,
							0x1B, &reg_data))
					return 0;
				/* "321e0102 orr w2, w8, #0x4" */
				reg_data = reg_data | 0x04;
				p_mir3da_general_ops->smi2c_write(handle, 0x1B,
								  reg_data);
			} else if (ops->campo_4 == 0) {
				if (p_mir3da_general_ops->smi2c_read(handle,
							0x19, &reg_data))
					return 0;
				reg_data = reg_data | 0x04;
				p_mir3da_general_ops->smi2c_write(handle, 0x19,
								  reg_data);
			}
		}
		break;

	case 3:
		if (ops->campo_4 == 2) {
			if (p_mir3da_general_ops->smi2c_read(handle, 0x16,
							     &reg_data))
				return 0;
			/* "121a7502 and w2, w8, #0xffffffcf" */
			reg_data = reg_data & ~0x30;
			p_mir3da_general_ops->smi2c_write(handle, 0x16,
							  reg_data);
		} else if (ops->campo_4 == 1) {
			if (p_mir3da_general_ops->smi2c_read(handle, 0x16,
							     &reg_data))
				return 0;
			/* "121d7102 and w2, w8, #0xfffffff8" */
			reg_data = reg_data & ~0x07;
			p_mir3da_general_ops->smi2c_write(handle, 0x16,
							  reg_data);
		}
		break;

	default:
		/* "[MIR3DA] Unsupport operation !\n" @0xffffff80091c0652 */
		MI_ERR("Unsupport operation !\n");
		break;
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* NSA_get_reg_data @0xffffff800878c2c0, 280 byte (statica)             */
/* ------------------------------------------------------------------ */

/*
 * Dump di 211 registri, 0x00..0xD2 —
 *   "110006d6 add w22, w22, #0x1" / "71034edf cmp w22, #0xd3"
 * — a sedici per riga: "72000edf tst w22, #0xf".  L'esito della lettura
 * NON viene controllato (nessun `cbnz` dopo la `blr`), di fabbrica.
 * Le tre stringhe: "---------start---------" @0xffffff80091c0672,
 * "\n%02x\t" @0xffffff80091c068a, "%02X " @0xffffff80091b570b,
 * "\n--------end---------\n" @0xffffff80091c0691.
 */
static int NSA_get_reg_data(PLAT_HANDLE handle, char *buf)
{
	unsigned char reg_data;
	int len;
	int n;
	int i;

	len = p_mir3da_general_ops->mysprintf(buf, "---------start---------");

	for (i = 0; i < 0xD3; i++) {
		if ((i & 0xF) == 0)
			len += p_mir3da_general_ops->mysprintf(buf + len,
							       "\n%02x\t", i);

		p_mir3da_general_ops->smi2c_read(handle, i, &reg_data);

		len += p_mir3da_general_ops->mysprintf(buf + len, "%02X ",
						       reg_data);
	}

	n = p_mir3da_general_ops->mysprintf(buf + len,
					    "\n--------end---------\n");

	return n + len;
}

/*
 * ==================================================================== *
 * LE DIECI CHE MANCAVANO                                               *
 * ==================================================================== *
 *
 * Il lotto del 2026-08-17 (docs/bringup/rapporti/rapporto-mir3da-core.md
 * §1) ne ha scritte 19 su 29 e ha lasciato fuori queste dieci, dicendo
 * perche': sono tutte `T` globali, non compaiono negli errori del linker e
 * il driver chiude senza le loro definizioni.  La stessa nota diceva anche
 * che «il disassemblato di tutte e dieci e' chiaro quanto quello delle
 * diciannove», e infatti.
 *
 * SONO IN CODA E NON IN ORDINE DI INDIRIZZO, ed e' una scelta dichiarata:
 * il resto del file segue l'ordine della mappa, e interlacciarle vorrebbe
 * dire spostare 1100 righe gia' riviste per una ragione che non riguarda
 * il loro contenuto.  L'ordine delle funzioni dentro un `.o` non entra in
 * nessuna delle misure di questo progetto -- si misura la DIMENSIONE di
 * ciascuna -- e questo file non ha vincoli di `__LINE__`, perche' il core
 * non chiama mai `printk` e nessuna sua stringa porta un numero di riga.
 */

/* ------------------------------------------------------------------ */
/* mir3da_register_read @0xffffff800878a0f4, 32 byte                    */
/* ------------------------------------------------------------------ */

/*
 * Un ponte, niente altro: chiama lo slot +0 di `general_op_s` e restituisce
 * quello che ha restituito lui.
 *   "f940cd08 ldr x8, [x8,#408]"   p_mir3da_general_ops
 *   "f9400108 ldr x8, [x8]"        lo slot smi2c_read (+0)
 *   "d63f0100 blr x8"
 * I tre argomenti passano intatti in x0/w1/x2: nessuna istruzione li tocca
 * fra l'ingresso e la chiamata, ed e' cio' che rende questa una `tail call`
 * in tutto tranne il nome (clang tiene il frame per il `bl`).
 */
int mir3da_register_read(PLAT_HANDLE handle, u8 addr, u8 *data)
{
	return p_mir3da_general_ops->smi2c_read(handle, addr, data);
}

/* ------------------------------------------------------------------ */
/* mir3da_register_read_continuously @0xffffff800878a114, 52 byte       */
/* ------------------------------------------------------------------ */

/*
 * Lo stesso ponte sullo slot +16, ma il valore di ritorno NON passa: la
 * lettura riesce se ha letto ESATTAMENTE `count` byte, e la funzione
 * restituisce 0 o 1.
 *   "f9400908 ldr x8, [x8,#16]"    lo slot smi2c_read_block
 *   "2a0203f3 mov w19, w2"         `count` messo da parte prima
 *   "6b33001f cmp w0, w19, uxtb"   il confronto e' a OTTO BIT
 *   "1a9f07e0 cset w0, ne"         1 se diverso, 0 se uguale
 *
 * Il `uxtb` sul secondo operando e' il fatto interessante: `count` e'
 * confrontato ESTESO DA UN BYTE, il che dice che il parametro e' un `u8` --
 * un `int` darebbe un confronto pieno.  E' la stessa classe di prova del
 * `sxtw` di ilitek_bus.c: la larghezza di un parametro letta dal confronto
 * invece che indovinata.
 */
int mir3da_register_read_continuously(PLAT_HANDLE handle, u8 addr, u8 count,
				      u8 *data)
{
	return count != p_mir3da_general_ops->smi2c_read_block(handle, addr,
							       count, data);
}

/* ------------------------------------------------------------------ */
/* mir3da_register_mask_write @0xffffff800878a168, 164 byte             */
/* ------------------------------------------------------------------ */

/*
 * Leggi-modifica-scrivi su un registro, con maschera.  Il buffer di lettura
 * sta sulla pila ("910013e2 add x2, sp, #0x4") e c'e' il canarino
 * ("f9454508 ldr x8, [x8,#2696]" all'ingresso,
 *  "97e49016 bl <__stack_chk_fail>" in coda), che e' come si riconosce un
 * `unsigned char` locale il cui indirizzo esce dalla funzione.
 *
 * La modifica e' tre istruzioni, e vanno lette insieme:
 *   "0a15028a and w10, w20, w21"   data & mask
 *   "0a350108 bic w8, w8, w21"     tmp & ~mask
 *   "2a0a0102 orr w2, w8, w10"     l'unione
 * `bic` e' `and` col secondo operando negato: e' la forma che il
 * compilatore usa per `x & ~y` quando `y` e' in un registro, e non lascia
 * ambiguita' sul segno dell'operazione.
 *
 * Sul fallimento della lettura ("35000160 cbnz w0, ...") la scrittura NON
 * avviene e la funzione restituisce quello che ha restituito la lettura:
 * il ramo salta direttamente all'epilogo senza toccare w0.
 */
int mir3da_register_mask_write(PLAT_HANDLE handle, u8 addr, u8 mask, u8 data)
{
	int res;
	/*
	 * NON AZZERATA, e il binario lo dice: fra il prologo e la
	 * "d63f0100 blr"@0xffffff800878a1ac che chiama `smi2c_read` la fabbrica
	 * non scrive niente nello slot che le passa -- "910013e2 add"@0xffffff800878a190
	 * mette x2 = sp+4, e l'unica scrittura la fa la lettura stessa
	 * ("394013e8 ldrb"@0xffffff800878a1b4 la rilegge dopo). Da noi `= 0`
	 * emetteva una "390013ff strb wzr, [sp,#4]" in piu': quattro byte, ed
	 * erano esattamente i quattro che separavano questa funzione dai 164 di
	 * fabbrica.
	 *
	 * DIFETTO DELLA FABBRICA, riprodotto: se `smi2c_read` non scrive nulla e
	 * torna zero, `tmp` resta indefinita e viene usata lo stesso. E' lo
	 * stesso difetto, nella stessa forma, di `aw87329_set_hwen`.
	 */
	u8 tmp;

	res = p_mir3da_general_ops->smi2c_read(handle, addr, &tmp);
	if (res)
		return res;

	tmp &= ~mask;
	tmp |= data & mask;

	return p_mir3da_general_ops->smi2c_write(handle, addr, tmp);
}

/* ------------------------------------------------------------------ */
/* mir3da_direction_remap @0xffffff800878a20c, 124 byte                 */
/* ------------------------------------------------------------------ */

/*
 * La tabella delle otto orientazioni sta in `.rodata` a
 * 0xffffff8008f5417c ("9105f129 add x9, x9, #0x17c"), indicizzata con un
 * passo di SEDICI byte ("8b23d129 add x9, x9, w3, sxtw #4"), e i suoi byte
 * si leggono:
 *
 *   [0] (0, 0, 0, 0)      [4] (1, 0, 1, 0)
 *   [1] (0, 1, 0, 1)      [5] (0, 0, 1, 1)
 *   [2] (1, 1, 0, 0)      [6] (0, 1, 1, 0)
 *   [3] (1, 0, 0, 1)      [7] (1, 1, 1, 1)
 *
 * Quattro `int`, e i primi tre sono i segni: il codice li usa come
 *   "1b0a7d0a mul w10, w8, w10"    v * s
 *   "4b0a0508 sub w8, w8, w10, lsl #1"   v - 2*(v*s)
 * che vale `v` per s=0 e `-v` per s=1.  I tre load sono `ldrh` -- meta'
 * bassa di ogni `int` -- perche' i valori sono 0 o 1 e clang restringe.
 *
 * DUE COLONNE NON SONO LETTE DALLA TABELLA, e questo e' il punto:
 *
 *   - lo SCAMBIO x/y esce da una costante, "5280154b mov w11, #0xaa" piu'
 *     "9ac3256a lsr x10, x11, x3" e "360000aa tbz w10, #0": 0xAA ha i bit
 *     dispari accesi, ed e' ESATTAMENTE la quarta colonna della tabella.
 *     clang ha visto che quella colonna vale `indice & 1` e l'ha sostituita
 *     con una maschera.
 *   - il RITORNO esce da un test di intervallo, "927ef508 and x8, x8,
 *     #0xfffffffffffffffc" piu' "f100111f cmp x8, #0x4" e
 *     "5a880500 cneg w0, w8, ne" con w8 = -1: vale -1 per indice 4..7 e +1
 *     altrimenti, che e' ESATTAMENTE la terza colonna.
 *
 * Sono due letture di tabella che il compilatore ha convertito in
 * aritmetica perche' la tabella e' costante e piccola.  Scriverle come
 * costanti qui sarebbe piu' corto e sbagliato: nasconderebbe che la fonte
 * e' la stessa riga di tabella.
 */
struct mir3da_direction {
	int sign_x;
	int sign_y;
	int sign_z;
	int swap_xy;
};

static const struct mir3da_direction mir3da_dir_tbl[8] = {
	{ 0, 0, 0, 0 },
	{ 0, 1, 0, 1 },
	{ 1, 1, 0, 0 },
	{ 1, 0, 0, 1 },
	{ 1, 0, 1, 0 },
	{ 0, 0, 1, 1 },
	{ 0, 1, 1, 0 },
	{ 1, 1, 1, 1 },
};

int mir3da_direction_remap(short *x, short *y, short *z, int direction)
{
	const struct mir3da_direction *d = &mir3da_dir_tbl[direction];
	short tmp;

	/* "79400008 ldrh w8, [x0]" ... "79000008 strh w8, [x0]" */
	*x = *x - 2 * (*x * d->sign_x);
	/* "79400028 ldrh w8, [x1]" ... "79000028 strh w8, [x1]" */
	*y = *y - 2 * (*y * d->sign_y);
	/* "7940004a ldrh w10, [x2]" ... "79000049 strh w9, [x2]" */
	*z = *z - 2 * (*z * d->sign_z);

	if (d->swap_xy) {
		/* "79400029 ldrh w9, [x1]" e "7940000a ldrh w10, [x0]" */
		tmp = *x;
		*x = *y;
		*y = tmp;
	}

	return d->sign_z ? -1 : 1;
}

/* ------------------------------------------------------------------ */
/* cycle_read_xyz @0xffffff800878abb0, 288 byte                         */
/* ------------------------------------------------------------------ */

/*
 * Media di `count` letture grezze.  I tre accumulatori sono azzerati PRIMA
 * di guardare `count` ("b900007f str wzr, [x3]",
 * "b900005f str wzr, [x2]", "b900003f str wzr, [x1]", e solo dopo
 * "34000404 cbz w4, ..."), quindi con count = 0 escono a zero e la
 * divisione avviene lo stesso -- vedi sotto.
 *
 * Dentro il ciclo, le tre variabili corte sulla pila sono riazzerate a ogni
 * giro ("79001bff strh wzr, [sp,#12]" e le due sorelle) e sommate con
 * ESTENSIONE DI SEGNO ("79c02be8 ldrsh w8, [sp,#20]"): sono `short`.
 * Fra un giro e l'altro c'e' "f9402908 ldr x8, [x8,#80]" con
 * "528000a0 mov w0, #0x5", cioe' `msdelay(5)`.
 *
 * LA DIVISIONE PER ZERO NON E' PROTETTA, e si riproduce.  Il ramo con
 * count = 0 salta a "2a1f03e8 mov w8, wzr" e cade sulla stessa
 * "1ad40d08 sdiv w8, w8, w20" degli altri: `sdiv` per zero su aarch64 da'
 * zero e non solleva, quindi di fabbrica il caso e' silenzioso. Aggiungere
 * una guardia qui sarebbe codice che nel binario non c'e'.
 */
int cycle_read_xyz(PLAT_HANDLE handle, int *x, int *y, int *z, int count)
{
	short raw_x, raw_y, raw_z;
	/*
	 * `i` E' SENZA SEGNO, e la guardia del ciclo lo prova. La fabbrica entra
	 * nel ciclo con "34000404 cbz"@0xffffff800878abf4 -- un salto su
	 * `count == 0` -- mentre con un `int` clang deve emettere
	 * "7100049f cmp w4, #1" piu' un `b.lt`, perche' un conteggio negativo
	 * e' possibile. Quei quattro byte erano tutta la differenza.
	 *
	 * Il tipo del PARAMETRO resta `int`: quello lo fissa il chiamante, ed e'
	 * l'indice che il confronto senza segno svela.
	 */
	unsigned int i;

	*z = 0;
	*y = 0;
	*x = 0;

	for (i = 0; i < count; i++) {
		raw_z = 0;
		raw_y = 0;
		raw_x = 0;

		/* "97fffddf bl <mir3da_read_raw_data>" */
		mir3da_read_raw_data(handle, &raw_x, &raw_y, &raw_z);

		*x += raw_x;
		*y += raw_y;
		*z += raw_z;

		/* "f9402908 ldr x8, [x8,#80]" -- lo slot msdelay */
		p_mir3da_general_ops->msdelay(5);
	}

	/* "1ad40d08 sdiv w8, w8, w20", tre volte */
	*x = *x / count;
	*y = *y / count;
	*z = *z / count;

	/* "2a1f03e0 mov w0, wzr" */
	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_write_offset @0xffffff800878aebc, 144 byte                    */
/* ------------------------------------------------------------------ */

/*
 * Scrive i nove registri di offset del chip, uno per giro.  L'indirizzo di
 * ciascuno esce dalla tabella dei descrittori:
 *   "f940d2a9 ldr x9, [x21,#416]"      p_mir3da_chip_ops
 *   "b98d9eca ldrsw x10, [x22,#3484]"  mir3da_chip_index, CON SEGNO
 *   "9b172549 madd x9, x10, x23, x9"   x 184
 *   "8b28c929 add x9, x9, w8, sxtw #2" x 4, il passo di mir_reg_obj
 *   "79c08d21 ldrsh w1, [x9,#70]"      offset_regs[i].addr, CON SEGNO
 *
 * Il ciclo si ferma per DUE ragioni diverse, e vanno tenute distinte:
 *   - "37f80161 tbnz w1, #31" -- l'indirizzo e' negativo, cioe' la
 *     sentinella -1: si esce restituendo ZERO
 *     ("2a1f03e0 mov w0, wzr"), come se fosse riuscito;
 *   - "350000a0 cbnz w0, ..." -- la scrittura e' fallita: si esce
 *     restituendo l'errore, senza passare per il `mov w0, wzr`.
 * Il limite superiore e' NOVE, provato senza segno
 * ("7100211f cmp w8, #0x8" con "54fffe29 b.ls"), quindi i valori ammessi
 * sono 0..8.
 */
int mir3da_write_offset(PLAT_HANDLE handle, u8 *offset)
{
	int i, res;

	for (i = 0; i < 9; i++) {
		/*
		 * I DUE GLOBALI SI RILEGGONO A OGNI GIRO, e non e' un
		 * dettaglio: le due `ldr` stanno DENTRO il ciclo, alla sua
		 * testa (0xffffff800878aef0 e 0xffffff800878aef4), non prima.
		 * Estrarli in un locale davanti al `for` -- che e' la forma
		 * piu' naturale da scrivere -- permette a clang di SROTOLARE
		 * tutte e nove le iterazioni, e la funzione passa da 144 a
		 * 436 byte. Misurato: e' stata la prima stesura.
		 * Riletti dal globale la funzione scende a 148 byte, e il
		 * ciclo torna a essere un ciclo.
		 *
		 * DIVERGENZA APERTA, E NON AGGIUSTATA. Con
		 * `mir3da_chip_index` non piu' `static` (vedi il suo
		 * commento) clang torna a srotolare e la funzione risale a
		 * 492 byte contro i 144 di fabbrica. Le tre forme misurate:
		 *   locale + static      436
		 *   globale + static     148
		 *   globale + non-static 492
		 * Un indice `unsigned` -- suggerito dal "54fffe29 b.ls" del
		 * binario, che confronta senza segno -- e' stato provato e
		 * NON cambia niente: sempre 492. La forma resta quella che il
		 * disassemblato descrive; il numero e' peggiore, e si
		 * riporta. Il `sxtw` di "93407d19 sxtw x25, w8" dice comunque
		 * che l'indice e' un `int`, ed e' `int`.
		 */
		if (p_mir3da_chip_ops[mir3da_chip_index].offset_regs[i].addr < 0)
			break;

		/* "38796a62 ldrb w2, [x19,x25]" -- offset[i] */
		res = p_mir3da_general_ops->smi2c_write(handle,
			p_mir3da_chip_ops[mir3da_chip_index].offset_regs[i].addr,
			offset[i]);
		if (res)
			return res;
	}

	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_calibrate @0xffffff800878af4c, 8 byte                         */
/* ------------------------------------------------------------------ */

/*
 * Due istruzioni: "2a1f03e0 mov w0, wzr" e "d65f03c0 ret".  Non tocca
 * niente e restituisce zero.
 *
 * LA FIRMA NON E' MISURATA e non lo puo' essere: nessun argomento viene
 * letto, e nessun chiamante esiste nel kernel di fabbrica.  Quella scritta
 * qui e' quella delle due sorelle a +152 e +160 della tabella
 * (`NSA_NTO_calibrate`, `NSA_NTO_auto_calibrate`), che hanno lo stesso
 * corpo e la stessa sorte.  E' una SCELTA per simmetria, non una misura.
 */
int mir3da_calibrate(PLAT_HANDLE handle, int z_dir)
{
	return 0;
}

/* ------------------------------------------------------------------ */
/* mir3da_interrupt_ops @0xffffff800878af54, 48 byte                    */
/* ------------------------------------------------------------------ */

/*
 * Ponte verso lo slot +168 della voce di tabella del chip corrente, cioe'
 * `NSA_interrupt_ops` per l'unica voce che esiste.  E' la gemella di
 * `mir3da_get_reg_data`, che fa la stessa cosa con +176.
 *   "f940d108 ldr x8, [x8,#416]"       p_mir3da_chip_ops
 *   "b98d9d29 ldrsw x9, [x9,#3484]"    mir3da_chip_index
 *   "9b0a2128 madd x8, x9, x10, x8"    x 184
 *   "f9405508 ldr x8, [x8,#168]"       lo slot interrupt_ops
 *   "d63f0100 blr x8"
 * I due argomenti passano intatti: nessuna istruzione tocca x0 o x1.
 */
int mir3da_interrupt_ops(PLAT_HANDLE handle, void *ops)
{
	return p_mir3da_chip_ops[mir3da_chip_index].interrupt_ops(handle, ops);
}

/* ------------------------------------------------------------------ */
/* mir3da_set_odr @0xffffff800878b134, 208 byte                         */
/* ------------------------------------------------------------------ */

/*
 * Sceglie una delle TRE voci di `odr_regs` in base al periodo richiesto e
 * ci fa sopra una lettura-modifica-scrittura identica a quella di
 * `mir3da_register_mask_write`.
 *
 * La scelta dell'indice e' due confronti con segno, e l'ordine conta:
 *   "71002c3f cmp w1, #0xb"    delay < 11 ?
 *   "1a9fa7eb cset w11, lt"    -> 1, altrimenti 0
 *   "7100183f cmp w1, #0x6"    delay < 6 ?
 *   "321f03ec orr w12, wzr, #0x2"
 *   "9a8bb18b csel x11, x12, x11, lt"  -> 2, altrimenti quello di prima
 * cioe' 2 sotto 6, 1 fra 6 e 10, 0 da 11 in su: piu' e' corto il periodo,
 * piu' alta e' la frequenza.
 *
 * I tre campi della voce escono da tre load di larghezza diversa, ed e' la
 * prova che `mir_reg_obj` e' fatta come dice la sua definizione:
 *   "7940d533 ldrh w19, [x9,#106]"   addr, sedici bit
 *   "3941b135 ldrb w21, [x9,#108]"   mask, otto
 *   "3941b536 ldrb w22, [x9,#109]"   value, otto
 *
 * `p_mir3da_general_ops` e la tabella sono caricati con UN SOLO `ldp`
 * ("a9402528 ldp x8, x9, [x9]" su "91066129 add x9, x9, #0x198"), perche'
 * i due puntatori sono adiacenti in `.bss` a 0x9cba198 e 0x9cba1a0.
 */
int mir3da_set_odr(PLAT_HANDLE handle, int delay)
{
	struct mir_reg_obj *odr;
	int idx, res;
	short addr;
	u8 mask, value;
	u8 tmp;

	/*
	 * E' L'INDICE a essere scelto, non il puntatore: il binario fa i due
	 * `cset`/`csel` su un intero e poi UNA sola aritmetica di indirizzo
	 * ("9b0c2549 madd" piu' "8b0b0929 add x9, x9, x11, lsl #2").
	 * Scegliendo fra tre puntatori distinti clang produce tre percorsi.
	 */
	if (delay < 6)
		idx = 2;
	else if (delay < 11)
		idx = 1;
	else
		idx = 0;

	odr = &p_mir3da_chip_ops[mir3da_chip_index].odr_regs[idx];

	/*
	 * I TRE CAMPI SI LEGGONO PRIMA della chiamata e vivono in registri
	 * salvati attraverso di essa -- w19 (addr), w21 (mask), w22 (value):
	 * "7940d533 ldrh w19, [x9,#106]", "3941b135 ldrb w21, [x9,#108]",
	 * "3941b536 ldrb w22, [x9,#109]" stanno tutte e tre PRIMA di
	 * "d63f0100 blr x8", e dopo la chiamata nessuna rilettura li tocca.
	 * Lasciandoli come accessi a `odr->` dopo la chiamata, clang li
	 * ricarica -- non puo' escludere che la chiamata indiretta abbia
	 * scritto la tabella -- e la funzione cresce.
	 */
	addr = odr->addr;
	mask = odr->mask;
	value = odr->value;

	res = p_mir3da_general_ops->smi2c_read(handle, addr, &tmp);
	if (res)
		return res;

	tmp &= ~mask;
	tmp |= value & mask;

	return p_mir3da_general_ops->smi2c_write(handle, addr, tmp);
}

/* ------------------------------------------------------------------ */
/* mir3da_temp_calibrate_detect_static @0xffffff800878bd70, 280 byte    */
/* ------------------------------------------------------------------ */

/*
 * L'altra meta' della macchina descritta al §3 del cappello: conta da
 * quanti campioni consecutivi il sensore e' fermo.
 *
 * IL PRIMO CAMPIONE E' UN CASO A SE'.  "39472109 ldrb w9, [x8,#456]" legge
 * `have_last` e "7100053f cmp w9, #0x1" lo confronta con uno; se NON e'
 * ancora uno, il ramo a 0xffffff800878bd9c salva i tre valori e mette
 * `have_last` a 1 ("3907210c strb w12, [x8,#456]"), e il confronto che
 * segue viene fatto CON SE STESSI -- w8/w9/w10 sono gli argomenti, non i
 * vecchi campioni. La differenza e' quindi zero e il primo giro conta
 * sempre come "fermo".  E' cio' che il binario fa.
 *
 * La distanza e' la somma dei tre valori assoluti, ognuno con la stessa
 * coppia:
 *   "4b2aa16a sub w10, w11, w10, sxth"   differenza a 16 bit con segno
 *   "5a8a554a cneg w10, w10, mi"         valore assoluto
 * e la soglia e' SESSANTA ("7100f03f cmp w1, #0x3c").
 *
 * Il contatore non e' un semplice `++`:
 *   "1a8aa7e2 csinc w2, wzr, w10, ge"
 * vale zero se la distanza e' >= 60, e `count_static + 1` altrimenti --
 * cioe' azzera al primo movimento invece di decrementare.
 *
 * La traccia e' l'unica del blocco che nomina i propri valori:
 *   "[MIR3DA] delta_sum=%d count_static=%d\n"@0xffffff80091c05ae
 * ed e' `MI_MSG`, guardata dal bit 2 di `Log_level`
 * ("3610010c tbz w12, #2, ..."). Dopo la chiamata il contatore viene
 * RILETTO dalla memoria ("b941ba62 ldr w2, [x19,#440]"), non tenuto in un
 * registro: e' quello che rende `count_static` una variabile vera e non un
 * temporaneo.
 *
 * Il tetto e' DIECI ("7100285f cmp w2, #0xa" con "b901ba68 str w8,
 * [x19,#440]" che ci riscrive esattamente 10): il contatore si ferma li' e
 * la funzione restituisce 1.
 */
int mir3da_temp_calibrate_detect_static(short x, short y, short z)
{
	int delta_sum;

	/* "39472109 ldrb w9, [x8,#456]" */
	if (have_last != 1) {
		/* "79037920 strh w0, [x9,#444]" e le due sorelle */
		last_x = x;
		last_y = y;
		last_z = z;
		/* "3907210c strb w12, [x8,#456]" */
		have_last = 1;
	}

	/*
	 * "4b2aa16a sub w10, w11, w10, sxth" piu'
	 * "5a8a554a cneg w10, w10, mi", tre volte, e le due somme
	 * "0b0a0129 add w9, w9, w10" e "0b080121 add w1, w9, w8".
	 */
	delta_sum = abs(x - last_x) + abs(y - last_y) + abs(z - last_z);

	/* "79037960 strh w0, [x11,#444]" e le due sorelle: si aggiorna DOPO */
	last_x = x;
	last_y = y;
	last_z = z;

	/*
	 * "7100f03f cmp w1, #0x3c" piu' "1a8aa7e2 csinc w2, wzr, w10, ge":
	 * zero se ha superato la soglia, altrimenti uno in piu'.
	 */
	if (delta_sum >= 60)
		count_static = 0;
	else
		count_static = count_static + 1;

	/* "[MIR3DA] delta_sum=%d count_static=%d\n"@0xffffff80091c05ae */
	MI_MSG("delta_sum=%d count_static=%d\n", delta_sum, count_static);

	/* "7100285f cmp w2, #0xa" -- il tetto */
	if (count_static >= 10) {
		count_static = 10;
		return 1;
	}

	return 0;
}
