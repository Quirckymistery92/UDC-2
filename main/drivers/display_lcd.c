/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * Display LCD Driver - JD9365 DSI Implementation
 */

#include "display_lcd.h"

/* ESP */
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_ldo_regulator.h"

/* Módulos */
#include "esp_lcd_jd9365.h"
#include "desc_apps.h"

/* Variables */
    #define TAG "display_lcd"

    static app_desc_status_t app = {
        .app_desc = DSC_DRIVER_BRIGHTNESS,
        .app_name = "Driver Backlight",
        .app_tag = TAG,
        .init_result = ESP_OK
    };

static const jd9365_lcd_init_cmd_t lcd_init_cmds[] = {
    /* ── Page 0 ── */
    {0xE0, (const uint8_t[]){0x00}, 1, 0},
    {0xE1, (const uint8_t[]){0x93}, 1, 0},
    {0xE2, (const uint8_t[]){0x65}, 1, 0},
    {0xE3, (const uint8_t[]){0xF8}, 1, 0},
    {0x80, (const uint8_t[]){0x01}, 1, 0},

    /* ── Page 1: Power / VCOM / Gamma ── */
    {0xE0, (const uint8_t[]){0x01}, 1, 0},
    {0x00, (const uint8_t[]){0x00}, 1, 0},
    {0x01, (const uint8_t[]){0x39}, 1, 0},
    {0x03, (const uint8_t[]){0x10}, 1, 0},
    {0x04, (const uint8_t[]){0x41}, 1, 0},

    {0x0C, (const uint8_t[]){0x74}, 1, 0},
    {0x17, (const uint8_t[]){0x00}, 1, 0},
    {0x18, (const uint8_t[]){0xD7}, 1, 0},
    {0x19, (const uint8_t[]){0x00}, 1, 0},
    {0x1A, (const uint8_t[]){0x00}, 1, 0},

    {0x1B, (const uint8_t[]){0xD7}, 1, 0},
    {0x1C, (const uint8_t[]){0x00}, 1, 0},
    {0x24, (const uint8_t[]){0xFE}, 1, 0},
    {0x35, (const uint8_t[]){0x26}, 1, 0},
    {0x37, (const uint8_t[]){0x69}, 1, 0},

    {0x38, (const uint8_t[]){0x05}, 1, 0},
    {0x39, (const uint8_t[]){0x06}, 1, 0},
    {0x3A, (const uint8_t[]){0x08}, 1, 0},
    {0x3C, (const uint8_t[]){0x78}, 1, 0},
    {0x3D, (const uint8_t[]){0xFF}, 1, 0},

    {0x3E, (const uint8_t[]){0xFF}, 1, 0},
    {0x3F, (const uint8_t[]){0xFF}, 1, 0},
    {0x40, (const uint8_t[]){0x06}, 1, 0},
    {0x41, (const uint8_t[]){0xA0}, 1, 0},
    {0x43, (const uint8_t[]){0x14}, 1, 0},

    {0x44, (const uint8_t[]){0x0B}, 1, 0},
    {0x45, (const uint8_t[]){0x30}, 1, 0},
    {0x4B, (const uint8_t[]){0x04}, 1, 0},
    {0x55, (const uint8_t[]){0x02}, 1, 0},
    {0x57, (const uint8_t[]){0x89}, 1, 0},

    {0x59, (const uint8_t[]){0x0A}, 1, 0},
    {0x5A, (const uint8_t[]){0x28}, 1, 0},

    /* ── Positive Gamma ── */
    {0x5B, (const uint8_t[]){0x15}, 1, 0},
    {0x5D, (const uint8_t[]){0x50}, 1, 0},
    {0x5E, (const uint8_t[]){0x37}, 1, 0},
    {0x5F, (const uint8_t[]){0x29}, 1, 0},
    {0x60, (const uint8_t[]){0x1E}, 1, 0},

    {0x61, (const uint8_t[]){0x1D}, 1, 0},
    {0x62, (const uint8_t[]){0x12}, 1, 0},
    {0x63, (const uint8_t[]){0x1A}, 1, 0},
    {0x64, (const uint8_t[]){0x08}, 1, 0},
    {0x65, (const uint8_t[]){0x25}, 1, 0},

    {0x66, (const uint8_t[]){0x26}, 1, 0},
    {0x67, (const uint8_t[]){0x28}, 1, 0},
    {0x68, (const uint8_t[]){0x49}, 1, 0},
    {0x69, (const uint8_t[]){0x3A}, 1, 0},
    {0x6A, (const uint8_t[]){0x43}, 1, 0},

    {0x6B, (const uint8_t[]){0x3A}, 1, 0},
    {0x6C, (const uint8_t[]){0x3B}, 1, 0},
    {0x6D, (const uint8_t[]){0x32}, 1, 0},
    {0x6E, (const uint8_t[]){0x1F}, 1, 0},
    {0x6F, (const uint8_t[]){0x0E}, 1, 0},

    /* ── Negative Gamma ── */
    {0x70, (const uint8_t[]){0x50}, 1, 0},
    {0x71, (const uint8_t[]){0x37}, 1, 0},
    {0x72, (const uint8_t[]){0x29}, 1, 0},
    {0x73, (const uint8_t[]){0x1E}, 1, 0},
    {0x74, (const uint8_t[]){0x1D}, 1, 0},

    {0x75, (const uint8_t[]){0x12}, 1, 0},
    {0x76, (const uint8_t[]){0x1A}, 1, 0},
    {0x77, (const uint8_t[]){0x08}, 1, 0},
    {0x78, (const uint8_t[]){0x25}, 1, 0},
    {0x79, (const uint8_t[]){0x26}, 1, 0},

    {0x7A, (const uint8_t[]){0x28}, 1, 0},
    {0x7B, (const uint8_t[]){0x49}, 1, 0},
    {0x7C, (const uint8_t[]){0x3A}, 1, 0},
    {0x7D, (const uint8_t[]){0x43}, 1, 0},
    {0x7E, (const uint8_t[]){0x3A}, 1, 0},

    {0x7F, (const uint8_t[]){0x3B}, 1, 0},
    {0x80, (const uint8_t[]){0x32}, 1, 0},
    {0x81, (const uint8_t[]){0x1F}, 1, 0},
    {0x82, (const uint8_t[]){0x0E}, 1, 0},

    /* ── Page 2: GIP (Gate In Panel) Routing ── */
    {0xE0, (const uint8_t[]){0x02}, 1, 0},

    {0x00, (const uint8_t[]){0x1F}, 1, 0},
    {0x01, (const uint8_t[]){0x1F}, 1, 0},
    {0x02, (const uint8_t[]){0x52}, 1, 0},
    {0x03, (const uint8_t[]){0x51}, 1, 0},
    {0x04, (const uint8_t[]){0x50}, 1, 0},

    {0x05, (const uint8_t[]){0x4B}, 1, 0},
    {0x06, (const uint8_t[]){0x4A}, 1, 0},
    {0x07, (const uint8_t[]){0x49}, 1, 0},
    {0x08, (const uint8_t[]){0x48}, 1, 0},
    {0x09, (const uint8_t[]){0x47}, 1, 0},

    {0x0A, (const uint8_t[]){0x46}, 1, 0},
    {0x0B, (const uint8_t[]){0x45}, 1, 0},
    {0x0C, (const uint8_t[]){0x44}, 1, 0},
    {0x0D, (const uint8_t[]){0x40}, 1, 0},
    {0x0E, (const uint8_t[]){0x41}, 1, 0},

    {0x0F, (const uint8_t[]){0x1F}, 1, 0},
    {0x10, (const uint8_t[]){0x1F}, 1, 0},
    {0x11, (const uint8_t[]){0x1F}, 1, 0},
    {0x12, (const uint8_t[]){0x1F}, 1, 0},
    {0x13, (const uint8_t[]){0x1F}, 1, 0},

    {0x14, (const uint8_t[]){0x1F}, 1, 0},
    {0x15, (const uint8_t[]){0x1F}, 1, 0},
    {0x16, (const uint8_t[]){0x1F}, 1, 0},
    {0x17, (const uint8_t[]){0x1F}, 1, 0},
    {0x18, (const uint8_t[]){0x52}, 1, 0},

    {0x19, (const uint8_t[]){0x51}, 1, 0},
    {0x1A, (const uint8_t[]){0x50}, 1, 0},
    {0x1B, (const uint8_t[]){0x4B}, 1, 0},
    {0x1C, (const uint8_t[]){0x4A}, 1, 0},
    {0x1D, (const uint8_t[]){0x49}, 1, 0},

    {0x1E, (const uint8_t[]){0x48}, 1, 0},
    {0x1F, (const uint8_t[]){0x47}, 1, 0},
    {0x20, (const uint8_t[]){0x46}, 1, 0},
    {0x21, (const uint8_t[]){0x45}, 1, 0},
    {0x22, (const uint8_t[]){0x44}, 1, 0},

    {0x23, (const uint8_t[]){0x40}, 1, 0},
    {0x24, (const uint8_t[]){0x41}, 1, 0},
    {0x25, (const uint8_t[]){0x1F}, 1, 0},
    {0x26, (const uint8_t[]){0x1F}, 1, 0},
    {0x27, (const uint8_t[]){0x1F}, 1, 0},

    {0x28, (const uint8_t[]){0x1F}, 1, 0},
    {0x29, (const uint8_t[]){0x1F}, 1, 0},
    {0x2A, (const uint8_t[]){0x1F}, 1, 0},
    {0x2B, (const uint8_t[]){0x1F}, 1, 0},
    {0x2C, (const uint8_t[]){0x1F}, 1, 0},

    {0x2D, (const uint8_t[]){0x1F}, 1, 0},
    {0x2E, (const uint8_t[]){0x52}, 1, 0},
    {0x2F, (const uint8_t[]){0x40}, 1, 0},
    {0x30, (const uint8_t[]){0x41}, 1, 0},
    {0x31, (const uint8_t[]){0x48}, 1, 0},

    {0x32, (const uint8_t[]){0x49}, 1, 0},
    {0x33, (const uint8_t[]){0x4A}, 1, 0},
    {0x34, (const uint8_t[]){0x4B}, 1, 0},
    {0x35, (const uint8_t[]){0x44}, 1, 0},
    {0x36, (const uint8_t[]){0x45}, 1, 0},

    {0x37, (const uint8_t[]){0x46}, 1, 0},
    {0x38, (const uint8_t[]){0x47}, 1, 0},
    {0x39, (const uint8_t[]){0x51}, 1, 0},
    {0x3A, (const uint8_t[]){0x50}, 1, 0},
    {0x3B, (const uint8_t[]){0x1F}, 1, 0},

    {0x3C, (const uint8_t[]){0x1F}, 1, 0},
    {0x3D, (const uint8_t[]){0x1F}, 1, 0},
    {0x3E, (const uint8_t[]){0x1F}, 1, 0},
    {0x3F, (const uint8_t[]){0x1F}, 1, 0},
    {0x40, (const uint8_t[]){0x1F}, 1, 0},

    {0x41, (const uint8_t[]){0x1F}, 1, 0},
    {0x42, (const uint8_t[]){0x1F}, 1, 0},
    {0x43, (const uint8_t[]){0x1F}, 1, 0},
    {0x44, (const uint8_t[]){0x52}, 1, 0},
    {0x45, (const uint8_t[]){0x40}, 1, 0},

    {0x46, (const uint8_t[]){0x41}, 1, 0},
    {0x47, (const uint8_t[]){0x48}, 1, 0},
    {0x48, (const uint8_t[]){0x49}, 1, 0},
    {0x49, (const uint8_t[]){0x4A}, 1, 0},
    {0x4A, (const uint8_t[]){0x4B}, 1, 0},

    {0x4B, (const uint8_t[]){0x44}, 1, 0},
    {0x4C, (const uint8_t[]){0x45}, 1, 0},
    {0x4D, (const uint8_t[]){0x46}, 1, 0},
    {0x4E, (const uint8_t[]){0x47}, 1, 0},
    {0x4F, (const uint8_t[]){0x51}, 1, 0},

    {0x50, (const uint8_t[]){0x50}, 1, 0},
    {0x51, (const uint8_t[]){0x1F}, 1, 0},
    {0x52, (const uint8_t[]){0x1F}, 1, 0},
    {0x53, (const uint8_t[]){0x1F}, 1, 0},
    {0x54, (const uint8_t[]){0x1F}, 1, 0},

    {0x55, (const uint8_t[]){0x1F}, 1, 0},
    {0x56, (const uint8_t[]){0x1F}, 1, 0},
    {0x57, (const uint8_t[]){0x1F}, 1, 0},

    /* ── GIP Timing ── */
    {0x58, (const uint8_t[]){0x40}, 1, 0},
    {0x59, (const uint8_t[]){0x00}, 1, 0},
    {0x5A, (const uint8_t[]){0x00}, 1, 0},
    {0x5B, (const uint8_t[]){0x10}, 1, 0},
    {0x5C, (const uint8_t[]){0x05}, 1, 0},
    {0x5D, (const uint8_t[]){0x50}, 1, 0},
    {0x5E, (const uint8_t[]){0x01}, 1, 0},

    {0x5F, (const uint8_t[]){0x02}, 1, 0},
    {0x60, (const uint8_t[]){0x50}, 1, 0},
    {0x61, (const uint8_t[]){0x06}, 1, 0},
    {0x62, (const uint8_t[]){0x04}, 1, 0},
    {0x63, (const uint8_t[]){0x03}, 1, 0},

    {0x64, (const uint8_t[]){0x64}, 1, 0},
    {0x65, (const uint8_t[]){0x65}, 1, 0},
    {0x66, (const uint8_t[]){0x0B}, 1, 0},
    {0x67, (const uint8_t[]){0x73}, 1, 0},
    {0x68, (const uint8_t[]){0x07}, 1, 0},

    {0x69, (const uint8_t[]){0x06}, 1, 0},
    {0x6A, (const uint8_t[]){0x64}, 1, 0},
    {0x6B, (const uint8_t[]){0x08}, 1, 0},
    {0x6C, (const uint8_t[]){0x00}, 1, 0},
    {0x6D, (const uint8_t[]){0x32}, 1, 0},

    {0x6E, (const uint8_t[]){0x08}, 1, 0},

    /* ── Page 4 ── */
    {0xE0, (const uint8_t[]){0x04}, 1, 0},
    {0x2C, (const uint8_t[]){0x6B}, 1, 0},
    {0x35, (const uint8_t[]){0x08}, 1, 0},
    {0x37, (const uint8_t[]){0x00}, 1, 0},

    /* ── Page 0: Display On sequence ── */
    {0xE0, (const uint8_t[]){0x00}, 1, 0},
    {0x11, (const uint8_t[]){0x00}, 1, 0},
    {0x29, (const uint8_t[]){0x00}, 1, 0},      /* Display ON + 5ms */
    {0x11, (const uint8_t[]){0x00}, 1, 120},    /* Sleep Out + 120ms */
    {0x35, (const uint8_t[]){0x00}, 1, 0},      /* Tearing Effect Line ON */
};

