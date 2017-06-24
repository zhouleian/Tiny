#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "Worker.h"

//初始化worker_buf结构
void worker_readinitb(worker_buf *wbuf, int fd){
	wbuf->worker_fd = fd;
	wbuf->worker_cnt = 0;
	wbuf->worker_bufptr = wbuf->worker_buf;
}

//从worker_buf里读取n大小数据到usrbuf,返回实际读取数据大小
static ssize_t worker_read(worker_buf* wbuf,char* usrbuf,size_t n){
	int cnt;
	while(wbuf->worker_cnt <= 0){//worker_buf里没有数据,从worker_fd里获取
		wbuf->worker_cnt = read(wbuf->worker_fd,wbuf->worker_buf,sizeof(wbuf->worker_buf));
		if(wbuf->worker_cnt < 0){
			if(errno == EAGAIN){//没有数据了，已经读完了所有数据
				return -EAGAIN;
			}else if(errno != EINTR)//信号
				return -1;
		}else if(wbuf->worker_cnt == 0)
			return 0;
		else
			wbuf->worker_bufptr = wbuf->worker_buf;
	}
	//最终读取数据大小是，n和worker_cnt的较小值
	if(n < wbuf->worker_cnt)
		cnt = n;
	else
		cnt = wbuf->worker_cnt;
	memcpy(usrbuf,wbuf->worker_buf,cnt);
	wbuf->worker_bufptr += cnt;
	wbuf->worker_cnt -= cnt;
	return cnt;
}


//从worker_buf里读取n大小数据到usrbuf,可能要读取多次,返回已经读取的数据
ssize_t	worker_readnb(worker_buf *wbuf, void *usrbuf, size_t n){
	ssize_t nleft = n;
	ssize_t nread;
	char *buf = wbuf->worker_buf;
	while(nleft < 0){
		if((nread = worker_read(wbuf,usrbuf,nleft)) < 0)
			return -1;
		else if(nread == 0)
			return 0;
		nleft -= nread;
		buf += nread;
	}
	return (n-nleft);
}

//读取一行
ssize_t	worker_readlineb(worker_buf *wbuf, void *usrbuf, size_t maxlen){
	int n,wc;
	char c;
	char * buf = usrbuf;
	for(n = 1;n < maxlen;n++){
		if((wc = worker_read(wbuf,&c,1))== -EAGAIN)//现在还没有数据可读，等待下一次可读时候再读
			return wc;
		else if(wc == 0){
			if(n ==1)
				return 0;//没有数据可读,返回
			else
				break;//读到了空，但是已经读取了一部分数据
		}else if(wc ==1){//读到一个字节数据
			*buf++ = c;
			if(c == '\n'){
				n++;
				break;
			}
		}else
			return -1;
	}
	*buf = 0;
	return n-1;
}


//读取到客户fd的buf中
ssize_t worker_readn(int fd,void* usrbuf,size_t n){
	char *buf = (char*)usrbuf;
	ssize_t nleft = n;
	ssize_t nread;
	while(nleft > 0){
		if((nread = read(fd,buf,nleft)) < 0)
			if(errno == EINTR)//信号捕获，while调用read
				nread = 0;
			else{
				log_err("read errno == %d\n",errno);
				return -1;
			}
		if(nread == 0)
			break;
		nleft = nleft-nread;
		buf += nread;
	}
	return n-nleft;
}


//将客户buf中数据读取到服务器buf中
ssize_t worker_writen(int fd, void *usrbuf, size_t n){
	size_t nleft = n;
    ssize_t nwritten;
    char *bufp = (char *)usrbuf;
        
    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR)//信号捕获，while调用write
                nwritten = 0;
            else {
                log_err("errno == %d\n", errno);
                return -1;
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    
    return n;
}

ssize_t Worker_readn(int fd, void *usrbuf, size_t n) {
    ssize_t nr;
  
    if ((nr = Worker_readn(fd, usrbuf, n)) < 0)
		log_err("Worker_readn error");
    return nr;
}
void Worker_writen(int fd, void *usrbuf, size_t n) {
    if (worker_writen(fd, usrbuf, n) != n)
		log_err("Worker_writen error");
}
void Worker_readinitb(worker_buf* wbuf, int fd){
    worker_readinitb(wbuf, fd);
} 

ssize_t Worker_readnb(worker_buf* wbuf, void *usrbuf, size_t n) {
    ssize_t rc;

    if ((rc = worker_readnb(wbuf, usrbuf, n)) < 0)
		log_err("Worker_readnb error");
    return rc;
}

ssize_t Worker_readlineb(worker_buf *wbuf, void *usrbuf, size_t maxlen) {
    ssize_t rc;

    if ((rc = worker_readlineb(wbuf, usrbuf, maxlen)) < 0)
		log_err("Worker_readlineb error");
    return rc;
}
 
