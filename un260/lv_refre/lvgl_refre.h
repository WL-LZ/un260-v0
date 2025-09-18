#ifndef LVGL_REFRE_H
#define LVGL_REFRE_H

#include "lvgl/lvgl.h"
#include "un260/lv_core/lv_page_manager.h" 
#ifdef __cplusplus
extern "C" {  
#endif

    typedef enum {
        PAGE_MAIN,
        PAGE_SETTING
    } page_id_t;


    void ui_switch_to(page_id_t page);
    void lvgl_ui_init(void);           // 确保声明与实现一致
    void create_setting_page(void);
    void create_main_page(void);

    void page_01_mode_switch_refre(void);
    void page_01_create_mian_scrollable_container(void);

    void page_01_add_refre(void);
    void page_01_work_refre(void);
    void page_01_batch_refre(void);
    void page_01_face_refre(void);
    void page_01_cfd_refre(void);
    void page_01_speed_refre(void);
    void page_01_err_num_refre(void);
    void page_01_curr_img_refre(void);

    void page_03_create_batch_label_switcher(lv_obj_t* parent);
    void page_03_batch_num_container(void);
    void page_03_batch_num_refre(void);
    void page_03_batch_mode_status_refre(void);


    void page_02_a_page_refre(void);
    void page_02_b_page_refre(void);
    void page_02_c_page_refre(void);
    void page_02_curr_refre(void);
    void page_02_a_page_num_refre(void);
    void page_02_b_page_num_refre(void);
    void page_02_c_page_num_refre(void);


    void page_07_curr_img_refre(void);



#ifdef __cplusplus
}
#endif

#endif // LVGL_REFRE_H
