/* drivers/input/touchscreen/gt1x_tpd.c
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

#include "include/gt1x_tpd_common.h"
#if TPD_SUPPORT_I2C_DMA
#include <linux/dma-mapping.h>
#endif

#ifdef CONFIG_GTP_ICS_SLOT_REPORT
#include <linux/input/mt.h>
#endif

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/regulator/consumer.h>
#include <uapi/linux/sched/types.h>

#include <linux/suspend.h>

/* gt1x_suspend/gt1x_resume stanno in gt1x_generic.c (0xffffff8008a74cdc e
 * 0xffffff8008a74fb8) e include/gt1x_tpd_common.h non le dichiara.  Il
 * prototipo sta QUI e non nell'header perche' l'header e' condiviso fra le sei
 * unita' e questo lotto non lo tocca: la voce e' riportata come delta di
 * header al lotto di merge.  Nessun effetto sul binario -- e' una
 * dichiarazione.
 */
extern s32 gt1x_suspend(void);
extern s32 gt1x_resume(void);

/* g0fc0b8 -- lo stesso byte a 0xffffff800a0fc0b8 che gt1x_generic.c dichiara
 * e documenta (vedi il suo cappello): il binario non lo nomina e il nome
 * viene dall'indirizzo.  Qui serve perche' tpd_event_handler lo legge --
 * "3942e108 ldrb"@0xffffff8008a78028 -- con la stessa forma delle altre
 * cinque letture.  Anche questa dichiarazione e' un delta di header.
 */
extern u8 g0fc0b8;

/* fix_tp_proc_info -- di fabbrica sta a 0xffffff8008a5151c, in un oggetto
 * che NON e' questo driver (il blocco Goodix comincia a 0xffffff8008a7221c);
 * il cappello di gt1x_generic.c la nomina gia' parlando del buffer che essa
 * riempie con __memcpy.  Non e' in ALPS: ne' i .c ne' i .h di
 * drivers/input/touchscreen/mediatek/ la contengono, quindi resta un simbolo
 * IRRISOLTO, dichiarato e non definito (regola 6).
 * La firma e' letta dal sito di chiamata: x0 = il buffer della sprintf,
 * w1 = il valore di ritorno della sprintf,
 *   "2a0003e1 mov"@0xffffff8008a77a5c   w1 = w0 (la lunghezza)
 *   "910023e0 add"@0xffffff8008a77a60   x0 = sp+8 (il buffer)
 *   "97ff66ae bl"@0xffffff8008a77a64
 * Il TIPO DI RITORNO non e' misurabile -- nessuno lo guarda -- e `void` e'
 * una scelta.
 */
extern void fix_tp_proc_info(char *buf, int len);

/*1 enable,0 disable,touch_panel_eint default status, */
/* need to confirm after register eint*/
/* `static` E SENZA INIZIALIZZATORE, tutte e due misurate.  Di fabbrica
 * irq_flag e' letta a un byte e provata a bit --
 *   "3969c109 ldrb"@0xffffff8008a76e08 / "370000c9 tbnz"@0xffffff8008a76e10
 *   "3929c109 strb"@0xffffff8008a76e20 / "3929c11f strb"@0xffffff8008a76e78
 * -- che e' la forma ridotta che clang usa solo quando la variabile e'
 * `static` e in tutta l'unita' assume due soli valori.  ALPS la dichiara
 * globale (`int irq_flag = 1;`).  E il valore iniziale e' ZERO, non uno:
 * l'indirizzo 0xffffff800a100a70 cade in .bss, che parte a
 * 0xffffff8009a5b80c, mentre un inizializzatore diverso da zero la
 * porterebbe in .data.  Con `static int irq_flag = 1;` gt1x_irq_enable
 * misura ancora 88 byte ma la coppia ldrb/tbnz si rovescia.
 */
static int irq_flag;
static spinlock_t irq_flag_lock;
/*0 power off,default, 1 power on*/
static int power_flag;
static int tpd_flag;
/* TOLTE, e sono cinque scelte con la stessa prova delle sei funzioni: gli
 * unici usi che ALPS ne fa stanno dentro tpd_enter_tui, tpd_exit_tui,
 * gt1x_pm_notifier, tpd_suspend e tpd_resume, e nessuno di quei corpi e'
 * nel binario di fabbrica.
 *   static int tpd_pm_flag;   static int tpd_tui_flag;
 *   static int tpd_tui_low_power_skipped;   static bool gtp_suspend;
 *   static DECLARE_WAIT_QUEUE_HEAD(pm_waiter);   DEFINE_MUTEX(tui_lock);
 * Anche `update_thread` e `pm_notifier_block`, che stavano sotto
 * CONFIG_GTP_AUTO_UPDATE: quella CONFIG e' accesa nel nostro albero e le due
 * variabili restavano senza nessun uso.  Nel binario di fabbrica non c'e'
 * nessun kthread di auto-update in tpd_registration -- un solo
 * kthread_create_on_node in tutta la funzione,
 * "97d983ed bl"@0xffffff8008a77b50 -- e nessun register_pm_notifier.
 */
int tpd_halt;
static int tpd_eint_mode = 1;
static struct task_struct *thread;
static struct task_struct *probe_thread;
static int tpd_polling_time = 50;
static DECLARE_WAIT_QUEUE_HEAD(waiter);
DECLARE_WAIT_QUEUE_HEAD(init_waiter);
DEFINE_MUTEX(i2c_access);
/* touch_irq: di fabbrica sta a 0xffffff800a100a74, cioe' IN MEZZO ai simboli
 * locali dell'oggetto (irq_flag_lock 0xa6c, irq_flag 0xa70, power_flag 0xa78,
 * tpd_eint_mode 0xa7c, tpd_flag 0xa80), mentre clang mette i globali PRIMA
 * dei locali -- nel nostro oggetto touch_irq, tpd_halt e int_type stanno a
 * 0, 4, 8 e i locali cominciano a 0xc.  E' un indizio che di fabbrica sia
 * `static`, ma NON cambia un byte di codice (le letture restano `ldr w` in
 * tutti e due i casi, e l'offset e' un sito di rilocazione): riportato, non
 * applicato.
 */
unsigned int touch_irq;
u8 int_type;

#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT] = TPD_WARP_END;
#endif

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

/* LE DUE TABELLE DEI TASTI, LETTE DAL BINARIO.  ALPS passa a
 * tpd_button_setting i campi di tpd_dts_data (tpd_key_num, tpd_key_local,
 * tpd_key_dim_local); di fabbrica gli argomenti sono una costante e due
 * indirizzi nella sezione dati del driver stesso --
 *   "320007e0 orr"@0xffffff8008a776b0    w0 = 3
 *   "911a6021 add"@0xffffff8008a776a8    x1 = 0xffffff800998a698
 *   "911a9042 add"@0xffffff8008a776ac    x2 = 0xffffff800998a6a4
 *   "97ff6b70 bl"@0xffffff8008a776b4     -> tpd_button_setting
 * e i byte a quei due indirizzi sono, letti dall'ELF:
 *   0xffffff800998a698  8b000000 ac000000 9e000000            -> 139 172 158
 *   0xffffff800998a6a4  3c000000 34080000 32000000 14000000
 *                       b4000000 34080000 32000000 14000000
 *                       2c010000 34080000 32000000 14000000
 * cioe' tre tasti e tre quaterne (x, y, larghezza, altezza).  I NOMI sono
 * miei: il binario da' i valori e la lunghezza, non come si chiamano.  139,
 * 172 e 158 sono KEY_MENU, KEY_HOMEPAGE e KEY_BACK, ma qui restano numeri
 * perche' e' quello che il binario contiene.
 */
/* LA TABELLA DEI MODULI E IL SUO BUFFER.  I NOMI VENGONO DALL'INDIRIZZO --
 * il binario non li nomina e stock.map contiene solo simboli di funzione --
 * e cio' che di loro e' misurato e' questo.
 *
 * g98a468 sta a 0xffffff800998a468, e' fatta di righe da QUARANTA byte
 *   ("52800508 mov"@0xffffff8008a77990 mette 0x28 in w8 e
 *    "9b0824c1 madd"@0xffffff8008a77998 calcola x1 = sensor_id*40 + base)
 * ed e' lunga SEI righe: la variabile che la segue nella sezione dati e'
 * tpd_polling_time a 0xffffff800998a558, cioe' 0xf0 = 240 = 6*40 byte piu'
 * in la'.  Il contenuto, letto dall'ELF:
 *   "xinpengda"@0xffffff800998a468   "none_3"@0xffffff800998a4e0
 *   "none_1"@0xffffff800998a490      "none_4"@0xffffff800998a508
 *   "hongzhan"@0xffffff800998a4b8    "none_5"@0xffffff800998a530
 *
 * g100a39 sta a 0xffffff800a100a39, in .bss, ed e' il bersaglio delle due
 * strcpy: "9128e400 add"@0xffffff8008a779a4 e "9128e508 add"@0xffffff8008a779fc.
 * LA SUA LUNGHEZZA NON E' DETERMINATA dal binario, solo limitata: la
 * variabile che la segue e' irq_flag_lock a 0xffffff800a100a6c, che vuole
 * quattro byte di allineamento, quindi la lunghezza sta fra 48 e 51.  Ho
 * scelto 50, ed e' una SCELTA dichiarata.
 */
static char g98a468[6][40] = {
	"xinpengda", "none_1", "hongzhan", "none_3", "none_4", "none_5",
};
static char g100a39[50];

static int tpd_keys_local[3] = {139, 172, 158};
static int tpd_keys_dim_local[3][4] = {
	{60, 2100, 50, 20}, {180, 2100, 50, 20}, {300, 2100, 50, 20},
};

static int tpd_event_handler(void *unused);
static int tpd_i2c_probe(struct i2c_client *client,
			 const struct i2c_device_id *id);
static int tpd_i2c_detect(struct i2c_client *client,
			  struct i2c_board_info *info);
static int tpd_i2c_remove(struct i2c_client *client);

static irqreturn_t tpd_eint_interrupt_handler(unsigned int irq,
					      struct irq_desc *desc);

