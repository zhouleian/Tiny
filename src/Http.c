#include "Http.h"
#include "Log.h"
#include "Worker.h"
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "Http.h"
#include "Http_parse.h"
#include "Http_request.h"
#include "Epoll.h"
#include "Timer.h"
#include "Io_route.h"

static void get_filetype(const char* filename,char* filetype);
static int parse_uri(char* uri,int uri_length,char* filename,char* cgiargs);
static void do_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
//static void serve_dynamic(int fd, char *filename, char *cgiargs);
static void serve_static(int fd, char *filename, size_t filesize,http_out_t* out);
static void serve_dynamic(int fd, char *filename, char* cgiargs);
static char* ROOT = NULL;
//文件类型
mime_type_t tiny_mime[] = {
	{".html", "text/html"},
	{".xml", "text/xml"},
	{".xhtml", "application/xhtml+xml"},
	{".txt", "text/plain"},
	{".rtf", "application/rtf"},
	{".pdf", "application/pdf"},
	{".word", "application/msword"},
	{".png", "image/png"},
	{".gif", "image/gif"},
	{".jpg", "image/jpeg"},
	{".jpeg", "image/jpeg"},
	{".au", "audio/basic"},
	{".mpeg", "video/mpeg"},
	{".mpg", "video/mpeg"},
	{".avi", "video/x-msvideo"},
	{".gz", "application/x-gzip"},
	{".tar", "application/x-tar"},
	{".css", "text/css"},
	{NULL ,"text/plain"}
};
//connect成功后，处理用户请求
void do_request(void* ptr){
	http_request_t *r = (http_request_t *)ptr;

	int fd = r->fd;
	int rc, n;
	char filename[SHORTLINE];//shortline
	struct stat sbuf;
	ROOT = r->root;

	size_t remain_size;

	del_timer(r);
	for(;;) {
		remain_size = MAX_BUF -r->last;
	//	plast = &r->buf[r->last % MAX_BUF];
	//	remain_size = MIN(MAX_BUF - (r->last - r->pos) - 1, MAX_BUF - r->last % MAX_BUF);

		n = read(fd, r->buf, remain_size);
		check(r->last - r->pos < MAX_BUF, "request buffer overflow!");

		if (n == 0) {   
			// EOF,客户关闭fd
			log_info("read return 0, ready to close fd %d, remain_size = %zu", fd, remain_size);
			goto err;
		}

		if (n < 0) {
			if (errno != EAGAIN) {
				log_err("read err, and errno = %d", errno);
				goto err;
			}
			break;
		}

		r->last += n;
		check(r->last - r->pos < MAX_BUF, "request buffer overflow!");
		//解析请求行
		log_info("ready to parse request line"); 
		rc = http_parse_request_line(r);
		if (rc == EAGAIN) {
			continue;
		} else if (rc != OK) {
			log_err("rc != OK");
			goto err;
		}

		//获取请求体的method和uri
		log_info("method == %.*s", (int)(r->method_end - r->request_start), (char *)r->request_start);
		log_info("uri == %.*s", (int)(r->uri_end - r->uri_start), (char *)r->uri_start);

		debug("ready to parse request body");
		rc = http_parse_request_body(r);
		if (rc == EAGAIN) {
			continue;
		} else if (rc != OK) {
			log_err("rc != OK");
			goto err;
		}

		//处理http header
		http_out_t *out = (http_out_t *)malloc(sizeof(http_out_t));
		if (out == NULL) {
			log_err("no enough space for http_out_t");
			exit(1);
		}

		rc = init_out_t(out, fd);
		check(rc == OK, "init_out_t");
		char cgiargs[SHORTLINE];

		//解析uri,获取filename，uri
		int is_static = parse_uri(r->uri_start, r->uri_end - r->uri_start, filename, cgiargs);
		//找filename
		if(stat(filename, &sbuf) < 0) {
			do_error(fd, filename, "404", "Not Found", "Tiny can't find the file");
			continue;
		}

		if(is_static){//处理静态内容
			if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
				do_error(fd, filename, "403", "Forbidden","Tiny can't read the file");
				continue;
			}

			out->mtime = sbuf.st_mtime;//文件内容最后被修改的时间
			//得到header之后，处理静态内容的时候
			http_handle_header(r, out);
			check(list_empty(&(r->list)) == 1, "header list should be empty");

			if (out->status == 0) {
				out->status = HTTP_OK;//200 OK
			}

			serve_static(fd, filename, sbuf.st_size, out);

			if (!out->keep_alive) {
				log_info("no keep_alive! ready to close");
				free(out);
				goto close;
			}
			free(out);
		}
		else{//处理动态内容
			//printf("\n处理动态内容\n");
			if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { 
				do_error(fd, filename, "403", "Forbidden","Tiny couldn't run the CGI program");
				return;
			}
			//TODO
			serve_dynamic(fd, filename, cgiargs);
		}

	}

	struct epoll_event event;
	event.data.ptr = ptr;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;//防止同一fd在新数据到来时，两个线程来处理这个fd

	modify_event(r->epfd, r->fd, &event);//修改epoll
	add_timer(r, TIMEOUT_DEFAULT, http_close_conn);
	return;

