#include "un260/boot/boot_service.h"
#include "un260/protocol/protocol_send.h"
#include <stddef.h>

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

void boot_service_reset_handshake(void)
{
    g_handshake_state = HANDSHAKE_IDLE;
    g_handshake_tick = 0;
    g_handshake_start_tick = 0;
}

bool boot_service_request_handshake(uint32_t request_tick)
{
    uint8_t payload = 0x01;

    if (g_handshake_state == HANDSHAKE_IDLE) {
        g_handshake_start_tick = request_tick;
    }

    protocol_send(0x01, &payload, 1);
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

bool boot_service_request_next_self_test(void)
{
    selftest_type_t step;
    uint8_t protocol_step;

    if (g_self_test_sequence_index >= sizeof(g_self_test_sequence) / sizeof(g_self_test_sequence[0])) return false;
    step = g_self_test_sequence[g_self_test_sequence_index];
    if (!boot_self_test_step_to_protocol(step, &protocol_step)) return false;
    protocol_send(0x37, &protocol_step, 1);
    g_self_test_sequence_index++;
    return true;
}

uint8_t boot_service_self_test_sequence_index(void)
{
    return g_self_test_sequence_index;
}
