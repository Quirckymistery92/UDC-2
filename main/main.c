/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * UDC-2 Main Application
 * ESP32-P4 + JD9365 DSI Display + GSL3680 Touch + LVGL 9
 *
 * Uses ui_manager.h for all UI interaction (task-safe command queue)
 */

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_controller.h"
#include "rtc_rx8025t.h"
#include "ui_manager.h"
#include "spiffs_controller.h"
#include "jpegc_element.h"
#include "battery_actions.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "UDC-2 starting...");

    /* I2C master bus — must be first, shared by touch and RTC */
    ESP_LOGI(TAG, "Initializing I2C controller...");
    ESP_ERROR_CHECK(i2c_controller_init());

    /* RTC — sync system time before anything else needs it */
    ESP_LOGI(TAG, "Initializing RTC (RX8025T)...");
    esp_err_t rtc_err = rtc_rx8025t_init();
    if (rtc_err == ESP_FAIL) {
        ESP_LOGW(TAG, "RTC: time not synced (first boot or power loss) - set time manually");
    } else {
        ESP_ERROR_CHECK(rtc_err);
    }

    /* Mount SPIFFS – must happen before LVGL so lv_fs_posix can read images */
    ESP_LOGI(TAG, "Mounting SPIFFS...");
    ESP_ERROR_CHECK(spiffs_controller_init());

    /* Initialize entire UI subsystem (LVGL task + backlight) */
    ESP_LOGI(TAG, "Initializing UI subsystem...");
    ESP_ERROR_CHECK(ui_init());
    ESP_LOGI(TAG, "UI subsystem initialized");

    /* Wait for screens to be fully created inside LVGL task */
    vTaskDelay(pdMS_TO_TICKS(500));

    /* Load the home screen */
    ESP_ERROR_CHECK(ui_screen_change(SCREEN_ID_DS_HOME,
                                     LV_SCR_LOAD_ANIM_FADE_IN,
                                     200, 0, false));

    /* Initialize JPEGC element */
    ESP_LOGI(TAG, "Initializing JPEGC element...");
    ESP_ERROR_CHECK(jpegc_element_init());

    /* Initialize battery guard abstraction layer */
    ESP_LOGI(TAG, "Initializing battery actions...");
    ESP_ERROR_CHECK(battery_actions_init());

    ESP_LOGI(TAG, "UDC-2 initialized successfully");

    /* Keep app main alive */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
