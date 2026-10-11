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

#include "app_launcher.h"
#include "version_info.h"

/* Variable */
    #define TAG "Bootloader"
    #define TIME_WAIT_FAILSAFE_MIN 15

__attribute__((noreturn)) void terrible_fail_boot(app_desc_status_t *app)
{
    if(app != NULL) ESP_LOGW(TAG, "El compoente %s era escencial en el arranque. ", app->app_name);

    ESP_LOGE(TAG, "El sistema se reiniciará pronto.");

    /* Auto Reboot */
    uint8_t let = 0;

    while(let < TIME_WAIT_FAILSAFE_MIN) {
        vTaskDelay(pdMS_TO_TICKS(60000UL)); /* 1m interval */
        ESP_LOGI(TAG, "Falló la inicialización del sistema, codigo de error: %s", app != NULL ? esp_err_to_name(app->init_result) : "N/A");
        let++;
    }

    esp_restart();

    while(true) {
        vTaskDelay(portMAX_DELAY);
    }

    __builtin_unreachable();
}

void app_main(void)
{
    ESP_LOGI(TAG, "UDC-2 starting...");
    
    esp_err_t ret = app_launcher_init();
    if(unlikely(ret != ESP_OK)) terrible_fail_boot(NULL);
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "UDC-2 initialized successfully");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Número de Build : %d", BUILD_COUNTER);
    ESP_LOGI(TAG, "Fecha / Hora    : %s", BUILD_DATETIME);
    ESP_LOGI(TAG, "Rama Git        : %s", BUILD_GIT_BRANCH);
    ESP_LOGI(TAG, "Commit Git      : %s", BUILD_GIT_COMMIT);
    ESP_LOGI(TAG, "========================================");

    /* Keep app main alive */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));

    }
}
