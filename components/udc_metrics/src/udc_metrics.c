#include "udc_metrics.h"

/* ESP */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_heap_caps.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "esp_heap_caps.h"

/* Módulos */
#include "esp_littlefs.h"

/* Variables & estructuras */
    static const char * const TAG = "Metrics";
    static app_desc_status_t apd = {
        .app_desc = DSC_APP_METRICS,
        .app_name = "Métricas",
        .app_tag = TAG,
        .init_result = ESP_OK
    };
    struct udc_metric_sub_t {
        udc_metric_type_t type;
        uint32_t interval_ticks;
        uint8_t priority;
        uint32_t id;
        uint32_t target_ticks;
        bool to_delete;
        bool is_paused;
        bool startup_run_pending;
        
        udc_metric_config_t config;
        udc_metric_callback_t cb;
        void *user_ctx;

        struct {
            uint32_t prev_total_time;
            uint32_t prev_idle_time[configNUM_CORES];
        } cpu_state;
        
        struct udc_metric_sub_t *next;
    };

    static struct udc_metric_sub_t *head = NULL;
    static uint32_t global_sub_id = 0;
    static SemaphoreHandle_t list_mutex = NULL;

static inline uint8_t udc_metric_to_percent(uint32_t used, uint32_t total) {
    if (total == 0u) return 0u;
    uint32_t pct = (used * 100u) / total;
    if (pct > 100u) pct = 100u;
    return (uint8_t)pct;
}

static udc_metric_size_unit_t udc_metric_select_scale(uint64_t total_bytes) {
    if (total_bytes >= (1024ULL * 1024ULL * 1024ULL)) {
        return UDC_METRIC_UNIT_GB;
    }
    if (total_bytes >= (1024ULL * 1024ULL)) {
        return UDC_METRIC_UNIT_MB;
    }
    if (total_bytes >= 1024ULL) {
        return UDC_METRIC_UNIT_KB;
    }
    return UDC_METRIC_UNIT_BYTES;
}

static uint32_t udc_metric_scale_value(uint64_t value, udc_metric_size_unit_t unit) {
    switch (unit) {
        case UDC_METRIC_UNIT_BYTES:
            return (uint32_t)value;
        case UDC_METRIC_UNIT_KB:
            return (uint32_t)(value / 1024ULL);
        case UDC_METRIC_UNIT_MB:
            return (uint32_t)(value / (1024ULL * 1024ULL));
        case UDC_METRIC_UNIT_GB:
            return (uint32_t)(value / (1024ULL * 1024ULL * 1024ULL));
        default:
            return 0u;
    }
}

/* --- Lectores Desacoplados --- */

static udc_metric_status_t fetch_ram_data(const udc_metric_cfg_ram_t *cfg, udc_metric_ram_data_t *out) {
    multi_heap_info_t info;
    heap_caps_get_info(&info, cfg->memory_caps);

    if (info.total_free_bytes == 0 && info.total_allocated_bytes == 0) {
        return UDC_METRIC_ERR_NOT_SUPPORTED;
    }

    const uint64_t total_bytes = (uint64_t)info.total_free_bytes + (uint64_t)info.total_allocated_bytes;
    const uint64_t used_bytes = (uint64_t)info.total_allocated_bytes;
    out->unit = udc_metric_select_scale(total_bytes);
    out->total = udc_metric_scale_value(total_bytes, out->unit);
    out->used = udc_metric_scale_value(used_bytes, out->unit);
    out->min_used_ever = udc_metric_scale_value(info.minimum_free_bytes == 0 ? used_bytes : (uint64_t)info.total_allocated_bytes - (uint64_t)info.minimum_free_bytes, out->unit);
    return UDC_METRIC_OK;
}

static udc_metric_status_t fetch_vfs_data(const udc_metric_cfg_vfs_t *cfg, udc_metric_vfs_data_t *out) {
    if (!cfg || !cfg->identifier) return UDC_METRIC_ERR_NOT_MOUNTED;
    
    esp_err_t err = ESP_FAIL;

    switch (cfg->fs_type) {
        case UDC_VFS_TYPE_FAT: {
            uint64_t fat_total = 0;
            uint64_t fat_free = 0;
            err = esp_vfs_fat_info(cfg->identifier, &fat_total, &fat_free);
            if (err == ESP_OK) {
                out->unit = udc_metric_select_scale(fat_total);
                out->total = udc_metric_scale_value(fat_total, out->unit);
                out->used = udc_metric_scale_value(fat_total - fat_free, out->unit);
            }
            break;
        }
        case UDC_VFS_TYPE_LITTLEFS: {
            size_t lfs_total = 0, lfs_used = 0;
            err = esp_littlefs_info(cfg->identifier, &lfs_total, &lfs_used);
            if (err == ESP_OK) {
                out->unit = udc_metric_select_scale((uint64_t)lfs_total);
                out->total = udc_metric_scale_value((uint64_t)lfs_total, out->unit);
                out->used = udc_metric_scale_value((uint64_t)lfs_used, out->unit);
            }
            break;
        }
    }

    if (err == ESP_OK) {
        return UDC_METRIC_OK;
    }
    return UDC_METRIC_ERR_NOT_MOUNTED;
}

