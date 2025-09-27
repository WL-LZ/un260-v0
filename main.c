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
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_system/platform_app.h"

//-------------------- UART 打印函数 --------------------

//-------------------- 全局变量 --------------------
 int fd4 = -1, fd5 = -1, fd6 = -1;
static bool uart_running = false;

#define RECV_BUF_SIZE 256
static uint8_t gPCRecvBuff[RECV_BUF_SIZE];
static int gPCRecvIndex = 0;
static int gPCRecvLen = 0;
static int gPCRecvSig = 0;        // 是否正在接收一帧
static int gPCRecvComplete = 0;   // 一帧接收完成标志

// 添加互斥锁保护共享变量
static pthread_mutex_t recv_mutex = PTHREAD_MUTEX_INITIALIZER;

//-------------------- 工具函数 --------------------
bool check_aa_header(const char* data, int len) {
    return (len >= 2 && (unsigned char)data[0] == 0xFD && (unsigned char)data[1] == 0xDF);
}

void* uart4_thread(void* arg) {
    uint8_t byte;

    uart_printf(fd6, "UART4 start\n");

    // 清空接收缓存状态
    pthread_mutex_lock(&recv_mutex);
    gPCRecvIndex = 0;
    gPCRecvLen = 0;
    gPCRecvSig = 0;
    gPCRecvComplete = 0;
    pthread_mutex_unlock(&recv_mutex);

    while (uart_running) {
        int len = uart_recv(fd4, (char*)&byte, 1, 100);
        if (len > 0) {
           // uart_printf(fd6, "UART4: receive code 0x%02X\n", byte);

            pthread_mutex_lock(&recv_mutex);
            
            // 只有在没有待处理的完整帧时才接收新数据
            if (!gPCRecvComplete) {
                if (gPCRecvSig) {
                    // 如果正在接收帧，结果又遇到新的 0xFD，就丢弃前面的
                    if (byte == 0xFD && gPCRecvIndex > 0) {
                        uart_printf(fd6, "UART4: found new 0xFD, reset previous frame\n");
                        gPCRecvIndex = 0;
                        gPCRecvLen = 0;
                        gPCRecvSig = 1; // 重新开始
                        gPCRecvBuff[gPCRecvIndex++] = byte;
                        pthread_mutex_unlock(&recv_mutex);
                        continue;
                    }

                    gPCRecvBuff[gPCRecvIndex++] = byte;

                    if (gPCRecvIndex == 2) {
                        if (byte != 0xDF) {
                            uart_printf(fd6, "UART4: second byte not 0xDF, reset\n");
                            gPCRecvSig = 0;
                            gPCRecvIndex = 0;
                        }
                    }
                    else if (gPCRecvIndex == 3) {
                        gPCRecvLen = byte - 3;
                       // uart_printf(fd6, "UART4: code len gPCRecvLen=%d (all len=%d)\n", gPCRecvLen, byte);

                        if (byte < 3 || byte > RECV_BUF_SIZE) {
                            uart_printf(fd6, "UART4: invalid len=%d, reset\n", byte);
                            gPCRecvSig = 0;
                            gPCRecvIndex = 0;
                            gPCRecvLen = 0;
                        }
                    }
                    else if (gPCRecvIndex > 3) {
                        gPCRecvLen--;
                        if (gPCRecvLen == 0) {
                            // 设置完成标志，但不清零其他变量
                            gPCRecvComplete = 1;
                            gPCRecvSig = 0;
                            uart_printf(fd6, "UART4: receive complete frame len=%d\n", gPCRecvIndex);
                            
                            // 立即转发到UART5
                            if (fd5 >= 0) {
                                int send_len = uart_send(fd5, (const char*)gPCRecvBuff, gPCRecvIndex);
                               // uart_printf(fd6, "UART4: sent UART5, len=%d\n", send_len);

                                uart_printf(fd6, "sent data: ");
                                for (int i = 0; i < gPCRecvIndex; i++) {
                                    uart_printf(fd6, "%02X ", gPCRecvBuff[i]);
                                }
                                uart_printf(fd6, "\n");
                            } else {
                                uart_printf(fd6, "UART4: fd5 < 0, can't send\n");
                            }
                            
                        }
                    }

                    // 防止溢出
                    if (gPCRecvIndex >= RECV_BUF_SIZE) {
                        uart_printf(fd6, "UART4: buffer overflow, reset\n");
                        gPCRecvSig = 0;
                        gPCRecvIndex = 0;
                        gPCRecvLen = 0;
                    }
                }
                else if (byte == 0xFD) {
                    // 检测到起始字节，开始接收新帧
                    uart_printf(fd6, "UART4: first code 0xFD\n");
                    gPCRecvIndex = 0;
                    gPCRecvSig = 1;
                    gPCRecvBuff[gPCRecvIndex++] = byte;
                }
            }
            
            pthread_mutex_unlock(&recv_mutex);
        }

        usleep(1000);
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

void PCCmdHandle(void)
{
    pthread_mutex_lock(&recv_mutex);
    
    if (!gPCRecvComplete) {
        pthread_mutex_unlock(&recv_mutex);
        return;  
    }
    
    uart_printf(fd6, "enter PCCmdHandle \n");

    uint8_t *buf = gPCRecvBuff;
    uint8_t Len = buf[2];  
    uint8_t cmd = buf[3];  
    
 switch (cmd) {
    case 0x0E:   // 点钞信息
    {
        uint32_t amount = ((uint32_t)buf[4] << 24) |
                          ((uint32_t)buf[5] << 16) |
                          ((uint32_t)buf[6] << 8)  |
                          ((uint32_t)buf[7]);
        uint16_t qty = ((uint16_t)buf[8] << 8) | buf[9];
        uint8_t ret_count = buf[10];   
        uint8_t status    = buf[11];   
        switch (status) {
            case 0x00: // 点钞中
            case 0x01:
                sim.total_amount = amount;
                sim.total_pcs = qty;
                sim.err_num = ret_count;
                ui_refresh_main_page();
                uart_printf(fd6, "Counting: amount=%u, qty=%u, ret=%u, status=0x%02X\n",amount, qty, ret_count, status);
                ui_refresh_main_page(); 
                break;
            case 0x02: // 点钞结束
                uart_printf(fd6, "Count finished\n");
                ui_refresh_main_page(); 
                break;
            default:
                uart_printf(fd6, "Unknown status 0x%02X\n", status);
                break;
        }

        break;
    }
    case 0x03:  //解析curr
    {
        uint8_t status = buf[5];
        if (status == 0x01) {
         uart_printf(fd6, "Set %s curr suceess\n",Machine_para.curr_code);
        } else if(status == 0x02)
        {
          uart_printf(fd6, "Set %s curr fail\n",Machine_para.curr_code);
        }
        

    }
case 0x0B: // 面额详细列表
{
    char denom_str[9] = {0};
    memcpy(denom_str, &buf[4], 8);
    int denom_value = atoi(denom_str);

    char pcs_str[4] = {0};
    memcpy(pcs_str, &buf[12], 3);
    int pcs_value = atoi(pcs_str);
    for (int i = 0; i < sim.denom_number; i++) {
        if (sim.denom[i].value == denom_value) {
            sim.denom[i].pcs += pcs_value;
            sim.denom[i].amount = sim.denom[i].value * sim.denom[i].pcs;
            break;
        }
    }
    uart_printf(fd6, "Denom=%d, pcs=%d, total_amount=%.2f\n",
                denom_value, pcs_value, (float)(denom_value * pcs_value));
    ui_refresh_main_page();
    break;
}


    default:
        uart_printf(fd6, "Unknown command 0x%02X\n", cmd);
        break;
}

    // 处理完成后清零标志和缓冲区
    gPCRecvComplete = 0;
    gPCRecvIndex = 0;
    gPCRecvLen = 0;
    
    pthread_mutex_unlock(&recv_mutex);
}

//-------------------- 主函数 --------------------
int main(void) {
    lv_init();
    lv_img_cache_set_size(IMG_CACHE_NUM);
    aic_dec_create();

    lv_port_disp_init();
    lv_port_indev_init();
    ui_manager_switch(UI_PAGE_MAIN);

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

    // 测试UART6输出
    for(int i=0;i<5;i++){
        uart_printf(fd6, "UART6测试输出 %d\n", i);
        sleep(1);
    }

    while(1) {
        lv_timer_handler();
        PCCmdHandle();        
        usleep(1000);
    }

    uart_running = false;
    if (fd4 >= 0) uart_close(fd4);
    if (fd5 >= 0) uart_close(fd5);
    if (fd6 >= 0) uart_close(fd6);

    return 0;
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