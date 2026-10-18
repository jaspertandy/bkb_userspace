/**
 * Copyright 2021 Charly Delay <charly@codesink.dev> (@0xcharly)
 * Copyright 2023 casuanoob <casuanoob@hotmail.com> (@casuanoob)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#define DYNAMIC_KEYMAP_LAYER_COUNT 8
#define LED_MATRIX_MODULE_PIN GP12

#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_LED_STATE_ENABLE

#define ENCODER_RESOLUTION 4

#ifdef AUTO_MOUSE_DEFAULT_LAYER
#undef AUTO_MOUSE_DEFAULT_LAYER
#endif
#define AUTO_MOUSE_DEFAULT_LAYER 4

#ifdef LED_DPI_INDICATOR_INDEX
#undef LED_DPI_INDICATOR_INDEX
#endif
#define LED_DPI_INDICATOR_INDEX 7

#ifdef RGBLIGHT_LED_COUNT
#undef RGBLIGHT_LED_COUNT
#endif
#define RGBLIGHT_LED_COUNT 72

// LED matrix panel: one non-reactive animation (purple rain), always playing.
#define LED_MATRIX_MODULE_MOTION_USER_COUNT 1
#define LED_MATRIX_MODULE_MOTION_STYLE (BKLM_MOTION_USER + 0)
#define LED_MATRIX_MODULE_MOTION_CYCLE (BKLM_MOTION_USER + 0)
#define LED_MATRIX_MODULE_MOTION_IDLE 1
#define LED_MATRIX_MODULE_MOTION_IDLE_MS 0
// Keep the panel on the animation: never dim or blank it, and make the style
// "preview" (which outranks every other indicator) effectively permanent.
#define LED_MATRIX_MODULE_DIM_MS 0xFFFFFFFFu
#define LED_MATRIX_MODULE_OFF_MS 0xFFFFFFFFu
#define LED_MATRIX_MODULE_MOTION_PREVIEW_MS 0x7FFFFFFFu