#define GTP_DRIVER_NAME "gt1x"
static const struct i2c_device_id tpd_i2c_id[] = {{GTP_DRIVER_NAME, 0}, {} };
static unsigned short force[] = {0, GTP_I2C_ADDRESS, I2C_CLIENT_END,
				 I2C_CLIENT_END};
static const unsigned short *const forces[] = {force, NULL};

/* "mediatek,cap_touch"@0xffffff8008f7ead8 -- DEBOLE: i byte occorrono TRE
 * volte nell'immagine (0xffffff8008f7ead8, 0xffffff8008f7f1c0,
 * 0xffffff8008f7f4a0), perche' `compatible` e' un campo dentro la struct e
 * non un puntatore, e ogni driver touch ne ha una copia.  Da sola la
 * citazione non prova quale delle tre sia questa.
 */
static const struct of_device_id tpd_of_match[] = {
	{.compatible = "mediatek,cap_touch"}, {},
};
static struct i2c_driver tpd_i2c_driver = {
	.probe = tpd_i2c_probe,
	.remove = tpd_i2c_remove,
	.detect = tpd_i2c_detect,
	.driver.name = GTP_DRIVER_NAME,
	.driver = {

			.name = GTP_DRIVER_NAME, .of_match_table = tpd_of_match,
		},
	.id_table = tpd_i2c_id,
	.address_list = (const unsigned short *)forces,
};

#if TPD_SUPPORT_I2C_DMA
static u8 *gpDMABuf_va;
static dma_addr_t gpDMABuf_pa;
struct mutex dma_mutex;
DEFINE_MUTEX(dma_mutex);

static s32 i2c_dma_write_mtk(u16 addr, u8 *buffer, s32 len)
{
	s32 ret = 0;
	s32 pos = 0;
	s32 transfer_length;
	u16 address = addr;

	struct i2c_msg msg = {
		.flags = !I2C_M_RD,
		.ext_flag = (gt1x_i2c_client->ext_flag | I2C_ENEXT_FLAG |
			     I2C_DMA_FLAG),
		.addr = (gt1x_i2c_client->addr & I2C_MASK_FLAG),
		.timing = I2C_MASTER_CLOCK,
		.buf = (u8 *)(uintptr_t)gpDMABuf_pa,
	};

	mutex_lock(&dma_mutex);
	while (pos != len) {
		if (len - pos > (IIC_DMA_MAX_TRANSFER_SIZE - GTP_ADDR_LENGTH))
			transfer_length =
				IIC_DMA_MAX_TRANSFER_SIZE - GTP_ADDR_LENGTH;
		else
			transfer_length = len - pos;

		gpDMABuf_va[0] = (address >> 8) & 0xFF;
		gpDMABuf_va[1] = address & 0xFF;
		memcpy(&gpDMABuf_va[GTP_ADDR_LENGTH], &buffer[pos],
		       transfer_length);

		msg.len = transfer_length + GTP_ADDR_LENGTH;
		if (!gtp_suspend) { /*workround log too much*/
			ret = i2c_transfer(gt1x_i2c_client->adapter, &msg, 1);
			if (ret != 1) {
				GTP_INFO("I2c Transfer error! (%d)", ret);
				ret = ERROR_IIC;
				break;
			}
		} else {
			ret = ERROR_IIC;
			break;
		}
		ret = 0;
		pos += transfer_length;
		address += transfer_length;
	}
	mutex_unlock(&dma_mutex);
	return ret;
}

static s32 i2c_dma_read_mtk(u16 addr, u8 *buffer, s32 len)
{
	s32 ret = ERROR;
	s32 pos = 0;
	s32 transfer_length;
	u16 address = addr;
	u8 addr_buf[GTP_ADDR_LENGTH] = {0};

	struct i2c_msg msgs[2] = {
		{
			.flags = 0, /*!I2C_M_RD,*/
			.addr = (gt1x_i2c_client->addr & I2C_MASK_FLAG),
			.timing = I2C_MASTER_CLOCK,
			.len = GTP_ADDR_LENGTH,
			.buf = addr_buf,
		},
		{
			.flags = I2C_M_RD,
			.ext_flag = (gt1x_i2c_client->ext_flag |
				     I2C_ENEXT_FLAG | I2C_DMA_FLAG),
			.addr = (gt1x_i2c_client->addr & I2C_MASK_FLAG),
			.timing = I2C_MASTER_CLOCK,
			.buf = (u8 *)(uintptr_t)gpDMABuf_pa,
		},
	};
	mutex_lock(&dma_mutex);
	while (pos != len) {
		if (len - pos > IIC_DMA_MAX_TRANSFER_SIZE)
			transfer_length = IIC_DMA_MAX_TRANSFER_SIZE;
		else
			transfer_length = len - pos;

		msgs[0].buf[0] = (address >> 8) & 0xFF;
		msgs[0].buf[1] = address & 0xFF;
		msgs[1].len = transfer_length;

		ret = i2c_transfer(gt1x_i2c_client->adapter, msgs, 2);
		if (ret != 2) {
			GTP_ERROR("I2C Transfer error! (%d)", ret);
			ret = ERROR_IIC;
			break;
		}
		ret = 0;
		memcpy(&buffer[pos], gpDMABuf_va, transfer_length);
		pos += transfer_length;
		address += transfer_length;
	};
	mutex_unlock(&dma_mutex);
	return ret;
}

#else

static s32 i2c_write_mtk(u16 addr, u8 *buffer, s32 len)
{
	s32 ret;

	struct i2c_msg msg = {
		.flags = 0,
#ifdef CONFIG_MTK_I2C_EXTENSION
		.addr = (gt1x_i2c_client->addr & I2C_MASK_FLAG) |
			(I2C_ENEXT_FLAG), /*remain*/
		.timing = I2C_MASTER_CLOCK,
#else
		.addr = gt1x_i2c_client->addr, /*remain*/
#endif
	};

	ret = _do_i2c_write(&msg, addr, buffer, len);
	return ret;
}

static s32 i2c_read_mtk(u16 addr, u8 *buffer, s32 len)
{
	int ret;
	u8 addr_buf[GTP_ADDR_LENGTH] = {(addr >> 8) & 0xFF, addr & 0xFF};

	struct i2c_msg msgs[2] = {
		{
#ifdef CONFIG_MTK_I2C_EXTENSION
			.addr = ((gt1x_i2c_client->addr & I2C_MASK_FLAG) |
				 (I2C_ENEXT_FLAG)),
			.timing = I2C_MASTER_CLOCK,
#else
			.addr = gt1x_i2c_client->addr,
#endif
			.flags = 0,
			.buf = addr_buf,
			.len = GTP_ADDR_LENGTH,
		},
		{
#ifdef CONFIG_MTK_I2C_EXTENSION
			.addr = ((gt1x_i2c_client->addr & I2C_MASK_FLAG) |
				 (I2C_ENEXT_FLAG)),
			.timing = I2C_MASTER_CLOCK,
#else
			.addr = gt1x_i2c_client->addr,
#endif
			.flags = I2C_M_RD,
		},
	};

	ret = _do_i2c_read(msgs, addr, buffer, len);
	return ret;
}
#endif /* TPD_SUPPORT_I2C_DMA */

/**
 * @return: return 0 if success, otherwise return a negative number
 *          which contains the error code.
 */
s32 gt1x_i2c_read(u16 addr, u8 *buffer, s32 len)
{
#if TPD_SUPPORT_I2C_DMA
	return i2c_dma_read_mtk(addr, buffer, len);
#else
	return i2c_read_mtk(addr, buffer, len);
#endif
}

/**
 * @return: return 0 if success, otherwise return a negative number
 *          which contains the error code.
 */
s32 gt1x_i2c_write(u16 addr, u8 *buffer, s32 len)
{
#if TPD_SUPPORT_I2C_DMA
	return i2c_dma_write_mtk(addr, buffer, len);
#else
	return i2c_write_mtk(addr, buffer, len);
#endif
}

#ifdef TPD_REFRESH_RATE
/*******************************************************
 * Function:
 *   Write refresh rate
 *
 * Input:
 *   rate: refresh rate N (Duration=5+N ms, N=0~15)
 *
 * Output:
 *   Executive outcomes.0---succeed.
 *******************************************************/
static u8 gt1x_set_refresh_rate(u8 rate)
{
	u8 buf[1] = {rate};

	if (rate > 0xf) {
		GTP_ERROR("Refresh rate is over range (%d)", rate);
		return ERROR_VALUE;
	}

	GTP_INFO("Refresh rate change to %d", rate);
	return gt1x_i2c_write(GTP_REG_REFRESH_RATE, buf, sizeof(buf));
}

/*******************************************************
 * Function:
 *    Get refresh rate
 *
 * Output:
 *    Refresh rate or error code
 *******************************************************/
