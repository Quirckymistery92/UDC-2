#include "actions_display_locked.h"
#include "actions_control.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "sys_module.h"
#include "ui_label_abs.h"
#include "ui_image_sia_internal.h"

//**********************************************/
// Global variables
//**********************************************/
#define ADL_STRINGS_USER_DISABLED "El usuario seleccionado está inhabilitado."

typedef enum {
    DL_UI_NONE,
    DL_UI_USERS,
    DL_UI_UNLOCK_ALL_METHODS,
    DL_UI_UNLOCK_PIN_METHOD,
    DL_UI_UNLOCK_FACEID_METHOD,
    DL_UI_MAX
} dl_action_t;
static const char *TAG = "UI Internal";
static lv_obj_t *ui_obj_navegation[DL_UI_MAX];
static SemaphoreHandle_t action_mutex = NULL;

static uint16_t user_selected_id = 0;
static size_t user_selected_pin_size = 0;
static char pin_buf[7] = {0};
static uint8_t pin_len = 0;
#define LEDS_COUNT 6
static lv_obj_t *leds_pin[LEDS_COUNT];
//**********************************************/
// 1. Display Auto Clear
//**********************************************/
static void ui_int_helper_obj_disable(lv_obj_t * obj, bool disable_obj, bool thread_safe) {
    if (!obj) return;

    if(thread_safe) {
        if(!disable_obj) {
            ui_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            ui_obj_add_state(obj, LV_STATE_DISABLED);
        }else{
            ui_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            ui_obj_remove_state(obj, LV_STATE_DISABLED);
        }
        return;
    }

    if(!disable_obj){
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_state(obj, LV_STATE_DISABLED);
    }else{
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_state(obj, LV_STATE_DISABLED);
    }
}
//**********************************************/
// 1. Display Auto Clear
//**********************************************/
static void ui_int__cb_ds_locked(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SCREEN_UNLOADED) {
        for(int i = 0; i < LEDS_COUNT; i++) {
            if(leds_pin[i]) lv_obj_add_flag(leds_pin[i], LV_OBJ_FLAG_HIDDEN);
        }

        for(int i = 0; i < DL_UI_MAX; i++) {
            if(ui_obj_navegation[i]) lv_obj_remove_flag(ui_obj_navegation[i], LV_OBJ_FLAG_HIDDEN);
        }

        user_selected_id = 0;
        user_selected_pin_size = 0;
        memset(pin_buf, 0, sizeof(pin_buf));
        pin_len = 0;
    }

}

//**********************************************/
// 1. MOTOR DEL SELECTOR (Picker Engine)
//**********************************************/

/* Devuelve el objeto hijo que se encuentra físicamente más cerca del centro de su padre */
static lv_obj_t* ui_picker_get_centered_item(lv_obj_t *parent) {
    lv_area_t parent_coords;
    lv_obj_get_coords(parent, &parent_coords);
    int32_t parent_center = (parent_coords.y1 + parent_coords.y2) / 2;
    
    lv_obj_t *closest_child = NULL;
    int32_t min_dist = INT32_MAX;
    uint32_t child_cnt = lv_obj_get_child_count(parent);

    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i);
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

//**********************************************/
// 2. GESTOR DE ESTADO (State Manager)
//**********************************************/

