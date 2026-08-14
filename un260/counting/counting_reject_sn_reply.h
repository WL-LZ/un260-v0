#ifndef COUNTING_REJECT_SN_REPLY_H
#define COUNTING_REJECT_SN_REPLY_H

#include <stdbool.h>
#include <stdint.h>

#include "counting_detail_state.h"
#include "counting_session_state.h"
#include "un260/lv_system/platform_app.h"

typedef enum {
    COUNTING_DETAIL_REPLY_INVALID = 0,
    COUNTING_DETAIL_REPLY_IGNORED,
    COUNTING_DETAIL_REPLY_START,
    COUNTING_DETAIL_REPLY_DATA,
    COUNTING_DETAIL_REPLY_END,
    COUNTING_DETAIL_REPLY_MEMORY_ERROR,
} counting_detail_reply_result_t;

typedef struct {
    void (*on_history_frame)(const char *tag, const uint8_t *buf, uint8_t len);
    void (*on_reject_analysis_ready)(void);
    void (*on_history_record_ready)(void);
    void (*on_detail_complete)(void);
    bool (*is_main_page_active)(void);
} counting_reject_sn_reply_hooks_t;

counting_detail_reply_result_t counting_reject_sn_reply_dispatch(
    uint8_t cmd,
    counting_detail_state_t *detail,
    counting_session_state_t *session,
    counting_sim_t *sim_data,
    const uint8_t *buf,
    uint8_t len,
    const counting_reject_sn_reply_hooks_t *hooks);

#endif
