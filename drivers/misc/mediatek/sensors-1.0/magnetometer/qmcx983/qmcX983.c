// SPDX-License-Identifier: GPL-2.0
/*
 * QST QMC7983 magnetometer ("qmcX983" in the factory symbols), Doogee S88
 * Pro.
 *
 * Reconstructed from the disassembly of the factory kernel, not adapted from
 * another phone. Every constant carries the disassembly line it was read from
 * -- a value without its citation is a defect.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_magnetometer_qmcx983_qmcX983.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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
 * The two printk levels, read from the prefix of every format:
 *   "\x016[QMC-Msensor] ..."          -> KERN_INFO, no implicit arguments
 *   "\x013[QMC-Msensor] %s %d : ..."  -> KERN_ERR, with __func__ and __LINE__
 * First citation of each:
 *   <<\x016[QMC-Msensor] %s\n>>@0xffffff80091c1410
 * the fragment is "%s\n", the same text scnprintf uses elsewhere;
 * here it sits at 0xffffff80091c1420, the clinching citation is
 * the one at 0xffffff8009226be1
 * the fragment is "%s\n", the same text scnprintf uses elsewhere;
 * here it sits at 0xffffff80091c1420, the clinching citation is
 * the one at 0xffffff8009226be1
 *   <<\x013[QMC-Msensor] %s %d : add driver error\n>>@0xffffff80091c13af
 * the fragment that appears in the code: "add driver error\n"@0xffffff80091c13c7
 */
#define MAGN_TAG	"[QMC-Msensor] "
#define MAGN_LOG(fmt, args...)	pr_info(MAGN_TAG fmt, ##args)
#define MAGN_ERR(fmt, args...)	\
	pr_err(MAGN_TAG "%s %d : " fmt, __func__, __LINE__, ##args)

/*
 * "qmcX983"@0xffffff80091c13a7, pointed at by the relocations a
 * 0xffffff800992f138 (mag_init_info.name) e 0xffffff800992f198
 * (i2c_driver.driver.name).
 */
#define QMCX983_DEV_NAME	"qmcX983"

/*
 * <<orr w1,wzr,#0x20>>@0xffffff800878f558 (third argument of scnprintf in
 * show_chipinfo_value): the buffer scratch buffer is 32 byte.
 */
#define QMCX983_BUFSIZE		0x20

/*
 * <<cmp w21,#0x9>>@0xffffff800878dd80 in I2C_RxData: the read fails from 9
 * bytes upwards, and the message prints 8 (<<orr w4,wzr,#0x8>>@0xffffff800878dda8).
 */
#define C_I2C_FIFO_SIZE		8

/*
 * <<mov w8,#0x2c ; strh w8,[x21,#2]>>@0xffffff800878e144: the I2C address the
 * probe forces into client->addr.
 */
#define QMCX983_I2C_ADDR	0x2c

/* The registers, each from the point where the binary loads it. */
#define QMCX983_REG_DATA	0x00	/* <<strb wzr,[sp]>>@0xffffff800878f67c */
#define QMCX983_REG_STATUS	0x06	/* <<orr w22,wzr,#0x6>>@0xffffff800878f648 */
#define QMCX983_REG_CTRL1	0x09	/* <<mov w21,#0x9>>@0xffffff800878dc10 */
#define QMCX983_REG_CTRL2	0x0a	/* <<mov w8,#0xc0a>>@0xffffff80087902c8 */
#define QMCX983_REG_FBR		0x0b	/* <<mov w8,#0xb>>@0xffffff800878df74 */
#define QMCX983_REG_CHIPID	0x0d	/* <<mov w8,#0xd>>@0xffffff800878e208 */
#define QMCX983_REG_OTP_ADDR	0x2e	/* <<mov w9,#0x2e>>@0xffffff800878e2b4 */
#define QMCX983_REG_OTP_DATA	0x2f	/* <<mov w8,#0x2f>>@0xffffff800878e2e4 */

/*
 * The chip identifiers are the indices of the show_chipinfo_value jump table
 * ("adr x10,0xffffff800878f4e0" with the byte table 00 05 09 0c 13 at
 * 0xffffff8008f54570): 0..4, in the order of the five strings reached.
 */
#define QMC6983_A1_D1		0	/* "QMC6983_A1_D1 Chip"@0xffffff80091c1981 */
#define QMC6983_E1		1	/* "QMC6983_E1 Chip"@0xffffff80091c195b */
#define QMC6983_E1_METAL	2	/* "QMC6983_E1_Metal Chip"@0xffffff80091c196b */
#define QMC7983_VERTICAL	3	/* "QMC7983_Vertical Chip"@0xffffff80091c1932 */
#define QMC7983_SLOPE		4	/* "QMC7983_Slope Chip"@0xffffff80091c1948 */

/*
 * QMCX983IO = 0x83 and the two commands, from the two comparisons in
 * qmcX983_unlocked_ioctl:
 *   <<mov w8,#0x8341 ; movk w8,#0xc008,lsl #16>>@0xffffff800878fb48 -> 0xc0088341
 *   <<mov w8,#0x8340 ; movk w8,#0x4008,lsl #16>>@0xffffff800878fb58 -> 0x40088340
 * that is _IOWR/_IOW(0x83, 0x41/0x40, char[8]). The encoded size is 8 while
 * the buffer actually copied is 16
 * (<<orr w2,wzr,#0x10>>@0xffffff800878fc64): the discrepancy is the factory's.
 */
#define QMCX983IO		0x83
#define QMCX983_IOC_WRITE	_IOW(QMCX983IO, 0x40, char[8])
#define QMCX983_IOC_READ	_IOWR(QMCX983IO, 0x41, char[8])

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800878e124, 64 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_magnetometer_qmcx983_qmcX983.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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
 * The global struct mag_hw that get_cust_mag() returns. It is not static: the
 * symbol in the oracle is T (global), and in the "sibling" driver of the same
 * MediaTek family (akm09918.c, same tree, line "struct mag_hw mag_cust;") the
 * field is defined the same way -- get_cust_mag returns its address directly
 * (a single adrp/add, no intermediate ldr from a separate pointer), which is
 * exactly the pattern here. The real address (0xffffff8009cba228) is in the
 * factory .bss.
 */
struct mag_hw mag_cust;

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8009cba248).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_magnetometer_qmcx983_qmcX983.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct i2c_client *this_client;
static struct mutex read_i2c_xyz;
static struct mutex sensor_data_mutex;
static atomic_t open_count;
static bool qmcX983_init_flag;
static int otp_a;
static int otp_b;
/*
 * open_flag is an atomic_t and not an int: qmcX983_m_enable assigns v_open_flag
 * to it (<<str w3,[x8,#676]>>@0xffffff800878eda0) and then READS IT BACK
 * (<<ldr w4,[x8,#676]>>@0xffffff800878eda4) to pass it to the printk. On a plain
 * int the compiler would have reused w3: the re-read is the signature of
 * atomic_set/atomic_read (WRITE_ONCE/READ_ONCE).
 */
