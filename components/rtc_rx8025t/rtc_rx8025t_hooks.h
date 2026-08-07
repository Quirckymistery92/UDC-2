/**
 * @file rtc_rx8025t_hooks.h
 * @brief RTC hooks for system integration.
 *
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * rtc_rx8025t_hooks — Header privado interno
 *
 * Declara los hooks de actualización llamados desde rtc_rx8025t.c.
 * NO incluir fuera del componente rtc_rx8025t.
 * La implementación vive en rtc_rx8025t_hooks.c.
 */

#pragma once

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Llamado cada 1 segundo. */
void rtc_rx8025t_hooks_tick_1s(const struct tm *t);

/** @brief Llamado solo cuando cambia el minuto o la hora. */
void rtc_rx8025t_hooks_tick_min_hour(const struct tm *t);

/** @brief Llamado solo cuando cambia el día. */
void rtc_rx8025t_hooks_tick_date(const struct tm *t);

#ifdef __cplusplus
}
#endif