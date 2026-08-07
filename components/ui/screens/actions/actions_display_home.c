#include "actions_display_home.h"
#include "actions_control.h"
#include "esp_log.h"
#include "sys_module.h"

void ui_int__cb_ds_home(lv_event_t *e) {
    
}
void ui_int__cb_btn_home_weather(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED){}
    else if(code == LV_EVENT_LONG_PRESSED) {}
}
void ui_int__act_display_home_start_act(void) {
    ui_pub_actions_backg_update_DSHOME("bg_thunder_rail.jpg");
    lv_obj_add_event_cb(objects.ds_home, ui_int__cb_ds_home, LV_EVENT_SCREEN_LOAD_START, NULL);
    //lv_obj_add_event_cb(objects.ds_home_actm_sys_weather, ui_int__cb_btn_home_weather, LV_EVENT_CLICKED, NULL);
    //lv_obj_add_event_cb(objects.ds_home_actm_sys_weather, ui_int__cb_btn_home_weather, LV_EVENT_LONG_PRESSED, NULL);
}