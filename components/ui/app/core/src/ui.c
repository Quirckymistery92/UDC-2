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

/* ESP */
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

/* Módulos */
#include "ui_label_abs.h"
#include "interface.h"
#include "../../private/porting/porting.h"
#include "images.h"
#include "ui_zb_controller.h"

/* Variables */
    static TaskHandle_t      g_lvgl_task      = NULL;
    static QueueHandle_t     g_lvgl_queue     = NULL;
    static SemaphoreHandle_t  g_sync_handle     = NULL;
    static uint32_t          g_lvgl_next_timeout = 0;
    static const char * const TAG = "lvgl_ui";
    static app_desc_status_t apd = {
        .app_desc = DSC_APP_UX,
        .app_name = "UX Graphics",
        .app_tag = TAG,
        .init_result = ESP_OK
    };

    #define LVGL_TASK_STACK_SIZE        (32 * 1024)
    #define LVGL_TASK_PRIORITY          (20) //(CONFIG_LVGL_TASK_PRIORITY)
    #define LVGL_QUEUE_LENGTH           (32)
    #define LVGL_PORT_TASK_MAX_DELAY_MS (500)
    #define LVGL_PORT_TASK_MIN_DELAY_MS (1)
    #define LVGL_TICK_PERIOD_MS         (CONFIG_LVGL_TICK_PERIOD_MS)

/* ══════════════════════════════════════════════════════════════════
 *                              Helpers
 * ══════════════════════════════════════════════════════════════════ */

static lv_obj_t *get_screen_obj(uint8_t screen_id)
{
    uint8_t index = __builtin_ctz(screen_id) -1;
    return ((lv_obj_t **)&objects)[index];
}

/* ══════════════════════════════════════════════════════════════════
 *                  Procesadores de comandos para LVGL
 * ══════════════════════════════════════════════════════════════════ */

