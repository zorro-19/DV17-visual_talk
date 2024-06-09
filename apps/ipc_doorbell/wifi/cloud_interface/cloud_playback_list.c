#define _STDIO_H_
#include "xciot_api.h"
#include "xciot_cmd_ipc.h"

#include "app_config.h"
#include "json_c/json.h"
#include "fs.h"
#include "generic/list.h"
#include "simple_mov_unpkg.h"
#include "time.h"
#include "system/init.h"
#include "os/os_compat.h"
#include "doorbell_event.h"
#include "system/event.h"


#define CONFIG_PLAYBACK_LIST_ENABLE  1



static char list_path[][20] = {
    "DCIM/1/",
    "DCIM/2/",
    "DCIM/3/",
};


static const char g_file_name_prefix[][20] = {
    "KEY_",
    "PIR_",
    "MSG_",
};
#define FILE_NAME_PREFIX_LEN   4;

typedef struct {
    struct list_head root_head;
    int total_video_cnt;
    int ro_video_cnt;
}LIST_ROOT;


LIST_ROOT list_root_info  = {0};

//下标表示第几天
typedef struct {
   struct list_head t_head; //时的根节点
   int day_video_cnt;
}DAY_LIST_INFO; //所有的视频具体挂到每一天上


//下标表示第几月
typedef struct {
   DAY_LIST_INFO day[31];
}MON_LIST_INFO;

typedef struct{
   struct list_head entry;
   MON_LIST_INFO mon[12];
   u32 year;
}YEAR_LIST_INFO;



typedef struct { //时的信息
    struct list_head entry;
    u8 dir[16];//文件夹序号
    char name[32];//文件名
    int32_t day;//用于检索文件名
    uint64_t start_time;
    u32      length;
    u8 used;
    u8 ro_attr;
//    uint64_t file_id;
//    uint64_t thum_fig;

} CLOUD_PLAYBACK_LIST_INFO;


static YEAR_LIST_INFO  year_list_info[24];
static CLOUD_PLAYBACK_LIST_INFO  cloud_playback_list_info[2048];

static OS_MUTEX cloud_playback_list_mutex;
static int cloud_playback_list_task_pid;
static u8 cloud_playback_list_task_kill_flag;

extern uint64_t covBeijing2UnixTimeStp(struct tm *p);

static YEAR_LIST_INFO *find_empty_year_list_info(void)
{
    int i;
    YEAR_LIST_INFO *info = NULL;

    os_mutex_pend(&cloud_playback_list_mutex, 0);
    for (i = 0; i < sizeof(year_list_info); i++) {
        if (year_list_info[i].year == 0) {
            year_list_info[i].year = !0;
            info = &year_list_info[i];
            break;
        }
    }
    os_mutex_post(&cloud_playback_list_mutex);
    return info;
}

static void free_used_year_list_info(YEAR_LIST_INFO *info)
{
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    info->year = 0;
    os_mutex_post(&cloud_playback_list_mutex);
}



static CLOUD_PLAYBACK_LIST_INFO *find_empty_cloud_playback_list_info(void)
{
    int i;
    CLOUD_PLAYBACK_LIST_INFO *info = NULL;

    os_mutex_pend(&cloud_playback_list_mutex, 0);
    for (i = 0; i < sizeof(cloud_playback_list_info); i++) {
        if (cloud_playback_list_info[i].used == 0) {
            cloud_playback_list_info[i].used = !0;
            info = &cloud_playback_list_info[i];
            break;
        }
    }
    os_mutex_post(&cloud_playback_list_mutex);
    return info;
}

static void free_used_cloud_playback_list_info(CLOUD_PLAYBACK_LIST_INFO *info)
{
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    info->used = 0;
    os_mutex_post(&cloud_playback_list_mutex);
}

void *find_year_list(int year,struct list_head *head)
{
    u8 find = 0;
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    YEAR_LIST_INFO *p = NULL,*n = NULL;
    list_for_each_entry_safe(p, n, head, entry) {
        if(p->year == year){
            //说明当前年节点存在
            find = 1;
            break;
        }
    }
    os_mutex_post(&cloud_playback_list_mutex);
    if(find){
        return (void *)p;
    }else{
        return NULL;
    }
}



