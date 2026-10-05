/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * LVGL 9 Porting – ESP32-P4 DSI + PPA Asynchronous Bounding-Box Synchronization
 */

#include "porting.h"

/* ESP */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_cache.h"
#include "driver/ppa.h"
#include "sdkconfig.h"

/* Módulos */
#include "display_lcd.h"
#include "display_touch.h"

/* ══════════════════════════════════════════════════════════════════
 *                        Global Variables
 * ══════════════════════════════════════════════════════════════════ */
static const char * const TAG = "lvgl_porting";

#define LVGL_H_RES      LCD_H_RES
#define LVGL_V_RES      LCD_V_RES
#define BPP             2
#define LV_CF           LV_COLOR_FORMAT_RGB565
#define FRAME_BUF_SIZE  ((size_t)LVGL_H_RES * (size_t)LVGL_V_RES * BPP)

#define LVGL_ALGINED_SIZE 128

static lcd_handles_t s_lcd_handles = {0};
static esp_lcd_touch_handle_t s_touch_handle = NULL;
static void *s_fb0 = NULL;
static void *s_fb1 = NULL;

/* Arquitectura PPA Asíncrona (Aprobada para DSI + P4) */
static ppa_client_handle_t s_ppa_client = NULL;
static SemaphoreHandle_t s_vsync_sem = NULL;
static TaskHandle_t s_sync_task = NULL;

/* Variables de estado para la sincronización entre frames */
static lv_area_t s_dirty_bbox;
static bool s_first_flush = true;
static lv_display_t *s_disp_to_flush = NULL;
static void *s_drawn_fb = NULL;

/* ══════════════════════════════════════════════════════════════════
 *                              Helpers
 * ══════════════════════════════════════════════════════════════════ */
static inline void sync_cache_aligned(void *buf, uint32_t offset, uint32_t size, int flags) {
    uint32_t start_addr = (uint32_t)buf + offset;
    
    uint32_t aligned_start = start_addr & ~(LVGL_ALGINED_SIZE - 1);
    uint32_t aligned_size = (size + (start_addr - aligned_start) + (LVGL_ALGINED_SIZE - 1)) & ~(LVGL_ALGINED_SIZE - 1);

    esp_cache_msync((void *)aligned_start, aligned_size, flags);
}

/* ══════════════════════════════════════════════════════════════════
 *               Callback de interrupción DSI en VSYNC
 * ══════════════════════════════════════════════════════════════════ */

static bool on_refresh_done_cb(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx)
{
    BaseType_t higher_prio_woken = pdFALSE;
    if (s_vsync_sem) {
        xSemaphoreGiveFromISR(s_vsync_sem, &higher_prio_woken);
    }
    return higher_prio_woken == pdTRUE;
}

/* ══════════════════════════════════════════════════════════════════
 * PPA Asynchronous Bounding-Box Synchronization Task (Alta prioridad)
 * ══════════════════════════════════════════════════════════════════ */
static void ppa_sync_task_func(void *arg)
{
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        esp_lcd_panel_draw_bitmap(s_lcd_handles.panel, 0, 0, LCD_H_RES, LCD_V_RES, s_drawn_fb);

        xSemaphoreTake(s_vsync_sem, 0); //Limpieza
        xSemaphoreTake(s_vsync_sem, portMAX_DELAY);

        void *offline_fb = (s_drawn_fb == s_fb0) ? s_fb1 : s_fb0;
        uint32_t w = s_dirty_bbox.x2 - s_dirty_bbox.x1 + 1;
        uint32_t h = s_dirty_bbox.y2 - s_dirty_bbox.y1 + 1;
        uint32_t offset = (s_dirty_bbox.y1 * LVGL_H_RES + s_dirty_bbox.x1) * BPP;
        uint32_t size = (h * LVGL_H_RES) * BPP; 

        sync_cache_aligned(s_drawn_fb, offset, size, ESP_CACHE_MSYNC_FLAG_DIR_C2M);

        // Configuración SRM estricta del PPA
        ppa_srm_oper_config_t srm_cfg;
        memset(&srm_cfg, 0, sizeof(srm_cfg));

        srm_cfg.in.buffer = s_drawn_fb;
        srm_cfg.in.pic_w = LVGL_H_RES;
        srm_cfg.in.pic_h = LVGL_V_RES;
        srm_cfg.in.block_offset_x = s_dirty_bbox.x1;
        srm_cfg.in.block_offset_y = s_dirty_bbox.y1;
        srm_cfg.in.block_w = w;
        srm_cfg.in.block_h = h;
        srm_cfg.in.srm_cm = PPA_SRM_COLOR_MODE_RGB565;

        srm_cfg.out.buffer = offline_fb;
        srm_cfg.out.buffer_size = FRAME_BUF_SIZE;
        srm_cfg.out.pic_w = LVGL_H_RES;
        srm_cfg.out.pic_h = LVGL_V_RES;
        srm_cfg.out.block_offset_x = s_dirty_bbox.x1;
        srm_cfg.out.block_offset_y = s_dirty_bbox.y1;
        srm_cfg.out.srm_cm = PPA_SRM_COLOR_MODE_RGB565;

        srm_cfg.rotation_angle = PPA_SRM_ROTATION_ANGLE_0;
        srm_cfg.scale_x = 1.0f;
        srm_cfg.scale_y = 1.0f;
        srm_cfg.mode = PPA_TRANS_MODE_BLOCKING;

        ppa_do_scale_rotate_mirror(s_ppa_client, &srm_cfg);
        sync_cache_aligned(offline_fb, offset, size, ESP_CACHE_MSYNC_FLAG_DIR_M2C);

        s_first_flush = true;
        lv_display_flush_ready(s_disp_to_flush);   
    }
}

