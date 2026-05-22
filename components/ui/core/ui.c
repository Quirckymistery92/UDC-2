/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * LVGL 9 Custom Porting - UI Core Task
 *
 * Architecture:
 * - Single task managing LVGL renderer
 * - Message queue for task-safe commands from any context
 * - All LVGL API calls happen exclusively inside lvgl_task()
 */

#include "ui.h"
#include "ui_label_abs.h"

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

/* Private porting header */
#include "../private/porting/porting.h"

/* Internal UI resources */
#include "../screens/screens.h"
#include "../images/images.h"
#include "../screens/actions.h"
#include "vars.h"

static const char *TAG = "lvgl_ui";

/* ==================================================================
 *  Global State
 * ================================================================== */

static TaskHandle_t      g_lvgl_task      = NULL;
static QueueHandle_t     g_lvgl_queue     = NULL;
static SemaphoreHandle_t g_lvgl_mutex     = NULL;
static uint32_t          g_lvgl_next_timeout = 0;
static int16_t           currentScreen    = -1;

#define LVGL_TASK_STACK_SIZE        (32 * 1024)
#define LVGL_TASK_PRIORITY          (CONFIG_LVGL_TASK_PRIORITY)
#define LVGL_QUEUE_LENGTH           (32)
#define LVGL_PORT_TASK_MAX_DELAY_MS (500)
#define LVGL_PORT_TASK_MIN_DELAY_MS (1)
#define LVGL_TICK_PERIOD_MS         (CONFIG_LVGL_TICK_PERIOD_MS)

/* ==================================================================
 *  Helpers
 * ================================================================== */

static lv_obj_t *get_screen_obj(int16_t screen_id)
{
    int16_t idx = screen_id - 1;
    if (idx < 0) return NULL;
    return ((lv_obj_t **)&objects)[idx];
}

/* ==================================================================
 *  Command Processing (runs ONLY inside lvgl_task)
 * ================================================================== */