void *create_year_list(CLOUD_PLAYBACK_LIST_INFO *info,struct list_head *head)
{
    int year = info->day / 10000;
    int r =  info->day % 10000;
    int mon = r / 100;
    int day = r % 100;
  os_mutex_pend(&cloud_playback_list_mutex, 0);
    u8 find = 0;
    YEAR_LIST_INFO *p = find_empty_year_list_info();
     printf("\n >>>>>>>>>>%s %d\n",__func__,__LINE__);
    if(p){
        p->year = year;
        //直接初始化一整年的链表
        for(int i = 0;i < 12; i++ ){
            for(int j = 0;j < 31; j++){
                struct list_head *t_head = &p->mon[i].day[j].t_head;
                INIT_LIST_HEAD(t_head);
            }
        }
             printf("\n >>>>>>>>>>%s %d\n",__func__,__LINE__);
        if(list_empty(head)){
            printf("\n >>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
            list_add_tail(&p->entry, head);
        }else{
                YEAR_LIST_INFO *p_tmp = NULL,*n = NULL;
                list_for_each_entry_safe(p_tmp, n, head, entry) {
                printf("\n >>>>>>>>>>>>>>>>>>>>p->year = %lld p_tmp->year = %lld\n",p->year ,p_tmp->year);
                if(p->year > p_tmp->year ){
                    printf("\n >>>>>>>>>>>>>>>>>> p_tmp->year = %d\n",p_tmp->year);
                    __list_add(&p->entry,p_tmp->entry.prev,&p_tmp->entry );
                    find = 1;
                    break;
                }
            }
             if(find == 0){
                printf("\n >>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
                list_add_tail(&p->entry, head);
             }
        }
    }
        os_mutex_post(&cloud_playback_list_mutex);
    return p;
}

void free_year_list(YEAR_LIST_INFO *p)
{
      os_mutex_pend(&cloud_playback_list_mutex, 0);
    if(p){
        list_del(&p->entry);
        free_used_year_list_info(p);
    }
           os_mutex_post(&cloud_playback_list_mutex);
}





void *add_time_list(CLOUD_PLAYBACK_LIST_INFO  *info,void *priv)
{
    YEAR_LIST_INFO *year_info = (YEAR_LIST_INFO *)priv;
    DAY_LIST_INFO *day_info = NULL;
    u8 find = 0;
    CLOUD_PLAYBACK_LIST_INFO  *p = NULL,*n = NULL;
    int year = info->day / 10000;
    int r =  info->day % 10000;
    int mon = r / 100;
    int day = r % 100;
      os_mutex_pend(&cloud_playback_list_mutex, 0);
    day_info = &year_info->mon[mon - 1].day[day - 1];
    struct list_head *t_head = &day_info->t_head;


    if(list_empty(t_head)){
        list_add_tail(&info->entry, t_head);
    }else{

        list_for_each_entry_safe(p, n, t_head, entry) {
            //printf("\n >>>>>>>>>>>>>>>>>>>>p->start_time = %lld info->start_time = %lld\n",p->start_time ,info->start_time);
            //printf("\n >>>>>>>>>>>>>>>>>> p->name = %s\n",p->name);
            if(info->start_time > p->start_time ){
                 //printf("\n >>>>>>>>>>>>>>>>>> info->name = %s\n",info->name);
                __list_add(&info->entry,p->entry.prev,&p->entry );
                find = 1;
                break;
            }
        }
         if(find == 0){
            list_add_tail(&info->entry, t_head);
         }
    }
    if(info->ro_attr){
        list_root_info.ro_video_cnt++;
    }
    day_info->day_video_cnt++;
    list_root_info.total_video_cnt++;
    os_mutex_post(&cloud_playback_list_mutex);
}


void cloud_playback_list_info_add(CLOUD_PLAYBACK_LIST_INFO * info)
{
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    CLOUD_PLAYBACK_LIST_INFO *p, *n;

    int year = info->day / 10000;
    int r =  info->day % 10000;
    int mon = r / 100;
    int day = r % 100;

    //printf("\n >>>>>>>>>>%s %d\n",__func__,__LINE__);
    //printf("\n year = %d mon = %d day = %d\n",year,mon,day);
    //判断根节点是否为空
    if(list_empty(&list_root_info.root_head)){

        //申请一个年节点
        YEAR_LIST_INFO *year_info = create_year_list(info,&list_root_info.root_head);
        //增加一个时节点
        add_time_list(info,year_info);
    }else{
        //printf("\n >>>>>>>>>>%s %d\n",__func__,__LINE__);
        //年节点不为空
        YEAR_LIST_INFO *year_info = find_year_list(year,&list_root_info.root_head);
        //如果年节点存在
        if(year_info){
            //增加一个时节点
            add_time_list(info,year_info);
        }else{
            //申请一个年节点
            YEAR_LIST_INFO *year_info = create_year_list(info,&list_root_info.root_head);
            //增加一个时节点
            add_time_list(info,year_info);
        }
        //printf("\n >>>>>>>>>>%s %d\n",__func__,__LINE__);

    }
    os_mutex_post(&cloud_playback_list_mutex);
}


void cloud_playback_list_info_del(void *priv,void *info)
{
    YEAR_LIST_INFO *year_info = (YEAR_LIST_INFO *)priv ;
    DAY_LIST_INFO *day_info = NULL;
    CLOUD_PLAYBACK_LIST_INFO * p = (CLOUD_PLAYBACK_LIST_INFO * )info;
    int year = p->day / 10000;
    int r =  p->day % 10000;
    int mon = r / 100;
    int day = r % 100;
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    day_info = &year_info->mon[mon - 1].day[day - 1];

    printf("\n >>>>>>>%s %d\n",__func__,__LINE__);
    //删除时节点
    list_del(&p->entry);

    day_info->day_video_cnt--;
    list_root_info.total_video_cnt--;
    if(!strcmp(p->dir,"DCIM/2/") &&  p->ro_attr){
        printf("\n\n>>>>>>>>>>>>>> p->name = %s \n\n", p->name);
        list_root_info.ro_video_cnt--;
    }
    free_used_cloud_playback_list_info(p);
    printf("\n >>>>>>>%s %d\n",__func__,__LINE__);
    //如果当前时节点删除后，时节点链表为空
     os_mutex_post(&cloud_playback_list_mutex);
}





int get_utc_time_for_name(struct tm *p, char *name, int len)
{
    char *str = NULL;
    for (u8 i = 0; i < (sizeof(g_file_name_prefix) / sizeof(g_file_name_prefix[0])); i++) {

        str = strstr(name, g_file_name_prefix[i]);
        if (str) {
            break;
        }
    }
    if (!str) {
//        printf("\n name = %s not match g_file_name_prefix\n",name);
        return -1;
    }
    str += FILE_NAME_PREFIX_LEN;
    p->tm_year = (str[0] - '0') * 1000 + (str[1] - '0') * 100 + (str[2] - '0') * 10 + (str[3] - '0');
    str += 4;
    p->tm_mon = (str[0] - '0') * 10 + (str[1] - '0');
    str += 2;
    p->tm_mday = (str[0] - '0') * 10 + (str[1] - '0');
    str += 2 + 1;
    p->tm_hour = (str[0] - '0') * 10 + (str[1] - '0');
    str += 2;
    p->tm_min = (str[0] - '0') * 10 + (str[1] - '0');
    str += 2;
    p->tm_sec = (str[0] - '0') * 10 + (str[1] - '0');
//    printf("time is: %d/%02d/%02d-%02d:%02d:%02d \n",
//                    p->tm_year, p->tm_mon, p->tm_mday, \
//                    p->tm_hour, p->tm_min, p->tm_sec );
    return 0;
}


int cloud_playback_list_clear(void)
{
#if CONFIG_PLAYBACK_LIST_ENABLE

    if (!os_mutex_valid(&cloud_playback_list_mutex)) {
        return -1;
    }
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    {
        YEAR_LIST_INFO *y_tmp, *y_n_tmp;

        CLOUD_PLAYBACK_LIST_INFO *p ,*n;

        list_for_each_entry_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {

                for(int i = 0;i < 12; i++ ){
                    for(int j = 0;j < 31; j++){
                        struct list_head *t_head = &y_tmp->mon[i].day[j].t_head;
                        if(list_empty(t_head)){
                            continue;
                        }
                        list_for_each_entry_safe(p, n, t_head, entry) {
                                printf("\n>>>>>>>>>>>>>>>>>>%s p->name = %s\n",__func__,p->name);
                                cloud_playback_list_info_del(y_tmp,p);
                        }
                    }
                }

        }
    }



    {
        YEAR_LIST_INFO *y_tmp, *y_n_tmp;
        CLOUD_PLAYBACK_LIST_INFO *p ,*n;
        u32 find_del_year = 0;

        list_for_each_entry_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {

                for(int i = 0;i < 12; i++ ){
                    for(int j = 0;j < 31; j++){
                        struct list_head *t_head = &y_tmp->mon[i].day[j].t_head;
                        if(list_empty(t_head)){
                            continue;
                        }
                        find_del_year = 1;
                        break;
                    }
                }
                if(find_del_year){
                    free_year_list(y_tmp);
                }else{
                    find_del_year = 0;
                }
        }
    }

    os_mutex_post(&cloud_playback_list_mutex);
#endif // CONFIG_PLAYBACK_LIST_ENABLE
    return 0;
}

int get_msg_ro_attr_num(void)
{
    int  ret;
    if (!os_mutex_valid(&cloud_playback_list_mutex) || !storage_device_ready() ) {
        return 0;
    }
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    ret = list_root_info.ro_video_cnt;
    os_mutex_post(&cloud_playback_list_mutex);
    return ret;
}

void clean_msg_ro_attr(char *name)
{
    if (!os_mutex_valid(&cloud_playback_list_mutex) || !storage_device_ready() ) {
        return;
    }
    struct tm t;
    get_utc_time_for_name(&t, name, strlen(name));

    os_mutex_pend(&cloud_playback_list_mutex, 0);
    {
        YEAR_LIST_INFO *y_tmp, *y_n_tmp;
        CLOUD_PLAYBACK_LIST_INFO *p ,*n;

        list_for_each_entry_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {

            struct list_head *t_head = &y_tmp->mon[t.tm_mon -1].day[t.tm_mday -1].t_head;
            if(list_empty(t_head)){
                break;
            }
            list_for_each_entry_safe(p, n, t_head, entry) {

                if(!strcmp(p->dir,"DCIM/2/") && !strcmp(name,p->name) && p->ro_attr){
                    printf("\n\n>>>>>>>>>>>>>> p->name = %s \n\n", p->name);
                    list_root_info.ro_video_cnt--;
                    break;
                }
            }
        }
    }
    os_mutex_post(&cloud_playback_list_mutex);
}


#if  1



void  find_next_file(uint64_t start_time, char *path){

    struct tm t = {0};
    YEAR_LIST_INFO *y_tmp, *y_n_tmp;
    MON_LIST_INFO *m_tmp, *m_n_tmp;
    DAY_LIST_INFO *d_tmp, *d_n_tmp;
    CLOUD_PLAYBACK_LIST_INFO *p ,*n;
    static u8 next_flag=0;

    covUnixTimeStp2Beijing(start_time, &t);
    printf(" =%llu convert is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d\n", start_time,
           t.tm_year, t.tm_mon, t.tm_mday, \
           t.tm_hour, t.tm_min, t.tm_sec, t.tm_wday, t.tm_yday);

  // list_for_each_entry_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {
          list_for_each_entry_reverse_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry){
            printf("\n >>>>>>>>>>>>>>>>>> y_tmp->year = %d\n",y_tmp->year);
            if(t.tm_year != y_tmp->year){
                continue;
            }
            struct list_head *t_head = &y_tmp->mon[t.tm_mon - 1].day[t.tm_mday -1].t_head;
           // list_for_each_entry_safe(p, n, t_head, entry) {
            list_for_each_entry_reverse_safe(p, n, t_head, entry) {
             //   printf("\n  >>>>>>>>>>>>>>>>>>1 p->name = %s\n",p->name);
            struct tm t1 = {0};
            covUnixTimeStp2Beijing(p->start_time, &t1);
		#if  1
            printf("\n p->start_time: %llu ,convert is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d\n", p->start_time,
            t1.tm_year, t1.tm_mon, t1.tm_mday, \
            t1.tm_hour, t1.tm_min, t1.tm_sec, t1.tm_wday, t1.tm_yday);
        #endif
         //   printf("\n ===========covUnixTimeStp2Beijing >>>>>>>>>>>>>>>>>>p->name = %s,p->start_time:%llu\n",p->name,p->start_time);

			if (p) {
                     #if  0
                    if(start_time == p->start_time) {
                        snprintf(path, 128, CONFIG_ROOT_PATH"%s%s", p->dir, p->name);

                        printf("\n path0:%s \n",path);
                        break;
                    }
                    #else

                    if(next_flag){//找到下一个文件

                        snprintf(path, 128, CONFIG_ROOT_PATH"%s%s", p->dir, p->name);

                        printf("\n ===next_path====:%s \n",path);
                        next_flag=0;
                        break ;
                    }
                   //  if((t.tm_min == t1.tm_min)&&(t.tm_hour == t1.tm_hour)) {//当前app 下发的文件
                      if((t.tm_min == t1.tm_min?1: (t.tm_min>t1.tm_min+1 ?0:( t.tm_sec>=t1.tm_sec?0:1)  ))&&(t.tm_hour == t1.tm_hour))  {//当前app 下发的文件

                        next_flag=1;

                        printf("\n  find app curr  file \n");
                       // snprintf(path, 128, CONFIG_ROOT_PATH"%s%s", p->dir, p->name);

                    //    printf("\n path:%s \n",path);
                      //  break;
                    }

                    #endif

                }
            }

        }




}



#endif // 1

void cloud_playback_list_get_name_for_start_time(uint64_t start_time,uint64_t *fd_time, char *path,int32_t rtype,struct tm *st_time,struct tm *file_time)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    YEAR_LIST_INFO *y_tmp, *y_n_tmp;
     MON_LIST_INFO *m_tmp, *m_n_tmp;
    DAY_LIST_INFO *d_tmp, *d_n_tmp;
    CLOUD_PLAYBACK_LIST_INFO *p, *n;
    int i = 0;
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&list_root_info.root_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return;
    }


    struct tm t = {0};
    covUnixTimeStp2Beijing(start_time, &t);
    #if  0
    memcpy(st_time,&t,sizeof(struct tm));
    printf("\n st_timer is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d\n",
           st_time->tm_year, st_time->tm_mon, st_time->tm_mday, \
           st_time->tm_hour, st_time->tm_min, st_time->tm_sec, st_time->tm_wday, st_time->tm_yday);
    #endif

    #if  0
    printf("%llu convert is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d,rtype:%d\n", start_time,
           t.tm_year, t.tm_mon, t.tm_mday, \
           t.tm_hour, t.tm_min, t.tm_sec, t.tm_wday, t.tm_yday,rtype);
    #endif
    {


            if(rtype){//列表方式
            list_for_each_entry_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {


           // printf("\n %d >>>>>>>>>>>>>>>>>> y_tmp->year = %d\n",i,y_tmp->year);
            if(t.tm_year != y_tmp->year){
                continue;
            }
            struct list_head *t_head = &y_tmp->mon[t.tm_mon - 1].day[t.tm_mday -1].t_head;

            list_for_each_entry_safe(p, n, t_head, entry) {


           //printf("\n %d >>>>>>>>>>>>>>>>>>2 p->name = %s\n",i,p->name);
            struct tm t1 = {0};
            covUnixTimeStp2Beijing(p->start_time, &t1);
		#if  0
            printf("\n p->start_time: %llu ,convert is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d\n", p->start_time,
            t1.tm_year, t1.tm_mon, t1.tm_mday, \
            t1.tm_hour, t1.tm_min, t1.tm_sec, t1.tm_wday, t1.tm_yday);

            printf("\n ===========covUnixTimeStp2Beijing >>>>>>>>>>>>>>>>>>%d, p->name = %s,p->start_time:%llu\n",i,p->name,p->start_time);
        #endif
            //printf("\n  >>>>>>>>>>>>>>>>>>2 rtype = %d\n",rtype);
			if (p) {

                    if(start_time == p->start_time) {
                        snprintf(path, 128, CONFIG_ROOT_PATH"%s%s", p->dir, p->name);


                        printf("\n path1:%s \n",path);
                        break;
                    }


                }
            }

        }
        }else{





           list_for_each_entry_reverse_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry){

          //  printf("\n %d >>>>>>>>>>>>>>>>>> y_tmp->year = %d\n",i,y_tmp->year);
            if(t.tm_year != y_tmp->year){
                continue;
            }
            struct list_head *t_head = &y_tmp->mon[t.tm_mon - 1].day[t.tm_mday -1].t_head;



            list_for_each_entry_reverse_safe(p, n, t_head, entry) {

         //   printf("\n %d >>>>>>>>>>>>>>>>>>2 p->name = %s\n",i,p->name);
            struct tm t1 = {0};
            covUnixTimeStp2Beijing(p->start_time, &t1);
		#if  0
            printf("\n p->start_time: %llu ,convert is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d\n", p->start_time,
            t1.tm_year, t1.tm_mon, t1.tm_mday, \
            t1.tm_hour, t1.tm_min, t1.tm_sec, t1.tm_wday, t1.tm_yday);

            printf("\n ===========covUnixTimeStp2Beijing >>>>>>>>>>>>>>>>>>%d, p->name = %s,p->start_time:%llu\n",i,p->name,p->start_time);
        #endif
           // printf("\n  >>>>>>>>>>>>>>>>>>1 rtype = %d\n",rtype);
			if (p) {

                     if((t.tm_min == t1.tm_min?1: (t.tm_min>t1.tm_min+1 ?0:( t.tm_sec>=t1.tm_sec?0:1)  ))&&(t.tm_hour == t1.tm_hour)) {
                        snprintf(path, 128, CONFIG_ROOT_PATH"%s%s", p->dir, p->name);

                        #if  0
                        memcpy(file_time,&t1,sizeof(struct tm));
                        printf("\n file_timer is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d\n",
                       file_time->tm_year, file_time->tm_mon, file_time->tm_mday, \
                       file_time->tm_hour, file_time->tm_min, file_time->tm_sec, file_time->tm_wday, file_time->tm_yday);

                       #endif


                        *fd_time= p->start_time;

                       // printf("\n  KK>>>>>>>>>>>>>>>>>> fd_time = %d\n",*fd_time);
                        printf("\n path2:%s \n",path);
                        break;
                    }


                }
            }

        }


        }




    }

    os_mutex_post(&cloud_playback_list_mutex);

