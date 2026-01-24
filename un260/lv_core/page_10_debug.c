/* ================= page_10_debug.h ================= */
#ifndef PAGE_10_DEBUG_H
#define PAGE_10_DEBUG_H

#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/platform_app.h"
#include <string.h>
#include "lvgl/lvgl.h"
#include "../aic_ui/aic_ui.h"
#include "../../third-party/lvgl-8.3.2/src/widgets/lv_textarea.h"



#endif

/* ================= page_10_debug.c ================= */
#include "page_10_debug.h"
#include <stdio.h>
#include <string.h>
#define MAX_LOG_LABELS 200
static lv_obj_t* log_labels[MAX_LOG_LABELS];
static int log_label_count = 0;

// UI对象
static lv_obj_t* ta_input;           // 输入框
static lv_obj_t* kb_hex;             // HEX键盘
static lv_obj_t* log_area;           // 日志滚动区
static lv_obj_t* log_label;          // 日志文本
static lv_obj_t* label_tx_count;     // 发送计数
static lv_obj_t* label_rx_count;     // 接收计数

// 统计数据
static uint32_t tx_count = 0;
static uint32_t rx_count = 0;

// 日志缓冲区（避免频繁分配内存）
#define LOG_BUF_SIZE 4096
static char log_buffer[LOG_BUF_SIZE];

/* ========= HEX 键盘布局 ========= */
static const char* kb_hex_map[] = {
    "1", "2", "3", "4", "5", "\n",
    "6", "7", "8", "9", "0", "\n",
    "A", "B", "C", "D", "E", "F", "\n",
    "SPACE", "CLR", ""
};

static const lv_btnmatrix_ctrl_t kb_hex_ctrl_map[] = {
    1, 1, 1, 1, 1,                    // 1-5
    1, 1, 1, 1, 1,                    // 6-0
    1, 1, 1, 1, 1, 1,                 // A-F
    2,  1                           // SPACE, <-, CLR
};

/* ---------- 自动添加空格逻辑 ---------- */
static void ta_auto_space_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;

    lv_obj_t* ta = lv_event_get_target(e);
    const char* txt = lv_textarea_get_text(ta);
    int len = strlen(txt);

    // 如果长度小于前缀，直接恢复前缀
    if (len < 6) { // "FD DF " 长度为6
        lv_textarea_set_text(ta, "FD DF ");
        lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
        return;
    }

    // 每输入2个HEX字符自动加空格（从第6位开始）
    if (len > 6 && txt[len - 1] != ' ' && ((len - 6) % 3) == 2) {
        lv_textarea_add_char(ta, ' ');
    }
}

/* ---------- HEX键盘按键事件 ---------- */
static void kb_hex_event_cb(lv_event_t* e) {
    lv_obj_t* kb = lv_event_get_target(e);
    uint16_t btn_id = lv_btnmatrix_get_selected_btn(kb);
    const char* txt = lv_btnmatrix_get_btn_text(kb, btn_id);

    if (strcmp(txt, "SPACE") == 0) {
        lv_textarea_add_char(ta_input, ' ');
    }
    else if (strcmp(txt, "CLR") == 0) {
        lv_textarea_set_text(ta_input, "");
    }
    else if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) {
        lv_textarea_del_char_forward(ta_input);
    }
    else {
        lv_textarea_add_text(ta_input, txt);
    }
}

/* ---------- 追加日志（带颜色） ---------- */
static void append_log(const char* prefix, const char* data, const char* color_hex)
{
    if (!log_area) return;

    if (log_label_count >= MAX_LOG_LABELS) {
        // 删除最老的一条
        lv_obj_del(log_labels[0]);
        for (int i = 1; i < log_label_count; i++) log_labels[i - 1] = log_labels[i];
        log_label_count--;
    }

    lv_obj_t* lbl = lv_label_create(log_area);
    log_labels[log_label_count++] = lbl;

    static uint32_t timestamp = 0;
    timestamp++;
    char buf[256];
    snprintf(buf, sizeof(buf), "%s %04d: %s", prefix, timestamp, data);
    lv_label_set_text(lbl, buf);

    // 解析颜色
    unsigned int r = 0, g = 0, b = 0;
    if (strlen(color_hex) == 6) sscanf(color_hex, "%02X%02X%02X", &r, &g, &b);
    lv_color_t c = lv_color_make(r, g, b);
    lv_obj_set_style_text_color(lbl, c, 0);

    // 排列纵向
    lv_coord_t y = 5;
    for (int i = 0; i < log_label_count - 1; i++) {
        y += lv_obj_get_height(log_labels[i]) + 2;
    }
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 5, y);

    lv_obj_scroll_to_y(log_area, y, LV_ANIM_ON);
}
static int hex_str_to_bytes(const char *str, uint8_t *out, int max_len)
{
    int count = 0;
    while (*str && *(str + 1) && count < max_len) {
        if (*str == ' ') {
            str++;
            continue;
        }
        unsigned int val;
        if (sscanf(str, "%2x", &val) != 1)
            break;
        out[count++] = (uint8_t)val;
        str += 2;
    }
    return count;
}

