#ifndef PAGE_07_CURR_H
#define PAGE_07_CURR_H

#include "lvgl/lvgl.h"
#include "un260/currency/currency_service.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_page_07_curr_create(lv_obj_t* parent);
void ui_page_07_curr_destroy(void);
void page_07_curr_img_refre(void);
void page_07_curr_img_reset(void);
void page_07_curr_apply_switch_result(const currency_switch_result_t* result);

#ifdef __cplusplus
}
#endif

#endif // PAGE_07_CURR_H
