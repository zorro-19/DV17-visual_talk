#define _STDIO_H_
#include "xciot_api.h"
#include "packet.h"
#include "init.h"



extern uint64_t get_utc_ms(void);
extern int net_video_rec_get_fps(void);
extern int net_video_rec_get_audio_rate();

typedef struct cloud_playback_info {
    struct list_head entry;
    u8 kill_flag;
    int task_id;
    char task_name[32];
    int32_t conn_id;
    uint64_t file_id;
    int64_t start_time;
    int32_t rtype;
} CLOUD_PLAYBACK_INFO;

#define VIDEO_INTERVAL  (1000 / net_video_rec_get_fps())
//包长度 / 每ms音频长度  = 音频间隔, -8 每次提前8ms进行发送
#define AUDIO_INTERVAL   ((AUDIO_PACKET_LEN / (net_video_rec_get_audio_rate() / 500)) - 8)
#define AUDIO_PACKET_LEN        2048
static LIST_HEAD(cloud_playback_list_head);
static OS_MUTEX cloud_playback_mutex;


/*
 * 获取文件缩略图
 * name: 文件名
 * type: 文件类型(loop, emr)---暂时用不上
 * thm_addr:缩略图二进制文件的存放地址
 * 返回值:读到的缩略图的长度
 * */
int get_file_thm(char *name, char type, char *thm_addr, int len)
{
#define FILE_THM_OFFSET 0X200

    int ret;
    FILE *fd;

    fd = fopen(name, "r");
    if (!fd) {
        printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
        thm_addr = NULL;
        return 0;
    }
    if (!is_vaild_mov_file(fd)) {
        printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
        fclose(fd);
        return 0;
    }


    fseek(fd, 0x200, SEEK_SET);
    ret = fread(fd, thm_addr, len);
    if (ret != len) {
        printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
        fclose(fd);
        return 0;
    }
    u8 *stream = thm_addr;


#if 0
    int i = 0;
    for (i = len - 1  ; i > 0 ; i--) {
        if (stream[i - 1] == 0xFF && stream[i] == 0xD9) {
            break;
        }
    }
    i++;
#else
    int i = 0;
    for (; i < len ; i++) {
        if (stream[i] == 0xFF && stream[i + 1] == 0xD9) {
            break;
        }
    }
    i += 2;
#endif
    fclose(fd);

    return i + 1;
}

static int find_idr_frame2(struct __packet_info *pinfo, u32 offset)
{
    int i = 0;
    char buf[5];
    int  sample_offset;
    int sample_size;
    int ret;

    i = offset;
    while (1) {

        if (i >= pinfo->info.video_sample_count) {
            //返回之后，会发结束帧,才不会导致APP快进花屏
            return pinfo->info.video_sample_count;
        }

        sample_size = get_sample_size(pinfo->info.stsz_tab, i);
        if (sample_size == -1) {
            printf("\n[Error] %s %d\n", __func__, __LINE__);
            return pinfo->info.video_sample_count;
        }
        sample_offset = get_chunk_offset(pinfo->info.stco_tab, i);
        if (sample_offset == -1) {
            printf("\n[Error] %s %d\n", __func__, __LINE__);
            return pinfo->info.video_sample_count;
        }
        if (false == fseek(pinfo->fd, sample_offset, SEEK_SET)) {
            printf("\n[Error] %s %d\n", __func__, __LINE__);
            return pinfo->info.video_sample_count;
        }
        ret = fread(pinfo->fd, buf, 5);
        if (ret != 5) {
            printf("\n[Error] %s %d\n", __func__, __LINE__);
            return pinfo->info.video_sample_count;
        }
        if (buf[4] != 0x67) {
#if 0
            i++; //往后找I帧
#else
            i--;
#endif

        } else {
            return i;
        }
    }

}

