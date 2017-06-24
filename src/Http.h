#ifndef HTTP_H
#define HTTP_H

#include <strings.h>
#include <stdint.h>
#include "Worker.h"
#include "List.h"
#include "Log.h"
#include "Http_request.h"
   
#define MAXLINE     8192
#define SHORTLINE   512

//在.c文件中使用strcasecmp实现了，但是后面有多个循环，又重新用的switch
#define str3_cmp(m, c0, c1, c2, c3)                                       \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)
#define str3Ocmp(m, c0, c1, c2, c3)                                       \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

#define str4cmp(m, c0, c1, c2, c3)                                        \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

#define MIN(a,b) ((a) < (b) ? (a):(b))
typedef struct mime_type_s {
	const char *type;
	const char *value;
} mime_type_t;

void do_request(void *infd);

#endif
