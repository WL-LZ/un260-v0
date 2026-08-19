#include "un260/diagnostic/sensor_state.h"

#include <string.h>

static sensor_voltage_snapshot_t g_sensor_voltage;

void sensor_state_clear(void)
{
    memset(g_sensor_voltage.valid, 0, sizeof(g_sensor_voltage.valid));
    g_sensor_voltage.update_count++;
}

void sensor_state_set_voltage(uint8_t channel, uint8_t raw)
{
    if (channel >= SENSOR_VOLTAGE_CH_NUM) return;
    g_sensor_voltage.raw[channel] = raw;
    g_sensor_voltage.valid[channel] = true;
    g_sensor_voltage.update_count++;
}

void sensor_state_get_snapshot(sensor_voltage_snapshot_t *snapshot)
{
    if (snapshot != NULL) *snapshot = g_sensor_voltage;
}
