/**
 * @file ui_zb_controller.c
 */

#include "ui_zb_controller.h"

/* ESP */
#include <string.h>
#include <stdio.h>

/* Módulos */
#include "styles.h"
#include "ui_image_sia_internal.h"
#include "fonts.h"
#include "interface.h"

// Registro interno del dispositivo en UI
typedef struct {
    bool is_active;         // Bandera para saber si este slot está ocupado
    uint16_t id;
    ui_zb_type_dev_t type;
    uint8_t col;            // Posición X en el Grid
    uint8_t row;            // Posición Y en el Grid
    
    lv_obj_t *container;
    lv_obj_t *icon;
    lv_obj_t *spinner;
    lv_obj_t *label_name;
    lv_obj_t *label_temp_c; 
    lv_obj_t *label_temp_f; 
    lv_obj_t *bar_light;    
    
    // Buffers alojados estrictamente vía lv_malloc
    char *buff_name;
    char *buff_temp_c;
    char *buff_temp_f;
} ui_zb_record_t;

// Al no autogestionar huecos, evaluamos el arreglo usando el flag is_active
static ui_zb_record_t active_devices[MAX_DEVICES_UX];

/* ==========================================
 *        HELPERS & CALLBACKS
 * ========================================== */

static int find_device_index_by_id(uint16_t id) {
    for (int i = 0; i < MAX_DEVICES_UX; i++) {
        if (active_devices[i].is_active && active_devices[i].id == id) {
            return i;
        }
    }
    return -1;
}

static int find_free_index(void) {
    for (int i = 0; i < MAX_DEVICES_UX; i++) {
        if (!active_devices[i].is_active) {
            return i;
        }
    }
    return -1;
}

// Callback multiuso en crudo. El ID viaja inyectado en el user_data.
static void ui_zb_generic_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    uint16_t id = (uint16_t)(uintptr_t)lv_event_get_user_data(e);
    
    if(code == LV_EVENT_CLICKED) {
        // Lógica aplicativa: Enviar toggle por Zigbee, etc.
        // printf("Click en dispositivo ID: %d\n", id);
    }
}

/* ==========================================
 *        CONSTRUCTORES DE PLANTILLAS
 * ========================================== */

