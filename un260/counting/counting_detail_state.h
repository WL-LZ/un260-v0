#ifndef COUNTING_DETAIL_STATE_H
#define COUNTING_DETAIL_STATE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool wait_sn_after_reject_end;
    bool query_pending;
    bool query_deferred;
    bool query_got_frame;
    uint32_t query_tick;
    uint8_t query_retry;
    uint32_t query_idle_retry_tick;
} counting_detail_state_t;

#endif
