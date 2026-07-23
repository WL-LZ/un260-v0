#include "machine_state.h"
#include "un260/lv_system/user_cfg.h"

static uint8_t g_batch_num = 0;
static bool g_batch_enabled = true;
static bool g_aging_running = false;

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

void machine_state_confirm_aging_running(bool running)
{
    g_aging_running = running;
}

bool machine_state_aging_running(void)
{
    return g_aging_running;
}
