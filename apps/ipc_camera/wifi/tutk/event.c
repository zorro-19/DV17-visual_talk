#include "http/http_cli.h"
#include "generic/log.h"
#include "json_c/json.h"
#include "json_c/json_tokener.h"
#include "os/os_api.h"


#define JL_SERVER_ADDR "ipc.jlspx.top:8080"
static httpcli_ctx jl_ctx;
static char access_token[128];
static char refresh_token[128];







static int __http_post_mothed(const char *url, char *buffer, u32 length, int (*cb)(char *, void *), void *priv)
{
    int error = 0;
    http_body_obj http_body_buf;
    httpcli_ctx *ctx = &jl_ctx;

    log_d("url->%s\n", url);

    memset(&http_body_buf, 0x0, sizeof(http_body_obj));
    http_body_buf.recv_len = 0;
    http_body_buf.buf_len = 4 * 1024;
    http_body_buf.buf_count = 1;
    http_body_buf.p = (char *) malloc(http_body_buf.buf_len * sizeof(char));
    if (http_body_buf.p == NULL) {
        return -1;
    }

    memset(ctx, 0x0, sizeof(httpcli_ctx));
    ctx->url = url;
    ctx->priv = &http_body_buf;
    ctx->post_data = buffer;
    ctx->data_len  = length;
    ctx->data_format  = "application/json";
    ctx->connection = "close";
    ctx->timeout_millsec = 6000;

    error = httpcli_post(ctx);
    if (error == HERROR_OK) {
        /* log_d("recv : %s\n", http_body_buf.p); */
        if (cb) {
            error = cb(http_body_buf.p, priv);
        }
    } else {
        error = -1;
    }

    if (http_body_buf.p) {
        free(http_body_buf.p);
    }

    return error;
}


/*登录杰理服务器*/
static int server_login_callback(char *data, void *priv)
{
    /* printf("data=>%s\n",data); */
    json_object *new_obj = NULL;
    json_object *j_access_token = NULL;
    json_object *j_refresh_token = NULL;
    new_obj = json_tokener_parse(data);


    if (!json_object_object_get_ex(new_obj, "access_token", &j_access_token)) {

    }

    if (!json_object_object_get_ex(new_obj, "refresh_token", &j_refresh_token)) {

    }

    const char *access = json_object_get_string(j_access_token);
    const char *refresh = json_object_get_string(j_refresh_token);

    strcpy(access_token, access);
    strcpy(refresh_token, refresh);

    printf("access_token=%s\n", access_token);
    printf("refresh_token=%s\n", refresh_token);

    json_object_put(new_obj);
    return 0;


}

#define JL_SERVER_LOGIN_URL "http://%s/oauth/token?username=%s&password=%s&grant_type=password&scope=all&client_id=client&client_secret=%s&auth_type=device"
int jl_server_login(const char *username, const char *password)
{
    int ret = 0;

    char *buffer = malloc(1024);
    if (!buffer) {
        return -1;
    }

    int length = snprintf(buffer, 1024, JL_SERVER_LOGIN_URL, JL_SERVER_ADDR, username, password, password);

    ret = __http_post_mothed(buffer, NULL, 0, server_login_callback, NULL);

    if (ret) {
        log_e("%s  http_post fail\n", __func__);
        free(buffer);
        return -1;
    }

    free(buffer);
    return ret;

}



static int post_alarm_text_callback(char *data, void *priv)
{
    printf("data=>%s\n", data);
#if 0
    json_object *new_obj = NULL;
    json_object *j_access_token = NULL;
    json_object *j_refresh_token = NULL;
    new_obj = json_tokener_parse(data);


    if (!json_object_object_get_ex(new_obj, "access_token", &j_access_token)) {

    }

    if (!json_object_object_get_ex(new_obj, "refresh_token", &j_refresh_token)) {

    }

    const char *access = json_object_get_string(j_access_token);
    const char *refresh = json_object_get_string(j_refresh_token);

    strcpy(access_token, access);
    strcpy(refresh_token, refresh);

    printf("access_token=%s\n", access_token);
    printf("refresh_token=%s\n", refresh_token);

    json_object_put(new_obj);
#endif
    return 0;


}

#define JL_ALARM_TEXT_URL "http://%s/v1/device/event/alarm_text?access_token=%s&type=%d&data=%s"
int jl_post_alarm_text(u32 type, const char *data)
{
    int ret = 0;

    char *buffer = malloc(1024);
    if (!buffer) {
        return -1;
    }

    int length = snprintf(buffer, 1024, JL_ALARM_TEXT_URL, JL_SERVER_ADDR, access_token, type, data);

    ret = __http_post_mothed(buffer, NULL, 0, post_alarm_text_callback, NULL);

    if (ret) {
        log_e("%s  http_post fail\n", __func__);
        free(buffer);
        return -1;
    }

    free(buffer);
    return ret;

}


