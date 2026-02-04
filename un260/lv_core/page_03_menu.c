#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "../aic_ui/aic_ui.h"
#include <stdlib.h>
#include <string.h>


lv_obj_t* batch_num_display;

int batch_num = 0;
int batch_num_index=0;
 char input_batch_num[9]= {0};



// 添加长度变量
int page_03_menu_len = 0;

// 定义动画和状态变量
bool is_amount_active = false;  // 初始状态  PCS为激活状态
static lv_anim_t anim_amount;
static lv_anim_t anim_pcs;

// 定义初始位置常量（相对于容器的坐标）
#define PCS_ACTIVE_Y 63     
#define AMOUNT_INACTIVE_Y 95 

// 动画回调函数 控制amount_label标签Y位置
static void amount_batch_label_anim_cb(void* var, int32_t v)
{
    lv_obj_t* label = (lv_obj_t*)var;
    lv_obj_set_y(label, v);
}

//  控制pcs_label标签Y位置
static void pcs_batch_label_anim_cb(void* var, int32_t v)
{
    lv_obj_t* label = (lv_obj_t*)var;
    lv_obj_set_y(label, v);
}

// 控制字体缩放
static void font_scale_anim_cb(void* var, int32_t v)
{
    lv_obj_t* label = (lv_obj_t*)var;
    lv_obj_set_style_transform_zoom(label, v, 0);
}

