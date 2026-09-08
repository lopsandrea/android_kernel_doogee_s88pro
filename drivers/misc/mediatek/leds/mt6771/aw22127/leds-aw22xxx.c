// SPDX-License-Identifier: GPL-2.0
/*
 * LED della scocca Awinic AW22xxx del Doogee S88 Pro (CONFIG_WTK_LEDS_AW22127).
 *
 * Ricostruito leggendo il kernel di fabbrica disassemblato, non adattato da
 * un altro telefono e non copiato da un driver aw22xxx di terzi: il bersaglio
 * e' questo binario. Ogni registro e ogni costante di questo file ha la sua
 * riga di disassemblato citata in docs/bringup/driver-riscritti.md, e ogni
 * stringa e' citata nella forma "testo"@0xINDIRIZZO, che tools/verificacitazioni.py
 * rilegge dal binario di fabbrica: se un valore non e' verificabile cosi',
 * e' un difetto.
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
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/workqueue.h>

/*
 * Struttura dati del driver, dedotta dagli offset che le funzioni del lotto
 * 1 leggono e scrivono su "chip" (il puntatore ottenuto risalendo da
 * dev_get_drvdata()). Costruita per campi via via che i lotti la
 * identificano -- lo stesso metodo di aw87329.c.
 *
 * offset 0x00: client, "struct i2c_client *". aw22xxx_i2c_write legge
 *   *chip (ldr x0,[x0], ffffff800879f568) prima di passarlo a
 *   i2c_smbus_write_byte_data; stessa cosa in aw22xxx_i2c_read
 *   (ffffff80087a0174).
 * offset 0x08: dev, "struct device *". Identificato, non piu' riservato: le
 *   due dev_err del lotto lo caricano con **una sola istruzione**, il che
 *   esclude &chip->client->dev (che ne richiederebbe due, una ldr per
 *   client e una add per l'offset di "dev" dentro struct i2c_client):
 *     ffffff800879ffdc:	f9400680 	ldr	x0, [x20,#8]    (aw22xxx_hw_reset, x20 = chip)
 *     ffffff80087a0834:	f85f82a0 	ldur	x0, [x21,#-8]   (aw22xxx_hwen_store, x21 = cdev = chip+0x10)
 *   Entrambe caricano il puntatore *memorizzato* a chip+0x08 e lo passano a
 *   dev_err come primo argomento, cioe' come "struct device *".
 * offset 0x10: cdev, "struct led_classdev" incorporato. dev_get_drvdata(dev)
 *   restituisce &chip->cdev (il led core registra cosi'); il driver risale
 *   a chip sottraendo 0x10 (ffffff80087a0618 in aw22xxx_reg_show,
 *   ffffff80087a0714 in aw22xxx_reg_store: "sub xN, x8, #0x10" da x8 =
 *   dev_get_drvdata(dev)). sizeof(struct led_classdev) su questo albero e
 *   questo .config e' stato misurato, non assunto: un simbolo
 *   __probe_led_classdev_size da 0x150 byte (336), compilato e ispezionato
 *   con objdump -t su out-aw22127/vmlinux, poi rimosso -- non e' nel driver
 *   finale, e' un fatto di questo registro (docs/bringup/driver-riscritti.md).
 * offset 0x160: brightness_work, "struct work_struct" -- identificato dal
 *   lotto 3, non piu' riservato. aw22xxx_set_brightness accoda cdev+0x150
 *   ("add x2,x0,#0x150"@ffffff80087a05c4 con x0 = &chip->cdev), cioe'
 *   chip+0x160; e aw22xxx_brightness_work risale a chip sottraendo 0x160 dal
 *   proprio argomento ("sub x19,x0,#0x160"@ffffff80087a0220). Il gestore e'
 *   scritto qui da aw22xxx_i2c_probe: "str x10,[x26,#376]" con
 *   x10 = aw22xxx_brightness_work (ffffff800879fd18).
 * offset 0x180: task_work, "struct work_struct". aw22xxx_task0_store passa
 *   x20+0x170 (x20 = dev_get_drvdata(dev), cioe' &chip->cdev) come terzo
 *   argomento di queue_work_on (ffffff80087a0fbc: "add x2,x20,#0x170"):
 *   0x170 + 0x10 (l'offset di cdev in chip) = 0x180. La CPU passata a
 *   queue_work_on e' il letterale 8 (ffffff80087a0fc0: "orr w0,wzr,#0x8"),
 *   che **e' WORK_CPU_UNBOUND**, non la CPU numero 8 -- che su un MT6771
 *   (CPU 0..7) non esiste. In include/linux/workqueue.h di questo albero
 *   WORK_CPU_UNBOUND = NR_CPUS, e:
 *     $ grep -n "NR_CPUS" /mnt/s88pro/kernel-work/e977_dg_m13_71_q0.config
 *     456:CONFIG_NR_CPUS=8
 *   Il sorgente di fabbrica e' dunque schedule_work(&chip->...), che si
 *   espande in queue_work(system_wq, ...) e quindi in
 *   queue_work_on(WORK_CPU_UNBOUND, system_wq, ...). Scriverlo come
 *   letterale 8 dava oggi lo stesso codice macchina ma il significato
 *   sbagliato, e sarebbe divergito in silenzio al primo .config con un
 *   NR_CPUS diverso.
 * offset 0x254: task0, u8. aw22xxx_task0_show legge [x8,#580] da x8 =
 *   dev_get_drvdata(dev) (cioe' &chip->cdev, NON chip -- l'accesso non passa
 *   dal sub #0x10): 580 + 0x10 (l'offset di cdev in chip) = 0x254, l'offset
 *   che il compilatore produce per chip->task0 quando chip = container_of
 *   (cdev, struct aw22xxx_chip, cdev) viene piegato in un'unica costante.
 *   Citazione: ffffff80087a0f68 (aw22xxx_task0_show), ffffff80087a0fbc/
 *   ffffff80087a0fc4 (aw22xxx_task0_store).
 *
 * Lotto 2, altri campi -- tutti dallo stesso schema (offset citato dal
 * disassemblato e' relativo a "cdev" -- x8/x20/x21 = dev_get_drvdata(dev),
 * NON chip; sommare 0x10 per l'offset in chip):
 * offset 0x1a0: fw_work, "struct work_struct". aw22xxx_fw_store:
 *   "add x2,x20,#0x190"@ffffff80087a090c -> cdev+0x190=chip+0x1a0.
 * offset 0x1c0: cfg_work, "struct work_struct". aw22xxx_cfg_store:
 *   "add x2,x20,#0x1b0"@ffffff80087a0a44 -> cdev+0x1b0=chip+0x1c0.
 *   (0x1a0-0x180=0x1c0-0x1a0=0x20: conferma indipendente che
 *   sizeof(struct work_struct) su questo albero e' 0x20, coerente con la
 *   sizeof calcolata sotto per il resto della struct.)
 * offset 0x1e0: fw_timer, "struct hrtimer" (lotto 3). aw22xxx_i2c_probe
 *   chiama hrtimer_init su chip+0x1e0 ("add x19,x20,#0x1e0"@ffffff800879fe2c)
 *   e vi scrive il gestore a chip+0x208 ("str x8,[x20,#520]"
 *   @ffffff800879fe74, x8 = aw22xxx_fw_timer_func): 0x208-0x1e0 = 0x28, che
 *   e' l'offset di "function" in struct hrtimer su questo albero
 *   (timerqueue_node 0x20 + _softexpires 0x8). aw22xxx_fw_timer_func risale
 *   al work sottraendo 0x40 dal proprio argomento
 *   ("sub x2,x19,#0x40"@ffffff80087a1180): 0x1e0-0x40 = 0x1a0 = fw_work,
 *   quindi sizeof(struct hrtimer) = 0x40 -- letto, non assunto.
 * offset 0x220: cfg_lock, "struct mutex" (lotto 3). __mutex_init riceve
 *   chip+0x220 ("add x0,x0,#0x220"@ffffff800879f7f4) e come secondo
 *   argomento, a 0xffffff80091c71b9, il testo "&aw22xxx->cfg_lock", che e'
 *   la stringificazione che la macro mutex_init() fa del suo argomento:
 *   e' cosi' che il campo prende nome, e che la variabile locale di
 *   aw22xxx_i2c_probe prende nome "aw22xxx". Lo stesso indirizzo lo blocca
 *   aw22xxx_hw_reset ("add x19,x20,#0x220"@ffffff800879ff90).
 * offset 0x240: reset_gpio, int. aw22xxx_hwen_show: "ldr w0,[x8,#560]"
 *   @ffffff80087a075c -> cdev+560=chip+0x240; stesso campo in
 *   aw22xxx_hwen_store ("ldr w0,[x21,#560]"@ffffff80087a0810) e in
 *   aw22xxx_hw_reset ("ldr w0,[x20,#576]"@ffffff800879ffa0 -- 576 e'
 *   relativo a *chip* stavolta, non a cdev: 576 = 0x240, stesso offset,
 *   coerente: aw22xxx_hw_reset riceve gia' "chip", non "cdev").
 *   Il nome viene dal lotto 3, non dall'attributo sysfs che lo espone: la
 *   proprieta' del device tree e' "reset-gpio"@0xffffff80091c72a7, l'etichetta
 *   passata a devm_gpio_request_one e' "aw22xxx_rst"@0xffffff80091c71f2 e i
 *   messaggi lo chiamano "reset gpio". L'attributo sysfs si chiama "hwen"
 *   (relocazioni.py, rapporto-strutture-aw22127.txt) e le due funzioni che
 *   lo servono conservano quel nome: e' il nome dell'attributo, non del campo.
 * offset 0x244: irq_gpio, int (lotto 3). aw22xxx_i2c_probe:
 *   "str w0,[x22,#580]"@ffffff800879f85c dopo of_get_named_gpio_flags con
 *   "irq-gpio"@0xffffff80091c7315; aw22xxx_i2c_remove lo rilegge
 *   ("ldr w0,[x19,#580]"@ffffff800879ff08) per gpiod_to_irq. Che i due gpio
 *   siano adiacenti e int lo prova la sola istruzione con cui la probe li
 *   mette entrambi a -1 quando non c'e' device tree:
 *     ffffff800879f8bc:	92800008 	mov	x8, #0xffffffffffffffff
 *     ffffff800879f8c0:	f90122c8 	str	x8, [x22,#576]
 *   -- una str a 64 bit su chip+0x240 scrive 0x240 e 0x244 insieme.
 * offset 0x248: flags, u8 (lotto 3). aw22xxx_i2c_probe lo legge per decidere
 *   se registrare l'interrupt ("ldrb w8,[x22,#584]"@ffffff800879fb18,
 *   "tbnz w8,#0") e ne alza il bit 0 quando rinuncia
 *   ("orr w8,w8,#0x1; strb w8,[x26,#584]"@ffffff800879fc28/fc2c).
 * offset 0x249: chipid, u8 (lotto 3). Scritto dal riconoscimento del chip
 *   dentro aw22xxx_i2c_probe: 1 per aw22118 ("strb w8,[x22,#585]"
 *   @ffffff800879fa28 con w8=1), 2 per aw22127 (stesso store
 *   @ffffff800879fadc con w8=2). Nessuna funzione del blocco lo rilegge.
 * offset 0x24a: fw, u8. aw22xxx_fw_store: "strb w8,[x20,#570]"
 *   @ffffff80087a08fc -> cdev+570=chip+0x24a. Il lotto 3 mostra a cosa
 *   serve: aw22xxx_fw_loaded lo legge come "forza l'aggiornamento anche se
 *   il bist coincide" ("ldrb w8,[x19,#586]; cbz"@ffffff80087a165c/1660) e lo
 *   azzera dopo l'aggiornamento ("strb wzr,[x19,#586]"@ffffff80087a1c10).
 * offset 0x24b: fw_update, u8 (lotto 3). Stato dell'aggiornamento del
 *   firmware: 1 = in corso ("strb w8,[x19,#171]"@ffffff80087a11e0 con
 *   x19 = fw_work = chip+0x1a0, quindi chip+0x24b, w8=1, in
 *   aw22xxx_fw_work_routine), 2 = completo, 3 = fallito
 *   ("strb w8,[x19,#587]"@ffffff80087a1bf4). aw22xxx_cfg_work_routine
 *   rifiuta di caricare una configurazione se non vale 2
 *   ("ldrb w8,[x20,#587]; cmp w8,#0x2"@ffffff80087a1290/1294).
 * offset 0x24c: imax, u32. aw22xxx_imax_show: "ldr w3,[x22,#572]"
 *   @ffffff80087a0cb0; aw22xxx_imax_store: "str w8,[x21,#572]"
 *   @ffffff80087a0d40 -> cdev+572=chip+0x24c.
 * offset 0x255: task1, u8. aw22xxx_task1_show/_store, offset cdev+581
 *   (ffffff80087a1018, ffffff80087a1074) -> chip+0x255.
 * offset 0x256: effect, u8. aw22xxx_effect_show/_store, offset cdev+582
 *   (ffffff80087a0a94, ffffff80087a0af0) -> chip+0x256.
 * offset 0x257: cfg, u8. aw22xxx_cfg_show/_store, offset cdev+583
 *   (ffffff80087a09b8, ffffff80087a0a34) -> chip+0x257.
 * offset 0x258: rgb[9], u32[9]. aw22xxx_rgb_show legge cdev+584,588,...,616
 *   (nove interi, passo 4, ffffff80087a0dc0..0ea0); aw22xxx_rgb_store scrive
 *   "str w9,[x8,#584]"@ffffff80087a0f20 con x8 = chip(-0x10 gia' fatto) +
 *   indice*4 -- cdev+584=chip+0x258.
 */
struct aw22xxx_chip {
	struct i2c_client *client;			/* 0x000 */
	struct device *dev;				/* 0x008 */
	struct led_classdev cdev;			/* 0x010, sizeof 0x150 misurato */
	struct work_struct brightness_work;		/* 0x160 */
	struct work_struct task_work;			/* 0x180 */
	struct work_struct fw_work;			/* 0x1a0 */
	struct work_struct cfg_work;			/* 0x1c0 */
	struct hrtimer fw_timer;			/* 0x1e0, sizeof 0x40 letto */
	struct mutex cfg_lock;				/* 0x220 */
	int reset_gpio;					/* 0x240 */
	int irq_gpio;					/* 0x244 */
	u8 flags;					/* 0x248 */
	u8 chipid;					/* 0x249 */
	u8 fw;						/* 0x24a */
	u8 fw_update;					/* 0x24b */
	u32 imax;					/* 0x24c */
	u8 reserved_250[0x254 - 0x250];		/* 0x250..0x253, non identificato */
	u8 task0;					/* 0x254 */
	u8 task1;					/* 0x255 */
	u8 effect;					/* 0x256 */
	u8 cfg;						/* 0x257 */
	u32 rgb[9];					/* 0x258..0x27b */
};

/*
 * La dimensione totale non e' un'inferenza: aw22xxx_i2c_probe la passa a
 * devm_kzalloc come immediato.
 *   ffffff800879f7c4:	52901802 	mov	w2, #0x80c0
 *   ffffff800879f7c8:	52805001 	mov	w1, #0x280                 // #640
 *   ffffff800879f7cc:	72a02802 	movk	w2, #0x140, lsl #16
 *   ffffff800879f7d0:	aa1303e0 	mov	x0, x19
 *   ffffff800879f7d4:	97f54923 	bl	ffffff80084f1c60 <devm_kmalloc>
 * 0x280 = 640 byte, e l'ultimo campo identificato (rgb[8]) finisce a 0x27c:
 * i quattro byte finali sono l'allineamento a 8 della struct, non un campo.
 * w2 = 0x014080c0 = GFP_KERNEL (0x014000c0) | __GFP_ZERO (0x8000): la
 * chiamata di fabbrica e' devm_kzalloc, non devm_kmalloc -- e' lo stesso
 * difetto che su aw87329 era stato scritto al contrario, qui letto dal bit.
 *
 * Il "sizeof(struct aw22xxx_chip)" che questo file scrive vale 640 solo se
 * sizeof(struct hrtimer) = 0x40 e sizeof(struct mutex) = 0x20 su questo
 * albero e questo .config. Non e' assunto: se una delle due fosse diversa,
 * l'immediato emesso dal nostro build non sarebbe 0x280 e gli offset dei
 * campi dopo 0x1e0 non coinciderebbero -- entrambe le cose sono verificate
 * sul .o (vedi docs/bringup/driver-riscritti.md, lotto 3).
 */

/*
 * Contenitore in cui aw22xxx_fw_loaded copia il firmware dopo averlo
 * validato. La forma viene dal codice che lo riempie, non da un driver di
 * terzi: quattro interi e poi i dati, e la kzalloc chiede
 * "cont->size + 0x10" ("add x0,x8,#0x10"@ffffff80087a142c), cioe' i sedici
 * byte di intestazione piu' il file intero.
 *   len     +0x00: "str w2,[x20]"@ffffff80087a151c
 *   version +0x04: "str w2,[x20,#4]"@ffffff80087a14b8
 *   bist    +0x08: "str w2,[x20,#8]"@ffffff80087a14dc
 *   key     +0x0c: "str w2,[x20,#12]"@ffffff80087a14f8
 *   data    +0x10: "add x0,x20,#0x10"@ffffff80087a152c, primo argomento
 *                  della memcpy che copia il corpo del firmware.
 */
struct aw22xxx_container {
	unsigned int len;
	unsigned int version;
	unsigned int bist;
	unsigned int key;
	unsigned char data[];
};

/*
 * Tabella di accesso ai 256 registri, letta da .rodata byte per byte (non
 * calcolata): usata da aw22xxx_reg_show per decidere quali registri stampare
 * (bit 0 = leggibile; osservato solo 0x00, 0x01, 0x03 nel binario -- il
 * significato del bit 1 non e' esercitato da questo lotto e non viene
 * inventato).
 *
 * $ objdump -s --start-address=0xffffff8008f55b98 --stop-address=0xffffff8008f55c98 oracolo/stock.elf
 * ffffff8008f55b98 01030303 03030301 03030303 03030103  ................
 * ffffff8008f55ba8 03000000 00000000 00000000 00000000  ................
 * ffffff8008f55bb8 03030303 03010100 00000000 00000000  ................
 * ffffff8008f55bc8 03030303 03030303 03030000 00000000  ................
 * ffffff8008f55bd8..bf7 00000000 (191 byte a 0, indici 96..254)
 * ffffff8008f55c97 03 (indice 255: NON zero -- vedi sotto)
 *
 * DIFETTO CORRETTO (revisione lotto 3, D1): il commento diceva "azzerata
 * fino a 0x8f55c98" per l'intero blocco 0xbd8..0xc97, una verifica
 * dichiarata e mai fatta byte per byte. L'ultimo byte del blocco,
 * all'indice 255, e' 0x03 e non 0x00: la voce successiva di .rodata e'
 * l'id_table ("aw22xxx_led", righe piu' sotto), che fissa la fine
 * dell'array senza margine di dubbio --
 * $ objdump -s --start-address=0xffffff8008f55c90 --stop-address=0xffffff8008f55ca0 oracolo/stock.elf
 * ffffff8008f55c90 00000000 00000003 61773232 7878785f  ........aw22xxx_
 * -- 0xffffff8008f55c97 = 0x03, 0xffffff8008f55c98 = "aw22xxx_led". Con
 * l'indice 255 a 0x00 (come prima di questa correzione) il registro 0xff --
 * quello di selezione pagina, scritto di continuo dal driver -- risultava
 * marcato non leggibile, e aw22xxx_reg_show non lo stampava mai; di
 * fabbrica lo stampa (bit 0 di 0x03 alzato). La correzione non cambia la
 * dimensione di nessuna funzione (verificato: 37 simboli, aw22xxx_reg_show
 * e aw22xxx_fw_loaded invariate), perche' e' un dato in .rodata, non
 * codice -- la classe di difetto che il mandato della revisione chiedeva
 * di cercare.
 */
static const u8 aw22xxx_reg_access[256] = {
	0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x03,
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 0xffffff8008f55bd8 .. 0xffffff8008f55c96: 191 byte a 0 (indici 96..254) */
	[255] = 0x03,	/* 0xffffff8008f55c97, vedi sopra */
};

/*
 * Nomi delle 14 configurazioni "cfg", 0x40 byte l'una, lette da .rodata
 * (lotto 2, usate da aw22xxx_cfg_show):
 * $ objdump -s --start-address=0xffffff8009932650 --stop-address=0xffffff80099329d0 oracolo/stock.elf
 * (solo il testo non nullo di ciascuna voce da 0x40 byte, indirizzo iniziale
 * a fianco -- il resto di ogni voce e' zero, il compilatore lo aggiunge da
 * solo per un array di char[N] inizializzato con una stringa piu' corta di N)
 */
