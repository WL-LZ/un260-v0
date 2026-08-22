#include <unistd.h>
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "aic_ui.h"
#include "aic_dec.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_system/app_clock.h"
#include "un260/app_service/app_boot_runtime.h"
#include "un260/app_service/app_command_runtime.h"
#include "un260/app_service/app_serial_runtime.h"
#include "un260/app_service/app_setting_runtime.h"
#include "un260/app_service/app_ui_runtime.h"
#include "un260/device_info/device_info.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/ui_history_data.h"
#include "aic_ui/perf_stats.h"

//-------------------- 主函数 --------------------
int main(void) {
    lv_init();
    lv_img_cache_set_size(IMG_CACHE_NUM);
    aic_dec_create();

    lv_port_disp_init();
    lv_port_indev_init();
    user_cfg_password_load();
    user_cfg_screenshot_load();
    user_cfg_screen_recording_load();
    user_cfg_performance_monitor_load();
    device_info_init(UI_VERSION);
    ui_history_data_init();
    ui_manager_switch(UI_PAGE_BOOT_ANIM);
    perf_stats_init();
    app_ui_runtime_init();

    if (!app_serial_runtime_start()) {
        return -1;
    }
    while (1) {
        uint64_t loop_start_us = app_clock_monotonic_us();
        uint32_t now = app_clock_uptime_ms();
        ui_page_t current_page = ui_manager_get_current_page();
        uint64_t lvgl_start_us;
        uint64_t lvgl_end_us;
        uint64_t loop_end_us;

        lvgl_start_us = app_clock_monotonic_us();
        lv_timer_handler();
        lvgl_end_us = app_clock_monotonic_us();
        perf_stats_report_lvgl_time_us(
            app_clock_elapsed_us32(lvgl_start_us, lvgl_end_us));
        app_command_runtime_process_frames();
        app_ui_runtime_poll(now);

        app_command_runtime_poll(now);

        app_boot_runtime_poll(now, current_page == UI_PAGE_BOOT);

        loop_end_us = app_clock_monotonic_us();
        perf_stats_report_loop_time_us(
            app_clock_elapsed_us32(loop_start_us, loop_end_us));
        usleep(1000);
    }
    app_setting_runtime_stop();
    app_serial_runtime_stop();

    return 0;
}
