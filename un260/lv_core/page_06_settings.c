#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "un260/lv_system/user_cfg.h"
#include <string.h>
#include "lvgl/lvgl.h"
#include "../aic_ui/aic_ui.h"

/* =========================
 * 静态对象指针
 * ========================= */
static lv_obj_t* left_menu_container = NULL;

/* 菜单按钮和标签 */
static lv_obj_t* btn_system = NULL;
static lv_obj_t* btn_maintenance = NULL;
static lv_obj_t* btn_user = NULL;
static lv_obj_t* btn_version = NULL;
static lv_obj_t* btn_data_collection = NULL;

static lv_obj_t* label_system = NULL;
static lv_obj_t* label_maintenance = NULL;
static lv_obj_t* label_user = NULL;
static lv_obj_t* label_version = NULL;
static lv_obj_t* label_data_collection = NULL;

/* 子页面 */
static lv_obj_t* system_page = NULL;
static lv_obj_t* maintenance_page = NULL;
static lv_obj_t* user_page = NULL;
static lv_obj_t* version_page = NULL;
static lv_obj_t* data_collection_page = NULL;

/* 当前菜单索引 */
static int current_menu_index = -1;

/* =========================
 * 前置声明
 * ========================= */
static void menu_btn_event_cb(lv_event_t* e);
static void page_06_update_menu_state(int index);
static void page_06_switch_sub_page(int index);

/* =========================
 * UI元素定义（使用ui_element_t结构）
 * ========================= */
