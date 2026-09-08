// SPDX-License-Identifier: GPL-2.0
/*
 * Magnetometro QST QMC7983 ("qmcX983" nei simboli di fabbrica) del Doogee
 * S88 Pro.
 *
 * Ricostruito leggendo il kernel di fabbrica disassemblato (l'oracolo,
 * docs/bringup/kernel-oracolo.md), non adattato da un altro telefono. Ogni
 * costante di questo file ha accanto la riga di disassemblato da cui viene,
 * nella forma <<testo>>@0xINDIRIZZO: se un valore non ha la sua citazione, non
 * e' stato letto ed e' un difetto.
 *
 * Il confine del driver e' 49 funzioni (48 nel blocco contiguo
 * 0xffffff800878dba4..0xffffff800878ffff piu' qmcX983_init nella .init.text),
 * verificato in entrambe le direzioni dalla revisione indipendente
 * docs/bringup/rapporti/revisione-qmcx983.md.
 *
 * *** Vincolo: -Werror sul sottoalbero ***
 * drivers/misc/mediatek/Makefile impone subdir-ccflags-y += -Werror, quindi
 * -Wunused-function e' un errore. Delle 49 funzioni solo 2 sono globali; le
 * altre 47 sono statiche e compilano soltanto se la catena
 *   module_init -> mag_driver_add(&qmcX983_init_info)
 *               -> qmcX983_local_init -> i2c_add_driver(&qmcX983_i2c_driver)
 *               -> qmcX983_i2c_probe -> tutto il resto
 * e' presente per intero. Per questo il lotto 2 e' l'intero driver e non un
 * sottoinsieme: non esiste un sottoinsieme proprio che compili.
 *
 * *** L'ordine delle funzioni ***
 * Non e' l'ordine di emissione del binario (che clang riordina): e' l'ordine
 * ricostruito dai __LINE__ che ogni MAGN_ERR incorpora. Il binario li porta
 * come immediati, e sono monotoni nel sorgente vero:
 *   mag_i2c_read_block 198/204, mag_i2c_write_block 227/239,
 *   store_layout_value 608-622, show_trace_value 654, store_trace_value
 *   668/678, store_WRregisters_value 714-723, store_registers_value 740/744,
 *   qmcX983_create_attr 852, qmcX983_unlocked_ioctl 1070,
 *   qmcX983_m_set_delay 1198/1204, qmcX983_m_enable 1221-1246,
 *   qmcX983_m_get_data 1268/1274, qmcX983_device_check 1472-1515,
 *   qmcx983_get_OTP 1549-1602, qmcX983_i2c_probe 1637-1746,
 *   qmcX983_i2c_remove 1761, qmcX983_local_init 1782/1788.
 * Nel binario invece qmcX983_i2c_probe sta nona e qmcX983_enable ultima:
 * l'ordine di emissione **non** e' l'ordine del sorgente, contrariamente a
 * quanto affermava la versione precedente di questo commento.
 *
 * *** Nomi ***
 * I nomi delle 49 funzioni vengono dalla symtab ricostruita dell'oracolo e
 * sono quelli di fabbrica. I nomi delle variabili statiche, dei tipi e delle
 * macro NON sono nel binario: quelli qui sotto sono scelti da chi ha
 * riscritto, e la sola cosa provata dal binario e' l'indirizzo, la dimensione
 * e l'uso di ciascuna. Dove il nome viene da una stringa del binario
 * (v_open_flag, open_flag, hw_registers, chip_id) e' annotato.
 */
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#include <hwmsen_helper.h>
#include <hwmsensor.h>
#include <sensors_io.h>

#include "cust_mag.h"
#include "mag.h"

/*
 * I due livelli di printk, letti dal prefisso di ogni formato:
 *   "\x016[QMC-Msensor] ..."          -> KERN_INFO, nessun argomento implicito
 *   "\x013[QMC-Msensor] %s %d : ..."  -> KERN_ERR, con __func__ e __LINE__
 * Prima citazione di ciascuno:
 *   <<\x016[QMC-Msensor] %s\n>>@0xffffff80091c1410
 * il frammento e' "%s\n", lo stesso testo che scnprintf usa altrove;
	 * qui sta a 0xffffff80091c1420, la citazione probante e'
	 * quella di 0xffffff8009226be1
 * il frammento e' "%s\n", lo stesso testo che scnprintf usa altrove;
	 * qui sta a 0xffffff80091c1420, la citazione probante e'
	 * quella di 0xffffff8009226be1
 *   <<\x013[QMC-Msensor] %s %d : add driver error\n>>@0xffffff80091c13af
 * il frammento che sta nel codice: "add driver error\n"@0xffffff80091c13c7
 */
