#include "page_18_pure.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/lv_page_event.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/ui_lang.h"
#include "un260/lv_system/ui_text.h"
#include <stdio.h>
#include <string.h>

LV_FONT_DECLARE(MONTSERRAT_EXTRABOLD_35);
LV_FONT_DECLARE(MONTSERRAT_Medium_15);
LV_FONT_DECLARE(lv_font_rajdhani_194);
LV_FONT_DECLARE(lv_font_rajdhani_142);
LV_FONT_DECLARE(lv_font_rajdhani_27);

static lv_obj_t* pure_amount_value = NULL;
static lv_obj_t* pure_pcs_value = NULL;
static lv_obj_t* pure_reject_value = NULL;
static lv_obj_t* pure_label_amount = NULL;
static lv_obj_t* pure_label_total_top = NULL;
static lv_obj_t* pure_label_value = NULL;
static lv_obj_t* pure_label_pcs = NULL;
static lv_obj_t* pure_label_total_bottom = NULL;
static lv_obj_t* pure_label_pieces = NULL;
static lv_obj_t* pure_label_reject = NULL;
static lv_obj_t* pure_btn_start_label = NULL;
static lv_obj_t* pure_btn_clear_label = NULL;
static lv_timer_t* pure_refresh_timer = NULL;
static bool pure_page_exiting = false;
static language_t pure_lang_cache = LANGUAGE_EN;

static void pure_btn_touch_feedback_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* btn = lv_event_get_target(e);
    uint32_t i;
    uint32_t child_cnt;
    lv_opa_t text_opa;

    if (btn == NULL || !lv_obj_is_valid(btn)) {
        return;
    }

    if (code == LV_EVENT_PRESSED) {
        lv_obj_set_style_bg_opa(btn, LV_OPA_70, 0);
        text_opa = LV_OPA_70;
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        text_opa = LV_OPA_COVER;
    } else {
        return;
    }

    child_cnt = lv_obj_get_child_cnt(btn);
    for (i = 0; i < child_cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(btn, i);
        if (child && lv_obj_is_valid(child)) {
            lv_obj_set_style_text_opa(child, text_opa, 0);
        }
    }
}

static void pure_format_amount(char* buf, size_t size, float amount)
{
    int len;
    int dest_index;

    if (buf == NULL || size == 0U) {
        return;
    }

    if (amount < 0.0f) {
        amount = 0.0f;
    }

    snprintf(buf, size, "%.0f", amount);

    len = (int)strlen(buf);
    if (len <= 3) {
        return;
    }

    {
        char temp[32];
        int i;

        strncpy(temp, buf, sizeof(temp) - 1);
        temp[sizeof(temp) - 1] = '\0';

        dest_index = 0;
        for (i = 0; i < len && dest_index < (int)size - 1; i++) {
            buf[dest_index++] = temp[i];
            if (i < len - 1 && ((len - i - 1) % 3) == 0 && dest_index < (int)size - 1) {
                buf[dest_index++] = ',';
            }
        }
        buf[dest_index] = '\0';
    }
}

static void pure_format_pcs(char* buf, size_t size, int pcs)
{
    int len;
    int dest_index;

    if (buf == NULL || size == 0U) {
        return;
    }

    if (pcs < 0) {
        pcs = 0;
    }

    snprintf(buf, size, "%d", pcs);

    len = (int)strlen(buf);
    if (len <= 3) {
        return;
    }

    {
        char temp[32];
        int i;

        strncpy(temp, buf, sizeof(temp) - 1);
        temp[sizeof(temp) - 1] = '\0';

        dest_index = 0;
        for (i = 0; i < len && dest_index < (int)size - 1; i++) {
            buf[dest_index++] = temp[i];
            if (i < len - 1 && ((len - i - 1) % 3) == 0 && dest_index < (int)size - 1) {
                buf[dest_index++] = ',';
            }
        }
        buf[dest_index] = '\0';
    }
}

