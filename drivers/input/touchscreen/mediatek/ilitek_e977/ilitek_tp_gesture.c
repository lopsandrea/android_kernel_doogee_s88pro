// SPDX-License-Identifier: GPL-2.0
/*
 * The gesture switches Android uses, in /sys/class/tp_gesture/:
 *
 *   gesenable  double tap to wake, on (1) or off (0)
 *   gesmask    the other gestures, as a hexadecimal bit mask: bit n stands
 *              for the controller's gesture code 0x60 + n
 *
 * Not part of the factory kernel, unlike the rest of this directory. The
 * factory driver already has the gesture mode: with it on, suspend leaves the
 * controller watching the panel and reporting what is drawn on it. But it
 * turns into a key only the double tap, as KEY_POWER; the rest it recognises
 * and drops. And the only way to turn the mode on is a debug command on the
 * driver's proc nodes, which flips it rather than setting it.
 *
 * Here the gestures the user enables come back as keys of their own, from
 * KEY_GESTURE_BASE on: past every code the input layer defines, so that no
 * keyboard can send them. LineageOS's touchscreen gesture settings match them
 * by scan code, whatever device they come from.
 *
 * What the controller recognised when tried (codes from Ilitek's driver):
 *
 *   0x60 up   0x61 down   0x62 left   0x63 right
 *   0x64 M    0x65 W      0x66 C      0x68 V      0x69 O
 *
 * reliably, with scores of 84 to 100 out of 100. S and Z came back as 0xff,
 * an unknown gesture, and E as a C followed by a swipe right; their codes
 * (0x6a, 0x6b, 0x67) are accepted here all the same.
 *
 * Two flags decide what suspend does, and both follow "any gesture on". The
 * driver's own (c532) puts the controller in gesture mode; mtk_tpd.c's
 * g0fc0b8 is read by the panel's lcm_suspend(), which keeps the panel bias
 * (VPOS and VNEG, on the MT6370) on when it is set and cuts it otherwise.
 * This is a TDDI: touch and display share the chip, and with the bias cut
 * the controller stops listening a moment after the screen goes dark.
 * Nothing in the factory kernel ever sets g0fc0b8.
 *
 * The stock vendor image already expects gesenable -- init.mt6771.rc hands it
 * to the system user on post-fs-data -- yet no factory kernel for this panel
 * ever created it; gesmask is changed the same way by this device's init
 * script. Android reaches gesenable through the power HAL (Settings > Display
 * > Tap to wake) and gesmask through the touch HAL (LineageOS's touchscreen
 * gestures).
 */
#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "ilitek.h"
#include "ilitek_tp_gesture.h"

/* mtk_tpd.c; see above. */
extern u8 g0fc0b8;

#define GESTURE_CODE_FIRST	0x60
#define GESTURE_CODE_LAST	0x6b
#define GESTURE_COUNT		(GESTURE_CODE_LAST - GESTURE_CODE_FIRST + 1)

/* BTN_TRIGGER_HAPPY40, the last code defined, is 0x2e7. */
#define KEY_GESTURE_BASE	0x2e8

static bool double_tap;
static unsigned long gesture_mask;

/* Both flags are read on the next suspend, and c532 on the resume after it. */
static void gesture_mode_update(void)
{
	bool on = double_tap || gesture_mask;

	idev->c532 = on;
	g0fc0b8 = on;
	ILI_INFO("gesture = %d (double tap %d, mask 0x%lx)\n",
		 idev->c532, double_tap, gesture_mask);
}

bool ilitek_tp_gesture_double_tap_enabled(void)
{
	return double_tap;
}

void ilitek_tp_gesture_report(struct input_dev *in, u8 code)
{
	unsigned int key;

	if (code < GESTURE_CODE_FIRST || code > GESTURE_CODE_LAST)
		return;
	if (!test_bit(code - GESTURE_CODE_FIRST, &gesture_mask))
		return;

	key = KEY_GESTURE_BASE + (code - GESTURE_CODE_FIRST);
	input_report_key(in, key, 1);
	input_sync(in);
	input_report_key(in, key, 0);
	input_sync(in);
}

static ssize_t gesenable_show(struct class *class,
			      struct class_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", double_tap);
}

static ssize_t gesenable_store(struct class *class,
			       struct class_attribute *attr,
			       const char *buf, size_t count)
{
	bool enable;
	int ret;

	ret = kstrtobool(buf, &enable);
	if (ret)
		return ret;

	double_tap = enable;
	gesture_mode_update();

	return count;
}

static CLASS_ATTR_RW(gesenable);

static ssize_t gesmask_show(struct class *class,
			    struct class_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "0x%lx\n", gesture_mask);
}

static ssize_t gesmask_store(struct class *class,
			     struct class_attribute *attr,
			     const char *buf, size_t count)
{
	unsigned long mask;
	int ret;

	ret = kstrtoul(buf, 16, &mask);
	if (ret)
		return ret;
	if (mask & ~GENMASK(GESTURE_COUNT - 1, 0))
		return -EINVAL;

	gesture_mask = mask;
	gesture_mode_update();

	return count;
}

static CLASS_ATTR_RW(gesmask);

void ilitek_tp_gesture_init(void)
{
	struct class *cls;
	int i;

	if (!idev->c16) {
		ILI_ERR("No input device, gestures left off\n");
		return;
	}
	for (i = 0; i < GESTURE_COUNT; i++)
		input_set_capability(idev->c16, EV_KEY, KEY_GESTURE_BASE + i);

	cls = class_create(THIS_MODULE, "tp_gesture");
	if (IS_ERR(cls)) {
		ILI_ERR("Failed to create the tp_gesture class\n");
		return;
	}

	if (class_create_file(cls, &class_attr_gesenable) ||
	    class_create_file(cls, &class_attr_gesmask)) {
		ILI_ERR("Failed to create the tp_gesture nodes\n");
		class_destroy(cls);
	}
}
