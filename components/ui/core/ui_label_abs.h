/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * ui_label_abs — Internal header
 *
 * Tabla estática de buffers en PSRAM para labels con texto dinámico.
 * Cada entry asocia un enum ID → buffer fijo en PSRAM → lv_obj_t* (double pointer).
 *
 * PRIVADO: no incluir fuera del componente ui. La API pública es ui_manager.h.
 */

#pragma once

#include "esp_err.h"
#include "lvgl.h"
#include "../include/ui_manager.h"  /* ui_label_abs_id_t definido ahí */

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════
 *  API interna — llamada exclusivamente desde ui_manager.c
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief Copia el texto al buffer PSRAM de la entry indicada y devuelve
 *        el puntero al objeto LVGL y al buffer ya escrito.
 *
 * @param id       ID del label (enum ui_label_abs_id_t)
 * @param text     Texto a copiar (stack/heap/literal — no importa el origen)
 * @param out_obj  [out] Puntero al lv_obj_t* correspondiente
 * @param out_buf  [out] Puntero al buffer PSRAM ya actualizado (static, siempre vivo)
 * @return ESP_OK si la operación fue exitosa
 */
esp_err_t label_abs_prepare(ui_label_abs_id_t id,
                             const char       *text,
                             lv_obj_t        **out_obj,
                             const char      **out_buf);

esp_err_t ui_int__helper_label_abs_apply_sync(ui_label_abs_id_t id, const char *text);
#ifdef __cplusplus
}
#endif
