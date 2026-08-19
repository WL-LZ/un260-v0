#ifndef LV_PAGE_MANAGER_H
#define LV_PAGE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif
#include "un260/lv_resources/lv_img_init.h" 
    extern lv_obj_t* main_page;
    extern lv_obj_t* setting_page;
    extern lv_obj_t* menu_page;
    typedef enum {
        UI_PAGE_BOOT_ANIM = 0,
        UI_PAGE_MAIN,
        UI_PAGE_LIST,
        UI_PAGE_MENU,
        UI_PAGE_SETTING,
        UI_PAGE_DETAIL,
        UI_PAGE_SET_PASSAGE,
        UI_PAGE_CURR,
        UI_PAGE_BOOT,
        UI_PAGE_CIS_CALIB,
        UI_PAGE_DEBUG,
        UI_PAGE_TIMESET,
        UI_PAGE_SENSOR,
        UI_PAGE_UPGRADE,
        UI_PAGE_MAIN_UPGRADE,
        UI_PAGE_IMAGE_UPGRADE,
        UI_PAGE_UI_UPGRADE,
        UI_PAGE_MOTOR_TEST,
        UI_PAGE_PURE,
        UI_PAGE_HISTORY,
        UI_PAGE_PRINT_SETTING,
        UI_PAGE_LANGUAGE_SETTING,
        UI_PAGE_DOUBLE_NOTE_SETTING,
        UI_PAGE_FLAP_SETTING,
        UI_PAGE_REJECT_POCKET_SETTING,
        UI_PAGE_SERIAL_NUMBER_SETTING,
        UI_PAGE_AGING_SETTING,
        UI_PAGE_CFD_LEVEL_SETTING,
        UI_PAGE_IMAGE_GET,
        UI_PAGE_PASSWORD_CHANGE,
        UI_PAGE_FACTORY_SETTING,
        UI_PAGE_WAVE_GET,
        UI_PAGE_COUNT
    } ui_page_t;

    void ui_manager_init(void);     //page管理
    void ui_manager_switch(ui_page_t page);     //page切换
    void ui_manager_push_page(ui_page_t page);      //页面堆栈：进入新页面
    bool ui_manager_pop_page(void);         //页面堆栈：返回上一页
    void ui_manager_clear_stack(void);      //清空页面堆栈
    ui_page_t ui_manager_get_current_page(void);   //获取当前页
    typedef struct {
        ui_element_t* list;
        int len;
    }ui_element_group_t;


    extern ui_element_t page_01_main_obj[];
    extern int page_01_main_len;

    extern ui_element_t page_02_list_obj[];
    extern int page_02_list_len;

    extern ui_element_t page_03_menu_obj[];
    extern int page_03_menu_len;

    extern ui_element_t page_04_set_obj[];
    extern int page_04_set_obj_len;

    extern ui_element_t page_06_settins_password_obj[];
    extern int page_06_settins_password_len;

    extern ui_element_t page_07_curr_obj[];
    extern int page_07_curr_len;

    extern ui_element_group_t all_ui_groups[];
#ifdef __cplusplus
}
#endif

#endif // LV_PAGE_MANAGER_H
