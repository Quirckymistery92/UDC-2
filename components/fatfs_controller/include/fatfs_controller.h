/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * FATFS Controller – mount / unmount / status helpers
 */

#ifndef FATFS_CONTROLLER_H
#define FATFS_CONTROLLER_H

#include "esp_err.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Default mount point used across the project */
#define FATFS_MOUNT_POINT "/fatfs"

/**
 * @brief Mount the FATFS partition.
 *
 * Uses the "storage" partition (type data, subtype fat) and
 * mounts it at FATFS_MOUNT_POINT.  Safe to call more than once;
 * subsequent calls return ESP_OK immediately.
 *
 * @return ESP_OK on success, or an esp_err_t error code.
 */
esp_err_t fatfs_controller_init(void);

/**
 * @brief Unmount a previously mounted FATFS partition.
 *
 * @return ESP_OK on success, or ESP_ERR_INVALID_STATE if not mounted.
 */
esp_err_t fatfs_controller_deinit(void);

/**
 * @brief Check whether FATFS is currently mounted.
 */
bool fatfs_controller_is_mounted(void);

/**
 * @brief Log total / used bytes to the console.
 *
 * @return ESP_OK on success.
 */
esp_err_t fatfs_controller_info(size_t *total, size_t *used);

const char* fatfs_controller_get_mount_point(void);

#ifdef __cplusplus
}
#endif

#endif /* FATFS_CONTROLLER_H */
