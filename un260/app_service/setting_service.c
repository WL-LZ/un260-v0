#include "un260/app_service/setting_service.h"
#include "un260/protocol/protocol_send.h"
#include "un260/protocol/mode_codec.h"
#include <stddef.h>

static bool g_mode_req_pending = false;
static uint8_t g_mode_req_target = 0;
static uint32_t g_mode_req_tick = 0;
static bool g_page01_add_req_pending = false;
static bool g_page01_add_req_target = false;
static uint32_t g_page01_add_req_tick = 0;
static bool g_page01_fo_req_pending = false;
static uint8_t g_page01_fo_req_target = 0;
static uint32_t g_page01_fo_req_tick = 0;
static bool g_page01_speed_req_pending = false;
static uint8_t g_page01_speed_req_target = 0;
static uint32_t g_page01_speed_req_tick = 0;
static bool g_page01_work_req_pending = false;
static uint8_t g_page01_work_req_target = 0;
static uint32_t g_page01_work_req_tick = 0;
static bool g_page03_beep_req_pending = false;
static bool g_page03_beep_req_target = false;
static uint32_t g_page03_beep_req_tick = 0;
static bool g_batch_req_pending = false;
static setting_batch_request_type_t g_batch_req_type = SETTING_BATCH_REQUEST_NONE;
static setting_batch_snapshot_t g_batch_req_target = { false, 0 };
static setting_batch_snapshot_t g_batch_req_previous = { false, 0 };

bool setting_service_request_mode(uint8_t target, uint32_t request_tick)
{
    uint8_t protocol_mode;

    if (g_mode_req_pending) return false;
    if (!mode_codec_encode(target, &protocol_mode)) return false;
    g_mode_req_pending = true;
    g_mode_req_target = target;
    g_mode_req_tick = request_tick;
    protocol_send(0x04, &protocol_mode, 1);
    return true;
}

bool setting_service_mode_is_pending(void)
{
    return g_mode_req_pending;
}

uint8_t setting_service_mode_target(void)
{
    return g_mode_req_target;
}

uint32_t setting_service_mode_tick(void)
{
    return g_mode_req_tick;
}

void setting_service_mode_finish(void)
{
    g_mode_req_pending = false;
}

bool setting_service_request_add(bool target, uint32_t request_tick)
{
    uint8_t add_cmd;

    if (g_page01_add_req_pending) return false;
    g_page01_add_req_pending = true;
    g_page01_add_req_target = target;
    g_page01_add_req_tick = request_tick;
    add_cmd = target ? 0x01 : 0x00;
    protocol_send(0x39, &add_cmd, 1);
    return true;
}

bool setting_service_add_is_pending(void)
{
    return g_page01_add_req_pending;
}

bool setting_service_add_target(void)
{
    return g_page01_add_req_target;
}

uint32_t setting_service_add_tick(void)
{
    return g_page01_add_req_tick;
}

void setting_service_add_finish(void)
{
    g_page01_add_req_pending = false;
}

bool setting_service_request_fo_mode(uint8_t target, uint32_t request_tick)
{
    uint8_t fo_cmd;

    if (g_page01_fo_req_pending) return false;
    g_page01_fo_req_pending = true;
    g_page01_fo_req_target = target;
    g_page01_fo_req_tick = request_tick;
    fo_cmd = target;
    protocol_send(0x3a, &fo_cmd, 1);
    return true;
}

bool setting_service_fo_mode_is_pending(void)
{
    return g_page01_fo_req_pending;
}

uint8_t setting_service_fo_mode_target(void)
{
    return g_page01_fo_req_target;
}

uint32_t setting_service_fo_mode_tick(void)
{
    return g_page01_fo_req_tick;
}

void setting_service_fo_mode_finish(void)
{
    g_page01_fo_req_pending = false;
}

