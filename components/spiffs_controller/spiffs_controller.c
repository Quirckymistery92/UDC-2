/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPIFFS Controller – implementation
 */

#include "spiffs_controller.h"

#include <string.h>
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "spiffs_ctrl";
static bool s_mounted = false;

esp_err_t spiffs_controller_init(void)
{
    if (s_mounted) {
        ESP_LOGW(TAG, "Already mounted");
        return ESP_OK;
    }

    esp_vfs_spiffs_conf_t conf = {
        .base_path              = SPIFFS_MOUNT_POINT,
        .partition_label        = "storage",
        .max_files              = 8,
        .format_if_mount_failed = false,
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount SPIFFS");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "SPIFFS partition not found");
        } else {
            ESP_LOGE(TAG, "SPIFFS init error: %s", esp_err_to_name(ret));
        }
        return ret;
    }

    s_mounted = true;

    size_t total = 0, used = 0;
    ret = esp_spiffs_info("storage", &total, &used);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SPIFFS mounted at %s  total: %zu  used: %zu",
                 SPIFFS_MOUNT_POINT, total, used);
    }

    return ESP_OK;
}

esp_err_t spiffs_controller_deinit(void)
{
    if (!s_mounted) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_vfs_spiffs_unregister("storage");
    if (ret == ESP_OK) {
        s_mounted = false;
        ESP_LOGI(TAG, "SPIFFS unmounted");
    }
    return ret;
}

bool spiffs_controller_is_mounted(void)
{
    return s_mounted;
}

esp_err_t spiffs_controller_info(size_t *total, size_t *used)
{
    if (!s_mounted) return ESP_ERR_INVALID_STATE;
    return esp_spiffs_info("storage", total, used);
}
