/* ══════════════════════════════════════════════════════════════════
 * ══════════════════════════════════════════════════════════════════ 
 *                   ui_label_abs.c Implementación
 * ══════════════════════════════════════════════════════════════════ 
 * ══════════════════════════════════════════════════════════════════ */

#include "ui_label_abs.h"
#include "../screens/screens.h"
#include <string.h>
#include "esp_attr.h"
#include "esp_log.h"
#include <stdarg.h>
#include <stdio.h>
static const char *TAG = "ABS Text";

/* ══════════════════════════════════════════════════════════════════
 *                  Tamaños de buffer por label
 * ══════════════════════════════════════════════════════════════════ */
#define BUF_SIZE_NOTCH_HOUR   12 
#define BUF_SIZE_NOTCH_EXT_BAT 17
#define BUF_SIZE_LOCK_USER_SELECTED_NAME 32
#define BUF_SIZE_LOCK_USER_SELECTED_DATA 28

#define BUF_SIZE_USERS_1_NAME 16
#define BUF_SIZE_USERS_1_DATA 28

#define BUF_SIZE_USERS_2_NAME 16
#define BUF_SIZE_USERS_2_DATA 28

#define BUF_SIZE_USERS_3_NAME 16
#define BUF_SIZE_USERS_3_DATA 28

#define BUF_SIZE_USERS_4_NAME 16
#define BUF_SIZE_USERS_4_DATA 28

#define BUF_SIZE_USERS_5_NAME 16
#define BUF_SIZE_USERS_5_DATA 28

#define BUF_SIZE_USERS_6_NAME 16
#define BUF_SIZE_USERS_6_DATA 28

#define BUF_SIZE_LOCK_PIN_MESSAGE 64

#define BUF_SIZE_DTOP_POPUP_TAG 32
#define BUF_SIZE_DTOP_POPUP_MESSAGE 128

#define BUF_SIZE_DTOP_NOTIFY_USER 256

#define BUF_SIZE_HOME_USER_LOGGED_NAME 32

/* ══════════════════════════════════════════════════════════════════
 *                  Buffers estáticos en PSRAM
 * ══════════════════════════════════════════════════════════════════ */
static EXT_RAM_BSS_ATTR char buf_notch_hour[BUF_SIZE_NOTCH_HOUR];
static EXT_RAM_BSS_ATTR char buf_notch_ext_bat[BUF_SIZE_NOTCH_EXT_BAT];

static EXT_RAM_BSS_ATTR char buf_users_1_name[BUF_SIZE_USERS_1_NAME];
static EXT_RAM_BSS_ATTR char buf_users_1_data[BUF_SIZE_USERS_1_DATA];

static EXT_RAM_BSS_ATTR char buf_users_2_name[BUF_SIZE_USERS_2_NAME];
static EXT_RAM_BSS_ATTR char buf_users_2_data[BUF_SIZE_USERS_2_DATA];

static EXT_RAM_BSS_ATTR char buf_users_3_name[BUF_SIZE_USERS_3_NAME];
static EXT_RAM_BSS_ATTR char buf_users_3_data[BUF_SIZE_USERS_3_DATA];

static EXT_RAM_BSS_ATTR char buf_users_4_name[BUF_SIZE_USERS_4_NAME];
static EXT_RAM_BSS_ATTR char buf_users_4_data[BUF_SIZE_USERS_4_DATA];

static EXT_RAM_BSS_ATTR char buf_users_5_name[BUF_SIZE_USERS_5_NAME];
static EXT_RAM_BSS_ATTR char buf_users_5_data[BUF_SIZE_USERS_5_DATA];

static EXT_RAM_BSS_ATTR char buf_users_6_name[BUF_SIZE_USERS_6_NAME];
static EXT_RAM_BSS_ATTR char buf_users_6_data[BUF_SIZE_USERS_6_DATA];

static EXT_RAM_BSS_ATTR char buf_lock_user_selected_name[BUF_SIZE_LOCK_USER_SELECTED_NAME];
static EXT_RAM_BSS_ATTR char buf_lock_user_selected_data[BUF_SIZE_LOCK_USER_SELECTED_DATA];