static void lvgl_process_cmd(const lvgl_msg_t *msg)
{
    if (!msg) return;

    switch (msg->cmd) {

    case LVGL_CMD_SCREEN_CHANGE: {
        lv_obj_t *scr = get_screen_obj(msg->data.screen_change.screen_id);
        if (scr) {
            currentScreen = msg->data.screen_change.screen_id - 1;
            lv_scr_load_anim(scr,
                             msg->data.screen_change.anim,
                             msg->data.screen_change.time,
                             msg->data.screen_change.delay,
                             msg->data.screen_change.auto_del);
            ESP_LOGD(TAG, "Screen changed to id %d", msg->data.screen_change.screen_id);
        }
        break;
    }

    case LVGL_CMD_LABEL_SET_TEXT_STATIC:
        if (msg->data.label_text_static.obj && msg->data.label_text_static.text) {
            lv_label_set_text_static(msg->data.label_text_static.obj,
                                     msg->data.label_text_static.text);
        }
        break;

    case LVGL_CMD_LABEL_ABS_SET_TEXT: {
        /* Ejecutado en la task LVGL — contexto seguro para modificar objetos.
         * Copia el texto pre-formateado al buffer PSRAM y apunta el label. */
        lv_obj_t   *obj = NULL;
        const char *buf = NULL;
        if (label_abs_prepare(msg->data.label_abs.id,
                              msg->data.label_abs.text,
                              &obj, &buf) == ESP_OK) {
            lv_label_set_text_static(obj, buf);
        }
        break;
    }

    case LVGL_CMD_OBJ_ADD_FLAG:
        if (msg->data.obj_flag.obj)
            lv_obj_add_flag(msg->data.obj_flag.obj, msg->data.obj_flag.flags);
        break;

    case LVGL_CMD_OBJ_REMOVE_FLAG:
        if (msg->data.obj_flag.obj)
            lv_obj_remove_flag(msg->data.obj_flag.obj, msg->data.obj_flag.flags);
        break;

    case LVGL_CMD_OBJ_ADD_STATE:
        if (msg->data.obj_state.obj)
            lv_obj_add_state(msg->data.obj_state.obj, msg->data.obj_state.state);
        break;

    case LVGL_CMD_OBJ_REMOVE_STATE:
        if (msg->data.obj_state.obj)
            lv_obj_remove_state(msg->data.obj_state.obj, msg->data.obj_state.state);
        break;

    case LVGL_CMD_OBJ_SET_STYLE_OPA:
        if (msg->data.style_opa.obj)
            lv_obj_set_style_opa(msg->data.style_opa.obj,
                                 msg->data.style_opa.opa,
                                 msg->data.style_opa.selector);
        break;

    case LVGL_CMD_OBJ_SET_STYLE_BG_OPA:
        if (msg->data.style_opa.obj)
            lv_obj_set_style_bg_opa(msg->data.style_opa.obj,
                                    msg->data.style_opa.opa,
                                    msg->data.style_opa.selector);
        break;

    case LVGL_CMD_OBJ_SET_STYLE_BG_COLOR:
        if (msg->data.style_color.obj)
            lv_obj_set_style_bg_color(msg->data.style_color.obj,
                                      msg->data.style_color.color,
                                      msg->data.style_color.selector);
        break;

    case LVGL_CMD_OBJ_SET_STYLE_TEXT_COLOR:
        if (msg->data.style_color.obj)
            lv_obj_set_style_text_color(msg->data.style_color.obj,
                                        msg->data.style_color.color,
                                        msg->data.style_color.selector);
        break;

    case LVGL_CMD_OBJ_SET_STYLE_BG_IMAGE_SRC:
        if (msg->data.style_bg_image_src.obj)
            lv_obj_set_style_bg_image_src(msg->data.style_bg_image_src.obj,
                                         msg->data.style_bg_image_src.src,
                                         msg->data.style_bg_image_src.selector);
        break;

    case LVGL_CMD_IMAGE_SET_SRC:
        if (msg->data.image_src.obj && msg->data.image_src.src)
            lv_image_set_src(msg->data.image_src.obj, msg->data.image_src.src);
        break;

    case LVGL_CMD_OBJ_SET_POS:
        if (msg->data.obj_pos.obj)
            lv_obj_set_pos(msg->data.obj_pos.obj,
                           msg->data.obj_pos.x, msg->data.obj_pos.y);
        break;

    case LVGL_CMD_OBJ_SET_SIZE:
        if (msg->data.obj_size.obj)
            lv_obj_set_size(msg->data.obj_size.obj,
                            msg->data.obj_size.w, msg->data.obj_size.h);
        break;

    case LVGL_CMD_OBJ_SET_ALIGN:
        if (msg->data.obj_align.obj)
            lv_obj_set_align(msg->data.obj_align.obj,
                             msg->data.obj_align.align);
        break;

    case LVGL_CMD_OBJ_INVALIDATE:
        if (msg->data.obj_simple.obj)
            lv_obj_invalidate(msg->data.obj_simple.obj);
        break;

    case LVGL_CMD_OBJ_SET_HIDDEN:
        if (msg->data.obj_simple.obj) {
            if (msg->data.obj_simple.value)
                lv_obj_add_flag(msg->data.obj_simple.obj, LV_OBJ_FLAG_HIDDEN);
            else
                lv_obj_remove_flag(msg->data.obj_simple.obj, LV_OBJ_FLAG_HIDDEN);
        }
        break;

    default:
        ESP_LOGW(TAG, "Unknown command: %d", msg->cmd);
        break;
    }
}

/* ==================================================================
 *  LVGL Timer Tick
 * ================================================================== */

static void lvgl_tick_timer_cb(void *arg)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

/* ==================================================================
 *  LVGL Task
 * ================================================================== */

static void lvgl_task(void *arg)
{
    ESP_LOGI(TAG, "LVGL task started");

    lvgl_porting_set_task_handle(xTaskGetCurrentTaskHandle());

    lv_init();

    lv_display_t *disp = lvgl_porting_display_init();
    if (!disp) {
        ESP_LOGE(TAG, "Display init failed");
        vTaskDelete(NULL);
        return;
    }

    lv_indev_t *indev = lvgl_porting_touch_init(disp);
    if (!indev) {
        ESP_LOGW(TAG, "Touch init failed - continuing without touch");
    }

    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp,
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED),
        true, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);

    create_screens();
    ESP_LOGI(TAG, "All screens created");

    /*
     * Frame pacing note:
     * ──────────────────
     * display_flush_wait_cb (porting.c) blocks on the VSYNC semaphore
     * for ~33 ms every frame during active rendering, so lv_timer_handler
     * naturally returns after ~33 ms and LVGL runs at ≤ 30 fps.
     *
     * In idle periods (no dirty regions) lv_timer_handler returns quickly
     * with the time until the next LVGL animation timer fires.  We cap
     * that sleep at LVGL_FRAME_PERIOD_MS so command-queue latency stays
     * bounded and the task always yields at least once per frame period.
     *
     * xQueueReceive is the yield point: PPA and other tasks execute
     * while LVGL is waiting for the next command or timer event.
     */