static u8 gt1x_get_refresh_rate(void)
{
	int ret;
	u8 buf[1] = {0x00};

	ret = gt1x_i2c_read(GTP_REG_REFRESH_RATE, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	GTP_INFO("Refresh rate is %d", buf[0]);
	return buf[0];
}

/*=============================================================*/
static ssize_t show_refresh_rate(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int ret = gt1x_get_refresh_rate();

	if (ret < 0)
		return 0;
	else
		return sprintf(buf, "%d\n", ret);
}

static ssize_t store_refresh_rate(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{
	unsigned long rate;
	int ret;

	ret = kstrtoul(buf, 0, &rate);
	gt1x_set_refresh_rate(rate);
	return size;
}

static DEVICE_ATTR(tpd_refresh_rate, 0664, show_refresh_rate,
		   store_refresh_rate);

static struct device_attribute *gt9xx_attrs[] = {
	&dev_attr_tpd_refresh_rate,
};
#endif
/*=============================================================*/

/* Di fabbrica sono OTTO byte scritti in una volta sola, non una strncpy su
 * tutto il campo: la stringa "mtk-tpd" col suo NUL sta esattamente in una
 * parola da 64 bit, e clang la materializza con mov+3 movk.
 *   "d28e8da8 mov"@0xffffff8008a77acc    x8 = 0x746d       'm','t'
 *   "f2a5ad68 movk"@0xffffff8008a77ad0   |= 0x2d6b << 16   'k','-'
 *   "f2ce0e88 movk"@0xffffff8008a77ad4   |= 0x7074 << 32   't','p'
 *   "f2e00c88 movk"@0xffffff8008a77ad8   |= 0x0064 << 48   NUL,'d'
 *   "f9000028 str"@0xffffff8008a77ae0    [x1] = x8
 * `strncpy(info->type, "mtk-tpd", sizeof(info->type))` -- che e' quello che
 * ALPS scrive -- riempirebbe di zeri tutti e venti i byte di I2C_NAME_SIZE e
 * misura 40 byte invece di 28.
 */
static int tpd_i2c_detect(struct i2c_client *client,
			  struct i2c_board_info *info)
{
	strcpy(info->type, "mtk-tpd");
	return 0;
}

/* Incorporata in tpd_on, dove il ciclo di cinque giri e' srotolato per
 * intero: ogni giro ha la stessa sequenza di `bl`.  Il primo e' verso
 * gt1x_reset_guitar (0xffffff8008a741ac), NON verso gt1x_select_addr
 * (0xffffff8008a74158) come scrive ALPS, e il suo risultato non e' guardato;
 * la msleep(20) che ALPS mette dopo non c'e' affatto.
 *   "320003e0 orr"@0xffffff8008a77408    w0 = SWITCH_ON
 *   "97fffea4 bl"@0xffffff8008a7740c     -> gt1x_power_switch
 *   "97fff367 bl"@0xffffff8008a77410     -> gt1x_reset_guitar (scartato)
 *   "97fff438 bl"@0xffffff8008a77414     -> gt1x_get_chip_type
 *   "35000060 cbnz"@0xffffff8008a77418
 *   "97fff364 bl"@0xffffff8008a7741c     -> gt1x_reset_guitar
 *   "340000e0 cbz"@0xffffff8008a77420
 * e la stessa sequenza si ripete a 0xffffff8008a77454, 0xffffff8008a7749c,
 * 0xffffff8008a774e4 e 0xffffff8008a7752c.  Le due chiamate a
 * gt1x_reset_guitar sono davvero DUE: quattro istruzioni di distanza, con il
 * cbnz in mezzo.
 *
 * I tre messaggi di tpd_on, che la incorpora:
 * "tpd_on"@0xffffff800924bfee (il __func__)
 * "<<GTP-ERR>>[%s:%d] I2C Power on ERROR!\n"@0xffffff800924bfc6
 * "<<GTP-DBG>>[%s:%d]Wakeup sleep send gt1x_config success.\n"@0xffffff800924bff5
 * "<<GTP-ERR>>[%s:%d] GTP later resume failed.\n"@0xffffff800924c02f
 */
static int tpd_power_on(void)
{
	gt1x_power_switch(SWITCH_ON);

	gt1x_reset_guitar();

	if (gt1x_get_chip_type() != 0)
		return -1;

	if (gt1x_reset_guitar() != 0)
		return -1;

	return 0;
}

void gt1x_irq_enable(void)
{
	unsigned long flags;

	spin_lock_irqsave(&irq_flag_lock, flags);
	if (irq_flag == 0) {
		irq_flag = 1;
		enable_irq(touch_irq);
	}
	spin_unlock_irqrestore(&irq_flag_lock, flags);
}

void gt1x_irq_disable(void)
{
	unsigned long flags;

	spin_lock_irqsave(&irq_flag_lock, flags);
	if (irq_flag == 1) {
		irq_flag = 0;
		disable_irq(touch_irq);
	}
	spin_unlock_irqrestore(&irq_flag_lock, flags);
}

/* DI FABBRICA QUESTA FUNZIONE RESTITUISCE UN VALORE, e la dimensione lo dice
 * per quattro byte: l'epilogo azzera w0 prima di tornare,
 *   "2a1f03e0 mov"@0xffffff8008a76fac
 *   "d65f03c0 ret"@0xffffff8008a76fb4
 * mentre le altre `void` dello stesso blocco non lo fanno (gt1x_irq_enable
 * chiude con "a9417bfd ldp"@0xffffff8008a76e38 / "f84207f3 ldr" / "ret", e
 * gt1x_touch_down uguale).  Il prototipo dell'header e' stato cambiato di
 * conseguenza: e' l'unica riga di include/gt1x_tpd_common.h che questo lotto
 * tocca, ed e' dichiarata.
 *
 * E LA PAUSA E' DI DIECI MILLISECONDI, non venti:
 *   "52800140 mov"@0xffffff8008a76ec4   w0 = 0xa
 *   "97db6f53 bl"@0xffffff8008a76ec8    -> msleep (0xffffff8008152c14)
 *
 * `power_flag` e' letto a un byte solo -- "3969e268 ldrb"@0xffffff8008a76edc
 * e "3929e268 strb"@0xffffff8008a76f2c -- come irq_flag: e' `static` e vale
 * solo 0 o 1, e clang lo restringe.  Il ramo SWITCH_ON prova il bit
 * ("37000648 tbnz"@0xffffff8008a76ee0), quello SWITCH_OFF confronta con uno
 * ("7100051f cmp"@0xffffff8008a76f3c): la stessa coppia asimmetrica di
 * gt1x_irq_enable/gt1x_irq_disable, e viene dal compilatore, non dal
 * sorgente.  SWITCH_OFF vale 0 ("34000353 cbz"@0xffffff8008a76ecc) e
 * SWITCH_ON vale 1 ("7100067f cmp"@0xffffff8008a76ed0).
 *
 * Il regolatore e' tpd->reg, secondo campo della struttura:
 *   "f9416108 ldr"@0xffffff8008a76f00   x8 = tpd
 *   "f9400500 ldr"@0xffffff8008a76f04   x0 = [x8,#8]
 * I cinque messaggi, per intero e nell'ordine in cui il binario li mette:
 * "<<GTP-DBG>>[%s:%d]Power switch on!\n"@0xffffff800924bdc8
 * "gt1x_power_switch"@0xffffff800924bdec  (il __func__, x1 di tutte e cinque)
 * "<<GTP-ERR>>[%s:%d] regulator_enable() failed!\n\n"@0xffffff800924bdfe
 * "<<GTP-DBG>>[%s:%d]Power switch off!\n"@0xffffff800924be2e
 * "<<GTP-ERR>>[%s:%d] regulator_disable() failed!\n\n"@0xffffff800924be53
 * "<<GTP-ERR>>[%s:%d] Invalid power switch command!\n"@0xffffff800924be84
 */
s32 gt1x_power_switch(s32 state)
{
	int ret = 0;

	GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);
	GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
	msleep(10);

	switch (state) {
	case SWITCH_ON:
		if (power_flag == 0) {
#line 437
			GTP_DEBUG("Power switch on!");
			ret = regulator_enable(tpd->reg); /*enable regulator*/
			if (ret)
#line 441
				GTP_ERROR("regulator_enable() failed!\n");
			power_flag = 1;
		}
		break;
	case SWITCH_OFF:
		if (power_flag == 1) {
#line 448
			GTP_DEBUG("Power switch off!");
			ret = regulator_disable(tpd->reg);/*disable regulator*/
			if (ret)
#line 452
				GTP_ERROR("regulator_disable() failed!\n");
			power_flag = 0;
		}
		break;
	default:
#line 462
		GTP_ERROR("Invalid power switch command!");
		break;
	}
	return 0;
}


/* SEI FUNZIONI TOLTE, ed e' una scelta con una prova sola per tutte e sei:
 * sono GLOBALI (nessuna e' `static`, nessuna sta sotto un #ifdef che di
 * fabbrica sia spento), quindi il compilatore le DEVE emettere e nessun
 * chiamante puo' assorbirle; e nel blocco di fabbrica
 * [0xffffff8008a7221c, 0xffffff8008a7b600) -- 85 simboli, letti uno per uno
 * da stock.map, che contiene 30.100 `T` e 23.372 `t` -- non ce n'e' nessuna.
 *   gt1x_is_tpd_halt        12 byte   (ALPS: `return tpd_halt;`)
 *   gt1x_auto_update_done   52
 *   gt1x_pm_notifier       272        letterale 'PM_RESTORE_PREPARE enter':
 *                                     0 occorrenze nell'immagine
 *   tpd_reregister_from_tui 76
 *   tpd_enter_tui           60        letterale 'enter tui': 0 occorrenze
 *   tpd_exit_tui           248        letterale 'exit TUI+': 0 occorrenze
 * (le tre conte di occorrenze sono del cappello di gt1x_wtk.c, sezione
 * "I 15 SIMBOLI SENZA OMOLOGO"; qui si aggiunge l'argomento della mappa, che
 * vale anche per le tre che non stampano niente.)
 *
 * Toglierle ha un secondo effetto, misurato: tpd_irq_registration passa da
 * TRE chiamanti a UNO e clang la incorpora in tpd_registration, che e'
 * esattamente cio' che il binario di fabbrica mostra (vedi il cappello di
 * tpd_irq_registration).  Con le sei al loro posto tpd_registration misurava
 * 624 byte contro i 992 di fabbrica.
 *
 * Restano nell'header le loro `extern`: una dichiarazione non emette codice,
 * e toglierla e' lavoro del lotto di merge (delta di header).
 */

/* Di fabbrica questa funzione e' INCORPORATA dentro tpd_registration, e lo
 * dice il suo __func__: i sei printk che stanno fra 0xffffff8008a77cf0 e
 * 0xffffff8008a77e38 passano in x1 "tpd_irq_registration"@0xffffff800924c4c9,
 * non "tpd_registration"@0xffffff800924c36e che e' quello dei printk che li
 * precedono nella stessa funzione.  E' la classe B5 vista al contrario: qui
 * il __func__ CONFERMA che due funzioni distinte sono state fuse.
 *
 * Tre differenze rispetto ad ALPS, tutte misurate:
 * (a) IL RISULTATO DI of_property_read_u32_array NON E' GUARDATO.  Fra
 *       "9403c122 bl"@0xffffff8008a77d3c   -> of_property_read_variable_u32_array
 *     e
 *       "294053e0 ldp"@0xffffff8008a77d40  w0,w20 = ints[0],ints[1]
 *     non c'e' nessun cbz/cbnz/cmp: si passa dritti a gpio_to_desc.  Sparisce
 *     con esso il messaggio "[%s]debounce time not found", che nel binario
 *     non esiste.
 * (b) L'ULTIMO MESSAGGIO E' UNO SOLO e porta tre valori:
 *     "<<GTP-INF>>[%s:%d] irq:%d, debounce:%d-%d:\n"@0xffffff800924c5a5
 *       "b94a7683 ldr"@0xffffff8008a77e1c   w3 = touch_irq
 *       "294017e4 ldp"@0xffffff8008a77e20   w4,w5 = ints[0], ints[1]
 *     al posto dei due di ALPS ("[%s]debounce:%d-%d" e "[%s]irq:%d").
 * (c) `gt1x_int_type` e' letto a un byte -- "39670263 ldrb"@0xffffff8008a77d60
 *     -- cioe' e' un u8, come lo dichiara l'header.
 *
 * "<<GTP-INF>>[%s:%d] Device Tree Tpd_irq_registration!\n"@0xffffff800924c493
 * "<<GTP-INF>>[%s:%d] Device gt1x_int_type = %d!\n"@0xffffff800924c4de
 * "TOUCH_PANEL-eint"@0xffffff800924c50d
 * "<<GTP-ERR>>[%s:%d] tpd request_irq IRQ LINE NOT AVAILABLE!.\n"@0xffffff800924c51e
 * "<<GTP-ERR>>[%s:%d] tpd request_irq can not find touch eint device node!.\n"@0xffffff800924c55b
 * "debounce"@0xffffff800911984a
 * I due rami di request_irq differiscono solo per il flag:
 *   "321f03e3 orr"@0xffffff8008a77da0   w3 = 2 = IRQF_TRIGGER_FALLING
 *   "320003e3 orr"@0xffffff8008a77de8   w3 = 1 = IRQF_TRIGGER_RISING
 * e il confronto che segue e' con UNO, non con zero:
 *   "7100041f cmp"@0xffffff8008a77db0 / "5400032b b.lt"@0xffffff8008a77db4
 * cioe' `if (ret > 0)`, che e' il difetto di ALPS e si riproduce (regola 7:
 * request_irq torna NEGATIVO in errore, quindi quel ramo non scatta mai).
 */
static int tpd_irq_registration(void)
{
	struct device_node *node = NULL;
	int ret = 0;
	u32 ints[2] = {0, 0};

#line 473
	GTP_INFO("Device Tree Tpd_irq_registration!");

	node = of_find_matching_node(node, touch_of_match);
	if (node) {
		of_property_read_u32_array(node, "debounce", ints,
					   ARRAY_SIZE(ints));
		gpio_set_debounce(ints[0], ints[1]);

		touch_irq = irq_of_parse_and_map(node, 0);
#line 481
		GTP_INFO("Device gt1x_int_type = %d!", gt1x_int_type);
		if (!gt1x_int_type) { /*EINTF_TRIGGER*/
			ret = request_irq(
				touch_irq,
				(irq_handler_t)tpd_eint_interrupt_handler,
				IRQF_TRIGGER_RISING, "TOUCH_PANEL-eint", NULL);
			if (ret > 0) {
				ret = -1;
				GTP_ERROR(
#line 488
					"tpd request_irq IRQ LINE NOT AVAILABLE!.");
			}
		} else {
			ret = request_irq(
				touch_irq,
				(irq_handler_t)tpd_eint_interrupt_handler,
				IRQF_TRIGGER_FALLING, "TOUCH_PANEL-eint", NULL);
			if (ret > 0) {
				ret = -1;
				GTP_ERROR(
#line 495
					"tpd request_irq IRQ LINE NOT AVAILABLE!.");
			}
		}
	} else {
		GTP_ERROR(
#line 499
			"tpd request_irq can not find touch eint device node!.");
		ret = -1;
	}
	GTP_INFO("irq:%d, debounce:%d-%d:", touch_irq, ints[0], ints[1]);
	return ret;
}

/* Sei differenze rispetto ad ALPS, tutte lette dai 992 byte di fabbrica.
 *
 * (1) LA spin_lock_init STA QUI, non in tpd_local_init: subito dopo aver
 *     salvato il client, e prima di gt1x_init.
 *       "f9045520 str"@0xffffff8008a77b0c   gt1x_i2c_client = client
 *       "b90a6d5f str"@0xffffff8008a77b10   [irq_flag_lock] = 0
 *       "97fff5f5 bl"@0xffffff8008a77b14    -> gt1x_init
 *     (2668 = 0xa6c, che e' l'indirizzo che gt1x_irq_enable passa a
 *     _raw_spin_lock_irqsave.)
 * (2) IL FALLIMENTO DI gt1x_init NON TORNA.  Dopo aver azzerato i due
 *     massimi si prosegue: "340000a0 cbz"@0xffffff8008a77b18 salta le due
 *     `str wzr` e riprende alla lettura di tpd_load_status, che e' il vero
 *     bivio -- "34001a28 cbz"@0xffffff8008a77b34 salta TUTTO il resto e va
 *     direttamente alla coda comune (0xffffff8008a77e78).
 * (3) tpd_button_setting E' CHIAMATA UNA SECONDA VOLTA, con gli stessi tre
 *     argomenti di tpd_local_init: "320007e0 orr"@0xffffff8008a77ba8 (w0=3),
 *     "911a6021 add"@0xffffff8008a77ba0, "911a9042 add"@0xffffff8008a77ba4,
 *     "97ff6a32 bl"@0xffffff8008a77bac.
 * (4) LE CAPACITA' DI TASTO SONO QUINDICI E LETTERALI, senza nessun `if` e
 *     senza nessun ciclo: quindici `bl input_set_capability` consecutivi con
 *     w1 = 1 (EV_KEY) e w2 costante.  I quindici immediati, nell'ordine:
 *       "528005c2 mov"@0xffffff8008a77bbc  0x2e = 46
 *       "52800222 mov"@0xffffff8008a77bd0  0x11 = 17
 *       "52800642 mov"@0xffffff8008a77be4  0x32 = 50
 *       "321d07e2 orr"@0xffffff8008a77bf8  0x18 = 24
 *       "320013e2 orr"@0xffffff8008a77c0c  0x1f = 31
 *       "52800582 mov"@0xffffff8008a77c20  0x2c = 44
 *       "52800242 mov"@0xffffff8008a77c34  0x12 = 18
 *       "528005e2 mov"@0xffffff8008a77c48  0x2f = 47
 *       "52800ce2 mov"@0xffffff8008a77c5c  0x67 = 103
 *       "52800d82 mov"@0xffffff8008a77c70  0x6c = 108
 *       "52800d22 mov"@0xffffff8008a77c84  0x69 = 105
 *       "52800d42 mov"@0xffffff8008a77c98  0x6a = 106
 *       "52800bc2 mov"@0xffffff8008a77cac  0x5e = 94
 *       "52800e82 mov"@0xffffff8008a77cc0  0x74 = 116
 *       "52800422 mov"@0xffffff8008a77cd4  0x21 = 33
 *     I NOMI qui sotto non sono una scelta: sono quelli che
 *     include/uapi/linux/input-event-codes.h assegna a quei numeri, e ogni
 *     numero ne ha uno solo.  Come li scrivesse il sorgente di fabbrica il
 *     binario non lo dice.
 * (5) LA CODA E' COMUNE ai due rami e mette check_flag a vero PRIMA di
 *     svegliare init_waiter:
 *       "39268109 strb"@0xffffff8008a77e94   check_flag = 1
 *       "320003e1 orr"@0xffffff8008a77e88    w1 = 1 = TASK_INTERRUPTIBLE
 *       "97da953a bl"@0xffffff8008a77e98     -> __wake_up
 *     quindi wake_up_interruptible, non wake_up (che passerebbe 3).
 * (6) NON C'E' NESSUN kthread per l'auto-update e nessun register_pm_notifier:
 *     in tutta la funzione i `bl kthread_create_on_node` sono UNO solo
 *     ("97d983ed bl"@0xffffff8008a77b50), e il nome che gli passa e'
 *     "mtk-tpd"@0xffffff800923d9b2.
 *
 * "<<GTP-INF>>[%s:%d] mtk-tpd failed to create kernel thread: %d\n\n"@0xffffff800924c32e
 * "tpd_registration"@0xffffff800924c36e
 * Il ritorno e' sempre zero: "2a1f03e0 mov"@0xffffff8008a77eb8.
 */
static int tpd_registration(void *client)
{
	s32 err = 0;

	gt1x_i2c_client = client;
	spin_lock_init(&irq_flag_lock);

	if (gt1x_init()) {
/* TP resolution == LCD resolution, no need to match resolution */
		 /* when initialized fail */
		gt1x_abs_x_max = 0;
		gt1x_abs_y_max = 0;
	}

	if (tpd_load_status == 0)
		goto out;

	thread = kthread_run(tpd_event_handler, 0, TPD_DEVICE);
	if (IS_ERR(thread)) {
		err = PTR_ERR(thread);
		GTP_INFO(TPD_DEVICE " failed to create kernel thread: %d\n",
#line 530
			 err);
	}
	/*initialize tpd button data*/
	tpd_button_setting(3, tpd_keys_local, tpd_keys_dim_local);

	input_set_capability(tpd->dev, EV_KEY, KEY_C);
	input_set_capability(tpd->dev, EV_KEY, KEY_W);
	input_set_capability(tpd->dev, EV_KEY, KEY_M);
	input_set_capability(tpd->dev, EV_KEY, KEY_O);
	input_set_capability(tpd->dev, EV_KEY, KEY_S);
	input_set_capability(tpd->dev, EV_KEY, KEY_Z);
	input_set_capability(tpd->dev, EV_KEY, KEY_E);
	input_set_capability(tpd->dev, EV_KEY, KEY_V);
	input_set_capability(tpd->dev, EV_KEY, KEY_UP);
	input_set_capability(tpd->dev, EV_KEY, KEY_DOWN);
	input_set_capability(tpd->dev, EV_KEY, KEY_LEFT);
	input_set_capability(tpd->dev, EV_KEY, KEY_RIGHT);
	input_set_capability(tpd->dev, EV_KEY, KEY_MUHENKAN);
	input_set_capability(tpd->dev, EV_KEY, KEY_POWER);
	input_set_capability(tpd->dev, EV_KEY, KEY_F);

	GTP_GPIO_AS_INT(GTP_INT_PORT);

	msleep(50);
	/* EINT device tree, default EINT enable */
	tpd_irq_registration();
	gt1x_irq_enable();

out:
	check_flag = true;
	wake_up_interruptible(&init_waiter);
	return 0;
}

/* Di fabbrica questa funzione fa quasi tutto quello che ALPS fa nel nodo
 * /proc: legge la versione, compone la riga di informazione del pannello e
 * la consegna a fix_tp_proc_info (0xffffff8008a5151c).  Otto differenze.
 *
 * (1) IL PRIMO E L'ULTIMO MESSAGGIO SONO printk NUDE, senza prefisso di
 *     livello e senza argomenti: alla chiamata e' preparato solo x0.
 *       "91066800 add"@0xffffff8008a77820 / "97daef2b bl"@0xffffff8008a77828
 *       "910c6400 add"@0xffffff8008a77a6c / "97daee99 bl"@0xffffff8008a77a70
 *     " tpd_i2c_probe start.\n"@0xffffff800924c19a
 *     " tpd_i2c_probe end.\n"@0xffffff800924c319
 *     Il byte che precede il primo e' il NUL della stringa prima, non un
 *     KERN_SOH: sono davvero senza livello.
 * (2) IN RECOVERY SI TORNA -1, non 0: "12800014 mov"@0xffffff8008a77838
 *     mette w20 = -1 e salta all'epilogo, che fa "2a1403e0 mov"@0xffffff8008a77a94.
 * (3) L'INDIRIZZO I2C E' RISCRITTO A 0x5D PRIMA DI AVVIARE IL THREAD:
 *       "52800ba8 mov"@0xffffff8008a77848    w8 = 0x5d
 *       "79000668 strh"@0xffffff8008a7785c   [client+2] = w8
 *     e +2 e' `addr` di struct i2c_client (flags sta a +0).  GTP_I2C_ADDRESS
 *     dell'header vale 0xBA, che e' lo stesso indirizzo a otto bit.
 * (4) L'ATTESA E' wait_event_interruptible, non wait_event_timeout: non c'e'
 *     nessun conto di scadenza e prepare_to_wait_event prende w2 = 1
 *     ("320003e2 orr"@0xffffff8008a778e4), col controllo di segnale
 *     "b5000180 cbnz"@0xffffff8008a778fc.
 * (5) DOPO L'ATTESA SI CONTROLLA tpd_load_status e si torna -1:
 *       "b94956c8 ldr"@0xffffff8008a77948 / "34fff768 cbz"@0xffffff8008a7794c
 *     (2388 = 0x954, lo stesso indirizzo che tpd_local_init legge).
 * (6) SI RILEGGE LA VERSIONE: "97fff256 bl"@0xffffff8008a7795c ->
 *     gt1x_read_version(&gt1x_version), e in errore si torna quel valore.
 *     "<<GTP-ERR>>[%s:%d] Get verision failed!\n"@0xffffff800924b3a1  (il
 *     refuso "verision" e' della fabbrica e si riproduce)
 * (7) IL CONFRONTO CON NULL SULL'INDIRIZZO DELLA RIGA C'E' DAVVERO:
 *       "9b0824c1 madd"@0xffffff8008a77998
 *       "b40002a1 cbz"@0xffffff8008a7799c
 *     e non e' eliminato perche' il kernel compila con
 *     -fno-delete-null-pointer-checks.  Il ramo "else" copia SETTE byte, non
 *     otto, e la parola scritta e' "unknow" con una `w` sola -- il refuso e'
 *     della fabbrica e si riproduce (regola 7).  I due store si sovrappongono
 *     di un byte, che e' come clang espande una strcpy di sette byte:
 *       "528dceaa mov"@0xffffff8008a779f8   w10 = 0x6e6b_6e75  'u','n','k','n'
 *       "b900010a str"@0xffffff8008a77a0c   [g100a39+0] = w10   (byte 0..3)
 *       "528dedc9 mov"@0xffffff8008a779f4   w9  = 0x0077_6f6e  'n','o','w',NUL
 *       "b8003109 stur"@0xffffff8008a77a08  [g100a39+3] = w9    (byte 3..6)
 *     cioe' 'u','n','k','n','o','w',NUL.  Con "unknown" (otto byte) clang
 *     emette un solo `str x9` e la funzione misura 692 invece di 700.
 *     Il letterale resta anche in rodata, benche' la strcpy sia stata
 *     incorporata, e sta appiccicato ai messaggi di questa unita':
 *     "unknow"@0xffffff800924bdc1 (i sette byte che precedono
 *     "<<GTP-DBG>>[%s:%d]Power switch on!\n"@0xffffff800924bdc8).
 * (8) LE TRE sprintf E LE DUE COSTANTI.  Il sensore vale 0, 2 o altro:
 *       "34000326 cbz"@0xffffff8008a779b0    sensor_id == 0
 *       "710008df cmp"@0xffffff8008a779c4    sensor_id == 2
 *     e i due rami costanti passano in w6
 *       "52802326 mov"@0xffffff8008a77a30    0x119 = 281
 *       "52804086 mov"@0xffffff8008a779e8    0x204 = 516
 *     "TP IC:GT%s,TP MODULE:%s,TP I2C ADR:0x%x,SW FirmWare:0x%06X,Sample FirmWare:0x%06X"@0xffffff800924c27f
 *     "TP IC:GT%s,TP MODULE:%s,TP I2C ADR:0x%x,SW FirmWare:0x%06X,sensor id:%d"@0xffffff800924c2d1
 *     Le due costanti sono scritte 0x119 e 0x204 perche' il formato le
 *     stampa con %06X; che nel sorgente di fabbrica fossero in esadecimale
 *     il binario non lo dice.
 *
 * "<<GTP-ERR>>[%s:%d] mtk-tpd failed to create kernel thread: %d\n\n"@0xffffff800924c1b1
 * "tpd_i2c_probe"@0xffffff800924c1f1
 * "<<GTP-INF>>[%s:%d] tpd_i2c_probe start.wait_event_interruptible\n"@0xffffff800924c1ff
 * "<<GTP-INF>>[%s:%d] tpd_i2c_probe end.wait_event_interruptible\n"@0xffffff800924c240
 * "tpd_probe"@0xffffff800923d424
 * Il buffer della sprintf e' di 512 byte: sta a sp+8 e il canarino, che
 * clang mette in cima all'area dei locali, e' a sp+0x208 --
 * "f81c83a8 stur"@0xffffff8008a77824 con x29 = sp_iniziale+0x30 e
 * "d10843ff sub"@0xffffff8008a7780c che apre 0x210 byte.
 */
static s32 tpd_i2c_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int err = 0;
	int len;
	char buf[512];

	printk(" tpd_i2c_probe start.\n");
#ifdef CONFIG_MTK_BOOT
	if (get_boot_mode() == RECOVERY_BOOT)
		return -1;
#endif
	client->addr = 0x5d;
	probe_thread =
		kthread_run(tpd_registration, (void *)client, "tpd_probe");
	if (IS_ERR(probe_thread)) {
		err = PTR_ERR(probe_thread);
		GTP_ERROR(TPD_DEVICE " failed to create kernel thread: %d\n",
#line 609
			  err);
		return err;
	}
#line 613
	GTP_INFO("tpd_i2c_probe start.wait_event_interruptible");
	wait_event_interruptible(init_waiter, check_flag == true);
	GTP_INFO("tpd_i2c_probe end.wait_event_interruptible");

	if (tpd_load_status == 0)
		return -1;

	err = gt1x_read_version(&gt1x_version);
	if (err) {
#line 625
		GTP_ERROR("Get verision failed!");
		return err;
	}

	if (g98a468[gt1x_version.sensor_id])
		strcpy(g100a39, g98a468[gt1x_version.sensor_id]);
	else
		strcpy(g100a39, "unknow");

	if (gt1x_version.sensor_id == 0)
		len = sprintf(
			buf,
			"TP IC:GT%s,TP MODULE:%s,TP I2C ADR:0x%x,SW FirmWare:0x%06X,Sample FirmWare:0x%06X",
			gt1x_version.product_id, g100a39, client->addr,
			gt1x_version.patch_id, 0x119);
	else if (gt1x_version.sensor_id == 2)
		len = sprintf(
			buf,
			"TP IC:GT%s,TP MODULE:%s,TP I2C ADR:0x%x,SW FirmWare:0x%06X,Sample FirmWare:0x%06X",
			gt1x_version.product_id, g100a39, client->addr,
			gt1x_version.patch_id, 0x204);
	else
		len = sprintf(
			buf,
			"TP IC:GT%s,TP MODULE:%s,TP I2C ADR:0x%x,SW FirmWare:0x%06X,sensor id:%d",
			gt1x_version.product_id, g100a39, client->addr,
			gt1x_version.patch_id, gt1x_version.sensor_id);

	fix_tp_proc_info(buf, len);
	printk(" tpd_i2c_probe end.\n");
	tpd_load_status = 1;
	return 0;
}

