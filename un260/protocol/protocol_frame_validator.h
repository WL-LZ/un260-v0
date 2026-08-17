#ifndef PROTOCOL_FRAME_VALIDATOR_H
#define PROTOCOL_FRAME_VALIDATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Validate the common receive-frame structure only.  The local command
 * builder appends PROTOCOL_FRAME_TRAILER, but replies from the controller
 * are framed by their declared length and do not guarantee that value in
 * the final byte.
 */
bool protocol_frame_is_valid(const uint8_t *data, size_t len);

#endif
