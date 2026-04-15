#include "smart_island.h"
#include "un260/lv_components/lv_print_toast.h"
#include "un260/lv_components/lv_qr_popup.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/ui_qr_data.h"
#include "un260/lv_system/ui_text.h"
#include <stdio.h>
#include <string.h>
#include "un260/lv_components/lv_capsule_pagination.h"

/* =========================
 * Smart Island - 旗舰高级版 (时间已完美修复)
 * ========================= */

/* 收起态：主界面B区位置 */
#define SMART_ISLAND_X                    492
#define SMART_ISLAND_Y                    348
#define SMART_ISLAND_W                    261
#define SMART_ISLAND_COMPACT_H            44
#define SMART_ISLAND_RADIUS               22

/* 信息页展开态：向上扩展 */
#define SMART_ISLAND_INFO_EXPAND_W        261
#define SMART_ISLAND_INFO_EXPAND_H        112

/* 功能页展开态：与B阶段一致 */
#define SMART_ISLAND_ACTION_EXPAND_W      261
#define SMART_ISLAND_ACTION_EXPAND_H      112
#define SMART_ISLAND_ACTION_EXPAND_X      0

/* 功能页按钮 (旗舰拟物排版) */
#define SMART_ISLAND_ACTION_PAGE_COUNT    4
#define SMART_ISLAND_ACTION_BTN_W         221 // 两侧留白 20px
#define SMART_ISLAND_ACTION_BTN_H         54
#define SMART_ISLAND_ACTION_BTN_X         20
#define SMART_ISLAND_ACTION_BTN_Y         29
#define SMART_ISLAND_PAGE_INDICATOR_Y    -11
/* 旗舰级色彩主题 (极度克制深邃) */
#define SMART_ISLAND_BG_IDLE              0x000000 // 纯黑底色
#define SMART_ISLAND_BG_COUNTING          0xC9FF18
#define SMART_ISLAND_BG_WARNING           0xFF9F0A
#define SMART_ISLAND_BG_ERROR             0xFF5A5F
#define SMART_ISLAND_BG_UPDATE            0x1E1E1E
#define SMART_ISLAND_TEXT_LIGHT           0xFFFFFF
#define SMART_ISLAND_TEXT_DARK            0x101300
#define SMART_ISLAND_TEXT_SUB             0x777777 // 高级灰标签色
#define SMART_ISLAND_READY_DOT            0x00E676 // 极客绿点
#define SMART_ISLAND_BTN_BG_TOP           0x1C1C1E // 按钮渐变浅色
#define SMART_ISLAND_BTN_BG_BOT           0x111111 // 按钮渐变深色
#define SMART_ISLAND_BTN_BORDER           0x333333 // 按钮切割边框
#define SMART_ISLAND_BTN_TEXT             0xFFFFFF

/* 动画参数 */
#define SMART_ISLAND_EXPAND_TIME          300 
#define SMART_ISLAND_COLLAPSE_TIME        250
#define SMART_ISLAND_TEXT_FADE_TIME       200
#define SMART_ISLAND_SPRING_TIME          320
#define SMART_ISLAND_PULSE_TIME           1200
#define SMART_ISLAND_RESULT_HOLD_MS       1000
#define SMART_ISLAND_WARNING_FLASH_MS     500

#define SMART_ISLAND_MINI_W               180
#define SMART_ISLAND_PAGE_SLIDE_OFFSET    SMART_ISLAND_W

/* 对象 */
static lv_obj_t *g_smart_island = NULL;
static lv_obj_t *g_smart_island_modal = NULL;
static lv_obj_t *g_smart_island_dot = NULL;
static lv_obj_t *g_smart_island_title = NULL;
static lv_obj_t *g_smart_island_subtitle = NULL;
static lv_obj_t *g_smart_island_time = NULL;
static lv_obj_t *g_smart_island_badge = NULL;
static lv_obj_t *g_smart_island_progress = NULL;

/* 页面容器 */
static lv_obj_t *g_smart_island_page_root = NULL;
static lv_obj_t *g_smart_island_page_info = NULL;
static lv_obj_t *g_smart_island_page_action = NULL;
static lv_obj_t *g_smart_island_action_track = NULL;
static lv_obj_t *g_smart_island_page_indicator = NULL;

/* 信息页 */
static lv_obj_t *g_smart_island_expand_title = NULL;
static lv_obj_t *g_smart_island_expand_subtitle = NULL;

/* 功能页按钮 */
static lv_obj_t *g_smart_island_action_btns[SMART_ISLAND_ACTION_PAGE_COUNT];
static lv_obj_t *g_smart_island_action_labels[SMART_ISLAND_ACTION_PAGE_COUNT];
static lv_obj_t *g_smart_island_action_arrows[SMART_ISLAND_ACTION_PAGE_COUNT];
static uint8_t g_smart_island_action_ids[SMART_ISLAND_ACTION_PAGE_COUNT];
static ui_text_id_t g_smart_island_action_text_ids[SMART_ISLAND_ACTION_PAGE_COUNT];
static char g_smart_island_action_texts[SMART_ISLAND_ACTION_PAGE_COUNT][32];

/* 状态 */
static smart_island_scene_t g_smart_island_scene = SMART_ISLAND_SCENE_IDLE;
static smart_island_visual_t g_smart_island_visual = SMART_ISLAND_VISUAL_COMPACT;
static smart_island_page_t g_smart_island_page = SMART_ISLAND_PAGE_INFO;
static smart_island_content_t g_smart_island_content;
static smart_island_action_cb_t g_smart_island_action_cb = NULL;

static lv_timer_t *g_smart_island_result_timer = NULL;
static lv_timer_t *g_smart_island_warning_timer = NULL;
static bool g_smart_island_warning_toggle = false;
static bool g_smart_island_created = false;
static bool g_smart_island_anim_running = false;
static bool g_smart_island_ignore_click_once = false;
static bool g_smart_island_ignore_action_click_once = false;
static uint8_t g_smart_island_action_page_count = SMART_ISLAND_ACTION_PAGE_COUNT;
static uint8_t g_smart_island_action_page_index = 0;
static int8_t g_smart_island_page_slide_dir = 0; // 0:默认 1:向左切页 -1:向右切页
static struct {
    bool pressed;
    bool swiped;
    lv_point_t start_pt;
} g_smart_island_swipe = { false, false, {0, 0} };

/* 文本缓存 */
static char g_smart_island_warning_text[64];
static char g_smart_island_result_text[64];

/* 内部函数声明 */
static void smart_island_enable_gesture_on_obj(lv_obj_t *obj); 
static void smart_island_swipe_cb(lv_event_t *e); 
static void smart_island_apply_scene_style(void); 
static void smart_island_apply_texts(const char *title, const char *subtitle); 
static void smart_island_update_idle_time(void); 
static void smart_island_stop_result_timer(void); 
static void smart_island_stop_warning_timer(void); 
static void smart_island_update_pages_visible(void); 
static void smart_island_pulse_start(void); 
static void smart_island_pulse_stop(void); 
static void smart_island_text_switch_anim(lv_obj_t *obj, const char *text); 
static void smart_island_visual_apply_now(smart_island_visual_t visual); 
static void smart_island_visual_apply_anim(smart_island_visual_t visual); 
static void smart_island_page_apply_now(smart_island_page_t page); 
static void smart_island_page_apply_anim(smart_island_page_t page); 
 static void smart_island_modal_update(void); 
