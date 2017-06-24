#ifndef WORKER_H
#define WORKER_H

#include <sys/types.h>
#include "Log.h"
#define BUFSIZE 8192
typedef struct{
	int worker_fd;
	ssize_t worker_cnt;//buf中未读的数据
	char* worker_bufptr;
	char worker_buf[BUFSIZE];
}worker_buf;



ssize_t worker_readn(int fd,void* usrbuf,size_t n);
ssize_t worker_writen(int fd, void *usrbuf, size_t n);
void worker_readinitb(worker_buf *wbuf, int fd); 
ssize_t	worker_readnb(worker_buf *wbuf, void *usrbuf, size_t n);
ssize_t	worker_readlineb(worker_buf *wbuf, void *usrbuf, size_t maxlen);

ssize_t Worker_readn(int fd,void* usrbuf,size_t n);
void Worker_writen(int fd, void *usrbuf, size_t n);
void Worker_readinitb(worker_buf *wbuf, int fd); 
ssize_t	Worker_readnb(worker_buf *wbuf, void *usrbuf, size_t n);
ssize_t	Worker_readlineb(worker_buf *wbuf, void *usrbuf, size_t maxlen);


#endif
