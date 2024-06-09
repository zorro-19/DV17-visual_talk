
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
$(AR_DIR)/$(LIB_PREFIX)tensorflow-microlite.a \
$(AR_DIR)/$(LIB_PREFIX)sys.a \
$(AR_DIR)/$(LIB_PREFIX)log.a \
$(AR_DIR)/$(LIB_PREFIX)dlmalloc.a  \
$(AR_DIR)/$(LIB_PREFIX)jpeg.a  \
$(AR_DIR)/$(LIB_PREFIX)h264.a  \
--whole-archive \
$(AR_DIR)/$(LIB_PREFIX)video_framework.a  \
$(AR_DIR)/$(LIB_PREFIX)dev.a  \
$(AR_DIR)/$(LIB_PREFIX)ui.a \
$(AR_DIR)/$(LIB_PREFIX)cpu.a  \
$(AR_DIR)/$(LIB_PREFIX)isp.a  \
$(AR_DIR)/$(LIB_PREFIX)_usb_syn.a \
--no-whole-archive \



#增加算法提供的库
LIBS += \
$(AR_DIR)/cpr.a  \
$(AR_DIR)/rs_ecc.a \
$(AR_DIR)/aac_enc_lib.a \
$(AR_DIR)/libFFT_pi32v2_OnChip.a \
$(AR_DIR)/libAptFilt_fix_pi32v2_OnChip.a \
$(AR_DIR)/libEchoSuppress_pi32v2_OnChip.a \
$(AR_DIR)/lib_dns.a \
$(AR_DIR)/libjlsp.a \
$(AR_DIR)/dv17_aac_dec_lib.a \

# $(AR_DIR)/orig_m4a_dec_lib.a \


LIBS += \
--end-group



objs += \
$(ROOT)/apps/$(APP_CASE)/auto_test.o \
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
$(ROOT)/apps/$(APP_CASE)/user_isp_cfg.o \
$(ROOT)/apps/$(APP_CASE)/user_scr_cfg.o \
$(ROOT)/apps/$(APP_CASE)/user_scr_draw.o \
$(ROOT)/apps/$(APP_CASE)/user_scr_tool.o \
$(ROOT)/apps/$(APP_CASE)/video_audio.o\
$(ROOT)/apps/$(APP_CASE)/video_dec.o \
$(ROOT)/apps/$(APP_CASE)/video_dec_cfg.o \
$(ROOT)/apps/$(APP_CASE)/video_photo.o \
$(ROOT)/apps/$(APP_CASE)/video_photo_cfg.o \
$(ROOT)/apps/$(APP_CASE)/video_rec.o \
$(ROOT)/apps/$(APP_CASE)/video_rec_cfg.o \
$(ROOT)/apps/$(APP_CASE)/video_system.o \
$(ROOT)/apps/$(APP_CASE)/video_system_cfg.o \
$(ROOT)/apps/$(APP_CASE)/usb_cam_h264.o \
$(ROOT)/apps/$(APP_CASE)/test_audio.o \
$(ROOT)/apps/$(APP_CASE)/aecfix_test.o \
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
$(ROOT)/apps/$(APP_CASE)/usb_host_user.o \
$(ROOT)/apps/$(APP_CASE)/play_usb_mic.o \
$(ROOT)/apps/$(APP_CASE)/play_usb_speaker.o \


#增加板级文件
objs += \
$(ROOT)/cpu/$(CPU)/board/board_dev_5701_20190809.o \
$(ROOT)/cpu/$(CPU)/board/board_dev_5701C_20191024.o \
$(ROOT)/cpu/$(CPU)/board/board_dev_5701_20190809_uvc_slave.o \





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


#include $(ROOT)/apps/$(APP_CASE)/tflite_micro_demo/Makefile


