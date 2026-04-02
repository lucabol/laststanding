/* test_support.h — shared test harness for all test shards.
 *
 * Include this AFTER all #define L_MAINFILE / #include directives.
 * Provides:
 *   - TEST_DECLARE_COUNTERS()        — declare per-TU counters
 *   - TEST_FUNCTION(name)            — print section header
 *   - TEST_ASSERT(cond, msg)         — assert; exits on failure
 *   - TEST_CHECK(cond, msg)          — assert; continues on failure (gfx/ui)
 *   - TEST_SECTION_PASS(name)        — print section pass
 *   - TEST_COUNT_ONLY(n) / TEST_PASS_ONLY(n)  — account for skipped tests
 *   - l_test_print_summary(p, t)     — print final pass/total line
 *   - l_test_build_bin_path(n, b, s) — build platform binary path
 *   - l_test_read_fd_all(fd, b, max) — read fd to completion
 *   - l_test_spawn_with_redirects()  — spawn process with fd redirection
 *   - l_test_open_loopback_listener(s, p)   — open TCP loopback listener
 *   - l_test_open_loopback_udp_socket(s, p) — open UDP loopback socket
 */

#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H

/* ── Counters ────────────────────────────────────────────────────────────── */

#define TEST_DECLARE_COUNTERS() \
    static int test_count = 0; \
    static int passed_count = 0

/* ── Assertion macros ────────────────────────────────────────────────────── */

#define TEST_FUNCTION(name) do { \
    puts("\n"); \
    puts("Testing " name "...\n"); \
} while(0)

#define TEST_ASSERT(condition, test_name) do { \
    test_count++; \
    if (condition) { \
        passed_count++; \
        puts("  [OK] " test_name "\n"); \
    } else { \
        puts("  [FAIL] " test_name " FAILED: " #condition "\n"); \
        exit(-1); \
    } \
} while(0)

/* TEST_CHECK: like TEST_ASSERT but continues on failure (used in gfx/ui). */
#define TEST_CHECK(condition, test_name) do { \
    test_count++; \
    if (condition) { \
        passed_count++; \
        puts("  [OK] " test_name "\n"); \
    } else { \
        puts("  [FAIL] " test_name " FAILED: " #condition "\n"); \
    } \
} while(0)

#define TEST_SECTION_PASS(name) do { \
    puts("  " name " tests: PASSED\n"); \
} while(0)

/* Count n tests as passed (used when skipping platform-specific tests). */
#define TEST_COUNT_ONLY(n) do { \
    test_count   += (n); \
    passed_count += (n); \
} while(0)

/* Alias — same semantics as TEST_COUNT_ONLY. */
#define TEST_PASS_ONLY(n) TEST_COUNT_ONLY(n)

/* ── Summary printer (no l_snprintf dependency) ──────────────────────────── */

static inline void l_test_print_summary(int passed, int total) {
    char pbuf[16], tbuf[16];
    l_itoa(passed, pbuf, 10);
    l_itoa(total,  tbuf, 10);
    puts("\n=== ");
    puts(pbuf);
    puts("/");
    puts(tbuf);
    puts(" tests passed ===\n");
}

/* ── Binary path helper ──────────────────────────────────────────────────── */

static inline void l_test_build_bin_path(const char *name, char *buf, size_t size) {
#ifdef _WIN32
    size_t n = l_strlen(name);
    if (n + 9 < size) {
        l_memcpy(buf, "bin\\", 4);
        l_memcpy(buf + 4, name, n);
        l_memcpy(buf + 4 + n, ".exe", 5);
    }
#elif defined(__arm__)
    size_t n = l_strlen(name);
    if (n + 10 < size) {
        l_memcpy(buf, "bin/", 4);
        l_memcpy(buf + 4, name, n);
        l_memcpy(buf + 4 + n, ".armhf", 7);
    }
#elif defined(__aarch64__)
    size_t n = l_strlen(name);
    if (n + 13 < size) {
        l_memcpy(buf, "bin/", 4);
        l_memcpy(buf + 4, name, n);
        l_memcpy(buf + 4 + n, ".aarch64", 9);
    }
#else
    size_t n = l_strlen(name);
    if (n + 5 < size) {
        l_memcpy(buf, "bin/", 4);
        l_memcpy(buf + 4, name, n + 1);
    }
#endif
}

/* ── FD helpers (not available on Windows freestanding) ─────────────────── */

#ifndef _WIN32

static inline int l_test_read_fd_all(L_FD fd, char *buf, int max) {
    int total = 0;
    while (total < max - 1) {
        ptrdiff_t n = l_read(fd, buf + total, (size_t)(max - 1 - total));
        if (n <= 0) break;
        total += (int)n;
    }
    buf[total] = '\0';
    return total;
}

static inline int l__test_redirect_fd(L_FD target, L_FD src, L_FD *saved) {
    *saved = -1;
    if (src < 0 || src == target) return 0;
    *saved = l_dup(target);
    if (*saved < 0) return -1;
    if (l_dup2(src, target) != target) { l_close(*saved); *saved = -1; return -1; }
    return 0;
}

static inline int l__test_restore_fd(L_FD target, L_FD saved) {
    if (saved < 0) return 0;
    int ok = (l_dup2(saved, target) == target) ? 0 : -1;
    if (l_close(saved) < 0) ok = -1;
    return ok;
}

static inline L_PID l_test_spawn_with_redirects(
        const char *path, char *const argv[], char *const envp[],
        L_FD stdin_fd, L_FD stdout_fd, L_FD stderr_fd, int *restore_ok) {
    L_FD si = -1, so = -1, se = -1;
    L_PID pid = -1;
    int ok = 1;
    if (restore_ok) *restore_ok = 1;
    if (l__test_redirect_fd(L_STDIN,  stdin_fd,  &si) < 0) goto done;
    if (l__test_redirect_fd(L_STDOUT, stdout_fd, &so) < 0) goto done;
    if (l__test_redirect_fd(L_STDERR, stderr_fd, &se) < 0) goto done;
    pid = l_spawn(path, argv, envp);
done:
    if (l__test_restore_fd(L_STDERR, se) < 0) ok = 0;
    if (l__test_restore_fd(L_STDOUT, so) < 0) ok = 0;
    if (l__test_restore_fd(L_STDIN,  si) < 0) ok = 0;
    if (restore_ok) *restore_ok = ok;
    return pid;
}

#endif /* !_WIN32 */

/* ── Loopback socket helpers (requires L_WITHSOCKETS) ───────────────────── */

#ifdef L_WITHSOCKETS

static inline int l_test_open_loopback_listener(L_SOCKET *out_server, int *out_port) {
    int base = 20000 + (int)(l_getpid() % 20000);
    for (int i = 0; i < 128; i++) {
        int port = base + i;
        L_SOCKET s = l_socket_tcp();
        if (s < 0) return -1;
        if (l_socket_bind(s, port) == 0 && l_socket_listen(s, 2) == 0) {
            *out_server = s;
            *out_port   = port;
            return 0;
        }
        l_socket_close(s);
    }
    return -1;
}

static inline int l_test_open_loopback_udp_socket(L_SOCKET *out_sock, int *out_port) {
    int base = 24000 + (int)(l_getpid() % 16000);
    for (int i = 0; i < 128; i++) {
        int port = base + i;
        L_SOCKET s = l_socket_udp();
        if (s < 0) return -1;
        if (l_socket_bind(s, port) == 0) {
            *out_sock  = s;
            *out_port  = port;
            return 0;
        }
        l_socket_close(s);
    }
    return -1;
}

#endif /* L_WITHSOCKETS */

#endif /* TEST_SUPPORT_H */
