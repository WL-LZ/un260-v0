#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "../aic_ui/aic_ui.h"

// 添加长度变量

ui_element_t page_06_settins_password_obj[] = {


    //////////////////////////////////////////////////////
  //***************    BG_IMG_LIST  *******************//
////////////////////////////////////////////////////////

    { "page_05_set_password_bg_img.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 0, 0, 1280, 400, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

        //////////////////////////////////////////////////////
       //***************    BTN_LIST   *********************/
      //////////////////////////////////////////////////////
          { "03_home_btn", LV_OBJ_TYPE_BUTTON,NULL,
        { 1154, 276, 101, 78, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         page_01_back_btn_event_cb, 0, NULL, NULL ,
         UI_BTN_STYLE_APPLE},
        //////////////////////////////////////////////////////
       //***************  IMAGE_LIST **********************//
      //////////////////////////////////////////////////////


        //////////////////////////////////////////////////////
       //***************  LABEL_LIST **********************//
      //////////////////////////////////////////////////////
    { "05_set_password_title_label", LV_OBJ_TYPE_LABEL,NULL ,
        { 620, 13, 150, 36, 112, 112, 112 },
        { "SETTINGS", 112, 112, 112, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
        UI_BTN_STYLE_NONE },
    { "page_02_home_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1179, 289, 35, 35, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
};

int page_06_settins_password_len = sizeof(page_06_settins_password_obj) / sizeof(page_06_settins_password_obj[0]);

void ui_page_06_settings_create(lv_obj_t* parent)
{
    if (settings_page) return;
    settings_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(settings_page);
    lv_obj_set_pos(settings_page, 0, 0);
    lv_obj_set_size(settings_page, 1280, 400);
    lv_obj_clear_flag(settings_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(settings_page, LV_SCROLLBAR_MODE_OFF);
    lv_ui_obj_init(settings_page, page_06_settins_password_obj, page_06_settins_password_len);


};

void ui_page_06_settings_destroy(void)
{
    if (settings_page)
    {
        lv_obj_del(settings_page);
        settings_page = NULL;
    }

}