ui_element_t page_03_menu_obj[] = {
    { "page_03_menu_bg_img.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 0, 0, 1280, 400, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
//**************BTN************//
  { "03_home_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 1149, 189, 100, 100, 244, 244, 255 },
        { NULL, 0, 0, 0, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 18, 0, false },
        page_01_back_btn_event_cb, LV_EVENT_CLICKED, NULL, NULL,
        UI_BTN_STYLE_APPLE },

    { "03_cfd_l_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 710, 97, 72, 38, 100, 100, 100 },
        { "L", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_cfd_mode_event_cb, LV_EVENT_CLICKED, "0", NULL, UI_BTN_STYLE_ANDROID },

    { "03_cfd_m_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 802, 97, 72, 38, 100, 100, 100 },
        { "M", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_cfd_mode_event_cb, LV_EVENT_CLICKED, "1", NULL, UI_BTN_STYLE_ANDROID },

    { "03_cfd_h_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 894, 97, 72, 38, 100, 100, 100 },
        { "H", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_cfd_mode_event_cb, LV_EVENT_CLICKED, "2", NULL, UI_BTN_STYLE_ANDROID },

    { "03_speed_800_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 710, 155, 90, 38, 100, 100, 100 },
        { "600", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_speed_mode_event_cb, LV_EVENT_CLICKED, "0", NULL, UI_BTN_STYLE_ANDROID },

    { "03_speed_1000_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 820, 155, 90, 38, 100, 100, 100 },
        { "800", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_speed_mode_event_cb, LV_EVENT_CLICKED, "1", NULL, UI_BTN_STYLE_ANDROID },

    { "03_speed_1200_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 930, 155, 90, 38, 100, 100, 100 },
        { "1000", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_speed_mode_event_cb, LV_EVENT_CLICKED, "2", NULL, UI_BTN_STYLE_ANDROID },

    { "03_add_on_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 710, 218, 84, 38, 100, 100, 100 },
        { "ON", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_add_mode_event_cb, LV_EVENT_CLICKED, "1", NULL, UI_BTN_STYLE_ANDROID },

    { "03_add_off_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 814, 218, 84, 38, 100, 100, 100 },
        { "OFF", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_add_mode_event_cb, LV_EVENT_CLICKED, "0", NULL, UI_BTN_STYLE_ANDROID },

    { "03_fo_OFF_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 710, 286, 84, 38, 100, 100, 100 },
        { "OFF", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_fo_mode_event_cb, LV_EVENT_CLICKED, "0", NULL, UI_BTN_STYLE_ANDROID },

    { "03_fo_F_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 814, 286, 84, 38, 100, 100, 100 },
        { "F", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_fo_mode_event_cb, LV_EVENT_CLICKED, "1", NULL, UI_BTN_STYLE_ANDROID },

    { "03_fo_O_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 918, 286, 84, 38, 100, 100, 100 },
        { "O", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_fo_mode_event_cb, LV_EVENT_CLICKED, "2", NULL, UI_BTN_STYLE_ANDROID },

    { "03_fo_FO_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 1022, 286, 84, 38, 100, 100, 100 },
        { "F/O", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_fo_mode_event_cb, LV_EVENT_CLICKED, "3", NULL, UI_BTN_STYLE_ANDROID },

    { "03_work_auto_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 710, 341, 84, 38, 100, 100, 100 },
        { "AUTO", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_work_mode_event_cb, LV_EVENT_CLICKED, "0", NULL, UI_BTN_STYLE_ANDROID },

    { "03_work_manaul_btn", LV_OBJ_TYPE_BUTTON, NULL,
        { 814, 341, 120, 38, 100, 100, 100 },
        { "MANAUL", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 10, 0, false },
        page_03_work_mode_event_cb, LV_EVENT_CLICKED, "1", NULL, UI_BTN_STYLE_ANDROID },
//**********img************//


    { "page_02_home_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1171, 212, 55, 43, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE},
    

    { "03_menu_title_label", LV_OBJ_TYPE_LABEL, NULL,
        { 620, 13, 100, 36, 112, 112, 112 },
        { "MENU", 112, 112, 112, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },
    

    { "03_batch_title", LV_OBJ_TYPE_LABEL, NULL,
        { 178, 59, 210, 36, 255, 255, 255 },
        { "BATCH SETTING", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },

    { "03_batch_title", LV_OBJ_TYPE_LABEL, NULL,
        { 564, 59, 693, 36, 255, 255, 255 },
        { "FUNCTION", 255, 255, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },
    
    { "03_pcs_batch_label", LV_OBJ_TYPE_LABEL, NULL,
        { 165, 100, 212, 21, 255, 255, 255 },
        { "PCS BATCH", 47, 103, 242, &lv_font_montserrat_24, 0 },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },
    
    { "03_amount_batch_label", LV_OBJ_TYPE_LABEL, NULL,
        { 170, 132, 200, 15, 255, 255, 255 },
        { "AMOUNT BATCH", 112, 112, 112, &lv_font_montserrat_24, 0 },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },

    { "03_batch_num_mix_label", LV_OBJ_TYPE_LABEL, NULL,
        { 102, 354, 200, 15, 255, 255, 255 },
        { "BATCH NUM:", 152, 40, 48, &lv_font_montserrat_20, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },
    { "03_batch_num_label", LV_OBJ_TYPE_LABEL, NULL,
        { 252, 352, 200, 20, 255, 255, 255 },
        { "0", 152, 40, 48, &lv_font_montserrat_24, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },

//function_mode
    { "03_cfd_label", LV_OBJ_TYPE_LABEL, NULL,
        { 616, 105, 100, 40, 255, 255, 255 },
        { "CFD", 0, 115, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },
    { "03_speed_label", LV_OBJ_TYPE_LABEL, NULL,
        { 616, 163, 100, 40, 255, 255, 255 },
        { "SPEED", 0, 115, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },

    { "03_add_label", LV_OBJ_TYPE_LABEL, NULL,
        { 616, 226, 100, 40, 255, 255, 255 },
        { "ADD", 0, 115, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },


    { "03_f_o_label", LV_OBJ_TYPE_LABEL, NULL,
        { 616, 294, 100, 40, 255, 255, 255 },
        { "F/O", 0, 115, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },

    { "03_word_label", LV_OBJ_TYPE_LABEL, NULL,
        { 616, 349, 100, 40, 255, 255, 255 },
        { "WORK", 0, 115, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE },

 //*****KEYPAD*******//
{ "key_1", LV_OBJ_TYPE_BUTTON, NULL,
    {  93, 225, 90, 35, 100,100,100 }, // 254 - 29 = 225
    { "1", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
    { 255,10,0,true },
    page_03_batch_num_keypad_event_cb, LV_EVENT_CLICKED, "1", NULL, UI_BTN_STYLE_NONE },

{ "key_2", LV_OBJ_TYPE_BUTTON, NULL,
    { 188, 225, 90, 35, 100,100,100 },
    { "2", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
    { 255,10,0,true },
    page_03_batch_num_keypad_event_cb, LV_EVENT_CLICKED, "2", NULL, UI_BTN_STYLE_NONE },

{ "key_3", LV_OBJ_TYPE_BUTTON, NULL,
    { 283, 225, 90, 35, 100,100,100 },
    { "3", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
    { 255,10,0,true },
    page_03_batch_num_keypad_event_cb, LV_EVENT_CLICKED, "3", NULL, UI_BTN_STYLE_NONE },

{ "key_4", LV_OBJ_TYPE_BUTTON, NULL,
    { 378, 225, 90, 35, 100,100,100 },
    { "4", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
    { 255,10,0,true },
    page_03_batch_num_keypad_event_cb, LV_EVENT_CLICKED, "4", NULL, UI_BTN_STYLE_NONE },

    // 第二行：5 6 7 8
    { "key_5", LV_OBJ_TYPE_BUTTON, NULL,
        {  93, 265, 90, 35, 100,100,100 },
        { "5", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255,10,0,true },
        page_03_batch_num_keypad_event_cb, LV_EVENT_CLICKED, "5", NULL, UI_BTN_STYLE_NONE },

    { "key_6", LV_OBJ_TYPE_BUTTON, NULL,
        { 188, 265, 90, 35, 100,100,100 },
        { "6", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255,10,0,true },
        page_03_batch_num_keypad_event_cb, LV_EVENT_CLICKED, "6", NULL, UI_BTN_STYLE_NONE },

    { "key_7", LV_OBJ_TYPE_BUTTON, NULL,
        { 283, 265, 90, 35, 100,100,100 },
        { "7",255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255,10,0,true },
        page_03_batch_num_keypad_event_cb, LV_EVENT_CLICKED, "7", NULL, UI_BTN_STYLE_NONE },

    { "key_8", LV_OBJ_TYPE_BUTTON, NULL,
        { 378, 265, 90, 35, 100,100,100 },
        { "8", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255,10,0,true },
        page_03_batch_num_keypad_event_cb, LV_EVENT_CLICKED, "8", NULL, UI_BTN_STYLE_NONE },

        // 第三行：9 0 Del Enter
        { "key_9", LV_OBJ_TYPE_BUTTON, NULL,
            {  93, 305, 90, 35, 100,100,100 },
            { "9", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
            { 255,10,0,true },
            page_03_batch_num_keypad_event_cb, LV_EVENT_CLICKED, "9", NULL, UI_BTN_STYLE_NONE },

        { "key_0", LV_OBJ_TYPE_BUTTON, NULL,
            { 188, 305, 90, 35, 100,100,100 },
            { "0", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
            { 255,10,0,true },
            page_03_batch_num_keypad_event_cb, LV_EVENT_CLICKED, "0", NULL, UI_BTN_STYLE_NONE },

        { "key_del", LV_OBJ_TYPE_BUTTON, NULL,
            { 283, 305, 90, 35, 120,120, 120 },
            { " ", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
            { 255,10,0,true },
            page_03_batch_num_keypad_clear_event_cb, LV_EVENT_CLICKED, NULL, NULL, UI_BTN_STYLE_NONE },

        { "key_enter", LV_OBJ_TYPE_BUTTON, NULL,
            { 378, 305, 90, 35, 72,172,  80 },
            { " ", 255,255,255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
            { 255,10,0,true },
            page_03_batch_num_keypad_enter_event_cb, LV_EVENT_CLICKED, NULL, NULL, UI_BTN_STYLE_NONE },

        { "page_03_del_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 410, 312, 40, 40, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

        { "page_03_ok_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
            { 315, 315, 40, 40, 0, 0, 0 },
            { NULL, 0, 0, 0, NULL },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },
};

// 切换到AMOUNT BATCH激活状态
void switch_to_amount_batch(void)
{
    if (is_amount_active) return;

    is_amount_active = true;  
    
    lv_obj_t* amount_obj = find_obj_by_name("03_amount_batch_label", page_03_menu_obj, page_03_menu_len);
    
    // AMOUNT BATCH 向上移动动画
    lv_anim_init(&anim_amount);
    lv_anim_set_var(&anim_amount, amount_obj);
    lv_anim_set_exec_cb(&anim_amount, amount_batch_label_anim_cb);
    lv_anim_set_values(&anim_amount, AMOUNT_INACTIVE_Y, PCS_ACTIVE_Y);
    lv_anim_set_path_cb(&anim_amount, lv_anim_path_ease_out);  // 缓出动画
    lv_anim_start(&anim_amount);

    lv_obj_t* pcs_obj = find_obj_by_name("03_pcs_batch_label", page_03_menu_obj, page_03_menu_len);
    
    // PCS BATCH 向下移动动画
    lv_anim_init(&anim_pcs);
    lv_anim_set_var(&anim_pcs, pcs_obj);
    lv_anim_set_exec_cb(&anim_pcs, pcs_batch_label_anim_cb);
    lv_anim_set_values(&anim_pcs, PCS_ACTIVE_Y, AMOUNT_INACTIVE_Y);
    lv_anim_set_time(&anim_pcs, 300);
    lv_anim_set_path_cb(&anim_pcs, lv_anim_path_ease_out);
    lv_anim_start(&anim_pcs);

    lv_obj_set_style_text_color(amount_obj, lv_color_hex(0x4285F4), 0); 
    lv_obj_set_style_text_color(pcs_obj, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_opa(pcs_obj, 40, 0);  // 半透明
    lv_obj_set_style_text_opa(amount_obj, 255, 0);  

}

void switch_to_pcs_batch(void)
{
    if (!is_amount_active) return;

    is_amount_active = false;  
    
    lv_obj_t* amount_obj = find_obj_by_name("03_amount_batch_label", page_03_menu_obj, page_03_menu_len);
    lv_obj_t* pcs_obj = find_obj_by_name("03_pcs_batch_label", page_03_menu_obj, page_03_menu_len);
    
    lv_anim_init(&anim_pcs);
    lv_anim_set_var(&anim_pcs, pcs_obj);
    lv_anim_set_exec_cb(&anim_pcs, pcs_batch_label_anim_cb);
    lv_anim_set_values(&anim_pcs, AMOUNT_INACTIVE_Y, PCS_ACTIVE_Y);
    lv_anim_set_time(&anim_pcs, 300);
    lv_anim_set_path_cb(&anim_pcs, lv_anim_path_ease_out);
    lv_anim_start(&anim_pcs);

    lv_anim_init(&anim_amount);
    lv_anim_set_var(&anim_amount, amount_obj);
    lv_anim_set_exec_cb(&anim_amount, amount_batch_label_anim_cb);
    lv_anim_set_values(&anim_amount, PCS_ACTIVE_Y, AMOUNT_INACTIVE_Y); 
    lv_anim_set_time(&anim_amount, 300);
    lv_anim_set_path_cb(&anim_amount, lv_anim_path_ease_out);
    lv_anim_start(&anim_amount);

    lv_obj_set_style_text_color(pcs_obj, lv_color_hex(0x4285F4), 0);  
    lv_obj_set_style_text_color(amount_obj, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_opa(amount_obj, 40, 0);  
    lv_obj_set_style_text_opa(pcs_obj, 255, 0);  

    int num = atoi(input_batch_num);
    if (num > 200) {
        pcs_batch_num_lock_200 = true;
        strcpy(input_batch_num, "200");
        batch_num_index = 3;
        lv_label_set_text(batch_num_display, "200");
        lv_obj_set_align(batch_num_display, LV_ALIGN_RIGHT_MID);
    }

}





void toggle_batch_mode(void)
{
    if (is_amount_active) {
        switch_to_pcs_batch();
        Machine_para.batch_mode = PCS_BATCH_MODE;
    }
    else {
        switch_to_amount_batch();
        Machine_para.batch_mode = AMOUNT_BATCH_MODE;


    }
    printf("Machine_para.batch_mode: %s\n", Machine_para.batch_mode ? "AMOUNT MODE" : "PCS MODE");
}

//BATCH SWITCH
// 创建批次标签切换器


// 创建页面UI
void ui_page_03_menu_create(lv_obj_t* parent)
{
    if (menu_page) return;
    memset(input_batch_num, 0, sizeof(input_batch_num));
    batch_num_index = 0;
    menu_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(menu_page);
    lv_obj_set_pos(menu_page, 0, 0);
    lv_obj_set_size(menu_page, 1280, 400);
    lv_obj_clear_flag(menu_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(menu_page, LV_SCROLLBAR_MODE_OFF);

    page_03_menu_len = sizeof(page_03_menu_obj) / sizeof(ui_element_t);
    lv_ui_obj_init(menu_page, page_03_menu_obj, page_03_menu_len);
    page_03_batch_num_container();//BATCH_NUM_MIX
    page_03_create_batch_label_switcher(menu_page);
    //刷新batch_num
    page_03_batch_num_refre();
    //batch开关
    create_batch_num_switch(menu_page);
    lv_obj_t* obj = get_batch_switch_container();
    lv_obj_set_pos(obj, 417, 100);
    
    // 确保开关状态与Machine_para.batch_switch_enable一致
    set_batch_switch_state(Machine_para.batch_switch_enable);

    //初始化fuction开关状态
    page_03_update_menu_button_states_refresh();

}

void ui_page_03_menu_destroy(void)
{
    if (menu_page) {
        lv_obj_del(menu_page);
    }
    menu_page = NULL;

}
