#include "ui_zb_types.h"

esp_err_t ui_zb_hall_register_device(uint16_t id, const char *name, ui_zb_type_dev_t type, uint8_t column, uint8_t row,ui_zb_callback_result_t cb_result);

esp_err_t ui_zb_hall_unregister_device(uint16_t id, ui_zb_callback_result_t cb_result);

esp_err_t ui_zb_hall_update_device(uint16_t id, ui_zb_update_data_t data, ui_zb_callback_result_t cb_result);

esp_err_t ui_zb_hall_set_loading_device(uint16_t id, bool is_loading, ui_zb_callback_result_t cb_result);