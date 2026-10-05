/**
 * @file ui.h
 * @brief Internal LVGL command, queue, and porting declarations.
 *
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * LVGL 9 Custom Porting - Internal UI Core Header
 *
 * PRIVATE to the ui component. External code uses ui_manager.h.
 */

#pragma once

/* ESP */
#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

/* Módulos */
#include <lvgl.h>
#include "ui_zb_types.h"
#include "ui_label_abs.h"
#include "ui_image_sia.h"
#include "desc_apps.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LVGL_CHECK_OBJ 0

#if LVGL_CHECK_OBJ
#define LVGL_CHECK_OBJ_PTR(obj) do { \
    if (obj == NULL || !lv_obj_is_valid(obj)) { \
        ESP_LOGE("LVGL", "LVGL object %p pointer is NULL!", obj); \
        return; \
    } \
} while (0)
#else
#define LVGL_CHECK_OBJ_PTR(obj) do { } while (0)
#endif

/* ══════════════════════════════════════════════════════════════════
 *  Command Types
 * ══════════════════════════════════════════════════════════════════ */

typedef enum {
    /* Screen management */
    LVGL_CMD_SCREEN_CHANGE = 0,

    /* System */
    LVGL_CMD_SYS_LOAD_SCREENS,

    /* Label */
    LVGL_CMD_LABEL_SET_TEXT_STATIC,     /**< Puntero estático a buffer PSRAM (ui_label_abs) */
    LVGL_CMD_LABEL_ABS_SET_TEXT,        /**< Texto pre-formateado → copia a PSRAM en task LVGL */

    /* Object flags & state */
    LVGL_CMD_OBJ_ADD_FLAG,
    LVGL_CMD_OBJ_REMOVE_FLAG,
    LVGL_CMD_OBJ_ADD_STATE,
    LVGL_CMD_OBJ_REMOVE_STATE,
    LVGL_CMD_OBJ_SET_USER_DATA,

    /* Style properties */
    LVGL_CMD_OBJ_SET_STYLE_OPA,
    LVGL_CMD_OBJ_SET_STYLE_BG_COLOR,
    LVGL_CMD_OBJ_SET_STYLE_BG_OPA,
    LVGL_CMD_OBJ_SET_STYLE_TEXT_COLOR,
    LVGL_CMD_OBJ_SET_STYLE_BG_IMAGE_SRC,

    /* Image */
    LVGL_CMD_IMAGE_SET_SRC,

    /* Geometry */
    LVGL_CMD_OBJ_SET_POS,
    LVGL_CMD_OBJ_SET_SIZE,
    LVGL_CMD_OBJ_SET_ALIGN,

    /* Misc */
    LVGL_CMD_OBJ_INVALIDATE,
    LVGL_CMD_OBJ_SET_HIDDEN,

    /* Specific */
    LVGL_CMD_SPC_BAT_SET_DATA,   /**< Comando específico para actualizar datos de batería en pantalla */
    LVGL_CMD_SPC_CONTROL_PANEL_SHOW, /**< Comando específico para mostrar u ocultar el panel de control */
    LVGL_CMD_SPC_NOTIFICATION_SHOW, /**< Comando específico para mostrar notificaciones */

    /* Timer control */
    LVGL_CMD_TIMER_CONTROL,
    LVGL_CMD_TIMER_SET_PERIOD,
    LVGL_CMD_TIMER_RESET,
    LVGL_CMD_TIMER_SET_REPEAT_COUNT,
    LVGL_CMD_TIMER_SET_AUTO_DELETE,
    LVGL_CMD_TIMER_SET_USER_DATA,

    /* Zigbee Hall */
    LVGL_CMD_ZB_REGISTER, /* Registra el dispositivo Zigbee */
    LVGL_CMD_ZB_UNREGISTER, /* Eliminar registros de un dispositivo Zigbee */
    LVGL_CMD_ZB_UPDATE_LOADING, /* Settea un visual loading de un dispositivo Zigbee */
    LVGL_CMD_ZB_UPDATE_DATA, /* Actualizar datos de un dispositivo Zigbee */

    LVGL_CMD_SYS_DRIVER_WIFI, /* Controlador visual y estético de WiFi */
    LVGL_CMD_SYS_DRIVER_ZIGBEE, /* Controlador visual y estético de Zigbee */

    LVGL_CMD_MAX
} lvgl_cmd_t;

