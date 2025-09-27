#include "platform_app.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "un260/lv_resources/lv_img_init.h" 
#include "user_cfg.h"
#include "un260/lv_core/lv_page_manager.h"

// 全局变量定义
counting_sim_t sim = { 0 };
page_02_report_status_t page_02_a_report_status = { 0 };
page_02_report_status_t page_02_b_report_status = { 0 };
page_02_report_status_t page_02_c_report_status = { 0 };

lv_timer_t* sim_timer = NULL;
lv_obj_t* page_01_main_scroll_container = NULL;
lv_obj_t* page_01_main_page_pcs_label = NULL;
lv_obj_t* page_01_main_page_amount_label = NULL;
lv_timer_t* safe_reset_timer = NULL;

//金额模拟
const int USD_value[] = { 100,50,20,10,5,1 };
const int CNY_value[] = { 100,50,20,10,5,1 };
const int EUR_value[] = { 200,100,50,20,10,5 };
const int GBP_value[] = { 50,20,10,5,2,1 };
const int KRW_value[] = { 500,400,300,200,100,50,10,5,1 };
const int EGP_value[] = { 200, 100, 50, 20, 10, 5, 1 };
const int ISK_value[] = { 10000, 5000, 2000, 1000, 500, 100 };
const int PHP_value[] = { 1000, 500, 200, 100, 50, 20 };
const int SOS_value[] = { 1000, 500, 100, 50, 20, 10, 5, 1 };
const int TRY_value[] = { 200, 100, 50, 20, 10, 5, 1 };

const int USD_value_num = sizeof(USD_value) / sizeof(USD_value[0]);
const int CNY_value_num = sizeof(CNY_value) / sizeof(CNY_value[0]);
const int EUR_value_num = sizeof(EUR_value) / sizeof(EUR_value[0]);
const int GBP_value_num = sizeof(GBP_value) / sizeof(GBP_value[0]);
const int KRW_value_num = sizeof(KRW_value) / sizeof(KRW_value[0]);
const int EGP_value_num = sizeof(EGP_value) / sizeof(EGP_value[0]);
const int ISK_value_num = sizeof(ISK_value) / sizeof(ISK_value[0]);
const int PHP_value_num = sizeof(PHP_value) / sizeof(PHP_value[0]);
const int SOS_value_num = sizeof(SOS_value) / sizeof(SOS_value[0]);
const int TRY_value_num = sizeof(TRY_value) / sizeof(TRY_value[0]);

// 用于在页面切换时保存计数数据的临时存储
static counting_sim_t saved_sim_data;
static bool has_saved_data = false;

//获取obj对象
lv_obj_t* find_obj_by_name(const char* name, ui_element_t* page_cfg_obj, int len) {
    if (!name || !page_cfg_obj) {
#if LV_DEBUG
        printf("find_obj_by_name: name or page_cfg_obj is NULL!\n");
#endif
        return NULL;
    }

    for (int i = 0; i < len; i++) {
        if (strcmp(page_cfg_obj[i].obj_name, name) == 0) {
            if (page_cfg_obj[i].obj_ref && lv_obj_is_valid(page_cfg_obj[i].obj_ref)) {
                return page_cfg_obj[i].obj_ref;
            }
            else {
#if LV_DEBUG
                printf("find_obj_by_name: obj_ref invalid for name %s\n", name);
#endif
                return NULL;
            }
        }
    }

#if LV_DEBUG
    printf("find_obj_by_name: name %s not found\n", name);
#endif
    return NULL;
}

/* 把字符串码映射到枚举 */
static curr_item_t get_curr_item(const char* code) {
    if (strcmp(code, "CNY") == 0) return CURR_CNY_ITEM;
    if (strcmp(code, "USD") == 0) return CURR_USD_ITEM;
    if (strcmp(code, "EUR") == 0) return CURR_EUR_ITEM;
    if (strcmp(code, "GBP") == 0) return CURR_GBP_ITEM;
    if (strcmp(code, "KRW") == 0) return CURR_KRW_ITEM;
    if (strcmp(code, "EGP") == 0) return CURR_EGP_ITEM;
    if (strcmp(code, "ISK") == 0) return CURR_ISK_ITEM;
    if (strcmp(code, "PHP") == 0) return CURR_PHP_ITEM;
    if (strcmp(code, "SOS") == 0) return CURR_SOS_ITEM;
    if (strcmp(code, "TRY") == 0) return CURR_TRY_ITEM;


    return CURR_COUNT;  // 不认识
}

