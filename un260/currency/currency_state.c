#include "currency_state.h"
#include <string.h>

static const char g_currency_default_codes[][4] = { "USD", "CNY", "EUR", "AED", "SAR", "OMR", "QAR", "MAD",
                                                      "EGP", "DZD", "INR", "PKR", "GBP", "IQD" };
static currency_state_snapshot_t g_currency_state = {
    .count = sizeof(g_currency_default_codes) / sizeof(g_currency_default_codes[0]),
    .codes = { "USD", "CNY", "EUR", "AED", "SAR", "OMR", "QAR", "MAD",
               "EGP", "DZD", "INR", "PKR", "GBP", "IQD" },
    .active_code = "CNY",
    .active_currency = CURR_CNY_ITEM,
    .active_index = 0,
};
static char g_currency_sync_codes[MAX_CURRENCIES][4];
static uint8_t g_currency_sync_count = 0;

static void currency_state_copy_code(char dst[4], const char* src)
{
    if (!dst) return;
    memset(dst, 0, 4);
    if (!src) return;
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}

void currency_state_reset(void)
{
    memset(&g_currency_state, 0, sizeof(g_currency_state));
    g_currency_state.count = sizeof(g_currency_default_codes) / sizeof(g_currency_default_codes[0]);
    memcpy(g_currency_state.codes, g_currency_default_codes, sizeof(g_currency_default_codes));
    currency_state_copy_code(g_currency_state.active_code, "CNY");
    g_currency_state.active_currency = CURR_CNY_ITEM;
    g_currency_state.active_index = 0;
    memset(g_currency_sync_codes, 0, sizeof(g_currency_sync_codes));
    g_currency_sync_count = 0;
}

void currency_state_begin_list_sync(void)
{
    memset(g_currency_sync_codes, 0, sizeof(g_currency_sync_codes));
    g_currency_sync_count = 0;
}

void currency_state_append_list_code(uint8_t protocol_index, const char code[4])
{
    uint8_t index;

    if (protocol_index == 0 || protocol_index > MAX_CURRENCIES) return;
    index = (uint8_t)(protocol_index - 1);
    currency_state_copy_code(g_currency_sync_codes[index], code);
    if (g_currency_sync_count < protocol_index) g_currency_sync_count = protocol_index;
}

void currency_state_finish_list_sync(void)
{
    memcpy(g_currency_state.codes, g_currency_sync_codes, sizeof(g_currency_state.codes));
    g_currency_state.count = g_currency_sync_count;
}

curr_item_t currency_state_code_to_item(const char* code)
{
    if (!code) return CURR_COUNT;
    if (strncmp(code, "CNY", 3) == 0) return CURR_CNY_ITEM;
    if (strncmp(code, "USD", 3) == 0) return CURR_USD_ITEM;
    if (strncmp(code, "EUR", 3) == 0) return CURR_EUR_ITEM;
    if (strncmp(code, "GBP", 3) == 0) return CURR_GBP_ITEM;
    if (strncmp(code, "KRW", 3) == 0) return CURR_KRW_ITEM;
    if (strncmp(code, "EGP", 3) == 0) return CURR_EGP_ITEM;
    if (strncmp(code, "ISK", 3) == 0) return CURR_ISK_ITEM;
    if (strncmp(code, "PHP", 3) == 0) return CURR_PHP_ITEM;
    if (strncmp(code, "SOS", 3) == 0) return CURR_SOS_ITEM;
    if (strncmp(code, "TRY", 3) == 0) return CURR_TRY_ITEM;
    if (strncmp(code, "AED", 3) == 0) return CURR_AED_ITEM;
    if (strncmp(code, "SAR", 3) == 0) return CURR_SAR_ITEM;
    if (strncmp(code, "OMR", 3) == 0) return CURR_OMR_ITEM;
    if (strncmp(code, "QAR", 3) == 0) return CURR_QAR_ITEM;
    if (strncmp(code, "MAD", 3) == 0) return CURR_MAD_ITEM;
    if (strncmp(code, "DZD", 3) == 0) return CURR_DZD_ITEM;
    if (strncmp(code, "INR", 3) == 0) return CURR_INR_ITEM;
    if (strncmp(code, "PKR", 3) == 0) return CURR_PKR_ITEM;
    if (strncmp(code, "IQD", 3) == 0) return CURR_IQD_ITEM;
    return CURR_COUNT;
}

void currency_state_confirm_active_code(const char* code)
{
    if (!code) return;
    currency_state_copy_code(g_currency_state.active_code, code);
}

void currency_state_confirm_active_currency(curr_item_t currency)
{
    if (currency >= CURR_COUNT) return;
    g_currency_state.active_currency = currency;
}

void currency_state_confirm_active_index(uint8_t index)
{
    g_currency_state.active_index = index;
}

void currency_state_get_snapshot(currency_state_snapshot_t* snapshot)
{
    if (!snapshot) return;
    *snapshot = g_currency_state;
}

uint8_t currency_state_count(void)
{
    return g_currency_state.count;
}

bool currency_state_get_code(uint8_t index, char code[4])
{
    if (!code || index >= g_currency_state.count || index >= MAX_CURRENCIES) return false;
    currency_state_copy_code(code, g_currency_state.codes[index]);
    return true;
}

bool currency_state_find_code(const char* code, uint8_t* index)
{
    uint8_t i;

    if (!code) return false;
    for (i = 0; i < g_currency_state.count; i++) {
        if (strncmp(code, g_currency_state.codes[i], 3) == 0) {
            if (index) *index = i;
            return true;
        }
    }
    return false;
}

void currency_state_get_active_code(char code[4])
{
    currency_state_copy_code(code, g_currency_state.active_code);
}

curr_item_t currency_state_active_currency(void)
{
    return g_currency_state.active_currency;
}

uint8_t currency_state_active_index(void)
{
    return g_currency_state.active_index;
}
