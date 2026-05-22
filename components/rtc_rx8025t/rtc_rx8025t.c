/*
 * SPDX-FileCopyrightText: 2025
 * SPDX-License-Identifier: Apache-2.0
 *
 * RX8025T RTC driver - ESP-IDF 5.x implementation.
 *
 * I2C transfers use the new master API (i2c_master_transmit /
 * i2c_master_transmit_receive). The device handle is static and private.
 * An esp_timer fires every RTC_SYNC_PERIOD_US to re-sync the ESP system
 * clock from the RTC, correcting RTOS tick drift over time.
 *
 * Status handling on every read / sync:
 *   VLF=1  -> LOGE + ESP_FAIL  (power failure, time data lost)
 *   VDET=1 -> LOGW + ESP_OK    (low battery, time still valid)
 */

#include "rtc_rx8025t.h"
#include "rtc_rx8025t_hooks.h"   /* hooks de aplicación — privado */
#include "i2c_controller.h"

#include <string.h>
#include <sys/time.h>

#include "esp_log.h"
#include "esp_check.h"
#include "esp_timer.h"

static const char *TAG = "rtc_rx8025t";

/* ── I2C ─────────────────────────────────────────────────────────── */
#define RTC_I2C_ADDR        (0x32)
#define RTC_I2C_CLK_HZ      (400000)
#define RTC_I2C_TIMEOUT_MS  (50)

/* ── Register map ────────────────────────────────────────────────── */
#define REG_SECONDS         (0x00)
#define REG_MINUTES         (0x01)
#define REG_HOURS           (0x02)
#define REG_WEEKDAY         (0x03)
#define REG_DATE            (0x04)
#define REG_MONTH           (0x05)
#define REG_YEAR            (0x06)
#define REG_EXT             (0x0D)
#define REG_STATUS          (0x0E)
#define REG_CONTROL         (0x0F)

/* ── STATUS register bits ────────────────────────────────────────── */
#define BIT_VDET            (1 << 0)   /* low battery, compensation affected  */
#define BIT_VLF             (1 << 1)   /* voltage loss, time data invalid      */

/* ── CONTROL register defaults ───────────────────────────────────── */
#define CTRL_TEMP_COMP_2S   (0x40)     /* temperature compensation every 2 s  */

/* ── Periodic sync period ────────────────────────────────────────── */
#define RTC_SYNC_PERIOD_US  (1000ULL * 1000)          /* 1 second  */

/* ── Private state ───────────────────────────────────────────────── */
static i2c_master_dev_handle_t s_dev    = NULL;
static esp_timer_handle_t      s_timer  = NULL;

/* ═══════════════════════════ Helpers ════════════════════════════════ */

static inline uint8_t bcd2dec(uint8_t b) { return (b >> 4) * 10 + (b & 0x0F); }
static inline uint8_t dec2bcd(uint8_t d) { return ((d / 10) << 4) | (d % 10); }

/* RX8025T weekday: bit-encoded (bit N = day N, Sunday=bit0).
 * struct tm tm_wday: 0=Sunday … 6=Saturday.                         */
static inline uint8_t tm_wday_to_rtc(int wday) { return (uint8_t)(1 << wday); }
static inline int     rtc_wday_to_tm(uint8_t b)
{
    for (int i = 0; i < 7; i++) {
        if (b & (1 << i)) return i;
    }
    return 0;
}

/* ── Low-level register I/O ─────────────────────────────────────── */

static esp_err_t reg_read(uint8_t addr, uint8_t *out, size_t len)
{
    return i2c_master_transmit_receive(s_dev, &addr, 1,
                                       out, len, RTC_I2C_TIMEOUT_MS);
}

static esp_err_t reg_write(uint8_t addr, const uint8_t *data, size_t len)
{
    uint8_t buf[1 + len];
    buf[0] = addr;
    memcpy(&buf[1], data, len);
    return i2c_master_transmit(s_dev, buf, sizeof(buf), RTC_I2C_TIMEOUT_MS);
}

