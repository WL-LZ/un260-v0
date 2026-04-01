#ifndef LV_UPGRADE_POPUP_H
#define LV_UPGRADE_POPUP_H

#include <stdbool.h>

void lv_upgrade_popup_process_detect(bool usb_present, bool package_found);
void lv_upgrade_popup_show_result(bool success, const char* desc_text);
bool lv_upgrade_popup_is_showing(void);

#endif
