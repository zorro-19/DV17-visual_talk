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
 * $Id: http.c 3/14/2009 windyrobin@Gmail.com nanjing china$
 */

#include "websrv_http.h"
#include "wrhash.h"
#include "websrv_io.h"
#include "sock_api.h"
#include "os/os_compat.h"

/*defined in request.c*/
extern int  requestHandler(void * s);

struct WrContext WrContext;


/*copy form moogoose*/
static const struct {
	const char	*ext;
	const char	*type;
} mmt[] = {
	{"html",	"text/html"			},
	{"htm",		"text/html"			},
	{"shtm",	"text/html"			},
	{"shtml",	"text/html"			},
	{"css",		"text/css"			},
	{"js",		"application/x-javascript"	},
	{"ico",		"image/x-icon"			},
	{"gif",		"image/gif"			},
	{"jpg",		"image/jpeg"			},
	{"jpeg",	"image/jpeg"			},
	{"png",		"image/png"			},
	{"svg",		"image/svg+xml"			},
	{"torrent",	"application/x-bittorrent"	},
	{"wav",		"audio/x-wav"			},
	{"mp3",		"audio/x-mp3"			},
	{"mid",		"audio/mid"			},
	{"m3u",		"audio/x-mpegurl"		},
	{"ram",		"audio/x-pn-realaudio"		},
	{"ra",		"audio/x-pn-realaudio"		},
	{"doc",		"application/msword",		},
	{"exe",		"application/octet-stream"	},
	{"zip",		"application/x-zip-compressed"	},
	{"xls",		"application/excel"		},
	{"tgz",		"application/x-tar-gz"		},
	{"tar.gz",	"application/x-tar-gz"		},
	{"tar",		"application/x-tar"		},
	{"gz",		"application/x-gunzip"		},
	{"arj",		"application/x-arj-compressed"	},
	{"rar",		"application/x-arj-compressed"	},
	{"rtf",		"application/rtf"		},
	{"pdf",		"application/pdf"		},
	{"swf",		"application/x-shockwave-flash"	},
	{"mpg",		"video/mpeg"			},
	{"mpeg",	"video/mpeg"			},
	{"asf",		"video/x-ms-asf"		},
	{"avi",		"video/x-msvideo"		},
	{"bmp",		"image/bmp"			},
	{NULL,		NULL}
};

int websrv_uninit(void)
{
    struct CliContext  *cli;
    struct list_head *pos;

	WrContext.quitFlag = 1;
	thread_kill(&WrContext.pid, KILL_WAIT);
	sock_unreg(WrContext.srv_sockhdl);

    while(1)
    {
        os_mutex_pend(&WrContext.mutex,0);

        if(list_empty(&WrContext.cli_list_head))
        {
            os_mutex_post(&WrContext.mutex);
            break;
        }
        cli = list_first_entry(&WrContext.cli_list_head, struct CliContext, entry);
        os_mutex_post(&WrContext.mutex);

        websrv_disconnect_cli(cli);
    }

    os_mutex_del(&WrContext.mutex,1);

	wr_hash_clear(&WrContext.mimeMap);
	wr_hash_clear(&WrContext.pageMap);

	return 0;
}

void websrv_disconnect_cli(char *ipaddr)
{
    struct list_head *pos_cli,*node;
    struct CliContext *cli;
    bool find_cli = 0;

//    puts("|websrv_disconnect_cli\n");
    os_mutex_pend(&WrContext.mutex,0);
    list_for_each_safe(pos_cli, node, &WrContext.cli_list_head)
    {
        cli = list_entry(pos_cli, struct CliContext,entry);
        if(0 == strcmp(ipaddr,cli->ipaddr))
        {
            find_cli = 1;
            --WrContext.ClientCnt;
            list_del(&cli->entry);
            break;
        }
    }
    os_mutex_post(&WrContext.mutex);

    if(!find_cli)
    {
//        puts("|websrv_disconnect_cli_not_find_cli\n");
        return;
    }
    cli->quitFlag = 1;
    thread_kill(&cli->pid, KILL_WAIT);
    sock_unreg(cli->sock_hdl);
    printf("websrv_disconnect_cli %s OK!\n", cli->ipaddr);
    free(cli);
}

static int wr_cli_sock_cb(enum sock_api_msg_type type, void* priv)
{
    struct CliContext *cli = (struct CliContext *)priv;

    if(WrContext.quitFlag || cli->quitFlag)
        return -1;

    return 0;
}

static int wr_http_sock_cb(enum sock_api_msg_type type, void* priv)
{
    if(WrContext.quitFlag)
        return -1;

    return 0;
}

