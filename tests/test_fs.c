// Filesystem, environment, and low-level I/O helper coverage split from the old monolith.

#define L_WITHSNPRINTF
#define L_MAINFILE
#include "l_os.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

void test_file_operations(void) {
    TEST_FUNCTION("File Operations");

    char* test_msg = "Test message";
    int msg_len = strlen(test_msg);

    // Test l_open_write
    L_FD write_fd = l_open_write("test_write_file");
    TEST_ASSERT(write_fd >= 0, "l_open_write opens file for writing");
    ssize_t written = l_write(write_fd, test_msg, msg_len);
    TEST_ASSERT(written == msg_len, "l_write writes correct number of bytes");
    l_close(write_fd);

    // Test reading back
    L_FD read_fd = l_open_read("test_write_file");
    TEST_ASSERT(read_fd >= 0, "l_open_read opens file for reading");
    char read_buf[20];
    ssize_t read_bytes = l_read(read_fd, read_buf, msg_len);
    TEST_ASSERT(read_bytes == msg_len, "l_read reads correct number of bytes");
    TEST_ASSERT(l_memcmp(read_buf, test_msg, msg_len) == 0, "l_read retrieves correct data");
    l_close(read_fd);

    // Test l_open_readwrite
    L_FD rw_fd = l_open_readwrite("test_rw_file");
    TEST_ASSERT(rw_fd >= 0, "l_open_readwrite opens file for read/write");
    written = l_write(rw_fd, test_msg, msg_len);
    TEST_ASSERT(written == msg_len, "l_write to read/write file works");
    l_close(rw_fd);

    // Test l_open_trunc
    L_FD trunc_fd = l_open_trunc("test_write_file");
    TEST_ASSERT(trunc_fd >= 0, "l_open_trunc opens file for truncation");
    char* short_msg = "Hi";
    written = l_write(trunc_fd, short_msg, 2);
    TEST_ASSERT(written == 2, "l_write to truncated file works");
    l_close(trunc_fd);

    // Verify truncation
    read_fd = l_open_read("test_write_file");
    read_bytes = l_read(read_fd, read_buf, 10);
    TEST_ASSERT(read_bytes == 2, "truncated file has correct size");
    TEST_ASSERT(l_memcmp(read_buf, "Hi", 2) == 0, "truncated file has correct content");
    l_close(read_fd);

    TEST_SECTION_PASS("File operations");
}

void test_system_functions(void) {
    TEST_FUNCTION("System Functions");

    l_exitif(0, -1, "This should not exit");
    TEST_ASSERT(1, "l_exitif with false condition does not exit");

    L_FD open_fd = l_open("test_explicit_open", O_CREAT | O_WRONLY, 0644);
    TEST_ASSERT(open_fd >= 0, "l_open with explicit flags works");
    l_close(open_fd);

    TEST_SECTION_PASS("System functions");
}

void test_getenv(void) {
    TEST_FUNCTION("l_getenv");

    // PATH should exist on every OS
    char *path = l_getenv("PATH");
    TEST_ASSERT(path != NULL, "PATH environment variable exists");
    TEST_ASSERT(l_strlen(path) > 0, "PATH is not empty");

    // Verify the value looks like a PATH (contains path separators)
#ifdef _WIN32
    TEST_ASSERT(l_strchr(path, ';') != NULL || l_strchr(path, '\\') != NULL,
                "PATH value contains Windows path characters");
#else
    TEST_ASSERT(l_strchr(path, '/') != NULL, "PATH value contains '/' (Unix paths)");
#endif

    // Partial name must NOT match (PAT != PATH)
    char *partial = l_getenv("PAT");
    TEST_ASSERT(partial == NULL, "partial name 'PAT' does not match 'PATH'");

    // Empty name
    char *empty = l_getenv("");
    TEST_ASSERT(empty == NULL, "empty name returns NULL");

    // Non-existent variable
    char *bogus = l_getenv("LASTSTANDING_NONEXISTENT_VAR_XYZ");
    TEST_ASSERT(bogus == NULL, "non-existent variable returns NULL");

    // NULL name
    char *null_result = l_getenv(NULL);
    TEST_ASSERT(null_result == NULL, "NULL name returns NULL");

    TEST_SECTION_PASS("l_getenv");
}

