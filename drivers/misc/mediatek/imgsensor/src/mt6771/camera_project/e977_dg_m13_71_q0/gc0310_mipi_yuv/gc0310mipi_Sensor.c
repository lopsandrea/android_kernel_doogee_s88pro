/*
 * GC0310 MIPI YUV -- ricostruito dal binario di fabbrica.
 *
 * NON esiste in nessun albero pubblico: ALPS ha un gc0310.c per x86 sotto
 * staging/media/atomisp, che e' un'altra piattaforma e un'altra interfaccia.
 * Ogni riga qui viene da stock.elf.
 *
 * Che il file stia in camera_project/e977_dg_m13_71_q0/ non lo dice un
 * __FILE__ -- GC0310 non usa quella macro -- ma l'adiacenza in stock.map, e
 * il fatto che il nome della directory debba combaciare con la voce di
 * CONFIG_CUSTOM_KERNEL_IMGSENSOR: il Makefile di mt6771 costruisce il
 * percorso da li'.
 */

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/string.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "imgsensor_i2c.h"

#include "imgsensor_hw.h"
#include "kd_camera_feature.h"

#include "gc0310mipi_Sensor.h"

/*
 * gi2c NON e' dichiarata in imgsensor_i2c.h: il .c la definisce e basta.
 * La fabbrica ci arriva lo stesso -- "f000ac28 adrp"@0xffffff800870aa3c piu'
 * `add #0x258` danno 0xffffff8009c91258, che e' &gi2c.inst[1].pi2c_client --
 * quindi anche il suo sorgente se la dichiara da se'.
 */
extern struct IMGSENSOR_I2C gi2c;

/*
 * IL LUCCHETTO E' DI QUESTO FILE, e la prova e' un indirizzo.
 *
 * Tutte le operazioni I2C di GC0310 prendono il mutex a
 * 0xffffff800991a098 ("90009080 adrp"@0xffffff800870ac0c piu' `add #0x98`),
 * mentre GC032A prende quello a 0xffffff800991b860. Due indirizzi diversi
 * per la stessa operazione sono due oggetti diversi: un DEFINE_MUTEX per
 * file, non quello condiviso di imgsensor.
 */
static DEFINE_MUTEX(gc0310_mutex);

/*
 * I NOMI SINTETICI SONO GLI INDIRIZZI. Questi quattro oggetti sono statici e
 * stock.map non li nomina; battezzarli "cap_state" o "video_mode" vorrebbe
 * dire spacciare una lettura per una misura.
 */
static kal_bool g9c95930;
static kal_bool g9c95938;
static kal_bool g9c9593c;
static kal_uint8 g9c95940;
static MUINT32 g9c959bc;
static MSDK_SENSOR_CONFIG_STRUCT g9c95944;

#define GC0310MIPI_I2C_ADDR 0x21   /* sette bit: "52800429 mov"@0xffffff800870ac24 */
#define GC0310MIPI_I2C_SPEED 400   /* 0x61a80 = 400000 */

/*
 * L'ISTANZA I2C E' LA NUMERO 1, e si ricava dallo spiazzamento:
 * imgsensor_i2c_init mostra che gi2c.inst[N] sta a 0xffffff8009c90250 +
 * N * 4096 ("8b083128 add"@0xffffff8008704970 e' uno scorrimento di dodici
 * bit), quindi 0x9c91250 e' inst[1].
 */
#define GC0310MIPI_INST (&gi2c.inst[IMGSENSOR_I2C_DEV_1])

/*
 * LA SCRITTURA CHE LE TABELLE USANO. E' static e la fabbrica la incorpora
 * 309 volte in GC0310MIPI_Sensor_Init e 126 in GC0310MIPIGammaSelect: in
 * stock.map non c'e' nessun simbolo per lei, mentre GC0310MIPI_write_reg --
 * che ha lo stesso corpo -- c'e'.
 *
 * Il printk stampa `ret`, che a quel punto vale ancora zero, e la velocita'.
 * Sembra un difetto e lo e': il valore utile -- quale registro non e' andato
 * -- non viene stampato. E' cosi' anche in imgsensor_i2c.c di ALPS, da cui
 * questo codice e' copiato, e si riproduce (regola 7).
 */
static void GC0310MIPI_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
	struct IMGSENSOR_I2C_INST *pinst = GC0310MIPI_INST;
	int ret = 0;
	char puSendCmd[2] = { (char)addr, (char)para };

	mutex_lock(&gc0310_mutex);

	pinst->msg[0].addr = GC0310MIPI_I2C_ADDR;
	pinst->msg[0].flags = 0;
	pinst->msg[0].len = 2;
	pinst->msg[0].buf = puSendCmd;

	if (mtk_i2c_transfer(pinst->pi2c_client->adapter, pinst->msg, 1, 0,
			     GC0310MIPI_I2C_SPEED * 1000) != 1)
		pr_err("--lijian--I2C write failed (0x%x)! speed(0=%d)\n",
		       ret, GC0310MIPI_I2C_SPEED);

	mutex_unlock(&gc0310_mutex);
}

/*
 * LA LETTURA, gemella della scrittura: due messaggi invece di uno, e il
 * valore torna dai due byte del buffer di ricezione.
 */
static kal_uint16 GC0310MIPI_read_cmos_sensor_(kal_uint8 addr)
{
	struct IMGSENSOR_I2C_INST *pinst = GC0310MIPI_INST;
	int ret = 0;
	char puSendCmd = (char)addr;
	kal_uint16 get_byte = 0;

	mutex_lock(&gc0310_mutex);

	pinst->msg[0].addr = GC0310MIPI_I2C_ADDR;
	pinst->msg[0].flags = 0;
	pinst->msg[0].len = 1;
	pinst->msg[0].buf = &puSendCmd;

	pinst->msg[1].addr = GC0310MIPI_I2C_ADDR;
	pinst->msg[1].flags = I2C_M_RD;
	pinst->msg[1].len = 1;
	pinst->msg[1].buf = (u8 *)&get_byte;

	if (mtk_i2c_transfer(pinst->pi2c_client->adapter, pinst->msg, 2, 0,
			     GC0310MIPI_I2C_SPEED * 1000) != 2)
		pr_err("--lijian--I2C read failed (0x%x)! speed(0=%d)\n",
		       ret, GC0310MIPI_I2C_SPEED);

	mutex_unlock(&gc0310_mutex);

	return get_byte;
}

/*
 * I DUE AIUTANTI CHE NESSUNO CHIAMA. Esistono in stock.map con collegamento
 * esterno, ma nel binario non c'e' un solo `bl` verso di loro: sono copie di
 * imgsensor_i2c_write e imgsensor_i2c_read di ALPS, private di questo file,
 * senza il controllo `pi2c_client == NULL` e senza il __ratelimit -- il
 * printk qui e' diretto.
 *
 * Restano perche' restano nel binario (regola 6): toglierle sarebbe
 * riprodurre il file a meno di due funzioni.
 */
int gc0310_i2c_write(u8 *pwrite_data, u16 write_length, u16 write_per_cycle,
		     u16 id, int speed)
{
	struct IMGSENSOR_I2C_INST *pinst = GC0310MIPI_INST;
	struct i2c_msg *pmsg = pinst->msg;
	u8 *pdata = pwrite_data;
	u8 *pend = pwrite_data + write_length;
	int ret = 0;
	int i = 0;

	mutex_lock(&gc0310_mutex);

	while (pdata < pend && i < IMGSENSOR_I2C_CMD_LENGTH_MAX) {
		pmsg[i].addr = id >> 1;
		pmsg[i].flags = 0;
		pmsg[i].len = write_per_cycle;
		pmsg[i].buf = pdata;
		pdata += write_per_cycle;
		i++;
	}

	if (mtk_i2c_transfer(pinst->pi2c_client->adapter, pinst->msg, i, 0,
			     ((speed > 0) && (speed <= 1000))
				? speed * 1000 : GC0310MIPI_I2C_SPEED * 1000)
			!= i) {
		pr_err("--lijian--I2C write failed (0x%x)! speed(0=%d)\n",
		       ret, speed);
		ret = -1;
	}

	mutex_unlock(&gc0310_mutex);

	return ret;
}

