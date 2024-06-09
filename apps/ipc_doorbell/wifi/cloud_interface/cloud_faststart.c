#include "app_config.h"
#include "system/includes.h"
#include "doorbell_event.h"
#ifdef PRODUCT_TEST_ENABLE
#include "datatype.h"
#endif

/***
*采用52 index存放vm info
*/
struct fastdata_vm_info {
    int len;
};
static int cfg_write_addr = 0x3BB000;
static int fastdata_write_addr;
static int backdata_write_addr;
static int video_plane_write_addr;

void dev_on_backup(uint8_t *data, size_t len)
{

    put_buf(data,len);
    printf("\n >>>>>>>>>>>>>>%d,%s %d\n",len,__FUNCTION__,__LINE__);
    void *dev;
    dev = dev_open("spiflash", NULL);
    if (!dev) {
        goto exit;
    }
    dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, 0);
    dev_ioctl(dev, IOCTL_ERASE_SECTOR, backdata_write_addr);
    dev_bulk_write(dev, data, backdata_write_addr, len); //此接口需要擦除flash之后才能写入
    extern int spi_flash_write_protect();
    spi_flash_write_protect();

    struct fastdata_vm_info info;
    info.len = len;

    db_update_buffer(BACKUP_VM_INDEX, &info, sizeof(struct fastdata_vm_info));
exit:
    if (dev) {
        dev_close(dev);
    }

}


int iot_dev_read_backup_data(char *data, int len)
{
    void *fd;
    fd = dev_open("spiflash", NULL);
    if (!fd) {
        goto exit;
    }

    printf("\n  len:::%d\n",len);
    struct fastdata_vm_info info;
    int ret = db_select_buffer(BACKUP_VM_INDEX, &info, sizeof(struct fastdata_vm_info));


    printf("\n info.len=========================%d\n ",info.len);
    if (ret != sizeof(struct fastdata_vm_info) || info.len == 0) {
        return 0;
    }

    dev_bulk_read(fd, data, backdata_write_addr, len);
exit:
    if (fd) {
        dev_close(fd);
    }
   // put_buf(data,len);
    return info.len;
}

void clean_backup_data_to_flash(void)
{
   #if 1
    struct fastdata_vm_info info;
    info.len = 0;
    db_update_buffer(BACKUP_VM_INDEX, &info, sizeof(struct fastdata_vm_info));
   #endif
}

void clean_fastdata_to_flash(void)
{
    struct fastdata_vm_info info;
    info.len = 0;
    db_update_buffer(FASTSTART_VM_INDEX, &info, sizeof(struct fastdata_vm_info));
}

int write_data_to_flash(char *data, int len)
{
    void *dev;
    dev = dev_open("spiflash", NULL);
    if (!dev) {
        goto exit;
    }
    dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, 0);
    dev_ioctl(dev, IOCTL_ERASE_SECTOR, fastdata_write_addr);
    dev_bulk_write(dev, data, fastdata_write_addr, len); //此接口需要擦除flash之后才能写入
    extern int spi_flash_write_protect();
    spi_flash_write_protect();
    struct fastdata_vm_info info;
    info.len = len;

    db_update_buffer(FASTSTART_VM_INDEX, &info, sizeof(struct fastdata_vm_info));
exit:
    if (dev) {
        dev_close(dev);
    }
    return 0;
}
int read_data_for_flash(char *data, int len)
{

    void *fd;
    fd = dev_open("spiflash", NULL);
    if (!fd) {
        goto exit;
    }
    struct fastdata_vm_info info;
    int ret = db_select_buffer(FASTSTART_VM_INDEX, &info, sizeof(struct fastdata_vm_info));

    if (ret != sizeof(struct fastdata_vm_info) || info.len == 0) {
        return 0;
    }

    dev_bulk_read(fd, data, fastdata_write_addr, len);
exit:
    if (fd) {
        dev_close(fd);
    }

    return info.len;
}

int get_video_planetime_vmaddr(){

    return  video_plane_write_addr;

}

void dev_write_plane_time_vm(uint8_t *data, size_t len)
{
    put_buf(data,len);
    printf("\n >>>>>>>>>>>>>>%s %d\n",__func__,__LINE__);
    void *dev;
    dev = dev_open("spiflash", NULL);
    if (!dev) {
        goto exit;
    }
    dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, 0);
    dev_ioctl(dev, IOCTL_ERASE_SECTOR, video_plane_write_addr);
    dev_bulk_write(dev, data, video_plane_write_addr, len); //此接口需要擦除flash之后才能写入
    extern int spi_flash_write_protect();
    spi_flash_write_protect();

exit:
    if (dev) {
        dev_close(dev);
    }

}

