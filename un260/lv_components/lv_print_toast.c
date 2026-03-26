#include "lv_print_toast.h"
#include <string.h>

// 打印提示框参数
#define PRINT_TOAST_X           502
#define PRINT_TOAST_Y           51
#define PRINT_TOAST_W           277
#define PRINT_TOAST_H           101

// 动画参数
#define PRINT_TOAST_SHOW_TIME   520
#define PRINT_TOAST_HIDE_TIME   500
#define PRINT_TOAST_START_Y     (-28)
#define PRINT_TOAST_HIDE_Y      5
#define PRINT_TOAST_ZOOM_START  242
#define PRINT_TOAST_ZOOM_END    256

// 全局对象
static lv_obj_t *g_print_toast = NULL;
static lv_obj_t *g_print_toast_label = NULL;
static bool g_print_toast_visible = false;
static bool g_print_toast_hiding = false;
static uint32_t g_print_toast_show_tick = 0;

// Y 动画回调
static void print_toast_anim_y_cb(void *var, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)var, (lv_coord_t)v);
}
static void print_toast_anim_zoom_cb(void *var, int32_t v)
{
    lv_obj_set_style_transform_zoom((lv_obj_t *)var, v, 0);
}

static void print_toast_anim_bg_opa_cb(void *var, int32_t v)
{
    lv_obj_set_style_bg_opa((lv_obj_t *)var, (lv_opa_t)v, 0);
    lv_obj_set_style_shadow_opa((lv_obj_t *)var, (lv_opa_t)((v * LV_OPA_20) / LV_OPA_COVER), 0);
}

static void print_toast_anim_text_opa_cb(void *var, int32_t v)
{
    lv_obj_set_style_text_opa((lv_obj_t *)var, (lv_opa_t)v, 0);
}

