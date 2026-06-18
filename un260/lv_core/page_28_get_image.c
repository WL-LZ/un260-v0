#include "page_28_get_image.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/settings_detail_ui.h"
#include "un260/lv_system/ui_text.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IMAGE_SOURCE_COUNT 6
#define IMAGE_MAX_WIDTH 512
#define IMAGE_MAX_HEIGHT 384
#define IMAGE_PREVIEW_W 760
#define IMAGE_PREVIEW_H 230

typedef struct {
    uint8_t id;
    ui_text_id_t text_id;
    uint32_t color_hex;
} image_source_t;

typedef struct {
    lv_obj_t* card;
    lv_obj_t* dot;
    lv_obj_t* label;
} image_source_item_t;

static const image_source_t g_image_sources[IMAGE_SOURCE_COUNT] = {
    { 0x01, UI_TEXT_SETTINGS_IMAGE_GET_UP_WHITE, 0x38BDF8 },
    { 0x02, UI_TEXT_SETTINGS_IMAGE_GET_DOWN_WHITE, 0x0EA5E9 },
    { 0x03, UI_TEXT_SETTINGS_IMAGE_GET_UP_IR_REFLECT, 0x8B5CF6 },
    { 0x04, UI_TEXT_SETTINGS_IMAGE_GET_DOWN_IR_REFLECT, 0x6366F1 },
    { 0x05, UI_TEXT_SETTINGS_IMAGE_GET_UP_IR_TRANS, 0x24B47E },
    { 0x06, UI_TEXT_SETTINGS_IMAGE_GET_DOWN_IR_TRANS, 0xF59D2A },
};

static lv_obj_t* image_page = NULL;
static lv_obj_t* image_canvas = NULL;
static lv_obj_t* image_placeholder = NULL;
static lv_obj_t* image_status_label = NULL;
static lv_obj_t* image_size_label = NULL;
static lv_obj_t* image_title_label = NULL;
static lv_color_t* image_buffer = NULL;
static image_source_item_t source_items[IMAGE_SOURCE_COUNT] = { 0 };
static uint16_t image_width = 0;
static uint16_t image_height = 0;
static uint16_t rows_received = 0;
static uint8_t selected_image_id = 1;
static bool image_receiving = false;

static const image_source_t* image_find_source(uint8_t id)
{
    for (size_t i = 0; i < IMAGE_SOURCE_COUNT; i++) {
        if (g_image_sources[i].id == id) {
            return &g_image_sources[i];
        }
    }

    return &g_image_sources[0];
}

static size_t image_source_index(uint8_t id)
{
    for (size_t i = 0; i < IMAGE_SOURCE_COUNT; i++) {
        if (g_image_sources[i].id == id) {
            return i;
        }
    }

    return 0;
}

static lv_coord_t image_zoom_for_size(uint16_t width, uint16_t height)
{
    uint32_t zoom_w;
    uint32_t zoom_h;
    uint32_t zoom;

    if (width == 0 || height == 0) return 256;

    zoom_w = (uint32_t)IMAGE_PREVIEW_W * 256U / width;
    zoom_h = (uint32_t)IMAGE_PREVIEW_H * 256U / height;
    zoom = zoom_w < zoom_h ? zoom_w : zoom_h;
    if (zoom == 0) zoom = 1;
    if (zoom > 256) zoom = 256;

    return (lv_coord_t)zoom;
}

static void image_set_status(const char* text, lv_color_t color)
{
    if (!image_status_label || !lv_obj_is_valid(image_status_label)) return;

    lv_label_set_text(image_status_label, text);
    lv_obj_set_style_text_color(image_status_label, color, 0);
}

static void image_refresh_size_label(void)
{
    if (!image_size_label || !lv_obj_is_valid(image_size_label)) return;

    if (image_width == 0 || image_height == 0) {
        lv_label_set_text(image_size_label, "-- x --");
        return;
    }

    lv_label_set_text_fmt(image_size_label, "%u x %u", image_width, image_height);
}

