#ifndef UN260_APP_SERVICE_APP_COMMAND_RUNTIME_H
#define UN260_APP_SERVICE_APP_COMMAND_RUNTIME_H

#include <stdint.h>

void app_command_runtime_process_frames(void);
void app_command_runtime_poll(uint32_t now_ms);

#endif
