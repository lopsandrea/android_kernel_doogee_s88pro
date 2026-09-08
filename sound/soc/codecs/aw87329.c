// SPDX-License-Identifier: GPL-2.0
/*
 * Amplificatore di potenza audio Awinic AW87329 del Doogee S88 Pro.
 *
 * Ricostruito leggendo il kernel di fabbrica disassemblato, non adattato da
 * un altro telefono. Ogni registro e ogni costante di questo file ha la sua
 * riga di disassemblato citata in docs/bringup/driver-riscritti.md: se un
 * valore non e' li', non e' stato letto ed e' un difetto.
 *
 * Quello che non era chiaro dal disassemblato e' rimasto fuori, elencato nel
 * registro. Non e' stato completato per somiglianza.
 */
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

/*
 * Struttura dati del driver, dedotta dagli offset che le funzioni del
 * lotto 1 e del lotto 2 leggono e scrivono su g_aw87329.
 *
 * L'offset 0xc, lasciato "non identificato" dal lotto 1 (nessuna delle
 * cinque funzioni di quel lotto lo toccava), e' stato identificato qui:
 * aw87329_audio_kspk/_drcv/_abrcv/_off lo controllano tutte per prime
 * (ldrb w9,[x8,#12]; cbz -> "%s: aw87329 init failed\n") -- e' un flag
 * "il driver e' inizializzato", non un segnaposto.
 */
struct aw87329 {
	struct i2c_client *i2c;
	/*
	 * Offset 0x8. Numero di GPIO usato per accendere/spegnere/
	 * resettare il chip. Confrontato con 0x1ff (511) e passato
	 * a gpio_to_desc(): vedi aw87329_hw_on/_hw_off/_hw_reset.
	 */
	int hwen_gpio;
	/*
	 * Offset 0xc. Identificato nel lotto 2 (vedi sopra): diverso da
	 * zero quando il driver e' stato inizializzato con successo.
	 */
	u8 inited;
	/*
	 * Offset 0xd. Flag "chip acceso": 1 dopo hw_on/hw_reset riusciti,
	 * 0 dopo hw_off o dopo un hw_reset fallito.
	 */
	u8 hwen_flag;
	/*
	 * Offset 0xe/0xf/0x10, identificati nel lotto 2. Un flag per
	 * modalita': diverso da zero se la tabella dei registri di quella
	 * modalita' e' stata caricata da fuori (aw87329_kspk_reg/_drcv_reg/
	 * _abrcv_reg, sotto), altrimenti si usa la tabella di default in
	 * .rodata. Vedi aw87329_kspk_reg_val e le sue due sorelle.
	 */
	u8 kspk_reg_loaded;
	u8 drcv_reg_loaded;
	u8 abrcv_reg_loaded;
	/*
	 * Identificati nel lotto 3, da aw87329_i2c_probe: sizeof(struct
	 * aw87329) e' 0x78 (120, "orr w1,wzr,#0x78" nell'unico
	 * devm_kmalloc del driver) -- troppo per i soli campi sopra (17
	 * byte). Il resto e' un hrtimer e un work_struct incorporati,
	 * inizializzati in probe e usati da cfg_timer_func/cfg_work_routine
	 * (vedi sotto, fuori dai 27 -- trovati in questo lotto):
	 *
	 *   timer a offset 0x18 (add x0,x8,#0x18 passato a hrtimer_init e
	 *   hrtimer_start_range_ns); .function a offset 0x40 relativo a
	 *   timer (str x9,[x8,#64], x9 = indirizzo di cfg_timer_func).
	 *   work a offset 0x58 (add x2,x8,#0x18... la costante
	 *   0xfffffffe0 scritta a offset 88 e' il valore d'inizializzazione
	 *   di atomic_long_t work.data di INIT_WORK, non calcolato a mano);
	 *   .func a offset 0x70 relativo a struct aw87329 (stp x9,x11,
	 *   [x8,#104] -- x11 = indirizzo di cfg_work_routine).
	 *
	 * Gli offset esatti li scrive il compilatore dai veri
	 * linux/hrtimer.h e linux/workqueue.h di questo albero: qui non
	 * servono, bastano i campi con i tipi giusti nell'ordine giusto
	 * (0x10..0x18 di padding per l'allineamento a 8 byte di hrtimer,
	 * anch'esso non scritto a mano).
	 */
	struct hrtimer timer;
	struct work_struct work;
};

static struct aw87329 *g_aw87329;

/*
 * Tre puntatori statici, uno per modalita', a una tabella di registri
 * caricata da fuori (formato non noto: nel lotto 2 sono solo letti, mai
 * scritti -- l'assegnazione e' presumibilmente in un lotto successivo,
 * insieme alle *_cfg_loaded che il brief cita come chiamanti di queste
 * funzioni). Nomi scelti per descrittivita', non recuperati dal kernel di
 * fabbrica (la disassemblazione non risolve simboli per dati statici,
 * solo per funzioni): l'unico fatto verificato e' l'indirizzo .bss/.data,
 * a 8 byte l'uno dopo g_aw87329 (752), quindi 760/768/776 -- l'ordine
 * kspk/drcv/abrcv qui sotto rispecchia quello.
 */
static u8 *aw87329_kspk_reg;
static u8 *aw87329_drcv_reg;
static u8 *aw87329_abrcv_reg;

/*
 * Tabelle di registri di default, usate quando la corrispondente
 * *_reg_loaded e' zero. Undici byte ciascuna (indici 0..10, anche se gli
 * unici usati dalle aw87329_audio_* del lotto 2 sono 1..10), lette da
 * .rodata con objdump -s sull'indirizzo esatto -- non calcolate, non
 * indovinate:
 *
 *   kspk : 0xffffff8008faf0b3..0xffffff8008faf0bd
 *   drcv : 0xffffff8008faf0be..0xffffff8008faf0c8
 *   abrcv: 0xffffff8008faf0c9..0xffffff8008faf0d3
 *
 * $ objdump -s --start-address=0xffffff8008faf0a0 \
 *       --stop-address=0xffffff8008faf0e0 oracolo/stock.elf
 * ffffff8008faf0b0 00000039 0ea30605 10075206 0896390a  ...9......R...9.
 * ffffff8008faf0c0 ab060500 0f520908 97390aaf 0605000f  .....R...9......
 * ffffff8008faf0d0 52090897 00000000 00000000 00000000  R...............
 */
static const u8 aw87329_kspk_reg_default[11] = {
	0x39, 0x0e, 0xa3, 0x06, 0x05, 0x10, 0x07, 0x52, 0x06, 0x08, 0x96,
};
static const u8 aw87329_drcv_reg_default[11] = {
	0x39, 0x0a, 0xab, 0x06, 0x05, 0x00, 0x0f, 0x52, 0x09, 0x08, 0x97,
};
static const u8 aw87329_abrcv_reg_default[11] = {
	0x39, 0x0a, 0xaf, 0x06, 0x05, 0x00, 0x0f, 0x52, 0x09, 0x08, 0x97,
};

/*
 * i2c_read_reg e i2c_write_reg sono definite in fondo/in mezzo al file
 * (seguono l'ordine della sequenza-bersaglio, non l'ordine di chiamata:
 * vedi driver-riscritti.md). aw87329_read_chipid chiama la prima,
 * aw87329_audio_kspk la seconda, entrambe prima del punto in cui sono
 * definite -- da qui le dichiarazioni in avanti, altrimenti -Werror=
 * implicit-function-declaration ferma la build.
 */
static int i2c_read_reg(u8 reg_addr);
static void i2c_write_reg(u8 reg_addr, u8 val);

/*
 * aw87329_hw_on: 0xffffff8008c585cc, 164 byte, globale (T).
 *
 * ffffff8008c585d8..585e8: printk(KERN_INFO "%s enter\n"@0x92a2f79,
 *   "aw87329_hw_on"@0x92a2f85)
 *   - il fmt e' "\x016%s enter\n" nel binario, KERN_INFO="\001""6" concatenato
 *     dal preprocessore nel letterale;
 *     ffffff8008c585d8 adrp x0 / ffffff8008c585e0 add x0,x0,#0xf79 e
 *     ffffff8008c585dc adrp x1 / ffffff8008c585e4 add x1,x1,#0xf85 sulla
 *     pagina 0xffffff80092a2000 -- e' QUESTA coppia di istruzioni a costruire
 *     i due indirizzi citati, non una somiglianza di testo.
 * ffffff8008c585ec..58600: if (g_aw87329 && gpio_is_valid(g_aw87329->hwen_gpio))
 *   - cbz su g_aw87329 (offset 752 nel .bss del file, indirizzo non
 *     significativo qui); ldr w0,[x8,#8] legge hwen_gpio; "cmp w0,#0x1ff;
 *     b.hi" e' esattamente gpio_is_valid() inlineata (asm-generic/gpio.h:
 *     number>=0 && number<ARCH_NR_GPIOS, con ARCH_NR_GPIOS=512 di default
 *     su questo albero: CONFIG_ARCH_NR_GPIO non e' impostato nel config di
 *     fabbrica).
 * ffffff8008c58604..5863c: due cicli gpio_to_desc()+gpiod_set_raw_value_
 *   cansleep()+msleep(2), prima a 0 poi a 1, poi hwen_flag = 1 (strb w20,
 *   [x8,#13], w20 caricato con 1 all'orr w20,wzr,#0x1 di 0x58628).
 * ffffff8008c58644..5865c: else path, dev_err(&g_aw87329->i2c->dev,
 *   "%s:  failed\n", "aw87329_hw_on")
 *   - "ldr x8,[x8]" deref di g_aw87329 (offset 0, campo i2c) anche quando
 *     g_aw87329 e' NULL (nessuna guardia aggiuntiva: e' il comportamento
 *     del kernel di fabbrica, riprodotto cosi' com'e'); "add x0,x8,#0x20"
 *     e' &i2c->dev, verificato: offsetof(struct i2c_client, dev) e' 0x20
 *     in questo albero (flags+addr=4, name[20], adapter*=8 -> 4+20+8=0x20,
 *     niente campo "driver" in questa versione di struct i2c_client).
 *   - "%s:  failed\n"@0x91c7380 (senza prefisso KERN_*: e' dev_err, che il
 *     livello lo mette da se'), "aw87329_hw_on"@0x92a2f85 -- la stessa
 *     stringa del printk sopra, ricaricata da
 *     ffffff8008c5864c adrp x2 / ffffff8008c58658 add x2,x2,#0xf85.
 * ffffff8008c58660..5866c: return 0 (comune a entrambi i percorsi).
 */
int aw87329_hw_on(void)
{
	printk(KERN_INFO "%s enter\n", "aw87329_hw_on");

	if (g_aw87329 && gpio_is_valid(g_aw87329->hwen_gpio)) {
		gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 0);
		msleep(2);
		gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 1);
		msleep(2);
		g_aw87329->hwen_flag = 1;
	} else {
		dev_err(&g_aw87329->i2c->dev, "%s:  failed\n", "aw87329_hw_on");
	}

	return 0;
}

/*
 * aw87329_hw_off: 0xffffff8008c58670, 132 byte, globale (T).
 *
 * Stessa ossatura di aw87329_hw_on, ma un solo impulso GPIO (portato a 0,
 * niente impulso a 1) e hwen_flag messo a 0 invece che a 1 -- per questo
 * il prologo salva solo x19 e non x20 (non serve un registro per il
 * valore 1 da salvare per dopo): 132 byte contro 164.
 *
 * ffffff8008c5867c..5868c: printk(KERN_INFO "%s enter\n", "aw87329_hw_off")
 *   - stringa nome a 0x92a2f93 ("aw87329_hw_off").
 * ffffff8008c58690..586a4: stessa guardia if (g_aw87329 && gpio_is_valid(...))
 *   di aw87329_hw_on.
 * ffffff8008c586a8..586c0: gpio_to_desc()+gpiod_set_raw_value_cansleep(desc,0)
 *   +msleep(2), poi hwen_flag = 0 (strb wzr,[x8,#13]).
 * ffffff8008c586c8..586e0: else path, dev_err(&g_aw87329->i2c->dev,
 *   "%s:  failed\n", "aw87329_hw_off") -- stessa forma di aw87329_hw_on,
 *   stringa nome a 0x92a2f93.
 */
int aw87329_hw_off(void)
{
	printk(KERN_INFO "%s enter\n", "aw87329_hw_off");

	if (g_aw87329 && gpio_is_valid(g_aw87329->hwen_gpio)) {
		gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 0);
		msleep(2);
		g_aw87329->hwen_flag = 0;
	} else {
		dev_err(&g_aw87329->i2c->dev, "%s:  failed\n", "aw87329_hw_off");
	}

	return 0;
}

/*
 * aw87329_kspk_reg_val: 0xffffff8008c586f4, 56 byte, globale (T).
 *
 * Restituisce il valore del registro "index" per la modalita' altoparlante
 * (kspk): dalla tabella caricata via aw87329_kspk_reg se
 * g_aw87329->kspk_reg_loaded e' diverso da zero, altrimenti dalla tabella
 * di default in .rodata (sopra). Nessun controllo su g_aw87329 == NULL
 * (non c'e' nel disassemblato, non lo aggiungiamo).
 *
 * ffffff8008c586f4..586fc: g_aw87329->kspk_reg_loaded (ldrb w8,[x8,#14],
 *   offset 0xe).
 * ffffff8008c58700: cbz w8 -> ramo di default (0x58718).
 * ffffff8008c58704..58714: ramo "caricato": x8 = aw87329_kspk_reg (letto
 *   dall'indirizzo .bss/.data a offset 760); "add x8,x8,w0,uxtb" e'
 *   aritmetica su puntatore a byte con indice a 8 bit (uxtb -> il
 *   parametro e' u8, non int); "ldrb w0,[x8,#4]!" e' un pre-indicizzato:
 *   legge *(aw87329_kspk_reg + index + 4). return aw87329_kspk_reg[index+4].
 * ffffff8008c58718..58728: ramo di default: x8 = &aw87329_kspk_reg_default
 *   (adrp+add #0xb3) + index (uxtb); ldrb w0,[x8]. return
 *   aw87329_kspk_reg_default[index].
 */
u8 aw87329_kspk_reg_val(u8 index)
{
	if (g_aw87329->kspk_reg_loaded)
		return aw87329_kspk_reg[index + 4];

	return aw87329_kspk_reg_default[index];
}

/*
 * aw87329_drcv_reg_val: 0xffffff8008c5872c, 56 byte, globale (T).
 * Identica a aw87329_kspk_reg_val, sul campo/puntatore/tabella della
 * modalita' ricevitore (drcv): offset 0xf, aw87329_drcv_reg (offset 760+8),
 * aw87329_drcv_reg_default (.rodata #0xbe).
 *
 * ffffff8008c5872c..58738: g_aw87329->drcv_reg_loaded (ldrb w8,[x8,#15]).
 * ffffff8008c5873c..5874c: ramo "caricato": aw87329_drcv_reg[index+4]
 *   (ldr x8,[x8,#768]; ldrb w0,[x8,#4]! dopo add x8,x8,w0,uxtb).
 * ffffff8008c58750..58760: ramo di default: rodata+0xbe+index.
 */
u8 aw87329_drcv_reg_val(u8 index)
{
	if (g_aw87329->drcv_reg_loaded)
		return aw87329_drcv_reg[index + 4];

	return aw87329_drcv_reg_default[index];
}

