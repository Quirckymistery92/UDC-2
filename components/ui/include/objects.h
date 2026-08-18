/**
 * @file screens.h
 * @brief Objetos y pantallas de la UI.
 *
 */

#pragma once
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeración de pantallas disponibles en el sistema.
 *
 * Los valores representan las pantallas principales que puede mostrar la UI.
 */
typedef enum {
    SCREEN_MASK_NONE = (1 << 0),
    
    SCREEN_ID_DS_TOP = (1 << 1),
    SCREEN_ID_DS_LOADER = (1 << 2),
    SCREEN_ID_DS_LOCKED = (1 << 3),
    SCREEN_ID_DS_HOME = (1 << 4),
    SCREEN_ID_DS_CONFIG = (1 << 5),
    SCREEN_ID_DS_BLOCK = (1 << 6),

    SCREEN_MASK_ALL = (SCREEN_ID_DS_TOP | SCREEN_ID_DS_LOADER | SCREEN_ID_DS_LOCKED | SCREEN_ID_DS_HOME | SCREEN_ID_DS_CONFIG | SCREEN_ID_DS_BLOCK)
} ScreensEnum;

/**
 * @brief Contenedor global de objetos LVGL usados por las pantallas.
 *
 * Esta estructura agrupa los punteros `lv_obj_t*` para los widgets y
 * contenedores que forman cada screen. Se utiliza para acceder de forma
 * centralizada a los objetos creados en el módulo de pantallas.
 */
