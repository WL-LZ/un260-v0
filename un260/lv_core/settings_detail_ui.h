#ifndef SETTINGS_DETAIL_UI_H
#define SETTINGS_DETAIL_UI_H

#include "lvgl/lvgl.h"
#include <stdbool.h>
#include <stdint.h>

lv_obj_t* settings_detail_create_page(lv_obj_t* parent, const char* title,
                                      lv_event_cb_t back_cb,
                                      lv_obj_t** out_content);
lv_obj_t* settings_detail_create_page_ex(lv_obj_t* parent, const char* title,
                                         lv_event_cb_t back_cb,
                                         lv_obj_t** out_content,
                                         lv_obj_t** out_back_btn);
lv_obj_t* settings_detail_create_card(lv_obj_t* parent, lv_coord_t x, lv_coord_t y,
                                      lv_coord_t w, lv_coord_t h);
lv_obj_t* settings_detail_create_button(lv_obj_t* parent, lv_coord_t x, lv_coord_t y,
                                        lv_coord_t w, lv_coord_t h,
                                        const char* text, lv_color_t bg,
                                        lv_event_cb_t cb, void* user_data);
lv_obj_t* settings_detail_create_label(lv_obj_t* parent, const char* text,
                                       const lv_font_t* font, lv_color_t color,
                                       lv_coord_t x, lv_coord_t y);
bool settings_detail_send_command(uint8_t cmd_g, const uint8_t* cmd_s,
                                  uint16_t cmd_s_len);

#endif
