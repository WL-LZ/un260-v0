#ifndef SETTING_SERVICE_H
#define SETTING_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SETTING_BATCH_REQUEST_NONE = 0,
    SETTING_BATCH_REQUEST_NUMBER,
    SETTING_BATCH_REQUEST_SWITCH,
} setting_batch_request_type_t;

typedef struct {
    bool enable;
    uint8_t num;
} setting_batch_snapshot_t;

typedef struct {
    setting_batch_request_type_t type;
    setting_batch_snapshot_t target;
    setting_batch_snapshot_t previous;
} setting_batch_result_t;

typedef struct {
    uint8_t target;
    uint8_t previous;
    bool success;
    bool timeout;
} setting_value_result_t;

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

bool setting_service_request_batch_number(uint8_t num, bool previous_enable, uint8_t previous_num);
bool setting_service_request_batch_switch(bool target_enable, uint8_t sent_num, bool previous_enable, uint8_t previous_num);
bool setting_service_batch_take_result(uint8_t status, setting_batch_result_t *result);
bool setting_service_batch_take_timeout(setting_batch_result_t *result);

bool setting_service_request_double_note_level(uint8_t target, uint8_t previous);
bool setting_service_take_double_note_level_result(uint8_t status, setting_value_result_t *result);
bool setting_service_take_double_note_level_timeout(setting_value_result_t *result);
void setting_service_clear_double_note_level_request(void);

bool setting_service_request_flap_position(uint8_t target, uint8_t previous);
bool setting_service_take_flap_position_result(uint8_t status, setting_value_result_t *result);
bool setting_service_take_flap_position_timeout(setting_value_result_t *result);

bool setting_service_request_reject_pocket_max(uint8_t target, uint8_t previous);
bool setting_service_take_reject_pocket_max_result(uint8_t status, setting_value_result_t *result);
bool setting_service_take_reject_pocket_max_timeout(setting_value_result_t *result);
void setting_service_clear_reject_pocket_max_request(void);

#endif