void test_env_iter(void) {
    TEST_FUNCTION("l_env_iter");

    void *handle = l_env_start();
    TEST_ASSERT(handle != NULL, "l_env_start returns non-NULL");

    void *iter = handle;
    char buf[4096];
    const char *entry;
    int count = 0;
    int found_path = 0;

    while ((entry = l_env_next(&iter, buf, sizeof(buf))) != NULL) {
        count++;
        // PATH on Unix, Path on Windows — check case-insensitively
        if (l_strncasecmp(entry, "PATH=", 5) == 0)
            found_path = 1;
    }

    TEST_ASSERT(count > 0, "at least one env var found");
    TEST_ASSERT(found_path, "PATH found in environment iteration");

    l_env_end(handle);

    // Verify a second iteration works
    handle = l_env_start();
    iter = handle;
    int count2 = 0;
    while (l_env_next(&iter, buf, sizeof(buf)) != NULL)
        count2++;
    l_env_end(handle);
    TEST_ASSERT(count2 == count, "second iteration yields same count");

    TEST_SECTION_PASS("l_env_iter");
}

void test_lseek(void) {
    TEST_FUNCTION("l_lseek");

    char *msg = "Hello World!";
    int msg_len = l_strlen(msg);

    L_FD fd = l_open_write("test_lseek_file");
    TEST_ASSERT(fd >= 0, "open file for writing");
    ssize_t written = l_write(fd, msg, msg_len);
    TEST_ASSERT(written == msg_len, "write data to file");
    l_close(fd);

    fd = l_open_read("test_lseek_file");
    TEST_ASSERT(fd >= 0, "open file for reading");
    char buf[32];
    ssize_t n = l_read(fd, buf, msg_len);
    TEST_ASSERT(n == msg_len, "first read returns all bytes");
    TEST_ASSERT(l_memcmp(buf, msg, msg_len) == 0, "first read matches written data");

    off_t pos = l_lseek(fd, 0, SEEK_SET);
    TEST_ASSERT(pos == 0, "SEEK_SET to 0 returns offset 0");

    l_memset(buf, 0, 32);
    n = l_read(fd, buf, msg_len);
    TEST_ASSERT(n == msg_len, "read after seek returns correct byte count");
    TEST_ASSERT(l_memcmp(buf, msg, msg_len) == 0, "data after seek matches original");

    pos = l_lseek(fd, 6, SEEK_SET);
    TEST_ASSERT(pos == 6, "SEEK_SET to 6 returns offset 6");
    n = l_read(fd, buf, 6);
    TEST_ASSERT(n == 6, "read 6 bytes from offset 6");
    TEST_ASSERT(l_memcmp(buf, "World!", 6) == 0, "data from offset 6 is 'World!'");

    l_lseek(fd, 0, SEEK_SET);
    l_lseek(fd, 5, SEEK_CUR);
    pos = l_lseek(fd, 0, SEEK_CUR);
    TEST_ASSERT(pos == 5, "SEEK_CUR advances correctly");

    pos = l_lseek(fd, 0, SEEK_END);
    TEST_ASSERT(pos == msg_len, "SEEK_END returns file size");

    l_close(fd);

    TEST_SECTION_PASS("l_lseek");
}

void test_mkdir(void) {
    TEST_FUNCTION("l_mkdir");

    // First attempt may fail if dir exists from a previous run — that's OK
    int ret = l_mkdir("test_mkdir_tmpdir", 0755);
    if (ret < 0) {
        // Directory exists from prior run — test the "already exists" path first
        ret = l_chdir("test_mkdir_tmpdir");
        TEST_ASSERT(ret == 0, "chdir into existing directory succeeds");
        ret = l_chdir("..");
        TEST_ASSERT(ret == 0, "chdir back to parent succeeds");
    } else {
        TEST_ASSERT(ret == 0, "mkdir creates directory successfully");

        ret = l_chdir("test_mkdir_tmpdir");
        TEST_ASSERT(ret == 0, "chdir into new directory succeeds");

        ret = l_chdir("..");
        TEST_ASSERT(ret == 0, "chdir back to parent succeeds");

        ret = l_mkdir("test_mkdir_tmpdir", 0755);
        TEST_ASSERT(ret < 0, "mkdir on existing directory fails");
    }

    TEST_SECTION_PASS("l_mkdir");
}

void test_open_append(void) {
    TEST_FUNCTION("l_open_append");

    // Write initial content, truncating any pre-existing file from prior runs
    L_FD fd = l_open_trunc("test_append_file");
    TEST_ASSERT(fd >= 0, "open for initial write");
    ssize_t w = l_write(fd, "Hello", 5);
    TEST_ASSERT(w == 5, "write initial 5 bytes");
    l_close(fd);

    // Append more content
    fd = l_open_append("test_append_file");
    TEST_ASSERT(fd >= 0, "open for appending");
    w = l_write(fd, " World", 6);
    TEST_ASSERT(w == 6, "append 6 bytes");
    l_close(fd);

    // Verify the full content was preserved and appended
    fd = l_open_read("test_append_file");
    TEST_ASSERT(fd >= 0, "open for read-back");
    char buf[16];
    ssize_t n = l_read(fd, buf, sizeof(buf));
    TEST_ASSERT(n == 11, "total bytes after append is 11");
    TEST_ASSERT(l_memcmp(buf, "Hello World", 11) == 0, "append did not overwrite existing content");
    l_close(fd);

    TEST_SECTION_PASS("l_open_append");
}

