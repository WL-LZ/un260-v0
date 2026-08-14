#include "uart_bridge_service.h"

#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#include "lv_drivers.h"

#define UART_BRIDGE_BUFFER_SIZE 256

static pthread_mutex_t g_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t g_thread;
static bool g_started;
static bool g_running;
static int g_source_fd = -1;
static int g_target_fd = -1;
static int g_log_fd = -1;

static bool uart_bridge_service_should_run(void)
{
    bool running;

    pthread_mutex_lock(&g_state_mutex);
    running = g_running;
    pthread_mutex_unlock(&g_state_mutex);
    return running;
}

static void *uart_bridge_service_thread(void *arg)
{
    uint8_t buffer[UART_BRIDGE_BUFFER_SIZE];

    (void)arg;
    uart_printf(g_log_fd, "UART5 start\n");

    while (uart_bridge_service_should_run()) {
        int len = uart_recv(g_source_fd, (char *)buffer, sizeof(buffer), 100);

        if (len > 0) {
            int sent;

            uart_printf(g_log_fd, "UART5 recive %d 字节: ", len);
            for (int i = 0; i < len; i++) {
                uart_printf(g_log_fd, "%02X ", buffer[i]);
            }
            uart_printf(g_log_fd, "\n");

            sent = uart_send(g_target_fd, (const char *)buffer, len);
            uart_printf(g_log_fd, "UART5: sent UART4，长度=%d\n", sent);
        }
        usleep(1000);
    }

    uart_printf(g_log_fd, "UART5 end\n");
    return NULL;
}

bool uart_bridge_service_start(int source_fd, int target_fd, int log_fd)
{
    int create_result;

    if (source_fd < 0 || target_fd < 0 || log_fd < 0) {
        return false;
    }

    pthread_mutex_lock(&g_state_mutex);
    if (g_started) {
        pthread_mutex_unlock(&g_state_mutex);
        return false;
    }

    g_source_fd = source_fd;
    g_target_fd = target_fd;
    g_log_fd = log_fd;
    g_running = true;
    create_result = pthread_create(&g_thread, NULL, uart_bridge_service_thread, NULL);
    if (create_result != 0) {
        g_running = false;
        g_source_fd = -1;
        g_target_fd = -1;
        g_log_fd = -1;
        pthread_mutex_unlock(&g_state_mutex);
        return false;
    }
    g_started = true;
    pthread_mutex_unlock(&g_state_mutex);

    return true;
}

void uart_bridge_service_stop(void)
{
    pthread_t thread;

    pthread_mutex_lock(&g_state_mutex);
    if (!g_started) {
        pthread_mutex_unlock(&g_state_mutex);
        return;
    }
    g_running = false;
    thread = g_thread;
    pthread_mutex_unlock(&g_state_mutex);

    pthread_join(thread, NULL);

    pthread_mutex_lock(&g_state_mutex);
    g_started = false;
    g_source_fd = -1;
    g_target_fd = -1;
    g_log_fd = -1;
    pthread_mutex_unlock(&g_state_mutex);
}