static EXT_RAM_BSS_ATTR char buf_lock_pin_message[BUF_SIZE_LOCK_PIN_MESSAGE];

static EXT_RAM_BSS_ATTR char buf_dtop_popup_tag[BUF_SIZE_DTOP_POPUP_TAG];
static EXT_RAM_BSS_ATTR char buf_dtop_popup_message[BUF_SIZE_DTOP_POPUP_MESSAGE];

static EXT_RAM_BSS_ATTR char buf_dtop_notify_user[BUF_SIZE_DTOP_NOTIFY_USER];

static EXT_RAM_BSS_ATTR char buf_home_user_logged_name[BUF_SIZE_HOME_USER_LOGGED_NAME];
/* ══════════════════════════════════════════════════════════════════
 *                          Entrada de tabla
 * ══════════════════════════════════════════════════════════════════ */

typedef struct {
    lv_obj_t  **obj_ptr;
    char       *buf;
    uint16_t    buf_size;
} label_abs_entry_t;

/* ══════════════════════════════════════════════════════════════════
 *                          Tabla estática
 * ══════════════════════════════════════════════════════════════════ */

static const label_abs_entry_t s_table[UI_LABEL_ABS_COUNT] = {

    [UI_LABEL_ABS_NOTCH_HOUR] = {
        .obj_ptr  = &objects.dtop_notch_min_hour,
        .buf      = buf_notch_hour,
        .buf_size = BUF_SIZE_NOTCH_HOUR,
    },
    [UI_LABEL_ABS_LOCKED_USER_SELECTED_NAME] = {
        .obj_ptr = &objects.dl_home_unlock_user_select_name,
        .buf = buf_lock_user_selected_name,
        .buf_size = BUF_SIZE_LOCK_USER_SELECTED_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USER_SELECTED_DATA] = {
        .obj_ptr = &objects.dl_home_unlock_user_select_data,
        .buf = buf_lock_user_selected_data,
        .buf_size = BUF_SIZE_LOCK_USER_SELECTED_DATA,
    },
    [UI_LABEL_ABS_NOTCH_EXT_BAT] = {
        .obj_ptr  = &objects.dtop_notch_exp_dat_bat_data,
        .buf      = buf_notch_ext_bat,
        .buf_size = BUF_SIZE_NOTCH_EXT_BAT,
    },
    
    [UI_LABEL_ABS_LOCKED_USERS_1_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_1_name,
        .buf      = buf_users_1_name,
        .buf_size = BUF_SIZE_USERS_1_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_1_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_1_data,
        .buf      = buf_users_1_data,
        .buf_size = BUF_SIZE_USERS_1_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_2_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_2_name,
        .buf      = buf_users_2_name,
        .buf_size = BUF_SIZE_USERS_2_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_2_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_2_data,
        .buf      = buf_users_2_data,
        .buf_size = BUF_SIZE_USERS_2_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_3_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_3_name,
        .buf      = buf_users_3_name,
        .buf_size = BUF_SIZE_USERS_3_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_3_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_3_data,
        .buf      = buf_users_3_data,
        .buf_size = BUF_SIZE_USERS_3_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_4_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_4_name,
        .buf      = buf_users_4_name,
        .buf_size = BUF_SIZE_USERS_4_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_4_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_4_data,
        .buf      = buf_users_4_data,
        .buf_size = BUF_SIZE_USERS_4_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_5_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_5_name,
        .buf      = buf_users_5_name,
        .buf_size = BUF_SIZE_USERS_5_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_5_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_5_data,
        .buf      = buf_users_5_data,
        .buf_size = BUF_SIZE_USERS_5_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_6_NAME] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_6_name,
        .buf      = buf_users_6_name,
        .buf_size = BUF_SIZE_USERS_6_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_6_DATA] = {
        .obj_ptr  = &objects.dl_home_unlock_userpick_6_data,
        .buf      = buf_users_6_data,
        .buf_size = BUF_SIZE_USERS_6_DATA,
    },
    [UI_LABEL_ABS_LOCKED_PIN_MESSAGE] = {
        .obj_ptr  = &objects.dl_home_unlock_pin_message,
        .buf      = buf_lock_pin_message,
        .buf_size = BUF_SIZE_LOCK_PIN_MESSAGE, 
    },
    [UI_LABEL_ABS_DTOP_POPUP_TAG] = {
        .obj_ptr  = &objects.dtop_modal_pup_tit_tag,
        .buf      = buf_dtop_popup_tag,
        .buf_size = BUF_SIZE_DTOP_POPUP_TAG,
    },
    [UI_LABEL_ABS_DTOP_POPUP_MESSAGE] = {
        .obj_ptr  = &objects.dtop_modal_pup_tit_data,
        .buf      = buf_dtop_popup_message,
        .buf_size = BUF_SIZE_DTOP_POPUP_MESSAGE,
    },
    [UI_LABEL_ABS_DTOP_NOTIFY_USER] = {
        .obj_ptr = &objects.dtop_notifiy_message,
        .buf = buf_dtop_notify_user,
        .buf_size = BUF_SIZE_DTOP_NOTIFY_USER
    },
    [UI_LABEL_ABS_HOME_USER_LOGGED_NAME] = {
        .obj_ptr  = &objects.ds_home_users_name,
        .buf      = buf_home_user_logged_name,
        .buf_size = BUF_SIZE_HOME_USER_LOGGED_NAME,
    }
};

