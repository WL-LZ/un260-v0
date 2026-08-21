#include "app_counting_runtime.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "lvgl/lvgl.h"

#include "un260/counting/counting_control_reply.h"
#include "un260/counting/counting_denom_reply.h"
#include "un260/counting/counting_history_service.h"
#include "un260/counting/counting_info_reply.h"
#include "un260/counting/counting_reject_analysis_service.h"
#include "un260/counting/counting_reject_sn_reply.h"
#include "un260/data_collection/data_collection.h"
#include "un260/diagnostic/diagnostic.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/page_06_settings.h"
#include "un260/lv_core/page_31_get_wave.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/machine_state/machine_state.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/ui_history_data.h"

typedef struct {
    counting_session_state_t *session;
    counting_sim_t *sim_data;
} app_counting_detail_context_t;

static void app_counting_runtime_format_amount(char *dest,
                                               size_t dest_size,
                                               float amount)
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
        if (i < len - 1 && ((len - i - 1) % 3) == 0 &&
            dest_index < (int)dest_size - 1) {
            dest[dest_index++] = ',';
        }
    }
    dest[dest_index] = '\0';
}

/* 0x0E is high frequency, so only update the compact summary fields. */
static void app_counting_runtime_refresh_compact(const counting_sim_t *sim_data)
{
    char amount_buf[32];

    update_label_by_name(page_01_main_obj,
                         page_01_main_len,
                         "01_pcs_label",
                         "%d",
                         sim_data->total_pcs);
    app_counting_runtime_format_amount(amount_buf,
                                       sizeof(amount_buf),
                                       sim_data->total_amount);
    update_label_by_name(page_01_main_obj,
                         page_01_main_len,
                         "01_amount_label",
                         "%s",
                         amount_buf);
}

static bool app_counting_runtime_main_page_active(void)
{
    return ui_manager_get_current_page() == UI_PAGE_MAIN &&
           page_01_main_is_created();
}

static bool app_counting_runtime_should_keep_current_page(void)
{
    ui_page_t page = ui_manager_get_current_page();

    return page == UI_PAGE_DEBUG ||
           page == UI_PAGE_IMAGE_GET ||
           page == UI_PAGE_WAVE_GET ||
           page == UI_PAGE_SENSOR;
}

static bool app_counting_runtime_cb_calibration_active(void)
{
    calibration_state_snapshot_t calibration;

    diagnostic_calibration_get_snapshot(&calibration);
    return calibration.session_active &&
           calibration.target == CALIB_TARGET_CB;
}

static void app_counting_runtime_on_start_success(const uint8_t *buf, uint8_t len)
{
    counting_history_session_start(buf, len);

    if (data_collection_state_mode() != DATA_COLLECT_MODE_NONE) {
        data_collection_state_set_status("Counting started...");
        page_06_data_collection_refresh();
    } else if (!app_counting_runtime_cb_calibration_active() &&
               ui_manager_get_current_page() != UI_PAGE_PURE &&
               !app_counting_runtime_should_keep_current_page()) {
        ui_manager_switch(UI_PAGE_MAIN);
    }
}

static void app_counting_runtime_on_error_frame(const char *tag,
                                                const uint8_t *buf,
                                                uint8_t len)
{
    counting_history_capture_error(tag, buf, len);
}

static void app_counting_runtime_on_start_failure(const char *description)
{
    char status[160];

    if (data_collection_state_mode() == DATA_COLLECT_MODE_NONE) {
        return;
    }

    snprintf(status,
             sizeof(status),
             "Start failed: %s",
             description != NULL ? description : "Unknown");
    data_collection_state_set_status(status);
    page_06_data_collection_refresh();
}

static const counting_control_reply_hooks_t g_counting_control_hooks = {
    .on_start_success = app_counting_runtime_on_start_success,
    .on_error_frame = app_counting_runtime_on_error_frame,
    .on_start_failure = app_counting_runtime_on_start_failure,
};

static const counting_denom_reply_hooks_t g_counting_denom_hooks = {
    .on_history_frame = counting_history_append_frame,
};

