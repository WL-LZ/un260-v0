#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "lvgl/lvgl.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_core/lv_page_event.h"
#include "un260/lv_core/lv_page_declear.h"
#include "un260/lv_resources/lv_img_init.h"




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
    switch (Machine_para.mode)
    {

    case MODE_MDC:
        update_label_by_name(page_01_main_obj, page_01_main_len, "mix_label", "MDC");
            break;
    case MODE_CNT:
        update_label_by_name(page_01_main_obj, page_01_main_len, "mix_label", "CNT");
        break;
    case MODE_VER:
        update_label_by_name(page_01_main_obj, page_01_main_len, "mix_label", "VER");
        break;
    case MODE_SDC:
        update_label_by_name(page_01_main_obj, page_01_main_len, "mix_label", "SDC");
        break;
    default:
        update_label_by_name(page_01_main_obj, page_01_main_len, "mix_label", "NONE");

        break;
    }

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
    if (sim.denom_number > PAGE_01_REPORT_ITEM)
        lv_obj_add_flag(page_01_main_scroll_container, LV_OBJ_FLAG_SCROLLABLE);
    else
        lv_obj_clear_flag(page_01_main_scroll_container, LV_OBJ_FLAG_SCROLLABLE);
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
{   if(Machine_para.batch_num)
    update_label_by_name(page_03_menu_obj, page_03_menu_len, "03_batch_num_label", "%d", Machine_para.batch_num);
else
{
    return;
}

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
    lv_obj_t* a_denom;
    lv_obj_t* a_pcs;
    lv_obj_t* a_amount;
    char a_denom_buf[32], a_pcs_buf[32], a_amount_buf[32];
    for (int i = 0; i < PAGE_02_A_ITEM; i++)
    {
        int row;
        row = i + 1;
        snprintf(a_denom_buf, sizeof(a_denom_buf), "02_a_denom_%d", row);
        snprintf(a_pcs_buf, sizeof(a_pcs_buf), "02_a_pcs_%d", row);
        snprintf(a_amount_buf, sizeof(a_amount_buf), "02_a_amount_%d", row);
        a_denom = find_obj_by_name(a_denom_buf, page_02_list_obj, page_02_list_len);
        a_pcs = find_obj_by_name(a_pcs_buf, page_02_list_obj, page_02_list_len);
        a_amount = find_obj_by_name(a_amount_buf, page_02_list_obj, page_02_list_len);
        int temp_current;
        temp_current = i + (page_02_a_report_status.curent_page - 1) * PAGE_02_A_ITEM;

            update_label_by_name(page_02_list_obj, page_02_list_len, a_denom_buf, "%d", sim.denom[temp_current].value);
            update_label_by_name(page_02_list_obj, page_02_list_len, a_pcs_buf, "%d", sim.denom[temp_current].pcs);
            update_label_by_name(page_02_list_obj, page_02_list_len, a_amount_buf, "%.0f", sim.denom[temp_current].amount);
            update_label_by_name(page_02_list_obj, page_02_list_len, "02_a_pcs_amount", "%d", sim.total_pcs);
            update_label_by_name(page_02_list_obj, page_02_list_len, "02_a_amount_total", "%.0f", sim.total_amount);

            if (temp_current > sim.denom_number)
            {
            lv_obj_add_flag(a_denom, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(a_pcs, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(a_amount, LV_OBJ_FLAG_HIDDEN);

            }
            else
            {
            lv_obj_clear_flag(a_denom, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(a_pcs, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(a_amount, LV_OBJ_FLAG_HIDDEN);
            }

    }
}
//仿真模式 非实机函数

void page_02_b_page_refre(void)
{
    lv_obj_t* b_no;
    lv_obj_t* b_denom;
    lv_obj_t* b_sn;
    char b_no_buf[32], b_denom_buf[32], b_sn_buf[32];
    for (int i = 0; i < PAGE_02_B_ITEM; i++)
    {
        int row;
        row = i + 1;
        snprintf(b_no_buf, sizeof(b_no_buf), "02_b_no_%d", row);
        snprintf(b_denom_buf, sizeof(b_denom_buf), "02_b_denom_%d", row);
        snprintf(b_sn_buf, sizeof(b_sn_buf), "02_b_sn_%d", row);
        b_no = find_obj_by_name(b_no_buf, page_02_list_obj, page_02_list_len);
        b_denom = find_obj_by_name(b_denom_buf, page_02_list_obj, page_02_list_len);
        b_sn = find_obj_by_name(b_sn_buf, page_02_list_obj, page_02_list_len);
        int temp_current;
        temp_current = i + (page_02_b_report_status.curent_page - 1) * PAGE_02_B_ITEM;
        if (temp_current < sim.total_pcs)
        {
            update_label_by_name(page_02_list_obj, page_02_list_len, b_no_buf, "%d", (page_02_b_report_status.curent_page - 1) * PAGE_02_B_ITEM + i + 1);
            update_label_by_name(page_02_list_obj, page_02_list_len, b_denom_buf, "%d", sim.denom_mix[temp_current]);
            if (sim.sn_str)
            {
                update_label_by_name(page_02_list_obj, page_02_list_len, b_sn_buf, "%s", sim.sn_str[temp_current]);

            }
            lv_obj_clear_flag(b_no, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(b_denom, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(b_sn, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(b_no, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(b_denom, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(b_sn, LV_OBJ_FLAG_HIDDEN);
        }


    }
}
//仿真模式 非实机函数
void page_02_c_page_refre(void)
{
    lv_obj_t* c_no;
    lv_obj_t* c_denom;
    lv_obj_t* c_reject;
    char c_no_buf[32], c_denom_buf[32], c_reject_buf[32];
    //模拟报错
    sim.err_str = malloc(sizeof(char*) * sim.err_num);
    sim.err_str[0] = malloc(32);
    strcpy(sim.err_str[0], "IMG 163");
    for (int i = 0; i < PAGE_02_B_ITEM; i++)
    {
        int row;
        row = i + 1;
        snprintf(c_no_buf, sizeof(c_no_buf), "02_c_no_%d", row);
        snprintf(c_denom_buf, sizeof(c_denom_buf), "02_c_denom_%d", row);
        snprintf(c_reject_buf, sizeof(c_reject_buf), "02_c_reject_%d", row);
        c_no = find_obj_by_name(c_no_buf, page_02_list_obj, page_02_list_len);
        c_denom = find_obj_by_name(c_denom_buf, page_02_list_obj, page_02_list_len);
        c_reject = find_obj_by_name(c_reject_buf, page_02_list_obj, page_02_list_len);
        int temp_current;
        temp_current = i + (page_02_c_report_status.curent_page - 1) * PAGE_02_A_ITEM;
        if (temp_current < sim.err_num)
        {
            update_label_by_name(page_02_list_obj, page_02_list_len, c_no_buf, "%d", (page_02_c_report_status.curent_page - 1) * PAGE_02_A_ITEM + i + 1);
            update_label_by_name(page_02_list_obj, page_02_list_len, c_denom_buf, "%d", sim.denom[temp_current].value);
            update_label_by_name(page_02_list_obj, page_02_list_len, c_reject_buf, "%s", sim.err_str[temp_current]);
            lv_obj_clear_flag(c_no, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(c_denom, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(c_reject, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(c_no, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(c_denom, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(c_reject, LV_OBJ_FLAG_HIDDEN);

        }

    }
    printf("sim.err_num = %d\n", sim.err_num);
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
    
    update_label_by_name(page_01_main_obj, page_01_main_len, "add_label", "%s", Machine_para.add_enabel == true?"ADD:ON":"ADD:OFF");

}
void page_01_work_refre(void)
{
    char* work[2] = {"AUTO","MANUAL"};
    update_label_by_name(page_01_main_obj, page_01_main_len, "auto_label", "%s", work[Machine_para.work_mode]);

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


}
void page_01_face_refre(void)
{
    char* batch[4] = { "F./O. : OFF","F.","O." ,"F./O."};

    update_label_by_name(page_01_main_obj, page_01_main_len, "face_label", "%s", batch[Machine_para.fo_mode]);

}
void page_01_cfd_refre(void)
{
    char* cfd[3] = { "L","M","H"  };
    update_label_by_name(page_01_main_obj, page_01_main_len, "cfd_value_label", "%s", cfd[Machine_para.cfd_mode]);
    printf("cfd:%s\n", cfd[Machine_para.cfd_mode]);

}
void page_01_speed_refre(void)
{
    int speed[3] = { 800,1000,1200 };
    update_label_by_name(page_01_main_obj, page_01_main_len, "speed_num_label", "%d", speed[Machine_para.speed]);
}

void page_01_err_num_refre(void)
{
    char buf[4];
    snprintf(buf, sizeof(buf), "%d",sim.err_num);
    update_label_by_name(page_01_main_obj, page_01_main_len, "reject_num_label", "%s", buf);
}

void page_01_curr_img_refre(void)
{

    lv_obj_t* tmp_curr_img = find_obj_by_name("curr_USD_img", page_01_main_obj, page_01_main_len);
    lv_img_set_src(tmp_curr_img, get_currency_img(Machine_para.curr_code));
}


static void page_indicator_create(lv_obj_t* parent, int total_pages, int current_page)
{
    /* 创建容器 */
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 200, 100);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), 0);  // 背景色可自行调整
    lv_obj_set_style_radius(cont, 5, 0);
    lv_obj_set_style_border_width(cont, 2, 0);
    lv_obj_set_style_border_color(cont, lv_color_hex(0x555555), 0);
    lv_obj_center(cont);

    const lv_coord_t ball_d = 12;
    const lv_coord_t spacing = 8;
    lv_coord_t total_width = total_pages * ball_d + (total_pages - 1) * spacing;
    lv_coord_t start_x = (200 - total_width) / 2;

    for (int i = 0; i < total_pages; i++) {
        lv_obj_t* ball = lv_obj_create(cont);
        lv_obj_set_size(ball, ball_d, ball_d);
        lv_obj_set_style_radius(ball, LV_RADIUS_CIRCLE, 0);
        if (i == current_page) {
            lv_obj_set_style_bg_color(ball, lv_color_hex(0xFFFFFF), 0);
        }
        else {
            lv_obj_set_style_bg_color(ball, lv_color_hex(0x888888), 0);
        }
        lv_obj_set_pos(ball,
            start_x + i * (ball_d + spacing),
            (100 - ball_d) / 2);
    }
}

static lv_obj_t* curr_imgs[100];

static void scroll_snap_event_cb(lv_event_t* e) //页面吸附
{
    lv_obj_t* cont = lv_event_get_target(e);
    lv_coord_t x = lv_obj_get_scroll_x(cont);
    const int PAGE_W = 1072;
    const int X_OFF = 76;
    static int last_page = 0;
    int diff = x - (last_page * PAGE_W + X_OFF);
    int next = last_page;
    if (diff > 100)      next++;
    else if (diff < -100) next--;
    int max_page = (currencies_count + 7) / 8 - 1;
    if (next < 0)         next = 0;
    if (next > max_page)  next = max_page;
    last_page = next;
    page_indicator_create(curr_page, max_page + 1, last_page);
    lv_obj_scroll_to_x(cont, next * PAGE_W + X_OFF, LV_ANIM_ON);
}

static void page_07_cont_click_cb(lv_event_t* e)
{
    int idx = (int)(uintptr_t)lv_event_get_user_data(e);
    if (idx >= 0 && idx < currencies_count) {
        memcpy(Machine_para.curr_code, currencies[idx], 4);
        set_curr(get_curr_item(currencies[idx]));
        send_command(fd4, 0x03, (const uint8_t *)currencies[idx], 3);
        ui_manager_switch(UI_PAGE_MAIN);
    }
}

#define MAX_PAGES 10
static lv_obj_t* page_dots[MAX_PAGES];  // 存储小球对象
static int g_total_pages = 0;  //需要全局声明

//小球指示器函数 需要换文件位置
void tabview_event_cb(lv_event_t* e)
{
    lv_obj_t* tabview = lv_event_get_target(e);
    int cur = lv_tabview_get_tab_act(tabview);  // 当前页索引

    for (int i = 0; i < g_total_pages; i++) {
        lv_obj_set_style_bg_color(page_dots[i],
            (i == cur) ? lv_color_hex(0x0295FF) : lv_color_hex(0xFFFFFF), 0);
    }
}

void page_07_curr_img_refre(void)//图片刷新
{
    g_total_pages = (currencies_count + 7) / 8;

    lv_obj_t* tabview = lv_tabview_create(curr_page, LV_DIR_TOP, 0);
    lv_obj_set_size(tabview, 1051, 301);
    lv_obj_set_pos(tabview, 113, 84);
    lv_obj_set_style_bg_opa(tabview, LV_OPA_0, 0);
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_add_flag(tab_btns, LV_OBJ_FLAG_HIDDEN);
    // 注册事件回调
    lv_obj_add_event_cb(tabview, tabview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    for (int page = 0; page < g_total_pages; page++) {
        char tab_name[16];
        snprintf(tab_name, sizeof(tab_name), "page_%d", page);
        lv_obj_t* tab = lv_tabview_add_tab(tabview, tab_name);

        // 设置tab属性
        lv_obj_set_style_bg_opa(tab, LV_OPA_0, 0);
        lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);

        for (int i = 0; i < 8; i++) {
            int idx = page * 8 + i;
            if (idx >= currencies_count) break;

            int row = i / 4;
            int col = i % 4;
            int x = 61 + col * 254;
            int y = 16 + row * 138;

            curr_imgs[idx] = lv_img_create(tab);
            lv_img_set_src(curr_imgs[idx], get_currency_img(currencies[idx]));
            lv_obj_set_pos(curr_imgs[idx], x, y);
            lv_obj_set_size(curr_imgs[idx], 182, 103);

            lv_obj_add_flag(curr_imgs[idx], LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(curr_imgs[idx], page_07_cont_click_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)idx);

            lv_obj_t* lbl = lv_label_create(tab);
            lv_label_set_text(lbl, currencies[idx]);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, 0);
            lv_obj_set_style_text_color(lbl, lv_color_hex(0x000000), 0);
            lv_obj_set_pos(lbl, x + 70, y + 102);
        }
    }

    //小球指示器
    if (g_total_pages > 1) {
        lv_obj_t* indicator_cont = lv_obj_create(curr_page);
        lv_obj_set_style_border_width(indicator_cont, 0, LV_PART_MAIN);
        lv_obj_set_size(indicator_cont, 1051, 60);
        lv_obj_set_pos(indicator_cont, 113, 33);
        lv_obj_set_style_bg_opa(indicator_cont, LV_OPA_TRANSP, 0);
        lv_obj_clear_flag(indicator_cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_move_foreground(indicator_cont);

        int spacing = 20;
        int radius = 10;
        int total_w = g_total_pages * (radius * 2 + spacing) - spacing;
        int start_x = (1051 - total_w) / 2;

        for (int i = 0; i < g_total_pages; i++) {
            page_dots[i] = lv_obj_create(indicator_cont);
            lv_obj_set_size(page_dots[i], radius * 2, radius * 2);
            lv_obj_set_style_radius(page_dots[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(page_dots[i],(i == 0) ? lv_color_hex(0x0295FF) : lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_opa(page_dots[i], LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(page_dots[i], 0, LV_PART_MAIN);
            lv_obj_set_style_outline_width(page_dots[i], 0, LV_PART_MAIN);
            lv_obj_set_style_shadow_width(page_dots[i], 0, LV_PART_MAIN);
            lv_obj_clear_flag(page_dots[i], LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_pos(page_dots[i], start_x + i * (radius * 2 + spacing), 5);



        }
    }
}


