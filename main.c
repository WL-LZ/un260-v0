#include <unistd.h>
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "aic_ui.h"
#include "aic_dec.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/lv_page_event.h"
#include "un260/app_service/app_clock.h"
#include "un260/app_service/app_boot_runtime.h"
#include "un260/app_service/app_command_runtime.h"
#include "un260/app_service/app_serial_runtime.h"
#include "un260/app_service/app_setting_runtime.h"
#include "un260/device_info/device_info.h"
#include "un260/lv_system/ui_screenshot.h"
#include "un260/lv_components/lv_upgrade_popup.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/ui_history_data.h"
#include "un260/lv_core/ui_upgrade_service.h"
#include "aic_ui/perf_stats.h"
//-------------------- UART 打印函数 --------------------

//-------------------- 全局变量 --------------------
#define UI_UPGRADE_DETECT_INTERVAL_MS 500
static uint32_t g_ui_upgrade_detect_tick = 0;

static void ui_upgrade_popup_poll(uint32_t now)
{
    ui_upgrade_detect_info_t detect_info;
    ui_page_t current_page;

    current_page = ui_manager_get_current_page();

    if (current_page == UI_PAGE_BOOT_ANIM || current_page == UI_PAGE_BOOT) return;
    if (current_page == UI_PAGE_UI_UPGRADE) return;
    if ((now - g_ui_upgrade_detect_tick) < UI_UPGRADE_DETECT_INTERVAL_MS) return;

    g_ui_upgrade_detect_tick = now;
    ui_upgrade_service_detect(&detect_info);
    lv_upgrade_popup_process_detect(detect_info.usb_present,
                                    detect_info.package_found,
                                    detect_info.package_hash_match);
}

//-------------------- 主函数 --------------------
int main(void) {
    lv_init();
    lv_img_cache_set_size(IMG_CACHE_NUM);
    aic_dec_create();

    lv_port_disp_init();
    lv_port_indev_init();
    user_cfg_password_load();
    user_cfg_screenshot_load();
    device_info_init(UI_VERSION);
    ui_history_data_init();
    ui_manager_switch(UI_PAGE_BOOT_ANIM);
    perf_stats_init();

    if (!app_serial_runtime_start()) {
        return -1;
    }
    while (1) {
        uint32_t now = app_clock_uptime_ms();
        ui_page_t current_page = ui_manager_get_current_page();
        uint64_t ui_start_us;
        uint64_t ui_end_us;
        uint32_t ui_time_us;

        ui_start_us = app_clock_monotonic_us();
        lv_timer_handler();
        ui_end_us = app_clock_monotonic_us();
        ui_time_us = app_clock_elapsed_us32(ui_start_us, ui_end_us);
        perf_stats_report_ui_time_us(ui_time_us);
        app_command_runtime_process_frames();
        page_setting_req_poll();
        ui_screenshot_indicator_poll();
        ui_count_end_anim_poll();
        ui_upgrade_popup_poll(now);

        app_command_runtime_poll(now);

        app_boot_runtime_poll(now, current_page == UI_PAGE_BOOT);

        usleep(1000);
    }
    app_setting_runtime_stop();
    app_serial_runtime_stop();

    return 0;
}
