/*
 * SPDX-FileCopyrightText: 2026
 * SPDX-License-Identifier: Apache-2.0
 */
#include "battery_guard.h"

/* ESP */
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

/* Variables & Estructuras */
    static const char * const TAG = "battery_guard";

    // Configuración del ADC
    #define BATTERY_GUARD_ADC_UNIT           ADC_UNIT_2
    #define BATTERY_GUARD_ADC_CHANNEL        ADC_CHANNEL_3
    #define BATTERY_GUARD_ADC_ATTEN          ADC_ATTEN_DB_12
    #define BATTERY_GUARD_ADC_BITWIDTH       ADC_BITWIDTH_DEFAULT

    // Configuración de Filtros y Muestras
    #define BATTERY_GUARD_MEDIAN_SAMPLES     9
    #define BATTERY_GUARD_EMA_ALPHA          0.1f
    #define BATTERY_GUARD_DIVIDER_RATIO      1.68f

    // Umbrales por defecto (ahora solo en RAM)
    #define BATTERY_GUARD_VOLTAGE_CUTOFF_MV  3100
    #define BATTERY_GUARD_DEFAULT_VMIN_MV    3200
    #define BATTERY_GUARD_DEFAULT_VMAX_MV    4200

    #define BATTERY_GUARD_MIN_INTERVAL_MS    200
    #define BATTERY_GUARD_DEFAULT_INTERVAL   800
    #define BATTERY_GUARD_LOW_HYST_PCT       2
    #define BATTERY_GUARD_CRIT_HYST_PCT      1

    #define BATTERY_GUARD_TASK_STACK         4096
    #define BATTERY_GUARD_TASK_PRIO          5

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
        
        // Filtros y aprendizaje
        bool ema_valid;
        float ema_voltage_mv;
        int vmin_mv;
        int vmax_mv;

        // Latches para evitar spam de eventos
        bool low_alert_latched;
        bool critical_latched;
        bool shutdown_sequence_done;
        
        battery_status_t last_emitted_status; // Para evaluar cambios
    } battery_guard_ctx_t;

    static battery_guard_ctx_t s_ctx;

    
static inline int clamp_int(int value, int min_v, int max_v) {
    if (value < min_v) return min_v;
    if (value > max_v) return max_v;
    return value;
}