static void lvgl_process_cmd(const lvgl_msg_t *msg)
{
    if (!msg) return;

    switch (msg->cmd) {

    case LVGL_CMD_SCREEN_CHANGE: {
        lv_obj_t *scr = get_screen_obj(msg->data.screen_change.screen_id);
        LVGL_CHECK_OBJ_PTR(scr);
        if (scr) {
            //currentScreen = msg->data.screen_change.screen_id;
            lv_scr_load_anim(scr,
                             msg->data.screen_change.anim,
                             msg->data.screen_change.time,
                             msg->data.screen_change.delay,
                             false);
            ESP_LOGD(TAG, "Screen changed to id %d", msg->data.screen_change.screen_id);
        }
        break;
    }

    case LVGL_CMD_LABEL_SET_TEXT_STATIC:
        LVGL_CHECK_OBJ_PTR(msg->data.label_text_static.obj);
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
            LVGL_CHECK_OBJ_PTR(obj);
            lv_label_set_text_static(obj, buf);
        }
        break;
    }

    case LVGL_CMD_OBJ_ADD_FLAG:
        LVGL_CHECK_OBJ_PTR(msg->data.obj_flag.obj);
        if (msg->data.obj_flag.obj)
            lv_obj_add_flag(msg->data.obj_flag.obj, msg->data.obj_flag.flags);
        break;

    case LVGL_CMD_OBJ_REMOVE_FLAG:
        LVGL_CHECK_OBJ_PTR(msg->data.obj_flag.obj);
        if (msg->data.obj_flag.obj)
            lv_obj_remove_flag(msg->data.obj_flag.obj, msg->data.obj_flag.flags);
        break;

    case LVGL_CMD_OBJ_ADD_STATE:
        LVGL_CHECK_OBJ_PTR(msg->data.obj_state.obj);
        if (msg->data.obj_state.obj)
            lv_obj_add_state(msg->data.obj_state.obj, msg->data.obj_state.state);
        break;

    case LVGL_CMD_OBJ_REMOVE_STATE:
        LVGL_CHECK_OBJ_PTR(msg->data.obj_state.obj);
        if (msg->data.obj_state.obj)
            lv_obj_remove_state(msg->data.obj_state.obj, msg->data.obj_state.state);
        break;

    case LVGL_CMD_OBJ_SET_USER_DATA:
        LVGL_CHECK_OBJ_PTR(msg->data.obj_user_data.obj);
        if (msg->data.obj_user_data.obj)
            lv_obj_set_user_data(msg->data.obj_user_data.obj,
                                 msg->data.obj_user_data.user_data);
        break;

    case LVGL_CMD_OBJ_SET_STYLE_OPA:
        LVGL_CHECK_OBJ_PTR(msg->data.style_opa.obj);
        if (msg->data.style_opa.obj)
            lv_obj_set_style_opa(msg->data.style_opa.obj,
                                 msg->data.style_opa.opa,
                                 msg->data.style_opa.selector);
        break;

    case LVGL_CMD_OBJ_SET_STYLE_BG_OPA:
        LVGL_CHECK_OBJ_PTR(msg->data.style_opa.obj);
        if (msg->data.style_opa.obj)
            lv_obj_set_style_bg_opa(msg->data.style_opa.obj,
                                    msg->data.style_opa.opa,
                                    msg->data.style_opa.selector);
        break;

    case LVGL_CMD_OBJ_SET_STYLE_BG_COLOR:
        LVGL_CHECK_OBJ_PTR(msg->data.style_color.obj);
        if (msg->data.style_color.obj)
            lv_obj_set_style_bg_color(msg->data.style_color.obj,
                                      msg->data.style_color.color,
                                      msg->data.style_color.selector);
        break;

    case LVGL_CMD_OBJ_SET_STYLE_TEXT_COLOR:
        LVGL_CHECK_OBJ_PTR(msg->data.style_color.obj);
        if (msg->data.style_color.obj)
            lv_obj_set_style_text_color(msg->data.style_color.obj,
                                        msg->data.style_color.color,
                                        msg->data.style_color.selector);
        break;

    case LVGL_CMD_OBJ_SET_STYLE_BG_IMAGE_SRC:
        LVGL_CHECK_OBJ_PTR(msg->data.style_bg_image_src.obj);
        if (msg->data.style_bg_image_src.obj)
            lv_obj_set_style_bg_image_src(msg->data.style_bg_image_src.obj,
                                         msg->data.style_bg_image_src.src,
                                         msg->data.style_bg_image_src.selector);
        break;

    case LVGL_CMD_IMAGE_SET_SRC:
        LVGL_CHECK_OBJ_PTR(msg->data.image_src.obj);
        if (lv_obj_get_class(msg->data.image_src.obj) != &lv_image_class) {
                ESP_LOGE(TAG, "El objeto %p no es un obj del tipo imagen.", msg->data.image_src.obj);
                ESP_LOGE(TAG, "Clase actual: %p | Clase esperada: %p", lv_obj_get_class(msg->data.image_src.obj), &lv_image_class);
                break;
            }
        
        if (msg->data.image_src.obj && msg->data.image_src.src)
            lv_image_set_src(msg->data.image_src.obj, msg->data.image_src.src);
        break;

    case LVGL_CMD_OBJ_SET_POS:
        LVGL_CHECK_OBJ_PTR(msg->data.obj_pos.obj);
        if (msg->data.obj_pos.obj)
            lv_obj_set_pos(msg->data.obj_pos.obj,
                           msg->data.obj_pos.x, msg->data.obj_pos.y);
        break;

    case LVGL_CMD_OBJ_SET_SIZE:
        LVGL_CHECK_OBJ_PTR(msg->data.obj_size.obj);
        if (msg->data.obj_size.obj)
            lv_obj_set_size(msg->data.obj_size.obj,
                            msg->data.obj_size.w, msg->data.obj_size.h);
        break;

    case LVGL_CMD_OBJ_SET_ALIGN:
        LVGL_CHECK_OBJ_PTR(msg->data.obj_align.obj);
        if (msg->data.obj_align.obj)
            lv_obj_set_align(msg->data.obj_align.obj,
                             msg->data.obj_align.align);
        break;

    case LVGL_CMD_OBJ_INVALIDATE:
        LVGL_CHECK_OBJ_PTR(msg->data.obj_simple.obj);
        if (msg->data.obj_simple.obj)
            lv_obj_invalidate(msg->data.obj_simple.obj);
        break;

    case LVGL_CMD_OBJ_SET_HIDDEN:
        LVGL_CHECK_OBJ_PTR(msg->data.obj_simple.obj);
        if (msg->data.obj_simple.obj) {
            if (msg->data.obj_simple.value)
                lv_obj_add_flag(msg->data.obj_simple.obj, LV_OBJ_FLAG_HIDDEN);
            else
                lv_obj_remove_flag(msg->data.obj_simple.obj, LV_OBJ_FLAG_HIDDEN);
        }
        break;

    case LVGL_CMD_SPC_BAT_SET_DATA:{
        uint16_t battery_percent = msg->data.spc_bat_set_data.percentage;
        bool driver_st = msg->data.spc_bat_set_data.driver_ok;
        bool is_charging = msg->data.spc_bat_set_data.is_charging;

        esp_err_t ret = ui_int__dtop_battery_state(battery_percent, is_charging, driver_st);
        if(ret != ESP_OK) ui_int__dtop_notify_user_show(true, "Fallas con el driver de batería.");
    }
    break;

    case LVGL_CMD_TIMER_CONTROL:
        if(msg->data.timer_resume.timer) {
            if (msg->data.timer_resume.extra.apply_pause) {
                lv_timer_pause(msg->data.timer_resume.timer);
            } else {
                lv_timer_resume(msg->data.timer_resume.timer);
            }
        }
    break;

    case LVGL_CMD_TIMER_SET_PERIOD:
        if(msg->data.timer_resume.timer) {
            lv_timer_set_period(msg->data.timer_resume.timer, msg->data.timer_resume.extra.period);
        }
    break;

    case LVGL_CMD_TIMER_RESET:
        if(msg->data.timer_resume.timer) {
            lv_timer_reset(msg->data.timer_resume.timer);
        }
    break;

    case LVGL_CMD_TIMER_SET_REPEAT_COUNT:
        if(msg->data.timer_resume.timer) {
            lv_timer_set_repeat_count(msg->data.timer_resume.timer, msg->data.timer_resume.extra.repeat_count);
        }
    break;

    case LVGL_CMD_TIMER_SET_AUTO_DELETE:
        if(msg->data.timer_resume.timer) {
            lv_timer_set_auto_delete(msg->data.timer_resume.timer, msg->data.timer_resume.extra.auto_delete);
        }
    break;

    case LVGL_CMD_TIMER_SET_USER_DATA:
        if(msg->data.timer_resume.timer) {
            lv_timer_set_user_data(msg->data.timer_resume.timer, msg->data.timer_resume.extra.user_data);
        }
    break;
    
    case LVGL_CMD_SPC_NOTIFICATION_SHOW:
        if(!ui_int__display_is_created_ds_top()) {
            ESP_LOGI(TAG, "No se puede mostrar notificación cuando la display ds_top no está inicializada. girl!");
            return;
        }
        esp_err_t ret = ui_int__dtop_notify_user_show(msg->data.spc_notification_show.is_alert, msg->data.spc_notification_show.msg);
        if(ret != ESP_OK) {
            ESP_LOGE(TAG, "Error al mostrar notificación: %s", esp_err_to_name(ret));
        }
    break;

    case LVGL_CMD_SYS_LOAD_SCREENS: {
        uint8_t mask = msg->data.sys_load_screens.screen_id_mask;

        if(mask & SCREEN_ID_DS_TOP) {
            if(!ui_int__display_is_created_ds_top()) {
                ui_int__display_create_ds_top();
            }
        }
        if(mask & SCREEN_ID_DS_LOADER) {
            if(!ui_int__display_is_created_ds_loader()) {
                ui_int__display_create_ds_loader();
            }
        }
        if(mask & SCREEN_ID_DS_LOCKED) {
            if(!ui_int__display_is_created_ds_locked()) {
                ui_int__display_create_ds_locked();
            }
        }
        if(mask & SCREEN_ID_DS_HOME) {
            if(!ui_int__display_is_created_ds_home()) {
                ui_int__display_create_ds_home();
            }
        }
        if(mask & SCREEN_ID_DS_CONFIG) {
            if(!ui_int__display_is_created_ds_settings()) {
                ui_int__display_create_ds_settings();
            }
        }
        if(mask & SCREEN_ID_DS_BLOCK) {
            if(!ui_int__display_is_created_ds_block()) {
                ui_int__display_create_ds_block();
            }
        }

        uint8_t load = msg->data.sys_load_screens.load;

        if (load != SCREEN_MASK_NONE && load != SCREEN_MASK_ALL &&
            (load & SCREEN_MASK_ALL) == load && (load & (load - 1U)) == 0) {

                lv_obj_t *scr = get_screen_obj(load);
                if(scr) {
                    lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, false);
                } else {
                    ESP_LOGI(TAG, "La display a cargar no está inicializada o apunta a NULL");
                }
        }

        ui_int__helper_clear_lock_waiting_screen();
    }
        break;

    case LVGL_CMD_ZB_REGISTER:{
        uint16_t id_dev = msg->data.zigbee_hall.id;
        ui_zb_callback_result_t cb_dev = msg->data.zigbee_hall.result_cb;

        if(!ui_int__display_is_created_ds_home()) {
            if(cb_dev) cb_dev(id_dev, ESP_ERR_NOT_ALLOWED);
            break;
        }

        const char* name_dev = msg->data.zigbee_hall.payload.register_dev.name;

        ui_zb_type_dev_t type_dev = msg->data.zigbee_hall.payload.register_dev.type;

        uint8_t row_grid = msg->data.zigbee_hall.payload.register_dev.row;
        uint8_t col_grid = msg->data.zigbee_hall.payload.register_dev.col;

        esp_err_t ret = ui_zb_device_register(id_dev, name_dev, type_dev, col_grid, row_grid);

        if(cb_dev) cb_dev(id_dev, ret);
        break;
    }
    case LVGL_CMD_ZB_UNREGISTER:{
        uint16_t id_dev = msg->data.zigbee_hall.id;
        ui_zb_callback_result_t cb_dev = msg->data.zigbee_hall.result_cb;

        esp_err_t ret = ui_zb_device_remove(id_dev);
        
        if(cb_dev) cb_dev(id_dev, ret);
        break;
    }
    case LVGL_CMD_ZB_UPDATE_DATA:{
        uint16_t id_dev = msg->data.zigbee_hall.id;
        ui_zb_callback_result_t cb_dev = msg->data.zigbee_hall.result_cb;
        ui_zb_update_data_t update_data = msg->data.zigbee_hall.payload.update_data;

        esp_err_t ret = ui_zb_device_send_data(id_dev, &update_data);

        if(cb_dev) cb_dev(id_dev, ret);

        break;
    }
    case LVGL_CMD_ZB_UPDATE_LOADING:{
        uint16_t id_dev = msg->data.zigbee_hall.id;
        ui_zb_callback_result_t cb_dev = msg->data.zigbee_hall.result_cb;
        bool is_loading = msg->data.zigbee_hall.payload.is_loading;

        esp_err_t ret = ui_zb_device_set_loading(id_dev, is_loading);

        if(cb_dev) cb_dev(id_dev, ret);

        break;
    }

    case LVGL_CMD_SYS_DRIVER_WIFI:{
        const char *message = msg->data.drivers.state_message;
        bool driver_ok = msg->data.drivers.driver_ok;
        ui_icon_id_t icon = msg->data.drivers.icon;

        esp_err_t ret = ui_int__dtop_wifi_state(message, driver_ok, icon);
        if(ret != ESP_OK){
            ESP_LOGI(TAG, "Falló el driver WiFi aplicando el estado en la UX, error: %s", esp_err_to_name(ret));
        }
        break;
    }
    case LVGL_CMD_SYS_DRIVER_ZIGBEE:{
        const char *message = msg->data.drivers.state_message;
        bool driver_ok = msg->data.drivers.driver_ok;

        esp_err_t ret = ui_int__dtop_zigbee_state(message, driver_ok);
        if(ret != ESP_OK){
            ESP_LOGI(TAG, "Falló el driver Zigbee aplicando el estado en la UX, error: %s", esp_err_to_name(ret));
        }
        break;
    }

    default:
        ESP_LOGW(TAG, "Unknown command: %d", msg->cmd);
        break;
    }
}
/* ══════════════════════════════════════════════════════════════════
 *                            LVGL Ticks
 * ══════════════════════════════════════════════════════════════════ */