/* Extrae el ID, consulta la DB y actualiza la UI visual del usuario seleccionado */
static esp_err_t ui_state_set_active_user(lv_obj_t *user_cont) {
    void * user_data_id = lv_obj_get_user_data(user_cont);
    uint16_t user_id = (uint16_t)(uintptr_t)user_data_id;


    sys_id_user_view_t u_get;
    esp_err_t err = sys_id_user_get_data(user_id, &u_get);
    if(err != ESP_OK) {
        ESP_LOGE("DL_ACTIONS", "Error al cargar datos del usuario ID: %d", user_id);
        return err;
    }

    ui_int_helper_obj_disable(objects.dl_home_unlock_pin_keyboard, u_get.is_enabled, false);
    if(!u_get.is_enabled) ui_int__helper_label_abs_apply_sync(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, ADL_STRINGS_USER_DISABLED);

    user_selected_id = user_id;
    user_selected_pin_size = u_get.pin_length;
    
    ui_int__helper_label_abs_apply_sync(UI_LABEL_ABS_LOCKED_USER_SELECTED_NAME, u_get.name);
    
    // Actualizar Avatar
    const lv_img_dsc_t *img_src = images_ctl_internal_get_src(u_get.avatar_id);
    lv_image_set_src(objects.dl_home_unlock_user_select_img, img_src);

    /*{
    //actualizar data
    const char *data = ACL_m_data_get_perm_to_string(user_selected_data.user_permission);
    lv_obj_t *obj = NULL;
    const char *buf = NULL;
    label_abs_prepare(UI_LABEL_ABS_LOCKED_USER_SELECTED_DATA, data, &obj, &buf);
    lv_label_set_text_static(obj, buf);
    }*/

    ESP_LOGI("DL_ACTIONS", "Usuario Activo -> ID:%d | Nombre:%s", user_id, u_get.name);
    
    return ESP_OK;
}

//**********************************************/
// 3. GESTOR DE INTERFAZ (UI Router)
//**********************************************/

/* Maneja las transiciones de pantalla encendiendo/apagando contenedores limpios */
static void ui_router_navigate(dl_action_t current_view, dl_action_t next_view) {
    if(current_view >= DL_UI_MAX || next_view >= DL_UI_MAX) return;

    lv_obj_t *current_obj = ui_obj_navegation[current_view];
    lv_obj_t *next_obj = ui_obj_navegation[next_view];

    // Lógicas de transición específicas
    if (next_view == DL_UI_USERS) {
        lv_obj_add_flag(objects.dl_home_unlock_user_select_cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(objects.dl_home_unlock_cont_modal, LV_OBJ_FLAG_HIDDEN);
    } else if (next_view == DL_UI_UNLOCK_ALL_METHODS) {
        lv_obj_remove_flag(objects.dl_home_unlock_user_select_cont, LV_OBJ_FLAG_HIDDEN);
    }

    // Intercambio visual
    if (current_obj != NULL) lv_obj_add_flag(current_obj, LV_OBJ_FLAG_HIDDEN);
    if (next_obj != NULL) lv_obj_remove_flag(next_obj, LV_OBJ_FLAG_HIDDEN);

    // Actualizar a dónde nos lleva el botón "Atrás"
    lv_obj_set_user_data(objects.dl_home_unlock_btn_back, (void*)(uintptr_t)next_view);
}

//**********************************************/
// Callbacks de Eventos
//**********************************************/

static void ui_int__cb_dl_home_lock_act_transitions(lv_event_t * e) {
    lv_obj_scroll_to_view(objects.dl_home_unlock_userpick_1_cont, LV_ANIM_OFF);
    ui_router_navigate(DL_UI_NONE, DL_UI_USERS);
}

static void ui_int__cb_Dl_home_unlock_btn_back(lv_event_t * e) {   
    dl_action_t current = (dl_action_t)(uintptr_t)lv_obj_get_user_data(lv_event_get_target(e));
    
    switch(current) {
        case DL_UI_USERS:
            lv_obj_add_flag(objects.dl_home_unlock_cont_modal, LV_OBJ_FLAG_HIDDEN);
            ui_router_navigate(current, DL_UI_NONE);
            break;
        case DL_UI_UNLOCK_ALL_METHODS:
            ui_router_navigate(current, DL_UI_USERS);
            break;
        case DL_UI_UNLOCK_PIN_METHOD:
        case DL_UI_UNLOCK_FACEID_METHOD:
            ui_router_navigate(current, DL_UI_UNLOCK_ALL_METHODS);
            for(int i = 0; i < LEDS_COUNT; i++) { //Hacemos clear despues de irnos. dejar limpio
                lv_obj_add_flag(leds_pin[i], LV_OBJ_FLAG_HIDDEN);
            }
            ui_int__helper_label_abs_apply_sync(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "");
            break;
        default: break;
    }
}

/* Disparado cuando el scroll se detiene naturalmente */
static void ui_int__cb_dl_home_unlock_userpick_m_cont(lv_event_t *e) {
    lv_obj_t *parent = lv_event_get_target(e);
    lv_obj_t *centered = ui_picker_get_centered_item(parent);
    
    if(centered == NULL) return;

    // Limpiar estados de todos y solo marcar el centrado (feedback visual)
    uint32_t child_cnt = lv_obj_get_child_count(parent);
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_remove_state(lv_obj_get_child(parent, i), LV_STATE_CHECKED);
    }
    lv_obj_add_state(centered, LV_STATE_CHECKED);
}

