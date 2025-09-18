#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "un260/lv_system/user_cfg.h"
#include <string.h>
#include "lvgl/lvgl.h"
#include "../aic_ui/aic_ui.h"

// 密码输入相关变量 - 改为全局变量
char input_password[5] = {0}; // 存储用户输入的密码
int password_index = 0;       // 当前输入位置
lv_obj_t *password_display;   // 密码显示标签


ui_element_t page_05_set_password_obj[] = {
    // 背景图
    { "page_05_set_password_bg_img.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 0, 0, 1280, 400, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    // 返回按钮
    { "05_home_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 1154, 276, 101, 78, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
        page_01_back_btn_event_cb, 0, NULL, NULL, UI_BTN_STYLE_APPLE },

    // 返回按钮图标
    { "page_01_home_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1167, 31, 35, 35, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    // 标题
    { "05_set_password_title_label", LV_OBJ_TYPE_LABEL, NULL,
        { 620, 13, 150, 36, 112, 112, 112 },
        { "SETTINGS", 112, 112, 112, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL, UI_BTN_STYLE_NONE },

    //密码键盘
    { "key_1", LV_OBJ_TYPE_BUTTON, NULL,
    { 470, 180, 80, 60, 255, 255, 255 },
    { "1", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
     { 255, 10, 0, true },
     page_05_set_password_keypad_event_cb, LV_EVENT_CLICKED, "1", NULL, UI_BTN_STYLE_NONE },


    { "key_2", LV_OBJ_TYPE_BUTTON, NULL,
        { 560, 180, 80, 60, 255, 255, 255 },
        { "2", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_event_cb, LV_EVENT_CLICKED, "2", NULL , UI_BTN_STYLE_NONE },

    { "key_3", LV_OBJ_TYPE_BUTTON, NULL,
        { 650, 180, 80, 60, 255, 255, 255 },
        { "3", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_event_cb, LV_EVENT_CLICKED, "3", NULL , UI_BTN_STYLE_NONE },

    { "key_4", LV_OBJ_TYPE_BUTTON, NULL,
        { 740, 180, 80, 60, 255, 255, 255 },
        { "4", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_event_cb, LV_EVENT_CLICKED, "4", NULL , UI_BTN_STYLE_NONE },

    { "key_5", LV_OBJ_TYPE_BUTTON, NULL,
        { 470, 250, 80, 60, 255, 255, 255 },
        { "5", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_event_cb, LV_EVENT_CLICKED, "5", NULL , UI_BTN_STYLE_NONE }, 

    { "key_6", LV_OBJ_TYPE_BUTTON, NULL,
        { 560, 250, 80, 60, 255, 255, 255 },
        { "6", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_event_cb, LV_EVENT_CLICKED, "6", NULL , UI_BTN_STYLE_NONE },

    { "key_7", LV_OBJ_TYPE_BUTTON, NULL,
        { 650, 250, 80, 60, 255, 255, 255 },
        { "7", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_event_cb, LV_EVENT_CLICKED, "7", NULL , UI_BTN_STYLE_NONE },

    { "key_8", LV_OBJ_TYPE_BUTTON, NULL,
        { 740, 250, 80, 60, 255, 255, 255 },
        { "8", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_event_cb, LV_EVENT_CLICKED, "8", NULL , UI_BTN_STYLE_NONE },

    { "key_9", LV_OBJ_TYPE_BUTTON, NULL,
        { 470, 320, 80, 60, 255, 255, 255 },
        { "9", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_event_cb, LV_EVENT_CLICKED, "9", NULL , UI_BTN_STYLE_NONE },

    { "key_0", LV_OBJ_TYPE_BUTTON, NULL,
        { 560, 320, 80, 60, 255, 255, 255 },
        { "0", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_event_cb, LV_EVENT_CLICKED, "0", NULL , UI_BTN_STYLE_NONE },

    { "key_del", LV_OBJ_TYPE_BUTTON, NULL,
        { 650, 320, 80, 60, 120, 120, 120 },
        { " ", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_clear_event_cb, LV_EVENT_CLICKED, NULL, NULL , UI_BTN_STYLE_NONE },

    { "key_enter", LV_OBJ_TYPE_BUTTON, NULL,
        { 740, 320, 80, 60, 72, 172, 80 },
        { " ", 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, true },
        page_05_set_password_keypad_enter_event_cb, LV_EVENT_CLICKED, NULL, NULL, UI_BTN_STYLE_NONE },


    { "page_03_del_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 401, 303, 40, 40, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_03_ok_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 306, 303, 40, 40, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

};

int page_05_set_password_len = sizeof(page_05_set_password_obj) / sizeof(page_05_set_password_obj[0]);

void ui_page_05_set_password_create(lv_obj_t* parent)
{
    if (set_password_page) return;
    
    // 重置密码输入状态
    memset(input_password, 0, sizeof(input_password));
    password_index = 0;
    
    // 创建页面
    set_password_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(set_password_page);
    lv_obj_set_pos(set_password_page, 0, 0);
    lv_obj_set_size(set_password_page, 1280, 400);
    lv_obj_clear_flag(set_password_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(set_password_page, LV_SCROLLBAR_MODE_OFF);
    
    // 初始化基本UI元素
    lv_ui_obj_init(set_password_page, page_05_set_password_obj, page_05_set_password_len);
    
    // 创建密码显示区域
    lv_obj_t *password_area = lv_obj_create(set_password_page);
    lv_obj_set_size(password_area, 400, 60);
    lv_obj_align(password_area, LV_ALIGN_TOP_MID, 0,90);

    lv_obj_set_style_border_width(password_area, 1, 0);
    lv_obj_set_style_radius(password_area, 48, 0);
    lv_obj_clear_flag(password_area, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(password_area, LV_SCROLLBAR_MODE_OFF); // 滚动条关闭
    // 创建密码显示标签
    password_display = lv_label_create(password_area);
    lv_obj_set_style_text_font(password_display, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(password_display, lv_color_hex(0x000000), 0);
    lv_obj_center(password_display);
    lv_label_set_text(password_display, "");
    

}

void ui_page_05_set_password_destroy(void)
{
    if (set_password_page) {
        lv_obj_del(set_password_page);
        set_password_page = NULL;
    }
    if (page_05_password_del_timer) {
        lv_timer_del(page_05_password_del_timer);
        page_05_password_del_timer = NULL;
    }
}
