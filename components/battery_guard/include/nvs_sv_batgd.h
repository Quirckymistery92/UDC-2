/*
 * SPDX-FileCopyrightText: 2026
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t nvs_sv_batgd_save_vmin_mv(int32_t vmin_mv);
esp_err_t nvs_sv_batgd_load_vmin_mv(int32_t *out_vmin_mv);
esp_err_t nvs_sv_batgd_save_vmax_mv(int32_t vmax_mv);
esp_err_t nvs_sv_batgd_load_vmax_mv(int32_t *out_vmax_mv);

#ifdef __cplusplus
}
#endif