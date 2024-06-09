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
$(AR_DIR)/lib_usb_syn.a \
$(AR_DIR)/libqrcode.a \
$(AR_DIR)/libFFT_pi32v2_OnChip.a\
$(AR_DIR)/libAptFilt_fix_pi32v2_OnChip.a\
$(AR_DIR)/libEchoSuppress_pi32v2_OnChip.a\
$(AR_DIR)/libjlsp.a\
$(AR_DIR)/lib_dns.a\
$(AR_DIR)/dv17_aac_dec_lib.a \




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
$(AR_DIR)/avsdk.a  \




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


ifeq ($(CONFIG_MARVELL8782_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)marvell_8782.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa.a \

endif


ifeq ($(CONFIG_MARVELL8801_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)marvell_8801.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa.a \

endif

ifeq ($(CONFIG_RTL8188E_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)rtl8188e.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa.a \

endif

ifeq ($(CONFIG_RTL8192E_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)rtl8192e.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa.a \

endif

ifeq ($(CONFIG_RTL8192F_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)rtl8192f.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa.a \

endif

ifeq ($(CONFIG_RTL8821CS_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)rtl8821cs.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa.a \

endif

ifeq ($(CONFIG_RTL8822BS_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)rtl8822bs.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa.a \

endif

ifeq ($(CONFIG_S9082_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)9082.a  \
$(AR_DIR)/$(LIB_PREFIX)S9082_wifi_todo.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_wpa.a \

endif


ifeq ($(CONFIG_HI3881_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)Hi3881.a  \
$(AR_DIR)/$(LIB_PREFIX)hostapd_and_wpasupplicant_hi3881.a \

endif

ifeq ($(CONFIG_HI3861L_ENABLE),y)
LIBS += \
$(AR_DIR)/$(LIB_PREFIX)Hi3861L.a  \

endif






LIBS += \
--end-group







objs += \
$(ROOT)/apps/$(APP_CASE)/aecfix_test.o \
$(ROOT)/apps/$(APP_CASE)/mpu_test.o \
$(ROOT)/apps/$(APP_CASE)/app_database.o \
$(ROOT)/apps/$(APP_CASE)/app_main.o \
$(ROOT)/apps/$(APP_CASE)/backlight.o \
$(ROOT)/apps/$(APP_CASE)/device_mount.o \
$(ROOT)/apps/$(APP_CASE)/key_voice.o \
$(ROOT)/apps/$(APP_CASE)/parking_detect.o \
$(ROOT)/apps/$(APP_CASE)/upgrade.o \
$(ROOT)/apps/$(APP_CASE)/usb_app.o \
$(ROOT)/apps/$(APP_CASE)/usb_app_cfg.o \
$(ROOT)/apps/$(APP_CASE)/usb_host_user.o \
$(ROOT)/apps/$(APP_CASE)/user_isp_cfg.o \
$(ROOT)/apps/$(APP_CASE)/user_scr_cfg.o \
$(ROOT)/apps/$(APP_CASE)/user_scr_tool.o \
$(ROOT)/apps/$(APP_CASE)/version.o \
$(ROOT)/apps/$(APP_CASE)/video_audio.o\
$(ROOT)/apps/$(APP_CASE)/video_dec.o \
$(ROOT)/apps/$(APP_CASE)/video_dec_cfg.o \
$(ROOT)/apps/$(APP_CASE)/video_photo.o \
$(ROOT)/apps/$(APP_CASE)/video_photo_cfg.o \
$(ROOT)/apps/$(APP_CASE)/video_rec.o \
$(ROOT)/apps/$(APP_CASE)/video_rec_cfg.o \
$(ROOT)/apps/$(APP_CASE)/video_system.o \
$(ROOT)/apps/$(APP_CASE)/video_system_cfg.o \
$(ROOT)/apps/$(APP_CASE)/test_audio.o \
$(ROOT)/apps/$(APP_CASE)/play_usb_speaker.o \
$(ROOT)/apps/$(APP_CASE)/play_usb_mic.o \
$(ROOT)/apps/$(APP_CASE)/gsensor/da380.o \
$(ROOT)/apps/$(APP_CASE)/gsensor/gma301.o \
$(ROOT)/apps/$(APP_CASE)/gsensor/msa300.o \
$(ROOT)/apps/$(APP_CASE)/gsensor/stk8312.o \
$(ROOT)/apps/$(APP_CASE)/gsensor/gSensor_manage.o \
$(ROOT)/apps/$(APP_CASE)/touch_panel/touch_panel_manager.o \
$(ROOT)/apps/$(APP_CASE)/touch_panel/GT911.o \
$(ROOT)/apps/$(APP_CASE)/touch_panel/GT911_B.o \
$(ROOT)/apps/$(APP_CASE)/ui/ui_action_animation.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_LY/ui_action_sys.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_LY/ui_action_rec.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_LY/ui_action_photo.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_LY/ui_action_dec.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_LY/ui_action_usb.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_LY/ui_action_upd.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_JL02/ui_action_dec.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_JL02/ui_action_main.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_JL02/ui_action_photo.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_JL02/ui_action_rec.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_JL02/ui_action_sys.o \
$(ROOT)/apps/$(APP_CASE)/ui/style_JL02/ui_action_usb.o \

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
$(ROOT)/apps/$(APP_CASE)/wifi/spec_uart_test.o \
$(ROOT)/apps/$(APP_CASE)/wifi/ctp_cmd.o\
$(ROOT)/apps/$(APP_CASE)/wifi/hi3861L_task.o\
$(ROOT)/apps/$(APP_CASE)/wifi/net_video_rec.o\
$(ROOT)/apps/$(APP_CASE)/wifi/strm_video_rec.o\
$(ROOT)/apps/$(APP_CASE)/wifi/video_preview.o\
$(ROOT)/apps/$(APP_CASE)/wifi/video_playback.o\
$(ROOT)/apps/$(APP_CASE)/wifi/media_file_list.o\
$(ROOT)/apps/$(APP_CASE)/wifi/packet.o\
$(ROOT)/apps/$(APP_CASE)/wifi/simple_mov_unpkg.o\
$(ROOT)/apps/$(APP_CASE)/wifi/network_upgrade.o\
$(ROOT)/apps/$(APP_CASE)/wifi/rt_talk_net.o\
$(ROOT)/apps/$(APP_CASE)/wifi/rt_talk_voice.o\
$(ROOT)/apps/$(APP_CASE)/wifi/get_image_data.o\
$(ROOT)/apps/$(APP_CASE)/wifi/network_mssdp.o\
$(ROOT)/apps/$(APP_CASE)/wifi/stream/stream_core.o\
$(ROOT)/apps/$(APP_CASE)/wifi/stream/video_rt_rtsp.o\
$(ROOT)/apps/$(APP_CASE)/wifi/stream/video_rt_tcp.o\
$(ROOT)/apps/$(APP_CASE)/wifi/stream/video_rt_udp.o\
$(ROOT)/apps/$(APP_CASE)/wifi/stream/video_rt_usr.o\
$(ROOT)/apps/$(APP_CASE)/wifi/ftpd_vfs_reg.o\
$(ROOT)/apps/$(APP_CASE)/wifi/doorbell/doorbell_app.o\
$(ROOT)/apps/$(APP_CASE)/wifi/doorbell/doorbell_event.o\
$(ROOT)/apps/$(APP_CASE)/wifi/doorbell/irled_ctrl.o\
$(ROOT)/apps/$(APP_CASE)/wifi/cloud_interface/cloud_faststart.o\
$(ROOT)/apps/$(APP_CASE)/wifi/cloud_interface/cloud_net_config.o\
$(ROOT)/apps/$(APP_CASE)/wifi/cloud_interface/cloud_playback.o\
$(ROOT)/apps/$(APP_CASE)/wifi/cloud_interface/cloud_playback_list.o\
$(ROOT)/apps/$(APP_CASE)/wifi/cloud_interface/cloud_rt_talk.o\
$(ROOT)/apps/$(APP_CASE)/wifi/cloud_interface/cloud_sleep.o\
$(ROOT)/apps/$(APP_CASE)/wifi/cloud_interface/cloud_storage.o\
$(ROOT)/apps/$(APP_CASE)/wifi/cloud_interface/cloud_upgrade.o\
$(ROOT)/apps/$(APP_CASE)/wifi/cloud_interface/cloud_utc_time.o\
$(ROOT)/apps/$(APP_CASE)/wifi/avsdk/ut_dev_ipc_cmd.o\
$(ROOT)/apps/$(APP_CASE)/wifi/avsdk/ut_lowpower_cmd.o\
$(ROOT)/apps/$(APP_CASE)/wifi/avsdk/ut_xciot.o\





#在app_case加入board文件，后续改到case里面放board
objs += \
$(ROOT)/cpu/$(CPU)/board/board_dev_wifi_5701_20190809.o \
$(ROOT)/cpu/$(CPU)/board/board_dev_wifi_5702_20190809.o \
$(ROOT)/cpu/$(CPU)/board/board_dev_wifi_5712_20210316.o \
$(ROOT)/cpu/$(CPU)/board/board_dev_wifi_5713_20210317.o \
$(ROOT)/cpu/$(CPU)/board/board_doorbell_wifi_5713_20220325.o \
$(ROOT)/cpu/$(CPU)/board/board_dev_wifi_5713_20210901.o \













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
-I$(ROOT)/include_lib/net/server \
-I$(ROOT)/include_lib/server \
-I$(ROOT)/include_lib/device/cpu/$(CPU) \

#net
includes +=\
-I$(ROOT)/include_lib/net  \
-I$(ROOT)/include_lib/net/server  \
-I$(ROOT)/include_lib/net/avsdk  \
-I$(ROOT)/include_lib/net/wifi_module  \
-I$(ROOT)/include_lib/net/hostapdandwpa_supplicant  \
-I$(ROOT)/include_lib/net/device  \
-I$(ROOT)/include_lib/net/lwip/port/ \
-I$(ROOT)/include_lib/net/lwip/app/ \
-I$(ROOT)/include_lib/net/lwip/src/include  \
-I$(ROOT)/apps/$(APP_CASE)/wifi \
-I$(ROOT)/apps/$(APP_CASE)/wifi/cloud_interface \
-I$(ROOT)/apps/$(APP_CASE)/wifi/avsdk \
-I$(ROOT)/apps/$(APP_CASE)/wifi/include \
-I$(ROOT)/apps/$(APP_CASE)/wifi/wifi_car_camera/include \




