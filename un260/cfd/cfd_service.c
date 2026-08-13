#include "cfd_service.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define CFD_QUERY_TIMEOUT_MS 800U

static bool g_cfd_query_pending = false;
static char g_cfd_query_currency[4] = "";
static uint64_t g_cfd_query_tick_ms = 0;

static uint64_t cfd_service_now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

static bool cfd_service_query_expired(void)
{
    return g_cfd_query_pending &&
           cfd_service_now_ms() - g_cfd_query_tick_ms >= CFD_QUERY_TIMEOUT_MS;
}

static void cfd_service_copy_currency(char dst[4], const char *src)
{
    size_t len = 0;

    if (src) {
        while (len < 3 && src[len] != '\0') {
            dst[len] = src[len];
            len++;
        }
    }
    dst[len] = '\0';
}

bool cfd_service_request_query(const char currency[4])
{
    if (!currency || currency[0] == '\0') return false;
    if (g_cfd_query_pending) return false;

    cfd_service_copy_currency(g_cfd_query_currency, currency);
    g_cfd_query_pending = true;
    g_cfd_query_tick_ms = cfd_service_now_ms();
    return true;
}

void cfd_service_cancel_query(void)
{
    g_cfd_query_pending = false;
    g_cfd_query_currency[0] = '\0';
    g_cfd_query_tick_ms = 0;
}

bool cfd_service_take_query_result(const char currency[4])
{
    if (cfd_service_query_expired()) return false;
    if (!g_cfd_query_pending) return false;
    if (!currency || strncmp(currency, g_cfd_query_currency, 3) != 0) return false;
    cfd_service_cancel_query();
    return true;
}

bool cfd_service_take_query_timeout(void)
{
    if (!cfd_service_query_expired()) return false;
    cfd_service_cancel_query();
    return true;
}