/* ══════════════════════════════════════════════════════════════════
 *               Callback LVGL para el dibujado de frames
 * ══════════════════════════════════════════════════════════════════ */
static void display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    if (s_first_flush) {
        s_dirty_bbox = *area;
        s_first_flush = false;
    } else {
        if (area->x1 < s_dirty_bbox.x1) s_dirty_bbox.x1 = area->x1;
        if (area->y1 < s_dirty_bbox.y1) s_dirty_bbox.y1 = area->y1;
        if (area->x2 > s_dirty_bbox.x2) s_dirty_bbox.x2 = area->x2;
        if (area->y2 > s_dirty_bbox.y2) s_dirty_bbox.y2 = area->y2;
    }

    if (lv_display_flush_is_last(disp)) {
        s_disp_to_flush = disp;
        s_drawn_fb = px_map;
        // Despertar a la tarea PPA para que administre el VSYNC y la copia
        xTaskNotifyGive(s_sync_task);
    } else {
        lv_display_flush_ready(disp);
    }
}

/* ══════════════════════════════════════════════════════════════════
 *               Callback LVGL para la lectura del touch
 * ══════════════════════════════════════════════════════════════════ */
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

void lvgl_porting_set_task_handle(TaskHandle_t task)
{
    (void)task; 
}
/* ══════════════════════════════════════════════════════════════════
 *              Inicialiación del display y touch para LVGL
 * ══════════════════════════════════════════════════════════════════ */
lv_display_t *lvgl_porting_display_init(void)
{
    ESP_ERROR_CHECK(display_lcd_init(&s_lcd_handles));

    void *fb0 = NULL, *fb1 = NULL;
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_get_frame_buffer(
                        s_lcd_handles.panel, 2, &fb0, &fb1));
    s_fb0 = fb0;
    s_fb1 = fb1;
    s_vsync_sem = xSemaphoreCreateBinary();

    // Iniciar tarea background de sincronización (Alta prioridad)
    xTaskCreatePinnedToCore(ppa_sync_task_func, "ppa_sync", 4096, NULL, configMAX_PRIORITIES - 1, &s_sync_task, tskNO_AFFINITY);

    // Inicializar PPA
    ppa_client_config_t ppa_cfg;
    memset(&ppa_cfg, 0, sizeof(ppa_cfg));
    ppa_cfg.oper_type = PPA_OPERATION_SRM;
    
    ESP_ERROR_CHECK(ppa_register_client(&ppa_cfg, &s_ppa_client));
    ESP_LOGI(TAG, "PPA hardware acceleration engine registered");

    lv_display_t *disp = lv_display_create(LVGL_H_RES, LVGL_V_RES);
    if (!disp) { ESP_LOGE(TAG, "lv_display_create failed"); return NULL; }

    lv_display_set_color_format(disp, LV_CF);
    lv_display_set_buffers(disp, fb0, fb1, FRAME_BUF_SIZE, LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_flush_cb(disp, display_flush_cb);

    esp_lcd_dpi_panel_event_callbacks_t cbs = {
        .on_refresh_done = on_refresh_done_cb,
    };
    ESP_ERROR_CHECK(esp_lcd_dpi_panel_register_event_callbacks(s_lcd_handles.panel, &cbs, disp));

    lv_timer_t *refr_timer = lv_display_get_refr_timer(disp);
    if (refr_timer) {
        lv_timer_set_period(refr_timer, LCD_FRAME_PERIOD_MS);
    }

    ESP_LOGI(TAG, "Display ready: %dx%d RGB565 DIRECT+sync,, %d fps limit",
             LVGL_H_RES, LVGL_V_RES, LCD_TARGET_FPS);
    return disp;
}

lv_indev_t *lvgl_porting_touch_init(lv_display_t *disp)
{
    if (display_touch_init(&s_touch_handle) != ESP_OK) {
        ESP_LOGE(TAG, "Touch controller init failed");
        return NULL;
    }
    lv_indev_t *indev = lv_indev_create();

    if (!indev) {
      ESP_LOGE(TAG, "lv_indev_create failed");
      return NULL;
    }
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_display(indev, disp);
    lv_indev_set_read_cb(indev, touch_read_cb);

    ESP_LOGI(TAG, "Touch initialised");
    return indev;
}