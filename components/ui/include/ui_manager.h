/**
 * @file ui_manager.h
 * @brief UI manager public API.
 *
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

typedef enum {
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

/**
 * @brief Señala al subsistema UI que la pantalla indicada está lista
 *        para continuar con el flujo sincronizado de inicialización.
 *
 * @param screen_id Identificador de pantalla del enum ScreensEnum.
 */
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
 * @param img ID de imagen de fondo
 * @param obj Objeto LVGL destino que recibirá la nueva imagen
 * @return ESP_OK si el comando fue encolado correctamente
 */
esp_err_t ui_async_screen_backg_update_DSHOME(ui_backg_img_t img, lv_obj_t *obj);

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
    UI_LABEL_ABS_DTOP_POPUP_TAG,
    UI_LABEL_ABS_DTOP_POPUP_MESSAGE,
    UI_LABEL_ABS_DTOP_NOTIFY_USER,
    UI_LABEL_ABS_HOME_USER_LOGGED_NAME,
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

/**
 * @brief Añade una bandera a un objeto LVGL.
 *
 * @param obj   Objeto LVGL destino.
 * @param flags Bandera(s) a aplicar.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_add_flag(lv_obj_t *obj, lv_obj_flag_t flags);

/**
 * @brief Elimina una bandera de un objeto LVGL.
 *
 * @param obj   Objeto LVGL destino.
 * @param flags Bandera(s) a quitar.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_remove_flag(lv_obj_t *obj, lv_obj_flag_t flags);

/* ══════════════════════════════════════════════════════════════════
 *  Object State
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief Añade un estado LVGL a un objeto.
 *
 * @param obj   Objeto LVGL destino.
 * @param state Estado a aplicar.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_add_state(lv_obj_t *obj, lv_state_t state);

/**
 * @brief Elimina un estado LVGL de un objeto.
 *
 * @param obj   Objeto LVGL destino.
 * @param state Estado a eliminar.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_remove_state(lv_obj_t *obj, lv_state_t state);

/**
 * @brief Asigna datos de usuario a un objeto LVGL.
 *
 * @param obj       Objeto LVGL destino.
 * @param user_data Puntero de usuario.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_set_user_data(lv_obj_t *obj, void *user_data);

/* ══════════════════════════════════════════════════════════════════
 *  Style Properties
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief Cambia la opacidad de un estilo en un objeto LVGL.
 *
 * @param obj      Objeto LVGL destino.
 * @param opa      Opacidad deseada.
 * @param selector Selector de estilo.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_set_style_opa(lv_obj_t *obj, lv_opa_t opa,
                               lv_style_selector_t selector);

/**
 * @brief Cambia el color de fondo de un objeto LVGL.
 *
 * @param obj      Objeto LVGL destino.
 * @param color    Color de fondo.
 * @param selector Selector de estilo.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_set_style_bg_color(lv_obj_t *obj, lv_color_t color,
                                    lv_style_selector_t selector);

/**
 * @brief Cambia la opacidad del fondo de un objeto LVGL.
 *
 * @param obj      Objeto LVGL destino.
 * @param opa      Opacidad de fondo.
 * @param selector Selector de estilo.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_set_style_bg_opa(lv_obj_t *obj, lv_opa_t opa,
                                  lv_style_selector_t selector);

/**
 * @brief Establece la imagen de fondo de un objeto LVGL.
 *
 * @param obj      Objeto LVGL destino.
 * @param src      Fuente de imagen.
 * @param selector Selector de estilo.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_set_style_bg_image_src(lv_obj_t *obj,
                                        const void *src,
                                        lv_style_selector_t selector);

/**
 * @brief Cambia el color de texto de un objeto LVGL.
 *
 * @param obj      Objeto LVGL destino.
 * @param color    Color de texto.
 * @param selector Selector de estilo.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_set_style_text_color(lv_obj_t *obj, lv_color_t color,
                                      lv_style_selector_t selector);

/* ══════════════════════════════════════════════════════════════════
 *  Image
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief Cambia la fuente de imagen de un objeto LVGL.
 *
 * @param obj Objeto LVGL destino.
 * @param src Fuente de imagen.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_image_set_src(lv_obj_t *obj, const void *src);

/**
 * @brief Cambia la imagen de un lv_obj_t usando un ícono identificado por su ID.
 *
 * @param obj     Objeto LVGL destino (tipo image).
 * @param icon_id Identificador del ícono estático.
 * @return ESP_OK si el comando fue encolado correctamente.
 * @return ESP_ERR_INVALID_ARG Si el objeto es NULL o el ID está fuera de rango.
 */
esp_err_t ui_image_set_src_by_id(lv_obj_t *obj, ui_icon_id_t icon_id);

/* ══════════════════════════════════════════════════════════════════
 *  Geometry
 * ══════════════════════════════════════════════════════════ */

