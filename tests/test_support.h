#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H

#define TEST_DECLARE_COUNTERS() \
    static int test_count = 0; \
    static int passed_count = 0

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

#define TEST_CHECK(condition, test_name) do { \
    test_count++; \
    if (condition) { \
        passed_count++; \
    } else { \
        puts("  [FAIL] "); \
        puts(test_name); \
        puts("\n"); \
    } \
} while(0)

#define TEST_SECTION_PASS(name) do { \
    puts("  " name " tests: PASSED\n"); \
} while(0)

#define TEST_COUNT_ONLY(count) do { \
    test_count += (count); \
} while(0)

#define TEST_PASS_ONLY(count) do { \
    test_count += (count); \
    passed_count += (count); \
} while(0)

static inline void l_test_print_num(int n) {
    char buf[16];
    itoa(n, buf, sizeof(buf));
    puts(buf);
}

static inline void l_test_print_summary(int passed, int total) {
    puts("  ");
    l_test_print_num(passed);
    puts("/");
    l_test_print_num(total);
    puts(" tests passed\n");
}

#ifdef L_WITHSNPRINTF
static inline void l_test_build_bin_path(const char *name, char *buf, size_t size) {
#ifdef _WIN32
    l_snprintf(buf, size, "bin\\%s.exe", name);
#elif defined(__arm__)
    l_snprintf(buf, size, "bin/%s.armhf", name);
#elif defined(__aarch64__)
    l_snprintf(buf, size, "bin/%s.aarch64", name);
#else
    l_snprintf(buf, size, "bin/%s", name);
#endif
}
#endif

static inline int l_test_read_fd_all(L_FD fd, char *buf, int max) {
    int total = 0;
    while (total < max - 1) {
        ssize_t n = l_read(fd, buf + total, (size_t)(max - 1 - total));
        if (n <= 0)
            break;
        total += (int)n;
    }
    buf[total] = '\0';
    return total;
}

static inline int l_test_redirect_std_fd(L_FD target_fd, L_FD source_fd, L_FD *saved_fd) {
    *saved_fd = -1;
    if (source_fd == L_SPAWN_INHERIT || source_fd == target_fd)
        return 0;

    *saved_fd = l_dup(target_fd);
    if (*saved_fd < 0)
        return -1;

    if (l_dup2(source_fd, target_fd) != target_fd) {
        l_close(*saved_fd);
        *saved_fd = -1;
        return -1;
    }
    return 0;
}

static inline int l_test_restore_std_fd(L_FD target_fd, L_FD saved_fd) {
    int ok = 1;
    if (saved_fd < 0)
        return 0;
    if (l_dup2(saved_fd, target_fd) != target_fd)
        ok = 0;
    if (l_close(saved_fd) < 0)
        ok = 0;
    return ok ? 0 : -1;
}

static inline L_PID l_test_spawn_with_redirects(const char *path, char *const argv[], char *const envp[],
                                                L_FD stdin_fd, L_FD stdout_fd, L_FD stderr_fd,
                                                int *restore_ok) {
    L_FD saved_in = -1;
    L_FD saved_out = -1;
    L_FD saved_err = -1;
    int ok = 1;
    L_PID pid = -1;

    if (restore_ok)
        *restore_ok = 1;

    if (l_test_redirect_std_fd(L_STDIN, stdin_fd, &saved_in) < 0)
        goto done;
    if (l_test_redirect_std_fd(L_STDOUT, stdout_fd, &saved_out) < 0)
        goto done;
    if (l_test_redirect_std_fd(L_STDERR, stderr_fd, &saved_err) < 0)
        goto done;

    pid = l_spawn(path, argv, envp);

done:
    if (l_test_restore_std_fd(L_STDERR, saved_err) < 0)
        ok = 0;
    if (l_test_restore_std_fd(L_STDOUT, saved_out) < 0)
        ok = 0;
    if (l_test_restore_std_fd(L_STDIN, saved_in) < 0)
        ok = 0;

    if (restore_ok)
        *restore_ok = ok;
    return pid;
}

#ifdef L_WITHSOCKETS
static inline int l_test_open_loopback_listener(L_SOCKET *out_server, int *out_port) {
    int base_port = 20000 + (int)(l_getpid() % 20000);

    for (int i = 0; i < 128; i++) {
        int port = base_port + i;
        L_SOCKET server = l_socket_tcp();
        if (server < 0)
            return -1;
        if (l_socket_bind(server, port) == 0 && l_socket_listen(server, 2) == 0) {
            *out_server = server;
            *out_port = port;
            return 0;
        }
        l_socket_close(server);
    }

    return -1;
}

static inline int l_test_open_loopback_udp_socket(L_SOCKET *out_sock, int *out_port) {
    int base_port = 24000 + (int)(l_getpid() % 16000);

    for (int i = 0; i < 128; i++) {
        int port = base_port + i;
        L_SOCKET sock = l_socket_udp();
        if (sock < 0)
            return -1;
        if (l_socket_bind(sock, port) == 0) {
            *out_sock = sock;
            *out_port = port;
            return 0;
        }
        l_socket_close(sock);
    }

    return -1;
}
#endif

#endif
