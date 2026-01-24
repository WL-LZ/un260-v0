#include "lvgl/lvgl.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_core/lv_page_manager.h"
#include "lv_page_event.h"
#include "un260/lv_system/platform_app.h"
#include <string.h>
#include "lvgl/src/misc/lv_timer.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "un260/lv_core/page_03_menu.h"
#include "un260/lv_drivers/lv_drivers.h"

lv_timer_t* page_03_batch_num_del_timer = NULL;
lv_timer_t* page_05_password_del_timer = NULL;

// 声明外部变量


//跳转页面
void page_switch_btn_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        ui_page_t target_page = (ui_page_t)(uintptr_t)lv_event_get_user_data(e);
        ui_manager_switch(target_page);
    }
}


void page_01_list_btn_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        icon_feedback_comp("page_01_list_icon.png", page_01_main_obj, page_01_main_len);

        ui_manager_push_page(UI_PAGE_LIST);
    }
}
void page_01_menu_btn_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        icon_feedback_comp("page_01_menu_icon.png", page_01_main_obj, page_01_main_len);

        ui_manager_push_page(UI_PAGE_MENU);
    }
}

 void page_01_back_btn_event_cb(lv_event_t* e) {

     if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
         ui_manager_pop_page();
     }
 }

void page_01_start_btn_event_cb(lv_event_t* e) // 开始仿真
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        icon_feedback_comp("page_01_start_icon.png", page_01_main_obj, page_01_main_len);

         //start_counting_sim();
        sim_data_init(); //
        int fd4 = uart_open("/dev/ttyS4");        
        uart_config(fd4, 115200, 8, 'N', 1);     
        unsigned char atb_cmd[6] = {0xFD, 0xDF, 0x06, 0x0A, 0x01 , 0x01};
        uart_send(fd4, (char*)atb_cmd, 6);           
        uart_close(fd4);
    }
}


 void page_01_esc_btn_event_cb(lv_event_t* e)
 {
     if (lv_event_get_code(e) == LV_EVENT_CLICKED)
         icon_feedback_comp("page_01_esc_icon.png", page_01_main_obj, page_01_main_len);

     stop_counting_sim();
     sim_clear_all_sn(&sim);
    

 }

// static Machine_Mode_t mode_next(int temp_mode)
// {
//     switch(temp_mode) {
//         case MODE_MDC: return Machine_MODE_SDC;
//         case MODE_SDC: return Machine_MODE_CNT;
//         case MODE_CNT: return Machine_MODE_MDC;
//         default:               return Machine_MODE_MDC; // 循环回到起点
//     }
// }


// void page_01_mode_btn_event_cb(lv_event_t* e)
// {
//     if (lv_event_get_code(e) == LV_EVENT_CLICKED)
//     {   
//         // 用当前模式来循环，而不是每次都从 MDC 开始
//         Machine_Mode_t Temp_Mode = mode_next(Machine_para.mode);
//         Machine_para.mode = Temp_Mode;  // 更新当前模式

//         uart_printf(fd6, "mode:%02X\n", Temp_Mode);
//         icon_feedback_comp("page_01_mode_icon.png", page_01_main_obj, page_01_main_len);

//         uint8_t send_data = 0;

//         if (strcmp(Machine_para.curr_code, "AUT") == 0)
//         {
//             send_data = Machine_AUT_MODE_MDC;
//         }
//         else if (strcmp(Machine_para.curr_code, "MUL") == 0)
//         {
//             send_data = Machine_MUL_MODE_MDC;
//         }
//         else
//         {
//             // 普通循环模式
//             send_data = Temp_Mode;
//         }

//         send_command(fd4, 0x04, &send_data, 1);  // 发送字节
//         Machine_work_code.mode_code = send_data; // 保存当前发送的模式代码
//         uart_printf(fd6, "send_data:%02X\n", send_data);
        
