#include "page_16_ui_upgrade.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/settings_detail_ui.h"
#include "un260/lv_core/ui_upgrade_service.h"
#include "un260/lv_components/lv_upgrade_popup.h"
#include "un260/lv_system/ui_text.h"

#include <stdbool.h>

static lv_obj_t*   upgrade_page = NULL;
static lv_obj_t*   upgrade_usb_status_label = NULL;
static lv_obj_t*   upgrade_file_status_label = NULL;
static lv_obj_t*   upgrade_hint_label = NULL;
static lv_timer_t* upgrade_status_timer = NULL;

static lv_obj_t*   g_upgrade_btn = NULL;
static lv_obj_t*   g_esc_btn = NULL;

static lv_obj_t*   g_arc = NULL;
static lv_obj_t*   g_arc_label = NULL;
static lv_obj_t*   g_poweroff_label = NULL;

static lv_timer_t* g_wait_timer = NULL;
static lv_timer_t* g_upgrade_result_timer = NULL;
static uint32_t    g_wait_sec = 0;
static bool        g_upgrading = false;

static lv_anim_t   g_arc_anim;
static bool        g_arc_anim_inited = false;

static const char* page_16_text_get(ui_text_id_t text_id) //获取升级页面当前语言文本
{
    return ui_text_get(text_id);
}

static void update_upgrade_status(void) //刷新U盘/挂载/包状态（升级中不刷新）
{
    ui_upgrade_detect_info_t detect_info;

    if (g_upgrading) return;
    ui_upgrade_service_detect(&detect_info);

    if (upgrade_usb_status_label && lv_obj_is_valid(upgrade_usb_status_label)) {
        lv_label_set_text_fmt(upgrade_usb_status_label, page_16_text_get(UI_TEXT_PAGE16_USB_STATUS_FMT),
                              detect_info.usb_present ?
                              page_16_text_get(UI_TEXT_PAGE16_USB_INSERTED) :
                              page_16_text_get(UI_TEXT_PAGE16_USB_NOT_INSERTED));
        lv_obj_set_style_text_color(upgrade_usb_status_label,
            detect_info.usb_present ? lv_color_hex(0x1F9D55) : lv_color_hex(0xC03A2B), 0);
    }

    if (upgrade_file_status_label && lv_obj_is_valid(upgrade_file_status_label)) {
        lv_label_set_text_fmt(upgrade_file_status_label,
            page_16_text_get(UI_TEXT_PAGE16_FILE_STATUS_FMT),
            detect_info.usb_mounted ?
            page_16_text_get(UI_TEXT_PAGE16_MOUNT_OK) :
            page_16_text_get(UI_TEXT_PAGE16_MOUNT_NOT),
            detect_info.package_found ?
            page_16_text_get(UI_TEXT_PAGE16_PACKAGE_FOUND) :
            page_16_text_get(UI_TEXT_PAGE16_PACKAGE_NOT_FOUND));
    }

    if (upgrade_hint_label && lv_obj_is_valid(upgrade_hint_label)) {
        if (!detect_info.usb_present) {
            lv_label_set_text(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_INSERT_USB));
            lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0xC03A2B), 0);
        } else if (!detect_info.usb_mounted) {
            lv_label_set_text(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_INSERTED_CLICK_UPGRADE));
            lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0xC07A2B), 0);
        } else if (!detect_info.package_found) {
            lv_label_set_text(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_MISSING_PACKAGE));
            lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0xC07A2B), 0);
        } else {
            lv_label_set_text(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_READY));
            lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0x1F9D55), 0);
        }
    }
}

static void upgrade_status_timer_cb(lv_timer_t* timer) //定时刷新状态
{
    (void)timer;
    update_upgrade_status();
}

