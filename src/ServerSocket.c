#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include "ServerSocket.h"


int open_listenfd(int port){
	int listenfd,optval;
	if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		return -1;
	//撤销"address already in use"的错误bind设置
	if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(optval)) < 0)
		return -1;
	//bind,listen
	struct sockaddr_in servaddr;
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons((unsigned int)port);
	if(bind(listenfd,(SA*)&servaddr,sizeof(servaddr)) < 0){
		log_err("server_socket::open_listenfd bind failed");
		return -1;
	}
	if(listen(listenfd,LISTENQ) < 0){
		log_err("listen failed");
		return -1;
	}
	return listenfd;
}
//设置非阻塞fd
int set_socket_nonblock(int fd){
	int flags;
	if((flags = fcntl(fd,F_GETFL,0)) < 0){
		log_err("fcntl failed");
		return -1;
	}
	if((fcntl(fd,F_SETFL,flags | O_NONBLOCK)) < 0){
		log_err("fcntl failed");
		return -1;
	}
	return 0;
}


//读取配置文件到buf
int read_conf(char *filename,conf_t *cf,char *buf,int len){
	FILE *fp = fopen(filename, "r");
    if (!fp) {
        log_err("cannot open config file: %s", filename);
        return CONF_ERROR;
    }

    int pos = 0;
    char *delim_pos;
    int line_len;
    char *cur_pos = buf+pos;

    while (fgets(cur_pos, len-pos, fp)) {
        delim_pos = strstr(cur_pos, "=");
        line_len = strlen(cur_pos);
        
        if (!delim_pos)
            return CONF_ERROR;
        
        if (cur_pos[strlen(cur_pos) - 1] == '\n') {
            cur_pos[strlen(cur_pos) - 1] = '\0';
        }

        if (strncmp("root", cur_pos, 4) == 0) {
            cf->root = delim_pos + 1;
        }

        if (strncmp("port", cur_pos, 4) == 0) {
            cf->port = atoi(delim_pos + 1);     
        }
        cur_pos += line_len;
    }

    fclose(fp);
    return CONF_OK;
}



