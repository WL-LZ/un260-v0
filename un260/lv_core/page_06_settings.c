#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "un260/lv_system/user_cfg.h"
#include <string.h>
#include "lvgl/lvgl.h"
#include "../aic_ui/aic_ui.h"
#include "un260/lv_core/page_09_cis_cala.h"
/* 版本信息 */
static lv_obj_t* label_main_app;
static lv_obj_t* label_image_app;
static lv_obj_t* label_fpga;
static lv_obj_t* label_thka;
static lv_obj_t* label_ecb;
static lv_obj_t* label_display;

/* system 子页面升级按钮 */
static lv_obj_t* btn_upgrade_menu = NULL;


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
static lv_obj_t* btn_cis_calib = NULL;



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
//数据采集
static lv_obj_t* dc_btn_all = NULL;
static lv_obj_t* dc_btn_false = NULL;
static lv_obj_t* dc_btn_start = NULL;
static lv_obj_t* dc_btn_disable = NULL;

static lv_obj_t* dc_label_all = NULL;
static lv_obj_t* dc_label_false = NULL;
static lv_obj_t* dc_check_all = NULL;
static lv_obj_t* dc_check_false = NULL;

static lv_obj_t* dc_mode_value_label = NULL;
static lv_obj_t* dc_pcs_label = NULL;
static lv_obj_t* dc_status_label = NULL;

/* 当前菜单索引 */
static int current_menu_index = -1;

/* =========================
 * 前置声明
 * ========================= */
static void menu_btn_event_cb(lv_event_t* e);
static void page_06_update_menu_state(int index);
static void page_06_switch_sub_page(int index);
//数据采集
static void create_data_collection_page_content(lv_obj_t* parent);
static void data_collect_mode_btn_event_cb(lv_event_t* e);
static void data_collect_start_btn_event_cb(lv_event_t* e);
static void data_collect_disable_btn_event_cb(lv_event_t* e);
static void update_data_collect_btn_style(lv_obj_t* btn, lv_obj_t* label, lv_obj_t* check, bool selected);
static const char* get_data_collect_mode_name(data_collect_mode_t mode);
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

/* 版本信息 */
static lv_obj_t* create_version_label(lv_obj_t* parent,lv_coord_t x,lv_coord_t y,const char* text)
{
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    return label;
}

/* 版本详情 */
static void create_version_page(lv_obj_t* parent)
{
    char buf[64];
    lv_coord_t y = 40;

    if (!Machine_Statue.version_valid) {
        create_version_label(parent, 40, y, "Version not available");
        return;
    }

    snprintf(buf, sizeof(buf), "Main App     : %s", Machine_Statue.main_app);
    label_main_app = create_version_label(parent, 40, y, buf);
    y += 40;

    snprintf(buf, sizeof(buf), "Image App    : %s", Machine_Statue.image_app);
    label_image_app = create_version_label(parent, 40, y, buf);
    y += 40;

    snprintf(buf, sizeof(buf), "FPGA         : %s", Machine_Statue.fpga);
    label_fpga = create_version_label(parent, 40, y, buf);
    y += 40;

    snprintf(buf, sizeof(buf), "Main BOOT    : %s", Machine_Statue.thka_app);
    label_thka = create_version_label(parent, 40, y, buf);
    y += 40;

    snprintf(buf, sizeof(buf), "Image BOOT   : %s", Machine_Statue.ecb);
    label_ecb = create_version_label(parent, 40, y, buf);
    y += 40;

    snprintf(buf, sizeof(buf), "Display App  : %s", Machine_Statue.display_app);
    label_display = create_version_label(parent, 40, y, buf);
}

static void debug_enter_btn_cb(lv_event_t* e)
{
    ui_manager_switch(UI_PAGE_DEBUG);
}

static void sensor_enter_btn_cb(lv_event_t* e)
{
    (void)e;
    ui_manager_push_page(UI_PAGE_SENSOR);
}

static void upgrade_enter_btn_cb(lv_event_t* e)
{
    (void)e;
    ui_manager_push_page(UI_PAGE_UPGRADE);
}

