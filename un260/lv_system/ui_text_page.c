#include "ui_text.h"

const ui_text_item_t g_ui_text_page_group[UI_TEXT_PAGE_MAX] = {
    //PAGE01
    [UI_TEXT_PAGE01_QR_CODE] = {"QR CODE", "二维码", "QR 코드"},
    [UI_TEXT_PAGE01_BOTTOM_MODE_MDC] = {"MDC", "MDC", "MDC"},
    [UI_TEXT_PAGE01_BOTTOM_MODE_SDC] = {"SDC", "SDC", "SDC"},
    [UI_TEXT_PAGE01_BOTTOM_MODE_CNT] = {"CNT", "CNT", "CNT"},
    [UI_TEXT_PAGE01_BOTTOM_ADD_ON] = {"ADD:ON", "ADD:开", "ADD:ON"},
    [UI_TEXT_PAGE01_BOTTOM_ADD_OFF] = {"ADD:OFF", "ADD:关", "ADD:OFF"},
    [UI_TEXT_PAGE01_BOTTOM_WORK_AUTO] = {"AUTO", "自动", "AUTO"},
    [UI_TEXT_PAGE01_BOTTOM_WORK_MANUAL] = {"MANUAL", "手动", "MANUAL"},
    [UI_TEXT_PAGE01_BOTTOM_FO_ON] = {"F./O:ON", "F./O:开", "F./O:ON"},
    [UI_TEXT_PAGE01_BOTTOM_FO_OFF] = {"F./O:OFF", "F./O:关", "F./O:OFF"},

    //PAGE16
    [UI_TEXT_PAGE16_TITLE] = {"UI UPGRADE", "界面升级", "UI 업그레이드"},
    [UI_TEXT_PAGE16_ESC] = {"ESC", "返回", "ESC"},
    [UI_TEXT_PAGE16_USB_STATUS_FMT] = {"U DISK: %s", "U盘: %s", "U DISK: %s"},
    [UI_TEXT_PAGE16_USB_INSERTED] = {"INSERTED", "已插入", "삽입됨"},
    [UI_TEXT_PAGE16_USB_NOT_INSERTED] = {"NOT INSERTED", "未插入", "삽입 안 됨"},
    [UI_TEXT_PAGE16_FILE_STATUS_FMT] = {"MOUNT: %s   PACKAGE: %s", "挂载: %s   升级包: %s", "MOUNT: %s   PACKAGE: %s"},
    [UI_TEXT_PAGE16_MOUNT_OK] = {"OK", "正常", "정상"},
    [UI_TEXT_PAGE16_MOUNT_NOT] = {"NOT MOUNTED", "未挂载", "마운트 안 됨"},
    [UI_TEXT_PAGE16_PACKAGE_FOUND] = {"FOUND", "已找到", "찾음"},
    [UI_TEXT_PAGE16_PACKAGE_NOT_FOUND] = {"NOT FOUND", "未找到", "찾지 못함"},
    [UI_TEXT_PAGE16_HINT_INSERT_USB] = {"Insert U disk to begin.", "请插入U盘开始升级。", "업그레이드를 시작하려면 USB를 삽입하세요."},
    [UI_TEXT_PAGE16_HINT_INSERTED_CLICK_UPGRADE] = {"U disk inserted. Click UPGRADE to auto-mount and upgrade.", "U盘已插入，点击升级开始自动挂载并升级。", "USB가 삽입되었습니다. UPGRADE를 눌러 자동 마운트 후 업그레이드하세요."},
    [UI_TEXT_PAGE16_HINT_MISSING_PACKAGE] = {"Mounted. Missing /mnt/usb/update/test_lvgl.", "已挂载，但缺少 /mnt/usb/update/test_lvgl。", "마운트되었지만 /mnt/usb/update/test_lvgl 이 없습니다."},
    [UI_TEXT_PAGE16_HINT_READY] = {"Ready to upgrade UI.", "已就绪，可以升级界面。", "UI 업그레이드 준비 완료."},
    [UI_TEXT_PAGE16_HINT_UPGRADING_WAIT] = {"Upgrading... please wait.", "正在升级，请稍候。", "업그레이드 중입니다. 잠시만 기다려 주세요."},
    [UI_TEXT_PAGE16_HINT_UPGRADING_WAIT_FMT] = {"Upgrading... please wait. (%us)", "正在升级，请稍候。(%us)", "업그레이드 중입니다. 잠시만 기다려 주세요. (%us)"},
    [UI_TEXT_PAGE16_HINT_FINISHED] = {"Update finished.", "升级完成。", "업데이트 완료."},
    [UI_TEXT_PAGE16_HINT_FAILED] = {"Update failed. Please check the package.", "升级失败，请检查升级包。", "업데이트 실패. 패키지를 확인해 주세요."},
    [UI_TEXT_PAGE16_HINT_NO_USB] = {"No U disk. Please insert U disk.", "未检测到U盘，请插入U盘。", "USB가 없습니다. USB를 삽입해 주세요."},
    [UI_TEXT_PAGE16_HINT_AUTO_MOUNT_FAILED] = {"Auto-mount failed. Please mount U disk to /mnt/usb.", "自动挂载失败，请将U盘挂载到 /mnt/usb。", "자동 마운트 실패. USB를 /mnt/usb 에 마운트해 주세요."},
    [UI_TEXT_PAGE16_HINT_START_SCRIPT_FAILED] = {"Start update script failed.", "启动升级脚本失败。", "업데이트 스크립트 시작 실패."},
    [UI_TEXT_PAGE16_UPGRADE_BTN] = {"UPGRADE UI", "升级界面", "UI 업그레이드"},
    [UI_TEXT_PAGE16_POWER_OFF] = {"Do NOT power off during update.", "升级过程中请勿断电。", "업데이트 중 전원을 끄지 마십시오."},
    [UI_TEXT_PAGE16_POWER_OFF_LONG] = {"Still updating... Do NOT power off.", "仍在升级中，请勿断电。", "계속 업데이트 중입니다. 전원을 끄지 마십시오."}
};
