#include <curl/curl.h>

int fileno = 0;


size_t write_function(char *buffer,
                      size_t size,
                      size_t nitems,
                      void *outstream)
{

    printf("outstream:%s \n", buffer);
    return nitems * size;

}

size_t read_function(char *buffer,
                     size_t size,
                     size_t nitems,
                     void *outstream)
{

    printf("instream:%s \n", buffer);
    return nitems * size;

}




int curl_test(void)
{
    CURL *curl = NULL;
    CURLcode res;
    char *view;
    printf("CURL testing...\n");


    curl = curl_easy_init();
    printf("%s %d\n", __func__, __LINE__);
    if (curl) {

        curl_easy_setopt(curl, CURLOPT_URL, "https://asia-vsaasapi-tutk.kalayservice.com/api/v1/stream/stream_url/DVUUB17CKDVCSH6GUHC1?stoken=F6Y3LJeJ9hPlQaiTaoH707J9nbHF3is2quZAFHG7rdq8aXtXjOj_fue35u2hHYRS3-wCeLcesKXzp053lQWt04PdIXsU4YVZ2UDljOC2PL4=&starttime=1630038100&protocol=tutk&event_id=999&event_file=resource.bin&media_type=0");
        //一定要重定义这两回调
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_function);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            printf("Error : %s, code : %d\n", curl_easy_strerror(res), res);
        } else {
            printf("curl_easy_perform OK !!!\n");
        }

        res = curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &view);
        view[strlen(view)] = '\0';

        if (res != CURLE_OK) {
            printf("Error code : %d\n", res);
            printf("Error : %s\n", curl_easy_strerror(res));
        } else {
            printf("view IP : %s\n", view);
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    } else {
        printf("curl_easy_init error\n");
    }
    return 0;
}


size_t write_callbackfun(char *buffer,
                         size_t size,
                         size_t nitems,
                         void *outstream)
{

    printf("outstream:%s \n", buffer);
    return nitems * size;

}

struct data_struct {

    char *data;
    int   data_len;
    int   pos;
};
static void *wfile = NULL;
size_t read_callbackfun(char *buffer,
                        size_t size,
                        size_t nitems,
                        void *outstream)
{
    struct data_struct *d = (struct data_struct *)outstream;

    int block_size = 0;

    block_size = d->data_len > (size * nitems) ? (size * nitems) : d->data_len;


    /* printf("d->pos:%d   read size:%d  data_len:%d\n",d->pos,block_size,d->data_len); */

    memcpy(buffer, d->data + d->pos, block_size);
    fwrite(wfile, buffer, block_size);
    /* printf("%s",buffer); */
    /* if(block_size < 16*1024) { */
    /* printf("json_data:%s",buffer+block_size -512); */
    /* } */
    d->pos += block_size;
    d->data_len -= block_size;

    return block_size;

}
#include "app_config.h"
#include "fs/fs.h"
#define BUFFER_SIZE  (400 * 1024)

static char indata[BUFFER_SIZE];
static char outdata[BUFFER_SIZE * 2];
#define DATA_FORMAT  "{\"date\":\"%s\",\"sn\":\"%s\",\"img\":\"data:image/jpeg;base64,%s\"}"
int upload_file(char *imagePath)
{
    bool res = false;
    int  ret = 0;
    int output_len = 0;
    char *json_data = NULL;
    printf("line:%d upload_file start\n", __LINE__);
    //read  file
    if (!imagePath) {
        printf("argument can't be empty\n");
        return false;
    }
    FILE *file = fopen(imagePath, "r");
    if (file == NULL) {
        printf("%s open file failed\n", imagePath);
        return false;
    }


    wfile = fopen(CONFIG_ROOT_PATH"text.txt", "w+");
    if (file == NULL) {
        printf("%s open file failed\n", imagePath);
        return false;
    }




    int real_size = fread(file, indata, BUFFER_SIZE);

    fclose(file);


    //base64 enc
    /* int mbedtls_base64_encode(unsigned char *dst, size_t dlen, size_t *olen, */
    /* const unsigned char *src, size_t slen) */

    put_buf(indata, 32);

    mbedtls_base64_encode(outdata, BUFFER_SIZE * 2, &output_len, indata, real_size);



    /* printf("outdata:%s\n",outdata); */
    printf("\n filesize==============%d\n ", real_size);
    printf("output_len:%d\n", output_len);



    //json
    json_data = calloc(output_len + 512, 1);

    if (!json_data) {

        return -1;
    }

    int size = snprintf(json_data, output_len + 512, DATA_FORMAT, "2021-09-13  09:50:50", "123123123", outdata);




    printf("json_data:%s\n", json_data + size - 512);


    struct data_struct d;

    d.data = json_data;
    d.data_len = size;
    d.pos    = 0;



    struct curl_slist *headers = NULL;
    CURL *curl;
    char url[1024] = {0};
    curl  = curl_easy_init();
    if (curl) {
        sprintf(url, "https://testapi.xiaoxiangchengbo.com/receivePhoto");

        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callbackfun);

        curl_easy_setopt(curl, CURLOPT_READDATA, &d);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callbackfun);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        struct curl_slist *list = NULL;

        list  = curl_slist_append(list, "Content-Type: application/json; charset=utf-8");
        char buf[64] = {0};
        sprintf(buf, "Content-Length:%d", size);
        list  = curl_slist_append(list, buf);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);


        /* curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); */

        /* 指定图片大小，否则遇到'\0'就停止了*/
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, size);

        printf("start send file to server\n");

        CURLcode code = curl_easy_perform(curl);
        if (code == CURLE_OK) {
            res = true;
        } else {
            printf("line:%d curl_easy_perform() failed: %s\n", __LINE__, curl_easy_strerror(code));
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    fclose(wfile);
    free(json_data);


    return res;
}



