#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <event2/event.h>
#include <event2/util.h>

#define PAYMENT_PORT 8888
#define PAYMENT_SERVER "127.0.0.1"

struct payment_context {
    struct event_base* base;
    struct event* event_write;
    struct event* event_read;
    const char* payment_contents;
    int payment_contents_len;
    int recved;
};

void request_cb(evutil_socket_t sock, short flags, void* args) {
    struct payment_context* pc = (struct payment_context *)args;
    ssize_t ret = send(sock, pc->payment_contents, pc->payment_contents_len, 0);
    printf("connected, write to payment server: %zd\n", ret);
    event_add(pc->event_read, 0);
}

void response_cb(evutil_socket_t sock, short flags, void* args) {
    struct payment_context* pc = (struct payment_context*) args;
    char buf[128] = {0};
    ssize_t ret = recv(sock, buf, 128, 0);
    printf("response_cb, read %zd bytes\n", ret);
    if (ret > 0) {
        pc->recved += ret;
        printf("recv:%s\n", buf);
    } else if (ret == 0) {
        printf("response_cb connection closed\n");
        event_base_loopexit(pc->base, NULL);
    }

    if (pc->recved < pc->payment_contents_len) {
        event_add(pc->event_read, 0);
    }
}

static evutil_socket_t make_tcp_socket() {
    int on = 1;
    evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    evutil_make_socket_nonblocking(sock);
    setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void* )&on, sizeof(on));

    return sock;
}

static void payment_client(struct event_base* base) {
    evutil_socket_t sock = make_tcp_socket();
    struct sockaddr_in serverAddr;
    struct event* ev_write = 0;
    struct event* ev_read = 0;
    struct timeval tv = {10, 0};
    struct payment_context* pc = (struct payment_context*)calloc(1, sizeof(struct payment_context));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PAYMENT_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(PAYMENT_SERVER);
    memset(serverAddr.sin_zero, 0x00, 8);
    connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    ev_write = event_new(base, sock, EV_WRITE, request_cb, (void*)pc);
    ev_read = event_new(base, sock, EV_READ, response_cb, (void*)pc);

    pc->event_write = ev_write;
    pc->event_read = ev_read;
    pc->base = base;
    pc->payment_contents = strdup("payment client tneilc ohce\n");
    pc->payment_contents_len = strlen(pc->payment_contents);
    pc->recved = 0;

    event_add(ev_write, &tv);
}

int main(int argc, const char *argv[]) {
    struct event_base * base = 0;

    base = event_base_new();
    payment_client(base);
    event_base_dispatch(base);
    event_base_free(base);

    return 0;
}