static atomic_t open_flag;
static unsigned char v_open_flag;
static unsigned char hw_registers;

/*
 * chip_id lives in .data (0xffffff800992f2a0) and is 1 in the factory image
 * (the word at that address is 0x0000000000000001, with no relocation):
 * it is initialised to QMC6983_E1, not to zero. The "default" branch of
 * qmcX983_device_check leaves it alone, and the probe notices
 * (<<ldr w8,[x8,#672] ; cbz w8>>@0xffffff800878e390).
 */
static int chip_id = QMC6983_E1;

static int qmcX983_local_init(void);
static int qmcX983_local_remove(void);

/*
 * get_cust_mag: 0xffffff800878dba4, 12 byte, T (global).
 *   adrp x0, 0xffffff8009cba000 ; add x0, x0, #0x228 ; ret
 * Returns &mag_cust (0xffffff8009cba228, in the factory .bss).
 */
struct mag_hw *get_cust_mag(void)
{
	return &mag_cust;
}

/*
 * mag_i2c_read_block() was reconstructed from the factory kernel disassembly (0xffffff80091c1317, 192 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_magnetometer_qmcx983_qmcX983.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static inline int mag_i2c_read_block(struct i2c_client *client, u8 addr,
				     u8 *data, u8 len)
{
	u8 beg = addr;
	/*
	 * The explicit clearing is not stylistic redundancy: the factory really does
	 * it before filling the fields
	 * (<<str xzr,[sp,#8]>>@0xffffff800878dd44 and
	 *  <<stp xzr,xzr,[sp,#16]>>@0xffffff800878dd3c, each followed by the real
	 * field writes). With a complete designated initialiser clang does not emit
	 * those three stores and the function loses 12 bytes
	 * (measured: 332 instead of 344).
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
		/*
		 * "\x013[QMC-Msensor] %s %d :  length %d exceeds %d\n"
		 * @0xffffff80091c12e8, line 198 (<<mov w2,#0xc6>>@0xffffff800878dda4).
		 * the fragment that appears in the code: " length %d exceeds %d\n"@0xffffff80091c1300
		 * Two spaces after the colon: they are in the binary.
		 */
		MAGN_ERR(" length %d exceeds %d\n", len, C_I2C_FIFO_SIZE);
		return -EINVAL;
	}

	err = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (err != 2) {
		/*
		 * "\x013[QMC-Msensor] %s %d : i2c_transfer error: (%d %p %d) %d\n"
		 * @0xffffff80091c132a, line 204 (<<mov w2,#0xcc>>@0xffffff800878ddf8).
		 * the fragment that appears in the code: "i2c_transfer error: (%d %p %d) %d\n"@0xffffff80091c1342
		 * The unlock sits inside the branches and not at the tail: in the factory
		 * build the successful branch unlocks at 0xffffff800878ddd8 and returns at
		 * once, without carrying err along in a register.
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
		/*
		 * <<cmp w19,#0x8 ; b.cc>>@0xffffff800878fbec: on write the
		 * limit is 8 exclusive (len+1 bytes are transmitted), while on
		 * read it was 9 exclusive. Line 227
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
	/*
	 * Here the unlock precedes the sign check: in the factory build
	 * "mutex_unlock" sits at 0xffffff800878dc80 and the "tbnz w19,#31" only
	 * after it, at 0xffffff800878dc84.
	 */
	mutex_unlock(&read_i2c_xyz);
	if (err < 0) {
		/*
		 * "\x013[QMC-Msensor] %s %d : send command error!!\n"
		 * @0xffffff80091c1379, line 239 (<<mov w2,#0xef>>@0xffffff800878dcb8)
		 * the fragment that appears in the code: "send command error!!\n"@0xffffff80091c1391
		 */
#line 239
		MAGN_ERR("send command error!!\n");
		return -EFAULT;
	}
	/*
	 * 0 and not err: the callers write "if (mag_i2c_write_block(...))" and in the
	 * factory build the successful branch returns 0
	 * (<<mov w0,wzr>>@0xffffff800878dc88 in qmcX983_set_range). The binary does
	 * not distinguish this form from "return err" with a caller testing "< 0":
	 * the first was chosen.
	 */
	return 0;
}

/*
 * I2C_RxData: 0xffffff800878dcf0, 344 bytes, t (static).
 * The two entry checks return -EINVAL
 * (<<mov w0,#0xffffffea>>@0xffffff800878dd10), every later failure -1
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
 * I2C_TxData has no symbol of its own: it is always inlined. The pointer
 * check stays visible as <<cmn x22,#0x1 ; b.eq>>@0xffffff800878fbc4 inside
 * qmcX983_unlocked_ioctl, that is, the comparison of &sData[1] with NULL.
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
 * qmcX983_set_range: 0xffffff800878dbb0, 320 byte, T (global).
 */
