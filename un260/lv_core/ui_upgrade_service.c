#include "ui_upgrade_service.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define UI_UPGRADE_USB_MNT             "/mnt/usb"
#define UI_UPGRADE_FILE_PATH           "/mnt/usb/update/test_lvgl"
#define UI_UPGRADE_SCRIPT_PATH         "/usr/bin/ui_update.sh"  
#define UI_UPGRADE_STATUS_FILE_PATH    "/tmp/ui_update.status"

typedef struct {
    bool running;
    bool finished;
    bool success;
    pid_t child_pid;
    unsigned long start_ms;
    unsigned long status_mtime_ms;
    ui_upgrade_service_status_t status;
} ui_upgrade_service_ctx_t;

static ui_upgrade_service_ctx_t g_ui_upgrade_service;

static unsigned long ui_upgrade_service_now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long)ts.tv_sec * 1000UL + (unsigned long)(ts.tv_nsec / 1000000UL);
}

static bool ui_upgrade_service_file_exists(const char* path)
{
    struct stat st;
    return (path != NULL) && (stat(path, &st) == 0) && S_ISREG(st.st_mode);
}

static bool ui_upgrade_service_usb_device_present(void)
{
    return (access("/dev/sda1", F_OK) == 0) || (access("/dev/sdb1", F_OK) == 0);
}

static const char* ui_upgrade_service_get_usb_device_path(void)
{
    if (access("/dev/sda1", F_OK) == 0) return "/dev/sda1";
    if (access("/dev/sdb1", F_OK) == 0) return "/dev/sdb1";
    return NULL;
}

static bool ui_upgrade_service_get_mounted_device(char* dev_path, size_t dev_path_size)
{
    FILE* fp = fopen("/proc/mounts", "r");
    char dev[128];
    char dir[128];
    char fstype[64];
    bool found = false;

    if (dev_path == NULL || dev_path_size == 0) return false;
    dev_path[0] = '\0';

    if (fp == NULL) return false;

    while (fscanf(fp, "%127s %127s %63s %*s %*d %*d\n", dev, dir, fstype) == 3) {
        if (strcmp(dir, UI_UPGRADE_USB_MNT) == 0) {
            snprintf(dev_path, dev_path_size, "%s", dev);
            found = true;
            break;
        }
    }

    fclose(fp);
    return found;
}

static bool ui_upgrade_service_usb_mounted(void)
{
    FILE* fp = fopen("/proc/mounts", "r");
    char dev[128];
    char dir[128];
    char fstype[64];
    bool mounted = false;

    if (fp == NULL) return false;

    while (fscanf(fp, "%127s %127s %63s %*s %*d %*d\n", dev, dir, fstype) == 3) {
        if (strcmp(dir, UI_UPGRADE_USB_MNT) == 0) {
            mounted = true;
            break;
        }
    }

    fclose(fp);
    return mounted;
}

static bool ui_upgrade_service_usb_mount_stale(void)
{
    char mounted_dev[128];
    const char* usb_dev;

    if (!ui_upgrade_service_get_mounted_device(mounted_dev, sizeof(mounted_dev))) {
        return false;
    }

    if (access(mounted_dev, F_OK) != 0) {
        return true;
    }

    usb_dev = ui_upgrade_service_get_usb_device_path();
    if (usb_dev == NULL) {
        return true;
    }

    return strcmp(mounted_dev, usb_dev) != 0;
}

static void ui_upgrade_service_try_umount_usb(void)
{
    if (!ui_upgrade_service_usb_mounted()) return;
    system("umount /mnt/usb 2>/dev/null");
}

static void ui_upgrade_service_try_mount_usb(void)
{
    const char* usb_dev;

    system("mkdir -p /mnt/usb");

    if (ui_upgrade_service_usb_mounted()) {
        if (!ui_upgrade_service_usb_mount_stale()) {
            return;
        }
        ui_upgrade_service_try_umount_usb();
    }

    usb_dev = ui_upgrade_service_get_usb_device_path();
    if (usb_dev == NULL) return;

    if (strcmp(usb_dev, "/dev/sda1") == 0) {
        system("mount /dev/sda1 /mnt/usb 2>/dev/null");
    } else {
        system("mount /dev/sdb1 /mnt/usb 2>/dev/null");
    }
}

