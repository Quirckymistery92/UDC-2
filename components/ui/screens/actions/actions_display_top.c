#include "actions_display_top.h"
#include "ui_label_abs.h"
#include "ui_image_sia_internal.h"
#include "actions_control.h"
#include "esp_log.h"

#define TAG "UI_INT_DTP"
//********************************************************/
// Modal, paneles y popups.
//********************************************************/
static uint8_t s_modal_counter = 0;
static bool is_in_use_popup = false;
static bool is_in_use_ctrl_pnael = false;

void modal_acquire(void) {
    if(s_modal_counter == 0) {
        lv_obj_remove_flag(objects.dtop_modal_cont, LV_OBJ_FLAG_HIDDEN);
    }
    s_modal_counter++;
}
void modal_release(void) {
    if(s_modal_counter == 0) return;
    s_modal_counter--;
    if(s_modal_counter == 0) lv_obj_add_flag(objects.dtop_modal_cont, LV_OBJ_FLAG_HIDDEN);
}
void ui_int__dtop_show_control_panel(bool show) {
    if(show) {
        if(is_in_use_ctrl_pnael) return;
        modal_acquire();
        is_in_use_ctrl_pnael = true;
        lv_obj_remove_flag(objects.dtop_modal_panelctl, LV_OBJ_FLAG_HIDDEN);
    } else {
        if(!is_in_use_ctrl_pnael) return;
        modal_release();
        is_in_use_ctrl_pnael = false;
        lv_obj_add_flag(objects.dtop_modal_panelctl, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_int__cb_panelctrl_exit(lv_event_t * e) {
    ui_int__dtop_show_control_panel(false);
}

void ui_int__cb_panelctrl_show(lv_event_t * e) {
    ui_int__dtop_show_control_panel(true);
}
void ui_int__cb_btn_action(lv_event_t * e) { //Callback para BTN RUN y BTN CANCEL
    lv_obj_t *obj = lv_event_get_target(e);

    modal_release(); // Liberar el modal y salir en ambos contextos de botones
    is_in_use_popup = false;
    lv_obj_add_flag(objects.dtop_modal_pup_cont, LV_OBJ_FLAG_HIDDEN);

    if(obj == objects.dtop_modal_pup_tit_btn_run) { // Distingue unicamente la ejecución del callback
        ui_int__dtop_popup_callback_t callback = (ui_int__dtop_popup_callback_t)lv_obj_get_user_data(obj);
        if(callback != NULL) callback();
    }
}

esp_err_t ui_int__dtop_show_popup(ui_int__dtop_popup_configure_t config) {
    
    if(config.message == NULL || config.tag == NULL ||
        config.btn_type > dtop_POPUP_MAX_BTN ||
        config.type > dtop_POPUP_MAX_TYPE) return ESP_ERR_INVALID_ARG;

    if(is_in_use_popup) return ESP_ERR_INVALID_STATE;

    /* Control de tipo */
    const char * title = NULL;
    const lv_image_dsc_t * icon_src = NULL;
    switch(config.type) {
        case dtopPOPUP_INFO:
            icon_src = images_ctl_internal_get_src(UI_ICON_INFORMATION30);
            title = "Información";
            break;
        case dtopPOPUP_WARN:
            icon_src = images_ctl_internal_get_src(UI_ICON_WARNING30);
            title = "Advertencia";
            break;
        default:
            break;
    };

    /* Control de botones */
    const char * btn_run_text = NULL;
    const char * btn_ext_text = NULL;
    switch(config.btn_type) {
        case dtopPOPUP_BTN_OK_AND_EXT:
            btn_run_text = "Aceptar";
            btn_ext_text = "Cancelar";
            break;
        case dtopPOPUP_BTN_RST_AND_EXT:
            btn_run_text = "Restablecer";
            btn_ext_text = "Cancelar";
            break;
        case dtop_POPUP_BTN_ONLY_ACPT:
            btn_run_text = "Aceptar";
            btn_ext_text = "";
            break;
        default:
            break;
    }

    /* Control de boton ONLY_ACCEPT */
    if(config.btn_type == dtop_POPUP_BTN_ONLY_ACPT) {
        lv_obj_add_flag(objects.dtop_modal_pup_tit_btn_cancel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(objects.dtop_modal_pup_tit_btn_cancel, LV_OBJ_FLAG_HIDDEN);
    }

    /* ===== Aplicador ===== */

    // Íconos y texto del título.
    lv_image_set_src(objects.dtop_modal_pup_tit_img1, icon_src);
    lv_image_set_src(objects.dtop_modal_pup_tit_img2, icon_src);
    lv_label_set_text_static(objects.dtop_modal_pup_tit_title, title);

    // Texto del tag (dueño del mensaje) y el mensaje.
    ui_int__helper_label_abs_apply_sync(UI_LABEL_ABS_DTOP_POPUP_TAG, config.tag);
    ui_int__helper_label_abs_apply_sync(UI_LABEL_ABS_DTOP_POPUP_MESSAGE, config.message);

    // Texto de los botones.
    lv_label_set_text_static(objects.dtop_modal_pup_tit_btn_run_lbl, btn_run_text);
    lv_label_set_text_static(objects.dtop_modal_pup_tit_btn_cancel_lbl, btn_ext_text);

    /* Le enviamos el callback al btn de run o aceptar */
    if(config.btn_run != NULL) lv_obj_set_user_data(objects.dtop_modal_pup_tit_btn_run, (void *)config.btn_run);
    else
    lv_obj_set_user_data(objects.dtop_modal_pup_tit_btn_run, NULL);
    
    modal_acquire();
    lv_obj_remove_flag(objects.dtop_modal_pup_cont, LV_OBJ_FLAG_HIDDEN);
    
    is_in_use_popup = true;

    return ESP_OK;
}
//********************************************************/
// Modal, paneles y popups END.
//********************************************************/

//********************************************************/
// Notch controller
//********************************************************/
lv_anim_timeline_t * notch_timeline = NULL;
lv_timer_t * auto_close_timer = NULL;
lv_timer_t * restore_click_timer = NULL;

static void set_opa_cb(void * obj, int32_t v) {
    lv_obj_set_style_opa((lv_obj_t *)obj, v, LV_PART_MAIN);
}

// Callback restore interactivity
static void restore_click_cb(lv_timer_t * t) {
    lv_obj_add_flag(objects.dtop_notch_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_timer_pause(t);
}

// Callback para el auto-cierre
static void auto_close_cb(lv_timer_t * t) {
    lv_timer_pause(t);

    if (lv_obj_has_state(objects.dtop_notch_cont, LV_STATE_CHECKED)) {
        lv_obj_remove_state(objects.dtop_notch_cont, LV_STATE_CHECKED);
        lv_obj_remove_flag(objects.dtop_notch_cont, LV_OBJ_FLAG_CLICKABLE);
        
        lv_anim_timeline_set_reverse(notch_timeline, true);
        lv_anim_timeline_start(notch_timeline);

        lv_timer_reset(restore_click_timer);
        lv_timer_resume(restore_click_timer);
    }
}

void init_notch_animations(void) {
    notch_timeline = lv_anim_timeline_create();

    // 1. FADE OUT DEL CONTENIDO COMPACTO (0 ms a 150 ms)
    lv_anim_t a_min_opa;
    lv_anim_init(&a_min_opa);
    lv_anim_set_var(&a_min_opa, objects.dtop_notch_min_cont);
    lv_anim_set_values(&a_min_opa, 255, 0);
    lv_anim_set_time(&a_min_opa, 150);
    lv_anim_set_exec_cb(&a_min_opa, set_opa_cb);

    // 2. EXPANSIÓN DEL NOTCH (150 ms a 450 ms) - Efecto Burbuja Cálida
    lv_anim_t a_w;
    lv_anim_init(&a_w);
    lv_anim_set_var(&a_w, objects.dtop_notch_cont);
    lv_anim_set_values(&a_w, 250, 360);
    lv_anim_set_time(&a_w, 300);
    lv_anim_set_exec_cb(&a_w, (lv_anim_exec_xcb_t)lv_obj_set_width);
    lv_anim_set_path_cb(&a_w, lv_anim_path_overshoot);

    lv_anim_t a_h;
    lv_anim_init(&a_h);
    lv_anim_set_var(&a_h, objects.dtop_notch_cont);
    lv_anim_set_values(&a_h, 80, 220);
    lv_anim_set_time(&a_h, 300);
    lv_anim_set_exec_cb(&a_h, (lv_anim_exec_xcb_t)lv_obj_set_height);
    lv_anim_set_path_cb(&a_h, lv_anim_path_overshoot);

    lv_anim_t a_y;
    lv_anim_init(&a_y);
    lv_anim_set_var(&a_y, objects.dtop_notch_cont);
    lv_anim_set_values(&a_y, -35, 10);
    lv_anim_set_time(&a_y, 300);
    lv_anim_set_exec_cb(&a_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a_y, lv_anim_path_overshoot);

    // 3. FADE IN DEL CONTENIDO EXPANDIDO (450 ms a 600 ms)
    lv_anim_t a_exp_opa;
    lv_anim_init(&a_exp_opa);
    lv_anim_set_var(&a_exp_opa, objects.dtop_notch_exp_cont);
    lv_anim_set_values(&a_exp_opa, 0, 255);
    lv_anim_set_time(&a_exp_opa, 150);
    lv_anim_set_exec_cb(&a_exp_opa, set_opa_cb);

    lv_anim_timeline_add(notch_timeline, 0, &a_min_opa);
    lv_anim_timeline_add(notch_timeline, 150, &a_w);
    lv_anim_timeline_add(notch_timeline, 150, &a_h);
    lv_anim_timeline_add(notch_timeline, 150, &a_y);
    lv_anim_timeline_add(notch_timeline, 450, &a_exp_opa);
    
    lv_obj_set_style_opa(objects.dtop_notch_exp_cont, 0, LV_PART_MAIN);

    auto_close_timer = lv_timer_create(auto_close_cb, 15000, NULL);
    lv_timer_pause(auto_close_timer);

    restore_click_timer = lv_timer_create(restore_click_cb, 600, NULL);
    lv_timer_pause(restore_click_timer);
}

void ui_int__cb_dtop_notch_controller(lv_event_t * e)
{
    if(notch_timeline == NULL) return;

    bool is_expanded = lv_obj_has_state(objects.dtop_notch_cont, LV_STATE_CHECKED);

    if (is_expanded) {
        lv_timer_pause(auto_close_timer);
        
        lv_obj_remove_state(objects.dtop_notch_cont, LV_STATE_CHECKED);
        lv_anim_timeline_set_reverse(notch_timeline, true);
    } else {
        lv_obj_add_state(objects.dtop_notch_cont, LV_STATE_CHECKED);
        lv_anim_timeline_set_reverse(notch_timeline, false);

        lv_timer_reset(auto_close_timer);
        lv_timer_resume(auto_close_timer);
    }

    lv_obj_remove_flag(objects.dtop_notch_cont, LV_OBJ_FLAG_CLICKABLE);

    lv_anim_timeline_start(notch_timeline);

    lv_timer_reset(restore_click_timer);
    lv_timer_resume(restore_click_timer);
}
//********************************************************/
// END Notch Controller END
//********************************************************/

//********************************************************/
// Notifications Fast Information - Reemplazo Inmediato (Preempción)
//********************************************************/

static lv_anim_timeline_t *notif_timeline = NULL;
static lv_timer_t *notif_entry_timer = NULL;   // 600ms para revelar contenido al llegar a -3
static lv_timer_t *notif_read_timer = NULL;    // 7000ms de lectura para el usuario
static lv_timer_t *notif_exit_timer = NULL;    // 600ms para retracción completa

// Callback del timer de entrada (llega a -3 a los 600ms)
static void anim_notify_user_entry_timer_cb(lv_timer_t * timer) {
    lv_timer_pause(timer);
    // Ahora que está en posición y estático, revelamos el texto e imagen de forma segura
    lv_obj_remove_flag(objects.dtop_notifiy_message, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(objects.dtop_notifiy_img, LV_OBJ_FLAG_HIDDEN);
    
    // Iniciamos el conteo de lectura de 7 segundos
    lv_timer_reset(notif_read_timer);
    lv_timer_resume(notif_read_timer);
}

// Callback del timer de lectura (pasan 7 segundos)
static void anim_notify_user_read_timer_cb(lv_timer_t * timer) {
    lv_timer_pause(timer);
    
    // Ocultamos texto e imagen antes de la retracción para evitar recálculos de layout
    lv_obj_add_flag(objects.dtop_notifiy_message, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(objects.dtop_notifiy_img, LV_OBJ_FLAG_HIDDEN);
    
    if(notif_timeline) {
        lv_anim_timeline_set_reverse(notif_timeline, true);
        lv_anim_timeline_start(notif_timeline);
        
        lv_timer_reset(notif_exit_timer);
        lv_timer_resume(notif_exit_timer);
    }
}

// Callback del timer de salida (terminó de retraerse a 270)
static void anim_notify_user_exit_timer_cb(lv_timer_t * timer) {
    lv_timer_pause(timer);
}

// Click del usuario: extiende el tiempo de lectura si se hace click
static void anim_notify_user_cb_dtop_notifiy_cont(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        if(notif_read_timer && !lv_timer_get_paused(notif_read_timer)) {
            lv_timer_reset(notif_read_timer);
        }
    }
}

void init_anim_notify_user_timeline(void) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, objects.dtop_notifiy_cont);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_values(&a, 270, -3);
    lv_anim_set_time(&a, 600);
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    
    notif_timeline = lv_anim_timeline_create();
    lv_anim_timeline_add(notif_timeline, 0, &a);

    notif_entry_timer = lv_timer_create(anim_notify_user_entry_timer_cb, 600, NULL);
    lv_timer_pause(notif_entry_timer);

    notif_read_timer = lv_timer_create(anim_notify_user_read_timer_cb, 7000, NULL);
    lv_timer_pause(notif_read_timer);

    notif_exit_timer = lv_timer_create(anim_notify_user_exit_timer_cb, 600, NULL);
    lv_timer_pause(notif_exit_timer);
}
esp_err_t ui_int__anim_notify_user_show_fmt(bool is_alert, const char *fmt, ...)
{
    if(fmt == NULL) return ESP_ERR_INVALID_ARG;

    va_list args;
    va_start(args, fmt);
    char text_static[256];
    int len = vsnprintf(text_static, sizeof(text_static), fmt, args);
    va_end(args);

    if(len < 0) return ESP_ERR_INVALID_ARG;
    else if (len >= sizeof(text_static)) {
        ESP_LOGW(TAG, "Texto de notificación truncado en %s", __func__);
        return ESP_ERR_NO_MEM;
    }

    return ui_int__anim_notify_user_show(is_alert, text_static);
}

esp_err_t ui_int__anim_notify_user_show(bool is_alert, const char *text)
{
    if(text == NULL) return ESP_ERR_INVALID_ARG;

    // Pre-imposición inmediata
    lv_timer_pause(notif_entry_timer);
    lv_timer_pause(notif_read_timer);
    lv_timer_pause(notif_exit_timer);

    lv_obj_add_flag(objects.dtop_notifiy_message, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(objects.dtop_notifiy_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_x(objects.dtop_notifiy_cont, 270);

    // El texto que se aplica acá tiene un límite de 256 bytes, igual que el text_static
    ui_int__helper_label_abs_apply_sync(UI_LABEL_ABS_DTOP_NOTIFY_USER, text);
    
    const lv_image_dsc_t *img = is_alert ? 
        images_ctl_internal_get_src(UI_ICON_WARNING30) : 
        images_ctl_internal_get_src(UI_ICON_INFORMATION30);

    if(objects.dtop_notifiy_img && img != NULL) {
        lv_image_set_src(objects.dtop_notifiy_img, img);
    }

    lv_anim_timeline_set_reverse(notif_timeline, false);
    lv_anim_timeline_set_progress(notif_timeline, 0);
    lv_anim_timeline_start(notif_timeline);

    lv_timer_reset(notif_entry_timer);
    lv_timer_resume(notif_entry_timer);
    return ESP_OK;
}
//********************************************************/
// END Notifications Fast Information END
//********************************************************/

//********************************************************/
// PUBLIC API INTERNAL
//********************************************************/
void ui_int__act_display_top_start_act(void)
{
    init_notch_animations();
    lv_obj_add_event_cb(objects.dtop_notch_cont, ui_int__cb_dtop_notch_controller, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(objects.dtop_modal_pup_tit_btn_run, ui_int__cb_btn_action, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(objects.dtop_modal_pup_tit_btn_cancel, ui_int__cb_btn_action, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(objects.dtop_modal_panelctl_btni_exit, ui_int__cb_panelctrl_exit, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(objects.dtop_notch_exp_cpanel_btn, ui_int__cb_panelctrl_show, LV_EVENT_CLICKED, NULL);

    init_anim_notify_user_timeline();
    lv_obj_add_event_cb(objects.dtop_notifiy_cont, anim_notify_user_cb_dtop_notifiy_cont, LV_EVENT_CLICKED, NULL);
}