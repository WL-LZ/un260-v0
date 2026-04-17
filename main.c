#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "aic_ui.h"
#include "aic_dec.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/lv_page_declear.h"
#include "un260/lv_core/lv_page_event.h"
#include "un260/lv_core/page_01_main.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/ui_text.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_core/page_08_boot.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_components/lv_print_toast.h"
#include "un260/lv_core/ui_upgrade_service.h"
#include "aic_ui/perf_stats.h"
#include <stdlib.h>
//-------------------- UART 打印函数 --------------------

//-------------------- 全局变量 --------------------
 int fd4 = -1, fd5 = -1, fd6 = -1;
static bool uart_running = false;
#define MAX_CMD_PER_TICK  8
#define RECV_BUF_SIZE 512
#define MAX_CMD_QUEUE 32

typedef struct {
    uint8_t data[RECV_BUF_SIZE];
    int len;
} cmd_frame_t;

static cmd_frame_t cmd_queue[MAX_CMD_QUEUE];
static volatile int queue_head = 0;
static volatile int queue_tail = 0;
static volatile int queue_count = 0;

static uint8_t gPCRecvBuff[RECV_BUF_SIZE];
static int gPCRecvIndex = 0;
static int gPCRecvLen = 0;
static int gPCRecvSig = 0;        // 是否正在接收一帧
static int gPCRecvComplete = 0;   // 一帧接收完成标志


// 添加互斥锁保护共享变量
static pthread_mutex_t recv_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_wait_sn_after_reject_end = false;
static bool g_denom_query_pending = false;
static bool g_denom_query_deferred = false;
static bool g_denom_query_got_frame = false;
static uint32_t g_denom_query_tick = 0;
static uint8_t g_denom_query_retry = 0;
static uint32_t g_denom_query_idle_retry_tick = 0;
static lv_timer_t* g_mode_clear_timer = NULL;
#define UI_UPGRADE_DETECT_INTERVAL_MS 500
#define DENOM_QUERY_TIMEOUT_MS 1500
#define DENOM_QUERY_MAX_RETRY 2
#define DENOM_QUERY_IDLE_RETRY_MS 2500
static uint32_t g_ui_upgrade_detect_tick = 0;

static uint8_t g_boot_selftest_result[5] = {0};
static bool g_boot_selftest_has_error = false;
static uint8_t g_boot_selftest_first_error_type = 0;
static uint8_t g_boot_selftest_first_error_result = 0;
static void ui_upgrade_popup_poll(uint32_t now)
{
    ui_upgrade_detect_info_t detect_info;
    page_id_t current_page;

    current_page = ui_manager_get_current_page();

    if (current_page == UI_PAGE_BOOT_ANIM || current_page == UI_PAGE_BOOT) return;
    if (current_page == UI_PAGE_UI_UPGRADE) return;
    if ((now - g_ui_upgrade_detect_tick) < UI_UPGRADE_DETECT_INTERVAL_MS) return;

    g_ui_upgrade_detect_tick = now;
    ui_upgrade_service_detect(&detect_info);
    lv_upgrade_popup_process_detect(detect_info.usb_present, detect_info.package_found);
}

static void boot_selftest_result_reset(void)
{
    memset(g_boot_selftest_result, 0, sizeof(g_boot_selftest_result));
    g_boot_selftest_has_error = false;
    g_boot_selftest_first_error_type = 0;
    g_boot_selftest_first_error_result = 0;
}

//-------------------- 工具函数 --------------------
bool check_aa_header(const char* data, int len) {
    return (len >= 2 && (unsigned char)data[0] == 0xFD && (unsigned char)data[1] == 0xDF);
}

uint32_t custom_tick_get(void);

static const char* get_currency_error_desc(uint8_t code)
{
    if (code < sizeof(g_currency_error_desc) / sizeof(g_currency_error_desc[0]) &&
        g_currency_error_desc[code] != NULL) {
        return g_currency_error_desc[code];
    }
    return "Unknown Error";
}

/* UI显示使用协议错误类型（短文本），不使用调试长文案 */
static const char* get_start_ui_error_desc(uint8_t code)
{
    if (code == 0x00) {
        return ui_text_get(UI_TEXT_WIDGET_FAULT_NO_NOTE_MAIN);
    }

    if (code < sizeof(g_start_error_desc) / sizeof(g_start_error_desc[0]) &&
        g_start_error_desc[code] != NULL) {
        return g_start_error_desc[code];
    }

    return ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR);
}

static void sim_clear_sn_only(counting_sim_t* sim_data)
{
    if (!sim_data) return;
    if (sim_data->sn_str != NULL) {
        for (int i = 0; i < sim_data->total_pcs; i++) {
            if (sim_data->sn_str[i] != NULL) {
                free(sim_data->sn_str[i]);
                sim_data->sn_str[i] = NULL;
            }
        }
        free(sim_data->sn_str);
        sim_data->sn_str = NULL;
    }
    sim_data->total_pcs = 0;
    memset(sim_data->denom_mix, 0, sizeof(sim_data->denom_mix));
    sim_data->sn_capacity = 0;
}

static bool sim_ensure_sn_capacity(counting_sim_t* sim_data, int new_total)
{
    if (!sim_data || new_total <= 0) return false;
    if (new_total > (int)(sizeof(sim_data->denom_mix) / sizeof(sim_data->denom_mix[0]))) {
        return false;
    }

    if (sim_data->sn_str == NULL) {
        sim_data->sn_capacity = 0;
    }

    if (new_total <= sim_data->sn_capacity) {
        if (sim_data->total_pcs < new_total) {
            sim_data->total_pcs = new_total;
        }
        return true;
    }

    int new_cap = sim_data->sn_capacity > 0 ? sim_data->sn_capacity : 64;
    while (new_cap < new_total) {
        new_cap *= 2;
        if (new_cap > 10000) {
            new_cap = 10000;
            break;
        }
    }
    if (new_cap < new_total) return false;

    char** new_ptr = realloc(sim_data->sn_str, sizeof(char*) * new_cap);
    if (new_ptr == NULL) {
        return false;
    }
    if (new_cap > sim_data->sn_capacity) {
        memset(new_ptr + sim_data->sn_capacity, 0, sizeof(char*) * (new_cap - sim_data->sn_capacity));
    }
    sim_data->sn_str = new_ptr;
    sim_data->sn_capacity = new_cap;
    sim_data->total_pcs = new_total;
    return true;
}

static void ui_refresh_main_page_throttled(void)
{
    static uint32_t last_ms = 0;
    uint32_t now = custom_tick_get();
    if ((uint32_t)(now - last_ms) < 50) {
        return;
    }
    last_ms = now;
    ui_refresh_main_page();
}

static void format_amount_with_comma_fast(char* dest, size_t dest_size, float amount)
{
    char temp[32];
    int len;
    int dest_index = 0;
    int i;

    if (dest == NULL || dest_size == 0U) {
        return;
    }

    lv_snprintf(temp, sizeof(temp), "%.0f", amount);
    len = (int)strlen(temp);

    if (len <= 3) {
        lv_snprintf(dest, dest_size, "%s", temp);
        return;
    }

    for (i = 0; i < len && dest_index < (int)dest_size - 1; i++) {
        dest[dest_index++] = temp[i];
        if (i < len - 1 && ((len - i - 1) % 3) == 0 && dest_index < (int)dest_size - 1) {
            dest[dest_index++] = ',';
        }
    }
    dest[dest_index] = '\0';
}

/* 0x0E 高频帧：仅刷新主界面左侧紧凑区 */
static void ui_refresh_main_compact_fast(void)
{
    char amount_buf[32];

    update_label_by_name(page_01_main_obj, page_01_main_len, "01_pcs_label", "%d", sim.total_pcs);
    format_amount_with_comma_fast(amount_buf, sizeof(amount_buf), sim.total_amount);
    update_label_by_name(page_01_main_obj, page_01_main_len, "01_amount_label", "%s", amount_buf);
}

