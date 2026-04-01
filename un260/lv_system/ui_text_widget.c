#include "ui_text.h"
#include "ui_lang.h"
#include <stddef.h>

extern const ui_text_item_t g_ui_text_page_group[UI_TEXT_PAGE_MAX];

static const ui_text_item_t g_ui_text_widget_group[UI_TEXT_MAX - UI_TEXT_WIDGET_BASE] = {
    //升级弹窗
    [UI_TEXT_WIDGET_UPGRADE_POPUP_TAG - UI_TEXT_WIDGET_BASE] = {"SYSTEM UPGRADE", "系统升级", "시스템 업그레이드"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_PROMPT_TITLE - UI_TEXT_WIDGET_BASE] = {"Upgrade package detected", "检测到升级包", "업그레이드 패키지 감지됨"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_PROMPT_DESC - UI_TEXT_WIDGET_BASE] = {
        "A USB drive with an upgrade package was detected. Do not power off the device or remove the USB drive during the upgrade.",
        "检测到包含升级包的U盘。升级期间请勿断电或移除U盘。",
        "업그레이드 패키지가 포함된 USB 드라이브가 감지되었습니다. 업그레이드 중에는 전원을 끄거나 USB 드라이브를 제거하지 마십시오."
    },
    [UI_TEXT_WIDGET_UPGRADE_POPUP_PROMPT_BTN_CANCEL - UI_TEXT_WIDGET_BASE] = {"Cancel", "取消", "취소"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_PROMPT_BTN_CONFIRM - UI_TEXT_WIDGET_BASE] = {"Upgrade Now", "立即升级", "지금 업그레이드"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_PROGRESS_TITLE - UI_TEXT_WIDGET_BASE] = {"Updating system", "正在升级系统", "시스템 업그레이드 중"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_PROGRESS_DESC - UI_TEXT_WIDGET_BASE] = {"Do not power off or remove the USB drive", "请勿断电或移除U盘", "전원을 끄거나 USB 드라이브를 제거하지 마십시오"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_PROGRESS_STEP_VERIFY - UI_TEXT_WIDGET_BASE] = {"Verifying upgrade package", "正在校验升级包", "업그레이드 패키지 확인 중"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_PROGRESS_STEP_VERIFY_FAIL - UI_TEXT_WIDGET_BASE] = {"Upgrade package verification failed", "升级包校验失败", "업그레이드 패키지 확인 실패"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_SUCCESS_TITLE - UI_TEXT_WIDGET_BASE] = {"Upgrade complete", "升级完成", "업그레이드 완료"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_SUCCESS_DESC - UI_TEXT_WIDGET_BASE] = {"The system has been updated successfully. Reboot the device now?", "系统已升级成功，是否现在重启设备？", "시스템이 성공적으로 업데이트되었습니다. 지금 장치를 재부팅하시겠습니까?"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_SUCCESS_BTN_LATER - UI_TEXT_WIDGET_BASE] = {"Later", "稍后重启", "나중에"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_SUCCESS_BTN_CONFIRM - UI_TEXT_WIDGET_BASE] = {"Confirm Reboot", "确认重启", "재부팅 확인"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_FAIL_TITLE - UI_TEXT_WIDGET_BASE] = {"Upgrade failed", "升级失败", "업그레이드 실패"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_FAIL_DESC - UI_TEXT_WIDGET_BASE] = {"The upgrade package is invalid. Please check the file and try again.", "升级文件异常，请检查后重试。", "업그레이드 패키지에 문제가 있습니다. 파일을 확인한 후 다시 시도하십시오."},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_FAIL_BTN_RETRY - UI_TEXT_WIDGET_BASE] = {"Retry Detection", "重新检测", "다시 검색"},
    [UI_TEXT_WIDGET_UPGRADE_POPUP_FAIL_START_SCRIPT - UI_TEXT_WIDGET_BASE] = {"Failed to start the upgrade script. Please try again.", "升级脚本启动失败，请重试。", "업그레이드 스크립트를 시작하지 못했습니다. 다시 시도하십시오."},

    //打印弹窗
    [UI_TEXT_WIDGET_PRINT_TOAST_PRINTING - UI_TEXT_WIDGET_BASE] = {"Printing...", "正在打印...", "인쇄 중..."},
    [UI_TEXT_WIDGET_PRINT_TOAST_COUNT_FIRST - UI_TEXT_WIDGET_BASE] = {"Please count first", "请先点钞", "먼저 계수해 주세요"}
};

static const char* ui_text_pick(const ui_text_item_t* item) //按当前语言选择文本
{
    if (item == NULL) {
        return "";
    }

    switch (ui_lang_get()) {
    case LANGUAGE_CN:
        return item->cn ? item->cn : item->en;

    case LANGUAGE_KR:
        return item->kr ? item->kr : item->en;

    case LANGUAGE_EN:
    default:
        return item->en ? item->en : "";
    }
}

const char* ui_text_get(ui_text_id_t text_id) //统一获取当前语言文本
{
    if (text_id < UI_TEXT_PAGE_MAX) {
        return ui_text_pick(&g_ui_text_page_group[text_id]);
    }

    if (text_id >= UI_TEXT_WIDGET_BASE && text_id < UI_TEXT_MAX) {
        return ui_text_pick(&g_ui_text_widget_group[text_id - UI_TEXT_WIDGET_BASE]);
    }

    return "";
}
