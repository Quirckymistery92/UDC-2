#include "udc_time.h"
#include "rtc_rx8025t.h"
#include "esp_err.h"
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

static const char * const s_tz_table[] = {
    "UTC0",                             // 0: Default
    "America/Argentina/Buenos_Aires",   // 1: Argentina
    "America/Santiago",                 // 2: Chile
    "America/Sao_Paulo",                // 3: Brasil
};
#define TZ_TABLE_SIZE (sizeof(s_tz_table) / sizeof(s_tz_table[0]))

static uint8_t s_current_tz_index = 0;

static time_t timegm_utc(const struct tm *tm)
{
    if (tm == NULL) return (time_t)-1;

    int year = tm->tm_year + 1900;
    int mon = tm->tm_mon + 1;
    int mday = tm->tm_mday;

    if (mon <= 2) {
        year -= 1;
        mon += 12;
    }

    int64_t y = year;
    int64_t months = mon;
    int64_t days = 365 * (y - 1970) + (y - 1969) / 4 - (y - 1901) / 100 + (y - 1601) / 400;
    days += (153 * (months - 3) + 2) / 5 + (mday - 1);
    int64_t secs = days * 86400 + tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;

    return (time_t)secs;
}

// --- Inicialización y Configuración ---

esp_err_t udc_time_set_timezone(uint8_t tz_index)
{
    if (tz_index >= TZ_TABLE_SIZE) {
        tz_index = 0; // Fallback seguro a UTC
    }
    
    if(setenv("TZ", s_tz_table[tz_index], 1) != 0) {
        return ESP_ERR_NOT_SUPPORTED;
    }
    tzset();
    s_current_tz_index = tz_index;

    return ESP_OK;
}

esp_err_t udc_time_init(uint8_t tz_index)
{
    return udc_time_set_timezone(tz_index);
}

// --- RELOJ INTERNO ---

esp_err_t udc_time_get_int_time_t(time_t *out_time)
{
    if(out_time == NULL) return ESP_ERR_INVALID_ARG;
    *out_time = time(NULL);
    return ESP_OK;
}

esp_err_t udc_time_get_int_struct_tm(struct tm *out_time)
{
    if(out_time == NULL) return ESP_ERR_INVALID_ARG;
    time_t now = time(NULL);
    // localtime_r aplica el offset de la zona configurada en setenv
    if(localtime_r(&now, out_time) == NULL) return ESP_FAIL;
    return ESP_OK;
}

esp_err_t udc_time_set_int_time_t(time_t set_time)
{
    if(set_time == (time_t)-1) return ESP_ERR_INVALID_ARG;
    struct timeval tv = { .tv_sec = set_time, .tv_usec = 0 };
    if (settimeofday(&tv, NULL) != 0) return ESP_FAIL;
    return ESP_OK;
}

esp_err_t udc_time_set_int_struct_tm(const struct tm *set_time)
{
    if(set_time == NULL) return ESP_ERR_INVALID_ARG;
    struct tm temp_set_tm = *set_time;
    // mktime asume que temp_set_tm está en hora local, 
    // y lo convierte a epoch UTC basado en el setenv actual.
    time_t timestamp = mktime(&temp_set_tm);
    if(timestamp == (time_t)-1) return ESP_ERR_INVALID_STATE;
    return udc_time_set_int_time_t(timestamp);
}

// --- RELOJ EXTERNO RTC ---

esp_err_t udc_time_get_ext_time_t(time_t *out_time)
{
    if(out_time == NULL) return ESP_ERR_INVALID_ARG;
    struct tm strc_time;
    esp_err_t ret = rtc_rx8025t_get_time(&strc_time);
    if(ret != ESP_OK) return ret;

    // El RTC nos da UTC puro, lo pasamos a epoch sin tocar TZ
    time_t tdf_time = timegm_utc(&strc_time);
    if(tdf_time == (time_t)-1) return ESP_FAIL;

    *out_time = tdf_time;
    return ESP_OK;
}

esp_err_t udc_time_get_ext_struct_tm(struct tm *out_time)
{
    if(out_time == NULL) return ESP_ERR_INVALID_ARG;
    // Retorna lo que tiene el RTC tal cual (UTC puro)
    return rtc_rx8025t_get_time(out_time);
}

esp_err_t udc_time_set_ext_time_t(time_t set_time) 
{
    if(set_time == (time_t)-1) return ESP_ERR_INVALID_ARG;
    struct tm rtc_tm;
    // Convertimos de epoch a struct tm en UTC puro
    if (gmtime_r(&set_time, &rtc_tm) == NULL) return ESP_FAIL;
    return rtc_rx8025t_set_time(&rtc_tm);
}

esp_err_t udc_time_set_ext_struct_tm(const struct tm *set_time)
{
    if (set_time == NULL) return ESP_ERR_INVALID_ARG;
    // Esperamos que set_time ya venga en UTC puro
    return rtc_rx8025t_set_time((struct tm *)set_time);
}

// --- SINCRONIZACIÓN ---

esp_err_t udc_time_sync_system_from_rtc(void)
{
    if (!rtc_rx8025t_is_valid()) return ESP_ERR_INVALID_STATE;

    return rtc_rx8025t_force_sync();
}