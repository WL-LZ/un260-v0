#ifndef __BOOT_STATE_H__
#define __BOOT_STATE_H__

typedef enum {
    HANDSHAKE_IDLE = 0,
    HANDSHAKE_SENT,
    HANDSHAKE_OK,
} handshake_state_t;

#endif
