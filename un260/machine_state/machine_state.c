#include "machine_state.h"
#include "un260/lv_system/user_cfg.h"

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
