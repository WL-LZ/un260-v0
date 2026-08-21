#include "usb_storage.h"

#include <errno.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static bool usb_storage_find_device(char* path, size_t path_size)
{
    static const char* patterns[] = {
        "/dev/sd[a-z][0-9]*",
        "/dev/sd[a-z]"
    };
    size_t i;

    if (path == NULL || path_size == 0U) return false;
    path[0] = '\0';

    for (i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
        glob_t matches;
        size_t j;

        memset(&matches, 0, sizeof(matches));
        if (glob(patterns[i], 0, NULL, &matches) != 0) {
            globfree(&matches);
            continue;
        }

        for (j = 0; j < matches.gl_pathc; j++) {
            const char* device = matches.gl_pathv[j];
            struct stat st;

            if (device == NULL || stat(device, &st) != 0 ||
                !S_ISBLK(st.st_mode)) {
                continue;
            }

            snprintf(path, path_size, "%s", device);
            globfree(&matches);
            return true;
        }

        globfree(&matches);
    }

    return false;
}

static bool usb_storage_get_mounted_device(char* path, size_t path_size)
{
    FILE* fp;
    char device[128];
    char mount_path[128];
    char fs_type[64];
    bool found = false;

    if (path == NULL || path_size == 0U) return false;
    path[0] = '\0';

    fp = fopen("/proc/mounts", "r");
    if (fp == NULL) return false;

    while (fscanf(fp, "%127s %127s %63s %*s %*d %*d\n",
                  device, mount_path, fs_type) == 3) {
        if (strcmp(mount_path, USB_STORAGE_MOUNT_POINT) == 0) {
            snprintf(path, path_size, "%s", device);
            found = true;
            break;
        }
    }

    fclose(fp);
    return found;
}

static bool usb_storage_is_mounted(void)
{
    char device[128];

    return usb_storage_get_mounted_device(device, sizeof(device));
}

static bool usb_storage_prepare_mount_point(void)
{
    struct stat st;

    if (stat(USB_STORAGE_MOUNT_POINT, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    if (errno != ENOENT) return false;
    return mkdir(USB_STORAGE_MOUNT_POINT, 0755) == 0;
}

static bool usb_storage_unmount(void)
{
    if (!usb_storage_is_mounted()) return true;
    if (system("umount " USB_STORAGE_MOUNT_POINT " 2>/dev/null") != 0) {
        return false;
    }
    return !usb_storage_is_mounted();
}

static bool usb_storage_mount_device(const char* device)
{
    char mounted_device[128];
    char command[192];

    if (device == NULL || device[0] == '\0') return false;

    if (usb_storage_get_mounted_device(mounted_device,
                                       sizeof(mounted_device))) {
        if (access(mounted_device, F_OK) == 0) return true;
        if (!usb_storage_unmount()) return false;
    }
    if (!usb_storage_prepare_mount_point()) return false;

    snprintf(command, sizeof(command), "mount '%s' %s 2>/dev/null",
             device, USB_STORAGE_MOUNT_POINT);
    if (system(command) != 0) return false;
    return usb_storage_is_mounted();
}

void usb_storage_refresh(usb_storage_status_t* status)
{
    char device[128];
    bool present;
    bool mounted;

    if (status == NULL) return;

    present = usb_storage_find_device(device, sizeof(device));
    if (present) {
        mounted = usb_storage_mount_device(device);
    } else {
        (void)usb_storage_unmount();
        mounted = usb_storage_is_mounted();
    }

    status->device_present = present;
    status->mounted = mounted;
}

bool usb_storage_prepare(void)
{
    char device[128];

    if (!usb_storage_find_device(device, sizeof(device))) return false;
    return usb_storage_mount_device(device);
}

bool usb_storage_available(void)
{
    char mounted_device[128];
    char device[128];

    if (usb_storage_get_mounted_device(mounted_device,
                                       sizeof(mounted_device)) &&
        access(mounted_device, F_OK) == 0) {
        return true;
    }
    return usb_storage_find_device(device, sizeof(device));
}
