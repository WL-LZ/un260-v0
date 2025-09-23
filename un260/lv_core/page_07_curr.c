#include "lv_page_declear.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "../aic_ui/aic_ui.h"

#include "un260/lv_refre/lvgl_refre.h"

const char* currencies[] = {
    "USD", "CNY","GBP","EUR","EGP","ISK","PHP","SOS","TRY", "CNY","GBP","EUR","EGP","ISK","PHP","SOS","TRY", "CNY","GBP","EUR","EGP","ISK","PHP","SOS","TRY"
};

int currencies_count = sizeof(currencies) / sizeof(currencies[0]);

ui_element_t page_07_curr_obj[] = {
    // 背景图

    { "page_07_currency_bg_img.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 0, 0, 1280, 400, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "07_curr_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 550, 13, 200, 36, 112, 112, 112 },
        { "CURRENCY", 112, 112, 112, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "02_home_btn", LV_OBJ_TYPE_BUTTON,NULL,
        { 1177, 163, 95, 94, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         page_01_back_btn_event_cb, 0, NULL, NULL ,
         UI_BTN_STYLE_APPLE},
         
     { "page_02_home_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1197, 186, 55, 43, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
        NULL, 0, NULL, NULL,
        UI_BTN_STYLE_NONE},        
//img
 
    /*{ "07_curr_01", LV_OBJ_TYPE_IMAGE, &page_07_currency_bg_img,
        { 167, 110, 182, 103, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "07_curr_02", LV_OBJ_TYPE_IMAGE, &page_07_currency_bg_img,
        { 421, 110, 182, 103, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
        
    { "07_curr_03", LV_OBJ_TYPE_IMAGE, &page_07_currency_bg_img,
        { 675, 110, 182, 103, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "07_curr_04", LV_OBJ_TYPE_IMAGE, &page_07_currency_bg_img,
        { 929, 110, 182, 103, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "07_curr_05", LV_OBJ_TYPE_IMAGE, &page_07_currency_bg_img,
        { 167, 248, 182, 103, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "07_curr_06", LV_OBJ_TYPE_IMAGE, &page_07_currency_bg_img,
        { 421, 248, 182, 103, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "07_curr_07", LV_OBJ_TYPE_IMAGE, &page_07_currency_bg_img,
        { 675, 248, 182, 103, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "07_curr_08", LV_OBJ_TYPE_IMAGE, &page_07_currency_bg_img,
        { 929, 248, 182, 103, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },*/


};

int page_07_curr_len = sizeof(page_07_curr_obj) / sizeof(page_07_curr_obj[0]);

void ui_page_07_curr_create(lv_obj_t* parent)
{
    if (curr_page) return;
    curr_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(curr_page);
    lv_obj_set_pos(curr_page, 0, 0);
    lv_obj_set_size(curr_page, 1280, 400);
    lv_obj_clear_flag(curr_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(curr_page, LV_SCROLLBAR_MODE_OFF);
    lv_ui_obj_init(curr_page, page_07_curr_obj, page_07_curr_len);
    page_07_curr_img_refre();

};

void ui_page_07_curr_destroy(void)
{
    if (curr_page)
    {
        lv_obj_del(curr_page);
        curr_page = NULL;
    }

}
