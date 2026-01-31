#ifndef USER_CFG_H
#define USER_CFG_H
#include <stdint.h>
#include "un260/lv_drivers/lv_drivers.h"
#define LV_DEBUG 1
typedef enum {
    CURR_CNY_ITEM = 0,
    CURR_USD_ITEM ,
    CURR_EUR_ITEM ,
    CURR_GBP_ITEM ,
    CURR_KRW_ITEM,
    CURR_EGP_ITEM,
    CURR_ISK_ITEM,
    CURR_PHP_ITEM,
    CURR_SOS_ITEM,
    CURR_TRY_ITEM,
    CURR_AED_ITEM,
    CURR_SAR_ITEM,
    CURR_OMR_ITEM,
    CURR_QAR_ITEM,
    CURR_MAD_ITEM,
    CURR_DZD_ITEM,    
    CURR_INR_ITEM,
    CURR_PKR_ITEM,
    CURR_IQD_ITEM,
    CURR_COUNT
}curr_item_t;
#define PCS_BATCH_MODE 0
#define AMOUNT_BATCH_MODE 1
//Function
#define SPEED_MODE 3
#define CFD_MODE 3
#define FO_MODE 4
#define ADD_MODE 2
#define WORK_MODE 2
#define PAGE_01_REPORT_ITEM 8
#define PAGE_02_A_ITEM 8
#define PAGE_02_B_ITEM 9
#define PAGE_02_C_ITEM 9
#define PAGE_02_DEBUG 1
#define PAGE_07_CURRENCIES 4
typedef struct {

    int mode;
    char curr_code[4];
    uint8_t speed;
    bool add_enabel;
    bool start_auto;
    curr_item_t current_currency;
    char password[5];
    int batch_mode;
    int32_t batch_num;
    bool batch_switch_enable;
    uint8_t cfd_mode;
    uint8_t fo_mode;
    uint8_t work_mode;
    uint8_t language;
    uint8_t selected_currency;
    uint32_t batch_amount;
    uint8_t reject_pocket_max;
    bool buzzer_enable;
    bool serial_num_enable;
}Machine_para_t;

extern Machine_para_t Machine_para;

typedef struct {
    // 版本信息
    char main_app[32];      // 主控软件版本
    char image_app[32];     // 图像软件版本 
    char fpga[32];          // FPGA版本
    char thka_app[32];      // 主控BOOT版本
    char ecb[32];           // 图像BOOT版本
    char display_app[32];   // 显示软件版本(预留)
    bool version_valid;     // 版本数据是否有效
    handshake_state_t g_handshake_state;
} Machine_Statue_t;

extern Machine_Statue_t Machine_Statue;

 enum {
    MODE_NONE,
    MODE_MDC,
    MODE_CNT,
    MODE_VER,
    MODE_SDC

};


void user_data_init(void);

extern const char* currencies[];
extern int  currencies_count;


#endif // !USER_CFG_H
