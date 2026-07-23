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
typedef struct {
    bool received;
    uint8_t result;
} boot_self_test_result_t;
static boot_self_test_result_t g_self_test_results[sizeof(g_self_test_sequence) / sizeof(g_self_test_sequence[0])];
static bool g_self_test_has_failure = false;
static uint8_t g_self_test_first_failure_step = 0;
static uint8_t g_self_test_first_failure_result = 0;

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

static bool boot_self_test_protocol_step_to_index(uint8_t protocol_step, uint8_t *index)
{
    if (index == NULL) return false;

    switch (protocol_step) {
    case SELFTEST_CONFIG:
        *index = 0;
        return true;
    case SELFTEST_SENSOR:
        *index = 1;
        return true;
    case SELFTEST_MOTOR:
        *index = 2;
        return true;
    case SELFTEST_MAGNET:
        *index = 3;
        return true;
    case SELFTEST_IMAGE:
        *index = 4;
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

void boot_service_reset_self_test_results(void)
{
    for (uint8_t i = 0; i < sizeof(g_self_test_results) / sizeof(g_self_test_results[0]); i++) {
        g_self_test_results[i].received = false;
        g_self_test_results[i].result = 0;
    }
    g_self_test_has_failure = false;
    g_self_test_first_failure_step = 0;
    g_self_test_first_failure_result = 0;
}

bool boot_service_record_self_test_result(uint8_t protocol_step, uint8_t result, uint8_t *index)
{
    uint8_t item_index;
    bool valid_step = boot_self_test_protocol_step_to_index(protocol_step, &item_index);

    if (valid_step) {
        g_self_test_results[item_index].received = true;
        g_self_test_results[item_index].result = result;
        if (index != NULL) {
            *index = item_index;
        }
    }

    if (result != 0x01 && !g_self_test_has_failure) {
        g_self_test_has_failure = true;
        g_self_test_first_failure_step = protocol_step;
        g_self_test_first_failure_result = result;
    }

    return valid_step;
}

bool boot_service_self_test_all_received(void)
{
    for (uint8_t i = 0; i < sizeof(g_self_test_results) / sizeof(g_self_test_results[0]); i++) {
        if (!g_self_test_results[i].received) return false;
    }
    return true;
}

bool boot_service_self_test_has_failure(void)
{
    return g_self_test_has_failure;
}

bool boot_service_get_first_self_test_failure(uint8_t *protocol_step, uint8_t *result)
{
    if (!g_self_test_has_failure || protocol_step == NULL || result == NULL) return false;

    *protocol_step = g_self_test_first_failure_step;
    *result = g_self_test_first_failure_result;
    return true;
}

bool boot_service_get_self_test_result(uint8_t protocol_step, uint8_t *result)
{
    uint8_t index;

    if (result == NULL || !boot_self_test_protocol_step_to_index(protocol_step, &index)) return false;
    if (!g_self_test_results[index].received) return false;

    *result = g_self_test_results[index].result;
    return true;
}
