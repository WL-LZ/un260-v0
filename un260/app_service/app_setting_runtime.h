#ifndef UN260_APP_SERVICE_APP_SETTING_RUNTIME_H
#define UN260_APP_SERVICE_APP_SETTING_RUNTIME_H

#include <stdint.h>

void app_setting_runtime_handle_basic_reply(uint8_t cmd,
                                            uint8_t *buf,
                                            uint8_t len);
void app_setting_runtime_stop(void);

#endif
