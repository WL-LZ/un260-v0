#include "basic_setting_reply_dispatch.h"

#include <stdbool.h>

#include "un260/app_service/setting_service.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_core/lv_page_event.h"
#include "un260/lv_core/page_03_menu.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_core/page_01_main.h"
#include "un260/machine_state/machine_state.h"
#include "un260/protocol/mode_codec.h"
#include "un260/lv_system/user_cfg.h"

basic_setting_reply_action_t basic_setting_reply_dispatch(uint8_t cmd,
                                                           const uint8_t *buf,
                                                           uint8_t len)
{
    basic_setting_reply_action_t actions = BASIC_SETTING_REPLY_ACTION_NONE;

    switch (cmd) {
    case 0x04:
    {
        if (len < 6) break;

        uint8_t status = buf[4];

        if (status == 0x01)
        {
            uint8_t requested_mode = 0;
            if (setting_service_mode_is_pending()) {
                requested_mode = setting_service_mode_target();
                machine_state_confirm_mode(requested_mode);
            }
            {
                const char* mode_str = "NONE";
                if (machine_state_mode() == MODE_MDC) mode_str = "MDC";
                else if (machine_state_mode() == MODE_SDC) mode_str = "SDC";
                else if (machine_state_mode() == MODE_CNT) mode_str = "CNT";

                if (requested_mode != 0) {
                    icon_feedback_comp("page_01_mode_icon.png", page_01_main_obj, page_01_main_len);
                }
                update_label_by_name(page_01_main_obj, page_01_main_len, "mix_label", "%s", mode_str);
                update_label_by_name(page_01_main_obj, page_01_main_len, "mode_label", "%s", mode_str);
                page_01_bottom_a_refresh_mode(true);
            }
            setting_service_mode_finish();
            actions = (basic_setting_reply_action_t)(actions |
                      BASIC_SETTING_REPLY_ACTION_SCHEDULE_MODE_CLEAR);
            uart_printf(fd6, "Set work mode success\n");
            smart_island_refresh_summary();
        }
        else if (status == 0x02)
        {
            setting_service_mode_finish();
            uart_printf(fd6, "Set work mode fail\n");
            show_start_fault_popup(0x02, 0x06);
        }
        else if (status == 0x03)
        {
            if (len < 7) break;
            uint8_t protocol_mode = buf[5];
            uint8_t machine_mode;

            if (!mode_codec_decode(protocol_mode, &machine_mode)) {
                uart_printf(fd6, "Boot work mode invalid: 0x%02X\n", protocol_mode);
                break;
            }

            machine_state_confirm_mode(machine_mode);
            setting_service_mode_finish();
            page_01_mode_switch_refre();
            uart_printf(fd6, "Boot work mode: 0x%02X\n", protocol_mode);
            smart_island_refresh_summary();
        }
        break;
    }

    case 0x06:
    {
        if (len < 6) break;

        uint8_t status = buf[4];

        if (status == 0x01)
        {
            setting_batch_result_t result;
            if (setting_service_batch_take_result(status, &result)) {
                if (result.type == SETTING_BATCH_REQUEST_NUMBER) {
                    page_03_batch_set_result(true, &result);
                } else if (result.type == SETTING_BATCH_REQUEST_SWITCH) {
                    batch_switch_on_0x06_result(true, &result);
                }
            }
            uart_printf(fd6, "Set batch num success\n");
            smart_island_refresh_summary();
        }
        else if (status == 0x02)
        {
            setting_batch_result_t result;
            if (setting_service_batch_take_result(status, &result)) {
                if (result.type == SETTING_BATCH_REQUEST_NUMBER) {
                    page_03_batch_set_result(false, &result);
                } else if (result.type == SETTING_BATCH_REQUEST_SWITCH) {
                    batch_switch_on_0x06_result(false, &result);
                }
            }
            show_batch_set_fail_popup();
            uart_printf(fd6, "Set batch num fail\n");
        }
        else if (status == 0x03)
        {
            if (len < 7) break;
            machine_state_confirm_batch(buf[5] != 200, buf[5]);
            set_batch_switch_state(machine_state_batch_enabled());
            if (machine_state_batch_enabled()) {
                update_label_by_name(page_03_menu_obj, page_03_menu_len, "03_batch_num_label",
                                     "%d", machine_state_batch_num());
            } else {
                update_label_by_name(page_03_menu_obj, page_03_menu_len, "03_batch_num_label",
                                     "%s", "OFF");
            }
            page_01_batch_refre();
            uart_printf(fd6, "Boot batch num: %d\n", machine_state_batch_num());
            smart_island_refresh_summary();
        }
        break;
    }

    case 0x39:
    {
        if (len < 6) break;
        uint8_t sub = buf[4];

        if (sub == 0x00) {
            if (setting_service_add_is_pending()) {
                bool target = setting_service_add_target();
                setting_service_add_finish();
                machine_state_confirm_add(target);
                page_01_bottom_a_refresh_add(true);
            }
            page_03_update_menu_button_states_refresh();
            uart_printf(fd6, "ADD set success\n");
            smart_island_refresh_summary();
        } else if (sub == 0x01) {
            if (setting_service_add_is_pending()) {
                setting_service_add_finish();
            }
            uart_printf(fd6, "ADD set failed\n");
            show_start_fault_popup(0x02, 0x06);
            page_03_update_menu_button_states_refresh();
        } else if (sub == 0x02) {
            uint8_t v = buf[5];
            if (v == 0x00) {
                machine_state_confirm_add(false);
            } else if (v == 0x01) {
                machine_state_confirm_add(true);
            } else {
                uart_printf(fd6, "ADD boot status: unexpected raw=0x%02X, keep %s\n",
                            v, machine_state_add_enabled() ? "ON" : "OFF");
            }
            if (setting_service_add_is_pending()) {
                setting_service_add_finish();
            }
            page_01_bottom_a_refresh_add(false);
            uart_printf(fd6, "ADD boot status: raw=0x%02X -> %s\n",
                        v, machine_state_add_enabled() ? "ON" : "OFF");
            smart_island_refresh_summary();
            page_03_update_menu_button_states_refresh();
        }
        break;
    }

    case 0x15:
    {
        if (len < 6) break;
        uint8_t sub = buf[4];

        if (sub == 0x01) {
            if (setting_service_beep_is_pending()) {
                bool target = setting_service_beep_target();
                setting_service_beep_finish();
                machine_state_confirm_buzzer(target);
            }
            uart_printf(fd6, "BEEP set success\n");
            page_03_update_menu_button_states_refresh();
        } else if (sub == 0x02) {
            uart_printf(fd6, "BEEP set failed\n");
            if (setting_service_beep_is_pending()) {
                setting_service_beep_finish();
            }
            show_start_fault_popup(0x02, 0x06);
            page_03_update_menu_button_states_refresh();
        } else if (sub == 0x03) {
            if (len < 7) break;
            uint8_t v = buf[5];
            machine_state_confirm_buzzer(v == 0x01);
            if (setting_service_beep_is_pending()) {
                setting_service_beep_finish();
            }
            uart_printf(fd6, "BEEP boot status: %s\n", machine_state_buzzer_enabled() ? "ON" : "OFF");
            page_03_update_menu_button_states_refresh();
        }
        break;
    }

    case 0x16:
    {
        if (len < 6) break;
        uint8_t type = buf[4];
        uint8_t res  = buf[5];

        if (type >= 0x01 && type <= 0x03) {
            if (res == 0x01) {
                uint8_t target_speed = (uint8_t)(0x03 - type);
                if (!setting_service_speed_is_pending()) {
                    uart_printf(fd6, "SPEED set SUCCESS ignored: no pending request\n");
                    break;
                }
                target_speed = setting_service_speed_target();
                setting_service_speed_finish();
                machine_state_confirm_speed(target_speed);
                page_03_update_menu_button_states_refresh();
                page_01_bottom_c_refresh_speed(true);
                page_01_speed_refre();
                uart_printf(fd6, "SPEED set SUCCESS: type=0x%02X -> ui=%u\n",
                            type, machine_state_speed());
                smart_island_refresh_summary();
            } else if (res == 0x02) {
                if (setting_service_speed_is_pending()) {
                    setting_service_speed_finish();
                }
                page_03_update_menu_button_states_refresh();
                uart_printf(fd6, "SPEED set FAIL: type=0x%02X\n", type);
                show_start_fault_popup(0x02, 0x06);
            } else {
                uart_printf(fd6, "SPEED set UNKNOWN result: type=0x%02X, res=0x%02X\n", type, res);
            }
        } else if (type == 0x04) {
            if (res <= 0x02) {
                machine_state_confirm_speed((uint8_t)(0x02 - res));
                page_03_update_menu_button_states_refresh();
                uart_printf(fd6, "SPEED boot sync: mode=0x%02X -> ui=%u\n", res, machine_state_speed());
                smart_island_refresh_summary();
            } else {
                uart_printf(fd6, "SPEED boot sync: invalid mode=0x%02X\n", res);
            }
        } else {
            uart_printf(fd6, "0x16: unknown type=0x%02X, res=0x%02X\n", type, res);
        }
        break;
    }

    case 0x3A:
    {
        if (len < 6) {
            uart_printf(fd6, "0x3A: frame too short (%d)\n", len);
            break;
        }
        uint8_t type = buf[4];
        uint8_t val  = buf[5];
        if (type == 0x05) {
            if (val <= 0x03) {
                machine_state_confirm_fo_mode(val);
                uart_printf(fd6, "FO boot sync: mode=0x%02X -> ui=%u\n", val, machine_state_fo_mode());
            } else {
                uart_printf(fd6, "FO boot sync: invalid mode=0x%02X\n", val);
            }
            if (setting_service_fo_mode_is_pending()) {
                setting_service_fo_mode_finish();
            }
            page_01_bottom_a_refresh_fo(false);
            smart_island_refresh_summary();
            break;
        }
        if (type <= 0x03) {
            if (val == 0x01) {
                uint8_t target_mode = 0;
                if (!setting_service_fo_mode_is_pending()) {
                    uart_printf(fd6, "FO set SUCCESS ignored: no pending request\n");
                    break;
                }
                target_mode = setting_service_fo_mode_target();
                setting_service_fo_mode_finish();
                machine_state_confirm_fo_mode(target_mode);
                page_01_bottom_a_refresh_fo(true);
                page_03_update_menu_button_states_refresh();
                uart_printf(fd6, "FO set SUCCESS: type=0x%02X -> ui=%u\n", type, machine_state_fo_mode());
                smart_island_refresh_summary();
            } else if (val == 0x02) {
                if (setting_service_fo_mode_is_pending()) {
                    setting_service_fo_mode_finish();
                }
                uart_printf(fd6, "FO set FAIL: type=0x%02X\n", type);
                show_start_fault_popup(0x02, 0x06);
                page_03_update_menu_button_states_refresh();
            } else {
                uart_printf(fd6, "FO set UNKNOWN result: type=0x%02X, res=0x%02X\n", type, val);
            }
        } else {
            uart_printf(fd6, "0x3A: unknown type=0x%02X, val=0x%02X\n", type, val);
        }
        break;
    }

    case 0x38:
    {
        if (len < 5) break;

        if (len == 7 && buf[4] == 0x02) {
            uint8_t mode = buf[5];
            if (mode == 0x00) {
                machine_state_confirm_work_mode(1);
            } else if (mode == 0x01) {
                machine_state_confirm_work_mode(0);
            }
            if (setting_service_work_mode_is_pending()) {
                setting_service_work_mode_finish();
            }
            page_01_bottom_a_refresh_work(false);
            uart_printf(fd6, "0x38 BOOT mode=0x%02X\n", mode);
            smart_island_refresh_summary();
            break;
        }

        uint8_t res = buf[4];
        if (res == 0x00) {
            uint8_t target_mode = 0;
            if (!setting_service_work_mode_is_pending()) {
                uart_printf(fd6, "0x38 MANUAL OK ignored: no pending request\n");
                break;
            }
            target_mode = setting_service_work_mode_target();
            setting_service_work_mode_finish();
            machine_state_confirm_work_mode(target_mode);
            page_01_bottom_a_refresh_work(true);
            page_03_update_menu_button_states_refresh();
            uart_printf(fd6, "0x38 MANUAL OK\n");
            smart_island_refresh_summary();
        } else if (res == 0x01) {
            uint8_t target_mode = 0;
            if (!setting_service_work_mode_is_pending()) {
                uart_printf(fd6, "0x38 AUTO OK ignored: no pending request\n");
                break;
            }
            target_mode = setting_service_work_mode_target();
            setting_service_work_mode_finish();
            machine_state_confirm_work_mode(target_mode);
            page_01_bottom_a_refresh_work(true);
            page_03_update_menu_button_states_refresh();
            uart_printf(fd6, "0x38 AUTO OK\n");
            smart_island_refresh_summary();
        } else {
            if (setting_service_work_mode_is_pending()) {
                setting_service_work_mode_finish();
            }
            uart_printf(fd6, "0x38 RES=0x%02X\n", res);
            show_start_fault_popup(0x02, 0x06);
            page_03_update_menu_button_states_refresh();
        }
        break;
    }

    default:
        break;
    }

    return actions;
}
