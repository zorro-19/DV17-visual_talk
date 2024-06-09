#include "system/includes.h"
#include "app_config.h"


#define FILE_SIZE   20 * 1024 * 1024

static u8 write_buf[32 * 1024 + 500];

static void fs_write_test()
{

    for (int step = 0; step < 2048; step++) {

        int wlen = 1;

        FILE *f = fopen(CONFIG_STORAGE_PATH"/C/CC/wt_001.m", "w+");

        if (!f) {
            return;
        }

        memset(write_buf, (0x5A + step) & 0xff, sizeof(write_buf));


        for (int i = 0; i < FILE_SIZE;) {
            int len = fwrite(f, write_buf, wlen);
            if (len != wlen) {
                log_e("fwrite: %x, %x, %x\n", i, len, wlen);
                return;
            }
            i += wlen;
            if (i == FILE_SIZE) {
                break;
            }
            wlen +=  jiffies % sizeof(write_buf);
            if (wlen > sizeof(write_buf)) {
                wlen = 2;
            }
            if (i + wlen > FILE_SIZE) {
                wlen = FILE_SIZE - i;
            }
        }
        printf("write_ok: %x, %x\n", wlen, fpos(f));
        fseek(f, 0, SEEK_SET);

        int rlen = 1024;

        for (int i = 0; i < FILE_SIZE; i += rlen) {
            memset(write_buf, 0, rlen);
            int len = fread(f, write_buf, rlen);
            if (len != rlen) {
                log_e("fread: %x, %x, %x\n", len, rlen, fpos(f));
                fclose(f);
                return;
            }
            for (int i = 0; i < rlen; i++) {
                if (write_buf[i] != ((0x5A + step) & 0xff)) {
                    log_e("not eq: %x, %x, %x\n", write_buf[i], fpos(f), i);
                    fclose(f);
                    return;
                }
            }
            if (i + rlen >= FILE_SIZE) {
                rlen = FILE_SIZE - i;
            }
        }

        log_d("---------------------fs_write_suss: %d\n", step);

        fdelete(f);
    }

}

static void fs_test_task(void *p)
{
    OS_SEM sem;

    os_sem_create(&sem, 0);

    while (1) {
        fs_write_test();
        os_sem_pend(&sem, 0);
    }

}



static int fs_test_init()
{
    task_create(fs_test_task, NULL, "fs_test");
    return 0;
}
/*late_initcall(fs_test_init);*/
