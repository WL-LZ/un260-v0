#ifndef BOOT_SERVICE_H
#define BOOT_SERVICE_H

#include <stdbool.h>
#include <stdint.h>
#include "un260/boot/boot_state.h"

void boot_service_reset_handshake(void);
bool boot_service_request_handshake(uint32_t request_tick);
void boot_service_confirm_handshake(void);
handshake_state_t boot_service_handshake_state(void);
uint32_t boot_service_handshake_tick(void);
uint32_t boot_service_handshake_start_tick(void);
void boot_service_reset_self_test(void);
bool boot_service_request_self_test(selftest_type_t step, uint32_t request_tick);
bool boot_service_self_test_is_pending(void);
selftest_type_t boot_service_self_test_step(void);
uint32_t boot_service_self_test_tick(void);
uint8_t boot_service_self_test_sequence_index(void);
void boot_service_finish_self_test(void);

#endif
