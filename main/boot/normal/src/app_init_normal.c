#include "app_init_normal.h"

/* ESP */
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

/* Build Info */
#include "build_timestamp.h"

/* Módulos */
#include "ui_manager.h"
#include "jpegc_element.h"
#include "rtc_rx8025t.h"
#include "user_manager.h"
#include "battery.h"
#include "display_backlight.h"
#include "udc_metrics.h"
#include "i2c_controller.h"
#include "littlefs_controller.h"
#include "desc_apps.h"
#include "app_launcher.h"

esp_err_t app_init_normal(const char *TAG)
{
    app_desc_status_t * app;
    esp_err_t err = ESP_OK;

    app = i2c_controller_init();
    app_launcher_reg_nd_log(app);
    DSC_RETURN_ERR_ON_STRUCT(app);

    app = littlefs_controller_init();
    app_launcher_reg_nd_log(app);
    DSC_RETURN_ERR_ON_STRUCT(app);
    
    /* Inicializamos display Backlight */
    app = display_lcd_brightness_init();
    DSC_RETURN_ERR_ON_STRUCT(app);
    
    setenv("TZ", "ART3", 1);
    tzset();

    /* Controlador de reloj RTC externo */
    app = rtc_rx8025t_init(3600, NULL);
    app_launcher_reg_nd_log(app);
    if(app->init_result == ESP_OK){
        if(!rtc_rx8025t_is_valid()) {
            const struct tm datetime = {
                .tm_sec = (BUILD_SEC >= 0 && BUILD_SEC <= 60) ? BUILD_SEC : 0, // segundos
                .tm_min = (BUILD_MIN >= 0 && BUILD_MIN <= 59) ? BUILD_MIN : 0, // minutos
                .tm_hour = (BUILD_HOUR >= 0 && BUILD_HOUR <= 23) ? BUILD_HOUR : 0, // horas
                .tm_mday = (BUILD_MDAY >= 1 && BUILD_MDAY <= 31) ? BUILD_MDAY : 1, // día del més
                .tm_mon = (BUILD_MON >= 0 && BUILD_MON <= 11) ? BUILD_MON : 0, // mes
                .tm_year = (BUILD_YEAR >= 0) ? BUILD_YEAR : 100, // año
                .tm_wday = (BUILD_WDAY >= 0 && BUILD_WDAY <= 6) ? BUILD_WDAY : 0, // día de la semana (sabado, domingo, martes...)
                .tm_yday = (BUILD_YDAY >= 0 && BUILD_YDAY <= 365) ? BUILD_YDAY : 0, // día del año (0 = 1 de enero.)
                .tm_isdst = -1
            };
            rtc_rx8025t_set_time(&datetime);

            ESP_LOGW(TAG, "RTC datetime default: sec=%d, min=%d, hour=%d, mday=%d, mon=%d, year=%d, wday=%d, yday=%d",
                    datetime.tm_sec, datetime.tm_min, datetime.tm_hour, datetime.tm_mday,
                    datetime.tm_mon, datetime.tm_year, datetime.tm_wday, datetime.tm_yday);
        }
    }

    app = jpegc_element_init();
    app_launcher_reg_nd_log(app);
    DSC_RETURN_ERR_ON_STRUCT(app);

    app = udc_metrics_init();
    app_launcher_reg_nd_log(app);
    DSC_RETURN_ERR_ON_STRUCT(app);
    
    app = ui_init();
    app_launcher_reg_nd_log(app);
    DSC_RETURN_ERR_ON_STRUCT(app);

    err = ui_screen_loader(SCREEN_MASK_ALL, SCREEN_ID_DS_LOADER); /* Enviamos a LVGL las pantallas a cargar. (Petición SRC_LOAD) ASYNC */
    ui_label_set_text_abs(UI_LABEL_ABS_LOADER_DATA, "%s", "Iniciando sistema...");
    display_lcd_backlight_on(); /* Encendemos Backlight */

    ui__helper_sync_sys_screens_get_semaphore(); /* Esperamos que SRC_LOAD apruebe y libere el semaphore. */
    
    app = user_manager_init();
    app_launcher_reg_nd_log(app);
    DSC_RETURN_ERR_ON_STRUCT(app);

    /* Se mueve a top cuando se construya un lector de bat a nivel I2C */
    app = bat_g_boot_normal(TAG);
    app_launcher_reg_nd_log(app);
    DSC_RETURN_ERR_ON_STRUCT(app);

    ui_screen_change(SCREEN_ID_DS_LOCKED, LV_SCREEN_LOAD_ANIM_NONE, 0, 5000, false);

    vTaskDelay(pdTICKS_TO_MS(5000));

    ui_obj_remove_flag(objects.display_top, LV_OBJ_FLAG_HIDDEN);

    return err;
}