static void lvgl_tick_timer_cb(void *arg)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *                          LVGL Task
 * ══════════════════════════════════════════════════════════════════ */

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

    ui_int_sys_actions_init();
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

/* ══════════════════════════════════════════════════════════════════
 *                     Internal API Implementation
 * ══════════════════════════════════════════════════════════════════ */

app_desc_status_t *lvgl_init(void)
{
    ESP_LOGI(TAG, "Iniciando UX UDC-2");

    DSC_RETURN_STRUCT_ON_FALSE(g_lvgl_task == NULL, &apd, ESP_OK);

    g_lvgl_queue = xQueueCreate(LVGL_QUEUE_LENGTH, sizeof(lvgl_msg_t));

    DSC_RETURN_STRUCT_ON_FALSE(g_lvgl_queue != NULL, &apd, ESP_FAIL);

    g_sync_handle = xSemaphoreCreateBinary();
    if (!g_sync_handle) {
        vQueueDelete(g_lvgl_queue);
        return DESC_AP_INSERT(&apd, ESP_FAIL);
    }

    const esp_timer_create_args_t tick_timer_args = {
        .callback = lvgl_tick_timer_cb,
        .name     = "ux_timer",
    };
    esp_timer_handle_t tick_timer = NULL;

    esp_err_t err = esp_timer_create(&tick_timer_args, &tick_timer);
    DSC_RETURN_STRUCT_ON_FALSE(err == ESP_OK, &apd, ESP_FAIL);

    err = esp_timer_start_periodic(tick_timer, LVGL_TICK_PERIOD_MS * 1000);
    DSC_RETURN_STRUCT_ON_FALSE(err == ESP_OK, &apd, ESP_FAIL);
    
    BaseType_t ret = xTaskCreatePinnedToCore(lvgl_task, "UX_UDC",
                                 LVGL_TASK_STACK_SIZE, NULL,
                                 LVGL_TASK_PRIORITY, &g_lvgl_task, 1);
    if (ret != pdPASS) {
        vQueueDelete(g_lvgl_queue);
        vSemaphoreDelete(g_sync_handle);
        return DESC_AP_INSERT(&apd, ESP_FAIL);
    }

    return DESC_AP_INSERT(&apd, ESP_OK);
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

/* ══════════════════════════════════════════════════════════════════
 *        Gestor de sincronización de inicialización de pantallas
 * ══════════════════════════════════════════════════════════════════ */

void ui_int__helper_get_lock_waiting_screen(void) {
    if(g_sync_handle == NULL) return;

    xSemaphoreTake(g_sync_handle, portMAX_DELAY);
}
void ui_int__helper_clear_lock_waiting_screen(void) {
    if(g_sync_handle == NULL) return;

    xSemaphoreGive(g_sync_handle);
}
