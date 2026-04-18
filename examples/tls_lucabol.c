// examples/tls_lucabol.c — Fetch https://www.lucabol.com/ over TLS.
//
// Regression example: this site exercises a TLS 1.2/1.3 handshake whose
// server hello + certificate chain has historically been fragmented across
// TCP segments. That is exactly the path that used to trigger the
// SEC_E_INCOMPLETE_MESSAGE handshake spin bug in l_tls.h. Keep this example
// in the tree so builds regularly cover that code path.
//
// Usage: bin/tls_lucabol
//
// Prints HTTP status line + response body (up to a few KB) and exits 0 on
// success.

#define L_MAINFILE
#include "l_tls.h"

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

#if !L_TLS_AVAILABLE
    puts("TLS not available on this platform.\n");
    return 0;
#else
    const char *host = "www.lucabol.com";
    const char *path = "/";

    if (l_tls_init() != 0) { puts("TLS init failed\n"); return 1; }

    puts("Connecting to www.lucabol.com:443 ...\n");
    int h = l_tls_connect(host, 443);
    if (h < 0) { puts("TLS connect failed\n"); l_tls_cleanup(); return 1; }

    puts("Handshake OK. Sending GET / ...\n");

    char req[512];
    int off = 0;
    const char *p;
    for (p = "GET ";                             *p; p++) req[off++] = *p;
    for (p = path;                               *p; p++) req[off++] = *p;
    for (p = " HTTP/1.1\r\nHost: ";              *p; p++) req[off++] = *p;
    for (p = host;                               *p; p++) req[off++] = *p;
    for (p = "\r\nUser-Agent: l_tls-example\r\nConnection: close\r\n\r\n";
                                                 *p; p++) req[off++] = *p;

    if (l_tls_send(h, req, off) < 0) {
        puts("TLS send failed\n");
        l_tls_close(h); l_tls_cleanup(); return 1;
    }

    char buf[4096];
    int n, total = 0;
    while ((n = l_tls_recv(h, buf, (int)sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        puts(buf);
        total += n;
    }

    if (n < 0) {
        puts("\n[TLS recv error]\n");
        l_tls_close(h); l_tls_cleanup(); return 1;
    }

    puts("\n[tls_lucabol: clean EOF, bytes = ");
    char num[16]; l_itoa(total, num, 10); puts(num);
    puts("]\n");

    l_tls_close(h);
    l_tls_cleanup();
    return 0;
#endif
}
