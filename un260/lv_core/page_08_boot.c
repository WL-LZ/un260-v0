#include "un260/lv_core/page_08_boot.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "../aic_ui/aic_ui.h"
#include "page_08_boot.h"
static lv_timer_t* boot_waiting_timer = NULL;
static lv_obj_t* boot_waiting_label = NULL;
static uint8_t boot_waiting_dot_state = 0;
static lv_obj_t* boot_progress_bg = NULL;
static lv_obj_t* boot_progress_fill = NULL;
static lv_obj_t* boot_progress_percent_label = NULL;
static lv_obj_t* boot_progress_loading_label = NULL;
static uint8_t boot_progress_percent = 0;
static lv_timer_t* boot_progress_handshake_timer = NULL;

#define BOOT_SELFTEST_LIST_X 575
#define BOOT_SELFTEST_LIST_Y 76
#define BOOT_SELFTEST_LIST_COUNT 5

static lv_obj_t* boot_selftest_list = NULL;
static const char* boot_selftest_names[BOOT_SELFTEST_LIST_COUNT] = {
    "Read config parameters...",
    "Sensor self-test running...",
    "Motor self-test running...",
    "Electromagnet self-test running...",
    "Image board self-test running...",
};

ui_element_t page_08_curr_obj[] = {
    // 背景图

    { "page_08_boot_bg_img.png", LV_OBJ_TYPE_IMAGE, &page_07_currency_bg_img,
        { 0, 0, 1280, 400, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },


};

int page_08_curr_len = sizeof(page_08_curr_obj) / sizeof(page_08_curr_obj[0]);

#define BOOTLOG_BUFFER_SIZE 8192       
#define BOOTLOG_CONT_W 682
#define BOOTLOG_CONT_H 354
#define BOOTLOG_CONT_X 575
#define BOOTLOG_CONT_Y 37

static lv_obj_t* boot_cont = NULL;
static lv_obj_t* boot_label = NULL;
static lv_timer_t* boot_timer = NULL;
static char boot_buf[BOOTLOG_BUFFER_SIZE];

static void boot_selftest_list_create(lv_obj_t* parent); // 创建自检卡片列表
static void boot_selftest_list_apply_state(uint8_t index, lv_selftest_list_state_t state); // 刷新单卡状态
static void boot_selftest_list_mark_pending(void); // 全部恢复待测

#define BOOT_PROGRESS_X 39
#define BOOT_PROGRESS_Y 380
#define BOOT_PROGRESS_W 1203
#define BOOT_PROGRESS_H 3

static void boot_progress_apply(uint8_t percent)
{
    if (percent > 100) percent = 100;
    boot_progress_percent = percent;

    if (boot_progress_fill && lv_obj_is_valid(boot_progress_fill)) {
        lv_coord_t w = (lv_coord_t)((BOOT_PROGRESS_W * boot_progress_percent) / 100);
        lv_obj_set_width(boot_progress_fill, w);
    }

    if (boot_progress_percent_label && lv_obj_is_valid(boot_progress_percent_label)) {
        lv_label_set_text_fmt(boot_progress_percent_label, "%u%%", boot_progress_percent);
    }
}

static void boot_progress_handshake_timer_cb(lv_timer_t* t)
{
    LV_UNUSED(t);
    if (boot_progress_percent >= 18) return;

    uint8_t next = (uint8_t)(boot_progress_percent + 3);
    if (next > 18) next = 18;
    boot_progress_apply(next);
}

static void boot_progress_handshake_tick_start(void)
{
    if (boot_progress_handshake_timer == NULL) {
        boot_progress_handshake_timer = lv_timer_create(boot_progress_handshake_timer_cb, 1000, NULL);
    } else {
        lv_timer_resume(boot_progress_handshake_timer);
    }
}

static void boot_progress_handshake_tick_stop(void)
{
    if (boot_progress_handshake_timer) {
        lv_timer_del(boot_progress_handshake_timer);
        boot_progress_handshake_timer = NULL;
    }
}

void boot_progress_set(uint8_t percent)
{
    boot_progress_apply(percent);
    if (percent >= 20) {
        boot_progress_handshake_tick_stop();
    }
    if (percent >= 100) {
        boot_selftest_list_finish();
    }
}

void boot_progress_reset(void)
{
    boot_progress_apply(0);
}

static void boot_progress_create(lv_obj_t* parent)
{
    boot_progress_bg = lv_obj_create(parent);
    lv_obj_remove_style_all(boot_progress_bg);
    lv_obj_set_pos(boot_progress_bg, BOOT_PROGRESS_X, BOOT_PROGRESS_Y);
    lv_obj_set_size(boot_progress_bg, BOOT_PROGRESS_W, BOOT_PROGRESS_H);
    lv_obj_set_style_bg_color(boot_progress_bg, lv_color_hex(0xDDEFF9), 0);
    lv_obj_set_style_bg_opa(boot_progress_bg, LV_OPA_COVER, 0);
    lv_obj_clear_flag(boot_progress_bg, LV_OBJ_FLAG_SCROLLABLE);

    boot_progress_fill = lv_obj_create(parent);
    lv_obj_remove_style_all(boot_progress_fill);
    lv_obj_set_pos(boot_progress_fill, BOOT_PROGRESS_X, BOOT_PROGRESS_Y);
    lv_obj_set_size(boot_progress_fill, 0, BOOT_PROGRESS_H);
    lv_obj_set_style_bg_color(boot_progress_fill, lv_color_hex(0x10A5E9), 0);
    lv_obj_set_style_bg_opa(boot_progress_fill, LV_OPA_COVER, 0);
    lv_obj_clear_flag(boot_progress_fill, LV_OBJ_FLAG_SCROLLABLE);

    boot_progress_loading_label = lv_label_create(parent);
    lv_label_set_text(boot_progress_loading_label, "LOADING");
    lv_obj_set_pos(boot_progress_loading_label, 44, 361);
    lv_obj_set_style_text_color(boot_progress_loading_label, lv_color_hex(0x6D92AA), 0);
    lv_obj_set_style_text_font(boot_progress_loading_label, LV_FONT_DEFAULT, 0);

    boot_progress_percent_label = lv_label_create(parent);
    lv_label_set_text(boot_progress_percent_label, "0%");
    lv_obj_set_pos(boot_progress_percent_label, 1220, 361);
    lv_obj_set_style_text_color(boot_progress_percent_label, lv_color_hex(0x6D92AA), 0);
    lv_obj_set_style_text_font(boot_progress_percent_label, LV_FONT_DEFAULT, 0);

    boot_progress_apply(boot_progress_percent);
}

static void boot_selftest_list_create(lv_obj_t* parent) // 创建自检卡片列表
{
    if (parent == NULL || boot_selftest_list != NULL) {
        return;
    }

    lv_selftest_list_config_t cfg;

    lv_selftest_list_config_init(&cfg);
    cfg.item_w = 682;
    cfg.item_h = 36;
    cfg.item_gap = 10;
    cfg.item_pad_x = 10;
    cfg.icon_size = 15;
    cfg.spinner_size = 15;
    cfg.name_gap = 7;
    cfg.state_w = 100;

    boot_selftest_list = lv_selftest_list_create_with_config(parent, BOOT_SELFTEST_LIST_COUNT, &cfg);
    if (boot_selftest_list == NULL) {
        return;
    }

    lv_obj_set_pos(boot_selftest_list, BOOT_SELFTEST_LIST_X, BOOT_SELFTEST_LIST_Y);
    lv_obj_move_foreground(boot_selftest_list);

    for (uint8_t i = 0; i < BOOT_SELFTEST_LIST_COUNT; i++) {
        lv_selftest_list_set_item(boot_selftest_list, i, boot_selftest_names[i],
                                  LV_SELFTEST_LIST_STATE_PENDING);
    }
}

static void boot_selftest_list_apply_state(uint8_t index, lv_selftest_list_state_t state) // 刷新单卡状态
{
    if (boot_selftest_list == NULL || index >= BOOT_SELFTEST_LIST_COUNT) {
        return;
    }

    lv_selftest_list_set_item_state(boot_selftest_list, index, state);
}

static void boot_selftest_list_mark_pending(void) // 全部恢复待测
{
    for (uint8_t i = 0; i < BOOT_SELFTEST_LIST_COUNT; i++) {
        boot_selftest_list_apply_state(i, LV_SELFTEST_LIST_STATE_PENDING);
    }
}

void boot_selftest_list_reset(void) // 重置自检卡片显示
{
    if (boot_selftest_list == NULL || !lv_obj_is_valid(boot_selftest_list)) {
        return;
    }

    boot_selftest_list_mark_pending();
}

void boot_selftest_list_sync_step(uint8_t step) // 根据步骤同步自检卡片
{
    if (boot_selftest_list == NULL || !lv_obj_is_valid(boot_selftest_list)) {
        return;
    }

    if (step >= BOOT_SELFTEST_LIST_COUNT) {
        boot_selftest_list_finish();
        return;
    }

    for (uint8_t i = 0; i < BOOT_SELFTEST_LIST_COUNT; i++) {
        if (i < step) {
            boot_selftest_list_apply_state(i, LV_SELFTEST_LIST_STATE_SUCCESS);
        } else if (i == step) {
            boot_selftest_list_apply_state(i, LV_SELFTEST_LIST_STATE_LOADING);
        } else {
            boot_selftest_list_apply_state(i, LV_SELFTEST_LIST_STATE_PENDING);
        }
    }
}

void boot_selftest_list_finish(void) // 自检完成后全部置成功
{
    if (boot_selftest_list == NULL || !lv_obj_is_valid(boot_selftest_list)) {
        return;
    }

    for (uint8_t i = 0; i < BOOT_SELFTEST_LIST_COUNT; i++) {
        boot_selftest_list_apply_state(i, LV_SELFTEST_LIST_STATE_SUCCESS);
    }
}

static void boot_waiting_timer_cb(lv_timer_t* timer)
{
    (void)timer;

    if (boot_waiting_label == NULL) return;

    switch (boot_waiting_dot_state)
    {
        case 0:
            lv_label_set_text(boot_waiting_label, "Waiting for controller response.");
            break;

        case 1:
            lv_label_set_text(boot_waiting_label, "Waiting for controller response..");
            break;

        default:
            lv_label_set_text(boot_waiting_label, "Waiting for controller response...");
            break;
    }

    boot_waiting_dot_state++;
    if (boot_waiting_dot_state >= 3)
        boot_waiting_dot_state = 0;
}

void boot_waiting_anim_start(void)
{
    if (boot_cont == NULL || boot_label == NULL) return;

    if (boot_waiting_label == NULL) {
        boot_waiting_label = lv_label_create(boot_cont);

        lv_label_set_long_mode(boot_waiting_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(boot_waiting_label, BOOTLOG_CONT_W - 16);

        lv_obj_set_style_text_color(boot_waiting_label, lv_color_hex(0x0084FF), 0);
        lv_obj_set_style_text_font(boot_waiting_label, LV_FONT_DEFAULT, 0);
        lv_coord_t line_h = lv_font_get_line_height(LV_FONT_DEFAULT);
        lv_obj_align(boot_waiting_label, LV_ALIGN_TOP_LEFT, 8, 8 + line_h + 2);
    }

    boot_waiting_dot_state = 0;
    lv_label_set_text(boot_waiting_label, "Waiting for controller response...");

    if (boot_waiting_timer == NULL) {
        boot_waiting_timer = lv_timer_create(boot_waiting_timer_cb, 500, NULL);
    } else {
        lv_timer_resume(boot_waiting_timer);
    }
}

void boot_waiting_anim_stop(void)
{
    if (boot_waiting_timer) {
        lv_timer_del(boot_waiting_timer);
        boot_waiting_timer = NULL;
    }

    if (boot_waiting_label && lv_obj_is_valid(boot_waiting_label)) {
        lv_obj_del(boot_waiting_label);
    }
    boot_waiting_label = NULL;

    boot_waiting_dot_state = 0;
    boot_progress_handshake_tick_stop();
}

void bootlog_append(const char* text)
{
    if (!boot_label) return;

    strncat(boot_buf, text,
            BOOTLOG_BUFFER_SIZE - strlen(boot_buf) - 2);
    strncat(boot_buf, "\n",
            BOOTLOG_BUFFER_SIZE - strlen(boot_buf) - 1);

    lv_label_set_text(boot_label, boot_buf);
    lv_obj_scroll_to_y(boot_cont,
        lv_obj_get_height(boot_label),
        LV_ANIM_OFF);
}


void bootlog_start(lv_obj_t* parent)
{
    if (boot_timer) return; // 已在运行，则不重复创建

    memset(boot_buf, 0, sizeof(boot_buf));

    // 创建容器（白底）
    boot_cont = lv_obj_create(parent);
    lv_obj_set_size(boot_cont, BOOTLOG_CONT_W, BOOTLOG_CONT_H);
    lv_obj_set_pos(boot_cont, BOOTLOG_CONT_X, BOOTLOG_CONT_Y);
    lv_obj_set_style_bg_opa(boot_cont, LV_OPA_TRANSP, 0);

    // 🔹 删除边框
    lv_obj_set_style_border_width(boot_cont, 0, 0);

    // 🔹 去掉滚动条
    lv_obj_set_scrollbar_mode(boot_cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_pad_all(boot_cont, 8, 0);

    boot_label = lv_label_create(boot_cont);
    lv_obj_set_width(boot_label, BOOTLOG_CONT_W - 16);
    lv_label_set_long_mode(boot_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(boot_label, LV_ALIGN_TOP_LEFT, 8, 8);
    lv_label_set_text(boot_label, "");

    // boot 自检日志字体统一使用 #0084FF
    lv_obj_set_style_text_color(boot_label, lv_color_hex(0x0084FF), 0);
    lv_obj_set_style_text_font(boot_label, LV_FONT_DEFAULT, 0);
    //boot_timer = lv_timer_create(boot_timer_cb, BOOTLOG_LINE_MS, NULL);
}
void bootlog_stop(void)
{
    if (boot_timer) {
        lv_timer_del(boot_timer);
        boot_timer = NULL;
    }
    if (boot_cont) {
        lv_obj_del(boot_cont);
        boot_cont = NULL;
        boot_label = NULL;
    }
}

void ui_page_08_curr_create(lv_obj_t* parent)
{
    if (boot_page) return;
    boot_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(boot_page);
    lv_obj_set_pos(boot_page, 0, 0);
    lv_obj_set_size(boot_page, 1280, 400);
    lv_obj_clear_flag(boot_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(boot_page, LV_SCROLLBAR_MODE_OFF);
    lv_ui_obj_init(boot_page, page_08_curr_obj, page_08_curr_len);
    bootlog_start(boot_page);
    boot_progress_create(boot_page);
    boot_selftest_list_create(boot_page);
    boot_selftest_list_reset();
    boot_selftest_list_sync_step(boot_get_selftest_step());

    /* Restore legacy behavior: show waiting hint on boot page during handshake stage. */
    if (g_boot_stage == BOOT_STAGE_HANDSHAKE &&
        Machine_Statue.g_handshake_state != HANDSHAKE_OK) {
        boot_waiting_anim_start();
        boot_progress_set(0);
        boot_progress_handshake_tick_start();
    } else if (g_boot_stage >= BOOT_STAGE_SENSOR && g_boot_stage <= BOOT_STAGE_IMAGE) {
        uint8_t selftest_ok_cnt = (uint8_t)(g_boot_stage - BOOT_STAGE_SENSOR);
        boot_progress_set((uint8_t)(20 + selftest_ok_cnt * 10));
    } else if (g_boot_stage == BOOT_STAGE_DONE) {
        boot_progress_set(100);
    } else {
        boot_waiting_anim_stop();
    }

};

void ui_page_08_curr_destroy(void)
{
    if (boot_page)
    {
        lv_obj_del(boot_page);
        boot_page = NULL;
    }
    boot_progress_bg = NULL;
    boot_progress_fill = NULL;
    boot_progress_percent_label = NULL;
    boot_progress_loading_label = NULL;
    boot_selftest_list = NULL;
    boot_progress_handshake_tick_stop();
    boot_waiting_anim_stop();
}
