#include "lv_fault_popup.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_system/user_cfg.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef MACHINE_MODEL_NAME
#define MACHINE_MODEL_NAME "UN260"
#endif

#define FAULT_POPUP_BG_PATH   "L:/usr/local/share/lvgl_data/fault_popup_bg.png"
#define FAULT_MACHINE_IMG_FMT "L:/usr/local/share/lvgl_data/%02Xmachine.png"
#define FAULT_ERR_IMG_FMT     "L:/usr/local/share/lvgl_data/%02X_err.png"

static lv_obj_t* g_fault_popup = NULL;
static lv_obj_t* g_fault_machine_img = NULL;
static lv_obj_t* g_fault_err_img = NULL;
static lv_obj_t* g_fault_radar_1 = NULL;
static lv_obj_t* g_fault_radar_2 = NULL;
static lv_timer_t* g_fault_anim_timer = NULL;
static uint16_t g_fault_anim_tick = 0;

static lv_obj_t* g_fault_title_1 = NULL;
static lv_obj_t* g_fault_title_2 = NULL;
static lv_obj_t* g_fault_time_label = NULL;
static lv_obj_t* g_fault_model_label = NULL;
static lv_obj_t* g_fault_main_desc_label = NULL;
static lv_obj_t* g_fault_reason_label = NULL;
static lv_obj_t* g_fault_solution_label = NULL;
static lv_obj_t* g_fault_version_label = NULL;
static lv_obj_t* g_solution_title = NULL;
static lv_obj_t* g_reason_title = NULL;


static fault_popup_data_t g_fault_popup_data;

/* =========================
 * 原因 / 方案映射
 * ========================= */
static const char* g_boot_reason_desc[0x100] = {
    [0x01] = "Sensor self-test returned abnormal result.",
    [0x02] = "Motor self-test returned abnormal result.",
    [0x03] = "Electromagnet self-test returned abnormal result.",
    [0x04] = "Configuration readback abnormal.",
    [0x05] = "Image board self-test abnormal.",
};

static const char* g_boot_solution_desc[0x100] = {
    [0x01] = "Press CONFIRM and check sensor voltage page.",
    [0x02] = "Press CONFIRM and check motor related hardware.",
    [0x03] = "Press CONFIRM and check electromagnet wiring.",
    [0x04] = "Press CONFIRM and verify machine parameters.",
    [0x05] = "Press CONFIRM and check image board connection.",
};

static const char* g_start_reason_desc[0x100] = {
    [0x01] = "Machine state does not allow start counting.",
    [0x02] = "Detected abnormal machine status before counting.",
    [0x03] = "Sensor state is abnormal before start.",
    [0x04] = "Transport path may be blocked before counting.",
};

static const char* g_start_solution_desc[0x100] = {
    [0x01] = "Check machine status and retry.",
    [0x02] = "Remove abnormal notes or foreign objects and retry.",
    [0x03] = "Check sensor area and clean if needed.",
    [0x04] = "Open cover and remove jammed note before retry.",
};

static const char* g_runtime_reason_desc[0x100] = {
    [0x01] = "Upper channel sensor detected blockage.",
    [0x02] = "Lower channel sensor detected abnormal status.",
    [0x03] = "Transport path may contain foreign object.",
    [0x04] = "Machine path feedback is abnormal during counting.",
};

static const char* g_runtime_solution_desc[0x100] = {
    [0x01] = "Stop counting, open the cover, remove the blocked note, then close the cover.",
    [0x02] = "Check the lower path and clean the sensor area.",
    [0x03] = "Remove the foreign object and restart counting.",
    [0x04] = "Check the transmission path and restart the machine.",
};

/* =========================
 * 工具函数
 * ========================= */
