#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define UX_CHECK_ABSOLUTE_EXCLUSIVE_CHAR(txt, max, min) \
        do { \
            if ((txt) == NULL) return ESP_ERR_INVALID_ARG; \
            size_t size = strnlen((txt), (max)); \
            if (size >= (max) || size <= (size_t)(min)) return ESP_ERR_INVALID_ARG; \
        } while(0)


#ifdef __cplusplus
}
#endif