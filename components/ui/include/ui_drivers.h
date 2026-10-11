#pragma once

/* ESP */
#include "esp_err.h"

/* Módulos */
#include "ui_image_sia.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════
 *  Drivers
 * ══════════════════════════════════════════════════════════════════ */
esp_err_t ui__drivers_state_push_wifi(const char* driver_message, bool driver_ok, ui_icon_id_t driver_icon);

esp_err_t ui__drivers_state_push_zigbee(const char* driver_message, bool driver_ok);

esp_err_t ui__drivers_battery_set_status(int percentage, bool is_charging, bool driver_ok);

#ifdef __cplusplus
}
#endif