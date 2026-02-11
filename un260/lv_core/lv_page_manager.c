
#include "lvgl/lvgl.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/page_04_set.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_refre/lvgl_refre.h"
#include"lv_page_declear.h"

// 定义页面对象
lv_obj_t* main_page = NULL;
lv_obj_t* setting_page = NULL;
lv_obj_t* list_page = NULL;
lv_obj_t* menu_page = NULL;
lv_obj_t* set_password_page = NULL;
lv_obj_t* settings_page = NULL;
lv_obj_t* curr_page = NULL;
lv_obj_t* boot_page = NULL;
lv_obj_t* cis_calib_page = NULL;
lv_obj_t* page_debug = NULL;
lv_obj_t* page_sensor = NULL;

ui_element_group_t all_ui_groups[] = {
    { page_01_main_obj, 0 },
    { page_02_list_obj, 0 },
    { page_03_menu_obj, 0 },
   // { page_04_set_obj,  0 },
};

static ui_page_t current_page = (ui_page_t)-1;
//销毁当前页面
static void destroy_current_page(void)
{
    if (current_page == UI_PAGE_MAIN) {
        if (main_page && lv_obj_is_valid(main_page)) {
            pause_counting_sim(); // 暂停计数
            lv_obj_add_flag(main_page, LV_OBJ_FLAG_HIDDEN); // 隐藏主页面
            return; // 直接返回，不执行销毁
        }
    }
    switch (current_page) {
    case UI_PAGE_MAIN:    ui_main_destroy();    break;
    case UI_PAGE_LIST:    ui_page_02_list_destroy();    break;
    case UI_PAGE_MENU:    ui_page_03_menu_destroy(); break;
    case UI_PAGE_SET_PASSAGE: ui_page_05_set_password_destroy(); break;
    case UI_PAGE_SETTING: ui_page_06_settings_destroy(); break;
    case UI_PAGE_CURR: ui_page_07_curr_destroy(); break;
    case UI_PAGE_BOOT: ui_page_08_curr_destroy(); break;
    case UI_PAGE_DETAIL:   break;
    case UI_PAGE_COUNT:   break;
    case UI_PAGE_CIS_CALIB: ui_page_cis_calib_destroy(); break;
    case UI_PAGE_TIMESET: ui_page_11_timeset_destroy(); break;
    case UI_PAGE_DEBUG: ui_page_10_debug_destroy(); break;
    case UI_PAGE_SENSOR: ui_page_12_sensor_destroy(); break;

    }
}

static void create_new_page(ui_page_t page)
{
    if (page == UI_PAGE_MAIN && main_page && lv_obj_is_valid(main_page)) {
        lv_obj_clear_flag(main_page, LV_OBJ_FLAG_HIDDEN); // 显示主页面
        resume_counting_sim(); // 恢复计数
        page_01_add_refre();
        page_01_work_refre();
        page_01_batch_refre();
        page_01_face_refre();
        page_01_cfd_refre();
        page_01_speed_refre();
        page_01_err_num_refre();
        page_01_curr_img_refre();
        //sim_data_init();
        ui_refresh_main_page();

        return; // 直接返回，不执行创建
    }
    switch (page) {
    case UI_PAGE_MAIN:    ui_main_create(lv_scr_act()); resume_counting_sim();   break;
    case UI_PAGE_LIST:    ui_page_02_list_create(lv_scr_act());    break;
    case UI_PAGE_MENU:    ui_page_03_menu_create(lv_scr_act()); break;
    case UI_PAGE_SET_PASSAGE: ui_page_05_set_password_create(lv_scr_act()); break;
    case UI_PAGE_SETTING: ui_page_06_settings_create(lv_scr_act()); break;
    case UI_PAGE_CURR: ui_page_07_curr_create(lv_scr_act()); break;
    case UI_PAGE_BOOT: ui_page_08_curr_create(lv_scr_act()); break;
    case UI_PAGE_CIS_CALIB: ui_page_cis_calib_create(lv_scr_act()); break;
    case UI_PAGE_DEBUG: ui_page_10_debug_create(); break;
    case UI_PAGE_TIMESET: ui_page_11_timeset_create(lv_scr_act()); break;
    case UI_PAGE_SENSOR: ui_page_12_sensor_create(lv_scr_act()); break;
    }
}


