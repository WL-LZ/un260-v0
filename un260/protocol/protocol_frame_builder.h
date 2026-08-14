#ifndef PROTOCOL_FRAME_BUILDER_H
#define PROTOCOL_FRAME_BUILDER_H

#include <stddef.h>
#include <stdint.h>

int protocol_frame_build(uint8_t *output,
                         size_t output_capacity,
                         uint8_t cmd,
                         const uint8_t *payload,
                         uint16_t payload_len);

#endif