//刷新字符串
void update_label_by_name(ui_element_t* page_cfg_obj, int len,const char* name, const char* fmt, ...) {
    lv_obj_t* label = find_obj_by_name(name, page_cfg_obj, len);
    if (!label || !lv_obj_is_valid(label) || !lv_obj_check_type(label, &lv_label_class)) return;

    static char buf[64];
    va_list args;
    va_start(args, fmt);
    lv_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    lv_label_set_text(label, buf);
}

//设置国家curr
void set_curr(curr_item_t curr)
{
    if (curr > CURR_COUNT)
    {
#if LV_DEBUG
        printf("set_curr is error");
#endif
        return;
    }
    if (curr == Machine_para.current_currency) return;

    Machine_para.current_currency = curr;
    sim_data_init();
    ui_refresh_main_page();

}

void sim_data_init(void)
{
    printf("Current currency enum: %d\n", Machine_para.current_currency);

    counting_sim_t* sim_data = &sim;
    memset(sim_data, 0, sizeof(counting_sim_t));
    const int* arr = NULL;
    int count = 0;

    switch (get_curr_item(Machine_para.curr_code))
    {
    case CURR_USD_ITEM:
        arr = USD_value;
        count = USD_value_num;
        strcpy(Machine_para.curr_code, "USD");
        break;
    case CURR_CNY_ITEM:
        arr = CNY_value;
        count = CNY_value_num;
        strcpy(Machine_para.curr_code, "CNY");
        break;
    case CURR_GBP_ITEM:
        arr = GBP_value;
        count = GBP_value_num;
        strcpy(Machine_para.curr_code, "GBP");
        break;
    case CURR_EUR_ITEM:
        arr = EUR_value;
        count = EUR_value_num;
        strcpy(Machine_para.curr_code, "EUR");
        break;
    case CURR_KRW_ITEM:
        arr = KRW_value;
        count = KRW_value_num;
        strcpy(Machine_para.curr_code, "KRW");
        break;
    default:
        arr = USD_value;
        count = USD_value_num;
        strcpy(Machine_para.curr_code, "USD");
        break;
    }
    for (int i = 0; i < count; i++)
    {
        sim_data->denom[i].value = arr[i];
        sim_data->denom[i].amount = 0;
        sim_data->denom[i].pcs = 0;
    }
    sim_data->denom_number = count;
    sim_data->total_amount = 0;
    sim_data->total_pcs = 0;
    sim_data->is_paused = false;  // 初始化暂停标志为false

}


