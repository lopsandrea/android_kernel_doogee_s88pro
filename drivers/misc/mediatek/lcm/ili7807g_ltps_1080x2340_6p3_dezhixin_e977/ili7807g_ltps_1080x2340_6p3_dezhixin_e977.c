/*
 * Panel ili7807g_ltps_1080x2340_6p3_dezhixin_e977
 *
 * Reconstructed for the Doogee S88 Pro. Doogee never published the sources and
 * this panel exists in no public tree: the name encodes the module integrator
 * (dezhixin) and the Wingtech ODM project (e977).
 *
 * The initialisation sequence below -- 202 entries -- was extracted from the
 * factory bootloader with tools/lcmextract.py. The DSI parameters come from
 * LK's log on the phone:
 *
 *   [DISPCHECK][LCM] DSI Mode: SYNC_EVENT_VDO_MODE
 *   [DISPCHECK][LCM] LANE_NUM: 4, data_format:(0,2,0,0)
 *   [DISPCHECK][LCM] vact: 2, vbp: 6, vfp: 38, vact_line: 2340,
 *                    hact: 5, hbp: 53, hfp: 53, hactive: 1080
 *   [DISPCHECK][LCM] PLL_CLOCK: 560, ssc_disable: 0, cont_clock: 0
 *   [LK_DDP/DSI]     PLL config: data_rate=1120
 *
 * and the exact name from the running factory driver, read in
 * /sys/kernel/debug/mtkfb:
 *
 *   LCM Driver=[ili7807g_ltps_1080x2340_6p3_dezhixin_e977], Resolution=1080x2340, Interface:DSI, LCM Connected:Y
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#define LOG_TAG "LCM"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"
#include "../../pmic/mt6370/inc/mt6370_pmu.h"

extern struct mt6370_pmu_chip *g9b2c548;
extern unsigned char g0fc0b8;
extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int *rawdata);

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#define LCM_LOGI(string, args...)  dprintf(0, "[LK/"LOG_TAG"]"string, ##args)
#else
#include <linux/kernel.h>
#include <linux/module.h>
#define LCM_LOGI(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif

static struct LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)	(lcm_util.set_reset_pin((v)))
#define MDELAY(n)		(lcm_util.mdelay(n))
#define UDELAY(n)		(lcm_util.udelay(n))
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
	lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define read_reg_v2(cmd, buffer, buffer_size) \
	lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

/* Resolution, and physical size of the 6.3 inch 19.5:9 screen */
#define FRAME_WIDTH		(1080)
#define FRAME_HEIGHT		(2340)
#define LCM_PHYSICAL_WIDTH	(67000)
#define LCM_PHYSICAL_HEIGHT	(145300)
#define LCM_DENSITY		(480)