/*
 * aw87329_abrcv_reg_val: 0xffffff8008c58764, 56 byte, globale (T).
 * Identica alle due sopra, sul campo/puntatore/tabella della modalita'
 * ricevitore alternativo (abrcv): offset 0x10, aw87329_abrcv_reg
 * (offset 768+8=776), aw87329_abrcv_reg_default (.rodata #0xc9).
 *
 * ffffff8008c58764..58770: g_aw87329->abrcv_reg_loaded (ldrb w8,[x8,#16]).
 * ffffff8008c58774..58784: ramo "caricato": aw87329_abrcv_reg[index+4]
 *   (ldr x8,[x8,#776]).
 * ffffff8008c58788..58798: ramo di default: rodata+0xc9+index.
 */
u8 aw87329_abrcv_reg_val(u8 index)
{
	if (g_aw87329->abrcv_reg_loaded)
		return aw87329_abrcv_reg[index + 4];

	return aw87329_abrcv_reg_default[index];
}

/*
 * aw87329_audio_kspk: 0xffffff8008c5879c, 552 byte, globale (T).
 *
 * Accende il chip in modalita' altoparlante: controlla che il driver sia
 * inizializzato, accende l'hardware se non gia' acceso, poi scrive undici
 * registri (indirizzi 1..10, poi di nuovo 1) leggendo ogni valore da
 * aw87329_kspk_reg_val(). Undici "bl i2c_write_reg" contate nel
 * disassemblato -- non dieci, non dodici.
 *
 * **Nel binario di fabbrica non c'e' nessuna "bl aw87329_kspk_reg_val"**:
 * il compilatore l'ha incorporata undici volte. Scrivere il sorgente come
 * una chiamata a aw87329_kspk_reg_val(N) e' l'interpretazione corretta
 * della logica (la stessa guardia sul flag, lo stesso puntatore, la stessa
 * tabella di default della funzione a se stante), ma quello che si legge
 * qui sotto per ogni registro (il "csel" fra i due indirizzi candidati) e'
 * il corpo di aw87329_kspk_reg_val() gia' incorporato dal compilatore di
 * fabbrica, non una lettura diretta scritta cosi' nel sorgente originale.
 *
 * ffffff8008c587b4: cbz x8, ...58990 -- if (!g_aw87329) -> errore.
 * ffffff8008c587b8..587bc: ldrb w9,[x8,#12] (offset 0xc, "inited"); cbz ->
 *   ...5899c -- if (!g_aw87329->inited) -> errore.
 * ffffff8008c587c0..587c8: ldrb w9,[x8,#13] (hwen_flag); cbnz salta la
 *   chiamata -- if (!g_aw87329->hwen_flag) aw87329_hw_on().
 * ffffff8008c587cc: ldr x8,[x19,#752] -- ricarica g_aw87329 dopo la
 *   chiamata (il compilatore lo fa perche' la funzione chiamata puo' aver
 *   sporcato i registri, non perche' il puntatore cambi).
 * ffffff8008c587d0..587fc: val = aw87329_kspk_reg_val(1);
 *   i2c_write_reg(1, val & ~0x8) -- "and w1,w8,#0xfffffff7" e' l'unica
 *   delle undici scritture con maschera: bit 3 forzato a 0.
 * ffffff8008c58800..58984: reg 2..10 (w0 = 0x2..0xa in sequenza,
 *   verificato immediato per immediato), poi di nuovo reg 1 stavolta
 *   senza maschera (0x58964..58984, w1 = valore grezzo).
 * ffffff8008c58988..5898c: mov w0,wzr; b ...589b4 -- return 0.
 * ffffff8008c58990..589b0: i due percorsi di errore confluiscono nello
 *   stesso printk (KERN_ERR, "\x013" -- vedi le stringhe sotto), con
 *   messaggi diversi ma lo stesso nome "aw87329_audio_kspk"; return 1.
 *
 * Stringhe (pagina 0xffffff80092a2000, byte per byte; il prefisso "\x013" nel
 * binario e' il KERN_ERR concatenato nel letterale, non parte del testo):
 *   "%s: aw87329 is NULL\n"@0x92a2fa2
 *     -- ffffff8008c58990 adrp x0 / ffffff8008c58994 add x0,x0,#0xfa2
 *   "%s: aw87329 init failed\n"@0x92a2fcc
 *     -- ffffff8008c5899c adrp x0 / ffffff8008c589a0 add x0,x0,#0xfcc
 *   "aw87329_audio_kspk"@0x92a2fb9
 *     -- ffffff8008c589a4 adrp x1 / ffffff8008c589a8 add x1,x1,#0xfb9
 *
 * I due percorsi d'errore caricano due fmt diversi in x0 e confluiscono in
 * UNA sola "bl printk" (fusione di code delle code fatta dal compilatore):
 * per questo il blocco ha 1 printk statica e 2 printk nel sorgente.
 */
int aw87329_audio_kspk(void)
{
	u8 val;

	if (!g_aw87329) {
		printk(KERN_ERR "%s: aw87329 is NULL\n", "aw87329_audio_kspk");
		return 1;
	}

	if (!g_aw87329->inited) {
		printk(KERN_ERR "%s: aw87329 init failed\n", "aw87329_audio_kspk");
		return 1;
	}

	if (!g_aw87329->hwen_flag)
		aw87329_hw_on();

	val = aw87329_kspk_reg_val(1);
	i2c_write_reg(1, val & ~0x8);

	val = aw87329_kspk_reg_val(2);
	i2c_write_reg(2, val);
	val = aw87329_kspk_reg_val(3);
	i2c_write_reg(3, val);
	val = aw87329_kspk_reg_val(4);
	i2c_write_reg(4, val);
	val = aw87329_kspk_reg_val(5);
	i2c_write_reg(5, val);
	val = aw87329_kspk_reg_val(6);
	i2c_write_reg(6, val);
	val = aw87329_kspk_reg_val(7);
	i2c_write_reg(7, val);
	val = aw87329_kspk_reg_val(8);
	i2c_write_reg(8, val);
	val = aw87329_kspk_reg_val(9);
	i2c_write_reg(9, val);
	val = aw87329_kspk_reg_val(10);
	i2c_write_reg(10, val);

	val = aw87329_kspk_reg_val(1);
	i2c_write_reg(1, val);

	return 0;
}

/*
 * i2c_write_reg: 0xffffff8008c589c4, 168 byte, statica (t).
 *
 * Stessa ossatura di i2c_read_reg (lotto 1): due tentativi di
 * i2c_smbus_write_byte_data, msleep(2) fra l'uno e l'altro. A differenza
 * di i2c_read_reg pero' il "ret" finale non e' mai salvato in una
 * variabile ne' restituito: dopo "tbz w0,#31" (successo) si salta
 * direttamente all'epilogo, e nessuno dei cinque chiamanti (le tre
 * aw87329_audio_*, aw87329_audio_off) controlla w0 dopo la "bl" -- da qui
 * la scelta di renderla void, non int.
 *
 * ffffff8008c589e4..589f4: ret = i2c_smbus_write_byte_data(g_aw87329->i2c,
 *   reg_addr, val) -- "ldr x8,[x8]" e' il campo i2c a offset 0 (lotto 1);
 *   w1=reg_addr, w2=val (w19/w20 salvano i parametri prima della prima
 *   chiamata, riusati identici al retry).
 * ffffff8008c589f8: tbz w0,#31, ...58a5c -- se non negativo, salta
 *   all'epilogo.
 * ffffff8008c58a00..58a1c: se negativo: printk(KERN_ERR "%s: i2c_write
 *   cnt=%d error=%d\n", "i2c_write_reg", 0, ret); msleep(2).
 * ffffff8008c58a20..58a34: retry, stessa i2c_smbus_write_byte_data.
 * ffffff8008c58a38..58a58: se ancora negativo: stesso printk con cnt=1
 *   (orr w2,wzr,#0x1); msleep(2).
 * ffffff8008c58a5c: epilogo comune a tutti i percorsi.
 *
 * Stringhe ("\x013" = KERN_ERR concatenato, non parte del testo):
 *   "%s: i2c_write cnt=%d error=%d\n"@0x91c7110
 *     -- ffffff8008c58a00 adrp x0 / ffffff8008c58a08 add x0,x0,#0x110, e di
 *        nuovo ffffff8008c58a3c/ffffff8008c58a44 per il secondo tentativo.
 *   "i2c_write_reg"@0x92a3091 (non "aw87329_i2c_write_reg": stessa
 *     convenzione di i2c_read_reg nel lotto 1, verificata di nuovo qui)
 *     -- ffffff8008c58a04 adrp x1 / ffffff8008c58a0c add x1,x1,#0x91.
 */
static void i2c_write_reg(u8 reg_addr, u8 val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(g_aw87329->i2c, reg_addr, val);
	if (ret < 0) {
		printk(KERN_ERR "%s: i2c_write cnt=%d error=%d\n",
		       "i2c_write_reg", 0, ret);
		msleep(2);

		ret = i2c_smbus_write_byte_data(g_aw87329->i2c, reg_addr, val);
		if (ret < 0) {
			printk(KERN_ERR "%s: i2c_write cnt=%d error=%d\n",
			       "i2c_write_reg", 1, ret);
			msleep(2);
		}
	}
}

/*
 * aw87329_audio_drcv: 0xffffff8008c58a6c, 552 byte, globale (T).
 * Identica a aw87329_audio_kspk, sostituendo il flag/tabella della
 * modalita' ricevitore (offset 0xf, aw87329_drcv_reg_val) e il nome nei
 * messaggi. Stessa sequenza di undici registri (1..10, poi 1), stesso
 * unico "and ...,#0xfffffff7" sulla prima scrittura del registro 1. Stessa
 * avvertenza di aw87329_audio_kspk sull'inlining: nessuna "bl
 * aw87329_drcv_reg_val" nel binario di fabbrica.
 *
 * Stringhe (stessa pagina, verificate):
 *   "%s: aw87329 is NULL\n"@0x92a2fa2 (condivisa con audio_kspk)
 *     -- ffffff8008c58c60 adrp x0 / ffffff8008c58c64 add x0,x0,#0xfa2
 *   "%s: aw87329 init failed\n"@0x92a2fcc (condivisa)
 *     -- ffffff8008c58c6c adrp x0 / ffffff8008c58c70 add x0,x0,#0xfcc
 *   "aw87329_audio_drcv"@0x92a2fe7
 *     -- ffffff8008c58c74 adrp x1 / ffffff8008c58c78 add x1,x1,#0xfe7
 */
int aw87329_audio_drcv(void)
{
	u8 val;

	if (!g_aw87329) {
		printk(KERN_ERR "%s: aw87329 is NULL\n", "aw87329_audio_drcv");
		return 1;
	}

	if (!g_aw87329->inited) {
		printk(KERN_ERR "%s: aw87329 init failed\n", "aw87329_audio_drcv");
		return 1;
	}

	if (!g_aw87329->hwen_flag)
		aw87329_hw_on();

	val = aw87329_drcv_reg_val(1);
	i2c_write_reg(1, val & ~0x8);

	val = aw87329_drcv_reg_val(2);
	i2c_write_reg(2, val);
	val = aw87329_drcv_reg_val(3);
	i2c_write_reg(3, val);
	val = aw87329_drcv_reg_val(4);
	i2c_write_reg(4, val);
	val = aw87329_drcv_reg_val(5);
	i2c_write_reg(5, val);
	val = aw87329_drcv_reg_val(6);
	i2c_write_reg(6, val);
	val = aw87329_drcv_reg_val(7);
	i2c_write_reg(7, val);
	val = aw87329_drcv_reg_val(8);
	i2c_write_reg(8, val);
	val = aw87329_drcv_reg_val(9);
	i2c_write_reg(9, val);
	val = aw87329_drcv_reg_val(10);
	i2c_write_reg(10, val);

	val = aw87329_drcv_reg_val(1);
	i2c_write_reg(1, val);

	return 0;
}

/*
 * aw87329_audio_abrcv: 0xffffff8008c58c94, 552 byte, globale (T).
 * Identica alle due sopra, sostituendo il flag/tabella della modalita'
 * ricevitore alternativo (offset 0x10, aw87329_abrcv_reg_val) e il nome
 * nei messaggi. Stessa avvertenza sull'inlining: nessuna "bl
 * aw87329_abrcv_reg_val" nel binario di fabbrica.
 *
 * Stringhe (verificate):
 *   "%s: aw87329 is NULL\n"@0x92a2fa2 (condivisa)
 *     -- ffffff8008c58e88 adrp x0 / ffffff8008c58e8c add x0,x0,#0xfa2
 *   "%s: aw87329 init failed\n"@0x92a2fcc (condivisa)
 *     -- ffffff8008c58e94 adrp x0 / ffffff8008c58e98 add x0,x0,#0xfcc
 *   "aw87329_audio_abrcv"@0x92a2ffa
 *     -- ffffff8008c58e9c adrp x1 / ffffff8008c58ea0 add x1,x1,#0xffa
 */
int aw87329_audio_abrcv(void)
{
	u8 val;

	if (!g_aw87329) {
		printk(KERN_ERR "%s: aw87329 is NULL\n", "aw87329_audio_abrcv");
		return 1;
	}

	if (!g_aw87329->inited) {
		printk(KERN_ERR "%s: aw87329 init failed\n", "aw87329_audio_abrcv");
		return 1;
	}

	if (!g_aw87329->hwen_flag)
		aw87329_hw_on();

	val = aw87329_abrcv_reg_val(1);
	i2c_write_reg(1, val & ~0x8);

	val = aw87329_abrcv_reg_val(2);
	i2c_write_reg(2, val);
	val = aw87329_abrcv_reg_val(3);
	i2c_write_reg(3, val);
	val = aw87329_abrcv_reg_val(4);
	i2c_write_reg(4, val);
	val = aw87329_abrcv_reg_val(5);
	i2c_write_reg(5, val);
	val = aw87329_abrcv_reg_val(6);
	i2c_write_reg(6, val);
	val = aw87329_abrcv_reg_val(7);
	i2c_write_reg(7, val);
	val = aw87329_abrcv_reg_val(8);
	i2c_write_reg(8, val);
	val = aw87329_abrcv_reg_val(9);
	i2c_write_reg(9, val);
	val = aw87329_abrcv_reg_val(10);
	i2c_write_reg(10, val);

	val = aw87329_abrcv_reg_val(1);
	i2c_write_reg(1, val);

	return 0;
}

