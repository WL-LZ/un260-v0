#include "cfd_service.h"

#include <stddef.h>

static bool g_cfd_query_pending = false;
static char g_cfd_query_currency[4] = "";

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
    if (g_cfd_query_pending) return false;

    cfd_service_copy_currency(g_cfd_query_currency, currency);
    g_cfd_query_pending = true;
    return true;
}

void cfd_service_cancel_query(void)
{
    g_cfd_query_pending = false;
}

bool cfd_service_take_query_result(void)
{
    if (!g_cfd_query_pending) return false;
    g_cfd_query_pending = false;
    return true;
}