#define BOUNDARY                    "----WebKitFormBoundary7MA4YWxkTrZu0gW"
static const char http_head_tab[] =
    "POST /v1/device/event/alarm_file?access_token=%s HTTP/1.1\r\n"
    "Host: ipc.jlspx.top:8080\r\n"
    "User-Agent: */*\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: close\r\n"
    "Content-type: multipart/form-data; boundary="BOUNDARY"\r\n"
    "Content-Length: %d\r\n"
    "\r\n";

static const char http_data_f[] =
    "--"BOUNDARY"\r\n"
    "Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"
    "Content-Type: %s\r\n"
    "\r\n";
static const char http_data_m[] =
    "\r\n--"BOUNDARY"\r\n"
    "Content-Disposition: form-data; name=\"%s\"\r\n\r\n%s";




static const char http_data_b[] =
    "\r\n--"BOUNDARY"--\r\n";






#if 1
static int __post_resources(const char *url, u8 *buffer, u32 length, int (*cb)(char *, void *), void *priv)
{
    int error = 0;
    int url_buf_len = 2048;
    char *url_buf = NULL;
    http_body_obj http_body_buf;

    httpcli_ctx *ctx = &jl_ctx;

    char *http_body_before_buf = NULL;
    int http_body_before_len = 0;

    memset(&http_body_buf, 0x0, sizeof(http_body_obj));

    url_buf = calloc(1, url_buf_len + sizeof(http_data_f) + 64);
    if (!url_buf) {
        goto __checkin_exit;
    }
    http_body_before_buf = url_buf + url_buf_len;

    http_body_buf.recv_len = 0;
    http_body_buf.buf_len = 1024;
    http_body_buf.buf_count = 1;
    http_body_buf.p = (char *) malloc(http_body_buf.buf_len * sizeof(char));
    if (http_body_buf.p == NULL) {
        goto __checkin_exit;
    }

    http_body_before_len = snprintf(http_body_before_buf, sizeof(http_data_f) + 64, http_data_f, "file", "IMG_001.JPG", "image/jpeg");
    if (http_body_before_len >= sizeof(http_data_f) + 64) {
        goto __checkin_exit;
    }


    char *mbuf = calloc(256 * 3, 1);
    if (!mbuf) {
        goto __checkin_exit;
    }

    int mlen[3];

    mlen[0] = snprintf(mbuf, 256 * 3, http_data_m, "type", "1");

    mlen[1] = snprintf(mbuf + mlen[0], 256 * 2, http_data_m, "store_type", "image");

    mlen[2] = snprintf(mbuf + mlen[0] + mlen[1], 256, http_data_m, "filename", "001.jpg");


    printf("mlen[0]:%d  mlen[1]:%d  mlen[2]:%d\n", mlen[0], mlen[1], mlen[2]);

    int http_more_data_addr[6];
    int http_more_data_len[6 + 1];
    http_more_data_addr[0] = (int)(http_body_before_buf);
    http_more_data_addr[1] = (int)(buffer);
    http_more_data_addr[2] = (int)(mbuf);
    http_more_data_addr[3] = (int)(mbuf + mlen[0]);
    http_more_data_addr[4] = (int)(mbuf + mlen[0] + mlen[1]);
    http_more_data_addr[5] = (int)http_data_b;

    http_more_data_len[0] = http_body_before_len;
    http_more_data_len[1] = length;
    http_more_data_len[2] = mlen[0];
    http_more_data_len[3] = mlen[1];
    http_more_data_len[4] = mlen[2];
    http_more_data_len[5] = strlen(http_data_b);
    http_more_data_len[6] = 0;

    memset(ctx, 0x0, sizeof(httpcli_ctx));
    ctx->more_data = http_more_data_addr;
    ctx->more_data_len = http_more_data_len;
    ctx->timeout_millsec = 8000;

    ctx->url = url;
    ctx->priv = &http_body_buf;

    snprintf(url_buf, url_buf_len, http_head_tab, access_token, (int)(http_body_before_len + length + strlen(http_data_b) + mlen[0] + mlen[1] + mlen[2]));
    ctx->user_http_header = url_buf;

    error = httpcli_post(ctx);
    if (error == HERROR_OK) {
        if (cb) {
            error = cb(http_body_buf.p, priv);
        }
    } else {
        error = -1;
    }

__checkin_exit:
    if (url_buf) {
        free(url_buf);
    }
    if (http_body_buf.p) {
        free(http_body_buf.p);
    }
    free(mbuf);

    return error;
}

static int post_res_callback(char *data, void *priv)
{
    printf("data=>%s\n", data);
    return 0;
}


#define JL_ALARM_RES_URL "http://%s/v1/device/event/alarm_file?access_token=%s"
int jl_post_alarm_res(u8 *buffer, u32 length)
{
    int ret = 0;

    char *url = malloc(1024);
    if (!url) {
        return -1;
    }

    snprintf(url, 1024, JL_ALARM_RES_URL, JL_SERVER_ADDR, access_token);

    ret = __post_resources(url, buffer, length, post_res_callback, NULL);

    if (ret) {
        log_e("%s  http_post fail\n", __func__);
        free(buffer);
        return -1;
    }

    free(url);
    return ret;

}






#endif








