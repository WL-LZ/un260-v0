#include "ui_history_data.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "un260/lv_system/machine_time.h"
#include "un260/currency/currency_state.h"

#define UI_HISTORY_STORE_PATH "/etc/ui_state/history_state.cfg"
#define UI_HISTORY_MAGIC 0x48495354u
#define UI_HISTORY_VERSION 1u

static ui_history_store_t g_history_store;
static bool g_history_loaded = false;
static unsigned int g_history_file_magic = 0;
static unsigned int g_history_file_version = 0;

static void history_data_set_defaults(void)
{
    memset(&g_history_store, 0, sizeof(g_history_store));
    g_history_file_magic = 0;
    g_history_file_version = 0;
}

static int history_ensure_store_dir(void)
{
    const char *slash;
    char dir_path[128];
    size_t dir_len;
    int dir_fd;

    slash = strrchr(UI_HISTORY_STORE_PATH, '/');
    if (slash == NULL) {
        return -1;
    }

    dir_len = (size_t)(slash - UI_HISTORY_STORE_PATH);
    if (dir_len == 0 || dir_len >= sizeof(dir_path)) {
        return -1;
    }

    memcpy(dir_path, UI_HISTORY_STORE_PATH, dir_len);
    dir_path[dir_len] = '\0';

    if (mkdir(dir_path, 0755) != 0 && errno != EEXIST) {
        return -1;
    }

    dir_fd = open(dir_path, O_RDONLY | O_DIRECTORY);
    if (dir_fd >= 0) {
        fsync(dir_fd);
        close(dir_fd);
    }

    return 0;
}

static void history_save_to_file(void)
{
    char tmp_path[128];
    FILE *fp;
    int fd;
    int i;
    const char *slash;
    char dir_path[128];

    if (history_ensure_store_dir() != 0) {
        return;
    }

    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", UI_HISTORY_STORE_PATH);
    fd = open(tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return;
    }

    fp = fdopen(fd, "w");
    if (fp == NULL) {
        close(fd);
        return;
    }

    fprintf(fp, "magic=%u\n", UI_HISTORY_MAGIC);
    fprintf(fp, "version=%u\n", UI_HISTORY_VERSION);
    fprintf(fp, "total_notes_counted=%u\n", (unsigned)g_history_store.total_notes_counted);
    fprintf(fp, "record_count=%u\n", (unsigned)g_history_store.record_count);
    for (i = 0; i < g_history_store.record_count && i < UI_HISTORY_MAX_RECORDS; i++) {
        const ui_history_record_t *rec = &g_history_store.records[i];
        fprintf(fp, "record%02d_valid=%d\n", i, rec->valid ? 1 : 0);
        fprintf(fp, "record%02d_selected=%d\n", i, rec->selected ? 1 : 0);
        fprintf(fp, "record%02d_no=%u\n", i, (unsigned)rec->record_no);
        fprintf(fp, "record%02d_pcs=%u\n", i, (unsigned)rec->pcs);
        fprintf(fp, "record%02d_amount=%u\n", i, (unsigned)rec->amount);
        fprintf(fp, "record%02d_currency=%s\n", i, rec->currency);
        fprintf(fp, "record%02d_time=%04u-%02u-%02u %02u:%02u:%02u\n",
                i, (unsigned)rec->year, (unsigned)rec->month, (unsigned)rec->day,
                (unsigned)rec->hour, (unsigned)rec->minute, (unsigned)rec->second);
        fprintf(fp, "record%02d_denom=%s\n", i, rec->denom_text);
        fprintf(fp, "record%02d_sn=%s\n", i, rec->sn_text);
        fprintf(fp, "record%02d_err=%s\n", i, rec->error_frame_text);
    }

    fflush(fp);
    fsync(fd);
    fclose(fp);

    if (rename(tmp_path, UI_HISTORY_STORE_PATH) != 0) {
        unlink(tmp_path);
        return;
    }

    slash = strrchr(UI_HISTORY_STORE_PATH, '/');
    if (slash != NULL) {
        size_t dir_len = (size_t)(slash - UI_HISTORY_STORE_PATH);
        if (dir_len < sizeof(dir_path)) {
            memcpy(dir_path, UI_HISTORY_STORE_PATH, dir_len);
            dir_path[dir_len] = '\0';
            fd = open(dir_path, O_RDONLY | O_DIRECTORY);
            if (fd >= 0) {
                fsync(fd);
                close(fd);
            }
        }
    }
}

