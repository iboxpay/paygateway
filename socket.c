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
#define PAYMENT_SERVER "127.0.0.1"
static char *server_name = "PayGateway/1.0.0 (Unix)";

struct payment_context {
    const char* payment_contents;
    int payment_contents_len;
    int recved;
};

static const char * method_strmap[] = {
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "MKCOL",
    "COPY",
    "MOVE",
    "OPTIONS",
    "PROPFIND",
    "PROPATCH",
    "LOCK",
    "UNLOCK",
    "TRACE",
    "CONNECT",
    "PATCH",
    "UNKNOWN",
};

static int print_headers(evhtp_header_t * header, void * arg);

static void backend_cb(struct bufferevent* bev, void* ctx) {
    evhtp_request_t* req = (evhtp_request_t *)ctx;

    const char *uri = req->uri->path->full;

    int req_method = evhtp_request_get_method(req);
    if (req_method >= 16) {
        req_method = 16;
    }

    struct evbuffer* output = bufferevent_get_output(bev);
    struct evbuffer* input = bufferevent_get_input(bev);

    size_t input_len = evbuffer_get_length(input);
    printf("input_len: %zu\n", input_len);

    size_t output_len1 = evbuffer_get_length(output);
    printf("output_len1: %zu\n\n", output_len1);

    evbuffer_add_printf(req->buffer_out, "uri : %s\r\n", uri);
    evbuffer_add_printf(req->buffer_out, "query : %s\r\n", req->uri->query_raw);
    evhtp_headers_for_each(req->uri->query, print_headers, req->buffer_out);
    evbuffer_add_printf(req->buffer_out, "Method : %s\n", method_strmap[req_method]);
    evhtp_headers_for_each(req->headers_in, print_headers, req->buffer_out);

    puts(">>>");

    evhtp_headers_add_header(req->headers_out, evhtp_header_new("Server", server_name, 0, 0));
    evhtp_headers_add_header(req->headers_out, evhtp_header_new("Content-Type", "text/plain", 0, 0));

    evhtp_send_reply(req, EVHTP_RES_OK);
    evhtp_request_resume(req);
}

static void eventcb(struct bufferevent* bev, short events, void* ctx) {
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


    evbuf_t *buf = req->buffer_in;;
    puts("Input data: <<<");
    while (evbuffer_get_length(buf)) {
        int n;
        char cbuf[128];
        n = evbuffer_remove(buf, cbuf, sizeof(buf)-1);
        if (n > 0)
            (void) fwrite(cbuf, 1, n, stdout);
    }

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

/**
 * @brief print_headers It displays all headers and values.
 *
 * @param header The header of a request.
 * @param arg The evbuff you want to store the k-v string.
 *
 * @return It always return 1 for success.
 */
static int print_headers(evhtp_header_t * header, void * arg)
{
    evbuf_t * buf = arg;

    evbuffer_add(buf, header->key, header->klen);
    evbuffer_add(buf, ": ", 2);
    evbuffer_add(buf, header->val, header->vlen);
    evbuffer_add(buf, "\r\n", 2);
    return 1;
}

