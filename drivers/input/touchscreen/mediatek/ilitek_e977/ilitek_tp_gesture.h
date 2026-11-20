/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The gesture switches Android uses. Not part of the factory kernel: see
 * ilitek_tp_gesture.c.
 */
#ifndef _ILITEK_TP_GESTURE_H_
#define _ILITEK_TP_GESTURE_H_

#include <linux/input.h>
#include <linux/types.h>

void ilitek_tp_gesture_init(void);

/* Whether a double tap should come back as the power key. */
bool ilitek_tp_gesture_double_tap_enabled(void);

/* Reports the key of a gesture other than the double tap, if it is enabled. */
void ilitek_tp_gesture_report(struct input_dev *in, u8 code);

#endif
