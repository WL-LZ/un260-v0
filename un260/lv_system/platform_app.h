#ifndef PLATFORM_APP_H
#define PLATFORM_APP_H
#include "lvgl/lvgl.h"
#include "un260/counting/counting_data_types.h"
#include "un260/lv_resources/lv_img_init.h"
#include "user_cfg.h"

void start_counting_sim(void);
void stop_counting_sim(void);
void pause_counting_sim(void);
void resume_counting_sim(void);
void sim_data_init(void);
void ui_refresh_main_page(void);
void ui_count_end_anim_cancel(void);
void ui_count_end_anim_begin(const char *result_text);
void ui_count_end_anim_poll(void);
int sim_get_sn_valid_count(void);
int sim_get_sn_nth_valid_index(int nth);
void page_01_main_detail_refresh_rows_only(void);
void cleanup_counting_sim(void);
void update_label_by_name(ui_element_t* page_cfg_obj, int len, const char* name, const char* fmt, ...);
lv_obj_t* find_obj_by_name(const char* name, ui_element_t* page_cfg_obj, int len);
void sim_reset_counting_result(counting_sim_t* sim_data);
void sim_reset_for_currency(counting_sim_t* sim_data);
void update_label_with_simple_highlight(ui_element_t* page_cfg_obj, int len,
    const char* name, const char* fmt, ...);
void mode_switch(void);
void sim_clear_err_only(counting_sim_t* sim_data);
#endif // !PLATFORM_APP_H