/* Di fabbrica il gestore NON legge irq_flag e NON esce prima: disabilita
 * sempre.  Il lucchetto e' lo spin_lock semplice, non la variante irqsave --
 *   "940ffa2c bl"@0xffffff8008a78354  -> _raw_spin_lock   (0xffffff8008e76c04)
 *   "940ffaad bl"@0xffffff8008a78370  -> _raw_spin_unlock (0xffffff8008e76e24)
 * mentre gt1x_irq_enable/disable qui accanto chiamano _raw_spin_lock_irqsave
 * (0xffffff8008e76c88), quindi la differenza e' misurata e non dedotta.
 * E la disable_irq_nosync sta DENTRO il lucchetto:
 *   "3929c11f strb"@0xffffff8008a78364  irq_flag = 0
 *   "97db09f5 bl"@0xffffff8008a78368   -> disable_irq_nosync
 *   "940ffaad bl"@0xffffff8008a78370   -> _raw_spin_unlock
 * `tpd_flag = 1` e' scritto PRIMA di prendere il lucchetto
 *   "392a0109 strb"@0xffffff8008a78350
 * ed e' un `strb` di un byte: tpd_flag e' `static` e clang lo restringe (vale
 * solo 0 o 1).  Il ritorno e' IRQ_HANDLED, "320003e0 orr"@0xffffff8008a78390.
 *
 * IL RISVEGLIO E' `wake_up_interruptible`, NON `wake_up`: la dimensione non
 * lo denuncia (stesso numero di istruzioni), la codifica si'.  __wake_up
 * prende in w1 la maschera degli stati, e di fabbrica vale UNO --
 *   "320003e1 orr"@0xffffff8008a7837c   w1 = 1  = TASK_INTERRUPTIBLE
 *   "320003e2 orr"@0xffffff8008a78380   w2 = 1  = nr_exclusive
 *   "aa1f03e3 mov"@0xffffff8008a78384   x3 = 0  = key
 *   "97da93fe bl"@0xffffff8008a78388    -> __wake_up (0xffffff800811d380)
 * mentre `wake_up` passerebbe TASK_NORMAL = 3 (`orr w1, wzr, #0x3`), che e'
 * cio' che ALPS scriveva qui.
 */
