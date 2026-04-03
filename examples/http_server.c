// HTTP server demo — multi-client via l_poll, multiple endpoints.
// Build: compile with L_WITHSOCKETS and L_WITHSNPRINTF defined. On Windows, link -lws2_32.
// Usage: http_server [port]   (default 8080)
//        Ctrl+C to shut down gracefully.

#define L_WITHSOCKETS
#define L_WITHSNPRINTF
#define L_MAINFILE
#include "l_os.h"

#define MAX_CLIENTS 16
#define BUF_SIZE    4096

static volatile int running = 1;
static int request_count = 0;
static int connection_count = 0;
static long long start_time = 0;

static void sigint_handler(int sig) { (void)sig; running = 0; }

static void timestamp(char *buf, size_t len) {
    long long t = l_time(0);
    L_Tm tm = l_localtime(t);
    l_strftime(buf, len, "%Y-%m-%d %H:%M:%S", &tm);
}

static void log_request(const char *method, const char *path, int status, int bytes) {
    char ts[32];
    timestamp(ts, sizeof(ts));
    l_printf("[%s] %s %s -> %d (%d bytes)\n", ts, method, path, status, bytes);
}

// Build an HTTP response into buf, return total length
static int make_response(char *buf, size_t cap, int status, const char *status_text,
                         const char *content_type, const char *body, int body_len) {
    return l_snprintf(buf, cap,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n%s",
        status, status_text, content_type, body_len, body);
}

static void handle_request(L_SOCKET client) {
    char req[BUF_SIZE];
    ptrdiff_t n = l_socket_recv(client, req, sizeof(req) - 1);
    if (n <= 0) return;
    req[n] = '\0';

    // Parse "GET /path HTTP/..."
    char *method_end = l_strchr(req, ' ');
    if (!method_end) return;
    char *path_start = method_end + 1;
    char *path_end = l_strchr(path_start, ' ');
    if (!path_end) return;
    *path_end = '\0';

    char resp[BUF_SIZE];
    char body[2048];
    int body_len, resp_len, status;
    const char *ctype;

    request_count++;

    if (l_strcmp(path_start, "/") == 0) {
        status = 200;
        ctype = "text/html";
        body_len = l_snprintf(body, sizeof(body),
            "<html><body>"
            "<h1>laststanding HTTP server</h1>"
            "<ul>"
            "<li><a href=\"/hello\">/hello</a> - greeting</li>"
            "<li><a href=\"/time\">/time</a> - current time</li>"
            "<li><a href=\"/json\">/json</a> - sample JSON</li>"
            "<li><a href=\"/stats\">/stats</a> - server stats</li>"
            "</ul></body></html>");
    } else if (l_strcmp(path_start, "/hello") == 0) {
        status = 200;
        ctype = "text/plain";
        body_len = l_snprintf(body, sizeof(body), "Hello from laststanding!");
    } else if (l_strcmp(path_start, "/time") == 0) {
        char ts[64];
        timestamp(ts, sizeof(ts));
        status = 200;
        ctype = "text/plain";
        body_len = l_snprintf(body, sizeof(body), "%s", ts);
    } else if (l_strcmp(path_start, "/json") == 0) {
        status = 200;
        ctype = "application/json";
        body_len = l_snprintf(body, sizeof(body),
            "{\"name\":\"laststanding\",\"version\":\"1.0\","
            "\"functions\":230,"
            "\"platforms\":[\"windows\",\"linux\",\"arm\",\"aarch64\"]}");
    } else if (l_strcmp(path_start, "/stats") == 0) {
        long long uptime = l_time(0) - start_time;
        status = 200;
        ctype = "text/plain";
        body_len = l_snprintf(body, sizeof(body),
            "Uptime: %lld seconds\nRequests served: %d\nConnections: %d",
            uptime, request_count, connection_count);
    } else {
        status = 404;
        ctype = "text/plain";
        body_len = l_snprintf(body, sizeof(body), "404 Not Found: %s", path_start);
    }

    resp_len = make_response(resp, sizeof(resp), status,
                             status == 200 ? "OK" : "Not Found", ctype, body, body_len);
    l_socket_send(client, resp, (size_t)resp_len);
    log_request("GET", path_start, status, body_len);
}

int main(int argc, char *argv[]) {
    int port = 8080;
    if (argc > 1) port = l_atoi(argv[1]);

    l_signal(L_SIGINT, sigint_handler);
    start_time = l_time(0);

    L_SOCKET server = l_socket_tcp();
    if (server < 0) { l_puts("Failed to create socket\n"); return 1; }
    if (l_socket_bind(server, port) < 0) { l_puts("Failed to bind\n"); l_socket_close(server); return 1; }
    if (l_socket_listen(server, MAX_CLIENTS) < 0) { l_puts("Failed to listen\n"); l_socket_close(server); return 1; }

    l_printf("HTTP server listening on port %d (Ctrl+C to stop)\n", port);

    L_PollFd fds[1 + MAX_CLIENTS];
    int nfds = 1;
    fds[0].fd = (L_FD)server;
    fds[0].events = L_POLLIN;
    fds[0].revents = 0;

    while (running) {
        int ready = l_poll(fds, nfds, 1000);
        if (ready < 0) break;
        if (ready == 0) continue;

        // Accept new connections
        if (fds[0].revents & L_POLLIN) {
            L_SOCKET client = l_socket_accept(server);
            if (client >= 0 && nfds < 1 + MAX_CLIENTS) {
                fds[nfds].fd = (L_FD)client;
                fds[nfds].events = L_POLLIN;
                fds[nfds].revents = 0;
                nfds++;
                connection_count++;
            } else if (client >= 0) {
                l_socket_close(client);
            }
        }

        // Handle client data
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & (L_POLLIN | L_POLLERR | L_POLLHUP)) {
                if (fds[i].revents & L_POLLIN)
                    handle_request((L_SOCKET)fds[i].fd);
                l_socket_close((L_SOCKET)fds[i].fd);
                fds[i] = fds[nfds - 1];
                nfds--;
                i--;
            }
        }
    }

    l_puts("Shutting down...\n");
    for (int i = 1; i < nfds; i++)
        l_socket_close((L_SOCKET)fds[i].fd);
    l_socket_close(server);
    return 0;
}
