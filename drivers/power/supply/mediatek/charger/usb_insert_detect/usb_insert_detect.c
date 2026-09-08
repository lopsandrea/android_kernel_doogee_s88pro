// SPDX-License-Identifier: GPL-2.0
/*
 * `usb_insert_detect` -- il rilevamento dell'inserimento USB del Doogee S88
 * Pro, ricostruito leggendo il kernel di fabbrica disassemblato.
 *
 * L'unita' di traduzione ha OTTO funzioni, in corsa contigua subito dopo la
 * fine di `mt5725`:
 *
 *   ffffff8008ace894 t Mt5725_get_rxdetect                    <- ultima di mt5725
 *   ffffff8008ace8f0 T usb_insert_online                        20 byte
 *   ffffff8008ace904 t usb_insert_detect_probe                 596
 *   ffffff8008aceb58 t usb_insert_detect_remove                  8
 *   ffffff8008aceb60 t usb_insert_detect_thread_kthread        416
 *   ffffff8008aced00 t usb_insert_detect_eint_interrupt_handler 80
 *   ffffff8008aced50 t usb_insert_detect_show_debug             48
 *   ffffff8008aced80 t usb_insert_detect_store_debug           124
 *   ffffff8009385a80 t usb_insert_detect_init                   52  (.init.text)
 *
 * PERCHE' E' STATO SCRITTO PER INTERO. Fino al 23 agosto 2026 di questo file
 * esisteva la sola `usb_insert_online`, aggiunta perche' `mt5725` la chiama e
 * senza di essa quel driver non si linka. Il confronto delle funzioni di
 * inizializzazione fra l'immagine di fabbrica e la nostra
 * (`tools/confrontainitcall.py`) ha poi detto che di tutte le 2.437 `_init`
 * della fabbrica ne mancavano DUE, e una era `usb_insert_detect_init`: senza,
 * il driver non si registra, `usb_insert_stato` non lo scrive nessuno, e
 * `mt5725_reverse_charge` prende sempre lo stesso ramo. Era l'unico buco
 * FUNZIONALE dell'intera superficie di avvio.
 *
 * COSA FA. Un GPIO segnala l'inserimento del cavo. L'interruzione sveglia un
 * kthread, che legge il GPIO, gira il tipo di interruzione per il fronte
 * opposto, aggiorna `usb_insert_stato` e -- se la ricarica inversa wireless e'
 * attiva (`mt5725_rvs_online`) -- riconfigura il caricatore: uscendo dalla
 * modalita' OTG quando il cavo entra, rientrandoci quando esce.
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/wait.h>

#include <mt-plat/charger_class.h>

/* Di `mt5725`, e solo LETTA da qui: 0xffffff800a110fec.
 * "b94feea8 ldr"@0xffffff8008acec60 e "b94feea8 ldr"@0xffffff8008acecb4. */
extern int mt5725_rvs_online;

extern struct charger_device *get_charger_by_name(const char *name);

/* DELTA DI HEADER, non fatto qui per la regola sui file condivisi: nessun
 * header di ALPS dichiara `mt_charger_set_opa_mode`, che sta in
 * drivers/misc/mediatek/pmic/mt6370/ (vedi
 * patches/kernel-stock/mt6370-charger-set-opa-mode.patch). Come fa gia'
 * `mt5725.c`, la si dichiara LOCALMENTE. */
extern int mt_charger_set_opa_mode(struct charger_device *chg_dev, bool en);

/*
 * 0xffffff800a110ff4, `int`. L'unico stato che `usb_insert_online` legge:
 * "d000b208 adrp"@0xffffff8008ace8f0 sulla pagina 0xffffff800a110000 piu'
 * "b94ff508 ldr"@0xffffff8008ace8f4 con offset 4084 = 0xff4.
 *
 * NON E' `static`: lo scrive il kthread di questa unita'
 * ("b90ff794 str"@0xffffff8008acec68 e "b90ff79f str"@0xffffff8008acecbc) e
 * lo legge `mt5725_reverse_charge`. Il nome e' SCELTO: il binario non lo
 * nomina, e la mappa non ha simboli di dato.
 */
int usb_insert_stato;

