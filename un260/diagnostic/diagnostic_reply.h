#ifndef DIAGNOSTIC_REPLY_H
#define DIAGNOSTIC_REPLY_H

#include <stdint.h>

typedef enum {
    DIAGNOSTIC_REPLY_INVALID = 0,
    DIAGNOSTIC_REPLY_IGNORED,
    DIAGNOSTIC_REPLY_SENSOR_START,
    DIAGNOSTIC_REPLY_SENSOR_DATA,
    DIAGNOSTIC_REPLY_SENSOR_END,
    DIAGNOSTIC_REPLY_CALIBRATION_UPDATED,
} diagnostic_reply_result_t;

typedef struct {
    void (*on_calibration_changed)(void);
} diagnostic_reply_hooks_t;

diagnostic_reply_result_t diagnostic_reply_dispatch(
    uint8_t cmd,
    const uint8_t *buf,
    uint8_t len,
    const diagnostic_reply_hooks_t *hooks);

#endif
