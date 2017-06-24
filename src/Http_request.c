#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <math.h>
#include <time.h>
#include <unistd.h>
#include "Http.h"
#include "Http_request.h"
#include "DefineConf.h"
#include "Log.h"

static int http_process_ignore(http_request_t *r, http_out_t *out, char *data, int len);
static int http_process_connection(http_request_t *r, http_out_t *out, char *data, int len);
static int http_process_if_modified_since(http_request_t *r, http_out_t *out, char *data, int len);

http_header_handle_t http_headers_in[] = {
    {"Host", http_process_ignore},
    {"Connection", http_process_connection},
    {"If-Modified-Since", http_process_if_modified_since},
    {"", http_process_ignore}
};

int init_request_t(http_request_t *r, int fd, int epfd, conf_t *cf) {
    r->fd = fd;
    r->epfd = epfd;
    r->pos = r->last = 0;
    r->state = 0;
    r->root = cf->root;
    Init_list_head(&(r->list));

    return OK;
}

int free_request_t(http_request_t *r) {
    (void) r;
    return OK;
}

int init_out_t(http_out_t *o, int fd) {
    o->fd = fd;
    o->keep_alive = 0;
    o->modified = 1;
    o->status = 0;

    return OK;
}

int free_out_t(http_out_t *o) {
    (void) o;
    return OK;
}

void http_handle_header(http_request_t *r, http_out_t *o) {
    list_head *pos;
    http_header_t *hd;
    http_header_handle_t *header_in;
    int len;

    list_for_each(pos, &(r->list)) {
        hd = list_entry(pos, http_header_t, list);

        for (header_in = http_headers_in; 
            strlen(header_in->name) > 0;
            header_in++) {
            if (strncmp(hd->key_start, header_in->name, hd->key_end - hd->key_start) == 0) {
                len = hd->value_end - hd->value_start;
                (*(header_in->handler))(r, o, hd->value_start, len);
                break;
            }    
        }

        list_del(pos);
        free(hd);
    }
}

int http_close_conn(http_request_t *r) {
    close(r->fd);
    free(r);

    return OK;
}

static int http_process_ignore(http_request_t *r, http_out_t *out, char *data, int len) {
    (void) r;
    (void) out;
    (void) data;
    (void) len;
    
    return OK;
}

static int http_process_connection(http_request_t *r, http_out_t *out, char *data, int len) {
    (void) r;
    if (strncasecmp("keep-alive", data, len) == 0) {
        out->keep_alive = 1;
    }

    return OK;
}

static int http_process_if_modified_since(http_request_t *r, http_out_t *out, char *data, int len) {
    (void) r;
    (void) len;

    struct tm tm;
    if (strptime(data, "%a, %d %b %Y %H:%M:%S GMT", &tm) == (char *)NULL) {
        return OK;
    }
    time_t client_time = mktime(&tm);

    double time_diff = difftime(out->mtime, client_time);
    if (fabs(time_diff) < 1e-6) {
        out->modified = 0;
        out->status = HTTP_NOT_MODIFIED;
    }
    
    return OK;
}

const char *get_shortmsg_from_status_code(int status_code) {
    if (status_code == HTTP_OK) {
        return "OK";
    }
    if (status_code == HTTP_NOT_MODIFIED) {
        return "Not Modified";
    }
    if (status_code == HTTP_NOT_FOUND) {
        return "Not Found";
    }
    return "Unknown";
}

