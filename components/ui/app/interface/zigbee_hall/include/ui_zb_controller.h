/**
 * @file ui_zb_controller.h
 * @brief Controlador centralizado para manipulación manual del Grid de Zigbee.
 */
#pragma once

#include "esp_err.h"
#include "ui_zb_types.h"

// Registra el dispositivo requiriendo columna (X) y fila (Y) exacta para el Grid.
esp_err_t ui_zb_device_register(uint16_t id, const char *name, ui_zb_type_dev_t type, uint8_t col, uint8_t row);

// Libera todos los recursos (lv_free, lv_obj_delete) y deja el slot vacío sin alterar el resto del Grid.
esp_err_t ui_zb_device_remove(uint16_t id);

// Actualiza el estado o la data del dispositivo usando la unión. Filtra por type internamente.
esp_err_t ui_zb_device_send_data(uint16_t id, const ui_zb_update_data_t *update);

// Muestra/Oculta el spinner de carga
esp_err_t ui_zb_device_set_loading(uint16_t id, bool is_loading);