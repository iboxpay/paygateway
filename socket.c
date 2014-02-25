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

#include <event2/dns.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "socket.h"

#define PAYMENT_PORT 8000
#define PAYMENT_SERVER "0.0.0.0"

struct payment_context {
    const char* payment_contents;
    int payment_contents_len;
    int recved;
};

static void backend_cb(struct bufferevent *bev, void *ptr) {
    //evhtp_request_t * frontend_req = (evhtp_request_t *)ptr;

    struct evbuffer *input = bufferevent_get_input(bev);
    //evbuffer_prepend_buffer(frontend_req->buffer_out, input);

    printf("backend_cb");
    //evhtp_send_reply(frontend_req, EVHTP_RES_OK);
    //evhtp_request_resume(frontend_req);
}

static void eventcb(struct bufferevent *bev, short events, void *ptr) {
    if (events & BEV_EVENT_CONNECTED) {
        printf("Connect okay.\n");
    } else if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
        printf("Closing\n");
        bufferevent_free(bev);
    }
}

/**
 * @brief router_request_cb The callback of a router request.
 *
 * @param req The request with router protobuf data.
 * @param arg It is not useful.
 */
void router_request_cb(evhtp_request_t* req, void* arg) {
    int * aux;
    int   thr;

    aux = (int *)evthr_get_aux(req->conn->thread);
    thr = *aux;

    printf("  Received router request on thread %d... ", thr);

    /* Pause the router request while we run the backend requests. */
    evhtp_request_pause(req);

    struct event_base* base;
    struct evdns_base* dns_base;
    struct bufferevent* bev;

    base = event_base_new();
    dns_base = evdns_base_new(base, 1);
    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, backend_cb, NULL, eventcb, req);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
    evbuffer_add_printf(bufferevent_get_output(bev), "GET / HTTP/1.1\r\n");
    bufferevent_socket_connect_hostname(
            bev, dns_base, AF_UNSPEC, PAYMENT_SERVER, PAYMENT_PORT);
    event_base_dispatch(base);

    printf("Ok.\n");
}

