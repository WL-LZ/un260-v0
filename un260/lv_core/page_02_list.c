#include "un260/lv_core/page_02_list.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "../aic_ui/aic_ui.h"


// 添加长度变量
int page_02_list_len = 0;

ui_element_t page_02_list_obj[] = {

    //////////////////////////////////////////////////////
  //***************    BG_IMG_LIST  *******************//
////////////////////////////////////////////////////////

    { "page_02_list_img.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 0, 0, 1280, 400, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

  //////////////////////////////////////////////////////
 //***************    BTN_LIST   *********************/
//////////////////////////////////////////////////////

    { "02_home_btn", LV_OBJ_TYPE_BUTTON,NULL,
        { 1154, 276, 101, 78, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         page_01_back_btn_event_cb, 0, NULL, NULL ,
         UI_BTN_STYLE_APPLE},

    { "02_print", LV_OBJ_TYPE_BUTTON,NULL,
        { 1154, 160, 101, 78, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         page_01_print_btn_event_cb, 0, NULL, NULL ,
         UI_BTN_STYLE_APPLE},
    { "02_a_up", LV_OBJ_TYPE_BUTTON,NULL,
        { 25, 79, 359, 141, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         page_03_a_up_event_cb, 0, NULL, NULL ,
         UI_BTN_STYLE_NO_FEEDBACK},
    { "02_a_down", LV_OBJ_TYPE_BUTTON,NULL,
        { 25, 220, 359, 141, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         page_03_a_down_event_cb, 0, NULL, NULL ,
         UI_BTN_STYLE_NO_FEEDBACK},
    { "02_b_up", LV_OBJ_TYPE_BUTTON,NULL,
        { 405, 79, 392, 141, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         page_03_b_up_event_cb, 0, NULL, NULL ,
         UI_BTN_STYLE_NO_FEEDBACK},
    { "02_b_down", LV_OBJ_TYPE_BUTTON,NULL,
        { 405, 220, 392, 141, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         page_03_b_down_event_cb, 0, NULL, NULL ,
         UI_BTN_STYLE_NO_FEEDBACK},
    { "02_c_up", LV_OBJ_TYPE_BUTTON,NULL,
        { 820, 79, 309, 141, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         page_03_c_up_event_cb, 0, NULL, NULL ,
         UI_BTN_STYLE_NO_FEEDBACK},
    { "02_c_down", LV_OBJ_TYPE_BUTTON,NULL,
        { 820, 220, 309, 141, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         page_03_c_down_event_cb, 0, NULL, NULL ,
         UI_BTN_STYLE_NO_FEEDBACK},

  //////////////////////////////////////////////////////
 //***************  IMAGE_LIST **********************//
//////////////////////////////////////////////////////

    { "page_02_home_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1179, 289, 43, 43, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "page_01_print_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1187, 180, 43, 43, 255, 255, 255 },
        { NULL, 0, 0, 0, NULL },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

  //////////////////////////////////////////////////////
 //***************  LABEL_LIST **********************//
//////////////////////////////////////////////////////

    { "02_list_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 610, 13, 70, 36, 112, 112, 112 },
        { "LIST", 112, 112, 112, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "02_a_denom_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 58, 58, 70, 36, 255, 255, 255 },
        { "DENOM", 255, 255, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "02_a_pcs_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 171, 58, 70, 36, 255, 255, 255 },
        { "PCS", 255, 255, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "02_a_amount_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 267, 58, 80, 36, 255, 255, 255 },
        { "AMOUNT", 255, 255, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "02_b_no_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 444, 58, 70, 36, 255, 255, 255 },
        { "NO", 255, 255, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "02_b_sn_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 516, 58, 70, 36, 255, 255, 255 },
        { "SN", 255, 255, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "02_b_denom_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 679, 58, 70, 36, 255, 255, 255 },
        { "DENOM", 255, 255, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "02_c_no_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 834, 58, 70, 36, 255, 255, 255 },
        { "NO", 255, 255, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "02_c_denom_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 887, 58, 70, 36, 255, 255, 255 },
        { "PCS", 255, 255, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

    { "02_c_reject_title_label", LV_OBJ_TYPE_LABEL, NULL ,
        { 973, 58, 70, 36, 255, 255, 255 },
        { "REJECT", 255, 255, 255, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, NULL, NULL ,
         UI_BTN_STYLE_NONE},

//************  a_REPORT ****************//
    { "02_a_denom_1", LV_OBJ_TYPE_LABEL, NULL,
      { 72,  89, 80, 36, 255, 255, 255 },
      { "1", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "1", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_denom_2", LV_OBJ_TYPE_LABEL, NULL,
      { 72, 120, 80, 36, 255, 255, 255 },
      { "2", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "2", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_denom_3", LV_OBJ_TYPE_LABEL, NULL,
      { 72, 151, 80, 36, 255, 255, 255 },
      { "3", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "3", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_denom_4", LV_OBJ_TYPE_LABEL, NULL,
      { 72, 182, 80, 36, 255, 255, 255 },
      { "4", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "4", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_denom_5", LV_OBJ_TYPE_LABEL, NULL,
      { 72, 213, 80, 36, 255, 255, 255 },
      { "5", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "5", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_denom_6", LV_OBJ_TYPE_LABEL, NULL,
      { 72, 244, 80, 36, 255, 255, 255 },
      { "6", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "6", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_denom_7", LV_OBJ_TYPE_LABEL, NULL,
      { 72, 275, 80, 36, 255, 255, 255 },
      { "7", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "7", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_denom_8", LV_OBJ_TYPE_LABEL, NULL,
      { 72, 306, 80, 36, 255, 255, 255 },
      { "8", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "8", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_denom_total", LV_OBJ_TYPE_LABEL, NULL,
      { 58, 337, 80, 36, 255, 255, 255 },
      { "TOTAL", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, 0, NULL,
      UI_BTN_STYLE_NONE },
//a_pcs
    { "02_a_pcs_1", LV_OBJ_TYPE_LABEL, NULL,
      { 171,  89, 70, 36, 255, 255, 255 },
      { "1", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "1", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_pcs_2", LV_OBJ_TYPE_LABEL, NULL,
      { 171, 120, 70, 36, 255, 255, 255 },
      { "2", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "2", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_pcs_3", LV_OBJ_TYPE_LABEL, NULL,
      { 171, 151, 70, 36, 255, 255, 255 },
      { "3", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "3", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_pcs_4", LV_OBJ_TYPE_LABEL, NULL,
      { 171, 182, 70, 36, 255, 255, 255 },
      { "4", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "4", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_pcs_5", LV_OBJ_TYPE_LABEL, NULL,
      { 171, 213, 70, 36, 255, 255, 255 },
      { "5", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "5", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_pcs_6", LV_OBJ_TYPE_LABEL, NULL,
      { 171, 244, 70, 36, 255, 255, 255 },
      { "6", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "6", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_pcs_7", LV_OBJ_TYPE_LABEL, NULL,
      { 171, 275, 70, 36, 255, 255, 255 },
      { "7", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "7", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_pcs_8", LV_OBJ_TYPE_LABEL, NULL,
      { 171, 306, 70, 36, 255, 255, 255 },
      { "8", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, "8", NULL,
      UI_BTN_STYLE_NONE },

    { "02_a_pcs_amount", LV_OBJ_TYPE_LABEL, NULL,
      { 171, 337, 70, 36, 255, 255, 255 },
      { "0", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
      { 255, 18, 0, false },
      NULL, 0, 0, NULL,
      UI_BTN_STYLE_NONE },
//amount_total
      { "02_a_amount_1", LV_OBJ_TYPE_LABEL, NULL,
        { 267,  89, 80, 36, 255, 255, 255 },
        { "1", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "1", NULL,
        UI_BTN_STYLE_NONE },

      { "02_a_amount_2", LV_OBJ_TYPE_LABEL, NULL,
        { 267, 120, 80, 36, 255, 255, 255 },
        { "2", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "2", NULL,
        UI_BTN_STYLE_NONE },

      { "02_a_amount_3", LV_OBJ_TYPE_LABEL, NULL,
        { 267, 151, 80, 36, 255, 255, 255 },
        { "3", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "3", NULL,
        UI_BTN_STYLE_NONE },

      { "02_a_amount_4", LV_OBJ_TYPE_LABEL, NULL,
        { 267, 182, 80, 36, 255, 255, 255 },
        { "4", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "4", NULL,
        UI_BTN_STYLE_NONE },

      { "02_a_amount_5", LV_OBJ_TYPE_LABEL, NULL,
        { 267, 213, 80, 36, 255, 255, 255 },
        { "5", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "5", NULL,
        UI_BTN_STYLE_NONE },

      { "02_a_amount_6", LV_OBJ_TYPE_LABEL, NULL,
        { 267, 244, 80, 36, 255, 255, 255 },
        { "6", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "6", NULL,
        UI_BTN_STYLE_NONE },

      { "02_a_amount_7", LV_OBJ_TYPE_LABEL, NULL,
        { 267, 275, 80, 36, 255, 255, 255 },
        { "7", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "7", NULL,
        UI_BTN_STYLE_NONE },

      { "02_a_amount_8", LV_OBJ_TYPE_LABEL, NULL,
        { 267, 306, 80, 36, 255, 255, 255 },
        { "8", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "8", NULL,
        UI_BTN_STYLE_NONE },

      { "02_a_amount_total", LV_OBJ_TYPE_LABEL, NULL,
        { 267, 337, 80, 36, 255, 255, 255 },
        { "0", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, 0, NULL,
        UI_BTN_STYLE_NONE },

//************  b_REPORT ****************//
    { "02_b_no_1", LV_OBJ_TYPE_LABEL, NULL ,
        { 446, 89, 70, 36, 255, 255, 255 },
        { "1", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, "1", NULL ,
         UI_BTN_STYLE_NONE },
    { "02_b_no_2", LV_OBJ_TYPE_LABEL, NULL ,
        { 446, 120, 70, 36, 255, 255, 255 },
        { "2", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
         NULL, 0, "2", NULL ,
         UI_BTN_STYLE_NONE },
      { "02_b_no_3", LV_OBJ_TYPE_LABEL, NULL ,
          { 446, 151, 70, 36, 255, 255, 255 },
          { "3", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
           NULL, 0, "3", NULL ,
           UI_BTN_STYLE_NONE },
      { "02_b_no_4", LV_OBJ_TYPE_LABEL, NULL ,
          { 446, 182, 70, 36, 255, 255, 255 },
          { "4", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
           NULL, 0, "4", NULL ,
           UI_BTN_STYLE_NONE },
      { "02_b_no_5", LV_OBJ_TYPE_LABEL, NULL ,
          { 446, 213, 70, 36, 255, 255, 255 },
          { "5", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
           NULL, 0, "5", NULL ,
           UI_BTN_STYLE_NONE },
      { "02_b_no_6", LV_OBJ_TYPE_LABEL, NULL ,
          { 446, 244, 70, 36, 255, 255, 255 },
          { "6", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
           NULL, 0, "6", NULL ,
           UI_BTN_STYLE_NONE },
      { "02_b_no_7", LV_OBJ_TYPE_LABEL, NULL ,
          { 446, 275, 70, 36, 255, 255, 255 },
          { "7", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
           NULL, 0, "7", NULL ,
           UI_BTN_STYLE_NONE },
      { "02_b_no_8", LV_OBJ_TYPE_LABEL, NULL ,
          { 446, 306, 70, 36, 255, 255, 255 },
          { "8", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
           NULL, 0, "8", NULL ,
           UI_BTN_STYLE_NONE },
        { "02_b_no_9", LV_OBJ_TYPE_LABEL, NULL ,
            { 446, 337, 70, 36, 255, 255, 255 },
            { "9", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
             NULL, 0, "9", NULL ,
             UI_BTN_STYLE_NONE },

//b_sn
      { "02_b_sn_1", LV_OBJ_TYPE_LABEL, NULL,
        { 516,  89, 200, 36, 255, 255, 255 },
        { "1", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "1", NULL,
        UI_BTN_STYLE_NONE },

      { "02_b_sn_2", LV_OBJ_TYPE_LABEL, NULL,
        { 516, 120, 200, 36, 255, 255, 255 },
        { "2", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "2", NULL,
        UI_BTN_STYLE_NONE },

      { "02_b_sn_3", LV_OBJ_TYPE_LABEL, NULL,
        { 516, 151, 200, 36, 255, 255, 255 },
        { "3", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "3", NULL,
        UI_BTN_STYLE_NONE },

      { "02_b_sn_4", LV_OBJ_TYPE_LABEL, NULL,
        { 516, 182, 200, 36, 255, 255, 255 },
        { "4", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "4", NULL,
        UI_BTN_STYLE_NONE },

      { "02_b_sn_5", LV_OBJ_TYPE_LABEL, NULL,
        { 516, 213, 200, 36, 255, 255, 255 },
        { "5", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "5", NULL,
        UI_BTN_STYLE_NONE },

      { "02_b_sn_6", LV_OBJ_TYPE_LABEL, NULL,
        { 516, 244, 200, 36, 255, 255, 255 },
        { "6", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "6", NULL,
        UI_BTN_STYLE_NONE },

      { "02_b_sn_7", LV_OBJ_TYPE_LABEL, NULL,
        { 516, 275, 200, 36, 255, 255, 255 },
        { "7", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "7", NULL,
        UI_BTN_STYLE_NONE },

      { "02_b_sn_8", LV_OBJ_TYPE_LABEL, NULL,
        { 516, 306, 200, 36, 255, 255, 255 },
        { "8", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "8", NULL,
        UI_BTN_STYLE_NONE },

      { "02_b_sn_9", LV_OBJ_TYPE_LABEL, NULL,
        { 516, 337, 200, 36, 255, 255, 255 },
        { "9", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
        { 255, 18, 0, false },
        NULL, 0, "9", NULL,
        UI_BTN_STYLE_NONE },
//b_denom
          { "02_b_denom_1", LV_OBJ_TYPE_LABEL, NULL,
            { 679,  89, 70, 36, 255, 255, 255 },
            { "1", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "1", NULL,
            UI_BTN_STYLE_NONE },

          { "02_b_denom_2", LV_OBJ_TYPE_LABEL, NULL,
            { 679, 120, 70, 36, 255, 255, 255 },
            { "2", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "2", NULL,
            UI_BTN_STYLE_NONE },

          { "02_b_denom_3", LV_OBJ_TYPE_LABEL, NULL,
            { 679, 151, 70, 36, 255, 255, 255 },
            { "3", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "3", NULL,
            UI_BTN_STYLE_NONE },

          { "02_b_denom_4", LV_OBJ_TYPE_LABEL, NULL,
            { 679, 182, 70, 36, 255, 255, 255 },
            { "4", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "4", NULL,
            UI_BTN_STYLE_NONE },

          { "02_b_denom_5", LV_OBJ_TYPE_LABEL, NULL,
            { 679, 213, 70, 36, 255, 255, 255 },
            { "5", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "5", NULL,
            UI_BTN_STYLE_NONE },

          { "02_b_denom_6", LV_OBJ_TYPE_LABEL, NULL,
            { 679, 244, 70, 36, 255, 255, 255 },
            { "6", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "6", NULL,
            UI_BTN_STYLE_NONE },

          { "02_b_denom_7", LV_OBJ_TYPE_LABEL, NULL,
            { 679, 275, 70, 36, 255, 255, 255 },
            { "7", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "7", NULL,
            UI_BTN_STYLE_NONE },

          { "02_b_denom_8", LV_OBJ_TYPE_LABEL, NULL,
            { 679, 306, 70, 36, 255, 255, 255 },
            { "8", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "8", NULL,
            UI_BTN_STYLE_NONE },

          { "02_b_denom_9", LV_OBJ_TYPE_LABEL, NULL,
            { 679, 337, 70, 36, 255, 255, 255 },
            { "9", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "9", NULL,
            UI_BTN_STYLE_NONE },
//************  c_REPORT ****************//
        { "02_c_no_1", LV_OBJ_TYPE_LABEL, NULL,
          { 834,  89, 70, 36, 255, 255, 255 },
          { "1", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "1", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_no_2", LV_OBJ_TYPE_LABEL, NULL,
          { 834, 120, 70, 36, 255, 255, 255 },
          { "2", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "2", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_no_3", LV_OBJ_TYPE_LABEL, NULL,
          { 834, 151, 70, 36, 255, 255, 255 },
          { "3", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "3", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_no_4", LV_OBJ_TYPE_LABEL, NULL,
          { 834, 182, 70, 36, 255, 255, 255 },
          { "4", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "4", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_no_5", LV_OBJ_TYPE_LABEL, NULL,
          { 834, 213, 70, 36, 255, 255, 255 },
          { "5", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "5", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_no_6", LV_OBJ_TYPE_LABEL, NULL,
          { 834, 244, 70, 36, 255, 255, 255 },
          { "6", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "6", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_no_7", LV_OBJ_TYPE_LABEL, NULL,
          { 834, 275, 70, 36, 255, 255, 255 },
          { "7", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "7", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_no_8", LV_OBJ_TYPE_LABEL, NULL,
          { 834, 306, 70, 36, 255, 255, 255 },
          { "8", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "8", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_no_9", LV_OBJ_TYPE_LABEL, NULL,
          { 834, 337, 70, 36, 255, 255, 255 },
          { "9", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "9", NULL,
          UI_BTN_STYLE_NONE },
//c_denom

          { "02_c_denom_1", LV_OBJ_TYPE_LABEL, NULL,
            { 887,  89, 70, 36, 255, 255, 255 },
            { "1", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "1", NULL,
            UI_BTN_STYLE_NONE },

          { "02_c_denom_2", LV_OBJ_TYPE_LABEL, NULL,
            { 887, 120, 70, 36, 255, 255, 255 },
            { "2", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "2", NULL,
            UI_BTN_STYLE_NONE },

          { "02_c_denom_3", LV_OBJ_TYPE_LABEL, NULL,
            { 887, 151, 70, 36, 255, 255, 255 },
            { "3", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "3", NULL,
            UI_BTN_STYLE_NONE },

          { "02_c_denom_4", LV_OBJ_TYPE_LABEL, NULL,
            { 887, 182, 70, 36, 255, 255, 255 },
            { "4", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "4", NULL,
            UI_BTN_STYLE_NONE },

          { "02_c_denom_5", LV_OBJ_TYPE_LABEL, NULL,
            { 887, 213, 70, 36, 255, 255, 255 },
            { "5", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "5", NULL,
            UI_BTN_STYLE_NONE },

          { "02_c_denom_6", LV_OBJ_TYPE_LABEL, NULL,
            { 887, 244, 70, 36, 255, 255, 255 },
            { "6", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "6", NULL,
            UI_BTN_STYLE_NONE },

          { "02_c_denom_7", LV_OBJ_TYPE_LABEL, NULL,
            { 887, 275, 70, 36, 255, 255, 255 },
            { "7", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "7", NULL,
            UI_BTN_STYLE_NONE },

          { "02_c_denom_8", LV_OBJ_TYPE_LABEL, NULL,
            { 887, 306, 70, 36, 255, 255, 255 },
            { "8", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "8", NULL,
            UI_BTN_STYLE_NONE },

          { "02_c_denom_9", LV_OBJ_TYPE_LABEL, NULL,
            { 887, 337, 70, 36, 255, 255, 255 },
            { "9", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
            { 255, 18, 0, false },
            NULL, 0, "9", NULL,
            UI_BTN_STYLE_NONE },
//c_err_code
        { "02_c_reject_1", LV_OBJ_TYPE_LABEL, NULL,
          { 973,  89, 200, 36, 255, 255, 255 },
          { "1", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "1", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_reject_2", LV_OBJ_TYPE_LABEL, NULL,
          { 973, 120, 200, 36, 255, 255, 255 },
          { "2", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "2", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_reject_3", LV_OBJ_TYPE_LABEL, NULL,
          { 973, 151, 200, 36, 255, 255, 255 },
          { "3", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "3", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_reject_4", LV_OBJ_TYPE_LABEL, NULL,
          { 973, 182, 200, 36, 255, 255, 255 },
          { "4", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "4", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_reject_5", LV_OBJ_TYPE_LABEL, NULL,
          { 973, 213, 200, 36, 255, 255, 255 },
          { "5", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "5", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_reject_6", LV_OBJ_TYPE_LABEL, NULL,
          { 973, 244, 200, 36, 255, 255, 255 },
          { "6", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "6", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_reject_7", LV_OBJ_TYPE_LABEL, NULL,
          { 973, 275, 200, 36, 255, 255, 255 },
          { "7", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "7", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_reject_8", LV_OBJ_TYPE_LABEL, NULL,
          { 973, 306, 200, 36, 255, 255, 255 },
          { "8", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "8", NULL,
          UI_BTN_STYLE_NONE },

        { "02_c_reject_9", LV_OBJ_TYPE_LABEL, NULL,
          { 973, 337, 200, 36, 255, 255, 255 },
          { "9", 112, 112, 112, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT },
          { 255, 18, 0, false },
          NULL, 0, "9", NULL,
          UI_BTN_STYLE_NONE },

        { "02_page_curr", LV_OBJ_TYPE_LABEL, NULL,
          { 1135, 58, 140, 36, 255, 255, 255 },
          { "USD", 0, 115, 255, &lv_font_montserrat_42, LV_TEXT_ALIGN_CENTER },
          { 255, 18, 0, false },
          NULL, 0, 0, NULL,
          UI_BTN_STYLE_NONE },

        { "02_a_page_refre", LV_OBJ_TYPE_LABEL, NULL,
          { 151, 371, 99, 27, 121, 150, 0 },
          { "1/2", 255, 255, 255, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
          { 255, 18, 0, false },
          NULL, 0, 0, NULL,
          UI_BTN_STYLE_NONE },
                
        { "02_b_page_refre", LV_OBJ_TYPE_LABEL, NULL,
          { 545, 371, 99, 27, 121, 150, 0 },
          { "1/2", 255, 255, 255, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
          { 255, 18, 0, false },
          NULL, 0, 0, NULL,
          UI_BTN_STYLE_NONE },
                
        { "02_c_page_refre", LV_OBJ_TYPE_LABEL, NULL,
          { 925, 371, 99, 27, 121, 150, 0 },
          { "200/200", 255, 255, 255, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
          { 255, 18, 0, false },
          NULL, 0, 0, NULL,
          UI_BTN_STYLE_NONE },

};



void ui_page_02_list_create(lv_obj_t* parent)
{
    page_02_report_init();

    //creat page_main 
    if (list_page) return;
    list_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(list_page);
    lv_obj_set_pos(list_page, 0, 0);
    lv_obj_set_size(list_page, 1280, 400);
    lv_obj_clear_flag(list_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(list_page, LV_SCROLLBAR_MODE_OFF); // 滚动条关闭

    // 计算数组长度
    page_02_list_len = sizeof(page_02_list_obj) / sizeof(ui_element_t);
    
    //创建图片
    lv_ui_obj_init(list_page, page_02_list_obj, page_02_list_len);
    page_02_a_page_refre();
    page_02_b_page_refre();
    page_02_c_page_refre();
    page_02_curr_refre();
    page_02_a_page_num_refre();
    page_02_b_page_num_refre();
    page_02_c_page_num_refre();

}

void ui_page_02_list_destroy(void)
{
    if (list_page) {
        lv_obj_del(list_page);
        list_page = NULL;
    }
}