#define MAGN_TAG	"[QMC-Msensor] "
#define MAGN_LOG(fmt, args...)	pr_info(MAGN_TAG fmt, ##args)
#define MAGN_ERR(fmt, args...)	\
	pr_err(MAGN_TAG "%s %d : " fmt, __func__, __LINE__, ##args)

/*
 * "qmcX983"@0xffffff80091c13a7, puntato dalle rilocazioni a
 * 0xffffff800992f138 (mag_init_info.name) e 0xffffff800992f198
 * (i2c_driver.driver.name).
 */
#define QMCX983_DEV_NAME	"qmcX983"

/*
 * <<orr w1,wzr,#0x20>>@0xffffff800878f558 (terzo argomento di scnprintf in
 * show_chipinfo_value): il buffer d'appoggio standard e' di 32 byte.
 */
#define QMCX983_BUFSIZE		0x20

/*
 * <<cmp w21,#0x9>>@0xffffff800878dd80 in I2C_RxData: la lettura fallisce da 9
 * byte in su, e il messaggio stampa 8 (<<orr w4,wzr,#0x8>>@0xffffff800878dda8).
 */
#define C_I2C_FIFO_SIZE		8

/*
 * <<mov w8,#0x2c ; strh w8,[x21,#2]>>@0xffffff800878e144: l'indirizzo I2C che
 * la probe forza in client->addr.
 */
#define QMCX983_I2C_ADDR	0x2c

/* I registri, ciascuno dal punto in cui il binario lo carica. */
#define QMCX983_REG_DATA	0x00	/* <<strb wzr,[sp]>>@0xffffff800878f67c */
#define QMCX983_REG_STATUS	0x06	/* <<orr w22,wzr,#0x6>>@0xffffff800878f648 */
#define QMCX983_REG_CTRL1	0x09	/* <<mov w21,#0x9>>@0xffffff800878dc10 */
#define QMCX983_REG_CTRL2	0x0a	/* <<mov w8,#0xc0a>>@0xffffff80087902c8 */
#define QMCX983_REG_FBR		0x0b	/* <<mov w8,#0xb>>@0xffffff800878df74 */
#define QMCX983_REG_CHIPID	0x0d	/* <<mov w8,#0xd>>@0xffffff800878e208 */
#define QMCX983_REG_OTP_ADDR	0x2e	/* <<mov w9,#0x2e>>@0xffffff800878e2b4 */
#define QMCX983_REG_OTP_DATA	0x2f	/* <<mov w8,#0x2f>>@0xffffff800878e2e4 */

/*
 * Gli identificativi di chip sono gli indici della tavola di salto di
 * show_chipinfo_value ("adr x10,0xffffff800878f4e0" con la tavola di byte
 * 00 05 09 0c 13 a 0xffffff8008f54570): 0..4, nell'ordine delle cinque
 * stringhe raggiunte.
 */
#define QMC6983_A1_D1		0	/* "QMC6983_A1_D1 Chip"@0xffffff80091c1981 */
#define QMC6983_E1		1	/* "QMC6983_E1 Chip"@0xffffff80091c195b */
#define QMC6983_E1_METAL	2	/* "QMC6983_E1_Metal Chip"@0xffffff80091c196b */
#define QMC7983_VERTICAL	3	/* "QMC7983_Vertical Chip"@0xffffff80091c1932 */
#define QMC7983_SLOPE		4	/* "QMC7983_Slope Chip"@0xffffff80091c1948 */

/*
 * QMCX983IO = 0x83 e i due comandi, dai due confronti di
 * qmcX983_unlocked_ioctl:
 *   <<mov w8,#0x8341 ; movk w8,#0xc008,lsl #16>>@0xffffff800878fb48 -> 0xc0088341
 *   <<mov w8,#0x8340 ; movk w8,#0x4008,lsl #16>>@0xffffff800878fb58 -> 0x40088340
 * cioe' _IOWR/_IOW(0x83, 0x41/0x40, char[8]). La dimensione codificata e' 8
 * mentre il buffer davvero copiato e' 16
 * (<<orr w2,wzr,#0x10>>@0xffffff800878fc64): la discrepanza e' di fabbrica.
 */
#define QMCX983IO		0x83
#define QMCX983_IOC_WRITE	_IOW(QMCX983IO, 0x40, char[8])
#define QMCX983_IOC_READ	_IOWR(QMCX983IO, 0x41, char[8])

/*
 * struct qmcX983_i2c_data: 64 byte
 * (<<orr w2,wzr,#0x40>>@0xffffff800878e124, terzo argomento di
 * kmem_cache_alloc_trace). Gli offset vengono da accessi osservati:
 *   +0   client       <<str x21,[x19]>>@0xffffff800878e214
 *   +8   hw           <<add x1,x19,#0x8>>@0xffffff800878e138 (get_mag_dts_func)
 *   +40  layout       <<str w3,[x19,#40]>>@0xffffff800878e1c0
 *   +44  trace        <<str wzr,[x19,#44]>>@0xffffff800878e1e0
 *   +48  cvt          <<add x1,x19,#0x30>>@0xffffff800878e150 (hwmsen_get_convert)
 *   +56  xy_sensitivity <<strh w8,[x10,#56]>>@0xffffff800878dc20
 *   +58  z_sensitivity  <<strh w8,[x10,#58]>>@0xffffff800878dc24
 * struct hwmsen_convert ha sign[4]/map[4] (C_MAX_HWMSEN_EVENT_NUM = 4),
 * coerente con i sign letti a +48/+49/+50 e i map a +52/+53/+54
 * ("ldrsb w9,[x20,#48]" e <<ldrb w8,[x20,#52]>>@0xffffff800878f6f0: fra i due
 * gruppi c'e' un byte di salto, che solo un array da 4 spiega).
 *
 * layout e trace sono qui atomic_t per convenzione del framework MediaTek:
 * il binario non li distingue da due int, perche' atomic_set/atomic_read
 * generano lo stesso str/ldr singolo.
 */
struct qmcX983_i2c_data {
	struct i2c_client *client;
	struct mag_hw hw;
	atomic_t layout;
	atomic_t trace;
	struct hwmsen_convert cvt;
	short xy_sensitivity;
	short z_sensitivity;
};

/*
 * struct mag_hw globale che get_cust_mag() restituisce. Non e' static: il
 * simbolo nell'oracolo e' T (globale), e nel driver "sibling" della stessa
 * famiglia MediaTek (akm09918.c, stesso albero, riga "struct mag_hw
 * mag_cust;") il campo e' definito allo stesso modo -- get_cust_mag ne
 * restituisce l'indirizzo direttamente (un solo adrp/add, nessun ldr
 * intermedio da un puntatore separato), che e' esattamente il pattern qui.
 * L'indirizzo reale (0xffffff8009cba228) e' in .bss di fabbrica.
 */
struct mag_hw mag_cust;

/*
 * .bss di fabbrica, ciascuna dal suo indirizzo:
 *   0xffffff8009cba248  this_client       <<ldr x22,[x8,#584]>>@0xffffff800878dd30
 *   0xffffff8009cba250  read_i2c_xyz      __mutex_init con <<&read_i2c_xyz>>@0xffffff80091c14d2
 *   0xffffff8009cba270  open_count        <<str wzr,[x8,#624]>>@0xffffff800878e020
 *   0xffffff8009cba274  qmcX983_init_flag <<ldrb w8,[x8,#628]>>@0xffffff800878e04c
 *   0xffffff8009cba278  sensor_data_mutex __mutex_init con <<&sensor_data_mutex>>@0xffffff80091c14bf
 *   0xffffff8009cba29c  otp_a             <<ldr w2,[x8,#668]>>@0xffffff800878fad0
 *   0xffffff8009cba2a0  otp_b             <<ldr w3,[x9,#672]>>@0xffffff800878fad4
 *   0xffffff8009cba2a4  open_flag         <<str w3,[x8,#676]>>@0xffffff800878eda0
 *   0xffffff8009cba2a8  v_open_flag       <<ldrb w8,[x19,#680]>>@0xffffff800878ed24
 *   0xffffff8009cba2ac  hw_registers      <<ldrb w8,[x8,#684]>>@0xffffff800878f21c
 * I nomi open_flag/v_open_flag/hw_registers vengono dalle stringhe di
 * fabbrica "open_flag = 0x%x, v_open_flag=0x%x\n"@0xffffff80091c1aeb e
 * "hw_registers = 0x%02x\n"@0xffffff80091c17ed.
 *
 * qmcX983_init_flag e' un byte, non un int: local_init lo legge con
 * <<ldrb w8,[x8,#628] ; tbz w8,#0>>@0xffffff800878e04c e la probe ci scrive 1
 * con <<strb w21,[x8,#628]>>@0xffffff800878ec30.
 *
 * v_open_flag e' un byte per lo stesso motivo (ldrb/strb, mai ldrh).
 *
 * sensor_data_mutex viene inizializzata dalla probe e non e' mai presa da
 * nessuna delle 49 funzioni: e' cosi' di fabbrica.
 */
static struct i2c_client *this_client;
static struct mutex read_i2c_xyz;
static struct mutex sensor_data_mutex;
static atomic_t open_count;
static bool qmcX983_init_flag;
static int otp_a;
static int otp_b;
/*
 * open_flag e' atomic_t e non int: qmcX983_m_enable gli assegna v_open_flag
 * (<<str w3,[x8,#676]>>@0xffffff800878eda0) e poi lo RILEGGE
 * (<<ldr w4,[x8,#676]>>@0xffffff800878eda4) per passarlo al printk. Su un int
 * semplice il compilatore avrebbe riusato w3: la rilettura e' la firma di
 * atomic_set/atomic_read (WRITE_ONCE/READ_ONCE).
 */
static atomic_t open_flag;
static unsigned char v_open_flag;
static unsigned char hw_registers;

/*
 * chip_id sta in .data (0xffffff800992f2a0) e vale 1 nell'immagine di
 * fabbrica (la parola a quell'indirizzo e' 0x0000000000000001, senza
 * rilocazione): e' inizializzata a QMC6983_E1, non a zero. Il ramo "default"
 * di qmcX983_device_check la lascia intatta, e la probe se ne accorge
 * (<<ldr w8,[x8,#672] ; cbz w8>>@0xffffff800878e390).
 */
static int chip_id = QMC6983_E1;

static int qmcX983_local_init(void);
static int qmcX983_local_remove(void);

/*
 * get_cust_mag: 0xffffff800878dba4, 12 byte, T (globale).
 *   adrp x0, 0xffffff8009cba000 ; add x0, x0, #0x228 ; ret
 * Restituisce &mag_cust (0xffffff8009cba228, in .bss di fabbrica).
 */
struct mag_hw *get_cust_mag(void)
{
	return &mag_cust;
}

/*
 * mag_i2c_read_block e mag_i2c_write_block non hanno un simbolo proprio: di
 * fabbrica sono sempre incorporate. Che siano funzioni del sorgente e non
 * macro lo dimostrano i loro __func__ e __LINE__, che restano nei printk:
 * <<mag_i2c_read_block>>@0xffffff80091c1317 alle righe 198 e 204,
 * <<mag_i2c_write_block>>@0xffffff80091c1365 alle righe 227 e 239.
 *
 * Sono "inline" perche' nell'oracolo non esiste nessun simbolo con quei nomi
 * pur avendo, la seconda, quindici punti di chiamata: senza "inline" clang
 * ne emette una copia fuori linea e qmcX983_unlocked_ioctl perde 192 byte
 * rispetto alla fabbrica (misurato).
 */
static inline int mag_i2c_read_block(struct i2c_client *client, u8 addr,
				     u8 *data, u8 len)
{
	u8 beg = addr;
	/* L'azzeramento esplicito non e' ridondanza di stile: la fabbrica lo
	 * esegue davvero prima di riempire i campi
	 * (<<str xzr,[sp,#8]>>@0xffffff800878dd44 e
	 *  <<stp xzr,xzr,[sp,#16]>>@0xffffff800878dd3c, entrambi seguiti dalle
	 * scritture vere dei campi). Con un inizializzatore designato completo
	 * clang non emette quei tre store e la funzione perde 12 byte
	 * (misurato: 332 invece di 344).
	 */
	struct i2c_msg msgs[2] = { {0}, {0} };
	int err;

	mutex_lock(&read_i2c_xyz);
	msgs[0].addr = client->addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &beg;

	msgs[1].addr = client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = len;
	msgs[1].buf = data;

	if (!client) {
		mutex_unlock(&read_i2c_xyz);
		return -EINVAL;
	} else if (len > C_I2C_FIFO_SIZE) {
		mutex_unlock(&read_i2c_xyz);
		/* "\x013[QMC-Msensor] %s %d :  length %d exceeds %d\n"
		 * @0xffffff80091c12e8, riga 198 (<<mov w2,#0xc6>>@0xffffff800878dda4).
 * il frammento che sta nel codice: " length %d exceeds %d\n"@0xffffff80091c1300
		 * Due spazi dopo i due punti: sono nel binario.
		 */
		MAGN_ERR(" length %d exceeds %d\n", len, C_I2C_FIFO_SIZE);
		return -EINVAL;
	}

	err = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (err != 2) {
		/* "\x013[QMC-Msensor] %s %d : i2c_transfer error: (%d %p %d) %d\n"
		 * @0xffffff80091c132a, riga 204 (<<mov w2,#0xcc>>@0xffffff800878ddf8).
 * il frammento che sta nel codice: "i2c_transfer error: (%d %p %d) %d\n"@0xffffff80091c1342
		 * Lo sblocco sta dentro i rami e non in coda: di fabbrica il
		 * ramo che riesce sblocca a 0xffffff800878ddd8 e ritorna subito,
		 * senza portarsi dietro err in un registro.
		 */
		MAGN_ERR("i2c_transfer error: (%d %p %d) %d\n",
#line 204
			 addr, data, len, err);
		mutex_unlock(&read_i2c_xyz);
		return -EIO;
	}
	mutex_unlock(&read_i2c_xyz);
	return 0;
}

static inline int mag_i2c_write_block(struct i2c_client *client, u8 addr,
				      u8 *data, u8 len)
{
	int err, idx, num;
	char buf[C_I2C_FIFO_SIZE];

	mutex_lock(&read_i2c_xyz);
	if (!client) {
		mutex_unlock(&read_i2c_xyz);
		return -EINVAL;
	} else if (len >= C_I2C_FIFO_SIZE) {
		/* <<cmp w19,#0x8 ; b.cc>>@0xffffff800878fbec: in scrittura il
		 * limite e' 8 escluso (si trasmette len+1 byte), mentre in
		 * lettura era 9 escluso. Riga 227
		 * (<<mov w2,#0xe3>>@0xffffff800878fc10).
		 */
		mutex_unlock(&read_i2c_xyz);
		MAGN_ERR(" length %d exceeds %d\n", len, C_I2C_FIFO_SIZE);
		return -EINVAL;
	}

	num = 0;
	buf[num++] = addr;
	for (idx = 0; idx < len; idx++)
		buf[num++] = data[idx];

	err = i2c_master_send(client, buf, num);
	/* Qui lo sblocco precede il controllo del segno: di fabbrica
	 * "mutex_unlock" sta a 0xffffff800878dc80 e il "tbnz w19,#31" solo
	 * dopo, a 0xffffff800878dc84.
	 */
	mutex_unlock(&read_i2c_xyz);
	if (err < 0) {
		/* "\x013[QMC-Msensor] %s %d : send command error!!\n"
		 * @0xffffff80091c1379, riga 239 (<<mov w2,#0xef>>@0xffffff800878dcb8)
 * il frammento che sta nel codice: "send command error!!\n"@0xffffff80091c1391
		 */
#line 239
		MAGN_ERR("send command error!!\n");
		return -EFAULT;
	}
	/* 0 e non err: i chiamanti scrivono "if (mag_i2c_write_block(...))" e
	 * di fabbrica il ramo che riesce restituisce 0
	 * (<<mov w0,wzr>>@0xffffff800878dc88 in qmcX983_set_range). Il binario
	 * non distingue questa forma da "return err" con un chiamante che
	 * testa "< 0": si e' scelta la prima.
	 */
	return 0;
}

/*
 * I2C_RxData: 0xffffff800878dcf0, 344 byte, t (statica).
 * I due controlli d'ingresso restituiscono -EINVAL
 * (<<mov w0,#0xffffffea>>@0xffffff800878dd10), ogni fallimento successivo -1
 * (<<mov w0,#0xffffffff>>@0xffffff800878de18).
 */
static int I2C_RxData(char *rxData, int length)
{
	if (rxData == NULL || length < 1)
		return -EINVAL;
	if (mag_i2c_read_block(this_client, rxData[0], (u8 *)rxData, length))
		return -1;
	return 0;
}

/*
 * I2C_TxData non ha simbolo proprio: e' sempre incorporata. Il controllo sul
 * puntatore resta visibile come <<cmn x22,#0x1 ; b.eq>>@0xffffff800878fbc4
 * dentro qmcX983_unlocked_ioctl, cioe' il confronto di &sData[1] con NULL.
 */
static int I2C_TxData(char *txData, int length)
{
	if (txData == NULL || length < 2)
		return -EINVAL;
	if (mag_i2c_write_block(this_client, txData[0],
				(u8 *)&txData[1], length - 1))
		return -1;
	return 0;
}

/*
 * qmcX983_set_range: 0xffffff800878dbb0, 320 byte, T (globale).
 */
int qmcX983_set_range(short range)
{
	int err = 0;
	unsigned char data[2];
	int ran;
	struct i2c_client *client = this_client;
	struct qmcX983_i2c_data *obj = i2c_get_clientdata(client);

	/* <<cmp w9,#0x3 ; b.hi>>@0xffffff800878dbd0 su (range & 0xffff): oltre 3
	 * si esce con -EINVAL (<<mov w0,#0xffffffea>>@0xffffff800878dc90)
	 */
	if ((unsigned short)range > 3)
		return -EINVAL;

	/* "mov x9,#0x2 ; movk x9,#0x8,lsl#16 ; movk x9,#0xc,lsl#32 ;
	 *  movk x9,#0x14,lsl#48"@0xffffff800878dbe4: la costante a 64 bit
	 * 0x0014000c00080002 viene scalata di (range*16) bit
	 * (<<ubfiz x8,x8,#4,#32 ; lsr x8,x9,x8>>@0xffffff800878dbfc) e troncata a
	 * 16 bit, cioe' e' la tabella {2, 8, 12, 20} indicizzata da range.
	 */
	ran = (int)((0x0014000c00080002ULL >> (range * 16)) & 0xffff);

	/* <<mov w11,#0x4e20>>@0xffffff800878dc08 = 20000 */
	obj->xy_sensitivity = 20000 / ran;
	obj->z_sensitivity = 20000 / ran;

	data[0] = QMCX983_REG_CTRL1;
	err = I2C_RxData(data, 1);

	/* <<and w8,w8,#0xffffffcf ; orr w8,w8,w19,lsl #4>>@0xffffff800878dc40 */
	data[1] = (data[0] & 0xcf) | (range << 4);
	data[0] = QMCX983_REG_CTRL1;

	/* Il valore restituito e' quello della scrittura, non quello della
	 * lettura: di fabbrica il ramo che riesce mette zero in w0
	 * (<<mov w0,wzr>>@0xffffff800878dc88) senza rileggere nulla.
	 */
	err = I2C_TxData(data, 2);
	return err;
}

/*
 * qmcX983_set_mode: 0xffffff800878de48, 244 byte, T (globale).
 */
int qmcX983_set_mode(unsigned char mode)
{
	int err = 0;
	unsigned char data[2];

	data[0] = QMCX983_REG_CTRL1;
	err = I2C_RxData(data, 1);
	/* <<and w8,w8,#0xfc ; orr w8,w8,w20>>@0xffffff800878de90 */
	data[1] = (data[0] & 0xfc) | mode;
	data[0] = QMCX983_REG_CTRL1;

	err = I2C_TxData(data, 2);
	return err;
}

/*
 * qmcX983_set_ratio: 0xffffff800878df3c, 196 byte, T (globale).
 * Nessuna lettura preliminare: scrive e basta.
 */
int qmcX983_set_ratio(unsigned char ratio)
{
	unsigned char data[2];

	data[0] = QMCX983_REG_FBR;
	data[1] = ratio;

	return I2C_TxData(data, 2);
}

/*
 * qmcX983_read_mag_xyz: 0xffffff800878f5f8, 356 byte, t.
 */
static int qmcX983_read_mag_xyz(int *data)
{
	int res;
	unsigned char mag_data[6];
	int hw_d[3] = {0};
	int t1 = 0;
	unsigned char rdy = 0;
	struct qmcX983_i2c_data *clientdata = i2c_get_clientdata(this_client);

	/* <<\x016[QMC-Msensor] %s\n>>@0xffffff80091c1410 con
 * il frammento e' "%s\n", lo stesso testo che scnprintf usa altrove;
	 * qui sta a 0xffffff80091c1420, la citazione probante e'
	 * quella di 0xffffff8009226be1
 * il frammento e' "%s\n", lo stesso testo che scnprintf usa altrove;
	 * qui sta a 0xffffff80091c1420, la citazione probante e'
	 * quella di 0xffffff8009226be1
	 * <<qmcX983_read_mag_xyz>>@0xffffff80091c199f
	 */
	MAGN_LOG("%s\n", __func__);

	/* <<cmp w21,#0x2 ; b.hi>>@0xffffff800878f65c con w21 inizializzata a 1
	 * (<<orr w21,wzr,#0x1>>@0xffffff800878f644): al massimo tre letture.
	 * <<and w8,w8,#0x7>>@0xffffff800878f66c: si esce appena uno dei tre bit
	 * bassi del registro di stato e' alto.
	 */
	while (!(rdy & 0x07) && (t1 < 3)) {
		mag_data[0] = QMCX983_REG_STATUS;
		res = I2C_RxData(mag_data, 1);
		rdy = mag_data[0];
		t1++;
	}

	mag_data[0] = QMCX983_REG_DATA;
	res = I2C_RxData(mag_data, 6);
	if (res != 0)
		return res;

	/* <<lsl w9,w9,#8 ; sxth w9,w9 ; orr w8,w9,w8>>@0xffffff800878f698 */
	hw_d[0] = (short)(mag_data[1] << 8 | mag_data[0]);
	hw_d[1] = (short)(mag_data[3] << 8 | mag_data[2]);
	hw_d[2] = (short)(mag_data[5] << 8 | mag_data[4]);

	/* <<mov w11,#0x3e8>>@0xffffff800878f6b8 = 1000; il divisore di x e y e'
	 * il campo a +56, quello di z il campo a +58
	 * (<<ldrsh w9,[x20,#56]>>@0xffffff800878f6a4,
	 *  <<ldrsh w8,[x20,#58]>>@0xffffff800878f6d8)
	 */
	hw_d[0] = hw_d[0] * 1000 / clientdata->xy_sensitivity;
	hw_d[1] = hw_d[1] * 1000 / clientdata->xy_sensitivity;
	hw_d[2] = hw_d[2] * 1000 / clientdata->z_sensitivity;

	data[0] = hw_d[clientdata->cvt.map[0]] * clientdata->cvt.sign[0];
	data[1] = hw_d[clientdata->cvt.map[1]] * clientdata->cvt.sign[1];
	data[2] = hw_d[clientdata->cvt.map[2]] * clientdata->cvt.sign[2];

	return res;
}

/*
 * qmcX983_enable: 0xffffff800879014c, 964 byte, t.
 * Il registro w0 non viene mai riscritto prima dell'epilogo: la funzione non
 * restituisce nulla, ed e' quindi void -- coerente col fatto che nessuno dei
 * due chiamanti (qmcX983_m_enable, qmcX983_factory_enable_sensor) ne usi il
 * risultato. Una funzione dichiarata int che cade fuori senza return sarebbe
 * un errore, qui, per -Wreturn-type dentro -Werror.
 */
static void qmcX983_enable(void)
{
	unsigned char databuf[2];
	struct i2c_client *client = this_client;

	/* <<mov w8,#0x121>>@0xffffff8008790184 -> {0x21, 0x01} */
	databuf[0] = 0x21;
	databuf[1] = 0x01;
	mag_i2c_write_block(client, databuf[0], &databuf[1], 1);

	/* <<mov w8,#0x4020>>@0xffffff80087901ec -> {0x20, 0x40} */
	databuf[0] = 0x20;
	databuf[1] = 0x40;
	mag_i2c_write_block(this_client, databuf[0], &databuf[1], 1);

	/* <<ldr w8,[x21,#672] ; cbz w8>>@0xffffff8008790244 */
	if (chip_id != QMC6983_A1_D1) {
		/* <<mov w8,#0x8029>>@0xffffff8008790260 -> {0x29, 0x80} */
		databuf[0] = 0x29;
		databuf[1] = 0x80;
		mag_i2c_write_block(this_client, databuf[0], &databuf[1], 1);

		/* <<mov w8,#0xc0a>>@0xffffff80087902c8 -> {0x0a, 0x0c} */
		databuf[0] = QMCX983_REG_CTRL2;
		databuf[1] = 0x0c;
		mag_i2c_write_block(this_client, databuf[0], &databuf[1], 1);

		/* "cmp w8,#0x4 ; b.eq" e "cmp w8,#0x2 ; b.ne"
		 * @0xffffff8008790320
		 */
		if (chip_id == QMC7983_SLOPE || chip_id == QMC6983_E1_METAL) {
			/* <<mov w8,#0x801b>>@0xffffff8008790344 -> {0x1b, 0x80} */
			databuf[0] = 0x1b;
			databuf[1] = 0x80;
			mag_i2c_write_block(this_client, databuf[0],
					    &databuf[1], 1);
		}
	}

	/* <<mov w8,#0x1d09>>@0xffffff80087903ac -> {0x09, 0x1d} */
	databuf[0] = QMCX983_REG_CTRL1;
	databuf[1] = 0x1d;
	mag_i2c_write_block(this_client, databuf[0], &databuf[1], 1);

	/* <<orr w0,wzr,#0x1 ; bl qmcX983_set_range>>@0xffffff8008790400 */
	qmcX983_set_range(1);

	/* <<mov w8,#0x10b>>@0xffffff800879041c -> {0x0b, 0x01} */
	databuf[0] = QMCX983_REG_FBR;
	databuf[1] = 0x01;
	mag_i2c_write_block(this_client, databuf[0], &databuf[1], 1);

	/* <<mov w0,#0x4e20 ; mov w1,#0x7530>>@0xffffff8008790470 */
	usleep_range(20000, 30000);

	databuf[0] = QMCX983_REG_CTRL1;
	databuf[1] = 0x1d;
	mag_i2c_write_block(this_client, databuf[0], &databuf[1], 1);
}

/*
 * FctShipmntTestProcess_Body: 0xffffff800878e000, 8 byte, T (globale).
 *   orr w0, wzr, #0x1 ; ret
 * Nome e linkage globale presi dall'oracolo cosi' come sono.
 */
int FctShipmntTestProcess_Body(void)
{
	return 1;
}

/*
 * show_layout_value: 0xffffff800878f75c, 104 byte, t.
 */
static ssize_t show_layout_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = this_client;
	struct qmcX983_i2c_data *data = i2c_get_clientdata(client);

	/* "(%d, %d)\n[%+2d %+2d %+2d]\n[%+2d %+2d %+2d]\n"@0xffffff80091c19b4;
	 * <<orr w1,wzr,#0x1000>>@0xffffff800878f780 = PAGE_SIZE
	 */
	return scnprintf(buf, PAGE_SIZE,
			 "(%d, %d)\n[%+2d %+2d %+2d]\n[%+2d %+2d %+2d]\n",
			 data->hw.direction, atomic_read(&data->layout),
			 data->cvt.sign[0], data->cvt.sign[1],
			 data->cvt.sign[2],
			 data->cvt.map[0], data->cvt.map[1],
			 data->cvt.map[2]);
}