int gc0310_i2c_read(u8 *pwrite_data, u16 write_length, u8 *pread_data,
		    u16 read_length, u16 id, int speed)
{
	struct IMGSENSOR_I2C_INST *pinst = GC0310MIPI_INST;
	int ret = 0;

	mutex_lock(&gc0310_mutex);

	pinst->msg[0].addr = id >> 1;
	pinst->msg[0].flags = 0;
	pinst->msg[0].len = write_length;
	pinst->msg[0].buf = pwrite_data;

	pinst->msg[1].addr = id >> 1;
	pinst->msg[1].flags = I2C_M_RD;
	pinst->msg[1].len = read_length;
	pinst->msg[1].buf = pread_data;

	if (mtk_i2c_transfer(pinst->pi2c_client->adapter, pinst->msg, 2, 0,
			     ((speed > 0) && (speed <= 1000))
				? speed * 1000 : GC0310MIPI_I2C_SPEED * 1000)
			!= 2) {
		pr_err("--lijian--I2C read failed (0x%x)! speed(0=%d)\n",
		       ret, speed);
		ret = -1;
	}

	mutex_unlock(&gc0310_mutex);

	return ret;
}

/*
 * L'otturatore sta in due registri e si rimette insieme con un innesto:
 * "331d7113 bfi"@0xffffff800870ae20 mette i bit del 0x03 sopra il byte del
 * 0x04. Il primo si legge come byte, il secondo come mezza parola -- lo
 * dicono le due load, `ldrb` e `ldrh`.
 */
kal_uint16 GC0310MIPI_Read_Shutter(void)
{
	kal_uint8 temp_reg1;
	kal_uint16 temp_reg2;

	temp_reg1 = GC0310MIPI_read_cmos_sensor_(0x04);
	temp_reg2 = GC0310MIPI_read_cmos_sensor_(0x03);

	return (temp_reg2 << 8) | temp_reg1;
}

void GC0310MIPI_write_reg(kal_uint8 addr, kal_uint8 para)
{
	GC0310MIPI_write_cmos_sensor(addr, para);
}

kal_uint16 GC0310MIPI_read_cmos_sensor(kal_uint8 addr)
{
	return GC0310MIPI_read_cmos_sensor_(addr);
}

kal_uint16 GC0310MIPI_read_reg(kal_uint8 addr)
{
	return GC0310MIPI_read_cmos_sensor_(addr);
}

/*
 * 309 SCRITTURE DI SEGUITO, e qui davvero non c'e' nessuno switch: contati i
 * salti condizionati sono 310, cioe' esattamente uno per scrittura -- il
 * controllo d'errore dell'I2C incorporato -- e nient'altro.
 *
 * Estratte con tools/estraiscritture.py da 0xffffff800870e994 a
 * 0xffffff8008716eb4. Le prime undici sono la sequenza d'accensione che chi
 * conosce la GC0310 si aspetta; il riscontro con una fonte esterna NON e'
 * stato fatto.
 */
