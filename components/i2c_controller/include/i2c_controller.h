/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * I2C Controller - shared I2C master bus wrapper.
 */

#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Shared bus pin/port configuration ── */
#define I2C_CTRL_PORT   (I2C_NUM_1)
#define I2C_CTRL_SDA    (GPIO_NUM_7)
#define I2C_CTRL_SCL    (GPIO_NUM_8)

esp_err_t i2c_controller_init(void);
esp_err_t i2c_controller_get_bus_handle(i2c_master_bus_handle_t *ret_handle);
esp_err_t i2c_controller_add_device(uint16_t dev_addr, uint32_t clk_hz,
                                     i2c_master_dev_handle_t *ret_handle);
esp_err_t i2c_controller_remove_device(i2c_master_dev_handle_t dev_handle);
esp_err_t i2c_controller_deinit(void);

#ifdef __cplusplus
}
#endif
