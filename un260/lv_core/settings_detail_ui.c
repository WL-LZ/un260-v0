#include "un260/lv_core/settings_detail_ui.h"
#include "un260/lv_core/page_06_settings.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_system/ui_text.h"

#define SETTINGS_DETAIL_W             1280
#define SETTINGS_DETAIL_H             400
#define SETTINGS_DETAIL_HEADER_H      55

static lv_color_t detail_bg(void)      { return lv_color_hex(0xF7F8FA); }
static lv_color_t detail_panel(void)   { return lv_color_hex(0xFFFFFF); }
static lv_color_t detail_line(void)    { return lv_color_hex(0xE9EDF2); }
static lv_color_t detail_grid(void)    { return lv_color_hex(0xECEFF3); }
static lv_color_t detail_primary(void) { return lv_color_hex(0x08C5D6); }
static lv_color_t detail_primary_2(void){ return lv_color_hex(0xE3FAFD); }
static lv_color_t detail_text(void)    { return lv_color_hex(0x2D3440); }

static void detail_style_plain(lv_obj_t* obj)
{
    lv_obj_remove_style_all(obj);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
}

lv_obj_t* settings_detail_create_label(lv_obj_t* parent, const char* text,
                                       const lv_font_t* font, lv_color_t color,
                                       lv_coord_t x, lv_coord_t y)
{
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_pos(label, x, y);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
    return label;
}

static void settings_detail_create_grid(lv_obj_t* parent)
{
    for (int x = 0; x < SETTINGS_DETAIL_W; x += 32) {
        lv_obj_t* line = lv_obj_create(parent);
        detail_style_plain(line);
        lv_obj_set_pos(line, x, SETTINGS_DETAIL_HEADER_H);
        lv_obj_set_size(line, 1, SETTINGS_DETAIL_H - SETTINGS_DETAIL_HEADER_H);
        lv_obj_set_style_bg_color(line, detail_grid(), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_50, 0);
    }

    for (int y = SETTINGS_DETAIL_HEADER_H; y < SETTINGS_DETAIL_H; y += 32) {
        lv_obj_t* line = lv_obj_create(parent);
        detail_style_plain(line);
        lv_obj_set_pos(line, 0, y);
        lv_obj_set_size(line, SETTINGS_DETAIL_W, 1);
        lv_obj_set_style_bg_color(line, detail_grid(), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_50, 0);
    }
}

lv_obj_t* settings_detail_create_page(lv_obj_t* parent, const char* title,
                                      lv_event_cb_t back_cb,
                                      lv_obj_t** out_content)
{
    return settings_detail_create_page_ex(parent, title, back_cb, out_content, NULL);
}

