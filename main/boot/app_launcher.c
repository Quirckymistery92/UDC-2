#include "app_launcher.h"

#include "esp_log.h"
#include "esp_err.h"

/* Apps */
#include "app_init_normal.h"

/* Módulos */
#include "NVSm_sys.h"
#include "i2c_controller.h"
#include "fatfs_controller.h"

static uint8_t APP_RUN_RUTE = 1;

esp_err_t app_launcher_init(const char *TAG)
{
    ESP_LOGI(TAG, "Inicializando NVS...");
    ESP_ERROR_CHECK(NVSm_sys_init());

    ESP_LOGI(TAG, "Inicializando comunicaciones I2C...");
    ESP_ERROR_CHECK(i2c_controller_init());

    ESP_LOGI(TAG, "Inicializando y montando unidad FatFS...");
    ESP_ERROR_CHECK(fatfs_controller_init());

    switch(APP_RUN_RUTE) {
        case 1:
            ESP_LOGI(TAG, "Iniciando app...");
            return app_init_normal(TAG);
        default:
            ESP_LOGE(TAG, "Ruta de inicio de aplicación no válida: %d", APP_RUN_RUTE);
            return ESP_ERR_INVALID_STATE;
    }
}