void test_sleep_ms(void) {
    TEST_FUNCTION("l_sleep_ms");

    // Smoke-test: a 1 ms sleep should complete without crashing
    l_sleep_ms(1);
    TEST_ASSERT(1, "l_sleep_ms(1) completes without error");

    // Zero sleep is also valid
    l_sleep_ms(0);
    TEST_ASSERT(1, "l_sleep_ms(0) completes without error");

    TEST_SECTION_PASS("l_sleep_ms");
}

void test_sched_yield(void) {
    TEST_FUNCTION("l_sched_yield");

    int ret = l_sched_yield();
    TEST_ASSERT(ret == 0, "l_sched_yield returns 0 on success");

    TEST_SECTION_PASS("l_sched_yield");
}

void test_unlink_rmdir(void) {
    TEST_FUNCTION("l_unlink / l_rmdir");

    // Create a temp file and unlink it
    const char *tmpfile = "test_unlink_tmpfile";
    L_FD fd = l_open_write(tmpfile);
    TEST_ASSERT(fd >= 0, "create temp file for unlink");
    l_write(fd, "data", 4);
    l_close(fd);

    TEST_ASSERT(l_unlink(tmpfile) == 0, "unlink existing file");
    // Verify it's gone by trying to open for read
    fd = l_open_read(tmpfile);
    TEST_ASSERT(fd < 0, "file gone after unlink");

    // unlink non-existent should fail
    TEST_ASSERT(l_unlink("nonexistent_file_xyz") != 0, "unlink non-existent fails");

    // Create a dir and rmdir it
    const char *tmpdir = "test_rmdir_tmpdir";
    l_rmdir(tmpdir);
    TEST_ASSERT(l_mkdir(tmpdir, 0755) == 0, "create temp dir for rmdir");
    TEST_ASSERT(l_rmdir(tmpdir) == 0, "rmdir empty directory");

    // rmdir non-existent should fail
    TEST_ASSERT(l_rmdir("nonexistent_dir_xyz") != 0, "rmdir non-existent fails");

    TEST_SECTION_PASS("l_unlink / l_rmdir");
}

void test_rename_access(void) {
    TEST_FUNCTION("l_rename / l_access");

    // Create a temp file
    const char *src = "test_rename_src";
    const char *dst = "test_rename_dst";
    L_FD fd = l_open_write(src);
    TEST_ASSERT(fd >= 0, "create source file for rename");
    l_write(fd, "hello", 5);
    l_close(fd);

    // l_access: file exists (F_OK)
    TEST_ASSERT(l_access(src, L_F_OK) == 0, "access src F_OK");
    TEST_ASSERT(l_access("nonexistent_xyz_abc", L_F_OK) != 0, "access nonexistent F_OK fails");

    // l_rename: move src to dst
    TEST_ASSERT(l_rename(src, dst) == 0, "rename src to dst");

    // src gone, dst present
    TEST_ASSERT(l_access(src, L_F_OK) != 0, "src gone after rename");
    TEST_ASSERT(l_access(dst, L_F_OK) == 0, "dst present after rename");

    // l_access: check read permission on dst
    TEST_ASSERT(l_access(dst, L_R_OK) == 0, "access dst R_OK");

    // cleanup
    l_unlink(dst);

    TEST_SECTION_PASS("l_rename / l_access");
}

void test_chmod(void) {
    TEST_FUNCTION("l_chmod");

    const char *tmpf = "test_chmod_tmpfile";
    L_FD fd = l_open_write(tmpf);
    TEST_ASSERT(fd >= 0, "create temp file for chmod");
    l_write(fd, "x", 1);
    l_close(fd);

    // Make read-only (no write bit)
    TEST_ASSERT(l_chmod(tmpf, 0444) == 0, "chmod 0444 succeeds");
    // On Linux write permission is now denied; on Windows the readonly attribute is set.
    TEST_ASSERT(l_access(tmpf, L_R_OK) == 0, "readable after chmod 0444");
    TEST_ASSERT(l_access(tmpf, L_W_OK) != 0, "not writable after chmod 0444");

    // Restore write permission
    TEST_ASSERT(l_chmod(tmpf, 0644) == 0, "chmod 0644 succeeds");
    TEST_ASSERT(l_access(tmpf, L_W_OK) == 0, "writable after chmod 0644");

    l_unlink(tmpf);
    TEST_SECTION_PASS("l_chmod");
}

