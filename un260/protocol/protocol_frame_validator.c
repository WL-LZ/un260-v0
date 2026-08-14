#include "protocol_frame_validator.h"

#include "protocol_frame.h"

bool protocol_frame_is_valid(const uint8_t *data, size_t len)
{
    if (data == NULL || len < PROTOCOL_FRAME_MIN_SIZE ||
        len > PROTOCOL_FRAME_MAX_SIZE) {
        return false;
    }
    if (data[0] != PROTOCOL_FRAME_HEADER_FIRST ||
        data[1] != PROTOCOL_FRAME_HEADER_SECOND) {
        return false;
    }
    if ((size_t)data[2] != len) {
        return false;
    }
    return data[len - 1U] == PROTOCOL_FRAME_TRAILER;
}
