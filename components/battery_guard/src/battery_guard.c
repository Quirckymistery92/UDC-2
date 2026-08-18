/*
 * SPDX-FileCopyrightText: 2026
 * SPDX-License-Identifier: Apache-2.0
 */

#include "battery_guard.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "esp_check.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs_sv_batgd.h"

static const char *TAG = "battery_guard";

#define BATTERY_GUARD_ADC_UNIT           ADC_UNIT_2
#define BATTERY_GUARD_ADC_CHANNEL        ADC_CHANNEL_3
#define BATTERY_GUARD_ADC_ATTEN          ADC_ATTEN_DB_12
#define BATTERY_GUARD_ADC_BITWIDTH       ADC_BITWIDTH_DEFAULT

#define BATTERY_GUARD_AVG_SAMPLES        16
#define BATTERY_GUARD_EMA_ALPHA          0.1f
#define BATTERY_GUARD_DIVIDER_RATIO      1.68f

#define BATTERY_GUARD_VOLTAGE_CUTOFF_MV  3100
#define BATTERY_GUARD_DEFAULT_VMIN_MV    3200
#define BATTERY_GUARD_DEFAULT_VMAX_MV    4200

#define BATTERY_GUARD_MIN_INTERVAL_MS    200
#define BATTERY_GUARD_DEFAULT_INTERVAL   800

#define BATTERY_GUARD_CHG_WINDOW_FAST_MS 1500
#define BATTERY_GUARD_CHG_WINDOW_SLOW_MS 4000
#define BATTERY_GUARD_CHG_DELTA_ON_MV    6
#define BATTERY_GUARD_CHG_DELTA_OFF_MV   (-12)
#define BATTERY_GUARD_CHG_HOLD_DELTA_MV  (-4)
#define BATTERY_GUARD_CHG_ON_COUNT       2
#define BATTERY_GUARD_CHG_OFF_COUNT      2
#define BATTERY_GUARD_CHG_NEAR_FULL_MV   20

#define BATTERY_GUARD_CHG_RAMP_STEP_PCT  1
#define BATTERY_GUARD_CHG_RAMP_MS        30000

#define BATTERY_GUARD_NVS_SAVE_DELTA_MV  20
#define BATTERY_GUARD_NVS_SAVE_MIN_MS    (30ULL * 60ULL * 1000ULL)

#define BATTERY_GUARD_LOW_HYST_PCT       2
#define BATTERY_GUARD_CRIT_HYST_PCT      1
#define BATTERY_GUARD_TASK_STACK         4096
#define BATTERY_GUARD_TASK_PRIO          5

typedef struct {
    int mv;
    int64_t ts_ms;
} voltage_sample_t;

typedef struct {
    bool initialized;
    bool running;

    TaskHandle_t task_handle;
    SemaphoreHandle_t lock;

    adc_oneshot_unit_handle_t adc_handle;
    adc_cali_handle_t cali_handle;
    bool cali_enabled;

    battery_guard_config_t cfg;
    battery_status_t status;

    bool ema_valid;
    float ema_voltage_mv;

    voltage_sample_t history[16];
    size_t history_count;
    size_t history_idx;

    int chg_on_votes;
    int chg_off_votes;
    bool charge_anchor_active;
    int charge_anchor_percentage;
    int64_t last_charge_ramp_ms;

    int vmin_mv;
    int vmax_mv;
    int persisted_vmin_mv;
    int persisted_vmax_mv;
    int64_t last_nvs_save_ms;

    bool low_alert_latched;
    bool critical_latched;
    bool shutdown_sequence_done;
    int64_t last_critical_log_ms;
} battery_guard_ctx_t;

static battery_guard_ctx_t s_ctx;

static int64_t bg_now_ms(void)
{
    return esp_timer_get_time() / 1000;
}

static int clamp_int(int value, int min_v, int max_v)
{
    if (value < min_v) {
        return min_v;
    }
    if (value > max_v) {
        return max_v;
    }
    return value;
}