static const char aw22xxx_cfg_names[14][0x40] = {
	"aw22xxx_cfg_led_off.bin",		/* 0xffffff8009932650 */
	"aw22xxx_cfg_led_on.bin",		/* 0xffffff8009932690 */
	"aw22xxx_cfg_led_breath.bin",		/* 0xffffff80099326d0 */
	"aw22xxx_cfg_led_collision.bin",	/* 0xffffff8009932710 */
	"aw22xxx_cfg_led_skyline.bin",		/* 0xffffff8009932750 */
	"aw22xxx_cfg_led_flower.bin",		/* 0xffffff8009932790 */
	"aw22xxx_cfg_audio_skyline.bin",	/* 0xffffff80099327d0 */
	"aw22xxx_cfg_audio_flower.bin",	/* 0xffffff8009932810 */
	"music_sync.bin",			/* 0xffffff8009932850 */
	"call_reminder.bin",			/* 0xffffff8009932890 */
	"charging.bin",			/* 0xffffff80099328d0 */
	"full_charged.bin",			/* 0xffffff8009932910 */
	"power_onoff.bin",			/* 0xffffff8009932950 */
	"short_message_notice.bin",		/* 0xffffff8009932990 */
};

/*
 * Nomi delle 13 correnti massime "imax", 0x20 byte l'una (lotto 2, usate da
 * aw22xxx_imax_show):
 * $ objdump -s --start-address=0xffffff80099329d0 --stop-address=0xffffff8009932b70 oracolo/stock.elf
 */
static const char aw22xxx_imax_names[13][0x20] = {
	"AW22XXX_IMAX_2mA",	/* 0xffffff80099329d0 */
	"AW22XXX_IMAX_3mA",	/* 0xffffff80099329f0 */
	"AW22XXX_IMAX_4mA",	/* 0xffffff8009932a10 */
	"AW22XXX_IMAX_6mA",	/* 0xffffff8009932a30 */
	"AW22XXX_IMAX_9mA",	/* 0xffffff8009932a50 */
	"AW22XXX_IMAX_10mA",	/* 0xffffff8009932a70 */
	"AW22XXX_IMAX_15mA",	/* 0xffffff8009932a90 */
	"AW22XXX_IMAX_20mA",	/* 0xffffff8009932ab0 */
	"AW22XXX_IMAX_30mA",	/* 0xffffff8009932ad0 */
	"AW22XXX_IMAX_40mA",	/* 0xffffff8009932af0 */
	"AW22XXX_IMAX_45mA",	/* 0xffffff8009932b10 */
	"AW22XXX_IMAX_60mA",	/* 0xffffff8009932b30 */
	"AW22XXX_IMAX_75mA",	/* 0xffffff8009932b50 */
};

/*
 * aw22xxx_imax_store (sotto) legge un byte da questa tabella indicizzato
 * dal valore grezzo scritto in sysfs (senza limiti: "ldrb w9,[x9,x8]"
 * @ffffff80087a0d3c, x8 = valore letto da sscanf, non controllato), poi lo
 * satura a un massimo di 0xf (csel su "cc", cioe' min(table[val], 0xf)).
 * 32 byte letti da .rodata:
 * $ objdump -s --start-address=0xffffff8008f55cd8 --stop-address=0xffffff8008f55cf8 oracolo/stock.elf
 * ffffff8008f55cd8 08000901 020b030c 040e0506 07000000  ................
 * ffffff8008f55ce8 00030606 06090c0f 00000000 00000000  ................
 */
static const u8 aw22xxx_imax_clamp_table[32] = {
	0x08, 0x00, 0x09, 0x01, 0x02, 0x0b, 0x03, 0x0c,
	0x04, 0x0e, 0x05, 0x06, 0x07, 0x00, 0x00, 0x00,
	0x00, 0x03, 0x06, 0x06, 0x06, 0x09, 0x0c, 0x0f,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/*
 * g_aw22xxx_chip -- puntatore globale al singolo chip del driver (letto da
 * aw22xxx_led_effect1/2, che non ricevono "chip" come argomento: entrambe
 * sono funzioni globali "T" da zero parametri, e leggono
 * "ldr x0,[x19,#4072]" da adrp x19,0xffffff8009d05000
 * (ffffff800879f3fc/f400) -- indirizzo 0xffffff8009d05fe8. Lo assegna
 * aw22xxx_i2c_probe ("str x0,[x28,#4072]"@ffffff800879f7dc, subito dopo la
 * devm_kzalloc e **prima** del controllo di nullita'), e lo riazzera la via
 * d'errore della probe ("str xzr,[x28,#4072]"@ffffff800879fa98). Finche'
 * resta NULL, le due funzioni scriverebbero a un chip nullo se chiamate --
 * comportamento di fabbrica, non nostro (lo stesso vale su g_aw87329 prima
 * della probe).
 *
 * R12 della revisione -- il buco di quattro byte fra g_aw22xxx_chip
 * (0xffffff8009d05fe8, 8 byte, fine 0xffffff8009d05ff0) e
 * g_aw22xxx_ledeffect (0xffffff8009d05ff4) -- e' **chiuso**: a
 * 0xffffff8009d05ff0 c'e' la "struct lock_class_key" anonima che la macro
 * mutex_init() genera, e che aw22xxx_i2c_probe passa come terzo argomento
 * a __mutex_init:
 *   ffffff800879f7e8:	d000ab22 	adrp	x2, ffffff8009d05000
 *   ffffff800879f7fc:	913fc042 	add	x2, x2, #0xff0
 *   ffffff800879f800:	97e6333f 	bl	ffffff800812c4fc <__mutex_init>
 * Con CONFIG_LOCKDEP spento "struct lock_class_key" e' vuota, quindi occupa
 * un byte e l'allineamento a 4 di g_aw22xxx_ledeffect consuma gli altri tre.
 * Non e' una variabile del driver che manchi al sorgente: la produce la
 * macro, e la produrra' anche qui, perche' questo file ora scrive
 * mutex_init(&aw22xxx->cfg_lock).
 */
static struct aw22xxx_chip *g_aw22xxx_chip;

/*
 * Dichiarazioni anticipate: aw22xxx_led_effect1 (posizione 1) chiama
 * aw22xxx_i2c_write, che di fabbrica sta alla posizione 2 (dopo), e
 * aw22xxx_i2c_probe (posizione 4) chiama quasi tutto il resto del file.
 * Non e' un'invenzione: e' cio' che l'ordine di emissione impone (Task 2,
 * Step 1 del piano). Il corpo vero resta alla sua posizione nella sequenza.
 */
static void aw22xxx_i2c_write(struct aw22xxx_chip *chip, u8 reg, u8 value);
static int aw22xxx_i2c_read(struct aw22xxx_chip *chip, u8 reg, u8 *value);
static void aw22xxx_hw_reset(struct aw22xxx_chip *chip);
static irqreturn_t aw22xxx_irq(int irq, void *data);
static void aw22xxx_brightness_work(struct work_struct *work);
static void aw22xxx_task_work(struct work_struct *work);
static void aw22xxx_set_brightness(struct led_classdev *cdev,
				    enum led_brightness brightness);
static enum hrtimer_restart aw22xxx_fw_timer_func(struct hrtimer *timer);
static void aw22xxx_fw_work_routine(struct work_struct *work);
static void aw22xxx_cfg_work_routine(struct work_struct *work);
static void aw22xxx_fw_loaded(const struct firmware *cont, void *context);
static void aw22xxx_cfg_loaded(const struct firmware *cont, void *context);
static const struct attribute_group aw22xxx_attribute_group;

/*
 * aw22xxx_led_effect1 -- posizione 1 della sequenza-bersaglio, lotto 2.
 * ffffff800879f3f0, 356 byte. Globale (T), zero parametri.
 *
 * Quattordici scritture di registro via aw22xxx_i2c_write, tredici seguite
 * da msleep(1) (l'ultima no: subito ret). Tabella registro/valore, dagli
 * immediati (tutti "orr wN,wzr,#imm" o "mov wN,#imm" -- citati come
 * vengono, nessuno dedotto); reg 0x05 compare quattro volte, reg 0x04 e
 * reg 0x0c due volte ciascuno, con valori diversi ogni volta:
 *   reg=0xff val=0x00   reg=0x02 val=0x01   reg=0x03 val=0x10
 *   reg=0x0c val=0x00   reg=0x05 val=0x01   reg=0x04 val=0x01
 *   reg=0x09 val=0x01   reg=0x06 val=0x6a   reg=0x04 val=0x03
 *   reg=0x05 val=0x41   reg=0x0b val=0x0f   reg=0x0c val=0x03
 *   reg=0x05 val=0x01   reg=0x05 val=0x20
 */
void aw22xxx_led_effect1(void)
{
	aw22xxx_i2c_write(g_aw22xxx_chip, 0xff, 0x00); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x02, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x03, 0x10); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0c, 0x00); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x04, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x09, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x06, 0x6a); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x04, 0x03); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x41); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0b, 0x0f); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0c, 0x03); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x20);
}

/*
 * aw22xxx_i2c_write -- posizione 2 della sequenza-bersaglio, lotto 1.
 * ffffff800879f554, 148 byte.
 *
 * Scrive un registro via i2c_smbus_write_byte_data; se fallisce (bit 31 di
 * w0, cioe' valore negativo), stampa un errore e riprova una volta sola
 * dopo msleep(1). Se anche il secondo tentativo fallisce, la funzione esce
 * comunque senza segnalarlo al chiamante (nessun "mov w0" prima del ret in
 * nessuno dei due percorsi d'uscita): e' un comportamento di fabbrica, non
 * un difetto nostro -- e' cosi' che aw87329_i2c_write_reg si comportava, e
 * qui e' identico. Riprodotto cosi', non "corretto".
 *
 * Stringhe:
 *   KERN_ERR "%s: i2c_write cnt=%d error=%d\n"@0xffffff80091c7110
 *   "aw22xxx_i2c_write"@0xffffff80091c7131
 */
static void aw22xxx_i2c_write(struct aw22xxx_chip *chip, u8 reg, u8 value)
{
	int ret;

	ret = i2c_smbus_write_byte_data(chip->client, reg, value);
	if (ret < 0) {
		printk(KERN_ERR "%s: i2c_write cnt=%d error=%d\n",
		       "aw22xxx_i2c_write", 0, ret);
		msleep(1);
		ret = i2c_smbus_write_byte_data(chip->client, reg, value);
		if (ret < 0) {
			printk(KERN_ERR "%s: i2c_write cnt=%d error=%d\n",
			       "aw22xxx_i2c_write", 1, ret);
			msleep(1);
		}
	}
}

/*
 * aw22xxx_led_effect2 -- posizione 3 della sequenza-bersaglio, lotto 2.
 * ffffff800879f5e8, 356 byte. Globale (T), zero parametri. Stessa forma
 * esatta di aw22xxx_led_effect1 (stessi registri, stessi valori, tredici
 * msleep(1)), **tranne l'ultima scrittura**: reg=0x05 val=0x2e (46) invece
 * di val=0x20 (32) -- letto due volte sul disassemblato per essere sicuri
 * non fosse un errore di trascrizione (ffffff800879f738: "mov w2,#0x2e").
 */
void aw22xxx_led_effect2(void)
{
	aw22xxx_i2c_write(g_aw22xxx_chip, 0xff, 0x00); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x02, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x03, 0x10); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0c, 0x00); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x04, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x09, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x06, 0x6a); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x04, 0x03); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x41); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0b, 0x0f); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x0c, 0x03); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x01); msleep(1);
	aw22xxx_i2c_write(g_aw22xxx_chip, 0x05, 0x2e);
}

/*
 * Le cinque funzioni che seguono non sono simboli di fabbrica: sono i nomi
 * che il kernel di fabbrica stampa da dentro aw22xxx_i2c_probe, e che quindi
 * erano funzioni a se' nel sorgente originale, incorporate da clang perche'
 * hanno un solo chiamante. Non sono state inventate: ciascuna prende il nome
 * dalla stringa che passa alle proprie printk/dev_err, e ciascuna di quelle
 * stringhe e' citata qui sotto. Scriverle come funzioni statiche riproduce
 * la stessa incorporazione (clang le fonde di nuovo dentro la probe) e tiene
 * separato cio' che di fabbrica era separato.
 *
 *   "aw22xxx_parse_dt"@0xffffff80091c72e8
 *   "aw22xxx_read_chipid"@0xffffff80091c73bd
 *   "aw22xxx_interrupt_clear"@0xffffff80091c74a1
 *   "aw22xxx_interrupt_setup"@0xffffff80091c7489
 *   "aw22xxx_parse_led_cdev"@0xffffff80091c751c
 */

/*
 * aw22xxx_parse_dt -- incorporata in aw22xxx_i2c_probe.
 * ffffff800879f80c..ffffff800879f878 (ramo "np != NULL").
 *
 * of_get_named_gpio(np, nome, 0) e' l'inline che chiama
 * of_get_named_gpio_flags(np, nome, 0, NULL): quattro argomenti, il quarto
 * a zero ("mov w2,wzr; mov x3,xzr"@ffffff800879f818/f81c).
 *
 * Stringhe:
 *   "reset-gpio"@0xffffff80091c72a7
 *   "%s: no reset gpio provided, will not HW reset device\n"@0xffffff80091c72b2
 *   "%s: reset gpio provided ok\n"@0xffffff80091c72f9
 *   "irq-gpio"@0xffffff80091c7315
 *   "%s: no irq gpio provided, will not suppport intterupt\n"@0xffffff80091c731e
 *     -- tre "p" in "suppport" e due "t" in "intterupt", letti byte per byte
 *        dal binario: non sono refusi di questo file.
 *   "%s: irq gpio provided ok\n"@0xffffff80091c7355
 *
 * I due errori sono dev_err e i due successi dev_info: lo dice la funzione
 * chiamata, non il livello di un prefisso ("bl dev_err"@ffffff800879f928
 * contro "bl _dev_info"@ffffff800879f840/f878).
 */
static int aw22xxx_parse_dt(struct device *dev, struct aw22xxx_chip *aw22xxx,
			     struct device_node *np)
{
	aw22xxx->reset_gpio = of_get_named_gpio(np, "reset-gpio", 0);
	if (aw22xxx->reset_gpio < 0) {
		dev_err(dev, "%s: no reset gpio provided, will not HW reset device\n",
			"aw22xxx_parse_dt");
		return -1;
	} else {
		dev_info(dev, "%s: reset gpio provided ok\n", "aw22xxx_parse_dt");
	}

	aw22xxx->irq_gpio = of_get_named_gpio(np, "irq-gpio", 0);
	if (aw22xxx->irq_gpio < 0) {
		dev_err(dev, "%s: no irq gpio provided, will not suppport intterupt\n",
			"aw22xxx_parse_dt");
		return -1;
	} else {
		dev_info(dev, "%s: irq gpio provided ok\n", "aw22xxx_parse_dt");
	}

	return 0;
}

/*
 * aw22xxx_read_chipid -- incorporata in aw22xxx_i2c_probe.
 * ffffff800879f95c..ffffff800879fb04, piu' la coda a ffffff800879fd74.
 *
 * Il ciclo di fabbrica e' srotolato **due volte** (due letture del registro
 * 0x01, la seconda a ffffff800879f9c4, e due copie del messaggio
 * "unsupported device revision" a ffffff800879f9a8 e ffffff800879fd74): il
 * sorgente ha un ciclo che gira al massimo due volte, non due letture
 * scritte a mano.
 *
 * Registri: scrive 0x00 in 0xff, poi 0x55 in 0x01 ("mov w2,#0x55"
 * @ffffff800879f974), aspetta 2 ms e rilegge 0x01 aspettandosi 0x76
 * ("cmp w2,#0x76"@ffffff800879f9a0). Poi legge il registro 0x00 e ne
 * riconosce due valori: 0x18 -> chipid = 1, 0x27 -> chipid = 2
 * ("cmp w2,#0x27"@ffffff800879fa0c, "cmp w2,#0x18"@ffffff800879fa14).
 * L'ordine in cui il binario confronta i due (prima 0x27) non e' l'ordine
 * del sorgente: le due stringhe stanno in .rodata nell'ordine aw22118,
 * aw22127, ed e' quello che questo file riproduce.
 *
 * Il codice d'uscita: -EIO se l'I2C fallisce ("orr w22,wzr,#0xfffffffb"
 * @ffffff800879fa4c, cioe' -5), -EINVAL se dopo due giri l'id non e' 0x76
 * ("mov w22,#0xffffffea"@ffffff800879fd90, cioe' -22).
 *
 * Stringhe:
 *   KERN_ERR "%s: failed to read register AW22XXX_REG_ID: %d\n"@0xffffff80091c738d
 *   KERN_INFO "%s aw22xxx detected\n"@0xffffff80091c73d1
 *   KERN_INFO "%s: chipid: aw22118\n"@0xffffff80091c73e8
 *   KERN_INFO "%s: chipid: aw22127\n"@0xffffff80091c73ff
 *   KERN_ERR "%s: unknown id=0x%02x\n"@0xffffff80091c7416
 *   KERN_INFO "%s unsupported device revision (0x%x)\n"@0xffffff80091c742f
 *     -- questa e' KERN_INFO ("\0016"), non KERN_ERR, benche' sia un errore:
 *        il byte di livello e' 0x36 a 0xffffff80091c7430.
 */
static int aw22xxx_read_chipid(struct aw22xxx_chip *aw22xxx)
{
	unsigned char cnt = 0;
	unsigned char reg_val = 0;	/* "strb wzr,[sp,#12]"@ffffff800879f964 */
	int ret;

	aw22xxx_i2c_write(aw22xxx, 0xff, 0x00);
	aw22xxx_i2c_write(aw22xxx, 0x01, 0x55);
	msleep(2);

	while (cnt < 2) {
		ret = aw22xxx_i2c_read(aw22xxx, 0x01, &reg_val);
		if (ret < 0) {
			dev_err(aw22xxx->dev,
				"%s: failed to read register AW22XXX_REG_ID: %d\n",
				"aw22xxx_read_chipid", ret);
			return -EIO;
		}
		if (reg_val == 0x76) {
			printk(KERN_INFO "%s aw22xxx detected\n",
			       "aw22xxx_read_chipid");
			aw22xxx_i2c_read(aw22xxx, 0x00, &reg_val);
			switch (reg_val) {
			case 0x18:
				aw22xxx->chipid = 1;
				printk(KERN_INFO "%s: chipid: aw22118\n",
				       "aw22xxx_read_chipid");
				break;
			case 0x27:
				aw22xxx->chipid = 2;
				printk(KERN_INFO "%s: chipid: aw22127\n",
				       "aw22xxx_read_chipid");
				break;
			default:
				printk(KERN_ERR "%s: unknown id=0x%02x\n",
				       "aw22xxx_read_chipid", reg_val);
				break;
			}
			return 0;
		}
		printk(KERN_INFO "%s unsupported device revision (0x%x)\n",
		       "aw22xxx_read_chipid", reg_val);
		cnt++;
		msleep(1);
	}

	return -EINVAL;
}

/*
 * aw22xxx_interrupt_clear -- incorporata in aw22xxx_i2c_probe.
 * ffffff800879fb40..ffffff800879fb6c. Legge e basta il registro 0x0a
 * ("mov w1,#0xa"@ffffff800879fb50) e ne stampa il valore: leggerlo e' cio'
 * che azzera lo stato di interrupt del chip, non c'e' nessuna scrittura.
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_interrupt_clear"@0xffffff80091c74a1
 *   KERN_INFO "%s: reg INTST=0x%x\n"@0xffffff80091c74b9
 */
static void aw22xxx_interrupt_clear(struct aw22xxx_chip *aw22xxx)
{
	unsigned char reg_val;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_interrupt_clear");
	aw22xxx_i2c_read(aw22xxx, 0x0a, &reg_val);
	printk(KERN_INFO "%s: reg INTST=0x%x\n", "aw22xxx_interrupt_clear",
	       reg_val);
}

/*
 * aw22xxx_interrupt_setup -- incorporata in aw22xxx_i2c_probe.
 * ffffff800879fb28..ffffff800879fb90. Stampa il proprio ingresso, chiama
 * aw22xxx_interrupt_clear e alza il bit 0 del registro 0x09.
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_interrupt_setup"@0xffffff80091c7489
 */
static void aw22xxx_interrupt_setup(struct aw22xxx_chip *aw22xxx)
{
	unsigned char reg_val;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_interrupt_setup");
	aw22xxx_interrupt_clear(aw22xxx);
	aw22xxx_i2c_read(aw22xxx, 0x09, &reg_val);
	aw22xxx_i2c_write(aw22xxx, 0x09, reg_val | 0x01);
}

