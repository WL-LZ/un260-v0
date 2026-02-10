#include "lvgl/lvgl.h"
#include "un260/lv_resources/lv_img_init.h" 
#include "user_cfg.h"

void machine_time_pause(bool pause);
void machine_time_set(uint16_t year, uint8_t mon, uint8_t day,
    uint8_t hour, uint8_t min, uint8_t sec);
void machine_time_get(uint16_t* year, uint8_t* mon, uint8_t* day,
    uint8_t* hour, uint8_t* min, uint8_t* sec);
void machine_time_format(char* out, uint32_t out_len);
void machine_time_init(void);