#endif
}



int cloud_playback_list_get_days(void *__req, void *__rsp)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    ipc_HistoryDays_Req *req = (ipc_HistoryDays_Req *)__req;
    ipc_HistoryDays_Resp *rsp = (ipc_HistoryDays_Resp *)__rsp;
    CLOUD_PLAYBACK_LIST_INFO *p, *n;
    int i;
    int j = sizeof(rsp->days) / sizeof(int32_t);
    rsp->days_count = 0;

    u32  date =   req->year * 100 + (req->month / 10) * 10 + (req->month % 10);
    for (int i = 0; i < j; i++) {
        rsp->days[i] = 0;
    }
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&list_root_info.root_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return -1;
    }

    {
        YEAR_LIST_INFO *y_tmp, *y_n_tmp;
        MON_LIST_INFO *m_tmp;
        CLOUD_PLAYBACK_LIST_INFO *p ,*n;

        list_for_each_entry_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {

            printf("\n%d >>>>>>>>>>>>>>>>>> y_tmp->year = %d\n",i,y_tmp->year);
            if(req->year != y_tmp->year){
                continue;
            }
            m_tmp = &y_tmp->mon[req->month -1];
            for(int day = 0;day < 31;day++){
                 struct list_head *t_head = &m_tmp->day[day].t_head;

                    list_for_each_entry_safe(p, n, t_head, entry) {
                        if(list_empty(t_head)){
                            continue;
                        }
                        printf("\n%d >>>>>>>>>>>>>>>>>>4 p->name = %s\n",i,p->name);
                        if (date == (p->day / 100)) {
                            printf("\n day = %d\n",p->day % 100);
                            for (i = 0; i <= rsp->days_count; i++) {
                                if (!rsp->days[i] || rsp->days[i] == (p->day % 100)) {
                                    break;
                                }
                            }
                            if (!rsp->days[i] && rsp->days_count < (j - 1)) {
                                rsp->days[i] = (p->day % 100);
                                rsp->days_count++;
                                rsp->total++;
                                break;
                            }
                        }
                    }
            }
        }
    }


    os_mutex_post(&cloud_playback_list_mutex);

