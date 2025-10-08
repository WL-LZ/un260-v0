#ifndef PLATFORM_APP_H
#define PLATFORM_APP_H
#include "lvgl/lvgl.h"
#include "un260/lv_resources/lv_img_init.h" 
#include "user_cfg.h"


typedef struct {
    int value;
    uint8_t pcs;
    float amount;
}denom_t;

typedef struct {
    denom_t denom[15];
    uint8_t denom_number;
    int total_pcs;
    float total_amount;
    char** sn_str;
    char** err_str;
    bool is_paused;
    uint16_t err_num;
    int denom_mix[10000];
}counting_sim_t;
//page_02
typedef struct {
    bool printing;
    uint8_t curent_page;
    uint8_t total_page;
}page_02_report_status_t;
extern page_02_report_status_t page_02_a_report_status;
extern page_02_report_status_t page_02_b_report_status;
extern page_02_report_status_t page_02_c_report_status;

extern bool is_amount_active;




extern counting_sim_t sim;
extern lv_timer_t* sim_timer;

extern lv_obj_t* page_01_main_scroll_container;
extern lv_obj_t* page_01_main_page_pcs_label;
extern lv_obj_t* page_01_main_page_amount_label;

extern lv_timer_t* safe_reset_timer;

void start_counting_sim(void);
void stop_counting_sim(void);
void pause_counting_sim(void);
void resume_counting_sim(void);
void sim_data_init(void);
void ui_refresh_main_page(void);//d动态刷新数据
void cleanup_counting_sim(void);//清除资源 防止野指针
void sim_timer_cb(lv_timer_t* timer);
void update_label_by_name(ui_element_t* page_cfg_obj, int len, const char* name, const char* fmt, ...);
lv_obj_t* find_obj_by_name(const char* name, ui_element_t* page_cfg_obj, int len);
void save_counting_data(void);
void restore_counting_data(void);
void sim_clear_all_sn(counting_sim_t* sim_data);
 curr_item_t get_curr_item(const char* code);
void update_label_with_simple_highlight(ui_element_t* page_cfg_obj, int len,
    const char* name, const char* fmt, ...);
void mode_switch(void);
void page_02_report_init(void);
#endif // !PLATFORM_APP_H

