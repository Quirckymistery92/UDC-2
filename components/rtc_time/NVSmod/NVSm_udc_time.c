#include "NVSm_udc_time.h"
#include "NVSm_sys.h"

#define NVS_NAMESPACE "udc_time"
#define NVS_KEY_TZ    "tz_idx"

esp_err_t NVSm_udc_time_save_tz_index(uint8_t tz_index)
{
    if (!NVSm_sys_is_initialized()) return ESP_ERR_INVALID_STATE;

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    err = nvs_set_u8(my_handle, NVS_KEY_TZ, tz_index);
    if (err == ESP_OK) {
        err = nvs_commit(my_handle);
    }
    
    nvs_close(my_handle);
    return err;
}

esp_err_t NVSm_udc_time_load_tz_index(uint8_t *tz_index)
{
    if (tz_index == NULL) return ESP_ERR_INVALID_ARG;
    if (!NVSm_sys_is_initialized()) return ESP_ERR_INVALID_STATE;

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        // Si no existe el namespace (primera vez), devolvemos fallback seguro (0)
        *tz_index = 0; 
        return ESP_ERR_NOT_FOUND;
    }

    err = nvs_get_u8(my_handle, NVS_KEY_TZ, tz_index);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        *tz_index = 0; // Valor por defecto si la llave no existe
    }

    nvs_close(my_handle);
    return err; // Puede devolver ESP_ERR_NVS_NOT_FOUND, el llamador decide si es crítico
}