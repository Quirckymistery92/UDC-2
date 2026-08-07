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
#include "actions_control.h"

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
#include "vars.h"


/* ══════════════════════════════════════════════════════════════════
 *                              Global Variables
 * ══════════════════════════════════════════════════════════════════ */

#define MAX_SCREEN_SEMAPHORE 3
typedef struct {
    SemaphoreHandle_t semaphore;
    bool in_use;
}sync_control_helper_t;

static sync_control_helper_t sync_control[MAX_SCREEN_SEMAPHORE];

static TaskHandle_t      g_lvgl_task      = NULL;
static QueueHandle_t     g_lvgl_queue     = NULL;
static SemaphoreHandle_t g_lvgl_mutex     = NULL;
static uint32_t          g_lvgl_next_timeout = 0;
static int16_t           currentScreen    = -1;
static const char *TAG = "lvgl_ui";

#define LVGL_TASK_STACK_SIZE        (32 * 1024)
#define LVGL_TASK_PRIORITY          (CONFIG_LVGL_TASK_PRIORITY)
#define LVGL_QUEUE_LENGTH           (32)
#define LVGL_PORT_TASK_MAX_DELAY_MS (500)
#define LVGL_PORT_TASK_MIN_DELAY_MS (1)
#define LVGL_TICK_PERIOD_MS         (CONFIG_LVGL_TICK_PERIOD_MS)

/* ══════════════════════════════════════════════════════════════════
 *                              Helpers
 * ══════════════════════════════════════════════════════════════════ */

