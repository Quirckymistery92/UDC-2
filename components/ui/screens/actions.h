#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t actions_sys_backg_init(void);
esp_err_t actions_sys_backg_update(const char *image_path);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/