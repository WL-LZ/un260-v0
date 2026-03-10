#ifndef PAGE_07_CURR_H
#include "lvgl/lvgl.h"
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
void ui_page_08_curr_create(lv_obj_t* parent);
void ui_page_08_curr_destroy(void);
extern void bootlog_append(const char* text);
void boot_waiting_anim_start(void);
void boot_waiting_anim_stop(void);
#endif // !PAGE_07_CURR_H
