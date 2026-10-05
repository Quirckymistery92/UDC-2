/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * Touch Driver - GSL3680 I2C Implementation
 */

#include "display_touch.h"
#include "i2c_controller.h"

#include "esp_log.h"
#include "esp_check.h"
#include "esp_lcd_touch_gsl3680.h"

static const char * const TAG = "display_touch";

/* ────────────────── Touch Init ────────────────── */

esp_err_t display_touch_init(esp_lcd_touch_handle_t *ret_touch)
{
    /* GSL3680 I2C address config - required for proper init sequence */
    static esp_lcd_touch_io_gsl3680_config_t gsl3680_io_cfg = {
        .dev_addr = ESP_LCD_TOUCH_IO_I2C_GSL3680_ADDRESS,
    };

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max       = TOUCH_H_RES,
        .y_max       = TOUCH_V_RES,
        .rst_gpio_num = TOUCH_GPIO_RST,
        .int_gpio_num = TOUCH_GPIO_INT,
        .levels = {
            .reset     = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy  = 0, // Disable swapping X and Y axes
            .mirror_x = 1, // Enable mirroring X-axis
            .mirror_y = 0, // Disable mirroring Y-axis
        },
        .driver_data = &gsl3680_io_cfg,
    };

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    i2c_master_bus_handle_t i2c_handle = NULL;
    ESP_RETURN_ON_ERROR(i2c_controller_get_bus_handle(&i2c_handle),
                        TAG, "Failed to get I2C bus handle");

    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GSL3680_CONFIG();
    tp_io_config.scl_speed_hz = TOUCH_I2C_CLK_SPEED_HZ;

    ESP_RETURN_ON_ERROR(
        esp_lcd_new_panel_io_i2c(i2c_handle, &tp_io_config, &tp_io_handle),
        TAG, "Touch panel IO init failed"
    );

    ESP_RETURN_ON_ERROR(
        esp_lcd_touch_new_i2c_gsl3680(tp_io_handle, &tp_cfg, ret_touch),
        TAG, "GSL3680 touch init failed"
    );

    ESP_LOGI(TAG, "Touch GSL3680 initialized (res=%dx%d)", TOUCH_H_RES, TOUCH_V_RES);
    return ESP_OK;
}
