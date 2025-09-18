#ifndef LV_COMPONENTS_H
#define LV_COMPONENTS_H
#include "lvgl/lvgl.h"
#include"un260/lv_resources/lv_img_init.h"
void create_batch_num_switch(lv_obj_t* parent);
lv_obj_t* get_batch_switch_container(void);
void set_batch_switch_state(bool enable);
void icon_feedback_comp(const char* name, ui_element_t* page_cfg_obj, int len);
#endif // !LV_COMPONENTS_H