/* Disparado al tocar explícitamente un usuario */
static void ui_int__cb_dl_home_unlock_userpick_X(lv_event_t *e) {
    lv_obj_t *current_target = lv_event_get_current_target(e);
    lv_obj_t *parent = lv_obj_get_parent(current_target);
    
    // Validamos usando física geométrica, ignorando posibles bugs de estados LVGL
    lv_obj_t *centered_obj = ui_picker_get_centered_item(parent);

    if(current_target == centered_obj) {
        // Es el elemento central: procedemos a cargar datos y avanzar
        if (ui_state_set_active_user(current_target) == ESP_OK) {
            ui_router_navigate(DL_UI_USERS, DL_UI_UNLOCK_ALL_METHODS);
        }
    } else {
        // No está en el centro: lo centramos
        lv_obj_scroll_to_view(current_target, LV_ANIM_ON); 
    }
}

static void ui_int__cb_dl_home_unlock_go_btn(lv_event_t *e) {
    dl_action_t btn_id = (dl_action_t)(uintptr_t)lv_event_get_user_data(e);
    ui_router_navigate(DL_UI_UNLOCK_ALL_METHODS, btn_id);
}

//**********************************************/
// Pin user control
//**********************************************/
static void sys_identity_check_login_async(sys_id_event_t event, uint16_t user_id, void *ctx) {
    switch(event){
        case SYS_ID_EVT_SUCCESS:
                ui_screen_change(SCREEN_ID_DS_HOME, LV_SCREEN_LOAD_ANIM_FADE_OUT, 100, 50, false);
                ESP_LOGI(TAG, "La clave de usuario fue válida y el sistema fue logeado.");
            break;
        case SYS_ID_EVT_ERR_WRONG_PIN:
                ui_label_set_text_abs(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "La clave no funcinó, intentalo de nuevo.");
            break;
        case SYS_ID_EVT_ERR_LOCKED:
                ui_label_set_text_abs(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "El usuario está bloqueado actualmente.");
            return;
        case SYS_ID_EVT_ERR_NOT_FOUND:
                ui_label_set_text_abs(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, "El usuario seleccionado no es valido.");
            return;
        case SYS_ID_EVT_ERR_DISABLED:
            ui_label_set_text_abs(UI_LABEL_ABS_LOCKED_PIN_MESSAGE, ADL_STRINGS_USER_DISABLED);
            return;
        default:
            return;
    }
    ui_int_helper_obj_disable(objects.dl_home_unlock_pin_keyboard, false, true);
}
static void helper_update_pin_leds(uint8_t len) {
    for (int i = 0; i < LEDS_COUNT; i++) {
        if (i < len) lv_obj_clear_flag(leds_pin[i], LV_OBJ_FLAG_HIDDEN);
        else         lv_obj_add_flag(leds_pin[i], LV_OBJ_FLAG_HIDDEN);
    }
}

static void helper_check_pin(const char *input) {
    if(strlen(input) != user_selected_pin_size) return;
    ui_int_helper_obj_disable(objects.dl_home_unlock_pin_keyboard, true, false);
    esp_err_t ret = sys_id_sess_req_login(user_selected_id, input,sys_identity_check_login_async,NULL);
    if(ret != ESP_OK) ESP_LOGI(TAG, "Falló al enviar en cola un request login");
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

    } else if (code == LV_EVENT_READY) {
        helper_check_pin(pin_buf);
    }
}

