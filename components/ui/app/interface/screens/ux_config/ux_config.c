#include "ux_config.h"

/* ESP */

/* Módulos */
#include "interface.h"

/* Variables */
    static bool s_init = false;


void ui_int__display_create_ds_config(void) {
    lv_obj_t *obj = lv_obj_create(0);
    objects.ds_config = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 800, 1280);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_SNAPPABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    
    //Actions call
    s_init = true;
}
bool ui_int__display_is_created_ds_config(void)
{
    return s_init;
}
