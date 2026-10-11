#include "nvsm_boot.h"

/* ESP */
#include "string.h"

/* Módulos */
#include "NVSm_sys.h"

#define NVSM_NP_BOOTING "app_bootloader"
#define NVSM_NP_BOOTING_DEFAULT_VALUE 0


#define NVSM_KEY_BOOT_FAILCOUNT "bt_failcount" 
#define NVSM_KEY_BOOT_BOOTINIT "bt_bootinit"

esp_err_t _read_blob_history_bootfail(bt_fail_history_t *boot_hist_fail)
{
    if(unlikely(boot_hist_fail == NULL)) return ESP_ERR_INVALID_ARG;

    memset(boot_hist_fail, 0, sizeof(bt_fail_history_t));

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVSM_NP_BOOTING, NVS_READWRITE, &handle);
    if (unlikely(err != ESP_OK)) return err;

    size_t required_size = sizeof(bt_fail_history_t);
    err = nvs_get_blob(handle, NVSM_KEY_BOOT_FAILCOUNT, boot_hist_fail, &required_size);
    nvs_close(handle);

    /* Camuflamos el error porque no existe el dato por un valor por default arriba,
    no debe ser indicado como falla */
    if (err == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    
    return err;
}

esp_err_t nvsm_boot_failcount_read(bt_fail_history_t *boot_hist_fail)
{
    if(unlikely(boot_hist_fail == NULL)) return ESP_ERR_INVALID_ARG;
    return _read_blob_history_bootfail(boot_hist_fail);
}

esp_err_t nvsm_boot_failcount_write(uint32_t bitmask, esp_err_t error_code)
{
    bt_fail_history_t tmp;
    esp_err_t err = _read_blob_history_bootfail(&tmp);
    if(unlikely(err != ESP_OK)) return err;
    if(tmp.total_index >= APP_LAUNCHER_MAX_ERR_BOOTING) return ESP_OK;

    uint8_t index = tmp.actually_index;

    tmp.events[index].bitmask = bitmask;
    tmp.events[index].error_code = error_code;

    tmp.total_index++;

    if(tmp.actually_index < (APP_LAUNCHER_MAX_ERR_BOOTING - 1)) tmp.actually_index++;
    
    nvs_handle_t handle;
    err = nvs_open(NVSM_NP_BOOTING, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    size_t required_size = sizeof(bt_fail_history_t);
    err = nvs_set_blob(handle, NVSM_KEY_BOOT_FAILCOUNT, &tmp, required_size);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}
esp_err_t nvsm_boot_failcount_reset(void)
{
    bt_fail_history_t tmp;
    memset(&tmp, 0, sizeof(bt_fail_history_t));

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVSM_NP_BOOTING, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(handle, NVSM_KEY_BOOT_FAILCOUNT, &tmp, sizeof(bt_fail_history_t));
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

esp_err_t nvsm_boot_bootinit_read(uint16_t *bootinit)
{
    *bootinit = NVSM_NP_BOOTING_DEFAULT_VALUE;
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVSM_NP_BOOTING, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_get_u16(handle, NVSM_KEY_BOOT_BOOTINIT, bootinit);
    nvs_close(handle);

    /* Camuflamos el error porque no existe el dato por un valor por default arriba,
    no debe ser indicado como falla */
    if (err == ESP_ERR_NVS_NOT_FOUND) return ESP_OK; 
    
    return err;
}
esp_err_t nvsm_boot_bootinit_write(uint16_t bootinit)
{
    if(bootinit > APP_LAUNCHER_MAX_BOOTS) bootinit = APP_LAUNCHER_MAX_BOOTS; // Limite de 3 intentos de boot
    
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVSM_NP_BOOTING, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_u16(handle, NVSM_KEY_BOOT_BOOTINIT, bootinit);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

esp_err_t nvsm_boot_bootinit_reset(void)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVSM_NP_BOOTING, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    err = nvs_set_u16(handle, NVSM_KEY_BOOT_BOOTINIT, NVSM_NP_BOOTING_DEFAULT_VALUE);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

