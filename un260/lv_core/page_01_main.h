#ifndef PAGE_01_MAIN_H
#define PAGE_01_MAIN_H

#include "lvgl/lvgl.h"
#include "un260/lv_resources/lv_img_init.h" 


void ui_main_create(lv_obj_t* parent);
void ui_main_destroy(void);
void page_01_update_language_texts(void);

#endif // PAGE_01_MAIN_H
