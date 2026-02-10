#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "lv_page_event.h"
#include <stdio.h>
#include "un260/lv_system/platform_app.h" 
#include <string.h>
#include "un260/lv_resources/lv_img_init.h" 
#include "un260/lv_refre/lvgl_refre.h"
#include "../aic_ui/aic_ui.h"

#include "un260/lv_system/lv_str.h" 
#include "lv_page_declear.h"
#include "un260/lv_system/user_cfg.h"
// 添加数组元素计数变量
int page_01_main_len = 0;

//          "obj_name", obj_type, img_src,
//          { x, y, w, h, r, g, b },
//          { laebl, r, g, b, font,align },
//          { opacity, radius, border_width, use_custom_style },
//          event_cb, event_code, user_data, obj_ref ,btn_style}
//label 背景默认透明


static void time_area_clicked_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_push_page(UI_PAGE_TIMESET);   /* 你的页面跳转接口按工程实际替换 */
}

static lv_obj_t* s_time_label = NULL;
static lv_timer_t* s_time_timer = NULL;

static void main_time_refresh(void)
{
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%02u:%02u:%02u",
        (unsigned)Machine_para.hour,
        (unsigned)Machine_para.minute,
        (unsigned)Machine_para.second);
    if (s_time_label) lv_label_set_text(s_time_label, buf);
}

static void main_time_timer_cb(lv_timer_t* t)
{
    (void)t;
    main_time_refresh();
}