//     }
// }

 void page_01_mode_btn_event_cb(lv_event_t* e)
 {
     if (lv_event_get_code(e) == LV_EVENT_CLICKED)
     {
         icon_feedback_comp("page_01_mode_icon.png", page_01_main_obj, page_01_main_len);

         if (Machine_para.mode == MODE_MDC)
             Machine_para.mode = MODE_SDC;
         else if (Machine_para.mode == MODE_SDC)
             Machine_para.mode = MODE_CNT;
         else if (Machine_para.mode == MODE_CNT)
             Machine_para.mode = MODE_MDC;
         else
             Machine_para.mode = MODE_MDC;
         page_01_mode_switch_refre();
         
         uint8_t mode_cmd = 0;
         if(Machine_para.mode == MODE_MDC)
              mode_cmd = 0x03;
         else if (Machine_para.mode == MODE_SDC)
              mode_cmd = 0x04;
         else if (Machine_para.mode == MODE_CNT)
              mode_cmd = 0x05;
         else       
         {
            Machine_para.mode = MODE_MDC;
            mode_cmd = 0x03;
         }
         
         send_command(fd4, 0x04, &mode_cmd, 1);
         
        sim_data_init(); //


     }

 }


void page_01_set_btn_event_cb(lv_event_t* e){
 if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
     icon_feedback_comp("page_01_set_icon.png", page_01_main_obj, page_01_main_len);

     ui_manager_switch(UI_PAGE_SET_PASSAGE);
     }
 }

void page_01_curr_btn_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {

        ui_manager_switch(UI_PAGE_CURR);
    }

}

//设置密码界面按键输入
void page_05_set_password_keypad_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const char* password_get_txt = lv_event_get_user_data(e);
    if (!password_get_txt || password_index >= 4) return;

    input_password[password_index] = password_get_txt[0];
    password_index++;
    input_password[password_index] = '\0';
    lv_label_set_text(password_display, input_password);


}

//密码清除事件
void page_05_set_password_keypad_clear_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    icon_feedback_comp("page_03_ok_icon.png", page_05_set_password_obj, page_05_set_password_len);

    memset(input_password,0,sizeof(input_password));
    password_index = 0;
    lv_label_set_text(password_display, "");

}


// page_05 密码错误定时器
void page_05_error_label_timer_cb(lv_timer_t* timer) {
    lv_obj_t* error_label = (lv_obj_t*) timer->user_data;  // v8 写法
    if (error_label && lv_obj_is_valid(error_label)) {
        lv_obj_del(error_label);
    }
    lv_timer_del(timer);
    page_05_password_del_timer = NULL;
    printf("dress:%p\n", &page_05_password_del_timer);
}

// page_03 batch_num定时器
void page_03_delete_tip_label_cb(lv_timer_t* t) {
    lv_obj_t* lbl = (lv_obj_t*) t->user_data;  // v8 写法
    if (lbl && lv_obj_is_valid(lbl)) {
        lv_obj_del(lbl);
    }
    lv_timer_del(t);
    page_03_batch_num_del_timer = NULL;
    printf("dress:%p\n", &page_03_batch_num_del_timer);
}






//设置密码界面确认时间
void page_05_set_password_keypad_enter_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    icon_feedback_comp("page_03_del_icon.png", page_05_set_password_obj, page_05_set_password_len);

    if (page_05_password_del_timer) {
        lv_timer_del(page_05_password_del_timer);
        page_05_password_del_timer = NULL;
        printf("del\n");
    }
    if (strcmp(Machine_para.password, input_password) == 0)
        ui_manager_switch(UI_PAGE_SETTING);
    else
    {
        lv_obj_t* err_txt;
        memset(input_password, 0, sizeof(input_password));
        password_index = 0;
        lv_label_set_text(password_display, "");

        err_txt = lv_label_create(set_password_page);
        lv_obj_set_style_text_color(err_txt, lv_color_make(0,0,0), 0);
        lv_label_set_text(err_txt, "password error!");
        lv_obj_align(err_txt, LV_ALIGN_TOP_MID, 0, 150);

        page_05_password_del_timer = lv_timer_create(page_05_error_label_timer_cb,2000,err_txt);
    }

}


// 输入事件回调函数（处理触摸事件）

