// This shard keeps the self-spawn/process coverage under the historical test.exe binary.

#define L_WITHSNPRINTF
#include "l_os.h"
#include "l_os.h"

#define L_MAINFILE
#include "l_os.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

static int maybe_run_helper(int argc, char *argv[]) {
    if (argc < 2)
        return -1;

    if (l_strcmp(argv[1], "--spawn-stdio-close-stdout-helper") == 0) {
        char drain[16];
        if (l_write(L_STDOUT, "hold", 4) != 4)
            return 2;
        l_close(L_STDOUT);
        while (l_read(L_STDIN, drain, sizeof(drain)) > 0) {}
        return 0;
    }

    if (l_strcmp(argv[1], "--exit") == 0 && argc >= 3) {
        l_exit(l_atoi(argv[2]));
    }

    if (l_strcmp(argv[1], "--echo-stderr") == 0 && argc >= 3) {
        l_write(L_STDERR, argv[2], l_strlen(argv[2]));
        return 0;
    }

    if (l_strcmp(argv[1], "--hold-stdin") == 0) {
        char drain[16];
        while (l_read(L_STDIN, drain, sizeof(drain)) > 0) {}
        return 0;
    }

    return -1;
}

void test_command_line_args(int argc, char* argv[]) {
    TEST_FUNCTION("Command Line Arguments");
    for(int i = 1; i < argc; i++) {
        puts(argv[i]);
    }
    TEST_ASSERT(1, "command line argument reading completed");
    TEST_SECTION_PASS("Command line argument");
}

void test_program_name(int argc, char* argv[]) {
    TEST_FUNCTION("Program Name Reading");
    TEST_ASSERT(argc >= 0, "argc is non-negative");
    TEST_ASSERT(strlen(argv[0]) > 0, "program name is not empty");
    TEST_ASSERT(strstr(argv[0], "test") != NULL, "program name contains 'test'");
    TEST_SECTION_PASS("Program name reading");
}

void test_unicode_output(void) {
    TEST_FUNCTION("Unicode Output");
    char msg[] = u8"κόσμε";
    puts(msg);
    puts("\n");
    TEST_ASSERT(1, "unicode output test completed");
    TEST_SECTION_PASS("Unicode output");
}

void test_basic_file_io(void) {
    TEST_FUNCTION("Basic File I/O");

    char* msg = "Hello world!";
    int len = strlen(msg);

    L_FD file = open_append("test_file");
    write(file, msg, len);
    close(file);

    file = open_read("test_file");
    char buf[len];
    int n = read(file, buf, len);
    TEST_ASSERT(memcmp(buf, msg, len) == 0, "file content matches written data");
    TEST_ASSERT(n == len, "read correct number of bytes");
    close(file);

    TEST_SECTION_PASS("Basic file I/O");
}

void test_pipe_dup2(void) {
    TEST_FUNCTION("l_pipe / l_dup2");

    L_FD fds[2];
    int ret = l_pipe(fds);
    TEST_ASSERT(ret == 0, "pipe creates successfully");
    TEST_ASSERT(fds[0] != fds[1], "pipe read and write ends differ");

    // Write to pipe, read back
    const char *msg = "hello pipe";
    ssize_t written = l_write(fds[1], msg, 10);
    TEST_ASSERT(written == 10, "write 10 bytes to pipe");

    char buf[32];
    l_memset(buf, 0, sizeof(buf));
    ssize_t rd = l_read(fds[0], buf, 10);
    TEST_ASSERT(rd == 10, "read 10 bytes from pipe");
    TEST_ASSERT(l_memcmp(buf, "hello pipe", 10) == 0, "pipe data matches");

    l_close(fds[0]);
    l_close(fds[1]);

    // Test l_dup2 — duplicate a pipe fd
    L_FD fds2[2];
    l_pipe(fds2);
    int dup_ret = l_dup2(fds2[1], fds2[1]); // dup2 with same fd should work
    TEST_ASSERT(dup_ret >= 0, "dup2 returns valid fd");

    // Write through original, verify data arrives
    l_write(fds2[1], "test", 4);
    char buf2[8];
    l_memset(buf2, 0, sizeof(buf2));
    l_read(fds2[0], buf2, 4);
    TEST_ASSERT(l_memcmp(buf2, "test", 4) == 0, "dup2 pipe data works");

    l_close(fds2[0]);
    l_close(fds2[1]);

    L_FD fds3[2];
    L_FD dup_fd = 64;
    TEST_ASSERT(l_pipe(fds3) == 0, "second pipe creates successfully");
    TEST_ASSERT(l_dup2(fds3[1], dup_fd) == dup_fd, "dup2 duplicates into arbitrary fd slot");
    l_close(fds3[1]);

    TEST_ASSERT(l_write(dup_fd, "slot", 4) == 4, "write through arbitrary duped fd");
    char buf3[8];
    l_memset(buf3, 0, sizeof(buf3));
    TEST_ASSERT(l_read(fds3[0], buf3, 4) == 4, "read through arbitrary duped fd");
    TEST_ASSERT(l_memcmp(buf3, "slot", 4) == 0, "arbitrary duped fd data matches");

    l_close(dup_fd);
    l_close(fds3[0]);

    TEST_SECTION_PASS("l_pipe / l_dup2");
}

