#include "act_home.h"

/* ESP */
#include "esp_err.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Módulos */
#include "interface.h"
#include "ui_image_sia_internal.h"
#include "ui_label_abs.h"
#include "ui_zb_controller.h"
#include "user_mgr.h"
#include "udc_metrics.h"
#include "littlefs_controller.h"

/* Variables */
    #define TAG "ACT_HOME"
    #define AUSER_ANIM_TIME_MS 350
    #define METRIC_HANDLE_MAX 3
    static udc_metric_handle_t metrics_handle[METRIC_HANDLE_MAX];

typedef enum {
    UX_NAV_ID_HOME = 1,
    UX_NAV_ID_IOT = 2,
    UX_NAV_ID_DASHBOARD = 3
} navegation_id_t;

typedef enum {
    UX_ACT_USER_NONE = -1,
    UX_ACT_USER_LOGOUT = 0,
    UX_ACT_USER_LOCK,
    UX_ACT_USER_RESTART,
    UX_ACT_USER_POWEROFF
} action_user_t;

static void ux_home__cb_dhome_act_scenes(lv_event_t *e)
{
}
static void ux_home__cb_dhome_act_home_zigbee(lv_event_t *e)
{
}
static void ux_home__cb_dhome_act_home_settings(lv_event_t *e)
{
}

static void callback_logout_users(user_mgr_event_t event, uint16_t user_id, void *ctx)
{
    if(event == USER_MGR_EVT_SUCCESS){
        ui_int__dtop_notify_user_show(false, "Sesión cerrada correctamente");
        ESP_LOGI(TAG, "Sesión cerrada correctamente");
    } else {
        ui_int__dtop_notify_user_show(false, "Falló al cerrar la sesión de este usuario");
        ESP_LOGI(TAG, "Falló al cerrar la sesión de este usuario");
    }
}

