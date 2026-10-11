#pragma once

/* ESP */

/* Módulos */
#include "NVSm_sys.h"
#include "app_launcher.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t bitmask;
    esp_err_t error_code;
} bt_fail_history_event_t;

typedef struct {
    bt_fail_history_event_t events[APP_LAUNCHER_MAX_ERR_BOOTING];
    uint8_t actually_index;
    uint8_t total_index;
} bt_fail_history_t;

esp_err_t nvsm_boot_failcount_read(bt_fail_history_t *boot_hist_fail);
esp_err_t nvsm_boot_failcount_write(uint32_t bitmask, esp_err_t error_code);
esp_err_t nvsm_boot_failcount_reset(void);

esp_err_t nvsm_boot_bootinit_read(uint16_t *bootinit);
esp_err_t nvsm_boot_bootinit_write(uint16_t bootinit);
esp_err_t nvsm_boot_bootinit_reset(void);


esp_err_t nvsm_boot_historico_read(bt_fail_history_t *historial);

#ifdef __cplusplus
}
#endif