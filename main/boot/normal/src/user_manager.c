#include "user_manager.h"

/* ESP */
#include "esp_check.h"
#include "freertos/FreeRTOS.h"

/* Módulos */
#include "ui_manager.h"
#include "user_mgr.h"

/* Variables & estructuras */
    static const char * const TAG = "USR_M";
    typedef struct {
        lv_obj_t **obj;
        ui_label_abs_id_t label;
        lv_obj_t **obj_img;
        ui_label_abs_id_t data;
    } ui_items_t;
    static ui_items_t ui_items[USER_MGR_MAX_USERS];
    static app_desc_status_t apd = {
        .app_desc = DSC_APP_USER_MANAGER,
        .app_name = "Admin. de usuarios",
        .app_tag = TAG,
        .init_result = ESP_FAIL
    };

static void on_sorted_users_ready(user_mgr_user_view_t **sorted_array, uint16_t count, void *ctx) {

    for(uint8_t i = 0; i < USER_MGR_MAX_USERS; i++) {
        if(i < count) {
            // Si hay un usuario en este índice, mostramos y cargamos sus datos
            ESP_LOGI(TAG, "Slot %d | id:%d | name:%s | avatar_id:%d | lock: %s", i, sorted_array[i]->id, sorted_array[i]->name, sorted_array[i]->avatar_id, sorted_array[i]->is_enabled ? "Sí" : "No");

            if(!sorted_array[i]->is_enabled) { //Si está suspendido el usuario no va a llamar a las otras funciones, es ambiguo si no se va a usar.
                ui_obj_add_state(*ui_items[i].obj, LV_STATE_DISABLED);
                ui_obj_remove_flag(*ui_items[i].obj, LV_OBJ_FLAG_CLICKABLE);
                ui_label_set_text_abs(ui_items[i].data,"%s", "Bloqueado");
            } else {
                ui_obj_remove_state(*ui_items[i].obj, LV_STATE_DISABLED);
                ui_obj_add_flag(*ui_items[i].obj, LV_OBJ_FLAG_CLICKABLE);
                ui_label_set_text_abs(ui_items[i].data,"%s", sorted_array[i]->role_str);
                ui_obj_set_user_data(*ui_items[i].obj, (void*)(uintptr_t)sorted_array[i]->id);
            }

            ui_obj_remove_flag(*ui_items[i].obj, LV_OBJ_FLAG_HIDDEN); // Aparece...
            ui_label_set_text_abs(ui_items[i].label, "%s", sorted_array[i]->name); // Se llama..
            ui_image_set_src_by_id(*ui_items[i].obj_img, sorted_array[i]->avatar_id);
        } else {
            // Si no hay usuario, ocultamos el contenedor
            ui_obj_add_flag(*ui_items[i].obj, LV_OBJ_FLAG_HIDDEN);
            ui_obj_set_user_data(*ui_items[i].obj, NULL);
        }
    }
}
void ui_ident_bridge_handler(user_mgr_event_t event) {
    switch (event) {
        case USER_MGR_EVT_GLOBAL_DB_CHANGED:
            user_mgr_user_req_sorted_view(on_sorted_users_ready, NULL);
            break;

        case USER_MGR_EVT_GLOBAL_LOGOUT:
            ui_screen_change(SCREEN_ID_DS_LOCKED, LV_SCREEN_LOAD_ANIM_FADE_OUT, 1000, 0, false);
            break;
        
        case USER_MGR_EVT_GLOBAL_LOGIN: {
            uint16_t logged_in_user_id = user_mgr_sess_get_active();
            user_mgr_user_view_t u_get;
            esp_err_t ret = user_mgr_user_get_data(logged_in_user_id, &u_get);
            if(ret != ESP_OK) {
                ui_spc_send_notify_user(true,"Ocurrió un error inesperado, por favor vuelva a intentarlo (FC: %s)", __func__);
                return;
            }
            ui_label_set_text_abs(UI_LABEL_ABS_HOME_USER_LOGGED_NAME, "%s", u_get.name);
            ui_image_set_src_by_id(objects.dhome_user_login_img, u_get.avatar_id);
            
            ui_screen_change(SCREEN_ID_DS_HOME, LV_SCREEN_LOAD_ANIM_FADE_OUT, 100, 50, false);
            }
            break;
            
        default:
            break;
    }
}
app_desc_status_t* user_manager_init(void)
{
    bool __is_valid = ui_display_is_created_ds_locked();
    DSC_RETURN_STRUCT_ON_FALSE(__is_valid, &apd, ESP_ERR_INVALID_STATE);

    ui_items[0].obj = &objects.dlock_unlock_userpick_1_cont;
    ui_items[0].obj_img = &objects.dlock_unlock_userpick_1_img;
    ui_items[0].label = UI_LABEL_ABS_LOCKED_USERS_1_NAME;
    ui_items[0].data = UI_LABEL_ABS_LOCKED_USERS_1_DATA;

    ui_items[1].obj = &objects.dlock_unlock_userpick_2_cont;
    ui_items[1].obj_img = &objects.dlock_unlock_userpick_2_img;
    ui_items[1].label = UI_LABEL_ABS_LOCKED_USERS_2_NAME;
    ui_items[1].data = UI_LABEL_ABS_LOCKED_USERS_2_DATA;

    ui_items[2].obj = &objects.dlock_unlock_userpick_3_cont;
    ui_items[2].obj_img = &objects.dlock_unlock_userpick_3_img;
    ui_items[2].label = UI_LABEL_ABS_LOCKED_USERS_3_NAME;
    ui_items[2].data = UI_LABEL_ABS_LOCKED_USERS_3_DATA;

    ui_items[3].obj = &objects.dlock_unlock_userpick_4_cont;
    ui_items[3].obj_img = &objects.dlock_unlock_userpick_4_img;
    ui_items[3].label = UI_LABEL_ABS_LOCKED_USERS_4_NAME;
    ui_items[3].data = UI_LABEL_ABS_LOCKED_USERS_4_DATA;

    ui_items[4].obj = &objects.dlock_unlock_userpick_5_cont;
    ui_items[4].obj_img = &objects.dlock_unlock_userpick_5_img;
    ui_items[4].label = UI_LABEL_ABS_LOCKED_USERS_5_NAME;
    ui_items[4].data = UI_LABEL_ABS_LOCKED_USERS_5_DATA;

    ui_items[5].obj = &objects.dlock_unlock_userpick_6_cont;
    ui_items[5].obj_img = &objects.dlock_unlock_userpick_6_img;
    ui_items[5].label = UI_LABEL_ABS_LOCKED_USERS_6_NAME;
    ui_items[5].data = UI_LABEL_ABS_LOCKED_USERS_6_DATA;

    esp_err_t ret = user_mgr_init(ui_ident_bridge_handler);
    DSC_RETURN_STRUCT_ON_FALSE(ret == ESP_OK, &apd, ret);

    return DESC_AP_INSERT(&apd, ret);
}
