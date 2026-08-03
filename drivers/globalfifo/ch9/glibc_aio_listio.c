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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <aio.h>
#include <fcntl.h>
#include <errno.h>

int main(void)
{
    char stdin_buf[100] = { 0 };
    char* file_buf      = NULL;
    int fd              = -1;

    struct aiocb cb1, cb2;
    memset(&cb1, 0, sizeof(cb1));
    memset(&cb2, 0, sizeof(cb2));

    fd = open("file1.txt", O_RDONLY);
    if (fd < 0) {
        perror("open file1.txt");
        return 1;
    }

    file_buf = malloc(100);
    if (!file_buf) {
        perror("malloc");
        close(fd);
        return 1;
    }
    memset(file_buf, 0, 100);

    cb1.aio_fildes     = STDIN_FILENO;
    cb1.aio_buf        = stdin_buf;
    cb1.aio_nbytes     = sizeof(stdin_buf) - 1;
    cb1.aio_offset     = 0;
    cb1.aio_lio_opcode = LIO_READ;

    cb2.aio_fildes     = fd;
    cb2.aio_buf        = file_buf;
    cb2.aio_nbytes     = 99;
    cb2.aio_offset     = 0;
    cb2.aio_lio_opcode = LIO_READ;

    struct aiocb* list[2] = { &cb1, &cb2 };

    printf("请输入内容并回车（同时读取 file1.txt）:\n");

    /* LIO_WAIT: 提交后阻塞，直到 list 中请求完成 */
    if (lio_listio(LIO_WAIT, list, 2, NULL) < 0) {
        perror("lio_listio");
        free(file_buf);
        close(fd);
        return 1;
    }

    int e1 = aio_error(&cb1);
    int e2 = aio_error(&cb2);

    ssize_t n1 = -1, n2 = -1;
    if (e1 == 0)
        n1 = aio_return(&cb1);
    if (e2 == 0)
        n2 = aio_return(&cb2);

    if (n1 >= 0) {
        if (n1 < (ssize_t) sizeof(stdin_buf))
            stdin_buf[n1] = '\0';
        else
            stdin_buf[sizeof(stdin_buf) - 1] = '\0';
        printf("[STDIN] %zd bytes: \"%s\"\n", n1, stdin_buf);
    } else {
        printf("[STDIN] fail, aio_error=%d (%s)\n", e1, strerror(e1));
    }

    if (n2 >= 0) {
        if (n2 < 100)
            file_buf[n2] = '\0';
        else
            file_buf[99] = '\0';
        printf("[file1.txt] %zd bytes: \"%s\"\n", n2, file_buf);
    } else {
        printf("[file1.txt] fail, aio_error=%d (%s)\n", e2, strerror(e2));
    }

    free(file_buf);
    close(fd);
    return 0;
}
