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
    [UI_TEXT_WIDGET_PRINT_TOAST_COUNT_FIRST - UI_TEXT_WIDGET_BASE] = {"Please count first", "请先点钞", "먼저 계수해 주세요"},

    //二维码弹窗
    [UI_TEXT_WIDGET_QR_POPUP_TITLE - UI_TEXT_WIDGET_BASE] = {"COUNTING QR", "点钞二维码", "계수 QR"},
    [UI_TEXT_WIDGET_QR_POPUP_DESC - UI_TEXT_WIDGET_BASE] = {"Scan with your phone to view counting details", "使用手机扫码查看本次点钞详情", "휴대폰으로 스캔해 이번 계수 상세를 확인하세요"},
    [UI_TEXT_WIDGET_QR_POPUP_BTN_CLOSE - UI_TEXT_WIDGET_BASE] = {"Close", "关闭", "닫기"},
    [UI_TEXT_WIDGET_QR_POPUP_NO_DATA - UI_TEXT_WIDGET_BASE] = {"No counting data", "暂无点钞数据", "계수 데이터가 없습니다"},
    [UI_TEXT_WIDGET_QR_POPUP_DATA_TOO_LARGE - UI_TEXT_WIDGET_BASE] = {"QR data is too large", "二维码数据过大", "QR 데이터가 너무 큽니다"},
    [UI_TEXT_WIDGET_QR_DATA_TIME - UI_TEXT_WIDGET_BASE] = {"TIME", "时间", "시간"},
    [UI_TEXT_WIDGET_QR_DATA_CUR - UI_TEXT_WIDGET_BASE] = {"CUR", "币种", "통화"},
    [UI_TEXT_WIDGET_QR_DATA_AMOUNT - UI_TEXT_WIDGET_BASE] = {"AMOUNT", "金额", "금액"},
    [UI_TEXT_WIDGET_QR_DATA_PCS - UI_TEXT_WIDGET_BASE] = {"PCS", "张数", "매수"},
    [UI_TEXT_WIDGET_QR_DATA_SN - UI_TEXT_WIDGET_BASE] = {"SN", "冠字号", "일련번호"},
    [UI_TEXT_WIDGET_QR_DATA_ERR - UI_TEXT_WIDGET_BASE] = {"ERR", "报错", "오류"},

    //灵动岛动作页
    [UI_TEXT_WIDGET_SMART_ISLAND_ACTION_QR - UI_TEXT_WIDGET_BASE] = {"QR", "二维码", "QR"},
    [UI_TEXT_WIDGET_SMART_ISLAND_ACTION_TIME - UI_TEXT_WIDGET_BASE] = {"Time", "时间", "시간"},
    [UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC3 - UI_TEXT_WIDGET_BASE] = {"Func3", "功能3", "기능3"},
    [UI_TEXT_WIDGET_SMART_ISLAND_ACTION_FUNC4 - UI_TEXT_WIDGET_BASE] = {"Func4", "功能4", "기능4"},

    //灵动岛状态文案
    [UI_TEXT_WIDGET_SMART_ISLAND_READY_TITLE - UI_TEXT_WIDGET_BASE] = {"Ready", "就绪", "준비됨"},
    [UI_TEXT_WIDGET_SMART_ISLAND_READY_SUBTITLE - UI_TEXT_WIDGET_BASE] = {"System OK", "系统正常", "시스템 정상"},
    [UI_TEXT_WIDGET_SMART_ISLAND_EXPAND_TITLE - UI_TEXT_WIDGET_BASE] = {"SYSTEM STATUS", "系统状态", "시스템 상태"},
    [UI_TEXT_WIDGET_SMART_ISLAND_EXPAND_SUBTITLE - UI_TEXT_WIDGET_BASE] = {"ALL ACTIVE", "全部正常", "모두 정상"},
    [UI_TEXT_WIDGET_SMART_ISLAND_COUNTING_TITLE - UI_TEXT_WIDGET_BASE] = {"Counting...", "正在点钞...", "계수 중..."},
    [UI_TEXT_WIDGET_SMART_ISLAND_COUNTING_SUBTITLE - UI_TEXT_WIDGET_BASE] = {"Running", "运行中", "실행 중"},
    [UI_TEXT_WIDGET_SMART_ISLAND_RUNNING_TITLE - UI_TEXT_WIDGET_BASE] = {"Running...", "正在运行...", "실행 중..."},
    [UI_TEXT_WIDGET_SMART_ISLAND_RUNNING_SUBTITLE - UI_TEXT_WIDGET_BASE] = {"Check banknotes", "请检查纸钞", "지폐를 확인해 주세요"},
    [UI_TEXT_WIDGET_SMART_ISLAND_WARNING_TITLE - UI_TEXT_WIDGET_BASE] = {"Warning", "警告", "경고"},
    [UI_TEXT_WIDGET_SMART_ISLAND_WARNING_SUBTITLE - UI_TEXT_WIDGET_BASE] = {"Warning", "警告", "경고"},
    [UI_TEXT_WIDGET_SMART_ISLAND_RESULT_TITLE - UI_TEXT_WIDGET_BASE] = {"Count Finished", "点钞完成", "계수 완료"},
    [UI_TEXT_WIDGET_SMART_ISLAND_RESULT_SUBTITLE - UI_TEXT_WIDGET_BASE] = {"Done", "完成", "완료"},
    [UI_TEXT_WIDGET_SMART_ISLAND_COUNT_FINISHED - UI_TEXT_WIDGET_BASE] = {"Count Finished", "点钞完成", "계수 완료"},
    [UI_TEXT_WIDGET_SMART_ISLAND_COUNT_ERROR - UI_TEXT_WIDGET_BASE] = {"Count Error", "点钞错误", "계수 오류"},
    [UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_TITLE - UI_TEXT_WIDGET_BASE] = {"Updating...", "正在升级...", "업데이트 중..."},
    [UI_TEXT_WIDGET_SMART_ISLAND_UPDATE_SUBTITLE - UI_TEXT_WIDGET_BASE] = {"Please wait", "请稍候", "잠시만 기다려 주세요"},
    [UI_TEXT_WIDGET_SMART_ISLAND_QR_READY - UI_TEXT_WIDGET_BASE] = {"QR Ready", "二维码就绪", "QR 준비됨"},
    [UI_TEXT_WIDGET_SMART_ISLAND_QR_SUBTITLE - UI_TEXT_WIDGET_BASE] = {"Tap to continue", "点击继续", "탭하여 계속"}
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
