#ifndef BASIC_SETTING_REPLY_DISPATCH_H
#define BASIC_SETTING_REPLY_DISPATCH_H

#include <stdint.h>

typedef enum {
    BASIC_SETTING_REPLY_ACTION_NONE = 0,
    BASIC_SETTING_REPLY_ACTION_SCHEDULE_MODE_CLEAR = 1 << 0,
} basic_setting_reply_action_t;

basic_setting_reply_action_t basic_setting_reply_dispatch(uint8_t cmd,
                                                           const uint8_t *buf,
                                                           uint8_t len);

#endif
