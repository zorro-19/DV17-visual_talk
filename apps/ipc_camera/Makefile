
export LIBS = \
--start-group \
$(AR_DIR)/$(LIB_PREFIX)common.a \
$(AR_DIR)/$(LIB_PREFIX)rtos.a \
$(AR_DIR)/$(LIB_PREFIX)sdx.a  \
$(AR_DIR)/$(LIB_PREFIX)res.a  \
$(AR_DIR)/$(LIB_PREFIX)usb.a \
$(AR_DIR)/$(LIB_PREFIX)fs.a  \
$(AR_DIR)/$(LIB_PREFIX)upgrade.a \
$(AR_DIR)/$(LIB_PREFIX)font.a \
$(AR_DIR)/$(LIB_PREFIX)server_core.a  \
$(AR_DIR)/$(LIB_PREFIX)ui_server.a  \
$(AR_DIR)/$(LIB_PREFIX)video_engine_server.a  \
$(AR_DIR)/$(LIB_PREFIX)usb_server.a  \
$(AR_DIR)/$(LIB_PREFIX)video_dec_server.a  \
$(AR_DIR)/$(LIB_PREFIX)video_rec_server.a  \
$(AR_DIR)/$(LIB_PREFIX)audio_server.a  \
$(AR_DIR)/$(LIB_PREFIX)vm.a  \
$(AR_DIR)/$(LIB_PREFIX)dlmalloc.a  \
$(AR_DIR)/$(LIB_PREFIX)h264.a  \
$(AR_DIR)/$(LIB_PREFIX)jpeg.a  \
--whole-archive \
$(AR_DIR)/$(LIB_PREFIX)dev.a  \
$(AR_DIR)/$(LIB_PREFIX)ui.a \
$(AR_DIR)/$(LIB_PREFIX)video_framework.a  \
$(AR_DIR)/$(LIB_PREFIX)sys.a \
$(AR_DIR)/$(LIB_PREFIX)cpu.a  \
$(AR_DIR)/$(LIB_PREFIX)isp.a  \
--no-whole-archive \
$(AR_DIR)/$(LIB_PREFIX)log.a  \

#增加算法提供的库
LIBS +=\
$(AR_DIR)/cpr.a  \
$(AR_DIR)/rs_ecc.a \
$(AR_DIR)/aac_enc_lib.a \
$(AR_DIR)/dv17_aac_dec_lib.a \
$(AR_DIR)/libqrcode.a \
$(AR_DIR)/lib_usb_syn.a \

#增加网络库
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)hsm.a  \
$(AR_DIR)/$(LIB_PREFIX)sdio_card.a \
$(AR_DIR)/$(LIB_PREFIX)wifi_dev.a  \
$(AR_DIR)/$(LIB_PREFIX)mbedtls.a\
--whole-archive \
$(AR_DIR)/$(LIB_PREFIX)lwip.a  \
--no-whole-archive \
$(AR_DIR)/$(LIB_PREFIX)json.a  \
$(AR_DIR)/$(LIB_PREFIX)net_server.a  \
$(AR_DIR)/$(LIB_PREFIX)http_server.a \
$(AR_DIR)/$(LIB_PREFIX)http_cli.a \
$(AR_DIR)/$(LIB_PREFIX)iperf.a\
$(AR_DIR)/$(LIB_PREFIX)rtsp.a \
$(AR_DIR)/$(LIB_PREFIX)ftpd.a \
$(AR_DIR)/$(LIB_PREFIX)curl.a \
$(AR_DIR)/$(LIB_PREFIX)tutk.a \




