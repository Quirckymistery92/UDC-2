#include "battery.h"

/* ESP */
#include "esp_log.h"
#include "esp_check.h"

/* Módulos */
#include "battery_guard.h"
#include "ui_drivers.h"

/* Variables */
    static app_desc_status_t apd = {
        .app_desc = DSC_DRIVER_BATTERY,
        .app_name = "Driver batería",
        .app_tag = NULL,
        .init_result = ESP_OK
    };

static void bat_g_cb_low_bat(int percentage)
{
    (void)percentage;
}

static void bat_g_cb_shutdown_sys(void)
{

}
static void bat_g_cb_status(battery_status_t status)
{
    ui__drivers_battery_set_status(status.percentage, status.is_charging, status.driver_ok);
}
app_desc_status_t* bat_g_boot_normal(const char * TAG)
{
    esp_err_t err = ESP_OK;
    
    apd.app_tag = TAG;

    err = battery_guard_init();
    DSC_RETURN_STRUCT_ON_FALSE(err == ESP_OK, &apd, err);

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
    DSC_RETURN_STRUCT_ON_FALSE(err == ESP_OK, &apd, err);

    err = battery_guard_start();
    DSC_RETURN_STRUCT_ON_FALSE(err == ESP_OK, &apd, err);

    return DESC_AP_INSERT(&apd, err);
}