/*
 * aw22xxx_parse_led_cdev -- incorporata in aw22xxx_i2c_probe.
 * ffffff800879fc30..ffffff800879fdcc, piu' i quattro dev_err condivisi a
 * ffffff800879fdd0..ffffff800879fe1c.
 *
 * Scorre i figli del nodo del device tree con of_get_next_child(np, temp)
 * -- il ciclo e' esattamente for_each_child_of_node: prima chiamata con
 * secondo argomento NULL ("mov x1,xzr"@ffffff800879fc4c), poi con il figlio
 * precedente ("mov x1,x19"@ffffff800879fcf0) -- e per ogni figlio legge
 * quattro proprieta'. Le tre of_property_read_u32 sono
 * of_property_read_variable_u32_array(np, nome, dest, 1, 0)
 * ("orr w3,wzr,#0x1; mov x4,xzr"), cioe' l'inline of_property_read_u32.
 *
 * I destinatari, dagli indirizzi che il binario calcola una volta sola
 * prima del ciclo (x26 = chip):
 *   x21 = chip+0x10  ("add x21,x26,#0x10"@ffffff800879fc64)  -> cdev.name
 *   x22 = chip+0x24c ("add x22,x26,#0x24c"@ffffff800879fc68) -> imax
 *   x23 = chip+0x18  ("add x23,x26,#0x18"@ffffff800879fc6c)  -> cdev.brightness
 *   x24 = chip+0x1c  ("add x24,x26,#0x1c"@ffffff800879fc70)  -> cdev.max_brightness
 *
 * Dopo il ciclo: due INIT_WORK, il puntatore a funzione della led_classdev,
 * led_classdev_register e sysfs_create_group. led_classdev_register e' un
 * inline che chiama of_led_classdev_register(parent, NULL, cdev)
 * ("mov x1,xzr"@ffffff800879fd44 come secondo argomento).
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_parse_led_cdev"@0xffffff80091c751c
 *   "aw22xxx,name"@0xffffff80091c7533
 *   "Failure reading led name, ret = %d\n"@0xffffff80091c7540
 *   "aw22xxx,imax"@0xffffff80091c7564
 *   "Failure reading imax, ret = %d\n"@0xffffff80091c7571
 *   "aw22xxx,brightness"@0xffffff80091c7591
 *   "Failure reading brightness, ret = %d\n"@0xffffff80091c75a4
 *   "aw22xxx,max_brightness"@0xffffff80091c75ca
 *   "Failure reading max brightness, ret = %d\n"@0xffffff80091c75e1
 *   "unable to register led ret=%d\n"@0xffffff80091c760b
 *   "led sysfs ret: %d\n"@0xffffff80091c762a
 * Nessuna di queste dev_err passa un nome di funzione: il primo argomento e'
 * chip->dev ("ldr x0,[x26,#8]") e il secondo e' direttamente il formato.
 */
static int aw22xxx_parse_led_cdev(struct aw22xxx_chip *aw22xxx,
				   struct device_node *np)
{
	struct device_node *temp;
	int ret = -1;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_parse_led_cdev");

	for_each_child_of_node(np, temp) {
		ret = of_property_read_string(temp, "aw22xxx,name",
					       &aw22xxx->cdev.name);
		if (ret < 0) {
			dev_err(aw22xxx->dev,
				"Failure reading led name, ret = %d\n", ret);
			return ret;
		}
		ret = of_property_read_u32(temp, "aw22xxx,imax",
					    &aw22xxx->imax);
		if (ret < 0) {
			dev_err(aw22xxx->dev,
				"Failure reading imax, ret = %d\n", ret);
			return ret;
		}
		ret = of_property_read_u32(temp, "aw22xxx,brightness",
					    &aw22xxx->cdev.brightness);
		if (ret < 0) {
			dev_err(aw22xxx->dev,
				"Failure reading brightness, ret = %d\n", ret);
			return ret;
		}
		ret = of_property_read_u32(temp, "aw22xxx,max_brightness",
					    &aw22xxx->cdev.max_brightness);
		if (ret < 0) {
			dev_err(aw22xxx->dev,
				"Failure reading max brightness, ret = %d\n",
				ret);
			return ret;
		}
	}

	INIT_WORK(&aw22xxx->brightness_work, aw22xxx_brightness_work);
	INIT_WORK(&aw22xxx->task_work, aw22xxx_task_work);

	aw22xxx->cdev.brightness_set = aw22xxx_set_brightness;
	ret = led_classdev_register(aw22xxx->dev, &aw22xxx->cdev);
	if (ret) {
		dev_err(aw22xxx->dev, "unable to register led ret=%d\n", ret);
		return ret;
	}

	ret = sysfs_create_group(&aw22xxx->cdev.dev->kobj,
				  &aw22xxx_attribute_group);
	if (ret) {
		dev_err(aw22xxx->dev, "led sysfs ret: %d\n", ret);
		led_classdev_unregister(&aw22xxx->cdev);
		return ret;
	}

	return 0;
}

/*
 * aw22xxx_i2c_probe -- posizione 4 della sequenza-bersaglio, lotto 3.
 * ffffff800879f74c, 1904 byte. E' la radice del driver: senza di lei il
 * resto del file non e' raggiungibile e clang lo elimina.
 *
 * Il nome della variabile locale non e' una scelta di stile: la macro
 * mutex_init() stringifica il proprio argomento, e il testo che finisce nel
 * binario si legge a 0xffffff80091c71b9 ed e' "&aw22xxx->cfg_lock". Perche'
 * questo file
 * produca quel letterale la locale deve chiamarsi "aw22xxx" e il campo
 * "cfg_lock". Le altre funzioni del file continuano a chiamare "chip" il
 * proprio parametro, che nel binario non lascia traccia.
 *
 * Dopo la mutex_init il codice di fabbrica **non usa piu' la locale**: ogni
 * blocco successivo ricarica il globale.
 *     ffffff800879f804:	f947f796 	ldr	x22, [x28,#4072]   (parse_dt)
 *     ffffff800879f87c:	f947f796 	ldr	x22, [x28,#4072]   (richiesta gpio)
 *     ffffff800879f94c:	f947f796 	ldr	x22, [x28,#4072]   (hw_reset)
 *     ffffff800879f958:	f947f796 	ldr	x22, [x28,#4072]   (read_chipid)
 *     ffffff800879fb08:	f947f796 	ldr	x22, [x28,#4072]   (interrupt_setup)
 *     ffffff800879fc20:	f947f79a 	ldr	x26, [x28,#4072]   (parse_led_cdev)
 *     ffffff800879fe20:	f947f794 	ldr	x20, [x28,#4072]   (timer e work)
 * Una locale tenuta in un registro callee-saved non verrebbe mai ricaricata:
 * queste sette letture sono la prova che il sorgente scrive
 * g_aw22xxx_chip->... e non aw22xxx->... da qui in poi. Dentro ogni blocco,
 * invece, il puntatore resta in registro attraverso le chiamate -- cioe' e'
 * stato passato per valore a una funzione poi incorporata.
 * L'ultimo blocco (timer e work) e' l'unico che non ha un nome stampato:
 * si sa che il globale e' letto una volta sola li' e poi tenuto in registro,
 * non si sa se il sorgente lo facesse con una funzione o con una locale.
 * Qui e' scritto con una locale, e questa nota dichiara cio' che non e'
 * stato possibile stabilire.
 *
 * i2c_check_functionality(client->adapter, I2C_FUNC_I2C): il binario carica
 * adapter->algo->functionality e la chiama indirettamente, poi prova il bit 0
 * ("tbnz w0,#0"@ffffff800879f7a8) -- I2C_FUNC_I2C vale 0x00000001.
 *
 * gpio_is_valid(x) e' "cmp w1,#0x1ff; b.hi" (confronto **senza segno**: un
 * gpio negativo e' maggiore di 0x1ff e salta), cioe' 0 <= x < 512 con
 * ARCH_NR_GPIOS = 512.
 *
 * devm_gpio_request_one riceve 0 per il reset (GPIOF_OUT_INIT_LOW, "mov
 * x2,xzr"@ffffff800879f898) e 1 per l'interrupt (GPIOF_IN, "orr w2,wzr,#0x1"
 * @ffffff800879f8d8).
 *
 * devm_request_threaded_irq: handler NULL ("mov x2,xzr"@ffffff800879fbc4),
 * thread_fn aw22xxx_irq ("add x3,x3,#0x8"@ffffff800879fbb4 su
 * adrp 0xffffff80087a0000), flag 0x2002 ("mov w4,#0x2002"@ffffff800879fbbc)
 * = IRQF_TRIGGER_FALLING (0x2) | IRQF_ONESHOT (0x2000), nome "aw22xxx"
 * @0xffffff80091c7b6a -- **non** "leds_aw22xxx", che sta a
 * 0xffffff80091c7b65, cinque byte prima, ed e' un letterale diverso di cui
 * questo e' la coda
 * condivisa (il linker unisce le code): l'indirizzo dell'istruzione
 * ("add x5,x5,#0xb6a"@ffffff800879fbb8) e' quello che conta.
 *
 * SCOPERTA (revisione lotto 3, D5, non un difetto): "leds_aw22xxx" a
 * 0xffffff80091c7b65 non e' solo un letterale di coda condivisa -- e'
 * anche il "modname" dei quattro descrittori _ddebug di
 * aw22xxx_cfg_loaded (pr_debug), letti da .rela.dyn seguendo la struct
 * __dyndbg (passo 0x28 da 0xffffff8009a1d3c8). Gli stessi descrittori
 * portano "filename" = "drivers/misc/mediatek/leds/mt6771/aw22127/
 * leds-aw22xxx.c" (non "aw22xxx.c" ne' senza il sottolivello "aw22127/") e
 * quattro numeri di riga del sorgente di fabbrica: 610, 618, 621, 625. Il
 * KBUILD_MODNAME di fabbrica e' dunque "leds_aw22xxx" (l'oggetto si
 * chiama leds-aw22xxx.o).
 *
 * CORRETTO (dopo D5): questo file vive ora in
 * drivers/misc/mediatek/leds/mt6771/aw22127/leds-aw22xxx.c, con
 * obj-$(CONFIG_WTK_LEDS_AW22127) += leds-aw22xxx.o nel Makefile della
 * sottodirectory e += aw22127/ in quello di mt6771/: KBUILD_MODNAME
 * risultante e' "leds_aw22xxx", verificato leggendo il .ddebug del nostro
 * stesso build (tools/ddebug.py out-aw22127/vmlinux --modulo leds_aw22xxx),
 * non solo dedotto da scripts/Makefile.build. E' una correzione di **dati**
 * (i quattro _ddebug del nostro build avevano modname="aw22xxx" e filename
 * diverso), invisibile a ogni misura di dimensione.
 *
 * Verificato leggendo il .ddebug del nostro stesso build (non solo dedotto):
 *   $ ddebug.py out-aw22127/vmlinux --tutte --modulo leds_aw22xxx
 *   leds_aw22xxx  .../leds/mt6771/aw22127/leds-aw22xxx.c  aw22xxx_cfg_loaded  2792
 *   leds_aw22xxx  .../leds/mt6771/aw22127/leds-aw22xxx.c  aw22xxx_cfg_loaded  2801
 *   leds_aw22xxx  .../leds/mt6771/aw22127/leds-aw22xxx.c  aw22xxx_cfg_loaded  2807
 *   leds_aw22xxx  .../leds/mt6771/aw22127/leds-aw22xxx.c  aw22xxx_cfg_loaded  2814
 * modname e filename coincidono coi quattro di fabbrica; i numeri di riga no
 * (2792/2801/2807/2814, passi 9/6/7 -- non 610/618/621/625, passi 8/3/4), il
 * che e' atteso nell'assoluto (~2150 righe di commento derivativo precedono
 * qui la funzione, assenti di fabbrica: ogni riga riportata e' quella di
 * *chiusura* della chiamata pr_debug multilinea, non della prima, sia qui
 * sia -- verosimilmente -- di fabbrica). Il *passo* fra le quattro chiamate
 * pero' non e' lo stesso (9/6/7 contro 8/3/4): con ogni probabilita' perche'
 * qui aw22xxx_i2c_write(...) va a capo su due righe per stare sotto 80
 * colonne mentre di fabbrica ci stava su una, non perche' manchi codice --
 * ma e' un'ipotesi con la causa nominata, non un fatto accertato: e' la
 * prima misura diretta che il progetto ha sulla *forma* del sorgente di
 * fabbrica in questo punto, e dice che il nostro codice, pur semanticamente
 * equivalente (37/37 presenti, 34/37 di dimensione uguale) e con lo stesso
 * numero e ordine di rami (cfg==1 / flag==1 con if-else annidato / else),
 * non e' provato disposto sulle stesse righe: solo il suo comportamento
 * lo e'. Non e' un difetto da correggere a forza.
 *
 * hrtimer_start riceve 10 000 000 000 ns = 10 s:
 *   ffffff800879fe38:	d29c8001 	mov	x1, #0xe400
 *   ffffff800879fe48:	f2aa8161 	movk	x1, #0x540b, lsl #16
 *   ffffff800879fe64:	f2c00041 	movk	x1, #0x2, lsl #32
 * cioe' 0x2540be400, che e' ktime_set(10, 0). Il modo e' HRTIMER_MODE_REL
 * ("orr w3,wzr,#0x1"@ffffff800879fe68) e il clock CLOCK_MONOTONIC
 * ("orr w1,wzr,#0x1"@ffffff800879fe24 verso hrtimer_init).
 *
 * Il risultato di aw22xxx_parse_led_cdev **non e' controllato**: tutti i suoi
 * rami d'errore confluiscono a ffffff800879fe20, che e' l'inizializzazione
 * del timer, non un'uscita. Riprodotto cosi'.
 *
 * i2c_set_clientdata(client, ...) compare **due volte**
 * ("str x0,[x21,#184]"@ffffff800879f7f0 e "str x26,[x21,#184]"
 * @ffffff800879fc40): non e' un refuso di questo file.
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_i2c_probe"@0xffffff80091c71a7
 *   "check_functionality failed\n"@0xffffff80092a30d9 (letterale condiviso,
 *      fuori dall'area di questo driver: conta l'indirizzo, non la vicinanza)
 *   a 0xffffff80091c71b9 "&aw22xxx->cfg_lock" (lo produce mutex_init, non e'
 *      un letterale di questo file: percio' non e' scritto nella forma
 *      "testo"@indirizzo, che tools/verificacitazioni.py riserva ai letterali
 *      che stanno davvero nel .c)
 *   "%s: failed to parse device tree node\n"@0xffffff80091c71cc
 *   "aw22xxx_rst"@0xffffff80091c71f2
 *   "%s: rst request failed\n"@0xffffff80091c71fe
 *   "aw22xxx_int"@0xffffff80091c7216
 *   "%s: int request failed\n"@0xffffff80091c7222
 *   "%s: aw22xxx_read_chipid failed ret=%d\n"@0xffffff80091c723a
 *   "%s: failed to request IRQ %d: %d\n"@0xffffff80091c7261
 *   "%s skipping IRQ registration\n"@0xffffff8009263461 (condiviso)
 *   "aw22xxx"@0xffffff80091c7b6a
 *   KERN_INFO "%s probe completed successfully!\n"@0xffffff80091c7283
 */
static int aw22xxx_i2c_probe(struct i2c_client *client,
			      const struct i2c_device_id *id)
{
	struct aw22xxx_chip *aw22xxx;
	struct device_node *np = client->dev.of_node;
	int ret;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_i2c_probe");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "check_functionality failed\n");
		return -EIO;
	}

	aw22xxx = devm_kzalloc(&client->dev, sizeof(struct aw22xxx_chip),
			        GFP_KERNEL);
	g_aw22xxx_chip = aw22xxx;
	if (aw22xxx == NULL)
		return -ENOMEM;

	aw22xxx->client = client;
	aw22xxx->dev = &client->dev;
	i2c_set_clientdata(client, aw22xxx);
	mutex_init(&aw22xxx->cfg_lock);

	if (np) {
		ret = aw22xxx_parse_dt(&client->dev, g_aw22xxx_chip, np);
		if (ret) {
			dev_err(&client->dev,
				"%s: failed to parse device tree node\n",
				"aw22xxx_i2c_probe");
			ret = -1;
			goto err_kfree;
		}
	} else {
		g_aw22xxx_chip->reset_gpio = -1;
		g_aw22xxx_chip->irq_gpio = -1;
	}

	if (gpio_is_valid(g_aw22xxx_chip->reset_gpio)) {
		ret = devm_gpio_request_one(&client->dev,
					     g_aw22xxx_chip->reset_gpio,
					     GPIOF_OUT_INIT_LOW, "aw22xxx_rst");
		if (ret) {
			dev_err(&client->dev, "%s: rst request failed\n",
				"aw22xxx_i2c_probe");
			goto err_kfree;
		}
	}

	if (gpio_is_valid(g_aw22xxx_chip->irq_gpio)) {
		ret = devm_gpio_request_one(&client->dev,
					     g_aw22xxx_chip->irq_gpio,
					     GPIOF_IN, "aw22xxx_int");
		if (ret) {
			dev_err(&client->dev, "%s: int request failed\n",
				"aw22xxx_i2c_probe");
			goto err_kfree;
		}
	}

	aw22xxx_hw_reset(g_aw22xxx_chip);

	ret = aw22xxx_read_chipid(g_aw22xxx_chip);
	if (ret < 0) {
		dev_err(&client->dev, "%s: aw22xxx_read_chipid failed ret=%d\n",
			"aw22xxx_i2c_probe", ret);
		goto err_gpio_free;
	}

	if (gpio_is_valid(g_aw22xxx_chip->irq_gpio) &&
	    !(g_aw22xxx_chip->flags & 0x01)) {
		aw22xxx_interrupt_setup(g_aw22xxx_chip);
		ret = devm_request_threaded_irq(&client->dev,
						 gpio_to_irq(g_aw22xxx_chip->irq_gpio),
						 NULL, aw22xxx_irq,
						 IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
						 "aw22xxx", g_aw22xxx_chip);
		if (ret) {
			dev_err(&client->dev,
				"%s: failed to request IRQ %d: %d\n",
				"aw22xxx_i2c_probe",
				gpio_to_irq(g_aw22xxx_chip->irq_gpio), ret);
			goto err_gpio_free;
		}
	} else {
		dev_info(&client->dev, "%s skipping IRQ registration\n",
			  "aw22xxx_i2c_probe");
		g_aw22xxx_chip->flags |= 0x01;
	}

	i2c_set_clientdata(client, g_aw22xxx_chip);
	aw22xxx_parse_led_cdev(g_aw22xxx_chip, np);

	aw22xxx = g_aw22xxx_chip;
	hrtimer_init(&aw22xxx->fw_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	aw22xxx->fw_timer.function = aw22xxx_fw_timer_func;
	INIT_WORK(&aw22xxx->fw_work, aw22xxx_fw_work_routine);
	INIT_WORK(&aw22xxx->cfg_work, aw22xxx_cfg_work_routine);
	hrtimer_start(&aw22xxx->fw_timer, ktime_set(10, 0), HRTIMER_MODE_REL);

	printk(KERN_INFO "%s probe completed successfully!\n",
	       "aw22xxx_i2c_probe");
	return 0;

err_gpio_free:
	devm_gpio_free(&client->dev, g_aw22xxx_chip->reset_gpio);
	devm_gpio_free(&client->dev, g_aw22xxx_chip->irq_gpio);
err_kfree:
	devm_kfree(&client->dev, g_aw22xxx_chip);
	g_aw22xxx_chip = NULL;
	return ret;
}

/*
 * aw22xxx_i2c_remove -- posizione 5 della sequenza-bersaglio, lotto 3.
 * ffffff800879febc, 176 byte.
 *
 * Simmetrica alla probe, con due differenze che sono nel binario e vanno
 * riprodotte:
 *   - le due devm_gpio_free sono protette da gpio_is_valid
 *     ("cmp w1,#0x1ff; b.hi"@ffffff800879ff28 e ffffff800879ff3c), mentre
 *     nella via d'errore della probe non lo sono;
 *   - devm_free_irq viene chiamata sempre, anche quando l'interrupt non e'
 *     stato registrato.
 *
 * Restituisce 0 fisso ("mov w0,wzr"@ffffff800879ff60).
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_i2c_remove"@0xffffff80091c7c19
 */
static int aw22xxx_i2c_remove(struct i2c_client *client)
{
	struct aw22xxx_chip *aw22xxx = i2c_get_clientdata(client);

	printk(KERN_INFO "%s: enter\n", "aw22xxx_i2c_remove");

	sysfs_remove_group(&aw22xxx->cdev.dev->kobj, &aw22xxx_attribute_group);
	led_classdev_unregister(&aw22xxx->cdev);

	devm_free_irq(&client->dev, gpio_to_irq(aw22xxx->irq_gpio), aw22xxx);

	if (gpio_is_valid(aw22xxx->reset_gpio))
		devm_gpio_free(&client->dev, aw22xxx->reset_gpio);
	if (gpio_is_valid(aw22xxx->irq_gpio))
		devm_gpio_free(&client->dev, aw22xxx->irq_gpio);

	devm_kfree(&client->dev, aw22xxx);

	return 0;
}

