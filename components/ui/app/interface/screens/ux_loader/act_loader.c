#include "act_loader.h"

/* ESP */

/* Módulos */
#include "interface.h"


static lv_anim_t * ui_anim_glow_ptr = NULL;

static void logo_glow_anim_cb(void * var, int32_t v) {
    if(var != NULL) {
        lv_obj_set_style_opa((lv_obj_t *)var, v, LV_PART_MAIN);
    }
}

static void ui_int__cb_ds_loader(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_SCREEN_LOADED) {
        lv_obj_set_style_opa(objects.dload_firmware_img, 255, LV_PART_MAIN);

        if(ui_anim_glow_ptr != NULL) {
            lv_anim_delete(objects.dload_firmware_img, logo_glow_anim_cb);
        }

        lv_anim_t a1;
        lv_anim_init(&a1);
        lv_anim_set_var(&a1, objects.dload_firmware_img);
        lv_anim_set_values(&a1, 255, 120);
        lv_anim_set_duration(&a1, 1000);
        lv_anim_set_reverse_duration(&a1, 1000);
        lv_anim_set_path_cb(&a1, lv_anim_path_ease_in_out);
        lv_anim_set_repeat_count(&a1, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(&a1, logo_glow_anim_cb);

        ui_anim_glow_ptr = lv_anim_start(&a1);
    } 
    else if(code == LV_EVENT_SCREEN_UNLOADED) {
        if(objects.dload_firmware_img != NULL) {
            lv_anim_delete(objects.dload_firmware_img, logo_glow_anim_cb);
        }
        ui_anim_glow_ptr = NULL;

        lv_obj_set_style_opa(objects.dload_firmware_img, 0, LV_PART_MAIN);
    }
}

void ui_int__act_display_loader_start_act(void) {
    lv_obj_add_event_cb(objects.display_loader, ui_int__cb_ds_loader, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(objects.display_loader, ui_int__cb_ds_loader, LV_EVENT_SCREEN_UNLOADED, NULL);
}
