/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * ui_image_sia.c — Implementación de la tabla SIA y ui_image_set_src_sia
 *
 * La tabla sia_icons[] vive en flash (const) y mapea directamente
 * ui_icon_id_t → lv_img_dsc_t*. ui_image_set_src_sia delega al
 * comando LVGL_CMD_IMAGE_SET_SRC via ui_image_set_src(), sin añadir
 * ningún comando extra al queue.
 */

#include "ui_image_sia.h"
#include "ui_manager.h"
#include "images.h"

/* ══════════════════════════════════════════════════════════════════
 *  Tabla SIA — compile-time, reside en flash
 *
 *  El orden DEBE coincidir con ui_icon_id_e en ui_image_sia.h.
 *  Usar designated initializers para detectar huecos en compile-time.
 * ══════════════════════════════════════════════════════════════════ */
static const lv_img_dsc_t *const sia_icons[UI_ICON_COUNT] = {
    [UI_ICON_BAT_CHARGING]       = &img_bat_charging,
    [UI_ICON_BAR_HOME_SMILE]     = &img_bar_home_smile,
    [UI_ICON_BAT_ULTRA_LOW]      = &img_bat_ultra_low,
    [UI_ICON_BAT_LOW]            = &img_bat_low,
    [UI_ICON_BAT_MED]            = &img_bat_med,
    [UI_ICON_BAT_MAX]            = &img_bat_max,
    [UI_ICON_IOT_ACTIVE]         = &img_iot_active,
    [UI_ICON_WIFI_ON]            = &img_wifi_on,
    [UI_ICON_WIFI_OFF]           = &img_wifi_off,
    [UI_ICON_TESTING]            = &img_testing,
    [UI_ICON_MINI_PLUG_LOCAL20]  = &img_mini_plug_local20,
    [UI_ICON_MINI_IOT_WIRELESS20]= &img_mini_iot_wireless20,
    [UI_ICON_IOT_FACE_ID]        = &img_iot_face_id,
    [UI_ICON_UTILS_APP_TIMER]    = &img_utils_app_timer,
    [UI_ICON_UTILS_AI_ACTIVITY]  = &img_utils_ai_activity,
    [UI_ICON_USER_LOGO]          = &img_user_logo,
    [UI_ICON_MENUDOT_V]          = &img_menudot_v,
};

/* Verificación en compile-time: la tabla cubre todos los IDs */
_Static_assert(
    sizeof(sia_icons) / sizeof(sia_icons[0]) == UI_ICON_COUNT,
    "sia_icons[]: faltan entradas — sincronizar con ui_icon_id_e"
);

/* ══════════════════════════════════════════════════════════════════
 *  API
 * ══════════════════════════════════════════════════════════════════ */
 const lv_img_dsc_t *images_ctl_internal_get_src(ui_icon_id_t icon_id)
{
    if (icon_id >= UI_ICON_COUNT) {
        return NULL;
    }
    return sia_icons[icon_id];
}
/*
esp_err_t ui_image_set_src_sia(lv_obj_t *obj, ui_icon_id_t icon_id)
{
    if (!obj || icon_id >= UI_ICON_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }
    return ui_image_set_src(obj, sia_icons[icon_id]);
}
*/
