/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * RX8025T RTC driver for ESP-IDF 5.x
 */

#pragma once

#include <time.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the RX8025T driver.
 *
 * Registers the device on the shared I2C bus (i2c_controller must already be
 * initialized), clears fault flags, applies default config, and synchronizes
 * the ESP system clock if the RTC holds valid data.
 *
 * @return ESP_OK          - success, system time synced.
 *         ESP_FAIL        - VLF set: time data lost after power failure, not synced.
 *         other           - I2C or allocation error.
 */
esp_err_t rtc_rx8025t_init(void);

/**
 * @brief Read the current time from the RTC.
 *
 * @param[out] t  Filled with current RTC time as POSIX struct tm (UTC).
 * @return ESP_OK          - valid time returned.
 *         ESP_FAIL        - VLF set, time not reliable.
 *         other           - I2C error.
 */
esp_err_t rtc_rx8025t_get_time(struct tm *t);

/**
 * @brief Write time to the RTC.
 *
 * @param[in]  t  Time to set (POSIX struct tm, UTC).
 * @return ESP_OK on success.
 */
esp_err_t rtc_rx8025t_set_time(const struct tm *t);

/**
 * @brief Check if the RTC holds valid (non-corrupted) time.
 *
 * Reads the VLF status bit only. No side effects, no logging.
 *
 * @return true  - time is reliable.
 *         false - time data lost (VLF set, power failure occurred).
 */
bool rtc_rx8025t_is_valid(void);

/**
 * @brief Deinitialize the driver, stop the sync timer, release I2C device.
 *
 * @return ESP_OK on success.
 */
esp_err_t rtc_rx8025t_deinit(void);

#ifdef __cplusplus
}
#endif
