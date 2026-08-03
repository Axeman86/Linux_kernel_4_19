/*
 *     Copyright (c) 2026 Dreame SmartHome LLC.
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
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

static void signalio_handler(int signum)
{
    printf("receive a signal from globalfifo,signalnum:%d\n", signum);
}

int main(int argc, char* argv[])
{

    int fd = open("/dev/globalfifo", O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("device open failure:");
        return EXIT_FAILURE;
    }

    signal(SIGIO, signalio_handler);
    fcntl(fd, F_SETOWN, getpid());
    int oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oflags | FASYNC);

    while (1) {
        sleep(100);
    }

    return EXIT_SUCCESS;
}