/*
 * store_layout_value: 0xffffff800878f7c4, 304 byte, t.
 */
static ssize_t store_layout_value(struct device_driver *ddri,
				  const char *buf, size_t count)
{
	struct i2c_client *client = this_client;
	struct qmcX983_i2c_data *data = i2c_get_clientdata(client);
	int layout = 0;

	/* "%d"@0xffffff8009216030 */
	if (sscanf(buf, "%d", &layout) == 1) {
		atomic_set(&data->layout, layout);
		if (!hwmsen_get_convert(layout, &data->cvt)) {
			/* "\x013[QMC-Msensor] %s %d : HWMSEN_GET_CONVERT function error!\r\n"
			 * @0xffffff80091c19e0, riga 608
 * il frammento che sta nel codice: "HWMSEN_GET_CONVERT function error!\r\n"@0xffffff80091c19f8
			 * (<<mov w2,#0x260>>@0xffffff800878f8a0)
			 */
#line 608
			MAGN_ERR("HWMSEN_GET_CONVERT function error!\r\n");
		} else if (!hwmsen_get_convert(data->hw.direction, &data->cvt)) {
			/* "\x013[QMC-Msensor] %s %d : invalid layout: %d, restore to %d\n"
			 * @0xffffff80091c1a30, riga 612
 * il frammento che sta nel codice: "invalid layout: %d, restore to %d\n"@0xffffff80091c1a48
			 * (<<mov w2,#0x264>>@0xffffff800878f8bc)
			 */
			MAGN_ERR("invalid layout: %d, restore to %d\n",
#line 612
				 layout, data->hw.direction);
		} else {
			/* "\x013[QMC-Msensor] %s %d : invalid layout: (%d, %d)\n"
			 * @0xffffff80091c1a6b, riga 616
 * il frammento che sta nel codice: "invalid layout: (%d, %d)\n"@0xffffff80091c1a83
			 * (<<mov w2,#0x268>>@0xffffff800878f858)
			 */
			MAGN_ERR("invalid layout: (%d, %d)\n",
#line 616
				 layout, data->hw.direction);
			hwmsen_get_convert(0, &data->cvt);
		}
	} else {
		/* "\x013[QMC-Msensor] %s %d : invalid format = '%s'\n"
		 * @0xffffff80091c1a9d, riga 622 (<<mov w2,#0x26e>>@0xffffff800878f880)
 * il frammento che sta nel codice: "invalid format = '%s'\n"@0xffffff80091c1ab5
		 */
#line 622
		MAGN_ERR("invalid format = '%s'\n", buf);
	}
	return count;
}

/*
 * show_trace_value: 0xffffff800878f990, 96 byte, t.
 */
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	struct qmcX983_i2c_data *data = i2c_get_clientdata(this_client);

	if (data == NULL) {
		/* "\x013[QMC-Msensor] %s %d : qmcX983_i2c_data is null!!\n"
		 * @0xffffff80091c1804, riga 654 (<<mov w2,#0x28e>>@0xffffff800878f9dc)
 * il frammento che sta nel codice: "qmcX983_i2c_data is null!!\n"@0xffffff80091c181c
		 */
#line 654
		MAGN_ERR("qmcX983_i2c_data is null!!\n");
		return -EINVAL;
	}
	/* "0x%04X\n"@0xffffff80091be5fe */
	return scnprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&data->trace));
}

/*
 * store_trace_value: 0xffffff800878f9f0, 208 byte, t.
 */
static ssize_t store_trace_value(struct device_driver *ddri,
				 const char *buf, size_t count)
{
	struct qmcX983_i2c_data *data = i2c_get_clientdata(this_client);
	/* Non inizializzata: la fabbrica non azzera questa variabile prima
	 * della sscanf -- in store_layout_value invece lo fa
	 * (<<str wzr,[sp,#4]>>@0xffffff800878f808), qui quello store non c'e'.
	 * Azzerarla costa 4 byte in piu' (misurato: 212 invece di 208).
	 */
	int trace;

	if (data == NULL) {
		/* riga 668 (<<mov w2,#0x29c>>@0xffffff800878fa60) */
#line 668
		MAGN_ERR("qmcX983_i2c_data is null!!\n");
		return -EINVAL;
	}
	/* "0x%x"@0xffffff8009142911 */
	if (sscanf(buf, "0x%x", &trace) == 1) {
		atomic_set(&data->trace, trace);
	} else {
		/* "\x013[QMC-Msensor] %s %d : invalid content: '%s', length = %zd\n"
		 * @0xffffff80091c1b32, riga 678 (<<mov w2,#0x2a6>>@0xffffff800878fa80)
 * il frammento che sta nel codice: "invalid content: '%s', length = %zd\n"@0xffffff80091c1b4a
		 */
#line 678
		MAGN_ERR("invalid content: '%s', length = %zd\n", buf, count);
	}
	return count;
}

/*
 * show_WRregisters_value: 0xffffff800878f1e4, 172 byte, t.
 */
static ssize_t show_WRregisters_value(struct device_driver *ddri, char *buf)
{
	unsigned char databuf[2];
	int res;

	MAGN_LOG("%s\n", __func__);
	databuf[0] = hw_registers;
	res = I2C_RxData(databuf, 1);
	if (res != 0) {
		/* <<mov x0,#0xfffffffffffffff2>>@0xffffff800878f234 = -EFAULT */
		return -EFAULT;
	}
	/* <<\x016[QMC-Msensor] QMCX983 hw_registers = 0x%02x\n>>@0xffffff80091c17d5
	 * il frammento che sta nel codice: "QMCX983 hw_registers = 0x%02x\n"@0xffffff80091c17e5
	 */
	MAGN_LOG("QMCX983 hw_registers = 0x%02x\n", databuf[0]);
	/* "hw_registers = 0x%02x\n"@0xffffff80091c17ed */
	return scnprintf(buf, PAGE_SIZE, "hw_registers = 0x%02x\n", databuf[0]);
}