static void bytes_to_hex_str(const uint8_t *buf, int len, char *out, int out_size)
{
    int pos = 0;
    for (int i = 0; i < len && pos < out_size - 3; i++) {
        pos += snprintf(out + pos, out_size - pos, "%02X ", buf[i]);
    }
}

static void btn_send_event_cb(lv_event_t* e)
{
    const char* cmd_str = lv_textarea_get_text(ta_input);
    if (!cmd_str || strlen(cmd_str) < 8) return;

    uint8_t frame[64];
    int len = hex_str_to_bytes(cmd_str, frame, sizeof(frame));
    if (len < 6) return;

    // 协议校验
    if (frame[0] != 0xFD || frame[1] != 0xDF) {
        append_log("ERR", "Invalid header", "FF0000");
        return;
    }

    uint8_t cmd_g = frame[3];                 // CMD-G
    uint8_t *cmd_s = &frame[4];               // CMD-Sx
    uint16_t cmd_s_len = len - 5;             // 去掉 FD DF LEN CMD-G CRC

    /* ===== 调用真实发送 ===== */
    send_command(fd4, cmd_g, cmd_s, cmd_s_len);

    /* ===== UI 显示 ===== */
    append_log("TX", cmd_str, "00FF00");
    tx_count++;
    lv_label_set_text_fmt(label_tx_count, "TX: %d", tx_count);

    lv_textarea_set_text(ta_input, "FD DF ");
    lv_textarea_set_cursor_pos(ta_input, LV_TEXTAREA_CURSOR_LAST);
}



/* ---------- 清空日志按钮 ---------- */
static void btn_clear_log_event_cb(lv_event_t* e) {
    for (int i = 0; i < log_label_count; i++) {
        lv_obj_del(log_labels[i]);
    }
    log_label_count = 0;
    tx_count = 0;
    rx_count = 0;
    lv_label_set_text(label_tx_count, "TX: 0");
    lv_label_set_text(label_rx_count, "RX: 0");
}


/* ---------- 清空输入框按钮 ---------- */
static void btn_clear_input_event_cb(lv_event_t* e) {
    lv_textarea_set_text(ta_input, "");
}

/* ---------- 快捷命令按钮 ---------- */
static void btn_quick_cmd_event_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    const char* cmd = (const char*)lv_event_get_user_data(e);
    lv_textarea_set_text(ta_input, cmd);
}