int qmcX983_set_range(short range)
{
	int err = 0;
	unsigned char data[2];
	int ran;
	struct i2c_client *client = this_client;
	struct qmcX983_i2c_data *obj = i2c_get_clientdata(client);

	/*
	 * <<cmp w9,#0x3 ; b.hi>>@0xffffff800878dbd0 on (range & 0xffff): above 3
	 * it returns -EINVAL (<<mov w0,#0xffffffea>>@0xffffff800878dc90)
	 */
	if ((unsigned short)range > 3)
		return -EINVAL;

	/*
	 * "mov x9,#0x2 ; movk x9,#0x8,lsl#16 ; movk x9,#0xc,lsl#32 ;
	 *  movk x9,#0x14,lsl#48"@0xffffff800878dbe4: the 64-bit constant
	 * 0x0014000c00080002 is shifted by (range*16) bits
	 * (<<ubfiz x8,x8,#4,#32 ; lsr x8,x9,x8>>@0xffffff800878dbfc) and truncated to
	 * 16 bits, that is, it is the table {2, 8, 12, 20} indexed by range.
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

	/*
	 * The value returned is the write's, not the read's: in the factory build
	 * the successful branch puts zero in w0
	 * (<<mov w0,wzr>>@0xffffff800878dc88) without reading anything back.
	 */
	err = I2C_TxData(data, 2);
	return err;
}

/*
 * qmcX983_set_mode: 0xffffff800878de48, 244 byte, T (global).
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
 * qmcX983_set_ratio: 0xffffff800878df3c, 196 bytes, T (global).
 * No preliminary read: it just writes.
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

	/*
 * <<\x016[QMC-Msensor] %s\n>>@0xffffff80091c1410 with
 * the fragment is "%s\n", the same text scnprintf uses elsewhere;
 * here it sits at 0xffffff80091c1420, the clinching citation is
 * the one at 0xffffff8009226be1
 * the fragment is "%s\n", the same text scnprintf uses elsewhere;
 * here it sits at 0xffffff80091c1420, the clinching citation is
 * the one at 0xffffff8009226be1
 * <<qmcX983_read_mag_xyz>>@0xffffff80091c199f
 */
	MAGN_LOG("%s\n", __func__);

	/*
	 * <<cmp w21,#0x2 ; b.hi>>@0xffffff800878f65c with w21 initialised to 1
	 * (<<orr w21,wzr,#0x1>>@0xffffff800878f644): at most three reads.
	 * <<and w8,w8,#0x7>>@0xffffff800878f66c: it exits as soon as one of the three
	 * low bits of the status register is high.
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

	/*
	 * <<mov w11,#0x3e8>>@0xffffff800878f6b8 = 1000; the divisor for x and y is
	 * the field at +56, the one for z the field at +58
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
 * qmcX983_enable: 0xffffff800879014c, 964 bytes, t.
 * Register w0 is never rewritten before the epilogue: the function returns
 * nothing, and is therefore void -- consistent with neither of its two
 * callers (qmcX983_m_enable, qmcX983_factory_enable_sensor) using the result.
 * A function declared int that falls through without a return would be an
 * error here, under -Wreturn-type inside -Werror.
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
 * FctShipmntTestProcess_Body: 0xffffff800878e000, 8 bytes, T (global).
 *   orr w0, wzr, #0x1 ; ret
 * Name and global linkage taken from the oracle as they stand.
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
			/*
			 * "\x013[QMC-Msensor] %s %d : HWMSEN_GET_CONVERT function error!\r\n"
			 * @0xffffff80091c19e0, line 608
			 * the fragment that appears in the code: "HWMSEN_GET_CONVERT function error!\r\n"@0xffffff80091c19f8
			 * (<<mov w2,#0x260>>@0xffffff800878f8a0)
			 */
