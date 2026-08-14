#ifndef COUNTING_DENOM_QUERY_SERVICE_H
#define COUNTING_DENOM_QUERY_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "counting_detail_state.h"

void counting_denom_query_invalidate(counting_detail_state_t *detail);
void counting_denom_query_mark_frame_received(counting_detail_state_t *detail);
bool counting_denom_query_complete(counting_detail_state_t *detail);
void counting_denom_query_trigger(counting_detail_state_t *detail,
                                  uint32_t now_ms,
                                  bool boot_ready);
void counting_denom_query_poll(counting_detail_state_t *detail,
                               uint32_t now_ms,
                               bool boot_ready,
                               bool main_page_active);

#endif
