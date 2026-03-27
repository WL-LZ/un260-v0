#ifndef LV_PRINT_TOAST_H
#define LV_PRINT_TOAST_H

#include "lvgl/lvgl.h"
#include <stdbool.h>

#define LV_PRINT_TOAST_DEFAULT_X            502
#define LV_PRINT_TOAST_DEFAULT_Y            31
#define LV_PRINT_TOAST_DEFAULT_W            277
#define LV_PRINT_TOAST_DEFAULT_H            101
#define LV_PRINT_TOAST_DEFAULT_TEXT         "Printing..."
#define LV_PRINT_TOAST_DEFAULT_SHOW_LOADER  true
#define LV_PRINT_TOAST_DEFAULT_ALIGN_CENTER false
#define LV_PRINT_TOAST_DEFAULT_USE_TEXT_AREA false
#define LV_PRINT_TOAST_DEFAULT_TEXT_X       0
#define LV_PRINT_TOAST_DEFAULT_TEXT_Y       0
#define LV_PRINT_TOAST_DEFAULT_TEXT_W       0
#define LV_PRINT_TOAST_DEFAULT_TEXT_H       0
#define LV_PRINT_TOAST_DEFAULT_TEXT_FONT    (&lv_font_montserrat_24)
#define LV_PRINT_TOAST_DEFAULT_BG_COLOR     lv_color_hex(0xFFFFFF)
#define LV_PRINT_TOAST_DEFAULT_TEXT_COLOR   lv_color_hex(0x2F3542)

typedef struct {
    lv_coord_t x;
    lv_coord_t y;
    lv_coord_t w;
    lv_coord_t h;
    lv_coord_t text_x;
    lv_coord_t text_y;
    lv_coord_t text_w;
    lv_coord_t text_h;
    lv_color_t bg_color;
    lv_color_t text_color;
    const char *text;
    const lv_font_t *text_font;
    bool show_loader;
    bool align_center;
    bool use_text_area;
} lv_print_toast_config_t;

#define LV_PRINT_TOAST_CONFIG(_x, _y, _w, _h, _text, _show_loader) \
    {                                                               \
        .x = (_x),                                                  \
        .y = (_y),                                                  \
        .w = (_w),                                                  \
        .h = (_h),                                                  \
        .text_x = LV_PRINT_TOAST_DEFAULT_TEXT_X,                    \
        .text_y = LV_PRINT_TOAST_DEFAULT_TEXT_Y,                    \
        .text_w = LV_PRINT_TOAST_DEFAULT_TEXT_W,                    \
        .text_h = LV_PRINT_TOAST_DEFAULT_TEXT_H,                    \
        .bg_color = LV_PRINT_TOAST_DEFAULT_BG_COLOR,                \
        .text_color = LV_PRINT_TOAST_DEFAULT_TEXT_COLOR,            \
        .text = (_text),                                            \
        .text_font = LV_PRINT_TOAST_DEFAULT_TEXT_FONT,              \
        .show_loader = (_show_loader),                              \
        .align_center = false,                                      \
        .use_text_area = LV_PRINT_TOAST_DEFAULT_USE_TEXT_AREA,      \
    }

#define LV_PRINT_TOAST_CONFIG_EX(_x, _y, _w, _h, _text_x, _text_y, _text_w, _text_h,      \
                                 _bg_color, _text_color, _text, _font, _show_loader,      \
                                 _align_center, _use_text_area)                            \
    {                                                                                       \
        .x = (_x),                                                                          \
        .y = (_y),                                                                          \
        .w = (_w),                                                                          \
        .h = (_h),                                                                          \
        .text_x = (_text_x),                                                                \
        .text_y = (_text_y),                                                                \
        .text_w = (_text_w),                                                                \
        .text_h = (_text_h),                                                                \
        .bg_color = (_bg_color),                                                            \
        .text_color = (_text_color),                                                        \
        .text = (_text),                                                                    \
        .text_font = (_font),                                                               \
        .show_loader = (_show_loader),                                                      \
        .align_center = (_align_center),                                                    \
        .use_text_area = (_use_text_area),                                                  \
    }

#define LV_PRINT_TOAST_DEFAULT_CONFIG() \
    LV_PRINT_TOAST_CONFIG_EX(LV_PRINT_TOAST_DEFAULT_X, LV_PRINT_TOAST_DEFAULT_Y, \
                             LV_PRINT_TOAST_DEFAULT_W, LV_PRINT_TOAST_DEFAULT_H, \
                             LV_PRINT_TOAST_DEFAULT_TEXT_X, LV_PRINT_TOAST_DEFAULT_TEXT_Y, \
                             LV_PRINT_TOAST_DEFAULT_TEXT_W, LV_PRINT_TOAST_DEFAULT_TEXT_H, \
                             LV_PRINT_TOAST_DEFAULT_BG_COLOR, \
                             LV_PRINT_TOAST_DEFAULT_TEXT_COLOR, \
                             LV_PRINT_TOAST_DEFAULT_TEXT, LV_PRINT_TOAST_DEFAULT_TEXT_FONT, \
                             LV_PRINT_TOAST_DEFAULT_SHOW_LOADER, \
                             LV_PRINT_TOAST_DEFAULT_ALIGN_CENTER, \
                             LV_PRINT_TOAST_DEFAULT_USE_TEXT_AREA)

void lv_print_toast_create(void);
void lv_print_toast_create_with_config(const lv_print_toast_config_t *config);
void lv_print_toast_show(const char *text);
void lv_print_toast_show_with_config(const lv_print_toast_config_t *config);
void lv_print_toast_set_text(const char *text);
void lv_print_toast_hide(void);

#endif
