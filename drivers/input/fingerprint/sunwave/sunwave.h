// SPDX-License-Identifier: GPL-2.0
/*
 * L'interfaccia fra le due unita' di traduzione del driver Sunwave.
 *
 * Contiene cio' che sf_ctl.c e sf_hw.c devono condividere, e nient'altro:
 * la struttura dati privata (che sf_ctl.c istanzia e sf_hw.c riempie di
 * puntatori a funzione), le due funzioni globali della parte hardware, e la
 * variabile che sf_probe scrive e sf_ctl_driver_init legge.
 *
 * Che l'interfaccia sia esattamente questa non e' una scelta: le uniche due
 * funzioni con linkage globale nel blocco di fabbrica sono sf_platform_init
 * e sf_platform_exit (`T` in oracolo/stock.map, contro `t` per le altre
 * sedici).
 */
#ifndef __SUNWAVE_H
#define __SUNWAVE_H

#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/notifier.h>
#include <linux/pm_wakeup.h>
#include <linux/spi/spi.h>
#include <linux/types.h>
#include <linux/workqueue.h>

/*
 * Struttura dati privata del driver. Tutti gli spiazzamenti vengono da
 * accessi letti nel disassemblato dell'oracolo: l'istanza statica sta a
 * 0xffffff800998dbf0 e le funzioni la indirizzano con adrp+add, quindi ogni
 * "0x998dcNN" della colonna commenti e' 0x998dbf0 + spiazzamento.
 *
 * Resta un buco non identificato (spiazzamento 80): nessuna delle 18
 * funzioni lo tocca. E' tenuto come riserva esplicita invece di essere
 * riempito con un campo plausibile.
 *
 * QUANTO E' GRANDE DAVVERO (rilievo R4 della revisione del lotto 2): 416 e'
 * il sizeof del NOSTRO oggetto, misurato con objdump -t sul nostro .o, non
 * una misura di fabbrica. Dal binario di fabbrica lo spiazzamento massimo
 * toccato e' 408 (`spi`, 8 byte) e il primo oggetto identificabile dopo
 * sf_ctl_dev e' l'attribute_group a 0xffffff800998dde0 = base+496: la taglia
 * di fabbrica e' quindi compresa fra 416 e 496, e gli 80 byte di coda sono
 * un intervallo di cui il binario non dice niente. Cio' che e' verificato
 * sono i QUATTORDICI SPIAZZAMENTI, non la taglia.
 */
