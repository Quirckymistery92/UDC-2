#pragma once
#include <sys/time.h>
#include <time.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================= CONTROL ================= */

/**
 * @brief Inicializador del sistema de reloj. (Encapsulado set_timezone)
 * 
 */
esp_err_t udc_time_init(uint8_t tz_index);

/**
 * @brief Configurador de timezone para el reloj interno.
 * 
 * @param tz_index Véase @see udc_time.c
 * 0: Default
 * 1: Argentina
 * 2: Chile
 * 3: Brasil 
 * @return esp_err_t ESP_ERR_NOT_SUPPORTED: Operación fallida.
 * @return esp_err_t ESP_OK: Operación correcta
 */
esp_err_t udc_time_set_timezone(uint8_t tz_index);

/**
 * @brief Sincronizador async del RTC al interno.
 * 
 * @return esp_err_t ESP_ERR_INVALID_STATE: Falló al comunicarse con la tarea interna.
 * @return esp_err_t ESP_OK: Operación notificada a la task.
 */
esp_err_t udc_time_sync_system_from_rtc(void);

/* ================= INTERNAL RTC ================= */

/**
 * @brief Getter para el tiempo formato time_t. UB: Interno
 * 
 * @param out_time Devuelve el dato solicitado
 *
 * @return esp_err_t ESP_ERR_INVALID_ARG: Parámetros inválidos.
 * @return esp_err_t ESP_OK: Operación correcta.
 */
esp_err_t udc_time_get_int_time_t(time_t *out_time);

/**
 * @brief Getter para el tiempo formato struct tm. UB: Interno
 * 
 * @param out_time Devuelve el dato solicitado
 *
 * @return esp_err_t ESP_ERR_INVALID_ARG: Parámetros inválidos.
 * @return esp_err_t ESP_OK: Operación correcta.
 */
esp_err_t udc_time_get_int_struct_tm(struct tm *out_time);

/**
 * @brief Setter para el tiempo formato time_t. UB: Interno
 * 
 * @param set_time Toma la hora solicitada.
 *
 * @return esp_err_t ESP_ERR_INVALID_ARG: Parámetros inválidos.
 * @return esp_err_t ESP_OK: Operación correcta.
 */
esp_err_t udc_time_set_int_time_t(time_t set_time);

/**
 * @brief Setter para el tiempo formato struct tm. UB: Interno
 * 
 * @param set_time Toma la hora solicitada.
 *
 * @return esp_err_t ESP_ERR_INVALID_ARG: Parámetros inválidos.
 * @return esp_err_t ESP_OK: Operación correcta.
 */
esp_err_t udc_time_set_int_struct_tm(const struct tm *set_time);

/* ================= EXTERNAL RTC ================= */

/**
 * @brief Getter para el tiempo formato time_t. UB: External (i2c read pipeline) TZ ignored
 * 
 * @param out_time Devuelve el dato solicitado
 *
 * @return esp_err_t ESP_ERR_INVALID_ARG: Parámetros inválidos.
 * @return esp_err_t ESP_OK: Operación correcta.
 */
esp_err_t udc_time_get_ext_time_t(time_t *out_time);

/**
 * @brief Getter para el tiempo formato struct tm. UB: External (i2c read pipeline) TZ ignored
 * 
 * @param out_time Devuelve el dato solicitado
 *
 * @return esp_err_t ESP_ERR_INVALID_ARG: Parámetros inválidos.
 * @return esp_err_t ESP_OK: Operación correcta.
 */
esp_err_t udc_time_get_ext_struct_tm(struct tm *out_time);

/**
 * @brief Setter para el tiempo formato time_t. UB: External (i2c set pipeline) TZ ignored
 * 
 * @param set_time Toma la hora solicitada.
 *
 * @return esp_err_t ESP_ERR_INVALID_ARG: Parámetros inválidos.
 * @return esp_err_t ESP_OK: Operación correcta.
 */
esp_err_t udc_time_set_ext_time_t(time_t set_time);

/**
 * @brief Setter para el tiempo formato struct tm. UB: External (i2c set pipeline) TZ ignored
 * 
 * @param set_time Toma la hora solicitada.
 *
 * @return esp_err_t ESP_ERR_INVALID_ARG: Parámetros inválidos.
 * @return esp_err_t ESP_OK: Operación correcta.
 */
esp_err_t udc_time_set_ext_struct_tm(const struct tm *set_time);

#ifdef __cplusplus
}
#endif