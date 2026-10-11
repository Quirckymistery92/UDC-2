#include "clock_control.h"

/* ESP */
#include <sys/time.h>

/* Módulos */
#include <lvgl.h>
#include "interface.h"
#include "../screens_int_all.h" // Todas las pantallas
#include "udc_time.h"
#include "ui_label_abs.h"

#define TIME_SYNC_CLOCK_SEC 1
static lv_timer_t * clock_timer = NULL;

static const char * clock_day_of_week(int tm_wday) {
        const char* dias[] = {
        "Domingo", "Lunes", "Martes", "Miércoles", 
        "Jueves", "Viernes", "Sábado"
    };
    
    if (tm_wday < 0 || tm_wday > 6) {
        return "Desconocido";
    }
    return dias[tm_wday];
}
static const char *clock_mouth_of_year(int tm_mon) {
        const char* meses[] = {
        "enero", "febrero", "narzo", "abril", 
        "nayo", "junio", "julio", "agosto", 
        "septiembre", "octubre", "noviembre", "diciembre"
    };
    
    if (tm_mon < 0 || tm_mon > 11) {
        return "Desconocido";
    }
    return meses[tm_mon];
}
static void update_time_timer_cb(lv_timer_t * timer)
{
    esp_err_t ret = ESP_OK;

    struct tm timeinfo;
    ret = udc_time_get_int_struct_tm(&timeinfo);
    if(ret != ESP_OK) return;

    // Cache para evitar escrituras redundantes
    static int last_notch_min = -1;
    static int last_notch_sec = -1;
    static int last_locked_min = -1;
    static int last_locked_day = -1;
    static int last_home_day = -1;

    if (ui_int__display_is_created_ds_top()) {
      if (timeinfo.tm_min != last_notch_min) {
        ui_int__helper_label_abs_apply_sync_fmt(
            UI_LABEL_ABS_NOTCH_HOUR, "%02d:%02d", timeinfo.tm_hour,
            timeinfo.tm_min);
        last_notch_min = timeinfo.tm_min;
      }

      if (timeinfo.tm_sec != last_notch_sec) {
        ui_int__helper_label_abs_apply_sync_fmt(
            UI_LABEL_ABS_NOTCH_EXP_dHOUR, "%02d:%02d:%02d", timeinfo.tm_hour,
            timeinfo.tm_min, timeinfo.tm_sec);
        last_notch_sec = timeinfo.tm_sec;
      }
    } else {
      last_notch_min = -1;
      last_notch_sec = -1;
    }

    if (ui_int__display_is_created_ds_locked()) {
      if (timeinfo.tm_min != last_locked_min) {
        ui_int__helper_label_abs_apply_sync_fmt(
            UI_LABEL_ABS_LOCKED_SCR_HOUR, "%02d:%02d", timeinfo.tm_hour,
            timeinfo.tm_min);
        last_locked_min = timeinfo.tm_min;
      }

      if (timeinfo.tm_mday != last_locked_day) {
        ui_int__helper_label_abs_apply_sync_fmt(UI_LABEL_ABS_LOCKED_SRC_DATE, "%s, %d de %s del %d", clock_day_of_week(timeinfo.tm_wday),
                                                                                                           timeinfo.tm_mday, 
                                                                                                           clock_mouth_of_year(timeinfo.tm_mon),
                                                                                                           timeinfo.tm_year + 1900);
        last_locked_day = timeinfo.tm_mday;
      }
    } else {
      last_locked_min = -1;
      last_locked_day = -1;
    }

    /* if (ui_int__is_screen_created_ds_block()) { */

    if(ui_int__display_is_created_ds_home()) {
      if (timeinfo.tm_mday != last_home_day) {
        ui_int__helper_label_abs_apply_sync_fmt(UI_LABEL_ABS_HOME_SRC_DATE, "%s, %d de %s del %d", clock_day_of_week(timeinfo.tm_wday),
                                                                                                     timeinfo.tm_mday, 
                                                                                                     clock_mouth_of_year(timeinfo.tm_mon),
                                                                                                     timeinfo.tm_year + 1900);
        last_home_day = timeinfo.tm_mday;
      }
    } else {
      last_home_day = -1;
    }
}
void ui_int__sys_action_clock_init(void)
{
    clock_timer = lv_timer_create(update_time_timer_cb, TIME_SYNC_CLOCK_SEC * 1000, NULL);

    lv_timer_ready(clock_timer);
}
