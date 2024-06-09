#include "device/uart.h"
#include "os/os_compat.h"
#include "app_core.h"
#include "action.h"
/*****************************************************************************
 Description:特殊串口初始化函数
******************************************************************************/
static void *uart_dev_handle;

int spec_uart_recv(char *buf, u32 len)
{
    return dev_read(uart_dev_handle, buf, len);
}

int spec_uart_send(char *buf, u32 len)
{
    return dev_write(uart_dev_handle, buf, len);
}

extern u8 simulate_state;
//void video_standby_post_msg_async(const char *msg, ...)
//{
//    if(msg){
//        printf("/**** [%s] %s\n",__func__,msg);
//    }
//    else{
//        printf("/**** post null\n");
//    }
//
//#ifdef CONFIG_UI_ENABLE
//        union uireq req;
//        va_list argptr;
//
//        va_start(argptr, msg);
//
//        if (__this->ui) {
//            req.msg.receiver = ID_WINDOW_MAIN_AND_REC;
//            req.msg.msg = msg;
//            req.msg.exdata = argptr;
//
//            server_request_async(__this->ui, UI_REQ_MSG, &req);
//        }
//        else{
//            printf("/****** post err\n");
//        }
//
//        va_end(argptr);
//
//#endif
//}
static int simulate_app_accept_call(void)
{
    struct intent it;
    printf("[%s] %d\n",__func__,__LINE__);

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_CHANGE_WORK;
    it.data = "app_accept_call";
    int err = start_app_async(&it, NULL, NULL);
    if (err) {
        printf("msg change err! %d\n", err);
    }

    return 0;
}

static int simulate_app_close_call(void)
{
    struct intent it;
    printf("[%s] %d\n",__func__,__LINE__);

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_CHANGE_WORK;
    it.data = "app_close_call";
    int err = start_app_async(&it, NULL, NULL);
    if (err) {
        printf("msg change err! %d\n", err);
    }

    return 0;
}

static int simulate_app_refuse_calling(void)
{
    struct intent it;
    printf("[%s] %d\n",__func__,__LINE__);

    init_intent(&it);
    it.name	= "video_rec";
    it.action = ACTION_VIDEO_REC_CHANGE_WORK;
    it.data = "app_refuse_calling";
    int err = start_app_async(&it, NULL, NULL);
    if (err) {
        printf("msg change err! %d\n", err);
    }

    return 0;
}

extern u8 contact_num ;
void spec_uart_test_thread(void *priv)
{
    static char uart_circlebuf[1 * 1024] __attribute__((aligned(32))); //串口循环数据buf,根据需要设置大小

    int parm;

    uart_dev_handle = dev_open("uart3", 0);
    ASSERT(uart_dev_handle != NULL, "open uart err");
    dev_ioctl(uart_dev_handle, UART_SET_CIRCULAR_BUFF_ADDR, (int)uart_circlebuf);


    parm = sizeof(uart_circlebuf);

    dev_ioctl(uart_dev_handle, UART_SET_CIRCULAR_BUFF_LENTH, (int)&parm);


#if 0 // 是否设置为 接收完指定长度数据, spec_uart_recv才出来
    parm = 1;
    dev_ioctl(uart_dev_handle, UART_SET_RECV_ALL, (int)&parm);
#endif

#if 1 // 是否设置为阻塞方式读
    parm = 1;
    dev_ioctl(uart_dev_handle, UART_SET_RECV_BLOCK, (int)&parm);
#endif

    dev_ioctl(uart_dev_handle, UART_START, (int)0);

    spec_uart_send("/**** uart3 ready\n",18);

    printf("/**** uart3 ready\n");
    while (1) {
#if 1
        int ret;
        char c;
        ret = spec_uart_recv(&c, sizeof(c));
//        if (ret > 0) {
//            if (c == 'a') {
//                cpu_reset();
//            }
//        }
        if (ret > 0) {
//            simulate_state = c-'0';
            switch(c){
//            case '0':
//                extern u8 simulate_state;
//                simulate_state = 1;
//                spec_uart_send("receive 0\n",10);
//                notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"network_connect_start",0);
//                break;
//            case '1':
//                spec_uart_send("receive 1\n",10);
//                notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"network_connecting",0);
//                break;
//            case '2':
//                spec_uart_send("receive 2\n",10);
//                notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"connect_succ",0);
//                break;
//            case '3':
//                spec_uart_send("receive 3\n",10);
//                notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"network_connect_start",0);
//                break;
//            case '4':
//                spec_uart_send("receive 4\n",10);
//                notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"network_connecting",0);
//                break;
//            case '5':
//                spec_uart_send("receive 5\n",10);
//                notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"connect_fail",0);
//                break;
            case '6':
                spec_uart_send("receive 6\n",10);
                simulate_state = 2;
//                notify_msg_to_video_rec(ACTION_VIDEO_DISTRIBUTE_NETWORK,"connect_fail",0);
                break;
//            case '1':
////                video_standby_post_msg_async("app_accept_call");
//                simulate_app_accept_call();
//                spec_uart_send("receive 1\n",10);
//                break;
//            case '2':
//                simulate_app_close_call();
//                spec_uart_send("receive 2\n",10);
//                break;
//            case '3':
//                simulate_app_refuse_calling();
//                spec_uart_send("receive 3\n",10);
//                break;
//            case '4':
//                contact_num = 1;
//                spec_uart_send("receive 4\n",10);
//                break;
//            case '5':
//                contact_num = 3;
//                spec_uart_send("receive 5\n",10);
//                break;
            }
        }

#endif

#if 0
        char buf[200];
        strcpy(buf, "spec_uart_test\n");
        spec_uart_send(buf, strlen(buf));
#endif

        os_time_dly(50);
    }
}

int spec_uart_test_main(void)
{
    return thread_fork("spec_uart_test_main", 2, 0x1000, 0, 0, spec_uart_test_thread, NULL);
}
