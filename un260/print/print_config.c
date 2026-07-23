#include "print_config.h"

#include <stddef.h>

static print_config_value_t g_print_config = {
    .space_top = 0,
    .head1 = "",
    .head2 = "",
    .content = PRINT_SETTING_CONTENT_LIST,
    .space_bottom = 0,
};

static void print_config_copy_head(char dst[PRINT_SETTING_HEAD_MAX_LEN + 1], const char *src)
{
    size_t len = 0;

    if (src) {
        while (len < PRINT_SETTING_HEAD_MAX_LEN && src[len] != '\0') {
            dst[len] = src[len];
            len++;
        }
    }
    dst[len] = '\0';
}

void print_config_get(print_config_value_t *value)
{
    if (!value) return;
    *value = g_print_config;
}

void print_config_confirm(const print_config_value_t *value)
{
    if (!value) return;

    g_print_config.space_top = value->space_top;
    print_config_copy_head(g_print_config.head1, value->head1);
    print_config_copy_head(g_print_config.head2, value->head2);
    g_print_config.content = value->content;
    g_print_config.space_bottom = value->space_bottom;
}
