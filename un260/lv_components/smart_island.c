#include "smart_island.h"
#include "un260/lv_components/smart_island/smart_island_internal.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_components/lv_print_toast.h"
#include "un260/lv_components/lv_qr_popup.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/machine_time.h"
#include "un260/lv_system/ui_qr_data.h"
#include "un260/lv_system/ui_text.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/page_18_pure.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/machine_state/machine_state.h"
#include "un260/currency/currency_state.h"
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
#define SMART_ISLAND_ACTION_PAGE_COUNT    SMART_ISLAND_ACTION_PAGE_CAPACITY
#define SMART_ISLAND_ACTION_PAGE_DEFAULT_COUNT 3
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

/* 内部状态统一收口，后续子模块通过 internal.h 共享。 */
smart_island_context_t g_si_ctx = {
    .action.page_count = SMART_ISLAND_ACTION_PAGE_DEFAULT_COUNT,
    .view.scene = SMART_ISLAND_SCENE_IDLE,
    .view.visual = SMART_ISLAND_VISUAL_COMPACT,
    .view.page = SMART_ISLAND_PAGE_INFO,
    .view.bg_current = SMART_ISLAND_BG_IDLE,
    .view.bg_from = SMART_ISLAND_BG_IDLE,
    .view.bg_to = SMART_ISLAND_BG_IDLE,
    .warning.level = SMART_ISLAND_WARNING_LEVEL_WARNING,
    .warning.fault.source = FAULT_SRC_START_COUNT,
    .text.idle_quality_percent = 100,
};

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
static void smart_island_clear_object_refs(void);
static void smart_island_stop_result_timer(void); 
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
static void smart_island_warning_marquee_start(void);
static void smart_island_warning_marquee_stop(void);
static void smart_island_warning_apply_static_layout(void);
static void smart_island_warning_marquee_finish_cb(lv_anim_t *a);
static void smart_island_warning_marquee_run_step(void);
static void smart_island_warning_flash_finish_cb(lv_anim_t *a);
static void smart_island_bg_color_apply_anim(uint32_t dst_hex);
static void smart_island_bg_color_anim_ready_cb(lv_anim_t *a);
static void smart_island_apply_idle_line_text(char *dst, size_t dst_size, const char *text);
static void smart_island_apply_quality_indicator(void);
static void smart_island_action_btn_touch_anim_cb(lv_event_t *e);
static void smart_island_action_btn_set_pressed_visual(lv_obj_t *btn, bool pressed);
static void smart_island_action_btn_style_apply(uint8_t index);
static void smart_island_warning_fault_capture(void);
static bool smart_island_warning_fault_show(void);
static void smart_island_warning_fault_clear(void);
static bool smart_island_warning_pocket_confirm(void);

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
        if (g_si_ctx.objects.action_buttons[i] == btn) {
            label = g_si_ctx.objects.action_labels[i];
            arrow = g_si_ctx.objects.action_arrows[i];
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

static void smart_island_warning_fault_capture(void)
{
    fault_source_t source;
    uint8_t fault_type;
    uint8_t code;

    if (fault_popup_get_pending_fault(&source, &fault_type, &code)) {
        g_si_ctx.warning.fault.valid = true;
        g_si_ctx.warning.fault.source = source;
        g_si_ctx.warning.fault.fault_type = fault_type;
        g_si_ctx.warning.fault.code = code;
    } else {
        smart_island_warning_fault_clear();
    }
}

static bool smart_island_warning_fault_show(void)
{
    if (!g_si_ctx.warning.fault.valid) {
        return false;
    }

    switch (g_si_ctx.warning.fault.source) {
    case FAULT_SRC_START_COUNT:
        show_start_fault_popup(g_si_ctx.warning.fault.fault_type,
                               g_si_ctx.warning.fault.code);
        return true;
    case FAULT_SRC_RUNTIME:
        show_runtime_fault_popup(g_si_ctx.warning.fault.code);
        return true;
    case FAULT_SRC_BOOT:
    default:
        return false;
    }
}

static void smart_island_warning_fault_clear(void)
{
    g_si_ctx.warning.fault.valid = false;
    g_si_ctx.warning.fault.source = FAULT_SRC_START_COUNT;
    g_si_ctx.warning.fault.fault_type = 0;
    g_si_ctx.warning.fault.code = 0;
}

static bool smart_island_warning_pocket_confirm(void)
{
    uint8_t clear_cmd = 0x01;
    uint8_t code;

    if (!g_si_ctx.warning.fault.valid ||
        g_si_ctx.warning.fault.source != FAULT_SRC_START_COUNT) {
        return false;
    }

    code = g_si_ctx.warning.fault.code;
    if (code != 0x05 && code != 0x07 && code != 0x08) {
        return false;
    }

    send_command(fd4, 0x3D, &clear_cmd, 1);
    return true;
}

static void smart_island_anim_bg_color_cb(void *var, int32_t v)
{
    lv_color_t from_c;
    lv_color_t to_c;
    lv_color_t mix_c;

    if (var == NULL) {
        return;
    }

    from_c = lv_color_hex(g_si_ctx.view.bg_from);
    to_c = lv_color_hex(g_si_ctx.view.bg_to);
    mix_c = lv_color_mix(to_c, from_c, (lv_opa_t)v);
    lv_obj_set_style_bg_color((lv_obj_t *)var, mix_c, 0);
}
static void smart_island_anim_finish_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    g_si_ctx.view.anim_running = false;
}

static void smart_island_action_page_slide_anim_finish_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    g_si_ctx.view.anim_running = false;
    g_si_ctx.action.ignore_action_click_once = false;
}

static void smart_island_bg_color_anim_ready_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    g_si_ctx.view.bg_anim_running = false;
    g_si_ctx.view.bg_current = g_si_ctx.view.bg_to;
}

static void smart_island_bg_color_apply_anim(uint32_t dst_hex)
{
    lv_anim_t a;
    lv_color_t style_c;

    if (g_si_ctx.objects.root == NULL || !lv_obj_is_valid(g_si_ctx.objects.root)) {
        return;
    }

    if (g_si_ctx.view.bg_anim_running && g_si_ctx.view.bg_to == dst_hex) {
        return;
    }

    if (!g_si_ctx.view.bg_anim_running && g_si_ctx.view.bg_current == dst_hex) {
        lv_obj_set_style_bg_color(g_si_ctx.objects.root, lv_color_hex(dst_hex), 0);
        return;
    }

    style_c = lv_obj_get_style_bg_color(g_si_ctx.objects.root, LV_PART_MAIN);
    g_si_ctx.view.bg_from = lv_color_to32(style_c);
    g_si_ctx.view.bg_to = dst_hex;

    lv_anim_del(g_si_ctx.objects.root, smart_island_anim_bg_color_cb);
    lv_anim_init(&a);
    lv_anim_set_var(&a, g_si_ctx.objects.root);
    lv_anim_set_exec_cb(&a, smart_island_anim_bg_color_cb);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, SMART_ISLAND_COLOR_ANIM_TIME);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_ready_cb(&a, smart_island_bg_color_anim_ready_cb);
    lv_anim_start(&a);
    g_si_ctx.view.bg_anim_running = true;
}

static void smart_island_warning_marquee_stop(void)
{
    if (g_si_ctx.objects.title && lv_obj_is_valid(g_si_ctx.objects.title)) {
        lv_anim_del(g_si_ctx.objects.title, smart_island_anim_x_cb);
        lv_anim_del(g_si_ctx.objects.title, smart_island_anim_text_opa_cb);
        lv_obj_set_style_text_opa(g_si_ctx.objects.title, LV_OPA_COVER, 0);
    }
    if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title)) {
        lv_anim_del(g_si_ctx.objects.expand_title, smart_island_anim_x_cb);
        lv_anim_del(g_si_ctx.objects.expand_title, smart_island_anim_text_opa_cb);
        lv_obj_set_style_text_opa(g_si_ctx.objects.expand_title, LV_OPA_COVER, 0);
    }
    g_si_ctx.warning.marquee_running = false;
    g_si_ctx.warning.marquee_step = 0;
    g_si_ctx.warning.text_width_compact = 0;
    g_si_ctx.warning.text_width_expand = 0;
    smart_island_reset_compact_header_position();
    smart_island_warning_apply_static_layout();
}

static void smart_island_warning_apply_static_layout(void)
{
    lv_coord_t compact_visible = SMART_ISLAND_W - 36 - 14;
    lv_coord_t expand_visible = SMART_ISLAND_W - 32 - 12;

    if (g_si_ctx.view.scene != SMART_ISLAND_SCENE_WARNING) {
        return;
    }

    if (g_si_ctx.objects.title && lv_obj_is_valid(g_si_ctx.objects.title)) {
        lv_label_set_long_mode(g_si_ctx.objects.title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_si_ctx.objects.title, compact_visible);
        lv_obj_set_x(g_si_ctx.objects.title, 36);
        lv_obj_set_y(g_si_ctx.objects.title, 13);
    }

    if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title)) {
        lv_label_set_long_mode(g_si_ctx.objects.expand_title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_si_ctx.objects.expand_title, expand_visible);
        lv_obj_set_x(g_si_ctx.objects.expand_title, 20);
        lv_obj_set_y(g_si_ctx.objects.expand_title, 18);
    }
}

static void smart_island_warning_marquee_finish_cb(lv_anim_t *a)
{
    LV_UNUSED(a);
    if (!g_si_ctx.warning.marquee_running) return;
    g_si_ctx.warning.marquee_step++;
    smart_island_warning_marquee_run_step();
}

static void smart_island_warning_flash_finish_cb(lv_anim_t *a)
{
    bool pocket_confirmed;

    LV_UNUSED(a);
    smart_island_warning_marquee_stop();
    pocket_confirmed = smart_island_warning_pocket_confirm();
    if (!fault_popup_is_showing()) {
        smart_island_restore_idle();
        if (!pocket_confirmed) {
            fault_popup_schedule_auto_confirm();
        }
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

    if (g_si_ctx.objects.title == NULL || !lv_obj_is_valid(g_si_ctx.objects.title)) {
        return;
    }

    if (g_si_ctx.view.scene != SMART_ISLAND_SCENE_WARNING) {
        return;
    }

    smart_island_warning_pocket_confirm();
    smart_island_warning_marquee_stop();
    title_text = lv_label_get_text(g_si_ctx.objects.title);
    title_font = lv_obj_get_style_text_font(g_si_ctx.objects.title, LV_PART_MAIN);
    text_w = (lv_coord_t)lv_txt_get_width(title_text ? title_text : "",
                                          (uint32_t)strlen(title_text ? title_text : ""),
                                          title_font,
                                          lv_obj_get_style_text_letter_space(g_si_ctx.objects.title, LV_PART_MAIN),
                                          LV_TEXT_FLAG_NONE);
    g_si_ctx.warning.text_width_compact = text_w;

    if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title)) {
        expand_text = lv_label_get_text(g_si_ctx.objects.expand_title);
        expand_font = lv_obj_get_style_text_font(g_si_ctx.objects.expand_title, LV_PART_MAIN);
        expand_text_w = (lv_coord_t)lv_txt_get_width(expand_text ? expand_text : "",
                                                     (uint32_t)strlen(expand_text ? expand_text : ""),
                                                     expand_font,
                                                     lv_obj_get_style_text_letter_space(g_si_ctx.objects.expand_title, LV_PART_MAIN),
                                                     LV_TEXT_FLAG_NONE);
        g_si_ctx.warning.text_width_expand = expand_text_w;
    }

    if (g_si_ctx.warning.text_width_compact <= compact_visible) {
        lv_coord_t compact_center_x = (SMART_ISLAND_W - g_si_ctx.warning.text_width_compact) / 2;
        if (compact_center_x < 0) compact_center_x = 0;

        lv_label_set_long_mode(g_si_ctx.objects.title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_si_ctx.objects.title, LV_SIZE_CONTENT);
        lv_obj_set_x(g_si_ctx.objects.title, compact_center_x);
        lv_obj_set_y(g_si_ctx.objects.title, 13);

        lv_anim_init(&a);
        lv_anim_set_var(&a, g_si_ctx.objects.title);
        lv_anim_set_exec_cb(&a, smart_island_anim_text_opa_cb);
        lv_anim_set_values(&a, LV_OPA_100, LV_OPA_40);
        lv_anim_set_time(&a, SMART_ISLAND_WARNING_FLASH_TIME);
        lv_anim_set_playback_time(&a, SMART_ISLAND_WARNING_FLASH_TIME);
        lv_anim_set_repeat_count(&a, 3);
        lv_anim_set_path_cb(&a, lv_anim_path_linear);
        lv_anim_set_ready_cb(&a, smart_island_warning_flash_finish_cb);
        lv_anim_start(&a);

        if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title)) {
            lv_coord_t expand_center_x = (SMART_ISLAND_W - g_si_ctx.warning.text_width_expand) / 2;
            if (expand_center_x < 0) expand_center_x = 0;

            lv_label_set_long_mode(g_si_ctx.objects.expand_title, LV_LABEL_LONG_CLIP);
            lv_obj_set_width(g_si_ctx.objects.expand_title, LV_SIZE_CONTENT);
            lv_obj_set_x(g_si_ctx.objects.expand_title, expand_center_x);
            lv_obj_set_y(g_si_ctx.objects.expand_title, 30);

            lv_anim_init(&a);
            lv_anim_set_var(&a, g_si_ctx.objects.expand_title);
            lv_anim_set_exec_cb(&a, smart_island_anim_text_opa_cb);
            lv_anim_set_values(&a, LV_OPA_100, LV_OPA_40);
            lv_anim_set_time(&a, SMART_ISLAND_WARNING_FLASH_TIME);
            lv_anim_set_playback_time(&a, SMART_ISLAND_WARNING_FLASH_TIME);
            lv_anim_set_repeat_count(&a, 3);
            lv_anim_set_path_cb(&a, lv_anim_path_linear);
            lv_anim_start(&a);
        }
        g_si_ctx.warning.marquee_running = true;
        return;
    }

    g_si_ctx.warning.marquee_running = true;
    g_si_ctx.warning.marquee_step = 0;
    lv_label_set_long_mode(g_si_ctx.objects.title, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_si_ctx.objects.title, compact_visible);
    lv_obj_set_x(g_si_ctx.objects.title, 36);
    if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title)) {
        lv_label_set_long_mode(g_si_ctx.objects.expand_title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_si_ctx.objects.expand_title, expand_visible);
        lv_obj_set_x(g_si_ctx.objects.expand_title, 32);
    }
    LV_UNUSED(a);
    smart_island_warning_marquee_run_step();
}