void sim_timer_cb(lv_timer_t* timer)
{
    (void)timer;
    counting_sim_t* sim_data = &sim;
    
    if (sim.denom_number <= 0)  
    {
#if LV_DEBUG
        printf("denom_number is invalid: %d\n", sim.denom_number);
#endif
        return;
    }
    sim_data->err_num = 1;
    int ridx = lv_rand(0, sim_data->denom_number - 1);
    int delta = lv_rand(1, 5);
    sim_data->denom[ridx].pcs += delta;
    sim_data->denom[ridx].amount = sim_data->denom[ridx].value * sim_data->denom[ridx].pcs;
    
    int total_pcs = 0;
    float total_amount = 0;
    for (int i = 0; i < sim_data->denom_number; i++)
    {
        total_pcs += sim_data->denom[i].pcs;
        total_amount += sim_data->denom[i].amount;
    }
    
    
    char buf[12];  
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int charset_size = sizeof(charset) - 1;
    
    if (total_pcs > sim_data->total_pcs)
    {
        char** temp_sn_str = malloc(sizeof(char*) * total_pcs);
        if (temp_sn_str == NULL)
        {
#if LV_DEBUG
            printf("Failed to allocate memory for temp_sn_str\n");
#endif
            return;
        }
        
        int old_total_pcs = sim_data->total_pcs;
        
       
        if (sim_data->sn_str != NULL)
        {
            for (int i = 0; i < old_total_pcs; i++)
            {
                temp_sn_str[i] = sim_data->sn_str[i];
            }
            free(sim_data->sn_str);
        }
        else
        {
            
            for (int i = 0; i < old_total_pcs; i++)
            {
                temp_sn_str[i] = NULL;
            }
        }
        
        for (int i = old_total_pcs; i < total_pcs; i++)
        {
            sim_data->denom_mix[i] = sim_data->denom[lv_rand(0, sim.denom_number - 1)].value;
            temp_sn_str[i] = malloc(12);
            if (temp_sn_str[i] == NULL)
            {
#if LV_DEBUG
                printf("Failed to allocate memory for sn_str[%d]\n", i);
#endif
                for (int j = old_total_pcs; j < i; j++)
                {
                    if (temp_sn_str[j] != NULL)  
                    {
                        free(temp_sn_str[j]);
                    }
                }
                free(temp_sn_str);
                return;
            }
            
            // 生成随机冠字号
            for (int j = 0; j < 11; j++)
            {
                buf[j] = charset[lv_rand(0, charset_size - 1)];
            }
            buf[11] = '\0';
            strcpy(temp_sn_str[i], buf);
        }
        
        sim_data->sn_str = temp_sn_str;
    }
    
    sim_data->total_pcs = total_pcs;
    sim_data->total_amount = total_amount;
    sim_data->err_num = 1;

    ui_refresh_main_page();
}

void safe_reset_cb(lv_timer_t* timer)
{
    sim_data_init();
    ui_refresh_main_page();
    safe_reset_timer = NULL;
}
void start_counting_sim(void) {
    counting_sim_t* sim_data = &sim;
    
    // 如果计时器已存在但被暂停，则恢复它
    if (sim_timer && sim_data->is_paused) {
        resume_counting_sim();
        return;
    }
    else if (!sim_timer) {
        sim_data_init();
        sim_timer = lv_timer_create(sim_timer_cb, 200, NULL);
    }
}


void stop_counting_sim(void)
{
    if (!sim_timer) return;
    
    lv_timer_del(sim_timer);
    sim_timer = NULL;
    
    counting_sim_t* sim_data = &sim;
    sim_data->is_paused = false;  // 重置暂停标志

    if(safe_reset_timer)
    {
        lv_timer_del(safe_reset_timer);
    }
    safe_reset_timer = lv_timer_create(safe_reset_cb, 5, NULL);
    lv_timer_set_repeat_count(safe_reset_timer, 1);
}
//处理金额格式
void format_amount_with_comma(char* dest, size_t dest_size, float amount) {
    
    char temp[32];
    snprintf(temp, sizeof(temp), "%.0f", amount);

    int len = strlen(temp);
    int comma_count = (len - 1) / 3;  

    
    if (len <= 3) {
        strncpy(dest, temp, dest_size - 1);
        dest[dest_size - 1] = '\0';
        return;
    }

   
    int dest_index = 0;
    int digit_count = 0;

   
    for (int i = 0; i < len; i++) {
        if (dest_index >= dest_size - 1) break;

        dest[dest_index++] = temp[i];
        digit_count++;

       
        if (i < len - 1 && (len - i - 1) % 3 == 0) {
            if (dest_index < dest_size - 1) {
                dest[dest_index++] = ',';
            }
        }
    }

    dest[dest_index] = '\0';
}


