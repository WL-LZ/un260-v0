#ifndef DATA_COLLECTION_REPLY_H
#define DATA_COLLECTION_REPLY_H

#include <stdint.h>

typedef enum {
    DATA_COLLECTION_REPLY_INVALID = 0,
    DATA_COLLECTION_REPLY_STATUS_UPDATED,
    DATA_COLLECTION_REPLY_EXITED,
    DATA_COLLECTION_REPLY_UNKNOWN,
} data_collection_reply_result_t;

data_collection_reply_result_t data_collection_reply_handle(const uint8_t *buf,
                                                             uint8_t len);

#endif