static battery_guard_config_t bg_default_config(void) {
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

static int bg_voltage_to_percentage(int voltage_mv) {
    int vmin = s_ctx.vmin_mv;
    int vmax = s_ctx.vmax_mv;
    if (vmax <= vmin || voltage_mv <= vmin) return 0;
    if (voltage_mv >= vmax) return 100;
    
    int num = (voltage_mv - vmin) * 100;
    int den = (vmax - vmin);
    return clamp_int(num / den, 0, 100);
}

// Filtro de Mediana + EMA
static esp_err_t bg_adc_read_filtered_mv(int *out_filtered_mv) {
    if (!out_filtered_mv) return ESP_ERR_INVALID_ARG;

    int samples[BATTERY_GUARD_MEDIAN_SAMPLES];
    
    // 1. Ráfaga de lecturas rápidas
    for (int i = 0; i < BATTERY_GUARD_MEDIAN_SAMPLES; ++i) {
        int raw = 0, mv = 0;
        ESP_RETURN_ON_ERROR(adc_oneshot_read(s_ctx.adc_handle, BATTERY_GUARD_ADC_CHANNEL, &raw), TAG, "adc read fail");
        
        if (s_ctx.cali_enabled) {
            adc_cali_raw_to_voltage(s_ctx.cali_handle, raw, &mv);
        } else {
            mv = raw;
        }
        samples[i] = mv;
    }

    // 2. Ordenamiento por Inserción (muy rápido para arrays pequeños)
    for (int i = 1; i < BATTERY_GUARD_MEDIAN_SAMPLES; i++) {
        int key = samples[i];
        int j = i - 1;
        while (j >= 0 && samples[j] > key) {
            samples[j + 1] = samples[j];
            j = j - 1;
        }
        samples[j + 1] = key;
    }

    // 3. Extraer mediana y aplicar divisor
    int median_mv = samples[BATTERY_GUARD_MEDIAN_SAMPLES / 2];
    float corrected_mv = (float)median_mv * BATTERY_GUARD_DIVIDER_RATIO;

    // 4. Aplicar EMA (Exponential Moving Average)
    if (!s_ctx.ema_valid) {
        s_ctx.ema_voltage_mv = corrected_mv;
        s_ctx.ema_valid = true;
    } else {
        s_ctx.ema_voltage_mv = (BATTERY_GUARD_EMA_ALPHA * corrected_mv) + 
                               ((1.0f - BATTERY_GUARD_EMA_ALPHA) * s_ctx.ema_voltage_mv);
    }

    *out_filtered_mv = (int)lroundf(s_ctx.ema_voltage_mv);
    return ESP_OK;
}

static void bg_update_learning_ram(int filtered_mv, int percentage) {
    // Si vemos que el voltaje sube por encima del máximo conocido, actualizamos el techo temporal
    if (filtered_mv > (s_ctx.vmax_mv + 5)) {
        s_ctx.vmax_mv = clamp_int(filtered_mv, s_ctx.vmin_mv + 200, 4500);
    }
    // Si la batería se reporta muy baja pero el voltaje es aún menor que el mínimo, bajamos el suelo
    if (percentage <= 2 && filtered_mv < (s_ctx.vmin_mv - 5)) {
        s_ctx.vmin_mv = clamp_int(filtered_mv, 3000, s_ctx.vmax_mv - 200);
    }
}

static void battery_guard_task(void *arg) {
    (void)arg;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (s_ctx.running) {
        int filtered_mv = 0;
        
        // Si la lectura falla, esperamos al siguiente ciclo
        if (bg_adc_read_filtered_mv(&filtered_mv) != ESP_OK) {
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(s_ctx.cfg.interval_ms));
            continue;
        }

        battery_status_t current_status = {0};
        
        // CORRECCIÓN AQUÍ: Inicializamos config con valores seguros por si falla el semáforo
        battery_guard_config_t cfg_local = bg_default_config(); 
        
        bool trigger_low_cb = false;
        bool trigger_shutdown = false;
        bool status_changed = false;

        if (xSemaphoreTake(s_ctx.lock, pdMS_TO_TICKS(100)) == pdTRUE) {
            cfg_local = s_ctx.cfg; // Ahora sí, tomamos la config real y segura
            
            // 1. Cálculos de estado
            int target_pct = bg_voltage_to_percentage(filtered_mv);
            bg_update_learning_ram(filtered_mv, target_pct);
            
            s_ctx.status.voltage_mv = filtered_mv;
            s_ctx.status.percentage = target_pct;
            s_ctx.status.is_charging = false; 
            s_ctx.status.driver_ok = true; // Capa de compatibilidad I2C

            current_status = s_ctx.status;

            // 2. Evaluación de cambios (Anti-Spam de Callbacks)
            status_changed = (current_status.percentage != s_ctx.last_emitted_status.percentage) ||
                             (current_status.is_charging != s_ctx.last_emitted_status.is_charging) ||
                             (current_status.driver_ok != s_ctx.last_emitted_status.driver_ok);

            if (status_changed) {
                s_ctx.last_emitted_status = current_status;
            }

            // 3. Manejo de alertas (con Histéresis / Latch)
            if (current_status.percentage <= cfg_local.threshold_low && !s_ctx.low_alert_latched) {
                s_ctx.low_alert_latched = true;
                trigger_low_cb = true; // Se dispara solo al cruzar hacia abajo
            } else if (current_status.percentage > (cfg_local.threshold_low + BATTERY_GUARD_LOW_HYST_PCT)) {
                s_ctx.low_alert_latched = false; // Se reinicia el cerrojo
            }

            // 4. Apagado crítico
            if (current_status.percentage <= cfg_local.threshold_critical && !s_ctx.critical_latched) {
                s_ctx.critical_latched = true;
            } else if (current_status.percentage > (cfg_local.threshold_critical + BATTERY_GUARD_CRIT_HYST_PCT)) {
                s_ctx.critical_latched = false;
            }

            if (current_status.voltage_mv <= BATTERY_GUARD_VOLTAGE_CUTOFF_MV && !s_ctx.shutdown_sequence_done) {
                s_ctx.shutdown_sequence_done = true;
                trigger_shutdown = true;
            }

            xSemaphoreGive(s_ctx.lock);
        }

        // Ejecución de Callbacks fuera del mutex
        if (trigger_low_cb && cfg_local.on_low_battery != NULL) {
            cfg_local.on_low_battery(current_status.percentage);
        }

        if (status_changed && cfg_local.on_status_change != NULL) {
            cfg_local.on_status_change(current_status);
        }

        if (trigger_shutdown) {
            ESP_LOGE(TAG, "Battery critical voltage (%d mV). Shutting down.", current_status.voltage_mv);
            if (cfg_local.on_shutdown_prepare != NULL) {
                cfg_local.on_shutdown_prepare();
            }
            if (cfg_local.auto_deep_sleep) {
                esp_deep_sleep_start();
            }
        }

        // Bloqueo preciso sin acumular drift de tiempo
        // Ahora es 100% seguro porque cfg_local siempre tiene un valor válido
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(cfg_local.interval_ms));
    }

    s_ctx.task_handle = NULL;
    vTaskDelete(NULL);
}

esp_err_t battery_guard_init(void) {
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
        ESP_LOGW(TAG, "ADC Calibration fallback (%s)", esp_err_to_name(cal_err));
        s_ctx.cali_enabled = false;
    }

    // Inicializamos valores de aprendizaje en RAM
    s_ctx.vmin_mv = BATTERY_GUARD_DEFAULT_VMIN_MV;
    s_ctx.vmax_mv = BATTERY_GUARD_DEFAULT_VMAX_MV;
    
    // Estado inicial ficticio para forzar la primera emisión del callback
    s_ctx.last_emitted_status.percentage = -1; 
    s_ctx.status.driver_ok = true;
    s_ctx.initialized = true;
    
    ESP_LOGI(TAG, "battery_guard initialized (No-NVS, Median-EMA Filter)");
    return ESP_OK;
}

esp_err_t battery_guard_set_config(const battery_guard_config_t *config) {
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

esp_err_t battery_guard_start(void) {
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

esp_err_t battery_guard_stop(void) {
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

esp_err_t battery_guard_get_status(battery_status_t *out_status) {
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

esp_err_t battery_guard_deinit(void) {
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