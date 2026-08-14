#ifndef PROTOCOL_FRAME_VALIDATOR_H
#define PROTOCOL_FRAME_VALIDATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool protocol_frame_is_valid(const uint8_t *data, size_t len);

#endif