/*
 * aw22xxx_hw_reset -- posizione 6 della sequenza-bersaglio, lotto 2.
 * ffffff800879ff6c, 156 byte.
 *
 * printk(KERN_INFO "%s: enter\n"@0xffffff80091c719a,
 *        "aw22xxx_hw_reset"@0xffffff80091c736f);
 * mutex_lock(&chip->cfg_lock) -- il mutex sta a chip+0x220 (ffffff800879ff90:
 *   "add x19,x20,#0x220"). Il lotto 3 lo ha reso un membro vero: il nome del
 *   campo viene dalla stringificazione che mutex_init() ne fa dentro
 *   aw22xxx_i2c_probe: a 0xffffff80091c71b9 si legge "&aw22xxx->cfg_lock".
 * Se chip e' NULL, salta il controllo GPIO e va diretto al dev_err finale
 * (cbz x20, +0x70): un altro bug/scorciatoia di fabbrica, riprodotto.
 * Altrimenti, se chip->reset_gpio (offset 0x240, letto nel lotto 2) supera
 * `gpio_is_valid` -- confronto SENZA SEGNO con 0x1ff, cioe' 0 <= n < 512, e
 * un gpio negativo lo fallisce -- spegne il gpio (valore 0), msleep(1), poi
 * lo alza (valore 1), msleep(1). Se il gpio non e' valido o chip e' NULL:
 * dev_err. (Questa nota diceva «<= 0x1ff» e «> 0x1ff», cioe' la forma CON
 * SEGNO che la correzione del gpio ha confutato: la fabbrica emette `b.hi`,
 * non `b.gt`.)
 *
 * dev_err(chip->dev, "%s:  failed\n"@0xffffff80091c7380,
 *         "aw22xxx_hw_reset"@0xffffff80091c736f) -- "%s:  failed\n" ha
 *   **due spazi** dopo i due punti, letti byte per byte, non un refuso di
 *   questo file. Il primo argomento viene da "ldr x0,[x20,#8]"
 *   @ffffff800879ffdc: **una sola istruzione**, cioe' il puntatore
 *   memorizzato a chip+0x08 (campo "dev"), non &chip->client->dev, che
 *   richiederebbe una ldr per client piu' una add per l'offset di dev
 *   dentro struct i2c_client.
 */
static void aw22xxx_hw_reset(struct aw22xxx_chip *chip)
{
	printk(KERN_INFO "%s: enter\n", "aw22xxx_hw_reset");
	mutex_lock(&chip->cfg_lock);
	/*
	 * `gpio_is_valid`, NON un confronto scritto a mano. Il binario ha UN
	 * SOLO confronto e SENZA SEGNO -- "7107fc1f cmp"@0xffffff800879ffa4
	 * con 0x1ff e "540001a8 b.hi"@0xffffff800879ffa8 -- ed e' la forma in
	 * cui clang riduce `n >= 0 && n < ARCH_NR_GPIOS` (512, verificato in
	 * include/asm-generic/gpio.h) quando `n` e' un `int`: le due prove
	 * diventano una sola su `(unsigned)n`. Scritto `<= 0x1ff` a mano si
	 * perde il `>= 0` e il binario emette "b.gt", CON segno: la dimensione
	 * non cambia di un byte, la condizione si'. Il cappello di questo file
	 * lo diceva gia' a riga 821; il codice non lo seguiva.
	 */
	if (chip && gpio_is_valid(chip->reset_gpio)) {
		gpiod_set_raw_value_cansleep(gpio_to_desc(chip->reset_gpio), 0);
		msleep(1);
		gpiod_set_raw_value_cansleep(gpio_to_desc(chip->reset_gpio), 1);
		msleep(1);
	} else {
		dev_err(chip->dev, "%s:  failed\n", "aw22xxx_hw_reset");
	}
	mutex_unlock(&chip->cfg_lock);
}

/*
 * aw22xxx_irq -- posizione 7 della sequenza-bersaglio, lotto 3.
 * ffffff80087a0008, 344 byte. E' il thread_fn passato a
 * devm_request_threaded_irq da aw22xxx_i2c_probe; il secondo argomento
 * (x1, il "dev_id") e' il chip.
 *
 * Legge il registro di stato 0x0a, e agisce **solo se il bit 0 e' alzato**
 * ("tbz w21,#0"@ffffff80087a006c salta tutto il corpo). Poi porta il chip in
 * standby: azzera il bit 1 del registro 0x04, poi il bit 0 del registro
 * 0x04, poi il bit 0 del registro 0x02, e aspetta 2 ms. Le tre maschere sono
 * "and w2,w8,#0xfffffffd" (~0x02) e due "and w2,w8,#0xfffffffe" (~0x01).
 *
 * Restituisce 1 = IRQ_HANDLED ("orr w0,wzr,#0x1"@ffffff80087a0150), sempre,
 * anche quando il bit 0 non era alzato.
 *
 * Le quattro letture usano **quattro celle di stack distinte** (sp+8, sp+12,
 * sp+16, sp+20): una sola variabile ne userebbe una sola, quindi il sorgente
 * di fabbrica ne dichiara quattro. Non cambia una sola istruzione ne' un
 * byte -- gli offset immediati hanno la stessa codifica -- ed e' scritto
 * cosi' perche' e' cio' che il disassemblato mostra.
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_irq"@0xffffff80091c74cf
 *   KERN_INFO "%s: reg INTST=0x%x\n"@0xffffff80091c74b9 (lo stesso formato
 *      che usa aw22xxx_interrupt_clear)
 *   KERN_INFO "%s: functions compelte!\n"@0xffffff80091c74db ("compelte" e'
 *      nel binario, non e' un refuso di questo file)
 *   KERN_INFO "%s: enter standby mode!\n"@0xffffff80091c74f6
 *   KERN_INFO "%s exit\n"@0xffffff80091c7511 (senza i due punti, a
 *      differenza di "%s: exit\n"@0xffffff80091c79f3 che usa il lotto del
 *      firmware)
 */
static irqreturn_t aw22xxx_irq(int irq, void *data)
{
	struct aw22xxx_chip *chip = data;
	u8 reg_val;
	u8 reg_val1;
	u8 reg_val2;
	u8 reg_val3;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_irq");

	aw22xxx_i2c_read(chip, 0x0a, &reg_val);
	printk(KERN_INFO "%s: reg INTST=0x%x\n", "aw22xxx_irq", reg_val);

	if (reg_val & 0x01) {
		printk(KERN_INFO "%s: functions compelte!\n", "aw22xxx_irq");

		aw22xxx_i2c_write(chip, 0xff, 0x00);
		aw22xxx_i2c_read(chip, 0x04, &reg_val1);
		aw22xxx_i2c_write(chip, 0x04, reg_val1 & (~0x02));
		aw22xxx_i2c_read(chip, 0x04, &reg_val2);
		aw22xxx_i2c_write(chip, 0x04, reg_val2 & (~0x01));
		aw22xxx_i2c_read(chip, 0x02, &reg_val3);
		aw22xxx_i2c_write(chip, 0x02, reg_val3 & (~0x01));
		msleep(2);

		printk(KERN_INFO "%s: enter standby mode!\n", "aw22xxx_irq");
	}

	printk(KERN_INFO "%s exit\n", "aw22xxx_irq");

	return IRQ_HANDLED;
}

/*
 * posizione 8 della sequenza-bersaglio: aw22xxx_i2c_read, lotto 1.
 * ffffff80087a0160, 164 byte.
 *
 * Stessa forma di aw22xxx_i2c_write (leggi, ritenta una volta, msleep(1) fra
 * i tentativi). Il valore letto viene scritto nell'output *value* **solo se
 * uno dei due tentativi e' riuscito**: il ramo del secondo fallimento salta
 * a "mov w0,w20" (ffffff80087a01c8), cioe' **una istruzione dopo** lo
 * "strb w20,[x19]" (ffffff80087a01c4), scavalcandolo:
 *   ffffff80087a01c0:	37f800e0 	tbnz	w0, #31, ffffff80087a01dc
 *   ffffff80087a01c4:	39000274 	strb	w20, [x19]
 *   ffffff80087a01c8:	2a1403e0 	mov	w0, w20
 *   [...]
 *   ffffff80087a0200:	17fffff2 	b	ffffff80087a01c8
 * Il codice d'errore viene comunque restituito al chiamante in w0, in
 * entrambi i percorsi.
 *
 * Stringhe:
 *   KERN_ERR "%s: i2c_read cnt=%d error=%d\n"@0xffffff80091c7458
 *   "aw22xxx_i2c_read"@0xffffff80091c7478
 */
static int aw22xxx_i2c_read(struct aw22xxx_chip *chip, u8 reg, u8 *value)
{
	int ret;

	ret = i2c_smbus_read_byte_data(chip->client, reg);
	if (ret < 0) {
		printk(KERN_ERR "%s: i2c_read cnt=%d error=%d\n",
		       "aw22xxx_i2c_read", 0, ret);
		msleep(1);
		ret = i2c_smbus_read_byte_data(chip->client, reg);
		if (ret < 0) {
			printk(KERN_ERR "%s: i2c_read cnt=%d error=%d\n",
			       "aw22xxx_i2c_read", 1, ret);
			msleep(1);
			return ret;
		}
	}
	*value = (u8)ret;
	return ret;
}

/*
 * aw22xxx_brightness_work -- posizione 9 della sequenza-bersaglio, lotto 3.
 * ffffff80087a0204, 464 byte.
 *
 * Risale al chip sottraendo 0x160 dal "work" ricevuto
 * ("sub x19,x0,#0x160"@ffffff80087a0220): e' container_of piegato in una
 * costante, e insieme a "add x2,x0,#0x150"@ffffff80087a05c4 di
 * aw22xxx_set_brightness fissa brightness_work a chip+0x160.
 *
 * Spegne sempre (scrive 0xff nel registro 0x05, azzera il bit 1 del
 * registro 0x04 e il bit 0 del registro 0x02, aspetta 2 ms) e **poi**
 * riaccende solo se cdev.brightness e' diverso da zero:
 *   ffffff80087a029c:	d1052288 	sub	x8, x20, #0x148
 *   ffffff80087a02a0:	b9400108 	ldr	w8, [x8]
 *   ffffff80087a02a4:	34000848 	cbz	w8, ffffff80087a03ac
 * x20 = work = chip+0x160, quindi x8 = chip+0x18 = cdev+0x8, che e' il campo
 * "brightness" della led_classdev -- lo stesso che aw22xxx_set_brightness
 * scrive con "str w1,[x0,#8]"@ffffff80087a05b8.
 *
 * chip->imax e' letto **come byte** ("ldrb w8,[x19,#588]"@ffffff80087a02f8)
 * e saturato a 0xf ("cmp w8,#0xf; csel w20,w8,w9,cc" con w9 = 0xf): e' il
 * valore grezzo, non passa per aw22xxx_imax_clamp_table -- a differenza di
 * aw22xxx_led_init, che invece la tabella la usa (e li' la lettura e'
 * davvero a 32 bit: "ldr w8,[x19,#588]"@ffffff80087a1c5c -- le due letture
 * *sono* diverse di fabbrica, chip->imax e' u32 e va indicizzato per intero
 * nella tabella).
 *
 * DIFETTO CORRETTO (revisione lotto 3, D6): con "imax = chip->imax; if
 * (imax > 0x0f) imax = 0x0f;" il nostro build leggeva a 32 bit ("ldr"),
 * non a 8 ("ldrb") come la fabbrica -- divergenza di forma, invisibile
 * alla dimensione (464/464 prima e dopo). Provate sei forme (cast
 * esplicito, maschera "& 0xff", espressione condizionale, dichiarazione
 * anticipata, lettura per puntatore a byte): solo "min_t(u8, chip->imax,
 * 0x0f)" -- l'idioma normale del kernel per un clamp -- produce
 * "ldrb+mov+cmp+csel" istruzione per istruzione identico alla fabbrica,
 * dimensione invariata (464/464 qui, 476/476 in aw22xxx_task_work sotto).
 * Compilato fuori dall'albero con la riga di comando esatta di questo
 * file per la controprova.
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_brightness_work"@0xffffff80091c763d
 */
static void aw22xxx_brightness_work(struct work_struct *work)
{
	struct aw22xxx_chip *chip =
		container_of(work, struct aw22xxx_chip, brightness_work);
	u8 reg_val;
	u8 reg_val1;
	u8 reg_val2;
	u8 reg_val3;
	u8 reg_val4;
	u8 imax;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_brightness_work");

	aw22xxx_i2c_write(chip, 0x05, 0xff);
	aw22xxx_i2c_read(chip, 0x04, &reg_val);
	aw22xxx_i2c_write(chip, 0x04, reg_val & (~0x02));
	aw22xxx_i2c_read(chip, 0x02, &reg_val1);
	aw22xxx_i2c_write(chip, 0x02, reg_val1 & (~0x01));
	msleep(2);

	if (chip->cdev.brightness) {
		aw22xxx_i2c_read(chip, 0x02, &reg_val2);
		aw22xxx_i2c_write(chip, 0x02, reg_val2 | 0x01);
		msleep(2);
		aw22xxx_i2c_read(chip, 0x04, &reg_val3);
		aw22xxx_i2c_write(chip, 0x04, reg_val3 | 0x01);

		imax = min_t(u8, chip->imax, 0x0f);
		aw22xxx_i2c_write(chip, 0xff, 0x00);
		aw22xxx_i2c_write(chip, 0x0b, imax);

		aw22xxx_i2c_write(chip, 0x21, 0xe1);
		aw22xxx_i2c_write(chip, 0x22, 0x00);
		aw22xxx_i2c_write(chip, 0x20, 0x02);
		aw22xxx_i2c_write(chip, 0x23, 0x3d);
		aw22xxx_i2c_write(chip, 0x20, 0x00);

		aw22xxx_i2c_write(chip, 0x05, 0x82);

		aw22xxx_i2c_read(chip, 0x04, &reg_val4);
		aw22xxx_i2c_write(chip, 0x04, reg_val4 | 0x02);
	}
}

/*
 * aw22xxx_task_work -- posizione 10 della sequenza-bersaglio, lotto 3.
 * ffffff80087a03d4, 476 byte.
 *
 * Stessa forma esatta di aw22xxx_brightness_work, con tre differenze:
 *   - risale al chip sottraendo 0x180 ("sub x19,x0,#0x180"@ffffff80087a03f0),
 *     cioe' il work sta a chip+0x180;
 *   - la condizione non e' cdev.brightness ma chip->task0
 *     ("ldrb w8,[x20,#212]"@ffffff80087a046c, x20 = work = chip+0x180,
 *     quindi chip+0x254);
 *   - al posto della scrittura fissa "0x05 <- 0x82" scrive nei registri 0x05
 *     e 0x06 i due campi task0 e task1:
 *       ffffff80087a0544:	39495262 	ldrb	w2, [x19,#596]   (chip+0x254)
 *       ffffff80087a0554:	39495662 	ldrb	w2, [x19,#597]   (chip+0x255)
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_task_work"@0xffffff80091c7655
 */
static void aw22xxx_task_work(struct work_struct *work)
{
	struct aw22xxx_chip *chip =
		container_of(work, struct aw22xxx_chip, task_work);
	u8 reg_val;
	u8 reg_val1;
	u8 reg_val2;
	u8 reg_val3;
	u8 reg_val4;
	u8 imax;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_task_work");

	aw22xxx_i2c_write(chip, 0x05, 0xff);
	aw22xxx_i2c_read(chip, 0x04, &reg_val);
	aw22xxx_i2c_write(chip, 0x04, reg_val & (~0x02));
	aw22xxx_i2c_read(chip, 0x02, &reg_val1);
	aw22xxx_i2c_write(chip, 0x02, reg_val1 & (~0x01));
	msleep(2);

	if (chip->task0) {
		aw22xxx_i2c_read(chip, 0x02, &reg_val2);
		aw22xxx_i2c_write(chip, 0x02, reg_val2 | 0x01);
		msleep(2);
		aw22xxx_i2c_read(chip, 0x04, &reg_val3);
		aw22xxx_i2c_write(chip, 0x04, reg_val3 | 0x01);

		imax = min_t(u8, chip->imax, 0x0f);
		aw22xxx_i2c_write(chip, 0xff, 0x00);
		aw22xxx_i2c_write(chip, 0x0b, imax);

		aw22xxx_i2c_write(chip, 0x21, 0xe1);
		aw22xxx_i2c_write(chip, 0x22, 0x00);
		aw22xxx_i2c_write(chip, 0x20, 0x02);
		aw22xxx_i2c_write(chip, 0x23, 0x3d);
		aw22xxx_i2c_write(chip, 0x20, 0x00);

		aw22xxx_i2c_write(chip, 0x05, chip->task0);
		aw22xxx_i2c_write(chip, 0x06, chip->task1);

		aw22xxx_i2c_read(chip, 0x04, &reg_val4);
		aw22xxx_i2c_write(chip, 0x04, reg_val4 | 0x02);
	}
}

/*
 * aw22xxx_set_brightness -- posizione 11 della sequenza-bersaglio, lotto 3.
 * ffffff80087a05b0, 40 byte. E' il puntatore che aw22xxx_parse_led_cdev
 * scrive in cdev.brightness_set ("str x8,[x26,#48]"@ffffff800879fd4c, cioe'
 * cdev+0x20).
 *
 * Scrive la luminosita' nel campo della led_classdev
 * ("str w1,[x0,#8]"@ffffff80087a05b8) e accoda brightness_work. Il "8"
 * passato a queue_work_on ("orr w0,wzr,#0x8"@ffffff80087a05c8) e'
 * WORK_CPU_UNBOUND, vedi la nota in testa al file: qui e' schedule_work.
 */
static void aw22xxx_set_brightness(struct led_classdev *cdev,
				    enum led_brightness brightness)
{
	struct aw22xxx_chip *chip =
		container_of(cdev, struct aw22xxx_chip, cdev);

	chip->cdev.brightness = brightness;
	schedule_work(&chip->brightness_work);
}

/*
 * aw22xxx_reg_show -- posizione 12 della sequenza-bersaglio, lotto 1.
 * ffffff80087a05d8, 232 byte.
 *
 * Legge il registro 0xff (ffffff80087a0604: "orr w1,wzr,#0xff"): se il
 * valore letto e' diverso da zero, il dump ignora aw22xxx_reg_access e
 * stampa tutti i 256 registri; altrimenti stampa solo quelli marcati
 * leggibili (bit 0 di aw22xxx_reg_access[i]). Il significato del registro
 * 0xff (probabilmente un flag di test/debug del chip) non e' documentato da
 * nessuna stringa nel binario: il comportamento e' riprodotto cosi' com'e',
 * senza dargli un nome che il disassemblato non da'.
 *
 * Stringa del formato per riga: "reg:0x%02x=0x%02x \n"@0xffffff80091c7667
 * (lo spazio prima di "\n" e' nel binario, byte per byte -- non e' un
 * refuso di questo file).
 *
 * SCARTO NON CHIUSO, +8 byte (240 contro 232). Non e' un difetto di
 * dato o di logica (il ciclo scritto qui e' semanticamente identico:
 * verificato con reg_access[255] ora corretto -- vedi sopra -- che la
 * funzione stampa lo stesso insieme di registri). E' rotazione del ciclo:
 * di fabbrica la guardia "probe_val" cade *dentro* il corpo per
 * fall-through, mentre qui il corpo e' preceduto da un salto al latch:
 *   FABBRICA                                NOSTRO
 *   cbnz w24, <corpo>    (se probe_val)     b <latch>
 *   ldrb w8,[x25,x22]                       <corpo>
 *   tbz  w8,#0, <salta>                     latch: add/cmp/b.eq uscita
 *   <corpo>                                 cbnz w24, <corpo>
 *   add x22,x22,#1                          ldrb w8,[x25,x22]
 *   cmp x22,#0x100                          tbnz w8,#0, <corpo>
 *   b.ne <inizio ciclo>                     b <latch>            <- +1 istr.
 *                                                                 e il b
 *                                                                 iniziale <- +1
 * Tre forme di sorgente provate (if positivo invece di continue: 240,
 * invariata; while con incremento esplicito: 252, peggiora; if annidati:
 * 240, invariata), nessuna chiude lo scarto. Resta aperto, con la causa
 * nominata (rotazione del ciclo che clang r383902 sceglie diversamente da
 * r353983c) invece di essere lasciato come "non indagato".
 */
static ssize_t aw22xxx_reg_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	u8 probe_val = 0;
	u8 val = 0;
	ssize_t len = 0;
	int i;

	aw22xxx_i2c_read(chip, 0xff, &probe_val);
	for (i = 0; i < 256; i++) {
		if (!probe_val && !(aw22xxx_reg_access[i] & 0x1))
			continue;
		aw22xxx_i2c_read(chip, i, &val);
		len += snprintf(buf + len, PAGE_SIZE - len,
				 "reg:0x%02x=0x%02x \n", i, val);
	}
	return len;
}

/*
 * aw22xxx_reg_store -- posizione 13 della sequenza-bersaglio, lotto 1.
 * ffffff80087a06c0, 136 byte.
 *
 * Formato di sscanf: "%x %x"@0xffffff80091411fa. Se sono stati letti
 * esattamente due valori (cmp w0,#0x2), scrive col registro.
 *
 * DIFETTO CORRETTO (revisione lotto 3, D4): con due scalari separati
 * ("unsigned int reg = 0, value = 0") la funzione misurava 132 contro 136
 * di fabbrica (-4), scarto lasciato "non indagato" per due lotti. Di
 * fabbrica i due destinatari di sscanf sono elementi di un array, non due
 * variabili indipendenti -- il segnale era nel disassemblato:
 *   FABBRICA: mov x9,sp ; orr x3,x9,#0x4 ; mov x2,sp ; ... ; ldp w1,w2,[sp]
 *   (scambiato per rumore di compilatore in due revisioni precedenti: non
 *   lo era. "mov x9,sp" seguito da uno scostamento e' cio' che clang emette
 *   quando i due destinatari sono &databuf[0]/&databuf[1], non due &reg/
 *   &value separati: l'indirizzo base si calcola una volta sola.)
 * Con "unsigned int databuf[2]" la funzione e' 136/136, **istruzione per
 * istruzione** identica alla fabbrica, incluso quel "mov x9,sp; orr
 * x3,x9,#0x4" (compilato fuori dall'albero con la riga di comando esatta
 * di questo file, out-aw22127/.../.aw22xxx.o.cmd -- vedi
 * docs/bringup/driver-riscritti.md).
 */
