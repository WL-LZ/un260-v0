#include "smart_island.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_components/lv_print_toast.h"
#include "un260/lv_components/lv_qr_popup.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/ui_qr_data.h"
#include "un260/lv_system/ui_text.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/page_18_pure.h"
#include <stdio.h>
#include <string.h>
#include "un260/lv_components/lv_capsule_pagination.h"
#include "lvgl/src/misc/lv_txt.h"

/* =========================
 * Smart Island - 旗舰高级版 (时间已完美修复)
 * ========================= */

/* 收起态：主界面B区位置 */
#define SMART_ISLAND_X                    492
#define SMART_ISLAND_Y                    348
#define SMART_ISLAND_W                    261
#define SMART_ISLAND_COMPACT_H            44
#define SMART_ISLAND_RADIUS               22

/* 功能页展开态：与B阶段一致 */
#define SMART_ISLAND_ACTION_EXPAND_H      112

/* 功能页按钮 (旗舰拟物排版) */
#define SMART_ISLAND_ACTION_PAGE_COUNT    4
#define SMART_ISLAND_ACTION_BTN_W         221 // 两侧留白 20px
#define SMART_ISLAND_ACTION_BTN_H         54
#define SMART_ISLAND_ACTION_BTN_X         20
#define SMART_ISLAND_ACTION_BTN_Y         29
#define SMART_ISLAND_PAGE_INDICATOR_Y    -6
/* 旗舰级色彩主题 (极度克制深邃) */
#define SMART_ISLAND_BG_IDLE              0x111111
#define SMART_ISLAND_BG_COUNTING          0x111111
#define SMART_ISLAND_BG_WARNING           0xF59E0B
#define SMART_ISLAND_BG_ERROR             0xFF5A5F
#define SMART_ISLAND_BG_SUCCESS           0x17A673
#define SMART_ISLAND_BG_UPDATE            0x111111
#define SMART_ISLAND_TEXT_LIGHT           0xFFFFFF
#define SMART_ISLAND_TEXT_SUB             0x777777 // 高级灰标签色
#define SMART_ISLAND_LAST_TEXT_GRAY       0x737373
#define SMART_ISLAND_RESULT_OK_COLOR      0x22C55E
#define SMART_ISLAND_RESULT_ISSUE_COLOR   0xFF5A5F
#define SMART_ISLAND_RESULT_ISSUE_TITLE_COLOR 0xFFD400
#define SMART_ISLAND_RESULT_DETAIL_GRAY   0xA3A3A3
#define SMART_ISLAND_RESULT_NEUTRAL_GRAY  0x737373
#define SMART_ISLAND_READY_DOT            0x00E676 // 极客绿点
#define SMART_ISLAND_DOT_NON_IDLE         0xFFFFFF
#define SMART_ISLAND_BTN_BG_TOP           0x1C1C1E // 按钮渐变浅色
#define SMART_ISLAND_BTN_BG_BOT           0x111111 // 按钮渐变深色
#define SMART_ISLAND_BTN_TEXT             0xFFFFFF
#define SMART_ISLAND_BTN_BORDER           0x2C2C2E
#define SMART_ISLAND_BTN_ARROW            0x8E8E93
#define SMART_ISLAND_BTN_SWITCH_ON_TOP    0x234A34
#define SMART_ISLAND_BTN_SWITCH_ON_BOT    0x1A3528
#define SMART_ISLAND_BTN_SWITCH_ON_TEXT   0xB8F5C7
#define SMART_ISLAND_BTN_SWITCH_OFF_TOP   0x2A2A2C
#define SMART_ISLAND_BTN_SWITCH_OFF_BOT   0x202022
#define SMART_ISLAND_BTN_SWITCH_OFF_TEXT  0xB0B0B2

/* 动画参数 */
#define SMART_ISLAND_EXPAND_TIME          300 
#define SMART_ISLAND_COLLAPSE_TIME        250
#define SMART_ISLAND_RESULT_HOLD_MS       1000
#define SMART_ISLAND_WARNING_MARQUEE_TIME 1680
#define SMART_ISLAND_WARNING_MARQUEE_CYCLES 2
#define SMART_ISLAND_WARNING_FLASH_TIME   1000
#define SMART_ISLAND_COLOR_ANIM_TIME      300

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
static lv_obj_t *g_smart_island_expand_last = NULL;
static lv_obj_t *g_smart_island_expand_divider = NULL;
static lv_obj_t *g_smart_island_expand_footer = NULL;
static lv_obj_t *g_smart_island_expand_extra = NULL;
static lv_obj_t *g_smart_island_quality_bar_bg = NULL;
static lv_obj_t *g_smart_island_quality_bar_fg = NULL;
static lv_obj_t *g_smart_island_quality_percent = NULL;

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
static smart_island_warning_level_t g_smart_island_warning_level = SMART_ISLAND_WARNING_LEVEL_WARNING;
static bool g_smart_island_warning_marquee_running = false;
static uint8_t g_smart_island_warning_marquee_step = 0;
static lv_coord_t g_smart_island_warning_text_w_compact = 0;
static lv_coord_t g_smart_island_warning_text_w_expand = 0;
static bool g_smart_island_created = false;
static bool g_smart_island_anim_running = false;
static bool g_smart_island_ignore_click_once = false;
static bool g_smart_island_ignore_action_click_once = false;
static bool g_smart_island_pure_count_enabled = false;
static uint8_t g_smart_island_action_page_count = SMART_ISLAND_ACTION_PAGE_COUNT;
static uint8_t g_smart_island_action_page_index = 0;
static int8_t g_smart_island_page_slide_dir = 0; // 0:默认 1:向左切页 -1:向右切页
static uint32_t g_smart_island_bg_cur = SMART_ISLAND_BG_IDLE;
static uint32_t g_smart_island_bg_from = SMART_ISLAND_BG_IDLE;
static uint32_t g_smart_island_bg_to = SMART_ISLAND_BG_IDLE;
static bool g_smart_island_bg_anim_running = false;
static struct {
    bool pressed;
    bool swiped;
    lv_point_t start_pt;
} g_smart_island_swipe = { false, false, {0, 0} };

/* 文本缓存 */
static char g_smart_island_warning_text[64];
static char g_smart_island_result_text[64];
static char g_smart_island_compact_text[196];
static char g_smart_island_info_title_text[196];
static char g_smart_island_info_summary_text[196];
static char g_smart_island_info_footer_text[196];
static char g_smart_island_info_extra_text[196];
static char g_smart_island_idle_custom_line1[96];
static char g_smart_island_idle_custom_line2[96];
static char g_smart_island_idle_custom_line3[96];
static uint8_t g_smart_island_idle_quality_percent = 100;
static bool g_smart_island_idle_has_issue = false;
static bool g_smart_island_idle_has_data = false;
static bool g_smart_island_idle_no_count = false;
static bool g_smart_island_count_session_active = false;
static int g_smart_island_reject_base_expected = 0;
static int g_smart_island_reject_base_detail = 0;

/* 内部函数声明 */
static void smart_island_enable_gesture_on_obj(lv_obj_t *obj); 
static void smart_island_swipe_cb(lv_event_t *e); 
static void smart_island_apply_scene_style(void); 
static void smart_island_apply_texts(void); 
static void smart_island_rebuild_scene_texts(void);
static void smart_island_update_idle_time(void); 
static void smart_island_get_currency_code(char *buf, size_t size);
static const char *smart_island_get_work_mode_text(void);
static bool smart_island_batch_enabled(void);
static uint16_t smart_island_get_reject_count(void);
static void smart_island_stop_result_timer(void); 
static void smart_island_stop_warning_timer(void); 
static void smart_island_update_pages_visible(void); 
static void smart_island_pulse_stop(void); 
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
static void smart_island_action_page_slide_anim_finish_cb(lv_anim_t *a);
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
static void smart_island_update_time_clickable(void);
static void smart_island_warning_marquee_start(void);
static void smart_island_warning_marquee_stop(void);
static void smart_island_warning_apply_static_layout(void);
static void smart_island_warning_marquee_finish_cb(lv_anim_t *a);
static void smart_island_warning_marquee_timeout_cb(lv_timer_t *timer);
static void smart_island_warning_marquee_run_step(void);
static void smart_island_warning_flash_finish_cb(lv_anim_t *a);
static void smart_island_bg_color_apply_anim(uint32_t dst_hex);
static void smart_island_bg_color_anim_ready_cb(lv_anim_t *a);
static void smart_island_apply_idle_line_text(char *dst, size_t dst_size, const char *text);
static void smart_island_apply_quality_indicator(void);
static bool smart_island_currency_error_is_suspect(uint8_t code);
static bool smart_island_currency_error_is_damaged(uint8_t code);
static int smart_island_get_reject_detail_total(void);
static void smart_island_action_btn_touch_anim_cb(lv_event_t *e);
static void smart_island_action_btn_set_pressed_visual(lv_obj_t *btn, bool pressed);
static void smart_island_action_btn_style_apply(uint8_t index);
static void smart_island_time_click_cb(lv_event_t *e);

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

static void smart_island_action_btn_touch_anim_cb(lv_event_t *e)
{
    lv_obj_t *btn;
    lv_event_code_t code;

    if (e == NULL) {
        return;
    }

    btn = lv_event_get_target(e);
    if (btn == NULL || !lv_obj_is_valid(btn)) {
        return;
    }

    code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        lv_obj_set_style_translate_y(btn, 1, 0);
        smart_island_action_btn_set_pressed_visual(btn, true);
        return;
    }

    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_set_style_translate_y(btn, 0, 0);
        smart_island_action_btn_set_pressed_visual(btn, false);
    }
}

