#include "un260/app_service/setting_service.h"
#include "un260/protocol/protocol_send.h"
#include "un260/protocol/mode_codec.h"
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#ifndef SETTING_BATCH_TRACE_ENABLE
#define SETTING_BATCH_TRACE_ENABLE 0
#endif

#define SETTING_DETAIL_REQUEST_TIMEOUT_MS 800U

static uint64_t setting_service_now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

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
static uint64_t g_batch_req_tick_ms = 0;
static bool g_double_note_req_pending = false;
static uint8_t g_double_note_req_target = 0;
static uint8_t g_double_note_req_previous = 0;
static uint64_t g_double_note_req_tick_ms = 0;
static bool g_flap_req_pending = false;
static uint8_t g_flap_req_target = 0;
static uint8_t g_flap_req_previous = 0;
static uint64_t g_flap_req_tick_ms = 0;
static bool g_reject_pocket_req_pending = false;
static uint8_t g_reject_pocket_req_target = 0;
static uint8_t g_reject_pocket_req_previous = 0;
static uint64_t g_reject_pocket_req_tick_ms = 0;

static bool setting_service_request_expired(bool pending, uint64_t request_tick_ms)
{
    return pending && setting_service_now_ms() - request_tick_ms >= SETTING_DETAIL_REQUEST_TIMEOUT_MS;
}

#if SETTING_BATCH_TRACE_ENABLE
static uint32_t g_batch_trace_next_seq = 0;
static uint32_t g_batch_trace_active_seq = 0;
static uint64_t g_batch_trace_tx_ms = 0;

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
    g_batch_trace_tx_ms = setting_service_now_ms();
    printf("[BATCH_TRACE] TX seq=%u time_ms=%llu source=%s payload=%u target_enable=%u target_num=%u previous_enable=%u previous_num=%u\n",
           g_batch_trace_active_seq, (unsigned long long)g_batch_trace_tx_ms,
           setting_batch_trace_source(g_batch_req_type), (unsigned)g_batch_req_target.num,
           (unsigned)g_batch_req_target.enable, (unsigned)g_batch_req_target.num,
           (unsigned)g_batch_req_previous.enable, (unsigned)g_batch_req_previous.num);
}

static void setting_batch_trace_reject(setting_batch_request_type_t source, uint8_t payload)
{
    uint64_t now_ms = setting_service_now_ms();

    printf("[BATCH_TRACE] REJECT time_ms=%llu source=%s payload=%u active_seq=%u active_source=%s active_target_enable=%u active_target_num=%u\n",
           (unsigned long long)now_ms, setting_batch_trace_source(source), (unsigned)payload,
           g_batch_trace_active_seq, setting_batch_trace_source(g_batch_req_type),
           (unsigned)g_batch_req_target.enable, (unsigned)g_batch_req_target.num);
}

