/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * rtc_rx8025t_hooks — Implementación de hooks de aplicación
 *
 * Contiene toda la lógica de software que reacciona a los eventos
 * del RTC: detección de cambio de minuto/hora y actualización de UI.
 * Este archivo es el único que debe incluir ui_manager.h en el componente.
 *
 * Para agregar una actualización por segundo: on_tick_1s()
 * Para agregar una actualización por cambio M/H: on_tick_min_hour()
 */

#include "rtc_rx8025t_hooks.h"
#include "ui_manager.h"

static int s_last_min  = -1;
static int s_last_hour = -1;
static int s_last_mday = -1;

/* ── Hook: cada 1 segundo ───────────────────────────────────────── */

static void on_tick_seconds(const struct tm *t)
{
    (void)t;
}

/* ── Hook: cambio de minuto y/o hora ────────────────────────────── */

static void on_tick_min_hour(const struct tm *t)
{
    ui_label_set_text_abs(UI_LABEL_ABS_NOTCH_HOUR, "%02d:%02d", t->tm_hour, t->tm_min);
}

/* ── Hook: cambio de día ────────────────────────────────────────── */

static void on_tick_date(const struct tm *t)
{
    (void)t;
}

/* ── Puntos de entrada desde rtc_rx8025t.c ──────────────────────── */

void rtc_rx8025t_hooks_tick_1s(const struct tm *t)
{
    on_tick_seconds(t);
}

void rtc_rx8025t_hooks_tick_min_hour(const struct tm *t)
{
    if (t->tm_min != s_last_min || t->tm_hour != s_last_hour) {
        s_last_min  = t->tm_min;
        s_last_hour = t->tm_hour;
        on_tick_min_hour(t);
    }
}

void rtc_rx8025t_hooks_tick_date(const struct tm *t)
{
    if (t->tm_mday != s_last_mday) {
        s_last_mday = t->tm_mday;
        on_tick_date(t);
    }
}
