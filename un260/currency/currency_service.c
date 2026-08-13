#include "currency_service.h"
#include <string.h>
#include <time.h>

#define CURRENCY_SWITCH_TIMEOUT_MS 800U

typedef struct {
    bool pending;
    uint8_t target_index;
    char target_code[4];
    char previous_code[4];
    curr_item_t previous_currency;
    uint8_t previous_index;
    uint64_t request_tick_ms;
} currency_switch_request_t;

static currency_switch_request_t g_currency_switch_request;

static uint64_t currency_service_now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

static bool currency_service_request_expired(void)
{
    return g_currency_switch_request.pending &&
           currency_service_now_ms() - g_currency_switch_request.request_tick_ms >= CURRENCY_SWITCH_TIMEOUT_MS;
}

static void currency_service_copy_code(char dst[4], const char src[4])
{
    memset(dst, 0, 4);
    if (!src) return;
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}

bool currency_service_request_switch(uint8_t target_index, const char target_code[4])
{
    if (g_currency_switch_request.pending || !target_code || target_code[0] == '\0') return false;

    memset(&g_currency_switch_request, 0, sizeof(g_currency_switch_request));
    g_currency_switch_request.target_index = target_index;
    currency_service_copy_code(g_currency_switch_request.target_code, target_code);
    currency_state_get_active_code(g_currency_switch_request.previous_code);
    g_currency_switch_request.previous_currency = currency_state_active_currency();
    g_currency_switch_request.previous_index = currency_state_active_index();
    g_currency_switch_request.request_tick_ms = currency_service_now_ms();
    g_currency_switch_request.pending = true;
    return true;
}

bool currency_service_take_switch_result(uint8_t status, currency_switch_result_t* result)
{
    if (currency_service_request_expired()) return false;
    if (!g_currency_switch_request.pending || !result) return false;
    if (status != 0x01 && status != 0x02) return false;

    memset(result, 0, sizeof(*result));
    result->success = (status == 0x01);
    result->target_index = g_currency_switch_request.target_index;
    currency_service_copy_code(result->target_code, g_currency_switch_request.target_code);
    currency_service_copy_code(result->previous_code, g_currency_switch_request.previous_code);
    result->previous_currency = g_currency_switch_request.previous_currency;
    result->previous_index = g_currency_switch_request.previous_index;
    memset(&g_currency_switch_request, 0, sizeof(g_currency_switch_request));
    return true;
}

bool currency_service_take_switch_timeout(currency_switch_result_t* result)
{
    if (!result || !currency_service_request_expired()) return false;

    memset(result, 0, sizeof(*result));
    result->success = false;
    result->target_index = g_currency_switch_request.target_index;
    currency_service_copy_code(result->target_code, g_currency_switch_request.target_code);
    currency_service_copy_code(result->previous_code, g_currency_switch_request.previous_code);
    result->previous_currency = g_currency_switch_request.previous_currency;
    result->previous_index = g_currency_switch_request.previous_index;
    memset(&g_currency_switch_request, 0, sizeof(g_currency_switch_request));
    return true;
}

bool currency_service_switch_pending(void)
{
    return g_currency_switch_request.pending;
}
