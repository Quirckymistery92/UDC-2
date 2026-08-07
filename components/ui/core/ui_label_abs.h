/**
 * @file ui_label_abs.h
 * @author Alex Jaque
 * @brief Gestor de buffers estáticos para textos dinámicos de LVGL.
 * @details
 * Este módulo proporciona una forma de manejar textos dinámicos en la interfaz de usuario (UX)
 * utilizando buffers estáticos en PSRAM. Cada label dinámico tiene un buffer asociado que se
 * puede actualizar en tiempo de ejecución. La tabla estática `s_table` mapea cada label a su buffer correspondiente y al objeto LVGL asociado.
 * @version 0.2.7
 * @date 2026-07-09
 * 
 * @copyright Copyright (c) 2026 Alex Jaque.
 * SPDX-License-Identifier: Apache-2.0
 * 
 * @see ui_label_abs.h
 * @par Testing (11:13 07/08/2026)
 * - Frag Mem: PASSED.
 * - Heap: PASSED.
 * - LVGL: PASSED.
 * 
 */
#pragma once

#include "esp_err.h"
#include "lvgl.h"
#include "../include/ui_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Copia el texto al buffer PSRAM de la entry indicada y devuelve
 *        el puntero al objeto LVGL y al buffer ya escrito.
 *
 * @param id       ID del label (enum ui_label_abs_id_t en ui_manager.h)
 * @param text     Texto a copiar (stack/heap/literal — no importa el origen)
 * @param out_obj  [out] Puntero al lv_obj_t* correspondiente
 * @param out_buf  [out] Puntero al buffer PSRAM ya actualizado (static, siempre vivo)
 * @return ESP_ERR_INVALID_ARG: Argumentos inválidos.
 * @return ESP_ERR_INVALID_STATE: El objeto LVGL aún no ha sido creado.
 * @return ESP_OK: Operación exitosa.
 */
esp_err_t label_abs_prepare(ui_label_abs_id_t id,
                             const char       *text,
                             lv_obj_t        **out_obj,
                             const char      **out_buf);

/**
 * @brief Helper interno más rápido para aplicar textos internos dentro de la task. (Aplicador rápido)
 * 
 * @param id       ID del label (enum ui_label_abs_id_t en ui_manager.h)
 * @param text     Texto a copiar (stack/heap/literal — no importa el origen)
 * @return ESP_ERR_INVALID_ARG: Argumentos inválidos.
 * @return ESP_ERR_INVALID_STATE: El objeto LVGL aún no ha sido creado.
 * @return ESP_OK: Operación exitosa.
 */
esp_err_t ui_int__helper_label_abs_apply_sync(ui_label_abs_id_t id, const char *text);

/**
 * @brief Helper interno más rápido para aplicar textos formateados dentro de la task. (Aplicador rápido)
 * 
 * @param id ID del label (enum ui_label_abs_id_t en ui_manager.h)
 * @param fmt Texto formateado (printf-style)
 * @param ... Argumentos para el formato
 * @return ESP_ERR_INVALID_ARG: Argumentos inválidos.
 * @return ESP_ERR_INVALID_STATE: El objeto LVGL aún no ha sido creado.
 * @return ESP_OK: Operación exitosa.
 */
esp_err_t ui_int__helper_label_abs_apply_sync_fmt(ui_label_abs_id_t id, const char *fmt, ...);
#ifdef __cplusplus
}
#endif