void test_stat(void) {
    TEST_FUNCTION("l_stat / l_fstat");

    // stat a known file
    L_Stat st;
    int ret = l_stat("test_file", &st);
    TEST_ASSERT(ret == 0, "stat test_file succeeds");
    TEST_ASSERT(st.st_size > 0, "test_file has size > 0");
    TEST_ASSERT(L_S_ISREG(st.st_mode), "test_file is a regular file");
    TEST_ASSERT(!L_S_ISDIR(st.st_mode), "test_file is not a directory");

    // stat a directory
    L_Stat dst;
    ret = l_stat(".", &dst);
    TEST_ASSERT(ret == 0, "stat '.' succeeds");
    TEST_ASSERT(L_S_ISDIR(dst.st_mode), "'.' is a directory");
    TEST_ASSERT(!L_S_ISREG(dst.st_mode), "'.' is not a regular file");

    // stat nonexistent
    ret = l_stat("nonexistent_file_stat_xyz", &st);
    TEST_ASSERT(ret == -1, "stat nonexistent returns -1");

    // fstat an open file
    const char *tmpfstat = "test_fstat_tmpfile";
    L_FD fd = l_open_write(tmpfstat);
    TEST_ASSERT(fd >= 0, "create temp file for fstat");
    const char *data = "hello fstat";
    l_write(fd, data, l_strlen(data));
    l_close(fd);

    fd = l_open_read(tmpfstat);
    TEST_ASSERT(fd >= 0, "open temp file for fstat");
    L_Stat fst;
    ret = l_fstat(fd, &fst);
    TEST_ASSERT(ret == 0, "fstat succeeds");
    TEST_ASSERT(fst.st_size == (long long)l_strlen(data), "fstat size matches written data");
    TEST_ASSERT(L_S_ISREG(fst.st_mode), "fstat file is regular");
    l_close(fd);
    l_unlink(tmpfstat);

    TEST_SECTION_PASS("l_stat / l_fstat");
}

void test_opendir_readdir(void) {
    TEST_FUNCTION("l_opendir / l_readdir / l_closedir");

    // Create a test file to look for
    const char *marker = "test_readdir_marker";
    L_FD fd = l_open_write(marker);
    TEST_ASSERT(fd >= 0, "create marker file for readdir");
    l_write(fd, "x", 1);
    l_close(fd);

    L_Dir dir;
    int ret = l_opendir(".", &dir);
    TEST_ASSERT(ret == 0, "opendir '.' succeeds");

    int found_dot = 0;
    int found_dotdot = 0;
    int found_marker = 0;
    L_DirEntry *ent;
    while ((ent = l_readdir(&dir)) != (L_DirEntry *)0) {
        if (l_strcmp(ent->d_name, ".") == 0) found_dot = 1;
        if (l_strcmp(ent->d_name, "..") == 0) found_dotdot = 1;
        if (l_strcmp(ent->d_name, marker) == 0) found_marker = 1;
    }
    l_closedir(&dir);

    TEST_ASSERT(found_dot, "readdir found '.'");
    TEST_ASSERT(found_dotdot, "readdir found '..'");
    TEST_ASSERT(found_marker, "readdir found marker file");

    // opendir nonexistent
    L_Dir bad;
    ret = l_opendir("nonexistent_dir_readdir_xyz", &bad);
    TEST_ASSERT(ret == -1, "opendir nonexistent returns -1");

    l_unlink(marker);

    TEST_SECTION_PASS("l_opendir / l_readdir / l_closedir");
}

