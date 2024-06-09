
#ifndef _ABNORMAL_OFF_H_
#define _ABNORMAL_OFF_H_

#include <stdarg.h>
#include "generic/typedef.h"
#include "os/os_api.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/prot/ethernet.h"
#include <streaming_media_server/fenice_config.h>

typedef struct _FENICE_BANORMAL_OFF {
    unsigned char flag;
    unsigned char pend_flag;
    unsigned int accept_num;
    OS_SEM sem;
    OS_SEM accept_sem;
    OS_MUTEX mutex;
    OS_MUTEX select_mutex;
} FENICE_BANORMAL_OFF;


typedef struct _FENICE_TRANSFER {
    unsigned int type;  /* app层控制采用的协议类型--TCP/默认 */
    unsigned int port;  /* app层配置的端口号 */
    int (*exit)(void);  /* 关闭底层硬件 */
    int (*setup)(void);  /* 开启底层硬件 */
    int (*get_video_info)(struct fenice_source_info *info);/*获取配置视频相关参数*/
    int (*get_audio_info)(struct fenice_source_info *info);/*获取配置音频参数*/
    int (*set_media_info)(struct fenice_source_info *info);/*设置前后视：0前视，1后视*/
} FENICE_TRANSFER;

#endif

