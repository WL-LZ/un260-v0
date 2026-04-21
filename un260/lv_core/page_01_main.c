#include "un260/lv_core/page_01_main.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "lv_page_event.h"
#include <stdio.h>
#include "un260/lv_system/platform_app.h" 
#include <string.h>
#include "un260/lv_resources/lv_img_init.h" 
#include "un260/lv_refre/lvgl_refre.h"
#include "../aic_ui/aic_ui.h"
#include "un260/lv_components/smart_island.h"
#include "un260/lv_system/lv_str.h" 
#include "lv_page_declear.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_components/lv_print_toast.h"
#include "un260/lv_drivers/lv_drivers.h"
#include "un260/lv_system/machine_time.h"
#include "un260/lv_system/ui_text.h"
#include "un260/lv_system/ui_export_data.h"

// 添加数组元素计数变量
int page_01_main_len = 0;

//          "obj_name", obj_type, img_src,
//          { x, y, w, h, r, g, b },
//          { laebl, r, g, b, font,align },
//          { opacity, radius, border_width, use_custom_style },
//          event_cb, event_code, user_data, obj_ref ,btn_style}
//label 背景默认透明


static lv_obj_t* s_time_label = NULL;
static lv_timer_t* s_time_timer = NULL;
static bool s_main_init_protocol_sent = false;
static lv_obj_t* s_bottom_area_a = NULL;
static lv_obj_t* s_bottom_area_b = NULL;
static lv_obj_t* s_bottom_area_c = NULL;
static lv_obj_t* s_bottom_a_btn_mode = NULL;
static lv_obj_t* s_bottom_a_btn_add = NULL;
static lv_obj_t* s_bottom_a_btn_work = NULL;
static lv_obj_t* s_bottom_a_btn_fo = NULL;
static lv_obj_t* s_bottom_a_label_mode = NULL;
static lv_obj_t* s_bottom_a_label_add = NULL;
static lv_obj_t* s_bottom_a_label_work = NULL;
static lv_obj_t* s_bottom_a_label_fo = NULL;
static lv_obj_t* s_bottom_c_btn_batch = NULL;
static lv_obj_t* s_bottom_c_btn_speed = NULL;
static lv_obj_t* s_bottom_c_box_cfd = NULL;
static lv_obj_t* s_bottom_c_label_batch = NULL;
static lv_obj_t* s_bottom_c_label_speed = NULL;
static lv_obj_t* s_bottom_c_label_cfd = NULL;

static void page_01_smart_island_action_cb(uint8_t action_id)
{
    if (action_id == SMART_ISLAND_ACTION_TIME_SETTING) {
        ui_export_data_request();
    }
}

typedef enum {
    PAGE_01_BOTTOM_TEXT_ANIM_NONE = 0,
    PAGE_01_BOTTOM_TEXT_ANIM_FADE,
    PAGE_01_BOTTOM_TEXT_ANIM_SLIDE,
    PAGE_01_BOTTOM_TEXT_ANIM_ZOOM
} page_01_bottom_text_anim_t;

static void main_time_refresh(void)
{
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%02u:%02u:%02u",
        (unsigned)Machine_para.hour,
        (unsigned)Machine_para.minute,
        (unsigned)Machine_para.second);
    if (s_time_label) lv_label_set_text(s_time_label, buf);
}

static void main_time_timer_cb(lv_timer_t* t)
{
    (void)t;
    main_time_refresh();
    smart_island_refresh_time(); //刷新灵动岛默认时间
}

static void page_01_main_send_init_protocol(void) //主界面首次进入时发送初始化协议
{
    uint8_t sub = 0x02;

    if (s_main_init_protocol_sent) return;
    if (fd4 < 0) return;

    send_command(fd4, 0xC0, &sub, 1); //FD DF 06 C0 02：通知下位机进入采集误报数据模式

    /* 预留：后续新增主界面首次进入协议时，继续在这里统一发送。 */

    s_main_init_protocol_sent = true;
}

static void page_01_bottom_text_anim_opa_cb(void* var, int32_t v) //底部按钮文本透明度动画
{
    lv_obj_set_style_text_opa((lv_obj_t*)var, (lv_opa_t)v, 0);
}

static void page_01_bottom_text_anim_x_cb(void* var, int32_t v) //底部按钮文本横向位移动画
{
    lv_obj_set_style_translate_x((lv_obj_t*)var, (lv_coord_t)v, 0);
}

