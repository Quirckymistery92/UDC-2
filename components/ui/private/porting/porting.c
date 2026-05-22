/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * LVGL 9 Porting – Display DSI + Touch GSL3680
 *
 * Architecture: DIRECT mode, driver fbs (PSRAM), 30 fps cap via #define.
 *
 * Buffers:
 *   The JD9365 DPI driver allocates LCD_DPI_BUFFER_NUMS (= 2) full-frame
 *   PSRAM buffers. We obtain both pointers via
 *   esp_lcd_dpi_panel_get_frame_buffer() and pass them directly to LVGL.
 *   No manual PSRAM allocation.
 *
 * Render mode – DIRECT:
 *   LVGL renders only dirty regions into the active (back) frame buffer.
 *   On each flush call the same dirty area is also memcpy'd to the OTHER
 *   frame buffer, keeping both buffers in sync. Without this, the inactive
 *   buffer accumulates stale/black regions that become visible on the next
 *   buffer flip → tearing in dark/black sections.
 *
 * Anti-tearing:
 *   Buffer flip (draw_bitmap) + VSYNC wait happen only on the LAST flush of
 *   each frame (lv_disp_flush_is_last). Intermediate partial flushes just
 *   copy to the other buffer and call flush_ready immediately.
 *   TaskNotify (ulTaskNotifyTake/vTaskNotifyGiveFromISR) is used instead of
 *   a binary semaphore – matches the Espressif BSP reference and avoids
 *   stale-token edge-cases without the drain trick.
 *
 * FPS cap via #define:
 *   LCD_TARGET_FPS in display_lcd.h sets both the DPI pixel clock and
 *   the LVGL refresh timer period. Change that single #define to adjust.
 */

#include "porting.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "sdkconfig.h"
#include "lvgl.h"

#include "display_lcd.h"
#include "display_touch.h"

static const char *TAG = "lvgl_porting";

#define LVGL_H_RES      LCD_H_RES
#define LVGL_V_RES      LCD_V_RES
#define BPP             2
#define LV_CF           LV_COLOR_FORMAT_RGB565
#define FRAME_BUF_SIZE  ((size_t)LVGL_H_RES * (size_t)LVGL_V_RES * BPP)

static lcd_handles_t           s_lcd_handles  = {0};
static esp_lcd_touch_handle_t  s_touch_handle = NULL;
static TaskHandle_t            s_lvgl_task    = NULL;  /* set by lvgl_porting_set_task_handle */
static void                   *s_fb0          = NULL;  /* both frame buffer pointers for dirty-area sync */
static void                   *s_fb1          = NULL;

static bool on_refresh_done_cb(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx)
{
    (void)panel;
    (void)edata;
    (void)user_ctx;
    BaseType_t higher_prio_woken = pdFALSE;
    if (s_lvgl_task) {
        vTaskNotifyGiveFromISR(s_lvgl_task, &higher_prio_woken);
    }
    return higher_prio_woken == pdTRUE;
}

