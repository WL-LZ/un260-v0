#include "data_collection_reply.h"

#include <stdio.h>

#include "data_collection_state.h"

data_collection_reply_result_t data_collection_reply_handle(const uint8_t *buf,
                                                             uint8_t len)
{
    const char *status;
    char unknown_status[48];

    /* 4 字节帧头/命令 + 1 字节状态 + 1 字节 CRC。 */
    if (buf == NULL || len < 6) {
        return DATA_COLLECTION_REPLY_INVALID;
    }

    switch (buf[4]) {
    case 0x01:
        status = "ALL DATA collection mode ready.";
        break;
    case 0x02:
        status = "Collection completed. Data can be copied from USB.";
        break;
    case 0x03:
        status = "FALSE REPORT collection mode ready.";
        break;
    case 0x05:
        status = "USB ready. You can start counting.";
        break;
    case 0x06:
        status = "USB not ready. Collection mode error.";
        break;
    case 0xFF:
        data_collection_state_exit("Collection mode exited.");
        return DATA_COLLECTION_REPLY_EXITED;
    default:
        snprintf(unknown_status, sizeof(unknown_status),
                 "Collection reply: 0x%02X", buf[4]);
        data_collection_state_set_status(unknown_status);
        return DATA_COLLECTION_REPLY_UNKNOWN;
    }

    data_collection_state_set_status(status);
    return DATA_COLLECTION_REPLY_STATUS_UPDATED;
}
