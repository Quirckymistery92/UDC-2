/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * Display LCD Driver - JD9365 DSI Configuration
 * Resolution: 1280x800, MIPI-DSI 2 lanes
 */

#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "driver/ledc.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ────────────────────────── LCD Resolution ────────────────────────── */
#define LCD_H_RES                   (800)
#define LCD_V_RES                   (1280)

/* ──────────────────────── GPIO Pins ──────────────────────────── */
#define LCD_GPIO_BL                 (GPIO_NUM_23)
#define LCD_GPIO_RST                (GPIO_NUM_27)

/* ──────────────────────── Backlight LEDC ─────────────────────── */
#define LCD_BL_LEDC_CH              (LEDC_CHANNEL_0)

/* ──────────────────────── Draw Buffer ────────────────────────── */
/* 2 frame buffers allocated by the DPI driver in PSRAM.
 * LVGL receives both pointers and uses them in PARTIAL render mode. */
#define LCD_DPI_BUFFER_NUMS         (2)

/* ── Target refresh rate ────────────────────────────────────────── */
/* Adjust this single value to change the display FPS cap.
 * Used both to set the DPI pixel clock and the LVGL refresh timer. */
#define LCD_TARGET_FPS              (40)
#define LCD_FRAME_PERIOD_MS         (1000 / LCD_TARGET_FPS)  /* 25 ms */

/* ──────────────────────── Handles ─────────────────────────────── */
typedef struct {
    esp_lcd_dsi_bus_handle_t    mipi_dsi_bus;
    esp_lcd_panel_io_handle_t   io;
    esp_lcd_panel_handle_t      panel;
    esp_lcd_panel_handle_t      control;
} lcd_handles_t;

/**
 * @brief Initialize display backlight (LEDC PWM)
 */
esp_err_t display_lcd_brightness_init(void);

/**
 * @brief Set display brightness (0-100%)
 */
esp_err_t display_lcd_brightness_set(int brightness_percent);

/**
 * @brief Turn backlight ON (100%)
 */
esp_err_t display_lcd_backlight_on(void);

/**
 * @brief Turn backlight OFF (0%)
 */
esp_err_t display_lcd_backlight_off(void);

/**
 * @brief Create and initialize the MIPI-DSI LCD panel
 *
 * @param[out] ret_handles  All LCD handles
 * @return ESP_OK on success
 */
esp_err_t display_lcd_init(lcd_handles_t *ret_handles);

#ifdef __cplusplus
}
#endif
