#include "un260/lv_core/page_08_boot.h"
#include "un260/lv_core/lv_page_manager.h"
#include "un260/lv_resources/lv_image_declear.h" 
#include "un260/lv_resources/lv_img_init.h" 
#include "lv_page_event.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/user_cfg.h"
#include "un260/lv_components/lv_components.h"
#include "un260/lv_refre/lvgl_refre.h"
#include "../aic_ui/aic_ui.h"
#include "page_08_boot.h"
ui_element_t page_08_curr_obj[] = {
    // 背景图

    { "page_08_boot_bg_img.png", LV_OBJ_TYPE_IMAGE, &page_07_currency_bg_img,
        { 0, 0, 1280, 400, 0, 0, 0 },
        { NULL, 0, 0, 0, NULL },
        { 255, 0, 0, false },
        NULL, 0, NULL, NULL },


};

int page_08_curr_len = sizeof(page_08_curr_obj) / sizeof(page_08_curr_obj[0]);

#define BOOTLOG_LINE_MS 400           
#define BOOTLOG_BUFFER_SIZE 8192       
#define BOOTLOG_CONT_W 682
#define BOOTLOG_CONT_H 354
#define BOOTLOG_CONT_X 575
#define BOOTLOG_CONT_Y 37

static lv_obj_t* boot_cont = NULL;
static lv_obj_t* boot_label = NULL;
static lv_timer_t* boot_timer = NULL;
static char boot_buf[BOOTLOG_BUFFER_SIZE];
static int boot_line_index = 0;

// 把你给的日志原样按行放到这里
static const char* boot_lines[] = {
    "Pre-Boot Program ... ",
    "DDR3 128MB",
    "Going to init DDR3. freq: 672MHz",
    "DDR3 initialized",
    "41123 56716 83516",
    "PBP done",
    "",
    "U-Boot SPL ",
    "[SPL]: Boot device = 5(BD_SPINAND)",
    "Trying to boot from SPINAND",
    "Jumping to Linux via RISC-V OpenSBI",
    "[    1.369909] Timeout during wait phy stop state c",
    "Startup time: 3.686 sec (from Power-On-Reset)",
    "Load touch driver: /",
    "Starting test_lvgl: OK",
    "Starting syslogd: OK",
    "Starting klogd: OK",
    "Starting mdev... OK",
    "Starting system message bus: dbus-daemon: dbus-daemon: no version information available (required by dbus-daemon)",
    "done",
    "Starting adbd: mkdir: can't create directory '/dev/pts': File exists",
    "Link encap:Local Loopback",
    "          inet addr:127.0.0.1  Mask:255.0.0.0",
    "          UP LOOPBACK RUNNING  MTU:65536  Metric:1",
    "          RX packets:0 errors:0 dropped:0 overruns:0 frame:0",
    "          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0",
    "          collisions:0 txqueuelen:1000",
    "          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)",
    "",
    "install_listener('tcp:5037','*smartsocket*')>>>",
     "Detecting Image Sensor>>>",
    "Detecting UV Sensor>>>",
    "Detecting MGS Sensor>>>",
    "Detecting MGB Sensor>>>",
    "Detecting Double Sensor>>>",
    "Detecting Start Sensor>>>",
    "Detecting Clear Sensor>>>",
    "Starting Image Board>>>",
    "Loading Clear Sensor Sensitivity>>>",
    "OK",
    "Welcome to UN260A!"
};
static const int boot_line_count = sizeof(boot_lines) / sizeof(boot_lines[0]);

static void boot_timer_cb(lv_timer_t* t)
{
    (void)t;
    if (!boot_label || !boot_cont) return;

    if (boot_line_index >= boot_line_count) {
        if (boot_timer) {
            lv_timer_del(boot_timer);
            boot_timer = NULL;
        }
        return;
    }

    size_t cur_len = strlen(boot_buf);
    size_t avail = (BOOTLOG_BUFFER_SIZE - cur_len - 2);
    if (avail > 0) {
        strncat(boot_buf, boot_lines[boot_line_index], avail);
        strncat(boot_buf, "\n", avail);
        lv_label_set_text(boot_label, boot_buf);
#ifdef LV_USE_OBJ_SCROLL_TO_VIEW
        lv_obj_scroll_to_view(boot_cont, boot_label, LV_ANIM_OFF);
#else
        lv_coord_t lh = lv_obj_get_height(boot_label);
        lv_coord_t ch = lv_obj_get_height(boot_cont);
        if (lh > ch) {
            lv_obj_scroll_to_y(boot_cont, lh, LV_ANIM_OFF);
        }
#endif
    }

    boot_line_index++;
}

void bootlog_start(lv_obj_t* parent)
{
    if (boot_timer) return; // 已在运行，则不重复创建

    memset(boot_buf, 0, sizeof(boot_buf));
    boot_line_index = 0;

    // 创建容器（白底）
    boot_cont = lv_obj_create(parent);
    lv_obj_set_size(boot_cont, BOOTLOG_CONT_W, BOOTLOG_CONT_H);
    lv_obj_set_pos(boot_cont, BOOTLOG_CONT_X, BOOTLOG_CONT_Y);
    lv_obj_set_style_bg_opa(boot_cont, LV_OPA_TRANSP, 0);

    // 🔹 删除边框
    lv_obj_set_style_border_width(boot_cont, 0, 0);

    // 🔹 去掉滚动条
    lv_obj_set_scrollbar_mode(boot_cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_style_pad_all(boot_cont, 8, 0);

    boot_label = lv_label_create(boot_cont);
    lv_obj_set_width(boot_label, BOOTLOG_CONT_W - 16);
    lv_label_set_long_mode(boot_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(boot_label, LV_ALIGN_TOP_LEFT, 8, 8);

    // 🔹 设置字体为绿色 (85,164,85)
    lv_obj_set_style_text_color(boot_label, lv_color_make(85, 164, 85), 0);

    boot_timer = lv_timer_create(boot_timer_cb, BOOTLOG_LINE_MS, NULL);
}
void bootlog_stop(void)
{
    if (boot_timer) {
        lv_timer_del(boot_timer);
        boot_timer = NULL;
    }
    if (boot_cont) {
        lv_obj_del(boot_cont);
        boot_cont = NULL;
        boot_label = NULL;
    }
}

void ui_page_08_curr_create(lv_obj_t* parent)
{
    if (boot_page) return;
    boot_page = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(boot_page);
    lv_obj_set_pos(boot_page, 0, 0);
    lv_obj_set_size(boot_page, 1280, 400);
    lv_obj_clear_flag(boot_page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(boot_page, LV_SCROLLBAR_MODE_OFF);
    lv_ui_obj_init(boot_page, page_08_curr_obj, page_08_curr_len);
    bootlog_start(boot_page);


};

void ui_page_08_curr_destroy(void)
{
    if (boot_page)
    {
        lv_obj_del(boot_page);
        boot_page = NULL;
    }

}