static void wr_http_start(void *arg)
{
	struct list_head *pos;
	struct sockaddr_in  client_addr;
	socklen_t socketLength = sizeof(struct sockaddr_in);

    WEB_DBG("\n wrHttp is running...\n");

    while(1)
    {
        struct CliContext *cli = (struct CliContext *)calloc(sizeof(struct CliContext), 1);
        if(cli == NULL)
        {
            printf("%s %d->Error in calloc()\n", __FUNCTION__, __LINE__);
            msleep(100);
            continue;
        }

        cli->sock_hdl = sock_accept(WrContext.srv_sockhdl, (struct sockaddr *)&client_addr, &socketLength, wr_cli_sock_cb, (void *)cli);
        if (cli->sock_hdl == NULL)
        {
            free(cli);
            printf("%s %d->Error in sock_accept()\n",__FUNCTION__, __LINE__);
            break;
        }
        printf("web_http connected form %s:%d \n", inet_ntoa(client_addr.sin_addr.s_addr),htons(client_addr.sin_port));

        websrv_disconnect_cli(inet_ntoa(client_addr.sin_addr.s_addr));

        while(WrContext.ClientCnt > WR_SOCK_BACKLOG)
        {
            printf("web_http_server too much clients connected, waitting other client exit...\n");
            msleep(1000);
        }

        strcpy(cli->ipaddr, inet_ntoa(client_addr.sin_addr.s_addr));

        os_mutex_pend(&WrContext.mutex,0);
        ++WrContext.ClientCnt;
        list_add_tail(&cli->entry, &WrContext.cli_list_head);
        os_mutex_post(&WrContext.mutex);

        thread_fork("requestHandler", 10, 4000, 0, &cli->pid,requestHandler, (void *)cli);
    }
}

const char *get_mime_type(const char *path)
{
	const char	*extension;
	const char *ret=NULL;
	if (path!=NULL && (extension = strrchr(path, '.')) != NULL) {
		ret = wr_hash_find(&WrContext.mimeMap ,++extension);
	}
	return (ret!=NULL) ? ret : ("text/plain");
}

void cgi_page_add(const char * pageName,\
				  void *f)
{
	wr_hash_add(&WrContext.pageMap ,pageName ,f);
}

void* cgi_page_find(const char *pageName)
{
	return wr_hash_find(&WrContext.pageMap ,pageName);
}

void websrv_index_html_reg(const char *fname, const char *contents, unsigned int len)
{
    WrContext.index_html_fn = fname;
    websrv_virfile_reg(fname,contents,len);
}

int websrv_init(void)
{
	int i=-1;
	/*(97+97) * 4 = 800 ,assume page number is 50 and mimitype is 50 ,
	so 100*sizeof(hash_list) = 1200 ,so 2048 is just ok*/
	static char pool[2048];
	static wr_mpool mpool;

	memset(&WrContext, 0, sizeof(struct WrContext));

	wr_mpool_init(&mpool ,pool ,sizeof(pool));
	wr_hash_init(&WrContext.mimeMap ,&mpool ,97);
	wr_hash_init(&WrContext.pageMap ,&mpool ,97);
	WrContext.pageMap.hashcmp = wr_hashcasecmp_str;
	WrContext.mimeMap.hashcmp = wr_hashcasecmp_str;
	/*add mime type map*/
	while(mmt[++i].ext != NULL){
		wr_hash_add(&WrContext.mimeMap ,mmt[i].ext ,mmt[i].type);
	}

    os_mutex_create(&WrContext.mutex);
	INIT_LIST_HEAD(&WrContext.cli_list_head);

    return 0;
}

int websrv_start(void)
{
    if(WrContext.index_html_fn == NULL)
    {
        puts("websrv_init FAIL, AT LEAST REG INDEX_HTML!\n");
        return -1;
    }

    WrContext.srv_sockhdl = sock_reg(AF_INET, SOCK_STREAM, 0, wr_http_sock_cb, NULL);
    if (WrContext.srv_sockhdl == NULL)
    {
        errorLog("wr_http_start socket error");
        goto EXIT;
    }

    if(sock_set_reuseaddr(WrContext.srv_sockhdl))
    {
        errorLog("wr_http_start sock_set_reuseaddr error");
        goto EXIT;
    }
    struct sockaddr _ss;
    struct sockaddr_in *dest_addr = (struct sockaddr_in *)&_ss;
    dest_addr->sin_family = AF_INET;
    dest_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr->sin_port = htons(WEB_SRV_PORT);
    if (sock_bind(WrContext.srv_sockhdl, (struct sockaddr *)&_ss, sizeof (_ss)))
    {
        errorLog("wr_http_start bind error");
        goto EXIT;
    }

    if (sock_listen(WrContext.srv_sockhdl, WR_SOCK_BACKLOG))
    {
        errorLog("wr_http_start sock_listen error");
        goto EXIT;
    }

    thread_fork("wr_http_start", 11, 4000, 0, &WrContext.pid,wr_http_start, NULL);

    return 0;

EXIT:

    if(WrContext.srv_sockhdl)
        sock_unreg(WrContext.srv_sockhdl);

    return -1;
}