int dev_read_plane_time_vm(char *data, int len)
{
    void *fd;
    fd = dev_open("spiflash", NULL);
    if (!fd) {
        goto exit;
    }

    dev_bulk_read(fd, data, video_plane_write_addr, len);
exit:
    if (fd) {
        dev_close(fd);
        //put_buf(data,len);
    }else{

    return 0;

    }

    return len;
}
void get_flash_idle_info(void)
{
    void *dev;
    u32 flash_size;
    dev = dev_open("spiflash", NULL);
    if (!dev) {
        printf("\n %s %d err\n", __func__, __LINE__);
        goto exit;
    }
    dev_ioctl(dev, IOCTL_GET_CAPACITY, (u32)&flash_size);

    printf("\n flash_size :%x,%x\n",flash_size,get_vm_usage());
    flash_size -= get_vm_usage();
    printf("\n flash_size = 0x%x  \n", flash_size); //此地址为did写入地址
    flash_size -= 64 * 1024;
    printf("\n flash_size = 0x%x  \n", flash_size); //此地址为did写入地址



    //did占用4k
    //cfg_write_addr = flash_size;//此地址为did写入地址
    fastdata_write_addr = flash_size + 4 * 1024;
    backdata_write_addr = fastdata_write_addr + 4 * 1024;
    video_plane_write_addr = backdata_write_addr + 4 * 1024;

    printf("\n cfg_write_addr = 0x%x\n", flash_size); //此地址为did写入地址
exit:
    if (dev) {
        dev_close(dev);
    }
}

void doorbell_read_cfg_info(u8 *cfg, int len)
{
#if 0
    void *dev = dev_open("spiflash", NULL);
    if (dev) {
        dev_bulk_read(dev, cfg, cfg_write_addr, len);
        dev_close(dev);
        for (int i = 0; i < len; i++) {
            if (cfg[i] == 0xff) {
                cfg[i] = 0;
                break;
            }
        }
    }
#else
    int idx = 0;
    product_read_license(&idx, cfg , &len);
    printf("\n >>>>>>>>>>>>>>>len = %d\n",len);
#endif
}
void doorbell_write_cfg_buf_info(void *buf, int len)
{
#if 0
    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        goto __exit;
    }
    dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, 0);
    dev_ioctl(dev, IOCTL_ERASE_SECTOR, cfg_write_addr);
    dev_bulk_write(dev, buf, cfg_write_addr, len); //此接口需要擦除flash之后才能写入
    extern int spi_flash_write_protect();
    spi_flash_write_protect();
__exit:
    if (dev) {
        dev_close(dev);
    }
#else
    u8 product_write_license(u8 idx, u8 *buf, u32 len, u32 file_size);
    u8 idx = 0;
    product_write_license(0, buf, len, len); //这里设置buf len = file_size就行
#endif // 0
}


void doorbell_write_cfg_info(void)
{
#define CFG_LEN 1024
    void *fd = NULL;
    struct vfscan *fs = NULL;
    void *cfg =  NULL;

    void *dev = dev_open("spiflash", NULL);
    if (!dev) {
        goto __exit;
    }
    if (storage_device_ready()) {
        fs = fscan(CONFIG_ROOT_PATH, "-tINI -st");
        if (!fs) {
            goto __exit;
        }
        fd = fselect(fs, FSEL_FIRST_FILE, 0);
        if (fd) {
            cfg = calloc(1, CFG_LEN);
            if (!cfg) {
                goto __exit;
            }
            char name[128];
            fget_name(fd, name, sizeof(name));
            printf("\n name = %s\n", name);

            int len = fread(fd, cfg, CFG_LEN);
#if 0
            dev_ioctl(dev, IOCTL_SET_WRITE_PROTECT, 0);
            dev_ioctl(dev, IOCTL_ERASE_SECTOR, cfg_write_addr);
            dev_bulk_write(dev, cfg, cfg_write_addr, CFG_LEN); //此接口需要擦除flash之后才能写入
            extern int spi_flash_write_protect();
            spi_flash_write_protect();
#else
            u8 product_write_license(u8 idx, u8 *buf, u32 len, u32 file_size);
            u8 idx = 0;
            u8 ret=product_write_license(0, cfg, len, len); //这里设置buf len = file_size就行
            printf("\n ========ret========%d\n",ret);
#endif
            #ifdef PRODUCT_TEST_ENABLE
            if(ret==ERR_ALREADY_EXIST){
              printf("\n license is exist\n ");
             goto __exit;
            }
            #endif
            fdelete(fd);
        }
    }
__exit:
    if (cfg) {
        free(cfg);
    }
    if (fs) {
        fscan_release(fs);
    }
    if (dev) {
        dev_close(dev);
    }
}

int doorbell_read_cfg_did(char *buf, int buf_len)
{
    char *str = NULL;
    char *cfg = calloc(1, 1024);
    if (!cfg) {
        printf("\n %s %d\n", __func__, __LINE__);
        goto __exit;
    }
    doorbell_read_cfg_info(cfg, 1024);
    if (!strlen(cfg)) {
        printf("\n %s %d\n", __func__, __LINE__);
        free(cfg);
        goto __exit;
    }

    str = strstr(cfg, "did = ");
    if (str) {
        str += strlen("did = ");
        memset(buf, 0, buf_len);
        memcpy(buf, str, strstr(str, "\n") - str);
        printf("\n did = %s\n", buf);
    }

    free(cfg);
    return strlen(buf);
__exit:
    return 0;
}
