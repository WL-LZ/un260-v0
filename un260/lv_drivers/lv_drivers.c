#include "lv_drivers.h"
#include "un260/protocol/protocol_send.h"
#include "un260/boot/boot_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
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


 const char* g_currency_error_desc[0x32] = {
    [0x00] = "No Error",
    [0x01] = "IMG F1",
    [0x02] = "IMG F2",
    [0x03] = "IMG F3",
    [0x04] = "IMG F4",
    [0x05] = "IMG F5",
    [0x06] = "IMG F6",
    [0x07] = "IMG F7",
    [0x08] = "IMG F8",
    [0x09] = "IMG F9",
    [0x0A] = "IMG F10",
    [0x0B] = "IMG F11",
    [0x0C] = "IMG F12",
    [0x0D] = "IMG F13",
    [0x0E] = "IMG F14",
    [0x0F] = "IMG F15",
    [0x10] = "ST Full",
    [0x11] = "MG Qty",
    [0x12] = "MG Pos",
    [0x13] = "MT Qty",
    [0x14] = "MT Code",
    [0x15] = "UV",
    [0x16] = "Double 1",
    [0x17] = "Double 2",
    [0x18] = "Long",
    [0x19] = "Short",
    [0x1A] = "GAP",
    [0x1B] = "Time out",
    [0x1C] = "Size Unknow",
    [0x1D] = "Ort Unknow",
    [0x1E] = "Version Unknow",
    [0x1F] = "Face Error",
    [0x20] = "Ort Error",
    [0x21] = "ANGLE",
    [0x22] = "IR-OVD",
    [0x23] = "IR-MT",
    [0x24] = "Hole",
    [0x25] = "DogEar",
    [0x26] = "DIRT",
    [0x27] = "Tape",
    [0x28] = "Tears",
    [0x29] = "Crumples",
    [0x2A] = "De_ink",
    [0x2B] = "Soiling",
    [0x2C] = "Error_Limpness",
    [0x2D] = "IMG F&O Err",
    [0x2E] = "OCR1 Err",
    [0x2F] = "OCR2 Err",
    [0x30] = "OCR3 Err",
    [0x31] = "Double Rsv",
};



/* 打开串口 */
int uart_open(const char *device)
{
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("uart_open");
        return -1;
    }
    fcntl(fd, F_SETFL, 0);  // 阻塞模式
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
        default: speed = B115200; break;
    }
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);


    tty.c_cc[VTIME] = 1; // 0.1s
    tty.c_cc[VMIN]  = 1;

    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }
    return 0;
}

/* 接收数据 */
int uart_recv(int fd, char *rcv_buf, int data_len, int timeout)
{
    fd_set fs_read;
    struct timeval tv;
    int ret;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);

    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    memset(rcv_buf, 0, data_len);

    ret = select(fd + 1, &fs_read, NULL, NULL, &tv);
    if (ret > 0) return read(fd, rcv_buf, data_len);
    else return -1;
}

/* 发送数据 */
int uart_send(int fd, const char *send_buf, int data_len)
{
    ssize_t ret = write(fd, send_buf, data_len);
    tcflush(fd, TCOFLUSH);
    return (ret == data_len) ? ret : -1;
}

void uart_close(int fd)
{
    if (fd > 0) close(fd);
}


//通用发送指令
int protocol_send(uint8_t cmd_g, const uint8_t *cmd_s, uint16_t cmd_s_len)
{
    uint8_t buf[256];
    int i = 0;

    buf[i++] = CHECK1;
    buf[i++] = CHECK2;

    // 整帧总长度 = 头(2) + 长度字段(1) + CMD-G(1) + CMD-Sx(N) + CRC(1)
    uint16_t len = 2 + 1 + 1 + cmd_s_len + 1;
    buf[i++] = (uint8_t)len;   // 第3字节是整帧长度
    buf[i++] = cmd_g;

    if (cmd_s && cmd_s_len > 0) {
        memcpy(&buf[i], cmd_s, cmd_s_len);
        i += cmd_s_len;
    }

    buf[i++] = 0x0A; // CRC

    uart_printf(fd6, "Send CMD: 0x%02X, Data:", cmd_g);
    for (size_t j = 0; j < i; j++) {
        uart_printf(fd6, " %02X", buf[j]);
    }

    return uart_send(fd4, (const char *)buf, i);
}

int send_command(int fd, uint8_t cmd_g, const uint8_t *cmd_s, uint16_t cmd_s_len)
{
    (void)fd;
    return protocol_send(cmd_g, cmd_s, cmd_s_len);
}


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

void machine_handshake_send(void)
{
    uint32_t now = custom_tick_get();
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