/* ========== 主创建函数 ========== */
void ui_page_10_debug_create(void) {
    // page_debug 已存在，直接使用
    page_debug = lv_obj_create(lv_scr_act());

    lv_obj_set_size(page_debug, 1280, 400);
    lv_obj_set_style_bg_color(page_debug, lv_color_hex(0x1a1a1a), 0);
    lv_obj_clear_flag(page_debug, LV_OBJ_FLAG_SCROLLABLE);

    // 初始化日志缓冲区
    log_buffer[0] = '\0';
    tx_count = 0;
    rx_count = 0;

    /* ================= 左侧输入区（宽度550） ================= */
    lv_obj_t* left_panel = lv_obj_create(page_debug);
    lv_obj_set_size(left_panel, 550, 375);
    lv_obj_set_pos(left_panel, 5, 5);
    lv_obj_set_style_bg_color(left_panel, lv_color_hex(0x2a2a2a), 0);
    lv_obj_set_style_border_color(left_panel, lv_color_hex(0x3a3a3a), 0);
    lv_obj_set_style_border_width(left_panel, 1, 0);
    lv_obj_set_style_radius(left_panel, 8, 0);
    lv_obj_clear_flag(left_panel, LV_OBJ_FLAG_SCROLLABLE);

    // 标题
    lv_obj_t* label_title = lv_label_create(left_panel);
    lv_label_set_text(label_title, "HEX Command");
    lv_obj_set_style_text_color(label_title, lv_color_white(), 0);
    lv_obj_set_pos(label_title, 15, 10);

    // 输入框
    ta_input = lv_textarea_create(left_panel);
    lv_obj_set_size(ta_input, 380, 50);
    lv_obj_set_pos(ta_input, 15, 40);
    lv_textarea_set_placeholder_text(ta_input, "FD DF XX XX 0A");
    lv_textarea_set_text(ta_input, "FD DF ");        // 默认固定前缀
    lv_textarea_set_cursor_pos(ta_input, LV_TEXTAREA_CURSOR_LAST); // 光标移动到末尾
    lv_textarea_set_one_line(ta_input, true);
    lv_obj_set_style_bg_color(ta_input, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_text_color(ta_input, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_color(ta_input, lv_color_hex(0x4A9EFF), 0);
    lv_obj_add_event_cb(ta_input, ta_auto_space_event_cb, LV_EVENT_VALUE_CHANGED, NULL);


    // 发送按钮
    lv_obj_t* btn_send = lv_btn_create(left_panel);
    lv_obj_set_size(btn_send, 110, 90);
    lv_obj_set_pos(btn_send, 405, 40);
    lv_obj_set_style_bg_color(btn_send, lv_color_hex(0x00AA00), 0);
    lv_obj_add_event_cb(btn_send, btn_send_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* btn_send_label = lv_label_create(btn_send);
    lv_label_set_text(btn_send_label, "Send");
    lv_obj_set_style_text_font(btn_send_label, &lv_font_montserrat_20, 0);
    lv_obj_center(btn_send_label);
    // 发送按钮
    lv_obj_t* btn_sec = lv_btn_create(left_panel);
    lv_obj_set_size(btn_sec, 110, 30);
    lv_obj_set_pos(btn_sec, 160, 0);
    lv_obj_set_style_bg_color(btn_sec, lv_color_hex(0x00AA00), 0);
    lv_obj_add_event_cb(btn_sec, page_01_back_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* btn_esc_label = lv_label_create(btn_sec);
    lv_label_set_text(btn_esc_label, "ESC");
    lv_obj_set_style_text_font(btn_esc_label, &lv_font_montserrat_20, 0);
    lv_obj_center(btn_esc_label);
    
    // 清空输入按钮
    lv_obj_t* btn_clear_input = lv_btn_create(left_panel);
    lv_obj_set_size(btn_clear_input, 100, 35);
    lv_obj_set_pos(btn_clear_input, 15, 100);
    lv_obj_set_style_bg_color(btn_clear_input, lv_color_hex(0x555555), 0);
    lv_obj_add_event_cb(btn_clear_input, btn_clear_input_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* lbl_clr = lv_label_create(btn_clear_input);
    lv_label_set_text(lbl_clr, "Clear");
    lv_obj_center(lbl_clr);

    // 快捷命令按钮（示例）
    lv_obj_t* btn_quick1 = lv_btn_create(left_panel);
    lv_obj_set_size(btn_quick1, 100, 35);
    lv_obj_set_pos(btn_quick1, 125, 100);
    lv_obj_set_style_bg_color(btn_quick1, lv_color_hex(0x4A9EFF), 0);
    lv_obj_add_event_cb(btn_quick1, btn_quick_cmd_event_cb, LV_EVENT_CLICKED, "FD DF");

    lv_obj_t* lbl_q1 = lv_label_create(btn_quick1);
    lv_label_set_text(lbl_q1, "Query");
    lv_obj_center(lbl_q1);

    // HEX键盘
    kb_hex = lv_btnmatrix_create(left_panel);
    lv_btnmatrix_set_map(kb_hex, kb_hex_map);
    lv_btnmatrix_set_ctrl_map(kb_hex, kb_hex_ctrl_map);
    lv_obj_set_size(kb_hex, 500, 200);
    lv_obj_set_pos(kb_hex, 15, 145);
    lv_obj_set_style_bg_color(kb_hex, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(kb_hex, 0, 0);
    lv_obj_add_event_cb(kb_hex, kb_hex_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* ================= 右侧日志区（宽度680） ================= */
    lv_obj_t* right_panel = lv_obj_create(page_debug);
    lv_obj_set_size(right_panel, 680, 380);
    lv_obj_set_pos(right_panel, 570, 5);
    lv_obj_set_style_bg_color(right_panel, lv_color_hex(0x2a2a2a), 0);
    lv_obj_set_style_border_color(right_panel, lv_color_hex(0x3a3a3a), 0);
    lv_obj_set_style_border_width(right_panel, 1, 0);
    lv_obj_set_style_radius(right_panel, 8, 0);
    lv_obj_clear_flag(right_panel, LV_OBJ_FLAG_SCROLLABLE);

    // 顶部栏
    lv_obj_t* top_bar = lv_obj_create(right_panel);
    lv_obj_set_size(top_bar, 600, 40);
    lv_obj_set_pos(top_bar, 15, 10);
    lv_obj_set_style_bg_color(top_bar, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(top_bar, 0, 0);
    lv_obj_clear_flag(top_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* label_log = lv_label_create(top_bar);
    lv_label_set_text(label_log, "Comm Log");
    lv_obj_set_style_text_color(label_log, lv_color_white(), 0);
    // 居中
    lv_coord_t a_parent_h = lv_obj_get_height(top_bar);
    lv_coord_t a_label_h = lv_obj_get_height(label_log);
    lv_obj_set_pos(label_log, 5, (a_parent_h - a_label_h) / 2);

    // 统计标签
    label_tx_count = lv_label_create(top_bar);
    lv_label_set_text(label_tx_count, "TX: 0");
    lv_obj_set_style_text_color(label_tx_count, lv_color_hex(0x00FF00), 0);
    lv_coord_t b_parent_h = lv_obj_get_height(top_bar);
    lv_coord_t b_label_h = lv_obj_get_height(label_log);
    lv_obj_set_pos(label_tx_count, 300, (b_parent_h - b_label_h) / 2);
    label_rx_count = lv_label_create(top_bar);
    lv_label_set_text(label_rx_count, "RX: 0");
    lv_obj_set_style_text_color(label_rx_count, lv_color_hex(0x4A9EFF), 0);
    lv_coord_t c_parent_h = lv_obj_get_height(top_bar);
    lv_coord_t c_label_h = lv_obj_get_height(label_log);
    lv_obj_set_pos(label_rx_count, 380, (c_parent_h - c_label_h) / 2);
    // 清空日志按钮
    lv_obj_t* btn_clear_log = lv_btn_create(top_bar);
    lv_obj_set_size(btn_clear_log, 80, 30);
    lv_obj_align(btn_clear_log, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_bg_color(btn_clear_log, lv_color_hex(0xAA0000), 0);
    lv_obj_add_event_cb(btn_clear_log, btn_clear_log_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* lbl_clr_log = lv_label_create(btn_clear_log);
    lv_label_set_text(lbl_clr_log, "clear");
    lv_obj_center(lbl_clr_log);

    // 日志滚动区域
    log_area = lv_obj_create(right_panel);
    lv_obj_set_size(log_area, 600, 290);
    lv_obj_set_pos(log_area, 15, 60);
    lv_obj_set_style_bg_color(log_area, lv_color_black(), 0);
    lv_obj_set_style_border_color(log_area, lv_color_hex(0x444444), 0);
    lv_obj_set_style_radius(log_area, 4, 0);
    lv_obj_set_scroll_dir(log_area, LV_DIR_VER);

    log_label = lv_label_create(log_area);
    lv_label_set_text(log_label, "");
    lv_obj_set_width(log_label, 560);
    lv_label_set_long_mode(log_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(log_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(log_label, &lv_font_montserrat_14, 0);
}

void ui_page_10_debug_destroy(void) {
    if (page_debug) {
        lv_obj_clean(page_debug);  // 清空子对象，但不删除page_debug本身
    }
}

/* ========== 外部调用：追加接收日志 ========== */
void debug_append_rx_log(const char* data) {
    append_log("RX", data, "4A9EFF");
    rx_count++;
    lv_label_set_text_fmt(label_rx_count, "RX: %d", rx_count);
}
