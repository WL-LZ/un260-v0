#ifndef UN260_APP_SERVICE_APP_SETTING_RUNTIME_H
#define UN260_APP_SERVICE_APP_SETTING_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

bool app_setting_runtime_handle_reply(uint8_t cmd, uint8_t *buf, uint8_t len);
void app_setting_runtime_stop(void);

#endif
