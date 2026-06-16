#pragma once

#include "ui.h"
#include "actions_display_top.h"
#include "actions_display_locked.h"
#include "actions_display_home.h"
#include "esp_err.h"

esp_err_t ui_int_sys_actions_init(void);

esp_err_t ui_pub_actions_backg_update(const char *image_path, lv_obj_t *target_obj);
