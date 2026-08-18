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

#include "boot/app_launcher.h"
#include "version_info.h"

static const char *TAG = "Boot";

void app_main(void)
{
    ESP_LOGI(TAG, "UDC-2 starting...");
    
    esp_err_t ret = app_launcher_init(TAG);
    if(ret != ESP_OK){
        ESP_LOGI(TAG, "Falló la inicialización del sistema, codigo de error: %s", esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "UDC-2 initialized successfully on Build");
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
