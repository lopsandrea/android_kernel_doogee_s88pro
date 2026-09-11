// SPDX-License-Identifier: GPL-2.0
/*
 * Ilitek TDDI touch panel, Doogee S88 Pro -- group H: the /proc nodes, the
 * ioctl and the netlink channel.
 *
 * Reconstructed from the disassembly of the factory kernel; the factory kernel
 * is a 4.14. No public source was read.
 *
 * The working notes behind this file -- the disassembly citations, the
 * measurements against the factory binary, the batch-by-batch record of how
 * each function was derived -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <net/net_namespace.h>
#include <net/netlink.h>
#include <net/sock.h>

#include "ilitek.h"

/* The block globals. NAMES CHOSEN: the binary does not name them. */
/*
 * 0xffffff800a1006b0, 4-byte elements indexed with ZERO extension
 * ("b8345b24 str"@0xffffff8008a70c34, `[x25, w20, uxtw #2]`). The NUMBER of
 * elements is NOT measured -- the binary never compares the index against
 * any limit -- and is inferred from the distance to the next global
 * (0xffffff800a1006c4 - 0xffffff800a1006b0 = 20 = 5 x 4). It is a CHOICE.
 * The NAME is not chosen, though: it comes from the format at
 * 0xffffff8009249c8c, "rw_reg[%d] = 0x%x\n".
 */
static u32 rw_reg[5];
static int netlink_pid;			/* 0xffffff800a1006c4, 4 byte */
static struct sk_buff *netlink_skb;	/* 0xffffff800a1006c8, 8 byte */
static struct nlmsghdr *netlink_nlh;	/* 0xffffff800a1006d0, 8 byte */
static struct sock *netlink_sk;		/* 0xffffff800a1006d8, 8 byte */
static struct proc_dir_entry *proc_dir;	/* 0xffffff800a1006e0, 8 byte */

/*
 * 0xffffff800a0ff6b0, 4096 bytes. NAME CHOSEN. The SIZE is not chosen:
 * the `copy_to_user` check in `ilitek_proc_debug_switch_read` compares the
 * length with 4096 ("7140041f cmp"@0xffffff8008a6c834) and passes 4096 as the
 * first argument of the WARN ("321403e3 orr"@0xffffff8008a6c8ec), that is,
 * `__builtin_object_size(g_user_buf, 0)` is 4096.
 */
static unsigned char g_user_buf[4096];

/*
 * 0xffffff800a1006e8, TWO bytes: the binary writes it with `strh`
 * ("790dd114 strh"@0xffffff8008a6ddd8) and reads it with `ldrh`
 * ("794dd2d5 ldrh"@0xffffff8008a6dcf4). NAME CHOSEN; the width is not.
 */
static u16 ioctl_len;

/*
 * The two opaque pointers `idev->c48` and `idev->c56`, read at constant
 * offsets by `ilitek_node_ioctl`. `ilitek.h` keeps them `void *` and this
 * batch does not change that: the offsets are measured at the cited sites,
 * case by case.
 */
#define IDEV_C48_AT(off)	(*(u32 *)((u8 *)idev->c48 + (off)))
#define IDEV_C56_AT(off)	(*(u32 *)((u8 *)idev->c56 + (off)))

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a718d0, 16 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_dbl {
	u8 c0;
	void *c8;
};

/*
 * The factory's form of free: the ADDRESS of the pointer is computed once
 * and survives the `kfree` ("f8408ec0 ldr"@0xffffff8008a71918, pre-indexed
 * with writeback, and "f90002df str"@0xffffff8008a71924 which reuses the same
 * register). Writing `if (p->field) { kfree(p->field); p->field = NULL; }`
 * instead makes the global be re-read after the call, at a cost of one
 * instruction per site.
 */
static inline void ipio_kfree(void **mem)
{
	if (*mem != NULL) {
		kfree(*mem);
		*mem = NULL;
	}
}

#define IDEV_DBL	((struct ilitek_dbl *)idev->c592)

/*
 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a70e94, 152 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
struct ilitek_file_buffer {
	char *c0;
	char c8[128];
	int c136;
	int c140;
	int c144;
};

static void file_write(struct ilitek_file_buffer *file, bool new_open);
static int debug_mode_get_data(struct ilitek_file_buffer *file, u8 type,
			       u32 frame);

/*
 * +568 of `struct ilitek_tddi_dev`: a `wait_queue_head_t` that `ilitek.h`
 * does NOT declare (between `c564` and `c592` the header has a hole).
 * Measured here: "9108e100 add"@0xffffff8008a71d64 passes `idev + 0x238` to
 * `prepare_to_wait_event` with state 1 (TASK_INTERRUPTIBLE), and
 * "9108e100 add"@0xffffff8008a71d9c passes it to `finish_wait`.
 * See the header delta.
 */
#define IDEV_INQ	(*(wait_queue_head_t *)((char *)idev + 568))

static int ilitek_debug_node_buff_control(bool enable);

static ssize_t ilitek_node_ioctl_write(struct file *f, const char __user *buf,
				       size_t size, loff_t *pos);

static long ilitek_node_compat_ioctl(struct file *f, unsigned int cmd,
				     unsigned long arg);

static long ilitek_node_ioctl(struct file *f, unsigned int cmd,
			      unsigned long arg);
static ssize_t ilitek_node_mp_lcm_on_test_read(struct file *f, char __user *buf,
					       size_t size, loff_t *pos);
static ssize_t ilitek_node_mp_lcm_off_test_read(struct file *f,
						char __user *buf,
						size_t size, loff_t *pos);
static ssize_t ilitek_proc_debug_message_read(struct file *f, char __user *buf,
					      size_t size, loff_t *pos);
static ssize_t ilitek_proc_debug_switch_read(struct file *f, char __user *buf,
					     size_t size, loff_t *pos);
static ssize_t ilitek_node_fw_upgrade_read(struct file *f, char __user *buf,
					   size_t size, loff_t *pos);
static ssize_t ilitek_proc_fw_process_read(struct file *f, char __user *buf,
					   size_t size, loff_t *pos);
static ssize_t ilitek_proc_get_delta_data_read(struct file *f,
					       char __user *buf,
					       size_t size, loff_t *pos);
static ssize_t ilitek_proc_fw_get_raw_data_read(struct file *f,
						char __user *buf,
						size_t size, loff_t *pos);
static ssize_t ilitek_proc_rw_tp_reg_read(struct file *f, char __user *buf,
					  size_t size, loff_t *pos);
static ssize_t ilitek_proc_rw_tp_reg_write(struct file *f,
					   const char __user *buf,
					   size_t size, loff_t *pos);
static ssize_t ilitek_proc_fw_pc_counter_read(struct file *f, char __user *buf,
					      size_t size, loff_t *pos);
static ssize_t ilitek_proc_get_debug_mode_data_read(struct file *f,
						    char __user *buf,
						    size_t size, loff_t *pos);
static ssize_t ilitek_proc_get_debug_mode_data_write(struct file *f,
						     const char __user *buf,
						     size_t size, loff_t *pos);
static ssize_t ilitek_proc_debug_level_read(struct file *f, char __user *buf,
					    size_t size, loff_t *pos);

/* Group F, 0xffffff8008a57e88. */
int ilitek_tddi_ic_func_ctrl(const char *name, int ctrl);

int get_tp_recore_data(void);

/*
 * Group F, 0xffffff8008a57d20. No argument register is set at the call
 * site at 0xffffff8008a70d10, and the body does not read x0.
 */
void ilitek_tddi_ic_get_pc_counter(void);
/* Group F, 0xffffff8008a571a8. */
int ilitek_ice_mode_ctrl(int enable, int mcu);
/*
 * Group F, 0xffffff8008a57098 and 0xffffff8008a56ee0. The signatures come
 * from the call sites of `ilitek_proc_rw_tp_reg_read`.
 */
int ilitek_ice_mode_write(u32 addr, u32 data, int len);
int ilitek_ice_mode_read(u32 addr, void *data, int len);
/* Group E, 0xffffff8008a5aad8. The same signature declared in ilitek_bus.c. */
void ilitek_dump_data(void *data, int a, int b, int c, const char *label);

/* ==================================================================== */

/*
 * str2hex() was reconstructed from the factory kernel disassembly (0xffffff8008a6b528, 144 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int str2hex(char *str)
{
	int strlen, result, intermed, intermedtop;
	char *s = str;

	while (*s != 0x0)
		s++;

	strlen = (int)(s - str);
	s = str;
	if (*s != 0x30)
		return -1;

	s++;
	if (*s != 'x' && *s != 'X')
		return -1;

	s++;
	result = 0;
	while (*s != 0x0) {
		intermed = *s & 0x0f;
		intermedtop = *s & 0xf0;
		if (intermedtop == 0x60 || intermedtop == 0x40)
			intermed += 0x09;
		intermed = intermed << (4 * (strlen - 3));
		result = result | intermed;
		strlen--;
		s++;
	}
	return result;
}

/*
 * katoi() was reconstructed from the factory kernel disassembly (0xffffff8008a6b5b8, 116 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int katoi(char *str)
{
	int result = 0, signal = 0, d;

	if (*str == '-') {
		str++;
		signal = 1;
	} else if (*str == '+')
		str++;

	d = *str - '0';
	while (d >= 0 && d <= 9) {
		result = result * 10 + d;
		str++;
		d = *str - '0';
	}

	if (signal)
		result = -result;

	return result;
}

/*
 * get_tp_recore_ctrl() was reconstructed from the factory kernel disassembly (0xffffff8008a6b62c, 376 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int get_tp_recore_ctrl(int val)
{
	int ret = 0;

	switch (val) {
	case 0:
#line 1147
		ILI_INFO("recore enable");
		ret = ilitek_tddi_ic_func_ctrl("tp_recore", 1);
		mdelay(200);
		break;
	case 1:
		mdelay(50);
		ILI_INFO("Get data");
		ret = ilitek_tddi_ic_func_ctrl("tp_recore", 2);
		if (ret < 0) {
			ILI_ERR("cmd fail\n");
			break;
		}
		if (get_tp_recore_data() < 0)
#line 1162
			ILI_ERR("get data fail\n");
#line 1164
		ILI_INFO("recore reset");
		ret = ilitek_tddi_ic_func_ctrl("tp_recore", 3);
		if (ret < 0)
			ILI_ERR("cmd fail\n");
		break;
	case 2:
#line 1172
		ILI_INFO("recore disable");
		ret = ilitek_tddi_ic_func_ctrl("tp_recore", 0);
		break;
	default:
		break;
	}
	return ret;
}

/*
 * get_tp_recore_data() was reconstructed from the factory kernel disassembly (0xffffff8008a6b7a4, 724 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
int get_tp_recore_data(void)
{
	u8 buf[8] = {0};
	u16 *data;
	u32 addr;
	int len, unit, i;
	s8 lndex;
	u8 fram_num, record_case, stat;
	int ice = idev->c712;

	if (idev->c784(buf, 8) < 0) {
#line 1192
		ILI_ERR("Get info fail\n");
		return -1;
	}

	addr = 0x20000 | (buf[0] << 8) | buf[1];
	len = (buf[2] << 8) | buf[3];
	lndex = (s8)buf[4];
	fram_num = buf[5];
	record_case = buf[6];
	stat = buf[7];

	ILI_INFO("addr = 0x%x, len = %d, lndex = 0x%x, fram num = %d, record_case = 0x%x\n",
#line 1203
		 addr, len, lndex, fram_num, record_case);
	ilitek_dump_data(buf, 8, 8, 0, "all record bytes");

	data = kzalloc(len, GFP_ATOMIC);
	if (IS_ERR(data) || data == NULL) {
		ILI_ERR("Failed to allocate packet memory, %ld\n", PTR_ERR(data));
		return -1;
	}

	if (!ice)
		ilitek_ice_mode_ctrl(1, 1);

	buf[0] = 0x25;
	buf[1] = (u8)((addr & 0x000000ff) >> 0);
	buf[2] = (u8)((addr & 0x0000ff00) >> 8);
	buf[3] = (u8)((addr & 0x00ff0000) >> 16);

	if (idev->c776(buf, 4)) {
#line 1222
		ILI_ERR("Failed to write iram data\n");
		return -ENODEV;
	}

	if (idev->c784(data, len)) {
		ILI_ERR("Failed to Read iram data\n");
		return -ENODEV;
	}

	unit = len / (fram_num * 2);
	for (i = 0; i < fram_num; i++) {
		ilitek_dump_data(&data[lndex * unit], 16, unit,
				 idev->c248, "recore_data");
		lndex--;
		if (lndex < 0)
			lndex = fram_num - 1;
	}

	if (!ice)
		ilitek_ice_mode_ctrl(0, 1);

	if (record_case == 2) {
#line 1245
		ILI_INFO("tp_palm_stat = %d\n", stat & 0x3);
		ILI_INFO("app_an_stat = %d\n", (stat >> 2) & 0x7);
		ILI_INFO("app_check_abnor = %d\n", (stat >> 5) & 0x1);
		ILI_INFO("wrong_bg = %d\n", (stat >> 6) & 0x1);
	}

	kfree(data);
	return 0;
}

/*
 * gesture_fail_reason() was reconstructed from the factory kernel disassembly (0xffffff8008a6ba78, 348 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void gesture_fail_reason(int enable)
{
	u8 cmd[24] = {0};

	if (ilitek_tddi_ic_func_ctrl("knock_en", 8) < 0)
#line 1262
		ILI_ERR("set symbol failed");

	cmd[0] = 0x01;
	cmd[1] = 0x0A;
	cmd[2] = 0x10;
	cmd[3] = enable & 0x01;
	cmd[4] = 0xFF;
	cmd[5] = 0xFF;
	if (idev->c776(cmd, 6) < 0)
#line 1275
		ILI_ERR("enable gesture fail reason failed");

	cmd[0] = 0x01;
	cmd[1] = 0x0A;
	cmd[2] = 0x12;
	cmd[3] = 0x01;
	memset(&cmd[4], 0xFF, 20);
	if (idev->c776(cmd, 24) < 0)
#line 1284
		ILI_ERR("set gesture parameters failed");

	cmd[0] = 0x01;
	cmd[1] = 0x0A;
	cmd[2] = 0x11;
	cmd[3] = 0x01;
	if (idev->c776(cmd, 4) < 0)
#line 1292
		ILI_ERR("get gesture parameters failed");
}

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
typedef int (*ilitek_c792_t)(void *c8, const void *tx, u32 n_tx,
			     void *rx, u32 n_rx);
#define IDEV_C792	((ilitek_c792_t)idev->c792)

/*
 * THE FUNCTIONS FROM OTHER GROUPS CALLED BY `ilitek_node_ioctl_write`.
 * `ilitek.h` does not declare them; the signatures come from this
 * function's call sites, not from a hunch, and where the binary does not
 * decide, the comment says so.
 */