static ssize_t aw22xxx_reg_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int databuf[2] = { 0, 0 };

	if (sscanf(buf, "%x %x", &databuf[0], &databuf[1]) == 2)
		aw22xxx_i2c_write(chip, (u8)databuf[0], (u8)databuf[1]);
	return count;
}

/*
 * aw22xxx_hwen_show -- posizione 14, lotto 2. ffffff80087a0748, 72 byte.
 * Stringa: "hwen=%d\n"@0xffffff80091c7680.
 */
static ssize_t aw22xxx_hwen_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);

	return snprintf(buf, PAGE_SIZE, "hwen=%d\n",
			 gpiod_get_raw_value(gpio_to_desc(chip->reset_gpio)));
}

/*
 * aw22xxx_hwen_store -- posizione 15, lotto 2. ffffff80087a0790, 236 byte.
 *
 * Formato di sscanf: "%x"@0xffffff800926f8f8, una sola conversione
 * (cmp w0,#0x1). A differenza di aw22xxx_task0_store, qui "val" **e'
 * azzerato prima** della chiamata -- "str wzr,[sp,#4]"@ffffff80087a07cc,
 * sedici byte prima della "bl sscanf"@ffffff80087a07d0 -- e il valore di
 * ritorno e' controllato (cmp w0,#0x1 / b.ne). Se il valore parsato e' 1,
 * chiama aw22xxx_hw_reset(chip); altrimenti (qualunque altro valore) fa
 * l'equivalente di uno spegnimento diretto, inline (non richiama
 * aw22xxx_hw_reset -- l'unica bl a quella funzione, in tutto il blocco, e'
 * nel ramo val==1):
 *   printk(KERN_INFO "%s: enter\n"@0xffffff80091c719a, "aw22xxx_hw_off"@0xffffff80091c7689);
 *   if (chip && gpio_is_valid(chip->reset_gpio)) gpiod_set_raw_value_cansleep(.., 0), msleep(1);
 * Il controllo su chip nullo c'e' anche qui, nella stessa forma di
 * aw22xxx_hw_reset ("cbz x20"@ffffff800879ff9c), e salta allo stesso dev_err:
 *   ffffff80087a080c:	b4000154 	cbz	x20, ffffff80087a0834
 * (x20 = x21 - 0x10 = chip, "sub x20,x21,#0x10"@ffffff80087a07e0.)
 *   else dev_err(chip->dev, "%s:  failed\n"@0xffffff80091c7380,
 *                "aw22xxx_hw_off"@0xffffff80091c7689);
 * "aw22xxx_hw_off" non e' un simbolo separato (non e' fra le 37): con ogni
 * probabilita' era una funzione a se' che clang ha incorporato qui -- non
 * la si scrive come funzione a parte per lo stesso motivo di
 * aw22xxx_read_chipid (vedi le trappole del piano).
 */
static ssize_t aw22xxx_hwen_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val = 0;	/* "str wzr,[sp,#4]"@ffffff80087a07cc */

	if (sscanf(buf, "%x", &val) == 1) {
		if (val == 1) {
			aw22xxx_hw_reset(chip);
		} else {
			printk(KERN_INFO "%s: enter\n", "aw22xxx_hw_off");
			if (chip && gpio_is_valid(chip->reset_gpio)) {
				gpiod_set_raw_value_cansleep(
					gpio_to_desc(chip->reset_gpio), 0);
				msleep(1);
			} else {
				dev_err(chip->dev, "%s:  failed\n",
					"aw22xxx_hw_off");
			}
		}
	}
	return count;
}

/*
 * aw22xxx_fw_show -- posizione 16, lotto 2. ffffff80087a087c, 48 byte.
 * Stringa: "firmware name = %s\n"@0xffffff80091c7698. Il secondo argomento
 * (x3, adrp+add a 0x91c76ac) e' un'altra stringa costante, non un campo del
 * chip: "aw22xxx_fw.bin"@0xffffff80091c76ac -- il nome del file e' sempre
 * quello, l'attributo non lo legge da nessuno stato.
 */
static ssize_t aw22xxx_fw_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "firmware name = %s\n",
			 "aw22xxx_fw.bin");
}

/*
 * aw22xxx_fw_store -- posizione 17, lotto 2. ffffff80087a08ac, 152 byte.
 *
 * Formato "%x"@0xffffff800926f8f8, una conversione (cmp w0,#0x1). Se convertito,
 * chip->fw = (u8)val (offset 570, letto nel lotto 2) **sempre**, anche se
 * il confronto per accodare il lavoro e' su val==1 soltanto (il campo si
 * scrive comunque, per qualunque valore convertito -- la strb precede il
 * b.ne che salta l'accodamento). Se val==1, schedule_work(&chip->fw_work)
 * (offset cdev+0x190=chip+0x1a0, lotto 2).
 *
 * SCARTO NON CHIUSO, +4 byte (156 contro 152). Di fabbrica il valore e'
 * caricato **una volta sola** e il confronto precede la strb:
 *   ffffff80087a08f4:	b94007e8 	ldr	w8, [sp,#4]
 *   ffffff80087a08f8:	7100051f 	cmp	w8, #0x1
 *   ffffff80087a08fc:	3908ea88 	strb	w8, [x20,#570]
 *   ffffff80087a0900:	540000c1 	b.ne	ffffff80087a0918
 * Con questa forma clang r383902 ricarica "val" dopo la strb, perche' con
 * -fno-strict-aliasing non puo' escludere che chip->fw aliasi lo slot di
 * stack il cui indirizzo e' passato a sscanf ("ldr w8,[sp,#4]" due volte).
 * Tre forme misurate, tutte semanticamente identiche:
 *   chip->fw = (u8)val; if (val == 1)        -> 156  (questa)
 *   chip->fw = (u8)val; if (chip->fw == 1)   -> 156, e in piu' un
 *                                               "and w9,w8,#0xff" che la
 *                                               fabbrica non ha: esclusa
 *   if (val==1) {strb; accoda} else {strb}   -> 152, identica istruzione per
 *                                               istruzione alla fabbrica
 * La terza chiude il numero ma duplica l'assegnamento in due rami: e' codice
 * scritto per il compilatore, non letto dal disassemblato. Non e' adottata.
 *
 * LOTTO 3 -- la causa vera, che le tre misure precedenti non nominavano.
 * Non e' l'hoisting di SimplifyCFG. E' che **di fabbrica il valore prodotto
 * da sscanf viene letto una volta sola**, e quella singola lettura alimenta
 * sia il confronto sia la strb. La prova sta nel confronto con le due
 * funzioni gemelle di questo stesso driver, che invece **coincidono**:
 *
 *   aw22xxx_cfg_store (148/148) rilegge il **campo**, non la variabile, e
 *   clang gli inoltra il valore appena scritto senza ricaricare nulla:
 *     ffffff80087a0a30:	394013e8 	ldrb	w8, [sp,#4]
 *     ffffff80087a0a34:	39091e88 	strb	w8, [x20,#583]
 *     ffffff80087a0a38:	340000c8 	cbz	w8, ffffff80087a0a50
 *   -- una ldrb (otto bit), perche' il confronto e' su chip->cfg.
 *
 *   aw22xxx_imax_store (180/180) legge la tabella **prima** della str, cosi'
 *   la sola lettura basta a entrambi gli usi (e' la correzione R8).
 *
 * In aw22xxx_fw_store la fabbrica ha invece una "ldr" a **32 bit** e un
 * "cmp w8,#0x1" a 32 bit: il confronto e' su "val", non su chip->fw (che e'
 * un byte e darebbe una ldrb). Quindi il sorgente di fabbrica legge "val"
 * una volta e la usa due volte, mentre questa forma la nomina due volte e
 * clang, con -fno-strict-aliasing, deve ricaricarla dopo la strb.
 *
 * Misurato: aggiungendo una copia intermedia ("prova = val; chip->fw =
 * (u8)prova; if (prova == 1)") la funzione fa **152/152**, identica alla
 * fabbrica. Anche questa e' una variabile che esiste solo per far caricare
 * una volta al compilatore: non e' adottata, per la stessa ragione della
 * terza forma.
 *
 * Resta dunque una terza spiegazione che non si puo' provare ne' smentire:
 * il clang di fabbrica e' r353983c, che non esiste piu' negli archivi
 * disponibili (HANDOFF.md §5), e questo build usa r383902. Una singola
 * lettura al posto di due e' esattamente il genere di differenza che due
 * versioni di GVN possono produrre dallo stesso sorgente. Lo scarto resta
 * **aperto e dichiarato**, con la causa ora nominata: non l'hoisting, ma il
 * numero di letture dello slot di stack.
 *
 * VERIFICA AGGIUNTIVA (revisione lotto 3): il rifiuto sopra era stato
 * pronunciato senza aver provato la forma "databuf[N]" che chiude i due
 * scarti gemelli di aw22xxx_reg_store e aw22xxx_rgb_store (vedi D4 li').
 * Provata anche qui, fuori dall'albero con la riga di comando esatta:
 *   unsigned int databuf[1] = { 0 };
 *   if (sscanf(buf, "%x", &databuf[0]) == 1) { chip->fw = (u8)databuf[0]; ... }
 * Risultato: 156, invariata rispetto alla forma con scalare singolo. La
 * forma-array non era la causa qui (un solo elemento non da' a clang nulla
 * da materializzare in piu'), quindi il rifiuto **regge**, ma ora poggia su
 * un esperimento e non su un'analogia con reg_store/rgb_store che si e'
 * rivelata non trasferibile.
 */
static ssize_t aw22xxx_fw_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val = 0;	/* "str wzr,[sp,#4]"@ffffff80087a08e4 */

	if (sscanf(buf, "%x", &val) == 1) {
		chip->fw = (u8)val;
		if (val == 1)
			schedule_work(&chip->fw_work);
	}
	return count;
}

/*
 * aw22xxx_cfg_show -- posizione 18, lotto 2. ffffff80087a0944, 164 byte.
 *
 * Stampa quattordici righe "cfg[%x] = %s\n"@0xffffff80091c76bb, una per
 * nome in aw22xxx_cfg_names[] (0xffffff8009932650, letta byte per byte:
 * vedi la tabella sotto), poi una riga finale
 * "current cfg = %s\n"@0xffffff80091c76c9 (citazione su una sola riga: se
 * spezzata a fine riga, RE_CITAZIONE di verificacitazioni.py non la aggancia
 * e il letterale risulta non citato) col nome all'indice **chip->effect**,
 * non chip->cfg:
 *   ffffff80087a09b8:	39491b08 	ldrb	w8, [x24,#582]
 *   ffffff80087a09c4:	8b081b23 	add	x3, x25, x8, lsl #6
 * x24 = dev_get_drvdata(dev) = &chip->cdev, quindi cdev+582 = chip+0x256,
 * che e' il campo "effect" (lo stesso 582 che leggono aw22xxx_effect_show
 * @ffffff80087a0a94 e aw22xxx_effect_store @ffffff80087a0af0). Il campo
 * "cfg" e' a cdev+583 ed e' quello che aw22xxx_cfg_store scrive
 * ("strb w8,[x20,#583]"@ffffff80087a0a34): la riga "current cfg" **non** lo
 * legge. E' una stranezza del driver di fabbrica -- l'attributo "cfg"
 * stampa come corrente il nome indicizzato da "effect" -- e va riprodotta,
 * non normalizzata. L'indice e' moltiplicato per 0x40 ("lsl #6", la
 * dimensione di una voce di aw22xxx_cfg_names).
 */
static ssize_t aw22xxx_cfg_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	ssize_t len = 0;
	int i;

	for (i = 0; i < 14; i++)
		len += snprintf(buf + len, PAGE_SIZE - len, "cfg[%x] = %s\n",
				 i, aw22xxx_cfg_names[i]);
	len += snprintf(buf + len, PAGE_SIZE - len, "current cfg = %s\n",
			 aw22xxx_cfg_names[chip->effect]);
	return len;
}

/*
 * aw22xxx_cfg_store -- posizione 19, lotto 2. ffffff80087a09e8, 148 byte.
 *
 * Formato "%d"@0xffffff8009216030 (non "%x": diverso da hwen/fw/task0/
 * task1/imax, verificato leggendo i byte all'indirizzo), una conversione
 * (cmp w0,#0x1). chip->cfg = (u8)val (offset 583) sempre, se convertito; se
 * il valore e' diverso da zero (cbz salta altrimenti),
 * schedule_work(&chip->cfg_work) (offset cdev+0x1b0=chip+0x1c0; il "8" di
 * "orr w0,wzr,#0x8"@ffffff80087a0a48 e' WORK_CPU_UNBOUND, vedi la nota in
 * testa al file).
 */
static ssize_t aw22xxx_cfg_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val = 0;	/* "str wzr,[sp,#4]"@ffffff80087a0a20 */

	if (sscanf(buf, "%d", &val) == 1) {
		chip->cfg = (u8)val;
		if (chip->cfg)
			schedule_work(&chip->cfg_work);
	}
	return count;
}

/*
 * aw22xxx_effect_show -- posizione 20, lotto 2. ffffff80087a0a7c, 48 byte.
 *
 * Stringa: "effect = 0x%02x\n"@0xffffff80091c7758. **Corretto da
 * tools/verificacitazioni.py**: la prima stesura scambiava questa funzione
 * con aw22xxx_ledeffect_show, citando l'indirizzo sbagliato (0x7756 invece
 * di 0x758, e il testo lungo "idflag..." invece della coda corta). Il
 * binario ha due letterali che condividono i byte finali (il compilatore
 * unisce le stringhe: il letterale lungo comincia a 0xffffff80091c7752 e,
 * sei byte dopo -- "idflag" e' lungo sei caratteri -- la sua coda comincia
 * a 0xffffff80091c7758, che e' quella citata qui sopra); l'offset
 * dell'istruzione ("add x2,x2,#0x758"@ffffff80087a0a90) e' quello vero,
 * letto sul disassemblato di *questa* funzione, non dedotto per somiglianza
 * con l'altra. Legge chip->effect (offset 0x256, lotto 2).
 */
static ssize_t aw22xxx_effect_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);

	return snprintf(buf, PAGE_SIZE, "effect = 0x%02x\n", chip->effect);
}

/*
 * aw22xxx_effect_store -- posizione 21, lotto 2. ffffff80087a0aac, 108 byte.
 * Formato "%d"@0xffffff8009216030 (come cfg_store). **Nessun controllo sul
 * valore di ritorno di sscanf** (nessun cmp/branch fra bl sscanf e la
 * lettura di [sp,#4]): stesso bug di "val" non inizializzato letto in
 * aw22xxx_task0_store, riprodotto allo stesso modo (val non inizializzato).
 */
static ssize_t aw22xxx_effect_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val;

	sscanf(buf, "%d", &val);
	chip->effect = (u8)val;
	return count;
}

/*
 * aw22xxx_imax_show -- posizione 22, lotto 2. ffffff80087a0b18, 464 byte.
 *
 * Stampa tredici righe "imax[%x] = %s\n"@0xffffff80091c76db, una per nome
 * in aw22xxx_imax_names[] (0xffffff80099329d0, 13 voci da 0x20 byte), poi
 * una riga finale "current id = 0x%02x, imax = %s\n"@0xffffff80091c76ea con
 * l'indice raw (chip->imax, offset 572, lotto 2) e il nome corrispondente
 * nella stessa tabella indicizzato da quel valore -- **non validato**: se
 * chip->imax >= 13, l'indice nella tabella esce dai limiti (lettura oltre
 * aw22xxx_imax_names, comportamento di fabbrica non nostro: "add x4,x20,
 * x3,lsl#5"@ffffff80087a0cc4 non ha nessun confronto prima).
 */
static ssize_t aw22xxx_imax_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	ssize_t len = 0;
	int i;

	for (i = 0; i < 13; i++)
		len += snprintf(buf + len, PAGE_SIZE - len, "imax[%x] = %s\n",
				 i, aw22xxx_imax_names[i]);
	len += snprintf(buf + len, PAGE_SIZE - len,
			 "current id = 0x%02x, imax = %s\n",
			 chip->imax, aw22xxx_imax_names[chip->imax]);
	return len;
}

/*
 * aw22xxx_imax_store -- posizione 23, lotto 2. ffffff80087a0ce8, 180 byte.
 *
 * Formato "%x"@0xffffff800926f8f8, non controllato (nessun cmp dopo bl sscanf: stesso
 * bug del valore non inizializzato). chip->imax = val (offset 572, **senza
 * troncare a u8** -- l'unico fra i campi a un byte del lotto 2 che e' un
 * int pieno, str non strb: "str w8,[x21,#572]"@ffffff80087a0d40). **La
 * lettura della tabella precede la scrittura del campo**, non la segue:
 *   ffffff80087a0d3c:	38686929 	ldrb	w9, [x9,x8]
 *   ffffff80087a0d40:	b9023ea8 	str	w8, [x21,#572]
 * L'ordine non e' cosmetico: con -fno-strict-aliasing clang non puo'
 * escludere che chip->imax aliasi lo slot di stack passato a sscanf, quindi
 * se la str venisse prima dovrebbe ricaricare "val" per indicizzare la
 * tabella (+4 byte, misurati: 184 contro 180).
 * La tabella e' aw22xxx_imax_clamp_table a
 * 0xffffff8008f55cd8 (indicizzata dal valore raw, **anche questa senza
 * limiti**): min(table[val], 0xf) -- "cmp w9,#0xf; csel w21,w9,w8,cc" con
 * w8=0xf, cioe' se il byte e' minore di 0xf lo usa cosi' com'e', altrimenti
 * lo satura a 0xf. Scrive prima il registro 0xff con valore 0, poi il
 * registro 0x0b col valore appena letto/saturato.
 */
static ssize_t aw22xxx_imax_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val;
	u8 clamp;

	sscanf(buf, "%x", &val);
	clamp = aw22xxx_imax_clamp_table[val];
	chip->imax = val;
	if (clamp >= 0xf)
		clamp = 0xf;
	aw22xxx_i2c_write(chip, 0xff, 0x00);
	aw22xxx_i2c_write(chip, 0x0b, clamp);
	return count;
}

/*
 * aw22xxx_rgb_show -- posizione 24, lotto 2. ffffff80087a0d9c, 312 byte.
 * Nove righe "rgb[%d] = 0x%06x\n"@0xffffff80091c770a, una per chip->rgb[i]
 * (offset 584+4i, lotto 2, i=0..8).
 */
static ssize_t aw22xxx_rgb_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	ssize_t len = 0;
	int i;

	for (i = 0; i < 9; i++)
		len += snprintf(buf + len, PAGE_SIZE - len,
				 "rgb[%d] = 0x%06x\n", i, chip->rgb[i]);
	return len;
}

/*
 * aw22xxx_rgb_store -- posizione 25, lotto 2. ffffff80087a0ed4, 124 byte.
 * Formato "%x %x"@0xffffff80091411fa (stesso di aw22xxx_reg_store), indice
 * e valore. **Nessun controllo sul valore di ritorno di sscanf ne' sui
 * limiti dell'indice** (0..8 atteso, non verificato: "add x8,x20,x8,lsl#2"
 * @ffffff80087a0f1c legge l'indice grezzo, un indice >8 scrive fuori
 * chip->rgb[]).
 *
 * DIFETTO CORRETTO (revisione lotto 3, D4): con "unsigned int idx, val"
 * separati la funzione misurava 116 contro 124 di fabbrica (-8), stesso
 * scarto e stessa causa di aw22xxx_reg_store qui sopra -- il sorgente di
 * fabbrica usa un array di due elementi, non due scalari. Con
 * "unsigned int databuf[2]" la funzione e' 124/124, istruzione per
 * istruzione identica alla fabbrica (stesso "mov x9,sp; add x3,x9,#0x4"
 * verificato compilando fuori dall'albero).
 */
static ssize_t aw22xxx_rgb_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int databuf[2];

	sscanf(buf, "%x %x", &databuf[0], &databuf[1]);
	chip->rgb[databuf[0]] = databuf[1];
	return count;
}

/*
 * aw22xxx_task0_show -- posizione 26 della sequenza-bersaglio, lotto 1.
 * ffffff80087a0f50, 48 byte. E' una delle cinque *_show da 48 byte (fw,
 * effect, task0, task1, ledeffect): lo stampo minimo del driver, calibrato
 * qui prima di ripeterlo (vedi lotto 2).
 *
 * Stringa: "task0 = 0x%02x\n"@0xffffff80091c7722.
 */
static ssize_t aw22xxx_task0_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);

	return snprintf(buf, PAGE_SIZE, "task0 = 0x%02x\n", chip->task0);
}

