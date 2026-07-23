#include "serial_number_state.h"
#include "un260/lv_system/user_cfg.h"

static bool g_serial_number_enabled = false;
static uint8_t g_serial_number_level = SERIAL_NUMBER_LEVEL_OFF;

uint8_t serial_number_state_normalize_level(uint8_t level)
{
    if (level > SERIAL_NUMBER_LEVEL_MAX) return SERIAL_NUMBER_LEVEL_OFF;
    return level;
}

void serial_number_state_confirm(bool enabled, uint8_t level)
{
    g_serial_number_enabled = enabled;
    g_serial_number_level = level;
}

bool serial_number_state_enabled(void)
{
    return g_serial_number_enabled;
}

uint8_t serial_number_state_level(void)
{
    return g_serial_number_level;
}
