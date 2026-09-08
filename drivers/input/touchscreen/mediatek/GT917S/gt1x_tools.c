/* drivers/input/touchscreen/goodix_tool.c
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
 * LOTTO "tools" (2026-08-22): LE NOTE STANNO IN FONDO AL FILE, non qui.
 * I printk della fabbrica fissano venticinque __LINE__ (87, 90, 97, 100,
 * 141, 185, 190, 191, 199, 222, 225, 247, 261, 270, 306, 311, 315, 327,
 * 365, 371, 379, 390, 391, 402, 414): una riga aggiunta qui li sposta.
 */
#include "include/gt1x_tpd_common.h"
#include <generated/utsrelease.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

static ssize_t gt1x_tool_read(struct file *filp, char __user *buffer,
			      size_t count, loff_t *ppos);
static ssize_t gt1x_tool_write(struct file *filp, const char *buffer,
			       size_t count, loff_t *ppos);

static s32 gt1x_tool_release(struct inode *inode, struct file *filp);
static s32 gt1x_tool_open(struct inode *inode, struct file *file);

#pragma pack(1)
struct st_cmd_head {
	u8 wr;		  /*write read flag,0:R  1:W  2:PID 3:*/
	u8 flag;	  /*0:no need flag/int 1: need flag  2:need int*/
	u8 flag_addr[2];  /*flag address*/
	u8 flag_val;      /*flag val*/
	u8 flag_relation; /*flag_val:flag 0:not equal 1:equal 2:> 3:<*/
	u16 circle;       /*polling cycle*/
	u8 times;	  /*plling times*/
	u8 retry;	  /*I2C retry times*/
	u16 delay;	  /*delay before read or after write*/
	u16 data_len;     /*data length*/
	u8 addr_len;      /*address length*/
	u8 addr[2];       /*address*/
	u8 res[3];	  /*reserved*/
	u8 *data;	  /*data pointer*/
};
#pragma pack()
s32 DATA_LENGTH;
static struct st_cmd_head cmd_head;
DEFINE_MUTEX(rw_mutex);

s8 TP_TYPE[16] = "GT1X";

#define UPDATE_FUNCTIONS
#define DATA_LENGTH_UINT 512
#define CMD_HEAD_LENGTH (sizeof(struct st_cmd_head) - sizeof(u8 *))

static char procname[20] = {0};

static struct proc_dir_entry *gt1x_tool_proc_entry;
static struct file_operations gt1x_tool_fops = {
	.read = gt1x_tool_read,
	.write = gt1x_tool_write,
	.open = gt1x_tool_open,
	.release = gt1x_tool_release,
	.owner = THIS_MODULE,
};
static void set_tool_node_name(char *procname)
{
	int v0 = 0, v1 = 0, v2 = 0;
	int ret;

	ret = sscanf(UTS_RELEASE, "%d.%d.%d", &v0, &v1, &v2);
	sprintf(procname, "gmnode%02d%02d%02d", v0, v1, v2);
}

int gt1x_init_tool_node(void)
{
	memset(&cmd_head, 0, sizeof(cmd_head));
	cmd_head.wr = 1; /*if the first operation is read, will return fail.*/
	cmd_head.data = kzalloc(DATA_LENGTH_UINT, GFP_KERNEL);
	if (cmd_head.data == NULL) {
		GTP_ERROR("Apply for memory failed.");
		return -1;
	}
	GTP_INFO("Alloc memory size:%d.", DATA_LENGTH_UINT);
	DATA_LENGTH = DATA_LENGTH_UINT - GTP_ADDR_LENGTH;

	set_tool_node_name(procname);

	gt1x_tool_proc_entry = proc_create(procname, 0666, NULL, &gt1x_tool_fops);
	if (gt1x_tool_proc_entry == NULL) {
		GTP_ERROR("CAN't create proc entry /proc/%s.", procname);
		return -1;
	}
	GTP_INFO("Created proc entry /proc/%s.", procname);
	return 0;
}

void gt1x_deinit_tool_node(void)
{
	remove_proc_entry(procname, NULL);
	kfree(cmd_head.data);
	cmd_head.data = NULL;
}

/* Nessuna delle tre qui sotto e' un simbolo della mappa di fabbrica. */
static s32 tool_i2c_read(u8 *buf, u16 len)
{
	u16 addr = (buf[0] << 8) + buf[1];

	if (!gt1x_i2c_read(addr, &buf[2], len))
		return 1;
	return -1;
}

static s32 tool_i2c_write(u8 *buf, u16 len)
{
	u16 addr = (buf[0] << 8) + buf[1];

	if (!gt1x_i2c_write(addr, &buf[2], len - 2))
		return 1;
	return -1;
}

static u8 relation(u8 src, u8 dst, u8 rlt)
{
	u8 ret = 0;

	switch (rlt) {
	case 0:
		ret = (src != dst) ? true : false;
		break;

	case 1:
		ret = (src == dst) ? true : false;
		GTP_DEBUG("equal:src:0x%02x   dst:0x%02x   ret:%d.", src, dst, (s32)ret);
		break;

	case 2:
		ret = (src > dst) ? true : false;
		break;

	case 3:
		ret = (src < dst) ? true : false;
		break;

	case 4:
		ret = (src & dst) ? true : false;
		break;

	case 5:
		ret = (!(src | dst)) ? true : false;
		break;

	default:
		ret = false;
		break;
	}

	return ret;
}

