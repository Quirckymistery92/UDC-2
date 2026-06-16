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
 *  Enum — IDs de íconos estáticos asignados (SIA) (Static Icons Asigned)
 *
 *  Cada valor corresponde al índice dentro de la tabla sia_icons[].
 *  El tamaño subyacente es uint8_t → máx. 255 íconos.
 * ══════════════════════════════════════════════════════════════════ */
typedef enum : uint8_t {
    UI_ICON_BAT_CHARGING        = 0,  /**< Batería cargando            */
    UI_ICON_BAT_ULTRA_LOW = 1,            /**< Batería ultrabaja           */
    UI_ICON_BAT_LOW = 2,                  /**< Batería baja                */
    UI_ICON_BAT_MED = 3,                  /**< Batería media               */
    UI_ICON_BAT_MAX = 4,                  /**< Batería máxima              */
    UI_ICON_IOT_ACTIVE = 5,               /**< IoT activo                  */
    UI_ICON_WIFI_ON = 6,                  /**< WiFi conectado              */
    UI_ICON_WIFI_OFF = 7,                 /**< WiFi desconectado           */
    UI_ICON_IOT_FACE_ID = 8,              /**< IoT Face ID                 */
    UI_ICON_MENUDOT_V = 9,                /**< Menú punto vertical         */

    UI_ICON_USER_LOGO_DEFAULT = 11,        /**< Logo de usuario             */
    UI_ICON_USER_LOGO_BABY = 12,           /**< Logo de bebé                */
    UI_ICON_USER_LOGO_CHILD = 13,          /**< Logo de niño                */
    UI_ICON_USER_LOGO_DIAMOND = 14,        /**< Logo de diamante            */
    UI_ICON_USER_LOGO_DOG = 15,            /**< Logo de perro               */
    UI_ICON_USER_LOGO_FATHER = 16,         /**< Logo de padre               */
    UI_ICON_USER_LOGO_GIRL = 17,           /**< Logo de chica               */
    UI_ICON_USER_LOGO_GRANNY = 18,         /**< Logo de abuela              */
    UI_ICON_USER_LOGO_GUEST = 19,          /**< Logo de invitado            */
    UI_ICON_USER_LOGO_GUY = 20,            /**< Logo de chico               */
    UI_ICON_USER_LOGO_HAPPY = 21,          /**< Logo de usuario feliz       */
    UI_ICON_USER_LOGO_MATH = 22,           /**< Logo de usuario matemática  */
    UI_ICON_USER_LOGO_MOM = 23,            /**< Logo de usuario mamá        */
    UI_ICON_USER_LOGO_NATURE = 24,         /**< Logo de usuario natural     */
    UI_ICON_USER_LOGO_NORMAL = 25,         /**< Logo de usuario cara normal */
    UI_ICON_USER_LOGO_OWL = 26,            /**< Logo de usuario búho        */
    UI_ICON_USER_LOGO_PAWPRINT = 27,       /**< Logo de usuario huella gato */
    UI_ICON_USER_LOGO_RAT = 28,            /**< Logo de usuario rata        */
    UI_ICON_USER_LOGO_ROBOT = 29,          /**< Logo de usuario robot       */
    UI_ICON_USER_LOGO_SAD = 30,            /**< Logo de usuario cara triste */
    UI_ICON_USER_LOGO_SHOCKED = 31,        /**< Logo de usuario cara asombro*/
    UI_ICON_USER_LOGO_VERY_HAPPY = 32,     /**< Logo de usuario muy feliz   */
    UI_ICON_USER_LOGO_WORRIED = 33,        /**< Logo de usuario cara asombro*/
    /* ── Añadir nuevos íconos antes de esta línea ────────────────── */
    UI_ICON_COUNT                     /**< Total de íconos (centinela) */
} ui_icon_id_t;

/* Verificación en compile-time C11: el enum cabe en uint8_t */ 
_Static_assert(UI_ICON_COUNT <= 255,
               "ui_icon_id_t: demasiados iconos para uint8_t");

#ifdef __cplusplus
}
#endif