void GC0310MIPI_Sensor_Init(void)
{
	GC0310MIPI_write_cmos_sensor(0xfe, 0xf0);
	GC0310MIPI_write_cmos_sensor(0xfe, 0xf0);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0xfc, 0x0e);
	GC0310MIPI_write_cmos_sensor(0xfc, 0x0e);
	GC0310MIPI_write_cmos_sensor(0xf2, 0x80);
	GC0310MIPI_write_cmos_sensor(0xf3, 0x00);
	GC0310MIPI_write_cmos_sensor(0xf7, 0x1b);
	GC0310MIPI_write_cmos_sensor(0xf8, 0x04);
	GC0310MIPI_write_cmos_sensor(0xf9, 0x8e);
	GC0310MIPI_write_cmos_sensor(0xfa, 0x11);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x03);
	GC0310MIPI_write_cmos_sensor(0x40, 0x08);
	GC0310MIPI_write_cmos_sensor(0x42, 0x00);
	GC0310MIPI_write_cmos_sensor(0x43, 0x00);
	GC0310MIPI_write_cmos_sensor(0x01, 0x03);
	GC0310MIPI_write_cmos_sensor(0x10, 0x84);
	GC0310MIPI_write_cmos_sensor(0x01, 0x03);
	GC0310MIPI_write_cmos_sensor(0x02, 0x33);
	GC0310MIPI_write_cmos_sensor(0x03, 0x94);
	GC0310MIPI_write_cmos_sensor(0x04, 0x01);
	GC0310MIPI_write_cmos_sensor(0x05, 0x00);
	GC0310MIPI_write_cmos_sensor(0x06, 0x80);
	GC0310MIPI_write_cmos_sensor(0x11, 0x1e);
	GC0310MIPI_write_cmos_sensor(0x12, 0x00);
	GC0310MIPI_write_cmos_sensor(0x13, 0x05);
	GC0310MIPI_write_cmos_sensor(0x15, 0x10);
	GC0310MIPI_write_cmos_sensor(0x21, 0x10);
	GC0310MIPI_write_cmos_sensor(0x22, 0x01);
	GC0310MIPI_write_cmos_sensor(0x23, 0x10);
	GC0310MIPI_write_cmos_sensor(0x24, 0x02);
	GC0310MIPI_write_cmos_sensor(0x25, 0x10);
	GC0310MIPI_write_cmos_sensor(0x26, 0x03);
	GC0310MIPI_write_cmos_sensor(0x29, 0x06);
	GC0310MIPI_write_cmos_sensor(0x2a, 0x0a);
	GC0310MIPI_write_cmos_sensor(0x2b, 0x06);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0x00, 0x2f);
	GC0310MIPI_write_cmos_sensor(0x01, 0x0f);
	GC0310MIPI_write_cmos_sensor(0x02, 0x04);
	GC0310MIPI_write_cmos_sensor(0x03, 0x03);
	GC0310MIPI_write_cmos_sensor(0x04, 0x50);
	GC0310MIPI_write_cmos_sensor(0x09, 0x00);
	GC0310MIPI_write_cmos_sensor(0x0a, 0x00);
	GC0310MIPI_write_cmos_sensor(0x0b, 0x00);
	GC0310MIPI_write_cmos_sensor(0x0c, 0x04);
	GC0310MIPI_write_cmos_sensor(0x0d, 0x01);
	GC0310MIPI_write_cmos_sensor(0x0e, 0xe8);
	GC0310MIPI_write_cmos_sensor(0x0f, 0x02);
	GC0310MIPI_write_cmos_sensor(0x10, 0x88);
	GC0310MIPI_write_cmos_sensor(0x16, 0x00);
	GC0310MIPI_write_cmos_sensor(0x17, 0x14);
	GC0310MIPI_write_cmos_sensor(0x18, 0x1a);
	GC0310MIPI_write_cmos_sensor(0x19, 0x14);
	GC0310MIPI_write_cmos_sensor(0x1b, 0x48);
	GC0310MIPI_write_cmos_sensor(0x1c, 0x1c);
	GC0310MIPI_write_cmos_sensor(0x1e, 0x6b);
	GC0310MIPI_write_cmos_sensor(0x1f, 0x28);
	GC0310MIPI_write_cmos_sensor(0x20, 0x8b);
	GC0310MIPI_write_cmos_sensor(0x21, 0x49);
	GC0310MIPI_write_cmos_sensor(0x22, 0xb0);
	GC0310MIPI_write_cmos_sensor(0x23, 0x04);
	GC0310MIPI_write_cmos_sensor(0x24, 0x16);
	GC0310MIPI_write_cmos_sensor(0x34, 0x20);
	GC0310MIPI_write_cmos_sensor(0x26, 0x23);
	GC0310MIPI_write_cmos_sensor(0x28, 0xff);
	GC0310MIPI_write_cmos_sensor(0x29, 0x00);
	GC0310MIPI_write_cmos_sensor(0x32, 0x00);
	GC0310MIPI_write_cmos_sensor(0x33, 0x10);
	GC0310MIPI_write_cmos_sensor(0x37, 0x20);
	GC0310MIPI_write_cmos_sensor(0x38, 0x10);
	GC0310MIPI_write_cmos_sensor(0x47, 0x80);
	GC0310MIPI_write_cmos_sensor(0x4e, 0x66);
	GC0310MIPI_write_cmos_sensor(0xa8, 0x02);
	GC0310MIPI_write_cmos_sensor(0xa9, 0x80);
	GC0310MIPI_write_cmos_sensor(0x40, 0xff);
	GC0310MIPI_write_cmos_sensor(0x41, 0x21);
	GC0310MIPI_write_cmos_sensor(0x42, 0xcf);
	GC0310MIPI_write_cmos_sensor(0x44, 0x02);
	GC0310MIPI_write_cmos_sensor(0x45, 0xa8);
	GC0310MIPI_write_cmos_sensor(0x46, 0x02);
	GC0310MIPI_write_cmos_sensor(0x4a, 0x11);
	GC0310MIPI_write_cmos_sensor(0x4b, 0x01);
	GC0310MIPI_write_cmos_sensor(0x4c, 0x20);
	GC0310MIPI_write_cmos_sensor(0x4d, 0x05);
	GC0310MIPI_write_cmos_sensor(0x4f, 0x01);
	GC0310MIPI_write_cmos_sensor(0x50, 0x01);
	GC0310MIPI_write_cmos_sensor(0x55, 0x01);
	GC0310MIPI_write_cmos_sensor(0x56, 0xe0);
	GC0310MIPI_write_cmos_sensor(0x57, 0x02);
	GC0310MIPI_write_cmos_sensor(0x58, 0x80);
	GC0310MIPI_write_cmos_sensor(0x70, 0x70);
	GC0310MIPI_write_cmos_sensor(0x5a, 0x84);
	GC0310MIPI_write_cmos_sensor(0x5b, 0xc9);
	GC0310MIPI_write_cmos_sensor(0x5c, 0xed);
	GC0310MIPI_write_cmos_sensor(0x77, 0x74);
	GC0310MIPI_write_cmos_sensor(0x78, 0x40);
	GC0310MIPI_write_cmos_sensor(0x79, 0x5f);
	GC0310MIPI_write_cmos_sensor(0x82, 0x14);
	GC0310MIPI_write_cmos_sensor(0x83, 0x0b);
	GC0310MIPI_write_cmos_sensor(0x89, 0xf0);
	GC0310MIPI_write_cmos_sensor(0x8f, 0xaa);
	GC0310MIPI_write_cmos_sensor(0x90, 0x8c);
	GC0310MIPI_write_cmos_sensor(0x91, 0x90);
	GC0310MIPI_write_cmos_sensor(0x92, 0x03);
	GC0310MIPI_write_cmos_sensor(0x93, 0x03);
	GC0310MIPI_write_cmos_sensor(0x94, 0x05);
	GC0310MIPI_write_cmos_sensor(0x95, 0x65);
	GC0310MIPI_write_cmos_sensor(0x96, 0xf0);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0x9a, 0x20);
	GC0310MIPI_write_cmos_sensor(0x9b, 0x80);
	GC0310MIPI_write_cmos_sensor(0x9c, 0x40);
	GC0310MIPI_write_cmos_sensor(0x9d, 0x80);
	GC0310MIPI_write_cmos_sensor(0xa1, 0x30);
	GC0310MIPI_write_cmos_sensor(0xa2, 0x32);
	GC0310MIPI_write_cmos_sensor(0xa4, 0x30);
	GC0310MIPI_write_cmos_sensor(0xa5, 0x30);
	GC0310MIPI_write_cmos_sensor(0xaa, 0x10);
	GC0310MIPI_write_cmos_sensor(0xac, 0x22);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0xbf, 0x08);
	GC0310MIPI_write_cmos_sensor(0xc0, 0x16);
	GC0310MIPI_write_cmos_sensor(0xc1, 0x28);
	GC0310MIPI_write_cmos_sensor(0xc2, 0x41);
	GC0310MIPI_write_cmos_sensor(0xc3, 0x5a);
	GC0310MIPI_write_cmos_sensor(0xc4, 0x6c);
	GC0310MIPI_write_cmos_sensor(0xc5, 0x7a);
	GC0310MIPI_write_cmos_sensor(0xc6, 0x96);
	GC0310MIPI_write_cmos_sensor(0xc7, 0xac);
	GC0310MIPI_write_cmos_sensor(0xc8, 0xbc);
	GC0310MIPI_write_cmos_sensor(0xc9, 0xc9);
	GC0310MIPI_write_cmos_sensor(0xca, 0xd3);
	GC0310MIPI_write_cmos_sensor(0xcb, 0xdd);
	GC0310MIPI_write_cmos_sensor(0xcc, 0xe5);
	GC0310MIPI_write_cmos_sensor(0xcd, 0xf1);
	GC0310MIPI_write_cmos_sensor(0xce, 0xfa);
	GC0310MIPI_write_cmos_sensor(0xcf, 0xff);
	GC0310MIPI_write_cmos_sensor(0xd0, 0x40);
	GC0310MIPI_write_cmos_sensor(0xd1, 0x34);
	GC0310MIPI_write_cmos_sensor(0xd2, 0x34);
	GC0310MIPI_write_cmos_sensor(0xd3, 0x40);
	GC0310MIPI_write_cmos_sensor(0xd6, 0xf2);
	GC0310MIPI_write_cmos_sensor(0xd7, 0x1b);
	GC0310MIPI_write_cmos_sensor(0xd8, 0x18);
	GC0310MIPI_write_cmos_sensor(0xdd, 0x03);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0x05, 0x30);
	GC0310MIPI_write_cmos_sensor(0x06, 0x75);
	GC0310MIPI_write_cmos_sensor(0x07, 0x40);
	GC0310MIPI_write_cmos_sensor(0x08, 0xb0);
	GC0310MIPI_write_cmos_sensor(0x0a, 0xc5);
	GC0310MIPI_write_cmos_sensor(0x0b, 0x11);
	GC0310MIPI_write_cmos_sensor(0x0c, 0x00);
	GC0310MIPI_write_cmos_sensor(0x12, 0x52);
	GC0310MIPI_write_cmos_sensor(0x13, 0x38);
	GC0310MIPI_write_cmos_sensor(0x18, 0x95);
	GC0310MIPI_write_cmos_sensor(0x19, 0x96);
	GC0310MIPI_write_cmos_sensor(0x1f, 0x20);
	GC0310MIPI_write_cmos_sensor(0x20, 0xc0);
	GC0310MIPI_write_cmos_sensor(0x3e, 0x40);
	GC0310MIPI_write_cmos_sensor(0x3f, 0x57);
	GC0310MIPI_write_cmos_sensor(0x40, 0x7d);
	GC0310MIPI_write_cmos_sensor(0x03, 0x60);
	GC0310MIPI_write_cmos_sensor(0x44, 0x02);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0x1c, 0x91);
	GC0310MIPI_write_cmos_sensor(0x21, 0x15);
	GC0310MIPI_write_cmos_sensor(0x50, 0x80);
	GC0310MIPI_write_cmos_sensor(0x56, 0x04);
	GC0310MIPI_write_cmos_sensor(0x59, 0x08);
	GC0310MIPI_write_cmos_sensor(0x5b, 0x02);
	GC0310MIPI_write_cmos_sensor(0x61, 0x8d);
	GC0310MIPI_write_cmos_sensor(0x62, 0xa7);
	GC0310MIPI_write_cmos_sensor(0x63, 0xd0);
	GC0310MIPI_write_cmos_sensor(0x65, 0x06);
	GC0310MIPI_write_cmos_sensor(0x66, 0x06);
	GC0310MIPI_write_cmos_sensor(0x67, 0x84);
	GC0310MIPI_write_cmos_sensor(0x69, 0x08);
	GC0310MIPI_write_cmos_sensor(0x6a, 0x25);
	GC0310MIPI_write_cmos_sensor(0x6b, 0x01);
	GC0310MIPI_write_cmos_sensor(0x6c, 0x00);
	GC0310MIPI_write_cmos_sensor(0x6d, 0x02);
	GC0310MIPI_write_cmos_sensor(0x6e, 0xf0);
	GC0310MIPI_write_cmos_sensor(0x6f, 0x80);
	GC0310MIPI_write_cmos_sensor(0x76, 0x80);
	GC0310MIPI_write_cmos_sensor(0x78, 0xaf);
	GC0310MIPI_write_cmos_sensor(0x79, 0x75);
	GC0310MIPI_write_cmos_sensor(0x7a, 0x40);
	GC0310MIPI_write_cmos_sensor(0x7b, 0x50);
	GC0310MIPI_write_cmos_sensor(0x7c, 0x0c);
	GC0310MIPI_write_cmos_sensor(0x90, 0xc9);
	GC0310MIPI_write_cmos_sensor(0x91, 0xbe);
	GC0310MIPI_write_cmos_sensor(0x92, 0xe2);
	GC0310MIPI_write_cmos_sensor(0x93, 0xc9);
	GC0310MIPI_write_cmos_sensor(0x95, 0x1b);
	GC0310MIPI_write_cmos_sensor(0x96, 0xe2);
	GC0310MIPI_write_cmos_sensor(0x97, 0x49);
	GC0310MIPI_write_cmos_sensor(0x98, 0x1b);
	GC0310MIPI_write_cmos_sensor(0x9a, 0x49);
	GC0310MIPI_write_cmos_sensor(0x9b, 0x1b);
	GC0310MIPI_write_cmos_sensor(0x9c, 0xc3);
	GC0310MIPI_write_cmos_sensor(0x9d, 0x49);
	GC0310MIPI_write_cmos_sensor(0x9f, 0xc7);
	GC0310MIPI_write_cmos_sensor(0xa0, 0xc8);
	GC0310MIPI_write_cmos_sensor(0xa1, 0x00);
	GC0310MIPI_write_cmos_sensor(0xa2, 0x00);
	GC0310MIPI_write_cmos_sensor(0x86, 0x00);
	GC0310MIPI_write_cmos_sensor(0x87, 0x00);
	GC0310MIPI_write_cmos_sensor(0x88, 0x00);
	GC0310MIPI_write_cmos_sensor(0x89, 0x00);
	GC0310MIPI_write_cmos_sensor(0xa4, 0xb9);
	GC0310MIPI_write_cmos_sensor(0xa5, 0xa0);
	GC0310MIPI_write_cmos_sensor(0xa6, 0xba);
	GC0310MIPI_write_cmos_sensor(0xa7, 0x92);
	GC0310MIPI_write_cmos_sensor(0xa9, 0xba);
	GC0310MIPI_write_cmos_sensor(0xaa, 0x80);
	GC0310MIPI_write_cmos_sensor(0xab, 0x9d);
	GC0310MIPI_write_cmos_sensor(0xac, 0x7f);
	GC0310MIPI_write_cmos_sensor(0xae, 0xbb);
	GC0310MIPI_write_cmos_sensor(0xaf, 0x9d);
	GC0310MIPI_write_cmos_sensor(0xb0, 0xc8);
	GC0310MIPI_write_cmos_sensor(0xb1, 0x97);
	GC0310MIPI_write_cmos_sensor(0xb3, 0xb7);
	GC0310MIPI_write_cmos_sensor(0xb4, 0x7f);
	GC0310MIPI_write_cmos_sensor(0xb5, 0x00);
	GC0310MIPI_write_cmos_sensor(0xb6, 0x00);
	GC0310MIPI_write_cmos_sensor(0x8b, 0x00);
	GC0310MIPI_write_cmos_sensor(0x8c, 0x00);
	GC0310MIPI_write_cmos_sensor(0x8d, 0x00);
	GC0310MIPI_write_cmos_sensor(0x8e, 0x00);
	GC0310MIPI_write_cmos_sensor(0x94, 0x55);
	GC0310MIPI_write_cmos_sensor(0x99, 0xa6);
	GC0310MIPI_write_cmos_sensor(0x9e, 0xaa);
	GC0310MIPI_write_cmos_sensor(0xa3, 0x0a);
	GC0310MIPI_write_cmos_sensor(0x8a, 0x00);
	GC0310MIPI_write_cmos_sensor(0xa8, 0x55);
	GC0310MIPI_write_cmos_sensor(0xad, 0x55);
	GC0310MIPI_write_cmos_sensor(0xb2, 0x55);
	GC0310MIPI_write_cmos_sensor(0xb7, 0x05);
	GC0310MIPI_write_cmos_sensor(0x8f, 0x00);
	GC0310MIPI_write_cmos_sensor(0xb8, 0xcb);
	GC0310MIPI_write_cmos_sensor(0xb9, 0x9b);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0xd0, 0x38);
	GC0310MIPI_write_cmos_sensor(0xd1, 0x00);
	GC0310MIPI_write_cmos_sensor(0xd2, 0x02);
	GC0310MIPI_write_cmos_sensor(0xd3, 0x04);
	GC0310MIPI_write_cmos_sensor(0xd4, 0x38);
	GC0310MIPI_write_cmos_sensor(0xd5, 0x12);
	GC0310MIPI_write_cmos_sensor(0xd6, 0x30);
	GC0310MIPI_write_cmos_sensor(0xd7, 0x00);
	GC0310MIPI_write_cmos_sensor(0xd8, 0x0a);
	GC0310MIPI_write_cmos_sensor(0xd9, 0x16);
	GC0310MIPI_write_cmos_sensor(0xda, 0x39);
	GC0310MIPI_write_cmos_sensor(0xdb, 0xf8);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0xc1, 0x3c);
	GC0310MIPI_write_cmos_sensor(0xc2, 0x50);
	GC0310MIPI_write_cmos_sensor(0xc3, 0x00);
	GC0310MIPI_write_cmos_sensor(0xc4, 0x40);
	GC0310MIPI_write_cmos_sensor(0xc5, 0x30);
	GC0310MIPI_write_cmos_sensor(0xc6, 0x30);
	GC0310MIPI_write_cmos_sensor(0xc7, 0x10);
	GC0310MIPI_write_cmos_sensor(0xc8, 0x00);
	GC0310MIPI_write_cmos_sensor(0xc9, 0x00);
	GC0310MIPI_write_cmos_sensor(0xdc, 0x20);
	GC0310MIPI_write_cmos_sensor(0xdd, 0x10);
	GC0310MIPI_write_cmos_sensor(0xdf, 0x00);
	GC0310MIPI_write_cmos_sensor(0xde, 0x00);
	GC0310MIPI_write_cmos_sensor(0x01, 0x10);
	GC0310MIPI_write_cmos_sensor(0x0b, 0x31);
	GC0310MIPI_write_cmos_sensor(0x0e, 0x50);
	GC0310MIPI_write_cmos_sensor(0x0f, 0x0f);
	GC0310MIPI_write_cmos_sensor(0x10, 0x6e);
	GC0310MIPI_write_cmos_sensor(0x12, 0xa0);
	GC0310MIPI_write_cmos_sensor(0x15, 0x60);
	GC0310MIPI_write_cmos_sensor(0x16, 0x60);
	GC0310MIPI_write_cmos_sensor(0x17, 0xe0);
	GC0310MIPI_write_cmos_sensor(0xcc, 0x0c);
	GC0310MIPI_write_cmos_sensor(0xcd, 0x10);
	GC0310MIPI_write_cmos_sensor(0xce, 0xa0);
	GC0310MIPI_write_cmos_sensor(0xcf, 0xe6);
	GC0310MIPI_write_cmos_sensor(0x45, 0xf7);
	GC0310MIPI_write_cmos_sensor(0x46, 0xff);
	GC0310MIPI_write_cmos_sensor(0x47, 0x15);
	GC0310MIPI_write_cmos_sensor(0x48, 0x03);
	GC0310MIPI_write_cmos_sensor(0x4f, 0x60);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0x05, 0x02);
	GC0310MIPI_write_cmos_sensor(0x06, 0xd1);
	GC0310MIPI_write_cmos_sensor(0x07, 0x00);
	GC0310MIPI_write_cmos_sensor(0x08, 0x22);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0x25, 0x00);
	GC0310MIPI_write_cmos_sensor(0x26, 0x6a);
	GC0310MIPI_write_cmos_sensor(0x27, 0x02);
	GC0310MIPI_write_cmos_sensor(0x28, 0x12);
	GC0310MIPI_write_cmos_sensor(0x29, 0x03);
	GC0310MIPI_write_cmos_sensor(0x2a, 0x50);
	GC0310MIPI_write_cmos_sensor(0x2b, 0x05);
	GC0310MIPI_write_cmos_sensor(0x2c, 0xcc);
	GC0310MIPI_write_cmos_sensor(0x2d, 0x07);
	GC0310MIPI_write_cmos_sensor(0x2e, 0x74);
	GC0310MIPI_write_cmos_sensor(0x3c, 0x20);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x03);
	GC0310MIPI_write_cmos_sensor(0x10, 0x94);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
}

