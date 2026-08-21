#ifndef USB_STORAGE_H
#define USB_STORAGE_H

#include <stdbool.h>

#define USB_STORAGE_MOUNT_POINT "/mnt/usb"

typedef struct {
    bool device_present;
    bool mounted;
} usb_storage_status_t;

void usb_storage_refresh(usb_storage_status_t* status);
bool usb_storage_prepare(void);
bool usb_storage_available(void);

#endif
