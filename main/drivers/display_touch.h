/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * Touch Driver - GSL3680 I2C Configuration
 */

#pragma once

#include "esp_err.h"
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ──────────────────────── Touch GPIO ──────────────────────── */
#define TOUCH_GPIO_RST              (GPIO_NUM_22)
#define TOUCH_GPIO_INT              (GPIO_NUM_21)
#define TOUCH_I2C_CLK_SPEED_HZ      (400000)

/* ──────────────────────── Touch Resolution ────────────────── */
/* Must match the LCD resolution for correct coordinate mapping */
#define TOUCH_H_RES                 (800)
#define TOUCH_V_RES                 (1280)

/**
 * @brief Initialize the GSL3680 touch controller.
 *        I2C bus must already be initialized via i2c_controller_init().
 *
 * @param[out] ret_touch  Returned touch handle.
 * @return ESP_OK on success.
 */
esp_err_t display_touch_init(esp_lcd_touch_handle_t *ret_touch);

#ifdef __cplusplus
}
#endif
