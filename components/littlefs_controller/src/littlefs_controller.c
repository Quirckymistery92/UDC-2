/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * LittleFS Controller – implementation
 */

#include "littlefs_controller.h"

/* ESP */
#include <string.h>
#include "esp_log.h"

/* Módulos */
#include "esp_littlefs.h"

/* Variables */
    static const char * const TAG = "LittleFS_Ctrl";
    static bool s_mounted = false;
    static app_desc_status_t apd = {
        .app_desc = DSC_APP_STORAGE_FLASH,
        .app_name = "Memoria Interna",
        .app_tag = TAG,
        .init_result = ESP_OK,
    };

app_desc_status_t* littlefs_controller_init(void)
{
   DSC_RETURN_STRUCT_ON_FALSE(!s_mounted, &apd, ESP_OK);

    // Configuración de montaje para LittleFS
    esp_vfs_littlefs_conf_t conf = {
        .base_path = LITTLEFS_MOUNT_POINT,
        .partition_label = "storage",
        .format_if_mount_failed = false,
        .dont_mount = false,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    DSC_RETURN_STRUCT_ON_FALSE(ret == ESP_OK, &apd, ret);

    s_mounted = true;

    size_t total_bytes = 0, used_bytes = 0;
    ret = esp_littlefs_info(conf.partition_label, &total_bytes, &used_bytes);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "LittleFS mounted at %s  total: %zu  used: %zu",
                 LITTLEFS_MOUNT_POINT,
                 total_bytes,
                 used_bytes);
    }

    return DESC_AP_INSERT(&apd, ret);
}

esp_err_t littlefs_controller_deinit(void)
{
    if (!s_mounted) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_vfs_littlefs_unregister("storage");
    if (ret == ESP_OK) {
        s_mounted = false;
        ESP_LOGI(TAG, "LittleFS unmounted");
    }
    return ret;
}

bool littlefs_controller_is_mounted(void)
{
    return s_mounted;
}

esp_err_t littlefs_controller_info(size_t *total, size_t *used)
{
    if (!s_mounted) return ESP_ERR_INVALID_STATE;

    size_t total_bytes = 0, used_bytes = 0;
    esp_err_t ret = esp_littlefs_info("storage", &total_bytes, &used_bytes);
    if (ret != ESP_OK) {
        return ret;
    }

    if (total != NULL) {
        *total = total_bytes;
    }
    if (used != NULL) {
        *used = used_bytes; // LittleFS devuelve directamente el espacio usado
    }
    return ESP_OK;
}

const char* littlefs_controller_get_mount_point(void)
{
    return LITTLEFS_MOUNT_POINT;
}