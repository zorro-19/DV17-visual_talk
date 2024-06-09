#include "system/includes.h"
#include "server/ui_server.h"
#include "server/video_server.h"
#include "server/video_engine_server.h"
#include "server/usb_server.h"
#include "video_rec.h"
#include "video_system.h"
#include "gSensor_manage.h"
#include "user_isp_cfg.h"


#include "action.h"
#include "style.h"
#include "app_config.h"
#include "asm/debug.h"
#include "vrec_osd.h"
#include "vrec_icon_osd.h"
#include "app_database.h"
#include "storage_device.h"
#include "asm/lcd_config.h"
#include "doorbell_event.h"

#include "video_photo.h"
#include "server/video_dec_server.h"

#if (APP_CASE == __WIFI_CAR_CAMERA__)
#include "net_config.h"
#include "net_video_rec.h"
#endif

#define MESSAGE_STATE_NO_DEV         0x0
#define MESSAGE_STATE_DEVICE_MOUNT   0x1

struct message_app_handle {
    u8 state;
    u8 mode;

//    struct server *usb_slave;
//    struct server *usb_host;
    struct server *ui;
    u8 *buf;
    u32 buf_size;
};

static struct message_app_handle handler = {
    .state = MESSAGE_STATE_NO_DEV,
};

#define __this  (&handler)

static int state_machine(struct application *app, enum app_state state, struct intent *it)
{

    switch (state) {
    case APP_STA_CREATE:
        memset(__this, 0x0, sizeof(*__this));
#ifdef CONFIG_UI_ENABLE
        __this->ui = server_open("ui_server", NULL);
        if (!__this->ui) {
            return -EFAULT;
        }
#endif
        break;
    case APP_STA_START:
        switch (it->action) {
        case ACTION_VIDEO_MESSAGE_MAIN:
//            usb_slave_start();
//            __this->mode = 0;
            break;
        case ACTION_VIDEO_MESSAGE_SET_CONFIG:
//            __this->mode = usb_app_set_config(it);
            break;
        case ACTION_VIDEO_MESSAGE_GET_CONFIG:
//            usb_app_get_config(it);
            break;
        }
        break;
    case APP_STA_PAUSE:
    /* return -EFAULT; */
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
//        usb_slave_stop();

        break;
    case APP_STA_DESTROY:
        if (__this->ui) {
            server_close(__this->ui);
        }
        break;
    }

    return 0;

}
static int message_app_key_event_handler(struct key_event *key)
{

    switch (key->event) {
    case KEY_EVENT_CLICK:
        switch (key->value) {
        case KEY_OK:
            //usb_app_select();
            break;
        case KEY_UP:

            break;
        case KEY_DOWN:

            break;
        default:
            break;
        }
        break;
    case KEY_EVENT_LONG:
        if (key->value == KEY_POWER) {
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

static int message_app_device_event_handler(struct sys_event *event)
{

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        break;
    case DEVICE_EVENT_ONLINE:
        break;
    case DEVICE_EVENT_OUT:
        //usb_app_pause();
        break;
    }
    return false;
}

static int event_handler(struct application *app, struct sys_event *event)
{

    switch (event->type) {
    case SYS_KEY_EVENT:
        return message_app_key_event_handler(&event->u.key);
    case SYS_DEVICE_EVENT:
        return message_app_device_event_handler(event);
    default:
        return false;
    }
    return false;
}


static const struct application_operation message_app_ops = {
    .state_machine  = state_machine,
    .event_handler  = event_handler,
};

REGISTER_APPLICATION(app_usb) = {
    .name 	= "message_app",
    .action	= ACTION_VIDEO_MESSAGE_MAIN,
    .ops 	= &message_app_ops,
    .state  = APP_STA_DESTROY,
};


