// tests/test_tls_live.c — Live-fire TLS regression test.
//
// This is an OPT-IN network test: it only runs when the environment variable
// L_TLS_LIVE=1 is set. Otherwise it prints a skip notice and exits 0 so it
// is safe to drop into CI.
//
// When enabled, it connects to www.lucabol.com and text.npr.org over TLS,
// issues a GET /, and verifies that:
//   * l_tls_connect completes (handshake succeeds)
//   * a well-formed HTTP response arrives
//   * l_tls_recv terminates on clean EOF (returns 0), not error (returns -1)
//
// The handshake path is the one that used to spin on SEC_E_INCOMPLETE_MESSAGE
// when the server's TLS handshake was fragmented. The EOF path exercises the
// SEC_I_CONTEXT_EXPIRED (close_notify) handling.
//
// Manual run:
//     set L_TLS_LIVE=1 && bin\test_tls_live.exe

#define L_MAINFILE
#include "l_tls.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

#if L_TLS_AVAILABLE
static int fetch_site(const char *host) {
    int h = l_tls_connect(host, 443);
    if (h < 0) return -1;

    char req[512];
    int off = 0;
    const char *p;
    for (p = "GET / HTTP/1.1\r\nHost: "; *p; p++) req[off++] = *p;
    for (p = host;                        *p; p++) req[off++] = *p;
    for (p = "\r\nUser-Agent: l_tls-test\r\nConnection: close\r\n\r\n";
                                          *p; p++) req[off++] = *p;

    if (l_tls_send(h, req, off) < 0) { l_tls_close(h); return -2; }

    char buf[4096];
    char head[16] = {0};
    int head_len = 0;
    int n, total = 0;
    int saw_eof = 0;
    while ((n = l_tls_recv(h, buf, (int)sizeof(buf))) != 0) {
        if (n < 0) { l_tls_close(h); return -3; }
        if (head_len < 15) {
            int copy = n;
            if (head_len + copy > 15) copy = 15 - head_len;
            for (int i = 0; i < copy; i++) head[head_len++] = buf[i];
            head[head_len] = '\0';
        }
        total += n;
    }
    saw_eof = 1;
    l_tls_close(h);

    if (!saw_eof) return -4;
    if (total < 100) return -5;
    // Response must start with "HTTP/1."
    if (head[0] != 'H' || head[1] != 'T' || head[2] != 'T' || head[3] != 'P' ||
        head[4] != '/' || head[5] != '1' || head[6] != '.') return -6;
    return total;
}
#endif

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    char *live = l_getenv("L_TLS_LIVE");
    if (!live || live[0] != '1') {
        puts("test_tls_live: set L_TLS_LIVE=1 to enable (skipping).\n");
        puts("PASS\n");
        return 0;
    }

#if !L_TLS_AVAILABLE
    puts("test_tls_live: TLS not available on this platform.\n");
    puts("PASS\n");
    return 0;
#else
    puts("Testing live TLS against real hosts...\n");
    if (l_tls_init() != 0) { puts("FAIL: l_tls_init\n"); return 1; }

    TEST_FUNCTION("www.lucabol.com");
    int r1 = fetch_site("www.lucabol.com");
    TEST_ASSERT(r1 > 0, "fetch https://www.lucabol.com/ returns body and clean EOF");

    TEST_FUNCTION("text.npr.org");
    int r2 = fetch_site("text.npr.org");
    TEST_ASSERT(r2 > 0, "fetch https://text.npr.org/ returns body and clean EOF");

    l_tls_cleanup();

    l_test_print_summary(passed_count, test_count);
    if (passed_count != test_count) { puts("FAIL\n"); exit(1); }
    puts("PASS\n");
    return 0;
#endif
}
