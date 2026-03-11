#include "lv_fault_popup.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_system/user_cfg.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define FAULT_POPUP_BG_PATH   "L:/usr/local/share/lvgl_data/fault_popup_bg.png"
#define FAULT_MACHINE_IMG_FMT "L:/usr/local/share/lvgl_data/%02Xmachine.png"
#define FAULT_ERR_IMG_FMT     "L:/usr/local/share/lvgl_data/%02X_err.png"

static lv_obj_t* g_fault_popup = NULL;
static lv_obj_t* g_fault_machine_img = NULL;
static lv_obj_t* g_fault_err_img = NULL;
static lv_obj_t* g_fault_time_label = NULL;
static lv_obj_t* g_fault_model_label = NULL;
static lv_obj_t* g_fault_desc_label = NULL;
static lv_obj_t* g_fault_cause_label = NULL;
static lv_obj_t* g_fault_solution_label = NULL;
static lv_obj_t* g_fault_version_label = NULL;

static lv_obj_t* g_fault_anim_parent = NULL;
static lv_obj_t* g_fault_ripple_1 = NULL;
static lv_obj_t* g_fault_ripple_2 = NULL;
static lv_timer_t* g_fault_anim_timer = NULL;
static uint16_t g_fault_anim_tick = 0;
#ifndef MACHINE_MODEL_NAME
#define MACHINE_MODEL_NAME "UN260"
#endif
static const char* g_fault_reason_desc[0x100] = {
    [0x01] = "Sensor signal abnormal.",
    [0x02] = "Transport channel blocked.",
    [0x03] = "Motor feedback abnormal.",
    [0x04] = "Image board communication abnormal.",
};

static const char* g_fault_solution_desc[0x100] = {
    [0x01] = "Check sensor wiring and clean the sensor area.",
    [0x02] = "Open the channel and remove the jammed note.",
    [0x03] = "Check motor wiring and retry after power cycle.",
    [0x04] = "Check image board cable and restart the machine.",
};

static const char* get_fault_desc(uint8_t code)
{
    if (code < 0x32 && g_currency_error_desc[code] != NULL) {
        return g_currency_error_desc[code];
    }
    return "Unknown fault";
}

static const char* get_fault_reason(uint8_t code)
{
    if (g_fault_reason_desc[code] != NULL) {
        return g_fault_reason_desc[code];
    }
    return "Please check machine status and related hardware.";
}

static const char* get_fault_solution(uint8_t code)
{
    if (g_fault_solution_desc[code] != NULL) {
        return g_fault_solution_desc[code];
    }
    return "Press CONFIRM after checking the machine.";
}

static const char* get_fault_machine_img(uint8_t code)
{
    static char path[128];
    snprintf(path, sizeof(path), FAULT_MACHINE_IMG_FMT, code);
    return path;
}

static const char* get_fault_err_img(uint8_t code)
{
    static char path[128];
    snprintf(path, sizeof(path), FAULT_ERR_IMG_FMT, code);
    return path;
}

