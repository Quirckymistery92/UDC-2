#include "ui_label_abs.h"

/* ESP */
#include <string.h>
#include "esp_attr.h"
#include "esp_log.h"
#include <stdarg.h>
#include <stdio.h>

/* Módulos*/
#include "objects.h"

static const char * const TAG = "ABS_Text";

/* ══════════════════════════════════════════════════════════════════
 *                  Buffers estáticos en PSRAM
 * ══════════════════════════════════════════════════════════════════ */
static EXT_RAM_BSS_ATTR char buf_notch_hour[BUF_SIZE_MAX_TIME_FORMAT_HM];
static EXT_RAM_BSS_ATTR char buf_notch_dhour[BUF_SIZE_MAX_TIME_FORMAT_HMS];

static EXT_RAM_BSS_ATTR char buf_notch_ext_bat[BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE];

static EXT_RAM_BSS_ATTR char buf_notch_ext_zigbee[BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE];

static EXT_RAM_BSS_ATTR char buf_notch_ext_ubication[BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE];

static EXT_RAM_BSS_ATTR char buf_notch_ext_wifi[BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE];

static EXT_RAM_BSS_ATTR char buf_locked_src_hour[BUF_SIZE_MAX_TIME_FORMAT_HM];
static EXT_RAM_BSS_ATTR char buf_locked_src_date[BUF_SIZE_MAX_DATE_FORMAT_STR];

static EXT_RAM_BSS_ATTR char buf_home_src_date[BUF_SIZE_MAX_DATE_FORMAT_STR];

static EXT_RAM_BSS_ATTR char buf_users_1_name[BUF_SIZE_USERS_MAX_NAME];
static EXT_RAM_BSS_ATTR char buf_users_1_data[BUF_SIZE_USERS_MAX_DATA];

static EXT_RAM_BSS_ATTR char buf_users_2_name[BUF_SIZE_USERS_MAX_NAME];
static EXT_RAM_BSS_ATTR char buf_users_2_data[BUF_SIZE_USERS_MAX_DATA];

static EXT_RAM_BSS_ATTR char buf_users_3_name[BUF_SIZE_USERS_MAX_NAME];
static EXT_RAM_BSS_ATTR char buf_users_3_data[BUF_SIZE_USERS_MAX_DATA];

static EXT_RAM_BSS_ATTR char buf_users_4_name[BUF_SIZE_USERS_MAX_NAME];
static EXT_RAM_BSS_ATTR char buf_users_4_data[BUF_SIZE_USERS_MAX_DATA];

static EXT_RAM_BSS_ATTR char buf_users_5_name[BUF_SIZE_USERS_MAX_NAME];
static EXT_RAM_BSS_ATTR char buf_users_5_data[BUF_SIZE_USERS_MAX_DATA];

static EXT_RAM_BSS_ATTR char buf_users_6_name[BUF_SIZE_USERS_MAX_NAME];
static EXT_RAM_BSS_ATTR char buf_users_6_data[BUF_SIZE_USERS_MAX_DATA];

static EXT_RAM_BSS_ATTR char buf_lock_user_selected_name[BUF_SIZE_USERS_MAX_NAME];
static EXT_RAM_BSS_ATTR char buf_lock_user_selected_data[BUF_SIZE_USERS_MAX_DATA];

static EXT_RAM_BSS_ATTR char buf_lock_pin_message[BUF_SIZE_LOCK_PIN_MESSAGE];

static EXT_RAM_BSS_ATTR char buf_dtop_popup_tag[BUF_SIZE_DTOP_POPUP_TAG];
static EXT_RAM_BSS_ATTR char buf_dtop_popup_message[BUF_SIZE_DTOP_POPUP_MESSAGE];

static EXT_RAM_BSS_ATTR char buf_dtop_notify_user[BUF_SIZE_DTOP_NOTIFY_USER];

static EXT_RAM_BSS_ATTR char buf_home_user_logged_name[BUF_SIZE_HOME_USER_LOGGED_NAME];

static EXT_RAM_BSS_ATTR char buf_loader_data[BUF_SIZE_LOADER_DATA];

static EXT_RAM_BSS_ATTR char buf_sys_metric_sram[BUF_SIZE_SYS_METRIC_MAX_SIZE];

static EXT_RAM_BSS_ATTR char buf_sys_metric_psram[BUF_SIZE_SYS_METRIC_MAX_SIZE];

