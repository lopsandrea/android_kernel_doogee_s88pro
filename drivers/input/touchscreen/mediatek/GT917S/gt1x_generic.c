/* drivers/input/touchscreen/gt1x_generic.c
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
 * Revision RecordL:
 *      V1.0:  first release. 2014/09/28.
 *
 */

#include <linux/fs.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "gt1x_config.h"
#include "include/gt1x_tpd_common.h"

#ifdef CONFIG_GTP_PROXIMITY
#include <linux/hwmsen_dev.h>
#include <linux/hwmsensor.h>
#include <linux/sensors_io.h>
#endif

#ifdef CONFIG_GTP_ICS_SLOT_REPORT
#include <linux/input/mt.h>
#endif

/* LE CITAZIONI DEI LETTERALI DI QUESTA UNITA'.
 * Forma verificabile "testo"@0xINDIRIZZO: il testo e' quello ASSEMBLATO
 * dalla macro (prefisso "<<GTP-xxx>>[%s:%d]" + formato + "\n"), perche' e'
 * quello che esiste nel binario -- il letterale nudo del codice non ci
 * compare mai.  verificacitazioni.py non riconosce quella testa (il suo
 * RE_TESTA_ASSEMBLATA apre solo sul prefisso KERN_SOH) e le segna
 * NON_ANCORATA: e' un limite dello strumento, gia' riportato dal lotto
 * extents, non un difetto delle citazioni -- i byte a quegli indirizzi sono
 * esattamente quelli scritti, NUL compreso.
 *
 * TRE dei tredici strcmp di gt1x_debug_write_proc NON hanno nessuna stringa
 * nel binario, e non e' una svista: clang li ha ridotti a un confronto con
 * una costante immediata, perche' il testo con il NUL sta in quattro o otto
 * byte.  "int" e' "52883c80"-style: "528dcd29 mov"@0xffffff8008a75e14 +
 * "72a00e89 movk"@0xffffff8008a75e18 danno 0x0074_6e69 = 'i','n','t',NUL,
 * confrontati con "b94133e8 ldr"@0xffffff8008a75e10 (quattro byte di
 * mode_str).  "poweron" e "version" allo stesso modo su otto byte
 * ("d28dee09 mov"@0xffffff8008a75e28 e "d28caec9 mov"@0xffffff8008a75e5c).
 * Percio' non sono citate: nel binario non c'e' nessun indirizzo da citare.
 * Le sette voci che il codice usa SOLO sotto #ifdef spenti ("NEAR", "on",
 * "off", "enable", "disable", "fail", "success", tutte in
 * gt1x_ps_operate/gt1x_report_ps sotto CONFIG_GTP_PROXIMITY) NON sono citate:
 * i byte che si trovano in giro per il binario sono di ALTRI driver, e un
 * indirizzo cosi' non prova niente.  Stanno fra le eccezioni.
 *
 * "<<GTP-ERR>>[%s:%d] Create proc entry /proc/%s FAILED!\n"@0xffffff800924b6d1
 * "<<GTP-INF>>[%s:%d] Created proc entry /proc/%s.\n"@0xffffff800924a511
 * "==== GT1X default config setting in driver====\n"@0xffffff800924b71d
 * "0x%02X,"@0xffffff800924b74d
 * "\n"@0xffffff8009245994 -- 48916 occorrenze nel binario: DEBOLE, non
 *   prova l'indirizzo da sola.  Quello che gt1x_parse_config passa a printk
 *   e' pero' misurato: "91265000 add"@0xffffff8008a73b40 (x0 = 0x9245994).
 * "==== GT1X config read from chip====\n"@0xffffff800924b755
 * "<<GTP-INF>>[%s:%d] I2C TRANSFER: %d\n"@0xffffff800924b77a
 * "==== GT1X Version Info ====\n"@0xffffff800924b7b4
 * "ProductID: GT%c%c%c%c\n"@0xffffff800924b7d1
 * "PatchID: %02X%02X\n"@0xffffff800924b7e8
 * "MaskID: %02X%02X\n"@0xffffff800924b7fb
 * "SensorID: %02X\n"@0xffffff800924b80d
 * "<<GTP-DBG>>[%s:%d]write count %ld\n\n"@0xffffff800924b81d
 * "<<GTP-ERR>>[%s:%d] Too much data, buffer size: %d, data:%ld\n"@0xffffff800924b857
 * "<<GTP-ERR>>[%s:%d] copy from user fail!\n"@0xffffff800924b894
 * "%s %d"@0xffffff8009263d94
 * "clear_config"@0xffffff800924b8bd
 * "<<GTP-INF>>[%s:%d] Force clear gt1x_config\n"@0xffffff800924b8ca
 * "init"@0xffffff8009271d89 -- 523 occorrenze nel binario, DEBOLE per il
 *   conteggio; l'indirizzo pero' e' MISURATO, non scelto: e' quello che
 *   "91362421 add"@0xffffff8008a75de4 mette in x1 per __pi_memcmp.
 * "<<GTP-INF>>[%s:%d] Init panel\n"@0xffffff800924b8f6
 * "chip"@0xffffff800911c38b -- 6 occorrenze, indirizzo misurato da
 *   "910e2c21 add"@0xffffff8008a75dfc.
 * "<<GTP-INF>>[%s:%d] Get chip type:\n"@0xffffff800924b915
 * "<<GTP-INF>>[%s:%d] Disable irq.\n"@0xffffff800924b938
 * "<<GTP-INF>>[%s:%d] Enable irq.\n"@0xffffff800924b959
 * "poweroff"@0xffffff80090e3dce -- 7 occorrenze, indirizzo misurato da
 *   "91373821 add"@0xffffff8008a75e44.
 * "reset"@0xffffff8009232c75 -- 86 occorrenze, indirizzo misurato da
 *   "9131d421 add"@0xffffff8008a75e78.
 * "%s %s"@0xffffff8009140a04 -- 6 occorrenze, indirizzo misurato da
 *   "91281021 add"@0xffffff8008a75e90.
 * "update"@0xffffff80091d4fcf -- 57 occorrenze, indirizzo misurato da
 *   "913f3c21 add"@0xffffff8008a75ea8.
 * "sendconfig"@0xffffff800924b981
 * "debug_gesture"@0xffffff800924b98c
 * "force_update"@0xffffff800924b99a
 * "<<GTP-ERR>>[%s:%d] Open config file error!(file: %s)\n"@0xffffff800924ab1f
 * "<<GTP-ERR>>[%s:%d] Config is invalid!(length: %d)\n"@0xffffff800924ab67
 * "<<GTP-ERR>>[%s:%d] Allocate memory failed!(size: %d)\n"@0xffffff800924ab9a
 * "<<GTP-ERR>>[%s:%d] Read %d bytes from file failed!\n"@0xffffff800924abd0
 * "<<GTP-INF>>[%s:%d] Parse config file: %s (%d bytes)\n"@0xffffff800924ac04
 * "\n<<GTP-DBG>>:"@0xffffff800924ac62
 * "0x%02x,"@0xffffff800924ac70
 * "<<GTP-ERR>>[%s:%d] Illegal config file!\n"@0xffffff800924ac39
 * "<<GTP-ERR>>[%s:%d] I2c Transfer error! (%d)\n"@0xffffff800924ac78
 * "<<GTP-ERR>>[%s:%d] I2c transfer error! (%d)\n"@0xffffff800924acb2
 * "<<GTP-ERR>>[%s:%d] Hardware Info:%08X\n"@0xffffff800924b67d
 * "<<GTP-ERR>>[%s:%d] I2c failed%d.\n"@0xffffff800924b6a4
 * "<<GTP-ERR>>[%s:%d] i2c_read_dbl_check length %d is too long, exceed %zu\n"@0xffffff800924aced
 * "<<GTP-ERR>>[%s:%d] i2c read 0x%04X, %d bytes, double check failed!\n"@0xffffff800924ad4e
 * "<<GTP-INF>>[%s:%d] X_MAX = %d, Y_MAX = %d, TRIGGER = 0x%02x\n"@0xffffff800924ad92
 * "<<GTP-INF>>[%s:%d] X_MAX=%d,Y_MAX=%d,TRIGGER=0x%02x,WAKEUP_LEVEL=%d\n"@0xffffff800924addd
 * "<<GTP-DBG>>[%s:%d]Set reset status.\n"@0xffffff800924b52a
 * "<<GTP-INF>>[%s:%d] GTP RESET!\n"@0xffffff800924ae32
 * "<<GTP-ERR>>[%s:%d] Read version failed!(checksum error)\n"@0xffffff800924ae63
 * "<<GTP-ERR>>[%s:%d] Read version failed!\n"@0xffffff800924aeae
 * "<<GTP-DBG>>[%s:%d]Read version : %d\n"@0xffffff800924aed7
 * "<<GTP-INF>>[%s:%d] IC VERSION:GT%s_%06X(Patch)_%04X(Mask)_%02X(SensorID)\n"@0xffffff800924aefc
 * "<<GTP-ERR>>[%s:%d] I2c communication error.\n"@0xffffff800924af46
 * "<<GTP-INF>>[%s:%d] Chip Type: %s\n"@0xffffff800924af86
 * "GT1X"@0xffffff800924a83f -- 2 occorrenze
 * "GT2X"@0xffffff800924afa8
 * "<<GTP-INF>>[%s:%d] Enter sleep mode!\n"@0xffffff800924b565
 * "<<GTP-ERR>>[%s:%d] Enter sleep mode failed.\n"@0xffffff800924b59c
 * "<<GTP-DBG>>[%s:%d]Wake up begin.\n"@0xffffff800924b5c9
 * "<<GTP-ERR>>[%s:%d] Wake up sleep failed.\n"@0xffffff800924b5fd
 * "<<GTP-INF>>[%s:%d] Wake up end.\n"@0xffffff800924b627
 * "<<GTP-INF>>[%s:%d] force_reset_guitar\n"@0xffffff800924afad
 * "<<GTP-ERR>>[%s:%d] I2C transfer error. errno:%d\n"@0xffffff800924afe5
 * "<<GTP-DBG>>[%s:%d]Request state:0x%02x.\n"@0xffffff800924b031
 * "<<GTP-INF>>[%s:%d] Request Config.\n"@0xffffff800924b05a
 * "<<GTP-INF>>[%s:%d] Send gt1x_config success.\n"@0xffffff800924b07e
 * "<<GTP-INF>>[%s:%d] Request Reset.\n"@0xffffff800924b0ac
 * "<<GTP-INF>>[%s:%d] Request Ref.\n"@0xffffff800924b0cf
 * "<<GTP-INF>>[%s:%d] Request main clock.\n"@0xffffff800924b0f0
 * "<<GTP-ERR>>[%s:%d] Illegal finger number!\n"@0xffffff800924b118
 * "<<GTP-ERR>>[%s:%d] Checksum error[%x]\n"@0xffffff800924b15c
 * "<<GTP-DBG>>[%s:%d]--lan-- Key Down.\n"@0xffffff800924b183
 * "<<GTP-DBG>>[%s:%d]--lan-- Key Up.\n"@0xffffff800924b1a8
 * "<<GTP-DBG>>[%s:%d](%d)(%d,%d)[%d]\n"@0xffffff800924b1cb
 * "<<GTP-DBG>>[%s:%d]Released Touch.\n"@0xffffff800924b1ee
 * "<<GTP-DBG>>[%s:%d]Additional Int Pulse.\n"@0xffffff800924b211
 * "<<GTP-INF>>[%s:%d] Suspend start...\n"@0xffffff800924b23a
 * "<<GTP-DBG>>[%s:%d]0x81AA: 0x%02X\n"@0xffffff800924b26c
 * "<<GTP-INF>>[%s:%d] hotknot is paired!\n"@0xffffff800924b28e
 * "<<GTP-ERR>>[%s:%d] Suspend failed.\n"@0xffffff800924b2b5
 * "<<GTP-INF>>[%s:%d] Suspend end...\n"@0xffffff800924b2d9
 * "<<GTP-INF>>[%s:%d] Resume start...\n"@0xffffff800924b2fc
 * "<<GTP-ERR>>[%s:%d] Resume failed.\n"@0xffffff800924b32c
 * "<<GTP-DBG>>[%s:%d]Resume end.\n"@0xffffff800924b34f
 * "<<GTP-ERR>>[%s:%d] Reset guitar failed!\n"@0xffffff800924b36e
 * "<<GTP-ERR>>[%s:%d] Get verision failed!\n"@0xffffff800924b3a1
 * "<<GTP-ERR>>[%s:%d] chip is not gt5xxx or gt9xx.\n"@0xffffff800924b3ca
 * "<<GTP-ERR>>[%s:%d] Check main system not pass[0x%2X].\n"@0xffffff800924b3fb
 * "<<GTP-ERR>>[%s:%d] Check subsystem not pass[0x%2X].\n"@0xffffff800924b432
 * "<<GTP-ERR>>[%s:%d] Init failed, use default setting\n"@0xffffff800924b467
 * "<<GTP-ERR>>[%s:%d] Get chip type failed!\n"@0xffffff800924b49c
 * "<<GTP-ERR>>[%s:%d] Init panel failed.\n"@0xffffff800924b4c6
 * "gt1x_workthread"@0xffffff800924b4ed
 * "<<GTP-ERR>>[%s:%d] Create workqueue failed!\n"@0xffffff800924b4fd
 */

