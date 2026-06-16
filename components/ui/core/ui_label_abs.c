/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * ui_label_abs — Implementación
 *
 * Tabla interna de buffers estáticos en PSRAM para labels dinámicos.
 * Los buffers se ubican en PSRAM mediante EXT_RAM_BSS_ATTR, garantizando
 * asignación estática en compile/link time — sin malloc, sin fragmentación.
 *
 * ─── CÓMO AGREGAR UN NUEVO LABEL ────────────────────────────────────────────
 *
 *  1. En include/ui_manager.h, agregar el ID al enum ui_label_abs_id_t:
 *         UI_LABEL_ABS_MI_LABEL,
 *
 *  2. Aquí, definir el tamaño del buffer:
 *         #define BUF_SIZE_MI_LABEL   N   // N = max chars + 1 (null terminator)
 *
 *  3. Declarar el buffer estático en PSRAM:
 *         static EXT_RAM_BSS_ATTR char buf_mi_label[BUF_SIZE_MI_LABEL];
 *
 *  4. Agregar la entry en s_table[]:
 *         [UI_LABEL_ABS_MI_LABEL] = {
 *             .obj_ptr  = &objects.mi_label,
 *             .buf      = buf_mi_label,
 *             .buf_size = BUF_SIZE_MI_LABEL,
 *         },
 *  5: listo xd
 * ────────────────────────────────────────────────────────────────────────────
 */

#include "ui_label_abs.h"
#include "../screens/screens.h"   /* objects_t, extern objects */
#include <string.h>
#include "esp_attr.h"
#include "esp_log.h"

static const char *TAG = "label_abs";

/* ══════════════════════════════════════════════════════════════════
 *  Tamaños de buffer por label (compile time)
 * ══════════════════════════════════════════════════════════════════ */

#define BUF_SIZE_NOTCH_HOUR   12   /* "HH:MM:SS\0" + margen = 12 */
#define BUF_SIZE_NOTCH_EXT_BAT 17
#define BUF_SIZE_LOCK_USER_SELECTED_NAME 16
#define BUF_SIZE_LOCK_USER_SELECTED_DATA 28

#define BUF_SIZE_USERS_1_NAME 16
#define BUF_SIZE_USERS_1_DATA 28

#define BUF_SIZE_USERS_2_NAME 16
#define BUF_SIZE_USERS_2_DATA 28

#define BUF_SIZE_USERS_3_NAME 16
#define BUF_SIZE_USERS_3_DATA 28

#define BUF_SIZE_USERS_4_NAME 16
#define BUF_SIZE_USERS_4_DATA 28

#define BUF_SIZE_USERS_5_NAME 16
#define BUF_SIZE_USERS_5_DATA 28

#define BUF_SIZE_USERS_6_NAME 16
#define BUF_SIZE_USERS_6_DATA 28

#define BUF_SIZE_LOCK_PIN_MESSAGE 64
/* Add buff: #define BUF_SIZE_XXX   N */

/* ══════════════════════════════════════════════════════════════════
 *  Buffers estáticos en PSRAM
 *  EXT_RAM_BSS_ATTR → segmento .ext_ram.bss → PSRAM, sin malloc
 * ══════════════════════════════════════════════════════════════════ */

static EXT_RAM_BSS_ATTR char buf_notch_hour[BUF_SIZE_NOTCH_HOUR];
static EXT_RAM_BSS_ATTR char buf_notch_ext_bat[BUF_SIZE_NOTCH_EXT_BAT];

static EXT_RAM_BSS_ATTR char buf_users_1_name[BUF_SIZE_USERS_1_NAME];
static EXT_RAM_BSS_ATTR char buf_users_1_data[BUF_SIZE_USERS_1_DATA];

static EXT_RAM_BSS_ATTR char buf_users_2_name[BUF_SIZE_USERS_2_NAME];
static EXT_RAM_BSS_ATTR char buf_users_2_data[BUF_SIZE_USERS_2_DATA];

static EXT_RAM_BSS_ATTR char buf_users_3_name[BUF_SIZE_USERS_3_NAME];
static EXT_RAM_BSS_ATTR char buf_users_3_data[BUF_SIZE_USERS_3_DATA];

static EXT_RAM_BSS_ATTR char buf_users_4_name[BUF_SIZE_USERS_4_NAME];
static EXT_RAM_BSS_ATTR char buf_users_4_data[BUF_SIZE_USERS_4_DATA];

static EXT_RAM_BSS_ATTR char buf_users_5_name[BUF_SIZE_USERS_5_NAME];
static EXT_RAM_BSS_ATTR char buf_users_5_data[BUF_SIZE_USERS_5_DATA];

static EXT_RAM_BSS_ATTR char buf_users_6_name[BUF_SIZE_USERS_6_NAME];
static EXT_RAM_BSS_ATTR char buf_users_6_data[BUF_SIZE_USERS_6_DATA];

static EXT_RAM_BSS_ATTR char buf_lock_user_selected_name[BUF_SIZE_LOCK_USER_SELECTED_NAME];
static EXT_RAM_BSS_ATTR char buf_lock_user_selected_data[BUF_SIZE_LOCK_USER_SELECTED_DATA];

static EXT_RAM_BSS_ATTR char buf_lock_pin_message[BUF_SIZE_LOCK_PIN_MESSAGE];
/* Add buff: static EXT_RAM_BSS_ATTR char buf_xxx[BUF_SIZE_XXX]; */