/*
 * store_WRregisters_value: 0xffffff800878f290, 316 byte, t.
 */
static ssize_t store_WRregisters_value(struct device_driver *ddri,
				       const char *buf, size_t count)
{
	struct i2c_client *client = this_client;
	struct qmcX983_i2c_data *data = i2c_get_clientdata(client);
	unsigned char databuf[2];

	if (data == NULL) {
		/* riga 714 (<<mov w2,#0x2ca>>@0xffffff800878f35c) */
#line 714
		MAGN_ERR("qmcX983_i2c_data is null!!\n");
		return -EINVAL;
	}
	/* "\x013[QMC-Msensor] %s %d : QMC6938:store_WRregisters_value: 0x%2x \n"
	 * @0xffffff80091c1850, riga 718 (<<mov w2,#0x2ce>>@0xffffff800878f2dc)
 * il frammento che sta nel codice: "QMC6938:store_WRregisters_value: 0x%2x \n"@0xffffff80091c1868
	 */
#line 718
	MAGN_ERR("QMC6938:store_WRregisters_value: 0x%2x \n", buf[0]);

	databuf[0] = hw_registers;
	databuf[1] = buf[0];
	mag_i2c_write_block(client, databuf[0], &databuf[1], 1);

	/* "\x013[QMC-Msensor] %s %d : QMC6938: write registers 0x%2x  ---> 0x%2x success! \n"
	 * @0xffffff80091c1891, riga 723 (<<mov w2,#0x2d3>>@0xffffff800878f38c);
 * il frammento che sta nel codice: "QMC6938: write registers 0x%2x  ---> 0x%2x success! \n"@0xffffff80091c18a9
	 * il primo argomento e' hw_registers riletto
	 * (<<ldrb w3,[x22,#684]>>@0xffffff800878f378)
	 */
	MAGN_ERR("QMC6938: write registers 0x%2x  ---> 0x%2x success! \n",
#line 723
		 hw_registers, buf[0]);
	return count;
}

/*
 * show_registers_value: 0xffffff800878f3cc, 80 byte, t.
 */
static ssize_t show_registers_value(struct device_driver *ddri, char *buf)
{
	MAGN_LOG("QMCX983 hw_registers = 0x%02x\n", hw_registers);
	return scnprintf(buf, PAGE_SIZE, "hw_registers = 0x%02x\n",
			 hw_registers);
}

/*
 * store_registers_value: 0xffffff800878f41c, 116 byte, t.
 */
static ssize_t store_registers_value(struct device_driver *ddri,
				     const char *buf, size_t count)
{
	struct qmcX983_i2c_data *data = i2c_get_clientdata(this_client);

	if (data == NULL) {
		/* riga 740 (<<mov w2,#0x2e4>>@0xffffff800878f474) */
#line 740
		MAGN_ERR("qmcX983_i2c_data is null!!\n");
		return -EINVAL;
	}
	hw_registers = buf[0];
	/* "\x013[QMC-Msensor] %s %d : QMC6938: REGISTERS = 0x%2x\n"
	 * @0xffffff80091c18f5, riga 744 (<<mov w2,#0x2e8>>@0xffffff800878f454)
 * il frammento che sta nel codice: "QMC6938: REGISTERS = 0x%2x\n"@0xffffff80091c190d
	 */
#line 744
	MAGN_ERR("QMC6938: REGISTERS = 0x%2x\n", buf[0]);
	return count;
}

/*
 * show_chipinfo_value: 0xffffff800878f490, 248 byte, t.
 */
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	char strbuf[QMCX983_BUFSIZE];

	if (this_client == NULL) {
		/* <<strb wzr,[sp,#8]>>@0xffffff800878f4ec: un solo byte azzerato,
		 * non tutto il buffer
		 */
		strbuf[0] = '\0';
	} else {
		switch (chip_id) {
		case QMC6983_A1_D1:
			strcpy(strbuf, "QMC6983_A1_D1 Chip");
			break;
		case QMC6983_E1:
			strcpy(strbuf, "QMC6983_E1 Chip");
			break;
		case QMC6983_E1_METAL:
			strcpy(strbuf, "QMC6983_E1_Metal Chip");
			break;
		case QMC7983_VERTICAL:
			strcpy(strbuf, "QMC7983_Vertical Chip");
			break;
		case QMC7983_SLOPE:
			strcpy(strbuf, "QMC7983_Slope Chip");
			break;
		}
	}
	/* "%s\n"@0xffffff8009226be1 */
	return scnprintf(buf, QMCX983_BUFSIZE, "%s\n", strbuf);
}

/*
 * show_sensordata_value: 0xffffff800878f588, 112 byte, t.
 */
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	int sensordata[3];

	qmcX983_read_mag_xyz(sensordata);
	/* "%d %d %d\n"@0xffffff8009212de0 */
	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n",
			 sensordata[0], sensordata[1], sensordata[2]);
}

/*
 * show_status_value: 0xffffff800878f8f4, 156 byte, t.
 */
static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
	struct qmcX983_i2c_data *data = i2c_get_clientdata(this_client);
	ssize_t len = 0;

	/* "CUST: %d %d (%d %d)\n"@0xffffff80091c1acc, argomenti letti come
	 * "ldp w3,w4,[x8,#8]" e <<ldp w5,w6,[x8,#16]>>@0xffffff800878f928
	 */
	len += scnprintf(buf + len, PAGE_SIZE - len, "CUST: %d %d (%d %d)\n",
			 data->hw.i2c_num, data->hw.direction,
			 data->hw.power_id, data->hw.power_vol);
	/* "OPEN: %d\n"@0xffffff80091c1ae1 */
	len += scnprintf(buf + len, PAGE_SIZE - len, "OPEN: %d\n",
			 atomic_read(&open_count));
	/* "open_flag = 0x%x, v_open_flag=0x%x\n"@0xffffff80091c1aeb */
	len += scnprintf(buf + len, PAGE_SIZE - len,
			 "open_flag = 0x%x, v_open_flag=0x%x\n",
			 atomic_read(&open_flag), v_open_flag);
	return len;
}

/*
 * show_regiter_map: 0xffffff800878f0d0, 276 byte, t. Il nome ha il refuso di
 * fabbrica ("regiter"), preso dalla symtab dell'oracolo cosi' com'e'.
 */
static ssize_t show_regiter_map(struct device_driver *ddri, char *buf)
{
	/* <<orr w1,wzr,#0x18>>@0xffffff800878f15c = 24 e
	 * <<mov w25,#0x12c>>@0xffffff800878f12c = 300
	 */
	unsigned char databuf[2];
	char tmpbuf[24];
	char strbuf[300];
	int i, res, len;

	MAGN_LOG("%s\n", __func__);
	/* <<cmp w20,#0xc>>@0xffffff800878f18c */
	for (i = 0; i < 12; i++) {
		databuf[0] = i;
		res = I2C_RxData(databuf, 1);
		if (res < 0) {
			/* "\x016[QMC-Msensor] QMCX983 dump registers 0x%02x failed !\n"
			 * @0xffffff80091c175d
 * il frammento che sta nel codice: "QMCX983 dump registers 0x%02x failed !\n"@0xffffff80091c176d
			 */
			MAGN_LOG("QMCX983 dump registers 0x%02x failed !\n", i);
		}
		/* "reg[0x%2x] =  0x%2x \n"@0xffffff80091c1795 */
		len = scnprintf(tmpbuf, sizeof(tmpbuf),
				"reg[0x%2x] =  0x%2x \n", i, databuf[0]);
		/* <<mul w8,w0,w20>>@0xffffff800878f16c: l'offset e' len*i, non la
		 * somma delle lunghezze -- e' cosi' di fabbrica.
		 * "  %s \n"@0xffffff80091c17ab
		 */
		snprintf(strbuf + len * i, sizeof(strbuf) - len * i,
			 "  %s \n", tmpbuf);
	}
	return scnprintf(buf, sizeof(strbuf), "%s\n", strbuf);
}

/*
 * show_shipment_test: 0xffffff800878f054, 116 byte, t.
 */
static ssize_t show_shipment_test(struct device_driver *ddri, char *buf)
{
	char strbuf[QMCX983_BUFSIZE];

	/* <<\x016[QMC-Msensor] shipment_test pass\n>>@0xffffff80091c1728
	 * il frammento che sta nel codice: "shipment_test pass\n"@0xffffff80091c1738
	 */
	MAGN_LOG("shipment_test pass\n");
	/* <<mov w8,#0x79 ; strh w8,[sp,#12]>>@0xffffff800878f084: due byte, 'y'
	 * e il terminatore
	 */
	strcpy(strbuf, "y");
	return sprintf(buf, "%s\n", strbuf);
}

/*
 * store_shipment_test: 0xffffff800878f0c8, 8 byte, t.
 *   mov x0, x2 ; ret
 */
static ssize_t store_shipment_test(struct device_driver *ddri,
				   const char *buf, size_t count)
{
	return count;
}

/*
 * show_OTP_value: 0xffffff800878fac0, 52 byte, t.
 */
static ssize_t show_OTP_value(struct device_driver *ddri, char *buf)
{
	/* "%d,%d\n"@0xffffff8009204fcf */
	return sprintf(buf, "%d,%d\n", otp_a, otp_b);
}

/*
 * Gli attributi sysfs. Nomi e permessi vengono dalle rilocazioni di
 * .rela.dyn, non letti a occhio -- il campo mode non e' relocato e si legge
 * dall'immagine all'indirizzo dell'attributo + 8:
 *   0xffffff800992f2a8 <<shipmenttest>>@0xffffff80091c171b  mode 0x1a4 = 0644
 *   0xffffff800992f2c8 <<regmap>>@0xffffff800917e035        mode 0x124 = 0444
 *   0xffffff800992f2e8 <<WRregisters>>@0xffffff80091c17b2   mode 0x1a4 = 0644
 *   0xffffff800992f308 <<registers>>@0xffffff80091c17b4     mode 0x1a4 = 0644
 *   0xffffff800992f328 <<chipinfo>>@0xffffff80091c1929      mode 0x124 = 0444
 *   0xffffff800992f348 <<sensordata>>@0xffffff80091c1994    mode 0x124 = 0444
 *   0xffffff800992f368 <<layout>>@0xffffff80090e39dc        mode 0x1a4 = 0644
 *   0xffffff800992f388 <<status>>@0xffffff8009228dda        mode 0x124 = 0444
 *   0xffffff800992f3a8 <<trace>>@0xffffff80090f369a         mode 0x1a4 = 0644
 *   0xffffff800992f3c8 <<otp>>@0xffffff8009292fc1           mode 0x124 = 0444
 * L'ordine dell'elenco e' quello dei dieci driver_remove_file srotolati in
 * qmcX983_delete_attr (0x2a8, 0x2c8, 0x2e8, 0x308, 0x328, 0x348, 0x368,
 * 0x388, 0x3a8, 0x3c8).
 */
static struct driver_attribute driver_attr_shipmenttest =
	__ATTR(shipmenttest, 0644, show_shipment_test, store_shipment_test);
static struct driver_attribute driver_attr_regmap =
	__ATTR(regmap, 0444, show_regiter_map, NULL);
static struct driver_attribute driver_attr_WRregisters =
	__ATTR(WRregisters, 0644, show_WRregisters_value,
	       store_WRregisters_value);
static struct driver_attribute driver_attr_registers =
	__ATTR(registers, 0644, show_registers_value, store_registers_value);
static struct driver_attribute driver_attr_chipinfo =
	__ATTR(chipinfo, 0444, show_chipinfo_value, NULL);
static struct driver_attribute driver_attr_sensordata =
	__ATTR(sensordata, 0444, show_sensordata_value, NULL);
static struct driver_attribute driver_attr_layout =
	__ATTR(layout, 0644, show_layout_value, store_layout_value);
static struct driver_attribute driver_attr_status =
	__ATTR(status, 0444, show_status_value, NULL);
static struct driver_attribute driver_attr_trace =
	__ATTR(trace, 0644, show_trace_value, store_trace_value);
static struct driver_attribute driver_attr_otp =
	__ATTR(otp, 0444, show_OTP_value, NULL);

