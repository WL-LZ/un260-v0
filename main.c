#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "aic_ui.h"
#include "aic_dec.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/lv_page_declear.h"
#include "un260/lv_core/lv_page_event.h"
#include "un260/app_service/setting_service.h"
#include "un260/app_service/app_clock.h"
#include "un260/app_service/app_boot_runtime.h"
#include "un260/app_service/app_counting_runtime.h"
#include "un260/app_service/app_currency_runtime.h"
#include "un260/app_service/app_protocol_runtime.h"
#include "un260/app_service/app_serial_runtime.h"
#include "un260/app_service/app_setting_runtime.h"
#include "un260/protocol/protocol_frame_format.h"
#include "un260/protocol/protocol_frame_queue.h"
#include "un260/boot/boot_service.h"
#include "un260/data_collection/data_collection_state.h"
#include "un260/counting/counting_session_state.h"
#include "un260/counting/counting_control_reply.h"
#include "un260/counting/counting_denom_query_service.h"
#include "un260/counting/counting_denom_reply.h"
#include "un260/counting/counting_history_service.h"
#include "un260/counting/counting_reject_analysis_service.h"
#include "un260/counting/counting_reject_sn_reply.h"
#include "un260/lv_system/ui_screenshot.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/ui_text.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/ui_history_data.h"
#include "un260/lv_components/lv_print_toast.h"
#include "un260/lv_core/ui_upgrade_service.h"
#include "aic_ui/perf_stats.h"
//-------------------- UART 打印函数 --------------------

//-------------------- 全局变量 --------------------
#define MAX_CMD_PER_TICK  64   // 每轮处理上限，避免长帧流长时间占用UI

static counting_detail_state_t g_counting_detail_state;
#define UI_UPGRADE_DETECT_INTERVAL_MS 500
static uint32_t g_ui_upgrade_detect_tick = 0;

static counting_session_state_t g_counting_session;
static bool is_main_page_active(void);

static bool ui_counting_should_keep_current_page(void)
{
    ui_page_t page = ui_manager_get_current_page();

    return page == UI_PAGE_DEBUG ||
           page == UI_PAGE_IMAGE_GET ||
           page == UI_PAGE_WAVE_GET ||
           page == UI_PAGE_SENSOR;
}

static void counting_control_on_start_success(const uint8_t *buf, uint8_t len)
{
    counting_history_session_start(buf, len);

    if (data_collection_state_mode() != DATA_COLLECT_MODE_NONE) {
        data_collection_state_set_status("Counting started...");
        page_06_data_collection_refresh();
    } else if (!g_cb_running &&
               ui_manager_get_current_page() != UI_PAGE_PURE &&
               !ui_counting_should_keep_current_page()) {
        ui_manager_switch(UI_PAGE_MAIN);
    }
}

static void counting_control_on_error_frame(const char *tag,
                                            const uint8_t *buf,
                                            uint8_t len)
{
    counting_history_capture_error(tag, buf, len);
}

static void counting_control_on_start_failure(const char *description)
{
    if (data_collection_state_mode() == DATA_COLLECT_MODE_NONE) {
        return;
    }

    {
        char status[160];
        snprintf(status, sizeof(status), "Start failed: %s", description);
        data_collection_state_set_status(status);
    }
    page_06_data_collection_refresh();
}

static const counting_control_reply_hooks_t g_counting_control_hooks = {
    .on_start_success = counting_control_on_start_success,
    .on_error_frame = counting_control_on_error_frame,
    .on_start_failure = counting_control_on_start_failure,
};

static void counting_denom_on_history_frame(const char *tag,
                                            const uint8_t *buf,
                                            uint8_t len)
{
    counting_history_append_frame(tag, buf, len);
}

static const counting_denom_reply_hooks_t g_counting_denom_hooks = {
    .on_history_frame = counting_denom_on_history_frame,
};

static void counting_detail_on_reject_analysis_ready(void)
{
    counting_reject_analysis_result_t result;

    if (!counting_reject_analysis_update(&g_counting_session, &sim, &result)) {
        return;
    }
    smart_island_set_count_analysis(g_counting_session.analysis_valid_pcs,
                                    result.suspect_pcs,
                                    result.damaged_pcs);
    uart_printf(fd6,
                "count analysis valid=%d expected=%d current=%u delta=%u source=%s suspect=%d damaged=%d\n",
                g_counting_session.analysis_valid_pcs,
                result.expected_issue,
                result.current_total,
                result.delta_total,
                result.source == COUNTING_REJECT_ANALYSIS_SOURCE_DELTA ? "delta" : "current",
                result.suspect_pcs,
                result.damaged_pcs);
}

