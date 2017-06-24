#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>

#define MAXEVENTS 1024

int create_epoll(int flags);
void add_event(int epollfd,int fd,struct epoll_event *event);
void modify_event(int epollfd,int fd,struct epoll_event *event);
void delete_event(int epollfd,int fd,struct epoll_event *event);
int wait_epoll(int epollfd,struct epoll_event *events,int maxevents,int timeout);


#endif
