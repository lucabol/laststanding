// examples/tls_npr.c — Fetch https://text.npr.org/ over TLS.
//
// Regression example: NPR's TLS edge is a large, realistic server that
// exercises fragmented handshake records and TLS 1.3 post-handshake messages
// (NewSessionTicket). This used to interact badly with bugs in l_tls.h's
// Windows SChannel handshake loop. Keeping this in the tree gives us a
// live-fire smoke test against a second independent site.
//
// Usage: bin/tls_npr

#define L_MAINFILE
#include "l_tls.h"

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

#if !L_TLS_AVAILABLE
    puts("TLS not available on this platform.\n");
    return 0;
#else
    const char *host = "text.npr.org";
    const char *path = "/";

    if (l_tls_init() != 0) { puts("TLS init failed\n"); return 1; }

    puts("Connecting to text.npr.org:443 ...\n");
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

    // Only print the first ~2 KB — NPR's homepage is large.
    char buf[4096];
    int n, total = 0, printed = 0;
    while ((n = l_tls_recv(h, buf, (int)sizeof(buf) - 1)) > 0) {
        if (printed < 2048) {
            int show = n;
            if (printed + show > 2048) show = 2048 - printed;
            buf[show] = '\0';
            puts(buf);
            printed += show;
        }
        total += n;
    }

    if (n < 0) {
        puts("\n[TLS recv error]\n");
        l_tls_close(h); l_tls_cleanup(); return 1;
    }

    puts("\n[tls_npr: clean EOF, bytes = ");
    char num[16]; l_itoa(total, num, 10); puts(num);
    puts("]\n");

    l_tls_close(h);
    l_tls_cleanup();
    return 0;
#endif
}