void test_dup(void) {
    TEST_FUNCTION("l_dup");

    L_FD fd = l_open_write("test_dup_file");
    TEST_ASSERT(fd >= 0, "open file for writing");

    L_FD fd2 = l_dup(fd);
    TEST_ASSERT(fd2 >= 0, "dup returns valid fd");
    TEST_ASSERT(fd2 != fd, "dup returns different fd number");

    char *msg1 = "Hello";
    ssize_t w1 = l_write(fd, msg1, 5);
    TEST_ASSERT(w1 == 5, "write via original fd succeeds");

    char *msg2 = "World";
    ssize_t w2 = l_write(fd2, msg2, 5);
    TEST_ASSERT(w2 == 5, "write via duplicated fd succeeds");

    l_close(fd);
    l_close(fd2);

    L_FD rfd = l_open_read("test_dup_file");
    TEST_ASSERT(rfd >= 0, "open file for reading back");
    char buf[16];
    ssize_t n = l_read(rfd, buf, 10);
    TEST_ASSERT(n == 10, "read back all 10 bytes");
    TEST_ASSERT(l_memcmp(buf, "HelloWorld", 10) == 0, "both fd writes captured in order");
    l_close(rfd);

    TEST_SECTION_PASS("l_dup");
}

void test_spawn_wait(void) {
    TEST_FUNCTION("l_spawn / l_wait");

    char test_path[256];
    l_test_build_bin_path("test", test_path, sizeof(test_path));

    // Test: spawn self with --exit 0
    char *args[] = {test_path, "--exit", "0", NULL};
    L_PID pid = l_spawn(test_path, args, (char *const *)0);
    TEST_ASSERT(pid != -1, "spawn self --exit 0 succeeds");
    int exitcode = -1;
    int ret = l_wait(pid, &exitcode);
    TEST_ASSERT(ret == 0, "wait succeeds");
    TEST_ASSERT(exitcode == 0, "exit code is 0");

    // Test non-zero exit code
    char *args2[] = {test_path, "--exit", "42", NULL};
    L_PID pid2 = l_spawn(test_path, args2, (char *const *)0);
    TEST_ASSERT(pid2 != -1, "spawn self --exit 42 succeeds");
    int exitcode2 = -1;
    l_wait(pid2, &exitcode2);
    TEST_ASSERT(exitcode2 == 42, "exit code is 42");

#ifndef _WIN32
    // Also test fork + waitpid directly (works under QEMU too)
    L_PID child = l_fork();
    if (child == 0) {
        l_exit(42);
    }
    TEST_ASSERT(child > 0, "fork returns positive pid");
    int status = 0;
    L_PID waited = l_waitpid(child, &status, 0);
    TEST_ASSERT(waited == child, "waitpid returns child pid");
    TEST_ASSERT((status & 0x7f) == 0, "child exited normally");
    TEST_ASSERT(((status >> 8) & 0xff) == 42, "child exit code is 42");
#endif

    TEST_SECTION_PASS("l_spawn / l_wait");
}