static udc_metric_status_t fetch_cpu_data(struct udc_metric_sub_t *sub, udc_metric_cpu_data_t *out) {
    uint32_t current_idle_time[configNUM_CORES] = {0};
    TaskStatus_t idle_status;

    uint32_t current_total_time = portGET_RUN_TIME_COUNTER_VALUE();

    for (int i = 0; i < configNUM_CORES; i++) {
        TaskHandle_t idle_handle = xTaskGetIdleTaskHandleForCore(i);
        if (idle_handle) {
            vTaskGetInfo(idle_handle, &idle_status, pdFALSE, eInvalid);
            current_idle_time[i] = idle_status.ulRunTimeCounter;
        }
    }

    if (sub->cpu_state.prev_total_time == 0u) {
        out->percent_used = 0u;
    } else {
        uint32_t delta_total = current_total_time - sub->cpu_state.prev_total_time;
        if (delta_total > 0u) {
            int core_id = sub->config.cpu.core_id;
            uint32_t delta_used = 0u;
            uint32_t total_capacity = 0u;

            if (core_id >= 0 && core_id < configNUM_CORES) {
                uint32_t delta_idle = current_idle_time[core_id] - sub->cpu_state.prev_idle_time[core_id];
                delta_used = (delta_idle <= delta_total) ? (delta_total - delta_idle) : 0u;
                total_capacity = delta_total;
            } else {
                total_capacity = delta_total * configNUM_CORES;
                uint32_t sum_delta_idle = 0u;
                for (int i = 0; i < configNUM_CORES; i++) {
                    sum_delta_idle += (current_idle_time[i] - sub->cpu_state.prev_idle_time[i]);
                }
                delta_used = (sum_delta_idle <= total_capacity) ? (total_capacity - sum_delta_idle) : 0u;
            }

            out->percent_used = udc_metric_to_percent(delta_used, total_capacity);
        } else {
            out->percent_used = 0u;
        }
    }

    sub->cpu_state.prev_total_time = current_total_time;
    for (int i = 0; i < configNUM_CORES; i++) {
        sub->cpu_state.prev_idle_time[i] = current_idle_time[i];
    }
    return UDC_METRIC_OK;
}

/* --- Motor Principal --- */

static void udc_metrics_task(void *arg) {
    while (1) {
        xSemaphoreTake(list_mutex, portMAX_DELAY);
        
        struct udc_metric_sub_t **curr_ptr = &head;
        while (*curr_ptr) {
            if ((*curr_ptr)->to_delete) {
                struct udc_metric_sub_t *tmp = *curr_ptr;
                *curr_ptr = (*curr_ptr)->next;
                heap_caps_free(tmp);
            } else {
                curr_ptr = &(*curr_ptr)->next;
            }
        }

        struct udc_metric_sub_t *best_ready = NULL;
        uint32_t now = xTaskGetTickCount();

        for (struct udc_metric_sub_t *curr = head; curr != NULL; curr = curr->next) {
            if (!curr->to_delete && !curr->is_paused &&(int32_t)(now - curr->target_ticks) >= 0) {
                if (!best_ready) {
                    best_ready = curr;
                } else {
                    if (curr->priority > best_ready->priority) {
                        best_ready = curr;
                    } else if (curr->priority == best_ready->priority && curr->id < best_ready->id) {
                        best_ready = curr;
                    }
                }
            }
        }

        if (best_ready != NULL) {
            udc_metric_event_t event = {0};
            event.type = best_ready->type;

            switch (event.type) {
                case UDC_METRIC_VFS_STATE:
                    event.status = fetch_vfs_data(&best_ready->config.vfs, &event.data.vfs); break;
                case UDC_METRIC_RAM_STATE:
                    event.status = fetch_ram_data(&best_ready->config.ram, &event.data.ram); break;
                case UDC_METRIC_CPU_USAGE:
                    event.status = fetch_cpu_data(best_ready, &event.data.cpu); break;
            }

            bool first_startup_run = best_ready->startup_run_pending;
            best_ready->target_ticks += best_ready->interval_ticks;
            if (first_startup_run) {
                best_ready->startup_run_pending = false;
                if (!best_ready->config.enabled_by_default) {
                    best_ready->is_paused = true;
                }
            }
            
            udc_metric_callback_t cb = best_ready->cb;
            void *user_ctx = best_ready->user_ctx;
            
            xSemaphoreGive(list_mutex);
            if (cb) cb(&event, user_ctx);
            
        } else {
            uint32_t min_wait = pdMS_TO_TICKS(100);
            for (struct udc_metric_sub_t *curr = head; curr != NULL; curr = curr->next) {
                if (!curr->to_delete && !curr->is_paused) {
                    uint32_t wait = curr->target_ticks - xTaskGetTickCount();
                    if (wait < min_wait) min_wait = wait;
                }
            }
            xSemaphoreGive(list_mutex);
            if (min_wait > 0) vTaskDelay(min_wait);
        }
    }
}

