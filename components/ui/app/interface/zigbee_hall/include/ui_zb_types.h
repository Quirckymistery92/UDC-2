/**
 * @file ui_zb_types.h
 * @brief Definiciones de tipos, unión de datos y configuración del Grid Zigbee.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "lvgl.h" // Incluir directamente lvgl para lv_malloc

#define GRID_MAX_X 5
#define GRID_MAX_Y 4
#define MAX_DEVICES_UX (GRID_MAX_X * GRID_MAX_Y)

#define MAX_DEV_NAME_LEN 31
#define MAX_TEMP_STR_LEN 12 // Ej: "-99.9 °C\0"

// Tipos de dispositivos soportados
typedef enum {
    UX_HOME_DEV_TYPE_LIGHT_ON_OFF       = 0x0100,
    UX_HOME_DEV_TYPE_LIGHT_DIMMER       = 0x0101,
    UX_HOME_DEV_TYPE_LIGHT_DIMMER_RGB   = 0x0102,
    UX_HOME_DEV_TYPE_SWITCH_ON_OFF      = 0x0000,
    UX_HOME_DEV_TYPE_SWITCH_DIMMER      = 0x0001,
    UX_HOME_DEV_TYPE_SENSOR_TEMPERATURE = 0x0302,
    UX_HOME_DEV_TYPE_LIGHT_SENSOR       = 0x0301,
    UX_HOME_DEV_TYPE_INTERNET_CONECTION = 0xC000
} ui_zb_type_dev_t;

#define IS_LIGHT_DEV(dat) ( \
    (dat) == UX_HOME_DEV_TYPE_LIGHT_ON_OFF || \
    (dat) == UX_HOME_DEV_TYPE_LIGHT_DIMMER || \
    (dat) == UX_HOME_DEV_TYPE_LIGHT_DIMMER_RGB \
)
#define IS_SWITCH_DEV(dat) ( \
    (dat) == UX_HOME_DEV_TYPE_SWITCH_ON_OFF || \
    (dat) == UX_HOME_DEV_TYPE_SWITCH_DIMMER \
)

// Unión de datos para aislar la carga útil por tipo
typedef union {
    struct {
        uint8_t state; // 0: Off, 1: On, >1: Dimmer value
    } light;
    struct {
        float temp_c;
        float temp_f;
    } temp;
    struct {
        int32_t level; // Nivel del barra del sensor de luz
    } light_sensor;
} ui_zb_dev_data_u;

// Struct transaccional para actualizar los dispositivos
typedef struct {
    bool status_ok;  // false = offline/fallo -> aplica DISABLED
    bool has_data;   // true = procesar union, false = solo actualizar status_ok
    ui_zb_dev_data_u data;
} ui_zb_update_data_t;

// Callback de error por incompatibilidad de datos/tipos
typedef void (*ui_zb_callback_result_t)(uint16_t id, esp_err_t result);