/*
 * aw87329_audio_off: 0xffffff8008c58ebc, 212 byte, globale (T).
 *
 * Spegne l'amplificatore: se il chip era acceso, scrive un valore fisso
 * (0xc, non dalla tabella dei registri) nel registro 1, poi spegne il GPIO
 * con la stessa ossatura di aw87329_hw_off.
 *
 * **Bug del driver di fabbrica, riprodotto cosi' com'e', non corretto**:
 * la stringa nome usata nei due messaggi del percorso "spegni il GPIO" e'
 * "aw87329_hw_off", non "aw87329_audio_off" -- verificato con objdump -s
 * sull'indirizzo esatto (0x92a2f93), la stessa stringa gia' letta nel
 * lotto 1 per aw87329_hw_off. Tutto il resto del corpo di quel percorso
 * (fmt "%s enter\n" a 0x92a2f79, fmt dev_err "%s:  failed\n" a 0x91c7380)
 * e' condiviso con aw87329_hw_off: sembra un copia-incolla del corpo di
 * hw_off dentro audio_off senza aggiornare il nome. I due messaggi di
 * guardia iniziali (g_aw87329 NULL, non inited) usano invece
 * correttamente "aw87329_audio_off" (0x92a300e, verificato).
 *
 * ffffff8008c58ed0: cbz x8,...58f3c -- if (!g_aw87329) -> errore, return 1.
 * ffffff8008c58ed4..58ed8: ldrb w9,[x8,#12] (offset 0xc); cbz ->...58f6c --
 *   if (!g_aw87329->inited) -> errore, return 1.
 * ffffff8008c58edc..58eec: ldrb w8,[x8,#13] (hwen_flag); cbz salta la
 *   scrittura -- if (hwen_flag) i2c_write_reg(1, 0xc) (orr w1,wzr,#0xc).
 * ffffff8008c58ef0..58f00: printk(KERN_INFO "%s enter\n", "aw87329_hw_off")
 *   -- il bug, vedi sopra.
 * ffffff8008c58f04..58f38: ricarica g_aw87329; se NULL o hwen_gpio non
 *   valido (stesso gpio_is_valid() del lotto 1) -> percorso di errore
 *   sotto; altrimenti gpiod_set_raw_value_cansleep(desc,0), msleep(2),
 *   hwen_flag=0, return 0.
 * ffffff8008c58f48..58f68: percorso di errore -- deref di g_aw87329 senza
 *   guardia aggiuntiva anche quando e' NULL (stesso comportamento delle
 *   hw_* del lotto 1, confermato di nuovo qui), dev_err(&g_aw87329->i2c->
 *   dev, "%s:  failed\n", "aw87329_hw_off") -- il bug di nuovo; return 0
 *   (non 1: il fallimento dello spegnimento GPIO non fa fallire la
 *   funzione, comportamento del kernel di fabbrica, non "sistemato").
 *
 * Stringhe di guardia (corrette, verificate):
 *   "%s: aw87329 is NULL\n"@0x92a2fa2
 *     -- ffffff8008c58f3c adrp x0 / ffffff8008c58f40 add x0,x0,#0xfa2
 *   "%s: aw87329 init failed\n"@0x92a2fcc
 *     -- ffffff8008c58f6c adrp x0 / ffffff8008c58f70 add x0,x0,#0xfcc
 *   "aw87329_audio_off"@0x92a300e
 *     -- ffffff8008c58f74 adrp x1 / ffffff8008c58f78 add x1,x1,#0xe
 */
int aw87329_audio_off(void)
{
	if (!g_aw87329) {
		printk(KERN_ERR "%s: aw87329 is NULL\n", "aw87329_audio_off");
		return 1;
	}

	if (!g_aw87329->inited) {
		printk(KERN_ERR "%s: aw87329 init failed\n", "aw87329_audio_off");
		return 1;
	}

	if (g_aw87329->hwen_flag)
		i2c_write_reg(1, 0xc);

	printk(KERN_INFO "%s enter\n", "aw87329_hw_off");

	if (!g_aw87329 || !gpio_is_valid(g_aw87329->hwen_gpio)) {
		dev_err(&g_aw87329->i2c->dev, "%s:  failed\n", "aw87329_hw_off");
		return 0;
	}

	gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 0);
	msleep(2);
	g_aw87329->hwen_flag = 0;

	return 0;
}

/*
 * aw87329_hw_reset: 0xffffff8008c58f90, 168 byte, globale (T).
 *
 * Identica ossatura di aw87329_hw_on (due impulsi, 0 poi 1, hwen_flag=1
 * nel percorso riuscito), con un'unica differenza verificata: nel percorso
 * di errore scrive anche hwen_flag = 0 prima di dev_err (strb wzr,[x8,#13]
 * a 0xffffff8008c59020, assente nell'else di hw_on/hw_off) -- 168 byte
 * contro 164 di hw_on, coerente con quell'istruzione in piu'.
 *
 * ffffff8008c58f9c..58fac: printk(KERN_INFO "%s enter\n"@0x92a2f79,
 *   "aw87329_hw_reset"@0x92a3020)
 *   -- ffffff8008c58fa0 adrp x1 / ffffff8008c58fa8 add x1,x1,#0x20.
 * ffffff8008c58fb0..58fc4: stessa guardia if (g_aw87329 && gpio_is_valid(...)).
 * ffffff8008c58fc8..59004: due cicli gpio_to_desc()+gpiod_set_raw_value_
 *   cansleep()+msleep(2), 0 poi 1, poi hwen_flag = 1.
 * ffffff8008c59008..59024: else path, hwen_flag = 0 poi dev_err(&g_aw87329->
 *   i2c->dev, "%s:  failed\n"@0x91c7380, "aw87329_hw_reset"@0x92a3020) -- la
 *   stessa stringa nome del printk sopra, ricaricata da
 *   ffffff8008c59010 adrp x2 / ffffff8008c5901c add x2,x2,#0x20.
 */
int aw87329_hw_reset(void)
{
	printk(KERN_INFO "%s enter\n", "aw87329_hw_reset");

	if (g_aw87329 && gpio_is_valid(g_aw87329->hwen_gpio)) {
		gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 0);
		msleep(2);
		gpiod_set_raw_value_cansleep(gpio_to_desc(g_aw87329->hwen_gpio), 1);
		msleep(2);
		g_aw87329->hwen_flag = 1;
	} else {
		g_aw87329->hwen_flag = 0;
		dev_err(&g_aw87329->i2c->dev, "%s:  failed\n", "aw87329_hw_reset");
	}

	return 0;
}

/*
 * aw87329_read_chipid: 0xffffff8008c59038, 152 byte, globale (T).
 * Chiama i2c_read_reg, poi printk e msleep -- verificato, non supposto
 * (vedi il brief del Task 2).
 *
 * ffffff8008c59040..59048: val = i2c_read_reg(0) & 0xff
 *   ("mov w0,wzr; bl i2c_read_reg; and w2,w0,#0xff").
 * ffffff8008c5904c..59050: cmp w2,#0x39; b.eq -> successo diretto (0x39 =
 *   57 decimale, il chip id atteso di AW87329).
 * ffffff8008c59054..5906c: se val != 0x39: printk(KERN_INFO
 *   "%s: aw87329 chipid=0x%x error\n"@0x92a3060,
 *   "aw87329_read_chipid"@0x92a304c, val); msleep(2)
 *   - ffffff8008c59054 adrp x0 / ffffff8008c5905c add x0,x0,#0x60 e
 *     ffffff8008c59058 adrp x1 / ffffff8008c59060 add x1,x1,#0x4c; terzo
 *     argomento = w2 (val, ancora nel registro dalla and precedente,
 *     nessun mov intermedio).
 * ffffff8008c59070..59080: retry, val = i2c_read_reg(0) & 0xff; se ancora
 *   != 0x39 -> salta a 0x590a8 (fallimento).
 * ffffff8008c59084..590a4: successo (diretto o dopo retry): printk(KERN_INFO
 *   "%s: aw87329 chipid=0x%x\n"@0x92a3031, "aw87329_read_chipid"@0x92a304c,
 *   0x39) -- terzo argomento e' l'immediato 0x39 (mov w2,#0x39), non val;
 *   poi return 0. **L'indirizzo di QUESTA fmt non era mai stato annotato**
 *   (il commento diceva solo "fmt riusata" per l'altra): letto ora da
 *   ffffff8008c59084 adrp x0 / ffffff8008c5908c add x0,x0,#0x31, ed e' una
 *   stringa distinta da quella d'errore, non la stessa.
 * ffffff8008c590a8..590cc: fallimento dopo il retry: stesso printk di
 *   errore ("%s: aw87329 chipid=0x%x error\n"@0x92a3060 di nuovo,
 *   ffffff8008c590a8 adrp x0 / ffffff8008c590b0 add x0,x0,#0x60),
 *   msleep(2), return -EINVAL (mov w0,#0xffffffea = -22).
 *
 * Codice scritto due volte in linea (nessun ciclo, nessun'etichetta di
 * back-edge nel disassemblato): il retry e' srotolato, non e' un for/while.
 */
int aw87329_read_chipid(void)
{
	int val;

	val = i2c_read_reg(0) & 0xff;
	if (val != 0x39) {
		printk(KERN_INFO "%s: aw87329 chipid=0x%x error\n",
		       "aw87329_read_chipid", val);
		msleep(2);

		val = i2c_read_reg(0) & 0xff;
		if (val != 0x39) {
			printk(KERN_INFO "%s: aw87329 chipid=0x%x error\n",
			       "aw87329_read_chipid", val);
			msleep(2);
			return -EINVAL;
		}
	}

	printk(KERN_INFO "%s: aw87329 chipid=0x%x\n", "aw87329_read_chipid", 0x39);
	return 0;
}

/*
 * i2c_read_reg: 0xffffff8008c590d0, 168 byte, statica (t).
 * Non porta il nome del chip: il conteggio per prefisso non la vedeva,
 * l'adiacenza si'.
 *
 * ffffff8008c590dc..590f8: ret = i2c_smbus_read_byte_data(g_aw87329->i2c,
 *   reg_addr)
 *   - "ldr x8,[x8]" deref di g_aw87329 (offset 0, campo i2c), "mov x0,x8"
 *     e' il primo argomento, "mov w1,w20" (w20 = reg_addr salvato
 *     all'ingresso) e' il secondo.
 * ffffff8008c590fc..59100: "tbz w0,#31" -- se il risultato non e' negativo
 *   (successo), salta direttamente alla fine (0x59164).
 * ffffff8008c59104..59124: se negativo: printk(KERN_ERR
 *   "%s: i2c_read cnt=%d error=%d\n"@0x91c7458, "i2c_read_reg"@0x92a309f,
 *   0, ret); msleep(2)
 *   - nel binario la fmt e' "\x013%s: i2c_read cnt=%d error=%d\n"
 *     (KERN_ERR="\001""3" concatenato nel letterale); il nome e'
 *     "i2c_read_reg", non "aw87329_i2c_read_reg" -- letto, non dedotto.
 *     ffffff8008c59104 adrp x0 / ffffff8008c5910c add x0,x0,#0x458 e
 *     ffffff8008c59108 adrp x1 / ffffff8008c59110 add x1,x1,#0x9f.
 * ffffff8008c59128..59138: retry, ret = i2c_smbus_read_byte_data(g_aw87329
 *   ->i2c, reg_addr) di nuovo.
 * ffffff8008c5913c..59160: se ancora negativo: stesso printk con cnt=1
 *   (orr w2,wzr,#0x1) invece di 0; msleep(2).
 * ffffff8008c59164..59174: return ret (w19, l'ultimo valore letto).
 */
static int i2c_read_reg(u8 reg_addr)
{
	int ret;

	ret = i2c_smbus_read_byte_data(g_aw87329->i2c, reg_addr);
	if (ret < 0) {
		printk(KERN_ERR "%s: i2c_read cnt=%d error=%d\n",
		       "i2c_read_reg", 0, ret);
		msleep(2);

		ret = i2c_smbus_read_byte_data(g_aw87329->i2c, reg_addr);
		if (ret < 0) {
			printk(KERN_ERR "%s: i2c_read cnt=%d error=%d\n",
			       "i2c_read_reg", 1, ret);
			msleep(2);
		}
	}

	return ret;
}

/*
 * ==========================================================================
 * Lotto 3: sysfs, sonda I2C, caricamento firmware.
 *
 * Posizioni 14-27 della sequenza-bersaglio (le ultime: questo lotto si
 * accoda, non si inserisce). Tutte statiche (t), tranne aw87329_pa_init
 * (sezione __init, lontana dal resto -- vedi in fondo).
 *
 * Una scoperta di questo lotto, non prevista dal brief: aw87329_i2c_probe
 * prende l'indirizzo di DUE funzioni -- cfg_timer_func (0xffffff8008c59e68,
 * 68 byte) e cfg_work_routine (0xffffff8008c59eac, 124 byte) -- che
 * ESISTONO in oracolo/stock.map, statiche (t), esattamente adiacenti alla
 * fine di questo blocco (subito dopo aw87329_set_mode, prima del tutto
 * estraneo move_addr_to_kernel) ma NON erano fra i 27 nomi del Task 1: il
 * confine del blocco, come gia' successo per altri quindici driver (vedi
 * HANDOFF.md), sottostimava di due. Sono scritte qui sotto, disassemblate
 * per intero come le altre, e il conteggio del driver passa da 27 a 29 --
 * vedi docs/bringup/driver-riscritti.md.
 * ==========================================================================
 */

/* Forward declaration: definite in fondo al file, prese per indirizzo da
 * aw87329_i2c_probe (hrtimer.function e work.func) -- vedi sopra. */
static enum hrtimer_restart cfg_timer_func(struct hrtimer *timer);
static void cfg_work_routine(struct work_struct *work);

/* Forward declaration: sysfs show/store, prese per indirizzo dalle
 * DEVICE_ATTR qui sotto -- necessarie prima di aw87329_i2c_probe, che crea
 * il gruppo sysfs, ma le funzioni vere sono scritte piu' sotto nell'ordine
 * della sequenza-bersaglio (posizioni 15-21). Stesso motivo della forward
 * declaration di i2c_read_reg/i2c_write_reg nei lotti 1-2. */