static battery_guard_config_t bg_default_config(void)
{
    battery_guard_config_t cfg = {
        .threshold_low = 20,
        .threshold_critical = 5,
        .on_low_battery = NULL,
        .auto_deep_sleep = true,
        .on_shutdown_prepare = NULL,
        .on_status_change = NULL,
        .interval_ms = BATTERY_GUARD_DEFAULT_INTERVAL,
    };
    return cfg;
}

static int bg_voltage_to_percentage(int voltage_mv)
{
    int vmin = s_ctx.vmin_mv;
    int vmax = s_ctx.vmax_mv;

    if (vmax <= vmin) {
        return 0;
    }

    if (voltage_mv <= vmin) {
        return 0;
    }
    if (voltage_mv >= vmax) {
        return 100;
    }

    int num = (voltage_mv - vmin) * 100;
    int den = (vmax - vmin);
    return clamp_int(num / den, 0, 100);
}

static void bg_history_push(int voltage_mv, int64_t now_ms)
{
    s_ctx.history[s_ctx.history_idx].mv = voltage_mv;
    s_ctx.history[s_ctx.history_idx].ts_ms = now_ms;

    s_ctx.history_idx = (s_ctx.history_idx + 1U) % (sizeof(s_ctx.history) / sizeof(s_ctx.history[0]));
    if (s_ctx.history_count < (sizeof(s_ctx.history) / sizeof(s_ctx.history[0]))) {
        s_ctx.history_count++;
    }
}

