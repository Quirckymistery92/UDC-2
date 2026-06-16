#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "user_manager_nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

// ==========================================
// CORE FUNCIONES
// ==========================================

/**
 * @brief Inicializa el NVS y el Mutex de protección
 */
esp_err_t NVSm_sys_init(void);

/**
 * @brief Desinicializa NVS (principalmente para testing)
 */
esp_err_t NVSm_sys_deinit(void);

/**
 * @brief Helpers de concurrencia expuestos para los submódulos
 */
esp_err_t NVSm_sys_lock(void);
void NVSm_sys_unlock(void);


// ==========================================
// INCLUDES DE SUBMÓDULOS (Plug & Play)
// ==========================================
// Si un módulo ya no se usa, simplemente comentas o eliminas su include aquí
// y lo retiras de la carpeta modules/

// ...

#ifdef __cplusplus
}
#endif