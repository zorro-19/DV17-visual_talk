#ifndef UI_WATCH_H
#define UI_WATCH_H


#include "ui/control.h"
#include "ui/ui_core.h"


#define WATCH_CHILD_NUM 	(CTRL_WATCH_CHILD_END - CTRL_WATCH_CHILD_BEGIN)


struct watch_pic_info {
    struct ui_ctrl_info_head head;
    u16 cent_x;
    u16 cent_y;
    struct ui_image_list *img;
};


struct ui_watch {
    struct element elm;
    struct element child_elm[WATCH_CHILD_NUM];
    u8 hour;
    u8 min;
    u8 sec;
    u8 updata;
    void *timer;
    const struct layout_info *info;
    const struct watch_pic_info *pic_info[WATCH_CHILD_NUM];
    const struct element_event_handler *handler;
};

int ui_watch_set_time_by_id(int id, int hour, int min, int sec);













#endif