static irqreturn_t tpd_eint_interrupt_handler(unsigned int irq,
					      struct irq_desc *desc)
{
	TPD_DEBUG_PRINT_INT;
	tpd_flag = 1;
	spin_lock(&irq_flag_lock);
	irq_flag = 0;
	disable_irq_nosync(touch_irq);
	spin_unlock(&irq_flag_lock);
	wake_up_interruptible(&waiter);
	return IRQ_HANDLED;
}

/* Le due funzioni di riporto.  Di fabbrica NON contengono ne'
 * TPD_DEBUG_SET_TIME ne' TPD_EM_PRINT: con TPD_DEBUG_CODE accesa (vedi il
 * cappello di tpd.h) quelle due macro sono chiamate vere a
 * tpd_debug_set_time (0xffffff8008a529d8) e a tpd_em_log_output /
 * MET_touch, e nei 320 byte di gt1x_touch_down non c'e' nessun `bl` verso
 * quegli indirizzi -- le uniche chiamate sono input_event, get_boot_mode e
 * tpd_button.  Percio' le righe non ci sono, e con esse spariscono
 * tpd_history_x/tpd_history_y: erano lette SOLO dalla TPD_EM_PRINT di
 * gt1x_touch_up.  (Scelta dichiarata: due `static` scritte e mai lette non
 * entrano nel binario in nessun caso, quindi il binario non puo' dire se il
 * sorgente di fabbrica le contenga.)
 *
 * ABS_MT_PRESSURE 0x3a "52800742 mov"@0xffffff8008a77004
 * ABS_MT_TOUCH_MAJOR 0x30 "321c07e2 orr"@0xffffff8008a77024
 * ABS_MT_TRACKING_ID 0x39 "52800722 mov"@0xffffff8008a7703c
 * ABS_MT_POSITION_X 0x35 "528006a2 mov"@0xffffff8008a77070
 * ABS_MT_POSITION_Y 0x36 "528006c2 mov"@0xffffff8008a77088
 * BTN_TOUCH 0x14a "52802942 mov"@0xffffff8008a76fec
 * il ramo virtuale e' `(!size) && (!id)`, cioe' un OR dei due:
 *   "2a1602a8 orr"@0xffffff8008a7700c  w8 = id | size
 *   "340001c8 cbz"@0xffffff8008a77010
 * e il valore che vi si riporta e' 100, "52800c83 mov"@0xffffff8008a77048.
 */