static lv_obj_t *get_screen_obj(uint8_t screen_id)
{
    if (screen_id > SCREEN_ID_DS_CONFIG) return NULL;
    return ((lv_obj_t **)&objects)[screen_id];
}
esp_err_t helper_semaphore_control_sync_create(void)
{
    for(size_t i = 0; i < MAX_SCREEN_SEMAPHORE; i++) {
        sync_control[i].semaphore = xSemaphoreCreateBinary();
        if(sync_control[i].semaphore == NULL){
            ESP_LOGI(TAG, "Falló la asignación de semáforos internos de UI de exclusión condicional síncrona de inicialización");
            return ESP_FAIL;
        }
        sync_control[i].in_use = true;
    }
    return ESP_OK;
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
            currentScreen = msg->data.screen_change.screen_id;
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
        
        if(!ui_display_is_created_ds_top()) {
            ESP_LOGI(TAG, "No se puede aplicar contenido de fecha y hora cuando la display ds_top no está inicializada. girl!");
            return;
        }
        LVGL_CHECK_OBJ_PTR(objects.dtop_notch_min_img_bat);
        LVGL_CHECK_OBJ_PTR(objects.dtop_notch_exp_dat_bat_img);
        char message_status_buf[17];
        const char* message_data = "Error";

        if(msg->data.spc_bat_set_data.percentage > 100 || msg->data.spc_bat_set_data.percentage < 0)
        {
                ESP_LOGW(TAG, "Battery percentage out of range: %d", msg->data.spc_bat_set_data.percentage);
                goto exit;
        }

        if(msg->data.spc_bat_set_data.is_charging)
        {
            lv_image_set_src(objects.dtop_notch_min_img_bat, &img_bat_charging30);
            lv_image_set_src(objects.dtop_notch_exp_dat_bat_img, &img_bat_charging30);

            message_data = msg->data.spc_bat_set_data.percentage == 100 ? "%d%% Cargado" : "%d%% Cargando";
        }
        else
        {   
            message_data = "%d%% descargando";
            switch(msg->data.spc_bat_set_data.percentage)
            {
                case 0 ... 10:
                    lv_image_set_src(objects.dtop_notch_min_img_bat, &img_bat_ultra_low30);
                    lv_image_set_src(objects.dtop_notch_exp_dat_bat_img, &img_bat_ultra_low30);
                    break;
                case 11 ... 20:
                    lv_image_set_src(objects.dtop_notch_min_img_bat, &img_bat_low30);
                    lv_image_set_src(objects.dtop_notch_exp_dat_bat_img, &img_bat_low30);
                    break;
                case 21 ... 60:
                    lv_image_set_src(objects.dtop_notch_min_img_bat, &img_bat_med30);
                    lv_image_set_src(objects.dtop_notch_exp_dat_bat_img, &img_bat_med30);
                    break;
                default:
                    lv_image_set_src(objects.dtop_notch_min_img_bat, &img_bat_max30);
                    lv_image_set_src(objects.dtop_notch_exp_dat_bat_img, &img_bat_max30);
                    break;
            }
        }

    exit:
        
        snprintf(message_status_buf, sizeof(message_status_buf), message_data, msg->data.spc_bat_set_data.percentage);
        lv_obj_t   *obj = NULL;
        const char *buf = NULL;
        if (label_abs_prepare(UI_LABEL_ABS_NOTCH_EXT_BAT, message_status_buf, &obj, &buf) == ESP_OK){ lv_label_set_text_static(obj, buf); }
                 
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
        if(!ui_display_is_created_ds_top()) {
            ESP_LOGI(TAG, "No se puede mostrar notificación cuando la display ds_top no está inicializada. girl!");
            return;
        }
        esp_err_t ret = ui_int__anim_notify_user_show(msg->data.spc_notification_show.is_alert, msg->data.spc_notification_show.msg);
        if(ret != ESP_OK) {
            ESP_LOGE(TAG, "Error al mostrar notificación: %s", esp_err_to_name(ret));
        }
    break;

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

/* ══════════════════════════════════════════════════════════════════
 *                     Internal API Implementation
 * ══════════════════════════════════════════════════════════════════ */

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

    esp_err_t err = helper_semaphore_control_sync_create();
    ESP_RETURN_ON_FALSE(err == ESP_OK, ESP_FAIL, TAG, "Falló la inicializacion de semáforos internos de UI de gestión síncrona de inicialización");

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

/* ══════════════════════════════════════════════════════════════════
 *                         Backlight (sync)
 * ══════════════════════════════════════════════════════════════════ */

extern esp_err_t lvgl_porting_backlight_on(void);
extern esp_err_t lvgl_porting_backlight_off(void);
extern esp_err_t lvgl_porting_backlight_set(int brightness_percent);

esp_err_t lvgl_backlight_on(void)  { return lvgl_porting_backlight_on();  }
esp_err_t lvgl_backlight_off(void) { return lvgl_porting_backlight_off(); }
esp_err_t lvgl_backlight_set(int p) { return lvgl_porting_backlight_set(p); }

/* ══════════════════════════════════════════════════════════════════
 *        Gestor de sincronización de inicialización de pantallas
 * ══════════════════════════════════════════════════════════════════ */

void ui_int__helper_get_lock_waiting_screen(uint8_t screen_id) {
    if(sync_control[screen_id].semaphore == NULL || sync_control[screen_id].in_use == false) return;

    xSemaphoreTake(sync_control[screen_id].semaphore, portMAX_DELAY);
    /**
     * Cómo funciona esto:
     * Crea el número total de screens en UI en un enum y en paralelo un control de carga de screens.
     * Semáforo inicializa ya tomado, otras task a usar esta función toman el semaforo y se bloquean esperando.
     * Cuando la Task UI procesa screen_id = SCREEN_ID_DS_TOP la misma libera el mutex y deja que la Task bloqueada continúe su schenduler.
     * Está diseñado para usarse una sola vez en runtime e inicialización, el boolean es para inutilziarlo una vez terminado. Lo noté algo mejor
     * que usar un destructor del semáforo, pero si te sirve, podes destruir y quitar el struct y dejar el array solo de semáforos y al terminar liberar el mutex
     * y eliminar los semáforos apuntando a NULL.
     * 
     */
    
}
void ui_int__helper_clear_lock_waiting_screen(uint8_t screen_id) {
    if(sync_control[screen_id].semaphore == NULL || sync_control[screen_id].in_use == false) return;
    xSemaphoreGive(sync_control[screen_id].semaphore);
    sync_control[screen_id].in_use = false;
    /**
     * El semáforo fué liberado, y el arranque es síncrono y funcional.
     * Acá es un punto decisivo de tu caso si quieres eliminar el
     * semáforo o no, depende del programador actual.
     * 
     */
}
