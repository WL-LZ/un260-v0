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

// 定义Machine_para变量
Machine_para_t Machine_para = {
    .mode = 1,  //1 MDC , 2 CNT ,3 VER , 4 SDC
    .curr_code = "CNY",
    .speed = 0,
    .add_enable = 0,
    .start_auto = 0,
    .current_currency = CURR_CNY_ITEM,
    .password = "1111",               // 默认密码1111
    .batch_mode = 0,                  //0 pcs ,1 amount
    .cfd_mode = 0,
    .fo_mode = 0,
    .work_mode = 0,
    .language = 0,
    .selected_currency = 0,
    .batch_amount = 0,
    .reject_pocket_max = REJECT_POCKET_MIN_CAPACITY,
    .buzzer_enable = 1,
    .serial_num_enable = 0,
    .currency_count = DEFAULT_CURRENCY_COUNT,
    .currencies = { "USD", "CNY", "EUR", "AED", "SAR", "OMR", "QAR", "MAD",
                    "EGP", "DZD", "INR", "PKR", "GBP", "IQD" },
    .year = 2024,
    .month = 10,
    .day = 26,
    .hour = 11,
    .minute = 28,
    .second = 30,
    .last_total_pcs = 0,
    .last_total_amount = 0,
    .history_total_notes_counted = 0,
    .print_space_top = 0,
    .print_head1 = "",
    .print_head2 = "",
    .print_content = PRINT_SETTING_CONTENT_LIST,
    .print_space_bottom = 0,
    .double_note_level = DOUBLE_NOTE_LEVEL_MIN,
    .flap_position = FLAP_POSITION_UP,
    .serial_number_level = SERIAL_NUMBER_LEVEL_OFF,
    .aging_running = false,
    .cfd_setting_currency = "CNY",
    .cfd_levels = {
        { 3, 3, 3, 3 },
        { 3, 3, 3, 3 },
        { 3, 3, 3, 3 },
    },
};
sensor_voltage_t g_sensor_voltage = { 0 };

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

    lv_snprintf(Machine_para.password, sizeof(Machine_para.password), "%s", buf);
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
    lv_snprintf(Machine_para.password, sizeof(Machine_para.password), "%s", password);
    return true;
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

//void user_data_init(void) {
//    memcpy(&Machine_para, &default_para, sizeof(Machine_para_t));
//}
                     
