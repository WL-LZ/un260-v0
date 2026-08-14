#include "counting_history_service.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/page_19_history.h"
#include "un260/lv_system/ui_history_data.h"

#define COUNTING_HISTORY_FRAME_TEXT_SIZE 160
#define COUNTING_HISTORY_SESSION_LOG_SIZE 4096
#define COUNTING_HISTORY_HEX_BUFFER_SIZE (UINT8_MAX * 3U + 1U)

static char g_last_error_frame_text[COUNTING_HISTORY_FRAME_TEXT_SIZE];
static char g_last_start_frame_text[COUNTING_HISTORY_FRAME_TEXT_SIZE];
static char g_last_end_frame_text[COUNTING_HISTORY_FRAME_TEXT_SIZE];
static char g_session_log_text[COUNTING_HISTORY_SESSION_LOG_SIZE];
static size_t g_session_log_len;

static void counting_history_frame_to_hex(const uint8_t *buf,
                                          uint8_t len,
                                          char *out,
                                          size_t out_size)
{
    size_t pos = 0;

    if (out == NULL || out_size == 0U) {
        return;
    }
    out[0] = '\0';
    if (buf == NULL || len == 0U) {
        return;
    }

    for (uint16_t i = 0; i < len && pos + 3U < out_size; i++) {
        int written = snprintf(out + pos, out_size - pos, "%02X ", buf[i]);

        if (written != 3) {
            break;
        }
        pos += 3U;
    }
    if (pos > 0U) {
        out[pos - 1U] = '\0';
    }
}

static void counting_history_session_reset(void)
{
    g_last_start_frame_text[0] = '\0';
    g_last_end_frame_text[0] = '\0';
    g_session_log_text[0] = '\0';
    g_session_log_len = 0;
}

void counting_history_append_frame(const char *tag,
                                   const uint8_t *buf,
                                   uint8_t len)
{
    char hex[COUNTING_HISTORY_HEX_BUFFER_SIZE];
    int written;

    if (tag == NULL || buf == NULL || len == 0U ||
        g_session_log_len >= sizeof(g_session_log_text) - 1U) {
        return;
    }

    counting_history_frame_to_hex(buf, len, hex, sizeof(hex));
    written = snprintf(g_session_log_text + g_session_log_len,
                       sizeof(g_session_log_text) - g_session_log_len,
                       "%s %s\n", tag, hex);
    if (written <= 0) {
        return;
    }

    g_session_log_len += (size_t)written;
    if (g_session_log_len >= sizeof(g_session_log_text)) {
        g_session_log_len = sizeof(g_session_log_text) - 1U;
        g_session_log_text[g_session_log_len] = '\0';
    }
}

void counting_history_session_start(const uint8_t *buf, uint8_t len)
{
    g_last_error_frame_text[0] = '\0';
    counting_history_session_reset();
    counting_history_frame_to_hex(buf, len,
                                  g_last_start_frame_text,
                                  sizeof(g_last_start_frame_text));
    counting_history_append_frame("0x0A", buf, len);
}

void counting_history_capture_error(const char *tag,
                                    const uint8_t *buf,
                                    uint8_t len)
{
    counting_history_frame_to_hex(buf, len,
                                  g_last_error_frame_text,
                                  sizeof(g_last_error_frame_text));
    counting_history_append_frame(tag, buf, len);
}

void counting_history_capture_end(const uint8_t *buf, uint8_t len)
{
    counting_history_append_frame("0x0E", buf, len);
    counting_history_frame_to_hex(buf, len,
                                  g_last_end_frame_text,
                                  sizeof(g_last_end_frame_text));
}

static bool counting_history_copy_sim_snapshot(counting_sim_t *dst,
                                               const counting_sim_t *src)
{
    int i;

    if (dst == NULL || src == NULL) {
        return false;
    }

    *dst = *src;
    dst->sn_str = NULL;
    dst->sn_capacity = 0;

    if (src->sn_str == NULL || src->sn_capacity <= 0) {
        return true;
    }

    dst->sn_str = calloc((size_t)src->sn_capacity, sizeof(char *));
    if (dst->sn_str == NULL) {
        return false;
    }

    dst->sn_capacity = src->sn_capacity;
    for (i = 0; i < src->sn_capacity; i++) {
        size_t text_len;

        if (src->sn_str[i] == NULL) {
            continue;
        }
        text_len = strlen(src->sn_str[i]);
        dst->sn_str[i] = malloc(text_len + 1U);
        if (dst->sn_str[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(dst->sn_str[j]);
            }
            free(dst->sn_str);
            dst->sn_str = NULL;
            dst->sn_capacity = 0;
            return false;
        }
        memcpy(dst->sn_str[i], src->sn_str[i], text_len + 1U);
    }

    return true;
}

static void counting_history_free_sim_snapshot(counting_sim_t *sim_data)
{
    if (sim_data == NULL || sim_data->sn_str == NULL) {
        return;
    }

    for (int i = 0; i < sim_data->sn_capacity; i++) {
        free(sim_data->sn_str[i]);
    }
    free(sim_data->sn_str);
    sim_data->sn_str = NULL;
    sim_data->sn_capacity = 0;
}

static void counting_history_clear_pending(counting_session_state_t *session)
{
    session->history_record.valid = false;
    session->history_record.end_seen = false;
    session->history_record.pcs = 0;
    session->history_record.total_after = 0;
    session->history_record.amount = 0.0f;
    g_last_error_frame_text[0] = '\0';
    counting_history_session_reset();
}

bool counting_history_try_commit(counting_session_state_t *session,
                                 const counting_sim_t *sim_data)
{
    counting_sim_t snapshot;
    bool appended;

    if (session == NULL || sim_data == NULL ||
        !session->history_record.valid || !session->history_record.end_seen) {
        return false;
    }
    if (!counting_history_copy_sim_snapshot(&snapshot, sim_data)) {
        return false;
    }

    snapshot.total_pcs = (int)session->history_record.pcs;
    snapshot.total_amount = session->history_record.amount;
    appended = ui_history_record_append_from_session(
        &snapshot,
        session->history_record.pcs,
        session->history_record.total_after,
        g_last_error_frame_text,
        g_last_start_frame_text,
        g_last_end_frame_text,
        g_session_log_text);
    counting_history_free_sim_snapshot(&snapshot);
    if (!appended) {
        return false;
    }

    counting_history_clear_pending(session);
    if (ui_manager_get_current_page() == UI_PAGE_HISTORY) {
        ui_page_19_history_refresh();
    }
    return true;
}