#define REGFLAG_DELAY		0xFFE
#define REGFLAG_END_OF_TABLE	0xFFF

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xFF, 3, {0x78, 0x07, 0x01} },
	{0x00, 1, {0x41} },
	{0x01, 1, {0x50} },
	{0x02, 1, {0x08} },
	{0x03, 1, {0x51} },
	{0x08, 1, {0x81} },
	{0x09, 1, {0x02} },
	{0x0A, 1, {0x30} },
	{0x0C, 1, {0x08} },
	{0x0E, 1, {0x51} },
	{0x31, 1, {0x07} },
	{0x32, 1, {0x07} },
	{0x33, 1, {0x07} },
	{0x34, 1, {0x07} },
	{0x35, 1, {0x07} },
	{0x36, 1, {0x02} },
	{0x37, 1, {0x2E} },
	{0x38, 1, {0x2F} },
	{0x39, 1, {0x30} },
	{0x3A, 1, {0x34} },
	{0x3B, 1, {0x35} },
	{0x3C, 1, {0x36} },
	{0x3D, 1, {0x07} },
	{0x3E, 1, {0x07} },
	{0x3F, 1, {0x07} },
	{0x40, 1, {0x2C} },
	{0x41, 1, {0x28} },
	{0x42, 1, {0x13} },
	{0x43, 1, {0x11} },
	{0x44, 1, {0x07} },
	{0x45, 1, {0x07} },
	{0x46, 1, {0x07} },
	{0x47, 1, {0x07} },
	{0x48, 1, {0x09} },
	{0x49, 1, {0x07} },
	{0x4A, 1, {0x07} },
	{0x4B, 1, {0x07} },
	{0x4C, 1, {0x07} },
	{0x4D, 1, {0x07} },
	{0x4E, 1, {0x02} },
	{0x4F, 1, {0x2E} },
	{0x50, 1, {0x2F} },
	{0x51, 1, {0x30} },
	{0x52, 1, {0x34} },
	{0x53, 1, {0x35} },
	{0x54, 1, {0x36} },
	{0x55, 1, {0x07} },
	{0x56, 1, {0x07} },
	{0x57, 1, {0x07} },
	{0x58, 1, {0x2C} },
	{0x59, 1, {0x28} },
	{0x5A, 1, {0x12} },
	{0x5B, 1, {0x10} },
	{0x5C, 1, {0x07} },
	{0x5D, 1, {0x07} },
	{0x5E, 1, {0x07} },
	{0x5F, 1, {0x07} },
	{0x60, 1, {0x08} },
	{0xA7, 1, {0x10} },
	{0xB2, 1, {0x00} },
	{0xD1, 1, {0x12} },
	{0xD3, 1, {0x40} },
	{0xD4, 1, {0x04} },
	{0xD8, 1, {0x64} },
	{0xE6, 1, {0x22} },
	{0xFF, 3, {0x78, 0x07, 0x02} },
	{0x01, 1, {0x35} },
	{0x06, 1, {0x70} },
	{0x07, 1, {0x70} },
	{0x78, 1, {0x26} },
	{0x79, 1, {0x26} },
	{0x40, 1, {0x08} },
	{0x41, 1, {0x00} },
	{0x42, 1, {0x04} },
	{0x43, 1, {0x17} },
	{0x47, 1, {0x00} },
	{0x53, 1, {0x04} },
	{0x5F, 1, {0x36} },
	{0xFF, 3, {0x78, 0x07, 0x05} },
	{0x27, 1, {0x44} },
	{0x28, 1, {0x54} },
	{0x2B, 1, {0x08} },
	{0xA0, 1, {0x44} },
	{0xA2, 1, {0x54} },
	{0xBC, 1, {0x54} },
	{0x03, 1, {0x00} },
	{0x04, 1, {0x8B} },
	{0x63, 1, {0x79} },
	{0x64, 1, {0x79} },
	{0x68, 1, {0x70} },
	{0x69, 1, {0x89} },
	{0x6A, 1, {0x40} },
	{0x6B, 1, {0x50} },
	{0xFF, 3, {0x78, 0x07, 0x06} },
	{0xD6, 1, {0x67} },
	{0x2E, 1, {0x01} },
	{0xC0, 1, {0x91} },
	{0xC1, 1, {0x04} },
	{0xC3, 1, {0x0E} },
	{0x11, 1, {0x03} },
	{0x12, 1, {0x10} },
	{0x13, 1, {0x54} },
	{0x14, 1, {0x41} },
	{0x15, 1, {0x00} },
	{0x16, 1, {0x41} },
	{0x17, 1, {0x47} },
	{0x18, 1, {0x3A} },
	{0xB4, 1, {0xD4} },
	{0xB5, 1, {0x08} },
	{0xFF, 3, {0x78, 0x07, 0x07} },
	{0x06, 1, {0x90} },
	{0xFF, 3, {0x78, 0x07, 0x08} },
	{0xE0, 40, {0x00, 0x00, 0x1C, 0x49, 0x00, 0x6B, 0x88, 0xA0, 0x00, 0xB5, 0xC8, 0xD9, 0x15, 0x0F, 0x39, 0x78, 0x25, 0xA7, 0xF0, 0x2B, 0x2A, 0x2C, 0x65, 0xA6, 0x3E, 0xCE, 0x02, 0x23, 0x3F, 0x4F, 0x5C, 0x6A, 0x3F, 0x7A, 0x8D, 0xA2, 0x3F, 0xBD, 0xDF, 0xE6} },
	{0xE1, 40, {0x00, 0x00, 0x1C, 0x49, 0x00, 0x6B, 0x88, 0xA0, 0x00, 0xB5, 0xC8, 0xD9, 0x15, 0x0F, 0x39, 0x78, 0x25, 0xA7, 0xF0, 0x2B, 0x2A, 0x2C, 0x65, 0xA6, 0x3E, 0xCE, 0x02, 0x23, 0x3F, 0x4F, 0x5C, 0x6A, 0x3F, 0x7A, 0x8D, 0xA2, 0x3F, 0xBD, 0xDF, 0xE6} },
	{0xFF, 3, {0x78, 0x07, 0x09} },
	{0xE0, 40, {0x00, 0x00, 0x1E, 0x4D, 0x00, 0x70, 0x8D, 0xA5, 0x00, 0xBB, 0xCD, 0xDE, 0x15, 0x14, 0x3E, 0x7C, 0x25, 0xAB, 0xF4, 0x2D, 0x2A, 0x2F, 0x67, 0xA7, 0x3E, 0xCF, 0x03, 0x25, 0x3F, 0x50, 0x5D, 0x6C, 0x3F, 0x7D, 0x8F, 0xA5, 0x3F, 0xC0, 0xE1, 0xE6} },
	{0xE1, 40, {0x00, 0x00, 0x1E, 0x4D, 0x00, 0x70, 0x8D, 0xA5, 0x00, 0xBB, 0xCD, 0xDE, 0x15, 0x14, 0x3E, 0x7C, 0x25, 0xAB, 0xF4, 0x2D, 0x2A, 0x2F, 0x67, 0xA7, 0x3E, 0xCF, 0x03, 0x25, 0x3F, 0x50, 0x5D, 0x6C, 0x3F, 0x7D, 0x8F, 0xA5, 0x3F, 0xC0, 0xE1, 0xE6} },
	{0xFF, 3, {0x78, 0x07, 0x0A} },
	{0xE0, 40, {0x00, 0x00, 0x25, 0x59, 0x00, 0x7D, 0x9A, 0xB2, 0x00, 0xC7, 0xDA, 0xEA, 0x15, 0x1E, 0x47, 0x83, 0x25, 0xB1, 0xF7, 0x31, 0x2A, 0x33, 0x69, 0xA8, 0x3E, 0xD0, 0x04, 0x26, 0x3F, 0x51, 0x5E, 0x6D, 0x3F, 0x7D, 0x90, 0xA5, 0x3F, 0xC0, 0xDF, 0xE6} },
	{0xE1, 40, {0x00, 0x00, 0x25, 0x59, 0x00, 0x7D, 0x9A, 0xB2, 0x00, 0xC7, 0xDA, 0xEA, 0x15, 0x1E, 0x47, 0x83, 0x25, 0xB1, 0xF7, 0x31, 0x2A, 0x33, 0x69, 0xA8, 0x3E, 0xD0, 0x04, 0x26, 0x3F, 0x51, 0x5E, 0x6D, 0x3F, 0x7D, 0x90, 0xA5, 0x3F, 0xC0, 0xDF, 0xE6} },
	{0xFF, 3, {0x78, 0x07, 0x0E} },
	{0x00, 1, {0xA1} },
	{0x4D, 1, {0x5F} },
	{0x41, 1, {0x08} },
	{0x43, 1, {0xB3} },
	{0x47, 1, {0x80} },
	{0x49, 1, {0xC3} },
	{0xB0, 1, {0x31} },
	{0xB1, 1, {0x60} },
	{0xB2, 1, {0x60} },
	{0xB3, 1, {0x00} },
	{0xB4, 1, {0x33} },
	{0x45, 1, {0x0A} },
	{0x46, 1, {0x61} },
	{0xBC, 1, {0x04} },
	{0xBD, 1, {0xFC} },
	{0xC0, 1, {0x34} },
	{0xC6, 1, {0x60} },
	{0xC7, 1, {0x60} },
	{0xC8, 1, {0x60} },
	{0xC9, 1, {0x60} },
	{0xE0, 1, {0x08} },
	{0xE1, 1, {0x00} },
	{0xE2, 1, {0x04} },
	{0xE3, 1, {0x17} },
	{0xE4, 1, {0x04} },
	{0xE5, 1, {0x04} },
	{0xE6, 1, {0x00} },
	{0xFF, 3, {0x78, 0x07, 0x0E} },
	{0x07, 1, {0x21} },
	{0x4B, 1, {0x14} },
	{0xFF, 3, {0x78, 0x07, 0x0C} },
	{0x00, 1, {0x19} },
	{0x01, 1, {0x2A} },
	{0x02, 1, {0x17} },
	{0x03, 1, {0x1C} },
	{0x04, 1, {0x17} },
	{0x05, 1, {0x19} },
	{0x06, 1, {0x17} },
	{0x07, 1, {0x16} },
	{0x08, 1, {0x18} },
	{0x09, 1, {0x1E} },
	{0x0A, 1, {0x19} },
	{0x0B, 1, {0x29} },
	{0x0C, 1, {0x17} },
	{0x0D, 1, {0x18} },
	{0x0E, 1, {0x19} },
	{0x0F, 1, {0x27} },
	{0x10, 1, {0x18} },
	{0x11, 1, {0x1D} },
	{0x12, 1, {0x18} },
	{0x13, 1, {0x20} },
	{0x14, 1, {0x19} },
	{0x15, 1, {0x25} },
	{0x16, 1, {0x17} },
	{0x17, 1, {0x1A} },
	{0x18, 1, {0x19} },
	{0x19, 1, {0x28} },
	{0x1A, 1, {0x17} },
	{0x1B, 1, {0x1B} },
	{0x1C, 1, {0x18} },
	{0x1D, 1, {0x21} },
	{0x1E, 1, {0x19} },
	{0x1F, 1, {0x26} },
	{0x20, 1, {0x18} },
	{0x21, 1, {0x24} },
	{0x22, 1, {0x18} },
	{0x23, 1, {0x1F} },
	{0x24, 1, {0x17} },
	{0x25, 1, {0x17} },
	{0x26, 1, {0x18} },
	{0x27, 1, {0x23} },
	{0x28, 1, {0x18} },
	{0x29, 1, {0x22} },
	{0xFF, 3, {0x78, 0x07, 0x04} },
	{0xB7, 1, {0xF0} },
	{0xFF, 3, {0x78, 0x07, 0x00} },
	{0x11, 0, {} },
	{REGFLAG_DELAY, 150, {} },
	{0x29, 0, {} },
	{REGFLAG_DELAY, 20, {} },
	{REGFLAG_END_OF_TABLE, 0x00, {} },
};


