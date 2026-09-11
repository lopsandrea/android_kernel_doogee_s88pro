// SPDX-License-Identifier: GPL-2.0
/*
 * FocalTech FT8719 touch panel, Doogee S88 Pro (MT6771) -- the I2C bus
 * translation unit, and that alone. Declared partial batch.
 *
 * Reconstructed from the disassembly of the factory kernel. No public source
 * was read: the public FocalTech drivers in the ALPS tree
 * (focaltech_fhd_touch/, focaltech_touch/) were not opened.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mutex.h>

/*
 * FTS_DEBUG() was reconstructed from the factory kernel disassembly (0xffffff800924f634).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define FTS_DEBUG(fmt, args...)		printk("[FTS]" fmt "\n", ##args)
#define FTS_ERROR(fmt, args...)		printk(KERN_ERR "[FTS][Error]" fmt "\n", ##args)
#define FTS_FUNC_ENTER()		printk("[FTS]%s: Enter\n", __func__)
#define FTS_FUNC_EXIT()			printk("[FTS]%s: Exit(%d)\n", __func__, __LINE__)

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a80338).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
#define I2C_RETRY_NUMBER		3

/*
 * DEFINE_MUTEX() was reconstructed from the factory kernel disassembly (0xffffff800998cbe0, 32 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static DEFINE_MUTEX(fts_g_998cbe0);

/*
 * fts_i2c_read() was reconstructed from the factory kernel disassembly (0xffffff8008a802ac, 488 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_i2c_read(struct i2c_client *client, char *writebuf, int writelen,
		 char *readbuf, int readlen)
{
	int ret = 0;
	int i = 0;

	if (client == NULL) {
		FTS_ERROR("[IIC][%s]i2c_client==NULL!", __func__);
		return -EINVAL;
	}

	mutex_lock(&fts_g_998cbe0);
	if (readlen > 0) {
		if (writelen > 0) {
			struct i2c_msg msgs[] = {
				{
					.addr = client->addr,
					.flags = 0,
					.len = writelen,
					.buf = writebuf,
				},
				{
					.addr = client->addr,
					.flags = I2C_M_RD,
					.len = readlen,
					.buf = readbuf,
				},
			};
			for (i = 0; i < I2C_RETRY_NUMBER; i++) {
				ret = i2c_transfer(client->adapter, msgs, 2);
				if (ret < 0)
					FTS_ERROR("[IIC]: i2c_transfer(write) error, ret=%d!!", ret);
				else
					break;
			}
		} else {
			struct i2c_msg msgs[] = {
				{
					.addr = client->addr,
					.flags = I2C_M_RD,
					.len = readlen,
					.buf = readbuf,
				},
			};
			for (i = 0; i < I2C_RETRY_NUMBER; i++) {
				ret = i2c_transfer(client->adapter, msgs, 1);
				if (ret < 0)
					FTS_ERROR("[IIC]: i2c_transfer(read) error, ret=%d!!", ret);
				else
					break;
			}
		}
	}
	mutex_unlock(&fts_g_998cbe0);

	return ret;
}

/*
 * fts_i2c_write() was reconstructed from the factory kernel disassembly (0xffffff8008a80494, 304 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_i2c_write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret = 0;
	int i = 0;

	if (client == NULL) {
		FTS_ERROR("[IIC][%s]i2c_client==NULL!", __func__);
		return -EINVAL;
	}

	mutex_lock(&fts_g_998cbe0);
	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
				.addr = client->addr,
				.flags = 0,
				.len = writelen,
				.buf = writebuf,
			},
		};
		for (i = 0; i < I2C_RETRY_NUMBER; i++) {
			ret = i2c_transfer(client->adapter, msgs, 1);
			if (ret < 0)
				FTS_ERROR("[IIC]: i2c_transfer(write) error, ret=%d!!", ret);
			else
				break;
		}
	}
	mutex_unlock(&fts_g_998cbe0);

	return ret;
}

/*
 * fts_i2c_write_reg() was reconstructed from the factory kernel disassembly (0xffffff8008a805c4, 80 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_i2c_write_reg(struct i2c_client *client, u8 regaddr, u8 regvalue)
{
	u8 buf[2] = { 0 };

	buf[0] = regaddr;
	buf[1] = regvalue;

	return fts_i2c_write(client, buf, sizeof(buf));
}

/*
 * fts_i2c_read_reg() was reconstructed from the factory kernel disassembly (0xffffff8008a80614, 84 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_i2c_read_reg(struct i2c_client *client, u8 regaddr, u8 *regvalue)
{
	return fts_i2c_read(client, &regaddr, 1, regvalue, 1);
}

/*
 * fts_i2c_hid2std() was reconstructed from the factory kernel disassembly (0xffffff8008a80668, 232 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void fts_i2c_hid2std(struct i2c_client *client)
{
	int ret = 0;
	u8 buf[3] = { 0xeb, 0xaa, 0x09 };

	ret = fts_i2c_write(client, buf, 3);
	if (ret < 0) {
		FTS_ERROR("hid2std cmd write fail");
	} else {
		msleep(10);
		buf[0] = buf[1] = buf[2] = 0;
		ret = fts_i2c_read(client, NULL, 0, buf, 3);
		if (ret < 0) {
			FTS_ERROR("hid2std cmd read fail");
		} else if ((buf[0] == 0xeb) && (buf[1] == 0xaa) && (buf[2] == 0x08)) {
			FTS_DEBUG("hidi2c change to stdi2c successful");
		} else {
			FTS_ERROR("hidi2c change to stdi2c fail");
		}
	}
}

/*
 * fts_i2c_init() was reconstructed from the factory kernel disassembly (0xffffff8008a80750, 72 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int fts_i2c_init(void)
{
	FTS_FUNC_ENTER();
	FTS_FUNC_EXIT();

	return 0;
}

int fts_i2c_exit(void)
{
	FTS_FUNC_ENTER();
	FTS_FUNC_EXIT();

	return 0;
}

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ft8719_focaltech_i2c.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
