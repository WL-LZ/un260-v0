#include "cfd_state.h"

#include <stddef.h>
#include <string.h>

static cfd_state_value_t g_cfd_state = {
    .currency = "CNY",
    .levels = {
        { 3, 3, 3, 3 },
        { 3, 3, 3, 3 },
        { 3, 3, 3, 3 },
    },
};

static void cfd_state_copy_currency(char dst[4], const char *src)
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

void cfd_state_get(cfd_state_value_t *value)
{
    if (!value) return;
    *value = g_cfd_state;
}

void cfd_state_confirm(const cfd_state_value_t *value)
{
    if (!value) return;

    cfd_state_copy_currency(g_cfd_state.currency, value->currency);
    memcpy(g_cfd_state.levels, value->levels, sizeof(g_cfd_state.levels));
}
