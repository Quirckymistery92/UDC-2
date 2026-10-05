#pragma once

/* ESP */
#include "esp_err.h"

/* Módulos */
#include "desc_apps.h"

#ifdef __cplusplus
extern "C" {
#endif

app_desc_status_t* bat_g_boot_normal(const char * TAG);

#ifdef __cplusplus
}
#endif