static void page_01_bottom_text_anim_zoom_cb(void* var, int32_t v) //底部按钮文本缩放动画
{
    lv_obj_set_style_transform_zoom((lv_obj_t*)var, (lv_coord_t)v, 0);
}

static void page_01_bottom_label_anim_run(lv_obj_t* label, const char* text,
    page_01_bottom_text_anim_t anim_type) //刷新底部按钮文本并执行轻量动画
{
    lv_anim_t a;
    const char* old_text;

    if (label == NULL || text == NULL) return;
    old_text = lv_label_get_text(label);

    if (old_text != NULL && strcmp(old_text, text) == 0) {
        return; //文本未变化时不重复做动画，避免闪烁
    }

    lv_anim_del(label, NULL);
    lv_label_set_text(label, text);

    lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    lv_obj_set_style_translate_x(label, 0, 0);
    lv_obj_set_style_transform_zoom(label, 256, 0);

    if (anim_type == PAGE_01_BOTTOM_TEXT_ANIM_NONE) {
        return;
    }

    lv_anim_init(&a);
    lv_anim_set_var(&a, label);
    lv_anim_set_time(&a, 160);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);

    if (anim_type == PAGE_01_BOTTOM_TEXT_ANIM_FADE) {
        lv_obj_set_style_text_opa(label, LV_OPA_60, 0);
        lv_anim_set_values(&a, LV_OPA_60, LV_OPA_COVER);
        lv_anim_set_exec_cb(&a, page_01_bottom_text_anim_opa_cb);
        lv_anim_start(&a);
        return;
    }

    if (anim_type == PAGE_01_BOTTOM_TEXT_ANIM_SLIDE) {
        lv_obj_set_style_text_opa(label, LV_OPA_70, 0);
        lv_obj_set_style_translate_x(label, 12, 0);

        lv_anim_set_values(&a, LV_OPA_70, LV_OPA_COVER);
        lv_anim_set_exec_cb(&a, page_01_bottom_text_anim_opa_cb);
        lv_anim_start(&a);

        lv_anim_init(&a);
        lv_anim_set_var(&a, label);
        lv_anim_set_time(&a, 160);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_set_values(&a, 12, 0);
        lv_anim_set_exec_cb(&a, page_01_bottom_text_anim_x_cb);
        lv_anim_start(&a);
        return;
    }

    if (anim_type == PAGE_01_BOTTOM_TEXT_ANIM_ZOOM) {
        lv_obj_set_style_text_opa(label, LV_OPA_70, 0);
        lv_obj_set_style_transform_zoom(label, 236, 0);

        lv_anim_set_values(&a, LV_OPA_70, LV_OPA_COVER);
        lv_anim_set_exec_cb(&a, page_01_bottom_text_anim_opa_cb);
        lv_anim_start(&a);

        lv_anim_init(&a);
        lv_anim_set_var(&a, label);
        lv_anim_set_time(&a, 160);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_set_values(&a, 236, 256);
        lv_anim_set_exec_cb(&a, page_01_bottom_text_anim_zoom_cb);
        lv_anim_start(&a);
    }
}

static const char* page_01_bottom_mode_text_get(uint8_t mode) //获取底部A区模式文本
{
    switch (mode) {
    case MODE_MDC: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_MODE_MDC);
    case MODE_SDC: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_MODE_SDC);
    case MODE_CNT: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_MODE_CNT);
    default: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_MODE_MDC);
    }
}

static const char* page_01_bottom_add_text_get(void) //获取底部A区ADD文本
{
    return Machine_para.add_enable ? ui_text_get(UI_TEXT_PAGE01_BOTTOM_ADD_ON) :
        ui_text_get(UI_TEXT_PAGE01_BOTTOM_ADD_OFF);
}

static const char* page_01_bottom_work_text_get(void) //获取底部A区工作模式文本
{
    return Machine_para.work_mode ? ui_text_get(UI_TEXT_PAGE01_BOTTOM_WORK_MANUAL) :
        ui_text_get(UI_TEXT_PAGE01_BOTTOM_WORK_AUTO);
}

static const char* page_01_bottom_fo_text_get(void) //获取底部A区F/O文本
{
    switch (Machine_para.fo_mode) {
    case 0: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_FO_OFF);
    case 1: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_FO_F);
    case 2: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_FO_O);
    case 3: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_FO_FO);
    default: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_FO_OFF);
    }
}