static void history_parse_line(char *line)
{
    int index;
    ui_history_record_t *rec;

    if (strncmp(line, "magic=", 6) == 0) {
        g_history_file_magic = (unsigned int)strtoul(line + 6, NULL, 0);
        return;
    }

    if (strncmp(line, "version=", 8) == 0) {
        g_history_file_version = (unsigned int)strtoul(line + 8, NULL, 0);
        return;
    }

    if (strncmp(line, "total_notes_counted=", 20) == 0) {
        g_history_store.total_notes_counted = (uint32_t)strtoul(line + 20, NULL, 0);
        return;
    }

    if (strncmp(line, "record_count=", 13) == 0) {
        int cnt = atoi(line + 13);
        if (cnt < 0) cnt = 0;
        if (cnt > UI_HISTORY_MAX_RECORDS) cnt = UI_HISTORY_MAX_RECORDS;
        g_history_store.record_count = (uint8_t)cnt;
        return;
    }

    if (sscanf(line, "record%02d_valid=", &index) == 1) {
        if (index >= 0 && index < UI_HISTORY_MAX_RECORDS) {
            rec = &g_history_store.records[index];
            rec->valid = atoi(strchr(line, '=') + 1) ? true : false;
        }
        return;
    }

    if (sscanf(line, "record%02d_selected=", &index) == 1) {
        if (index >= 0 && index < UI_HISTORY_MAX_RECORDS) {
            rec = &g_history_store.records[index];
            rec->selected = atoi(strchr(line, '=') + 1) ? true : false;
        }
        return;
    }

    if (sscanf(line, "record%02d_no=", &index) == 1) {
        if (index >= 0 && index < UI_HISTORY_MAX_RECORDS) {
            rec = &g_history_store.records[index];
            rec->record_no = (uint32_t)strtoul(strchr(line, '=') + 1, NULL, 0);
        }
        return;
    }

    if (sscanf(line, "record%02d_pcs=", &index) == 1) {
        if (index >= 0 && index < UI_HISTORY_MAX_RECORDS) {
            rec = &g_history_store.records[index];
            rec->pcs = (uint32_t)strtoul(strchr(line, '=') + 1, NULL, 0);
        }
        return;
    }

    if (sscanf(line, "record%02d_amount=", &index) == 1) {
        if (index >= 0 && index < UI_HISTORY_MAX_RECORDS) {
            rec = &g_history_store.records[index];
            rec->amount = (uint32_t)strtoul(strchr(line, '=') + 1, NULL, 0);
        }
        return;
    }

    if (sscanf(line, "record%02d_currency=", &index) == 1) {
        if (index >= 0 && index < UI_HISTORY_MAX_RECORDS) {
            rec = &g_history_store.records[index];
            lv_snprintf(rec->currency, sizeof(rec->currency), "%s", strchr(line, '=') + 1);
            rec->currency[strcspn(rec->currency, "\r\n")] = '\0';
        }
        return;
    }

    if (sscanf(line, "record%02d_time=", &index) == 1) {
        unsigned y, mon, d, h, m, s;
        if (index >= 0 && index < UI_HISTORY_MAX_RECORDS &&
            sscanf(strchr(line, '=') + 1, "%u-%u-%u %u:%u:%u", &y, &mon, &d, &h, &m, &s) == 6) {
            rec = &g_history_store.records[index];
            rec->year = (uint16_t)y;
            rec->month = (uint8_t)mon;
            rec->day = (uint8_t)d;
            rec->hour = (uint8_t)h;
            rec->minute = (uint8_t)m;
            rec->second = (uint8_t)s;
        }
        return;
    }

    if (sscanf(line, "record%02d_denom=", &index) == 1) {
        if (index >= 0 && index < UI_HISTORY_MAX_RECORDS) {
            rec = &g_history_store.records[index];
            lv_snprintf(rec->denom_text, sizeof(rec->denom_text), "%s", strchr(line, '=') + 1);
            rec->denom_text[strcspn(rec->denom_text, "\r\n")] = '\0';
        }
        return;
    }

    if (sscanf(line, "record%02d_sn=", &index) == 1) {
        if (index >= 0 && index < UI_HISTORY_MAX_RECORDS) {
            rec = &g_history_store.records[index];
            lv_snprintf(rec->sn_text, sizeof(rec->sn_text), "%s", strchr(line, '=') + 1);
            rec->sn_text[strcspn(rec->sn_text, "\r\n")] = '\0';
        }
        return;
    }

    if (sscanf(line, "record%02d_err=", &index) == 1) {
        if (index >= 0 && index < UI_HISTORY_MAX_RECORDS) {
            rec = &g_history_store.records[index];
            lv_snprintf(rec->error_frame_text, sizeof(rec->error_frame_text), "%s", strchr(line, '=') + 1);
            rec->error_frame_text[strcspn(rec->error_frame_text, "\r\n")] = '\0';
        }
        return;
    }
}

