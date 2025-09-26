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

//-------------------- UART 打印函数 --------------------
void uart_printf(int fd, const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len > 0) {
        if (len > sizeof(buf)) len = sizeof(buf);
        int n = write(fd, buf, len);
        if (n != len) {
            printf("UART写失败 %d/%d\n", n, len);
        }
    }
}

//-------------------- 全局变量 --------------------
static int fd4 = -1, fd5 = -1, fd6 = -1;
static bool uart_running = false;

#define RECV_BUF_SIZE 256
static uint8_t gPCRecvBuff[RECV_BUF_SIZE];
static int gPCRecvIndex = 0;
static int gPCRecvLen = 0;
static int gPCRecvSig = 0;        // 是否正在接收一帧
static int gPCRecvComplete = 0;   // 一帧接收完成标志

//-------------------- 工具函数 --------------------
bool check_aa_header(const char* data, int len) {
    return (len >= 2 && (unsigned char)data[0] == 0xFD && (unsigned char)data[1] == 0xDF);
}


void* uart4_thread(void* arg) {
    uint8_t byte;

    uart_printf(fd6, "UART4 start\n");

    // 清空接收缓存状态
    gPCRecvIndex = 0;
    gPCRecvLen = 0;
    gPCRecvSig = 0;
    gPCRecvComplete = 0;

    while (uart_running) {
        int len = uart_recv(fd4, (char*)&byte, 1, 100);
        if (len > 0) {
            uart_printf(fd6, "UART4: receive code 0x%02X\n", byte);

            if (!gPCRecvComplete) {
                if (gPCRecvSig) {
                    // 如果正在接收帧，结果又遇到新的 0xFD，就丢弃前面的
                    if (byte == 0xFD && gPCRecvIndex > 0) {
                        uart_printf(fd6, "UART4: found new 0xFD, reset previous frame\n");
                        gPCRecvIndex = 0;
                        gPCRecvLen = 0;
                        gPCRecvSig = 1; // 重新开始
                        gPCRecvBuff[gPCRecvIndex++] = byte;
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
                        uart_printf(fd6, "UART4: code len gPCRecvLen=%d (all len=%d)\n", gPCRecvLen, byte);

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
                            gPCRecvComplete = 1;
                            gPCRecvSig = 0;
                            uart_printf(fd6, "UART4: receive all frame len=%d\n", gPCRecvIndex);
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

            // 如果一帧接收完成，进行转发
            if (gPCRecvComplete) {
                if (fd5 >= 0) {
                    int send_len = uart_send(fd5, (const char*)gPCRecvBuff, gPCRecvIndex);
                    uart_printf(fd6, "UART4: sent UART5, len=%d\n", send_len);

                    uart_printf(fd6, "sent data: ");
                    for (int i = 0; i < gPCRecvIndex; i++) {
                        uart_printf(fd6, "%02X ", gPCRecvBuff[i]);
                    }
                    uart_printf(fd6, "\n");
                } else {
                    uart_printf(fd6, "UART4: fd5 < 0, can't send\n");
                }

                gPCRecvIndex = 0;
                gPCRecvLen = 0;
                gPCRecvComplete = 0;
            }
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
