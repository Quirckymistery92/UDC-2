#pragma once

/* ESP */
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UDC_ERR_NVS 0xc000
#define UDC_ERR_STORAGE 0xc001
#define UDC_ERR_INTL_COMM 0xc002
#define UDC_ERR_ 0xc003

/* Variables */
    #define DESC_AP_INSERT(app, insert_err) \
        ((app)->init_result = (insert_err), (app))

#define DSC_RETURN_STRUCT_ON_FALSE(condicion, app, insert_err)  \
    do {                                                        \
        if (unlikely(!(condicion))) {                           \
            (app)->init_result = (insert_err);                  \
            return (app);                                       \
        }                                                       \
    } while (0)

#define DSC_RETURN_ERR_ON_STRUCT(app_ptr)                       \
    do {                                                        \
        if (unlikely((app_ptr)->init_result != ESP_OK)) {       \
            return (app_ptr->init_result);                      \
        }                                                       \
    } while (0)



    typedef enum {
        DSC_NONE = 0,

        DSC_APP_STORAGE_FLASH_NVS = 1 << 0,
        DSC_APP_STORAGE_FLASH     = 1 << 1,
        DSC_APP_STORAGE_EXTERNAL  = 1 << 2,
        DSC_APP_DECODER_IMAGES    = 1 << 3,        
        DSC_APP_RTC_EXTERNAL      = 1 << 4,
        DSC_APP_METRICS           = 1 << 5,
        DSC_APP_UX                = 1 << 6,
        DSC_APP_USER_MANAGER      = 1 << 7,

        DSC_DRIVER_COMM_I2C       = 1 << 8,
        DSC_DRIVER_COMM_SDIO      = 1 << 9,
        DSC_DRIVER_DISPLAY        = 1 << 10,
        DSC_DRIVER_TOUCH          = 1 << 11,
        DSC_DRIVER_BRIGHTNESS     = 1 << 12,
        DSC_DRIVER_BATTERY        = 1 << 13,
        
        DSC_CT_ANY_STORAGE = (
            DSC_APP_STORAGE_FLASH | DSC_APP_STORAGE_EXTERNAL
        ),

        DSC_CT_ANY_INTERNAL_COM = (
            DSC_DRIVER_COMM_I2C | DSC_DRIVER_COMM_SDIO
        ),

        DSC_CT_ALL_MASKS = (
            DSC_APP_STORAGE_FLASH_NVS | DSC_APP_STORAGE_FLASH | DSC_APP_STORAGE_EXTERNAL |
            DSC_APP_DECODER_IMAGES    | DSC_APP_RTC_EXTERNAL   | DSC_APP_METRICS          |
            DSC_APP_UX                | DSC_APP_USER_MANAGER   | DSC_DRIVER_COMM_I2C      |
            DSC_DRIVER_COMM_SDIO      | DSC_DRIVER_DISPLAY     | DSC_DRIVER_TOUCH         |
            DSC_DRIVER_BRIGHTNESS     | DSC_DRIVER_BATTERY
        )
    } app_descriptor_t;

    typedef struct {
        esp_err_t init_result;
        const app_descriptor_t app_desc;
        const char *app_tag;
        const char *app_name;
    }app_desc_status_t;

#ifdef __cplusplus
}
#endif