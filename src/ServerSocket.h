#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H
#include "DefineConf.h"
#include "Log.h"

struct conf_s {
	void *root;
	int port;
};
typedef struct conf_s conf_t;

int open_listenfd(int port);
int set_socket_nonblock(int fd);
int read_conf(char *filename,conf_t *cf,char *buf,int len);


#endif