/*******************GLOBAL VARIABLE*********************/
/* g1008a0 -- u8 a 0xffffff800a1008a0, otto byte prima di gt1x_i2c_client
 * ("f9445508 ldr"@0xffffff8008a74170 legge [x8,#2216] = 0xffffff800a1008a8).
 * Il binario non la nomina.  Cio' che di lei e' MISURATO, tutto in
 * gt1x_touch_event_handler: vale 1 mentre un dito e' giu'
 * ("3922811c strb"@0xffffff8008a74b48, w28 = 1) e 0 quando il tocco e'
 * rilasciato ("3922811f strb"@0xffffff8008a74b0c, wzr); se vale non-zero il
 * ramo dei tasti esce subito con 0 ("34000069 cbz"@0xffffff8008a74a3c).
 * NON e' `static`: una `static u8` che assume solo 0 e 1 clang la
 * restringerebbe a `tbz #0`, e qui la lettura e' `ldrb` + `cbz` a byte pieno.
 */
u8 g1008a0;
struct i2c_client *gt1x_i2c_client;
static struct workqueue_struct *gt1x_workqueue;

u8 gt1x_config[GTP_CONFIG_MAX_LENGTH] = {0};

u32 gt1x_cfg_length = GTP_CONFIG_MAX_LENGTH;
bool check_flag;

enum CHIP_TYPE_T gt1x_chip_type = CHIP_TYPE_NONE;
struct gt1x_version_info gt1x_version = {.product_id = {0},
					 .patch_id = 0,
					 .mask_id = 0,
					 .sensor_id = 0,
					 .match_opt = 0};

#if defined(CONFIG_GTP_WITH_STYLUS) && defined(CONFIG_GTP_HAVE_STYLUS_KEY)
const u16 gt1x_stylus_key_array[] = GTP_STYLUS_KEY_TAB;
#endif

u8 gt1x_clk_buf[6];
u8 gt1x_clk_retries;
u8 gt1x_ref_retries;
u8 gt1x_driver_num;
u8 gt1x_sensor_num;

u8 gt1x_int_type;
u8 gt1x_wakeup_level;
u32 gt1x_abs_x_max;
u32 gt1x_abs_y_max;
u8 gt1x_rawdiff_mode;

u8 gt1x_init_failed;

u8 is_resetting;

static ssize_t gt1x_debug_read_proc(struct file *, char __user *, size_t,
				    loff_t *);
static ssize_t gt1x_debug_write_proc(struct file *, const char __user *, size_t,
				     loff_t *);

static struct proc_dir_entry *gt1x_debug_proc_entry;

/* DELTA DI HEADER: `gt1x_deinit_node` e' definita non-static in
 * gt1x_extents.c ma `include/gt1x_tpd_common.h` dichiara solo la sua
 * gemella `gt1x_init_node` (riga 314).  Di fabbrica `gt1x_deinit`
 * la chiama -- "97fff4b2 bl"@0xffffff8008a75904 punta a
 * 0xffffff8008a72bcc, che nella mappa e' `gt1x_deinit_node`.  La
 * dichiarazione va nell'header, ed e' lavoro del lotto di merge:
 * qui sta in locale per non toccare un file condiviso.
 */
extern void gt1x_deinit_node(void);

/* DELTA DI HEADER: `tpd_halt` e' definita in gt1x_tpd.c (riga 49) e
 * `include/gt1x_tpd_common.h` esporta solo la funzione che la legge
 * (`gt1x_is_tpd_halt`, riga 364).  Di fabbrica gt1x_suspend e gt1x_resume
 * la SCRIVONO direttamente: "b909ce68 str"@0xffffff8008a74d68 (w8 = 1) e
 * "b909cd1f str"@0xffffff8008a75290 (wzr), entrambe a [x8,#2508] cioe'
 * 0xffffff800a1009cc, lo stesso indirizzo che tpd_off scrive.
 * La dichiarazione va nell'header, ed e' lavoro del lotto di merge.
 */
extern int tpd_halt;

/* DELTA DI HEADER: `gt1x_gesture_debug` e' definita non-static in gt1x_wtk.c
 * (di fabbrica a 0xffffff8008a72a38) e l'header non la dichiara.  Di fabbrica
 * gt1x_debug_write_proc la chiama: "97fff2cd bl"@0xffffff8008a75f04 punta
 * proprio a 0xffffff8008a72a38.  La dichiarazione va nell'header.
 */
extern void gt1x_gesture_debug(int on);

/* g0fc0b8 -- u8 ESTERNO a questo driver, a 0xffffff800a1008b8... no:
 * a 0xffffff800a0fc0b8.  Il binario NON lo nomina e nessuna sezione della
 * mappa lo copre (stock.map ha solo simboli di funzione), quindi il nome
 * qui e' derivato dall'indirizzo e non e' un fatto.
 * Cio' che di lui e' MISURATO -- SEI letture in tutta l'immagine, tutte
 * `ldrb` seguito da `cbz`/`cbnz`, mai una scrittura:
 *   "3942e108 ldrb"@0xffffff80087fca6c  (lcm_suspend)
 *   "3942e108 ldrb"@0xffffff80087fd858  (lcm_suspend, secondo ramo)
 *   "3942e108 ldrb"@0xffffff8008a74d74  (gt1x_suspend)
 *   "3942e108 ldrb"@0xffffff8008a75008  (gt1x_resume)
 *   "3942e108 ldrb"@0xffffff8008a75ef0  (gt1x_debug_write_proc)
 *   "3942e108 ldrb"@0xffffff8008a78028  (tpd_event_handler)
 * Il byte che lo segue, 0xffffff800a0fc0b9, e' il buffer che
 * `fix_tp_proc_info` riempie con __memcpy: sono nello stesso oggetto,
 * che non e' il Goodix.  Restera' IRRISOLTO, ed e' dichiarato.
 *
 * (Questa nota diceva CINQUE e ne elencava SEI. Rifatta la scansione su
 * tutte le 4.875.132 istruzioni disassemblate dell'immagine, gli accessi a
 * 0xffffff800a0fc0b8 sono sei, esattamente i sei elencati: il numero era
 * sbagliato e le citazioni erano gia' tutte giuste.)
 */
extern u8 g0fc0b8;

/* update_info_c16 -- DELTA DI HEADER.  Di fabbrica `struct fw_update_info`
 * ha un `int` in PIU' rispetto ad ALPS, a offset 16, e per lui il campo
 * `firmware` slitta da 16 a 24.  Misurato sull'intera immagine:
 *   offset 24 e' `firmware`  -- "f9438d28 ldr"@0xffffff8008a79458 in
 *     gt1x_update_judge, che poi legge [x8,#12] e [x8,#13], cioe' i campi
 *     di `struct fw_info` che ALPS legge da `update_info.firmware`;
 *   offset 16 e' il campo NUOVO, scritto solo qui
 *     ("b9071128 str"@0xffffff8008a75f30) e letto solo da gt1x_update_judge
 *     ("b9471129 ldr"@0xffffff8008a79638), dove decide se stampare
 *     e aggiornare comunque quando le versioni non lo giustificherebbero.
 * Il binario NON lo nomina: qui e' `c16`, l'offset, e non un nome inventato.
 * L'aggiunta del campo all'header e' lavoro del lotto di merge; finche' non
 * c'e', l'accesso e' scritto per offset -- e' la STESSA parola di memoria,
 * non un ripiego semantico.
 */
#define update_info_c16 (*(int *)((char *)&update_info + 16))

static const struct file_operations gt1x_debug_fops = {
	.owner = THIS_MODULE,
	.read = gt1x_debug_read_proc,
	.write = gt1x_debug_write_proc,
};

s32 gt1x_init_debug_node(void)
{
	gt1x_debug_proc_entry =
		proc_create(GT1X_DEBUG_PROC_FILE, 0660, NULL, &gt1x_debug_fops);
	if (gt1x_debug_proc_entry == NULL) {
		GTP_ERROR("Create proc entry /proc/%s FAILED!",
#line 95
			  GT1X_DEBUG_PROC_FILE);
		return -1;
	}
	GTP_INFO("Created proc entry /proc/%s.", GT1X_DEBUG_PROC_FILE);
	return 0;
}

void gt1x_deinit_debug_node(void)
{
	if (gt1x_debug_proc_entry != NULL)
		remove_proc_entry(GT1X_DEBUG_PROC_FILE, NULL);
}

