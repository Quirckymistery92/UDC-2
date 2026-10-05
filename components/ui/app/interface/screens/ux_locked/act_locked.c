#include "act_locked.h"

/* ESP */
#include "esp_log.h"

/* Módulos */
#include "interface.h"
#include "user_mgr.h"
#include "ui_label_abs.h"
#include "ui_image_sia_internal.h"

//**********************************************/
// Global variables
//**********************************************/

typedef enum {
    DL_UI_NONE = 0,
    DL_UI_USERS = 1,
    DL_UI_UNLOCK_ALL_METHODS = 2,
    DL_UI_UNLOCK_PIN_METHOD = 3,
    DL_UI_UNLOCK_FACEID_METHOD = 4,
    DL_UI_MAX
} dl_action_t;

static const char * const TAG = "ACT_LOCKED";
static lv_obj_t *ui_obj_navegation[DL_UI_MAX];
static uint16_t user_selected_id = 0;
static size_t user_selected_pin_size = 0;
#define LEDS_COUNT 6
static lv_obj_t *leds_pin[LEDS_COUNT];
static uint8_t pin_len = 0;
static char pin_buf[7] = {0};
static lv_timer_t * lock_wakeup_timer = NULL;
static bool user_has_login_lock = false;

//**********************************************/
// Helpers globales
//**********************************************/
static void helper_update_pin_leds(uint8_t len) {
    for (int i = 0; i <= LEDS_COUNT; i++) {
        if (i < len) lv_obj_remove_flag(leds_pin[i], LV_OBJ_FLAG_HIDDEN);
        else         lv_obj_add_flag(leds_pin[i], LV_OBJ_FLAG_HIDDEN);
    }
}

static void helper_clear_pin(void) {
    memset(pin_buf, 0, sizeof(pin_buf));
    pin_len = 0;
    helper_update_pin_leds(0);
}
static esp_err_t ui_state_set_active_user(uint16_t user_id) {
    user_mgr_user_view_t u_get;
    esp_err_t err = user_mgr_user_get_data(user_id, &u_get);
    if(err != ESP_OK) {
        ESP_LOGE("DL_ACTIONS", "Error al cargar datos del usuario ID: %d", user_id);
        return err;
    }

    user_selected_id = user_id;
    user_selected_pin_size = u_get.pin_length;
    
    ui_int__helper_label_abs_apply_sync(UI_LABEL_ABS_LOCKED_USER_SELECTED_NAME, u_get.name);
    
    const lv_img_dsc_t *img_src = ui_int__sia_icons_get_src_by_id(u_get.avatar_id);
    lv_image_set_src(objects.dlock_unlock_user_select_img, img_src);

    ui_int__helper_label_abs_apply_sync(UI_LABEL_ABS_LOCKED_USER_SELECTED_DATA, u_get.role_str);

    ESP_LOGI("DL_ACTIONS", "Usuario Activo -> ID:%d | Nombre:%s", user_id, u_get.name);
    
    return ESP_OK;
}

//**********************************************/
// Pipeline route & user select
//**********************************************/
static void ui_int_helper_obj_disable(lv_obj_t * obj, bool disable_obj, bool thread_safe) {
    if (!obj) return;

    if(thread_safe) {
        if(disable_obj) {
            ui_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            ui_obj_add_state(obj, LV_STATE_DISABLED);
        }else{
            ui_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            ui_obj_remove_state(obj, LV_STATE_DISABLED);
        }
        return;
    }

    if(disable_obj){
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_state(obj, LV_STATE_DISABLED);
    }else{
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_state(obj, LV_STATE_DISABLED);
    }
}

