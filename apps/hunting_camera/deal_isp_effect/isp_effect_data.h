#ifndef __ISP_EFFECT_DATA_H_

#define __ISP_EFFECT_DATA_H_




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






extern void  isp_effect_start(int fd);
//extern unsigned char outdoor[2067];
//extern unsigned char indoor[2067];
//extern unsigned char night_data[2067];




#endif

