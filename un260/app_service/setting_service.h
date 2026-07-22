#ifndef SETTING_SERVICE_H
#define SETTING_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

bool setting_service_request_mode(uint8_t target, uint32_t request_tick);
bool setting_service_mode_is_pending(void);
uint8_t setting_service_mode_target(void);
uint32_t setting_service_mode_tick(void);
void setting_service_mode_finish(void);

bool setting_service_request_add(bool target, uint32_t request_tick);
bool setting_service_add_is_pending(void);
bool setting_service_add_target(void);
uint32_t setting_service_add_tick(void);
void setting_service_add_finish(void);

bool setting_service_request_fo_mode(uint8_t target, uint32_t request_tick);
bool setting_service_fo_mode_is_pending(void);
uint8_t setting_service_fo_mode_target(void);
uint32_t setting_service_fo_mode_tick(void);
void setting_service_fo_mode_finish(void);

bool setting_service_request_speed(uint8_t target, uint32_t request_tick);
bool setting_service_speed_is_pending(void);
uint8_t setting_service_speed_target(void);
uint32_t setting_service_speed_tick(void);
void setting_service_speed_finish(void);

bool setting_service_request_work_mode(uint8_t target, uint32_t request_tick);
bool setting_service_work_mode_is_pending(void);
uint8_t setting_service_work_mode_target(void);
uint32_t setting_service_work_mode_tick(void);
void setting_service_work_mode_finish(void);

bool setting_service_request_beep(bool target, uint32_t request_tick);
bool setting_service_beep_is_pending(void);
bool setting_service_beep_target(void);
uint32_t setting_service_beep_tick(void);
void setting_service_beep_finish(void);

#endif
