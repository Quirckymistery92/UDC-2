#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: st_BarNavLine
//

void init_style_st_bar_nav_line_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_shadow_width(style, 120);
    lv_style_set_shadow_color(style, lv_color_hex(0xff1a1a1a));
    lv_style_set_shadow_opa(style, 75);
    lv_style_set_shadow_ofs_x(style, -10);
    lv_style_set_line_width(style, 3);
};

lv_style_t *get_style_st_bar_nav_line_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_bar_nav_line_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_bar_nav_line(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_bar_nav_line_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_bar_nav_line(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_bar_nav_line_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: st_let_num
//

void init_style_st_let_num_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xffffffff));
    lv_style_set_text_align(style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_text_font(style, &ui_font_mulish14);
};

lv_style_t *get_style_st_let_num_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_let_num_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_let_num(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_let_num_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_let_num(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_let_num_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: St_Wn_Btns
//

void init_style_st_wn_btns_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xffffffff));
    lv_style_set_bg_opa(style, 40);
    lv_style_set_border_color(style, lv_color_hex(0xffffffff));
    lv_style_set_border_opa(style, 90);
    lv_style_set_border_width(style, 1);
    lv_style_set_radius(style, 15);
    lv_style_set_shadow_opa(style, 50);
    lv_style_set_shadow_color(style, lv_color_hex(0xff000000));
    lv_style_set_shadow_width(style, 10);
    lv_style_set_text_font(style, &ui_font_mulish14);
    lv_style_set_text_color(style, lv_color_hex(0xffffffff));
};

lv_style_t *get_style_st_wn_btns_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_wn_btns_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_st_wn_btns_MAIN_CHECKED(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff9c9c9c));
    lv_style_set_text_color(style, lv_color_hex(0xff222222));
};

lv_style_t *get_style_st_wn_btns_MAIN_CHECKED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_wn_btns_MAIN_CHECKED(style);
    }
    return style;
};

void init_style_st_wn_btns_MAIN_PRESSED(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff9c9c9c));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_st_wn_btns_MAIN_PRESSED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_wn_btns_MAIN_PRESSED(style);
    }
    return style;
};

void add_style_st_wn_btns(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_wn_btns_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_st_wn_btns_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_add_style(obj, get_style_st_wn_btns_MAIN_PRESSED(), LV_PART_MAIN | LV_STATE_PRESSED);
};

void remove_style_st_wn_btns(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_wn_btns_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_st_wn_btns_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_remove_style(obj, get_style_st_wn_btns_MAIN_PRESSED(), LV_PART_MAIN | LV_STATE_PRESSED);
};

//
// Style: St_DsL_extra_btn
//

void init_style_st_ds_l_extra_btn_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_opa(style, 0);
    lv_style_set_radius(style, 0);
    lv_style_set_bg_color(style, lv_color_hex(0xff222222));
};

lv_style_t *get_style_st_ds_l_extra_btn_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_ds_l_extra_btn_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_st_ds_l_extra_btn_MAIN_PRESSED(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff222222));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_st_ds_l_extra_btn_MAIN_PRESSED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_ds_l_extra_btn_MAIN_PRESSED(style);
    }
    return style;
};

void add_style_st_ds_l_extra_btn(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_ds_l_extra_btn_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_st_ds_l_extra_btn_MAIN_PRESSED(), LV_PART_MAIN | LV_STATE_PRESSED);
};

void remove_style_st_ds_l_extra_btn(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_ds_l_extra_btn_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_st_ds_l_extra_btn_MAIN_PRESSED(), LV_PART_MAIN | LV_STATE_PRESSED);
};

//
// Style: St_Dtop_Modal
//

void init_style_st_dtop_modal_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_border_color(style, lv_color_hex(0xffffffff));
    lv_style_set_border_width(style, 1);
    lv_style_set_border_opa(style, 90);
    lv_style_set_shadow_width(style, 10);
    lv_style_set_shadow_opa(style, 20);
    lv_style_set_shadow_color(style, lv_color_hex(0xffffffff));
    lv_style_set_bg_color(style, lv_color_hex(0xff0d0d0d));
    lv_style_set_bg_opa(style, 200);
    lv_style_set_shadow_spread(style, 5);
};

lv_style_t *get_style_st_dtop_modal_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_dtop_modal_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_dtop_modal(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_dtop_modal_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_dtop_modal(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_dtop_modal_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: St_Dtop_popup_btn
//

void init_style_st_dtop_popup_btn_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff1c1c1c));
    lv_style_set_bg_opa(style, 255);
};

lv_style_t *get_style_st_dtop_popup_btn_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_dtop_popup_btn_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_st_dtop_popup_btn(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_dtop_popup_btn_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_st_dtop_popup_btn(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_dtop_popup_btn_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_st_bar_nav_line,
        add_style_st_let_num,
        add_style_st_wn_btns,
        add_style_st_ds_l_extra_btn,
        add_style_st_dtop_modal,
        add_style_st_dtop_popup_btn,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_st_bar_nav_line,
        remove_style_st_let_num,
        remove_style_st_wn_btns,
        remove_style_st_ds_l_extra_btn,
        remove_style_st_dtop_modal,
        remove_style_st_dtop_popup_btn,
    };
    remove_style_funcs[styleIndex](obj);
}