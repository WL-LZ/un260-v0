#ifndef LV_PRINT_TOAST_H
#define LV_PRINT_TOAST_H

#include "lvgl/lvgl.h"

void lv_print_toast_create(void);
void lv_print_toast_show(const char *text);
void lv_print_toast_hide(void);

#endif