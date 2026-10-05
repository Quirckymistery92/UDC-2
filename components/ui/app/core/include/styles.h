#pragma once

/* ESP */

/* Módulos */
#include <lvgl.h>


// Style: st_let_num
lv_style_t *get_style_st_let_num_MAIN_DEFAULT();
void add_style_st_let_num(lv_obj_t *obj);
void remove_style_st_let_num(lv_obj_t *obj);

// Style: St_Wn_Btns
lv_style_t *get_style_st_wn_btns_MAIN_DEFAULT();
lv_style_t *get_style_st_wn_btns_MAIN_CHECKED();
lv_style_t *get_style_st_wn_btns_MAIN_PRESSED();
void add_style_st_wn_btns(lv_obj_t *obj);
void remove_style_st_wn_btns(lv_obj_t *obj);

// Style: St_Dtop_Modal
lv_style_t *get_style_st_dtop_modal_MAIN_DEFAULT();
void add_style_st_dtop_modal(lv_obj_t *obj);
void remove_style_st_dtop_modal(lv_obj_t *obj);

// Style: St_Dtop_popup_btn
lv_style_t *get_style_st_dtop_popup_btn_MAIN_DEFAULT();
void add_style_st_dtop_popup_btn(lv_obj_t *obj);
void remove_style_st_dtop_popup_btn(lv_obj_t *obj);

// Style: St_action_btn_user
lv_style_t *get_style_st_action_btn_user_MAIN_DEFAULT();
lv_style_t *get_style_st_action_btn_user_MAIN_DISABLED();
void add_style_st_action_btn_user(lv_obj_t *obj);
void remove_style_st_action_btn_user(lv_obj_t *obj);

