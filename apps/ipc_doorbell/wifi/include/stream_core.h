#ifndef __STREAM_CORE__H
#define __STREAM_CORE__H

#include "generic/typedef.h"
#include "net_config.h"

struct net_video_stream_sub {

    const char *name;

    void *(*open)(const char *path, const char *mode);

    int (*write)(void *file, void *buf, u32 len, u8 type);

    int (*ctrl)(void *file, u32 cmd, u32 arg);

    void (*close)(void *file);

    void *private_data;

};

#define REGISTER_NET_VIDEO_STREAM_SUDDEV(dev) \
	static struct net_video_stream_sub dev sec(.net_video_stream)



extern struct net_video_stream_sub net_video_stream_sub_begin[];

extern struct net_video_stream_sub net_video_stream_sub_end[];


void *stream_open(const char *path, const char *mode);

int stream_write(void *file, void *buf, u32 len, u32 type);

int stream_ctrl(void *file, u32 cmd, u32 arg);

void stream_close(void *file);







#endif