static esp_err_t reg_write_byte(uint8_t addr, uint8_t val)
{
    return reg_write(addr, &val, 1);
}

/* ── Status evaluation (shared by init, get, timer) ─────────────── */

static esp_err_t check_status(uint8_t status)
{
    if (status & BIT_VLF) {
        ESP_LOGE(TAG, "Time data lost - power failure detected (VLF=1)");
        return ESP_FAIL;
    }
    if (status & BIT_VDET) {
        ESP_LOGW(TAG, "Low battery voltage detected (VDET=1) - temp compensation affected");
    }
    return ESP_OK;
}

/* ── Internal raw read (no status check) ────────────────────────── */

static esp_err_t read_time_raw(struct tm *t)
{
    uint8_t raw[7];
    ESP_RETURN_ON_ERROR(reg_read(REG_SECONDS, raw, sizeof(raw)),
                        TAG, "I2C read failed");

    t->tm_sec   = bcd2dec(raw[0] & 0x7F);
    t->tm_min   = bcd2dec(raw[1] & 0x7F);
    t->tm_hour  = bcd2dec(raw[2] & 0x3F);
    t->tm_wday  = rtc_wday_to_tm(raw[3]);
    t->tm_mday  = bcd2dec(raw[4] & 0x3F);
    t->tm_mon   = bcd2dec(raw[5] & 0x1F) - 1;   /* RTC: 1-12, tm: 0-11 */
    t->tm_year  = bcd2dec(raw[6]) + 100;          /* RTC: 0-99(2000), tm: years since 1900 */
    t->tm_isdst = -1;
    return ESP_OK;
}

/* ── System clock sync ───────────────────────────────────────────── */

static void sync_system_time(void)
{
    struct tm t;
    if (read_time_raw(&t) != ESP_OK) {
        ESP_LOGE(TAG, "Sync failed: I2C read error");
        return;
    }

    /* Actualizar reloj del sistema — corrección de drift del tick RTOS */
    struct timeval tv = { .tv_sec = mktime(&t), .tv_usec = 0 };
    settimeofday(&tv, NULL);

    rtc_rx8025t_hooks_tick_1s(&t);
    rtc_rx8025t_hooks_tick_min_hour(&t);
    rtc_rx8025t_hooks_tick_date(&t);

    /*ESP_LOGI(TAG, "System time synced from RTC: %02d:%02d:%02d  %02d/%02d/%04d",
             t.tm_hour, t.tm_min, t.tm_sec,
             t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);*/
}

/* ── Timer callback ──────────────────────────────────────────────── */

