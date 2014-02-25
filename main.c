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

int num_threads = 4;
uint64_t max_keepalives = 60;
int backlog = 1024;
int port = 4567;

static void sigint_cb(int sig, short why, void* data);

int main(int argc, const char* argv[]) {
    struct event* ev_sigint;

    evbase_t* evbase = event_base_new();

    evhtp_t* htp = evhtp_new(evbase, NULL);

    evhtp_set_cb(htp, "/paygateway/api/ApiRequest", router_request_cb, NULL);
#ifndef EVHTP_DISABLE_EVTHR
    evhtp_use_threads(htp, NULL, num_threads, NULL);
#endif
#ifndef WIN32
    ev_sigint = evsignal_new(evbase, SIGINT, sigint_cb, evbase);
    evsignal_add(ev_sigint, NULL);
#endif
    evhtp_set_max_keepalive_requests(htp, max_keepalives);
    evhtp_bind_socket(htp, "0.0.0.0", port, backlog);
    event_base_loop(evbase, 0);

    event_free(ev_sigint);
    evhtp_unbind_socket(htp);

    evhtp_free(htp);
    event_base_free(evbase);

    fprintf(stdout, "\nByebye!\n");
    return 0;
}

static void sigint_cb(int sig, short why, void* data) {
    event_base_loopexit(data, NULL);
}

