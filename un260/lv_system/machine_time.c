#include "platform_app.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "un260/lv_resources/lv_img_init.h" 
#include "user_cfg.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_system/platform_app.h"
 lv_timer_t* s_time_timer = NULL;

 bool is_leap(uint16_t y)
{
    return ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
}

 uint8_t days_in_month(uint16_t y, uint8_t m)
{
     const uint8_t mdays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    if (m < 1 || m > 12) return 31;
    if (m == 2) return (uint8_t)(mdays[1] + (is_leap(y) ? 1 : 0));
    return mdays[m - 1];
}

 void clamp_date(void)
{
    if (Machine_para.year < 2000) Machine_para.year = 2000;
    if (Machine_para.month < 1) Machine_para.month = 1;
    if (Machine_para.month > 12) Machine_para.month = 12;

    uint8_t md = days_in_month(Machine_para.year, Machine_para.month);
    if (Machine_para.day < 1) Machine_para.day = 1;
    if (Machine_para.day > md) Machine_para.day = md;

    if (Machine_para.hour > 23) Machine_para.hour = 23;
    if (Machine_para.minute > 59) Machine_para.minute = 59;
    if (Machine_para.second > 59) Machine_para.second = 59;
}

 void tick_1s(void)
{
    clamp_date();

    Machine_para.second++;
    if (Machine_para.second >= 60) {
        Machine_para.second = 0;
        Machine_para.minute++;
        if (Machine_para.minute >= 60) {
            Machine_para.minute = 0;
            Machine_para.hour++;
            if (Machine_para.hour >= 24) {
                Machine_para.hour = 0;
                Machine_para.day++;
                uint8_t md = days_in_month(Machine_para.year, Machine_para.month);
                if (Machine_para.day > md) {
                    Machine_para.day = 1;
                    Machine_para.month++;
                    if (Machine_para.month > 12) {
                        Machine_para.month = 1;
                        Machine_para.year++;
                        if (Machine_para.year < 2000) Machine_para.year = 2000;
                    }
                }
            }
        }
    }
}

 void time_timer_cb(lv_timer_t* t)
{
    (void)t;
    tick_1s();

    /* 主界面有时间label就实时刷新 */
    char buf[64];
    machine_time_format(buf, sizeof(buf));
    update_label_by_name(page_01_main_obj, page_01_main_len, "time_label", "%s", buf);
}

void machine_time_init(void)
{
    if (s_time_timer) return;
    s_time_timer = lv_timer_create(time_timer_cb, 1000, NULL);
}

void machine_time_pause(bool pause)
{
    if (!s_time_timer) return;
    if (pause) lv_timer_pause(s_time_timer);
    else lv_timer_resume(s_time_timer);
}

void machine_time_set(uint16_t year, uint8_t mon, uint8_t day,
                      uint8_t hour, uint8_t min, uint8_t sec)
{
    Machine_para.year = year;
    Machine_para.month = mon;
    Machine_para.day = day;
    Machine_para.hour = hour;
    Machine_para.minute = min;
    Machine_para.second = sec;
    clamp_date();
}

void machine_time_get(uint16_t* year, uint8_t* mon, uint8_t* day,
                      uint8_t* hour, uint8_t* min, uint8_t* sec)
{
    if (year) *year = Machine_para.year;
    if (mon)  *mon  = Machine_para.month;
    if (day)  *day  = Machine_para.day;
    if (hour) *hour = Machine_para.hour;
    if (min)  *min  = Machine_para.minute;
    if (sec)  *sec  = Machine_para.second;
}

void machine_time_format(char* out, uint32_t out_len)
{
    if (!out || out_len == 0) return;
    lv_snprintf(out, out_len, "%04u/%02u/%02u/%02u/%02u/%02u",
        (unsigned)Machine_para.year,
        (unsigned)Machine_para.month,
        (unsigned)Machine_para.day,
        (unsigned)Machine_para.hour,
        (unsigned)Machine_para.minute,
        (unsigned)Machine_para.second);
}