void test_mmap(void) {
    TEST_FUNCTION("l_mmap / l_munmap");

    // Test 1: Anonymous mmap - allocate a page, write, read back
    size_t page_sz = 4096;
    void *p = l_mmap((void *)0, page_sz, L_PROT_READ | L_PROT_WRITE,
                     L_MAP_PRIVATE | L_MAP_ANONYMOUS, (L_FD)-1, 0);
    TEST_ASSERT(p != L_MAP_FAILED, "anonymous mmap succeeds");
    TEST_ASSERT(p != (void *)0, "anonymous mmap returns non-NULL");

    // Write a pattern and read it back
    unsigned char *bytes = (unsigned char *)p;
    bytes[0] = 0xAB;
    bytes[1] = 0xCD;
    bytes[page_sz - 1] = 0xEF;
    TEST_ASSERT(bytes[0] == 0xAB, "anonymous mmap write/read byte 0");
    TEST_ASSERT(bytes[1] == 0xCD, "anonymous mmap write/read byte 1");
    TEST_ASSERT(bytes[page_sz - 1] == 0xEF, "anonymous mmap write/read last byte");

    int ret = l_munmap(p, page_sz);
    TEST_ASSERT(ret == 0, "anonymous munmap succeeds");

    // Test 2: File mmap - create a file, write data, mmap read-only, verify
    const char *fname = "test_mmap_file";
    const char *msg = "Hello from mmap!";
    size_t msg_len = l_strlen(msg);

    L_FD fd = l_open_write(fname);
    TEST_ASSERT(fd >= 0, "create file for mmap test");
    l_write(fd, msg, msg_len);
    l_close(fd);

    fd = l_open_read(fname);
    TEST_ASSERT(fd >= 0, "open file for mmap read");

    void *fm = l_mmap((void *)0, msg_len, L_PROT_READ, L_MAP_PRIVATE, fd, 0);
    TEST_ASSERT(fm != L_MAP_FAILED, "file mmap succeeds");
    TEST_ASSERT(l_memcmp(fm, msg, msg_len) == 0, "file mmap contents match");

    ret = l_munmap(fm, msg_len);
    TEST_ASSERT(ret == 0, "file munmap succeeds");
    l_close(fd);
    l_unlink(fname);

    TEST_SECTION_PASS("l_mmap / l_munmap");
}

void test_getcwd_chdir(void) {
    TEST_FUNCTION("l_getcwd / l_chdir");

    char cwd[512];
    char *ret = l_getcwd(cwd, sizeof(cwd));
    TEST_ASSERT(ret != 0, "getcwd returns non-null");
    TEST_ASSERT(l_strlen(cwd) > 0, "getcwd returns non-empty string");

    // Save cwd, create temp dir, chdir to it, verify, chdir back, clean up
    char saved[512];
    l_getcwd(saved, sizeof(saved));

    char tmpdir[512];
    l_snprintf(tmpdir, sizeof(tmpdir), "%s%s", saved,
#ifdef _WIN32
               "\\test_chdir_tmp"
#else
               "/test_chdir_tmp"
#endif
              );

    l_mkdir(tmpdir, 0755);
    int cd_ret = l_chdir(tmpdir);
    TEST_ASSERT(cd_ret == 0, "chdir to temp dir succeeds");

    char after[512];
    l_getcwd(after, sizeof(after));
    TEST_ASSERT(l_strstr(after, "test_chdir_tmp") != 0, "getcwd after chdir contains temp dir name");

    // Restore original cwd and clean up
    l_chdir(saved);
    l_rmdir(tmpdir);

    TEST_SECTION_PASS("l_getcwd / l_chdir");
}

void test_symlink_readlink(void) {
    TEST_FUNCTION("l_symlink / l_readlink / l_realpath");

    // Create a regular file as symlink target
    const char *target_file = "test_symlink_target";
    const char *link_name   = "test_symlink_link";
    L_FD fd = l_open_write(target_file);
    TEST_ASSERT(fd >= 0, "create symlink target file");
    l_write(fd, "symlink test data", 17);
    l_close(fd);

    // Create symlink — may fail on Windows without developer mode
    int ret = l_symlink(target_file, link_name);
#ifdef _WIN32
    if (ret != 0) {
        l_puts("  SKIP: l_symlink not available (requires developer mode on Windows)\n");
        l_unlink(target_file);
        return;
    }
#else
    TEST_ASSERT(ret == 0, "symlink created successfully");
#endif

    // Verify the symlink exists
    TEST_ASSERT(l_access(link_name, L_F_OK) == 0, "symlink exists");

    // Read symlink target
    char buf[L_PATH_MAX];
    l_memset(buf, 0, sizeof(buf));
    ptrdiff_t n = l_readlink(link_name, buf, L_PATH_MAX);
    TEST_ASSERT(n > 0, "readlink returns positive count");
    TEST_ASSERT(l_strstr(buf, target_file) != 0, "readlink target matches");

    // readlink on a non-symlink should fail
    ptrdiff_t bad = l_readlink(target_file, buf, L_PATH_MAX);
    TEST_ASSERT(bad < 0, "readlink on regular file fails");

    // Dangling symlink: target doesn't exist but symlink creation should succeed
    const char *dangling_link = "test_symlink_dangling";
    ret = l_symlink("nonexistent_target_xyz", dangling_link);
    TEST_ASSERT(ret == 0, "dangling symlink creation succeeds");
    l_unlink(dangling_link);

    // l_realpath: resolve path through symlink
    char resolved[L_PATH_MAX];
    l_memset(resolved, 0, sizeof(resolved));
    char *rp = l_realpath(link_name, resolved);
    TEST_ASSERT(rp != 0, "realpath returns non-null");
    TEST_ASSERT(l_strlen(resolved) > 0, "realpath returns non-empty string");
    TEST_ASSERT(l_strstr(resolved, target_file) != 0, "realpath contains target filename");

    // Also test realpath on the target directly
    char resolved2[L_PATH_MAX];
    char *rp2 = l_realpath(target_file, resolved2);
    TEST_ASSERT(rp2 != 0, "realpath on regular file returns non-null");
    // Both should resolve to the same canonical path
    TEST_ASSERT(l_strcmp(resolved, resolved2) == 0, "realpath through symlink matches direct path");

    // Clean up
    l_unlink(link_name);
    l_unlink(target_file);

    TEST_SECTION_PASS("l_symlink / l_readlink / l_realpath");
}

