#include "lv_components.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/ui_text.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_drivers/lv_drivers.h"

typedef struct {
    lv_obj_t* switch_container;
    lv_obj_t* switch_knob;
    lv_obj_t* label_on;
    lv_obj_t* label_off;
} batch_switch_t;

static batch_switch_t batch_switch = {
    .switch_container = NULL,
    .switch_knob = NULL,
    .label_on = NULL,
    .label_off = NULL,
};
static bool g_batch_switch_wait_ack = false;
static bool g_batch_switch_prev_state = false;
static bool g_batch_switch_target_state = false;
static uint8_t g_batch_switch_sent_num = 200;
static uint8_t g_batch_last_on_num = 100;

void set_batch_switch_state(bool enable);

static lv_obj_t* g_boot_err_mask = NULL;
static lv_obj_t* g_boot_err_popup = NULL;
static lv_obj_t* g_boot_err_info_label = NULL;
static lv_obj_t* g_batch_set_fail_mask = NULL;
static lv_obj_t* g_batch_set_fail_popup = NULL;
static lv_obj_t* g_curr_set_fail_mask = NULL;
static lv_obj_t* g_curr_set_fail_popup = NULL;
static lv_obj_t* g_comm_err_mask = NULL;
static lv_obj_t* g_comm_err_popup = NULL;

void hide_boot_selftest_error_popup(void)
{
    if (g_boot_err_popup && lv_obj_is_valid(g_boot_err_popup)) {
        lv_obj_del(g_boot_err_popup);
    }
    g_boot_err_popup = NULL;
    g_boot_err_info_label = NULL;

    if (g_boot_err_mask && lv_obj_is_valid(g_boot_err_mask)) {
        lv_obj_del(g_boot_err_mask);
    }
    g_boot_err_mask = NULL;
}

static void boot_selftest_error_confirm_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    hide_boot_selftest_error_popup();
    ui_manager_switch(UI_PAGE_SENSOR);
}

