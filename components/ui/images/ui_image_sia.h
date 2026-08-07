/**
 * @file ui_image_sia.h
 * @brief Static icon identifiers for SIA image resources.
 *
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
typedef enum {
  UI_ICON_BAT_CHARGING,  /**< Batería cargando            */
  UI_ICON_BAT_ULTRA_LOW, /**< Batería ultrabaja           */
  UI_ICON_BAT_LOW,       /**< Batería baja                */
  UI_ICON_BAT_MED,       /**< Batería media               */
  UI_ICON_BAT_MAX,       /**< Batería máxima              */
  UI_ICON_IOT_ACTIVE,    /**< IoT activo                  */
  UI_ICON_WIFI_ON,       /**< WiFi conectado              */
  UI_ICON_WIFI_OFF,      /**< WiFi desconectado           */
  UI_ICON_IOT_FACE_ID,   /**< IoT Face ID                 */
  UI_ICON_MENUDOT_V,     /**< Menú punto vertical         */
  UI_ICON_INFORMATION30, /**< Icono de información 30px  */
  UI_ICON_WARNING30,     /**< Icono de advertencia 30px  */

  UI_ICON_USER_LOGO_DEFAULT,    /**< Logo de usuario             */
  UI_ICON_USER_LOGO_BABY,       /**< Logo de bebé                */
  UI_ICON_USER_LOGO_CHILD,      /**< Logo de niño                */
  UI_ICON_USER_LOGO_DIAMOND,    /**< Logo de diamante            */
  UI_ICON_USER_LOGO_DOG,        /**< Logo de perro               */
  UI_ICON_USER_LOGO_FATHER,     /**< Logo de padre               */
  UI_ICON_USER_LOGO_GIRL,       /**< Logo de chica               */
  UI_ICON_USER_LOGO_GRANNY,     /**< Logo de abuela              */
  UI_ICON_USER_LOGO_GUEST,      /**< Logo de invitado            */
  UI_ICON_USER_LOGO_GUY,        /**< Logo de chico               */
  UI_ICON_USER_LOGO_HAPPY,      /**< Logo de usuario feliz       */
  UI_ICON_USER_LOGO_MATH,       /**< Logo de usuario matemática  */
  UI_ICON_USER_LOGO_MOM,        /**< Logo de usuario mamá        */
  UI_ICON_USER_LOGO_NATURE,     /**< Logo de usuario natural     */
  UI_ICON_USER_LOGO_NORMAL,     /**< Logo de usuario cara normal */
  UI_ICON_USER_LOGO_OWL,        /**< Logo de usuario búho        */
  UI_ICON_USER_LOGO_PAWPRINT,   /**< Logo de usuario huella gato */
  UI_ICON_USER_LOGO_RAT,        /**< Logo de usuario rata        */
  UI_ICON_USER_LOGO_ROBOT,      /**< Logo de usuario robot       */
  UI_ICON_USER_LOGO_SAD,        /**< Logo de usuario cara triste */
  UI_ICON_USER_LOGO_SHOCKED,    /**< Logo de usuario cara asombro*/
  UI_ICON_USER_LOGO_VERY_HAPPY, /**< Logo de usuario muy feliz   */
  UI_ICON_USER_LOGO_WORRIED,    /**< Logo de usuario cara asombro*/

  /* ── Añadir nuevos íconos antes de esta línea ────────────────── */
  UI_ICON_COUNT /**< Total de íconos (centinela) */
} ui_icon_id_t;

/* Verificación en compile-time: El enum cabe en uint8_t */ 
_Static_assert(UI_ICON_COUNT <= 255,
               "ui_icon_id_t: demasiados iconos para uint8_t");

#ifdef __cplusplus
}
#endif