static bool bg_get_delta_ms(int window_ms, int *out_delta_mv)
{
    if (out_delta_mv == NULL || s_ctx.history_count < 2) {
        return false;
    }

    size_t newest_idx = (s_ctx.history_idx + (sizeof(s_ctx.history) / sizeof(s_ctx.history[0])) - 1U) % (sizeof(s_ctx.history) / sizeof(s_ctx.history[0]));
    int newest_mv = s_ctx.history[newest_idx].mv;
    int64_t newest_ts = s_ctx.history[newest_idx].ts_ms;

    int found_delta = 0;
    bool found = false;

    for (size_t i = 0; i < s_ctx.history_count; ++i) {
        size_t idx = (s_ctx.history_idx + (sizeof(s_ctx.history) / sizeof(s_ctx.history[0])) - 1U - i) % (sizeof(s_ctx.history) / sizeof(s_ctx.history[0]));
        int64_t age = newest_ts - s_ctx.history[idx].ts_ms;
        if (age >= window_ms) {
            found_delta = newest_mv - s_ctx.history[idx].mv;
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    *out_delta_mv = found_delta;
    return true;
}

static void bg_update_charge_state(int filtered_mv)
{
    int delta_fast_mv = 0;
    int delta_slow_mv = 0;
    bool valid_fast = bg_get_delta_ms(BATTERY_GUARD_CHG_WINDOW_FAST_MS, &delta_fast_mv);
    bool valid_slow = bg_get_delta_ms(BATTERY_GUARD_CHG_WINDOW_SLOW_MS, &delta_slow_mv);

    if (!valid_fast && !valid_slow) {
        return;
    }

    int trend_mv = valid_fast ? delta_fast_mv : delta_slow_mv;
    int hold_mv = valid_slow ? delta_slow_mv : delta_fast_mv;
    bool near_full = filtered_mv >= (s_ctx.vmax_mv - BATTERY_GUARD_CHG_NEAR_FULL_MV);

    bool vote_on = (trend_mv >= BATTERY_GUARD_CHG_DELTA_ON_MV) ||
                   (near_full && hold_mv >= BATTERY_GUARD_CHG_HOLD_DELTA_MV);
    bool vote_off = (trend_mv <= BATTERY_GUARD_CHG_DELTA_OFF_MV) &&
                    (hold_mv <= BATTERY_GUARD_CHG_HOLD_DELTA_MV);

    if (vote_on) {
        s_ctx.chg_on_votes++;
        s_ctx.chg_off_votes = 0;
    } else if (vote_off) {
        s_ctx.chg_off_votes++;
        s_ctx.chg_on_votes = 0;
    } else {
        /* Mantener estado actual ante zona neutra para evitar "flapping". */
        if (s_ctx.status.is_charging) {
            s_ctx.chg_off_votes = 0;
        } else {
            s_ctx.chg_on_votes = 0;
        }
    }

    if (!s_ctx.status.is_charging && s_ctx.chg_on_votes >= BATTERY_GUARD_CHG_ON_COUNT) {
        s_ctx.status.is_charging = true;
        s_ctx.charge_anchor_active = true;
        s_ctx.charge_anchor_percentage = s_ctx.status.percentage;
        s_ctx.last_charge_ramp_ms = bg_now_ms();
        ESP_LOGI(TAG, "Charge detected (d_fast=%d mV, d_slow=%d mV)", delta_fast_mv, delta_slow_mv);
    }

    if (s_ctx.status.is_charging && s_ctx.chg_off_votes >= BATTERY_GUARD_CHG_OFF_COUNT) {
        s_ctx.status.is_charging = false;
        s_ctx.charge_anchor_active = false;
        ESP_LOGI(TAG, "Charge stopped (d_fast=%d mV, d_slow=%d mV)", delta_fast_mv, delta_slow_mv);
    }
}

static void bg_update_learning(int filtered_mv)
{
    int64_t now_ms = bg_now_ms();

    if (s_ctx.status.is_charging && filtered_mv > (s_ctx.vmax_mv + 5)) {
        s_ctx.vmax_mv = clamp_int(filtered_mv, s_ctx.vmin_mv + 200, 4500);
    }

    if (!s_ctx.status.is_charging && s_ctx.status.percentage <= 2 && filtered_mv < (s_ctx.vmin_mv - 5)) {
        s_ctx.vmin_mv = clamp_int(filtered_mv, 3000, s_ctx.vmax_mv - 200);
    }

    bool vmin_changed = abs(s_ctx.vmin_mv - s_ctx.persisted_vmin_mv) >= BATTERY_GUARD_NVS_SAVE_DELTA_MV;
    bool vmax_changed = abs(s_ctx.vmax_mv - s_ctx.persisted_vmax_mv) >= BATTERY_GUARD_NVS_SAVE_DELTA_MV;
    bool time_ok = (now_ms - s_ctx.last_nvs_save_ms) >= BATTERY_GUARD_NVS_SAVE_MIN_MS;

    if (time_ok && (vmin_changed || vmax_changed)) {
        if (vmin_changed) {
            if (nvs_sv_batgd_save_vmin_mv((int32_t)s_ctx.vmin_mv) == ESP_OK) {
                s_ctx.persisted_vmin_mv = s_ctx.vmin_mv;
            }
        }
        if (vmax_changed) {
            if (nvs_sv_batgd_save_vmax_mv((int32_t)s_ctx.vmax_mv) == ESP_OK) {
                s_ctx.persisted_vmax_mv = s_ctx.vmax_mv;
            }
        }
        s_ctx.last_nvs_save_ms = now_ms;
    }
}

static int bg_apply_anti_jump(int target_pct)
{
    int64_t now_ms = bg_now_ms();

    if (!s_ctx.status.is_charging) {
        s_ctx.charge_anchor_active = false;
        return target_pct;
    }

    if (!s_ctx.charge_anchor_active) {
        s_ctx.charge_anchor_active = true;
        s_ctx.charge_anchor_percentage = s_ctx.status.percentage;
        s_ctx.last_charge_ramp_ms = now_ms;
    }

    int result = s_ctx.status.percentage;
    if (result < s_ctx.charge_anchor_percentage) {
        result = s_ctx.charge_anchor_percentage;
    }

    if (target_pct > result && (now_ms - s_ctx.last_charge_ramp_ms) >= BATTERY_GUARD_CHG_RAMP_MS) {
        result += BATTERY_GUARD_CHG_RAMP_STEP_PCT;
        s_ctx.last_charge_ramp_ms = now_ms;
    }

    return clamp_int(result, 0, 100);
}

static bool bg_handle_thresholds(const battery_guard_config_t *cfg)
{
    bool trigger_low_cb = false;

    if (s_ctx.status.percentage <= cfg->threshold_low && !s_ctx.low_alert_latched) {
        s_ctx.low_alert_latched = true;
        trigger_low_cb = true;
    }

    if (s_ctx.status.percentage > (cfg->threshold_low + BATTERY_GUARD_LOW_HYST_PCT)) {
        s_ctx.low_alert_latched = false;
    }

    if (s_ctx.status.percentage <= cfg->threshold_critical && !s_ctx.critical_latched) {
        s_ctx.critical_latched = true;
    }

    if (s_ctx.status.percentage > (cfg->threshold_critical + BATTERY_GUARD_CRIT_HYST_PCT)) {
        s_ctx.critical_latched = false;
    }

    return trigger_low_cb;
}

static void bg_placeholder_shutdown_peripherals(void)
{
    ESP_LOGW(TAG, "Placeholder: apagar display/backlight/perifericos");
}

static esp_err_t bg_adc_read_filtered_mv(int *out_filtered_mv)
{
    if (out_filtered_mv == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    int32_t sum_mv = 0;
    for (int i = 0; i < BATTERY_GUARD_AVG_SAMPLES; ++i) {
        int raw = 0;
        ESP_RETURN_ON_ERROR(adc_oneshot_read(s_ctx.adc_handle, BATTERY_GUARD_ADC_CHANNEL, &raw), TAG, "adc read fail");

        int mv = 0;
        if (s_ctx.cali_enabled) {
            ESP_RETURN_ON_ERROR(adc_cali_raw_to_voltage(s_ctx.cali_handle, raw, &mv), TAG, "adc calibration fail");
        } else {
            mv = raw;
        }
        sum_mv += mv;
    }

    float avg_mv = (float)sum_mv / (float)BATTERY_GUARD_AVG_SAMPLES;
    float corrected_mv = avg_mv * BATTERY_GUARD_DIVIDER_RATIO;

    if (!s_ctx.ema_valid) {
        s_ctx.ema_voltage_mv = corrected_mv;
        s_ctx.ema_valid = true;
    } else {
        s_ctx.ema_voltage_mv = (BATTERY_GUARD_EMA_ALPHA * corrected_mv) + ((1.0f - BATTERY_GUARD_EMA_ALPHA) * s_ctx.ema_voltage_mv);
    }

    *out_filtered_mv = (int)lroundf(s_ctx.ema_voltage_mv);
    return ESP_OK;
}

static void battery_guard_task(void *arg)
{
    (void)arg;

    while (s_ctx.running) {
        battery_guard_config_t cfg_local = bg_default_config();
        battery_status_t status_snapshot = {0};
        battery_status_t previous = {0};
        void (*status_cb)(battery_status_t) = NULL;
        void (*low_cb)(int) = NULL;
        void (*shutdown_cb)(void) = NULL;
        bool trigger_low_cb = false;
        bool status_changed = false;
        bool trigger_shutdown = false;
        bool auto_sleep = true;

        int filtered_mv = 0;
        if (bg_adc_read_filtered_mv(&filtered_mv) != ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(s_ctx.cfg.interval_ms));
            continue;
        }

        if (xSemaphoreTake(s_ctx.lock, pdMS_TO_TICKS(100)) == pdTRUE) {
            cfg_local = s_ctx.cfg;
            previous = s_ctx.status;

            s_ctx.status.voltage_mv = filtered_mv;
            bg_history_push(filtered_mv, bg_now_ms());
            bg_update_charge_state(filtered_mv);
            bg_update_learning(filtered_mv);

            int target_pct = bg_voltage_to_percentage(filtered_mv);
            s_ctx.status.percentage = bg_apply_anti_jump(target_pct);

            trigger_low_cb = bg_handle_thresholds(&cfg_local);
            if (cfg_local.auto_deep_sleep) {
                if (s_ctx.status.voltage_mv <= BATTERY_GUARD_VOLTAGE_CUTOFF_MV && !s_ctx.shutdown_sequence_done) {
                    s_ctx.shutdown_sequence_done = true;
                    trigger_shutdown = true;
                }
            } else {
                if (s_ctx.status.voltage_mv <= BATTERY_GUARD_VOLTAGE_CUTOFF_MV) {
                    int64_t now_ms = bg_now_ms();
                    if ((now_ms - s_ctx.last_critical_log_ms) >= 30000) {
                        ESP_LOGW(TAG, "Critical voltage (%d mV) detected with auto_deep_sleep=false; system keeps running", s_ctx.status.voltage_mv);
                        s_ctx.last_critical_log_ms = now_ms;
                    }
                }
                s_ctx.shutdown_sequence_done = false;
            }

            status_changed = (s_ctx.status.percentage != previous.percentage) ||
                             (s_ctx.status.is_charging != previous.is_charging);

            status_snapshot = s_ctx.status;
            status_cb = cfg_local.on_status_change;
            low_cb = cfg_local.on_low_battery;
            shutdown_cb = cfg_local.on_shutdown_prepare;
            auto_sleep = cfg_local.auto_deep_sleep;

            xSemaphoreGive(s_ctx.lock);
        }

        if (trigger_low_cb && low_cb != NULL) {
            low_cb(status_snapshot.percentage);
        }

        if (status_changed && status_cb != NULL) {
            status_cb(status_snapshot);
        }

        if (trigger_shutdown) {
            ESP_LOGE(TAG, "Battery critical voltage (%d mV)", status_snapshot.voltage_mv);
            if (shutdown_cb != NULL) {
                shutdown_cb();
            }
            bg_placeholder_shutdown_peripherals();
            if (auto_sleep) {
                ESP_LOGW(TAG, "Entering deep sleep due to critical battery");
                esp_deep_sleep_start();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(cfg_local.interval_ms));
    }

    s_ctx.task_handle = NULL;
    vTaskDelete(NULL);
}

static esp_err_t bg_load_learning_from_nvs(void)
{
    int32_t vmin = 0;
    int32_t vmax = 0;

    esp_err_t err_vmin = nvs_sv_batgd_load_vmin_mv(&vmin);
    esp_err_t err_vmax = nvs_sv_batgd_load_vmax_mv(&vmax);

    if (err_vmin != ESP_OK) {
        vmin = BATTERY_GUARD_DEFAULT_VMIN_MV;
        (void)nvs_sv_batgd_save_vmin_mv(vmin);
    }
    if (err_vmax != ESP_OK) {
        vmax = BATTERY_GUARD_DEFAULT_VMAX_MV;
        (void)nvs_sv_batgd_save_vmax_mv(vmax);
    }

    if (vmax <= (vmin + 100)) {
        vmin = BATTERY_GUARD_DEFAULT_VMIN_MV;
        vmax = BATTERY_GUARD_DEFAULT_VMAX_MV;
        (void)nvs_sv_batgd_save_vmin_mv(vmin);
        (void)nvs_sv_batgd_save_vmax_mv(vmax);
    }

    s_ctx.vmin_mv = (int)vmin;
    s_ctx.vmax_mv = (int)vmax;
    s_ctx.persisted_vmin_mv = s_ctx.vmin_mv;
    s_ctx.persisted_vmax_mv = s_ctx.vmax_mv;
    s_ctx.last_nvs_save_ms = bg_now_ms();

    ESP_LOGI(TAG, "Learning loaded: vmin=%d mv, vmax=%d mv", s_ctx.vmin_mv, s_ctx.vmax_mv);
    return ESP_OK;
}

esp_err_t battery_guard_init(void)
{
    if (s_ctx.initialized) {
        return ESP_OK;
    }

    memset(&s_ctx, 0, sizeof(s_ctx));
    s_ctx.cfg = bg_default_config();

    s_ctx.lock = xSemaphoreCreateMutex();
    if (s_ctx.lock == NULL) {
        return ESP_ERR_NO_MEM;
    }

    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = BATTERY_GUARD_ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&unit_cfg, &s_ctx.adc_handle), TAG, "adc unit init fail");

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = BATTERY_GUARD_ADC_ATTEN,
        .bitwidth = BATTERY_GUARD_ADC_BITWIDTH,
    };
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(s_ctx.adc_handle, BATTERY_GUARD_ADC_CHANNEL, &chan_cfg), TAG, "adc channel cfg fail");

    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = BATTERY_GUARD_ADC_UNIT,
        .atten = BATTERY_GUARD_ADC_ATTEN,
        .bitwidth = BATTERY_GUARD_ADC_BITWIDTH,
    };

    esp_err_t cal_err = adc_cali_create_scheme_curve_fitting(&cali_cfg, &s_ctx.cali_handle);
    if (cal_err == ESP_OK) {
        s_ctx.cali_enabled = true;
    } else {
        ESP_LOGW(TAG, "Calibration unavailable, fallback without cali (%s)", esp_err_to_name(cal_err));
        s_ctx.cali_enabled = false;
    }

    ESP_RETURN_ON_ERROR(bg_load_learning_from_nvs(), TAG, "nvs load fail");

    s_ctx.status.voltage_mv = s_ctx.vmin_mv;
    s_ctx.status.percentage = 0;
    s_ctx.status.is_charging = false;

    s_ctx.initialized = true;
    ESP_LOGI(TAG, "battery_guard initialized");
    return ESP_OK;
}