ui_element_t page_06_settings_obj[] = {
    //////////////////////////////////////////////////////
    //***************    BG_IMG_LIST  *******************//
    //////////////////////////////////////////////////////
    { "06_settings_bg_img", LV_OBJ_TYPE_IMAGE, &page_05_set_password_bg_img,
        { 0, 0, 1280, 400, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL, UI_BTN_STYLE_NONE },

        //////////////////////////////////////////////////////
        //***************    BTN_LIST   *********************/
        //////////////////////////////////////////////////////
        { "06_home_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 1149, 189, 100, 100, 244, 244, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, false },
            page_01_back_btn_event_cb, 0, NULL, NULL, UI_BTN_STYLE_APPLE },

            //////////////////////////////////////////////////////
            //***************  LABEL_LIST **********************//
            //////////////////////////////////////////////////////
            { "06_settings_title_label", LV_OBJ_TYPE_LABEL, NULL,
                { 620, 13, 150, 36, 112, 112, 112 },
                { "SETTINGS", 112, 112, 112, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
                { 255, 18, 0, false },
                NULL, 0, NULL, NULL, UI_BTN_STYLE_NONE },
};

int page_06_settings_obj_len = sizeof(page_06_settings_obj) / sizeof(page_06_settings_obj[0]);

/* =========================
 * 菜单状态刷新
 * ========================= */
static void page_06_update_menu_state(int index)
{
    lv_obj_t* btns[] = {
        btn_system,
        btn_maintenance,
        btn_user,
        btn_version,
        btn_data_collection
    };

    lv_obj_t* labels[] = {
        label_system,
        label_maintenance,
        label_user,
        label_version,
        label_data_collection
    };

    for (int i = 0; i < 5; i++) {
        if (i == index) {
            /* 选中 */
            lv_obj_set_style_bg_color(btns[i], lv_color_make(60, 120, 240), 0);
            lv_obj_set_style_text_color(labels[i], lv_color_white(), 0);
        }
        else {
            /* 未选中 */
            lv_obj_set_style_bg_color(btns[i], lv_color_make(240, 240, 240), 0);
            lv_obj_set_style_text_color(labels[i], lv_color_black(), 0);
        }
    }

    current_menu_index = index;
}

/* =========================
 * 子页面切换
 * ========================= */
static void page_06_switch_sub_page(int index)
{
    lv_obj_add_flag(system_page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(maintenance_page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(user_page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(version_page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(data_collection_page, LV_OBJ_FLAG_HIDDEN);

    switch (index) {
    case 0: lv_obj_clear_flag(system_page, LV_OBJ_FLAG_HIDDEN); break;
    case 1: lv_obj_clear_flag(maintenance_page, LV_OBJ_FLAG_HIDDEN); break;
    case 2: lv_obj_clear_flag(user_page, LV_OBJ_FLAG_HIDDEN); break;
    case 3: lv_obj_clear_flag(version_page, LV_OBJ_FLAG_HIDDEN); break;
    case 4: lv_obj_clear_flag(data_collection_page, LV_OBJ_FLAG_HIDDEN); break;
    default: break;
    }
}

/* =========================
 * 菜单事件回调
 * ========================= */
static void menu_btn_event_cb(lv_event_t* e)
{
    int index = (int)(intptr_t)lv_event_get_user_data(e);

    if (index == current_menu_index) {
        return;
    }

    page_06_update_menu_state(index);
    page_06_switch_sub_page(index);
}

/* =========================
 * 创建左侧菜单
 * ========================= */

static void create_menu_btn(lv_obj_t* parent, lv_obj_t** btn, lv_obj_t** label,const char* text, lv_coord_t y, uint8_t index)
{
    *btn = lv_btn_create(parent);
    lv_obj_set_size(*btn, 280, 60);
    lv_obj_set_pos(*btn, 10, y);
    lv_obj_clear_flag(*btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(*btn, menu_btn_event_cb,LV_EVENT_CLICKED, (void*)(uintptr_t)index);

    *label = lv_label_create(*btn);
    lv_label_set_text(*label, text);
    lv_obj_set_style_text_font(*label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(*label, lv_color_black(), 0);
    lv_obj_center(*label);
}


static void create_left_menu(lv_obj_t* parent)
{
    left_menu_container = lv_obj_create(parent);
    lv_obj_set_size(left_menu_container, 300, 400);
    lv_obj_set_pos(left_menu_container, 0, 0);
    lv_obj_clear_flag(left_menu_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(left_menu_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(left_menu_container, lv_color_make(235, 235, 235), 0);
    lv_obj_set_style_border_width(left_menu_container, 0, 0);

    create_menu_btn(left_menu_container, &btn_system, &label_system,"System", 20, 0);

    create_menu_btn(left_menu_container, &btn_maintenance, &label_maintenance,"Maintenance", 90, 1);

    create_menu_btn(left_menu_container, &btn_user, &label_user,"User", 160, 2);

    create_menu_btn(left_menu_container, &btn_version, &label_version, "Version", 230, 3);

    create_menu_btn(left_menu_container, &btn_data_collection, &label_data_collection,"Data Collection", 300, 4);
}

/* =========================
 * 创建子页面
 * ========================= */
static lv_obj_t* create_sub_page(lv_obj_t* parent, const char* text)
{
    lv_obj_t* page = lv_obj_create(parent);
    lv_obj_set_size(page, 980, 400);
    lv_obj_set_pos(page, 310, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(page, lv_color_white(), 0);

    lv_obj_t* label = lv_label_create(page);
    lv_label_set_text(label, text);
    lv_obj_center(label);

    return page;
}

/* =========================
 * 页面创建
 * ========================= */
void ui_page_06_settings_create(lv_obj_t* parent)
{
    if (settings_page) return;

    // 创建主页面容器
    settings_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(settings_page);
    lv_obj_set_pos(settings_page, 0, 0);
    lv_obj_set_size(settings_page, 1280, 400);
    lv_obj_clear_flag(settings_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(settings_page, LV_SCROLLBAR_MODE_OFF);

    // 使用ui_element_t初始化基础UI（背景、返回按钮、标题）
    lv_ui_obj_init(settings_page, page_06_settings_obj, page_06_settings_obj_len);

    // 创建左侧菜单
    create_left_menu(settings_page);

    // 创建右侧子页面
    system_page = create_sub_page(settings_page, "System Settings");
    maintenance_page = create_sub_page(settings_page, "Maintenance Settings");
    user_page = create_sub_page(settings_page, "User Settings");
    version_page = create_sub_page(settings_page, "Version Information");
    data_collection_page = create_sub_page(settings_page, "Data Collection Settings");

    // 默认显示 Maintenance 页面（索引1）
    page_06_update_menu_state(1);
    page_06_switch_sub_page(1);
}

/* =========================
 * 页面销毁
 * ========================= */
void ui_page_06_settings_destroy(void)
{
    if (!settings_page) {
        return;
    }

    lv_obj_del(settings_page);
    settings_page = NULL;

    left_menu_container = NULL;

    btn_system = btn_maintenance = btn_user =
        btn_version = btn_data_collection = NULL;

    label_system = label_maintenance = label_user =
        label_version = label_data_collection = NULL;

    system_page = maintenance_page = user_page =
        version_page = data_collection_page = NULL;

    current_menu_index = -1;
}