void gt1x_touch_down(s32 x, s32 y, s32 size, s32 id)
{
	input_report_key(tpd->dev, BTN_TOUCH, 1);
	if ((!size) && (!id)) {
		/* for virtual button */
		input_report_abs(tpd->dev, ABS_MT_PRESSURE, 100);
		input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 100);
	} else {
		input_report_abs(tpd->dev, ABS_MT_PRESSURE, size);
		input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, size);
		input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, id);
	}
	input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(tpd->dev);
#ifdef CONFIG_MTK_BOOT
	if (tpd_dts_data.use_tpd_button) {
		if (get_boot_mode() == FACTORY_BOOT ||
		    get_boot_mode() == RECOVERY_BOOT)
			tpd_button(x, y, 1);
	}
#endif
}

/* gt1x_touch_up alza anche BTN_TOUCH, che ALPS qui non tocca:
 *   "320003e1 orr"@0xffffff8008a7710c   w1 = EV_KEY
 *   "52802942 mov"@0xffffff8008a77110   w2 = BTN_TOUCH
 *   "2a1f03e3 mov"@0xffffff8008a77114   w3 = 0
 *   "97ff3db9 bl"@0xffffff8008a7711c    -> input_event (0xffffff8008a46800)
 */
void gt1x_touch_up(s32 id)
{
	input_report_key(tpd->dev, BTN_TOUCH, 0);
	input_mt_sync(tpd->dev);
#ifdef CONFIG_MTK_BOOT
	if (tpd_dts_data.use_tpd_button) {
		if (get_boot_mode() == FACTORY_BOOT ||
		    get_boot_mode() == RECOVERY_BOOT)
			tpd_button(0, 0, 0);
	}
#endif
}

#ifdef CONFIG_GTP_CHARGER_SWITCH
u32 gt1x_get_charger_status(void)
{
	u32 chr_status = 0;
#ifdef MT6573
	chr_status = *(u32 *)CHR_CON0;
	chr_status &= (1 << 13);
#else /* ( defined(MT6575) || defined(MT6577) || defined(MT6589) ) */
	chr_status = upmu_is_chr_det();
#endif
	return chr_status;
}
#endif

/* Sette differenze rispetto ad ALPS, misurate sui 1104 byte di fabbrica.
 *
 * (1) L'ATTESA E' INTERROMPIBILE.  prepare_to_wait_event riceve w2 = 1 --
 *     "320003e2 orr"@0xffffff8008a77f9c -- e il suo risultato e' guardato
 *     ("b5000180 cbnz"@0xffffff8008a77fb0), che e' il controllo di segnale
 *     che solo wait_event_interruptible fa.  ALPS scrive wait_event.
 * (2) IL CONTROLLO DI update_info.status C'E' ED E' PRIMA DEL LUCCHETTO:
 *       "b9470708 ldr"@0xffffff8008a77ff4   w8 = update_info.status
 *       "34000128 cbz"@0xffffff8008a77ff8
 *       "52806062 mov"@0xffffff8008a78000   __LINE__ = 771
 *     e il mutex_lock viene dopo, a "940fecd7 bl"@0xffffff8008a78020.
 *     "<<GTP-DBG>>[%s:%d]Ignore interrupts during fw updating.\n"@0xffffff800924c3bd
 * (3) IL RAMO DEL GESTO E' SOTTO UN `if`, non sotto un #ifdef: il guardiano
 *     e' lo stesso byte che gt1x_suspend e gt1x_resume leggono in
 *     gt1x_generic.c -- "3942e108 ldrb"@0xffffff8008a78028 (0xffffff800a0fc0b8,
 *     che quel file chiama g0fc0b8 perche' il binario non lo nomina).
 * (4) IL MESSAGGIO DEL SOSPESO E' UN ALTRO:
 *     "<<GTP-DBG>>[%s:%d]Ignore interrupts after suspend.\n"@0xffffff800924c3f6
 *     al posto di "return for interrupt after suspend...  " di ALPS, che
 *     nell'immagine non c'e'.
 * (5) `finger` E' CON SEGNO.  "39c0e3e8 ldrsb"@0xffffff8008a78118 e non un
 *     ldrb, e il bit provato e' il 31 della parola estesa --
 *     "36f80ae8 tbz"@0xffffff8008a78124 -- non il 7.  ALPS lo dichiara u8.
 * (6) DOPO gt1x_touch_event_handler C'E' UNA printk NUDA, senza prefisso e
 *     senza __func__: riceve solo x0 e w1.
 *       "91114800 add"@0xffffff8008a78148   x0 = 0xffffff800924c452
 *       "2a1a03e1 mov"@0xffffff8008a7814c   w1 = ret
 *       "97daece1 bl"@0xffffff8008a78150    -> printk
 *     "--lan-- ret = %d.\n"@0xffffff800924c452
 *     e non c'e' nessun `if (ret)` dopo: si va dritti a exit_work_func
 *     ("1400001b b"@0xffffff8008a78154 -> 0xffffff8008a781c0).
 * (7) IL RAMO D'ERRORE DELLA LETTURA I2C SBLOCCA DUE VOLTE, e si riproduce
 *     (regola 7).  Dopo gt1x_power_reset fa gt1x_irq_enable e mutex_unlock
 *       "97fff154 bl"@0xffffff8008a78170   -> gt1x_power_reset
 *       "940ffac3 bl"@0xffffff8008a7817c   -> _raw_spin_lock_irqsave
 *       "940fec84 bl"@0xffffff8008a781bc   -> mutex_unlock (0xffffff800998a6d8)
 *     e POI cade in exit_work_func ("b949b508 ldr"@0xffffff8008a781c4 e' la
 *     parola dopo), che li rifa tutti e due
 *       "940ffa98 bl"@0xffffff8008a78228   -> _raw_spin_lock_irqsave
 *       "940fec58 bl"@0xffffff8008a7826c   -> mutex_unlock
 *     Non e' una duplicazione del compilatore: le due copie NON hanno lo
 *     stesso successore, e in mezzo c'e' il blocco di end_cmd.
 * (8) LA CONDIZIONE DI end_cmd HA UN SECONDO PEZZO che ALPS non ha:
 *       "350002c8 cbnz"@0xffffff8008a781c8   salta se gt1x_rawdiff_mode
 *       "36f8009a tbz"@0xffffff8008a781cc    scrive se ret >= 0
 *       "52b00028 mov"@0xffffff8008a781d0    w8 = 0x80010000 = ERROR_VALUE
 *       "6b08035f cmp"@0xffffff8008a781d4    scrive anche se ret == ERROR_VALUE
 *
 * "tpd_event_handler"@0xffffff800924c3ab
 * "<<GTP-DBG>>[%s:%d]Polling coordinate mode!\n"@0xffffff800924c37f
 * "<<GTP-ERR>>[%s:%d] I2C transfer error!\n"@0xffffff800924c42a
 * "<<GTP-INF>>[%s:%d] I2C write end_cmd  error!\n"@0xffffff800924c465
 *
 * DELTA DI HEADER GIA' APERTO DA UN ALTRO LOTTO, e questa e' la terza
 * conferma: "b949b508 ldr"@0xffffff8008a781c4 legge gt1x_rawdiff_mode a
 * QUATTRO byte, mentre gt1x_tpd_common.h:397 la dichiara `u8` e
 * gt1x_generic.c:212 la definisce `u8`.  Da noi esce `ldrb`.  Non cambia la
 * dimensione (una posizione di mnemonico), e la correzione tocca
 * gt1x_generic.c, che e' di un altro lotto: la lascio al merge, come gia'
 * fa la nota D1 in coda a gt1x_tools.c.
 */
