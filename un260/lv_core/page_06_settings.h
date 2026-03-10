#ifndef PAGE_06_SETTINGS_H
#define PAGE_06_SETTINGS_H
#include <stdio.h>
#include "lvgl/lvgl.h"
#include "un260/lv_resources/lv_img_init.h" 

void ui_page_06_settings_create(lv_obj_t* parent);
void ui_page_06_settings_destroy(void);
void page_06_data_collection_refresh(void);
#endif // !PAGE_06_SETTINGS_H