static void display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    /* Mirror this dirty area to the OTHER frame buffer so both stay in sync.
     * Without this, the inactive buffer accumulates stale (black) content
     * that becomes visible the next time it's used → tearing on dark sections. */
    void *other_fb = (px_map == s_fb0) ? s_fb1 : s_fb0;
    if (other_fb) {
        int32_t w = area->x2 - area->x1 + 1;
        for (int32_t row = area->y1; row <= area->y2; row++) {
            const uint8_t *src = (const uint8_t *)px_map   + (row * LCD_H_RES + area->x1) * BPP;
            uint8_t       *dst = (uint8_t *)other_fb        + (row * LCD_H_RES + area->x1) * BPP;
            memcpy(dst, src, (size_t)w * BPP);
        }
    }

    /* Only flip the DPI output pointer on the last dirty area of this frame.
     * Flipping on every partial call would show mid-frame content. */
    if (!lv_disp_flush_is_last(disp)) {
        lv_disp_flush_ready(disp);
        return;
    }

    /* Flip: tell the DPI controller to start outputting px_map next VSYNC. */
    esp_lcd_panel_draw_bitmap(s_lcd_handles.panel,
                              0, 0, LCD_H_RES, LCD_V_RES,
                              px_map);

    /* Block until the ISR confirms the flip is complete (VSYNC fired).
     * ulTaskNotifyTake + vTaskNotifyGiveFromISR: no stale-token risk. */
    ulTaskNotifyValueClear(NULL, ULONG_MAX);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    lv_disp_flush_ready(disp);
}

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (!s_touch_handle) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    esp_lcd_touch_read_data(s_touch_handle);
    esp_lcd_touch_point_data_t point = {0};
    uint8_t touch_cnt = 0;
    esp_err_t err = esp_lcd_touch_get_data(s_touch_handle, &point, &touch_cnt, 1);
    if (err == ESP_OK && touch_cnt > 0) {
        data->point.x = point.x;
        data->point.y = point.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

/* Reserved: declared in porting.h and called by ui.c. Stores the LVGL task
 * handle so the ISR (on_refresh_done_cb) can unblock it via TaskNotify. */
void lvgl_porting_set_task_handle(TaskHandle_t task)
{
    s_lvgl_task = task;
}

lv_display_t *lvgl_porting_display_init(void)
{
    ESP_ERROR_CHECK(display_lcd_init(&s_lcd_handles));
    ESP_LOGI(TAG, "Display hardware initialised (JD9365 MIPI-DSI, num_fbs=%d, %d fps)",
             LCD_DPI_BUFFER_NUMS, LCD_TARGET_FPS);

    void *fb0 = NULL, *fb1 = NULL;
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_get_frame_buffer(
                        s_lcd_handles.panel, 2, &fb0, &fb1));
    s_fb0 = fb0;
    s_fb1 = fb1;
    ESP_LOGI(TAG, "Driver fbs: [0]=%p [1]=%p (%u B each)",
             fb0, fb1, (unsigned)FRAME_BUF_SIZE);

    lv_display_t *disp = lv_display_create(LVGL_H_RES, LVGL_V_RES);
    if (!disp) { ESP_LOGE(TAG, "lv_display_create failed"); return NULL; }

    /* NOTE: semaphore removed – VSYNC sync now uses ulTaskNotifyTake /
     *       vTaskNotifyGiveFromISR (requires s_lvgl_task to be set first). */

    lv_display_set_color_format(disp, LV_CF);
    lv_display_set_buffers(disp, fb0, fb1, FRAME_BUF_SIZE,
                           LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_flush_cb(disp, display_flush_cb);

    esp_lcd_dpi_panel_event_callbacks_t cbs = {
        .on_refresh_done = on_refresh_done_cb,
    };
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_register_event_callbacks(s_lcd_handles.panel, &cbs, disp));

    lv_timer_t *refr_timer = lv_display_get_refr_timer(disp);
    if (refr_timer) {
        lv_timer_set_period(refr_timer, LCD_FRAME_PERIOD_MS);
        ESP_LOGI(TAG, "Refresh timer: %d ms (%d fps max)",
                 LCD_FRAME_PERIOD_MS, LCD_TARGET_FPS);
    }

    ESP_LOGI(TAG, "Display ready: %dx%d RGB565 DIRECT+sync, driver PSRAM fbs, %d fps cap",
             LVGL_H_RES, LVGL_V_RES, LCD_TARGET_FPS);
    return disp;
}

lv_indev_t *lvgl_porting_touch_init(lv_display_t *disp)
{
    ESP_LOGI(TAG, "Initializing touch (GSL3680)...");
    if (display_touch_init(&s_touch_handle) != ESP_OK) {
        ESP_LOGE(TAG, "Touch controller init failed"); return NULL;
    }
    lv_indev_t *indev = lv_indev_create();
    if (!indev) { ESP_LOGE(TAG, "lv_indev_create failed"); return NULL; }
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_display(indev, disp);
    lv_indev_set_read_cb(indev, touch_read_cb);
    ESP_LOGI(TAG, "Touch initialised");
    return indev;
}

esp_err_t lvgl_porting_backlight_on(void)            { return display_lcd_backlight_on(); }
esp_err_t lvgl_porting_backlight_off(void)           { return display_lcd_backlight_off(); }
esp_err_t lvgl_porting_backlight_set(int pct)        { return display_lcd_brightness_set(pct); }