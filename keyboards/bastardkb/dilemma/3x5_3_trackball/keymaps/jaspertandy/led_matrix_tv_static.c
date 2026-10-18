// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * TV static that typing silences. White noise fills the panel. A keypress
 * cuts the static and lights one cell: the key's column, and a random row
 * within the band that maps to the key's row (16 panel rows / 4 key rows =
 * 4-row bands, thumbs at the bottom). The cell stays full white while the key
 * is held, then fades out on release. Once the last flash has died, the static
 * fades back in.
 */

#include QMK_KEYBOARD_H

#include "led_matrix_motion.h"
#include "led_matrix_tv_static.h"

#define TV_FLASH_MS 60   /* full white after release (a tap shows at least this long) */
#define TV_FADE_MS 450   /* then fades out */
#define TV_END_MS (TV_FLASH_MS + TV_FADE_MS)
#define TV_HELD UINT32_MAX /* release time while the key is still down */
#define TV_STATIC_IN_MS 400 /* static fades back in over this */

#define TV_STATIC_MAX 90 /* brightest static cell; the odd cell flashes brighter */
#define TV_BAND_ROWS (BKLM_ROWS / 4)

/* Panel column for each matrix position. Finger columns get their own panel
 * column (left 0-4, right 7-11). The inner thumbs sit under the inner finger
 * columns; the other thumbs share the two middle panel columns (5 and 6). */
static const uint8_t tv_panel_col[MATRIX_ROWS][MATRIX_COLS] = {
    {0, 1, 2, 3, 4},   {0, 1, 2, 3, 4},   {0, 1, 2, 3, 4},   {5, 5, 4, 0, 0},   /* left: 3 finger rows, thumbs (outer, middle, inner) */
    {11, 10, 9, 8, 7}, {11, 10, 9, 8, 7}, {11, 10, 9, 8, 7}, {6, 7, 6, 0, 0},   /* right: mirrored, thumbs (middle, inner, outer) */
};

/* Key row 0..3 (top finger row .. thumbs) for each matrix row. */
static const uint8_t tv_key_row[MATRIX_ROWS] = {0, 1, 2, 3, 0, 1, 2, 3};

static uint32_t     tv_released_at[BKLM_ROWS][BKLM_COLS]; /* 0 = dark, TV_HELD = key down */
static uint8_t      tv_cell_of[MATRIX_ROWS][MATRIX_COLS];  /* panel y the held key lit */
static matrix_row_t tv_prev[MATRIX_ROWS];
static uint32_t     tv_quiet_since; /* when the last flash died */
static bool         tv_quiet = true;

static void tv_hit(uint8_t r, uint8_t c) {
    const uint8_t x = tv_panel_col[r][c];
    /* Panel y is up: key row 0 is the top band, thumbs the bottom one. */
    const uint8_t band_bottom = (uint8_t)((3 - tv_key_row[r]) * TV_BAND_ROWS);
    const uint8_t y           = (uint8_t)(band_bottom + bklm_rand() % TV_BAND_ROWS);
    tv_cell_of[r][c]     = y;
    tv_released_at[y][x] = TV_HELD;
    tv_quiet             = false;
}

static void tv_release(uint8_t r, uint8_t c) {
    const uint8_t x = tv_panel_col[r][c];
    const uint8_t y = tv_cell_of[r][c];
    if (tv_released_at[y][x] != TV_HELD) return; /* another key took this cell over and already let go */
    uint32_t now = timer_read32();
    if (now == 0 || now == TV_HELD) now = 1;
    tv_released_at[y][x] = now;
}

/* Called every loop (housekeeping_task_user): catches presses the 50 ms frame
 * rate would miss. The matrix is mirrored to the panel half, so this sees both
 * sides regardless of which half has USB. */
void tv_static_scan_matrix(void) {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        const matrix_row_t now      = matrix_get_row(r);
        const matrix_row_t pressed  = now & ~tv_prev[r];
        const matrix_row_t released = tv_prev[r] & ~now;
        tv_prev[r]                  = now;
        if (!pressed && !released) continue;
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            if (pressed & ((matrix_row_t)1 << c)) tv_hit(r, c);
            if (released & ((matrix_row_t)1 << c)) tv_release(r, c);
        }
    }
}

bool tv_static_paint(RGB *pixels, const bklm_motion_t *m) {
    /* Flashes first: while any is alive the static stays off. */
    bool any = false;
    for (uint8_t y = 0; y < BKLM_ROWS; y++) {
        for (uint8_t x = 0; x < BKLM_COLS; x++) {
            uint32_t hit = 0;
            if (tv_released_at[y][x] == TV_HELD) {
                hit = 255;
                any = true;
            } else if (tv_released_at[y][x]) {
                const uint32_t t = timer_elapsed32(tv_released_at[y][x]);
                if (t >= TV_END_MS) {
                    tv_released_at[y][x] = 0;
                } else {
                    hit = t < TV_FLASH_MS ? 255 : 255 * (TV_END_MS - t) / TV_FADE_MS;
                    any = true;
                }
            }
            *bklm_px(pixels, x, y) = (RGB){.r = (uint8_t)hit, .g = (uint8_t)hit, .b = (uint8_t)hit};
        }
    }
    if (any) {
        return true;
    }

    /* Nothing left: the static fades back in from the moment the last flash died. */
    if (!tv_quiet) {
        tv_quiet       = true;
        tv_quiet_since = m->now;
    }
    const uint32_t since = timer_elapsed32(tv_quiet_since);
    const uint32_t level = since >= TV_STATIC_IN_MS ? 255 : 255 * since / TV_STATIC_IN_MS;
    for (uint8_t y = 0; y < BKLM_ROWS; y++) {
        for (uint8_t x = 0; x < BKLM_COLS; x++) {
            uint32_t v = bklm_rand() % (TV_STATIC_MAX + 1);
            if ((bklm_rand() & 31) == 0) v = 200; /* occasional bright speck */
            v                      = v * level / 255;
            *bklm_px(pixels, x, y) = (RGB){.r = (uint8_t)v, .g = (uint8_t)v, .b = (uint8_t)v};
        }
    }
    return true;
}
