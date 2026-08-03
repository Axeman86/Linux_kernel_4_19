#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <memory.h>

#define FIFO_CLEAR 0x01
#define BUFFER_LEN 20

int main(int argc, char* argv[])
{
    int fd = open("/dev/globalfifo", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open failed");
        return EXIT_FAILURE;
    }

    if (ioctl(fd, FIFO_CLEAR, 0) < 0) {
        perror("ioctl FIFO_CLEAR failed");
        goto err;
    }

    int epfd = epoll_create(1);
    if (epfd < 0) {
        perror("epoll_create failed");
        goto err;
    }

    struct epoll_event ev_globalfifo;
    bzero(&ev_globalfifo, sizeof(ev_globalfifo));
    ev_globalfifo.events = EPOLLIN | EPOLLPRI;

    int err = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev_globalfifo);
    if (err < 0) {
        perror("epoll_ctl failed");
        goto err;
    }

    err = epoll_wait(epfd, &ev_globalfifo, 1, 15000);
    if (err < 0) {
        perror("epoll_wait failed");
        goto err;
    } else if (err == 0) {
        printf("epoll_wait timeout, no data input\n");
    } else {
        printf("epoll_wait success, events: %d\n", ev_globalfifo.events);
    }

    err = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev_globalfifo);
    if (err < 0) {
        perror("epoll_ctl DEL failed");
        goto err;
    }

    close(epfd);
    close(fd);
    return EXIT_SUCCESS;

err:
    if (epfd >= 0)
        close(epfd);
    if (fd >= 0)
        close(fd);
    return EXIT_FAILURE;
}