static void smart_island_action_btn_set_pressed_visual(lv_obj_t *btn, bool pressed)
{
    uint8_t i;
    lv_obj_t *label = NULL;
    lv_obj_t *arrow = NULL;

    if (btn == NULL || !lv_obj_is_valid(btn)) {
        return;
    }

    lv_obj_set_style_bg_opa(btn, pressed ? LV_OPA_70 : LV_OPA_COVER, 0);

    for (i = 0; i < SMART_ISLAND_ACTION_PAGE_COUNT; i++) {
        if (g_smart_island_action_btns[i] == btn) {
            label = g_smart_island_action_labels[i];
            arrow = g_smart_island_action_arrows[i];
            break;
        }
    }

    if (label && lv_obj_is_valid(label)) {
        lv_obj_set_style_text_opa(label, pressed ? LV_OPA_70 : LV_OPA_COVER, 0);
    }

    if (arrow && lv_obj_is_valid(arrow)) {
        lv_obj_set_style_text_opa(arrow, pressed ? LV_OPA_70 : LV_OPA_COVER, 0);
    }
}
static void smart_island_anim_bg_color_cb(void *var, int32_t v)
{
    lv_color_t from_c;
    lv_color_t to_c;
    lv_color_t mix_c;

    if (var == NULL) {
        return;
    }

    from_c = lv_color_hex(g_smart_island_bg_from);
    to_c = lv_color_hex(g_smart_island_bg_to);
    mix_c = lv_color_mix(to_c, from_c, (lv_opa_t)v);
    lv_obj_set_style_bg_color((lv_obj_t *)var, mix_c, 0);
}
static void smart_island_anim_finish_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    g_smart_island_anim_running = false;
}

static void smart_island_action_page_slide_anim_finish_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    g_smart_island_anim_running = false;
    g_smart_island_ignore_action_click_once = false;
}

static void smart_island_bg_color_anim_ready_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    g_smart_island_bg_anim_running = false;
    g_smart_island_bg_cur = g_smart_island_bg_to;
}

static void smart_island_bg_color_apply_anim(uint32_t dst_hex)
{
    lv_anim_t a;
    lv_color_t style_c;

    if (g_smart_island == NULL || !lv_obj_is_valid(g_smart_island)) {
        return;
    }

    if (g_smart_island_bg_anim_running && g_smart_island_bg_to == dst_hex) {
        return;
    }

    if (!g_smart_island_bg_anim_running && g_smart_island_bg_cur == dst_hex) {
        lv_obj_set_style_bg_color(g_smart_island, lv_color_hex(dst_hex), 0);
        return;
    }

    style_c = lv_obj_get_style_bg_color(g_smart_island, LV_PART_MAIN);
    g_smart_island_bg_from = lv_color_to32(style_c);
    g_smart_island_bg_to = dst_hex;

    lv_anim_del(g_smart_island, smart_island_anim_bg_color_cb);
    lv_anim_init(&a);
    lv_anim_set_var(&a, g_smart_island);
    lv_anim_set_exec_cb(&a, smart_island_anim_bg_color_cb);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, SMART_ISLAND_COLOR_ANIM_TIME);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_ready_cb(&a, smart_island_bg_color_anim_ready_cb);
    lv_anim_start(&a);
    g_smart_island_bg_anim_running = true;
}

static void smart_island_warning_marquee_stop(void)
{
    if (g_smart_island_title && lv_obj_is_valid(g_smart_island_title)) {
        lv_anim_del(g_smart_island_title, smart_island_anim_x_cb);
        lv_anim_del(g_smart_island_title, smart_island_anim_text_opa_cb);
        lv_obj_set_style_text_opa(g_smart_island_title, LV_OPA_COVER, 0);
    }
    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        lv_anim_del(g_smart_island_expand_title, smart_island_anim_x_cb);
        lv_anim_del(g_smart_island_expand_title, smart_island_anim_text_opa_cb);
        lv_obj_set_style_text_opa(g_smart_island_expand_title, LV_OPA_COVER, 0);
    }
    g_smart_island_warning_marquee_running = false;
    g_smart_island_warning_marquee_step = 0;
    g_smart_island_warning_text_w_compact = 0;
    g_smart_island_warning_text_w_expand = 0;
    smart_island_reset_compact_header_position();
    smart_island_warning_apply_static_layout();
}

static void smart_island_warning_apply_static_layout(void)
{
    lv_coord_t compact_visible = SMART_ISLAND_W - 36 - 14;
    lv_coord_t expand_visible = SMART_ISLAND_W - 32 - 12;

    if (g_smart_island_scene != SMART_ISLAND_SCENE_WARNING) {
        return;
    }

    if (g_smart_island_title && lv_obj_is_valid(g_smart_island_title)) {
        lv_label_set_long_mode(g_smart_island_title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_smart_island_title, compact_visible);
        lv_obj_set_x(g_smart_island_title, 36);
        lv_obj_set_y(g_smart_island_title, 13);
    }

    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        lv_label_set_long_mode(g_smart_island_expand_title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_smart_island_expand_title, expand_visible);
        lv_obj_set_x(g_smart_island_expand_title, 20);
        lv_obj_set_y(g_smart_island_expand_title, 18);
    }
}

static void smart_island_warning_marquee_finish_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    if (!g_smart_island_warning_marquee_running) return;
    g_smart_island_warning_marquee_step++;
    smart_island_warning_marquee_run_step();
}

static void smart_island_warning_marquee_timeout_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    smart_island_stop_warning_timer();
    if (!fault_popup_is_showing()) {
        smart_island_restore_idle();
        fault_popup_schedule_auto_confirm();
    }
}

static void smart_island_warning_flash_finish_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    smart_island_warning_marquee_stop();
    if (!fault_popup_is_showing()) {
        smart_island_restore_idle();
        fault_popup_schedule_auto_confirm();
    }
}

static void smart_island_warning_marquee_start(void)
{
    lv_anim_t a;
    lv_coord_t text_w;
    lv_coord_t expand_text_w;
    const char *title_text;
    const char *expand_text;
    const lv_font_t *title_font;
    const lv_font_t *expand_font;
    lv_coord_t compact_visible = SMART_ISLAND_W - 36 - 14;
    lv_coord_t expand_visible = SMART_ISLAND_W - 32 - 12;

    if (g_smart_island_title == NULL || !lv_obj_is_valid(g_smart_island_title)) {
        return;
    }

    if (g_smart_island_scene != SMART_ISLAND_SCENE_WARNING) {
        return;
    }

    smart_island_warning_marquee_stop();
    title_text = lv_label_get_text(g_smart_island_title);
    title_font = lv_obj_get_style_text_font(g_smart_island_title, LV_PART_MAIN);
    text_w = (lv_coord_t)lv_txt_get_width(title_text ? title_text : "",
                                          (uint32_t)strlen(title_text ? title_text : ""),
                                          title_font,
                                          lv_obj_get_style_text_letter_space(g_smart_island_title, LV_PART_MAIN),
                                          LV_TEXT_FLAG_NONE);
    g_smart_island_warning_text_w_compact = text_w;

    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        expand_text = lv_label_get_text(g_smart_island_expand_title);
        expand_font = lv_obj_get_style_text_font(g_smart_island_expand_title, LV_PART_MAIN);
        expand_text_w = (lv_coord_t)lv_txt_get_width(expand_text ? expand_text : "",
                                                     (uint32_t)strlen(expand_text ? expand_text : ""),
                                                     expand_font,
                                                     lv_obj_get_style_text_letter_space(g_smart_island_expand_title, LV_PART_MAIN),
                                                     LV_TEXT_FLAG_NONE);
        g_smart_island_warning_text_w_expand = expand_text_w;
    }

    if (g_smart_island_warning_text_w_compact <= compact_visible) {
        lv_coord_t compact_center_x = (SMART_ISLAND_W - g_smart_island_warning_text_w_compact) / 2;
        if (compact_center_x < 0) compact_center_x = 0;

        lv_label_set_long_mode(g_smart_island_title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_smart_island_title, LV_SIZE_CONTENT);
        lv_obj_set_x(g_smart_island_title, compact_center_x);
        lv_obj_set_y(g_smart_island_title, 13);

        lv_anim_init(&a);
        lv_anim_set_var(&a, g_smart_island_title);
        lv_anim_set_exec_cb(&a, smart_island_anim_text_opa_cb);
        lv_anim_set_values(&a, LV_OPA_100, LV_OPA_40);
        lv_anim_set_time(&a, SMART_ISLAND_WARNING_FLASH_TIME);
        lv_anim_set_playback_time(&a, SMART_ISLAND_WARNING_FLASH_TIME);
        lv_anim_set_repeat_count(&a, 3);
        lv_anim_set_path_cb(&a, lv_anim_path_linear);
        lv_anim_set_ready_cb(&a, smart_island_warning_flash_finish_cb);
        lv_anim_start(&a);

        if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
            lv_coord_t expand_center_x = (SMART_ISLAND_W - g_smart_island_warning_text_w_expand) / 2;
            if (expand_center_x < 0) expand_center_x = 0;

            lv_label_set_long_mode(g_smart_island_expand_title, LV_LABEL_LONG_CLIP);
            lv_obj_set_width(g_smart_island_expand_title, LV_SIZE_CONTENT);
            lv_obj_set_x(g_smart_island_expand_title, expand_center_x);
            lv_obj_set_y(g_smart_island_expand_title, 30);

            lv_anim_init(&a);
            lv_anim_set_var(&a, g_smart_island_expand_title);
            lv_anim_set_exec_cb(&a, smart_island_anim_text_opa_cb);
            lv_anim_set_values(&a, LV_OPA_100, LV_OPA_40);
            lv_anim_set_time(&a, SMART_ISLAND_WARNING_FLASH_TIME);
            lv_anim_set_playback_time(&a, SMART_ISLAND_WARNING_FLASH_TIME);
            lv_anim_set_repeat_count(&a, 3);
            lv_anim_set_path_cb(&a, lv_anim_path_linear);
            lv_anim_start(&a);
        }
        g_smart_island_warning_marquee_running = true;
        return;
    }

    g_smart_island_warning_marquee_running = true;
    g_smart_island_warning_marquee_step = 0;
    lv_label_set_long_mode(g_smart_island_title, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_smart_island_title, compact_visible);
    lv_obj_set_x(g_smart_island_title, 36);
    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        lv_label_set_long_mode(g_smart_island_expand_title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_smart_island_expand_title, expand_visible);
        lv_obj_set_x(g_smart_island_expand_title, 32);
    }
    LV_UNUSED(a);
    smart_island_warning_marquee_run_step();
}

