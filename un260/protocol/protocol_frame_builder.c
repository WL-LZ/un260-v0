#include "protocol_frame_builder.h"

#include <string.h>

#include "protocol_frame.h"

int protocol_frame_build(uint8_t *output,
                         size_t output_capacity,
                         uint8_t cmd,
                         const uint8_t *payload,
                         uint16_t payload_len)
{
    size_t frame_len;

    if (output == NULL || payload_len > PROTOCOL_FRAME_MAX_PAYLOAD) {
        return -1;
    }
    if (payload_len > 0 && payload == NULL) {
        return -1;
    }

    frame_len = (size_t)payload_len + PROTOCOL_FRAME_OVERHEAD;
    if (output_capacity < frame_len) {
        return -1;
    }

    output[0] = PROTOCOL_FRAME_HEADER_FIRST;
    output[1] = PROTOCOL_FRAME_HEADER_SECOND;
    output[2] = (uint8_t)frame_len;
    output[3] = cmd;
    if (payload_len > 0) {
        memcpy(&output[PROTOCOL_FRAME_PAYLOAD_OFFSET], payload, payload_len);
    }
    output[frame_len - 1] = PROTOCOL_FRAME_TRAILER;

    return (int)frame_len;
}