static struct driver_attribute *qmcX983_attr_list[] = {
	&driver_attr_shipmenttest,
	&driver_attr_regmap,
	&driver_attr_WRregisters,
	&driver_attr_registers,
	&driver_attr_chipinfo,
	&driver_attr_sensordata,
	&driver_attr_layout,
	&driver_attr_status,
	&driver_attr_trace,
	&driver_attr_otp,
};

/*
 * qmcX983_create_attr: nessun simbolo proprio, sempre incorporata nella
 * probe; nome e riga restano nel printk
 * (<<qmcX983_create_attr>>@0xffffff80091c1707, riga 852).
 */
static int qmcX983_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(qmcX983_attr_list);

	if (driver == NULL)
		return -EINVAL;
	for (idx = 0; idx < num; idx++) {
		err = driver_create_file(driver, qmcX983_attr_list[idx]);
		if (err) {
			/* "\x013[QMC-Msensor] %s %d : driver_create_file (%s) = %d\n"
			 * @0xffffff80091c16d1, riga 852
 * il frammento che sta nel codice: "driver_create_file (%s) = %d\n"@0xffffff80091c16e9
			 * (<<mov w2,#0x354>>@0xffffff800878e860); il primo
			 * argomento e' attr->attr.name
			 * (<<ldr x3,[x21]>>@0xffffff800878e848)
			 */
			MAGN_ERR("driver_create_file (%s) = %d\n",
#line 852
				 qmcX983_attr_list[idx]->attr.name, err);
			break;
		}
	}
	return err;
}

/*
 * qmcX983_delete_attr: 0xffffff800878ef8c, 200 byte, t.
 * I dieci driver_remove_file sono srotolati nell'ordine esatto della tabella
 * di rilocazioni 0xffffff800992f2a8..0xffffff800992f3c8.
 */
static int qmcX983_delete_attr(struct device_driver *driver)
{
	int idx;
	int num = ARRAY_SIZE(qmcX983_attr_list);

	if (driver == NULL)
		return -EINVAL;
	for (idx = 0; idx < num; idx++)
		driver_remove_file(driver, qmcX983_attr_list[idx]);
	return 0;
}

/*
 * qmcX983_unlocked_ioctl: 0xffffff800878faf4, 852 byte, t.
 */
static long qmcX983_unlocked_ioctl(struct file *file, unsigned int cmd,
				   unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	char sData[16];
	/* ret e' int e non long: di fabbrica l'estensione di segno compare solo
	 * sul ramo d'errore (<<sxtw x0,w0>>@0xffffff800878fd70), cioe' alla
	 * conversione del valore di ritorno, non subito dopo la chiamata.
	 */
	int ret = 0;
	struct qmcX983_i2c_data *data = i2c_get_clientdata(this_client);

	/* <<ldr w8,[x8,#44] ; tbz w8,#0>>@0xffffff800878fb30: bit 0 di trace */
	if (data && (atomic_read(&data->trace) & 0x01)) {
		/* "\x016[QMC-Msensor] qmcX983_unlocked_ioctl !cmd= 0x%x\n"
		 * @0xffffff80091c1b7b
 * il frammento che sta nel codice: "qmcX983_unlocked_ioctl !cmd= 0x%x\n"@0xffffff80091c1b8b
		 */
		MAGN_LOG("qmcX983_unlocked_ioctl !cmd= 0x%x\n", cmd);
	}

	switch (cmd) {
	case QMCX983_IOC_WRITE:
		if (argp == NULL) {
			/* <<\x016[QMC-Msensor] invalid argument.>>@0xffffff80091c1bae
			 * il frammento che sta nel codice: "invalid argument."@0xffffff80091c1bbe
			 */
			MAGN_LOG("invalid argument.");
			return -EINVAL;
		}
		if (copy_from_user(sData, argp, sizeof(sData))) {
			/* "\x016[QMC-Msensor] copy_from_user failed."
			 * @0xffffff80091c1bd0
 * il frammento che sta nel codice: "copy_from_user failed."@0xffffff80091c1be0
			 */
			MAGN_LOG("copy_from_user failed.");
			return -EFAULT;
		}
		/* <<sub w8,w19,#0x2 ; cmp w8,#0xe ; b.cs>>@0xffffff800878fbb4:
		 * ammesso solo sData[0] fra 2 e 15
		 */
		if (sData[0] < 2 || sData[0] >= sizeof(sData)) {
			MAGN_LOG("invalid argument.");
			return -EINVAL;
		}
		ret = I2C_TxData(&sData[1], sData[0]);
		if (ret < 0)
			return ret;
		return 0;

	case QMCX983_IOC_READ:
		if (argp == NULL) {
			MAGN_LOG("invalid argument.");
			return -EINVAL;
		}
		if (copy_from_user(sData, argp, sizeof(sData))) {
			MAGN_LOG("copy_from_user failed.");
			return -EFAULT;
		}
		/* <<sub w8,w1,#0x1 ; cmp w8,#0xf ; b.cc>>@0xffffff800878fc74:
		 * ammesso solo sData[0] fra 1 e 15
		 */
		if (sData[0] < 1 || sData[0] >= sizeof(sData)) {
			MAGN_LOG("invalid argument.");
			return -EINVAL;
		}
		ret = I2C_RxData(&sData[1], sData[0]);
		if (ret < 0)
			return ret;
		if (copy_to_user(argp, sData, sData[0] + 1)) {
			/* "\x016[QMC-Msensor] copy_to_user failed."
			 * @0xffffff80091c1bf7
 * il frammento che sta nel codice: "copy_to_user failed."@0xffffff80091c1c07
			 */
			MAGN_LOG("copy_to_user failed.");
			return -EFAULT;
		}
		return 0;

	default:
		/* "\x013[QMC-Msensor] %s %d : %s not supported = 0x%04x"
		 * @0xffffff80091c1c1c, riga 1070
 * il frammento che sta nel codice: "%s not supported = 0x%04x"@0xffffff80091c1c34
		 * (<<mov w2,#0x42e>>@0xffffff800878fca4);
		 * <<mov x0,#0xfffffffffffffdfd>>@0xffffff800878fcb4 = -ENOIOCTLCMD
		 */
		/* Ogni ramo esce con un return proprio, non attraverso un
		 * accumulatore: di fabbrica ciascuna uscita scrive direttamente
		 * in x0 (<<mov x0,#0xfffffffffffffdfd>>@0xffffff800878fcb4) e
		 * salta all'epilogo comune.
		 */
#line 1070
		MAGN_ERR("%s not supported = 0x%04x", __func__, cmd);
		return -ENOIOCTLCMD;
	}
	return ret;
}

/*
 * qmcX983_open: 0xffffff800878fe48, 76 byte, t.
 */
static int qmcX983_open(struct inode *inode, struct file *file)
{
	struct qmcX983_i2c_data *obj = i2c_get_clientdata(this_client);

	/* <<tbz w8,#3>>@0xffffff800878fe6c: bit 3 di trace */
	if (atomic_read(&obj->trace) & 0x08) {
		/* "\x016[QMC-Msensor] Open device node:qmcX983\n"
		 * @0xffffff80091c1c65
 * il frammento che sta nel codice: "Open device node:qmcX983\n"@0xffffff80091c1c75
		 */
		MAGN_LOG("Open device node:qmcX983\n");
	}
	return nonseekable_open(inode, file);
}

/*
 * qmcX983_release: 0xffffff800878fe94, 80 byte, t.
 */
static int qmcX983_release(struct inode *inode, struct file *file)
{
	struct qmcX983_i2c_data *obj = i2c_get_clientdata(this_client);

	/* ldxr/"sub w10,w10,#0x1"/stxr su 0xffffff8009cba270
	 * @0xffffff800878feac: e' atomic_dec, non una scrittura semplice
	 */
	atomic_dec(&open_count);
	if (atomic_read(&obj->trace) & 0x08) {
		/* "\x016[QMC-Msensor] Release device node:qmcX983\n"
		 * @0xffffff80091c1c8f
 * il frammento che sta nel codice: "Release device node:qmcX983\n"@0xffffff80091c1c9f
		 */
		MAGN_LOG("Release device node:qmcX983\n");
	}
	return 0;
}

/*
 * file_operations a 0xffffff800992f3e8: solo tre campi relocati, agli offset
 * 0x48 (unlocked_ioctl), 0x60 (open) e 0x70 (release). Nessun compat_ioctl a
 * 0x50, benche' il config di fabbrica abbia CONFIG_COMPAT=y.
 *
 * Non e' const: di fabbrica questa tabella sta in .data, incastrata fra
 * l'ultimo driver_attr_* (che finisce a 0x992f3e8) e mag_factory_fops
 * (0x992f4d8) -- non in .rodata, dove la nostra `const` la metteva
 * (revisione indipendente, revisione-qmcx983-completo.md, R3).
 */
static struct file_operations qmcX983_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = qmcX983_unlocked_ioctl,
	.open = qmcX983_open,
	.release = qmcX983_release,
};

/*
 * miscdevice a 0xffffff800992f240: minor 0xff = MISC_DYNAMIC_MINOR, name
 * relocato a "qst_msensor"@0xffffff80091c1b6f, fops a 0xffffff800992f3e8.
 */
static struct miscdevice qmcX983_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "qst_msensor",
	.fops = &qmcX983_fops,
};

/*
 * qmcX983_m_open_report_data: 0xffffff800878ee88, 8 byte, t.
 *   mov w0, wzr ; ret
 */
static int qmcX983_m_open_report_data(int open)
{
	return 0;
}

/*
 * qmcX983_m_set_delay: 0xffffff800878ee24, 100 byte, t.
 */
static int qmcX983_m_set_delay(u64 ns)
{
	struct i2c_client *client = this_client;

	if (client == NULL) {
		/* "\x013[QMC-Msensor] %s %d : this_client is null!\n"
		 * @0xffffff80091c1d3f, riga 1198
 * il frammento che sta nel codice: "this_client is null!\n"@0xffffff80091c1d57
		 * (<<mov w2,#0x4ae>>@0xffffff800878ee5c)
		 */
#line 1198
		MAGN_ERR("this_client is null!\n");
		return -EINVAL;
	}
	if (i2c_get_clientdata(client) == NULL) {
		/* "\x013[QMC-Msensor] %s %d : data is null!\n"
		 * @0xffffff80091c1d7e, riga 1204
 * il frammento che sta nel codice: "data is null!\n"@0xffffff80091c1d96
		 * (<<mov w2,#0x4b4>>@0xffffff800878ee74)
		 */
#line 1204
		MAGN_ERR("data is null!\n");
		return -EINVAL;
	}
	return 0;
}

/*
 * qmcX983_m_enable: 0xffffff800878ece8, 316 byte, t.
 */
static int qmcX983_m_enable(int en)
{
	int value = en;
	unsigned char databuf[2];
	struct i2c_client *client = this_client;
	struct qmcX983_i2c_data *data;

	if (client == NULL) {
		/* riga 1221 (<<mov w2,#0x4c5>>@0xffffff800878edf8) */
#line 1221
		MAGN_ERR("this_client is null!\n");
		return -EINVAL;
	}
	data = i2c_get_clientdata(client);
	if (data == NULL) {
		/* riga 1228 (<<mov w2,#0x4cc>>@0xffffff800878ee10) */
#line 1228
		MAGN_ERR("data is null!\n");
		return -EINVAL;
	}

	/* <<cmp w0,#0x1 ; b.ne>>@0xffffff800878ed18 */
	if (value == 1) {
		/* <<orr w8,w8,#0x1>>@0xffffff800878ed28 */
		v_open_flag |= 0x01;
		qmcX983_enable();
	} else {
		/* <<mov w8,#0x1c09>>@0xffffff800878ed4c -> {0x09, 0x1c} */
		databuf[0] = QMCX983_REG_CTRL1;
		databuf[1] = 0x1c;
		mag_i2c_write_block(client, databuf[0], &databuf[1], 1);
		/* <<and w3,w9,#0x3e>>@0xffffff800878ed94 */
		v_open_flag &= 0x3e;
	}
	atomic_set(&open_flag, v_open_flag);
	/* "\x013[QMC-Msensor] %s %d : qmcX983 v_open_flag = 0x%x,open_flag= 0x%x\n"
	 * @0xffffff80091c1da5, riga 1246 (<<mov w2,#0x4de>>@0xffffff800878edb8)
 * il frammento che sta nel codice: "qmcX983 v_open_flag = 0x%x,open_flag= 0x%x\n"@0xffffff80091c1dbd
	 */
	MAGN_ERR("qmcX983 v_open_flag = 0x%x,open_flag= 0x%x\n",
#line 1246
		 v_open_flag, atomic_read(&open_flag));
	return 0;
}

/*
 * qmcX983_batch: 0xffffff800878ee90, 8 byte, t.
 *   mov w0, wzr ; ret
 */
