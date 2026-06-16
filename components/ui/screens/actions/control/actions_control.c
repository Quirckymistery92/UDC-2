#include "actions_control.h"
#include "esp_err.h"
#include "esp_check.h"

#include "jpegc_element.h"

/**
 * @brief Handles and extra
 * 
 */

 static jpegc_element_handle_t jpegc_handle;
 static const char *TAG = "lvgl_actions";

/***************** Callbacks for JPEGc Element *****************/
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
        ui_obj_set_style_bg_image_src(cb_data->target_obj,
                                     cb_data->img_dsc,
                                     LV_PART_MAIN | LV_STATE_DEFAULT);
    }

}
/***************** End of Callbacks for JPEGc Element *****************/

//*********************************************************************/
// Inicializador de acciones UI/UX. INTERNAL API (Si no sabes lo que haces, no toques)
//*********************************************************************/
esp_err_t ui_int_sys_actions_init(void)
{
        jpegc_element_config_t config = {
        .output_format = JPEGC_OUTPUT_FORMAT_RGB565,
        .buff_kb = -1,
        .pre_cb = jpegc_element_cb_pre,
        .post_cb = jpegc_element_cb_post,
        .intr_priority = 1,
        .timeout_ms = 500,
        .fs_letter = "/fatfs",
        .fs_default_dir = "images",
    };
    esp_err_t err = jpegc_element_handle_create(&jpegc_handle, &config);
    ESP_RETURN_ON_FALSE(err == ESP_OK, ESP_FAIL, TAG, "Failed to create JPEGc handle: %d", err);
    return err;
}
//*********************************************************************/
// Fin del inicializador de acciones UI/UX
//*********************************************************************/

//**********************************************************************/
// Jpegc Element Actions. Cambiar y/o añadir background a una imagen. PUBLIC API INTERNAL.
//**********************************************************************/
 esp_err_t ui_pub_actions_backg_update(const char *image_path, lv_obj_t *target_obj)
{
    jpegc_element_process_data_t process_data = {
        .input_path = image_path,
        .target_obj = target_obj,
    };
    esp_err_t err = jpegc_element_process(jpegc_handle, &process_data);
    ESP_RETURN_ON_FALSE(err == ESP_OK, ESP_FAIL, TAG, "Failed to update background image with JPEGc: %d", err);
    return ESP_OK;
}