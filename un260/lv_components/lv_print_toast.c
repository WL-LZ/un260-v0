#include "lv_print_toast.h"
#include "lv_loading_orbit.h"
#include <string.h>

// 打印提示框参数
#define PRINT_TOAST_X               502
#define PRINT_TOAST_Y               31
#define PRINT_TOAST_W               277
#define PRINT_TOAST_H               101

// 动画参数
#define PRINT_TOAST_SHOW_TIME       285
#define PRINT_TOAST_HIDE_TIME       500
#define PRINT_TOAST_AUTO_HIDE_MS    5000

// 显示：从屏幕顶部之外滑下到最终位置
#define PRINT_TOAST_START_Y         (-150)
#define PRINT_TOAST_END_Y           31

// 淡入：与位移同步，避免先显示完再挪位置
#define PRINT_TOAST_OPA_START       0
#define PRINT_TOAST_OPA_END         LV_OPA_COVER
#define PRINT_TOAST_FADE_TIME       285
#define PRINT_TOAST_TEXT_FADE_DELAY 0

// 全局对象
static lv_obj_t *g_print_toast = NULL;
static lv_obj_t *g_print_toast_label = NULL;
static lv_obj_t *g_print_toast_loader = NULL;
static lv_timer_t *g_print_toast_auto_hide_timer = NULL;
static bool g_print_toast_visible = false;
static bool g_print_toast_hiding = false;

// Y 动画回调
static void print_toast_anim_y_cb(void *var, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)var, (lv_coord_t)v);
}

// 背景/阴影透明度动画回调
static void print_toast_anim_bg_opa_cb(void *var, int32_t v)
{
    lv_obj_set_style_bg_opa((lv_obj_t *)var, (lv_opa_t)v, 0);
    lv_obj_set_style_shadow_opa((lv_obj_t *)var,
                                (lv_opa_t)((v * LV_OPA_20) / LV_OPA_COVER), 0);
}

// 文字透明度动画回调
static void print_toast_anim_text_opa_cb(void *var, int32_t v)
{
    lv_obj_set_style_text_opa((lv_obj_t *)var, (lv_opa_t)v, 0);
}

static void print_toast_stop_auto_hide_timer(void)
{
    if (g_print_toast_auto_hide_timer) {
        lv_timer_del(g_print_toast_auto_hide_timer);
        g_print_toast_auto_hide_timer = NULL;
    }
}

// 隐藏动画结束回调
static void print_toast_hide_ready_cb(lv_anim_t *a)
{
    LV_UNUSED(a);

    if (g_print_toast && lv_obj_is_valid(g_print_toast)) {
        lv_obj_add_flag(g_print_toast, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(g_print_toast, PRINT_TOAST_X, PRINT_TOAST_END_Y);
        lv_obj_set_style_bg_opa(g_print_toast, LV_OPA_0, 0);
        lv_obj_set_style_shadow_opa(g_print_toast, LV_OPA_0, 0);

        if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
            lv_obj_set_style_text_opa(g_print_toast_label, LV_OPA_0, 0);
        }
    }

    if (g_print_toast_loader && lv_obj_is_valid(g_print_toast_loader)) {
        lv_obj_set_style_opa(g_print_toast_loader, LV_OPA_COVER, 0);
    }

    print_toast_stop_auto_hide_timer();
    g_print_toast_visible = false;
    g_print_toast_hiding = false;
}

static void print_toast_auto_hide_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    print_toast_stop_auto_hide_timer();
    lv_print_toast_hide();
}

