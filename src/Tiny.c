

#include <stdint.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "DefineConf.h"
#include "Log.h"
#include "Timer.h"
#include "Http.h"
#include "Epoll.h"
#include "ServerSocket.h"

extern struct epoll_event *events;

int main(int argc, char* argv[]) {
    int rc;
    int opt = 0;
    int options_index = 0;
	char *conf_file = "tiny.conf";

    
    //读取配置文件
    char conf_buf[BUFLEN];
    conf_t cf;
    rc = read_conf(conf_file, &cf, conf_buf, BUFLEN);
    check(rc == CONF_OK, "read conf err");

	//如果客户端关闭，断电的情况，为了防止SIGPIPE信号默认的终止程序，设置忽略这个信号
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;//信号处置设置为忽略
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL)) {
        log_err("install sigal handler for SIGPIPE failed");
        return 0;
    }
    
    //初始化监听套接字，设置非阻塞
    int listenfd;
    struct sockaddr_in clientaddr;
    socklen_t inlen = 1;
    memset(&clientaddr, 0, sizeof(struct sockaddr_in));  
    
    listenfd = open_listenfd(cf.port);
    rc = set_socket_nonblock(listenfd);
    check(rc == 0, "make_socket_non_blocking");

    int epfd = create_epoll(0);
    struct epoll_event event;
    //初始化请求结构request
    http_request_t *request = (http_request_t *)malloc(sizeof(http_request_t));
    init_request_t(request, listenfd, epfd, &cf);
	//就将lsitenfd加入事件
    event.data.ptr = (void *)request;
    event.events = EPOLLIN | EPOLLET;
    add_event(epfd, listenfd, &event);

    timer_init();

    log_info("tiny started.");
    int n;
    int i, fd;
    int time;

    while (1) {
        time = find_timer();
        debug("wait time = %d", time);
        n = wait_epoll(epfd, events, MAXEVENTS, time);
        handle_expire_timers();
        
        for (i = 0; i < n; i++) {
            http_request_t *r = (http_request_t *)events[i].data.ptr;
            fd = r->fd;
            
            if (listenfd == fd) {//请求结构中的fd是监听fd，说明有连接请求，接受连接

                int infd;
                while(1) {
                    infd = accept(listenfd, (struct sockaddr *)&clientaddr, &inlen);
                    if (infd < 0) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            /*处理完所有连接 */
                            break;
                        } else {
                            log_err("accept");
                            break;
                        }
                    }
					//已连接设置为非阻塞
                    rc = set_socket_nonblock(infd);
                    check(rc == 0, "make_socket_non_blocking");
                    log_info("new connection fd %d", infd);
                    
                    http_request_t *request = (http_request_t *)malloc(sizeof(http_request_t));
                    if (request == NULL) {
                        log_err("malloc(sizeof(http_request_t))");
                        break;
                    }

                    init_request_t(request, infd, epfd, &cf);
                    event.data.ptr = (void *)request;
                    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

                    add_event(epfd, infd, &event);
                    add_timer(request, TIMEOUT_DEFAULT, http_close_conn);
                }

            } else {
                if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN))) {
                    log_err("epoll error fd: %d", r->fd);
                    close(fd);
                    continue;
                }

                log_info("new data from fd %d", fd);

                do_request(events[i].data.ptr);
            }
        }   //end of for
    }   // end of while(1)
    


    return 0;
}
