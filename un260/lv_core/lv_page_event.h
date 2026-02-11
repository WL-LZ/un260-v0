#ifndef LV_PAGE_EVENT_H
#define LV_PAGE_EVENT_H
//管理事件
#include "lvgl/lvgl.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_components/lv_components.h"
#include <stdlib.h>
#include <string.h>

extern lv_obj_t* password_display;
extern char input_password[5];
extern int password_index;
//batch num
extern lv_obj_t* batch_num_display;

extern int batch_num;
extern int batch_num_index;
extern char input_batch_num[9];
extern bool pcs_batch_num_lock_200;

/*****************定时器********************/
void page_03_delete_tip_label_cb(lv_timer_t* t);
void page_05_error_label_timer_cb(lv_timer_t* t);
extern lv_timer_t* page_03_batch_num_del_timer;
extern lv_timer_t* page_05_password_del_timer;

void page_01_print_btn_event_cb(lv_event_t* e);

/*****************事件函数********************/


void page_switch_btn_event_cb(lv_event_t* e);
void page_01_back_btn_event_cb(lv_event_t* e);
void btn_event_cb(lv_event_t* e);
void page_01_list_btn_event_cb(lv_event_t* e);
void page_01_menu_btn_event_cb(lv_event_t* e);
 void page_06_back_btn_event_cb(lv_event_t* e) ;
// 跑钞仿真相关函数
void page_01_start_btn_event_cb(lv_event_t* e);
void page_01_esc_btn_event_cb(lv_event_t* e);
void page_01_mode_btn_event_cb(lv_event_t* e);

void page_01_set_btn_event_cb(lv_event_t* e);
void page_01_curr_btn_event_cb(lv_event_t* e);
// 密码键盘相关函数
void page_05_set_password_keypad_event_cb(lv_event_t* e);
void page_05_set_password_keypad_clear_event_cb(lv_event_t* e);
void page_05_set_password_keypad_enter_event_cb(lv_event_t* e);

void page_03_batch_label_input_event_cb(lv_event_t* e);
void page_03_void_batch_label_gesture_event_cb(lv_event_t* e);

//page 03 batch按键
void page_03_batch_num_keypad_event_cb(lv_event_t* e);
void page_03_batch_num_keypad_clear_event_cb(lv_event_t* e);
void page_03_batch_num_keypad_enter_event_cb(lv_event_t* e);

//page 03 功能页面
void page_03_cfd_mode_event_cb(lv_event_t* e);
void page_03_speed_mode_event_cb(lv_event_t* e);
void page_03_add_mode_event_cb(lv_event_t* e);
void page_03_fo_mode_event_cb(lv_event_t* e);
void page_03_work_mode_event_cb(lv_event_t* e);
void page_03_update_menu_button_states_refresh(void);
//PAGE03 翻页
void page_03_a_up_event_cb(lv_event_t* e);
void page_03_a_down_event_cb(lv_event_t* e);
void page_03_b_up_event_cb(lv_event_t* e);
void page_03_b_down_event_cb(lv_event_t* e);
void page_03_c_up_event_cb(lv_event_t* e);
void page_03_c_down_event_cb(lv_event_t* e);
//PAGE 07 货币切换


#endif // !LV_PAGE_EVENT_H

