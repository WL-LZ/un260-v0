#ifndef UN260_LV_DRIVERS_UART_IO_H
#define UN260_LV_DRIVERS_UART_IO_H

#include <stddef.h>
#include <stdint.h>

int uart_write_all(int fd, const void *data, size_t data_len);
int uart_send(int fd, const char *send_buf, int data_len);
void uart_printf(int fd, const char *fmt, ...);
void uart_log_hex(int fd,
                  const char *prefix,
                  const uint8_t *data,
                  size_t data_len,
                  size_t preview_limit);

#endif
