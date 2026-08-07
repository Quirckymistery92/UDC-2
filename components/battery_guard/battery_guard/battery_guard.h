/*
 * SPDX-FileCopyrightText: 2026
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int voltage_mv;
    int percentage;
    bool is_charging;
} battery_status_t;

typedef struct {
    int threshold_low;
    int threshold_critical;
    void (*on_low_battery)(int percentage);
    bool auto_deep_sleep;
    void (*on_shutdown_prepare)(void);
    void (*on_status_change)(battery_status_t status);
    uint32_t interval_ms;
} battery_guard_config_t;

esp_err_t battery_guard_init(void);
esp_err_t battery_guard_set_config(const battery_guard_config_t *config);
esp_err_t battery_guard_start(void);
esp_err_t battery_guard_stop(void);
esp_err_t battery_guard_get_status(battery_status_t *out_status);
esp_err_t battery_guard_deinit(void);

#ifdef __cplusplus
}
#endif