static ssize_t aw87329_get_reg(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t aw87329_set_reg(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count);
static ssize_t aw87329_get_hwen(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t aw87329_set_hwen(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count);
static ssize_t aw87329_get_update(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t aw87329_set_update(struct device *dev, struct device_attribute *attr,
				   const char *buf, size_t count);
static ssize_t aw87329_get_mode(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t aw87329_set_mode(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count);

/* Forward declaration: aw87329_set_update (posizione 21) e cfg_work_routine
 * (fuori dai 27, vedi sotto) prendono l'indirizzo di
 * aw87329_kspk_cfg_loaded (posizione 22) prima che sia definita; e la
 * catena di richieste firmware ha ciascuna callback che prende
 * l'indirizzo della successiva prima che sia definita (kspk->drcv->abrcv,
 * nell'ordine della sequenza-bersaglio 22-23-24). */
static void aw87329_kspk_cfg_loaded(const struct firmware *fw, void *context);
static void aw87329_drcv_cfg_loaded(const struct firmware *fw, void *context);
static void aw87329_abrcv_cfg_loaded(const struct firmware *fw, void *context);

/*
 * I quattro attributi sysfs, e il gruppo che li raccoglie: **verificati
 * leggendo `.rela.dyn`, non dedotti**. Prima stesura: nomi dedotti dai
 * printf format embedded nei show, permessi dichiarati "non verificabili
 * dal disassemblato, scelta convenzionale" -- sbagliato, segnalato in
 * revisione. `stock.elf` e' `CONFIG_RELOCATABLE=y`: i puntatori dati sono
 * zero nell'immagine e il valore vero sta nell'addend di una relocazione
 * `R_AARCH64_RELATIVE` in `.rela.dyn` (178.391 voci, `readelf -r --wide`).
 * Seguita la catena dall'indirizzo che `aw87329_i2c_probe` passa a
 * `sysfs_create_group` (x1 = adrp+add #0x78 dalla pagina 0x99ca000 =
 * 0xffffff8009 9ca078 = &aw87329_attribute_group):
 *
 *   $ readelf -r --wide oracolo/stock.elf | grep -E '^ffffff80099ca0(9|a|b|c)'
 *   ffffff80099ca090  ...  R_AARCH64_RELATIVE   -7ff6635f60   (attrs, offset
 *     0x18 in struct attribute_group: name/is_visible/is_bin_visible
 *     prima, nessuna relocazione = NULL, verificato)
 *   -7ff6635f60, interpretato come intero a 64 bit con segno e poi
 *     riletto come non firmato, e' 0xffffff80099ca0a0 -- l'array di
 *     quattro `struct attribute *` (+ terminatore NULL, nessuna
 *     relocazione all'ultimo slot) subito dopo la struct.
 *   I quattro puntatori dell'array risolvono a
 *     0x99ca0c8/0x99ca0e8/0x99ca108/0x99ca128 -- l'indirizzo di ciascun
 *     `struct device_attribute` (`&dev_attr_X.attr`, essendo `attr` il
 *     primo campo). Ogni struct e' 0x20 (32) byte: attr.name(reloc)
 *     +attr.mode(due byte, NON relocato, valore diretto)+show(reloc)+
 *     store(reloc) -- CONFIG_DEBUG_LOCK_ALLOC non impostata in questo
 *     config, quindi struct attribute e' 16 byte, non di piu'.
 *   `objdump -s` sull'offset +8 di ciascuna struct (il campo mode, letto
 *     diretto, non relocato): `b0 01 00 00` in tutte e quattro = 0x1b0 =
 *     **0660 ottale, non 0644**.
 *   I quattro nomi (relocazione di attr.name, risolta con objdump -s
 *     come le altre stringhe): "reg", "hwen", "update", "mode" -- **nello
 *     stesso ordine gia' scritto qui sotto**.
 *   I quattro show/store (relocazioni, risolte contro gli indirizzi delle
 *     nostre funzioni note): 0xc5946c/0xc594ec (get_reg/set_reg),
 *     0xc59564/0xc59598 (get_hwen/set_hwen), 0xc59670/0xc59678
 *     (get_update/set_update), 0xc59d60/0xc59dc8 (get_mode/set_mode) --
 *     **coincidono esattamente** con le otto funzioni scritte in questo
 *     lotto: la conferma indipendente piu' forte che il lotto potesse
 *     avere, sia sui nomi sia sull'accoppiamento show/store.
 */
static DEVICE_ATTR(reg, 0660, aw87329_get_reg, aw87329_set_reg);
static DEVICE_ATTR(hwen, 0660, aw87329_get_hwen, aw87329_set_hwen);
static DEVICE_ATTR(update, 0660, aw87329_get_update, aw87329_set_update);
static DEVICE_ATTR(mode, 0660, aw87329_get_mode, aw87329_set_mode);

static struct attribute *aw87329_attributes[] = {
	&dev_attr_reg.attr,
	&dev_attr_hwen.attr,
	&dev_attr_update.attr,
	&dev_attr_mode.attr,
	NULL,
};

static struct attribute_group aw87329_attribute_group = {
	.attrs = aw87329_attributes,
};

/*
 * aw87329_i2c_probe: 0xffffff8008c59178, 708 byte, statica (t). Di gran
 * lunga la funzione piu' grande del driver.
 *
 * ffffff8008c59178..59188: prologo; x21 = client->dev.of_node (ldr
 *   x21,[x0,#616] -- offset verificato: offsetof(i2c_client,dev)=0x20 dal
 *   lotto 1, +584 dentro struct device per of_node, non ricalcolato a
 *   mano qui: si scrive client->dev.of_node e si lascia al compilatore).
 *   x20 = client; x19 = &client->dev (add x19,x0,#0x20, stesso offset del
 *   lotto 1).
 * ffffff8008c59194..591a4: printk(KERN_INFO "%s Enter\n"@0x92a30b7,
 *   "aw87329_i2c_probe"@0x92a30c3) -- nel binario "\x016%s Enter\n", con la
 *   E maiuscola (le altre funzioni usano "%s enter\n"@0x92a2f79, minuscola:
 *   sono due stringhe diverse, non un refuso di lettura).
 *   ffffff8008c59194 adrp x0 / ffffff8008c5919c add x0,x0,#0xb7 e
 *   ffffff8008c59198 adrp x1 / ffffff8008c591a0 add x1,x1,#0xc3.
 * ffffff8008c591a8..591b4: x0 = client->adapter (ldr x0,[x20,#24]); x8 =
 *   adapter->algo (ldr x8,[x0,#16]); x8 = algo->functionality (ldr
 *   x8,[x8,#16]); blr x8 -- e' i2c_check_functionality() inlineata
 *   (static inline in linux/i2c.h): "(adap->algo->functionality(adap) &
 *   func) == func".
 * ffffff8008c591b8: tbnz w0,#0 -- bit 0 del risultato e' I2C_FUNC_I2C
 *   (0x1): se presente, salta il fallimento.
 * ffffff8008c591bc..591d8: se assente: dev_err(dev, "%s: check_functionality
 *   failed\n"@0x92a30d5, "aw87329_i2c_probe"@0x92a30c3); return -ENODEV
 *   (mov w21,#0xffffffed = -19).
 * ffffff8008c591dc..591f8: priv = devm_kzalloc(dev, 0x78, GFP_KERNEL) --
 *   w1=0x78 (120 = sizeof(struct aw87329), verificato, vedi sopra); w2 si
 *   costruisce come 0x80c0|(0x140<<16) = 0x014080c0. **Corretto dopo
 *   revisione, trovato dal confronto delle istruzioni normalizzato**: la
 *   prima stesura scriveva `devm_kmalloc(dev, ..., GFP_KERNEL)`, che in
 *   questo albero costruisce 0x0140_00c0 (verificato confrontando le due
 *   build istruzione per istruzione: lo stesso `mov`+`movk` costruisce un
 *   valore diverso). La differenza e' esattamente `__GFP_ZERO` (0x8000,
 *   `include/linux/gfp.h`): il fabbrica chiama `devm_kzalloc`, non
 *   `devm_kmalloc` -- `devm_kzalloc` e' una static inline in
 *   `include/linux/device.h` che fa `return devm_kmalloc(dev, size, gfp |
 *   __GFP_ZERO)`, quindi il target della `bl` resta lo stesso
 *   (`devm_kmalloc`, verificato di nuovo) e solo l'immediato del flag
 *   cambia -- questo e' il motivo per cui il nome della funzione chiamata
 *   non tradiva l'errore, solo il valore. g_aw87329 = priv (str
 *   x0,[x22,#752], la stessa .bss del lotto 1/2, adrp x22 sulla pagina
 *   fissa 0xa19c000). if (!priv) return -ENOMEM (mov w21,#0xfffffff4 =
 *   -12, ffffff8008c5928c).
 * ffffff8008c591fc..59200: priv->i2c = client (str x20,[x0]); poi str
 *   x0,[x20,#184] -- offset 184 = 32(dev)+152 dentro struct device:
 *   i2c_set_clientdata(client, priv) (dev_set_drvdata inlineata),
 *   scritta con l'API, non con l'offset a mano.
 * ffffff8008c59204: cbz x21, 0x59294 -- if (!np) { priv->hwen_gpio = -1
 *   (mov w8,#0xffffffff; str w8,[x0,#8]); } salta a 0x5929c.
 * ffffff8008c59208..5921c: else: of_get_named_gpio(np, "reset-gpio"@0x91c72a7,
 *   0) (of_get_named_gpio_flags inlineata, x3=xzr=flags NULL) -- la pagina
 *   0x91c7000 e' condivisa con la funzione gemella aw22xxx_parse_dt;
 *   ffffff8008c59208 adrp x1 / ffffff8008c5920c add x1,x1,#0x2a7.
 *
 *   **Nome di proprieta' del device tree: verificato anche contro il DTB di
 *   fabbrica, non solo contro le stringhe del kernel.** dtbo.img decompilato
 *   (dtbo-prova/fabbrica.dtbo) contiene il nodo
 *
 *       aw87329@59 {
 *               compatible = "awinic,aw87329_pa";
 *               reg = <0x59>;
 *               reset-gpio = <0xffffffff 0x99 0x0>;
 *               status = "okay";
 *       };
 *
 *   e il fixup "/fragment@8/__overlay__/aw87329@59:reset-gpio:0" nella
 *   sezione __fixups__ verso il nodo "pio": la proprieta' si chiama
 *   davvero "reset-gpio" (GPIO 0x99 = 153), non "reset-gpios".
 *
 *   gpio = risultato; priv->hwen_gpio = gpio (str w0,[x8,#8], g_aw87329
 *   ricaricato).
 * ffffff8008c59228: tbnz w0,#31 -> 0x593b0 (gpio<0, fallimento gia'
 *   nella lettura DT).
 * ffffff8008c5922c..59240: dev_info(dev, "%s: reset gpio provided
 *   ok\n"@0x91c72f9, "aw87329_parse_dt"@0x92a3168) -- il nome usato in
 *   QUESTO messaggio (e nei due sotto) e' "aw87329_parse_dt", non
 *   "aw87329_i2c_probe": la stringa esiste (0xffffff80092a3168..317f),
 *   ma nessun simbolo t/T "aw87329_parse_dt" e' in stock.map. Stessa
 *   diagnosi delle *_reg_val del lotto 2: una funzione statica chiamata
 *   una sola volta (da qui), incorporata dal compilatore e sparita dal
 *   symtab. Scritta qui come logica inline dentro aw87329_i2c_probe (non
 *   come funzione a se', che introdurrebbe un simbolo mai esistito),
 *   riusando pero' le stringhe letterali esatte "aw87329_parse_dt" dove
 *   il disassemblato le mostra -- non e' __func__, sono costanti scritte
 *   a mano nel sorgente originale (il testo e' quello, verificato).
 * ffffff8008c59244..59250: reg = priv->hwen_gpio; cmp w1,#0x1ff; b.hi ->
 *   0x5929c (gpio_is_valid() inlineata come nei lotti 1/2: se NON valido,
 *   salta la richiesta e va dritto a hw_reset).
 * ffffff8008c59254..59268: se valido: devm_gpio_request_one(dev, gpio,
 *   GPIOF_OUT_INIT_LOW, "aw87329_rst") -- flags=xzr=0, verificato uguale
 *   a GPIOF_OUT_INIT_LOW=(GPIOF_DIR_OUT|GPIOF_INIT_LOW)=(0<<0)|(0<<1)=0 in
 *   questo albero (kernel-stock/alps-mtkwatch/include/linux/gpio.h);
 *   label "aw87329_rst"@0x92a30f5 (ffffff8008c59254 adrp x3 /
 *   ffffff8008c59258 add x3,x3,#0xf5). cbz w0 -> 0x5929c (successo).
 * ffffff8008c5926c..59288: se fallisce: dev_err(dev, "%s: rst request
 *   failed\n"@0x91c71fe, "aw87329_i2c_probe"@0x92a30c3) -- **corretto dopo
 *   revisione**: la prima stesura scriveva "gpio request failed\n", un
 *   letterale plausibile dal contesto (devm_gpio_request_one fallita) ma
 *   non letto: il testo vero, risolto con objdump -s sullo stesso
 *   indirizzo gia' citato, e' "rst request failed\n" (la pagina e'
 *   condivisa con aw22xxx_parse_dt, che ha lo stesso messaggio). return
 *   ret (mov w21,w0, salta alla cleanup che libera priv, 0x59418).
 * ffffff8008c59294..59298: (ramo "np assente", vedi sopra) priv->hwen_gpio
 *   = -1.
 * ffffff8008c5929c..592a4: aw87329_hw_reset(); ret =
 *   aw87329_read_chipid(); tbnz w0,#31 -> 0x593e8 (fallimento).
 * ffffff8008c592a8..592b4: ret = sysfs_create_group(&dev->kobj,
 *   &aw87329_attribute_group) (add x0,x20,#0x30: offset 48 = 32(dev)+16
 *   dentro struct device per kobj, scritto con l'API). x1 = indirizzo
 *   del gruppo attributi (0x99ca078, dato non decodificabile oltre
 *   l'indirizzo: e' la nostra aw87329_attribute_group).
 * ffffff8008c592b8: tbz w0,#31 -> 0x592d4 (successo, salta il log).
 * ffffff8008c592bc..592d0: se fallisce (ret<0): dev_info(dev, "%s error
 *   creating sysfs attr files\n"@0x92a3128, "aw87329_i2c_probe"@0x92a30c3)
 *   -- livello INFO per un errore, non ERR: comportamento del fabbrica,
 *   riprodotto cosi' com'e' (non un bug di nome sbagliato come in
 *   audio_off, solo un livello di log discutibile). La funzione NON
 *   fallisce per questo: continua comunque.
 * ffffff8008c592d4..592e8: priv->kspk_reg_loaded = 0; priv->drcv_reg_loaded
 *   = 0 (strh wzr,[x8,#14], le due u8 adiacenti scritte insieme dal
 *   compilatore); priv->abrcv_reg_loaded = 0 (strb wzr,[x8,#16]).
 * ffffff8008c592ec: hrtimer_init(&priv->timer, CLOCK_MONOTONIC,
 *   HRTIMER_MODE_REL) -- w1=1 (clockid), w2=1 (mode); add x0,x8,#0x18
 *   (timer a offset 0x18, vedi la struct sopra).
 * ffffff8008c592f4..5931c: priv->timer.function = cfg_timer_func (str
 *   x9,[x8,#64], x9 = adrp 0x8c59000 + #0xe68 = 0xffffff8008c59e68 =
 *   cfg_timer_func, risolto in stock.map); costruzione dell'immediato a
 *   64 bit x1 = 0x00012a05f200 = 5.000.000.000 (mov+movk+movk) -- 5
 *   secondi in nanosecondi, il ktime passato sotto.
 * ffffff8008c59318..59330: INIT_WORK(&priv->work, cfg_work_routine) --
 *   x11 = adrp 0x8c59000 + #0xeac = 0xffffff8008c59eac = cfg_work_routine
 *   (risolto in stock.map); le altre store (offset 88/96/104) sono
 *   l'inizializzazione di atomic_long_t data e della list_head interna
 *   di work_struct fatta dalla macro, non calcolate a mano.
 * ffffff8008c59334: hrtimer_start_range_ns(&priv->timer,
 *   ns_to_ktime(5000000000ULL), 0, HRTIMER_MODE_REL) -- w3=1 (mode),
 *   x2=xzr (range_ns=0).
 * ffffff8008c59338..593a0: **aw87329_hw_off() chiamata e incorporata**:
 *   stesso identico corpo del lotto 1 (printk "%s enter\n"@0x92a2f79 +
 *   "aw87329_hw_off"@0x92a2f93, guardia g_aw87329&&gpio_is_valid,
 *   gpiod_set_raw_value_cansleep+msleep(2)+hwen_flag=0, altrimenti
 *   dev_err "%s:  failed\n"@0x91c7380 con lo stesso nome
 *   "aw87329_hw_off") -- non c'e' nessuna "bl aw87329_hw_off" qui: il
 *   compilatore ha incorporato la chiamata (stessa diagnosi delle
 *   *_reg_val nel lotto 2, applicata stavolta a una funzione GLOBALE
 *   chiamata da un solo altro punto in questo file). Scritta come
 *   chiamata vera aw87329_hw_off() nel sorgente: e' la lettura piu'
 *   semplice e quella che il nome della stringa conferma.
 * ffffff8008c593a0..593ac: priv->inited = 1 (strb w9,[x8,#12]); return 0
 *   (mov w21,wzr) -- **sempre**, anche se il ramo hw_off e' fallito
 *   internamente (i due percorsi convergono qui prima del return).
 * ffffff8008c593b0..593e4: primo fallimento gpio (gpio<0 dalla lettura
 *   DT): DUE dev_err in sequenza -- "%s: no reset gpio provided\n"@0x92a314c
 *   (ffffff8008c593b0 adrp x1 / ffffff8008c593b8 add x1,x1,#0x14c) con nome
 *   "aw87329_parse_dt"@0x92a3168, poi "%s: failed to
 *   parse device tree node\n"@0x91c71cc (pagina condivisa con
 *   aw22xxx_hw_reset) con nome "aw87329_i2c_probe"@0x92a30c3; return -1
 *   (mov w21,#0xffffffff).
 * ffffff8008c593e8..59418: fallimento read_chipid: dev_err(dev, "%s:
 *   aw87329_read_chipid failed ret=%d\n"@0x92a3101, "aw87329_i2c_probe",
 *   ret); poi devm_gpio_free(dev, priv->hwen_gpio) (solo su questo
 *   percorso: il gpio era stato richiesto prima di arrivare qui).
 * ffffff8008c59418..59428: cleanup finale comune a tutti i percorsi di
 *   errore dopo l'allocazione riuscita: devm_kfree(dev, priv); g_aw87329
 *   = NULL (str xzr,[x22,#752]); return w21.
 *
 * **Corretto dopo revisione**: il gfp `0x80c0|(0x140<<16)` di
 * `devm_kzalloc`/`kzalloc` (qui e nelle tre `*_cfg_loaded` sotto) e il gfp
 * `0xc0|(0x140<<16)` delle `request_firmware_nowait` NON sono lo stesso
 * immediato a 32 bit (0x014080c0 contro 0x014000c0) -- la prima stesura
 * liquidava la differenza come "interna a come ciascun percorso combina i
 * bit", **sbagliato**: la differenza e' esattamente `__GFP_ZERO` (0x8000),
 * e ha un nome, non e' rumore -- vedi sopra: le allocazioni sono davvero
 * "zeroing" (`kzalloc`/`devm_kzalloc`), le `request_firmware_nowait` no
 * (il loro argomento gfp e' `GFP_KERNEL` semplice, nessuna zeroing
 * richiesta all'API).
 */
static int aw87329_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct device_node *np = client->dev.of_node;
	struct device *dev = &client->dev;
	struct aw87329 *priv;
	int gpio;
	int ret;

	printk(KERN_INFO "%s Enter\n", "aw87329_i2c_probe");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(dev, "%s: check_functionality failed\n", "aw87329_i2c_probe");
		return -ENODEV;
	}

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	g_aw87329 = priv;
	if (!priv)
		return -ENOMEM;

	priv->i2c = client;
	i2c_set_clientdata(client, priv);

	if (np) {
		gpio = of_get_named_gpio(np, "reset-gpio", 0);
		priv->hwen_gpio = gpio;
		if (gpio < 0) {
			dev_err(dev, "%s: no reset gpio provided\n", "aw87329_parse_dt");
			dev_err(dev, "%s: failed to parse device tree node\n",
				"aw87329_i2c_probe");
			ret = -1;
			goto err;
		}

		dev_info(dev, "%s: reset gpio provided ok\n", "aw87329_parse_dt");

		if (gpio_is_valid(priv->hwen_gpio)) {
			ret = devm_gpio_request_one(dev, priv->hwen_gpio,
						     GPIOF_OUT_INIT_LOW, "aw87329_rst");
			if (ret) {
				dev_err(dev, "%s: rst request failed\n", "aw87329_i2c_probe");
				goto err;
			}
		}
	} else {
		priv->hwen_gpio = -1;
	}

	aw87329_hw_reset();
	ret = aw87329_read_chipid();
	if (ret < 0) {
		dev_err(dev, "%s: aw87329_read_chipid failed ret=%d\n",
			"aw87329_i2c_probe", ret);
		devm_gpio_free(dev, g_aw87329->hwen_gpio);
		goto err;
	}

	ret = sysfs_create_group(&dev->kobj, &aw87329_attribute_group);
	if (ret < 0)
		dev_info(dev, "%s error creating sysfs attr files\n", "aw87329_i2c_probe");

	priv->kspk_reg_loaded = 0;
	priv->drcv_reg_loaded = 0;
	priv->abrcv_reg_loaded = 0;

	hrtimer_init(&priv->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	priv->timer.function = cfg_timer_func;
	INIT_WORK(&priv->work, cfg_work_routine);
	hrtimer_start_range_ns(&priv->timer, ns_to_ktime(5000000000ULL), 0, HRTIMER_MODE_REL);

	/* aw87329_hw_off() incorporata dal compilatore, vedi sopra. */
	aw87329_hw_off();

	priv->inited = 1;
	return 0;

err:
	/* CODA D'ERRORE COMUNE a tutti i percorsi dopo l'allocazione riuscita.
	 * La fabbrica la mostra a 0xffffff8008c59418..59428: rilegge
	 * g_aw87329 ("f9417ac1 ldr"@0xffffff8008c59418), chiama devm_kfree
	 * ("97e262eb bl"@0xffffff8008c59420) e azzera il globale. Senza,
	 * la struttura resta allocata e g_aw87329 punta a memoria liberata
	 * dal devm alla rimozione del dispositivo. */
	devm_kfree(dev, g_aw87329);
	g_aw87329 = NULL;
	return ret;
}

/*
 * aw87329_i2c_remove: 0xffffff8008c5943c, 48 byte, statica (t).
 *
 * ffffff8008c5943c..59448: gpio = g_aw87329->hwen_gpio (ldr x8,[x8,#752];
 *   ldr w1,[x8,#8]); cmp w1,#0x1ff; b.hi -> 0x59464 (gpio_is_valid()
 *   inlineata come sempre: se non valido, salta direttamente al return).
 * ffffff8008c59450..5945c: se valido: devm_gpio_free(&client->dev, gpio)
 *   (add x0,x0,#0x20, x1=w1 gia' caricato).
 * ffffff8008c59464..59468: return 0.
 */
static int aw87329_i2c_remove(struct i2c_client *client)
{
	if (gpio_is_valid(g_aw87329->hwen_gpio))
		devm_gpio_free(&client->dev, g_aw87329->hwen_gpio);

	return 0;
}

/*
 * aw87329_get_reg: 0xffffff8008c5946c, 128 byte, statica (t).
 *
 * Elenca i registri 0..10 (undici, la stessa quantita' delle tabelle di
 * default del lotto 2), leggendoli via i2c_read_reg, in un ciclo che pero'
 * conta fino a 15 saltando (continue, non break) gli indici 11..14:
 *
 * ffffff8008c59480..59494: w23 = 0x1000 (PAGE_SIZE, orr w23,wzr,#0x1000);
 *   x22 = &"reg:0x%02x=0x%02x\n"@0x92a3179 (ffffff8008c59480 adrp x22 sulla
 *   pagina 0xffffff80092a3000 / ffffff8008c59494 add x22,x22,#0x179 --
 *   caricata una volta sola FUORI dal ciclo, il che e' anche la conferma
 *   che il ciclo c'e').
 * ffffff8008c59498..5949c: cmp w21,#0xa; b.hi -> 0x594c8 (se indice>10,
 *   salta il corpo, va all'incremento: continue).
 * ffffff8008c594a0..594c4: val = i2c_read_reg(i) & 0xff (and w4,w0,#0xff);
 *   len += snprintf(buf+len, PAGE_SIZE-len, "reg:0x%02x=0x%02x\n", i,
 *   val) (x1 = x23-x19 = size rimanente, x0 = x20+x19 = buf+len).
 * ffffff8008c594c8..594d0: i++; cmp w21,#0xf(15); b.ne -> torna al loop
 *   (continua finche' i != 15).
 * ffffff8008c594d4: return len (x19, sxtw accumulato).
 */
static ssize_t aw87329_get_reg(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;
	unsigned char val;
	unsigned char i;

	for (i = 0; i < 15; i++) {
		if (i > 10)
			continue;

		val = i2c_read_reg(i) & 0xff;
		len += snprintf(buf + len, PAGE_SIZE - len, "reg:0x%02x=0x%02x\n", i, val);
	}

	return len;
}

/*
 * aw87329_set_reg: 0xffffff8008c594ec, 120 byte, statica (t).
 *
 * ffffff8008c594fc..59524: canary dello stack caricato/salvato (pattern
 *   __stack_chk_guard, comune a tutte le funzioni con array locali di
 *   questo lotto: non e' un valore del driver, non lo commentiamo di
 *   nuovo sotto); sscanf(buf, "%x %x"@0x91411fa, &reg, &val).
 *
 *   **Citazione DEBOLE, e per questo accompagnata dalla prova.** I byte
 *   "%x %x\0" compaiono DUE volte in stock.elf -- 0xffffff80091411fa e
 *   0xffffff8009272e3f -- quindi la sola corrispondenza byte-per-byte non
 *   distingue i due indirizzi: l'altro avrebbe "verificato" altrettanto
 *   bene. Che sia il primo lo dice l'istruzione, non il testo:
 *   ffffff8008c59508 adrp x1, ffffff8009141000 / ffffff8008c59518 add
 *   x1,x1,#0x1fa -- e' questa coppia, dentro questa funzione, a
 *   costruire l'argomento di sscanf.
 * ffffff8008c59528..59534: cmp w0,#0x2; b.ne -> salta la scrittura (se
 *   sscanf non ha convertito esattamente due valori); altrimenti
 *   i2c_write_reg(reg, val) (ldp w0,w1,[sp]).
 * ffffff8008c59538..5955c: verifica canary, return count (x19 = x3,
 *   invariato).
 */
static ssize_t aw87329_set_reg(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned int reg = 0;
	unsigned int val = 0;

	if (sscanf(buf, "%x %x", &reg, &val) == 2)
		i2c_write_reg(reg, val);

	return count;
}

/*
 * aw87329_get_hwen: 0xffffff8008c59564, 52 byte, statica (t).
 *
 * ffffff8008c5956c..59588: snprintf(buf, PAGE_SIZE, "hwen: %d\n"@0x92a318c,
 *   g_aw87329->hwen_flag) -- ffffff8008c59578 adrp x2 / ffffff8008c5957c add
 *   x2,x2,#0x18c; valore = ldrb
 *   w3,[x8,#13] (hwen_flag, offset verificato nel lotto 1/2); w1=0x1000
 *   (PAGE_SIZE).
 * ffffff8008c5958c: sxtw x0,w0 -- return (ssize_t)ret.
 */
static ssize_t aw87329_get_hwen(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "hwen: %d\n", g_aw87329->hwen_flag);
}

/*
 * aw87329_set_hwen: 0xffffff8008c59598, 216 byte, statica (t).
 *
 * ffffff8008c595a8..595c8: sscanf(buf, "%d"@0x9216030, &val) -- due byte,
 *   "%d\0", non una stringa piu' lunga.
 *
 *   **Citazione DEBOLE, la piu' debole del file, e per questo accompagnata
 *   dalla prova.** I byte "%d\0" compaiono **521 volte** in stock.elf: che
 *   esistano a 0x9216030 non prova quasi nulla da solo -- 520 altri
 *   indirizzi avrebbero "verificato" allo stesso modo. Quello che lo prova
 *   e' l'istruzione, ed e' la stessa pagina in tutti e tre i punti d'uso:
 *     set_hwen  : ffffff8008c595b0 adrp x1, ffffff8009216000 /
 *                 ffffff8008c595b8 add x1,x1,#0x30
 *     set_update: ffffff8008c59690 adrp x1 / ffffff8008c59698 add #0x30
 *     set_mode  : ffffff8008c59de0 adrp x1 / ffffff8008c59de8 add #0x30
 *   Tre `sscanf` diversi che costruiscono lo stesso indirizzo: la stringa
 *   e' condivisa nel fabbrica, non triplicata.
 * ffffff8008c595cc: ldr w8,[sp,#8] (rilegge val appena scritto da sscanf).
 * ffffff8008c595d0..595d8: cbz w8 -> 0x595dc (val==0); altrimenti (val
 *   diverso da zero): bl aw87329_hw_on() -- chiamata vera, non
 *   incorporata (a differenza del ramo sotto).
 * ffffff8008c595dc..59644: **aw87329_hw_off() incorporata di nuovo**,
 *   identica a quella vista in aw87329_i2c_probe (stesse stringhe
 *   "aw87329_hw_off"@0x92a2f93, "%s:  failed\n"@0x91c7380): il fabbrica
 *   qui NON emette una "bl aw87329_hw_off", mentre per hw_on la emette --
 *   asimmetria del compilatore fra i due punti di chiamata, non del
 *   sorgente (che con ogni probabilita' chiama entrambe allo stesso
 *   modo, come scritto qui sotto).
 * ffffff8008c59644..59668: verifica canary, return count.
 */
static ssize_t aw87329_set_hwen(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count)
{
	/*
	 * NON AZZERATA, e il binario lo dice: prima della
	 * "94084887 bl"@0xffffff8008c595c8 verso <sscanf> la fabbrica non
	 * scrive niente nello slot che le passa ("910023e2 add"@0xffffff8008c595bc
	 * mette x2 = sp+8, e l'unica scrittura la fa sscanf). Da noi `= 0`
	 * emetteva una "b90007ff str wzr, [sp,#4]" in piu': quattro byte, ed
	 * erano esattamente i quattro che separavano questa funzione dai 216
	 * di fabbrica.
	 *
	 * DIFETTO DELLA FABBRICA, riprodotto: se `sscanf` non converte niente
	 * la variabile resta indefinita e viene usata lo stesso.
	 *
	 * ESTESO A `set_update`, e la prima stesura di questa nota sbagliava.
	 * Diceva che le sorelle hanno «divergenza di SEGNO OPPOSTO» e portava la
	 * misura di UNA sola: e' vero di `set_mode` (156 contro 160, le servirebbe
	 * un'istruzione in piu') e FALSO di `set_update`, che misurava 204 contro
	 * 200 -- lo stesso segno e la stessa entita' che aveva `set_hwen`. Tolto
	 * anche li' l'azzeramento, `set_update` va a 200 ESATTA e il file passa da
	 * 23 a 24 funzioni su 29 uguali al byte col compilatore di fabbrica.
	 *
	 * `set_mode` resta divergente, e li' la causa e' davvero un'altra.
	 *
	 * I SESSANTAQUATTRO BYTE DI TELAIO, e cio' che di essi e' MISURATO.
	 * Tutte e tre le `set_*` riservano 0x70 dove noi riserviamo 0x30, ed e'
	 * esattamente lo stesso scarto in tutte e tre. Ma NON LI TOCCANO MAI:
	 * in tutte e tre gli unici accessi alla pila sono lo slot di `sscanf`
	 * (sp+8), il canarino e i registri salvati --
	 *   set_update: "f9002bf3 str"@0xffffff8008c5967c (x19 a sp+80),
	 *               "b9400be8 ldr"@0xffffff8008c596ac (sp+8), e basta;
	 *   set_mode:   le stesse due a 0xffffff8008c59dcc e 0xffffff8008c59dfc.
	 * Sessantaquattro byte riservati e mai letti ne' scritti: nel sorgente
	 * di fabbrica c'e' un oggetto locale che il compilatore non ha potuto
	 * togliere del tutto. Quale, il binario non lo dice -- e un telaio piu'
	 * grande non costa istruzioni, quindi la dimensione non ne risente.
	 */
	unsigned int val;

	sscanf(buf, "%d", &val);
	if (val)
		aw87329_hw_on();
	else
		aw87329_hw_off();

	return count;
}

/*
 * aw87329_get_update: 0xffffff8008c59670, 8 byte, statica (t).
 *
 * ffffff8008c59670..59674: mov x0,xzr; ret -- **return 0, senza
 *   leggere ne' dev ne' attr ne' buf**. Due istruzioni, 8 byte esatti.
 *   Non e' un errore di lettura nostro: il "get" di questo attributo, di
 *   fabbrica, non scrive mai nulla nel buffer sysfs (il file, letto,
 *   risulta sempre vuoto). Comportamento riprodotto cosi' com'e'.
 */
static ssize_t aw87329_get_update(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

/*
 * aw87329_set_update: 0xffffff8008c59678, 200 byte, statica (t).
 *
 * ffffff8008c59688..596ac: sscanf(buf, "%d", &val) -- stesso fmt
 *   "%d"@0x9216030 di set_hwen; cbz w8 -> 0x59714 (val==0, nessuna
 *   azione, salta dritto al return).
 * ffffff8008c596b4..596ec: se val diverso da zero: priv->kspk_reg_loaded
 *   = 0; priv->drcv_reg_loaded = 0 (strh wzr,[x8,#14], le due insieme);
 *   priv->abrcv_reg_loaded = 0 (strb wzr,[x8,#16]) -- stessa coppia di
 *   store vista in aw87329_i2c_probe: azzera i flag "caricato" prima di
 *   richiedere un firmware nuovo.
 * ffffff8008c596f0: request_firmware_nowait(NULL, true,
 *   "aw87329_kspk.bin"@0x92a31db, &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
 *   aw87329_kspk_cfg_loaded) -- sette argomenti letti uno per uno: x0=xzr
 *   (module=NULL: THIS_MODULE compila a NULL perche' questo file e'
 *   built-in, non modulo, "#else #define THIS_MODULE ((struct module
 *   *)0)" in include/linux/export.h), w1=1(uevent=true), x2=nome file,
 *   x3=x9+0x20=&i2c->dev (x9=ldr x9,[x8], g_aw87329->i2c), w4=gfp, x5=xzr
 *   (context=NULL), x6=adrp(aw87329_hw_reset base)+#0x740 =
 *   0xffffff8008c59740 = aw87329_kspk_cfg_loaded (risolto in stock.map).
 * ffffff8008c596f4..59710: cbz w0 -> 0x59714 (successo, salta il log);
 *   altrimenti printk(KERN_ERR "%s: request_firmware_nowait failed with
 *   read %s"@0x92a3196, "aw87329_set_update"@0x92a31c8,
 *   "aw87329_kspk.bin"@0x92a31db).
 * **Corretto da tools/verificacitazioni.py**: la prima stesura scriveva un
 *   "\n" finale in questa stringa (qui e nelle altre tre chiamate identiche
 *   sotto) che nel binario di fabbrica non c'e' -- il byte dopo "%s" e'
 *   direttamente il terminatore, letto e confermato byte per byte da
 *   0xffffff80092a3196.
 * ffffff8008c59714..59738: verifica canary, return count.
 */
static ssize_t aw87329_set_update(struct device *dev, struct device_attribute *attr,
				   const char *buf, size_t count)
{
	unsigned int val;
	int ret;

	sscanf(buf, "%d", &val);
	if (val) {
		g_aw87329->kspk_reg_loaded = 0;
		g_aw87329->drcv_reg_loaded = 0;
		g_aw87329->abrcv_reg_loaded = 0;

		ret = request_firmware_nowait(NULL, true, "aw87329_kspk.bin",
					       &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
					       aw87329_kspk_cfg_loaded);
		if (ret)
			printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
			       "aw87329_set_update", "aw87329_kspk.bin");
	}

	return count;
}

/*
 * aw87329_kspk_cfg_loaded: 0xffffff8008c59740, 576 byte, statica (t).
 * Callback di completamento di request_firmware_nowait: void (*cont)(const
 * struct firmware *fw, void *context) -- x0=fw, x1=context (mai letto).
 *
 * ffffff8008c59760: cbz x0 -> 0x598c8 (fw NULL: lettura fallita).
 * ffffff8008c59764..59784: (fw non NULL) x3 = fw->size (ldr x3,[x0],
 *   offset 0 di struct firmware, verificato in
 *   alps-mtkwatch/include/linux/firmware.h); printk(KERN_INFO "%s: loaded
 *   %s - size: %zu\n"@0x91c77d4, "aw87329_kspk_cfg_loaded"@0x92a31ec,
 *   "aw87329_kspk.bin"@0x92a31db, fw->size).
 * ffffff8008c59788: ldr x8,[x19] (rilegge fw->size, x19=fw salvato dal
 *   prologo).
 * ffffff8008c5978c..597cc: cbz x8 (size==0) salta il ciclo sotto; altrimenti
 *   per i in 0..fw->size: printk(KERN_INFO "%s: cont: addr:0x%02x,
 *   data:0x%02x\n"@0x92a3204, nome, i, fw->data[i]) (x8=fw->data, offset
 *   8 di struct firmware).
 * ffffff8008c597cc..597d8: aw87329_kspk_reg = kzalloc(fw->size + 4,
 *   GFP_KERNEL) -- **corretto dopo revisione, kzalloc non kmalloc**: w1
 *   costruisce 0x80c0|(0x140<<16), che include __GFP_ZERO (0x8000, vedi
 *   sopra in aw87329_i2c_probe); kzalloc() e' `kmalloc(size, flags |
 *   __GFP_ZERO)` (static inline, include/linux/slab.h), e kmalloc() con
 *   dimensione non costante a compile time compila in una bl __kmalloc,
 *   verificato: e' il target della bl qui, invariato. Risultato salvato nella
 *   .bss a offset 760 (str x0,[x24,#760], la stessa variabile statica
 *   del lotto 2).
 * ffffff8008c597e4: cbz x0 -> 0x5994c (kmalloc fallito).
 * ffffff8008c597e8..597fc: *(u32*)aw87329_kspk_reg = fw->size (str
 *   w8,[x0], scrittura a 32 bit, non 64: conferma il formato "header di 4
 *   byte" gia' dedotto nel lotto 2 da aw87329_kspk_reg[index+4]);
 *   memcpy(aw87329_kspk_reg+4, fw->data, fw->size) (bl __memcpy, x0=dest,
 *   x1=src, x2=size -- src e size ricaricati da [x19]/[x19,#8], lo stesso
 *   registro/offset del fw ancora valido).
 * ffffff8008c59800..59804: release_firmware(fw).
 * ffffff8008c59808..59884: **stesso corpo di aw87329_kspk_reg_val(),
 *   incorporato di nuovo** (identico csel fra tabella caricata e tabella
 *   di default, stesso controllo di g_aw87329->kspk_reg_loaded a offset
 *   0xe, verificato istruzione per istruzione uguale al lotto 2): per i
 *   in 0..(header appena scritto, riletto da aw87329_kspk_reg[0], non
 *   piu' da fw->size che non e' piu' valido dopo release_firmware):
 *   printk(KERN_INFO "%s: spk_cnt: addr:0x%02x, data:0x%02x\n"@0x92a3249,
 *   nome, i, aw87329_kspk_reg_val(i)). Scritta come chiamata alla
 *   funzione vera del lotto 2, non come csel a mano: stessa convenzione
 *   gia' adottata per aw87329_audio_kspk.
 * ffffff8008c59818: cbz w10, 0x59888 (header==0) salta il ciclo sopra
 *   (nessuna diagnosi da stampare se la dimensione e' zero) -- NON e'
 *   l'indirizzo 0x59888 stesso ad ospitare la cbz, e' il suo BERSAGLIO:
 *   la cbz vera sta 0x70 prima, appena dopo la ldr che rilegge l'header.
 *   Entrambi i rami (ciclo eseguito o saltato) confluiscono comunque allo
 *   stesso punto, 0x59888, prima di procedere.
 * ffffff8008c59888..598b8: priv->kspk_reg_loaded = 1 (strb w10,[x9,#14],
 *   w10=1); request_firmware_nowait(NULL, true,
 *   "aw87329_drcv.bin"@0x92a3272, &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
 *   aw87329_drcv_cfg_loaded) -- x6 = adrp(hw_reset base)+#0x980 =
 *   0xffffff8008c59980 = aw87329_drcv_cfg_loaded.
 * ffffff8008c598bc..598c4: cbz w0 -> fine (successo); altrimenti ricarica
 *   g_aw87329 e salta a 0x59928 (blocco di errore condiviso, vedi sotto).
 * ffffff8008c598c8..598e0: (fw NULL) printk(KERN_ERR "%s: failed to read
 *   %s\n"@0x91c77bb, "aw87329_kspk_cfg_loaded", "aw87329_kspk.bin").
 * ffffff8008c598e4..598e8: release_firmware(NULL) (mov x0,xzr; bl
 *   release_firmware -- sicuro, release_firmware ha un controllo NULL
 *   interno).
 * ffffff8008c598ec..59920: **stessa** request_firmware_nowait di sopra
 *   per aw87329_drcv.bin (stesso target, stesso nome file) -- ma SENZA
 *   impostare kspk_reg_loaded=1 prima (coerente: qui fw era NULL, non
 *   c'e' nulla di "caricato" da segnalare). cbz w0 -> fine; altrimenti
 *   cade nello stesso blocco di errore 0x59928.
 * ffffff8008c59928..59948: blocco di errore condiviso da entrambi i
 *   percorsi sopra: printk(KERN_ERR "%s: request_firmware_nowait failed
 *   with read %s"@0x92a3196, "aw87329_kspk_cfg_loaded"@0x92a31ec,
 *   "aw87329_drcv.bin"@0x92a3272); priv->drcv_reg_loaded = 0 (strb
 *   wzr,[x8,#15], difensivo).
 * ffffff8008c5994c..59964: (kmalloc fallito) release_firmware(fw);
 *   printk(KERN_ERR "%s: error allocating memory\n"@0x92a322a,
 *   "aw87329_kspk_cfg_loaded") -- **nessun tentativo di richiedere
 *   drcv.bin in questo ramo**: un kmalloc fallito interrompe la catena
 *   per intero, a differenza di un fw NULL che invece la fa proseguire.
 */
static void aw87329_kspk_cfg_loaded(const struct firmware *fw, void *context)
{
	unsigned int i;
	size_t len;
	int ret;

	if (!fw) {
		printk(KERN_ERR "%s: failed to read %s\n",
		       "aw87329_kspk_cfg_loaded", "aw87329_kspk.bin");
		release_firmware(fw);
		/*
		 * IL RAMO D'ERRORE HA LA SUA `request_firmware_nowait`, NON un
		 * `goto` a quella in coda. Nel binario di fabbrica le chiamate a
		 * <request_firmware_nowait> sono DUE
		 * ("97e2baf4 bl"@0xffffff8008c598b8 e
		 * "97e2badb bl"@0xffffff8008c5991c) e ognuna e' seguita dal suo
		 * controllo ("34000560 cbz"@0xffffff8008c598bc e
		 * "34000240 cbz"@0xffffff8008c59920). Col `goto` che c'era prima
		 * clang le fondeva in una sola: -48 byte.
		 *
		 * RESTA +28, E LA RAGIONE E' MISURATA. La fabbrica ha DUE chiamate
		 * e UNA sola coda di errore: dal primo sito
		 * "14000019 b"@0xffffff8008c598c4 salta dentro la coda del
		 * secondo. Noi le teniamo separate, e sono sette istruzioni.
		 * Il perche' e' visibile nel binario: i due siti leggono
		 * `g_aw87329` in registri DIVERSI ("f9417ac8 ldr" da x22 al primo,
		 * "f9417a68 ldr" da x19 al secondo), cioe' nel sorgente di
		 * fabbrica i due percorsi arrivano li' con stati diversi, e clang
		 * ha potuto fondere solo da quel punto in poi. La forma che
		 * produce quella differenza di stato non e' stata trovata.
		 *
		 * TRE FORME PROVATE, coi numeri:
		 *   `goto` alla coda unica          -48   (una chiamata sola)
		 *   due chiamate, controllo solo    -8    (ma il binario il
		 *     nella seconda                        controllo lo mostra
		 *                                          dopo TUTTE E DUE)
		 *   due chiamate, due controlli     +28   (questa)
		 */
		ret = request_firmware_nowait(NULL, true, "aw87329_drcv.bin",
					       &g_aw87329->i2c->dev, GFP_KERNEL,
					       NULL, aw87329_drcv_cfg_loaded);
		if (ret) {
			printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
			       "aw87329_kspk_cfg_loaded", "aw87329_drcv.bin");
			g_aw87329->drcv_reg_loaded = 0;
		}
		return;
	}

	printk(KERN_INFO "%s: loaded %s - size: %zu\n",
	       "aw87329_kspk_cfg_loaded", "aw87329_kspk.bin", fw->size);

	for (i = 0; i < fw->size; i++)
		printk(KERN_INFO "%s: cont: addr:0x%02x, data:0x%02x\n",
		       "aw87329_kspk_cfg_loaded", i, fw->data[i]);

	len = fw->size;
	aw87329_kspk_reg = kzalloc(len + 4, GFP_KERNEL);
	if (!aw87329_kspk_reg) {
		release_firmware(fw);
		printk(KERN_ERR "%s: error allocating memory\n", "aw87329_kspk_cfg_loaded");
		return;
	}

	*(u32 *)aw87329_kspk_reg = fw->size;
	memcpy(aw87329_kspk_reg + 4, fw->data, fw->size);
	release_firmware(fw);

	for (i = 0; i < len; i++)
		printk(KERN_INFO "%s: spk_cnt: addr:0x%02x, data:0x%02x\n",
		       "aw87329_kspk_cfg_loaded", i, aw87329_kspk_reg_val(i));

	g_aw87329->kspk_reg_loaded = 1;

	ret = request_firmware_nowait(NULL, true, "aw87329_drcv.bin",
				       &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
				       aw87329_drcv_cfg_loaded);
	if (ret) {
		printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
		       "aw87329_kspk_cfg_loaded", "aw87329_drcv.bin");
		g_aw87329->drcv_reg_loaded = 0;
	}
}