static void smart_island_expand_if_needed(bool anim_en);
static void smart_island_action_btn_create(void); 
static void smart_island_action_page_set_index(uint8_t index, bool anim_en); 
static void smart_island_action_page_slide_anim(uint8_t old_index, uint8_t new_index); 
static void smart_island_action_item_apply(uint8_t index); 
static void smart_island_page_slide_anim(smart_island_page_t old_page, smart_island_page_t new_page); 
static void smart_island_page_slide_anim_finish_cb(lv_anim_t *a); 
static void smart_island_reset_page_positions(void); 
static void smart_island_reset_compact_header_position(void); 
static void smart_island_reset_time_position(void); 
static void smart_island_raise_compact_header(void); 
static void smart_island_show_qr_popup(void); 
static void smart_island_show_qr_error_toast(const char *text);
static void smart_island_action_page_refresh_language_texts(void); 
static void smart_island_open_info_page_by_left_swipe(void);
static void smart_island_open_action_last_page_by_right_swipe(void);
static const char *smart_island_text_or_default(const char *text, ui_text_id_t text_id); 
static uint8_t smart_island_page_indicator_count_get(void);
static uint8_t smart_island_page_indicator_active_get(void);
static void smart_island_page_indicator_sync(bool anim_en);

static void smart_island_anim_w_cb(void *var, int32_t v) { lv_obj_set_width((lv_obj_t *)var, (lv_coord_t)v); }
static void smart_island_anim_h_cb(void *var, int32_t v) { lv_obj_set_height((lv_obj_t *)var, (lv_coord_t)v); }
static void smart_island_anim_x_cb(void *var, int32_t v) { lv_obj_set_x((lv_obj_t *)var, (lv_coord_t)v); }
static void smart_island_anim_y_cb(void *var, int32_t v) { lv_obj_set_y((lv_obj_t *)var, (lv_coord_t)v); }
static void smart_island_anim_text_opa_cb(void *var, int32_t v) { lv_obj_set_style_text_opa((lv_obj_t *)var, (lv_opa_t)v, 0); }
static void smart_island_anim_zoom_cb(void *var, int32_t v) { lv_obj_set_style_transform_zoom((lv_obj_t *)var, (lv_coord_t)v, 0); }
static void smart_island_anim_translate_x_cb(void *var, int32_t v)
{
    lv_obj_set_style_translate_x((lv_obj_t *)var, (lv_coord_t)v, 0);
}
static void smart_island_anim_finish_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    g_smart_island_anim_running = false;
}
static void smart_island_page_slide_anim_finish_cb(lv_anim_t *a) 
{
    LV_UNUSED(a);

    smart_island_update_pages_visible();
    smart_island_reset_page_positions();

    if (g_smart_island_time && lv_obj_is_valid(g_smart_island_time)) {
        lv_obj_set_style_translate_x(g_smart_island_time, 0, 0);
    }

    if (g_smart_island_page == SMART_ISLAND_PAGE_INFO) {
        smart_island_reset_compact_header_position();
        smart_island_reset_time_position();

        if (g_smart_island_time && lv_obj_is_valid(g_smart_island_time)) {
            smart_island_update_idle_time();
            lv_obj_clear_flag(g_smart_island_time, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        if (g_smart_island_time && lv_obj_is_valid(g_smart_island_time)) {
            lv_obj_add_flag(g_smart_island_time, LV_OBJ_FLAG_HIDDEN);
            smart_island_reset_time_position();
        }
    }

    g_smart_island_ignore_click_once = false;
    g_smart_island_ignore_action_click_once = false;
    g_smart_island_page_slide_dir = 0;
    g_smart_island_anim_running = false;
}
static void smart_island_reset_page_positions(void) 
{
    if (g_smart_island_page_root && lv_obj_is_valid(g_smart_island_page_root)) lv_obj_set_x(g_smart_island_page_root, 0);
    if (g_smart_island_page_info && lv_obj_is_valid(g_smart_island_page_info)) lv_obj_set_x(g_smart_island_page_info, 0);
    if (g_smart_island_page_action && lv_obj_is_valid(g_smart_island_page_action)) lv_obj_set_x(g_smart_island_page_action, 0);
    if (g_smart_island_action_track && lv_obj_is_valid(g_smart_island_action_track)) {
        lv_obj_set_x(g_smart_island_action_track, -(lv_coord_t)g_smart_island_action_page_index * SMART_ISLAND_W);
    }
}

static void smart_island_reset_compact_header_position(void) 
{
    if (g_smart_island_dot && lv_obj_is_valid(g_smart_island_dot)) {
        lv_obj_set_x(g_smart_island_dot, 16);
        lv_obj_set_y(g_smart_island_dot, 17);
    }
    if (g_smart_island_title && lv_obj_is_valid(g_smart_island_title)) {
        lv_obj_set_x(g_smart_island_title, 32);
        lv_obj_set_y(g_smart_island_title, 12);
    }
}

/* 完美还原您原版的时间对齐逻辑 */
static void smart_island_reset_time_position(void) 
{
    if (g_smart_island_time == NULL || !lv_obj_is_valid(g_smart_island_time)) {
        return;
    }

    if (g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED) {
        lv_obj_align(g_smart_island_time, LV_ALIGN_TOP_RIGHT, -14, 12);
    } else {
        lv_obj_align(g_smart_island_time, LV_ALIGN_RIGHT_MID, -14, 0);
    }
}

static void smart_island_raise_compact_header(void) 
{
    if (g_smart_island_dot && lv_obj_is_valid(g_smart_island_dot)) lv_obj_move_foreground(g_smart_island_dot);
    if (g_smart_island_title && lv_obj_is_valid(g_smart_island_title)) lv_obj_move_foreground(g_smart_island_title);
    if (g_smart_island_time && lv_obj_is_valid(g_smart_island_time)) lv_obj_move_foreground(g_smart_island_time);
}

static uint8_t smart_island_page_indicator_count_get(void)
{
    return (uint8_t)(g_smart_island_action_page_count + 1U);
}

static uint8_t smart_island_page_indicator_active_get(void)
{
    if (g_smart_island_page == SMART_ISLAND_PAGE_ACTION) {
        return (uint8_t)(g_smart_island_action_page_index + 1U);
    }

    return 0U;
}

static void smart_island_page_indicator_sync(bool anim_en)
{
    uint8_t count;
    uint8_t active;

    if (g_smart_island_page_indicator == NULL || !lv_obj_is_valid(g_smart_island_page_indicator)) {
        return;
    }

    count = smart_island_page_indicator_count_get();
    active = smart_island_page_indicator_active_get();

    lv_capsule_pagination_set_count(g_smart_island_page_indicator, count);
    if (anim_en) {
        lv_capsule_pagination_set_active_page(g_smart_island_page_indicator, active);
    } else {
        lv_capsule_pagination_set_active_page_now(g_smart_island_page_indicator, active);
    }
}

static void smart_island_open_info_page_by_left_swipe(void)
{
    g_smart_island_page_slide_dir = 1;
    smart_island_expand_if_needed(true);
    smart_island_set_page(SMART_ISLAND_PAGE_INFO, true);
}

static void smart_island_open_action_last_page_by_right_swipe(void)
{
    if (g_smart_island_action_page_count == 0U) {
        return;
    }

    smart_island_action_page_set_index((uint8_t)(g_smart_island_action_page_count - 1U), false);
    g_smart_island_page_slide_dir = -1;
    smart_island_expand_if_needed(true);
    smart_island_set_page(SMART_ISLAND_PAGE_ACTION, true);
}

 

static void smart_island_expand_if_needed(bool anim_en) 
{
    if (g_smart_island_visual != SMART_ISLAND_VISUAL_EXPANDED) {
        smart_island_set_visual(SMART_ISLAND_VISUAL_EXPANDED, anim_en);
    }
}
static void smart_island_swipe_cb(lv_event_t *e) 
{
    lv_event_code_t code;
    lv_indev_t *indev;
    lv_point_t pt;

    if (g_smart_island_visual != SMART_ISLAND_VISUAL_EXPANDED) return;

    indev = lv_event_get_indev(e);
    if (indev == NULL) {
        indev = lv_indev_get_act();
        if (indev == NULL) return;
    }

    lv_indev_get_point(indev, &pt);
    code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        g_smart_island_swipe.pressed = true;
        g_smart_island_swipe.swiped = false;
        g_smart_island_swipe.start_pt = pt;
        return;
    }

    if (code == LV_EVENT_PRESSING) {
        lv_coord_t dx;
        lv_coord_t dy;

        if (!g_smart_island_swipe.pressed || g_smart_island_swipe.swiped) return;

        dx = pt.x - g_smart_island_swipe.start_pt.x;
        dy = pt.y - g_smart_island_swipe.start_pt.y;

        if (LV_ABS(dx) > 10 && LV_ABS(dx) > LV_ABS(dy)) {
            g_smart_island_ignore_click_once = true;
            g_smart_island_ignore_action_click_once = true;
            g_smart_island_swipe.swiped = true;

            if (dx < 0) {
                if (g_smart_island_page == SMART_ISLAND_PAGE_INFO) {
                    smart_island_open_action_page();
                } else if (g_smart_island_page == SMART_ISLAND_PAGE_ACTION) {
                    if (g_smart_island_action_page_index + 1U < g_smart_island_action_page_count) {
                        smart_island_action_page_set_index(g_smart_island_action_page_index + 1U, true);
                    } else {
                        smart_island_open_info_page_by_left_swipe();
                    }
                }
            } else {
                if (g_smart_island_page == SMART_ISLAND_PAGE_ACTION) {
                    if (g_smart_island_action_page_index > 0U) {
                        smart_island_action_page_set_index(g_smart_island_action_page_index - 1U, true);
                    } else {
                        smart_island_open_info_page();
                    }
                } else if (g_smart_island_page == SMART_ISLAND_PAGE_INFO) {
                    smart_island_open_action_last_page_by_right_swipe();
                }
            }
        }
        return;
    }

    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        g_smart_island_swipe.pressed = false;
        g_smart_island_swipe.swiped = false;
    }
}   
static void smart_island_enable_gesture_on_obj(lv_obj_t *obj) 
{
    if (obj == NULL || !lv_obj_is_valid(obj)) return;
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE); 
    lv_obj_add_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE); 
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE); 
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_add_event_cb(obj, smart_island_swipe_cb, LV_EVENT_PRESSED, NULL); 
    lv_obj_add_event_cb(obj, smart_island_swipe_cb, LV_EVENT_PRESSING, NULL); 
    lv_obj_add_event_cb(obj, smart_island_swipe_cb, LV_EVENT_RELEASED, NULL); 
}

