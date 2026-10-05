/**
 * @file porting.h
 * @brief LVGL porting interface declarations.
 *
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * LVGL 9 Porting - Display DSI + Touch GSL3680
 * Private implementation (not exposed to application)
 */

#pragma once

#include "esp_err.h"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize display hardware and register with LVGL
 *
 * Called from LVGL task during lvgl_init()
 * Must run within LVGL task context
 *
 * @return lv_display_t pointer on success, NULL on failure
 */
lv_display_t *lvgl_porting_display_init(void);

/**
 * @brief Initialize touch hardware and register with LVGL
 *
 * Called from LVGL task during lvgl_init()
 * Must run within LVGL task context
 *
 * @param disp  Display pointer (for touch input routing)
 * @return lv_indev_t pointer on success, NULL on failure
 */
lv_indev_t *lvgl_porting_touch_init(lv_display_t *disp);

/**
 * @brief Set the LVGL task handle for VSYNC notification
 *
 * MUST be called from the LVGL task after task creation.
 * This is required for tear-free display synchronization.
 *
 * @param task Handle of the LVGL task (xTaskGetCurrentTaskHandle())
 */
void lvgl_porting_set_task_handle(TaskHandle_t task);

#ifdef __cplusplus
}
#endif