#ifndef __BOOT_STATE_H__
#define __BOOT_STATE_H__

typedef enum {
    HANDSHAKE_IDLE = 0,
    HANDSHAKE_SENT,
    HANDSHAKE_OK,
} handshake_state_t;

typedef enum {
    SELFTEST_NONE = 0,
    SELFTEST_SENSOR = 0x01,
    SELFTEST_MOTOR  = 0x02,
    SELFTEST_MAGNET = 0x03,
    SELFTEST_CONFIG = 0x04,
    SELFTEST_IMAGE  = 0x05,
} selftest_type_t;

#endif
