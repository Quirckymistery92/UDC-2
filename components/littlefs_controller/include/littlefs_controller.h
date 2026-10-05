/**
 * @file littlefs_controller.h
 * @brief Controlador de LittleFS.
 *
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * LittleFS Controller – mount / unmount / status helpers
 */

#pragma once

/* ESP */
#include "esp_err.h"
#include <stddef.h>
#include <stdbool.h>

/* Módulos */
#include "desc_apps.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Default mount point used across the project */
#define LITTLEFS_MOUNT_POINT "/littlefs"

/**
 * @brief Mount the LittleFS partition.
 *
 * Uses the "storage" partition (type data, subtype spiffs or littlefs) and
 * mounts it at LITTLEFS_MOUNT_POINT.  Safe to call more than once;
 * subsequent calls return ESP_OK immediately.
 *
 */
app_desc_status_t* littlefs_controller_init(void);

/**
 * @brief Unmount a previously mounted LittleFS partition.
 *
 * @return ESP_OK on success, or ESP_ERR_INVALID_STATE if not mounted.
 */
esp_err_t littlefs_controller_deinit(void);

/**
 * @brief Check whether LittleFS is currently mounted.
 */
bool littlefs_controller_is_mounted(void);

/**
 * @brief Log total / used bytes to the console.
 *
 * @return ESP_OK on success.
 */
esp_err_t littlefs_controller_info(size_t *total, size_t *used);

const char* littlefs_controller_get_mount_point(void);

#ifdef __cplusplus
}
#endif