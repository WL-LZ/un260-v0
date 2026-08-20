#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <stdbool.h>
#include <stdint.h>

#define SENSOR_VOLTAGE_CH_NUM 11

typedef struct {
    uint8_t raw[SENSOR_VOLTAGE_CH_NUM];
    bool valid[SENSOR_VOLTAGE_CH_NUM];
    uint32_t update_count;
} sensor_voltage_snapshot_t;

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

void sensor_state_clear(void);
void sensor_state_set_voltage(uint8_t channel, uint8_t raw);
void sensor_state_get_snapshot(sensor_voltage_snapshot_t *snapshot);

diagnostic_reply_result_t diagnostic_reply_dispatch(uint8_t cmd, const uint8_t *buf, uint8_t len, const diagnostic_reply_hooks_t *hooks);

#endif
