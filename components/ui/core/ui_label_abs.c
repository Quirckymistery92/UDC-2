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
 *
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
/* Add buff: #define BUF_SIZE_XXX   N */

/* ══════════════════════════════════════════════════════════════════
 *  Buffers estáticos en PSRAM
 *  EXT_RAM_BSS_ATTR → segmento .ext_ram.bss → PSRAM, sin malloc
 * ══════════════════════════════════════════════════════════════════ */

static EXT_RAM_BSS_ATTR char buf_notch_hour[BUF_SIZE_NOTCH_HOUR];
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
        .obj_ptr  = &objects.notch_hour,
        .buf      = buf_notch_hour,
        .buf_size = BUF_SIZE_NOTCH_HOUR,
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
