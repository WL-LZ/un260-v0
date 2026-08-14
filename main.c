#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "aic_ui.h"
#include "aic_dec.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_drivers/uart_bridge_service.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/lv_page_declear.h"
#include "un260/lv_core/lv_page_event.h"
#include "un260/app_service/setting_service.h"
#include "un260/app_service/app_clock.h"
#include "un260/machine_state/machine_state.h"
#include "un260/protocol/basic_setting_reply_dispatch.h"
#include "un260/protocol/protocol_frame_queue.h"
#include "un260/protocol/protocol_rx_service.h"
#include "un260/protocol/setting_reply_dispatch.h"
#include "un260/protocol/startup_sync_reply.h"
#include "un260/protocol/auxiliary_reply.h"
#include "un260/protocol/stream_data_reply.h"
#include "un260/boot/boot_service.h"
#include "un260/boot/boot_reply.h"
#include "un260/device_info/device_reply.h"
#include "un260/data_collection/data_collection_reply.h"
#include "un260/data_collection/data_collection_state.h"
#include "un260/diagnostic/diagnostic_reply.h"
#include "un260/currency/currency_reply.h"
#include "un260/counting/counting_session_state.h"
#include "un260/counting/counting_control_reply.h"
#include "un260/counting/counting_denom_reply.h"
#include "un260/counting/counting_info_reply.h"
#include "un260/counting/counting_reject_sn_reply.h"
#include "un260/lv_core/page_01_main.h"
#include "un260/lv_system/ui_screenshot.h"
#include "un260/lv_core/page_19_history.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/ui_text.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/ui_history_data.h"
#include "un260/lv_core/page_08_boot.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_components/lv_print_toast.h"
#include "un260/lv_core/ui_upgrade_service.h"
#include "aic_ui/perf_stats.h"
#include <stdlib.h>
//-------------------- UART 打印函数 --------------------

//-------------------- 全局变量 --------------------
 int fd4 = -1, fd5 = -1, fd6 = -1;
#define MAX_CMD_PER_TICK  64   // 每轮处理上限，避免长帧流长时间占用UI

static counting_detail_state_t g_counting_detail_state;
static lv_timer_t* g_mode_clear_timer = NULL;
#define UI_UPGRADE_DETECT_INTERVAL_MS 500
#define DENOM_QUERY_TIMEOUT_MS 1500
#define DENOM_QUERY_MAX_RETRY 2
#define DENOM_QUERY_IDLE_RETRY_MS 2500
static uint32_t g_ui_upgrade_detect_tick = 0;

static counting_session_state_t g_counting_session;
static uint16_t g_reject_pocket_snapshot[0x100] = {0};
static char g_history_last_error_frame_text[160] = {0};
static char g_history_last_start_frame_text[160] = {0};
static char g_history_last_end_frame_text[160] = {0};
static char g_history_session_log_text[4096] = {0};
static size_t g_history_session_log_len = 0;
static void frame_to_hex_str(const uint8_t *buf, int len, char *out, int out_len);
static bool history_copy_sim_snapshot(counting_sim_t *dst, const counting_sim_t *src);
static void history_free_sim_snapshot(counting_sim_t *sim_data);
static void history_try_commit_pending_record(void);
static void pending_result_recalc_issue_from_reject_detail(void);
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

static void history_capture_error_frame(const uint8_t *buf, int len)
{
    if (buf == NULL || len <= 0) {
        g_history_last_error_frame_text[0] = '\0';
        return;
    }

    frame_to_hex_str(buf, len, g_history_last_error_frame_text,
                     (int)sizeof(g_history_last_error_frame_text));
}

static void history_session_reset(void)
{
    g_history_last_start_frame_text[0] = '\0';
    g_history_last_end_frame_text[0] = '\0';
    g_history_session_log_text[0] = '\0';
    g_history_session_log_len = 0;
}

