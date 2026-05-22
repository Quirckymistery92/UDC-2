/*
 * SPDX-FileCopyrightText: 2026
 * SPDX-License-Identifier: Apache-2.0
 */

#include "nvs_sv_batgd.h"

#include <stdbool.h>

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "nvs_sv_batgd";
static const char *s_namespace = "batgd";

static esp_err_t nvs_sv_batgd_ensure_flash_ready(void)
{
    static bool s_ready = false;

    if (s_ready) {
        return ESP_OK;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err == ESP_OK) {
        s_ready = true;
    }
    return err;
}

static esp_err_t nvs_sv_batgd_save_i32(const char *key, int32_t value)
{
    esp_err_t err = nvs_sv_batgd_ensure_flash_ready();
    if (err != ESP_OK) {
        return err;
    }

    nvs_handle_t handle;
    err = nvs_open(s_namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_i32(handle, key, value);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

static esp_err_t nvs_sv_batgd_load_i32(const char *key, int32_t *out_value)
{
    if (out_value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = nvs_sv_batgd_ensure_flash_ready();
    if (err != ESP_OK) {
        return err;
    }

    nvs_handle_t handle;
    err = nvs_open(s_namespace, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_get_i32(handle, key, out_value);
    nvs_close(handle);
    return err;
}

esp_err_t nvs_sv_batgd_save_vmin_mv(int32_t vmin_mv)
{
    ESP_LOGD(TAG, "save vmin_mv=%ld", (long)vmin_mv);
    return nvs_sv_batgd_save_i32("vmin_mv", vmin_mv);
}

esp_err_t nvs_sv_batgd_load_vmin_mv(int32_t *out_vmin_mv)
{
    return nvs_sv_batgd_load_i32("vmin_mv", out_vmin_mv);
}

esp_err_t nvs_sv_batgd_save_vmax_mv(int32_t vmax_mv)
{
    ESP_LOGD(TAG, "save vmax_mv=%ld", (long)vmax_mv);
    return nvs_sv_batgd_save_i32("vmax_mv", vmax_mv);
}

esp_err_t nvs_sv_batgd_load_vmax_mv(int32_t *out_vmax_mv)
{
    return nvs_sv_batgd_load_i32("vmax_mv", out_vmax_mv);
}
