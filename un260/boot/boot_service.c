#include "un260/boot/boot_service.h"
#include <stddef.h>

#define BOOT_SERVICE_TOTAL_TIMEOUT_MS 60000

static boot_stage_t g_stage = BOOT_STAGE_HANDSHAKE;
static uint32_t g_boot_start_tick = 0;
static bool g_boot_started = false;
static bool g_boot_timeout_consumed = false;
static handshake_state_t g_handshake_state = HANDSHAKE_IDLE;
static uint32_t g_handshake_tick = 0;
static uint32_t g_handshake_start_tick = 0;
static const selftest_type_t g_self_test_sequence[] = {
    SELFTEST_CONFIG,
    SELFTEST_SENSOR,
    SELFTEST_MOTOR,
    SELFTEST_MAGNET,
    SELFTEST_IMAGE,
};
static uint8_t g_self_test_sequence_index = 0;

static bool boot_self_test_step_to_protocol(selftest_type_t step, uint8_t *protocol_step)
{
    if (protocol_step == NULL) return false;

    switch (step) {
    case SELFTEST_SENSOR:
        *protocol_step = 0x01;
        return true;
    case SELFTEST_MOTOR:
        *protocol_step = 0x02;
        return true;
    case SELFTEST_MAGNET:
        *protocol_step = 0x03;
        return true;
    case SELFTEST_CONFIG:
        *protocol_step = 0x04;
        return true;
    case SELFTEST_IMAGE:
        *protocol_step = 0x05;
        return true;
    default:
        return false;
    }
}

void boot_service_start(uint32_t now_ms)
{
    if (g_boot_started) return;

    g_boot_started = true;
    g_boot_start_tick = now_ms;
    g_boot_timeout_consumed = false;
}

void boot_service_set_stage(boot_stage_t stage)
{
    g_stage = stage;
}

boot_stage_t boot_service_get_stage(void)
{
    return g_stage;
}

void boot_service_advance_stage(void)
{
    g_stage++;
}

bool boot_service_check_total_timeout(uint32_t now_ms)
{
    if (!g_boot_started || g_boot_timeout_consumed) return false;
    if ((uint32_t)(now_ms - g_boot_start_tick) < BOOT_SERVICE_TOTAL_TIMEOUT_MS) return false;

    g_boot_timeout_consumed = true;
    return true;
}

void boot_service_reset_handshake(void)
{
    g_handshake_state = HANDSHAKE_IDLE;
    g_handshake_tick = 0;
    g_handshake_start_tick = 0;
}

bool boot_service_request_handshake(uint32_t request_tick)
{
    if (g_handshake_state == HANDSHAKE_IDLE) {
        g_handshake_start_tick = request_tick;
    }

    g_handshake_state = HANDSHAKE_SENT;
    g_handshake_tick = request_tick;
    return true;
}

void boot_service_confirm_handshake(void)
{
    g_handshake_state = HANDSHAKE_OK;
    g_handshake_start_tick = 0;
}

handshake_state_t boot_service_handshake_state(void)
{
    return g_handshake_state;
}

uint32_t boot_service_handshake_tick(void)
{
    return g_handshake_tick;
}

uint32_t boot_service_handshake_start_tick(void)
{
    return g_handshake_start_tick;
}

void boot_service_reset_self_test(void)
{
    g_self_test_sequence_index = 0;
}

bool boot_service_next_self_test_protocol_step(uint8_t *protocol_step)
{
    selftest_type_t step;

    if (protocol_step == NULL) return false;
    if (g_self_test_sequence_index >= sizeof(g_self_test_sequence) / sizeof(g_self_test_sequence[0])) return false;
    step = g_self_test_sequence[g_self_test_sequence_index];
    if (!boot_self_test_step_to_protocol(step, protocol_step)) return false;
    g_self_test_sequence_index++;
    return true;
}

uint8_t boot_service_self_test_sequence_index(void)
{
    return g_self_test_sequence_index;
}
