#pragma once

#include "boot_core.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize NORMAL boot mode
 * 
 * Standard boot with essential components.
 * Uses default configuration for production use.
 * 
 * @return ESP_OK if initialization successful, error code otherwise
 */
esp_err_t boot_normal_init(void);

/**
 * @brief Execute NORMAL boot sequence
 * 
 * Initializes all mandatory components for normal operation
 * 
 * @return ESP_OK if boot successful, error code otherwise
 */
esp_err_t boot_normal_execute(void);

#ifdef __cplusplus
}
#endif