/*
 * aw87329_drcv_cfg_loaded: 0xffffff8008c59980, 576 byte, statica (t).
 * Identica a aw87329_kspk_cfg_loaded, sostituendo: campo drcv_reg_loaded
 * (offset 0xf invece di 0xe), puntatore aw87329_drcv_reg (.bss offset 768
 * invece di 760), tabella aw87329_drcv_reg_default (.rodata #0xbe),
 * funzione aw87329_drcv_reg_val, nome proprio
 * "aw87329_drcv_cfg_loaded"@0x92a3283 (ffffff8008c599b0 adrp x1 /
 * ffffff8008c599bc add x1,x1,#0x283), file proprio
 * "aw87329_drcv.bin"@0x92a3272 (lo stesso letto come argomento in
 * ingresso), file successivo "aw87329_abrcv.bin"@0x92a32c4 e relativo
 * target aw87329_abrcv_cfg_loaded (adrp(hw_reset base)+#0xbc0 =
 * 0xffffff8008c59bc0, risolto in stock.map). Il ciclo di log dei valori
 * risolti usa "%s: rcv_cnt: addr:0x%02x, data:0x%02x\n"@0x92a329b
 * (ffffff8008c59a60 adrp x20 / ffffff8008c59a74 add x20,x20,#0x29b) --
 * lo stesso indirizzo che abrcv_cfg_loaded ricostruisce sotto
 * (ffffff8008c59c98 adrp x20 / ffffff8008c59ca8 add x20,x20,#0x29b):
 * nessuna stringa "drcv_cnt" o "abrcv_cnt" separata esiste, ed e' una
 * lettura, non un'assunzione.
 */
