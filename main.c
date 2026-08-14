#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
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
#include "un260/app_service/app_serial_runtime.h"
#include "un260/machine_state/machine_state.h"
#include "un260/protocol/basic_setting_reply_dispatch.h"
#include "un260/protocol/protocol_frame_format.h"
#include "un260/protocol/protocol_frame_queue.h"
#include "un260/protocol/setting_reply_dispatch.h"
#include "un260/protocol/startup_sync_reply.h"
#include "un260/protocol/auxiliary_reply.h"
#include "un260/protocol/stream_data_reply.h"
#include "un260/boot/boot_service.h"
#include "un260/device_info/device_reply.h"
#include "un260/data_collection/data_collection_reply.h"
#include "un260/data_collection/data_collection_state.h"
#include "un260/diagnostic/diagnostic_reply.h"
#include "un260/currency/currency_reply.h"
#include "un260/counting/counting_session_state.h"
#include "un260/counting/counting_control_reply.h"
#include "un260/counting/counting_denom_query_service.h"
#include "un260/counting/counting_denom_reply.h"
#include "un260/counting/counting_history_service.h"
#include "un260/counting/counting_info_reply.h"
#include "un260/counting/counting_reject_analysis_service.h"
#include "un260/counting/counting_reject_sn_reply.h"
#include "un260/lv_core/page_01_main.h"
#include "un260/lv_system/ui_screenshot.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/ui_text.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/ui_history_data.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_components/lv_print_toast.h"
#include "un260/lv_core/ui_upgrade_service.h"
#include "aic_ui/perf_stats.h"
//-------------------- UART 打印函数 --------------------

//-------------------- 全局变量 --------------------
#define MAX_CMD_PER_TICK  64   // 每轮处理上限，避免长帧流长时间占用UI

static counting_detail_state_t g_counting_detail_state;
static lv_timer_t* g_mode_clear_timer = NULL;
#define UI_UPGRADE_DETECT_INTERVAL_MS 500
static uint32_t g_ui_upgrade_detect_tick = 0;

static counting_session_state_t g_counting_session;
static bool is_main_page_active(void);
static void trigger_auto_wave_after_detail(void);

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
    trigger_auto_wave_after_detail();
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

//-------------------- 工具函数 --------------------
const char* get_currency_error_desc(uint8_t code)
{
    if (code < sizeof(g_currency_error_desc) / sizeof(g_currency_error_desc[0]) &&
        g_currency_error_desc[code] != NULL) {
        return g_currency_error_desc[code];
    }
    return "Unknown Error";
}

static void format_amount_with_comma_fast(char* dest, size_t dest_size, float amount)
{
    char temp[32];
    int len;
    int dest_index = 0;
    int i;

    if (dest == NULL || dest_size == 0U) {
        return;
    }

    lv_snprintf(temp, sizeof(temp), "%.0f", amount);
    len = (int)strlen(temp);

    if (len <= 3) {
        lv_snprintf(dest, dest_size, "%s", temp);
        return;
    }

    for (i = 0; i < len && dest_index < (int)dest_size - 1; i++) {
        dest[dest_index++] = temp[i];
        if (i < len - 1 && ((len - i - 1) % 3) == 0 && dest_index < (int)dest_size - 1) {
            dest[dest_index++] = ',';
        }
    }
    dest[dest_index] = '\0';
}

/* 0x0E 高频帧：仅刷新主界面左侧紧凑区 */
static void ui_refresh_main_compact_fast(void)
{
    char amount_buf[32];

    update_label_by_name(page_01_main_obj, page_01_main_len, "01_pcs_label", "%d", sim.total_pcs);
    format_amount_with_comma_fast(amount_buf, sizeof(amount_buf), sim.total_amount);
    update_label_by_name(page_01_main_obj, page_01_main_len, "01_amount_label", "%s", amount_buf);
}

static bool is_main_page_active(void)
{
    return ui_manager_get_current_page() == UI_PAGE_MAIN &&
           main_page && lv_obj_is_valid(main_page);
}

static void schedule_auto_wave_after_count(void)
{
    ui_page_t page = ui_manager_get_current_page();

    g_counting_session.auto_wave_pending = false;

    if (Machine_para.work_mode != 0) return;
    if (page != UI_PAGE_WAVE_GET) return;

    g_counting_session.auto_wave_pending = true;
    uart_printf(fd6, "auto wave scheduled after count\n");
}

static void trigger_auto_wave_after_detail(void)
{
    bool sent = false;

    if (!g_counting_session.auto_wave_pending) return;
    g_counting_session.auto_wave_pending = false;

    if (Machine_para.work_mode != 0 ||
        ui_manager_get_current_page() != UI_PAGE_WAVE_GET) {
        return;
    }

    sent = ui_page_31_get_wave_request();

    uart_printf(fd6, "auto wave after count: sent=%d\n", sent ? 1 : 0);
}

