#include "sock_api/sock_api.h"
#include "server/rt_stream_pkg.h"   //head info .h
#include "server/video_server.h"//app_struct
#include "server/video_dec_server.h"//dec_struct
#include "server/ctp_server.h"//enum ctp_cli_msg_type
#include "action.h"//it.action = ACTION_NET_SCR...
#include "net_config.h"
/* #include "time_compile.h" */

/* #ifdef CONFIG_NET_CTP */

#define DEST_PORT 3333
#define CONN_PORT 2229
#define DEST_IP_SERVER "172.16.23.151"
#define HEAD_DATA 20
#define JPG_MAX_SIZE 200*1024
#define JPG_FPS 30
#define JPG_SRC_H 720
#define JPG_SRC_W 1280

struct __JPG_HW {
    u32 src_w;
    u32 src_h;
    u32 buf_len;
    u8  buf[];
};

struct __NET_CTP_INFO {
    u8 state;
    void *video_dec;
};

static struct __NET_CTP_INFO  net_ctp_info = {0};

int ctp_callback(void *hdl, enum ctp_cli_msg_type type, const char *topic, const char *parm_list, void *priv)
{
    const char topic_dat = *topic;
    const char parm_list_dat = *parm_list;
    //putchar('a');
    return 0;
}


int net_ctp_devinit(void)
{
    struct application *app = NULL;
    if (net_ctp_info.state) {
        printf("\n [WARING] %s multiple init\n", __func__);
        return 0 ;
    }
    struct video_dec_arg arg_recv = {0};
    arg_recv.dev_name = "video_dec";
    arg_recv.video_buf_size = JPG_MAX_SIZE;
    net_ctp_info.video_dec = server_open("video_dec_server", &arg_recv);
    if (net_ctp_info.video_dec == NULL) {
        printf("\n [ERROR] video_dec_server open err\n");
        goto EXIT;
    }
    app = get_current_app();
    printf("\n app-> name = %s\n", app->name);
    if (!app || (strcmp("video_rec", app->name) && strcmp("net_video_rec", app->name))) {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    net_ctp_info.state = 1;
    return 0;

EXIT:
    if (net_ctp_info.video_dec) {
        server_close(net_ctp_info.video_dec);
        net_ctp_info.video_dec = NULL;
    }
    return -1;
}


int net_ctp_dec(struct __JPG_HW *info, struct __NET_CTP_INFO *ctp_info)
{
    int err;
    if (!ctp_info->state) {
        printf("\n [WARING] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    union video_dec_req dec_req = {0};
    dec_req.dec.fb = "fb1";
    dec_req.dec.left = 0;
    dec_req.dec.top = 0;

    dec_req.dec.width = info->src_w;//后续需要居中显示
    dec_req.dec.height = info->src_h;
    dec_req.dec.preview = 1;
    /* printf("info->buf_len:%d  %d",info->buf_len,200 * 1024); */
    dec_req.dec.image.buf = info->buf;
    dec_req.dec.image.size = info->buf_len;
    err = server_request(ctp_info->video_dec, VIDEO_REQ_DEC_IMAGE, &dec_req);
    if (err) {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    return 0;
EXIT:
    return -1;
}


void thread_socket_recv(void *sockfd)
{
    struct frm_head *head = NULL;
    struct __JPG_HW *jpg_recv = (struct __JPG_HW *) calloc(1, sizeof(struct __JPG_HW) + sizeof(u8) * (JPG_MAX_SIZE)); // no free
    if (!jpg_recv) {
        printf("\n[ERROR]  jpg calloc \n");
    }
    jpg_recv->src_w = JPG_SRC_W;
    jpg_recv->src_h = JPG_SRC_H;
    memset(jpg_recv->buf, 0, JPG_MAX_SIZE);

    char buf_head[HEAD_DATA] = {0};
    int ret, dev_ret;

    printf("\n-----  Enter_thread_recv -----  \n");
    int net_hide_main_ui(void);
    int net_video_disp_start(int id);
    /* net_hide_main_ui(); */
    /* net_video_disp_start(0); */

    extern int net_video_disp_stop(int id);
    extern int net_hide_main_ui(void);
    net_video_disp_stop(0);
    net_hide_main_ui();


    //u32 opt = 1;
    //setsockopt((int)sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    //sock_setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    while (1) {

        msleep(1);//delete some printf
        ret = sock_recv(sockfd, buf_head, HEAD_DATA, MSG_WAITALL);
        msleep(10);
        if (ret) {

            head = (struct frm_head *)buf_head;// Forced Conversion
            if (sock_recv(sockfd, jpg_recv->buf, head->frm_sz, MSG_WAITALL)) {
                msleep(10);

                put_buf(jpg_recv->buf, 32);
                switch (head->type) {
                case JPEG_TYPE_VIDEO :
                    jpg_recv->buf_len = head->frm_sz;
                    dev_ret = net_ctp_dec(jpg_recv, &net_ctp_info);
                    if (dev_ret != 0) {
                        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
                        goto EXIT;
                    }
                    break;
                case PCM_TYPE_AUDIO:
                    break;
                default:
                    break;
                }
                head->type = 0;
            } else {
                printf("\nErr ret sock_recv - data\n");
                goto EXIT;
            }

        } else {
            printf("\n-------recv fail --------\n");
            goto EXIT;
        }
    }
EXIT:
    free(jpg_recv);
    printf("\n\n\n----free jpg_recv-----\n\n\n");
    ctp_cli_uninit();
    extern int net_video_disp_start(int id);
    extern int net_show_main_ui(void);
    net_video_disp_start(0);
    net_show_main_ui();
}

void net_ctp_recv(void)
{
    void  *sockfd = NULL;
    struct sockaddr_in conn_addr;
    int ret;
    u32 pid;

    conn_addr.sin_family = AF_INET;
    conn_addr.sin_addr.s_addr = inet_addr(DEST_IP_SERVER) ;
    conn_addr.sin_port = htons(CONN_PORT);
    sockfd = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);
    //SOCK_STREAM  表示的是创建一个TCP的socket  然后对应的回调函数connect_callback表示在这条socket上的一些操作都会调用回调函数
    if (sockfd == NULL) {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
    }

    if (sock_connect(sockfd, &conn_addr, sizeof(struct sockaddr)) == -1) {
        printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
    } else {
        ret = net_ctp_devinit();
        if (ret == 0) {
            printf("\n-----net-ctp-dev-init-Sueccess\n");
            if ((thread_fork("thread_socket_recv", 10, 300 * 1024, 0, &pid, thread_socket_recv, sockfd))) {
                printf("\n [ERROR] %s - %d\n", __FUNCTION__, __LINE__);
            }
        } else {
            printf("\n[ERR]- net-scr-devinit\n");
        }

    }

}


void *net_ctp_init(void)
{
    void *sockfd = NULL;
    struct sockaddr_in dest_addr;
    const char topic_1[] = {"APP_ACCESS"};
    //APP每次建立连接，必须发送此Topic给设备，设备根据当前情况允许或拒绝连接，当APP接入设备之后，设备会主动将心跳包、白平衡、SD卡状态、电池状态等信息发送给app,app根据这些信息进行界面初始化
    const char topic_2[] = {"DATE_TIME"};
    //获取当前设备时间
    const char topic_3[] = {"OPEN_RT_STREAM"};

//    const char content_test[]={"{.  \"op\" : \"PUT\",.  \"param\" : {.    \"type\" : \"1\",.    \"ver\" : \"1.0\".  }.}"};   //去掉'.',不合法的字符，命令能识别回车和空格。
    const char content_1[] = {"{\"op\":\"PUT\",\"param\":{\"type\":\"1\",\"ver\":\"1.0\"}}"};               //实验命令成功
    const char content_2[] = {"{\"op\":\"PUT\",\"param\":{\"rate\":\"8000\",\"w\":\"1280\",\"fps\":\"30\",\"h\":\"720\",\"format\":\"0\"}}"};

    memset(&dest_addr, 0x0, sizeof(dest_addr));                         //对server_info初始化操作，全部清零
    ctp_cli_init();

    /* char *Str_time; */
    /* sprintf(&Str_time, "{\"op\":\"PUT\",\"param\":{\"date\":\"%d%02d%02d%02d%02d%02d\"}}",YEAR,MONTH,DAY,HOUR,MINUTE,SECOND); */

    /* printf("\n\n DATA - %d:%d:%d  - %d:%d:%d \n\n",YEAR,MONTH,DAY,HOUR,MINUTE,SECOND); */

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(DEST_IP_SERVER) ;
    dest_addr.sin_port = htons(DEST_PORT);
    sockfd = ctp_cli_reg(1, &dest_addr, ctp_callback, NULL, "KEEP_ALIVE_INTERVAL", 10000);
    //sockfd = sock_reg(AF_INET, SOCK_STREAM, 0, NULL, NULL);         //类似句柄...代码成功连接

    if (sockfd == NULL) {
        printf("\n--------------CTP connect failed----------------\n");
    } else {
        printf("\n--------------CTP connect success---------------\n");
    }

    if (ctp_cli_send(sockfd, &topic_1, &content_1) == 0) {
        /* if(ctp_cli_send(sockfd,&topic_2,&Str_time)==0) */
        /* { */
        net_ctp_recv();
        if (ctp_cli_send(sockfd, &topic_3, &content_2) == 0) {

        } else {
            printf("\n------------------send error3-------------------\n");
        }
        /* } */
        /* else printf("\n------------------send error2-------------------\n"); */
    } else {
        printf("\n------------------send error1-------------------\n");
    }

    return sockfd;
}

/* #endif */