//    printf("\n rsp->days_count= %d rsp->total = %d \n",rsp->days_count,rsp->total);
#endif
    return 0;
}


int cloud_playback_list_set_info(void  *__p,void *__req,void *__rsp)
{
    int ret = 0;
    int count = 0;
    ipc_HistoryDayList_Req *req = (ipc_HistoryDayList_Req *)__req;
    ipc_HistoryDayList_Resp *rsp = (ipc_HistoryDayList_Resp *)__rsp;
    CLOUD_PLAYBACK_LIST_INFO *p = (CLOUD_PLAYBACK_LIST_INFO *)__p;
    int j = 0;
    //当前页偏移
	int cur_page_offset;
    int page_offset_start = (req->page - 1) * req->page_size;
    if(req->rtype){
        cur_page_offset = rsp->historys_count + (req->page - 1) * req->page_size;
    }else{
        cur_page_offset = rsp->history_range_count + (req->page - 1) * req->page_size;  //p->name =
    }
    int page_offset_end = page_offset_start + req->page_size;

    #if  0
       printf("\n p->start_time==%llu\n,\
        p->length:%d\n,\
        rsp->historys_count:%d\n,\
        req->rtype:%d\n,\
        req->page:%d\n,\
        req->page_size:%d\n,\
        req->history_type_count:%d\n,\
        req->order:%d\n,\
        rsp->history_range_count:%d\n\
        rsp->total:%llu   \n",\

           p->start_time,\
           p->length,\
           rsp->historys_count,\
           req->rtype,\
           req->page,\
           req->page_size,\
           req->history_type_count,\
           req->order,\
           rsp->history_range_count,\
           rsp->total);

     #else

     #if  0
        printf("\n \
        req->page:%d\n,\
        req->history_type_count:%d\n,\
        rsp->history_range_count:%d\n\
        rsp->total:%llu   \n",\


           req->page,\
           req->history_type_count,\
           rsp->history_range_count,\
           rsp->total);
      #endif
     #endif
    // printf("\n>>>> req->rtype = %d\n",req->rtype);
    if(req->rtype) {

      if (p->day == req->day || req->day == 0) {
        //所以数据实际上是从页起始到页结束
        //数据是按照默认排序从 (page-1) * page_size 到 page * page_size之间的数据

        //跳过页偏移
        if(rsp->total < page_offset_start){
            rsp->total++;
            return 0;
        }
        //当前页未满
        if (cur_page_offset < page_offset_end) {

      //      printf("\n rsp->historys_count :%d\n",rsp->historys_count);
            rsp->historys[ rsp->historys_count].start_time = p->start_time * 1000;  //01:05:03
            rsp->historys[ rsp->historys_count].length = p->length;
            rsp->historys[ rsp->historys_count].file_id = p->start_time;
            rsp->historys[ rsp->historys_count].thum_fid = p->start_time;
        //    printf("\n p->start_time * 1000 :%d,p->length:%d\n",p->start_time * 1000,p->length);
            u8 type = 255;
            int i;
            for(i  = 0;i < (sizeof(g_file_name_prefix)/ sizeof(g_file_name_prefix[0]));i++){
                if (strstr(p->name, g_file_name_prefix[i])) {
                    break;
                }
            }
            if (i == 0) {
                type = E_IOT_EVENT_VISUAL_DOORBELL;
            } else if (i == 1) {
                type = E_IOT_EVENT_SENSE_HUMAN;
            } else if (i == 2) {
                type = 62;
            }
            for (j = 0; j < req->history_type_count; j++) {
           //     printf("\ntype = %d req->history_type[%d] = %d\n", type, j, req->history_type[j]);
                if (type == req->history_type[j] ||  req->history_type[j] == 0) {
                    break;
                }
            }
            if (j == req->history_type_count) {
                return 0;
            }
            rsp->historys[rsp->historys_count].history_type = type; //视频类型
            //当前返回数
            rsp->historys_count++;
        }else{
            ret = 1;
        }
        /* 总记录数*/
        rsp->total++;
        }

    } else {

      if (p->day == req->day || req->day == 0) {

           //跳过页偏移
        if(rsp->total < page_offset_start){
           // printf("\n L\n");
            rsp->total++;
            return 0;
        }
           //当前页未满
        if (cur_page_offset < page_offset_end) {


        rsp->history_range[rsp->history_range_count].start_time = p->start_time*1000;
        rsp->history_range[rsp->history_range_count].length = p->length;
        rsp->history_range[rsp->history_range_count].history_type = 255; //视频类型

      #if  0
        u8 type = 255;
            int i;
            for(i  = 0;i < (sizeof(g_file_name_prefix)/ sizeof(g_file_name_prefix[0]));i++){
                if (strstr(p->name, g_file_name_prefix[i])) {
                    break;
                }
            }
            if (i == 0) {
                type = E_IOT_EVENT_VISUAL_DOORBELL;
            } else if (i == 1) {
                type = E_IOT_EVENT_SENSE_HUMAN;
            } else if (i == 2) {
                type = 62;
            }
            for (j = 0; j < req->history_type_count; j++) {
           //     printf("\ntype = %d req->history_type[%d] = %d\n", type, j, req->history_type[j]);
                if (type == req->history_type[j] ||  req->history_type[j] == 0) {
                    break;
                }
            }
            if (j == req->history_type_count) {
                return 0;
            }
            rsp->historys[rsp->historys_count].history_type = type; //视频类型
         #endif

        rsp->history_range_count++;


        }else{
            ret = 1;
        }

          /* 总记录数*/
        rsp->total++;
      }

 }


    return ret;
}