static void mode_switch_clear_timer_cb(lv_timer_t* timer)
{
    LV_UNUSED(timer);
    g_mode_clear_timer = NULL;
    sim_clear_all_sn(&sim);
}

static void schedule_mode_switch_clear(void)
{
    // 无数据时不做清理，避免无意义开销
    if (sim.total_pcs == 0 && sim.err_num == 0 && sim.err_expected == 0) {
        return;
    }

    if (g_mode_clear_timer != NULL) {
        lv_timer_del(g_mode_clear_timer);
        g_mode_clear_timer = NULL;
    }

    // 让底部文本动画先跑起来，再执行重置，降低按键卡顿体感
    g_mode_clear_timer = lv_timer_create(mode_switch_clear_timer_cb, 120, NULL);
    if (g_mode_clear_timer != NULL) {
        lv_timer_set_repeat_count(g_mode_clear_timer, 1);
    }
}

static void pccmd_handle_device_reply(uint8_t cmd, const uint8_t *buf, uint8_t len)
{
    device_reply_result_t reply = device_reply_dispatch(cmd, buf, len);

    if (reply.kind == DEVICE_REPLY_VERSION_UPDATED) {
        uart_printf(fd6, "Version Info Received\n");
    } else if (reply.kind == DEVICE_REPLY_MAIN_UPGRADE_STATUS) {
        ui_page_14_main_upgrade_on_reply(0xA1, reply.status);
        uart_printf(fd6, "0xA1 res=0x%02X\n", reply.status);
    } else if (reply.kind == DEVICE_REPLY_IMAGE_UPGRADE_STATUS) {
        ui_page_15_image_upgrade_on_reply(0xB0, reply.status);
        uart_printf(fd6, "0xB0 res=0x%02X\n", reply.status);
    }
}

static void diagnostic_on_calibration_changed(void)
{
    cis_calib_ui_refresh();
}

static const diagnostic_reply_hooks_t g_diagnostic_reply_hooks = {
    .on_calibration_changed = diagnostic_on_calibration_changed,
};

static void pccmd_handle_basic_setting(uint8_t cmd, uint8_t *buf, uint8_t len)
{
    basic_setting_reply_action_t actions = basic_setting_reply_dispatch(cmd, buf, len);

    if ((actions & BASIC_SETTING_REPLY_ACTION_SCHEDULE_MODE_CLEAR) != 0) {
        schedule_mode_switch_clear();
    }
}

static void pccmd_handle_auxiliary_reply(uint8_t cmd, const uint8_t *buf, uint8_t len)
{
    auxiliary_reply_result_t reply = auxiliary_reply_dispatch(cmd, buf, len);

    switch (reply.kind) {
    case AUXILIARY_REPLY_DISPLAY_MAIN:
        uart_printf(fd6, "0x40 switch to main SUCCESS\n");
        break;
    case AUXILIARY_REPLY_DISPLAY_DETAIL:
        uart_printf(fd6, "0x40 switch to detail SUCCESS\n");
        break;
    case AUXILIARY_REPLY_DISPLAY_UNKNOWN:
        uart_printf(fd6, "0x40 unknown result=0x%02X\n", reply.value);
        break;
    case AUXILIARY_REPLY_PRINT_DETAIL:
        uart_printf(fd6, "0x3C print detail frame\n");
        break;
    case AUXILIARY_REPLY_PRINT_DONE:
        uart_printf(fd6, "0x3C print done\n");
        break;
    case AUXILIARY_REPLY_PRINT_UNKNOWN:
        uart_printf(fd6, "0x3C unknown len=%d\n", reply.frame_len);
        break;
    case AUXILIARY_REPLY_CLEAR_DATA_ACK:
        uart_printf(fd6, "0x3B clear data ack: res=0x%02X\n", reply.value);
        break;
    case AUXILIARY_REPLY_INVALID:
    default:
        uart_printf(fd6, "0x%02X invalid len=%d\n", cmd, len);
        break;
    }
}

