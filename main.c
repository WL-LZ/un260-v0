#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include "lvgl/lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "aic_ui.h"
#include "aic_dec.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/lv_page_declear.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/platform_app.h"
#include <stdlib.h>

//-------------------- UART 打印函数 --------------------

//-------------------- 全局变量 --------------------
 int fd4 = -1, fd5 = -1, fd6 = -1;
static bool uart_running = false;

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

//-------------------- 工具函数 --------------------
bool check_aa_header(const char* data, int len) {
    return (len >= 2 && (unsigned char)data[0] == 0xFD && (unsigned char)data[1] == 0xDF);
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
    uart_printf(fd6, "UART4 start (queue version)\n");
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
                            uart_printf(fd6, "UART4: invalid len=%d, reset\n", byte);
                            gPCRecvSig = 0;
                            gPCRecvIndex = 0;
                            gPCRecvLen = 0;
                        }
                    } else if (gPCRecvIndex > 3) {
                        gPCRecvLen--;
                        if (gPCRecvLen == 0) {
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
                        uart_printf(fd6, "UART4: buffer overflow, reset\n");
                        gPCRecvSig = 0;
                        gPCRecvIndex = 0;
                        gPCRecvLen = 0;
                    }
                } else if (byte == 0xFD) {
                    gPCRecvIndex = 0;
                    gPCRecvSig = 1;
                    gPCRecvBuff[gPCRecvIndex++] = byte;
                    uart_printf(fd6, "UART4: new frame started\n");
                }
            }
            pthread_mutex_unlock(&recv_mutex);
        }
        usleep(100);
    }
    uart_printf(fd6, "UART4 end\n");
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
    ui_manager_switch(UI_PAGE_MAIN); // 切到主页面
    lv_timer_del(timer);             // 删除定时器
}


void PCCmdHandle(void)
{
    cmd_frame_t frame;

    while (dequeue_cmd(&frame)) {
        uint8_t *buf = frame.data;
        uint8_t len  = frame.len;
        uint8_t cmd  = buf[3];

    /* ========= 新增：打印到 Debug 日志 ========= */
    char hex_log[256];
    frame_to_hex_str(buf, len, hex_log, sizeof(hex_log));
    debug_append_rx_log(hex_log);
    /* ========================================== */

        uart_printf(fd6, "Processing command 0x%02X, len=%d\n", cmd, len);

        switch (cmd) {

        /* ================== 0x01 握手 ================== */
        case 0x01: 
        {
            if (buf[4] == 0x01) {
                Machine_Statue.g_handshake_state = HANDSHAKE_OK;
                g_boot_stage = BOOT_STAGE_SENSOR;

                bootlog_append("Handshake OK");
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

            strcpy(Machine_Statue.display_app, "N/A");
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
                sim.err_num      = ret;
                ui_refresh_main_page();
            } else if (status == 0x02) {
                uart_printf(fd6, "Count finished\n");
                ui_refresh_main_page();
            }
            break;
        }

        /* ================== 0x03 设置货币 ================== */
        case 0x03:
        {
            if (len < 6) break;

            uint8_t status = buf[5];

            uart_printf(fd6,
                "Set %s curr %s\n",
                Machine_para.curr_code,
                (status == 0x01) ? "success" : "fail");

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
        uart_printf(fd6, "0x0B denom detail receive start\n");
        ui_refresh_main_page();
        break;
    }

    /* end frame: FF...FF (11 bytes payload are 0xFF) */
    bool all_ff = true;
    for (int i = 4; i < 15; i++) {
        if (buf[i] != 0xFF) { all_ff = false; break; }
    }
    if (all_ff) {
        uart_printf(fd6, "0x0B denom detail receive end\n");
        ui_refresh_main_page();
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
        if (sim.denom_number < 16 /* 你sim里数组上限 */) {
            int i = sim.denom_number;
            sim.denom[i].value = denom;
            sim.denom[i].pcs = pcs;
            sim.denom[i].amount = denom * pcs;
            sim.denom_number++;
        }
    }

    ui_refresh_main_page();
    break;
}


        /* ================== 0x5B CIS 校准 ================== */
        case 0x5B:
        {
            if (len < 5) break;

            switch (buf[4]) {
            case 0x01: cis_state = CIS_CALIB_RUNNING; break;
            case 0x02: cis_state = CIS_CALIB_SUCCESS; break;
            case 0x03: cis_state = CIS_CALIB_FAIL_UPPER; break;
            case 0x04: cis_state = CIS_CALIB_FAIL_LOWER; break;
            case 0x05: cis_state = CIS_CALIB_FAIL_IR; break;
            default:   break;
            }

            cis_calib_ui_refresh();
            break;
        }
        /* ================== 0x37 自检 ================== */
        case 0x37:   // 自检响应
        {
            uint8_t type   = buf[4];   // 0x01~0x05
            uint8_t result = buf[5];   // 0x01 成功 / 0x02 失败

            const char *name = NULL;

            switch (type) {
            case 0x01: name = "Sensor"; break;
            case 0x02: name = "Motor";  break;
            case 0x03: name = "Magnet"; break;
            case 0x04: name = "Config"; break;
            case 0x05: name = "Image";  break;
            default: break;
            }

            if (result == 0x01) {
                char buf_log[64];
                snprintf(buf_log, sizeof(buf_log),
                        "%s self-test SUCCESS", name);
                bootlog_append(buf_log);
            } else {
                char buf_log[64];
                snprintf(buf_log, sizeof(buf_log),
                        "%s self-test FAIL", name);
                bootlog_append(buf_log);
                g_boot_stage = BOOT_STAGE_FAIL;
                break;
            }

            // 进入下一阶段
            g_boot_stage++;

            if (g_boot_stage <= BOOT_STAGE_IMAGE) {
                boot_send_next_selftest();
            } else {
                bootlog_append("All self-tests finished");
                g_boot_stage = BOOT_STAGE_DONE;
                send_command(fd4, 0x56, (uint8_t[]){0x01}, 1);
            // 创建一个 2 秒后触发的定时器
                lv_timer_create(boot_selftest_finish_cb, 2000, NULL);     
            }

            break;
        }
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
                uart_printf(fd6, "ADD set success\n");
            } else if (sub == 0x01) { 
                uart_printf(fd6, "ADD set failed\n");
                Machine_para.add_enable = !Machine_para.add_enable;
                page_03_update_menu_button_states_refresh();
            } else if (sub == 0x02) { 
                uint8_t v = buf[5];
                Machine_para.add_enable = (v == 0x00) ? true : false;
                uart_printf(fd6, "ADD boot status: %s\n", Machine_para.add_enable ? "ON" : "OFF");
                page_03_update_menu_button_states_refresh();
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
{
    send_command(fd4, 0x04, Machine_MODE_MDC, 1); // 发送设置币种命令，默认USD
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
    ui_manager_switch(UI_PAGE_BOOT);

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

    // 测试UART6输出
    for(int i=0;i<5;i++){
        uart_printf(fd6, "UART6测试输出 %d\n", i);
        sleep(1);
    }

    while(1) {
        lv_timer_handler();
        PCCmdHandle();        // 现在PCCmdHandle是队列出队+解析
        
    if (g_boot_stage == BOOT_STAGE_HANDSHAKE) {
        if (Machine_Statue.g_handshake_state == HANDSHAKE_IDLE ||
            (Machine_Statue.g_handshake_state == HANDSHAKE_SENT &&
             custom_tick_get() - g_handshake_tick > HANDSHAKE_TIMEOUT_MS)) {

            machine_handshake_send();
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