/*
 * 0xffffff8008a5a8bc, 0xffffff8008a5a61c, 0xffffff8008a5a004,
 * 0xffffff8008a59d74, 0xffffff8008a5a3e4, 0xffffff8008a5a1cc: six consecutive
 * calls of the "getinfo" command (0xffffff8008a6d1e0 .. 0xffffff8008a6d200)
 * which set no argument register and whose result is not used.
 */
int ilitek_tddi_ic_get_info(void);
int ilitek_tddi_ic_get_protocl_ver(void);
int ilitek_tddi_ic_get_fw_ver(void);
int ilitek_tddi_ic_get_core_ver(void);
int ilitek_tddi_ic_get_tp_info(void);
int ilitek_tddi_ic_get_panel_info(void);
/*
 * 0xffffff8008a55664. A single 32-bit argument
 * ("b9400680 ldr"@0xffffff8008a6d4b4).
 */
int core_spi_setup(int clk);
/*
 * 0xffffff8008a58704: two integers and a pointer
 * ("29408680 ldp"@0xffffff8008a6d7b4, "910093e2 add"@0xffffff8008a6d7b8);
 * what the third argument points at is read back with `ldrb`
 * ("394093e3 ldrb"@0xffffff8008a6d7c0), so it is one byte wide.
 */
int ilitek_tddi_ic_get_ddi_reg_onepage(int page, int reg, u8 *data);
/* 0xffffff8008a5836c: three ints ("29408680 ldp"@0xffffff8008a6d7f8 e
 * "b9400e82 ldr"@0xffffff8008a6d7fc). */
int ilitek_tddi_ic_set_ddi_reg_onepage(int page, int reg, int data);
/*
 * 0xffffff8008a68280 and 0xffffff8008a6714c: two integers and a third argument
 * that holds the constant 0 ("2a1f03e2 mov"@0xffffff8008a6d838) in one case and
 * the constant 1 ("320003e2 orr"@0xffffff8008a6d874) in the other. That it is a
 * `bool` and not an `int` is NOT decided by the binary: it is a choice.
 */
int ilitek_tddi_fw_dump_flash_data(int start, int end, bool user);
int ilitek_fw_dump_iram_data(int start, int end, bool user);
/*
 * 0xffffff8008a59f0c. A single 32-bit argument
 * ("b9400680 ldr"@0xffffff8008a6d898).
 */
int ilitek_tddi_fw_uart_ctrl(int ctrl);
/*
 * 0xffffff8008a5bdc4. No argument register is set at the site
 * ("97ffb946 bl"@0xffffff8008a6d8ac is preceded by the chain's `cbz`).
 */
int ilitek_tddi_touch_esd_gesture_flash(void);

