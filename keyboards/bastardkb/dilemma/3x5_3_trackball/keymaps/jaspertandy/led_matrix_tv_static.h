#pragma once
#include "led_matrix_motion.h"

void tv_static_scan_matrix(void);
bool tv_static_paint(RGB *pixels, const bklm_motion_t *m);
