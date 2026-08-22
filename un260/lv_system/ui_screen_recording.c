#include "ui_screen_recording.h"

#include "un260/lv_components/lv_print_toast.h"
#include "un260/lv_system/ui_text.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/recording/screen_recording_service.h"

#include "lvgl/lvgl.h"

#include <stdbool.h>

#define UI_SCREEN_RECORDING_BUTTON_X 1204
#define UI_SCREEN_RECORDING_BUTTON_Y 3
#define UI_SCREEN_RECORDING_BUTTON_SIZE 44

static lv_obj_t *g_record_button;
static lv_obj_t *g_record_mark;
static screen_recording_state_t g_last_state = SCREEN_RECORDING_IDLE;

static void ui_screen_recording_show_toast(const char *text, bool alarm)
{
    lv_print_toast_config_t toast_cfg = lv_print_toast_get_default_config();

    toast_cfg.w = 390;
    toast_cfg.h = 101;
    toast_cfg.text = text;
    toast_cfg.show_loader = false;
    toast_cfg.align_center = true;
    toast_cfg.use_text_area = false;
    toast_cfg.loader_color = alarm ? lv_color_hex(0xC0392B)
                                   : lv_color_hex(0x18A66A);
    toast_cfg.auto_hide_ms = 1800;
    lv_print_toast_show_with_config(&toast_cfg);
}

static void ui_screen_recording_apply_state(screen_recording_state_t state)
{
    bool stop_shape = state != SCREEN_RECORDING_IDLE;

    if (g_record_mark == NULL || !lv_obj_is_valid(g_record_mark)) {
        return;
    }
    lv_obj_set_size(g_record_mark, stop_shape ? 12 : 11,
                    stop_shape ? 12 : 11);
    lv_obj_set_style_radius(g_record_mark, stop_shape ? 2 : LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(g_record_mark, lv_color_hex(0xFF3B30), 0);
    lv_obj_set_style_bg_opa(g_record_mark, LV_OPA_COVER, 0);
    lv_obj_center(g_record_mark);
    if (g_record_button != NULL && lv_obj_is_valid(g_record_button)) {
        if (state == SCREEN_RECORDING_STOPPING) {
            lv_obj_clear_flag(g_record_button, LV_OBJ_FLAG_CLICKABLE);
        } else {
            lv_obj_add_flag(g_record_button, LV_OBJ_FLAG_CLICKABLE);
        }
    }
    g_last_state = state;
}

static void ui_screen_recording_click_cb(lv_event_t *event)
{
    screen_recording_state_t state;
    screen_recording_start_result_t result;

    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }
    state = screen_recording_service_state();
    if (state == SCREEN_RECORDING_STARTING ||
        state == SCREEN_RECORDING_ACTIVE) {
        screen_recording_service_request_stop();
        ui_screen_recording_apply_state(SCREEN_RECORDING_STOPPING);
        ui_screen_recording_show_toast(
            ui_text_get(UI_TEXT_WIDGET_SCREEN_RECORDING_STOPPING), false);
        return;
    }
    if (state != SCREEN_RECORDING_IDLE) {
        return;
    }

    result = screen_recording_service_start();
    if (result == SCREEN_RECORDING_START_OK) {
        ui_screen_recording_apply_state(SCREEN_RECORDING_STARTING);
        ui_screen_recording_show_toast(
            ui_text_get(UI_TEXT_WIDGET_SCREEN_RECORDING_STARTED), false);
    } else if (result == SCREEN_RECORDING_START_USB_NOT_READY) {
        ui_screen_recording_show_toast(
            ui_text_get(UI_TEXT_WIDGET_SCREENSHOT_INSERT_USB), true);
    } else if (result != SCREEN_RECORDING_START_BUSY) {
        ui_screen_recording_show_toast(
            ui_text_get(UI_TEXT_WIDGET_SCREEN_RECORDING_FAILED), true);
    }
}

static void ui_screen_recording_create(void)
{
    if (g_record_button != NULL && lv_obj_is_valid(g_record_button)) {
        return;
    }
    g_record_button = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(g_record_button);
    lv_obj_set_pos(g_record_button, UI_SCREEN_RECORDING_BUTTON_X,
                   UI_SCREEN_RECORDING_BUTTON_Y);
    lv_obj_set_size(g_record_button, UI_SCREEN_RECORDING_BUTTON_SIZE,
                    UI_SCREEN_RECORDING_BUTTON_SIZE);
    lv_obj_set_style_bg_color(g_record_button, lv_color_hex(0xE8E8E8),
                              LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(g_record_button, LV_OPA_50, LV_STATE_PRESSED);
    lv_obj_set_style_radius(g_record_button, 10, 0);
    lv_obj_clear_flag(g_record_button, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_record_button, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(g_record_button, ui_screen_recording_click_cb,
                        LV_EVENT_CLICKED, NULL);

    g_record_mark = lv_obj_create(g_record_button);
    lv_obj_remove_style_all(g_record_mark);
    lv_obj_clear_flag(g_record_mark, LV_OBJ_FLAG_CLICKABLE);
    ui_screen_recording_apply_state(screen_recording_service_state());
}

void ui_screen_recording_indicator_poll(void)
{
    screen_recording_completion_info_t completion;
    screen_recording_state_t state = screen_recording_service_state();
    bool enabled = user_cfg_screen_recording_enabled();

    if (screen_recording_service_poll_completion(&completion)) {
        if (completion.result == SCREEN_RECORDING_COMPLETION_SAVED) {
            ui_screen_recording_show_toast(
                ui_text_get(UI_TEXT_WIDGET_SCREEN_RECORDING_SAVED), false);
        } else {
            ui_screen_recording_show_toast(
                ui_text_get(UI_TEXT_WIDGET_SCREEN_RECORDING_FAILED), true);
        }
        state = screen_recording_service_state();
    }

    if (!enabled && state == SCREEN_RECORDING_IDLE) {
        if (g_record_button != NULL && lv_obj_is_valid(g_record_button)) {
            lv_obj_add_flag(g_record_button, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }
    ui_screen_recording_create();
    lv_obj_clear_flag(g_record_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(g_record_button);
    if (state != g_last_state) {
        ui_screen_recording_apply_state(state);
    }
}
