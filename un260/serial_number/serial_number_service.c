#include "serial_number_service.h"
#include "un260/serial_number/serial_number_state.h"
#include "un260/protocol/protocol_request.h"

#include <stddef.h>

#define SERIAL_NUMBER_REQUEST_TIMEOUT_MS 800U

static protocol_request_t g_serial_number_request = PROTOCOL_REQUEST_INITIALIZER(SERIAL_NUMBER_REQUEST_TIMEOUT_MS);
static bool g_serial_number_request_target_enabled = false;
static uint8_t g_serial_number_request_target_level = 0;
static bool g_serial_number_request_previous_enabled = false;
static uint8_t g_serial_number_request_previous_level = 0;

bool serial_number_service_request(bool target_enabled, uint8_t target_level)
{
    if (!protocol_request_begin(&g_serial_number_request)) return false;

    g_serial_number_request_target_enabled = target_enabled;
    g_serial_number_request_target_level = target_level;
    g_serial_number_request_previous_enabled = serial_number_state_enabled();
    g_serial_number_request_previous_level = serial_number_state_level();
    return true;
}

void serial_number_service_cancel_request(void)
{
    protocol_request_finish(&g_serial_number_request);
}

bool serial_number_service_take_reply(uint8_t level, uint8_t status, serial_number_setting_result_t *result)
{
    if (!protocol_request_can_take_result(&g_serial_number_request) || result == NULL) return false;
    if (status != 0x01 && status != 0x02) return false;
    if (status == 0x01 && level != g_serial_number_request_target_level) return false;

    result->success = (status == 0x01);
    result->target_enabled = g_serial_number_request_target_enabled;
    result->target_level = g_serial_number_request_target_level;
    result->previous_enabled = g_serial_number_request_previous_enabled;
    result->previous_level = g_serial_number_request_previous_level;
    result->response_level = level;
    protocol_request_finish(&g_serial_number_request);
    return true;
}

bool serial_number_service_take_timeout(serial_number_setting_result_t *result)
{
    if (!result || !protocol_request_take_timeout(&g_serial_number_request)) return false;

    result->success = false;
    result->target_enabled = g_serial_number_request_target_enabled;
    result->target_level = g_serial_number_request_target_level;
    result->previous_enabled = g_serial_number_request_previous_enabled;
    result->previous_level = g_serial_number_request_previous_level;
    result->response_level = g_serial_number_request_target_level;
    return true;
}
