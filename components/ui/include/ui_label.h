#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════
 *                  Tamaños de buffer por label
 * ══════════════════════════════════════════════════════════════════ */
#define BUF_SIZE_MAX_TIME_FORMAT_HM 7 /* HH:MM\0 */
#define BUF_SIZE_MAX_TIME_FORMAT_HMS 10 /* HH:MM:SS\0 */
#define BUF_SIZE_MAX_DATE_FORMAT_DMY 13 /* DD:MM:YYYY\0 */
#define BUF_SIZE_MAX_DATE_FORMAT_STR 38 /* DD de M del YYYY\0*/ // 13 de agosto del 2026. Toma MAX de names en ES: Septiembre(10)

#define BUF_SIZE_USERS_MAX_NAME 16
#define BUF_SIZE_USERS_MAX_DATA 28

#define BUF_SIZE_LOCK_PIN_MESSAGE 64

#define BUF_SIZE_DTOP_POPUP_TAG 32
#define BUF_SIZE_DTOP_POPUP_MESSAGE 128

#define BUF_SIZE_DTOP_NOTIFY_USER 256

#define BUF_SIZE_HOME_USER_LOGGED_NAME 32

#define BUF_SIZE_LOADER_DATA 32

#define BUF_SIZE_MAX_NOTCH_MESSAGE_SERVICE 19 /* Con teminador nulo obviamente xd */

#define BUF_SIZE_SYS_METRIC_MAX_SIZE 17


/* ══════════════════════════════════════════════════════════════════
 *  Label — ABS (Assigned Buffer Static)
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief IDs de labels con buffer estático en PSRAM.
 *
 * Cada entry corresponde a un lv_obj_t* (label) y un buffer fijo
 * en PSRAM asignado en compile time. Agregar nuevos IDs aquí y
 * registrar la entry correspondiente en core/ui_label_abs.c.
 */
typedef enum {
    UI_LABEL_NONE = 0,
    UI_LABEL_ABS_NOTCH_HOUR,
    UI_LABEL_ABS_NOTCH_EXP_dHOUR, // dHour significa Hora detallada que incluye segundos.
    UI_LABEL_ABS_LOCKED_SCR_HOUR,
    UI_LABEL_ABS_LOCKED_SRC_DATE, // Date significa la fecha actual. Formato: DD de MM del YYYY
    UI_LABEL_ABS_HOME_SRC_DATE,
    UI_LABEL_ABS_LOCKED_USER_SELECTED_NAME,
    UI_LABEL_ABS_LOCKED_USER_SELECTED_DATA,
    UI_LABEL_ABS_NOTCH_EXT_BAT,
    UI_LABEL_ABS_NOTCH_EXT_WIFI,
    UI_LABEL_ABS_NOTCH_EXT_ZIGBEE,
    UI_LABEL_ABS_NOTCH_EXT_UBICATION,
    UI_LABEL_ABS_LOCKED_USERS_1_NAME,
    UI_LABEL_ABS_LOCKED_USERS_1_DATA,
    UI_LABEL_ABS_LOCKED_USERS_2_NAME,
    UI_LABEL_ABS_LOCKED_USERS_2_DATA,
    UI_LABEL_ABS_LOCKED_USERS_3_NAME,
    UI_LABEL_ABS_LOCKED_USERS_3_DATA,
    UI_LABEL_ABS_LOCKED_USERS_4_NAME,
    UI_LABEL_ABS_LOCKED_USERS_4_DATA,
    UI_LABEL_ABS_LOCKED_USERS_5_NAME,
    UI_LABEL_ABS_LOCKED_USERS_5_DATA,
    UI_LABEL_ABS_LOCKED_USERS_6_NAME,
    UI_LABEL_ABS_LOCKED_USERS_6_DATA,
    UI_LABEL_ABS_LOCKED_PIN_MESSAGE,
    UI_LABEL_ABS_DTOP_POPUP_TAG,
    UI_LABEL_ABS_DTOP_POPUP_MESSAGE,
    UI_LABEL_ABS_DTOP_NOTIFY_USER,
    UI_LABEL_ABS_HOME_USER_LOGGED_NAME,
    UI_LABEL_ABS_LOADER_DATA,
    UI_LABEL_ABS_HOME_SYS_CPU,
    UI_LABEL_ABS_HOME_SYS_SRAM,
    UI_LABEL_ABS_HOME_SYS_PSRAM,
    UI_LABEL_ABS_COUNT
} ui_label_abs_id_t;

#ifdef __cplusplus
}
#endif