#line 608
			MAGN_ERR("HWMSEN_GET_CONVERT function error!\r\n");
		} else if (!hwmsen_get_convert(data->hw.direction, &data->cvt)) {
			/*
			 * "\x013[QMC-Msensor] %s %d : invalid layout: %d, restore to %d\n"
			 * @0xffffff80091c1a30, line 612
			 * the fragment that appears in the code: "invalid layout: %d, restore to %d\n"@0xffffff80091c1a48
			 * (<<mov w2,#0x264>>@0xffffff800878f8bc)
			 */
			MAGN_ERR("invalid layout: %d, restore to %d\n",
#line 612
				 layout, data->hw.direction);
		} else {
			/*
			 * "\x013[QMC-Msensor] %s %d : invalid layout: (%d, %d)\n"
			 * @0xffffff80091c1a6b, line 616
			 * the fragment that appears in the code: "invalid layout: (%d, %d)\n"@0xffffff80091c1a83
			 * (<<mov w2,#0x268>>@0xffffff800878f858)
			 */
			MAGN_ERR("invalid layout: (%d, %d)\n",
#line 616
				 layout, data->hw.direction);
			hwmsen_get_convert(0, &data->cvt);
		}
	} else {
		/*
		 * "\x013[QMC-Msensor] %s %d : invalid format = '%s'\n"
		 * @0xffffff80091c1a9d, line 622 (<<mov w2,#0x26e>>@0xffffff800878f880)
		 * the fragment that appears in the code: "invalid format = '%s'\n"@0xffffff80091c1ab5
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
		/*
		 * "\x013[QMC-Msensor] %s %d : qmcX983_i2c_data is null!!\n"
		 * @0xffffff80091c1804, line 654 (<<mov w2,#0x28e>>@0xffffff800878f9dc)
		 * the fragment that appears in the code: "qmcX983_i2c_data is null!!\n"@0xffffff80091c181c
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
	/*
	 * Not initialised: the factory does not clear this variable before the
	 * sscanf -- in store_layout_value it does
	 * (<<str wzr,[sp,#4]>>@0xffffff800878f808), here that store is absent.
	 * Clearing it costs 4 bytes more (measured: 212 instead of 208).
	 */
	int trace;

	if (data == NULL) {
		/* line 668 (<<mov w2,#0x29c>>@0xffffff800878fa60) */
#line 668
		MAGN_ERR("qmcX983_i2c_data is null!!\n");
		return -EINVAL;
	}
	/* "0x%x"@0xffffff8009142911 */
	if (sscanf(buf, "0x%x", &trace) == 1) {
		atomic_set(&data->trace, trace);
	} else {
		/*
		 * "\x013[QMC-Msensor] %s %d : invalid content: '%s', length = %zd\n"
		 * @0xffffff80091c1b32, line 678 (<<mov w2,#0x2a6>>@0xffffff800878fa80)
		 * the fragment that appears in the code: "invalid content: '%s', length = %zd\n"@0xffffff80091c1b4a
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
	/*
	 * <<\x016[QMC-Msensor] QMCX983 hw_registers = 0x%02x\n>>@0xffffff80091c17d5
	 * the fragment that appears in the code: "QMCX983 hw_registers = 0x%02x\n"@0xffffff80091c17e5
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
		/* line 714 (<<mov w2,#0x2ca>>@0xffffff800878f35c) */
#line 714
		MAGN_ERR("qmcX983_i2c_data is null!!\n");
		return -EINVAL;
	}
	/*
	 * "\x013[QMC-Msensor] %s %d : QMC6938:store_WRregisters_value: 0x%2x \n"
	 * @0xffffff80091c1850, line 718 (<<mov w2,#0x2ce>>@0xffffff800878f2dc)
	 * the fragment that appears in the code: "QMC6938:store_WRregisters_value: 0x%2x \n"@0xffffff80091c1868
	 */
#line 718
	MAGN_ERR("QMC6938:store_WRregisters_value: 0x%2x \n", buf[0]);

	databuf[0] = hw_registers;
	databuf[1] = buf[0];
	mag_i2c_write_block(client, databuf[0], &databuf[1], 1);

	/*
	 * "\x013[QMC-Msensor] %s %d : QMC6938: write registers 0x%2x  ---> 0x%2x success! \n"
	 * @0xffffff80091c1891, line 723 (<<mov w2,#0x2d3>>@0xffffff800878f38c);
	 * the fragment that appears in the code: "QMC6938: write registers 0x%2x  ---> 0x%2x success! \n"@0xffffff80091c18a9
	 * the first argument is hw_registers read back
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
		/* line 740 (<<mov w2,#0x2e4>>@0xffffff800878f474) */
#line 740
		MAGN_ERR("qmcX983_i2c_data is null!!\n");
		return -EINVAL;
	}
	hw_registers = buf[0];
	/*
	 * "\x013[QMC-Msensor] %s %d : QMC6938: REGISTERS = 0x%2x\n"
	 * @0xffffff80091c18f5, line 744 (<<mov w2,#0x2e8>>@0xffffff800878f454)
	 * the fragment that appears in the code: "QMC6938: REGISTERS = 0x%2x\n"@0xffffff80091c190d
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
		/*
		 * <<strb wzr,[sp,#8]>>@0xffffff800878f4ec: a single byte cleared,
		 * not the whole buffer
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

	/* "CUST: %d %d (%d %d)\n"@0xffffff80091c1acc, arguments read as
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
 * show_regiter_map: 0xffffff800878f0d0, 276 bytes, t. The name carries the
 * factory typo ("regiter"), taken from the oracle symtab as it stands.
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
			/*
			 * "\x016[QMC-Msensor] QMCX983 dump registers 0x%02x failed !\n"
			 * @0xffffff80091c175d
			 * the fragment that appears in the code: "QMCX983 dump registers 0x%02x failed !\n"@0xffffff80091c176d
			 */
			MAGN_LOG("QMCX983 dump registers 0x%02x failed !\n", i);
		}
		/* "reg[0x%2x] =  0x%2x \n"@0xffffff80091c1795 */
		len = scnprintf(tmpbuf, sizeof(tmpbuf),
				"reg[0x%2x] =  0x%2x \n", i, databuf[0]);
		/*
		 * <<mul w8,w0,w20>>@0xffffff800878f16c: the offset is len*i, not the
		 * sum of the lengths -- that is how the factory has it.
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

	/*
	 * <<\x016[QMC-Msensor] shipment_test pass\n>>@0xffffff80091c1728
	 * the fragment that appears in the code: "shipment_test pass\n"@0xffffff80091c1738
	 */
	MAGN_LOG("shipment_test pass\n");
	/* <<mov w8,#0x79 ; strh w8,[sp,#12]>>@0xffffff800878f084: two byte, 'y'
	 * e the terminator
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
 * This section was reconstructed from the factory kernel disassembly (0xffffff800992f2a8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_magnetometer_qmcx983_qmcX983.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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
 * qmcX983_create_attr: no symbol of its own, always inlined into the
 * probe; the name and the line stay in the printk
 * (<<qmcX983_create_attr>>@0xffffff80091c1707, line 852).
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
			/*
			 * "\x013[QMC-Msensor] %s %d : driver_create_file (%s) = %d\n"
			 * @0xffffff80091c16d1, line 852
			 * the fragment that appears in the code: "driver_create_file (%s) = %d\n"@0xffffff80091c16e9
			 * (<<mov w2,#0x354>>@0xffffff800878e860); the first
			 * argument is attr->attr.name
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
 * qmcX983_delete_attr: 0xffffff800878ef8c, 200 bytes, t.
 * The ten driver_remove_file calls are unrolled in the exact order of the
 * relocation table 0xffffff800992f2a8..0xffffff800992f3c8.
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
	/*
	 * ret is an int and not a long: in the factory build the sign extension
	 * appears only on the error path (<<sxtw x0,w0>>@0xffffff800878fd70), that
	 * is, at the conversion of the return value, not right after the call.
	 */
	int ret = 0;
	struct qmcX983_i2c_data *data = i2c_get_clientdata(this_client);

	/* <<ldr w8,[x8,#44] ; tbz w8,#0>>@0xffffff800878fb30: bit 0 of trace */
	if (data && (atomic_read(&data->trace) & 0x01)) {
		/*
		 * "\x016[QMC-Msensor] qmcX983_unlocked_ioctl !cmd= 0x%x\n"
		 * @0xffffff80091c1b7b
		 * the fragment that appears in the code: "qmcX983_unlocked_ioctl !cmd= 0x%x\n"@0xffffff80091c1b8b
		 */
		MAGN_LOG("qmcX983_unlocked_ioctl !cmd= 0x%x\n", cmd);
	}

	switch (cmd) {
	case QMCX983_IOC_WRITE:
		if (argp == NULL) {
			/*
			 * <<\x016[QMC-Msensor] invalid argument.>>@0xffffff80091c1bae
			 * the fragment that appears in the code: "invalid argument."@0xffffff80091c1bbe
			 */
			MAGN_LOG("invalid argument.");
			return -EINVAL;
		}
		if (copy_from_user(sData, argp, sizeof(sData))) {
			/*
			 * "\x016[QMC-Msensor] copy_from_user failed."
			 * @0xffffff80091c1bd0
			 * the fragment that appears in the code: "copy_from_user failed."@0xffffff80091c1be0
			 */
			MAGN_LOG("copy_from_user failed.");
			return -EFAULT;
		}
		/*
		 * <<sub w8,w19,#0x2 ; cmp w8,#0xe ; b.cs>>@0xffffff800878fbb4:
		 * only sData[0] between 2 and 15 is allowed
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
		/*
		 * <<sub w8,w1,#0x1 ; cmp w8,#0xf ; b.cc>>@0xffffff800878fc74:
		 * only sData[0] between 1 and 15 is allowed
		 */
		if (sData[0] < 1 || sData[0] >= sizeof(sData)) {
			MAGN_LOG("invalid argument.");
			return -EINVAL;
		}
		ret = I2C_RxData(&sData[1], sData[0]);
		if (ret < 0)
			return ret;
		if (copy_to_user(argp, sData, sData[0] + 1)) {
			/*
			 * "\x016[QMC-Msensor] copy_to_user failed."
			 * @0xffffff80091c1bf7
			 * the fragment that appears in the code: "copy_to_user failed."@0xffffff80091c1c07
			 */
			MAGN_LOG("copy_to_user failed.");
			return -EFAULT;
		}
		return 0;

	default:
		/*
		 * "\x013[QMC-Msensor] %s %d : %s not supported = 0x%04x"
		 * @0xffffff80091c1c1c, line 1070
		 * the fragment that appears in the code: "%s not supported = 0x%04x"@0xffffff80091c1c34
		 * (<<mov w2,#0x42e>>@0xffffff800878fca4);
		 * <<mov x0,#0xfffffffffffffdfd>>@0xffffff800878fcb4 = -ENOIOCTLCMD
		 */
		/*
		 * Every branch leaves through a return of its own, not through an
		 * accumulator: in the factory build each exit writes directly
		 * into x0 (<<mov x0,#0xfffffffffffffdfd>>@0xffffff800878fcb4) and
		 * jumps to the common epilogue.
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

	/* <<tbz w8,#3>>@0xffffff800878fe6c: bit 3 of trace */
	if (atomic_read(&obj->trace) & 0x08) {
		/*
		 * "\x016[QMC-Msensor] Open device node:qmcX983\n"
		 * @0xffffff80091c1c65
		 * the fragment that appears in the code: "Open device node:qmcX983\n"@0xffffff80091c1c75
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

	/*
	 * ldxr/"sub w10,w10,#0x1"/stxr on 0xffffff8009cba270
	 * @0xffffff800878feac: it is an atomic_dec, not a plain write
	 */
	atomic_dec(&open_count);
	if (atomic_read(&obj->trace) & 0x08) {
		/*
		 * "\x016[QMC-Msensor] Release device node:qmcX983\n"
		 * @0xffffff80091c1c8f
		 * the fragment that appears in the code: "Release device node:qmcX983\n"@0xffffff80091c1c9f
		 */
		MAGN_LOG("Release device node:qmcX983\n");
	}
	return 0;
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800992f3e8).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_magnetometer_qmcx983_qmcX983.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static struct file_operations qmcX983_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = qmcX983_unlocked_ioctl,
	.open = qmcX983_open,
	.release = qmcX983_release,
};