static void image_refresh_sources(void)
{
    size_t selected_index = image_source_index(selected_image_id);

    for (size_t i = 0; i < IMAGE_SOURCE_COUNT; i++) {
        bool selected = (i == selected_index);
        lv_color_t color = lv_color_hex(g_image_sources[i].color_hex);

        if (source_items[i].card) {
            lv_obj_set_style_bg_color(source_items[i].card,
                                      selected ? lv_color_hex(0xF2FBFF) : lv_color_hex(0xFFFFFF),
                                      0);
            lv_obj_set_style_border_color(source_items[i].card,
                                          selected ? color : lv_color_hex(0xDDE6EF),
                                          0);
        }

        if (source_items[i].dot) {
            lv_obj_set_style_bg_color(source_items[i].dot, color, 0);
            lv_obj_set_style_bg_opa(source_items[i].dot, selected ? LV_OPA_COVER : LV_OPA_40, 0);
        }

        if (source_items[i].label) {
            lv_obj_set_style_text_color(source_items[i].label,
                                        selected ? lv_color_hex(0x1F2937) : lv_color_hex(0x6B7A90),
                                        0);
        }
    }

    if (image_title_label) {
        const image_source_t* source = image_find_source(selected_image_id);
        lv_label_set_text(image_title_label, ui_text_get(source->text_id));
    }
}

static void image_clear_buffer(void)
{
    if (image_buffer) {
        lv_mem_free(image_buffer);
        image_buffer = NULL;
    }

    image_width = 0;
    image_height = 0;
    rows_received = 0;
    image_receiving = false;

    if (image_canvas && lv_obj_is_valid(image_canvas)) {
        lv_obj_add_flag(image_canvas, LV_OBJ_FLAG_HIDDEN);
    }
    if (image_placeholder && lv_obj_is_valid(image_placeholder)) {
        lv_obj_clear_flag(image_placeholder, LV_OBJ_FLAG_HIDDEN);
    }
    image_refresh_size_label();
}

static bool image_prepare_buffer(uint16_t width, uint16_t height)
{
    size_t pixel_count;

    image_clear_buffer();
    if (width == 0 || height == 0 ||
        width > IMAGE_MAX_WIDTH || height > IMAGE_MAX_HEIGHT) {
        image_set_status(ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_SIZE_ERROR),
                         lv_color_hex(0xC03A2B));
        return false;
    }

    pixel_count = (size_t)width * height;
    if (!image_canvas || !lv_obj_is_valid(image_canvas)) {
        return false;
    }

    image_buffer = (lv_color_t*)lv_mem_alloc(pixel_count * sizeof(lv_color_t));
    if (!image_buffer) {
        image_set_status(ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_MEMORY_ERROR),
                         lv_color_hex(0xC03A2B));
        return false;
    }

    image_width = width;
    image_height = height;
    image_receiving = true;

    for (size_t i = 0; i < pixel_count; i++) {
        image_buffer[i] = lv_color_hex(0xEFF4F8);
    }

    lv_canvas_set_buffer(image_canvas, image_buffer, image_width, image_height,
                         LV_IMG_CF_TRUE_COLOR);
    lv_img_set_zoom(image_canvas, image_zoom_for_size(image_width, image_height));
    lv_obj_center(image_canvas);
    lv_obj_clear_flag(image_canvas, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(image_placeholder, LV_OBJ_FLAG_HIDDEN);
    image_refresh_size_label();
    image_set_status(ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_RECEIVING),
                     lv_color_hex(0x0878C8));
    return true;
}

static void image_draw_row(uint16_t row, const uint8_t* pixels, uint16_t pixel_len)
{
    uint16_t y;
    uint16_t copy_len;

    if (!image_buffer || !image_receiving || image_width == 0 || image_height == 0) return;
    if (row == 0) return;

    y = (uint16_t)(row - 1);
    if (y >= image_height) return;

    copy_len = pixel_len < image_width ? pixel_len : image_width;
    for (uint16_t x = 0; x < copy_len; x++) {
        uint8_t gray = pixels[x];
        image_buffer[(size_t)y * image_width + x] = lv_color_make(gray, gray, gray);
    }

    if (row > rows_received) {
        rows_received = row;
    }

    lv_obj_invalidate(image_canvas);
    if (image_status_label && lv_obj_is_valid(image_status_label)) {
        lv_label_set_text_fmt(image_status_label, "%s %u/%u",
                              ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_RECEIVING),
                              rows_received, image_height);
    }
}

