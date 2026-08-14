#include "app_setting_runtime.h"

#include <stddef.h>

#include "lvgl/lvgl.h"

#include "un260/lv_system/platform_app.h"
#include "un260/protocol/basic_setting_reply_dispatch.h"

#define APP_SETTING_MODE_CLEAR_DELAY_MS 120

static lv_timer_t *g_mode_clear_timer;

static void app_setting_runtime_mode_clear_timer_cb(lv_timer_t *timer)
{
    if (timer == NULL || timer != g_mode_clear_timer) {
        return;
    }

    g_mode_clear_timer = NULL;
    sim_clear_all_sn(&sim);
}

static void app_setting_runtime_schedule_mode_clear(void)
{
    if (sim.total_pcs == 0 && sim.err_num == 0 && sim.err_expected == 0) {
        return;
    }

    if (g_mode_clear_timer != NULL) {
        lv_timer_del(g_mode_clear_timer);
        g_mode_clear_timer = NULL;
    }

    g_mode_clear_timer = lv_timer_create(app_setting_runtime_mode_clear_timer_cb,
                                         APP_SETTING_MODE_CLEAR_DELAY_MS,
                                         NULL);
    if (g_mode_clear_timer == NULL) {
        sim_clear_all_sn(&sim);
        return;
    }
    lv_timer_set_repeat_count(g_mode_clear_timer, 1);
}

void app_setting_runtime_handle_basic_reply(uint8_t cmd,
                                            uint8_t *buf,
                                            uint8_t len)
{
    basic_setting_reply_action_t actions =
        basic_setting_reply_dispatch(cmd, buf, len);

    if ((actions & BASIC_SETTING_REPLY_ACTION_SCHEDULE_MODE_CLEAR) != 0) {
        app_setting_runtime_schedule_mode_clear();
    }
}

void app_setting_runtime_stop(void)
{
    if (g_mode_clear_timer != NULL) {
        lv_timer_del(g_mode_clear_timer);
        g_mode_clear_timer = NULL;
    }
}
