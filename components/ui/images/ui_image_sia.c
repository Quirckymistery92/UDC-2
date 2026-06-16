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
    /* 0 */ [UI_ICON_BAT_CHARGING]       = &img_bat_charging30,
    /* 2 */ [UI_ICON_BAT_ULTRA_LOW]      = &img_bat_ultra_low30,
    /* 3 */ [UI_ICON_BAT_LOW]            = &img_bat_low30,
    /* 4 */ [UI_ICON_BAT_MED]            = &img_bat_med30,
    /* 5 */ [UI_ICON_BAT_MAX]            = &img_bat_max30,
    /* 6 */ [UI_ICON_IOT_ACTIVE]         = &img_iot_active30,
    /* 7 */ [UI_ICON_WIFI_ON]            = &img_wifi_on30,
            [UI_ICON_WIFI_OFF]           = &img_wifi_off30,
            [UI_ICON_IOT_FACE_ID]        = &img_face_id40,
            [UI_ICON_MENUDOT_V]          = &img_menudot_v30,
            [UI_ICON_USER_LOGO_DEFAULT]          = &img_user_logo_default50,
            [UI_ICON_USER_LOGO_BABY]          = &img_user_logo_baby50,
            [UI_ICON_USER_LOGO_CHILD]          = &img_user_logo_child50,
            [UI_ICON_USER_LOGO_DIAMOND]          = &img_user_logo_diamond50,
            [UI_ICON_USER_LOGO_DOG]          = &img_user_logo_dog50,
            [UI_ICON_USER_LOGO_FATHER]          = &img_user_logo_father50,
            [UI_ICON_USER_LOGO_GIRL]          = &img_user_logo_girl50,
            [UI_ICON_USER_LOGO_GRANNY]          = &img_user_logo_granny50,
            [UI_ICON_USER_LOGO_GUEST]          = &img_user_logo_guest50,
            [UI_ICON_USER_LOGO_GUY]          = &img_user_logo_guy50,
            [UI_ICON_USER_LOGO_HAPPY]          = &img_user_logo_happy50,
            [UI_ICON_USER_LOGO_MATH]          = &img_user_logo_math50,
            [UI_ICON_USER_LOGO_MOM]          = &img_user_logo_mom50,
            [UI_ICON_USER_LOGO_NATURE]          = &img_user_logo_nature50,
            [UI_ICON_USER_LOGO_NORMAL]          = &img_user_logo_normal50,
            [UI_ICON_USER_LOGO_OWL]          = &img_user_logo_owl50,
            [UI_ICON_USER_LOGO_PAWPRINT]          = &img_user_logo_pawprint50,
            [UI_ICON_USER_LOGO_RAT]          = &img_user_logo_rat50,
            [UI_ICON_USER_LOGO_ROBOT]          = &img_user_logo_robot50,
            [UI_ICON_USER_LOGO_SAD]          = &img_user_logo_sad50,
            [UI_ICON_USER_LOGO_SHOCKED]          = &img_user_logo_shocked50,
            [UI_ICON_USER_LOGO_VERY_HAPPY]          = &img_user_logo_very_happy50,
            [UI_ICON_USER_LOGO_WORRIED]          = &img_user_logo_worried50,
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