int cloud_playback_list_get(void *__req, void *__rsp)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
  //  struct tm t = {0};
  //  char start_time_str[20] = {0};
  //  char end_time_str[20]    = {0};

    CLOUD_PLAYBACK_LIST_INFO *p, *n;
    ipc_HistoryDayList_Req *req = (ipc_HistoryDayList_Req *)__req; // 格式 YYYYMMDD
    ipc_HistoryDayList_Resp *rsp = (ipc_HistoryDayList_Resp *)__rsp;
    int year = req->day / 10000;
    int r =  req->day % 10000;
    int mon = r / 100;
    int day = r % 100;
    u8 page_full_flag = 0;
    int day_video_cnt = 0;

    printf("\n ==========year:%d,mon:%d,day:%d========\n ",year,mon,day);
    if(!os_mutex_valid(&cloud_playback_list_mutex)){
    printf("\n mutex_invalid \n");
     return -1;

    }

    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&list_root_info.root_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return -1;
    }

    {
        YEAR_LIST_INFO *y_tmp, *y_n_tmp;
        CLOUD_PLAYBACK_LIST_INFO *p ,*n;
        DAY_LIST_INFO *day_info = NULL;

     //  printf("\n >>>>>>>>>>>>>>>> req->order = %d,req->rtype:%d,%llu\n",req->order,req->rtype,req->start_time);
        if (req->order == 2) {
           list_for_each_entry_reverse_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {
              //  printf("\n >>>>>>>>>>>>>>>>>> y_tmp->year = %d\n",y_tmp->year);
                if(req->day != 0){

                    if(year != y_tmp->year){
                        continue;
                    }
                    day_info = &y_tmp->mon[mon - 1].day[day - 1];
                    struct list_head *t_head = &day_info->t_head;
                    day_video_cnt = day_info->day_video_cnt;
                    list_for_each_entry_reverse_safe(p, n, t_head, entry) {
                        printf("\n===M>>>>>>>>>>>>>>>>>>5 p->name = %s\n",p->name);
                        if(cloud_playback_list_set_info(p,__req,__rsp)){
                            page_full_flag = 1;
                            break;
                        }
                    }
                }else{
                    for(int i = 0;i < 12; i++ ){
                        for(int j = 0;j < 31; j++){
                            struct list_head *t_head = &y_tmp->mon[i].day[j].t_head;
                            if(list_empty(t_head)){
                                continue;
                            }
                            list_for_each_entry_safe(p, n, t_head, entry) {
                                printf("\n >>>N>>>>>>>>>>>>>>> p->name = %s\n",p->name);
                                if(cloud_playback_list_set_info(p,__req,__rsp)){
                                    page_full_flag = 1;
                                    break;
                                }
                            }
                        }
                    }
                }
                if( page_full_flag){
                    goto __exit;
                }
            }
        }else{
            list_for_each_entry_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {
          //  printf("\n ===1===>>>>>>>>>>>>>>>>>> y_tmp->year = %d,req->day=%d\n",y_tmp->year,req->day);
                if(req->day != 0){
                    int year = req->day / 10000;
                  //  printf("\n year :%d\n",year);
                    if(year != y_tmp->year){
                        continue;
                    }

                    day_info = &y_tmp->mon[mon - 1].day[day - 1];
                    struct list_head *t_head = &day_info->t_head;
                    day_video_cnt = day_info->day_video_cnt;

                    list_for_each_entry_safe(p, n, t_head, entry) {
                       // printf("\n>>>>>>>>>>>>>>>>>>6 p->name = %s\n",p->name);
                        if(cloud_playback_list_set_info(p,__req,__rsp)){
                            page_full_flag = 1;
                            break;
                        }
                    }
                }else{
                    for(int i = 0;i < 12; i++ ){
                        for(int j = 0;j < 31; j++){
                            struct list_head *t_head = &y_tmp->mon[11 - i].day[30 - j].t_head;
                            if(list_empty(t_head)){
                                continue;
                            }
                            list_for_each_entry_safe(p, n, t_head, entry) {
                                printf("\n>>>>>>>L>>>>>>>>>>> p->name = %s,rsp->historys_count:%d\n",p->name,rsp->historys_count);
                                if(cloud_playback_list_set_info(p,__req,__rsp)){
                                    page_full_flag = 1;
                                    break;
                                }
                            }
                        }
                    }
                }
                if( page_full_flag){
                    goto __exit;
                }
            }
        }


    }
