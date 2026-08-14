#include "counting_reject_sn_reply.h"

#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "un260/counting/counting_reject_reason.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_core/page_01_detail_scroll.h"
#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/page_02_list.h"
#include "un260/lv_drivers/lv_drivers.h"

#define COUNTING_SN_MAX_COUNT 10000

static void counting_detail_record_history(
    const counting_reject_sn_reply_hooks_t *hooks,
    const char *tag,
    const uint8_t *buf,
    uint8_t len)
{
    if (hooks != NULL && hooks->on_history_frame != NULL) {
        hooks->on_history_frame(hooks->context, tag, buf, len);
    }
}

static bool counting_detail_is_main_page_active(
    const counting_reject_sn_reply_hooks_t *hooks)
{
    return hooks != NULL && hooks->is_main_page_active != NULL &&
           hooks->is_main_page_active(hooks->context);
}

static uint8_t counting_reject_page_count(uint16_t item_count)
{
    unsigned int page_count;

    if (item_count == 0) {
        return 1;
    }

    page_count = ((unsigned int)item_count + PAGE_02_C_ITEM - 1U) / PAGE_02_C_ITEM;
    return page_count > UINT8_MAX ? UINT8_MAX : (uint8_t)page_count;
}

static void counting_reject_refresh_pages(const counting_sim_t *sim_data)
{
    page_02_c_report_status.total_page = counting_reject_page_count(sim_data->err_num);
    page_02_c_page_refre();
    page_02_c_page_num_refre();
}

static void counting_sn_clear(counting_sim_t *sim_data)
{
    if (sim_data == NULL) {
        return;
    }

    if (sim_data->sn_str != NULL) {
        int capacity = sim_data->sn_capacity;
        if (capacity < 0 || capacity > COUNTING_SN_MAX_COUNT) {
            capacity = 0;
        }
        for (int i = 0; i < capacity; i++) {
            free(sim_data->sn_str[i]);
        }
        free(sim_data->sn_str);
        sim_data->sn_str = NULL;
    }

    memset(sim_data->denom_mix, 0, sizeof(sim_data->denom_mix));
    sim_data->sn_capacity = 0;
    page_01_detail_scroll_reset_all();
}

static bool counting_sn_ensure_capacity(counting_sim_t *sim_data, int new_total)
{
    int old_capacity;
    int new_capacity;
    char **new_ptr;

    if (sim_data == NULL || new_total <= 0 || new_total > COUNTING_SN_MAX_COUNT) {
        return false;
    }

    old_capacity = sim_data->sn_str != NULL ? sim_data->sn_capacity : 0;
    if (old_capacity < 0 || old_capacity > COUNTING_SN_MAX_COUNT) {
        return false;
    }
    if (new_total <= old_capacity) {
        return true;
    }

    new_capacity = old_capacity > 0 ? old_capacity : 64;
    while (new_capacity < new_total) {
        if (new_capacity > COUNTING_SN_MAX_COUNT / 2) {
            new_capacity = COUNTING_SN_MAX_COUNT;
        } else {
            new_capacity *= 2;
        }
    }

    new_ptr = realloc(sim_data->sn_str, sizeof(*new_ptr) * (size_t)new_capacity);
    if (new_ptr == NULL) {
        return false;
    }
    memset(new_ptr + old_capacity, 0,
           sizeof(*new_ptr) * (size_t)(new_capacity - old_capacity));
    sim_data->sn_str = new_ptr;
    sim_data->sn_capacity = new_capacity;
    return true;
}

