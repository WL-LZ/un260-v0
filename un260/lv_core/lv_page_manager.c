
#include "lvgl/lvgl.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_system/platform_app.h"
#include "un260/protocol/protocol_send.h"
#include "un260/lv_core/page_01_main.h"
#include"lv_page_declear.h"

#define UI_PAGE_STACK_CAPACITY 10
#define UI_PAGE_INVALID ((ui_page_t)-1)

typedef struct {
    ui_page_t current;
    ui_page_t stack[UI_PAGE_STACK_CAPACITY];
    int stack_top;
} ui_page_manager_context_t;

static ui_page_manager_context_t g_page_manager = {
    .current = UI_PAGE_INVALID,
    .stack_top = -1,
};

typedef void (*ui_page_create_fn_t)(lv_obj_t *parent);
typedef void (*ui_page_destroy_fn_t)(void);

typedef struct {
    ui_page_create_fn_t create;
    ui_page_destroy_fn_t destroy;
} ui_page_registration_t;

static void ui_manager_create_main(lv_obj_t *parent)
{
    ui_main_create(parent);
    resume_counting_sim();
}

static void ui_manager_create_debug(lv_obj_t *parent)
{
    LV_UNUSED(parent);
    ui_page_10_debug_create();
}

static const ui_page_registration_t g_page_registry[UI_PAGE_COUNT] = {
    [UI_PAGE_BOOT_ANIM] = { ui_page_00_boot_anim_create, ui_page_00_boot_anim_destroy },
    [UI_PAGE_MAIN] = { ui_manager_create_main, ui_main_destroy },
    [UI_PAGE_LIST] = { ui_page_02_list_create, ui_page_02_list_destroy },
    [UI_PAGE_MENU] = { ui_page_03_menu_create, ui_page_03_menu_destroy },
    [UI_PAGE_SETTING] = { ui_page_06_settings_create, ui_page_06_settings_destroy },
    [UI_PAGE_SET_PASSAGE] = { ui_page_05_set_password_create, ui_page_05_set_password_destroy },
    [UI_PAGE_CURR] = { ui_page_07_curr_create, ui_page_07_curr_destroy },
    [UI_PAGE_BOOT] = { ui_page_08_curr_create, ui_page_08_curr_destroy },
    [UI_PAGE_CIS_CALIB] = { ui_page_cis_calib_create, ui_page_cis_calib_destroy },
    [UI_PAGE_DEBUG] = { ui_manager_create_debug, ui_page_10_debug_destroy },
    [UI_PAGE_TIMESET] = { ui_page_11_timeset_create, ui_page_11_timeset_destroy },
    [UI_PAGE_SENSOR] = { ui_page_12_sensor_create, ui_page_12_sensor_destroy },
    [UI_PAGE_UPGRADE] = { ui_page_13_upgrade_create, ui_page_13_upgrade_destroy },
    [UI_PAGE_MAIN_UPGRADE] = { ui_page_14_main_upgrade_create, ui_page_14_main_upgrade_destroy },
    [UI_PAGE_IMAGE_UPGRADE] = { ui_page_15_image_upgrade_create, ui_page_15_image_upgrade_destroy },
    [UI_PAGE_UI_UPGRADE] = { ui_page_16_ui_upgrade_create, ui_page_16_ui_upgrade_destroy },
    [UI_PAGE_MOTOR_TEST] = { ui_page_17_motor_test_create, ui_page_17_motor_test_destroy },
    [UI_PAGE_PURE] = { ui_page_18_pure_create, ui_page_18_pure_destroy },
    [UI_PAGE_HISTORY] = { ui_page_19_history_create, ui_page_19_history_destroy },
    [UI_PAGE_PRINT_SETTING] = { ui_page_20_set_print_create, ui_page_20_set_print_destroy },
    [UI_PAGE_LANGUAGE_SETTING] = { ui_page_21_set_language_create, ui_page_21_set_language_destroy },
    [UI_PAGE_DOUBLE_NOTE_SETTING] = { ui_page_22_set_double_note_create, ui_page_22_set_double_note_destroy },
    [UI_PAGE_FLAP_SETTING] = { ui_page_23_set_flap_create, ui_page_23_set_flap_destroy },
    [UI_PAGE_REJECT_POCKET_SETTING] = { ui_page_24_set_reject_pocket_create, ui_page_24_set_reject_pocket_destroy },
    [UI_PAGE_SERIAL_NUMBER_SETTING] = { ui_page_25_set_serial_number_create, ui_page_25_set_serial_number_destroy },
    [UI_PAGE_AGING_SETTING] = { ui_page_26_set_aging_create, ui_page_26_set_aging_destroy },
    [UI_PAGE_CFD_LEVEL_SETTING] = { ui_page_27_set_cfd_level_create, ui_page_27_set_cfd_level_destroy },
    [UI_PAGE_IMAGE_GET] = { ui_page_28_get_image_create, ui_page_28_get_image_destroy },
    [UI_PAGE_PASSWORD_CHANGE] = { ui_page_29_set_password_create, ui_page_29_set_password_destroy },
    [UI_PAGE_FACTORY_SETTING] = { ui_page_30_set_factory_create, ui_page_30_set_factory_destroy },
    [UI_PAGE_WAVE_GET] = { ui_page_31_get_wave_create, ui_page_31_get_wave_destroy },
};