static void aw87329_drcv_cfg_loaded(const struct firmware *fw, void *context)
{
	unsigned int i;
	size_t len;
	int ret;

	if (!fw) {
		printk(KERN_ERR "%s: failed to read %s\n",
		       "aw87329_drcv_cfg_loaded", "aw87329_drcv.bin");
		release_firmware(fw);
		/* Come in `aw87329_kspk_cfg_loaded`: la chiamata e' ripetuta qui,
		 * non c'e' un `goto` alla coda. Vedi li' la nota per esteso. */
		ret = request_firmware_nowait(NULL, true, "aw87329_abrcv.bin",
					       &g_aw87329->i2c->dev, GFP_KERNEL,
					       NULL, aw87329_abrcv_cfg_loaded);
		if (ret) {
			printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
			       "aw87329_drcv_cfg_loaded", "aw87329_abrcv.bin");
			g_aw87329->abrcv_reg_loaded = 0;
		}
		return;
	}

	printk(KERN_INFO "%s: loaded %s - size: %zu\n",
	       "aw87329_drcv_cfg_loaded", "aw87329_drcv.bin", fw->size);

	for (i = 0; i < fw->size; i++)
		printk(KERN_INFO "%s: cont: addr:0x%02x, data:0x%02x\n",
		       "aw87329_drcv_cfg_loaded", i, fw->data[i]);

	len = fw->size;
	aw87329_drcv_reg = kzalloc(len + 4, GFP_KERNEL);
	if (!aw87329_drcv_reg) {
		release_firmware(fw);
		printk(KERN_ERR "%s: error allocating memory\n", "aw87329_drcv_cfg_loaded");
		return;
	}

	*(u32 *)aw87329_drcv_reg = fw->size;
	memcpy(aw87329_drcv_reg + 4, fw->data, fw->size);
	release_firmware(fw);

	for (i = 0; i < len; i++)
		printk(KERN_INFO "%s: rcv_cnt: addr:0x%02x, data:0x%02x\n",
		       "aw87329_drcv_cfg_loaded", i, aw87329_drcv_reg_val(i));

	g_aw87329->drcv_reg_loaded = 1;

	ret = request_firmware_nowait(NULL, true, "aw87329_abrcv.bin",
				       &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
				       aw87329_abrcv_cfg_loaded);
	if (ret) {
		printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
		       "aw87329_drcv_cfg_loaded", "aw87329_abrcv.bin");
		g_aw87329->abrcv_reg_loaded = 0;
	}
}

