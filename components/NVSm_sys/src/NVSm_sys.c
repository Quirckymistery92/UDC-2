#include "NVSm_sys.h"

/* ESP */
#include "esp_log.h"
#include "esp_check.h"

/* Variables */
    #define TAG "NVS_Manager"
    static bool s_initialized = false;
    
    static app_desc_status_t apd = {
        .app_desc = DSC_APP_STORAGE_FLASH_NVS,
        .app_name = "Controlador NVS",
        .app_tag = TAG,
        .init_result = ESP_OK
    };

app_desc_status_t* NVSm_sys_init(void) {

    DSC_RETURN_STRUCT_ON_FALSE(!s_initialized, &apd, ESP_OK);

    esp_err_t err = nvs_flash_init();
    DSC_RETURN_STRUCT_ON_FALSE(err == ESP_OK, &apd, ESP_OK);
    
    if (err == ESP_OK)  s_initialized = true;
    
    return DESC_AP_INSERT(&apd, err);
}

esp_err_t NVSm_sys_hard_reset(void)
{
    if (s_initialized) return ESP_ERR_INVALID_STATE;

    return nvs_flash_erase();
}

esp_err_t NVSm_sys_deinit(void) {
    if (!s_initialized) return ESP_OK;

    esp_err_t err = nvs_flash_deinit();

    if (err == ESP_OK) s_initialized = false;
    return err;
}

bool NVSm_sys_is_initialized(void) {
    return s_initialized;
}