static void util_set_children_visibility(lv_obj_t *parent, bool hidden)
{
    uint32_t child_cnt = lv_obj_get_child_cnt(parent);
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        if(hidden) {
            lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_remove_flag(child, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void anim_cb_translate_x(void *var, int32_t v)
{
    lv_obj_set_style_translate_x((lv_obj_t *)var, v, 0);
}

static void anim_cb_translate_y(void *var, int32_t v)
{
    lv_obj_set_style_translate_y((lv_obj_t *)var, v, 0);
}

static void anim_ready_cb_open_children(lv_anim_t *a)
{
    util_set_children_visibility((lv_obj_t *)a->var, false);
}

static void ux_home__cb_dhome_action_user_cont(lv_event_t *e)
{
    lv_obj_t *trigger_btn = lv_event_get_target(e);
    lv_obj_t *child_menu = objects.dhome_auser_panel;

    util_set_children_visibility(child_menu, true);
    
    lv_obj_set_style_translate_x(child_menu, 0, 0);
    lv_obj_set_style_translate_y(child_menu, 0, 0);
    
    lv_obj_remove_flag(objects.dhome_auser_modal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_update_layout(objects.dhome_auser_modal);

    lv_area_t trigger_area;
    lv_area_t menu_area;
    lv_obj_get_coords(trigger_btn, &trigger_area);
    lv_obj_get_coords(child_menu, &menu_area);

    int32_t start_tx = ((trigger_area.x1 + trigger_area.x2) / 2) - ((menu_area.x1 + menu_area.x2) / 2);
    int32_t start_ty = ((trigger_area.y1 + trigger_area.y2) / 2) - ((menu_area.y1 + menu_area.y2) / 2);

    lv_anim_t ax;
    lv_anim_init(&ax);
    lv_anim_set_var(&ax, child_menu);
    lv_anim_set_time(&ax, AUSER_ANIM_TIME_MS);
    lv_anim_set_values(&ax, start_tx, 0);
    lv_anim_set_exec_cb(&ax, anim_cb_translate_x);
    lv_anim_set_path_cb(&ax, lv_anim_path_ease_out);
    lv_anim_start(&ax);

    lv_anim_t ay;
    lv_anim_init(&ay);
    lv_anim_set_var(&ay, child_menu);
    lv_anim_set_time(&ay, AUSER_ANIM_TIME_MS);
    lv_anim_set_values(&ay, start_ty, 0);
    lv_anim_set_exec_cb(&ay, anim_cb_translate_y);
    lv_anim_set_path_cb(&ay, lv_anim_path_ease_out);
    lv_anim_set_ready_cb(&ay, anim_ready_cb_open_children);
    lv_anim_start(&ay);
}

static void ux_home__cb_dhome_action_user_buttons(lv_event_t *e)
{
    void *user_data = lv_event_get_user_data(e);
    action_user_t act = (action_user_t)(uintptr_t)user_data;

    lv_obj_add_flag(objects.dhome_auser_modal, LV_OBJ_FLAG_HIDDEN);

    switch(act) {
        case UX_ACT_USER_LOGOUT:
            user_mgr_sess_req_logout(callback_logout_users, NULL);
            break;
        case UX_ACT_USER_LOCK:
            ui_screen_change(SCREEN_ID_DS_LOCKED, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, false);
            break;
        case UX_ACT_USER_RESTART:
            break;
        case UX_ACT_USER_POWEROFF:
            break;
        case UX_ACT_USER_NONE:
        default:
            break;
    }
}

static void ux_home__cb_dhome_navegation(lv_event_t *e)
{
    void *user_data = lv_event_get_user_data(e);
    navegation_id_t ID = (uint8_t)(uintptr_t)user_data;
    switch(ID) {
        case UX_NAV_ID_HOME:
            lv_obj_remove_flag(objects.dhome_act_home_cont, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(objects.dhome_act_iot_cont, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(objects.dhome_act_dashboard_cont, LV_OBJ_FLAG_HIDDEN);

            lv_obj_remove_state(objects.dhome_nav_btn_iot, LV_STATE_CHECKED);
            lv_obj_remove_state(objects.dhome_nav_btn_dbd, LV_STATE_CHECKED);
            lv_obj_add_state(objects.dhome_nav_btn_home, LV_STATE_CHECKED);
        break;

        case UX_NAV_ID_IOT:
            lv_obj_add_flag(objects.dhome_act_home_cont, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(objects.dhome_act_iot_cont, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(objects.dhome_act_dashboard_cont, LV_OBJ_FLAG_HIDDEN);

            lv_obj_remove_state(objects.dhome_nav_btn_home, LV_STATE_CHECKED);
            lv_obj_remove_state(objects.dhome_nav_btn_dbd, LV_STATE_CHECKED);
            lv_obj_add_state(objects.dhome_nav_btn_iot, LV_STATE_CHECKED);
        break;

        case UX_NAV_ID_DASHBOARD:
            lv_obj_add_flag(objects.dhome_act_home_cont, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(objects.dhome_act_iot_cont, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(objects.dhome_act_dashboard_cont, LV_OBJ_FLAG_HIDDEN);

            lv_obj_remove_state(objects.dhome_nav_btn_home, LV_STATE_CHECKED);
            lv_obj_remove_state(objects.dhome_nav_btn_iot, LV_STATE_CHECKED);
            lv_obj_add_state(objects.dhome_nav_btn_dbd, LV_STATE_CHECKED);
        break;

        default:
            ESP_LOGI(TAG, "Opción de navegación por Home inválida");
        break;
    }
}


static const char *ui_int__metric_unit_to_str(udc_metric_size_unit_t unit)
{
    switch (unit) {
        case UDC_METRIC_UNIT_BYTES: return "B";
        case UDC_METRIC_UNIT_KB: return "Kb";
        case UDC_METRIC_UNIT_MB: return "Mb";
        case UDC_METRIC_UNIT_GB: return "Gb";
        default: return "B";
    }
}

static void ui_int__cb_metrics_async(const udc_metric_event_t *event, void *user_ctx)
{
    uint8_t ctz = (uint8_t)(uintptr_t)user_ctx;

    if (event->status != UDC_METRIC_OK) {
        ESP_LOGE(TAG, "[%u] Error en lectura. Código de estado: %d", ctz, event->status);
        return;
    }
    switch(event->type) {
        case UDC_METRIC_CPU_USAGE:
            ui_label_set_text_abs(UI_LABEL_ABS_HOME_SYS_CPU, "CPU: %u%%", event->data.cpu.percent_used);
            break;

        case UDC_METRIC_RAM_STATE: {
            const char *unit = ui_int__metric_unit_to_str(event->data.ram.unit);
            switch(ctz) {
                case 1:
                    ui_label_set_text_abs(UI_LABEL_ABS_HOME_SYS_PSRAM, "P: %u / %u %s",
                                          event->data.ram.used,
                                          event->data.ram.total,
                                          unit);
                    break;

                case 2:
                    ui_label_set_text_abs(UI_LABEL_ABS_HOME_SYS_SRAM, "I: %u / %u %s",
                                          event->data.ram.used,
                                          event->data.ram.total,
                                          unit);
                    break;

                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

/* Inicializador de funciones para in y out de eventos para esta display */
static void ui_int__cb_display_home(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_SCREEN_LOADED) {
        for(uint8_t i = 0; i < METRIC_HANDLE_MAX; i++) {
            udc_metrics_set_paused(metrics_handle[i], false);
        }
    } else if(code == LV_EVENT_SCREEN_UNLOAD_START) {
        for(uint8_t i = 0; i < METRIC_HANDLE_MAX; i++) {
            udc_metrics_set_paused(metrics_handle[i], true);
        }
    }
}
void ui_int__act_display_home_start_act(void) {
    ui_pub_actions_backg_update_disp_home(ui_int__sia_images_get_path_by_id(UI_IMAGE_THUNDER_RAIL));

    lv_obj_add_event_cb(objects.dhome_nav_btn_home, ux_home__cb_dhome_navegation, LV_EVENT_CLICKED, (void *)UX_NAV_ID_HOME);
    lv_obj_add_event_cb(objects.dhome_nav_btn_iot, ux_home__cb_dhome_navegation, LV_EVENT_CLICKED, (void *)UX_NAV_ID_IOT);
    lv_obj_add_event_cb(objects.dhome_nav_btn_dbd, ux_home__cb_dhome_navegation, LV_EVENT_CLICKED, (void *)UX_NAV_ID_DASHBOARD);

    lv_obj_add_event_cb(objects.dhome_user_login_cont, ux_home__cb_dhome_action_user_cont, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(objects.dhome_auser_modal, ux_home__cb_dhome_action_user_buttons, LV_EVENT_CLICKED, (void *)UX_ACT_USER_NONE);
    
    lv_obj_add_event_cb(objects.dhome_auser_p_btn_logout, ux_home__cb_dhome_action_user_buttons, LV_EVENT_CLICKED, (void *)UX_ACT_USER_LOGOUT);
    lv_obj_add_event_cb(objects.dhome_auser_p_btn_lock, ux_home__cb_dhome_action_user_buttons, LV_EVENT_CLICKED, (void *)UX_ACT_USER_LOCK);
    lv_obj_add_event_cb(objects.dhome_auser_p_btn_restart, ux_home__cb_dhome_action_user_buttons, LV_EVENT_CLICKED, (void *)UX_ACT_USER_RESTART);
    lv_obj_add_event_cb(objects.dhome_auser_p_btn_poweroff, ux_home__cb_dhome_action_user_buttons, LV_EVENT_CLICKED, (void *)UX_ACT_USER_POWEROFF);

    lv_obj_add_event_cb(objects.dhome_act_home_settings, ux_home__cb_dhome_act_home_settings, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(objects.dhome_act_home_scenes, ux_home__cb_dhome_act_scenes, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(objects.dhome_act_home_zigbee, ux_home__cb_dhome_act_home_zigbee, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(objects.display_home, ui_int__cb_display_home, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(objects.display_home, ui_int__cb_display_home, LV_EVENT_SCREEN_UNLOAD_START, NULL);

    udc_metric_config_t config = {
        .run_on_start = true,
        .enabled_by_default = false,
        .cpu.core_id = -1
    };
    metrics_handle[0] = udc_metrics_subscribe(UDC_METRIC_CPU_USAGE, 1500, 3, &config, ui_int__cb_metrics_async, NULL);

    config.ram.memory_caps = MALLOC_CAP_SPIRAM;
    metrics_handle[1] = udc_metrics_subscribe(UDC_METRIC_RAM_STATE, 300000, 3, &config, ui_int__cb_metrics_async, (void *)1);

    config.ram.memory_caps = MALLOC_CAP_INTERNAL;
    metrics_handle[2] = udc_metrics_subscribe(UDC_METRIC_RAM_STATE, 300000, 4, &config, ui_int__cb_metrics_async, (void *)2);

}