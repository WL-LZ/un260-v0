#include "lvgl/lvgl.h"
#include"user_cfg.h"
#include <string.h>

// 定义Machine_para变量
Machine_para_t Machine_para = {
    1,  //1 MDC , 2 CNT ,3 VER , 4 SDC
    "CNY",                // 改为 "EUR"
    0,
    0,
    0,
    CURR_CNY_ITEM,         // 改为 EUR
    "1234" ,               // 默认密码1234
    0,                       //0 pcs ,1 amount
    0,                      //batch_num
    1,                      //batch_switch_enable
    0,                      //cfd
    0,                      //F O
    0,                      //work:auto muanul
    0,
    0,                      //batch_num
    0,                      //reject_pocket_max
    0,                      //buzzer_enable
    0,                      //serial_num_enable
};
Machine_Statue_t Machine_Statue = { 0 };

//void user_data_init(void) {
//    memcpy(&Machine_para, &default_para, sizeof(Machine_para_t));
//}
                     