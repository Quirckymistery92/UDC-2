#pragma once

/* ESP */
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/* Módulos */
#include "desc_apps.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- 1. Tipos y Estados --- */

typedef enum {
    UDC_METRIC_CPU_USAGE,
    UDC_METRIC_RAM_STATE,
    UDC_METRIC_VFS_STATE
} udc_metric_type_t;

typedef enum {
    UDC_METRIC_OK = 0,
    UDC_METRIC_ERR_MEMORY,
    UDC_METRIC_ERR_NOT_MOUNTED,
    UDC_METRIC_ERR_NOT_SUPPORTED
} udc_metric_status_t;

typedef enum {
    UDC_VFS_TYPE_FAT,     // SD Cards, FATFS
    UDC_VFS_TYPE_LITTLEFS // LittleFS
} udc_vfs_type_t;

typedef enum {
    UDC_METRIC_UNIT_BYTES = 0,
    UDC_METRIC_UNIT_KB,
    UDC_METRIC_UNIT_MB,
    UDC_METRIC_UNIT_GB
} udc_metric_size_unit_t;

/* --- 2. Configuraciones (Entrada) --- */

typedef struct {
    int core_id; /* -1 para Global, 0 para PRO_CPU, 1 para APP_CPU */
} udc_metric_cfg_cpu_t;

typedef struct {
    uint32_t memory_caps; /* MALLOC_CAP_INTERNAL o MALLOC_CAP_SPIRAM */
} udc_metric_cfg_ram_t;

typedef struct {
    udc_vfs_type_t fs_type;
    const char *identifier; // FAT: Usa el mount_point ("/sd"). SPIFFS/LFS: Usa el partition_label ("storage")
} udc_metric_cfg_vfs_t;

typedef struct {
    bool run_on_start; /* Se ejecuta al menos una vez */
    bool enabled_by_default; /* Estado por default, true para START y false para PAUSE*/
    union {
        udc_metric_cfg_cpu_t cpu;
        udc_metric_cfg_ram_t ram;
        udc_metric_cfg_vfs_t vfs;
    };
} udc_metric_config_t;

/* --- 3. Datos (Salida) --- */

typedef struct {
    uint8_t percent_used;
} udc_metric_cpu_data_t;

typedef struct {
    uint32_t total;
    uint32_t used;
    uint32_t min_used_ever;
    udc_metric_size_unit_t unit;
} udc_metric_ram_data_t;

typedef struct {
    uint32_t total;
    uint32_t used;
    udc_metric_size_unit_t unit;
} udc_metric_vfs_data_t;

typedef struct {
    udc_metric_type_t type;
    udc_metric_status_t status;
    union {
        udc_metric_cpu_data_t cpu;
        udc_metric_ram_data_t ram;
        udc_metric_vfs_data_t vfs;
    } data;
} udc_metric_event_t;

/* --- 4. API Pública --- */

typedef struct udc_metric_sub_t* udc_metric_handle_t; 
typedef void (*udc_metric_callback_t)(const udc_metric_event_t *event, void *user_ctx);

app_desc_status_t* udc_metrics_init(void);

udc_metric_handle_t udc_metrics_subscribe(
    udc_metric_type_t type, 
    uint32_t interval_ms, 
    uint8_t priority, 
    const udc_metric_config_t *config,
    udc_metric_callback_t cb, 
    void *user_ctx
);

/**
 * @brief Pausa o reanuda una métrica eficientemente.
 * 
 * @param handle El handle de la métrica a pausar/reanudar.
 * @param pause  true para pausar, false para reanudar.
 * @return ESP_OK si fue exitoso, ESP_ERR_INVALID_ARG si el handle es nulo.
 */
esp_err_t udc_metrics_set_paused(udc_metric_handle_t handle, bool pause);

esp_err_t udc_metrics_unsubscribe(udc_metric_handle_t handle);

#ifdef __cplusplus
}
#endif