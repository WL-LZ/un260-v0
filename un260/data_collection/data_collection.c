#include "data_collection.h"

#include <stdio.h>

#define DATA_COLLECTION_STATUS_MAX 128

static data_collect_mode_t g_mode = DATA_COLLECT_MODE_NONE;
static uint16_t g_pcs;
static char g_status[DATA_COLLECTION_STATUS_MAX] = "Please select a collection mode.";

data_collect_mode_t data_collection_state_mode(void)
{
    return g_mode;
}

uint16_t data_collection_state_pcs(void)
{
    return g_pcs;
}

const char *data_collection_state_status(void)
{
    return g_status;
}

void data_collection_state_set_status(const char *status)
{
    snprintf(g_status, sizeof(g_status), "%s", status != NULL ? status : "");
}

void data_collection_state_select_mode(data_collect_mode_t mode, const char *status)
{
    if (mode != DATA_COLLECT_MODE_ALL && mode != DATA_COLLECT_MODE_FALSE) return;
    g_mode = mode;
    g_pcs = 0;
    data_collection_state_set_status(status);
}

void data_collection_state_reset_pcs(void)
{
    g_pcs = 0;
}

void data_collection_state_exit(const char *status)
{
    g_mode = DATA_COLLECT_MODE_NONE;
    g_pcs = 0;
    data_collection_state_set_status(status);
}

data_collection_reply_result_t data_collection_reply_handle(const uint8_t *buf, uint8_t len)
{
    const char *status;
    char unknown_status[48];

    /* 4 字节帧头/命令 + 1 字节状态 + 1 字节 CRC。 */
    if (buf == NULL || len < 6) return DATA_COLLECTION_REPLY_INVALID;

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
        snprintf(unknown_status, sizeof(unknown_status), "Collection reply: 0x%02X", buf[4]);
        data_collection_state_set_status(unknown_status);
        return DATA_COLLECTION_REPLY_UNKNOWN;
    }

    data_collection_state_set_status(status);
    return DATA_COLLECTION_REPLY_STATUS_UPDATED;
}