/* Gli altri stati, tutti in `.bss` sulla pagina 0xffffff800a111000. Gli
 * offset sono MISURATI, i nomi sono scelti.
 *   +368 "f900b900 str"@0xffffff8008ace99c   il caricatore primario
 *   +376 "f900be80 str"@0xffffff8008ace9b8   il pinctrl
 *   +384 "f900c100 str"@0xffffff8008acea04   lo stato di pin
 *   +392 "f900c680 str"@0xffffff8008acea5c   il nodo del device tree
 *   +400 "b90192a0 str"@0xffffff8008acea7c   il gpio
 *   +404 "b9019500 str"@0xffffff8008aceacc   l'irq
 *   +408 "b901994b str"@0xffffff8008acead4   il flag di debug
 *   +412 "39067109 strb"@0xffffff8008aced30  la sveglia del kthread
 */
static struct charger_device *usb_insert_detect_chg_dev;
static struct pinctrl *usb_insert_detect_pinctrl;
static struct pinctrl_state *usb_insert_detect_pin_init;
static struct device_node *usb_insert_detect_node;
static int usb_insert_detect_gpio;
static int usb_insert_detect_irq;
static int usb_insert_detect_flag;

/*
 * LA SVEGLIA E' UN BYTE, non un `int`, e il binario lo dice due volte:
 * "394672e8 ldrb"@0xffffff8008acebb0 la legge a un byte e
 * "39067109 strb"@0xffffff8008aced30 la scrive a un byte. Il test e'
 * "37000388 tbnz w8, #0", cioe' sul solo bit 0: e' un `bool`.
 */
static bool usb_insert_detect_thread_flag;

/* 0xffffff80099a77d8, scritto a zero dalla probe
 * ("b907d93f str"@0xffffff8008acead0) e dal kthread col valore del gpio
 * ("b907db20 str"@0xffffff8008acec3c). */
static int usb_insert_detect_gpio_state;

static DECLARE_WAIT_QUEUE_HEAD(usb_insert_detect_waiter);

/*
 * ------------------------------------------------------------------
 * usb_insert_online @0xffffff8008ace8f0, 20 byte
 * ------------------------------------------------------------------
 * Cinque istruzioni, nessuna chiamata, nessun frame.
 *
 *   ffffff8008ace8f0:	d000b208 	adrp	x8, ffffff800a110000
 *   ffffff8008ace8f4:	b94ff508 	ldr	w8, [x8,#4084]
 *   ffffff8008ace8f8:	7100011f 	cmp	w8, #0x0
 *   ffffff8008ace8fc:	1a9f07e0 	cset	w0, ne
 *   ffffff8008ace900:	d65f03c0 	ret
 *
 * IL RITORNO E' 0 O 1, NON IL VALORE. "1a9f07e0 cset"@0xffffff8008ace8fc
 * normalizza: qualunque valore diverso da zero diventa uno. Restituire
 * direttamente `usb_insert_stato` darebbe un `ldr`+`ret` senza `cset`, ed e'
 * la differenza che la dimensione denuncia.
 *
 * IL TIPO DI RITORNO NON E' MISURATO. Il `cset w0` scrive 32 bit e non dice
 * se il sorgente dica `int` o `bool`: e' una SCELTA, presa per `int` perche'
 * il simbolo e' `T` e un chiamante fuori dall'unita' -- `mt5725` -- lo usa.
 */
int usb_insert_online(void)
{
	return usb_insert_stato != 0;
}

/*
 * usb_insert_detect_show_debug @0xffffff8008aced50, 48 byte.
 *   "321403e1 orr"@0xffffff8008aced6c   w1 = 0x1000 = PAGE_SIZE
 *   "b9419903 ldr"@0xffffff8008aced5c   il flag, a +408
 *   "93407c00 sxtw"@0xffffff8008aced74  il ritorno e' esteso con segno a 64
 *                                        bit: e' una `ssize_t`
 */
static ssize_t usb_insert_detect_show_debug(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", usb_insert_detect_flag);
}

