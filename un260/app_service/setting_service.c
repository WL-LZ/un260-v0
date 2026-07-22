#include "un260/app_service/setting_service.h"
#include "un260/protocol/protocol_send.h"
#include "un260/protocol/mode_codec.h"
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#ifndef SETTING_BATCH_TRACE_ENABLE
#define SETTING_BATCH_TRACE_ENABLE 1
#endif

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

#if SETTING_BATCH_TRACE_ENABLE
static uint32_t g_batch_trace_next_seq = 0;
static uint32_t g_batch_trace_active_seq = 0;
static uint64_t g_batch_trace_tx_ms = 0;

static uint64_t setting_batch_trace_now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

static const char *setting_batch_trace_source(setting_batch_request_type_t type)
{
    if (type == SETTING_BATCH_REQUEST_NUMBER) return "NUMBER";
    if (type == SETTING_BATCH_REQUEST_SWITCH) return "SWITCH";
    return "NONE";
}

static const char *setting_batch_trace_status(uint8_t status)
{
    return status == 0x01 ? "SUCCESS" : "FAIL";
}

static void setting_batch_trace_tx(void)
{
    g_batch_trace_active_seq = ++g_batch_trace_next_seq;
    g_batch_trace_tx_ms = setting_batch_trace_now_ms();
    printf("[BATCH_TRACE] TX seq=%u time_ms=%llu source=%s payload=%u target_enable=%u target_num=%u previous_enable=%u previous_num=%u\n",
           g_batch_trace_active_seq, (unsigned long long)g_batch_trace_tx_ms,
           setting_batch_trace_source(g_batch_req_type), (unsigned)g_batch_req_target.num,
           (unsigned)g_batch_req_target.enable, (unsigned)g_batch_req_target.num,
           (unsigned)g_batch_req_previous.enable, (unsigned)g_batch_req_previous.num);
}

static void setting_batch_trace_reject(setting_batch_request_type_t source, uint8_t payload)
{
    uint64_t now_ms = setting_batch_trace_now_ms();

    printf("[BATCH_TRACE] REJECT time_ms=%llu source=%s payload=%u active_seq=%u active_source=%s active_target_enable=%u active_target_num=%u\n",
           (unsigned long long)now_ms, setting_batch_trace_source(source), (unsigned)payload,
           g_batch_trace_active_seq, setting_batch_trace_source(g_batch_req_type),
           (unsigned)g_batch_req_target.enable, (unsigned)g_batch_req_target.num);
}

static void setting_batch_trace_rx(uint8_t status)
{
    uint64_t now_ms = setting_batch_trace_now_ms();
    uint64_t latency_ms = now_ms - g_batch_trace_tx_ms;

    printf("[BATCH_TRACE] RX seq=%u time_ms=%llu latency_ms=%llu status=%s source=%s payload=%u target_enable=%u target_num=%u previous_enable=%u previous_num=%u\n",
           g_batch_trace_active_seq, (unsigned long long)now_ms, (unsigned long long)latency_ms,
           setting_batch_trace_status(status),
           setting_batch_trace_source(g_batch_req_type), (unsigned)g_batch_req_target.num,
           (unsigned)g_batch_req_target.enable, (unsigned)g_batch_req_target.num,
           (unsigned)g_batch_req_previous.enable, (unsigned)g_batch_req_previous.num);
}
#endif

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

    if (g_batch_req_pending) {
#if SETTING_BATCH_TRACE_ENABLE
        setting_batch_trace_reject(SETTING_BATCH_REQUEST_NUMBER, batch_cmd);
#endif
        return false;
    }
    g_batch_req_type = SETTING_BATCH_REQUEST_NUMBER;
    g_batch_req_target.enable = true;
    g_batch_req_target.num = num;
    g_batch_req_previous.enable = previous_enable;
    g_batch_req_previous.num = previous_num;
    g_batch_req_pending = true;
#if SETTING_BATCH_TRACE_ENABLE
    setting_batch_trace_tx();
#endif
    protocol_send(0x06, &batch_cmd, 1);
    return true;
}

bool setting_service_request_batch_switch(bool target_enable, uint8_t sent_num, bool previous_enable, uint8_t previous_num)
{
    uint8_t batch_cmd = sent_num;

    if (g_batch_req_pending) {
#if SETTING_BATCH_TRACE_ENABLE
        setting_batch_trace_reject(SETTING_BATCH_REQUEST_SWITCH, batch_cmd);
#endif
        return false;
    }
    g_batch_req_type = SETTING_BATCH_REQUEST_SWITCH;
    g_batch_req_target.enable = target_enable;
    g_batch_req_target.num = sent_num;
    g_batch_req_previous.enable = previous_enable;
    g_batch_req_previous.num = previous_num;
    g_batch_req_pending = true;
#if SETTING_BATCH_TRACE_ENABLE
    setting_batch_trace_tx();
#endif
    protocol_send(0x06, &batch_cmd, 1);
    return true;
}

bool setting_service_batch_take_result(uint8_t status, setting_batch_result_t *result)
{
    if (!g_batch_req_pending) {
#if SETTING_BATCH_TRACE_ENABLE
        if (status == 0x01 || status == 0x02) {
            printf("[BATCH_TRACE] RX_UNMATCHED time_ms=%llu status=%s\n",
                   (unsigned long long)setting_batch_trace_now_ms(), setting_batch_trace_status(status));
        }
#endif
        return false;
    }
    if (status != 0x01 && status != 0x02) {
#if SETTING_BATCH_TRACE_ENABLE
        printf("[BATCH_TRACE] RX_INVALID time_ms=%llu status=%u pending=1\n",
               (unsigned long long)setting_batch_trace_now_ms(), (unsigned)status);
#endif
        return false;
    }
    if (result == NULL) {
#if SETTING_BATCH_TRACE_ENABLE
        printf("[BATCH_TRACE] RX_RESULT_NULL time_ms=%llu status=%s pending=1 active_seq=%u\n",
               (unsigned long long)setting_batch_trace_now_ms(), setting_batch_trace_status(status),
               g_batch_trace_active_seq);
#endif
        return false;
    }

#if SETTING_BATCH_TRACE_ENABLE
    setting_batch_trace_rx(status);
#endif
    result->type = g_batch_req_type;
    result->target = g_batch_req_target;
    result->previous = g_batch_req_previous;
    g_batch_req_pending = false;
    g_batch_req_type = SETTING_BATCH_REQUEST_NONE;
    return true;
}