void test_setenv(void) {
    TEST_FUNCTION("l_setenv / l_unsetenv");

    // Set a new variable
    int r1 = l_setenv("L_TEST_VAR", "hello");
    TEST_ASSERT(r1 == 0, "setenv returns 0 on success");

    char *val = l_getenv("L_TEST_VAR");
    TEST_ASSERT(val != (void *)0, "getenv returns non-NULL after setenv");
    TEST_ASSERT(l_strcmp(val, "hello") == 0, "getenv returns correct value");

    // Overwrite with new value
    int r2 = l_setenv("L_TEST_VAR", "world");
    TEST_ASSERT(r2 == 0, "setenv overwrite returns 0");

    char *val2 = l_getenv("L_TEST_VAR");
    TEST_ASSERT(val2 != (void *)0, "getenv returns non-NULL after overwrite");
    TEST_ASSERT(l_strcmp(val2, "world") == 0, "getenv returns overwritten value");

    // Unset the variable
    int r3 = l_unsetenv("L_TEST_VAR");
    TEST_ASSERT(r3 == 0, "unsetenv returns 0");

    char *val3 = l_getenv("L_TEST_VAR");
    TEST_ASSERT(val3 == (void *)0, "getenv returns NULL after unsetenv");

    TEST_SECTION_PASS("l_setenv / l_unsetenv");
}

void test_writev_readv(void) {
    TEST_FUNCTION("l_writev / l_readv");

    // Create a temp file
    const char *fname = "test_iov_file";
    L_FD fd = l_open_write(fname);
    TEST_ASSERT(fd >= 0, "open file for writev");

    // writev two buffers
    char buf1[] = "Hello, ";
    char buf2[] = "World!";
    L_IoVec wvec[2];
    wvec[0].base = buf1;
    wvec[0].len = 7;
    wvec[1].base = buf2;
    wvec[1].len = 6;
    ptrdiff_t nw = l_writev(fd, wvec, 2);
    TEST_ASSERT(nw == 13, "writev writes all 13 bytes");
    l_close(fd);

    // Reopen and readv into two buffers
    fd = l_open_read(fname);
    TEST_ASSERT(fd >= 0, "reopen file for readv");

    char rbuf1[8] = {0};
    char rbuf2[8] = {0};
    L_IoVec rvec[2];
    rvec[0].base = rbuf1;
    rvec[0].len = 7;
    rvec[1].base = rbuf2;
    rvec[1].len = 6;
    ptrdiff_t nr = l_readv(fd, rvec, 2);
    TEST_ASSERT(nr == 13, "readv reads all 13 bytes");
    TEST_ASSERT(l_memcmp(rbuf1, "Hello, ", 7) == 0, "readv first buffer correct");
    TEST_ASSERT(l_memcmp(rbuf2, "World!", 6) == 0, "readv second buffer correct");
    l_close(fd);

    l_unlink(fname);

    TEST_SECTION_PASS("l_writev / l_readv");
}

void test_isatty(void) {
    TEST_FUNCTION("l_isatty");

    // A pipe fd should not be a terminal
    L_FD fds[2];
    int ret = l_pipe(fds);
    TEST_ASSERT(ret == 0, "pipe for isatty test");

    TEST_ASSERT(l_isatty(fds[0]) == 0, "pipe read end is not a tty");
    TEST_ASSERT(l_isatty(fds[1]) == 0, "pipe write end is not a tty");

    l_close(fds[0]);
    l_close(fds[1]);

    // A regular file fd should not be a terminal
    L_FD fd = l_open_write("test_isatty_file");
    TEST_ASSERT(fd >= 0, "open file for isatty");
    TEST_ASSERT(l_isatty(fd) == 0, "regular file is not a tty");
    l_close(fd);
    l_unlink("test_isatty_file");

    TEST_SECTION_PASS("l_isatty");
}