static void history_session_append_line(const char *tag, const uint8_t *buf, int len)
{
    char hex[256];
    int written;

    if (tag == NULL || buf == NULL || len <= 0) {
        return;
    }

    frame_to_hex_str(buf, len, hex, (int)sizeof(hex));
    written = snprintf(g_history_session_log_text + g_history_session_log_len,
                       sizeof(g_history_session_log_text) - g_history_session_log_len,
                       "%s %s\n", tag, hex);
    if (written > 0) {
        g_history_session_log_len += (size_t)written;
        if (g_history_session_log_len >= sizeof(g_history_session_log_text)) {
            g_history_session_log_len = sizeof(g_history_session_log_text) - 1;
            g_history_session_log_text[g_history_session_log_len] = '\0';
        }
    }
}

static void counting_control_on_start_success(const uint8_t *buf, uint8_t len)
{
    g_history_last_error_frame_text[0] = '\0';
    g_history_last_start_frame_text[0] = '\0';
    g_history_last_end_frame_text[0] = '\0';
    history_session_reset();
    frame_to_hex_str(buf, len, g_history_last_start_frame_text,
                     (int)sizeof(g_history_last_start_frame_text));
    history_session_append_line("0x0A", buf, len);

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
    history_capture_error_frame(buf, len);
    history_session_append_line(tag, buf, len);
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
    history_session_append_line(tag, buf, len);
}

static const counting_denom_reply_hooks_t g_counting_denom_hooks = {
    .on_history_frame = counting_denom_on_history_frame,
};

static void counting_detail_on_reject_analysis_ready(void)
{
    pending_result_recalc_issue_from_reject_detail();
}