/*
 * SETTE RAMI, NON UNA LISTA PIATTA -- e il documento diceva il contrario.
 *
 * "7100181f cmp"@0xffffff800870aff0 confronta l'argomento con 6 e
 * "d61f0140 br"@0xffffff800870b014 salta per tavola: sette casi, con le
 * scritture divise 35, 15, 15, 14, 15, 15, 17. Estrarle di seguito le
 * concatenava tutte in una funzione sola, e la misura lo diceva -- 144 byte
 * di meno -- ma la spiegazione "non c'e' nessuno switch" era gia' scritta e
 * l'avevo creduta.
 *
 * Il conteggio che l'aveva prodotta -- "127 salti condizionati, uno per
 * scrittura" -- non era sbagliato: e' che `br` NON E' un salto condizionato,
 * e quel conteggio non poteva vederlo.
 *
 * L'ordine dei casi nel sorgente NON e' l'ordine in memoria: il caso 1 sta
 * dopo il 4. Si legge dalla tavola a 0xffffff8008f49e7c, non dagli indirizzi.
 */
void GC0310MIPIGammaSelect(kal_uint8 index)
{
	switch (index) {
	case 0:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0xbf, 0x06);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x12);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x22);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x35);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x4b);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x5f);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x72);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x8d);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xa4);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xb8);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xc8);
		GC0310MIPI_write_cmos_sensor(0xca, 0xd4);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xde);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xe6);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xf1);
		GC0310MIPI_write_cmos_sensor(0xce, 0xf8);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0xbf, 0x0b);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x17);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x2a);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x41);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x54);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x66);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x74);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x8c);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xa3);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xb5);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xc4);
		GC0310MIPI_write_cmos_sensor(0xca, 0xd0);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xdb);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xe5);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xf0);
		GC0310MIPI_write_cmos_sensor(0xce, 0xf7);
		GC0310MIPI_write_cmos_sensor(0xcf, 0xff);
		break;
	case 2:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x0b);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x17);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x2a);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x41);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x54);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x66);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x74);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x8c);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xa3);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xb5);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xc4);
		GC0310MIPI_write_cmos_sensor(0xca, 0xd0);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xdb);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xe5);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xf0);
		break;
	case 3:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x0e);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x1c);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x34);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x48);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x5a);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x6b);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x7b);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x95);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xab);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xbf);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xce);
		GC0310MIPI_write_cmos_sensor(0xca, 0xd9);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xe4);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xec);
		break;
	case 4:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x10);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x20);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x38);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x4e);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x63);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x76);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x87);
		GC0310MIPI_write_cmos_sensor(0xc6, 0xa2);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xb8);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xca);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xd8);
		GC0310MIPI_write_cmos_sensor(0xca, 0xe3);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xeb);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xf0);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xf8);
		break;
	case 1:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x08);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x0f);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x21);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x32);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x43);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x50);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x5e);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x78);
		GC0310MIPI_write_cmos_sensor(0xc7, 0x90);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xa6);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xb9);
		GC0310MIPI_write_cmos_sensor(0xca, 0xc9);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xd6);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xe0);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xee);
		break;
	case 5:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x14);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x28);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x44);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x5d);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x72);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x86);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x95);
		GC0310MIPI_write_cmos_sensor(0xc6, 0xb1);
		GC0310MIPI_write_cmos_sensor(0xc7, 0xc6);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xd5);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xe1);
		GC0310MIPI_write_cmos_sensor(0xca, 0xea);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xf1);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xf5);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xfb);
		break;
	case 6:
		GC0310MIPI_write_cmos_sensor(0xbf, 0x0b);
		GC0310MIPI_write_cmos_sensor(0xc0, 0x16);
		GC0310MIPI_write_cmos_sensor(0xc1, 0x29);
		GC0310MIPI_write_cmos_sensor(0xc2, 0x3c);
		GC0310MIPI_write_cmos_sensor(0xc3, 0x4f);
		GC0310MIPI_write_cmos_sensor(0xc4, 0x5f);
		GC0310MIPI_write_cmos_sensor(0xc5, 0x6f);
		GC0310MIPI_write_cmos_sensor(0xc6, 0x8a);
		GC0310MIPI_write_cmos_sensor(0xc7, 0x9f);
		GC0310MIPI_write_cmos_sensor(0xc8, 0xb4);
		GC0310MIPI_write_cmos_sensor(0xc9, 0xc6);
		GC0310MIPI_write_cmos_sensor(0xca, 0xd3);
		GC0310MIPI_write_cmos_sensor(0xcb, 0xdd);
		GC0310MIPI_write_cmos_sensor(0xcc, 0xe5);
		GC0310MIPI_write_cmos_sensor(0xcd, 0xf1);
		GC0310MIPI_write_cmos_sensor(0xce, 0xfa);
		GC0310MIPI_write_cmos_sensor(0xcf, 0xff);
		break;
	default:
		break;
	}
}