static void setting_batch_trace_rx(uint8_t status)
{
    uint64_t now_ms = setting_service_now_ms();
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
    if (protocol_send(0x04, &protocol_mode, 1) < 0) {
        g_mode_req_pending = false;
        return false;
    }
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
    add_cmd = target ? 0x01 : 0x00;
    g_page01_add_req_pending = true;
    g_page01_add_req_target = target;
    g_page01_add_req_tick = request_tick;
    if (protocol_send(0x39, &add_cmd, 1) < 0) {
        g_page01_add_req_pending = false;
        return false;
    }
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
    fo_cmd = target;
    g_page01_fo_req_pending = true;
    g_page01_fo_req_target = target;
    g_page01_fo_req_tick = request_tick;
    if (protocol_send(0x3a, &fo_cmd, 1) < 0) {
        g_page01_fo_req_pending = false;
        return false;
    }
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
    if (target == 0) {
        speed_cmd = 0x03;
    } else if (target == 1) {
        speed_cmd = 0x02;
    } else {
        speed_cmd = 0x01;
    }
    g_page01_speed_req_pending = true;
    g_page01_speed_req_target = target;
    g_page01_speed_req_tick = request_tick;
    if (protocol_send(0x16, &speed_cmd, 1) < 0) {
        g_page01_speed_req_pending = false;
        return false;
    }
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
    work_cmd = (target == 1) ? 0x00 : 0x01;
    g_page01_work_req_pending = true;
    g_page01_work_req_target = target;
    g_page01_work_req_tick = request_tick;
    if (protocol_send(0x38, &work_cmd, 1) < 0) {
        g_page01_work_req_pending = false;
        return false;
    }
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
    beep_cmd = target ? 0x01 : 0x02;
    g_page03_beep_req_pending = true;
    g_page03_beep_req_target = target;
    g_page03_beep_req_tick = request_tick;
    if (protocol_send(0x15, &beep_cmd, 1) < 0) {
        g_page03_beep_req_pending = false;
        return false;
    }
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
    g_batch_req_tick_ms = setting_service_now_ms();
#if SETTING_BATCH_TRACE_ENABLE
    setting_batch_trace_tx();
#endif
    if (protocol_send(0x06, &batch_cmd, 1) < 0) {
        g_batch_req_pending = false;
        g_batch_req_type = SETTING_BATCH_REQUEST_NONE;
        return false;
    }
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
    g_batch_req_tick_ms = setting_service_now_ms();
#if SETTING_BATCH_TRACE_ENABLE
    setting_batch_trace_tx();
#endif
    if (protocol_send(0x06, &batch_cmd, 1) < 0) {
        g_batch_req_pending = false;
        g_batch_req_type = SETTING_BATCH_REQUEST_NONE;
        return false;
    }
    return true;
}