static void counting_detail_on_history_record_ready(void)
{
    counting_history_try_commit(&g_counting_session, &sim);
}

static void counting_detail_on_complete(void)
{
    app_counting_runtime_handle_detail_complete(&g_counting_session);
}

static const counting_reject_sn_reply_hooks_t g_counting_detail_hooks = {
    .on_history_frame = counting_denom_on_history_frame,
    .on_reject_analysis_ready = counting_detail_on_reject_analysis_ready,
    .on_history_record_ready = counting_detail_on_history_record_ready,
    .on_detail_complete = counting_detail_on_complete,
    .is_main_page_active = is_main_page_active,
};

static void ui_upgrade_popup_poll(uint32_t now)
{
    ui_upgrade_detect_info_t detect_info;
    page_id_t current_page;

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

static bool is_main_page_active(void)
{
    return ui_manager_get_current_page() == UI_PAGE_MAIN &&
           main_page && lv_obj_is_valid(main_page);
}

void PCCmdHandle(void)
{
    protocol_frame_t frame;
    int processed = 0;
    while (processed < MAX_CMD_PER_TICK && protocol_frame_queue_pop(&frame)) {
        processed++; // 每轮最多处理 MAX_CMD_PER_TICK 帧，避免丢帧
        uint8_t *buf = frame.data;
        uint8_t len  = frame.len;
        uint8_t cmd  = buf[3];

    /* ========= 新增：打印到 Debug 日志 ========= */
    char hex_log[256];
    protocol_frame_format_hex(buf, len, hex_log, sizeof(hex_log));
    debug_append_rx_log(hex_log);
    /* ========================================== */

        if (app_setting_runtime_handle_reply(cmd, buf, len) ||
            app_protocol_runtime_handle_reply(cmd, buf, len)) {
            continue;
        }

        //uart_printf(fd6, "Processing command 0x%02X, len=%d\n", cmd, len);

        switch (cmd) {

        case 0x01:
            app_boot_runtime_handle_reply(&g_counting_session, cmd, buf, len);
            break;

        /* ================== 0x0E 点钞信息 ================== */
        case 0x0E:
            app_counting_runtime_handle_info(&g_counting_session, &sim, buf, len);
            break;

        /* ================== 0x03 设置货币 ================== */
        case 0x03:
            app_currency_runtime_handle_reply(&g_counting_detail_state,
                                              &g_counting_session,
                                              buf,
                                              len);
            break;
        /* ================== 0x0A 启动回复 / 0x0F 运行故障 ================== */
        case 0x0F:
        case 0x0A:
            counting_control_reply_dispatch(cmd,
                                            &g_counting_session,
                                            buf,
                                            len,
                                            &g_counting_control_hooks);
            break;

        /* ================== 0x0B 面额明细 ================== */
        case 0x0B:
            counting_denom_reply_handle(&g_counting_detail_state,
                                        &g_counting_session,
                                        &sim,
                                        buf,
                                        len,
                                        &g_counting_denom_hooks);
            break;

        /* ================== 0x0C 退钞明细 ================== */
        case 0x0D:
        case 0x0C:
            counting_reject_sn_reply_dispatch(cmd,
                                              &g_counting_detail_state,
                                              &g_counting_session,
                                              &sim,
                                              buf,
                                              len,
                                              &g_counting_detail_hooks);
            break;
        case 0x37:
            app_boot_runtime_handle_reply(&g_counting_session, cmd, buf, len);
            break;
        default:
            uart_printf(fd6, "Unknown command 0x%02X\n", cmd);
            break;
        }
    }
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
        PCCmdHandle();
        page_setting_req_poll();
        ui_screenshot_indicator_poll();
        ui_count_end_anim_poll();
        ui_upgrade_popup_poll(now);

        counting_denom_query_poll(
            &g_counting_detail_state,
            now,
            boot_service_get_stage() == BOOT_STAGE_DONE ||
            boot_service_get_stage() == BOOT_STAGE_FAIL,
            is_main_page_active());

        app_boot_runtime_poll(now, current_page == UI_PAGE_BOOT);

        usleep(1000);
    }
    app_setting_runtime_stop();
    app_serial_runtime_stop();

    return 0;
}
