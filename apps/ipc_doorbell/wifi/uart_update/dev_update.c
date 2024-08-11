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

#ifdef  USER_UART_UPDATE_ENABLE

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

        char name[128] = {0};
        snprintf(name, sizeof(name), "%s%s", CONFIG_ROOT_PATH, path_name);

         printf("update_path:::::::::::::::::::::::::::::::: %s\n", name);


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
#endif


