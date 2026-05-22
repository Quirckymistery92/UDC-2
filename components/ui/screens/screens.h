#pragma once
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_DS_HOME = 1,
    _SCREEN_ID_LAST = 1
};

typedef struct _objects_t {
    lv_obj_t *ds_home;
    lv_obj_t *notch_canvas_cont;
    lv_obj_t *notch_icon_iot;
    lv_obj_t *notch_icon_wifi;
    lv_obj_t *notch_icon_battery;
    lv_obj_t *notch_hour;
    lv_obj_t *ds_home_users_cont;
    lv_obj_t *ds_home_users_image;
    lv_obj_t *ds_home_users_name;
    lv_obj_t *ds_home_users_message;
    lv_obj_t *ds_home_actm_cont;
    lv_obj_t *ds_home_actm_extra_menudot;
    lv_obj_t *ds_home_actm_sys_cont;
    lv_obj_t *ds_home_actm_sys_weather;
    lv_obj_t *ds_home_actm_sys_weather_lb;
    lv_obj_t *ds_home_actm_sys_ia;
    lv_obj_t *ds_home_actm_sys_ia_lb;
    lv_obj_t *ds_home_actm_sys_zigbee_state;
    lv_obj_t *ds_home_actm_sys_zigbee_state_lb;
    lv_obj_t *ds_home_actm_sys_settings;
    lv_obj_t *ds_home_actm_sys_settings_lb;
    lv_obj_t *ds_home_actm_sys_state;
    lv_obj_t *ds_home_actm_sys_state_lb;
    lv_obj_t *ds_home_actm_sys_security;
    lv_obj_t *ds_home_actm_sys_security_lb;
    lv_obj_t *ds_home_actm_extra_iot;
    lv_obj_t *ds_home_actm_iot_cont;
    lv_obj_t *ds_home_actm_iot_dev0;
    lv_obj_t *ds_home_actm_iot_dev0_lb;
    lv_obj_t *ds_home_actm_iot_dev1;
    lv_obj_t *ds_home_actm_iot_dev1_lb;
    lv_obj_t *ds_home_actm_iot_dev2;
    lv_obj_t *ds_home_actm_iot_dev2_lb;
    lv_obj_t *ds_home_actm_iot_dev3;
    lv_obj_t *ds_home_actm_iot_dev3_lb;
    lv_obj_t *ds_home_actm_iot_dev4;
    lv_obj_t *ds_home_actm_iot_dev4_lb;
    lv_obj_t *ds_home_actm_iot_dev5;
    lv_obj_t *ds_home_actm_iot_dev5_lb;
    lv_obj_t *ds_home_actm_iot_dev6;
    lv_obj_t *ds_home_actm_iot_dev6_lb;
    lv_obj_t *ds_home_actm_iot_dev7;
    lv_obj_t *ds_home_actm_iot_dev7_lb;
    lv_obj_t *ds_home_actm_iot_dev8;
    lv_obj_t *ds_home_actm_iot_dev8_lb;
    lv_obj_t *ds_home_actm_iot_dev9;
    lv_obj_t *ds_home_actm_iot_dev9_lb;
    lv_obj_t *ds_home_actm_iot_dev10;
    lv_obj_t *ds_home_actm_iot_dev10_lb;
    lv_obj_t *ds_home_actm_iot_dev11;
    lv_obj_t *ds_home_actm_iot_dev11_lb;
    lv_obj_t *ds_home_actm_iot_dev12;
    lv_obj_t *ds_home_actm_iot_dev12_lb;
    lv_obj_t *ds_home_actm_iot_dev13;
    lv_obj_t *ds_home_actm_iot_dev13_lb;
    lv_obj_t *ds_home_actm_iot_dev14;
    lv_obj_t *ds_home_actm_iot_dev14_lb;
    lv_obj_t *ds_home_actm_iot_dev15;
    lv_obj_t *ds_home_actm_iot_dev15_lb;
    lv_obj_t *ds_home_actm_iot_dev16;
    lv_obj_t *ds_home_actm_iot_dev16_lb;
    lv_obj_t *ds_home_actm_iot_dev17;
    lv_obj_t *ds_home_actm_iot_dev17_lb;
    lv_obj_t *ds_home_actm_iot_dev18;
    lv_obj_t *ds_home_actm_iot_dev18_lb;
    lv_obj_t *ds_home_actm_iot_dev19;
    lv_obj_t *ds_home_actm_iot_dev19_lb;
    lv_obj_t *ds_home_nav_cont;
    lv_obj_t *ds_home_nav_btn_home;
    lv_obj_t *ds_home_nav_btn_home_lbl;
    lv_obj_t *ds_home_nav_btn_iot;
    lv_obj_t *ds_home_nav_btn_iot_lbl;
    lv_obj_t *ds_home_nav_btn_dashb;
    lv_obj_t *ds_home_nav_btn_dashb_lbl;
    lv_obj_t *ds_home_nav_btn_extra;
    lv_obj_t *ds_home_nav_btn_extra_lbl;
} objects_t;

extern objects_t objects;

void ui_create_screen_ds_home(void);
void lvgl_tick_screen_ds_home();

void lvgl_tick_screen_by_id(enum ScreensEnum screenId);
void lvgl_tick_screen_by_int(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif