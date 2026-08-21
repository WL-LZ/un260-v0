#include "lvgl/lvgl.h"
#include"user_cfg.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEFAULT_CURRENCY_COUNT 14
#define UI_STATE_DIR "/etc/ui_state"
#define USER_PASSWORD_PATH UI_STATE_DIR "/password.cfg"
#define SCREENSHOT_CFG_PATH UI_STATE_DIR "/screenshot.cfg"
#define SCREENSHOT_CFG_TMP_PATH UI_STATE_DIR "/screenshot.cfg.tmp"

static bool g_screenshot_enabled = true;

static char g_user_password[USER_PASSWORD_MAX_LEN + 1] = "1111";

static bool user_cfg_password_is_valid(const char* password)
{
    size_t len;

    if (!password) return false;

    len = strlen(password);
    if (len == 0 || len > USER_PASSWORD_MAX_LEN) return false;

    for (size_t i = 0; i < len; i++) {
        if (password[i] < '0' || password[i] > '9') {
            return false;
        }
    }

    return true;
}

static void user_cfg_password_store(const char *password)
{
    size_t len = strlen(password);

    if (len > USER_PASSWORD_MAX_LEN) {
        len = USER_PASSWORD_MAX_LEN;
    }
    memcpy(g_user_password, password, len);
    g_user_password[len] = '\0';
}

bool user_cfg_password_load(void)
{
    FILE* fp;
    char buf[USER_PASSWORD_MAX_LEN + 4];
    size_t len;

    fp = fopen(USER_PASSWORD_PATH, "r");
    if (!fp) return false;

    if (!fgets(buf, sizeof(buf), fp)) {
        fclose(fp);
        return false;
    }
    fclose(fp);

    len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        buf[--len] = '\0';
    }

    if (!user_cfg_password_is_valid(buf)) {
        return false;
    }

    user_cfg_password_store(buf);
    return true;
}

bool user_cfg_password_save(const char* password)
{
    FILE* fp;

    if (!user_cfg_password_is_valid(password)) {
        return false;
    }

    mkdir(UI_STATE_DIR, 0755);
    fp = fopen(USER_PASSWORD_PATH, "w");
    if (!fp) {
        return false;
    }

    fprintf(fp, "%s\n", password);
    fclose(fp);
    user_cfg_password_store(password);
    return true;
}

const char *user_cfg_password_get(void)
{
    return g_user_password;
}

bool user_cfg_screenshot_load(void)
{
    FILE* fp;
    int value;

    fp = fopen(SCREENSHOT_CFG_PATH, "r");
    if (fp == NULL) {
        g_screenshot_enabled = true;
        return false;
    }

    if (fscanf(fp, "%d", &value) != 1 || (value != 0 && value != 1)) {
        fclose(fp);
        g_screenshot_enabled = true;
        return false;
    }

    fclose(fp);
    g_screenshot_enabled = value != 0;
    return true;
}

bool user_cfg_screenshot_save(bool enabled)
{
    FILE* fp;
    int fd;
    bool write_ok = true;

    if (mkdir(UI_STATE_DIR, 0755) != 0 && errno != EEXIST) {
        return false;
    }

    fp = fopen(SCREENSHOT_CFG_TMP_PATH, "w");
    if (fp == NULL) {
        return false;
    }

    if (fprintf(fp, "%d\n", enabled ? 1 : 0) < 0 || fflush(fp) != 0) {
        fclose(fp);
        unlink(SCREENSHOT_CFG_TMP_PATH);
        return false;
    }

    fd = fileno(fp);
    if (fd < 0 || fsync(fd) != 0) {
        write_ok = false;
    }
    if (fclose(fp) != 0) {
        write_ok = false;
    }
    if (!write_ok) {
        unlink(SCREENSHOT_CFG_TMP_PATH);
        return false;
    }

    if (rename(SCREENSHOT_CFG_TMP_PATH, SCREENSHOT_CFG_PATH) != 0) {
        unlink(SCREENSHOT_CFG_TMP_PATH);
        return false;
    }

    g_screenshot_enabled = enabled;
    return true;
}

bool user_cfg_screenshot_enabled(void)
{
    return g_screenshot_enabled;
}
