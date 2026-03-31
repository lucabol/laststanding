#define L_WITHSOCKETS
#define L_MAINFILE
#include "l_os.h"

// http_get: simple HTTP GET over TCP sockets
// Usage: http_get <ip> <port> <path>
// Example: http_get 93.184.216.34 80 /index.html
// NOTE: IP address only (no DNS resolution) — use tools like `dig` to resolve hostnames first.

int main(int argc, char *argv[]) {
    if (argc != 4) {
        l_puts("Usage: http_get <ip> <port> <path>\n");
        l_puts("Perform a simple HTTP GET request.\n");
        l_puts("NOTE: IP address only, no DNS resolution.\n");
        l_puts("Example: http_get 93.184.216.34 80 /index.html\n");
        return 0;
    }

    const char *ip = argv[1];
    int port = l_atoi(argv[2]);
    const char *path = argv[3];

    L_SOCKET sock = l_socket_tcp();
    if (sock < 0) { l_puts("Error: cannot create socket\n"); return 1; }

    if (l_socket_connect(sock, ip, port) < 0) {
        l_puts("Error: cannot connect to ");
        l_puts(ip);
        l_puts("\n");
        l_socket_close(sock);
        return 1;
    }

    // Build HTTP request
    L_Buf req;
    l_buf_init(&req);
    l_buf_push_cstr(&req, "GET ");
    l_buf_push_cstr(&req, path);
    l_buf_push_cstr(&req, " HTTP/1.1\r\nHost: ");
    l_buf_push_cstr(&req, ip);
    l_buf_push_cstr(&req, "\r\nConnection: close\r\n\r\n");

    l_socket_send(sock, req.data, req.len);
    l_buf_free(&req);

    // Read response with poll timeout
    L_Buf resp;
    l_buf_init(&resp);
    char buf[4096];

    for (;;) {
        L_PollFd pfd;
        pfd.fd = (L_FD)sock;
        pfd.events = L_POLLIN;
        pfd.revents = 0;

        int ready = l_poll(&pfd, 1, 5000); // 5 second timeout
        if (ready <= 0) break;

        if (pfd.revents & (L_POLLERR | L_POLLHUP)) break;
        if (pfd.revents & L_POLLIN) {
            ptrdiff_t n = l_socket_recv(sock, buf, sizeof(buf));
            if (n <= 0) break;
            l_buf_push(&resp, buf, (size_t)n);
        }
    }

    // Print response to stdout
    if (resp.len > 0)
        l_write(L_STDOUT, resp.data, resp.len);

    l_buf_free(&resp);
    l_socket_close(sock);
    return 0;
}
