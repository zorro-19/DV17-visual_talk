#ifndef __ASM_FBDEV_H
#define __ASM_FBDEV_H

#include "device/video.h"


#define FBDEV_FULL_SCREEN_FIRST 1
#define FBDEV_FULL_IMAGE_FIRST  2


void *video_fb_open(struct video_format *, int, int);
int video_fb_get_map(void *, struct fb_map_user *map);
int video_fb_put_map(void *, struct fb_map_user *map);
int video_fb_close(void *fb);

int video_fb_set_disp_win(void *_hdl, struct video_window *win);




#endif