/*
 * QUATTRO FUNZIONI VUOTE DI FABBRICA. Non sono abbozzi: nel binario il loro
 * corpo intero e' `ret`, quattro byte, e stock.map le elenca con quella
 * dimensione. Riprodurle vuote e' quel che chiede la regola 7.
 */
void GC0310MIPI_Set_Shutter(kal_uint16 iShutter)
{
}

void GC0310MIPI_config_window(kal_uint16 startx, kal_uint16 starty,
			      kal_uint16 width, kal_uint16 height)
{
}

void GC0310MIPI_SetGain(kal_uint16 iGain)
{
}

void GC0310MIPI_Write_More_Registers(void)
{
}

UINT32 GC0310MIPIClose(void)
{
	return ERROR_NONE;
}

UINT32 GC0310MIPIGetResolution(
	MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	pSensorResolution->SensorPreviewWidth = GC0310MIPI_IMAGE_SENSOR_PV_WIDTH;
	pSensorResolution->SensorPreviewHeight = GC0310MIPI_IMAGE_SENSOR_PV_HEIGHT;
	pSensorResolution->SensorFullWidth = GC0310MIPI_IMAGE_SENSOR_FULL_WIDTH;
	pSensorResolution->SensorFullHeight = GC0310MIPI_IMAGE_SENSOR_FULL_HEIGHT;
	pSensorResolution->SensorVideoWidth = GC0310MIPI_IMAGE_SENSOR_VIDEO_WIDTH;
	pSensorResolution->SensorVideoHeight = GC0310MIPI_IMAGE_SENSOR_VIDEO_HEIGHT;

	return ERROR_NONE;
}

/*
 * LA FINESTRA E' SEMPRE LA STESSA, e comincia dalla riga 1: la fabbrica
 * costruisce 0x01DA_0278_0001_0000 in un registro solo e lo scrive con una
 * store da otto byte, cioe' {0, 1, 632, 474}.
 */
UINT32 GC0310MIPIPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
			 MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	g9c95930 = (pSensorConfigData->SensorOperationMode == 1)
			? KAL_TRUE : KAL_FALSE;

	pImageWindow->GrabStartX = 0;
	pImageWindow->GrabStartY = 1;
	pImageWindow->ExposureWindowWidth = GC0310MIPI_IMAGE_SENSOR_PV_WIDTH;
	pImageWindow->ExposureWindowHeight = GC0310MIPI_IMAGE_SENSOR_PV_HEIGHT;

	memcpy(&g9c95944, pSensorConfigData,
	       sizeof(MSDK_SENSOR_CONFIG_STRUCT));

	return ERROR_NONE;
}

UINT32 GC0310MIPICapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
			 MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	g9c95938 = KAL_TRUE;

	pImageWindow->GrabStartX = 0;
	pImageWindow->GrabStartY = 1;
	pImageWindow->ExposureWindowWidth = GC0310MIPI_IMAGE_SENSOR_FULL_WIDTH;
	pImageWindow->ExposureWindowHeight = GC0310MIPI_IMAGE_SENSOR_FULL_HEIGHT;

	memcpy(&g9c95944, pSensorConfigData,
	       sizeof(MSDK_SENSOR_CONFIG_STRUCT));

	return ERROR_NONE;
}

/*
 * Control NON guarda lo scenario: chiama Preview e basta, e restituisce TRUE
 * invece di ERROR_NONE. Nel binario non c'e' nessun confronto su x0, e le 21
 * istruzioni che seguono sono quelle di Preview incorporata.
 */
UINT32 GC0310MIPIControl(enum MSDK_SCENARIO_ID_ENUM ScenarioId,
			 MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
			 MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	GC0310MIPIPreview(pImageWindow, pSensorConfigData);

	return TRUE;
}

/*
 * Quindici o trenta fotogrammi al secondo, tutto il resto e' un messaggio e
 * nient'altro: il valore di ritorno e' TRUE anche quando la velocita' e'
 * sbagliata.
 */