/*
 * aw22xxx_task0_store -- posizione 27 della sequenza-bersaglio, lotto 1.
 * ffffff80087a0f80, 128 byte.
 *
 * Formato di sscanf: "%x"@0xffffff800926f8f8: **una sola conversione**, non
 * due -- letto byte per byte (3 byte esatti: 0x25 0x78 0x00). Il
 * valore restituito da sscanf **non e' controllato** (nessun cmp/branch fra
 * la bl sscanf e la ldr successiva): se sscanf non trova nulla da
 * convertire, "val" resta quello che c'era sullo stack -- non inizializzato
 * (non c'e' nessuno "str wzr,[sp,#4]" prima della chiamata, a differenza di
 * aw22xxx_reg_store che azzera i suoi locali). E' un bug di fabbrica
 * (lettura di memoria non inizializzata), riprodotto qui non dichiarando
 * "val" con inizializzatore -- non corretto.
 */
static ssize_t aw22xxx_task0_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val;

	sscanf(buf, "%x", &val);
	chip->task0 = (u8)val;
	schedule_work(&chip->task_work);
	return count;
}

/*
 * aw22xxx_task1_show -- posizione 28, lotto 2. ffffff80087a1000, 48 byte.
 * Stessa forma esatta di aw22xxx_task0_show: stampo minimo confermato una
 * quarta volta (fw, effect, task0 gia' nel lotto 1; task1 qui).
 * Stringa: "task1 = 0x%02x\n"@0xffffff80091c7738. Campo chip->task1
 * (offset 581, lotto 2).
 */
static ssize_t aw22xxx_task1_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);

	return snprintf(buf, PAGE_SIZE, "task1 = 0x%02x\n", chip->task1);
}

/*
 * aw22xxx_task1_store -- posizione 29, lotto 2. ffffff80087a1030, 108 byte.
 * Stessa forma di aw22xxx_effect_store: formato "%x"@0xffffff800926f8f8, valore di
 * ritorno **non controllato**, "val" non inizializzato se sscanf fallisce.
 * Scrive chip->task1 (offset 581) e ritorna, senza accodare nessun lavoro
 * (a differenza di task0_store: qui non c'e' nessuna bl queue_work_on).
 */
static ssize_t aw22xxx_task1_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct aw22xxx_chip *chip = container_of(cdev, struct aw22xxx_chip, cdev);
	unsigned int val;

	sscanf(buf, "%x", &val);
	chip->task1 = (u8)val;
	return count;
}

/*
 * aw22xxx_ledeffect_show -- posizione 30, lotto 2. ffffff80087a109c, 48 byte.
 * Quinta e ultima delle *_show da 48 byte (fw, effect, task0, task1, ora
 * ledeffect): lo stampo torna identico una quinta volta. A differenza delle
 * altre, legge da un **globale**, non da chip: "ldr w3,[x8,#4084]" da
 * adrp x8,0xffffff8009d05000 -- indirizzo 0xffffff8009d05ff4, 12 byte dopo
 * g_aw22xxx_chip (0xffffff8009d05fe8).
 *
 * Stringa: "idflageffect = 0x%02x\n"@0xffffff80091c7752 (**non**
 * "effect = 0x%02x\n": quella e' la coda condivisa che usa
 * aw22xxx_effect_show, sei byte dopo -- vedi la nota li'. L'offset
 * dell'istruzione qui e' "add x2,x2,#0x752"@ffffff80087a10b4, letto su
 * *questa* funzione).
 */
static int g_aw22xxx_ledeffect;

static ssize_t aw22xxx_ledeffect_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	return snprintf(buf, PAGE_SIZE, "idflageffect = 0x%02x\n", g_aw22xxx_ledeffect);
}

/*
 * aw22xxx_ledeffect_store -- posizione 31, lotto 2. ffffff80087a10cc,
 * 136 byte.
 *
 * Formato "%d"@0xffffff8009216030 (come cfg_store/effect_store), **non
 * controllato**: g_aw22xxx_ledeffect = val (il globale, non un campo del
 * chip) viene scritto **sempre**, anche se sscanf non ha convertito nulla.
 * Poi: se val==2, aw22xxx_led_effect2(); se val==1, aw22xxx_led_effect1();
 * qualunque altro valore, nessuna chiamata. Le due bl (a
 * ffffff800879f3f0/f5e8) sono le uniche due di tutta l'immagine verso
 * quei due indirizzi (verificato: preambolo del piano).
 */
static ssize_t aw22xxx_ledeffect_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned int val;

	sscanf(buf, "%d", &val);
	g_aw22xxx_ledeffect = val;
	if (val == 2)
		aw22xxx_led_effect2();
	else if (val == 1)
		aw22xxx_led_effect1();
	return count;
}

/*
 * aw22xxx_fw_timer_func -- posizione 32 della sequenza-bersaglio, lotto 3.
 * ffffff80087a1154, 72 byte.
 *
 * E' il gestore di un hrtimer, non di un timer_list: restituisce zero
 * ("mov w0,wzr"@ffffff80087a1190), e un gestore di timer_list e' void.
 * Zero e' HRTIMER_NORESTART.
 *
 * Risale al work sottraendo 0x40 dal proprio argomento
 * ("sub x2,x19,#0x40"@ffffff80087a1180): il timer sta a chip+0x1e0 e il work
 * accodato a chip+0x1a0, cioe' fw_work. Il "8" di
 * "orr w0,wzr,#0x8"@ffffff80087a1184 e' WORK_CPU_UNBOUND (schedule_work).
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_fw_timer_func"@0xffffff80091c7769
 */
static enum hrtimer_restart aw22xxx_fw_timer_func(struct hrtimer *timer)
{
	struct aw22xxx_chip *chip =
		container_of(timer, struct aw22xxx_chip, fw_timer);

	printk(KERN_INFO "%s: enter\n", "aw22xxx_fw_timer_func");

	schedule_work(&chip->fw_work);

	return HRTIMER_NORESTART;
}

/*
 * aw22xxx_fw_work_routine -- posizione 33 della sequenza-bersaglio, lotto 3.
 * ffffff80087a119c, 136 byte.
 *
 * Due printk di ingresso con due nomi diversi: il secondo,
 * "aw22xxx_fw_update"@0xffffff80091c7797, e' una funzione che clang ha
 * incorporato qui (un solo chiamante) ed e' scritta come tale.
 *
 * request_firmware_nowait(module, uevent, name, device, gfp, context, cont):
 *   x0 = xzr            -> THIS_MODULE, che senza MODULE definito e' NULL
 *   w1 = 1              -> uevent
 *   x2 = "aw22xxx_fw.bin"@0xffffff80091c76ac
 *   x3 = chip->dev      -- "sub x8,x19,#0x198; ldr x3,[x8]"
 *                          @ffffff80087a11e4/11e8: x19 = fw_work = chip+0x1a0,
 *                          quindi chip+0x08, che e' il campo "dev"
 *   w4 = 0x014000c0     -> GFP_KERNEL ("mov w4,#0xc0; movk w4,#0x140,lsl#16")
 *   x5 = chip
 *   x6 = aw22xxx_fw_loaded ("add x6,x6,#0x314"@ffffff80087a11fc su
 *        adrp 0xffffff80087a1000, cioe' 0xffffff80087a1314)
 *
 * Lo stato dell'aggiornamento e' messo a 1 prima della richiesta
 * ("orr w8,wzr,#0x1; strb w8,[x19,#171]"@ffffff80087a11dc/11e0, cioe'
 * chip+0x24b).
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_fw_work_routine"@0xffffff80091c777f
 *   "aw22xxx_fw_update"@0xffffff80091c7797
 *   "aw22xxx_fw.bin"@0xffffff80091c76ac
 */
static void aw22xxx_fw_update(struct aw22xxx_chip *chip)
{
	printk(KERN_INFO "%s: enter\n", "aw22xxx_fw_update");

	chip->fw_update = 1;
	request_firmware_nowait(THIS_MODULE, 1, "aw22xxx_fw.bin", chip->dev,
				 GFP_KERNEL, chip, aw22xxx_fw_loaded);
}

static void aw22xxx_fw_work_routine(struct work_struct *work)
{
	struct aw22xxx_chip *chip =
		container_of(work, struct aw22xxx_chip, fw_work);

	printk(KERN_INFO "%s: enter\n", "aw22xxx_fw_work_routine");

	aw22xxx_fw_update(chip);
}

/*
 * aw22xxx_cfg_work_routine -- posizione 34 della sequenza-bersaglio, lotto 3.
 * ffffff80087a1224, 240 byte. Stessa struttura: incorpora
 * "aw22xxx_cfg_update"@0xffffff80091c7ae1.
 *
 * L'indice della configurazione e' **chip->effect** (cdev-relativo 598, cioe'
 * chip+0x256: "ldrb w2,[x19,#150]"@ffffff80087a1260 con x19 = cfg_work =
 * chip+0x1c0), non chip->cfg -- la stessa stranezza gia' letta in
 * aw22xxx_cfg_show. Il limite e' 13 ("cmp x2,#0xd; b.hi"@ffffff80087a1264),
 * cioe' le quattordici voci di aw22xxx_cfg_names.
 *
 * Prima di accodare la richiesta di firmware verifica che chip->fw_update
 * valga 2 ("ldrb w8,[x20,#587]; cmp w8,#0x2"@ffffff80087a1290/1294) e prende
 * il mutex ("add x0,x19,#0x60"@ffffff80087a129c, cioe' chip+0x220 =
 * cfg_lock). **Il mutex non viene rilasciato qui**: lo rilascia
 * aw22xxx_cfg_loaded, e solo nel proprio ramo di successo. Riprodotto cosi'.
 *
 * La callback e' aw22xxx_cfg_loaded ("add x6,x6,#0xe18"@ffffff80087a12bc su
 * adrp 0xffffff80087a1000 = 0xffffff80087a1e18) -- e' il dubbio che la
 * revisione aveva lasciato aperto ("callback a aw22xxx_task1_show + 0xe18"):
 * 0xffffff80087a1e18 e' aw22xxx_cfg_loaded nella mappa dell'oracolo, e
 * l'"aw22xxx_task1_show" del disassemblato e' solo il simbolo piu' vicino
 * alla pagina con cui objdump annota l'adrp.
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_cfg_work_routine"@0xffffff80091c7ac8
 *   "aw22xxx_cfg_update"@0xffffff80091c7ae1
 *   KERN_INFO "%s: cfg name=%s\n"@0xffffff80091c7af4
 *   KERN_ERR "%s: effect 0x%02x over max value \n"@0xffffff80091c7b07
 *      (lo spazio prima di "\n" e' nel binario)
 *   KERN_ERR "%s: fw update error: not compelte \n"@0xffffff80091c7b2c
 *      ("compelte", e lo spazio prima di "\n", sono nel binario)
 */
static void aw22xxx_cfg_update(struct aw22xxx_chip *chip)
{
	printk(KERN_INFO "%s: enter\n", "aw22xxx_cfg_update");

	if (chip->effect > 13) {
		printk(KERN_ERR "%s: effect 0x%02x over max value \n",
		       "aw22xxx_cfg_update", chip->effect);
		return;
	}

	printk(KERN_INFO "%s: cfg name=%s\n", "aw22xxx_cfg_update",
	       aw22xxx_cfg_names[chip->effect]);

	if (chip->fw_update != 2) {
		printk(KERN_ERR "%s: fw update error: not compelte \n",
		       "aw22xxx_cfg_update");
		return;
	}

	mutex_lock(&chip->cfg_lock);
	request_firmware_nowait(THIS_MODULE, 1,
				 aw22xxx_cfg_names[chip->effect], chip->dev,
				 GFP_KERNEL, chip, aw22xxx_cfg_loaded);
}

static void aw22xxx_cfg_work_routine(struct work_struct *work)
{
	struct aw22xxx_chip *chip =
		container_of(work, struct aw22xxx_chip, cfg_work);

	printk(KERN_INFO "%s: enter\n", "aw22xxx_cfg_work_routine");

	aw22xxx_cfg_update(chip);
}

/*
 * aw22xxx_i2c_writes -- incorporata in aw22xxx_fw_loaded (un solo chiamante,
 * dentro il ciclo di programmazione della flash).
 * ffffff80087a19d4..ffffff80087a1a38.
 *
 * Alloca len+1 byte, ci mette il registro come primo byte
 * ("mov w8,#0x23; strb w8,[x0],#1"@ffffff80087a19ec/19f4), copia il blocco e
 * lo manda con i2c_master_send. Il valore di ritorno **non e' controllato dal
 * chiamante** e nemmeno l'esito dell'allocazione ferma il ciclo: in caso di
 * kmalloc fallita il binario stampa e prosegue con l'iterazione successiva
 * ("bl printk"@ffffff80087a1a38 -> ffffff80087a1a3c, la coda comune).
 *
 * La gfp e' GFP_KERNEL **senza** __GFP_ZERO: il binario la ricava togliendo
 * 0x8000 dalla costante usata per la kzalloc precedente
 * ("sub w22,w23,#0x8,lsl #12"@ffffff80087a1968, con w23 = 0x014080c0),
 * quindi 0x014000c0. E' kmalloc, non kzalloc.
 *
 * Stringhe:
 *   KERN_ERR "%s: can not allocate memory\n"@0xffffff80091c7a79
 *   "aw22xxx_i2c_writes"@0xffffff80091c7a98
 *   KERN_ERR "%s: i2c master send error\n"@0xffffff80091c7aab
 */
static int aw22xxx_i2c_writes(struct aw22xxx_chip *chip, u8 reg,
			       u8 *buf, unsigned int len)
{
	u8 *data;
	int ret;

	data = kmalloc(len + 1, GFP_KERNEL);
	if (data == NULL) {
		printk(KERN_ERR "%s: can not allocate memory\n",
		       "aw22xxx_i2c_writes");
		return -ENOMEM;
	}

	data[0] = reg;
	memcpy(&data[1], buf, len);

	ret = i2c_master_send(chip->client, data, len + 1);
	if (ret < 0)
		printk(KERN_ERR "%s: i2c master send error\n",
		       "aw22xxx_i2c_writes");

	kfree(data);

	return ret;
}

/*
 * aw22xxx_led_init -- incorporata due volte in aw22xxx_fw_loaded
 * (ffffff80087a1bfc..ffffff80087a1cd4 e ffffff80087a1d34..ffffff80087a1e08).
 *
 * Qui, a differenza di aw22xxx_brightness_work, la corrente massima passa
 * per aw22xxx_imax_clamp_table:
 *   ffffff80087a1c5c:	b9424e68 	ldr	w8, [x19,#588]      (chip->imax)
 *   ffffff80087a1c64:	91336129 	add	x9, x9, #0xcd8      (la tabella)
 *   ffffff80087a1c6c:	38686928 	ldrb	w8, [x9,x8]
 *   ffffff80087a1c7c:	71003d1f 	cmp	w8, #0xf
 *   ffffff80087a1c80:	1a893115 	csel	w21, w8, w9, cc
 * -- lo stesso schema di aw22xxx_imax_store, indice non validato compreso.
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_led_init"@0xffffff80091c79ff
 *   KERN_INFO "%s: exit\n"@0xffffff80091c79f3
 *
 * "inline" -- la scelta e' dichiarata, non nascosta. Cio' che il binario di
 * fabbrica prova e' che questa funzione **non esiste come simbolo**: il blocco
 * ne ha 36 piu' aw22xxx_i2c_init, e "aw22xxx_led_init" c'e' solo come
 * letterale, mentre il suo corpo compare due volte dentro aw22xxx_fw_loaded.
 * Con il solo "static", il nostro clang la lascia fuori linea (due
 * chiamanti, 292 byte) e il nostro build ha **38 simboli invece di 37**, con
 * aw22xxx_fw_loaded a 2412 contro 2820 (riprodotto e verificato in una
 * revisione indipendente). Con "inline" i simboli tornano 37 e fw_loaded
 * 2828, cioe' +8.
 *
 * La ragione non e' "una soglia di clang" genericamente intesa: e' il
 * divario fra il compilatore di fabbrica, **r353983c**, e il nostro,
 * **r383902** (HANDOFF.md §5) -- r353983c non e' negli archivi disponibili
 * e non e' possibile ricompilare con quello. Che due versioni di clang
 * decidano diversamente se incorporare una funzione di 292 byte con due
 * chiamanti e' normale; "inline" qui non modella una scelta del sorgente
 * di fabbrica, **compensa il divario di versione**. Cio' che questa nota
 * non puo' affermare e' che il sorgente di fabbrica scrivesse la parola
 * "inline": quello il binario non lo dice, e per questo non si e' passati
 * a "__always_inline" (un'affermazione piu' forte di quanto il binario
 * sostenga). Dice solo che l'incorporazione c'e' stata, e questa e' la
 * riga di sorgente minima che la riproduce.
 */
static inline void aw22xxx_led_init(struct aw22xxx_chip *chip)
{
	u8 reg_val;
	u8 reg_val1;
	u8 imax;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_led_init");

	aw22xxx_i2c_write(chip, 0x01, 0x55);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x02, &reg_val);
	aw22xxx_i2c_write(chip, 0x02, reg_val | 0x01);
	msleep(2);

	imax = aw22xxx_imax_clamp_table[chip->imax];
	if (imax > 0x0f)
		imax = 0x0f;
	aw22xxx_i2c_write(chip, 0xff, 0x00);
	aw22xxx_i2c_write(chip, 0x0b, imax);

	aw22xxx_i2c_read(chip, 0x02, &reg_val1);
	aw22xxx_i2c_write(chip, 0x02, reg_val1 & (~0x01));
	msleep(2);

	printk(KERN_INFO "%s: exit\n", "aw22xxx_led_init");
}

/*
 * aw22xxx_container_update -- incorporata in aw22xxx_fw_loaded.
 * ffffff80087a1714..ffffff80087a1bec.
 *
 * Programma la flash interna del chip col contenuto del contenitore e poi
 * rilegge il "bist" per verificarlo. Il ciclo di scrittura manda blocchi da
 * al massimo 128 byte ("cmp w8,#0x80; csel w25,w8,w28,cc" con w28 = 0x80,
 * @ffffff80087a19cc/19d0) all'indirizzo corrente, scritto nei registri 0x22
 * (byte alto) e 0x21 (byte basso).
 *
 * L'esito finisce in chip->fw_update: 2 se il bist coincide o se non e'
 * ancora finito, 3 se non coincide ("orr w8,wzr,#0x2"@ffffff80087a1bbc,
 * "orr w8,wzr,#0x3"@ffffff80087a1bec, "strb w8,[x19,#587]"
 * @ffffff80087a1bf4).
 *
 * Stringhe:
 *   "aw22xxx_container_update"@0xffffff80091c7a10
 *   KERN_ERR "%s: bist check is running, reg0x24=0x%02x\n"@0xffffff80091c79b0
 *   KERN_INFO "%s: bist check pass, bist=0x%04x\n"@0xffffff80091c78ff
 *   KERN_ERR "%s: bist check fail, bist=0x%04x\n"@0xffffff80091c7a29
 *   KERN_ERR "%s: fw update failed, please reset phone\n"@0xffffff80091c7a4d
 */
static void aw22xxx_container_update(struct aw22xxx_chip *chip,
				      struct aw22xxx_container *aw22xxx_fw)
{
	unsigned int i;
	unsigned int len;
	u8 reg_val;
	u8 reg_val1;
	u8 reg_val2;
	u8 reg_val3;
	u8 reg_val4;
	unsigned int flash_bist;