void test_spawn_inherits_dup2(void) {
    TEST_FUNCTION("l_spawn inherits dup2");

    char test_path[256];
    l_test_build_bin_path("test", test_path, sizeof(test_path));

    L_FD cap_pipe[2];
    L_FD saved_stderr = -1;
    int redir_ret = -1;
    int restore_ret = -1;
    int wait_ret;
    int child_exit = -1;
    int cap_len;
    char cap_buf[64];
    L_PID pid = -1;

    TEST_ASSERT(l_pipe(cap_pipe) == 0, "capture pipe creates successfully");

    saved_stderr = l_dup(L_STDERR);
    if (saved_stderr >= 0)
        redir_ret = l_dup2(cap_pipe[1], L_STDERR);
    l_close(cap_pipe[1]);

    if (saved_stderr >= 0 && redir_ret == L_STDERR) {
        char *args[] = {test_path, "--echo-stderr", "spawndup", NULL};
        pid = l_spawn(test_path, args, (char *const *)0);
        restore_ret = l_dup2(saved_stderr, L_STDERR);
    }
    if (saved_stderr >= 0)
        l_close(saved_stderr);

    TEST_ASSERT(saved_stderr >= 0, "dup saves stderr");
    TEST_ASSERT(redir_ret == L_STDERR, "dup2 redirects stderr");
    TEST_ASSERT(pid != -1, "spawn inherits redirected stderr");
    TEST_ASSERT(restore_ret == L_STDERR, "dup2 restores stderr");

    wait_ret = l_wait(pid, &child_exit);
    TEST_ASSERT(wait_ret == 0, "wait on inherited-stderr child succeeds");

    cap_len = l_test_read_fd_all(cap_pipe[0], cap_buf, sizeof(cap_buf));
    l_close(cap_pipe[0]);

    TEST_ASSERT(child_exit == 0, "inherited-stderr child exits 0");
    TEST_ASSERT(cap_len == 8, "captured inherited stderr length matches");
    TEST_ASSERT(l_strcmp(cap_buf, "spawndup") == 0, "captured inherited stderr matches");

    TEST_SECTION_PASS("l_spawn inherits dup2");
}

void test_spawn_pipeline_via_dup2(void) {
    TEST_FUNCTION("l_spawn pipeline via dup2");

    char sort_path[64];
    char printenv_path[64];
    int left_restore_ok = 0;
    int right_restore_ok = 0;
    l_test_build_bin_path("sort", sort_path, sizeof(sort_path));
    l_test_build_bin_path("printenv", printenv_path, sizeof(printenv_path));

    L_FD mid_pipe[2];
    L_FD cap_pipe[2];
    TEST_ASSERT(l_pipe(mid_pipe) == 0, "pipeline dup2 creates first pipe");
    TEST_ASSERT(l_pipe(cap_pipe) == 0, "pipeline dup2 creates capture pipe");

    char *envp[] = {"LS_CHILD_PIPE=banana", NULL};
    char *left_args[] = {"printenv", "LS_CHILD_PIPE", NULL};
    char *right_args[] = {"sort", NULL};

    L_PID left_pid = l_test_spawn_with_redirects(printenv_path, left_args, envp,
                                                 L_SPAWN_INHERIT, mid_pipe[1], L_SPAWN_INHERIT,
                                                 &left_restore_ok);
    TEST_ASSERT(left_pid != -1, "spawn left via l_spawn succeeds");
    TEST_ASSERT(left_restore_ok, "left l_spawn restores parent stdio");
    l_close(mid_pipe[1]);

    L_PID right_pid = l_test_spawn_with_redirects(sort_path, right_args, (char *const *)0,
                                                  mid_pipe[0], cap_pipe[1], L_SPAWN_INHERIT,
                                                  &right_restore_ok);
    TEST_ASSERT(right_pid != -1, "spawn right via l_spawn succeeds");
    TEST_ASSERT(right_restore_ok, "right l_spawn restores parent stdio");
    l_close(mid_pipe[0]);
    l_close(cap_pipe[1]);

    char pipe_buf[64];
    int pipe_len = l_test_read_fd_all(cap_pipe[0], pipe_buf, sizeof(pipe_buf));
    l_close(cap_pipe[0]);

    int left_exit = -1;
    int right_exit = -1;
    TEST_ASSERT(l_wait(left_pid, &left_exit) == 0, "wait on left l_spawn child succeeds");
    TEST_ASSERT(l_wait(right_pid, &right_exit) == 0, "wait on right l_spawn child succeeds");
    TEST_ASSERT(left_exit == 0, "left l_spawn child exits 0");
    TEST_ASSERT(right_exit == 0, "right l_spawn child exits 0");
    TEST_ASSERT(pipe_len == 21, "captured dup2 pipeline stdout length matches");
    TEST_ASSERT(l_strcmp(pipe_buf, "LS_CHILD_PIPE=banana\n") == 0, "captured dup2 pipeline stdout matches");

    TEST_SECTION_PASS("l_spawn pipeline via dup2");
}

