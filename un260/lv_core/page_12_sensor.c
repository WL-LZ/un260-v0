#include "page_12_sensor.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_system/user_cfg.h"

#define SENSOR_SCALE_Y_DEFAULT 330
#define SENSOR_QUERY_PERIOD_MS 300

static lv_obj_t* sensor_page = NULL;
static lv_timer_t* sensor_poll_timer = NULL;
static lv_obj_t* value_labels[SENSOR_VOLTAGE_CH_NUM] = { 0 };
static uint32_t last_update_count = 0;

static const char* sensor_names[SENSOR_VOLTAGE_CH_NUM] = {
    "QTH", "QTL", "RJH", "RJL", "PS1L", "PS1R", "PS2", "PS5L", "PS5R", "ST", "SD"
};

static float sensor_raw_to_volt(uint8_t raw)
{
    return ((float)raw / 256.0f) * ((float)SENSOR_SCALE_Y_DEFAULT / 100.0f);
}

static void sensor_send_query(void)
{
    const uint8_t req[2] = { 0x01, 0x01 };
    send_command(fd4, 0x1D, req, 2);
}

static void sensor_refresh_view(void)
{
    if (last_update_count == g_sensor_voltage.update_count) return;
    last_update_count = g_sensor_voltage.update_count;

    for (int i = 0; i < SENSOR_VOLTAGE_CH_NUM; i++) {
        if (!value_labels[i] || !lv_obj_is_valid(value_labels[i])) continue;

        if (g_sensor_voltage.valid[i]) {
            const uint8_t raw = g_sensor_voltage.raw[i];
            const float v = sensor_raw_to_volt(raw);
            lv_label_set_text_fmt(value_labels[i], "%.3f V", v);
            lv_obj_set_style_text_color(value_labels[i], lv_color_hex(0x1C8E4D), 0);
        } else {
            lv_label_set_text(value_labels[i], "-- V");
            lv_obj_set_style_text_color(value_labels[i], lv_color_hex(0xdee2de), 0);
        }
    }
}

static void sensor_poll_timer_cb(lv_timer_t* timer)
{
    (void)timer;
    sensor_send_query();
    sensor_refresh_view();
}

static void sensor_esc_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_pop_page();
}

void ui_page_12_sensor_create(lv_obj_t* parent)
{
    (void)parent;
    if (sensor_page) return;

    sensor_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(sensor_page);
    lv_obj_set_size(sensor_page, 1280, 400);
    lv_obj_set_pos(sensor_page, 0, 0);
    lv_obj_clear_flag(sensor_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(sensor_page, lv_color_hex(0xEBF3FF), 0);
    lv_obj_set_style_bg_grad_color(sensor_page, lv_color_hex(0xF9FBFF), 0);
    lv_obj_set_style_bg_grad_dir(sensor_page, LV_GRAD_DIR_VER, 0);

    lv_obj_t* title = lv_label_create(sensor_page);
    lv_label_set_text(title, "SENSOR VOLTAGE");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x183B61), 0);
    lv_obj_set_pos(title, 24, 16);


    lv_obj_t* esc = lv_btn_create(sensor_page);
    lv_obj_set_size(esc, 100, 60);
    lv_obj_set_pos(esc, 1160, 16);
    lv_obj_set_style_radius(esc, 12, 0);
    lv_obj_set_style_bg_color(esc, lv_color_hex(0x3EC1F7), 0);
    lv_obj_add_event_cb(esc, sensor_esc_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* esc_label = lv_label_create(esc);
    lv_label_set_text(esc_label, "ESC");
    lv_obj_set_style_text_font(esc_label, &lv_font_montserrat_20, 0);
    lv_obj_center(esc_label);

    lv_obj_t* grid = lv_obj_create(sensor_page);
    lv_obj_set_pos(grid, 20, 88);
    lv_obj_set_size(grid, 1240, 296);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_layout(grid, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(grid, 6, 0);
    lv_obj_set_style_pad_gap(grid, 10, 0);

    for (int i = 0; i < SENSOR_VOLTAGE_CH_NUM; i++) {
        lv_obj_t* card = lv_obj_create(grid);
        lv_obj_set_size(card, 236, 84);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_radius(card, 12, 0);
        lv_obj_set_style_bg_color(card, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(card, 2, 0);
        lv_obj_set_style_border_color(card, lv_color_hex(0xD6E6F5), 0);
        lv_obj_set_style_pad_all(card, 10, 0);

        lv_obj_t* name = lv_label_create(card);
        lv_label_set_text(name, sensor_names[i]);
        lv_obj_set_style_text_font(name, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(name, lv_color_hex(0x355779), 0);
        lv_obj_align(name, LV_ALIGN_TOP_LEFT, 0, 0);

        value_labels[i] = lv_label_create(card);
        lv_label_set_text(value_labels[i], "-- V");
        lv_obj_set_style_text_font(value_labels[i], &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(value_labels[i], lv_color_hex(0x8B9AAF), 0);
        lv_obj_align(value_labels[i], LV_ALIGN_BOTTOM_LEFT, 0, 0);
    }

    last_update_count = 0;
    sensor_refresh_view();
    sensor_send_query();
    sensor_poll_timer = lv_timer_create(sensor_poll_timer_cb, SENSOR_QUERY_PERIOD_MS, NULL);
}

void ui_page_12_sensor_destroy(void)
{
    if (sensor_poll_timer) {
        lv_timer_del(sensor_poll_timer);
        sensor_poll_timer = NULL;
    }

    if (sensor_page) {
        lv_obj_del(sensor_page);
        sensor_page = NULL;
    }

    for (int i = 0; i < SENSOR_VOLTAGE_CH_NUM; i++) {
        value_labels[i] = NULL;
    }
}
