/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * I2C Controller - shared I2C master bus implementation.
 */

#include "i2c_controller.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "i2c_ctrl";

static i2c_master_bus_handle_t s_bus_handle = NULL;

esp_err_t i2c_controller_init(void)
{
    if (s_bus_handle != NULL) {
        return ESP_OK;
    }

    i2c_master_bus_config_t cfg = {
        .i2c_port          = I2C_CTRL_PORT,
        .sda_io_num        = I2C_CTRL_SDA,
        .scl_io_num        = I2C_CTRL_SCL,
        .clk_source        = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority     = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&cfg, &s_bus_handle),
                        TAG, "Bus init failed");
    ESP_LOGI(TAG, "I2C master bus ready (port=%d SDA=%d SCL=%d)",
             I2C_CTRL_PORT, I2C_CTRL_SDA, I2C_CTRL_SCL);
    return ESP_OK;
}

esp_err_t i2c_controller_get_bus_handle(i2c_master_bus_handle_t *ret_handle)
{
    ESP_RETURN_ON_FALSE(s_bus_handle != NULL, ESP_ERR_INVALID_STATE,
                        TAG, "Bus not initialized");
    *ret_handle = s_bus_handle;
    return ESP_OK;
}

esp_err_t i2c_controller_add_device(uint16_t dev_addr, uint32_t clk_hz,
                                     i2c_master_dev_handle_t *ret_handle)
{
    ESP_RETURN_ON_FALSE(s_bus_handle != NULL, ESP_ERR_INVALID_STATE,
                        TAG, "Bus not initialized");
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = dev_addr,
        .scl_speed_hz    = clk_hz,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_bus_handle, &dev_cfg, ret_handle),
                        TAG, "Add device 0x%02X failed", dev_addr);
    ESP_LOGI(TAG, "Device 0x%02X registered at %lu Hz", (unsigned)dev_addr, (unsigned long)clk_hz);
    return ESP_OK;
}

esp_err_t i2c_controller_remove_device(i2c_master_dev_handle_t dev_handle)
{
    ESP_RETURN_ON_FALSE(dev_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "Null handle");
    ESP_RETURN_ON_ERROR(i2c_master_bus_rm_device(dev_handle),
                        TAG, "Remove device failed");
    return ESP_OK;
}

esp_err_t i2c_controller_deinit(void)
{
    if (s_bus_handle == NULL) {
        return ESP_OK;
    }
    ESP_RETURN_ON_ERROR(i2c_del_master_bus(s_bus_handle), TAG, "Bus deinit failed");
    s_bus_handle = NULL;
    ESP_LOGI(TAG, "I2C master bus destroyed");
    return ESP_OK;
}
