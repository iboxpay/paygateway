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

#include <stdio.h>
#include "httpd.h"

static int make_http_request(evbase_t* evbase,
             evthr_t* evthr,
             const char* const host,
             const short port,
             const char* const path,
             evhtp_headers_t* headers,
             evhtp_callback_cb  cb,
             void* arg) {
    evhtp_connection_t * conn;
    evhtp_request_t    * request;

    conn         = evhtp_connection_new(evbase, host, port);
    conn->thread = evthr;
    request      = evhtp_request_new(cb, arg);

    evhtp_headers_add_header(request->headers_out,
                             evhtp_header_new("Host", "localhost", 0, 0));
    evhtp_headers_add_header(request->headers_out,
                             evhtp_header_new("User-Agent", "libevhtp", 0, 0));
    evhtp_headers_add_header(request->headers_out,
                             evhtp_header_new("Connection", "close", 0, 0));

    evhtp_headers_add_headers(request->headers_out, headers);

    printf("Making backend request...\n");
    evhtp_make_request(conn, request, htp_method_GET, path);
    printf("Ok.\n");

    return 0;
}

static void http_backend_cb(evhtp_request_t * backend_req, void * arg) {
    evhtp_request_t * frontend_req = (evhtp_request_t *)arg;

    evbuffer_prepend_buffer(frontend_req->buffer_out, backend_req->buffer_in);
    evhtp_headers_add_headers(frontend_req->headers_out, backend_req->headers_in);

    /*
     * char body[1024] = { '\0' };
     * ev_ssize_t len = evbuffer_copyout(frontend_req->buffer_out, body, sizeof(body));
     * printf("Backend %zu: %s\n", len, body);
     */

    evhtp_send_reply(frontend_req, EVHTP_RES_OK);
    evhtp_request_resume(frontend_req);
}

/**
 * @brief frontend_request_cb The callback of a frontend request.
 *
 * @param req The request you want to dump.
 * @param arg It is not useful.
 */
void frontend_request_cb(evhtp_request_t* req, void* arg) {
    int * aux;
    int   thr;

    aux = (int *)evthr_get_aux(req->conn->thread);
    thr = *aux;

    printf("  Received frontend request on thread %d... ", thr);

    /* Pause the frontend request while we run the backend requests. */
    evhtp_request_pause(req);

    make_http_request(evthr_get_base(req->conn->thread),
                 req->conn->thread,
                 "127.0.0.1", 8000,
                 req->uri->path->full,
                 req->headers_in, http_backend_cb, req);

    printf("Ok.\n");
}

