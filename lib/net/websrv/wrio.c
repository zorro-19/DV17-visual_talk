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
 * $Id: wrio.c 3/14/2009 windyrobin@Gmail.com nanjing china$
 */
#include "websrv_io.h"
#include "wrstring.h"
#include "sock_api/sock_api.h"
#include "stdlib.h"

extern u32 timer_get_ms(void);
typedef struct _virtual_file
{
    const char *fname;
	const char *contents;
	unsigned int len;
	unsigned int pos;
	long mtime;
	struct _virtual_file *next;
}virtual_file;
virtual_file *virtual_files_head = NULL;

int websrv_virfile_reg(const char *fname, const char *contents, unsigned int len)
{
    virtual_file *virtfile = malloc(sizeof(virtual_file));
    if(virtfile == NULL)
    {
        perror("websrv_virfile_reg malloc == NULL\n");
        return -1;
    }

    virtfile->fname = fname;
    virtfile->contents = contents;
    virtfile->len = len;
    virtfile->pos = 0;
    virtfile->mtime = timer_get_ms();
    virtfile->next = virtual_files_head;
    virtual_files_head = virtfile;

    return 0;
}

int websrv_virfile_unreg(const char *filePath)
{
    virtual_file *virtfile = virtual_files_head;

	while (virtfile != NULL)
	{
		if (!strcmp(filePath, virtfile->fname))
		{
		    free(virtfile);
		    virtfile = NULL;
            return 0;
		}
		virtfile = virtfile->next;
	}
	return -1;
}

virtual_file *virtual_open(const char *filePath)
{
    virtual_file *virtfile = virtual_files_head;

	while (virtfile != NULL)
	{
		if (!strcmp(filePath, virtfile->fname))
		{
            return virtfile;
		}
		virtfile = virtfile->next;
	}
	return NULL;
}

int virtual_read(virtual_file *file, char *buf, unsigned int buflen)
{
	int len;

	len = MIN(buflen, file->len - file->pos);
	memcpy(buf, file->contents + file->pos, len);

    file->pos += len;

	return len;
}

int virtual_close(virtual_file *file)
{
    file->pos = 0;
    return 0;
}
int virtual_stat(const char *filePath, struct stat *st)
{
    virtual_file *virtfile;

    virtfile = virtual_open(filePath);
    if(virtfile == NULL)
        return -1;

    st->st_size = virtfile->len;
    st->st_mtime = virtfile->mtime;

    return 0;
}
/*send file stream ,if ok return the final send size ,else
return last send error ,
maybe you want to rewrite the code use filemapping/mmap or
win32 api ,here i just call POSIX interface!
*/
int sendFileStream(const WrHttp *pHttp ,const char *filePath)
{
    char *buf;
	int rSize;
	int ret;
	/*must add O_BINARY on windows ,it cause me dubug for 2hours ,fuck!!*/
	virtual_file *fd = virtual_open(filePath);
	if(fd == NULL)
	{
	    errorLog("sendFileStream virtual_open fail\n");
	    return -1;
	}
	buf = (char *)malloc(PAGE_SIZE);
	if(buf == NULL)
	{
	    errorLog("sendFileStream malloc fail\n");
	    return -1;
	}
	while((rSize =virtual_read(fd ,buf ,PAGE_SIZE)) > 0){
		if((ret=wr_sock_nwrite(pHttp->sockhdl ,buf ,rSize)) <0)
		{
		    break;
		}

		//printf("rSize ,ret : %d, %d\n" ,rSize ,ret);
	}
	virtual_close(fd);
	free(buf);
	return ret;
}

/*read exactly n bytes ,
if ok return n
else retrun -1
here we use macro recv-timeout defined in http.h*/

int wr_sock_nread(void *sockhdl ,char *buf, size_t bufsize ,size_t n)
{
	size_t nRead = 0;
	size_t nLeft = n;
	int ret;

	while(nLeft > 0){
		ret =sock_recv(sockhdl ,buf+nRead ,MIN(nLeft,WR_TCP_QUAN),0);
		/*socket closed or error occurred*/
		if(ret <= 0)
			break;
		else{
			nRead += ret;
			nLeft -= ret;
		}
	}
	return nRead==n ? n : -1;
}

