
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <time.h>
#include "Http.h"
#include "ServerSocket.h"
#include "Log.h"
#include "DefineConf.h"

#define HTTP_PARSE_INVALID_METHOD        10
#define HTTP_PARSE_INVALID_REQUEST       11
#define HTTP_PARSE_INVALID_HEADER        12

#define HTTP_UNKNOWN                     0x0001
#define HTTP_GET                         0x0002
#define HTTP_HEAD                        0x0004
#define HTTP_POST                        0x0008

#define HTTP_OK                          200

#define HTTP_NOT_MODIFIED                304

#define HTTP_NOT_FOUND                   404

#define MAX_BUF 8192

typedef struct http_request_s {
    void *root;
    int fd;
    int epfd;
    char buf[MAX_BUF];  
    size_t pos, last;
    int state;
    void *request_start;
    void *method_end;
    int method;//tiny支持的方法
    void *uri_start;//uri开始指针
    void *uri_end;      //uri结束指针，指向空，不指向uri内容
    int http_major;//HTTP版本的1.0的1
    int http_minor;//HTTP版本1.0的0
    void *request_end;

    struct list_head list;//存储请求头
    void *cur_header_key_start;
    void *cur_header_key_end;
    void *cur_header_value_start;
    void *cur_header_value_end;

    void *timer;
}http_request_t;

typedef struct {
    int fd;
    int keep_alive;
    time_t mtime;   //修改文件的时间
    int modified;     //是否修改  
    int status;
} http_out_t;

typedef struct http_header_s {
    void *key_start, *key_end;
    void *value_start, *value_end;
    list_head list;
}http_header_t;

typedef int (*http_header_handler_pt)(http_request_t *r, http_out_t *o, char *data, int len);

typedef struct {
    char *name;
    http_header_handler_pt handler;
} http_header_handle_t;

int init_request_t(http_request_t *r, int fd, int epfd,conf_t* cf);
int http_close_conn(http_request_t *r);//http.c

void http_handle_header(http_request_t *r, http_out_t *o);//http.c
int free_request_t(http_request_t *r);

int free_out_t(http_out_t *o);

const char *get_shortmsg_from_status_code(int status_code);

int init_out_t(http_out_t *o, int fd);//http.c
extern http_header_handle_t     http_headers_in[];

#endif


