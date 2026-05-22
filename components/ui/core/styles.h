#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: st_BarNavLine
lv_style_t *get_style_st_bar_nav_line_MAIN_DEFAULT();
void add_style_st_bar_nav_line(lv_obj_t *obj);
void remove_style_st_bar_nav_line(lv_obj_t *obj);

// Style: st_let_num
lv_style_t *get_style_st_let_num_MAIN_DEFAULT();
void add_style_st_let_num(lv_obj_t *obj);
void remove_style_st_let_num(lv_obj_t *obj);

// Style: St_Wn_Btns
lv_style_t *get_style_st_wn_btns_MAIN_DEFAULT();
lv_style_t *get_style_st_wn_btns_MAIN_CHECKED();
void add_style_st_wn_btns(lv_obj_t *obj);
void remove_style_st_wn_btns(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/