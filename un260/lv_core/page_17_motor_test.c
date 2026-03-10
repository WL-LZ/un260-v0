#include "page_17_motor_test.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_drivers/lv_drivers.h"

static lv_obj_t* motor_test_page = NULL;

typedef struct {
    const char* title;
    uint8_t cmd_g;
    uint8_t forward_cmd[2];
    uint8_t stop_cmd[2];
    lv_obj_t* status_label;
} motor_item_t;

static motor_item_t motors[] = {
    { "LEVEL 1 MOTOR", 0x52, {0x01, 0x01}, {0x00, 0x00}, NULL },
    { "LEVEL 2 MOTOR", 0x53, {0x01, 0x01}, {0x00, 0x00}, NULL },
    { "IMPELLER MOTOR", 0x54, {0x01, 0x01}, {0x01, 0x02}, NULL },
};

static void motor_status_set(motor_item_t* item, const char* status)
{
    if (item == NULL || item->status_label == NULL) return;
    lv_label_set_text(item->status_label, status);
}

static void motor_test_stop_all(void)
{
    send_command(fd4, motors[0].cmd_g, motors[0].stop_cmd, 2);
    send_command(fd4, motors[1].cmd_g, motors[1].stop_cmd, 2);
    send_command(fd4, motors[2].cmd_g, motors[2].stop_cmd, 2);

    motor_status_set(&motors[0], "STOP");
    motor_status_set(&motors[1], "STOP");
    motor_status_set(&motors[2], "STOP");
}

static void motor_test_esc_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    motor_test_stop_all();
    ui_manager_pop_page();
}

static void motor_forward_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    motor_item_t* item = (motor_item_t*)lv_event_get_user_data(e);
    if (!item) return;

    send_command(fd4, item->cmd_g, item->forward_cmd, 2);
    motor_status_set(item, "FORWARD");
}



static void motor_stop_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    motor_item_t* item = (motor_item_t*)lv_event_get_user_data(e);
    if (!item) return;

    send_command(fd4, item->cmd_g, item->stop_cmd, 2);
    motor_status_set(item, "STOP");
}


static lv_obj_t* create_motor_card(lv_obj_t* parent, motor_item_t* item, lv_coord_t x, lv_coord_t y)
{
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_size(card, 390, 220);
    lv_obj_set_pos(card, x, y);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(card, 14, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0xD6E6F5), 0);
    lv_obj_set_style_pad_all(card, 12, 0);

    lv_obj_t* title = lv_label_create(card);
    lv_label_set_text(title, item->title);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x183B61), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    item->status_label = lv_label_create(card);
    lv_label_set_text(item->status_label, "STOP");
    lv_obj_set_style_text_font(item->status_label, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(item->status_label, lv_color_hex(0x183B61), 0);
    lv_obj_align(item->status_label, LV_ALIGN_TOP_MID, 0, 32);

    lv_obj_t* btn_forward = lv_btn_create(card);
    lv_obj_set_size(btn_forward, 150, 44);
    lv_obj_align(btn_forward, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_radius(btn_forward, 10, 0);
    lv_obj_set_style_bg_color(btn_forward, lv_color_hex(0x3EC1F7), 0);
    lv_obj_add_event_cb(btn_forward, motor_forward_cb, LV_EVENT_CLICKED, item);

    lv_obj_t* forward_label = lv_label_create(btn_forward);
    lv_label_set_text(forward_label, "FORWARD");
    lv_obj_set_style_text_font(forward_label, &lv_font_montserrat_16, 0);
    lv_obj_center(forward_label);

    lv_obj_t* btn_stop = lv_btn_create(card);
    lv_obj_set_size(btn_stop, 150, 44);
    lv_obj_align(btn_stop, LV_ALIGN_CENTER, 0, 64);
    lv_obj_set_style_radius(btn_stop, 10, 0);
    lv_obj_set_style_bg_color(btn_stop, lv_color_hex(0xE95F5F), 0);
    lv_obj_add_event_cb(btn_stop, motor_stop_cb, LV_EVENT_CLICKED, item);

    lv_obj_t* stop_label = lv_label_create(btn_stop);
    lv_label_set_text(stop_label, "STOP");
    lv_obj_set_style_text_font(stop_label, &lv_font_montserrat_16, 0);
    lv_obj_center(stop_label);

    return card;
}

void ui_page_17_motor_test_create(lv_obj_t* parent)
{
    (void)parent;
    if (motor_test_page) return;

    motor_test_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(motor_test_page);
    lv_obj_set_size(motor_test_page, 1280, 400);
    lv_obj_set_pos(motor_test_page, 0, 0);
    lv_obj_clear_flag(motor_test_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(motor_test_page, lv_color_hex(0xEBF3FF), 0);
    lv_obj_set_style_bg_grad_color(motor_test_page, lv_color_hex(0xF9FBFF), 0);
    lv_obj_set_style_bg_grad_dir(motor_test_page, LV_GRAD_DIR_VER, 0);

    lv_obj_t* title = lv_label_create(motor_test_page);
    lv_label_set_text(title, "MOTOR TEST");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x183B61), 0);
    lv_obj_set_pos(title, 24, 16);

    lv_obj_t* esc = lv_btn_create(motor_test_page);
    lv_obj_set_size(esc, 100, 60);
    lv_obj_set_pos(esc, 1160, 16);
    lv_obj_set_style_radius(esc, 12, 0);
    //lv_obj_set_style_bg_color(esc, lv_color_hex(0x3EC1F7), 0);
    lv_obj_add_event_cb(esc, motor_test_esc_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* esc_label = lv_label_create(esc);
    lv_label_set_text(esc_label, "ESC");
    lv_obj_set_style_text_font(esc_label, &lv_font_montserrat_20, 0);
    lv_obj_center(esc_label);

    create_motor_card(motor_test_page, &motors[0], 24, 96);
    create_motor_card(motor_test_page, &motors[1], 444, 96);
    create_motor_card(motor_test_page, &motors[2], 864, 96);
}

void ui_page_17_motor_test_destroy(void)
{
    motor_test_stop_all();

    if (motor_test_page) {
        lv_obj_del(motor_test_page);
        motor_test_page = NULL;
    }

    motors[0].status_label = NULL;
    motors[1].status_label = NULL;
    motors[2].status_label = NULL;
}