/*
 * usb_insert_detect_store_debug @0xffffff8008aced80, 124 byte.
 *
 * IL RAMO DI STAMPA E' UN `pr_debug`, E LO PROVA LA GUARDIA:
 * "39730908 ldrb"@0xffffff8008acedb0 legge un byte di un descrittore
 * `_ddebug` e "361001c8 tbz w8, #2" salta se il bit 2 e' spento -- e' la
 * forma di `CONFIG_DYNAMIC_DEBUG`, e la chiamata e' a
 * <__dynamic_pr_debug> ("97e6322b bl"@0xffffff8008acedd0), non a <printk>.
 *
 * IL RITORNO E' `count` E NON QUELLO DI `sscanf`:
 * "aa0303f3 mov"@0xffffff8008aceda0 salva x3 in x19 PRIMA della chiamata, e
 * "aa1303e0 mov"@0xffffff8008acedf0 lo rimette in x0 alla fine.
 */
static ssize_t usb_insert_detect_store_debug(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t count)
{
	if (sscanf(buf, "%d", &usb_insert_detect_flag) == 0)
		printk("%s param get failed\n", "usb_insert_detect_store_debug");
	else
		pr_debug("usb_insert_detect_flag  = %d\n", usb_insert_detect_flag);

	return count;
}

static DEVICE_ATTR(usb_insert_detect_debug, 0664, usb_insert_detect_show_debug,
		   usb_insert_detect_store_debug);

/*
 * usb_insert_detect_eint_interrupt_handler @0xffffff8008aced00, 80 byte.
 *
 * L'ORDINE E' MISURATO, e non e' quello che verrebbe naturale: la sveglia si
 * alza PRIMA della `__wake_up` ("39067109 strb"@0xffffff8008aced30 precede
 * "97d93993 bl"@0xffffff8008aced34) e l'interruzione si disabilita DOPO
 * ("97d9af7f bl"@0xffffff8008aced40). E' il kthread a riabilitarla.
 *
 * "320003e0 orr"@0xffffff8008aced44 rende 1 = IRQ_HANDLED.
 */
static irqreturn_t usb_insert_detect_eint_interrupt_handler(int irq, void *dev_id)
{
	printk("[USB_INSERT_DETECT] eint interrupt\n");

	usb_insert_detect_thread_flag = true;
	wake_up(&usb_insert_detect_waiter);
	disable_irq_nosync(usb_insert_detect_irq);

	return IRQ_HANDLED;
}

/*
 * usb_insert_detect_thread_kthread @0xffffff8008aceb60, 416 byte.
 *
 * IL CICLO NON FINISCE MAI: l'ultima istruzione dei due rami e'
 * "17ffffc5 b"@0xffffff8008aceca8 e "17ffffb0 b"@0xffffff8008acecfc, tutte e
 * due all'indietro verso la stampa condivisa a 0xffffff8008acebbc. Non c'e'
 * nessun `kthread_should_stop`, e non c'e' `usb_insert_detect_remove` che lo
 * fermi: e' un difetto di fabbrica, ed e' riprodotto (regola 7).
 *
 * LA CODA DI STAMPA E' CONDIVISA fra i due rami, e per questo la `printk`
 * finale e la `enable_irq` stanno FUORI dall'`if`.
 *
 * I DUE VALORI DI CORRENTE sono misurati:
 *   "529c6c01 mov"@0xffffff8008acec80 + "72a002c1 movk" -> 0x16e360 = 1500000
 *   "52923801 mov"@0xffffff8008acece8 + "72a00421 movk" -> 0x2191c0 = 2200000
 *
 * E I DUE TIPI DI INTERRUZIONE si girano a ogni passaggio:
 *   "321d03e1 orr"@0xffffff8008acec58   w1 = 8 = IRQ_TYPE_LEVEL_LOW
 *   "321e03e1 orr"@0xffffff8008acecac   w1 = 4 = IRQ_TYPE_LEVEL_HIGH
 * cioe' il livello atteso e' sempre l'opposto di quello appena letto.
 */
