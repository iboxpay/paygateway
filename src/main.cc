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

extern void init_daemon(void);

int num_threads = 4;
uint64_t max_keepalives = 60;
int backlog = 1024;

char httpd_option_listen[32] = "0.0.0.0";
int httpd_option_port = 8080;
int httpd_option_daemon = 0;
int httpd_option_timeout = 120;  // in seconds

/* Terminate gracefully on SIGTERM */
void sigterm_cb(int fd, short event, void * arg) {
    evbase_t     * evbase = (evbase_t *)arg;
    struct timeval tv     = { 100000, 0 }; /* 100 ms */

    event_base_loopexit(evbase, &tv);
}

void init_thread_cb(evhtp_t * htp, evthr_t * thr, void * arg) {
    static int aux = 0;

    printf("Spinning up a thread: %d\n", ++aux);
    evthr_set_aux(thr, &aux);
}

int parse_args(int argc, char** argv);

int main(int argc, char** argv) {
    struct event* ev_sigterm;

    parse_args(argc, argv);

    if (httpd_option_daemon) {
        init_daemon();
    }

    evbase_t* evbase = event_base_new();

    evhtp_t* htp = evhtp_new(evbase, NULL);

    evhtp_set_cb(htp, "/", frontend_request_cb, NULL);
    evhtp_set_cb(htp, "/paygateway/api/ApiRequest", router_request_cb, NULL);
    evhtp_use_threads(htp, init_thread_cb, num_threads, NULL);

    ev_sigterm = evsignal_new(evbase, SIGTERM, sigterm_cb, evbase);
    evsignal_add(ev_sigterm, NULL);

    evhtp_set_max_keepalive_requests(htp, max_keepalives);
    evhtp_bind_socket(htp, httpd_option_listen, httpd_option_port, backlog);
    event_base_loop(evbase, 0);

    event_free(ev_sigterm);
    evhtp_unbind_socket(htp);

    evhtp_free(htp);
    event_base_free(evbase);

    fprintf(stdout, "\nClean exit!\n");
    return 0;
}

void show_help() {
    const char* help = "PayGateway server.\n\n"
        "-l <ip_addr> interface to listen on, default is 0.0.0.0\n"
        "-p <num>     port number to listen on, default is 8080\n"
        "-d           run as a deamon\n"
        "-t <second>  timeout for a http request, default is 120 seconds\n"
        "-h           print this help and exit\n"
        "\n";
    fprintf(stderr, "%s", help);
}

int parse_args(int argc, char** argv) {
    int c;
    while ((c = getopt(argc, argv, "l:p:dt:h")) != -1) {
        switch (c) {
            case 'l' :
                strncpy(httpd_option_listen, optarg, sizeof(httpd_option_listen));
                break;
            case 'p' :
                httpd_option_port = atoi(optarg);
                break;
            case 'd' :
                httpd_option_daemon = 1;
                break;
            case 't' :
                httpd_option_timeout = atoi(optarg);
                break;
            case 'h' :
            default :
                show_help();
                exit(EXIT_SUCCESS);
        }
    }

    return 0;
}

