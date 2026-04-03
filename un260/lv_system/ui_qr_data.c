#include "ui_qr_data.h"
#include "un260/lv_system/platform_app.h"
#include "un260/lv_system/machine_time.h"
#include "un260/lv_system/ui_text.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static bool ui_qr_data_append(char* buf, size_t buf_size, size_t* used,
    const char* fmt, ...) //向二维码文本缓冲区追加内容
{
    int written;
    va_list args;

    if (buf == NULL || used == NULL || fmt == NULL || *used >= buf_size) {
        return false;
    }

    va_start(args, fmt);
    written = vsnprintf(buf + *used, buf_size - *used, fmt, args);
    va_end(args);

    if (written < 0 || (size_t)written >= (buf_size - *used)) {
        return false;
    }

    *used += (size_t)written;
    return true;
}

bool ui_qr_data_is_ready(void) //判断当前是否有有效点钞数据
{
    if (sim.total_pcs > 0) return true;
    if (sim.total_amount > 0.0f) return true;
    if (sim.err_num > 0) return true;
    return false;
}

bool ui_qr_data_build(char* buf, size_t buf_size) //组装当前点钞结果二维码文本
{
    size_t used = 0;
    uint16_t year = 0;
    uint8_t mon = 0, day = 0, hour = 0, min = 0, sec = 0;
    bool has_sn = false;
    bool has_err = false;
    int i;

    if (buf == NULL || buf_size == 0 || !ui_qr_data_is_ready()) {
        return false;
    }

    buf[0] = '\0';
    machine_time_get(&year, &mon, &day, &hour, &min, &sec);

    if (!ui_qr_data_append(buf, buf_size, &used,
        "%s=%04u-%02u-%02u %02u:%02u:%02u\n",
        ui_text_get(UI_TEXT_WIDGET_QR_DATA_TIME),
        (unsigned)year, (unsigned)mon, (unsigned)day,
        (unsigned)hour, (unsigned)min, (unsigned)sec)) {
        return false;
    }

    if (!ui_qr_data_append(buf, buf_size, &used, "%s=%s\n",
        ui_text_get(UI_TEXT_WIDGET_QR_DATA_CUR), Machine_para.curr_code)) {
        return false;
    }

    if (!ui_qr_data_append(buf, buf_size, &used, "%s=%.0f\n",
        ui_text_get(UI_TEXT_WIDGET_QR_DATA_AMOUNT), sim.total_amount)) {
        return false;
    }

    if (!ui_qr_data_append(buf, buf_size, &used, "%s=%d\n",
        ui_text_get(UI_TEXT_WIDGET_QR_DATA_PCS), sim.total_pcs)) {
        return false;
    }

    if (!ui_qr_data_append(buf, buf_size, &used, "%s=",
        ui_text_get(UI_TEXT_WIDGET_QR_DATA_SN))) {
        return false;
    }

    for (i = 0; i < sim.total_pcs; i++) {
        if (sim.sn_str == NULL || sim.sn_str[i] == NULL || sim.sn_str[i][0] == '\0') {
            continue;
        }

        if (has_sn) {
            if (!ui_qr_data_append(buf, buf_size, &used, ",")) {
                return false;
            }
        }

        if (!ui_qr_data_append(buf, buf_size, &used, "%s", sim.sn_str[i])) {
            return false;
        }

        has_sn = true;
    }

    if (!has_sn) {
        if (!ui_qr_data_append(buf, buf_size, &used, "-")) {
            return false;
        }
    }

    if (!ui_qr_data_append(buf, buf_size, &used, "\n%s=",
        ui_text_get(UI_TEXT_WIDGET_QR_DATA_ERR))) {
        return false;
    }

    for (i = 0; i < sim.err_num; i++) {
        if (sim.err_str == NULL || sim.err_str[i] == NULL || sim.err_str[i][0] == '\0') {
            continue;
        }

        if (has_err) {
            if (!ui_qr_data_append(buf, buf_size, &used, ",")) {
                return false;
            }
        }

        if (!ui_qr_data_append(buf, buf_size, &used, "%s", sim.err_str[i])) {
            return false;
        }

        if (sim.err_pcs != NULL) {
            if (!ui_qr_data_append(buf, buf_size, &used, "(%u)", (unsigned)sim.err_pcs[i])) {
                return false;
            }
        }

        has_err = true;
    }

    if (!has_err) {
        if (!ui_qr_data_append(buf, buf_size, &used, "-")) {
            return false;
        }
    }

    return true;
}