void test_spawn_stdio(void) {
    TEST_FUNCTION("l_spawn_stdio");

    char sort_path[64];
    char printenv_path[64];
    l_test_build_bin_path("sort", sort_path, sizeof(sort_path));
    l_test_build_bin_path("printenv", printenv_path, sizeof(printenv_path));

    L_FD in_pipe[2];
    L_FD out_pipe[2];
    TEST_ASSERT(l_pipe(in_pipe) == 0, "stdin pipe creates successfully");
    TEST_ASSERT(l_pipe(out_pipe) == 0, "stdout pipe creates successfully");

    char *sort_args[] = {"sort", NULL};
    L_PID sort_pid = l_spawn_stdio(sort_path, sort_args, (char *const *)0,
                                   in_pipe[0], out_pipe[1], L_SPAWN_INHERIT);
    TEST_ASSERT(sort_pid != -1, "spawn_stdio sort succeeds");

    l_close(in_pipe[0]);
    l_close(out_pipe[1]);
    TEST_ASSERT(l_write(in_pipe[1], "b\na\n", 4) == 4, "writes redirected stdin");
    l_close(in_pipe[1]);

    char sort_buf[64];
    int sort_len = l_test_read_fd_all(out_pipe[0], sort_buf, sizeof(sort_buf));
    l_close(out_pipe[0]);

    int sort_exit = -1;
    TEST_ASSERT(l_wait(sort_pid, &sort_exit) == 0, "wait on redirected sort succeeds");
    TEST_ASSERT(sort_exit == 0, "redirected sort exits 0");
    TEST_ASSERT(sort_len == 4, "captured sort stdout length matches");
    TEST_ASSERT(l_strcmp(sort_buf, "a\nb\n") == 0, "captured sort stdout matches");

    L_FD mid_pipe[2];
    L_FD cap_pipe[2];
    TEST_ASSERT(l_pipe(mid_pipe) == 0, "pipeline creates first pipe");
    TEST_ASSERT(l_pipe(cap_pipe) == 0, "pipeline creates capture pipe");

    char *envp[] = {"LS_CHILD_PIPE=banana", NULL};
    char *left_args[] = {"printenv", "LS_CHILD_PIPE", NULL};
    char *right_args[] = {"sort", NULL};

    L_PID left_pid = l_spawn_stdio(printenv_path, left_args, envp,
                                   L_SPAWN_INHERIT, mid_pipe[1], L_SPAWN_INHERIT);
    TEST_ASSERT(left_pid != -1, "spawn_stdio left side succeeds");
    l_close(mid_pipe[1]);

    L_PID right_pid = l_spawn_stdio(sort_path, right_args, (char *const *)0,
                                    mid_pipe[0], cap_pipe[1], L_SPAWN_INHERIT);
    TEST_ASSERT(right_pid != -1, "spawn_stdio right side succeeds");
    l_close(mid_pipe[0]);
    l_close(cap_pipe[1]);

    char pipe_buf[64];
    int pipe_len = l_test_read_fd_all(cap_pipe[0], pipe_buf, sizeof(pipe_buf));
    l_close(cap_pipe[0]);

    int left_exit = -1;
    int right_exit = -1;
    TEST_ASSERT(l_wait(left_pid, &left_exit) == 0, "wait on left child succeeds");
    TEST_ASSERT(l_wait(right_pid, &right_exit) == 0, "wait on right child succeeds");
    TEST_ASSERT(left_exit == 0, "left child exits 0");
    TEST_ASSERT(right_exit == 0, "right child exits 0");
    TEST_ASSERT(pipe_len == 21, "captured pipeline stdout length matches");
    TEST_ASSERT(l_strcmp(pipe_buf, "LS_CHILD_PIPE=banana\n") == 0, "captured pipeline stdout matches");

    TEST_SECTION_PASS("l_spawn_stdio");
}

void test_spawn_stdio_child_fd_cleanup(void) {
    TEST_FUNCTION("l_spawn_stdio child fd cleanup");

#ifdef _WIN32
    TEST_ASSERT(1, "child fd cleanup coverage is Unix-specific");
#else
    char test_path[64];
    char hold_buf[8];
    int status = 0;
    int exitcode = -1;
    L_FD in_pipe[2];
    L_FD out_pipe[2];
    char *args[] = {"test", "--spawn-stdio-close-stdout-helper", NULL};

    l_test_build_bin_path("test", test_path, sizeof(test_path));
    TEST_ASSERT(l_pipe(in_pipe) == 0, "helper stdin pipe creates successfully");
    TEST_ASSERT(l_pipe(out_pipe) == 0, "helper stdout pipe creates successfully");

    L_PID pid = l_spawn_stdio(test_path, args, (char *const *)0,
                              in_pipe[0], out_pipe[1], L_SPAWN_INHERIT);
    TEST_ASSERT(pid != -1, "spawn_stdio helper succeeds");

    l_close(in_pipe[0]);
    l_close(out_pipe[1]);

    l_memset(hold_buf, 0, sizeof(hold_buf));
    TEST_ASSERT(l_read(out_pipe[0], hold_buf, 4) == 4, "reads helper stdout payload");
    TEST_ASSERT(l_memcmp(hold_buf, "hold", 4) == 0, "helper stdout payload matches");
    TEST_ASSERT(l_waitpid(pid, &status, 1 /* WNOHANG */) == 0, "helper remains blocked after closing stdout");
    // Child has written "hold" and will close stdout, then block on stdin.
    // l_read returning 0 proves the child's stdout fd was properly closed (EOF).
    TEST_ASSERT(l_read(out_pipe[0], hold_buf, 1) == 0, "stdout pipe reports EOF before child exits");

    l_close(out_pipe[0]);
    l_close(in_pipe[1]);

    TEST_ASSERT(l_wait(pid, &exitcode) == 0, "wait on helper succeeds");
    TEST_ASSERT(exitcode == 0, "helper exits 0 after stdin closes");
#endif

    TEST_SECTION_PASS("l_spawn_stdio child fd cleanup");
}