static void build_base_container(lv_obj_t *parent, ui_zb_record_t *rec) {
    rec->container = lv_button_create(parent);
    lv_obj_set_size(rec->container, 100, 50);
    lv_obj_remove_flag(rec->container, LV_OBJ_FLAG_GESTURE_BUBBLE | LV_OBJ_FLAG_SCROLL_CHAIN_HOR |
                                       LV_OBJ_FLAG_SCROLL_CHAIN_VER | LV_OBJ_FLAG_SCROLL_ELASTIC |
                                       LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_ON_FOCUS |
                                       LV_OBJ_FLAG_SCROLL_WITH_ARROW | LV_OBJ_FLAG_SNAPPABLE);
    add_style_st_wn_btns(rec->container);
    
    // Inyectar el ID para el callback general
    lv_obj_add_event_cb(rec->container, ui_zb_generic_event_cb, LV_EVENT_ALL, (void *)(uintptr_t)rec->id);

    // Setup de posicionamiento EXACTO exigido por el sistema superior
    lv_obj_set_style_grid_cell_column_pos(rec->container, rec->col, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_grid_cell_row_pos(rec->container, rec->row, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_grid_cell_column_span(rec->container, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_grid_cell_row_span(rec->container, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_grid_cell_x_align(rec->container, LV_GRID_ALIGN_STRETCH, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_grid_cell_y_align(rec->container, LV_GRID_ALIGN_STRETCH, LV_PART_MAIN | LV_STATE_DEFAULT);

    rec->spinner = lv_spinner_create(rec->container);
    lv_obj_set_pos(rec->spinner, 92, -2);
    lv_obj_set_size(rec->spinner, 13, 13);
    lv_obj_add_flag(rec->spinner, LV_OBJ_FLAG_HIDDEN); // Hidden por default
    lv_obj_remove_flag(rec->spinner, LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_GESTURE_BUBBLE |
                                     LV_OBJ_FLAG_PRESS_LOCK | LV_OBJ_FLAG_SCROLL_ELASTIC |
                                     LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_WITH_ARROW |
                                     LV_OBJ_FLAG_SNAPPABLE);
    lv_obj_set_style_arc_width(rec->spinner, 3, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(rec->spinner, lv_color_hex(0xffffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(rec->spinner, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void create_template_light(lv_obj_t *parent, ui_zb_record_t *rec) {
    build_base_container(parent, rec);

    rec->icon = lv_image_create(rec->container);
    lv_obj_set_pos(rec->icon, 0, -19);
    lv_obj_set_size(rec->icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_image_set_src(rec->icon, ui_int__sia_icons_get_src_by_id(UI_ICON_ZB_LIGHT_OFF)); 
    lv_obj_remove_flag(rec->icon, LV_OBJ_FLAG_CLICK_FOCUSABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_align(rec->icon, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    rec->label_name = lv_label_create(rec->container);
    lv_obj_set_pos(rec->label_name, 0, 46);
    lv_obj_set_size(rec->label_name, 120, LV_SIZE_CONTENT);
    add_style_st_let_num(rec->label_name);
    lv_obj_set_style_align(rec->label_name, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(rec->label_name, &ui_font_mulish_bold14, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void create_template_temperature(lv_obj_t *parent, ui_zb_record_t *rec) {
    build_base_container(parent, rec);

    rec->icon = lv_image_create(rec->container);
    lv_obj_set_pos(rec->icon, -24, -19);
    lv_image_set_src(rec->icon, ui_int__sia_icons_get_src_by_id(UI_ICON_ZB_TEMPERATURE));
    lv_obj_set_style_align(rec->icon, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    rec->label_temp_c = lv_label_create(rec->container);
    lv_obj_set_pos(rec->label_temp_c, 4, 14);
    add_style_st_let_num(rec->label_temp_c);
    lv_obj_set_style_text_font(rec->label_temp_c, &ui_font_mulish16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(rec->label_temp_c, LV_ALIGN_TOP_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

    rec->label_temp_f = lv_label_create(rec->container);
    lv_obj_set_pos(rec->label_temp_f, 4, 37);
    add_style_st_let_num(rec->label_temp_f);
    lv_obj_set_style_text_font(rec->label_temp_f, &ui_font_mulish16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(rec->label_temp_f, LV_ALIGN_TOP_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

    rec->label_name = lv_label_create(rec->container);
    lv_obj_set_pos(rec->label_name, 0, 46);
    lv_obj_set_size(rec->label_name, 120, LV_SIZE_CONTENT);
    add_style_st_let_num(rec->label_name);
    lv_obj_set_style_align(rec->label_name, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(rec->label_name, &ui_font_mulish_bold14, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static void create_template_light_sensor(lv_obj_t *parent, ui_zb_record_t *rec) {
    build_base_container(parent, rec);
    
    rec->bar_light = lv_bar_create(rec->container);
    lv_obj_set_pos(rec->bar_light, 20, 34);
    lv_obj_set_size(rec->bar_light, 54, 24);
    lv_obj_remove_flag(rec->bar_light, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(rec->bar_light, lv_color_hex(0xffffff00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(rec->bar_light, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    rec->icon = lv_image_create(rec->container);
    lv_obj_set_pos(rec->icon, 0, -19);
    lv_image_set_src(rec->icon, ui_int__sia_icons_get_src_by_id(UI_ICON_ZB_LIGHT_AMBIENT));
    lv_obj_set_style_align(rec->icon, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    rec->label_name = lv_label_create(rec->container);
    lv_obj_set_pos(rec->label_name, 0, 46);
    lv_obj_set_size(rec->label_name, 120, LV_SIZE_CONTENT);
    add_style_st_let_num(rec->label_name);
    lv_obj_set_style_align(rec->label_name, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(rec->label_name, &ui_font_mulish_bold14, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/* ==========================================
 *        API PRINCIPAL
 * ========================================== */

esp_err_t ui_zb_device_register(uint16_t id, const char *name, ui_zb_type_dev_t type, uint8_t col, uint8_t row) {
    if (find_device_index_by_id(id) >= 0) return ESP_OK;

    int idx = find_free_index();
    if (idx < 0) return ESP_ERR_NO_MEM; // Arreglo visual lleno

    // Iniciar slot limpio
    memset(&active_devices[idx], 0, sizeof(ui_zb_record_t));
    
    active_devices[idx].is_active = true;
    active_devices[idx].id = id;
    active_devices[idx].type = type;
    active_devices[idx].col = col;
    active_devices[idx].row = row;

    // Alocamos buffer estrictamente en LVGL memory area
    active_devices[idx].buff_name = lv_malloc(MAX_DEV_NAME_LEN);
    if (!active_devices[idx].buff_name) return ESP_ERR_NO_MEM;
    strncpy(active_devices[idx].buff_name, name, MAX_DEV_NAME_LEN - 1);
    active_devices[idx].buff_name[MAX_DEV_NAME_LEN - 1] = '\0';

    // Generar layout segun el tipo
    switch(type) {
        case UX_HOME_DEV_TYPE_SENSOR_TEMPERATURE:
            create_template_temperature(objects.dhome_act_iot_grid, &active_devices[idx]);
            active_devices[idx].buff_temp_c = lv_malloc(MAX_TEMP_STR_LEN);
            active_devices[idx].buff_temp_f = lv_malloc(MAX_TEMP_STR_LEN);
            lv_label_set_text_static(active_devices[idx].label_temp_c, "-- °C");
            lv_label_set_text_static(active_devices[idx].label_temp_f, "-- °F");
            break;
            
        case UX_HOME_DEV_TYPE_LIGHT_SENSOR:
            create_template_light_sensor(objects.dhome_act_iot_grid, &active_devices[idx]);
            lv_bar_set_value(active_devices[idx].bar_light, 0, LV_ANIM_OFF);
            break;
            
        default:
            create_template_light(objects.dhome_act_iot_grid, &active_devices[idx]);
            break;
    }

    lv_label_set_text_static(active_devices[idx].label_name, active_devices[idx].buff_name);

    return ESP_OK;
}

esp_err_t ui_zb_device_remove(uint16_t id) {
    int index = find_device_index_by_id(id);
    if (index < 0) return ESP_ERR_NOT_FOUND;

    // Liberación estricta de memoria LVGL
    if (active_devices[index].buff_name) lv_free(active_devices[index].buff_name);
    if (active_devices[index].buff_temp_c) lv_free(active_devices[index].buff_temp_c);
    if (active_devices[index].buff_temp_f) lv_free(active_devices[index].buff_temp_f);
    if (active_devices[index].container) lv_obj_delete(active_devices[index].container); // Destruye hijos implícitamente

    // Limpiamos el slot y lo marcamos como libre, SIN hacer un shift al resto del array. 
    // De esta manera respetamos los huecos vacíos de la DB.
    memset(&active_devices[index], 0, sizeof(ui_zb_record_t));
    
    return ESP_OK;
}

esp_err_t ui_zb_device_set_loading(uint16_t id, bool is_loading) {
    int index = find_device_index_by_id(id);
    if (index < 0) return ESP_ERR_NOT_FOUND;

    if (is_loading) {
        lv_obj_remove_flag(active_devices[index].spinner, LV_OBJ_FLAG_HIDDEN);
        if(active_devices[index].icon) lv_obj_add_flag(active_devices[index].icon, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(active_devices[index].spinner, LV_OBJ_FLAG_HIDDEN);
        if(active_devices[index].icon) lv_obj_remove_flag(active_devices[index].icon, LV_OBJ_FLAG_HIDDEN);
    }
    return ESP_OK;
}

esp_err_t ui_zb_device_send_data(uint16_t id, const ui_zb_update_data_t *update) {
    int index = find_device_index_by_id(id);
    if (index < 0) return ESP_ERR_NOT_FOUND;
    
    ui_zb_record_t *rec = &active_devices[index];

    // 1. Manejo del estado global offline/fallas
    if (!update->status_ok) {
        lv_obj_add_state(rec->container, LV_STATE_DISABLED);
        // Fallback visual si el dispositivo usa iconos dinámicos
        if (IS_LIGHT_DEV(rec->type)) lv_image_set_src(rec->icon, ui_int__sia_icons_get_src_by_id(UI_ICON_ZB_LIGHT_ERROR));
        if (IS_SWITCH_DEV(rec->type)) lv_image_set_src(rec->icon, ui_int__sia_icons_get_src_by_id(UI_ICON_ZB_SWITCH_ERROR));
        return ESP_OK;

    } else {
        lv_obj_remove_state(rec->container, LV_STATE_DISABLED);
    }

    // Si no hay datos por cambiar y solo era update de estado
    if (!update->has_data) return ESP_OK;

    // 2. Descarte interno: Validar si la union empuja datos coherentes con el type dev
    switch (rec->type) {
        case UX_HOME_DEV_TYPE_LIGHT_DIMMER_RGB:
        case UX_HOME_DEV_TYPE_LIGHT_DIMMER:
        case UX_HOME_DEV_TYPE_LIGHT_ON_OFF:
            if (update->data.light.state == 0) {
                lv_image_set_src(rec->icon, ui_int__sia_icons_get_src_by_id(UI_ICON_ZB_LIGHT_OFF));
            } else {
                lv_image_set_src(rec->icon, ui_int__sia_icons_get_src_by_id(UI_ICON_ZB_LIGHT_ON));
            }
            break;

        case UX_HOME_DEV_TYPE_SWITCH_DIMMER:
        case UX_HOME_DEV_TYPE_SWITCH_ON_OFF:
            if (update->data.light.state == 0) {
                lv_image_set_src(rec->icon, ui_int__sia_icons_get_src_by_id(UI_ICON_ZB_SWITCH_OFF));
            } else {
                lv_image_set_src(rec->icon, ui_int__sia_icons_get_src_by_id(UI_ICON_ZB_SWITCH_ON));
            }
            break;

        case UX_HOME_DEV_TYPE_SENSOR_TEMPERATURE:
            if (!rec->buff_temp_c || !rec->buff_temp_f) {
                return ESP_ERR_INVALID_STATE;
            }
            snprintf(rec->buff_temp_c, MAX_TEMP_STR_LEN, "%.1f °C", update->data.temp.temp_c);
            snprintf(rec->buff_temp_f, MAX_TEMP_STR_LEN, "%.1f °F", update->data.temp.temp_f);
            
            // Reasignamos static para obligar a lvgl a re-dibujar sobre el buffer actualizado
            lv_label_set_text_static(rec->label_temp_c, rec->buff_temp_c);
            lv_label_set_text_static(rec->label_temp_f, rec->buff_temp_f);
            break;

        case UX_HOME_DEV_TYPE_LIGHT_SENSOR:
            if (!rec->bar_light) {
                return ESP_ERR_INVALID_STATE;
            }
            lv_bar_set_value(rec->bar_light, update->data.light_sensor.level, LV_ANIM_ON);
            break;

        default:
            return ESP_ERR_NOT_FOUND;
    }

    return ESP_OK;
}