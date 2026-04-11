// tests/test_tls.c — Tests for l_tls.h TLS client
#define L_MAINFILE
#include "l_tls.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

static void test_init_cleanup(void) {
    TEST_FUNCTION("l_tls_init / l_tls_cleanup");
#if L_TLS_AVAILABLE
    TEST_ASSERT(l_tls_init() == 0, "TLS init succeeds on Windows");
    TEST_ASSERT(l_tls_init() == 0, "TLS double-init is idempotent");
    l_tls_cleanup();
    // Re-init after cleanup should work
    TEST_ASSERT(l_tls_init() == 0, "TLS re-init after cleanup succeeds");
    l_tls_cleanup();
#else
    TEST_ASSERT(l_tls_init() == -1, "TLS init returns -1 (not available)");
    l_tls_cleanup(); // no-op, should not crash
    TEST_ASSERT(1, "TLS cleanup no-op did not crash");
#endif
}

static void test_invalid_handles(void) {
    TEST_FUNCTION("invalid handle operations");
    TEST_ASSERT(l_tls_send(-1, "x", 1) == -1, "send on handle -1 returns -1");
    TEST_ASSERT(l_tls_send(99, "x", 1) == -1, "send on handle 99 returns -1");
    TEST_ASSERT(l_tls_recv_byte(-1) == -1, "recv_byte on handle -1 returns -1");
    TEST_ASSERT(l_tls_recv_byte(99) == -1, "recv_byte on handle 99 returns -1");
    char buf[16];
    TEST_ASSERT(l_tls_recv(-1, buf, 16) == -1, "recv on handle -1 returns -1");
    TEST_ASSERT(l_tls_recv(99, buf, 16) == -1, "recv on handle 99 returns -1");
    // close on invalid handle should not crash
    l_tls_close(-1);
    l_tls_close(99);
    TEST_ASSERT(1, "close on invalid handles did not crash");
}

static void test_connect_without_init(void) {
    TEST_FUNCTION("connect without init");
#if L_TLS_AVAILABLE
    // connect should auto-init
    // We connect to a non-existent host — should fail gracefully
    int h = l_tls_connect("192.0.2.1", 443); // TEST-NET, guaranteed unreachable
    TEST_ASSERT(h == -1, "connect to unreachable host returns -1");
    l_tls_cleanup();
#else
    int h = l_tls_connect("example.com", 443);
    TEST_ASSERT(h == -1, "connect returns -1 (not available)");
#endif
}

static void test_availability_macro(void) {
    TEST_FUNCTION("L_TLS_AVAILABLE macro");
#ifdef _WIN32
    TEST_ASSERT(L_TLS_AVAILABLE == 1, "L_TLS_AVAILABLE is 1 on Windows");
#else
    TEST_ASSERT(L_TLS_AVAILABLE == 0, "L_TLS_AVAILABLE is 0 on non-Windows");
#endif
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    puts("Testing l_tls.h TLS client...\n");

    test_init_cleanup();
    test_invalid_handles();
    test_connect_without_init();
    test_availability_macro();

    l_test_print_summary(passed_count, test_count);
    if (passed_count != test_count) { puts("FAIL\n"); exit(1); }
    puts("PASS\n");
    return 0;
}
