#include "cfd_service.h"
#include "un260/protocol/protocol_request.h"

#include <stddef.h>
#include <string.h>

#define CFD_QUERY_TIMEOUT_MS 800U

static protocol_request_t g_cfd_query_request = PROTOCOL_REQUEST_INITIALIZER(CFD_QUERY_TIMEOUT_MS);
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
    if (!currency || currency[0] == '\0') return false;
    if (!protocol_request_begin(&g_cfd_query_request)) return false;

    cfd_service_copy_currency(g_cfd_query_currency, currency);
    return true;
}

void cfd_service_cancel_query(void)
{
    protocol_request_finish(&g_cfd_query_request);
    g_cfd_query_currency[0] = '\0';
}

bool cfd_service_take_query_result(const char currency[4])
{
    if (!protocol_request_can_take_result(&g_cfd_query_request)) return false;
    if (!currency || strncmp(currency, g_cfd_query_currency, 3) != 0) return false;
    cfd_service_cancel_query();
    return true;
}

bool cfd_service_take_query_timeout(void)
{
    if (!protocol_request_take_timeout(&g_cfd_query_request)) return false;
    g_cfd_query_currency[0] = '\0';
    return true;
}
