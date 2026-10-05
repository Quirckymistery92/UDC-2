set(UI_SRCS_CORE
    "app/core/src/ui.c"
    "app/core/src/ui_manager.c"
    "app/core/src/ui_label_abs.c"
    "app/core/src/styles.c"
)

set(UI_SRCS_FONTS
    "app/fonts/src/ui_font_anton80.c"
    "app/fonts/src/ui_font_anton22.c"
    "app/fonts/src/ui_font_mulish12.c"
    "app/fonts/src/ui_font_mulish14.c"
    "app/fonts/src/ui_font_mulish16.c"
    "app/fonts/src/ui_font_mulish18.c"
    "app/fonts/src/ui_font_mulish20.c"
    "app/fonts/src/ui_font_mulish30.c"
    "app/fonts/src/ui_font_mulish40.c"
    "app/fonts/src/ui_font_mulish_bold12.c"
    "app/fonts/src/ui_font_mulish_bold14.c"
    "app/fonts/src/ui_font_mulish_bold16.c"
    "app/fonts/src/ui_font_mulish_bold18.c"
    "app/fonts/src/ui_font_mulish_bold20.c"
    "app/fonts/src/ui_font_mulish_bold24.c"
    "app/fonts/src/ui_font_mulish_bold26.c"
    "app/fonts/src/ui_font_outfit26.c"
)

set(UI_SRCS_IMAGES
    "app/images/control/ui_image_sia.c"

    "app/images/src/ui_image_arrow_back30.c"
    "app/images/src/ui_image_arrow_restart30.c"
    "app/images/src/ui_image_arrow_top40.c"
    "app/images/src/ui_image_bat_charging30.c"
    "app/images/src/ui_image_bat_low30.c"
    "app/images/src/ui_image_bat_max30.c"
    "app/images/src/ui_image_bat_med30.c"
    "app/images/src/ui_image_bat_ultra_low30.c"
    "app/images/src/ui_image_brightness30.c"
    "app/images/src/ui_image_control_panel20.c"
    "app/images/src/ui_image_darrow_top30.c"
    "app/images/src/ui_image_eye_detected30.c"
    "app/images/src/ui_image_eye_normal30.c"
    "app/images/src/ui_image_eye_off30.c"
    "app/images/src/ui_image_face_id40.c"
    "app/images/src/ui_image_face_id50.c"
    "app/images/src/ui_image_dialog_info20.c"
    "app/images/src/ui_image_location30.c"
    "app/images/src/ui_image_pin50.c"
    "app/images/src/ui_image_sound30.c"
    "app/images/src/ui_image_dialog_warn20.c"
    "app/images/src/ui_image_wifi_off30.c"
    "app/images/src/ui_image_wifi_on30.c"
    "app/images/src/ui_image_user_logo_default50.c"
    "app/images/src/ui_image_user_logo_baby50.c"
    "app/images/src/ui_image_user_logo_child50.c"
    "app/images/src/ui_image_user_logo_diamond50.c"
    "app/images/src/ui_image_user_logo_dog50.c"
    "app/images/src/ui_image_user_logo_father50.c"
    "app/images/src/ui_image_user_logo_girl50.c"
    "app/images/src/ui_image_user_logo_granny50.c"
    "app/images/src/ui_image_user_logo_guest50.c"
    "app/images/src/ui_image_user_logo_guy50.c"
    "app/images/src/ui_image_user_logo_happy50.c"
    "app/images/src/ui_image_user_logo_math50.c"
    "app/images/src/ui_image_user_logo_mom50.c"
    "app/images/src/ui_image_user_logo_nature50.c"
    "app/images/src/ui_image_user_logo_normal50.c"
    "app/images/src/ui_image_user_logo_owl50.c"
    "app/images/src/ui_image_user_logo_pawprint50.c"
    "app/images/src/ui_image_user_logo_rat50.c"
    "app/images/src/ui_image_user_logo_robot50.c"
    "app/images/src/ui_image_user_logo_sad50.c"
    "app/images/src/ui_image_user_logo_shocked50.c"
    "app/images/src/ui_image_user_logo_very_happy50.c"
    "app/images/src/ui_image_user_logo_worried50.c"
    "app/images/src/ui_image_device_locked.c"
    "app/images/src/ui_image_firmware_logo.c"
    "app/images/src/ui_image_information_only230x280.c"
    "app/images/src/ui_image_support_assistant120.c"
    "app/images/src/ui_image_ambient_light90.c"
    "app/images/src/ui_image_hardware_chip30.c"
    "app/images/src/ui_image_hardware_ram_ext30.c"
    "app/images/src/ui_image_hardware_ram30.c"
    "app/images/src/ui_image_light_bulb_error90.c"
    "app/images/src/ui_image_light_bulb_off90.c"
    "app/images/src/ui_image_light_bulb_on90.c"
    "app/images/src/ui_image_switch_off90.c"
    "app/images/src/ui_image_switch_error90.c"
    "app/images/src/ui_image_scenes_autom80.c"
    "app/images/src/ui_image_settings80.c"
    "app/images/src/ui_image_switch_on90.c"
    "app/images/src/ui_image_temperature90.c"
    "app/images/src/ui_image_wt_humidity15.c"
    "app/images/src/ui_image_wt_pcloudy_day70.c"
    "app/images/src/ui_image_wt_rain30.c"
    "app/images/src/ui_image_wt_winds15.c"
    "app/images/src/ui_image_zigbee_icon75.c"
    "app/images/src/ui_image_zigbee_icon30.c"
    "app/images/src/ui_image_action_lock60.c"
    "app/images/src/ui_image_action_logout60.c"
    "app/images/src/ui_image_action_poweroff60.c"
    "app/images/src/ui_image_action_restart60.c"
    "app/images/src/ui_image_dimmer_light40.c"
    "app/images/src/ui_image_goto_output20.c"
    "app/images/src/ui_image_palet_edit40.c"
)

