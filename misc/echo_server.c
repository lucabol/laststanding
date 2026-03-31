// TCP echo server demo — handles multiple clients simultaneously via l_poll.
// This is a showcase program, not an automated test (it blocks waiting for connections).
// Build: compile with L_WITHSOCKETS defined. On Windows, link -lws2_32.
// Usage: run the binary, then connect with: nc 127.0.0.1 7878
//        multiple clients can connect at once (up to 16)

#define L_WITHSOCKETS
#define L_MAINFILE
#include "l_os.h"

#define MAX_CLIENTS 16

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    L_SOCKET server = l_socket_tcp();
    if (server < 0) { puts("Failed to create socket\n"); return 1; }
    if (l_socket_bind(server, 7878) < 0) { puts("Failed to bind to port 7878\n"); l_socket_close(server); return 1; }
    if (l_socket_listen(server, MAX_CLIENTS) < 0) { puts("Failed to listen\n"); l_socket_close(server); return 1; }

    puts("Echo server listening on port 7878 (max 16 clients)...\n");

    L_PollFd fds[1 + MAX_CLIENTS];
    int nfds = 1;

    // Slot 0 = listener
    fds[0].fd = (L_FD)server;
    fds[0].events = L_POLLIN;
    fds[0].revents = 0;

    for (;;) {
        int ready = l_poll(fds, nfds, -1);
        if (ready < 0) { puts("poll error\n"); break; }

        // Check listener for new connections
        if (fds[0].revents & L_POLLIN) {
            L_SOCKET client = l_socket_accept(server);
            if (client >= 0 && nfds < 1 + MAX_CLIENTS) {
                fds[nfds].fd = (L_FD)client;
                fds[nfds].events = L_POLLIN;
                fds[nfds].revents = 0;
                nfds++;
                puts("Client connected.\n");
            } else if (client >= 0) {
                puts("Max clients reached, rejecting.\n");
                l_socket_close(client);
            }
        }

        // Check each client slot
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & (L_POLLERR | L_POLLHUP)) {
                puts("Client disconnected.\n");
                l_socket_close((L_SOCKET)fds[i].fd);
                fds[i] = fds[nfds - 1];
                nfds--;
                i--;
                continue;
            }
            if (fds[i].revents & L_POLLIN) {
                char buf[1024];
                ptrdiff_t n = l_socket_recv((L_SOCKET)fds[i].fd, buf, sizeof(buf));
                if (n <= 0) {
                    puts("Client disconnected.\n");
                    l_socket_close((L_SOCKET)fds[i].fd);
                    fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                } else {
                    l_socket_send((L_SOCKET)fds[i].fd, buf, (size_t)n);
                }
            }
        }
    }

    for (int i = 1; i < nfds; i++)
        l_socket_close((L_SOCKET)fds[i].fd);
    l_socket_close(server);
    return 0;
}