/*
 * TRENTA TENTATIVI, UNO AL SECONDO. Il ritardo fra un tentativo e l'altro e'
 * mdelay(1000) -- mille giri di udelay(1000), che nel binario si vedono come
 * un contatore a -1000 e un `adds`/`b.cc` -- e i tentativi sono 30
 * ("321f0fe0 orr"@0xffffff8008716f04 mette 0x1e). Mezzo minuto buono se il
 * sensore non c'e'. E' quel che fa la fabbrica.
 *
 * E lavora solo se l'indice del sensore e' 3: g9c90244 lo lascia li'
 * imgsensor_hw_power.
 */
/*
 * Legge 0x42, alza o abbassa il bit 1, riscrive. Il binario prova il bit 0
 * dell'argomento ("36000073 tbz"), non l'argomento intero: e' quel che fa un
 * kal_bool passato a un `if`.
 */
static void GC0310MIPI_awb_enable(kal_bool enable)
{
	kal_uint16 temp_AWB_Reg = GC0310MIPI_read_cmos_sensor_(0x42);

	if (enable)
		temp_AWB_Reg |= 0x02;
	else
		temp_AWB_Reg &= ~0x02;

	GC0310MIPI_write_cmos_sensor(0x42, temp_AWB_Reg);
}

/*
 * AWB: cinque modalita' scrivono, una spegne l'automatico, le altre no.
 *
 * I casi si leggono dalla tavola a 0xffffff8008f49e8a
 * (tools/casiswitch.py), e i valori dagli `strh` che caricano
 * (dato << 8) | registro: 0x4877 e' "0x77 = 0x48", non due cose diverse.
 *
 * SHADE, TWILIGHT e WARM_FLUORESCENT cadono nel default e restituiscono
 * FALSE: la fabbrica non le tratta. I loro tre numeri -- 4, 5 e 7 -- puntano
 * tutti allo stesso indirizzo, che e' il ritorno con w0 rimasto a zero.
 */
UINT32 GC0310MIPI_set_param_wb(UINT16 para)
{
	if (para > AWB_MODE_TUNGSTEN)
		return FALSE;

	switch (para) {
	case AWB_MODE_OFF:
		break;

	case AWB_MODE_AUTO:
		GC0310MIPI_awb_enable(KAL_TRUE);
		break;

	case AWB_MODE_DAYLIGHT:
		GC0310MIPI_awb_enable(KAL_FALSE);
		GC0310MIPI_write_cmos_sensor(0x77, 0x74);
		GC0310MIPI_write_cmos_sensor(0x78, 0x52);
		GC0310MIPI_write_cmos_sensor(0x79, 0x40);
		break;

	case AWB_MODE_CLOUDY_DAYLIGHT:
		GC0310MIPI_awb_enable(KAL_FALSE);
		GC0310MIPI_write_cmos_sensor(0x77, 0x8c);
		GC0310MIPI_write_cmos_sensor(0x78, 0x50);
		GC0310MIPI_write_cmos_sensor(0x79, 0x40);
		break;

	case AWB_MODE_FLUORESCENT:
		GC0310MIPI_awb_enable(KAL_FALSE);
		GC0310MIPI_write_cmos_sensor(0x77, 0x40);
		GC0310MIPI_write_cmos_sensor(0x78, 0x42);
		GC0310MIPI_write_cmos_sensor(0x79, 0x50);
		break;

	case AWB_MODE_INCANDESCENT:
		GC0310MIPI_awb_enable(KAL_FALSE);
		GC0310MIPI_write_cmos_sensor(0x77, 0x48);
		GC0310MIPI_write_cmos_sensor(0x78, 0x40);
		GC0310MIPI_write_cmos_sensor(0x79, 0x5c);
		break;

	case AWB_MODE_TUNGSTEN:
		GC0310MIPI_awb_enable(KAL_FALSE);
		GC0310MIPI_write_cmos_sensor(0x77, 0x40);
		GC0310MIPI_write_cmos_sensor(0x78, 0x54);
		GC0310MIPI_write_cmos_sensor(0x79, 0x70);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

/*
 * EFFETTI: undici casi, cinque dei quali cadono nel default.
 * Tavola a 0xffffff8008f49e9e.
 */
UINT32 GC0310MIPI_set_param_effect(UINT16 para)
{
	if (para > MEFFECT_SEPIABLUE)
		return FALSE;

	switch (para) {
	case MEFFECT_OFF:
		GC0310MIPI_write_cmos_sensor(0x43, 0x00);
		break;

	case MEFFECT_MONO:
		GC0310MIPI_write_cmos_sensor(0x43, 0x02);
		GC0310MIPI_write_cmos_sensor(0xda, 0x00);
		break;

	case MEFFECT_SEPIA:
		GC0310MIPI_write_cmos_sensor(0x43, 0x02);
		GC0310MIPI_write_cmos_sensor(0xda, 0xd0);
		GC0310MIPI_write_cmos_sensor(0xdb, 0x28);
		break;

	case MEFFECT_NEGATIVE:
		GC0310MIPI_write_cmos_sensor(0x43, 0x01);
		break;

	case MEFFECT_SEPIAGREEN:
		GC0310MIPI_write_cmos_sensor(0x43, 0x02);
		GC0310MIPI_write_cmos_sensor(0xda, 0xc0);
		GC0310MIPI_write_cmos_sensor(0xdb, 0xc0);
		break;

	case MEFFECT_SEPIABLUE:
		GC0310MIPI_write_cmos_sensor(0x43, 0x02);
		GC0310MIPI_write_cmos_sensor(0xda, 0x50);
		GC0310MIPI_write_cmos_sensor(0xdb, 0xe0);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

UINT32 GC0310MIPIGetSensorID(UINT32 *sensorID)
{
	kal_uint8 retry = 30;

	if (g9c90244 != 3) {
		*sensorID = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}

	do {
		*sensorID = (GC0310MIPI_read_cmos_sensor_(0xf0) << 8)
			  | GC0310MIPI_read_cmos_sensor_(0xf1);
		if (*sensorID == GC0310_SENSOR_ID)
			break;
		printk("Read Sensor ID Fail = 0x%04x\n", *sensorID);
		mdelay(1000);
	} while (--retry);

	printk("---GC0310MIPIGetSensorID--*sensorID==%d--\n", *sensorID);

	if (*sensorID != GC0310_SENSOR_ID) {
		*sensorID = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
	}

	return ERROR_NONE;
}

/*
 * TRE LETTURE, E BASTA UNA A SBAGLIARE. Il confronto e' rovesciato rispetto
 * a quel che ci si aspetta: il ciclo NON cerca finche' trova, gira tre volte
 * e ogni giro deve tornare 0xa310. Al primo diverso esce con l'errore
 * ("54000185 b.ne"@0xffffff80087172c4 va al ramo di fallimento), e solo dopo
 * i tre giri buoni stampa "OK" e inizializza.
 *
 * Il ritardo iniziale e' mdelay(10): dieci udelay(1000) srotolati.
 *
 * E se l'indice del sensore non e' 3 esce SUBITO, senza stampare niente --
 * il ramo a 0xffffff8008717304 mette 0x10 e salta all'epilogo.
 */
UINT32 GC0310MIPIOpen(void)
{
	kal_int8 i = 0;
	kal_uint16 sensor_id = 0;

	printk("<Jet> Entry GC0310MIPIOpen!!!\r\n");

	if (g9c90244 != 3)
		return ERROR_SENSOR_CONNECT_FAIL;

	mdelay(10);

	for (i = 0; i < 3; i++) {
		sensor_id = (GC0310MIPI_read_cmos_sensor_(0xf0) << 8)
			  | GC0310MIPI_read_cmos_sensor_(0xf1);
		if (sensor_id != GC0310_SENSOR_ID) {
			printk("GC0310MIPI Read Sensor ID Fail[open] = 0x%x\n",
			       sensor_id);
			return ERROR_SENSOR_CONNECT_FAIL;
		}
	}

	printk("GC0310MIPI_ Sensor Read ID OK \r\n");

	GC0310MIPI_Sensor_Init();

	return ERROR_NONE;
}

/*
 * TRENTASEI CAMPI, letti dagli spiazzamenti e non dai nomi.
 *
 * La fabbrica riempie la struttura con store larghe: "str x8, [x1]" mette
 * quattro u16 in un colpo (632, 474, 632, 474), "stur x9, [x1,#9]" ne mette
 * otto da un byte, e cosi' via. I nomi qui sotto vengono da
 * ACDK_SENSOR_INFO_STRUCT contando gli spiazzamenti, non da un driver
 * simile: e' l'unico modo, perche' due campi vicini dello stesso tipo non si
 * distinguono in nessun altro modo.
 *
 * I campi fra 139 e 142 -- i quattro ritardi dell'esposizione automatica --
 * NON vengono scritti. Non e' una dimenticanza mia: nel binario non c'e'
 * nessuna store verso quegli spiazzamenti, e riempirli farebbe crescere la
 * funzione.
 *
 * La copia finale va DALLA statica ALLA struttura del chiamante, al
 * contrario di quel che fanno Preview e Capture.
 */
/*
 * TRE SCRITTURE, E LA SECONDA NON VA A 0xFE. Il buffer di
 * due byte tiene l indirizzo in [sp,#4] e il dato in [sp,#5]: la seconda
 * scrittura cambia l indirizzo A 0x3C prima del ramo ("52800788 orr"@0xffffff800870e78c
 * seguito da `strb w8, [sp,#4]`) e poi solo il dato nei due rami. Avevo
 * letto il secondo strb e non il primo, e la funzione era 216 byte corta.
 *
 * E in coda lascia il valore dell'argomento in un globale.
 */
void GC0310MIPINightMode(kal_bool enable)
{
	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);

	if (enable) {
		GC0310MIPI_write_cmos_sensor(0x3c, 0x30);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		g9c9593c = 1;
	} else {
		GC0310MIPI_write_cmos_sensor(0x3c, 0x20);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		g9c9593c = 0;
	}
}

/*
 * IL SETTIMO PUNTATORE della SENSOR_FUNCTION_STRUCT -- quello che ALPS non
 * ha e che camera_main3_yuv_bv_show chiama a +48.
 *
 * Tre scritture e quattro letture, e SOLO LA PRIMA LETTURA SERVE: il valore
 * di ritorno e' quello di 0xEF ("394013f3 ldrb"@0xffffff800871aa54 e' l'unica
 * cosa che finisce in w19, e w19 e' quel che torna). Le altre tre si fanno e
 * si buttano. Non e' codice morto -- sono operazioni I2C, hanno effetto sul
 * chip -- ma il loro risultato non lo guarda nessuno.
 */
UINT32 GC0310ReadBV(void)
{
	kal_uint8 temp;

	GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
	GC0310MIPI_write_cmos_sensor(0x21, 0x16);
	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);

	temp = GC0310MIPI_read_cmos_sensor_(0xef);

	GC0310MIPI_read_cmos_sensor_(0x48);
	GC0310MIPI_read_cmos_sensor_(0xf0);
	GC0310MIPI_read_cmos_sensor_(0xf1);

	return temp;
}

/*
 * QUINDICI CASI SU SESSANTASEI, e gli altri cinquantuno cadono nel default.
 *
 * L'indice dello switch e' FeatureId - 3001 (SENSOR_FEATURE_START + 1), e la
 * tavola e' A BYTE ("3868692b ldrb"@0xffffff800871a4b0, senza `lsl`): 66
 * voci, 16 corpi distinti. tools/casiswitch.py la legge.
 *
 * Il caso della velocita' predefinita per scenario ha uno switch ANNIDATO su
 * cinque valori, e nella tavola a 0xffffff8008f49efd tutte e cinque le voci
 * valgono zero: tutti gli scenari cadono nello stesso corpo, che scrive 300.
 * Lo switch c'e' e non fa niente -- si riproduce com'e' (regola 7).
 */
/*
 * SETTE PASSI DI ESPOSIZIONE, e i sette valori sono una scala regolare:
 * 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50 da -3 a +3.
 *
 * QUALI sette lo dice un `ror`: "13027d08 ror"@0xffffff8008718f7c ruota
 * l'argomento di due prima del confronto con 6, quindi passano solo i
 * multipli di quattro -- 0, 4, 8, 12, 16, 20, 24. Nell'enum di ALPS quei
 * sette sono esattamente AE_EV_COMP_00, _10, _20, _30, n10, n20, n30: i
 * passi interi, senza i mezzi.
 *
 * Senza leggere il `ror` si sarebbe scritto un case per 0..6, che sono
 * AE_EV_COMP_00, _03, _05, _07, _10, _13, _15 -- sette nomi plausibili e
 * sbagliati.
 */
UINT32 GC0310MIPI_set_param_exposure(UINT16 para)
{
	switch (para) {
	case AE_EV_COMP_00:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x38);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_10:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x40);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_20:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x48);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_30:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x50);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_n10:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x30);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_n20:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x28);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_EV_COMP_n30:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x13, 0x20);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	default:
		break;
	}

	return TRUE;
}

