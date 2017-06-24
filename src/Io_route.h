#ifndef IO_ROUTE_H
#define IO_ROUTE_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
void unix_error(char *msg) /* Unix-style error */{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

int Open(const char *pathname, int flags, mode_t mode) {
    int rc;

    if ((rc = open(pathname, flags, mode))  < 0)
		unix_error("Open error");
    return rc;
}

void Close(int fd) {
    int rc;

    if ((rc = close(fd)) < 0)
		unix_error("Close error");
}
void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset) {
    void *ptr;

    if ((ptr = mmap(addr, len, prot, flags, fd, offset)) == ((void *) -1))
		unix_error("mmap error");
    return(ptr);
}

void Munmap(void *start, size_t length) {
    if (munmap(start, length) < 0)
		unix_error("munmap error");
}
pid_t Fork(void) {
    pid_t pid;

    if ((pid = fork()) < 0)
		unix_error("Fork error");
    return pid;
}

int Dup2(int fd1, int fd2) {
    int rc;

    if ((rc = dup2(fd1, fd2)) < 0)
	unix_error("Dup2 error");
    return rc;
}

void Execve(const char *filename, char *const argv[], char *const envp[]) {
    if (execve(filename, argv, envp) < 0)
		unix_error("Execve error");
}
pid_t Wait(int *status) {
    pid_t pid;

    if ((pid  = wait(status)) < 0)
	unix_error("Wait error");
    return pid;
}

#endif
