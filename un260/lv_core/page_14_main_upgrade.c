#include "page_14_main_upgrade.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/settings_detail_ui.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_system/ui_text.h"

#include <stdbool.h>

#define MAIN_UPGRADE_TIMEOUT_MS 20000

static lv_obj_t* page_14 = NULL;
static lv_obj_t* lbl_upgrade = NULL;
static lv_timer_t* timeout_timer = NULL;

static bool waiting_upgrade_result = false;
static uint32_t wait_start_tick = 0;

static uint8_t last_a1 = 0x00;
static bool has_last_a1 = false;

static const char* a1_text(uint8_t res)
{
    switch (res) {
    case 0x01: return "Start main-board upgrade";
    case 0x02: return "Main-board upgrading";
    case 0x03: return "Main-board upgrade success";
    case 0x04: return "Main-board upgrade success";
    case 0xF1: return "No upgrade file";
    case 0xF2: return "Upgrade file mismatch";
    case 0xF3: return "Main-board upgrade failed";
    default:   return "Unknown status";
    }
}

static bool a1_terminal(uint8_t res)
{
    return (res == 0x03 || res == 0x04 || res == 0xF1 || res == 0xF2 || res == 0xF3);
}

static void esc_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_pop_page();
}

static void timeout_cb(lv_timer_t* t)
{
    (void)t;
    if (!waiting_upgrade_result) return;

    if (lv_tick_elaps(wait_start_tick) >= MAIN_UPGRADE_TIMEOUT_MS) {
        waiting_upgrade_result = false;
        if (lbl_upgrade && lv_obj_is_valid(lbl_upgrade)) {
            lv_label_set_text(lbl_upgrade, ui_text_get(UI_TEXT_SETTINGS_MAIN_UPGRADE_TIMEOUT));
            lv_obj_set_style_text_color(lbl_upgrade, lv_color_hex(0xC03A2B), 0);
        }
    }
}

static void start_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    if (fd4 < 0) {
        lv_label_set_text(lbl_upgrade, ui_text_get(UI_TEXT_SETTINGS_UART_NOT_READY));
        lv_obj_set_style_text_color(lbl_upgrade, lv_color_hex(0xC03A2B), 0);
        return;
    }

    send_command(fd4, 0xA1, (const uint8_t[]){0x01}, 1);
    waiting_upgrade_result = true;
    wait_start_tick = lv_tick_get();

    lv_label_set_text(lbl_upgrade, ui_text_get(UI_TEXT_SETTINGS_MAIN_UPGRADE_SENT));
    lv_obj_set_style_text_color(lbl_upgrade, lv_color_hex(0x2D3A4A), 0);
}

void ui_page_14_main_upgrade_on_reply(uint8_t cmd_g, uint8_t res)
{
    if (cmd_g != 0xA1) return;

    has_last_a1 = true;
    last_a1 = res;

    if (lbl_upgrade && lv_obj_is_valid(lbl_upgrade)) {
        lv_label_set_text_fmt(lbl_upgrade, "Main-board: %s", a1_text(res));

        if (res == 0x03 || res == 0x04) {
            lv_obj_set_style_text_color(lbl_upgrade, lv_color_hex(0x1F9D55), 0);
        } else if (res == 0xF1 || res == 0xF2 || res == 0xF3) {
            lv_obj_set_style_text_color(lbl_upgrade, lv_color_hex(0xC03A2B), 0);
        } else {
            lv_obj_set_style_text_color(lbl_upgrade, lv_color_hex(0x2D3A4A), 0);
        }
    }

    if (a1_terminal(res)) {
        waiting_upgrade_result = false;
    }
}

void ui_page_14_main_upgrade_create(lv_obj_t* parent)
{
    if (page_14) return;

    lv_obj_t* content = NULL;
    page_14 = settings_detail_create_page(parent, ui_text_get(UI_TEXT_SETTINGS_MAIN_UPGRADE_TITLE),
                                          esc_cb, &content);

    lv_obj_t* card = settings_detail_create_card(content, 40, 45, 1200, 260);

    lbl_upgrade = lv_label_create(card);
    lv_label_set_text(lbl_upgrade, ui_text_get(UI_TEXT_SETTINGS_PRESS_START_MAIN_UPGRADE));
    lv_obj_set_style_text_font(lbl_upgrade, &lv_font_instrument_sans_medium_22, 0);
    lv_obj_set_pos(lbl_upgrade, 36, 60);

    settings_detail_create_button(card, 36, 150, 250, 70,
                                  ui_text_get(UI_TEXT_SETTINGS_START_UPGRADE),
                                  lv_color_hex(0x08C5D6),
                                  start_cb, NULL);

    if (has_last_a1) {
        ui_page_14_main_upgrade_on_reply(0xA1, last_a1);
    }

    if (!timeout_timer) {
        timeout_timer = lv_timer_create(timeout_cb, 200, NULL);
    }
}

void ui_page_14_main_upgrade_destroy(void)
{
    if (timeout_timer) {
        lv_timer_del(timeout_timer);
        timeout_timer = NULL;
    }

    if (page_14 && lv_obj_is_valid(page_14)) {
        lv_obj_del(page_14);
    }

    page_14 = NULL;
    lbl_upgrade = NULL;
    waiting_upgrade_result = false;
}