static void smart_island_warning_marquee_run_step(void)
{
    lv_anim_t a;
    lv_coord_t compact_visible = SMART_ISLAND_W - 36 - 14;
    lv_coord_t expand_visible = SMART_ISLAND_W - 32 - 12;
    lv_coord_t compact_left = (lv_coord_t)(36 - g_si_ctx.warning.text_width_compact);
    lv_coord_t compact_right = (lv_coord_t)(36 + compact_visible);
    lv_coord_t expand_left = (lv_coord_t)(32 - g_si_ctx.warning.text_width_expand);
    lv_coord_t expand_right = (lv_coord_t)(32 + expand_visible);
    bool left_to_right;
    lv_coord_t from_x;
    lv_coord_t to_x;

    if (!g_si_ctx.warning.marquee_running) return;

    if (g_si_ctx.warning.marquee_step >= (uint8_t)(SMART_ISLAND_WARNING_MARQUEE_CYCLES * 2U)) {
        bool pocket_confirmed;

        smart_island_warning_marquee_stop();
        pocket_confirmed = smart_island_warning_pocket_confirm();
        if (!fault_popup_is_showing()) {
            smart_island_restore_idle();
            if (!pocket_confirmed) {
                fault_popup_schedule_auto_confirm();
            }
        }
        return;
    }

    left_to_right = ((g_si_ctx.warning.marquee_step % 2U) == 0U);
    from_x = left_to_right ? compact_left : compact_right;
    to_x = left_to_right ? compact_right : compact_left;

    lv_anim_init(&a);
    lv_anim_set_var(&a, g_si_ctx.objects.title);
    lv_anim_set_exec_cb(&a, smart_island_anim_x_cb);
    lv_anim_set_values(&a, from_x, to_x);
    lv_anim_set_time(&a, SMART_ISLAND_WARNING_MARQUEE_TIME);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_ready_cb(&a, smart_island_warning_marquee_finish_cb);
    lv_anim_start(&a);

    if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title) &&
        g_si_ctx.warning.text_width_expand > expand_visible) {
        from_x = left_to_right ? expand_left : expand_right;
        to_x = left_to_right ? expand_right : expand_left;
        lv_anim_init(&a);
        lv_anim_set_var(&a, g_si_ctx.objects.expand_title);
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

    if (g_si_ctx.objects.time && lv_obj_is_valid(g_si_ctx.objects.time)) {
        lv_obj_set_style_translate_x(g_si_ctx.objects.time, 0, 0);
    }

    if (g_si_ctx.view.page == SMART_ISLAND_PAGE_INFO) {
        smart_island_reset_compact_header_position();
        smart_island_reset_time_position();

        if (g_si_ctx.objects.time && lv_obj_is_valid(g_si_ctx.objects.time)) {
            smart_island_update_idle_time();
            lv_obj_clear_flag(g_si_ctx.objects.time, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        if (g_si_ctx.objects.time && lv_obj_is_valid(g_si_ctx.objects.time)) {
            lv_obj_add_flag(g_si_ctx.objects.time, LV_OBJ_FLAG_HIDDEN);
            smart_island_reset_time_position();
        }
    }

    g_si_ctx.action.ignore_click_once = false;
    g_si_ctx.action.ignore_action_click_once = false;
    g_si_ctx.view.page_slide_dir = 0;
    g_si_ctx.view.anim_running = false;
}
static void smart_island_reset_page_positions(void) 
{
    if (g_si_ctx.objects.page_root && lv_obj_is_valid(g_si_ctx.objects.page_root)) lv_obj_set_x(g_si_ctx.objects.page_root, 0);
    if (g_si_ctx.objects.page_info && lv_obj_is_valid(g_si_ctx.objects.page_info)) lv_obj_set_x(g_si_ctx.objects.page_info, 0);
    if (g_si_ctx.objects.page_action && lv_obj_is_valid(g_si_ctx.objects.page_action)) lv_obj_set_x(g_si_ctx.objects.page_action, 0);
    if (g_si_ctx.objects.action_track && lv_obj_is_valid(g_si_ctx.objects.action_track)) {
        lv_obj_set_x(g_si_ctx.objects.action_track, -(lv_coord_t)g_si_ctx.action.page_index * SMART_ISLAND_W);
    }
}

static void smart_island_reset_compact_header_position(void) 
{
    if (g_si_ctx.objects.dot && lv_obj_is_valid(g_si_ctx.objects.dot)) {
        lv_obj_set_x(g_si_ctx.objects.dot, 16);
        lv_obj_set_y(g_si_ctx.objects.dot, 17);
        lv_obj_set_style_translate_x(g_si_ctx.objects.dot, 0, 0);
    }
    if (g_si_ctx.objects.title && lv_obj_is_valid(g_si_ctx.objects.title)) {
        lv_label_set_long_mode(g_si_ctx.objects.title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_si_ctx.objects.title, 150);
        lv_obj_set_x(g_si_ctx.objects.title, 36);
        lv_obj_set_y(g_si_ctx.objects.title, 13);
        lv_obj_set_style_translate_x(g_si_ctx.objects.title, 0, 0);
    }
    if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title)) {
        lv_label_set_long_mode(g_si_ctx.objects.expand_title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(g_si_ctx.objects.expand_title, 213);
        lv_obj_set_x(g_si_ctx.objects.expand_title, 24);
        lv_obj_set_y(g_si_ctx.objects.expand_title, 30);
    }
}

/* 完美还原您原版的时间对齐逻辑 */
static void smart_island_reset_time_position(void) 
{
    if (g_si_ctx.objects.time == NULL || !lv_obj_is_valid(g_si_ctx.objects.time)) {
        return;
    }

    if (g_si_ctx.view.visual == SMART_ISLAND_VISUAL_EXPANDED) {
        lv_obj_align(g_si_ctx.objects.time, LV_ALIGN_TOP_RIGHT, -14, 12);
    } else {
        lv_obj_align(g_si_ctx.objects.time, LV_ALIGN_RIGHT_MID, -14, 0);
    }
}

static void smart_island_raise_compact_header(void) 
{
    if (g_si_ctx.objects.dot && lv_obj_is_valid(g_si_ctx.objects.dot)) lv_obj_move_foreground(g_si_ctx.objects.dot);
    if (g_si_ctx.objects.title && lv_obj_is_valid(g_si_ctx.objects.title)) lv_obj_move_foreground(g_si_ctx.objects.title);
    if (g_si_ctx.objects.time && lv_obj_is_valid(g_si_ctx.objects.time)) lv_obj_move_foreground(g_si_ctx.objects.time);
}

static uint8_t smart_island_page_indicator_count_get(void)
{
    return (uint8_t)(g_si_ctx.action.page_count + 1U);
}

static uint8_t smart_island_page_indicator_active_get(void)
{
    if (g_si_ctx.view.page == SMART_ISLAND_PAGE_ACTION) {
        return (uint8_t)(g_si_ctx.action.page_index + 1U);
    }

    return 0U;
}

static void smart_island_page_indicator_sync(bool anim_en)
{
    uint8_t count;
    uint8_t active;

    if (g_si_ctx.objects.page_indicator == NULL || !lv_obj_is_valid(g_si_ctx.objects.page_indicator)) {
        return;
    }

    count = smart_island_page_indicator_count_get();
    active = smart_island_page_indicator_active_get();

    lv_capsule_pagination_set_count(g_si_ctx.objects.page_indicator, count);
    if (anim_en) {
        lv_capsule_pagination_set_active_page(g_si_ctx.objects.page_indicator, active);
    } else {
        lv_capsule_pagination_set_active_page_now(g_si_ctx.objects.page_indicator, active);
    }
}

static void smart_island_open_info_page_by_left_swipe(void)
{
    g_si_ctx.view.page_slide_dir = 1;
    smart_island_expand_if_needed(true);
    smart_island_set_page(SMART_ISLAND_PAGE_INFO, true);
}

static void smart_island_open_action_last_page_by_right_swipe(void)
{
    if (g_si_ctx.action.page_count == 0U) {
        return;
    }

    smart_island_action_page_set_index((uint8_t)(g_si_ctx.action.page_count - 1U), false);
    g_si_ctx.view.page_slide_dir = -1;
    smart_island_expand_if_needed(true);
    smart_island_set_page(SMART_ISLAND_PAGE_ACTION, true);
}

 

static void smart_island_expand_if_needed(bool anim_en) 
{
    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_RESULT) {
        return;
    }

    if (g_si_ctx.view.visual != SMART_ISLAND_VISUAL_EXPANDED) {
        smart_island_set_visual(SMART_ISLAND_VISUAL_EXPANDED, anim_en);
    }
}
static void smart_island_swipe_cb(lv_event_t *e) 
{
    lv_event_code_t code;
    lv_indev_t *indev;
    lv_point_t pt;

    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_WARNING) return;
    if (g_si_ctx.view.visual != SMART_ISLAND_VISUAL_EXPANDED) return;

    indev = lv_event_get_indev(e);
    if (indev == NULL) {
        indev = lv_indev_get_act();
        if (indev == NULL) return;
    }

    lv_indev_get_point(indev, &pt);
    code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        g_si_ctx.view.swipe.pressed = true;
        g_si_ctx.view.swipe.swiped = false;
        g_si_ctx.view.swipe.start_pt = pt;
        return;
    }

    if (code == LV_EVENT_PRESSING) {
        lv_coord_t dx;
        lv_coord_t dy;

        if (!g_si_ctx.view.swipe.pressed || g_si_ctx.view.swipe.swiped) return;

        dx = pt.x - g_si_ctx.view.swipe.start_pt.x;
        dy = pt.y - g_si_ctx.view.swipe.start_pt.y;

        if (LV_ABS(dx) > 10 && LV_ABS(dx) > LV_ABS(dy)) {
            g_si_ctx.action.ignore_click_once = true;
            g_si_ctx.action.ignore_action_click_once = true;
            g_si_ctx.view.swipe.swiped = true;

            if (dx < 0) {
                if (g_si_ctx.view.page == SMART_ISLAND_PAGE_INFO) {
                    smart_island_open_action_page();
                } else if (g_si_ctx.view.page == SMART_ISLAND_PAGE_ACTION) {
                    if (g_si_ctx.action.page_index + 1U < g_si_ctx.action.page_count) {
                        smart_island_action_page_set_index(g_si_ctx.action.page_index + 1U, true);
                    } else {
                        smart_island_open_info_page_by_left_swipe();
                    }
                }
            } else {
                if (g_si_ctx.view.page == SMART_ISLAND_PAGE_ACTION) {
                    if (g_si_ctx.action.page_index > 0U) {
                        smart_island_action_page_set_index(g_si_ctx.action.page_index - 1U, true);
                    } else {
                        smart_island_open_info_page();
                    }
                } else if (g_si_ctx.view.page == SMART_ISLAND_PAGE_INFO) {
                    smart_island_open_action_last_page_by_right_swipe();
                }
            }
        }
        return;
    }

    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        g_si_ctx.view.swipe.pressed = false;
        g_si_ctx.view.swipe.swiped = false;
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
    fault_popup_clear_pending();
    fault_popup_reset_auto_retry();
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

    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_WARNING) {
        if (g_si_ctx.action.ignore_click_once) {
            g_si_ctx.action.ignore_click_once = false;
            return;
        }
        if (fault_popup_show_pending_now() || smart_island_warning_fault_show()) {
            smart_island_warning_marquee_stop();
        }
        return;
    }

    if (g_si_ctx.view.anim_running) return;

    if (g_si_ctx.action.ignore_click_once) {
        g_si_ctx.action.ignore_click_once = false;
        return;
    }

    /* 点钞完成态只保留绿色收起岛，不允许点开展开页 */
    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_RESULT) {
        return;
    }

    if (g_si_ctx.view.visual == SMART_ISLAND_VISUAL_EXPANDED) {
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
    if (g_si_ctx.action.ignore_action_click_once) {
        g_si_ctx.action.ignore_action_click_once = false;
        return;
    }

    page_index = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    if (page_index >= SMART_ISLAND_ACTION_PAGE_COUNT) return;

    action_id = g_si_ctx.action.ids[page_index];

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

    if (g_si_ctx.action.callback) {
        g_si_ctx.action.callback(action_id);
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
    if (g_si_ctx.lifecycle.result_timer) {
        lv_timer_del(g_si_ctx.lifecycle.result_timer);
        g_si_ctx.lifecycle.result_timer = NULL;
    }
}

static void smart_island_clear_object_refs(void)
{
    uint8_t i;

    g_si_ctx.objects.root = NULL;
    g_si_ctx.objects.modal = NULL;
    g_si_ctx.objects.dot = NULL;
    g_si_ctx.objects.title = NULL;
    g_si_ctx.objects.subtitle = NULL;
    g_si_ctx.objects.time = NULL;
    g_si_ctx.objects.badge = NULL;
    g_si_ctx.objects.progress = NULL;
    g_si_ctx.objects.page_root = NULL;
    g_si_ctx.objects.page_info = NULL;
    g_si_ctx.objects.page_action = NULL;
    g_si_ctx.objects.expand_title = NULL;
    g_si_ctx.objects.expand_subtitle = NULL;
    g_si_ctx.objects.expand_last = NULL;
    g_si_ctx.objects.expand_divider = NULL;
    g_si_ctx.objects.expand_footer = NULL;
    g_si_ctx.objects.expand_extra = NULL;
    g_si_ctx.objects.action_track = NULL;
    g_si_ctx.objects.page_indicator = NULL;
    g_si_ctx.objects.quality_bar_bg = NULL;
    g_si_ctx.objects.quality_bar_fg = NULL;
    g_si_ctx.objects.quality_percent = NULL;

    for (i = 0; i < SMART_ISLAND_ACTION_PAGE_COUNT; i++) {
        g_si_ctx.objects.action_buttons[i] = NULL;
        g_si_ctx.objects.action_labels[i] = NULL;
        g_si_ctx.objects.action_arrows[i] = NULL;
    }
}

static void smart_island_update_idle_time(void) 
{
    machine_time_value_t now;
    char buf[16];
    if (g_si_ctx.objects.time == NULL || !lv_obj_is_valid(g_si_ctx.objects.time)) return;
    machine_time_get(&now);
    lv_snprintf(buf, sizeof(buf), "%02u:%02u:%02u",
        (unsigned)now.hour,
        (unsigned)now.minute,
        (unsigned)now.second);
    lv_label_set_text(g_si_ctx.objects.time, buf);
}

static void smart_island_get_currency_code(char *buf, size_t size)
{
    char curr_code[4];
    char selected_code[4];
    uint8_t selected_index;

    if (buf == NULL || size == 0U) return;

    currency_state_get_active_code(curr_code);
    selected_index = currency_state_active_index();
    if (curr_code[0] != '\0') {
        lv_snprintf(buf, size, "%s", curr_code);
    } else if (currency_state_get_code(selected_index, selected_code) && selected_code[0] != '\0') {
        lv_snprintf(buf, size, "%s", selected_code);
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
    uint8_t batch_num = machine_state_batch_num();

    return machine_state_batch_enabled() && batch_num > 0 && batch_num != 200;
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

static void smart_island_apply_quality_indicator(void)
{
    lv_coord_t fg_w;
    char percent_buf[8];

    if (g_si_ctx.objects.quality_bar_bg == NULL || !lv_obj_is_valid(g_si_ctx.objects.quality_bar_bg) ||
        g_si_ctx.objects.quality_bar_fg == NULL || !lv_obj_is_valid(g_si_ctx.objects.quality_bar_fg) ||
        g_si_ctx.objects.quality_percent == NULL || !lv_obj_is_valid(g_si_ctx.objects.quality_percent)) {
        return;
    }

    /* 不要跟随 page 在切页动画中反复 hidden/unhidden，避免回到信息页时闪一下 */
    if (!(g_si_ctx.view.scene == SMART_ISLAND_SCENE_IDLE &&
          g_si_ctx.view.visual == SMART_ISLAND_VISUAL_EXPANDED)) {
        lv_obj_add_flag(g_si_ctx.objects.quality_bar_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_si_ctx.objects.quality_percent, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    fg_w = (lv_coord_t)((56 * g_si_ctx.text.idle_quality_percent) / 100);
    if (fg_w < 0) {
        fg_w = 0;
    }
    if (fg_w > 56) {
        fg_w = 56;
    }

    lv_obj_set_width(g_si_ctx.objects.quality_bar_fg, fg_w);

    if (!g_si_ctx.text.idle_has_data) {
        lv_obj_set_style_bg_color(g_si_ctx.objects.quality_bar_bg, lv_color_hex(SMART_ISLAND_RESULT_NEUTRAL_GRAY), 0);
        lv_obj_set_style_bg_color(g_si_ctx.objects.quality_bar_fg, lv_color_hex(SMART_ISLAND_RESULT_NEUTRAL_GRAY), 0);
    } else {
        lv_obj_set_style_bg_color(g_si_ctx.objects.quality_bar_bg, lv_color_hex(SMART_ISLAND_RESULT_ISSUE_COLOR), 0);
        lv_obj_set_style_bg_color(g_si_ctx.objects.quality_bar_fg, lv_color_hex(SMART_ISLAND_RESULT_OK_COLOR), 0);
    }

    lv_snprintf(percent_buf, sizeof(percent_buf), "%u%%", (unsigned)g_si_ctx.text.idle_quality_percent);
    lv_label_set_text(g_si_ctx.objects.quality_percent, percent_buf);
    if (!g_si_ctx.text.idle_has_data) {
        lv_obj_set_style_text_color(g_si_ctx.objects.quality_percent, lv_color_hex(SMART_ISLAND_RESULT_NEUTRAL_GRAY), 0);
    } else if (g_si_ctx.text.idle_has_issue) {
        lv_obj_set_style_text_color(g_si_ctx.objects.quality_percent, lv_color_hex(SMART_ISLAND_TEXT_LIGHT), 0);
    } else {
        lv_obj_set_style_text_color(g_si_ctx.objects.quality_percent, lv_color_hex(SMART_ISLAND_RESULT_OK_COLOR), 0);
    }

    lv_obj_clear_flag(g_si_ctx.objects.quality_bar_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_si_ctx.objects.quality_percent, LV_OBJ_FLAG_HIDDEN);
}

static void smart_island_apply_texts(void)
{
    if (g_si_ctx.objects.title && lv_obj_is_valid(g_si_ctx.objects.title)) {
        lv_label_set_text(g_si_ctx.objects.title, g_si_ctx.text.compact);
    }
    if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title)) {
        lv_label_set_text(g_si_ctx.objects.expand_title, g_si_ctx.text.info_title);
    }
    if (g_si_ctx.objects.expand_subtitle && lv_obj_is_valid(g_si_ctx.objects.expand_subtitle)) {
        lv_label_set_text(g_si_ctx.objects.expand_subtitle, g_si_ctx.text.info_summary);
    }
    if (g_si_ctx.objects.expand_last && lv_obj_is_valid(g_si_ctx.objects.expand_last)) {
        lv_label_set_text(g_si_ctx.objects.expand_last, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_LAST_TAG));
    }
    if (g_si_ctx.objects.expand_footer && lv_obj_is_valid(g_si_ctx.objects.expand_footer)) {
        lv_label_set_text(g_si_ctx.objects.expand_footer, g_si_ctx.text.info_footer);
        if (g_si_ctx.text.info_footer[0] != '\0') lv_obj_clear_flag(g_si_ctx.objects.expand_footer, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_si_ctx.objects.expand_footer, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_si_ctx.objects.expand_extra && lv_obj_is_valid(g_si_ctx.objects.expand_extra)) {
        lv_label_set_text(g_si_ctx.objects.expand_extra, g_si_ctx.text.info_extra);
        if (g_si_ctx.text.info_extra[0] != '\0') lv_obj_clear_flag(g_si_ctx.objects.expand_extra, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_si_ctx.objects.expand_extra, LV_OBJ_FLAG_HIDDEN);
    }
}

static void smart_island_rebuild_scene_texts(void)
{
    char curr[8] = {0};
    char detail_line[96] = {0};
    const char *work_text = smart_island_get_work_mode_text();
    memset(g_si_ctx.text.compact, 0, sizeof(g_si_ctx.text.compact));
    memset(g_si_ctx.text.info_title, 0, sizeof(g_si_ctx.text.info_title));
    memset(g_si_ctx.text.info_summary, 0, sizeof(g_si_ctx.text.info_summary));
    memset(g_si_ctx.text.info_footer, 0, sizeof(g_si_ctx.text.info_footer));
    memset(g_si_ctx.text.info_extra, 0, sizeof(g_si_ctx.text.info_extra));

    smart_island_get_currency_code(curr, sizeof(curr));

    switch (g_si_ctx.view.scene) {
    case SMART_ISLAND_SCENE_COUNTING:
        lv_snprintf(g_si_ctx.text.compact, sizeof(g_si_ctx.text.compact), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNTING_TITLE));
        lv_snprintf(g_si_ctx.text.info_title, sizeof(g_si_ctx.text.info_title), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNTING_INFO_TITLE));
        if (sim.total_pcs > 0 && sim.total_amount > 0.0f) {
            lv_snprintf(g_si_ctx.text.info_summary, sizeof(g_si_ctx.text.info_summary),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_CUR_PCS_AMOUNT_FMT), curr, sim.total_pcs, sim.total_amount);
        } else if (sim.total_pcs > 0) {
            lv_snprintf(g_si_ctx.text.info_summary, sizeof(g_si_ctx.text.info_summary),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_CUR_PCS_FMT), curr, sim.total_pcs);
        } else {
            lv_snprintf(g_si_ctx.text.info_summary, sizeof(g_si_ctx.text.info_summary),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_CUR_MODE_FMT), curr, work_text);
        }
        if (smart_island_batch_enabled() && sim.total_pcs > 0) {
            lv_snprintf(g_si_ctx.text.info_footer, sizeof(g_si_ctx.text.info_footer),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_BATCH_PROGRESS_FMT), sim.total_pcs, (int)machine_state_batch_num());
        } else {
            lv_snprintf(g_si_ctx.text.info_footer, sizeof(g_si_ctx.text.info_footer), "%s", work_text);
        }
        break;

    case SMART_ISLAND_SCENE_RESULT:
        lv_snprintf(g_si_ctx.text.compact, sizeof(g_si_ctx.text.compact), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_FINISHED));
        g_si_ctx.text.info_title[0] = '\0';
        g_si_ctx.text.info_summary[0] = '\0';
        g_si_ctx.text.info_footer[0] = '\0';
        g_si_ctx.text.info_extra[0] = '\0';
        break;

    case SMART_ISLAND_SCENE_WARNING:
        lv_snprintf(g_si_ctx.text.compact, sizeof(g_si_ctx.text.compact), "%s",
            smart_island_text_or_default(g_si_ctx.warning.text, UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR));
        lv_snprintf(g_si_ctx.text.info_title, sizeof(g_si_ctx.text.info_title), "%s",
            smart_island_text_or_default(g_si_ctx.warning.text, UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR));
        lv_snprintf(g_si_ctx.text.info_summary, sizeof(g_si_ctx.text.info_summary), "%s",
            smart_island_text_or_default(g_si_ctx.warning.text, UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR));
        lv_snprintf(g_si_ctx.text.info_footer, sizeof(g_si_ctx.text.info_footer), "%s",
            "");
        break;

    case SMART_ISLAND_SCENE_UPDATE:
        lv_snprintf(g_si_ctx.text.compact, sizeof(g_si_ctx.text.compact), "%s",
            smart_island_text_or_default(g_si_ctx.view.content.title, UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_TITLE));
        lv_snprintf(g_si_ctx.text.info_title, sizeof(g_si_ctx.text.info_title), "%s",
            smart_island_text_or_default(g_si_ctx.view.content.title, UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_TITLE));
        lv_snprintf(g_si_ctx.text.info_summary, sizeof(g_si_ctx.text.info_summary), "%s",
            smart_island_text_or_default(g_si_ctx.view.content.subtitle, UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_SUBTITLE));
        lv_snprintf(g_si_ctx.text.info_footer, sizeof(g_si_ctx.text.info_footer), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_INFO_FOOTER));
        break;

    case SMART_ISLAND_SCENE_QR:
        lv_snprintf(g_si_ctx.text.compact, sizeof(g_si_ctx.text.compact), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_QR_READY));
        lv_snprintf(g_si_ctx.text.info_title, sizeof(g_si_ctx.text.info_title), "%s",
            smart_island_text_or_default(g_si_ctx.view.content.title, UI_TEXT_WIDGET_SMART_ISLAND_QR_READY));
        lv_snprintf(g_si_ctx.text.info_summary, sizeof(g_si_ctx.text.info_summary), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_QR_INFO_SUBTITLE));
        if (sim.total_pcs > 0 && sim.total_amount > 0.0f) {
            lv_snprintf(g_si_ctx.text.info_footer, sizeof(g_si_ctx.text.info_footer),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_CUR_PCS_AMOUNT_FMT), curr, sim.total_pcs, sim.total_amount);
        } else {
            lv_snprintf(g_si_ctx.text.info_footer, sizeof(g_si_ctx.text.info_footer), "%s",
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_QR_INFO_FOOTER));
        }
        break;

    case SMART_ISLAND_SCENE_IDLE:
    default:
    {
        int current_valid = g_si_ctx.text.analysis_valid ? g_si_ctx.text.analysis_valid_pcs : 0;
        int current_total = 0;
        int current_suspect = g_si_ctx.text.analysis_valid ? g_si_ctx.text.analysis_suspect_pcs : 0;
        int current_damaged = g_si_ctx.text.analysis_valid ? g_si_ctx.text.analysis_damaged_pcs : 0;
        int current_issue = current_suspect + current_damaged;

        lv_snprintf(g_si_ctx.text.compact, sizeof(g_si_ctx.text.compact),
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_READY_CUR_FMT), curr);

        g_si_ctx.text.info_title[0] = '\0';

        current_total = current_valid + current_issue;
        g_si_ctx.text.idle_has_issue = (current_issue > 0);
        g_si_ctx.text.idle_has_data = g_si_ctx.text.analysis_valid && current_total > 0;
        g_si_ctx.text.idle_no_count = !g_si_ctx.text.analysis_valid;

        if (g_si_ctx.text.idle_line1[0] != '\0') {
            lv_snprintf(g_si_ctx.text.info_summary, sizeof(g_si_ctx.text.info_summary), "%s",
                g_si_ctx.text.idle_line1);
        } else if (sim.last_total_pcs > 0 || sim.last_total_amount > 0.0f) {
            lv_snprintf(g_si_ctx.text.info_summary, sizeof(g_si_ctx.text.info_summary),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_PCS_AMOUNT_FMT), sim.last_total_pcs, sim.last_total_amount);
        } else if (Machine_para.last_total_pcs > 0U || Machine_para.last_total_amount > 0U) {
            lv_snprintf(g_si_ctx.text.info_summary, sizeof(g_si_ctx.text.info_summary),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_PCS_AMOUNT_FMT),
                (int)Machine_para.last_total_pcs, (float)Machine_para.last_total_amount);
        } else {
            lv_snprintf(g_si_ctx.text.info_summary, sizeof(g_si_ctx.text.info_summary),
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_PCS_AMOUNT_FMT), 0, 0.0f);
        }

        if (g_si_ctx.text.idle_line2[0] != '\0') {
            lv_snprintf(g_si_ctx.text.info_footer, sizeof(g_si_ctx.text.info_footer), "%s",
                g_si_ctx.text.idle_line2);
        } else if (g_si_ctx.text.idle_no_count) {
            lv_snprintf(g_si_ctx.text.info_footer, sizeof(g_si_ctx.text.info_footer), "%s",
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_IDLE_NO_COUNT));
        } else {
            lv_snprintf(g_si_ctx.text.info_footer, sizeof(g_si_ctx.text.info_footer), "%s",
                g_si_ctx.text.idle_has_issue
                ? ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RESULT_ISSUE_TITLE)
                : ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RESULT_OK_TITLE));
        }

        if (g_si_ctx.text.idle_line3[0] != '\0') {
            lv_snprintf(g_si_ctx.text.info_extra, sizeof(g_si_ctx.text.info_extra), "%s",
                g_si_ctx.text.idle_line3);
        } else if (g_si_ctx.text.idle_no_count) {
            lv_snprintf(g_si_ctx.text.info_extra, sizeof(g_si_ctx.text.info_extra), "%s",
                ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_PLACE_BANKNOTES));
        } else {
            if (g_si_ctx.text.idle_has_issue) {
                lv_snprintf(detail_line, sizeof(detail_line),
                    ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RESULT_ISSUE_DETAIL_FMT),
                    current_suspect,
                    current_damaged);
                lv_snprintf(g_si_ctx.text.info_extra, sizeof(g_si_ctx.text.info_extra), "%s", detail_line);
            } else {
                lv_snprintf(g_si_ctx.text.info_extra, sizeof(g_si_ctx.text.info_extra), "%s",
                    ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_RESULT_OK_DETAIL));
            }
        }

        if (current_total > 0) {
            g_si_ctx.text.idle_quality_percent = (uint8_t)((current_valid * 100) / current_total);
        } else {
            g_si_ctx.text.idle_quality_percent = 0;
        }
        break;
    }
    }

    smart_island_apply_texts();
}

