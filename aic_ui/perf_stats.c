#include "perf_stats.h"

#include <stddef.h>

#include "cpu_mem.h"
#include "lv_port_disp.h"

static perf_stats_snapshot_t g_perf_stats = {0};
static struct cpu_occupy g_cpu_prev = {0};
static bool g_cpu_prev_valid = false;

typedef struct {
    uint64_t total_us;
    uint32_t max_us;
    uint32_t count;
} perf_time_accumulator_t;

static perf_time_accumulator_t g_lvgl_time;
static perf_time_accumulator_t g_loop_time;
static perf_time_accumulator_t g_main_refresh_time;

static void perf_time_report(perf_time_accumulator_t *time,
                             uint32_t elapsed_us)
{
    time->total_us += elapsed_us;
    time->count++;
    if (elapsed_us > time->max_us) {
        time->max_us = elapsed_us;
    }
}

static void perf_time_sample(perf_time_accumulator_t *time,
                             float *avg_ms,
                             float *max_ms)
{
    if (time->count > 0) {
        *avg_ms = (float)time->total_us / (float)time->count / 1000.0f;
        *max_ms = (float)time->max_us / 1000.0f;
    } else {
        *avg_ms = 0.0f;
        *max_ms = 0.0f;
    }

    time->total_us = 0;
    time->max_us = 0;
    time->count = 0;
}

void perf_stats_init(void)
{
    g_perf_stats.fps = 0;
    g_perf_stats.cpu_percent = 0.0f;
    g_perf_stats.mem_mb = 0.0f;
    g_perf_stats.lvgl_avg_ms = 0.0f;
    g_perf_stats.lvgl_max_ms = 0.0f;
    g_perf_stats.loop_avg_ms = 0.0f;
    g_perf_stats.loop_max_ms = 0.0f;
    g_perf_stats.main_refresh_avg_ms = 0.0f;
    g_perf_stats.main_refresh_max_ms = 0.0f;
    g_perf_stats.cpu_valid = false;
    g_perf_stats.mem_valid = false;
    g_lvgl_time = (perf_time_accumulator_t){0};
    g_loop_time = (perf_time_accumulator_t){0};
    g_main_refresh_time = (perf_time_accumulator_t){0};
    g_cpu_prev_valid = (cpu_occupy_get(&g_cpu_prev) == 0);
}

void perf_stats_report_lvgl_time_us(uint32_t elapsed_us)
{
    perf_time_report(&g_lvgl_time, elapsed_us);
}

void perf_stats_report_loop_time_us(uint32_t elapsed_us)
{
    perf_time_report(&g_loop_time, elapsed_us);
}

void perf_stats_report_main_refresh_time_us(uint32_t elapsed_us)
{
    perf_time_report(&g_main_refresh_time, elapsed_us);
}

void perf_stats_reset_window(void)
{
    g_lvgl_time = (perf_time_accumulator_t){0};
    g_loop_time = (perf_time_accumulator_t){0};
    g_main_refresh_time = (perf_time_accumulator_t){0};
    g_cpu_prev_valid = (cpu_occupy_get(&g_cpu_prev) == 0);
}

void perf_stats_sample(void)
{
    struct cpu_occupy cpu_now;
    struct memory_occupy mem_now;

    g_perf_stats.fps = fbdev_draw_fps();

    if (cpu_occupy_get(&cpu_now) == 0) {
        if (g_cpu_prev_valid) {
            float cpu = cpu_occupy_cal(&g_cpu_prev, &cpu_now);
            if (cpu < 0.0f) cpu = 0.0f;
            if (cpu > 100.0f) cpu = 100.0f;
            g_perf_stats.cpu_percent = cpu;
            g_perf_stats.cpu_valid = true;
        } else {
            g_perf_stats.cpu_valid = false;
        }

        g_cpu_prev = cpu_now;
        g_cpu_prev_valid = true;
    } else {
        g_perf_stats.cpu_valid = false;
    }

    if (mem_occupy_get(&mem_now) == 0) {
        g_perf_stats.mem_mb = mem_occupy_cal_size(&mem_now);
        g_perf_stats.mem_valid = true;
    } else {
        g_perf_stats.mem_valid = false;
    }

    perf_time_sample(&g_lvgl_time,
                     &g_perf_stats.lvgl_avg_ms,
                     &g_perf_stats.lvgl_max_ms);
    perf_time_sample(&g_loop_time,
                     &g_perf_stats.loop_avg_ms,
                     &g_perf_stats.loop_max_ms);
    if (g_main_refresh_time.count > 0) {
        perf_time_sample(&g_main_refresh_time,
                         &g_perf_stats.main_refresh_avg_ms,
                         &g_perf_stats.main_refresh_max_ms);
    }
}

void perf_stats_get_snapshot(perf_stats_snapshot_t *out)
{
    if (out == NULL) {
        return;
    }

    *out = g_perf_stats;
}