void cloud_playback_task(void *priv)
{

    CLOUD_PLAYBACK_INFO *p = (struct avplay_info *)priv;


  //p->start_time=(1712543541-set_time*2);
    int32_t conn_id = p->conn_id;
	uint64_t file_id, fd_time;
	if(p->rtype){
    file_id = p->file_id;
    }else{

    #if 0
	file_id =p->start_time;//-set_time*2;// p->file_id;  app下发的时间点
    p->start_time=  file_id;
    p->file_id=file_id;
    #else
    file_id =p->start_time;

    #endif
	}
    char path[128];
    char next_path[128]={0};
    struct tm t;
    uint64_t a_send_time, v_send_time;
    int64_t a_timestamp_ms, v_timestamp_ms;


    a_timestamp_ms = v_timestamp_ms = p->start_time * 1000;

    char *pcm_data = NULL;
    int offset = 0;
    int audio_remain = 0;
    int send_audio_flag = 1;
    int send_video_flag = 1;
    struct __packet_info pinfo = {0};
    int ret = 0;
    int i = 0, j = 0;

    struct tm st_time={0};struct tm file_time={0};

  //  printf("\n =====p->conn_id===%d,  req->file_id =======%llu ,p->start_time:%llu,p->rtype:%d\n",p->conn_id, file_id,p->start_time,p->rtype);

    extern void cloud_playback_list_get_name_for_start_time(uint64_t start_time,uint64_t *fd_time, char *path,int32_t rtype,struct tm *st_time,struct tm *file_time);
    cloud_playback_list_get_name_for_start_time(file_id,&fd_time, path,p->rtype,NULL,NULL);



    pinfo.len = IMAGE_SIZE;
    pinfo.data = (u8 *)zalloc(IMAGE_SIZE);
    if (pinfo.data == NULL) {
        printf("\n[Error] %s %d malloc fali\n", __func__, __LINE__);
        goto err1;
    }

    pinfo.fd =  fopen(path, "r");
    if (!pinfo.fd) {
        goto err1;
    }

    if (!is_vaild_mov_file(pinfo.fd)) {
        printf("\n[Error] %s %d FILE is vaild\n", __func__, __LINE__);
        goto err1;
    }
    pinfo.type = H264_TYPE_VIDEO;

    if (get_video_media_info(&pinfo)) {
        printf("\n[Error] %s %d  get media info fail\n", __func__, __LINE__);
        goto err1;
    }

    pcm_data = calloc(1, 8192 * 2 + 1024);
    if (!pcm_data) {
        puts("\n pcm_data malloc fail\n");
        goto err1;
    }
//  printf("\n offset = %lld\n", p->start_time - p->file_id);
    int first_I_frame = 0 ;
    //p->start_time - p->file_id 单位为s
    //i = (p->start_time - p->file_id) * net_video_rec_get_fps();
  //  printf("\n fd_time==============%llu\n ",fd_time);
    i = (p->start_time - fd_time) * net_video_rec_get_fps();

    i = find_idr_frame2(&pinfo, i);
    j = (i / net_video_rec_get_fps() * 1000) / 512;
    //更新一下p->start_time
    p->start_time = (p->file_id + (i / net_video_rec_get_fps())) * 1000;


    printf("\n>>>>> i = %d j = %d\n", i, j);

  //  printf("\n  pinfo.info.video_sample_count = %d  pinfo.info.audio_chunk_num = %d\n",pinfo.info.video_sample_count,pinfo.info.audio_chunk_num);

    while (1) {
        if (p->kill_flag) {

            printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
            break;
        }
        /*send  audio*/
        if (pinfo.info.video_sample_count && pinfo.info.audio_chunk_num && audio_remain == 0) {
            if (j < pinfo.info.audio_chunk_num) {

                int audio_offset = get_audio_chunk_offset(pinfo.info.audio_stco_tab, j);
                if (audio_offset == -1) {
                     printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
                    goto err1;
                }
                if (false == fseek(pinfo.fd, audio_offset, SEEK_SET)) {
                     printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
                    goto err1;
                }

                ret = fread(pinfo.fd, pcm_data, pinfo.info.audio_block_size);
                if (ret <= 0) {
                     printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
                    goto err1;
                }
//                printf("\n pinfo.info.audio_block_size = %d\n", ret);

//                putchar('A');
                //put_buf(pinfo.data, 32);
                audio_remain = ret;

            }
        }
        if (send_audio_flag && audio_remain != 0) {

            send_audio_flag = 0;
            a_send_time = get_utc_ms();
            int audio_len = audio_remain > AUDIO_PACKET_LEN ? AUDIO_PACKET_LEN : audio_remain;

            ret = avsdk_write_history_audio(conn_id, E_IOT_MEDIASTREAM_TYPE_PCM, 0, a_timestamp_ms, file_id, NULL, pcm_data + offset, audio_len); //Ã¿´Î·¢1024£¬²»Ö§³Ö·¢´ó°ü
            if (ret == -9) {
                printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
                goto err1;
            }
            audio_remain -= audio_len;
            offset += audio_len;
            if (audio_remain == 0) {
                j++;
                offset = 0;
            }
            a_timestamp_ms += AUDIO_INTERVAL + 8; //补回
        }

        if (send_video_flag) {
            send_video_flag  = 0;

//          printf("\n i = %d\n",i);
            /*send  video*/
            u32 tmp ;
            char *buffer = NULL;
            u32 tmp2;
            u32 start_code = 0x01000000;
            int sample_size = get_sample_size(pinfo.info.stsz_tab, i);
            if (sample_size == -1) {
                printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
                goto err1;
            }
            int sample_offset = get_chunk_offset(pinfo.info.stco_tab, i);
            if (sample_offset == -1) {
                 printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
                goto err1;
            }
            if (sample_size > pinfo.len) {
                printf("I frame size:0x%x   offset:0x%x buffer size:%d\n", sample_size, sample_offset, pinfo.len);
                printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
                goto err1;
            }
            fseek(pinfo.fd, sample_offset, SEEK_SET);
            ret = fread(pinfo.fd, pinfo.data, sample_size);
            if (ret <= 0) {
                 printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
                goto err1;
            }
            buffer = (char *)pinfo.data;
            u8 iskeyframe = 0;
            if (*((char *)(buffer + 4)) == 0x67) {
                iskeyframe = 1;
                //处理PPS帧和SPS帧 I帧
                memcpy(&tmp, buffer, 4);
                tmp = htonl(tmp);
                memcpy(buffer, &start_code, 4);
                memcpy(&tmp2, buffer + tmp + 4, 4);
                tmp2 = htonl(tmp2);
                memcpy(buffer + tmp + 4, &start_code, 4);
                memcpy(buffer + tmp + tmp2 + 8, &start_code, 4);
                //printf("tmp %d  tmp2 %d\n",tmp,tmp2);
                //printf("&&&&&& video-I seq = %d\n",i);
            } else {
                //	   处理P帧
                memcpy(buffer, &start_code, 4);
            }
            v_send_time = get_utc_ms();
            if (first_I_frame == 0 && iskeyframe == 1) {
                first_I_frame = 1;
            }
            if (first_I_frame) {
                ret = avsdk_write_history_video(conn_id, E_IOT_MEDIASTREAM_TYPE_H264, iskeyframe, 0, v_timestamp_ms, file_id, NULL, buffer, sample_size);
                if (ret == -9) {
                     printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
                    goto err1;
                }
            }
            v_timestamp_ms += VIDEO_INTERVAL;
            i++;
            if (i >= pinfo.info.video_sample_count) {

			printf(">>>>>>>>>>>%s %d,p->rtype:%d\n", __FUNCTION__, __LINE__,p->rtype);
                // 一段播放完
            if(!p->rtype){

            extern  void  find_next_file(uint64_t start_time, char *path);
            find_next_file(file_id,next_path);
           // find_next_file(*fd_time,next_path);
            #if  1
            struct tm p_n;
            get_utc_time_for_name(&p_n, next_path, strlen(next_path));
            char day[32];
            //snprintf(day, sizeof(day), "%d%02d%02d", p.tm_year, p.tm_mon, p.tm_mday);
            //info->day = atol(day);
            file_id = covBeijing2UnixTimeStp(&p_n);
            printf("\n ===== next_file_id =======%llu\n" ,file_id);
           #endif
            // snprintf(file_name, sizeof(file_name), "%sVID_%04d.MOV", CONFIG_REC_PATH_0, file_id);
            printf("\n play next file_name = %s \n", next_path);

            if (pinfo.info.stco_tab != NULL) {
                free(pinfo.info.stco_tab);
                pinfo.info.stco_tab = NULL;
            }
            if (pinfo.info.stsz_tab != NULL) {
                free(pinfo.info.stsz_tab);
                pinfo.info.stsz_tab = NULL;
            }
            if (pinfo.info.audio_stco_tab != NULL) {
                free(pinfo.info.audio_stco_tab);
                pinfo.info.audio_stco_tab = NULL;
            }
            if (pinfo.fd != NULL) {
                fclose(pinfo.fd);
                pinfo.fd = NULL;
            }
            if (pinfo.data) {
                free(pinfo.data);
            }
            if (pcm_data) {
                free(pcm_data);
                pcm_data=NULL;
            }
            pinfo.data = NULL;
            memset(&pinfo, 0x0, sizeof(struct __packet_info ));


            a_timestamp_ms = v_timestamp_ms = file_id * 1000;
            offset = 0;
            audio_remain = 0;
            send_audio_flag = 1;
            send_video_flag = 1;

            pinfo.len = IMAGE_SIZE;
            pinfo.data = (u8 *)zalloc(IMAGE_SIZE);
            if (pinfo.data == NULL) {
                printf("\n[Error] %s %d malloc fali\n", __func__, __LINE__);
                goto err1;
            }
            if(!pinfo.fd){
            pinfo.fd =  fopen(next_path, "r");
            if (!pinfo.fd) {
                goto err1;
            }
            }
            if (!is_vaild_mov_file(pinfo.fd)) {
                printf("\n[Error] %s %d FILE is vaild\n", __func__, __LINE__);
                goto err1;
            }
            pinfo.type = H264_TYPE_VIDEO;

            if (get_video_media_info(&pinfo)) {
                printf("\n[Error] %s %d  get media info fail\n", __func__, __LINE__);
                goto err1;
            }

            pcm_data = calloc(1, 8192 * 2 + 1024);
            if (!pcm_data) {
                puts("\n pcm_data malloc fail\n");
                goto err1;
            }
        //    printf("\n offset = %lld\n", p->start_time - p->file_id);
            int first_I_frame = 0 ;

            //p->start_time - p->file_id 单位为s
            i = (p->start_time - p->file_id) * net_video_rec_get_fps();
            i = find_idr_frame2(&pinfo, i);
            j = (i / net_video_rec_get_fps() * 1000) / 512;
            //更新一下p->start_time
            p->start_time = (p->file_id + (i / net_video_rec_get_fps())) * 1000;





            i = 0;
            j = 0;


              //  break;
            }else{

            break;
			}


            }

        }

        if ((get_utc_ms() - v_send_time) >= VIDEO_INTERVAL) {
            send_video_flag = 1;
        }
        if ((get_utc_ms() - a_send_time) >= AUDIO_INTERVAL) {
            send_audio_flag = 1;
        }
        if (!send_video_flag && !send_audio_flag) {
            extern void vPortYield(void);
            vPortYield();
        }
    }

err1:
//    fclose(fd);
     printf(">>>>>>>>>>>%s %d\n", __FUNCTION__, __LINE__);
    if (pinfo.info.stco_tab != NULL) {
        free(pinfo.info.stco_tab);
        pinfo.info.stco_tab = NULL;
    }
    if (pinfo.info.stsz_tab != NULL) {
        free(pinfo.info.stsz_tab);
        pinfo.info.stsz_tab = NULL;
    }
    if (pinfo.info.audio_stco_tab != NULL) {
        free(pinfo.info.audio_stco_tab);
        pinfo.info.audio_stco_tab = NULL;
    }
    if (pinfo.fd != NULL) {
        fclose(pinfo.fd);
        pinfo.fd = NULL;
    }
    if (pinfo.data) {
        free(pinfo.data);
    }
    if (pcm_data) {
        free(pcm_data);
    }
    pinfo.data = NULL;
}



