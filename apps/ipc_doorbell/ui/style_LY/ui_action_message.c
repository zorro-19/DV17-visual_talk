#include "ui/includes.h"
#include "system/includes.h"
#include "server/ui_server.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "app_database.h"
#include "server/audio_server.h"
#include "server/video_dec_server.h"
#include "asm/lcd_config.h"

#define _STDIO_H_
#include "time.h"
#include "xciot_api.h"


#ifdef CONFIG_UI_STYLE_LY_ENABLE

extern int sys_cur_mod;  /* 1:rec, 2:tph, 3:dec, 4:audio, 5:music */

#define STYLE_NAME  LY

struct message_menu_info {
    volatile char if_in_rec;    /* 是否正在录像 */
    u8 lock_file_flag;          /* 是否当前文件被锁 */
    u8 sd_write_err;			/* 写卡错误 */

    u8 menu_status;             /*0 menu off, 1 menu on*/
    u8 battery_val;
    u8 battery_char;
    u8 hlight_show_status;  /* 前照灯显示状态 */

    int vanish_line;
    int car_head_line;

    int remain_time;
};

static struct message_menu_info handler = {0};
#define __this 	(&handler)
#define sizeof_this     (sizeof(struct video_menu_info))

static struct server *audio = NULL;
static FILE *file;
static int rec_time = 0;



#endif