static counting_detail_reply_result_t counting_reject_reply_handle(
    counting_detail_state_t *detail,
    counting_sim_t *sim_data,
    const uint8_t *buf,
    uint8_t len,
    const counting_reject_sn_reply_hooks_t *hooks)
{
    uint8_t err_code;
    uint8_t pcs;

    if (detail == NULL || sim_data == NULL || buf == NULL || len < 7) {
        return COUNTING_DETAIL_REPLY_INVALID;
    }

    err_code = buf[4];
    pcs = buf[5];
    if (err_code == 0x00 && pcs == 0x00) {
        sim_clear_err_only(sim_data);
        counting_detail_record_history(hooks, "0x0C", buf, len);
        return COUNTING_DETAIL_REPLY_START;
    }

    if (err_code == 0xFF && pcs == 0xFF) {
        counting_detail_record_history(hooks, "0x0C", buf, len);
        page_02_c_report_status.curent_page = 1;
        counting_reject_refresh_pages(sim_data);
        if (hooks != NULL && hooks->on_reject_analysis_ready != NULL) {
            hooks->on_reject_analysis_ready(hooks->context);
        }
        uart_printf(fd6, "0x0C reject detail receive end, parsed=%u expected=%u\n",
                    sim_data->err_num, sim_data->err_expected);
        smart_island_refresh_summary();
        if (counting_detail_is_main_page_active(hooks)) {
            ui_refresh_main_page();
        }
        if (detail->wait_sn_after_reject_end) {
            uint8_t sn_req[2] = {0x01, 0x01};
            send_command(fd4, 0x0D, sn_req, 2);
            detail->wait_sn_after_reject_end = false;
        }
        return COUNTING_DETAIL_REPLY_END;
    }

    if (sim_data->err_expected == 0) {
        uart_printf(fd6, "0x0C detail ignored because err_expected=0\n");
        return COUNTING_DETAIL_REPLY_IGNORED;
    }

    if (!sim_ensure_err_capacity(sim_data, (int)sim_data->err_num + 1)) {
        uart_printf(fd6, "0x0C: err capacity fail idx=%u\n", sim_data->err_num);
        return COUNTING_DETAIL_REPLY_MEMORY_ERROR;
    }
    counting_detail_record_history(hooks, "0x0C", buf, len);

    {
        int index = sim_data->err_num;
        const char *description = counting_reject_reason_get(err_code);
        size_t description_len = strlen(description);
        char *description_copy = malloc(description_len + 1);

        if (description_copy == NULL) {
            uart_printf(fd6, "0x0C: err malloc fail idx=%d\n", index);
            return COUNTING_DETAIL_REPLY_MEMORY_ERROR;
        }
        memcpy(description_copy, description, description_len + 1);
        sim_data->err_str[index] = description_copy;
        sim_data->err_pcs[index] = pcs;
        sim_data->err_code[index] = err_code;
        sim_data->err_num++;
    }

    counting_reject_refresh_pages(sim_data);
    smart_island_refresh_summary();
    return COUNTING_DETAIL_REPLY_DATA;
}

static bool counting_sn_payload_is(const uint8_t *buf, int payload_end, uint8_t value)
{
    for (int i = 4; i < payload_end; i++) {
        if (buf[i] != value) {
            return false;
        }
    }
    return true;
}