static void arc_set_value_cb(void* obj, int32_t v) //圆环动画回调：设置0-90并更新文本
{
    lv_obj_t* arc = (lv_obj_t*)obj;
    if (!arc || !lv_obj_is_valid(arc)) return;

    if (v > 90) v = 90;
    if (v < 0)  v = 0;

    lv_arc_set_value(arc, (int16_t)v);

    if (g_arc_label && lv_obj_is_valid(g_arc_label)) {
        lv_label_set_text_fmt(g_arc_label, "%d%%", (int)v);
    }
}

static void wait_timer_cb(lv_timer_t* t) //升级中：每秒更新提示文字
{
    (void)t;
    g_wait_sec++;

    if (upgrade_hint_label && lv_obj_is_valid(upgrade_hint_label)) {
        lv_label_set_text_fmt(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_UPGRADING_WAIT_FMT), (unsigned)g_wait_sec);
        lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0x1F9D55), 0);
    }

    if (g_wait_sec == 60 && g_poweroff_label && lv_obj_is_valid(g_poweroff_label)) {
        lv_label_set_text(g_poweroff_label, page_16_text_get(UI_TEXT_PAGE16_POWER_OFF_LONG));
    }
}

static void upgrade_waiting_exit(bool success)
{
    if (g_wait_timer) {
        lv_timer_del(g_wait_timer);
        g_wait_timer = NULL;
    }
    if (g_upgrade_result_timer) {
        lv_timer_del(g_upgrade_result_timer);
        g_upgrade_result_timer = NULL;
    }

    g_upgrading = false;

    if (g_upgrade_btn) lv_obj_clear_state(g_upgrade_btn, LV_STATE_DISABLED);
    if (g_esc_btn)     lv_obj_clear_state(g_esc_btn, LV_STATE_DISABLED);

    if (g_arc && lv_obj_is_valid(g_arc)) {
        lv_anim_del(g_arc, arc_set_value_cb);
    }

    if (g_poweroff_label && lv_obj_is_valid(g_poweroff_label)) {
        lv_label_set_text(g_poweroff_label,
                          success ?
                          page_16_text_get(UI_TEXT_PAGE16_HINT_FINISHED) :
                          page_16_text_get(UI_TEXT_PAGE16_HINT_FAILED));
    }
}

static void upgrade_result_timer_cb(lv_timer_t* timer)
{
    ui_upgrade_service_status_t status;

    (void)timer;

    ui_upgrade_service_poll(&status);

    if (g_arc && lv_obj_is_valid(g_arc)) {
        lv_arc_set_value(g_arc, (int16_t)status.progress);
    }
    if (g_arc_label && lv_obj_is_valid(g_arc_label)) {
        lv_label_set_text_fmt(g_arc_label, "%d%%", status.progress);
    }

    if (upgrade_hint_label && lv_obj_is_valid(upgrade_hint_label) &&
        status.step_text[0] != '\0') {
        lv_label_set_text(upgrade_hint_label, status.step_text);
        lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0x1F9D55), 0);
    }

    if (!status.finished) return;

    upgrade_waiting_exit(status.success);

    if (status.success) {
        lv_upgrade_popup_show_result(true, status.result_text);
    } else {
        lv_upgrade_popup_show_result(false, status.result_text);
        if (upgrade_hint_label && lv_obj_is_valid(upgrade_hint_label)) {
            lv_label_set_text(upgrade_hint_label,
                              status.result_text[0] != '\0' ?
                              status.result_text :
                              page_16_text_get(UI_TEXT_PAGE16_HINT_FAILED));
            lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0xC03A2B), 0);
        }
    }
}