static void ui_router_navigate(dl_action_t current_view, dl_action_t next_view) {
    if(current_view >= DL_UI_MAX || next_view >= DL_UI_MAX) return;

    lv_obj_t *current_obj = ui_obj_navegation[current_view];
    lv_obj_t *next_obj = ui_obj_navegation[next_view];

    if (next_view == DL_UI_USERS) {
        lv_obj_add_flag(objects.dlock_unlock_user_select_cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.dlock_unlock_cont_modal, LV_OBJ_FLAG_HIDDEN);
    } else if (next_view == DL_UI_UNLOCK_ALL_METHODS) {
        lv_obj_remove_flag(objects.dlock_unlock_user_select_cont, LV_OBJ_FLAG_HIDDEN);
    }

    if (current_obj != NULL) lv_obj_add_flag(current_obj, LV_OBJ_FLAG_HIDDEN);
    if (next_obj != NULL) lv_obj_remove_flag(next_obj, LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_user_data(objects.dlock_unlock_btn_back, (void*)(uintptr_t)next_view);
}

static void ui_int__cb_dl_home_lock_act_transitions(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        bool temp_state = false;
        esp_err_t ret = user_mgr_sess_has_active_user_sync(&temp_state);
        if(ret == ESP_OK) {
            if(temp_state) {
                uint16_t id = user_mgr_sess_get_active();
                esp_err_t ret = ui_state_set_active_user(id);
                if(ret == ESP_OK){
                    user_has_login_lock = temp_state;
                    lv_obj_remove_flag(objects.dlock_unlock_cont_modal, LV_OBJ_FLAG_HIDDEN);
                    ui_router_navigate(DL_UI_NONE, DL_UI_UNLOCK_ALL_METHODS);
                    return;
                }
            }
        }

        lv_obj_scroll_to_view(objects.dlock_unlock_userpick_1_cont, LV_ANIM_ON);
        ui_router_navigate(DL_UI_NONE, DL_UI_USERS);
    }else if (code == LV_EVENT_LONG_PRESSED) {
        const user_mgr_user_create_t new_user = {
            .avatar_id = UI_ICON_USER_LOGO_ROBOT,
            .face_data  = NULL,
            .name = "Owner",
            .permission = USER_MGR_PERM_ADMIN,
            .pin = "101010"
        };

        esp_err_t ret = user_mgr_user_req_create(&new_user, NULL, NULL);
        if(ret != ESP_OK) {
            ESP_LOGI(TAG, "Falló al crear el usuario");
        }
    }
}

static void ui_int__cb_Dl_home_unlock_btn_back(lv_event_t * e) {   
    dl_action_t current = (dl_action_t)(uintptr_t)lv_obj_get_user_data(lv_event_get_target(e));
    if(current >= DL_UI_MAX) return;
    
    switch(current) {
        case DL_UI_USERS:
            lv_obj_add_flag(objects.dlock_unlock_cont_modal, LV_OBJ_FLAG_HIDDEN);
            ui_router_navigate(current, DL_UI_NONE);
            break;
        case DL_UI_UNLOCK_ALL_METHODS: {
            if(user_has_login_lock) {
                lv_obj_add_flag(objects.dlock_unlock_cont_modal, LV_OBJ_FLAG_HIDDEN);
                ui_router_navigate(current, DL_UI_NONE);
                break;
            }
            ui_router_navigate(current, DL_UI_USERS);
            break;
        }
        case DL_UI_UNLOCK_PIN_METHOD:
        case DL_UI_UNLOCK_FACEID_METHOD:
            ui_router_navigate(current, DL_UI_UNLOCK_ALL_METHODS);
            helper_clear_pin();
            ui_int__helper_label_abs_apply_sync(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "");
            ui_int_helper_obj_disable(objects.dlock_unlock_pin_keyboard, false, false);

            if(lock_wakeup_timer != NULL) {
                lv_timer_pause(lock_wakeup_timer); //esta llamada es interna sync. por las dudas porque las otras llamadas del timer son con ui_ de prefix.
            }
            break;
        default: break;
    }
}

static lv_obj_t* ui_picker_get_centered_item(lv_obj_t *parent) {
    lv_area_t parent_coords;
    lv_obj_get_coords(parent, &parent_coords);
    int32_t parent_center = (parent_coords.y1 + parent_coords.y2) / 2;
    
    lv_obj_t *closest_child = NULL;
    int32_t min_dist = INT32_MAX;
    uint32_t child_cnt = lv_obj_get_child_count(parent);

    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i);

        if(lv_obj_has_flag(child, LV_OBJ_FLAG_HIDDEN)) continue;

        lv_area_t child_coords;
        lv_obj_get_coords(child, &child_coords);
        
        int32_t child_center = (child_coords.y1 + child_coords.y2) / 2;
        int32_t dist = LV_ABS(child_center - parent_center);

        if(dist < min_dist) {
            min_dist = dist;
            closest_child = child;
        }
    }
    return closest_child;
}

