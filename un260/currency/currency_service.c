#include "currency_service.h"
#include <string.h>

typedef struct {
    bool pending;
    uint8_t target_index;
    char target_code[4];
    char previous_code[4];
    curr_item_t previous_currency;
    uint8_t previous_index;
} currency_switch_request_t;

static currency_switch_request_t g_currency_switch_request;

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
    g_currency_switch_request.pending = true;
    return true;
}

bool currency_service_take_switch_result(uint8_t status, currency_switch_result_t* result)
{
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

bool currency_service_switch_pending(void)
{
    return g_currency_switch_request.pending;
}