/* ══════════════════════════════════════════════════════════════════
 *                              API
 * ══════════════════════════════════════════════════════════════════ */

esp_err_t label_abs_prepare(ui_label_abs_id_t id,
                             const char       *text,
                             lv_obj_t        **out_obj,
                             const char      **out_buf)
{
    if (id >= UI_LABEL_ABS_COUNT || !text || !out_obj || !out_buf) {
        return ESP_ERR_INVALID_ARG;
    }

    const label_abs_entry_t *e = &s_table[id];

    if (!(*e->obj_ptr)) {
        ESP_LOGE(TAG, "label id=%d: obj aún no creado", (int)id);
        return ESP_ERR_INVALID_STATE;
    }

    /* Copia segura al buffer PSRAM — trunca si excede buf_size */
    size_t len = strlcpy(e->buf, text, (size_t)e->buf_size);
    if(len >= e->buf_size) {
        ESP_LOGW(TAG, "label id=%d: texto truncado a %d bytes", (int)id, (int)e->buf_size);
    }

    *out_obj = *e->obj_ptr;
    *out_buf = (const char *)e->buf;

    return ESP_OK;
}
esp_err_t ui_int__helper_label_abs_apply_sync(ui_label_abs_id_t id, const char *text) {
    if (id >= UI_LABEL_ABS_COUNT || !text) {
        return ESP_ERR_INVALID_ARG;
    }
    lv_obj_t *obj = NULL;
    const char * buf = NULL;
    esp_err_t ret = label_abs_prepare(id, text, &obj, &buf);
    if(ret != ESP_OK) return ret;

    lv_label_set_text_static(obj, buf);
    return ret;
}
esp_err_t ui_int__helper_label_abs_apply_sync_fmt(ui_label_abs_id_t id, const char *fmt, ...) {
    if (id >= UI_LABEL_ABS_COUNT || !fmt) {
        return ESP_ERR_INVALID_ARG;
    }

    const label_abs_entry_t *e = &s_table[id];

    if (!(*e->obj_ptr)) {
        ESP_LOGE(TAG, "label id=%d: obj aún no creado", (int)id);
        return ESP_ERR_INVALID_STATE;
    }

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(e->buf, (size_t)e->buf_size, fmt, args);
    va_end(args);

    if (len >= e->buf_size) {
        ESP_LOGW(TAG, "label id=%d: texto formateado truncado a %d bytes (requería %d)", 
                 (int)id, (int)e->buf_size, len);
    } else if (len < 0) {
        ESP_LOGE(TAG, "label id=%d: error al formatear el texto", (int)id);
        return ESP_FAIL;
    }

    lv_label_set_text_static(*e->obj_ptr, e->buf);
    return ESP_OK;
}