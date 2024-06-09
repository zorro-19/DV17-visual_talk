/*************************************************************************
	> File Name: video_engine.h
	> Author:
	> Mail:
	> Created Time: Tue 17 Jan 2017 12:03:57 PM HKT
 ************************************************************************/

#ifndef _FISH_UNDISTORT_H
#define _FISH_UNDISTORT_H
#include "typedef.h"
#include "device/device.h"
#include "ioctl.h"

typedef struct {
    int src_width;
    int src_height;
    int src_stride;
    int dst_width;
    int dst_height;
    int dst_stride;
    void *src_img;
    void *grid_info;
    void *dst_img;
    u8 blocksize;
    u8 ydef;
} fec_param_t;


enum {
    FISH_BLOCK_16,
    FISH_BLOCK_8,
};
#define FEC_SET_RUN 			_IOW('W', 0, sizeof(int))

int fec_perspective_transform(unsigned char *src, int sw, int sh, int ss, unsigned char *dst, int dw, int dh, int ds, int blocksize, unsigned char pdef, float m [3][3], short *gridbuf, int gridbuf_size);

#endif
