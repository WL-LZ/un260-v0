#ifndef COUNTING_HISTORY_SERVICE_H
#define COUNTING_HISTORY_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "counting_data_types.h"
#include "counting_session_state.h"

void counting_history_session_start(const uint8_t *buf, uint8_t len);
void counting_history_capture_error(const char *tag,
                                    const uint8_t *buf,
                                    uint8_t len);
void counting_history_append_frame(const char *tag,
                                   const uint8_t *buf,
                                   uint8_t len);
void counting_history_capture_end(const uint8_t *buf, uint8_t len);
bool counting_history_try_commit(counting_session_state_t *session,
                                 const counting_sim_t *sim_data);

#endif
