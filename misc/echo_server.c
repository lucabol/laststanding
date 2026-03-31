// TCP echo server demo — accepts one connection and echoes back data.
// This is a showcase program, not an automated test (it blocks waiting for connections).
// Build: compile with L_WITHSOCKETS defined. On Windows, link -lws2_32.
// Usage: run the binary, then connect with: nc 127.0.0.1 7878

#define L_WITHSOCKETS
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    L_SOCKET server = l_socket_tcp();
    if (server < 0) {
        puts("Failed to create socket\n");
        return 1;
    }

    if (l_socket_bind(server, 7878) < 0) {
        puts("Failed to bind to port 7878\n");
        l_socket_close(server);
        return 1;
    }

    if (l_socket_listen(server, 1) < 0) {
        puts("Failed to listen\n");
        l_socket_close(server);
        return 1;
    }

    puts("Echo server listening on port 7878...\n");

    L_SOCKET client = l_socket_accept(server);
    if (client < 0) {
        puts("Failed to accept connection\n");
        l_socket_close(server);
        return 1;
    }

    puts("Client connected. Echoing...\n");

    char buf[1024];
    ptrdiff_t n;
    while ((n = l_socket_recv(client, buf, sizeof(buf))) > 0) {
        l_socket_send(client, buf, (size_t)n);
    }

    puts("Client disconnected.\n");
    l_socket_close(client);
    l_socket_close(server);
    return 0;
}