static bool is_main_page_active(void)
{
    return ui_manager_get_current_page() == UI_PAGE_MAIN &&
           main_page && lv_obj_is_valid(main_page);
}

// Clear cached denomination rows; next view refresh will wait for master 0x0B data.
static void clear_master_denom_cache(void)
{
    memset(sim.denom, 0, sizeof(sim.denom));
    sim.denom_number = 0;
    g_denom_query_got_frame = false;
}

static void request_denom_list(void)
{
    uint8_t sub = 0x01;
    send_command(fd4, 0x0B, &sub, 1);
    g_denom_query_pending = true;
    g_denom_query_got_frame = false;
    g_denom_query_tick = custom_tick_get();
    uart_printf(fd6, "request denom list: 0x0B 0x01\n");
}

static void trigger_denom_query(void)
{
    g_denom_query_retry = 0;
    /* Avoid injecting extra command during boot self-test/param-read flow. */
    if (g_boot_stage != BOOT_STAGE_DONE && g_boot_stage != BOOT_STAGE_FAIL) {
        g_denom_query_deferred = true;
        uart_printf(fd6, "defer denom query until boot done\n");
        return;
    }
    request_denom_list();
}

static void mode_switch_clear_timer_cb(lv_timer_t* timer)
{
    LV_UNUSED(timer);
    g_mode_clear_timer = NULL;
    sim_clear_all_sn(&sim);
}

static void schedule_mode_switch_clear(void)
{
    // 无数据时不做清理，避免无意义开销
    if (sim.total_pcs == 0 && sim.err_num == 0 && sim.err_expected == 0) {
        return;
    }

    if (g_mode_clear_timer != NULL) {
        lv_timer_del(g_mode_clear_timer);
        g_mode_clear_timer = NULL;
    }

    // 让底部文本动画先跑起来，再执行重置，降低按键卡顿体感
    g_mode_clear_timer = lv_timer_create(mode_switch_clear_timer_cb, 120, NULL);
    if (g_mode_clear_timer != NULL) {
        lv_timer_set_repeat_count(g_mode_clear_timer, 1);
    }
}

// ===== RX HEX 转字符串 =====
static void frame_to_hex_str(const uint8_t *buf, int len, char *out, int out_len)
{
    int pos = 0;
    for (int i = 0; i < len && pos + 3 < out_len; i++) {
        pos += snprintf(out + pos, out_len - pos, "%02X ", buf[i]);
    }
    if (pos > 0) out[pos - 1] = '\0'; // 去掉最后一个空格
}


// 队列入队
static bool enqueue_cmd(const uint8_t* data, int len) {
    bool ret = false;
    pthread_mutex_lock(&queue_mutex);
    if (queue_count < MAX_CMD_QUEUE) {
        memcpy(cmd_queue[queue_tail].data, data, len);
        cmd_queue[queue_tail].len = len;
        queue_tail = (queue_tail + 1) % MAX_CMD_QUEUE;
        queue_count++;
        ret = true;
    }
    pthread_mutex_unlock(&queue_mutex);
    return ret;
}
// 队列出队
static bool dequeue_cmd(cmd_frame_t* frame) {
    bool ret = false;
    pthread_mutex_lock(&queue_mutex);
    if (queue_count > 0) {
        *frame = cmd_queue[queue_head];
        queue_head = (queue_head + 1) % MAX_CMD_QUEUE;
        queue_count--;
        ret = true;
    }
    pthread_mutex_unlock(&queue_mutex);
    return ret;
}

void* uart4_thread(void* arg) {
    uint8_t byte;
    //uart_printf(fd6, "UART4 start (queue version)\n");
    pthread_mutex_lock(&recv_mutex);
    gPCRecvIndex = 0;
    gPCRecvLen = 0;
    gPCRecvSig = 0;
    gPCRecvComplete = 0;
    pthread_mutex_unlock(&recv_mutex);
    while (uart_running) {
        int len = uart_recv(fd4, (char*)&byte, 1, 10);
        if (len > 0) {
            pthread_mutex_lock(&recv_mutex);
            if (!gPCRecvComplete) {
                if (gPCRecvSig) {
                    gPCRecvBuff[gPCRecvIndex++] = byte;
                    if (gPCRecvIndex == 2) {
                        if (byte != 0xDF) {
                            gPCRecvSig = 0;
                            gPCRecvIndex = 0;
                            if (byte == 0xFD) {
                                gPCRecvIndex = 0;
                                gPCRecvSig = 1;
                                gPCRecvBuff[gPCRecvIndex++] = byte;
                            }
                        }
                    } else if (gPCRecvIndex == 3) {
                        gPCRecvLen = byte - 3;
                        if (byte < 3 || byte > RECV_BUF_SIZE) {
                            //uart_printf(fd6, "UART4: invalid len=%d, reset\n", byte);
                            gPCRecvSig = 0;
                            gPCRecvIndex = 0;
                            gPCRecvLen = 0;
                        }
                    } else if (gPCRecvIndex > 3) {
                        gPCRecvLen--;
                        if (gPCRecvLen == 0) {

                            /* ===== DEBUG: 打印完整帧 ===== */
                            uart_printf(fd6, "RX: ");
                            for (int i = 0; i < gPCRecvIndex; i++) {
                                uart_printf(fd6, "%02X ", gPCRecvBuff[i]);
                            }
                            uart_printf(fd6, "\n");
                            /* ============================ */

                            // 一帧接收完成，立即入队
                            if (!enqueue_cmd(gPCRecvBuff, gPCRecvIndex)) {
                                uart_printf(fd6, "UART4: queue full, drop frame\n");
                            }
                            gPCRecvSig = 0;
                            gPCRecvIndex = 0;
                            gPCRecvLen = 0;
                        }
                    }
                    if (gPCRecvIndex >= RECV_BUF_SIZE) {
                        //uart_printf(fd6, "UART4: buffer overflow, reset\n");
                        gPCRecvSig = 0;
                        gPCRecvIndex = 0;
                        gPCRecvLen = 0;
                    }
                } else if (byte == 0xFD) {
                    gPCRecvIndex = 0;
                    gPCRecvSig = 1;
                    gPCRecvBuff[gPCRecvIndex++] = byte;
                    //uart_printf(fd6, "UART4: new frame started\n");
                }
            }
            pthread_mutex_unlock(&recv_mutex);
        }
        usleep(100);
    }
    //uart_printf(fd6, "UART4 end\n");
    return NULL;
}

void* uart5_thread(void* arg) {
    uint8_t buf[256];

    uart_printf(fd6, "UART5 start\n");

    while (uart_running) {
        int len = uart_recv(fd5, (char*)buf, sizeof(buf), 100);
        if (len > 0) {

            uart_printf(fd6, "UART5 recive %d 字节: ", len);
            for(int i = 0; i < len; i++) {
                uart_printf(fd6, "%02X ", buf[i]);
            }
            uart_printf(fd6, "\n");

            if (fd4 >= 0) {
                int ret = uart_send(fd4, (const char*)buf, len);
                uart_printf(fd6, "UART5: sent UART4，长度=%d\n", ret);
            }
        }
        usleep(1000); // 1ms
    }

    uart_printf(fd6, "UART5 end\n");
    return NULL;
}

static void boot_selftest_finish_cb(lv_timer_t* timer)
{
    boot_selftest_list_finish();     // 自检结束后补全最后一项成功状态
    ui_manager_switch(UI_PAGE_MAIN); // 切到主页面
    lv_timer_del(timer);             // 删除定时器
}

