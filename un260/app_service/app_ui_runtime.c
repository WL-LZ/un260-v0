#include "app_ui_runtime.h"

#include "un260/app_service/app_setting_runtime.h"
#include "un260/lv_components/lv_upgrade_popup.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/ui_upgrade_service.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/ui_screenshot.h"

#define APP_UI_UPGRADE_DETECT_INTERVAL_MS 500U

static uint32_t g_upgrade_detect_tick;

static void app_ui_runtime_poll_upgrade(uint32_t now_ms)
{
    ui_upgrade_detect_info_t detect_info;
    ui_page_t current_page = ui_manager_get_current_page();

    if (current_page == UI_PAGE_BOOT_ANIM ||
        current_page == UI_PAGE_BOOT ||
        current_page == UI_PAGE_UI_UPGRADE) {
        return;
    }
    if ((now_ms - g_upgrade_detect_tick) < APP_UI_UPGRADE_DETECT_INTERVAL_MS) {
        return;
    }

    g_upgrade_detect_tick = now_ms;
    ui_upgrade_service_detect(&detect_info);
    lv_upgrade_popup_process_detect(&detect_info);
}

void app_ui_runtime_poll(uint32_t now_ms)
{
    app_setting_runtime_poll();
    ui_screenshot_indicator_poll();
    ui_count_end_anim_poll();
    app_ui_runtime_poll_upgrade(now_ms);
}
