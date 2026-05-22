/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * ui_image_sia.h — Static Icon Assigned (SIA) helper
 *
 * Define el enum ui_icon_id_t que mapea compile-time cada ícono/imagen
 * registrado en la carpeta images/ a un identificador tipado uint8_t.
 *
 * Para añadir un nuevo ícono:
 *   1. Agregar el .c del ícono en images/ y declarar su lv_img_dsc_t en images.h
 *   2. Añadir una nueva entrada al final de ui_icon_id_e (antes de UI_ICON_COUNT)
 *   3. Añadir la entrada correspondiente en la tabla sia_icons[] de ui_image_sia.c
 *   Eso es todo — no hay que tocar nada más.
 */

#pragma once

#include <stdint.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════
 *  Enum — IDs de íconos estáticos asignados (SIA)
 *
 *  Cada valor corresponde al índice dentro de la tabla sia_icons[].
 *  El tamaño subyacente es uint8_t → máx. 255 íconos.
 * ══════════════════════════════════════════════════════════════════ */
typedef enum : uint8_t {
    UI_ICON_BAT_CHARGING        = 0,  /**< Batería cargando            */
    UI_ICON_BAR_HOME_SMILE,           /**< Barra home — smile          */
    UI_ICON_BAT_ULTRA_LOW,            /**< Batería ultrabaja           */
    UI_ICON_BAT_LOW,                  /**< Batería baja                */
    UI_ICON_BAT_MED,                  /**< Batería media               */
    UI_ICON_BAT_MAX,                  /**< Batería máxima              */
    UI_ICON_IOT_ACTIVE,               /**< IoT activo                  */
    UI_ICON_WIFI_ON,                  /**< WiFi conectado              */
    UI_ICON_WIFI_OFF,                 /**< WiFi desconectado           */
    UI_ICON_TESTING,                  /**< Ícono de testing            */
    UI_ICON_MINI_PLUG_LOCAL20,        /**< Mini plug local (20 px)     */
    UI_ICON_MINI_IOT_WIRELESS20,      /**< Mini IoT wireless (20 px)   */
    UI_ICON_IOT_FACE_ID,              /**< IoT Face ID                 */
    UI_ICON_UTILS_APP_TIMER,          /**< Utils — app timer           */
    UI_ICON_UTILS_AI_ACTIVITY,        /**< Utils — AI activity         */
    UI_ICON_USER_LOGO,                /**< Logo de usuario             */
    UI_ICON_MENUDOT_V,                /**< Menú punto vertical         */
    /* ── Añadir nuevos íconos antes de esta línea ────────────────── */
    UI_ICON_COUNT                     /**< Total de íconos (centinela) */
} ui_icon_id_t;

/* Verificación en compile-time C11: el enum cabe en uint8_t */ 
_Static_assert(UI_ICON_COUNT <= 255,
               "ui_icon_id_t: demasiados iconos para uint8_t");

#ifdef __cplusplus
}
#endif
