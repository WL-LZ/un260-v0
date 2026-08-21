#include "app_currency_runtime.h"

#include <stdbool.h>
#include "lvgl/lvgl.h"

#include "un260/app_service/app_clock.h"
#include "un260/app_service/app_counting_runtime.h"
#include "un260/boot/boot_service.h"
#include "un260/counting/counting_denom_query_service.h"
#include "un260/currency/currency_reply.h"
#include "un260/lv_core/page_07_curr.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_system/platform_app.h"
#include "un260/protocol/protocol_send.h"

#define APP_CURRENCY_CLEAR_DATA_CMD 0x01

static bool app_currency_runtime_boot_ready(void)
{
    boot_stage_t stage = boot_service_get_stage();

    return stage == BOOT_STAGE_DONE || stage == BOOT_STAGE_FAIL;
}

static void app_currency_runtime_trigger_denom_query(
    counting_detail_state_t *detail_state)
{
    counting_denom_query_trigger(detail_state,
                                 app_clock_uptime_ms(),
                                 app_currency_runtime_boot_ready());
}

void app_currency_runtime_handle_reply(counting_detail_state_t *detail_state,
                                       counting_session_state_t *session,
                                       const uint8_t *buf,
                                       uint8_t len)
{
    currency_reply_result_t reply;

    if (detail_state == NULL || session == NULL) {
        return;
    }

    reply = currency_reply_handle(buf, len);
    if (reply.kind == CURRENCY_REPLY_SWITCH_SUCCESS) {
        const uint8_t clear_data = APP_CURRENCY_CLEAR_DATA_CMD;

        sim_reset_for_currency(&sim);
        app_counting_runtime_reset_session(session, "currency change");
        if (protocol_send(0x3B, &clear_data, 1) < 0) {
            uart_printf(fd6, "currency changed, controller data clear send failed\n");
        }
        page_07_curr_apply_switch_result(&reply.switch_result);
        uart_printf(fd6, "Set %s curr success\n", reply.active_code);
        detail_state->wait_sn_after_reject_end = false;
        app_currency_runtime_trigger_denom_query(detail_state);
    } else if (reply.kind == CURRENCY_REPLY_SWITCH_FAILURE) {
        page_07_curr_apply_switch_result(&reply.switch_result);
        uart_printf(fd6, "Set %s curr fail\n", reply.active_code);
    } else if (reply.kind == CURRENCY_REPLY_BOOT_ACTIVE) {
        uart_printf(fd6, "Boot curr: %s\n", reply.active_code);
        sim_reset_for_currency(&sim);
        app_counting_runtime_reset_session(session, "boot currency sync");
        detail_state->wait_sn_after_reject_end = false;
        counting_denom_query_invalidate(detail_state);
        app_currency_runtime_trigger_denom_query(detail_state);
    }
}