static void pure_refresh_values(void)
{
    char amount_buf[32];
    char pcs_buf[32];
    int reject_cnt = (sim.err_expected > sim.err_num) ? sim.err_expected : sim.err_num;

    if (pure_amount_value && lv_obj_is_valid(pure_amount_value)) {
        pure_format_amount(amount_buf, sizeof(amount_buf), sim.total_amount);
        lv_label_set_text(pure_amount_value, amount_buf);
    }

    if (pure_pcs_value && lv_obj_is_valid(pure_pcs_value)) {
        pure_format_pcs(pcs_buf, sizeof(pcs_buf), sim.total_pcs);
        lv_label_set_text(pure_pcs_value, pcs_buf);
    }

    if (pure_reject_value && lv_obj_is_valid(pure_reject_value)) {
        lv_label_set_text_fmt(pure_reject_value, "%d", reject_cnt);
    }
}

static void pure_refresh_language_texts(void)
{
    if (pure_label_amount && lv_obj_is_valid(pure_label_amount)) {
        lv_label_set_text(pure_label_amount, ui_text_get(UI_TEXT_WIDGET_PURE_AMOUNT));
    }
    if (pure_label_total_top && lv_obj_is_valid(pure_label_total_top)) {
        lv_label_set_text(pure_label_total_top, ui_text_get(UI_TEXT_WIDGET_PURE_TOTAL));
    }
    if (pure_label_value && lv_obj_is_valid(pure_label_value)) {
        lv_label_set_text(pure_label_value, ui_text_get(UI_TEXT_WIDGET_PURE_VALUE));
    }
    if (pure_label_pcs && lv_obj_is_valid(pure_label_pcs)) {
        lv_label_set_text(pure_label_pcs, ui_text_get(UI_TEXT_WIDGET_PURE_PCS));
    }
    if (pure_label_total_bottom && lv_obj_is_valid(pure_label_total_bottom)) {
        lv_label_set_text(pure_label_total_bottom, ui_text_get(UI_TEXT_WIDGET_PURE_TOTAL));
    }
    if (pure_label_pieces && lv_obj_is_valid(pure_label_pieces)) {
        lv_label_set_text(pure_label_pieces, ui_text_get(UI_TEXT_WIDGET_PURE_PIECES));
    }
    if (pure_label_reject && lv_obj_is_valid(pure_label_reject)) {
        lv_label_set_text(pure_label_reject, ui_text_get(UI_TEXT_WIDGET_PURE_REJECT));
    }
    if (pure_btn_start_label && lv_obj_is_valid(pure_btn_start_label)) {
        lv_label_set_text(pure_btn_start_label, ui_text_get(UI_TEXT_WIDGET_PURE_START));
    }
    if (pure_btn_clear_label && lv_obj_is_valid(pure_btn_clear_label)) {
        lv_label_set_text(pure_btn_clear_label, ui_text_get(UI_TEXT_WIDGET_PURE_CLEAR));
    }
}

static void pure_refresh_timer_cb(lv_timer_t* t)
{
    (void)t;
    if (pure_lang_cache != ui_lang_get()) {
        pure_lang_cache = ui_lang_get();
        pure_refresh_language_texts();
    }
    pure_refresh_values();
}