static int usb_insert_detect_thread_kthread(void *x)
{
	while (1) {
		wait_event_interruptible(usb_insert_detect_waiter,
					 usb_insert_detect_thread_flag);
		usb_insert_detect_thread_flag = false;

		usb_insert_detect_gpio_state =
			gpio_get_value(usb_insert_detect_gpio);
		printk("[USB_INSERT_DETECT] gpio value=%d\n",
		       usb_insert_detect_gpio_state);

		if (usb_insert_detect_gpio_state == 1) {
			irq_set_irq_type(usb_insert_detect_irq,
					 IRQ_TYPE_LEVEL_LOW);
			usb_insert_detect_flag = 0;
			usb_insert_stato = 1;
			if (mt5725_rvs_online) {
				mt_charger_set_opa_mode(usb_insert_detect_chg_dev, 0);
				charger_dev_set_boost_current_limit(usb_insert_detect_chg_dev,
								    1500000);
				msleep(40);
				charger_dev_enable(usb_insert_detect_chg_dev, 1);
			}
			printk("[USB_INSERT_DETECT] usb_insert_detect_gpio_state = 1\n");
		} else {
			irq_set_irq_type(usb_insert_detect_irq,
					 IRQ_TYPE_LEVEL_HIGH);
			usb_insert_detect_flag = 1;
			usb_insert_stato = 0;
			if (mt5725_rvs_online) {
				charger_dev_enable(usb_insert_detect_chg_dev, 0);
				msleep(40);
				mt_charger_set_opa_mode(usb_insert_detect_chg_dev, 1);
				charger_dev_set_boost_current_limit(usb_insert_detect_chg_dev,
								    2200000);
			}
			printk("[USB_INSERT_DETECT] uninsert usb_insert_detect_gpio_state = 0\n");
		}

		enable_irq(usb_insert_detect_irq);
	}

	return 0;
}

/*
 * usb_insert_detect_probe @0xffffff8008ace904, 596 byte.
 *
 * IL FALLIMENTO DI `device_create_file` NON FERMA LA PROBE: dopo
 * "34000080 cbz"@0xffffff8008ace928 i due rami si riuniscono sulla stessa
 * "97d992e5 bl"@0xffffff8008ace940 verso <printk> e si prosegue. E' un
 * difetto di fabbrica, riprodotto.
 *
 * "add x19, x0, #0x10"@0xffffff8008ace914 e' `&pdev->dev`.
 */