static void upgrade_ui_enter_waiting(void) //进入升级等待界面（圆环跑到90%后停住）
{
    g_upgrading = true;
    g_wait_sec = 0;

    if (g_upgrade_btn) lv_obj_add_state(g_upgrade_btn, LV_STATE_DISABLED);
    if (g_esc_btn)     lv_obj_add_state(g_esc_btn, LV_STATE_DISABLED);

    if (upgrade_hint_label && lv_obj_is_valid(upgrade_hint_label)) {
        lv_label_set_text(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_UPGRADING_WAIT));
        lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0x1F9D55), 0);
    }

    if (!g_arc) {
        g_arc = lv_arc_create(upgrade_page);
        lv_obj_set_size(g_arc, 110, 110);
        lv_obj_set_pos(g_arc, 1080, 184);
        lv_arc_set_range(g_arc, 0, 100);
        lv_arc_set_rotation(g_arc, 270);
        lv_arc_set_bg_angles(g_arc, 0, 360);
        lv_obj_remove_style(g_arc, NULL, LV_PART_KNOB);
        lv_arc_set_value(g_arc, 0);
        lv_obj_clear_flag(g_arc, LV_OBJ_FLAG_CLICKABLE);
    } else {
        lv_arc_set_value(g_arc, 0);
    }

    if (!g_arc_label) {
        g_arc_label = lv_label_create(g_arc);
        lv_obj_center(g_arc_label);
    }
    lv_label_set_text(g_arc_label, "0%");

    if (!g_poweroff_label) {
        g_poweroff_label = lv_label_create(upgrade_page);
        lv_label_set_text(g_poweroff_label, page_16_text_get(UI_TEXT_PAGE16_POWER_OFF));
        lv_obj_set_style_text_color(g_poweroff_label, lv_color_hex(0xC07A2B), 0);
        lv_obj_set_pos(g_poweroff_label, 360, 305);
    } else {
        lv_label_set_text(g_poweroff_label, page_16_text_get(UI_TEXT_PAGE16_POWER_OFF));
    }

    if (!g_arc_anim_inited) {
        lv_anim_init(&g_arc_anim);
        g_arc_anim_inited = true;
    } else {
        lv_anim_del(g_arc, arc_set_value_cb);
    }

    lv_anim_set_var(&g_arc_anim, g_arc);
    lv_anim_set_exec_cb(&g_arc_anim, arc_set_value_cb);
    lv_anim_set_values(&g_arc_anim, 0, 90);
    lv_anim_set_time(&g_arc_anim, 800);
    lv_anim_set_path_cb(&g_arc_anim, lv_anim_path_ease_out);
    lv_anim_start(&g_arc_anim);

    if (!g_wait_timer) {
        g_wait_timer = lv_timer_create(wait_timer_cb, 1000, NULL);
    } else {
        lv_timer_reset(g_wait_timer);
    }

    lv_timer_handler();
}

static void upgrade_esc_btn_cb(lv_event_t* e) //ESC返回上一页（升级中禁用）
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (g_upgrading) return;
    ui_manager_pop_page();
}

static void upgrade_start_btn_cb(lv_event_t* e) //升级按钮：检查U盘->启动脚本->进入等待动画
{
    ui_upgrade_detect_info_t detect_info;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (g_upgrading) return;

    ui_upgrade_service_detect(&detect_info);

    if (!detect_info.usb_present) {
        if (upgrade_hint_label) {
            lv_label_set_text(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_NO_USB));
            lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0xC03A2B), 0);
        }
        return;
    }

    if (!detect_info.usb_mounted) {
        if (upgrade_hint_label) {
            lv_label_set_text(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_AUTO_MOUNT_FAILED));
            lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0xC03A2B), 0);
        }
        return;
    }

    if (!detect_info.package_found) {
        if (upgrade_hint_label) {
            lv_label_set_text(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_MISSING_PACKAGE));
            lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0xC03A2B), 0);
        }
        return;
    }

    upgrade_ui_enter_waiting();

    ui_upgrade_service_reset();
    if (ui_upgrade_service_start() != 0) {
        upgrade_waiting_exit(false);
        if (upgrade_hint_label) {
            lv_label_set_text(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_START_SCRIPT_FAILED));
            lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0xC03A2B), 0);
        }
        return;
    }

    if (!g_upgrade_result_timer) {
        g_upgrade_result_timer = lv_timer_create(upgrade_result_timer_cb, 200, NULL);
    } else {
        lv_timer_reset(g_upgrade_result_timer);
    }
}

