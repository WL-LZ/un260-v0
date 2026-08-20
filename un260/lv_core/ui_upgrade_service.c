#include "ui_upgrade_service.h"

#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

#include "un260/app_service/app_clock.h"

#define UI_UPGRADE_USB_MNT             "/mnt/usb"
#define UI_UPGRADE_FILE_PATH           "/mnt/usb/update/test_lvgl"
#define UI_UPGRADE_RUNNING_FILE_PATH   "/proc/self/exe"
#define UI_UPGRADE_SCRIPT_PATH         "/usr/bin/ui_update.sh"  
#define UI_UPGRADE_STATUS_FILE_PATH    "/tmp/ui_update.status"

typedef struct {
    bool valid;
    dev_t device;
    ino_t inode;
    off_t size;
    time_t mtime;
    time_t ctime;
    uint64_t hash;
} ui_upgrade_hash_cache_t;

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
static bool g_ui_upgrade_running_hash_ready = false;
static uint64_t g_ui_upgrade_running_hash = 0;
static ui_upgrade_hash_cache_t g_ui_upgrade_pkg_hash_cache;

static unsigned long ui_upgrade_service_now_ms(void)
{
    return (unsigned long)app_clock_monotonic_ms();
}

static bool ui_upgrade_service_file_exists(const char* path)
{
    struct stat st;
    return (path != NULL) && (stat(path, &st) == 0) && S_ISREG(st.st_mode);
}

static bool ui_upgrade_service_hash_file_fnv1a64(const char* path, uint64_t* hash_out)
{
    FILE* fp;
    size_t read_len;
    unsigned char buf[4096];
    uint64_t hash = 1469598103934665603ULL;

    if (path == NULL || hash_out == NULL) {
        return false;
    }

    fp = fopen(path, "rb");
    if (fp == NULL) {
        return false;
    }

    while ((read_len = fread(buf, 1, sizeof(buf), fp)) > 0) {
        size_t i;
        for (i = 0; i < read_len; i++) {
            hash ^= (uint64_t)buf[i];
            hash *= 1099511628211ULL;
        }
    }

    if (ferror(fp) != 0) {
        fclose(fp);
        return false;
    }

    fclose(fp);
    *hash_out = hash;
    return true;
}

static void ui_upgrade_service_hash_cache_clear(ui_upgrade_hash_cache_t* cache)
{
    if (cache != NULL) {
        memset(cache, 0, sizeof(*cache));
    }
}

static bool ui_upgrade_service_hash_file_cached(const char* path,
                                                ui_upgrade_hash_cache_t* cache,
                                                uint64_t* hash_out)
{
    struct stat before;
    struct stat after;
    uint64_t hash = 0;

    if (path == NULL || cache == NULL || hash_out == NULL) {
        return false;
    }

    if (stat(path, &before) != 0 || !S_ISREG(before.st_mode)) {
        ui_upgrade_service_hash_cache_clear(cache);
        return false;
    }

    if (cache->valid &&
        cache->device == before.st_dev &&
        cache->inode == before.st_ino &&
        cache->size == before.st_size &&
        cache->mtime == before.st_mtime &&
        cache->ctime == before.st_ctime) {
        *hash_out = cache->hash;
        return true;
    }

    if (!ui_upgrade_service_hash_file_fnv1a64(path, &hash)) {
        ui_upgrade_service_hash_cache_clear(cache);
        return false;
    }
    if (stat(path, &after) != 0 || !S_ISREG(after.st_mode) ||
        before.st_dev != after.st_dev ||
        before.st_ino != after.st_ino ||
        before.st_size != after.st_size ||
        before.st_mtime != after.st_mtime ||
        before.st_ctime != after.st_ctime) {
        ui_upgrade_service_hash_cache_clear(cache);
        return false;
    }

    cache->valid = true;
    cache->device = after.st_dev;
    cache->inode = after.st_ino;
    cache->size = after.st_size;
    cache->mtime = after.st_mtime;
    cache->ctime = after.st_ctime;
    cache->hash = hash;
    *hash_out = hash;
    return true;
}

