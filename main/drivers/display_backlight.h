#pragma once

/* ESP */
#include "esp_err.h"
#include "desc_apps.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize display backlight (LEDC PWM)
 */
app_desc_status_t* display_lcd_brightness_init(void);

/**
 * @brief Set display brightness (0-100)
 */
esp_err_t display_lcd_brightness_set(uint8_t brightness_percent);

/**
 * @brief Turn backlight ON (100%)
 */
esp_err_t display_lcd_backlight_on(void);

/**
 * @brief Turn backlight OFF (0%)
 */
esp_err_t display_lcd_backlight_off(void);

#ifdef __cplusplus
}
#endif