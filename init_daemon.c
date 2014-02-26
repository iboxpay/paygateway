/*
 * Copyright (C) 2014, Lytsing Huang <hlqing@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

void init_daemon(void)
{
    int pid;
    int i;
    if ((pid = fork()) > 0) {
        exit(0);
    } else if ( pid < 0) {
        exit(1);
    }

    setsid();

    if ((pid = fork()) > 0) {
        exit(0);
    } else if (pid < 0) {
        exit(1);
    }

    for (i=0; i< NOFILE; ++i) {
        close(i);
    }

    chdir("/tmp");
    umask(0);
    return;
}

