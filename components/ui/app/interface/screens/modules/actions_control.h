#pragma once

/* ESP */
#include <stdbool.h>
#include "esp_err.h"

/* Módulos */
#include "../screens_int_all.h"
#include "ui_image_sia_internal.h"

esp_err_t ui_int_sys_actions_init(void);

esp_err_t ui_pub_actions_backg_update_disp_home(const char *image_path);
esp_err_t ui_pub_actions_backg_update_disp_lock(const char *image_path);

typedef void (*ui_int__dtop_popup_callback_t)(void);

typedef enum {
    dtopPOPUP_INFO = 1,
    dtopPOPUP_WARN = 2,
    dtop_POPUP_MAX_TYPE
} ui_int__dtop_popup_type_t;

typedef enum {
    dtopPOPUP_BTN_OK_AND_EXT = 0,
    dtopPOPUP_BTN_RST_AND_EXT = 1,
    dtop_POPUP_BTN_ONLY_ACPT = 2,
    dtop_POPUP_MAX_BTN,
} ui_int__dtop_popup_btn_type_t;

typedef struct {
    uint8_t type;
    uint8_t btn_type;
    ui_int__dtop_popup_callback_t btn_run;
    const char *tag;
    const char *message;
} ui_int__dtop_popup_configure_t;

esp_err_t ui_int__dtop_show_popup(ui_int__dtop_popup_configure_t config);
esp_err_t ui_int__dtop_notify_user_show(bool is_alert, const char *text);
esp_err_t ui_int__dtop_notify_user_show_fmt(bool is_alert, const char *fmt, ...);

esp_err_t ui_int__dtop_zigbee_state(const char* state_message, bool driver_ok);
esp_err_t ui_int__dtop_wifi_state(const char* state_message, bool driver_ok, ui_icon_id_t icon);
esp_err_t ui_int__dtop_battery_state(uint16_t bat_percent, bool is_charging, bool driver_ok);