ifeq ($(CONFIG_RTL8189E_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)rtl8189e.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa.a \

endif

ifeq ($(CONFIG_RTL8189F_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)rtl8189f.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa.a \

endif



ifeq ($(CONFIG_HI3881_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)hi3881.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa_hi3881.a \

endif







LIBS += \
--end-group







objs += \
$(ROOT)/apps/$(APP_CASE)/app_database.o \
$(ROOT)/apps/$(APP_CASE)/app_main.o \
$(ROOT)/apps/$(APP_CASE)/backlight.o \
$(ROOT)/apps/$(APP_CASE)/device_mount.o \
$(ROOT)/apps/$(APP_CASE)/key_voice.o \
$(ROOT)/apps/$(APP_CASE)/parking_detect.o \
$(ROOT)/apps/$(APP_CASE)/upgrade.o \
$(ROOT)/apps/$(APP_CASE)/usb_app.o \
$(ROOT)/apps/$(APP_CASE)/usb_app_cfg.o \
$(ROOT)/apps/$(APP_CASE)/user_isp_cfg.o \
$(ROOT)/apps/$(APP_CASE)/video_rec.o \
$(ROOT)/apps/$(APP_CASE)/video_rec_cfg.o \
$(ROOT)/apps/$(APP_CASE)/video_system.o \
$(ROOT)/apps/$(APP_CASE)/video_system_cfg.o \

#net
objs +=\
$(ROOT)/apps/$(APP_CASE)/wifi/wifi-tools/WL_EFUSE.o \
$(ROOT)/apps/$(APP_CASE)/wifi/wifi-tools/WL_AP_CONF.o \
$(ROOT)/apps/$(APP_CASE)/wifi/wifi-tools/WL_STA_CONF.o \
$(ROOT)/apps/$(APP_CASE)/wifi/wifi-tools/ap_config.o \
$(ROOT)/apps/$(APP_CASE)/wifi/wifi-tools/iperf_test.o\
$(ROOT)/apps/$(APP_CASE)/wifi/wifi-tools/RTL_PHY_REG_PG.o\
$(ROOT)/apps/$(APP_CASE)/wifi/wifi-tools/rtw_cache.o\
$(ROOT)/apps/$(APP_CASE)/wifi/wifi-tools/uaputl_conf.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/ctp_cmd.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/event.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/get_image_data.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/media_file_list.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/recover_file.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/step_motor.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/tutk_platform.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/wifi_app_task.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/wire_app_task.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/lte_app_task.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/stream/video_rt_tutk.o\
$(ROOT)/apps/$(APP_CASE)/wifi/tutk/curl_test.o\






#在app_case加入board文件，后续改到case里面放board
objs += \
$(ROOT)/cpu/$(CPU)/board/board_dev_wifi_5701_20190809.o \
$(ROOT)/cpu/$(CPU)/board/board_dev_wifi_5702_20190809.o \
$(ROOT)/cpu/$(CPU)/board/board_dev_wifi_5712_20210316.o \
$(ROOT)/cpu/$(CPU)/board/board_dev_wifi_5713_20210317.o \












includes += \
-I$(ROOT)/apps/$(APP_CASE)/include \
-I$(ROOT)/include_lib/ \
-I$(ROOT)/include_lib/utils \
-I$(ROOT)/include_lib/utils/ui \
-I$(ROOT)/include_lib/driver/device \
-I$(ROOT)/include_lib/driver/device/video/ \
-I$(ROOT)/apps \
-I$(ROOT)/apps/$(APP_CASE) \
-I$(ROOT)/apps/common/net/ \
-I$(ROOT)/include_lib/net \
-I$(ROOT)/include_lib/net/curl/include \
-I$(ROOT)/include_lib/net/server \
-I$(ROOT)/include_lib/server \
-I$(ROOT)/include_lib/device/cpu/$(CPU) \

#net
includes +=\
-I$(ROOT)/include_lib/net  \
-I$(ROOT)/include_lib/net/server  \
-I$(ROOT)/include_lib/net/wifi_module  \
-I$(ROOT)/include_lib/net/hostapdandwpa_supplicant  \
-I$(ROOT)/include_lib/net/device  \
-I$(ROOT)/include_lib/net/lwip/port/ \
-I$(ROOT)/include_lib/net/lwip/app/ \
-I$(ROOT)/include_lib/net/lwip/src/include  \
-I$(ROOT)/include_lib/net/tutk/  \
-I$(ROOT)/include_lib/net/tutk/P2PCam  \
-I$(ROOT)/apps/$(APP_CASE)/wifi \
-I$(ROOT)/apps/$(APP_CASE)/wifi/tutk/include \





