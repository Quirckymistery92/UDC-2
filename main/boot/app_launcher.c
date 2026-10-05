#include "app_launcher.h"

/* ESP */
#include "esp_log.h"
#include "esp_err.h"

/* Apps */
#include "app_init_normal.h"

/* Módulos */
#include "NVSm_sys.h"
#include "desc_apps.h"
#include "../main.h"
#include "nvsm_boot.h"

/* Variables */
    #define TAG "AppLauncher"
    static uint16_t mask_boot = 0;
    static uint16_t mask_failed = 0;
    enum {
        BOOT_INIT_NORMAL = 0,
        BOOT_INIT_OFFENSIVE = 1,
        BOOT_INIT_FLASH_RECOVERY = 2,
        BOOT_INIT_RECOVERY = 3,
    };

esp_err_t app_launcher_init(void)
{
    app_desc_status_t * app = NVSm_sys_init();
    app_launcher_reg_nd_log(app);
    DSC_RETURN_ERR_ON_STRUCT(app);

    bt_fail_history_t boot_hist_fail;
    esp_err_t err = nvsm_boot_failcount_read(&boot_hist_fail);
    if(likely(err == ESP_OK)){
        if(unlikely(boot_hist_fail.actually_index >= APP_LAUNCHER_MAX_ERR_BOOTING)) {
            /* Run boot recovery */
        }
    }

    esp_err_t ret = ESP_OK;
    uint16_t bootinit = 0;
    ret = nvsm_boot_bootinit_read(&bootinit);
    if(unlikely(ret != ESP_OK)) ESP_LOGW(TAG, "Error de lectura de boot_init, usando default");

    switch(bootinit) {
        case BOOT_INIT_NORMAL:

            ESP_LOGW(TAG, "Boot normal");
            ret = app_init_normal(TAG);
            if(unlikely(ret != ESP_OK)){
                ESP_LOGE(TAG, "Error de inicialización de apps: %s", esp_err_to_name(ret));
                nvsm_boot_failcount_write(mask_failed, ret);
                return ESP_FAIL;
            }
            break;
            
        case BOOT_INIT_OFFENSIVE:
            ESP_LOGW(TAG, "Boot ofensivo");
            break;
        case BOOT_INIT_FLASH_RECOVERY:
            ESP_LOGW(TAG, "Boot recovery flash");
            break;
        case BOOT_INIT_RECOVERY:
            ESP_LOGW(TAG, "Boot recovery");
            break;
        default:
            ESP_LOGW(TAG, "Boot desconocido: %d", bootinit);
            return ESP_FAIL;
    }

    return ret;
}

void app_launcher_reg_nd_log(app_desc_status_t *app)
{
    if(unlikely(app == NULL)) return;

    if(app->init_result != ESP_OK) {
        ESP_LOGE(app->app_tag, "%s. RP---> (%s)", app->app_name, esp_err_to_name(app->init_result));
    } else {
        ESP_LOGI(app->app_tag, "%s. RP---> (%s)", app->app_name, esp_err_to_name(app->init_result));
    }
    if(app->init_result == ESP_OK) {
        mask_boot |= app->app_desc;
    }else {
        mask_failed |= app->app_desc;
    }
}