static void create_system_page_content(lv_obj_t* parent)
{
    btn_upgrade_menu = lv_btn_create(parent);
    lv_obj_set_size(btn_upgrade_menu, 360, 80);
    lv_obj_set_pos(btn_upgrade_menu, 54, 70);
    lv_obj_add_event_cb(btn_upgrade_menu, upgrade_enter_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* lbl_upgrade_menu = lv_label_create(btn_upgrade_menu);
    lv_label_set_text(lbl_upgrade_menu, "UPGRADE");
    lv_obj_center(lbl_upgrade_menu);
}
static void motor_test_enter_btn_cb(lv_event_t* e)
{
    (void)e;
    ui_manager_push_page(UI_PAGE_MOTOR_TEST);
}
static void create_maintenance_page_content(lv_obj_t* parent)
{
    btn_cis_calib = lv_btn_create(parent);
    lv_obj_set_size(btn_cis_calib, 300, 80);
    lv_obj_set_pos(btn_cis_calib, 54, 7);
    lv_obj_add_event_cb(btn_cis_calib, cis_enter_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* label = lv_label_create(btn_cis_calib);
    lv_label_set_text(label, "CIS Calibration");
    lv_obj_center(label);

    lv_obj_t* btn_motor = lv_btn_create(parent);
    lv_obj_set_size(btn_motor, 300, 80);
    lv_obj_set_pos(btn_motor, 380, 7);
    lv_obj_add_event_cb(btn_motor, motor_test_enter_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* label_2 = lv_label_create(btn_motor);
    lv_label_set_text(label_2, "MOTOR TEST");
    lv_obj_center(label_2);

    lv_obj_t* btn_debug = lv_btn_create(parent);
    lv_obj_set_size(btn_debug, 300, 80);
    lv_obj_set_pos(btn_debug, 54, 104);
    lv_obj_add_event_cb(btn_debug, debug_enter_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* label_1 = lv_label_create(btn_debug);
    lv_label_set_text(label_1, "DEBUG");
    lv_obj_center(label_1);

    lv_obj_t* btn_sensor = lv_btn_create(parent);
    lv_obj_set_size(btn_sensor, 300, 80);
    lv_obj_set_pos(btn_sensor, 54, 201);
    lv_obj_add_event_cb(btn_sensor, sensor_enter_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* label_3 = lv_label_create(btn_sensor);
    lv_label_set_text(label_3, "SENSOR PARAMETERS");
    lv_obj_center(label_3);
}
//数据采集
static const char* get_data_collect_mode_name(data_collect_mode_t mode)
{
    switch (mode) {
    case DATA_COLLECT_MODE_ALL:
        return "ALL DATA";
    case DATA_COLLECT_MODE_FALSE:
        return "ERROR DATA";
    default:
        return " ";
    }
}

static void update_data_collect_btn_style(lv_obj_t* btn, lv_obj_t* label, lv_obj_t* check, bool selected)
{
    if (!btn || !label || !check) return;

    lv_obj_set_style_bg_color(btn,
        selected ? lv_color_make(60, 120, 240) : lv_color_make(20, 190, 170), 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, 12, 0);

    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_color(check, lv_color_white(), 0);

    lv_label_set_text(check, selected ? LV_SYMBOL_OK : "");
    lv_obj_align(check, LV_ALIGN_RIGHT_MID, -16, 0);
}

void page_06_data_collection_refresh(void)
{
    if (!data_collection_page) return;

    update_data_collect_btn_style(
        dc_btn_all, dc_label_all, dc_check_all,
        g_data_collect_mode == DATA_COLLECT_MODE_ALL);

    update_data_collect_btn_style(
        dc_btn_false, dc_label_false, dc_check_false,
        g_data_collect_mode == DATA_COLLECT_MODE_FALSE);

    if (dc_mode_value_label) {
        lv_label_set_text_fmt(dc_mode_value_label, "%s", get_data_collect_mode_name(g_data_collect_mode));
    }

    if (dc_pcs_label) {
        lv_label_set_text_fmt(dc_pcs_label, "PCS:%d", g_data_collect_pcs);
    }

    if (dc_status_label) {
        lv_label_set_text(dc_status_label, g_data_collect_status);
    }
}

static void data_collect_mode_btn_event_cb(lv_event_t* e)
{
    uint8_t sub = (uint8_t)(uintptr_t)lv_event_get_user_data(e);

    if (sub == 0x01) {
        g_data_collect_mode = DATA_COLLECT_MODE_ALL;
        snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                 "Requesting ALL DATA collection mode...");
    } else if (sub == 0x02) {
        g_data_collect_mode = DATA_COLLECT_MODE_FALSE;
        snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                 "Requesting FALSE REPORT collection mode...");
    } else {
        return;
    }

    g_data_collect_pcs = 0;
    page_06_data_collection_refresh();
    send_command(fd4, 0xC0, &sub, 1);
}

static void data_collect_start_btn_event_cb(lv_event_t* e)
{
    (void)e;

    if (g_data_collect_mode == DATA_COLLECT_MODE_NONE) {
        snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                 "Please select a collection mode first");
        page_06_data_collection_refresh();
        return;
    }

    uint8_t start_cmd = 0x01;
    g_data_collect_pcs = 0;
    snprintf(g_data_collect_status, sizeof(g_data_collect_status),
             "Counting command sent. Waiting for controller reply...");
    page_06_data_collection_refresh();
    send_command(fd4, 0x0A, &start_cmd, 1);
}

static void data_collect_disable_btn_event_cb(lv_event_t* e)
{
    (void)e;

    uint8_t sub = 0xFF;

    g_data_collect_mode = DATA_COLLECT_MODE_NONE;
    g_data_collect_pcs = 0;
    snprintf(g_data_collect_status, sizeof(g_data_collect_status),
             "Exiting collection mode");
    page_06_data_collection_refresh();

    send_command(fd4, 0xC0, &sub, 1);
}

static lv_obj_t* create_dc_mode_button(lv_obj_t* parent,
                                       lv_coord_t x, lv_coord_t y,
                                       const char* text,
                                       lv_event_cb_t cb,
                                       uint8_t sub,
                                       lv_obj_t** out_label,
                                       lv_obj_t** out_check)
{
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 360, 54);
    lv_obj_set_pos(btn, x, y);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, (void*)(uintptr_t)sub);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 18, 0);

    lv_obj_t* check = lv_label_create(btn);
    lv_label_set_text(check, "");
    lv_obj_align(check, LV_ALIGN_RIGHT_MID, -16, 0);

    if (out_label) *out_label = label;
    if (out_check) *out_check = check;

    return btn;
}