bool setting_service_batch_take_result(uint8_t status, setting_batch_result_t *result)
{
    if (setting_service_request_expired(g_batch_req_pending, g_batch_req_tick_ms)) return false;
    if (!g_batch_req_pending) {
#if SETTING_BATCH_TRACE_ENABLE
        if (status == 0x01 || status == 0x02) {
            printf("[BATCH_TRACE] RX_UNMATCHED time_ms=%llu status=%s\n",
                   (unsigned long long)setting_service_now_ms(), setting_batch_trace_status(status));
        }
#endif
        return false;
    }
    if (status != 0x01 && status != 0x02) {
#if SETTING_BATCH_TRACE_ENABLE
        printf("[BATCH_TRACE] RX_INVALID time_ms=%llu status=%u pending=1\n",
               (unsigned long long)setting_service_now_ms(), (unsigned)status);
#endif
        return false;
    }
    if (result == NULL) {
#if SETTING_BATCH_TRACE_ENABLE
        printf("[BATCH_TRACE] RX_RESULT_NULL time_ms=%llu status=%s pending=1 active_seq=%u\n",
               (unsigned long long)setting_service_now_ms(), setting_batch_trace_status(status),
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
    g_batch_req_tick_ms = 0;
    return true;
}

bool setting_service_batch_take_timeout(setting_batch_result_t *result)
{
    if (!result || !setting_service_request_expired(g_batch_req_pending, g_batch_req_tick_ms)) return false;

    result->type = g_batch_req_type;
    result->target = g_batch_req_target;
    result->previous = g_batch_req_previous;
    g_batch_req_pending = false;
    g_batch_req_type = SETTING_BATCH_REQUEST_NONE;
    g_batch_req_tick_ms = 0;
    return true;
}

bool setting_service_request_double_note_level(uint8_t target, uint8_t previous)
{
    if (g_double_note_req_pending) return false;

    g_double_note_req_pending = true;
    g_double_note_req_target = target;
    g_double_note_req_previous = previous;
    g_double_note_req_tick_ms = setting_service_now_ms();
    if (protocol_send(0x31, &target, 1) < 0) {
        setting_service_clear_double_note_level_request();
        return false;
    }

    return true;
}

bool setting_service_take_double_note_level_result(uint8_t status, setting_value_result_t *result)
{
    if (setting_service_request_expired(g_double_note_req_pending, g_double_note_req_tick_ms)) return false;
    if (!g_double_note_req_pending || result == NULL) return false;
    if (status != 0x01 && status != 0x02) return false;

    result->target = g_double_note_req_target;
    result->previous = g_double_note_req_previous;
    result->success = (status == 0x01);
    result->timeout = false;
    g_double_note_req_pending = false;
    g_double_note_req_tick_ms = 0;
    return true;
}

bool setting_service_take_double_note_level_timeout(setting_value_result_t *result)
{
    if (!result || !setting_service_request_expired(g_double_note_req_pending, g_double_note_req_tick_ms)) return false;

    result->target = g_double_note_req_target;
    result->previous = g_double_note_req_previous;
    result->success = false;
    result->timeout = true;
    setting_service_clear_double_note_level_request();
    return true;
}

void setting_service_clear_double_note_level_request(void)
{
    g_double_note_req_pending = false;
    g_double_note_req_tick_ms = 0;
}

bool setting_service_request_flap_position(uint8_t target, uint8_t previous)
{
    if (g_flap_req_pending) return false;

    g_flap_req_pending = true;
    g_flap_req_target = target;
    g_flap_req_previous = previous;
    g_flap_req_tick_ms = setting_service_now_ms();
    if (protocol_send(0x42, &target, 1) < 0) {
        g_flap_req_pending = false;
        g_flap_req_tick_ms = 0;
        return false;
    }

    return true;
}

bool setting_service_take_flap_position_result(uint8_t status, setting_value_result_t *result)
{
    if (setting_service_request_expired(g_flap_req_pending, g_flap_req_tick_ms)) return false;
    if (!g_flap_req_pending || result == NULL) return false;
    if (status != 0x00 && status != 0x01) return false;

    result->target = g_flap_req_target;
    result->previous = g_flap_req_previous;
    result->success = (status == 0x00);
    result->timeout = false;
    g_flap_req_pending = false;
    g_flap_req_tick_ms = 0;
    return true;
}

bool setting_service_take_flap_position_timeout(setting_value_result_t *result)
{
    if (!result || !setting_service_request_expired(g_flap_req_pending, g_flap_req_tick_ms)) return false;

    result->target = g_flap_req_target;
    result->previous = g_flap_req_previous;
    result->success = false;
    result->timeout = true;
    g_flap_req_pending = false;
    g_flap_req_tick_ms = 0;
    return true;
}

bool setting_service_request_reject_pocket_max(uint8_t target, uint8_t previous)
{
    uint8_t payload[2] = { 0x01, target };

    if (g_reject_pocket_req_pending) return false;

    g_reject_pocket_req_pending = true;
    g_reject_pocket_req_target = target;
    g_reject_pocket_req_previous = previous;
    g_reject_pocket_req_tick_ms = setting_service_now_ms();
    if (protocol_send(0x08, payload, sizeof(payload)) < 0) {
        setting_service_clear_reject_pocket_max_request();
        return false;
    }

    return true;
}

bool setting_service_take_reject_pocket_max_result(uint8_t status, setting_value_result_t *result)
{
    if (setting_service_request_expired(g_reject_pocket_req_pending, g_reject_pocket_req_tick_ms)) return false;
    if (!g_reject_pocket_req_pending || result == NULL) return false;
    if (status != 0x01 && status != 0x02) return false;

    result->target = g_reject_pocket_req_target;
    result->previous = g_reject_pocket_req_previous;
    result->success = (status == 0x01);
    result->timeout = false;
    g_reject_pocket_req_pending = false;
    g_reject_pocket_req_tick_ms = 0;
    return true;
}

bool setting_service_take_reject_pocket_max_timeout(setting_value_result_t *result)
{
    if (!result || !setting_service_request_expired(g_reject_pocket_req_pending, g_reject_pocket_req_tick_ms)) return false;

    result->target = g_reject_pocket_req_target;
    result->previous = g_reject_pocket_req_previous;
    result->success = false;
    result->timeout = true;
    setting_service_clear_reject_pocket_max_request();
    return true;
}

void setting_service_clear_reject_pocket_max_request(void)
{
    g_reject_pocket_req_pending = false;
    g_reject_pocket_req_tick_ms = 0;
}