static int qmcX983_batch(int flag, int64_t samplingPeriodNs,
			 int64_t maxBatchReportLatencyNs)
{
	return 0;
}

/*
 * qmcX983_flush: 0xffffff800878ee98, 20 byte, t.
 *   bl mag_flush_report ; ret -- il valore restituito e' quello dell'helper
 */
static int qmcX983_flush(void)
{
	return mag_flush_report();
}

/*
 * qmcX983_m_get_data: 0xffffff800878eeac, 224 byte, t.
 */
static int qmcX983_m_get_data(int *x, int *y, int *z, int *status)
{
	int mag[3];
	struct timespec time;
	struct i2c_client *client = this_client;

	if (client == NULL) {
		/* riga 1268 (<<mov w2,#0x4f4>>@0xffffff800878ef60) */
		MAGN_ERR("this_client is null!\n");
		return -EINVAL;
	}
	if (i2c_get_clientdata(client) == NULL) {
		/* riga 1274 (<<mov w2,#0x4fa>>@0xffffff800878ef78) */
		MAGN_ERR("data is null!\n");
		return -EINVAL;
	}

	qmcX983_read_mag_xyz(mag);
	*x = mag[0];
	*y = mag[1];
	*z = mag[2];
	/* <<orr w9,wzr,#0x3>>@0xffffff800878eefc */
	*status = 3;
	/* "orr w0,wzr,#0x1 ; bl ktime_get_with_offset ; bl ns_to_timespec"
	 * @0xffffff800878ef00: l'offset 1 e' TK_OFFS_BOOT, e il risultato non
	 * viene usato da nessuna parte -- e' cosi' di fabbrica.
	 */
	time = ktime_to_timespec(ktime_get_boottime());
	return 0;
}

/*
 * qmcX983_factory_enable_sensor: 0xffffff800878fee4, 260 byte, t.
 * Stesso corpo di qmcX983_m_enable ma con i due messaggi di errore a
 * KERN_INFO e senza il printk finale.
 */
static int qmcX983_factory_enable_sensor(bool enable_disable,
					 int64_t sample_periods_ms)
{
	unsigned char databuf[2];
	struct i2c_client *client = this_client;
	struct qmcX983_i2c_data *data;

	if (client == NULL) {
		/* <<\x016[QMC-Msensor] this_client is null!\n>>@0xffffff80091c1cbc
		 * il frammento e' lo stesso testo della variante KERN_ERR, gia'
		 * citata a 0xffffff80091c1d57: qui sta a 0xffffff80091c1ccc
		 */
		MAGN_LOG("this_client is null!\n");
		return -EINVAL;
	}
	data = i2c_get_clientdata(client);
	if (data == NULL) {
		/* <<\x016[QMC-Msensor] data is null!\n>>@0xffffff80091c1ce2
		 * il frammento e' lo stesso testo della variante KERN_ERR, gia'
		 * citata a 0xffffff80091c1d96: qui sta a 0xffffff80091c1cf2
		 */
		MAGN_LOG("data is null!\n");
		return -EINVAL;
	}

	/* <<tbz w0,#0>>@0xffffff800878ff14 */
	if (enable_disable) {
		v_open_flag |= 0x01;
		qmcX983_enable();
	} else {
		/* <<mov w8,#0x1c09>>@0xffffff800878ff44 -> {0x09, 0x1c} */
		databuf[0] = QMCX983_REG_CTRL1;
		databuf[1] = 0x1c;
		mag_i2c_write_block(client, databuf[0], &databuf[1], 1);
		v_open_flag &= 0x3e;
	}
	atomic_set(&open_flag, v_open_flag);
	return 0;
}

/*
 * qmcX983_factory_get_data: 0xffffff800878ffe8, 284 byte, t.
 * I due printk d'errore portano __func__ = "qmcX983_m_get_data" e le righe
 * 1268/1274: qmcX983_m_get_data e' incorporata qui dentro, e le tre variabili
 * locali che le si passano devono essere azzerate all'inizio, perche' sul
 * ramo d'errore il binario scrive comunque zero nei tre elementi
 * (<<mov w8,wzr ; mov w9,wzr ; mov w10,wzr>>@0xffffff80087900ec).
 */
static int qmcX983_factory_get_data(int32_t data[3], int *status)
{
	int x = 0, y = 0, z = 0;
	int err;

	err = qmcX983_m_get_data(&x, &y, &z, status);
	/* moltiplicazione per 0x66666667 e "asr x9,x9,#34"
	 * @0xffffff8008790064: divisione intera per 10
	 */
	data[0] = x / 10;
	data[1] = y / 10;
	data[2] = z / 10;
	return err;
}

/*
 * qmcX983_factory_get_raw_data: 0xffffff8008790104, 32 byte, t.
 */
static int qmcX983_factory_get_raw_data(int32_t data[3])
{
	/* "\x016[QMC-Msensor] do not support qmcX983_factory_get_raw_data!\n"
	 * @0xffffff80091c1d01
 * il frammento che sta nel codice: "do not support qmcX983_factory_get_raw_data!\n"@0xffffff80091c1d11
	 */
	MAGN_LOG("do not support qmcX983_factory_get_raw_data!\n");
	return 0;
}

/*
 * 0xffffff8008790124, 0x879012c, 0x8790134, 0x879013c, 0x8790144: 8 byte
 * ciascuna, "mov w0,wzr ; ret".
 */
static int qmcX983_factory_enable_calibration(void)
{
	return 0;
}

static int qmcX983_factory_clear_cali(void)
{
	return 0;
}

static int qmcX983_factory_set_cali(int32_t offset[3])
{
	return 0;
}

static int qmcX983_factory_get_cali(int32_t offset[3])
{
	return 0;
}

static int qmcX983_factory_do_self_test(void)
{
	return 0;
}

/* mag_factory_fops a 0xffffff800992f4d8: otto puntatori relocati, uno per
 * campo, nell'ordine di struct mag_factory_fops
 */
static struct mag_factory_fops qmcX983_factory_fops = {
	.enable_sensor = qmcX983_factory_enable_sensor,
	.get_data = qmcX983_factory_get_data,
	.get_raw_data = qmcX983_factory_get_raw_data,
	.enable_calibration = qmcX983_factory_enable_calibration,
	.clear_cali = qmcX983_factory_clear_cali,
	.set_cali = qmcX983_factory_set_cali,
	.get_cali = qmcX983_factory_get_cali,
	.do_self_test = qmcX983_factory_do_self_test,
};

/* mag_factory_public a 0xffffff800992f290: gain = 1 e sensitivity = 1 (la
 * parola a quell'indirizzo e' 0x0000000100000001), fops relocata a
 * 0xffffff800992f4d8
 */
static struct mag_factory_public qmcX983_factory_device = {
	.gain = 1,
	.sensitivity = 1,
	.fops = &qmcX983_factory_fops,
};

/*
 * qmcX983_device_check: nessun simbolo proprio, sempre incorporata nella
 * probe; nome e righe restano nei printk
 * (<<qmcX983_device_check>>@0xffffff80091c167d, righe 1472/1488/1495/1509/1515).
 */
static int qmcX983_device_check(void)
{
	int res = 0;
	unsigned char databuf[2] = {0};
	unsigned char value = 0;

	databuf[0] = QMCX983_REG_CHIPID;
	res = I2C_RxData(databuf, 1);
	if (res < 0) {
		/* "\x013[QMC-Msensor] %s %d : %s: I2C_RxData failed\n"
		 * @0xffffff80091c164e, riga 1472
 * il frammento che sta nel codice: "%s: I2C_RxData failed\n"@0xffffff80091c1666
		 * (<<mov w2,#0x5c0>>@0xffffff800878e26c)
		 */
#line 1472
		MAGN_ERR("%s: I2C_RxData failed\n", __func__);
		return res;
	}
	value = databuf[0];

	/* <<cmp w8,#0x31>>@0xffffff800878e230, <<cmp w8,#0x32>>@0xffffff800878e238,
	 * <<cmp w8,#0xff>>@0xffffff800878e240
	 */
	if (value == 0x31) {
		chip_id = QMC6983_E1;
	} else if (value == 0x32) {
		/* <<mov w8,#0x12e>>@0xffffff800878e29c -> {0x2e, 0x01} */
		databuf[0] = QMCX983_REG_OTP_ADDR;
		databuf[1] = 0x01;
		res = I2C_TxData(databuf, 2);
		if (res < 0) {
			/* "\x013[QMC-Msensor] %s %d : %s: I2C_TxData failed\n"
			 * @0xffffff80091c1692, riga 1488
 * il frammento che sta nel codice: "%s: I2C_TxData failed\n"@0xffffff80091c16aa
			 * (<<mov w2,#0x5d0>>@0xffffff800878e94c)
			 */
#line 1488
			MAGN_ERR("%s: I2C_TxData failed\n", __func__);
			return res;
		}
		databuf[0] = QMCX983_REG_OTP_DATA;
		res = I2C_RxData(databuf, 1);
		if (res < 0) {
			/* riga 1495 (<<mov w2,#0x5d7>>@0xffffff800878e9ec) */
			MAGN_ERR("%s: I2C_RxData failed\n", __func__);
			return res;
		}
		/* <<tbnz w8,#2>>@0xffffff800878e300 */
		if (databuf[0] & 0x04) {
			chip_id = QMC6983_E1_METAL;
		} else {
			/* <<mov w8,#0xf2e>>@0xffffff800878e30c -> {0x2e, 0x0f} */
			databuf[0] = QMCX983_REG_OTP_ADDR;
			databuf[1] = 0x0f;
			res = I2C_TxData(databuf, 2);
			if (res < 0) {
				/* riga 1509
				 * (<<mov w2,#0x5e5>>@0xffffff800878ea90)
				 */
#line 1509
				MAGN_ERR("%s: I2C_TxData failed\n", __func__);
				return res;
			}
			databuf[0] = QMCX983_REG_OTP_DATA;
			res = I2C_RxData(databuf, 1);
			if (res < 0) {
				/* riga 1515
				 * (<<mov w2,#0x5eb>>@0xffffff800878eadc)
				 */
#line 1515
				MAGN_ERR("%s: I2C_RxData failed\n", __func__);
				return res;
			}
			/* <<and w8,w8,#0x3c ; cmp w8,#0x8>>@0xffffff800878e370
			 * e <<cmp w8,#0xc>>@0xffffff800878e37c
			 */
			if ((databuf[0] & 0x3c) == 0x08)
				chip_id = QMC7983_VERTICAL;
			else if ((databuf[0] & 0x3c) == 0x0c)
				chip_id = QMC7983_SLOPE;
		}
	} else if (value == 0xff) {
		chip_id = QMC6983_A1_D1;
	}
	return 0;
}

/*
 * qmcx983_get_OTP: nessun simbolo proprio, incorporata nella probe; nome e
 * righe dai printk (<<qmcx983_get_OTP>>@0xffffff80091c16c1 -- la 'x'
 * minuscola e' di fabbrica -- righe 1549/1557/1576/1584/1594/1602).
 *
 * Le mdelay: dieci __const_udelay(0x418958) consecutivi, e 0x418958 =
 * 1000 * 0x10c7 cioe' udelay(1000) secondo include/asm-generic/delay.h;
 * dieci di fila sono il ciclo "while (__ms--) udelay(1000)" di
 * include/linux/delay.h per mdelay(10), srotolato
 * (<<mov w0,#0x8958 ; movk w0,#0x41,lsl#16>>@0xffffff800878e3e4).
 */
