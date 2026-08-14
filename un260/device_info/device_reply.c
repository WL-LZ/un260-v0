#include "device_reply.h"

#include <string.h>

#include "device_info.h"

static device_reply_result_t device_reply_version_handle(const uint8_t *buf,
                                                         uint8_t len)
{
    device_reply_result_t reply;
    const uint8_t *payload;
    device_info_remote_versions_t versions;

    memset(&reply, 0, sizeof(reply));
    reply.kind = DEVICE_REPLY_INVALID;
    /* 4 字节帧头/命令 + 14 字节版本载荷 + 1 字节 CRC。 */
    if (buf == NULL || len < 19) {
        return reply;
    }

    payload = &buf[4];
    versions = (device_info_remote_versions_t){
        .main_app = {payload[0], payload[1], payload[2]},
        .image_app = {payload[3], payload[4], payload[5]},
        .fpga = {payload[6], payload[7]},
        .main_boot = {payload[8], payload[9], payload[10]},
        .image_boot = {payload[11], payload[12], payload[13]},
    };
    device_info_confirm_remote_versions(&versions);
    reply.kind = DEVICE_REPLY_VERSION_UPDATED;
    return reply;
}

static device_reply_result_t device_reply_upgrade_handle(uint8_t cmd,
                                                         const uint8_t *buf,
                                                         uint8_t len)
{
    device_reply_result_t reply;

    memset(&reply, 0, sizeof(reply));
    reply.kind = DEVICE_REPLY_INVALID;
    if (buf == NULL || len < 6) {
        return reply;
    }

    reply.status = buf[4];
    reply.kind = cmd == 0xA1 ? DEVICE_REPLY_MAIN_UPGRADE_STATUS
                             : DEVICE_REPLY_IMAGE_UPGRADE_STATUS;
    return reply;
}

device_reply_result_t device_reply_dispatch(uint8_t cmd,
                                            const uint8_t *buf,
                                            uint8_t len)
{
    device_reply_result_t reply;

    switch (cmd) {
    case 0x17:
        return device_reply_version_handle(buf, len);
    case 0xA1:
    case 0xB0:
        return device_reply_upgrade_handle(cmd, buf, len);
    default:
        memset(&reply, 0, sizeof(reply));
        reply.kind = DEVICE_REPLY_INVALID;
        return reply;
    }
}