/*******************************************************
 *Function:
 *   Comfirm function.
 *Input:
 *   None.
 *Output:
 *   Return write length.
 ********************************************************/
static u8 comfirm(void)
{
	s32 i = 0;
	u8 buf[32];

	memcpy(buf, cmd_head.flag_addr, cmd_head.addr_len);

	for (i = 0; i < cmd_head.times; i++) {
		if (tool_i2c_read(buf, 1) <= 0) {
			GTP_ERROR("Read flag data failed!");
			return -1;
		}
		if (true == relation(buf[GTP_ADDR_LENGTH], cmd_head.flag_val,
				     cmd_head.flag_relation)) {
			GTP_DEBUG("value at flag addr:0x%02x.", buf[GTP_ADDR_LENGTH]);
			GTP_DEBUG("flag value:0x%02x.", cmd_head.flag_val);
			break;
		}

		msleep(cmd_head.circle);
	}

	if (i >= cmd_head.times) {
		GTP_ERROR("Didn't get the flag to continue!");
		return -1;
	}

	return 0;
}

/*******************************************************
 *Function:
 *   Goodix tool write function.
 *Input:
 * standard proc write function param.
 *Output:
 *   Return write length.
 ********************************************************/
static ssize_t gt1x_tool_write(struct file *filp, const char __user *buff,
			       size_t len, loff_t *data)
{
	u64 ret = 0;

	GTP_DEBUG_FUNC();
	ret = copy_from_user(&cmd_head, buff, CMD_HEAD_LENGTH);
	if (ret)
		GTP_ERROR("copy_from_user failed.");

	GTP_DEBUG_ARRAY((u8 *)cmd_head.data, cmd_head.data_len);
	GTP_DEBUG("wr  :0x%02x.", cmd_head.wr);
	/*
	 * GTP_DEBUG("flag:0x%02x.", cmd_head.flag);
	 *  GTP_DEBUG("flag addr:0x%02x%02x.", cmd_head.flag_addr[0],
	 * cmd_head.flag_addr[1]);
	 *  GTP_DEBUG("flag val:0x%02x.", cmd_head.flag_val);
	 *  GTP_DEBUG("flag rel:0x%02x.", cmd_head.flag_relation);
	 *  GTP_DEBUG("circle  :%d.", (s32)cmd_head.circle);
	 *  GTP_DEBUG("times   :%d.", (s32)cmd_head.times);
	 *  GTP_DEBUG("retry   :%d.", (s32)cmd_head.retry);
	 *  GTP_DEBUG("delay   :%d.", (s32)cmd_head.delay);
	 *  GTP_DEBUG("data len:%d.", (s32)cmd_head.data_len);
	 *  GTP_DEBUG("addr len:%d.", (s32)cmd_head.addr_len);
	 *  GTP_DEBUG("addr:0x%02x%02x.", cmd_head.addr[0], cmd_head.addr[1]);
	 *  GTP_DEBUG("len:%d.", (s32)len);
	 *  GTP_DEBUG("buf[20]:0x%02x.", buff[CMD_HEAD_LENGTH]);
	 */
	if (cmd_head.wr == 1) {
		u16 addr, data_len, pos;

		if (cmd_head.flag == 1) {
			if (comfirm()) {
				GTP_ERROR("[WRITE]Comfirm fail!");
				return -1;
			}
		} else if (cmd_head.flag == 2) {
			/*Need interrupt!*/
		}
		addr = (cmd_head.addr[0] << 8) + cmd_head.addr[1];
		data_len = cmd_head.data_len;
		pos = 0;
		while (data_len > 0) {
			len = data_len > DATA_LENGTH ? DATA_LENGTH : data_len;
			ret = copy_from_user(&cmd_head.data[GTP_ADDR_LENGTH],
					     &buff[CMD_HEAD_LENGTH + pos], len);
			if (ret) {
				GTP_ERROR("[WRITE]copy_from_user failed.");
				return -1;
			}
			cmd_head.data[0] = ((addr >> 8) & 0xFF);
			cmd_head.data[1] = (addr & 0xFF);

			GTP_DEBUG_ARRAY(cmd_head.data, len + GTP_ADDR_LENGTH);

			if (tool_i2c_write(cmd_head.data, len + GTP_ADDR_LENGTH) <= 0) {
				GTP_ERROR("[WRITE]Write data failed!");
				return -1;
			}
			addr += len;
			pos += len;
			data_len -= len;
		}

		if (cmd_head.delay)
			msleep(cmd_head.delay);

		return cmd_head.data_len + CMD_HEAD_LENGTH;
	} else if (cmd_head.wr == 3) { /*gt1x unused*/

		memcpy(TP_TYPE, cmd_head.data, cmd_head.data_len);
		return cmd_head.data_len + CMD_HEAD_LENGTH;
	} else if (cmd_head.wr == 5) {

		/*memcpy(TP_TYPE, cmd_head.data, cmd_head.data_len);*/
		return cmd_head.data_len + CMD_HEAD_LENGTH;
	} else if (cmd_head.wr == 7) { /*disable irq!*/
		gt1x_irq_disable();
#ifdef CONFIG_GTP_ESD_PROTECT
		gt1x_esd_switch(SWITCH_OFF);
#endif
		return CMD_HEAD_LENGTH;
	} else if (cmd_head.wr == 9) { /*enable irq!*/
		gt1x_irq_enable();
#ifdef CONFIG_GTP_ESD_PROTECT
		gt1x_esd_switch(SWITCH_ON);
#endif
		return CMD_HEAD_LENGTH;
	} else if (cmd_head.wr == 17) {
		ret = copy_from_user(&cmd_head.data[GTP_ADDR_LENGTH],
				     &buff[CMD_HEAD_LENGTH], cmd_head.data_len);
		if (ret) {
			GTP_ERROR("copy_from_user failed.");
			return -1;
		}

		if (cmd_head.data[GTP_ADDR_LENGTH]) {
			GTP_DEBUG("gtp enter rawdiff.");
			gt1x_rawdiff_mode = true;
		} else {
			gt1x_rawdiff_mode = false;
			GTP_DEBUG("gtp leave rawdiff.");
		}
		return CMD_HEAD_LENGTH;
	} else if (cmd_head.wr == 11) {
		gt1x_enter_update_mode();
	} else if (cmd_head.wr == 13) {
		gt1x_leave_update_mode();
	} else if (cmd_head.wr == 15) {
		struct task_struct *thread = NULL;

		memset(cmd_head.data, 0, cmd_head.data_len + 1);
		memcpy(cmd_head.data, &buff[CMD_HEAD_LENGTH], cmd_head.data_len);
		GTP_DEBUG("update firmware, filename: %s", cmd_head.data);
		thread = kthread_run((int (*)(void *))gt1x_update_firmware,
				     (void *)cmd_head.data, "GT1x FW Update");
		if (IS_ERR(thread))
			return PTR_ERR(thread);
	}
	return CMD_HEAD_LENGTH;
}