static void history_load_from_file(void)
{
    FILE *fp;
    char line[1024];

    history_data_set_defaults();

    fp = fopen(UI_HISTORY_STORE_PATH, "r");
    if (fp == NULL) {
        g_history_loaded = true;
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        history_parse_line(line);
    }

    fclose(fp);

    if (g_history_file_magic != UI_HISTORY_MAGIC || g_history_file_version != UI_HISTORY_VERSION) {
        history_data_set_defaults();
    }

    if (g_history_store.record_count > UI_HISTORY_MAX_RECORDS) {
        g_history_store.record_count = UI_HISTORY_MAX_RECORDS;
    }

    g_history_loaded = true;
}

static void history_ensure_loaded(void)
{
    if (!g_history_loaded) {
        history_load_from_file();
    }
}

void ui_history_data_init(void)
{
    history_ensure_loaded();
}

const ui_history_store_t *ui_history_data_get(void)
{
    history_ensure_loaded();
    return &g_history_store;
}

uint32_t ui_history_total_notes_counted_get(void)
{
    history_ensure_loaded();
    return g_history_store.total_notes_counted;
}

void ui_history_total_notes_counted_set(uint32_t total)
{
    history_ensure_loaded();
    g_history_store.total_notes_counted = total;
    history_save_to_file();
}

void ui_history_total_notes_counted_clear(void)
{
    ui_history_total_notes_counted_set(0);
}

static void history_record_format_denom(const counting_sim_t *sim_data, char *dst, size_t size)
{
    int i;
    size_t pos = 0;

    if (dst == NULL || size == 0) {
        return;
    }

    dst[0] = '\0';
    if (sim_data == NULL) {
        return;
    }

    for (i = 0; i < sim_data->denom_number && i < (int)(sizeof(sim_data->denom) / sizeof(sim_data->denom[0])); i++) {
        int written;

        if (sim_data->denom[i].value <= 0 || sim_data->denom[i].pcs == 0) {
            continue;
        }
        if (pos + 1 >= size) {
            break;
        }

        written = lv_snprintf(dst + pos, size - pos, "%s%d x %u",
                              (pos > 0) ? "\n" : "",
                              sim_data->denom[i].value,
                              (unsigned)sim_data->denom[i].pcs);
        if (written < 0) {
            break;
        }
        pos += (size_t)written;
        if (pos + 1 >= size) {
            break;
        }
    }
}

