#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/settings_detail_ui.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/ui_text.h"
#include <string.h>
#include "lvgl/lvgl.h"
#include "../aic_ui/aic_ui.h"
static lv_obj_t* btn_cis_calib = NULL;

static lv_obj_t* cis_page = NULL;
static lv_obj_t* cis_status_label = NULL;
static lv_obj_t* cis_start_btn = NULL;
static lv_obj_t* cb_start_btn = NULL;
/*CIS校准页面*/
void cis_enter_btn_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_push_page(UI_PAGE_CIS_CALIB);
}
static void cis_esc_btn_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_pop_page();
}
static void cis_start_btn_cb(lv_event_t* e)
{
    uint8_t sub = 0x01;

    if (!settings_detail_send_command(0x5B, &sub, 1)) {
        return;
    }

    g_calib_target = CALIB_TARGET_CIS;
    cis_state = CIS_CALIB_RUNNING;
    cis_calib_ui_refresh();
}
static void cb_start_btn_cb(lv_event_t* e)
{
    uint8_t sub = 0x01;

    if (!settings_detail_send_command(0x5F, &sub, 1)) {
        return;
    }

    g_calib_target = CALIB_TARGET_CB;
    cb_state = CB_CALIB_RUNNING;
    cis_calib_ui_refresh();
    g_cb_running = 1;
}

void ui_page_cis_calib_create(lv_obj_t* parent)
{
    if (cis_page) return;

    lv_obj_t* content = NULL;
    cis_page = settings_detail_create_page(parent, ui_text_get(UI_TEXT_SETTINGS_CIS_CALIBRATION),
                                           cis_esc_btn_cb, &content);

    lv_obj_t* card = settings_detail_create_card(content, 220, 56, 840, 210);
    cis_start_btn = settings_detail_create_button(card, 60, 72, 320, 58,
                                                  ui_text_get(UI_TEXT_SETTINGS_CIS_CALIBRATION),
                                                  lv_color_hex(0x08C5D6),
                                                  cis_start_btn_cb, NULL);
    cb_start_btn = settings_detail_create_button(card, 460, 72, 320, 58,
                                                 ui_text_get(UI_TEXT_SETTINGS_COLOR_BALANCE),
                                                 lv_color_hex(0x08C5D6),
                                                 cb_start_btn_cb, NULL);

    cis_status_label = settings_detail_create_label(card, ui_text_get(UI_TEXT_SETTINGS_CIS_IDLE),
                                                    &lv_font_instrument_sans_medium_20,
                                                    lv_color_hex(0x0D3440), 60, 154);
}
void ui_page_cis_calib_destroy(void)
{
    if (!cis_page) return;
    lv_obj_del(cis_page);
    cis_page = NULL;
    cis_status_label = NULL;
    cis_start_btn = NULL;
    cb_start_btn = NULL;
    g_cb_running = 0;
   // /* 恢复主页面的金额详情容器显示（如果主页面存在） */
   // if (page_01_main_scroll_container && lv_obj_is_valid(page_01_main_scroll_container)) {
   //     lv_obj_clear_flag(page_01_main_scroll_container, LV_OBJ_FLAG_HIDDEN);
   // }
}

void cis_calib_ui_refresh(void)
{
    if (!cis_status_label) return;

    lv_color_t color = lv_color_black();
    const char* text = ui_text_get(UI_TEXT_SETTINGS_CIS_IDLE);

    if (g_calib_target == CALIB_TARGET_CB) {
        switch (cb_state) {
        case CB_CALIB_IDLE:
            text = ui_text_get(UI_TEXT_SETTINGS_CIS_IDLE);
            color = lv_color_black();
            break;

        case CB_CALIB_RUNNING:
            text = ui_text_get(UI_TEXT_SETTINGS_CB_STARTED);
            color = lv_color_make(0, 120, 255);
            break;

        case CB_CALIB_SUCCESS:
            text = ui_text_get(UI_TEXT_SETTINGS_CB_SUCCESS);
            color = lv_color_make(0, 180, 0);
            break;

        case CB_CALIB_FAIL_IR:
            text = ui_text_get(UI_TEXT_SETTINGS_CB_FAIL_IR);
            color = lv_color_make(255, 0, 0);
            break;

        default:
            break;
        }
    } else {
        switch (cis_state) {
        case CIS_CALIB_IDLE:
            text = ui_text_get(UI_TEXT_SETTINGS_CIS_IDLE);
            color = lv_color_black();
            break;

        case CIS_CALIB_RUNNING:
            text = ui_text_get(UI_TEXT_SETTINGS_CIS_STARTED);
            color = lv_color_make(0, 120, 255);
            break;

        case CIS_CALIB_SUCCESS:
            text = ui_text_get(UI_TEXT_SETTINGS_CIS_SUCCESS);
            color = lv_color_make(0, 180, 0);
            break;

        case CIS_CALIB_FAIL_UPPER:
            text = ui_text_get(UI_TEXT_SETTINGS_CIS_FAIL_UPPER);
            color = lv_color_make(255, 0, 0);
            break;

        case CIS_CALIB_FAIL_LOWER:
            text = ui_text_get(UI_TEXT_SETTINGS_CIS_FAIL_LOWER);
            color = lv_color_make(255, 0, 0);
            break;

        case CIS_CALIB_FAIL_IR:
            text = ui_text_get(UI_TEXT_SETTINGS_CIS_FAIL_IR);
            color = lv_color_make(255, 0, 0);
            break;

        default:
            break;
        }
    }

    lv_label_set_text(cis_status_label, text);
    lv_obj_set_style_text_color(cis_status_label, color, 0);
}