static ssize_t gt1x_debug_read_proc(struct file *file, char __user *page,
				    size_t size, loff_t *ppos)
{
	char *ptr = (char *)page;
	char temp_data[GTP_CONFIG_MAX_LENGTH] = {0};
	int i;
	ssize_t ret = 0;

	if (*ppos)
		return 0;

	ptr += sprintf(ptr,
		       "==== GT1X default config setting in driver====\n");

	for (i = 0; i < GTP_CONFIG_MAX_LENGTH; i++) {
		ptr += sprintf(ptr, "0x%02X,", gt1x_config[i]);
		if (i % 10 == 9)
			ptr += sprintf(ptr, "\n");
	}

	ptr += sprintf(ptr, "\n");

	ptr += sprintf(ptr, "==== GT1X config read from chip====\n");
	i = gt1x_i2c_read(GTP_REG_CONFIG_DATA, temp_data,
			  GTP_CONFIG_MAX_LENGTH);
#line 131
	GTP_INFO("I2C TRANSFER: %d", i);
	for (i = 0; i < GTP_CONFIG_MAX_LENGTH; i++) {
		ptr += sprintf(ptr, "0x%02X,", temp_data[i]);

		if (i % 10 == 9)
			ptr += sprintf(ptr, "\n");
	}

	/* Touch PID & VID */
	ptr += sprintf(ptr, "\n");
	ptr += sprintf(ptr, "==== GT1X Version Info ====\n");

	gt1x_i2c_read(GTP_REG_VERSION, temp_data, 12);
	ptr += sprintf(ptr, "ProductID: GT%c%c%c%c\n", temp_data[0],
		       temp_data[1], temp_data[2], temp_data[3]);
	ptr += sprintf(ptr, "PatchID: %02X%02X\n", temp_data[4], temp_data[5]);
	ptr += sprintf(ptr, "MaskID: %02X%02X\n", temp_data[7], temp_data[8]);
	ptr += sprintf(ptr, "SensorID: %02X\n", temp_data[10] & 0x0F);

	ret = ptr - (char *)page;
	*ppos += ret;

	return ret;
}

static ssize_t gt1x_debug_write_proc(struct file *file,
				     const char __user *buffer, size_t count,
				     loff_t *ppos)
{
	u8 buf[GTP_CONFIG_MAX_LENGTH] = {0};
	char mode_str[50] = {0};
	int mode;
	char arg1[50] = {0};
	u8 cfg[GTP_CONFIG_MAX_LENGTH] = {0};

#line 163
	GTP_DEBUG("write count %ld\n", (unsigned long)count);

	if (count > GTP_CONFIG_MAX_LENGTH) {
		GTP_ERROR("Too much data, buffer size: %d, data:%ld",
#line 166
			  GTP_CONFIG_MAX_LENGTH, (unsigned long)count);
		return -EFAULT;
	}

	if (copy_from_user(buf, buffer, count)) {
		GTP_ERROR("copy from user fail!");
		return -EFAULT;
	}

	/*send config*/
	if (count == gt1x_cfg_length) {
		memcpy(gt1x_config, buf, count);
		gt1x_abs_x_max = (gt1x_config[RESOLUTION_LOC + 1] << 8) +
				 gt1x_config[RESOLUTION_LOC];
		gt1x_abs_y_max = (gt1x_config[RESOLUTION_LOC + 3] << 8) +
				 gt1x_config[RESOLUTION_LOC + 2];
		return count;
	}

	sscanf(buf, "%s %d", (char *)&mode_str, &mode);

	/*force clear gt1x_config*/
	if (strcmp(mode_str, "clear_config") == 0) {
#line 192
		GTP_INFO("Force clear gt1x_config");
		gt1x_send_cmd(GTP_CMD_CLEAR_CFG, 0);
		return count;
	}
	if (strcmp(mode_str, "init") == 0) {
		GTP_INFO("Init panel");
		gt1x_init_panel();
		return count;
	}
	if (strcmp(mode_str, "chip") == 0) {
		GTP_INFO("Get chip type:");
		gt1x_get_chip_type();
		return count;
	}
	if (strcmp(mode_str, "int") == 0) {
		if (mode == 0) {
			GTP_INFO("Disable irq.");
			gt1x_irq_disable();
		} else {
			GTP_INFO("Enable irq.");
			gt1x_irq_enable();
		}
		return count;
	}

	if (strcmp(mode_str, "poweron") == 0) {
		gt1x_power_switch(1);
		return count;
	}

	if (strcmp(mode_str, "poweroff") == 0) {
		gt1x_power_switch(0);
		return count;
	}

	if (strcmp(mode_str, "version") == 0) {
		gt1x_read_version(NULL);
		return count;
	}

	if (strcmp(mode_str, "reset") == 0) {
		gt1x_irq_disable();
		gt1x_reset_guitar();
		gt1x_irq_enable();
		return count;
	}

	sscanf(buf, "%s %s", (char *)&mode_str, (char *)&arg1);

	if (strcmp(mode_str, "update") == 0) {
		gt1x_update_firmware(arg1);
		return count;
	}
	if (strcmp(mode_str, "sendconfig") == 0) {
		if (gt1x_parse_config(arg1, cfg) < 0)
			return -1;
		return count;
	}
	if (strcmp(mode_str, "debug_gesture") == 0) {
		if (g0fc0b8)
			gt1x_gesture_debug(mode != 0);
	}
	if (strcmp(mode_str, "force_update") == 0)
		update_info_c16 = (mode != 0);

	return gt1x_debug_proc(buf, count);
}

/* gt1x_char2hex -- di fabbrica NON ha simbolo proprio: e' incorporata due
 * volte dentro gt1x_parse_config (0xffffff8008a73a18 e 0xffffff8008a73a50,
 * stessa successione di sei istruzioni).  Il valore d'errore e' 0xFF, e lo
 * prova il controllo del chiamante, che nega e maschera a otto bit:
 *   "2a2c03ee mvn"@0xffffff8008a73a80 seguita da
 *   "72001ddf tst"@0xffffff8008a73a84  (tst w14, #0xff).
 */
static u8 gt1x_char2hex(u8 c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return 0xFF;
}

int gt1x_parse_config(char *filename, u8 *config)
{
	mm_segment_t old_fs;
	struct file *fp = NULL;
	u8 *buf = NULL;
	int len = 0;
	int i = 0;
	int j = 0;
	u8 high = 0;
	u8 low = 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(fp)) {
#line 302
		GTP_ERROR("Open config file error!(file: %s)", filename);
		j = -1;
		goto exit_fs;
	}

	len = fp->f_op->llseek(fp, 0, SEEK_END);
	if (len < GTP_CONFIG_MAX_LENGTH ||
	    len > GTP_CONFIG_MAX_LENGTH * 6) {
#line 307
		GTP_ERROR("Config is invalid!(length: %d)", len);
		j = -1;
		goto exit_close;
	}

	buf = kzalloc(len, GFP_KERNEL);
	if (buf == NULL) {
#line 312
		GTP_ERROR("Allocate memory failed!(size: %d)", len);
		j = -1;
		goto exit_close;
	}

	fp->f_op->llseek(fp, 0, SEEK_SET);
	if (fp->f_op->read(fp, (char *)buf, len, &fp->f_pos) != len)
#line 317
		GTP_ERROR("Read %d bytes from file failed!", len);

#line 320
	GTP_INFO("Parse config file: %s (%d bytes)", filename, len);

	while (i < len) {
		if (buf[i] == ' ' || buf[i] == ',' || buf[i] == '\r' ||
		    buf[i] == '\n') {
			i++;
			continue;
		}
		if (buf[i] != '0')
			goto illegal;
		if (buf[i + 1] != 'x' && buf[i + 1] != 'X')
			goto illegal;

		high = gt1x_char2hex(buf[i + 2]);
		low = gt1x_char2hex(buf[i + 3]);
		if (high == 0xFF || low == 0xFF)
			goto illegal;

		config[j++] = (high << 4) + low;
		if (j > GTP_CONFIG_MAX_LENGTH - 1)
			break;
		i += 4;
	}

	if (j < GTP_CONFIG_MIN_LENGTH || config[j - 1] != 1) {
		j = -1;
		goto exit_free;
	}

	for (i = 0; i < j; i++) {
		if (i % 10 == 0)
			printk("\n<<GTP-DBG>>:");
		printk("0x%02x,", config[i]);
	}
	printk("\n");
	goto exit_free;

illegal:
#line 338
	GTP_ERROR("Illegal config file!");
	j = -1;
exit_free:
	kfree(buf);
exit_close:
	filp_close(fp, NULL);
exit_fs:
	set_fs(old_fs);
	return j;
}

s32 _do_i2c_read(struct i2c_msg *msgs, u16 addr, u8 *buffer, s32 len)
{
	s32 ret = -1;
	s32 pos = 0;
	s32 data_length = len;
	s32 transfer_length = 0;
	u8 *data = NULL;
	u16 address = addr;

	data = kmalloc(IIC_MAX_TRANSFER_SIZE < (len + GTP_ADDR_LENGTH)
			       ? IIC_MAX_TRANSFER_SIZE
			       : (len + GTP_ADDR_LENGTH),
		       GFP_KERNEL);
	if (data == NULL)
		return ERROR_MEM;
	msgs[1].buf = data;

	while (pos != data_length) {
		if ((data_length - pos) > IIC_MAX_TRANSFER_SIZE)
			transfer_length = IIC_MAX_TRANSFER_SIZE;
		else
			transfer_length = data_length - pos;
		msgs[0].buf[0] = (address >> 8) & 0xFF;
		msgs[0].buf[1] = address & 0xFF;
		msgs[1].len = transfer_length;

		ret = i2c_transfer(gt1x_i2c_client->adapter, msgs, 2);
		if (ret != 2) {
#line 391
			GTP_ERROR("I2c Transfer error! (%d)", ret);
			kfree(data);
			return ERROR_IIC;
		}
		memcpy(&buffer[pos], msgs[1].buf, transfer_length);
		pos += transfer_length;
		address += transfer_length;
	}

	kfree(data);
	return 0;
}

s32 _do_i2c_write(struct i2c_msg *msg, u16 addr, u8 *buffer, s32 len)
{
	s32 ret = -1;
	s32 pos = 0;
	s32 data_length = len;
	s32 transfer_length = 0;
	u8 *data = NULL;
	u16 address = addr;

	data = kmalloc(IIC_MAX_TRANSFER_SIZE < (len + GTP_ADDR_LENGTH)
			       ? IIC_MAX_TRANSFER_SIZE
			       : (len + GTP_ADDR_LENGTH),
		       GFP_KERNEL);
	if (data == NULL)
		return ERROR_MEM;

	msg->buf = data;

	while (pos != data_length) {
		if ((data_length - pos) >
		    (IIC_MAX_TRANSFER_SIZE - GTP_ADDR_LENGTH))
			transfer_length =
				IIC_MAX_TRANSFER_SIZE - GTP_ADDR_LENGTH;
		else
			transfer_length = data_length - pos;

		msg->buf[0] = (address >> 8) & 0xFF;
		msg->buf[1] = address & 0xFF;
		msg->len = transfer_length + GTP_ADDR_LENGTH;
		memcpy(&msg->buf[GTP_ADDR_LENGTH], &buffer[pos],
		       transfer_length);

		ret = i2c_transfer(gt1x_i2c_client->adapter, msg, 1);
		if (ret != 1) {
#line 433
			GTP_ERROR("I2c transfer error! (%d)", ret);
			kfree(data);
			return ERROR_IIC;
		}
		pos += transfer_length;
		address += transfer_length;
	}

	kfree(data);
	return 0;
}

