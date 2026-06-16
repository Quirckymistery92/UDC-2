#include "sys_module.h"
#include "sys_identity.h"
#include "esp_check.h"
#include "ui_manager.h"

static const char *TAG = "Sys Module";

typedef struct {
    lv_obj_t *obj;
    ui_label_abs_id_t label;
    lv_obj_t *obj_img;
    ui_label_abs_id_t data;
} ui_items_t;

// 2. El array global donde mapeabas los 6 slots de la pantalla
static ui_items_t ui_items[SYS_ID_MAX_USERS];

static void on_sorted_users_ready(sys_id_user_view_t **sorted_array, uint16_t count, void *ctx) {
    for(uint8_t i = 0; i < SYS_ID_MAX_USERS; i++) {
        if(i < count) {
            // Si hay un usuario en este índice, mostramos y cargamos sus datos
            ESP_LOGI(TAG, "Slot %d | id:%d | name:%s", i, sorted_array[i]->id, sorted_array[i]->name);
            
            ui_obj_remove_flag(ui_items[i].obj, LV_OBJ_FLAG_HIDDEN);
            ui_label_set_text_abs(ui_items[i].label, "%s", sorted_array[i]->name);
            ui_obj_set_user_data(ui_items[i].obj, (void*)(uintptr_t)sorted_array[i]->id);
            ui_image_set_src_by_id(ui_items[i].obj_img, sorted_array[i]->avatar_id);
            //ui_label_set_text_abs(ui_items[i].data,"%s", ACL_m_data_get_perm_to_string(sorted_array[i]->permission));
        } else {
            // Si no hay usuario, ocultamos el contenedor y blanqueamos los datos
            ui_obj_add_flag(ui_items[i].obj, LV_OBJ_FLAG_HIDDEN);
            ui_label_set_text_abs(ui_items[i].label, "");
            ui_obj_set_user_data(ui_items[i].obj, NULL);
            ui_image_set_src_by_id(ui_items[i].obj_img, UI_ICON_USER_LOGO_DEFAULT);
        }
    }
}
void ui_ident_bridge_handler(sys_id_event_t event) {
    switch (event) {
        case SYS_ID_EVT_GLOBAL_DB_CHANGED:
            // Alguien creó, borró o modificó un usuario. Pedimos la lista nueva.
            sys_id_user_req_sorted_view(on_sorted_users_ready, NULL);
            break;

        case SYS_ID_EVT_GLOBAL_LOGOUT:
            // La sesión se cerró. Mandamos a la pantalla de bloqueo.
            // ui_screen_load(UI_LOCK_SCREEN); <-- Usa tu función real aquí
            break;
            
        default:
            break;
    }
}
esp_err_t sys_module_glob_init(void) {
    esp_err_t ret = ESP_OK;
    ui_items[0].obj = objects.dl_home_unlock_userpick_1_cont;
    ui_items[0].obj_img = objects.dl_home_unlock_userpick_1_img;
    ui_items[0].label = UI_LABEL_ABS_LOCKED_USERS_1_NAME;
    ui_items[0].data = UI_LABEL_ABS_LOCKED_USERS_1_DATA;

    ui_items[1].obj = objects.dl_home_unlock_userpick_2_cont;
    ui_items[1].obj_img = objects.dl_home_unlock_userpick_2_img;
    ui_items[1].label = UI_LABEL_ABS_LOCKED_USERS_2_NAME;
    ui_items[1].data = UI_LABEL_ABS_LOCKED_USERS_2_DATA;

    ui_items[2].obj = objects.dl_home_unlock_userpick_3_cont;
    ui_items[2].obj_img = objects.dl_home_unlock_userpick_3_img;
    ui_items[2].label = UI_LABEL_ABS_LOCKED_USERS_3_NAME;
    ui_items[2].data = UI_LABEL_ABS_LOCKED_USERS_3_DATA;

    ui_items[3].obj = objects.dl_home_unlock_userpick_4_cont;
    ui_items[3].obj_img = objects.dl_home_unlock_userpick_4_img;
    ui_items[3].label = UI_LABEL_ABS_LOCKED_USERS_4_NAME;
    ui_items[3].data = UI_LABEL_ABS_LOCKED_USERS_4_DATA;

    ui_items[4].obj = objects.dl_home_unlock_userpick_5_cont;
    ui_items[4].obj_img = objects.dl_home_unlock_userpick_5_img;
    ui_items[4].label = UI_LABEL_ABS_LOCKED_USERS_5_NAME;
    ui_items[4].data = UI_LABEL_ABS_LOCKED_USERS_5_DATA;

    ui_items[5].obj = objects.dl_home_unlock_userpick_6_cont;
    ui_items[5].obj_img = objects.dl_home_unlock_userpick_6_img;
    ui_items[5].label = UI_LABEL_ABS_LOCKED_USERS_6_NAME;
    ui_items[5].data = UI_LABEL_ABS_LOCKED_USERS_6_DATA;

    ret = sys_id_init(ui_ident_bridge_handler);
    ESP_RETURN_ON_FALSE(ret == ESP_OK, ret, TAG, "No fue posible inicializar Sys_Indentity");
    ESP_LOGI(TAG, "Inicialiado el sistema de identity");
    return ret;
}
