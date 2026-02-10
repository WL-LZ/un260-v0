#include "un260/lv_core/page_08_boot.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "../aic_ui/aic_ui.h"
#include "un260/lv_system/machine_time.h"
#include "page_11_timeset.h"
#include <string.h>

static lv_obj_t* timeset_page = NULL;
static lv_obj_t* lbl_time = NULL;

static lv_obj_t* r_year = NULL;
static lv_obj_t* r_mon = NULL;
static lv_obj_t* r_day = NULL;
static lv_obj_t* r_hour = NULL;
static lv_obj_t* r_min = NULL;
static lv_obj_t* r_sec = NULL;
static bool s_roller_syncing = false;

static void refresh_time_label(void)
{
    char buf[64];
    machine_time_format(buf, sizeof(buf));
    if (lbl_time && lv_obj_is_valid(lbl_time)) {
        lv_label_set_text(lbl_time, buf);
    }
}

static void roller_changed_cb(lv_event_t* e)
{
    if (s_roller_syncing) return;

    uint16_t y = (uint16_t)(2000 + lv_roller_get_selected(r_year));
    uint8_t  m = (uint8_t)(1 + lv_roller_get_selected(r_mon));
    uint8_t  d = (uint8_t)(1 + lv_roller_get_selected(r_day));
    uint8_t  hh = (uint8_t)(lv_roller_get_selected(r_hour));
    uint8_t  mm = (uint8_t)(lv_roller_get_selected(r_min));
    uint8_t  ss = (uint8_t)(lv_roller_get_selected(r_sec));

    machine_time_set(y, m, d, hh, mm, ss);
    machine_time_get(&y, &m, &d, &hh, &mm, &ss);
    s_roller_syncing = true;
    lv_roller_set_selected(r_year, (int)(y - 2000), LV_ANIM_OFF);
    lv_roller_set_selected(r_mon, (int)(m - 1), LV_ANIM_OFF);
    lv_roller_set_selected(r_day, (int)(d - 1), LV_ANIM_OFF);
    lv_roller_set_selected(r_hour, (int)hh, LV_ANIM_OFF);
    lv_roller_set_selected(r_min, (int)mm, LV_ANIM_OFF);
    lv_roller_set_selected(r_sec, (int)ss, LV_ANIM_OFF);
    s_roller_syncing = false;
    refresh_time_label();
}

static void roller_scroll_end_cb(lv_event_t* e)
{
    if ((lv_event_code_t)lv_event_get_code(e) != LV_EVENT_SCROLL_END) return;
    roller_changed_cb(e);
}