void app_counting_runtime_reset_session(counting_session_state_t *session,
                                        const char *reason)
{
    bool history_discarded;

    if (session == NULL) {
        return;
    }

    history_discarded = counting_history_discard_pending(session);
    memset(session, 0, sizeof(*session));
    ui_count_end_anim_cancel();
    if (history_discarded) {
        uart_debug_printf("pending history discarded by %s\n",
                    reason != NULL ? reason : "session reset");
    }
}

static void app_counting_runtime_on_detail_history(void *context,
                                                   const char *tag,
                                                   const uint8_t *buf,
                                                   uint8_t len)
{
    (void)context;
    counting_history_append_frame(tag, buf, len);
}

static void app_counting_runtime_on_reject_analysis(void *context)
{
    app_counting_detail_context_t *detail_context = context;
    counting_reject_analysis_result_t result;

    if (detail_context == NULL ||
        !counting_reject_analysis_update(detail_context->session,
                                         detail_context->sim_data,
                                         &result)) {
        return;
    }

    smart_island_set_count_analysis(detail_context->session->analysis_valid_pcs,
                                    result.suspect_pcs,
                                    result.damaged_pcs);
    uart_debug_printf(                "count analysis valid=%d expected=%d current=%u delta=%u source=%s suspect=%d damaged=%d\n",
                detail_context->session->analysis_valid_pcs,
                result.expected_issue,
                result.current_total,
                result.delta_total,
                result.source == COUNTING_REJECT_ANALYSIS_SOURCE_DELTA ?
                    "delta" : "current",
                result.suspect_pcs,
                result.damaged_pcs);
}

static void app_counting_runtime_report_history_commit(
    const counting_session_state_t *session,
    counting_history_commit_result_t result,
    uint8_t previous_attempts)
{
    if (session == NULL) {
        return;
    }

    if (result == COUNTING_HISTORY_COMMIT_RETRY_PENDING) {
        uart_debug_printf("history save failed, retry scheduled attempt=%u\n",
                    session->history_record.save_attempts);
    } else if (result == COUNTING_HISTORY_COMMIT_FAILED) {
        uart_debug_printf("history save failed after %u attempts\n",
                    session->history_record.save_attempts);
    } else if (result == COUNTING_HISTORY_COMMIT_SAVED && previous_attempts > 0) {
        uart_debug_printf("history save recovered after %u retries\n",
                    previous_attempts);
    }
}

static void app_counting_runtime_try_history_commit(
    counting_session_state_t *session,
    const counting_sim_t *sim_data,
    uint32_t now_ms)
{
    counting_history_commit_result_t result;
    uint8_t previous_attempts;

    if (session == NULL || sim_data == NULL) {
        return;
    }

    previous_attempts = session->history_record.save_attempts;
    result = counting_history_try_commit(session, sim_data, now_ms);
    app_counting_runtime_report_history_commit(session, result,
                                               previous_attempts);
}

static void app_counting_runtime_on_history_record(void *context)
{
    app_counting_detail_context_t *detail_context = context;

    if (detail_context != NULL) {
        app_counting_runtime_try_history_commit(detail_context->session,
                                                detail_context->sim_data,
                                                lv_tick_get());
    }
}

static void app_counting_runtime_on_detail_complete(void *context)
{
    app_counting_detail_context_t *detail_context = context;

    if (detail_context != NULL) {
        app_counting_runtime_handle_detail_complete(detail_context->session);
    }
}

static bool app_counting_runtime_is_main_page_active(void *context)
{
    (void)context;
    return app_counting_runtime_main_page_active();
}

static void app_counting_runtime_schedule_auto_wave(counting_session_state_t *session)
{
    session->auto_wave_pending = false;
    if (machine_state_work_mode() != 0 ||
        ui_manager_get_current_page() != UI_PAGE_WAVE_GET) {
        return;
    }

    session->auto_wave_pending = true;
    uart_debug_printf("auto wave scheduled after count\n");
}

