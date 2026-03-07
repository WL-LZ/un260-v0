#include "page_14_main_upgrade.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_drivers/lv_drivers.h"

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
            lv_label_set_text(lbl_upgrade, "Main-board upgrade timeout (20s), upgrade failed");
            lv_obj_set_style_text_color(lbl_upgrade, lv_color_hex(0xC03A2B), 0);
        }
    }
}

static void start_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    if (fd4 < 0) {
        lv_label_set_text(lbl_upgrade, "UART not ready");
        lv_obj_set_style_text_color(lbl_upgrade, lv_color_hex(0xC03A2B), 0);
        return;
    }

    send_command(fd4, 0xA1, (const uint8_t[]){0x01}, 1);
    waiting_upgrade_result = true;
    wait_start_tick = lv_tick_get();

    lv_label_set_text(lbl_upgrade, "Main-board upgrade command sent, waiting status...");
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

    lv_obj_t* root = parent ? parent : lv_scr_act();
    page_14 = lv_obj_create(root);
    lv_obj_remove_style_all(page_14);
    lv_obj_set_pos(page_14, 0, 0);
    lv_obj_set_size(page_14, 1280, 400);
    lv_obj_clear_flag(page_14, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(page_14, lv_color_hex(0xF2F6FB), 0);
    lv_obj_set_style_bg_opa(page_14, LV_OPA_COVER, 0);

    lv_obj_t* title = lv_label_create(page_14);
    lv_label_set_text(title, "MAIN BOARD UPGRADE");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(title, 36, 20);

    lv_obj_t* esc_btn = lv_btn_create(page_14);
    lv_obj_set_size(esc_btn, 100, 60);
    lv_obj_set_pos(esc_btn, 1160, 14);
    lv_obj_add_event_cb(esc_btn, esc_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* esc_label = lv_label_create(esc_btn);
    lv_label_set_text(esc_label, "ESC");
    lv_obj_center(esc_label);

    lv_obj_t* card = lv_obj_create(page_14);
    lv_obj_set_size(card, 1200, 260);
    lv_obj_set_pos(card, 40, 100);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lbl_upgrade = lv_label_create(card);
    lv_label_set_text(lbl_upgrade, "Press START to begin main-board upgrade");
    lv_obj_set_style_text_font(lbl_upgrade, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(lbl_upgrade, 36, 60);

    lv_obj_t* btn_start = lv_btn_create(card);
    lv_obj_set_size(btn_start, 250, 70);
    lv_obj_set_pos(btn_start, 36, 150);
    lv_obj_add_event_cb(btn_start, start_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* start_label = lv_label_create(btn_start);
    lv_label_set_text(start_label, "START UPGRADE");
    lv_obj_center(start_label);

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