/* DI FABBRICA E' `static`, e la prova e' doppia: (a) il simbolo non esiste in
 * NESSUN punto della mappa dell'intera immagine, (b) il suo corpo compare
 * incorporato nel chiamante con il PROPRIO __func__.  `static` qui non si
 * puo' scrivere: `include/gt1x_tpd_common.h` la dichiara `extern` e
 * gt1x_tpd.c la chiama, e quell'header e' condiviso -- la modifica e' un
 * DELTA DI HEADER, lavoro del lotto di merge.  L'attributo qui sotto NON
 * genera codice: riproduce la sola decisione dell'inliner che `static`
 * prenderebbe.  Misurato: con l'attributo gt1x_suspend fa 732 e gt1x_resume
 * 816 (esatti); senza, 340 e 312.
 */
__attribute__((always_inline)) s32 gt1x_i2c_test(void)
{
	u8 retry = 0;
	s32 ret = -1;
	u32 hw_info = 0;

	GTP_DEBUG_FUNC();

	while (retry++ < 3) {
		ret = gt1x_i2c_read(GTP_REG_HW_INFO, (u8 *)&hw_info,
				    sizeof(hw_info));
		if (!ret) {
#line 456
			GTP_INFO("Hardware Info:%08X", hw_info);
			return ret;
		}

		msleep(10);
		GTP_ERROR("Hardware Info:%08X", hw_info);
		GTP_ERROR("I2c failed%d.", retry);
	}

	return ERROR_RETRY;
}

s32 gt1x_i2c_read_dbl_check(u16 addr, u8 *buffer, s32 len)
{
	u8 buf[16] = {0};
	u8 confirm_buf[16] = {0};
	s32 ret = 0;

	if (len > 16) {
		GTP_ERROR(
			"i2c_read_dbl_check length %d is too long, exceed %zu",
#line 483
			len, sizeof(buf));
		return ERROR_IIC;
	}

	memset(buf, 0xAA, 16);
	ret = gt1x_i2c_read(addr, buf, len);
	if (ret < 0)
		return ret;

	msleep(5);

	memset(confirm_buf, 0, 16);
	ret = gt1x_i2c_read(addr, confirm_buf, len);
	if (ret < 0)
		return ret;

	if (!memcmp(buf, confirm_buf, len)) {
		memcpy(buffer, confirm_buf, len);
		return 0;
	}
#line 504
	GTP_ERROR("i2c read 0x%04X, %d bytes, double check failed!", addr, len);
	return 1;
}

/**
 * gt1x_get_info - Get information from ic, such as resolution and
 * int trigger type
 * Return    <0: i2c failed, 0: i2c ok
 */
s32 gt1x_get_info(void)
{
	u8 opr_buf[4] = {0};
	s32 ret = 0;

	ret = gt1x_i2c_read(GTP_REG_CONFIG_DATA + 1, opr_buf, 4);
	if (ret < 0)
		return ret;

	gt1x_abs_x_max = (opr_buf[1] << 8) + opr_buf[0];
	gt1x_abs_y_max = (opr_buf[3] << 8) + opr_buf[2];

	ret = gt1x_i2c_read(GTP_REG_CONFIG_DATA + 6, opr_buf, 1);
	if (ret < 0)
		return ret;
	gt1x_int_type = opr_buf[0] & 0x03;

	GTP_INFO("X_MAX = %d, Y_MAX = %d, TRIGGER = 0x%02x", gt1x_abs_x_max,
#line 532
		 gt1x_abs_y_max, gt1x_int_type);

	return 0;
}

/**
 * gt1x_send_cfg - Send gt1x_config Function.
 * @config: pointer of the configuration array.
 * @cfg_len: length of configuration array.
 * Return 0--success,non-0--fail.
 */
s32 gt1x_send_cfg(u8 *config, int cfg_len)
{
#ifdef CONFIG_GTP_DRIVER_SEND_CFG
	int i;
	s32 ret = 0;
	s32 retry = 0;
	u16 checksum = 0;

	GTP_DEBUG("Driver Send Config, length: %d", cfg_len);
	for (i = 0; i < cfg_len - 3; i += 2)
		checksum += (config[i] << 8) + config[i + 1];
	if (!checksum) {
		GTP_ERROR("Invalid config, all of the bytes is zero!");
		return -1;
	}
	checksum = 0 - checksum;
	GTP_DEBUG("Config checksum: 0x%04X", checksum);
	config[cfg_len - 3] = (checksum >> 8) & 0xFF;
	config[cfg_len - 2] = checksum & 0xFF;
	config[cfg_len - 1] = 0x01;

	while (retry++ < 5) {
		ret = gt1x_i2c_write(GTP_REG_CONFIG_DATA, config, cfg_len);
		if (!ret) {
			msleep(
				200);
/* must 200ms, wait for storing config into flash. */
			GTP_DEBUG("Send config successfully!");
			return 0;
		}
	}
	GTP_ERROR("Send config failed!");
	return ret;
#endif
	return 0;
}

/* `gt1x_sensor_id_check` (GT5688/gt1x_generic.c:489) NON C'E' PIU', ed e' una
 * SCELTA dichiarata, non una misura.  In ALPS il suo unico chiamante sta
 * dentro #ifdef CONFIG_GTP_DRIVER_SEND_CFG, che il binario impone SPENTA
 * (`gt1x_send_cfg` di fabbrica e' otto byte: "2a1f03e0 mov"@0xffffff8008a74040
 * seguita da "d65f03c0 ret"@0xffffff8008a74044).  Con la CONFIG spenta resta
 * inutilizzata e il -Werror si ferma su «unused function».
 * LA CONSEGNA DI QUESTO LOTTO CHIEDEVA di verificare se i 548 byte mancanti a
 * gt1x_init venissero da lei: NO.  Vengono da due funzioni incorporate --
 * `gt1x_get_chip_type` (__func__ "gt1x_get_chip_type" a
 * "913dcc21 add"@0xffffff8008a756f8, righe 922 e 934) e
 * `gt1x_init_debug_node` (__func__ "gt1x_init_debug_node" a
 * "911c2021 add"@0xffffff8008a7587c, righe 95 e 98) -- e dal ciclo di
 * ritentativi srotolato cinque volte invece di tre.  Chiuso il conto,
 * gt1x_init misura 1528 su 1528 byte.
 *
 * QUESTA NOTA DICEVA «con RESIDUO ZERO di codifica», E NON ERA VERO.
 * Quando e' stata scritta, gt1x_init aveva DICIANNOVE posizioni non
 * classificate, tutte valori di __LINE__ (le righe 2361/2367/2388/2396/2405
 * di fabbrica contro le nostre, e due __func__): la dimensione tornava e il
 * confronto per istruzione no. Il file non aveva nessuna direttiva #line, e
 * le note in testa spostano centinaia di righe.
 * Ora che tools/derivaline.py ricava le #line dal binario la funzione e' a
 * UNA posizione da leggere (347 istruzioni uguali su 382), ma il numero
 * pubblicato era comunque piu' forte della misura che lo sorreggeva.
 * Una `static` mai chiamata non entra nel binario in nessun caso: il binario
 * non puo' dire se il sorgente di fabbrica la contenga.
 */

