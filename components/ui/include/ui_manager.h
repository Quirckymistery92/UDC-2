/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * UI Manager - Public API
 *
 * This is the ONLY header external code should include to interact
 * with the UI subsystem. All calls are task-safe (enqueued to LVGL task).
 */

#pragma once

#include "esp_err.h"
#include "lvgl.h"
#include "screens.h"     /* ScreensEnum, objects_t, extern objects */
#include "ui_image_sia.h" /* ui_icon_id_t — enum SIA de íconos estáticos asignados */

#ifdef __cplusplus
extern "C" {
#endif

#define UI_CMD_TIMEOUT_MS  (1000)

typedef enum : uint8_t {
    UI_IMG_THUNDER_RAIL,
} ui_backg_img_t;

/* ══════════════════════════════════════════════════════════════════
 *  Initialization & Control
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize the entire UI subsystem.
 *
 * Internally calls lvgl_init() (task, queue, mutex, tick timer)
 * then waits for the task to be ready and turns on the backlight.
 *
 * @return ESP_OK on success
 */
esp_err_t ui_init(void);

void ui__helper_sync_sys_screens_semaphore(ScreensEnum screen_id);

/* ══════════════════════════════════════════════════════════════════
 *  Screen Management
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief Change the active screen with animation.
 *
 * @param screen_id  Screen identifier from ScreensEnum
 * @param anim       Transition animation (e.g. LV_SCR_LOAD_ANIM_FADE_IN)
 * @param time       Animation duration in ms
 * @param delay      Delay before animation starts in ms
 * @param auto_del   Delete the old screen after transition
 * @return ESP_OK if command enqueued
 */
esp_err_t ui_screen_change(ScreensEnum screen_id,
                           lv_scr_load_anim_t anim,
                           uint32_t time,
                           uint32_t delay,
                           bool auto_del);
/**
 * @brief Cambiar imagen del fondo de pantalla de Home (Método Asíncrono)
 * 
 * @param image_path Ruta de la imagen a establecer como fondo
 * @return ´esp_err_t´ Código de error de la operación
 */
esp_err_t ui_async_screen_backg_update(ui_backg_img_t img, lv_obj_t *obj);
/* ══════════════════════════════════════════════════════════════════
 *  Label — ABS (Assigned Buffer Static)
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief IDs de labels con buffer estático en PSRAM.
 *
 * Cada entry corresponde a un lv_obj_t* (label) y un buffer fijo
 * en PSRAM asignado en compile time. Agregar nuevos IDs aquí y
 * registrar la entry correspondiente en core/ui_label_abs.c.
 */
typedef enum {
    UI_LABEL_ABS_NOTCH_HOUR = 0,  /**< Notch — hora "HH:MM" / "HH:MM:SS" (buf: 12 bytes) */
    UI_LABEL_ABS_LOCKED_USER_SELECTED_NAME,
    UI_LABEL_ABS_LOCKED_USER_SELECTED_DATA,
    UI_LABEL_ABS_NOTCH_EXT_BAT,
    UI_LABEL_ABS_NOTCH_EXT_WIFI,
    UI_LABEL_ABS_NOTCH_EXT_ZIGBEE,
    UI_LABEL_ABS_NOTCH_EXT_UBICATION,
    UI_LABEL_ABS_LOCKED_USERS_1_NAME,
    UI_LABEL_ABS_LOCKED_USERS_1_DATA,
    UI_LABEL_ABS_LOCKED_USERS_2_NAME,
    UI_LABEL_ABS_LOCKED_USERS_2_DATA,
    UI_LABEL_ABS_LOCKED_USERS_3_NAME,
    UI_LABEL_ABS_LOCKED_USERS_3_DATA,
    UI_LABEL_ABS_LOCKED_USERS_4_NAME,
    UI_LABEL_ABS_LOCKED_USERS_4_DATA,
    UI_LABEL_ABS_LOCKED_USERS_5_NAME,
    UI_LABEL_ABS_LOCKED_USERS_5_DATA,
    UI_LABEL_ABS_LOCKED_USERS_6_NAME,
    UI_LABEL_ABS_LOCKED_USERS_6_DATA,
    UI_LABEL_ABS_LOCKED_PIN_MESSAGE,
    UI_LABEL_ABS_COUNT
} ui_label_abs_id_t;

/**
 * @brief Actualiza el texto de un label usando su buffer estático en PSRAM.
 *
 * El texto se formatea con printf-style en la task del caller y se encola
 * en el mensaje. La task LVGL lo copia al buffer PSRAM y llama
 * "abs" = Assigned Buffer Static: el buffer es fijo, asignado en compile time, siempre vivo.
 *
 * @param id    ID del label (ui_label_abs_id_t)
 * @param fmt   Formato printf
 * @param ...   Argumentos del formato
 * @return ESP_OK si el comando fue encolado correctamente
 */
esp_err_t ui_label_set_text_abs(ui_label_abs_id_t id, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

/* ══════════════════════════════════════════════════════════════════
 *  Object Flags
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_add_flag(lv_obj_t *obj, lv_obj_flag_t flags);
esp_err_t ui_obj_remove_flag(lv_obj_t *obj, lv_obj_flag_t flags);

/* ══════════════════════════════════════════════════════════════════
 *  Object State
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_add_state(lv_obj_t *obj, lv_state_t state);
esp_err_t ui_obj_remove_state(lv_obj_t *obj, lv_state_t state);

esp_err_t ui_obj_set_user_data(lv_obj_t *obj, void *user_data);

/* ══════════════════════════════════════════════════════════════════
 *  Style Properties
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_set_style_opa(lv_obj_t *obj, lv_opa_t opa,
                               lv_style_selector_t selector);
esp_err_t ui_obj_set_style_bg_color(lv_obj_t *obj, lv_color_t color,
                                    lv_style_selector_t selector);
esp_err_t ui_obj_set_style_bg_opa(lv_obj_t *obj, lv_opa_t opa,
                                  lv_style_selector_t selector);
esp_err_t ui_obj_set_style_bg_image_src(lv_obj_t *obj,
                                        const void *src,
                                        lv_style_selector_t selector);
esp_err_t ui_obj_set_style_text_color(lv_obj_t *obj, lv_color_t color,
                                      lv_style_selector_t selector);

/* ══════════════════════════════════════════════════════════════════
 *  Image
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_image_set_src(lv_obj_t *obj, const void *src);

/**
 * @brief Cambia la imagen de un lv_obj_t usando un ícono identificado por su ID de la tabla SIA (Static Icon Assigned).
 * 
 * @param obj      Objeto imagen LVGL destino (lv_obj_t*, tipo image).
 * @param icon_id  ID del ícono, valor de @ref ui_icon_id_t.
 *                 Debe ser < @c UI_ICON_COUNT.
 * @return ESP_OK              Comando encolado correctamente.
 * @return ESP_ERR_INVALID_ARG @p obj es NULL o @p icon_id fuera de rango.
 */
esp_err_t ui_image_set_src_by_id(lv_obj_t *obj, ui_icon_id_t icon_id);

/* ══════════════════════════════════════════════════════════════════
 *  Geometry
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_set_pos(lv_obj_t *obj, int32_t x, int32_t y);
esp_err_t ui_obj_set_size(lv_obj_t *obj, int32_t w, int32_t h);
esp_err_t ui_obj_set_align(lv_obj_t *obj, lv_align_t align);

/* ══════════════════════════════════════════════════════════════════
 *  Miscellaneous
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_invalidate(lv_obj_t *obj);
esp_err_t ui_obj_set_hidden(lv_obj_t *obj, bool hidden);

/* ══════════════════════════════════════════════════════════════════
 *  Specific
 * ══════════════════════════════════════════════════════════════════ */
 
esp_err_t ui_battery_set_data(int percentage, int voltage_mv, bool is_charging);

bool ui_display_is_created_ds_home(void);
bool ui_display_is_created_ds_top(void);
bool ui_display_is_created_ds_locked(void);

/* ══════════════════════════════════════════════════════════════════
 *  Backlight
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_backlight_on(void);
esp_err_t ui_backlight_off(void);
esp_err_t ui_backlight_set(int brightness_percent);

#ifdef __cplusplus
}
#endif