set(UI_SRCS_PRIVATE
    "app/private/porting/porting.c"
)

set(UI_SRCS_SCREENS
    "app/interface/objects.c"
    "app/interface/zigbee_hall/ui_zb_controller.c"

    "app/interface/screens/modules/actions_control.c"
    "app/interface/screens/modules/clock_control.c"

    "app/interface/screens/ux_block/ux_block.c"

    "app/interface/screens/ux_config/ux_config.c"

    "app/interface/screens/ux_home/ux_home.c"
    "app/interface/screens/ux_home/act_home.c"

    "app/interface/screens/ux_loader/ux_loader.c"
    "app/interface/screens/ux_loader/act_loader.c"

    "app/interface/screens/ux_locked/ux_locked.c"
    "app/interface/screens/ux_locked/act_locked.c"

    "app/interface/screens/ux_top/ux_top.c"
    "app/interface/screens/ux_top/act_top.c"
)

set(UI_SRCS
    ${UI_SRCS_CORE}
    ${UI_SRCS_FONTS}
    ${UI_SRCS_IMAGES}
    ${UI_SRCS_PRIVATE}
    ${UI_SRCS_SCREENS}
)

idf_component_register(
    SRCS
        ${UI_SRCS}
    INCLUDE_DIRS
        "include"
    PRIV_INCLUDE_DIRS
        "app/core/include"
        "app/fonts/include"
        "app/images/include"
        "app/interface/include"
        "app/interface/zigbee_hall/include"
    REQUIRES
        "lvgl"
    PRIV_REQUIRES
        "esp_lcd_jd9365"
        "esp_lcd_touch_gsl3680"
        "esp_lcd_touch"
        "driver"
        "esp_lcd"
        "esp_hw_support"
        "esp_timer"
        "esp_driver_ppa"
        "udc_metrics"
        "jpegc_element"
        "esp_mm"
        "user_management"
        "rtc_time"
        "esp_driver_gpio"
        "i2c_controller"
        "main"
)





