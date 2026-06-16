#include "NVSm_sys.h"
#include "user_manager_nvs.h"

#define NVS_NAMESPACE "user_m_ns"
#define NVS_KEY_GUEST "guest_en"

bool NVSmModule_user_m_R_guest_state(void) {
    nvs_handle_t my_handle;
    esp_err_t err;
    uint8_t state = 1; // Default: true
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
    if (err == ESP_OK) {
        nvs_get_u8(my_handle, NVS_KEY_GUEST, &state);
        nvs_close(my_handle);
    }
    return state == 1;
}

esp_err_t NVSmModule_user_m_W_guest_state(bool enable) {
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err == ESP_OK) {
        nvs_set_u8(my_handle, NVS_KEY_GUEST, enable ? 1 : 0);
        nvs_commit(my_handle);
        nvs_close(my_handle);
    }
    return err;
}