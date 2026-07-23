#ifndef PRINT_CONFIG_H
#define PRINT_CONFIG_H

#include "un260/lv_system/user_cfg.h"

#include <stdint.h>

typedef struct {
    uint8_t space_top;
    char head1[PRINT_SETTING_HEAD_MAX_LEN + 1];
    char head2[PRINT_SETTING_HEAD_MAX_LEN + 1];
    uint8_t content;
    uint8_t space_bottom;
} print_config_value_t;

void print_config_get(print_config_value_t *value);
void print_config_confirm(const print_config_value_t *value);

#endif