static void smart_island_result_timer_cb(lv_timer_t *timer) 
{
    LV_UNUSED(timer);
    smart_island_stop_result_timer();
    smart_island_restore_idle();
}

static void smart_island_warning_timer_cb(lv_timer_t *timer) 
{
    LV_UNUSED(timer);

    if (g_smart_island_scene != SMART_ISLAND_SCENE_WARNING) {
        smart_island_stop_warning_timer();
        return;
    }

    g_smart_island_warning_toggle = !g_smart_island_warning_toggle;

    if (g_smart_island_warning_toggle) {
        smart_island_text_switch_anim(g_smart_island_title, g_smart_island_warning_text);
        smart_island_text_switch_anim(g_smart_island_subtitle, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_WARNING_SUBTITLE));
        smart_island_text_switch_anim(g_smart_island_expand_title, g_smart_island_warning_text);
        smart_island_text_switch_anim(g_smart_island_expand_subtitle, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_WARNING_SUBTITLE));
    } else {
        smart_island_text_switch_anim(g_smart_island_title, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RUNNING_TITLE));
        smart_island_text_switch_anim(g_smart_island_subtitle, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RUNNING_SUBTITLE));
        smart_island_text_switch_anim(g_smart_island_expand_title, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RUNNING_TITLE));
        smart_island_text_switch_anim(g_smart_island_expand_subtitle, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RUNNING_SUBTITLE));
    }
}

static void smart_island_modal_click_cb(lv_event_t *e) 
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    smart_island_close();
}

static void smart_island_click_cb(lv_event_t *e) 
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (g_smart_island_anim_running) return;
    if (g_smart_island_ignore_click_once) {
        g_smart_island_ignore_click_once = false;
        return;
    }

    if (g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED) {
        smart_island_close();
    } else {
        smart_island_open_info_page();
    }
}

static void smart_island_action_btn_cb(lv_event_t *e) 
{
    uint8_t page_index = 0;
    uint8_t action_id = 0;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (g_smart_island_ignore_action_click_once) {
        g_smart_island_ignore_action_click_once = false;
        return;
    }

    page_index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    if (page_index >= SMART_ISLAND_ACTION_PAGE_COUNT) return;

    action_id = g_smart_island_action_ids[page_index];

    if (action_id == SMART_ISLAND_ACTION_QR) {
        smart_island_show_qr_popup();
        return;
    }

    if (g_smart_island_action_cb) {
        g_smart_island_action_cb(action_id);
    }
}

static const char *smart_island_text_or_default(const char *text, ui_text_id_t text_id) 
{
    if (text && text[0] != '\0') {
        return text;
    }

    return ui_text_get(text_id);
}

static void smart_island_show_qr_error_toast(const char *text)
{
    lv_print_toast_config_t toast_cfg = lv_print_toast_get_default_config();

    toast_cfg.w = 320;
    toast_cfg.h = 101;
    toast_cfg.text = smart_island_text_or_default(text, UI_TEXT_WIDGET_QR_POPUP_NO_DATA);
    toast_cfg.show_loader = true;
    toast_cfg.align_center = true;
    toast_cfg.use_text_area = false;
    toast_cfg.loader_color = lv_color_hex(0xC0392B);
    toast_cfg.auto_hide_ms = 2000;

    lv_print_toast_show_with_config(&toast_cfg);
}

static void smart_island_show_qr_popup(void) 
{
    char qr_text[3072];
    if (!ui_qr_data_is_ready()) {
        smart_island_show_qr_error_toast(ui_text_get(UI_TEXT_WIDGET_QR_POPUP_NO_DATA));
        return;
    }
    if (!ui_qr_data_build(qr_text, sizeof(qr_text))) {
        smart_island_show_qr_error_toast(ui_text_get(UI_TEXT_WIDGET_QR_POPUP_DATA_TOO_LARGE));
        return;
    }
    if (!lv_qr_popup_show(qr_text)) {
        smart_island_show_qr_error_toast(ui_text_get(UI_TEXT_WIDGET_QR_POPUP_DATA_TOO_LARGE));
    }
}

static void smart_island_stop_result_timer(void) 
{
    if (g_smart_island_result_timer) {
        lv_timer_del(g_smart_island_result_timer);
        g_smart_island_result_timer = NULL;
    }
}

static void smart_island_stop_warning_timer(void) 
{
    if (g_smart_island_warning_timer) {
        lv_timer_del(g_smart_island_warning_timer);
        g_smart_island_warning_timer = NULL;
    }
    g_smart_island_warning_toggle = false;
}

static void smart_island_update_idle_time(void) 
{
    char buf[16];
    if (g_smart_island_time == NULL || !lv_obj_is_valid(g_smart_island_time)) return;
    lv_snprintf(buf, sizeof(buf), "%02u:%02u:%02u",
        (unsigned)Machine_para.hour,
        (unsigned)Machine_para.minute,
        (unsigned)Machine_para.second);
    lv_label_set_text(g_smart_island_time, buf);
}

static void smart_island_apply_texts(const char *title, const char *subtitle) 
{
    if (g_smart_island_title && lv_obj_is_valid(g_smart_island_title)) {
        lv_label_set_text(g_smart_island_title, smart_island_text_or_default(title, UI_TEXT_WIDGET_SMART_ISLAND_READY_TITLE));
    }
    if (g_smart_island_subtitle && lv_obj_is_valid(g_smart_island_subtitle)) {
        lv_label_set_text(g_smart_island_subtitle, smart_island_text_or_default(subtitle, UI_TEXT_WIDGET_SMART_ISLAND_READY_SUBTITLE));
    }
    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        lv_label_set_text(g_smart_island_expand_title, smart_island_text_or_default(title, UI_TEXT_WIDGET_SMART_ISLAND_EXPAND_TITLE));
    }
    if (g_smart_island_expand_subtitle && lv_obj_is_valid(g_smart_island_expand_subtitle)) {
        lv_label_set_text(g_smart_island_expand_subtitle, smart_island_text_or_default(subtitle, UI_TEXT_WIDGET_SMART_ISLAND_EXPAND_SUBTITLE));
    }
}

