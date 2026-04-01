#include "lv_upgrade_popup_text.h"
#include "un260/lv_system/ui_lang.h"

typedef struct {
    const char* en;
    const char* cn;
    const char* kr;
} upgrade_popup_text_item_t;

static const upgrade_popup_text_item_t g_upgrade_popup_text_group[UPGRADE_POPUP_TEXT_MAX] = {
    [UPGRADE_POPUP_TEXT_TAG] = {"SYSTEM UPGRADE", "系统升级", "시스템 업그레이드"},
    [UPGRADE_POPUP_TEXT_PROMPT_TITLE] = {"Upgrade package detected", "检测到升级包", "업그레이드 패키지 감지됨"},
    [UPGRADE_POPUP_TEXT_PROMPT_DESC] = {
        "A USB drive with an upgrade package was detected. Do not power off the device or remove the USB drive during the upgrade.",
        "检测到包含升级包的U盘。升级期间请勿断电或移除U盘。",
        "업그레이드 패키지가 포함된 USB 드라이브가 감지되었습니다. 업그레이드 중에는 전원을 끄거나 USB 드라이브를 제거하지 마십시오."
    },
    [UPGRADE_POPUP_TEXT_PROMPT_BTN_CANCEL] = {"Cancel", "取消", "취소"},
    [UPGRADE_POPUP_TEXT_PROMPT_BTN_CONFIRM] = {"Upgrade Now", "立即升级", "지금 업그레이드"},
    [UPGRADE_POPUP_TEXT_PROGRESS_TITLE] = {"Updating system", "正在升级系统", "시스템 업그레이드 중"},
    [UPGRADE_POPUP_TEXT_PROGRESS_DESC] = {
        "Do not power off or remove the USB drive",
        "请勿断电或移除U盘",
        "전원을 끄거나 USB 드라이브를 제거하지 마십시오"
    },
    [UPGRADE_POPUP_TEXT_PROGRESS_STEP_VERIFY] = {"Verifying upgrade package", "正在校验升级包", "업그레이드 패키지 확인 중"},
    [UPGRADE_POPUP_TEXT_PROGRESS_STEP_VERIFY_FAIL] = {
        "Upgrade package verification failed",
        "升级包校验失败",
        "업그레이드 패키지 확인 실패"
    },
    [UPGRADE_POPUP_TEXT_SUCCESS_TITLE] = {"Upgrade complete", "升级完成", "업그레이드 완료"},
    [UPGRADE_POPUP_TEXT_SUCCESS_DESC] = {
        "The system has been updated successfully. Reboot the device now?",
        "系统已升级成功，是否现在重启设备？",
        "시스템이 성공적으로 업데이트되었습니다. 지금 장치를 재부팅하시겠습니까?"
    },
    [UPGRADE_POPUP_TEXT_SUCCESS_BTN_LATER] = {"Later", "稍后重启", "나중에"},
    [UPGRADE_POPUP_TEXT_SUCCESS_BTN_CONFIRM] = {"Confirm Reboot", "确认重启", "재부팅 확인"},
    [UPGRADE_POPUP_TEXT_FAIL_TITLE] = {"Upgrade failed", "升级失败", "업그레이드 실패"},
    [UPGRADE_POPUP_TEXT_FAIL_DESC] = {
        "The upgrade package is invalid. Please check the file and try again.",
        "升级文件异常，请检查后重试。",
        "업그레이드 패키지에 문제가 있습니다. 파일을 확인한 후 다시 시도하십시오."
    },
    [UPGRADE_POPUP_TEXT_FAIL_BTN_RETRY] = {"Retry Detection", "重新检测", "다시 검색"},
    [UPGRADE_POPUP_TEXT_FAIL_START_SCRIPT] = {
        "Failed to start the upgrade script. Please try again.",
        "升级脚本启动失败，请重试。",
        "업그레이드 스크립트를 시작하지 못했습니다. 다시 시도하십시오."
    }
};

const char* lv_upgrade_popup_text_get(upgrade_popup_text_id_t text_id) //获取升级弹窗当前语言文本
{
    const upgrade_popup_text_item_t* item;

    if (text_id >= UPGRADE_POPUP_TEXT_MAX) {
        return "";
    }

    item = &g_upgrade_popup_text_group[text_id];

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
