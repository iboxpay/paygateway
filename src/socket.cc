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

#include <iostream>
#include <string>

#include "socket.h"
#include "cup_payment.pb.h"
#include "src/base/base64.h"

#define PAYMENT_PORT 8000
#define PAYMENT_SERVER "127.0.0.1"
static const char *server_name = "PayGateway/1.0.0 (Unix)";

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

static int make_socket_request(evbase_t* base,
             const char* const host,
             unsigned short port,
             const char* data,
             bufferevent_data_cb read_cb,
             bufferevent_event_cb event_cb,
             void* ctx) {
    struct timeval tv;
    struct evdns_base* dns_base;
    struct bufferevent* bev;

    dns_base = evdns_base_new(base, 1);
    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, read_cb, NULL, event_cb, ctx);

    //  Set timeout.
    /* tv.tv_sec = 30; */
    /* tv.tv_usec = 0; */
    /* bufferevent_set_timeouts(bev, &tv, NULL); */

    bufferevent_enable(bev, EV_READ|EV_WRITE);
    evbuffer_add_printf(bufferevent_get_output(bev), "%s", data);
    bufferevent_socket_connect_hostname(
            bev, dns_base, AF_UNSPEC, host, port);
    return 0;
}

static void backend_cb(struct bufferevent* bev, void* ctx) {
    evhtp_request_t* req = (evhtp_request_t *)ctx;

    const char *uri = req->uri->path->full;

    int req_method = evhtp_request_get_method(req);
    if (req_method >= 16) {
        req_method = 16;
    }

    evbuffer_add_printf(req->buffer_out, "uri : %s\r\n", uri);
    evbuffer_add_printf(req->buffer_out, "query : %s\r\n", req->uri->query_raw);
    evhtp_headers_for_each(req->uri->query, print_headers, req->buffer_out);
    evbuffer_add_printf(req->buffer_out, "Method : %s\n", method_strmap[req_method]);
    evhtp_headers_for_each(req->headers_in, print_headers, req->buffer_out);

    puts(">>>");

    evhtp_headers_add_header(req->headers_out, evhtp_header_new("Server", server_name, 0, 0));
    evhtp_headers_add_header(req->headers_out, evhtp_header_new("Content-Type", "text/plain", 0, 0));
    evhtp_headers_add_header(req->headers_out, evhtp_header_new("Connection", "Keep-Alive", 0, 0));

    evhtp_send_reply(req, EVHTP_RES_OK);
    evhtp_request_resume(req);
}

static void eventcb(struct bufferevent* bev, short events, void* ctx) {
    int finished = 0;

    if (events & BEV_EVENT_CONNECTED) {
        printf("Connect okay.\n");
    } else if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
        int err = bufferevent_socket_get_dns_error(bev);
        if (err) {
            printf("DNS error: %s\n", evutil_gai_strerror(err));
        }
        finished = 1;
        printf("Closing\n");
    } else if (events & BEV_EVENT_TIMEOUT) {
        finished = 1;
        printf("Timed out\n");
    }

    if (finished) {
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

    puts("Input data: <<<");
    const char* version = evhtp_kv_find(req->uri->query, "version");
    const char* request = evhtp_kv_find(req->uri->query, "request");

    if (version == NULL || request == NULL) {
        evhtp_send_reply(req, EVHTP_RES_OK);
        return;
    }

    std::cout << version << std::endl;

    const std::string encoded = request;
    std::string decoded = base64_decode(encoded);
    cup::SaleRequest sale_req;
    sale_req.ParseFromString(decoded);
    std::cout << sale_req.cmdtype() << std::endl;
    std::cout << sale_req.signtype() << std::endl;


    /* Pause the router request while we run the backend requests. */
    evhtp_request_pause(req);

    make_socket_request(evthr_get_base(req->conn->thread), PAYMENT_SERVER, PAYMENT_PORT,
            "GET / HTTP/1.1\r\n",
            backend_cb, eventcb, req);
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
static int print_headers(evhtp_header_t * header, void * arg) {
    evbuf_t* buf = (evbuf_t*)arg;

    evbuffer_add(buf, header->key, header->klen);
    evbuffer_add(buf, ": ", 2);
    evbuffer_add(buf, header->val, header->vlen);
    evbuffer_add(buf, "\r\n", 2);
    return 1;
}