static lv_obj_t* page_objects[UI_PAGE_COUNT] = { NULL };

static void hide_current_page(void)
{
    if (current_page >= 0 && current_page < UI_PAGE_COUNT && page_objects[current_page]) {
        // 如果当前是主页面，暂停计数
        if (current_page == UI_PAGE_MAIN) {
            pause_counting_sim();
        }

        // 隐藏当前页面
        lv_obj_add_flag(page_objects[current_page], LV_OBJ_FLAG_HIDDEN);
    }
}

//static void show_page(ui_page_t page)
//{
//    // 如果页面对象不存在，创建它
//    if (page >= 0 && page < UI_PAGE_COUNT && !page_objects[page]) {
//        switch (page) {
//        case UI_PAGE_MAIN:
//            ui_main_create(lv_scr_act());
//            page_objects[UI_PAGE_MAIN] = main_page;
//            break;
//        case UI_PAGE_LIST:
//            ui_page_02_list_create(lv_scr_act());
//            page_objects[UI_PAGE_LIST] = list_page;
//            break;
//        case UI_PAGE_MENU:
//            ui_page_03_menu_create(lv_scr_act());
//            page_objects[UI_PAGE_MENU] = menu_page;
//            break;
//        case UI_PAGE_SETTING:
//            ui_setting_create(lv_scr_act());
//            page_objects[UI_PAGE_SETTING] = setting_page;
//            break;
//        }
//    }
//
//    // 显示页面
//    if (page >= 0 && page < UI_PAGE_COUNT && page_objects[page]) {
//        lv_obj_clear_flag(page_objects[page], LV_OBJ_FLAG_HIDDEN);
//
//        // 如果是主页面，恢复计数
//        if (page == UI_PAGE_MAIN) {
//            resume_counting_sim();
//        }
//    }
//}


void ui_manager_switch(ui_page_t page)
{

    if (page == current_page) return;
    destroy_current_page();
    create_new_page(page);
    current_page = page;
}


// 页面堆栈
#define MAX_PAGE 10
static ui_page_t page_stack[MAX_PAGE];
static int top = -1;
static page_data_t machine_data = { 0 };
void ui_manager_init(void) {
    // 初始化共享数据
    machine_data.count_value = 0;
    machine_data.batch_value = 100;
    machine_data.sp_level = 800;
    machine_data.is_auto_mode = true;

    // 更新ui_groups的长度值
    all_ui_groups[0].len = page_01_main_len;
    all_ui_groups[1].len = page_02_list_len;
    all_ui_groups[2].len = page_03_menu_len;

    // 初始化堆栈
    top = -1;

    // 显示主页面

    ui_manager_switch(UI_PAGE_MAIN);
    //sim_data_init();           // 初始化面额列表、count=0、amount=0
    sim_clear_all_sn(&sim);

}


void ui_manager_push_page(ui_page_t page)
{

    if (page == current_page)  return;

    //当前页入栈
    if (top < MAX_PAGE - 1 && current_page != (ui_page_t)-1)  //当前页有效
    {
        page_stack[++top] = current_page;

    }
    else
    {
        for (int i = 0; i < MAX_PAGE - 1; i++)
            page_stack[i] = page_stack[i + 1];
        page_stack[MAX_PAGE - 1] = current_page;

    }
    ui_manager_switch(page);
}

bool ui_manager_pop_page(void)
{
    if (top < 0)
    {
        // 栈为空时，判断是否需要返回主页面
        if (current_page != (ui_page_t)-1 && current_page != UI_PAGE_MAIN) {
            ui_manager_switch(UI_PAGE_MAIN);
            return true;
        }
        return false;  
    }

    // 出栈
    ui_page_t prev_page = page_stack[top--];
    ui_manager_switch(prev_page);
    return true;
}


// 读取当前页
ui_page_t ui_manager_get_current_page(void) {
    return current_page;
}



// 获取机器状态
page_data_t* ui_manager_get_data(void) {
    return &machine_data;
}