s32 gt1x_init_panel(void)
{
	s32 ret = 0;
	u8 cfg_len = 0;

#ifdef CONFIG_GTP_DRIVER_SEND_CFG
	u8 sensor_id = 0;

	const u8 cfg_grp0[] = GTP_CFG_GROUP0;
	const u8 cfg_grp1[] = GTP_CFG_GROUP1;
	const u8 cfg_grp2[] = GTP_CFG_GROUP2;
	const u8 cfg_grp3[] = GTP_CFG_GROUP3;
	const u8 cfg_grp4[] = GTP_CFG_GROUP4;
	const u8 cfg_grp5[] = GTP_CFG_GROUP5;
	const u8 *cfgs[] = {cfg_grp0, cfg_grp1, cfg_grp2,
			    cfg_grp3, cfg_grp4, cfg_grp5};
	u8 cfg_lens[] = {CFG_GROUP_LEN(cfg_grp0), CFG_GROUP_LEN(cfg_grp1),
			 CFG_GROUP_LEN(cfg_grp2), CFG_GROUP_LEN(cfg_grp3),
			 CFG_GROUP_LEN(cfg_grp4), CFG_GROUP_LEN(cfg_grp5)};

#ifdef CONFIG_GTP_CHARGER_SWITCH
	const u8 cfg_grp0_charger[] = GTP_CFG_GROUP0_CHARGER;
	const u8 cfg_grp1_charger[] = GTP_CFG_GROUP1_CHARGER;
	const u8 cfg_grp2_charger[] = GTP_CFG_GROUP2_CHARGER;
	const u8 cfg_grp3_charger[] = GTP_CFG_GROUP3_CHARGER;
	const u8 cfg_grp4_charger[] = GTP_CFG_GROUP4_CHARGER;
	const u8 cfg_grp5_charger[] = GTP_CFG_GROUP5_CHARGER;
	const u8 *cfgs_charger[] = {cfg_grp0_charger, cfg_grp1_charger,
				    cfg_grp2_charger, cfg_grp3_charger,
				    cfg_grp4_charger, cfg_grp5_charger};
	u8 cfg_lens_charger[] = {CFG_GROUP_LEN(cfg_grp0_charger),
				 CFG_GROUP_LEN(cfg_grp1_charger),
				 CFG_GROUP_LEN(cfg_grp2_charger),
				 CFG_GROUP_LEN(cfg_grp3_charger),
				 CFG_GROUP_LEN(cfg_grp4_charger),
				 CFG_GROUP_LEN(cfg_grp5_charger)};
#endif /* end  CONFIG_GTP_CHARGER_SWITCH */

	GTP_DEBUG("Config Groups Length: %d, %d, %d, %d, %d, %d", cfg_lens[0],
		  cfg_lens[1], cfg_lens[2], cfg_lens[3], cfg_lens[4],
		  cfg_lens[5]);

	gt1x_sensor_id_check(&sensor_id);
	sensor_id = sensor_id & 0x0f;

	if (sensor_id >= 6 || cfg_lens[sensor_id] < GTP_CONFIG_MIN_LENGTH ||
	    cfg_lens[sensor_id] > GTP_CONFIG_MAX_LENGTH) {
		sensor_id = 0;
	}

	cfg_len = cfg_lens[sensor_id];

	GTP_INFO("CTP_CONFIG_GROUP%d used, gt1x_config length: %d", sensor_id,
		 cfg_len);

	if (cfg_len < GTP_CONFIG_MIN_LENGTH ||
	    cfg_len > GTP_CONFIG_MAX_LENGTH) {
		GTP_ERROR(
			"CTP_CONFIG_GROUP%d is INVALID CONFIG GROUP! NO Config Sent! ;"
			"You need to check you header file CFG_GROUP section!",
			sensor_id + 1);
		return -1;
	}

	memset(gt1x_config, 0, sizeof(gt1x_config));
	memcpy(gt1x_config, cfgs[sensor_id], cfg_len);

/* clear the flag, avoid failure when send the_config of driver. */
/* gt1x_config[0] &= 0x7F; */

#ifdef CONFIG_GTP_CUSTOM_CFG
	gt1x_config[RESOLUTION_LOC] = (u8)tpd_dts_data.tpd_resolution[0];
	gt1x_config[RESOLUTION_LOC + 1] =
		(u8)(tpd_dts_data.tpd_resolution[0] >> 8);
	gt1x_config[RESOLUTION_LOC + 2] = (u8)tpd_dts_data.tpd_resolution[1];
	gt1x_config[RESOLUTION_LOC + 3] =
		(u8)(tpd_dts_data.tpd_resolution[1] >> 8);

	GTP_INFO("Res: %d * %d, trigger: %d", tpd_dts_data.tpd_resolution[0],
		 tpd_dts_data.tpd_resolution[1], GTP_INT_TRIGGER);

	if (GTP_INT_TRIGGER == 0) { /* RISING  */
		gt1x_config[TRIGGER_LOC] &= 0xfe;
	} else if (GTP_INT_TRIGGER == 1) { /* FALLING */
		gt1x_config[TRIGGER_LOC] |= 0x01;
	}
#endif /* END CONFIG_GTP_CUSTOM_CFG */

#ifdef CONFIG_GTP_CHARGER_SWITCH
	GTP_DEBUG("Charger Config Groups Length: %d, %d, %d, %d, %d, %d",
		  cfg_lens_charger[0], cfg_lens_charger[1], cfg_lens_charger[2],
		  cfg_lens_charger[3], cfg_lens_charger[4],
		  cfg_lens_charger[5]);

	memset(gt1x_config_charger, 0, sizeof(gt1x_config_charger));
	if (cfg_lens_charger[sensor_id] == cfg_len)
		memcpy(gt1x_config_charger, cfgs_charger[sensor_id], cfg_len);

	/* clear the flag, avoid failure when send the config of driver. */
	gt1x_config_charger[0] &= 0x7F;

#ifdef CONFIG_GTP_CUSTOM_CFG
	gt1x_config_charger[RESOLUTION_LOC] =
		(u8)tpd_dts_data.tpd_resolution[0];
	gt1x_config_charger[RESOLUTION_LOC + 1] =
		(u8)(tpd_dts_data.tpd_resolution[0] >> 8);
	gt1x_config_charger[RESOLUTION_LOC + 2] =
		(u8)tpd_dts_data.tpd_resolution[1];
	gt1x_config_charger[RESOLUTION_LOC + 3] =
		(u8)(tpd_dts_data.tpd_resolution[1] >> 8);

	if (GTP_INT_TRIGGER == 0) { /* RISING  */
		gt1x_config_charger[TRIGGER_LOC] &= 0xfe;
	} else if (GTP_INT_TRIGGER == 1) { /* FALLING */
		gt1x_config_charger[TRIGGER_LOC] |= 0x01;
	}
#endif /* END CONFIG_GTP_CUSTOM_CFG */
	if (cfg_lens_charger[sensor_id] != cfg_len)
		memset(gt1x_config_charger, 0, sizeof(gt1x_config_charger));
#endif /* END CONFIG_GTP_CHARGER_SWITCH */

#else  /* DRIVER NOT SEND CONFIG */
	cfg_len = GTP_CONFIG_MAX_LENGTH;
	ret = gt1x_i2c_read(GTP_REG_CONFIG_DATA, gt1x_config, cfg_len);
	if (ret < 0)
		return ret;
#endif /* END CONFIG_GTP_DRIVER_SEND_CFG */

	GTP_DEBUG_FUNC();
	/* match resolution when gt1x_abs_x_max & */
	/* gt1x_abs_y_max have been set already */
	if ((gt1x_abs_x_max == 0) && (gt1x_abs_y_max == 0)) {
		gt1x_abs_x_max = (gt1x_config[RESOLUTION_LOC + 1] << 8) +
				 gt1x_config[RESOLUTION_LOC];
		gt1x_abs_y_max = (gt1x_config[RESOLUTION_LOC + 3] << 8) +
				 gt1x_config[RESOLUTION_LOC + 2];
		gt1x_int_type = (gt1x_config[TRIGGER_LOC]) & 0x03;
		gt1x_wakeup_level = !(gt1x_config[MODULE_SWITCH3_LOC] & 0x20);
	} else {
		gt1x_config[RESOLUTION_LOC] = (u8)gt1x_abs_x_max;
		gt1x_config[RESOLUTION_LOC + 1] = (u8)(gt1x_abs_x_max >> 8);
		gt1x_config[RESOLUTION_LOC + 2] = (u8)gt1x_abs_y_max;
		gt1x_config[RESOLUTION_LOC + 3] = (u8)(gt1x_abs_y_max >> 8);
		gt1x_config[MODULE_SWITCH3_LOC] =
			(gt1x_config[MODULE_SWITCH3_LOC] & 0xDF) |
			((!gt1x_wakeup_level) << 5);
		gt1x_config[TRIGGER_LOC] =
			(gt1x_config[TRIGGER_LOC] & 0xFC) | gt1x_int_type;
#ifdef CONFIG_GTP_CHARGER_SWITCH
		gt1x_config_charger[RESOLUTION_LOC] = (u8)gt1x_abs_x_max;
		gt1x_config_charger[RESOLUTION_LOC + 1] =
			(u8)(gt1x_abs_x_max >> 8);
		gt1x_config_charger[RESOLUTION_LOC + 2] = (u8)gt1x_abs_y_max;
		gt1x_config_charger[RESOLUTION_LOC + 3] =
			(u8)(gt1x_abs_y_max >> 8);
		set_reg_bit(gt1x_config[MODULE_SWITCH3_LOC], 5,
			    !gt1x_wakeup_level);
		gt1x_config[TRIGGER_LOC] =
			(gt1x_config[TRIGGER_LOC] & 0xFC) | gt1x_int_type;
#endif
	}

	GTP_INFO("X_MAX=%d,Y_MAX=%d,TRIGGER=0x%02x,WAKEUP_LEVEL=%d",
		 gt1x_abs_x_max, gt1x_abs_y_max, gt1x_int_type,
#line 683
		 gt1x_wakeup_level);

	gt1x_cfg_length = cfg_len;
	ret = gt1x_send_cfg(gt1x_config, gt1x_cfg_length);
	return ret;
}

void gt1x_select_addr(void)
{
	GTP_GPIO_OUTPUT(GTP_RST_PORT, 0);
	GTP_GPIO_OUTPUT(GTP_INT_PORT, gt1x_i2c_client->addr == 0x14);
	msleep(2);
	GTP_GPIO_OUTPUT(GTP_RST_PORT, 1);
	msleep(2);
}

static s32 gt1x_set_reset_status(void)
{
	s32 ret = 0;
	u8 value[4] = {0xAA, 0x00, 0x56, 0xAA};

#line 705
	GTP_DEBUG("Set reset status.");

	ret = gt1x_i2c_write(GTP_REG_CMD + 1, &value[1], 3);
	if (ret < 0)
		return ret;

	return gt1x_i2c_write(GTP_REG_CMD, value, 1);
}

s32 gt1x_reset_guitar(void)
{
	s32 ret = 0;

#line 815
	GTP_INFO("GTP RESET!");

	/* select i2c address */
	gt1x_select_addr();
	msleep(8);

	/* int synchronization */
	GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
	msleep(50);
	GTP_GPIO_AS_INT(GTP_INT_PORT);

	ret = gt1x_set_reset_status();
	return ret;
}

/**
 * gt1x_read_version - Read gt1x version info.
 * @ver_info: address to store version info
 * Return 0-succeed.
 */
s32 gt1x_read_version(struct gt1x_version_info *ver_info)
{
	s32 ret = -1;
	u8 buf[12] = {0};
	u32 mask_id = 0;
	u32 patch_id = 0;
	u8 product_id[5] = {0};
	u8 sensor_id = 0;
	u8 match_opt = 0;
	int i, retry = 3;
	u8 checksum = 0;

	GTP_DEBUG_FUNC();

	while (retry--) {
		ret = gt1x_i2c_read_dbl_check(GTP_REG_VERSION, buf,
					      sizeof(buf));
		if (!ret) {
			checksum = 0;

			for (i = 0; i < sizeof(buf); i++)
				checksum += buf[i];

			if (checksum == 0 &&
	/* first 3 bytes must be number or char */
			    /*sensor id == 0xFF, retry */
			    IS_NUM_OR_CHAR(buf[0]) &&
			    IS_NUM_OR_CHAR(buf[1]) && IS_NUM_OR_CHAR(buf[2]) &&
			    buf[10] != 0xFF) {
				break;
			}
#line 869
			GTP_ERROR("Read version failed!(checksum error)");
		} else {
#line 872
			GTP_ERROR("Read version failed!");
		}
		GTP_DEBUG("Read version : %d", retry);
		msleep(100);
	}

	/* DIFETTO TROVATO E NON APPLICATO, e la ragione e' misurata.
	 * Di fabbrica questo ramo azzera anche sensor_id:
	 *   "b4000053 cbz"@0xffffff8008a74430  (ver_info != NULL)
	 *   "3900367f strb"@0xffffff8008a74434 (wzr in [x19,#13], cioe'
	 *                                       ver_info->sensor_id, offset 13
	 *                                       nella struct impacchettata)
	 * cioe'  if (ver_info != NULL) ver_info->sensor_id = 0;
	 * Scritto DA SOLO costa due istruzioni e porta la funzione da 576 a 584:
	 * misurato, non dedotto.  Vuol dire che in questa funzione le divergenze
	 * sono ALMENO DUE, e la seconda e' il telaio di pila -- di fabbrica
	 * "d101c3ff sub"@0xffffff8008a742b4 riserva 0x70, il nostro 0x80, sedici
	 * byte in piu' di locali, e da li' discende lo slittamento di TUTTI i
	 * numeri di registro (residuo 71 su 144 istruzioni).  gt1x_read_version
	 * non e' nel mandato di questo lotto e misurava 576 su 576 gia' prima:
	 * la correzione parziale la peggiorerebbe, quindi e' DICHIARATA e non
	 * applicata.
	 */
	if (retry <= 0)
		return -1;

	mask_id = (u32)((buf[7] << 16) | (buf[8] << 8) | buf[9]);
	patch_id = (u32)((buf[4] << 16) | (buf[5] << 8) | buf[6]);
	memcpy(product_id, buf, 4);
	sensor_id = buf[10] & 0x0F;
	match_opt = (buf[10] >> 4) & 0x0F;

	GTP_INFO("IC VERSION:GT%s_%06X(Patch)_%04X(Mask)_%02X(SensorID)",
#line 890
		 product_id, patch_id >> 8, mask_id >> 8, sensor_id);

	if (ver_info != NULL) {
		ver_info->mask_id = mask_id;
		ver_info->patch_id = patch_id;
		memcpy(ver_info->product_id, product_id, 5);
		ver_info->sensor_id = sensor_id;
		ver_info->match_opt = match_opt;
	}
	return 0;
}

/**
 * gt1x_get_chip_type - get chip type .
 *
 * different chip synchronize in different way,
 */