/* --- API Pública --- */

app_desc_status_t* udc_metrics_init(void) {
    DSC_RETURN_STRUCT_ON_FALSE(list_mutex == NULL, &apd, ESP_ERR_INVALID_STATE);

    list_mutex = xSemaphoreCreateMutex();
    DSC_RETURN_STRUCT_ON_FALSE(list_mutex != NULL, &apd, ESP_ERR_NO_MEM);

    BaseType_t ret = xTaskCreate(udc_metrics_task, "udc_met_eng", 4096, NULL, 15, NULL);
    DSC_RETURN_STRUCT_ON_FALSE(ret == pdPASS, &apd, ESP_FAIL);

    return DESC_AP_INSERT(&apd, ESP_OK);
}

udc_metric_handle_t udc_metrics_subscribe(
    udc_metric_type_t type, uint32_t interval_ms, uint8_t priority, 
    const udc_metric_config_t *config, udc_metric_callback_t cb, void *user_ctx) 
{
    struct udc_metric_sub_t *new_sub = heap_caps_calloc(1, sizeof(struct udc_metric_sub_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!new_sub) return NULL;

    new_sub->type = type;
    new_sub->interval_ticks = pdMS_TO_TICKS(interval_ms);
    new_sub->priority = priority;
    new_sub->cb = cb;
    new_sub->user_ctx = user_ctx;
    new_sub->to_delete = false;
    
    if (config) {
        new_sub->config = *config;
    } else {
        new_sub->config = (udc_metric_config_t){0};
        new_sub->config.run_on_start = false;
        new_sub->config.enabled_by_default = false;
    }

    new_sub->startup_run_pending = new_sub->config.run_on_start;
    const bool run_on_start = new_sub->startup_run_pending;
    const bool enabled_by_default = new_sub->config.enabled_by_default;

    xSemaphoreTake(list_mutex, portMAX_DELAY);
    new_sub->id = ++global_sub_id;
    new_sub->target_ticks = xTaskGetTickCount() + (run_on_start ? 0u : new_sub->interval_ticks);
    new_sub->is_paused = !(enabled_by_default || run_on_start);
    new_sub->config.run_on_start = false;
    new_sub->config.enabled_by_default = enabled_by_default;

    new_sub->next = head;
    head = new_sub;
    xSemaphoreGive(list_mutex);

    return new_sub;
}

esp_err_t udc_metrics_set_paused(udc_metric_handle_t handle, bool pause) {
    if (!handle) return ESP_ERR_INVALID_ARG;

    xSemaphoreTake(list_mutex, portMAX_DELAY);
    
    // Solo actuamos si realmente hay un cambio de estado
    if (handle->is_paused != pause) {
        handle->is_paused = pause;
        
        if (!pause) {
            handle->target_ticks = xTaskGetTickCount() + handle->interval_ticks;
            handle->cpu_state.prev_total_time = 0;
            
            for (int i = 0; i < configNUM_CORES; i++) {
                handle->cpu_state.prev_idle_time[i] = 0;
            }
        }
    }
    
    xSemaphoreGive(list_mutex);
    return ESP_OK;
}

esp_err_t udc_metrics_unsubscribe(udc_metric_handle_t handle) {
    if (!handle) return ESP_ERR_INVALID_ARG;
    
    xSemaphoreTake(list_mutex, portMAX_DELAY);
    handle->to_delete = true; 
    xSemaphoreGive(list_mutex);
    
    return ESP_OK;
}