#pragma once

/* ESP */
#include "esp_err.h"
#include "stdbool.h"

/* Módulos */
#include "desc_apps.h"

#define APP_LAUNCHER_MAX_BOOTS 3
#define APP_LAUNCHER_MAX_ERR_BOOTING 3

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t app_launcher_init(void);

void app_launcher_reg_nd_log(app_desc_status_t *app);

#ifdef __cplusplus
}
#endif