static void create_data_collection_page_content(lv_obj_t* parent)
{
    /* 左上两个模式按钮 */
    dc_btn_all = create_dc_mode_button(parent, 40, 28,"ALL DATA",data_collect_mode_btn_event_cb, 0x01,&dc_label_all, &dc_check_all);
    dc_btn_false = create_dc_mode_button(parent, 40, 100,"ERROR REPORT",data_collect_mode_btn_event_cb, 0x02,&dc_label_false, &dc_check_false);

    /* 右上开始按钮 */
    /* START 按钮 */
    dc_btn_start = lv_btn_create(parent);
    lv_obj_set_size(dc_btn_start, 220, 68);
    lv_obj_set_pos(dc_btn_start, 560, 14);
    lv_obj_clear_flag(dc_btn_start, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(dc_btn_start, lv_color_make(0, 180, 220), 0);
    lv_obj_set_style_border_width(dc_btn_start, 0, 0);
    lv_obj_set_style_radius(dc_btn_start, 14, 0);
    lv_obj_add_event_cb(dc_btn_start, data_collect_start_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* start_label = lv_label_create(dc_btn_start);
    lv_label_set_text(start_label, "START");
    lv_obj_set_style_text_font(start_label, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(start_label, lv_color_white(), 0);
    lv_obj_center(start_label);

    /* DISABLE 按钮 */
    dc_btn_disable = lv_btn_create(parent);
    lv_obj_set_size(dc_btn_disable, 220, 68);
    lv_obj_set_pos(dc_btn_disable, 560, 98);
    lv_obj_clear_flag(dc_btn_disable, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(dc_btn_disable, lv_color_make(150, 150, 150), 0);
    lv_obj_set_style_border_width(dc_btn_disable, 0, 0);
    lv_obj_set_style_radius(dc_btn_disable, 14, 0);
    lv_obj_add_event_cb(dc_btn_disable, data_collect_disable_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* disable_label = lv_label_create(dc_btn_disable);
    lv_label_set_text(disable_label, "DISABLE");
    lv_obj_set_style_text_font(disable_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(disable_label, lv_color_white(), 0);
    lv_obj_center(disable_label);

    /* 下方状态卡片 */
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, 900, 165);
    lv_obj_set_pos(card, 40, 185);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(card, 10, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_bg_color(card, lv_color_make(248, 248, 248), 0);

    lv_obj_t* mode_title = lv_label_create(card);
    lv_label_set_text(mode_title, "COLLECTION MODE");
    lv_obj_set_style_text_color(mode_title, lv_color_make(0, 180, 220), 0);
    lv_obj_set_style_text_font(mode_title, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(mode_title, 22, 8);

    dc_mode_value_label = lv_label_create(card);
    lv_label_set_text(dc_mode_value_label, " ");
    lv_obj_set_style_text_font(dc_mode_value_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(dc_mode_value_label, lv_color_make(0, 150, 190), 0);
    lv_obj_set_pos(dc_mode_value_label, 22, 38);

    dc_pcs_label = lv_label_create(card);
    lv_label_set_text(dc_pcs_label, "PCS:0");
    lv_obj_set_style_text_font(dc_pcs_label, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(dc_pcs_label, lv_color_make(0, 150, 190), 0);
    lv_obj_align(dc_pcs_label, LV_ALIGN_TOP_MID, 0, 12);

    dc_status_label = lv_label_create(card);
    lv_label_set_text(dc_status_label, "Please select a collection mode");
    lv_obj_set_width(dc_status_label, 820);
    lv_label_set_long_mode(dc_status_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(dc_status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(dc_status_label, &lv_font_montserrat_22, 0);
    lv_obj_align(dc_status_label, LV_ALIGN_CENTER, 0, 22);

    page_06_data_collection_refresh();
}
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
    case 0:
        lv_obj_clear_flag(system_page, LV_OBJ_FLAG_HIDDEN);
        break;

    case 1:
        lv_obj_clear_flag(maintenance_page, LV_OBJ_FLAG_HIDDEN);
        break;

    case 2:
        lv_obj_clear_flag(user_page, LV_OBJ_FLAG_HIDDEN);
        break;

    case 3:
        lv_obj_clear_flag(version_page, LV_OBJ_FLAG_HIDDEN);
        break;

    case 4:
        g_data_collect_mode = DATA_COLLECT_MODE_NONE;
        g_data_collect_pcs = 0;
        snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                 "Please select a collection mode.");
        lv_obj_clear_flag(data_collection_page, LV_OBJ_FLAG_HIDDEN);
        page_06_data_collection_refresh();
        break;

    default:
        break;
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
    /* ===== ESC 按键 ===== */
    lv_obj_t* esc_btn = lv_btn_create(page);
    lv_obj_set_size(esc_btn, 100, 60);
    lv_obj_set_pos(esc_btn, 830, 7);   // 右上角
    lv_obj_clear_flag(esc_btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(esc_btn, page_01_back_btn_event_cb,
        LV_EVENT_CLICKED, NULL);
    lv_obj_t* esc_label = lv_label_create(esc_btn);
    lv_label_set_text(esc_label, "ESC");
    lv_obj_center(esc_label);
    if (text && text[0] != '\0') {
        lv_obj_t* label = lv_label_create(page);
        lv_label_set_text(label, text);
        lv_obj_center(label);
    }


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
    create_system_page_content(system_page);
    maintenance_page = create_sub_page(settings_page, " ");
    create_maintenance_page_content(maintenance_page);
    user_page = create_sub_page(settings_page, "User Settings");
    version_page = create_sub_page(settings_page, " ");
    data_collection_page = create_sub_page(settings_page, "");
    create_data_collection_page_content(data_collection_page);
    create_version_page(version_page);

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

    btn_upgrade_menu = NULL;
    dc_btn_all = NULL;
    dc_btn_false = NULL;
    dc_btn_start = NULL;
    dc_btn_disable = NULL;

    dc_label_all = NULL;
    dc_label_false = NULL;
    dc_check_all = NULL;
    dc_check_false = NULL;

    dc_mode_value_label = NULL;
    dc_pcs_label = NULL;
    dc_status_label = NULL;
}