/*
 * miscdevice a 0xffffff800992f240: minor 0xff = MISC_DYNAMIC_MINOR, name
 * relocated to "qst_msensor"@0xffffff80091c1b6f, fops a 0xffffff800992f3e8.
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
		/*
		 * "\x013[QMC-Msensor] %s %d : this_client is null!\n"
		 * @0xffffff80091c1d3f, line 1198
		 * the fragment that appears in the code: "this_client is null!\n"@0xffffff80091c1d57
		 * (<<mov w2,#0x4ae>>@0xffffff800878ee5c)
		 */
#line 1198
		MAGN_ERR("this_client is null!\n");
		return -EINVAL;
	}
	if (i2c_get_clientdata(client) == NULL) {
		/*
		 * "\x013[QMC-Msensor] %s %d : data is null!\n"
		 * @0xffffff80091c1d7e, line 1204
		 * the fragment that appears in the code: "data is null!\n"@0xffffff80091c1d96
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
		/* line 1221 (<<mov w2,#0x4c5>>@0xffffff800878edf8) */
#line 1221
		MAGN_ERR("this_client is null!\n");
		return -EINVAL;
	}
	data = i2c_get_clientdata(client);
	if (data == NULL) {
		/* line 1228 (<<mov w2,#0x4cc>>@0xffffff800878ee10) */
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
	/*
	 * "\x013[QMC-Msensor] %s %d : qmcX983 v_open_flag = 0x%x,open_flag= 0x%x\n"
	 * @0xffffff80091c1da5, line 1246 (<<mov w2,#0x4de>>@0xffffff800878edb8)
	 * the fragment that appears in the code: "qmcX983 v_open_flag = 0x%x,open_flag= 0x%x\n"@0xffffff80091c1dbd
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
 * qmcX983_flush: 0xffffff800878ee98, 20 bytes, t.
 *   bl mag_flush_report ; ret -- the value returned is the helper's
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
		/* line 1268 (<<mov w2,#0x4f4>>@0xffffff800878ef60) */
		MAGN_ERR("this_client is null!\n");
		return -EINVAL;
	}
	if (i2c_get_clientdata(client) == NULL) {
		/* line 1274 (<<mov w2,#0x4fa>>@0xffffff800878ef78) */
		MAGN_ERR("data is null!\n");
		return -EINVAL;
	}

	qmcX983_read_mag_xyz(mag);
	*x = mag[0];
	*y = mag[1];
	*z = mag[2];
	/* <<orr w9,wzr,#0x3>>@0xffffff800878eefc */
	*status = 3;
	/*
	 * "orr w0,wzr,#0x1 ; bl ktime_get_with_offset ; bl ns_to_timespec"
	 * @0xffffff800878ef00: offset 1 is TK_OFFS_BOOT, and the result is not
	 * used anywhere -- that is how the factory has it.
	 */
	time = ktime_to_timespec(ktime_get_boottime());
	return 0;
}