__exit:


    printf("\n __exit req->page = %d\n", req->page);
    printf("\n __exit rsp->total = %llu \n",  rsp->total);
	 printf("\n __exit req->rtype = %llu \n",  req->rtype);
    if(req->day == 0){
       rsp->total = list_root_info.total_video_cnt;
    }else{
       rsp->total = day_video_cnt;
    }
    if(req->rtype){


      rsp->history_range_count=0;
    }else{

	  rsp->historys_count=0;
	}

  //  printf("\n>>>>>>>6>>>>>>>>>>> rsp->history_range_count:%d\n",rsp->history_range_count);

  //  printf("\n ====rsp->total:%llu, req->rtype:%d \n",rsp->total, req->rtype);
    os_mutex_post(&cloud_playback_list_mutex);

#endif // CONFIG_PLAYBACK_LIST_ENABLE

    return 0;
}

int cloud_playback_list_add(char *path)
{

#if CONFIG_PLAYBACK_LIST_ENABLE
    if (!storage_device_ready() || !os_mutex_valid(&cloud_playback_list_mutex)) {
        return -1;
    }
    void *fd = fopen(path, "r");
    if (!fd) {
        printf("\n not fopen\n");
        return 0;
    }

    if (!is_vaild_mov_file(fd)) {
        fclose(fd);
        printf("\n invaild_mov_file\n");
        return 0;
    }

    extern int read_time_scale_dur(FILE * file_fp, struct __mov_unpkg_info * info);
    struct __mov_unpkg_info unpkg_info = {0};;
    if (read_time_scale_dur(fd, &unpkg_info)) {
        fclose(fd);
        return 0;
    }
    CLOUD_PLAYBACK_LIST_INFO *info = find_empty_cloud_playback_list_info();
    if (!info) {
        printf("\n %s %d\n", __func__, __LINE__);
        fclose(fd);
        return 0;
    }
    info->length = unpkg_info.durition / unpkg_info.scale;

    char name[128];
    int len = fget_name(fd, name, sizeof(name));
    snprintf(info->name, sizeof(info->name),"%s", name);

    struct tm p;
    get_utc_time_for_name(&p, info->name, strlen(info->name));
    char day[32];
    snprintf(day, sizeof(day), "%d%02d%02d", p.tm_year, p.tm_mon, p.tm_mday);
    info->day = atol(day);
    info->start_time = covBeijing2UnixTimeStp(&p);


   // printf("\n info->start_time========================%d\n ",info->start_time);
    int list_path_num = sizeof(list_path) / sizeof(list_path[0]);
    printf( " \n list_path_num = %d \n",list_path_num);
    for(int i = 0;i<list_path_num ;i++){
        if(strstr(path,list_path[i])){
            strcpy(info->dir,list_path[i]);
        }
    }
    info->ro_attr = 0;
    if(!strcmp(info->dir,"DCIM/2/")){
        int attr;
        fget_attr(fd, &attr);
        if (attr & F_ATTR_RO) {
            info->ro_attr = 1;
        }
    }

    //info->file_id = info->thum_fig =  info->start_time;
#if 0
    printf("\n>>>>>>>>>>>>>>>>>> info->name = %s\n", info->name);
    printf("\n>>>>>>>>>>>>>>>>>> info->length = %d \n", info->length);
    printf("\n>>>>>>>>>>>>>>>>>> info->start_time = %llu \n", info->start_time);
    printf("\n>>>>>>>>>>>>>>>>>> info->day = %llu \n", info->day);
#endif
    fclose(fd);

    os_mutex_pend(&cloud_playback_list_mutex, 0);
    cloud_playback_list_info_add(info);
    //list_add(&info->entry, &list_root_info.root_head);
    os_mutex_post(&cloud_playback_list_mutex);
#endif // CONFIG_PLAYBACK_LIST_ENABLE

    return 0;
}



