#ifndef SETTING_REPLY_DISPATCH_H
#define SETTING_REPLY_DISPATCH_H

#include <stdbool.h>
#include <stdint.h>

bool setting_reply_dispatch_detail(uint8_t cmd, const uint8_t *buf, uint8_t len);

#endif