static void smart_island_modal_update(void) 
{
    if (g_si_ctx.objects.modal == NULL || !lv_obj_is_valid(g_si_ctx.objects.modal)) return;
    if (g_si_ctx.view.visual == SMART_ISLAND_VISUAL_EXPANDED) {
        lv_obj_clear_flag(g_si_ctx.objects.modal, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(g_si_ctx.objects.modal);
        lv_obj_move_foreground(g_si_ctx.objects.root);
    } else {
        lv_obj_add_flag(g_si_ctx.objects.modal, LV_OBJ_FLAG_HIDDEN);
    }
}

static void smart_island_update_pages_visible(void) 
{
    if (g_si_ctx.objects.page_root && lv_obj_is_valid(g_si_ctx.objects.page_root)) {
        if (g_si_ctx.view.visual == SMART_ISLAND_VISUAL_EXPANDED) {
            lv_obj_clear_flag(g_si_ctx.objects.page_root, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_si_ctx.objects.page_root, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_si_ctx.objects.page_info && lv_obj_is_valid(g_si_ctx.objects.page_info)) {
        if (g_si_ctx.view.page == SMART_ISLAND_PAGE_INFO) {
            lv_obj_clear_flag(g_si_ctx.objects.page_info, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_si_ctx.objects.page_info, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_si_ctx.objects.page_action && lv_obj_is_valid(g_si_ctx.objects.page_action)) {
        if (g_si_ctx.view.page == SMART_ISLAND_PAGE_ACTION) {
            lv_obj_clear_flag(g_si_ctx.objects.page_action, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_si_ctx.objects.page_action, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    if (g_si_ctx.objects.page_indicator && lv_obj_is_valid(g_si_ctx.objects.page_indicator)) {
        if (g_si_ctx.view.visual == SMART_ISLAND_VISUAL_EXPANDED) {
            lv_obj_clear_flag(g_si_ctx.objects.page_indicator, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_si_ctx.objects.page_indicator, LV_OBJ_FLAG_HIDDEN);
        }
    }

    smart_island_apply_quality_indicator();
}

static void smart_island_apply_scene_style(void) 
{
    uint32_t bg_hex = SMART_ISLAND_BG_IDLE;
    lv_color_t title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
    lv_color_t dot_color = lv_color_hex(SMART_ISLAND_READY_DOT);
    bool show_time = true;
    bool show_dot = true;
    
    if (g_si_ctx.objects.root == NULL || !lv_obj_is_valid(g_si_ctx.objects.root)) return;

    switch (g_si_ctx.view.scene) {
    case SMART_ISLAND_SCENE_IDLE:
    case SMART_ISLAND_SCENE_QR:
        bg_hex = SMART_ISLAND_BG_IDLE;
        title_color = lv_color_hex(SMART_ISLAND_TEXT_LIGHT);
        dot_color = lv_color_hex(SMART_ISLAND_READY_DOT);
        show_time = (g_si_ctx.view.scene == SMART_ISLAND_SCENE_IDLE);
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
            (g_si_ctx.warning.level == SMART_ISLAND_WARNING_LEVEL_ERROR)
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
    if (g_si_ctx.view.visual == SMART_ISLAND_VISUAL_EXPANDED &&
        g_si_ctx.view.page == SMART_ISLAND_PAGE_ACTION) {
        show_time = false;
    }

    smart_island_bg_color_apply_anim(bg_hex);
    lv_obj_set_style_border_width(g_si_ctx.objects.root, 0, 0);
    lv_obj_set_style_outline_width(g_si_ctx.objects.root, 0, 0);
    lv_obj_set_style_shadow_width(g_si_ctx.objects.root, 0, 0);

    if (g_si_ctx.objects.title && lv_obj_is_valid(g_si_ctx.objects.title)) {
        lv_obj_set_style_text_color(g_si_ctx.objects.title, title_color, 0);
        lv_obj_set_style_text_opa(g_si_ctx.objects.title, LV_OPA_COVER, 0);
        lv_obj_clear_flag(g_si_ctx.objects.title, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (g_si_ctx.objects.subtitle && lv_obj_is_valid(g_si_ctx.objects.subtitle)) {
        lv_obj_add_flag(g_si_ctx.objects.subtitle, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title)) {
        lv_obj_set_style_text_opa(g_si_ctx.objects.expand_title, LV_OPA_COVER, 0);
        lv_obj_clear_flag(g_si_ctx.objects.expand_title, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_si_ctx.objects.expand_subtitle && lv_obj_is_valid(g_si_ctx.objects.expand_subtitle)) {
        lv_obj_set_style_text_opa(g_si_ctx.objects.expand_subtitle, LV_OPA_COVER, 0);
        lv_obj_clear_flag(g_si_ctx.objects.expand_subtitle, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_si_ctx.objects.expand_last && lv_obj_is_valid(g_si_ctx.objects.expand_last)) {
        if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_IDLE) {
            lv_obj_clear_flag(g_si_ctx.objects.expand_last, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_si_ctx.objects.expand_last, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (g_si_ctx.objects.expand_divider && lv_obj_is_valid(g_si_ctx.objects.expand_divider)) {
        if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_IDLE) {
            lv_obj_clear_flag(g_si_ctx.objects.expand_divider, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_si_ctx.objects.expand_divider, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (g_si_ctx.objects.expand_footer && lv_obj_is_valid(g_si_ctx.objects.expand_footer)) {
        if (g_si_ctx.text.info_footer[0] != '\0') lv_obj_clear_flag(g_si_ctx.objects.expand_footer, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_si_ctx.objects.expand_footer, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_si_ctx.objects.expand_extra && lv_obj_is_valid(g_si_ctx.objects.expand_extra)) {
        if (g_si_ctx.text.info_extra[0] != '\0') lv_obj_clear_flag(g_si_ctx.objects.expand_extra, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_si_ctx.objects.expand_extra, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_recolor(g_si_ctx.objects.expand_extra, true);
    }
    if (g_si_ctx.objects.expand_subtitle && lv_obj_is_valid(g_si_ctx.objects.expand_subtitle)) {
        lv_label_set_recolor(g_si_ctx.objects.expand_subtitle, true);
    }

    if (g_si_ctx.objects.expand_subtitle && lv_obj_is_valid(g_si_ctx.objects.expand_subtitle)) {
        if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_IDLE) {
            lv_obj_set_style_text_color(g_si_ctx.objects.expand_subtitle, lv_color_hex(SMART_ISLAND_TEXT_LIGHT), 0);
            lv_obj_set_style_text_align(g_si_ctx.objects.expand_subtitle, LV_TEXT_ALIGN_RIGHT, 0);
            lv_obj_set_width(g_si_ctx.objects.expand_subtitle, 173);
            lv_obj_set_pos(g_si_ctx.objects.expand_subtitle, 68, 36);
        } else {
            lv_obj_set_style_text_color(g_si_ctx.objects.expand_subtitle, title_color, 0);
            lv_obj_set_style_text_align(g_si_ctx.objects.expand_subtitle, LV_TEXT_ALIGN_LEFT, 0);
            lv_obj_set_width(g_si_ctx.objects.expand_subtitle, SMART_ISLAND_W - 20);
            lv_obj_set_pos(g_si_ctx.objects.expand_subtitle, 20, 42);
        }
    }

    if (g_si_ctx.objects.expand_last && lv_obj_is_valid(g_si_ctx.objects.expand_last)) {
        lv_obj_set_style_text_color(g_si_ctx.objects.expand_last, lv_color_hex(SMART_ISLAND_RESULT_DETAIL_GRAY), 0);
    }

    if (g_si_ctx.objects.expand_divider && lv_obj_is_valid(g_si_ctx.objects.expand_divider)) {
        lv_obj_set_style_bg_color(g_si_ctx.objects.expand_divider, lv_color_hex(0x515151), 0);
        lv_obj_set_style_bg_opa(g_si_ctx.objects.expand_divider, LV_OPA_COVER, 0);
    }

    if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title)) {
        lv_obj_set_pos(g_si_ctx.objects.expand_title, 20, 18);
    }

    if (g_si_ctx.objects.expand_footer && lv_obj_is_valid(g_si_ctx.objects.expand_footer)) {
        lv_obj_set_pos(g_si_ctx.objects.expand_footer, 20, 62);
    }

    if (g_si_ctx.objects.expand_extra && lv_obj_is_valid(g_si_ctx.objects.expand_extra)) {
        lv_obj_set_pos(g_si_ctx.objects.expand_extra, 20, 80);
    }

    if (g_si_ctx.objects.quality_bar_bg && lv_obj_is_valid(g_si_ctx.objects.quality_bar_bg)) {
        lv_obj_set_pos(g_si_ctx.objects.quality_bar_bg, 166, 64);
    }
    if (g_si_ctx.objects.quality_percent && lv_obj_is_valid(g_si_ctx.objects.quality_percent)) {
        lv_obj_set_pos(g_si_ctx.objects.quality_percent, 228, 62);
    }
    if (g_si_ctx.objects.expand_last && lv_obj_is_valid(g_si_ctx.objects.expand_last)) {
        lv_obj_set_pos(g_si_ctx.objects.expand_last, 20, 36);
    }
    if (g_si_ctx.objects.expand_divider && lv_obj_is_valid(g_si_ctx.objects.expand_divider)) {
        lv_obj_set_pos(g_si_ctx.objects.expand_divider, 20, 56);
    }

    if (g_si_ctx.objects.expand_title && lv_obj_is_valid(g_si_ctx.objects.expand_title)) {
        if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_IDLE) {
            lv_obj_add_flag(g_si_ctx.objects.expand_title, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(g_si_ctx.objects.expand_title, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (g_si_ctx.objects.expand_footer && lv_obj_is_valid(g_si_ctx.objects.expand_footer)) {
        if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_IDLE) {
            lv_color_t footer_color;
            if (g_si_ctx.text.idle_no_count) {
                footer_color = lv_color_hex(SMART_ISLAND_TEXT_SUB);
            } else if (g_si_ctx.text.idle_has_issue) {
                footer_color = lv_color_hex(SMART_ISLAND_RESULT_ISSUE_TITLE_COLOR);
            } else {
                footer_color = lv_color_hex(SMART_ISLAND_RESULT_OK_COLOR);
            }
            lv_obj_set_style_text_color(g_si_ctx.objects.expand_footer, footer_color, 0);
        } else {
            lv_obj_set_style_text_color(g_si_ctx.objects.expand_footer, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);
        }
    }

    if (g_si_ctx.objects.expand_extra && lv_obj_is_valid(g_si_ctx.objects.expand_extra)) {
        if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_IDLE) {
            lv_color_t extra_color = lv_color_hex(g_si_ctx.text.idle_has_issue
                ? SMART_ISLAND_RESULT_ISSUE_COLOR
                : (g_si_ctx.text.idle_no_count ? SMART_ISLAND_LAST_TEXT_GRAY : SMART_ISLAND_RESULT_DETAIL_GRAY));
            lv_obj_set_style_text_color(g_si_ctx.objects.expand_extra, extra_color, 0);
        } else {
            lv_obj_set_style_text_color(g_si_ctx.objects.expand_extra, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);
        }
    }

    if (g_si_ctx.objects.dot && lv_obj_is_valid(g_si_ctx.objects.dot)) {
        lv_obj_set_style_bg_color(g_si_ctx.objects.dot, dot_color, 0);
        if (show_dot) lv_obj_clear_flag(g_si_ctx.objects.dot, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_si_ctx.objects.dot, LV_OBJ_FLAG_HIDDEN);
    }

    if (show_time) {
        smart_island_update_idle_time();
        lv_obj_clear_flag(g_si_ctx.objects.time, LV_OBJ_FLAG_HIDDEN);
        smart_island_reset_time_position();
    } else {
        lv_obj_add_flag(g_si_ctx.objects.time, LV_OBJ_FLAG_HIDDEN);
    }

    if (g_si_ctx.objects.progress && lv_obj_is_valid(g_si_ctx.objects.progress)) {
        if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_UPDATE) lv_obj_clear_flag(g_si_ctx.objects.progress, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(g_si_ctx.objects.progress, LV_OBJ_FLAG_HIDDEN);
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

    if (g_si_ctx.objects.root == NULL || !lv_obj_is_valid(g_si_ctx.objects.root)) return;

    if (visual == SMART_ISLAND_VISUAL_MINI) {
        w = SMART_ISLAND_MINI_W;
        x = SMART_ISLAND_X + (SMART_ISLAND_W - w) / 2;
    } else if (visual == SMART_ISLAND_VISUAL_EXPANDED) {
        y = SMART_ISLAND_Y - (SMART_ISLAND_ACTION_EXPAND_H - SMART_ISLAND_COMPACT_H);
        h = SMART_ISLAND_ACTION_EXPAND_H;
    }

    lv_obj_set_pos(g_si_ctx.objects.root, x, y);
    lv_obj_set_size(g_si_ctx.objects.root, w, h);
    
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

    if (g_si_ctx.objects.root == NULL || !lv_obj_is_valid(g_si_ctx.objects.root)) return;

    if (visual == SMART_ISLAND_VISUAL_MINI) {
        dst_w = SMART_ISLAND_MINI_W;
        dst_x = SMART_ISLAND_X + (SMART_ISLAND_W - dst_w) / 2;
        anim_time = SMART_ISLAND_COLLAPSE_TIME;
    } else if (visual == SMART_ISLAND_VISUAL_EXPANDED) {
        dst_y = SMART_ISLAND_Y - (SMART_ISLAND_ACTION_EXPAND_H - SMART_ISLAND_COMPACT_H);
        dst_h = SMART_ISLAND_ACTION_EXPAND_H;
    }

    g_si_ctx.view.anim_running = true;

    lv_anim_del(g_si_ctx.objects.root, smart_island_anim_x_cb);
    lv_anim_del(g_si_ctx.objects.root, smart_island_anim_y_cb);
    lv_anim_del(g_si_ctx.objects.root, smart_island_anim_w_cb);
    lv_anim_del(g_si_ctx.objects.root, smart_island_anim_h_cb);

    lv_anim_init(&a);
    lv_anim_set_var(&a, g_si_ctx.objects.root);
    lv_anim_set_exec_cb(&a, smart_island_anim_y_cb);
    lv_anim_set_values(&a, lv_obj_get_y(g_si_ctx.objects.root), dst_y);
    lv_anim_set_time(&a, anim_time);
    lv_anim_set_path_cb(&a, (visual == SMART_ISLAND_VISUAL_EXPANDED) ? lv_anim_path_overshoot : lv_anim_path_ease_out);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, smart_island_anim_h_cb);
    lv_anim_set_values(&a, lv_obj_get_height(g_si_ctx.objects.root), dst_h);
    lv_anim_set_ready_cb(&a, smart_island_anim_finish_cb);
    lv_anim_start(&a);

    smart_island_reset_time_position();

    smart_island_update_pages_visible();
    smart_island_modal_update();
}

static void smart_island_pulse_stop(void) 
{
    if (g_si_ctx.objects.root == NULL || !lv_obj_is_valid(g_si_ctx.objects.root)) return;
    lv_anim_del(g_si_ctx.objects.root, smart_island_anim_zoom_cb);
    lv_obj_set_style_transform_zoom(g_si_ctx.objects.root, 256, 0);
}
static void smart_island_page_slide_anim(smart_island_page_t old_page, smart_island_page_t new_page) 
{
    lv_anim_t a;
    lv_obj_t *old_obj = NULL, *new_obj = NULL;
    lv_coord_t start_old_x = 0, end_old_x = 0;
    lv_coord_t start_new_x = 0, end_new_x = 0;
    lv_coord_t delta_x = 0;
    bool slide_left = false;

    if (g_si_ctx.objects.page_info == NULL || g_si_ctx.objects.page_action == NULL) return;

    old_obj = (old_page == SMART_ISLAND_PAGE_INFO) ? g_si_ctx.objects.page_info : g_si_ctx.objects.page_action;
    new_obj = (new_page == SMART_ISLAND_PAGE_INFO) ? g_si_ctx.objects.page_info : g_si_ctx.objects.page_action;

    if (old_obj == new_obj) return;

    g_si_ctx.view.anim_running = true;

    smart_island_raise_compact_header();

    lv_obj_clear_flag(old_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(new_obj, LV_OBJ_FLAG_HIDDEN);

    if (g_si_ctx.view.page_slide_dir > 0) {
        slide_left = true;
    } else if (g_si_ctx.view.page_slide_dir < 0) {
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

    if (g_si_ctx.objects.title && lv_obj_is_valid(g_si_ctx.objects.title)) {
        lv_anim_del(g_si_ctx.objects.title, smart_island_anim_translate_x_cb);
        lv_obj_clear_flag(g_si_ctx.objects.title, LV_OBJ_FLAG_HIDDEN);
        if (new_page == SMART_ISLAND_PAGE_ACTION) {
            lv_obj_set_style_translate_x(g_si_ctx.objects.title, 0, 0);
            lv_anim_init(&a);
            lv_anim_set_var(&a, g_si_ctx.objects.title);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, 0, delta_x);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        } else {
            lv_obj_set_style_translate_x(g_si_ctx.objects.title, -delta_x, 0);
            lv_anim_init(&a);
            lv_anim_set_var(&a, g_si_ctx.objects.title);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, -delta_x, 0);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        }
    }

    if (g_si_ctx.objects.dot && lv_obj_is_valid(g_si_ctx.objects.dot)) {
        lv_anim_del(g_si_ctx.objects.dot, smart_island_anim_translate_x_cb);
        lv_obj_clear_flag(g_si_ctx.objects.dot, LV_OBJ_FLAG_HIDDEN);
        if (new_page == SMART_ISLAND_PAGE_ACTION) {
            lv_obj_set_style_translate_x(g_si_ctx.objects.dot, 0, 0);
            lv_anim_init(&a);
            lv_anim_set_var(&a, g_si_ctx.objects.dot);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, 0, delta_x);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        } else {
            lv_obj_set_style_translate_x(g_si_ctx.objects.dot, -delta_x, 0);
            lv_anim_init(&a);
            lv_anim_set_var(&a, g_si_ctx.objects.dot);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, -delta_x, 0);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        }
    }

    if (g_si_ctx.objects.time && lv_obj_is_valid(g_si_ctx.objects.time)) {
        lv_anim_del(g_si_ctx.objects.time, smart_island_anim_translate_x_cb);

        if (new_page == SMART_ISLAND_PAGE_ACTION) {
            lv_obj_clear_flag(g_si_ctx.objects.time, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_translate_x(g_si_ctx.objects.time, 0, 0);

            lv_anim_init(&a);
            lv_anim_set_var(&a, g_si_ctx.objects.time);
            lv_anim_set_exec_cb(&a, smart_island_anim_translate_x_cb);
            lv_anim_set_values(&a, 0, delta_x);
            lv_anim_set_time(&a, 180);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
            lv_anim_start(&a);
        } else {
            smart_island_reset_time_position();
            smart_island_update_idle_time();
            lv_obj_clear_flag(g_si_ctx.objects.time, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_translate_x(g_si_ctx.objects.time, -delta_x, 0);

            lv_anim_init(&a);
            lv_anim_set_var(&a, g_si_ctx.objects.time);
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
    g_si_ctx.view.page = page;
    smart_island_update_pages_visible();
    smart_island_reset_page_positions();
}

static void smart_island_page_apply_anim(smart_island_page_t page) 
{
    smart_island_page_t old_page = g_si_ctx.view.page;
    g_si_ctx.view.page = page;
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

    btn = g_si_ctx.objects.action_buttons[index];
    arrow = g_si_ctx.objects.action_arrows[index];
    if (btn == NULL || !lv_obj_is_valid(btn)) {
        return;
    }

    is_switch = (g_si_ctx.action.ids[index] == SMART_ISLAND_ACTION_FUNC3 ||
        g_si_ctx.action.ids[index] == SMART_ISLAND_ACTION_FUNC4);

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
            lv_obj_set_style_text_font(arrow, &lv_font_instrument_sans_medium_18, 0);
            lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -16, 0);
        }
        return;
    }

    if (g_si_ctx.action.ids[index] == SMART_ISLAND_ACTION_FUNC4) {
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
        lv_obj_set_style_text_font(arrow, &lv_font_instrument_sans_medium_12, 0);
        lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -12, 0);
    }
}

static void smart_island_action_item_apply(uint8_t index) 
{
    lv_obj_t *btn = NULL, *label = NULL;
    lv_coord_t page_x = 0;

    if (index >= SMART_ISLAND_ACTION_PAGE_COUNT) return;

    btn = g_si_ctx.objects.action_buttons[index];
    label = g_si_ctx.objects.action_labels[index];

    if (btn == NULL || !lv_obj_is_valid(btn)) return;

    if (index >= g_si_ctx.action.page_count) {
        lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    page_x = (lv_coord_t)index * SMART_ISLAND_W;
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(btn, page_x + SMART_ISLAND_ACTION_BTN_X, SMART_ISLAND_ACTION_BTN_Y);

    if (label && lv_obj_is_valid(label)) {
        if (g_si_ctx.action.ids[index] == SMART_ISLAND_ACTION_FUNC3) {
            lv_label_set_text(label, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC3));
        } else if (g_si_ctx.action.ids[index] == SMART_ISLAND_ACTION_FUNC4) {
            lv_label_set_text(label, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC4));
        } else if (g_si_ctx.action.text_ids[index] < UI_TEXT_MAX) {
            lv_label_set_text(label, ui_text_get(g_si_ctx.action.text_ids[index]));
        } else if (g_si_ctx.action.texts[index][0] != '\0') {
            lv_label_set_text(label, g_si_ctx.action.texts[index]);
        } else {
            lv_label_set_text(label, "");
        }
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 16, 0); 
    }

    smart_island_action_btn_style_apply(index);
}
static void smart_island_action_page_refresh_language_texts(void) 
{
    for (uint8_t i = 0; i < g_si_ctx.action.page_count; i++) {
        if (g_si_ctx.action.text_ids[i] < UI_TEXT_MAX) {
            lv_snprintf(g_si_ctx.action.texts[i], sizeof(g_si_ctx.action.texts[i]), "%s",
                ui_text_get(g_si_ctx.action.text_ids[i]));
        }
        smart_island_action_item_apply(i);
    }
}
static void smart_island_action_page_slide_anim(uint8_t old_index, uint8_t new_index) 
{
    lv_anim_t a;

    if (g_si_ctx.objects.action_track == NULL || !lv_obj_is_valid(g_si_ctx.objects.action_track)) return;

    if (old_index == new_index) {
        lv_obj_set_x(g_si_ctx.objects.action_track, -(lv_coord_t)new_index * SMART_ISLAND_W);
        return;
    }

    g_si_ctx.view.anim_running = true;
    lv_anim_del(g_si_ctx.objects.action_track, smart_island_anim_x_cb);

    lv_anim_init(&a);
    lv_anim_set_var(&a, g_si_ctx.objects.action_track);
    lv_anim_set_exec_cb(&a, smart_island_anim_x_cb);
    lv_anim_set_values(&a, -(lv_coord_t)old_index * SMART_ISLAND_W, -(lv_coord_t)new_index * SMART_ISLAND_W);
    lv_anim_set_time(&a, 160);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&a, smart_island_action_page_slide_anim_finish_cb);
    lv_anim_start(&a);
}
static void smart_island_action_page_set_index(uint8_t index, bool anim_en) 
{
    uint8_t target_index = (index >= g_si_ctx.action.page_count && g_si_ctx.action.page_count > 0U)
        ? (uint8_t)(g_si_ctx.action.page_count - 1U)
        : index;
    uint8_t old_index = g_si_ctx.action.page_index;

    if (g_si_ctx.action.page_count == 0U) return;

    g_si_ctx.action.page_index = target_index;
    smart_island_page_indicator_sync(anim_en);

    if (g_si_ctx.objects.action_track == NULL || !lv_obj_is_valid(g_si_ctx.objects.action_track)) return;

    if (anim_en) {
        smart_island_action_page_slide_anim(old_index, target_index);
    } else {
        lv_anim_del(g_si_ctx.objects.action_track, smart_island_anim_x_cb);
        lv_obj_set_x(g_si_ctx.objects.action_track, -(lv_coord_t)target_index * SMART_ISLAND_W);
    }
}
bool smart_island_action_page_set_count(uint8_t count) 
{
    uint8_t new_count = (count == 0U) ? 1U : (count > SMART_ISLAND_ACTION_PAGE_COUNT ? SMART_ISLAND_ACTION_PAGE_COUNT : count);

    g_si_ctx.action.page_count = new_count;

    if (g_si_ctx.action.page_index >= g_si_ctx.action.page_count) {
        g_si_ctx.action.page_index = (uint8_t)(g_si_ctx.action.page_count - 1U);
    }

    if (g_si_ctx.objects.action_track && lv_obj_is_valid(g_si_ctx.objects.action_track)) {
        lv_obj_set_size(g_si_ctx.objects.action_track,
                        (lv_coord_t)(SMART_ISLAND_W * g_si_ctx.action.page_count),
                        SMART_ISLAND_ACTION_EXPAND_H);
    }

    for (uint8_t i = 0; i < SMART_ISLAND_ACTION_PAGE_COUNT; i++) {
        smart_island_action_item_apply(i);
    }

    if (g_si_ctx.objects.action_track && lv_obj_is_valid(g_si_ctx.objects.action_track)) {
        lv_obj_set_x(g_si_ctx.objects.action_track,
                     -(lv_coord_t)g_si_ctx.action.page_index * SMART_ISLAND_W);
    }

    smart_island_page_indicator_sync(false);
    return true;
}
bool smart_island_action_page_set_lang_item(uint8_t index, uint8_t action_id, ui_text_id_t text_id) 
{
    if (index >= SMART_ISLAND_ACTION_PAGE_COUNT ||
        (unsigned int)text_id >= (unsigned int)UI_TEXT_MAX) {
        return false;
    }
    g_si_ctx.action.ids[index] = action_id;
    g_si_ctx.action.text_ids[index] = text_id;
    g_si_ctx.action.texts[index][0] = '\0';
    if (index + 1U > g_si_ctx.action.page_count) smart_island_action_page_set_count(index + 1U);
    else smart_island_action_item_apply(index);
    return true;
}

bool smart_island_action_page_set_item(uint8_t index, uint8_t action_id, const char *text) 
{
    if (index >= SMART_ISLAND_ACTION_PAGE_COUNT) return false;
    g_si_ctx.action.ids[index] = action_id;
    g_si_ctx.action.text_ids[index] = UI_TEXT_MAX;
    if (text && text[0] != '\0') lv_snprintf(g_si_ctx.action.texts[index], sizeof(g_si_ctx.action.texts[index]), "%s", text);
    else g_si_ctx.action.texts[index][0] = '\0';
    if (index + 1U > g_si_ctx.action.page_count) smart_island_action_page_set_count(index + 1U);
    else smart_island_action_item_apply(index);
    return true;
}
static void smart_island_action_btn_create(void) 
{
    static const uint8_t default_ids[SMART_ISLAND_ACTION_PAGE_COUNT] = {
        SMART_ISLAND_ACTION_FUNC4, SMART_ISLAND_ACTION_FUNC3,
        SMART_ISLAND_ACTION_TIME_SETTING, SMART_ISLAND_ACTION_QR
    };
    static const ui_text_id_t default_text_ids[SMART_ISLAND_ACTION_PAGE_COUNT] = {
        UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC4, UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC3,
        UI_TEXT_WIDGET_SMART_ISLAND_ACTION_TIME, UI_TEXT_WIDGET_SMART_ISLAND_ACTION_QR
    };
    lv_obj_t *btn = NULL, *label = NULL, *arrow = NULL;

    g_si_ctx.objects.action_track = lv_obj_create(g_si_ctx.objects.page_action);
    lv_obj_remove_style_all(g_si_ctx.objects.action_track);
    lv_obj_set_size(g_si_ctx.objects.action_track,
                    (lv_coord_t)(SMART_ISLAND_W * g_si_ctx.action.page_count),
                    SMART_ISLAND_ACTION_EXPAND_H);
    lv_obj_set_style_bg_opa(g_si_ctx.objects.action_track, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(g_si_ctx.objects.action_track, LV_OBJ_FLAG_SCROLLABLE);
    smart_island_enable_gesture_on_obj(g_si_ctx.objects.action_track);

    for (uint8_t i = 0; i < SMART_ISLAND_ACTION_PAGE_COUNT; i++) {
        btn = lv_obj_create(g_si_ctx.objects.action_track);
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
        lv_obj_set_style_text_font(label, &lv_font_instrument_sans_medium_14, 0);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 16, 0);

        arrow = lv_label_create(btn);
        lv_label_set_text(arrow, ">");
        lv_obj_set_style_text_color(arrow, lv_color_hex(0x666666), 0);
        lv_obj_set_style_text_font(arrow, &lv_font_instrument_sans_medium_18, 0);
        lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -16, 0);

        g_si_ctx.objects.action_buttons[i] = btn;
        g_si_ctx.objects.action_labels[i] = label;
        g_si_ctx.objects.action_arrows[i] = arrow;
        g_si_ctx.action.ids[i] = default_ids[i];
        g_si_ctx.action.text_ids[i] = default_text_ids[i];
        g_si_ctx.action.texts[i][0] = '\0';
    }

    smart_island_action_page_set_count(SMART_ISLAND_ACTION_PAGE_DEFAULT_COUNT);
    smart_island_action_page_refresh_language_texts();
    smart_island_action_page_set_index(0U, false);
}
void smart_island_create(lv_obj_t *parent) 
{
    if (parent == NULL || !lv_obj_is_valid(parent)) return;

    if (g_si_ctx.lifecycle.created && g_si_ctx.objects.root && lv_obj_is_valid(g_si_ctx.objects.root)) {
        lv_obj_t *cur_parent = lv_obj_get_parent(g_si_ctx.objects.root);
        if (cur_parent != parent) {
            if (g_si_ctx.objects.modal && lv_obj_is_valid(g_si_ctx.objects.modal)) {
                lv_obj_set_parent(g_si_ctx.objects.modal, parent);
                lv_obj_set_pos(g_si_ctx.objects.modal, 0, 0);
            }
            lv_obj_set_parent(g_si_ctx.objects.root, parent);
            lv_obj_move_foreground(g_si_ctx.objects.root);
            smart_island_modal_update();
        }
        return;
    }

    if (g_si_ctx.lifecycle.created) {
        smart_island_destroy();
    }

    memset(&g_si_ctx.view.content, 0, sizeof(g_si_ctx.view.content));
    memset(g_si_ctx.warning.text, 0, sizeof(g_si_ctx.warning.text));
    memset(g_si_ctx.text.result, 0, sizeof(g_si_ctx.text.result));
    memset(g_si_ctx.text.info_extra, 0, sizeof(g_si_ctx.text.info_extra));
    memset(g_si_ctx.text.idle_line1, 0, sizeof(g_si_ctx.text.idle_line1));
    memset(g_si_ctx.text.idle_line2, 0, sizeof(g_si_ctx.text.idle_line2));
    memset(g_si_ctx.text.idle_line3, 0, sizeof(g_si_ctx.text.idle_line3));

    g_si_ctx.objects.modal = lv_obj_create(parent);
    lv_obj_remove_style_all(g_si_ctx.objects.modal);
    lv_obj_set_size(g_si_ctx.objects.modal, 1280, 400);
    lv_obj_set_style_bg_opa(g_si_ctx.objects.modal, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(g_si_ctx.objects.modal, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(g_si_ctx.objects.modal, smart_island_modal_click_cb, LV_EVENT_CLICKED, NULL);

    g_si_ctx.objects.root = lv_obj_create(parent);
    lv_obj_remove_style_all(g_si_ctx.objects.root);
    lv_obj_set_pos(g_si_ctx.objects.root, SMART_ISLAND_X, SMART_ISLAND_Y);
    lv_obj_set_size(g_si_ctx.objects.root, SMART_ISLAND_W, SMART_ISLAND_COMPACT_H);
    lv_obj_set_style_radius(g_si_ctx.objects.root, SMART_ISLAND_RADIUS, 0);
    lv_obj_set_style_bg_opa(g_si_ctx.objects.root, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_si_ctx.objects.root, lv_color_hex(SMART_ISLAND_BG_IDLE), 0);
    lv_obj_set_style_border_width(g_si_ctx.objects.root, 0, 0);
    lv_obj_set_style_outline_width(g_si_ctx.objects.root, 0, 0);
    lv_obj_set_style_shadow_width(g_si_ctx.objects.root, 0, 0);
    smart_island_enable_gesture_on_obj(g_si_ctx.objects.root);
    lv_obj_add_event_cb(g_si_ctx.objects.root, smart_island_click_cb, LV_EVENT_CLICKED, NULL);

    g_si_ctx.objects.dot = lv_obj_create(g_si_ctx.objects.root);
    lv_obj_remove_style_all(g_si_ctx.objects.dot);
    lv_obj_set_size(g_si_ctx.objects.dot, 8, 8);
    lv_obj_set_pos(g_si_ctx.objects.dot, 20, 18);
    lv_obj_set_style_radius(g_si_ctx.objects.dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(g_si_ctx.objects.dot, LV_OPA_COVER, 0);

    g_si_ctx.objects.title = lv_label_create(g_si_ctx.objects.root);
    lv_label_set_text(g_si_ctx.objects.title, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_READY_TITLE));
    lv_obj_set_width(g_si_ctx.objects.title, SMART_ISLAND_W - 36 - 14);
    lv_obj_set_pos(g_si_ctx.objects.title, 36, 13);
    lv_obj_set_style_text_font(g_si_ctx.objects.title, &lv_font_instrument_sans_semibold_14, 0);

    g_si_ctx.objects.subtitle = lv_label_create(g_si_ctx.objects.root);
    lv_obj_add_flag(g_si_ctx.objects.subtitle, LV_OBJ_FLAG_HIDDEN);

    g_si_ctx.objects.time = lv_label_create(g_si_ctx.objects.root);
    lv_label_set_text(g_si_ctx.objects.time, "00:00:00");
    lv_obj_set_style_text_font(g_si_ctx.objects.time, &lv_font_instrument_sans_medium_18, 0);
    lv_obj_set_style_text_color(g_si_ctx.objects.time, lv_color_hex(SMART_ISLAND_TEXT_LIGHT), 0);
    lv_obj_align(g_si_ctx.objects.time, LV_ALIGN_RIGHT_MID, -14, 0);
    lv_obj_clear_flag(g_si_ctx.objects.time, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_CLICK_FOCUSABLE);

    g_si_ctx.objects.badge = lv_obj_create(g_si_ctx.objects.root);
    lv_obj_remove_style_all(g_si_ctx.objects.badge);
    lv_obj_add_flag(g_si_ctx.objects.badge, LV_OBJ_FLAG_HIDDEN);

    g_si_ctx.objects.progress = lv_bar_create(g_si_ctx.objects.root);
    lv_obj_set_size(g_si_ctx.objects.progress, 160, 4);
    lv_obj_align(g_si_ctx.objects.progress, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_color(g_si_ctx.objects.progress, lv_color_hex(0x2E2E2E), LV_PART_MAIN);
    lv_obj_set_style_bg_color(g_si_ctx.objects.progress, lv_color_hex(0x00E676), LV_PART_INDICATOR);
    lv_obj_add_flag(g_si_ctx.objects.progress, LV_OBJ_FLAG_HIDDEN);

    g_si_ctx.objects.page_root = lv_obj_create(g_si_ctx.objects.root);
    lv_obj_remove_style_all(g_si_ctx.objects.page_root);
    lv_obj_set_size(g_si_ctx.objects.page_root, SMART_ISLAND_W, SMART_ISLAND_ACTION_EXPAND_H);
    smart_island_enable_gesture_on_obj(g_si_ctx.objects.page_root);
    lv_obj_add_flag(g_si_ctx.objects.page_root, LV_OBJ_FLAG_HIDDEN);

    g_si_ctx.objects.page_info = lv_obj_create(g_si_ctx.objects.page_root);
    lv_obj_remove_style_all(g_si_ctx.objects.page_info);
    lv_obj_set_size(g_si_ctx.objects.page_info, SMART_ISLAND_W, SMART_ISLAND_ACTION_EXPAND_H);
    smart_island_enable_gesture_on_obj(g_si_ctx.objects.page_info);

    g_si_ctx.objects.expand_title = lv_label_create(g_si_ctx.objects.page_info);
    lv_label_set_text(g_si_ctx.objects.expand_title, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_EXPAND_TITLE));
    lv_label_set_long_mode(g_si_ctx.objects.expand_title, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_si_ctx.objects.expand_title, SMART_ISLAND_W - 24 - 12);
    lv_obj_set_pos(g_si_ctx.objects.expand_title, 20, 18);
    lv_obj_set_style_text_font(g_si_ctx.objects.expand_title, &lv_font_instrument_sans_semibold_12, 0);
    lv_obj_set_style_text_color(g_si_ctx.objects.expand_title, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);
    lv_obj_add_flag(g_si_ctx.objects.expand_title, LV_OBJ_FLAG_HIDDEN);

    g_si_ctx.objects.expand_subtitle = lv_label_create(g_si_ctx.objects.page_info);
    lv_label_set_text(g_si_ctx.objects.expand_subtitle, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_EXPAND_SUBTITLE));
    lv_label_set_long_mode(g_si_ctx.objects.expand_subtitle, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_si_ctx.objects.expand_subtitle, 173);
    lv_obj_set_pos(g_si_ctx.objects.expand_subtitle, 68, 36);
    lv_obj_set_style_text_font(g_si_ctx.objects.expand_subtitle, &lv_font_instrument_sans_semibold_12, 0);
    lv_obj_set_style_text_color(g_si_ctx.objects.expand_subtitle, lv_color_hex(SMART_ISLAND_TEXT_LIGHT), 0);
    lv_obj_set_style_text_align(g_si_ctx.objects.expand_subtitle, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_recolor(g_si_ctx.objects.expand_subtitle, true);

    g_si_ctx.objects.expand_last = lv_label_create(g_si_ctx.objects.page_info);
    lv_label_set_text(g_si_ctx.objects.expand_last, ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_LAST_TAG));
    lv_label_set_long_mode(g_si_ctx.objects.expand_last, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_si_ctx.objects.expand_last, 40);
    lv_obj_set_pos(g_si_ctx.objects.expand_last, 20, 36);
    lv_obj_set_style_text_align(g_si_ctx.objects.expand_last, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_font(g_si_ctx.objects.expand_last, &lv_font_instrument_sans_medium_12, 0);
    lv_obj_set_style_text_color(g_si_ctx.objects.expand_last, lv_color_hex(SMART_ISLAND_LAST_TEXT_GRAY), 0);

    g_si_ctx.objects.expand_divider = lv_obj_create(g_si_ctx.objects.page_info);
    lv_obj_remove_style_all(g_si_ctx.objects.expand_divider);
    lv_obj_set_size(g_si_ctx.objects.expand_divider, SMART_ISLAND_W - 40, 1);
    lv_obj_set_pos(g_si_ctx.objects.expand_divider, 20, 56);
    lv_obj_set_style_bg_opa(g_si_ctx.objects.expand_divider, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_si_ctx.objects.expand_divider, lv_color_hex(0x515151), 0);

    g_si_ctx.objects.expand_footer = lv_label_create(g_si_ctx.objects.page_info);
    lv_label_set_text(g_si_ctx.objects.expand_footer, "");
    lv_label_set_long_mode(g_si_ctx.objects.expand_footer, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_si_ctx.objects.expand_footer, 150);
    lv_obj_set_pos(g_si_ctx.objects.expand_footer, 20, 62);
    lv_obj_set_style_text_font(g_si_ctx.objects.expand_footer, &lv_font_instrument_sans_medium_12, 0);
    lv_obj_set_style_text_color(g_si_ctx.objects.expand_footer, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);
    lv_obj_add_flag(g_si_ctx.objects.expand_footer, LV_OBJ_FLAG_HIDDEN);

    g_si_ctx.objects.expand_extra = lv_label_create(g_si_ctx.objects.page_info);
    lv_label_set_text(g_si_ctx.objects.expand_extra, "");
    lv_label_set_long_mode(g_si_ctx.objects.expand_extra, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(g_si_ctx.objects.expand_extra, 150);
    lv_obj_set_pos(g_si_ctx.objects.expand_extra, 20, 80);
    lv_obj_set_style_text_font(g_si_ctx.objects.expand_extra, &lv_font_instrument_sans_medium_12, 0);
    lv_obj_set_style_text_color(g_si_ctx.objects.expand_extra, lv_color_hex(SMART_ISLAND_TEXT_SUB), 0);
    lv_obj_add_flag(g_si_ctx.objects.expand_extra, LV_OBJ_FLAG_HIDDEN);

    g_si_ctx.objects.quality_bar_bg = lv_obj_create(g_si_ctx.objects.page_info);
    lv_obj_remove_style_all(g_si_ctx.objects.quality_bar_bg);
    lv_obj_set_size(g_si_ctx.objects.quality_bar_bg, 56, 8);
    lv_obj_set_pos(g_si_ctx.objects.quality_bar_bg, 166, 64);
    lv_obj_set_style_radius(g_si_ctx.objects.quality_bar_bg, 4, 0);
    lv_obj_set_style_bg_opa(g_si_ctx.objects.quality_bar_bg, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_si_ctx.objects.quality_bar_bg, lv_color_hex(SMART_ISLAND_RESULT_ISSUE_COLOR), 0);
    lv_obj_add_flag(g_si_ctx.objects.quality_bar_bg, LV_OBJ_FLAG_HIDDEN);

    g_si_ctx.objects.quality_bar_fg = lv_obj_create(g_si_ctx.objects.quality_bar_bg);
    lv_obj_remove_style_all(g_si_ctx.objects.quality_bar_fg);
    lv_obj_set_size(g_si_ctx.objects.quality_bar_fg, 56, 8);
    lv_obj_set_pos(g_si_ctx.objects.quality_bar_fg, 0, 0);
    lv_obj_set_style_radius(g_si_ctx.objects.quality_bar_fg, 4, 0);
    lv_obj_set_style_bg_opa(g_si_ctx.objects.quality_bar_fg, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_si_ctx.objects.quality_bar_fg, lv_color_hex(SMART_ISLAND_RESULT_OK_COLOR), 0);

    g_si_ctx.objects.quality_percent = lv_label_create(g_si_ctx.objects.page_info);
    lv_label_set_text(g_si_ctx.objects.quality_percent, "100%");
    lv_obj_set_pos(g_si_ctx.objects.quality_percent, 228, 62);
    lv_obj_set_style_text_font(g_si_ctx.objects.quality_percent, &lv_font_instrument_sans_medium_12, 0);
    lv_obj_set_style_text_color(g_si_ctx.objects.quality_percent, lv_color_hex(SMART_ISLAND_RESULT_OK_COLOR), 0);
    lv_obj_add_flag(g_si_ctx.objects.quality_percent, LV_OBJ_FLAG_HIDDEN);

    g_si_ctx.objects.page_action = lv_obj_create(g_si_ctx.objects.page_root);
    lv_obj_remove_style_all(g_si_ctx.objects.page_action);
    lv_obj_set_size(g_si_ctx.objects.page_action, SMART_ISLAND_W, SMART_ISLAND_ACTION_EXPAND_H);
    smart_island_enable_gesture_on_obj(g_si_ctx.objects.page_action);
    lv_obj_add_flag(g_si_ctx.objects.page_action, LV_OBJ_FLAG_HIDDEN);

    g_si_ctx.objects.page_indicator = lv_capsule_pagination_create(g_si_ctx.objects.root);
    if (g_si_ctx.objects.page_indicator && lv_obj_is_valid(g_si_ctx.objects.page_indicator)) {
        lv_obj_align(g_si_ctx.objects.page_indicator, LV_ALIGN_BOTTOM_MID, 0, SMART_ISLAND_PAGE_INDICATOR_Y);
        lv_obj_add_flag(g_si_ctx.objects.page_indicator, LV_OBJ_FLAG_HIDDEN);
    }
    smart_island_action_btn_create();

    g_si_ctx.view.scene = SMART_ISLAND_SCENE_IDLE;
    g_si_ctx.view.visual = SMART_ISLAND_VISUAL_COMPACT;
    g_si_ctx.view.page = SMART_ISLAND_PAGE_INFO;
    g_si_ctx.text.idle_quality_percent = 100;
    g_si_ctx.text.idle_has_issue = false;
    g_si_ctx.text.idle_has_data = false;
    g_si_ctx.text.idle_no_count = true;
    g_si_ctx.lifecycle.count_session_active = false;
    g_si_ctx.view.bg_current = SMART_ISLAND_BG_IDLE;
    g_si_ctx.view.bg_from = SMART_ISLAND_BG_IDLE;
    g_si_ctx.view.bg_to = SMART_ISLAND_BG_IDLE;
    g_si_ctx.view.bg_anim_running = false;
    g_si_ctx.lifecycle.created = true;
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
    smart_island_warning_marquee_stop();
    smart_island_pulse_stop();
    if (g_si_ctx.objects.root && lv_obj_is_valid(g_si_ctx.objects.root)) lv_obj_del(g_si_ctx.objects.root);
    if (g_si_ctx.objects.modal && lv_obj_is_valid(g_si_ctx.objects.modal)) lv_obj_del(g_si_ctx.objects.modal);
    smart_island_clear_object_refs();
    g_si_ctx.view.page_slide_dir = 0;
    g_si_ctx.view.anim_running = false;
    g_si_ctx.action.ignore_click_once = false;
    g_si_ctx.action.ignore_action_click_once = false;
    g_si_ctx.warning.marquee_running = false;
    g_si_ctx.warning.marquee_step = 0;
    g_si_ctx.warning.text_width_compact = 0;
    g_si_ctx.warning.text_width_expand = 0;
    g_si_ctx.view.swipe.pressed = false;
    g_si_ctx.view.swipe.swiped = false;
    g_si_ctx.view.swipe.start_pt.x = 0;
    g_si_ctx.view.swipe.start_pt.y = 0;
    smart_island_warning_fault_clear();
    g_si_ctx.view.bg_current = SMART_ISLAND_BG_IDLE;
    g_si_ctx.view.bg_from = SMART_ISLAND_BG_IDLE;
    g_si_ctx.view.bg_to = SMART_ISLAND_BG_IDLE;
    g_si_ctx.view.bg_anim_running = false;
    g_si_ctx.text.idle_line1[0] = '\0';
    g_si_ctx.text.idle_line2[0] = '\0';
    g_si_ctx.text.idle_line3[0] = '\0';
    g_si_ctx.text.info_extra[0] = '\0';
    g_si_ctx.text.idle_quality_percent = 100;
    g_si_ctx.text.idle_has_issue = false;
    g_si_ctx.text.idle_has_data = false;
    g_si_ctx.text.idle_no_count = true;
    g_si_ctx.lifecycle.count_session_active = false;

    g_si_ctx.lifecycle.created = false;
}

bool smart_island_is_attached_to(lv_obj_t *parent)
{
    if (parent == NULL || !lv_obj_is_valid(parent)) {
        return false;
    }

    if (g_si_ctx.objects.root == NULL || !lv_obj_is_valid(g_si_ctx.objects.root)) {
        return false;
    }

    return lv_obj_get_parent(g_si_ctx.objects.root) == parent;
}

void smart_island_refresh_time(void) 
{
    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_IDLE &&
        !(g_si_ctx.view.visual == SMART_ISLAND_VISUAL_EXPANDED &&
          g_si_ctx.view.page == SMART_ISLAND_PAGE_ACTION)) {
        smart_island_update_idle_time();
    }
}
void smart_island_set_visual(smart_island_visual_t visual, bool anim_en) 
{
    if ((unsigned int)visual > (unsigned int)SMART_ISLAND_VISUAL_EXPANDED) return;
    if (g_si_ctx.objects.root == NULL || !lv_obj_is_valid(g_si_ctx.objects.root)) return;

    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_RESULT && visual == SMART_ISLAND_VISUAL_EXPANDED) {
        visual = SMART_ISLAND_VISUAL_COMPACT;
    }

    g_si_ctx.view.visual = visual;
    if (anim_en) smart_island_visual_apply_anim(visual);
    else smart_island_visual_apply_now(visual);
    smart_island_update_pages_visible();
    smart_island_modal_update();
    smart_island_apply_scene_style();
}

void smart_island_set_scene(smart_island_scene_t scene, const char *title, const char *subtitle) 
{
    if ((unsigned int)scene > (unsigned int)SMART_ISLAND_SCENE_QR) return;

    g_si_ctx.view.scene = scene;
    smart_island_stop_result_timer();

    if (title && title[0] != '\0') lv_snprintf(g_si_ctx.view.content.title, sizeof(g_si_ctx.view.content.title), "%s", title);
    else g_si_ctx.view.content.title[0] = '\0';

    if (subtitle && subtitle[0] != '\0') lv_snprintf(g_si_ctx.view.content.subtitle, sizeof(g_si_ctx.view.content.subtitle), "%s", subtitle);
    else g_si_ctx.view.content.subtitle[0] = '\0';

    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_notify_count_start(void)
{
    /* 新会话开始前先清掉上一轮残留的结束动画状态 */
    ui_count_end_anim_cancel();

    g_si_ctx.lifecycle.count_session_active = true;
    g_si_ctx.warning.level = SMART_ISLAND_WARNING_LEVEL_WARNING;
    smart_island_warning_fault_clear();
    smart_island_warning_marquee_stop();
    g_si_ctx.text.result[0] = '\0';
    smart_island_set_scene(SMART_ISLAND_SCENE_COUNTING, NULL, NULL);
    smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, true);
}

void smart_island_notify_count_end(const char *result_text)
{
    g_si_ctx.lifecycle.count_session_active = false;
    if (result_text && result_text[0] != '\0') {
        lv_snprintf(g_si_ctx.text.result, sizeof(g_si_ctx.text.result), "%s", result_text);
    } else {
        g_si_ctx.text.result[0] = '\0';
    }
    smart_island_set_scene(SMART_ISLAND_SCENE_RESULT, NULL, NULL);
    smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, true);
    smart_island_stop_result_timer();
    g_si_ctx.lifecycle.result_timer = lv_timer_create(smart_island_result_timer_cb, SMART_ISLAND_RESULT_HOLD_MS, NULL);
    if (g_si_ctx.lifecycle.result_timer == NULL) {
        smart_island_restore_idle();
    }
}

void smart_island_set_count_analysis(int valid_pcs, int suspect_pcs, int damaged_pcs)
{
    g_si_ctx.text.analysis_valid_pcs = valid_pcs > 0 ? valid_pcs : 0;
    g_si_ctx.text.analysis_suspect_pcs = suspect_pcs > 0 ? suspect_pcs : 0;
    g_si_ctx.text.analysis_damaged_pcs = damaged_pcs > 0 ? damaged_pcs : 0;
    g_si_ctx.text.analysis_valid = true;
    smart_island_refresh_summary();
}

void smart_island_clear_count_analysis(void)
{
    g_si_ctx.text.analysis_valid = false;
    g_si_ctx.text.analysis_valid_pcs = 0;
    g_si_ctx.text.analysis_suspect_pcs = 0;
    g_si_ctx.text.analysis_damaged_pcs = 0;
}

void smart_island_notify_warning_level(const char *warn_text, smart_island_warning_level_t level)
{
    char next_warning_text[sizeof(g_si_ctx.warning.text)];

    if ((unsigned int)level > (unsigned int)SMART_ISLAND_WARNING_LEVEL_ERROR) {
        return;
    }

    if (warn_text && warn_text[0] != '\0') {
        lv_snprintf(next_warning_text, sizeof(next_warning_text), "%s", warn_text);
    } else {
        lv_snprintf(next_warning_text, sizeof(next_warning_text), "%s",
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR));
    }

    /* 同一条异常重复上报时，不重启 warning 动画，避免 ESC/CLEAR/START 触发“重新闪烁” */
    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_WARNING &&
        g_si_ctx.warning.level == level &&
        strcmp(g_si_ctx.warning.text, next_warning_text) == 0) {
        if (!g_si_ctx.warning.marquee_running) {
            smart_island_warning_apply_static_layout();
            if (!fault_popup_is_showing()) {
                smart_island_warning_marquee_start();
            }
        }
        return;
    }

    g_si_ctx.warning.level = level;
    smart_island_warning_fault_capture();

    lv_snprintf(g_si_ctx.warning.text, sizeof(g_si_ctx.warning.text), "%s",
        next_warning_text);

    smart_island_set_scene(
        SMART_ISLAND_SCENE_WARNING,
        g_si_ctx.warning.text,
        NULL
    );

    g_si_ctx.view.page = SMART_ISLAND_PAGE_INFO;
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
    if (progress > 100U) progress = 100U;
    g_si_ctx.view.content.progress = progress;
    smart_island_set_scene(SMART_ISLAND_SCENE_UPDATE,
        text,
        ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_SUBTITLE));
    if (g_si_ctx.objects.progress && lv_obj_is_valid(g_si_ctx.objects.progress)) {
        lv_obj_clear_flag(g_si_ctx.objects.progress, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_value(g_si_ctx.objects.progress, progress, LV_ANIM_ON);
    }
    smart_island_open_info_page();
}

void smart_island_notify_qr(const char *text) 
{
    smart_island_set_scene(SMART_ISLAND_SCENE_QR,
        text,
        ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_QR_INFO_SUBTITLE));
    if (g_si_ctx.objects.progress && lv_obj_is_valid(g_si_ctx.objects.progress)) lv_obj_add_flag(g_si_ctx.objects.progress, LV_OBJ_FLAG_HIDDEN);
    smart_island_open_info_page();
}

void smart_island_restore_idle(void) 
{
    g_si_ctx.lifecycle.count_session_active = false;
    smart_island_warning_marquee_stop();
    g_si_ctx.warning.level = SMART_ISLAND_WARNING_LEVEL_WARNING;
    smart_island_warning_fault_clear();
    if (g_si_ctx.objects.progress && lv_obj_is_valid(g_si_ctx.objects.progress)) {
        lv_obj_add_flag(g_si_ctx.objects.progress, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_value(g_si_ctx.objects.progress, 0, LV_ANIM_OFF);
    }
    g_si_ctx.warning.text[0] = '\0';
    g_si_ctx.text.result[0] = '\0';
    smart_island_set_scene(SMART_ISLAND_SCENE_IDLE, NULL, NULL);
    smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, true);
    smart_island_update_idle_time();
    smart_island_reset_compact_header_position();
    smart_island_reset_time_position();
}

bool smart_island_is_expanded(void) { return g_si_ctx.view.visual == SMART_ISLAND_VISUAL_EXPANDED; }

void smart_island_set_page(smart_island_page_t page, bool anim_en) 
{
    if ((unsigned int)page > (unsigned int)SMART_ISLAND_PAGE_ACTION) return;

    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_RESULT) {
        g_si_ctx.view.page = SMART_ISLAND_PAGE_INFO;
        smart_island_set_visual(SMART_ISLAND_VISUAL_COMPACT, false);
        return;
    }

    if (anim_en) smart_island_page_apply_anim(page);
    else smart_island_page_apply_now(page);
    smart_island_page_indicator_sync(anim_en);
}

smart_island_page_t smart_island_get_page(void) { return g_si_ctx.view.page; }

void smart_island_register_action_cb(smart_island_action_cb_t cb) { g_si_ctx.action.callback = cb; }

void smart_island_refresh_language_texts(void)
{
    smart_island_action_page_refresh_language_texts();
    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_refresh_summary(void)
{
    /* Warning 场景中避免外部刷新重刷样式，防止 ESC/CLEAR 造成文本“重新闪烁” */
    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_WARNING) {
        return;
    }

    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_set_idle_info_line1(const char *text)
{
    smart_island_apply_idle_line_text(g_si_ctx.text.idle_line1, sizeof(g_si_ctx.text.idle_line1), text);
    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_set_idle_info_line2(const char *text)
{
    smart_island_apply_idle_line_text(g_si_ctx.text.idle_line2, sizeof(g_si_ctx.text.idle_line2), text);
    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_set_idle_info_line3(const char *text)
{
    smart_island_apply_idle_line_text(g_si_ctx.text.idle_line3, sizeof(g_si_ctx.text.idle_line3), text);
    smart_island_rebuild_scene_texts();
    smart_island_apply_scene_style();
}

void smart_island_close(void) 
{
    g_si_ctx.view.page = SMART_ISLAND_PAGE_INFO;
    smart_island_restore_idle();
    smart_island_action_page_set_index(0U, false);
    smart_island_page_indicator_sync(false);
    smart_island_reset_page_positions();
    smart_island_reset_compact_header_position();
    smart_island_reset_time_position();
    g_si_ctx.action.ignore_click_once = false;
    g_si_ctx.action.ignore_action_click_once = false;
}

void smart_island_open_info_page(void) 
{
    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_RESULT) {
        return;
    }

    smart_island_expand_if_needed(true);
    smart_island_set_page(SMART_ISLAND_PAGE_INFO, true);
}

void smart_island_open_action_page(void) 
{
    if (g_si_ctx.view.scene == SMART_ISLAND_SCENE_RESULT) {
        return;
    }

    smart_island_action_page_set_index(0U, false);
    smart_island_expand_if_needed(true);
    smart_island_set_page(SMART_ISLAND_PAGE_ACTION, true);
}

void smart_island_set_pure_count_enabled(bool enabled)
{
    g_si_ctx.lifecycle.pure_count_enabled = enabled;
}

bool smart_island_pure_count_is_enabled(void)
{
    return g_si_ctx.lifecycle.pure_count_enabled;
}
