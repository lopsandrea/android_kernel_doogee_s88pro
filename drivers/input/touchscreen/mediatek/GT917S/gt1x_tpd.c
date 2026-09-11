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

/*
 * gt1x_suspend/gt1x_resume live in gt1x_generic.c (0xffffff8008a74cdc and
 * 0xffffff8008a74fb8) and include/gt1x_tpd_common.h does not declare them.  The
 * prototype is HERE and not in the header because the header is shared between
 * the six units and this batch does not touch it: the entry is reported as a
 * header delta to the merge batch.  No effect on the binary -- it is a
 * declaration.
 */
extern s32 gt1x_suspend(void);
extern s32 gt1x_resume(void);

/*
 * g0fc0b8 -- the same byte at 0xffffff800a0fc0b8 that gt1x_generic.c declares
 * and documents (see its header): the binary does not name it and the name
 * comes from the address.  It is needed here because tpd_event_handler reads
 * it -- "3942e108 ldrb"@0xffffff8008a78028 -- in the same form as the other
 * five reads.  This declaration too is a header delta.
 */
extern u8 g0fc0b8;

/*
 * fix_tp_proc_info() was reconstructed from the factory kernel disassembly (0xffffff8008a5151c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
extern void fix_tp_proc_info(char *buf, int len);

/*1 enable,0 disable,touch_panel_eint default status, */
/* need to confirm after register eint*/
/*
 * `static` AND WITHOUT AN INITIALISER, both measured.  In the factory build
 * irq_flag is read as a byte and tested bitwise --
 *   "3969c109 ldrb"@0xffffff8008a76e08 / "370000c9 tbnz"@0xffffff8008a76e10
 *   "3929c109 strb"@0xffffff8008a76e20 / "3929c11f strb"@0xffffff8008a76e78
 * -- which is the narrowed form clang uses only when the variable is
 * `static` and takes just two values across the whole unit.  ALPS declares it
 * global (`int irq_flag = 1;`).  And the initial value is ZERO, not one:
 * the address 0xffffff800a100a70 falls in .bss, which starts at
 * 0xffffff8009a5b80c, whereas a non-zero initialiser would put it
 * in .data.  With `static int irq_flag = 1;` gt1x_irq_enable
 * still measures 88 bytes but the ldrb/tbnz pair comes out reversed.
 */
static int irq_flag;
static spinlock_t irq_flag_lock;
/*0 power off,default, 1 power on*/
static int power_flag;
static int tpd_flag;
/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int tpd_halt;
static int tpd_eint_mode = 1;
static struct task_struct *thread;
static struct task_struct *probe_thread;
static int tpd_polling_time = 50;
static DECLARE_WAIT_QUEUE_HEAD(waiter);
DECLARE_WAIT_QUEUE_HEAD(init_waiter);
DEFINE_MUTEX(i2c_access);
/*
 * touch_irq: in the factory build it sits at 0xffffff800a100a74, that is IN
 * THE MIDDLE of the object's local symbols (irq_flag_lock 0xa6c, irq_flag
 * 0xa70, power_flag 0xa78, tpd_eint_mode 0xa7c, tpd_flag 0xa80), while clang
 * places globals BEFORE locals -- in our object touch_irq, tpd_halt and
 * int_type are at 0, 4, 8 and the locals start at 0xc.  It is a hint that in
 * the factory build it is `static`, but it does NOT change a byte of code
 * (the reads stay `ldr w` either way, and the offset is a relocation site):
 * reported, not applied.
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

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a776b0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998a468, 40 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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

/*
 * "mediatek,cap_touch"@0xffffff8008f7ead8 -- WEAK: the bytes occur THREE
 * times in the image (0xffffff8008f7ead8, 0xffffff8008f7f1c0,
 * 0xffffff8008f7f4a0), because `compatible` is a field inside the struct and
 * not a pointer, and every touch driver has a copy.  On its own the citation
 * does not prove which of the three this is.
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

/*
 * tpd_i2c_detect() was reconstructed from the factory kernel disassembly (0xffffff8008a77acc).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int tpd_i2c_detect(struct i2c_client *client,
			  struct i2c_board_info *info)
{
	strcpy(info->type, "mtk-tpd");
	return 0;
}

/*
 * tpd_power_on() was reconstructed from the factory kernel disassembly (0xffffff8008a741ac).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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

/*
 * gt1x_power_switch() was reconstructed from the factory kernel disassembly (0xffffff8008a76fac).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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


/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a7221c, 12 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * tpd_irq_registration() was reconstructed from the factory kernel disassembly (0xffffff8008a77cf0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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

/*
 * tpd_registration() was reconstructed from the factory kernel disassembly (0xffffff8008a77b0c, 992 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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

/*
 * tpd_i2c_probe() was reconstructed from the factory kernel disassembly (0xffffff8008a5151c).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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

/*
 * tpd_eint_interrupt_handler() was reconstructed from the factory kernel disassembly (0xffffff8008a78354).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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

/*
 * gt1x_touch_down() was reconstructed from the factory kernel disassembly (0xffffff8008a529d8, 320 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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

/*
 * gt1x_touch_up raises BTN_TOUCH too, which ALPS does not touch here:
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

/*
 * tpd_event_handler() was reconstructed from the factory kernel disassembly (0xffffff8008a77f9c, 1104 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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

/*
 * gt1x_debug_proc() was reconstructed from the factory kernel disassembly (0xffffff8008a771d4, 536 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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

/*
 * tpd_local_init() was reconstructed from the factory kernel disassembly (0xffffff8008a775f4).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
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

/*
 * tpd_suspend() was reconstructed from the factory kernel disassembly (0xffffff8008a777d0).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void tpd_suspend(struct device *h)
{
	gt1x_suspend();
}

static void tpd_resume(struct device *h)
{
	gt1x_resume();
}

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff800998a578).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_GT917S_gt1x_tpd.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
/*
 * IT IS CALLED `gt1x_tpd_driver` AND IT IS NOT `static`, and gt1x_wtk.c says
 * so: that file -- which is the Wingtech addition and lives in another unit --
 * registers and removes it under that name (tpd_driver_add/&gt1x_tpd_driver at
 * line 1392, tpd_driver_remove at 859).  The GT5688 copy called it
 * `tpd_device_driver` and kept it static because there the same file registers
 * it.
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
	/*
	 * with the leading space: the assembled message has TWO spaces after the
	 * bracket --
	 * "<<GTP-INF>>[%s:%d]  Goodix touch panel driver init.\n"@0xffffff800924c5db
	 */
	GTP_INFO(" Goodix touch panel driver init.");
	tpd_get_dts_info();
	/*
	 * "tpd_driver_init"@0xffffff800924c610 is the __func__ of both messages; the
	 * second one is
	 * "<<GTP-INF>>[%s:%d] add generic driver failed\n\n"@0xffffff800924c620
	 * and the branch leading to it is "36f800e0 tbz"@0xffffff8009383448, that is
	 * `< 0`.  This function lives in .init.text, OUTSIDE the measured block
	 * [0xffffff8008a76cf4, 0xffffff8008a7839c): the map puts it at
	 * 0xffffff8009383418 and measures 88 bytes, the same as ours.
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
