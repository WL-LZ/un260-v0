#include "diagnostic.h"

#include <stddef.h>
#include <string.h>

static sensor_voltage_snapshot_t g_sensor_voltage;
cis_calib_state_t cis_state = CIS_CALIB_IDLE;
cb_calib_state_t cb_state = CB_CALIB_IDLE;
calib_target_t g_calib_target = CALIB_TARGET_CIS;
uint8_t g_cb_running = 0;

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

static int diagnostic_sensor_index_to_channel(uint8_t index)
{
    if (index >= 0x01 && index <= SENSOR_VOLTAGE_CH_NUM) return (int)index - 1;
    return -1;
}

static diagnostic_reply_result_t diagnostic_sensor_reply_handle(const uint8_t *buf, uint8_t len)
{
    uint8_t index;
    uint8_t value;
    int channel;

    if (buf == NULL || len < 7) return DIAGNOSTIC_REPLY_INVALID;

    index = buf[4];
    value = buf[5];
    if (index == 0x00 && value == 0x00) {
        sensor_state_clear();
        return DIAGNOSTIC_REPLY_SENSOR_START;
    }
    if (index == 0xFF && value == 0xFF) return DIAGNOSTIC_REPLY_SENSOR_END;

    channel = diagnostic_sensor_index_to_channel(index);
    if (channel < 0) return DIAGNOSTIC_REPLY_IGNORED;

    sensor_state_set_voltage(channel, value);
    return DIAGNOSTIC_REPLY_SENSOR_DATA;
}

static bool diagnostic_cis_state_decode(uint8_t raw, cis_calib_state_t *state)
{
    if (state == NULL) return false;

    switch (raw) {
    case 0x01: *state = CIS_CALIB_RUNNING; return true;
    case 0x02: *state = CIS_CALIB_SUCCESS; return true;
    case 0x03: *state = CIS_CALIB_FAIL_UPPER; return true;
    case 0x04: *state = CIS_CALIB_FAIL_LOWER; return true;
    case 0x05: *state = CIS_CALIB_FAIL_IR; return true;
    default: return false;
    }
}

static bool diagnostic_cb_state_decode(uint8_t raw, cb_calib_state_t *state)
{
    if (state == NULL) return false;

    switch (raw) {
    case 0x01: *state = CB_CALIB_RUNNING; return true;
    case 0x02: *state = CB_CALIB_SUCCESS; return true;
    case 0x05: *state = CB_CALIB_FAIL_IR; return true;
    default: return false;
    }
}

static diagnostic_reply_result_t diagnostic_calibration_reply_handle(const uint8_t *buf, uint8_t len, const diagnostic_reply_hooks_t *hooks)
{
    bool updated;

    /* 状态字节之后还应有校验字节。 */
    if (buf == NULL || len < 6) return DIAGNOSTIC_REPLY_INVALID;

    if (g_calib_target == CALIB_TARGET_CB) {
        updated = diagnostic_cb_state_decode(buf[4], &cb_state);
    } else {
        updated = diagnostic_cis_state_decode(buf[4], &cis_state);
    }
    if (!updated) return DIAGNOSTIC_REPLY_IGNORED;

    if (hooks != NULL && hooks->on_calibration_changed != NULL) hooks->on_calibration_changed();
    return DIAGNOSTIC_REPLY_CALIBRATION_UPDATED;
}

diagnostic_reply_result_t diagnostic_reply_dispatch(uint8_t cmd, const uint8_t *buf, uint8_t len, const diagnostic_reply_hooks_t *hooks)
{
    switch (cmd) {
    case 0x1D:
        return diagnostic_sensor_reply_handle(buf, len);
    case 0x5B:
        return diagnostic_calibration_reply_handle(buf, len, hooks);
    default:
        return DIAGNOSTIC_REPLY_INVALID;
    }
}