/*
 * aw87329_abrcv_cfg_loaded: 0xffffff8008c59bc0, 416 byte, statica (t).
 * Piu' corta delle due sopra: e' l'ultima della catena, non richiede
 * nessun firmware successivo.
 *
 * Stessa struttura per il caricamento (nome
 * "aw87329_abrcv_cfg_loaded"@0x92a32d6 -- ffffff8008c59be4 adrp x1 /
 * ffffff8008c59bf0 add x1,x1,#0x2d6 --, file "aw87329_abrcv.bin"@0x92a32c4,
 * campo abrcv_reg_loaded
 * offset 0x10, puntatore aw87329_abrcv_reg .bss offset 776, tabella
 * aw87329_abrcv_reg_default .rodata #0xc9, funzione
 * aw87329_abrcv_reg_val, formato di log "rcv_cnt" riusato da drcv, vedi
 * sopra). Diverge dopo il ciclo di log:
 *
 * ffffff8008c59c88..59cfc: se la dimensione (riletta dall'header) e' zero,
 *   salta il ciclo; in ogni caso, dopo (ciclo eseguito o no):
 * ffffff8008c59cfc..59d04: priv->abrcv_reg_loaded = 1 (orr w8,wzr,#0x1;
 *   strb w8,[x9], x9 = g_aw87329+0x10) **incondizionato**, poi fine --
 *   **nessuna request_firmware_nowait qui**: la catena kspk->drcv->abrcv
 *   termina.
 * ffffff8008c59d08..59d2c: (fw NULL) printk(KERN_ERR "%s: failed to read
 *   %s\n"@0x91c77bb, nome, file); release_firmware(NULL); fine -- flag
 *   NON impostato (resta 0, coerente: niente da segnalare come caricato).
 * ffffff8008c59d30..59d48: (kmalloc fallito) release_firmware(fw);
 *   printk(KERN_ERR "%s: error allocating memory\n"@0x92a322a, nome);
 *   fine -- flag NON impostato.
 */
static void aw87329_abrcv_cfg_loaded(const struct firmware *fw, void *context)
{
	unsigned int i;
	size_t len;

	if (!fw) {
		printk(KERN_ERR "%s: failed to read %s\n",
		       "aw87329_abrcv_cfg_loaded", "aw87329_abrcv.bin");
		release_firmware(fw);
		return;
	}

	printk(KERN_INFO "%s: loaded %s - size: %zu\n",
	       "aw87329_abrcv_cfg_loaded", "aw87329_abrcv.bin", fw->size);

	for (i = 0; i < fw->size; i++)
		printk(KERN_INFO "%s: cont: addr:0x%02x, data:0x%02x\n",
		       "aw87329_abrcv_cfg_loaded", i, fw->data[i]);

	len = fw->size;
	aw87329_abrcv_reg = kzalloc(len + 4, GFP_KERNEL);
	if (!aw87329_abrcv_reg) {
		release_firmware(fw);
		printk(KERN_ERR "%s: error allocating memory\n", "aw87329_abrcv_cfg_loaded");
		return;
	}

	*(u32 *)aw87329_abrcv_reg = fw->size;
	memcpy(aw87329_abrcv_reg + 4, fw->data, fw->size);
	release_firmware(fw);

	for (i = 0; i < len; i++)
		printk(KERN_INFO "%s: rcv_cnt: addr:0x%02x, data:0x%02x\n",
		       "aw87329_abrcv_cfg_loaded", i, aw87329_abrcv_reg_val(i));

	g_aw87329->abrcv_reg_loaded = 1;
}

/*
 * aw87329_get_mode: 0xffffff8008c59d60, 104 byte, statica (t).
 *
 * Nessuna snprintf: il testo e' un letterale costante, e clang lo scrive
 * come una sequenza di store non allineate (ldur/stur) invece di una
 * chiamata -- ottimizzazione standard per uno sprintf senza conversioni.
 * Il testo, ricostruito dai quattro pezzi, e' lungo esattamente 52 byte --
 * "mov w0,#0x34" (52) e' il valore di ritorno, e coincide byte per byte
 * con la somma delle quattro righe (12+13+13+14):
 *
 *   "0: off mode\n"@0x92a32ef
 *     -- ffffff8008c59d60 adrp x8  / ffffff8008c59d64 add x8,x8,#0x2ef
 *   "1: kspk mode\n"@0x92a32fc
 *     -- ffffff8008c59d6c adrp x10 / ffffff8008c59d70 add x10,x10,#0x2fc
 *   "2: drcv mode\n"@0x92a330a
 *     -- ffffff8008c59d80 adrp x11 / ffffff8008c59d88 add x11,x11,#0x30a
 *   "3: abrcv mode\n"@0x92a3318
 *     -- ffffff8008c59d98 adrp x8  / ffffff8008c59d9c add x8,x8,#0x318
 *
 * I quattro indirizzi sono consecutivi e distanti esattamente quanto la
 * lunghezza di ciascun pezzo piu' il terminatore (0x2ef+13=0x2fc,
 * 0x2fc+14=0x30a, 0x30a+14=0x318, 0x318+15=0x327 = "cfg_timer_func"): e'
 * questa aritmetica, non l'occhio, a dire che i pezzi sono quattro e
 * terminati singolarmente.
 *
 * **Corretto dopo la prima misura** (104 fabbrica / 48 nostra): un
 * letterale unico "riga1" "riga2" "riga3" "riga4" (concatenazione di
 * stringhe adiacenti dello standard C, che il preprocessore fonde in UN
 * blob di 52 byte senza terminatori intermedi) compila in soli 12
 * ldp/stp -- piu' denso della fabbrica. Ma i quattro pezzi in .rodata
 * SONO terminati da null CIASCUNO per conto proprio (verificato: la
 * lettura per-indirizzo si ferma a ogni \0, a 0x92a32ef, 0x92a32fc,
 * 0x92a330a, 0x92a3318 separatamente) -- il fabbrica ha quattro
 * letterali SEPARATI, non uno concatenato: il sorgente originale, con
 * ogni probabilita', chiama sprintf() quattro volte, una per riga, non
 * una volta con un letterale unico. Riscritta cosi': la nostra build
 * scende a 25 istruzioni (verificato con
 * aarch64-linux-android-objdump su out/vmlinux, vedi il registro), un
 * risultato di forma molto piu' vicino, anche se la dimensione totale
 * resta diversa (i quattro letterali finiscono su indirizzi .rodata che
 * il NOSTRO compilatore alloca diversamente, non essendo byte-per-byte
 * identico all'indirizzamento del fabbrica).
 */
static ssize_t aw87329_get_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t len = 0;

	len += sprintf(buf + len, "0: off mode\n");
	len += sprintf(buf + len, "1: kspk mode\n");
	len += sprintf(buf + len, "2: drcv mode\n");
	len += sprintf(buf + len, "3: abrcv mode\n");

	return len;
}

/*
 * aw87329_set_mode: 0xffffff8008c59dc8, 160 byte, statica (t).
 *
 * ffffff8008c59df8..59e04: sscanf(buf, "%d", &val) (fmt "%d"@0x9216030,
 *   condiviso con set_hwen/set_update); cmp w8,#0x3; b.hi -> 0x59e20
 *   (default, val>3 o negativo: il confronto e' senza segno, quindi un
 *   int negativo letto da sscanf ci finisce dentro).
 * ffffff8008c59e08..59e1c: tabella di salto a un byte per caso, letta con
 *   objdump -s (0xffffff8008faf0a0: 00 02 04 06): br a
 *   (adr 0x59e20) + tabella[val]*4 -- indici 0,2,4,6 (non 0,1,2,3: ogni
 *   "case" e' due istruzioni, bl+b, quindi passo 2 non 1).
 * ffffff8008c59e20: case 0 (e default, stesso indirizzo): bl
 *   aw87329_audio_off.
 * ffffff8008c59e28: case 1: bl aw87329_audio_kspk.
 * ffffff8008c59e30: case 2: bl aw87329_audio_drcv.
 * ffffff8008c59e38: case 3: bl aw87329_audio_abrcv.
 * ffffff8008c59e3c..59e64: verifica canary, return count.
 */
static ssize_t aw87329_set_mode(struct device *dev, struct device_attribute *attr,
				 const char *buf, size_t count)
{
	unsigned int val = 0;

	sscanf(buf, "%d", &val);
	switch (val) {
	case 1:
		aw87329_audio_kspk();
		break;
	case 2:
		aw87329_audio_drcv();
		break;
	case 3:
		aw87329_audio_abrcv();
		break;
	case 0:
	default:
		aw87329_audio_off();
		break;
	}

	return count;
}

/*
 * cfg_timer_func: 0xffffff8008c59e68, 68 byte, statica (t). NON e' uno
 * dei 27 nomi della sequenza-bersaglio: trovata in questo lotto (vedi
 * l'introduzione sopra), e' il callback dell'hrtimer avviato da
 * aw87329_i2c_probe (priv->timer.function).
 *
 * ffffff8008c59e70..59e80: printk(KERN_INFO "%s enter\n"@0x92a2f79,
 *   "cfg_timer_func"@0x92a3327).
 * ffffff8008c59e84..5989c: queue_work_on(8, wq, &g_aw87329->work) -- w0=8
 *   (cpu, immediato, orr w0,wzr,#0x8); x1 = valore letto da un indirizzo
 *   fisso (adrp 0x985e000 + #0xae8 = 0xffffff800985eae8). x2 =
 *   &g_aw87329->work (add x2,x8,#0x58, offset 0x58 della struct, vedi
 *   sopra).
 *
 * **`system_wq`, verificato in revisione, non piu' un'inferenza**:
 * `stock.elf`/`stock.map` non hanno simboli dati (zero righe D/d/B/b),
 * quindi il nome non si legge direttamente -- ma la STESSA pagina, allo
 * stesso offset 0xa88 (2696), e' gia' nota per essere `__stack_chk_guard`
 * (il pattern canary dello stack protector, usato identico da
 * aw87329_get_reg/set_reg/set_hwen/set_update/set_mode di questo stesso
 * lotto). Confrontando il DELTA fra i due indirizzi con lo stesso delta
 * nella nostra build (che ha un `out/vmlinux` non spogliato, quindi con
 * nomi veri):
 *
 *   $ nm out/vmlinux | grep -w '__stack_chk_guard\|system_wq'
 *   ffffff800979ea88 D __stack_chk_guard
 *   ffffff800979eae8 D system_wq
 *
 * Delta nella nostra build: 0xae8-0xa88 = 0x60 (96) byte. Delta nel
 * fabbrica (0x985eae8-0x985ea88): **esattamente lo stesso 0x60**. Le due
 * build hanno codice driver quasi completamente diverso prima di questo
 * punto nel link, eppure la distanza fra questi due globali del kernel
 * "core" (link-order determinato da codice non-driver, non toccato da
 * questo lotto) e' identica byte per byte: prova sufficientemente forte
 * che l'indirizzo del fabbrica e' davvero `system_wq`, non una
 * coincidenza.
 * ffffff8008c59ea0: return HRTIMER_NORESTART (mov w0,wzr = 0, il valore
 *   dell'enum in linux/hrtimer.h).
 */
static enum hrtimer_restart cfg_timer_func(struct hrtimer *timer)
{
	printk(KERN_INFO "%s enter\n", "cfg_timer_func");

	queue_work_on(8, system_wq, &g_aw87329->work);

	return HRTIMER_NORESTART;
}

/*
 * cfg_work_routine: 0xffffff8008c59eac, 124 byte, statica (t). Come
 * cfg_timer_func, non e' uno dei 27: e' il callback del work_struct
 * avviato da cfg_timer_func (priv->work.func, INIT_WORK in probe).
 *
 * ffffff8008c59eb4..59ec4: printk(KERN_INFO "%s enter\n"@0x92a2f79,
 *   "cfg_work_routine"@0x91c7ad0).
 * ffffff8008c59ec8..59efc: request_firmware_nowait(NULL, true,
 *   "aw87329_kspk.bin"@0x92a31db, &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
 *   aw87329_kspk_cfg_loaded) -- **stessa identica chiamata** di
 *   aw87329_set_update (stesso file, stesso target, stesso gfp 0xc0|
 *   (0x140<<16)): il timer a 5 secondi di probe riavvia la stessa catena
 *   di caricamento firmware innescata manualmente da sysfs/update.
 * ffffff8008c59f00..59f1c: cbz w0 -> fine (successo); altrimenti
 *   printk(KERN_ERR "%s: request_firmware_nowait failed with read
 *   %s"@0x92a3196, "cfg_work_routine"@0x91c7ad0, "aw87329_kspk.bin").
 */
static void cfg_work_routine(struct work_struct *work)
{
	int ret;

	printk(KERN_INFO "%s enter\n", "cfg_work_routine");

	ret = request_firmware_nowait(NULL, true, "aw87329_kspk.bin",
				       &g_aw87329->i2c->dev, GFP_KERNEL, NULL,
				       aw87329_kspk_cfg_loaded);
	if (ret)
		printk(KERN_ERR "%s: request_firmware_nowait failed with read %s",
		       "cfg_work_routine", "aw87329_kspk.bin");
}

