#include "lv_print_toast_text.h"
#include "un260/lv_system/ui_lang.h"

typedef struct {
    const char* en;
    const char* cn;
    const char* kr;
} print_toast_text_item_t;

static const print_toast_text_item_t g_print_toast_text_group[PRINT_TOAST_TEXT_MAX] = {
    [PRINT_TOAST_TEXT_PRINTING] = {
        "Printing...",
        "正在打印...",
        "인쇄 중..."
    },
    [PRINT_TOAST_TEXT_COUNT_FIRST] = {
        "Please count first",
        "请先点钞",
        "먼저 계수해 주세요"
    }
};

const char* lv_print_toast_text_get(print_toast_text_id_t text_id) //获取打印提示框当前语言文本
{
    const print_toast_text_item_t* item;

    if (text_id >= PRINT_TOAST_TEXT_MAX) {
        return "";
    }

    item = &g_print_toast_text_group[text_id];

    switch (ui_lang_get()) {
    case LANGUAGE_CN:
        return item->cn ? item->cn : item->en;

    case LANGUAGE_KR:
        return item->kr ? item->kr : item->en;

    case LANGUAGE_EN:
    default:
        return item->en ? item->en : "";
    }
}