/*
 * DUE SOLE FREQUENZE DI RETE, e il resto non fa niente: "7100051f cmp"
 * confronta con 1 e il ramo diverso esce senza scrivere. Diciassette
 * scritture per lato, e le ultime due sono in comune -- la fabbrica ne emette
 * 15 + 17 perche' il ramo a 50 Hz salta dentro la coda dell altro dopo aver
 * messo il suo valore per 0x2E, che e 0x74 contro 0x40.
 */
UINT32 GC0310MIPI_set_param_banding(UINT16 para)
{
	switch (para) {
	case AE_FLICKER_MODE_50HZ:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0x05, 0x02);
		GC0310MIPI_write_cmos_sensor(0x06, 0xd1);
		GC0310MIPI_write_cmos_sensor(0x07, 0x00);
		GC0310MIPI_write_cmos_sensor(0x08, 0x22);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x25, 0x00);
		GC0310MIPI_write_cmos_sensor(0x26, 0x6a);
		GC0310MIPI_write_cmos_sensor(0x27, 0x02);
		GC0310MIPI_write_cmos_sensor(0x28, 0x12);
		GC0310MIPI_write_cmos_sensor(0x29, 0x03);
		GC0310MIPI_write_cmos_sensor(0x2a, 0x50);
		GC0310MIPI_write_cmos_sensor(0x2b, 0x05);
		GC0310MIPI_write_cmos_sensor(0x2c, 0xcc);
		GC0310MIPI_write_cmos_sensor(0x2d, 0x07);
		GC0310MIPI_write_cmos_sensor(0x2e, 0x74);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	case AE_FLICKER_MODE_60HZ:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0x05, 0x02);
		GC0310MIPI_write_cmos_sensor(0x06, 0x60);
		GC0310MIPI_write_cmos_sensor(0x07, 0x00);
		GC0310MIPI_write_cmos_sensor(0x08, 0x58);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x25, 0x00);
		GC0310MIPI_write_cmos_sensor(0x26, 0x60);
		GC0310MIPI_write_cmos_sensor(0x27, 0x02);
		GC0310MIPI_write_cmos_sensor(0x28, 0x40);
		GC0310MIPI_write_cmos_sensor(0x29, 0x03);
		GC0310MIPI_write_cmos_sensor(0x2a, 0x60);
		GC0310MIPI_write_cmos_sensor(0x2b, 0x06);
		GC0310MIPI_write_cmos_sensor(0x2c, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2d, 0x08);
		GC0310MIPI_write_cmos_sensor(0x2e, 0x40);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		break;

	default:
		break;
	}

	return TRUE;
}

/*
 * TRENTASEI SCRITTURE: una prima del ramo, trentaquattro se il motivo di
 * prova si accende, una sola se si spegne. Il messaggio in testa stampa
 * l argomento mascherato a un bit ("12000021 and"@0xffffff80087194d4).
 */
UINT32 GC0310MIPISetTestPatternMode(kal_bool bEnable)
{
	printk("test pattern bEnable:=%d\n", bEnable);

	GC0310MIPI_write_cmos_sensor(0xfe, 0x00);

	if (bEnable) {
		GC0310MIPI_write_cmos_sensor(0x4f, 0x00);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x01);
		GC0310MIPI_write_cmos_sensor(0x0a, 0x45);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0x48, 0x00);
		GC0310MIPI_write_cmos_sensor(0x03, 0x03);
		GC0310MIPI_write_cmos_sensor(0x04, 0x50);
		GC0310MIPI_write_cmos_sensor(0x18, 0x1a);
		GC0310MIPI_write_cmos_sensor(0x40, 0x08);
		GC0310MIPI_write_cmos_sensor(0x41, 0x00);
		GC0310MIPI_write_cmos_sensor(0x42, 0x00);
		GC0310MIPI_write_cmos_sensor(0x70, 0x50);
		GC0310MIPI_write_cmos_sensor(0x71, 0x20);
		GC0310MIPI_write_cmos_sensor(0x72, 0x40);
		GC0310MIPI_write_cmos_sensor(0x77, 0x40);
		GC0310MIPI_write_cmos_sensor(0x78, 0x40);
		GC0310MIPI_write_cmos_sensor(0x79, 0x40);
		GC0310MIPI_write_cmos_sensor(0xd0, 0x40);
		GC0310MIPI_write_cmos_sensor(0xd1, 0x20);
		GC0310MIPI_write_cmos_sensor(0xd2, 0x20);
		GC0310MIPI_write_cmos_sensor(0x26, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2a, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2b, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2c, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2d, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2e, 0x00);
		GC0310MIPI_write_cmos_sensor(0x2f, 0x00);
		GC0310MIPI_write_cmos_sensor(0x30, 0x00);
		GC0310MIPI_write_cmos_sensor(0x31, 0x00);
		GC0310MIPI_write_cmos_sensor(0x9b, 0x00);
		GC0310MIPI_write_cmos_sensor(0x9e, 0xc0);
		GC0310MIPI_write_cmos_sensor(0xdd, 0x00);
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		GC0310MIPI_write_cmos_sensor(0x4c, 0x04);
	} else {
		GC0310MIPI_write_cmos_sensor(0x4c, 0x00);
	}

	return ERROR_NONE;
}

