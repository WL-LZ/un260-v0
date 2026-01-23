#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/platform_app.h"
#include <string.h>
#include "lvgl/lvgl.h"
#include "../aic_ui/aic_ui.h"
static lv_obj_t* btn_cis_calib = NULL;

static lv_obj_t* cis_page = NULL;
static lv_obj_t* cis_status_label = NULL;
static lv_obj_t* cis_start_btn = NULL;
/*CIS校准页面*/
void cis_enter_btn_cb(lv_event_t* e)
{
    ui_manager_switch(UI_PAGE_CIS_CALIB);
}
static void cis_start_btn_cb(lv_event_t* e)
{
    uint8_t sub = 0x01;
    send_command(fd4, 0x5B, &sub, 1);
}
void ui_page_cis_calib_create(lv_obj_t* parent)
{
    if (cis_page) return;

 //   /* 隐藏主页面的金额详情容器，避免显示在CIS页面上 */
 //   if (page_01_main_scroll_container && lv_obj_is_valid(page_01_main_scroll_container)) {
 //       lv_obj_add_flag(page_01_main_scroll_container, LV_OBJ_FLAG_HIDDEN);
 //   }

    cis_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(cis_page);
    lv_obj_set_size(cis_page, 1280, 400);
    lv_obj_clear_flag(cis_page, LV_OBJ_FLAG_SCROLLABLE);

    /* ESC 按钮 */
    lv_obj_t* esc_btn = lv_btn_create(cis_page);
    lv_obj_set_size(esc_btn, 100, 60);
    lv_obj_set_pos(esc_btn, 1150, 10);
    lv_obj_add_event_cb(esc_btn, page_01_back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* esc_label = lv_label_create(esc_btn);
    lv_label_set_text(esc_label, "ESC");
    lv_obj_center(esc_label);

    /* 开始校验按钮 */
    cis_start_btn = lv_btn_create(cis_page);
    lv_obj_set_size(cis_start_btn, 300, 100);
    lv_obj_center(cis_start_btn);
    lv_obj_add_event_cb(cis_start_btn, cis_start_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* start_label = lv_label_create(cis_start_btn);
    lv_label_set_text(start_label, "Start Calibration");
    lv_obj_center(start_label);

    /* 状态显示 */
    cis_status_label = lv_label_create(cis_page);
    lv_label_set_text(cis_status_label, "Idle");
    lv_obj_align(cis_status_label, LV_ALIGN_BOTTOM_MID, 0, -40);
}
void ui_page_cis_calib_destroy(void)
{
    if (!cis_page) return;
    lv_obj_del(cis_page);
    cis_page = NULL;
    cis_status_label = NULL;
    cis_start_btn = NULL;
    
   // /* 恢复主页面的金额详情容器显示（如果主页面存在） */
   // if (page_01_main_scroll_container && lv_obj_is_valid(page_01_main_scroll_container)) {
   //     lv_obj_clear_flag(page_01_main_scroll_container, LV_OBJ_FLAG_HIDDEN);
   // }
}

void cis_calib_ui_refresh(void)
{
    if (!cis_status_label) return;

    lv_color_t color = lv_color_black();
    const char* text = "Idle";

    switch (cis_state) {
    case CIS_CALIB_RUNNING:
        text = "Calibration Started";
        color = lv_color_make(0, 120, 255);   // 蓝色
        break;

    case CIS_CALIB_SUCCESS:
        text = "Calibration Success";
        color = lv_color_make(0, 180, 0);     // 绿色
        break;

    case CIS_CALIB_FAIL_UPPER:
        text = "Calibration Fail (Upper CIS)";
        color = lv_color_make(255, 0, 0);
        break;

    case CIS_CALIB_FAIL_LOWER:
        text = "Calibration Fail (Lower CIS)";
        color = lv_color_make(255, 0, 0);
        break;

    case CIS_CALIB_FAIL_IR:
        text = "Calibration Fail (IR)";
        color = lv_color_make(255, 0, 0);
        break;

    default:
        break;
    }

    lv_label_set_text(cis_status_label, text);
    lv_obj_set_style_text_color(cis_status_label, color, 0);
}