int cloud_playback_list_remove(void *__req, void *__rsp)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    ipc_HistoryDel_Req *req = (ipc_HistoryDel_Req *)__req;
    ipc_HistoryDel_Resp *rsp = (ipc_HistoryDel_Resp *)__rsp;

    uint64_t file_id = 0;
    char path[128];

    int total_id = req->file_id_count;
    if (!storage_device_ready() || !os_mutex_valid(&cloud_playback_list_mutex)) {
        return -1;
    }
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&list_root_info.root_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return -1;
    }
    {
        int i = 0;
        YEAR_LIST_INFO *y_tmp, *y_n_tmp;
        CLOUD_PLAYBACK_LIST_INFO *p ,*n;

        list_for_each_entry_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {
            file_id = req->file_id[i];
            struct tm t = {0};
            covUnixTimeStp2Beijing(file_id, &t);
//            printf("%llu convert is: %d/%02d/%02d-%02d:%02d:%02d weed = %d year_day = %d\n", file_id,
//                t.tm_year, t.tm_mon, t.tm_mday, \
//                t.tm_hour, t.tm_min, t.tm_sec, t.tm_wday, t.tm_yday);
            //printf("\n >>>>>>>>>>>>>>>>>> y_tmp->year = %d\n",y_tmp->year);
            if(t.tm_year != y_tmp->year){
                continue;
            }
            struct list_head *t_head = &y_tmp->mon[t.tm_mon - 1].day[t.tm_mday - 1].t_head;
            list_for_each_entry_safe(p, n,t_head, entry) {
                if( file_id == p->start_time){
                    cloud_playback_list_info_del(y_tmp,p);
                    snprintf(path, 128, CONFIG_ROOT_PATH"%s%s", p->dir, p->name);
                    void *fd = fopen(path, "r");
                    if (fd) {
                        fdelete(fd);
                    }
                    i++;
                    if (i >= total_id) {
                        break;
                    }
                    file_id = req->file_id[i];
                }
           }



        }
    }

    os_mutex_post(&cloud_playback_list_mutex);
#endif // CONFIG_PLAYBACK_LIST_ENABLE
    return 0;
}




int cloud_playback_list_local_remove(char *name)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    CLOUD_PLAYBACK_LIST_INFO *p, *n;
    uint64_t file_id = 0;

    int i;
    if (!storage_device_ready() || !os_mutex_valid(&cloud_playback_list_mutex)) {
        return -1;
    }
    os_mutex_pend(&cloud_playback_list_mutex, 0);
    if (list_empty(&list_root_info.root_head)) {
        printf("\n %s %d\n", __func__, __LINE__);
        os_mutex_post(&cloud_playback_list_mutex);
        return -1;
    }

    {

        struct tm t;
        get_utc_time_for_name(&t, name, strlen(name));

        YEAR_LIST_INFO *y_tmp, *y_n_tmp;

        CLOUD_PLAYBACK_LIST_INFO *p ,*n;

        list_for_each_entry_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {
            //printf("\n>>>>>>>>>>>>>>>>>> y_tmp->year = %d\n",y_tmp->year);
            if(t.tm_year != y_tmp->year){
                continue;
            }
            struct list_head *t_head = &y_tmp->mon[t.tm_mon - 1].day[t.tm_mday - 1].t_head;
            list_for_each_entry_safe(p, n, t_head, entry) {
                //printf("\n >>>>>>>>>>>>>>>>>> p->name = %s\n",p->name);
                if (p) {
                    if (strncmp(name, p->name,strlen(p->name))) {
                        cloud_playback_list_info_del(y_tmp,p);
                    }
                }
            }

        }
    }
    os_mutex_post(&cloud_playback_list_mutex);
#endif // CONFIG_PLAYBACK_LIST_ENABLE
    return 0;
}

void check_create_dir(void)
{
    void *fd = NULL;
    char dir[][128] = {
        CONFIG_REC_PATH_0,
       // CONFIG_REC_PATH_1,
       // CONFIG_REC_PATH_2,
       // CONFIG_REC_PATH_3,
    };
    int dir_num = sizeof(dir) / sizeof(dir[0]);
    char path[128];

    for(int i = 0;i < dir_num ;i++){
        printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
        fd = fopen(dir[i],"r");
        if(!fd){
            printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
            snprintf(path,sizeof(path),"%s%s",dir[i],"test.txt");

            fd = fopen(path,"w+");
            printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
            fdelete(fd);
        }else{
            printf("\n >>>>>>>>>>>>%s %d\n",__func__,__LINE__);
            fclose(fd);
        }
    }



}