static void get_fault_time_str(char* buf, size_t size)
{
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);

    if (tm_now == NULL) {
        snprintf(buf, size, "--:--:--");
        return;
    }

    snprintf(buf, size, "%04d-%02d-%02d %02d:%02d:%02d",
             tm_now->tm_year + 1900,
             tm_now->tm_mon + 1,
             tm_now->tm_mday,
             tm_now->tm_hour,
             tm_now->tm_min,
             tm_now->tm_sec);
}
static void fault_anim_timer_cb(lv_timer_t* timer)
{
    (void)timer;

    if (g_fault_anim_parent == NULL) return;
    if (g_fault_ripple_1 == NULL || g_fault_ripple_2 == NULL) return;

    g_fault_anim_tick++;

    /* 一个完整周期 0~79 */
    uint16_t phase1 = g_fault_anim_tick % 80;
    uint16_t phase2 = (g_fault_anim_tick + 40) % 80;

    lv_obj_t* ripples[2] = { g_fault_ripple_1, g_fault_ripple_2 };
    uint16_t phases[2] = { phase1, phase2 };

    for (int i = 0; i < 2; i++) {
        uint16_t p = phases[i];

        /* 前 60 帧扩散，后 20 帧隐藏等待 */
        if (p < 60) {
            lv_coord_t size = 18 + (p * 78) / 60;   // 18 -> 96
            lv_opa_t opa = 220 - (p * 200) / 60;    // 220 -> 20

            lv_obj_clear_flag(ripples[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(ripples[i], size, size);
            lv_obj_set_pos(ripples[i], 110 - size / 2, 118 - size / 2);
            lv_obj_set_style_border_opa(ripples[i], opa, 0);
        } else {
            lv_obj_add_flag(ripples[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}
static void fault_popup_confirm_cb(lv_event_t* e)
{
    (void)e;
    hide_fault_popup();
}

static lv_obj_t* create_info_box(lv_obj_t* parent,
                                 lv_coord_t x, lv_coord_t y,
                                 lv_coord_t w, lv_coord_t h,
                                 lv_color_t bg,
                                 lv_color_t border)
{
    lv_obj_t* box = lv_obj_create(parent);
    lv_obj_set_size(box, w, h);
    lv_obj_set_pos(box, x, y);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(box, 14, 0);
    lv_obj_set_style_bg_color(box, bg, 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(box, 1, 0);
    lv_obj_set_style_border_color(box, border, 0);
    lv_obj_set_style_shadow_width(box, 0, 0);
    return box;
}

bool fault_popup_is_showing(void)
{
    return g_fault_popup != NULL;
}

void hide_fault_popup(void)
{
    if (g_fault_anim_timer) {
        lv_timer_del(g_fault_anim_timer);
        g_fault_anim_timer = NULL;
    }

    if (g_fault_popup) {
        lv_obj_del(g_fault_popup);
        g_fault_popup = NULL;
    }

    g_fault_machine_img = NULL;
    g_fault_err_img = NULL;
    g_fault_time_label = NULL;
    g_fault_model_label = NULL;
    g_fault_desc_label = NULL;
    g_fault_cause_label = NULL;
    g_fault_solution_label = NULL;
    g_fault_version_label = NULL;
    g_fault_anim_parent = NULL;
    g_fault_ripple_1 = NULL;
    g_fault_ripple_2 = NULL;
}

void show_fault_popup(uint8_t fault_code)
{
    char time_buf[64];
    const char* desc = get_fault_desc(fault_code);
    const char* reason = get_fault_reason(fault_code);
    const char* solution = get_fault_solution(fault_code);

    hide_fault_popup();

    get_fault_time_str(time_buf, sizeof(time_buf));

    g_fault_popup = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(g_fault_popup);
    lv_obj_set_size(g_fault_popup, 1230, 368);
    lv_obj_center(g_fault_popup);
    lv_obj_clear_flag(g_fault_popup, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* bg = lv_img_create(g_fault_popup);
    lv_img_set_src(bg, FAULT_POPUP_BG_PATH);
    lv_obj_set_size(bg, 1230, 368);
    lv_obj_center(bg);

    /* 左侧图片区 */
    g_fault_anim_parent = lv_obj_create(g_fault_popup);
    lv_obj_set_size(g_fault_anim_parent, 540, 320);
    lv_obj_set_pos(g_fault_anim_parent, 8, 24);
    lv_obj_clear_flag(g_fault_anim_parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(g_fault_anim_parent, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_fault_anim_parent, 0, 0);
    lv_obj_set_style_shadow_width(g_fault_anim_parent, 0, 0);

    g_fault_machine_img = lv_img_create(g_fault_anim_parent);
    lv_img_set_src(g_fault_machine_img, get_fault_machine_img(fault_code));
    lv_obj_set_pos(g_fault_machine_img, 6, 16);

    g_fault_err_img = lv_img_create(g_fault_anim_parent);
    lv_img_set_src(g_fault_err_img, get_fault_err_img(fault_code));
    lv_obj_set_pos(g_fault_err_img, 280, 16);

    g_fault_ripple_1 = lv_obj_create(g_fault_anim_parent);
    lv_obj_set_size(g_fault_ripple_1, 18, 18);
    lv_obj_set_pos(g_fault_ripple_1, 110 - 9, 118 - 9);
    lv_obj_clear_flag(g_fault_ripple_1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_fault_ripple_1, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(g_fault_ripple_1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_fault_ripple_1, 2, 0);
    lv_obj_set_style_border_color(g_fault_ripple_1, lv_color_hex(0xFF3B30), 0);
    lv_obj_set_style_border_opa(g_fault_ripple_1, LV_OPA_90, 0);
    lv_obj_set_style_shadow_width(g_fault_ripple_1, 0, 0);
    lv_obj_set_style_outline_width(g_fault_ripple_1, 0, 0);

    g_fault_ripple_2 = lv_obj_create(g_fault_anim_parent);
    lv_obj_set_size(g_fault_ripple_2, 18, 18);
    lv_obj_set_pos(g_fault_ripple_2, 110 - 9, 118 - 9);
    lv_obj_clear_flag(g_fault_ripple_2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_fault_ripple_2, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(g_fault_ripple_2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_fault_ripple_2, 2, 0);
    lv_obj_set_style_border_color(g_fault_ripple_2, lv_color_hex(0xFF3B30), 0);
    lv_obj_set_style_border_opa(g_fault_ripple_2, LV_OPA_90, 0);
    lv_obj_set_style_shadow_width(g_fault_ripple_2, 0, 0);
    lv_obj_set_style_outline_width(g_fault_ripple_2, 0, 0);

    g_fault_anim_tick = 0;
    g_fault_anim_timer = lv_timer_create(fault_anim_timer_cb, 40, NULL);



    g_fault_time_label = lv_label_create(g_fault_popup);
    lv_label_set_text_fmt(g_fault_time_label, "TIME: %s", time_buf);
    lv_obj_set_style_text_color(g_fault_time_label, lv_color_hex(0x666666), 0);
    lv_obj_set_style_text_font(g_fault_time_label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(g_fault_time_label, 630, 20);

    g_fault_model_label = lv_label_create(g_fault_popup);
    lv_label_set_text_fmt(g_fault_model_label, "MODEL: %s", MACHINE_MODEL_NAME);
    lv_obj_set_style_text_color(g_fault_model_label, lv_color_hex(0x666666), 0);
    lv_obj_set_style_text_font(g_fault_model_label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(g_fault_model_label, 900, 20);

    /* 故障描述框 */
    lv_obj_t* desc_box = create_info_box(
        g_fault_popup, 600, 101, 604, 60,
        lv_color_hex(0xFEEFEE), lv_color_hex(0xFEC7C4));
    lv_obj_set_style_bg_opa(desc_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(desc_box, LV_OPA_TRANSP, 0);

    g_fault_cause_label = lv_label_create(desc_box);
    lv_label_set_text_fmt(g_fault_cause_label, "CAUSE: %s", reason);
    lv_obj_set_width(g_fault_cause_label, 560);
    lv_label_set_long_mode(g_fault_cause_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(g_fault_cause_label, lv_color_hex(0x555555), 0);
    lv_obj_set_style_text_font(g_fault_cause_label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(g_fault_cause_label, 29, 10);
    /* 分隔线 */


    /* 原因+方案框 */
    lv_obj_t* info_box = create_info_box(
        g_fault_popup, 600, 214, 604, 85,
        lv_color_hex(0xF8F8F8), lv_color_hex(0xF3F4F6));
    lv_obj_set_style_bg_opa(info_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(info_box, LV_OPA_TRANSP, 0);




    g_fault_solution_label = lv_label_create(info_box);
    lv_label_set_text_fmt(g_fault_solution_label, "SOLUTION: %s", solution);
    lv_obj_set_width(g_fault_solution_label, 560);
    lv_label_set_long_mode(g_fault_solution_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(g_fault_solution_label, lv_color_hex(0x555555), 0);
    lv_obj_set_style_text_font(g_fault_solution_label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(g_fault_solution_label, 10, 28);

    g_fault_version_label = lv_label_create(g_fault_popup);
    lv_label_set_text_fmt(g_fault_version_label, "MAIN-APP: %s", Machine_Statue.main_app);
    lv_obj_set_style_text_color(g_fault_version_label, lv_color_hex(0x666666), 0);
    lv_obj_set_style_text_font(g_fault_version_label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(g_fault_version_label, 590, 320);

    /* CONFIRM 按钮 */
    lv_obj_t* confirm_btn = lv_btn_create(g_fault_popup);
    lv_obj_set_size(confirm_btn, 120, 52);
    lv_obj_set_pos(confirm_btn, 1060, 306);
    lv_obj_set_style_radius(confirm_btn, 12, 0);
    lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0x1677FF), 0);
    lv_obj_set_style_border_width(confirm_btn, 0, 0);
    lv_obj_add_event_cb(confirm_btn, fault_popup_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "CONFIRM");
    lv_obj_set_style_text_color(confirm_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(confirm_label, &lv_font_montserrat_18, 0);
    lv_obj_center(confirm_label);
}