static void esc_btn_cb(lv_event_t* e)
{
    if ((lv_event_code_t)lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_pop_page();
}

static lv_obj_t* create_roller(lv_obj_t* parent, int x, int y, int w, int h,
    const char* opts, int sel)
{
    lv_obj_t* r = lv_roller_create(parent);
    lv_obj_set_pos(r, x, y);
    lv_obj_set_size(r, w, h);
    lv_roller_set_options(r, opts, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(r, 3);
    lv_roller_set_selected(r, sel, LV_ANIM_OFF);
    lv_obj_set_style_text_align(r, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_event_cb(r, roller_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(r, roller_scroll_end_cb, LV_EVENT_SCROLL_END, NULL);
    return r;
}


void ui_page_11_timeset_create(lv_obj_t* parent)
{
    (void)parent;
    if (timeset_page) return;

    /* 进入设置页时暂停自动走时，避免调节时被跳动 */
    machine_time_pause(true);

    timeset_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(timeset_page);
    lv_obj_set_pos(timeset_page, 0, 0);
    lv_obj_set_size(timeset_page, 1280, 400);
    lv_obj_clear_flag(timeset_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(timeset_page, lv_color_make(250, 250, 250), 0);
    lv_obj_set_style_bg_opa(timeset_page, LV_OPA_COVER, 0);

    /* 标题 */
    lv_obj_t* title = lv_label_create(timeset_page);
    lv_label_set_text(title, "TIME SET");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_make(112, 112, 112), 0);
    lv_obj_set_pos(title, 30, 18);

    /* ESC按钮(100*60 右上角) */
    lv_obj_t* esc = lv_btn_create(timeset_page);
    lv_obj_set_size(esc, 100, 60);
    lv_obj_set_pos(esc, 1280 - 100 - 20, 15);
    lv_obj_add_event_cb(esc, esc_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_radius(esc, 12, 0);
    lv_obj_set_style_bg_color(esc, lv_color_hex(0x3ec1f7), 0);
    lv_obj_set_style_bg_opa(esc, LV_OPA_COVER, 0);
    lv_obj_t* esc_label = lv_label_create(esc);
    lv_label_set_text(esc_label, "ESC");
    lv_obj_center(esc_label);
    lv_obj_set_style_text_font(esc_label, &lv_font_montserrat_20, 0);

    /* 当前时间显示 */
    lbl_time = lv_label_create(timeset_page);
    lv_obj_set_pos(lbl_time, 30, 70);
    lv_obj_set_size(lbl_time, 1220, 40);
    lv_obj_set_style_text_font(lbl_time, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(lbl_time, lv_color_make(0, 115, 255), 0);
    refresh_time_label();

    lv_obj_t* hint = lv_label_create(timeset_page);
    lv_label_set_text(hint, "Swipe/roll to set: YYYY/MM/DD/HH/MM/SS");
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(hint, lv_color_make(136, 137, 138), 0);
    lv_obj_set_pos(hint, 30, 118);

    /* 生成roller选项字符串 */
    static char opt_year[2048];
    static char opt_mon[128];
    static char opt_day[256];
    static char opt_h[256];
    static char opt_m[256];
    static char opt_s[256];

    opt_year[0] = '\0';
    opt_mon[0] = '\0';
    opt_day[0] = '\0';
    opt_h[0] = '\0';
    opt_m[0] = '\0';
    int pos = 0;
    for (int i = 2000; i <= 2099; i++) {
        pos += lv_snprintf(opt_year + pos, sizeof(opt_year) - pos, (i == 2099) ? "%d" : "%d\n", i);
    }
    pos = 0;
    for (int i = 1; i <= 12; i++) {
        pos += lv_snprintf(opt_mon + pos, sizeof(opt_mon) - pos, (i == 12) ? "%02d" : "%02d\n", i);
    }
    pos = 0;
    for (int i = 1; i <= 31; i++) {
        pos += lv_snprintf(opt_day + pos, sizeof(opt_day) - pos, (i == 31) ? "%02d" : "%02d\n", i);
    }
    pos = 0;
    for (int i = 0; i <= 23; i++) {
        pos += lv_snprintf(opt_h + pos, sizeof(opt_h) - pos, (i == 23) ? "%02d" : "%02d\n", i);
    }
    pos = 0;
    for (int i = 0; i <= 59; i++) {
        pos += lv_snprintf(opt_m + pos, sizeof(opt_m) - pos, (i == 59) ? "%02d" : "%02d\n", i);
    }
    memcpy(opt_s, opt_m, sizeof(opt_s));

    uint16_t y; uint8_t mo, d, hh, mm, ss;
    machine_time_get(&y, &mo, &d, &hh, &mm, &ss);

    int base_x = 30;
    int base_y = 140;
    int rw = 150;
    int rh = 200;
    int gap = 20;

    r_year = create_roller(timeset_page, base_x + (rw + gap) * 0, base_y, rw, rh, opt_year, (int)(y - 2000));
    r_mon = create_roller(timeset_page, base_x + (rw + gap) * 1, base_y, rw, rh, opt_mon, (int)(mo - 1));
    r_day = create_roller(timeset_page, base_x + (rw + gap) * 2, base_y, rw, rh, opt_day, (int)(d - 1));
    r_hour = create_roller(timeset_page, base_x + (rw + gap) * 3, base_y, rw, rh, opt_h, (int)(hh));
    r_min = create_roller(timeset_page, base_x + (rw + gap) * 4, base_y, rw, rh, opt_m, (int)(mm));
    r_sec = create_roller(timeset_page, base_x + (rw + gap) * 5, base_y, rw, rh, opt_s, (int)(ss));
}

void ui_page_11_timeset_destroy(void)
{
    if (timeset_page) {
        lv_obj_del(timeset_page);
        timeset_page = NULL;
        lbl_time = NULL;
        r_year = r_mon = r_day = r_hour = r_min = r_sec = NULL;
    }
    machine_time_pause(false);
}