ui_element_t page_01_main_obj[] = {
    //////////////////////////////////////////////////////
  //***************    BG_IMG_LIST  *******************//
////////////////////////////////////////////////////////

    { "page_01_back.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 0, 0, 1280, 400, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

        //////////////////////////////////////////////////////
    //***************    BTN_LIST   *********************//
    //////////////////////////////////////////////////////

        { "menu_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 1125, 18, 119, 92, 244, 244, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_menu_btn_event_cb, 0, NULL , NULL ,UI_BTN_STYLE_APPLE},

        { "start_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 1125, 128, 119, 92, 244, 244, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_start_btn_event_cb , LV_EVENT_CLICKED, NULL, NULL ,UI_BTN_STYLE_APPLE},

        { "esc_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 1125, 238, 119, 92, 244, 244, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_esc_btn_event_cb, LV_EVENT_CLICKED, NULL, NULL ,UI_BTN_STYLE_APPLE},

        { "mode_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 15, 36, 75, 66, 255, 255, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_mode_btn_event_cb, LV_EVENT_CLICKED, NULL, NULL ,UI_BTN_STYLE_PRESS_FEEL },

        { "setting_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 15, 109, 75, 60, 255, 255, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_set_btn_event_cb, LV_EVENT_CLICKED, NULL, NULL ,UI_BTN_STYLE_PRESS_FEEL },

        { "list_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 15, 176, 75, 60, 255, 255, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_list_btn_event_cb, 0, (void*)(uintptr_t)UI_PAGE_LIST, NULL ,
            UI_BTN_STYLE_PRESS_FEEL},
        { "print_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 15, 243, 75, 73, 255, 255, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_print_btn_event_cb, 0, NULL, NULL ,UI_BTN_STYLE_PRESS_FEEL },

        { "curr_img_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 159, 33, 184, 100, 255, 255, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 7, 0, true },
            page_01_curr_btn_event_cb, 0, NULL, NULL ,UI_BTN_STYLE_PRESS_FEEL },
  //////////////////////////////////////////////////////
 //***************  IMAGE_LIST **********************//
//////////////////////////////////////////////////////

    { "page_01_collect_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1025, 353, 29, 30, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "curr_USD_img", LV_OBJ_TYPE_IMAGE, &page_01_curr_USD,
        { 156, 31, 182, 103, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_esc_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1167, 254, 38, 38, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_list_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 31, 184, 39, 24, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_menu_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1167, 31, 35, 35, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_mode_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 30, 43, 42, 34, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_print_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 32, 249, 38, 35, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_set_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 33, 113, 35, 35, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_start_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1167, 142, 41, 35, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_usb_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 985, 353, 22, 30, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
  //////////////////////////////////////////////////////
 //***************  LABEL_LIST **********************//
//////////////////////////////////////////////////////
    { "01_pcs_label", LV_OBJ_TYPE_LABEL, NULL,
        { 280, 144, 350, 40, 0, 0, 0 },
        { "0", 202, 23, 0, &lv_font_montserrat_44, LV_TEXT_ALIGN_RIGHT },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
    { "01_amount_label", LV_OBJ_TYPE_LABEL, NULL,
        { 280, 226, 350, 40, 0, 0, 0 },
        { "0", 202, 23, 0, &lv_font_montserrat_48, LV_TEXT_ALIGN_RIGHT },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
    { "reject_label", LV_OBJ_TYPE_LABEL, NULL,
        { 382, 67, 110, 27, 0, 0, 0 },
        { "REJECT:", 202, 23, 0, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "mode_label", LV_OBJ_TYPE_LABEL, NULL,
        { 10, 79, 81, 17, 0, 0, 0 },
        { "MODE", 136, 137, 138, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "setting_label", LV_OBJ_TYPE_LABEL, NULL,
        { 10, 151, 81, 17, 0, 0, 0 },
        { "SETTING", 136, 137, 138, &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "list_label", LV_OBJ_TYPE_LABEL, NULL,
        { 10, 215, 81, 17, 0, 0, 0 },
        { "LIST", 136, 137, 138, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "print_label", LV_OBJ_TYPE_LABEL, NULL,
        { 10, 292, 81, 17, 0, 0, 0 },
        { "PRINT", 136, 137, 138, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "mix_label", LV_OBJ_TYPE_LABEL, NULL,
        { 12, 357, 96, 17, 0, 0, 0 },
        { "MIX", 136, 137, 138, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "add_label", LV_OBJ_TYPE_LABEL, NULL,
        { 110, 357, 105, 17, 0, 0, 0 },
        { "ADD", 136, 137, 138, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "auto_label", LV_OBJ_TYPE_LABEL, NULL,
        { 217, 357, 122, 17, 0, 0, 0 },
        { "AUTO", 136, 137, 138, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "bacth_label", LV_OBJ_TYPE_LABEL, NULL,
        { 341, 357, 89, 17, 0, 0, 0 },
        { "BATCH:", 136, 137, 138, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "bacth_num_label", LV_OBJ_TYPE_LABEL, NULL,
        { 434, 357, 128, 17, 0, 0, 0 },
        { "10000", 136, 137, 138, &lv_font_montserrat_18, LV_TEXT_ALIGN_LEFT },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "face_label", LV_OBJ_TYPE_LABEL, NULL,
        { 566, 357, 111, 17, 0, 0, 0 },
        { "FACE", 136, 137, 138, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "cfd_label", LV_OBJ_TYPE_LABEL, NULL,
        { 698, 357, 50, 17, 0, 0, 0 },
        { "CFD:", 136, 137, 138, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "cfd_value_label", LV_OBJ_TYPE_LABEL, NULL,
        { 748, 357, 20, 17, 0, 0, 0 },
        { "H", 136, 137, 138, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "speed_label", LV_OBJ_TYPE_LABEL, NULL,
        { 827, 357, 33, 17, 0, 0, 0 },
        { "SP:", 136, 137, 138, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "speed_num_label", LV_OBJ_TYPE_LABEL, NULL,
        { 862, 357, 54, 17, 0, 0, 0 },
        { "1000", 136, 137, 138, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "pcs_label", LV_OBJ_TYPE_LABEL, NULL,
        { 145, 175, 54, 23, 0, 0, 0 },
        { "PCS", 0, 115, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "reject_num_label", LV_OBJ_TYPE_LABEL, NULL,
        { 509, 67, 110, 27, 0, 0, 0 },
        { "0", 202, 23, 0, &lv_font_montserrat_20, LV_TEXT_ALIGN_LEFT },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "curr_icon_label", LV_OBJ_TYPE_LABEL, NULL,
        { 145, 254, 60, 27, 0, 0, 0 },
        { "USD", 0, 115, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "list_demo_label", LV_OBJ_TYPE_LABEL, NULL,
        { 728, 24, 70, 23, 0, 0, 0 },
        { "DENOM", 121, 150, 0, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "list_pcs_label", LV_OBJ_TYPE_LABEL, NULL,
        { 826, 24, 54, 23, 0, 0, 0 },
        { "PCS", 121, 150, 0, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },

        NULL, 0, NULL, NULL },

    { "list_amount_label", LV_OBJ_TYPE_LABEL, NULL,
        { 933, 24, 80, 23, 0, 0, 0 },
        { "AMOUNT", 121, 150, 0, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "menu_btn_label", LV_OBJ_TYPE_LABEL, NULL,
        { 1144, 77, 80, 23, 0, 0, 0 },
        { "MENU", 142, 143, 145, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "start_btn_label", LV_OBJ_TYPE_LABEL, NULL,
        { 1144, 188, 80, 23, 0, 0, 0 },
        { "START", 142, 143, 145, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "esc_btn_label", LV_OBJ_TYPE_LABEL, NULL,
        { 1144, 298, 80, 23, 0, 0, 0 },
        { "ESC", 142, 143, 145, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
//list_label
        { "denom_1_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 54, 54, 23, 0, 0, 0 },
            { "---", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_1_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 54, 54, 23, 0, 0, 0 },
            { "0", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_1_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 54, 80, 23, 0, 0, 0 },
            { "0.00", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_2_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 84, 54, 23, 0, 0, 0 },
            { "---", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_2_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 84, 54, 23, 0, 0, 0 },
            { "0", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_2_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 84, 80, 23, 0, 0, 0 },
            { "0.00", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_3_label", LV_OBJ_TYPE_LABEL, NULL,
            { 0, 114, 54, 23, 0, 0, 0 },
            { "---", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_3_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 114, 54, 23, 0, 0, 0 },
            { "0", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_3_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 114, 80, 23, 0, 0, 0 },
            { "0.00", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_4_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 144, 54, 23, 0, 0, 0 },
            { "---", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_4_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 144, 54, 23, 0, 0, 0 },
            { "0", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_4_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 144, 80, 23, 0, 0, 0 },
            { "0.00", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_5_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 174, 54, 23, 0, 0, 0 },
            { "---", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_5_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 174, 54, 23, 0, 0, 0 },
            { "0", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_5_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 174, 80, 23, 0, 0, 0 },
            { "0.00", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_6_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 204, 54, 23, 0, 0, 0 },
            { "---", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_6_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 204, 54, 23, 0, 0, 0 },
            { "0", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_6_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 204, 80, 23, 0, 0, 0 },
            { "0.00", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_7_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 234, 54, 23, 0, 0, 0 },
            { "---", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_7_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 234, 54, 23, 0, 0, 0 },
            { "0", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_7_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 234, 80, 23, 0, 0, 0 },
            { "0.00", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_8_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 264, 54, 23, 0, 0, 0 },
            { "---", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_8_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 264, 54, 23, 0, 0, 0 },
            { "0", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_8_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 264, 80, 23, 0, 0, 0 },
            { "0.00", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_9_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 294, 54, 23, 0, 0, 0 },
            { "---", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_9_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 294, 54, 23, 0, 0, 0 },
            { "0", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_9_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 294, 80, 23, 0, 0, 0 },
            { "0.00", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_10_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 324, 54, 23, 0, 0, 0 },
            { "---", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_10_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 324, 54, 23, 0, 0, 0 },
            { "0", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_10_label", LV_OBJ_TYPE_LABEL, NULL,
            { 0, 324, 80, 23, 0, 0, 0 },
            { "0.00", 142, 143, 145, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "total_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 305, 54, 23, 0, 0, 0 },
            { "TOTAL", 0, 180, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "total_pcs_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 305, 54, 23, 0, 0, 0 },
            { "0", 0, 180, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "total_amount_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 305, 80, 23, 0, 0, 0 },
            { "0.00", 0, 180, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

};

static lv_obj_t* time_label = NULL;
static lv_obj_t* time_btn = NULL;
static void PAGE_01_time_obj_creat(void)
{
time_label = lv_label_create(main_page);
lv_obj_set_pos(time_label, 569, 24);
lv_obj_set_size(time_label, 269, 60);
lv_obj_set_style_text_color(time_label, lv_color_make(136, 136, 136),0);
lv_obj_set_style_text_font(time_label, &lv_font_montserrat_20, 0);
lv_label_set_text(time_label, "");
    
}

static void PAGE_01_time_btn_obj_creat(void)
{
time_btn = lv_btn_create(main_page);
lv_obj_set_pos(time_btn, 534, 0);
lv_obj_set_size(time_btn, 133, 62);
lv_obj_set_style_bg_opa(time_btn, LV_OPA_TRANSP, 0);
lv_obj_set_style_border_opa(time_btn, LV_OPA_TRANSP, 0);
lv_obj_set_style_shadow_opa(time_btn, LV_OPA_TRANSP, 0);
lv_obj_add_event_cb(time_btn, time_area_clicked_cb, LV_EVENT_CLICKED, NULL);
    
}
// 添加数组元素计数变量
void ui_main_create(lv_obj_t* parent)
{
    //creat page_main 
    if (main_page) return;
    main_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(main_page);
    lv_obj_set_pos(main_page, 0, 0);
    lv_obj_set_size(main_page, 1280, 400);
    lv_obj_clear_flag(main_page, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_scrollbar_mode(main_page, LV_SCROLLBAR_MODE_OFF); // 滚动条关闭
    static bool frist_creat = true;

    // 计算数组长度
    page_01_main_len = sizeof(page_01_main_obj) / sizeof(ui_element_t);
    
    //创建图片和其他UI元素
    lv_ui_obj_init(main_page, page_01_main_obj, page_01_main_len);
    

    if (frist_creat)
    {
        sim_data_init();
        page_01_mode_switch_refre();

        frist_creat = false;
    }
    // 创建滚动容器并将标签放入容器
   // sim_data_init();           // 初始化面额列表、count=0、amount=0
    machine_time_init();
    ui_refresh_main_page();    // 把面额+"0"+"0.00"写到每一行
    page_01_create_mian_scrollable_container();
    page_01_add_refre();
    page_01_work_refre();
    page_01_batch_refre();
    page_01_face_refre();
    page_01_cfd_refre();
    page_01_speed_refre();
    page_01_err_num_refre();
    page_01_curr_img_refre();
    PAGE_01_time_obj_creat();
    PAGE_01_time_btn_obj_creat();
    s_time_label = time_label;
    main_time_refresh();
    if (!s_time_timer) s_time_timer = lv_timer_create(main_time_timer_cb, 1000, NULL);



}

void ui_main_destroy(void)
{
    if (main_page) {
        // 安全清理所有资源和定时器
        cleanup_counting_sim();
        
        lv_obj_del(main_page);
        main_page = NULL;
    }
}

//// 更新页面上的所有多语言文本
//void page_01_update_language_texts(void) {
//    // 仅在页面存在时更新
//    if (!main_page) return;
//    
//    lv_obj_t* reject_label = find_obj_by_name("reject_label", page_01_main_obj, page_01_main_len);
//    if (reject_label) {
//        lv_label_set_text(reject_label, get_str(Str_reject));
//    }
//    
//   
//}