static bool image_send_request(void)
{
    uint8_t payload = selected_image_id;

    return settings_detail_send_command(0x47, &payload, 1);
}

static void image_request_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    image_clear_buffer();
    if (image_send_request()) {
        image_set_status(ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_WAITING),
                         lv_color_hex(0x0878C8));
    }
}

static void image_source_cb(lv_event_t* e)
{
    uint8_t image_id;

    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    image_id = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    if (image_id < 1 || image_id > IMAGE_SOURCE_COUNT) return;

    selected_image_id = image_id;
    image_refresh_sources();
}

static void image_esc_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_pop_page();
}

static lv_obj_t* image_create_card(lv_obj_t* parent, lv_coord_t x, lv_coord_t y,
                                   lv_coord_t w, lv_coord_t h)
{
    lv_obj_t* card = settings_detail_create_card(parent, x, y, w, h);
    lv_obj_set_style_shadow_width(card, 10, 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_10, 0);
    return card;
}

static void image_create_source_item(lv_obj_t* parent, size_t index)
{
    const image_source_t* source = &g_image_sources[index];
    lv_obj_t* item = lv_obj_create(parent);
    lv_coord_t y = (lv_coord_t)(26 + index * 43);

    lv_obj_remove_style_all(item);
    lv_obj_set_pos(item, 18, y);
    lv_obj_set_size(item, 244, 36);
    lv_obj_set_style_bg_color(item, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(item, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(item, 2, 0);
    lv_obj_set_style_border_color(item, lv_color_hex(0xDDE6EF), 0);
    lv_obj_set_style_radius(item, 8, 0);
    lv_obj_set_style_translate_y(item, 0, LV_STATE_PRESSED);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(item, image_source_cb, LV_EVENT_CLICKED,
                        (void*)(uintptr_t)source->id);

    source_items[index].dot = lv_obj_create(item);
    lv_obj_remove_style_all(source_items[index].dot);
    lv_obj_set_pos(source_items[index].dot, 15, 12);
    lv_obj_set_size(source_items[index].dot, 12, 12);
    lv_obj_set_style_radius(source_items[index].dot, 6, 0);
    lv_obj_set_style_bg_color(source_items[index].dot, lv_color_hex(source->color_hex), 0);
    lv_obj_set_style_bg_opa(source_items[index].dot, LV_OPA_COVER, 0);
    lv_obj_clear_flag(source_items[index].dot, LV_OBJ_FLAG_SCROLLABLE);

    source_items[index].label = settings_detail_create_label(item, ui_text_get(source->text_id),
                                                             &lv_font_montserrat_14,
                                                             lv_color_hex(0x2D3440), 40, 10);
    source_items[index].card = item;
}

static void image_create_left_panel(lv_obj_t* parent)
{
    lv_obj_t* card = image_create_card(parent, 38, 18, 286, 306);

    settings_detail_create_label(card, ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_SOURCE),
                                 &lv_font_montserrat_16, lv_color_hex(0x2D3440), 22, 16);

    for (size_t i = 0; i < IMAGE_SOURCE_COUNT; i++) {
        image_create_source_item(card, i);
    }
}

static void image_create_preview(lv_obj_t* parent)
{
    lv_obj_t* card = image_create_card(parent, 350, 18, 840, 306);
    lv_obj_t* film;
    lv_obj_t* accent;

    image_title_label = settings_detail_create_label(card, "",
                                                     &lv_font_montserrat_18,
                                                     lv_color_hex(0x2D3440), 32, 18);
    image_size_label = settings_detail_create_label(card, "-- x --",
                                                    &lv_font_montserrat_14,
                                                    lv_color_hex(0x7686A5), 644, 22);

    accent = lv_obj_create(card);
    lv_obj_remove_style_all(accent);
    lv_obj_set_pos(accent, 30, 50);
    lv_obj_set_size(accent, 780, 2);
    lv_obj_set_style_bg_color(accent, lv_color_hex(0xE9EDF2), 0);
    lv_obj_set_style_bg_opa(accent, LV_OPA_COVER, 0);
    lv_obj_clear_flag(accent, LV_OBJ_FLAG_SCROLLABLE);

    film = lv_obj_create(card);
    lv_obj_remove_style_all(film);
    lv_obj_set_pos(film, 40, 66);
    lv_obj_set_size(film, IMAGE_PREVIEW_W, IMAGE_PREVIEW_H);
    lv_obj_set_style_bg_color(film, lv_color_hex(0x111827), 0);
    lv_obj_set_style_bg_opa(film, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(film, 1, 0);
    lv_obj_set_style_border_color(film, lv_color_hex(0x233249), 0);
    lv_obj_set_style_radius(film, 10, 0);
    lv_obj_clear_flag(film, LV_OBJ_FLAG_SCROLLABLE);

    image_placeholder = settings_detail_create_label(film,
                                                     ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_EMPTY),
                                                     &lv_font_montserrat_18,
                                                     lv_color_hex(0x94A3B8), 0, 0);
    lv_obj_center(image_placeholder);

    image_canvas = lv_canvas_create(film);
    lv_obj_add_flag(image_canvas, LV_OBJ_FLAG_HIDDEN);

    image_status_label = settings_detail_create_label(card,
                                                      ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_IDLE),
                                                      &lv_font_montserrat_14,
                                                      lv_color_hex(0x7686A5), 40, 270);

    settings_detail_create_button(card, 664, 260, 136, 34,
                                  ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_BUTTON),
                                  lv_color_hex(0x0878C8), image_request_cb, NULL);
}