void page_03_batch_label_input_event_cb(lv_event_t* e)
{
    if (!Machine_para.batch_switch_enable) return;

    static struct {
        bool is_dragging;      // 是否正在拖拽
        lv_coord_t start_y;    // 开始Y坐标
        lv_coord_t current_y;  // 当前Y坐标
        bool has_switched;     // 是否已经切换过
    } drag_state = { false, 0, 0, false };
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t* indev = lv_indev_get_act();
    lv_point_t point;
    lv_indev_get_point(indev, &point);

    switch (code) {
    case LV_EVENT_PRESSED:
        drag_state.is_dragging = true;
        drag_state.start_y = point.y;
        drag_state.current_y = point.y;
        drag_state.has_switched = false;
        break;

    case LV_EVENT_PRESSING:
        if (drag_state.is_dragging) {
            drag_state.current_y = point.y;
            lv_coord_t diff = drag_state.current_y - drag_state.start_y;

            if (!drag_state.has_switched && abs(diff) > 20) {
                // 进行一次模式切换
                toggle_batch_mode();
                drag_state.has_switched = true;
            }
        }
        break;

    case LV_EVENT_RELEASED:
        drag_state.is_dragging = false;
        drag_state.has_switched = false;
        break;

    case LV_EVENT_GESTURE:
    {
        // 切换模式
        lv_dir_t dir = lv_indev_get_gesture_dir(indev);
        if (dir == LV_DIR_TOP || dir == LV_DIR_BOTTOM) {
            toggle_batch_mode();
        }
    }
    break;

    default:
        break;
    }
    page_03_batch_mode_status_refre();
}


// 手势事件回调函数
void page_03_void_batch_label_gesture_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());

        if (dir == LV_DIR_BOTTOM) {
            switch_to_pcs_batch();
        }
        else if (dir == LV_DIR_TOP) {
            switch_to_amount_batch();
        }
    }
}



void page_03_batch_num_keypad_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const char* password_get_txt = lv_event_get_user_data(e);
    if (!password_get_txt) return;
    char input_num = password_get_txt[0];
    // 禁止多位前导0
    if (input_num == '0')
    {
        if (batch_num_index == 0 || (batch_num_index == 1 && input_batch_num[0] == '0'))  {
            
            input_batch_num[0] = '0';
            input_batch_num[1] = '\0';
            batch_num_index = 1;
            lv_label_set_text(batch_num_display, input_batch_num);
            lv_obj_set_align(batch_num_display, LV_ALIGN_RIGHT_MID);
            return;
        }
    }
    if (batch_num_index == 1 && input_num != '0'&& input_batch_num[0] == '0')   //判断当前是否在第一位为0时做预置数输入
    {
        input_batch_num[0] = input_num;
        input_batch_num[1] = '\0';
        batch_num_index = 1;
        lv_label_set_text(batch_num_display, input_batch_num);
        lv_obj_set_align(batch_num_display, LV_ALIGN_RIGHT_MID);
        return;
    }
    if (batch_num_index >= 8)
    {
        for (int i = 0; i < 7; i++)
        {
            input_batch_num[i] = input_batch_num[i+1];

        }
        input_batch_num[7] = input_num;
        input_batch_num[8] = '\0';

    }
    else
    {
        input_batch_num[batch_num_index] = input_num;
        batch_num_index++;
        input_batch_num[batch_num_index] = '\0';

    }
    lv_label_set_text(batch_num_display,input_batch_num);
    lv_obj_set_align(batch_num_display, LV_ALIGN_RIGHT_MID);
}

void page_03_batch_num_keypad_clear_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    icon_feedback_comp("page_03_ok_icon.png", page_03_menu_obj, page_03_menu_len);
    memset(input_batch_num, 0, sizeof(input_batch_num));
    batch_num_index = 0;
    lv_label_set_text(batch_num_display, "0");
}




