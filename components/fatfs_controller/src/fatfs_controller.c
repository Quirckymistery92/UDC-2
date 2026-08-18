/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * FATFS Controller – implementation
 */

#include "fatfs_controller.h"

#include <string.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"

static const char *TAG = "fatfs_ctrl";
static bool s_mounted = false;
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

esp_err_t fatfs_controller_init(void)
{
    if (s_mounted) {
        ESP_LOGW(TAG, "Already mounted");
        return ESP_OK;
    }

    esp_vfs_fat_mount_config_t mount_config = VFS_FAT_MOUNT_DEFAULT_CONFIG();
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 10;

    esp_err_t ret = esp_vfs_fat_spiflash_mount_rw_wl(FATFS_MOUNT_POINT, "storage", &mount_config, &s_wl_handle);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "FATFS partition not found");
        } else {
            ESP_LOGE(TAG, "FATFS init error: %s", esp_err_to_name(ret));
        }
        return ret;
    }

    s_mounted = true;

    uint64_t total_bytes = 0, free_bytes = 0;
    ret = esp_vfs_fat_info(FATFS_MOUNT_POINT, &total_bytes, &free_bytes);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "FATFS mounted at %s  total: %llu  free: %llu",
                 FATFS_MOUNT_POINT,
                 (unsigned long long) total_bytes,
                 (unsigned long long) free_bytes);
    }

    return ESP_OK;
}

esp_err_t fatfs_controller_deinit(void)
{
    if (!s_mounted) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_vfs_fat_spiflash_unmount_rw_wl(FATFS_MOUNT_POINT, s_wl_handle);
    if (ret == ESP_OK) {
        s_mounted = false;
        s_wl_handle = WL_INVALID_HANDLE;
        ESP_LOGI(TAG, "FATFS unmounted");
    }
    return ret;
}

bool fatfs_controller_is_mounted(void)
{
    return s_mounted;
}

esp_err_t fatfs_controller_info(size_t *total, size_t *used)
{
    if (!s_mounted) return ESP_ERR_INVALID_STATE;

    uint64_t total_bytes = 0, free_bytes = 0;
    esp_err_t ret = esp_vfs_fat_info(FATFS_MOUNT_POINT, &total_bytes, &free_bytes);
    if (ret != ESP_OK) {
        return ret;
    }

    if (total != NULL) {
        *total = (size_t) total_bytes;
    }
    if (used != NULL) {
        *used = (size_t) (total_bytes - free_bytes);
    }
    return ESP_OK;
}

const char* fatfs_controller_get_mount_point(void)
{
    return FATFS_MOUNT_POINT;
}
