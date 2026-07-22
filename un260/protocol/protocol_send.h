#ifndef PROTOCOL_SEND_H
#define PROTOCOL_SEND_H

#include <stdint.h>

int protocol_send(uint8_t cmd_g, const uint8_t *cmd_s, uint16_t cmd_s_len);

#endif
