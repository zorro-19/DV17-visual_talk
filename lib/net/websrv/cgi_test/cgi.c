/*
 * Copyright (c) 2009 windyrobin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * $Id: cgi.c 3/14/2009 windyrobin@Gmail.com nanjing china$
 */

#include "websrv_io.h"
#include "misc.h"
#include "http.h"
#include "wrio.h"
#include <stdlib.h>

/**************************************************************/
enum {
    WEB_VOLUM_DOWN = 0,
    WEB_VOLUM_UP,
    WEB_MUSIC_MODE,
    WEB_MUSIC_STOP,
};

extern const unsigned char FAVICON_ICO_FNAME[];
extern const unsigned char FAVICON_ICO[4286];
extern const unsigned char INDEX_HTML_FNAME[];
extern const unsigned char INDEX_HTML[1992];
extern const unsigned char INDEX_HTML_FNAME2[];
extern const unsigned char INDEX_HTML2[2319];
extern const unsigned char JQUERY_MIN_JS_FNAME[];
extern const unsigned char JQUERY_MIN_JS[95702];

static int cgi_page_login(WrHttp *pHttp);
static int cgi_ap_sp_switch(WrHttp *pHttp);
static int cgi_name_get(WrHttp *pHttp);
static int cgi_volume_get(WrHttp *pHttp);
/**************************************************************/


/**************************************************************/
struct cgi_page_table {
    const char *page;
    const int (*page_action)(WrHttp *);
};

//cgi列表,根据需要添加表项
static struct cgi_page_table page_table[] = {
    //cgi名        回调函数
    {"login.cgi", cgi_page_login},
    {"ap_sp_switch.cgi", cgi_ap_sp_switch},
    {"volume.cgi", cgi_volume_get},
};

struct html_page_table {
    const unsigned char *html_name;
    const unsigned char *html_data;
    const unsigned int data_size;
};

//网页列表, 根据需要添加网页
static struct html_page_table html_table[] = {
    //网页名             网页数据          数据大小
    {INDEX_HTML_FNAME2, INDEX_HTML2, sizeof(INDEX_HTML2)},	    //网页2
    {INDEX_HTML_FNAME, INDEX_HTML, sizeof(INDEX_HTML)},   		//网页1
};

//注册网页
static void html_page_register(const struct html_page_table *html_table, const int page_num)
{
    int i = 0;
    int ret = 0;

    ret = page_num;
    for (i; i < ret; i++) {
        websrv_index_html_reg(html_table[i].html_name, html_table[i].html_data, html_table[i].data_size);
    }
}

//注册cgi
static void web_page_register(const struct cgi_page_table *page_table, const int page_num)
{
    int i = 0;
    int ret = 0;

    ret = page_num;
    for (i; i < ret; i++) {
        cgi_page_add(page_table[i].page, page_table[i].page_action);
    }
}

/**************************************************************/

static int cgi_page_login(WrHttp *pHttp)
{
    const char *smsg = "login success";
    const char *emsg = "login error";
    static const char *user = "8188AP";
    static const char *pwd = "88888888";

    const char *pRet = emsg;

    const char *pUser, *pPwd;
    const char *date;
    WEB_DBG("\n--login.cgi--\n");
    print_param(pHttp);

    pUser = get_param_info(pHttp, "user");
    pPwd = get_param_info(pHttp, "pwd");

    if (strcmp(user, pUser) == 0 && strcmp(pwd, pPwd) == 0) {
        pRet = smsg;
    }

    wr_send_msg(pHttp, NULL, pRet, strlen(pRet));

    return 0;
}

static int cgi_ap_sp_switch(WrHttp *pHttp)
{
    const char *param;
    const char *ap_mode = "switch to AP mode";
    const char *sta_mode = "switch to STA mode";

    printf("\n--ap_sp_switch.cgi--\n");
    print_param(pHttp);

    param = get_param_info(pHttp, "ap_sp_switch");

    return 0;
}

static int cgi_name_get(WrHttp *pHttp)
{
    const char *param;
    print_param(pHttp);

    param = get_param_info(pHttp, "cname");

    wr_send_msg(pHttp, NULL, param, strlen(param));

    return 0;
}

static int cgi_volume_get(WrHttp *pHttp)
{
    const char *param;

    print_param(pHttp);

    param = get_param_info(pHttp, "data");

    /* extern int html_music_play(int value); */

    switch (atoi(param)) {
    case WEB_VOLUM_DOWN:
        puts("volume down");
        /* html_music_play(0); */
        break;

    case WEB_VOLUM_UP:
        puts("volume up");
        /* html_music_play(1); */
        break;

    case WEB_MUSIC_MODE:
        puts("music play mode");
        /* html_music_play(2); */
        break;

    case WEB_MUSIC_STOP:
        puts("music play stop/start");
        /* html_music_play(3); */
        break;

    default:
        break;
    }

    return 0;
}

//web服务器测试
//注意：端口设置在websrv_http.h文件中 (WEB_SRV_PORT)
void websrv_cgi_test(void)
{
    websrv_init();

    //注册网页
    html_page_register(html_table, sizeof(html_table) / sizeof(struct html_page_table));

    websrv_virfile_reg(JQUERY_MIN_JS_FNAME, JQUERY_MIN_JS, sizeof(JQUERY_MIN_JS));
    websrv_virfile_reg(FAVICON_ICO_FNAME, FAVICON_ICO, sizeof(FAVICON_ICO));

    //注册cgi
    web_page_register(page_table, sizeof(page_table) / sizeof(struct cgi_page_table));

    websrv_start();

    /* websrv_uninit(); */
}

//该代码段放置在app_music.c文件中
/* int html_music_play(int value) */
/* { */
/* switch (value) */
/* { */
/* case 0: */
/* if (0 == __set_dec_volume(-VOLUME_STEP) && 0 == __this->dec_ops->dec_volume(-VOLUME_STEP)) { */
/* #ifdef CONFIG_STORE_VOLUME */
/* db_update("vol", __this->volume); */
/* db_flush(); */
/* #endif */
/* } */
/* break; */

/* case 1: */
/* if (0 == __set_dec_volume(VOLUME_STEP) && 0 == __this->dec_ops->dec_volume(VOLUME_STEP)) { */
/* #ifdef CONFIG_STORE_VOLUME */
/* db_update("vol", __this->volume); */
/* db_flush(); */
/* #endif */
/* } */
/* break; */

/* case 2: */
/* local_music_play_mode_switch(); */
/* break; */

/* case 3: */
/* if (__this->play_voice_prompt) { */
/* app_music_stop_voice_prompt(); */
/* } else { */
/* __this->dec_ops->dec_play_pause(1); */
/* } */
/* break; */

/* default: */
/* break; */

/* } */
/* return 0; */
/* } */