void ui_page_16_ui_upgrade_create(lv_obj_t* parent) //创建升级页面
{
    if (upgrade_page) return;

    lv_obj_t* content = NULL;
    upgrade_page = settings_detail_create_page_ex(parent, page_16_text_get(UI_TEXT_PAGE16_TITLE),
                                                  upgrade_esc_btn_cb, &content, &g_esc_btn);

    lv_obj_t* card = settings_detail_create_card(content, 40, 45, 1200, 260);

    upgrade_usb_status_label = lv_label_create(card);
    lv_label_set_text_fmt(upgrade_usb_status_label, page_16_text_get(UI_TEXT_PAGE16_USB_STATUS_FMT),
                          page_16_text_get(UI_TEXT_PAGE16_USB_NOT_INSERTED));
    lv_obj_set_style_text_font(upgrade_usb_status_label, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(upgrade_usb_status_label, 36, 28);

    upgrade_file_status_label = lv_label_create(card);
    lv_label_set_text_fmt(upgrade_file_status_label, page_16_text_get(UI_TEXT_PAGE16_FILE_STATUS_FMT),
                          page_16_text_get(UI_TEXT_PAGE16_MOUNT_NOT),
                          page_16_text_get(UI_TEXT_PAGE16_PACKAGE_NOT_FOUND));
    lv_obj_set_style_text_font(upgrade_file_status_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(upgrade_file_status_label, 36, 78);

    upgrade_hint_label = lv_label_create(card);
    lv_label_set_text(upgrade_hint_label, page_16_text_get(UI_TEXT_PAGE16_HINT_INSERT_USB));
    lv_obj_set_style_text_font(upgrade_hint_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(upgrade_hint_label, lv_color_hex(0x5F6E7D), 0);
    lv_obj_set_pos(upgrade_hint_label, 36, 126);

    g_upgrade_btn = settings_detail_create_button(card, 36, 170, 220, 70,
                                                  page_16_text_get(UI_TEXT_PAGE16_UPGRADE_BTN),
                                                  lv_color_hex(0x08C5D6),
                                                  upgrade_start_btn_cb, NULL);

    g_upgrading = false;
    g_wait_sec = 0;

    update_upgrade_status();
    upgrade_status_timer = lv_timer_create(upgrade_status_timer_cb, 500, NULL);
}

void ui_page_16_ui_upgrade_destroy(void) //销毁升级页面
{
    if (upgrade_status_timer) {
        lv_timer_del(upgrade_status_timer);
        upgrade_status_timer = NULL;
    }

    if (g_wait_timer) {
        lv_timer_del(g_wait_timer);
        g_wait_timer = NULL;
    }

    if (g_upgrade_result_timer) {
        lv_timer_del(g_upgrade_result_timer);
        g_upgrade_result_timer = NULL;
    }

    if (g_arc && lv_obj_is_valid(g_arc)) {
        lv_anim_del(g_arc, arc_set_value_cb);
        lv_obj_del(g_arc);
    }

    if (g_poweroff_label && lv_obj_is_valid(g_poweroff_label)) {
        lv_obj_del(g_poweroff_label);
    }

    if (upgrade_page && lv_obj_is_valid(upgrade_page)) {
        lv_obj_del(upgrade_page);
    }

    upgrade_page = NULL;
    upgrade_usb_status_label = NULL;
    upgrade_file_status_label = NULL;
    upgrade_hint_label = NULL;
    g_upgrade_btn = NULL;
    g_esc_btn = NULL;

    g_arc = NULL;
    g_arc_label = NULL;
    g_poweroff_label = NULL;

    g_wait_sec = 0;
    g_upgrading = false;
}
