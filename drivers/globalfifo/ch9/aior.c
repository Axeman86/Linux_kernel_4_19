/*
 *     Copyright (c) 2026 SmartHome LLC.
 *     All rights reserved.
 *
 *     Licensed under the Apache License, Version 2.0 (the "License");
 *     you may not use this file except in compliance with the License.
 *     You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <inttypes.h>

#include <libaio.h>

#define BUF_SEZE 4096

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDWR | O_DIRECT);
    if (fd < 0) {
        perror("open error");
        goto err;
    }

    // Allocate aligned buffer
    unsigned char* buf = NULL;
    int ret            = posix_memalign((void**) &buf, 512, BUF_SEZE + 1);
    if (ret != 0) {
        fprintf(stderr, "posix_memalign error: %s\n", strerror(ret));
        goto err1;
    }
    memset(buf, 0, BUF_SEZE + 1);

    io_context_t ctx = 0;
    ret              = io_setup(128, &ctx);
    if (ret < 0) {
        printf("io_setup error:%s", strerror(-ret));
        goto err2;
    }

    // setup I/O control block
    struct iocb cb = { 0 };
    io_prep_pread(&cb, fd, buf, BUF_SEZE, 0);

    struct iocb* cbs[1] = { &cb };
    ret                 = io_submit(ctx, 1, cbs);
    if (ret != 1) {
        if (ret < 0) {
            printf("io_submit error:%s\n", strerror(-ret));
        } else {
            fprintf(stderr, "could not submit IOs");
        }
        goto err3;
    }

    // get the reply
    struct io_event events[1];
    ret = io_getevents(ctx, 1, 1, events, NULL);
    if (ret != 1) {
        if (ret < 0) {
            printf("io_getevents error:%s\n", strerror(-ret));
        } else {
            fprintf(stderr, "could not get Events");
        }
        goto err3;
    }

    if (events[0].res2 == 0) {
        printf("%s\n", buf);
    } else {
        printf("AIO error:%s", strerror(-events[0].res));
        goto err3;
    }

    if ((ret = io_destroy(ctx)) < 0) {
        printf("io_destroy error:%s", strerror(-ret));
        goto err2;
    }

    free(buf);
    close(fd);

    return EXIT_SUCCESS;

err3:
    if ((ret = io_destroy(ctx)) < 0) {
        printf("io_destroy error:%s", strerror(-ret));
    }
err2:
    if (buf) {
        free(buf);
    }
err1:
    if (fd >= 0) {
        close(fd);
    }
err:
    return EXIT_FAILURE;
}
