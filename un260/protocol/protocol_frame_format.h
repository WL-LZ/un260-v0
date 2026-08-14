#ifndef PROTOCOL_FRAME_FORMAT_H
#define PROTOCOL_FRAME_FORMAT_H

#include <stddef.h>
#include <stdint.h>

size_t protocol_frame_format_hex(const uint8_t *data,
                                 size_t len,
                                 char *output,
                                 size_t output_size);

#endif
