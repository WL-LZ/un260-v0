#ifndef PAGE_20_SET_PRINT_H
#define PAGE_20_SET_PRINT_H

#include "lvgl/lvgl.h"
#include <stdint.h>

void ui_page_20_set_print_create(lv_obj_t* parent);
void ui_page_20_set_print_destroy(void);
void ui_page_20_set_print_on_boot_setting(const uint8_t* data, uint16_t len);
void ui_page_20_set_print_on_reply(uint8_t sub_cmd, uint8_t res);

#endif