	aw22xxx_i2c_write(chip, 0x24, 0x00);
	aw22xxx_i2c_write(chip, 0xff, 0x00);
	aw22xxx_i2c_write(chip, 0x01, 0x55);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x02, &reg_val);
	aw22xxx_i2c_write(chip, 0x02, reg_val | 0x01);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x04, &reg_val1);
	aw22xxx_i2c_write(chip, 0x04, reg_val1 | 0x01);

	aw22xxx_i2c_write(chip, 0x80, 0xec);
	aw22xxx_i2c_write(chip, 0x35, 0x29);
	aw22xxx_i2c_write(chip, 0x38, aw22xxx_fw->key);

	aw22xxx_i2c_write(chip, 0x22, 0x00);
	aw22xxx_i2c_write(chip, 0x21, 0x00);
	aw22xxx_i2c_write(chip, 0x20, 0x03);
	aw22xxx_i2c_write(chip, 0x30, 0x03);
	aw22xxx_i2c_write(chip, 0x23, 0x00);
	msleep(40);
	aw22xxx_i2c_write(chip, 0x30, 0x00);

	aw22xxx_i2c_write(chip, 0x22, 0x40);
	aw22xxx_i2c_write(chip, 0x21, 0x00);
	aw22xxx_i2c_write(chip, 0x30, 0x02);
	aw22xxx_i2c_write(chip, 0x23, 0x00);
	msleep(6);
	aw22xxx_i2c_write(chip, 0x30, 0x00);

	aw22xxx_i2c_write(chip, 0x22, 0x42);
	aw22xxx_i2c_write(chip, 0x21, 0x00);
	aw22xxx_i2c_write(chip, 0x30, 0x02);
	aw22xxx_i2c_write(chip, 0x23, 0x00);
	msleep(6);
	aw22xxx_i2c_write(chip, 0x30, 0x00);

	aw22xxx_i2c_write(chip, 0x22, 0x44);
	aw22xxx_i2c_write(chip, 0x21, 0x00);
	aw22xxx_i2c_write(chip, 0x30, 0x02);
	aw22xxx_i2c_write(chip, 0x23, 0x00);
	msleep(6);
	aw22xxx_i2c_write(chip, 0x30, 0x00);

	aw22xxx_i2c_write(chip, 0x20, 0x00);
	aw22xxx_i2c_write(chip, 0x20, 0x03);

	for (i = 0; i < aw22xxx_fw->len; i += len) {
		aw22xxx_i2c_write(chip, 0x22, i >> 8);
		aw22xxx_i2c_write(chip, 0x21, i & 0xff);
		aw22xxx_i2c_write(chip, 0x11, 0x01);
		aw22xxx_i2c_write(chip, 0x30, 0x04);

		len = aw22xxx_fw->len - i;
		if (len > 128)
			len = 128;
		aw22xxx_i2c_writes(chip, 0x23, &aw22xxx_fw->data[i], len);

		aw22xxx_i2c_write(chip, 0x11, 0x00);
		aw22xxx_i2c_write(chip, 0x30, 0x00);
	}

	aw22xxx_i2c_write(chip, 0x20, 0x00);

	aw22xxx_i2c_write(chip, 0x01, 0x55);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x02, &reg_val2);
	aw22xxx_i2c_write(chip, 0x02, reg_val2 | 0x01);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x04, &reg_val3);
	aw22xxx_i2c_write(chip, 0x04, reg_val3 | 0x01);

	aw22xxx_i2c_write(chip, 0x22, (aw22xxx_fw->len - 1) >> 8);
	aw22xxx_i2c_write(chip, 0x21, (aw22xxx_fw->len - 1) & 0xff);
	aw22xxx_i2c_write(chip, 0x24, 0x07);
	msleep(5);

	aw22xxx_i2c_read(chip, 0x24, &reg_val4);
	if (reg_val4 != 0x05) {
		printk(KERN_ERR "%s: bist check is running, reg0x24=0x%02x\n",
		       "aw22xxx_container_update", reg_val4);
		aw22xxx_i2c_write(chip, 0x24, 0x00);
		chip->fw_update = 2;
		return;
	}

	aw22xxx_i2c_read(chip, 0x25, &reg_val4);
	flash_bist = reg_val4;
	aw22xxx_i2c_read(chip, 0x26, &reg_val4);
	flash_bist |= reg_val4 << 8;

	if (flash_bist != aw22xxx_fw->bist) {
		printk(KERN_ERR "%s: bist check fail, bist=0x%04x\n",
		       "aw22xxx_container_update", flash_bist);
		printk(KERN_ERR "%s: fw update failed, please reset phone\n",
		       "aw22xxx_container_update");
		chip->fw_update = 3;
		return;
	}

	printk(KERN_INFO "%s: bist check pass, bist=0x%04x\n",
	       "aw22xxx_container_update", flash_bist);
	aw22xxx_i2c_write(chip, 0x24, 0x00);
	chip->fw_update = 2;
}

/*
 * aw22xxx_fw_loaded -- posizione 35 della sequenza-bersaglio, lotto 3.
 * ffffff80087a1314, 2820 byte: la funzione piu' grande del driver.
 *
 * IL FORMATO DEL FIRMWARE, dedotto dal codice che lo legge -- non da un
 * driver di terzi. Ogni campo con l'istruzione che lo estrae:
 *
 *   data[0..1]   somma di controllo, big-endian su 16 bit
 *                  ffffff80087a13f0:	39400109 	ldrb	w9, [x8]
 *                  ffffff80087a13f4:	39400508 	ldrb	w8, [x8,#1]
 *                  ffffff80087a13f8:	33181d28 	bfi	w8, w9, #8, #8
 *                la somma verificata e' quella dei byte **dal 2 in poi**,
 *                troncata a 16 bit ("and w2,w12,#0xffff"@ffffff80087a13b8),
 *                col ciclo che parte da data[2]
 *                ("orr w11,wzr,#0x2"@ffffff80087a13a0) e finisce a
 *                cont->size-1, saltato del tutto se size < 3
 *                ("cmp x9,#0x3; b.cc"@ffffff80087a1390/1394)
 *   data[2]      chip_id       ("ldrb w2,[x8,#2]"@ffffff80087a1448)
 *   data[3..18]  customer, 16 byte di testo
 *                  ffffff80087a146c:	f840b109 	ldur	x9, [x8,#11]
 *                  ffffff80087a1470:	f8403108 	ldur	x8, [x8,#3]
 *                  ffffff80087a1478:	a901a7e8 	stp	x8, x9, [sp,#24]
 *   data[19..26] project, 8 byte di testo
 *                  ffffff80087a1490:	f8413108 	ldur	x8, [x8,#19]
 *                  ffffff80087a1498:	f9000fe8 	str	x8, [sp,#24]
 *   data[27..30] version, big-endian su 32 bit
 *                  ffffff80087a14b0:	b841b108 	ldur	w8, [x8,#27]
 *                  ffffff80087a14b4:	5ac00902 	rev	w2, w8
 *   data[34..35] bist, big-endian su 16 bit
 *                  ffffff80087a14d0:	39408909 	ldrb	w9, [x8,#34]
 *                  ffffff80087a14d4:	39408d02 	ldrb	w2, [x8,#35]
 *                  ffffff80087a14d8:	33181d22 	bfi	w2, w9, #8, #8
 *   data[36]     key           ("ldrb w2,[x8,#36]"@ffffff80087a14f4)
 *   data[38..39] len, big-endian su 16 bit
 *                  ffffff80087a1510:	39409909 	ldrb	w9, [x8,#38]
 *                  ffffff80087a1514:	39409d02 	ldrb	w2, [x8,#39]
 *                  ffffff80087a1518:	33181d22 	bfi	w2, w9, #8, #8
 *   data[40..]   corpo, lungo "len" byte
 *                  ffffff80087a152c:	91004280 	add	x0, x20, #0x10
 *                  ffffff80087a1530:	9100a101 	add	x1, x8, #0x28
 *                  ffffff80087a1534:	941ad9f3 	bl	__memcpy
 *
 * COSA NON E' STATO POSSIBILE STABILIRE, e non viene inventato:
 *   - i byte 31, 32, 33 e 37 non hanno un significato ricavabile: nessuna
 *     istruzione li indirizza *singolarmente*. Sono pero' **letti**, come
 *     ogni altro byte da 2 a cont->size-1: e' il ciclo della somma di
 *     controllo qui sopra ("orr w11,wzr,#0x2"@ffffff80087a13a0 /
 *     "ldrb w12,[x8,x11]"@ffffff80087a13a4) che li attraversa e li somma.
 *     DIFETTO CORRETTO (revisione lotto 3, D2): la formulazione precedente,
 *     "non sono letti da nessuna istruzione", era falsa e smentita dal
 *     ciclo citato trenta righe sopra nello stesso commento -- alterare
 *     uno di quei byte fa fallire il checksum, quindi il driver *nota* la
 *     loro alterazione anche senza sapere cosa significhino. Che siano
 *     riempimento di allineamento resta un'ipotesi, non un fatto: il
 *     codice non li indirizza per un significato proprio, ma li somma;
 *   - customer e project sono copiati **nella stessa cella di stack**
 *     (sp+24), sedici byte il primo e otto il secondo. All'ingresso la
 *     funzione azzera trentadue byte, sp+24..sp+55
 *     ("stp xzr,xzr,[sp,#40]"@ffffff80087a1358 e
 *     "stp xzr,xzr,[sp,#24]"@ffffff80087a135c), di cui usa solo i primi
 *     sedici. Non e' distinguibile se il sorgente dichiarasse un solo buffer
 *     da 32 byte o due da 16 di cui il secondo mai scritto: qui e' scritto
 *     con un buffer solo, che e' la lettura minima. La conseguenza
 *     osservabile -- "fw project" stampato dallo stesso buffer di
 *     "fw customer", quindi con in coda cio' che resta del primo -- e' nel
 *     binario e non viene corretta;
 *   - il chip_id letto dal file e' stampato ma **mai confrontato** con
 *     chip->chipid: il firmware non viene rifiutato se e' di un altro chip.
 *
 * SCARTO NON CHIUSO, +8 byte (2828 contro 2820) una volta che
 * aw22xxx_led_init e' incorporata come di fabbrica. Le due istruzioni in piu'
 * non sono un'istruzione sbagliata: sono una scelta di disposizione dei
 * blocchi attorno a aw22xxx_i2c_writes incorporata. Confronto istruzione per
 * istruzione (705 di fabbrica contro 707 nostre, 482 uguali dopo la
 * normalizzazione delle rilocazioni): clang duplica nel nostro build la coda
 * del ciclo ("i2c_write(0x11,0); i2c_write(0x30,0); prova di fine ciclo")
 * anche sulla via in cui la kmalloc fallisce, mentre di fabbrica quella coda
 * e' condivisa fra le due vie; e nel controllo finale del bist la fabbrica
 * riusa il registro gia' caricato dove noi aggiungiamo un "mov w2,w21".
 * Nessuna delle due e' una differenza di sorgente che il disassemblato
 * mostri: restano dichiarate invece che inseguite.
 *
 * Una seconda divergenza, di forma e non di dimensione: le due letture a
 * 16 bit big-endian (somma di controllo e bist) di fabbrica sono
 * "ldrb + ldrb + bfi", nel nostro build "ldrh + lsl + rev". Tre istruzioni in
 * entrambi i casi, stessi byte, stesso valore -- clang riconosce
 * l'espressione (a<<8)|b come una lettura combinata e la fabbrica no. La
 * lettura a 32 bit (version) e' invece "ldur + rev" in entrambi.
 *
 * CHIUSA COME "NON DI SORGENTE" (revisione lotto 3). Sei forme di sorgente
 * diverse per la stessa espressione -- (d[0]<<8)|d[1], d[1]|(d[0]<<8],
 * d[0]*256+d[1], variabili locali intermedie di tipo diverso (u8, u16,
 * u32) -- compilate fuori dall'albero con la riga di comando esatta di
 * questo file danno **tutte** "ldrh+lsl+rev": nessuna produce
 * "ldrb+ldrb+bfi". La combinazione di due letture a byte in una ldrh con
 * bswap e' una decisione del backend, non del sorgente; che la fabbrica non
 * la prenda a 16 bit ma la prenda a 32 (identica nei due build per
 * "version") e' la firma della differenza di versione fra il clang di
 * fabbrica (r353983c) e il nostro (r383902), gia' nominata in
 * HANDOFF.md §5 -- non una divergenza da inseguire nel sorgente.
 *
 * Il contenitore viene liberato con kfree su tutte le vie che ci arrivano.
 * Le tre vie che escono prima (cont nullo, somma di controllo sbagliata,
 * kzalloc fallita) chiamano solo release_firmware -- e nella prima la
 * chiamano con NULL ("mov x0,xzr"@ffffff80087a13e0), che e' cio' che il
 * binario fa.
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_fw_loaded"@0xffffff80091c77a9
 *   KERN_ERR "%s: failed to read %s\n"@0xffffff80091c77bb
 *   KERN_INFO "%s: loaded %s - size: %zu\n"@0xffffff80091c77d4
 *   KERN_ERR "%s: check sum err: check_sum=0x%04x\n"@0xffffff80091c77f1
 *   KERN_INFO "%s: check sum pass : 0x%04x\n"@0xffffff80091c7818
 *   KERN_ERR "%s: Error allocating memory\n"@0xffffff80091c7837
 *   KERN_INFO "%s: fw chip_id : 0x%02x\n"@0xffffff80091c7856
 *   KERN_INFO "%s: fw customer: %s\n"@0xffffff80091c7871
 *   KERN_INFO "%s: fw project: %s\n"@0xffffff80091c7888
 *   KERN_INFO "%s: fw version : 0x%04x\n"@0xffffff80091c789e
 *   KERN_INFO "%s: fw bist : 0x%04x\n"@0xffffff80091c78b9
 *   KERN_INFO "%s: fw key : 0x%04x\n"@0xffffff80091c78d1
 *   KERN_INFO "%s: fw len : 0x%04x\n"@0xffffff80091c78e8
 *   KERN_INFO "%s: bist check pass, bist=0x%04x\n"@0xffffff80091c78ff
 *   KERN_INFO "%s: fw version: 0x%04x, force update fw\n"@0xffffff80091c7923
 *   KERN_INFO "%s: bist check fail, fw bist=0x%04x, flash bist=0x%04x\n"@0xffffff80091c794e
 *   KERN_INFO "%s: find new fw: 0x%04x, need update\n"@0xffffff80091c7988
 *   KERN_ERR "%s: bist check is running, reg0x24=0x%02x\n"@0xffffff80091c79b0
 *   KERN_INFO "%s: fw need update\n"@0xffffff80091c79dd
 *   KERN_INFO "%s: exit\n"@0xffffff80091c79f3
 *   "aw22xxx_fw.bin"@0xffffff80091c76ac
 */
static void aw22xxx_fw_loaded(const struct firmware *cont, void *context)
{
	struct aw22xxx_chip *chip = context;
	struct aw22xxx_container *aw22xxx_fw;
	/*
	 * TRENTADUE, e il binario lo dice con tre fatti che si incastrano.
	 * (a) Il canarino sta a sp+56: "f9001fe8 str"@0xffffff80087a1354 lo
	 * scrive e "f9401fe8 ldr"@0xffffff80087a1cec lo rilegge prima di
	 * <__stack_chk_fail>. (b) Due azzeramenti contigui coprono [24, 56):
	 * "a901ffff stp"@0xffffff80087a135c e "a902ffff stp"@0xffffff80087a1358
	 * -- TRENTADUE byte, e finiscono esattamente dove comincia il canarino.
	 * (c) Il puntatore che la `printk` %s riceve e' sp+24, la base di quella
	 * stessa regione: "910063e2 add"@0xffffff80087a1468.
	 *
	 * QUESTA NOTA DICEVA «SEDICI, NON TRENTADUE», E ERA SBAGLIATA DUE VOLTE.
	 * Diceva che la fabbrica legge oltre il vettore -- non lo fa: copia 16
	 * byte in 32 azzerati, e il terminatore c'e'. E dichiarava «difetto di
	 * fabbrica riprodotto» un difetto che la modifica aveva INTRODOTTO.
	 *
	 * Il ragionamento che l'aveva prodotta era giusto nei numeri e sbagliato
	 * nella conclusione. Il telaio di fabbrica e' 0xa0 con i registri a
	 * sp+64 ("d10283ff sub"@0xffffff80087a1314), il nostro e' 0xb0 con i
	 * registri a sp+80: SEDICI byte di troppo. Ma non sono nel vettore --
	 * sono SOTTO. La fabbrica tiene i suoi locali in [12, 24), noi in
	 * [8, 40): venti byte di riversamento in piu', perche' teniamo in pila
	 * valori che la fabbrica tiene nei registri. Accorciare `temp_buf` di
	 * sedici byte compensava lo spreco e faceva tornare il totale: e' il
	 * numero aggiustato perche' torni, non la causa trovata.
	 *
	 * E LA CAUSA VERA DEI QUATTRO BYTE ERA ALTROVE, trovata la sera dello
	 * stesso giorno con `tools/cercainizializzatori.py`: un `u8 reg_val = 0`
	 * poche righe piu' sotto, che emetteva una "390043ff strb wzr, [sp,#16]"
	 * che la fabbrica non ha. Tolto quello, la funzione e' ESATTA a 2820 con
	 * il vettore giusto -- e il file e' 37 su 37, con la somma degli scarti a
	 * ZERO.
	 *
	 * Vale la pena dirlo per intero: accorciare `temp_buf` faceva tornare lo
	 * stesso numero, e per la ragione sbagliata. Due difetti che si
	 * compensano danno una misura giusta e un codice sbagliato, ed e'
	 * esattamente il caso in cui il numero da solo non basta.
	 */
	unsigned char temp_buf[32] = { 0 };
	unsigned short check_sum = 0;
	u8 reg_val;
	u8 reg_val1;
	u8 reg_val2;
	unsigned int flash_bist;
	int i;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_fw_loaded");

	if (!cont) {
		printk(KERN_ERR "%s: failed to read %s\n", "aw22xxx_fw_loaded",
		       "aw22xxx_fw.bin");
		release_firmware(cont);
		return;
	}

	printk(KERN_INFO "%s: loaded %s - size: %zu\n", "aw22xxx_fw_loaded",
	       "aw22xxx_fw.bin", cont->size);

	for (i = 2; i < cont->size; i++)
		check_sum += cont->data[i];

	if (check_sum != ((cont->data[0] << 8) | cont->data[1])) {
		printk(KERN_ERR "%s: check sum err: check_sum=0x%04x\n",
		       "aw22xxx_fw_loaded", check_sum);
		release_firmware(cont);
		return;
	}

	printk(KERN_INFO "%s: check sum pass : 0x%04x\n", "aw22xxx_fw_loaded",
	       check_sum);

	aw22xxx_fw = kzalloc(cont->size + sizeof(struct aw22xxx_container),
			      GFP_KERNEL);
	if (!aw22xxx_fw) {
		release_firmware(cont);
		printk(KERN_ERR "%s: Error allocating memory\n",
		       "aw22xxx_fw_loaded");
		return;
	}

	printk(KERN_INFO "%s: fw chip_id : 0x%02x\n", "aw22xxx_fw_loaded",
	       cont->data[2]);

	memcpy(temp_buf, &cont->data[3], 16);
	printk(KERN_INFO "%s: fw customer: %s\n", "aw22xxx_fw_loaded",
	       temp_buf);

	memcpy(temp_buf, &cont->data[19], 8);
	printk(KERN_INFO "%s: fw project: %s\n", "aw22xxx_fw_loaded", temp_buf);

	aw22xxx_fw->version = (cont->data[27] << 24) | (cont->data[28] << 16) |
			       (cont->data[29] << 8) | cont->data[30];
	printk(KERN_INFO "%s: fw version : 0x%04x\n", "aw22xxx_fw_loaded",
	       aw22xxx_fw->version);

	aw22xxx_fw->bist = (cont->data[34] << 8) | cont->data[35];
	printk(KERN_INFO "%s: fw bist : 0x%04x\n", "aw22xxx_fw_loaded",
	       aw22xxx_fw->bist);

	aw22xxx_fw->key = cont->data[36];
	printk(KERN_INFO "%s: fw key : 0x%04x\n", "aw22xxx_fw_loaded",
	       aw22xxx_fw->key);

	aw22xxx_fw->len = (cont->data[38] << 8) | cont->data[39];
	printk(KERN_INFO "%s: fw len : 0x%04x\n", "aw22xxx_fw_loaded",
	       aw22xxx_fw->len);

	memcpy(aw22xxx_fw->data, &cont->data[40], aw22xxx_fw->len);
	release_firmware(cont);

	aw22xxx_i2c_write(chip, 0x01, 0x55);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x02, &reg_val1);
	aw22xxx_i2c_write(chip, 0x02, reg_val1 | 0x01);
	msleep(2);
	aw22xxx_i2c_read(chip, 0x04, &reg_val2);
	aw22xxx_i2c_write(chip, 0x04, reg_val2 | 0x01);

	aw22xxx_i2c_write(chip, 0x22, (aw22xxx_fw->len - 1) >> 8);
	aw22xxx_i2c_write(chip, 0x21, (aw22xxx_fw->len - 1) & 0xff);
	aw22xxx_i2c_write(chip, 0x24, 0x07);
	msleep(5);

	aw22xxx_i2c_read(chip, 0x24, &reg_val);
	if (reg_val == 0x05) {
		aw22xxx_i2c_read(chip, 0x25, &reg_val);
		flash_bist = reg_val;
		aw22xxx_i2c_read(chip, 0x26, &reg_val);
		flash_bist |= reg_val << 8;

		if (flash_bist == aw22xxx_fw->bist) {
			printk(KERN_INFO "%s: bist check pass, bist=0x%04x\n",
			       "aw22xxx_fw_loaded", flash_bist);
			if (!chip->fw) {
				kfree(aw22xxx_fw);
				aw22xxx_i2c_write(chip, 0x24, 0x00);
				aw22xxx_led_init(chip);
				chip->fw_update = 2;
				return;
			}
			printk(KERN_INFO
			       "%s: fw version: 0x%04x, force update fw\n",
			       "aw22xxx_fw_loaded", aw22xxx_fw->version);
		} else {
			printk(KERN_INFO
			       "%s: bist check fail, fw bist=0x%04x, flash bist=0x%04x\n",
			       "aw22xxx_fw_loaded", aw22xxx_fw->bist,
			       flash_bist);
			printk(KERN_INFO
			       "%s: find new fw: 0x%04x, need update\n",
			       "aw22xxx_fw_loaded", aw22xxx_fw->version);
		}
	} else {
		printk(KERN_ERR "%s: bist check is running, reg0x24=0x%02x\n",
		       "aw22xxx_fw_loaded", reg_val);
		printk(KERN_INFO "%s: fw need update\n", "aw22xxx_fw_loaded");
	}

	aw22xxx_container_update(chip, aw22xxx_fw);
	kfree(aw22xxx_fw);

	chip->fw = 0;
	aw22xxx_led_init(chip);

	printk(KERN_INFO "%s: exit\n", "aw22xxx_fw_loaded");
}

