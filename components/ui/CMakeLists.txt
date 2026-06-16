set(UI_SRCS_CORE
    "core/ui.c"
    "core/ui_manager.c"
    "core/ui_label_abs.c"
    "core/styles.c"
)

set(UI_SRCS_FONTS
    "fonts/ui_font_anton80.c"
    "fonts/ui_font_mulish12.c"
    "fonts/ui_font_mulish14.c"
    "fonts/ui_font_mulish16.c"
    "fonts/ui_font_mulish18.c"
    "fonts/ui_font_mulish20.c"
    "fonts/ui_font_mulish30.c"
    "fonts/ui_font_mulish_bold14.c"
    "fonts/ui_font_mulish_bold16.c"
    "fonts/ui_font_mulish_bold18.c"
    "fonts/ui_font_mulish_bold20.c"
    "fonts/ui_font_mulish_bold24.c"
)

set(UI_SRCS_IMAGES
    "images/ui_image_arrow_back30.c"
    "images/ui_image_arrow_restart30.c"
    "images/ui_image_arrow_top40.c"
    "images/ui_image_bat_charging30.c"
    "images/ui_image_bat_low30.c"
    "images/ui_image_bat_max30.c"
    "images/ui_image_bat_med30.c"
    "images/ui_image_bat_ultra_low30.c"
    "images/ui_image_brightness30.c"
    "images/ui_image_control_panel20.c"
    "images/ui_image_darrow_top30.c"
    "images/ui_image_eye_detected30.c"
    "images/ui_image_eye_normal30.c"
    "images/ui_image_eye_off30.c"
    "images/ui_image_face_id40.c"
    "images/ui_image_face_id50.c"
    "images/ui_image_information30.c"
    "images/ui_image_iot_active30.c"
    "images/ui_image_location30.c"
    "images/ui_image_menudot_v30.c"
    "images/ui_image_pin50.c"
    "images/ui_image_sia.c"
    "images/ui_image_sound30.c"
    "images/ui_image_warning30.c"
    "images/ui_image_wifi_off30.c"
    "images/ui_image_wifi_on30.c"
    "images/ui_image_user_logo_default50.c"
    "images/ui_image_user_logo_baby50.c"
    "images/ui_image_user_logo_child50.c"
    "images/ui_image_user_logo_diamond50.c"
    "images/ui_image_user_logo_dog50.c"
    "images/ui_image_user_logo_father50.c"
    "images/ui_image_user_logo_girl50.c"
    "images/ui_image_user_logo_granny50.c"
    "images/ui_image_user_logo_guest50.c"
    "images/ui_image_user_logo_guy50.c"
    "images/ui_image_user_logo_happy50.c"
    "images/ui_image_user_logo_math50.c"
    "images/ui_image_user_logo_mom50.c"
    "images/ui_image_user_logo_nature50.c"
    "images/ui_image_user_logo_normal50.c"
    "images/ui_image_user_logo_owl50.c"
    "images/ui_image_user_logo_pawprint50.c"
    "images/ui_image_user_logo_rat50.c"
    "images/ui_image_user_logo_robot50.c"
    "images/ui_image_user_logo_sad50.c"
    "images/ui_image_user_logo_shocked50.c"
    "images/ui_image_user_logo_very_happy50.c"
    "images/ui_image_user_logo_worried50.c"
)

set(UI_SRCS_PRIVATE
    "private/lv_mem_port.c"
    "private/porting/porting.c"
)

set(UI_SRCS_SCREENS
    "screens/screens.c"
    
    "screens/actions/actions_display_top.c"
    "screens/actions/control/actions_control.c"
    "screens/actions/actions_display_locked.c"
    "screens/actions/actions_display_home.c"
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
        "core"
        "fonts"
        "images"
        "screens"
    PRIV_INCLUDE_DIRS
        "private"
        "private/porting"
        "screens/actions"
        "screens/actions/control"
    REQUIRES
        "lvgl"
        "i2c_controller"
        "esp_driver_gpio"
        "main"
    PRIV_REQUIRES
        "esp_lcd_jd9365"
        "esp_lcd_touch_gsl3680"
        "esp_lcd_touch"
        "driver"
        "esp_lcd"
        "esp_hw_support"
        "esp_timer"
        "esp_driver_ppa"
        "jpegc_element"
        "esp_mm"
        "sys_modules"
)

# Force the linker to pull lv_mem_port.c.obj from libui.a before processing liblvgl__lvgl.a.
# Without this, lv_malloc_core/lv_free_core/lv_realloc_core/lv_mem_init stay unresolved because
# all occurrences of libui.a appear before the second pass of liblvgl__lvgl.a in the link order.
target_link_libraries(${COMPONENT_LIB} INTERFACE
    "-Wl,-u,lv_malloc_core"
    "-Wl,-u,lv_free_core"
    "-Wl,-u,lv_realloc_core"
    "-Wl,-u,lv_mem_init"
)