#define LVGL_FRAME_PERIOD_MS  33   /* ceil(1000 / 30 fps) */

    g_lvgl_next_timeout = LVGL_FRAME_PERIOD_MS;

    while (1) {
        lvgl_msg_t msg;

        /* Yield here: blocks up to g_lvgl_next_timeout ms.
         * PPA, sensor tasks, and idle run during this window. */
        if (xQueueReceive(g_lvgl_queue, &msg,
                          pdMS_TO_TICKS(g_lvgl_next_timeout)) == pdTRUE) {
            lvgl_process_cmd(&msg);
            /* Drain any additional queued commands without extra blocking. */
            while (xQueueReceive(g_lvgl_queue, &msg, 0) == pdTRUE) {
                lvgl_process_cmd(&msg);
            }
        }

        /* Run LVGL timers.  During active rendering this call blocks
         * inside display_flush_wait_cb for ~33 ms (VSYNC); otherwise
         * it returns quickly with the time until the next timer. */
        uint32_t timer_ret = lv_timer_handler();

        /* Clamp idle sleep: always yield at least 1 tick,
         * never sleep longer than one frame period. */
        if (timer_ret < LVGL_PORT_TASK_MIN_DELAY_MS)
            timer_ret = LVGL_PORT_TASK_MIN_DELAY_MS;
        if (timer_ret > LVGL_FRAME_PERIOD_MS)
            timer_ret = LVGL_FRAME_PERIOD_MS;

        g_lvgl_next_timeout = timer_ret;
    }
}

/* ==================================================================
 *  Internal API Implementation
 * ================================================================== */

esp_err_t lvgl_init(void)
{
    ESP_LOGI(TAG, "Initializing LVGL core system...");

    if (g_lvgl_task != NULL) {
        ESP_LOGW(TAG, "LVGL already initialized");
        return ESP_OK;
    }

    g_lvgl_mutex = xSemaphoreCreateMutex();
    if (!g_lvgl_mutex) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_FAIL;
    }

    g_lvgl_queue = xQueueCreate(LVGL_QUEUE_LENGTH, sizeof(lvgl_msg_t));
    if (!g_lvgl_queue) {
        ESP_LOGE(TAG, "Failed to create queue");
        vSemaphoreDelete(g_lvgl_mutex);
        return ESP_FAIL;
    }

    const esp_timer_create_args_t tick_timer_args = {
        .callback = lvgl_tick_timer_cb,
        .name     = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer = NULL;
    ESP_RETURN_ON_ERROR(esp_timer_create(&tick_timer_args, &tick_timer),
                        TAG, "Tick timer create failed");
    ESP_RETURN_ON_ERROR(esp_timer_start_periodic(tick_timer, LVGL_TICK_PERIOD_MS * 1000),
                        TAG, "Tick timer start failed");

    BaseType_t ret = xTaskCreate(lvgl_task, "lvgl_task",
                                 LVGL_TASK_STACK_SIZE, NULL,
                                 LVGL_TASK_PRIORITY, &g_lvgl_task);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LVGL task");
        vQueueDelete(g_lvgl_queue);
        vSemaphoreDelete(g_lvgl_mutex);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "LVGL core system initialized");
    return ESP_OK;
}

esp_err_t lvgl_enqueue_cmd(const lvgl_msg_t *msg, uint32_t timeout_ms)
{
    if (!msg || !g_lvgl_queue) return ESP_FAIL;

    TickType_t ticks = (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);
    if (xQueueSend(g_lvgl_queue, msg, ticks) == pdTRUE)
        return ESP_OK;

    ESP_LOGW(TAG, "Failed to enqueue command %d", msg->cmd);
    return ESP_FAIL;
}

bool lvgl_porting_lock(uint32_t timeout_ms)
{
    if (!g_lvgl_mutex) return false;
    TickType_t ticks = (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(g_lvgl_mutex, ticks) == pdTRUE;
}

void lvgl_porting_unlock(void)
{
    if (g_lvgl_mutex) xSemaphoreGive(g_lvgl_mutex);
}

/* ==================================================================
 *  Backlight (direct calls, no queue needed)
 * ================================================================== */

extern esp_err_t lvgl_porting_backlight_on(void);
extern esp_err_t lvgl_porting_backlight_off(void);
extern esp_err_t lvgl_porting_backlight_set(int brightness_percent);

esp_err_t lvgl_backlight_on(void)  { return lvgl_porting_backlight_on();  }
esp_err_t lvgl_backlight_off(void) { return lvgl_porting_backlight_off(); }
esp_err_t lvgl_backlight_set(int p) { return lvgl_porting_backlight_set(p); }