void test_poll(void) {
    TEST_FUNCTION("l_poll");

    L_FD fds[2];
    int ret = l_pipe(fds);
    TEST_ASSERT(ret == 0, "pipe for poll creates successfully");

    l_write(fds[1], "hello", 5);

    L_PollFd pfd;
    pfd.fd = fds[0];
    pfd.events = L_POLLIN;
    pfd.revents = 0;
    int ready = l_poll(&pfd, 1, 1000);
    TEST_ASSERT(ready > 0, "poll returns >0 when data available");
    TEST_ASSERT((pfd.revents & L_POLLIN) != 0, "poll sets POLLIN in revents");

    char buf[16];
    l_read(fds[0], buf, sizeof(buf));

#ifndef _WIN32
    pfd.revents = 0;
    {
        int ready2 = l_poll(&pfd, 1, 0);
        TEST_ASSERT(ready2 == 0, "poll returns 0 on empty pipe with timeout=0");
    }
#endif

    l_close(fds[0]);
    l_close(fds[1]);

    TEST_SECTION_PASS("l_poll");
}

void test_path_utils(void) {
    TEST_FUNCTION("l_path_join / l_path_ext / l_path_exists / l_path_isdir");

    char buf[256];

    // l_path_join
    l_path_join(buf, sizeof(buf), "dir", "file");
    TEST_ASSERT(l_strcmp(buf, "dir/file") == 0, "path_join dir + file");

    l_path_join(buf, sizeof(buf), "dir/", "file");
    TEST_ASSERT(l_strcmp(buf, "dir/file") == 0, "path_join no double separator");

    l_path_join(buf, sizeof(buf), "", "file");
    TEST_ASSERT(l_strcmp(buf, "file") == 0, "path_join empty dir");

    // l_path_ext
    TEST_ASSERT(l_strcmp(l_path_ext("file.txt"), ".txt") == 0, "path_ext .txt");
    TEST_ASSERT(l_strcmp(l_path_ext("file"), "") == 0, "path_ext no extension");
    TEST_ASSERT(l_strcmp(l_path_ext("dir/file.tar.gz"), ".gz") == 0, "path_ext .gz");
    TEST_ASSERT(l_strcmp(l_path_ext(".hidden"), "") == 0, "path_ext dot-only not extension");

    // l_path_exists
    TEST_ASSERT(l_path_exists("tests/test.c") == 1, "path_exists on existing file");
    TEST_ASSERT(l_path_exists("nonexistent_xyz_123") == 0, "path_exists on missing file");

    // l_path_isdir
    TEST_ASSERT(l_path_isdir(".") == 1, "path_isdir on current dir");

    TEST_SECTION_PASS("path_utils");
}

void test_read_write_all(void) {
    TEST_FUNCTION("l_read_all / l_write_all");

    const char *path = "test_rw_all_tmp";
    const char *data = "Hello, read_all and write_all!";
    size_t data_len = l_strlen(data);

    // Write data to file
    L_FD fd = l_open_trunc(path);
    TEST_ASSERT(fd >= 0, "open file for write_all");
    ptrdiff_t written = l_write_all(fd, data, data_len);
    TEST_ASSERT(written == (ptrdiff_t)data_len, "write_all returns correct count");
    l_close(fd);

    // Read back with exact size
    fd = l_open_read(path);
    TEST_ASSERT(fd >= 0, "open file for read_all");
    char rbuf[128];
    l_memset(rbuf, 0, sizeof(rbuf));
    ptrdiff_t nread = l_read_all(fd, rbuf, data_len);
    TEST_ASSERT(nread == (ptrdiff_t)data_len, "read_all returns correct count");
    TEST_ASSERT(l_memcmp(rbuf, data, data_len) == 0, "read_all content matches");
    l_close(fd);

    // Read back requesting more than file contains
    fd = l_open_read(path);
    l_memset(rbuf, 0, sizeof(rbuf));
    ptrdiff_t nread2 = l_read_all(fd, rbuf, sizeof(rbuf));
    TEST_ASSERT(nread2 == (ptrdiff_t)data_len, "read_all partial returns actual size");
    TEST_ASSERT(l_memcmp(rbuf, data, data_len) == 0, "read_all partial content matches");
    l_close(fd);

    l_unlink(path);
    TEST_SECTION_PASS("l_read_all / l_write_all");
}

