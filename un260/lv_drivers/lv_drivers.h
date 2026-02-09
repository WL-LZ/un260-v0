#ifndef UART_DRV_H
#define UART_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

extern const char* g_currency_error_desc[0x32];


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

extern void machine_handshake_send(void);

extern int fd4;
extern int fd5;
extern int fd6;
#define CHECK1 0xFD
#define CHECK2 0xDF
typedef uint8_t Machine_Mode_t;
extern void debug_append_rx_log(const char* data); // 接收数据时调用

#define Machine_MODE_MDC   0x03
#define Machine_MODE_SDC   0x04
#define Machine_MODE_CNT   0x05
#define Machine_MODE_MAX   0x06
#define Machine_AUT_MODE_MDC 0X01
#define Machine_MUL_MODE_MDC 0X02
#define Machine_CURR_MDOE_MAX 0X03

#define Query_ver_cmd 0x01
#define curr_cmd 0x01
typedef struct {
    uint8_t mode_code;
} Machine_work_code_t;
extern Machine_work_code_t Machine_work_code;

/*握手协议*/
typedef enum {
    HANDSHAKE_IDLE = 0,
    HANDSHAKE_SENT,
    HANDSHAKE_OK,
} handshake_state_t;
extern uint32_t g_handshake_tick;
#define HANDSHAKE_TIMEOUT_MS  1000

// CIS校准状态枚举
typedef enum {
    CIS_CALIB_IDLE = 0,          // 空闲
    CIS_CALIB_RUNNING,           // 校验中
    CIS_CALIB_SUCCESS,           // 校验成功
    CIS_CALIB_FAIL_UPPER,        // 上CIS失败
    CIS_CALIB_FAIL_LOWER,        // 下CIS失败
    CIS_CALIB_FAIL_IR            // 红外失败
} cis_calib_state_t;
extern cis_calib_state_t cis_state;

/* 获取币种状态 */
typedef enum {
    CURR_QUERY_IDLE = 0,
    CURR_QUERY_RUNNING,
    CURR_QUERY_DONE,
    CURR_QUERY_FAIL,
} curr_query_state_t;

extern curr_query_state_t curr_query_state ;

typedef enum {
    BOOT_STAGE_HANDSHAKE = 0,

    BOOT_STAGE_SENSOR,
    BOOT_STAGE_MOTOR,
    BOOT_STAGE_MAGNET,
    BOOT_STAGE_CONFIG,
    BOOT_STAGE_IMAGE,

    BOOT_STAGE_DONE,
    BOOT_STAGE_FAIL,
} boot_stage_t;

extern boot_stage_t g_boot_stage; //boot状态机

typedef enum {
    SELFTEST_SENSOR = 0x01,
    SELFTEST_MOTOR  = 0x02,
    SELFTEST_MAGNET = 0x03,
    SELFTEST_CONFIG = 0x04,
    SELFTEST_IMAGE  = 0x05,
} selftest_type_t; //自检命令枚举

void boot_send_next_selftest(void);
// CIS校准命令
#define CIS_Calib_cmd  0x01
#ifdef __cplusplus
}
#endif

#endif
