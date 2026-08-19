#include <stdio.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_core/page_01_detail_scroll.h"
#include "un260/lv_core/page_01_main.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/machine_state/machine_state.h"
#include "un260/currency/currency_state.h"
#include "un260/lv_core/lv_page_event.h"
#include "un260/lv_core/page_02_list.h"
#include "un260/lv_core/lv_page_declear.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_resources/lv_img_init.h"

//主界面右侧详情数据写入容器

void page_01_mode_switch_refre()
{
    const char* mode_str = "NONE";

    switch (machine_state_mode())
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

    page_01_detail_scroll_attach(main_page, page_01_main_scroll_container);
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

    printf("machine batch mode: %s\n", machine_state_batch_mode()?"AMOUNT MODE":"PCS MODE");

    if (amount_obj && pcs_obj)
    {

        lv_obj_set_parent(amount_obj, page_03_batch_container);
        lv_obj_set_parent(pcs_obj, page_03_batch_container);

        // 根据已确认的批次模式初始化状态和位置
        if (machine_state_batch_mode() == AMOUNT_BATCH_MODE)
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

    if (!machine_state_batch_enabled())
    {
        if (machine_state_batch_mode() == AMOUNT_BATCH_MODE)
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

void page_03_batch_num_refre(void)
{
    if (machine_state_batch_enabled()) {
        if (machine_state_batch_num() > 0) {
            update_label_by_name(page_03_menu_obj, page_03_menu_len, "03_batch_num_label", "%d", machine_state_batch_num());
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
    if (!machine_state_batch_enabled())
    {
        if (machine_state_batch_mode() == AMOUNT_BATCH_MODE)
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
        if (machine_state_batch_mode() == AMOUNT_BATCH_MODE)
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

void page_01_add_refre(void)
{
    
    update_label_by_name(page_01_main_obj, page_01_main_len, "add_label", "%s", machine_state_add_enabled()?"ADD:ON":"ADD:OFF");
    page_01_bottom_a_refresh_add(false);

}
void page_01_work_refre(void)
{
    char* work[2] = {"AUTO","MANUAL"};
    update_label_by_name(page_01_main_obj, page_01_main_len, "auto_label", "%s", work[machine_state_work_mode()]);
    page_01_bottom_a_refresh_work(false);

}
void page_01_batch_refre(void)
{
    char* batch[2] = { "BATCH :","VBATCH :" };
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", machine_state_batch_num());
    update_label_by_name(page_01_main_obj, page_01_main_len, "bacth_label", "%s", batch[machine_state_batch_mode()]);
    if (machine_state_batch_enabled())
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

    update_label_by_name(page_01_main_obj, page_01_main_len, "face_label", "%s", batch[machine_state_fo_mode()]);
    page_01_bottom_a_refresh_fo(false);

}
void page_01_cfd_refre(void)
{
    char* cfd[3] = { "L","M","H"  };
    update_label_by_name(page_01_main_obj, page_01_main_len, "cfd_value_label", "%s", cfd[machine_state_cfd_mode()]);
    printf("cfd:%s\n", cfd[machine_state_cfd_mode()]);
    page_01_bottom_c_refresh_cfd();

}
void page_01_speed_refre(void)
{
    int speed[3] = { 600,800,1000 };
    update_label_by_name(page_01_main_obj, page_01_main_len, "speed_num_label", "%d", speed[machine_state_speed()]);
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
    char curr_code[4];

    lv_obj_t* tmp_curr_img = find_obj_by_name("curr_USD_img", page_01_main_obj, page_01_main_len);
    currency_state_get_active_code(curr_code);
    lv_img_set_src(tmp_curr_img, get_currency_img(curr_code));
}
