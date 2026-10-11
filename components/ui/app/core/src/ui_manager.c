/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * UI Manager - Public API Implementation
 *
 * Every ui_*() function builds a lvgl_msg_t and enqueues it
 * to the LVGL task for safe execution.
 */

#include "ui_manager.h"

/* ESP */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Módulos */
#include "ui.h"
#include "ui_label_abs.h"
#include "ui_image_sia_internal.h"
#include "../../private/porting/porting.h"
#include "interface.h"
#include "ui_zb_controller.h"
#include "ui_check.h"

/* ══════════════════════════════════════════════════════════════════
 *  Initialization & Control
 * ══════════════════════════════════════════════════════════════════ */

app_desc_status_t *ui_init(void)
{
    return lvgl_init();

}
void ui__helper_sync_sys_screens_get_semaphore(void) {
    return ui_int__helper_get_lock_waiting_screen();
}

/* ══════════════════════════════════════════════════════════════════
 *  Screen
 * ══════════════════════════════════════════════════════════════════ */
esp_err_t ui_screen_loader(ScreensEnum screens_ids, ScreensEnum change_screen)
{
    if (screens_ids == SCREEN_MASK_NONE) {
        return ESP_ERR_INVALID_ARG;
    }

    if (change_screen != SCREEN_MASK_NONE &&
        (change_screen == SCREEN_MASK_ALL ||
         (change_screen & SCREEN_MASK_ALL) != change_screen ||
         (change_screen & (change_screen - 1U)) != 0)) {
        return ESP_ERR_INVALID_ARG;
    }
    
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_SYS_LOAD_SCREENS,
        .data.sys_load_screens = {
            .screen_id_mask = (uint8_t)screens_ids,
            .load = (uint8_t)change_screen,
        },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_screen_change(ScreensEnum screen_id,
                           lv_scr_load_anim_t anim,
                           uint32_t time,
                           uint32_t delay,
                           bool auto_del)
{
    if (screen_id == 0 ||
        (screen_id & (screen_id - 1)) != 0 ||
        (screen_id & SCREEN_MASK_ALL) == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_SCREEN_CHANGE,
        .data.screen_change = {
            .screen_id = (int16_t)screen_id,
            .anim      = anim,
            .time      = time,
            .delay     = delay,
            .auto_del  = auto_del,
        },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Label — ABS (Assigned Buffer Static)
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_label_set_text_abs(ui_label_abs_id_t id, const char *fmt, ...)
{
    if (!fmt) return ESP_ERR_INVALID_ARG;

    lvgl_msg_t msg = {
        .cmd            = LVGL_CMD_LABEL_ABS_SET_TEXT,
        .data.label_abs = { .id = id },
    };

    /* Formatear en el caller — texto listo antes de entrar a la queue.
     * msg.data.label_abs.text es un char[256] en stack/queue, no en PSRAM.
     * La task LVGL lo copia al buffer PSRAM en su propio contexto. */
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg.data.label_abs.text, sizeof(msg.data.label_abs.text), fmt, args);
    va_end(args);

    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Object Flags
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_add_flag(lv_obj_t *obj, lv_obj_flag_t flags)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_ADD_FLAG,
        .data.obj_flag = { .obj = obj, .flags = flags },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_remove_flag(lv_obj_t *obj, lv_obj_flag_t flags)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_REMOVE_FLAG,
        .data.obj_flag = { .obj = obj, .flags = flags },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Object State
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_add_state(lv_obj_t *obj, lv_state_t state)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_ADD_STATE,
        .data.obj_state = { .obj = obj, .state = state },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_remove_state(lv_obj_t *obj, lv_state_t state)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_REMOVE_STATE,
        .data.obj_state = { .obj = obj, .state = state },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_user_data(lv_obj_t *obj, void *user_data)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_USER_DATA,
        .data.obj_user_data = { .obj = obj, .user_data = user_data },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Style
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_set_style_opa(lv_obj_t *obj, lv_opa_t opa,
                               lv_style_selector_t selector)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_STYLE_OPA,
        .data.style_opa = { .obj = obj, .opa = opa, .selector = selector },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_style_bg_color(lv_obj_t *obj, lv_color_t color,
                                    lv_style_selector_t selector)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_STYLE_BG_COLOR,
        .data.style_color = { .obj = obj, .color = color, .selector = selector },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_style_bg_opa(lv_obj_t *obj, lv_opa_t opa,
                                  lv_style_selector_t selector)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_STYLE_BG_OPA,
        .data.style_opa = { .obj = obj, .opa = opa, .selector = selector },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_style_text_color(lv_obj_t *obj, lv_color_t color,
                                      lv_style_selector_t selector)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_STYLE_TEXT_COLOR,
        .data.style_color = { .obj = obj, .color = color, .selector = selector },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Image
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_image_set_src(lv_obj_t *obj, const void *src)
{
    if (!obj || !src) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_IMAGE_SET_SRC,
        .data.image_src = { .obj = obj, .src = src },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_obj_set_style_bg_image_src(lv_obj_t *obj, const void *src,
                                        lv_style_selector_t selector)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_STYLE_BG_IMAGE_SRC,
        .data.style_bg_image_src = { .obj = obj, .src = src, .selector = selector },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_image_set_src_by_id(lv_obj_t *obj, ui_icon_id_t icon_id)
{
    if (!obj || icon_id >= UI_ICON_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }
    const lv_img_dsc_t *src = ui_int__sia_icons_get_src_by_id(icon_id);
    if (!src) {
        return ESP_ERR_INVALID_ARG;
    }
    return ui_image_set_src(obj, src);
}
/* ══════════════════════════════════════════════════════════════════
 *  Geometry
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_set_pos(lv_obj_t *obj, int32_t x, int32_t y)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_POS,
        .data.obj_pos = { .obj = obj, .x = x, .y = y },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_size(lv_obj_t *obj, int32_t w, int32_t h)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_SIZE,
        .data.obj_size = { .obj = obj, .w = w, .h = h },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_align(lv_obj_t *obj, lv_align_t align)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_ALIGN,
        .data.obj_align = { .obj = obj, .align = align },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Misc
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t ui_obj_invalidate(lv_obj_t *obj)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_INVALIDATE,
        .data.obj_simple = { .obj = obj, .value = false },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui_obj_set_hidden(lv_obj_t *obj, bool hidden)
{
    if (!obj) return ESP_ERR_INVALID_ARG;
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_OBJ_SET_HIDDEN,
        .data.obj_simple = { .obj = obj, .value = hidden },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
/* ══════════════════════════════════════════════════════════════════
 *  Timer
 * ══════════════════════════════════════════════════════════════════ */
esp_err_t ui_timer_control(lv_timer_t *timer, bool pause) {
  if (!timer)
    return ESP_ERR_INVALID_ARG;
  lvgl_msg_t msg = {
      .cmd = LVGL_CMD_TIMER_CONTROL,
      .data.timer_resume = {.timer = timer, .extra.apply_pause = pause},
  };
  return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_timer_set_period(lv_timer_t *timer, uint32_t period) {
  if (!timer)
    return ESP_ERR_INVALID_ARG;
  lvgl_msg_t msg = {
      .cmd = LVGL_CMD_TIMER_SET_PERIOD,
      .data.timer_resume = {.timer = timer, .extra.period = period},
  };
  return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_timer_reset(lv_timer_t *timer) {
  if (!timer)
    return ESP_ERR_INVALID_ARG;
  lvgl_msg_t msg = {
      .cmd = LVGL_CMD_TIMER_RESET,
      .data.timer_resume = {.timer = timer},
  };
  return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_timer_set_repeat_count(lv_timer_t *timer, uint32_t repeat_count) {
  if (!timer)
    return ESP_ERR_INVALID_ARG;
  lvgl_msg_t msg = {
      .cmd = LVGL_CMD_TIMER_SET_REPEAT_COUNT,
      .data.timer_resume = {.timer = timer, .extra.repeat_count = repeat_count},
  };
  return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_timer_set_auto_delete(lv_timer_t *timer, bool auto_delete) {
  if (!timer)
    return ESP_ERR_INVALID_ARG;
  lvgl_msg_t msg = {
      .cmd = LVGL_CMD_TIMER_SET_AUTO_DELETE,
      .data.timer_resume = {.timer = timer, .extra.auto_delete = auto_delete},
  };
  return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_timer_set_user_data(lv_timer_t *timer, void *user_data) {
  if (!timer)
    return ESP_ERR_INVALID_ARG;
  lvgl_msg_t msg = {
      .cmd = LVGL_CMD_TIMER_SET_USER_DATA,
      .data.timer_resume = {.timer = timer, .extra.user_data = user_data},
  };
  return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
/* ══════════════════════════════════════════════════════════════════
 *  Specific
 * ══════════════════════════════════════════════════════════════════ */

bool ui_display_is_created_ds_home(void) {
    return ui_int__display_is_created_ds_home();
}
bool ui_display_is_created_ds_top(void) {
    return ui_int__display_is_created_ds_top();
}
bool ui_display_is_created_ds_locked(void) {
    return ui_int__display_is_created_ds_locked();
}
esp_err_t ui_spc_send_notify_user(bool is_alert, const char *fmt, ...)
{
    if (!fmt) return ESP_ERR_INVALID_ARG;

    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_SPC_NOTIFICATION_SHOW,
        .data.spc_notification_show = { .is_alert = is_alert },
    };

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg.data.spc_notification_show.msg, sizeof(msg.data.spc_notification_show.msg), fmt, args);
    va_end(args);

    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
/* ══════════════════════════════════════════════════════════════════
 *  Zigbee Hall
 * ══════════════════════════════════════════════════════════════════ */
esp_err_t ui_zb_hall_register_device(uint16_t id, const char *name, ui_zb_type_dev_t type, uint8_t column, uint8_t row, ui_zb_callback_result_t cb_result)
{
    if (name == NULL) return ESP_ERR_INVALID_ARG;
    
    size_t len_name = strlen(name);
    if (len_name < 5 || len_name >= MAX_DEV_NAME_LEN) return ESP_ERR_INVALID_ARG;

    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_ZB_REGISTER,
        .data.zigbee_hall = { .id = id, .result_cb = cb_result, .payload.register_dev = { .col = column, .row = row, .type = type} }
    };
    strlcpy(msg.data.zigbee_hall.payload.register_dev.name, name, MAX_DEV_NAME_LEN);

    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_zb_hall_unregister_device(uint16_t id, ui_zb_callback_result_t cb_result)
{
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_ZB_UNREGISTER,
        .data.zigbee_hall = { .id = id, .result_cb = cb_result}
    };

    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_zb_hall_update_device(uint16_t id, ui_zb_update_data_t data, ui_zb_callback_result_t cb_result)
{
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_ZB_UPDATE_DATA,
        .data.zigbee_hall = { .id = id, .result_cb = cb_result, .payload.update_data = data}
    };

    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui_zb_hall_set_loading_device(uint16_t id, bool is_loading, ui_zb_callback_result_t cb_result)
{
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_ZB_UPDATE_LOADING,
        .data.zigbee_hall = {.id = id, .result_cb = cb_result, .payload.is_loading = is_loading}
    };
    
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

/* ══════════════════════════════════════════════════════════════════
 *  Drivers (Export to API ui_drivers.h)
 * ══════════════════════════════════════════════════════════════════ */
esp_err_t ui__drivers_state_push_wifi(const char* driver_message, bool driver_ok, ui_icon_id_t driver_icon)
{
    UX_CHECK_ABSOLUTE_EXCLUSIVE_CHAR(driver_message,BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE, 0);

    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_SYS_DRIVER_WIFI,
        .data.drivers = {.driver_ok = driver_ok, .icon = driver_icon }
    };
    strlcpy(msg.data.drivers.state_message, driver_message,BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE);

    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}

esp_err_t ui__drivers_state_push_zigbee(const char* driver_message, bool driver_ok)
{
    UX_CHECK_ABSOLUTE_EXCLUSIVE_CHAR(driver_message,BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE, 0);

    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_SYS_DRIVER_WIFI,
        .data.drivers.driver_ok = driver_ok
    };
    strlcpy(msg.data.drivers.state_message, driver_message,BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE);

    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
esp_err_t ui__drivers_battery_set_status(int percentage, bool is_charging, bool driver_ok)
{
    if (percentage < 0 || percentage > 100) {
        return ESP_ERR_INVALID_ARG;
    }
    lvgl_msg_t msg = {
        .cmd = LVGL_CMD_SPC_BAT_SET_DATA,
        .data.spc_bat_set_data = { .percentage = percentage, .driver_ok = driver_ok, .is_charging = is_charging },
    };
    return lvgl_enqueue_cmd(&msg, UI_CMD_TIMEOUT_MS);
}
