#ifndef COUNTING_DATA_STORE_H
#define COUNTING_DATA_STORE_H

#include <stdbool.h>

#include "counting_data_types.h"

void counting_data_clear_serials(counting_sim_t *sim_data);
bool counting_data_ensure_serial_capacity(counting_sim_t *sim_data,
                                          int required_count);
int counting_data_serial_scan_limit(const counting_sim_t *sim_data);
void counting_data_clear_errors(counting_sim_t *sim_data);
bool counting_data_ensure_error_capacity(counting_sim_t *sim_data,
                                         int required_count);

#endif