/*
 * ilitek_node_ioctl_write() was reconstructed from the factory kernel disassembly (0xffffff8008a6c904, 4668 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t ilitek_node_ioctl_write(struct file *filp,
				       const char __user *buff, size_t size,
				       loff_t *pos)
{
	int i;
	int *data = NULL;
	u8 *rxbuf = NULL;
	char cmd[512] = {0};
	char *token = NULL, *cur = NULL;
	u8 temp[256] = {0};
	u8 ddi_data = 0;
	int w_len = 0, r_len = 0, delay = 0;
	size_t len = 0;
	ssize_t count = size;

	/*
	 * "f10806bf cmp"@0xffffff8008a6c950 compares `size - 1`
	 * ("d1000675 sub"@0xffffff8008a6c94c) with 513, and
	 * "54000123 b.cc"@0xffffff8008a6c99c carries on only if it is LESS:
	 * the error fires for `size - 1 > 512`, not for `size > 512`.
	 * The local buffer is 512 bytes ("321703e2 orr"@0xffffff8008a6c93c
	 * is the third argument of the initial `__memset`).
	 */
	if (size - 1 > 512) {
#line 1305
		ILI_ERR("ERROR! input length is larger than local buffer\n");
		return -1;
	}

	mutex_lock(&idev->touch_mutex);

	/*
	 * FACTORY DEFECT F9: the `copy_from_user` error path does NOT
	 * release the mutex. "17fffdb0 b"@0xffffff8008a6d2f4 jumps to the
	 * `printk` shared with the length error
	 * ("97db1ac8 bl"@0xffffff8008a6c9b4) and from there to
	 * "140001ea b"@0xffffff8008a6c9bc, which goes DIRECTLY to the stack
	 * canary check and the epilogue: the `mutex_unlock` of
	 * "9410189b bl"@0xffffff8008a6d160 is not on that path. Reproduced.
	 */
	if (buff != NULL) {
		if (copy_from_user(cmd, buff, size - 1)) {
#line 1313
			ILI_ERR("Failed to copy data from user space\n");
			return -1;
		}
	}

	ILI_INFO("size = %d, cmd = %s\n", (int)size, cmd);

	/*
	 * kcalloc() was reconstructed from the factory kernel disassembly (0xffffff8008a6ca64, 2048 bytes).
	 *
	 * The working notes -- the disassembly citations, the measurements against
	 * the factory binary and the reasoning behind each choice -- are in
	 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
	 * in the oracolo repository. They are kept in Italian, as the project's
	 * internal record.
	 */
	data = kcalloc(512, sizeof(int), GFP_KERNEL);

	cur = cmd;
	i = 0;
	/*
	 * "940fef95 bl"@0xffffff8008a6ca80 towards <strsep>, with
	 * "91241c21 add"@0xffffff8008a6ca78 loading the separator ",".
	 * `str2hex` is INLINED here: the marker is
	 * "7101819f cmp"@0xffffff8008a6cb08 (the lower/upper case test on the
	 * nibble), and there is no `bl <str2hex>` anywhere in the block.
	 * The index is a SIGNED integer:
	 * "b835da84 str"@0xffffff8008a6cb38 indexes with `sxtw`.
	 */
	while ((token = strsep(&cur, ",")) != NULL) {
		data[i] = str2hex(token);
#line 1326
		ILI_INFO("data[%d] = %x\n", i, data[i]);
		i++;
	}

	ILI_INFO("cmd = %s\n", cmd);

	/*
	 * "940fade3 bl"@0xffffff8008a6cb74 towards <__pi_strlen>, ONCE
	 * only: the result serves all forty `strncmp`s
	 * ("aa1503e2 mov"@0xffffff8008a6cb88 and its thirty-nine twins).
	 */
	len = strlen(cmd);

	if (!strncmp(cmd, "hwreset", len)) {
		/* "321f03e0 orr"@0xffffff8008a6cfdc */
		ilitek_tddi_reset_ctrl(2);
	} else if (!strcmp(cmd, "rawdatarecore")) {
		/*
		 * This section was reconstructed from the factory kernel disassembly (0xffffff8009247564).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		switch (data[1]) {
		case 0:
			get_tp_recore_ctrl(0);
			break;
		case 1:
			get_tp_recore_ctrl(1);
			break;
		case 2:
			get_tp_recore_ctrl(2);
			break;
		}
	} else if (!strcmp(cmd, "switchdemodebuginfomode")) {
		/* "321f03e0 orr"@0xffffff8008a6d048 */
		ilitek_set_tp_data_len(2);
	} else if (!strcmp(cmd, "gesturedemoen")) {
		/*
		 * "1a9f07e8 cset"@0xffffff8008a6d068 (`ne`) and
		 * "b9021d28 str"@0xffffff8008a6d06c: it is a conversion to
		 * 0/1, not a copy.
		 */
		idev->c540 = (data[1] != 0);
		/* "320007e0 orr"@0xffffff8008a6d060 */
		ilitek_set_tp_data_len(3);
	} else if (!strcmp(cmd, "gesturefailrsn")) {
		/*
		 * Two distinct call sites and not a `cset`:
		 * "320003e0 orr"@0xffffff8008a6d0c4 and
		 * "2a1f03e0 mov"@0xffffff8008a6d110 converge on
		 * "97fffa59 bl"@0xffffff8008a6d114. `data[1]` is RE-READ after
		 * the call ("b9400688 ldr"@0xffffff8008a6d118).
		 */
		if (data[1])
			gesture_fail_reason(1);
		else
			gesture_fail_reason(0);
		ILI_INFO("%s gesture fail reason\n",
#line 1354
			 data[1] ? "ENABLE" : "DISABLE");
	} else if (!strncmp(cmd, "icwholereset", len)) {
		/* "2a1f03e1 mov"@0xffffff8008a6d0d0 then
		 * "2a1f03e0 mov"@0xffffff8008a6d0d8 */
		ilitek_ice_mode_ctrl(1, 0);
		ilitek_tddi_reset_ctrl(0);
	} else if (!strncmp(cmd, "iccodereset", len)) {
		/* "320003e0 orr"@0xffffff8008a6d0f4 is the 1 of reset_ctrl */
		ilitek_ice_mode_ctrl(1, 0);
		ilitek_tddi_reset_ctrl(1);
		ilitek_ice_mode_ctrl(0, 0);
	} else if (!strcmp(cmd, "infofromhex")) {
		/* "1a9f07e8 cset"@0xffffff8008a6d1b0 e
		 * "3909f128 strb"@0xffffff8008a6d1b4 (one byte). */
		idev->c636 = (data[1] != 0);
		/* "b9400683 ldr"@0xffffff8008a6d1b8: `data[1]` is READ BACK. */
#line 1364
		ILI_INFO("info from hex = %d\n", data[1]);
	} else if (!strncmp(cmd, "getinfo", len)) {
		ilitek_ice_mode_ctrl(1, 0);
		ilitek_tddi_ic_get_info();
		ilitek_ice_mode_ctrl(0, 0);
		ilitek_tddi_ic_get_protocl_ver();
		ilitek_tddi_ic_get_fw_ver();
		ilitek_tddi_ic_get_core_ver();
		ilitek_tddi_ic_get_tp_info();
		ilitek_tddi_ic_get_panel_info();
		/* "91356063 add"@0xffffff8008a6d218 loads
		 * "2.0.6.0.191122"@0xffffff800923ed58 */
#line 1374
		ILI_INFO("Driver version = %s\n", "2.0.6.0.191122");
		/*
		 * "f9414903 ldr"@0xffffff8008a6d238: the field at +656 passed
		 * to a `%s`, hence a pointer to text. `ilitek.h` keeps it a
		 * `u64`: the cast is declared, not hidden.
		 */
#line 1375
		ILI_INFO("TP module = %s\n", (char *)(uintptr_t)idev->c656);
	} else if (!strncmp(cmd, "enableicemode", len)) {
		/*
		 * "7100051f cmp"@0xffffff8008a6d254 with 1, and
		 * "54fff541 b.ne"@0xffffff8008a6d258 towards the site shared
		 * with "disableicemode": these are TWO distinct `bl`s, not a
		 * `cset`.
		 */
		if (data[1] == 1)
			ilitek_ice_mode_ctrl(1, 1);
		else
			ilitek_ice_mode_ctrl(1, 0);
	} else if (!strncmp(cmd, "wqctrl", len)) {
		/*
		 * "52000123 eor"@0xffffff8008a6d288 on a byte just read
		 * ("39483d09 ldrb"@0xffffff8008a6d27c) and written back
		 * ("39083d03 strb"@0xffffff8008a6d28c). With `u8 c527` the
		 * form `x = !x` would cost an extra `cmp`+`cset`: see the
		 * header delta, entry D-H6.
		 */
		idev->c527 ^= 1;
#line 1383
		ILI_INFO("wq_ctrl flag= %d\n", idev->c527);
	} else if (!strncmp(cmd, "disableicemode", len)) {
		ilitek_ice_mode_ctrl(0, 0);
	} else if (!strncmp(cmd, "enablewqesd", len)) {
		ilitek_tddi_wq_ctrl(0, 1);
	} else if (!strncmp(cmd, "enablewqbat", len)) {
		ilitek_tddi_wq_ctrl(1, 1);
	} else if (!strncmp(cmd, "disablewqesd", len)) {
		ilitek_tddi_wq_ctrl(0, 0);
	} else if (!strncmp(cmd, "disablewqbat", len)) {
		ilitek_tddi_wq_ctrl(1, 0);
	} else if (!strncmp(cmd, "gesture", len)) {
		/* "52000123 eor"@0xffffff8008a6d314 on the byte at +532 */
		idev->c532 ^= 1;
#line 1396
		ILI_INFO("gesture = %d\n", idev->c532);
	} else if (!strncmp(cmd, "esdgesture", len)) {
		/*
		 * "36fff100 tbz"@0xffffff8008a6d32c tests the sign bit alone:
		 * it is `< 0`, not `!= 0`.
		 */
		if (ilitek_tddi_gesture_recovery() < 0) {
#line 1399
			ILI_ERR("Gesture recovery failed\n");
			count = -1;
		}
	} else if (!strncmp(cmd, "esdspi", len)) {
		ilitek_tddi_spi_recovery();
	} else if (!strncmp(cmd, "sleepin", len)) {
		/* "9110c400 add"@0xffffff8008a6d364 loads
		 * "sleep"@0xffffff800911c431 */
		ilitek_tddi_ic_func_ctrl("sleep", 0);
	} else if (!strncmp(cmd, "deepsleepin", len)) {
		/* "320007e1 orr"@0xffffff8008a6d378 */
		ilitek_tddi_ic_func_ctrl("sleep", 3);
	} else if (!strncmp(cmd, "iceflag", len)) {
		/*
		 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a6d3a0).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		idev->c712 = (data[1] == 1);
#line 1413
		ILI_INFO("ice mode flag = %d\n", READ_ONCE(idev->c712));
	} else if (!strncmp(cmd, "gesturenormal", len)) {
		/* "528000a9 mov"@0xffffff8008a6d3c8 (5, `movz`) e
		 * "b9021909 str"@0xffffff8008a6d400 */
		idev->c536 = 5;
		ILI_INFO("gesture mode = %d\n", idev->c536);
	} else if (!strncmp(cmd, "gestureinfo", len)) {
		/* "321e03e9 orr"@0xffffff8008a6d3ec (4, logical immediate) */
		idev->c536 = 4;
		ILI_INFO("gesture mode = %d\n", idev->c536);
	} else if (!strncmp(cmd, "netlink", len)) {
		/* "52000123 eor"@0xffffff8008a6d430 on the byte at +530 */
		idev->c530 ^= 1;
#line 1422
		ILI_INFO("netlink flag= %d\n", idev->c530);
	} else if (!strncmp(cmd, "switchtestmode", len)) {
		/* "320003e0 orr"@0xffffff8008a6d43c */
		ilitek_tddi_switch_tp_mode(1);
	} else if (!strncmp(cmd, "switchdebugmode", len)) {
		/* "320003e0 orr"@0xffffff8008a6d454 */
		ilitek_set_tp_data_len(1);
	} else if (!strncmp(cmd, "switchdemomode", len)) {
		/*
		 * No `mov w0`: w0 is already 0 from the chain's `cbz`
		 * ("34003240 cbz"@0xffffff8008a6ce18).
		 */
		ilitek_set_tp_data_len(0);
	} else if (!strncmp(cmd, "switchgesturedebugmode", len)) {
		/* "32000be0 orr"@0xffffff8008a6d45c */
		ilitek_set_tp_data_len(7);
	} else if (!strncmp(cmd, "dbgflag", len)) {
		/* "52000123 eor"@0xffffff8008a6d48c on the byte at +556 */
		idev->c556 ^= 1;
#line 1433
		ILI_INFO("debug flag message = %d\n", idev->c556);
	} else if (!strncmp(cmd, "spiclk", len)) {
		/*
		 * `data[1]` read TWICE:
		 * "b9400683 ldr"@0xffffff8008a6d498 for the printk and
		 * "b9400680 ldr"@0xffffff8008a6d4b4 for the call.
		 */
#line 1435
		ILI_INFO("spi clk num = %d\n", data[1]);
		core_spi_setup(data[1]);
	} else if (!strncmp(cmd, "ss", len)) {
		ILI_INFO("sense_stop = %d\n", data[1]);
		/* "1a9f07e8 cset"@0xffffff8008a6d4f0 e
		 * "390a0928 strb"@0xffffff8008a6d4f4 (byte at +642) */
		idev->c642 = (data[1] != 0);
	} else if (!strncmp(cmd, "iow", len)) {
		w_len = data[1];
#line 1443
		ILI_INFO("w_len = %d\n", w_len);
		/*
		 * The index of `data` starts at 2: the base of the loop is
		 * "b25f03fa orr"@0xffffff8008a6d538 (0x200000000), which
		 * "935eff48 asr"@0xffffff8008a6d54c reduces to 8 = 2 * 4, and the
		 * stride is "b26003fc orr"@0xffffff8008a6d548 (0x100000000),
		 * that is 4 after the same `asr`.
		 */
		for (i = 0; i < w_len; i++) {
			temp[i] = data[i + 2];
#line 1447
			ILI_INFO("write[%d] = %x\n", i, temp[i]);
		}
		/* "f9418508 ldr"@0xffffff8008a6d58c: the field at +776 */
		idev->c776(temp, w_len);
	} else if (!strncmp(cmd, "ior", len)) {
		r_len = data[1];
#line 1454
		ILI_INFO("r_len = %d\n", r_len);
		/* "f9418908 ldr"@0xffffff8008a6d5d0: the field at +784 */
		idev->c784(temp, r_len);
		for (i = 0; i < r_len; i++)
#line 1457
			ILI_INFO("read[%d] = %x\n", i, temp[i]);
	} else if (!strncmp(cmd, "iowr", len)) {
		w_len = data[1];
		r_len = data[2];
		delay = data[3];
		ILI_INFO("w_len = %d, r_len = %d, delay = %d\n",
#line 1464
			 w_len, r_len, delay);
		/*
		 * Base 0x400000000 ("b25e03fc orr"@0xffffff8008a6d65c), that is
		 * index 4 after "935eff88 asr"@0xffffff8008a6d670.
		 */
		for (i = 0; i < w_len; i++) {
			temp[i] = data[i + 4];
#line 1468
			ILI_INFO("write[%d] = %x\n", i, temp[i]);
		}
		idev->c776(temp, w_len);
		/* The clearing is lowered to pairs of `stp xzr`:
		 * "a90f7d1f stp"@0xffffff8008a6d6bc .. "f9001bff
		 * str"@0xffffff8008a6d700, 256 byte exact. */
		memset(temp, 0, sizeof(temp));
		/*
		 * The guard and the loop are those of `mdelay` on a non-constant
		 * value: "34000108 cbz"@0xffffff8008a6d704 and
		 * "cb0803f5 neg"@0xffffff8008a6d70c, with
		 * "52912b00 mov"@0xffffff8008a6d710 being `udelay(1000)`.
		 */
		mdelay(delay);
		idev->c784(temp, r_len);
		for (i = 0; i < r_len; i++)
#line 1476
			ILI_INFO("read[%d] = %x\n", i, temp[i]);
	} else if (!strncmp(cmd, "getddiregdata", len)) {
		ILI_INFO("Get ddi reg one page: page = %x, reg = %x\n",
			 data[1], data[2]);
		/*
		 * "910093e2 add"@0xffffff8008a6d7b8: the third argument is
		 * the address of a byte on the stack, cleared beforehand
		 * ("390093ff strb"@0xffffff8008a6d7ac).
		 */
		ilitek_tddi_ic_get_ddi_reg_onepage(data[1], data[2],
						   &ddi_data);
#line 1481
		ILI_INFO("ddi_data = %x\n", ddi_data);
	} else if (!strncmp(cmd, "setddiregdata", len)) {
		ILI_INFO("Set ddi reg one page: page = %x, reg = %x, data = %x\n",
#line 1483
			 data[1], data[2], data[3]);
		ilitek_tddi_ic_set_ddi_reg_onepage(data[1], data[2], data[3]);
	} else if (!strncmp(cmd, "dumpflashdata", len)) {
		/* "910a20a5 add"@0xffffff8008a6d828 loads
		 * "/sdcard/flash_dump"@0xffffff8009246288 */
		ILI_INFO("Start = 0x%x, End = 0x%x, Dump Hex path = %s\n",
#line 1486
			 data[1], data[2], "/sdcard/flash_dump");
		ilitek_tddi_fw_dump_flash_data(data[1], data[2], false);
	} else if (!strncmp(cmd, "dumpiramdata", len)) {
		/* "912e28a5 add"@0xffffff8008a6d864 loads
		 * "/sdcard/iram_dump"@0xffffff8009245b8a */
		ILI_INFO("Start = 0x%x, End = 0x%x, Dump IRAM path = %s\n",
#line 1489
			 data[1], data[2], "/sdcard/iram_dump");
		ilitek_fw_dump_iram_data(data[1], data[2], true);
	} else if (!strncmp(cmd, "edge_palm_ctrl", len)) {
		/*
		 * "91275000 add"@0xffffff8008a6d890 loads
		 * "edge_palm"@0xffffff80092419d4, which is NOT the text of the
		 * command: the command is "edge_palm_ctrl".
		 */
		ilitek_tddi_ic_func_ctrl("edge_palm", data[1]);
	} else if (!strncmp(cmd, "uart_mode_ctrl", len)) {
		ilitek_tddi_fw_uart_ctrl(data[1]);
	} else if (!strncmp(cmd, "flashesdgesture", len)) {
		ilitek_tddi_touch_esd_gesture_flash();
	} else if (!strncmp(cmd, "spiw", len)) {
		w_len = data[1];
		/*
		 * FACTORY DEFECT F10: the command byte 0x82 written into
		 * `temp[0]` ("52801048 mov"@0xffffff8008a6d8c0,
		 * "3900c3e8 strb"@0xffffff8008a6d8c4) is OVERWRITTEN by the
		 * first iteration of the loop, which also starts at index 0
		 * ("38366b68 strb"@0xffffff8008a6d90c indexes with x22 = i,
		 * and x27 = sp+0x30 is that same `temp[0]`). It survives only if
		 * `w_len` is zero. Reproduced.
		 */
		temp[0] = 0x82;
		for (i = 0; i < w_len; i++) {
			temp[i] = data[i + 2];
#line 1503
			ILI_INFO("write[%d] = %x\n", i, temp[i]);
		}
		IDEV_C792(idev->c8, temp, w_len, NULL, 0);
	} else if (!strncmp(cmd, "spir", len)) {
		r_len = data[1];
		/*
		 * "52901821 mov"@0xffffff8008a6d954 with
		 * "72a02801 movk"@0xffffff8008a6d958: 0x14080c1, that is
		 * GFP_KERNEL | __GFP_ZERO | __GFP_DMA.
		 */
		rxbuf = kzalloc(r_len, GFP_DMA | GFP_KERNEL);
		/*
		 * "b140041f cmn"@0xffffff8008a6d968 is the IS_ERR
		 * (comparison with -4096), "b4000935 cbz"@0xffffff8008a6d970 the
		 * NULL.
		 */
		if (IS_ERR(rxbuf) || rxbuf == NULL) {
			ILI_ERR("Failed to allocate packet memory, %ld\n",
				PTR_ERR(rxbuf));
		} else {
			temp[0] = 0x83;
			IDEV_C792(idev->c8, temp, 1, rxbuf, r_len);
			for (i = 0; i < r_len; i++)
#line 1519
				ILI_INFO("read[%d] = %x\n", i, rxbuf[i]);
		}
		/*
		 * FACTORY DEFECT F11: the `kfree` is on the error path too --
		 * "97db168a bl"@0xffffff8008a6daac (the printk) FALLS THROUGH to
		 * "aa1503e0 mov"@0xffffff8008a6dab0 and
		 * "97df8b3f bl"@0xffffff8008a6dab4 -- so with an ERR_PTR it frees
		 * a pointer that is not memory. Reproduced.
		 */
		kfree(rxbuf);
	} else if (!strncmp(cmd, "spirw", len)) {
		w_len = data[1];
		r_len = data[2];
		/*
		 * Base 0x300000000 ("b26007fb orr"@0xffffff8008a6da04), that is
		 * index 3.
		 */
		for (i = 0; i < w_len; i++) {
			temp[i] = data[i + 3];
#line 1528
			ILI_INFO("write[%d] = %x\n", i, temp[i]);
		}
		/*
		 * "340003a8 cbz"@0xffffff8008a6da50: with no read length nothing
		 * is allocated and `rxbuf` stays NULL
		 * ("aa1f03f7 mov"@0xffffff8008a6dac4).
		 */
		if (r_len) {
			rxbuf = kzalloc(r_len, GFP_DMA | GFP_KERNEL);
			if (IS_ERR(rxbuf) || rxbuf == NULL) {
				ILI_ERR("Failed to allocate packet memory, %ld\n",
					PTR_ERR(rxbuf));
				goto spirw_free;
			}
		}
		IDEV_C792(idev->c8, temp, w_len, rxbuf, r_len);
		for (i = 0; i < r_len; i++)
#line 1541
			ILI_INFO("read[%d] = %x\n", i, rxbuf[i]);
spirw_free:
		/*
		 * Here too the `kfree` is on the error path (defect F11):
		 * "97db1692 bl"@0xffffff8008a6da8c jumps to
		 * "aa1703e0 mov"@0xffffff8008a6db34.
		 */
		kfree(rxbuf);
	} else {
#line 1545
		ILI_ERR("Unknown command\n");
		count = -1;
	}

	/*
	 * "b4000074 cbz"@0xffffff8008a6d14c guards the `kfree`, and there is no
	 * store of NULL afterwards.
	 */
	if (data != NULL)
		kfree(data);

	mutex_unlock(&idev->touch_mutex);
	return count;
}

/*
 * This section was reconstructed from the factory kernel disassembly.
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */

/*
 * ilitek_node_compat_ioctl() was reconstructed from the factory kernel disassembly (0xffffff8008a6f138, 1840 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static long ilitek_node_compat_ioctl(struct file *filp, unsigned int cmd,
				     unsigned long arg)
{
	long ret = 0;

	/*
	 * "f9401408 ldr"@0xffffff8008a6f148 reads `filp + 40` and
	 * "f9402508 ldr"@0xffffff8008a6f150 reads `+72` of what it found:
	 * these are `offsetof(struct file, f_op)` and
	 * `offsetof(struct file_operations, unlocked_ioctl)` in 4.14.
	 * "b27bf7e0 orr"@0xffffff8008a6f208 is -25, that is -ENOTTY.
	 */
	if (!filp->f_op || !filp->f_op->unlocked_ioctl) {
#line 1560
		ILI_ERR("There's no unlocked_ioctl defined in file\n");
		return -ENOTTY;
	}

	/*
	 * "12001ea3 and"@0xffffff8008a6f178: the command masked to one byte,
	 * that is `_IOC_NR`.
	 */