static int sensor_idx_to_ch(uint8_t idx)
{
    switch (idx) {
    case 0x01: return 0;  // QTH
    case 0x02: return 1;  // QTL
    case 0x03: return 2;  // RJH
    case 0x04: return 3;  // RJL
    case 0x05: return 4;  // PS1L
    case 0x06: return 5;  // PS1R
    case 0x07: return 6;  // PS2
    case 0x08: return 7;  // PS5L
    case 0x09: return 8;  // PS5R
    case 0x0A: return 9;  // ST
    case 0x0B: return 10;  // SD
    default:   return -1;
    }
}


void PCCmdHandle(void)
{
    cmd_frame_t frame;
    int processed = 0;
    while (processed < MAX_CMD_PER_TICK && dequeue_cmd(&frame)) {
        processed++; // 每轮最多处理 MAX_CMD_PER_TICK 帧，避免丢帧
        uint8_t *buf = frame.data;
        uint8_t len  = frame.len;
        uint8_t cmd  = buf[3];

    /* ========= 新增：打印到 Debug 日志 ========= */
    char hex_log[256];
    frame_to_hex_str(buf, len, hex_log, sizeof(hex_log));
    debug_append_rx_log(hex_log);
    /* ========================================== */

        //uart_printf(fd6, "Processing command 0x%02X, len=%d\n", cmd, len);

        switch (cmd) {

        /* ================== 0x01 握手 ================== */
        case 0x01: 
        {
            if (buf[4] == 0x01) {
                boot_progress_set(20);
                Machine_Statue.g_handshake_state = HANDSHAKE_OK;
                g_handshake_start_tick = 0;
                boot_selftest_result_reset();
                g_boot_stage = BOOT_STAGE_SENSOR;
                boot_send_next_selftest();
            }
            break;
        }

        /* ================== 0x17 版本信息 ================== */
        case 0x17:
        {
            if (len < 18) {
                uart_printf(fd6, "0x17: frame too short (%d)\n", len);
                break;
            }

            uint8_t *p = &buf[4];
            strncpy(Machine_Statue.display_app, UI_VERSION, sizeof(Machine_Statue.display_app) - 1);
            Machine_Statue.display_app[sizeof(Machine_Statue.display_app) - 1] = '\0';
            snprintf(Machine_Statue.main_app,  sizeof(Machine_Statue.main_app),
                     "%d.%d.%d", p[0], p[1], p[2]);

            snprintf(Machine_Statue.image_app, sizeof(Machine_Statue.image_app),
                     "%d.%d.%d", p[3], p[4], p[5]);

            snprintf(Machine_Statue.fpga, sizeof(Machine_Statue.fpga),
                     "%d.%d", p[6], p[7]);

            snprintf(Machine_Statue.thka_app, sizeof(Machine_Statue.thka_app),
                     "%d.%d.%d", p[8], p[9], p[10]);

            snprintf(Machine_Statue.ecb, sizeof(Machine_Statue.ecb),
                     "%d.%d.%d", p[11], p[12], p[13]);
            Machine_Statue.version_valid = true;

            uart_printf(fd6, "Version Info Received\n");
            break;
        }

        /* ================== 0x0E 点钞信息 ================== */
        case 0x0E:
        {
            if (len < 12) break;

            uint8_t *p = &buf[4];

            uint32_t amount = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
            uint16_t qty    = (p[4] << 8) | p[5];
            uint8_t  ret    = p[6];
            uint8_t  status = p[7];

            if (status <= 0x01) {
                sim.total_amount = amount;
                sim.total_pcs    = qty;
                sim.err_expected = ret;
                ui_refresh_main_compact_fast();
                ui_refresh_main_page_throttled();
                if (!( !fault_popup_get_auto_enabled() && fault_popup_has_pending_start_issue())) {
                    smart_island_notify_count_start();
                    smart_island_refresh_summary();
                }
            } else if (status == 0x02) {
                uart_printf(fd6, "Count finished\n");
                ui_refresh_main_page();
                smart_island_notify_count_end(NULL);
                smart_island_refresh_summary();
            }
            break;
        }

        /* ================== 0x03 设置货币 ================== */
        case 0x03:
        {
            if (len < 6) break;

            uint8_t status = buf[4];

            if (status == 0x01)
            {
                page_07_curr_set_pending_result(status);
                uart_printf(fd6, "Set %s curr success\n", Machine_para.curr_code);
                trigger_denom_query();
                smart_island_refresh_summary();
            }
            else if (status == 0x02)
            {
                page_07_curr_set_pending_result(status);
                uart_printf(fd6, "Set %s curr fail\n", Machine_para.curr_code);
            }
            else if (status == 0x03)
            {
                if (len < 9) break;

                Machine_para.curr_code[0] = (char)buf[5];
                Machine_para.curr_code[1] = (char)buf[6];
                Machine_para.curr_code[2] = (char)buf[7];
                Machine_para.curr_code[3] = '\0';

                uart_printf(fd6, "Boot curr: %s\n", Machine_para.curr_code);
                clear_master_denom_cache();
                if (is_main_page_active()) {
                    ui_refresh_main_page();
                }
                trigger_denom_query();
                smart_island_refresh_summary();
            }

            break;
        }
        /* ================== 0x04 设置工作模式 ================== */
        case 0x04:
        {
            if (len < 6) break;

            uint8_t status = buf[4];

            if (status == 0x01)
            {
                uint8_t mode = Machine_work_code.mode_code;
                if (mode == 0x03) {
                    Machine_para.mode = MODE_MDC;
                } else if (mode == 0x04) {
                    Machine_para.mode = MODE_SDC;
                } else if (mode == 0x05) {
                    Machine_para.mode = MODE_CNT;
                }
                Machine_work_code.mode_code = 0;
                {
                    const char* mode_str = "NONE";
                    if (Machine_para.mode == MODE_MDC) mode_str = "MDC";
                    else if (Machine_para.mode == MODE_SDC) mode_str = "SDC";
                    else if (Machine_para.mode == MODE_CNT) mode_str = "CNT";

                    update_label_by_name(page_01_main_obj, page_01_main_len, "mix_label", "%s", mode_str);
                    update_label_by_name(page_01_main_obj, page_01_main_len, "mode_label", "%s", mode_str);
                    page_01_bottom_a_refresh_mode(true);
                }
                schedule_mode_switch_clear();
                uart_printf(fd6, "Set work mode success\n");
                smart_island_refresh_summary();
            }
            else if (status == 0x02)
            {
                Machine_work_code.mode_code = 0;
                uart_printf(fd6, "Set work mode fail\n");
                show_start_fault_popup(0x02, 0x06);
            }
            else if (status == 0x03)
            {
                if (len < 7) break;
                uint8_t mode = buf[5];

                if (mode == 0x03) {
                    Machine_para.mode = MODE_MDC;
                } else if (mode == 0x04) {
                    Machine_para.mode = MODE_SDC;
                } else if (mode == 0x05) {
                    Machine_para.mode = MODE_CNT;
                } else {
                    uart_printf(fd6, "Boot work mode invalid: 0x%02X\n", mode);
                    break;
                }

                Machine_work_code.mode_code = 0;
                page_01_mode_switch_refre();
                uart_printf(fd6, "Boot work mode: 0x%02X\n", mode);
                smart_island_refresh_summary();
            }

            break;
        }
        /* ================== 0x06 设置预置数 ================== */
        case 0x06:
        {
            if (len < 6) break;

            uint8_t status = buf[4];

            if (status == 0x01)
            {
                batch_switch_on_0x06_result(0x01);
                page_03_batch_set_result(0x01);
                uart_printf(fd6, "Set batch num success\n");
                smart_island_refresh_summary();
            }
            else if (status == 0x02)
            {
                batch_switch_on_0x06_result(0x02);
                page_03_batch_set_result(0x02);
                show_batch_set_fail_popup();
                uart_printf(fd6, "Set batch num fail\n");
            }
            else if (status == 0x03)
            {
                if (len < 7) break;
                Machine_para.batch_num = buf[5];
                Machine_para.batch_switch_enable = (Machine_para.batch_num != 200);
                set_batch_switch_state(Machine_para.batch_switch_enable);
                if (Machine_para.batch_switch_enable) {
                    update_label_by_name(page_03_menu_obj, page_03_menu_len, "03_batch_num_label",
                                         "%d", Machine_para.batch_num);
                } else {
                    update_label_by_name(page_03_menu_obj, page_03_menu_len, "03_batch_num_label",
                                         "%s", "OFF");
                }
                page_01_batch_refre();
                uart_printf(fd6, "Boot batch num: %d\n", Machine_para.batch_num);
                smart_island_refresh_summary();
            }

            break;
        }
        /* ================== 0x08 设置退钞口张数 ================== */
        case 0x08:
        {
            if (len < 7) break;

            uint8_t type = buf[4];
            uint8_t res  = buf[5];

            if (type != 0x01) {
                uart_printf(fd6, "0x08 unknown type=0x%02X, res=0x%02X\n", type, res);
                break;
            }

            if (res == 0x01) {
                uart_printf(fd6, "Reject pocket pcs set success\n");
            } else if (res == 0x02) {
                uart_printf(fd6, "Reject pocket pcs set fail\n");
            } else if (res == 0x03) {
                if (len < 8) break;
                Machine_para.reject_pocket_max = buf[6];
                uart_printf(fd6, "Boot reject pocket pcs: %u\n", Machine_para.reject_pocket_max);
            } else {
                uart_printf(fd6, "0x08 unknown res=0x%02X\n", res);
            }

            break;
        }
        /* ================== 0x0F 点钞过程中清分机状态 ================== */
        case 0x0F:
        {
            if (len < 6) break;

            uint8_t fault = buf[4];

            if (fault == 0x00) {
                hide_fault_popup();
                fault_popup_clear_pending();
                fault_popup_reset_auto_retry();
                g_sys_err_last_code = 0x00;
                smart_island_restore_idle();
                break;
            }

            fault_popup_report_runtime_fault(fault);
            uart_printf(fd6, "0x0F fault=0x%02X %s\n", fault, get_system_error_desc(fault));
            smart_island_notify_warning_level(get_system_error_desc(fault), SMART_ISLAND_WARNING_LEVEL_ERROR);
            break;
        }
        /* ================== 0x0a 返回主界面 ================== */
        case 0x0a:
        {
            if (len < 7) break;

            uint8_t type = buf[4];
            uint8_t val  = buf[5];

            if (type == 0x01) {
                if (val == 0x01) {
                    hide_counting_error_popup();
                    fault_popup_clear_pending();
                    fault_popup_reset_auto_retry();

                    if (g_data_collect_mode != DATA_COLLECT_MODE_NONE) {
                        snprintf(g_data_collect_status,
                                sizeof(g_data_collect_status),
                                "Counting started...");
                        page_06_data_collection_refresh();
                    }
                    else if (!g_cb_running) {
                        ui_manager_switch(UI_PAGE_MAIN);
                    }
                    smart_island_notify_count_start();
                } else if (val == 0x02) {
                    fault_popup_report_start_no_note();
                    uart_printf(fd6, "0x0A start fail (no note)\n");
                    smart_island_notify_warning_level(
                        ui_text_get(UI_TEXT_WIDGET_FAULT_NO_NOTE_MAIN),
                        SMART_ISLAND_WARNING_LEVEL_WARNING);

                    if (g_data_collect_mode != DATA_COLLECT_MODE_NONE) {
                        snprintf(g_data_collect_status,
                                sizeof(g_data_collect_status),
                                "Start failed: No banknotes detected");
                        page_06_data_collection_refresh();
                    }
                } else {
                    fault_popup_report_start_fault(type, val);
                    uart_printf(fd6, "0x0A start fail (normal): val=%02X desc=%s\n",
                                val, get_counting_error_desc(type, val));
                    smart_island_notify_warning_level(get_start_ui_error_desc(val), SMART_ISLAND_WARNING_LEVEL_ERROR);

                    if (g_data_collect_mode != DATA_COLLECT_MODE_NONE) {
                        snprintf(g_data_collect_status,
                                sizeof(g_data_collect_status),
                                "Start failed: %s",
                                get_counting_error_desc(type, val));
                        page_06_data_collection_refresh();
                    }
                }
            } else if (type == 0x02) {
                fault_popup_report_start_fault(type, val);
                uart_printf(fd6, "0x0A start fail (fault): code=%02X desc=%s\n",
                            val, get_counting_error_desc(type, val));
                smart_island_notify_warning_level(get_start_ui_error_desc(val), SMART_ISLAND_WARNING_LEVEL_ERROR);

                if (g_data_collect_mode != DATA_COLLECT_MODE_NONE) {
                    snprintf(g_data_collect_status,
                            sizeof(g_data_collect_status),
                            "Start failed: %s",
                            get_counting_error_desc(type, val));
                    page_06_data_collection_refresh();
                }
            } else {
                fault_popup_report_start_fault(type, val);
                uart_printf(fd6, "0x0A start fail (unknown type): type=%02X val=%02X\n",
                            type, val);
                smart_island_notify_warning_level(ui_text_get(UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR), SMART_ISLAND_WARNING_LEVEL_ERROR);
            }

            break;
        }
                /* ================== 0x0B 面额明细 ================== */
        case 0x0B:
        {
            if (len < 15) break;

            /* start frame: 00...00 (11 bytes payload are 0) */
            bool all_zero = true;
            for (int i = 4; i < 15; i++) {
                if (buf[i] != 0x00) { all_zero = false; break; }
            }
            if (all_zero) {
                memset(sim.denom, 0, sizeof(sim.denom));
                sim.denom_number = 0;
                g_denom_query_got_frame = true;
                uart_printf(fd6, "0x0B denom detail receive start\n");
                if (is_main_page_active()) {
                    ui_refresh_main_page_throttled();
                }
                break;
            }

            /* end frame: FF...FF (11 bytes payload are 0xFF) */
            bool all_ff = true;
            for (int i = 4; i < 15; i++) {
                if (buf[i] != 0xFF) { all_ff = false; break; }
            }
            if (all_ff) {
                uart_printf(fd6, "0x0B denom detail receive end\n");
                g_denom_query_got_frame = true;
                g_denom_query_retry = 0;

                if (g_denom_query_pending) {
                    g_denom_query_pending = false;
                    if (is_main_page_active()) {
                        ui_refresh_main_page();
                    }
                    break;
                }

                /* 串行时序：先请求 0x0C，等 0x0C 结束后再请求 0x0D */
                uint8_t reject_cmd = 0x01;
                send_command(fd4, 0x0C, &reject_cmd, 1);
                g_wait_sn_after_reject_end = true;
                if (is_main_page_active()) {
                    ui_refresh_main_page();
                }
                break;
            }

            /* normal data frame: denom(8 ascii) + pcs(3 ascii) */
            char denom_str[9] = {0};
            memcpy(denom_str, &buf[4], 8);
            int denom = atoi(denom_str);

            char pcs_str[4] = {0};
            memcpy(pcs_str, &buf[12], 3);
            int pcs = atoi(pcs_str);

            if (denom <= 0) break;
            g_denom_query_got_frame = true;

            int found = 0;
            for (int i = 0; i < sim.denom_number; i++) {
                if (sim.denom[i].value == denom) {
                    sim.denom[i].pcs += pcs;
                    sim.denom[i].amount = denom * sim.denom[i].pcs;
                    found = 1;
                    break;
                }
            }

            /* NEW: append if not found */
            if (!found) {
                if (sim.denom_number < (int)(sizeof(sim.denom) / sizeof(sim.denom[0]))) {
                    int i = sim.denom_number;
                    sim.denom[i].value = denom;
                    sim.denom[i].pcs = pcs;
                    sim.denom[i].amount = denom * pcs;
                    sim.denom_number++;
                }
            }

            if (is_main_page_active()) {
                ui_refresh_main_page_throttled();
            }

            break;
        }

        /* ================== 0x0C 退钞明细 ================== */
        case 0x0C:
        {
            if (len < 7) break;

            uint8_t err_code = buf[4];
            uint8_t pcs = buf[5];

            if (err_code == 0x00 && pcs == 0x00) {
                sim_clear_err_only(&sim);
                /* 保留期望数量（来自 0x0E），用于 LIST 立即看到“应有多少条” */
                //uart_printf(fd6, "0x0C reject detail receive start\n");
                break;
            }

            if (err_code == 0xFF && pcs == 0xFF) {
                page_02_c_report_status.curent_page = 1;
                page_02_c_report_status.total_page = (sim.err_num == 0)
                    ? 1
                    : ((sim.err_num + PAGE_02_C_ITEM - 1) / PAGE_02_C_ITEM);
                page_02_c_page_refre();
                page_02_c_page_num_refre();
                uart_printf(fd6, "0x0C reject detail receive end, parsed=%u expected=%u\n",
                            sim.err_num, sim.err_expected);
                smart_island_refresh_summary();
                if (g_wait_sn_after_reject_end) {
                    uint8_t sn_req[2] = { 0x01, 0x01 };
                    send_command(fd4, 0x0D, sn_req, 2);
                    g_wait_sn_after_reject_end = false;
                }
                break;
            }

            /* ESC 清除后 err_expected=0，此时丢弃延迟到达的旧错误明细，避免主界面与 list 显示不一致。 */
            if (sim.err_expected == 0) {
                uart_printf(fd6, "0x0C detail ignored because err_expected=0\n");
                break;
            }

            if (!sim_ensure_err_capacity(&sim, (int)sim.err_num + 1)) {
                uart_printf(fd6, "0x0C: err capacity fail idx=%u\n", sim.err_num);
                break;
            }

            int idx = sim.err_num;
            const char* desc = get_currency_error_desc(err_code);
            size_t desc_len = strlen(desc);

            sim.err_str[idx] = malloc(desc_len + 1);
            if (sim.err_str[idx] == NULL) {
                uart_printf(fd6, "0x0C: err malloc fail idx=%d\n", idx);
                break;
            }
            memcpy(sim.err_str[idx], desc, desc_len + 1);
            sim.err_pcs[idx] = pcs;
            sim.err_num++;
            /* 不等 end 帧：收到一条就刷新 LIST 的报错区 */
            page_02_c_report_status.total_page = (sim.err_num == 0)
                ? 1
                : ((sim.err_num + PAGE_02_C_ITEM - 1) / PAGE_02_C_ITEM);
            page_02_c_page_refre();
            page_02_c_page_num_refre();
            smart_island_refresh_summary();
            break;
        }

        /* ================== 0x0D 冠字号明细 ================== */
        case 0x0D:
        {
            if (len < 6) break;

            int payload_len = len - 4; // buf[4..] 为 payload
            if (payload_len < 2) break;

            // start frame: payload 全 0x00
            bool all_zero = true;
            for (int i = 4; i < len; i++) {
                if (buf[i] != 0x00) { all_zero = false; break; }
            }
            if (all_zero) {
                sim_clear_sn_only(&sim);
                uart_printf(fd6, "0x0D serial detail receive start\n");
                break;
            }

            // end frame: payload 全 0xFF
            bool all_ff = true;
            for (int i = 4; i < len; i++) {
                if (buf[i] != 0xFF) { all_ff = false; break; }
            }
            if (all_ff) {
                uart_printf(fd6, "0x0D serial detail receive end, total=%d\n", sim.total_pcs);
                page_02_report_init();
                page_02_b_page_refre();
                page_02_b_page_num_refre();
                break;
            }

            uint8_t seq = buf[4];
            if (seq == 0x00 || seq == 0xFF) break;
            int idx = (int)seq - 1;
            if (idx < 0 || idx >= 10000) break;

            int data_len = payload_len - 1; // 去掉序号字节
            if (data_len <= 0) break;

            // 协议末尾 1 字节校验，默认忽略
            int ascii_len = data_len - 1;
            if (ascii_len <= 0) break;

            char ascii_buf[32];
            if (ascii_len >= (int)sizeof(ascii_buf)) {
                ascii_len = (int)sizeof(ascii_buf) - 1;
            }
            memcpy(ascii_buf, &buf[5], ascii_len);
            ascii_buf[ascii_len] = '\0';

            // trim right spaces
            int r = ascii_len - 1;
            while (r >= 0 && ascii_buf[r] == ' ') {
                ascii_buf[r] = '\0';
                r--;
            }

            // trim left spaces
            char *p = ascii_buf;
            while (*p == ' ') p++;
            if (*p == '\0') break;

            // parse denom at head
            int denom = 0;
            while (*p && isdigit((unsigned char)*p)) {
                denom = denom * 10 + (*p - '0');
                p++;
            }
            while (*p == ' ') p++;
            if (*p == '\0') break;

            // ensure capacity and store
            if (!sim_ensure_sn_capacity(&sim, idx + 1)) {
                uart_printf(fd6, "0x0D: SN capacity fail idx=%d\n", idx);
                break;
            }

            if (sim.sn_str[idx]) {
                free(sim.sn_str[idx]);
                sim.sn_str[idx] = NULL;
            }
            size_t sn_len = strlen(p);
            sim.sn_str[idx] = malloc(sn_len + 1);
            if (!sim.sn_str[idx]) {
                uart_printf(fd6, "0x0D: SN malloc fail idx=%d\n", idx);
                break;
            }
            memcpy(sim.sn_str[idx], p, sn_len + 1);
            sim.denom_mix[idx] = denom;
            break;
        }
        /* ================== 0x1D 传感器电压 ================== */
        case 0x1D:
        {
            if (len < 7) break;

            uint8_t idx = buf[4];
            uint8_t val = buf[5];

            if (idx == 0x00 && val == 0x00) {
                memset(g_sensor_voltage.valid, 0, sizeof(g_sensor_voltage.valid));
                break;
            }

            if (idx == 0xFF && val == 0xFF) {
                break;
            }

            int ch = sensor_idx_to_ch(idx);
            if (ch >= 0) {
                g_sensor_voltage.raw[ch] = val;
                g_sensor_voltage.valid[ch] = true;
                g_sensor_voltage.update_count++;
            }

            break;
        }


        /* ================== 0x5B CIS 校准 ================== */
        case 0x5B:
        {
            if (len < 5) break;

            if (g_calib_target == CALIB_TARGET_CB) {
                switch (buf[4]) {
                case 0x01: cb_state = CB_CALIB_RUNNING; break;
                case 0x02: cb_state = CB_CALIB_SUCCESS; break;
                case 0x05: cb_state = CB_CALIB_FAIL_IR; break;
                default:   break;
                }
            } else {
                switch (buf[4]) {
                case 0x01: cis_state = CIS_CALIB_RUNNING; break;
                case 0x02: cis_state = CIS_CALIB_SUCCESS; break;
                case 0x03: cis_state = CIS_CALIB_FAIL_UPPER; break;
                case 0x04: cis_state = CIS_CALIB_FAIL_LOWER; break;
                case 0x05: cis_state = CIS_CALIB_FAIL_IR; break;
                default:   break;
                }
            }

            cis_calib_ui_refresh();
            break;
        }
        /* ================== 0x37 自检 ================== */
        case 0x37:
        {
            if (g_boot_stage == BOOT_STAGE_FAIL || g_boot_stage == BOOT_STAGE_DONE) {
                break;
            }

            if (len < 6) break;

            uint8_t test_type = buf[4];
            uint8_t result = buf[5];

            int index = -1;

            switch (test_type)
            {
                case 0x04:
                    index = 0;
                    break;

                case 0x01:
                    index = 1;
                    break;

                case 0x02:
                    index = 2;
                    break;

                case 0x03:
                    index = 3;
                    break;

                case 0x05:
                    index = 4;
                    break;

                default:
                    break;
            }

            if (result == 0x01) {
                if (index >= 0) {
                    g_boot_selftest_result[index] = 1;
                }

            } else {
                if (index >= 0) {
                    g_boot_selftest_result[index] = (result == 0x03) ? 3 : 2;
                }

                // 自检期间先记录首个错误，待全部流程结束后再统一弹出
                if (!g_boot_selftest_has_error) {
                    g_boot_selftest_has_error = true;
                    g_boot_selftest_first_error_type = test_type;
                    g_boot_selftest_first_error_result = result;
                }
            }

            if (index >= 0) {
                boot_selftest_list_set_result((uint8_t)index, result);
            }

            if (index >= 0) {
                boot_progress_set((uint8_t)(30 + index * 10));
            }

            g_boot_stage++;

            if (g_boot_stage <= BOOT_STAGE_IMAGE) {
                boot_send_next_selftest();
            } else {
                if (!g_boot_selftest_has_error) {
                    boot_progress_set(100);
                    send_command(fd4, 0x56, (uint8_t[]){0x01}, 1);
                    lv_timer_create(boot_selftest_finish_cb, 2000, NULL);
                } else {
                    g_boot_stage = BOOT_STAGE_FAIL;
                    // 自检失败也继续读取主控货币列表，避免页面回落本地默认配置
                    send_command(fd4, 0x56, (uint8_t[]){0x01}, 1);
                    show_boot_fault_popup(g_boot_selftest_first_error_type,
                                          g_boot_selftest_first_error_result);
                }
            }
        }
        break;
        /* ================== 0x58 用户偏好参数 ================== */
        case 0x58:
        {
            if (len < 6) break;

            uint8_t sub = buf[4];
            if (sub == 0x00) {
                uart_printf(fd6, "0x58 user preference receive start\n");
                break;
            }
            if (sub == 0xFF) {
                uart_printf(fd6, "0x58 user preference receive end\n");
                break;
            }

            switch (sub) {
            case 0x01: /* 工作模式 */
                Machine_para.mode = buf[5];
                break;

            case 0x02: /* 预置数量 */
                Machine_para.batch_num = buf[5];
                break;

            case 0x03: /* 预置金额 */
                if (len < 10) break;
                Machine_para.batch_amount = ((uint32_t)buf[5] << 24) |
                                            ((uint32_t)buf[6] << 16) |
                                            ((uint32_t)buf[7] << 8)  |
                                            (uint32_t)buf[8];
                break;

            case 0x04: /* 退钞口最大容量 */
                Machine_para.reject_pocket_max = buf[5];
                break;

            case 0x05: /* 蜂鸣器 */
                Machine_para.buzzer_enable = (buf[5] == 0x01);
                break;

            case 0x06: /* 点钞速度 */
            {
                uint8_t v = buf[5];
                if (v >= 1 && v <= SPEED_MODE) {
                    Machine_para.speed = v - 1;
                }
                break;
            }

            case 0x07: /* 冠字号 */
                Machine_para.serial_num_enable = (buf[5] == 0x01);
                break;

            case 0x08: /* 货币索引 */
                Machine_para.selected_currency = buf[5];
                break;

            case 0x09: /* 预置模式 */
                if (buf[5] == 0x01) {
                    Machine_para.batch_mode = PCS_BATCH_MODE;
                } else if (buf[5] == 0x02) {
                    Machine_para.batch_mode = AMOUNT_BATCH_MODE;
                }
                break;

            case 0x0A: /* 重张档位 */
            {
                uint8_t v = buf[5];
                if (v >= 1 && v <= CFD_MODE) {
                    Machine_para.cfd_mode = v - 1;
                }
                break;
            }

            default:
                break;
            }

            break;
        }
        /* ================== 0x56 货币查询 ================== */
        case 0x56:
        {
            if (len < 9) break;

            uint8_t idx = buf[4];
            uint8_t c1 = buf[5];
            uint8_t c2 = buf[6];
            uint8_t c3 = buf[7];

            if (idx == 0x00 && c1 == 0x00 && c2 == 0x00 && c3 == 0x00) {
                Machine_para.currency_count = 0;
                memset(Machine_para.currencies, 0, sizeof(Machine_para.currencies));
                uart_printf(fd6, "0x56 currency query start\n");
                break;
            }

            if (idx == 0xFF && c1 == 0xFF && c2 == 0xFF && c3 == 0xFF) {
                uart_printf(fd6, "0x56 currency query end, count=%d\n", Machine_para.currency_count);
                break;
            }

            if (idx >= 1 && idx <= MAX_CURRENCIES) {
                int pos = (int)idx - 1;
                Machine_para.currencies[pos][0] = (char)c1;
                Machine_para.currencies[pos][1] = (char)c2;
                Machine_para.currencies[pos][2] = (char)c3;
                Machine_para.currencies[pos][3] = '\0';
                if (Machine_para.currency_count < idx) {
                    Machine_para.currency_count = idx;
                }
                uart_printf(fd6, "0x56 currency[%d]=%s\n", pos, Machine_para.currencies[pos]);
            }
            break;
        }
        /* ================== 0x39 ADD setting ================== */
        case 0x39:
        {
            if (len < 6) break;
            uint8_t sub = buf[4];

            if (sub == 0x00) { 
                if (page_01_add_req_is_pending()) {
                    bool target = false;
                    page_01_add_req_finish(true, &target);
                    Machine_para.add_enable = target;
                    page_01_bottom_a_refresh_add(true);
                }
                uart_printf(fd6, "ADD set success\n");
                smart_island_refresh_summary();
            } else if (sub == 0x01) { 
                if (page_01_add_req_is_pending()) {
                    page_01_add_req_finish(false, NULL);
                }
                uart_printf(fd6, "ADD set failed\n");
                page_03_update_menu_button_states_refresh();
            } else if (sub == 0x02) { 
                uint8_t v = buf[5];
                Machine_para.add_enable = (v == 0x00) ? true : false;
                if (page_01_add_req_is_pending()) {
                    page_01_add_req_finish(false, NULL);
                }
                page_01_bottom_a_refresh_add(false);
                uart_printf(fd6, "ADD boot status: %s\n", Machine_para.add_enable ? "ON" : "OFF");
                smart_island_refresh_summary();
                page_03_update_menu_button_states_refresh();
            }

            break;
        }
        /* ================== 0x15 BEEP setting ================== */
        case 0x15:
        {
            if (len < 6) break;
            uint8_t sub = buf[4];

            if (sub == 0x01) {
                uart_printf(fd6, "BEEP set success\n");
            } else if (sub == 0x02) {
                uart_printf(fd6, "BEEP set failed\n");
                Machine_para.buzzer_enable = !Machine_para.buzzer_enable;
                page_03_update_menu_button_states_refresh();
            } else if (sub == 0x03) {
                if (len < 7) break;
                uint8_t v = buf[5];
                Machine_para.buzzer_enable = (v == 0x01);
                uart_printf(fd6, "BEEP boot status: %s\n", Machine_para.buzzer_enable ? "ON" : "OFF");
                page_03_update_menu_button_states_refresh();
            }

            break;
        }
        /* ================== 0x16 SPEED setting ================== */
        case 0x16:
        {
            if (len < 6) break;
            uint8_t type = buf[4];
            uint8_t res  = buf[5];

            if (type >= 0x01 && type <= 0x03) {
                if (res == 0x01) {
                    /* 协议: 0x01=1000,0x02=800,0x03=600 -> UI: 2,1,0 */
                    Machine_para.speed = (uint8_t)(0x03 - type);
                    page_03_update_menu_button_states_refresh();
                    uart_printf(fd6, "SPEED set SUCCESS: type=0x%02X -> ui=%u\n",
                                type, Machine_para.speed);
                    smart_island_refresh_summary();
                } else if (res == 0x02) {
                    page_03_update_menu_button_states_refresh();
                    uart_printf(fd6, "SPEED set FAIL: type=0x%02X\n", type);
                } else {
                    uart_printf(fd6, "SPEED set UNKNOWN result: type=0x%02X, res=0x%02X\n",
                                type, res);
                }
            } else if (type == 0x04) {
                /* 开机同步: 0x00=1000,0x01=800,0x02=600 -> UI: 2,1,0 */
                if (res <= 0x02) {
                    Machine_para.speed = (uint8_t)(0x02 - res);
                    page_03_update_menu_button_states_refresh();
                    uart_printf(fd6, "SPEED boot sync: mode=0x%02X -> ui=%u\n",
                                res, Machine_para.speed);
                    smart_island_refresh_summary();
                } else {
                    uart_printf(fd6, "SPEED boot sync: invalid mode=0x%02X\n", res);
                }
            } else {
                uart_printf(fd6, "0x16: unknown type=0x%02X, res=0x%02X\n", type, res);
            }

            break;
        }

        /* ================== 0x3A F/O 面向模式 ================== */
        case 0x3A:
        {
            if (len < 6) {
                uart_printf(fd6, "0x3A: frame too short (%d)\n", len);
                break;
            }
            uint8_t type = buf[4];   
            uint8_t val  = buf[5];   
            if (type == 0x05) {
                if (val >= 0x01 && val <= 0x04) {
                    Machine_para.fo_mode = (uint8_t)(val - 1); // 1~4 -> UI 0~3
                    uart_printf(fd6, "FO boot sync: mode=0x%02X -> ui=%u\n",
                                val, Machine_para.fo_mode);
                } else if (val <= 0x03) {
                    Machine_para.fo_mode = val; 
                    uart_printf(fd6, "FO boot sync(legacy): mode=0x%02X -> ui=%u\n",
                                val, Machine_para.fo_mode);
                } else {
                    uart_printf(fd6, "FO boot sync: invalid mode=0x%02X\n", val);
                }
                if (page_01_fo_req_is_pending()) {
                    page_01_fo_req_finish(false, NULL);
                }
                page_01_bottom_a_refresh_fo(false);
                smart_island_refresh_summary();
                break;
            }
            if (type >= 0x01 && type <= 0x04) {
                if (val == 0x01) {
                    if (page_01_fo_req_is_pending()) {
                        uint8_t target_mode = 0;
                        page_01_fo_req_finish(true, &target_mode);
                        Machine_para.fo_mode = target_mode;
                        page_01_bottom_a_refresh_fo(true);
                    } else {
                        Machine_para.fo_mode = (uint8_t)(type - 1);
                        page_01_bottom_a_refresh_fo(false);
                    }
                    uart_printf(fd6, "FO set SUCCESS: type=0x%02X -> ui=%u\n",
                                type, Machine_para.fo_mode);
                    smart_island_refresh_summary();
                } else if (val == 0x02) {
                    if (page_01_fo_req_is_pending()) {
                        page_01_fo_req_finish(false, NULL);
                    }
                    uart_printf(fd6, "FO set FAIL: type=0x%02X\n", type);
                    page_03_update_menu_button_states_refresh();
                } else {
                    uart_printf(fd6, "FO set UNKNOWN result: type=0x%02X, res=0x%02X\n", type, val);
                }
            } else {
                uart_printf(fd6, "0x3A: unknown type=0x%02X, val=0x%02X\n", type, val);
            }
            break;
        }
        /* ================== 0x38 手动/自动模式 ================== */

        case 0x38:
        {
            if (len < 5) break;

            if (len == 7 && buf[4] == 0x02) {
                uint8_t mode = buf[5];
                if (mode == 0x00) {
                    Machine_para.work_mode = 1;
                } else if (mode == 0x01) {
                    Machine_para.work_mode = 0;
                }
                if (page_01_work_req_is_pending()) {
                    page_01_work_req_finish(false, NULL);
                }
                page_01_bottom_a_refresh_work(false);
                uart_printf(fd6, "0x38 BOOT mode=0x%02X\n", mode);
                smart_island_refresh_summary();
                break;
            }

            uint8_t res = buf[4];
            if (res == 0x00) {
                if (page_01_work_req_is_pending()) {
                    uint8_t target_mode = 0;
                    page_01_work_req_finish(true, &target_mode);
                    Machine_para.work_mode = target_mode;
                    page_01_bottom_a_refresh_work(true);
                } else {
                    Machine_para.work_mode = 1;
                    page_01_bottom_a_refresh_work(false);
                }
                page_03_update_menu_button_states_refresh();
                uart_printf(fd6, "0x38 MANUAL OK\n");
                smart_island_refresh_summary();
            } else if (res == 0x01) {
                if (page_01_work_req_is_pending()) {
                    uint8_t target_mode = 0;
                    page_01_work_req_finish(true, &target_mode);
                    Machine_para.work_mode = target_mode;
                    page_01_bottom_a_refresh_work(true);
                } else {
                    Machine_para.work_mode = 0;
                    page_01_bottom_a_refresh_work(false);
                }
                page_03_update_menu_button_states_refresh();
                uart_printf(fd6, "0x38 AUTO OK\n");
                smart_island_refresh_summary();
            } else {
                if (page_01_work_req_is_pending()) {
                    page_01_work_req_finish(false, NULL);
                }
                uart_printf(fd6, "0x38 RES=0x%02X\n", res);
            }
                break;
            }

        /* ================== 0xA1 主控升级状态 ================== */
        case 0xA1:
        {
            if (len < 6) break;
            uint8_t res = buf[4];
            ui_page_14_main_upgrade_on_reply(0xA1, res);
            uart_printf(fd6, "0xA1 res=0x%02X\n", res);
            break;
        }

        /* ================== 0xB0 图像升级状态 ================== */
        case 0xB0:
        {
            if (len < 6) break;
            uint8_t res = buf[4];
            ui_page_15_image_upgrade_on_reply(0xB0, res);
            uart_printf(fd6, "0xB0 res=0x%02X\n", res);
            break;
        }
        case 0xC0:
        {
            if (len < 5) break;

            switch (buf[4]) {
            case 0x01:
                snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                        "ALL DATA collection mode ready.");
                break;

            case 0x02:
                snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                        "Collection completed. Data can be copied from USB.");
                break;

            case 0x03:
                snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                        "FALSE REPORT collection mode ready.");
                break;

            case 0x05:
                snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                        "USB ready. You can start counting.");
                break;

            case 0x06:
                snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                        "USB not ready. Collection mode error.");
                break;

            case 0xFF:
                g_data_collect_mode = DATA_COLLECT_MODE_NONE;
                g_data_collect_pcs = 0;
                snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                        "Collection mode exited.");
                break;

            default:
                snprintf(g_data_collect_status, sizeof(g_data_collect_status),
                        "Collection reply: 0x%02X", buf[4]);
                break;
            }

            page_06_data_collection_refresh();
            break;
        }
                /* ================== 0x3C 打印状态 ================== */
        case 0x3C:
        {
            if (len == 0x10) {
                uart_printf(fd6, "0x3C print detail frame\n");
            }
            else if (len == 0x08) {
                uart_printf(fd6, "0x3C print done\n");
            }
            else {
                uart_printf(fd6, "0x3C unknown len=%d\n", len);
            }

            break;
        }
        default:
            uart_printf(fd6, "Unknown command 0x%02X\n", cmd);
            break;
        }
    }
}