static bool ui_upgrade_service_package_hash_match_running(void)
{
    uint64_t package_hash = 0;

    if (!g_ui_upgrade_running_hash_ready) {
        if (!ui_upgrade_service_hash_file_fnv1a64(UI_UPGRADE_RUNNING_FILE_PATH,
                                                  &g_ui_upgrade_running_hash)) {
            return false;
        }
        g_ui_upgrade_running_hash_ready = true;
    }

    if (!ui_upgrade_service_hash_file_cached(UI_UPGRADE_FILE_PATH,
                                             &g_ui_upgrade_pkg_hash_cache,
                                             &package_hash)) {
        return false;
    }

    return package_hash == g_ui_upgrade_running_hash;
}

static bool ui_upgrade_service_find_usb_device_path(char* path, size_t path_size)
{
    static const char* patterns[] = {
        "/dev/sd[a-z][0-9]*",
        "/dev/sd[a-z]"
    };
    size_t i;

    if (path == NULL || path_size == 0) return false;
    path[0] = '\0';

    for (i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
        glob_t glob_buf;
        size_t j;

        memset(&glob_buf, 0, sizeof(glob_buf));
        if (glob(patterns[i], 0, NULL, &glob_buf) != 0) {
            globfree(&glob_buf);
            continue;
        }

        for (j = 0; j < glob_buf.gl_pathc; j++) {
            const char* dev = glob_buf.gl_pathv[j];
            struct stat st;

            if (dev == NULL) continue;
            if (stat(dev, &st) != 0) continue;
            if (!S_ISBLK(st.st_mode)) continue;

            snprintf(path, path_size, "%s", dev);
            globfree(&glob_buf);
            return true;
        }

        globfree(&glob_buf);
    }

    return false;
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

static bool ui_upgrade_service_mount_dir_prepare(void)
{
    struct stat st;

    if (stat(UI_UPGRADE_USB_MNT, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    if (errno != ENOENT) {
        return false;
    }
    return mkdir(UI_UPGRADE_USB_MNT, 0755) == 0;
}

static bool ui_upgrade_service_try_umount_usb(void)
{
    if (!ui_upgrade_service_usb_mounted()) {
        return true;
    }
    if (system("umount /mnt/usb 2>/dev/null") != 0) {
        return false;
    }
    return !ui_upgrade_service_usb_mounted();
}

static bool ui_upgrade_service_try_mount_usb(const char* usb_dev)
{
    char mounted_dev[128];
    char cmd[192];

    if (usb_dev == NULL || usb_dev[0] == '\0') {
        return false;
    }
    if (ui_upgrade_service_get_mounted_device(mounted_dev,
                                              sizeof(mounted_dev))) {
        if (access(mounted_dev, F_OK) == 0) {
            return true;
        }
        if (!ui_upgrade_service_try_umount_usb()) {
            return false;
        }
    }
    if (!ui_upgrade_service_mount_dir_prepare()) {
        return false;
    }

    snprintf(cmd, sizeof(cmd), "mount '%s' %s 2>/dev/null", usb_dev, UI_UPGRADE_USB_MNT);
    if (system(cmd) != 0) {
        return false;
    }
    return ui_upgrade_service_usb_mounted();
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
    ui_upgrade_service_hash_cache_clear(&g_ui_upgrade_pkg_hash_cache);
    g_ui_upgrade_service.child_pid = -1;
    ui_upgrade_service_set_status(false, false, false, 0,
                                  UI_UPGRADE_STAGE_NONE, "", "");
}

void ui_upgrade_service_detect(ui_upgrade_detect_info_t* info)
{
    char usb_dev[128];

    if (info == NULL) return;

    info->usb_present =
        ui_upgrade_service_find_usb_device_path(usb_dev, sizeof(usb_dev));
    if (info->usb_present) {
        info->usb_mounted = ui_upgrade_service_try_mount_usb(usb_dev);
    } else {
        (void)ui_upgrade_service_try_umount_usb();
        info->usb_mounted = ui_upgrade_service_usb_mounted();
    }
    info->package_found = info->usb_present && info->usb_mounted &&
                          ui_upgrade_service_file_exists(UI_UPGRADE_FILE_PATH);
    info->package_hash_match = false;

    if (info->package_found) {
        info->package_hash_match = ui_upgrade_service_package_hash_match_running();
    } else {
        ui_upgrade_service_hash_cache_clear(&g_ui_upgrade_pkg_hash_cache);
    }
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