void show_boot_selftest_error_popup(const char* msg)
{
    if (g_boot_err_popup && lv_obj_is_valid(g_boot_err_popup)) {
        if (g_boot_err_info_label && lv_obj_is_valid(g_boot_err_info_label)) {
            lv_label_set_text(g_boot_err_info_label, msg);
        }
        return;
    }

    lv_obj_t* scr = lv_scr_act();

    g_boot_err_mask = lv_obj_create(scr);
    lv_obj_remove_style_all(g_boot_err_mask);
    lv_obj_set_size(g_boot_err_mask, 1280, 400);
    lv_obj_set_style_bg_opa(g_boot_err_mask, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(g_boot_err_mask, lv_color_hex(0x000000), 0);

    g_boot_err_popup = lv_obj_create(scr);
    lv_obj_set_size(g_boot_err_popup, 700, 260);
    lv_obj_center(g_boot_err_popup);
    lv_obj_clear_flag(g_boot_err_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_boot_err_popup, 24, 0);
    lv_obj_set_style_bg_color(g_boot_err_popup, lv_color_hex(0xF4F7FB), 0);
    lv_obj_set_style_border_width(g_boot_err_popup, 2, 0);
    lv_obj_set_style_border_color(g_boot_err_popup, lv_color_hex(0xD7DEE8), 0);

    lv_obj_t* title = lv_label_create(g_boot_err_popup);
    lv_label_set_text(title, "SELF-TEST ERROR");
    lv_obj_set_style_text_font(title, &lv_font_instrument_sans_semibold_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x2D3A4A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 22);

    g_boot_err_info_label = lv_label_create(g_boot_err_popup);
    lv_label_set_text(g_boot_err_info_label, msg);
    lv_obj_set_width(g_boot_err_info_label, 620);
    lv_label_set_long_mode(g_boot_err_info_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(g_boot_err_info_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(g_boot_err_info_label, &lv_font_instrument_sans_medium_20, 0);
    lv_obj_set_style_text_color(g_boot_err_info_label, lv_color_hex(0x3C4D61), 0);
    lv_obj_align(g_boot_err_info_label, LV_ALIGN_TOP_MID, 0, 84);

    lv_obj_t* ok_btn = lv_btn_create(g_boot_err_popup);
    lv_obj_set_size(ok_btn, 180, 58);
    lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_radius(ok_btn, 16, 0);
    lv_obj_add_event_cb(ok_btn, boot_selftest_error_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "CONFIRM");
    lv_obj_set_style_text_font(ok_label, &lv_font_instrument_sans_bold_22, 0);
    lv_obj_center(ok_label);
}

void hide_batch_set_fail_popup(void)
{
    if (g_batch_set_fail_popup && lv_obj_is_valid(g_batch_set_fail_popup)) {
        lv_obj_del(g_batch_set_fail_popup);
    }
    g_batch_set_fail_popup = NULL;

    if (g_batch_set_fail_mask && lv_obj_is_valid(g_batch_set_fail_mask)) {
        lv_obj_del(g_batch_set_fail_mask);
    }
    g_batch_set_fail_mask = NULL;
}

void hide_currency_set_fail_popup(void)
{
    if (g_curr_set_fail_popup && lv_obj_is_valid(g_curr_set_fail_popup)) {
        lv_obj_del(g_curr_set_fail_popup);
    }
    g_curr_set_fail_popup = NULL;

    if (g_curr_set_fail_mask && lv_obj_is_valid(g_curr_set_fail_mask)) {
        lv_obj_del(g_curr_set_fail_mask);
    }
    g_curr_set_fail_mask = NULL;
}

void hide_communication_error_popup(void)
{
    if (g_comm_err_popup && lv_obj_is_valid(g_comm_err_popup)) {
        lv_obj_del(g_comm_err_popup);
    }
    g_comm_err_popup = NULL;

    if (g_comm_err_mask && lv_obj_is_valid(g_comm_err_mask)) {
        lv_obj_del(g_comm_err_mask);
    }
    g_comm_err_mask = NULL;
}

static void currency_set_fail_confirm_cb(lv_event_t* e)
{
    if ((lv_event_code_t)lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    hide_currency_set_fail_popup();
    ui_manager_switch(UI_PAGE_MAIN);
}

static void batch_set_fail_confirm_cb(lv_event_t* e)
{
    if ((lv_event_code_t)lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    hide_batch_set_fail_popup();
}

static void communication_error_confirm_cb(lv_event_t* e)
{
    if ((lv_event_code_t)lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    hide_communication_error_popup();
}

void show_communication_error_popup(void)
{
    if (g_comm_err_popup && lv_obj_is_valid(g_comm_err_popup)) {
        return;
    }

    lv_obj_t* scr = lv_scr_act();
    g_comm_err_mask = lv_obj_create(scr);
    lv_obj_remove_style_all(g_comm_err_mask);
    lv_obj_set_size(g_comm_err_mask, 1280, 400);
    lv_obj_set_style_bg_opa(g_comm_err_mask, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(g_comm_err_mask, lv_color_hex(0x000000), 0);

    g_comm_err_popup = lv_obj_create(scr);
    lv_obj_set_size(g_comm_err_popup, 740, 260);
    lv_obj_center(g_comm_err_popup);
    lv_obj_clear_flag(g_comm_err_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_comm_err_popup, 24, 0);
    lv_obj_set_style_bg_color(g_comm_err_popup, lv_color_hex(0xF4F7FB), 0);
    lv_obj_set_style_border_width(g_comm_err_popup, 2, 0);
    lv_obj_set_style_border_color(g_comm_err_popup, lv_color_hex(0xD7DEE8), 0);

    lv_obj_t* title = lv_label_create(g_comm_err_popup);
    lv_label_set_text(title, "COMMUNICATION ERROR");
    lv_obj_set_style_text_font(title, &lv_font_instrument_sans_semibold_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x2D3A4A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 22);

    lv_obj_t* info = lv_label_create(g_comm_err_popup);
    lv_label_set_text(info, "Communication may be abnormal. Please check the UI and controller connection.");
    lv_obj_set_width(info, 660);
    lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(info, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(info, &lv_font_instrument_sans_medium_20, 0);
    lv_obj_set_style_text_color(info, lv_color_hex(0x3C4D61), 0);
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 84);

    lv_obj_t* ok_btn = lv_btn_create(g_comm_err_popup);
    lv_obj_set_size(ok_btn, 180, 58);
    lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_radius(ok_btn, 16, 0);
    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0x1B86FF), 0);
    lv_obj_set_style_bg_opa(ok_btn, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(ok_btn, communication_error_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "CONFIRM");
    lv_obj_set_style_text_font(ok_label, &lv_font_instrument_sans_bold_22, 0);
    lv_obj_set_style_text_color(ok_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(ok_label);
}

void show_batch_set_fail_popup(void)
{
    if (g_batch_set_fail_popup && lv_obj_is_valid(g_batch_set_fail_popup)) {
        return;
    }

    lv_obj_t* scr = lv_scr_act();
    g_batch_set_fail_mask = lv_obj_create(scr);
    lv_obj_remove_style_all(g_batch_set_fail_mask);
    lv_obj_set_size(g_batch_set_fail_mask, 1280, 400);
    lv_obj_set_style_bg_opa(g_batch_set_fail_mask, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(g_batch_set_fail_mask, lv_color_hex(0x000000), 0);

    g_batch_set_fail_popup = lv_obj_create(scr);
    lv_obj_set_size(g_batch_set_fail_popup, 700, 260);
    lv_obj_center(g_batch_set_fail_popup);
    lv_obj_clear_flag(g_batch_set_fail_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_batch_set_fail_popup, 24, 0);
    lv_obj_set_style_bg_color(g_batch_set_fail_popup, lv_color_hex(0xF4F7FB), 0);
    lv_obj_set_style_border_width(g_batch_set_fail_popup, 2, 0);
    lv_obj_set_style_border_color(g_batch_set_fail_popup, lv_color_hex(0xD7DEE8), 0);

    lv_obj_t* title = lv_label_create(g_batch_set_fail_popup);
    lv_label_set_text(title, "BATCH SET FAILED");
    lv_obj_set_style_text_font(title, &lv_font_instrument_sans_semibold_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x2D3A4A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 22);

    lv_obj_t* info = lv_label_create(g_batch_set_fail_popup);
    lv_label_set_text(info, "Please remove banknotes from the feeder or reject pocket first.");
    lv_obj_set_width(info, 620);
    lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(info, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(info, &lv_font_instrument_sans_medium_20, 0);
    lv_obj_set_style_text_color(info, lv_color_hex(0x3C4D61), 0);
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 84);

    lv_obj_t* ok_btn = lv_btn_create(g_batch_set_fail_popup);
    lv_obj_set_size(ok_btn, 180, 58);
    lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_radius(ok_btn, 16, 0);
    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0x1B86FF), 0);
    lv_obj_set_style_bg_opa(ok_btn, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(ok_btn, batch_set_fail_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "CONFIRM");
    lv_obj_set_style_text_font(ok_label, &lv_font_instrument_sans_bold_22, 0);
    lv_obj_set_style_text_color(ok_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(ok_label);
}

void show_currency_set_fail_popup(void)
{
    if (g_curr_set_fail_popup && lv_obj_is_valid(g_curr_set_fail_popup)) {
        return;
    }

    lv_obj_t* scr = lv_scr_act();
    g_curr_set_fail_mask = lv_obj_create(scr);
    lv_obj_remove_style_all(g_curr_set_fail_mask);
    lv_obj_set_size(g_curr_set_fail_mask, 1280, 400);
    lv_obj_set_style_bg_opa(g_curr_set_fail_mask, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(g_curr_set_fail_mask, lv_color_hex(0x000000), 0);

    g_curr_set_fail_popup = lv_obj_create(scr);
    lv_obj_set_size(g_curr_set_fail_popup, 760, 280);
    lv_obj_center(g_curr_set_fail_popup);
    lv_obj_clear_flag(g_curr_set_fail_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_curr_set_fail_popup, 24, 0);
    lv_obj_set_style_bg_color(g_curr_set_fail_popup, lv_color_hex(0xF4F7FB), 0);
    lv_obj_set_style_border_width(g_curr_set_fail_popup, 2, 0);
    lv_obj_set_style_border_color(g_curr_set_fail_popup, lv_color_hex(0xD7DEE8), 0);

    lv_obj_t* title = lv_label_create(g_curr_set_fail_popup);
    lv_label_set_text(title, "CURRENCY SET FAILED");
    lv_obj_set_style_text_font(title, &lv_font_instrument_sans_semibold_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x2D3A4A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 22);

    lv_obj_t* info = lv_label_create(g_curr_set_fail_popup);
    lv_label_set_text(info,
        "There are banknotes still inside the machine or the sensor is abnormal.\n"
        "Currency change was rejected.");
    lv_obj_set_width(info, 680);
    lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(info, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(info, &lv_font_instrument_sans_medium_20, 0);
    lv_obj_set_style_text_color(info, lv_color_hex(0x3C4D61), 0);
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 82);

    lv_obj_t* ok_btn = lv_btn_create(g_curr_set_fail_popup);
    lv_obj_set_size(ok_btn, 180, 58);
    lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_radius(ok_btn, 16, 0);
    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0x1B86FF), 0);
    lv_obj_set_style_bg_opa(ok_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ok_btn, 0, 0);
    lv_obj_add_event_cb(ok_btn, currency_set_fail_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "CONFIRM");
    lv_obj_set_style_text_font(ok_label, &lv_font_instrument_sans_bold_22, 0);
    lv_obj_set_style_text_color(ok_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(ok_label);
}

static void update_switch_visual(bool enable, bool animate) {
    lv_coord_t cont_w = lv_obj_get_width(batch_switch.switch_container);
    lv_coord_t knob_w = lv_obj_get_width(batch_switch.switch_knob);

    if (enable) {
        //  ON 标签
        lv_obj_set_style_bg_color(batch_switch.switch_container, lv_palette_main(LV_PALETTE_GREEN), 0);

        lv_label_set_text(batch_switch.label_on, "ON");
        lv_obj_align(batch_switch.label_on, LV_ALIGN_LEFT_MID, 3, 0);
        lv_obj_clear_flag(batch_switch.label_on, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(batch_switch.label_off, LV_OBJ_FLAG_HIDDEN);

        // 滑块动画
        lv_coord_t target_x = cont_w - knob_w - 8;
        if (animate && lv_obj_get_x(batch_switch.switch_knob) != target_x) {
            // 执行动画
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, batch_switch.switch_knob);
            lv_anim_set_values(&a, lv_obj_get_x(batch_switch.switch_knob), target_x);
            lv_anim_set_time(&a, 250);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_start(&a);
        }
        else {
            lv_obj_set_x(batch_switch.switch_knob, target_x);
        }

    }
    else {
        // OFF 标签
        lv_obj_set_style_bg_color(batch_switch.switch_container, lv_palette_main(LV_PALETTE_GREY), 0);

        lv_label_set_text(batch_switch.label_off, "OFF");
        lv_obj_align(batch_switch.label_off, LV_ALIGN_RIGHT_MID, -3, 0);
        lv_obj_clear_flag(batch_switch.label_off, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_flag(batch_switch.label_on, LV_OBJ_FLAG_HIDDEN);

        lv_coord_t target_x = 4;
        if (animate && lv_obj_get_x(batch_switch.switch_knob) != target_x) {
            // 执行动画
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, batch_switch.switch_knob);
            lv_anim_set_values(&a, lv_obj_get_x(batch_switch.switch_knob), target_x);
            lv_anim_set_time(&a, 250);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_start(&a);
        }
        else {
            // 直接设置位置，无动画
            lv_obj_set_x(batch_switch.switch_knob, target_x);
        }
    }
    page_03_batch_mode_status_refre();
#if LV_DEBUG
    printf("batch_switch_status: %s\n", Machine_para.batch_switch_enable ? "ON" : "OFF");
#endif // LV_DEBUG

}

// 点击切换状态
static void switch_event_cb(lv_event_t* e) {
    LV_UNUSED(e);
    if (g_batch_switch_wait_ack) return;

    g_batch_switch_prev_state = Machine_para.batch_switch_enable;
    g_batch_switch_target_state = !g_batch_switch_prev_state;
    g_batch_switch_wait_ack = true;

    /* batch 开关行为：
     * ON  -> 发送用户预设的 pcs batch
     * OFF -> 固定发送 200
     */
    uint8_t batch_cmd = 200;
    if (g_batch_switch_target_state) {
        int preset = Machine_para.batch_num;
        if (preset <= 0 || preset >= 200) {
            preset = g_batch_last_on_num;
        }
        if (preset < 5) preset = 5;
        if (preset > 199) preset = 199;
        batch_cmd = (uint8_t)preset;
    }
    g_batch_switch_sent_num = batch_cmd;
    send_command(fd4, 0x06, &batch_cmd, 1);
}

void batch_switch_on_0x06_result(uint8_t status)
{
    if (!g_batch_switch_wait_ack) return;

    if (status == 0x01) {
        Machine_para.batch_switch_enable = g_batch_switch_target_state;
        if (Machine_para.batch_switch_enable) {
            Machine_para.batch_num = g_batch_switch_sent_num;
            if (Machine_para.batch_num >= 5 && Machine_para.batch_num <= 199) {
                g_batch_last_on_num = (uint8_t)Machine_para.batch_num;
            }
        } else {
            Machine_para.batch_num = 200;
        }
        update_switch_visual(Machine_para.batch_switch_enable, true);
        page_03_batch_num_refre();
        page_01_batch_refre();
        g_batch_switch_wait_ack = false;
        return;
    }

    if (status == 0x02) {
        Machine_para.batch_switch_enable = g_batch_switch_prev_state;
        g_batch_switch_wait_ack = false;
    }
}

// 创建批次开关组件
void create_batch_num_switch(lv_obj_t* parent) {
    // 自适应宽度
    lv_obj_t* tmp = lv_label_create(parent);
    lv_obj_set_style_text_font(tmp, &lv_font_instrument_sans_medium_24, 0);
    lv_label_set_text(tmp, "OFF");
    lv_obj_update_layout(tmp);
    lv_coord_t txt_w = lv_obj_get_width(tmp);
    lv_obj_del(tmp);

    // 创建容器
    batch_switch.switch_container = lv_obj_create(parent);
    lv_obj_set_size(batch_switch.switch_container, txt_w + 44, 40);
    lv_obj_set_style_radius(batch_switch.switch_container, 20, 0);
    lv_obj_set_style_pad_all(batch_switch.switch_container, 0, 0);
    lv_obj_set_style_bg_color(batch_switch.switch_container,
        Machine_para.batch_switch_enable ?
        lv_palette_main(LV_PALETTE_GREEN) :
        lv_palette_main(LV_PALETTE_GREY), 0);

    lv_obj_add_flag(batch_switch.switch_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(batch_switch.switch_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(batch_switch.switch_container, switch_event_cb, LV_EVENT_CLICKED, NULL);

    // 创建 OFF 标签
    batch_switch.label_off = lv_label_create(batch_switch.switch_container);
    lv_label_set_text(batch_switch.label_off, "OFF");
    lv_obj_set_style_text_font(batch_switch.label_off, &lv_font_instrument_sans_medium_24, 0);
    lv_obj_set_style_text_color(batch_switch.label_off, lv_color_white(), 0);

    // 创建 ON 标签
    batch_switch.label_on = lv_label_create(batch_switch.switch_container);
    lv_label_set_text(batch_switch.label_on, "ON");
    lv_obj_set_style_text_font(batch_switch.label_on, &lv_font_instrument_sans_medium_24, 0);
    lv_obj_set_style_text_color(batch_switch.label_on, lv_color_white(), 0);

    // 创建滑块
    batch_switch.switch_knob = lv_obj_create(batch_switch.switch_container);
    lv_obj_set_size(batch_switch.switch_knob, 30, 30);
    lv_obj_set_style_radius(batch_switch.switch_knob, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(batch_switch.switch_knob, lv_color_white(), 0);
    lv_obj_set_style_shadow_width(batch_switch.switch_knob, 5, 0);
    lv_obj_set_style_shadow_color(batch_switch.switch_knob, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(batch_switch.switch_knob, LV_OPA_20, 0);
    lv_obj_add_flag(batch_switch.switch_knob, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(batch_switch.switch_knob, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(batch_switch.switch_knob, switch_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_update_layout(batch_switch.switch_container);
    lv_obj_update_layout(batch_switch.switch_knob);

    // 居中Y
    lv_obj_set_y(batch_switch.switch_knob,
        (lv_obj_get_height(batch_switch.switch_container) - lv_obj_get_height(batch_switch.switch_knob)) / 2 - 2
    );

    // 设置初始状态，无动画
    update_switch_visual(Machine_para.batch_switch_enable, false);
}

// 外部调用：获取容器
lv_obj_t* get_batch_switch_container(void) {
    return batch_switch.switch_container;
}

//设置开关状态（自动更新UI，无动画）
void set_batch_switch_state(bool enable) {
    Machine_para.batch_switch_enable = enable;
    if (Machine_para.batch_num >= 5 && Machine_para.batch_num <= 199) {
        g_batch_last_on_num = (uint8_t)Machine_para.batch_num;
    }

    if (batch_switch.switch_container == NULL ||
        batch_switch.switch_knob == NULL ||
        batch_switch.label_on == NULL ||
        batch_switch.label_off == NULL) {
        return;
    }

    if (!lv_obj_is_valid(batch_switch.switch_container) ||
        !lv_obj_is_valid(batch_switch.switch_knob) ||
        !lv_obj_is_valid(batch_switch.label_on) ||
        !lv_obj_is_valid(batch_switch.label_off)) {
        return;
    }

    // 外部调用时不执行动画
    update_switch_visual(enable, false);
}

// 记录最近一次可恢复的 batch 数值
void batch_switch_set_last_on_num(uint8_t num)
{
    if (num >= 5 && num <= 199) {
        g_batch_last_on_num = num;
    }
}


static void zoom_anim_cb(void* var, int32_t zoom)
{
    lv_img_set_zoom((lv_obj_t*)var,zoom);
}

void icon_feedback_comp(const char* name,ui_element_t* page_cfg_obj,int len)
{
    lv_obj_t* img = find_obj_by_name(name,page_cfg_obj,len);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a,img);
    lv_anim_set_exec_cb(&a, zoom_anim_cb);
    lv_anim_set_values(&a,256,285);
    lv_anim_set_time(&a, 100);
    lv_anim_set_playback_time(&a, 100);  // 回缩动画时间
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}



const char* get_system_error_desc(uint8_t code)
{
    switch (code) {
    case 0x00: return "No Error";
    case 0x01: return "Feeder Jam";
    case 0x02: return "Upper passage Jam";
    case 0x03: return "Lower passage Jam";
    case 0x04: return "Reject Exit Jam";
    case 0x05: return "Stacker Exit Jam";
    case 0x06: return "Diverter Solenoid Fault";
    case 0x07: return "Stacker Pocket Residual Note";
    default:   return "Unknown Fault";
    }
}

static lv_obj_t* g_sys_err_mask = NULL;
static lv_obj_t* g_sys_err_popup = NULL;
static lv_obj_t* g_sys_err_info_label = NULL;
uint8_t g_sys_err_last_code = 0x00;

void hide_system_error_popup(void)
{
    if (g_sys_err_popup && lv_obj_is_valid(g_sys_err_popup)) {
        lv_obj_del(g_sys_err_popup);
    }
    g_sys_err_popup = NULL;
    g_sys_err_info_label = NULL;

    if (g_sys_err_mask && lv_obj_is_valid(g_sys_err_mask)) {
        lv_obj_del(g_sys_err_mask);
    }
    g_sys_err_mask = NULL;
}

void system_error_confirm_cb(lv_event_t* e)
{
    if ((lv_event_code_t)lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    uint8_t clear_cmd = 0x01;
    send_command(fd4, 0x3D, &clear_cmd, 1); /* FD DF 06 3D 01 0A */
    hide_system_error_popup();
    g_sys_err_last_code = 0x00;
}

void show_system_error_popup(uint8_t code)
{
    if (code == 0x00) return;
    if (g_sys_err_popup && lv_obj_is_valid(g_sys_err_popup)) {
        if (g_sys_err_last_code == code) {
            return;
        }
        if (g_sys_err_info_label && lv_obj_is_valid(g_sys_err_info_label)) {
            lv_label_set_text_fmt(g_sys_err_info_label, "%s", get_system_error_desc(code));
        }
        g_sys_err_last_code = code;
        return;
    }

    lv_obj_t* scr = lv_scr_act();
    g_sys_err_mask = lv_obj_create(scr);
    lv_obj_remove_style_all(g_sys_err_mask);
    lv_obj_set_size(g_sys_err_mask, 1280, 400);
    lv_obj_set_style_bg_opa(g_sys_err_mask, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(g_sys_err_mask, lv_color_hex(0x000000), 0);

    g_sys_err_popup = lv_obj_create(scr);
    lv_obj_set_size(g_sys_err_popup, 620, 250);
    lv_obj_center(g_sys_err_popup);
    lv_obj_clear_flag(g_sys_err_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_sys_err_popup, 26, 0);
    lv_obj_set_style_bg_color(g_sys_err_popup, lv_color_hex(0xF4F7FB), 0);
    lv_obj_set_style_border_width(g_sys_err_popup, 2, 0);
    lv_obj_set_style_border_color(g_sys_err_popup, lv_color_hex(0xD7DEE8), 0);
    lv_obj_set_style_shadow_width(g_sys_err_popup, 18, 0);
    lv_obj_set_style_shadow_opa(g_sys_err_popup, LV_OPA_40, 0);

    lv_obj_t* title = lv_label_create(g_sys_err_popup);
    lv_label_set_text(title, "SYSTEM ERROR");
    lv_obj_set_style_text_font(title, &lv_font_instrument_sans_semibold_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x2D3A4A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 22);

    g_sys_err_info_label = lv_label_create(g_sys_err_popup);
    lv_label_set_text_fmt(g_sys_err_info_label, "%s", get_system_error_desc(code));
    lv_obj_set_width(g_sys_err_info_label, 560);
    lv_label_set_long_mode(g_sys_err_info_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(g_sys_err_info_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(g_sys_err_info_label, &lv_font_instrument_sans_medium_22, 0);
    lv_obj_set_style_text_color(g_sys_err_info_label, lv_color_hex(0x3C4D61), 0);
    lv_obj_align(g_sys_err_info_label, LV_ALIGN_TOP_MID, 0, 88);

    lv_obj_t* ok_btn = lv_btn_create(g_sys_err_popup);
    lv_obj_set_size(ok_btn, 180, 58);
    lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_radius(ok_btn, 16, 0);
    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0x1B86FF), 0);
    lv_obj_set_style_bg_opa(ok_btn, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(ok_btn, system_error_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "CONFIRM");
    lv_obj_set_style_text_font(ok_label, &lv_font_instrument_sans_bold_22, 0);
    lv_obj_set_style_text_color(ok_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(ok_label);

    g_sys_err_last_code = code;
}

/* 启动点钞(0x0A) type=0x01: 异常原因表 */
static const char* g_counting_start_error_desc[0x100] = {
    [0x02] = "Please Place Banknotes in the Hopper",
};

/* 启动点钞(0x0A) type=0x02: 设备故障码 */
static const char* g_counting_fault_error_desc[0x100] = {
    [0x01] = "Upper Channel Sensor Blocked",
    [0x02] = "Lower Channel Sensor Blocked",
    [0x03] = "Reject Exit Sensor Blocked",
    [0x04] = "Reject Pocket Sensor Blocked",
    [0x05] = "Reject Pocket Full",
    [0x06] = "Stacker Pocket Sensor Blocked",
    [0x07] = "Stacker Pocket Full",
    [0x08] = "Stacker & Reject Pockets Full",
    [0x09] = "Upper & Lower Channels Open",
    [0x0A] = "Genuine Exit Sensor Blocked",
    [0x0B] = "Dust Cover / Baffle Not Closed",
    [0x0C] = "Flipper Position Fault",
    [0x0D] = "Encoder Fault",
};

const char* get_counting_error_desc(uint8_t type, uint8_t code)
{
    if (type == 0x01) {
        if (g_counting_start_error_desc[code] != NULL) {
            return g_counting_start_error_desc[code];
        }
        return "Start Counting Failed";
    }

    if (type == 0x02) {
        if (g_counting_fault_error_desc[code] != NULL) {
            return g_counting_fault_error_desc[code];
        }
        return "Counting Fault";
    }

    return "Unknown Counting Fault";
}

static const char* get_counting_ui_error_desc(uint8_t type, uint8_t code)
{
    if (type == 0x01 && (code == 0x00 || code == 0x02)) {
        return ui_text_get(UI_TEXT_WIDGET_FAULT_NO_NOTE_MAIN);
    }

    if (code < sizeof(g_start_error_desc) / sizeof(g_start_error_desc[0]) &&
        g_start_error_desc[code] != NULL) {
        return g_start_error_desc[code];
    }

    return ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR);
}

static lv_obj_t* g_count_err_mask = NULL;
static lv_obj_t* g_count_err_popup = NULL;
static lv_obj_t* g_count_err_info_label = NULL;
static uint8_t g_count_err_last_code = 0x00;
static uint8_t g_count_err_last_type = 0x00;

void hide_counting_error_popup(void)
{
    if (g_count_err_popup && lv_obj_is_valid(g_count_err_popup)) {
        lv_obj_del(g_count_err_popup);
    }
    g_count_err_popup = NULL;
    g_count_err_info_label = NULL;

    if (g_count_err_mask && lv_obj_is_valid(g_count_err_mask)) {
        lv_obj_del(g_count_err_mask);
    }
    g_count_err_mask = NULL;
}

void counting_error_confirm_cb(lv_event_t* e)
{
    if ((lv_event_code_t)lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    /* 与系统报错确认一致：发送清除命令 */
    uint8_t clear_cmd = 0x01;
    send_command(fd4, 0x3D, &clear_cmd, 1); /* FD DF 06 3D 01 0A */
    hide_counting_error_popup();
    g_count_err_last_code = 0x00;
    g_count_err_last_type = 0x00;
}

void show_counting_error_popup(uint8_t type, uint8_t code)
{
    if (code == 0x00) return;
    if (g_count_err_popup && lv_obj_is_valid(g_count_err_popup)) {
        if (g_count_err_last_type == type && g_count_err_last_code == code) {
            return;
        }
        if (g_count_err_info_label && lv_obj_is_valid(g_count_err_info_label)) {
            lv_label_set_text_fmt(g_count_err_info_label, "%s", get_counting_ui_error_desc(type, code));
        }
        g_count_err_last_type = type;
        g_count_err_last_code = code;
        return;
    }

    lv_obj_t* scr = lv_scr_act();
    g_count_err_mask = lv_obj_create(scr);
    lv_obj_remove_style_all(g_count_err_mask);
    lv_obj_set_size(g_count_err_mask, 1280, 400);
    lv_obj_set_style_bg_opa(g_count_err_mask, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(g_count_err_mask, lv_color_hex(0x000000), 0);

    g_count_err_popup = lv_obj_create(scr);
    lv_obj_set_size(g_count_err_popup, 620, 250);
    lv_obj_center(g_count_err_popup);
    lv_obj_clear_flag(g_count_err_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_count_err_popup, 26, 0);
    lv_obj_set_style_bg_color(g_count_err_popup, lv_color_hex(0xF4F7FB), 0);
    lv_obj_set_style_border_width(g_count_err_popup, 2, 0);
    lv_obj_set_style_border_color(g_count_err_popup, lv_color_hex(0xD7DEE8), 0);
    lv_obj_set_style_shadow_width(g_count_err_popup, 18, 0);
    lv_obj_set_style_shadow_opa(g_count_err_popup, LV_OPA_40, 0);

    lv_obj_t* title = lv_label_create(g_count_err_popup);
    lv_label_set_text(title, "COUNTING ERROR");
    lv_obj_set_style_text_font(title, &lv_font_instrument_sans_semibold_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x2D3A4A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 22);

    g_count_err_info_label = lv_label_create(g_count_err_popup);
    lv_label_set_text_fmt(g_count_err_info_label, "%s", get_counting_ui_error_desc(type, code));
    lv_obj_set_width(g_count_err_info_label, 560);
    lv_label_set_long_mode(g_count_err_info_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(g_count_err_info_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(g_count_err_info_label, &lv_font_instrument_sans_medium_22, 0);
    lv_obj_set_style_text_color(g_count_err_info_label, lv_color_hex(0x3C4D61), 0);
    lv_obj_align(g_count_err_info_label, LV_ALIGN_TOP_MID, 0, 88);

    lv_obj_t* ok_btn = lv_btn_create(g_count_err_popup);
    lv_obj_set_size(ok_btn, 180, 58);
    lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_radius(ok_btn, 16, 0);
    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0x1B86FF), 0);
    lv_obj_set_style_bg_opa(ok_btn, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(ok_btn, counting_error_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "CONFIRM");
    lv_obj_set_style_text_font(ok_label, &lv_font_instrument_sans_bold_22, 0);
    lv_obj_set_style_text_color(ok_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(ok_label);

    g_count_err_last_type = type;
    g_count_err_last_code = code;
}
