#include "app_init_normal.h"
#include "freertos/FreeRTOS.h"

/* ESP */
#include "esp_log.h"
#include "esp_check.h"

/* Build Info */
#include "build_timestamp.h"

/* Módulos */
#include "ui_manager.h"
#include "jpegc_element.h"
#include "battery_guard.h"
#include "rtc_rx8025t.h"
#include "sys_identity.h"
#include "user_manager.h"


static void bat_g_cb_low_bat(int percentage)
{
    (void)percentage;
}

static void bat_g_cb_shutdown_sys(void)
{

}

static void bat_g_cb_status(battery_status_t status)
{
    ui_battery_set_data(status.percentage, status.voltage_mv, status.is_charging);
}
esp_err_t app_init_normal(const char *TAG)
{
    esp_err_t err = ESP_FAIL;
    
    setenv("TZ", "ART3", 1);
    tzset();

    /* Controlador de reloj */
    ESP_LOGI(TAG, "Inicializando controlador del RTC");
    err = rtc_rx8025t_init(3600, NULL);
    ESP_RETURN_ON_FALSE(err == ESP_OK, err, TAG, "Error al inicializar el controlador del RTC");

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
        
        ESP_LOGW(TAG, "RTC sin datos, corrupto o sin batería, se restableció su valor por el datetime por default");
    }

    ESP_LOGI(TAG, "Inicializando JPEGc Element...");
    err = jpegc_element_init(); /* descompresor de imágenes en FatFS para LVGL */
    ESP_RETURN_ON_FALSE(err == ESP_OK, err, TAG, "Error al inicializar JPEGc Element");
    
    ESP_LOGI(TAG, "Inicializando LVGL...");
    err = ui_init(); /* Tarea y funcionabilidad LVGL */
    ESP_RETURN_ON_FALSE(err == ESP_OK, err, TAG, "Error al inicializar LVGL");

    err = ui_screen_loader(SCREEN_MASK_ALL, SCREEN_ID_DS_LOADER); //Enviamos a LVGL las pantallas a cargar. (Petición SRC_LOAD)
    ESP_RETURN_ON_FALSE(err == ESP_OK, err, TAG, "Error en la carga de display");
    ui_backlight_on(); //Encendemos el Backlight
    ui__helper_sync_sys_screens_get_semaphore(); // Esperamos que SRC_LOAD apruebe y libere el semaphore.

    ui_label_set_text_abs(UI_LABEL_ABS_LOADER_DATA, "%s", "Iniciando sistema...");

    /* Controlador de usuarios */
    ESP_LOGI(TAG, "Inicializando gestor de usuarios.");
    err = user_manager_init();
    ESP_RETURN_ON_FALSE(err == ESP_OK, err, TAG, "Error al inicializar el gestor de usuarios.");

    /* Controlador de batería */
    ESP_LOGI(TAG, "Inicializando controlador de batería...");
    err = battery_guard_init();
    ESP_RETURN_ON_FALSE(err == ESP_OK, err, TAG, "Error al inicializar el controlador de batería.");

    battery_guard_config_t cfg = {
        .threshold_low = 20,
        .threshold_critical = 5,
        .on_low_battery = bat_g_cb_low_bat,
        .auto_deep_sleep = false,
        .on_shutdown_prepare = bat_g_cb_shutdown_sys,
        .on_status_change = bat_g_cb_status,
        .interval_ms = 800,
    };

    err = battery_guard_set_config(&cfg);
    ESP_RETURN_ON_FALSE(err == ESP_OK, err, TAG, "Error configurando el controlador de batería.");

    err = battery_guard_start();
    ESP_RETURN_ON_FALSE(err == ESP_OK, err, TAG, "Error al ejecutar el controlador de batería");

    err = ui_screen_change(SCREEN_ID_DS_LOCKED, LV_SCREEN_LOAD_ANIM_NONE, 0, 5000, false);
    ESP_RETURN_ON_FALSE(err == ESP_OK, err, TAG, "Error al cambiar de pantalla");

    ui_obj_remove_flag(objects.ds_top, LV_OBJ_FLAG_HIDDEN);

    return ESP_OK;
}