/*
 * qmcX983_factory_enable_sensor: 0xffffff800878fee4, 260 bytes, t.
 * Same body as qmcX983_m_enable but with the two error messages at
 * KERN_INFO and without the final printk.
 */
static int qmcX983_factory_enable_sensor(bool enable_disable,
					 int64_t sample_periods_ms)
{
	unsigned char databuf[2];
	struct i2c_client *client = this_client;
	struct qmcX983_i2c_data *data;

	if (client == NULL) {
		/*
		 * <<\x016[QMC-Msensor] this_client is null!\n>>@0xffffff80091c1cbc
		 * the fragment is the same text as the KERN_ERR variant, already
		 * cited at 0xffffff80091c1d57: here it sits at 0xffffff80091c1ccc
		 */
		MAGN_LOG("this_client is null!\n");
		return -EINVAL;
	}
	data = i2c_get_clientdata(client);
	if (data == NULL) {
		/*
		 * <<\x016[QMC-Msensor] data is null!\n>>@0xffffff80091c1ce2
		 * the fragment is the same text as the KERN_ERR variant, already
		 * cited at 0xffffff80091c1d96: here it sits at 0xffffff80091c1cf2
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
 * qmcX983_factory_get_data: 0xffffff800878ffe8, 284 bytes, t.
 * The two error printks carry __func__ = "qmcX983_m_get_data" and lines
 * 1268/1274: qmcX983_m_get_data is inlined in here, and the three locals
 * passed to it have to be cleared at the start, because on the error path
 * the binary writes zero into the three elements anyway
 * (<<mov w8,wzr ; mov w9,wzr ; mov w10,wzr>>@0xffffff80087900ec).
 */
static int qmcX983_factory_get_data(int32_t data[3], int *status)
{
	int x = 0, y = 0, z = 0;
	int err;

	err = qmcX983_m_get_data(&x, &y, &z, status);
	/* multiplication by 0x66666667 e "asr x9,x9,#34"
	 * @0xffffff8008790064: integer division by 10
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
	/*
	 * "\x016[QMC-Msensor] do not support qmcX983_factory_get_raw_data!\n"
	 * @0xffffff80091c1d01
	 * the fragment that appears in the code: "do not support qmcX983_factory_get_raw_data!\n"@0xffffff80091c1d11
	 */
	MAGN_LOG("do not support qmcX983_factory_get_raw_data!\n");
	return 0;
}

/*
 * 0xffffff8008790124, 0x879012c, 0x8790134, 0x879013c, 0x8790144: 8 byte
 * each, "mov w0,wzr ; ret".
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

/*
 * mag_factory_fops at 0xffffff800992f4d8: eight relocated pointers, one per
 * field, in the order of struct mag_factory_fops
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

/* mag_factory_public at 0xffffff800992f290: gain = 1 and sensitivity = 1
 * (the word at that address is 0x0000000100000001), fops relocated to
 * 0xffffff800992f4d8
 */
static struct mag_factory_public qmcX983_factory_device = {
	.gain = 1,
	.sensitivity = 1,
	.fops = &qmcX983_factory_fops,
};

/*
 * qmcX983_device_check: no symbol of its own, always inlined into the
 * probe; the name and the lines stay in the printks
 * (<<qmcX983_device_check>>@0xffffff80091c167d, lines 1472/1488/1495/1509/1515).
 */
static int qmcX983_device_check(void)
{
	int res = 0;
	unsigned char databuf[2] = {0};
	unsigned char value = 0;

	databuf[0] = QMCX983_REG_CHIPID;
	res = I2C_RxData(databuf, 1);
	if (res < 0) {
		/*
		 * "\x013[QMC-Msensor] %s %d : %s: I2C_RxData failed\n"
		 * @0xffffff80091c164e, line 1472
		 * the fragment that appears in the code: "%s: I2C_RxData failed\n"@0xffffff80091c1666
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
			/*
			 * "\x013[QMC-Msensor] %s %d : %s: I2C_TxData failed\n"
			 * @0xffffff80091c1692, line 1488
			 * the fragment that appears in the code: "%s: I2C_TxData failed\n"@0xffffff80091c16aa
			 * (<<mov w2,#0x5d0>>@0xffffff800878e94c)
			 */
#line 1488
			MAGN_ERR("%s: I2C_TxData failed\n", __func__);
			return res;
		}
		databuf[0] = QMCX983_REG_OTP_DATA;
		res = I2C_RxData(databuf, 1);
		if (res < 0) {
			/* line 1495 (<<mov w2,#0x5d7>>@0xffffff800878e9ec) */
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
				/* line 1509
				 * (<<mov w2,#0x5e5>>@0xffffff800878ea90)
				 */
#line 1509
				MAGN_ERR("%s: I2C_TxData failed\n", __func__);
				return res;
			}
			databuf[0] = QMCX983_REG_OTP_DATA;
			res = I2C_RxData(databuf, 1);
			if (res < 0) {
				/* line 1515
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
 * qmcx983_get_OTP() was reconstructed from the factory kernel disassembly (0xffffff80091c16c1).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_magnetometer_qmcx983_qmcX983.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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
		/* line 1549 (<<mov w2,#0x60d>>@0xffffff800878e8cc) */
#line 1549
		MAGN_ERR("%s: I2C_TxData failed\n", __func__);
		return res;
	}
	mdelay(10);
	databuf[0] = QMCX983_REG_OTP_DATA;
	res = I2C_RxData(databuf, 1);
	if (res < 0) {
		/* line 1557 (<<mov w2,#0x615>>@0xffffff800878e90c) */
		MAGN_ERR("%s: I2C_RxData failed\n", __func__);
		return res;
	}
	value1 = databuf[0];
	/*
	 * "and w11,w8,#0x1f ; orr w12,w8,#0xffffffe0 ; tst w8,#0x20 ; csel"
	 * @0xffffff800878e484: a signed 6-bit field
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
		/* line 1576 (<<mov w2,#0x628>>@0xffffff800878ea24) */
#line 1576
		MAGN_ERR("%s: I2C_TxData failed\n", __func__);
		return res;
	}
	mdelay(10);
	databuf[0] = QMCX983_REG_OTP_DATA;
	res = I2C_RxData(databuf, 1);
	if (res < 0) {
		/* line 1584 (<<mov w2,#0x630>>@0xffffff800878ea40) */
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
		/* line 1594 (<<mov w2,#0x63a>>@0xffffff800878eac0) */
#line 1594
		MAGN_ERR("%s: I2C_TxData failed\n", __func__);
		return res;
	}
	mdelay(10);
	databuf[0] = QMCX983_REG_OTP_DATA;
	res = I2C_RxData(databuf, 1);
	if (res < 0) {
		/* line 1602 (<<mov w2,#0x642>>@0xffffff800878eaf8) */
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
 * qmcX983_suspend: 0xffffff8008790510, 8 bytes, t.
 * qmcX983_resume:  0xffffff8008790518, 8 bytes, t.
 * Both "mov w0,wzr ; ret". The dev_pm_ops at 0xffffff8008f54708 fills
 * suspend/resume/freeze/thaw/poweroff/restore with the same pair (offsets
 * 0x10, 0x18, 0x20, 0x28, 0x30, 0x38): that is exactly SIMPLE_DEV_PM_OPS.
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

/*
 * i2c_device_id at 0xffffff8008f547c0: a single entry, "qmcX983", then the
 * terminator zeroed at 0xffffff8008f547e0
 */
static const struct i2c_device_id qmcX983_i2c_id[] = {
	{QMCX983_DEV_NAME, 0},
	{}
};

/*
 * of_device_id at 0xffffff8008f54578: the compatible field (offset 0x40 in the
 * structure, name[32] + type[32]) holds "mediatek,msensor"@0xffffff8008f545b8
 */
static const struct of_device_id mag_of_match[] = {
	{.compatible = "mediatek,msensor"},
	{},
};

static int qmcX983_i2c_probe(struct i2c_client *client,
			     const struct i2c_device_id *id);
static int qmcX983_i2c_remove(struct i2c_client *client);

/*
 * i2c_driver at 0xffffff800992f158: probe (+0x10), remove (+0x18), detect
 * (+0xc0) relocated; driver.name -> "qmcX983", driver.of_match_table ->
 * 0xffffff8008f54578, driver.pm -> 0xffffff8008f54708, id_table ->
 * 0xffffff8008f547c0. Every other field is zero.
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
 * mag_init_info at 0xffffff800992f138: name/init/uninit relocated to
 * "qmcX983", qmcX983_local_init, qmcX983_local_remove; platform_diver_addr
 * (0xffffff800992f150) stays zero in the image and is filled in at run time
 * by mag_driver_add.
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

	/*
	 * <<mov w1,#0xc0 ; movk w1,#0x140,lsl#16>>@0xffffff800878e11c = 0x014000c0
	 * and <<orr w2,wzr,#0x40>>@0xffffff800878e124 = 64: GFP_KERNEL without
	 * __GFP_ZERO (0x8000), that is kmalloc and not kzalloc.
	 */
	data = kmalloc(sizeof(struct qmcX983_i2c_data), GFP_KERNEL);
	if (!data) {
		/*
		 * <<mov w20,#0xfffffff4>>@0xffffff800878e180 = -ENOMEM.
		 * This branch skips the kfree ("b 0xffffff800878e978", which is
		 * after the call to kfree).
		 */
		err = -ENOMEM;
		goto exit;
	}

	err = get_mag_dts_func(client->dev.of_node, &data->hw);
	if (err < 0) {
		/*
		 * "\x013[QMC-Msensor] %s %d : %s. get dts info fail\n"
		 * @0xffffff80091c1436, line 1637
		 * the fragment that appears in the code: "%s. get dts info fail\n"@0xffffff80091c144e
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
		/*
		 * "\x013[QMC-Msensor] %s %d : QMCX983 invalid direction: %d\n"
		 * @0xffffff80091c1465, line 1647
		 * the fragment that appears in the code: "QMCX983 invalid direction: %d\n"@0xffffff80091c147d
		 * (<<mov w2,#0x66f>>@0xffffff800878e174)
		 */
#line 1647
		MAGN_ERR("QMCX983 invalid direction: %d\n", data->hw.direction);
		goto exit_kfree;
	}
	/*
	 * <<\x016[QMC-Msensor] %s: direction: %d\n>>@0xffffff80091c149c
	 * the fragment that appears in the code: "%s: direction: %d\n"@0xffffff80091c14ac
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
		/*
 * <<\x016[QMC-Msensor] %s check ID faild!\n>>@0xffffff80091c14e0
 * the fragment that appears in the code: "%s check ID faild!\n"@0xffffff80091c14f0
 * -- the "faild" typo is the factory's
 */
		MAGN_LOG("%s check ID faild!\n", __func__);
		goto exit_kfree;
	}

	if (chip_id != QMC6983_A1_D1) {
		err = qmcx983_get_OTP();
		if (err < 0) {
			/*
			 * "\x016[QMC-Msensor] %s get OTP faild!\n"
			 * @0xffffff80091c1504
			 * the fragment that appears in the code: "%s get OTP faild!\n"@0xffffff80091c1514
			 */
			MAGN_LOG("%s get OTP faild!\n", __func__);
			goto exit_kfree;
		}
	} else {
		/* <<str wzr,[x9,#668]>>@0xffffff800878e6f8 e
		 * <<str w8,[x10,#672]>>@0xffffff800878e708 with w8 = 0
		 */
		otp_a = 0;
		otp_b = 0;
	}

	/*
	 * <<ldr x9,[x23,#336] ; adds x22,x9,#0x28>>@0xffffff800878e700:
	 * the argument is platform_diver_addr + 0x28, that is the offset of
	 * struct device_driver inside struct platform_driver, and the comparison
	 * with zero is the "driver == NULL" check made on the sum.
	 */
	err = qmcX983_create_attr(&qmcX983_init_info.platform_diver_addr->driver);
	if (err) {
		/*
		 * "\x013[QMC-Msensor] %s %d : create attribute err = %d\n"
		 * @0xffffff80091c1527, line 1683
		 * the fragment that appears in the code: "create attribute err = %d\n"@0xffffff80091c153f
		 * (<<mov w2,#0x693>>@0xffffff800878e894)
		 */
#line 1683
		MAGN_ERR("create attribute err = %d\n", err);
		goto exit_kfree;
	}

	err = misc_register(&qmcX983_device);
	if (err) {
		/*
		 * "\x013[QMC-Msensor] %s %d : qmcX983_device register failed\n"
		 * @0xffffff80091c155a, line 1691
		 * the fragment that appears in the code: "qmcX983_device register failed\n"@0xffffff80091c1572
		 * (<<mov w2,#0x69b>>@0xffffff800878eb1c)
		 */
#line 1691
		MAGN_ERR("qmcX983_device register failed\n");
		goto exit_misc_register_failed;
	}

	err = mag_factory_device_register(&qmcX983_factory_device);
	if (err) {
		/*
		 * "\x013[QMC-Msensor] %s %d : misc device register failed, err = %d\n"
		 * @0xffffff80091c1592, line 1697
		 * the fragment that appears in the code: "misc device register failed, err = %d\n"@0xffffff80091c15aa
		 * (<<mov w2,#0x6a1>>@0xffffff800878e838)
		 */
#line 1697
		MAGN_ERR("misc device register failed, err = %d\n", err);
		goto exit_misc_device_register_failed;
	}

	/* <<strb wzr,[sp,#96]>>@0xffffff800878eb88 = ctl+48 */
	ctl.is_report_input_direct = false;
	/*
	 * is_use_common_factory (ctl+50) is NOT assigned: it stays zero from the
	 * initial clearing of the structure. A `ctl.is_use_common_factory = false;`
	 * here would produce a `strb wzr` that does not exist anywhere in the
	 * factory function (independent review,
	 * docs/bringup/rapporti/revisione-qmcx983-complete.md, R6): the line had
	 * been written and the comment above it already contradicted it.
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
	/*
	 * a copy of 8 bytes from 0xffffff8008f54800 ("qmcX983") and the clearing of
	 * all the other 56 bytes of the field
	 * ("stur x8,[sp,#100]" ... <<stur xzr,[sp,#140]>>@0xffffff800878eb9c):
	 * that is strncpy semantics, not strcpy
	 */
	strncpy(ctl.libinfo.libname, QMCX983_DEV_NAME,
		sizeof(ctl.libinfo.libname));
	/* <<stp w8,w9,[sp,#164]>>@0xffffff800878eb68 = ctl+116 e ctl+120 */
	ctl.libinfo.layout = data->hw.direction;
	ctl.libinfo.deviceid = chip_id;

	err = mag_register_control_path(&ctl);
	if (err) {
		/*
		 * "register mag control path err\n"@0xffffff80091c15d1: without a
		 * KERN_* prefix, hence a bare printk and not pr_err
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
	/*
	 * <<\x016[QMC-Msensor] %s: OK\n>>@0xffffff80091c1610
	 * the fragment that appears in the code: "%s: OK\n"@0xffffff80091c1620
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
	/*
 * <<\x013[QMC-Msensor] %s %d : %s: err = %d\n>>@0xffffff80091c1628,
 * the fragment that appears in the code: "%s: err = %d\n"@0xffffff80091c1640
 * line 1746 (<<mov w2,#0x6d2>>@0xffffff800878e988)
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
		/*
		 * "\x013[QMC-Msensor] %s %d : qmcX983_delete_attr fail: %d\n"
		 * @0xffffff80091c1e10, line 1761
		 * the fragment that appears in the code: "qmcX983_delete_attr fail: %d\n"@0xffffff80091c1e28
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
 * qmcX983_local_init: 0xffffff800878e008, 128 bytes, t.
 * Both error branches return -EINVAL
 * (<<mov w0,#0xffffffea>>@0xffffff800878e07c), not -1.
 */
static int qmcX983_local_init(void)
{
	/* <<str wzr,[x8,#624]>>@0xffffff800878e020, before i2c_add_driver */
	atomic_set(&open_count, 0);

	if (i2c_add_driver(&qmcX983_i2c_driver)) {
		/*
		 * "\x013[QMC-Msensor] %s %d : add driver error\n"
		 * @0xffffff80091c13af, line 1782
		 * the fragment that appears in the code: "add driver error\n"@0xffffff80091c13c7
		 * (<<mov w2,#0x6f6>>@0xffffff800878e03c)
		 */
#line 1782
		MAGN_ERR("add driver error\n");
		return -EINVAL;
	}
	if (!qmcX983_init_flag) {
		/*
 * <<\x013[QMC-Msensor] %s %d : %s failed!\n>>@0xffffff80091c13ec,
 * the fragment that appears in the code: "%s failed!\n"@0xffffff80091c1404
 * line 1788 (<<mov w2,#0x6fc>>@0xffffff800878e070); the third
 * argument is __func__ again (<<mov x3,x1>>@0xffffff800878e074)
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
 * qmcX983_init: 0xffffff8009374654, 32 bytes, t, in .init.text.
 *   bl mag_driver_add(&qmcX983_init_info) ; mov w0, wzr ; ret
 * The value returned by mag_driver_add is discarded.
 */
static int __init qmcX983_init(void)
{
	mag_driver_add(&qmcX983_init_info);
	return 0;
}

module_init(qmcX983_init);

/*
 * qmcX983_exit() was reconstructed from the factory kernel disassembly (0xffffff80093aa5ec, 4 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_magnetometer_qmcx983_qmcX983.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void __exit qmcX983_exit(void)
{
}

module_exit(qmcX983_exit);

MODULE_DESCRIPTION("QST QMC7983 magnetometer (Doogee S88 Pro, dal disassemblato)");
MODULE_LICENSE("GPL");
/*
 * MODULE_VERSION() was reconstructed from the factory kernel disassembly (0xffffff800992f0f0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_sensors-1.0_magnetometer_qmcx983_qmcX983.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
MODULE_VERSION("driver version 3.4");
