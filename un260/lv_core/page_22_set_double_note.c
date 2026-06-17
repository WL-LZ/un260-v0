#include "page_22_set_double_note.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/settings_detail_ui.h"
#include "un260/lv_system/ui_text.h"
#include "un260/lv_system/user_cfg.h"

#include <stddef.h>
#include <stdint.h>

#define DOUBLE_NOTE_ITEM_W 326
#define DOUBLE_NOTE_ITEM_H 66
#define DOUBLE_NOTE_ITEM_GAP_X 30
#define DOUBLE_NOTE_ITEM_GAP_Y 10
#define DOUBLE_NOTE_OPTION_COUNT 3

typedef struct {
    uint8_t level;
    ui_text_id_t title_text;
    ui_text_id_t desc_text;
    uint32_t color_hex;
} double_note_option_t;

typedef struct {
    lv_obj_t* card;
    lv_obj_t* check;
} double_note_item_t;

static const double_note_option_t g_double_note_options[DOUBLE_NOTE_OPTION_COUNT] = {
    { 1, UI_TEXT_SETTINGS_DOUBLE_NOTE_LEVEL_1, UI_TEXT_SETTINGS_DOUBLE_NOTE_LEVEL_LOW, 0x24B47E },
    { 2, UI_TEXT_SETTINGS_DOUBLE_NOTE_LEVEL_2, UI_TEXT_SETTINGS_DOUBLE_NOTE_LEVEL_MID, 0xF59D2A },
    { 3, UI_TEXT_SETTINGS_DOUBLE_NOTE_LEVEL_3, UI_TEXT_SETTINGS_DOUBLE_NOTE_LEVEL_HIGH, 0xF04444 },
};

static lv_obj_t* double_note_page = NULL;
static lv_obj_t* preview_level_label = NULL;
static lv_obj_t* preview_desc_label = NULL;
static lv_obj_t* preview_bars[DOUBLE_NOTE_OPTION_COUNT] = { NULL };
static lv_obj_t* preview_bar_labels[DOUBLE_NOTE_OPTION_COUNT] = { NULL };
static double_note_item_t g_double_note_items[DOUBLE_NOTE_OPTION_COUNT] = { 0 };
static uint8_t pending_level = 0;
static uint8_t pending_prev_level = DOUBLE_NOTE_LEVEL_MIN;

static uint8_t double_note_normalize_level(uint8_t level)
{
    if (level < DOUBLE_NOTE_LEVEL_MIN || level > DOUBLE_NOTE_LEVEL_MAX) {
        return DOUBLE_NOTE_LEVEL_MIN;
    }

    return level;
}

static uint8_t double_note_get_level(void)
{
    return double_note_normalize_level(Machine_para.double_note_level);
}

static const double_note_option_t* double_note_find_option(uint8_t level)
{
    uint8_t normalized = double_note_normalize_level(level);

    for (size_t i = 0; i < DOUBLE_NOTE_OPTION_COUNT; i++) {
        if (g_double_note_options[i].level == normalized) {
            return &g_double_note_options[i];
        }
    }

    return &g_double_note_options[0];
}

static int double_note_option_index(uint8_t level)
{
    uint8_t normalized = double_note_normalize_level(level);

    for (size_t i = 0; i < DOUBLE_NOTE_OPTION_COUNT; i++) {
        if (g_double_note_options[i].level == normalized) {
            return (int)i;
        }
    }

    return 0;
}

static void double_note_refresh_view(void)
{
    uint8_t level = double_note_get_level();
    const double_note_option_t* option = double_note_find_option(level);
    int selected_index = double_note_option_index(level);
    lv_color_t active_color = lv_color_hex(option->color_hex);

    for (size_t i = 0; i < DOUBLE_NOTE_OPTION_COUNT; i++) {
        bool selected = ((int)i == selected_index);
        lv_obj_t* card = g_double_note_items[i].card;

        settings_detail_set_select_box_checked(g_double_note_items[i].check, selected);
        settings_detail_set_select_box_active(g_double_note_items[i].check, selected);

        if (card) {
            lv_obj_set_style_bg_color(card,
                                      selected ? lv_color_hex(0xF2FBFF) : lv_color_hex(0xFFFFFF),
                                      0);
            lv_obj_set_style_border_color(card,
                                          selected ? lv_color_hex(0x0878C8) : lv_color_hex(0xDDE6EF),
                                          0);
            lv_obj_set_style_border_width(card, 2, 0);
        }

        if (preview_bars[i]) {
            bool enabled = (g_double_note_options[i].level <= level);
            lv_obj_set_style_bg_color(preview_bars[i],
                                      enabled ? active_color : lv_color_hex(0xE9EDF2),
                                      0);
        }

        if (preview_bar_labels[i]) {
            lv_obj_set_style_text_color(preview_bar_labels[i],
                                        g_double_note_options[i].level == level ?
                                        active_color : lv_color_hex(0x7686A5),
                                        0);
        }
    }

    if (preview_level_label) {
        lv_label_set_text_fmt(preview_level_label, "%u", (unsigned)level);
        lv_obj_set_style_text_color(preview_level_label, active_color, 0);
    }

    if (preview_desc_label) {
        lv_label_set_text(preview_desc_label, ui_text_get(option->desc_text));
        lv_obj_set_style_text_color(preview_desc_label, active_color, 0);
    }
}

