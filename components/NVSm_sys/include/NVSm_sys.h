#pragma once

/* ESP */
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"

/* Módulos */
#include "desc_apps.h"

#ifdef __cplusplus
extern "C" {
#endif

// ==========================================
// CORE FUNCIONES
// ==========================================

/**
 * @brief Inicializa el NVS y el Mutex de protección
 */
app_desc_status_t* NVSm_sys_init(void);

/**
 * @brief Desinicializa NVS (principalmente para testing)
 */
esp_err_t NVSm_sys_deinit(void);

bool NVSm_sys_is_initialized(void);
#ifdef __cplusplus
}
#endif