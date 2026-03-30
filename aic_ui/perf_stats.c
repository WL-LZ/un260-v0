#include "perf_stats.h"

#include <stddef.h>

#include "cpu_mem.h"
#include "lv_port_disp.h"

static perf_stats_snapshot_t g_perf_stats = {0};
static struct cpu_occupy g_cpu_prev = {0};
static bool g_cpu_prev_valid = false;
static uint32_t g_ui_total_us = 0;
static uint32_t g_ui_max_us = 0;
static uint32_t g_ui_count = 0;

void perf_stats_init(void)
{
    g_perf_stats.fps = 0;
    g_perf_stats.cpu_percent = 0.0f;
    g_perf_stats.mem_mb = 0.0f;
    g_perf_stats.ui_avg_ms = 0.0f;
    g_perf_stats.ui_max_ms = 0.0f;
    g_perf_stats.cpu_valid = false;
    g_perf_stats.mem_valid = false;
    g_ui_total_us = 0;
    g_ui_max_us = 0;
    g_ui_count = 0;
    g_cpu_prev_valid = (cpu_occupy_get(&g_cpu_prev) == 0);
}

void perf_stats_report_ui_time_us(uint32_t ui_time_us)
{
    g_ui_total_us += ui_time_us;
    g_ui_count++;

    if (ui_time_us > g_ui_max_us) {
        g_ui_max_us = ui_time_us;
    }
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

    if (g_ui_count > 0) {
        g_perf_stats.ui_avg_ms = (float)g_ui_total_us / (float)g_ui_count / 1000.0f;
        g_perf_stats.ui_max_ms = (float)g_ui_max_us / 1000.0f;
    } else {
        g_perf_stats.ui_avg_ms = 0.0f;
        g_perf_stats.ui_max_ms = 0.0f;
    }

    g_ui_total_us = 0;
    g_ui_max_us = 0;
    g_ui_count = 0;
}

void perf_stats_get_snapshot(perf_stats_snapshot_t *out)
{
    if (out == NULL) {
        return;
    }

    *out = g_perf_stats;
}
