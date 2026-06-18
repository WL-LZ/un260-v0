#ifndef PAGE_22_SET_DOUBLE_NOTE_H
#define PAGE_22_SET_DOUBLE_NOTE_H

#include "lvgl/lvgl.h"
#include <stdint.h>

void ui_page_22_set_double_note_create(lv_obj_t* parent);
void ui_page_22_set_double_note_destroy(void);
void ui_page_22_set_double_note_on_boot_setting(uint8_t level);
void ui_page_22_set_double_note_on_reply(uint8_t level, uint8_t res);

#endif