static void smart_island_warning_marquee_run_step(void)
{
    lv_anim_t a;
    lv_coord_t compact_visible = SMART_ISLAND_W - 36 - 14;
    lv_coord_t expand_visible = SMART_ISLAND_W - 32 - 12;
    lv_coord_t compact_left = (lv_coord_t)(36 - g_smart_island_warning_text_w_compact);
    lv_coord_t compact_right = (lv_coord_t)(36 + compact_visible);
    lv_coord_t expand_left = (lv_coord_t)(32 - g_smart_island_warning_text_w_expand);
    lv_coord_t expand_right = (lv_coord_t)(32 + expand_visible);
    bool left_to_right;
    lv_coord_t from_x;
    lv_coord_t to_x;

    if (!g_smart_island_warning_marquee_running) return;

    if (g_smart_island_warning_marquee_step >= (uint8_t)(SMART_ISLAND_WARNING_MARQUEE_CYCLES * 2U)) {
        smart_island_warning_marquee_stop();
        if (!fault_popup_is_showing()) {
            smart_island_restore_idle();
            fault_popup_schedule_auto_confirm();
        }
        return;
    }

    left_to_right = ((g_smart_island_warning_marquee_step % 2U) == 0U);
    from_x = left_to_right ? compact_left : compact_right;
    to_x = left_to_right ? compact_right : compact_left;

    lv_anim_init(&a);
    lv_anim_set_var(&a, g_smart_island_title);
    lv_anim_set_exec_cb(&a, smart_island_anim_x_cb);
    lv_anim_set_values(&a, from_x, to_x);
    lv_anim_set_time(&a, SMART_ISLAND_WARNING_MARQUEE_TIME);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_ready_cb(&a, smart_island_warning_marquee_finish_cb);
    lv_anim_start(&a);

    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title) &&
        g_smart_island_warning_text_w_expand > expand_visible) {
        from_x = left_to_right ? expand_left : expand_right;
        to_x = left_to_right ? expand_right : expand_left;
        lv_anim_init(&a);
        lv_anim_set_var(&a, g_smart_island_expand_title);
        lv_anim_set_exec_cb(&a, smart_island_anim_x_cb);
        lv_anim_set_values(&a, from_x, to_x);
        lv_anim_set_time(&a, SMART_ISLAND_WARNING_MARQUEE_TIME);
        lv_anim_set_path_cb(&a, lv_anim_path_linear);
        lv_anim_start(&a);
    }
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
        lv_obj_set_style_translate_x(g_smart_island_dot, 0, 0);
    }
    if (g_smart_island_title && lv_obj_is_valid(g_smart_island_title)) {
        lv_label_set_long_mode(g_smart_island_title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_smart_island_title, 150);
        lv_obj_set_x(g_smart_island_title, 36);
        lv_obj_set_y(g_smart_island_title, 13);
        lv_obj_set_style_translate_x(g_smart_island_title, 0, 0);
    }
    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        lv_label_set_long_mode(g_smart_island_expand_title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_smart_island_expand_title, 213);
        lv_obj_set_x(g_smart_island_expand_title, 24);
        lv_obj_set_y(g_smart_island_expand_title, 30);
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
    if (g_smart_island_scene == SMART_ISLAND_SCENE_RESULT) {
        return;
    }

    if (g_smart_island_visual != SMART_ISLAND_VISUAL_EXPANDED) {
        smart_island_set_visual(SMART_ISLAND_VISUAL_EXPANDED, anim_en);
    }
}
static void smart_island_swipe_cb(lv_event_t *e) 
{
    lv_event_code_t code;
    lv_indev_t *indev;
    lv_point_t pt;

    if (g_smart_island_scene == SMART_ISLAND_SCENE_WARNING) return;
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

static void smart_island_modal_click_cb(lv_event_t *e) 
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    smart_island_close();
}
static void smart_island_click_cb(lv_event_t *e) 
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    if (g_smart_island_scene == SMART_ISLAND_SCENE_WARNING) {
        if (g_smart_island_ignore_click_once) {
            g_smart_island_ignore_click_once = false;
            return;
        }
        if (fault_popup_show_pending_now()) {
            smart_island_warning_marquee_stop();
        }
        return;
    }

    if (g_smart_island_anim_running) return;

    if (g_smart_island_ignore_click_once) {
        g_smart_island_ignore_click_once = false;
        return;
    }

    /* 点钞完成态只保留绿色收起岛，不允许点开展开页 */
    if (g_smart_island_scene == SMART_ISLAND_SCENE_RESULT) {
        return;
    }

    if (g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED) {
        smart_island_close();
    } else {
        smart_island_open_info_page();
    }
}

static void smart_island_time_click_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    if (g_smart_island_anim_running) {
        return;
    }

    if (g_smart_island_visual != SMART_ISLAND_VISUAL_EXPANDED) {
        return;
    }

    /* 时间点击优先进入设置页，阻断冒泡避免触发岛收起 */
    lv_event_stop_bubbling(e);
    ui_manager_push_page(UI_PAGE_TIMESET);
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

    if (action_id == SMART_ISLAND_ACTION_FUNC4) {
        bool enabled = smart_island_pure_count_is_enabled();
        smart_island_set_pure_count_enabled(!enabled);
        ui_state_save_pure_count_state();
        smart_island_action_item_apply(page_index);

        if (!enabled) {
            smart_island_close();
            ui_manager_push_page(UI_PAGE_PURE);
        } else {
            if (ui_manager_get_current_page() == UI_PAGE_PURE) {
                ui_page_18_pure_request_exit();
            } else {
                ui_manager_switch(UI_PAGE_MAIN);
            }
        }
        return;
    }

    if (action_id == SMART_ISLAND_ACTION_QR) {
        smart_island_show_qr_popup();
        return;
    }

    if (action_id == SMART_ISLAND_ACTION_FUNC3) {
        bool enabled = fault_popup_get_auto_enabled();
        fault_popup_set_auto_enabled(!enabled);
        ui_state_save_popup_auto_state();
        smart_island_action_item_apply(page_index);
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

static void smart_island_get_currency_code(char *buf, size_t size)
{
    if (buf == NULL || size == 0U) return;

    if (Machine_para.curr_code[0] != '\0') {
        lv_snprintf(buf, size, "%s", Machine_para.curr_code);
    } else if (Machine_para.selected_currency < Machine_para.currency_count &&
               Machine_para.currencies[Machine_para.selected_currency][0] != '\0') {
        lv_snprintf(buf, size, "%s", Machine_para.currencies[Machine_para.selected_currency]);
    } else {
        lv_snprintf(buf, size, "%s", "CUR");
    }
}

static const char *smart_island_get_work_mode_text(void)
{
    return Machine_para.work_mode ? ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_MODE_MANUAL)
                                  : ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_MODE_AUTO);
}

static bool smart_island_batch_enabled(void)
{
    return Machine_para.batch_switch_enable && Machine_para.batch_num > 0 && Machine_para.batch_num != 200;
}

static uint16_t smart_island_get_reject_count(void)
{
    return sim.err_num > 0 ? sim.err_num : sim.err_expected;
}

static void smart_island_apply_idle_line_text(char *dst, size_t dst_size, const char *text)
{
    if (dst == NULL || dst_size == 0U) {
        return;
    }

    if (text && text[0] != '\0') {
        lv_snprintf(dst, dst_size, "%s", text);
    } else {
        dst[0] = '\0';
    }
}

static bool smart_island_currency_error_is_suspect(uint8_t code)
{
    if (code >= 0x01 && code <= 0x0F) return true; /* IMG F1~F15 */
    if (code == 0x11 || code == 0x12 || code == 0x13 || code == 0x14) return true; /* MG/MT */
    if (code == 0x15 || code == 0x22 || code == 0x23) return true; /* UV/IR */
    if (code == 0x16 || code == 0x17 || code == 0x31) return true; /* Double */
    if (code == 0x1D || code == 0x1E || code == 0x1F || code == 0x20 || code == 0x21) return true; /* Version/Face/Ort/Angle */
    if (code == 0x2D || code == 0x2E || code == 0x2F || code == 0x30) return true; /* IMG F&O / OCR */
    if (code == 0x1C) return true; /* Size Unknow */
    return false;
}

static bool smart_island_currency_error_is_damaged(uint8_t code)
{
    if (code == 0x18 || code == 0x19 || code == 0x1A || code == 0x2C) return true; /* Long/Short/GAP/Limpness */
    if (code == 0x24 || code == 0x25 || code == 0x27 || code == 0x28 || code == 0x29) return true; /* Hole/DogEar/Tape/Tears/Crumples */
    if (code == 0x26 || code == 0x2A || code == 0x2B) return true; /* DIRT/De_ink/Soiling */
    return false;
}

static int smart_island_get_reject_detail_total(void)
{
    int total = 0;

    if (sim.err_num == 0 || sim.err_pcs == NULL) {
        return 0;
    }

    for (int i = 0; i < sim.err_num; i++) {
        total += sim.err_pcs[i];
    }
    return total;
}