static int usb_insert_detect_probe(struct platform_device *pdev)
{
	struct task_struct *thread;
	struct pinctrl_state *pin_init;
	struct pinctrl *pinctrl;
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_usb_insert_detect_debug);
	if (ret)
		printk("[usb_insert_detect]error creating sysfs files: usb_insert_detect_debug\n");
	else
		printk("[USB_INSERT_DETECT] dev attr usb_insert_detect_debug register success\n");

	thread = kthread_create(usb_insert_detect_thread_kthread, NULL,
				"usb_insert_detect_thread_kthread");
	if (IS_ERR(thread)) {
		printk("[USB_INSERT_DETECT] kthread create failed\n");
		return -1;
	}
	wake_up_process(thread);
	printk("[USB_INSERT_DETECT] kthread create success\n");

	usb_insert_detect_chg_dev = get_charger_by_name("primary_chg");
	if (!usb_insert_detect_chg_dev) {
		printk(KERN_NOTICE "%s: get primary charger device failed\n",
		       "usb_insert_detect_probe");
		return -1;
	}

	/*
	 * I DUE PUNTATORI VIVONO IN UN LOCALE, non solo nel globale, e il
	 * binario lo dice: "aa0003f3 mov"@0xffffff8008ace9ac tiene in x19 il
	 * valore appena reso da <devm_pinctrl_get> mentre
	 * "f900be80 str"@0xffffff8008ace9b8 lo scrive anche nel globale, e il
	 * ritorno del ramo d'errore e' "2a1303e0 mov w0, w19", cioe' il LOCALE.
	 * Leggendo `PTR_ERR(globale)` dopo la `printk` clang deve rileggere la
	 * memoria -- la chiamata potrebbe averla cambiata -- e sono quattro byte
	 * di troppo.
	 *
	 * `pinctrl_select_state` invece RILEGGE il globale
	 * ("f940be80 ldr"@0xffffff8008acea2c), e qui si fa lo stesso.
	 */
	pinctrl = devm_pinctrl_get(&pdev->dev);
	usb_insert_detect_pinctrl = pinctrl;
	if (IS_ERR(pinctrl)) {
		printk("[USB_INSERT_DETECT] Cannot find usb_insert_detect pinctrl!\n");
		return PTR_ERR(pinctrl);
	}

	pin_init = pinctrl_lookup_state(pinctrl,
					"state_usb_insert_detect_gpio_init");
	usb_insert_detect_pin_init = pin_init;
	if (IS_ERR(pin_init)) {
		printk("[USB_INSERT_DETECT] Cannot find usb_insert_detect pin_init!\n");
		return PTR_ERR(pin_init);
	}

	pinctrl_select_state(usb_insert_detect_pinctrl, pin_init);
	printk("[USB_INSERT_DETECT] set usb_insert_detect pin_init success!\n");

	usb_insert_detect_node =
		of_find_compatible_node(NULL, NULL, "mediatek,usb_insert_detect");
	if (!usb_insert_detect_node) {
		printk("[USB_INSERT_DETECT] find usb_insert_detect node failed\n");
		return 0;
	}

	usb_insert_detect_gpio =
		of_get_named_gpio(usb_insert_detect_node, "chip_en", 0);
	if (usb_insert_detect_gpio < 0) {
		printk(KERN_ERR "%s get usb_insert_detect_eint_pin failed!\n",
		       "usb_insert_detect_probe");
		return usb_insert_detect_gpio;
	}

	ret = gpio_request(usb_insert_detect_gpio, "usb_insert_detect_eint_pin");
	if (ret < 0) {
		printk(KERN_ERR "%s gpio_request failed, gpio=%d\n",
		       "usb_insert_detect_probe", usb_insert_detect_gpio);
		return ret;
	}

	usb_insert_detect_irq = irq_of_parse_and_map(usb_insert_detect_node, 0);
	usb_insert_detect_gpio_state = 0;
	usb_insert_detect_flag = 1;

	ret = request_irq(usb_insert_detect_irq,
			  usb_insert_detect_eint_interrupt_handler,
			  IRQF_TRIGGER_HIGH, "usb_insert_detect", NULL);
	printk("[USB_INSERT_DETECT] request_irq IRQF_TRIGGER_HIGH\n");
	if (ret) {
		printk("[USB_INSERT_DETECT] usb_insert_detect set irq failed!!\n");
		return ret;
	}

	printk("[USB_INSERT_DETECT] usb_insert_detect set irq success!!\n");
	return ret;
}

/*
 * usb_insert_detect_remove @0xffffff8008aceb58, 8 byte: due istruzioni,
 * "2a1f03e0 mov w0, wzr" e "d65f03c0 ret". Non ferma il kthread, non libera
 * il gpio, non toglie l'attributo sysfs -- e' un difetto di fabbrica, ed e'
 * riprodotto.
 */
static int usb_insert_detect_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id usb_insert_detect_of_match[] = {
	{ .compatible = "mediatek,usb_insert_detect" },
	{},
};

static struct platform_driver usb_insert_detect_driver = {
	.probe = usb_insert_detect_probe,
	.remove = usb_insert_detect_remove,
	.driver = {
		.name = "usb_insert_detect",
		.of_match_table = usb_insert_detect_of_match,
	},
};

/*
 * usb_insert_detect_init @0xffffff8009385a80, 52 byte, in `.init.text`.
 *
 * "12800240 mov"@0xffffff8009385aa8 rende -19 = -ENODEV.
 *
 * NON C'E' UN `module_exit`: la mappa non ha nessun `usb_insert_detect_exit`,
 * e nemmeno un `__exitcall`. Il driver si registra e non si toglie.
 */
static int __init usb_insert_detect_init(void)
{
	if (platform_driver_register(&usb_insert_detect_driver)) {
		printk("[USB_INSERT_DETECT] failed to register driver");
		return -ENODEV;
	}

	return 0;
}

module_init(usb_insert_detect_init);

MODULE_DESCRIPTION("Doogee S88 Pro USB insert detect");
MODULE_LICENSE("GPL");