static void sent_machine_code(void)
{   uint8_t mdc_cmd = 0x03;
    send_command(fd4, 0x04, &mdc_cmd, 1); // 发送设置币种命令，默认USD
}
static lv_obj_t* g_msgbox_cont = NULL;

static void msgbox_close_cb(lv_event_t* e)
{
    if (g_msgbox_cont && lv_obj_is_valid(g_msgbox_cont)) {
        lv_obj_del(g_msgbox_cont);
        g_msgbox_cont = NULL;
    }
}

void lv_show_center_msgbox(const char* title_text, const char* info_text)
{
    lv_obj_t* scr = lv_scr_act();

    g_msgbox_cont = lv_obj_create(scr);
    lv_obj_set_size(g_msgbox_cont, 400, 200);
    lv_obj_set_pos(g_msgbox_cont, 440, 100); // 居中在 (1280,400)
    lv_obj_set_style_bg_color(g_msgbox_cont, lv_color_hex(0xE1E1E1), 0); // 灰色
    lv_obj_set_style_radius(g_msgbox_cont, 10, 0);
    lv_obj_set_style_opa(g_msgbox_cont, LV_OPA_COVER, 0);

    lv_obj_add_event_cb(g_msgbox_cont, msgbox_close_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* label_title = lv_label_create(g_msgbox_cont);
    lv_label_set_text(label_title, title_text);
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_30, 0);
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* label_info = lv_label_create(g_msgbox_cont);
    lv_label_set_text(label_info, info_text);
    lv_obj_set_style_radius(g_msgbox_cont, 10, 0);
    lv_obj_set_style_text_font(label_info, &lv_font_montserrat_22, 0);

    lv_obj_align(label_info, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_add_event_cb(scr, msgbox_close_cb, LV_EVENT_CLICKED, NULL);
}

//-------------------- 自定义Tick --------------------
uint32_t custom_tick_get(void) {
    static uint64_t start_ms = 0;
    if (start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    return (uint32_t)(now_ms - start_ms);
}

//-------------------- 主函数 --------------------
int main(void) {
    lv_init();
    lv_img_cache_set_size(IMG_CACHE_NUM);
    aic_dec_create();

    lv_port_disp_init();
    lv_port_indev_init();
    ui_manager_switch(UI_PAGE_BOOT_ANIM);
    perf_stats_init();

    printf("=== 初始化UART4、UART5和UART6 ===\n");

    fd4 = uart_open("/dev/ttyS4");
    fd5 = uart_open("/dev/ttyS5");
    fd6 = uart_open("/dev/ttyS6");

    if (fd4 < 0 || fd5 < 0 || fd6 < 0) {
        printf("UART打开失败: fd4=%d fd5=%d fd6=%d\n", fd4, fd5, fd6);
        return -1;
    }

    uart_config(fd4, 115200, 8, 'N', 1);
    uart_config(fd5, 115200, 8, 'N', 1);
    uart_config(fd6, 115200, 8, 'N', 1);

    printf("UART配置完成\n");

    uart_running = true;
    pthread_t thread4, thread5;
    pthread_create(&thread4, NULL, uart4_thread, NULL);
    pthread_create(&thread5, NULL, uart5_thread, NULL);
    pthread_detach(thread4);
    pthread_detach(thread5);
   // machine_handshake_send(); 只发一次握手

    // 启动阶段避免阻塞，防止 LVGL 动画计时器错过播放窗口
    uart_printf(fd6, "UART6 ready\n");

    while (1) {
        uint32_t now = custom_tick_get();
        ui_page_t current_page = ui_manager_get_current_page();
        struct timeval ui_tv_start;
        struct timeval ui_tv_end;
        uint32_t ui_time_us;

        gettimeofday(&ui_tv_start, NULL);
        lv_timer_handler();
        gettimeofday(&ui_tv_end, NULL);
        ui_time_us = (uint32_t)((ui_tv_end.tv_sec - ui_tv_start.tv_sec) * 1000000UL +
                                (ui_tv_end.tv_usec - ui_tv_start.tv_usec));
        perf_stats_report_ui_time_us(ui_time_us);
        PCCmdHandle();
        ui_upgrade_popup_poll(now);

        if (g_denom_query_pending &&
            (now - g_denom_query_tick) >= DENOM_QUERY_TIMEOUT_MS) {
            g_denom_query_pending = false;
            if (!g_denom_query_got_frame) {
                if (g_denom_query_retry < DENOM_QUERY_MAX_RETRY) {
                    g_denom_query_retry++;
                    uart_printf(fd6, "0x0B query timeout, retry %u/%u\n",
                                g_denom_query_retry, DENOM_QUERY_MAX_RETRY);
                    request_denom_list();
                } else {
                    uart_printf(fd6, "0x0B query timeout, keep master-only mode (no local fallback)\n");
                }
            }
        }

        if (g_denom_query_deferred &&
            !g_denom_query_pending &&
            (g_boot_stage == BOOT_STAGE_DONE || g_boot_stage == BOOT_STAGE_FAIL)) {
            g_denom_query_deferred = false;
            request_denom_list();
        }

        if (!g_denom_query_pending &&
            !g_denom_query_got_frame &&
            is_main_page_active() &&
            (g_boot_stage == BOOT_STAGE_DONE || g_boot_stage == BOOT_STAGE_FAIL)) {
            if ((now - g_denom_query_idle_retry_tick) >= DENOM_QUERY_IDLE_RETRY_MS) {
                g_denom_query_idle_retry_tick = now;
                g_denom_query_retry = 0;
                uart_printf(fd6, "0x0B idle retry on main page\n");
                request_denom_list();
            }
        }

        if (g_boot_stage == BOOT_STAGE_HANDSHAKE &&
            current_page == UI_PAGE_BOOT)
        {
            if (Machine_Statue.g_handshake_state == HANDSHAKE_IDLE)
            {
                machine_handshake_send();
            }
            else if (Machine_Statue.g_handshake_state == HANDSHAKE_SENT)
            {
                if (g_handshake_start_tick != 0 &&
                    (now - g_handshake_start_tick) >= BOOT_HANDSHAKE_MAX_WAIT_MS)
                {
                    g_boot_stage = BOOT_STAGE_FAIL;

                    show_boot_selftest_error_popup(
                        "Controller handshake timeout.\nPress CONFIRM to enter sensor page.");
                }
                else if ((now - g_handshake_tick) >= HANDSHAKE_TIMEOUT_MS)
                {
                    machine_handshake_send();
                }
            }
        }

        if (g_boot_stage >= BOOT_STAGE_SENSOR && g_boot_stage <= BOOT_STAGE_IMAGE &&
            current_page == UI_PAGE_BOOT)
        {
            if (g_boot_stage_tick != 0 &&
                (uint32_t)(now - g_boot_stage_tick) >= BOOT_SELFTEST_TIMEOUT_MS)
            {
                g_boot_stage = BOOT_STAGE_FAIL;
                show_boot_selftest_error_popup(
                    "Self-test timeout.\nPress CONFIRM to enter sensor page.");
            }
        }

        usleep(1000);
    }
    uart_running = false;
    if (fd4 >= 0) uart_close(fd4);
    if (fd5 >= 0) uart_close(fd5);
    if (fd6 >= 0) uart_close(fd6);

    return 0;
}
