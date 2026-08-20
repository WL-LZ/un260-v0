#include "lv_drivers.h"
#include "uart_io.h"
#include "un260/app_service/app_clock.h"
#include "un260/protocol/protocol_send.h"
#include "un260/boot/boot_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/page_08_boot.h"

cis_calib_state_t cis_state = CIS_CALIB_IDLE;
cb_calib_state_t cb_state = CB_CALIB_IDLE;
calib_target_t g_calib_target = CALIB_TARGET_CIS;
curr_query_state_t curr_query_state = CURR_QUERY_IDLE;

uint8_t g_cb_running = 0;
static bool g_boot_waiting_log_shown = false;


const char* g_start_error_desc[0x12] = {
    [0x00] = "No Error",
    [0x01] = "Upper Channel Error",
    [0x02] = "Lower Channel Error",
    [0x03] = "Reject Exit Error",
    [0x04] = "Reject Pocket Error",
    [0x05] = "Reject Pocket Full Only",
    [0x06] = "Stacker Pocket Error",
    [0x07] = "Stacker Pocket Full Only",
    [0x08] = "Stacker and Reject Pockets Full",
    [0x09] = "Upper and Lower Channels Not Closed",
    [0x0A] = "Genuine Note Exit Error",
    [0x0B] = "Dust Cover / Baffle Closure Error",
    [0x0C] = "Flap Error",
    [0x0D] = "Encoder Disk Error",
};


/* 打开串口 */
int uart_open(const char *device)
{
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("uart_open");
        return -1;
    }
    if (fcntl(fd, F_SETFL, 0) < 0) {  // 阻塞模式
        perror("uart_open fcntl");
        close(fd);
        return -1;
    }
    return fd;
}

/* 配置串口 */
int uart_config(int fd, int baud, int dataBit, char parity, int stopBit)
{
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }

    cfmakeraw(&tty);  // 原始模式
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;

    // 数据位
    if (dataBit == 7) tty.c_cflag |= CS7;
    else if (dataBit == 8) tty.c_cflag |= CS8;
    else return -1;

    // 校验位
    switch (parity) {
        case 'N': case 'n': tty.c_cflag &= ~PARENB; break;
        case 'E': case 'e': tty.c_cflag |= PARENB; tty.c_cflag &= ~PARODD; break;
        case 'O': case 'o': tty.c_cflag |= PARENB | PARODD; break;
        default: return -1;
    }


    if (stopBit == 1) tty.c_cflag &= ~CSTOPB;
    else if (stopBit == 2) tty.c_cflag |= CSTOPB;
    else return -1;


    speed_t speed;
    switch (baud) {
        case 9600: speed = B9600; break;
        case 115200: speed = B115200; break;
        case 921600: speed = B921600; break;
        default: return -1;
    }
    if (cfsetispeed(&tty, speed) != 0 || cfsetospeed(&tty, speed) != 0) {
        perror("uart_config speed");
        return -1;
    }


    tty.c_cc[VTIME] = 1; // 0.1s
    tty.c_cc[VMIN]  = 1;

    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }
    return 0;
}

void uart_close(int fd)
{
    if (fd >= 0) close(fd);
}


void machine_handshake_send(void)
{
    uint32_t now = app_clock_uptime_ms();
    uint8_t payload = 0x01;

    boot_service_start(now);
    boot_service_request_handshake(now);
    protocol_send(0x01, &payload, 1);
}

void boot_send_next_selftest(void)
{
    uint8_t protocol_step;

    boot_selftest_list_sync_step(boot_service_self_test_sequence_index());
    if (boot_service_next_self_test_protocol_step(&protocol_step)) {
        protocol_send(0x37, &protocol_step, 1);
    }
}

uint8_t boot_get_selftest_step(void) // 获取当前自检步骤
{
    return boot_service_self_test_sequence_index();
}

void boot_handshake_waiting_log_reset(void)
{
    g_boot_waiting_log_shown = false;
}
