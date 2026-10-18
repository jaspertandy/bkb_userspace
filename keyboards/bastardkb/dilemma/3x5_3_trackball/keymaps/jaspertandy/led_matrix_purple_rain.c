// SPDX-License-Identifier: GPL-2.0-or-later
// Purple variant of the module's matrix rain (led_matrix_motion_matrix.c).

#include QMK_KEYBOARD_H

#include "led_matrix_motion.h"
#include "led_matrix_tv_static.h"

typedef struct {
    int32_t  head;  /* Q8 rows from the top; negative while waiting to start */
    uint16_t speed; /* Q8 rows per second */
    uint8_t  trail; /* rows */
} drop_t;

static void drop_start(drop_t *d, bool anywhere) {
    d->speed = (uint16_t)((5 + bklm_rand() % 9) * 256); /* 5-13 rows a second */
    d->trail = (uint8_t)(4 + bklm_rand() % 8);          /* 4-11 rows */
    d->head  = anywhere ? (int32_t)(bklm_rand() % (BKLM_ROWS * 256)) : -(int32_t)(bklm_rand() % (8 * 256));
}

static bool purple_rain(RGB *pixels, const bklm_motion_t *m) {
    static drop_t   drops[BKLM_COLS];
    static uint8_t  glyph[BKLM_ROWS][BKLM_COLS]; /* per-cell brightness jitter */
    static uint32_t flicker;

    if (m->fresh) {
        for (uint8_t x = 0; x < BKLM_COLS; x++) drop_start(&drops[x], true);
        for (uint8_t y = 0; y < BKLM_ROWS; y++)
            for (uint8_t x = 0; x < BKLM_COLS; x++) glyph[y][x] = (uint8_t)(150 + bklm_rand() % 106);
    }
    flicker += m->dt;
    while (flicker >= 60) {
        flicker -= 60;
        for (uint8_t k = 0; k < 10; k++) glyph[bklm_rand() % BKLM_ROWS][bklm_rand() % BKLM_COLS] = (uint8_t)(110 + bklm_rand() % 146);
    }

    for (uint8_t x = 0; x < BKLM_COLS; x++) {
        drop_t *d = &drops[x];
        d->head += (int32_t)(d->speed * m->dt / 1000);
        if ((d->head >> 8) - d->trail > BKLM_ROWS) drop_start(d, false);
        const int32_t head = d->head >> 8;
        for (int32_t row = head - d->trail; row <= head; row++) {
            if (row < 0 || row >= BKLM_ROWS) continue;
            RGB *p = bklm_px(pixels, x, (uint8_t)(BKLM_ROWS - 1 - row));
            if (row == head) {
                *p = (RGB){.r = 210, .g = 120, .b = 255}; /* pale purple head */
            } else {
                const uint32_t age = (uint32_t)(head - row);                     /* 1 .. trail */
                const uint32_t lv  = 255 * (d->trail + 1 - age) / (d->trail + 1); /* fades along the trail */
                const uint8_t  v   = (uint8_t)(lv * glyph[row][x] / 255);
                *p                 = (RGB){.r = (uint8_t)(v * 9 / 10), .g = (uint8_t)(v / 10), .b = v}; /* same hue all along, only brightness changes */
            }
        }
    }
    return true;
}

bool bklm_motion_user(uint8_t index, RGB *pixels, const bklm_motion_t *m) {
    switch (index) {
        case 0:
            return purple_rain(pixels, m);
        case 1:
            return tv_static_paint(pixels, m);
        default:
            return false;
    }
}