err:
close:
	rc = http_close_conn(r);
	check(rc == 0, "do_request: http_close_conn");
}
//解析是静态内容还是动态内容,后来添加了对uri的长度的限制和判断,传进来的cgiargs是空的
static int parse_uri(char* uri,int uri_length,char* filename,char* cgiargs){
	check(uri !=NULL,"parse_uri:uri is NULL");
	if(uri_length > (SHORTLINE >> 1)){
		log_err("uri too long : %.*s",uri_length,uri);
		return -1;//uri过长，退出
	}
	uri[uri_length] = '\0';
	char* ptr;
	int file_length;
	if(!strstr(uri,"cgi-bin")){
		//静态内容
		file_length = uri_length;
		debug("file_length = uri_length = %d\n",file_length);
		strcpy(cgiargs,"");
		strcpy(filename,ROOT);
		strcat(filename,uri);
		if(uri[uri_length-1] == '/')
			strcat(filename,"index.html");
		log_info("static::filename=%s",filename);
		return 1;//是静态内容
	}
	else{//动态内容，提取参数
		ptr = index(uri,'?');
		if(ptr){
			strcpy(cgiargs,ptr+1);//动态内容的参数
			*ptr = '\0';
			file_length = (int)(ptr - uri);
			debug("file_length = (ptr - uri) = %d", file_length);
		}else
			strcpy(cgiargs,"");
		strcpy(filename,ROOT);
		strcat(filename,uri);
		//printf("filename=%s\n",filename);
		return 0;//动态内容
	}
}
//服务器的错误处理函数
static void do_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
	char buf[MAXLINE], body[MAXLINE];

	//http响应体
	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

	//发送响应行和响应报头给客户
	sprintf(buf, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
	sprintf(buf,"%sServer:Tiny\r\n",buf);
	sprintf(buf, "Content-type: text/html\r\n");
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));

	Worker_writen(fd, buf, strlen(buf));
	Worker_writen(fd, body, strlen(body));
	return;
}

//处理静态文件,发送http响应给客户
static void serve_static(int fd, char *filename, size_t filesize,http_out_t* out){
	int srcfd;
	char *srcp, file_type[MAXLINE], buf[SHORTLINE],header[MAXLINE]; 
	//获取文件类型
	get_filetype(filename, file_type); 
	//返回头文件信息
	sprintf(header, "HTTP/1.1 %d %s\r\n", out->status, get_shortmsg_from_status_code(out->status));
	sprintf(header, "%sServer: Tiny Web Server\r\n", header);

	if (out->keep_alive) {
		sprintf(header, "%sConnection: keep-alive\r\n", header);
		sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, TIMEOUT_DEFAULT);
	}
	struct tm tm;
	if (out->modified) {//如果文件修改了
		sprintf(header, "%sContent-type: %s\r\n", header, file_type);
		sprintf(header, "%sContent-length: %zu\r\n", header, filesize);
		localtime_r(&(out->mtime), &tm);//文件上次的修改时间
		strftime(buf, SHORTLINE,  "%a, %d %b %Y %H:%M:%S GMT", &tm);
		sprintf(header, "%sLast-Modified: %s\r\n", header, buf);
	}

	sprintf(header, "%sServer: Tiny \r\n", header);
	sprintf(header, "%s\r\n", header);

	size_t n = (size_t)worker_writen(fd, header, strlen(header)); 
	check(n==strlen(header),"worker_writen error,errno = %d",errno);
	if(n!=strlen(header)){
		log_err("n != strlen(header)");
		return;
	}
	if(!out->modified)
		return;
	//只读方式打开文件，将请求文件映射到一个虚拟存储器空间
	srcfd = Open(filename, O_RDONLY, 0);   
	srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	Close(srcfd);                           
	Worker_writen(fd, srcp, filesize);         
	Munmap(srcp, filesize);                 
	return;
}

//从文件名获取文件类型
static void get_filetype(const char* filename,char* filetype){
	for(int i = 0;tiny_mime[i].type !=NULL;++i){
		if(strstr(filename,tiny_mime[i].type))
			strcpy(filetype,tiny_mime[i].value);
	}
}

//处理动态内容
static void serve_dynamic(int fd, char *filename, char *cgiargs) {
	char header[MAXLINE], *emptylist[] = { NULL };
	char *environ[] = {NULL};

	sprintf(header, "HTTP/1.1 200 OK\r\n"); 
	sprintf(header, "%sServer: Tiny Web Server\r\n", header);
	sprintf(header, "%sServer: Tiny \r\n", header);
	sprintf(header, "%s\r\n", header);
	size_t n = (size_t)worker_writen(fd, header, strlen(header)); 
	check(n==strlen(header),"worker_writen error,errno = %d",errno);

	if (Fork() == 0) { //child 
		//FIXME
		setenv("QUERY_STRING",cgiargs,1); //初始化环境变量QUERY_STRING
		Dup2(fd, STDOUT_FILENO); //重定向标准输出到已连接文件描述符
		//解析cgiargs
		int ret = -1;
		for(int i = 0;i < strlen(cgiargs);i++)
			if(cgiargs[i] == '&')
				ret = i;
		check(ret!=0,"cgiargs error");
		char p1[ret+1];
		for(int j = 0;j <ret;j++)
			p1[j] = cgiargs[j];
		p1[ret] = '\0';
		char *p2 = strchr(cgiargs,'&') + 1;
		emptylist[0] = p1;
		emptylist[1] = p2;
		Execve(filename, emptylist, environ); //执行cgi项目
	}
	Wait(NULL);//父进程等待子进程结束
}