/* ══════════════════════════════════════════════════════════════════
 *  Entrada de tabla
 * ══════════════════════════════════════════════════════════════════ */

typedef struct {
    lv_obj_t  **obj_ptr;   /**< Double pointer a objects.xxx — dirección fija en link time */
    char       *buf;       /**< Puntero al buffer PSRAM — dirección fija en link time      */
    uint16_t    buf_size;  /**< Tamaño en bytes del buffer (incluye null terminator)        */
} label_abs_entry_t;

/* ══════════════════════════════════════════════════════════════════
 *  Tabla estática — definida en compile time
 *  Los campos .buf y .obj_ptr son direcciones constantes (linkadas).
 *  El contenido de los buffers es mutable (escrito en runtime).
 * ══════════════════════════════════════════════════════════════════ */

static const label_abs_entry_t s_table[UI_LABEL_ABS_COUNT] = {

    [UI_LABEL_ABS_NOTCH_HOUR] = {
        .obj_ptr  = &objects.dtop_notch_min_hour,
        .buf      = buf_notch_hour,
        .buf_size = BUF_SIZE_NOTCH_HOUR,
    },
    [UI_LABEL_ABS_LOCKED_USER_SELECTED_NAME] = {
        .obj_ptr = &objects.dl_home_unlock_user_select_name,
        .buf = buf_lock_user_selected_name,
        .buf_size = BUF_SIZE_LOCK_USER_SELECTED_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USER_SELECTED_DATA] = {
        .obj_ptr = &objects.dl_home_unlock_user_select_data,
        .buf = buf_lock_user_selected_data,
        .buf_size = BUF_SIZE_LOCK_USER_SELECTED_DATA,
    },
    [UI_LABEL_ABS_NOTCH_EXT_BAT] = {
        .obj_ptr  = &objects.dtop_notch_exp_dat_bat_data,
        .buf      = buf_notch_ext_bat,
        .buf_size = BUF_SIZE_NOTCH_EXT_BAT,
    },
    
    [UI_LABEL_ABS_LOCKED_USERS_1_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_1_name,
        .buf      = buf_users_1_name,
        .buf_size = BUF_SIZE_USERS_1_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_1_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_1_data,
        .buf      = buf_users_1_data,
        .buf_size = BUF_SIZE_USERS_1_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_2_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_2_name,
        .buf      = buf_users_2_name,
        .buf_size = BUF_SIZE_USERS_2_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_2_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_2_data,
        .buf      = buf_users_2_data,
        .buf_size = BUF_SIZE_USERS_2_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_3_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_3_name,
        .buf      = buf_users_3_name,
        .buf_size = BUF_SIZE_USERS_3_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_3_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_3_data,
        .buf      = buf_users_3_data,
        .buf_size = BUF_SIZE_USERS_3_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_4_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_4_name,
        .buf      = buf_users_4_name,
        .buf_size = BUF_SIZE_USERS_4_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_4_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_4_data,
        .buf      = buf_users_4_data,
        .buf_size = BUF_SIZE_USERS_4_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_5_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_5_name,
        .buf      = buf_users_5_name,
        .buf_size = BUF_SIZE_USERS_5_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_5_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_5_data,
        .buf      = buf_users_5_data,
        .buf_size = BUF_SIZE_USERS_5_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_6_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_6_name,
        .buf      = buf_users_6_name,
        .buf_size = BUF_SIZE_USERS_6_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_6_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_6_data,
        .buf      = buf_users_6_data,
        .buf_size = BUF_SIZE_USERS_6_DATA,
    },
    [UI_LABEL_ABS_LOCKED_PIN_MESSAGE] = {
        .obj_ptr  = &objects.dl_home_unlock_pin_message,
        .buf      = buf_lock_pin_message,
        .buf_size = BUF_SIZE_LOCK_PIN_MESSAGE, 
    },
    /* Add buff:
    [UI_LABEL_ABS_XXX] = {
        .obj_ptr  = &objects.xxx,
        .buf      = buf_xxx,
        .buf_size = BUF_SIZE_XXX,
    },
    */
};

/* ══════════════════════════════════════════════════════════════════
 *  Implementación
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t label_abs_prepare(ui_label_abs_id_t id,
                             const char       *text,
                             lv_obj_t        **out_obj,
                             const char      **out_buf)
{
    if (id >= UI_LABEL_ABS_COUNT || !text || !out_obj || !out_buf) {
        return ESP_ERR_INVALID_ARG;
    }

    const label_abs_entry_t *e = &s_table[id];

    if (!(*e->obj_ptr)) {
        ESP_LOGE(TAG, "label id=%d: obj aún no creado", (int)id);
        return ESP_ERR_INVALID_STATE;
    }

    /* Copia segura al buffer PSRAM — trunca si excede buf_size */
    strlcpy(e->buf, text, (size_t)e->buf_size);

    *out_obj = *e->obj_ptr;
    *out_buf = (const char *)e->buf;

    return ESP_OK;
}
esp_err_t ui_int__helper_label_abs_apply_sync(ui_label_abs_id_t id, const char *text) {
    lv_obj_t *obj = NULL;
    const char * buf = NULL;
    esp_err_t ret = label_abs_prepare(id, text, &obj, &buf);
    if(ret != ESP_OK) return ret;

    lv_label_set_text_static(obj, buf);
    return ret;
}