void lv_print_toast_create(void)
{
    if (g_print_toast && lv_obj_is_valid(g_print_toast)) {
        return;
    }

    // 挂到 top layer，上层页面切换不影响
    g_print_toast = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(g_print_toast);

    lv_obj_set_pos(g_print_toast, PRINT_TOAST_X, PRINT_TOAST_END_Y);
    lv_obj_set_size(g_print_toast, PRINT_TOAST_W, PRINT_TOAST_H);

    lv_obj_clear_flag(g_print_toast, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(g_print_toast, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(g_print_toast, LV_OBJ_FLAG_HIDDEN);

    // 背景
    lv_obj_set_style_bg_color(g_print_toast, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(g_print_toast, LV_OPA_0, 0);
    lv_obj_set_style_radius(g_print_toast, 22, 0);
    lv_obj_set_style_border_width(g_print_toast, 0, 0);

    // 阴影：20% 黑色，右移 2，扩散 10
    lv_obj_set_style_shadow_color(g_print_toast, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(g_print_toast, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(g_print_toast, 10, 0);
    lv_obj_set_style_shadow_spread(g_print_toast, 0, 0);
    lv_obj_set_style_shadow_ofs_x(g_print_toast, 2, 0);
    lv_obj_set_style_shadow_ofs_y(g_print_toast, 4, 0);

    // 标签
    g_print_toast_label = lv_label_create(g_print_toast);
    lv_label_set_text(g_print_toast_label, "Printing...");
    lv_obj_set_width(g_print_toast_label, PRINT_TOAST_W - 20);
    lv_obj_center(g_print_toast_label);

    lv_obj_set_style_text_align(g_print_toast_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(g_print_toast_label, lv_color_hex(0x2F3542), 0);
    lv_obj_set_style_text_font(g_print_toast_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_opa(g_print_toast_label, LV_OPA_0, 0);

    g_print_toast_loader = lv_loading_orbit_create(g_print_toast);
    lv_obj_align(g_print_toast_loader, LV_ALIGN_TOP_RIGHT, -12, 10);
    lv_obj_set_style_opa(g_print_toast_loader, LV_OPA_COVER, 0);
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

    print_toast_stop_auto_hide_timer();

    // 删除旧动画，避免重复触发时跳动
    lv_anim_del(g_print_toast, print_toast_anim_y_cb);
    lv_anim_del(g_print_toast, print_toast_anim_bg_opa_cb);
    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_anim_del(g_print_toast_label, print_toast_anim_text_opa_cb);
    }
    if (g_print_toast_loader && lv_obj_is_valid(g_print_toast_loader)) {
        lv_anim_del(g_print_toast_loader, print_toast_anim_text_opa_cb);
    }

    g_print_toast_hiding = false;

    // 先设置初始状态，再显示，避免闪烁
    lv_obj_add_flag(g_print_toast, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(g_print_toast, PRINT_TOAST_X, PRINT_TOAST_START_Y);
    lv_obj_set_style_bg_opa(g_print_toast, PRINT_TOAST_OPA_START, 0);
    lv_obj_set_style_shadow_opa(g_print_toast,
                                (lv_opa_t)((PRINT_TOAST_OPA_START * LV_OPA_20) / LV_OPA_COVER), 0);

    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_obj_set_style_text_opa(g_print_toast_label, PRINT_TOAST_OPA_START, 0);
    }
    if (g_print_toast_loader && lv_obj_is_valid(g_print_toast_loader)) {
        lv_obj_set_style_opa(g_print_toast_loader, PRINT_TOAST_OPA_START, 0);
    }

    lv_obj_clear_flag(g_print_toast, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(g_print_toast);

    // 单段从顶部滑下，直接到位，不做回弹
    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, g_print_toast);
    lv_anim_set_exec_cb(&a1, print_toast_anim_y_cb);
    lv_anim_set_values(&a1, PRINT_TOAST_START_Y, PRINT_TOAST_END_Y);
    lv_anim_set_time(&a1, PRINT_TOAST_SHOW_TIME);
    lv_anim_set_path_cb(&a1, lv_anim_path_ease_out);
    lv_anim_start(&a1);

    // 背景/阴影淡入：保持现有质感
    lv_anim_t a2;
    lv_anim_init(&a2);
    lv_anim_set_var(&a2, g_print_toast);
    lv_anim_set_exec_cb(&a2, print_toast_anim_bg_opa_cb);
    lv_anim_set_values(&a2, PRINT_TOAST_OPA_START, PRINT_TOAST_OPA_END);
    lv_anim_set_time(&a2, PRINT_TOAST_FADE_TIME);
    lv_anim_set_path_cb(&a2, lv_anim_path_ease_out);
    lv_anim_start(&a2);

    // 文字淡入
    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_anim_t a3;
        lv_anim_init(&a3);
        lv_anim_set_var(&a3, g_print_toast_label);
        lv_anim_set_exec_cb(&a3, print_toast_anim_text_opa_cb);
        lv_anim_set_values(&a3, PRINT_TOAST_OPA_START, PRINT_TOAST_OPA_END);
        lv_anim_set_time(&a3, PRINT_TOAST_FADE_TIME);
        lv_anim_set_delay(&a3, PRINT_TOAST_TEXT_FADE_DELAY);
        lv_anim_set_path_cb(&a3, lv_anim_path_ease_out);
        lv_anim_start(&a3);
    }

    if (g_print_toast_loader && lv_obj_is_valid(g_print_toast_loader)) {
        lv_anim_t a4;
        lv_anim_init(&a4);
        lv_anim_set_var(&a4, g_print_toast_loader);
        lv_anim_set_exec_cb(&a4, print_toast_anim_text_opa_cb);
        lv_anim_set_values(&a4, PRINT_TOAST_OPA_START, PRINT_TOAST_OPA_END);
        lv_anim_set_time(&a4, PRINT_TOAST_FADE_TIME);
        lv_anim_set_path_cb(&a4, lv_anim_path_ease_out);
        lv_anim_start(&a4);
    }

    g_print_toast_visible = true;
    g_print_toast_hiding = false;
    g_print_toast_auto_hide_timer = lv_timer_create(print_toast_auto_hide_timer_cb,
                                                    PRINT_TOAST_AUTO_HIDE_MS, NULL);
    lv_timer_set_repeat_count(g_print_toast_auto_hide_timer, 1);
}

void lv_print_toast_hide(void)
{
    if (!g_print_toast || !lv_obj_is_valid(g_print_toast)) return;
    if (lv_obj_has_flag(g_print_toast, LV_OBJ_FLAG_HIDDEN)) return;
    if (!g_print_toast_visible || g_print_toast_hiding) return;

    lv_anim_del(g_print_toast, print_toast_anim_y_cb);
    lv_anim_del(g_print_toast, print_toast_anim_bg_opa_cb);
    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_anim_del(g_print_toast_label, print_toast_anim_text_opa_cb);
    }
    if (g_print_toast_loader && lv_obj_is_valid(g_print_toast_loader)) {
        lv_anim_del(g_print_toast_loader, print_toast_anim_text_opa_cb);
    }

    print_toast_stop_auto_hide_timer();
    g_print_toast_hiding = true;

    lv_obj_set_pos(g_print_toast, PRINT_TOAST_X, PRINT_TOAST_END_Y);
    lv_obj_set_style_bg_opa(g_print_toast, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_opa(g_print_toast, LV_OPA_20, 0);

    if (g_print_toast_label && lv_obj_is_valid(g_print_toast_label)) {
        lv_obj_set_style_text_opa(g_print_toast_label, LV_OPA_COVER, 0);
    }
    if (g_print_toast_loader && lv_obj_is_valid(g_print_toast_loader)) {
        lv_obj_set_style_opa(g_print_toast_loader, LV_OPA_COVER, 0);
    }

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

    if (g_print_toast_loader && lv_obj_is_valid(g_print_toast_loader)) {
        lv_anim_t a3;
        lv_anim_init(&a3);
        lv_anim_set_var(&a3, g_print_toast_loader);
        lv_anim_set_exec_cb(&a3, print_toast_anim_text_opa_cb);
        lv_anim_set_values(&a3, LV_OPA_COVER, LV_OPA_0);
        lv_anim_set_time(&a3, PRINT_TOAST_HIDE_TIME);
        lv_anim_set_path_cb(&a3, lv_anim_path_ease_out);
        lv_anim_start(&a3);
    }

}
