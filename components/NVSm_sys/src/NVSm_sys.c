#include "NVSm_sys.h"
#include "esp_log.h"

static const char *TAG = "NVSm";
static bool s_initialized = false;

esp_err_t NVSm_sys_init(void) {
    if (s_initialized) return ESP_OK;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Partición NVS corrupta, borrando..."); //Hay que hacer un guardia para que esta acción sera resposabilidad del usuario.
        err = nvs_flash_erase();
        if (err != ESP_OK) return err;
        err = nvs_flash_init();
    }
    
    if (err == ESP_OK) {
        s_initialized = true;
        ESP_LOGI(TAG, "NVS Inicializado.");
    }
    return err;
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
