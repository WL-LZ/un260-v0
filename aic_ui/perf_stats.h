#ifndef PERF_STATS_H
#define PERF_STATS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int fps;
    float cpu_percent;
    float mem_mb;
    float ui_avg_ms;
    float ui_max_ms;
    bool cpu_valid;
    bool mem_valid;
} perf_stats_snapshot_t;

void perf_stats_init(void);
void perf_stats_report_ui_time_us(uint32_t ui_time_us);
void perf_stats_sample(void);
void perf_stats_get_snapshot(perf_stats_snapshot_t *out);

#endif
