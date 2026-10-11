idf_component_register(
    SRCS
        "src/udc_metrics.c"
    INCLUDE_DIRS
        "include"
    PRIV_INCLUDE_DIRS
        "src"
    REQUIRES
        freertos esp_timer fatfs spiffs littlefs
)