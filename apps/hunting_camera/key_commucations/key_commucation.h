
#ifndef _MCU_COMMUCATION_WORK_
#define _MCU_COMMUCATION_WORK_

#include "system/includes.h"
#include "app_config.h"

/*
 DV17   PH11    PH1

工作模式            拨码到最左端
菜单模式            拨码到中间
关机模式             拨码到最右边
*/

#define MENU_ON_SET_GPIO                IO_PORT_PR_01
#define BL_OFF_GPIO                     IO_PORT_PR_02

#define LCD_DISP_BUSY_GPIO             IO_PORTH_11//    SETUP_MODE
#define WORKING_MODE_GPIO              IO_PORTH_10//    ON_MODE

#define WORKING_LED_GPIO              IO_PORTB_11//    LED

#define MENU_SETTING_CMD 0xA
#define STANDBY_CMD      0xB
#define RESP_POWER_OFF   0xF
#define RESP_ERROR_CMD   0xE


#define RESP_CLOSE_EVERYTIME_CMD   0x3





#endif

