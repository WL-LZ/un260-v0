#include "app_counting_runtime.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "lvgl/lvgl.h"

#include "un260/counting/counting_history_service.h"
#include "un260/counting/counting_info_reply.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/machine_state/machine_state.h"

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
           main_page != NULL && lv_obj_is_valid(main_page);
}

static void app_counting_runtime_schedule_auto_wave(counting_session_state_t *session)
{
    session->auto_wave_pending = false;
    if (machine_state_work_mode() != 0 ||
        ui_manager_get_current_page() != UI_PAGE_WAVE_GET) {
        return;
    }

    session->auto_wave_pending = true;
    uart_printf(fd6, "auto wave scheduled after count\n");
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

    result = counting_info_reply_handle(session, sim_data, buf, len);
    if (result.kind == COUNTING_INFO_REPLY_LIVE) {
        app_counting_runtime_refresh_compact(sim_data);
        counting_history_append_frame("0x0E", buf, len);
        if (fault_popup_get_auto_enabled() ||
            !fault_popup_has_pending_start_issue()) {
            smart_island_notify_count_start();
            smart_island_refresh_summary();
        }
    } else if (result.kind == COUNTING_INFO_REPLY_FINISHED) {
        uart_printf(fd6, "Count finished\n");
        counting_history_capture_end(buf, len);
        smart_island_set_count_analysis(session->analysis_valid_pcs,
                                        result.final_issue,
                                        0);
        counting_history_try_commit(session, sim_data);
        if (app_counting_runtime_main_page_active()) {
            ui_refresh_main_page();
        }
        app_counting_runtime_schedule_auto_wave(session);
    }
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
    uart_printf(fd6, "auto wave after count: sent=%d\n", sent ? 1 : 0);
}