esp_err_t battery_guard_set_config(const battery_guard_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    battery_guard_config_t normalized = *config;
    normalized.interval_ms = clamp_int((int)normalized.interval_ms, BATTERY_GUARD_MIN_INTERVAL_MS, 60 * 1000);
    normalized.threshold_low = clamp_int(normalized.threshold_low, 0, 100);
    normalized.threshold_critical = clamp_int(normalized.threshold_critical, 0, normalized.threshold_low);

    if (xSemaphoreTake(s_ctx.lock, pdMS_TO_TICKS(100)) == pdTRUE) {
        s_ctx.cfg = normalized;
        xSemaphoreGive(s_ctx.lock);
        return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}

esp_err_t battery_guard_start(void)
{
    if (!s_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (s_ctx.running) {
        return ESP_OK;
    }

    s_ctx.running = true;
    BaseType_t ok = xTaskCreate(
        battery_guard_task,
        "battery_guard",
        BATTERY_GUARD_TASK_STACK,
        NULL,
        BATTERY_GUARD_TASK_PRIO,
        &s_ctx.task_handle
    );

    if (ok != pdPASS) {
        s_ctx.running = false;
        s_ctx.task_handle = NULL;
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

esp_err_t battery_guard_stop(void)
{
    if (!s_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!s_ctx.running) {
        return ESP_OK;
    }

    s_ctx.running = false;

    for (int i = 0; i < 20; ++i) {
        if (s_ctx.task_handle == NULL) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    return ESP_OK;
}

esp_err_t battery_guard_get_status(battery_status_t *out_status)
{
    if (out_status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_ctx.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(s_ctx.lock, pdMS_TO_TICKS(100)) == pdTRUE) {
        *out_status = s_ctx.status;
        xSemaphoreGive(s_ctx.lock);
        return ESP_OK;
    }
    return ESP_ERR_TIMEOUT;
}

esp_err_t battery_guard_deinit(void)
{
    if (!s_ctx.initialized) {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(battery_guard_stop(), TAG, "stop fail");

    if (s_ctx.cali_enabled) {
        (void)adc_cali_delete_scheme_curve_fitting(s_ctx.cali_handle);
        s_ctx.cali_enabled = false;
    }

    if (s_ctx.adc_handle != NULL) {
        (void)adc_oneshot_del_unit(s_ctx.adc_handle);
        s_ctx.adc_handle = NULL;
    }

    if (s_ctx.lock != NULL) {
        vSemaphoreDelete(s_ctx.lock);
        s_ctx.lock = NULL;
    }

    s_ctx.initialized = false;
    return ESP_OK;
}