int cloud_playback_init(int32_t conn_id,  uint64_t file_id, int64_t start_time, int32_t rtype )
{
    static int count;
    char name[32];

    CLOUD_PLAYBACK_INFO *p = calloc(1, sizeof(CLOUD_PLAYBACK_INFO));
    if (p) {
        p->conn_id = conn_id;
        p->file_id = file_id;
        p->start_time = start_time;
        p->rtype=rtype;
        printf("\n  p->file_id  = %llu   p->start_time = %lld,p->rtype:%d\n", p->file_id, p->start_time,p->rtype);
    } else {
        return -1;
    }

    snprintf(p->task_name, sizeof(p->task_name), "cloud_playback_task%d", count++);

    thread_fork(p->task_name, 8, 0x1000, 0, &p->task_id, cloud_playback_task, p);
    printf("\n p->task_id:%d\n",p->task_id);

    os_mutex_pend(&cloud_playback_mutex, 0);
    list_add_tail(&p->entry, &cloud_playback_list_head);
    os_mutex_post(&cloud_playback_mutex);
    return 0;
}

int cloud_playback_uninit(int32_t conn_id,  uint64_t file_id)
{
    CLOUD_PLAYBACK_INFO *p;
    struct list_head *pos = NULL, *node = NULL;
    os_mutex_pend(&cloud_playback_mutex, 0);
    if (list_empty(&cloud_playback_list_head)) {
         putchar('N');
        os_mutex_post(&cloud_playback_mutex);
        return 0;
    }
    list_for_each_safe(pos, node, &cloud_playback_list_head) {
        p = list_entry(pos, CLOUD_PLAYBACK_INFO, entry);
        putchar('P');
        if (p) {
            printf("\n p->conn_id:%d\n",p->conn_id);
            printf("\n conn_id:%d\n",conn_id);
              printf("\n u_p->task_id:%d\n",p->task_id);
            if (p->conn_id == conn_id) {
                p->kill_flag = 1;
                os_time_dly(10);
                list_del(&p->entry);


                thread_kill(&p->task_id, 0);
                free(p);
                break;
            }
        }
    }
    os_mutex_post(&cloud_playback_mutex);

    return 0;
}
int cloud_playback_mutex_init(void)
{
    return os_mutex_create(&cloud_playback_mutex);
}
late_initcall(cloud_playback_mutex_init);