//主界面右侧详情数据初始化和写入
void ui_refresh_main_page(void) {
    counting_sim_t* sim_data = &sim;
    char buf[32];
    char amount_buf[32];

    //main_left_list
    lv_obj_t* curr_label;
    curr_label = find_obj_by_name("curr_icon_label", page_01_main_obj, page_01_main_len);

    if (curr_label && lv_obj_is_valid(curr_label))
    {
        switch (get_curr_item(Machine_para.curr_code))
        {

        case CURR_USD_ITEM:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "USD");
            break;
        case CURR_CNY_ITEM:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "CNY");
            break;
        case CURR_GBP_ITEM:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "GBP");
            break;
        case CURR_EUR_ITEM:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "EUR");
            break;

        case CURR_KRW_ITEM:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "KRW");
            break;


        case CURR_EGP_ITEM:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "EGP");
            break;
        case CURR_ISK_ITEM:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "ISK");
            break;
        case CURR_PHP_ITEM:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "PHP");
            break;
        case CURR_SOS_ITEM:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "SOS");
            break;
        case CURR_TRY_ITEM:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "TRY");
            break;

        default:
            update_label_by_name(page_01_main_obj, page_01_main_len, "curr_icon_label", "NONE");
            break;
        }


    }
    if (page_01_main_page_pcs_label == NULL || !lv_obj_is_valid(page_01_main_page_pcs_label))
    {
        page_01_main_page_pcs_label = find_obj_by_name("01_pcs_label", page_01_main_obj, page_01_main_len);
    }
    if (page_01_main_page_pcs_label || lv_obj_is_valid(page_01_main_page_pcs_label))
    {
        snprintf(buf, sizeof(buf), "%d", sim_data->total_pcs);
        lv_label_set_text(page_01_main_page_pcs_label, buf);
    }
    if (page_01_main_page_amount_label == NULL || !lv_obj_is_valid(page_01_main_page_amount_label))
    {
        page_01_main_page_amount_label = find_obj_by_name("01_amount_label", page_01_main_obj, page_01_main_len);
    }
    if (page_01_main_page_amount_label || lv_obj_is_valid(page_01_main_page_amount_label))
    {
        format_amount_with_comma(amount_buf, sizeof(amount_buf), sim_data->total_amount);
        lv_label_set_text(page_01_main_page_amount_label, amount_buf);
    }

    //main_right_list
    //清空

    for (int i = 0; i < 10; i++)
    {
        int row = i + 1;
        char denom_buf[32], pcs_buf[32], amount_buf[32];

        snprintf(denom_buf, sizeof(denom_buf), "denom_%d_label", row);
        snprintf(pcs_buf, sizeof(pcs_buf), "pcs_%d_label", row);
        snprintf(amount_buf, sizeof(amount_buf), "amount_%d_label", row);

        lv_obj_t* denom = find_obj_by_name(denom_buf, page_01_main_obj, page_01_main_len);
        lv_obj_t* pcs = find_obj_by_name(pcs_buf, page_01_main_obj, page_01_main_len);
        lv_obj_t* amount = find_obj_by_name(amount_buf, page_01_main_obj, page_01_main_len);

        if (i < sim_data->denom_number  &&sim_data->denom[i].value) {
            update_label_by_name(page_01_main_obj, page_01_main_len, denom_buf, "%d", sim_data->denom[i].value);
            update_label_by_name(page_01_main_obj, page_01_main_len, pcs_buf, "%d", sim_data->denom[i].pcs);
            update_label_by_name(page_01_main_obj, page_01_main_len, amount_buf, "%.0f", sim_data->denom[i].amount);

            if (denom) lv_obj_clear_flag(denom, LV_OBJ_FLAG_HIDDEN);
            if (pcs) lv_obj_clear_flag(pcs, LV_OBJ_FLAG_HIDDEN);
            if (amount) lv_obj_clear_flag(amount, LV_OBJ_FLAG_HIDDEN);

        }
        else {
            if (denom) lv_obj_add_flag(denom, LV_OBJ_FLAG_HIDDEN);
            if (pcs) lv_obj_add_flag(pcs, LV_OBJ_FLAG_HIDDEN);
            if (amount) lv_obj_add_flag(amount, LV_OBJ_FLAG_HIDDEN);
        }

    }

   
    update_label_by_name(page_01_main_obj, page_01_main_len, "total_pcs_label", "%d", sim_data->total_pcs);

    char amount_total[32];
    format_amount_with_comma(amount_total, sizeof(amount_total), sim_data->total_amount);
    lv_label_set_text(find_obj_by_name("total_amount_label", page_01_main_obj, page_01_main_len), amount_total);//更新总金额格式

    if (page_01_main_scroll_container != NULL && lv_obj_is_valid(page_01_main_scroll_container))
        if (sim_data->denom_number > PAGE_01_REPORT_ITEM)
            lv_obj_add_flag(page_01_main_scroll_container, LV_OBJ_FLAG_SCROLLABLE);
        else
        {
            lv_obj_clear_flag(page_01_main_scroll_container, LV_OBJ_FLAG_SCROLLABLE);

        }
    page_01_err_num_refre();
}