s32 gt1x_get_chip_type(void)
{
	u8 opr_buf[4] = {0x00};
	u8 gt1x_data[] = {0x02, 0x08, 0x90, 0x00};
	u8 gt9l_data[] = {0x01, 0x10, 0x90, 0x00};
	s32 ret = -1;

	/* chip type already exist */
	if (gt1x_chip_type != CHIP_TYPE_NONE)
		return 0;

	/* read hardware */
	ret = gt1x_i2c_read_dbl_check(GTP_REG_HW_INFO, opr_buf,
				      sizeof(opr_buf));
	if (ret) {
		GTP_ERROR("I2c communication error.");
		return -1;
	}

	/* find chip type */
	if (!memcmp(opr_buf, gt1x_data, sizeof(gt1x_data)))
		gt1x_chip_type = CHIP_TYPE_GT1X;
	else if (!memcmp(opr_buf, gt9l_data, sizeof(gt9l_data)))
		gt1x_chip_type = CHIP_TYPE_GT2X;

	if (gt1x_chip_type != CHIP_TYPE_NONE) {
		GTP_INFO("Chip Type: %s",
			 (gt1x_chip_type == CHIP_TYPE_GT1X) ? "GT1X" : "GT2X");
		return 0;
	} else {
		return -1;
	}
}

/**
 * gt1x_enter_sleep - Eter sleep function.
 *
 * Returns  0--success,non-0--fail.
 */
/* DI FABBRICA E' `static`, e la prova e' doppia: (a) il simbolo non esiste in
 * NESSUN punto della mappa dell'intera immagine, (b) il suo corpo compare
 * incorporato nel chiamante con il PROPRIO __func__.  `static` qui non si
 * puo' scrivere: `include/gt1x_tpd_common.h` la dichiara `extern` e
 * gt1x_tpd.c la chiama, e quell'header e' condiviso -- la modifica e' un
 * DELTA DI HEADER, lavoro del lotto di merge.  L'attributo qui sotto NON
 * genera codice: riproduce la sola decisione dell'inliner che `static`
 * prenderebbe.  Misurato: con l'attributo gt1x_suspend fa 732 e gt1x_resume
 * 816 (esatti); senza, 340 e 312.
 */
__attribute__((always_inline)) s32 gt1x_enter_sleep(void)
{
	s32 retry = 0;

	if (gt1x_wakeup_level == 1)
		GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
	msleep(5);

	while (retry++ < 3) {
		if (!gt1x_send_cmd(GTP_CMD_SLEEP, 0)) {
#line 961
			GTP_INFO("Enter sleep mode!");
			return 0;
		}
		msleep(10);
	}

	GTP_ERROR("Enter sleep mode failed.");
	return -1;
}

/* DI FABBRICA E' `static`, e la prova e' doppia: (a) il simbolo non esiste in
 * NESSUN punto della mappa dell'intera immagine, (b) il suo corpo compare
 * incorporato nel chiamante con il PROPRIO __func__.  `static` qui non si
 * puo' scrivere: `include/gt1x_tpd_common.h` la dichiara `extern` e
 * gt1x_tpd.c la chiama, e quell'header e' condiviso -- la modifica e' un
 * DELTA DI HEADER, lavoro del lotto di merge.  L'attributo qui sotto NON
 * genera codice: riproduce la sola decisione dell'inliner che `static`
 * prenderebbe.  Misurato: con l'attributo gt1x_suspend fa 732 e gt1x_resume
 * 816 (esatti); senza, 340 e 312.
 */
__attribute__((always_inline)) s32 gt1x_wakeup_sleep(void)
{
	u8 retry = 0;
	s32 ret = -1;
	int i2c_path = 0;

#line 986
	GTP_DEBUG("Wake up begin.");
	gt1x_irq_disable();

	while (retry++ < 2) {
		if (gesture_enabled) {
			gesture_doze_status = DOZE_DISABLED;
			ret = gt1x_reset_guitar();
			if (!ret)
				break;
		} else {
			/* wake up through int port */
			GTP_GPIO_OUTPUT(GTP_INT_PORT, gt1x_wakeup_level);
			msleep(5);

			/* Synchronize int IO */
			GTP_GPIO_OUTPUT(GTP_INT_PORT, 0);
			msleep(50);
			GTP_GPIO_AS_INT(GTP_INT_PORT);

			/* test i2c */
			ret = gt1x_i2c_test();
			if (!ret)
				break;
			i2c_path = 1;
		}
	}

	if (ret < 0 && i2c_path) {
		/* wakeup failed , try waking up by resetting */
		while (retry--) {
			ret = gt1x_reset_guitar();
			if (!ret)
				break;
		}
	}

	if (ret) {
#line 1035
		GTP_ERROR("Wake up sleep failed.");
		return -1;
	}
	GTP_INFO("Wake up end.");
	return 0;
}

s32 gt1x_send_cmd(u8 cmd, u8 data)
{
	s32 ret;
	static DEFINE_MUTEX(cmd_mutex);
	u8 buffer[3] = {cmd, data, 0};

	mutex_lock(&cmd_mutex);
	buffer[2] = (u8)((0 - cmd - data) & 0xFF);
	ret = gt1x_i2c_write(GTP_REG_CMD + 1, &buffer[1], 2);
	ret |= gt1x_i2c_write(GTP_REG_CMD, &buffer[0], 1);
	msleep(50);
	mutex_unlock(&cmd_mutex);

	return ret;
}

/**
 * gt1x_power_reset2 - compare with gt1x_power_reset(), remove irq operation
 * additional irq operation may lead to flow: enable->irq(disable)->enable
 * if irq and second enable are very close, it could lead to touch hang
 */

void gt1x_power_reset2(void)
{
	s32 i = 0;
	s32 ret = 0;

	if (is_resetting || update_info.status)
		return;
	GTP_INFO("force_reset_guitar");
	is_resetting = 1;
	gt1x_power_switch(SWITCH_OFF);
	msleep(30);
	gt1x_power_switch(SWITCH_ON);
	msleep(30);

	for (i = 0; i < 5; i++) {
		ret = gt1x_reset_guitar();
		if (ret < 0)
			continue;
		ret = gt1x_send_cfg(gt1x_config, gt1x_cfg_length);
		if (ret < 0) {
			msleep(500);
			continue;
		}
		break;
	}
	is_resetting = 0;
}

void gt1x_power_reset(void)
{
	s32 i = 0;
	s32 ret = 0;

	if (is_resetting || update_info.status)
		return;
	GTP_INFO("force_reset_guitar");
	is_resetting = 1;
	gt1x_irq_disable();
	gt1x_power_switch(SWITCH_OFF);
	msleep(30);
	gt1x_power_switch(SWITCH_ON);
	msleep(30);

	for (i = 0; i < 5; i++) {
		ret = gt1x_reset_guitar();
		if (ret < 0)
			continue;
		ret = gt1x_send_cfg(gt1x_config, gt1x_cfg_length);
		if (ret < 0) {
			msleep(500);
			continue;
		}
		break;
	}
	gt1x_irq_enable();
	is_resetting = 0;
}

s32 gt1x_request_event_handler(void)
{
	s32 ret = -1;
	u8 rqst_data = 0;

	ret = gt1x_i2c_read(GTP_REG_RQST, &rqst_data, 1);
	if (ret) {
#line 1106
		GTP_ERROR("I2C transfer error. errno:%d", ret);
		return -1;
	}
	GTP_DEBUG("Request state:0x%02x.", rqst_data);
	switch (rqst_data & 0x0F) {
	case GTP_RQST_CONFIG:
		GTP_INFO("Request Config.");
		ret = gt1x_send_cfg(gt1x_config, gt1x_cfg_length);
		if (ret) {
			GTP_ERROR("Send gt1x_config error.");
		} else {
			GTP_INFO("Send gt1x_config success.");
			rqst_data = GTP_RQST_RESPONDED;
			gt1x_i2c_write(GTP_REG_RQST, &rqst_data, 1);
		}
		break;
	case GTP_RQST_RESET:
		GTP_INFO("Request Reset.");
		gt1x_reset_guitar();
		rqst_data = GTP_RQST_RESPONDED;
		gt1x_i2c_write(GTP_REG_RQST, &rqst_data, 1);
		break;
	case GTP_RQST_BAK_REF:
		GTP_INFO("Request Ref.");
		break;
	case GTP_RQST_MAIN_CLOCK:
		GTP_INFO("Request main clock.");
		break;
#if 0
#ifdef CONFIG_GTP_HOTKNOT
	case GTP_RQST_HOTKNOT_CODE:
		GTP_INFO("Request HotKnot Code.");
		break;
#endif
#endif
	default:
		break;
	}
	return 0;
}

/**
 * gt1x_touch_event_handler - handle touch event
 * (pen event, key event, finger touch envent)
 * @data:
 * Return    <0: failed, 0: succeed
 */
s32 gt1x_touch_event_handler(u8 *data, struct input_dev *dev,
			     struct input_dev *pen_dev)
{
	/* r8f7f054 -- di fabbrica a 0xffffff8008f7f054, 24 byte di .rodata
	 * NON rilocati, letti a coppie da "29400500 ldp"@0xffffff8008a74aa8
	 * con passo 8 ("8b080d28 add"@0xffffff8008a74aa4, x8, lsl #3) e
	 * passati a gt1x_touch_down come x e y.  Il binario non li nomina.
	 *   0xffffff8008f7f054: 3c 00 00 00 34 08 00 00   ->   60, 2100
	 *   0xffffff8008f7f05c: b4 00 00 00 34 08 00 00   ->  180, 2100
	 *   0xffffff8008f7f064: 2c 01 00 00 34 08 00 00   ->  300, 2100
	 */
	static const int r8f7f054[3][2] = {
		{60, 2100}, {180, 2100}, {300, 2100}
	};
	u8 touch_data[1 + 8 * 5 + 2] = {0};
	u8 touch_num = 0;
	u16 cur_event = 0;
	static u16 pre_event;
	static u16 pre_index;

	u8 key_value = 0;
	u8 *coor_data = NULL;
	s32 input_x = 0;
	s32 input_y = 0;
	s32 input_w = 0;
	s32 id = 0;
	s32 i = 0;
	s32 ret = -1;
	u8 check_sum = 0;

	GTP_DEBUG_FUNC();
	touch_num = data[0] & 0x0f;
	if (touch_num > 5) {
#line 1171
		GTP_ERROR("Illegal finger number!");
		return ERROR_VALUE;
	}

	memcpy(touch_data, data, 11);

	/* read the remaining coor data */
	if (touch_num > 1) {
		ret = gt1x_i2c_read((GTP_READ_COOR_ADDR + 11), &touch_data[11],
				    1 + 8 * touch_num + 2 - 11);
		if (ret)
			return ret;
	}

	/* checksum */
	for (i = 0; i < 1 + 8 * touch_num + 2; i++)
		check_sum += touch_data[i];

	if (check_sum) {
		ret = gt1x_i2c_read(GTP_READ_COOR_ADDR, touch_data,
				    1 + 8 * touch_num + 2);
		if (ret)
			return ret;

		check_sum = 0;
		for (i = 0; i < 1 + 8 * touch_num + 2; i++)
			check_sum += touch_data[i];

		if (check_sum) {
#line 1202
			GTP_ERROR("Checksum error[%x]", check_sum);
			return ERROR_VALUE;
		}
	}