static int tpd_event_handler(void *unused)
{
	s8 finger = 0;
	u8 end_cmd = 0;
	s32 ret = 0;
	u8 point_data[11] = {0};
	struct sched_param param = {.sched_priority = 4};

	sched_setscheduler(current, SCHED_RR, &param);
	do {
		set_current_state(TASK_INTERRUPTIBLE);

		if (tpd_eint_mode) {
			wait_event_interruptible(waiter, tpd_flag != 0);
			tpd_flag = 0;
		} else {
#line 764
			GTP_DEBUG("Polling coordinate mode!");
			msleep(tpd_polling_time);
		}

		set_current_state(TASK_RUNNING);

		if (update_info.status) {
			GTP_DEBUG("Ignore interrupts during fw updating.");
			continue;
		}

		mutex_lock(&i2c_access);
/* don't reset before "if (tpd_halt..."  */

		if (g0fc0b8) {
			ret = gesture_event_handler(tpd->dev);
			if (ret >= 0) {
				gt1x_irq_enable();
				mutex_unlock(&i2c_access);
				continue;
			}
		}

		if (tpd_halt) {
			mutex_unlock(&i2c_access);
#line 791
			GTP_DEBUG("Ignore interrupts after suspend.");
			continue;
		}

		/* read coordinates */
		ret = gt1x_i2c_read(GTP_READ_COOR_ADDR, point_data,
				    sizeof(point_data));
		if (ret < 0) {
#line 798
			GTP_ERROR("I2C transfer error!");
			gt1x_power_reset();
			gt1x_irq_enable();
			mutex_unlock(&i2c_access);
			goto exit_work_func;
		}
		finger = point_data[0];

		/* response to a ic request */
		if (finger == 0x00)
			gt1x_request_event_handler();

		if ((finger & 0x80) == 0) {
			gt1x_irq_enable();
			mutex_unlock(&i2c_access);
			continue;
		}

		ret = gt1x_touch_event_handler(point_data, tpd->dev, NULL);
		printk("--lan-- ret = %d.\n", ret);

exit_work_func:

		if (!gt1x_rawdiff_mode && (ret >= 0 || ret == ERROR_VALUE)) {
			ret = gt1x_i2c_write(GTP_READ_COOR_ADDR, &end_cmd, 1);
			if (ret < 0)
#line 850
				GTP_INFO("I2C write end_cmd  error!");
		}
		gt1x_irq_enable();
		mutex_unlock(&i2c_access);

	} while (!kthread_should_stop());

	return 0;
}

/* Di fabbrica i rami di modo sono TRE, non sei: "polling", "eint" e
 * "switch".  I tre di ALPS che mancano ("enable_irq", "rerequest_irq",
 * "eint_dump_status") non ci sono, e la prova non e' la dimensione ma i
 * CONFRONTI: nei 536 byte ce ne sono esattamente tre, uno per modo --
 *   "eb09011f cmp"@0xffffff8008a771d4   contro la parola "polling\0", che
 *       clang materializza con "d28dee09 mov"@0xffffff8008a771c4 e tre movk
 *       (0x0067_6e69_6c6c_6f70);
 *   "940f824e bl"@0xffffff8008a771ec    -> __pi_memcmp, lunghezza 5
 *       ("528000a2 mov"@0xffffff8008a771e8), su "eint"@0xffffff80091feacc;
 *   "940f8248 bl"@0xffffff8008a77204    -> __pi_memcmp, lunghezza 7
 *       ("32000be2 orr"@0xffffff8008a77200), su "switch"@0xffffff80090ec24c.
 * Dopo il terzo si va dritti a "12800013 mov"@0xffffff8008a7720c (w19 = -1),
 * che e' il `return -1` finale: nessun quarto confronto, e nessuno dei
 * letterali "enable_irq", "rerequest_irq", "eint_dump_status" esiste da
 * nessuna parte nell'immagine.
 *
 * LA STRINGA DI FORMATO DELLA sscanf E' "%s %d", SENZA IL LIMITE:
 *   "%s %d"@0xffffff8009263d94, caricata da "91365021 add"@0xffffff8009... no:
 *   "91365021 add"@0xffffff8008a77198  (x1 = 0x9263d94)
 * ALPS scrive "%49s %d".  Il buffer resta di cinquanta byte azzerati
 * ("790083ff strh"@0xffffff8008a771a8 piu' i tre "stp xzr, xzr" che lo
 * precedono, 48+2 byte): e' una scrittura fuori limite vera della fabbrica e
 * si riproduce (regola 7).
 *
 * IL MESSAGGIO DELLA sscanf HA IL NOME SCRITTO DENTRO, non un %s:
 *   "<<GTP-ERR>>[%s:%d] gt1x_debug_proc sscanf failed\n"@0xffffff800924beb6
 * e infatti quel printk riceve solo x0, x1 e w2 -- "52806c82
 * mov"@0xffffff8008a77228 e' l'ultimo registro preparato prima della
 * chiamata, nessun x3.  ALPS scrive GTP_ERROR("%s sscanf failed", __func__),
 * che ne vorrebbe tre.
 *
 * IL RISVEGLIO E' wake_up_interruptible: "320003e1 orr"@0xffffff8008a7727c
 * mette w1 = 1 prima di "97da983b bl"@0xffffff8008a77294 (__wake_up), come
 * in tpd_eint_interrupt_handler.
 *
 * tpd_eint_mode e tpd_polling_time, misurati:
 *   "3929f109 strb"@0xffffff8008a77288   tpd_eint_mode = 0  (un byte: e'
 *       `static` e clang lo restringe; il valore INIZIALE e' 1, e per questo
 *       la forma ridotta memorizza 1 quando il sorgente scrive 0)
 *   "3929f11f strb"@0xffffff8008a772b8   tpd_eint_mode = 1
 *   "b905596a str"@0xffffff8008a7728c    tpd_polling_time = mode, quattro
 *       byte a 0xffffff800998a558, che nell'immagine contiene 32 00 00 00 =
 *       50: e' l'inizializzatore, e conferma `static int tpd_polling_time =
 *       50;` in .data e non in .bss.
 *
 * "gt1x_debug_proc"@0xffffff800924bee8
 * "<<GTP-INF>>[%s:%d] Switch to polling mode, polling time is %d\n"@0xffffff800924bef8
 * "<<GTP-INF>>[%s:%d] Wrong polling time, please set between 10~200ms\n"@0xffffff800924bf37
 * "<<GTP-INF>>[%s:%d] Switch to eint mode\n"@0xffffff800924bf7b
 * "<<GTP-ERR>>[%s:%d] error mode :%d\n"@0xffffff800924bfa3
 * L'intervallo di polling e' provato da
 *   "51002868 sub"@0xffffff8008a7723c   w8 = mode - 10
 *   "7102f91f cmp"@0xffffff8008a77240   contro 190
 *   "540003e8 b.hi"@0xffffff8008a77244
 * cioe' 10 <= mode <= 200.
 */
int gt1x_debug_proc(u8 *buf, int count)
{
	char mode_str[50] = {0};
	int mode;
	int ret;

	ret = sscanf(buf, "%s %d", (char *)&mode_str, &mode);
	if (ret < 0) {
#line 868
		GTP_ERROR("gt1x_debug_proc sscanf failed");
		return ret;
	}
	/***********POLLING/EINT MODE switch****************/
	if (strcmp(mode_str, "polling") == 0) {
		if (mode >= 10 && mode <= 200) {
			GTP_INFO("Switch to polling mode, polling time is %d",
#line 874
				 mode);
			tpd_eint_mode = 0;
			tpd_polling_time = mode;
			tpd_flag = 1;
			wake_up_interruptible(&waiter);
		} else {
			GTP_INFO(
#line 880
				"Wrong polling time, please set between 10~200ms");
		}
		return count;
	}
	if (strcmp(mode_str, "eint") == 0) {
		GTP_INFO("Switch to eint mode");
		tpd_eint_mode = 1;
		return count;
	}
	/**********************************************/
	if (strcmp(mode_str, "switch") == 0) {
		if (mode == 0) /*turn off*/
			tpd_off();
		else if (mode == 1) /*turn on*/
			tpd_on();
		else
			GTP_ERROR("error mode :%d", mode);
		return count;
	}

	return -1;
}

static u16 convert_productname(u8 *name)
{
	int i;
	u16 product = 0;

	for (i = 0; i < 4; i++) {
		product <<= 4;
		if (name[i] < '0' || name[i] > '9')
			product += '*';
		else
			product += name[i] - '0';
	}
	return product;
}

static int tpd_i2c_remove(struct i2c_client *client)
{
	gt1x_deinit();

	return 0;
}

