#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_core/lv_page_event.h"
#include "un260/lv_core/page_02_list.h"
#include "un260/lv_core/lv_page_declear.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_components/lv_fault_popup.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_resources/lv_img_init.h"
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

static lv_obj_t* s_page_01_scroll_spacer = NULL;
static lv_obj_t* s_page_01_scroll_hint_up = NULL;
static lv_obj_t* s_page_01_scroll_hint_down = NULL;
static bool s_page_01_scroll_hint_up_show = false;
static bool s_page_01_scroll_hint_down_show = false;
static lv_coord_t s_page_01_detail_scroll_y[PAGE_01_DETAIL_SECTION_C + 1] = { 0 };
static uint16_t s_page_01_detail_first_row_cache[PAGE_01_DETAIL_SECTION_C + 1] = { 0 };
static int16_t s_page_01_detail_last_render_first_row[PAGE_01_DETAIL_SECTION_C + 1] = { -1, -1, -1 };

#define PAGE_01_SCROLL_HINT_UP_IMG_PATH       "L:/usr/local/share/lvgl_data/main_pol_up.png"
#define PAGE_01_SCROLL_HINT_DOWN_IMG_PATH     "L:/usr/local/share/lvgl_data/main_pol_down.png"
#define PAGE_01_SCROLL_HINT_UP_IMG_PATH_USB   "L:/mnt/usb/lvgl_data/main_pol_up.png"
#define PAGE_01_SCROLL_HINT_DOWN_IMG_PATH_USB "L:/mnt/usb/lvgl_data/main_pol_down.png"
#define PAGE_01_DETAIL_ROW_Y_OFFSET           6
#define PAGE_01_DETAIL_SCROLL_EDGE_BUFFER     32

static int page_01_detail_row_count_get(page_01_detail_section_t section);
static uint8_t page_01_detail_visible_row_limit_get(page_01_detail_section_t section);
int page_01_detail_row_gap_get(int section);

static lv_coord_t page_01_detail_view_h_get(page_01_detail_section_t section)
{
    switch (section) {
    case PAGE_01_DETAIL_SECTION_A:
        return 240; // A区保留TOTAL区域
    case PAGE_01_DETAIL_SECTION_B:
    case PAGE_01_DETAIL_SECTION_C:
    default:
        return 273; // B/C区容器进一步回收，避免滑动内容超出背景框
    }
}

static lv_coord_t page_01_detail_content_h_get(page_01_detail_section_t section)
{
    int row_count;
    lv_coord_t row_gap;

    row_count = page_01_detail_row_count_get(section);
    row_gap = (lv_coord_t)page_01_detail_row_gap_get(section);

    return PAGE_01_DETAIL_ROW_Y_OFFSET + (lv_coord_t)row_count * row_gap
        + PAGE_01_DETAIL_SCROLL_EDGE_BUFFER;
}

static bool page_01_set_hint_img_src(lv_obj_t* img_obj, const char* primary_path, const char* fallback_path) //设置遮挡提示图片（带回退路径）
{
    lv_img_header_t header;

    if (!img_obj || !lv_obj_is_valid(img_obj)) return false;

    if (lv_img_decoder_get_info(primary_path, &header) == LV_RES_OK) {
        lv_img_set_src(img_obj, primary_path);
        return true;
    }

    if (fallback_path && lv_img_decoder_get_info(fallback_path, &header) == LV_RES_OK) {
        lv_img_set_src(img_obj, fallback_path);
        return true;
    }

    return false;
}

static uint8_t page_01_detail_visible_row_limit_get(page_01_detail_section_t section)
{
    switch (section) {
    case PAGE_01_DETAIL_SECTION_A:
        return 8; // A区底部要显示TOTAL，详情最多8行
    case PAGE_01_DETAIL_SECTION_B:
    case PAGE_01_DETAIL_SECTION_C:
    default:
        return 9; // B/C无TOTAL，可显示9行
    }
}

int page_01_detail_row_gap_get(int section)
{
    page_01_detail_section_t sec = (page_01_detail_section_t)section;

    switch (sec) {
    case PAGE_01_DETAIL_SECTION_A:
        return 32; // A区保持8行节奏
    case PAGE_01_DETAIL_SECTION_B:
    case PAGE_01_DETAIL_SECTION_C:
    default:
        return 31; // B/C区对齐list页面行距
    }
}

static int page_01_detail_row_count_get(page_01_detail_section_t section)
{
    int count = 0;

    switch (section) {
    case PAGE_01_DETAIL_SECTION_A:
        for (int i = 0; i < sim.denom_number &&
            i < (int)(sizeof(sim.denom) / sizeof(sim.denom[0])); i++) {
            if (sim.denom[i].value > 0) count++;
        }
        if (count > 10) count = 10;
        break;
    case PAGE_01_DETAIL_SECTION_B:
        if (sim.sn_str != NULL) {
            for (int i = 0; i < sim.total_pcs; i++) {
                if (sim.sn_str[i] != NULL && sim.denom_mix[i] > 0) count++;
            }
        }
        break;
    case PAGE_01_DETAIL_SECTION_C:
        count = sim.err_num;
        if (count > 10) count = 10;
        break;
    default:
        break;
    }

    return count;
}

static lv_coord_t page_01_detail_max_scroll_y_get(page_01_detail_section_t section)
{
    int row_count;
    int visible_limit;
    lv_coord_t content_h;
    lv_coord_t view_h;

    row_count = page_01_detail_row_count_get(section);
    visible_limit = page_01_detail_visible_row_limit_get(section);
    if (row_count <= visible_limit) {
        return 0;
    }

    content_h = page_01_detail_content_h_get(section);
    view_h = page_01_detail_view_h_get(section);
    if (content_h <= view_h) {
        return 0;
    }

    return content_h - view_h;
}

int page_01_detail_scroll_first_row_get(int section)
{
    lv_coord_t scroll_top;
    lv_coord_t max_scroll;
    uint16_t first_row = 0;
    const lv_coord_t row_gap = (lv_coord_t)page_01_detail_row_gap_get(section);

    if (section < PAGE_01_DETAIL_SECTION_A || section > PAGE_01_DETAIL_SECTION_C) return 0;
    if (!page_01_main_scroll_container || !lv_obj_is_valid(page_01_main_scroll_container)) {
        return (int)s_page_01_detail_first_row_cache[section];
    }

    scroll_top = lv_obj_get_scroll_top(page_01_main_scroll_container);
    if (scroll_top < 0) scroll_top = 0;
    max_scroll = page_01_detail_max_scroll_y_get(section);
    if (scroll_top > max_scroll) scroll_top = max_scroll;

    first_row = (uint16_t)(scroll_top / row_gap);
    s_page_01_detail_first_row_cache[section] = first_row;
    return (int)first_row;
}

bool page_01_is_small_denom_mode(void) //当前币种是否为“小面额可见行”模式
{
    page_01_detail_section_t section = page_01_detail_section_get();
    int row_count = page_01_detail_row_count_get(section);
    int visible_limit = page_01_detail_visible_row_limit_get(section);

    return row_count <= visible_limit;
}

static void page_01_detail_scroll_spacer_refresh(page_01_detail_section_t section)
{
    lv_coord_t spacer_y;

    if (s_page_01_scroll_spacer == NULL || !lv_obj_is_valid(s_page_01_scroll_spacer)) return;
    spacer_y = page_01_detail_content_h_get(section);
    if (spacer_y < 1) spacer_y = 1;
    lv_obj_set_pos(s_page_01_scroll_spacer, 0, spacer_y);
}

static void page_01_detail_scroll_save_current(void)
{
    page_01_detail_section_t section = page_01_detail_section_get();
    lv_coord_t scroll_top;

    if (!page_01_main_scroll_container || !lv_obj_is_valid(page_01_main_scroll_container)) return;
    scroll_top = lv_obj_get_scroll_top(page_01_main_scroll_container);
    if (scroll_top < 0) scroll_top = 0;
    s_page_01_detail_scroll_y[section] = scroll_top;
    s_page_01_detail_first_row_cache[section] =
        (uint16_t)(scroll_top / page_01_detail_row_gap_get(section));
}

static void page_01_detail_scroll_restore_current(bool anim_en)
{
    page_01_detail_section_t section = page_01_detail_section_get();
    lv_coord_t target_scroll;
    lv_coord_t max_scroll;
    lv_coord_t view_h;

    if (!page_01_main_scroll_container || !lv_obj_is_valid(page_01_main_scroll_container)) return;
    view_h = page_01_detail_view_h_get(section);
    lv_obj_set_height(page_01_main_scroll_container, view_h);

    page_01_detail_scroll_spacer_refresh(section);
    max_scroll = page_01_detail_max_scroll_y_get(section);
    target_scroll = s_page_01_detail_scroll_y[section];
    if (target_scroll < 0) target_scroll = 0;
    if (target_scroll > max_scroll) target_scroll = max_scroll;
    s_page_01_detail_scroll_y[section] = target_scroll;
    lv_obj_scroll_to_y(page_01_main_scroll_container, target_scroll, anim_en ? LV_ANIM_ON : LV_ANIM_OFF);
}

void page_01_detail_scroll_before_section_switch(void)
{
    page_01_detail_scroll_save_current();
}

void page_01_detail_scroll_after_section_switch(void)
{
    page_01_detail_scroll_restore_current(false);
    page_01_scroll_hint_on_enter();
}

void page_01_detail_scroll_sync_current_section(void)
{
    page_01_detail_scroll_restore_current(false);
    page_01_scroll_hint_on_enter();
}

void page_01_detail_scroll_reset_all(void)
{
    for (int i = 0; i <= PAGE_01_DETAIL_SECTION_C; i++) {
        s_page_01_detail_scroll_y[i] = 0;
        s_page_01_detail_first_row_cache[i] = 0;
        s_page_01_detail_last_render_first_row[i] = -1;
    }

    if (page_01_main_scroll_container && lv_obj_is_valid(page_01_main_scroll_container)) {
        lv_obj_scroll_to_y(page_01_main_scroll_container, 0, LV_ANIM_OFF);
    }

    page_01_scroll_hint_force_hide();
}

static void page_01_scroll_hint_sync_visible(void) //同步遮挡提示最终可见状态
{
    if (s_page_01_scroll_hint_up && lv_obj_is_valid(s_page_01_scroll_hint_up)) {
        if (s_page_01_scroll_hint_up_show) lv_obj_clear_flag(s_page_01_scroll_hint_up, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(s_page_01_scroll_hint_up, LV_OBJ_FLAG_HIDDEN);
    }
    if (s_page_01_scroll_hint_down && lv_obj_is_valid(s_page_01_scroll_hint_down)) {
        if (s_page_01_scroll_hint_down_show) lv_obj_clear_flag(s_page_01_scroll_hint_down, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(s_page_01_scroll_hint_down, LV_OBJ_FLAG_HIDDEN);
    }
}

static void page_01_scroll_hint_refresh(bool force_blink) //刷新主界面详情遮挡提示（无动画）
{
    lv_coord_t top_hidden = 0;
    lv_coord_t max_scroll = 0;
    lv_coord_t bottom_epsilon = 2;
    int row_count = 0;
    int visible_limit = 0;
    bool top_show;
    bool bottom_show;
    page_01_detail_section_t section = page_01_detail_section_get();
    (void)force_blink;

    if (!page_01_main_scroll_container || !lv_obj_is_valid(page_01_main_scroll_container)) return;
    row_count = page_01_detail_row_count_get(section);
    visible_limit = page_01_detail_visible_row_limit_get(section);
    max_scroll = page_01_detail_max_scroll_y_get(section);

    top_hidden = lv_obj_get_scroll_top(page_01_main_scroll_container);
    if (top_hidden < 0) top_hidden = 0;
    top_show = (top_hidden > 0);
    // 底部回弹收敛阶段给一个小阈值，避免“已到底部但down仍短暂显示”
    bottom_show = (row_count > visible_limit) && (top_hidden + bottom_epsilon < max_scroll);
    if (row_count <= visible_limit) {
        top_show = false;
        bottom_show = false;
    }

    if (s_page_01_scroll_hint_up_show != top_show ||
        s_page_01_scroll_hint_down_show != bottom_show) {
        s_page_01_scroll_hint_up_show = top_show;
        s_page_01_scroll_hint_down_show = bottom_show;
        page_01_scroll_hint_sync_visible();
    }
}

static void page_01_scroll_hint_event_cb(lv_event_t* e) //详情滚动事件：更新遮挡提示
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SCROLL) {
        page_01_detail_section_t section = page_01_detail_section_get();
        int first_row = 0;

        page_01_detail_scroll_save_current();
        if (section == PAGE_01_DETAIL_SECTION_B) {
            first_row = (int)s_page_01_detail_first_row_cache[section];
            if (s_page_01_detail_last_render_first_row[section] != first_row) {
                s_page_01_detail_last_render_first_row[section] = (int16_t)first_row;
                page_01_main_detail_refresh_rows_only();
            }
        }
        page_01_scroll_hint_refresh(false);
        return;
    }

    if (code == LV_EVENT_SCROLL_END) {
        if (page_01_is_small_denom_mode() &&
            page_01_main_scroll_container && lv_obj_is_valid(page_01_main_scroll_container)) {
            // 小面额币种允许拖动，但松手后回到顶部
            lv_obj_scroll_to_y(page_01_main_scroll_container, 0, LV_ANIM_ON);
            s_page_01_detail_scroll_y[page_01_detail_section_get()] = 0;
            page_01_scroll_hint_refresh(false);
            return;
        }
        page_01_detail_scroll_save_current();
        page_01_scroll_hint_refresh(false);
    }
}

void page_01_scroll_hint_on_enter(void) //主界面进入/切币种后触发遮挡提示检测
{
    page_01_scroll_hint_refresh(true);
}

void page_01_scroll_hint_force_hide(void) //强制隐藏主界面详情遮挡提示
{
    s_page_01_scroll_hint_up_show = false;
    s_page_01_scroll_hint_down_show = false;
    page_01_scroll_hint_sync_visible();
}

