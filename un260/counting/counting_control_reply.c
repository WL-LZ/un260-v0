#include "counting_control_reply.h"

#include <stddef.h>

#include "un260/counting/counting_history_service.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_system/ui_text.h"

static const char *counting_start_ui_error_desc(uint8_t code)
{
    if (code == 0x00) {
        return ui_text_get(UI_TEXT_WIDGET_FAULT_NO_NOTE_MAIN);
    }

    if (code < sizeof(g_start_error_desc) / sizeof(g_start_error_desc[0]) &&
        g_start_error_desc[code] != NULL) {
        return g_start_error_desc[code];
    }

    return ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR);
}

static void counting_control_report_error(const counting_control_reply_hooks_t *hooks,
                                          const char *tag,
                                          const uint8_t *buf,
                                          uint8_t len)
{
    if (hooks != NULL && hooks->on_error_frame != NULL) {
        hooks->on_error_frame(tag, buf, len);
    }
}

static void counting_control_report_start_failure(const counting_control_reply_hooks_t *hooks,
                                                  const char *description)
{
    if (hooks != NULL && hooks->on_start_failure != NULL) {
        hooks->on_start_failure(description);
    }
}

static void counting_control_handle_runtime_fault(const uint8_t *buf,
                                                  uint8_t len,
                                                  const counting_control_reply_hooks_t *hooks)
{
    uint8_t fault;

    if (buf == NULL || len < 6) {
        return;
    }

    fault = buf[4];
    if (fault == 0x00) {
        hide_fault_popup();
        fault_popup_clear_pending();
        fault_popup_reset_auto_retry();
        g_sys_err_last_code = 0x00;
        smart_island_restore_idle();
        return;
    }

    fault_popup_report_runtime_fault(fault);
    counting_control_report_error(hooks, "0x0F", buf, len);
    uart_printf(fd6, "0x0F fault=0x%02X %s\n", fault, get_system_error_desc(fault));
    smart_island_notify_warning_level(get_system_error_desc(fault),
                                      SMART_ISLAND_WARNING_LEVEL_ERROR);
}

static void counting_control_handle_start(const uint8_t *buf,
                                          uint8_t len,
                                          counting_session_state_t *session,
                                          const counting_control_reply_hooks_t *hooks)
{
    const char *description;
    uint8_t type;
    uint8_t value;

    if (session == NULL || buf == NULL || len < 7) {
        return;
    }

    type = buf[4];
    value = buf[5];

    if (type == 0x01 && value == 0x01) {
        if (counting_history_discard_pending(session)) {
            uart_printf(fd6, "pending history discarded by new counting session\n");
        }
        hide_counting_error_popup();
        fault_popup_clear_pending();
        fault_popup_reset_auto_retry();
        session->wait_start_ack = false;
        session->end_anim_wait_detail = false;
        session->expected_issue = 0;
        if (hooks != NULL && hooks->on_start_success != NULL) {
            hooks->on_start_success(buf, len);
        }
        smart_island_notify_count_start();
        return;
    }

    if (type == 0x01 && value == 0x02) {
        fault_popup_report_start_no_note();
        counting_control_report_error(hooks, "0x0A", buf, len);
        uart_printf(fd6, "0x0A start fail (no note)\n");
        smart_island_notify_warning_level(
            ui_text_get(UI_TEXT_WIDGET_FAULT_NO_NOTE_MAIN),
            SMART_ISLAND_WARNING_LEVEL_WARNING);
        counting_control_report_start_failure(hooks, "No banknotes detected");
        return;
    }

    fault_popup_report_start_fault(type, value);
    counting_control_report_error(hooks, "0x0A", buf, len);
    description = get_counting_error_desc(type, value);

    if (type == 0x01) {
        uart_printf(fd6, "0x0A start fail (normal): val=%02X desc=%s\n",
                    value, description);
        smart_island_notify_warning_level(counting_start_ui_error_desc(value),
                                          SMART_ISLAND_WARNING_LEVEL_ERROR);
        counting_control_report_start_failure(hooks, description);
    } else if (type == 0x02) {
        uart_printf(fd6, "0x0A start fail (fault): code=%02X desc=%s\n",
                    value, description);
        smart_island_notify_warning_level(counting_start_ui_error_desc(value),
                                          SMART_ISLAND_WARNING_LEVEL_ERROR);
        counting_control_report_start_failure(hooks, description);
    } else {
        uart_printf(fd6, "0x0A start fail (unknown type): type=%02X val=%02X\n",
                    type, value);
        smart_island_notify_warning_level(
            ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR),
            SMART_ISLAND_WARNING_LEVEL_ERROR);
    }
}

bool counting_control_reply_dispatch(uint8_t cmd,
                                     counting_session_state_t *session,
                                     const uint8_t *buf,
                                     uint8_t len,
                                     const counting_control_reply_hooks_t *hooks)
{
    switch (cmd) {
    case 0x0A:
        counting_control_handle_start(buf, len, session, hooks);
        return true;
    case 0x0F:
        counting_control_handle_runtime_fault(buf, len, hooks);
        return true;
    default:
        return false;
    }
}
