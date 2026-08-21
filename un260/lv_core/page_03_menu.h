#ifndef PAGE_03_MENU_H
#define PAGE_03_MENU_H
#include "lvgl/lvgl.h"
#include "un260/lv_resources/lv_img_init.h" 

extern ui_element_t page_03_menu_obj[];
extern int page_03_menu_len;

void ui_page_03_menu_create(lv_obj_t* parent);
void ui_page_03_menu_destroy(void);
void switch_to_amount_batch(void);
void switch_to_pcs_batch(void);
void toggle_batch_mode(void);
void page_03_menu_function_focus(uint8_t function);
void page_03_menu_function_feedback(uint8_t function, uint8_t value);
void page_03_menu_preview_refresh(void);
void page_03_menu_clear_batch_tip(void);
void page_03_menu_show_batch_saved_tip(void);
void page_03_menu_refresh_batch_number(void);
void page_03_menu_refresh_batch_mode(void);
void page_03_batch_num_edit_reset(void);
void page_03_batch_num_edit_input(char input_num);
bool page_03_batch_num_edit_value(int* value);

#endif // !PAGE_03_MENU_H
