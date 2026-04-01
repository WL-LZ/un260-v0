#ifndef LV_PRINT_TOAST_TEXT_H
#define LV_PRINT_TOAST_TEXT_H

typedef enum {
    PRINT_TOAST_TEXT_PRINTING = 0,
    PRINT_TOAST_TEXT_COUNT_FIRST,
    PRINT_TOAST_TEXT_MAX
} print_toast_text_id_t;

const char* lv_print_toast_text_get(print_toast_text_id_t text_id); //获取打印提示框当前语言文本

#endif