static void sync_timer_cb(void *arg)
{
    uint8_t status = 0;
    if (reg_read(REG_STATUS, &status, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Sync: STATUS read failed");
        return;
    }

    /* Reuse the same check_status logic as get_time — emits identical logs */
    if (check_status(status) != ESP_OK) {
        return;   /* VLF: LOGE already emitted by check_status, abort sync */
    }

    sync_system_time();   /* logs HH:MM:SS  DD/MM/YYYY on success */
}

/* ═══════════════════════════ Public API ═════════════════════════════ */

esp_err_t rtc_rx8025t_init(void)
{
    /* Register on shared I2C bus */
    ESP_RETURN_ON_ERROR(
        i2c_controller_add_device(RTC_I2C_ADDR, RTC_I2C_CLK_HZ, &s_dev),
        TAG, "Failed to add device on I2C bus");

    /* Read STATUS to evaluate initial state */
    uint8_t status = 0;
    ESP_RETURN_ON_ERROR(reg_read(REG_STATUS, &status, 1),
                        TAG, "Initial STATUS read failed");

    /* Hard reset if VLF or VDET is set */
    if (status & (BIT_VLF | BIT_VDET)) {
        uint8_t rst = 0x01;   /* RESET bit */
        reg_write_byte(REG_CONTROL, rst);
    }

    /* Clear all flags in EXT and STATUS, apply default control */
    reg_write_byte(REG_EXT,     0x00);
    reg_write_byte(REG_STATUS,  0x00);
    reg_write_byte(REG_CONTROL, CTRL_TEMP_COMP_2S);

    /* Re-read STATUS after clear */
    reg_read(REG_STATUS, &status, 1);

    esp_err_t validity = check_status(status);
    if (validity == ESP_FAIL) {
        /* VLF: time is invalid, do not sync system clock */
        ESP_LOGE(TAG, "RTC init: time not synced (power failure history)");
        /* Still create timer so future set_time + syncs will work */
    } else {
        sync_system_time();
    }

    /* Create periodic sync timer */
    esp_timer_create_args_t timer_args = {
        .callback        = sync_timer_cb,
        .arg             = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name            = "rtc_sync",
    };
    ESP_RETURN_ON_ERROR(esp_timer_create(&timer_args, &s_timer),
                        TAG, "Timer create failed");
    ESP_RETURN_ON_ERROR(esp_timer_start_periodic(s_timer, RTC_SYNC_PERIOD_US),
                        TAG, "Timer start failed");

    ESP_LOGI(TAG, "RX8025T initialized (sync every 5 min)");
    return validity;   /* ESP_OK or ESP_FAIL depending on VLF */
}

esp_err_t rtc_rx8025t_get_time(struct tm *t)
{
    ESP_RETURN_ON_FALSE(t != NULL, ESP_ERR_INVALID_ARG, TAG, "Null pointer");

    uint8_t status = 0;
    ESP_RETURN_ON_ERROR(reg_read(REG_STATUS, &status, 1), TAG, "STATUS read failed");
    ESP_RETURN_ON_ERROR(check_status(status), TAG, "Time not reliable");

    return read_time_raw(t);
}

esp_err_t rtc_rx8025t_set_time(const struct tm *t)
{
    ESP_RETURN_ON_FALSE(t != NULL, ESP_ERR_INVALID_ARG, TAG, "Null pointer");

    uint8_t raw[7];
    raw[0] = dec2bcd((uint8_t)t->tm_sec);
    raw[1] = dec2bcd((uint8_t)t->tm_min);
    raw[2] = dec2bcd((uint8_t)t->tm_hour);
    raw[3] = tm_wday_to_rtc(t->tm_wday);
    raw[4] = dec2bcd((uint8_t)t->tm_mday);
    raw[5] = dec2bcd((uint8_t)(t->tm_mon + 1));
    raw[6] = dec2bcd((uint8_t)(t->tm_year - 100));   /* store 2-digit year */

    ESP_RETURN_ON_ERROR(reg_write(REG_SECONDS, raw, sizeof(raw)),
                        TAG, "Time write failed");

    /* Clear VLF after a successful set so is_valid() returns true */
    uint8_t status = 0;
    reg_read(REG_STATUS, &status, 1);
    status &= ~BIT_VLF;
    reg_write_byte(REG_STATUS, status);

    ESP_LOGI(TAG, "RTC time set: %04d-%02d-%02d %02d:%02d:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    return ESP_OK;
}

bool rtc_rx8025t_is_valid(void)
{
    uint8_t status = 0;
    if (reg_read(REG_STATUS, &status, 1) != ESP_OK) {
        return false;
    }
    return !(status & BIT_VLF);
}

esp_err_t rtc_rx8025t_deinit(void)
{
    if (s_timer != NULL) {
        esp_timer_stop(s_timer);
        esp_timer_delete(s_timer);
        s_timer = NULL;
    }
    if (s_dev != NULL) {
        ESP_RETURN_ON_ERROR(i2c_controller_remove_device(s_dev),
                            TAG, "Remove device failed");
        s_dev = NULL;
    }
    ESP_LOGI(TAG, "RX8025T deinitialized");
    return ESP_OK;
}