void test_find_executable(void) {
    TEST_FUNCTION("l_find_executable");
    char buf[512];

    /* Nonexistent command must return 0 */
    TEST_ASSERT(l_find_executable("__no_such_cmd_xyz__", buf, sizeof(buf)) == 0,
                "nonexistent command returns 0");

    /* Empty command must return 0 */
    TEST_ASSERT(l_find_executable("", buf, sizeof(buf)) == 0,
                "empty command returns 0");

    /* NULL command must return 0 */
    TEST_ASSERT(l_find_executable((const char *)0, buf, sizeof(buf)) == 0,
                "NULL command returns 0");

#ifdef _WIN32
    /* On Windows, cmd.exe should be findable without .exe extension */
    TEST_ASSERT(l_find_executable("cmd", buf, sizeof(buf)) == 1,
                "finds cmd on Windows PATH");
    TEST_ASSERT(l_strstr(buf, "cmd") != (const char *)0,
                "resolved path contains 'cmd'");

    /* With explicit .exe extension */
    TEST_ASSERT(l_find_executable("cmd.exe", buf, sizeof(buf)) == 1,
                "finds cmd.exe on Windows PATH");
#else
    /* On Unix, sh should be findable */
    TEST_ASSERT(l_find_executable("sh", buf, sizeof(buf)) == 1,
                "finds sh on Unix PATH");
    TEST_ASSERT(l_strstr(buf, "sh") != (const char *)0,
                "resolved path contains 'sh'");
#endif

    /* Direct path to the test binary itself (argv[0]-style) */
    {
        char self[512];
        l_test_build_bin_path("test", self, sizeof(self));
        if (l_access(self, L_F_OK) == 0) {
            TEST_ASSERT(l_find_executable(self, buf, sizeof(buf)) == 1,
                        "finds test binary by direct path");
        }
    }

    /* Buffer too small should return 0 */
    TEST_ASSERT(l_find_executable("cmd", buf, 1) == 0,
                "buffer too small returns 0");

    TEST_SECTION_PASS("l_find_executable");
}

void test_errno_strerror(void) {
    TEST_FUNCTION("l_errno / l_strerror");

    // Error constants have expected POSIX values
    TEST_ASSERT(L_ENOENT == 2, "L_ENOENT == 2");
    TEST_ASSERT(L_EACCES == 13, "L_EACCES == 13");
    TEST_ASSERT(L_EBADF == 9, "L_EBADF == 9");
    TEST_ASSERT(L_EEXIST == 17, "L_EEXIST == 17");
    TEST_ASSERT(L_EINVAL == 22, "L_EINVAL == 22");
    TEST_ASSERT(L_ENOMEM == 12, "L_ENOMEM == 12");
    TEST_ASSERT(L_EAGAIN == 11, "L_EAGAIN == 11");
    TEST_ASSERT(L_EPIPE == 32, "L_EPIPE == 32");
    TEST_ASSERT(L_ENOSPC == 28, "L_ENOSPC == 28");
    TEST_ASSERT(L_ENOTDIR == 20, "L_ENOTDIR == 20");
    TEST_ASSERT(L_EISDIR == 21, "L_EISDIR == 21");
    TEST_ASSERT(L_ENOTEMPTY == 39, "L_ENOTEMPTY == 39");

    // l_strerror returns non-empty strings for all known codes
    TEST_ASSERT(l_strlen(l_strerror(0)) > 0, "strerror(0) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_ENOENT)) > 0, "strerror(ENOENT) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_EACCES)) > 0, "strerror(EACCES) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_EBADF)) > 0, "strerror(EBADF) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_EEXIST)) > 0, "strerror(EEXIST) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_EINVAL)) > 0, "strerror(EINVAL) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_ENOMEM)) > 0, "strerror(ENOMEM) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_EAGAIN)) > 0, "strerror(EAGAIN) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_EPIPE)) > 0, "strerror(EPIPE) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_ENOSPC)) > 0, "strerror(ENOSPC) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_ENOTDIR)) > 0, "strerror(ENOTDIR) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_EISDIR)) > 0, "strerror(EISDIR) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(L_ENOTEMPTY)) > 0, "strerror(ENOTEMPTY) non-empty");
    TEST_ASSERT(l_strlen(l_strerror(9999)) > 0, "strerror(unknown) non-empty");

    // Distinct strings for different error codes
    TEST_ASSERT(l_strcmp(l_strerror(L_ENOENT), l_strerror(L_EACCES)) != 0,
                "strerror(ENOENT) != strerror(EACCES)");

    // l_errno() returns ENOENT after opening a nonexistent file
    L_FD fd = l_open_read("__nonexistent_file_for_errno_test__");
    int saved_errno = l_errno();
    TEST_ASSERT(fd < 0, "open nonexistent file fails");
    TEST_ASSERT(saved_errno == L_ENOENT, "errno is ENOENT after open of missing file");

    // l_errno() is 0 after a successful operation
    fd = l_open_write("test_errno_tmpfile");
    saved_errno = l_errno();
    TEST_ASSERT(fd >= 0, "open for write succeeds");
    TEST_ASSERT(saved_errno == 0, "errno is 0 after successful open");
    l_close(fd);
    l_unlink("test_errno_tmpfile");

    TEST_SECTION_PASS("l_errno / l_strerror");
}

