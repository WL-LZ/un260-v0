#ifndef DEVICE_REPLY_H
#define DEVICE_REPLY_H

#include <stdint.h>

typedef enum {
    DEVICE_REPLY_INVALID = 0,
    DEVICE_REPLY_VERSION_UPDATED,
    DEVICE_REPLY_MAIN_UPGRADE_STATUS,
    DEVICE_REPLY_IMAGE_UPGRADE_STATUS,
} device_reply_kind_t;

typedef struct {
    device_reply_kind_t kind;
    uint8_t status;
} device_reply_result_t;

device_reply_result_t device_reply_dispatch(uint8_t cmd,
                                            const uint8_t *buf,
                                            uint8_t len);

#endif
