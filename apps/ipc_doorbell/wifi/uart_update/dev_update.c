//#include "dev_update.h"
//#include "dev_manager.h"
//#include "update/update.h"
//#include "update/update_loader_download.h"


#include "system/includes.h"
#include "server/video_server.h"
#include "server/video_dec_server.h"
#include "server/ui_server.h"
#include "action.h"
#include "style.h"
#include "app_config.h"
#include "storage_device.h"

#include "video_system.h"


#include "app_database.h"



#define DEV_UPDATE_EN		1
const char updata_file_name[] = "/*.UFW";

extern bool uart_update_send_update_ready(char *file_path);
extern bool get_uart_update_sta(void);
extern void storage_update_loader_download_init_with_file_hdl(
    int  type,
    char *update_path,
    void *fd,
    void (*cb)(void *priv, int type, u8 cmd),
    void *cb_priv,
    u8 task_en
);

static char update_path[48] = {0};
extern const char updata_file_name[];

struct __update_dev_reg {
    char *logo;
    int type;

};



static void dev_update_callback(void *priv, int type, u8 cmd)
{
//    struct __update_dev_reg *parm = (struct __update_dev_reg *)priv;
//    if (cmd == UPDATE_LOADER_OK) {
//        update_mode_api(type);
//    } else {
//        printf("update fail, cpu reset!!!\n");
//        cpu_reset();
//    }
}

static int uart_upg_handler=0;
static int exit_state=0;
void uart_open_file_uninit(void)
{

    printf("\n  kill_uart_open_file_uninit\n");
    exit_state = 1;
    thread_kill(&uart_upg_handler, KILL_WAIT);
}
void dev_update_check(char *path_name)
{


#if DEV_UPDATE_EN
#if  0

        //<尝试按照路径打开升级文件
//        char *updata_file = (char *)updata_file_name;  //const char updata_file_name[] = "/*.UFW";
//        if (*updata_file == '/') {
//            updata_file ++;
//        }
        memset(update_path, 0, sizeof(update_path));
//        sprintf(update_path, "%s%s", CONFIG_ROOT_PATH, updata_file);
//        printf("update_path:::::::::::::::::::::::::::::::: %s\n", update_path);
        struct vfscan *fs = fscan(CONFIG_ROOT_PATH, "-tUFW -sn -r");
        if(fs){
        char name[128] = {0};
        #define MAX_FILE_NAME_LEN       128
        FILE *file = fselect(fs, FSEL_FIRST_FILE, 0);
        int len = fget_name(file, name, MAX_FILE_NAME_LEN);

        printf("\n ==========name==========%s\n",name);
         sprintf(update_path, "%s%s", CONFIG_ROOT_PATH, name);
          printf("update_path:::::::::::::::::::::::::::::::: %s\n", update_path);

        }
       // FILE *fd = fopen(update_path, "r");
        FILE *fd = fopen(update_path, "r");
        if (!fd) {
            ///没有升级文件， 继续跑其他解码相关的流程
            printf("open update file err!!!\n");
            return ;
        }
        uart_update_send_update_ready(update_path);
#else
        char name[128] = {0};
        snprintf(name, sizeof(name), "%s%s", CONFIG_ROOT_PATH, path_name);

         printf("update_path:::::::::::::::::::::::::::::::: %s\n", name);


#endif
        uart_update_send_update_ready(name);
        while (get_uart_update_sta()) {
           // printf("\n u\n");

        if (exit_state) {
            exit_state = 0;
            break;
        }


        os_time_dly(10);

        uart_open_file_uninit();

        }

#endif

}

void dev_update_check_init(char *path_name){

  thread_fork("dev_update_check", 7, 512, 0, &uart_upg_handler, dev_update_check, path_name);

}