//**********************************************/
// Display Locked tick start.
//**********************************************/

void ui_int__act_display_locked_tick(void) {
    ui_obj_navegation[DL_UI_NONE] = objects.dl_home_lock_cont;
    ui_obj_navegation[DL_UI_USERS] = objects.dl_home_unlock_userpick_m_cont;
    ui_obj_navegation[DL_UI_UNLOCK_ALL_METHODS] = objects.dl_home_unlock_methods;
    ui_obj_navegation[DL_UI_UNLOCK_PIN_METHOD] = objects.dl_home_unlock_pin_cont;
    ui_obj_navegation[DL_UI_UNLOCK_FACEID_METHOD] = objects.dl_home_unlock_faceid_cont;

    // Callbacks Generales
    lv_obj_add_event_cb(objects.dl_home_lock_cont, ui_int__cb_dl_home_lock_act_transitions, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(objects.dl_home_unlock_btn_back, ui_int__cb_Dl_home_unlock_btn_back, LV_EVENT_CLICKED, NULL);
    
    // Callbacks del Scroll
    lv_obj_add_event_cb(objects.dl_home_unlock_userpick_cont, ui_int__cb_dl_home_unlock_userpick_m_cont, LV_EVENT_SCROLL_END, NULL);

    // Callbacks de los Usuarios
    lv_obj_t *users[] = {
        objects.dl_home_unlock_userpick_1_cont, objects.dl_home_unlock_userpick_2_cont,
        objects.dl_home_unlock_userpick_3_cont, objects.dl_home_unlock_userpick_4_cont,
        objects.dl_home_unlock_userpick_5_cont, objects.dl_home_unlock_userpick_6_cont
    };
    for(int i = 0; i < 6; i++) {
        lv_obj_add_event_cb(users[i], ui_int__cb_dl_home_unlock_userpick_X, LV_EVENT_CLICKED, NULL);
    }

    // Callbacks de Métodos
    lv_obj_add_event_cb(objects.dl_home_unlock_methods_faceid_cont, ui_int__cb_dl_home_unlock_go_btn, LV_EVENT_CLICKED, (void*)DL_UI_UNLOCK_FACEID_METHOD);
    lv_obj_add_event_cb(objects.dl_home_unlock_methods_pin_cont, ui_int__cb_dl_home_unlock_go_btn, LV_EVENT_CLICKED, (void*)DL_UI_UNLOCK_PIN_METHOD);

    // Callbacks de PIN
    lv_obj_add_event_cb(objects.dl_home_unlock_pin_keyboard, ui_int__cb_dl_home_unlock_pin_keyboard, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(objects.dl_home_unlock_pin_keyboard, ui_int__cb_dl_home_unlock_pin_keyboard, LV_EVENT_READY, NULL);

    //lv_obj_add_event_cb(objects.ds_locked, ui_int__cb_ds_locked, LV_EVENT_SCREEN_UNLOADED, NULL);
    
    leds_pin[0] = objects.dl_home_unlock_pin_input_led1;
    leds_pin[1] = objects.dl_home_unlock_pin_input_led2;
    leds_pin[2] = objects.dl_home_unlock_pin_input_led3;
    leds_pin[3] = objects.dl_home_unlock_pin_input_led4;
    leds_pin[4] = objects.dl_home_unlock_pin_input_led5;
    leds_pin[5] = objects.dl_home_unlock_pin_input_led6;

    // Asignación inicial
    lv_obj_scroll_to_view(objects.dl_home_unlock_userpick_1_cont, LV_ANIM_OFF);
    lv_obj_add_state(objects.dl_home_unlock_userpick_1_cont, LV_STATE_CHECKED);

    ui_pub_actions_backg_update("bg_thunder_rail.jpg", objects.ds_locked);
    
    action_mutex = xSemaphoreCreateMutex();
    if(action_mutex == NULL) {
        ESP_LOGE("DL_ACTIONS", "Failed to create mutex");
    }
}