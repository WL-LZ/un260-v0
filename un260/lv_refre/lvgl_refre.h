#ifndef LVGL_REFRE_H
#define LVGL_REFRE_H
#include"un260/lv_drivers/lv_drivers.h"
#include "lvgl/lvgl.h"
#include "un260/lv_core/lv_page_manager.h" 
#include "un260/lv_core/page_01_detail_scroll.h"
#include "un260/lv_system/ui_state_runtime.h"
#ifdef __cplusplus
extern "C" {  
#endif

    void page_01_mode_switch_refre(void);
    void page_01_add_refre(void);
    void page_01_work_refre(void);
    void page_01_batch_refre(void);
    void page_01_face_refre(void);
    void page_01_cfd_refre(void);
    void page_01_speed_refre(void);
    void page_01_err_num_refre(void);
    void page_01_curr_img_refre(void);
    void page_01_bottom_a_refresh_mode(bool anim_en);
    void page_01_bottom_a_refresh_mode_preview(uint8_t mode);
    void page_01_bottom_a_refresh_add(bool anim_en);
    void page_01_bottom_a_refresh_work(bool anim_en);
    void page_01_bottom_a_refresh_fo(bool anim_en);

#ifdef __cplusplus
}
#endif

#endif // LVGL_REFRE_H