/*
 * Tabelle dati dell'i2c_driver. **La prima stesura le dichiarava "nessuna
 * verificabile dal disassemblato (sono dati, non codice)" e le costruiva da
 * `strings -t x stock.elf | grep -i aw87329`: era falso.** Sono
 * verificabili, seguendo `.rela.dyn` (178.389 voci R_AARCH64_RELATIVE,
 * `tools/relocazioni.py --conta`) a partire dall'unico indirizzo che il
 * codice costruisce, quello che aw87329_pa_init passa a
 * i2c_register_driver:
 *
 *   ffffff8009390fa8 adrp x1, ffffff80099c9000 / ffffff8009390fac add
 *   x1,x1,#0xf90   ->  &aw87329_i2c_driver = 0xffffff80099c9f90
 *
 * Da li', con gli offset dei veri include/linux/i2c.h e
 * include/linux/device.h di questo albero (probe 0x10, remove 0x18,
 * driver 0x40, sizeof(struct device_driver) 0x78, quindi driver.name
 * 0x40, driver.of_match_table 0x40+40=0x68, id_table 0x40+0x78=0xb8):
 *
 *   0xffffff80099c9fa0 -> 0xffffff8008c59178  aw87329_i2c_probe
 *   0xffffff80099c9fa8 -> 0xffffff8008c5943c  aw87329_i2c_remove
 *   0xffffff80099c9fd0 -> 0xffffff80092a30ac  "aw87329_pa"   (driver.name)
 *   0xffffff80099c9ff8 -> 0xffffff8008faf0d8  (of_match_table)
 *   0xffffff80099ca048 -> 0xffffff8008faf268  (id_table)
 *
 * .owner e .driver.bus non hanno rilocazione: sono NULL, coerente con
 * THIS_MODULE==NULL per un file built-in (gia' verificato in probe).
 *
 *   "aw87329_pa"@0x92a30ac -- **citazione DEBOLE**: i byte "aw87329_pa\0"
 *   compaiono 3 volte in stock.elf (0xffffff8008faf11f,
 *   0xffffff8008faf268, 0xffffff80092a30ac). La prima NON e' una stringa a
 *   se': e' la coda di "awinic,aw87329_pa" (0x8faf118 + 7). La seconda e'
 *   il `name[20]` INLINE dentro l'i2c_device_id. Solo la terza e' il
 *   letterale puntato, ed e' la rilocazione di driver.name a dirlo.
 *
 *   struct i2c_device_id { char name[I2C_NAME_SIZE=20]; kernel_ulong_t
 *   driver_data; } -- il nome e' un array inline, non un puntatore:
 *
 *     $ aarch64-linux-android-objdump -s --start-address=0xffffff8008faf268 \
 *           --stop-address=0xffffff8008faf2c8 oracolo/stock.elf
 *      ffffff8008faf268 61773837 3332395f 70610000 00000000  aw87329_pa......
 *      ffffff8008faf278 00000000 00000000 00000000 00000000  ................
 *      ffffff8008faf288 00000000 00000000 00000000 00000000  ................
 *
 *   driver_data = 0, e la voce successiva (0x8faf288) e' tutta zeri: due
 *   voci, la seconda terminatore -- esattamente come scritto qui sotto.
 *
 *   "awinic,aw87329_pa"@0x8faf118 -- struct of_device_id { char name[32];
 *   char type[32]; char compatible[128]; const void *data; }: compatible
 *   e' a offset 64, e 0x8faf0d8+64 = 0x8faf118. Anche qui la stringa e'
 *   inline, non puntata:
 *
 *     $ aarch64-linux-android-objdump -s --start-address=0xffffff8008faf0d8 \
 *           --stop-address=0xffffff8008faf268 oracolo/stock.elf
 *      ffffff8008faf0d8 00000000 00000000 00000000 00000000  ................
 *      [... name[32] e type[32] tutti zeri, fino a 0x8faf117 ...]
 *      ffffff8008faf118 6177696e 69632c61 77383733 32395f70  awinic,aw87329_p
 *      ffffff8008faf128 61000000 00000000 00000000 00000000  a...............
 *      [... resto di compatible[128] e data zeri, poi la seconda voce
 *       0x8faf1a0..0x8faf267 tutta zeri: terminatore ...]
 *
 * **Verificato anche contro il DTB di fabbrica**, non solo contro il
 * kernel: il nodo aw87329@59 di dtbo.img dichiara
 * compatible = "awinic,aw87329_pa" (vedi aw87329_i2c_probe sopra per il
 * dump del nodo). E' l'unico modo di provare che il compatible del driver
 * e quello dell'hardware sono la stessa stringa.
 */
static const struct i2c_device_id aw87329_i2c_id[] = {
	{ "aw87329_pa", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, aw87329_i2c_id);

static const struct of_device_id aw87329_of_match[] = {
	{ .compatible = "awinic,aw87329_pa" },
	{ },
};
MODULE_DEVICE_TABLE(of, aw87329_of_match);

static struct i2c_driver aw87329_i2c_driver = {
	.driver = {
		.name = "aw87329_pa",
		.owner = THIS_MODULE,
		.of_match_table = aw87329_of_match,
	},
	.probe = aw87329_i2c_probe,
	.remove = aw87329_i2c_remove,
	.id_table = aw87329_i2c_id,
};

/*
 * aw87329_pa_init: 0xffffff8009390f6c, 124 byte, statica (t), sezione
 * __init (lontanissima dal resto del blocco, come previsto dal brief).
 *
 * **Corretto dopo revisione**: la prima stesura scriveva una sola printk
 * (quella della versione) ed etichettava la prima `bl` a 0x9390f8c come
 * se fosse quella -- sbagliato, e causa diretta dello scarto 124/104 che
 * era stato lasciato fra gli scarti non indagati. x19 = 0x92a3336 =
 * "aw87329_pa_init" e' caricato UNA volta (ffffff8009390f78..59390f80) e
 * riusato come argomento in ENTRAMBE le printk sotto -- e' questo che
 * aveva fatto sembrare le due chiamate una sola.
 *
 * ffffff8009390f8c: printk(KERN_INFO "%s enter\n"@0x92a2f79,
 *   "aw87329_pa_init"@0x92a3336) -- **la printk mancante**: stessa
 *   coppia fmt/nome vista in cfg_timer_func/cfg_work_routine, non la
 *   versione.
 * ffffff8009390fa4: printk(KERN_INFO
 *   "%s: driver version: %s\n"@0x92a3346, "aw87329_pa_init"@0x92a3336,
 *   "v1.1.2"@0x92a3360) -- la seconda chiamata, quella della versione;
 *   ffffff8009390f90 adrp x0 / ffffff8009390f98 add x0,x0,#0x346 e
 *   ffffff8009390f94 adrp x2 / ffffff8009390f9c add x2,x2,#0x360.
 *
 *   **Due citazioni spezzate su due righe fisiche, corrette qui**: questa
 *   e quella di "aw87329_drcv.bin" sopra erano scritte con l'indirizzo a
 *   capo (`"testo"\n *   @0x...`), forma che RE_CITAZIONE di
 *   tools/verificacitazioni.py non riconosce -- la stessa classe di
 *   difetto gia' contata su aw22127 (R9).
 * ffffff8009390fb0..59390fb4: ret = i2c_register_driver(THIS_MODULE,
 *   &aw87329_i2c_driver) -- x0=xzr: THIS_MODULE compila a NULL per lo
 *   stesso motivo del devm_kmalloc in probe (file built-in, non modulo).
 * ffffff8009390fbc: cbz w0 -> fine (successo).
 * ffffff8009390fc0..59390fd4: se fallisce: printk(KERN_INFO
 *   "****[%s] Unable to register driver (%d)\n"@0x9143281,
 *   "aw87329_pa_init"@0x92a3336, ret).
 *   **Corretto da tools/verificacitazioni.py**: la prima stesura scriveva
 *   KERN_ERR; il byte vero all'indirizzo citato e' \x016 (KERN_INFO), non
 *   \x013 (KERN_ERR) -- letto con relocazioni.Immagine.byte, non a occhio.
 *   Il fabbrica registra questo fallimento a livello INFO, non ERR: un
 *   dettaglio di severita' del log, non di logica, ma la fedelta' che
 *   questo progetto persegue e' anche questa.
 * ffffff8009390fd8..59390fe4: return ret.
 *
 * **CORREZIONE del 2026-08-21 -- la aw87329_pa_exit ESISTE.**  La stesura
 * precedente di questo commento concludeva che non ci fosse alcuna funzione
 * di uscita, e la prova addotta era: «nessun simbolo t/T con questo nome e'
 * in stock.map».  Quella prova non poteva funzionare.  `oracolo/stock.map`
 * e' `nm -n` di uno stock.elf la cui symtab e' ricostruita dalla tabella
 * kallsyms dell'immagine, e kallsyms -- con `# CONFIG_KALLSYMS_ALL is not
 * set` nel config di fabbrica -- emette soltanto i simboli che cadono in
 * [_stext,_etext] o in [_sinittext,_einittext]
 * (alps-mtkwatch/scripts/kallsyms.c righe 48-51).  `.exit.text` comincia
 * ESATTAMENTE a `_einittext` (0xffffff80093a8518) e prosegue oltre: nessuna
 * funzione `__exit` puo' comparire in stock.map, di questo driver o di
 * qualunque altro.  L'assenza dalla mappa non era un dato sul driver: era
 * un dato sul limite dello strumento.
 *
 * Letta direttamente dai byte, la funzione c'e', a 0xffffff80093aee94, 48
 * byte (censimento della .exit.text, lotto 0 del 2026-08-21):
 *   "a9bf7bfd stp"@0xffffff80093aee94   stp x29, x30, [sp,#-16]!
 *   "913de400 add"@0xffffff80093aeea4   add x0,x0,#0xf79 -> "%s enter\n"
 *   "91020421 add"@0xffffff80093aeea8   add x1,x1,#0x81  -> "aw87329_pa_exit"
 *   "97b6118a bl"@0xffffff80093aeeac    -> ffffff80081334d4 <printk>
 *   "913e4000 add"@0xffffff80093aeeb4   add x0,x0,#0xf90 -> 0xffffff80099c9f90
 *                                       = &aw87329_i2c_driver, lo stesso
 *                                       indirizzo che aw87329_pa_init passa
 *                                       a i2c_register_driver
 *   "97db6e02 bl"@0xffffff80093aeeb8    -> ffffff8008a8a6c0 <i2c_del_driver>
 *   "d65f03c0 ret"@0xffffff80093aeec0
 * I due letterali sono "\x016%s enter\n"@0xffffff80092a2f79 (KERN_INFO, lo
 * stesso formato di aw87329_pa_init) e "aw87329_pa_exit"@0xffffff80092a3081.
 * L'ordine e' quello giusto: x0 e' il formato, x1 il nome della funzione.
 *
 * Resta vero -- ed e' ora la nota corretta, non piu' la conclusione -- che
 * su arm64 `.exit.text` non viene mai scartata
 * (arch/arm64/kernel/vmlinux.lds.S righe 18-19, `#define ARM_EXIT_KEEP(x) x`
 * con il commento «.exit.text needed in case of alternative patching»), e che
 * per un file built-in `__exit` implica `__used` (include/linux/init.h righe
 * 78-85): la funzione sopravvive al link ANCHE se nessuno la nomina.  Cio'
 * che il binario NON dice e' se accanto ci fosse un `module_exit()`: il suo
 * puntatore finisce in `.exitcall.exit`, che il vmlinux.lds scarta
 * (`EXIT_CALL` dentro /DISCARD/, riga 98).  Il `module_exit()` scritto qui
 * sotto e' quindi una SCELTA dichiarata, non una misura; la funzione e la
 * sua attribuzione a questo driver sono invece misurate.
 *
 * MISURA DI DIMENSIONE NON FATTA: questo lotto non ha un albero di build per
 * aw87329 e non ha compilato il file.  I 48 byte di fabbrica sono letti dal
 * disassemblato; il confronto con il nostro `.o` va ancora fatto.
 */
static int __init aw87329_pa_init(void)
{
	int ret;

	printk(KERN_INFO "%s enter\n", "aw87329_pa_init");
	printk(KERN_INFO "%s: driver version: %s\n", "aw87329_pa_init", "v1.1.2");

	ret = i2c_register_driver(THIS_MODULE, &aw87329_i2c_driver);
	if (ret)
		printk(KERN_INFO "****[%s] Unable to register driver (%d)\n",
		       "aw87329_pa_init", ret);

	return ret;
}

module_init(aw87329_pa_init);

static void __exit aw87329_pa_exit(void)
{
	printk(KERN_INFO "%s enter\n", "aw87329_pa_exit");
	i2c_del_driver(&aw87329_i2c_driver);
}

module_exit(aw87329_pa_exit);

/*
 * **Le due sole eccezioni nominate di questo file** (`--eccezione` di
 * tools/verificacitazioni.py): i letterali di MODULE_DESCRIPTION e
 * MODULE_LICENSE non hanno un indirizzo da citare perche' **non esistono
 * affatto nel kernel di fabbrica**, non perche' sia scomodo cercarli.
 *
 * Il config di fabbrica ha `CONFIG_SND_SOC_AW87329=y`
 * (kernel-work/e977_dg_m13_71_q0.config:3003): il driver e' built-in, non
 * un modulo. E per il codice built-in `__MODULE_INFO` non emette nulla --
 * alps-mtkwatch/include/linux/moduleparam.h:
 *
 *     #ifdef MODULE
 *     #define __MODULE_INFO(tag, name, info)                            \
 *     static const char __UNIQUE_ID(name)[]                             \
 *       __used __attribute__((section(".modinfo"), unused, aligned(1))) \
 *       = __stringify(tag) "=" info
 *     #else  // !MODULE
 *     // This struct is here for syntactic coherency, it is not used
 *     #define __MODULE_INFO(tag, name, info)                            \
 *       struct __UNIQUE_ID(name) {}
 *     #endif
 *
 * Il ramo `#else` dichiara una struct vuota: zero byte nell'immagine.
 * Riscontro sul binario, non solo sul sorgente della macro:
 *
 *   - i byte "AW87329 PA Driver\0" compaiono **0 volte** in stock.elf;
 *   - i byte "GPL v2\0" compaiono 1 volta, a 0xffffff80090eeb6e, ma
 *     quell'occorrenza NON e' nostra: e' una voce della tabella delle
 *     licenze di kernel/module.c, come si vede dalle stringhe adiacenti --
 *     a 0x90eeb44 il messaggio di taint del modulo, a 0x90eeb75 la voce
 *     successiva della stessa tabella (GPL and additional rights).
 *     (Scritte qui in prosa e non nella forma "testo"-chiocciola-indirizzo
 *     apposta: sarebbero citazioni non ancorate, testi che questo file non
 *     contiene come letterali -- e tools/verificacitazioni.py le rifiuta,
 *     come ha fatto durante la stesura di questo stesso commento.)
 *
 * Citare quell'indirizzo sarebbe una citazione FALSA -- verificherebbe
 * byte per byte e non proverebbe niente. Per questo sono eccezioni
 * dichiarate per nome, non citazioni comode.
 */
MODULE_DESCRIPTION("AW87329 PA Driver");
MODULE_LICENSE("GPL v2");
