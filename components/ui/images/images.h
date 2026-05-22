#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_bat_charging;
extern const lv_img_dsc_t img_bar_home_smile;
extern const lv_img_dsc_t img_bat_ultra_low;
extern const lv_img_dsc_t img_bat_low;
extern const lv_img_dsc_t img_bat_med;
extern const lv_img_dsc_t img_bat_max;
extern const lv_img_dsc_t img_iot_active;
extern const lv_img_dsc_t img_wifi_on;
extern const lv_img_dsc_t img_wifi_off;
extern const lv_img_dsc_t img_testing;
extern const lv_img_dsc_t img_mini_plug_local20;
extern const lv_img_dsc_t img_mini_iot_wireless20;
extern const lv_img_dsc_t img_iot_face_id;
extern const lv_img_dsc_t img_utils_app_timer;
extern const lv_img_dsc_t img_utils_ai_activity;
extern const lv_img_dsc_t img_user_logo;
extern const lv_img_dsc_t img_menudot_v;

#define Simg_bg_thunder_rail  "S:bg_thunder_rail.jpg"

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[17];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/