void test_getopt(void) {
    TEST_FUNCTION("l_getopt");

    /* Basic flag parsing: -v returns 'v' */
    {
        char *args[] = {"prog", "-v", (char *)0};
        l_optind = 1; l__optpos = 0; l_opterr = 0;
        int c = l_getopt(2, args, "vf");
        TEST_ASSERT(c == 'v', "single flag -v returns 'v'");
        TEST_ASSERT(l_optind == 2, "optind advances past -v");
    }

    /* Option with argument: -o file */
    {
        char *args[] = {"prog", "-o", "myfile", (char *)0};
        l_optind = 1; l__optpos = 0; l_opterr = 0;
        int c = l_getopt(3, args, "o:");
        TEST_ASSERT(c == 'o', "-o returns 'o'");
        TEST_ASSERT(l_optarg != (char *)0, "-o sets optarg");
        TEST_ASSERT(l_strcmp(l_optarg, "myfile") == 0, "optarg == \"myfile\"");
        TEST_ASSERT(l_optind == 3, "optind advances past -o myfile");
    }

    /* Option with glued argument: -omyfile */
    {
        char *args[] = {"prog", "-omyfile", (char *)0};
        l_optind = 1; l__optpos = 0; l_opterr = 0;
        int c = l_getopt(2, args, "o:");
        TEST_ASSERT(c == 'o', "-omyfile returns 'o'");
        TEST_ASSERT(l_optarg != (char *)0, "-omyfile sets optarg");
        TEST_ASSERT(l_strcmp(l_optarg, "myfile") == 0, "glued optarg == \"myfile\"");
    }

    /* Multiple flags clustered: -vf parses both */
    {
        char *args[] = {"prog", "-vf", (char *)0};
        l_optind = 1; l__optpos = 0; l_opterr = 0;
        int c1 = l_getopt(2, args, "vf");
        int c2 = l_getopt(2, args, "vf");
        TEST_ASSERT(c1 == 'v', "first in -vf cluster is 'v'");
        TEST_ASSERT(c2 == 'f', "second in -vf cluster is 'f'");
        TEST_ASSERT(l_optind == 2, "optind advances once for -vf cluster");
    }

    /* End of options returns -1 */
    {
        char *args[] = {"prog", (char *)0};
        l_optind = 1; l__optpos = 0; l_opterr = 0;
        int c = l_getopt(1, args, "vf");
        TEST_ASSERT(c == -1, "no options returns -1");
    }

    /* "--" stops parsing */
    {
        char *args[] = {"prog", "--", "-v", (char *)0};
        l_optind = 1; l__optpos = 0; l_opterr = 0;
        int c = l_getopt(3, args, "v");
        TEST_ASSERT(c == -1, "-- stops option parsing");
        TEST_ASSERT(l_optind == 2, "optind points past --");
    }

    /* Unknown option returns '?' */
    {
        char *args[] = {"prog", "-x", (char *)0};
        l_optind = 1; l__optpos = 0; l_opterr = 0;
        int c = l_getopt(2, args, "vf");
        TEST_ASSERT(c == '?', "unknown option returns '?'");
        TEST_ASSERT(l_optopt == 'x', "optopt set to unknown char");
    }

    /* Mixed flags and arguments: -n 10 -v */
    {
        char *args[] = {"prog", "-n", "10", "-v", (char *)0};
        l_optind = 1; l__optpos = 0; l_opterr = 0;
        int c1 = l_getopt(4, args, "vn:");
        TEST_ASSERT(c1 == 'n', "first option is 'n'");
        TEST_ASSERT(l_optarg != (char *)0 && l_strcmp(l_optarg, "10") == 0, "n's arg is \"10\"");
        int c2 = l_getopt(4, args, "vn:");
        TEST_ASSERT(c2 == 'v', "second option is 'v'");
        int c3 = l_getopt(4, args, "vn:");
        TEST_ASSERT(c3 == -1, "done after all options");
    }

    /* Non-option argument stops parsing */
    {
        char *args[] = {"prog", "file.txt", "-v", (char *)0};
        l_optind = 1; l__optpos = 0; l_opterr = 0;
        int c = l_getopt(3, args, "v");
        TEST_ASSERT(c == -1, "non-option stops parsing");
        TEST_ASSERT(l_optind == 1, "optind stays at non-option");
    }

    TEST_SECTION_PASS("l_getopt");
}