/*if ok return n ,else
return -1*/
int wr_sock_nwrite(void *sockhdl ,const char *buf ,size_t n)
{
	size_t nLeft = n;
	size_t nWrite =0;
	int ret;
	while(nLeft > 0){
		ret =sock_send(sockhdl ,buf+nWrite ,nLeft ,0);
		/*socket closed or error occurred*/
		if(ret != nLeft)
		{
		    printf("wr_sock_nwrite error occurred:%d\n", sock_get_error(sockhdl));
		    break;
		}
		else{
			nLeft -= ret;
			nWrite += ret;
		}
	}
	return nWrite==n ? n : -1;
}

/*
\r\n\r\n or \n\n ,they can be separated in any position!!
@pBuf the data buf
@cPos current-check begin pos
@len  len to be checked(from cPos)
if OK return i>=0
else return -1
*/
static int isHeadEnd(char *pBuf ,char *cPos ,int len)
{
	int i=-1;
	while(++i < len){
		if('\n' == *cPos){
			if(	(cPos-1>=pBuf && '\n'==*(cPos-1)) || \
				(cPos-2>=pBuf && '\r'==*(cPos-1) && '\n'==*(cPos-2)) ){
				return i;
			}
		}
		++cPos;
	}
	return -1;
}

/*if read head ok return nread
if some error occurred ,return -1*/
int wr_read_head(void *sockhdl ,char *buf ,size_t bufsize)
{
	int nRead = 0;
	int rsize;
	int flag = -1;
	do {
		rsize = sock_recv(sockhdl, buf+nRead ,bufsize-nRead, 0);
		if(rsize <=0)
			break;

		flag = isHeadEnd(buf ,buf+nRead ,rsize);
		nRead += rsize;
	}while(flag < 0);

	return (flag <0) ? -1 : nRead;
}

/*if error occurred ,return -1,
and send http-400 ,bad request,
*/
int wr_load_body(WrHttp *pHttp)
{
	/*!!note ,we shouldn't decode querystring or post data here
	for they may contain character like '&', '=', or something like that*/
	int ret;
	int cLen ,rLen ,sLen;
	const char *pType ,*pLen;
	do{
		/*"x-www-form-urlencoded"*/
		pType = get_head_info(pHttp ,"Content-Type");
		pLen =  get_head_info(pHttp ,"Content-Length");

		if(pType == NULL || pLen == NULL){
			ret = -1;
			break;//error
		}
		/*content len*/
		cLen = atoi(pLen);
		if(cLen<= 0 || cLen >MAX_POST_SIZE){
			ret = -2;
			break;//error
		}
		/*len we required (include head)*/
		rLen = pHttp->curPos - pHttp->method + cLen;
		/*content len we have got*/
		sLen = pHttp->recvLen - (pHttp->curPos-pHttp->method);
		WEB_DBG("cLen ,rLen ,sLen: %d %d %d\n" ,cLen ,rLen ,sLen);
		if(cLen == sLen){
			WEB_DBG("just ok..\n");
			pHttp->postData = pHttp->curPos;
			ret = 1;
		}else if(cLen >sLen){
			/*pre recv buf is too short to hold the content we require
			so we should read socket again
			!!note ,you must check pHttp->postData==pHttp->curPos otherwise
			you should free it*/
			if((pHttp->postData == NULL) || (cLen + pHttp->curPos - pHttp->method > MAX_HEADER_SIZE)){
				pHttp->postData = malloc(cLen+8);
				memmove(pHttp->postData ,pHttp->curPos ,sLen);
			}
			ret = wr_sock_nread(pHttp->sockhdl ,pHttp->postData + sLen ,cLen-sLen ,cLen-sLen);
			if(cLen-sLen == ret){
				ret = 2;
			}else
				ret = -3;
		}else{
			ret = -4;
		}
	}while(0);
	if(ret >0){
		pHttp->paramEndPos = pHttp->postData + cLen;
		*(pHttp->paramEndPos) = '\0';
	}
	WEB_DBG("contentType : %d\n" ,ret);
	return ret;
}