static counting_detail_reply_result_t counting_sn_reply_handle(
    counting_session_state_t *session,
    counting_sim_t *sim_data,
    const uint8_t *buf,
    uint8_t len,
    const counting_reject_sn_reply_hooks_t *hooks)
{
    int payload_len;
    int payload_end;
    uint8_t sequence;
    int index;
    int ascii_len;
    char ascii_buf[32];
    char *cursor;
    int denom = 0;

    if (session == NULL || sim_data == NULL || buf == NULL || len < 6) {
        return COUNTING_DETAIL_REPLY_INVALID;
    }

    payload_len = len - 4;
    if (payload_len < 2) {
        return COUNTING_DETAIL_REPLY_INVALID;
    }
    payload_end = len - 1;

    if (counting_sn_payload_is(buf, payload_end, 0x00)) {
        counting_sn_clear(sim_data);
        counting_detail_record_history(hooks, "0x0D", buf, len);
        return COUNTING_DETAIL_REPLY_START;
    }

    if (counting_sn_payload_is(buf, payload_end, 0xFF)) {
        page_02_report_init();
        page_02_b_page_refre();
        page_02_b_page_num_refre();
        counting_detail_record_history(hooks, "0x0D", buf, len);
        session->history_record.end_seen = true;
        if (hooks != NULL && hooks->on_history_record_ready != NULL) {
            hooks->on_history_record_ready(hooks->context);
        }
        if (counting_detail_is_main_page_active(hooks)) {
            ui_refresh_main_page();
        }
        if (session->end_anim_wait_detail) {
            session->end_anim_wait_detail = false;
            ui_count_end_anim_begin(NULL);
        }
        if (hooks != NULL && hooks->on_detail_complete != NULL) {
            hooks->on_detail_complete(hooks->context);
        }
        return COUNTING_DETAIL_REPLY_END;
    }

    sequence = buf[4];
    if (sequence == 0x00 || sequence == 0xFF) {
        return COUNTING_DETAIL_REPLY_IGNORED;
    }
    index = (int)sequence - 1;
    if (index < 0 || index >= COUNTING_SN_MAX_COUNT) {
        return COUNTING_DETAIL_REPLY_IGNORED;
    }

    ascii_len = payload_len - 2;
    if (ascii_len <= 0) {
        return COUNTING_DETAIL_REPLY_IGNORED;
    }
    counting_detail_record_history(hooks, "0x0D", buf, len);

    if (ascii_len >= (int)sizeof(ascii_buf)) {
        ascii_len = (int)sizeof(ascii_buf) - 1;
    }
    memcpy(ascii_buf, &buf[5], (size_t)ascii_len);
    ascii_buf[ascii_len] = '\0';

    while (ascii_len > 0 && ascii_buf[ascii_len - 1] == ' ') {
        ascii_buf[--ascii_len] = '\0';
    }
    cursor = ascii_buf;
    while (*cursor == ' ') {
        cursor++;
    }
    if (*cursor == '\0') {
        return COUNTING_DETAIL_REPLY_IGNORED;
    }

    while (*cursor != '\0' && isdigit((unsigned char)*cursor)) {
        int digit = *cursor - '0';
        if (denom > (INT_MAX - digit) / 10) {
            return COUNTING_DETAIL_REPLY_IGNORED;
        }
        denom = denom * 10 + digit;
        cursor++;
    }
    while (*cursor == ' ') {
        cursor++;
    }
    if (*cursor == '\0') {
        return COUNTING_DETAIL_REPLY_IGNORED;
    }

    if (!counting_sn_ensure_capacity(sim_data, index + 1)) {
        uart_printf(fd6, "0x0D: SN capacity fail idx=%d\n", index);
        return COUNTING_DETAIL_REPLY_MEMORY_ERROR;
    }

    {
        size_t sn_len = strlen(cursor);
        char *sn_copy = malloc(sn_len + 1);
        if (sn_copy == NULL) {
            uart_printf(fd6, "0x0D: SN malloc fail idx=%d\n", index);
            return COUNTING_DETAIL_REPLY_MEMORY_ERROR;
        }
        memcpy(sn_copy, cursor, sn_len + 1);
        free(sim_data->sn_str[index]);
        sim_data->sn_str[index] = sn_copy;
        sim_data->denom_mix[index] = denom;
    }

    if (counting_detail_is_main_page_active(hooks) &&
        page_01_detail_section_get() == PAGE_01_DETAIL_SECTION_B) {
        page_01_main_detail_refresh_rows_only();
    }
    return COUNTING_DETAIL_REPLY_DATA;
}

counting_detail_reply_result_t counting_reject_sn_reply_dispatch(
    uint8_t cmd,
    counting_detail_state_t *detail,
    counting_session_state_t *session,
    counting_sim_t *sim_data,
    const uint8_t *buf,
    uint8_t len,
    const counting_reject_sn_reply_hooks_t *hooks)
{
    switch (cmd) {
    case 0x0C:
        return counting_reject_reply_handle(detail, sim_data, buf, len, hooks);
    case 0x0D:
        return counting_sn_reply_handle(session, sim_data, buf, len, hooks);
    default:
        return COUNTING_DETAIL_REPLY_INVALID;
    }
}