void cloud_playback_list_task(void *priv)
{
    int len;
    struct tm p;
    struct vfscan *fs = NULL;
    void *fd = NULL;
    struct __mov_unpkg_info unpkg_info = {0};
    char path[128];

    while (!storage_device_ready()) {
        if(cloud_playback_list_task_kill_flag){
            goto __exit;
        }
        msleep(10);
    }
    check_create_dir();

    int list_path_num = sizeof(list_path) / sizeof(list_path[0]);
    printf( " \n list_path_num = %d \n",list_path_num);

    for(int i = 0;i < list_path_num ;i++){
        snprintf(path,sizeof(path),CONFIG_ROOT_PATH"%s",list_path[i]);
        fs = fscan(path, "-tMOV -st");
        if (!fs) {
            printf("\n %s %d\n", __func__, __LINE__);
            return;
        }
        fd = fselect(fs, FSEL_LAST_FILE, 0);
        while (1) {
            if(cloud_playback_list_task_kill_flag){
                break;
            }
            if (!fd) {
                break;
            }

            if (!flen(fd) || !is_vaild_mov_file(fd)) {
                fclose(fd);
                fd = fselect(fs, FSEL_PREV_FILE, 0);
                continue;
            }


            if (read_time_scale_dur(fd, &unpkg_info) ||  read_time_scale_dur(fd, &unpkg_info) || read_height_and_length(fd, &unpkg_info)) {
                fclose(fd);
                fd = fselect(fs, FSEL_PREV_FILE, 0);
                continue;
            }
            CLOUD_PLAYBACK_LIST_INFO *info = find_empty_cloud_playback_list_info();
            if (!info) {
                printf("\n %s %d\n", __func__, __LINE__);
                fclose(fd);
                break;
            }

            if (unpkg_info.scale) {
                info->length = unpkg_info.durition / unpkg_info.scale;
            }
            strcpy(info->dir,list_path[i]);
            info->ro_attr = 0;
            if(!strcmp(info->dir,"DCIM/2/")){
                int attr;
                fget_attr(fd, &attr);
                if (attr & F_ATTR_RO) {
                    info->ro_attr = 1;
                }
            }
            char name[128];
            len = fget_name(fd, name, sizeof(name));
            snprintf(info->name, sizeof(info->name),"%s", name);
            fclose(fd);

            if (get_utc_time_for_name(&p, info->name, strlen(info->name))) {
                free_used_cloud_playback_list_info(info);
                fd = fselect(fs, FSEL_PREV_FILE, 0);
                continue;
            }

            char day[32];
            snprintf(day, sizeof(day), "%d%02d%02d", p.tm_year, p.tm_mon, p.tm_mday);
            info->day = atol(day);
            info->start_time = covBeijing2UnixTimeStp(&p);

            //info->file_id = info->thum_fig =  info->start_time;
#if 0
            printf("\n>>>>>>>>>>>>>>>>>> info->name = %s\n", info->name);
            printf("\n>>>>>>>>>>>>>>>>>> info->length = %d \n", info->length);
            printf("\n>>>>>>>>>>>>>>>>>> info->start_time = %llu \n", info->start_time);
            printf("\n>>>>>>>>>>>>>>>>>> info->day = %llu \n", info->day);
#endif
            os_mutex_pend(&cloud_playback_list_mutex, 0);
            cloud_playback_list_info_add(info);

            os_mutex_post(&cloud_playback_list_mutex);

            fd = fselect(fs, FSEL_PREV_FILE, 0);

            extern void vPortYield(void);
            vPortYield();

        }

        fscan_release(fs);
#if 0
        {
            YEAR_LIST_INFO *y_tmp, *y_n_tmp;
            CLOUD_PLAYBACK_LIST_INFO *p ,*n;

            list_for_each_entry_safe(y_tmp, y_n_tmp, &list_root_info.root_head, entry) {

                for(int i = 0;i < 12; i++ ){
                    for(int j = 0;j < 31; j++){
                        struct list_head *t_head = &y_tmp->mon[i].day[j].t_head;
                        if(list_empty(t_head)){
                            continue;
                        }
                        list_for_each_entry_safe(p, n, t_head, entry) {
                            printf("\n>>>>>>>>>>>>>>>>>>7 p->name = %s\n",p->name);
                        }
                    }
                }


            }
            cloud_playback_list_clear();
        }
#endif

    }
__exit:

//    printf("\n\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>get_msg_ro_attr_num() = %d\n\n",get_msg_ro_attr_num());
//    clean_msg_ro_attr("MSG_20231013_112654.MOV");
//    printf("\n\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>get_msg_ro_attr_num() = %d\n\n",get_msg_ro_attr_num());
    return;
}


int cloud_playback_list_init(void)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    if(!os_mutex_valid(&cloud_playback_list_mutex)){
        os_mutex_create(&cloud_playback_list_mutex);
        list_root_info.total_video_cnt = 0;
        INIT_LIST_HEAD(&list_root_info.root_head);
    }
    thread_fork("cloud_playback_list_task", 8, 0x1000, 0, &cloud_playback_list_task_pid, cloud_playback_list_task, NULL);
#endif // CONFIG_PLAYBACK_LIST_ENABLE
    return 0;
}

int cloud_playback_list_uninit(void)
{
#if CONFIG_PLAYBACK_LIST_ENABLE
    cloud_playback_list_task_kill_flag = 1;
    thread_kill(&cloud_playback_list_task_pid,0);
    cloud_playback_list_task_kill_flag = 0;
    cloud_playback_list_clear();
#endif
}

//late_initcall(cloud_playback_list_init);
extern int avsdk_stop(void);
extern void switch_usb_workmode(u8 on);
int enter_product_mode(){
    void *fd = NULL;
    char name[128];

     if(!storage_device_ready){

        printf("\n no sd in \n");
        return  0;
     }
     os_time_dly(20);
     snprintf(name, sizeof(name), "%s%s", CONFIG_ROOT_PATH, ENTER_PRODUCT_MODE_FILE_NAME);
     fd = fopen(name, "r");
     printf("\n product_fd======================%d\n",fd);
     if(fd){

     fclose(fd);
     fd=NULL;
     switch_usb_workmode(1);
     return 1;
     }else{

      return 0;
     }

    return  0;


}
static void cloud_list_device_event_handler(struct sys_event *event)
{
     printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%d,%s, %s %d\n",event->u.dev.event,event->arg,__func__,__LINE__);
    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        switch (event->u.dev.event) {
        case DEVICE_EVENT_ONLINE:
        case DEVICE_EVENT_IN:


            os_time_dly(200);
            if(enter_product_mode()){
                avsdk_stop();
                return  ;
            }

            cloud_playback_list_init();


            /*进行卡升级检查*/
            if (sdcard_upgrade_init()) {
                /*如果需要卡升级，不跑其他流程*/
                return;
            }
            /*检查是否需要从sd卡读取授权文件*/
            doorbell_write_cfg_info();
            break;

        case DEVICE_EVENT_OFFLINE:
        case DEVICE_EVENT_OUT:
             printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
            cloud_playback_list_uninit();
             printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
            break;
        }
    }
}

SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, cloud_list_device_event_handler, 0);












