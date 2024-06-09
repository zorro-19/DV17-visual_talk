
#include "pkg_recover.h"
#include "app_config.h"
#include "fs.h"
#include "lwip/sockets.h"




#define UNPKG_ERR() do {printf("%s  %d\n",__func__,__LINE__);return -1;}while(0)

int seek_to_moov(FILE *fp)
{
    int offset_moov = 0;
    if (fseek(fp, 0x1C, SEEK_SET) == false) {
        UNPKG_ERR();
    }
    //fread(&offset_moov,1,4,fp);
    if (fread(fp, &offset_moov, 4) != sizeof(u32)) {
        UNPKG_ERR();
    }
    if (fseek(fp, 0x1C + htonl(offset_moov), SEEK_SET) == false) {
        UNPKG_ERR();

    }
    return 0x1C + htonl(offset_moov);

}

int is_vaild_mov_file(FILE *fp)
{
    int result1 = 0;
    int result2 = 0;
    u32 size = 0;
    int offset = 0;
    char moov[9] = {0};
    offset = seek_to_moov(fp);
    fread(fp, moov, 8);
    if (strcmp(moov + 4, "moov")) {
        result1 = 1;
    }
    fseek(fp, offset + 0x1BD + 24, SEEK_SET);

    fread(fp, moov, 8);

    if (memcmp(moov + 4, "avc", 3)) {
        result2 = 1;
    }

    if (result1 || result2) {
        return 0;
    } else {
        return 1;
    }


}




int recover_file()
{

    struct vfscan *fs = NULL;
    void *fd = NULL;
    char file_name[32];
    char path[256];

    log_d("recover_test start\n");
    fs = fscan(CONFIG_REC_PATH_0, "-tMOV -sn");
    if (fs == NULL) {
        log_e("fscan fail\n");
        return -1;
    }


    while (1) {
        if (fd == NULL) {
            fd = fselect(fs, FSEL_FIRST_FILE, 0);
        } else {
            fd = fselect(fs, FSEL_NEXT_FILE, 0);
        }
        if (fd == NULL) {
            break;
        }

        if (!is_vaild_mov_file(fd)) {
            log_w(" Invalid MOV , recover\n");
            fget_name(fd, file_name, 32);
            sprintf(path, CONFIG_REC_PATH_0"%s", file_name);
            log_d("need recover path=>%s\n", path);
            void *pkg = NULL;
            pkg = pkg_rcv_open(path, "mov");

            if (!pkg) {
                log_e("rcv_fail\n\n");
                fclose(fd);
                continue;

            }

            pkg_rcv(pkg);

            pkg_rcv_close(pkg);
            log_d("recover successful\n");


        }

        fclose(fd);
    }

    fscan_release(fs);

    log_d("recover_test end\n");
    return 0;

}
