#include "system/includes.h"
#include "server/audio_server.h"
#include "server/server_core.h"

#include "app_config.h"

enum {
    SD_UNMOUNT,
    SD_MOUNT_SUSS,
    SD_MOUNT_FAILD,
};

static char *const sd_list[] = {
    "sd0",
    "sd1",
    "sd2",
};

static u32 class;
static u8 fs_mount = SD_UNMOUNT;
static OS_MUTEX sd_mutex;

static int __sd_mutex_init()
{
    return os_mutex_create(&sd_mutex);
}
early_initcall(__sd_mutex_init);


int mount_sd_to_fs(const char *name)
{
    int err = 0;
    struct imount *mt;
    int id = ((char *)name)[2] - '0';
    const char *dev  = sd_list[id];

    err = os_mutex_pend(&sd_mutex, 0);
    if (err) {
        return -EFAULT;
    }


    if (fs_mount == SD_MOUNT_SUSS) {
        goto __exit;
    }
    if (fs_mount == SD_MOUNT_FAILD) {
        err = -EFAULT;
        goto __exit;
    }
    if (!dev_online(dev)) {
        err = -EFAULT;
        goto __exit;
    }

    void *fd = dev_open(dev, 0);
    if (!fd) {
        err = -EFAULT;
        goto __err;
    }
    dev_ioctl(fd, SD_IOCTL_GET_CLASS, (u32)&class);
    if (class == SD_CLASS_10) {
        puts("sd card class: 10\n");
    } else {
        log_w("sd card class: %d\n", class * 2);
    }
    dev_close(fd);

    mt = mount(dev, CONFIG_STORAGE_PATH, "fat", FAT_CACHE_NUM, NULL);
    if (!mt) {
        puts("mount fail\n");
        err = -EFAULT;
    } else {
        puts("mount sd suss\n");
    }

__err:
    fs_mount = err ? SD_MOUNT_FAILD : SD_MOUNT_SUSS;
__exit:
    os_mutex_post(&sd_mutex);
//if (fs_mount == SD_MOUNT_SUSS){
    // extern void sd_check()
    //sd_check();
//}
    return err;
}

void unmount_sd_to_fs(const char *path)
{
    os_mutex_pend(&sd_mutex, 0);

    unmount(path);
    fs_mount = SD_UNMOUNT;

    os_mutex_post(&sd_mutex);
}

int storage_device_ready()
{
    if (!dev_online(SDX_DEV)) {
        return false;
    }
    if (fs_mount == SD_UNMOUNT) {
        mount_sd_to_fs(SDX_DEV);
    }
    int err =fdir_exist(CONFIG_STORAGE_PATH);
    printf("\n ==%s, %d==\n",__FUNCTION__,err);
    return err;
}

int storage_device_format()
{
    int err;

    unmount_sd_to_fs(CONFIG_STORAGE_PATH);

    err = f_format(SDX_DEV, "fat", 32 * 1024);
    printf("\n f_format__err:%d\n",err);
    if (err == 0) {
        mount_sd_to_fs(SDX_DEV);
    }

    return err;
}


/*
 * sd卡插拔事件处理
 */
static void sd_event_handler(struct sys_event *event)
{
    int id = ((char *)event->arg)[2] - '0';
    const char *dev  = sd_list[id];
    static u8 flasg=0;
   // printf("\n >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%d,%s,%s %d\n",event->u.dev.event,event->arg,__func__,__LINE__);
    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:


        if(!flasg){
        flasg=1;
        unmount_sd_to_fs(CONFIG_STORAGE_PATH);
        os_time_dly(20);

       // mount_sd_to_fs(SDX_DEV);
       mount_sd_to_fs(event->arg);
        }


       //




        break;
    case DEVICE_EVENT_OUT:
        printf("%s: out\n", dev);
        unmount_sd_to_fs(CONFIG_STORAGE_PATH);
        break;
    }
}

static char *const udisk_list[] = {
    "udisk0",
    "udisk1",
};
static int usb_disk_online(const char *dev_name)
{
    void *dev = NULL;
    u32 sta = 0;
    dev = dev_open(dev_name, 0);
    if (dev) {
        dev_ioctl(dev, IOCTL_GET_STATUS, (u32)&sta);
        dev_close(dev);
        return sta;
    }
    return 0;
}
static int usb_mass_storage_fs_test(const char *disk)
{
    struct imount *mt = NULL;
    char path[64];
    int ret = 0;
    u8 id = disk[5] - '0';
    char *const udisk_root_path[] = {
        "storage/udisk0",
        "storage/udisk1",
    };
    mt = mount(disk, udisk_root_path[id], "fat", FAT_CACHE_NUM, 0);
    if (!mt) {
        log_w("mount %s fail", disk);
        return -ENODEV;
    }
    sprintf(path, "%s/C/", udisk_root_path[id]);
    ret = strlen(path);
    if (path[ret - 1] != '/') {
        path[ret] = '/';
        path[ret + 1] = 0;
    }
    log_d("%d resolute path of file: %s", __LINE__, path);

#if 1
    ///TEST
    char path1[64];
    strcpy(path1, path);
    strcat(path1, "source.txt");
    log_d("resolute path of file: %s", path1);
    FILE *f0 = fopen(path1, "r");
    if (!f0) {
        log_e("fail to open file %s", path1);
        return -1;
    }
    log_d("succeed to open source file");
    strcpy(path1, path);
    strcat(path1, "dest.txt");
    log_d("resolute dst path of file: %s", path1);
    FILE *f1 = fopen(path1, "w+");
    if (!f1) {
        log_e("fail to open file %s", path1);
        return -1;
    }
    log_d("succeed to open dest file");

    u8 *wbuf = (u8 *)zalloc(512);
    u8 *rbuf = (u8 *)zalloc(512);
    if (!wbuf || !rbuf) {
        log_e("malloc wbuf or rbuf fail");
        return -ENOMEM;
    }
    //将一个文件的内容拷贝到另一个文件
    int rlen = fread(f0, rbuf, 512);
    while (rlen > 0) {
        int wlen = fwrite(f1, rbuf, rlen);
        rlen = fread(f0, rbuf, 512);
    }

    fclose(f0);
    fclose(f1);
    if (wbuf) {
        free(wbuf);
    }
    if (rbuf) {
        free(rbuf);
    }
#endif

    return 0;
}

/*
 * U盘插拔事件处理
 */
static void udisk_event_handler(struct sys_event *event)
{
    int id = ((char *)event->arg)[5] - '0';
    const char *dev  = udisk_list[id];
    char root_path[32];

    switch (event->u.dev.event) {
    case DEVICE_EVENT_IN:
        printf("\n %s: in\n", dev);
        if (usb_disk_online(dev)) {
            /* usb_mass_storage_fs_test(dev); */
            int udisk_upgrade_detect(void);
            os_time_dly(20); //防止app阻塞
            udisk_upgrade_detect();
        }
        break;
    case DEVICE_EVENT_OUT:
        sprintf(root_path, "storage/%s", dev);
        unmount(root_path);
        printf("\n %s: out\n", dev);
        break;
    }
}

static void device_event_handler(struct sys_event *event)
{
    if (!ASCII_StrCmp(event->arg, "sd*", 4)) {
        sd_event_handler(event);
    } else if (!ASCII_StrCmp(event->arg, "usb", 4)) {
    } else if (!ASCII_StrCmp(event->arg, "udisk*", 7)) {
        udisk_event_handler(event);
    }
}
/*
 * 静态注册设备事件回调函数，优先级为0
 */
SYS_EVENT_HANDLER(SYS_DEVICE_EVENT, device_event_handler, 0);