static void history_record_format_sn(const counting_sim_t *sim_data, char *dst, size_t size)
{
    int i;
    size_t pos = 0;

    if (dst == NULL || size == 0) {
        return;
    }

    dst[0] = '\0';
    if (sim_data == NULL || sim_data->sn_str == NULL) {
        return;
    }

    for (i = 0; i < sim_data->sn_capacity; i++) {
        int written;

        if (sim_data->sn_str[i] == NULL || sim_data->sn_str[i][0] == '\0') {
            continue;
        }
        if (pos + 1 >= size) {
            break;
        }

        written = lv_snprintf(dst + pos, size - pos, "%s%s",
                              (pos > 0) ? "\n" : "",
                              sim_data->sn_str[i]);
        if (written < 0) {
            break;
        }
        pos += (size_t)written;
        if (pos + 1 >= size) {
            break;
        }
    }
}

static void history_record_shift_right(void)
{
    int i;

    for (i = UI_HISTORY_MAX_RECORDS - 1; i > 0; i--) {
        g_history_store.records[i] = g_history_store.records[i - 1];
    }
}

bool ui_history_record_append_from_session(const counting_sim_t *sim_data, uint32_t pcs_total,
                                           uint32_t total_notes_after, const char *error_frame_text)
{
    ui_history_record_t *rec;
    machine_time_value_t now;
    char curr_code[4];

    history_ensure_loaded();

    if (sim_data == NULL) {
        return false;
    }

    if (g_history_store.record_count < UI_HISTORY_MAX_RECORDS) {
        g_history_store.record_count++;
    } else {
        history_record_shift_right();
    }

    rec = &g_history_store.records[0];
    memset(rec, 0, sizeof(*rec));
    rec->valid = true;
    rec->record_no = total_notes_after;
    rec->pcs = pcs_total;
    rec->amount = (uint32_t)((sim_data->total_amount < 0.0f) ? 0.0f : sim_data->total_amount);
    currency_state_get_active_code(curr_code);
    lv_snprintf(rec->currency, sizeof(rec->currency), "%s", curr_code);
    machine_time_get(&now);
    rec->year = now.year;
    rec->month = now.month;
    rec->day = now.day;
    rec->hour = now.hour;
    rec->minute = now.minute;
    rec->second = now.second;
    history_record_format_denom(sim_data, rec->denom_text, sizeof(rec->denom_text));
    history_record_format_sn(sim_data, rec->sn_text, sizeof(rec->sn_text));
    lv_snprintf(rec->error_frame_text, sizeof(rec->error_frame_text), "%s",
                error_frame_text ? error_frame_text : "");

    g_history_store.total_notes_counted = total_notes_after;
    history_save_to_file();
    return true;
}

bool ui_history_record_toggle_selected(uint8_t index)
{
    ui_history_record_t *rec;

    history_ensure_loaded();
    if (index >= g_history_store.record_count) {
        return false;
    }

    rec = &g_history_store.records[index];
    rec->selected = !rec->selected;
    history_save_to_file();
    return true;
}

bool ui_history_record_set_selected(uint8_t index, bool selected)
{
    ui_history_record_t *rec;

    history_ensure_loaded();
    if (index >= g_history_store.record_count) {
        return false;
    }

    rec = &g_history_store.records[index];
    rec->selected = selected;
    history_save_to_file();
    return true;
}

int ui_history_record_selected_first_index_get(void)
{
    int i;

    history_ensure_loaded();
    for (i = 0; i < g_history_store.record_count; i++) {
        if (g_history_store.records[i].selected) {
            return i;
        }
    }
    return -1;
}

int ui_history_record_selected_count_get(void)
{
    int i;
    int count = 0;

    history_ensure_loaded();
    for (i = 0; i < g_history_store.record_count; i++) {
        if (g_history_store.records[i].selected) {
            count++;
        }
    }
    return count;
}

void ui_history_record_clear_selected(void)
{
    int i;

    history_ensure_loaded();
    for (i = 0; i < g_history_store.record_count; i++) {
        g_history_store.records[i].selected = false;
    }
    history_save_to_file();
}

bool ui_history_record_get(uint8_t index, ui_history_record_t *out)
{
    history_ensure_loaded();
    if (out == NULL || index >= g_history_store.record_count) {
        return false;
    }

    *out = g_history_store.records[index];
    return true;
}
