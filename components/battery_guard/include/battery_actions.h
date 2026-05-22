/*
 * SPDX-FileCopyrightText: 2026
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa y arranca el sistema de guardia de batería.
 */
esp_err_t battery_actions_init(void);

/**
 * @brief Detiene la tarea y libera recursos del sistema de batería.
 */
esp_err_t battery_actions_stop(void);

#ifdef __cplusplus
}
#endif
