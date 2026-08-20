#include "counting_denom_reply.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "counting_denom_query_service.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/protocol/protocol_send.h"

static void counting_denom_record_history(const counting_denom_reply_hooks_t *hooks,
                                          const uint8_t *buf,
                                          uint8_t len)
{
    if (hooks != NULL && hooks->on_history_frame != NULL) {
        hooks->on_history_frame("0x0B", buf, len);
    }
}

static bool counting_denom_payload_is(const uint8_t *buf, uint8_t value)
{
    for (int i = 4; i < 15; i++) {
        if (buf[i] != value) {
            return false;
        }
    }
    return true;
}

static counting_denom_reply_result_t counting_denom_handle_end(
    counting_detail_state_t *detail,
    const counting_session_state_t *session,
    const uint8_t *buf,
    uint8_t len,
    const counting_denom_reply_hooks_t *hooks)
{
    bool query_was_pending;

    uart_printf(fd6, "0x0B denom detail receive end\n");
    query_was_pending = counting_denom_query_complete(detail);
    counting_denom_record_history(hooks, buf, len);

    if (query_was_pending) {
        if (!session->active) {
            ui_refresh_main_page();
        }
        return COUNTING_DENOM_REPLY_QUERY_END;
    }

    {
        uint8_t reject_cmd = 0x01;
        protocol_send(0x0C, &reject_cmd, 1);
    }
    detail->wait_sn_after_reject_end = true;
    if (!session->active) {
        ui_refresh_main_page();
    }
    return COUNTING_DENOM_REPLY_SESSION_END;
}

static counting_denom_reply_result_t counting_denom_handle_data(
    counting_detail_state_t *detail,
    counting_sim_t *sim_data,
    const uint8_t *buf,
    uint8_t len,
    const counting_denom_reply_hooks_t *hooks)
{
    char denom_text[9] = {0};
    char pcs_text[4] = {0};
    int denom;
    int pcs;

    memcpy(denom_text, &buf[4], 8);
    memcpy(pcs_text, &buf[12], 3);
    denom = atoi(denom_text);
    pcs = atoi(pcs_text);

    if (denom <= 0) {
        return COUNTING_DENOM_REPLY_IGNORED;
    }

    counting_denom_query_mark_frame_received(detail);
    counting_denom_record_history(hooks, buf, len);

    for (int i = 0; i < sim_data->denom_number; i++) {
        if (sim_data->denom[i].value == denom) {
            sim_data->denom[i].pcs += pcs;
            sim_data->denom[i].amount = denom * sim_data->denom[i].pcs;
            return COUNTING_DENOM_REPLY_DATA;
        }
    }

    if (sim_data->denom_number <
        (int)(sizeof(sim_data->denom) / sizeof(sim_data->denom[0]))) {
        int index = sim_data->denom_number;
        sim_data->denom[index].value = denom;
        sim_data->denom[index].pcs = pcs;
        sim_data->denom[index].amount = denom * pcs;
        sim_data->denom_number++;
    }

    return COUNTING_DENOM_REPLY_DATA;
}

counting_denom_reply_result_t counting_denom_reply_handle(
    counting_detail_state_t *detail,
    const counting_session_state_t *session,
    counting_sim_t *sim_data,
    const uint8_t *buf,
    uint8_t len,
    const counting_denom_reply_hooks_t *hooks)
{
    if (detail == NULL || session == NULL || sim_data == NULL || buf == NULL || len < 15) {
        return COUNTING_DENOM_REPLY_INVALID;
    }

    if (counting_denom_payload_is(buf, 0x00)) {
        memset(sim_data->denom, 0, sizeof(sim_data->denom));
        sim_data->denom_number = 0;
        counting_denom_query_mark_frame_received(detail);
        counting_denom_record_history(hooks, buf, len);
        uart_printf(fd6, "0x0B denom detail receive start\n");
        return COUNTING_DENOM_REPLY_START;
    }

    if (counting_denom_payload_is(buf, 0xFF)) {
        return counting_denom_handle_end(detail, session, buf, len, hooks);
    }

    return counting_denom_handle_data(detail, sim_data, buf, len, hooks);
}
