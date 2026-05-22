set(UI_SRCS_CORE
    "core/ui.c"
    "core/ui_manager.c"
    "core/ui_label_abs.c"
    "core/styles.c"
)

set(UI_SRCS_FONTS
    "fonts/ui_font_mulish14.c"
    "fonts/ui_font_mulish16.c"
    "fonts/ui_font_mulish18.c"
    "fonts/ui_font_mulish20.c"
    "fonts/ui_font_mulish30.c"
    "fonts/ui_font_mulish_bold14.c"
    "fonts/ui_font_mulish_bold16.c"
    "fonts/ui_font_mulish_bold18.c"
    "fonts/ui_font_mulish_bold20.c"
)

set(UI_SRCS_IMAGES
    "images/images.c"
    "images/ui_image_sia.c"
    "images/ui_image_bat_charging.c"
    "images/ui_image_bat_low.c"
    "images/ui_image_bat_max.c"
    "images/ui_image_bat_med.c"
    "images/ui_image_bat_ultra_low.c"
    "images/ui_image_iot_active.c"
    "images/ui_image_iot_face_id.c"
    "images/ui_image_menudot_v.c"
    "images/ui_image_mini_iot_wireless20.c"
    "images/ui_image_mini_plug_local20.c"
    "images/ui_image_testing.c"
    "images/ui_image_user_logo.c"
    "images/ui_image_utils_ai_activity.c"
    "images/ui_image_utils_app_timer.c"
    "images/ui_image_wifi_off.c"
    "images/ui_image_wifi_on.c"
)

set(UI_SRCS_PRIVATE
    "private/lv_mem_port.c"
    "private/porting/porting.c"
)

set(UI_SRCS_SCREENS
    "screens/screens.c"
    "screens/actions.c"
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