static void smart_island_apply_quality_indicator(void)
{
    lv_coord_t fg_w;
    char percent_buf[8];

    if (g_smart_island_quality_bar_bg == NULL || !lv_obj_is_valid(g_smart_island_quality_bar_bg) ||
        g_smart_island_quality_bar_fg == NULL || !lv_obj_is_valid(g_smart_island_quality_bar_fg) ||
        g_smart_island_quality_percent == NULL || !lv_obj_is_valid(g_smart_island_quality_percent)) {
        return;
    }

    /* 不要跟随 page 在切页动画中反复 hidden/unhidden，避免回到信息页时闪一下 */
    if (!(g_smart_island_scene == SMART_ISLAND_SCENE_IDLE &&
          g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED)) {
        lv_obj_add_flag(g_smart_island_quality_bar_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_smart_island_quality_percent, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    fg_w = (lv_coord_t)((56 * g_smart_island_idle_quality_percent) / 100);
    if (fg_w < 0) {
        fg_w = 0;
    }
    if (fg_w > 56) {
        fg_w = 56;
    }

    lv_obj_set_width(g_smart_island_quality_bar_fg, fg_w);

    if (!g_smart_island_idle_has_data) {
        lv_obj_set_style_bg_color(g_smart_island_quality_bar_bg, lv_color_hex(SMART_ISLAND_RESULT_NEUTRAL_GRAY), 0);
        lv_obj_set_style_bg_color(g_smart_island_quality_bar_fg, lv_color_hex(SMART_ISLAND_RESULT_NEUTRAL_GRAY), 0);
    } else {
        lv_obj_set_style_bg_color(g_smart_island_quality_bar_bg, lv_color_hex(SMART_ISLAND_RESULT_ISSUE_COLOR), 0);
        lv_obj_set_style_bg_color(g_smart_island_quality_bar_fg, lv_color_hex(SMART_ISLAND_RESULT_OK_COLOR), 0);
    }

    lv_snprintf(percent_buf, sizeof(percent_buf), "%u%%", (unsigned)g_smart_island_idle_quality_percent);
    lv_label_set_text(g_smart_island_quality_percent, percent_buf);
    if (!g_smart_island_idle_has_data) {
        lv_obj_set_style_text_color(g_smart_island_quality_percent, lv_color_hex(SMART_ISLAND_RESULT_NEUTRAL_GRAY), 0);
    } else if (g_smart_island_idle_has_issue) {
        lv_obj_set_style_text_color(g_smart_island_quality_percent, lv_color_hex(SMART_ISLAND_TEXT_LIGHT), 0);
    } else {
        lv_obj_set_style_text_color(g_smart_island_quality_percent, lv_color_hex(SMART_ISLAND_RESULT_OK_COLOR), 0);
    }

    lv_obj_clear_flag(g_smart_island_quality_bar_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_smart_island_quality_percent, LV_OBJ_FLAG_HIDDEN);
}

static void smart_island_apply_texts(void)
{
    if (g_smart_island_title && lv_obj_is_valid(g_smart_island_title)) {
        lv_label_set_text(g_smart_island_title, g_smart_island_compact_text);
    }
    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        lv_label_set_text(g_smart_island_expand_title, g_smart_island_info_title_text);
    }
    if (g_smart_island_expand_subtitle && lv_obj_is_valid(g_smart_island_expand_subtitle)) {
        lv_label_set_text(g_smart_island_expand_subtitle, g_smart_island_info_summary_text);
    }
    if (g_smart_island_expand_last && lv_obj_is_valid(g_smart_island_expand_last)) {
        lv_label_set_text(g_smart_island_expand_last, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_LAST_TAG));
    }
    if (g_smart_island_expand_footer && lv_obj_is_valid(g_smart_island_expand_footer)) {
        lv_label_set_text(g_smart_island_expand_footer, g_smart_island_info_footer_text);
        if (g_smart_island_info_footer_text[0] != '\0') lv_obj_clear_flag(g_smart_island_expand_footer, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_smart_island_expand_footer, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_smart_island_expand_extra && lv_obj_is_valid(g_smart_island_expand_extra)) {
        lv_label_set_text(g_smart_island_expand_extra, g_smart_island_info_extra_text);
        if (g_smart_island_info_extra_text[0] != '\0') lv_obj_clear_flag(g_smart_island_expand_extra, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_smart_island_expand_extra, LV_OBJ_FLAG_HIDDEN);
    }
}

static void smart_island_rebuild_scene_texts(void)
{
    char curr[8] = {0};
    char detail_line[96] = {0};
    const char *work_text = smart_island_get_work_mode_text();
    uint16_t reject_count = smart_island_get_reject_count();

    memset(g_smart_island_compact_text, 0, sizeof(g_smart_island_compact_text));
    memset(g_smart_island_info_title_text, 0, sizeof(g_smart_island_info_title_text));
    memset(g_smart_island_info_summary_text, 0, sizeof(g_smart_island_info_summary_text));
    memset(g_smart_island_info_footer_text, 0, sizeof(g_smart_island_info_footer_text));
    memset(g_smart_island_info_extra_text, 0, sizeof(g_smart_island_info_extra_text));

    smart_island_get_currency_code(curr, sizeof(curr));

    switch (g_smart_island_scene) {
    case SMART_ISLAND_SCENE_COUNTING:
        lv_snprintf(g_smart_island_compact_text, sizeof(g_smart_island_compact_text), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNTING_TITLE));
        lv_snprintf(g_smart_island_info_title_text, sizeof(g_smart_island_info_title_text), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNTING_INFO_TITLE));
        if (sim.total_pcs > 0 && sim.total_amount > 0.0f) {
            lv_snprintf(g_smart_island_info_summary_text, sizeof(g_smart_island_info_summary_text),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_CUR_PCS_AMOUNT_FMT), curr, sim.total_pcs, sim.total_amount);
        } else if (sim.total_pcs > 0) {
            lv_snprintf(g_smart_island_info_summary_text, sizeof(g_smart_island_info_summary_text),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_CUR_PCS_FMT), curr, sim.total_pcs);
        } else {
            lv_snprintf(g_smart_island_info_summary_text, sizeof(g_smart_island_info_summary_text),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_CUR_MODE_FMT), curr, work_text);
        }
        if (smart_island_batch_enabled() && sim.total_pcs > 0) {
            lv_snprintf(g_smart_island_info_footer_text, sizeof(g_smart_island_info_footer_text),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_BATCH_PROGRESS_FMT), sim.total_pcs, (int)Machine_para.batch_num);
        } else {
            lv_snprintf(g_smart_island_info_footer_text, sizeof(g_smart_island_info_footer_text), "%s", work_text);
        }
        break;

    case SMART_ISLAND_SCENE_RESULT:
        lv_snprintf(g_smart_island_compact_text, sizeof(g_smart_island_compact_text), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_FINISHED));
        g_smart_island_info_title_text[0] = '\0';
        g_smart_island_info_summary_text[0] = '\0';
        g_smart_island_info_footer_text[0] = '\0';
        g_smart_island_info_extra_text[0] = '\0';
        break;

    case SMART_ISLAND_SCENE_WARNING:
        lv_snprintf(g_smart_island_compact_text, sizeof(g_smart_island_compact_text), "%s",
            smart_island_text_or_default(g_smart_island_warning_text, UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR));
        lv_snprintf(g_smart_island_info_title_text, sizeof(g_smart_island_info_title_text), "%s",
            smart_island_text_or_default(g_smart_island_warning_text, UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR));
        lv_snprintf(g_smart_island_info_summary_text, sizeof(g_smart_island_info_summary_text), "%s",
            smart_island_text_or_default(g_smart_island_warning_text, UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR));
        lv_snprintf(g_smart_island_info_footer_text, sizeof(g_smart_island_info_footer_text), "%s",
            "");
        break;

    case SMART_ISLAND_SCENE_UPDATE:
        lv_snprintf(g_smart_island_compact_text, sizeof(g_smart_island_compact_text), "%s",
            smart_island_text_or_default(g_smart_island_content.title, UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_TITLE));
        lv_snprintf(g_smart_island_info_title_text, sizeof(g_smart_island_info_title_text), "%s",
            smart_island_text_or_default(g_smart_island_content.title, UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_TITLE));
        lv_snprintf(g_smart_island_info_summary_text, sizeof(g_smart_island_info_summary_text), "%s",
            smart_island_text_or_default(g_smart_island_content.subtitle, UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_SUBTITLE));
        lv_snprintf(g_smart_island_info_footer_text, sizeof(g_smart_island_info_footer_text), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_INFO_FOOTER));
        break;

    case SMART_ISLAND_SCENE_QR:
        lv_snprintf(g_smart_island_compact_text, sizeof(g_smart_island_compact_text), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_QR_READY));
        lv_snprintf(g_smart_island_info_title_text, sizeof(g_smart_island_info_title_text), "%s",
            smart_island_text_or_default(g_smart_island_content.title, UI_TEXT_WIDGET_SMART_ISLAND_QR_READY));
        lv_snprintf(g_smart_island_info_summary_text, sizeof(g_smart_island_info_summary_text), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_QR_INFO_SUBTITLE));
        if (sim.total_pcs > 0 && sim.total_amount > 0.0f) {
            lv_snprintf(g_smart_island_info_footer_text, sizeof(g_smart_island_info_footer_text),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_CUR_PCS_AMOUNT_FMT), curr, sim.total_pcs, sim.total_amount);
        } else {
            lv_snprintf(g_smart_island_info_footer_text, sizeof(g_smart_island_info_footer_text), "%s",
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_QR_INFO_FOOTER));
        }
        break;

    case SMART_ISLAND_SCENE_IDLE:
    default:
    {
        int current_valid = sim.total_pcs;
        int current_total = 0;
        int current_issue = 0;
        int current_suspect = 0;
        int current_damaged = 0;
        int issue_baseline = 0;

        lv_snprintf(g_smart_island_compact_text, sizeof(g_smart_island_compact_text),
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_READY_CUR_FMT), curr);

        g_smart_island_info_title_text[0] = '\0';

        issue_baseline = Machine_para.add_enable ? 0 : g_smart_island_reject_base_detail;

        if (sim.err_num > 0 && sim.err_pcs != NULL && sim.err_code != NULL) {
            for (int i = 0; i < sim.err_num; i++) {
                int pcs = sim.err_pcs[i];
                uint8_t code = sim.err_code[i];

                if (pcs <= 0) {
                    continue;
                }

                if (!Machine_para.add_enable && issue_baseline > 0) {
                    if (pcs <= issue_baseline) {
                        issue_baseline -= pcs;
                        continue;
                    }
                    pcs -= issue_baseline;
                    issue_baseline = 0;
                }

                if (smart_island_currency_error_is_damaged(code)) {
                    current_damaged += pcs;
                } else if (smart_island_currency_error_is_suspect(code)) {
                    current_suspect += pcs;
                } else {
                    current_suspect += pcs;
                }
            }
            current_issue = current_suspect + current_damaged;
        } else if (sim.err_expected > 0) {
            int expected_issue = sim.err_expected;
            int expected_baseline = Machine_para.add_enable ? 0 : g_smart_island_reject_base_expected;
            if (!Machine_para.add_enable) {
                if (expected_issue > expected_baseline) {
                    expected_issue -= expected_baseline;
                } else {
                    expected_issue = 0;
                }
            }
            current_issue = expected_issue;
            current_suspect = expected_issue;
            current_damaged = 0;
        }

        if (current_issue < 0) {
            current_issue = 0;
        }

        if (current_valid < 0) {
            current_valid = 0;
        }

        current_total = current_valid + current_issue;
        g_smart_island_idle_has_issue = (current_issue > 0);
        g_smart_island_idle_has_data = (current_total > 0);
        g_smart_island_idle_no_count = (sim.total_pcs == 0 && sim.total_amount <= 0.0f && reject_count == 0);

        if (g_smart_island_idle_custom_line1[0] != '\0') {
            lv_snprintf(g_smart_island_info_summary_text, sizeof(g_smart_island_info_summary_text), "%s",
                g_smart_island_idle_custom_line1);
        } else if (sim.last_total_pcs > 0 || sim.last_total_amount > 0.0f) {
            lv_snprintf(g_smart_island_info_summary_text, sizeof(g_smart_island_info_summary_text),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_PCS_AMOUNT_FMT), sim.last_total_pcs, sim.last_total_amount);
        } else if (Machine_para.last_total_pcs > 0U || Machine_para.last_total_amount > 0U) {
            lv_snprintf(g_smart_island_info_summary_text, sizeof(g_smart_island_info_summary_text),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_PCS_AMOUNT_FMT),
                (int)Machine_para.last_total_pcs, (float)Machine_para.last_total_amount);
        } else {
            lv_snprintf(g_smart_island_info_summary_text, sizeof(g_smart_island_info_summary_text),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_PCS_AMOUNT_FMT), 0, 0.0f);
        }

        if (g_smart_island_idle_custom_line2[0] != '\0') {
            lv_snprintf(g_smart_island_info_footer_text, sizeof(g_smart_island_info_footer_text), "%s",
                g_smart_island_idle_custom_line2);
        } else if (g_smart_island_idle_no_count) {
            lv_snprintf(g_smart_island_info_footer_text, sizeof(g_smart_island_info_footer_text), "%s",
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_IDLE_NO_COUNT));
        } else {
            lv_snprintf(g_smart_island_info_footer_text, sizeof(g_smart_island_info_footer_text), "%s",
                g_smart_island_idle_has_issue
                ? ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RESULT_ISSUE_TITLE)
                : ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RESULT_OK_TITLE));
        }

        if (g_smart_island_idle_custom_line3[0] != '\0') {
            lv_snprintf(g_smart_island_info_extra_text, sizeof(g_smart_island_info_extra_text), "%s",
                g_smart_island_idle_custom_line3);
        } else if (g_smart_island_idle_no_count) {
            lv_snprintf(g_smart_island_info_extra_text, sizeof(g_smart_island_info_extra_text), "%s",
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_PLACE_BANKNOTES));
        } else {
            if (g_smart_island_idle_has_issue) {
                lv_snprintf(detail_line, sizeof(detail_line),
                    ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RESULT_ISSUE_DETAIL_FMT),
                    current_suspect,
                    current_damaged);
                lv_snprintf(g_smart_island_info_extra_text, sizeof(g_smart_island_info_extra_text), "%s", detail_line);
            } else {
                lv_snprintf(g_smart_island_info_extra_text, sizeof(g_smart_island_info_extra_text), "%s",
                    ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RESULT_OK_DETAIL));
            }
        }

        if (current_total > 0) {
            g_smart_island_idle_quality_percent = (uint8_t)((current_valid * 100) / current_total);
        } else {
            g_smart_island_idle_quality_percent = 0;
        }
        break;
    }
    }

    smart_island_apply_texts();
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

    smart_island_update_time_clickable();
    smart_island_apply_quality_indicator();
}

