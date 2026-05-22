/*
 * SPDX-FileCopyrightText: 2026
 * SPDX-License-Identifier: Apache-2.0
 */

#include "battery_actions.h"
#include "ui_manager.h"
#include <stdbool.h>

#include "esp_log.h"
#include "battery_guard.h"

static const char *TAG = "battery_actions";
static bool s_actions_initialized = false;

static void cb_low_battery_empty(int percentage)
{
    (void)percentage;
}

static void cb_shutdown_prepare_empty(void)
{
}

static void cb_status_change_empty(battery_status_t status)
{
    ESP_LOGI(TAG, "Battery status changed: %d%%, %d mV, charging: %s",
             status.percentage, status.voltage_mv, status.is_charging ? "yes" : "no");


}

esp_err_t battery_actions_init(void)
{
    if (s_actions_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Init battery actions");

    ESP_ERROR_CHECK(battery_guard_init());

    battery_guard_config_t cfg = {
        .threshold_low = 20,
        .threshold_critical = 5,
        .on_low_battery = cb_low_battery_empty,
        .auto_deep_sleep = false,
        .on_shutdown_prepare = cb_shutdown_prepare_empty,
        .on_status_change = cb_status_change_empty,
        .interval_ms = 800,
    };

    ESP_ERROR_CHECK(battery_guard_set_config(&cfg));
    ESP_ERROR_CHECK(battery_guard_start());

    s_actions_initialized = true;
    return ESP_OK;
}

esp_err_t battery_actions_stop(void)
{
    if (!s_actions_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Stop battery actions");

    esp_err_t err_stop = battery_guard_stop();
    esp_err_t err_deinit = battery_guard_deinit();

    s_actions_initialized = false;

    if (err_stop != ESP_OK) {
        return err_stop;
    }
    return err_deinit;
}
