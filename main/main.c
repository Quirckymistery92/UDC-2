/*
 * SPDX-FileCopyrightText: 2026 Alex Jaque
 * SPDX-License-Identifier: Apache-2.0
 *
 * UDC-2 Main Application
 * ESP32-P4 + JD9365 DSI Display + GSL3680 Touch + LVGL 9
 *
 */

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_controller.h"
#include "rtc_rx8025t.h"
#include "ui_manager.h"
#include "fatfs_controller.h"
#include "jpegc_element.h"
#include "battery_actions.h"
#include "NVSm_sys.h"
#include "sys_module.h"
#include "version_info.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "UDC-2 starting...");

    ESP_LOGI(TAG, "Inicializing NVS...");
    ESP_ERROR_CHECK(NVSm_sys_init());

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

    /* Mount FATFS – must happen before LVGL so lv_fs_posix can read images */
    ESP_LOGI(TAG, "Mounting FATFS...");
    ESP_ERROR_CHECK(fatfs_controller_init());

    /* Initialize JPEGC element */
    ESP_LOGI(TAG, "Initializing JPEGC element...");
    ESP_ERROR_CHECK(jpegc_element_init());

    /* Initialize entire UI subsystem (LVGL task + backlight) */
    ESP_LOGI(TAG, "Initializing UI subsystem...");
    ESP_ERROR_CHECK(ui_init());
    ESP_LOGI(TAG, "UI subsystem initialized");

    /* Wait for screens to be fully created inside LVGL task */
    ui__helper_sync_sys_screens_semaphore(SCREEN_ID_DS_LOCKED);

    ESP_LOGI(TAG, "Inicializando módulos del sistema");
    ESP_ERROR_CHECK(sys_module_glob_init());

    /* Load the home screen */
    ESP_ERROR_CHECK(ui_screen_change(SCREEN_ID_DS_LOCKED,
                                     LV_SCR_LOAD_ANIM_FADE_IN,
                                     200, 0, false));


    /* Initialize battery guard abstraction layer */
    ESP_LOGI(TAG, "Initializing battery actions...");
    ESP_ERROR_CHECK(battery_actions_init());

    ESP_LOGI(TAG, "UDC-2 initialized successfully on Build");
    ESP_LOGI(TAG, "Número de Build : %d", BUILD_COUNTER);
    ESP_LOGI(TAG, "Fecha / Hora    : %s", BUILD_DATETIME);
    ESP_LOGI(TAG, "Rama Git        : %s", BUILD_GIT_BRANCH);
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Commit Git      : %s", BUILD_GIT_COMMIT);
    ESP_LOGI(TAG, "========================================");

    /* Keep app main alive */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));

    }
}