static EXT_RAM_BSS_ATTR char buf_sys_metric_cpu[BUF_SIZE_SYS_METRIC_MAX_SIZE -3];


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
        .buf_size = BUF_SIZE_MAX_TIME_FORMAT_HM,
    },
    [UI_LABEL_ABS_NOTCH_EXP_dHOUR] = {
        .obj_ptr = &objects.dtop_notch_exp_hour,
        .buf = buf_notch_dhour,
        .buf_size = BUF_SIZE_MAX_TIME_FORMAT_HMS
    },
    [UI_LABEL_ABS_LOCKED_SCR_HOUR] = {
        .obj_ptr = &objects.dlock_home_lock_time,
        .buf = buf_locked_src_hour,
        .buf_size = BUF_SIZE_MAX_TIME_FORMAT_HM
    },
    [UI_LABEL_ABS_LOCKED_SRC_DATE] = {
        .obj_ptr = &objects.dlock_home_lock_date,
        .buf = buf_locked_src_date,
        .buf_size = BUF_SIZE_MAX_DATE_FORMAT_STR
    },
    [UI_LABEL_ABS_HOME_SRC_DATE] = {
        .obj_ptr = &objects.dhome_datetime,
        .buf = buf_home_src_date,
        .buf_size = BUF_SIZE_MAX_DATE_FORMAT_STR
    },
    [UI_LABEL_ABS_LOCKED_USER_SELECTED_NAME] = {
        .obj_ptr = &objects.dlock_unlock_user_select_name,
        .buf = buf_lock_user_selected_name,
        .buf_size = BUF_SIZE_USERS_MAX_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USER_SELECTED_DATA] = {
        .obj_ptr = &objects.dlock_unlock_user_select_data,
        .buf = buf_lock_user_selected_data,
        .buf_size = BUF_SIZE_USERS_MAX_DATA,
    },
    [UI_LABEL_ABS_NOTCH_EXT_BAT] = {
        .obj_ptr  = &objects.dtop_notch_exp_dat_bat_data,
        .buf      = buf_notch_ext_bat,
        .buf_size = BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE,
    },
    [UI_LABEL_ABS_NOTCH_EXT_WIFI] = {
        .obj_ptr = &objects.dtop_notch_exp_dat_wifi_data,
        .buf = buf_notch_ext_wifi,
        .buf_size = BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE
    },
    [UI_LABEL_ABS_NOTCH_EXT_UBICATION] = {
        .obj_ptr = &objects.dtop_notch_exp_dat_ubi_data,
        .buf = buf_notch_ext_ubication,
        .buf_size = BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE
    },
    [UI_LABEL_ABS_NOTCH_EXT_ZIGBEE] = {
        .obj_ptr = &objects.dtop_notch_exp_dat_iot_data,
        .buf = buf_notch_ext_zigbee,
        .buf_size = BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE
    },
    [UI_LABEL_ABS_LOCKED_USERS_1_NAME] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_1_name,
        .buf      = buf_users_1_name,
        .buf_size = BUF_SIZE_USERS_MAX_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_1_DATA] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_1_data,
        .buf      = buf_users_1_data,
        .buf_size = BUF_SIZE_USERS_MAX_DATA,
    },
    [UI_LABEL_ABS_LOCKED_USERS_2_NAME] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_2_name,
        .buf      = buf_users_2_name,
        .buf_size = BUF_SIZE_USERS_MAX_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_2_DATA] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_2_data,
        .buf      = buf_users_2_data,
        .buf_size = BUF_SIZE_USERS_MAX_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_3_NAME] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_3_name,
        .buf      = buf_users_3_name,
        .buf_size = BUF_SIZE_USERS_MAX_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_3_DATA] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_3_data,
        .buf      = buf_users_3_data,
        .buf_size = BUF_SIZE_USERS_MAX_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_4_NAME] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_4_name,
        .buf      = buf_users_4_name,
        .buf_size = BUF_SIZE_USERS_MAX_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_4_DATA] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_4_data,
        .buf      = buf_users_4_data,
        .buf_size = BUF_SIZE_USERS_MAX_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_5_NAME] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_5_name,
        .buf      = buf_users_5_name,
        .buf_size = BUF_SIZE_USERS_MAX_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_5_DATA] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_5_data,
        .buf      = buf_users_5_data,
        .buf_size = BUF_SIZE_USERS_MAX_DATA,
    },

    [UI_LABEL_ABS_LOCKED_USERS_6_NAME] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_6_name,
        .buf      = buf_users_6_name,
        .buf_size = BUF_SIZE_USERS_MAX_NAME,
    },
    [UI_LABEL_ABS_LOCKED_USERS_6_DATA] = {
        .obj_ptr  = &objects.dlock_unlock_userpick_6_data,
        .buf      = buf_users_6_data,
        .buf_size = BUF_SIZE_USERS_MAX_DATA,
    },
    [UI_LABEL_ABS_LOCKED_PIN_MESSAGE] = {
        .obj_ptr  = &objects.dlock_unlock_pin_message,
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
        .obj_ptr  = &objects.dhome_user_login_name,
        .buf      = buf_home_user_logged_name,
        .buf_size = BUF_SIZE_HOME_USER_LOGGED_NAME,
    },
    [UI_LABEL_ABS_LOADER_DATA] = {
        .obj_ptr  = &objects.dload_firmware_data_lbl,
        .buf      = buf_loader_data,
        .buf_size = BUF_SIZE_LOADER_DATA
    },
    [UI_LABEL_ABS_HOME_SYS_CPU] = {
        .obj_ptr = &objects.dhome_act_home_syst_cpu,
        .buf = buf_sys_metric_cpu,
        .buf_size = BUF_SIZE_SYS_METRIC_MAX_SIZE -3,
    },
    [UI_LABEL_ABS_HOME_SYS_SRAM] = {
        .obj_ptr = &objects.dhome_act_home_syst_intram,
        .buf = buf_sys_metric_sram,
        .buf_size = BUF_SIZE_SYS_METRIC_MAX_SIZE,
    },
    [UI_LABEL_ABS_HOME_SYS_PSRAM] = {
        .obj_ptr = &objects.dhome_act_home_syst_psram,
        .buf = buf_sys_metric_psram,
        .buf_size = BUF_SIZE_SYS_METRIC_MAX_SIZE
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