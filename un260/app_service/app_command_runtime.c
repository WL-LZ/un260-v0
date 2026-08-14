#include "app_command_runtime.h"

#include <stdbool.h>
#include <stddef.h>

#include "lvgl/lvgl.h"

#include "un260/app_service/app_boot_runtime.h"
#include "un260/app_service/app_counting_runtime.h"
#include "un260/app_service/app_currency_runtime.h"
#include "un260/app_service/app_protocol_runtime.h"
#include "un260/app_service/app_setting_runtime.h"
#include "un260/boot/boot_service.h"
#include "un260/counting/counting_denom_query_service.h"
#include "un260/counting/counting_session_state.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_system/platform_app.h"
#include "un260/protocol/protocol_frame.h"
#include "un260/protocol/protocol_frame_format.h"
#include "un260/protocol/protocol_frame_queue.h"

#define APP_COMMAND_MAX_FRAMES_PER_TICK 64

static counting_detail_state_t g_counting_detail_state;
static counting_session_state_t g_counting_session;

static bool app_command_runtime_main_page_active(void)
{
    return ui_manager_get_current_page() == UI_PAGE_MAIN &&
           main_page != NULL && lv_obj_is_valid(main_page);
}

static void app_command_runtime_dispatch(uint8_t cmd,
                                         uint8_t *buf,
                                         uint8_t len)
{
    if (app_setting_runtime_handle_reply(cmd, buf, len) ||
        app_protocol_runtime_handle_reply(cmd, buf, len)) {
        return;
    }

    switch (cmd) {
    case 0x01:
    case 0x37:
        app_boot_runtime_handle_reply(&g_counting_session, cmd, buf, len);
        break;
    case 0x03:
        app_currency_runtime_handle_reply(&g_counting_detail_state,
                                          &g_counting_session,
                                          buf,
                                          len);
        break;
    case 0x0E:
        app_counting_runtime_handle_info(&g_counting_session, &sim, buf, len);
        break;
    case 0x0F:
    case 0x0A:
        app_counting_runtime_handle_control(cmd,
                                            &g_counting_session,
                                            buf,
                                            len);
        break;
    case 0x0B:
        app_counting_runtime_handle_denom(&g_counting_detail_state,
                                          &g_counting_session,
                                          &sim,
                                          buf,
                                          len);
        break;
    case 0x0D:
    case 0x0C:
        app_counting_runtime_handle_detail(cmd,
                                           &g_counting_detail_state,
                                           &g_counting_session,
                                           &sim,
                                           buf,
                                           len);
        break;
    default:
        uart_printf(fd6, "Unknown command 0x%02X\n", cmd);
        break;
    }
}

void app_command_runtime_process_frames(void)
{
    protocol_frame_t frame;
    int processed = 0;

    while (processed < APP_COMMAND_MAX_FRAMES_PER_TICK &&
           protocol_frame_queue_pop(&frame)) {
        char hex_log[256];
        uint8_t *buf = frame.data;
        uint8_t len = frame.len;

        processed++;
        protocol_frame_format_hex(buf, len, hex_log, sizeof(hex_log));
        debug_append_rx_log(hex_log);

        if (len < PROTOCOL_FRAME_MIN_SIZE) {
            uart_printf(fd6, "Queued frame dropped: invalid len=%u\n", len);
            continue;
        }

        app_command_runtime_dispatch(buf[3], buf, len);
    }
}

void app_command_runtime_poll(uint32_t now_ms)
{
    boot_stage_t stage = boot_service_get_stage();

    counting_denom_query_poll(&g_counting_detail_state,
                              now_ms,
                              stage == BOOT_STAGE_DONE || stage == BOOT_STAGE_FAIL,
                              app_command_runtime_main_page_active());
}