	/*
	 * cur_event , pre_event bit defination
	 * bit4	bit3		    bit2	 bit1	   bit0
	 * hover  stylus_key  stylus  key     touch
	 *
	 */
	key_value = touch_data[1 + 8 * touch_num];
	/* check current event */
	if ((touch_data[0] & 0x10) && key_value) {
		/* get current key states */
		if (key_value & 0xF0)
			SET_BIT(cur_event, BIT_STYLUS_KEY);
		else if (key_value & 0x0F)
			SET_BIT(cur_event, BIT_TOUCH_KEY);
	} else if (touch_num)
		SET_BIT(cur_event, BIT_TOUCH);

	if (CHK_BIT(cur_event, BIT_TOUCH_KEY) ||
	    CHK_BIT(pre_event, BIT_TOUCH_KEY)) {
		if (g1008a0)
			return 0;

		id = -1;
		if (key_value & 0x01)
			id = 0;
		else if (key_value & 0x02)
			id = 1;
		else if (key_value & 0x04)
			id = 2;

		if (id >= 0) {
			gt1x_touch_down(r8f7f054[id][0], r8f7f054[id][1], 0, 0);
#line 1289
			GTP_DEBUG("--lan-- Key Down.");
		} else {
			gt1x_touch_up(0);
#line 1295
			GTP_DEBUG("--lan-- Key Up.");
		}
	}

	/* finger touch event*/
	if (CHK_BIT(cur_event, BIT_TOUCH)) {
		u8 report_num = 0;

		g1008a0 = 1;
		coor_data = &touch_data[1];
		id = coor_data[0] & 0x0F;
		for (i = 0; i < 5; i++) {
			if (i == id) {
				input_x = coor_data[1] | (coor_data[2] << 8);
				input_y = coor_data[3] | (coor_data[4] << 8);
				input_w = coor_data[5] | (coor_data[6] << 8);

				GTP_DEBUG("(%d)(%d,%d)[%d]", id, input_x,
#line 1315
					  input_y, input_w);
				gt1x_touch_down(input_x, input_y, input_w, i);
				if (report_num++ < touch_num) {
					coor_data += 8;
					id = coor_data[0] & 0x0F;
				}
				pre_index |= 0x01 << i;
			} else if (pre_index & (0x01 << i)) {
				pre_index &= ~(0x01 << i);
			}
		}
	} else if (CHK_BIT(pre_event, BIT_TOUCH)) {
		gt1x_touch_up(0);
		g1008a0 = 0;
#line 1341
		GTP_DEBUG("Released Touch.");
		pre_index = 0;
	}

	/* input sync report */
	if (CHK_BIT(cur_event, BIT_STYLUS_KEY | BIT_STYLUS) ||
	    CHK_BIT(pre_event, BIT_STYLUS_KEY | BIT_STYLUS)) {
		input_sync(pen_dev);
	}

	if (CHK_BIT(cur_event, BIT_TOUCH_KEY | BIT_TOUCH) ||
	    CHK_BIT(pre_event, BIT_TOUCH_KEY | BIT_TOUCH)) {
		input_sync(dev);
	}

	if (!pre_event && !cur_event)
		GTP_DEBUG("Additional Int Pulse.");
	else
		pre_event = cur_event;

	return 0;
}

#ifdef CONFIG_GTP_WITH_STYLUS
struct input_dev *pen_dev;