// 隐藏动画结束回调
static void print_toast_hide_ready_cb(lv_anim_t *a)
{
    LV_UNUSED(a);

    if (g_print_toast && lv_obj_is_valid(g_print_toast)) {
        lv_obj_add_flag(g_print_toast, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(g_print_toast, PRINT_TOAST_X, PRINT_TOAST_Y);
        lv_obj_set_style_bg_opa(g_print_toast, LV_OPA_0, 0);
        lv_obj_set_style_shadow_opa(g_print_toast, LV_OPA_0, 0);
        lv_obj_set_style_transform_zoom(g_print_toast, PRINT_TOAST_ZOOM_END, 0);
        if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
            lv_obj_set_style_text_opa(g_print_toast_label, LV_OPA_0, 0);
        }
    }
    g_print_toast_visible = false;
    g_print_toast_hiding = false;
}
void lv_print_toast_create(void)
{
    if (g_print_toast && lv_obj_is_valid(g_print_toast)) {
        return;
    }

    // 挂到 top layer，上层页面切换不影响
    g_print_toast = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(g_print_toast);

    lv_obj_set_pos(g_print_toast, PRINT_TOAST_X, PRINT_TOAST_Y);
    lv_obj_set_size(g_print_toast, PRINT_TOAST_W, PRINT_TOAST_H);

    lv_obj_clear_flag(g_print_toast, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(g_print_toast, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_print_toast, LV_OBJ_FLAG_CLICKABLE);

    // 背景
    lv_obj_set_style_bg_color(g_print_toast, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(g_print_toast, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(g_print_toast, 22, 0);
    lv_obj_set_style_border_width(g_print_toast, 0, 0);

    // 阴影：20% 黑色，右移 2，扩散 10
    lv_obj_set_style_shadow_color(g_print_toast, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(g_print_toast, LV_OPA_20, 0);
    lv_obj_set_style_shadow_width(g_print_toast, 10, 0);
    lv_obj_set_style_shadow_spread(g_print_toast, 0, 0);
    lv_obj_set_style_shadow_ofs_x(g_print_toast, 2, 0);
    lv_obj_set_style_shadow_ofs_y(g_print_toast, 4, 0);

    // 默认透明，等 show 动画淡入
    lv_obj_set_style_bg_opa(g_print_toast, LV_OPA_0, 0);
    lv_obj_set_style_shadow_opa(g_print_toast, LV_OPA_0, 0);

    // 标签
    g_print_toast_label = lv_label_create(g_print_toast);
    lv_label_set_text(g_print_toast_label, "Printing...");
    lv_obj_set_width(g_print_toast_label, PRINT_TOAST_W - 20);
    lv_obj_center(g_print_toast_label);

    lv_obj_set_style_text_align(g_print_toast_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(g_print_toast_label, lv_color_hex(0x2F3542), 0);
    lv_obj_set_style_text_font(g_print_toast_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_opa(g_print_toast_label, LV_OPA_0, 0);
    lv_obj_set_style_transform_zoom(g_print_toast, 256, 0);
    lv_obj_set_style_transform_pivot_x(g_print_toast, PRINT_TOAST_W / 2, 0);
    lv_obj_set_style_transform_pivot_y(g_print_toast, 0, 0);
}

void lv_print_toast_show(const char *text)
{
    if (!g_print_toast || !lv_obj_is_valid(g_print_toast)) {
        lv_print_toast_create();
    }

    if (text && g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_label_set_text(g_print_toast_label, text);
        lv_obj_center(g_print_toast_label);
    }

    if (g_print_toast_visible && !g_print_toast_hiding &&
        lv_tick_elaps(g_print_toast_show_tick) < 800) {
        return;
    }

    if (g_print_toast_visible && !g_print_toast_hiding &&
        !lv_obj_has_flag(g_print_toast, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }

    lv_anim_del(g_print_toast, print_toast_anim_y_cb);
    lv_anim_del(g_print_toast, print_toast_anim_zoom_cb);
    lv_anim_del(g_print_toast, print_toast_anim_bg_opa_cb);
    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_anim_del(g_print_toast_label, print_toast_anim_text_opa_cb);
    }

    lv_obj_clear_flag(g_print_toast, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(g_print_toast, PRINT_TOAST_X, PRINT_TOAST_START_Y);
    lv_obj_set_style_bg_opa(g_print_toast, LV_OPA_0, 0);
    lv_obj_set_style_shadow_opa(g_print_toast, LV_OPA_0, 0);
    lv_obj_set_style_transform_zoom(g_print_toast, PRINT_TOAST_ZOOM_START, 0);
    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_obj_set_style_text_opa(g_print_toast_label, LV_OPA_0, 0);
    }
    g_print_toast_visible = true;
    g_print_toast_hiding = false;
    g_print_toast_show_tick = lv_tick_get();

    // 单段轻阻尼下落，做出类似手机消息的弹出感
    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, g_print_toast);
    lv_anim_set_exec_cb(&a1, print_toast_anim_y_cb);
    lv_anim_set_values(&a1, PRINT_TOAST_START_Y, PRINT_TOAST_Y);
    lv_anim_set_time(&a1, 420);
    lv_anim_set_path_cb(&a1, lv_anim_path_overshoot);
    lv_anim_start(&a1);

    // 淡入
    lv_anim_t a3;
    lv_anim_init(&a3);
    lv_anim_set_var(&a3, g_print_toast);
    lv_anim_set_exec_cb(&a3, print_toast_anim_bg_opa_cb);
    lv_anim_set_values(&a3, LV_OPA_0, LV_OPA_COVER);
    lv_anim_set_time(&a3, 180);
    lv_anim_set_path_cb(&a3, lv_anim_path_ease_out);
    lv_anim_start(&a3);

    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_anim_t a5;
    lv_anim_init(&a5);
    lv_anim_set_var(&a5, g_print_toast_label);
    lv_anim_set_exec_cb(&a5, print_toast_anim_text_opa_cb);
    lv_anim_set_values(&a5, LV_OPA_0, LV_OPA_COVER);
    lv_anim_set_time(&a5, 180);
    lv_anim_set_path_cb(&a5, lv_anim_path_ease_out);
    lv_anim_start(&a5);
    }

    // 由小变大，增强“弹出来”的感觉
    lv_anim_t a4;
    lv_anim_init(&a4);
    lv_anim_set_var(&a4, g_print_toast);
    lv_anim_set_exec_cb(&a4, print_toast_anim_zoom_cb);
    lv_anim_set_values(&a4, PRINT_TOAST_ZOOM_START, PRINT_TOAST_ZOOM_END);
    lv_anim_set_time(&a4, 380);
    lv_anim_set_path_cb(&a4, lv_anim_path_overshoot);
    lv_anim_start(&a4);
}
void lv_print_toast_hide(void)
{
    if (!g_print_toast || !lv_obj_is_valid(g_print_toast)) return;
    if (lv_obj_has_flag(g_print_toast, LV_OBJ_FLAG_HIDDEN)) return;
    if (!g_print_toast_visible || g_print_toast_hiding) return;

    lv_anim_del(g_print_toast, print_toast_anim_y_cb);
    lv_anim_del(g_print_toast, print_toast_anim_zoom_cb);
    lv_anim_del(g_print_toast, print_toast_anim_bg_opa_cb);
    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_anim_del(g_print_toast_label, print_toast_anim_text_opa_cb);
    }
    g_print_toast_hiding = true;

    lv_obj_set_pos(g_print_toast, PRINT_TOAST_X, PRINT_TOAST_Y);
    lv_obj_set_style_transform_zoom(g_print_toast, PRINT_TOAST_ZOOM_END, 0);
    lv_obj_set_style_bg_opa(g_print_toast, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_opa(g_print_toast, LV_OPA_20, 0);
    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_obj_set_style_text_opa(g_print_toast_label, LV_OPA_COVER, 0);
    }

    lv_anim_t a0;
    lv_anim_init(&a0);
    lv_anim_set_var(&a0, g_print_toast);
    lv_anim_set_exec_cb(&a0, print_toast_anim_y_cb);
    lv_anim_set_values(&a0, PRINT_TOAST_Y, PRINT_TOAST_HIDE_Y);
    lv_anim_set_time(&a0, PRINT_TOAST_HIDE_TIME);
    lv_anim_set_path_cb(&a0, lv_anim_path_ease_out);
    lv_anim_start(&a0);

    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, g_print_toast);
    lv_anim_set_exec_cb(&a1, print_toast_anim_bg_opa_cb);
    lv_anim_set_values(&a1, LV_OPA_COVER, LV_OPA_0);
    lv_anim_set_time(&a1, PRINT_TOAST_HIDE_TIME);
    lv_anim_set_path_cb(&a1, lv_anim_path_ease_out);
    lv_anim_set_ready_cb(&a1, print_toast_hide_ready_cb);
    lv_anim_start(&a1);

    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_anim_t a2;
        lv_anim_init(&a2);
        lv_anim_set_var(&a2, g_print_toast_label);
        lv_anim_set_exec_cb(&a2, print_toast_anim_text_opa_cb);
        lv_anim_set_values(&a2, LV_OPA_COVER, LV_OPA_0);
        lv_anim_set_time(&a2, PRINT_TOAST_HIDE_TIME);
        lv_anim_set_path_cb(&a2, lv_anim_path_ease_out);
        lv_anim_start(&a2);
    }
}
