#include "actions.h"
#include "screens.h"
#include "ui_manager.h"
#include "esp_err.h"
#include "ui.h"
#include "esp_log.h"
#include "esp_check.h"
// Para acciones internas expuestas del usurio
#include "jpegc_element.h"


/**
 * @brief Handles and extra
 * 
 */
 static jpegc_element_handle_t jpegc_handle;
 static const char *TAG = "lvgl_actions";


/**
 * @brief Callback Functions for JPEGc Element (button presses, etc.)
 * 
 * @note interno. No exponer a UI Manager, solo para uso dentro de screens.
 */
static void jpegc_element_cb_pre(void)
{
    ui_obj_set_style_bg_image_src(objects.ds_home, NULL,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
}
static void jpegc_element_cb_post(jpegc_element_cb_post_data_t *cb_data)
{
    if (cb_data->result == ESP_OK){
        ui_obj_set_style_bg_image_src(objects.ds_home,
                                     cb_data->img_dsc,
                                     LV_PART_MAIN | LV_STATE_DEFAULT);
    }

}
esp_err_t actions_sys_backg_init(void)
{
    jpegc_element_config_t config = {
        .output_format = JPEGC_OUTPUT_FORMAT_RGB565,
        .buff_kb = -1,
        .pre_cb = jpegc_element_cb_pre,
        .post_cb = jpegc_element_cb_post,
        .intr_priority = 1,
        .timeout_ms = 500,
        .fs_letter = "/spiffs",
        .fs_default_dir = "images",
    };
    esp_err_t err = jpegc_element_handle_create(&jpegc_handle, &config);
    ESP_RETURN_ON_FALSE(err == ESP_OK, ESP_FAIL, TAG, "Failed to create JPEGc handle: %d", err);

    jpegc_element_process_data_t process_data = {
        .input_path = "bg_thunder_rail.jpg",
    };
    err = jpegc_element_process(jpegc_handle, &process_data);
    ESP_RETURN_ON_FALSE(err == ESP_OK, ESP_FAIL,TAG, "Failed to process image with JPEGc: %d", err);
    return ESP_OK;
}
esp_err_t actions_sys_backg_update(const char *image_path)
{
    jpegc_element_process_data_t process_data = {
        .input_path = image_path,
    };
    esp_err_t err = jpegc_element_process(jpegc_handle, &process_data);
    ESP_RETURN_ON_FALSE(err == ESP_OK, ESP_FAIL, TAG, "Failed to update background image with JPEGc: %d", err);
    return ESP_OK;
}