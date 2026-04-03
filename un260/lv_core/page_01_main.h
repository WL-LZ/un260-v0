#ifndef PAGE_01_MAIN_H
#define PAGE_01_MAIN_H

#include "lvgl/lvgl.h"
#include "un260/lv_resources/lv_img_init.h" 


void ui_main_create(lv_obj_t* parent);
void ui_main_destroy(void);
void page_01_update_language_texts(void);
void page_01_bottom_a_refresh_mode(bool anim_en);
void page_01_bottom_a_refresh_mode_preview(uint8_t mode);
void page_01_bottom_a_refresh_add(bool anim_en);
void page_01_bottom_a_refresh_work(bool anim_en);
void page_01_bottom_a_refresh_fo(bool anim_en);
void page_01_bottom_c_refresh_batch(bool anim_en);
void page_01_bottom_c_refresh_speed(bool anim_en);
void page_01_bottom_c_refresh_cfd(void);

#endif // PAGE_01_MAIN_H