void page_03_batch_num_keypad_enter_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    icon_feedback_comp("page_03_del_icon.png", page_03_menu_obj, page_03_menu_len);

    if (page_03_batch_num_del_timer) {
        lv_timer_del(page_03_batch_num_del_timer);
        page_03_batch_num_del_timer = NULL;
        printf("del\n");

    }
    int num = atoi(input_batch_num);
    Machine_para.batch_num = num;
    // 清空输入缓存
    memset(input_batch_num, 0, sizeof(input_batch_num));
    batch_num_index = 0;

    // 更新显示标签
    lv_obj_t* set_succ;
    lv_label_set_text(batch_num_display, "0");
    set_succ = lv_label_create(menu_page);
    lv_obj_set_style_text_color(set_succ, lv_color_make(150,150, 150), 0);
    lv_label_set_text(set_succ, "Batch num saved successfully!");
    lv_obj_set_pos(set_succ, 160, 207);
    page_03_batch_num_del_timer = lv_timer_create(page_03_delete_tip_label_cb, 2000, set_succ);

    update_label_by_name(page_03_menu_obj, page_03_menu_len, "03_batch_num_label", "%d", Machine_para.batch_num);

    printf("%d\n", Machine_para.batch_num);


}
void page_03_update_menu_button_states_refresh(void)
{
    const char* page_03_cfd_mode_obj[] = { "03_cfd_l_btn","03_cfd_m_btn","03_cfd_h_btn" };
    const char* page_03_speed_mode_obj[] = { "03_speed_800_btn","03_speed_1000_btn","03_speed_1200_btn" };
    const char* page_03_add_mode_obj[] = {"03_add_on_btn","03_add_off_btn"};
    const char* page_03_fo_mode_obj[] = {"03_fo_OFF_btn","03_fo_F_btn","03_fo_O_btn","03_fo_FO_btn"};
    const char* page_03_work_mode_obj[] = { "03_work_auto_btn", "03_work_manaul_btn"};
    lv_color_t selected_color = lv_color_make(72, 172, 80);
    lv_color_t unselected_color = lv_color_make(248,252,248);
    lv_color_t selected_text_color = lv_color_make(255, 255, 255);
    lv_color_t unselected_text_color = lv_color_make(96, 100, 96);
    lv_color_t enable_color = lv_color_make(214, 45, 45);

    //CFD处理
    for (int i = 0; i < CFD_MODE; i++)
    {
        lv_obj_t* tmp_cfd_obj = find_obj_by_name(page_03_cfd_mode_obj[i], page_03_menu_obj, page_03_menu_len);
        bool sel = (i == Machine_para.cfd_mode);
        lv_obj_set_style_bg_color(tmp_cfd_obj,
            sel ? selected_color : unselected_color, 0);
        lv_obj_t* label_cfd = lv_obj_get_child(tmp_cfd_obj, 0);
        lv_obj_set_style_text_color(
            label_cfd,
            sel ? selected_text_color : unselected_text_color,
            0 
        );
    }
    //speed 处理
    for (int i = 0; i < SPEED_MODE; i++)
    {
        lv_obj_t* tmp_speed_obj = find_obj_by_name(page_03_speed_mode_obj[i], page_03_menu_obj, page_03_menu_len);
        bool sel = (i == Machine_para.speed);
        lv_obj_set_style_bg_color(tmp_speed_obj,
            sel ? selected_color : unselected_color, 0);
        lv_obj_t* label_speed = lv_obj_get_child(tmp_speed_obj, 0);
        lv_obj_set_style_text_color(
            label_speed,
            sel ? selected_text_color : unselected_text_color,
            0 
        );
    }
    //FO处理
    for (int i = 0; i < FO_MODE; i++)
    {
        lv_obj_t* tmp_fo_obj = find_obj_by_name(page_03_fo_mode_obj[i], page_03_menu_obj, page_03_menu_len);
        bool sel = (i == Machine_para.fo_mode);
        if (i == 0)
        {
            lv_obj_set_style_bg_color(tmp_fo_obj,
                sel ? enable_color : unselected_color, 0);
        }
        else
        {
            lv_obj_set_style_bg_color(tmp_fo_obj,
                sel ? selected_color : unselected_color, 0);
        }
        lv_obj_t* label_fo = lv_obj_get_child(tmp_fo_obj, 0);
        lv_obj_set_style_text_color(
            label_fo,
            sel ? selected_text_color : unselected_text_color,
            0  
        );
    }
    //work处理
    for (int i = 0; i < WORK_MODE; i++)
    {
        lv_obj_t* tmp_work_obj = find_obj_by_name(page_03_work_mode_obj[i], page_03_menu_obj, page_03_menu_len);
        bool sel = (i == Machine_para.work_mode);
        lv_obj_set_style_bg_color(tmp_work_obj,
            sel ? selected_color : unselected_color, 0);
        lv_obj_t* label_work = lv_obj_get_child(tmp_work_obj, 0);
        lv_obj_set_style_text_color(
            label_work,
            sel ? selected_text_color : unselected_text_color,
            0  
        );
    }
    //ADD 处理
        lv_obj_t* tmp_add_on_obj = find_obj_by_name(page_03_add_mode_obj[0], page_03_menu_obj, page_03_menu_len);
        lv_obj_t* tmp_add_off_obj = find_obj_by_name(page_03_add_mode_obj[1], page_03_menu_obj, page_03_menu_len);
        bool sel =  Machine_para.add_enabel;
        if (sel)
        {
            lv_obj_set_style_bg_color(tmp_add_on_obj,selected_color , 0);
            lv_obj_set_style_bg_color(tmp_add_off_obj, unselected_color, 0);

        }
        else
        {
            lv_obj_set_style_bg_color(tmp_add_off_obj, enable_color, 0);
            lv_obj_set_style_bg_color(tmp_add_on_obj, unselected_color, 0);

        }
        lv_obj_t* label_add_on = lv_obj_get_child(tmp_add_on_obj, 0);
        lv_obj_set_style_text_color(
            label_add_on,
            sel ? selected_text_color : unselected_text_color,
            0 
        );
        lv_obj_t* label_add_off = lv_obj_get_child(tmp_add_off_obj, 0);
        lv_obj_set_style_text_color(
            label_add_off,
            !sel ? selected_text_color : unselected_text_color,
            0  
        );

}

