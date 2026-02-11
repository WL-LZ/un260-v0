#ifndef LV_PAGE_MANAGER_H
#define LV_PAGE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif
#include "un260/lv_resources/lv_img_init.h" 
    extern lv_obj_t* main_page;
    extern lv_obj_t* setting_page;
    extern lv_obj_t* list_page;
    extern lv_obj_t* menu_page;
    extern lv_obj_t* set_password_page;
    extern lv_obj_t* settings_page;
    extern lv_obj_t* curr_page;
    extern lv_obj_t* boot_page;
    extern lv_obj_t* cis_calib_page;
    extern lv_obj_t* page_debug;
    extern lv_obj_t* page_sensor;


    typedef enum {
        UI_PAGE_MAIN = 0,
        UI_PAGE_LIST,
        UI_PAGE_MENU,
        UI_PAGE_SETTING,
        UI_PAGE_DETAIL,
        UI_PAGE_SET_PASSAGE,
        UI_PAGE_CURR,
        // 更多页面...
        UI_PAGE_BOOT,
        UI_PAGE_CIS_CALIB,
        UI_PAGE_DEBUG,
        UI_PAGE_TIMESET,
        UI_PAGE_SENSOR,
        UI_PAGE_COUNT
    } ui_page_t;

    void ui_manager_init(void);     //page管理
    void ui_manager_switch(ui_page_t page);     //page切换
    void ui_manager_push_page(ui_page_t page);      //页面堆栈：进入新页面
    bool ui_manager_pop_page(void);         //页面堆栈：返回上一页
    ui_page_t ui_manager_get_current_page(void);   //获取当前页
    typedef struct {
        int count_value;
        int batch_value;
        int sp_level;
        bool is_auto_mode;

    }page_data_t;

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

    extern ui_element_t page_05_set_password_obj[];
    extern int page_05_set_password_len;


    extern ui_element_t page_06_settins_password_obj[];
    extern int page_06_settins_password_len;

    extern ui_element_t page_07_curr_obj[];
    extern int page_07_curr_len;

    extern ui_element_group_t all_ui_groups[];
#ifdef __cplusplus
}
#endif

#endif // LV_PAGE_MANAGER_H
