/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * SPIFFS Controller – mount / unmount / status helpers
 */

#ifndef SPIFFS_CONTROLLER_H
#define SPIFFS_CONTROLLER_H

#include "esp_err.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Default mount point used across the project */
#define SPIFFS_MOUNT_POINT "/spiffs"

/**
 * @brief Mount the SPIFFS partition.
 *
 * Uses the "storage" partition (type data, subtype spiffs) and
 * mounts it at SPIFFS_MOUNT_POINT.  Safe to call more than once;
 * subsequent calls return ESP_OK immediately.
 *
 * @return ESP_OK on success, or an esp_err_t error code.
 */
esp_err_t spiffs_controller_init(void);

/**
 * @brief Unmount a previously mounted SPIFFS partition.
 *
 * @return ESP_OK on success, or ESP_ERR_INVALID_STATE if not mounted.
 */
esp_err_t spiffs_controller_deinit(void);

/**
 * @brief Check whether SPIFFS is currently mounted.
 */
bool spiffs_controller_is_mounted(void);

/**
 * @brief Log total / used bytes to the console.
 *
 * @return ESP_OK on success.
 */
esp_err_t spiffs_controller_info(size_t *total, size_t *used);

#ifdef __cplusplus
}
#endif

#endif /* SPIFFS_CONTROLLER_H */