typedef struct _objects_t {
    lv_obj_t *ds_top;
    lv_obj_t *ds_loader;
    lv_obj_t *ds_locked;
    lv_obj_t *ds_home;
    lv_obj_t *ds_config;
    lv_obj_t *ds_block;
    /* Display Top*/
    lv_obj_t *dtop_notch_cont;
    lv_obj_t *dtop_notch_min_cont;
    lv_obj_t *dtop_notch_min_hour;
    lv_obj_t *dtop_notch_min_img_iot;
    lv_obj_t *dtop_notch_min_img_wifi;
    lv_obj_t *dtop_notch_min_img_bat;
    lv_obj_t *dtop_notch_exp_cont;
    lv_obj_t *dtop_notch_exp_hour;
    lv_obj_t *dtop_notch_exp_dat_cont;
    lv_obj_t *dtop_notch_exp_dat_bat_cont;
    lv_obj_t *dtop_notch_exp_dat_bat_img;
    lv_obj_t *dtop_notch_exp_dat_bat_data;
    lv_obj_t *dtop_notch_exp_dat_ubi_cont;
    lv_obj_t *dtop_notch_exp_dat_ubi_img;
    lv_obj_t *dtop_notch_exp_dat_ubi_data;
    lv_obj_t *dtop_notch_exp_dat_iot_cont;
    lv_obj_t *dtop_notch_exp_dat_iot_img;
    lv_obj_t *dtop_notch_exp_dat_iot_data;
    lv_obj_t *dtop_notch_exp_dat_wifi_cont;
    lv_obj_t *dtop_notch_exp_dat_wifi_img;
    lv_obj_t *dtop_notch_exp_dat_wifi_data;
    lv_obj_t *dtop_notch_exp_cpanel_btn;
    lv_obj_t *dtop_notch_exp_cpanel_img;
    lv_obj_t *dtop_notch_exp_cpanel_lbl;
    lv_obj_t *dtop_modal_cont;
    lv_obj_t *dtop_modal_panelctl;
    lv_obj_t *dtop_modal_panelctl_name;
    lv_obj_t *dtop_modal_panelctl_bl_cont;
    lv_obj_t *dtop_modal_panelctl_bl_name;
    lv_obj_t *dtop_modal_panelctl_bl_slider;
    lv_obj_t *dtop_modal_panelctl_bl_img;
    lv_obj_t *dtop_modal_panelctl_sound_cont;
    lv_obj_t *dtop_modal_panelctl_sound_name;
    lv_obj_t *dtop_modal_panelctl_sound_slider;
    lv_obj_t *dtop_modal_panelctl_sound_img;
    lv_obj_t *dtop_modal_panelctl_msg;
    lv_obj_t *dtop_modal_panelctl_btni_exit;
    lv_obj_t *dtop_modal_pup_cont;
    lv_obj_t *dtop_modal_pup_tit_cont;
    lv_obj_t *dtop_modal_pup_tit_img1;
    lv_obj_t *dtop_modal_pup_tit_title;
    lv_obj_t *dtop_modal_pup_tit_img2;
    lv_obj_t *dtop_modal_pup_tit_tag;
    lv_obj_t *dtop_modal_pup_tit_data;
    lv_obj_t *dtop_modal_pup_tit_btn_cont;
    lv_obj_t *dtop_modal_pup_tit_btn_cancel;
    lv_obj_t *dtop_modal_pup_tit_btn_cancel_lbl;
    lv_obj_t *dtop_modal_pup_tit_btn_run;
    lv_obj_t *dtop_modal_pup_tit_btn_run_lbl;
    lv_obj_t *dtop_notifiy_cont;
    lv_obj_t *dtop_notifiy_img;
    lv_obj_t *dtop_notifiy_message;
    /* Display Loader */
    lv_obj_t *dload_firmware_img;
    lv_obj_t *dload_firmware_data_lbl;
    /* Display Lock */
    lv_obj_t *dl_home_lock_cont;
    lv_obj_t *dl_home_lock_time;
    lv_obj_t *dl_home_lock_date;
    lv_obj_t *dl_home_lock_msguser;
    lv_obj_t *dl_home_unlock_cont_modal;
    lv_obj_t *dl_home_unlock_cont_style;
    lv_obj_t *dl_home_unlock_userpick_m_cont;
    lv_obj_t *dl_home_unlock_userpick_m_title;
    lv_obj_t *dl_home_unlock_methods_message_1;
    lv_obj_t *dl_home_unlock_userpick_cont;
    lv_obj_t *dl_home_unlock_userpick_1_cont;
    lv_obj_t *dl_home_unlock_userpick_1_img;
    lv_obj_t *dl_home_unlock_userpick_1_name;
    lv_obj_t *dl_home_unlock_userpick_1_data;
    lv_obj_t *dl_home_unlock_userpick_2_cont;
    lv_obj_t *dl_home_unlock_userpick_2_img;
    lv_obj_t *dl_home_unlock_userpick_2_name;
    lv_obj_t *dl_home_unlock_userpick_2_data;
    lv_obj_t *dl_home_unlock_userpick_3_cont;
    lv_obj_t *dl_home_unlock_userpick_3_img;
    lv_obj_t *dl_home_unlock_userpick_3_name;
    lv_obj_t *dl_home_unlock_userpick_3_data;
    lv_obj_t *dl_home_unlock_userpick_4_cont;
    lv_obj_t *dl_home_unlock_userpick_4_img;
    lv_obj_t *dl_home_unlock_userpick_4_name;
    lv_obj_t *dl_home_unlock_userpick_4_data;
    lv_obj_t *dl_home_unlock_userpick_5_cont;
    lv_obj_t *dl_home_unlock_userpick_5_img;
    lv_obj_t *dl_home_unlock_userpick_5_name;
    lv_obj_t *dl_home_unlock_userpick_5_data;
    lv_obj_t *dl_home_unlock_userpick_6_cont;
    lv_obj_t *dl_home_unlock_userpick_6_img;
    lv_obj_t *dl_home_unlock_userpick_6_name;
    lv_obj_t *dl_home_unlock_userpick_6_data;
    lv_obj_t *dl_home_unlock_methods;
    lv_obj_t *dl_home_unlock_methods_lbl;
    lv_obj_t *dl_home_unlock_methods_message;
    lv_obj_t *dl_home_unlock_methods_faceid_cont;
    lv_obj_t *dl_home_unlock_methods_faceid_line;
    lv_obj_t *dl_home_unlock_methods_faceid_const;
    lv_obj_t *dl_home_unlock_methods_faceid_const_img;
    lv_obj_t *dl_home_unlock_methods_faceid_const_lbl;
    lv_obj_t *dl_home_unlock_methods_pin_cont;
    lv_obj_t *dl_home_unlock_methods_pin_line;
    lv_obj_t *dl_home_unlock_methods_pin_const;
    lv_obj_t *dl_home_unlock_methods_pin_const_img;
    lv_obj_t *dl_home_unlock_methods_pin_const_lbl;
    lv_obj_t *dl_home_unlock_faceid_cont;
    lv_obj_t *dl_home_unlock_faceid_name;
    lv_obj_t *dl_home_unlock_faceid_img;
    lv_obj_t *dl_home_unlock_faceid_message;
    lv_obj_t *dl_home_unlock_faceid_btn_rst;
    lv_obj_t *dl_home_unlock_faceid_btn_rst_img;
    lv_obj_t *dl_home_unlock_pin_cont;
    lv_obj_t *dl_home_unlock_pin_name;
    lv_obj_t *dl_home_unlock_pin_message;
    lv_obj_t *dl_home_unlock_pin_input_cont;
    lv_obj_t *dl_home_unlock_pin_input_led1;
    lv_obj_t *dl_home_unlock_pin_input_led2;
    lv_obj_t *dl_home_unlock_pin_input_led3;
    lv_obj_t *dl_home_unlock_pin_input_led4;
    lv_obj_t *dl_home_unlock_pin_input_led5;
    lv_obj_t *dl_home_unlock_pin_input_led6;
    lv_obj_t *dl_home_unlock_pin_keyboard;
    lv_obj_t *dl_home_unlock_user_select_cont;
    lv_obj_t *dl_home_unlock_user_select_img;
    lv_obj_t *dl_home_unlock_user_select_name;
    lv_obj_t *dl_home_unlock_user_select_data;
    lv_obj_t *dl_home_unlock_btn_back;
    lv_obj_t *dl_home_unlock_btn_back_img;
    /* Display Home */
    lv_obj_t *ds_home_datetime;
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
    /* Display Block */
    lv_obj_t *ds_block_descript_cont;
    lv_obj_t *ds_block_descript_img;
    lv_obj_t *ds_block_descript_data_lbl;
    lv_obj_t *ds_block_pninfo_cont;
    lv_obj_t *ds_block_pninfo_img;
    lv_obj_t *ds_block_pninfo_text_head;
    lv_obj_t *ds_block_pninfo_text_body;
    lv_obj_t *ds_block_pn_qr_text_head;
    lv_obj_t *ds_block_pn_qr_text_body;
    lv_obj_t *ds_block_pn_qr_qrcode;
    lv_obj_t *ds_block_pn_assist_cont;
    lv_obj_t *ds_block_pn_assist_img;
    lv_obj_t *ds_block_pn_assist_data;
} objects_t;

/**
 * @brief Instancia global de los objetos LVGL de la UI.
 *
 * Las pantallas y widgets del sistema se referencian a través de esta
 * variable desde otros módulos de UI.
 */
extern objects_t objects;

#ifdef __cplusplus
}
#endif
