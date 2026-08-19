#ifndef SENSOR_STATE_H
#define SENSOR_STATE_H

#include <stdbool.h>
#include <stdint.h>

#define SENSOR_VOLTAGE_CH_NUM 11

typedef struct {
    uint8_t raw[SENSOR_VOLTAGE_CH_NUM];
    bool valid[SENSOR_VOLTAGE_CH_NUM];
    uint32_t update_count;
} sensor_voltage_snapshot_t;

void sensor_state_clear(void);
void sensor_state_set_voltage(uint8_t channel, uint8_t raw);
void sensor_state_get_snapshot(sensor_voltage_snapshot_t *snapshot);

#endif
