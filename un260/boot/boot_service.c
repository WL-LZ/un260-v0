#include "un260/boot/boot_service.h"
#include "un260/protocol/protocol_send.h"

static handshake_state_t g_handshake_state = HANDSHAKE_IDLE;
static uint32_t g_handshake_tick = 0;
static uint32_t g_handshake_start_tick = 0;

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
