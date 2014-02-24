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

static char *server_name = "PaymentGateway/1.0.0 (Unix)";

static int print_headers(evhtp_header_t* header, void* arg);

static const char* method_strmap[] = {
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


/**
 * @brief router_request_cb The callback of a dump request.
 *
 * @param req The request you want to dump.
 * @param arg It is not useful.
 */
void router_request_cb(evhtp_request_t *req, void *arg)
{
    const char *uri = req->uri->path->full;

    int req_method = evhtp_request_get_method(req);
    if (req_method >= 16) {
        req_method = 16;
    }

    //LOG_PRINT(LOG_INFO, "Received a %s request for %s", method_strmap[req_method], uri);
    evbuffer_add_printf(req->buffer_out, "uri : %s\r\n", uri);
    evbuffer_add_printf(req->buffer_out, "query : %s\r\n", req->uri->query_raw);
    evhtp_headers_for_each(req->uri->query, print_headers, req->buffer_out);
    evbuffer_add_printf(req->buffer_out, "Method : %s\n", method_strmap[req_method]);
    evhtp_headers_for_each(req->headers_in, print_headers, req->buffer_out);

    evbuf_t *buf = req->buffer_in;;
    puts("Input data: <<<");
    while (evbuffer_get_length(buf)) {
        int n;
        char cbuf[128];
        n = evbuffer_remove(buf, cbuf, sizeof(buf)-1);
        if (n > 0)
            (void) fwrite(cbuf, 1, n, stdout);
    }
    puts(">>>");

    //evhtp_headers_add_header(req->headers_out, evhtp_header_new("Server", "zimg/1.0.0 (Unix) (OpenSUSE/Linux)", 0, 0));
    evhtp_headers_add_header(req->headers_out, evhtp_header_new("Server", server_name, 0, 0));
    evhtp_headers_add_header(req->headers_out, evhtp_header_new("Content-Type", "text/plain", 0, 0));
    evhtp_send_reply(req, EVHTP_RES_OK);
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
    evbuf_t * buf = arg;

    evbuffer_add(buf, header->key, header->klen);
    evbuffer_add(buf, ": ", 2);
    evbuffer_add(buf, header->val, header->vlen);
    evbuffer_add(buf, "\r\n", 2);
    return 1;
}

