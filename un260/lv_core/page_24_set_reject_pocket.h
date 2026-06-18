#ifndef PAGE_24_SET_REJECT_POCKET_H
#define PAGE_24_SET_REJECT_POCKET_H

#include "lvgl/lvgl.h"
#include <stdint.h>

void ui_page_24_set_reject_pocket_create(lv_obj_t* parent);
void ui_page_24_set_reject_pocket_destroy(void);
void ui_page_24_set_reject_pocket_on_boot_setting(uint8_t capacity);
void ui_page_24_set_reject_pocket_on_reply(uint8_t res);

#endif
