#include "app_boot_runtime.h"

#include <stddef.h>

#include "lvgl/lvgl.h"

#include "un260/boot/boot_reply.h"
#include "un260/boot/boot_service.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/page_08_boot.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_system/platform_app.h"

#define APP_BOOT_FINISH_DELAY_MS       2000
#define APP_BOOT_CURRENCY_LIST_CMD     0x56
#define APP_BOOT_CURRENCY_LIST_REQUEST 0x01

static void app_boot_runtime_request_currency_list(void)
{
    const uint8_t request = APP_BOOT_CURRENCY_LIST_REQUEST;

    send_command(fd4, APP_BOOT_CURRENCY_LIST_CMD, &request, 1);
}

static void app_boot_runtime_finish(counting_session_state_t *counting_session)
{
    boot_selftest_list_finish();
    sim_data_init();
    if (counting_session != NULL) {
        counting_session->active = false;
        counting_session->wait_start_ack = false;
        counting_session->end_anim_wait_detail = false;
        counting_session->last_result.valid = false;
    }
    ui_manager_switch(ui_state_pure_count_is_enabled() ? UI_PAGE_PURE : UI_PAGE_MAIN);
}

static void app_boot_runtime_finish_timer_cb(lv_timer_t *timer)
{
    counting_session_state_t *counting_session;

    if (timer == NULL) {
        return;
    }
    counting_session = (counting_session_state_t *)timer->user_data;
    app_boot_runtime_finish(counting_session);
    lv_timer_del(timer);
}

void app_boot_runtime_handle_reply(counting_session_state_t *counting_session,
                                   uint8_t cmd,
                                   const uint8_t *buf,
                                   uint8_t len)
{
    boot_reply_result_t reply = boot_reply_dispatch(cmd, buf, len);

    if (reply.kind == BOOT_REPLY_HANDSHAKE_ACCEPTED) {
        boot_progress_set(20);
        boot_send_next_selftest();
        return;
    }
    if (reply.kind != BOOT_REPLY_SELF_TEST_RECORDED) {
        return;
    }

    boot_selftest_list_set_result(reply.self_test_index, reply.self_test_result);
    boot_progress_set((uint8_t)(30 + reply.self_test_index * 10));

    if (reply.self_test_event == BOOT_SELF_TEST_EVENT_NONE) {
        boot_send_next_selftest();
    } else if (reply.self_test_event == BOOT_SELF_TEST_EVENT_SUCCESS) {
        lv_timer_t *finish_timer;

        boot_progress_set(100);
        app_boot_runtime_request_currency_list();
        finish_timer = lv_timer_create(app_boot_runtime_finish_timer_cb,
                                       APP_BOOT_FINISH_DELAY_MS,
                                       counting_session);
        if (finish_timer == NULL) {
            app_boot_runtime_finish(counting_session);
        }
    } else if (reply.self_test_event == BOOT_SELF_TEST_EVENT_FAILURE) {
        app_boot_runtime_request_currency_list();
        show_boot_fault_popup(reply.first_failure_step, reply.first_failure_result);
    }
}

void app_boot_runtime_poll(uint32_t now_ms, bool boot_page_active)
{
    boot_service_action_t action;

    if (!boot_page_active) {
        return;
    }

    action = boot_service_poll(now_ms);
    if (action == BOOT_SERVICE_ACTION_SEND_HANDSHAKE) {
        machine_handshake_send();
    } else if (action == BOOT_SERVICE_ACTION_HANDSHAKE_TIMEOUT) {
        show_boot_selftest_error_popup(
            "Controller handshake timeout.\nPress CONFIRM to enter sensor page.");
    } else if (action == BOOT_SERVICE_ACTION_SELF_TEST_TIMEOUT) {
        show_boot_selftest_error_popup(
            "Self-test timeout.\nPress CONFIRM to enter sensor page.");
    }
}
