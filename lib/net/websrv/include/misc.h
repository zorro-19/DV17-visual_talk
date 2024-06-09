#ifndef  _MISC_H__
#define  _MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http.h"
#include "wrstring.h"

void DBG(const char *fmt, ...);
void WEB_DBG(const char *fmt, ...);
void errorLog(const char *mess);
int url_decode(const char *src ,char *dst ,int is_form_url_encoded);
int httpd_url_decode(const char *src, char *dst, int is_form_url_encoded);
int UTF8toANSI(char *src);


static const char *methods[3] = {"HEAD", "GET", "POST"};



#ifdef __cplusplus
}
#endif
#endif