struct sf_ctl_device {
	/*
	 * Spiazzamento 0. sf_remove: "aa1303e0  mov x0, x19" con x19 alla
	 * base, subito prima di bl <misc_deregister> a 0xffffff8008a8315c.
	 * this_device di questa miscdevice sta a 48, che e' lo spiazzamento
	 * letto da sf_ctl_device_event ("f9461108  ldr x8, [x8,#3104]" ->
	 * 0x998dc20 = base+48) -- coincide con l'ABI di struct miscdevice di
	 * questo sublevel, che e' la conferma incrociata del fatto che il
	 * primo campo e' proprio una miscdevice.
	 */
	struct miscdevice miscdev;
	/* Spiazzamento 80: non identificato, nessuna delle 18 funzioni lo tocca. */
	unsigned char __resv0[8];
	/*
	 * Spiazzamento 88 (0x58). sf_remove:
	 * "b94c4900  ldr w0, [x8,#3144]" (0x998dc48 = base+88), poi
	 * "b9005a7f  str wzr, [x19,#88]".
	 */
	int irq;
	/*
	 * Spiazzamento 96 (0x60). sf_probe inizializza qui una work_struct:
	 * "f9001a8a  str x10, [x20,#48]" con x10 = 0xfffffffe0
	 * (WORK_STRUCT_NO_POOL), "f9001e8b  str x11, [x20,#56]" e
	 * "a904328b  stp x11, x12, [x20,#64]" con x11 = &work.entry (lista
	 * che punta a se stessa) e x12 = 0xffffff8008a8319c =
	 * sf_ctl_device_event -- cioe' INIT_WORK(&ctl->work,
	 * sf_ctl_device_event), con x20 = 0x998dc20 = base+48.
	 */
	struct work_struct work;
	/*
	 * Spiazzamento 128 (0x80). sf_remove: "f9400260  ldr x0, [x19]" con
	 * x19 = 0x998dc70 = base+128, subito prima di
	 * "bl <input_unregister_device>".
	 */
	struct input_dev *input;
	/*
	 * Spiazzamento 136 (0x88). sf_ctl_device_power:
	 * "f9404500  ldr x0, [x8,#136]".
	 */
	struct regulator *vdd_reg;
	/* Spiazzamento 144 (0x90). sf_platform_init: "a9093288  stp x8, x12, [x20,#144]". */
	int (*init_gpio_pins)(struct sf_ctl_device *ctl);
	/* Spiazzamento 152 (0x98). sf_platform_init, stessa stp di cui sopra (x12). */
	int (*free_gpio)(struct sf_ctl_device *ctl);
	/*
	 * Spiazzamento 160 (0xa0). sf_platform_init: "a90a2a89  stp x9, x10, [x20,#160]".
	 * Il parametro e' bool, non int -- vedi il commento lungo sopra
	 * sf_ctl_device_power.
	 */
	int (*power)(bool on);
	/*
	 * Spiazzamento 168 (0xa8). sf_platform_init, stessa stp di cui sopra (x10).
	 * Anche qui il parametro e' bool: sf_spi_clock_enable testa "on" con
	 * 360000e0 tbz w0,#0 a 0xffffff8008a83edc, un solo bit. Questo
	 * puntatore E' invocato: sf_ctl_ioctl fa
	 * "f9464d08  ldr x8, [x8,#3224]" + "320003e0 orr w0, wzr, #0x1" +
	 * "d63f0100 blr x8" (e la variante con w0 = 0), e sf_probe lo invoca
	 * da sf_read_sensor_id.
	 */
	int (*spi_clock_enable)(bool on);
	/*
	 * Spiazzamento 176 (0xb0). sf_platform_init: "f9005a8b  str x11, [x20,#176]".
	 * Invocato da sf_ctl_ioctl: "f9465108  ldr x8, [x8,#3232]" +
	 * "d63f0100 blr x8", senza argomenti.
	 */
	int (*reset)(void);
	/*
	 * Spiazzamento 184 (0xb8). sf_probe: "9102e2b4  add x20, x21, #0xb8"
	 * poi wakeup_source_prepare/wakeup_source_add; sf_remove:
	 * "9102e274  add x20, x19, #0xb8" poi wakeup_source_remove/_drop.
	 * Lo spiazzamento del campo successivo (368) meno 184 fa 184, che e'
	 * esattamente sizeof(struct wakeup_source) su questo sublevel: e' la
	 * conferma che fra i due campi non c'e' altro.
	 */
	struct wakeup_source ws;
	/*
	 * Spiazzamento 368 (0x170). sf_probe: "91046260  add x0, x19, #0x118"
	 * con x19 = 0x998dc48 = base+88, cioe' base+368, passato a
	 * fb_register_client; e "f9008e68  str x8, [x19,#280]" (stesso
	 * indirizzo) con x8 = 0xffffff8008a83218 = sf_fb_notifier_callback,
	 * cioe' notifier_call. sf_remove: "9103c260  add x0, x19, #0xf0" con
	 * x19 = base+128, di nuovo base+368, passato a fb_unregister_client.
	 */
	struct notifier_block notifier;
	/* Spiazzamento 392: non identificato, nessuna delle 18 lo tocca. */
	unsigned char __resv1[8];
	/*
	 * Spiazzamento 400 (0x190). Un int, non un puntatore: sf_ctl_ioctl lo
	 * legge con "b94d818b  ldr w11, [x12,#3456]" (0x998dd80 = base+400,
	 * `ldr w` = 32 bit) e lo consegna all'utente con la sequenza
	 * bics/csel + "b900014b  str w11, [x10]" che e' put_user su 4 byte.
	 * Il NOME non e' leggibile dal binario: nessun letterale lo nomina.
	 * Nessuna delle 18 funzioni lo scrive mai.
	 */
	int field_400;
	int __pad_404;
	/*
	 * Spiazzamento 408 (0x198). sf_probe: "f900ceb4  str x20, [x21,#408]"
	 * con x20 = lo spi_device passato a probe; sf_ctl_device_free_gpio:
	 * "f940ce68  ldr x8, [x19,#408]".
	 */
	struct spi_device *spi;
};

int sf_platform_init(struct sf_ctl_device *ctl);
void sf_platform_exit(struct sf_ctl_device *ctl);

/*
 * Un int a 0xffffff800a100ce0, scritto in un solo posto e letto in un solo
 * posto:
 *   sf_probe          "320003e9  orr w9, wzr, #0x1" +
 *                     "b90ce109  str w9, [x8,#3296]"   (0xa100ce0)
 *   sf_ctl_driver_init "b94ce108  ldr w8, [x8,#3296]" +
 *                     "7100051f  cmp w8, #0x1" +
 *                     "54000061  b.ne ...+0x44"  -- se vale 1, w19 = 0 e la
 *                     funzione salta ogni registrazione e ritorna 0.
 * Il nome non e' leggibile dal binario; il ruolo si', ed e' quello.
 */
extern int g_probe_done;

#endif /* __SUNWAVE_H */
