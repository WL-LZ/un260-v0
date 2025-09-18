#include"platform_app.h"
#include "lvgl/lvgl.h"
#include"lv_str.h"
#include"user_cfg.h"

const STR_t str_group[Str_Max] = {
    {"REJECT:","Reject","zhongwen","hanyu"}
};


const char* get_str_by_name(const char* name) {
    for (int i = 0; i < Str_Max; i++) {
        if (strcmp(str_group[i].name, name) == 0) {
            switch (Machine_para.language) {
            case LANGUAGE_EN: return str_group[i].strEN;
            case LANGUAGE_CN: return str_group[i].strCN;
            case LANGUAGE_KR: return str_group[i].strKR;
            default: return str_group[i].strEN;
            }
        }

    }
    return NULL; // 没找到
}