void cleanup_counting_sim(void) {
    // 停止所有计时器
    if (sim_timer) {
        lv_timer_del(sim_timer);
        sim_timer = NULL;
    }

    if (safe_reset_timer) {
        lv_timer_del(safe_reset_timer);
        safe_reset_timer = NULL;
    }

    // 重置标签引用
    page_01_main_page_amount_label = NULL;
    page_01_main_page_pcs_label = NULL;
    page_01_main_scroll_container = NULL;

    memset(&sim, 0, sizeof(counting_sim_t));
}

void pause_counting_sim(void)
{
    if (!sim_timer) return;  
    
    counting_sim_t* sim_data = &sim;
    if (!sim_data->is_paused) {
        lv_timer_pause(sim_timer);  
        sim_data->is_paused = true;  
#if LV_DEBUG
        printf("计数模拟已暂停\n");
#endif
    }
}

void resume_counting_sim(void)
{
    if (!sim_timer) return;  
    
    counting_sim_t* sim_data = &sim;
    if (sim_data->is_paused) {
        lv_timer_resume(sim_timer); 
        sim_data->is_paused = false; 
#if LV_DEBUG
        printf("计数模拟已恢复\n");
#endif
    }
}

void save_counting_data(void) {
    if (sim_timer != NULL) {
        // 复制当前数据到保存区
        memcpy(&saved_sim_data, &sim, sizeof(counting_sim_t));
        has_saved_data = true;
#if LV_DEBUG
        printf("计数数据已保存，总数量: %d, 总金额: %.2f\n", 
               saved_sim_data.total_pcs, saved_sim_data.total_amount);
#endif
    }
}

// 恢复保存的计数数据
void restore_counting_data(void) {
    if (has_saved_data) {
        // 恢复保存的数据
        memcpy(&sim, &saved_sim_data, sizeof(counting_sim_t));
        has_saved_data = false;
#if LV_DEBUG
        printf("计数数据已恢复，总数量: %d, 总金额: %.2f\n", 
               sim.total_pcs, sim.total_amount);
#endif
    }
}


//切换mode
void mode_switch(void)
{
    

}

void page_02_report_init(void)
{
    page_02_a_report_status.curent_page = 1;
    page_02_a_report_status.total_page = sim.denom_number == 0 ? 1 : (sim.denom_number + PAGE_02_A_ITEM - 1) / PAGE_02_A_ITEM;
    page_02_b_report_status.curent_page = 1;
    page_02_b_report_status.total_page = (sim.total_pcs == 0) ? 1 : ((sim.total_pcs + PAGE_02_B_ITEM - 1) / PAGE_02_B_ITEM);
    page_02_c_report_status.curent_page = 1;
    page_02_c_report_status.total_page = 1;
}

// 清空所有冠字号的函数
void sim_clear_all_sn(counting_sim_t* sim_data)
{
    if (sim_data->sn_str != NULL)
    {
        for (int i = 0; i < sim_data->total_pcs; i++)
        {
            if (sim_data->sn_str[i] != NULL)
            {
                free(sim_data->sn_str[i]);
                sim_data->sn_str[i] = NULL;
            }
        }
        free(sim_data->sn_str);
        sim_data->sn_str = NULL;
    }

    // 重置所有计数
    for (int i = 0; i < sim_data->denom_number; i++)
    {
        sim_data->denom[i].pcs = 0;
        sim_data->denom[i].amount = 0;
    }

    sim_data->total_pcs = 0;
    sim_data->total_amount = 0;
    sim_data->err_num = 0;
    ui_refresh_main_page();
}