/* ══════════════════════════════════════════════════════════════════
 *  Command Data Union
 * ══════════════════════════════════════════════════════════════════ */

typedef union {
    struct {
        int16_t             screen_id;
        lv_scr_load_anim_t  anim;
        uint32_t            time;
        uint32_t            delay;
        bool                auto_del;
    } screen_change;

    struct {
        lv_obj_t   *obj;
        const char *text;               /**< Puntero a buffer PSRAM estático (ui_label_abs) */
    } label_text_static;

    struct {
        uint8_t id;           /**< ID de la entry en la tabla PSRAM               */
        char              text[256];    /**< Texto pre-formateado en el caller, copiado aquí */
    } label_abs;

    struct {
        lv_obj_t      *obj;
        lv_obj_flag_t  flags;
    } obj_flag;

    struct {
        lv_obj_t   *obj;
        lv_state_t  state;
    } obj_state;

    struct {
        lv_obj_t           *obj;
        lv_opa_t            opa;
        lv_style_selector_t selector;
    } style_opa;

    struct {
        lv_obj_t           *obj;
        lv_color_t          color;
        lv_style_selector_t selector;
    } style_color;

    struct {
        lv_obj_t   *obj;
        const void *src;
        lv_style_selector_t selector;
    } style_bg_image_src;

    struct {
        lv_obj_t   *obj;
        const void *src;
    } image_src;

    struct {
        lv_obj_t *obj;
        int32_t   x;
        int32_t   y;
    } obj_pos;

    struct {
        lv_obj_t *obj;
        int32_t   w;
        int32_t   h;
    } obj_size;

    struct {
        lv_obj_t   *obj;
        lv_align_t  align;
    } obj_align;

    struct {
        lv_obj_t *obj;
        bool      value;
    } obj_simple;

    struct {
        lv_obj_t *obj;
        void     *user_data;
    } obj_user_data;
    
    struct {
        uint16_t percentage;
        bool driver_ok;
        bool is_charging;
    }spc_bat_set_data;

    struct {
        bool show;
    } spc_control_panel_show;

    struct {
        lv_timer_t *timer;
        union {
            bool apply_pause;
            uint32_t period;
            int32_t repeat_count;
            bool auto_delete;
            void * user_data;
        }extra;
    } timer_resume;

    struct {
        char msg[256];
        bool is_alert;
    }spc_notification_show;

    struct {
        uint8_t screen_id_mask;
        uint8_t load;
    } sys_load_screens;

    struct {
        uint16_t id;
        ui_zb_callback_result_t result_cb;
        union {
            struct {
                char name[MAX_DEV_NAME_LEN];
                ui_zb_type_dev_t type;
                uint8_t col;
                uint8_t row;
            }register_dev;

            ui_zb_update_data_t update_data;
            bool is_loading;
        }payload;

    } zigbee_hall;

    struct {
        char state_message[BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE];
        bool driver_ok;
        ui_icon_id_t icon;
    }drivers;

} lvgl_cmd_data_t;

/* ══════════════════════════════════════════════════════════════════
 *  Command Message
 * ══════════════════════════════════════════════════════════════════ */

typedef struct {
    lvgl_cmd_t      cmd;
    lvgl_cmd_data_t data;
} lvgl_msg_t;

/* ══════════════════════════════════════════════════════════════════
 *  Internal API (used by ui_manager.c within the component)
 * ══════════════════════════════════════════════════════════════════ */

app_desc_status_t *lvgl_init(void);
esp_err_t lvgl_enqueue_cmd(const lvgl_msg_t *msg, uint32_t timeout_ms);

bool lvgl_porting_lock(uint32_t timeout_ms);
void lvgl_porting_unlock(void);

void ui_int__helper_get_lock_waiting_screen(void);// go to API
void ui_int__helper_clear_lock_waiting_screen(void); // go to INTERNAL

#ifdef __cplusplus
}
#endif