UINT32 GC0310MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
				MUINT8 *pFeaturePara, MUINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16 = (UINT16 *)pFeaturePara;
	UINT32 *pFeatureReturnPara32 = (UINT32 *)pFeaturePara;
	UINT32 *pFeatureData32 = (UINT32 *)pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData =
		(MSDK_SENSOR_REG_INFO_STRUCT *)pFeaturePara;

	switch (FeatureId) {
	case SENSOR_FEATURE_GET_RESOLUTION:
		*pFeatureReturnPara16++ = GC0310MIPI_IMAGE_SENSOR_FULL_WIDTH;
		*pFeatureReturnPara16 = GC0310MIPI_IMAGE_SENSOR_FULL_HEIGHT;
		*pFeatureParaLen = 4;
		break;

	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
		*pFeatureReturnPara32 = 0xFFFFFFFF;
		*pFeatureParaLen = 4;
		break;

	case SENSOR_FEATURE_SET_NIGHTMODE:
		GC0310MIPINightMode((BOOL)*pFeaturePara);
		break;

	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
		g9c959bc = *pFeatureData32;
		break;

	case SENSOR_FEATURE_SET_REGISTER:
		GC0310MIPI_write_cmos_sensor(pSensorRegData->RegAddr,
					     pSensorRegData->RegData);
		break;

	case SENSOR_FEATURE_GET_REGISTER:
		pSensorRegData->RegData =
			GC0310MIPI_read_cmos_sensor_(pSensorRegData->RegAddr);
		break;

	case SENSOR_FEATURE_GET_CONFIG_PARA:
		memcpy(pFeaturePara, &g9c95944,
		       sizeof(MSDK_SENSOR_CONFIG_STRUCT));
		*pFeatureParaLen = sizeof(MSDK_SENSOR_CONFIG_STRUCT);
		break;

	case SENSOR_FEATURE_GET_GROUP_COUNT:
		*pFeatureReturnPara32 = 0;
		*pFeatureParaLen = 4;
		break;

	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
		*pFeatureReturnPara32 = 0xFFFFFFFF;
		*pFeatureParaLen = 4;
		break;

	case SENSOR_FEATURE_SET_VIDEO_MODE:
		GC0310MIPIYUVSetVideoMode(*pFeatureReturnPara16);
		break;

	case SENSOR_FEATURE_CHECK_SENSOR_ID:
		GC0310MIPIGetSensorID(pFeatureReturnPara32);
		break;

	case SENSOR_FEATURE_SET_TEST_PATTERN:
		GC0310MIPISetTestPatternMode((BOOL)*pFeaturePara);
		break;

	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		switch (*pFeatureData32) {
		default:
			*(MUINT32 *)(uintptr_t)(*(pFeatureData32 + 1)) = 300;
			break;
		}
		break;

	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
		*pFeatureReturnPara32 = 0x8CA5EFA8;
		*pFeatureParaLen = 4;
		break;

	case SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO:
		GC0310MIPI_write_cmos_sensor(0xfe, 0x00);
		*pFeatureReturnPara32 =
			(GC0310MIPI_read_cmos_sensor_(0x48) & 0x0e) ? 1 : 0;
		break;

	default:
		break;
	}

	return ERROR_NONE;
}

UINT32 GC0310MIPIGetInfo(enum MSDK_SCENARIO_ID_ENUM ScenarioId,
			 MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
			 MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	pSensorInfo->SensorPreviewResolutionX =
		GC0310MIPI_IMAGE_SENSOR_PV_WIDTH;
	pSensorInfo->SensorPreviewResolutionY =
		GC0310MIPI_IMAGE_SENSOR_PV_HEIGHT;
	pSensorInfo->SensorFullResolutionX =
		GC0310MIPI_IMAGE_SENSOR_FULL_WIDTH;
	pSensorInfo->SensorFullResolutionY =
		GC0310MIPI_IMAGE_SENSOR_FULL_HEIGHT;

	pSensorInfo->SensorClockFreq = 24;
	pSensorInfo->SensorCameraPreviewFrameRate = 30;
	pSensorInfo->SensorVideoFrameRate = 30;
	pSensorInfo->SensorStillCaptureFrameRate = 10;
	pSensorInfo->SensorWebCamCaptureFrameRate = 15;

	pSensorInfo->SensorClockPolarity = 1;
	pSensorInfo->SensorClockFallingPolarity = 1;
	pSensorInfo->SensorClockRisingCount = 0;
	pSensorInfo->SensorClockFallingCount = 2;
	pSensorInfo->SensorClockDividCount = 3;
	pSensorInfo->SensorPixelClockCount = 3;
	pSensorInfo->SensorDataLatchCount = 2;
	pSensorInfo->SensorHsyncPolarity = 1;
	pSensorInfo->SensorVsyncPolarity = 1;
	pSensorInfo->SensorInterruptDelayLines = 1;

	pSensorInfo->SensorResetActiveHigh = FALSE;
	pSensorInfo->SensorResetDelayCount = 1;
	pSensorInfo->SensroInterfaceType = 1;
	pSensorInfo->SensorOutputDataFormat = 6;
	pSensorInfo->SensorMIPILaneNumber = 0;

	pSensorInfo->CaptureDelayFrame = 2;
	pSensorInfo->PreviewDelayFrame = 2;
	pSensorInfo->VideoDelayFrame = 4;

	pSensorInfo->SensorGrabStartX = 0;
	pSensorInfo->SensorGrabStartY = 1;

	pSensorInfo->SensorDrivingCurrent = 0;
	pSensorInfo->SensorMasterClockSwitch = 0;
	pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	pSensorInfo->SensorWidthSampling = 0;
	pSensorInfo->SensorHightSampling = 0;
	pSensorInfo->SensorPacketECCOrder = 1;

	g9c95940 = 3;

	memcpy(pSensorConfigData, &g9c95944,
	       sizeof(MSDK_SENSOR_CONFIG_STRUCT));

	return ERROR_NONE;
}

UINT32 GC0310MIPIYUVSetVideoMode(UINT16 u2FrameRate)
{
	g9c95930 = KAL_TRUE;

	if (u2FrameRate != 15 && u2FrameRate != 30)
		printk("Wrong Frame Rate");

	return TRUE;
}
/*
 * LA STRUTTURA CON SETTE PUNTATORI, e il settimo e' quello che ALPS non ha.
 *
 * GC0310ReadBV sta a +48, subito dopo SensorClose: e' il puntatore che
 * camera_main3_yuv_bv_show chiama, ed e' proprio perche' i driver di ALPS lo
 * lasciano NULL che accendere CONFIG_CUSTOM_KERNEL_IMGSENSOR senza questi
 * sensori introduce una chiamata a puntatore nullo raggiungibile da spazio
 * utente. Con GC0310 scritto, quel buco si chiude da se'.
 */
static struct SENSOR_FUNCTION_STRUCT SensorFuncGC0310MIPI = {
	GC0310MIPIOpen,
	GC0310MIPIGetInfo,
	GC0310MIPIGetResolution,
	GC0310MIPIFeatureControl,
	GC0310MIPIControl,
	GC0310MIPIClose,
	GC0310ReadBV
};

UINT32 GC0310_MIPI_YUV_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	if (pfFunc != NULL)
		*pfFunc = &SensorFuncGC0310MIPI;

	return ERROR_NONE;
}