static int qmcx983_get_OTP(void)
{
	int res = 0;
	unsigned char databuf[2] = {0};
	unsigned char value1 = 0, value2 = 0;

	/* <<mov w23,#0xa2e>>@0xffffff800878e3a8 -> {0x2e, 0x0a} */
	databuf[0] = QMCX983_REG_OTP_ADDR;
	databuf[1] = 0x0a;
	res = I2C_TxData(databuf, 2);
	if (res < 0) {
		/* riga 1549 (<<mov w2,#0x60d>>@0xffffff800878e8cc) */
#line 1549
		MAGN_ERR("%s: I2C_TxData failed\n", __func__);
		return res;
	}
	mdelay(10);
	databuf[0] = QMCX983_REG_OTP_DATA;
	res = I2C_RxData(databuf, 1);
	if (res < 0) {
		/* riga 1557 (<<mov w2,#0x615>>@0xffffff800878e90c) */
		MAGN_ERR("%s: I2C_RxData failed\n", __func__);
		return res;
	}
	value1 = databuf[0];
	/* "and w11,w8,#0x1f ; orr w12,w8,#0xffffffe0 ; tst w8,#0x20 ; csel"
	 * @0xffffff800878e484: campo a 6 bit con segno
	 */
	if (value1 & 0x20)
		otp_a = value1 | 0xffffffe0;
	else
		otp_a = value1 & 0x1f;

	/* <<mov w10,#0xd>>@0xffffff800878e494 -> {0x2e, 0x0d} */
	databuf[0] = QMCX983_REG_OTP_ADDR;
	databuf[1] = 0x0d;
	res = I2C_TxData(databuf, 2);
	if (res < 0) {
		/* riga 1576 (<<mov w2,#0x628>>@0xffffff800878ea24) */
#line 1576
		MAGN_ERR("%s: I2C_TxData failed\n", __func__);
		return res;
	}
	mdelay(10);
	databuf[0] = QMCX983_REG_OTP_DATA;
	res = I2C_RxData(databuf, 1);
	if (res < 0) {
		/* riga 1584 (<<mov w2,#0x630>>@0xffffff800878ea40) */
		MAGN_ERR("%s: I2C_RxData failed\n", __func__);
		return res;
	}
	value1 = databuf[0];

	mdelay(10);
	/* <<mov w8,#0xf2e>>@0xffffff800878e5f8 -> {0x2e, 0x0f} */
	databuf[0] = QMCX983_REG_OTP_ADDR;
	databuf[1] = 0x0f;
	res = I2C_TxData(databuf, 2);
	if (res < 0) {
		/* riga 1594 (<<mov w2,#0x63a>>@0xffffff800878eac0) */
#line 1594
		MAGN_ERR("%s: I2C_TxData failed\n", __func__);
		return res;
	}
	mdelay(10);
	databuf[0] = QMCX983_REG_OTP_DATA;
	res = I2C_RxData(databuf, 1);
	if (res < 0) {
		/* riga 1602 (<<mov w2,#0x642>>@0xffffff800878eaf8) */
		MAGN_ERR("%s: I2C_RxData failed\n", __func__);
		return res;
	}
	value2 = databuf[0];

	/* "lsr w9,w23,#2 ; and w9,w9,#0x1c ; orr w8,w9,w8,lsr #6 ;
	 *  tst w23,#0x80 ; orr w9,w8,#0xffffffe0 ; csel"@0xffffff800878e6d4
	 */
	otp_b = ((value1 >> 2) & 0x1c) | (value2 >> 6);
	if (value1 & 0x80)
		otp_b |= 0xffffffe0;
	return 0;
}

/*
 * qmcX983_suspend: 0xffffff8008790510, 8 byte, t.
 * qmcX983_resume:  0xffffff8008790518, 8 byte, t.
 * Entrambe "mov w0,wzr ; ret". Il dev_pm_ops a 0xffffff8008f54708 riempie
 * con la stessa coppia suspend/resume/freeze/thaw/poweroff/restore (offset
 * 0x10, 0x18, 0x20, 0x28, 0x30, 0x38): e' esattamente SIMPLE_DEV_PM_OPS.
 */
static int qmcX983_suspend(struct device *dev)
{
	return 0;
}

static int qmcX983_resume(struct device *dev)
{
	return 0;
}

static SIMPLE_DEV_PM_OPS(qmcX983_pm_ops, qmcX983_suspend, qmcX983_resume);

/*
 * qmcX983_i2c_detect: 0xffffff800878ecc0, 40 byte, t.
 * <<mov w2,#0x14>>@0xffffff800878ecd4 = I2C_NAME_SIZE = sizeof(info->type).
 */
static int qmcX983_i2c_detect(struct i2c_client *client,
			      struct i2c_board_info *info)
{
	strlcpy(info->type, QMCX983_DEV_NAME, sizeof(info->type));
	return 0;
}

/* i2c_device_id a 0xffffff8008f547c0: una sola voce, "qmcX983", poi il
 * terminatore azzerato a 0xffffff8008f547e0
 */
static const struct i2c_device_id qmcX983_i2c_id[] = {
	{QMCX983_DEV_NAME, 0},
	{}
};

/* of_device_id a 0xffffff8008f54578: il campo compatible (offset 0x40 nella
 * struttura, name[32] + type[32]) contiene "mediatek,msensor"@0xffffff8008f545b8
 */
static const struct of_device_id mag_of_match[] = {
	{.compatible = "mediatek,msensor"},
	{},
};

static int qmcX983_i2c_probe(struct i2c_client *client,
			     const struct i2c_device_id *id);
static int qmcX983_i2c_remove(struct i2c_client *client);

/*
 * i2c_driver a 0xffffff800992f158: probe (+0x10), remove (+0x18), detect
 * (+0xc0) relocati; driver.name -> "qmcX983", driver.of_match_table ->
 * 0xffffff8008f54578, driver.pm -> 0xffffff8008f54708, id_table ->
 * 0xffffff8008f547c0. Tutti gli altri campi sono zero.
 */
static struct i2c_driver qmcX983_i2c_driver = {
	.probe = qmcX983_i2c_probe,
	.remove = qmcX983_i2c_remove,
	.detect = qmcX983_i2c_detect,
	.id_table = qmcX983_i2c_id,
	.driver = {
		.name = QMCX983_DEV_NAME,
		.of_match_table = mag_of_match,
		.pm = &qmcX983_pm_ops,
	},
};

/*
 * mag_init_info a 0xffffff800992f138: name/init/uninit relocati a
 * "qmcX983", qmcX983_local_init, qmcX983_local_remove; platform_diver_addr
 * (0xffffff800992f150) resta zero nell'immagine e viene riempito a runtime
 * da mag_driver_add.
 */
static struct mag_init_info qmcX983_init_info = {
	.name = QMCX983_DEV_NAME,
	.init = qmcX983_local_init,
	.uninit = qmcX983_local_remove,
};

/*
 * qmcX983_i2c_probe: 0xffffff800878e0b0, 2960 byte, t.
 */
static int qmcX983_i2c_probe(struct i2c_client *client,
			     const struct i2c_device_id *id)
{
	struct qmcX983_i2c_data *data;
	struct mag_control_path ctl = {0};
	struct mag_data_path mag_data = {0};
	int err = 0;

	MAGN_LOG("%s\n", __func__);

	/* <<mov w1,#0xc0 ; movk w1,#0x140,lsl#16>>@0xffffff800878e11c = 0x014000c0
	 * e <<orr w2,wzr,#0x40>>@0xffffff800878e124 = 64: GFP_KERNEL senza
	 * __GFP_ZERO (0x8000), cioe' kmalloc e non kzalloc.
	 */
	data = kmalloc(sizeof(struct qmcX983_i2c_data), GFP_KERNEL);
	if (!data) {
		/* <<mov w20,#0xfffffff4>>@0xffffff800878e180 = -ENOMEM.
		 * Questo ramo salta il kfree ("b 0xffffff800878e978", che e'
		 * dopo la chiamata a kfree).
		 */
		err = -ENOMEM;
		goto exit;
	}

	err = get_mag_dts_func(client->dev.of_node, &data->hw);
	if (err < 0) {
		/* "\x013[QMC-Msensor] %s %d : %s. get dts info fail\n"
		 * @0xffffff80091c1436, riga 1637
 * il frammento che sta nel codice: "%s. get dts info fail\n"@0xffffff80091c144e
		 * (<<mov w2,#0x665>>@0xffffff800878e198)
		 */
#line 1637
		MAGN_ERR("%s. get dts info fail\n", __func__);
		/* <<mov w20,#0xfffffff2>>@0xffffff800878e1a4 = -EFAULT */
		err = -EFAULT;
		goto exit_kfree;
	}

	client->addr = QMCX983_I2C_ADDR;

	err = hwmsen_get_convert(data->hw.direction, &data->cvt);
	if (err) {
		/* "\x013[QMC-Msensor] %s %d : QMCX983 invalid direction: %d\n"
		 * @0xffffff80091c1465, riga 1647
 * il frammento che sta nel codice: "QMCX983 invalid direction: %d\n"@0xffffff80091c147d
		 * (<<mov w2,#0x66f>>@0xffffff800878e174)
		 */
#line 1647
		MAGN_ERR("QMCX983 invalid direction: %d\n", data->hw.direction);
		goto exit_kfree;
	}
	/* <<\x016[QMC-Msensor] %s: direction: %d\n>>@0xffffff80091c149c
	 * il frammento che sta nel codice: "%s: direction: %d\n"@0xffffff80091c14ac
	 */
	MAGN_LOG("%s: direction: %d\n", __func__, data->hw.direction);

	atomic_set(&data->layout, data->hw.direction);
	atomic_set(&data->trace, 0);
	mutex_init(&sensor_data_mutex);
	mutex_init(&read_i2c_xyz);

	data->client = client;
	i2c_set_clientdata(client, data);
	this_client = client;

	err = qmcX983_device_check();
	if (err < 0) {
		/* <<\x016[QMC-Msensor] %s check ID faild!\n>>@0xffffff80091c14e0
 * il frammento che sta nel codice: "%s check ID faild!\n"@0xffffff80091c14f0
		 * -- il refuso "faild" e' di fabbrica
		 */
		MAGN_LOG("%s check ID faild!\n", __func__);
		goto exit_kfree;
	}

	if (chip_id != QMC6983_A1_D1) {
		err = qmcx983_get_OTP();
		if (err < 0) {
			/* "\x016[QMC-Msensor] %s get OTP faild!\n"
			 * @0xffffff80091c1504
 * il frammento che sta nel codice: "%s get OTP faild!\n"@0xffffff80091c1514
			 */
			MAGN_LOG("%s get OTP faild!\n", __func__);
			goto exit_kfree;
		}
	} else {
		/* <<str wzr,[x9,#668]>>@0xffffff800878e6f8 e
		 * <<str w8,[x10,#672]>>@0xffffff800878e708 con w8 = 0
		 */
		otp_a = 0;
		otp_b = 0;
	}

	/* <<ldr x9,[x23,#336] ; adds x22,x9,#0x28>>@0xffffff800878e700:
	 * l'argomento e' platform_diver_addr + 0x28, cioe' l'offset di
	 * struct device_driver dentro struct platform_driver, e il confronto
	 * con zero e' il controllo "driver == NULL" fatto sulla somma.
	 */
	err = qmcX983_create_attr(&qmcX983_init_info.platform_diver_addr->driver);
	if (err) {
		/* "\x013[QMC-Msensor] %s %d : create attribute err = %d\n"
		 * @0xffffff80091c1527, riga 1683
 * il frammento che sta nel codice: "create attribute err = %d\n"@0xffffff80091c153f
		 * (<<mov w2,#0x693>>@0xffffff800878e894)
		 */
#line 1683
		MAGN_ERR("create attribute err = %d\n", err);
		goto exit_kfree;
	}

	err = misc_register(&qmcX983_device);
	if (err) {
		/* "\x013[QMC-Msensor] %s %d : qmcX983_device register failed\n"
		 * @0xffffff80091c155a, riga 1691
 * il frammento che sta nel codice: "qmcX983_device register failed\n"@0xffffff80091c1572
		 * (<<mov w2,#0x69b>>@0xffffff800878eb1c)
		 */
#line 1691
		MAGN_ERR("qmcX983_device register failed\n");
		goto exit_misc_register_failed;
	}

	err = mag_factory_device_register(&qmcX983_factory_device);
	if (err) {
		/* "\x013[QMC-Msensor] %s %d : misc device register failed, err = %d\n"
		 * @0xffffff80091c1592, riga 1697
 * il frammento che sta nel codice: "misc device register failed, err = %d\n"@0xffffff80091c15aa
		 * (<<mov w2,#0x6a1>>@0xffffff800878e838)
		 */
#line 1697
		MAGN_ERR("misc device register failed, err = %d\n", err);
		goto exit_misc_device_register_failed;
	}

	/* <<strb wzr,[sp,#96]>>@0xffffff800878eb88 = ctl+48 */
	ctl.is_report_input_direct = false;
	/* is_use_common_factory (ctl+50) NON viene assegnata: resta a zero
	 * dall'azzeramento iniziale della struttura. Un `ctl.is_use_common_factory
	 * = false;` qui produrrebbe uno `strb wzr` che di fabbrica non esiste in
	 * nessun punto della funzione (revisione indipendente,
	 * docs/bringup/rapporti/revisione-qmcx983-completo.md, R6): la riga era
	 * stata scritta e il commento sopra la contraddiceva gia'.
	 */
	ctl.open_report_data = qmcX983_m_open_report_data;
	ctl.enable = qmcX983_m_enable;
	ctl.set_delay = qmcX983_m_set_delay;
	ctl.batch = qmcX983_batch;
	ctl.flush = qmcX983_flush;
	/* <<ldrb w11,[x19,#36] ; strb w11,[sp,#97]>>@0xffffff800878eb5c:
	 * data->hw.is_batch_supported (hw+28) in ctl+49
	 */
	ctl.is_support_batch = data->hw.is_batch_supported;
	/* copia di 8 byte da 0xffffff8008f54800 ("qmcX983") e azzeramento di
	 * tutti gli altri 56 byte del campo
	 * ("stur x8,[sp,#100]" ... <<stur xzr,[sp,#140]>>@0xffffff800878eb9c):
	 * e' la semantica di strncpy, non di strcpy
	 */
	strncpy(ctl.libinfo.libname, QMCX983_DEV_NAME,
		sizeof(ctl.libinfo.libname));
	/* <<stp w8,w9,[sp,#164]>>@0xffffff800878eb68 = ctl+116 e ctl+120 */
	ctl.libinfo.layout = data->hw.direction;
	ctl.libinfo.deviceid = chip_id;

	err = mag_register_control_path(&ctl);
	if (err) {
		/* "register mag control path err\n"@0xffffff80091c15d1: senza
		 * prefisso KERN_*, quindi printk nudo e non pr_err
		 */
		printk("register mag control path err\n");
		goto exit_register_path_failed;
	}

	/* "str w21,[sp,#16]" con w21 = 1 e "str x8,[sp,#24]"
	 * @0xffffff800878ebd4
	 */
	mag_data.div = 1;
	mag_data.get_data = qmcX983_m_get_data;
	err = mag_register_data_path(&mag_data);
	if (err) {
		/* "register data control path err\n"@0xffffff80091c15f0 */
		printk("register data control path err\n");
		goto exit_register_path_failed;
	}

	/* "strb w21,[x8,#628]" con w21 = 1 @0xffffff800878ec30 */
	qmcX983_init_flag = true;
	/* <<\x016[QMC-Msensor] %s: OK\n>>@0xffffff80091c1610
	 * il frammento che sta nel codice: "%s: OK\n"@0xffffff80091c1620
	 */
	MAGN_LOG("%s: OK\n", __func__);
	return 0;

exit_register_path_failed:
	mag_factory_device_deregister(&qmcX983_factory_device);
exit_misc_device_register_failed:
	misc_deregister(&qmcX983_device);
exit_misc_register_failed:
	qmcX983_delete_attr(&qmcX983_init_info.platform_diver_addr->driver);
exit_kfree:
	kfree(data);
exit:
	/* <<\x013[QMC-Msensor] %s %d : %s: err = %d\n>>@0xffffff80091c1628,
 * il frammento che sta nel codice: "%s: err = %d\n"@0xffffff80091c1640
	 * riga 1746 (<<mov w2,#0x6d2>>@0xffffff800878e988)
	 */
#line 1746
	MAGN_ERR("%s: err = %d\n", __func__, err);
	return err;
}

