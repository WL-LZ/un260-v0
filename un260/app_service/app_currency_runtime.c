#include "app_currency_runtime.h"

#include <stdbool.h>
#include <string.h>

#include "lvgl/lvgl.h"

#include "un260/app_service/app_clock.h"
#include "un260/boot/boot_service.h"
#include "un260/counting/counting_denom_query_service.h"
#include "un260/currency/currency_reply.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/page_07_curr.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_system/platform_app.h"

static bool app_currency_runtime_boot_ready(void)
{
    boot_stage_t stage = boot_service_get_stage();

    return stage == BOOT_STAGE_DONE || stage == BOOT_STAGE_FAIL;
}

static bool app_currency_runtime_main_page_active(void)
{
    return ui_manager_get_current_page() == UI_PAGE_MAIN &&
           main_page != NULL && lv_obj_is_valid(main_page);
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
        set_curr(get_curr_item(reply.switch_result.target_code));
        sim_clear_all_sn(&sim);
        page_07_curr_apply_switch_result(&reply.switch_result);
        uart_printf(fd6, "Set %s curr success\n", reply.active_code);
        session->end_anim_wait_detail = false;
        app_currency_runtime_trigger_denom_query(detail_state);
        smart_island_refresh_summary();
    } else if (reply.kind == CURRENCY_REPLY_SWITCH_FAILURE) {
        page_07_curr_apply_switch_result(&reply.switch_result);
        uart_printf(fd6, "Set %s curr fail\n", reply.active_code);
    } else if (reply.kind == CURRENCY_REPLY_BOOT_ACTIVE) {
        uart_printf(fd6, "Boot curr: %s\n", reply.active_code);
        memset(sim.denom, 0, sizeof(sim.denom));
        sim.denom_number = 0;
        counting_denom_query_invalidate(detail_state);
        if (app_currency_runtime_main_page_active()) {
            ui_refresh_main_page();
        }
        app_currency_runtime_trigger_denom_query(detail_state);
        smart_island_refresh_summary();
    }
}
