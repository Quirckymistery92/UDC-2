#pragma once

#include <stdint.h>
#include "esp_err.h"

// Guarda el índice de la zona horaria en NVS
esp_err_t NVSm_udc_time_save_tz_index(uint8_t tz_index);

// Carga el índice de la zona horaria desde NVS
esp_err_t NVSm_udc_time_load_tz_index(uint8_t *tz_index);