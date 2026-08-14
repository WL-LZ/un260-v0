#include "uart_io.h"

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

static pthread_mutex_t g_uart_tx_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_uart_log_mutex = PTHREAD_MUTEX_INITIALIZER;

int uart_write_all(int fd, const void *data, size_t data_len)
{
    const uint8_t *bytes = data;
    size_t written = 0;

    if (fd < 0 || (data == NULL && data_len > 0) || data_len > INT_MAX) {
        errno = EINVAL;
        return -1;
    }

    while (written < data_len) {
        ssize_t result = write(fd, bytes + written, data_len - written);

        if (result > 0) {
            written += (size_t)result;
            continue;
        }
        if (result < 0 && errno == EINTR) {
            continue;
        }

        if (result == 0) {
            errno = EIO;
        }
        return -1;
    }

    return (int)written;
}

int uart_send(int fd, const char *send_buf, int data_len)
{
    int result;

    if (data_len < 0) {
        errno = EINVAL;
        return -1;
    }

    pthread_mutex_lock(&g_uart_tx_mutex);
    result = uart_write_all(fd, send_buf, (size_t)data_len);
    pthread_mutex_unlock(&g_uart_tx_mutex);
    return result;
}

void uart_printf(int fd, const char *fmt, ...)
{
    char buf[256];
    va_list args;
    int formatted_len;
    size_t output_len;
    int write_result;

    if (fd < 0 || fmt == NULL) {
        return;
    }

    va_start(args, fmt);
    formatted_len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (formatted_len <= 0) {
        return;
    }

    output_len = (size_t)formatted_len;
    if (output_len >= sizeof(buf)) {
        output_len = sizeof(buf) - 1;
    }

    pthread_mutex_lock(&g_uart_log_mutex);
    write_result = uart_write_all(fd, buf, output_len);
    pthread_mutex_unlock(&g_uart_log_mutex);

    if (write_result != (int)output_len) {
        fprintf(stderr, "UART log write failed: %d/%zu\n", write_result, output_len);
    }
}
