#ifndef __MACHINE_STATE_H__
#define __MACHINE_STATE_H__

#include <stdbool.h>
#include <stdint.h>

void machine_state_confirm_buzzer(bool enabled);
bool machine_state_buzzer_enabled(void);

void machine_state_confirm_add(bool enabled);
bool machine_state_add_enabled(void);

void machine_state_confirm_fo_mode(uint8_t mode);
uint8_t machine_state_fo_mode(void);

void machine_state_confirm_speed(uint8_t speed);
uint8_t machine_state_speed(void);

void machine_state_confirm_work_mode(uint8_t mode);
uint8_t machine_state_work_mode(void);

#endif
