#ifndef LV_COMPONENTS_H
#define LV_COMPONENTS_H
#include "lvgl/lvgl.h"
#include"un260/lv_resources/lv_img_init.h"
extern uint8_t g_sys_err_last_code;
void create_batch_num_switch(lv_obj_t* parent);
lv_obj_t* get_batch_switch_container(void);
void set_batch_switch_state(bool enable);
void icon_feedback_comp(const char* name, ui_element_t* page_cfg_obj, int len);
void hide_system_error_popup(void);
void system_error_confirm_cb(lv_event_t* e);
void show_system_error_popup(uint8_t code);
const char* get_system_error_desc(uint8_t code);
void hide_counting_error_popup(void);
void counting_error_confirm_cb(lv_event_t* e);
void show_counting_error_popup(uint8_t code);
const char* get_counting_error_desc(uint8_t code);
#endif // !LV_COMPONENTS_H
