#include "counting_denom_query_service.h"

#include <stddef.h>

#include "un260/lv_drivers/lv_drivers.h"

#define DENOM_QUERY_CMD           0x0B
#define DENOM_QUERY_SUBCMD        0x01
#define DENOM_QUERY_TIMEOUT_MS    1500
#define DENOM_QUERY_MAX_RETRY     2
#define DENOM_QUERY_IDLE_RETRY_MS 2500

static void counting_denom_query_send(counting_detail_state_t *detail,
                                      uint32_t now_ms)
{
    const uint8_t subcmd = DENOM_QUERY_SUBCMD;

    send_command(fd4, DENOM_QUERY_CMD, &subcmd, 1);
    detail->query_pending = true;
    detail->query_got_frame = false;
    detail->query_tick = now_ms;
    uart_printf(fd6, "request denom list: 0x0B 0x01\n");
}

void counting_denom_query_invalidate(counting_detail_state_t *detail)
{
    if (detail != NULL) {
        detail->query_got_frame = false;
    }
}

void counting_denom_query_mark_frame_received(counting_detail_state_t *detail)
{
    if (detail != NULL) {
        detail->query_got_frame = true;
    }
}

bool counting_denom_query_complete(counting_detail_state_t *detail)
{
    bool was_pending;

    if (detail == NULL) {
        return false;
    }

    was_pending = detail->query_pending;
    detail->query_pending = false;
    detail->query_got_frame = true;
    detail->query_retry = 0;
    return was_pending;
}

void counting_denom_query_trigger(counting_detail_state_t *detail,
                                  uint32_t now_ms,
                                  bool boot_ready)
{
    if (detail == NULL) {
        return;
    }

    detail->query_retry = 0;
    if (!boot_ready) {
        detail->query_deferred = true;
        uart_printf(fd6, "defer denom query until boot done\n");
        return;
    }

    detail->query_deferred = false;
    counting_denom_query_send(detail, now_ms);
}

void counting_denom_query_poll(counting_detail_state_t *detail,
                               uint32_t now_ms,
                               bool boot_ready,
                               bool main_page_active)
{
    if (detail == NULL) {
        return;
    }

    if (detail->query_pending &&
        (uint32_t)(now_ms - detail->query_tick) >= DENOM_QUERY_TIMEOUT_MS) {
        detail->query_pending = false;
        if (!detail->query_got_frame) {
            if (detail->query_retry < DENOM_QUERY_MAX_RETRY) {
                detail->query_retry++;
                uart_printf(fd6, "0x0B query timeout, retry %u/%u\n",
                            detail->query_retry, DENOM_QUERY_MAX_RETRY);
                counting_denom_query_send(detail, now_ms);
                return;
            }

            detail->query_idle_retry_tick = now_ms;
            uart_printf(fd6,
                        "0x0B query timeout, keep master-only mode (no local fallback)\n");
            return;
        }
    }

    if (detail->query_deferred && !detail->query_pending && boot_ready) {
        detail->query_deferred = false;
        counting_denom_query_send(detail, now_ms);
        return;
    }

    if (!detail->query_pending && !detail->query_got_frame &&
        main_page_active && boot_ready &&
        (uint32_t)(now_ms - detail->query_idle_retry_tick) >= DENOM_QUERY_IDLE_RETRY_MS) {
        detail->query_idle_retry_tick = now_ms;
        detail->query_retry = 0;
        uart_printf(fd6, "0x0B idle retry on main page\n");
        counting_denom_query_send(detail, now_ms);
    }
}