void ui_page_18_pure_create(lv_obj_t* parent)
{
    lv_obj_t* bg = NULL;
    lv_obj_t* btn_start = NULL;
    lv_obj_t* btn_clear = NULL;
    lv_coord_t reject_num_h = 0;

    if (parent == NULL) {
        parent = lv_scr_act();
    }

    if (pure_page && lv_obj_is_valid(pure_page)) {
        return;
    }

    pure_page_exiting = false;

    pure_page = lv_obj_create(parent);
    lv_obj_remove_style_all(pure_page);
    lv_obj_set_size(pure_page, 1280, 400);
    lv_obj_set_pos(pure_page, 0, 0);
    lv_obj_clear_flag(pure_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(pure_page, lv_color_hex(0xFFFFFF), 0);

    bg = lv_img_create(pure_page);
    lv_img_set_src(bg, "L:/usr/local/share/lvgl_data/page_pure.png");
    lv_obj_set_pos(bg, 0, 0);

    pure_label_amount = lv_label_create(pure_page);
    lv_label_set_text(pure_label_amount, ui_text_get(UI_TEXT_WIDGET_PURE_AMOUNT));
    lv_obj_set_pos(pure_label_amount, 68, 75);
    lv_obj_set_style_text_font(pure_label_amount, &MONTSERRAT_EXTRABOLD_35, 0);
    lv_obj_set_style_text_color(pure_label_amount, lv_color_hex(0x636363), 0);

    pure_label_total_top = lv_label_create(pure_page);
    lv_label_set_text(pure_label_total_top, ui_text_get(UI_TEXT_WIDGET_PURE_TOTAL));
    lv_obj_set_pos(pure_label_total_top, 70, 114);
    lv_obj_set_style_text_font(pure_label_total_top, &MONTSERRAT_Medium_15, 0);
    lv_obj_set_style_text_color(pure_label_total_top, lv_color_hex(0x505050), 0);

    pure_label_value = lv_label_create(pure_page);
    lv_label_set_text(pure_label_value, ui_text_get(UI_TEXT_WIDGET_PURE_VALUE));
    lv_obj_set_pos(pure_label_value, 141, 114);
    lv_obj_set_style_text_font(pure_label_value, &MONTSERRAT_Medium_15, 0);
    lv_obj_set_style_text_color(pure_label_value, lv_color_hex(0x505050), 0);

    pure_amount_value = lv_label_create(pure_page);
    lv_label_set_text(pure_amount_value, "0");
    lv_obj_set_pos(pure_amount_value, 256, 66);
    lv_obj_set_size(pure_amount_value, 935, 125);
    lv_obj_set_style_text_align(pure_amount_value, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_font(pure_amount_value, &lv_font_rajdhani_194, 0);
    lv_obj_set_style_text_color(pure_amount_value, lv_color_hex(0x000000), 0);

    pure_label_pcs = lv_label_create(pure_page);
    lv_label_set_text(pure_label_pcs, ui_text_get(UI_TEXT_WIDGET_PURE_PCS));
    lv_obj_set_pos(pure_label_pcs, 68, 248);
    lv_obj_set_style_text_font(pure_label_pcs, &MONTSERRAT_EXTRABOLD_35, 0);
    lv_obj_set_style_text_color(pure_label_pcs, lv_color_hex(0x636363), 0);

    pure_label_total_bottom = lv_label_create(pure_page);
    lv_label_set_text(pure_label_total_bottom, ui_text_get(UI_TEXT_WIDGET_PURE_TOTAL));
    lv_obj_set_pos(pure_label_total_bottom, 70, 287);
    lv_obj_set_style_text_font(pure_label_total_bottom, &MONTSERRAT_Medium_15, 0);
    lv_obj_set_style_text_color(pure_label_total_bottom, lv_color_hex(0x505050), 0);

    pure_label_pieces = lv_label_create(pure_page);
    lv_label_set_text(pure_label_pieces, ui_text_get(UI_TEXT_WIDGET_PURE_PIECES));
    lv_obj_set_pos(pure_label_pieces, 141, 287);
    lv_obj_set_style_text_font(pure_label_pieces, &MONTSERRAT_Medium_15, 0);
    lv_obj_set_style_text_color(pure_label_pieces, lv_color_hex(0x505050), 0);

    pure_pcs_value = lv_label_create(pure_page);
    lv_label_set_text(pure_pcs_value, "0");
    lv_obj_set_pos(pure_pcs_value, 495, 250);
    lv_obj_set_size(pure_pcs_value, 684, 91);
    lv_obj_set_style_text_align(pure_pcs_value, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_font(pure_pcs_value, &lv_font_rajdhani_142, 0);
    lv_obj_set_style_text_color(pure_pcs_value, lv_color_hex(0x000000), 0);

    pure_label_reject = lv_label_create(pure_page);
    lv_label_set_text(pure_label_reject, ui_text_get(UI_TEXT_WIDGET_PURE_REJECT));
    lv_obj_set_pos(pure_label_reject, 70, 361);
    lv_obj_set_style_text_font(pure_label_reject, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(pure_label_reject, lv_color_hex(0xFF7171), 0);

    pure_reject_value = lv_label_create(pure_page);
    lv_label_set_text(pure_reject_value, "0");
    reject_num_h = lv_font_get_line_height(&lv_font_montserrat_18);
    lv_obj_set_size(pure_reject_value, 50, reject_num_h);
    lv_obj_set_pos(pure_reject_value, 183, 361);
    lv_obj_set_style_text_align(pure_reject_value, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_font(pure_reject_value, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(pure_reject_value, lv_color_hex(0xFF7171), 0);

    btn_start = lv_btn_create(pure_page);
    lv_obj_set_pos(btn_start, 1105, 361);
    lv_obj_set_size(btn_start, 131, 36);
    lv_obj_clear_flag(btn_start, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(btn_start, 8, 0);
    lv_obj_set_style_bg_color(btn_start, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(btn_start, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_start, 2, 0);
    lv_obj_set_style_border_color(btn_start, lv_color_hex(0x818181), 0);
    lv_obj_set_style_shadow_width(btn_start, 0, 0);
    lv_obj_add_event_cb(btn_start, page_01_start_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_start, pure_btn_touch_feedback_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(btn_start, pure_btn_touch_feedback_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(btn_start, pure_btn_touch_feedback_cb, LV_EVENT_PRESS_LOST, NULL);

    pure_btn_start_label = lv_label_create(btn_start);
    lv_label_set_text(pure_btn_start_label, ui_text_get(UI_TEXT_WIDGET_PURE_START));
    lv_obj_set_style_text_font(pure_btn_start_label, &lv_font_rajdhani_27, 0);
    lv_obj_set_style_text_color(pure_btn_start_label, lv_color_hex(0x818181), 0);
    lv_obj_center(pure_btn_start_label);

    btn_clear = lv_btn_create(pure_page);
    lv_obj_set_pos(btn_clear, 961, 361);
    lv_obj_set_size(btn_clear, 131, 36);
    lv_obj_clear_flag(btn_clear, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(btn_clear, 8, 0);
    lv_obj_set_style_bg_color(btn_clear, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(btn_clear, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_clear, 0, 0);
    lv_obj_set_style_shadow_width(btn_clear, 0, 0);
    lv_obj_add_event_cb(btn_clear, page_01_esc_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_clear, pure_btn_touch_feedback_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(btn_clear, pure_btn_touch_feedback_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(btn_clear, pure_btn_touch_feedback_cb, LV_EVENT_PRESS_LOST, NULL);

    pure_btn_clear_label = lv_label_create(btn_clear);
    lv_label_set_text(pure_btn_clear_label, ui_text_get(UI_TEXT_WIDGET_PURE_CLEAR));
    lv_obj_set_style_text_font(pure_btn_clear_label, &lv_font_rajdhani_27, 0);
    lv_obj_set_style_text_color(pure_btn_clear_label, lv_color_hex(0x818181), 0);
    lv_obj_center(pure_btn_clear_label);

    pure_lang_cache = ui_lang_get();
    pure_refresh_language_texts();

    pure_refresh_values();

    if (pure_refresh_timer == NULL) {
        pure_refresh_timer = lv_timer_create(pure_refresh_timer_cb, 100, NULL);
    }

    smart_island_create(pure_page);
}

void ui_page_18_pure_request_exit(void)
{
    if (pure_page == NULL || !lv_obj_is_valid(pure_page)) {
        ui_manager_switch(UI_PAGE_MAIN);
        return;
    }

    if (pure_page_exiting) {
        return;
    }

    pure_page_exiting = true;
    ui_manager_switch(UI_PAGE_MAIN);
    pure_page_exiting = false;
}

void ui_page_18_pure_destroy(void)
{
    if (pure_refresh_timer) {
        lv_timer_del(pure_refresh_timer);
        pure_refresh_timer = NULL;
    }

    smart_island_destroy();

    if (pure_page && lv_obj_is_valid(pure_page)) {
        lv_obj_del(pure_page);
    }

    pure_page = NULL;
    pure_amount_value = NULL;
    pure_pcs_value = NULL;
    pure_reject_value = NULL;
    pure_label_amount = NULL;
    pure_label_total_top = NULL;
    pure_label_value = NULL;
    pure_label_pcs = NULL;
    pure_label_total_bottom = NULL;
    pure_label_pieces = NULL;
    pure_label_reject = NULL;
    pure_btn_start_label = NULL;
    pure_btn_clear_label = NULL;
    pure_lang_cache = LANGUAGE_EN;
    pure_page_exiting = false;
}