/* ────────────────── Backlight (LEDC) ────────────────── */

app_desc_status_t* display_lcd_brightness_init(void)
{
    const ledc_timer_config_t bl_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num       = LEDC_TIMER_1,
        .freq_hz         = 5000,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    esp_err_t ret = ledc_timer_config(&bl_timer);
    DSC_RETURN_STRUCT_ON_FALSE(ret == ESP_OK, &app, ret);

    const ledc_channel_config_t bl_channel = {
        .gpio_num   = LCD_GPIO_BL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LCD_BL_LEDC_CH,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER_1,
        .duty       = 0,
        .hpoint     = 0,
        .flags.output_invert = 0
    };
    ret = ledc_channel_config(&bl_channel);
    DSC_RETURN_STRUCT_ON_FALSE(ret == ESP_OK, &app, ret);

    return DESC_AP_INSERT(&app, ret);
}

esp_err_t display_lcd_brightness_set(uint8_t brightness_percent)
{
    if (brightness_percent > 100) brightness_percent = 100;

    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness_percent);
    uint32_t duty = (1023 * brightness_percent) / 100;
    ESP_RETURN_ON_ERROR(ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_BL_LEDC_CH, duty), TAG, "Set duty failed");
    ESP_RETURN_ON_ERROR(ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_BL_LEDC_CH), TAG, "Update duty failed");
    return ESP_OK;
}

