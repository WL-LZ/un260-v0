#include "un260/boot/boot_service.h"
#include "un260/protocol/protocol_send.h"
#include <stddef.h>

static handshake_state_t g_handshake_state = HANDSHAKE_IDLE;
static uint32_t g_handshake_tick = 0;
static uint32_t g_handshake_start_tick = 0;
static bool g_self_test_pending = false;
static selftest_type_t g_self_test_step = SELFTEST_NONE;
static uint32_t g_self_test_tick = 0;

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
    g_self_test_pending = false;
    g_self_test_step = SELFTEST_NONE;
    g_self_test_tick = 0;
}

bool boot_service_request_self_test(selftest_type_t step, uint32_t request_tick)
{
    uint8_t protocol_step;

    if (!boot_self_test_step_to_protocol(step, &protocol_step)) return false;
    g_self_test_pending = true;
    g_self_test_step = step;
    g_self_test_tick = request_tick;
    protocol_send(0x37, &protocol_step, 1);
    return true;
}

bool boot_service_self_test_is_pending(void)
{
    return g_self_test_pending;
}

selftest_type_t boot_service_self_test_step(void)
{
    return g_self_test_step;
}

uint32_t boot_service_self_test_tick(void)
{
    return g_self_test_tick;
}

uint8_t boot_service_self_test_sequence_index(void)
{
    switch (g_self_test_step) {
    case SELFTEST_CONFIG:
        return 1;
    case SELFTEST_SENSOR:
        return 2;
    case SELFTEST_MOTOR:
        return 3;
    case SELFTEST_MAGNET:
        return 4;
    case SELFTEST_IMAGE:
        return 5;
    default:
        return 0;
    }
}

void boot_service_finish_self_test(void)
{
    g_self_test_pending = false;
}