static void pccmd_handle_stream_data_reply(uint8_t cmd, const uint8_t *buf, uint8_t len)
{
    stream_data_reply_view_t reply = stream_data_reply_parse(buf, len);

    if (reply.kind == STREAM_DATA_REPLY_IMAGE) {
        ui_page_28_get_image_on_frame(reply.payload, reply.payload_len);
    } else if (reply.kind == STREAM_DATA_REPLY_WAVE) {
        ui_page_31_get_wave_on_frame(reply.payload, reply.payload_len);
    } else {
        uart_printf(fd6, "0x%02X invalid len=%d\n", cmd, len);
    }
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

        //uart_printf(fd6, "Processing command 0x%02X, len=%d\n", cmd, len);

        switch (cmd) {

        case 0x01:
            app_boot_runtime_handle_reply(&g_counting_session, cmd, buf, len);
            break;

        case 0x17:
            pccmd_handle_device_reply(cmd, buf, len);
            break;

        /* ================== 0x0E 点钞信息 ================== */
        case 0x0E:
        {
            counting_info_reply_result_t result =
                counting_info_reply_handle(&g_counting_session, &sim, buf, len);

            if (result.kind == COUNTING_INFO_REPLY_LIVE) {
                ui_refresh_main_compact_fast();
                counting_history_append_frame("0x0E", buf, len);
                if (!(!fault_popup_get_auto_enabled() && fault_popup_has_pending_start_issue())) {
                    smart_island_notify_count_start();
                    smart_island_refresh_summary();
                }
            } else if (result.kind == COUNTING_INFO_REPLY_FINISHED) {
                uart_printf(fd6, "Count finished\n");
                counting_history_capture_end(buf, len);
                smart_island_set_count_analysis(g_counting_session.analysis_valid_pcs,
                                                result.final_issue,
                                                0);
                counting_history_try_commit(&g_counting_session, &sim);

                if (is_main_page_active()) {
                    ui_refresh_main_page();
                }
                schedule_auto_wave_after_count();
            }
            break;
        }

        /* ================== 0x03 设置货币 ================== */
        case 0x03:
        {
            currency_reply_result_t reply = currency_reply_handle(buf, len);

            if (reply.kind == CURRENCY_REPLY_SWITCH_SUCCESS) {
                set_curr(get_curr_item(reply.switch_result.target_code));
                sim_clear_all_sn(&sim);
                page_07_curr_apply_switch_result(&reply.switch_result);
                uart_printf(fd6, "Set %s curr success\n", reply.active_code);
                g_counting_session.end_anim_wait_detail = false;
                counting_denom_query_trigger(
                    &g_counting_detail_state,
                    app_clock_uptime_ms(),
                    boot_service_get_stage() == BOOT_STAGE_DONE ||
                    boot_service_get_stage() == BOOT_STAGE_FAIL);
                smart_island_refresh_summary();
            } else if (reply.kind == CURRENCY_REPLY_SWITCH_FAILURE) {
                page_07_curr_apply_switch_result(&reply.switch_result);
                uart_printf(fd6, "Set %s curr fail\n", reply.active_code);
            } else if (reply.kind == CURRENCY_REPLY_BOOT_ACTIVE) {
                uart_printf(fd6, "Boot curr: %s\n", reply.active_code);
                memset(sim.denom, 0, sizeof(sim.denom));
                sim.denom_number = 0;
                counting_denom_query_invalidate(&g_counting_detail_state);
                if (is_main_page_active()) {
                    ui_refresh_main_page();
                }
                counting_denom_query_trigger(
                    &g_counting_detail_state,
                    app_clock_uptime_ms(),
                    boot_service_get_stage() == BOOT_STAGE_DONE ||
                    boot_service_get_stage() == BOOT_STAGE_FAIL);
                smart_island_refresh_summary();
            }
            break;
        }
        case 0x04:
            pccmd_handle_basic_setting(cmd, buf, len);
            break;
        case 0x06:
            pccmd_handle_basic_setting(cmd, buf, len);
            break;
        /* ================== 0x08 设置退钞口张数 ================== */
        case 0x08:
            setting_reply_dispatch_detail(cmd, buf, len);
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
        case 0x1D:
        case 0x5B:
            diagnostic_reply_dispatch(cmd, buf, len, &g_diagnostic_reply_hooks);
            break;
        case 0x37:
            app_boot_runtime_handle_reply(&g_counting_session, cmd, buf, len);
            break;
        case 0x58:
        case 0x56:
            startup_sync_reply_dispatch(cmd, buf, len);
            break;
        case 0x39:
            pccmd_handle_basic_setting(cmd, buf, len);
            break;
        case 0x15:
            pccmd_handle_basic_setting(cmd, buf, len);
            break;
        case 0x16:
            pccmd_handle_basic_setting(cmd, buf, len);
            break;
        case 0x3A:
            pccmd_handle_basic_setting(cmd, buf, len);
            break;

        case 0x40:
        case 0x3C:
        case 0x3B:
            pccmd_handle_auxiliary_reply(cmd, buf, len);
            break;

        case 0x38:
            pccmd_handle_basic_setting(cmd, buf, len);
            break;

        case 0xA1:
        case 0xB0:
            pccmd_handle_device_reply(cmd, buf, len);
            break;
        case 0xC0:
            if (data_collection_reply_handle(buf, len) != DATA_COLLECTION_REPLY_INVALID) {
                page_06_data_collection_refresh();
            }
            break;
        case 0x31:
        case 0x32:
        case 0x41:
        case 0x42:
        case 0x44:
        case 0x45:
        case 0x46:
            setting_reply_dispatch_detail(cmd, buf, len);
            break;
        case 0x47:
        case 0x48:
            pccmd_handle_stream_data_reply(cmd, buf, len);
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
    app_serial_runtime_stop();

    return 0;
}
