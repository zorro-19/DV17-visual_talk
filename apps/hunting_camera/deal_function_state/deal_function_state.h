#ifndef __DEAL_POWER_STATE_H_

#define __DEAL_POWER_STATE_H_




#include "system/includes.h"
#include "server/video_server.h"


#include "system/includes.h"
#include "server/ui_server.h"
#include "action.h"
#include "ani_style.h"
#include "style.h"
#include "res_ver.h"
#include "app_config.h"
#include "gSensor_manage.h"
#include "video_rec.h"
#include "asm/rtc.h"
#include "storage_device.h"
#include "system/spinlock.h"
#include "asm/imc_driver.h"

#include "key_commucation.h"
#include "app_ui_index.h"
#include "isp_effect_data.h"
#define LOG_TAG "app_main"
#include "generic/log.h"
#if (APP_CASE == __WIFI_CAR_CAMERA__)
#include "server/ctp_server.h"
#include "server/net_server.h"
#include "net_config.h"
#endif
#if (APP_CASE == __WIFI_IPCAM__)
#include "device/wifi_dev.h"
#endif





extern  void  power_off();
extern void close_pir_power_off();
extern  void special_power_off_play_end(void *_ui);
extern void  set_dec_to_disp_poweroff(u8 is);
extern int get_dec_to_disp_poweroff_flag();
extern  void analyize_poweron_state();
#endif

