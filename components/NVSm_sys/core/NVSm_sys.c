#include "NVSm_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char *TAG = "NVSm_CORE";
static SemaphoreHandle_t nvs_mutex = NULL;

esp_err_t NVSm_sys_init(void) {
    if (nvs_mutex == NULL) {
        nvs_mutex = xSemaphoreCreateMutex();
        if (nvs_mutex == NULL) {
            ESP_LOGE(TAG, "Error al crear el Mutex de NVS");
            return ESP_ERR_NO_MEM;
        }
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Partición NVS truncada, borrando e inicializando...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    
    ESP_LOGI(TAG, "NVS Inicializado correctamente");
    return err;
}

esp_err_t NVSm_sys_deinit(void) {
    if (nvs_mutex != NULL) {
        vSemaphoreDelete(nvs_mutex);
        nvs_mutex = NULL;
    }
    return nvs_flash_deinit();
}

esp_err_t NVSm_sys_lock(void) {
    if (nvs_mutex == NULL) return ESP_ERR_INVALID_STATE;
    // Espera máxima de 1000 ticks (ajustable)
    if (xSemaphoreTake(nvs_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        return ESP_OK;
    }
    ESP_LOGE(TAG, "Timeout esperando el Mutex de NVS");
    return ESP_ERR_TIMEOUT;
}

void NVSm_sys_unlock(void) {
    if (nvs_mutex != NULL) {
        xSemaphoreGive(nvs_mutex);
    }
}