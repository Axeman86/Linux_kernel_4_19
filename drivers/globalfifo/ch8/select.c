#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define FIFO_CLEAR 0x01
#define BUFFER_SIZE 20

int main()
{
    int fd = open("/dev/globalfifo", O_RDONLY | O_NONBLOCK);
    if (fd != -1) {
        // FIFO_CLEAR
        if (ioctl(fd, FIFO_CLEAR) < 0) {
            perror("ioctl FIFO_CLEAR failed");
            close(fd);
            return -1;
        }

        fd_set rfds, wfds;
        while (1) {
            FD_ZERO(&rfds);
            FD_ZERO(&wfds);
            FD_SET(fd, &rfds);
            FD_SET(fd, &wfds);

            int ret = select(fd + 1, &rfds, &wfds, NULL, NULL);
            if (ret < 0) {
                perror("select failed");
                close(fd);
                return -1;
            }

            if (FD_ISSET(fd, &rfds)) {
                printf("Poll monitor:can be read\n");
            }

            if (FD_ISSET(fd, &wfds)) {
                printf("Poll monitor:can be written\n");
            }
        }

        close(fd);
    } else {
        perror("open failed");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