static void ui_upgrade_service_set_status(bool running,
                                          bool finished,
                                          bool success,
                                          int progress,
                                          ui_upgrade_stage_t stage,
                                          const char* step_text,
                                          const char* result_text)
{
    g_ui_upgrade_service.status.running = running;
    g_ui_upgrade_service.status.finished = finished;
    g_ui_upgrade_service.status.success = success;
    g_ui_upgrade_service.status.progress = progress;
    g_ui_upgrade_service.status.stage = stage;

    snprintf(g_ui_upgrade_service.status.step_text,
             sizeof(g_ui_upgrade_service.status.step_text),
             "%s", step_text ? step_text : "");
    snprintf(g_ui_upgrade_service.status.result_text,
             sizeof(g_ui_upgrade_service.status.result_text),
             "%s", result_text ? result_text : "");
}

static ui_upgrade_stage_t ui_upgrade_service_stage_from_name(const char* stage_name)
{
    if (stage_name == NULL) return UI_UPGRADE_STAGE_NONE;
    if (strcmp(stage_name, "verify") == 0) return UI_UPGRADE_STAGE_VERIFY;
    if (strcmp(stage_name, "write") == 0) return UI_UPGRADE_STAGE_WRITE;
    if (strcmp(stage_name, "finish") == 0) return UI_UPGRADE_STAGE_FINISH;
    if (strcmp(stage_name, "success") == 0) return UI_UPGRADE_STAGE_SUCCESS;
    if (strcmp(stage_name, "fail") == 0) return UI_UPGRADE_STAGE_FAIL;
    return UI_UPGRADE_STAGE_NONE;
}

static void ui_upgrade_service_set_progress_by_time(void)
{
    unsigned long elapsed_ms = ui_upgrade_service_now_ms() - g_ui_upgrade_service.start_ms;
    int progress = 0;
    ui_upgrade_stage_t stage = UI_UPGRADE_STAGE_VERIFY;
    const char* step_text = "Verifying upgrade package";

    if (elapsed_ms < 1200UL) {
        progress = (int)((elapsed_ms * 16UL) / 1200UL);
        stage = UI_UPGRADE_STAGE_VERIFY;
        step_text = "Verifying upgrade package";
    } else if (elapsed_ms < 5200UL) {
        progress = 24 + (int)(((elapsed_ms - 1200UL) * 50UL) / 4000UL);
        stage = UI_UPGRADE_STAGE_WRITE;
        step_text = "Writing system files";
    } else if (elapsed_ms < 7000UL) {
        progress = 82 + (int)(((elapsed_ms - 5200UL) * 13UL) / 1800UL);
        stage = UI_UPGRADE_STAGE_FINISH;
        step_text = "Finalizing upgrade";
    } else {
        progress = 95;
        stage = UI_UPGRADE_STAGE_FINISH;
        step_text = "Finalizing upgrade";
    }

    if (progress > g_ui_upgrade_service.status.progress) {
        ui_upgrade_service_set_status(true, false, false, progress, stage, step_text, "");
    }
}

static void ui_upgrade_service_load_status_file(void)
{
    struct stat st;
    FILE* fp;
    char line[256];
    char step_text[64] = {0};
    char result_text[128] = {0};
    char stage_name[32] = {0};
    int progress = -1;
    int success = -1;

    if (stat(UI_UPGRADE_STATUS_FILE_PATH, &st) != 0) return;

    if (g_ui_upgrade_service.status_mtime_ms == (unsigned long)st.st_mtime * 1000UL) {
        return;
    }

    fp = fopen(UI_UPGRADE_STATUS_FILE_PATH, "r");
    if (fp == NULL) return;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "progress=", 9) == 0) {
            progress = atoi(line + 9);
        } else if (strncmp(line, "stage=", 6) == 0) {
            sscanf(line + 6, "%31s", stage_name);
        } else if (strncmp(line, "step=", 5) == 0) {
            sscanf(line + 5, "%63[^\n]", step_text);
        } else if (strncmp(line, "message=", 8) == 0) {
            sscanf(line + 8, "%127[^\n]", result_text);
        } else if (strncmp(line, "success=", 8) == 0) {
            success = atoi(line + 8);
        }
    }

    fclose(fp);

    g_ui_upgrade_service.status_mtime_ms = (unsigned long)st.st_mtime * 1000UL;

    if (progress >= 0) g_ui_upgrade_service.status.progress = progress;
    if (step_text[0] != '\0') {
        snprintf(g_ui_upgrade_service.status.step_text,
                 sizeof(g_ui_upgrade_service.status.step_text), "%s", step_text);
    }
    if (result_text[0] != '\0') {
        snprintf(g_ui_upgrade_service.status.result_text,
                 sizeof(g_ui_upgrade_service.status.result_text), "%s", result_text);
    }
    if (stage_name[0] != '\0') {
        g_ui_upgrade_service.status.stage = ui_upgrade_service_stage_from_name(stage_name);
    }

    if (success == 1) {
        ui_upgrade_service_set_status(false, true, true, 100,
                                      UI_UPGRADE_STAGE_SUCCESS,
                                      "Upgrade complete",
                                      result_text[0] ? result_text : "System upgrade completed successfully");
        g_ui_upgrade_service.running = false;
        g_ui_upgrade_service.finished = true;
        g_ui_upgrade_service.success = true;
    } else if (success == 0) {
        ui_upgrade_service_set_status(false, true, false, 12,
                                      UI_UPGRADE_STAGE_FAIL,
                                      "Upgrade package verification failed",
                                      result_text[0] ? result_text : "The upgrade package is invalid. Please check the file and try again.");
        g_ui_upgrade_service.running = false;
        g_ui_upgrade_service.finished = true;
        g_ui_upgrade_service.success = false;
    }
}

