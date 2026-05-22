/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * UI Manager - Public API Implementation
 *
 * Every ui_*() function builds a lvgl_msg_t and enqueues it
 * to the LVGL task for safe execution.
 */

#include "ui_manager.h"
#include "ui.h"
#include "ui_label_abs.h"
#include "../screens/actions.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui_image_sia_internal.h"

static const char *TAG = "ui_mgr";
/**
 * @brief Retorna el valor pedido del enum a path const char* para usar con JPEGc (e.g. "bg_thunder_rail.jpg")
 * 
 * @param img Imagen de fondo a convertir
 * @return const char* Ruta de la imagen correspondiente
 */
const char* ui_backg_img_to_path(ui_backg_img_t img)
{
    switch (img) {
        case UI_IMG_THUNDER_RAIL: return "bg_thunder_rail.jpg";
        default: return NULL;
    }
}
/* ══════════════════════════════════════════════════════════════════
 *  Initialization
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_init(void)
{
    ESP_LOGI(TAG, "UI subsystem initializing...");

    esp_err_t ret = lvgl_init();
    if (ret != ESP_OK) return ret;

    /* Give the task time to create display + screens */
    vTaskDelay(pdMS_TO_TICKS(200));

    /* Turn on backlight */
    ret = lvgl_backlight_on();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Backlight failed: %s", esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "UI subsystem ready");
    return ESP_OK;
}

/* ══════════════════════════════════════════════════════════════════
 *  Screen
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_screen_change(enum ScreensEnum screen_id,
                           lv_scr_load_anim_t anim,
                           uint32_t time,
                           uint32_t delay,
                           bool auto_del)
{
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_SCREEN_CHANGE,
        .data.screen_change = {
            .screen_id = (int16_t)screen_id,
            .anim      = anim,
            .time      = time,
            .delay     = delay,
            .auto_del  = auto_del,
        },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

// Método Asíncrono para actualizar fondo de pantalla de Home
esp_err_t ui_async_screen_backg_update(ui_backg_img_t img)
{
    const char *image_path = ui_backg_img_to_path(img);
    if (!image_path) return ESP_ERR_INVALID_ARG;
    return actions_sys_backg_update(image_path);
}

/* ══════════════════════════════════════════════════════════════════
 *  Label — ABS (Assigned Buffer Static)
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_label_set_text_abs(ui_label_abs_id_t id, const char *fmt, ...)
{
    if (!fmt) return ESP_ERR_INVALID_ARG;

    lvgl_msg_t msg = {
        .cmd            = LVGL_CMD_LABEL_ABS_SET_TEXT,
        .data.label_abs = { .id = id },
    };

    /* Formatear en el caller — texto listo antes de entrar a la queue.
     * msg.data.label_abs.text es un char[256] en stack/queue, no en PSRAM.
     * La task LVGL lo copia al buffer PSRAM en su propio contexto. */
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg.data.label_abs.text, sizeof(msg.data.label_abs.text), fmt, args);
    va_end(args);

    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Object Flags
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_add_flag(lv_obj_t *obj, lv_obj_flag_t flags)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_ADD_FLAG,
        .data.obj_flag = { .obj = obj, .flags = flags },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_remove_flag(lv_obj_t *obj, lv_obj_flag_t flags)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_REMOVE_FLAG,
        .data.obj_flag = { .obj = obj, .flags = flags },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Object State
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_add_state(lv_obj_t *obj, lv_state_t state)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_ADD_STATE,
        .data.obj_state = { .obj = obj, .state = state },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_remove_state(lv_obj_t *obj, lv_state_t state)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_REMOVE_STATE,
        .data.obj_state = { .obj = obj, .state = state },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Style
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_set_style_opa(lv_obj_t *obj, lv_opa_t opa,
                               lv_style_selector_t selector)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_STYLE_OPA,
        .data.style_opa = { .obj = obj, .opa = opa, .selector = selector },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_style_bg_color(lv_obj_t *obj, lv_color_t color,
                                    lv_style_selector_t selector)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_STYLE_BG_COLOR,
        .data.style_color = { .obj = obj, .color = color, .selector = selector },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_style_bg_opa(lv_obj_t *obj, lv_opa_t opa,
                                  lv_style_selector_t selector)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_STYLE_BG_OPA,
        .data.style_opa = { .obj = obj, .opa = opa, .selector = selector },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_style_text_color(lv_obj_t *obj, lv_color_t color,
                                      lv_style_selector_t selector)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_STYLE_TEXT_COLOR,
        .data.style_color = { .obj = obj, .color = color, .selector = selector },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Image
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_image_set_src(lv_obj_t *obj, const void *src)
{
    if (!obj || !src) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_IMAGE_SET_SRC,
        .data.image_src = { .obj = obj, .src = src },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_obj_set_style_bg_image_src(lv_obj_t *obj, const void *src,
                                        lv_style_selector_t selector)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_STYLE_BG_IMAGE_SRC,
        .data.style_bg_image_src = { .obj = obj, .src = src, .selector = selector },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_image_set_src_by_id(lv_obj_t *obj, ui_icon_id_t icon_id)
{
    if (!obj || icon_id >= UI_ICON_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }
    const lv_img_dsc_t *src = images_ctl_internal_get_src(icon_id);
    if (!src) {
        return ESP_ERR_INVALID_ARG;
    }
    return ui_image_set_src(obj, src);
}
/* ══════════════════════════════════════════════════════════════════
 *  Geometry
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_set_pos(lv_obj_t *obj, int32_t x, int32_t y)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_POS,
        .data.obj_pos = { .obj = obj, .x = x, .y = y },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_size(lv_obj_t *obj, int32_t w, int32_t h)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_SIZE,
        .data.obj_size = { .obj = obj, .w = w, .h = h },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_align(lv_obj_t *obj, lv_align_t align)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_ALIGN,
        .data.obj_align = { .obj = obj, .align = align },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Misc
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_invalidate(lv_obj_t *obj)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_INVALIDATE,
        .data.obj_simple = { .obj = obj, .value = false },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_hidden(lv_obj_t *obj, bool hidden)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_HIDDEN,
        .data.obj_simple = { .obj = obj, .value = hidden },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Backlight (direct, no queue)
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_backlight_on(void)                   { return lvgl_backlight_on(); }
esp_err_t ui_backlight_off(void)                  { return lvgl_backlight_off(); }
esp_err_t ui_backlight_set(int brightness_percent) { return lvgl_backlight_set(brightness_percent); }
