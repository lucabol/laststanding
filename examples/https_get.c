// examples/https_get.c — Fetch an HTTPS page (Windows only, freestanding)
//
// Usage: bin/https_get [hostname] [path]
//   Default: fetches https://example.com/
//
// Demonstrates l_tls.h for HTTPS GET requests with zero external dependencies.

#define L_MAINFILE
#include "l_tls.h"

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    const char *host = argc > 1 ? argv[1] : "example.com";
    const char *path = argc > 2 ? argv[2] : "/";

#if !L_TLS_AVAILABLE
    puts("TLS not available on this platform.\n");
    (void)host; (void)path;
    return 0;
#else
    if (l_tls_init() != 0) {
        puts("TLS init failed\n");
        return 1;
    }

    puts("Connecting to ");
    puts(host);
    puts(":443...\n");

    int h = l_tls_connect(host, 443);
    if (h < 0) {
        puts("TLS connect failed\n");
        l_tls_cleanup();
        return 1;
    }

    puts("Connected. Sending GET request...\n");

    // Build HTTP request manually (no snprintf in freestanding)
    char req[512];
    int off = 0;
    const char *p;

    // "GET "
    for (p = "GET "; *p; p++) req[off++] = *p;
    // path
    for (p = path; *p && off < 400; p++) req[off++] = *p;
    // " HTTP/1.1\r\n"
    for (p = " HTTP/1.1\r\n"; *p; p++) req[off++] = *p;
    // "Host: "
    for (p = "Host: "; *p; p++) req[off++] = *p;
    // hostname
    for (p = host; *p && off < 480; p++) req[off++] = *p;
    // "\r\nConnection: close\r\n\r\n"
    for (p = "\r\nConnection: close\r\n\r\n"; *p; p++) req[off++] = *p;
    req[off] = '\0';

    if (l_tls_send(h, req, off) < 0) {
        puts("TLS send failed\n");
        l_tls_close(h);
        l_tls_cleanup();
        return 1;
    }

    // Read and display response
    char buf[4096];
    int n;
    while ((n = l_tls_recv(h, buf, (int)sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        puts(buf);
    }

    l_tls_close(h);
    l_tls_cleanup();
    return 0;
#endif
}