static u8 devicecount;
static s32 gt1x_tool_open(struct inode *inode, struct file *file)
{
	if (devicecount > 0)
		return -ERESTARTSYS;
	devicecount++;
	return 0;
}

static s32 gt1x_tool_release(struct inode *inode, struct file *filp)
{
	devicecount--;
	return 0;
}

/*******************************************************
 * Function:
 *   Goodix tool read function.
 *Input:
 * standard proc read function param.
 * Output:
 *   Return read length.
 ********************************************************/
static ssize_t gt1x_tool_read(struct file *filp, char __user *buffer,
			      size_t count, loff_t *ppos)
{
	GTP_DEBUG_FUNC();

	if (*ppos) {
		GTP_DEBUG("[PARAM]size: %zd, *ppos: %d", count, (int)*ppos);
		*ppos = 0;
		return 0;
	}

	if (cmd_head.wr % 2) {
		GTP_ERROR("[READ] invaild operator fail!");
		return -1;
	} else if (!cmd_head.wr) {
		u16 addr, data_len, len, loc;

		/* La fabbrica NON tronca qui cmd_head.data_len a DATA_LENGTH. */
		if (cmd_head.flag == 1) {
			if (comfirm()) {
				GTP_ERROR("[READ]Comfirm fail!");
				return -1;
			}
		} else if (cmd_head.flag == 2) {
			/*Need interrupt!*/
		}

		addr = (cmd_head.addr[0] << 8) + cmd_head.addr[1];
		data_len = cmd_head.data_len;
		loc = 0;

		GTP_DEBUG("[READ] ADDR:0x%04X.", addr);
		GTP_DEBUG("[READ] Length: %d", data_len);

		if (cmd_head.delay)
			msleep(cmd_head.delay);

		while (data_len > 0) {
			len = data_len > DATA_LENGTH ? DATA_LENGTH : data_len;
			cmd_head.data[0] = (addr >> 8) & 0xFF;
			cmd_head.data[1] = (addr & 0xFF);

			if (tool_i2c_read(cmd_head.data, len) <= 0) {
				GTP_ERROR("[READ]Read data failed!");
				return -1;
			}
			memcpy(&buffer[loc], &cmd_head.data[GTP_ADDR_LENGTH], len);
			data_len -= len;
			addr += len;
			loc += len;
			GTP_DEBUG_ARRAY(&cmd_head.data[GTP_ADDR_LENGTH], len);
		}
		*ppos += cmd_head.data_len;
		return cmd_head.data_len;
	} else if (cmd_head.wr == 2) {
		GTP_DEBUG("Return ic type:%s len:%d.", buffer, (s32)cmd_head.data_len);
		return -1;
	} else if (cmd_head.wr == 4) {
		buffer[0] = update_info.progress >> 8;
		buffer[1] = update_info.progress & 0xff;
		buffer[2] = update_info.max_progress >> 8;
		buffer[3] = update_info.max_progress & 0xff;
		*ppos += 4;
		return 4;
	} else if (cmd_head.wr == 6) {
		/*Read error code!*/
		return -1;
	} else if (cmd_head.wr == 8) { /*Read driver version*/
		s32 tmp_len = strlen(GTP_DRIVER_VERSION);

		memcpy(buffer, GTP_DRIVER_VERSION, tmp_len);
		buffer[tmp_len] = 0;
		*ppos += tmp_len + 1;
		return tmp_len + 1;
	}
	*ppos += cmd_head.data_len;
	return cmd_head.data_len;
}