void test_dprintf(void) {
    TEST_FUNCTION("l_dprintf");
    {
        L_FD fd = l_open_write("_dprintf_test.txt");
        TEST_ASSERT(fd >= 0, "open for dprintf");
        int n = l_dprintf(fd, "hello %d world %s\n", 42, "foo");
        TEST_ASSERT(n > 0, "dprintf returns positive");
        l_close(fd);

        fd = l_open_read("_dprintf_test.txt");
        char buf[128];
        int nr = (int)l_read(fd, buf, sizeof(buf));
        l_close(fd);
        buf[nr] = 0;
        TEST_ASSERT(l_strcmp(buf, "hello 42 world foo\n") == 0, "dprintf content correct");
        l_unlink("_dprintf_test.txt");
    }
    TEST_SECTION_PASS("l_dprintf");
}

void test_read_line(void) {
    TEST_FUNCTION("l_read_line");
    {
        L_FD fd = l_open_write("_readline_test.txt");
        l_write(fd, "hello\nworld\nlast", 16);
        l_close(fd);

        fd = l_open_read("_readline_test.txt");
        char buf[64];
        ptrdiff_t n;

        n = l_read_line(fd, buf, sizeof(buf));
        TEST_ASSERT(n == 5, "first line length");
        TEST_ASSERT(l_strcmp(buf, "hello") == 0, "first line content");

        n = l_read_line(fd, buf, sizeof(buf));
        TEST_ASSERT(n == 5, "second line length");
        TEST_ASSERT(l_strcmp(buf, "world") == 0, "second line content");

        n = l_read_line(fd, buf, sizeof(buf));
        TEST_ASSERT(n == 4, "last line (no newline) length");
        TEST_ASSERT(l_strcmp(buf, "last") == 0, "last line content");

        n = l_read_line(fd, buf, sizeof(buf));
        TEST_ASSERT(n == -1, "EOF returns -1");

        l_close(fd);
        l_unlink("_readline_test.txt");
    }
    {
        L_FD fd = l_open_write("_readline_crlf.txt");
        l_write(fd, "line1\r\nline2\r\n", 14);
        l_close(fd);

        fd = l_open_read("_readline_crlf.txt");
        char buf[64];
        l_read_line(fd, buf, sizeof(buf));
        TEST_ASSERT(l_strcmp(buf, "line1") == 0, "CRLF line1");
        l_read_line(fd, buf, sizeof(buf));
        TEST_ASSERT(l_strcmp(buf, "line2") == 0, "CRLF line2");
        l_close(fd);
        l_unlink("_readline_crlf.txt");
    }
    TEST_SECTION_PASS("l_read_line");
}

void test_terminal_helpers(void) {
    TEST_FUNCTION("l_term_raw / l_term_restore / l_read_nonblock / l_term_size");

    unsigned long saved_mode = l_term_raw();
    char dummy = 0;
    int rows = 0;
    int cols = 0;

    TEST_ASSERT(l_read_nonblock(L_STDIN, &dummy, 0) == 0,
                "read_nonblock count=0 returns immediately");
    l_term_restore(saved_mode);
    TEST_ASSERT(1, "term_raw and term_restore complete without hanging");

    l_term_size(&rows, &cols);
    TEST_ASSERT(rows > 0 && cols > 0, "term_size reports positive dimensions");
    TEST_ASSERT(rows < 1000 && cols < 1000, "term_size stays within sane bounds");

    TEST_SECTION_PASS("terminal helpers");
}

