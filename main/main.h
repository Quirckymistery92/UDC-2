#pragma once

/* ESP */
#include "esp_err.h"

/* Módulos */
#include "desc_apps.h"

#ifdef __cplusplus
extern "C" {
#endif

extern __attribute__((noreturn)) void terrible_fail_boot(app_desc_status_t *app);

#ifdef __cplusplus
}
#endif
