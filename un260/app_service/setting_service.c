#include "un260/app_service/setting_service.h"
#include "un260/lv_drivers/lv_drivers.h"

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