/*
 * qmcX983_i2c_remove: 0xffffff800878ec40, 128 byte, t.
 */
static int qmcX983_i2c_remove(struct i2c_client *client)
{
	int err = 0;

	err = qmcX983_delete_attr(&qmcX983_init_info.platform_diver_addr->driver);
	if (err) {
		/* "\x013[QMC-Msensor] %s %d : qmcX983_delete_attr fail: %d\n"
		 * @0xffffff80091c1e10, riga 1761
 * il frammento che sta nel codice: "qmcX983_delete_attr fail: %d\n"@0xffffff80091c1e28
		 * (<<mov w2,#0x6e1>>@0xffffff800878ec78)
		 */
#line 1761
		MAGN_ERR("qmcX983_delete_attr fail: %d\n", err);
	}

	/* <<str xzr,[x8,#584]>>@0xffffff800878ec88 */
	this_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	misc_deregister(&qmcX983_device);
	mag_factory_device_deregister(&qmcX983_factory_device);
	return 0;
}

/*
 * qmcX983_local_init: 0xffffff800878e008, 128 byte, t.
 * Entrambi i rami d'errore restituiscono -EINVAL
 * (<<mov w0,#0xffffffea>>@0xffffff800878e07c), non -1.
 */
static int qmcX983_local_init(void)
{
	/* <<str wzr,[x8,#624]>>@0xffffff800878e020, prima di i2c_add_driver */
	atomic_set(&open_count, 0);

	if (i2c_add_driver(&qmcX983_i2c_driver)) {
		/* "\x013[QMC-Msensor] %s %d : add driver error\n"
		 * @0xffffff80091c13af, riga 1782
 * il frammento che sta nel codice: "add driver error\n"@0xffffff80091c13c7
		 * (<<mov w2,#0x6f6>>@0xffffff800878e03c)
		 */
#line 1782
		MAGN_ERR("add driver error\n");
		return -EINVAL;
	}
	if (!qmcX983_init_flag) {
		/* <<\x013[QMC-Msensor] %s %d : %s failed!\n>>@0xffffff80091c13ec,
 * il frammento che sta nel codice: "%s failed!\n"@0xffffff80091c1404
		 * riga 1788 (<<mov w2,#0x6fc>>@0xffffff800878e070); il terzo
		 * argomento e' di nuovo __func__ (<<mov x3,x1>>@0xffffff800878e074)
		 */
#line 1788
		MAGN_ERR("%s failed!\n", __func__);
		return -EINVAL;
	}
	return 0;
}

/*
 * qmcX983_local_remove: 0xffffff800878e088, 40 byte, t.
 */
static int qmcX983_local_remove(void)
{
	atomic_set(&open_count, 0);
	i2c_del_driver(&qmcX983_i2c_driver);
	return 0;
}

/*
 * qmcX983_init: 0xffffff8009374654, 32 byte, t, nella .init.text.
 *   bl mag_driver_add(&qmcX983_init_info) ; mov w0, wzr ; ret
 * Il valore restituito da mag_driver_add viene scartato.
 */
static int __init qmcX983_init(void)
{
	mag_driver_add(&qmcX983_init_info);
	return 0;
}

module_init(qmcX983_init);

/*
 * ===========================================================================
 * qmcX983_exit -- 0xffffff80093aa5ec, 4 byte, `.exit.text`, corpo VUOTO
 * ===========================================================================
 *
 * **CORREZIONE del 2026-08-21 (secondo passaggio sulla `.exit.text`).** La
 * stesura precedente di questo commento concludeva «NESSUNA `__exit` PER
 * QUESTO DRIVER -- risultato negativo». La conclusione era SBAGLIATA, e lo
 * era per un difetto nello strumento che l'aveva prodotta, non nei dati.
 *
 * Il censimento del primo passaggio spezzava le funzioni solo dopo un `ret`
 * (e dopo un `b` che uscisse dalla sezione). Ma una funzione puo' finire con
 * un `b` ALL'INDIETRO dentro se stessa -- il salto di ritorno di un ramo
 * freddo -- e in quel caso il primo passaggio la fondeva con la funzione
 * seguente. E' esattamente cio' che accade qui: la `.exit.text` del core
 * magnetometro finisce con
 *   "97c2c426 bl"@0xffffff80093aa5e4    -> __dynamic_pr_debug
 *   "17ffffe3 b"@0xffffff80093aa5e8     -> 0xffffff80093aa574, all'indietro
 * e i 4 byte successivi venivano contati come suo corpo. Con la regola giusta
 * (ogni `b` incondizionato chiude un blocco; se l'istruzione seguente non e'
 * bersaglio di alcun salto, comincia una funzione) il core magnetometro
 * misura 144 byte e non 148, e a 0xffffff80093aa5ec resta una funzione
 * separata di 4 byte:
 *   "d65f03c0 ret"@0xffffff80093aa5ec
 * Nessun salto, in tutte le 5.018.179 righe del disassemblato dell'immagine,
 * ha 0xffffff80093aa5ec come bersaglio: non e' codice raggiungibile dal
 * vicino, e' una funzione a se'.
 *
 * Col confine corretto la sezione ha 692 funzioni, non 669, e 26 corpi vuoti
 * da 4 byte, non 25.
 *
 * ATTRIBUZIONE -- e' un'inferenza dall'ORDINE DI LINK, non la lettura di un
 * nome, perche' una funzione vuota non nomina niente. Va pesata per quello
 * che e'. Tre sezioni diverse dell'immagine, disposte tutte in ordine di
 * link, danno lo stesso ordinamento degli oggetti, e i 4 byte cadono nella
 * stessa lacuna in tutte e tre:
 *
 *   oggetto        dato                .init.text            .exit.text
 *   core mag       0xffffff800992ee60  mag_init   0x9374140  0xffffff80093aa55c
 *   >>> qmcX983    0xffffff800992f138  qmcX983_init 0x9374654 >>> 0xffffff80093aa5ec
 *   scp            0xffffff800992f750  scp_init   0x9374674  0xffffff80093aa5f0
 *
 *  - il dato: "91398000 add"@0xffffff80093aa5b8 mette 0xffffff800992ee60 in
 *    x0 per la platform_driver_unregister del core mag, indirizzo che nel
 *    resto dell'immagine e' usato solo da `mag_driver_add`;
 *    "911d4000 add"@0xffffff80093aa618 mette 0xffffff800992f750 in x0 per la
 *    misc_deregister di scp, indirizzo usato solo da `scp_init`; e
 *    `qmcX983_init_info` sta a 0xffffff800992f138, in mezzo ai due
 *    ("9104e000 add"@0xffffff8009374660);
 *  - la `.init.text`: in `oracolo/stock.map` le tre righe sono CONSECUTIVE,
 *    mag_init / qmcX983_init / scp_init, nessun altro simbolo fra loro;
 *  - la `.exit.text`: fra la fine del core mag (0xffffff80093aa5ec) e
 *    l'inizio di scp ("f81e0ff3 str"@0xffffff80093aa5f0) c'e' esattamente
 *    questa funzione e nient'altro.
 *
 * Il metodo e' TARATO su tre casi in cui la funzione si nomina da sola e
 * cade comunque nella lacuna che l'ordine di link prevede: `flashlight_exit`,
 * `flashlight_mt6370_exit` e `mir3da_exit` -- quest'ultimo a
 * 0xffffff80093aa528, nella lacuna fra il core accelerometro e il core mag,
 * dove `mir3da_init` sta fra `acc_init` e `mag_init`. Sulle 15 coppie
 * init/exit dei driver di questo progetto l'ordine e' monotono senza
 * eccezioni.
 *
 * CIO' CHE RESTA NON PROVATO, e va detto: un oggetto che avesse una `__exit`
 * vuota e NESSUN simbolo in `.init.text` e NESSUN dato proprio sarebbe
 * invisibile a tutti e tre gli ordinamenti, e potrebbe occupare questa
 * lacuna al posto di qmcX983. Non c'e' modo di escluderlo dal binario.
 *
 * IL CORPO, invece, e' misurato e non ammette alternative: 4 byte, un solo
 * `ret`, nessun accesso a memoria, nessuna chiamata. Coerente con
 * `qmcX983_init`, che si limita a `mag_driver_add`: `mag_driver_del` non
 * esiste come simbolo in stock.map, quindi non c'e' niente da disfare.
 *
 * Il `module_exit()` qui sotto e' una SCELTA dichiarata, non una misura: il
 * puntatore che genera finisce in `.exitcall.exit`, che vmlinux.lds scarta
 * (`EXIT_CALL` dentro /DISCARD/), mentre la funzione sopravvive comunque
 * perche' per un file built-in `__exit` implica `__used`
 * (include/linux/init.h righe 78-85). Dal binario «con module_exit()» e
 * «senza» sono indistinguibili.
 *
 * MISURA DI DIMENSIONE NON FATTA: questo lotto non ha un albero di build per
 * qmcX983 e non ha compilato il file. I 4 byte di fabbrica sono letti dal
 * disassemblato; il confronto con il nostro `.o` VA ANCORA FATTO.
 */
static void __exit qmcX983_exit(void)
{
}

module_exit(qmcX983_exit);

MODULE_DESCRIPTION("QST QMC7983 magnetometer (Doogee S88 Pro, dal disassemblato)");
MODULE_LICENSE("GPL");
/*
 * ___modver_attr a 0xffffff800992f0f0, appena prima di mag_init_info: i due
 * puntatori "orfani" che sembravano un'anomalia (0xffffff800992f128 e
 * f130) sono i campi module_name/version di questa struttura, raggiunta solo
 * dalla sezione __modver che kernel/params.c percorre al boot -- nessuna
 * funzione del driver la legge, per questo nessuna delle 49 la referenzia.
 * module_name punta alla stessa stringa "qmcX983" di mag_init_info.name
 * (0xffffff80091c13a7); version punta a "driver version 3.4"@0xffffff80091c1e59.
 * Verificato campo per campo dalla revisione indipendente
 * (docs/bringup/rapporti/revisione-qmcx983-completo.md, R1/§7).
 */
MODULE_VERSION("driver version 3.4");