static void ui_upgrade_service_update_child_state(void)
{
    int status = 0;
    pid_t ret;

    if (g_ui_upgrade_service.child_pid <= 0) return;

    ret = waitpid(g_ui_upgrade_service.child_pid, &status, WNOHANG);
    if (ret == 0) return;
    if (ret < 0) return;

    g_ui_upgrade_service.child_pid = -1;
    g_ui_upgrade_service.running = false;
    g_ui_upgrade_service.finished = true;

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        g_ui_upgrade_service.success = true;
        ui_upgrade_service_set_status(false, true, true, 100,
                                      UI_UPGRADE_STAGE_SUCCESS,
                                      "Upgrade complete",
                                      "The system has been updated successfully. Rebooting the device is recommended.");
    } else {
        g_ui_upgrade_service.success = false;
        ui_upgrade_service_set_status(false, true, false, 12,
                                      UI_UPGRADE_STAGE_FAIL,
                                      "Upgrade package verification failed",
                                      "The upgrade package is invalid. Please check the file and try again.");
    }
}

void ui_upgrade_service_reset(void)
{
    memset(&g_ui_upgrade_service, 0, sizeof(g_ui_upgrade_service));
    g_ui_upgrade_service.child_pid = -1;
    ui_upgrade_service_set_status(false, false, false, 0,
                                  UI_UPGRADE_STAGE_NONE, "", "");
}

void ui_upgrade_service_detect(ui_upgrade_detect_info_t* info)
{
    if (info == NULL) return;

    info->usb_present = ui_upgrade_service_usb_device_present();
    if (info->usb_present) {
        ui_upgrade_service_try_mount_usb();
    } else {
        ui_upgrade_service_try_umount_usb();
    }
    info->usb_mounted = ui_upgrade_service_usb_mounted();
    info->package_found = ui_upgrade_service_file_exists(UI_UPGRADE_FILE_PATH);
}

int ui_upgrade_service_start(void)
{
    pid_t pid;

    if (g_ui_upgrade_service.running) return -1;
    if (!ui_upgrade_service_file_exists(UI_UPGRADE_SCRIPT_PATH)) return -1;

    unlink(UI_UPGRADE_STATUS_FILE_PATH);

    pid = fork();
    if (pid < 0) return -1;

    if (pid == 0) {
        FILE* fp = fopen("/tmp/ui_update_child.log", "a");
        if (fp) {
            dup2(fileno(fp), STDOUT_FILENO);
            dup2(fileno(fp), STDERR_FILENO);
        }
        execl("/bin/sh", "sh", UI_UPGRADE_SCRIPT_PATH, (char*)NULL);
        _exit(127);
    }

    g_ui_upgrade_service.running = true;
    g_ui_upgrade_service.finished = false;
    g_ui_upgrade_service.success = false;
    g_ui_upgrade_service.child_pid = pid;
    g_ui_upgrade_service.start_ms = ui_upgrade_service_now_ms();
    g_ui_upgrade_service.status_mtime_ms = 0;

    ui_upgrade_service_set_status(true, false, false, 0,
                                  UI_UPGRADE_STAGE_VERIFY,
                                  "Verifying upgrade package", "");
    return 0;
}

void ui_upgrade_service_poll(ui_upgrade_service_status_t* status)
{
    if (g_ui_upgrade_service.running) {
        ui_upgrade_service_set_progress_by_time();
        ui_upgrade_service_load_status_file();
        ui_upgrade_service_update_child_state();
    }

    if (status != NULL) {
        *status = g_ui_upgrade_service.status;
    }
}

void ui_upgrade_service_reboot(void)
{
    sync();
    system("reboot");
}
