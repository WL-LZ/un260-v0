#include "machine_state.h"
#include "un260/lv_system/user_cfg.h"

static uint8_t g_batch_num = 0;
static bool g_batch_enabled = true;
static bool g_aging_running = false;
static uint8_t g_double_note_level = DOUBLE_NOTE_LEVEL_MIN;
static uint8_t g_flap_position = FLAP_POSITION_UP;
static uint8_t g_reject_pocket_max = REJECT_POCKET_MIN_CAPACITY;

void machine_state_confirm_mode(uint8_t mode)
{
    Machine_para.mode = mode;
}

uint8_t machine_state_mode(void)
{
    return Machine_para.mode;
}

void machine_state_confirm_buzzer(bool enabled)
{
    Machine_para.buzzer_enable = enabled;
}

bool machine_state_buzzer_enabled(void)
{
    return Machine_para.buzzer_enable;
}

void machine_state_confirm_add(bool enabled)
{
    Machine_para.add_enable = enabled;
}

bool machine_state_add_enabled(void)
{
    return Machine_para.add_enable;
}

void machine_state_confirm_fo_mode(uint8_t mode)
{
    Machine_para.fo_mode = mode;
}

uint8_t machine_state_fo_mode(void)
{
    return Machine_para.fo_mode;
}

void machine_state_confirm_speed(uint8_t speed)
{
    Machine_para.speed = speed;
}

uint8_t machine_state_speed(void)
{
    return Machine_para.speed;
}

void machine_state_confirm_work_mode(uint8_t mode)
{
    Machine_para.work_mode = mode;
}

uint8_t machine_state_work_mode(void)
{
    return Machine_para.work_mode;
}

void machine_state_confirm_batch(bool enabled, uint8_t num)
{
    g_batch_enabled = enabled;
    g_batch_num = num;
}

void machine_state_sync_batch_num(uint8_t num)
{
    g_batch_num = num;
}

void machine_state_confirm_batch_enable(bool enabled)
{
    g_batch_enabled = enabled;
}

bool machine_state_batch_enabled(void)
{
    return g_batch_enabled;
}

uint8_t machine_state_batch_num(void)
{
    return g_batch_num;
}

void machine_state_confirm_batch_amount(uint32_t amount)
{
    Machine_para.batch_amount = amount;
}

uint32_t machine_state_batch_amount(void)
{
    return Machine_para.batch_amount;
}

void machine_state_confirm_batch_mode(uint8_t mode)
{
    if (mode == PCS_BATCH_MODE || mode == AMOUNT_BATCH_MODE) {
        Machine_para.batch_mode = mode;
    }
}

uint8_t machine_state_batch_mode(void)
{
    return (uint8_t)Machine_para.batch_mode;
}

void machine_state_confirm_aging_running(bool running)
{
    g_aging_running = running;
}

bool machine_state_aging_running(void)
{
    return g_aging_running;
}

void machine_state_confirm_double_note_level(uint8_t level)
{
    g_double_note_level = level;
}

uint8_t machine_state_double_note_level(void)
{
    return g_double_note_level;
}

void machine_state_confirm_flap_position(uint8_t position)
{
    g_flap_position = position;
}

uint8_t machine_state_flap_position(void)
{
    return g_flap_position;
}

void machine_state_confirm_reject_pocket_max(uint8_t capacity)
{
    if (capacity < REJECT_POCKET_MIN_CAPACITY) {
        capacity = REJECT_POCKET_MIN_CAPACITY;
    } else if (capacity > REJECT_POCKET_MAX_CAPACITY) {
        capacity = REJECT_POCKET_MAX_CAPACITY;
    }
    g_reject_pocket_max = capacity;
}

uint8_t machine_state_reject_pocket_max(void)
{
    return g_reject_pocket_max;
}