void ui_switch_to(page_id_t page)
{
    lv_obj_add_flag(main_page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(setting_page, LV_OBJ_FLAG_HIDDEN);

    switch (page) {
    case PAGE_MAIN:
        lv_obj_clear_flag(main_page, LV_OBJ_FLAG_HIDDEN);
        break;
    case PAGE_SETTING:
        lv_obj_clear_flag(setting_page, LV_OBJ_FLAG_HIDDEN);
        break;
    }
}



// 主界面初始化
void create_main_page(void)
{

    main_page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_page, 1024, 400);

    lv_obj_t* label = lv_label_create(main_page);
    lv_label_set_text(label, "MAIN");
    lv_obj_center(label);

    lv_obj_t* btn = lv_btn_create(main_page);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_t* btn_label = lv_label_create(btn);
    lv_obj_align(btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(btn_label, "set");
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(btn_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(btn_label, &ui_font_Big, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(btn_label);
}

// 设置界面初始化
void create_setting_page(void)
{
    setting_page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(setting_page, 1024, 400);

    lv_obj_t* label = lv_label_create(setting_page);
    lv_label_set_text(label, "set_page");
    lv_obj_center(label);

    lv_obj_t* btn = lv_btn_create(setting_page);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_t* btn_label = lv_label_create(btn);
    lv_obj_align(btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(btn_label, "back");
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(btn_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    //  lv_obj_set_style_text_font(btn_label, &ui_font_Big, LV_PART_MAIN | LV_STATE_DEFAULT);

}

// UI 初始化（主入口）
void lvgl_ui_init(void)
{
    create_main_page();
    create_setting_page();

    // 默认只显示主界面
    lv_obj_add_flag(setting_page, LV_OBJ_FLAG_HIDDEN);
}



//主界面右侧详情数据写入容器

void page_01_mode_switch_refre()
{
    const char* mode_str = "NONE";

    switch (Machine_para.mode)
    {
    case MODE_MDC:
        mode_str = "MDC";
        break;

    case MODE_CNT:
        mode_str = "CNT";
        break;

    case MODE_VER:
        mode_str = "VER";
        break;

    case MODE_SDC:
        mode_str = "SDC";
        break;

    default:
        mode_str = "NONE";
        break;
    }

    update_label_by_name(page_01_main_obj, page_01_main_len, "mix_label", "%s", mode_str);
    update_label_by_name(page_01_main_obj, page_01_main_len, "mode_label", "%s", mode_str);
    page_01_bottom_a_refresh_mode(false);
}

void page_01_create_mian_scrollable_container(void)
{
    if (page_01_main_scroll_container) return;
    lv_obj_t* parent = find_obj_by_name("back_img", page_01_main_obj, page_01_main_len);
    if (!parent)
        parent = lv_scr_act();
    page_01_main_scroll_container = lv_obj_create(main_page);
    lv_obj_set_pos(page_01_main_scroll_container, 720, 54);
    lv_obj_set_size(page_01_main_scroll_container, 300, 240);
    lv_obj_set_style_bg_opa(page_01_main_scroll_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page_01_main_scroll_container, 0, 0);
    lv_obj_set_style_pad_all(page_01_main_scroll_container, 0, 0);
    lv_obj_set_scrollbar_mode(page_01_main_scroll_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(page_01_main_scroll_container, LV_DIR_VER);
    lv_obj_add_flag(page_01_main_scroll_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(page_01_main_scroll_container, page_01_detail_area_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(page_01_main_scroll_container, page_01_detail_area_event_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(page_01_main_scroll_container, page_01_detail_area_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(page_01_main_scroll_container, page_01_detail_area_event_cb, LV_EVENT_PRESS_LOST, NULL);
    lv_obj_add_event_cb(page_01_main_scroll_container, page_01_scroll_hint_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_event_cb(page_01_main_scroll_container, page_01_scroll_hint_event_cb, LV_EVENT_SCROLL_END, NULL);
 

    //将右边详情放到容器里面
    for (int i = 1; i <= 10; i++)
    {
        char name[32];
        lv_obj_t* obj;

        snprintf(name, sizeof(name), "denom_%d_label", i);
        obj = find_obj_by_name(name, page_01_main_obj, page_01_main_len);
        if (obj)
        {
            lv_obj_set_parent(obj, page_01_main_scroll_container);
            lv_obj_set_pos(obj, 8, (i - 1) * 32);
        }

        snprintf(name, sizeof(name), "pcs_%d_label", i);
        obj = find_obj_by_name(name, page_01_main_obj, page_01_main_len);
        if (obj)
        {
            lv_obj_set_parent(obj, page_01_main_scroll_container);
            lv_obj_set_pos(obj, 106, (i - 1) * 32);
        }


        snprintf(name, sizeof(name), "amount_%d_label", i);
        obj = find_obj_by_name(name, page_01_main_obj, page_01_main_len);
        if (obj)
        {
            lv_obj_set_parent(obj, page_01_main_scroll_container);
            lv_obj_set_pos(obj, 213, (i - 1) * 32);
        }
    }

    // 占位点高度由当前分区有效行数动态驱动
    s_page_01_scroll_spacer = lv_obj_create(page_01_main_scroll_container);
    lv_obj_remove_style_all(s_page_01_scroll_spacer);
    lv_obj_set_size(s_page_01_scroll_spacer, 1, 1);
    lv_obj_set_pos(s_page_01_scroll_spacer, 0, 241);
    lv_obj_set_style_bg_opa(s_page_01_scroll_spacer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(s_page_01_scroll_spacer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_page_01_scroll_spacer, LV_OBJ_FLAG_CLICKABLE);

    // 主界面详情区始终允许上下滑动，惯性由分区策略动态控制
    lv_obj_add_flag(page_01_main_scroll_container,
                    LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(page_01_main_scroll_container, LV_OBJ_FLAG_SCROLL_MOMENTUM);

    if (!s_page_01_scroll_hint_up || !lv_obj_is_valid(s_page_01_scroll_hint_up)) {
        s_page_01_scroll_hint_up = lv_img_create(main_page);
        lv_obj_remove_style_all(s_page_01_scroll_hint_up);
        lv_obj_set_pos(s_page_01_scroll_hint_up, 1051, 26);
        if (!page_01_set_hint_img_src(s_page_01_scroll_hint_up, PAGE_01_SCROLL_HINT_UP_IMG_PATH, PAGE_01_SCROLL_HINT_UP_IMG_PATH_USB)) {
            lv_obj_add_flag(s_page_01_scroll_hint_up, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_add_flag(s_page_01_scroll_hint_up, LV_OBJ_FLAG_HIDDEN);
    }

    if (!s_page_01_scroll_hint_down || !lv_obj_is_valid(s_page_01_scroll_hint_down)) {
        s_page_01_scroll_hint_down = lv_img_create(main_page);
        lv_obj_remove_style_all(s_page_01_scroll_hint_down);
        lv_obj_set_pos(s_page_01_scroll_hint_down, 1051, 309);
        if (!page_01_set_hint_img_src(s_page_01_scroll_hint_down, PAGE_01_SCROLL_HINT_DOWN_IMG_PATH, PAGE_01_SCROLL_HINT_DOWN_IMG_PATH_USB)) {
            lv_obj_add_flag(s_page_01_scroll_hint_down, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_add_flag(s_page_01_scroll_hint_down, LV_OBJ_FLAG_HIDDEN);
    }

    page_01_detail_scroll_sync_current_section();
}


void page_03_create_batch_label_switcher(lv_obj_t* parent)
{
    // 创建容器
    lv_obj_t* page_03_batch_container;

    page_03_batch_container = lv_obj_create(parent);
    lv_obj_remove_style_all(page_03_batch_container);
    lv_obj_set_pos(page_03_batch_container, 52, 37);
    lv_obj_set_size(page_03_batch_container, 460, 146);
    lv_obj_set_style_bg_color(page_03_batch_container, lv_color_hex(0xfffffdd6), 0);
    lv_obj_set_style_radius(page_03_batch_container, 10, 0);
    lv_obj_add_flag(page_03_batch_container, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_clear_flag(page_03_batch_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(page_03_batch_container, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* amount_obj = find_obj_by_name("03_amount_batch_label", page_03_menu_obj, page_03_menu_len);
    lv_obj_t* pcs_obj = find_obj_by_name("03_pcs_batch_label", page_03_menu_obj, page_03_menu_len);

    printf("Machine_para.batch_mode: %s\n", Machine_para.batch_mode?"AMOUNT MODE":"PCS MODE");

    if (amount_obj && pcs_obj)
    {

        lv_obj_set_parent(amount_obj, page_03_batch_container);
        lv_obj_set_parent(pcs_obj, page_03_batch_container);

        // 根据Machine_para.batch_mode初始化状态和位置
        if (Machine_para.batch_mode == AMOUNT_BATCH_MODE)
        {
            // AMOUNT模式
            is_amount_active = true;

            // AMOUNT 
            lv_obj_set_size(amount_obj, 210, 35);
            lv_obj_set_pos(amount_obj, 118, 63);  
            lv_label_set_long_mode(amount_obj, LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(amount_obj, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(amount_obj, lv_color_hex(0x4285F4), 0);  // 蓝色激活
            lv_obj_set_style_text_opa(amount_obj, 255, 0);  // 完全不透明

            // PCS
            lv_obj_set_size(pcs_obj, 160, 35);
            lv_obj_set_pos(pcs_obj, 143, 95);  
            lv_label_set_long_mode(pcs_obj, LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(pcs_obj, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(pcs_obj, lv_color_hex(0x888888), 0);  // 灰色非激活
            lv_obj_set_style_text_opa(pcs_obj, 40, 0);  // 半透明
        }
        else  
        {
            // PCS模式
            is_amount_active = false;

            // PCS
            lv_obj_set_size(pcs_obj, 160, 35);
            lv_obj_set_pos(pcs_obj, 143, 63);
            lv_label_set_long_mode(pcs_obj, LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(pcs_obj, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(pcs_obj, lv_color_hex(0x4285F4), 0); 
            lv_obj_set_style_text_opa(pcs_obj, 255, 0); 

            // AMOUNT 
            lv_obj_set_size(amount_obj, 210, 35);
            lv_obj_set_pos(amount_obj, 118, 95);  
            lv_label_set_long_mode(amount_obj, LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(amount_obj, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(amount_obj, lv_color_hex(0x888888), 0);  
            lv_obj_set_style_text_opa(amount_obj, 40, 0);  
        }
    }
    else
    {
        LV_LOG_WARN("amount_obj or pcs_obj not found. Check names or UI structure.");
    }

    if (!Machine_para.batch_switch_enable)
    {
        if (Machine_para.batch_mode == AMOUNT_BATCH_MODE)
        {
            lv_obj_set_style_text_color(amount_obj, lv_color_hex(0x888888), 0);  // 蓝色激活
            lv_obj_set_style_text_opa(amount_obj, 255, 0);  // 完全不透明
            lv_obj_set_style_text_color(pcs_obj, lv_color_hex(0x888888), 0);  // 灰色非激活
            lv_obj_set_style_text_opa(pcs_obj, 40, 0);  // 半透明

        }
        else
        {
            lv_obj_set_style_text_color(pcs_obj, lv_color_hex(0x888888), 0);  // 蓝色激活
            lv_obj_set_style_text_opa(pcs_obj, 255, 0);  // 完全不透明
            lv_obj_set_style_text_color(amount_obj, lv_color_hex(0x888888), 0);  // 灰色非激活
            lv_obj_set_style_text_opa(amount_obj, 40, 0);  // 半透明

        }

    }
    

        // 添加事件回调
        lv_obj_add_event_cb(page_03_batch_container, page_03_batch_label_input_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(page_03_batch_container, page_03_batch_label_input_event_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(page_03_batch_container, page_03_batch_label_input_event_cb, LV_EVENT_RELEASED, NULL);
        lv_obj_add_event_cb(page_03_batch_container, page_03_batch_label_input_event_cb, LV_EVENT_GESTURE, NULL);
    

}

void page_03_batch_num_container(void)
{
    //创建batch_num显示
    lv_obj_t* batch_num_area = lv_obj_create(menu_page);
    lv_obj_set_size(batch_num_area, 338, 49);
    lv_obj_set_pos(batch_num_area, 119, 156);
    lv_obj_set_style_bg_opa(batch_num_area, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(batch_num_area, LV_OPA_TRANSP, 0); // 保留边框（不透明）
    lv_obj_set_style_border_width(batch_num_area, 1, 0);
    lv_obj_set_style_radius(batch_num_area, 48, 0);
    lv_obj_clear_flag(batch_num_area, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(batch_num_area, LV_SCROLLBAR_MODE_OFF); // 滚动条关闭
    // 显示num标签
    batch_num_display = lv_label_create(batch_num_area);
    lv_obj_set_style_text_font(batch_num_display, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(batch_num_display, lv_color_hex(0x000000), 0);
    lv_obj_set_align(batch_num_display, LV_ALIGN_RIGHT_MID);
    lv_label_set_text(batch_num_display, "0");

    lv_obj_t* amount_obj = find_obj_by_name("03_amount_batch_label", page_03_menu_obj, page_03_menu_len);
    lv_obj_t* pcs_obj = find_obj_by_name("03_pcs_batch_label", page_03_menu_obj, page_03_menu_len);

    lv_obj_set_style_text_opa(pcs_obj, 255, 0);  // 半透明
    lv_obj_set_style_text_opa(amount_obj, 40, 0);
}


void page_03_batch_num_refre(void)
{
    if (Machine_para.batch_switch_enable) {
        if (Machine_para.batch_num > 0) {
            update_label_by_name(page_03_menu_obj, page_03_menu_len, "03_batch_num_label", "%d", Machine_para.batch_num);
        } else {
            update_label_by_name(page_03_menu_obj, page_03_menu_len, "03_batch_num_label", "%d", 200);
        }
    } else {
        update_label_by_name(page_03_menu_obj, page_03_menu_len, "03_batch_num_label", "%s", "OFF");
    }
    // 菜单重新显示时，编辑态统一回到 0，避免上次输入残留影响下一次输入
    page_03_batch_num_edit_reset();

}


void page_03_batch_mode_status_refre(void)
{
    lv_obj_t* amount_obj = find_obj_by_name("03_amount_batch_label", page_03_menu_obj, page_03_menu_len);
    lv_obj_t* pcs_obj = find_obj_by_name("03_pcs_batch_label", page_03_menu_obj, page_03_menu_len);
    if (!Machine_para.batch_switch_enable)
    {
        if (Machine_para.batch_mode == AMOUNT_BATCH_MODE)
        {
            lv_obj_set_style_text_color(amount_obj, lv_color_hex(0x888888), 0);  // 蓝色激活
            lv_obj_set_style_text_opa(amount_obj, 255, 0);  // 完全不透明
            lv_obj_set_style_text_color(pcs_obj, lv_color_hex(0x888888), 0);  // 灰色非激活
            lv_obj_set_style_text_opa(pcs_obj, 40, 0);  // 半透明

        }
        else
        {
            lv_obj_set_style_text_color(pcs_obj, lv_color_hex(0x888888), 0);  // 蓝色激活
            lv_obj_set_style_text_opa(pcs_obj, 255, 0);  // 完全不透明
            lv_obj_set_style_text_color(amount_obj, lv_color_hex(0x888888), 0);  // 灰色非激活
            lv_obj_set_style_text_opa(amount_obj, 40, 0);  // 半透明

        }

    }
    else
    {
        if (Machine_para.batch_mode == AMOUNT_BATCH_MODE)
        {
            lv_obj_set_style_text_color(amount_obj, lv_color_hex(0x4285F4), 0);  // 蓝色激活
            lv_obj_set_style_text_opa(amount_obj, 255, 0);  // 完全不透明
            lv_obj_set_style_text_color(pcs_obj, lv_color_hex(0x888888), 0);  // 灰色非激活
            lv_obj_set_style_text_opa(pcs_obj, 40, 0);  // 半透明

        }
        else
        {
            lv_obj_set_style_text_color(pcs_obj, lv_color_hex(0x4285F4), 0);  // 蓝色激活
            lv_obj_set_style_text_opa(pcs_obj, 255, 0);  // 完全不透明
            lv_obj_set_style_text_color(amount_obj, lv_color_hex(0x888888), 0);  // 灰色非激活
            lv_obj_set_style_text_opa(amount_obj, 40, 0);  // 半透明

        }
    }
}


typedef struct {
    bool is_dragging;
    lv_point_t click_offset;  // 鼠标点击点相对 obj 左上角的偏移
} drag_data_t;
static drag_data_t g_drag_data;

static void drag_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);
    lv_indev_t* indev = lv_event_get_indev(e);
    if (!indev) return;

    // 获取鼠标的绝对坐标
    lv_point_t mouse;
    lv_indev_get_point(indev, &mouse);

    // 获取父对象的绝对坐标范围，用于转换成相对坐标
    lv_obj_t* parent = lv_obj_get_parent(obj);
    lv_area_t parent_area;
    lv_obj_get_coords(parent, &parent_area);

    if (code == LV_EVENT_PRESSED) {
        lv_area_t coords;
        lv_obj_get_coords(obj, &coords);  // 得到对象在屏幕上的绝对坐标
        g_drag_data.is_dragging = true;
        g_drag_data.click_offset.x = mouse.x - coords.x1;
        g_drag_data.click_offset.y = mouse.y - coords.y1;
    }
    else if (code == LV_EVENT_PRESSING && g_drag_data.is_dragging) {
        // 计算新的绝对坐标，再转换为相对于父对象的坐标
        lv_coord_t abs_new_x = mouse.x - g_drag_data.click_offset.x;
        lv_coord_t abs_new_y = mouse.y - g_drag_data.click_offset.y;
        lv_obj_set_pos(obj, abs_new_x - parent_area.x1, abs_new_y - parent_area.y1);
        printf("mouse x:%d y:%d\n", mouse.x, mouse.y);

        printf("parn x:%d y:%d\n", abs_new_x - parent_area.x1, abs_new_y - parent_area.y1);
    }
    else if (code == LV_EVENT_RELEASED) {
        g_drag_data.is_dragging = false;
    }

}

static void liquid_glass_event_handler(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t* obj = lv_event_get_target(e);
        lv_obj_set_style_bg_color(obj, lv_color_hex(0xFFFFFF), 0);
    }
}

static void highlight_anim_cb(void* var, int32_t value) {
    lv_obj_set_x((lv_obj_t*)var, value);
}

void create_liquid_glass_panel(lv_obj_t* parent) {
    lv_obj_t* panel = lv_obj_create(parent);
    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, 300, 200);
    lv_obj_set_pos(panel, 100, 100);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    //液态玻璃
    static lv_style_t style_liquid;
    lv_style_init(&style_liquid);
    lv_style_set_bg_color(&style_liquid, lv_color_hex(0xFFFFFF));
    lv_style_set_bg_opa(&style_liquid, LV_OPA_50);
    lv_style_set_radius(&style_liquid, 20);
    lv_style_set_border_width(&style_liquid, 2);
    lv_style_set_border_color(&style_liquid, lv_color_hex(0xCCCCCC));
    lv_style_set_shadow_width(&style_liquid, 10);
    lv_style_set_shadow_color(&style_liquid, lv_color_hex(0xAAAAAA));
    lv_obj_add_style(panel, &style_liquid, 0);

    lv_obj_add_event_cb(panel, liquid_glass_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(panel, drag_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(panel, drag_event_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(panel, drag_event_cb, LV_EVENT_RELEASED, NULL);

    lv_obj_t* highlight = lv_obj_create(panel);
    lv_obj_remove_style_all(highlight);
    lv_obj_set_size(highlight, 100, lv_obj_get_height(panel));
    lv_obj_set_pos(highlight, -100, 0);

    static lv_style_t style_highlight;
    lv_style_init(&style_highlight);
    lv_style_set_bg_color(&style_highlight, lv_color_hex(0xFFFFFF));
    lv_style_set_bg_opa(&style_highlight, LV_OPA_30);
    lv_style_set_bg_grad_color(&style_highlight, lv_color_hex(0xFFFFFF));
    lv_style_set_bg_grad_dir(&style_highlight, LV_GRAD_DIR_HOR);
    lv_obj_add_style(highlight, &style_highlight, 0);

    //动画
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, highlight);
    lv_anim_set_values(&anim, -100, lv_obj_get_width(panel));
    lv_anim_set_time(&anim, 2000);
    lv_anim_set_delay(&anim, 500);
    lv_anim_set_playback_delay(&anim, 500);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&anim, highlight_anim_cb);
    lv_anim_start(&anim);
}


// void page_02_report_refre(void)
// {

//     lv_obj_t* a_total_amount;
//     lv_obj_t* a_total_pcs;
//     lv_obj_t* b_no;
//     lv_obj_t* b_denom;
//     lv_obj_t* b_sn;
//     lv_obj_t* c_no;
//     lv_obj_t* c_denom;
//     lv_obj_t* c_reject;
//     char a_denom_buf[32], a_pcs_buf[32], a_amount_buf[32], b_no_buf[32], b_denom_buf[32], b_sn_buf[32], c_no_buf[32], c_denom_buf[32], c_reject_buf[32];
//     for (int i = 0; i < sim.denom_number; i++)
//     {
//         int row;
//         row = i + 1;
//         snprintf(a_denom_buf,sizeof(a_denom_buf),"02_a_denom_%d",row);
//         snprintf(a_pcs_buf, sizeof(a_pcs_buf), "02_a_pcs_%d", row);
//         snprintf(a_amount_buf, sizeof(a_amount_buf), "02_a_amount_%d", row);
//         snprintf(b_no_buf, sizeof(b_no_buf), "02_b_no_%d", row);
//         snprintf(b_denom_buf, sizeof(b_denom_buf), "02_b_denom_%d", row);
//         snprintf(b_sn_buf, sizeof(b_sn_buf), "02_b_sn_%d", row);
//         snprintf(c_no_buf, sizeof(c_no_buf), "02_c_no_%d", row);
//         snprintf(c_denom_buf, sizeof(c_denom_buf), "02_c_denom_%d", row);
//         snprintf(c_reject_buf, sizeof(c_reject_buf), "02_c_reject_%d", row);


//         b_no = find_obj_by_name(b_no_buf, page_02_list_obj, page_02_list_len);
//         b_denom = find_obj_by_name(b_denom_buf, page_02_list_obj, page_02_list_len);
//         b_sn = find_obj_by_name(b_sn_buf, page_02_list_obj, page_02_list_len);
//         c_no = find_obj_by_name(c_no_buf, page_02_list_obj, page_02_list_len);
//         c_denom = find_obj_by_name(c_denom_buf, page_02_list_obj, page_02_list_len);
//         c_reject = find_obj_by_name(c_reject_buf, page_02_list_obj, page_02_list_len);

//         if (i < PAGE_02_B_ITEM)
//         {

//         }





//     }
// }
//仿真模式 非实机函数

void page_02_a_page_refre(void)
{
    update_label_by_name(page_02_list_obj, page_02_list_len, "02_a_pcs_amount", "%d", sim.total_pcs);
    update_label_by_name(page_02_list_obj, page_02_list_len, "02_a_amount_total", "%.0f", sim.total_amount);

    page_02_list_section_refresh(PAGE_02_SECTION_A);
}

static int page_02_b_valid_count_get(void) //获取B区有效冠字号条数（仅统计有SN且面额有效的数据）
{
    int valid_count = 0;

    if (sim.sn_str == NULL) return 0;

    for (int i = 0; i < sim.total_pcs; i++) {
        if (sim.sn_str[i] != NULL && sim.denom_mix[i] > 0) {
            valid_count++;
        }
    }
    return valid_count;
}

void page_02_b_page_refre(void)
{
    if (page_02_list_len <= 0) return;
    page_02_list_section_refresh(PAGE_02_SECTION_B);
}
//仿真模式 非实机函数
void page_02_c_page_refre(void)
{
    if (page_02_list_len <= 0) return;

    page_02_list_section_refresh(PAGE_02_SECTION_C);
}


void page_02_curr_refre(void)
{
    update_label_by_name(page_02_list_obj, page_02_list_len, "02_page_curr", "%s", Machine_para.curr_code);

}

void page_02_a_page_num_refre(void)
{
    int curr_num, total_num;
    // lv_obj_t* curr_obj;
    char curr_buf[16];
    curr_num = page_02_a_report_status.curent_page;
    total_num = page_02_a_report_status.total_page;
    snprintf(curr_buf, sizeof(curr_buf), "%d/%d", curr_num, total_num);
    // curr_obj = find_obj_by_name("02_a_page_refre", page_02_list_obj, page_02_list_len);
    update_label_by_name(page_02_list_obj, page_02_list_len, "02_a_page_refre", "%s", curr_buf);


}

void page_02_b_page_num_refre(void)
{
    int curr_num, total_num;
    // lv_obj_t* curr_obj;
    char curr_buf[16];
    int valid_total = page_02_b_valid_count_get();

    page_02_b_report_status.total_page = (valid_total == 0) ? 1 : ((valid_total + PAGE_02_B_ITEM - 1) / PAGE_02_B_ITEM);
    if (page_02_b_report_status.curent_page < 1) {
        page_02_b_report_status.curent_page = 1;
    } else if (page_02_b_report_status.curent_page > page_02_b_report_status.total_page) {
        page_02_b_report_status.curent_page = page_02_b_report_status.total_page;
    }

    curr_num = page_02_b_report_status.curent_page;
    total_num = page_02_b_report_status.total_page;
    snprintf(curr_buf, sizeof(curr_buf), "%d/%d", curr_num, total_num);
    // curr_obj = find_obj_by_name("02_b_page_refre", page_02_list_obj, page_02_list_len);
    update_label_by_name(page_02_list_obj, page_02_list_len, "02_b_page_refre", "%s", curr_buf);


}
void page_02_c_page_num_refre(void)
{
    int curr_num, total_num;
    // lv_obj_t* curr_obj;
    char curr_buf[16];
    curr_num = page_02_c_report_status.curent_page;
    total_num = page_02_c_report_status.total_page;
    snprintf(curr_buf, sizeof(curr_buf), "%d/%d", curr_num, total_num);
    // curr_obj = find_obj_by_name("02_c_page_refre", page_02_list_obj, page_02_list_len);
    update_label_by_name(page_02_list_obj, page_02_list_len, "02_c_page_refre", "%s", curr_buf);


}


void page_01_add_refre(void)
{
    
    update_label_by_name(page_01_main_obj, page_01_main_len, "add_label", "%s", Machine_para.add_enable == true?"ADD:ON":"ADD:OFF");
    page_01_bottom_a_refresh_add(false);

}
void page_01_work_refre(void)
{
    char* work[2] = {"AUTO","MANUAL"};
    update_label_by_name(page_01_main_obj, page_01_main_len, "auto_label", "%s", work[Machine_para.work_mode]);
    page_01_bottom_a_refresh_work(false);

}
void page_01_batch_refre(void)
{
    char* batch[2] = { "BATCH :","VBATCH :" };
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", Machine_para.batch_num);
    update_label_by_name(page_01_main_obj, page_01_main_len, "bacth_label", "%s", batch[Machine_para.batch_mode]);
    if (Machine_para.batch_switch_enable)
    {
        update_label_by_name(page_01_main_obj, page_01_main_len, "bacth_num_label", "%s", buf);
    }
    else
    {
        update_label_by_name(page_01_main_obj, page_01_main_len, "bacth_num_label", "%s", "OFF");
    }

    page_01_bottom_c_refresh_batch(false);

}
void page_01_face_refre(void)
{
    char* batch[4] = { "F./O. : OFF","F.","O." ,"F./O."};

    update_label_by_name(page_01_main_obj, page_01_main_len, "face_label", "%s", batch[Machine_para.fo_mode]);
    page_01_bottom_a_refresh_fo(false);

}
void page_01_cfd_refre(void)
{
    char* cfd[3] = { "L","M","H"  };
    update_label_by_name(page_01_main_obj, page_01_main_len, "cfd_value_label", "%s", cfd[Machine_para.cfd_mode]);
    printf("cfd:%s\n", cfd[Machine_para.cfd_mode]);
    page_01_bottom_c_refresh_cfd();

}
void page_01_speed_refre(void)
{
    int speed[3] = { 600,800,1000 };
    update_label_by_name(page_01_main_obj, page_01_main_len, "speed_num_label", "%d", speed[Machine_para.speed]);
    page_01_bottom_c_refresh_speed(false);
}

void page_01_err_num_refre(void)
{
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", sim.err_expected);
    update_label_by_name(page_01_main_obj, page_01_main_len, "reject_num_label", "%s", buf);
}

void page_01_curr_img_refre(void)
{

    lv_obj_t* tmp_curr_img = find_obj_by_name("curr_USD_img", page_01_main_obj, page_01_main_len);
    lv_img_set_src(tmp_curr_img, get_currency_img(Machine_para.curr_code));
}


#define CURR_MAX_ITEMS           MAX_CURRENCIES
#define CURR_SEL_W               288
#define CURR_SEL_H               400
#define CURR_VIEW_X              288
#define CURR_VIEW_Y              0
#define CURR_VIEW_W              992
#define CURR_VIEW_H              400

#define CURR_CARD_W              200
#define CURR_CARD_H              265
#define CURR_CARD_GAP            24
#define CURR_CARD_STRIDE         (CURR_CARD_W + CURR_CARD_GAP)
#define CURR_CARD_FIRST_X        36
#define CURR_CARD_Y              46
#define CURR_CARD_PAD_RIGHT      580
#define CURR_SEL_NEXT_EXTRA_GAP  10
#define CURR_LEFT_PEEK_W         ((CURR_CARD_W * 2) / 3)

#define CURR_TRACK_Y             365
#define CURR_TRACK_H             6
#define CURR_DRAG_THRESHOLD      14
#define CURR_FLING_FACTOR        1
#define CURR_FLING_TRIGGER       6
#define CURR_FLING_MAX           (CURR_CARD_STRIDE / 2)
#define CURR_SNAP_IDLE_MS        500
#define CURR_OVERSCROLL_SNAP_MS  300
#define CURR_SNAP_COMMIT_PX      (CURR_CARD_STRIDE / 4)

#define CURR_LEFT_BG_COLOR       0xEDF0F4
#define CURR_RIGHT_BG_COLOR      0xF4F5F7
#define CURR_CARD_BG_UNSEL       0xF7F7F7
#define CURR_TEXT_SEL            0xFC4000
#define CURR_TEXT_UNSEL          0xBEBFC0
#define CURR_IMG_UNSEL           0xCDCED0
#define CURR_TRACK_BG            0xEBECED
#define CURR_TRACK_FG            0x75A2DF

#define CURR_CARD_SELECTED_BG_PATH      "L:/usr/local/share/lvgl_data/selected_card.png"
#define CURR_GRID_SELECTED_MARK_PATH    "L:/usr/local/share/lvgl_data/view_selected.png"
#define CURR_CARD_SELECTED_BG_OFS_X     18
#define CURR_CARD_SELECTED_BG_OFS_TOP   16

#define CURR_CARD_SEL_W          ((CURR_CARD_W * 11) / 10)
#define CURR_CARD_SEL_H          ((CURR_CARD_H * 11) / 10)

#define CURR_BTN_W               70
#define CURR_BTN_H               36
#define CURR_BTN_Y               358
#define CURR_VIEW_BTN_X          18
#define CURR_FAV_BTN_X           113
#define CURR_BACK_BTN_X          207

#define CURR_FOCUS_BOX_X         24
#define CURR_FOCUS_BOX_Y         54
#define CURR_FOCUS_BOX_W         245
#define CURR_FOCUS_BOX_H         279

#define CURR_LEFT_IMG_ALIGN_Y    72
#define CURR_LEFT_IMG_ALIGN_X    2
#define CURR_LEFT_CODE_X         54
#define CURR_LEFT_CODE_Y         214
#define CURR_LEFT_CODE_DECOR_X   128
#define CURR_LEFT_CODE_DECOR_Y   228
#define CURR_LEFT_NO_X           184
#define CURR_LEFT_NO_Y           300

#define CURR_GRID_COLS           6
#define CURR_GRID_CELL_W         158
#define CURR_GRID_CELL_H         92
#define CURR_GRID_ITEM_W         132
#define CURR_GRID_START_X        8
#define CURR_GRID_START_Y        10
#define CURR_GRID_ROW_STEP       92
#define CURR_GRID_TEXT_BOTTOM    -4
#define CURR_GRID_GROUP_OFS_X    25
#define CURR_GRID_FLAG_Y         (-6)
#define CURR_GRID_FAV_X          -3
#define CURR_GRID_FAV_Y          -2

#define CURR_FAV_BTN_IN_CARD_X   139
#define CURR_FAV_BTN_IN_CARD_Y   13
#define CURR_FAV_BTN_IN_CARD_W   49
#define CURR_FAV_BTN_IN_CARD_H   49

#define CURR_FLAG_TARGET_W       82
#define CURR_FLAG_Y_IN_CARD      19

#define CURR_VIEW_MODE_CARD      0
#define CURR_VIEW_MODE_GRID      1

typedef struct {
    lv_obj_t* selected_bg;
    lv_obj_t* card;
    lv_obj_t* img;
    lv_obj_t* name;
    lv_obj_t* no;
    lv_obj_t* fav_btn;
    lv_obj_t* fav_icon;
    int base_x;
    int base_y;
    int abs_idx;
} curr_card_t;

typedef struct {
    lv_obj_t* item;
    lv_obj_t* img;
    lv_obj_t* selected_mark;
    lv_obj_t* name;
    lv_obj_t* fav_btn;
    lv_obj_t* fav_icon;
    int abs_idx;
} curr_grid_item_t;
#define UI_STATE_STORE_PATH "/etc/ui_state/ui_state.cfg"  //添加掉电保存
#define UI_STATE_MAGIC      0x55495354U   /* 'UIST' */
#define UI_STATE_VERSION    1

typedef struct {
    int view_mode;                      /* 0=card 1=grid */
    int fav_only;                       /* 0=off 1=on */
    int selected_abs_idx;               
    int fav_count;
    char fav_codes[CURR_MAX_ITEMS][4];
} page07_state_t;

typedef struct {
    int reserved05_enable;              
    int reserved06_enable;             
} common_page_state_t;

typedef struct {
    int detail_section;
} page01_state_t;

typedef struct {
    int reserved18_enable;
} page18_state_t;

typedef struct {
    unsigned int magic;
    unsigned int version;
    page01_state_t page01;
    page07_state_t page07;
    common_page_state_t page05;
    common_page_state_t page06;
    page18_state_t page18;
} ui_persist_state_t;

static ui_persist_state_t g_ui_state;
static bool g_ui_state_loaded = false;

static curr_card_t g_curr_cards[CURR_MAX_ITEMS];
static curr_grid_item_t g_curr_grid_items[CURR_MAX_ITEMS];
static char g_curr_fav_codes[CURR_MAX_ITEMS][4];
static int g_curr_fav_cnt = 0;

static int g_curr_visible_idx[CURR_MAX_ITEMS];
static int g_curr_visible_cnt = 0;

static int g_curr_sel_abs_idx = 0;
static int g_curr_sel_vis_idx = 0;
static int g_curr_view_mode = CURR_VIEW_MODE_CARD;
static bool g_curr_fav_only = false;
static int g_curr_pending_abs_idx = -1;
static char g_curr_pending_code[4] = {0};

static bool g_curr_touch_active = false;
static bool g_curr_touch_dragging = false;
static int g_curr_touch_start_scroll = 0;
static int g_curr_touch_start_vis_idx = 0;
static lv_point_t g_curr_touch_start_pt;
static lv_point_t g_curr_touch_last_pt;
static int g_curr_touch_last_dx = 0;
static uint32_t g_curr_last_drag_tick = 0;
static lv_timer_t* g_curr_snap_timer = NULL;
static int g_curr_snap_target_vis_idx = -1;

static lv_obj_t* g_curr_root = NULL;
static lv_obj_t* g_curr_left_panel = NULL;
static lv_obj_t* g_curr_left_img = NULL;
static lv_obj_t* g_curr_left_code = NULL;
static lv_obj_t* g_curr_left_code_decor = NULL;
static lv_obj_t* g_curr_left_no = NULL;

static lv_obj_t* g_curr_btn_view = NULL;
static lv_obj_t* g_curr_btn_view_label = NULL;
static lv_obj_t* g_curr_btn_fav = NULL;
static lv_obj_t* g_curr_btn_fav_label = NULL;
static lv_obj_t* g_curr_btn_back = NULL;
static lv_obj_t* g_curr_btn_back_label = NULL;

static lv_obj_t* g_curr_right_area = NULL;
static lv_obj_t* g_curr_card_layer = NULL;
static lv_obj_t* g_curr_grid_layer = NULL;

static lv_obj_t* g_curr_list = NULL;
static lv_obj_t* g_curr_track = NULL;
static lv_obj_t* g_curr_thumb = NULL;

static lv_obj_t* g_curr_grid_scroll = NULL;
static lv_obj_t* g_curr_empty_label = NULL;

static void curr_refresh_right_views(void);
static int ui_state_ensure_store_dir(void);
static void ui_state_set_defaults(void);
static void ui_state_load_from_file(void);
static void ui_state_save_to_file(void);

static void page07_state_pull_from_runtime(void);
static void page07_state_apply_to_runtime(void);
static void page01_state_pull_from_runtime(void);
static void page01_state_apply_to_runtime(void);

static void page05_state_pull_from_runtime(void);
static void page05_state_apply_to_runtime(void);

static void page06_state_pull_from_runtime(void);
static void page06_state_apply_to_runtime(void);
static void page18_state_pull_from_runtime(void);
static void page18_state_apply_to_runtime(void);
void ui_state_apply_common_runtime(void);
void ui_state_save_popup_auto_state(void);
void ui_state_save_pure_count_state(void);
bool ui_state_pure_count_is_enabled(void);
int ui_state_page01_detail_section_get(void);
void ui_state_save_page01_detail_section(void);

static void curr_apply_selected_style(void);
static void curr_style_back_button(void);
bool page_07_curr_set_pending_result(uint8_t status);

static bool curr_has_currency_code(const char* code);
static bool curr_add_favorite_code(const char* code);
static int curr_find_abs_idx_by_code(const char* code);
static void ui_state_set_defaults(void) //默认掉电配置
{
    memset(&g_ui_state, 0, sizeof(g_ui_state));
    g_ui_state.magic = UI_STATE_MAGIC;
    g_ui_state.version = UI_STATE_VERSION;

    g_ui_state.page01.detail_section = PAGE_01_DETAIL_SECTION_A;
    g_ui_state.page07.view_mode = CURR_VIEW_MODE_CARD;
    g_ui_state.page07.fav_only = 0;
    g_ui_state.page07.selected_abs_idx = 0;
    g_ui_state.page07.fav_count = 0;
    g_ui_state.page06.reserved06_enable = 1;
    g_ui_state.page18.reserved18_enable = 0;
}

static void page01_state_pull_from_runtime(void)
{
    g_ui_state.page01.detail_section = (int)page_01_detail_section_get();
}

static void page01_state_apply_to_runtime(void)
{
    int section = g_ui_state.page01.detail_section;

    if (section < PAGE_01_DETAIL_SECTION_A || section > PAGE_01_DETAIL_SECTION_C) {
        section = PAGE_01_DETAIL_SECTION_A;
    }

    page_01_detail_section_set((page_01_detail_section_t)section, false);
}

static void page07_state_pull_from_runtime(void)
{
    int i;

    g_ui_state.page07.view_mode = g_curr_view_mode;
    g_ui_state.page07.fav_only = g_curr_fav_only ? 1 : 0;
    g_ui_state.page07.selected_abs_idx = g_curr_sel_abs_idx;
    g_ui_state.page07.fav_count = g_curr_fav_cnt;

    memset(g_ui_state.page07.fav_codes, 0, sizeof(g_ui_state.page07.fav_codes));
    for (i = 0; i < g_curr_fav_cnt && i < CURR_MAX_ITEMS; i++) {
        memcpy(g_ui_state.page07.fav_codes[i], g_curr_fav_codes[i], 4);
    }
}
static void page07_state_apply_to_runtime(void)
{
    int i;

    g_curr_view_mode = g_ui_state.page07.view_mode;
    if (g_curr_view_mode != CURR_VIEW_MODE_CARD && g_curr_view_mode != CURR_VIEW_MODE_GRID) {
        g_curr_view_mode = CURR_VIEW_MODE_CARD;
        ui_state_save_to_file();
    }

    g_curr_fav_only = (g_ui_state.page07.fav_only != 0);

    g_curr_fav_cnt = 0;
    memset(g_curr_fav_codes, 0, sizeof(g_curr_fav_codes));
    for (i = 0; i < g_ui_state.page07.fav_count && i < CURR_MAX_ITEMS; i++) {
        if (!curr_has_currency_code(g_ui_state.page07.fav_codes[i])) continue;
        memcpy(g_curr_fav_codes[g_curr_fav_cnt], g_ui_state.page07.fav_codes[i], 4);
        g_curr_fav_cnt++;
    }

    if (g_ui_state.page07.selected_abs_idx >= 0 &&
        g_ui_state.page07.selected_abs_idx < Machine_para.currency_count) {
        g_curr_sel_abs_idx = g_ui_state.page07.selected_abs_idx;
    } else {
        g_curr_sel_abs_idx = curr_find_abs_idx_by_code(Machine_para.curr_code);
    }
}
static void page05_state_pull_from_runtime(void)
{
    
}

static void page05_state_apply_to_runtime(void)
{
}

static void page06_state_pull_from_runtime(void)
{
    g_ui_state.page06.reserved06_enable = fault_popup_get_auto_enabled() ? 1 : 0;
}

static void page06_state_apply_to_runtime(void)
{
    fault_popup_set_auto_enabled(g_ui_state.page06.reserved06_enable != 0);
}   //留接口

static void page18_state_pull_from_runtime(void)
{
    g_ui_state.page18.reserved18_enable = smart_island_pure_count_is_enabled() ? 1 : 0;
}

static void page18_state_apply_to_runtime(void)
{
    smart_island_set_pure_count_enabled(g_ui_state.page18.reserved18_enable != 0);
}
static int ui_state_ensure_store_dir(void)
{
    const char *slash;
    char dir_path[128];
    size_t dir_len;

    slash = strrchr(UI_STATE_STORE_PATH, '/');
    if (slash == NULL) return -1;

    dir_len = (size_t)(slash - UI_STATE_STORE_PATH);
    if (dir_len == 0 || dir_len >= sizeof(dir_path)) return -1;

    memcpy(dir_path, UI_STATE_STORE_PATH, dir_len);
    dir_path[dir_len] = '\0';

    if (access(dir_path, F_OK) == 0) {
        return 0;
    }

    if (mkdir(dir_path, 0775) == 0) {
#if LV_DEBUG
        printf("[ui_state] mkdir ok: %s\n", dir_path);
#endif
        return 0;
    }
#if LV_DEBUG
    printf("[ui_state] mkdir failed: %s err=%s\n", dir_path, strerror(errno));
#endif
    return -1;
}

static int curr_abs_i32(int v)
{
    return (v >= 0) ? v : -v;
}

static bool curr_code_eq3(const char* a, const char* b)
{
    return (a && b && strncmp(a, b, 3) == 0);
}

static int curr_find_abs_idx_by_code(const char* code)
{
    for (int i = 0; i < Machine_para.currency_count && i < CURR_MAX_ITEMS; i++) {
        if (curr_code_eq3(code, Machine_para.currencies[i])) return i;
    }
    return 0;
}
static bool curr_has_currency_code(const char* code)
{
    if (code == NULL || code[0] == '\0') return false;

    for (int i = 0; i < Machine_para.currency_count && i < CURR_MAX_ITEMS; i++) {
        if (curr_code_eq3(code, Machine_para.currencies[i])) return true;
    }
    return false;
}

static void ui_state_load_from_file(void)
{
    FILE *fp;
    char line[128];

    ui_state_set_defaults();

    fp = fopen(UI_STATE_STORE_PATH, "r");
#if LV_DEBUG
    printf("[ui_state] load from: %s\n", UI_STATE_STORE_PATH);
#endif
    if (fp == NULL) {
#if LV_DEBUG
        printf("[ui_state] load skipped, fopen failed: %s\n", strerror(errno));
#endif
        g_ui_state_loaded = true;
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "magic=", 6) == 0) {
            g_ui_state.magic = (unsigned int)strtoul(line + 6, NULL, 0);
        } else if (strncmp(line, "version=", 8) == 0) {
            g_ui_state.version = (unsigned int)strtoul(line + 8, NULL, 0);
        } else if (strncmp(line, "p01_detail_section=", 19) == 0) {
            g_ui_state.page01.detail_section = atoi(line + 19);
        } else if (strncmp(line, "p07_view_mode=", 14) == 0) {
            g_ui_state.page07.view_mode = atoi(line + 14);
        } else if (strncmp(line, "p07_fav_only=", 13) == 0) {
            g_ui_state.page07.fav_only = atoi(line + 13);
        } else if (strncmp(line, "p07_selected_abs_idx=", 21) == 0) {
            g_ui_state.page07.selected_abs_idx = atoi(line + 21);
        } else if (strncmp(line, "p07_fav_count=", 14) == 0) {
            g_ui_state.page07.fav_count = atoi(line + 14);
            if (g_ui_state.page07.fav_count < 0) g_ui_state.page07.fav_count = 0;
            if (g_ui_state.page07.fav_count > CURR_MAX_ITEMS) g_ui_state.page07.fav_count = CURR_MAX_ITEMS;
        } else if (strncmp(line, "p07_fav", 7) == 0) {
            int idx = -1;
            char code[4] = {0};
            if (sscanf(line, "p07_fav%d=%3[A-Z]", &idx, code) == 2) {
                if (idx >= 0 && idx < CURR_MAX_ITEMS && curr_has_currency_code(code)) {
                    memcpy(g_ui_state.page07.fav_codes[idx], code, 4);
                }
            }
        } else if (strncmp(line, "p05_reserved=", 13) == 0) {
            g_ui_state.page05.reserved05_enable = atoi(line + 13);
        } else if (strncmp(line, "p06_reserved=", 13) == 0) {
            g_ui_state.page06.reserved06_enable = atoi(line + 13);
        } else if (strncmp(line, "p18_reserved=", 13) == 0) {
            g_ui_state.page18.reserved18_enable = atoi(line + 13);
        }
    }

    fclose(fp);

    if (g_ui_state.magic != UI_STATE_MAGIC) {
#if LV_DEBUG
        printf("[ui_state] invalid magic, fallback defaults\n");
#endif
        ui_state_set_defaults();
    }

    g_ui_state_loaded = true;
}

void ui_state_apply_common_runtime(void)
{
    if (!g_ui_state_loaded) {
        ui_state_load_from_file();
    }

    page01_state_apply_to_runtime();
    page05_state_apply_to_runtime();
    page06_state_apply_to_runtime();
    page18_state_apply_to_runtime();
}

void ui_state_save_popup_auto_state(void)
{
    if (!g_ui_state_loaded) {
        ui_state_load_from_file();
    }

    page06_state_pull_from_runtime();
    ui_state_save_to_file();
}

void ui_state_save_pure_count_state(void)
{
    if (!g_ui_state_loaded) {
        ui_state_load_from_file();
    }

    page18_state_pull_from_runtime();
    ui_state_save_to_file();
}

bool ui_state_pure_count_is_enabled(void)
{
    if (!g_ui_state_loaded) {
        ui_state_load_from_file();
    }

    page18_state_apply_to_runtime();
    return smart_island_pure_count_is_enabled();
}

int ui_state_page01_detail_section_get(void)
{
    if (!g_ui_state_loaded) {
        ui_state_load_from_file();
    }

    return g_ui_state.page01.detail_section;
}

void ui_state_save_page01_detail_section(void)
{
    if (!g_ui_state_loaded) {
        ui_state_load_from_file();
    }

    page01_state_pull_from_runtime();
    ui_state_save_to_file();
}

static void ui_state_save_to_file(void)
{
    char tmp_path[128];
    int fd;
    FILE *fp;
    int dir_fd;
    const char *slash;
    char dir_path[128];
    int i;

    page01_state_pull_from_runtime();
    page07_state_pull_from_runtime();
    page05_state_pull_from_runtime();
    page06_state_pull_from_runtime();
    page18_state_pull_from_runtime();

    if (ui_state_ensure_store_dir() != 0) {
        printf("[ui_state] save aborted, store dir not ready\n");
        return;
    }

    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", UI_STATE_STORE_PATH);

#if LV_DEBUG
    printf("[ui_state] save to: %s\n", UI_STATE_STORE_PATH);
#endif

    fd = open(tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("[ui_state] open tmp file failed: %s\n", strerror(errno));
        return;
    }

    fp = fdopen(fd, "w");
    if (fp == NULL) {
        printf("[ui_state] fdopen failed: %s\n", strerror(errno));
        close(fd);
        return;
    }

    fprintf(fp, "magic=%u\n", g_ui_state.magic);
    fprintf(fp, "version=%u\n", g_ui_state.version);
    fprintf(fp, "p01_detail_section=%d\n", g_ui_state.page01.detail_section);

    fprintf(fp, "p07_view_mode=%d\n", g_ui_state.page07.view_mode);
    fprintf(fp, "p07_fav_only=%d\n", g_ui_state.page07.fav_only);
    fprintf(fp, "p07_selected_abs_idx=%d\n", g_ui_state.page07.selected_abs_idx);
    fprintf(fp, "p07_fav_count=%d\n", g_ui_state.page07.fav_count);

    for (i = 0; i < g_ui_state.page07.fav_count && i < CURR_MAX_ITEMS; i++) {
        fprintf(fp, "p07_fav%d=%s\n", i, g_ui_state.page07.fav_codes[i]);
    }

    fprintf(fp, "p05_reserved=%d\n", g_ui_state.page05.reserved05_enable);
    fprintf(fp, "p06_reserved=%d\n", g_ui_state.page06.reserved06_enable);
    fprintf(fp, "p18_reserved=%d\n", g_ui_state.page18.reserved18_enable);

    fflush(fp);
    fsync(fd);
    fclose(fp);

    if (rename(tmp_path, UI_STATE_STORE_PATH) != 0) {
        printf("[ui_state] rename failed: %s\n", strerror(errno));
        unlink(tmp_path);
        return;
    }

    slash = strrchr(UI_STATE_STORE_PATH, '/');
    if (slash != NULL) {
        size_t dir_len = (size_t)(slash - UI_STATE_STORE_PATH);
        if (dir_len < sizeof(dir_path)) {
            memcpy(dir_path, UI_STATE_STORE_PATH, dir_len);
            dir_path[dir_len] = '\0';

            dir_fd = open(dir_path, O_RDONLY | O_DIRECTORY);
            if (dir_fd >= 0) {
                fsync(dir_fd);
                close(dir_fd);
            }
        }
    }
}
static bool curr_is_favorite_code(const char* code)
{
    for (int i = 0; i < g_curr_fav_cnt; i++) {
        if (curr_code_eq3(code, g_curr_fav_codes[i])) return true;
    }
    return false;
}

static bool curr_add_favorite_code(const char* code)
{
    if (code == NULL || code[0] == '\0') return false;
    if (curr_is_favorite_code(code)) return true;
    if (g_curr_fav_cnt >= CURR_MAX_ITEMS) return false;

    g_curr_fav_codes[g_curr_fav_cnt][0] = code[0];
    g_curr_fav_codes[g_curr_fav_cnt][1] = code[1];
    g_curr_fav_codes[g_curr_fav_cnt][2] = code[2];
    g_curr_fav_codes[g_curr_fav_cnt][3] = '\0';
    g_curr_fav_cnt++;
    return true;
}

static void curr_remove_favorite_code(const char* code)
{
    for (int i = 0; i < g_curr_fav_cnt; i++) {
        if (!curr_code_eq3(code, g_curr_fav_codes[i])) continue;
        for (int j = i; j < g_curr_fav_cnt - 1; j++) {
            memcpy(g_curr_fav_codes[j], g_curr_fav_codes[j + 1], 4);
        }
        g_curr_fav_cnt--;
        return;
    }
}

static bool curr_is_favorite_abs_idx(int abs_idx)
{
    if (abs_idx < 0 || abs_idx >= Machine_para.currency_count) return false;
    return curr_is_favorite_code(Machine_para.currencies[abs_idx]);
}

static void curr_toggle_favorite_abs_idx(int abs_idx)
{
    if (abs_idx < 0 || abs_idx >= Machine_para.currency_count) return;

#if LV_DEBUG
    printf("[curr_fav] toggle abs_idx=%d code=%s\n",
           abs_idx, Machine_para.currencies[abs_idx]);
#endif

    if (curr_is_favorite_abs_idx(abs_idx)) {
        curr_remove_favorite_code(Machine_para.currencies[abs_idx]);
    } else {
        curr_add_favorite_code(Machine_para.currencies[abs_idx]);
    }

    ui_state_save_to_file();
}

static void curr_set_img_target_width(lv_obj_t* img, const char* code, int target_w)
{
    lv_img_header_t info;
    if (lv_img_decoder_get_info(get_currency_img(code), &info) == LV_RES_OK && info.w > 0) {
        int zoom = (target_w * 256) / (int)info.w;
        if (zoom < 32) zoom = 32;
        lv_img_set_zoom(img, zoom);
    }
}

static void curr_set_image_unselected_style(lv_obj_t* img)
{
    lv_obj_set_style_img_recolor(img, lv_color_hex(CURR_IMG_UNSEL), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_40, 0);
}

static void curr_set_image_selected_style(lv_obj_t* img)
{
    lv_obj_set_style_img_recolor(img, lv_color_hex(CURR_IMG_UNSEL), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_0, 0);
    lv_obj_set_style_img_opa(img, LV_OPA_COVER, 0);
}

static void curr_update_visible_idx(void)
{
    g_curr_visible_cnt = 0;
    int total = Machine_para.currency_count;
    if (total > CURR_MAX_ITEMS) total = CURR_MAX_ITEMS;

    for (int i = 0; i < total; i++) {
        bool keep = true;
        if (g_curr_fav_only) keep = curr_is_favorite_abs_idx(i);
        if (!keep) continue;
        g_curr_visible_idx[g_curr_visible_cnt++] = i;
    }
}

static int curr_find_visible_pos_by_abs(int abs_idx)
{
    for (int i = 0; i < g_curr_visible_cnt; i++) {
        if (g_curr_visible_idx[i] == abs_idx) return i;
    }
    return 0;
}

static int curr_scroll_x_abs(void)
{
    if (g_curr_list == NULL) return 0;
    return curr_abs_i32(lv_obj_get_scroll_x(g_curr_list));
}

static int curr_get_max_scroll(void)
{
    int cnt = g_curr_visible_cnt;
    if (cnt <= 0) return 0;
    int content_w = CURR_CARD_FIRST_X
                  + (cnt - 1) * CURR_CARD_STRIDE
                  + CURR_CARD_W
                  + CURR_CARD_PAD_RIGHT
                  + CURR_SEL_NEXT_EXTRA_GAP;
    int max_scroll = content_w - CURR_VIEW_W;
    if (max_scroll < 0) max_scroll = 0;
    return max_scroll;
}

static int curr_highlight_idx_from_scroll(int sx)
{
    if (g_curr_visible_cnt <= 0) return 0;
    if (g_curr_visible_cnt == 1) return 0;

    int max_scroll = curr_get_max_scroll();
    if (sx <= 2) return 0;
    if (sx >= max_scroll - 2) return g_curr_visible_cnt - 1;

    int idx = 1 + (sx + CURR_CARD_STRIDE / 2) / CURR_CARD_STRIDE;
    if (idx < 1) idx = 1;
    if (idx >= g_curr_visible_cnt) idx = g_curr_visible_cnt - 1;
    return idx;
}

static int curr_scroll_from_highlight_idx(int vis_idx)
{
    if (g_curr_visible_cnt <= 0) return 0;

    int max_scroll = curr_get_max_scroll();
    if (vis_idx <= 0) return 0;
    if (vis_idx >= g_curr_visible_cnt - 1) return max_scroll;

    /* Keep only one left-side card visible, with about 2/3 of that unselected card peeking in. */
    int sx = CURR_CARD_FIRST_X
           + (vis_idx - 1) * CURR_CARD_STRIDE
           + (CURR_CARD_W - CURR_LEFT_PEEK_W);

    if (sx < 0) sx = 0;
    if (sx > max_scroll) sx = max_scroll;
    return sx;
}

static int curr_get_track_base_width(void)
{
    int thumb_w;

    if (g_curr_visible_cnt <= 0) return CURR_VIEW_W;

    thumb_w = CURR_VIEW_W / g_curr_visible_cnt;
    if (thumb_w < 36) thumb_w = 36;
    if (thumb_w > CURR_VIEW_W) thumb_w = CURR_VIEW_W;
    return thumb_w;
}

static void curr_update_track_by_scroll(int sx)
{
    if (g_curr_thumb == NULL || g_curr_visible_cnt <= 0) return;

    int thumb_w = CURR_VIEW_W / g_curr_visible_cnt;
    if (thumb_w < 36) thumb_w = 36;
    if (thumb_w > CURR_VIEW_W) thumb_w = CURR_VIEW_W;

    int max_scroll = curr_get_max_scroll();
    if (max_scroll <= 0 || g_curr_visible_cnt <= 1) {
        lv_obj_set_size(g_curr_thumb, CURR_VIEW_W, CURR_TRACK_H);
        lv_obj_set_pos(g_curr_thumb, 0, CURR_TRACK_Y);
        return;
    }

    if (sx < 0) sx = 0;
    if (sx > max_scroll) sx = max_scroll;

    int idx = g_curr_sel_vis_idx;
    if (idx < 0) idx = 0;
    if (idx >= g_curr_visible_cnt) idx = g_curr_visible_cnt - 1;

    // 按高亮卡片索引均分滑块位置，避免尾部两档挤在一起
    int x = (idx * (CURR_VIEW_W - thumb_w)) / (g_curr_visible_cnt - 1);

    lv_obj_set_size(g_curr_thumb, thumb_w, CURR_TRACK_H);
    lv_obj_set_pos(g_curr_thumb, x, CURR_TRACK_Y);
}


static void curr_apply_overscroll_visual(int overscroll_px)
{
    int base_w;
    int shrink;
    int thumb_w;
    int thumb_x;

    if (g_curr_list == NULL) return;

    lv_obj_set_x(g_curr_list, overscroll_px);

    if (g_curr_thumb == NULL) return;

    if (overscroll_px == 0) {
        curr_update_track_by_scroll(curr_scroll_x_abs());
        return;
    }

    base_w = curr_get_track_base_width();
    shrink = curr_abs_i32(overscroll_px) / 2;
    if (shrink > base_w - 18) shrink = base_w - 18;
    if (shrink < 0) shrink = 0;
    thumb_w = base_w - shrink;
    if (thumb_w < 18) thumb_w = 18;

    thumb_x = (overscroll_px > 0) ? 0 : (CURR_VIEW_W - thumb_w);
    lv_obj_set_size(g_curr_thumb, thumb_w, CURR_TRACK_H);
    lv_obj_set_pos(g_curr_thumb, thumb_x, CURR_TRACK_Y);
}

static void curr_reset_overscroll_visual(void)
{
    curr_apply_overscroll_visual(0);
}

static void curr_overscroll_anim_x_cb(void* var, int32_t v)
{
    (void)var;
    curr_apply_overscroll_visual((int)v);
}

static void curr_animate_overscroll_back(void)
{
    int start = lv_obj_get_x(g_curr_list);
    lv_anim_t a;

    if (g_curr_list == NULL || start == 0) {
        curr_reset_overscroll_visual();
        return;
    }

    lv_anim_init(&a);
    lv_anim_set_var(&a, g_curr_list);
    lv_anim_set_exec_cb(&a, curr_overscroll_anim_x_cb);
    lv_anim_set_values(&a, start, 0);
    lv_anim_set_time(&a, 320);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_start(&a);
}



static void curr_set_left_info_by_abs(int abs_idx)
{
    if (abs_idx < 0 || abs_idx >= Machine_para.currency_count) return;
    lv_img_set_src(g_curr_left_img, get_currency_img(Machine_para.currencies[abs_idx]));
    lv_obj_align(g_curr_left_img, LV_ALIGN_TOP_MID, CURR_LEFT_IMG_ALIGN_X, CURR_LEFT_IMG_ALIGN_Y);
    lv_label_set_text_fmt(g_curr_left_code, "%s", Machine_para.currencies[abs_idx]);
    if (g_curr_left_code_decor) {
        lv_label_set_text_fmt(g_curr_left_code_decor, "%s", Machine_para.currencies[abs_idx]);
    }
    lv_label_set_text_fmt(g_curr_left_no, "NO.%02d", abs_idx + 1);
}

static void curr_style_view_button(void)
{
    if (g_curr_btn_view == NULL || g_curr_btn_view_label == NULL) return;

    lv_obj_set_style_radius(g_curr_btn_view, 10, 0);
    lv_obj_set_style_bg_opa(g_curr_btn_view, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_curr_btn_view, lv_color_hex(0x0073FF), 0);
    lv_obj_set_style_border_width(g_curr_btn_view, 0, 0);
    lv_obj_set_style_shadow_width(g_curr_btn_view, 12, 0);
    lv_obj_set_style_shadow_opa(g_curr_btn_view, LV_OPA_10, 0);
    lv_obj_set_style_text_color(g_curr_btn_view_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(g_curr_btn_view_label,
                      (g_curr_view_mode == CURR_VIEW_MODE_CARD) ? "CARD" : "VIEW");
    lv_obj_center(g_curr_btn_view_label);
}

static void curr_style_fav_button(void)
{
    if (g_curr_btn_fav == NULL || g_curr_btn_fav_label == NULL) return;

    lv_obj_set_style_radius(g_curr_btn_fav, 10, 0);
    lv_obj_set_style_bg_opa(g_curr_btn_fav, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_curr_btn_fav,
                              g_curr_fav_only ? lv_color_hex(0xE9DEBD) : lv_color_hex(0x8F8F8F), 0);
    lv_obj_set_style_border_width(g_curr_btn_fav, 0, 0);
    lv_obj_set_style_shadow_width(g_curr_btn_fav, 0, 0);
    lv_obj_set_style_shadow_opa(g_curr_btn_fav, LV_OPA_0, 0);
    lv_obj_set_style_text_color(g_curr_btn_fav_label,
                                g_curr_fav_only ? lv_color_hex(0x8A6A11) : lv_color_hex(0x5F5F5F), 0);
    lv_label_set_text(g_curr_btn_fav_label, "FAV");
    lv_obj_center(g_curr_btn_fav_label);
}

static void curr_style_back_button(void)
{
    if (g_curr_btn_back == NULL || g_curr_btn_back_label == NULL) return;

    lv_obj_set_style_radius(g_curr_btn_back, 10, 0);
    lv_obj_set_style_bg_opa(g_curr_btn_back, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(g_curr_btn_back, lv_color_hex(0xD9D9D9), 0);
    lv_obj_set_style_border_width(g_curr_btn_back, 0, 0);
    lv_obj_set_style_shadow_width(g_curr_btn_back, 0, 0);
    lv_obj_set_style_shadow_opa(g_curr_btn_back, LV_OPA_0, 0);
    lv_obj_set_style_text_color(g_curr_btn_back_label, lv_color_hex(0x000000), 0);
    lv_label_set_text(g_curr_btn_back_label, "BACK");
    lv_obj_center(g_curr_btn_back_label);
}

static void curr_refresh_left_buttons(void)
{
    curr_style_view_button();
    curr_style_fav_button();
    curr_style_back_button();
}

static void curr_back_btn_click_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_switch(UI_PAGE_MAIN);
}

static void curr_scroll_to_raw(int x, bool anim, bool apply_style)
{
    int max_scroll = curr_get_max_scroll();
    if (x < 0) x = 0;
    if (x > max_scroll) x = max_scroll;

    curr_reset_overscroll_visual();
    lv_obj_scroll_to_x(g_curr_list, x, anim ? LV_ANIM_ON : LV_ANIM_OFF);

    if (g_curr_visible_cnt > 0) {
        int vis_idx = curr_highlight_idx_from_scroll(x);
        int abs_idx = g_curr_visible_idx[vis_idx];
        if (abs_idx != g_curr_sel_abs_idx) {
            g_curr_sel_abs_idx = abs_idx;
            g_curr_sel_vis_idx = vis_idx;
            if (apply_style) {
                curr_apply_selected_style();
            }
        }
    }
    curr_update_track_by_scroll(x);
}

static void curr_scroll_to_visible_idx(int vis_idx, bool anim, bool apply_style)
{
    if (g_curr_visible_cnt <= 0) return;
    if (vis_idx < 0) vis_idx = 0;
    if (vis_idx >= g_curr_visible_cnt) vis_idx = g_curr_visible_cnt - 1;
    curr_scroll_to_raw(curr_scroll_from_highlight_idx(vis_idx), anim, apply_style);
}

static int curr_pick_nearest_visible_idx(void)
{
    return curr_highlight_idx_from_scroll(curr_scroll_x_abs());
}

static int curr_get_release_direction(void)
{
    int scroll_delta;

    scroll_delta = curr_scroll_x_abs() - g_curr_touch_start_scroll;
    if (scroll_delta > 0) return 1;
    if (scroll_delta < 0) return -1;
    if (g_curr_touch_last_dx < 0) return 1;
    if (g_curr_touch_last_dx > 0) return -1;
    return 0;
}

static int curr_pick_release_visible_idx(void)
{
    int step;
    int vis_idx;
    int drag_px;
    int dir;

    if (g_curr_visible_cnt <= 0) return 0;

    drag_px = curr_abs_i32(curr_scroll_x_abs() - g_curr_touch_start_scroll);
    dir = curr_get_release_direction();
    if (dir == 0) return g_curr_touch_start_vis_idx;

    step = (drag_px >= CURR_CARD_STRIDE * 2) ? 2 : 1;
    vis_idx = g_curr_touch_start_vis_idx + dir * step;

    if (vis_idx < 0) vis_idx = 0;
    if (vis_idx >= g_curr_visible_cnt) vis_idx = g_curr_visible_cnt - 1;
    return vis_idx;
}

static int curr_calc_release_scroll_target(void)
{
    int drag_px;
    int dir;
    int target_scroll;

    drag_px = curr_abs_i32(curr_scroll_x_abs() - g_curr_touch_start_scroll);
    dir = curr_get_release_direction();
    if (dir == 0) return curr_scroll_x_abs();

    if (drag_px >= CURR_CARD_STRIDE * 2) {
        target_scroll = curr_scroll_x_abs() + dir * (CURR_CARD_STRIDE * 2);
    } else if (drag_px >= CURR_CARD_STRIDE) {
        target_scroll = curr_scroll_x_abs() + dir * CURR_CARD_STRIDE;
    } else {
        target_scroll = curr_scroll_from_highlight_idx(curr_pick_release_visible_idx());
    }

    if (target_scroll < 0) target_scroll = 0;
    if (target_scroll > curr_get_max_scroll()) target_scroll = curr_get_max_scroll();
    return target_scroll;
}

static void curr_snap_timer_cb(lv_timer_t* t)
{
    int vis_idx;

    (void)t;
    g_curr_snap_timer = NULL;
    if (g_curr_visible_cnt <= 0) return;

    vis_idx = curr_pick_nearest_visible_idx();
    g_curr_snap_target_vis_idx = -1;
    g_curr_sel_vis_idx = vis_idx;
    g_curr_sel_abs_idx = g_curr_visible_idx[vis_idx];
    curr_apply_selected_style();
    curr_scroll_to_visible_idx(vis_idx, true, true);
}

static void curr_start_snap_timer(uint32_t ms)
{
    if (g_curr_snap_timer) {
        lv_timer_del(g_curr_snap_timer);
        g_curr_snap_timer = NULL;
    }
    g_curr_snap_timer = lv_timer_create(curr_snap_timer_cb, ms, NULL);
    if (g_curr_snap_timer) lv_timer_set_repeat_count(g_curr_snap_timer, 1);
}

static void curr_select_and_exit_abs(int abs_idx)
{
    if (abs_idx < 0 || abs_idx >= Machine_para.currency_count) return;
    if (g_curr_pending_abs_idx >= 0) return;
    if (curr_code_eq3(Machine_para.curr_code, Machine_para.currencies[abs_idx])) {
        ui_manager_switch(UI_PAGE_MAIN);
        return;
    }

    g_curr_pending_abs_idx = abs_idx;
    memcpy(g_curr_pending_code, Machine_para.currencies[abs_idx], 4);
    send_command(fd4, 0x03, (const uint8_t*)Machine_para.currencies[abs_idx], 3);
}

bool page_07_curr_set_pending_result(uint8_t status)
{
    if (g_curr_pending_abs_idx < 0) return false;

    if (status == 0x01) {
        memcpy(Machine_para.curr_code, g_curr_pending_code, 4);
        set_curr(get_curr_item(g_curr_pending_code));
        sim_clear_all_sn(&sim);
        memset(sim.denom, 0, sizeof(sim.denom)); //切币种时清空旧币种面额缓存，避免三角提示残留
        sim.denom_number = 0;
        sim.total_pcs = 0;
        sim.total_amount = 0.0f;
        g_curr_sel_abs_idx = g_curr_pending_abs_idx;
        g_curr_sel_vis_idx = curr_find_visible_pos_by_abs(g_curr_sel_abs_idx);
        g_curr_pending_abs_idx = -1;
        memset(g_curr_pending_code, 0, sizeof(g_curr_pending_code));
        ui_state_save_to_file();
        ui_manager_switch(UI_PAGE_MAIN);
        page_01_scroll_hint_force_hide();
        if (page_01_main_scroll_container && lv_obj_is_valid(page_01_main_scroll_container)) {
            // 切换币种成功后再次归零，确保不会出现首行被遮挡
            lv_obj_scroll_to_y(page_01_main_scroll_container, 0, LV_ANIM_OFF);
        }
        return true;
    }

    if (status == 0x02) {
        g_curr_pending_abs_idx = -1;
        memset(g_curr_pending_code, 0, sizeof(g_curr_pending_code));
        g_curr_sel_abs_idx = curr_find_abs_idx_by_code(Machine_para.curr_code);
        g_curr_sel_vis_idx = curr_find_visible_pos_by_abs(g_curr_sel_abs_idx);
        curr_set_left_info_by_abs(g_curr_sel_abs_idx);
        if (g_curr_view_mode == CURR_VIEW_MODE_CARD) {
            curr_apply_selected_style();
            curr_scroll_to_visible_idx(g_curr_sel_vis_idx, true, true);
        } else {
            curr_refresh_right_views();
        }
        show_currency_set_fail_popup();
        return true;
    }

    return false;
}

static void curr_update_card_fav_ui(int i)
{
    bool sel = (g_curr_cards[i].abs_idx == g_curr_sel_abs_idx);
    bool fav = curr_is_favorite_abs_idx(g_curr_cards[i].abs_idx);

    if (g_curr_cards[i].fav_btn == NULL || g_curr_cards[i].fav_icon == NULL) return;

    if (!sel || g_curr_view_mode != CURR_VIEW_MODE_CARD) {
        lv_obj_add_flag(g_curr_cards[i].fav_btn, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(g_curr_cards[i].fav_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_color(g_curr_cards[i].fav_btn, fav ? lv_color_hex(0xBFDFFF) : lv_color_hex(0xE5E5E6), 0);
    lv_obj_set_style_bg_opa(g_curr_cards[i].fav_btn, LV_OPA_COVER, 0);
    lv_img_set_src(g_curr_cards[i].fav_icon, fav ? "L:/usr/local/share/lvgl_data/fav.png" : "L:/usr/local/share/lvgl_data/unfav.png");
}

static void curr_update_grid_fav_ui(int i)
{
    bool fav = curr_is_favorite_abs_idx(g_curr_grid_items[i].abs_idx);

    if (g_curr_grid_items[i].fav_btn == NULL || g_curr_grid_items[i].fav_icon == NULL) return;

    lv_obj_set_style_bg_opa(g_curr_grid_items[i].fav_btn, LV_OPA_TRANSP, 0);
    lv_img_set_src(g_curr_grid_items[i].fav_icon, fav ? "L:/usr/local/share/lvgl_data/fav.png" : "L:/usr/local/share/lvgl_data/unfav.png");
}

static void curr_fav_press_feedback_cb(lv_event_t* e)
{
    lv_obj_t* btn = lv_event_get_target(e);
    if (btn == NULL) return;

    lv_obj_t* icon = lv_obj_get_child(btn, 0);
    if (icon == NULL) return;

    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        lv_obj_set_style_opa(btn, 220, 0);
        lv_img_set_zoom(icon, 235);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_set_style_opa(btn, LV_OPA_COVER, 0);
        lv_img_set_zoom(icon, 256);
    }
}

static void curr_apply_selected_style(void)
{
    if (g_curr_card_layer == NULL || g_curr_visible_cnt <= 0) return;

    for (int i = 0; i < g_curr_visible_cnt; i++) {
        bool sel = (i == g_curr_sel_vis_idx);
        int pos_x = g_curr_cards[i].base_x;
        int pos_y = g_curr_cards[i].base_y;

        if (i > g_curr_sel_vis_idx) pos_x += CURR_SEL_NEXT_EXTRA_GAP;

        lv_obj_set_style_border_color(g_curr_cards[i].card, lv_color_hex(0xDDE3EA), 0);
        lv_obj_set_style_radius(g_curr_cards[i].card, 30, 0);

        if (sel) {
            if (g_curr_cards[i].selected_bg) {
                lv_obj_set_pos(g_curr_cards[i].selected_bg,
                               pos_x - (CURR_CARD_SEL_W - CURR_CARD_W) / 2 - CURR_CARD_SELECTED_BG_OFS_X,
                               pos_y - (CURR_CARD_SEL_H - CURR_CARD_H) / 2 - CURR_CARD_SELECTED_BG_OFS_TOP);
                lv_obj_clear_flag(g_curr_cards[i].selected_bg, LV_OBJ_FLAG_HIDDEN);
            }

            lv_obj_set_size(g_curr_cards[i].card, CURR_CARD_SEL_W, CURR_CARD_SEL_H);
            lv_obj_set_pos(g_curr_cards[i].card,
                           pos_x - (CURR_CARD_SEL_W - CURR_CARD_W) / 2,
                           pos_y - (CURR_CARD_SEL_H - CURR_CARD_H) / 2);
            lv_obj_set_style_bg_opa(g_curr_cards[i].card, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(g_curr_cards[i].card, 0, 0);
            lv_obj_set_style_shadow_width(g_curr_cards[i].card, 0, 0);
            lv_obj_set_style_shadow_opa(g_curr_cards[i].card, LV_OPA_0, 0);
            lv_obj_set_style_text_color(g_curr_cards[i].name, lv_color_hex(CURR_TEXT_SEL), 0);
            lv_obj_set_style_text_color(g_curr_cards[i].no, lv_color_hex(0x202020), 0);
            curr_set_image_selected_style(g_curr_cards[i].img);
        } else {
            if (g_curr_cards[i].selected_bg) {
                lv_obj_add_flag(g_curr_cards[i].selected_bg, LV_OBJ_FLAG_HIDDEN);
            }

            lv_obj_set_size(g_curr_cards[i].card, CURR_CARD_W, CURR_CARD_H);
            lv_obj_set_pos(g_curr_cards[i].card, pos_x, pos_y);
            lv_obj_set_style_bg_opa(g_curr_cards[i].card, LV_OPA_COVER, 0);
            lv_obj_set_style_bg_color(g_curr_cards[i].card, lv_color_hex(CURR_CARD_BG_UNSEL), 0);
            lv_obj_set_style_border_width(g_curr_cards[i].card, 0, 0);
            lv_obj_set_style_shadow_width(g_curr_cards[i].card, 0, 0);
            lv_obj_set_style_shadow_opa(g_curr_cards[i].card, LV_OPA_0, 0);
            lv_obj_set_style_text_color(g_curr_cards[i].name, lv_color_hex(0x7E7E7E), 0);
            lv_obj_set_style_text_color(g_curr_cards[i].no, lv_color_hex(CURR_TEXT_UNSEL), 0);
            curr_set_image_unselected_style(g_curr_cards[i].img);
        }

        curr_update_card_fav_ui(i);
    }
}

static void curr_right_drag_cb(lv_event_t* e)
{
    if (g_curr_view_mode != CURR_VIEW_MODE_CARD) return;

    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) {
        lv_indev_t* indev = lv_indev_get_act();
        if (indev == NULL) return;

        lv_indev_get_point(indev, &g_curr_touch_start_pt);
        g_curr_touch_last_pt = g_curr_touch_start_pt;
        g_curr_touch_last_dx = 0;
        g_curr_touch_active = true;
        g_curr_touch_dragging = false;
        g_curr_touch_start_scroll = curr_scroll_x_abs();
        g_curr_touch_start_vis_idx = curr_pick_nearest_visible_idx();
        curr_reset_overscroll_visual();

        if (g_curr_snap_timer) {
            lv_timer_del(g_curr_snap_timer);
            g_curr_snap_timer = NULL;
        }
        g_curr_snap_target_vis_idx = -1;
        return;
    }

    if (code == LV_EVENT_PRESSING) {
        if (!g_curr_touch_active) return;

        lv_indev_t* indev = lv_indev_get_act();
        if (indev == NULL) return;

        lv_point_t p;
        lv_indev_get_point(indev, &p);

        int dx = p.x - g_curr_touch_start_pt.x;
        int dy = p.y - g_curr_touch_start_pt.y;
        g_curr_touch_last_dx = p.x - g_curr_touch_last_pt.x;
        g_curr_touch_last_pt = p;

        if (!g_curr_touch_dragging) {
            if (curr_abs_i32(dx) > CURR_DRAG_THRESHOLD && curr_abs_i32(dx) >= curr_abs_i32(dy)) {
                g_curr_touch_dragging = true;
            }
        }

        if (g_curr_touch_dragging) {
            int desired_scroll = g_curr_touch_start_scroll - dx;
            int max_scroll = curr_get_max_scroll();

            if (desired_scroll < 0) {
                curr_scroll_to_raw(0, false, false);
                curr_apply_overscroll_visual((-desired_scroll) / 3);
            } else if (desired_scroll > max_scroll) {
                curr_scroll_to_raw(max_scroll, false, false);
                curr_apply_overscroll_visual(-(desired_scroll - max_scroll) / 3);
            } else {
                curr_scroll_to_raw(desired_scroll, false, false);
            }
        }
        return;
    }

    if (code == LV_EVENT_RELEASED) {
        if (!g_curr_touch_active) return;
        g_curr_touch_active = false;

        if (g_curr_touch_dragging) {
            g_curr_touch_dragging = false;

            if (lv_obj_get_x(g_curr_list) != 0) {
                curr_animate_overscroll_back();
                curr_start_snap_timer(CURR_OVERSCROLL_SNAP_MS);
                g_curr_last_drag_tick = lv_tick_get();
                return;
            }

            int release_target_scroll;

            release_target_scroll = curr_calc_release_scroll_target();
            curr_scroll_to_raw(release_target_scroll, true, false);
            g_curr_snap_target_vis_idx = -1;
            curr_start_snap_timer(CURR_SNAP_IDLE_MS);
            g_curr_last_drag_tick = lv_tick_get();
            return;
        }

        curr_animate_overscroll_back();

        int vis_idx = curr_pick_nearest_visible_idx();
        g_curr_sel_vis_idx = vis_idx;
        g_curr_sel_abs_idx = g_curr_visible_idx[vis_idx];
        curr_apply_selected_style();
        curr_scroll_to_visible_idx(g_curr_sel_vis_idx, true, true);
    }
}

static void curr_card_click_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (g_curr_view_mode != CURR_VIEW_MODE_CARD) return;

    if (lv_tick_elaps(g_curr_last_drag_tick) < 220) {
        return;
    }

    int vis_idx = (int)(intptr_t)lv_event_get_user_data(e);
    if (vis_idx < 0 || vis_idx >= g_curr_visible_cnt) return;
    curr_select_and_exit_abs(g_curr_visible_idx[vis_idx]);
}

static void curr_fav_icon_click_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (g_curr_view_mode != CURR_VIEW_MODE_CARD) return;

    int vis_idx = (int)(intptr_t)lv_event_get_user_data(e);
    if (vis_idx < 0 || vis_idx >= g_curr_visible_cnt) return;

    int abs_idx = g_curr_visible_idx[vis_idx];
    curr_toggle_favorite_abs_idx(abs_idx);

    if (g_curr_fav_only && !curr_is_favorite_abs_idx(abs_idx)) {
        curr_refresh_right_views();
        return;
    }

    curr_apply_selected_style();
}

static void curr_grid_fav_click_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (g_curr_view_mode != CURR_VIEW_MODE_GRID) return;

    int vis_idx = (int)(intptr_t)lv_event_get_user_data(e);
    if (vis_idx < 0 || vis_idx >= g_curr_visible_cnt) return;

    int abs_idx = g_curr_visible_idx[vis_idx];
    curr_toggle_favorite_abs_idx(abs_idx);

    if (g_curr_fav_only && !curr_is_favorite_abs_idx(abs_idx)) {
        curr_refresh_right_views();
        return;
    }

    curr_update_grid_fav_ui(vis_idx);
}

static void curr_grid_item_click_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    if (g_curr_view_mode != CURR_VIEW_MODE_GRID) return;

    int vis_idx = (int)(intptr_t)lv_event_get_user_data(e);
    if (vis_idx < 0 || vis_idx >= g_curr_visible_cnt) return;

    int abs_idx = g_curr_visible_idx[vis_idx];
    g_curr_sel_abs_idx = abs_idx;
    g_curr_sel_vis_idx = vis_idx;
    curr_set_left_info_by_abs(abs_idx);
    curr_select_and_exit_abs(abs_idx);
}

static void curr_build_card_layer(void)
{
    g_curr_card_layer = lv_obj_create(g_curr_right_area);
    lv_obj_remove_style_all(g_curr_card_layer);
    lv_obj_set_size(g_curr_card_layer, CURR_VIEW_W, CURR_VIEW_H);
    lv_obj_set_pos(g_curr_card_layer, 0, 0);
    lv_obj_set_style_bg_opa(g_curr_card_layer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(g_curr_card_layer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(g_curr_card_layer, LV_SCROLLBAR_MODE_OFF);

    g_curr_list = lv_obj_create(g_curr_card_layer);
    lv_obj_remove_style_all(g_curr_list);
    lv_obj_set_size(g_curr_list, CURR_VIEW_W, CURR_TRACK_Y);
    lv_obj_set_pos(g_curr_list, 0, 0);
    lv_obj_set_style_bg_opa(g_curr_list, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(g_curr_list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_curr_list, LV_OBJ_FLAG_SCROLLABLE);

    for (int i = 0; i < g_curr_visible_cnt; i++) {
        int abs_idx = g_curr_visible_idx[i];
        int x = CURR_CARD_FIRST_X + i * CURR_CARD_STRIDE;

        g_curr_cards[i].abs_idx = abs_idx;
        g_curr_cards[i].base_x = x;
        g_curr_cards[i].base_y = CURR_CARD_Y;

        g_curr_cards[i].selected_bg = lv_img_create(g_curr_list);
        lv_img_set_src(g_curr_cards[i].selected_bg, CURR_CARD_SELECTED_BG_PATH);
        lv_obj_add_flag(g_curr_cards[i].selected_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_curr_cards[i].selected_bg, LV_OBJ_FLAG_CLICKABLE);

        g_curr_cards[i].card = lv_obj_create(g_curr_list);
        lv_obj_set_size(g_curr_cards[i].card, CURR_CARD_W, CURR_CARD_H);
        lv_obj_set_pos(g_curr_cards[i].card, x, CURR_CARD_Y);
        lv_obj_set_style_radius(g_curr_cards[i].card, 30, 0);
        lv_obj_set_style_bg_opa(g_curr_cards[i].card, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(g_curr_cards[i].card, lv_color_hex(CURR_CARD_BG_UNSEL), 0);
        lv_obj_set_style_border_width(g_curr_cards[i].card, 0, 0);
        lv_obj_set_style_shadow_width(g_curr_cards[i].card, 0, 0);
        lv_obj_set_style_shadow_opa(g_curr_cards[i].card, LV_OPA_0, 0);
        lv_obj_set_scrollbar_mode(g_curr_cards[i].card, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(g_curr_cards[i].card, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(g_curr_cards[i].card, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_add_event_cb(g_curr_cards[i].card, curr_card_click_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        lv_obj_add_event_cb(g_curr_cards[i].card, curr_right_drag_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(g_curr_cards[i].card, curr_right_drag_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_event_cb(g_curr_cards[i].card, curr_right_drag_cb, LV_EVENT_RELEASED, NULL);

        g_curr_cards[i].img = lv_img_create(g_curr_cards[i].card);
        lv_img_set_src(g_curr_cards[i].img, get_currency_img(Machine_para.currencies[abs_idx]));
        curr_set_img_target_width(g_curr_cards[i].img, Machine_para.currencies[abs_idx], CURR_FLAG_TARGET_W);
        lv_obj_set_pos(g_curr_cards[i].img, -35, CURR_FLAG_Y_IN_CARD);

        g_curr_cards[i].name = lv_label_create(g_curr_cards[i].card);
        lv_label_set_text_fmt(g_curr_cards[i].name, "%s", Machine_para.currencies[abs_idx]);
        lv_obj_set_pos(g_curr_cards[i].name, 21, 174);
        lv_obj_set_style_text_font(g_curr_cards[i].name, &lv_font_montserrat_30, 0);

        g_curr_cards[i].no = lv_label_create(g_curr_cards[i].card);
        lv_label_set_text_fmt(g_curr_cards[i].no, "NO.%02d", abs_idx + 1);
        lv_obj_set_pos(g_curr_cards[i].no, 21, 224);
        lv_obj_set_style_text_font(g_curr_cards[i].no, &lv_font_montserrat_14, 0);

        g_curr_cards[i].fav_btn = lv_obj_create(g_curr_cards[i].card);
        lv_obj_set_size(g_curr_cards[i].fav_btn, CURR_FAV_BTN_IN_CARD_W, CURR_FAV_BTN_IN_CARD_H);
        lv_obj_set_pos(g_curr_cards[i].fav_btn, CURR_FAV_BTN_IN_CARD_X, CURR_FAV_BTN_IN_CARD_Y);
        lv_obj_set_style_radius(g_curr_cards[i].fav_btn, 14, 0);
        lv_obj_set_style_border_width(g_curr_cards[i].fav_btn, 0, 0);
        lv_obj_set_scrollbar_mode(g_curr_cards[i].fav_btn, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(g_curr_cards[i].fav_btn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(g_curr_cards[i].fav_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(g_curr_cards[i].fav_btn, curr_fav_icon_click_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        lv_obj_add_event_cb(g_curr_cards[i].fav_btn, curr_fav_press_feedback_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(g_curr_cards[i].fav_btn, curr_fav_press_feedback_cb, LV_EVENT_RELEASED, NULL);
        lv_obj_add_event_cb(g_curr_cards[i].fav_btn, curr_fav_press_feedback_cb, LV_EVENT_PRESS_LOST, NULL);

        g_curr_cards[i].fav_icon = lv_img_create(g_curr_cards[i].fav_btn);
        lv_img_set_src(g_curr_cards[i].fav_icon, "L:/usr/local/share/lvgl_data/unfav.png");
        lv_obj_center(g_curr_cards[i].fav_icon);
    }

    lv_obj_t* tail = lv_obj_create(g_curr_list);
    lv_obj_remove_style_all(tail);
    lv_obj_set_size(tail, 1, 1);
    lv_obj_set_pos(tail, CURR_CARD_FIRST_X + g_curr_visible_cnt * CURR_CARD_STRIDE + CURR_CARD_PAD_RIGHT, 1);

    g_curr_track = lv_obj_create(g_curr_card_layer);
    lv_obj_remove_style_all(g_curr_track);
    lv_obj_set_size(g_curr_track, CURR_VIEW_W, CURR_TRACK_H);
    lv_obj_set_pos(g_curr_track, 0, CURR_TRACK_Y);
    lv_obj_set_style_bg_color(g_curr_track, lv_color_hex(CURR_TRACK_BG), 0);
    lv_obj_set_style_bg_opa(g_curr_track, LV_OPA_TRANSP, 0);

    g_curr_thumb = lv_obj_create(g_curr_card_layer);
    lv_obj_remove_style_all(g_curr_thumb);
    lv_obj_set_style_bg_color(g_curr_thumb, lv_color_hex(CURR_TRACK_FG), 0);
    lv_obj_set_style_bg_opa(g_curr_thumb, LV_OPA_90, 0);
    lv_obj_set_style_radius(g_curr_thumb, 3, 0);

}

static void curr_build_grid_layer(void)
{
    g_curr_grid_layer = lv_obj_create(g_curr_right_area);
    lv_obj_remove_style_all(g_curr_grid_layer);
    lv_obj_set_size(g_curr_grid_layer, CURR_VIEW_W, CURR_VIEW_H);
    lv_obj_set_pos(g_curr_grid_layer, 0, 0);
    lv_obj_set_style_bg_opa(g_curr_grid_layer, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(g_curr_grid_layer, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_curr_grid_layer, LV_OBJ_FLAG_SCROLLABLE);

    g_curr_grid_scroll = lv_obj_create(g_curr_grid_layer);
    lv_obj_remove_style_all(g_curr_grid_scroll);
    lv_obj_set_size(g_curr_grid_scroll, CURR_VIEW_W, CURR_VIEW_H);
    lv_obj_set_pos(g_curr_grid_scroll, 0, 0);
    lv_obj_set_style_bg_opa(g_curr_grid_scroll, LV_OPA_TRANSP, 0);
    lv_obj_set_scroll_dir(g_curr_grid_scroll, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(g_curr_grid_scroll, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(g_curr_grid_scroll, LV_OBJ_FLAG_SCROLLABLE);

    int rows = (g_curr_visible_cnt + CURR_GRID_COLS - 1) / CURR_GRID_COLS;
    int content_h = CURR_GRID_START_Y + rows * CURR_GRID_ROW_STEP + 10;
    if (content_h < CURR_VIEW_H) content_h = CURR_VIEW_H;

    lv_obj_t* content = lv_obj_create(g_curr_grid_scroll);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, CURR_VIEW_W, content_h);
    lv_obj_set_pos(content, 0, 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);

    for (int i = 0; i < g_curr_visible_cnt; i++) {
        int abs_idx = g_curr_visible_idx[i];
        int row = i / CURR_GRID_COLS;
        int col = i % CURR_GRID_COLS;
        int x = CURR_GRID_START_X + col * CURR_GRID_CELL_W;
        int y = CURR_GRID_START_Y + row * CURR_GRID_ROW_STEP;

        g_curr_grid_items[i].abs_idx = abs_idx;

        g_curr_grid_items[i].item = lv_obj_create(content);
        lv_obj_remove_style_all(g_curr_grid_items[i].item);
        lv_obj_set_size(g_curr_grid_items[i].item, CURR_GRID_ITEM_W, CURR_GRID_CELL_H);
        lv_obj_set_pos(g_curr_grid_items[i].item, x, y);
        lv_obj_set_style_bg_opa(g_curr_grid_items[i].item, LV_OPA_TRANSP, 0);
        lv_obj_clear_flag(g_curr_grid_items[i].item, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(g_curr_grid_items[i].item, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_flag(g_curr_grid_items[i].item, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(g_curr_grid_items[i].item, curr_grid_item_click_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);

        g_curr_grid_items[i].img = lv_img_create(g_curr_grid_items[i].item);
        lv_img_set_src(g_curr_grid_items[i].img, get_currency_img(Machine_para.currencies[abs_idx]));
        curr_set_img_target_width(g_curr_grid_items[i].img, Machine_para.currencies[abs_idx], CURR_FLAG_TARGET_W);
        lv_obj_align(g_curr_grid_items[i].img, LV_ALIGN_TOP_MID, CURR_GRID_GROUP_OFS_X, CURR_GRID_FLAG_Y);

        g_curr_grid_items[i].selected_mark = lv_img_create(g_curr_grid_items[i].item);
        lv_img_set_src(g_curr_grid_items[i].selected_mark, CURR_GRID_SELECTED_MARK_PATH);
        lv_obj_set_size(g_curr_grid_items[i].selected_mark, 24, 24);
        lv_obj_align_to(g_curr_grid_items[i].selected_mark,
                        g_curr_grid_items[i].img,
                        LV_ALIGN_CENTER, 0, 0);
        if (abs_idx != g_curr_sel_abs_idx) {
            lv_obj_add_flag(g_curr_grid_items[i].selected_mark, LV_OBJ_FLAG_HIDDEN);
        }

        g_curr_grid_items[i].fav_btn = lv_obj_create(g_curr_grid_items[i].item);
        lv_obj_set_size(g_curr_grid_items[i].fav_btn, CURR_FAV_BTN_IN_CARD_W - 2, CURR_FAV_BTN_IN_CARD_H - 2);
        lv_obj_set_pos(g_curr_grid_items[i].fav_btn, CURR_GRID_FAV_X, CURR_GRID_FAV_Y);
        lv_obj_set_style_radius(g_curr_grid_items[i].fav_btn, 0, 0);
        lv_obj_set_style_bg_opa(g_curr_grid_items[i].fav_btn, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(g_curr_grid_items[i].fav_btn, 0, 0);
        lv_obj_set_style_shadow_width(g_curr_grid_items[i].fav_btn, 0, 0);
        lv_obj_set_style_shadow_opa(g_curr_grid_items[i].fav_btn, LV_OPA_0, 0);
        lv_obj_set_scrollbar_mode(g_curr_grid_items[i].fav_btn, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(g_curr_grid_items[i].fav_btn, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_add_flag(g_curr_grid_items[i].fav_btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(g_curr_grid_items[i].fav_btn, curr_grid_fav_click_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
        lv_obj_add_event_cb(g_curr_grid_items[i].fav_btn, curr_fav_press_feedback_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(g_curr_grid_items[i].fav_btn, curr_fav_press_feedback_cb, LV_EVENT_RELEASED, NULL);
        lv_obj_add_event_cb(g_curr_grid_items[i].fav_btn, curr_fav_press_feedback_cb, LV_EVENT_PRESS_LOST, NULL);

        g_curr_grid_items[i].fav_icon = lv_img_create(g_curr_grid_items[i].fav_btn);
        lv_img_set_src(g_curr_grid_items[i].fav_icon, "L:/usr/local/share/lvgl_data/unfav.png");
        lv_obj_center(g_curr_grid_items[i].fav_icon);

        g_curr_grid_items[i].name = lv_label_create(g_curr_grid_items[i].item);
        lv_label_set_text_fmt(g_curr_grid_items[i].name, "%s", Machine_para.currencies[abs_idx]);
        lv_obj_set_width(g_curr_grid_items[i].name, CURR_GRID_ITEM_W);
        lv_obj_set_style_text_align(g_curr_grid_items[i].name, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(g_curr_grid_items[i].name, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(g_curr_grid_items[i].name,
                                    (abs_idx == g_curr_sel_abs_idx) ? lv_color_hex(CURR_TEXT_SEL) : lv_color_hex(0x7E7E7E), 0);
        lv_obj_align(g_curr_grid_items[i].name, LV_ALIGN_BOTTOM_MID, CURR_GRID_GROUP_OFS_X, -CURR_GRID_TEXT_BOTTOM);

        if (abs_idx == g_curr_sel_abs_idx) {
            curr_set_image_selected_style(g_curr_grid_items[i].img);
        } else {
            curr_set_image_unselected_style(g_curr_grid_items[i].img);
        }
        curr_update_grid_fav_ui(i);
    }

    g_curr_empty_label = NULL;
}

static void curr_set_mode_visible(void)
{
    if (g_curr_view_mode == CURR_VIEW_MODE_CARD) {
        if (g_curr_card_layer) {
            lv_obj_clear_flag(g_curr_card_layer, LV_OBJ_FLAG_HIDDEN);
            curr_apply_selected_style();
            curr_update_track_by_scroll(curr_scroll_x_abs());
        }
    } else {
        if (g_curr_grid_layer) {
            lv_obj_clear_flag(g_curr_grid_layer, LV_OBJ_FLAG_HIDDEN);
            for (int i = 0; i < g_curr_visible_cnt; i++) {
                curr_update_grid_fav_ui(i);
            }
        }
    }
}

static void curr_refresh_right_views(void)
{
    if (g_curr_right_area == NULL) return;

    if (g_curr_empty_label && lv_obj_is_valid(g_curr_empty_label)) {
        lv_obj_del(g_curr_empty_label);
        g_curr_empty_label = NULL;
    }

    curr_update_visible_idx();

    g_curr_sel_abs_idx = curr_find_abs_idx_by_code(Machine_para.curr_code);
    g_curr_sel_vis_idx = curr_find_visible_pos_by_abs(g_curr_sel_abs_idx);

    if (g_curr_card_layer && lv_obj_is_valid(g_curr_card_layer)) {
        lv_obj_del(g_curr_card_layer);
        g_curr_card_layer = NULL;
        g_curr_list = NULL;
        g_curr_track = NULL;
        g_curr_thumb = NULL;
    }

    if (g_curr_grid_layer && lv_obj_is_valid(g_curr_grid_layer)) {
        lv_obj_del(g_curr_grid_layer);
        g_curr_grid_layer = NULL;
        g_curr_grid_scroll = NULL;
        g_curr_empty_label = NULL;
    }

    memset(g_curr_cards, 0, sizeof(g_curr_cards));
    memset(g_curr_grid_items, 0, sizeof(g_curr_grid_items));

    if (g_curr_visible_cnt <= 0) {
        g_curr_empty_label = lv_label_create(g_curr_right_area);
        lv_label_set_text(g_curr_empty_label, g_curr_fav_only ? "NO FAVORITE CURRENCY" : "NO CURRENCY");
        lv_obj_set_style_text_color(g_curr_empty_label, lv_color_hex(0xB3B3B3), 0);
        lv_obj_set_style_text_font(g_curr_empty_label, &lv_font_montserrat_20, 0);
        lv_obj_center(g_curr_empty_label);
        return;
    }

    if (g_curr_view_mode == CURR_VIEW_MODE_CARD) {
        curr_build_card_layer();
    } else {
        curr_build_grid_layer();
    }
    curr_set_mode_visible();

    if (g_curr_view_mode == CURR_VIEW_MODE_CARD) {
        curr_scroll_to_visible_idx(g_curr_sel_vis_idx, false, true);
        curr_apply_selected_style();
        curr_update_track_by_scroll(curr_scroll_x_abs());
    }
}

static void curr_view_btn_click_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    g_curr_view_mode = (g_curr_view_mode == CURR_VIEW_MODE_CARD) ? CURR_VIEW_MODE_GRID : CURR_VIEW_MODE_CARD;
    curr_refresh_right_views();
    curr_refresh_left_buttons();
    ui_state_save_to_file();

}

static void curr_fav_btn_click_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

    g_curr_fav_only = !g_curr_fav_only;
    ui_state_save_to_file();
    curr_refresh_left_buttons();
    curr_refresh_right_views();
}

void page_07_curr_img_reset(void)
{
    if (g_curr_snap_timer) {
        lv_timer_del(g_curr_snap_timer);
        g_curr_snap_timer = NULL;
    }

    if (g_curr_root && lv_obj_is_valid(g_curr_root)) {
        lv_obj_del(g_curr_root);
    }

    g_curr_root = NULL;
    g_curr_left_panel = NULL;
    g_curr_left_img = NULL;
    g_curr_left_code = NULL;
    g_curr_left_code_decor = NULL;
    g_curr_left_no = NULL;

    g_curr_btn_view = NULL;
    g_curr_btn_view_label = NULL;
    g_curr_btn_fav = NULL;
    g_curr_btn_fav_label = NULL;
    g_curr_btn_back = NULL;
    g_curr_btn_back_label = NULL;

    g_curr_right_area = NULL;
    g_curr_card_layer = NULL;
    g_curr_grid_layer = NULL;

    g_curr_list = NULL;
    g_curr_track = NULL;
    g_curr_thumb = NULL;

    g_curr_grid_scroll = NULL;
    g_curr_empty_label = NULL;

    g_curr_touch_active = false;
    g_curr_touch_dragging = false;
    g_curr_touch_start_scroll = 0;
    g_curr_touch_last_dx = 0;
    g_curr_last_drag_tick = 0;

    memset(g_curr_cards, 0, sizeof(g_curr_cards));
    memset(g_curr_grid_items, 0, sizeof(g_curr_grid_items));
    memset(g_curr_visible_idx, 0, sizeof(g_curr_visible_idx));
    g_curr_visible_cnt = 0;
    g_curr_pending_abs_idx = -1;
    memset(g_curr_pending_code, 0, sizeof(g_curr_pending_code));
}

void page_07_curr_img_refre(void)
{
    if (curr_page == NULL || Machine_para.currency_count <= 0) return;

    if (!g_ui_state_loaded) {
        ui_state_load_from_file();
    }

    page07_state_apply_to_runtime();

    page_07_curr_img_reset();

    g_curr_sel_vis_idx = curr_find_visible_pos_by_abs(g_curr_sel_abs_idx);

    g_curr_root = lv_obj_create(curr_page);
    lv_obj_remove_style_all(g_curr_root);
    lv_obj_set_size(g_curr_root, 1280, 400);
    lv_obj_set_pos(g_curr_root, 0, 0);
    lv_obj_clear_flag(g_curr_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(g_curr_root, LV_SCROLLBAR_MODE_OFF);

    g_curr_left_panel = lv_obj_create(g_curr_root);
    lv_obj_remove_style_all(g_curr_left_panel);
    lv_obj_set_size(g_curr_left_panel, CURR_SEL_W, CURR_SEL_H);
    lv_obj_set_pos(g_curr_left_panel, 0, 0);
    lv_obj_set_style_bg_color(g_curr_left_panel, lv_color_hex(CURR_LEFT_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(g_curr_left_panel, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(g_curr_left_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(g_curr_left_panel, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* left_title = lv_label_create(g_curr_left_panel);
    lv_label_set_text(left_title, "CURRENCY");
    lv_obj_set_pos(left_title, 105, 14);
    lv_obj_set_style_text_font(left_title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(left_title, lv_color_hex(0x707070), 0);

    g_curr_left_img = lv_img_create(g_curr_left_panel);
    lv_img_set_zoom(g_curr_left_img, 170);
    lv_obj_align(g_curr_left_img, LV_ALIGN_TOP_MID, CURR_LEFT_IMG_ALIGN_X, CURR_LEFT_IMG_ALIGN_Y);

    g_curr_left_code_decor = lv_label_create(g_curr_left_panel);
    lv_obj_set_pos(g_curr_left_code_decor, CURR_LEFT_CODE_DECOR_X, CURR_LEFT_CODE_DECOR_Y);
    lv_obj_set_style_text_font(g_curr_left_code_decor, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(g_curr_left_code_decor, lv_color_hex(0xEBEBEB), 0);

    g_curr_left_code = lv_label_create(g_curr_left_panel);
    lv_obj_set_pos(g_curr_left_code, CURR_LEFT_CODE_X, CURR_LEFT_CODE_Y);
    lv_obj_set_style_text_font(g_curr_left_code, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(g_curr_left_code, lv_color_hex(0x202020), 0);

    g_curr_left_no = lv_label_create(g_curr_left_panel);
    lv_obj_set_pos(g_curr_left_no, CURR_LEFT_NO_X, CURR_LEFT_NO_Y);
    lv_obj_set_style_text_font(g_curr_left_no, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_curr_left_no, lv_color_hex(0x202020), 0);

    g_curr_btn_view = lv_btn_create(g_curr_left_panel);
    lv_obj_set_size(g_curr_btn_view, CURR_BTN_W, CURR_BTN_H);
    lv_obj_set_pos(g_curr_btn_view, CURR_VIEW_BTN_X, CURR_BTN_Y);
    lv_obj_add_event_cb(g_curr_btn_view, curr_view_btn_click_cb, LV_EVENT_CLICKED, NULL);
    g_curr_btn_view_label = lv_label_create(g_curr_btn_view);
    lv_label_set_text(g_curr_btn_view_label, "CARD");
    lv_obj_center(g_curr_btn_view_label);

    g_curr_btn_fav = lv_btn_create(g_curr_left_panel);
    lv_obj_set_size(g_curr_btn_fav, CURR_BTN_W, CURR_BTN_H);
    lv_obj_set_pos(g_curr_btn_fav, CURR_FAV_BTN_X, CURR_BTN_Y);
    lv_obj_add_event_cb(g_curr_btn_fav, curr_fav_btn_click_cb, LV_EVENT_CLICKED, NULL);
    g_curr_btn_fav_label = lv_label_create(g_curr_btn_fav);
    lv_label_set_text(g_curr_btn_fav_label, "FAV");
    lv_obj_center(g_curr_btn_fav_label);

    g_curr_btn_back = lv_btn_create(g_curr_left_panel);
    lv_obj_set_size(g_curr_btn_back, CURR_BTN_W, CURR_BTN_H);
    lv_obj_set_pos(g_curr_btn_back, CURR_BACK_BTN_X, CURR_BTN_Y);
    lv_obj_add_event_cb(g_curr_btn_back, curr_back_btn_click_cb, LV_EVENT_CLICKED, NULL);
    g_curr_btn_back_label = lv_label_create(g_curr_btn_back);
    lv_label_set_text(g_curr_btn_back_label, "BACK");
    lv_obj_center(g_curr_btn_back_label);

    g_curr_right_area = lv_obj_create(g_curr_root);
    lv_obj_remove_style_all(g_curr_right_area);
    lv_obj_set_size(g_curr_right_area, CURR_VIEW_W, CURR_VIEW_H);
    lv_obj_set_pos(g_curr_right_area, CURR_VIEW_X, CURR_VIEW_Y);
    lv_obj_set_style_bg_color(g_curr_right_area, lv_color_hex(CURR_RIGHT_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(g_curr_right_area, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(g_curr_right_area, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(g_curr_right_area, LV_SCROLLBAR_MODE_OFF);

    curr_set_left_info_by_abs(g_curr_sel_abs_idx);
    curr_refresh_left_buttons();
    curr_refresh_right_views();
}