/**
 * @brief Mueve un objeto LVGL a una nueva posición.
 *
 * @param obj Objeto LVGL destino.
 * @param x   Posición X absoluta.
 * @param y   Posición Y absoluta.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_set_pos(lv_obj_t *obj, int32_t x, int32_t y);

/**
 * @brief Cambia el tamaño de un objeto LVGL.
 *
 * @param obj Objeto LVGL destino.
 * @param w   Ancho en píxeles.
 * @param h   Alto en píxeles.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_set_size(lv_obj_t *obj, int32_t w, int32_t h);

/**
 * @brief Alinea un objeto LVGL respecto a su contenedor.
 *
 * @param obj   Objeto LVGL destino.
 * @param align Tipo de alineación LVGL.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_set_align(lv_obj_t *obj, lv_align_t align);

/* ══════════════════════════════════════════════════════════════════
 *  Miscellaneous
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief Marca un objeto LVGL para volver a dibujar.
 *
 * @param obj Objeto LVGL destino.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_invalidate(lv_obj_t *obj);

/**
 * @brief Muestra u oculta un objeto LVGL.
 *
 * @param obj    Objeto LVGL destino.
 * @param hidden true para ocultar, false para mostrar.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_obj_set_hidden(lv_obj_t *obj, bool hidden);

/* ══════════════════════════════════════════════════════════════════
 *  Specific
 * ══════════════════════════════════════════════════════════════════ */
 
/**
 * @brief Actualiza el estado de batería en la UI.
 *
 * @param percentage Nivel de carga en porcentaje.
 * @param voltage_mv Voltaje en milivoltios.
 * @param is_charging true si el dispositivo está cargando.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_battery_set_data(int percentage, int voltage_mv, bool is_charging);

/**
 * @brief Consulta si la pantalla DS_HOME ya fue creada.
 *
 * @return true si la pantalla existe.
 */
bool ui_display_is_created_ds_home(void);

/**
 * @brief Consulta si la pantalla DS_TOP ya fue creada.
 *
 * @return true si la pantalla existe.
 */
bool ui_display_is_created_ds_top(void);

/**
 * @brief Consulta si la pantalla DS_LOCKED ya fue creada.
 *
 * @return true si la pantalla existe.
 */
bool ui_display_is_created_ds_locked(void);

/**
 * @brief Envía una notificación al sistema UI.
 *
 * @param is_alert true para notificación de alerta, false para info.
 * @param fmt      Formato printf del mensaje.
 * @param ...      Argumentos del formato.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_spc_send_notify_user(bool is_alert, const char *fmt, ...);
/* ══════════════════════════════════════════════════════════════════
 *  Timer
 * ══════════════════════════════════════════════════════════════════ */
/**
 * @brief Pausa o reanuda un timer LVGL.
 *
 * @param timer Timer LVGL destino.
 * @param pause true para pausar, false para reanudar.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_timer_control(lv_timer_t *timer, bool pause);

/**
 * @brief Ajusta el periodo de un timer LVGL.
 *
 * @param timer  Timer LVGL destino.
 * @param period Periodo en milisegundos.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_timer_set_period(lv_timer_t *timer, uint32_t period);

/**
 * @brief Reinicia un timer LVGL.
 *
 * @param timer Timer LVGL destino.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_timer_reset(lv_timer_t *timer);

/**
 * @brief Cambia el contador de repetición de un timer LVGL.
 *
 * @param timer        Timer LVGL destino.
 * @param repeat_count Número de repeticiones.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_timer_set_repeat_count(lv_timer_t *timer, uint32_t repeat_count);

/**
 * @brief Habilita o deshabilita la eliminación automática de un timer LVGL.
 *
 * @param timer       Timer LVGL destino.
 * @param auto_delete true para eliminar automáticamente.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_timer_set_auto_delete(lv_timer_t *timer, bool auto_delete);

/**
 * @brief Asigna datos de usuario a un timer LVGL.
 *
 * @param timer     Timer LVGL destino.
 * @param user_data Puntero de usuario.
 * @return ESP_OK si el comando fue encolado correctamente.
 */
esp_err_t ui_timer_set_user_data(lv_timer_t *timer, void *user_data);
/* ══════════════════════════════════════════════════════════════════
 *  Backlight
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief Enciende la retroiluminación.
 *
 * @return ESP_OK si la operación tuvo éxito.
 */
esp_err_t ui_backlight_on(void);

/**
 * @brief Apaga la retroiluminación.
 *
 * @return ESP_OK si la operación tuvo éxito.
 */
esp_err_t ui_backlight_off(void);

/**
 * @brief Ajusta el brillo de la retroiluminación.
 *
 * @param brightness_percent Porcentaje de brillo (0-100).
 * @return ESP_OK si la operación tuvo éxito.
 */
esp_err_t ui_backlight_set(int brightness_percent);

#ifdef __cplusplus
}
#endif