void app_counting_runtime_handle_info(counting_session_state_t *session,
                                      counting_sim_t *sim_data,
                                      const uint8_t *buf,
                                      uint8_t len)
{
    counting_info_reply_result_t result;

    if (session == NULL || sim_data == NULL) {
        return;
    }

    result = counting_info_reply_handle(session, sim_data, buf, len,
                                        ui_history_total_notes_counted_get());
    if (result.kind == COUNTING_INFO_REPLY_LIVE) {
        app_counting_runtime_refresh_compact(sim_data);
        counting_history_append_frame("0x0E", buf, len);
        if (fault_popup_get_auto_enabled() ||
            !fault_popup_has_pending_start_issue()) {
            smart_island_notify_count_start();
            smart_island_refresh_summary();
        }
    } else if (result.kind == COUNTING_INFO_REPLY_FINISHED) {
        uart_debug_printf("Count finished\n");
        counting_history_capture_end(buf, len);
        smart_island_set_count_analysis(session->analysis_valid_pcs,
                                        result.final_issue,
                                        0);
        app_counting_runtime_try_history_commit(session, sim_data,
                                                lv_tick_get());
        if (app_counting_runtime_main_page_active()) {
            ui_refresh_main_page();
        }
        app_counting_runtime_schedule_auto_wave(session);
    }
}

void app_counting_runtime_handle_control(uint8_t cmd,
                                         counting_session_state_t *session,
                                         const uint8_t *buf,
                                         uint8_t len)
{
    if (session == NULL) {
        return;
    }

    counting_control_reply_dispatch(cmd,
                                    session,
                                    buf,
                                    len,
                                    &g_counting_control_hooks);
}

void app_counting_runtime_handle_denom(counting_detail_state_t *detail_state,
                                       counting_session_state_t *session,
                                       counting_sim_t *sim_data,
                                       const uint8_t *buf,
                                       uint8_t len)
{
    if (detail_state == NULL || session == NULL || sim_data == NULL) {
        return;
    }

    counting_denom_reply_handle(detail_state,
                                session,
                                sim_data,
                                buf,
                                len,
                                &g_counting_denom_hooks);
}

void app_counting_runtime_handle_detail(uint8_t cmd,
                                        counting_detail_state_t *detail_state,
                                        counting_session_state_t *session,
                                        counting_sim_t *sim_data,
                                        const uint8_t *buf,
                                        uint8_t len)
{
    app_counting_detail_context_t context;
    counting_reject_sn_reply_hooks_t hooks = { 0 };

    if (detail_state == NULL || session == NULL || sim_data == NULL) {
        return;
    }

    context.session = session;
    context.sim_data = sim_data;
    hooks.context = &context;
    hooks.on_history_frame = app_counting_runtime_on_detail_history;
    hooks.on_reject_analysis_ready = app_counting_runtime_on_reject_analysis;
    hooks.on_history_record_ready = app_counting_runtime_on_history_record;
    hooks.on_detail_complete = app_counting_runtime_on_detail_complete;
    hooks.is_main_page_active = app_counting_runtime_is_main_page_active;

    counting_reject_sn_reply_dispatch(cmd,
                                      detail_state,
                                      session,
                                      sim_data,
                                      buf,
                                      len,
                                      &hooks);
}

void app_counting_runtime_handle_detail_complete(counting_session_state_t *session)
{
    bool sent;

    if (session == NULL || !session->auto_wave_pending) {
        return;
    }
    session->auto_wave_pending = false;

    if (machine_state_work_mode() != 0 ||
        ui_manager_get_current_page() != UI_PAGE_WAVE_GET) {
        return;
    }

    sent = ui_page_31_get_wave_request();
    uart_debug_printf("auto wave after count: sent=%d\n", sent ? 1 : 0);
}

void app_counting_runtime_poll_history(counting_session_state_t *session,
                                       const counting_sim_t *sim_data,
                                       uint32_t now_ms)
{
    counting_history_commit_result_t result;
    uint8_t previous_attempts;

    if (session == NULL || sim_data == NULL) {
        return;
    }

    previous_attempts = session->history_record.save_attempts;
    result = counting_history_poll_commit(session, sim_data, now_ms);
    app_counting_runtime_report_history_commit(session, result,
                                               previous_attempts);
}