#line 1564
	ILI_DBG("cmd = %d\n", _IOC_NR(cmd));

	switch (cmd) {
	case _IOWR(0x64, 0, u32):
		ILI_DBG("compat_ioctl: convert i2c/spi write\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 0, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 1, u32):
#line 1576
		ILI_DBG("compat_ioctl: convert set write length\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 1, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 2, u32):
#line 1572
		ILI_DBG("compat_ioctl: convert i2c/spi read\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 2, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 3, u32):
#line 1580
		ILI_DBG("compat_ioctl: convert set read length\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 3, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 4, u32):
#line 1584
		ILI_DBG("compat_ioctl: convert hw reset\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 4, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 5, u32):
#line 1588
		ILI_DBG("compat_ioctl: convert power switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 5, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 6, u32):
#line 1592
		ILI_DBG("compat_ioctl: convert report switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 6, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 7, u32):
#line 1596
		ILI_DBG("compat_ioctl: convert irq switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 7, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 8, u32):
#line 1600
		ILI_DBG("compat_ioctl: convert debug level\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 8, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 9, u32):
#line 1604
		ILI_DBG("compat_ioctl: convert format mode\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 9, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 10, u32):
		/*
		 * The format is THE SAME as case 3 ("convert set read
		 * length"): "9102a400 add"@0xffffff8008a6f450 and
		 * "9102a400 add"@0xffffff8008a6f2ac load the same
		 * address 0xffffff80092490a9.
		 */
#line 1608
		ILI_DBG("compat_ioctl: convert set read length\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 10, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 11, u32):
#line 1612
		ILI_DBG("compat_ioctl: convert fw version\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 11, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 12, u32):
#line 1616
		ILI_DBG("compat_ioctl: convert core version\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 12, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 13, u32):
#line 1620
		ILI_DBG("compat_ioctl: convert driver version\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 13, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 14, u32):
#line 1624
		ILI_DBG("compat_ioctl: convert chip id\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 14, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 15, u32):
#line 1628
		ILI_DBG("compat_ioctl: convert netlink ctrl\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 15, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 16, u32):
#line 1632
		ILI_DBG("compat_ioctl: convert netlink status\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 16, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 17, u32):
#line 1636
		ILI_DBG("compat_ioctl: convert tp mode ctrl\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 17, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 18, u32):
#line 1640
		ILI_DBG("compat_ioctl: convert tp mode status\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 18, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 19, u32):
#line 1644
		ILI_DBG("compat_ioctl: convert tp mode switch\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 19, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 20, u32):
#line 1648
		ILI_DBG("compat_ioctl: convert interface type\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 20, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 21, u32):
#line 1652
		ILI_DBG("compat_ioctl: convert dump flash\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 21, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 22, u32):
#line 1656
		ILI_DBG("compat_ioctl: convert fw uart\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 22, unsigned long),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 23, u32):
#line 1660
		ILI_DBG("compat_ioctl: convert resolution\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 23, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 24, u32):
#line 1664
		ILI_DBG("compat_ioctl: convert tp info\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 24, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 25, u32):
#line 1668
		ILI_DBG("compat_ioctl: convert wrapper rw\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 25, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 26, u32):
#line 1672
		ILI_DBG("compat_ioctl: convert ddi write\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 26, u32),
			(unsigned long)compat_ptr(arg));
		break;
	case _IOWR(0x64, 27, u32):
#line 1676
		ILI_DBG("compat_ioctl: convert ddi read\n");
		ret = filp->f_op->unlocked_ioctl(filp,
			_IOWR(0x64, 27, u32),
			(unsigned long)compat_ptr(arg));
		break;
	default:
		/*
		 * "92804040 mov"@0xffffff8008a6f228 is -515, that is
		 * -ENOIOCTLCMD. The text says "return ilitek_node_ioctl" and the
		 * code does NOT call it: it is the factory's message and is
		 * reproduced as it stands (rule 7).
		 */
#line 1680
		ILI_ERR("no ioctl cmd, return ilitek_node_ioctl\n");
		return -ENOIOCTLCMD;
	}

	return ret;
}

/*
 * ilitek_node_ioctl() was reconstructed from the factory kernel disassembly (0xffffff8008a6db40, 5624 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static long ilitek_node_ioctl(struct file *filp, unsigned int cmd,
			      unsigned long arg)
{
	int ret = 0;
	u8 *szBuf = NULL;
	u8 if_to_user = 0;
	u32 buf[64] = {0};
	/*
	 * Read once on entry: see the header. `c528` and `c529` are the two
	 * bytes that say whether the ESD queue and the battery one were
	 * running; the first argument of `ilitek_tddi_wq_ctrl` is 0 for the
	 * ESD one and 1 for the battery one ("2a1f03e0 mov"@0xffffff8008a6dc7c
	 * against "320003e0 orr"@0xffffff8008a6dc8c).
	 */
	u8 c528 = idev->c528;
	u8 c529 = idev->c529;
	u32 c48_40, c48_44;

	/*
	 * "12181c29 and"@0xffffff8008a6db68 with
	 * "6b0a013f cmp"@0xffffff8008a6db70: the command type against 0x64.
	 */
	if (_IOC_TYPE(cmd) != 0x64) {
#line 1695
		ILI_ERR("The Magic number doesn't match\n");
		return -ENOTTY;
	}

	/*
	 * "7100707f cmp"@0xffffff8008a6dbcc with
	 * "540001e3 b.cc"@0xffffff8008a6dbd0: the number against 28.
	 */
	if (_IOC_NR(cmd) >= 28) {
#line 1700
		ILI_ERR("The number of ioctl doesn't match\n");
		return -ENOTTY;
	}

	ILI_DBG("cmd = %d\n", _IOC_NR(cmd));

	mutex_lock(&idev->touch_mutex);

	/*
	 * "321403e2 orr"@0xffffff8008a6dc60: 4096 bytes, and
	 * "72a02801 movk"@0xffffff8008a6dc5c with
	 * "52901801 mov"@0xffffff8008a6dc58: GFP_KERNEL | __GFP_ZERO, WITHOUT
	 * __GFP_DMA (here the mask is 0x14080c0, not 0x14080c1).
	 */
	szBuf = kzalloc(4096, GFP_KERNEL);
	if (IS_ERR(szBuf) || szBuf == NULL) {
#line 1710
		ILI_ERR("Failed to allocate mem\n");
		ret = -ENOMEM;
		goto out_no_free;
	}

	if (c528)
		ilitek_tddi_wq_ctrl(0, 0);
	if (c529)
		ilitek_tddi_wq_ctrl(1, 0);

	switch (cmd) {
	case _IOWR(0x64, 0, unsigned long):
		ILI_DBG("ioctl: write len = %d\n", ioctl_len);
		/*
		 * "794dd2d5 ldrh"@0xffffff8008a6dcf4 re-reads the global after
		 * the printk, "714006bf cmp"@0xffffff8008a6dcf8 compares it
		 * with 4096 and "54006669 b.ls"@0xffffff8008a6dcfc carries on: the
		 * error path is STRICTLY GREATER.
		 */
		if (ioctl_len > 4096) {
			ILI_ERR("ERROR! write len is largn than ioctl buf (%d, %ld)\n",
#line 1726
				ioctl_len, 4096L);
			ret = -ENOTTY;
			break;
		}
		if (copy_from_user(szBuf, (u8 __user *)arg, ioctl_len)) {
#line 1732
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		/* "f9418508 ldr"@0xffffff8008a6ea30: the field at +776. */
		ret = idev->c776(szBuf, ioctl_len);
		if (ret < 0)
			ILI_ERR("Failed to write data\n");
		break;

	case _IOWR(0x64, 1, u32):
		/* "790dd114 strh"@0xffffff8008a6ddd8: two bytes, not four. */
		ioctl_len = arg;
		ret = 0;
		break;

	case _IOWR(0x64, 2, unsigned long):
#line 1742
		ILI_DBG("ioctl: read len = %d\n", ioctl_len);
		if (ioctl_len > 4096) {
			ILI_ERR("ERROR! read len is largn than ioctl buf (%d, %ld)\n",
#line 1746
				ioctl_len, 4096L);
			ret = -ENOTTY;
			break;
		}
		/* "f9418908 ldr"@0xffffff8008a6ea68: the field at +784. */
		ret = idev->c784(szBuf, ioctl_len);
		if (ret < 0) {
			ILI_ERR("Failed to read data\n");
			break;
		}
		if (copy_to_user((u8 __user *)arg, szBuf, ioctl_len)) {
#line 1758
			ILI_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 3, u32):
		ioctl_len = arg;
		ret = 0;
		break;

	case _IOWR(0x64, 4, u32):
#line 1767
		ILI_DBG("ioctl: hw reset\n");
		/* "b9426500 ldr"@0xffffff8008a6e694: the field at +612, read as
		 * 32 bit and passed straight through. */
		ilitek_tddi_reset_ctrl(idev->c612);
		ret = 0;
		break;

	case _IOWR(0x64, 5, u32):
#line 1771
		ILI_DBG("Not implemented yet\n");
		ret = 0;
		break;

	case _IOWR(0x64, 6, u32):
		if (copy_from_user(szBuf, (u8 __user *)arg, 1)) {
#line 1775
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
#line 1780
		ILI_DBG("ioctl: report switch = %d\n", szBuf[0]);
		if (szBuf[0]) {
			idev->c531 = 1;
			ILI_DBG("report is enabled\n");
		} else {
			idev->c531 = 0;
			ILI_DBG("report is disabled\n");
		}
		ret = 0;
		break;

	case _IOWR(0x64, 7, u32):
		if (copy_from_user(szBuf, (u8 __user *)arg, 1)) {
#line 1791
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: irq switch = %d\n", szBuf[0]);
		if (szBuf[0])
			ilitek_plat_irq_enable();
		else
			ilitek_plat_irq_disable();
		ret = 0;
		break;

	case _IOWR(0x64, 8, u32):
		/*
		 * This section was reconstructed from the factory kernel disassembly (0xffffff8008a6e810).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		if (copy_from_user(szBuf, (u8 __user *)arg, sizeof(u32))) {
#line 1803
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ilitek_dbg_en ^= 1;
#line 1809
		ILI_INFO("ipio_debug_level = %d", ilitek_dbg_en);
		ret = 0;
		break;

	case _IOWR(0x64, 9, u32):
		if (copy_from_user(szBuf, (u8 __user *)arg, 3)) {
#line 1813
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: set func mode = %x,%x,%x\n",
			szBuf[0], szBuf[1], szBuf[2]);
		idev->c776(szBuf, 3);
		ret = 0;
		break;

	case _IOWR(0x64, 10, unsigned long):
#line 1822
		ILI_DBG("ioctl: get fw version\n");
		/*
		 * IDEV_C48_AT() was reconstructed from the factory kernel disassembly (0xffffff8008a6eb00).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		szBuf[3] = IDEV_C48_AT(48) & 0xFF;
		szBuf[2] = (IDEV_C48_AT(48) >> 8) & 0xFF;
		szBuf[1] = (IDEV_C48_AT(48) >> 16) & 0xFF;
		szBuf[0] = (IDEV_C48_AT(48) >> 24) & 0xFF;
		ILI_DBG("Firmware version = %d.%d.%d.%d\n",
#line 1828
			szBuf[0], szBuf[1], szBuf[2], szBuf[3]);
		if (copy_to_user((u8 __user *)arg, szBuf, sizeof(u32))) {
#line 1831
			ILI_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 11, unsigned long):
#line 1836
		ILI_DBG("ioctl: get protocl version\n");
		/*
		 * "f9401d69 ldr"@0xffffff8008a6ebbc reads the field at +56 and
		 * "b9400129 ldr"@0xffffff8008a6ebc0 the value at its offset 0.
		 * Three bytes, not four.
		 */
		szBuf[2] = IDEV_C56_AT(0) & 0xFF;
		szBuf[1] = (IDEV_C56_AT(0) >> 8) & 0xFF;
		szBuf[0] = (IDEV_C56_AT(0) >> 16) & 0xFF;
		ILI_DBG("Protocol version = %d.%d.%d\n",
#line 1841
			szBuf[0], szBuf[1], szBuf[2]);
		if (copy_to_user((u8 __user *)arg, szBuf, 3)) {
#line 1844
			ILI_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 12, unsigned long):
#line 1849
		ILI_DBG("ioctl: get core version\n");
		/*
		 * Same pattern as case 10, but at offset +52:
		 * "f9401989 ldr"@0xffffff8008a6ec74 then
		 * "b940352a ldr"@0xffffff8008a6ec78.
		 */
		szBuf[3] = IDEV_C48_AT(52) & 0xFF;
		szBuf[2] = (IDEV_C48_AT(52) >> 8) & 0xFF;
		szBuf[1] = (IDEV_C48_AT(52) >> 16) & 0xFF;
		szBuf[0] = (IDEV_C48_AT(52) >> 24) & 0xFF;
		ILI_DBG("Core version = %d.%d.%d.%d\n",
#line 1855
			szBuf[0], szBuf[1], szBuf[2], szBuf[3]);
		if (copy_to_user((u8 __user *)arg, szBuf, sizeof(u32))) {
#line 1858
			ILI_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 13, unsigned long):
#line 1863
		ILI_DBG("ioctl: get driver version\n");
		/*
		 * The copy is FIFTEEN bytes (8 + 7 overlapping,
		 * "f9400108 ldr"@0xffffff8008a6def4 and
		 * "f8407109 ldur"@0xffffff8008a6def0), that is the text PLUS the
		 * terminator; the copy towards user space is FOURTEEN
		 * ("321f0be2 orr"@0xffffff8008a6df34), that is without it.
		 */
		strcpy(szBuf, "2.0.6.0.191122");
		if (copy_to_user((u8 __user *)arg, szBuf,
				 strlen("2.0.6.0.191122"))) {
#line 1867
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 14, unsigned long):
#line 1872
		ILI_DBG("ioctl: get chip id\n");
		/*
		 * IDEV_C48_AT() was reconstructed from the factory kernel disassembly (0xffffff8008a6df84, 5632 bytes).
		 *
		 * The working notes -- the disassembly citations, the measurements against
		 * the factory binary and the reasoning behind each choice -- are in
		 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
		 * in the oracolo repository. They are kept in Italian, as the project's
		 * internal record.
		 */
		c48_40 = IDEV_C48_AT(40);
		c48_44 = IDEV_C48_AT(44);
		buf[0] = IDEV_C48_AT(8);
		buf[1] = c48_40;
		buf[2] = c48_44;
		if (copy_to_user((u32 __user *)arg, buf, 3 * sizeof(u32))) {
#line 1879
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 15, unsigned long):
		if (copy_from_user(szBuf, (u8 __user *)arg, 1)) {
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: netlink ctrl = %d\n", szBuf[0]);
		if (szBuf[0]) {
			idev->c530 = 1;
			ILI_DBG("ioctl: Netlink is enabled\n");
		} else {
			idev->c530 = 0;
			ILI_DBG("ioctl: Netlink is disabled\n");
		}
		ret = 0;
		break;

	case _IOWR(0x64, 16, unsigned long):
#line 1900
		ILI_DBG("ioctl: get netlink stat = %d\n", idev->c530);
		/*
		 * FACTORY DEFECT F13: it copies FOUR bytes
		 * ("321e03e2 orr"@0xffffff8008a6e0f0) starting from a field that
		 * occupies ONE ("39484903 ldrb"@0xffffff8008a6e0a4 and
		 * "39084903 strb"@0xffffff8008a6d434 in group H treat it as a byte):
		 * three bytes of the structure end up in user space. Reproduced.
		 */
		if (copy_to_user((u8 __user *)arg, &idev->c530, 4)) {
#line 1902
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 17, unsigned long):
		if (copy_from_user(szBuf, (u8 __user *)arg, sizeof(u32))) {
#line 1908
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: switch fw format = %d\n", szBuf[0]);
		switch (szBuf[0]) {
		case 0:
			/* "3949ed08 ldrb"@0xffffff8008a6e198: the byte at
			 * +635. */
			if (idev->c635) {
				if (ilitek_set_tp_data_len(6) < 0) {
#line 1916
					ILI_ERR("Failed to set demo len from gesture mode\n");
					ret = -ENOTTY;
					break;
				}
			} else {
				if (ilitek_tddi_switch_tp_mode(0) < 0) {
#line 1921
					ILI_ERR("Failed to switch demo mode\n");
					ret = -ENOTTY;
					break;
				}
			}
			ret = 0;
			break;
		case 1:
			if (ilitek_tddi_switch_tp_mode(1) < 0) {
#line 1927
				ILI_ERR("Failed to switch test mode\n");
				ret = -ENOTTY;
				break;
			}
			ret = 0;
			break;
		case 2:
			/*
			 * `+635` is read TWICE, and the second read comes
			 * AFTER `ilitek_tddi_switch_tp_mode`: it is the source that
			 * re-reads it ("3949ed08 ldrb"@0xffffff8008a6ee6c after
			 * "97ff928b bl"@0xffffff8008a6ee60).
			 */
			if (!idev->c635 && idev->c320) {
				if (ilitek_tddi_switch_tp_mode(0) < 0) {
#line 1933
					ILI_ERR("Failed to switch demo mode\n");
					ret = -ENOTTY;
					break;
				}
			}
			if (idev->c635) {
				if (ilitek_set_tp_data_len(7) < 0) {
#line 1941
					ILI_ERR("Failed to set debug len from gesture mode\n");
					ret = -ENOTTY;
					break;
				}
			} else {
				if (ilitek_set_tp_data_len(1) < 0) {
#line 1946
					ILI_ERR("Failed to set debug len\n");
					ret = -ENOTTY;
					break;
				}
			}
			ret = 0;
			break;
		default:
#line 1951
			ILI_ERR("Unknown TP mode ctrl\n");
			ret = -ENOTTY;
			break;
		}
		break;

	case _IOWR(0x64, 18, unsigned long):
#line 1956
		ILI_DBG("ioctl: current firmware mode = %d", idev->c320);
		if (copy_to_user((u8 __user *)arg, &idev->c320,
				 sizeof(idev->c320))) {
#line 1958
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 19, u32):
		if (copy_from_user(szBuf, (u8 __user *)arg, 1)) {
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: switch ice mode = %d", szBuf[0]);
		if (szBuf[0]) {
			idev->c712 = 1;
			ILI_DBG("ioctl: set ice mode enabled\n");
		} else {
			idev->c712 = 0;
			ILI_DBG("ioctl: set ice mode disabled\n");
		}
		ret = 0;
		break;

	case _IOWR(0x64, 20, unsigned long):
		/*
		 * NO `printk` here: the byte at offset 0 of `idev->c40`
		 * ("39400108 ldrb"@0xffffff8008a6e258) copied into a byte
		 * on the stack and then out to user space.
		 */
		if_to_user = *(u8 *)idev->c40;
		if (copy_to_user((u8 __user *)arg, &if_to_user, 1)) {
#line 1981
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 21, u32):
#line 1986
		ILI_DBG("ioctl: dump flash data\n");
		/*
		 * "320003e2 orr"@0xffffff8008a6e994 is the third argument set to 1,
		 * the first two are zero.
		 */
		ret = ilitek_tddi_fw_dump_flash_data(0, 0, true);
		if (ret < 0)
#line 1989
			ILI_ERR("ioctl: Failed to dump flash data\n");
		break;

	case _IOWR(0x64, 22, unsigned long):
		if (copy_from_user(szBuf, (u8 __user *)arg, 1)) {
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: fw UART  = %d\n", szBuf[0]);
		ilitek_tddi_fw_uart_ctrl(szBuf[0]);
		/* "3949d908 ldrb"@0xffffff8008a6e330: the byte at +630. */
		if_to_user = idev->c630;
		if (copy_to_user((u8 __user *)arg, &if_to_user, 1)) {
#line 2005
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 23, unsigned long):
#line 2010
		ILI_DBG("ioctl: get panel resolution\n");
		/* "7941e909 ldrh"@0xffffff8008a6e3b4 (+244) e
		 * "7941ed08 ldrh"@0xffffff8008a6e3b8 (+246). */
		buf[0] = idev->c244;
		buf[1] = idev->c246;
		if (copy_to_user((u32 __user *)arg, buf, 2 * sizeof(u32))) {
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 24, unsigned long):
#line 2021
		ILI_DBG("ioctl: get tp info\n");
		buf[0] = idev->c240;
		buf[1] = idev->c242;
		buf[2] = idev->c236;
		buf[3] = idev->c238;
		buf[4] = idev->c248;
		buf[5] = idev->c249;
		buf[6] = idev->c250;
		buf[7] = idev->c251;
		if (copy_to_user((u32 __user *)arg, buf, 8 * sizeof(u32))) {
#line 2033
			ILI_ERR("Failed to copy driver ver to user space\n");
			ret = -ENOTTY;
		}
		break;

	case _IOWR(0x64, 25, unsigned long):
		/*
		 * Level \x016 (KERN_INFO) and NO diagnostic guard: it is
		 * an ILI_INFO, not an ILI_DBG.
		 */
#line 2038
		ILI_INFO("Not supported in this version\n");
		ret = -ENOTTY;
		break;

	case _IOWR(0x64, 26, unsigned long):
		if (copy_from_user(szBuf, (u8 __user *)arg, 3)) {
#line 2043
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: page = %x, reg = %x, data = %x\n",
#line 2047
			szBuf[0], szBuf[1], szBuf[2]);
		ilitek_tddi_ic_set_ddi_reg_onepage(szBuf[0], szBuf[1],
						   szBuf[2]);
		ret = 0;
		break;

	case _IOWR(0x64, 27, unsigned long):
		if (copy_from_user(szBuf, (u8 __user *)arg, 2)) {
#line 2052
			ILI_ERR("Failed to copy data from user space\n");
			ret = -ENOTTY;
			break;
		}
		ILI_DBG("ioctl: page = %x, reg = %x\n", szBuf[0], szBuf[1]);
		/*
		 * "91000a75 add"@0xffffff8008a6e5e8: the third argument is
		 * `szBuf + 2`, and the byte that comes out is read back from there
		 * ("394002a3 ldrb"@0xffffff8008a6e5fc).
		 */
		ilitek_tddi_ic_get_ddi_reg_onepage(szBuf[0], szBuf[1],
						   szBuf + 2);
#line 2058
		ILI_DBG("ioctl: data = %x\n", szBuf[2]);
		if (copy_to_user((u8 __user *)arg, szBuf + 2, 1)) {
			ILI_ERR("Failed to copy data to user space\n");
			ret = -ENOTTY;
		}
		break;

	default:
		/*
		 * FACTORY DEFECT F14: the `default` prints nothing.
		 * The block at 0xffffff8008a6eeb4 is reached directly by the
		 * two `b.hi` ("54008808 b.hi"@0xffffff8008a6ddb4) and by all
		 * the missing entries of the two tables, and it contains no
		 * `bl <printk>`. Reproduced.
		 */
		ret = -ENOTTY;
		break;
	}

	kfree(szBuf);

out_no_free:
	if (c528)
		ilitek_tddi_wq_ctrl(0, 1);
	if (c529)
		ilitek_tddi_wq_ctrl(1, 1);

	mutex_unlock(&idev->touch_mutex);
	return ret;
}

/*
 * netlink_reply_msg -- 0xffffff8008a7156c, 380 bytes, `T` (called from
 * 0xffffff8008a5ccf8, outside the block).
 * The size of the `skb` is `NLMSG_SPACE(size)`: the binary computes
 * `(size + 19) & ~3` ("11004e88 add"@0xffffff8008a715f0 and
 * "121e7515 and"@0xffffff8008a715f4), which is ALIGN(16 + size, 4).
 * `alloc_skb` receives priority ZERO ("2a1f03e1 mov"@0xffffff8008a71600):
 * no GFP mask. That is how it is in the binary and it is reported as it is.
 * The message type is 3 ("320007e3 orr"@0xffffff8008a71630) and the flag
 * of `netlink_unicast` is MSG_DONTWAIT ("321a03e3 orr"@0xffffff8008a71680).
 * The slot cleared at +56 of the `skb` ("b900391f str"@0xffffff8008a71668) is
 * `NETLINK_CB(skb).dst_group` under the 4.14 layout of `netlink_skb_parms`
 * (cb at +40, portid at +52, dst_group at +56): assigning the NAME is a
 * reading of the known layout; what is measured is "a write of zero at
 * +56 of the skb".
 */
void netlink_reply_msg(void *raw, int size)
{
	int ret;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;

#line 2176
	ILI_INFO("The size of data being sent to user = %d\n", size);
	ILI_INFO("pid = %d\n", netlink_pid);
	ILI_INFO("Netlink is enable = %d\n", idev->c530);

	if (idev->c530) {
		skb = alloc_skb(NLMSG_SPACE(size), 0);
		netlink_skb = skb;
		if (!skb) {
			ILI_ERR("Failed to allocate new skb\n");
			return;
		}

		nlh = nlmsg_put(skb, 0, 0, 3, size, 0);
		netlink_nlh = nlh;
		NETLINK_CB(netlink_skb).dst_group = 0;
		memcpy(NLMSG_DATA(nlh), raw, size);
		ret = netlink_unicast(netlink_sk, netlink_skb, netlink_pid,
				      MSG_DONTWAIT);
		if (ret < 0)
#line 2196
			ILI_ERR("Failed to send data back to user\n");
	}
}

/*
 * netlink_recv_msg -- 0xffffff8008a72120, 252 bytes, `t`.
 * It is the `.input` of the netlink configuration: the ONLY relocation in
 * the 48 bytes at 0xffffff8008f7ed00 is at +8 and points here.
 * The pid comes from `nlh->nlmsg_pid`, at offset 12 of the header
 * ("b9400d03 ldr"@0xffffff8008a721ac), and ends up in the global at
 * 0xffffff800a1006c4 ("b906c6c3 str"@0xffffff8008a721b4).
 */
static void netlink_recv_msg(struct sk_buff *skb)
{
	netlink_pid = 0;

#line 2204
	ILI_INFO("Netlink = %d\n", idev->c530);

	netlink_nlh = (struct nlmsghdr *)skb->data;
	ILI_INFO("Received a request from client: %s, %d\n",
		 (char *)NLMSG_DATA(netlink_nlh),
		 (int)strlen((char *)NLMSG_DATA(netlink_nlh)));

	netlink_pid = netlink_nlh->nlmsg_pid;
#line 2214
	ILI_INFO("the pid of sending process = %d\n", netlink_pid);

	if (netlink_pid != 0) {
#line 2218
		ILI_ERR("The channel of Netlink has been established successfully !\n");
		idev->c530 = 1;
	} else {
		ILI_ERR("Failed to establish the channel between kernel and user space\n");
		idev->c530 = 0;
	}
}

/*
 * THE THIRTEEN `file_operations`, in the order they sit in memory
 * (0xffffff8009989500, stride 240 = sizeof(struct file_operations)), that
 * is, in source order. Slot +0 (`.owner`) is not relocated in any of the
 * thirteen.
 */
static struct file_operations proc_mp_lcm_on_test_fops = {
	.read = ilitek_node_mp_lcm_on_test_read,
};

static struct file_operations proc_mp_lcm_off_test_fops = {
	.read = ilitek_node_mp_lcm_off_test_read,
};

static struct file_operations proc_debug_message_fops = {
	.read = ilitek_proc_debug_message_read,
};

static struct file_operations proc_debug_message_switch_fops = {
	.read = ilitek_proc_debug_switch_read,
};

static struct file_operations proc_ioctl_fops = {
	.unlocked_ioctl = ilitek_node_ioctl,
	.compat_ioctl = ilitek_node_compat_ioctl,
	.write = ilitek_node_ioctl_write,
};

static struct file_operations proc_fw_upgrade_fops = {
	.read = ilitek_node_fw_upgrade_read,
};

static struct file_operations proc_fw_process_fops = {
	.read = ilitek_proc_fw_process_read,
};

static struct file_operations proc_get_delta_data_fops = {
	.read = ilitek_proc_get_delta_data_read,
};

static struct file_operations proc_fw_get_raw_data_fops = {
	.read = ilitek_proc_fw_get_raw_data_read,
};

static struct file_operations proc_rw_tp_reg_fops = {
	.read = ilitek_proc_rw_tp_reg_read,
	.write = ilitek_proc_rw_tp_reg_write,
};

static struct file_operations proc_fw_pc_counter_fops = {
	.read = ilitek_proc_fw_pc_counter_read,
};

static struct file_operations proc_get_debug_mode_data_fops = {
	.read = ilitek_proc_get_debug_mode_data_read,
	.write = ilitek_proc_get_debug_mode_data_write,
};

static struct file_operations proc_debug_level_fops = {
	.read = ilitek_proc_debug_level_read,
};

/*
 * THE TABLE OF THE THIRTEEN NODES, 13 x 32 bytes at 0xffffff800998a130.
 * The field names are `c<offset>` because the binary does not name them;
 * the comment states what IS MEASURED about each, at its point of use.
 */
struct ilitek_proc_node {
	/* +0  1st argument of proc_create: the node name. */
	char *c0;
	/* +8  receives the result of proc_create. */
	struct proc_dir_entry *c8;
	/* +16 4th argument of proc_create. */
	struct file_operations *c16;
	/* +24 one byte: 1 if proc_create returned non-NULL, 0 otherwise. */
	bool c24;
};

static struct ilitek_proc_node proc_table[13] = {
	{"ioctl", NULL, &proc_ioctl_fops, false},
	{"fw_process", NULL, &proc_fw_process_fops, false},
	{"fw_upgrade", NULL, &proc_fw_upgrade_fops, false},
	{"debug_level", NULL, &proc_debug_level_fops, false},
	{"mp_lcm_on_test", NULL, &proc_mp_lcm_on_test_fops, false},
	{"mp_lcm_off_test", NULL, &proc_mp_lcm_off_test_fops, false},
	{"debug_message", NULL, &proc_debug_message_fops, false},
	{"debug_message_switch", NULL, &proc_debug_message_switch_fops, false},
	{"fw_pc_counter", NULL, &proc_fw_pc_counter_fops, false},
	{"show_delta_data", NULL, &proc_get_delta_data_fops, false},
	{"show_raw_data", NULL, &proc_fw_get_raw_data_fops, false},
	{"get_debug_mode_data", NULL, &proc_get_debug_mode_data_fops, false},
	{"rw_tp_reg", NULL, &proc_rw_tp_reg_fops, false},
};

/*
 * netlink_init -- it does NOT exist as a symbol: the map does not hold it.
 * The binary gives it away through its own `__func__` ("netlink_init" at
 * 0xffffff800924a09d) inside `ilitek_tddi_node_init`, at lines 2240 and
 * 2243. It has to be written as a separate function and left to inline.
 * The netlink unit is 21 ("528002a1 mov"@0xffffff8008a717d0); the
 * configuration is 48 bytes copied from the constant at 0xffffff8008f7ed00
 * ("a9402909 ldp"@0xffffff8008a717b8), with a single relocation, at +8.
 */
static void netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = netlink_recv_msg,
	};

	netlink_sk = netlink_kernel_create(&init_net, 21, &cfg);

#line 2240
	ILI_INFO("Initialise Netlink and create its socket\n");

	if (!netlink_sk)
		ILI_ERR("Failed to create nelink socket\n");
}

/*
 * ilitek_tddi_node_init() was reconstructed from the factory kernel disassembly (0xffffff8008a716e8, 372 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
void ilitek_tddi_node_init(void)
{
	int i;

	proc_dir = proc_mkdir("ilitek", NULL);

	for (i = 0; i < 13; i++) {
		proc_table[i].c8 = proc_create(proc_table[i].c0, 0644,
					       proc_dir, proc_table[i].c16);
		if (proc_table[i].c8 == NULL) {
			proc_table[i].c24 = false;
			ILI_ERR("Failed to create %s under /proc\n",
#line 2261
				proc_table[i].c0);
		} else {
			proc_table[i].c24 = true;
			ILI_INFO("Succeed to create %s under /proc\n",
				 proc_table[i].c0);
		}
	}

	netlink_init();
}

/*
 * ilitek_proc_debug_switch_read -- 0xffffff8008a6c798, 364 bytes, `t`.
 * It calls `ilitek_debug_node_buff_control` OUT OF LINE
 * ("94001419 bl"@0xffffff8008a6c7f8) with the argument `!idev->c556`
 * ("1a9f17e0 cset"@0xffffff8008a6c7f4); it is the only non-inlined call
 * site, and divergence H5 explains why.
 */
static ssize_t ilitek_proc_debug_switch_read(struct file *filp,
					     char __user *buff, size_t size,
					     loff_t *pos)
{
	int len = 0;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	mutex_lock(&idev->debug_mutex);

	ilitek_debug_node_buff_control(!idev->c556);

	len = snprintf(g_user_buf, 4096, "dnp : %s\n",
		       idev->c556 ? "Enable" : "Disable");

	*pos = len;

	if (copy_to_user(buff, g_user_buf, len))
#line 701
		ILI_ERR("Failed to copy data to user space\n");

	mutex_unlock(&idev->debug_mutex);
	return len;
}

/*
 * ilitek_proc_fw_process_read -- 0xffffff8008a6fb24, 280 bytes, `t`.
 * DEFECT F1: the source of `copy_to_user` is NOT the formatted buffer but
 * `&idev->c444` ("9106f116 add"@0xffffff8008a6fbb8 computes idev+444, and
 * "aa1603e1 mov"@0xffffff8008a6fbfc passes it as the source). Reproduced.
 * The length is ZERO-EXTENDED ("2a1503f5 mov"@0xffffff8008a6fbac, that is
 * `mov w21,w21`): it is a `u32`, not an `int`.
 */
static ssize_t ilitek_proc_fw_process_read(struct file *filp,
					   char __user *buff, size_t size,
					   loff_t *pos)
{
	u32 len = 0;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	len = snprintf(g_user_buf, 4096, "%02d\n", idev->c444);

#line 1042
	ILI_INFO("update status = %d\n", idev->c444);

	if (copy_to_user(buff, &idev->c444, len))
		ILI_ERR("Failed to copy data to user space\n");

	*pos = len;
	return len;
}

/*
 * ilitek_debug_node_buff_control -- 0xffffff8008a7185c, 452 bytes, `t`.
 * The large allocation is 16384 bytes ("321203e0 orr"@0xffffff8008a718d0),
 * the loops count to 1024 ("7110011f cmp"@0xffffff8008a71970), so
 * the element is 16 bytes; the per-element allocation is 2048
 * ("321503e2 orr"@0xffffff8008a71934).
 * The free keeps the ADDRESS of the pointer across the `kfree`
 * ("f8408ec0 ldr"@0xffffff8008a71918, pre-indexed with writeback): writing
 * `if (p->field) { kfree(p->field); p->field = NULL; }` makes the global be
 * re-read after the call and costs one instruction per site -- measured,
 * 464 bytes against 456.
 * The exit is ALWAYS -ENOMEM ("12800160 mov"@0xffffff8008a71a0c), even on the
 * power-down path where no error occurred: defect F7, reproduced.
 * WITH THE FACTORY COMPILER it measures exactly 452 bytes (with r383902 it
 * measured 456: rotation of the freeing loops).
 */
static int ilitek_debug_node_buff_control(bool enable)
{
	int i;

	idev->c556 = enable;
#line 233
	ILI_INFO("Debug buf ctrl = %s\n", enable ? "Enable" : "Disable");

	if (!enable)
		goto out;

	idev->c560 = 0;
	idev->c564 = 0;
	ipio_kfree((void **)&idev->c592);

	idev->c592 = kzalloc(1024 * sizeof(struct ilitek_dbl), GFP_KERNEL);
	if (IS_ERR(idev->c592) || idev->c592 == NULL) {
		ILI_ERR("Failed to allocate idev->dbl mem, %ld\n",
#line 241
			PTR_ERR(idev->c592));
		goto out;
	}

	for (i = 0; i < 1024; i++) {
		IDEV_DBL[i].c0 = 0;
		ipio_kfree((void **)&IDEV_DBL[i].c8);
		IDEV_DBL[i].c8 = kzalloc(2048, GFP_KERNEL);
		if (IS_ERR(IDEV_DBL[i].c8) || IDEV_DBL[i].c8 == NULL) {
			ILI_ERR("Failed to allocate dbl[%d] mem, %ld\n", i,
				PTR_ERR(IDEV_DBL[i].c8));
			goto out;
		}
	}
	return 0;

out:
	for (i = 0; i < 1024; i++) {
		IDEV_DBL[i].c0 = 0;
		ipio_kfree((void **)&IDEV_DBL[i].c8);
	}
	ipio_kfree((void **)&idev->c592);
	return -ENOMEM;
}

/*
 * ilitek_proc_fw_pc_counter_read -- 0xffffff8008a70cd0, 308 bytes, `t`.
 * The two values printed are read TOGETHER
 * ("295c9103 ldp"@0xffffff8008a70d28): they are the fields at +228 and +232
 * of `idev`, adjacent and 32-bit. `ilitek_tddi_ic_get_pc_counter` is called
 * without preparing any argument register.
 */
static ssize_t ilitek_proc_fw_pc_counter_read(struct file *filp,
					      char __user *buff, size_t size,
					      loff_t *pos)
{
	int len = 0;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	ilitek_tddi_ic_get_pc_counter();

	len = snprintf(g_user_buf, 4096, "pc = 0x%x, latch = 0x%x\n",
		       idev->c228, idev->c232);

	if (copy_to_user(buff, g_user_buf, len))
#line 565
		ILI_ERR("Failed to copy data to user space\n");

	*pos += len;
	return len;
}

/*
 * ilitek_proc_debug_level_read -- 0xffffff8008a713f4, 376 bytes, `t`.
 * The diagnostic flag is FLIPPED with an `eor #1`
 * ("52000104 eor"@0xffffff8008a7145c) and the new value is also the fourth
 * argument of the message; the string chosen uses the OLD value instead
 * ("9a970303 csel"@0xffffff8008a71460), which after the flip is equivalent.
 * `*pos` is INCREMENTED, not assigned ("8b130108 add"@0xffffff8008a714a4).
 */
static ssize_t ilitek_proc_debug_level_read(struct file *filp,
					    char __user *buff, size_t size,
					    loff_t *pos)
{
	int len = 0;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	/*
	 * `^= 1` and NOT `= !ilitek_dbg_en`: the factory flips the byte with
	 * an EOR ("52000104 eor"@0xffffff8008a7145c), and on a `u8` the form
	 * `!x` does not produce that -- clang cannot prove the value lies in
	 * {0,1} and lowers `!x` to `cmp`+`cset`. MEASURED on the real `.o`: the
	 * size does not change (376 bytes either way), the mnemonic histogram
	 * does (`cset:+1 eor:-1`). It is the same form already used on the same
	 * global in `ilitek_node_ioctl`.
	 */
	ilitek_dbg_en ^= 1;

	ILI_INFO(" %s debug level = %x\n",
#line 1128
		 ilitek_dbg_en ? "Enable" : "Disable", ilitek_dbg_en);

	len = snprintf(g_user_buf, 4096, "debug level : %s\n",
		       ilitek_dbg_en ? "Enable" : "Disable");

	*pos += len;

	if (copy_to_user(buff, g_user_buf, len))
#line 1135
		ILI_ERR("Failed to copy data to user space\n");

	return len;
}

/*
 * file_write() was reconstructed from the factory kernel disassembly (0xffffff8008a71a20, 404 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static void file_write(struct ilitek_file_buffer *file, bool new_open)
{
	struct file *f = NULL;
	mm_segment_t fs;
	loff_t pos;

	if (file->c0 == NULL) {
#line 193
		ILI_ERR("str is invaild\n");
		return;
	}

	if (file->c8 == NULL) {
		ILI_ERR("file name is invaild\n");
		return;
	}

	if (file->c136 >= file->c144) {
		ILI_ERR("Saved to file length is too long !, %d\n", file->c136);
		return;
	}

	if (new_open)
		f = filp_open(file->c8, O_WRONLY | O_CREAT | O_TRUNC, 644);
	else
		f = filp_open(file->c8, O_WRONLY | O_CREAT | O_APPEND, 644);

	if (IS_ERR(f) || f == NULL) {
		ILI_ERR("Failed to open %s file\n", file->c8);
		return;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_write(f, file->c0, file->c136, &pos);
	set_fs(fs);
	filp_close(f, NULL);

	file->c140 += file->c136;
}

/*
 * ilitek_proc_get_debug_mode_data_read() was reconstructed from the factory kernel disassembly (0xffffff8008a70e04, 700 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t ilitek_proc_get_debug_mode_data_read(struct file *filp,
						    char __user *buff,
						    size_t size, loff_t *pos)
{
	struct ilitek_file_buffer csv;

	if (*pos != 0)
		return 0;

	memset(csv.c8, 0, sizeof(csv.c8));
	strcpy(csv.c8, "/sdcard/ILITEK_log.csv");
	csv.c136 = 0;
	csv.c140 = 0;
	csv.c144 = 10240;

	csv.c0 = vmalloc(csv.c144);
	if (IS_ERR(csv.c0) || csv.c0 == NULL) {
#line 827
		ILI_ERR("Failed to allocate CSV mem\n");
		goto out;
	}

	ILI_INFO("Get Raw data %d frame\n", idev->c600);
	ILI_INFO("Get Delta data %d frame\n", idev->c604);

	csv.c136 += snprintf(csv.c0 + csv.c136, csv.c144 - csv.c136,
			     "Get Raw data %d frame\n", idev->c600);
	csv.c136 += snprintf(csv.c0 + csv.c136, csv.c144 - csv.c136,
			     "Get Delta data %d frame\n", idev->c604);
	file_write(&csv, true);

	if (ilitek_set_tp_data_len(1) < 0) {
		ILI_ERR("Failed to set tp data length\n");
		goto out;
	}

	csv.c136 = 0;
	memset(csv.c0, 0, csv.c144);
	csv.c136 += snprintf(csv.c0 + csv.c136, csv.c144 - csv.c136,
			     "\n\n=======Raw data=======");
	file_write(&csv, false);
	if (debug_mode_get_data(&csv, 8, idev->c600) < 0)
		goto out;

	csv.c136 = 0;
	memset(csv.c0, 0, csv.c144);
	csv.c136 += snprintf(csv.c0 + csv.c136, csv.c144 - csv.c136,
			     "\n\n=======Delta data=======");
	file_write(&csv, false);
	if (debug_mode_get_data(&csv, 3, idev->c604) < 0)
		goto out;

	if (ilitek_set_tp_data_len(0) < 0)
#line 867
		ILI_ERR("Failed to set tp data length\n");

out:
	if (csv.c0 != NULL) {
		vfree(csv.c0);
		csv.c0 = NULL;
	}
	return 0;
}

/*
 * `dev_mkdir` does NOT exist as a symbol: the map does not hold it. The
 * binary gives it away through its own `__func__` -- "dev_mkdir"@0xffffff8009247c83
 * with `__LINE__` = 347 -- inside the TWO functions below
 * ("91320c21 add"@0xffffff8008a6bc50 and "91320c21 add"@0xffffff8008a6bf74).
 * It therefore has to be written as a separate function and left to inline.
 */
static int dev_mkdir(char *name, umode_t mode)
{
	int err;
	mm_segment_t fs;

#line 347
	ILI_INFO("mkdir: %s\n", name);

	fs = get_fs();
	set_fs(KERNEL_DS);
	err = sys_mkdir(name, mode);
	set_fs(fs);

	return err;
}

#define MP_TEST_TAIL(BUF, LEN)						\
	switch ((BUF)[1]) {						\
	case 0x65:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"Protocol version isn't matched, abort!"); \
		break;							\
	case 0x67:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"Not found ini file, abort!");		\
		break;							\
	case 0x68:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"Failed to get timing info, abort!");	\
		break;							\
	case 0x6b:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"Failed to allocated memory, abort!");	\
		break;							\
	case 0x6f:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"Failed to switch MP mode, abort!");	\
		break;							\
	case 0x70:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"FW still upgrading, abort!");		\
		break;							\
	case 0x71:							\
		LEN += snprintf((BUF) + LEN, 4096 - LEN, "%s\n",	\
				"MP formula is null, abort!");		\
		break;							\
	default:							\
		break;							\
	}

/*
 * ilitek_node_mp_lcm_on_test_read() was reconstructed from the factory kernel disassembly (0xffffff8008a6bbd4, 804 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t ilitek_node_mp_lcm_on_test_read(struct file *filp,
					       char __user *buff, size_t size,
					       loff_t *pos)
{
	int ret, len = 0;
	u8 esd = idev->c528, bat = idev->c529;

	if (*pos != 0)
		return 0;

#line 916
	ILI_INFO("Run MP test with LCM on\n");

	mutex_lock(&idev->touch_mutex);

	if (dev_mkdir("/sdcard/ilitek_mp_lcm_on_log", 0644) != 0)
#line 922
		ILI_ERR("Failed to create directory for mp_test\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 0);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 0);

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	idev->c624 = 0;
	ret = ilitek_tddi_mp_test_handler(g_user_buf, true);

	ILI_INFO("MP TEST %s, Error code = %d\n",
#line 933
		 (ret < 0) ? "FAIL" : "PASS", ret);

	g_user_buf[0] = 3;
	g_user_buf[1] = (ret < 0) ? -ret : ret;

	len = idev->c624 + 2;
	len += snprintf(g_user_buf + len, 4096 - len, "MP TEST %s\n",
			(ret < 0) ? "FAIL" : "PASS");

	MP_TEST_TAIL(g_user_buf, len)

	if (copy_to_user(buff, g_user_buf, len))
#line 957
		ILI_ERR("Failed to copy data to user space\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 1);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 1);

	*pos += len;

	mutex_unlock(&idev->touch_mutex);
	return len;
}

static ssize_t ilitek_node_mp_lcm_off_test_read(struct file *filp,
						char __user *buff, size_t size,
						loff_t *pos)
{
	int ret, len = 0;
	u8 esd = idev->c528, bat = idev->c529;

	if (*pos != 0)
		return 0;

#line 977
	ILI_INFO("Run MP test with LCM off\n");

	mutex_lock(&idev->touch_mutex);

	if (dev_mkdir("/sdcard/ilitek_mp_lcm_off_log", 0644) != 0)
#line 984
		ILI_ERR("Failed to create directory for mp_test\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 0);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 0);

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	idev->c624 = 0;
	ret = ilitek_tddi_mp_test_handler(g_user_buf, false);

	ILI_INFO("MP TEST %s, Error code = %d\n",
#line 995
		 (ret < 0) ? "FAIL" : "PASS", ret);

	g_user_buf[0] = 3;
	g_user_buf[1] = (ret < 0) ? -ret : ret;

	len = idev->c624 + 2;
	len += snprintf(g_user_buf + len, 4096 - len, "MP TEST %s\n",
			(ret < 0) ? "FAIL" : "PASS");

	MP_TEST_TAIL(g_user_buf, len)

	if (copy_to_user(buff, g_user_buf, len))
#line 1019
		ILI_ERR("Failed to copy data to user space\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 1);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 1);

	*pos += len;

	mutex_unlock(&idev->touch_mutex);
	return len;
}

/*
 * ilitek_node_fw_upgrade_read() was reconstructed from the factory kernel disassembly (0xffffff8008a6f868, 700 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t ilitek_node_fw_upgrade_read(struct file *filp,
					   char __user *buff, size_t size,
					   loff_t *pos)
{
	int ret, len = 0;
	u8 esd = idev->c528, bat = idev->c529;

	if (*pos != 0)
		return 0;

#line 1059
	ILI_INFO("Preparing to upgarde firmware\n");

	mutex_lock(&idev->touch_mutex);

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	if (esd)
		ilitek_tddi_wq_ctrl(0, 0);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 0);

	idev->c631 = 1;
	idev->c643 = 1;
	/*
	 * HEADER DELTA: `ilitek.h` declares
	 * `int ilitek_tddi_fw_upgrade_handler(void)`, but this site prepares
	 * x0 = NULL ("aa1f03e0 mov"@0xffffff8008a6f90c) right before the
	 * call ("97ff9058 bl"@0xffffff8008a6f918): the function takes ONE
	 * argument, a pointer, and here it is NULL. The cast reproduces the
	 * binary without touching the shared header.
	 */
	ret = ((int (*)(void *))ilitek_tddi_fw_upgrade_handler)(NULL);
	idev->c643 = 0;
	idev->c631 = 0;

	g_user_buf[0] = 0;
	g_user_buf[1] = (ret < 0) ? -ret : ret;

	len = 2;
	len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
			(g_user_buf[1] == 0) ? "Upgrade firmware = PASS" :
					       "Upgrade firmware = FAIL");

	switch (g_user_buf[1]) {
	case 0x0c:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to allocate memory, abort!");
		break;
	case 0x72:
		g_user_buf[0] = 0xff;
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to convert hex/ili file, abort!");
		break;
	case 0x73:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to operate ice mode, abort!");
		break;
	case 0x74:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to operate watch dog, abort!");
		break;
	case 0x75:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"CRC not matched, abort!");
		break;
	case 0x76:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to do reset, abort!");
		break;
	case 0x77:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to erase flash, abort!");
		break;
	case 0x78:
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to program flash, abort!");
		break;
	default:
		break;
	}

	if (copy_to_user(buff, g_user_buf, len))
#line 1107
		ILI_ERR("Failed to copy data to user space\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 1);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 1);

	*pos += len;

	mutex_unlock(&idev->touch_mutex);
	return len;
}

/*
 * ilitek_proc_rw_tp_reg_write() was reconstructed from the factory kernel disassembly (0xffffff8008a70a54, 636 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t ilitek_proc_rw_tp_reg_write(struct file *filp,
					   const char __user *buff,
					   size_t size, loff_t *pos)
{
	char buf[256] = {0};
	char *token = NULL, *cur = NULL;
	u32 i = 0;
	ssize_t ret = size;

	if (size - 1 > 256) {
#line 657
		ILI_ERR("ERROR! input length is larger than local buffer\n");
		return -1;
	}

	mutex_lock(&idev->touch_mutex);

	if (buff != NULL) {
		if (copy_from_user(buf, buff, size - 1)) {
			ILI_INFO("Failed to copy data from user space\n");
			ret = -1;
			goto out;
		}
	}

	cur = buf;
	token = strsep(&cur, ",");
	while (token != NULL) {
		rw_reg[i] = str2hex(token);
#line 673
		ILI_INFO("rw_reg[%d] = 0x%x\n", i, rw_reg[i]);
		i++;
		token = strsep(&cur, ",");
	}

out:
	mutex_unlock(&idev->touch_mutex);
	return ret;
}

/*
 * ilitek_proc_rw_tp_reg_read() was reconstructed from the factory kernel disassembly (0xffffff8008a70640, 1044 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t ilitek_proc_rw_tp_reg_read(struct file *filp, char __user *buff,
					  size_t size, loff_t *pos)
{
	int ret, len = 0;
	u32 type, stop_mcu, addr, write_data, write_len, read_data;
	u8 esd = idev->c528, bat = idev->c529;

	if (*pos != 0)
		return 0;

	stop_mcu = rw_reg[0];
	type = rw_reg[1];
	addr = rw_reg[2];
	write_data = rw_reg[3];
	write_len = rw_reg[4];

#line 588
	ILI_INFO("stop_mcu = %d\n", stop_mcu);

	if (esd)
		ilitek_tddi_wq_ctrl(0, 0);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 0);

	mutex_lock(&idev->touch_mutex);

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	ret = ilitek_ice_mode_ctrl(1, !stop_mcu);
	if (ret < 0) {
		ILI_ERR("Failed to enter ICE mode, ret = %d\n", ret);
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to enter ICE mode");
	}

	if (type) {
		ret = ilitek_ice_mode_write(addr, write_data, write_len);
		if (ret < 0) {
			ILI_ERR("Write data error\n");
			len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
					"Write data error");
		}
		ILI_INFO("[WRITE]:addr = 0x%06x, write = 0x%08x, len = %d byte\n",
#line 623
			 addr, write_data, write_len);
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
				"WRITE:addr = 0x%06x, write = 0x%08x, len =%d byte\n",
				addr, write_data, write_len);
	} else {
		ret = ilitek_ice_mode_read(addr, &read_data, sizeof(u32));
		if (ret < 0) {
			ILI_ERR("Read data error\n");
			len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
					"Read data error");
		}
		ILI_INFO("[READ]:addr = 0x%06x, read = 0x%08x\n", addr,
#line 615
			 read_data);
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
				"READ:addr = 0x%06x, read = 0x%08x\n", addr,
				read_data);
	}

	ret = ilitek_ice_mode_ctrl(0, !stop_mcu);
	if (ret < 0) {
		ILI_ERR("Failed to disable ICE mode, ret = %d\n", ret);
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%s\n",
				"Failed to disable ICE mode");
	}

	if (copy_to_user(buff, g_user_buf, len))
#line 638
		ILI_ERR("Failed to copy data to user space\n");

	if (esd)
		ilitek_tddi_wq_ctrl(0, 1);
	if (bat)
		ilitek_tddi_wq_ctrl(1, 1);

	*pos += len;

	mutex_unlock(&idev->touch_mutex);
	return len;
}

/*
 * debug_mode_get_data() was reconstructed from the factory kernel disassembly (0xffffff8008a71bb4, 1388 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static int debug_mode_get_data(struct ilitek_file_buffer *csv, u8 type,
			       u32 frame_count)
{
	int ret = 0, j;
	s16 temp;
	u16 i;
	u8 cmd[2] = {0};
	u8 row, col;
	u8 *data;

	ilitek_debug_node_buff_control(false);

	row = idev->c249;
	col = idev->c248;
	idev->c560 = 0;

	mutex_lock(&idev->touch_mutex);

	cmd[0] = 0xFA;
	cmd[1] = type;
	ret = idev->c776(cmd, 2);

	if (ilitek_debug_node_buff_control(true) < 0) {
#line 289
		ILI_ERR("Failed to allocate debug buf\n");
		ret = -ENOMEM;
		goto out;
	}

	mutex_unlock(&idev->touch_mutex);

	if (ret < 0) {
		ILI_ERR("Write 0xFA,0x%x failed\n", type);
		return ret;
	}

	for (i = 0; i < frame_count; i++) {
		csv->c136 = 0;

		ILI_INFO("frame = %d,index = %d,count = %d\n", i, idev->c564,
#line 303
			 idev->c560);

		if (!wait_event_interruptible_timeout(IDEV_INQ,
						      IDEV_DBL[idev->c564].c0,
						      msecs_to_jiffies(3000))) {
#line 305
			ILI_ERR("debug mode get data timeout!\n");
			goto out;
		}

		mutex_lock(&idev->touch_mutex);

		memset(csv->c0, 0, csv->c144);

		csv->c136 += snprintf(csv->c0 + csv->c136,
				      csv->c144 - csv->c136, "\n\nFrame%d,", i);

		for (j = 0; j < col; j++)
			csv->c136 += snprintf(csv->c0 + csv->c136,
					      csv->c144 - csv->c136,
					      "[X%d] ,", j);

		data = (u8 *)IDEV_DBL[idev->c564].c8 + 35;

		for (j = 0; j < col * row; j++) {
			temp = (data[0] << 8) | data[1];
			if (j % col == 0)
				csv->c136 += snprintf(csv->c0 + csv->c136,
						      csv->c144 - csv->c136,
						      "\n[Y%d] ,", j / col);
			csv->c136 += snprintf(csv->c0 + csv->c136,
					      csv->c144 - csv->c136, "%d, ",
					      temp);
			data += 2;
		}

		csv->c136 += snprintf(csv->c0 + csv->c136,
				      csv->c144 - csv->c136, "\n[X] ,");

		for (j = 0; j < col + row; j++) {
			temp = (data[0] << 8) | data[1];
			if (j == col)
				csv->c136 += snprintf(csv->c0 + csv->c136,
						      csv->c144 - csv->c136,
						      "\n[Y] ,");
			csv->c136 += snprintf(csv->c0 + csv->c136,
					      csv->c144 - csv->c136, "%d, ",
					      temp);
			data += 2;
		}

		file_write(csv, false);

		mutex_unlock(&idev->touch_mutex);

		IDEV_DBL[idev->c564].c0 = 0;
		idev->c564 = (idev->c564 + 1) % 1024;
	}

out:
	ilitek_debug_node_buff_control(false);
	return ret;
}

/*
 * ilitek_proc_get_debug_mode_data_write() was reconstructed from the factory kernel disassembly (0xffffff8008a710c0, 820 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t ilitek_proc_get_debug_mode_data_write(struct file *filp,
						     const char __user *buff,
						     size_t size, loff_t *pos)
{
	u8 temp[256] = {0}, i = 0;
	char buf[256] = {0};
	char *token = NULL, *cur = NULL;
	ssize_t ret = size;

	if (size - 1 > 256) {
#line 881
		ILI_ERR("ERROR! input length is larger than local buffer\n");
		return -1;
	}

	if (buff != NULL) {
		if (copy_from_user(buf, buff, size - 1)) {
			ILI_INFO("Failed to copy data from user space\n");
			return -1;
		}
	}

	ILI_INFO("size = %d, cmd = %s\n", (int)size, buf);

	cur = buf;
	token = strsep(&cur, ",");
	while (token != NULL) {
		temp[i] = str2hex(token);
#line 896
		ILI_INFO("temp[%d] = %d\n", i, temp[i]);
		i++;
		token = strsep(&cur, ",");
	}

	idev->c600 = (temp[0] << 8) | temp[1];
	idev->c604 = (temp[2] << 8) | temp[3];
	idev->c608 = (temp[4] << 8) | temp[5];

	ILI_INFO("Raw_count = %d, Delta_count = %d, BG_count = %d\n",
#line 904
		 idev->c600, idev->c604, idev->c608);

	return ret;
}

/*
 * ilitek_proc_get_delta_data_read() was reconstructed from the factory kernel disassembly (0xffffff8008a6fc3c, 1272 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t ilitek_proc_get_delta_data_read(struct file *filp,
					       char __user *buff, size_t size,
					       loff_t *pos)
{
	s16 *delta = NULL;
	int row = 0, col = 0;
	int ret, i, j, read_length = 0, len = 0;
	u8 cmd[2] = {0};
	u8 *data = NULL;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	ilitek_tddi_wq_ctrl(0, 0);
	ilitek_tddi_wq_ctrl(1, 0);

	mutex_lock(&idev->touch_mutex);

	row = idev->c249;
	col = idev->c248;
	read_length = 4 + row * col * 2 + 1;

#line 378
	ILI_INFO("read length = %d\n", read_length);

	data = kzalloc(read_length + 1, GFP_KERNEL);
	if (IS_ERR(data) || data == NULL) {
		ILI_ERR("Failed to allocate data mem\n");
		return 0;
	}

	delta = kzalloc(5120, GFP_KERNEL);
	if (IS_ERR(delta) || delta == NULL) {
		ILI_ERR("Failed to allocate delta mem\n");
		return 0;
	}

	cmd[0] = 0xB7;
	cmd[1] = 0x1;
	ret = idev->c776(cmd, 2);
	if (ret < 0) {
		ILI_ERR("Failed to write 0xB7,0x1 command, %d\n", ret);
		goto out;
	}

	msleep(120);

	ret = idev->c784(data, read_length);
	if (ret < 0) {
#line 405
		ILI_ERR("Read debug packet header failed, %d\n", ret);
		goto out;
	}

	cmd[1] = 0x3;
	ret = idev->c776(cmd, 2);
	if (ret < 0) {
		ILI_ERR("Failed to write 0xB7,0x3 command, %d\n", ret);
		goto out;
	}

	for (i = 0; i < row * col * 2; i++)
		delta[i] = (data[i * 2 + 4] << 8) + data[i * 2 + 5];

	len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
			"======== Deltadata ========\n");
	ILI_INFO("======== Deltadata ========\n");
	len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
			"Header 0x%x ,Type %d, Length %d\n", data[0], data[1],
			(data[2] << 8) | data[3]);
	ILI_INFO("Header 0x%x ,Type %d, Length %d\n", data[0], data[1],
		 (data[2] << 8) | data[3]);

	for (i = 0; i < row; i++) {
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "[%2d] ", i + 1);
		ILI_INFO("[%2d] ", i + 1);
		for (j = 0; j < col; j++) {
			len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%5d",
					delta[i * col + j]);
			pr_cont("%5d", delta[i * col + j]);
		}
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "\n");
		pr_cont("\n");
	}

	if (copy_to_user(buff, g_user_buf, len))
		ILI_ERR("Failed to copy data to user space\n");

	*pos += len;

out:
	mutex_unlock(&idev->touch_mutex);
	ilitek_tddi_wq_ctrl(0, 1);
	ilitek_tddi_wq_ctrl(1, 1);
	kfree(data);
	kfree(delta);
	return len;
}
static ssize_t ilitek_proc_fw_get_raw_data_read(struct file *filp,
						char __user *buff, size_t size,
						loff_t *pos)
{
	s16 *rawdata = NULL;
	int row = 0, col = 0;
	int ret, i, j, read_length = 0, len = 0;
	u8 cmd[2] = {0};
	u8 *data = NULL;

	if (*pos != 0)
		return 0;

	memset(g_user_buf, 0, 4096 * sizeof(unsigned char));

	ilitek_tddi_wq_ctrl(0, 0);
	ilitek_tddi_wq_ctrl(1, 0);

	mutex_lock(&idev->touch_mutex);

	row = idev->c249;
	col = idev->c248;
	read_length = 4 + row * col * 2 + 1;

#line 476
	ILI_INFO("read length = %d\n", read_length);

	data = kzalloc(read_length + 1, GFP_KERNEL);
	if (IS_ERR(data) || data == NULL) {
		ILI_ERR("Failed to allocate data mem\n");
		goto out;
	}

	rawdata = kzalloc(5120, GFP_KERNEL);
	if (IS_ERR(rawdata) || rawdata == NULL) {
		ILI_ERR("Failed to allocate rawdata mem\n");
		goto out;
	}

	cmd[0] = 0xB7;
	cmd[1] = 0x2;
	ret = idev->c776(cmd, 2);
	if (ret < 0) {
		ILI_ERR("Failed to write 0xB7,0x2 command, %d\n", ret);
		goto out;
	}

	msleep(120);

	ret = idev->c784(data, read_length);
	if (ret < 0) {
#line 503
		ILI_ERR("Read debug packet header failed, %d\n", ret);
		goto out;
	}

	cmd[1] = 0x3;
	ret = idev->c776(cmd, 2);
	if (ret < 0) {
		ILI_ERR("Failed to write 0xB7,0x3 command, %d\n", ret);
		goto out;
	}

	for (i = 0; i < row * col * 2; i++)
		rawdata[i] = (data[i * 2 + 4] << 8) + data[i * 2 + 5];

	len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
			"======== RawData ========\n");
	ILI_INFO("======== RawData ========\n");
	len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len,
			"Header 0x%x ,Type %d, Length %d\n", data[0], data[1],
			(data[2] << 8) | data[3]);
	ILI_INFO("Header 0x%x ,Type %d, Length %d\n", data[0], data[1],
		 (data[2] << 8) | data[3]);

	for (i = 0; i < row; i++) {
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "[%2d] ", i + 1);
		ILI_INFO("[%2d] ", i + 1);
		for (j = 0; j < col; j++) {
			len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "%5d",
					rawdata[i * col + j]);
			pr_cont("%5d", rawdata[i * col + j]);
		}
		len += snprintf(g_user_buf + len, sizeof(g_user_buf) - len, "\n");
		pr_cont("\n");
	}

	if (copy_to_user(buff, g_user_buf, len))
		ILI_ERR("Failed to copy data to user space\n");

	*pos += len;

out:
	mutex_unlock(&idev->touch_mutex);
	ilitek_tddi_wq_ctrl(0, 1);
	ilitek_tddi_wq_ctrl(1, 1);
	if (data)
		kfree(data);
	if (rawdata)
		kfree(rawdata);
	return len;
}

/*
 * ilitek_proc_debug_message_read() was reconstructed from the factory kernel disassembly (0xffffff8008a6c21c, 1404 bytes).
 *
 * The working notes -- the disassembly citations, the measurements against
 * the factory binary and the reasoning behind each choice -- are in
 * docs/bringup/verbali-driver/drivers_input_touchscreen_mediatek_ilitek_e977_ilitek_node.md
 * in the oracolo repository. They are kept in Italian, as the project's
 * internal record.
 */
static ssize_t ilitek_proc_debug_message_read(struct file *filp,
					      char __user *buff, size_t size,
					      loff_t *pos)
{
	unsigned long p = *pos;
	int i, ret = 0, send_data_len = 0, need = 0;
	unsigned char *tmp = NULL;
	unsigned char tmpbuf[128] = {0};
	u8 *data;

	if (filp->f_flags & O_NONBLOCK)
		return -EAGAIN;

	mutex_lock(&idev->debug_read_mutex);

	ILI_DBG("f_count= %d, index = %d, mark = %d\n", idev->c560,
#line 726
		idev->c564, IDEV_DBL[idev->c564].c0);

	if (!wait_event_interruptible_timeout(IDEV_INQ,
					      IDEV_DBL[idev->c564].c0,
					      msecs_to_jiffies(3000))) {
#line 728
		ILI_ERR("WARNING ! there's no data received.\n");
		mutex_unlock(&idev->debug_read_mutex);
		*pos = 0;
		return 0;
	}

	mutex_lock(&idev->debug_mutex);

	tmp = vmalloc(4096);
	if (IS_ERR(tmp) || tmp == NULL) {
		ILI_ERR("buffer vmalloc error\n");
		send_data_len += snprintf(tmpbuf, 128, "buffer vmalloc error\n");
		copy_to_user(buff, tmpbuf, send_data_len);
		goto out;
	}

	if (IDEV_DBL[idev->c564].c0) {
		data = (u8 *)IDEV_DBL[idev->c564].c8;

		switch (data[0]) {
		case 0x5A:
			need = 43;
			break;
		case 0xA7:
			need = 2040;
			break;
		case 0x7A:
			/*
			 * The factory has a SINGLE `madd`
			 * ("9b09291c madd"@0xffffff8008a6c3f4) fed by a
			 * multiplier the three branches compute
			 * (1, 2 and "4 or 0"): it is the form with the variable, not
			 * the one with the expression repeated case by case.
			 * See divergence H5.
			 */
			switch (data[3] & 0x0F) {
			case 0:
			case 1:
			case 6:
				need = data[1] * data[2] * 1 + 6;
				break;
			case 2:
			case 3:
				need = data[1] * data[2] * 2 + 6;
				break;
			case 4:
			case 5:
				need = data[1] * data[2] * 4 + 6;
				break;
			default:
				need = data[1] * data[2] * 0 + 6;
				break;
			}
			break;
		default:
			break;
		}

		for (i = 0; i < need; i++) {
			send_data_len += snprintf(tmp + send_data_len, 128,
						  "%02X", data[i]);
			if (send_data_len > 4095) {
				ILI_ERR("send_data_len = %d set 4096 i = %d\n",
#line 767
					send_data_len, i);
				send_data_len = 4096;
				break;
			}
		}

		send_data_len += snprintf(tmp + send_data_len, 128, "\n\n");

		if (p == 5 || size == 4096 || size == 2048) {
			IDEV_DBL[idev->c564].c0 = 0;
			idev->c564 = (idev->c564 + 1) % 1024;
		}
	}

	if (size == 4096)
		ret = copy_to_user(buff, tmp, send_data_len);
	else
		ret = copy_to_user(buff, tmp + p, send_data_len - p);

	if (send_data_len < 1 || send_data_len > 4096) {
#line 790
		ILI_ERR("send_data_len = %d set 4096\n", send_data_len);
		send_data_len = 4096;
	}

	if (ret) {
		ILI_ERR("copy_to_user err\n");
		goto out;
	}

	*pos += send_data_len;

#line 800
	ILI_DBG("Read %d bytes(s) from %ld\n", send_data_len, p);

out:
	mutex_unlock(&idev->debug_mutex);
	mutex_unlock(&idev->debug_read_mutex);
	if (tmp)
		vfree(tmp);
	return send_data_len;
}
