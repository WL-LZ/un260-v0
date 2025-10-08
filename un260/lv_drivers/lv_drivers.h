#ifndef UART_DRV_H
#define UART_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdint.h>

/* 打开串口设备 */
int uart_open(const char *device);

/* 配置串口
 * fd      : 串口文件描述符
 * baud    : 波特率 (9600, 115200, 921600 ...)
 * dataBit : 数据位 (7 或 8)
 * parity  : 校验位 ('N' 无, 'E' 偶校验, 'O' 奇校验)
 * stopBit : 停止位 (1 或 2)
 */
int uart_config(int fd, int baud, int dataBit, char parity, int stopBit);

/* 接收数据，带超时 (timeout 毫秒)
 * 返回值：读取字节数，-1 表示超时或错误
 */
int uart_recv(int fd, char *rcv_buf, int data_len, int timeout);

/* 发送数据
 * 返回值：发送字节数，-1 表示错误
 */
int uart_send(int fd, const char *send_buf, int data_len);

/* 关闭串口 */
void uart_close(int fd);
int send_command(int fd, uint8_t cmd_g, const uint8_t *cmd_s, uint16_t cmd_s_len);
void uart_printf(int fd, const char *fmt, ...);

extern int fd4;
extern int fd5;
extern int fd6;
#define CHECK1 0xFD
#define CHECK2 0xDF
typedef uint8_t Machine_Mode_t;

#define Machine_MODE_MDC   0x03
#define Machine_MODE_SDC   0x04
#define Machine_MODE_CNT   0x05
#define Machine_MODE_MAX   0x06
#define Machine_AUT_MODE_MDC 0X01
#define Machine_MUL_MODE_MDC 0X02
#define Machine_CURR_MDOE_MAX 0X03

typedef struct {
    uint8_t mode_code;
} Machine_work_code_t;
extern Machine_work_code_t Machine_work_code;

#ifdef __cplusplus
}
#endif

#endif