//cfd 模式
void page_03_cfd_mode_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const char* cfd_str = lv_event_get_user_data(e);
    uint8_t cfd_code = atoi(cfd_str);
    Machine_para.cfd_mode = cfd_code;
    page_03_update_menu_button_states_refresh();
    char cfd[3] = { 'L','M','H'};
#if LV_DEBUG
    printf("CFD模式切换到: %c\n", cfd[Machine_para.cfd_mode]);
#endif


}

//speed模式
void page_03_speed_mode_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const char* speed_str = lv_event_get_user_data(e);
    uint8_t speed_code = atoi(speed_str);
    Machine_para.speed = speed_code;
    page_03_update_menu_button_states_refresh();
#if LV_DEBUG
    printf("速度模式切换到： %d\n", Machine_para.speed);
#endif // LV_DEBUG

}

//ADD模式
void page_03_add_mode_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const char* add_str = lv_event_get_user_data(e);
    uint8_t add_code = atoi(add_str);
    Machine_para.add_enabel = (add_code > 0) ? true : false;
    page_03_update_menu_button_states_refresh();
#if LV_DEBUG
    printf("ADD模式切换为：%s\n", Machine_para.add_enabel ? "ON" : "OFF");
#endif // LV_DEBUG
}

//fo模式
void page_03_fo_mode_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    const char* fo_str = lv_event_get_user_data(e);
    uint8_t fo_code = atoi(fo_str);
    Machine_para.fo_mode = fo_code;
    page_03_update_menu_button_states_refresh();
#if LV_DEBUG
    char* fo[] = {"OFF","F","O","F/O"};
    printf("F/O 模式切换为：%s\n", fo[Machine_para.fo_mode]);
#endif // LV_DEBUG


}


//work模式
void page_03_work_mode_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)return;
    const char* word_str = lv_event_get_user_data(e);
    uint8_t word_code = atoi(word_str);
    Machine_para.work_mode = word_code;
    page_03_update_menu_button_states_refresh();
#if LV_DEBUG
    printf("工作模式切换为：%s\n", (Machine_para.work_mode > 0) ? "MANUAL" : "AUTO");
#endif // LV_DEBUG

}


void page_03_a_up_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        page_02_a_report_status.curent_page--;

        if (page_02_a_report_status.curent_page == 0)
            page_02_a_report_status.curent_page = page_02_a_report_status.total_page;

        page_02_a_page_refre();
        page_02_a_page_num_refre();
    }
}
void page_03_a_down_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        page_02_a_report_status.curent_page++;

        if (page_02_a_report_status.curent_page > page_02_a_report_status.total_page)
            page_02_a_report_status.curent_page = 1;

        page_02_a_page_refre();
        page_02_a_page_num_refre();
    }
}

void page_03_b_up_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        page_02_b_report_status.curent_page--;

        if (page_02_b_report_status.curent_page == 0)
            page_02_b_report_status.curent_page = page_02_b_report_status.total_page;

        page_02_b_page_refre();
        page_02_b_page_num_refre();
    }
}
void page_03_b_down_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        page_02_b_report_status.curent_page++;

        if (page_02_b_report_status.curent_page > page_02_b_report_status.total_page)
            page_02_b_report_status.curent_page = 1;

        page_02_b_page_refre();
        page_02_b_page_num_refre();
    }
}

void page_03_c_up_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        page_02_c_report_status.curent_page--;

        if (page_02_c_report_status.curent_page == 0)
            page_02_c_report_status.curent_page = PAGE_02_DEBUG;

        page_02_c_page_refre();
        page_02_c_page_num_refre();
    }
}
void page_03_c_down_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        page_02_c_report_status.curent_page++;

        if (page_02_c_report_status.curent_page > PAGE_02_DEBUG)
            page_02_c_report_status.curent_page = 1;

        page_02_c_page_refre();
        page_02_c_page_num_refre();
    }
}