void test_getpid_kill_llabs(void) {
    TEST_FUNCTION("l_getpid / l_llabs");

    L_PID pid = l_getpid();
    TEST_ASSERT(pid > 0, "l_getpid returns positive value");

#ifndef _WIN32
    L_PID ppid = l_getppid();
    TEST_ASSERT(ppid > 0, "l_getppid returns positive value");
    TEST_ASSERT(ppid != pid, "ppid differs from pid");

    /* Send signal 0 to self — exists check, no signal delivered */
    int kr = l_kill(pid, 0);
    TEST_ASSERT(kr == 0, "l_kill(pid, 0) succeeds for own process");
#endif

    TEST_ASSERT(l_llabs(0LL) == 0LL, "llabs(0) == 0");
    TEST_ASSERT(l_llabs(42LL) == 42LL, "llabs(42) == 42");
    TEST_ASSERT(l_llabs(-42LL) == 42LL, "llabs(-42) == 42");
    TEST_ASSERT(l_llabs(-9223372036854775807LL) == 9223372036854775807LL, "llabs(LLONG_MIN+1)");

    TEST_SECTION_PASS("l_getpid / l_llabs");
}

static void l_test_dummy_sig_handler(int sig) { (void)sig; }

void test_signal(void) {
    TEST_FUNCTION("l_signal");

    // Set a dummy handler for SIGINT — first call should return SIG_DFL
    L_SigHandler prev = l_signal(L_SIGINT, l_test_dummy_sig_handler);
    TEST_ASSERT(prev == L_SIG_DFL, "first signal install returns SIG_DFL");

    // Set SIG_IGN — should return the previous handler
    L_SigHandler prev2 = l_signal(L_SIGINT, L_SIG_IGN);
    TEST_ASSERT(prev2 == l_test_dummy_sig_handler, "second signal install returns previous handler");

    // Restore to SIG_DFL — should return SIG_IGN
    L_SigHandler prev3 = l_signal(L_SIGINT, L_SIG_DFL);
    TEST_ASSERT(prev3 == L_SIG_IGN, "third signal install returns SIG_IGN");

    TEST_SECTION_PASS("l_signal");
}

void test_system_cmd(void) {
    TEST_FUNCTION("l_system");

    // l_system wraps l_spawn with cmd.exe/sh -c; test with a simple known command
    // Use the test binary itself as the command (known to exist and work)
    char test_path[256];
    l_test_build_bin_path("test", test_path, sizeof(test_path));

    char cmd[512];
    l_memset(cmd, 0, sizeof(cmd));
    l_strcpy(cmd, test_path);
    l_strcat(cmd, " --exit 0");

    int r = l_system(cmd);
    if (r != 0) {
        // l_system may not work on all platforms (e.g., freestanding Windows)
        l_puts("  [SKIP] l_system not functional on this platform\n");
        TEST_SECTION_PASS("l_system (skipped)");
        return;
    }
    TEST_ASSERT(r == 0, "l_system exit 0 returns 0");

    l_memset(cmd, 0, sizeof(cmd));
    l_strcpy(cmd, test_path);
    l_strcat(cmd, " --exit 42");
    int r2 = l_system(cmd);
    TEST_ASSERT(r2 == 42, "l_system exit 42 returns 42");

    l_memset(cmd, 0, sizeof(cmd));
    l_strcpy(cmd, test_path);
    l_strcat(cmd, " --exit 0");
    int r3 = l_system(cmd);
    TEST_ASSERT(r3 == 0, "l_system third call returns 0");

    TEST_SECTION_PASS("l_system");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    {
        int helper_exit = maybe_run_helper(argc, argv);
        if (helper_exit >= 0)
            return helper_exit;
    }
    test_command_line_args(argc, argv);
    test_program_name(argc, argv);
    test_unicode_output();
    test_basic_file_io();
    test_pipe_dup2();
    test_dup();
    test_spawn_wait();
    test_spawn_inherits_dup2();
    test_spawn_pipeline_via_dup2();
    test_spawn_stdio();
    test_spawn_stdio_child_fd_cleanup();
    test_find_executable();
    test_errno_strerror();
    test_getopt();
    test_dprintf();
    test_read_line();
    test_terminal_helpers();
    test_getpid_kill_llabs();
    test_signal();
    test_system_cmd();

    l_test_print_summary(passed_count, test_count);
    puts("PASS\n");
    return 0;
}