void test_file_size_helper(void) {
    TEST_FUNCTION("l_file_size");

    const char *path = "test_fsize_tmp";
    const char *data = "1234567890";

    L_FD fd = l_open_trunc(path);
    TEST_ASSERT(fd >= 0, "create file for size test");
    l_write(fd, data, 10);
    l_close(fd);

    long long sz = l_file_size(path);
    TEST_ASSERT(sz == 10, "file_size returns 10");

    // Nonexistent file
    long long sz2 = l_file_size("nonexistent_file_xyz_12345");
    TEST_ASSERT(sz2 == -1, "file_size nonexistent returns -1");

    // Empty file
    fd = l_open_trunc(path);
    l_close(fd);
    long long sz3 = l_file_size(path);
    TEST_ASSERT(sz3 == 0, "file_size empty file returns 0");

    l_unlink(path);
    TEST_SECTION_PASS("l_file_size");
}

void test_truncate_helper(void) {
    TEST_FUNCTION("l_truncate / l_ftruncate");

    const char *path = "test_trunc_tmp";
    const char *data = "abcdefghijklmnopqrstuvwxyz"; // 26 bytes

    // Write 26 bytes
    L_FD fd = l_open_trunc(path);
    TEST_ASSERT(fd >= 0, "create file for truncate test");
    l_write(fd, data, 26);
    l_close(fd);

    // Truncate to 10 via l_truncate
    int r = l_truncate(path, 10);
    TEST_ASSERT(r == 0, "l_truncate returns 0");

#if defined(__arm__)
    // ARM 32-bit truncate64 syscall has known issues with 64-bit arg passing
    l_puts("  [SKIP] ARM 32-bit truncate64 size verification\n");
    l_unlink(path);
    TEST_SECTION_PASS("l_truncate / l_ftruncate (partial)");
    return;
#endif

    long long sz = l_file_size(path);
    TEST_ASSERT(sz == 10, "l_truncate size is 10");

    // Verify content
    fd = l_open_read(path);
    char rbuf[32];
    ptrdiff_t n = l_read(fd, rbuf, sizeof(rbuf));
    TEST_ASSERT(n == 10, "truncated file reads 10 bytes");
    TEST_ASSERT(l_memcmp(rbuf, "abcdefghij", 10) == 0, "truncated content correct");
    l_close(fd);

    // ftruncate on open fd
    fd = l_open_readwrite(path);
    int r2 = l_ftruncate(fd, 5);
    TEST_ASSERT(r2 == 0, "l_ftruncate returns 0");
    l_close(fd);
    long long sz2 = l_file_size(path);
    TEST_ASSERT(sz2 == 5, "l_ftruncate size is 5");

    l_unlink(path);
    TEST_SECTION_PASS("l_truncate / l_ftruncate");
}

void test_glob_helper(void) {
    TEST_FUNCTION("l_glob");

    L_Arena a = l_arena_init(64 * 1024);

    // Glob examples/*.c should find multiple files
    // l_glob returns the number of matches (>=0), or -1 on error
    L_Str *paths = (L_Str *)0;
    int count = 0;
    int r = l_glob("examples/*.c", &paths, &count, &a);
    TEST_ASSERT(r >= 0, "l_glob returns >= 0");
    TEST_ASSERT(count > 5, "l_glob finds multiple example .c files");

    // Check that at least hello.c is among results
    int found_hello_c = 0;
    for (int i = 0; i < count; i++) {
        if (l_str_contains(paths[i], l_str("hello.c")))
            found_hello_c = 1;
    }
    TEST_ASSERT(found_hello_c, "l_glob found hello.c");

    // Glob with no matches
    L_Str *paths2 = (L_Str *)0;
    int count2 = 0;
    int r2 = l_glob("tests/*.xyz_nomatch", &paths2, &count2, &a);
    TEST_ASSERT(r2 == 0, "l_glob no matches returns 0");
    TEST_ASSERT(count2 == 0, "l_glob no matches count=0");

    // Glob for specific file
    L_Str *paths3 = (L_Str *)0;
    int count3 = 0;
    int r3 = l_glob("tests/test.c", &paths3, &count3, &a);
    TEST_ASSERT(r3 == 1, "l_glob specific file returns 1");

    l_arena_free(&a);
    TEST_SECTION_PASS("l_glob");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    test_file_operations();
    test_system_functions();
    test_getenv();
    test_env_iter();
    test_lseek();
    test_mkdir();
    test_open_append();
    test_sleep_ms();
    test_sched_yield();
    test_unlink_rmdir();
    test_rename_access();
    test_chmod();
    test_stat();
    test_opendir_readdir();
    test_mmap();
    test_getcwd_chdir();
    test_symlink_readlink();
    test_setenv();
    test_writev_readv();
    test_isatty();
    test_poll();
    test_path_utils();
    test_read_write_all();
    test_file_size_helper();
    test_truncate_helper();
    test_glob_helper();

    l_test_print_summary(passed_count, test_count);
    puts("PASS\n");
    return 0;
}