bool setting_service_request_speed(uint8_t target, uint32_t request_tick)
{
    uint8_t speed_cmd = 0x01;

    if (g_page01_speed_req_pending) return false;
    g_page01_speed_req_pending = true;
    g_page01_speed_req_target = target;
    g_page01_speed_req_tick = request_tick;
    if (target == 0) {
        speed_cmd = 0x03;
    } else if (target == 1) {
        speed_cmd = 0x02;
    } else {
        speed_cmd = 0x01;
    }
    protocol_send(0x16, &speed_cmd, 1);
    return true;
}

bool setting_service_speed_is_pending(void)
{
    return g_page01_speed_req_pending;
}

uint8_t setting_service_speed_target(void)
{
    return g_page01_speed_req_target;
}

uint32_t setting_service_speed_tick(void)
{
    return g_page01_speed_req_tick;
}

void setting_service_speed_finish(void)
{
    g_page01_speed_req_pending = false;
}

bool setting_service_request_work_mode(uint8_t target, uint32_t request_tick)
{
    uint8_t work_cmd;

    if (g_page01_work_req_pending) return false;
    g_page01_work_req_pending = true;
    g_page01_work_req_target = target;
    g_page01_work_req_tick = request_tick;
    work_cmd = (target == 1) ? 0x00 : 0x01;
    protocol_send(0x38, &work_cmd, 1);
    return true;
}

bool setting_service_work_mode_is_pending(void)
{
    return g_page01_work_req_pending;
}

uint8_t setting_service_work_mode_target(void)
{
    return g_page01_work_req_target;
}

uint32_t setting_service_work_mode_tick(void)
{
    return g_page01_work_req_tick;
}

void setting_service_work_mode_finish(void)
{
    g_page01_work_req_pending = false;
}

bool setting_service_request_beep(bool target, uint32_t request_tick)
{
    uint8_t beep_cmd;

    if (g_page03_beep_req_pending) return false;
    g_page03_beep_req_pending = true;
    g_page03_beep_req_target = target;
    g_page03_beep_req_tick = request_tick;
    beep_cmd = target ? 0x01 : 0x02;
    protocol_send(0x15, &beep_cmd, 1);
    return true;
}

bool setting_service_beep_is_pending(void)
{
    return g_page03_beep_req_pending;
}

bool setting_service_beep_target(void)
{
    return g_page03_beep_req_target;
}

uint32_t setting_service_beep_tick(void)
{
    return g_page03_beep_req_tick;
}

void setting_service_beep_finish(void)
{
    g_page03_beep_req_pending = false;
}

bool setting_service_request_batch_number(uint8_t num, bool previous_enable, uint8_t previous_num)
{
    uint8_t batch_cmd = num;

    if (g_batch_req_pending) return false;
    g_batch_req_type = SETTING_BATCH_REQUEST_NUMBER;
    g_batch_req_target.enable = true;
    g_batch_req_target.num = num;
    g_batch_req_previous.enable = previous_enable;
    g_batch_req_previous.num = previous_num;
    g_batch_req_pending = true;
    protocol_send(0x06, &batch_cmd, 1);
    return true;
}

bool setting_service_request_batch_switch(bool target_enable, uint8_t sent_num, bool previous_enable, uint8_t previous_num)
{
    uint8_t batch_cmd = sent_num;

    if (g_batch_req_pending) return false;
    g_batch_req_type = SETTING_BATCH_REQUEST_SWITCH;
    g_batch_req_target.enable = target_enable;
    g_batch_req_target.num = sent_num;
    g_batch_req_previous.enable = previous_enable;
    g_batch_req_previous.num = previous_num;
    g_batch_req_pending = true;
    protocol_send(0x06, &batch_cmd, 1);
    return true;
}

bool setting_service_batch_take_result(uint8_t status, setting_batch_result_t *result)
{
    if (!g_batch_req_pending) return false;
    if (status != 0x01 && status != 0x02) return false;
    if (result == NULL) return false;

    result->type = g_batch_req_type;
    result->target = g_batch_req_target;
    result->previous = g_batch_req_previous;
    g_batch_req_pending = false;
    g_batch_req_type = SETTING_BATCH_REQUEST_NONE;
    return true;
}
