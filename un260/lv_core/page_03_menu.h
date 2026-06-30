#ifndef PAGE_03_MENU_H
#define PAGE_03_MENU_H
#include "lvgl/lvgl.h"
#include "un260/lv_resources/lv_img_init.h" 

void ui_page_03_menu_create(lv_obj_t* parent);
void ui_page_03_menu_destroy(void);
void switch_to_amount_batch(void);
void switch_to_pcs_batch(void);
void toggle_batch_mode(void);
void page_03_menu_function_focus(uint8_t function);
void page_03_menu_function_feedback(uint8_t function, uint8_t value);
void page_03_menu_preview_refresh(void);

#endif // !PAGE_03_MENU_H