static void smart_island_modal_update(void) 
{
    if (g_smart_island_modal == NULL || !lv_obj_is_valid(g_smart_island_modal)) return;
    if (g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED) {
        lv_obj_clear_flag(g_smart_island_modal, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(g_smart_island_modal);
        lv_obj_move_foreground(g_smart_island);
    } else {
        lv_obj_add_flag(g_smart_island_modal, LV_OBJ_FLAG_HIDDEN);
    }
}

static void smart_island_update_pages_visible(void) 
{
    if (g_smart_island_page_root && lv_obj_is_valid(g_smart_island_page_root)) {
        if (g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED) {
            lv_obj_clear_flag(g_smart_island_page_root, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_smart_island_page_root, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_smart_island_page_info && lv_obj_is_valid(g_smart_island_page_info)) {
        if (g_smart_island_page == SMART_ISLAND_PAGE_INFO) {
            lv_obj_clear_flag(g_smart_island_page_info, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_smart_island_page_info, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_smart_island_page_action && lv_obj_is_valid(g_smart_island_page_action)) {
        if (g_smart_island_page == SMART_ISLAND_PAGE_ACTION) {
            lv_obj_clear_flag(g_smart_island_page_action, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_smart_island_page_action, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
        if (g_smart_island_page_indicator && lv_obj_is_valid(g_smart_island_page_indicator)) {
        if (g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED) {
            lv_obj_clear_flag(g_smart_island_page_indicator, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_smart_island_page_indicator, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void smart_island_apply_scene_style(void) 
{
    lv_color_t bg_color = lv_color_hex(SMART_ISLAND_BG_IDLE);
    lv_color_t title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
    lv_color_t dot_color = lv_color_hex(SMART_ISLAND_READY_DOT);
    bool show_time = true;
    bool show_dot = true;
    
    if (g_smart_island == NULL || !lv_obj_is_valid(g_smart_island)) return;

    switch (g_smart_island_scene) {
    case SMART_ISLAND_SCENE_IDLE:
    case SMART_ISLAND_SCENE_RESULT:
    case SMART_ISLAND_SCENE_QR:
        bg_color = lv_color_hex(SMART_ISLAND_BG_IDLE);
        title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
        dot_color = lv_color_hex(SMART_ISLAND_READY_DOT);
        show_time = (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE);
        show_dot = (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE);
        smart_island_pulse_stop();
        break;

    case SMART_ISLAND_SCENE_COUNTING:
        bg_color = lv_color_hex(SMART_ISLAND_BG_COUNTING);
        title_color = lv_color_hex(SMART_ISLAND_TEXT_DARK);
        dot_color = lv_color_hex(SMART_ISLAND_TEXT_DARK);
        show_time = false;
        show_dot = false;
        smart_island_pulse_start();
        break;

    case SMART_ISLAND_SCENE_WARNING:
        bg_color = lv_color_hex(SMART_ISLAND_BG_WARNING);
        title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
        show_time = false;
        show_dot = false;
        smart_island_pulse_stop();
        break;

    case SMART_ISLAND_SCENE_UPDATE:
        bg_color = lv_color_hex(SMART_ISLAND_BG_UPDATE);
        title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
        show_time = false;
        show_dot = false;
        smart_island_pulse_stop();
        break;
    default: break;
    }

    /* 修复：功能页强制不显示时间，避免在 action 页被后续刷新出来 */
    if (g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED &&
        g_smart_island_page == SMART_ISLAND_PAGE_ACTION) {
        show_time = false;
    }

    lv_obj_set_style_bg_color(g_smart_island, bg_color, 0);

    if (g_smart_island_title && lv_obj_is_valid(g_smart_island_title)) {
        lv_obj_set_style_text_color(g_smart_island_title, title_color, 0);
        lv_obj_clear_flag(g_smart_island_title, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (g_smart_island_subtitle && lv_obj_is_valid(g_smart_island_subtitle)) {
        lv_obj_add_flag(g_smart_island_subtitle, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        lv_obj_clear_flag(g_smart_island_expand_title, LV_OBJ_FLAG_HIDDEN); 
    }
    if (g_smart_island_expand_subtitle && lv_obj_is_valid(g_smart_island_expand_subtitle)) {
        lv_obj_clear_flag(g_smart_island_expand_subtitle, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_smart_island_dot && lv_obj_is_valid(g_smart_island_dot)) {
        lv_obj_set_style_bg_color(g_smart_island_dot, dot_color, 0);
        if (show_dot) lv_obj_clear_flag(g_smart_island_dot, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_smart_island_dot, LV_OBJ_FLAG_HIDDEN);
    }

    if (show_time) {
        smart_island_update_idle_time();
        lv_obj_clear_flag(g_smart_island_time, LV_OBJ_FLAG_HIDDEN);
        smart_island_reset_time_position();
    } else {
        lv_obj_add_flag(g_smart_island_time, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_smart_island_progress && lv_obj_is_valid(g_smart_island_progress)) {
        if (g_smart_island_scene == SMART_ISLAND_SCENE_UPDATE) lv_obj_clear_flag(g_smart_island_progress, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_smart_island_progress, LV_OBJ_FLAG_HIDDEN);
    }

    smart_island_update_pages_visible();
    smart_island_modal_update();
}

static void smart_island_text_switch_anim(lv_obj_t *obj, const char *text) 
{
    lv_anim_t a;
    if (obj == NULL || !lv_obj_is_valid(obj)) return;
    lv_anim_del(obj, smart_island_anim_text_opa_cb);
    lv_obj_set_style_text_opa(obj, LV_OPA_30, 0);
    if (lv_obj_check_type(obj, &lv_label_class)) lv_label_set_text(obj, text ? text : "");
    
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, smart_island_anim_text_opa_cb);
    lv_anim_set_values(&a, LV_OPA_30, LV_OPA_COVER);
    lv_anim_set_time(&a, SMART_ISLAND_TEXT_FADE_TIME);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

static void smart_island_visual_apply_now(smart_island_visual_t visual) 
{
    lv_coord_t x = SMART_ISLAND_X;
    lv_coord_t y = SMART_ISLAND_Y;
    lv_coord_t w = SMART_ISLAND_W;
    lv_coord_t h = SMART_ISLAND_COMPACT_H;

    if (g_smart_island == NULL || !lv_obj_is_valid(g_smart_island)) return;

    if (visual == SMART_ISLAND_VISUAL_MINI) {
        w = SMART_ISLAND_MINI_W;
        x = SMART_ISLAND_X + (SMART_ISLAND_W - w) / 2;
    } else if (visual == SMART_ISLAND_VISUAL_EXPANDED) {
        y = SMART_ISLAND_Y - (SMART_ISLAND_ACTION_EXPAND_H - SMART_ISLAND_COMPACT_H);
        h = SMART_ISLAND_ACTION_EXPAND_H;
    }

    lv_obj_set_pos(g_smart_island, x, y);
    lv_obj_set_size(g_smart_island, w, h);
    
    smart_island_reset_time_position();

    smart_island_update_pages_visible();
    smart_island_modal_update();
}

static void smart_island_visual_apply_anim(smart_island_visual_t visual) 
{
    lv_anim_t a;
    lv_coord_t dst_x = SMART_ISLAND_X, dst_y = SMART_ISLAND_Y;
    lv_coord_t dst_w = SMART_ISLAND_W, dst_h = SMART_ISLAND_COMPACT_H;
    uint32_t anim_time = SMART_ISLAND_EXPAND_TIME;

    if (g_smart_island == NULL || !lv_obj_is_valid(g_smart_island)) return;

    if (visual == SMART_ISLAND_VISUAL_MINI) {
        dst_w = SMART_ISLAND_MINI_W;
        dst_x = SMART_ISLAND_X + (SMART_ISLAND_W - dst_w) / 2;
        anim_time = SMART_ISLAND_COLLAPSE_TIME;
    } else if (visual == SMART_ISLAND_VISUAL_EXPANDED) {
        dst_y = SMART_ISLAND_Y - (SMART_ISLAND_ACTION_EXPAND_H - SMART_ISLAND_COMPACT_H);
        dst_h = SMART_ISLAND_ACTION_EXPAND_H;
    }

    g_smart_island_anim_running = true;

    lv_anim_del(g_smart_island, smart_island_anim_x_cb);
    lv_anim_del(g_smart_island, smart_island_anim_y_cb);
    lv_anim_del(g_smart_island, smart_island_anim_w_cb);
    lv_anim_del(g_smart_island, smart_island_anim_h_cb);

    lv_anim_init(&a);
    lv_anim_set_var(&a, g_smart_island);
    lv_anim_set_exec_cb(&a, smart_island_anim_y_cb);
    lv_anim_set_values(&a, lv_obj_get_y(g_smart_island), dst_y);
    lv_anim_set_time(&a, anim_time);
    lv_anim_set_path_cb(&a, (visual == SMART_ISLAND_VISUAL_EXPANDED) ? lv_anim_path_overshoot : lv_anim_path_ease_out);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, smart_island_anim_h_cb);
    lv_anim_set_values(&a, lv_obj_get_height(g_smart_island), dst_h);
    lv_anim_set_ready_cb(&a, smart_island_anim_finish_cb);
    lv_anim_start(&a);

    smart_island_reset_time_position();

    smart_island_update_pages_visible();
    smart_island_modal_update();
}

static void smart_island_pulse_start(void) 
{
    lv_anim_t a;
    if (g_smart_island == NULL || !lv_obj_is_valid(g_smart_island)) return;
    lv_anim_del(g_smart_island, smart_island_anim_zoom_cb);
    lv_obj_set_style_transform_zoom(g_smart_island, 256, 0);
    lv_anim_init(&a);
    lv_anim_set_var(&a, g_smart_island);
    lv_anim_set_exec_cb(&a, smart_island_anim_zoom_cb);
    lv_anim_set_values(&a, 252, 256);
    lv_anim_set_time(&a, SMART_ISLAND_PULSE_TIME);
    lv_anim_set_playback_time(&a, SMART_ISLAND_PULSE_TIME);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}

static void smart_island_pulse_stop(void) 
{
    if (g_smart_island == NULL || !lv_obj_is_valid(g_smart_island)) return;
    lv_anim_del(g_smart_island, smart_island_anim_zoom_cb);
    lv_obj_set_style_transform_zoom(g_smart_island, 256, 0);
}
static void smart_island_page_slide_anim(smart_island_page_t old_page, smart_island_page_t new_page) 
{
    lv_anim_t a;
    lv_obj_t *old_obj = NULL, *new_obj = NULL;
    lv_coord_t start_old_x = 0, end_old_x = 0;
    lv_coord_t start_new_x = 0, end_new_x = 0;
    lv_coord_t delta_x = 0;
    bool slide_left = false;

    if (g_smart_island_page_info == NULL || g_smart_island_page_action == NULL) return;

    old_obj = (old_page == SMART_ISLAND_PAGE_INFO) ? g_smart_island_page_info : g_smart_island_page_action;
    new_obj = (new_page == SMART_ISLAND_PAGE_INFO) ? g_smart_island_page_info : g_smart_island_page_action;

    if (old_obj == new_obj) return;

    g_smart_island_anim_running = true;

    smart_island_raise_compact_header();

    lv_obj_clear_flag(old_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(new_obj, LV_OBJ_FLAG_HIDDEN);

    if (g_smart_island_page_slide_dir > 0) {
        slide_left = true;
    } else if (g_smart_island_page_slide_dir < 0) {
        slide_left = false;
    } else {
        slide_left = (new_page == SMART_ISLAND_PAGE_ACTION);
    }

    if (slide_left) {
        start_old_x = 0;
        end_old_x = -SMART_ISLAND_PAGE_SLIDE_OFFSET;
        start_new_x = SMART_ISLAND_PAGE_SLIDE_OFFSET;
        end_new_x = 0;
    } else {
        start_old_x = 0;
        end_old_x = SMART_ISLAND_PAGE_SLIDE_OFFSET;
        start_new_x = -SMART_ISLAND_PAGE_SLIDE_OFFSET;
        end_new_x = 0;
    }

    delta_x = end_old_x - start_old_x;

    lv_obj_set_x(old_obj, start_old_x);
    lv_obj_set_x(new_obj, start_new_x);

    lv_anim_init(&a);
    lv_anim_set_var(&a, old_obj);
    lv_anim_set_exec_cb(&a, smart_island_anim_x_cb);
    lv_anim_set_values(&a, start_old_x, end_old_x);
    lv_anim_set_time(&a, 180);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);

    lv_anim_init(&a);
    lv_anim_set_var(&a, new_obj);
    lv_anim_set_exec_cb(&a, smart_island_anim_x_cb);
    lv_anim_set_values(&a, start_new_x, end_new_x);
    lv_anim_set_time(&a, 180);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&a, smart_island_page_slide_anim_finish_cb);
    lv_anim_start(&a);

    if (g_smart_island_title && lv_obj_is_valid(g_smart_island_title)) {
        lv_anim_del(g_smart_island_title, smart_island_anim_x_cb);
        lv_anim_init(&a);
        lv_anim_set_var(&a, g_smart_island_title);
        lv_anim_set_exec_cb(&a, smart_island_anim_x_cb);
        lv_anim_set_values(&a, lv_obj_get_x(g_smart_island_title), lv_obj_get_x(g_smart_island_title) + delta_x);
        lv_anim_set_time(&a, 180);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
        lv_anim_start(&a);
    }

    if (g_smart_island_dot && lv_obj_is_valid(g_smart_island_dot)) {
        lv_anim_del(g_smart_island_dot, smart_island_anim_x_cb);
        lv_anim_init(&a);
        lv_anim_set_var(&a, g_smart_island_dot);
        lv_anim_set_exec_cb(&a, smart_island_anim_x_cb);
        lv_anim_set_values(&a, lv_obj_get_x(g_smart_island_dot), lv_obj_get_x(g_smart_island_dot) + delta_x);
        lv_anim_set_time(&a, 180);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
        lv_anim_start(&a);
    }

    if (g_smart_island_time && lv_obj_is_valid(g_smart_island_time)) {
        lv_anim_del(g_smart_island_time, smart_island_anim_translate_x_cb);

        if (new_page == SMART_ISLAND_PAGE_ACTION) {
            lv_obj_clear_flag(g_smart_island_time, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_translate_x(g_smart_island_time, 0, 0);

            lv_anim_init(&a);
            lv_anim_set_var(&a, g_smart_island_time);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, 0, delta_x);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        } else {
            smart_island_reset_time_position();
            smart_island_update_idle_time();
            lv_obj_clear_flag(g_smart_island_time, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_translate_x(g_smart_island_time, -delta_x, 0);

            lv_anim_init(&a);
            lv_anim_set_var(&a, g_smart_island_time);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, -delta_x, 0);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        }
    }
}
static void smart_island_page_apply_now(smart_island_page_t page) 
{
    g_smart_island_page = page;
    smart_island_update_pages_visible();
    smart_island_reset_page_positions();
}

static void smart_island_page_apply_anim(smart_island_page_t page) 
{
    smart_island_page_t old_page = g_smart_island_page;
    g_smart_island_page = page;
    smart_island_page_slide_anim(old_page, page);
}
static void smart_island_action_item_apply(uint8_t index) 
{
    lv_obj_t *btn = NULL, *label = NULL;
    lv_coord_t page_x = 0;

    if (index >= SMART_ISLAND_ACTION_PAGE_COUNT) return;

    btn = g_smart_island_action_btns[index];
    label = g_smart_island_action_labels[index];

    if (btn == NULL || !lv_obj_is_valid(btn)) return;

    if (index >= g_smart_island_action_page_count) {
        lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    page_x = (lv_coord_t)index * SMART_ISLAND_W;
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(btn, page_x + SMART_ISLAND_ACTION_BTN_X, SMART_ISLAND_ACTION_BTN_Y);

    if (label && lv_obj_is_valid(label)) {
        if (g_smart_island_action_text_ids[index] < UI_TEXT_MAX) {
            lv_label_set_text(label, ui_text_get(g_smart_island_action_text_ids[index]));
        } else if (g_smart_island_action_texts[index][0] != '\0') {
            lv_label_set_text(label, g_smart_island_action_texts[index]);
        } else {
            lv_label_set_text(label, "");
        }
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 16, 0); 
    }
}
static void smart_island_action_page_refresh_language_texts(void) 
{
    for (uint8_t i = 0; i < g_smart_island_action_page_count; i++) {
        if (g_smart_island_action_text_ids[i] < UI_TEXT_MAX) {
            lv_snprintf(g_smart_island_action_texts[i], sizeof(g_smart_island_action_texts[i]), "%s",
                ui_text_get(g_smart_island_action_text_ids[i]));
        }
        smart_island_action_item_apply(i);
    }
}
static void smart_island_action_page_slide_anim(uint8_t old_index, uint8_t new_index) 
{
    lv_anim_t a;

    if (g_smart_island_action_track == NULL || !lv_obj_is_valid(g_smart_island_action_track)) return;

    if (old_index == new_index) {
        lv_obj_set_x(g_smart_island_action_track, -(lv_coord_t)new_index * SMART_ISLAND_W);
        return;
    }

    g_smart_island_anim_running = true;
    lv_anim_del(g_smart_island_action_track, smart_island_anim_x_cb);

    lv_anim_init(&a);
    lv_anim_set_var(&a, g_smart_island_action_track);
    lv_anim_set_exec_cb(&a, smart_island_anim_x_cb);
    lv_anim_set_values(&a, -(lv_coord_t)old_index * SMART_ISLAND_W, -(lv_coord_t)new_index * SMART_ISLAND_W);
    lv_anim_set_time(&a, 160);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&a, smart_island_anim_finish_cb);
    lv_anim_start(&a);
}
static void smart_island_action_page_set_index(uint8_t index, bool anim_en) 
{
    uint8_t target_index = (index >= g_smart_island_action_page_count && g_smart_island_action_page_count > 0U)
        ? (uint8_t)(g_smart_island_action_page_count - 1U)
        : index;
    uint8_t old_index = g_smart_island_action_page_index;

    if (g_smart_island_action_page_count == 0U) return;

    g_smart_island_action_page_index = target_index;
    smart_island_page_indicator_sync(anim_en);

    if (g_smart_island_action_track == NULL || !lv_obj_is_valid(g_smart_island_action_track)) return;

    if (anim_en) {
        smart_island_action_page_slide_anim(old_index, target_index);
    } else {
        lv_anim_del(g_smart_island_action_track, smart_island_anim_x_cb);
        lv_obj_set_x(g_smart_island_action_track, -(lv_coord_t)target_index * SMART_ISLAND_W);
    }
}
bool smart_island_action_page_set_count(uint8_t count) 
{
    uint8_t new_count = (count == 0U) ? 1U : (count > SMART_ISLAND_ACTION_PAGE_COUNT ? SMART_ISLAND_ACTION_PAGE_COUNT : count);

    g_smart_island_action_page_count = new_count;

    if (g_smart_island_action_page_index >= g_smart_island_action_page_count) {
        g_smart_island_action_page_index = (uint8_t)(g_smart_island_action_page_count - 1U);
    }

    if (g_smart_island_action_track && lv_obj_is_valid(g_smart_island_action_track)) {
        lv_obj_set_size(g_smart_island_action_track,
                        (lv_coord_t)(SMART_ISLAND_W * g_smart_island_action_page_count),
                        SMART_ISLAND_ACTION_EXPAND_H);
    }

    for (uint8_t i = 0; i < SMART_ISLAND_ACTION_PAGE_COUNT; i++) {
        smart_island_action_item_apply(i);
    }

    if (g_smart_island_action_track && lv_obj_is_valid(g_smart_island_action_track)) {
        lv_obj_set_x(g_smart_island_action_track,
                     -(lv_coord_t)g_smart_island_action_page_index * SMART_ISLAND_W);
    }

    smart_island_page_indicator_sync(false);
    return true;
}
bool smart_island_action_page_set_lang_item(uint8_t index, uint8_t action_id, ui_text_id_t text_id) 
{
    if (index >= SMART_ISLAND_ACTION_PAGE_COUNT) return false;
    g_smart_island_action_ids[index] = action_id;
    g_smart_island_action_text_ids[index] = text_id;
    g_smart_island_action_texts[index][0] = '\0';
    if (index + 1U > g_smart_island_action_page_count) smart_island_action_page_set_count(index + 1U);
    else smart_island_action_item_apply(index);
    return true;
}

bool smart_island_action_page_set_item(uint8_t index, uint8_t action_id, const char *text) 
{
    if (index >= SMART_ISLAND_ACTION_PAGE_COUNT) return false;
    g_smart_island_action_ids[index] = action_id;
    g_smart_island_action_text_ids[index] = UI_TEXT_MAX;
    if (text && text[0] != '\0') lv_snprintf(g_smart_island_action_texts[index], sizeof(g_smart_island_action_texts[index]), "%s", text);
    else g_smart_island_action_texts[index][0] = '\0';
    if (index + 1U > g_smart_island_action_page_count) smart_island_action_page_set_count(index + 1U);
    else smart_island_action_item_apply(index);
    return true;
}
static void smart_island_action_btn_create(void) 
{
    static const uint8_t default_ids[SMART_ISLAND_ACTION_PAGE_COUNT] = {
        SMART_ISLAND_ACTION_QR, SMART_ISLAND_ACTION_TIME_SETTING,
        SMART_ISLAND_ACTION_FUNC3, SMART_ISLAND_ACTION_FUNC4
    };
    static const ui_text_id_t default_text_ids[SMART_ISLAND_ACTION_PAGE_COUNT] = {
        UI_TEXT_WIDGET_SMART_ISLAND_ACTION_QR, UI_TEXT_WIDGET_SMART_ISLAND_ACTION_TIME,
        UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC3, UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC4
    };
    lv_obj_t *btn = NULL, *label = NULL, *arrow = NULL;

    g_smart_island_action_track = lv_obj_create(g_smart_island_page_action);
    lv_obj_remove_style_all(g_smart_island_action_track);
    lv_obj_set_size(g_smart_island_action_track,
                    (lv_coord_t)(SMART_ISLAND_W * g_smart_island_action_page_count),
                    SMART_ISLAND_ACTION_EXPAND_H);
    lv_obj_set_style_bg_opa(g_smart_island_action_track, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(g_smart_island_action_track, LV_OBJ_FLAG_SCROLLABLE);
    smart_island_enable_gesture_on_obj(g_smart_island_action_track);

    for (uint8_t i = 0; i < SMART_ISLAND_ACTION_PAGE_COUNT; i++) {
        btn = lv_obj_create(g_smart_island_action_track);
        lv_obj_remove_style_all(btn);
        lv_obj_set_size(btn, SMART_ISLAND_ACTION_BTN_W, SMART_ISLAND_ACTION_BTN_H);
        
        lv_obj_set_style_radius(btn, 14, 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(SMART_ISLAND_BTN_BG_TOP), 0);
        lv_obj_set_style_bg_grad_color(btn, lv_color_hex(SMART_ISLAND_BTN_BG_BOT), 0);
        lv_obj_set_style_bg_grad_dir(btn, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(SMART_ISLAND_BTN_BORDER), 0);
        
        lv_obj_add_event_cb(btn, smart_island_action_btn_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        smart_island_enable_gesture_on_obj(btn);

        label = lv_label_create(btn);
        lv_obj_set_style_text_color(label, lv_color_hex(SMART_ISLAND_BTN_TEXT), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0); 
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 16, 0);

        arrow = lv_label_create(btn);
        lv_label_set_text(arrow, ">");
        lv_obj_set_style_text_color(arrow, lv_color_hex(0x666666), 0);
        lv_obj_set_style_text_font(arrow, &lv_font_montserrat_18, 0);
        lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -16, 0);

        g_smart_island_action_btns[i] = btn;
        g_smart_island_action_labels[i] = label;
        g_smart_island_action_arrows[i] = arrow;
        g_smart_island_action_ids[i] = default_ids[i];
        g_smart_island_action_text_ids[i] = default_text_ids[i];
        g_smart_island_action_texts[i][0] = '\0';
    }

    smart_island_action_page_set_count(SMART_ISLAND_ACTION_PAGE_COUNT);
    smart_island_action_page_refresh_language_texts();
    smart_island_action_page_set_index(0U, false);
}
void smart_island_create(lv_obj_t *parent) 
{
    if (parent == NULL || (g_smart_island_created && g_smart_island && lv_obj_is_valid(g_smart_island))) return;

    memset(&g_smart_island_content, 0, sizeof(g_smart_island_content));
    memset(g_smart_island_warning_text, 0, sizeof(g_smart_island_warning_text));
    memset(g_smart_island_result_text, 0, sizeof(g_smart_island_result_text));

    g_smart_island_modal = lv_obj_create(parent);
    lv_obj_remove_style_all(g_smart_island_modal);
    lv_obj_set_size(g_smart_island_modal, 1280, 400);
    lv_obj_set_style_bg_opa(g_smart_island_modal, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(g_smart_island_modal, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(g_smart_island_modal, smart_island_modal_click_cb, LV_EVENT_CLICKED, NULL);

    g_smart_island = lv_obj_create(parent);
    lv_obj_remove_style_all(g_smart_island);
    lv_obj_set_pos(g_smart_island, SMART_ISLAND_X, SMART_ISLAND_Y);
    lv_obj_set_size(g_smart_island, SMART_ISLAND_W, SMART_ISLAND_COMPACT_H);
    lv_obj_set_style_radius(g_smart_island, SMART_ISLAND_RADIUS, 0);
    lv_obj_set_style_bg_opa(g_smart_island, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_smart_island, lv_color_hex(SMART_ISLAND_BG_IDLE), 0);
    lv_obj_set_style_border_width(g_smart_island, 1, 0);
    lv_obj_set_style_border_color(g_smart_island, lv_color_hex(0x222222), 0);
    smart_island_enable_gesture_on_obj(g_smart_island);
    lv_obj_add_event_cb(g_smart_island, smart_island_click_cb, LV_EVENT_CLICKED, NULL);

    g_smart_island_dot = lv_obj_create(g_smart_island);
    lv_obj_remove_style_all(g_smart_island_dot);
    lv_obj_set_size(g_smart_island_dot, 8, 8);
    lv_obj_set_pos(g_smart_island_dot, 20, 18);
    lv_obj_set_style_radius(g_smart_island_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(g_smart_island_dot, LV_OPA_COVER, 0);

    g_smart_island_title = lv_label_create(g_smart_island);
    lv_label_set_text(g_smart_island_title, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_READY_TITLE));
    lv_obj_set_pos(g_smart_island_title, 36, 13);
    lv_obj_set_style_text_font(g_smart_island_title, &lv_font_montserrat_14, 0);

    g_smart_island_subtitle = lv_label_create(g_smart_island);
    lv_obj_add_flag(g_smart_island_subtitle, LV_OBJ_FLAG_HIDDEN); 

    g_smart_island_time = lv_label_create(g_smart_island);
    lv_label_set_text(g_smart_island_time, "00:00:00");
    lv_obj_set_style_text_font(g_smart_island_time, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(g_smart_island_time, lv_color_hex(SMART_ISLAND_TEXT_LIGHT), 0);
    lv_obj_align(g_smart_island_time, LV_ALIGN_RIGHT_MID, -14, 0);

    g_smart_island_badge = lv_obj_create(g_smart_island);
    lv_obj_remove_style_all(g_smart_island_badge);
    lv_obj_add_flag(g_smart_island_badge, LV_OBJ_FLAG_HIDDEN);

    g_smart_island_progress = lv_bar_create(g_smart_island);
    lv_obj_set_size(g_smart_island_progress, 160, 4);
    lv_obj_align(g_smart_island_progress, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_color(g_smart_island_progress, lv_color_hex(0x2E2E2E), LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_smart_island_progress, lv_color_hex(0x00E676), LV_PART_INDICATOR); 
    lv_obj_add_flag(g_smart_island_progress, LV_OBJ_FLAG_HIDDEN);

    g_smart_island_page_root = lv_obj_create(g_smart_island);
    lv_obj_remove_style_all(g_smart_island_page_root);
    lv_obj_set_size(g_smart_island_page_root, SMART_ISLAND_W, SMART_ISLAND_ACTION_EXPAND_H);
    smart_island_enable_gesture_on_obj(g_smart_island_page_root);
    lv_obj_add_flag(g_smart_island_page_root, LV_OBJ_FLAG_HIDDEN);

    g_smart_island_page_info = lv_obj_create(g_smart_island_page_root);
    lv_obj_remove_style_all(g_smart_island_page_info);
    lv_obj_set_size(g_smart_island_page_info, SMART_ISLAND_W, SMART_ISLAND_ACTION_EXPAND_H);
    smart_island_enable_gesture_on_obj(g_smart_island_page_info);

    g_smart_island_expand_title = lv_label_create(g_smart_island_page_info);
    lv_label_set_text(g_smart_island_expand_title, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_EXPAND_TITLE));
    lv_obj_set_pos(g_smart_island_expand_title, 24, 30);
    lv_obj_set_style_text_font(g_smart_island_expand_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(g_smart_island_expand_title, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);

    g_smart_island_expand_subtitle = lv_label_create(g_smart_island_page_info);
    lv_label_set_text(g_smart_island_expand_subtitle, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_EXPAND_SUBTITLE));
    lv_obj_set_pos(g_smart_island_expand_subtitle, 24, 52);
    lv_obj_set_style_text_font(g_smart_island_expand_subtitle, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(g_smart_island_expand_subtitle, lv_color_hex(SMART_ISLAND_TEXT_LIGHT), 0);

    g_smart_island_page_action = lv_obj_create(g_smart_island_page_root);
    lv_obj_remove_style_all(g_smart_island_page_action);
    lv_obj_set_size(g_smart_island_page_action, SMART_ISLAND_W, SMART_ISLAND_ACTION_EXPAND_H);
    smart_island_enable_gesture_on_obj(g_smart_island_page_action);
    lv_obj_add_flag(g_smart_island_page_action, LV_OBJ_FLAG_HIDDEN);

    g_smart_island_page_indicator = lv_capsule_pagination_create(g_smart_island);
    if (g_smart_island_page_indicator && lv_obj_is_valid(g_smart_island_page_indicator)) {
        lv_obj_align(g_smart_island_page_indicator, LV_ALIGN_BOTTOM_MID, 0, SMART_ISLAND_PAGE_INDICATOR_Y);
        lv_obj_add_flag(g_smart_island_page_indicator, LV_OBJ_FLAG_HIDDEN);
    }
    smart_island_action_btn_create();

    g_smart_island_scene = SMART_ISLAND_SCENE_IDLE;
    g_smart_island_visual = SMART_ISLAND_VISUAL_COMPACT;
    g_smart_island_page = SMART_ISLAND_PAGE_INFO;
    g_smart_island_created = true;
    smart_island_page_indicator_sync(false);
    smart_island_apply_scene_style();
    smart_island_update_idle_time();
    smart_island_update_pages_visible();
    smart_island_modal_update();
}

void smart_island_destroy(void) 
{
    smart_island_stop_result_timer();
    smart_island_stop_warning_timer();
    smart_island_pulse_stop();
    if (g_smart_island && lv_obj_is_valid(g_smart_island)) lv_obj_del(g_smart_island);
    if (g_smart_island_modal && lv_obj_is_valid(g_smart_island_modal)) lv_obj_del(g_smart_island_modal);
    g_smart_island = NULL;
    g_smart_island_modal = NULL;
    g_smart_island_page_root = NULL;
    g_smart_island_page_info = NULL;
    g_smart_island_page_action = NULL;
    g_smart_island_action_track = NULL;
    g_smart_island_page_indicator = NULL;
    g_smart_island_page_slide_dir = 0;

    g_smart_island_created = false;
}
void smart_island_refresh_time(void) 
{
    if (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE &&
        !(g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED &&
          g_smart_island_page == SMART_ISLAND_PAGE_ACTION)) {
        smart_island_update_idle_time();
    }
}
void smart_island_set_visual(smart_island_visual_t visual, bool anim_en) 
{
    if (g_smart_island == NULL || !lv_obj_is_valid(g_smart_island)) return;
    g_smart_island_visual = visual;
    if (anim_en) smart_island_visual_apply_anim(visual);
    else smart_island_visual_apply_now(visual);
    smart_island_update_pages_visible();
    smart_island_modal_update();
    smart_island_apply_scene_style();
}

void smart_island_set_scene(smart_island_scene_t scene, const char *title, const char *subtitle) 
{
    g_smart_island_scene = scene;
    smart_island_stop_result_timer();
    if (scene != SMART_ISLAND_SCENE_WARNING) smart_island_stop_warning_timer();
    smart_island_apply_texts(title, subtitle);
    smart_island_apply_scene_style();
}

void smart_island_notify_count_start(void) 
{
    smart_island_set_scene(SMART_ISLAND_SCENE_COUNTING,
        ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNTING_TITLE),
        ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNTING_SUBTITLE));
    smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, true);
}

void smart_island_notify_count_end(const char *result_text) 
{
    if (result_text && result_text[0] != '\0') lv_snprintf(g_smart_island_result_text, sizeof(g_smart_island_result_text), "%s", result_text);
    else lv_snprintf(g_smart_island_result_text, sizeof(g_smart_island_result_text), "%s", ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_FINISHED));
    smart_island_set_scene(SMART_ISLAND_SCENE_RESULT, g_smart_island_result_text, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RESULT_SUBTITLE));
    smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, true);
    smart_island_stop_result_timer();
    g_smart_island_result_timer = lv_timer_create(smart_island_result_timer_cb, SMART_ISLAND_RESULT_HOLD_MS, NULL);
}

void smart_island_notify_warning(const char *warn_text) 
{
    if (warn_text && warn_text[0] != '\0') lv_snprintf(g_smart_island_warning_text, sizeof(g_smart_island_warning_text), "%s", warn_text);
    else lv_snprintf(g_smart_island_warning_text, sizeof(g_smart_island_warning_text), "%s", ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR));
    smart_island_set_scene(SMART_ISLAND_SCENE_WARNING, g_smart_island_warning_text, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_WARNING_SUBTITLE));
    smart_island_open_info_page();
    smart_island_stop_warning_timer();
    g_smart_island_warning_timer = lv_timer_create(smart_island_warning_timer_cb, SMART_ISLAND_WARNING_FLASH_MS, NULL);
}

void smart_island_notify_update(uint16_t progress, const char *text) 
{
    smart_island_set_scene(SMART_ISLAND_SCENE_UPDATE,
        smart_island_text_or_default(text, UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_TITLE),
        ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_SUBTITLE));
    if (g_smart_island_progress && lv_obj_is_valid(g_smart_island_progress)) {
        lv_obj_clear_flag(g_smart_island_progress, LV_OBJ_FLAG_HIDDEN);
        if (progress > 100) progress = 100;
        lv_bar_set_value(g_smart_island_progress, progress, LV_ANIM_ON);
    }
    smart_island_open_info_page();
}

void smart_island_notify_qr(const char *text) 
{
    smart_island_set_scene(SMART_ISLAND_SCENE_QR,
        smart_island_text_or_default(text, UI_TEXT_WIDGET_SMART_ISLAND_QR_READY),
        ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_QR_SUBTITLE));
    if (g_smart_island_progress && lv_obj_is_valid(g_smart_island_progress)) lv_obj_add_flag(g_smart_island_progress, LV_OBJ_FLAG_HIDDEN);
    smart_island_open_info_page();
}

void smart_island_restore_idle(void) 
{
    if (g_smart_island_progress && lv_obj_is_valid(g_smart_island_progress)) {
        lv_obj_add_flag(g_smart_island_progress, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_value(g_smart_island_progress, 0, LV_ANIM_OFF);
    }
    smart_island_set_scene(SMART_ISLAND_SCENE_IDLE,
        ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_READY_TITLE),
        ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_READY_SUBTITLE));
    smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, true);
    smart_island_update_idle_time();
    smart_island_reset_compact_header_position();
    smart_island_reset_time_position();
}

bool smart_island_is_expanded(void) { return g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED; }

void smart_island_set_page(smart_island_page_t page, bool anim_en) 
{
    if (anim_en) smart_island_page_apply_anim(page);
    else smart_island_page_apply_now(page);
    smart_island_page_indicator_sync(anim_en);
}

smart_island_page_t smart_island_get_page(void) { return g_smart_island_page; }

void smart_island_register_action_cb(smart_island_action_cb_t cb) { g_smart_island_action_cb = cb; }

void smart_island_refresh_language_texts(void)
{
    smart_island_action_page_refresh_language_texts();

    if (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE) {
        smart_island_apply_texts(ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_READY_TITLE),
                                 ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_READY_SUBTITLE));
    } else if (g_smart_island_scene == SMART_ISLAND_SCENE_COUNTING) {
        smart_island_apply_texts(ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNTING_TITLE),
                                 ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNTING_SUBTITLE));
    } else if (g_smart_island_scene == SMART_ISLAND_SCENE_WARNING) {
        smart_island_apply_texts(g_smart_island_warning_text,
                                 ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_WARNING_SUBTITLE));
    } else if (g_smart_island_scene == SMART_ISLAND_SCENE_RESULT) {
        smart_island_apply_texts(g_smart_island_result_text,
                                 ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RESULT_SUBTITLE));
    } else if (g_smart_island_scene == SMART_ISLAND_SCENE_UPDATE) {
        smart_island_apply_texts(g_smart_island_title ? lv_label_get_text(g_smart_island_title) : NULL,
                                 ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_SUBTITLE));
    } else if (g_smart_island_scene == SMART_ISLAND_SCENE_QR) {
        smart_island_apply_texts(g_smart_island_title ? lv_label_get_text(g_smart_island_title) : NULL,
                                 ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_QR_SUBTITLE));
    }
}

void smart_island_close(void) 
{
    g_smart_island_page = SMART_ISLAND_PAGE_INFO;
    smart_island_restore_idle();
    smart_island_action_page_set_index(0U, false);
    smart_island_page_indicator_sync(false);
    smart_island_reset_page_positions();
    smart_island_reset_compact_header_position();
    smart_island_reset_time_position();
    g_smart_island_ignore_click_once = false;
    g_smart_island_ignore_action_click_once = false;
}

void smart_island_open_info_page(void) 
{
    smart_island_expand_if_needed(true);
    smart_island_set_page(SMART_ISLAND_PAGE_INFO, true);
}

void smart_island_open_action_page(void) 
{
    smart_island_action_page_set_index(0U, false);
    smart_island_expand_if_needed(true);
    smart_island_set_page(SMART_ISLAND_PAGE_ACTION, true);
}
