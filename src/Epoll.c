#include "Epoll.h"
#include "Log.h"
struct epoll_event *events;

int create_epoll(int flags){
	int fd = epoll_create1(flags);//没有使用epoll_create
	check(fd>0,"create_epoll:epoll_create1");
	events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*MAXEVENTS);
	check(events!=NULL,"create_epoll: malloc events");
	return fd;
}

void add_event(int epollfd,int fd,struct epoll_event *ev){
	int res = epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,ev);

	check(res==0,"add_event:epoll_ctl");
	return;
}

void modify_event(int epollfd,int fd,struct epoll_event *ev){
	int res = epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,ev);

	check(res==0,"modify_event:epoll_ctl");
	return;
}

void delete_event(int epollfd,int fd,struct epoll_event *ev){
	int res = epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,ev);

	check(res==0,"add_event:epoll_ctl");
	return;
}
int wait_epoll(int epollfd,struct epoll_event *events,int maxevents,int timeout){
	int ret = epoll_wait(epollfd,events,maxevents,timeout);
	check(ret>=0,"wait_epoll: epoll_wait");
	return ret;
}