void ui_page_28_get_image_create(lv_obj_t* parent)
{
    lv_obj_t* content = NULL;

    if (image_page) return;

    image_page = settings_detail_create_page(parent,
                                             ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_TITLE),
                                             image_esc_cb, &content);
    image_get_page = image_page;

    image_create_left_panel(content);
    image_create_preview(content);
    image_refresh_sources();
    image_refresh_size_label();
}

void ui_page_28_get_image_destroy(void)
{
    image_clear_buffer();

    if (image_page && lv_obj_is_valid(image_page)) {
        lv_obj_del(image_page);
    }

    image_page = NULL;
    image_get_page = NULL;
    image_canvas = NULL;
    image_placeholder = NULL;
    image_status_label = NULL;
    image_size_label = NULL;
    image_title_label = NULL;
    selected_image_id = 1;

    for (size_t i = 0; i < IMAGE_SOURCE_COUNT; i++) {
        source_items[i].card = NULL;
        source_items[i].dot = NULL;
        source_items[i].label = NULL;
    }
}

void ui_page_28_get_image_on_frame(const uint8_t* data, uint16_t len)
{
    uint8_t sub;

    if (!data || len < 1) return;
    if (!image_page || !lv_obj_is_valid(image_page)) return;

    sub = data[0];
    if (sub == 0x00) {
        uint8_t image_id;
        uint16_t width;
        uint16_t height;

        if (len < 6) return;
        image_id = data[1];
        width = (uint16_t)(((uint16_t)data[2] << 8) | data[3]);
        height = (uint16_t)(((uint16_t)data[4] << 8) | data[5]);
        selected_image_id = image_id;
        image_refresh_sources();
        image_prepare_buffer(width, height);
        return;
    }

    if (sub == 0xFF) {
        image_receiving = false;
        if (image_canvas && lv_obj_is_valid(image_canvas)) {
            lv_obj_invalidate(image_canvas);
        }
        image_set_status(ui_text_get(UI_TEXT_SETTINGS_IMAGE_GET_DONE),
                         lv_color_hex(0x24B47E));
        return;
    }

    if (sub >= 0x01 && sub <= IMAGE_SOURCE_COUNT) {
        uint16_t row;

        if (len < 4) return;
        row = (uint16_t)(((uint16_t)data[1] << 8) | data[2]);
        image_draw_row(row, &data[3], (uint16_t)(len - 3));
    }
}
