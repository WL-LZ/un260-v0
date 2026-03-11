#ifndef LV_FAULT_POPUP_H
#define LV_FAULT_POPUP_H

#include "lvgl/lvgl.h"
#include <stdint.h>

void show_fault_popup(uint8_t fault_code);
void hide_fault_popup(void);
bool fault_popup_is_showing(void);

#endif