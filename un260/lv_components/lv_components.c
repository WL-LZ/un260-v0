#include "lv_components.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_refre/lvgl_refre.h"


typedef struct {
    lv_obj_t* switch_container;
    lv_obj_t* switch_knob;
    lv_obj_t* label_on;
    lv_obj_t* label_off;
} batch_switch_t;

static batch_switch_t batch_switch = {
    .switch_container = NULL,
    .switch_knob = NULL,
    .label_on = NULL,
    .label_off = NULL,
};

void set_batch_switch_state(bool enable);

static void update_switch_visual(bool enable, bool animate) {
    lv_coord_t cont_w = lv_obj_get_width(batch_switch.switch_container);
    lv_coord_t knob_w = lv_obj_get_width(batch_switch.switch_knob);

    if (enable) {
        //  ON 标签
        lv_obj_set_style_bg_color(batch_switch.switch_container, lv_palette_main(LV_PALETTE_GREEN), 0);

        lv_label_set_text(batch_switch.label_on, "ON");
        lv_obj_align(batch_switch.label_on, LV_ALIGN_LEFT_MID, 3, 0);
        lv_obj_clear_flag(batch_switch.label_on, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(batch_switch.label_off, LV_OBJ_FLAG_HIDDEN);

        // 滑块动画
        lv_coord_t target_x = cont_w - knob_w - 8;
        if (animate && lv_obj_get_x(batch_switch.switch_knob) != target_x) {
            // 执行动画
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, batch_switch.switch_knob);
            lv_anim_set_values(&a, lv_obj_get_x(batch_switch.switch_knob), target_x);
            lv_anim_set_time(&a, 250);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_start(&a);
        }
        else {
            lv_obj_set_x(batch_switch.switch_knob, target_x);
        }

    }
    else {
        // OFF 标签
        lv_obj_set_style_bg_color(batch_switch.switch_container, lv_palette_main(LV_PALETTE_GREY), 0);

        lv_label_set_text(batch_switch.label_off, "OFF");
        lv_obj_align(batch_switch.label_off, LV_ALIGN_RIGHT_MID, -3, 0);
        lv_obj_clear_flag(batch_switch.label_off, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_flag(batch_switch.label_on, LV_OBJ_FLAG_HIDDEN);

        lv_coord_t target_x = 4;
        if (animate && lv_obj_get_x(batch_switch.switch_knob) != target_x) {
            // 执行动画
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, batch_switch.switch_knob);
            lv_anim_set_values(&a, lv_obj_get_x(batch_switch.switch_knob), target_x);
            lv_anim_set_time(&a, 250);
            lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
            lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_start(&a);
        }
        else {
            // 直接设置位置，无动画
            lv_obj_set_x(batch_switch.switch_knob, target_x);
        }
    }
    page_03_batch_mode_status_refre();
#if LV_DEBUG
    printf("batch_switch_status: %s\n", Machine_para.batch_switch_enable ? "ON" : "OFF");
#endif // LV_DEBUG

}

// 点击切换状态
static void switch_event_cb(lv_event_t* e) {
    LV_UNUSED(e);
    Machine_para.batch_switch_enable = !Machine_para.batch_switch_enable;
    update_switch_visual(Machine_para.batch_switch_enable, true);
}

// 创建批次开关组件
void create_batch_num_switch(lv_obj_t* parent) {
    // 自适应宽度
    lv_obj_t* tmp = lv_label_create(parent);
    lv_obj_set_style_text_font(tmp, &lv_font_montserrat_24, 0);
    lv_label_set_text(tmp, "OFF");
    lv_obj_update_layout(tmp);
    lv_coord_t txt_w = lv_obj_get_width(tmp);
    lv_obj_del(tmp);

    // 创建容器
    batch_switch.switch_container = lv_obj_create(parent);
    lv_obj_set_size(batch_switch.switch_container, txt_w + 44, 40);
    lv_obj_set_style_radius(batch_switch.switch_container, 20, 0);
    lv_obj_set_style_pad_all(batch_switch.switch_container, 0, 0);
    lv_obj_set_style_bg_color(batch_switch.switch_container,
        Machine_para.batch_switch_enable ?
        lv_palette_main(LV_PALETTE_GREEN) :
        lv_palette_main(LV_PALETTE_GREY), 0);

    lv_obj_add_flag(batch_switch.switch_container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(batch_switch.switch_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(batch_switch.switch_container, switch_event_cb, LV_EVENT_CLICKED, NULL);

    // 创建 OFF 标签
    batch_switch.label_off = lv_label_create(batch_switch.switch_container);
    lv_label_set_text(batch_switch.label_off, "OFF");
    lv_obj_set_style_text_font(batch_switch.label_off, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(batch_switch.label_off, lv_color_white(), 0);

    // 创建 ON 标签
    batch_switch.label_on = lv_label_create(batch_switch.switch_container);
    lv_label_set_text(batch_switch.label_on, "ON");
    lv_obj_set_style_text_font(batch_switch.label_on, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(batch_switch.label_on, lv_color_white(), 0);

    // 创建滑块
    batch_switch.switch_knob = lv_obj_create(batch_switch.switch_container);
    lv_obj_set_size(batch_switch.switch_knob, 30, 30);
    lv_obj_set_style_radius(batch_switch.switch_knob, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(batch_switch.switch_knob, lv_color_white(), 0);
    lv_obj_set_style_shadow_width(batch_switch.switch_knob, 5, 0);
    lv_obj_set_style_shadow_color(batch_switch.switch_knob, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(batch_switch.switch_knob, LV_OPA_20, 0);
    lv_obj_add_flag(batch_switch.switch_knob, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(batch_switch.switch_knob, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(batch_switch.switch_knob, switch_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_update_layout(batch_switch.switch_container);
    lv_obj_update_layout(batch_switch.switch_knob);

    // 居中Y
    lv_obj_set_y(batch_switch.switch_knob,
        (lv_obj_get_height(batch_switch.switch_container) - lv_obj_get_height(batch_switch.switch_knob)) / 2 - 2
    );

    // 设置初始状态，无动画
    update_switch_visual(Machine_para.batch_switch_enable, false);
}

// 外部调用：获取容器
lv_obj_t* get_batch_switch_container(void) {
    return batch_switch.switch_container;
}

//设置开关状态（自动更新UI，无动画）
void set_batch_switch_state(bool enable) {
    // 外部调用时不执行动画
    update_switch_visual(enable, false);
}


static void zoom_anim_cb(void* var, int32_t zoom)
{
    lv_img_set_zoom((lv_obj_t*)var,zoom);
}

void icon_feedback_comp(const char* name,ui_element_t* page_cfg_obj,int len)
{
    lv_obj_t* img = find_obj_by_name(name,page_cfg_obj,len);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a,img);
    lv_anim_set_exec_cb(&a, zoom_anim_cb);
    lv_anim_set_values(&a,256,285);
    lv_anim_set_time(&a, 100);
    lv_anim_set_playback_time(&a, 100);  // 回缩动画时间
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}
