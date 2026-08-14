#ifndef DATA_COLLECTION_STATE_H
#define DATA_COLLECTION_STATE_H

#include <stdint.h>

typedef enum {
    DATA_COLLECT_MODE_NONE = 0,
    DATA_COLLECT_MODE_ALL = 0x01,
    DATA_COLLECT_MODE_FALSE = 0x02,
} data_collect_mode_t;

data_collect_mode_t data_collection_state_mode(void);
uint16_t data_collection_state_pcs(void);
const char *data_collection_state_status(void);

void data_collection_state_select_mode(data_collect_mode_t mode, const char *status);
void data_collection_state_reset_pcs(void);
void data_collection_state_set_status(const char *status);
void data_collection_state_exit(const char *status);

#endif
