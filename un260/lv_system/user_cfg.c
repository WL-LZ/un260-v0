#include "lvgl/lvgl.h"
#include"user_cfg.h"
#include <string.h>

#define DEFAULT_CURRENCY_COUNT 14

// 定义Machine_para变量
Machine_para_t Machine_para = {
    .mode = 1,  //1 MDC , 2 CNT ,3 VER , 4 SDC
    .curr_code = "CNY",
    .speed = 0,
    .add_enable = 0,
    .start_auto = 0,
    .current_currency = CURR_CNY_ITEM,
    .password = "1234",               // 默认密码1234
    .batch_mode = 0,                  //0 pcs ,1 amount
    .batch_num = 0,
    .batch_switch_enable = 1,
    .cfd_mode = 0,
    .fo_mode = 0,
    .work_mode = 0,
    .language = 0,
    .selected_currency = 0,
    .batch_amount = 0,
    .reject_pocket_max = 0,
    .buzzer_enable = 1,
    .serial_num_enable = 0,
    .currency_count = DEFAULT_CURRENCY_COUNT,
    .currencies = { "USD", "CNY", "EUR", "AED", "SAR", "OMR", "QAR", "MAD",
                    "EGP", "DZD", "INR", "PKR", "GBP", "IQD" },
    .year = 2024,
    .month = 10,
    .day = 26,
    .hour = 11,
    .minute = 28,
    .second = 30,
    .last_total_pcs = 0,
    .last_total_amount = 0,
    .history_total_notes_counted = 0,
    .print_space_top = 0,
    .print_head1 = "",
    .print_head2 = "",
    .print_content = PRINT_SETTING_CONTENT_LIST,
    .print_space_bottom = 0,
    .double_note_level = DOUBLE_NOTE_LEVEL_MIN,
};
Machine_Statue_t Machine_Statue = { 0 };
sensor_voltage_t g_sensor_voltage = { 0 };

//void user_data_init(void) {
//    memcpy(&Machine_para, &default_para, sizeof(Machine_para_t));
//}
                     
