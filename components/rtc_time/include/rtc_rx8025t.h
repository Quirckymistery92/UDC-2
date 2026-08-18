/*
 * SPDX-FileCopyrightText: 2026
 * SPDX-License-Identifier: Apache-2.0
 *
 * RX8025T RTC driver - ESP-IDF 5.x implementation.
 * Thread-safe, background sync task, decoupled architecture.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Eventos emitidos por la Task interna de sincronización.
 */
typedef enum {
    RTC_RX8025T_EVENT_SYNC_OK,      /*!< Sincronización exitosa del RTC al OS */
    RTC_RX8025T_EVENT_POWER_FAIL,   /*!< Fallo de energía en RTC (VLF=1 o XSTP=1), requiere set_time() */
    RTC_RX8025T_EVENT_I2C_ERROR     /*!< Error de comunicación en el bus I2C */
} rtc_rx8025t_event_t;

/**
 * @brief Callback opcional para notificar a la aplicación eventos de la Task.
 * 
 * @param event Evento sucedido.
 * @param time_info Puntero con la hora recién leída (válido solo si EVENT_SYNC_OK).
 */
typedef void (*rtc_rx8025t_cb_t)(rtc_rx8025t_event_t event, const struct tm *time_info);

/**
 * @brief Inicializa el driver, registra en el I2C bus y levanta la Task.
 * 
 * @param sync_interval_sec Intervalo de sincronización en segundos (ej. 3600 para 1h). 
 *                          Si es 0, no se crea la Task (solo lectura/escritura manual).
 * @param cb                Callback de eventos (puede ser NULL).
 * @return ESP_OK si fue exitoso.
 */
esp_err_t rtc_rx8025t_init(uint32_t sync_interval_sec, rtc_rx8025t_cb_t cb);

/**
 * @brief Desinicializa el driver, destruye la Task de forma segura y libera el bus.
 */
esp_err_t rtc_rx8025t_deinit(void);

/**
 * @brief Despierta a la Task para que sincronice la hora inmediatamente.
 */
esp_err_t rtc_rx8025t_force_sync(void);

/**
 * @brief Lee la hora directamente del chip (Thread-safe).
 */
esp_err_t rtc_rx8025t_get_time(struct tm *t);

/**
 * @brief Escribe una nueva hora en el RTC, limpia banderas de error y sincroniza el OS.
 */
esp_err_t rtc_rx8025t_set_time(const struct tm *t);

/**
 * @brief Verifica si los datos del RTC son válidos (batería OK y oscilación estable).
 */
bool rtc_rx8025t_is_valid(void);

#ifdef __cplusplus
}
#endif