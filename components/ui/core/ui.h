/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * LVGL 9 Custom Porting - Internal UI Core Header
 *
 * PRIVATE to the ui component. External code uses ui_manager.h.
 */

#pragma once

#include "esp_err.h"
#include "lvgl.h"
#include "../include/ui_manager.h"   /* ui_label_abs_id_t */
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════
 *  Command Types
 * ══════════════════════════════════════════════════════════════════ */

typedef enum {
    /* Screen management */
    LVGL_CMD_SCREEN_CHANGE = 0,

    /* Label */
    LVGL_CMD_LABEL_SET_TEXT_STATIC,     /**< Puntero estático a buffer PSRAM (ui_label_abs) */
    LVGL_CMD_LABEL_ABS_SET_TEXT,        /**< Texto pre-formateado → copia a PSRAM en task LVGL */

    /* Object flags & state */
    LVGL_CMD_OBJ_ADD_FLAG,
    LVGL_CMD_OBJ_REMOVE_FLAG,
    LVGL_CMD_OBJ_ADD_STATE,
    LVGL_CMD_OBJ_REMOVE_STATE,

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
        ui_label_abs_id_t id;           /**< ID de la entry en la tabla PSRAM               */
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

esp_err_t lvgl_init(void);
esp_err_t lvgl_enqueue_cmd(const lvgl_msg_t *msg, uint32_t timeout_ms);

bool lvgl_porting_lock(uint32_t timeout_ms);
void lvgl_porting_unlock(void);

esp_err_t lvgl_backlight_on(void);
esp_err_t lvgl_backlight_off(void);
esp_err_t lvgl_backlight_set(int brightness_percent);

#ifdef __cplusplus
}
#endif