void gt1x_pen_init(void)
{
	s32 ret = 0;

	pen_dev = input_allocate_device();
	if (pen_dev == NULL) {
		GTP_ERROR("Failed to allocate input device for pen/stylus.");
		return;
	}

	pen_dev->evbit[0] =
		BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	pen_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

	set_bit(BTN_TOOL_PEN, pen_dev->keybit);
	set_bit(INPUT_PROP_DIRECT, pen_dev->propbit);

#ifdef CONFIG_GTP_HAVE_STYLUS_KEY
	input_set_capability(pen_dev, EV_KEY, BTN_STYLUS);
	input_set_capability(pen_dev, EV_KEY, BTN_STYLUS2);
#endif

	input_set_abs_params(pen_dev, ABS_MT_POSITION_X, 0, gt1x_abs_x_max, 0,
			     0);
	input_set_abs_params(pen_dev, ABS_MT_POSITION_Y, 0, gt1x_abs_y_max, 0,
			     0);
	input_set_abs_params(pen_dev, ABS_MT_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(pen_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(pen_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);

	pen_dev->name = "goodix-pen";
	pen_dev->phys = "input/ts";
	pen_dev->id.bustype = BUS_I2C;

	ret = input_register_device(pen_dev);
	if (ret) {
		GTP_ERROR("Register %s input device failed", pen_dev->name);
		return;
	}
}

void gt1x_pen_down(s32 x, s32 y, s32 size, s32 id)
{
	input_report_key(pen_dev, BTN_TOOL_PEN, 1);
#ifdef CONFIG_GTP_CHANGE_X2Y
	GTP_SWAP(x, y);
#endif

#ifdef CONFIG_GTP_ICS_SLOT_REPORT
	input_mt_slot(pen_dev, id);
	input_report_abs(pen_dev, ABS_MT_PRESSURE, size);
	input_report_abs(pen_dev, ABS_MT_TOUCH_MAJOR, size);
	input_report_abs(pen_dev, ABS_MT_TRACKING_ID, id);
	input_report_abs(pen_dev, ABS_MT_POSITION_X, x);
	input_report_abs(pen_dev, ABS_MT_POSITION_Y, y);
#else
	input_report_key(pen_dev, BTN_TOUCH, 1);
	if ((!size) && (!id)) {
		/* for virtual button */
		input_report_abs(pen_dev, ABS_MT_PRESSURE, 100);
		input_report_abs(pen_dev, ABS_MT_TOUCH_MAJOR, 100);
	} else {
		input_report_abs(pen_dev, ABS_MT_PRESSURE, size);
		input_report_abs(pen_dev, ABS_MT_TOUCH_MAJOR, size);
		input_report_abs(pen_dev, ABS_MT_TRACKING_ID, id);
	}
	input_report_abs(pen_dev, ABS_MT_POSITION_X, x);
	input_report_abs(pen_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(pen_dev);
#endif
}

void gt1x_pen_up(s32 id)
{
	input_report_key(pen_dev, BTN_TOOL_PEN, 0);
#ifdef CONFIG_GTP_ICS_SLOT_REPORT
	input_mt_slot(pen_dev, id);
	input_report_abs(pen_dev, ABS_MT_TRACKING_ID, -1);
#else
	input_report_key(pen_dev, BTN_TOUCH, 0);
	input_mt_sync(pen_dev);
#endif
}
#endif

/**
 *		PROXIMITY
 */
#ifdef CONFIG_GTP_PROXIMITY
#define GTP_REG_PROXIMITY_VALID 0x814E
#define GTP_REG_PROXIMITY_ENABLE 0x8049
u8 gt1x_proximity_flag;
u8 gt1x_proximity_detect = 1; /*0-->close ; 1--> far away*/
static struct hwmsen_object obj_ps;

s32 gt1x_ps_operate(void *self, u32 command, void *buff_in, s32 size_in,
		    void *buff_out, s32 size_out, s32 *actualout)
{
	s32 err = 0;
	s32 value;
	hwm_sensor_data *sensor_data;

	GTP_INFO("psensor operator cmd:%d", command);
	switch (command) {
	case SENSOR_DELAY:
		if ((buff_in == NULL) || (size_in < sizeof(int))) {
			GTP_ERROR("Set delay parameter error!");
			err = -EINVAL;
		}
		break;

	case SENSOR_ENABLE:
		if ((buff_in == NULL) || (size_in < sizeof(int))) {
			GTP_ERROR("Enable sensor parameter error!");
			err = -EINVAL;
		} else {
			value = *(int *)buff_in;
			err = gt1x_enable_ps(value);
		}

		break;

	case SENSOR_GET_DATA:
		if ((buff_out == NULL) ||
		    (size_out < sizeof(hwm_sensor_data))) {
			GTP_ERROR("Get sensor data parameter error!");
			err = -EINVAL;
		} else {
			sensor_data = (hwm_sensor_data *)buff_out;
			sensor_data->values[0] = gt1x_get_ps_value();
			sensor_data->value_divide = 1;
			sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
		}

		break;

	default:
		GTP_ERROR(
			"proxmy sensor operate function no this parameter %d!\n",
			command);
		err = -1;
		break;
	}

	return err;
}

void gt1x_ps_init(void)
{
	s32 err = 0;
	/*obj_ps.self = cm3623_obj;*/
	obj_ps.polling = 0; /*0--interrupt mode;1--polling mode;*/
	obj_ps.sensor_operate = gt1x_ps_operate;
	err = hwmsen_attach(ID_PROXIMITY, &obj_ps);
	if (err)
		GTP_ERROR("hwmsen attach fail, return:%d.", err);
}

void gt1x_report_ps(u8 state)
{
	s32 ret = -1;
	hwm_sensor_data sensor_data;
	/*get raw data*/
	GTP_DEBUG("P-sensor state:%s", state ? "AWAY" : "NEAR");
	/*map and store data to hwm_sensor_data*/
	sensor_data.values[0] = state;
	sensor_data.value_divide = 1;
	sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
	/*report to the up-layer*/
	ret = hwmsen_get_interrupt_data(ID_PROXIMITY, &sensor_data);

	if (ret)
		GTP_ERROR("Call hwmsen_get_interrupt_data fail = %d\n", ret);
}

static s32 gt1x_get_ps_value(void)
{
	return gt1x_proximity_detect;
}

static s32 gt1x_enable_ps(s32 enable)
{
	u8 state;
	s32 ret = -1;

	GTP_INFO("TPD proximity function to be %s.", enable ? "on" : "off");
	state = enable ? 1 : 0;
	ret = gt1x_i2c_write(GTP_REG_PROXIMITY_ENABLE, &state, 1);
	if (ret)
		GTP_ERROR("TPD %s proximity cmd failed.",
			  state ? "enable" : "disable");

	if (enable) {
		if (!ret) {
			gt1x_proximity_flag = 1;
			gt1x_proximity_detect = 1;
		}
	} else {
		gt1x_proximity_flag = 0;
	}

	GTP_INFO("TPD proximity function %s %s.", state ? "enable" : "disable",
		 ret ? "fail" : "success");
	return ret;
}

int gt1x_prox_event_handler(u8 *data)
{
	u8 proximity_status = 0;

	if (gt1x_proximity_flag) {
		GTP_DEBUG("REG INDEX[0x814E]:0x%02X\n", data[0]);
		proximity_status = (data[0] & 0x60) ? 0 : 1;
		if (proximity_status != gt1x_proximity_detect) {
			gt1x_report_ps(proximity_status);
			gt1x_proximity_detect = proximity_status;
		}
		if (proximity_status == 0)
			return 1;
		else
			return 0;
	}
	return -1;
}

#endif /*CONFIG_GTP_PROXIMITY */

/**
 *			ESD PROTECT
 */
#ifdef CONFIG_GTP_ESD_PROTECT
static int esd_work_cycle = 200;
static struct delayed_work esd_check_work;
static int esd_running;
struct mutex esd_lock;
static void gt1x_esd_check_func(struct work_struct *);

void gt1x_init_esd_protect(void)
{
	esd_work_cycle =
		2 * HZ; /*HZ: clock ticks in 1 second generated by system*/
	GTP_DEBUG("Clock ticks for an esd cycle: %d", esd_work_cycle);
	INIT_DELAYED_WORK(&esd_check_work, gt1x_esd_check_func);
	mutex_init(&esd_lock);
}

void gt1x_deinit_esd_protect(void)
{
	gt1x_esd_switch(SWITCH_OFF);
}

s32 gt1x_init_ext_watchdog(void)
{
	s32 ret;
	u8 value = 0xAA;

	GTP_DEBUG("Init external watchdog.");
	ret = gt1x_send_cmd(GTP_CMD_ESD, 0);
	ret |= gt1x_i2c_write(GTP_REG_ESD_CHECK, &value, 1);
	return ret;
}

void gt1x_esd_switch(s32 on)
{
	mutex_lock(&esd_lock);
	if (on == SWITCH_ON) { /* switch on esd check */
		if (!esd_running) {
			esd_running = 1;
			GTP_INFO("Esd protector started!");
			queue_delayed_work(gt1x_workqueue, &esd_check_work,
					   esd_work_cycle);
		}
	} else { /* switch off esd check */
		if (esd_running) {
			esd_running = 0;
			GTP_INFO("Esd protector stopped!");
			cancel_delayed_work(&esd_check_work);
		}
	}
	mutex_unlock(&esd_lock);
}

static void gt1x_esd_check_func(struct work_struct *work)
{
	s32 i = 0;
	s32 ret = -1;
	u8 esd_buf[4] = {0};

	if (!esd_running) {
		GTP_INFO("Esd protector suspended!");
		return;
	}

	for (i = 0; i < 3; i++) {
		ret = gt1x_i2c_read(GTP_REG_CMD, esd_buf, 4);
		GTP_DEBUG("[Esd]0x8040 = 0x%02X, 0x8043 = 0x%02X", esd_buf[0],
			  esd_buf[3]);
		if (!ret && esd_buf[0] != 0xAA && esd_buf[3] == 0xAA)
			break;
		msleep(50);
	}

	if (i < 3) {
		/* IC works normally, Write 0x8040 0xAA, feed the watchdog */
		gt1x_send_cmd(GTP_CMD_ESD, 0);
	} else {
		if (esd_running) {
			GTP_INFO("IC works abnormally! Process reset guitar.");
			memset(esd_buf, 0x01, sizeof(esd_buf));
			gt1x_i2c_write(0x4226, esd_buf, sizeof(esd_buf));
			msleep(50);

			gt1x_power_reset();
		} else {
			GTP_INFO("Esd protector suspended, no need reset!");
		}
	}

	mutex_lock(&esd_lock);
	if (esd_running)
		queue_delayed_work(gt1x_workqueue, &esd_check_work,
				   esd_work_cycle);
	else
		GTP_INFO("Esd protector suspended!");
	mutex_unlock(&esd_lock);
}
#endif

/**
 *         CHARGER SWITCH
 */
#ifdef CONFIG_GTP_CHARGER_SWITCH

u8 gt1x_config_charger[GTP_CONFIG_MAX_LENGTH] = {0};

static struct delayed_work charger_switch_work;
static int charger_work_cycle = 200;
static spinlock_t charger_lock;
static int charger_running;
static void gt1x_charger_work_func(struct work_struct *);

void gt1x_init_charger(void)
{
	charger_work_cycle =
		2 * HZ; /*HZ: clock ticks in 1 second generated by system*/
	GTP_DEBUG("Clock ticks for an charger cycle: %d", charger_work_cycle);
	INIT_DELAYED_WORK(&charger_switch_work, gt1x_charger_work_func);
	spin_lock_init(&charger_lock);
}

/**
 * gt1x_charger_switch - switch states of charging work thread
 *
 * @on: SWITCH_ON - start work thread, SWITCH_OFF: stop .
 *
 */
void gt1x_charger_switch(s32 on)
{
	spin_lock(&charger_lock);
	if (on == SWITCH_ON) {
		if (!charger_running) {
			charger_running = 1;
			spin_unlock(&charger_lock);
			GTP_INFO("Charger checker started!");
			queue_delayed_work(gt1x_workqueue, &charger_switch_work,
					   charger_work_cycle);
		} else {
			spin_unlock(&charger_lock);
		}
	} else {
		if (charger_running) {
			charger_running = 0;
			spin_unlock(&charger_lock);
			cancel_delayed_work(&charger_switch_work);
			GTP_INFO("Charger checker stopped!");
		} else {
			spin_unlock(&charger_lock);
		}
	}
}

/**
 * gt1x_charger_config - check and update charging status configuration
 * @dir_update
 * 0: check before send charging status configuration
 * 1: directly send charging status configuration
 *
 */
void gt1x_charger_config(s32 dir_update)
{
	static u8 chr_pluggedin;

	if (gt1x_get_charger_status()) {
		if (!chr_pluggedin || dir_update) {
			GTP_INFO("Charger Plugin.");
			if (gt1x_send_cfg(gt1x_config_charger, gt1x_cfg_length))
				GTP_ERROR(
					"Send config for Charger Plugin failed!");
			if (gt1x_send_cmd(GTP_CMD_CHARGER_ON, 0))
				GTP_ERROR(
					"Update status for Charger Plugin failed!");
			chr_pluggedin = 1;
		}
	} else {
		if (chr_pluggedin || dir_update) {
			GTP_INFO("Charger Plugout.");
			if (gt1x_send_cfg(gt1x_config, gt1x_cfg_length))
				GTP_INFO(
					"Send config for Charger Plugout failed!");
			if (gt1x_send_cmd(GTP_CMD_CHARGER_OFF, 0))
				GTP_ERROR(
					"Update status for Charger Plugout failed!");
			chr_pluggedin = 0;
		}
	}
}

static void gt1x_charger_work_func(struct work_struct *work)
{
	if (!charger_running) {
		GTP_INFO("Charger checker suspended!");
		return;
	}

	gt1x_charger_config(0);

	GTP_DEBUG("Charger check done!");
	if (charger_running)
		queue_delayed_work(gt1x_workqueue, &charger_switch_work,
				   charger_work_cycle);
}
#endif

s32 gt1x_suspend(void)
{
	s32 ret = 0;
	u8 buf[1] = {0};

	if (update_info.status)
		return 0;

#line 2214
	GTP_INFO("Suspend start...");

	if (hotknot_enabled) {
		ret = gt1x_i2c_read_dbl_check(0x81AA, buf, 1);
		if ((buf[0] == 0x55 && ret == 0) || hotknot_transfer_mode) {
#line 2232
			GTP_DEBUG("0x81AA: 0x%02X", buf[0]);
			GTP_INFO("hotknot is paired!");
			return 0;
		}
	}

	tpd_halt = 1;
	gt1x_irq_disable();

	if (g0fc0b8) {
		gesture_clear_wakeup_data();
		if (gesture_enabled) {
			gesture_enter_doze();
			gt1x_irq_enable();
			tpd_halt = 0;
		}
	} else {
		ret = gt1x_enter_sleep();
		if (ret < 0)
#line 2264
			GTP_ERROR("Suspend failed.");
	}

	msleep(58);
#line 2271
	GTP_INFO("Suspend end...");
	return 0;
}

s32 gt1x_resume(void)
{
	if (update_info.status)
		return 0;

#line 2288
	GTP_INFO("Resume start...");

	if (g0fc0b8) {
		if (!(tpd_halt | gesture_enabled))
			return 0;
	}

	if (gt1x_wakeup_sleep() < 0)
#line 2323
		GTP_ERROR("Resume failed.");

	if (!hotknot_enabled)
		gt1x_send_cmd(GTP_CMD_HN_EXIT_SLAVE, 0);

	tpd_halt = 0;
	gt1x_irq_enable();
#line 2343
	GTP_DEBUG("Resume end.");
	return 0;
}

s32 gt1x_init(void)
{
	s32 ret = -1;
	s32 retry = 0;
	u8 reg_val[1];

	gt1x_power_switch(SWITCH_ON);

	while (retry++ < 5) {
		gt1x_init_failed = 0;

		/* reset ic */
		ret = gt1x_reset_guitar();
		if (ret != 0) {
			GTP_ERROR("Reset guitar failed!");
			continue;
		}

		/* read version information */
		ret = gt1x_read_version(&gt1x_version);
		if (ret != 0) {
#line 2367
			GTP_ERROR("Get verision failed!");
			return -1;
		}

		if (gt1x_version.product_id[0] != '9' &&
		    gt1x_version.product_id[0] != '5') {
			GTP_ERROR("chip is not gt5xxx or gt9xx.");
			return -1;
		}

		tpd_load_status = 1;
		check_flag = true;
		wake_up_interruptible(&init_waiter);

		ret = gt1x_i2c_read_dbl_check(0x41E4, reg_val, 1);
		if (ret != 0)
			continue;

		if (reg_val[0] != 0xBE) {
			GTP_ERROR("Check main system not pass[0x%2X].",
#line 2388
				  reg_val[0]);
			gt1x_init_failed = 1;
		}

		ret = gt1x_i2c_read_dbl_check(0x5095, reg_val, 1);
		if (ret != 0)
			break;

		if (reg_val[0] == 0xAA)
			GTP_ERROR("Check subsystem not pass[0x%2X].",
#line 2396
				  reg_val[0]);
		ret = 0;
		break;
	}

	/* if the initialization fails, set default setting */
	if (ret | gt1x_init_failed) {
#line 2405
		GTP_ERROR("Init failed, use default setting");
		gt1x_abs_x_max = 4096;
		gt1x_abs_y_max = 4096;
		gt1x_int_type = GTP_INT_TRIGGER;
		gt1x_wakeup_level = GTP_WAKEUP_LEVEL;
	}

	/* get chip type */
	ret = gt1x_get_chip_type();
	if (ret != 0)
		GTP_ERROR("Get chip type failed!");

	ret = gt1x_read_version(&gt1x_version);
	if (ret != 0)
#line 2421
		GTP_ERROR("Get verision failed!");

	/* init and send configs */
	ret = gt1x_init_panel();
	if (ret != 0)
#line 2428
		GTP_ERROR("Init panel failed.");

	gt1x_workqueue = create_singlethread_workqueue("gt1x_workthread");
	if (gt1x_workqueue == NULL)
#line 2433
		GTP_ERROR("Create workqueue failed!");

	/* init auxiliary  node and functions */
	gt1x_init_debug_node();

#ifdef CONFIG_GTP_CREATE_WR_NODE
	gt1x_init_tool_node();
#endif

#if defined(CONFIG_GTP_GESTURE_WAKEUP) || defined(CONFIG_GTP_HOTKNOT)
	gt1x_init_node();
#endif

	return ret;
}

void gt1x_deinit(void)
{
	gt1x_deinit_debug_node();

#if defined(CONFIG_GTP_GESTURE_WAKEUP) || defined(CONFIG_GTP_HOTKNOT)
	gt1x_deinit_node();
#endif

#ifdef CONFIG_GTP_CREATE_WR_NODE
	gt1x_deinit_tool_node();
#endif

	if (gt1x_workqueue)
		destroy_workqueue(gt1x_workqueue);
}
