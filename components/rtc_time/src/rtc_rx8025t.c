/*
 * SPDX-FileCopyrightText: 2026
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rtc_rx8025t.h"
#include "i2c_controller.h"

#include <string.h>
#include <sys/time.h>

#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "RTC";

#define RTC_I2C_ADDR        (0x32)
#define RTC_I2C_CLK_HZ      (400000)
#define RTC_I2C_TIMEOUT_MS  (50)

#define REG_SECONDS         (0x00)
#define REG_MINUTES         (0x01)
#define REG_HOURS           (0x02)
#define REG_WEEKDAY         (0x03)
#define REG_DATE            (0x04)
#define REG_MONTH           (0x05)
#define REG_YEAR            (0x06)
#define REG_EXT             (0x0D)

#define REG_CTRL1           (0x0E) 
#define REG_CTRL2           (0x0F) 

#define BIT_VDET            (1 << 0) 
#define BIT_VLF             (1 << 1) 
#define BIT_XSTP            (1 << 5) 
#define CTRL_TEMP_COMP_2S   (0x40)   

#define SYNC_CMD_EXIT       (1 << 0)
#define SYNC_CMD_FORCE      (1 << 1)

static i2c_master_dev_handle_t s_dev         = NULL;
static SemaphoreHandle_t       s_mutex       = NULL;
static TaskHandle_t            s_task_handle = NULL;
static rtc_rx8025t_cb_t        s_user_cb     = NULL;

static inline uint8_t bcd2dec(uint8_t b) { return (b >> 4) * 10 + (b & 0x0F); }
static inline uint8_t dec2bcd(uint8_t d) { return ((d / 10) << 4) | (d % 10); }

static inline uint8_t tm_wday_to_rtc(int wday) { return (uint8_t)(1 << wday); }
static inline int rtc_wday_to_tm(uint8_t b) {
    for (int i = 0; i < 7; i++) {
        if (b & (1 << i)) return i;
    }
    return 0;
}

static esp_err_t reg_read(uint8_t addr, uint8_t *out, size_t len) {
    return i2c_master_transmit_receive(s_dev, &addr, 1, out, len, RTC_I2C_TIMEOUT_MS);
}

static esp_err_t reg_write(uint8_t addr, const uint8_t *data, size_t len) {
    uint8_t buf[1 + len];
    buf[0] = addr;
    memcpy(&buf[1], data, len);
    return i2c_master_transmit(s_dev, buf, sizeof(buf), RTC_I2C_TIMEOUT_MS);
}

static esp_err_t read_time_raw(struct tm *t) {
    uint8_t raw[7];
    ESP_RETURN_ON_ERROR(reg_read(REG_SECONDS, raw, sizeof(raw)), TAG, "Lectura I2C falló");

    t->tm_sec   = bcd2dec(raw[0] & 0x7F);
    t->tm_min   = bcd2dec(raw[1] & 0x7F);
    t->tm_hour  = bcd2dec(raw[2] & 0x3F);
    t->tm_wday  = rtc_wday_to_tm(raw[3]);
    t->tm_mday  = bcd2dec(raw[4] & 0x3F);
    t->tm_mon   = bcd2dec(raw[5] & 0x1F) - 1;
    t->tm_year  = bcd2dec(raw[6]) + 100; 
    t->tm_isdst = -1;
    return ESP_OK;
}

static void rtc_sync_task(void *arg) {
    uint32_t interval_ms = (uint32_t)arg * 1000;
    uint32_t notified_val = 0;

    while (1) {
        BaseType_t res = xTaskNotifyWait(0, ULONG_MAX, &notified_val, pdMS_TO_TICKS(interval_ms));

        if (res == pdPASS && (notified_val & SYNC_CMD_EXIT)) {
            break; 
        }

        if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
            uint8_t ctrls[2] = {0};
            
            // Lectura en ráfaga (2 bytes de un solo golpe)
            if (reg_read(REG_CTRL1, ctrls, 2) != ESP_OK) {
                if (s_user_cb) s_user_cb(RTC_RX8025T_EVENT_I2C_ERROR, NULL);
            } 
            else if ((ctrls[0] & BIT_XSTP) || (ctrls[1] & BIT_VLF)) {
                if (s_user_cb) s_user_cb(RTC_RX8025T_EVENT_POWER_FAIL, NULL);
            } 
            else {
                struct tm t;
                if (read_time_raw(&t) == ESP_OK) {
                    struct timeval tv = { .tv_sec = mktime(&t), .tv_usec = 0 };
                    settimeofday(&tv, NULL);
                    
                    if (s_user_cb) s_user_cb(RTC_RX8025T_EVENT_SYNC_OK, &t);
                }
            }
            xSemaphoreGive(s_mutex);
        }
    }

    s_task_handle = NULL;
    vTaskDelete(NULL);
}

esp_err_t rtc_rx8025t_init(uint32_t sync_interval_sec, rtc_rx8025t_cb_t cb) {
    if (s_mutex == NULL) {
        s_mutex = xSemaphoreCreateMutex();
        ESP_RETURN_ON_FALSE(s_mutex, ESP_ERR_NO_MEM, TAG, "No se pudo crear Mutex");
    }

    s_user_cb = cb;

    ESP_RETURN_ON_ERROR(i2c_controller_add_device(RTC_I2C_ADDR, RTC_I2C_CLK_HZ, &s_dev),
                        TAG, "Error registrando dispositivo I2C");

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    
    uint8_t ctrls[2] = {0};

    if (reg_read(REG_CTRL1, ctrls, 2) == ESP_OK) {

      bool rtc_valid = !(ctrls[0] & BIT_XSTP) && !(ctrls[1] & BIT_VLF);

      if (rtc_valid) {
        struct tm t;
        if (read_time_raw(&t) == ESP_OK) {
          struct timeval tv = {.tv_sec = mktime(&t), .tv_usec = 0};
          settimeofday(&tv, NULL);
        }
      } else {
        ESP_LOGW(TAG,
                 "RTC inválido: pérdida de energía u oscilación detenida.");
      }
    }

    xSemaphoreGive(s_mutex);

    if (sync_interval_sec > 0 && s_task_handle == NULL) {
        xTaskCreate(rtc_sync_task, "rtc_sync", 3072, (void *)sync_interval_sec, 5, &s_task_handle);
    }

    ESP_LOGI(TAG, "Driver RX8025T inicializado correctamente con mapeo fijo.");
    return ESP_OK;
}

esp_err_t rtc_rx8025t_force_sync(void) {
    if (s_task_handle) {
        xTaskNotify(s_task_handle, SYNC_CMD_FORCE, eSetBits);
        return ESP_OK;
    }
    return ESP_ERR_INVALID_STATE;
}

esp_err_t rtc_rx8025t_get_time(struct tm *t) {
    ESP_RETURN_ON_FALSE(t != NULL, ESP_ERR_INVALID_ARG, TAG, "Puntero nulo");
    ESP_RETURN_ON_FALSE(s_mutex != NULL, ESP_ERR_INVALID_STATE, TAG, "Driver no inicializado");

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    uint8_t ctrls[2] = {0};
    
    esp_err_t err = reg_read(REG_CTRL1, ctrls, 2);
    
    if (err == ESP_OK && !(ctrls[0] & BIT_XSTP) && !(ctrls[1] & BIT_VLF)) {
        err = read_time_raw(t);
    } else if (err == ESP_OK) {
        err = ESP_FAIL; // El bus leyó bien, pero la data es inválida por hardware
    }
    xSemaphoreGive(s_mutex);
    return err;
}

esp_err_t rtc_rx8025t_set_time(const struct tm *t) {
    ESP_RETURN_ON_FALSE(t != NULL, ESP_ERR_INVALID_ARG, TAG, "Puntero nulo");
    ESP_RETURN_ON_FALSE(s_mutex != NULL, ESP_ERR_INVALID_STATE, TAG, "Driver no inicializado");
    
    if (t->tm_year < 100) {
        ESP_LOGE(TAG, "Año inválido proporcionado. Debe ser igual o mayor al año 2000.");
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t raw[7];
    raw[0] = dec2bcd((uint8_t)t->tm_sec);
    raw[1] = dec2bcd((uint8_t)t->tm_min);
    raw[2] = dec2bcd((uint8_t)t->tm_hour);
    raw[3] = tm_wday_to_rtc(t->tm_wday);
    raw[4] = dec2bcd((uint8_t)t->tm_mday);
    raw[5] = dec2bcd((uint8_t)(t->tm_mon + 1));
    raw[6] = dec2bcd((uint8_t)(t->tm_year - 100));

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    
    esp_err_t err = reg_write(REG_SECONDS, raw, sizeof(raw));
    if (err == ESP_OK) {
        uint8_t ctrls[2] = {0};
        
        // Leemos los controles en 1 transacción
        if (reg_read(REG_CTRL1, ctrls, 2) == ESP_OK) {
            // Limpiamos las banderas
            ctrls[0] &= ~BIT_XSTP;
            ctrls[1] &= ~BIT_VLF;
            
            // Escribimos los controles limpios en 1 transacción atómica
            reg_write(REG_CTRL1, ctrls, 2);
        }

        struct tm local_tm = *t;
        struct timeval tv = { .tv_sec = mktime(&local_tm), .tv_usec = 0 };
        settimeofday(&tv, NULL);
    }
    
    xSemaphoreGive(s_mutex);
    return err;
}

bool rtc_rx8025t_is_valid(void) {
    if (s_mutex == NULL) return false;
    
    uint8_t ctrls[2] = {0};
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    esp_err_t err = reg_read(REG_CTRL1, ctrls, 2);
    xSemaphoreGive(s_mutex);

    return (err == ESP_OK && !(ctrls[0] & BIT_XSTP) && !(ctrls[1] & BIT_VLF));
}

esp_err_t rtc_rx8025t_deinit(void) {
    if (s_task_handle != NULL) {
        xTaskNotify(s_task_handle, SYNC_CMD_EXIT, eSetBits);
        
        int timeout_ms = 500;
        while (s_task_handle != NULL && timeout_ms > 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
            timeout_ms -= 10;
        }
        
        if (s_task_handle != NULL) {
            ESP_LOGE(TAG, "Falló la desinicialización del RTC-RX8025T, Por falta de respuesta de la tarea interna.");
            return ESP_ERR_TIMEOUT; 
        }
    }

    if (s_dev) {
        i2c_controller_remove_device(s_dev);
        s_dev = NULL;
    }
    
    if (s_mutex) {
        vSemaphoreDelete(s_mutex);
        s_mutex = NULL;
    }
    
    s_user_cb = NULL;
    
    return ESP_OK;
}