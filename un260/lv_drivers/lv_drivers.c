#include "lv_drivers.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

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

    // 停止位
    if (stopBit == 1) tty.c_cflag &= ~CSTOPB;
    else if (stopBit == 2) tty.c_cflag |= CSTOPB;
    else return -1;

    // 波特率
    speed_t speed;
    switch (baud) {
        case 9600: speed = B9600; break;
        case 115200: speed = B115200; break;
        case 921600: speed = B921600; break;
        default: speed = B115200; break;
    }
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    // 超时与最小接收字符
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

/* 关闭串口 */
void uart_close(int fd)
{
    if (fd > 0) close(fd);
}