static bool double_note_send_level(uint8_t level)
{
    uint8_t payload = double_note_normalize_level(level);

    if (!settings_detail_send_command(0x31, &payload, 1)) {
        return false;
    }

    return true;
}

static void double_note_esc_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_pop_page();
}

static void double_note_option_cb(lv_event_t* e)
{
    uint8_t level;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    level = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    pending_prev_level = double_note_get_level();
    if (!double_note_send_level(level)) {
        pending_level = 0;
        return;
    }

    pending_level = double_note_normalize_level(level);
    Machine_para.double_note_level = pending_level;
    double_note_refresh_view();
}

static lv_obj_t* double_note_create_card(lv_obj_t* parent, lv_coord_t x, lv_coord_t y,
                                         lv_coord_t w, lv_coord_t h)
{
    lv_obj_t* card = settings_detail_create_card(parent, x, y, w, h);
    lv_obj_set_style_shadow_width(card, 10, 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_10, 0);
    return card;
}

static void double_note_create_option(lv_obj_t* parent, size_t index,
                                      const double_note_option_t* option)
{
    lv_coord_t col = (lv_coord_t)(index % 2);
    lv_coord_t row = (lv_coord_t)(index / 2);
    lv_coord_t x = (lv_coord_t)(col * (DOUBLE_NOTE_ITEM_W + DOUBLE_NOTE_ITEM_GAP_X));
    lv_coord_t y = (lv_coord_t)(row * (DOUBLE_NOTE_ITEM_H + DOUBLE_NOTE_ITEM_GAP_Y));
    lv_color_t item_color = lv_color_hex(option->color_hex);
    lv_obj_t* item = lv_obj_create(parent);

    lv_obj_remove_style_all(item);
    lv_obj_set_pos(item, x, y);
    lv_obj_set_size(item, DOUBLE_NOTE_ITEM_W, DOUBLE_NOTE_ITEM_H);
    lv_obj_set_style_bg_color(item, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(item, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(item, 2, 0);
    lv_obj_set_style_border_color(item, lv_color_hex(0xDDE6EF), 0);
    lv_obj_set_style_radius(item, 6, 0);
    lv_obj_set_style_translate_y(item, 0, LV_STATE_PRESSED);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(item, double_note_option_cb, LV_EVENT_CLICKED,
                        (void*)(uintptr_t)option->level);

    lv_obj_t* badge = lv_obj_create(item);
    lv_obj_remove_style_all(badge);
    lv_obj_set_pos(badge, 22, 12);
    lv_obj_set_size(badge, 42, 42);
    lv_obj_set_style_bg_color(badge, lv_color_hex(0xF6FBFF), 0);
    lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(badge, 1, 0);
    lv_obj_set_style_border_color(badge, item_color, 0);
    lv_obj_set_style_radius(badge, 6, 0);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* badge_label = settings_detail_create_label(badge, "", &lv_font_montserrat_20,
                                                         item_color, 0, 0);
    lv_label_set_text_fmt(badge_label, "%u", (unsigned)option->level);
    lv_obj_center(badge_label);

    settings_detail_create_label(item, ui_text_get(option->title_text), &lv_font_montserrat_16,
                                 lv_color_hex(0x2D3440), 86, 24);

    g_double_note_items[index].card = item;
    g_double_note_items[index].check = settings_detail_create_select_box(item, 278, 18, 30,
                                                                         double_note_option_cb,
                                                                         (void*)(uintptr_t)option->level);
    lv_obj_set_style_translate_y(g_double_note_items[index].check, 0, LV_STATE_PRESSED);
}

static void double_note_create_list(lv_obj_t* parent)
{
    lv_obj_t* list_area = lv_obj_create(parent);

    lv_obj_remove_style_all(list_area);
    lv_obj_set_pos(list_area, 24, 64);
    lv_obj_set_size(list_area, 682, 220);
    lv_obj_set_style_bg_opa(list_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list_area, 0, 0);
    lv_obj_set_scroll_dir(list_area, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list_area, LV_SCROLLBAR_MODE_OFF);

    for (size_t i = 0; i < DOUBLE_NOTE_OPTION_COUNT; i++) {
        double_note_create_option(list_area, i, &g_double_note_options[i]);
    }
}

static void double_note_create_panel(lv_obj_t* parent)
{
    lv_obj_t* card = double_note_create_card(parent, 38, 18, 730, 306);

    settings_detail_create_label(card, ui_text_get(UI_TEXT_SETTINGS_DOUBLE_NOTE_DISPLAY),
                                 &lv_font_montserrat_16, lv_color_hex(0x2D3440), 24, 20);

    lv_obj_t* accent = lv_obj_create(card);
    lv_obj_remove_style_all(accent);
    lv_obj_set_pos(accent, 314, 26);
    lv_obj_set_size(accent, 102, 8);
    lv_obj_set_style_bg_color(accent, lv_color_hex(0x08C5D6), 0);
    lv_obj_set_style_bg_opa(accent, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(accent, 4, 0);
    lv_obj_clear_flag(accent, LV_OBJ_FLAG_SCROLLABLE);

    double_note_create_list(card);
}

static void double_note_create_preview(lv_obj_t* parent)
{
    lv_obj_t* card = settings_detail_create_card(parent, 820, 18, 370, 306);
    lv_obj_set_style_shadow_width(card, 8, 0);

    lv_obj_t* header = lv_obj_create(card);
    lv_obj_remove_style_all(header);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_size(header, 370, 42);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x08C5D6), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    settings_detail_create_label(header, ui_text_get(UI_TEXT_SETTINGS_DOUBLE_NOTE_PREVIEW),
                                 &lv_font_montserrat_18, lv_color_hex(0xFFFFFF), 150, 12);

    settings_detail_create_label(card, ui_text_get(UI_TEXT_SETTINGS_DOUBLE_NOTE_DISPLAY),
                                 &lv_font_montserrat_16, lv_color_hex(0x7686A5), 34, 62);

    preview_level_label = settings_detail_create_label(card, "", &lv_font_montserrat_48,
                                                       lv_color_hex(0x0878C8), 34, 90);

    preview_desc_label = settings_detail_create_label(card, "", &lv_font_montserrat_16,
                                                      lv_color_hex(0x0878C8), 0, 276);
    lv_obj_set_width(preview_desc_label, 370);
    lv_obj_set_style_text_align(preview_desc_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(preview_desc_label, LV_LABEL_LONG_CLIP);

    for (size_t i = 0; i < DOUBLE_NOTE_OPTION_COUNT; i++) {
        lv_coord_t h = (lv_coord_t)(48 + i * 24);
        lv_coord_t x = (lv_coord_t)(78 + i * 84);
        lv_coord_t y = (lv_coord_t)(224 - h);

        preview_bars[i] = lv_obj_create(card);
        lv_obj_remove_style_all(preview_bars[i]);
        lv_obj_set_pos(preview_bars[i], x, y);
        lv_obj_set_size(preview_bars[i], 52, h);
        lv_obj_set_style_bg_color(preview_bars[i], lv_color_hex(0xE9EDF2), 0);
        lv_obj_set_style_bg_opa(preview_bars[i], LV_OPA_COVER, 0);
        lv_obj_set_style_radius(preview_bars[i], 6, 0);
        lv_obj_clear_flag(preview_bars[i], LV_OBJ_FLAG_SCROLLABLE);

        preview_bar_labels[i] = settings_detail_create_label(card, "", &lv_font_montserrat_16,
                                                             lv_color_hex(0x7686A5),
                                                             (lv_coord_t)(x + 20), 238);
        lv_label_set_text_fmt(preview_bar_labels[i], "%u", (unsigned)g_double_note_options[i].level);
    }
}

void ui_page_22_set_double_note_create(lv_obj_t* parent)
{
    lv_obj_t* content = NULL;

    if (double_note_page) return;

    Machine_para.double_note_level = double_note_get_level();
    pending_level = 0;
    pending_prev_level = Machine_para.double_note_level;

    double_note_page = settings_detail_create_page(parent,
                                                   ui_text_get(UI_TEXT_SETTINGS_DOUBLE_NOTE_TITLE),
                                                   double_note_esc_cb, &content);
    double_note_setting_page = double_note_page;

    double_note_create_panel(content);
    double_note_create_preview(content);

    double_note_refresh_view();
}

void ui_page_22_set_double_note_destroy(void)
{
    if (double_note_page && lv_obj_is_valid(double_note_page)) {
        lv_obj_del(double_note_page);
    }

    double_note_page = NULL;
    double_note_setting_page = NULL;
    preview_level_label = NULL;
    preview_desc_label = NULL;
    pending_level = 0;
    pending_prev_level = DOUBLE_NOTE_LEVEL_MIN;

    for (size_t i = 0; i < DOUBLE_NOTE_OPTION_COUNT; i++) {
        g_double_note_items[i].card = NULL;
        g_double_note_items[i].check = NULL;
        preview_bars[i] = NULL;
        preview_bar_labels[i] = NULL;
    }
}

void ui_page_22_set_double_note_on_reply(uint8_t level, uint8_t res)
{
    uint8_t normalized_level = double_note_normalize_level(level);

    if (res == 0x01) {
        Machine_para.double_note_level = normalized_level;
        pending_level = 0;
        if (double_note_page) {
            double_note_refresh_view();
        }
        return;
    }

    if (pending_level == normalized_level) {
        Machine_para.double_note_level = double_note_normalize_level(pending_prev_level);
        pending_level = 0;
    }

    if (double_note_page) {
        double_note_refresh_view();
    }
}