/* Di fabbrica questa funzione e' piu' corta di ALPS di ottanta byte, e le
 * differenze sono cinque, tutte misurate.
 *
 * (1) NESSUN CONTROLLO IS_ERR SUL REGOLATORE.  Fra la regolator_get e la
 *     regulator_set_voltage non c'e' nessun confronto: la prima scrive il
 *     risultato in tpd->reg e la seconda parte subito --
 *       "97e8dced bl"@0xffffff8008a775f4   -> regulator_get
 *       "f9000500 str"@0xffffff8008a7760c  tpd->reg = x0
 *       "97e8e08f bl"@0xffffff8008a77610   -> regulator_set_voltage
 *     e l'unico cbz che segue e' su w0 della set_voltage
 *     ("34000120 cbz"@0xffffff8008a77614).
 * (2) NESSUNA spin_lock_init.  In tutti e 532 i byte non c'e' nessuna
 *     scrittura verso irq_flag_lock (0xffffff800a100a6c): il lucchetto e'
 *     inizializzato staticamente, quindi DEFINE_SPINLOCK e non
 *     `spinlock_t` + spin_lock_init.  L'indirizzo cade in .bss
 *     (.bss parte a 0xffffff8009a5b80c), che e' dove finisce anche un
 *     inizializzatore tutto a zero.
 * (3) IL MASSIMO DI ABS_MT_TRACKING_ID E' QUATTRO, costante:
 *       "52800721 mov"@0xffffff8008a77684    w1 = 0x39 = ABS_MT_TRACKING_ID
 *       "321e03e3 orr"@0xffffff8008a77688    w3 = 4
 *     Non c'e' nessuna lettura di tpd_dts_data.touch_max_num e nessun
 *     `if (!...)` che la riempia: quel campo sta a
 *     0xffffff800a0fbf94+offset e in questa funzione non viene toccato.
 *     Scrivo il numero 4: quale costante simbolica ci fosse nel sorgente di
 *     fabbrica il binario non lo dice.
 * (4) tpd_button_setting riceve 3 e due tabelle del driver (vedi il cappello
 *     di tpd_keys_local), e NON e' sotto nessun `if`: fra il cbz di
 *     tpd_load_status ("34000968 cbz"@0xffffff8008a7767c) e la chiamata non
 *     c'e' nessun altro salto.
 * (5) La prima GTP_INFO ha DUE spazi dopo la parentesi quadra, cioe' un
 *     letterale che comincia con uno spazio:
 *     "<<GTP-INF>>[%s:%d]  Device Tree get regulator!\n"@0xffffff800924c0ac
 *
 * Gli altri quattro messaggi:
 * "tpd_local_init"@0xffffff800923f9b7 (il __func__ di tutte e cinque)
 * "vtouch"@0xffffff800924c0dc
 * "<<GTP-ERR>>[%s:%d] regulator_set_voltage(%d) failed!\n\n"@0xffffff800924c0e3
 * "<<GTP-ERR>>[%s:%d] unable to add i2c driver.\n"@0xffffff800924c11a
 * "<<GTP-ERR>>[%s:%d] add error touch panel driver.\n"@0xffffff800924c148
 * "<<GTP-INF>>[%s:%d] end %s, %d\n\n"@0xffffff800924c17a
 * e la matrice di calibrazione, 32 byte copiati due volte, sta a
 * 0xffffff800924c650: 962 0 0 0 1600 0 0 0, che e' esattamente
 * TPD_CALIBRATION_MATRIX dell'header.
 *   "a941310b ldp"@0xffffff8008a776c0 / "a940210d ldp"@0xffffff8008a776c4
 *   "a901312b stp"@0xffffff8008a776dc / "a900212d stp"@0xffffff8008a776e0
 *   "a901314b stp"@0xffffff8008a776e4 / "a900214d stp"@0xffffff8008a776e8
 */
static int tpd_local_init(void)
{
	int ret;

	GTP_INFO(" Device Tree get regulator!");
	tpd->reg = regulator_get(tpd->tpd_dev, "vtouch");
	ret = regulator_set_voltage(tpd->reg, 2800000, 2800000); /*set 2.8v*/
	if (ret) {
		GTP_ERROR("regulator_set_voltage(%d) failed!\n", ret);
		return -1;
	}
	if (i2c_add_driver(&tpd_i2c_driver) != 0) {
		GTP_ERROR("unable to add i2c driver.");
		return -1;
	}
	if (tpd_load_status == 0) {
		GTP_ERROR("add error touch panel driver.");
		i2c_del_driver(&tpd_i2c_driver);
		return -1;
	}
	input_set_abs_params(tpd->dev, ABS_MT_TRACKING_ID, 0, 4, 0, 0);
	/*initialize tpd button data*/
	tpd_button_setting(3, tpd_keys_local, tpd_keys_dim_local);

	memcpy(tpd_calmat, tpd_def_calmat_local, 8 * 4);
	memcpy(tpd_def_calmat, tpd_def_calmat_local, 8 * 4);

	/*set vendor string*/
	tpd->dev->id.vendor = 0x00;
	tpd->dev->id.product = convert_productname(gt1x_version.product_id);
	tpd->dev->id.version = (gt1x_version.patch_id >> 8);

	GTP_INFO("end %s, %d\n", __func__, __LINE__);
	tpd_type_cap = 1;
	return 0;
}

/* Di fabbrica tpd_suspend e tpd_resume sono DUE RINVII di venti byte ciascuno:
 * tutto il corpo che ALPS tiene qui sta invece in gt1x_suspend/gt1x_resume,
 * che il lotto `generic` ha gia' scritto in gt1x_generic.c.  Le cinque
 * istruzioni di tpd_suspend, per intero:
 *   "a9bf7bfd stp"@0xffffff8008a777d0
 *   "910003fd mov"@0xffffff8008a777d4
 *   "97fff541 bl"@0xffffff8008a777d8   -> gt1x_suspend, 0xffffff8008a74cdc
 *   "a8c17bfd ldp"@0xffffff8008a777dc
 *   "d65f03c0 ret"@0xffffff8008a777e0
 * Nessuna preparazione di x0: il parametro `h` non viene passato, e il valore
 * di ritorno di gt1x_suspend (s32) viene scartato.  tpd_resume e' la stessa
 * forma con "97fff5f3 bl"@0xffffff8008a777ec -> gt1x_resume, 0xffffff8008a74fb8.
 */
static void tpd_suspend(struct device *h)
{
	gt1x_suspend();
}

static void tpd_resume(struct device *h)
{
	gt1x_resume();
}

/* Il nome e' "gt1x", non "gt9xx".  La struttura di fabbrica sta a
 * 0xffffff800998a578 -- e' quella che tpd_driver_init passa a tpd_driver_add,
 * "9115e000 add"@0xffffff8009383440 -- e il suo primo campo e' un puntatore
 * relocato:
 *   $ ./venv/bin/python3 relocazioni.py oracolo/stock.elf oracolo/stock.map \
 *       --indirizzo 0xffffff800998a578
 *   0xffffff800998a578: relocato, addend 0xffffff800924c0a7
 *   $ ./venv/bin/python3 leggi_stringa.py oracolo/stock.elf 0xffffff800924c0a7
 *   0xffffff800924c0a7: b'gt1x'  (len=4, hex=67743178)
 * "gt9xx" (quello di ALPS) non compare da nessuna parte nell'immagine.
 * L'indirizzo 0x998a578 conferma anche la disposizione dei dati: waiter sta a
 * 0x998a560 (24 byte) e tpd_i2c_driver a 0x998a5b0, quindi questa struttura
 * occupa esattamente i 56 byte fra le due, come nel nostro oggetto.
 */
/*
 * SI CHIAMA  E NON E' , e lo dice gt1x_wtk.c: quel
 * file -- che e' l'aggiunta Wingtech e sta in un'altra unita' -- la registra e
 * la toglie con quel nome (tpd_driver_add/&gt1x_tpd_driver alla riga 1392,
 * tpd_driver_remove alla 859).  La copia di GT5688 la chiamava
 *  e la teneva statica perche' li' e' lo stesso file a
 * registrarla.
 */
struct tpd_driver_t gt1x_tpd_driver = {
	.tpd_device_name = "gt1x",
	.tpd_local_init = tpd_local_init,
	.suspend = tpd_suspend,
	.resume = tpd_resume,
};

void tpd_off(void)
{
	gt1x_power_switch(SWITCH_OFF);
	tpd_halt = 1;
	gt1x_irq_disable();
}

void tpd_on(void)
{
	s32 ret = -1, retry = 0;

	while (retry++ < 5) {
		ret = tpd_power_on();
		if (ret < 0)
#line 1025
			GTP_ERROR("I2C Power on ERROR!");
		ret = gt1x_send_cfg(gt1x_config, gt1x_cfg_length);
		if (ret == 0) {
			GTP_DEBUG("Wakeup sleep send gt1x_config success.");
			break;
		}
	}
	if (ret < 0)
		GTP_ERROR("GTP later resume failed.");
	tpd_halt = 0;
}
/* called when loaded into kernel */
static int __init tpd_driver_init(void)
{
	/* con lo spazio davanti: il messaggio assemblato ha DUE spazi dopo la
	 * quadra --
	 * "<<GTP-INF>>[%s:%d]  Goodix touch panel driver init.\n"@0xffffff800924c5db
	 */
	GTP_INFO(" Goodix touch panel driver init.");
	tpd_get_dts_info();
	/* "tpd_driver_init"@0xffffff800924c610 e' il __func__ di tutti e due i
	 * messaggi; il secondo e'
	 * "<<GTP-INF>>[%s:%d] add generic driver failed\n\n"@0xffffff800924c620
	 * e il ramo che lo porta e' "36f800e0 tbz"@0xffffff8009383448, cioe'
	 * `< 0`.  Questa funzione sta in .init.text, FUORI dal blocco misurato
	 * [0xffffff8008a76cf4, 0xffffff8008a7839c): la mappa la mette a
	 * 0xffffff8009383418 e misura 88 byte, quanto la nostra.
	 */
	if (tpd_driver_add(&gt1x_tpd_driver) < 0)
		GTP_INFO("add generic driver failed\n");
	return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void)
{
	GTP_INFO("MediaTek gt91xx touch panel driver exit\n");
	tpd_driver_remove(&gt1x_tpd_driver);
}
module_init(tpd_driver_init);
module_exit(tpd_driver_exit);
