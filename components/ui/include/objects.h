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
    lv_obj_t *display_top;
    lv_obj_t *display_loader;
    lv_obj_t *display_lock;
    lv_obj_t *display_home;
    lv_obj_t *display_settings;
    lv_obj_t *display_block;
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
    lv_obj_t *dtop_modal_panelctl_bl_slider;
    lv_obj_t *dtop_modal_panelctl_sound_slider;
    lv_obj_t *dtop_modal_panelctl_btni_exit;
    lv_obj_t *dtop_modal_pup_cont;
    lv_obj_t *dtop_modal_pup_tit_img1;
    lv_obj_t *dtop_modal_pup_tit_img2;
    lv_obj_t *dtop_modal_pup_tit_title;
    lv_obj_t *dtop_modal_pup_tit_tag;
    lv_obj_t *dtop_modal_pup_tit_data;
    lv_obj_t *dtop_modal_pup_tit_btn_run;
    lv_obj_t *dtop_modal_pup_tit_btn_run_lbl;
    lv_obj_t *dtop_modal_pup_tit_btn_cancel;
    lv_obj_t *dtop_modal_pup_tit_btn_cancel_lbl;
    lv_obj_t *dtop_notifiy_cont;
    lv_obj_t *dtop_notifiy_img;
    lv_obj_t *dtop_notifiy_message;
    lv_obj_t *dload_firmware_img;
    lv_obj_t *dload_firmware_data_lbl;
    lv_obj_t *dlock_home_lock_cont;
    lv_obj_t *dlock_home_lock_time;
    lv_obj_t *dlock_home_lock_date;
    lv_obj_t *dlock_home_lock_msguser;
    lv_obj_t *dlock_unlock_cont_modal;
    lv_obj_t *dlock_unlock_userpick_m_cont;
    lv_obj_t *dlock_unlock_userpick_cont;
    lv_obj_t *dlock_unlock_userpick_1_cont;
    lv_obj_t *dlock_unlock_userpick_1_img;
    lv_obj_t *dlock_unlock_userpick_1_name;
    lv_obj_t *dlock_unlock_userpick_1_data;
    lv_obj_t *dlock_unlock_userpick_2_cont;
    lv_obj_t *dlock_unlock_userpick_2_img;
    lv_obj_t *dlock_unlock_userpick_2_name;
    lv_obj_t *dlock_unlock_userpick_2_data;
    lv_obj_t *dlock_unlock_userpick_3_cont;
    lv_obj_t *dlock_unlock_userpick_3_img;
    lv_obj_t *dlock_unlock_userpick_3_name;
    lv_obj_t *dlock_unlock_userpick_3_data;
    lv_obj_t *dlock_unlock_userpick_4_cont;
    lv_obj_t *dlock_unlock_userpick_4_img;
    lv_obj_t *dlock_unlock_userpick_4_name;
    lv_obj_t *dlock_unlock_userpick_4_data;
    lv_obj_t *dlock_unlock_userpick_5_cont;
    lv_obj_t *dlock_unlock_userpick_5_img;
    lv_obj_t *dlock_unlock_userpick_5_name;
    lv_obj_t *dlock_unlock_userpick_5_data;
    lv_obj_t *dlock_unlock_userpick_6_cont;
    lv_obj_t *dlock_unlock_userpick_6_img;
    lv_obj_t *dlock_unlock_userpick_6_name;
    lv_obj_t *dlock_unlock_userpick_6_data;
    lv_obj_t *dlock_unlock_methods_cont;
    lv_obj_t *dlock_unlock_methods_message;
    lv_obj_t *dlock_unlock_methods_faceid_cont;
    lv_obj_t *dlock_unlock_methods_pin_cont;
    lv_obj_t *dlock_unlock_faceid_cont;
    lv_obj_t *dlock_unlock_faceid_message;
    lv_obj_t *dlock_unlock_faceid_btn_rst;
    lv_obj_t *dlock_unlock_pin_cont;
    lv_obj_t *dlock_unlock_pin_message;
    lv_obj_t *dlock_unlock_pin_input_led1;
    lv_obj_t *dlock_unlock_pin_input_led2;
    lv_obj_t *dlock_unlock_pin_input_led3;
    lv_obj_t *dlock_unlock_pin_input_led4;
    lv_obj_t *dlock_unlock_pin_input_led5;
    lv_obj_t *dlock_unlock_pin_input_led6;
    lv_obj_t *dlock_unlock_pin_keyboard;
    lv_obj_t *dlock_unlock_user_select_cont;
    lv_obj_t *dlock_unlock_user_select_img;
    lv_obj_t *dlock_unlock_user_select_name;
    lv_obj_t *dlock_unlock_user_select_data;
    lv_obj_t *dlock_unlock_btn_back;
    lv_obj_t *dhome_datetime;
    lv_obj_t *dhome_user_login_cont;
    lv_obj_t *dhome_user_login_img;
    lv_obj_t *dhome_user_login_name;
    lv_obj_t *dhome_act_cont;
    lv_obj_t *dhome_act_home_cont;
    lv_obj_t *dhome_act_home_weather;
    lv_obj_t *dhome_act_home_wt_dat_stateday;
    lv_obj_t *dhome_act_home_wt_dat_ubname;
    lv_obj_t *dhome_act_home_wt_dat_img;
    lv_obj_t *dhome_act_home_wt_dat_stermic;
    lv_obj_t *dhome_act_home_wt_dat_grad_c;
    lv_obj_t *dhome_act_home_wt_dat_humidity_p;
    lv_obj_t *dhome_act_home_wt_dat_winds_kmh;
    lv_obj_t *dhome_act_home_wt_dat_uv_state;
    lv_obj_t *dhome_act_home_wt_d1;
    lv_obj_t *dhome_act_home_wt_d1_name;
    lv_obj_t *dhome_act_home_wt_d1_img;
    lv_obj_t *dhome_act_home_wt_d1_dat;
    lv_obj_t *dhome_act_home_wt_d2;
    lv_obj_t *dhome_act_home_wt_d2_name;
    lv_obj_t *dhome_act_home_wt_d2_img;
    lv_obj_t *dhome_act_home_wt_d2_data;
    lv_obj_t *dhome_act_home_wt_d3;
    lv_obj_t *dhome_act_home_wt_d3_name;
    lv_obj_t *dhome_act_home_wt_d3_img;
    lv_obj_t *dhome_act_home_wt_d3_data;
    lv_obj_t *dhome_act_home_scenes;
    lv_obj_t *dhome_act_home_zigbee;
    lv_obj_t *dhome_act_home_settings;
    lv_obj_t *dhome_act_home_syst;
    lv_obj_t *dhome_act_home_syst_cpu;
    lv_obj_t *dhome_act_home_syst_intram;
    lv_obj_t *dhome_act_home_syst_psram;
    lv_obj_t *dhome_act_iot_cont;
    lv_obj_t *dhome_act_iot_grid;
    lv_obj_t *dhome_act_dashboard_cont;
    lv_obj_t *dhome_act_dbd_label_inactive;
    lv_obj_t *dhome_act_dbd_dat_cont;
    lv_obj_t *dhome_act_dbd_dat_arc_health;
    lv_obj_t *dhome_act_dbd_dat_arc_fails;
    lv_obj_t *dhome_act_dbd_dat_st_health_lbl;
    lv_obj_t *dhome_act_dbd_dat_lqi_bar;
    lv_obj_t *dhome_act_dbd_dat_lqi_label;
    lv_obj_t *dhome_act_dbd_dat_ondevs_bar;
    lv_obj_t *dhome_act_dbd_dat_ondevs_lbl;
    lv_obj_t *dhome_act_dbd_dat_rssi_bar;
    lv_obj_t *dhome_act_dbd_dat_traffic_chart;
    lv_obj_t *dhome_nav_cont;
    lv_obj_t *dhome_nav_btn_home;
    lv_obj_t *dhome_nav_btn_iot;
    lv_obj_t *dhome_nav_btn_dbd;
    lv_obj_t *dhome_qc_modal;
    lv_obj_t *dhome_qc_panel;
    lv_obj_t *dhome_qc_p_st_led;
    lv_obj_t *dhome_qc_p_st_lbl_statedev;
    lv_obj_t *dhome_qc_p_st_img;
    lv_obj_t *dhome_qc_p_st_lbl_namedev;
    lv_obj_t *dhome_qc_p_act_dim_cont;
    lv_obj_t *dhome_qc_p_act_dim_in_sld;
    lv_obj_t *dhome_qc_p_act_cpr_cont;
    lv_obj_t *dhome_qc_p_act_cpr_hsl;
    lv_obj_t *dhome_qc_p_btn_gosettingsdev;
    lv_obj_t *dhome_auser_modal;
    lv_obj_t *dhome_auser_panel;
    lv_obj_t *dhome_auser_p_btn_logout;
    lv_obj_t *dhome_auser_p_btn_lock;
    lv_obj_t *dhome_auser_p_btn_restart;
    lv_obj_t *dhome_auser_p_btn_poweroff;
    lv_obj_t *dblock_state_dev_cont;
    lv_obj_t *dblock_state_dev_img;
    lv_obj_t *dblock_state_dev_label;
    lv_obj_t *dblock_pn_qr_qrcode;
    lv_obj_t *dblock_pn_assist_data;
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
