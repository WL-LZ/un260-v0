#ifndef USER_CFG_H
#define USER_CFG_H
#include <stdbool.h>
#include <stdint.h>
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
#define UI_VERSION  "V1.0.0"

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
#define MAX_CURRENCIES 32

#define PRINT_SETTING_HEAD_MAX_LEN 20
#define PRINT_SETTING_SPACE_MAX_LINES 99
#define PRINT_SETTING_CONTENT_LIST 0x01
#define PRINT_SETTING_CONTENT_SN 0x02
#define PRINT_SETTING_CONTENT_LIST_SN 0x03
#define DOUBLE_NOTE_LEVEL_MIN 1
#define DOUBLE_NOTE_LEVEL_MAX 3
#define FLAP_POSITION_UP 0x01
#define FLAP_POSITION_DOWN 0x02
#define REJECT_POCKET_MIN_CAPACITY 30
#define REJECT_POCKET_MAX_CAPACITY 100
#define SERIAL_NUMBER_LEVEL_OFF 0
#define SERIAL_NUMBER_LEVEL_MAX 3
#define CFD_SCENE_COUNT 3
#define CFD_ITEM_COUNT 4
#define CFD_LEVEL_MIN 1
#define CFD_LEVEL_MAX 5
#define USER_PASSWORD_MAX_LEN 31

typedef struct {

    int mode;
    uint8_t speed;
    bool add_enable;
    bool start_auto;
    char password[USER_PASSWORD_MAX_LEN + 1];
    int batch_mode;
    uint8_t cfd_mode;
    uint8_t fo_mode;
    uint8_t work_mode;
    uint8_t language;
    uint32_t batch_amount;
    bool buzzer_enable;
    bool serial_num_enable;
    uint16_t last_total_pcs;
    uint32_t last_total_amount;
    uint32_t history_total_notes_counted;
    uint8_t print_space_top;
    char print_head1[PRINT_SETTING_HEAD_MAX_LEN + 1];
    char print_head2[PRINT_SETTING_HEAD_MAX_LEN + 1];
    uint8_t print_content;
    uint8_t print_space_bottom;
    uint8_t serial_number_level;
    char cfd_setting_currency[4];
    uint8_t cfd_levels[CFD_SCENE_COUNT][CFD_ITEM_COUNT];
}Machine_para_t;

extern Machine_para_t Machine_para;

bool user_cfg_password_load(void);
bool user_cfg_password_save(const char* password);
bool user_cfg_screenshot_load(void);
bool user_cfg_screenshot_save(bool enabled);
bool user_cfg_screenshot_enabled(void);

#define SENSOR_VOLTAGE_CH_NUM 11
typedef struct {
    uint8_t raw[SENSOR_VOLTAGE_CH_NUM];
    bool valid[SENSOR_VOLTAGE_CH_NUM];
    uint32_t update_count;
} sensor_voltage_t;

extern sensor_voltage_t g_sensor_voltage;

 enum {
    MODE_NONE,
    MODE_MDC,
    MODE_CNT,
    MODE_VER,
    MODE_SDC

};


void user_data_init(void);


#endif // !USER_CFG_H