/*
 * THE SUSPEND TABLE, read at 0xffffff8009942758 and following (stride 72,
 * five entries): {0x28, 0}, {delay 20}, {0x10, 0}, {delay 120}, end.
 * lcm_suspend walks it with push_table and the compiler unrolls it entirely.
 */
static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x28, 0, {} },
	{REGFLAG_DELAY, 20, {} },
	{0x10, 0, {} },
	{REGFLAG_DELAY, 120, {} },
	{REGFLAG_END_OF_TABLE, 0, {} },
};

static void push_table(struct LCM_setting_table *table, unsigned int count,
		       unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		unsigned int cmd = table[i].cmd;

		switch (cmd) {
		case REGFLAG_DELAY:
			if (table[i].count <= 10)
				MDELAY(table[i].count);
			else
				MDELAY(table[i].count);
			break;
		case REGFLAG_END_OF_TABLE:
			break;
		default:
			dsi_set_cmdq_V2(cmd, table[i].count,
					table[i].para_list, force_update);
		}
	}
}

static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(struct LCM_UTIL_FUNCS));
}

static void lcm_get_params(struct LCM_PARAMS *params)
{

	memset(params, 0, sizeof(struct LCM_PARAMS));

	params->type = LCM_TYPE_DSI;
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	/*
	 * ONLY the two fields in millimetres, and the height is 146 -- not 145 as
	 * 145300/1000 would give. The factory writes 0x00000092_00000043 at
	 * +1172 ("f900014b str"@0xffffff80087fc71c on x19+0x494), that is 67 and
	 * 146, and does NOT touch physical_width_um (+1180), physical_height_um
	 * (+1184) or density (+40): the initial memset leaves them at zero.
	 * The offsets are asked of the compiler, not inferred.
	 */
	params->physical_width = 67;
	params->physical_height = 146;

	params->dsi.mode = SYNC_EVENT_VDO_MODE;
	params->dsi.LANE_NUM = LCM_FOUR_LANE;

	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	/*
	 * RGB666, and now it is measured: "b9020a68 str"@0xffffff80087fc72c
	 * writes 2 at +520, which is dsi.data_format.format (508 + 12, with the
	 * four fields four bytes each). The earlier note said "to be tried on its
	 * own" because it had been changed together with three other things and the
	 * phone would not boot: those other three were the result of a driver that
	 * was not the factory one.
	 */
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB666;

	params->dsi.packet_size = 256;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	/*
	 * Bytes per row: 1080 pixels x 3. Left at zero the DSI packs rows of the
	 * wrong length, and that is what we saw -- black and white stripes on an
	 * otherwise lit and recognised screen.
	 */
	params->dsi.word_count = FRAME_WIDTH * 3;

	params->dsi.vertical_sync_active = 2;
	params->dsi.vertical_backporch = 6;
	params->dsi.vertical_frontporch = 38;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 5;
	params->dsi.horizontal_backporch = 53;
	params->dsi.horizontal_frontporch = 53;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	params->dsi.PLL_CLOCK = 560;
	params->dsi.ssc_disable = 0;
	params->dsi.cont_clock = 0;

	/*
	 * This section was reconstructed from the factory kernel disassembly (0xffffff80087fc798).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_lcm_ili7807g_ltps_1080x2340_6p3_dezhixin_e977_ili7807g_ltps_1080x2340_6p3_dezhixin_e977.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0a;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;
}


static void lcm_init(void)
{
	int ret;

	/*
	 * THE THREE WRITES TO THE PMIC, which ALPS did not have: they switch on VPOS and VNEG
	 * of the panel bias. They are the first things the factory lcm_init
	 * does -- "97f737f9 bl"@0xffffff80087fc7d8 is the first of the three
	 * mt6370_pmu_reg_write, with registers 0xb3, 0xb4 and 0xb1 and values 0x24,
	 * 0x24 and 0x48.
	 *
	 * The error message of the third is the same as the first -- "vpos en
	 * error" and not "veng" -- and it is a factory typo, reproduced:
	 * "91070400 add"@0xffffff80087fc824 points at the same string as
	 * @0xffffff80087fc7e4.
	 */
	ret = mt6370_pmu_reg_write(g9b2c548, 0xb3, 0x24);
	if (ret < 0)
		printk("vpos en error\n");
	ret = mt6370_pmu_reg_write(g9b2c548, 0xb4, 0x24);
	if (ret < 0)
		printk("veng en error\n");
	ret = mt6370_pmu_reg_write(g9b2c548, 0xb1, 0x48);
	if (ret < 0)
		printk("vpos en error\n");

	/* The timings are 10, 10 and 120, not 5, 10 and 20 */
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

	push_table(lcm_initialization_setting,
		   sizeof(lcm_initialization_setting) /
		   sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	int ret;
	unsigned char v;

	/*
	 * This section was reconstructed from the factory kernel disassembly.
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_lcm_ili7807g_ltps_1080x2340_6p3_dezhixin_e977_ili7807g_ltps_1080x2340_6p3_dezhixin_e977.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	if (!lcm_util.dsi_set_cmdq_V2 || !lcm_util.mdelay) {
		pr_notice("%s: lcm_util not populated, skipping panel power-off\n",
			  __func__);
		return;
	}

	push_table(lcm_suspend_setting,
		   sizeof(lcm_suspend_setting) /
		   sizeof(struct LCM_setting_table), 1);

	/*
	 * AND THEN IT TURNS OFF THE BIAS, with a value coming from g0fc0b8 -- the
	 * same byte mtk_tpd.c declares and GT917S uses. The factory reads
	 * "3942e108 ldrb"@0xffffff80087fca6c and composes the value with
	 * "531d7109 lsl"@0xffffff80087fca7c and "2a081922 orr"@0xffffff80087fca80,
	 * that is (v << 3) | (v << 6) -- which is v * 72.
	 */
	/*
	 * This section was reconstructed from the factory kernel disassembly.
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_misc_mediatek_lcm_ili7807g_ltps_1080x2340_6p3_dezhixin_e977_ili7807g_ltps_1080x2340_6p3_dezhixin_e977.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	if (!g9b2c548) {
		pr_notice("%s: mt6370 not ready, skipping bias power-off\n",
			  __func__);
		return;
	}

	v = g0fc0b8;
	ret = mt6370_pmu_reg_write(g9b2c548, 0xb1, (v << 3) | (v << 6));
	if (ret < 0)
		printk("vpos en error\n");
}

static void lcm_resume(void)
{
	lcm_init();
}


/*
 * rgk_lcm_compare_id() was reconstructed from the factory kernel disassembly (0xffffff80087fcabc, 532 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_lcm_ili7807g_ltps_1080x2340_6p3_dezhixin_e977_ili7807g_ltps_1080x2340_6p3_dezhixin_e977.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static unsigned int rgk_lcm_compare_id(void)
{
	int data[4] = {0, 0, 0, 0};
	int rawdata = 0;
	unsigned int data_array[2];
	unsigned char readback[2];
	unsigned int mv, id;
	int ret;

	if (IMM_GetOneChannelValue(2, data, &rawdata) < 0)
		return 0;

	mv = data[0] * 1000 + data[1] * 10;
	printk("[adc_kernel]: lcm_vol= 0x%x\n", mv);
	if (mv > 100)
		return 0;

	ret = mt6370_pmu_reg_write(g9b2c548, 0xb3, 0x24);
	if (ret < 0)
		printk("vpos en error\n");
	ret = mt6370_pmu_reg_write(g9b2c548, 0xb4, 0x24);
	if (ret < 0)
		printk("veng en error\n");
	ret = mt6370_pmu_reg_write(g9b2c548, 0xb1, 0x48);
	if (ret < 0)
		printk("vpos en error\n");

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);

	data_array[0] = 0x00043902;
	data_array[1] = 0x060778ff;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);

	dsi_set_cmdq(data_array, 1, 1);
	read_reg_v2(0xf0, readback, 2);

	dsi_set_cmdq(data_array, 1, 1);
	read_reg_v2(0xf1, readback + 1, 2);

	id = (readback[0] << 8) | readback[1];

	ret = mt6370_pmu_reg_write(g9b2c548, 0xb1, 0);
	if (ret < 0)
		printk("vpos en error\n");

	return id == 0x7807;
}

/*
 * lcm_esd_check @0xffffff80087fccd0, 140 bytes. It sends 0x00023700 and reads
 * register 0x0A back: if the first byte is not 0x9C the panel is gone.
 *   "5286e009 mov"@0xffffff80087fccf0 is 0x3700, completed by
 *   "72a00049 movk"@0xffffff80087fccfc with 0x0002 on top;
 *   "7102711f cmp"@0xffffff80087fcd38 compares with 0x9c and
 *   "1a9f07e0 cset"@0xffffff80087fcd3c returns TRUE when it does NOT match.
 */
static unsigned int lcm_esd_check(void)
{
	unsigned int data_array[1];
	unsigned char buffer[4];

	data_array[0] = 0x00023700;
	dsi_set_cmdq(data_array, 1, 1);
	read_reg_v2(0x0a, buffer, 4);

	return buffer[0] != 0x9c;
}

/*
 * lcm_esd_recover @0xffffff80087fcd5c, 36 bytes. It prints and reinitialises.
 * The string says "ft8719", which is the TOUCH and not the panel: it is a factory
 * typo, and it is the same in all three panel drivers.
 */
static unsigned int lcm_esd_recover(void)
{
	printk("ft8719  lcm_esd_recover\n");
	lcm_init();

	return 1;
}

/*
 * lcm_ata_check() was reconstructed from the factory kernel disassembly (0xffffff80087fcd80, 260 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_misc_mediatek_lcm_ili7807g_ltps_1080x2340_6p3_dezhixin_e977_ili7807g_ltps_1080x2340_6p3_dezhixin_e977.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static unsigned int lcm_ata_check(unsigned char *buffer)
{
	unsigned int data_array[2];
	unsigned char readback[2];
	unsigned int id;

	data_array[0] = 0x00043902;
	data_array[1] = 0x060778ff;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);

	dsi_set_cmdq(data_array, 1, 1);
	read_reg_v2(0xf0, readback, 2);

	dsi_set_cmdq(data_array, 1, 1);
	read_reg_v2(0xf1, readback + 1, 2);

	id = (readback[0] << 8) | readback[1];
	printk("%s, kernel debug:id = 0x%08x\n", "lcm_ata_check", id);

	return id == 0x7807;
}

struct LCM_DRIVER ili7807g_ltps_1080x2340_6p3_dezhixin_e977_lcm_drv = {
	.name = "ili7807g_ltps_1080x2340_6p3_dezhixin_e977",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	/*
	 * NO init_power / suspend_power / resume_power: the factory does not
	 * set them. The relocations of its struct at 0xffffff800993ed60
	 * are at +0, +8, +16, +24, +32, +40, +80, +168, +176 and +192 -- and
	 * slots +48, +56 and +64, which are those three, have none. Our
	 * three empty functions do not even exist in the factory map.
	 */
	.compare_id = rgk_lcm_compare_id,
	.esd_check = lcm_esd_check,
	.esd_recover = lcm_esd_recover,
	.ata_check = lcm_ata_check,
};