static void ui_int__cb_dl_home_unlock_userpick_m_cont(lv_event_t *e) {
    lv_obj_t *parent = lv_event_get_target(e);
    lv_obj_t *centered = ui_picker_get_centered_item(parent);
    
    if(centered == NULL) return;

    uint32_t child_cnt = lv_obj_get_child_count(parent);
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_remove_state(lv_obj_get_child(parent, i), LV_STATE_CHECKED);
    }
    lv_obj_add_state(centered, LV_STATE_CHECKED);

    lv_area_t parent_coords, child_coords;
    lv_obj_get_coords(parent, &parent_coords);
    lv_obj_get_coords(centered, &child_coords);
    
    int32_t parent_center = (parent_coords.y1 + parent_coords.y2) / 2;
    int32_t child_center = (child_coords.y1 + child_coords.y2) / 2;
    
    int32_t diff = child_center - parent_center;
    
    if(LV_ABS(diff) <= 1) return;

    lv_coord_t current_scroll = lv_obj_get_scroll_y(parent);
    lv_obj_scroll_to_y(parent, current_scroll + diff, LV_ANIM_ON);
}
static void ui_int__cb_dl_home_unlock_userpick_X(lv_event_t *e) {
    lv_obj_t *current_target = lv_event_get_current_target(e);
    lv_obj_t *parent = lv_obj_get_parent(current_target);
    
    lv_obj_t *centered_obj = ui_picker_get_centered_item(parent);

    if(current_target == centered_obj) {
        void * user_data_id = lv_obj_get_user_data(current_target);
        uint16_t user_id = (uint16_t)(uintptr_t)user_data_id;
        if (ui_state_set_active_user(user_id) == ESP_OK) {
            user_has_login_lock = false;
            ui_router_navigate(DL_UI_USERS, DL_UI_UNLOCK_ALL_METHODS);
        }
    } else {
        lv_obj_scroll_to_view(current_target, LV_ANIM_ON); 
    }
}

static void ui_int__cb_dl_home_unlock_go_btn(lv_event_t *e) {
    dl_action_t btn_id = (dl_action_t)(uintptr_t)lv_event_get_user_data(e);
    if(btn_id >= DL_UI_MAX) return;

    if (user_mgr_auth_get_lock_temp_isLocked(user_selected_id)) {
        ESP_LOGI(TAG, "Acceso denegado temporal o permanentemente por backend de Auth: %s", user_mgr_auth_get_lock_msg(user_selected_id));
        ui_int__dtop_notify_user_show_fmt(true, "El usuario seleccionado no puede autenticarse porque está bloqueado. Tiempo de bloqueo: %s", user_mgr_auth_get_lock_msg(user_selected_id));
        return;
    }

    if (btn_id == DL_UI_UNLOCK_PIN_METHOD) {
        helper_clear_pin();
        ui_int__helper_label_abs_apply_sync(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "Digite su clave");
    }

    ui_router_navigate(DL_UI_UNLOCK_ALL_METHODS, btn_id);
}

//**********************************************/
// Control de PIN Method
//**********************************************/
static void ui_int__cb_lock_wakeup_timer(lv_timer_t * timer) {
    lv_timer_pause(timer);
    
    ui_int_helper_obj_disable(objects.dlock_unlock_pin_keyboard, false, false);
    ui_label_set_text_abs(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "Digite su clave");
    helper_clear_pin();
    
    ESP_LOGI(TAG, "Tiempo cumplido. Teclado rehabilitado para desbloqueo de usuario.");
}