esp_err_t display_lcd_backlight_on(void)
{
    return display_lcd_brightness_set(100);
}

esp_err_t display_lcd_backlight_off(void)
{
    return display_lcd_brightness_set(0);
}

/* ────────────────── MIPI-DSI PHY Power ────────────────── */

static esp_err_t enable_dsi_phy_power(void)
{
    static esp_ldo_channel_handle_t phy_pwr_chan = NULL;
    esp_ldo_channel_config_t ldo_cfg = {
        .chan_id    = 3,
        .voltage_mv = 2500,
    };
    ESP_RETURN_ON_ERROR(esp_ldo_acquire_channel(&ldo_cfg, &phy_pwr_chan), TAG, "Acquire LDO for DPHY failed");
    return ESP_OK;
}

/* ────────────────── LCD Panel Init ────────────────── */

esp_err_t display_lcd_init(lcd_handles_t *ret_handles)
{
    esp_err_t ret = ESP_OK;

    ESP_RETURN_ON_ERROR(enable_dsi_phy_power(), TAG, "DSI PHY power failed");

    esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL;
    esp_lcd_dsi_bus_config_t bus_config = {
        .bus_id = 0,
        .lane_bit_rate_mbps = 800,
        .num_data_lanes = 2,
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus), TAG, "New DSI bus failed");

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_dbi_io_config_t dbi_config = JD9365_PANEL_IO_DBI_CONFIG();
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &io), err, TAG, "New panel IO failed");

    esp_lcd_dpi_panel_config_t dpi_config = {
        .dpi_clk_src        = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = 45,
        .virtual_channel    = 0,
        .pixel_format       = LCD_COLOR_PIXEL_FORMAT_RGB565,
        .num_fbs            = LCD_DPI_BUFFER_NUMS,
        .video_timing = {
            .h_size             = LCD_H_RES,
            .v_size             = LCD_V_RES,
            .hsync_back_porch   = 80,
            .hsync_pulse_width  = 80,
            .hsync_front_porch  = 20,
            .vsync_back_porch   = 36,
            .vsync_pulse_width  = 20,
            .vsync_front_porch  = 16,
        },
        .flags.use_dma2d = true,
    };

    jd9365_vendor_config_t vendor_config = {
        .init_cmds      = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(jd9365_lcd_init_cmd_t),
        .mipi_config = {
            .dsi_bus    = mipi_dsi_bus,
            .dpi_config = &dpi_config,
            .lane_num   = 2,
        },
    };

    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num  = LCD_GPIO_RST,
        .rgb_ele_order   = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel  = 16,
        .vendor_config   = &vendor_config,
    };

    esp_lcd_panel_handle_t disp_panel = NULL;
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_jd9365(io, &lcd_dev_config, &disp_panel), err, TAG, "New LCD panel JD9365 failed");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_reset(disp_panel), err, TAG, "LCD panel reset failed");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_init(disp_panel), err, TAG, "LCD panel init failed");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_disp_on_off(disp_panel, true), err, TAG, "LCD panel ON failed");

    ret_handles->io           = io;
    ret_handles->mipi_dsi_bus = mipi_dsi_bus;
    ret_handles->panel        = disp_panel;
    ret_handles->control      = NULL;

    ESP_LOGI(TAG, "Display LCD initialized successfully");
    return ret;

err:
    if (disp_panel) esp_lcd_panel_del(disp_panel);
    if (io) esp_lcd_panel_io_del(io);
    if (mipi_dsi_bus) esp_lcd_del_dsi_bus(mipi_dsi_bus);
    return ret;
}