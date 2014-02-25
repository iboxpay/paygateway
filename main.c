/*
 * Copyright (C) 2014, Lytsing Huang <hlqing@gmail.com>.
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

#include <unistd.h>
#include <signal.h>
#include <evhtp.h>
#include <event2/event.h>

#include "httpd.h"
#include "socket.h"

int num_threads = 4;
uint64_t max_keepalives = 60;
int backlog = 1024;
int port = 8081;

/* Terminate gracefully on SIGTERM */
void sigterm_cb(int fd, short event, void * arg) {
    evbase_t     * evbase = (evbase_t *)arg;
    struct timeval tv     = { .tv_usec = 100000, .tv_sec = 0 }; /* 100 ms */

    event_base_loopexit(evbase, &tv);
}

void init_thread_cb(evhtp_t * htp, evthr_t * thr, void * arg) {
    static int aux = 0;

    printf("Spinning up a thread: %d\n", ++aux);
    evthr_set_aux(thr, &aux);
}

int main(int argc, const char* argv[]) {
    struct event* ev_sigterm;

    evbase_t* evbase = event_base_new();

    evhtp_t* htp = evhtp_new(evbase, NULL);

    evhtp_set_cb(htp, "/paygateway/api/Forward", frontend_request_cb, NULL);
    evhtp_set_cb(htp, "/paygateway/api/ApiRequest", router_request_cb, NULL);
    evhtp_use_threads(htp, init_thread_cb, num_threads, NULL);

    ev_sigterm = evsignal_new(evbase, SIGTERM, sigterm_cb, evbase);
    evsignal_add(ev_sigterm, NULL);

    evhtp_set_max_keepalive_requests(htp, max_keepalives);
    evhtp_bind_socket(htp, "0.0.0.0", port, backlog);
    event_base_loop(evbase, 0);

    event_free(ev_sigterm);
    evhtp_unbind_socket(htp);

    evhtp_free(htp);
    event_base_free(evbase);

    fprintf(stdout, "\nClean exit!\n");
    return 0;
}

