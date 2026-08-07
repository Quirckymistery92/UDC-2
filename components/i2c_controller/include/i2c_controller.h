/**
 * @file i2c_controller.h
 * @brief Controlador I2C maestro compartido por los componentes del proyecto.
 *
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Puerto I2C utilizado por el bus maestro compartido. */
#define I2C_CTRL_PORT   (I2C_NUM_1)
#define I2C_CTRL_SDA    (GPIO_NUM_7)
#define I2C_CTRL_SCL    (GPIO_NUM_8)

/**
 * @brief Inicializa el bus I2C maestro compartido.
 *
 * Si el bus ya fue inicializado, la función no realiza cambios y devuelve
 * @c ESP_OK. Utiliza el puerto y los pines definidos por las macros
 * @c I2C_CTRL_PORT, @c I2C_CTRL_SDA e @c I2C_CTRL_SCL.
 *
 * @return @c ESP_OK si el bus quedó disponible o ya estaba inicializado.
 * @return Código de error de ESP-IDF si no se pudo crear el bus.
 */
esp_err_t i2c_controller_init(void);

/**
 * @brief Obtiene el handle del bus I2C compartido.
 *
 * @param[out] ret_handle Puntero donde se almacena el handle del bus.
 * @return @c ESP_OK si el handle fue obtenido.
 * @return @c ESP_ERR_INVALID_STATE si el bus todavía no fue inicializado.
 */
esp_err_t i2c_controller_get_bus_handle(i2c_master_bus_handle_t *ret_handle);

/**
 * @brief Registra un dispositivo I2C en el bus compartido.
 *
 * El dispositivo se configura con direccionamiento de 7 bits y la frecuencia
 * SCL indicada por el llamador.
 *
 * @param[in] dev_addr Dirección I2C de 7 bits del dispositivo.
 * @param[in] clk_hz Frecuencia del reloj SCL en hercios.
 * @param[out] ret_handle Puntero donde se almacena el handle del dispositivo.
 * @return @c ESP_OK si el dispositivo fue registrado.
 * @return @c ESP_ERR_INVALID_STATE si el bus no fue inicializado.
 * @return Código de error de ESP-IDF si el registro falla.
 */
esp_err_t i2c_controller_add_device(uint16_t dev_addr, uint32_t clk_hz,
                                     i2c_master_dev_handle_t *ret_handle);

/**
 * @brief Elimina un dispositivo previamente registrado en el bus.
 *
 * @param[in] dev_handle Handle devuelto por
 *                       @ref i2c_controller_add_device.
 * @return @c ESP_OK si el dispositivo fue eliminado.
 * @return @c ESP_ERR_INVALID_ARG si el handle es nulo.
 * @return Código de error de ESP-IDF si la eliminación falla.
 */
esp_err_t i2c_controller_remove_device(i2c_master_dev_handle_t dev_handle);

/**
 * @brief Destruye el bus I2C maestro compartido.
 *
 * Debe llamarse después de eliminar los dispositivos registrados. Si el bus
 * ya estaba destruido, la función devuelve @c ESP_OK sin realizar cambios.
 *
 * @return @c ESP_OK si el bus fue destruido o ya no estaba inicializado.
 * @return Código de error de ESP-IDF si no se pudo destruir el bus.
 */
esp_err_t i2c_controller_deinit(void);

#ifdef __cplusplus
}
#endif