static bool ui_manager_page_is_registered(ui_page_t page)
{
    return page >= UI_PAGE_BOOT_ANIM && page < UI_PAGE_COUNT &&
           g_page_registry[page].create != NULL;
}

//销毁当前页面
static void destroy_current_page(void)
{
    if (g_page_manager.current == UI_PAGE_MAIN) {
        if (page_01_main_is_created()) {
            page_01_main_suspend();
            return; // 直接返回，不执行销毁
        }
    }
    if (g_page_manager.current >= UI_PAGE_BOOT_ANIM &&
        g_page_manager.current < UI_PAGE_COUNT &&
        g_page_registry[g_page_manager.current].destroy != NULL) {
        g_page_registry[g_page_manager.current].destroy();
    }
}

static void create_new_page(ui_page_t page)
{
    if (page == UI_PAGE_MAIN && page_01_main_resume()) {
        return; // 直接返回，不执行创建
    }
    g_page_registry[page].create(lv_scr_act());
}

typedef struct {
    ui_page_t from;
    ui_page_t to;
    uint8_t cmd_g;
    uint8_t cmd_s;
} page_switch_notify_rule_t;

static void ui_manager_send_protocol(uint8_t cmd_g, uint8_t cmd_s)
{
    protocol_send(cmd_g, &cmd_s, 1);
}

static void ui_manager_notify_page_switch(ui_page_t from, ui_page_t to)
{
    static const page_switch_notify_rule_t rules[] = {
        { UI_PAGE_MAIN, UI_PAGE_LIST, 0x40, 0x01 },
        { UI_PAGE_LIST, UI_PAGE_MAIN, 0x40, 0x00 },
    };

    for (size_t i = 0; i < sizeof(rules) / sizeof(rules[0]); i++) {
        if (rules[i].from == from && rules[i].to == to) {
            ui_manager_send_protocol(rules[i].cmd_g, rules[i].cmd_s);
            break;
        }
    }
}

void ui_manager_switch(ui_page_t page)
{
    ui_page_t from = g_page_manager.current;

    if (page == g_page_manager.current) return;
    if (!ui_manager_page_is_registered(page)) return;
    ui_manager_notify_page_switch(from, page);
    destroy_current_page();
    create_new_page(page);
    g_page_manager.current = page;
}

void ui_manager_init(void) {
    // 初始化堆栈
    g_page_manager.stack_top = -1;

    // 显示主页面

    ui_manager_switch(UI_PAGE_MAIN);
    sim_reset_counting_result(&sim);

}


void ui_manager_push_page(ui_page_t page)
{
    int i;

    if (page == g_page_manager.current ||
        !ui_manager_page_is_registered(page)) return;

    //当前页入栈
    if (g_page_manager.current != UI_PAGE_INVALID) {
        if (g_page_manager.stack_top < UI_PAGE_STACK_CAPACITY - 1) {
            g_page_manager.stack[++g_page_manager.stack_top] =
                g_page_manager.current;
        } else {
            for (i = 0; i < UI_PAGE_STACK_CAPACITY - 1; i++) {
                g_page_manager.stack[i] = g_page_manager.stack[i + 1];
            }
            g_page_manager.stack[UI_PAGE_STACK_CAPACITY - 1] =
                g_page_manager.current;
        }
    }
    ui_manager_switch(page);
}

bool ui_manager_pop_page(void)
{
    ui_page_t previous_page;

    if (g_page_manager.stack_top < 0)
    {
        // 栈为空时，判断是否需要返回主页面
        if (g_page_manager.current != UI_PAGE_INVALID &&
            g_page_manager.current != UI_PAGE_MAIN) {
            ui_manager_switch(UI_PAGE_MAIN);
            return true;
        }
        return false;  
    }

    // 出栈
    previous_page = g_page_manager.stack[g_page_manager.stack_top--];
    ui_manager_switch(previous_page);
    return true;
}

void ui_manager_clear_stack(void)
{
    g_page_manager.stack_top = -1;
}


// 读取当前页
ui_page_t ui_manager_get_current_page(void) {
    return g_page_manager.current;
}