/*
 * aw22xxx_cfg_loaded -- posizione 36 della sequenza-bersaglio, lotto 3.
 * ffffff80087a1e18, 640 byte.
 *
 * Il file di configurazione e' una sequenza di **coppie (registro, valore)**:
 * il ciclo avanza di due byte ("add w22,w22,#0x2"@ffffff80087a204c) e
 * confronta l'indice con cont->size ("cmp x8,x26; b.hi"@ffffff80087a2050).
 * L'indice e' un int, esteso con segno per il confronto con size_t
 * ("sxtw x26,w22"@ffffff80087a2048).
 *
 * La coppia (0xff, N) seleziona una pagina, e il driver **se la ricorda**
 * ("cmp w1,#0xff"@ffffff80087a1f38, "ldrb w21,[x8,#1]"@ffffff80087a1f40).
 * Nella pagina 1 i registri da 0x10 a 0x2a non prendono il valore scritto nel
 * file ma i colori impostati via sysfs: 27 registri = 9 valori RGB da 3 byte.
 *   ffffff80087a1f80:	51004029 	sub	w9, w1, #0x10
 *   ffffff80087a1f84:	7100693f 	cmp	w9, #0x1a
 *   ffffff80087a1f88:	54000548 	b.hi	ffffff80087a2030
 *   ffffff80087a1f9c:	d361fd08 	lsr	x8, x8, #33           (divisione per 3)
 *   ffffff80087a1fa0:	b8687b8a 	ldr	w10, [x28,x8,lsl #2]  (x28 = chip+0x258)
 *   ffffff80087a1fac:	531d1108 	ubfiz	w8, w8, #3, #5        (resto * 8)
 *   ffffff80087a1fb4:	4b080128 	sub	w8, w9, w8            (16 - resto*8)
 *   ffffff80087a1fb8:	1ac8255b 	lsr	w27, w10, w8
 * cioe' rgb[(reg-0x10)/3] >> (16 - ((reg-0x10)%3)*8): byte alto, medio,
 * basso. La moltiplicazione per 0xaaaaaaab seguita da "lsr #33" e' la
 * divisione per 3 che clang genera: non e' un'altra costante del driver.
 *
 * Se invece chip->cfg vale 1 ("ldrb w9,[x19,#599]; cmp w9,#0x1"
 * @ffffff80087a1f44/1f48) la sostituzione non avviene mai e il file passa
 * cosi' com'e'.
 *
 * Fuori dalla pagina 1, la coppia (0x02, valore col bit 0 alzato) fa
 * aspettare 2 ms ("cmp w9,#0x2"@ffffff80087a2014, "tbz w8,#0"
 * @ffffff80087a2020, "bl msleep"@ffffff80087a2028).
 *
 * Il mutex preso da aw22xxx_cfg_update viene rilasciato **solo** in fondo,
 * dopo il messaggio finale ("add x0,x19,#0x220"@ffffff80087a2074,
 * "bl mutex_unlock"@ffffff80087a2078): la via che esce perche' cont e' nullo
 * salta direttamente all'epilogo (ffffff80087a1ec0 -> ffffff80087a207c) e
 * **lascia il mutex bloccato**. E' un difetto del driver di fabbrica,
 * riprodotto.
 *
 * Due dettagli di scrittura, presi dal disassemblato e non dal gusto:
 *
 *   - la coppia si legge come "*(cont->data + i + 1)", non come
 *     "cont->data[i + 1]". Non e' la stessa aritmetica: la fabbrica calcola
 *     l'indirizzo **una volta** e poi usa lo scostamento +1,
 *       ffffff80087a1f30:	8b1a0108 	add	x8, x8, x26
 *       ffffff80087a1f34:	39400101 	ldrb	w1, [x8]
 *       ffffff80087a1f40:	39400515 	ldrb	w21, [x8,#1]
 *     mentre "data[i+1]" calcola l'indice i+1 e lo riestende a 64 bit a ogni
 *     uso -- con -fno-strict-overflow clang non puo' dimostrare che
 *     sxtw(i+1) sia sxtw(i)+1, e emette "sub w9,w22,#1; sxtw x27,w9" prima di
 *     ogni ldrb. Misurato: 676 byte con "data[i+1]", 660 con
 *     "*(data + i + 1)";
 *   - "reg" e "val" sono "unsigned char", non "unsigned int". Lo provano due
 *     cose che la fabbrica ha e la versione a 32 bit no:
 *       ffffff80087a1fa0:	b8687b8a 	ldr	w10, [x28,x8,lsl #2]
 *       ffffff80087a1fa4:	0b080508 	add	w8, w8, w8, lsl #1
 *     (con "unsigned int reg" clang inverte queste due e allunga il blocco) e
 *       ffffff80087a1fdc:	12001f64 	and	w4, w27, #0xff
 *     cioe' il troncamento a otto bit del valore passato alla pr_debug, che
 *     con un "unsigned int" e' un semplice "mov w4,w27".
 *   Con le due letture insieme la funzione misura 640 byte come di fabbrica;
 *   con una sola delle due, 660.
 *
 * Le quattro pr_debug non sono un'invenzione: il binario ha quattro
 * descrittori _ddebug consecutivi (0xffffff8009a1d3c8, 0xffffff8009a1d3f0,
 * 0xffffff8009a1d418, 0xffffff8009a1d440, passo 0x28 = sizeof(struct
 * _ddebug)) e altrettante chiamate a __dynamic_pr_debug protette dal bit di
 * stampa ("tbz w8,#2" sul byte dei flag), cioe' quattro pr_debug distinte nel
 * sorgente, nell'ordine in cui compaiono qui.
 *
 * Stringhe:
 *   KERN_INFO "%s: enter\n"@0xffffff80091c719a
 *   "aw22xxx_cfg_loaded"@0xffffff80091c7b52
 *   KERN_ERR "%s: failed to read %s\n"@0xffffff80091c77bb
 *   KERN_INFO "%s: loaded %s - size: %zu\n"@0xffffff80091c77d4
 *   "%s: addr:0x%02x, data:0x%02x\n"@0xffffff80091c7be0 (senza prefisso di
 *      livello: e' pr_debug)
 *   KERN_INFO "%s: cfg update complete\n"@0xffffff80091c7bfe
 */
static void aw22xxx_cfg_loaded(const struct firmware *cont, void *context)
{
	struct aw22xxx_chip *chip = context;
	unsigned char flag = 0;
	unsigned char reg;
	unsigned char val;
	int i;

	printk(KERN_INFO "%s: enter\n", "aw22xxx_cfg_loaded");

	if (!cont) {
		printk(KERN_ERR "%s: failed to read %s\n", "aw22xxx_cfg_loaded",
		       aw22xxx_cfg_names[chip->effect]);
		release_firmware(cont);
		return;
	}

	printk(KERN_INFO "%s: loaded %s - size: %zu\n", "aw22xxx_cfg_loaded",
	       aw22xxx_cfg_names[chip->effect], cont->size);

	for (i = 0; i < cont->size; i += 2) {
		if (cont->data[i] == 0xff)
			flag = *(cont->data + i + 1);

		if (chip->cfg == 1) {
			aw22xxx_i2c_write(chip, cont->data[i],
					   *(cont->data + i + 1));
			pr_debug("%s: addr:0x%02x, data:0x%02x\n",
				 "aw22xxx_cfg_loaded", cont->data[i],
				 *(cont->data + i + 1));
		} else if (flag == 1) {
			if (cont->data[i] >= 0x10 && cont->data[i] <= 0x2a) {
				reg = cont->data[i] - 0x10;
				val = chip->rgb[reg / 3] >>
				      (16 - (reg % 3) * 8);
				aw22xxx_i2c_write(chip, cont->data[i], val);
				pr_debug("%s: addr:0x%02x, data:0x%02x\n",
					 "aw22xxx_cfg_loaded", cont->data[i],
					 val);
			} else {
				aw22xxx_i2c_write(chip, cont->data[i],
						   *(cont->data + i + 1));
				pr_debug("%s: addr:0x%02x, data:0x%02x\n",
					 "aw22xxx_cfg_loaded", cont->data[i],
					 *(cont->data + i + 1));
			}
		} else {
			aw22xxx_i2c_write(chip, cont->data[i],
					   *(cont->data + i + 1));
			pr_debug("%s: addr:0x%02x, data:0x%02x\n",
				 "aw22xxx_cfg_loaded", cont->data[i],
				 *(cont->data + i + 1));
		}

		if (flag == 0) {
			if (cont->data[i] == 0x02 &&
			    (*(cont->data + i + 1) & 0x01))
				msleep(2);
		}
	}

	release_firmware(cont);

	printk(KERN_INFO "%s: cfg update complete\n", "aw22xxx_cfg_loaded");

	mutex_unlock(&chip->cfg_lock);
}

/*
 * I dieci DEVICE_ATTR e l'attribute_group, con l'ordine dell'array
 * attrs[] identico a quello con cui relocazioni.py li ha elencati (Task 1,
 * docs/bringup/rapporti/rapporto-strutture-aw22127.txt): reg, hwen, fw,
 * cfg, effect, imax, rgb, task0, task1, ledeffect -- tutti mode 0644.
 * Non basta scrivere le funzioni: un simbolo static il cui indirizzo non e'
 * mai preso da nessuno fa scattare -Wunused-function sotto -Werror (provato
 * nel lotto 1 costruendo senza questo blocco). sysfs_create_group() li usa
 * tutti insieme dentro aw22xxx_i2c_probe (lotto 3).
 */
static DEVICE_ATTR(reg, 0644, aw22xxx_reg_show, aw22xxx_reg_store);
static DEVICE_ATTR(hwen, 0644, aw22xxx_hwen_show, aw22xxx_hwen_store);
static DEVICE_ATTR(fw, 0644, aw22xxx_fw_show, aw22xxx_fw_store);
static DEVICE_ATTR(cfg, 0644, aw22xxx_cfg_show, aw22xxx_cfg_store);
static DEVICE_ATTR(effect, 0644, aw22xxx_effect_show, aw22xxx_effect_store);
static DEVICE_ATTR(imax, 0644, aw22xxx_imax_show, aw22xxx_imax_store);
static DEVICE_ATTR(rgb, 0644, aw22xxx_rgb_show, aw22xxx_rgb_store);
static DEVICE_ATTR(task0, 0644, aw22xxx_task0_show, aw22xxx_task0_store);
static DEVICE_ATTR(task1, 0644, aw22xxx_task1_show, aw22xxx_task1_store);
static DEVICE_ATTR(ledeffect, 0644, aw22xxx_ledeffect_show, aw22xxx_ledeffect_store);

static struct attribute *aw22xxx_attributes[] = {
	&dev_attr_reg.attr,
	&dev_attr_hwen.attr,
	&dev_attr_fw.attr,
	&dev_attr_cfg.attr,
	&dev_attr_effect.attr,
	&dev_attr_imax.attr,
	&dev_attr_rgb.attr,
	&dev_attr_task0.attr,
	&dev_attr_task1.attr,
	&dev_attr_ledeffect.attr,
	NULL,
};

/*
 * "const" qui non e' uno stile a caso: unused-const-variable e'
 * disabilitato dal Makefile del kernel (Global Constraints del piano,
 * righe 754/757, cc-disable-warning), quindi un attribute_group ancora
 * incompleto (due attributi su dieci, nessun probe che lo consumi finche'
 * non arriva il lotto 3) non ferma -Werror. sysfs_create_group() accetta
 * "const struct attribute_group *": non e' una scappatoia, e' il tipo che
 * l'API del kernel si aspetta comunque. dev_attr_reg/dev_attr_task0 e
 * aw22xxx_attributes, referenziati da qui, risultano "usati" per lo stesso
 * motivo (provato: senza questo blocco, -Wunused-function segnala le
 * quattro funzioni show/store; con solo DEVICE_ATTR+array senza il group,
 * -Wunused-variable segnala l'array; con questo blocco, zero errori).
 */
static const struct attribute_group aw22xxx_attribute_group = {
	.attrs = aw22xxx_attributes,
};

/*
 * Le tre strutture di registrazione, lette con tools/relocazioni.py dalle
 * .rela.dyn dell'oracolo, non ricostruite per somiglianza.
 *
 * struct i2c_driver a 0xffffff8009932218. Solo quattro campi su tutta la
 * struttura hanno una rilocazione:
 *   $ ./venv/bin/python3 relocazioni.py oracolo/stock.elf oracolo/stock.map \
 *       --indirizzo 0xffffff8009932228
 *   0xffffff8009932228: relocato, addend 0xffffff800879f74c   (probe)
 *   0xffffff8009932230: relocato, addend 0xffffff800879febc   (remove)
 *   0xffffff8009932258: relocato, addend 0xffffff80091c718e   (driver.name)
 *   0xffffff8009932280: relocato, addend 0xffffff8009932300   (of_match_table)
 *   0xffffff80099322d0: relocato, addend 0xffffff8008f55c98   (id_table)
 * Tutti gli altri slot sono zero: niente shutdown, niente alert, niente
 * command, niente detect, niente address_list. Gli offset (+0x10 probe,
 * +0x18 remove, +0x40 driver) coincidono con la struct i2c_driver di questo
 * albero, che ha "attach_adapter" a +0x08 e "probe_new" a +0x20.
 *
 *   driver.name    = "aw22xxx_led"@0xffffff80091c718e
 *   of_match_table = 0xffffff8009932300, una sola voce piu' il terminatore:
 *     il campo "compatible" sta a +0x40 della voce (name[32]+type[32]):
 *     "awinic,aw22xxx_led"@0xffffff8009932340; la seconda voce, a
 *     0xffffff8009932400, e' tutta a zero.
 *   id_table       = 0xffffff8008f55c98, una sola voce piu' il terminatore:
 *     $ objdump -s --start-address 0xffffff8008f55c98 \
 *         --stop-address 0xffffff8008f55cd8 oracolo/stock.elf
 *      ffffff8008f55c98 61773232 7878785f 6c656400 00000000  aw22xxx_led.....
 *      ffffff8008f55ca8 00000000 00000000 00000000 00000000  ................
 *      ffffff8008f55cb8 00000000 00000000 00000000 00000000  ................
 *      ffffff8008f55cc8 00000000 00000000 00000000 00000000  ................
 *     cioe' { "aw22xxx_led", 0 } e poi la voce vuota: 32 byte l'una
 *     (I2C_NAME_SIZE 20 + allineamento + driver_data). Che finisca
 *     esattamente dove comincia aw22xxx_imax_clamp_table
 *     (0xffffff8008f55cd8) e' la controprova della dimensione.
 *
 * MODULE_DEVICE_TABLE non lascia traccia in un kernel compilato senza
 * MODULE: se il sorgente di fabbrica ce l'avesse, questo binario non lo
 * direbbe. Per la stessa ragione non e' scritto qui.
 */
static const struct of_device_id aw22xxx_dt_match[] = {
	{ .compatible = "awinic,aw22xxx_led" },
	{ },
};

static const struct i2c_device_id aw22xxx_i2c_id[] = {
	{ "aw22xxx_led", 0 },
	{ },
};

static struct i2c_driver aw22xxx_i2c_driver = {
	.driver = {
		.name = "aw22xxx_led",
		.of_match_table = aw22xxx_dt_match,
	},
	.probe = aw22xxx_i2c_probe,
	.remove = aw22xxx_i2c_remove,
	.id_table = aw22xxx_i2c_id,
};

/*
 * aw22xxx_i2c_init -- posizione 37 della sequenza-bersaglio, lotto 3.
 * ffffff80093761b4, 84 byte. E' l'unica funzione del driver in sezione
 * __init: sta a un indirizzo lontanissimo dalle altre 36 perche' il linker
 * raccoglie .init.text a parte.
 *
 * Il livello dell'initcall non e' assunto: la voce della tabella che punta a
 * questa funzione sta a 0xffffff800941fe18, e l'array initcall_levels a
 * 0xffffff800940b070 delimita i livelli
 *   [0] 0xffffff800941df70  [1] 0xffffff800941df98  [2] 0xffffff800941e118
 *   [3] 0xffffff800941e1d0  [4] 0xffffff800941e298  [5] 0xffffff800941e580
 *   [6] 0xffffff800941e730  [7] 0xffffff800941fcd0  [8] 0xffffff800941ff50
 * 0xffffff800941fe18 cade fra [7] e [8], cioe' **livello 7 = late_initcall**,
 * non il livello 6 che module_init() darebbe. Il sotto-livello (7 contro 7s,
 * cioe' late_initcall contro late_initcall_sync) non e' distinguibile da qui:
 * la tabella non li separa. E' scritto late_initcall, e questa nota dichiara
 * cio' che resta indeterminato.
 *
 * **CORREZIONE del 2026-08-21 -- la funzione di uscita ESISTE.**  Questo
 * commento diceva «Non esiste una funzione di uscita fra le 37 del blocco:
 * nessun module_exit, nessuna i2c_del_driver».  Le 37 del blocco erano
 * cercate in `oracolo/stock.map`, e stock.map **finisce a `_einittext`**
 * (0xffffff80093a8518): la symtab di stock.elf viene dalla tabella kallsyms
 * dell'immagine, che con `# CONFIG_KALLSYMS_ALL is not set` tiene solo i
 * simboli in [_stext,_etext] e [_sinittext,_einittext]
 * (alps-mtkwatch/scripts/kallsyms.c righe 48-51).  `.exit.text` comincia
 * dove la mappa finisce: nessuna `__exit` di nessun driver e' mai stata
 * visibile a nessuno strumento di questo progetto.
 *
 * Letta dai byte, la funzione c'e', a 0xffffff80093aa6f4, 28 byte:
 *   "a9bf7bfd stp"@0xffffff80093aa6f4   stp x29, x30, [sp,#-16]!
 *   "910003fd mov"@0xffffff80093aa6f8   mov x29, sp
 *   "90002c40 adrp"@0xffffff80093aa6fc  adrp x0, 0xffffff8009932000
 *   "91086000 add"@0xffffff80093aa700   add x0,x0,#0x218 -> 0xffffff8009932218
 *                                       = &aw22xxx_i2c_driver, lo stesso
 *                                       indirizzo che aw22xxx_i2c_init passa
 *                                       a i2c_register_driver
 *                                       ("91086021 add"@0xffffff80093761d8)
 *   "97db7fef bl"@0xffffff80093aa704    -> ffffff8008a8a6c0 <i2c_del_driver>
 *   "a8c17bfd ldp"@0xffffff80093aa708   ldp x29, x30, [sp],#16
 *   "d65f03c0 ret"@0xffffff80093aa70c
 * Il corpo e' una sola i2c_del_driver: nessuna printk, nessun altro accesso.
 *
 * **IL NOME E' UNA SCELTA, NON UNA MISURA.**  A differenza di
 * aw87329_pa_exit o di mir3da_exit, questa funzione non stampa niente,
 * quindi nel binario non c'e' nessun `__func__` che la nomini; e la stringa
 * "aw22xxx_i2c_exit" non compare da nessuna parte nell'immagine (cercata su
 * tutti i byte, 0 occorrenze).  `aw22xxx_i2c_exit` e' il nome simmetrico a
 * `aw22xxx_i2c_init` ed e' scritto come tale: se qualcuno trovera' il nome
 * vero, questo va cambiato senza che cambi una sola istruzione.
 *
 * Anche il `module_exit()` e' una scelta dichiarata: il puntatore che
 * `module_exit()` produce finisce in `.exitcall.exit`, che vmlinux.lds
 * scarta (`EXIT_CALL` dentro /DISCARD/, riga 98), mentre la funzione
 * sopravvive comunque perche' per un built-in `__exit` implica `__used`
 * (include/linux/init.h righe 78-85).  Dal binario non si puo' distinguere
 * «con module_exit()» da «senza».
 *
 * MISURA DI DIMENSIONE NON FATTA: questo lotto non ha un albero di build per
 * aw22127.  I 28 byte di fabbrica sono letti dal disassemblato; il confronto
 * con il nostro `.o` va ancora fatto.
 *
 * i2c_add_driver e' l'inline che chiama
 * i2c_register_driver(THIS_MODULE, driver): il primo argomento e' xzr
 * ("mov x0,xzr"@ffffff80093761dc), cioe' NULL, che e' quanto vale THIS_MODULE
 * senza MODULE definito.
 *
 * Stringhe:
 *   KERN_INFO "aw22xxx driver version %s\n"@0xffffff80091c7143
 *   "v1.1.3"@0xffffff80091c7160
 *   KERN_ERR "fail to add aw22xxx device into i2c\n"@0xffffff80091c7167
 * -- nessuna delle tre passa un nome di funzione: sono formati senza "%s"
 *    iniziale, a differenza di tutte le altre printk del driver.
 */
static int __init aw22xxx_i2c_init(void)
{
	int ret;

	printk(KERN_INFO "aw22xxx driver version %s\n", "v1.1.3");

	ret = i2c_add_driver(&aw22xxx_i2c_driver);
	if (ret)
		printk(KERN_ERR "fail to add aw22xxx device into i2c\n");

	return ret;
}
late_initcall(aw22xxx_i2c_init);

static void __exit aw22xxx_i2c_exit(void)
{
	i2c_del_driver(&aw22xxx_i2c_driver);
}

module_exit(aw22xxx_i2c_exit);