static void smart_island_update_time_clickable(void)
{
    bool clickable;

    if (g_smart_island_time == NULL || !lv_obj_is_valid(g_smart_island_time)) {
        return;
    }

    clickable = (g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED &&
                 g_smart_island_page == SMART_ISLAND_PAGE_INFO);

    if (clickable) {
        lv_obj_add_flag(g_smart_island_time, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(g_smart_island_time, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    } else {
        lv_obj_clear_flag(g_smart_island_time, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(g_smart_island_time, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    }
}

static void smart_island_apply_scene_style(void) 
{
    uint32_t bg_hex = SMART_ISLAND_BG_IDLE;
    lv_color_t title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
    lv_color_t dot_color = lv_color_hex(SMART_ISLAND_READY_DOT);
    bool show_time = true;
    bool show_dot = true;
    
    if (g_smart_island == NULL || !lv_obj_is_valid(g_smart_island)) return;

    switch (g_smart_island_scene) {
    case SMART_ISLAND_SCENE_IDLE:
    case SMART_ISLAND_SCENE_QR:
        bg_hex = SMART_ISLAND_BG_IDLE;
        title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
        dot_color = lv_color_hex(SMART_ISLAND_READY_DOT);
        show_time = (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE);
        show_dot = true;
        smart_island_pulse_stop();
        break;

    case SMART_ISLAND_SCENE_RESULT:
        bg_hex = SMART_ISLAND_BG_SUCCESS;
        title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
        dot_color = lv_color_hex(SMART_ISLAND_DOT_NON_IDLE);
        show_time = false;
        show_dot = true;
        smart_island_pulse_stop();
        break;

    case SMART_ISLAND_SCENE_COUNTING:
        bg_hex = SMART_ISLAND_BG_COUNTING;
        title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
        dot_color = lv_color_hex(SMART_ISLAND_DOT_NON_IDLE);
        show_time = false;
        show_dot = true;
        smart_island_pulse_stop();
        break;
    case SMART_ISLAND_SCENE_WARNING:
        bg_hex =
            (g_smart_island_warning_level == SMART_ISLAND_WARNING_LEVEL_ERROR)
            ? SMART_ISLAND_BG_ERROR
            : SMART_ISLAND_BG_WARNING;
        title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
        dot_color = lv_color_hex(SMART_ISLAND_DOT_NON_IDLE);
        show_time = false;
        show_dot = true;
        smart_island_pulse_stop();
        break;

    case SMART_ISLAND_SCENE_UPDATE:
        bg_hex = SMART_ISLAND_BG_UPDATE;
        title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
        dot_color = lv_color_hex(SMART_ISLAND_DOT_NON_IDLE);
        show_time = false;
        show_dot = true;
        smart_island_pulse_stop();
        break;
    default: break;
    }

    /* 功能页仍不显示时间，头部通过滑动动画进出 */
    if (g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED &&
        g_smart_island_page == SMART_ISLAND_PAGE_ACTION) {
        show_time = false;
    }

    smart_island_bg_color_apply_anim(bg_hex);
    lv_obj_set_style_border_width(g_smart_island, 0, 0);
    lv_obj_set_style_outline_width(g_smart_island, 0, 0);
    lv_obj_set_style_shadow_width(g_smart_island, 0, 0);

    if (g_smart_island_title && lv_obj_is_valid(g_smart_island_title)) {
        lv_obj_set_style_text_color(g_smart_island_title, title_color, 0);
        lv_obj_set_style_text_opa(g_smart_island_title, LV_OPA_COVER, 0);
        lv_obj_clear_flag(g_smart_island_title, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (g_smart_island_subtitle && lv_obj_is_valid(g_smart_island_subtitle)) {
        lv_obj_add_flag(g_smart_island_subtitle, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        lv_obj_set_style_text_opa(g_smart_island_expand_title, LV_OPA_COVER, 0);
        lv_obj_clear_flag(g_smart_island_expand_title, LV_OBJ_FLAG_HIDDEN); 
    }
    if (g_smart_island_expand_subtitle && lv_obj_is_valid(g_smart_island_expand_subtitle)) {
        lv_obj_set_style_text_opa(g_smart_island_expand_subtitle, LV_OPA_COVER, 0);
        lv_obj_clear_flag(g_smart_island_expand_subtitle, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_smart_island_expand_last && lv_obj_is_valid(g_smart_island_expand_last)) {
        if (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE) {
            lv_obj_clear_flag(g_smart_island_expand_last, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_smart_island_expand_last, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (g_smart_island_expand_divider && lv_obj_is_valid(g_smart_island_expand_divider)) {
        if (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE) {
            lv_obj_clear_flag(g_smart_island_expand_divider, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_smart_island_expand_divider, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (g_smart_island_expand_footer && lv_obj_is_valid(g_smart_island_expand_footer)) {
        if (g_smart_island_info_footer_text[0] != '\0') lv_obj_clear_flag(g_smart_island_expand_footer, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_smart_island_expand_footer, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_smart_island_expand_extra && lv_obj_is_valid(g_smart_island_expand_extra)) {
        if (g_smart_island_info_extra_text[0] != '\0') lv_obj_clear_flag(g_smart_island_expand_extra, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_smart_island_expand_extra, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_recolor(g_smart_island_expand_extra, true);
    }
    if (g_smart_island_expand_subtitle && lv_obj_is_valid(g_smart_island_expand_subtitle)) {
        lv_label_set_recolor(g_smart_island_expand_subtitle, true);
    }

    if (g_smart_island_expand_subtitle && lv_obj_is_valid(g_smart_island_expand_subtitle)) {
        if (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE) {
            lv_obj_set_style_text_color(g_smart_island_expand_subtitle, lv_color_hex(SMART_ISLAND_TEXT_LIGHT), 0);
            lv_obj_set_style_text_align(g_smart_island_expand_subtitle, LV_TEXT_ALIGN_RIGHT, 0);
            lv_obj_set_width(g_smart_island_expand_subtitle, 173);
            lv_obj_set_pos(g_smart_island_expand_subtitle, 68, 36);
        } else {
            lv_obj_set_style_text_color(g_smart_island_expand_subtitle, title_color, 0);
            lv_obj_set_style_text_align(g_smart_island_expand_subtitle, LV_TEXT_ALIGN_LEFT, 0);
            lv_obj_set_width(g_smart_island_expand_subtitle, SMART_ISLAND_W - 20);
            lv_obj_set_pos(g_smart_island_expand_subtitle, 20, 42);
        }
    }

    if (g_smart_island_expand_last && lv_obj_is_valid(g_smart_island_expand_last)) {
        lv_obj_set_style_text_color(g_smart_island_expand_last, lv_color_hex(SMART_ISLAND_RESULT_DETAIL_GRAY), 0);
    }

    if (g_smart_island_expand_divider && lv_obj_is_valid(g_smart_island_expand_divider)) {
        lv_obj_set_style_bg_color(g_smart_island_expand_divider, lv_color_hex(0x515151), 0);
        lv_obj_set_style_bg_opa(g_smart_island_expand_divider, LV_OPA_COVER, 0);
    }

    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        lv_obj_set_pos(g_smart_island_expand_title, 20, 18);
    }

    if (g_smart_island_expand_footer && lv_obj_is_valid(g_smart_island_expand_footer)) {
        lv_obj_set_pos(g_smart_island_expand_footer, 20, 62);
    }

    if (g_smart_island_expand_extra && lv_obj_is_valid(g_smart_island_expand_extra)) {
        lv_obj_set_pos(g_smart_island_expand_extra, 20, 80);
    }

    if (g_smart_island_quality_bar_bg && lv_obj_is_valid(g_smart_island_quality_bar_bg)) {
        lv_obj_set_pos(g_smart_island_quality_bar_bg, 166, 64);
    }
    if (g_smart_island_quality_percent && lv_obj_is_valid(g_smart_island_quality_percent)) {
        lv_obj_set_pos(g_smart_island_quality_percent, 228, 62);
    }
    if (g_smart_island_expand_last && lv_obj_is_valid(g_smart_island_expand_last)) {
        lv_obj_set_pos(g_smart_island_expand_last, 20, 36);
    }
    if (g_smart_island_expand_divider && lv_obj_is_valid(g_smart_island_expand_divider)) {
        lv_obj_set_pos(g_smart_island_expand_divider, 20, 56);
    }

    if (g_smart_island_expand_title && lv_obj_is_valid(g_smart_island_expand_title)) {
        if (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE) {
            lv_obj_add_flag(g_smart_island_expand_title, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(g_smart_island_expand_title, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_smart_island_expand_footer && lv_obj_is_valid(g_smart_island_expand_footer)) {
        if (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE) {
            lv_color_t footer_color;
            if (g_smart_island_idle_no_count) {
                footer_color = lv_color_hex(SMART_ISLAND_TEXT_SUB);
            } else if (g_smart_island_idle_has_issue) {
                footer_color = lv_color_hex(SMART_ISLAND_RESULT_ISSUE_TITLE_COLOR);
            } else {
                footer_color = lv_color_hex(SMART_ISLAND_RESULT_OK_COLOR);
            }
            lv_obj_set_style_text_color(g_smart_island_expand_footer, footer_color, 0);
        } else {
            lv_obj_set_style_text_color(g_smart_island_expand_footer, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);
        }
    }

    if (g_smart_island_expand_extra && lv_obj_is_valid(g_smart_island_expand_extra)) {
        if (g_smart_island_scene == SMART_ISLAND_SCENE_IDLE) {
            lv_color_t extra_color = lv_color_hex(g_smart_island_idle_has_issue
                ? SMART_ISLAND_RESULT_ISSUE_COLOR
                : (g_smart_island_idle_no_count ? SMART_ISLAND_LAST_TEXT_GRAY : SMART_ISLAND_RESULT_DETAIL_GRAY));
            lv_obj_set_style_text_color(g_smart_island_expand_extra, extra_color, 0);
        } else {
            lv_obj_set_style_text_color(g_smart_island_expand_extra, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);
        }
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

    smart_island_apply_quality_indicator();
    smart_island_update_pages_visible();
    smart_island_modal_update();
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
        lv_anim_del(g_smart_island_title, smart_island_anim_translate_x_cb);
        lv_obj_clear_flag(g_smart_island_title, LV_OBJ_FLAG_HIDDEN);
        if (new_page == SMART_ISLAND_PAGE_ACTION) {
            lv_obj_set_style_translate_x(g_smart_island_title, 0, 0);
            lv_anim_init(&a);
            lv_anim_set_var(&a, g_smart_island_title);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, 0, delta_x);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        } else {
            lv_obj_set_style_translate_x(g_smart_island_title, -delta_x, 0);
            lv_anim_init(&a);
            lv_anim_set_var(&a, g_smart_island_title);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, -delta_x, 0);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        }
    }

    if (g_smart_island_dot && lv_obj_is_valid(g_smart_island_dot)) {
        lv_anim_del(g_smart_island_dot, smart_island_anim_translate_x_cb);
        lv_obj_clear_flag(g_smart_island_dot, LV_OBJ_FLAG_HIDDEN);
        if (new_page == SMART_ISLAND_PAGE_ACTION) {
            lv_obj_set_style_translate_x(g_smart_island_dot, 0, 0);
            lv_anim_init(&a);
            lv_anim_set_var(&a, g_smart_island_dot);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, 0, delta_x);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        } else {
            lv_obj_set_style_translate_x(g_smart_island_dot, -delta_x, 0);
            lv_anim_init(&a);
            lv_anim_set_var(&a, g_smart_island_dot);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, -delta_x, 0);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        }
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
static void smart_island_action_btn_style_apply(uint8_t index)
{
    lv_obj_t *btn = NULL;
    lv_obj_t *arrow = NULL;
    bool is_switch;
    bool enabled = false;

    if (index >= SMART_ISLAND_ACTION_PAGE_COUNT) {
        return;
    }

    btn = g_smart_island_action_btns[index];
    arrow = g_smart_island_action_arrows[index];
    if (btn == NULL || !lv_obj_is_valid(btn)) {
        return;
    }

    is_switch = (g_smart_island_action_ids[index] == SMART_ISLAND_ACTION_FUNC3 ||
        g_smart_island_action_ids[index] == SMART_ISLAND_ACTION_FUNC4);

    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(SMART_ISLAND_BTN_BORDER), 0);
    lv_obj_set_style_shadow_width(btn, 8, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
    lv_obj_set_style_shadow_color(btn, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_ofs_y(btn, 2, 0);

    if (!is_switch) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(SMART_ISLAND_BTN_BG_TOP), 0);
        lv_obj_set_style_bg_grad_color(btn, lv_color_hex(SMART_ISLAND_BTN_BG_BOT), 0);
        lv_obj_set_style_bg_grad_dir(btn, LV_GRAD_DIR_VER, 0);

        if (arrow && lv_obj_is_valid(arrow)) {
            lv_label_set_text(arrow, ">");
            lv_obj_set_style_text_color(arrow, lv_color_hex(SMART_ISLAND_BTN_ARROW), 0);
            lv_obj_set_style_text_font(arrow, &lv_font_montserrat_18, 0);
            lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -16, 0);
        }
        return;
    }

    if (g_smart_island_action_ids[index] == SMART_ISLAND_ACTION_FUNC4) {
        enabled = smart_island_pure_count_is_enabled();
    } else {
        enabled = fault_popup_get_auto_enabled();
    }
    lv_obj_set_style_bg_color(btn, lv_color_hex(enabled ? SMART_ISLAND_BTN_SWITCH_ON_TOP : SMART_ISLAND_BTN_SWITCH_OFF_TOP), 0);
    lv_obj_set_style_bg_grad_color(btn, lv_color_hex(enabled ? SMART_ISLAND_BTN_SWITCH_ON_BOT : SMART_ISLAND_BTN_SWITCH_OFF_BOT), 0);
    lv_obj_set_style_bg_grad_dir(btn, LV_GRAD_DIR_VER, 0);

    if (arrow && lv_obj_is_valid(arrow)) {
        lv_label_set_text(arrow, enabled ? "ON" : "OFF");
        lv_obj_set_style_text_color(arrow,
            lv_color_hex(enabled ? SMART_ISLAND_BTN_SWITCH_ON_TEXT : SMART_ISLAND_BTN_SWITCH_OFF_TEXT), 0);
        lv_obj_set_style_text_font(arrow, &lv_font_montserrat_12, 0);
        lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -12, 0);
    }
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
        if (g_smart_island_action_ids[index] == SMART_ISLAND_ACTION_FUNC3) {
            lv_label_set_text(label, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC3));
        } else if (g_smart_island_action_ids[index] == SMART_ISLAND_ACTION_FUNC4) {
            lv_label_set_text(label, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC4));
        } else if (g_smart_island_action_text_ids[index] < UI_TEXT_MAX) {
            lv_label_set_text(label, ui_text_get(g_smart_island_action_text_ids[index]));
        } else if (g_smart_island_action_texts[index][0] != '\0') {
            lv_label_set_text(label, g_smart_island_action_texts[index]);
        } else {
            lv_label_set_text(label, "");
        }
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 16, 0); 
    }

    smart_island_action_btn_style_apply(index);
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
    lv_anim_set_ready_cb(&a, smart_island_action_page_slide_anim_finish_cb);
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
        SMART_ISLAND_ACTION_FUNC4, SMART_ISLAND_ACTION_QR,
        SMART_ISLAND_ACTION_FUNC3, SMART_ISLAND_ACTION_TIME_SETTING
    };
    static const ui_text_id_t default_text_ids[SMART_ISLAND_ACTION_PAGE_COUNT] = {
        UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC4, UI_TEXT_WIDGET_SMART_ISLAND_ACTION_QR,
        UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC3, UI_TEXT_WIDGET_SMART_ISLAND_ACTION_TIME
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
        lv_obj_set_style_transform_zoom(btn, 256, 0);
        lv_obj_set_style_translate_y(btn, 0, 0);

        lv_obj_add_event_cb(btn, smart_island_action_btn_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        lv_obj_add_event_cb(btn, smart_island_action_btn_touch_anim_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(btn, smart_island_action_btn_touch_anim_cb, LV_EVENT_RELEASED, NULL);
        lv_obj_add_event_cb(btn, smart_island_action_btn_touch_anim_cb, LV_EVENT_PRESS_LOST, NULL);
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
    if (parent == NULL) return;

    if (g_smart_island_created && g_smart_island && lv_obj_is_valid(g_smart_island)) {
        lv_obj_t *cur_parent = lv_obj_get_parent(g_smart_island);
        if (cur_parent != parent) {
            if (g_smart_island_modal && lv_obj_is_valid(g_smart_island_modal)) {
                lv_obj_set_parent(g_smart_island_modal, parent);
                lv_obj_set_pos(g_smart_island_modal, 0, 0);
            }
            lv_obj_set_parent(g_smart_island, parent);
            lv_obj_move_foreground(g_smart_island);
            smart_island_modal_update();
        }
        return;
    }

    memset(&g_smart_island_content, 0, sizeof(g_smart_island_content));
    memset(g_smart_island_warning_text, 0, sizeof(g_smart_island_warning_text));
    memset(g_smart_island_result_text, 0, sizeof(g_smart_island_result_text));
    memset(g_smart_island_info_extra_text, 0, sizeof(g_smart_island_info_extra_text));
    memset(g_smart_island_idle_custom_line1, 0, sizeof(g_smart_island_idle_custom_line1));
    memset(g_smart_island_idle_custom_line2, 0, sizeof(g_smart_island_idle_custom_line2));
    memset(g_smart_island_idle_custom_line3, 0, sizeof(g_smart_island_idle_custom_line3));

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
    lv_obj_set_style_border_width(g_smart_island, 0, 0);
    lv_obj_set_style_outline_width(g_smart_island, 0, 0);
    lv_obj_set_style_shadow_width(g_smart_island, 0, 0);
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
    lv_obj_set_width(g_smart_island_title, SMART_ISLAND_W - 36 - 14);
    lv_obj_set_pos(g_smart_island_title, 36, 13);
    lv_obj_set_style_text_font(g_smart_island_title, &lv_font_montserrat_14, 0);

    g_smart_island_subtitle = lv_label_create(g_smart_island);
    lv_obj_add_flag(g_smart_island_subtitle, LV_OBJ_FLAG_HIDDEN); 

    g_smart_island_time = lv_label_create(g_smart_island);
    lv_label_set_text(g_smart_island_time, "00:00:00");
    lv_obj_set_style_text_font(g_smart_island_time, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(g_smart_island_time, lv_color_hex(SMART_ISLAND_TEXT_LIGHT), 0);
    lv_obj_align(g_smart_island_time, LV_ALIGN_RIGHT_MID, -14, 0);
    lv_obj_add_event_cb(g_smart_island_time, smart_island_time_click_cb, LV_EVENT_CLICKED, NULL);
    smart_island_update_time_clickable();

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
    lv_label_set_long_mode(g_smart_island_expand_title, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_smart_island_expand_title, SMART_ISLAND_W - 24 - 12);
    lv_obj_set_pos(g_smart_island_expand_title, 20, 18);
    lv_obj_set_style_text_font(g_smart_island_expand_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(g_smart_island_expand_title, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);
    lv_obj_add_flag(g_smart_island_expand_title, LV_OBJ_FLAG_HIDDEN);

    g_smart_island_expand_subtitle = lv_label_create(g_smart_island_page_info);
    lv_label_set_text(g_smart_island_expand_subtitle, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_EXPAND_SUBTITLE));
    lv_label_set_long_mode(g_smart_island_expand_subtitle, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_smart_island_expand_subtitle, 173);
    lv_obj_set_pos(g_smart_island_expand_subtitle, 68, 36);
    lv_obj_set_style_text_font(g_smart_island_expand_subtitle, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(g_smart_island_expand_subtitle, lv_color_hex(SMART_ISLAND_TEXT_LIGHT), 0);
    lv_obj_set_style_text_align(g_smart_island_expand_subtitle, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_recolor(g_smart_island_expand_subtitle, true);

    g_smart_island_expand_last = lv_label_create(g_smart_island_page_info);
    lv_label_set_text(g_smart_island_expand_last, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_LAST_TAG));
    lv_label_set_long_mode(g_smart_island_expand_last, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_smart_island_expand_last, 40);
    lv_obj_set_pos(g_smart_island_expand_last, 20, 36);
    lv_obj_set_style_text_align(g_smart_island_expand_last, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_font(g_smart_island_expand_last, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(g_smart_island_expand_last, lv_color_hex(SMART_ISLAND_LAST_TEXT_GRAY), 0);

    g_smart_island_expand_divider = lv_obj_create(g_smart_island_page_info);
    lv_obj_remove_style_all(g_smart_island_expand_divider);
    lv_obj_set_size(g_smart_island_expand_divider, SMART_ISLAND_W - 40, 1);
    lv_obj_set_pos(g_smart_island_expand_divider, 20, 56);
    lv_obj_set_style_bg_opa(g_smart_island_expand_divider, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_smart_island_expand_divider, lv_color_hex(0x515151), 0);

    g_smart_island_expand_footer = lv_label_create(g_smart_island_page_info);
    lv_label_set_text(g_smart_island_expand_footer, "");
    lv_label_set_long_mode(g_smart_island_expand_footer, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_smart_island_expand_footer, 150);
    lv_obj_set_pos(g_smart_island_expand_footer, 20, 62);
    lv_obj_set_style_text_font(g_smart_island_expand_footer, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(g_smart_island_expand_footer, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);
    lv_obj_add_flag(g_smart_island_expand_footer, LV_OBJ_FLAG_HIDDEN);

    g_smart_island_expand_extra = lv_label_create(g_smart_island_page_info);
    lv_label_set_text(g_smart_island_expand_extra, "");
    lv_label_set_long_mode(g_smart_island_expand_extra, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_smart_island_expand_extra, 150);
    lv_obj_set_pos(g_smart_island_expand_extra, 20, 80);
    lv_obj_set_style_text_font(g_smart_island_expand_extra, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(g_smart_island_expand_extra, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);
    lv_obj_add_flag(g_smart_island_expand_extra, LV_OBJ_FLAG_HIDDEN);

    g_smart_island_quality_bar_bg = lv_obj_create(g_smart_island_page_info);
    lv_obj_remove_style_all(g_smart_island_quality_bar_bg);
    lv_obj_set_size(g_smart_island_quality_bar_bg, 56, 8);
    lv_obj_set_pos(g_smart_island_quality_bar_bg, 166, 64);
    lv_obj_set_style_radius(g_smart_island_quality_bar_bg, 4, 0);
    lv_obj_set_style_bg_opa(g_smart_island_quality_bar_bg, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_smart_island_quality_bar_bg, lv_color_hex(SMART_ISLAND_RESULT_ISSUE_COLOR), 0);
    lv_obj_add_flag(g_smart_island_quality_bar_bg, LV_OBJ_FLAG_HIDDEN);

    g_smart_island_quality_bar_fg = lv_obj_create(g_smart_island_quality_bar_bg);
    lv_obj_remove_style_all(g_smart_island_quality_bar_fg);
    lv_obj_set_size(g_smart_island_quality_bar_fg, 56, 8);
    lv_obj_set_pos(g_smart_island_quality_bar_fg, 0, 0);
    lv_obj_set_style_radius(g_smart_island_quality_bar_fg, 4, 0);
    lv_obj_set_style_bg_opa(g_smart_island_quality_bar_fg, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_smart_island_quality_bar_fg, lv_color_hex(SMART_ISLAND_RESULT_OK_COLOR), 0);

    g_smart_island_quality_percent = lv_label_create(g_smart_island_page_info);
    lv_label_set_text(g_smart_island_quality_percent, "100%");
    lv_obj_set_pos(g_smart_island_quality_percent, 228, 62);
    lv_obj_set_style_text_font(g_smart_island_quality_percent, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(g_smart_island_quality_percent, lv_color_hex(SMART_ISLAND_RESULT_OK_COLOR), 0);
    lv_obj_add_flag(g_smart_island_quality_percent, LV_OBJ_FLAG_HIDDEN);

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
    g_smart_island_idle_quality_percent = 100;
    g_smart_island_idle_has_issue = false;
    g_smart_island_idle_has_data = false;
    g_smart_island_idle_no_count = true;
    g_smart_island_count_session_active = false;
    g_smart_island_reject_base_expected = 0;
    g_smart_island_reject_base_detail = 0;
    g_smart_island_bg_cur = SMART_ISLAND_BG_IDLE;
    g_smart_island_bg_from = SMART_ISLAND_BG_IDLE;
    g_smart_island_bg_to = SMART_ISLAND_BG_IDLE;
    g_smart_island_bg_anim_running = false;
    g_smart_island_created = true;
    smart_island_page_indicator_sync(false);
    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
    smart_island_update_idle_time();
    smart_island_update_pages_visible();
    smart_island_modal_update();
}

void smart_island_destroy(void) 
{
    smart_island_stop_result_timer();
    smart_island_stop_warning_timer();
    smart_island_warning_marquee_stop();
    smart_island_pulse_stop();
    if (g_smart_island && lv_obj_is_valid(g_smart_island)) lv_obj_del(g_smart_island);
    if (g_smart_island_modal && lv_obj_is_valid(g_smart_island_modal)) lv_obj_del(g_smart_island_modal);
    g_smart_island = NULL;
    g_smart_island_modal = NULL;
    g_smart_island_page_root = NULL;
    g_smart_island_page_info = NULL;
    g_smart_island_page_action = NULL;
    g_smart_island_expand_title = NULL;
    g_smart_island_expand_subtitle = NULL;
    g_smart_island_expand_last = NULL;
    g_smart_island_expand_divider = NULL;
    g_smart_island_expand_footer = NULL;
    g_smart_island_expand_extra = NULL;
    g_smart_island_quality_bar_bg = NULL;
    g_smart_island_quality_bar_fg = NULL;
    g_smart_island_quality_percent = NULL;
    g_smart_island_action_track = NULL;
    g_smart_island_page_indicator = NULL;
    g_smart_island_page_slide_dir = 0;
    g_smart_island_anim_running = false;
    g_smart_island_ignore_click_once = false;
    g_smart_island_ignore_action_click_once = false;
    g_smart_island_warning_marquee_running = false;
    g_smart_island_warning_marquee_step = 0;
    g_smart_island_warning_text_w_compact = 0;
    g_smart_island_warning_text_w_expand = 0;
    g_smart_island_swipe.pressed = false;
    g_smart_island_swipe.swiped = false;
    g_smart_island_swipe.start_pt.x = 0;
    g_smart_island_swipe.start_pt.y = 0;
    g_smart_island_bg_cur = SMART_ISLAND_BG_IDLE;
    g_smart_island_bg_from = SMART_ISLAND_BG_IDLE;
    g_smart_island_bg_to = SMART_ISLAND_BG_IDLE;
    g_smart_island_bg_anim_running = false;
    g_smart_island_idle_custom_line1[0] = '\0';
    g_smart_island_idle_custom_line2[0] = '\0';
    g_smart_island_idle_custom_line3[0] = '\0';
    g_smart_island_info_extra_text[0] = '\0';
    g_smart_island_idle_quality_percent = 100;
    g_smart_island_idle_has_issue = false;
    g_smart_island_idle_has_data = false;
    g_smart_island_idle_no_count = true;
    g_smart_island_count_session_active = false;
    g_smart_island_reject_base_expected = 0;
    g_smart_island_reject_base_detail = 0;

    g_smart_island_created = false;
}

bool smart_island_is_attached_to(lv_obj_t *parent)
{
    if (parent == NULL || !lv_obj_is_valid(parent)) {
        return false;
    }

    if (g_smart_island == NULL || !lv_obj_is_valid(g_smart_island)) {
        return false;
    }

    return lv_obj_get_parent(g_smart_island) == parent;
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

    if (g_smart_island_scene == SMART_ISLAND_SCENE_RESULT && visual == SMART_ISLAND_VISUAL_EXPANDED) {
        visual = SMART_ISLAND_VISUAL_COMPACT;
    }

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

    if (title && title[0] != '\0') lv_snprintf(g_smart_island_content.title, sizeof(g_smart_island_content.title), "%s", title);
    else g_smart_island_content.title[0] = '\0';

    if (subtitle && subtitle[0] != '\0') lv_snprintf(g_smart_island_content.subtitle, sizeof(g_smart_island_content.subtitle), "%s", subtitle);
    else g_smart_island_content.subtitle[0] = '\0';

    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_notify_count_start(void)
{
    if (!g_smart_island_count_session_active) {
        g_smart_island_reject_base_expected = sim.err_expected;
        g_smart_island_reject_base_detail = smart_island_get_reject_detail_total();
        g_smart_island_count_session_active = true;
    }
    g_smart_island_warning_level = SMART_ISLAND_WARNING_LEVEL_WARNING;
    smart_island_warning_marquee_stop();
    g_smart_island_result_text[0] = '\0';
    smart_island_set_scene(SMART_ISLAND_SCENE_COUNTING, NULL, NULL);
    smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, true);
}

void smart_island_notify_count_end(const char *result_text)
{
    g_smart_island_count_session_active = false;
    if (result_text && result_text[0] != '\0') {
        lv_snprintf(g_smart_island_result_text, sizeof(g_smart_island_result_text), "%s", result_text);
    } else {
        g_smart_island_result_text[0] = '\0';
    }
    smart_island_set_scene(SMART_ISLAND_SCENE_RESULT, NULL, NULL);
    smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, true);
    smart_island_stop_result_timer();
    g_smart_island_result_timer = lv_timer_create(smart_island_result_timer_cb, SMART_ISLAND_RESULT_HOLD_MS, NULL);
}

void smart_island_notify_warning_level(const char *warn_text, smart_island_warning_level_t level)
{
    char next_warning_text[sizeof(g_smart_island_warning_text)];

    if (warn_text && warn_text[0] != '\0') {
        lv_snprintf(next_warning_text, sizeof(next_warning_text), "%s", warn_text);
    } else {
        lv_snprintf(next_warning_text, sizeof(next_warning_text), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR));
    }

    /* 同一条异常重复上报时，不重启 warning 动画，避免 ESC/CLEAR/START 触发“重新闪烁” */
    if (g_smart_island_scene == SMART_ISLAND_SCENE_WARNING &&
        g_smart_island_warning_level == level &&
        strcmp(g_smart_island_warning_text, next_warning_text) == 0) {
        if (!g_smart_island_warning_marquee_running) {
            smart_island_warning_apply_static_layout();
            if (!fault_popup_is_showing()) {
                smart_island_warning_marquee_start();
            }
        }
        return;
    }

    g_smart_island_warning_level = level;

    lv_snprintf(g_smart_island_warning_text, sizeof(g_smart_island_warning_text), "%s",
        next_warning_text);

    smart_island_stop_warning_timer();

    smart_island_set_scene(
        SMART_ISLAND_SCENE_WARNING,
        g_smart_island_warning_text,
        NULL
    );

    g_smart_island_page = SMART_ISLAND_PAGE_INFO;
    smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, true);
    smart_island_reset_page_positions();
    smart_island_reset_compact_header_position();
    smart_island_reset_time_position();

    smart_island_warning_marquee_start();
}

void smart_island_notify_warning(const char *warn_text)
{
    smart_island_notify_warning_level(warn_text, SMART_ISLAND_WARNING_LEVEL_WARNING);
}
void smart_island_notify_update(uint16_t progress, const char *text) 
{
    g_smart_island_content.progress = progress;
    smart_island_set_scene(SMART_ISLAND_SCENE_UPDATE,
        text,
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
        text,
        ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_QR_INFO_SUBTITLE));
    if (g_smart_island_progress && lv_obj_is_valid(g_smart_island_progress)) lv_obj_add_flag(g_smart_island_progress, LV_OBJ_FLAG_HIDDEN);
    smart_island_open_info_page();
}

void smart_island_restore_idle(void) 
{
    g_smart_island_count_session_active = false;
    smart_island_warning_marquee_stop();
    g_smart_island_warning_level = SMART_ISLAND_WARNING_LEVEL_WARNING;
    if (g_smart_island_progress && lv_obj_is_valid(g_smart_island_progress)) {
        lv_obj_add_flag(g_smart_island_progress, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_value(g_smart_island_progress, 0, LV_ANIM_OFF);
    }
    g_smart_island_warning_text[0] = '\0';
    g_smart_island_result_text[0] = '\0';
    smart_island_set_scene(SMART_ISLAND_SCENE_IDLE, NULL, NULL);
    smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, true);
    smart_island_update_idle_time();
    smart_island_reset_compact_header_position();
    smart_island_reset_time_position();
}

bool smart_island_is_expanded(void) { return g_smart_island_visual == SMART_ISLAND_VISUAL_EXPANDED; }

void smart_island_set_page(smart_island_page_t page, bool anim_en) 
{
    if (g_smart_island_scene == SMART_ISLAND_SCENE_RESULT) {
        g_smart_island_page = SMART_ISLAND_PAGE_INFO;
        smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, false);
        return;
    }

    if (anim_en) smart_island_page_apply_anim(page);
    else smart_island_page_apply_now(page);
    smart_island_page_indicator_sync(anim_en);
}

smart_island_page_t smart_island_get_page(void) { return g_smart_island_page; }

void smart_island_register_action_cb(smart_island_action_cb_t cb) { g_smart_island_action_cb = cb; }

void smart_island_refresh_language_texts(void)
{
    smart_island_action_page_refresh_language_texts();
    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_refresh_summary(void)
{
    /* Warning 场景中避免外部刷新重刷样式，防止 ESC/CLEAR 造成文本“重新闪烁” */
    if (g_smart_island_scene == SMART_ISLAND_SCENE_WARNING) {
        return;
    }

    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_set_idle_info_line1(const char *text)
{
    smart_island_apply_idle_line_text(g_smart_island_idle_custom_line1, sizeof(g_smart_island_idle_custom_line1), text);
    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_set_idle_info_line2(const char *text)
{
    smart_island_apply_idle_line_text(g_smart_island_idle_custom_line2, sizeof(g_smart_island_idle_custom_line2), text);
    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_set_idle_info_line3(const char *text)
{
    smart_island_apply_idle_line_text(g_smart_island_idle_custom_line3, sizeof(g_smart_island_idle_custom_line3), text);
    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
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
    if (g_smart_island_scene == SMART_ISLAND_SCENE_RESULT) {
        return;
    }

    smart_island_expand_if_needed(true);
    smart_island_set_page(SMART_ISLAND_PAGE_INFO, true);
}

void smart_island_open_action_page(void) 
{
    if (g_smart_island_scene == SMART_ISLAND_SCENE_RESULT) {
        return;
    }

    smart_island_action_page_set_index(0U, false);
    smart_island_expand_if_needed(true);
    smart_island_set_page(SMART_ISLAND_PAGE_ACTION, true);
}

void smart_island_set_pure_count_enabled(bool enabled)
{
    g_smart_island_pure_count_enabled = enabled;
}

bool smart_island_pure_count_is_enabled(void)
{
    return g_smart_island_pure_count_enabled;
}
