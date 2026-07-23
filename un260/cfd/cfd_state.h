#ifndef CFD_STATE_H
#define CFD_STATE_H

#include "un260/lv_system/user_cfg.h"

#include <stdint.h>

typedef struct {
    char currency[4];
    uint8_t levels[CFD_SCENE_COUNT][CFD_ITEM_COUNT];
} cfd_state_value_t;

void cfd_state_get(cfd_state_value_t *value);
void cfd_state_confirm(const cfd_state_value_t *value);

#endif