/* ======================================================================
 * LOTTO "tools" -- 2026-08-22.  CORREZIONE di gt1x_tools.c contro l'oracolo,
 * non riscrittura: il file di partenza era la copia dell'albero ALPS
 * (drivers/input/touchscreen/mediatek/GT5688/) portata dentro da un lotto di
 * ricognizione.
 *
 * QUESTO CAPPELLO STA IN FONDO, e non in testa, per una ragione misurata: i
 * printk della fabbrica emettono __LINE__ come immediato, e venticinque di
 * quegli immediati sono nel blocco.  Una riga aggiunta sopra il codice li
 * sposta tutti e venticinque.  Il file e' quindi scritto perche' le righe
 * 87, 90, 97, 100, 141, 185, 190, 191, 199, 222, 225, 247, 261, 270, 306,
 * 311, 315, 327, 365, 371, 379, 390, 391, 402 e 414 cadano dove la fabbrica
 * le ha.  Esempi: "528011a2 mov"@0xffffff8008a76b6c (w2 = 141, la GTP_DEBUG
 * dentro relation, incorporata in comfirm), "52801c22 mov"@0xffffff8008a7667c
 * (w2 = 225), "528033c2 mov"@0xffffff8008a76470 (w2 = 414).
 *
 * Blocco di fabbrica: [0xffffff8008a761a4, 0xffffff8008a76ca8) = 2820 byte,
 * 7 funzioni.  Col compilatore DI FABBRICA (clang r353983c, LLVM 9.0.3):
 * 7 su 7 identiche al byte, 2820 su 2820, e RESIDUO ZERO nel confronto per
 * codifica su tutte e sette (705 istruzioni su 705).
 *
 * PARTENZA: 3712 byte, 2 funzioni su 7 (gt1x_deinit_tool_node e comfirm).
 *
 * ---------------------------------------------------------------------
 * 1. LE DIVERGENZE CORRETTE, E LA PROVA DI CIASCUNA
 * ---------------------------------------------------------------------
 *
 * 1.1  IL rw_mutex NON SI PRENDE MAI.  ALPS incapsula gt1x_tool_read e
 *   gt1x_tool_write fra mutex_lock(&rw_mutex) e mutex_unlock(&rw_mutex), con
 *   una mutex_unlock su ognuno dei rami d'uscita.  Nei 1792 byte delle due
 *   funzioni di fabbrica non c'e' una sola `bl` verso mutex_lock o
 *   mutex_unlock: le uniche chiamate sono printk, comfirm, msleep,
 *   gt1x_i2c_read, gt1x_i2c_write, gt1x_irq_enable, gt1x_irq_disable,
 *   gt1x_enter_update_mode, gt1x_leave_update_mode, kthread_create_on_node,
 *   wake_up_process, __memcpy, __memset, __check_object_size e
 *   __arch_copy_from_user.  Questo, da solo, vale 528 byte su gt1x_tool_read
 *   e 244 su gt1x_tool_write.
 *   La variabile rw_mutex pero' ESISTE: il suo wait_list si vede nel .data
 *   come coppia di puntatori a se stessa, a 0xffffff800998a340 e
 *   0xffffff800998a348, addend R_AARCH64_RELATIVE = 0xffffff800998a340 in
 *   entrambe -- cioe' LIST_HEAD_INIT(rw_mutex.wait_list) con rw_mutex a
 *   0xffffff800998a330.  Ed e' NON static: vedi 3.2.
 *
 * 1.2  gt1x_tool_read NON USA copy_to_user, USA memcpy.  Il ciclo di lettura
 *   chiama "940f85d4 bl"@0xffffff8008a765b0 (__memcpy) senza access_ok e
 *   senza __check_object_size, mentre gt1x_tool_write chiama davvero
 *   "940f822f bl"@0xffffff8008a76804 (__arch_copy_from_user) preceduta dal
 *   confronto con addr_limit.  E' un difetto della fabbrica -- si scrive in
 *   un puntatore utente senza controllarlo -- e si riproduce (regola 7).
 *   Vale anche per il ramo wr==4, che scrive i quattro byte del progresso
 *   direttamente in buffer con strb, e per il ramo wr==8.
 *
 * 1.3  gt1x_tool_read: il ramo wr==8 non alloca niente.  ALPS fa kzalloc +
 *   strncpy + copy_to_user + kfree.  La fabbrica copia i sedici byte della
 *   versione con una sola coppia di registri, "a9402508 ldp"@0xffffff8008a764cc
 *   e "a9002688 stp"@0xffffff8008a764d8, azzera il diciassettesimo con
 *   "3900429f strb"@0xffffff8008a764d0 e ritorna 17 con
 *   "52800220 mov"@0xffffff8008a764d4.  Nessuna GTP_ERROR, nessuna kfree.
 *
 * 1.4  gt1x_tool_read: il ramo wr==4 legge update_info due volte per campo.
 *   "53087d29 lsr"@0xffffff8008a76490 (progress >> 8) e il ricaricamento
 *   "b9400109 ldr"@0xffffff8008a76498 subito dopo dicono che il sorgente
 *   accede a update_info.progress campo per campo e non lo copia in un
 *   locale: il compilatore lo ricarica perche' la strb sul puntatore utente
 *   puo' aliasarlo (classe A2 al contrario).  Ritorna 4 con
 *   "321e03e0 orr"@0xffffff8008a7648c.
 *
 * 1.5  NESSUNA DELLE DUE FUNZIONI TRONCA data_len.  ALPS mette
 *   `if (cmd_head.data_len > DATA_LENGTH) cmd_head.data_len = DATA_LENGTH;`
 *   in testa a entrambe.  Nel binario non c'e': gt1x_tool_read passa da
 *   "f9400064 ldr"@0xffffff8008a76378 (*ppos) a
 *   "3967d108 ldrb"@0xffffff8008a763b0 (cmd_head.wr) senza toccare
 *   cmd_head.data_len, e gt1x_tool_write da "52801c22 mov"@0xffffff8008a7667c
 *   (la GTP_DEBUG di riga 225) direttamente allo switch.  Il troncamento c'e'
 *   solo DENTRO i due cicli, come "1a8ab13b csel"@0xffffff8008a76578
 *   (min con segno fra DATA_LENGTH e data_len).
 *
 * 1.6  gt1x_tool_write NON CONTROLLA che il puntatore cmd_head.data sia
 *   sopravvissuto alla copy_from_user.  ALPS salva pre_data_p/post_data_p e
 *   stampa "pointer is overwritten! %p, %p, %p, %p, %dx".  Quel messaggio non
 *   esiste nel binario di fabbrica e fra
 *   "940f8299 bl"@0xffffff8008a7665c (__arch_copy_from_user, 20 byte) e
 *   "97daf395 bl"@0xffffff8008a76680 (la printk di riga 225) ci sono solo il
 *   controllo dell'esito e il caricamento degli argomenti.
 *
 * 1.7  gt1x_tool_write, ramo wr==15: la fabbrica LANCIA UN THREAD.  ALPS
 *   chiama gt1x_update_firmware in linea e ne guarda il ritorno.  Qui:
 *   "12800002 mov"@0xffffff8008a76788 (w2 = -1 = NUMA_NO_NODE),
 *   "97d988de bl"@0xffffff8008a7678c (kthread_create_on_node),
 *   "b140041f cmn"@0xffffff8008a76790 (il confronto con -4096 di IS_ERR),
 *   "97d9d56c bl"@0xffffff8008a76798 (wake_up_process) -- cioe' kthread_run --
 *   e il nome del thread e' "GT1x FW Update"@0xffffff800924bdb2.
 *   Il ramo IS_ERR salta all'epilogo con x0 ancora uguale al puntatore, cioe'
 *   ritorna PTR_ERR(thread).
 *   Il file di firmware viene azzerato con "940f862f bl"@0xffffff8008a76744
 *   (__memset, data_len+1) e copiato con "940f856b bl"@0xffffff8008a76754
 *   (__memcpy) e NON con copy_from_user: secondo difetto della fabbrica
 *   riprodotto.
 *
 * 1.8  gt1x_tool_open e gt1x_tool_release NON LOGGANO.  36 e 24 byte contro
 *   108 e 92 di ALPS: le quattro GTP_DEBUG non ci sono.  Tutto il corpo di
 *   gt1x_tool_open e' "3968c109 ldrb"@0xffffff8008a76a60,
 *   "34000069 cbz"@0xffffff8008a76a64, "32175be0 orr"@0xffffff8008a76a68
 *   (w0 = 0xfffffe00 = -512 = -ERESTARTSYS), "2a1f03e0 mov"@0xffffff8008a76a70,
 *   "320003e9 orr"@0xffffff8008a76a74 e "3928c109 strb"@0xffffff8008a76a78.
 *   Il `cbz` invece di un `tbz #0` dice che devicecount NON e' ristretta a
 *   0/1 (classe A1): e' un u8 che si decrementa,
 *   "51000529 sub"@0xffffff8008a76a8c.
 *
 * 1.9  I PERMESSI DEL NODO PROC SONO 0666, non 0664:
 *   "528036c1 mov"@0xffffff8008a76274 -- w1 = 0x1b6 = 438 = 0666.  Stessa
 *   correzione che il lotto extents ha fatto su gt1x_init_node.
 *
 * 1.10 I QUATTRO MESSAGGI DI gt1x_init_tool_node SONO ALTRI, e due portano
 *   procname come argomento %s (x3 caricato prima della printk):
 *     "<<GTP-ERR>>[%s:%d] Apply for memory failed.\n"@0xffffff800924b9a7
 *     "<<GTP-INF>>[%s:%d] Alloc memory size:%d.\n"@0xffffff800924b9e8
 *     "<<GTP-ERR>>[%s:%d] CAN't create proc entry /proc/%s.\n"@0xffffff800924a4cc
 *     "<<GTP-INF>>[%s:%d] Created proc entry /proc/%s.\n"@0xffffff800924a511
 *   Gli ultimi due sono gli STESSI byte che usa gt1x_init_node in
 *   gt1x_extents.c: la fabbrica li condivide nel pool di .rodata.
 *   ALPS diceva "Applied memory size:%d.", "Couldn't create proc entry!" e
 *   "Create proc entry success!", tutti e tre assenti dal binario.
 *
 * 1.11 TP_TYPE VALE "GT1X", non "GT9XX": "GT1X"@0xffffff800998a350, che e'
 *   l'array in .data (16 byte, il resto a zero), non la stringa di .rodata.
 *
 * ---------------------------------------------------------------------
 * 2. CIO' CHE ERA GIA' GIUSTO, E CHE E' STATO VERIFICATO PER CODIFICA
 * ---------------------------------------------------------------------
 *
 * gt1x_deinit_tool_node (60 byte) e comfirm (528 byte) misuravano gia'
 * esattamente come di fabbrica.  Non e' stato dato per buono: il confronto
 * per codifica le da' 15 su 15 e 132 su 132 istruzioni, stesso mnemonico in
 * ogni posizione, residuo zero.  E i loro __LINE__ erano SBAGLIATI di due,
 * tre, quattro e cinque righe -- 143/188/194/196/204 invece di
 * 141/185/190/191/199 -- il che la misura di dimensione non poteva vedere.
 *
 * relation e' incorporata in comfirm ma resta una funzione a se' nel
 * sorgente: il __func__ del suo unico printk e' "relation"@0xffffff800924bc73
 * e non "comfirm"@0xffffff800924bba7 (classe B5 letta al contrario).  La sua
 * tabella di salto e' a 0xffffff8008f7f17a, sei byte {0,3,14,17,20,23}
 * relativi a "10000089 adr"@0xffffff8008a76b48: sei casi 0..5, come ALPS.
 *
 * tool_i2c_read e tool_i2c_write sono incorporate anch'esse.  Il loro corpo
 * si riconosce dalla coppia "5ac00908 rev"@0xffffff8008a76b24 +
 * "53107d00 lsr"@0xffffff8008a76b28, che e' come clang rende
 * (buf[0] << 8) + buf[1] su una ldrh.
 *
 * ---------------------------------------------------------------------
 * 3. LA DISPOSIZIONE DI .data, .bss E .rodata, CHE COMBACIA ANCH'ESSA
 * ---------------------------------------------------------------------
 *
 * 3.1  .bss di fabbrica, 0x41 byte: DATA_LENGTH a 0xffffff800a1009f0
 *   ("b909f109 str"@0xffffff8008a76240, spiazzamento 2544), cmd_head a
 *   0xffffff800a1009f4 ("3967d108 ldrb"@0xffffff8008a763b0, 2548, cioe'
 *   cmd_head.wr), procname a 0xffffff800a100a10, gt1x_tool_proc_entry a
 *   0xffffff800a100a28 (2600) e devicecount a 0xffffff800a100a30 (2608).
 *   Il nostro oggetto da' 0x00/0x04/0x20/0x38/0x40 sugli stessi cinque:
 *   stessa successione, stessi scarti, stessa dimensione.
 *   La struttura e' impaccata: "f8014260 stur"@0xffffff8008a761f4 scrive
 *   cmd_head.data a spiazzamento 20, che non e' allineato a 8 -- il
 *   `#pragma pack(1)` della copia e' giusto.  E la memset della struttura e'
 *   di 28 byte: "f900027f str"@0xffffff8008a761e8 (0..7),
 *   "a900fe7f stp"@0xffffff8008a761e4 (8..23) e
 *   "b9001a7f str"@0xffffff8008a761e0 (24..27).
 *
 * 3.2  .data di fabbrica, 0x120 = 288 byte esatti, in tre pezzi consecutivi:
 *   rw_mutex a 0xffffff800998a330 (32 byte), TP_TYPE a 0xffffff800998a350
 *   (16) e gt1x_tool_fops a 0xffffff800998a360 (240, da
 *   "900078a3 adrp"@0xffffff8008a7626c + "910d8063 add"@0xffffff8008a76270).
 *   Da questo scendono DUE correzioni che la dimensione non denuncia:
 *   - gt1x_tool_fops NON e' const.  Nel nostro oggetto la .rodata contiene
 *     SOLO le tre tabelle di salto (32 byte, esattamente quelle che di
 *     fabbrica stanno a 0xffffff8008f7f160): se le fops fossero const
 *     starebbero li' dietro, e invece stanno in mezzo al .data.
 *   - rw_mutex NON e' static.  Le tre variabili con inizializzatore escono
 *     nell'ordine del sorgente se sono globali, mentre clang rimanda in coda
 *     le static: l'ordine misurato rw_mutex, TP_TYPE, gt1x_tool_fops si
 *     ottiene solo con le prime due globali e la terza static.  ALPS scrive
 *     `static DEFINE_MUTEX(rw_mutex)`; DEFINE_MUTEX da sola non e' static.
 *     Verificato: col nostro sorgente il .data misura 0x120, con rw_mutex
 *     static e le fops const misurava 0x30 piu' 0x110 di .rodata.
 *
 * 3.3  Le tre tabelle di salto stanno a 0xffffff8008f7f160 (gt1x_tool_read,
 *   9 byte: {0,27,30,27,40,27,20,27,57}), 0xffffff8008f7f169
 *   (gt1x_tool_write, 17 byte, indice wr-1) e 0xffffff8008f7f17a (relation,
 *   6 byte).  Sono loro a dire QUALI valori di wr esistono: 0,2,4,6,8 in
 *   lettura (le voci dispari puntano tutte al ramo di default) e
 *   1,3,5,7,9,11,13,15,17 in scrittura.
 *
 * ---------------------------------------------------------------------
 * 4. DUE DELTA DI HEADER, RIPORTATI E NON APPLICATI
 * ---------------------------------------------------------------------
 *
 * D1.  gt1x_tpd_common.h:394 dichiara `extern u8 gt1x_rawdiff_mode;` e
 *   gt1x_generic.c:67 la definisce `u8`.  DI FABBRICA E' LARGA QUATTRO BYTE:
 *     "b909b509 str"@0xffffff8008a76844   str  w9, [x8,#2484]
 *     "b909b51f str"@0xffffff8008a76980   str  wzr, [x8,#2484]
 *   e la conferma viene da un'altra unita', tpd_event_handler:
 *     "b949b508 ldr"@0xffffff8008a781c4   ldr  w8, [x8,#2484]
 *   Un u8 non si scrive con una str a 32 bit: sporcherebbe i tre byte
 *   accanto.  Sono le UNICHE due posizioni di tutto il blocco in cui il
 *   nostro mnemonico differisce da quello di fabbrica (strb contro str).
 *   Misurato in isolamento, ricompilando questo stesso file con una copia
 *   dell'header in cui la sola riga 394 dice `extern int`: gt1x_tool_write
 *   resta 1120 byte e passa da 278 a 280 mnemonici uguali su 280, residuo
 *   zero in tutti e due i casi.  La correzione tocca gt1x_generic.c, che e'
 *   di un altro lotto: la lascio al merge.
 *
 * D2.  gt1x_tpd_common.h:115 dice
 *   `#define GTP_DRIVER_VERSION "V1.0<2014/09/28>"`.  Di fabbrica vale
 *   "V1.4<2015/07/10>"@0xffffff800924bb6b.  Non cambia un byte di codice --
 *   le due stringhe sono lunghe uguali (16), l'indirizzo e' un sito di
 *   rilocazione e strlen() e' costante in tutti e due i casi -- ma il nodo
 *   /proc risponderebbe la versione sbagliata.  GTP_DRIVER_VERSION non e'
 *   usata da nessun'altra unita' (verificato con grep sulle sei .c).
 *
 * ---------------------------------------------------------------------
 * 5. CIO' CHE E' SCELTA E NON MISURA
 * ---------------------------------------------------------------------
 *
 * S1.  `static struct st_cmd_head cmd_head;` -- la classe di memoria non si
 *   decide dal binario.  Serve che DATA_LENGTH esca PRIMA di cmd_head nel
 *   .bss, e clang rimanda in coda alla TU sia le definizioni tentative sia
 *   le static: si ottiene lo stesso identico .bss (0x00/0x04/0x20/0x38/0x40)
 *   sia rendendo cmd_head static, sia lasciandola globale e dando a
 *   DATA_LENGTH un inizializzatore esplicito `= 0`.  Provate tutte e due,
 *   danno gli stessi cinque spiazzamenti e le stesse 7 dimensioni su 7.
 *   Ho tenuto la prima.
 *
 * S2.  Il cast `(int (*)(void *))` su gt1x_update_firmware.  Con
 *   `extern int gt1x_update_firmware(char *)` dell'header, kthread_run non
 *   compila (-Werror=incompatible-pointer-types).  Il binario non puo' dire
 *   se di fabbrica ci sia un cast al sito di chiamata o una dichiarazione
 *   diversa nell'header: un cast fra puntatori a funzione non emette una
 *   sola istruzione.  Ho messo il cast perche' non tocca un file di un altro
 *   lotto.
 *
 * S3.  GTP_DEBUG_FUNC() e le due GTP_DEBUG_ARRAY().  Sono no-op
 *   (CONFIG_GTP_DEBUG_FUNC_ON e CONFIG_GTP_DEBUG_ARRAY_ON spenti) e non
 *   emettono niente: il binario non puo' dire se ci siano.  Occupano pero'
 *   righe, e le righe le decide il vincolo dei __LINE__.  Le ho tenute dove
 *   ALPS le aveva o, quando il conto non tornava, nel punto piu' vicino.
 *
 * S4.  Il commento di riga 112 e quello di riga 376 esistono perche' fra due
 *   __LINE__ di fabbrica serviva una riga in piu' di quante il codice ne
 *   occupi.  Dicono un fatto misurato, non un riempimento: le tre funzioni
 *   di 112 non sono simboli della mappa, e il troncamento di cui parla 376
 *   non e' nel binario (vedi 1.5).
 *
 * ---------------------------------------------------------------------
 * 6. IL CONFRONTO PER CODIFICA, FUNZIONE PER FUNZIONE
 * ---------------------------------------------------------------------
 *
 *   funzione                istruz.  mnemonici  riloc/chiam  RESIDUO
 *   gt1x_init_tool_node      95/95      95/95        51         0
 *   gt1x_deinit_tool_node    15/15      15/15         6         0
 *   gt1x_tool_read          168/168    168/168       57         0
 *   gt1x_tool_write         280/280    278/280      102         0
 *   gt1x_tool_open            9/9        9/9          3         0
 *   gt1x_tool_release         6/6        6/6          3         0
 *   comfirm                 132/132    132/132       38         0
 *   TOTALE                  705/705    703/705      260         0
 *
 * Le due posizioni con mnemonico diverso sono quelle del delta D1, e con
 * l'header corretto diventano 705 su 705.
 *
 * ---------------------------------------------------------------------
 * 7. I VENTOTTO LETTERALI DEL FILE, UNO PER UNO
 * ---------------------------------------------------------------------
 *
 * I due che entrano nel binario da soli, e che quindi si verificano:
 *   "%d.%d.%d"@0xffffff800924ba12
 *   "gmnode%02d%02d%02d"@0xffffff800924ba1b
 *   "GT1X"@0xffffff800998a350          (l'array TP_TYPE, in .data)
 *   "GT1x FW Update"@0xffffff800924bdb2 (il nome del kthread)
 *
 * I ventiquattro messaggi non entrano MAI nel binario come stringa a se':
 * le macro GTP_INFO/GTP_ERROR/GTP_DEBUG li saldano dentro un unico letterale
 * insieme alla testa "<<GTP-xxx>>[%s:%d]" e alla "\n" finale (vedi il DELTA
 * DI STRUMENTO in fondo).  Sono citati per quel che il binario contiene
 * davvero, byte per byte, NUL compreso:
 *   "<<GTP-ERR>>[%s:%d] Apply for memory failed.\n"@0xffffff800924b9a7
 *   "<<GTP-INF>>[%s:%d] Alloc memory size:%d.\n"@0xffffff800924b9e8
 *   "<<GTP-ERR>>[%s:%d] CAN't create proc entry /proc/%s.\n"@0xffffff800924a4cc
 *   "<<GTP-INF>>[%s:%d] Created proc entry /proc/%s.\n"@0xffffff800924a511
 *   "<<GTP-DBG>>[%s:%d]equal:src:0x%02x   dst:0x%02x   ret:%d.\n"@0xffffff800924bc38
 *   "<<GTP-ERR>>[%s:%d] Read flag data failed!\n"@0xffffff800924bb7c
 *   "<<GTP-DBG>>[%s:%d]value at flag addr:0x%02x.\n"@0xffffff800924bbaf
 *   "<<GTP-DBG>>[%s:%d]flag value:0x%02x.\n"@0xffffff800924bbdd
 *   "<<GTP-ERR>>[%s:%d] Didn't get the flag to continue!\n"@0xffffff800924bc03
 *   "<<GTP-ERR>>[%s:%d] copy_from_user failed.\n"@0xffffff800924a632
 *   "<<GTP-DBG>>[%s:%d]wr  :0x%02x.\n"@0xffffff800924bc8c
 *   "<<GTP-ERR>>[%s:%d] [WRITE]Comfirm fail!\n"@0xffffff800924bcac
 *   "<<GTP-ERR>>[%s:%d] [WRITE]copy_from_user failed.\n"@0xffffff800924bcd5
 *   "<<GTP-ERR>>[%s:%d] [WRITE]Write data failed!\n"@0xffffff800924bd07
 *   "<<GTP-DBG>>[%s:%d]gtp enter rawdiff.\n"@0xffffff800924bd35
 *   "<<GTP-DBG>>[%s:%d]gtp leave rawdiff.\n"@0xffffff800924bd5b
 *   "<<GTP-DBG>>[%s:%d]update firmware, filename: %s\n"@0xffffff800924bd81
 *   "<<GTP-DBG>>[%s:%d][PARAM]size: %zd, *ppos: %d\n"@0xffffff800924ba2e
 *   "<<GTP-ERR>>[%s:%d] [READ] invaild operator fail!\n"@0xffffff800924ba6c
 *   "<<GTP-ERR>>[%s:%d] [READ]Comfirm fail!\n"@0xffffff800924ba9e
 *   "<<GTP-DBG>>[%s:%d][READ] ADDR:0x%04X.\n"@0xffffff800924bac6
 *   "<<GTP-DBG>>[%s:%d][READ] Length: %d\n"@0xffffff800924baed
 *   "<<GTP-ERR>>[%s:%d] [READ]Read data failed!\n"@0xffffff800924bb12
 *   "<<GTP-DBG>>[%s:%d]Return ic type:%s len:%d.\n"@0xffffff800924bb3e
 *
 * "%zd" NON e' un refuso: la fabbrica stampa count come size_t e *ppos con
 * "%d" -- che su una loff_t e' un difetto della fabbrica, riprodotto.
 *
 * I due __func__ che il blocco emette e che nel sorgente non sono letterali:
 *   "comfirm"@0xffffff800924bba7 e "relation"@0xffffff800924bc73.
 *
 * DELTA DI STRUMENTO, riportato e non applicato -- lo stesso che il lotto
 * extents ha gia' documentato.  Le ventiquattro citazioni di messaggio qui
 * sopra escono NON_ANCORATA da verificacitazioni.py: il ramo "messaggio
 * assemblato dalla macro di log" e' aperto da RE_TESTA_ASSEMBLATA, che
 * riconosce solo il prefisso KERN_SOH (\x01 + cifra), e le macro Goodix
 * fanno printk("<<GTP-DBG>>[%s:%d]" fmt "\n", ...) senza livello KERN_*.
 * I byte a quegli indirizzi sono comunque esattamente quelli scritti, letti
 * uno per uno con leggi_stringa.py.  Il rimedio sarebbe allargare
 * RE_TESTA_ASSEMBLATA alla famiglia <<GTP-xxx>>[%s:%d]; non l'ho fatto
 * perche' lo strumento non e' mio.
 * ====================================================================== */