static void get_time_str(char* buf, size_t size)
{
    time_t now = time(NULL);
    struct tm tm_now;
    struct tm* p = localtime_r(&now, &tm_now);
    if (p == NULL) {
        snprintf(buf, size, "--:--:--");
        return;
    }

    /*
     * Prefer device time from Machine_para when available.
     * Keep current localtime defaults if fields are out of range.
     */
    if (Machine_para.year > 0) {
        tm_now.tm_year = (Machine_para.year >= 1900) ? (Machine_para.year - 1900) : Machine_para.year;
    }
    if (Machine_para.month >= 0 && Machine_para.month <= 11) {
        tm_now.tm_mon = Machine_para.month;
    }
    if (Machine_para.day >= 1 && Machine_para.day <= 31) {
        tm_now.tm_mday = Machine_para.day;
    }
    if (Machine_para.hour >= 0 && Machine_para.hour <= 23) {
        tm_now.tm_hour = Machine_para.hour;
    }
    if (Machine_para.minute >= 0 && Machine_para.minute <= 59) {
        tm_now.tm_min = Machine_para.minute;
    }
    if (Machine_para.second >= 0 && Machine_para.second <= 59) {
        tm_now.tm_sec = Machine_para.second;
    }

    snprintf(buf, size, "%04d-%02d-%02d %02d:%02d:%02d",
             tm_now.tm_year + 1900,
             tm_now.tm_mon + 1,
             tm_now.tm_mday,
             tm_now.tm_hour,
             tm_now.tm_min,
             tm_now.tm_sec);
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

static void get_fault_radar_pos(fault_source_t source, uint8_t code, lv_coord_t* x, lv_coord_t* y)
{
    (void)source;

    /* 先统一默认位置，后面你可以按 code 单独扩 */
    *x = 110;
    *y = 118;

    if (code == 0x01) {
        *x = 110;
        *y = 118;
    }
}

static const char* get_boot_title(uint8_t code)
{
    switch (code) {
    case 0x01: return "SENSOR SELF-TEST ERROR";
    case 0x02: return "MOTOR SELF-TEST ERROR";
    case 0x03: return "ELECTROMAGNET SELF-TEST ERROR";
    case 0x04: return "CONFIG SELF-TEST ERROR";
    case 0x05: return "IMAGE BOARD SELF-TEST ERROR";
    default:   return "BOOT SELF-TEST ERROR";
    }
}

static const char* get_boot_main_desc(uint8_t code)
{
    switch (code) {
    case 0x01: return "Sensor Self-Test Failed";
    case 0x02: return "Motor Self-Test Failed";
    case 0x03: return "Electromagnet Self-Test Failed";
    case 0x04: return "Read Config Parameters Failed";
    case 0x05: return "Image Board Self-Test Failed";
    default:   return "Boot Self-Test Failed";
    }
}

static const char* get_start_title(uint8_t code)
{
    if (code < 0x32 && g_currency_error_desc[code] != NULL) {
        return "START COUNT ERROR";
    }
    return "START COUNT ERROR";
}

static const char* get_start_main_desc(uint8_t code)
{
    if (code < 0x32 && g_currency_error_desc[code] != NULL) {
        return g_currency_error_desc[code];
    }
    return "Unknown Start Fault";
}

static const char* get_runtime_title(uint8_t code)
{
    (void)code;
    return "ERROR SENSOR";
}

static const char* get_runtime_main_desc(uint8_t code)
{
    const char* desc = get_system_error_desc(code);
    return desc ? desc : "Unknown Runtime Fault";
}

static const char* get_fault_reason_text(fault_source_t source, uint8_t code)
{
    switch (source) {
    case FAULT_SRC_BOOT:
        if (g_boot_reason_desc[code]) return g_boot_reason_desc[code];
        break;
    case FAULT_SRC_START_COUNT:
        if (g_start_reason_desc[code]) return g_start_reason_desc[code];
        break;
    case FAULT_SRC_RUNTIME:
        if (g_runtime_reason_desc[code]) return g_runtime_reason_desc[code];
        break;
    default:
        break;
    }

    return "Please check machine status and related hardware.";
}

static const char* get_fault_solution_text(fault_source_t source, uint8_t code)
{
    switch (source) {
    case FAULT_SRC_BOOT:
        if (g_boot_solution_desc[code]) return g_boot_solution_desc[code];
        break;
    case FAULT_SRC_START_COUNT:
        if (g_start_solution_desc[code]) return g_start_solution_desc[code];
        break;
    case FAULT_SRC_RUNTIME:
        if (g_runtime_solution_desc[code]) return g_runtime_solution_desc[code];
        break;
    default:
        break;
    }

    return "Press CONFIRM after checking the machine.";
}

static fault_confirm_action_t get_confirm_action_by_page(void)
{
    if (ui_manager_get_current_page() == UI_PAGE_BOOT) {
        return FAULT_CONFIRM_GOTO_SENSOR;
    }
    return FAULT_CONFIRM_CLOSE;
}

/* =========================
 * 雷达动画
 * ========================= */
static void fault_anim_timer_cb(lv_timer_t* timer)
{
    (void)timer;

    if (g_fault_radar_1 == NULL || g_fault_radar_2 == NULL) return;

    g_fault_anim_tick++;

    uint16_t phase1 = g_fault_anim_tick % 80;
    uint16_t phase2 = (g_fault_anim_tick + 40) % 80;

    lv_obj_t* ripples[2] = { g_fault_radar_1, g_fault_radar_2 };
    uint16_t phases[2] = { phase1, phase2 };

    for (int i = 0; i < 2; i++) {
        uint16_t p = phases[i];

        if (p < 60) {
            lv_coord_t size = 18 + (p * 78) / 60;
            lv_opa_t opa = 220 - (p * 200) / 60;

            lv_obj_clear_flag(ripples[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_size(ripples[i], size, size);
            lv_obj_set_pos(ripples[i],
                           g_fault_popup_data.radar_x - size / 2,
                           g_fault_popup_data.radar_y - size / 2);
            lv_obj_set_style_border_opa(ripples[i], opa, 0);
        } else {
            lv_obj_add_flag(ripples[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

/* =========================
 * UI
 * ========================= */
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

static void fault_popup_confirm_cb(lv_event_t* e)
{
    (void)e;

    if (g_fault_popup_data.confirm_action == FAULT_CONFIRM_GOTO_SENSOR) {
        hide_fault_popup();
        ui_manager_push_page(UI_PAGE_SENSOR);
    } else {
        hide_fault_popup();
    }
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
    g_fault_radar_1 = NULL;
    g_fault_radar_2 = NULL;
    g_fault_title_1 = NULL;
    g_fault_title_2 = NULL;
    g_fault_time_label = NULL;
    g_fault_model_label = NULL;
    g_fault_main_desc_label = NULL;
    g_fault_reason_label = NULL;
    g_fault_solution_label = NULL;
    g_fault_version_label = NULL;

    memset(&g_fault_popup_data, 0, sizeof(g_fault_popup_data));
}

void show_fault_popup_ex(const fault_popup_data_t* data)
{
    char time_buf[64];

    if (data == NULL) return;

    hide_fault_popup();
    g_fault_popup_data = *data;
    get_time_str(time_buf, sizeof(time_buf));

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
    g_fault_machine_img = lv_img_create(g_fault_popup);
    lv_img_set_src(g_fault_machine_img, data->machine_img_path);
    lv_obj_set_pos(g_fault_machine_img, 24, 52);

    g_fault_err_img = lv_img_create(g_fault_popup);
    lv_img_set_src(g_fault_err_img, data->err_img_path);
    lv_obj_set_pos(g_fault_err_img, 292, 52);

    /* 雷达 */
    g_fault_radar_1 = lv_obj_create(g_fault_popup);
    lv_obj_clear_flag(g_fault_radar_1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_fault_radar_1, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(g_fault_radar_1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_fault_radar_1, 2, 0);
    lv_obj_set_style_border_color(g_fault_radar_1, lv_color_hex(0xFF3B30), 0);
    lv_obj_set_style_shadow_width(g_fault_radar_1, 0, 0);
    lv_obj_set_style_outline_width(g_fault_radar_1, 0, 0);

    g_fault_radar_2 = lv_obj_create(g_fault_popup);
    lv_obj_clear_flag(g_fault_radar_2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(g_fault_radar_2, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(g_fault_radar_2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_fault_radar_2, 2, 0);
    lv_obj_set_style_border_color(g_fault_radar_2, lv_color_hex(0xFF3B30), 0);
    lv_obj_set_style_shadow_width(g_fault_radar_2, 0, 0);
    lv_obj_set_style_outline_width(g_fault_radar_2, 0, 0);

    g_fault_anim_tick = 0;
    g_fault_anim_timer = lv_timer_create(fault_anim_timer_cb, 30, NULL);

    /* 红框1 */
    g_fault_title_1 = lv_label_create(g_fault_popup);
    lv_label_set_text(g_fault_title_1, data->diagnostics_title);
    lv_obj_set_style_text_color(g_fault_title_1, lv_color_hex(0x7A7A7A), 0);
    lv_obj_set_style_text_font(g_fault_title_1, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(g_fault_title_1, 25, 12);

    /* 红框2 */
    g_fault_title_2 = lv_label_create(g_fault_popup);
    lv_label_set_text(g_fault_title_2, data->fault_type_title);
    lv_obj_set_style_text_color(g_fault_title_2, lv_color_hex(0xFF3B30), 0);
    lv_obj_set_style_text_font(g_fault_title_2, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(g_fault_title_2, 650, 20);

    /* 时间/机型 */
    g_fault_time_label = lv_label_create(g_fault_popup);
    lv_label_set_text_fmt(g_fault_time_label, "TIME: %s", time_buf);
    lv_obj_set_style_text_color(g_fault_time_label, lv_color_hex(0xB0B0B0), 0);
    lv_obj_set_style_text_font(g_fault_time_label, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(g_fault_time_label, 650, 46);

    g_fault_model_label = lv_label_create(g_fault_popup);
    lv_label_set_text_fmt(g_fault_model_label, "MODEL: %s", MACHINE_MODEL_NAME);
    lv_obj_set_style_text_color(g_fault_model_label, lv_color_hex(0xB0B0B0), 0);
    lv_obj_set_style_text_font(g_fault_model_label, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(g_fault_model_label, 880, 46);

    /* 红框3 */
    g_fault_main_desc_label = lv_label_create(g_fault_popup);
    lv_label_set_text(g_fault_main_desc_label, data->fault_main_desc);
    lv_obj_set_style_text_color(g_fault_main_desc_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(g_fault_main_desc_label, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(g_fault_main_desc_label, 607, 66);

    /* 原因框 */
    g_reason_title = lv_label_create(g_fault_popup);
    lv_label_set_text(g_reason_title, "Reason");
    lv_obj_set_width(g_reason_title, 100);
    lv_obj_set_style_text_color(g_reason_title, lv_color_hex(0xff3b30), 0);
    lv_obj_set_style_text_font(g_reason_title, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(g_reason_title, 629, 105);

    g_fault_reason_label = lv_label_create(g_fault_popup);
    lv_label_set_text_fmt(g_fault_reason_label, "%s", data->reason_text);
    lv_obj_set_width(g_fault_reason_label, 680);
    lv_label_set_long_mode(g_fault_reason_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(g_fault_reason_label, lv_color_hex(0x555555), 0);
    lv_obj_set_style_text_font(g_fault_reason_label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(g_fault_reason_label, 629, 128   );

    g_solution_title = lv_label_create(g_fault_popup);
    lv_label_set_text(g_solution_title, "Solution");
    lv_obj_set_width(g_solution_title, 100);
    lv_obj_set_style_text_color(g_solution_title, lv_color_hex(0x007aff), 0);
    lv_obj_set_style_text_font(g_solution_title, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(g_solution_title, 629, 219);

    g_fault_solution_label = lv_label_create(g_fault_popup);
    lv_label_set_text_fmt(g_fault_solution_label, " %s", data->solution_text);
    lv_obj_set_width(g_fault_solution_label, 680);
    lv_label_set_long_mode(g_fault_solution_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(g_fault_solution_label, lv_color_hex(0x555555), 0);
    lv_obj_set_style_text_font(g_fault_solution_label, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(g_fault_solution_label, 629, 243);

    /* 版本 */
    g_fault_version_label = lv_label_create(g_fault_popup);
    lv_label_set_text_fmt(g_fault_version_label, "MAIN-APP: %s", Machine_Statue.main_app);
    lv_obj_set_style_text_color(g_fault_version_label, lv_color_hex(0x999999), 0);
    lv_obj_set_style_text_font(g_fault_version_label, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(g_fault_version_label, 595, 342);

    /* confirm */
    lv_obj_t* confirm_btn = lv_btn_create(g_fault_popup);
    lv_obj_set_size(confirm_btn, 180, 45);
    lv_obj_set_pos(confirm_btn, 1023, 311);
    lv_obj_set_style_radius(confirm_btn, 14, 0);
    lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0x1677FF), 0);
    lv_obj_set_style_border_width(confirm_btn, 0, 0);
    lv_obj_add_event_cb(confirm_btn, fault_popup_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "CONFIRM");
    lv_obj_set_style_text_color(confirm_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(confirm_label, &lv_font_montserrat_20, 0);
    lv_obj_center(confirm_label);
}

/* =========================
 * 协议适配函数
 * ========================= */
void show_boot_fault_popup(uint8_t selftest_type, uint8_t result)
{
    fault_popup_data_t data;
    memset(&data, 0, sizeof(data));

    (void)result;

    data.source = FAULT_SRC_BOOT;
    data.code = selftest_type;
    data.diagnostics_title = "MACHINE DIAGNOSTICS";
    data.fault_type_title = get_boot_title(selftest_type);
    data.fault_main_desc = get_boot_main_desc(selftest_type);
    data.reason_text = get_fault_reason_text(FAULT_SRC_BOOT, selftest_type);
    data.solution_text = get_fault_solution_text(FAULT_SRC_BOOT, selftest_type);
    data.machine_img_path = get_fault_machine_img(selftest_type);
    data.err_img_path = get_fault_err_img(selftest_type);
    get_fault_radar_pos(FAULT_SRC_BOOT, selftest_type, &data.radar_x, &data.radar_y);
    data.confirm_action = get_confirm_action_by_page();

    show_fault_popup_ex(&data);
}

void show_start_fault_popup(uint8_t type, uint8_t code)
{
    fault_popup_data_t data;
    memset(&data, 0, sizeof(data));

    (void)type;

    data.source = FAULT_SRC_START_COUNT;
    data.code = code;
    data.diagnostics_title = "MACHINE DIAGNOSTICS";
    data.fault_type_title = get_start_title(code);
    data.fault_main_desc = get_start_main_desc(code);
    data.reason_text = get_fault_reason_text(FAULT_SRC_START_COUNT, code);
    data.solution_text = get_fault_solution_text(FAULT_SRC_START_COUNT, code);
    data.machine_img_path = get_fault_machine_img(code);
    data.err_img_path = get_fault_err_img(code);
    get_fault_radar_pos(FAULT_SRC_START_COUNT, code, &data.radar_x, &data.radar_y);
    data.confirm_action = get_confirm_action_by_page();

    show_fault_popup_ex(&data);
}

void show_runtime_fault_popup(uint8_t code)
{
    fault_popup_data_t data;
    memset(&data, 0, sizeof(data));

    data.source = FAULT_SRC_RUNTIME;
    data.code = code;
    data.diagnostics_title = "MACHINE DIAGNOSTICS";
    data.fault_type_title = get_runtime_title(code);
    data.fault_main_desc = get_runtime_main_desc(code);
    data.reason_text = get_fault_reason_text(FAULT_SRC_RUNTIME, code);
    data.solution_text = get_fault_solution_text(FAULT_SRC_RUNTIME, code);
    data.machine_img_path = get_fault_machine_img(code);
    data.err_img_path = get_fault_err_img(code);
    get_fault_radar_pos(FAULT_SRC_RUNTIME, code, &data.radar_x, &data.radar_y);
    data.confirm_action = get_confirm_action_by_page();

    show_fault_popup_ex(&data);
}
