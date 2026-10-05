#include "styles.h"

/* ESP */

/* Módulos */
#include "fonts.h"


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
    lv_style_set_bg_opa(style, 0);
    lv_style_set_radius(style, 15);
    lv_style_set_text_font(style, &ui_font_mulish14);
    lv_style_set_text_color(style, lv_color_hex(0xffffffff));
    lv_style_set_bg_color(style, lv_color_hex(0xffffffff));
    lv_style_set_shadow_color(style, lv_color_hex(0xff000000));
    lv_style_set_shadow_opa(style, 50);
    lv_style_set_shadow_width(style, 18);
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
    lv_style_set_bg_color(style, lv_color_hex(0xffe9e9e9));
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
    lv_style_set_bg_color(style, lv_color_hex(0xffbfbebe));
    lv_style_set_bg_opa(style, 70);
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
// Style: St_Dtop_Modal
//

void init_style_st_dtop_modal_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_shadow_width(style, 10);
    lv_style_set_shadow_opa(style, 20);
    lv_style_set_shadow_color(style, lv_color_hex(0xffffffff));
    lv_style_set_bg_color(style, lv_color_hex(0xff282b30));
    lv_style_set_bg_opa(style, 200);
    lv_style_set_shadow_spread(style, 5);
    lv_style_set_border_width(style, 0);
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
// Style: St_action_btn_user
//

void init_style_st_action_btn_user_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_bg_color(style, lv_color_hex(0xff2b2e33));
};

lv_style_t *get_style_st_action_btn_user_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_action_btn_user_MAIN_DEFAULT(style);
    }
    return style;
};

void init_style_st_action_btn_user_MAIN_DISABLED(lv_style_t *style) {
    lv_style_set_bg_opa(style, 0);
    lv_style_set_opa(style, 100);
};

lv_style_t *get_style_st_action_btn_user_MAIN_DISABLED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_st_action_btn_user_MAIN_DISABLED(style);
    }
    return style;
};

void add_style_st_action_btn_user(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_st_action_btn_user_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(obj, get_style_st_action_btn_user_MAIN_DISABLED(), LV_PART_MAIN | LV_STATE_DISABLED);
};

void remove_style_st_action_btn_user(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_st_action_btn_user_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(obj, get_style_st_action_btn_user_MAIN_DISABLED(), LV_PART_MAIN | LV_STATE_DISABLED);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_st_let_num,
        add_style_st_wn_btns,
        add_style_st_dtop_modal,
        add_style_st_dtop_popup_btn,
        add_style_st_action_btn_user,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_st_let_num,
        remove_style_st_wn_btns,
        remove_style_st_dtop_modal,
        remove_style_st_dtop_popup_btn,
        remove_style_st_action_btn_user,
    };
    remove_style_funcs[styleIndex](obj);
}