static void user_mgr_check_login_async(user_mgr_event_t event, uint16_t user_id, void *ctx) {
    switch(event){
        case USER_MGR_EVT_SUCCESS:
            break;
            
        case USER_MGR_EVT_ERR_WRONG_PIN:
            ui_label_set_text_abs(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "Clave incorrecta, intentalo de nuevo.");
            break;
            
        case USER_MGR_EVT_ERR_TEMP_LOCKED:{
            uint32_t wait_ms = user_mgr_auth_get_lock_remaining_ms(user_id);

            ui_label_set_text_abs(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "Demasiados intentos fallidos, intentar en %s.", user_mgr_auth_get_lock_msg(user_id));
            ui_int_helper_obj_disable(objects.dlock_unlock_pin_keyboard, true, true);
            if(lock_wakeup_timer != NULL && wait_ms > 0) {
                ui_timer_set_period(lock_wakeup_timer, wait_ms); //Llamadas Async
                ui_timer_reset(lock_wakeup_timer);
                ui_timer_control(lock_wakeup_timer, false);
            }
            break;
        }
            
        case USER_MGR_EVT_ERR_LOCKED:
            ui_label_set_text_abs(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "Usuario bloqueado.");
            ui_int_helper_obj_disable(objects.dlock_unlock_pin_keyboard, true, true);
            break;
            
        case USER_MGR_EVT_ERR_NOT_FOUND:
            ui_label_set_text_abs(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "El usuario seleccionado no es valido.");
            ui_int_helper_obj_disable(objects.dlock_unlock_pin_keyboard, true, true);
            break;
            
        case USER_MGR_EVT_ERR_DISABLED:
            ui_label_set_text_abs(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "El usuario seleccionado está desactivado.");
            ui_int_helper_obj_disable(objects.dlock_unlock_pin_keyboard, true, true);
            break;
            
        default:
            break;
    }
}
static void callback_user_mgr_checking_pass(user_mgr_event_t event, uint16_t user_id, void *ctx)
{
    if(event == USER_MGR_EVT_SUCCESS) {
        ui_screen_change(SCREEN_ID_DS_HOME, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, false);
        return;
    }
    user_mgr_check_login_async(event, user_id, ctx);
}
static void helper_check_pin(const char *input) {
    if(strlen(input) != user_selected_pin_size) return;
    esp_err_t ret = ESP_OK;
    if(user_has_login_lock) {
        ret = user_mgr_auth_req_validate_pin(user_selected_id, input, callback_user_mgr_checking_pass, NULL);
    }else {
        ret = user_mgr_sess_req_login(user_selected_id, input, user_mgr_check_login_async, NULL);
    }
    
    helper_clear_pin();
    if(ret != ESP_OK){
        ESP_LOGI(TAG, "Falló al enviar en cola un request login");
        ui_int__dtop_notify_user_show_fmt(true, "Ocurrió un error interno, vuelve a intentar (FC: %s)", __func__);
    }
}

static void ui_int__cb_dl_home_unlock_pin_keyboard(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_target_obj(e);
    
    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t btn_id = lv_buttonmatrix_get_selected_button(kb);
        const char *txt = lv_buttonmatrix_get_button_text(kb, btn_id);

        if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) {
            if (pin_len > 0) pin_buf[--pin_len] = '\0';
        } else if (pin_len < 6) {
            pin_buf[pin_len++] = txt[0];
            pin_buf[pin_len] = '\0';
        }
        helper_update_pin_leds(pin_len);
        lv_obj_send_event(kb, LV_EVENT_READY, NULL);
    }
    if (code == LV_EVENT_READY) {
        helper_check_pin(pin_buf);
    }
}