static void counting_detail_on_history_record_ready(void)
{
    history_try_commit_pending_record();
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

static void history_try_commit_pending_record(void)
{
    counting_sim_t history_sim_snapshot;
    uint32_t total_after;

    if (!g_counting_session.history_record.valid || !g_counting_session.history_record.end_seen) {
        return;
    }

    total_after = g_counting_session.history_record.total_after;
    if (history_copy_sim_snapshot(&history_sim_snapshot, &sim)) {
        history_sim_snapshot.total_pcs = (int)g_counting_session.history_record.pcs;
        history_sim_snapshot.total_amount = g_counting_session.history_record.amount;
        ui_history_record_append_from_session(&history_sim_snapshot,
                                              g_counting_session.history_record.pcs,
                                              total_after,
                                              g_history_last_error_frame_text,
                                              g_history_last_start_frame_text,
                                              g_history_last_end_frame_text,
                                              g_history_session_log_text);
        history_free_sim_snapshot(&history_sim_snapshot);
    }

    g_counting_session.history_record.valid = false;
    g_counting_session.history_record.end_seen = false;
    g_counting_session.history_record.pcs = 0;
    g_counting_session.history_record.total_after = 0;
    g_counting_session.history_record.amount = 0.0f;
    g_history_last_error_frame_text[0] = '\0';
    g_history_last_start_frame_text[0] = '\0';
    g_history_last_end_frame_text[0] = '\0';
    history_session_reset();

    if (ui_manager_get_current_page() == UI_PAGE_HISTORY) {
        ui_page_19_history_refresh();
    }
}

static bool history_copy_sim_snapshot(counting_sim_t *dst, const counting_sim_t *src)
{
    int i;

    if (dst == NULL || src == NULL) {
        return false;
    }

    *dst = *src;
    dst->sn_str = NULL;
    dst->sn_capacity = 0;

    if (src->sn_str == NULL || src->sn_capacity <= 0) {
        return true;
    }

    dst->sn_str = calloc((size_t)src->sn_capacity, sizeof(char *));
    if (dst->sn_str == NULL) {
        return false;
    }

    dst->sn_capacity = src->sn_capacity;
    for (i = 0; i < src->sn_capacity; i++) {
        if (src->sn_str[i] == NULL) {
            continue;
        }
        dst->sn_str[i] = malloc(strlen(src->sn_str[i]) + 1);
        if (dst->sn_str[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(dst->sn_str[j]);
            }
            free(dst->sn_str);
            dst->sn_str = NULL;
            dst->sn_capacity = 0;
            return false;
        }
        strcpy(dst->sn_str[i], src->sn_str[i]);
    }

    return true;
}

static void history_free_sim_snapshot(counting_sim_t *sim_data)
{
    int i;

    if (sim_data == NULL || sim_data->sn_str == NULL) {
        return;
    }

    for (i = 0; i < sim_data->sn_capacity; i++) {
        free(sim_data->sn_str[i]);
    }
    free(sim_data->sn_str);
    sim_data->sn_str = NULL;
    sim_data->sn_capacity = 0;
}
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
bool check_aa_header(const char* data, int len) {
    return (len >= 2 && (unsigned char)data[0] == 0xFD && (unsigned char)data[1] == 0xDF);
}

const char* get_currency_error_desc(uint8_t code)
{
    if (code < sizeof(g_currency_error_desc) / sizeof(g_currency_error_desc[0]) &&
        g_currency_error_desc[code] != NULL) {
        return g_currency_error_desc[code];
    }
    return "Unknown Error";
}

static bool currency_error_is_damaged(uint8_t code)
{
    if (code == 0x18 || code == 0x19 || code == 0x1A || code == 0x2C) return true; /* Long/Short/GAP/Limpness */
    if (code == 0x24 || code == 0x25 || code == 0x27 || code == 0x28 || code == 0x29) return true; /* Hole/DogEar/Tape/Tears/Crumples */
    if (code == 0x26 || code == 0x2A || code == 0x2B) return true; /* DIRT/De_ink/Soiling */
    return false;
}

static void pending_result_recalc_issue_from_reject_detail(void)
{
    uint16_t current_by_code[0x100] = {0};
    uint16_t delta_by_code[0x100] = {0};
    const uint16_t *selected_by_code;
    int suspect = 0;
    int damaged = 0;
    int issue = 0;
    int current_total = 0;
    int delta_total = 0;
    int expected_issue;
    bool use_delta = false;

    if (!g_counting_session.last_result.valid) {
        return;
    }

    if (sim.err_num == 0 || sim.err_pcs == NULL || sim.err_code == NULL) {
        return;
    }

    for (int i = 0; i < sim.err_num; i++) {
        int pcs = sim.err_pcs[i];
        uint8_t code = sim.err_code[i];

        if (pcs > 0) {
            current_by_code[code] += (uint16_t)pcs;
        }
    }

    for (int code = 0; code < 0x100; code++) {
        current_total += current_by_code[code];
        if (current_by_code[code] > g_reject_pocket_snapshot[code]) {
            delta_by_code[code] = current_by_code[code] - g_reject_pocket_snapshot[code];
            delta_total += delta_by_code[code];
        }
    }

    expected_issue = g_counting_session.last_result.expected_issue;
    if (expected_issue <= 0 && delta_total > 0) {
        expected_issue = delta_total;
        g_counting_session.last_result.expected_issue = expected_issue;
    }
    if (current_total == expected_issue) {
        selected_by_code = current_by_code;
    } else if (delta_total == expected_issue) {
        selected_by_code = delta_by_code;
        use_delta = true;
    } else if (delta_total > 0 && delta_total <= expected_issue) {
        selected_by_code = delta_by_code;
        use_delta = true;
    } else {
        selected_by_code = current_by_code;
    }

    for (int code = 0; code < 0x100; code++) {
        int pcs = selected_by_code[code];

        if (pcs <= 0) continue;
        if (currency_error_is_damaged((uint8_t)code)) {
            damaged += pcs;
        } else {
            suspect += pcs;
        }
    }

    issue = suspect + damaged;
    if (issue < expected_issue) {
        suspect += expected_issue - issue;
        issue = expected_issue;
    } else if (issue > expected_issue) {
        int overflow = issue - expected_issue;
        int reduce = suspect < overflow ? suspect : overflow;
        suspect -= reduce;
        overflow -= reduce;
        if (overflow > 0) {
            damaged = damaged > overflow ? damaged - overflow : 0;
        }
        issue = expected_issue;
    }

    g_counting_session.last_result.issue_pcs = issue;
    g_counting_session.last_result.suspect_pcs = suspect;
    g_counting_session.last_result.damaged_pcs = damaged;
    g_counting_session.last_result.valid_pcs = g_counting_session.last_result.pcs - issue;
    if (g_counting_session.last_result.valid_pcs < 0) {
        g_counting_session.last_result.valid_pcs = 0;
    }
    memcpy(g_reject_pocket_snapshot, current_by_code, sizeof(g_reject_pocket_snapshot));
    smart_island_set_count_analysis(g_counting_session.analysis_valid_pcs, suspect, damaged);
    uart_printf(fd6, "count analysis valid=%d expected=%d current=%d delta=%d source=%s suspect=%d damaged=%d\n",
                g_counting_session.analysis_valid_pcs, expected_issue, current_total, delta_total,
                use_delta ? "delta" : "current", suspect, damaged);
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

// Clear cached denomination rows; next view refresh will wait for master 0x0B data.
static void clear_master_denom_cache(void)
{
    memset(sim.denom, 0, sizeof(sim.denom));
    sim.denom_number = 0;
    g_counting_detail_state.query_got_frame = false;
}

static void request_denom_list(void)
{
    uint8_t sub = 0x01;
    send_command(fd4, 0x0B, &sub, 1);
    g_counting_detail_state.query_pending = true;
    g_counting_detail_state.query_got_frame = false;
    g_counting_detail_state.query_tick = app_clock_uptime_ms();
    uart_printf(fd6, "request denom list: 0x0B 0x01\n");
}

static void trigger_denom_query(void)
{
    g_counting_detail_state.query_retry = 0;
    /* Avoid injecting extra command during boot self-test/param-read flow. */
    if (boot_service_get_stage() != BOOT_STAGE_DONE && boot_service_get_stage() != BOOT_STAGE_FAIL) {
        g_counting_detail_state.query_deferred = true;
        uart_printf(fd6, "defer denom query until boot done\n");
        return;
    }
    request_denom_list();
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

// ===== RX HEX 转字符串 =====
static void frame_to_hex_str(const uint8_t *buf, int len, char *out, int out_len)
{
    int pos = 0;
    for (int i = 0; i < len && pos + 3 < out_len; i++) {
        pos += snprintf(out + pos, out_len - pos, "%02X ", buf[i]);
    }
    if (pos > 0) out[pos - 1] = '\0'; // 去掉最后一个空格
}


static void boot_selftest_finish_cb(lv_timer_t* timer)
{
    bool pure_count_enabled;

    boot_selftest_list_finish();     // 自检结束后补全最后一项成功状态
    sim_data_init();                 // 自检结束后初始化一次 sim
    g_counting_session.active = false;
    g_counting_session.wait_start_ack = false;
    g_counting_session.end_anim_wait_detail = false;
    g_counting_session.last_result.valid = false;
    pure_count_enabled = ui_state_pure_count_is_enabled();
    ui_manager_switch(pure_count_enabled ? UI_PAGE_PURE : UI_PAGE_MAIN); // 按掉电记忆恢复页面
    lv_timer_del(timer);             // 删除定时器
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

static void pccmd_handle_boot_and_selftest(uint8_t cmd, const uint8_t *buf, uint8_t len)
{
    boot_reply_result_t reply = boot_reply_dispatch(cmd, buf, len);

    if (reply.kind == BOOT_REPLY_HANDSHAKE_ACCEPTED) {
        boot_progress_set(20);
        boot_send_next_selftest();
    } else if (reply.kind == BOOT_REPLY_SELF_TEST_RECORDED) {
        boot_selftest_list_set_result(reply.self_test_index, reply.self_test_result);
        boot_progress_set((uint8_t)(30 + reply.self_test_index * 10));

        if (reply.self_test_event == BOOT_SELF_TEST_EVENT_NONE) {
            boot_send_next_selftest();
        } else if (reply.self_test_event == BOOT_SELF_TEST_EVENT_SUCCESS) {
            boot_progress_set(100);
            send_command(fd4, 0x56, (uint8_t[]){0x01}, 1);
            lv_timer_create(boot_selftest_finish_cb, 2000, NULL);
        } else if (reply.self_test_event == BOOT_SELF_TEST_EVENT_FAILURE) {
            // 自检失败也继续读取主控货币列表，避免页面回落本地默认配置
            send_command(fd4, 0x56, (uint8_t[]){0x01}, 1);
            show_boot_fault_popup(reply.first_failure_step, reply.first_failure_result);
        }
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
    frame_to_hex_str(buf, len, hex_log, sizeof(hex_log));
    debug_append_rx_log(hex_log);
    /* ========================================== */

        //uart_printf(fd6, "Processing command 0x%02X, len=%d\n", cmd, len);

        switch (cmd) {

        case 0x01:
            pccmd_handle_boot_and_selftest(cmd, buf, len);
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
                history_session_append_line("0x0E", buf, len);
                if (!(!fault_popup_get_auto_enabled() && fault_popup_has_pending_start_issue())) {
                    smart_island_notify_count_start();
                    smart_island_refresh_summary();
                }
            } else if (result.kind == COUNTING_INFO_REPLY_FINISHED) {
                uart_printf(fd6, "Count finished\n");
                history_session_append_line("0x0E", buf, len);
                frame_to_hex_str(buf, len, g_history_last_end_frame_text,
                                 (int)sizeof(g_history_last_end_frame_text));
                smart_island_set_count_analysis(g_counting_session.analysis_valid_pcs,
                                                result.final_issue,
                                                0);
                history_try_commit_pending_record();

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
                trigger_denom_query();
                smart_island_refresh_summary();
            } else if (reply.kind == CURRENCY_REPLY_SWITCH_FAILURE) {
                page_07_curr_apply_switch_result(&reply.switch_result);
                uart_printf(fd6, "Set %s curr fail\n", reply.active_code);
            } else if (reply.kind == CURRENCY_REPLY_BOOT_ACTIVE) {
                uart_printf(fd6, "Boot curr: %s\n", reply.active_code);
                clear_master_denom_cache();
                if (is_main_page_active()) {
                    ui_refresh_main_page();
                }
                trigger_denom_query();
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
            pccmd_handle_boot_and_selftest(cmd, buf, len);
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


static void sent_machine_code(void)
{   uint8_t mdc_cmd = 0x03;
    send_command(fd4, 0x04, &mdc_cmd, 1); // 发送设置币种命令，默认USD
}
static lv_obj_t* g_msgbox_cont = NULL;

static void msgbox_close_cb(lv_event_t* e)
{
    if (g_msgbox_cont && lv_obj_is_valid(g_msgbox_cont)) {
        lv_obj_del(g_msgbox_cont);
        g_msgbox_cont = NULL;
    }
}

void lv_show_center_msgbox(const char* title_text, const char* info_text)
{
    lv_obj_t* scr = lv_scr_act();

    g_msgbox_cont = lv_obj_create(scr);
    lv_obj_set_size(g_msgbox_cont, 400, 200);
    lv_obj_set_pos(g_msgbox_cont, 440, 100); // 居中在 (1280,400)
    lv_obj_set_style_bg_color(g_msgbox_cont, lv_color_hex(0xE1E1E1), 0); // 灰色
    lv_obj_set_style_radius(g_msgbox_cont, 10, 0);
    lv_obj_set_style_opa(g_msgbox_cont, LV_OPA_COVER, 0);

    lv_obj_add_event_cb(g_msgbox_cont, msgbox_close_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* label_title = lv_label_create(g_msgbox_cont);
    lv_label_set_text(label_title, title_text);
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_30, 0);
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* label_info = lv_label_create(g_msgbox_cont);
    lv_label_set_text(label_info, info_text);
    lv_obj_set_style_radius(g_msgbox_cont, 10, 0);
    lv_obj_set_style_text_font(label_info, &lv_font_montserrat_22, 0);

    lv_obj_align(label_info, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_add_event_cb(scr, msgbox_close_cb, LV_EVENT_CLICKED, NULL);
}

static void close_uart_devices(void)
{
    uart_close(fd4);
    uart_close(fd5);
    uart_close(fd6);
    fd4 = -1;
    fd5 = -1;
    fd6 = -1;
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

    printf("=== 初始化UART4、UART5和UART6 ===\n");

    fd4 = uart_open("/dev/ttyS4");
    fd5 = uart_open("/dev/ttyS5");
    fd6 = uart_open("/dev/ttyS6");

    if (fd4 < 0 || fd5 < 0 || fd6 < 0) {
        printf("UART打开失败: fd4=%d fd5=%d fd6=%d\n", fd4, fd5, fd6);
        close_uart_devices();
        return -1;
    }

    if (uart_config(fd4, 115200, 8, 'N', 1) < 0 ||
        uart_config(fd5, 115200, 8, 'N', 1) < 0 ||
        uart_config(fd6, 115200, 8, 'N', 1) < 0) {
        printf("UART配置失败\n");
        close_uart_devices();
        return -1;
    }

    printf("UART配置完成\n");

    if (!protocol_rx_service_start(fd4, fd6)) {
        printf("UART4接收服务启动失败\n");
        close_uart_devices();
        return -1;
    }
    if (!uart_bridge_service_start(fd5, fd4, fd6)) {
        printf("UART5转发服务启动失败\n");
        protocol_rx_service_stop();
        close_uart_devices();
        return -1;
    }
   // machine_handshake_send(); 只发一次握手

    // 启动阶段避免阻塞，防止 LVGL 动画计时器错过播放窗口
    uart_printf(fd6, "UART6 ready\n");

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

        if (g_counting_detail_state.query_pending &&
            (now - g_counting_detail_state.query_tick) >= DENOM_QUERY_TIMEOUT_MS) {
            g_counting_detail_state.query_pending = false;
            if (!g_counting_detail_state.query_got_frame) {
                if (g_counting_detail_state.query_retry < DENOM_QUERY_MAX_RETRY) {
                    g_counting_detail_state.query_retry++;
                    uart_printf(fd6, "0x0B query timeout, retry %u/%u\n",
                                g_counting_detail_state.query_retry, DENOM_QUERY_MAX_RETRY);
                    request_denom_list();
                } else {
                    uart_printf(fd6, "0x0B query timeout, keep master-only mode (no local fallback)\n");
                }
            }
        }

        if (g_counting_detail_state.query_deferred &&
            !g_counting_detail_state.query_pending &&
            (boot_service_get_stage() == BOOT_STAGE_DONE || boot_service_get_stage() == BOOT_STAGE_FAIL)) {
            g_counting_detail_state.query_deferred = false;
            request_denom_list();
        }

        if (!g_counting_detail_state.query_pending &&
            !g_counting_detail_state.query_got_frame &&
            is_main_page_active() &&
            (boot_service_get_stage() == BOOT_STAGE_DONE || boot_service_get_stage() == BOOT_STAGE_FAIL)) {
            if ((now - g_counting_detail_state.query_idle_retry_tick) >= DENOM_QUERY_IDLE_RETRY_MS) {
                g_counting_detail_state.query_idle_retry_tick = now;
                g_counting_detail_state.query_retry = 0;
                uart_printf(fd6, "0x0B idle retry on main page\n");
                request_denom_list();
            }
        }

        if (boot_service_get_stage() == BOOT_STAGE_HANDSHAKE &&
            current_page == UI_PAGE_BOOT)
        {
            if (boot_service_handshake_state() == HANDSHAKE_IDLE)
            {
                machine_handshake_send();
            }
            else if (boot_service_handshake_state() == HANDSHAKE_SENT)
            {
                if (boot_service_check_total_timeout(now))
                {
                    boot_service_set_stage(BOOT_STAGE_FAIL);

                    show_boot_selftest_error_popup(
                        "Controller handshake timeout.\nPress CONFIRM to enter sensor page.");
                }
                else if ((now - boot_service_handshake_tick()) >= BOOT_SERVICE_HANDSHAKE_RETRY_MS)
                {
                    machine_handshake_send();
                }
            }
        }

        if (boot_service_get_stage() >= BOOT_STAGE_SENSOR && boot_service_get_stage() <= BOOT_STAGE_IMAGE &&
            current_page == UI_PAGE_BOOT)
        {
            if (boot_service_check_total_timeout(now))
            {
                boot_service_set_stage(BOOT_STAGE_FAIL);
                show_boot_selftest_error_popup(
                    "Self-test timeout.\nPress CONFIRM to enter sensor page.");
            }
        }

        usleep(1000);
    }
    uart_bridge_service_stop();
    protocol_rx_service_stop();
    close_uart_devices();

    return 0;
}
