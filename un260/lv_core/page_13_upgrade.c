#include "page_13_upgrade.h"
#include "un260/lv_core/lv_page_manager.h"

static lv_obj_t* page_13 = NULL;

static void esc_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_pop_page();
}

static void main_upgrade_enter_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_push_page(UI_PAGE_MAIN_UPGRADE);
}

static void image_upgrade_enter_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_push_page(UI_PAGE_IMAGE_UPGRADE);
}

static void ui_upgrade_enter_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    ui_manager_push_page(UI_PAGE_UI_UPGRADE);
}

void ui_page_13_upgrade_create(lv_obj_t* parent)
{
    if (page_13) return;

    lv_obj_t* root = parent ? parent : lv_scr_act();
    page_13 = lv_obj_create(root);
    lv_obj_remove_style_all(page_13);
    lv_obj_set_pos(page_13, 0, 0);
    lv_obj_set_size(page_13, 1280, 400);
    lv_obj_clear_flag(page_13, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(page_13, lv_color_hex(0xF2F6FB), 0);
    lv_obj_set_style_bg_opa(page_13, LV_OPA_COVER, 0);

    lv_obj_t* title = lv_label_create(page_13);
    lv_label_set_text(title, "UPGRADE MENU");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(title, 36, 20);

    lv_obj_t* esc_btn = lv_btn_create(page_13);
    lv_obj_set_size(esc_btn, 100, 60);
    lv_obj_set_pos(esc_btn, 1160, 14);
    lv_obj_add_event_cb(esc_btn, esc_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* esc_label = lv_label_create(esc_btn);
    lv_label_set_text(esc_label, "ESC");
    lv_obj_center(esc_label);

    lv_obj_t* btn_main = lv_btn_create(page_13);
    lv_obj_set_size(btn_main, 360, 70);
    lv_obj_set_pos(btn_main, 460, 105);
    lv_obj_add_event_cb(btn_main, main_upgrade_enter_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* lbl_main = lv_label_create(btn_main);
    lv_label_set_text(lbl_main, "MAIN BOARD UPGRADE");
    lv_obj_center(lbl_main);

    lv_obj_t* btn_image = lv_btn_create(page_13);
    lv_obj_set_size(btn_image, 360, 70);
    lv_obj_set_pos(btn_image, 460, 185);
    lv_obj_add_event_cb(btn_image, image_upgrade_enter_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* lbl_image = lv_label_create(btn_image);
    lv_label_set_text(lbl_image, "IMAGE BOARD UPGRADE");
    lv_obj_center(lbl_image);

    lv_obj_t* btn_ui = lv_btn_create(page_13);
    lv_obj_set_size(btn_ui, 360, 70);
    lv_obj_set_pos(btn_ui, 460, 265);
    lv_obj_add_event_cb(btn_ui, ui_upgrade_enter_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* lbl_ui = lv_label_create(btn_ui);
    lv_label_set_text(lbl_ui, "UI UPGRADE");
    lv_obj_center(lbl_ui);
}

void ui_page_13_upgrade_destroy(void)
{
    if (page_13 && lv_obj_is_valid(page_13)) {
        lv_obj_del(page_13);
    }
    page_13 = NULL;
}