static void ui_int__cb_ds_locked(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SCREEN_UNLOADED) {
        for(int i = 0; i < LEDS_COUNT; i++) {
            if(leds_pin[i]) lv_obj_add_flag(leds_pin[i], LV_OBJ_FLAG_HIDDEN);
        }

        for(int i = 0; i < DL_UI_MAX; i++) {
            if(i == DL_UI_NONE) {
                 if(ui_obj_navegation[i]) lv_obj_remove_flag(ui_obj_navegation[i], LV_OBJ_FLAG_HIDDEN);
                continue;
            }

            if(ui_obj_navegation[i]) lv_obj_add_flag(ui_obj_navegation[i], LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_add_flag(objects.dlock_unlock_cont_modal, LV_OBJ_FLAG_HIDDEN);

        user_selected_id = 0;
        user_selected_pin_size = 0;
        memset(pin_buf, 0, sizeof(pin_buf));
        pin_len = 0;
    }
}
static void ui_int__cb_dlocked_user_change_explicit(lv_event_t *e)
{
    dl_action_t current = (dl_action_t)(uintptr_t)lv_obj_get_user_data(objects.dlock_unlock_btn_back);
    if(current >= DL_UI_MAX) return;
    ui_router_navigate(current, DL_UI_USERS);
}
//**********************************************/
// Display Locked tick start.
//**********************************************/

void ui_int__act_display_locked_start_act(void) {
    ui_obj_navegation[DL_UI_NONE] = objects.dlock_home_lock_cont;
    ui_obj_navegation[DL_UI_USERS] = objects.dlock_unlock_userpick_m_cont;
    ui_obj_navegation[DL_UI_UNLOCK_ALL_METHODS] = objects.dlock_unlock_methods_cont;
    ui_obj_navegation[DL_UI_UNLOCK_PIN_METHOD] = objects.dlock_unlock_pin_cont;
    ui_obj_navegation[DL_UI_UNLOCK_FACEID_METHOD] = objects.dlock_unlock_faceid_cont;

    if(lock_wakeup_timer == NULL) {
    // Lo creamos inactivo (pausado)
        lock_wakeup_timer = lv_timer_create(ui_int__cb_lock_wakeup_timer, 1000, NULL);
        lv_timer_pause(lock_wakeup_timer);
    }

    lv_obj_add_event_cb(objects.dlock_home_lock_cont, ui_int__cb_dl_home_lock_act_transitions, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(objects.dlock_home_lock_cont, ui_int__cb_dl_home_lock_act_transitions, LV_EVENT_LONG_PRESSED, NULL);

    lv_obj_add_event_cb(objects.dlock_unlock_btn_back, ui_int__cb_Dl_home_unlock_btn_back, LV_EVENT_CLICKED, NULL);
    
    lv_obj_add_event_cb(objects.dlock_unlock_userpick_cont, ui_int__cb_dl_home_unlock_userpick_m_cont, LV_EVENT_SCROLL_END, NULL);

    lv_obj_t *users[] = {
        objects.dlock_unlock_userpick_1_cont, objects.dlock_unlock_userpick_2_cont,
        objects.dlock_unlock_userpick_3_cont, objects.dlock_unlock_userpick_4_cont,
        objects.dlock_unlock_userpick_5_cont, objects.dlock_unlock_userpick_6_cont
    };
    for(int i = 0; i < 6; i++) {
        lv_obj_add_event_cb(users[i], ui_int__cb_dl_home_unlock_userpick_X, LV_EVENT_CLICKED, NULL);
    }
    
    lv_obj_add_event_cb(objects.dlock_unlock_methods_faceid_cont, ui_int__cb_dl_home_unlock_go_btn, LV_EVENT_CLICKED, (void*)(uintptr_t)DL_UI_UNLOCK_FACEID_METHOD);
    lv_obj_add_event_cb(objects.dlock_unlock_methods_pin_cont, ui_int__cb_dl_home_unlock_go_btn, LV_EVENT_CLICKED, (void*)(uintptr_t)DL_UI_UNLOCK_PIN_METHOD);

    lv_obj_add_event_cb(objects.dlock_unlock_pin_keyboard, ui_int__cb_dl_home_unlock_pin_keyboard, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(objects.dlock_unlock_pin_keyboard, ui_int__cb_dl_home_unlock_pin_keyboard, LV_EVENT_READY, NULL);
    
    lv_obj_add_event_cb(objects.display_lock, ui_int__cb_ds_locked, LV_EVENT_SCREEN_UNLOADED, NULL);
    
    leds_pin[0] = objects.dlock_unlock_pin_input_led1;
    leds_pin[1] = objects.dlock_unlock_pin_input_led2;
    leds_pin[2] = objects.dlock_unlock_pin_input_led3;
    leds_pin[3] = objects.dlock_unlock_pin_input_led4;
    leds_pin[4] = objects.dlock_unlock_pin_input_led5;
    leds_pin[5] = objects.dlock_unlock_pin_input_led6;
    
    lv_obj_scroll_to_view(objects.dlock_unlock_userpick_1_cont, LV_ANIM_OFF);
    lv_obj_add_state(objects.dlock_unlock_userpick_1_cont, LV_STATE_CHECKED);

    lv_obj_add_event_cb(objects.dlock_unlock_user_select_cont, ui_int__cb_dlocked_user_change_explicit, LV_EVENT_CLICKED, NULL);
    
    ui_pub_actions_backg_update_disp_lock(ui_int__sia_images_get_path_by_id(UI_IMAGE_THUNDER_RAIL));
}