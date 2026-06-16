#include "actions_display_top.h"

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

    // Verificamos que siga expandido antes de contraer
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
    lv_anim_set_values(&a_y, -50, 10);
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

    // Cargar las animaciones en el Timeline
    lv_anim_timeline_add(notch_timeline, 0, &a_min_opa);
    lv_anim_timeline_add(notch_timeline, 150, &a_w);
    lv_anim_timeline_add(notch_timeline, 150, &a_h);
    lv_anim_timeline_add(notch_timeline, 150, &a_y);
    lv_anim_timeline_add(notch_timeline, 450, &a_exp_opa);
    
    // Estado alfa inicial del contenedor expandido
    lv_obj_set_style_opa(objects.dtop_notch_exp_cont, 0, LV_PART_MAIN);

    // ASIGNACIÓN ESTÁTICA ÚNICA DE TIMERS
    auto_close_timer = lv_timer_create(auto_close_cb, 15000, NULL); // Cuenta de 15 segundos
    lv_timer_pause(auto_close_timer);

    restore_click_timer = lv_timer_create(restore_click_cb, 600, NULL); // Bloqueo anti-spam de 600ms
    lv_timer_pause(restore_click_timer);
}

void ui_int__cb_dtop_notch_controller(lv_event_t * e)
{
    if(notch_timeline == NULL) return;

    bool is_expanded = lv_obj_has_state(objects.dtop_notch_cont, LV_STATE_CHECKED);

    if (is_expanded) {
        // Está expandido -> Acción manual de achicar
        lv_timer_pause(auto_close_timer);
        
        lv_obj_remove_state(objects.dtop_notch_cont, LV_STATE_CHECKED);
        lv_anim_timeline_set_reverse(notch_timeline, true);
    } else {
        // Está compacto -> Acción de expandir
        lv_obj_add_state(objects.dtop_notch_cont, LV_STATE_CHECKED);
        lv_anim_timeline_set_reverse(notch_timeline, false);

        lv_timer_reset(auto_close_timer);
        lv_timer_resume(auto_close_timer);
    }

    // anti spam touch
    lv_obj_remove_flag(objects.dtop_notch_cont, LV_OBJ_FLAG_CLICKABLE);

    lv_anim_timeline_start(notch_timeline);

    lv_timer_reset(restore_click_timer);
    lv_timer_resume(restore_click_timer);
}
//********************************************************/
// PUBLIC API INTERNAL
//********************************************************/
void ui_int__act_display_top_tick(void)
{
    init_notch_animations();
    lv_obj_add_event_cb(objects.dtop_notch_cont, ui_int__cb_dtop_notch_controller, LV_EVENT_CLICKED, NULL);
}