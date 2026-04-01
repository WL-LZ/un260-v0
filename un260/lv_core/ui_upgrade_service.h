#ifndef UI_UPGRADE_SERVICE_H
#define UI_UPGRADE_SERVICE_H

#include <stdbool.h>

typedef enum {
    UI_UPGRADE_STAGE_NONE = 0,
    UI_UPGRADE_STAGE_VERIFY,
    UI_UPGRADE_STAGE_WRITE,
    UI_UPGRADE_STAGE_FINISH,
    UI_UPGRADE_STAGE_SUCCESS,
    UI_UPGRADE_STAGE_FAIL
} ui_upgrade_stage_t;

typedef struct {
    bool usb_present;
    bool usb_mounted;
    bool package_found;
} ui_upgrade_detect_info_t;

typedef struct {
    bool running;
    bool finished;
    bool success;
    int progress;
    ui_upgrade_stage_t stage;
    char step_text[64];
    char result_text[128];
} ui_upgrade_service_status_t;

void ui_upgrade_service_reset(void);
void ui_upgrade_service_detect(ui_upgrade_detect_info_t* info);
int ui_upgrade_service_start(void);
void ui_upgrade_service_poll(ui_upgrade_service_status_t* status);
void ui_upgrade_service_reboot(void);

#endif
