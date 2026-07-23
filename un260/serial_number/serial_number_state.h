#ifndef SERIAL_NUMBER_STATE_H
#define SERIAL_NUMBER_STATE_H

#include <stdbool.h>
#include <stdint.h>

uint8_t serial_number_state_normalize_level(uint8_t level);
void serial_number_state_confirm(bool enabled, uint8_t level);
bool serial_number_state_enabled(void);
uint8_t serial_number_state_level(void);

#endif