lv_obj_t* settings_detail_create_page_ex(lv_obj_t* parent, const char* title,
                                         lv_event_cb_t back_cb,
                                         lv_obj_t** out_content,
                                         lv_obj_t** out_back_btn)
{
    lv_obj_t* root_parent = parent ? parent : lv_scr_act();
    lv_obj_t* page = lv_obj_create(root_parent);
    detail_style_plain(page);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_size(page, SETTINGS_DETAIL_W, SETTINGS_DETAIL_H);
    lv_obj_set_style_bg_color(page, detail_bg(), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(page, 0, 0);

    settings_detail_create_grid(page);

    lv_obj_t* header = lv_obj_create(page);
    detail_style_plain(header);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_size(header, SETTINGS_DETAIL_W, SETTINGS_DETAIL_HEADER_H);
    lv_obj_set_style_bg_color(header, detail_panel(), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);

    lv_obj_t* accent = lv_obj_create(header);
    detail_style_plain(accent);
    lv_obj_set_pos(accent, 0, 0);
    lv_obj_set_size(accent, 5, SETTINGS_DETAIL_HEADER_H);
    lv_obj_set_style_bg_color(accent, detail_primary(), 0);
    lv_obj_set_style_bg_opa(accent, LV_OPA_COVER, 0);

    lv_obj_t* gear_bg = lv_obj_create(header);
    detail_style_plain(gear_bg);
    lv_obj_set_pos(gear_bg, 31, 13);
    lv_obj_set_size(gear_bg, 30, 30);
    lv_obj_set_style_bg_color(gear_bg, detail_primary_2(), 0);
    lv_obj_set_style_bg_opa(gear_bg, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(gear_bg, 4, 0);

    lv_obj_t* gear = settings_detail_create_label(gear_bg, LV_SYMBOL_SETTINGS,
                                                  &lv_font_montserrat_16,
                                                  detail_primary(), 0, 0);
    lv_obj_center(gear);

    settings_detail_create_label(header, title, &lv_font_montserrat_20,
                                 detail_text(), 72, 14);

    lv_obj_t* title_line = lv_obj_create(header);
    detail_style_plain(title_line);
    lv_obj_set_pos(title_line, 72, 43);
    lv_obj_set_size(title_line, 72, 3);
    lv_obj_set_style_bg_color(title_line, detail_primary(), 0);
    lv_obj_set_style_bg_opa(title_line, LV_OPA_COVER, 0);

    lv_obj_t* bottom_line = lv_obj_create(header);
    detail_style_plain(bottom_line);
    lv_obj_set_pos(bottom_line, 0, SETTINGS_DETAIL_HEADER_H - 1);
    lv_obj_set_size(bottom_line, SETTINGS_DETAIL_W, 1);
    lv_obj_set_style_bg_color(bottom_line, detail_line(), 0);
    lv_obj_set_style_bg_opa(bottom_line, LV_OPA_COVER, 0);

    lv_obj_t* esc = settings_detail_create_button(header, 1156, 10, 92, 35,
                                                  ui_text_get(UI_TEXT_SETTINGS_ESC),
                                                  lv_color_hex(0xF04444),
                                                  back_cb, NULL);
    lv_obj_set_style_shadow_width(esc, 0, 0);
    if (out_back_btn) {
        *out_back_btn = esc;
    }

    lv_obj_t* content = lv_obj_create(page);
    detail_style_plain(content);
    lv_obj_set_pos(content, 0, SETTINGS_DETAIL_HEADER_H);
    lv_obj_set_size(content, SETTINGS_DETAIL_W, SETTINGS_DETAIL_H - SETTINGS_DETAIL_HEADER_H);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);

    if (out_content) {
        *out_content = content;
    }

    return page;
}

lv_obj_t* settings_detail_create_card(lv_obj_t* parent, lv_coord_t x, lv_coord_t y,
                                      lv_coord_t w, lv_coord_t h)
{
    lv_obj_t* card = lv_obj_create(parent);
    detail_style_plain(card);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_bg_color(card, detail_panel(), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0xDDE6EF), 0);
    lv_obj_set_style_radius(card, 4, 0);
    lv_obj_set_style_shadow_width(card, 14, 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_20, 0);
    lv_obj_set_style_shadow_ofs_y(card, 6, 0);
    return card;
}

lv_obj_t* settings_detail_create_button(lv_obj_t* parent, lv_coord_t x, lv_coord_t y,
                                        lv_coord_t w, lv_coord_t h,
                                        const char* text, lv_color_t bg,
                                        lv_event_cb_t cb, void* user_data)
{
    lv_obj_t* btn = lv_obj_create(parent);
    detail_style_plain(btn);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_bg_color(btn, bg, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, 4, 0);
    lv_obj_set_style_shadow_width(btn, 10, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
    lv_obj_set_style_shadow_ofs_y(btn, 4, 0);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);

    if (cb) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
    }

    lv_obj_t* label = settings_detail_create_label(btn, text, &lv_font_montserrat_16,
                                                   detail_panel(), 0, 0);
    lv_obj_center(label);
    return btn;
}

bool settings_detail_send_command(uint8_t cmd_g, const uint8_t* cmd_s,
                                  uint16_t cmd_s_len)
{
    if (fd4 < 0) {
        page_06_settings_set_status(ui_text_get(UI_TEXT_SETTINGS_UART_NOT_READY),
                                    lv_color_hex(0xC03A2B));
        return false;
    }

    send_command(fd4, cmd_g, cmd_s, cmd_s_len);
    return true;
}