static const char* page_01_bottom_speed_text_get(void) //获取底部C区速度文本
{
    switch (Machine_para.speed) {
    case 0: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_SPEED_LOW);
    case 1: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_SPEED_MID);
    case 2: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_SPEED_HIGH);
    default: return ui_text_get(UI_TEXT_PAGE01_BOTTOM_SPEED_LOW);
    }
}

static lv_obj_t* page_01_bottom_btn_create(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
    lv_event_cb_t event_cb) //创建主界面底部A区按钮
{
    lv_obj_t* btn = lv_obj_create(main_page);

    lv_obj_remove_style_all(btn);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xF8F5F5), 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xCECECE), LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn, 32, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_outline_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);

    if (event_cb) {
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
    }

    return btn;
}

static lv_obj_t* page_01_bottom_btn_label_create(lv_obj_t* parent) //创建主界面底部A区按钮文本
{
    lv_obj_t* label = lv_label_create(parent);

    lv_obj_set_size(label, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_center(label);
    lv_obj_set_style_text_color(label, lv_color_hex(0x5D5D5D), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    return label;
}

static lv_obj_t* page_01_bottom_box_create(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
    lv_color_t bg_color) //创建主界面底部背景盒子
{
    lv_obj_t* obj = lv_obj_create(main_page);

    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(obj, bg_color, 0);
    lv_obj_set_style_radius(obj, 32, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_outline_width(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    return obj;
}

static void page_01_bottom_old_bar_hide(void) //隐藏旧底部信息栏对象，避免与新A区重叠
{
    const char* old_obj_name[] = {
        "mix_label", "add_label", "auto_label", "bacth_label", "bacth_num_label",
        "face_label", "cfd_label", "cfd_value_label", "speed_label", "speed_num_label",
        "page_01_collect_icon.png", "page_01_usb_icon.png"
    };
    lv_obj_t* obj;
    uint32_t i;

    for (i = 0; i < sizeof(old_obj_name) / sizeof(old_obj_name[0]); i++) {
        obj = find_obj_by_name(old_obj_name[i], page_01_main_obj, page_01_main_len);
        if (obj) {
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static lv_obj_t* page_01_bottom_c_btn_create(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
    lv_color_t bg_color, lv_event_cb_t event_cb, bool clickable) //创建主界面底部C区按钮
{
    lv_obj_t* btn = page_01_bottom_box_create(x, y, w, h, bg_color);

    if (clickable) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xCECECE), LV_STATE_PRESSED);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        if (event_cb) {
            lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);
        }
    } else {
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    }

    return btn;
}

static void page_01_bottom_a_create(void) //创建主界面底部A区四个按钮
{
    if (s_bottom_a_btn_mode || s_bottom_a_btn_add || s_bottom_a_btn_work || s_bottom_a_btn_fo) return;

    page_01_bottom_old_bar_hide();

    s_bottom_a_btn_mode = page_01_bottom_btn_create(18, 354, 85, 32, page_01_bottom_mode_btn_event_cb);
    s_bottom_a_btn_add = page_01_bottom_btn_create(109, 354, 95, 32, page_01_add_btn_event_cb);
    s_bottom_a_btn_work = page_01_bottom_btn_create(210, 354, 110, 32, page_01_work_btn_event_cb);
    s_bottom_a_btn_fo = page_01_bottom_btn_create(326, 354, 111, 32, page_01_fo_btn_event_cb);

    s_bottom_a_label_mode = page_01_bottom_btn_label_create(s_bottom_a_btn_mode);
    s_bottom_a_label_add = page_01_bottom_btn_label_create(s_bottom_a_btn_add);
    s_bottom_a_label_work = page_01_bottom_btn_label_create(s_bottom_a_btn_work);
    s_bottom_a_label_fo = page_01_bottom_btn_label_create(s_bottom_a_btn_fo);
}

static void page_01_bottom_a_destroy(void) //销毁主界面底部A区四个按钮
{
    if (s_bottom_a_btn_mode) lv_obj_del(s_bottom_a_btn_mode);
    if (s_bottom_a_btn_add) lv_obj_del(s_bottom_a_btn_add);
    if (s_bottom_a_btn_work) lv_obj_del(s_bottom_a_btn_work);
    if (s_bottom_a_btn_fo) lv_obj_del(s_bottom_a_btn_fo);

    s_bottom_a_btn_mode = NULL;
    s_bottom_a_btn_add = NULL;
    s_bottom_a_btn_work = NULL;
    s_bottom_a_btn_fo = NULL;
    s_bottom_a_label_mode = NULL;
    s_bottom_a_label_add = NULL;
    s_bottom_a_label_work = NULL;
    s_bottom_a_label_fo = NULL;
}

static void page_01_bottom_c_create(void) //创建主界面底部C区三个区域
{
    if (s_bottom_c_btn_batch || s_bottom_c_btn_speed || s_bottom_c_box_cfd) return;

    s_bottom_c_btn_batch = page_01_bottom_c_btn_create(805, 354, 209, 32,
        lv_color_hex(0xF8F5F5), page_01_bottom_batch_btn_event_cb, true);
    s_bottom_c_btn_speed = page_01_bottom_c_btn_create(1020, 354, 113, 32,
        lv_color_hex(0xF8F5F5), page_01_bottom_speed_btn_event_cb, true);
    s_bottom_c_box_cfd = page_01_bottom_c_btn_create(1139, 354, 104, 32,
        lv_color_hex(0xFFFFFF), NULL, false);

    s_bottom_c_label_batch = page_01_bottom_btn_label_create(s_bottom_c_btn_batch);
    s_bottom_c_label_speed = page_01_bottom_btn_label_create(s_bottom_c_btn_speed);
    s_bottom_c_label_cfd = page_01_bottom_btn_label_create(s_bottom_c_box_cfd);
}

static void page_01_bottom_c_destroy(void) //销毁主界面底部C区三个区域
{
    if (s_bottom_c_btn_batch) lv_obj_del(s_bottom_c_btn_batch);
    if (s_bottom_c_btn_speed) lv_obj_del(s_bottom_c_btn_speed);
    if (s_bottom_c_box_cfd) lv_obj_del(s_bottom_c_box_cfd);

    s_bottom_c_btn_batch = NULL;
    s_bottom_c_btn_speed = NULL;
    s_bottom_c_box_cfd = NULL;
    s_bottom_c_label_batch = NULL;
    s_bottom_c_label_speed = NULL;
    s_bottom_c_label_cfd = NULL;
}

static lv_obj_t* page_01_bottom_bg_create(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
    lv_color_t bg_color) //创建主界面底部背景块
{
    lv_obj_t* obj = lv_obj_create(main_page);

    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(obj, bg_color, 0);
    lv_obj_set_style_radius(obj, 40, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_outline_width(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    return obj;
}

static void page_01_bottom_bg_create_all(void) //创建主界面底部三个背景区域
{
    if (s_bottom_area_a || s_bottom_area_b || s_bottom_area_c) return;

    s_bottom_area_a = page_01_bottom_bg_create(10, 348, 435, 44, lv_color_make(255, 255, 255));
    s_bottom_area_b = page_01_bottom_bg_create(492, 348, 261, 44, lv_color_hex(0x656565));
    s_bottom_area_c = page_01_bottom_bg_create(799, 348, 450, 44, lv_color_make(255, 255, 255));
}

static void page_01_bottom_bg_destroy_all(void) //销毁主界面底部三个背景区域
{
    if (s_bottom_area_a) lv_obj_del(s_bottom_area_a);
    if (s_bottom_area_b) lv_obj_del(s_bottom_area_b);
    if (s_bottom_area_c) lv_obj_del(s_bottom_area_c);

    s_bottom_area_a = NULL;
    s_bottom_area_b = NULL;
    s_bottom_area_c = NULL;
}

void page_01_bottom_a_refresh_mode(bool anim_en) //刷新主界面底部A区模式文本
{
    page_01_bottom_label_anim_run(s_bottom_a_label_mode, page_01_bottom_mode_text_get(Machine_para.mode),
        anim_en ? PAGE_01_BOTTOM_TEXT_ANIM_SLIDE : PAGE_01_BOTTOM_TEXT_ANIM_NONE);
}

void page_01_bottom_a_refresh_mode_preview(uint8_t mode) //预刷新主界面底部A区模式文本
{
    page_01_bottom_label_anim_run(s_bottom_a_label_mode, page_01_bottom_mode_text_get(mode),
        PAGE_01_BOTTOM_TEXT_ANIM_SLIDE);
}

void page_01_bottom_a_refresh_add(bool anim_en) //刷新主界面底部A区ADD文本
{
    page_01_bottom_label_anim_run(s_bottom_a_label_add, page_01_bottom_add_text_get(),
        anim_en ? PAGE_01_BOTTOM_TEXT_ANIM_SLIDE : PAGE_01_BOTTOM_TEXT_ANIM_NONE);
}

void page_01_bottom_a_refresh_work(bool anim_en) //刷新主界面底部A区工作模式文本
{
    page_01_bottom_label_anim_run(s_bottom_a_label_work, page_01_bottom_work_text_get(),
        anim_en ? PAGE_01_BOTTOM_TEXT_ANIM_SLIDE : PAGE_01_BOTTOM_TEXT_ANIM_NONE);
}

void page_01_bottom_a_refresh_fo(bool anim_en) //刷新主界面底部A区F/O文本
{
    page_01_bottom_label_anim_run(s_bottom_a_label_fo, page_01_bottom_fo_text_get(),
        anim_en ? PAGE_01_BOTTOM_TEXT_ANIM_SLIDE : PAGE_01_BOTTOM_TEXT_ANIM_NONE);
}

void page_01_bottom_c_refresh_batch(bool anim_en) //刷新主界面底部C区Batch文本
{
    char text_buf[32];

    if (Machine_para.batch_switch_enable) {
        lv_snprintf(text_buf, sizeof(text_buf), ui_text_get(UI_TEXT_PAGE01_BOTTOM_BATCH_VALUE_FMT),
            Machine_para.batch_num);
    } else {
        lv_snprintf(text_buf, sizeof(text_buf), "%s", ui_text_get(UI_TEXT_PAGE01_BOTTOM_BATCH_OFF));
    }

    page_01_bottom_label_anim_run(s_bottom_c_label_batch, text_buf,
        anim_en ? PAGE_01_BOTTOM_TEXT_ANIM_NONE : PAGE_01_BOTTOM_TEXT_ANIM_NONE);
}

void page_01_bottom_c_refresh_speed(bool anim_en) //刷新主界面底部C区速度文本
{
    page_01_bottom_label_anim_run(s_bottom_c_label_speed, page_01_bottom_speed_text_get(),
        anim_en ? PAGE_01_BOTTOM_TEXT_ANIM_SLIDE : PAGE_01_BOTTOM_TEXT_ANIM_NONE);
}

void page_01_bottom_c_refresh_cfd(void) //刷新主界面底部C区CFD文本
{
    char text_buf[24];

    lv_snprintf(text_buf, sizeof(text_buf), ui_text_get(UI_TEXT_PAGE01_BOTTOM_CFD_FMT),
        "L");
    page_01_bottom_label_anim_run(s_bottom_c_label_cfd, text_buf, PAGE_01_BOTTOM_TEXT_ANIM_NONE);
}

ui_element_t page_01_main_obj[] = {
    //////////////////////////////////////////////////////
  //***************    BG_IMG_LIST  *******************//
////////////////////////////////////////////////////////

    { "page_01_back.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 0, 0, 1280, 400, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

        //////////////////////////////////////////////////////
    //***************    BTN_LIST   *********************//
    //////////////////////////////////////////////////////

        { "menu_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 1125, 18, 119, 92, 244, 244, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_menu_btn_event_cb, 0, NULL , NULL ,UI_BTN_STYLE_APPLE},

        { "start_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 1125, 128, 119, 92, 244, 244, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_start_btn_event_cb , LV_EVENT_CLICKED, NULL, NULL ,UI_BTN_STYLE_APPLE},

        { "esc_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 1125, 238, 119, 92, 244, 244, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_esc_btn_event_cb, LV_EVENT_CLICKED, NULL, NULL ,UI_BTN_STYLE_APPLE},

        { "mode_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 15, 36, 75, 66, 255, 255, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_mode_btn_event_cb, LV_EVENT_CLICKED, NULL, NULL ,UI_BTN_STYLE_PRESS_FEEL },

        { "setting_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 15, 109, 75, 60, 255, 255, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_set_btn_event_cb, LV_EVENT_CLICKED, NULL, NULL ,UI_BTN_STYLE_PRESS_FEEL },

        { "list_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 15, 176, 75, 60, 255, 255, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_list_btn_event_cb, 0, (void*)(uintptr_t)UI_PAGE_LIST, NULL ,
            UI_BTN_STYLE_PRESS_FEEL},
        { "print_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 15, 243, 75, 73, 255, 255, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 18, 0, true },
            page_01_print_btn_event_cb, 0, NULL, NULL ,UI_BTN_STYLE_PRESS_FEEL },

        { "curr_img_btn", LV_OBJ_TYPE_BUTTON, NULL,
            { 159, 33, 184, 100, 255, 255, 255 },
            { NULL, 0, 0, 0, NULL },
            { 255, 7, 0, true },
            page_01_curr_btn_event_cb, 0, NULL, NULL ,UI_BTN_STYLE_PRESS_FEEL },
  //////////////////////////////////////////////////////
 //***************  IMAGE_LIST **********************//
//////////////////////////////////////////////////////

    { "page_01_collect_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1025, 353, 29, 30, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "curr_USD_img", LV_OBJ_TYPE_IMAGE, &page_01_curr_USD,
        { 156, 31, 182, 103, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_esc_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1167, 254, 38, 38, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_list_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 31, 184, 39, 24, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_menu_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1167, 31, 35, 35, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_mode_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 30, 43, 42, 34, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_print_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 32, 249, 38, 35, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_set_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 33, 113, 35, 35, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_start_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 1167, 142, 41, 35, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "page_01_usb_icon.png", LV_OBJ_TYPE_IMAGE, NULL,
        { 985, 353, 22, 30, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
  //////////////////////////////////////////////////////
 //***************  LABEL_LIST **********************//
//////////////////////////////////////////////////////
    { "01_pcs_label", LV_OBJ_TYPE_LABEL, NULL,
        { 280, 144, 350, 40, 0, 0, 0 },
        { "0", 202, 23, 0, &lv_font_montserrat_44, LV_TEXT_ALIGN_RIGHT },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
    { "01_amount_label", LV_OBJ_TYPE_LABEL, NULL,
        { 280, 226, 350, 40, 0, 0, 0 },
        { "0", 202, 23, 0, &lv_font_montserrat_48, LV_TEXT_ALIGN_RIGHT },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
    { "reject_label", LV_OBJ_TYPE_LABEL, NULL,
        { 382, 67, 110, 27, 0, 0, 0 },
        { "REJECT:", 202, 23, 0, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "mode_label", LV_OBJ_TYPE_LABEL, NULL,
        { 10, 79, 81, 17, 0, 0, 0 },
        { "MODE", 119, 119, 119, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "setting_label", LV_OBJ_TYPE_LABEL, NULL,
        { 10, 151, 81, 17, 0, 0, 0 },
        { "SETTING", 119, 119, 119, &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "list_label", LV_OBJ_TYPE_LABEL, NULL,
        { 10, 215, 81, 17, 0, 0, 0 },
        { "LIST", 119, 119, 119, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "print_label", LV_OBJ_TYPE_LABEL, NULL,
        { 10, 292, 81, 17, 0, 0, 0 },
        { "PRINT", 119, 119, 119, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "mix_label", LV_OBJ_TYPE_LABEL, NULL,
        { 12, 357, 96, 17, 0, 0, 0 },
        { "MIX", 93, 93, 93, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "add_label", LV_OBJ_TYPE_LABEL, NULL,
        { 110, 357, 105, 17, 0, 0, 0 },
        { "ADD", 93, 93, 93, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "auto_label", LV_OBJ_TYPE_LABEL, NULL,
        { 217, 357, 122, 17, 0, 0, 0 },
        { "AUTO", 93, 93, 93, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "bacth_label", LV_OBJ_TYPE_LABEL, NULL,
        { 341, 357, 89, 17, 0, 0, 0 },
        { "BATCH:", 93, 93, 93, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "bacth_num_label", LV_OBJ_TYPE_LABEL, NULL,
        { 434, 357, 128, 17, 0, 0, 0 },
        { "10000", 93, 93, 93, &lv_font_montserrat_18, LV_TEXT_ALIGN_LEFT },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "face_label", LV_OBJ_TYPE_LABEL, NULL,
        { 566, 357, 111, 17, 0, 0, 0 },
        { "FACE", 93, 93, 93, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "cfd_label", LV_OBJ_TYPE_LABEL, NULL,
        { 698, 357, 50, 17, 0, 0, 0 },
        { "CFD:", 93, 93, 93, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "cfd_value_label", LV_OBJ_TYPE_LABEL, NULL,
        { 748, 357, 20, 17, 0, 0, 0 },
        { "H", 93, 93, 93, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "speed_label", LV_OBJ_TYPE_LABEL, NULL,
        { 827, 357, 33, 17, 0, 0, 0 },
        { "SP:", 93, 93, 93, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "speed_num_label", LV_OBJ_TYPE_LABEL, NULL,
        { 862, 357, 54, 17, 0, 0, 0 },
        { "1000", 93, 93, 93, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "pcs_label", LV_OBJ_TYPE_LABEL, NULL,
        { 145, 175, 54, 23, 0, 0, 0 },
        { "PCS", 0, 115, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "reject_num_label", LV_OBJ_TYPE_LABEL, NULL,
        { 484, 67, 81, 27, 0, 0, 0 },
        { "0", 202, 23, 0, &lv_font_montserrat_20, LV_TEXT_ALIGN_RIGHT },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "curr_icon_label", LV_OBJ_TYPE_LABEL, NULL,
        { 145, 254, 60, 27, 0, 0, 0 },
        { "USD", 0, 115, 255, &lv_font_montserrat_24, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "list_demo_label", LV_OBJ_TYPE_LABEL, NULL,
        { 728, 24, 70, 23, 0, 0, 0 },
        { "DENOM", 121, 150, 0, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "list_pcs_label", LV_OBJ_TYPE_LABEL, NULL,
        { 826, 24, 54, 23, 0, 0, 0 },
        { "PCS", 121, 150, 0, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },

        NULL, 0, NULL, NULL },

    { "list_amount_label", LV_OBJ_TYPE_LABEL, NULL,
        { 933, 24, 80, 23, 0, 0, 0 },
        { "AMOUNT", 121, 150, 0, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "menu_btn_label", LV_OBJ_TYPE_LABEL, NULL,
        { 1144, 77, 80, 23, 0, 0, 0 },
        { "MENU", 119, 119, 119, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "start_btn_label", LV_OBJ_TYPE_LABEL, NULL,
        { 1144, 188, 80, 23, 0, 0, 0 },
        { "START", 119, 119, 119, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },

    { "esc_btn_label", LV_OBJ_TYPE_LABEL, NULL,
        { 1144, 298, 80, 23, 0, 0, 0 },
        { "ESC", 119, 119, 119, &lv_font_montserrat_20, LV_TEXT_ALIGN_CENTER },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },
//list_label
        { "denom_1_label", LV_OBJ_TYPE_LABEL, NULL,
            { 714, 54, 77, 23, 0, 0, 0 },
            { "---", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_1_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 54, 54, 23, 0, 0, 0 },
            { "0", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_1_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 54, 80, 23, 0, 0, 0 },
            { "0.00", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_2_label", LV_OBJ_TYPE_LABEL, NULL,
            { 714, 84, 77, 23, 0, 0, 0 },
            { "---", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_2_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 84, 54, 23, 0, 0, 0 },
            { "0", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_2_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 84, 80, 23, 0, 0, 0 },
            { "0.00", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_3_label", LV_OBJ_TYPE_LABEL, NULL,
            { 0, 114, 77, 23, 0, 0, 0 },
            { "---", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_3_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 114, 54, 23, 0, 0, 0 },
            { "0", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_3_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 114, 80, 23, 0, 0, 0 },
            { "0.00", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_4_label", LV_OBJ_TYPE_LABEL, NULL,
            { 714, 144, 77, 23, 0, 0, 0 },
            { "---", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_4_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 144, 54, 23, 0, 0, 0 },
            { "0", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_4_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 144, 80, 23, 0, 0, 0 },
            { "0.00", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_5_label", LV_OBJ_TYPE_LABEL, NULL,
            { 714, 174, 77, 23, 0, 0, 0 },
            { "---", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_5_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 174, 54, 23, 0, 0, 0 },
            { "0", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_5_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 174, 80, 23, 0, 0, 0 },
            { "0.00", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_6_label", LV_OBJ_TYPE_LABEL, NULL,
            { 714, 204, 77, 23, 0, 0, 0 },
            { "---", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_6_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 204, 54, 23, 0, 0, 0 },
            { "0", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_6_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 204, 80, 23, 0, 0, 0 },
            { "0.00", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_7_label", LV_OBJ_TYPE_LABEL, NULL,
            { 714, 234, 77, 23, 0, 0, 0 },
            { "---", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_7_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 234, 54, 23, 0, 0, 0 },
            { "0", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_7_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 234, 80, 23, 0, 0, 0 },
            { "0.00", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_8_label", LV_OBJ_TYPE_LABEL, NULL,
            { 714, 264, 77, 23, 0, 0, 0 },
            { "---", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_8_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 264, 54, 23, 0, 0, 0 },
            { "0", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_8_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 264, 80, 23, 0, 0, 0 },
            { "0.00", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_9_label", LV_OBJ_TYPE_LABEL, NULL,
            { 714, 294, 77, 23, 0, 0, 0 },
            { "---", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_9_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 294, 54, 23, 0, 0, 0 },
            { "0", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_9_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 294, 80, 23, 0, 0, 0 },
            { "0.00", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "denom_10_label", LV_OBJ_TYPE_LABEL, NULL,
            { 714, 324, 77, 23, 0, 0, 0 },
            { "---", 102, 102, 102, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "pcs_10_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 324, 54, 23, 0, 0, 0 },
            { "0", 102, 102, 102, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "amount_10_label", LV_OBJ_TYPE_LABEL, NULL,
            { 0, 324, 80, 23, 0, 0, 0 },
            { "0.00", 102, 102, 102, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "total_label", LV_OBJ_TYPE_LABEL, NULL,
            { 728, 305, 54, 23, 0, 0, 0 },
            { "TOTAL", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "total_pcs_label", LV_OBJ_TYPE_LABEL, NULL,
            { 826, 305, 54, 23, 0, 0, 0 },
            { "0", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

        { "total_amount_label", LV_OBJ_TYPE_LABEL, NULL,
            { 933, 305, 80, 23, 0, 0, 0 },
            { "0.00", 93, 93, 93, &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER },
            { 255, 0, 0, false },
            NULL, 0, NULL, NULL },

};

// 添加数组元素计数变量
void ui_main_create(lv_obj_t* parent)
{
    //creat page_main 
    if (main_page) return;
    main_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(main_page);
    lv_obj_set_pos(main_page, 0, 0);
    lv_obj_set_size(main_page, 1280, 400);
    lv_obj_clear_flag(main_page, LV_OBJ_FLAG_SCROLLABLE); 
    lv_obj_set_scrollbar_mode(main_page, LV_SCROLLBAR_MODE_OFF); // 滚动条关闭
    static bool frist_creat = true;

    // 计算数组长度
    page_01_main_len = sizeof(page_01_main_obj) / sizeof(ui_element_t);
    
    //创建图片和其他UI元素
    lv_ui_obj_init(main_page, page_01_main_obj, page_01_main_len);
    

    if (frist_creat)
    {
        /* Main page denomination data is master-driven (0x0B), no local fallback here. */
        page_01_mode_switch_refre();

        frist_creat = false;
    }
    // 创建滚动容器并将标签放入容器
   // sim_data_init();           // 初始化面额列表、count=0、amount=0
    machine_time_init();
    ui_refresh_main_page();    // 把面额+"0"+"0.00"写到每一行
    page_01_create_mian_scrollable_container();
    page_01_add_refre();
    page_01_work_refre();
    page_01_batch_refre();
    page_01_face_refre();
    page_01_cfd_refre();
    page_01_speed_refre();
    page_01_err_num_refre();
    page_01_curr_img_refre();
    page_01_bottom_bg_create_all();
    page_01_bottom_a_create();
    page_01_bottom_c_create();
    page_01_bottom_a_refresh_mode(false);
    page_01_bottom_a_refresh_add(false);
    page_01_bottom_a_refresh_work(false);
    page_01_bottom_a_refresh_fo(false);
    page_01_bottom_c_refresh_batch(false);
    page_01_bottom_c_refresh_speed(false);
    page_01_bottom_c_refresh_cfd();
    s_time_label = NULL;
    main_time_refresh();
    if (!s_time_timer) s_time_timer = lv_timer_create(main_time_timer_cb, 1000, NULL);
    lv_print_toast_create();
    ui_state_apply_common_runtime();
    page_01_main_send_init_protocol();
    smart_island_create(main_page); //创建主界面B区灵动岛
    smart_island_register_action_cb(page_01_smart_island_action_cb);
    smart_island_refresh_time(); //初始化时间显示

}

void ui_main_destroy(void)
{
    if (main_page) {
        // 安全清理所有资源和定时器
        cleanup_counting_sim();
        page_01_bottom_a_destroy();
        page_01_bottom_c_destroy();
        page_01_bottom_bg_destroy_all();
        smart_island_destroy(); //销毁灵动岛
        lv_obj_del(main_page);
        main_page = NULL;
    }
}

//// 更新页面上的所有多语言文本
void page_01_update_language_texts(void) //刷新主界面多语言文本
{
    if (!main_page) return;

    page_01_bottom_a_refresh_mode(false);
    page_01_bottom_a_refresh_add(false);
    page_01_bottom_a_refresh_work(false);
    page_01_bottom_a_refresh_fo(false);
    page_01_bottom_c_refresh_batch(false);
    page_01_bottom_c_refresh_speed(false);
    page_01_bottom_c_refresh_cfd();
    smart_island_refresh_language_texts();
}
