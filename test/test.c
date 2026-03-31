// This tests multiple files including the library with just one of them defined as L_MAINFILE
#define L_WITHSNPRINTF
#define L_WITHSOCKETS
#include "l_os.h"
#include "l_os.h"

#define L_MAINFILE
#include "l_os.h"

// Test reporting macros
static int test_count = 0;
static int passed_count = 0;

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

#define TEST_SECTION_PASS(name) do { \
    puts("  " name " tests: PASSED\n"); \
} while(0)

static void build_bin_path(const char *name, char *buf, size_t size) {
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

static int read_fd_all(L_FD fd, char *buf, int max) {
    int total = 0;
    while (total < max - 1) {
        ssize_t n = l_read(fd, buf + total, (size_t)(max - 1 - total));
        if (n <= 0) break;
        total += (int)n;
    }
    buf[total] = '\0';
    return total;
}

static int redirect_std_fd(L_FD target_fd, L_FD source_fd, L_FD *saved_fd) {
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

static int restore_std_fd(L_FD target_fd, L_FD saved_fd) {
    int ok = 1;
    if (saved_fd < 0)
        return 0;
    if (l_dup2(saved_fd, target_fd) != target_fd)
        ok = 0;
    if (l_close(saved_fd) < 0)
        ok = 0;
    return ok ? 0 : -1;
}

static L_PID spawn_with_redirects(const char *path, char *const argv[], char *const envp[],
                                  L_FD stdin_fd, L_FD stdout_fd, L_FD stderr_fd,
                                  int *restore_ok) {
    L_FD saved_in = -1;
    L_FD saved_out = -1;
    L_FD saved_err = -1;
    int ok = 1;
    L_PID pid = -1;

    if (restore_ok)
        *restore_ok = 1;

    if (redirect_std_fd(L_STDIN, stdin_fd, &saved_in) < 0)
        goto done;
    if (redirect_std_fd(L_STDOUT, stdout_fd, &saved_out) < 0)
        goto done;
    if (redirect_std_fd(L_STDERR, stderr_fd, &saved_err) < 0)
        goto done;

    pid = l_spawn(path, argv, envp);

done:
    if (restore_std_fd(L_STDERR, saved_err) < 0)
        ok = 0;
    if (restore_std_fd(L_STDOUT, saved_out) < 0)
        ok = 0;
    if (restore_std_fd(L_STDIN, saved_in) < 0)
        ok = 0;

    if (restore_ok)
        *restore_ok = ok;
    return pid;
}



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

// ===================== Command line & program name =====================

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

// ===================== Unicode output =====================

void test_unicode_output(void) {
    TEST_FUNCTION("Unicode Output");
    char msg[] = u8"κόσμε";
    puts(msg);
    puts("\n");
    TEST_ASSERT(1, "unicode output test completed");
    TEST_SECTION_PASS("Unicode output");
}

// ===================== l_strlen =====================

void test_strlen(void) {
    TEST_FUNCTION("l_strlen");

    TEST_ASSERT(l_strlen("") == 0, "empty string has length 0");
    TEST_ASSERT(l_strlen("a") == 1, "single char has length 1");
    TEST_ASSERT(l_strlen("Hello") == 5, "'Hello' has length 5");
    TEST_ASSERT(l_strlen("Hello World") == 11, "'Hello World' has length 11");

    TEST_ASSERT(l_strlen("\t\n\r") == 3, "tab/newline/return has length 3");

    char buf[256];
    l_memset(buf, 'x', 255);
    buf[255] = '\0';
    TEST_ASSERT(l_strlen(buf) == 255, "255-char string has length 255");

    /* Alignment-boundary tests: exercise all phases of word-at-a-time path.
     * Use a 32-byte buffer with the null terminator placed at each of the
     * first 16 positions to cover every alignment offset. */
    char abuf[32];
    l_memset(abuf, 'A', 31);
    abuf[31] = '\0';
    {
        int ok = 1;
        int i;
        for (i = 0; i <= 15; i++) {
            abuf[i] = '\0';
            if (l_strlen(abuf) != (size_t)i) { ok = 0; break; }
            abuf[i] = 'A';
        }
        TEST_ASSERT(ok, "word-at-a-time: strlen correct at alignment offsets 0-15");
    }

    TEST_SECTION_PASS("l_strlen");
}

// ===================== l_strstr =====================

void test_strstr(void) {
    TEST_FUNCTION("l_strstr");

    char hay[] = "Hello World";
    TEST_ASSERT(l_strstr(hay, "Hello") == hay, "find at start");
    TEST_ASSERT(l_strstr(hay, "World") == hay + 6, "find at end");
    TEST_ASSERT(l_strstr(hay, "lo W") == hay + 3, "find in middle");
    TEST_ASSERT(l_strstr(hay, "xyz") == NULL, "not found returns NULL");
    TEST_ASSERT(l_strstr(hay, "") == hay, "empty needle matches start");
    char empty[] = "";
    TEST_ASSERT(l_strstr(empty, "") == empty, "empty hay + empty needle returns haystack (POSIX)");
    TEST_ASSERT(l_strstr("", "a") == NULL, "empty hay, non-empty needle");
    TEST_ASSERT(l_strstr(hay, "Hello World") == hay, "needle equals haystack");
    TEST_ASSERT(l_strstr(hay, "Hello World!") == NULL, "needle longer than haystack");

    char rep[] = "abcabcabc";
    TEST_ASSERT(l_strstr(rep, "abc") == rep, "repeated pattern finds first");
    TEST_ASSERT(l_strstr(rep, "cab") == rep + 2, "overlapping match");

    TEST_SECTION_PASS("l_strstr");
}

// ===================== l_strchr =====================

void test_strchr(void) {
    TEST_FUNCTION("l_strchr");

    char s[] = "abcdef";
    TEST_ASSERT(l_strchr(s, 'a') == s, "find first char");
    TEST_ASSERT(l_strchr(s, 'f') == s + 5, "find last char");
    TEST_ASSERT(l_strchr(s, 'z') == NULL, "missing char returns NULL");
    TEST_ASSERT(l_strchr("", 'a') == NULL, "empty string returns NULL");

    char dup[] = "aabaa";
    TEST_ASSERT(l_strchr(dup, 'a') == dup, "finds first of duplicates");
    TEST_ASSERT(l_strchr(dup, 'b') == dup + 2, "finds unique in duplicates");

    // C standard: strchr(s, '\0') must return pointer to null terminator
    TEST_ASSERT(l_strchr(s, '\0') == s + 6, "null char returns pointer to terminator");
    TEST_ASSERT(l_strchr("", '\0') != NULL, "null char in empty string returns non-NULL");

    TEST_SECTION_PASS("l_strchr");
}

// ===================== l_strrchr =====================

void test_strrchr(void) {
    TEST_FUNCTION("l_strrchr");

    char s[] = "abcabc";
    TEST_ASSERT(l_strrchr(s, 'a') == s + 3, "finds last 'a'");
    TEST_ASSERT(l_strrchr(s, 'c') == s + 5, "finds last 'c'");
    TEST_ASSERT(l_strrchr(s, 'z') == NULL, "missing char returns NULL");
    TEST_ASSERT(l_strrchr("", 'a') == NULL, "empty string returns NULL");

    char sx[] = "x";
    TEST_ASSERT(l_strrchr(sx, 'x') == sx, "single char match");
    TEST_ASSERT(l_strrchr(sx, 'y') == NULL, "single char no match");

    // C standard: strrchr(s, '\0') must return pointer to null terminator
    TEST_ASSERT(l_strrchr(s, '\0') == s + 6, "null char returns pointer to terminator");
    TEST_ASSERT(l_strrchr("", '\0') != NULL, "null char in empty string returns non-NULL");

    TEST_SECTION_PASS("l_strrchr");
}

// ===================== l_strcpy =====================

void test_strcpy(void) {
    TEST_FUNCTION("l_strcpy");

    char dst[32];

    // Return value
    char *result = l_strcpy(dst, "Hello");
    TEST_ASSERT(result == dst, "l_strcpy returns destination pointer");
    TEST_ASSERT(l_strncmp(dst, "Hello", 5) == 0, "copies string correctly");

    // Empty string
    l_strcpy(dst, "");
    TEST_ASSERT(dst[0] == '\0', "copy empty string");
    TEST_ASSERT(l_strlen(dst) == 0, "copied empty string has length 0");

    // Normal copy with null terminator
    l_strcpy(dst, "abc");
    TEST_ASSERT(l_strncmp(dst, "abc", 3) == 0, "copy 'abc'");
    TEST_ASSERT(dst[3] == '\0', "null terminator present after copy");

    // Longer string
    l_strcpy(dst, "Hello, World!");
    TEST_ASSERT(l_strlen(dst) == 13, "copied long string has correct length");
    TEST_ASSERT(l_strncmp(dst, "Hello, World!", 13) == 0, "long string content correct");

    TEST_SECTION_PASS("l_strcpy");
}

// ===================== l_strncpy =====================

void test_strncpy(void) {
    TEST_FUNCTION("l_strncpy");

    char buf[32];

    // Normal copy, n > strlen
    l_memset(buf, 'X', sizeof(buf));
    l_strncpy(buf, "hello", 10);
    TEST_ASSERT(l_strcmp(buf, "hello") == 0, "basic copy");
    TEST_ASSERT(buf[5] == '\0' && buf[6] == '\0' && buf[9] == '\0', "zero-padded");

    // Exact length copy, no null terminator written
    l_memset(buf, 'X', sizeof(buf));
    l_strncpy(buf, "abcde", 5);
    TEST_ASSERT(l_strncmp(buf, "abcde", 5) == 0, "exact n=strlen copy");
    TEST_ASSERT(buf[5] == 'X', "no write past n");

    // n=0 should not write anything
    l_memset(buf, 'Y', sizeof(buf));
    l_strncpy(buf, "test", 0);
    TEST_ASSERT(buf[0] == 'Y', "n=0 writes nothing");

    // Empty source: should zero-fill
    l_memset(buf, 'Z', sizeof(buf));
    l_strncpy(buf, "", 5);
    TEST_ASSERT(buf[0] == '\0' && buf[4] == '\0', "empty src zero-fills");

    // Return value is dst
    l_memset(buf, 0, sizeof(buf));
    TEST_ASSERT(l_strncpy(buf, "ret", 4) == buf, "returns dst pointer");

    TEST_SECTION_PASS("l_strncpy");
}

// ===================== l_strcat =====================

void test_strcat(void) {
    TEST_FUNCTION("l_strcat");

    char buf[32];
    l_strcpy(buf, "Hello");
    char *result = l_strcat(buf, ", World");
    TEST_ASSERT(result == buf, "l_strcat returns destination pointer");
    TEST_ASSERT(l_strcmp(buf, "Hello, World") == 0, "basic concatenation");

    l_strcpy(buf, "");
    l_strcat(buf, "abc");
    TEST_ASSERT(l_strcmp(buf, "abc") == 0, "append to empty string");

    l_strcpy(buf, "abc");
    l_strcat(buf, "");
    TEST_ASSERT(l_strcmp(buf, "abc") == 0, "append empty string");

    l_strcpy(buf, "");
    l_strcat(buf, "");
    TEST_ASSERT(l_strcmp(buf, "") == 0, "both empty");

    TEST_SECTION_PASS("l_strcat");
}

// ===================== l_strncat =====================

void test_strncat(void) {
    TEST_FUNCTION("l_strncat");

    char buf[32];
    l_strcpy(buf, "Hello");
    char *result = l_strncat(buf, ", World", 7);
    TEST_ASSERT(result == buf, "l_strncat returns destination pointer");
    TEST_ASSERT(l_strcmp(buf, "Hello, World") == 0, "full-length append");

    l_strcpy(buf, "Hello");
    l_strncat(buf, ", World", 2);
    TEST_ASSERT(l_strcmp(buf, "Hello, ") == 0, "partial append (n=2)");

    l_strcpy(buf, "abc");
    l_strncat(buf, "xyz", 0);
    TEST_ASSERT(l_strcmp(buf, "abc") == 0, "n=0 appends nothing");

    l_strcpy(buf, "abc");
    l_strncat(buf, "defgh", 100);
    TEST_ASSERT(l_strcmp(buf, "abcdefgh") == 0, "n larger than src always null-terminates");

    TEST_SECTION_PASS("l_strncat");
}

// ===================== l_strcmp =====================

void test_strcmp(void) {
    TEST_FUNCTION("l_strcmp");

    TEST_ASSERT(l_strcmp("abc", "abc") == 0, "equal strings");
    TEST_ASSERT(l_strcmp("", "") == 0, "two empty strings");
    TEST_ASSERT(l_strcmp("a", "a") == 0, "single equal char");

    TEST_ASSERT(l_strcmp("abc", "abd") < 0, "'abc' < 'abd'");
    TEST_ASSERT(l_strcmp("abc", "abcd") < 0, "prefix is less");
    TEST_ASSERT(l_strcmp("", "a") < 0, "empty < non-empty");
    TEST_ASSERT(l_strcmp("A", "a") < 0, "'A' < 'a'");

    TEST_ASSERT(l_strcmp("abd", "abc") > 0, "'abd' > 'abc'");
    TEST_ASSERT(l_strcmp("abcd", "abc") > 0, "longer > prefix");
    TEST_ASSERT(l_strcmp("a", "") > 0, "non-empty > empty");
    TEST_ASSERT(l_strcmp("b", "a") > 0, "'b' > 'a'");

    TEST_ASSERT(l_strcmp("\x80", "\x01") > 0, "unsigned: 0x80 > 0x01");

    TEST_SECTION_PASS("l_strcmp");
}

// ===================== l_strncmp =====================

void test_strncmp(void) {
    TEST_FUNCTION("l_strncmp");

    TEST_ASSERT(l_strncmp("Hello", "Hello", 5) == 0, "equal strings");
    TEST_ASSERT(l_strncmp("Hello", "Hell", 4) == 0, "partial match");
    TEST_ASSERT(l_strncmp("Hello", "Help", 3) == 0, "first 3 chars match");
    TEST_ASSERT(l_strncmp("Hello", "Help", 4) < 0, "'Hello' < 'Help'");
    TEST_ASSERT(l_strncmp("Help", "Hello", 4) > 0, "'Help' > 'Hello'");

    TEST_ASSERT(l_strncmp("abc", "xyz", 0) == 0, "n=0 always equal");
    TEST_ASSERT(l_strncmp("abc", "abc", 10) == 0, "equal strings with large n");
    TEST_ASSERT(l_strncmp("", "", 5) == 0, "two empty strings");
    TEST_ASSERT(l_strncmp("", "a", 1) < 0, "empty vs non-empty");
    TEST_ASSERT(l_strncmp("a", "", 1) > 0, "non-empty vs empty");
    TEST_ASSERT(l_strncmp("a", "b", 1) < 0, "'a' < 'b'");
    TEST_ASSERT(l_strncmp("b", "a", 1) > 0, "'b' > 'a'");

    TEST_SECTION_PASS("l_strncmp");
}

// ===================== l_reverse =====================

void test_reverse(void) {
    TEST_FUNCTION("l_reverse");

    char rev[] = "Hello";
    l_reverse(rev, 5);
    TEST_ASSERT(l_strncmp(rev, "olleH", 5) == 0, "reverses string");

    char even[] = "abcd";
    l_reverse(even, 4);
    TEST_ASSERT(l_strncmp(even, "dcba", 4) == 0, "reverse even-length string");

    char one[] = "x";
    l_reverse(one, 1);
    TEST_ASSERT(one[0] == 'x', "reverse single char is no-op");

    char two[] = "ab";
    l_reverse(two, 2);
    TEST_ASSERT(l_strncmp(two, "ba", 2) == 0, "reverse two chars");

    char pal[] = "abcba";
    l_reverse(pal, 5);
    TEST_ASSERT(l_strncmp(pal, "abcba", 5) == 0, "reverse palindrome unchanged");

    char dbl[] = "Hello";
    l_reverse(dbl, 5);
    l_reverse(dbl, 5);
    TEST_ASSERT(l_strncmp(dbl, "Hello", 5) == 0, "double reverse restores original");

    TEST_SECTION_PASS("l_reverse");
}

// ===================== l_isdigit =====================

void test_isdigit(void) {
    TEST_FUNCTION("l_isdigit");

    TEST_ASSERT(l_isdigit('0') != 0, "recognizes '0'");
    TEST_ASSERT(l_isdigit('5') != 0, "recognizes '5'");
    TEST_ASSERT(l_isdigit('9') != 0, "recognizes '9'");

    for (char c = '0'; c <= '9'; c++) {
        if (!l_isdigit(c)) {
            TEST_ASSERT(0, "l_isdigit must accept all 0-9");
        }
    }
    TEST_ASSERT(1, "all digits 0-9 recognized");

    TEST_ASSERT(l_isdigit('a') == 0, "rejects 'a'");
    TEST_ASSERT(l_isdigit('Z') == 0, "rejects 'Z'");
    TEST_ASSERT(l_isdigit(' ') == 0, "rejects space");
    TEST_ASSERT(l_isdigit('\0') == 0, "rejects null");
    TEST_ASSERT(l_isdigit('-') == 0, "rejects minus");
    TEST_ASSERT(l_isdigit('+') == 0, "rejects plus");
    TEST_ASSERT(l_isdigit('.') == 0, "rejects dot");
    TEST_ASSERT(l_isdigit('0' - 1) == 0, "rejects char before '0'");
    TEST_ASSERT(l_isdigit('9' + 1) == 0, "rejects char after '9'");

    TEST_SECTION_PASS("l_isdigit");
}

// ===================== l_isspace =====================

void test_isspace(void) {
    TEST_FUNCTION("l_isspace");

    TEST_ASSERT(l_isspace(' ')  != 0, "space");
    TEST_ASSERT(l_isspace('\t') != 0, "tab");
    TEST_ASSERT(l_isspace('\n') != 0, "newline");
    TEST_ASSERT(l_isspace('\r') != 0, "carriage return");
    TEST_ASSERT(l_isspace('\f') != 0, "form feed");
    TEST_ASSERT(l_isspace('\v') != 0, "vertical tab");

    TEST_ASSERT(l_isspace('a')  == 0, "rejects 'a'");
    TEST_ASSERT(l_isspace('0')  == 0, "rejects '0'");
    TEST_ASSERT(l_isspace('\0') == 0, "rejects null");
    TEST_ASSERT(l_isspace('!')  == 0, "rejects '!'");

    TEST_SECTION_PASS("l_isspace");
}

// ===================== l_isalpha / l_isalnum / l_isupper / l_islower / l_toupper / l_tolower =====================

void test_char_classification(void) {
    TEST_FUNCTION("l_isalpha/l_isalnum/l_isupper/l_islower/l_toupper/l_tolower");

    /* l_isalpha */
    TEST_ASSERT(l_isalpha('a') != 0, "isalpha 'a'");
    TEST_ASSERT(l_isalpha('z') != 0, "isalpha 'z'");
    TEST_ASSERT(l_isalpha('A') != 0, "isalpha 'A'");
    TEST_ASSERT(l_isalpha('Z') != 0, "isalpha 'Z'");
    TEST_ASSERT(l_isalpha('m') != 0, "isalpha 'm'");
    TEST_ASSERT(l_isalpha('0') == 0, "isalpha rejects '0'");
    TEST_ASSERT(l_isalpha('9') == 0, "isalpha rejects '9'");
    TEST_ASSERT(l_isalpha(' ') == 0, "isalpha rejects space");
    TEST_ASSERT(l_isalpha('_') == 0, "isalpha rejects '_'");
    TEST_ASSERT(l_isalpha('\0') == 0, "isalpha rejects null");

    /* l_isalnum */
    TEST_ASSERT(l_isalnum('a') != 0, "isalnum 'a'");
    TEST_ASSERT(l_isalnum('Z') != 0, "isalnum 'Z'");
    TEST_ASSERT(l_isalnum('5') != 0, "isalnum '5'");
    TEST_ASSERT(l_isalnum('0') != 0, "isalnum '0'");
    TEST_ASSERT(l_isalnum(' ') == 0, "isalnum rejects space");
    TEST_ASSERT(l_isalnum('!') == 0, "isalnum rejects '!'");
    TEST_ASSERT(l_isalnum('\0') == 0, "isalnum rejects null");

    /* l_isupper */
    TEST_ASSERT(l_isupper('A') != 0, "isupper 'A'");
    TEST_ASSERT(l_isupper('Z') != 0, "isupper 'Z'");
    TEST_ASSERT(l_isupper('M') != 0, "isupper 'M'");
    TEST_ASSERT(l_isupper('a') == 0, "isupper rejects 'a'");
    TEST_ASSERT(l_isupper('z') == 0, "isupper rejects 'z'");
    TEST_ASSERT(l_isupper('0') == 0, "isupper rejects '0'");

    /* l_islower */
    TEST_ASSERT(l_islower('a') != 0, "islower 'a'");
    TEST_ASSERT(l_islower('z') != 0, "islower 'z'");
    TEST_ASSERT(l_islower('m') != 0, "islower 'm'");
    TEST_ASSERT(l_islower('A') == 0, "islower rejects 'A'");
    TEST_ASSERT(l_islower('Z') == 0, "islower rejects 'Z'");
    TEST_ASSERT(l_islower('0') == 0, "islower rejects '0'");

    /* l_toupper */
    TEST_ASSERT(l_toupper('a') == 'A', "toupper 'a' -> 'A'");
    TEST_ASSERT(l_toupper('z') == 'Z', "toupper 'z' -> 'Z'");
    TEST_ASSERT(l_toupper('m') == 'M', "toupper 'm' -> 'M'");
    TEST_ASSERT(l_toupper('A') == 'A', "toupper 'A' unchanged");
    TEST_ASSERT(l_toupper('0') == '0', "toupper '0' unchanged");
    TEST_ASSERT(l_toupper(' ') == ' ', "toupper space unchanged");

    /* l_tolower */
    TEST_ASSERT(l_tolower('A') == 'a', "tolower 'A' -> 'a'");
    TEST_ASSERT(l_tolower('Z') == 'z', "tolower 'Z' -> 'z'");
    TEST_ASSERT(l_tolower('M') == 'm', "tolower 'M' -> 'm'");
    TEST_ASSERT(l_tolower('a') == 'a', "tolower 'a' unchanged");
    TEST_ASSERT(l_tolower('0') == '0', "tolower '0' unchanged");
    TEST_ASSERT(l_tolower(' ') == ' ', "tolower space unchanged");

    TEST_SECTION_PASS("l_char_classification");
}

// ===================== l_atoi / l_atol =====================

void test_atoi_atol(void) {
    TEST_FUNCTION("l_atoi / l_atol");

    TEST_ASSERT(l_atoi("0") == 0, "atoi '0'");
    TEST_ASSERT(l_atoi("1") == 1, "atoi '1'");
    TEST_ASSERT(l_atoi("-1") == -1, "atoi '-1'");
    TEST_ASSERT(l_atoi("42") == 42, "atoi '42'");
    TEST_ASSERT(l_atoi("-42") == -42, "atoi '-42'");
    TEST_ASSERT(l_atoi("123") == 123, "atoi '123'");
    TEST_ASSERT(l_atoi("-456") == -456, "atoi '-456'");
    TEST_ASSERT(l_atoi("007") == 7, "atoi leading zeros");
    TEST_ASSERT(l_atoi("00") == 0, "atoi '00'");
    TEST_ASSERT(l_atoi("12345") == 12345, "atoi '12345'");
    TEST_ASSERT(l_atoi("-99999") == -99999, "atoi '-99999'");
    TEST_ASSERT(l_atoi("2147483647") == 2147483647, "atoi INT_MAX");
    TEST_ASSERT(l_atoi("123abc") == 123, "atoi stops at non-digit");
    TEST_ASSERT(l_atoi("0x10") == 0, "atoi stops at 'x'");

    TEST_ASSERT(l_atol("0") == 0L, "atol '0'");
    TEST_ASSERT(l_atol("-1") == -1L, "atol '-1'");
    TEST_ASSERT(l_atol("123") == 123L, "atol '123'");
    TEST_ASSERT(l_atol("-456") == -456L, "atol '-456'");
    TEST_ASSERT(l_atol("999999") == 999999L, "atol '999999'");
    TEST_ASSERT(l_atol("100000") == 100000L, "atol '100000'");
    TEST_ASSERT(l_atol("999999999") == 999999999L, "atol large value");

    /* whitespace skipping (C standard compliance) */
    TEST_ASSERT(l_atoi("  42") == 42, "atoi skips leading spaces");
    TEST_ASSERT(l_atoi("\t-7") == -7, "atoi skips leading tab");
    TEST_ASSERT(l_atoi("\n 123") == 123, "atoi skips newline+space");
    TEST_ASSERT(l_atoi("+5") == 5, "atoi handles leading plus");
    TEST_ASSERT(l_atoi("  +99") == 99, "atoi whitespace then plus");
    TEST_ASSERT(l_atol("  -456") == -456L, "atol skips leading spaces");
    TEST_ASSERT(l_atol("  +999") == 999L, "atol handles leading plus");

    TEST_SECTION_PASS("l_atoi / l_atol");
}

// ===================== l_strtoul / l_strtol =====================

void test_strtoul_strtol(void) {
    char *ep;

    TEST_FUNCTION("l_strtoul / l_strtol");

    /* strtoul: basic decimal */
    TEST_ASSERT(l_strtoul("0",   NULL, 10) == 0UL,  "strtoul '0' base 10");
    TEST_ASSERT(l_strtoul("123", NULL, 10) == 123UL, "strtoul '123' base 10");
    TEST_ASSERT(l_strtoul("  42", NULL, 10) == 42UL, "strtoul leading spaces");
    TEST_ASSERT(l_strtoul("+99", NULL, 10) == 99UL,  "strtoul leading plus");

    /* strtoul: hex */
    TEST_ASSERT(l_strtoul("ff",   NULL, 16) == 255UL, "strtoul 'ff' base 16");
    TEST_ASSERT(l_strtoul("FF",   NULL, 16) == 255UL, "strtoul 'FF' base 16");
    TEST_ASSERT(l_strtoul("0xff", NULL,  0) == 255UL, "strtoul '0xff' base 0");
    TEST_ASSERT(l_strtoul("0XFF", NULL,  0) == 255UL, "strtoul '0XFF' base 0");

    /* strtoul: octal */
    TEST_ASSERT(l_strtoul("010",  NULL,  0) == 8UL,  "strtoul '010' base 0 -> octal");
    TEST_ASSERT(l_strtoul("10",   NULL,  8) == 8UL,  "strtoul '10' base 8");

    /* strtoul: endptr */
    ep = NULL;
    l_strtoul("123abc", &ep, 10);
    TEST_ASSERT(ep != NULL && ep[0] == 'a', "strtoul endptr stops at non-digit");

    ep = NULL;
    l_strtoul("no-digits", &ep, 10);
    TEST_ASSERT(ep != NULL && ep[0] == 'n', "strtoul endptr stays at start when no digits");

    /* strtoul: overflow -> ULONG_MAX */
    TEST_ASSERT(l_strtoul("99999999999999999999999", NULL, 10) == ULONG_MAX, "strtoul overflow -> ULONG_MAX");

    /* strtol: positive */
    TEST_ASSERT(l_strtol("42",  NULL, 10) == 42L,  "strtol '42'");
    TEST_ASSERT(l_strtol(" 42", NULL, 10) == 42L,  "strtol leading space");
    TEST_ASSERT(l_strtol("+7",  NULL, 10) == 7L,   "strtol leading plus");

    /* strtol: negative */
    TEST_ASSERT(l_strtol("-1",   NULL, 10) == -1L,   "strtol '-1'");
    TEST_ASSERT(l_strtol("-123", NULL, 10) == -123L, "strtol '-123'");

    /* strtol: hex / octal via base 0 */
    TEST_ASSERT(l_strtol("0x10", NULL, 0) == 16L,  "strtol '0x10' base 0");
    TEST_ASSERT(l_strtol("010",  NULL, 0) == 8L,   "strtol '010' base 0 -> octal");
    TEST_ASSERT(l_strtol("-0x1", NULL, 0) == -1L,  "strtol '-0x1' base 0");

    /* strtol: overflow/underflow */
    TEST_ASSERT(l_strtol("99999999999999999999999",  NULL, 10) == LONG_MAX, "strtol overflow -> LONG_MAX");
    TEST_ASSERT(l_strtol("-99999999999999999999999", NULL, 10) == LONG_MIN, "strtol underflow -> LONG_MIN");

    /* strtol: endptr */
    ep = NULL;
    l_strtol("-99xyz", &ep, 10);
    TEST_ASSERT(ep != NULL && ep[0] == 'x', "strtol endptr after digits");

    TEST_SECTION_PASS("l_strtoul / l_strtol");
}

// ===================== l_strtoull / l_strtoll =====================

void test_strtoull_strtoll(void) {
    char *ep;

    TEST_FUNCTION("l_strtoull / l_strtoll");

    /* strtoull: basic decimal */
    TEST_ASSERT(l_strtoull("0",   NULL, 10) == 0ULL,  "strtoull '0' base 10");
    TEST_ASSERT(l_strtoull("123", NULL, 10) == 123ULL, "strtoull '123' base 10");
    TEST_ASSERT(l_strtoull("  42", NULL, 10) == 42ULL, "strtoull leading spaces");
    TEST_ASSERT(l_strtoull("+99", NULL, 10) == 99ULL,  "strtoull leading plus");

    /* strtoull: hex */
    TEST_ASSERT(l_strtoull("ff",   NULL, 16) == 255ULL, "strtoull 'ff' base 16");
    TEST_ASSERT(l_strtoull("0xff", NULL,  0) == 255ULL, "strtoull '0xff' base 0");
    TEST_ASSERT(l_strtoull("0XFF", NULL,  0) == 255ULL, "strtoull '0XFF' base 0");

    /* strtoull: 64-bit values beyond ULONG_MAX on 32-bit platforms */
    TEST_ASSERT(l_strtoull("4294967296", NULL, 10) == 4294967296ULL, "strtoull 2^32");
    TEST_ASSERT(l_strtoull("18446744073709551615", NULL, 10) == 18446744073709551615ULL,
                "strtoull ULLONG_MAX");

    /* strtoull: overflow -> ULLONG_MAX */
    TEST_ASSERT(l_strtoull("99999999999999999999999999", NULL, 10) == 18446744073709551615ULL,
                "strtoull overflow -> ULLONG_MAX");

    /* strtoull: endptr */
    ep = NULL;
    l_strtoull("123abc", &ep, 10);
    TEST_ASSERT(ep != NULL && ep[0] == 'a', "strtoull endptr stops at non-digit");

    ep = NULL;
    l_strtoull("no-digits", &ep, 10);
    TEST_ASSERT(ep != NULL && ep[0] == 'n', "strtoull endptr stays at start when no digits");

    /* strtoll: positive */
    TEST_ASSERT(l_strtoll("42",  NULL, 10) == 42LL,  "strtoll '42'");
    TEST_ASSERT(l_strtoll(" 42", NULL, 10) == 42LL,  "strtoll leading space");
    TEST_ASSERT(l_strtoll("+7",  NULL, 10) == 7LL,   "strtoll leading plus");

    /* strtoll: negative */
    TEST_ASSERT(l_strtoll("-1",   NULL, 10) == -1LL,   "strtoll '-1'");
    TEST_ASSERT(l_strtoll("-123", NULL, 10) == -123LL, "strtoll '-123'");

    /* strtoll: 64-bit values */
    TEST_ASSERT(l_strtoll("9223372036854775807",  NULL, 10) == 9223372036854775807LL,
                "strtoll LLONG_MAX");
    TEST_ASSERT(l_strtoll("-9223372036854775808", NULL, 10) == (-9223372036854775807LL - 1LL),
                "strtoll LLONG_MIN");

    /* strtoll: overflow/underflow */
    TEST_ASSERT(l_strtoll("99999999999999999999999",  NULL, 10) == 9223372036854775807LL,
                "strtoll overflow -> LLONG_MAX");
    TEST_ASSERT(l_strtoll("-99999999999999999999999", NULL, 10) == (-9223372036854775807LL - 1LL),
                "strtoll underflow -> LLONG_MIN");

    /* strtoll: hex / octal via base 0 */
    TEST_ASSERT(l_strtoll("0x10", NULL, 0) == 16LL, "strtoll '0x10' base 0");
    TEST_ASSERT(l_strtoll("010",  NULL, 0) == 8LL,  "strtoll '010' base 0 -> octal");

    /* strtoll: endptr */
    ep = NULL;
    l_strtoll("-99xyz", &ep, 10);
    TEST_ASSERT(ep != NULL && ep[0] == 'x', "strtoll endptr after digits");

    TEST_SECTION_PASS("l_strtoull / l_strtoll");
}

// ===================== l_strtod / l_atof =====================

void test_strtod_atof(void) {
    char *ep;

    TEST_FUNCTION("l_strtod / l_atof");

    /* Basic positive */
    TEST_ASSERT(l_atof("0") == 0.0, "atof '0'");
    TEST_ASSERT(l_atof("1") == 1.0, "atof '1'");
    TEST_ASSERT(l_atof("3.14") > 3.13 && l_atof("3.14") < 3.15, "atof '3.14'");
    TEST_ASSERT(l_atof("100.0") == 100.0, "atof '100.0'");

    /* Negative */
    TEST_ASSERT(l_atof("-1") == -1.0, "atof '-1'");
    TEST_ASSERT(l_atof("-3.14") < -3.13 && l_atof("-3.14") > -3.15, "atof '-3.14'");

    /* Leading sign */
    TEST_ASSERT(l_atof("+2.5") == 2.5, "atof '+2.5'");

    /* Leading whitespace */
    TEST_ASSERT(l_atof("  42.0") == 42.0, "atof leading spaces");

    /* Exponent */
    TEST_ASSERT(l_atof("1e3") == 1000.0, "atof '1e3'");
    TEST_ASSERT(l_atof("1.5e2") == 150.0, "atof '1.5e2'");
    TEST_ASSERT(l_atof("1E-1") > 0.09 && l_atof("1E-1") < 0.11, "atof '1E-1'");
    TEST_ASSERT(l_atof("2.5e+1") == 25.0, "atof '2.5e+1'");
    TEST_ASSERT(l_atof("-1e2") == -100.0, "atof '-1e2'");

    /* Integer-like */
    TEST_ASSERT(l_atof("255") == 255.0, "atof '255'");
    TEST_ASSERT(l_atof("0.5") == 0.5, "atof '0.5'");
    TEST_ASSERT(l_atof(".5") == 0.5, "atof '.5' (no leading digit)");

    /* endptr */
    ep = (char *)0;
    l_strtod("3.14xyz", &ep);
    TEST_ASSERT(ep != (char *)0 && ep[0] == 'x', "strtod endptr stops at non-numeric");

    ep = (char *)0;
    l_strtod("  42 rest", &ep);
    TEST_ASSERT(ep != (char *)0 && ep[0] == ' ', "strtod endptr after number");

    ep = (char *)0;
    l_strtod("nope", &ep);
    TEST_ASSERT(ep != (char *)0 && ep[0] == 'n', "strtod no digits -> endptr at start");

    /* Special: inf */
    double inf_val = l_atof("inf");
    TEST_ASSERT(inf_val > DBL_MAX, "atof 'inf' is infinity");
    double ninf_val = l_atof("-inf");
    TEST_ASSERT(ninf_val < -DBL_MAX, "atof '-inf' is negative infinity");

    /* Special: nan */
    double nan_val = l_atof("nan");
    TEST_ASSERT(nan_val != nan_val, "atof 'nan' is NaN");

    TEST_SECTION_PASS("l_strtod / l_atof");
}

// ===================== l_strtof =====================

void test_strtof(void) {
    char *ep;

    TEST_FUNCTION("l_strtof");

    /* Basic round-trips — use tolerances appropriate for float precision */
    TEST_ASSERT(l_strtof("0", (char **)0) == 0.0f, "strtof '0'");
    TEST_ASSERT(l_strtof("1", (char **)0) == 1.0f, "strtof '1'");
    TEST_ASSERT(l_strtof("3.14", (char **)0) > 3.13f && l_strtof("3.14", (char **)0) < 3.15f, "strtof '3.14'");
    TEST_ASSERT(l_strtof("-1.5", (char **)0) == -1.5f, "strtof '-1.5'");
    TEST_ASSERT(l_strtof("1e3", (char **)0) == 1000.0f, "strtof '1e3'");
    TEST_ASSERT(l_strtof("+0.5", (char **)0) == 0.5f, "strtof '+0.5'");

    /* endptr */
    ep = (char *)0;
    l_strtof("2.5xyz", &ep);
    TEST_ASSERT(ep != (char *)0 && ep[0] == 'x', "strtof endptr stops at non-numeric");

    /* Result is a float (single-precision) */
    float fv = l_strtof("100.0", (char **)0);
    TEST_ASSERT(fv == 100.0f, "strtof returns float 100.0");

    TEST_SECTION_PASS("l_strtof");
}

void test_itoa(void) {
    TEST_FUNCTION("l_itoa");

    char buf[64];

    // Decimal
    l_itoa(0, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "0", 1) == 0, "itoa 0 decimal");
    TEST_ASSERT(buf[1] == '\0', "itoa 0 null-terminated");
    l_itoa(1, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "1", 1) == 0, "itoa 1");
    l_itoa(9, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "9", 1) == 0, "itoa single digit");
    l_itoa(10, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "10", 2) == 0, "itoa 10");
    l_itoa(123, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "123", 3) == 0, "itoa 123");
    l_itoa(12345, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "12345", 5) == 0, "itoa 12345");
    l_itoa(-1, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "-1", 2) == 0, "itoa -1");
    l_itoa(-456, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "-456", 4) == 0, "itoa -456");
    l_itoa(-999, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "-999", 4) == 0, "itoa -999");
    // INT_MIN must not overflow: -(-2147483648) wraps in signed int
    l_itoa(-2147483647 - 1, buf, 10);
    TEST_ASSERT(l_strcmp(buf, "-2147483648") == 0, "itoa INT_MIN");

    // Hex
    l_itoa(0, buf, 16);
    TEST_ASSERT(l_strncmp(buf, "0", 1) == 0, "itoa 0 hex");
    l_itoa(15, buf, 16);
    TEST_ASSERT(l_strncmp(buf, "f", 1) == 0, "itoa 15 hex is 'f'");
    l_itoa(16, buf, 16);
    TEST_ASSERT(l_strncmp(buf, "10", 2) == 0, "itoa 16 hex is '10'");
    l_itoa(255, buf, 16);
    TEST_ASSERT(l_strncmp(buf, "ff", 2) == 0, "itoa 255 hex is 'ff'");
    l_itoa(256, buf, 16);
    TEST_ASSERT(l_strncmp(buf, "100", 3) == 0, "itoa 256 hex is '100'");
    l_itoa(4096, buf, 16);
    TEST_ASSERT(l_strncmp(buf, "1000", 4) == 0, "itoa 4096 hex is '1000'");

    // Octal
    l_itoa(8, buf, 8);
    TEST_ASSERT(l_strncmp(buf, "10", 2) == 0, "itoa 8 octal is '10'");
    l_itoa(64, buf, 8);
    TEST_ASSERT(l_strncmp(buf, "100", 3) == 0, "itoa 64 octal is '100'");

    // Binary
    l_itoa(0, buf, 2);
    TEST_ASSERT(l_strncmp(buf, "0", 1) == 0, "itoa 0 binary");
    l_itoa(1, buf, 2);
    TEST_ASSERT(l_strncmp(buf, "1", 1) == 0, "itoa 1 binary");
    l_itoa(5, buf, 2);
    TEST_ASSERT(l_strncmp(buf, "101", 3) == 0, "itoa 5 binary is '101'");
    l_itoa(255, buf, 2);
    TEST_ASSERT(l_strncmp(buf, "11111111", 8) == 0, "itoa 255 binary");

    TEST_SECTION_PASS("l_itoa");
}

// ===================== Roundtrip: itoa -> atoi =====================

void test_itoa_atoi_roundtrip(void) {
    TEST_FUNCTION("itoa/atoi roundtrip");

    char buf[32];
    int values[] = {0, 1, -1, 42, -42, 100, -100, 9999, -9999, 2147483, -2147483, -2147483647 - 1};
    int n = sizeof(values) / sizeof(values[0]);

    for (int i = 0; i < n; i++) {
        l_itoa(values[i], buf, 10);
        int back = l_atoi(buf);
        if (back != values[i]) {
            TEST_ASSERT(0, "roundtrip failed");
        }
    }
    TEST_ASSERT(1, "all roundtrip conversions match");

    TEST_SECTION_PASS("itoa/atoi roundtrip");
}

// ===================== l_memcmp =====================

void test_memcmp(void) {
    TEST_FUNCTION("l_memcmp");

    TEST_ASSERT(l_memcmp("abc", "abc", 3) == 0, "equal buffers");
    TEST_ASSERT(l_memcmp("abc", "abd", 3) < 0, "'abc' < 'abd'");
    TEST_ASSERT(l_memcmp("abd", "abc", 3) > 0, "'abd' > 'abc'");
    TEST_ASSERT(l_memcmp("abc", "xyz", 0) == 0, "zero length always equal");

    char a[] = {1, 0, 3};
    char b[] = {1, 0, 3};
    char c[] = {1, 0, 4};
    TEST_ASSERT(l_memcmp(a, b, 3) == 0, "equal with embedded zeros");
    TEST_ASSERT(l_memcmp(a, c, 3) < 0, "differ after embedded zero");

    TEST_ASSERT(l_memcmp("x", "x", 1) == 0, "single byte equal");
    TEST_ASSERT(l_memcmp("a", "b", 1) < 0, "single byte less");

    TEST_SECTION_PASS("l_memcmp");
}

// ===================== l_memcpy =====================

void test_memcpy(void) {
    TEST_FUNCTION("l_memcpy");

    // Return value
    char copy_src[] = "Test";
    char copy_dst[10];
    void* copy_result = l_memcpy(copy_dst, copy_src, 4);
    TEST_ASSERT(copy_result == copy_dst, "returns destination pointer");
    TEST_ASSERT(l_memcmp(copy_dst, copy_src, 4) == 0, "copies data correctly");

    // Zero-length copy
    char dst[32] = "original";
    l_memcpy(dst, "NEW", 0);
    TEST_ASSERT(l_strncmp(dst, "original", 8) == 0, "zero-length copy leaves dst unchanged");

    // Copy with embedded null bytes
    char src[] = {1, 0, 2, 0, 3};
    char buf[5];
    l_memcpy(buf, src, 5);
    TEST_ASSERT(l_memcmp(buf, src, 5) == 0, "copy with embedded nulls");

    // Large-ish copy
    char big_src[128];
    char big_dst[128];
    l_memset(big_src, 'A', 128);
    l_memcpy(big_dst, big_src, 128);
    TEST_ASSERT(l_memcmp(big_dst, big_src, 128) == 0, "128-byte copy");

    // Single byte
    char one = 'Z';
    char out;
    l_memcpy(&out, &one, 1);
    TEST_ASSERT(out == 'Z', "single byte copy");

    // Alignment boundary: vary start offset of both src and dst to exercise
    // the word-at-a-time and tail phases of the optimised implementation.
    {
        char abuf_src[256];
        char abuf_dst[256];
        int i, ok;
        l_memset(abuf_src, 0, sizeof(abuf_src));
        for (i = 0; i < 128; i++) abuf_src[i] = (char)(i + 1);
        for (i = 0; i < 8; i++) {
            l_memset(abuf_dst, 0, sizeof(abuf_dst));
            l_memcpy(abuf_dst + i, abuf_src + i, 128);
            ok = (l_memcmp(abuf_dst + i, abuf_src + i, 128) == 0);
            TEST_ASSERT(ok, "word-at-a-time: memcpy correct at alignment offsets");
        }
    }

    TEST_SECTION_PASS("l_memcpy");
}

// ===================== l_memset =====================

void test_memset(void) {
    TEST_FUNCTION("l_memset");

    // Return value
    char mem_buf[10];
    void* result = l_memset(mem_buf, 'A', 5);
    TEST_ASSERT(result == mem_buf, "returns destination pointer");
    TEST_ASSERT(mem_buf[0] == 'A', "sets first byte");
    TEST_ASSERT(mem_buf[4] == 'A', "sets last byte");

    // Zero length
    char buf[16] = "Hello";
    l_memset(buf, 'X', 0);
    TEST_ASSERT(buf[0] == 'H', "zero-length memset leaves buffer unchanged");

    // Fill with zero
    char zbuf[8];
    l_memset(zbuf, 0, 8);
    for (int i = 0; i < 8; i++) {
        if (zbuf[i] != 0) {
            TEST_ASSERT(0, "memset with zero byte");
        }
    }
    TEST_ASSERT(1, "memset with zero fills all bytes");

    // Fill with 0xFF
    unsigned char fbuf[4];
    l_memset(fbuf, 0xFF, 4);
    TEST_ASSERT(fbuf[0] == 0xFF && fbuf[3] == 0xFF, "memset with 0xFF");

    // Single byte
    char single = 'A';
    l_memset(&single, 'B', 1);
    TEST_ASSERT(single == 'B', "single byte memset");

    // Large fill
    char large[256];
    l_memset(large, 'Q', 256);
    TEST_ASSERT(large[0] == 'Q' && large[127] == 'Q' && large[255] == 'Q',
                "256-byte memset consistent");

    // Alignment boundary: start at various unaligned offsets to exercise
    // head/word-at-a-time/tail phases of the optimised implementation.
    {
        char abuf[128];
        int i, j, ok;
        for (i = 0; i < 8; i++) {
            l_memset(abuf, 0, sizeof(abuf));
            l_memset(abuf + i, 'Z', 64);
            ok = 1;
            for (j = 0; j < i; j++) {
                if (abuf[j] != 0) { ok = 0; break; }
            }
            for (j = i; j < i + 64; j++) {
                if (abuf[j] != 'Z') { ok = 0; break; }
            }
            for (j = i + 64; j < 128; j++) {
                if (abuf[j] != 0) { ok = 0; break; }
            }
            TEST_ASSERT(ok, "aligned/unaligned memset fills exact range");
        }
    }

    TEST_SECTION_PASS("l_memset");
}

// ===================== l_memmove =====================

void test_memmove(void) {
    TEST_FUNCTION("l_memmove");

    // Return value
    char move_src[] = "Hello";
    char move_dst[10];
    void* move_result = l_memmove(move_dst, move_src, 5);
    TEST_ASSERT(move_result == move_dst, "returns destination pointer");
    TEST_ASSERT(l_memcmp(move_dst, move_src, 5) == 0, "copies data correctly");

    // Overlapping: dst after src
    char overlap[] = "Hello World";
    l_memmove(overlap + 2, overlap, 5);
    TEST_ASSERT(l_memcmp(overlap + 2, "Hello", 5) == 0, "overlap: dst after src");

    // Overlapping: src after dst
    char overlap1[] = "123456789";
    l_memmove(overlap1, overlap1 + 2, 5);
    TEST_ASSERT(l_memcmp(overlap1, "34567", 5) == 0, "overlap: src after dst");

    // Zero length
    char zero[] = "unchanged";
    l_memmove(zero, "XXX", 0);
    TEST_ASSERT(l_strncmp(zero, "unchanged", 9) == 0, "zero-length memmove");

    // Single byte
    char sa = 'A', sb = 'B';
    l_memmove(&sa, &sb, 1);
    TEST_ASSERT(sa == 'B', "single byte memmove");

    TEST_SECTION_PASS("l_memmove");
}

// ===================== l_wcslen =====================

void test_wcslen(void) {
    TEST_FUNCTION("l_wcslen");

    TEST_ASSERT(l_wcslen(L"") == 0, "empty wide string has length 0");
    TEST_ASSERT(l_wcslen(L"Hello") == 5, "L\"Hello\" has length 5");
    TEST_ASSERT(l_wcslen(L"a") == 1, "single wide char has length 1");
    TEST_ASSERT(l_wcslen(L"Hello World") == 11, "L\"Hello World\" has length 11");

    const wchar_t ws[] = {L'A', L'B', L'C', L'\0'};
    TEST_ASSERT(l_wcslen(ws) == 3, "manually constructed wide string has length 3");

    TEST_SECTION_PASS("l_wcslen");
}

// ===================== Basic file I/O =====================

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

// ===================== File operations =====================

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

// ===================== System functions =====================

void test_system_functions(void) {
    TEST_FUNCTION("System Functions");

    l_exitif(0, -1, "This should not exit");
    TEST_ASSERT(1, "l_exitif with false condition does not exit");

    L_FD open_fd = l_open("test_explicit_open", O_CREAT | O_WRONLY, 0644);
    TEST_ASSERT(open_fd >= 0, "l_open with explicit flags works");
    l_close(open_fd);

    TEST_SECTION_PASS("System functions");
}

// ===================== l_getenv =====================

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

// ===================== l_env_iter =====================

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

// ===================== l_lseek =====================

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

// ===================== l_dup =====================

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

// ===================== l_mkdir =====================

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

// ===================== l_open_append =====================

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

// ===================== l_sleep_ms =====================

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

// ===================== l_sched_yield =====================

void test_sched_yield(void) {
    TEST_FUNCTION("l_sched_yield");

    int ret = l_sched_yield();
    TEST_ASSERT(ret == 0, "l_sched_yield returns 0 on success");

    TEST_SECTION_PASS("l_sched_yield");
}

// ===================== l_memchr =====================

void test_memchr(void) {
    TEST_FUNCTION("l_memchr");

    const char *s = "Hello, World!";
    TEST_ASSERT(l_memchr(s, 'H', 13) == s,       "find at start");
    TEST_ASSERT(l_memchr(s, '!', 13) == s + 12,  "find at end");
    TEST_ASSERT(l_memchr(s, 'o', 13) == s + 4,   "find first of two");
    TEST_ASSERT(l_memchr(s, 'z', 13) == NULL,     "not found returns NULL");
    TEST_ASSERT(l_memchr(s, 'H', 0)  == NULL,     "n=0 always returns NULL");

    /* Byte value 0 must be findable (memchr works on raw bytes). */
    char buf[] = {1, 2, 0, 3, 4};
    TEST_ASSERT(l_memchr(buf, 0, 5) == buf + 2, "finds embedded null byte");
    TEST_ASSERT(l_memchr(buf, 0, 2) == NULL,    "null past n not found");

    /* 0xFF / negative value cast to unsigned char */
    char fbuf[] = {0x01, (char)0xFF, 0x02};
    TEST_ASSERT(l_memchr(fbuf, 0xFF, 3) == fbuf + 1, "finds 0xFF byte");

    /* Return type is void*: cast back to char* for pointer arithmetic */
    const char *hay = "abcabc";
    char *found = (char *)l_memchr(hay, 'b', 6);
    TEST_ASSERT(found == hay + 1, "returns pointer to first match");

    TEST_SECTION_PASS("l_memchr");
}

// ===================== l_memrchr =====================

void test_memrchr(void) {
    TEST_FUNCTION("l_memrchr");

    const char *s = "Hello, World!";  /* len 13 */
    TEST_ASSERT(l_memrchr(s, 'l', 13) == s + 10, "find last of three");
    TEST_ASSERT(l_memrchr(s, 'H', 13) == s,       "only occurrence at start");
    TEST_ASSERT(l_memrchr(s, '!', 13) == s + 12,  "only occurrence at end");
    TEST_ASSERT(l_memrchr(s, 'z', 13) == NULL,    "not found returns NULL");
    TEST_ASSERT(l_memrchr(s, 'H', 0)  == NULL,    "n=0 always returns NULL");

    /* embedded null byte */
    unsigned char buf[] = { 0xAA, 0xBB, 0x00, 0xCC, 0x00 };
    TEST_ASSERT(l_memrchr(buf, 0, 5) == (void *)(buf + 4), "finds last null byte");
    TEST_ASSERT(l_memrchr(buf, 0, 3) == (void *)(buf + 2), "finds first null in limited range");

    TEST_SECTION_PASS("l_memrchr");
}

// ===================== l_strnlen =====================

void test_strnlen(void) {
    TEST_FUNCTION("l_strnlen");

    TEST_ASSERT(l_strnlen("", 0)    == 0, "empty string, maxlen 0");
    TEST_ASSERT(l_strnlen("", 10)   == 0, "empty string, large maxlen");
    TEST_ASSERT(l_strnlen("Hello", 10) == 5, "short string within maxlen");
    TEST_ASSERT(l_strnlen("Hello", 5)  == 5, "maxlen equals length");
    TEST_ASSERT(l_strnlen("Hello", 3)  == 3, "maxlen truncates result");
    TEST_ASSERT(l_strnlen("Hello", 0)  == 0, "maxlen 0 returns 0");

    /* string with embedded null — should stop at maxlen, not the null beyond maxlen */
    const char buf[] = { 'a', 'b', 'c', '\0', 'd' };
    TEST_ASSERT(l_strnlen(buf, 5) == 3, "stops at null before maxlen");
    TEST_ASSERT(l_strnlen(buf, 2) == 2, "maxlen before null limits result");

    TEST_SECTION_PASS("l_strnlen");
}

// ===================== l_memmem =====================

void test_memmem(void) {
    TEST_FUNCTION("l_memmem");

    /* Basic found case */
    const char hay[] = "Hello, World!";
    TEST_ASSERT(l_memmem(hay, 13, "World", 5) == hay + 7, "finds needle in haystack");
    TEST_ASSERT(l_memmem(hay, 13, "Hello", 5) == hay,     "finds needle at start");
    TEST_ASSERT(l_memmem(hay, 13, "!",    1) == hay + 12, "finds single byte at end");

    /* Not found */
    TEST_ASSERT(l_memmem(hay, 13, "xyz", 3) == NULL, "needle not present returns NULL");

    /* Empty needle returns haystack */
    TEST_ASSERT(l_memmem(hay, 13, "", 0) == hay, "empty needle returns haystack pointer");

    /* Needle longer than haystack */
    TEST_ASSERT(l_memmem("ab", 2, "abc", 3) == NULL, "needle longer than haystack returns NULL");

    /* Binary data with embedded nulls */
    const char bin[] = {0x01, 0x00, 0x02, 0x03, 0x00, 0x04};
    TEST_ASSERT(l_memmem(bin, 6, "\x02\x03", 2) == bin + 2, "finds binary sequence");
    TEST_ASSERT(l_memmem(bin, 6, "\x00\x04", 2) == bin + 4, "finds sequence ending with last byte");

    /* Multiple occurrences: first one is returned */
    const char multi[] = "abcabc";
    TEST_ASSERT(l_memmem(multi, 6, "abc", 3) == multi, "returns first of multiple occurrences");

    /* Haystack length of 0 */
    TEST_ASSERT(l_memmem("x", 0, "x", 1) == NULL, "haystacklen=0 returns NULL");

    TEST_SECTION_PASS("l_memmem");
}

void test_snprintf(void) {
    TEST_FUNCTION("l_snprintf");
    char buf[128];

    /* Basic string output */
    l_snprintf(buf, sizeof(buf), "hello");
    TEST_ASSERT(l_strcmp(buf, "hello") == 0, "plain string");

    /* %s */
    l_snprintf(buf, sizeof(buf), "<%s>", "world");
    TEST_ASSERT(l_strcmp(buf, "<world>") == 0, "%s basic");

    /* %s with NULL */
    l_snprintf(buf, sizeof(buf), "%s", (char *)NULL);
    TEST_ASSERT(l_strcmp(buf, "(null)") == 0, "%s null");

    /* %s with width (right-align) */
    l_snprintf(buf, sizeof(buf), "%8s", "abc");
    TEST_ASSERT(l_strcmp(buf, "     abc") == 0, "%8s right-align");

    /* %s with width (left-align) */
    l_snprintf(buf, sizeof(buf), "%-8s|", "abc");
    TEST_ASSERT(l_strcmp(buf, "abc     |") == 0, "%-8s left-align");

    /* %s with precision */
    l_snprintf(buf, sizeof(buf), "%.3s", "hello");
    TEST_ASSERT(l_strcmp(buf, "hel") == 0, "%.3s truncation");

    /* %c */
    l_snprintf(buf, sizeof(buf), "%c", 'A');
    TEST_ASSERT(l_strcmp(buf, "A") == 0, "%c basic");

    /* %d basic */
    l_snprintf(buf, sizeof(buf), "%d", 42);
    TEST_ASSERT(l_strcmp(buf, "42") == 0, "%d positive");

    l_snprintf(buf, sizeof(buf), "%d", -7);
    TEST_ASSERT(l_strcmp(buf, "-7") == 0, "%d negative");

    l_snprintf(buf, sizeof(buf), "%d", 0);
    TEST_ASSERT(l_strcmp(buf, "0") == 0, "%d zero");

    /* %d with width */
    l_snprintf(buf, sizeof(buf), "%5d", 42);
    TEST_ASSERT(l_strcmp(buf, "   42") == 0, "%5d right-align");

    /* %d left-align */
    l_snprintf(buf, sizeof(buf), "%-5d|", 42);
    TEST_ASSERT(l_strcmp(buf, "42   |") == 0, "%-5d left-align");

    /* %d zero-pad */
    l_snprintf(buf, sizeof(buf), "%05d", 42);
    TEST_ASSERT(l_strcmp(buf, "00042") == 0, "%05d zero-pad");

    /* %u */
    l_snprintf(buf, sizeof(buf), "%u", 255u);
    TEST_ASSERT(l_strcmp(buf, "255") == 0, "%u basic");

    /* %x lower hex */
    l_snprintf(buf, sizeof(buf), "%x", 255u);
    TEST_ASSERT(l_strcmp(buf, "ff") == 0, "%x lower hex");

    /* %X upper hex */
    l_snprintf(buf, sizeof(buf), "%X", 255u);
    TEST_ASSERT(l_strcmp(buf, "FF") == 0, "%X upper hex");

    /* %08x */
    l_snprintf(buf, sizeof(buf), "%08x", 0xABu);
    TEST_ASSERT(l_strcmp(buf, "000000ab") == 0, "%08x zero-pad hex");

    /* %% literal percent */
    l_snprintf(buf, sizeof(buf), "100%%");
    TEST_ASSERT(l_strcmp(buf, "100%") == 0, "%% literal");

    /* %ld long */
    l_snprintf(buf, sizeof(buf), "%ld", (long)-100000L);
    TEST_ASSERT(l_strcmp(buf, "-100000") == 0, "%ld long negative");

    /* %lu unsigned long */
    l_snprintf(buf, sizeof(buf), "%lu", (unsigned long)999999UL);
    TEST_ASSERT(l_strcmp(buf, "999999") == 0, "%lu unsigned long");

    /* buffer truncation: return value = total chars that would be written */
    int ret = l_snprintf(buf, 4, "hello");
    TEST_ASSERT(ret == 5, "truncation: return value is full length");
    TEST_ASSERT(l_strcmp(buf, "hel") == 0, "truncation: buf holds first n-1 chars");

    /* n=0: no write, return value still correct */
    ret = l_snprintf(NULL, 0, "hi");
    TEST_ASSERT(ret == 2, "n=0: return value correct");

    /* Multiple format specifiers in one call */
    l_snprintf(buf, sizeof(buf), "%d + %d = %d", 1, 2, 3);
    TEST_ASSERT(l_strcmp(buf, "1 + 2 = 3") == 0, "multiple specifiers");

    TEST_SECTION_PASS("l_snprintf");
}

// ===================== l_snprintf float =====================

void test_snprintf_float(void) {
    TEST_FUNCTION("l_snprintf %%f/%%e/%%g");
    char buf[64];

    /* %f basic */
    l_snprintf(buf, sizeof(buf), "%f", 0.0);
    TEST_ASSERT(l_strcmp(buf, "0.000000") == 0, "%f 0.0");

    l_snprintf(buf, sizeof(buf), "%f", 1.0);
    TEST_ASSERT(l_strcmp(buf, "1.000000") == 0, "%f 1.0");

    l_snprintf(buf, sizeof(buf), "%f", -1.5);
    TEST_ASSERT(l_strcmp(buf, "-1.500000") == 0, "%f -1.5");

    l_snprintf(buf, sizeof(buf), "%.2f", 3.14159);
    TEST_ASSERT(l_strcmp(buf, "3.14") == 0, "%.2f 3.14159");

    l_snprintf(buf, sizeof(buf), "%.0f", 2.7);
    TEST_ASSERT(l_strcmp(buf, "3") == 0, "%.0f 2.7 rounds to 3");

    l_snprintf(buf, sizeof(buf), "%10.3f", 1.5);
    TEST_ASSERT(l_strcmp(buf, "     1.500") == 0, "%10.3f right-align");

    l_snprintf(buf, sizeof(buf), "%-10.3f|", 1.5);
    TEST_ASSERT(l_strcmp(buf, "1.500     |") == 0, "%-10.3f left-align");

    l_snprintf(buf, sizeof(buf), "%010.3f", 1.5);
    TEST_ASSERT(l_strcmp(buf, "000001.500") == 0, "%010.3f zero-pad");

    l_snprintf(buf, sizeof(buf), "%f", 100.0);
    TEST_ASSERT(l_strcmp(buf, "100.000000") == 0, "%f 100.0");

    /* %e basic */
    l_snprintf(buf, sizeof(buf), "%e", 0.0);
    TEST_ASSERT(l_strcmp(buf, "0.000000e+00") == 0, "%e 0.0");

    l_snprintf(buf, sizeof(buf), "%e", 1.0);
    TEST_ASSERT(l_strcmp(buf, "1.000000e+00") == 0, "%e 1.0");

    l_snprintf(buf, sizeof(buf), "%e", 1000.0);
    TEST_ASSERT(l_strcmp(buf, "1.000000e+03") == 0, "%e 1000.0");

    l_snprintf(buf, sizeof(buf), "%.2e", 3.14159);
    TEST_ASSERT(l_strcmp(buf, "3.14e+00") == 0, "%.2e 3.14159");

    l_snprintf(buf, sizeof(buf), "%e", -0.001);
    TEST_ASSERT(l_strcmp(buf, "-1.000000e-03") == 0, "%e -0.001");

    l_snprintf(buf, sizeof(buf), "%E", 1.5e10);
    TEST_ASSERT(l_strcmp(buf, "1.500000E+10") == 0, "%E uppercase");

    /* %g basic */
    l_snprintf(buf, sizeof(buf), "%g", 0.0);
    TEST_ASSERT(l_strcmp(buf, "0") == 0, "%g 0.0");

    l_snprintf(buf, sizeof(buf), "%g", 1.0);
    TEST_ASSERT(l_strcmp(buf, "1") == 0, "%g 1.0 strips trailing zeros");

    l_snprintf(buf, sizeof(buf), "%g", 3.14);
    TEST_ASSERT(l_strcmp(buf, "3.14") == 0, "%g 3.14");

    l_snprintf(buf, sizeof(buf), "%g", 1e6);
    TEST_ASSERT(l_strcmp(buf, "1e+06") == 0, "%g 1e6 uses scientific");

    l_snprintf(buf, sizeof(buf), "%g", 0.0001);
    TEST_ASSERT(l_strcmp(buf, "0.0001") == 0, "%g 0.0001 uses fixed");

    l_snprintf(buf, sizeof(buf), "%g", 0.00001);
    TEST_ASSERT(l_strcmp(buf, "1e-05") == 0, "%g 0.00001 uses scientific");

    l_snprintf(buf, sizeof(buf), "%.10g", 1.23456789);
    TEST_ASSERT(l_strncmp(buf, "1.23456789", 10) == 0, "%.10g precision");

    /* special values */
    double my_inf = l_atof("inf");
    l_snprintf(buf, sizeof(buf), "%f", my_inf);
    TEST_ASSERT(l_strcmp(buf, "inf") == 0, "%f inf");

    l_snprintf(buf, sizeof(buf), "%f", -my_inf);
    TEST_ASSERT(l_strcmp(buf, "-inf") == 0, "%f -inf");

    double my_nan = l_atof("nan");
    l_snprintf(buf, sizeof(buf), "%f", my_nan);
    TEST_ASSERT(l_strcmp(buf, "nan") == 0, "%f nan");

    TEST_SECTION_PASS("l_snprintf %%f/%%e/%%g");
}

void test_strcasecmp(void) {
    TEST_FUNCTION("l_strcasecmp / l_strncasecmp");

    TEST_ASSERT(l_strcasecmp("hello", "hello") == 0, "identical lowercase");
    TEST_ASSERT(l_strcasecmp("Hello", "hello") == 0, "mixed case equal");
    TEST_ASSERT(l_strcasecmp("HELLO", "hello") == 0, "all upper vs lower");
    TEST_ASSERT(l_strcasecmp("", "") == 0, "empty strings");
    TEST_ASSERT(l_strcasecmp("a", "b") < 0, "a < b");
    TEST_ASSERT(l_strcasecmp("b", "a") > 0, "b > a");
    TEST_ASSERT(l_strcasecmp("abc", "abd") < 0, "abc < abd");
    TEST_ASSERT(l_strcasecmp("abc", "ab") > 0, "longer > shorter prefix");
    TEST_ASSERT(l_strcasecmp("ab", "abc") < 0, "shorter prefix < longer");
    TEST_ASSERT(l_strcasecmp("123", "123") == 0, "digits equal");
    TEST_ASSERT(l_strcasecmp("Hello!", "hello!") == 0, "with punctuation");

    // l_strncasecmp
    TEST_ASSERT(l_strncasecmp("Hello", "hello", 5) == 0, "n=len equal");
    TEST_ASSERT(l_strncasecmp("Hello", "Help", 3) == 0, "n=3 first 3 match");
    TEST_ASSERT(l_strncasecmp("Hello", "Help", 4) != 0, "n=4 differ at 4th");
    TEST_ASSERT(l_strncasecmp("abc", "abd", 2) == 0, "n=2 first 2 match");
    TEST_ASSERT(l_strncasecmp("abc", "abd", 3) < 0, "n=3 c < d");
    TEST_ASSERT(l_strncasecmp("ABC", "abd", 0) == 0, "n=0 always equal");
    TEST_ASSERT(l_strncasecmp("", "", 5) == 0, "empty n=5");

    TEST_SECTION_PASS("l_strcasecmp / l_strncasecmp");
}

// ===================== l_strspn / l_strcspn =====================

void test_strspn(void) {
    TEST_FUNCTION("l_strspn / l_strcspn");

    // l_strspn
    TEST_ASSERT(l_strspn("abcdef", "abc") == 3, "first 3 in accept");
    TEST_ASSERT(l_strspn("abcdef", "fed") == 0, "first char not in accept");
    TEST_ASSERT(l_strspn("abcabc", "abc") == 6, "all chars in accept");
    TEST_ASSERT(l_strspn("", "abc") == 0, "empty string");
    TEST_ASSERT(l_strspn("abc", "") == 0, "empty accept");
    TEST_ASSERT(l_strspn("123abc", "321") == 3, "digits in accept");
    TEST_ASSERT(l_strspn("aaa", "a") == 3, "repeated single accept");

    // l_strcspn
    TEST_ASSERT(l_strcspn("abcdef", "de") == 3, "first reject at pos 3");
    TEST_ASSERT(l_strcspn("abcdef", "abc") == 0, "first char in reject");
    TEST_ASSERT(l_strcspn("abcdef", "xyz") == 6, "no reject chars found");
    TEST_ASSERT(l_strcspn("", "abc") == 0, "empty string");
    TEST_ASSERT(l_strcspn("abc", "") == 3, "empty reject");
    TEST_ASSERT(l_strcspn("hello world", " ") == 5, "space as reject");

    TEST_SECTION_PASS("l_strspn / l_strcspn");
}

// ===================== l_strpbrk =====================

void test_strpbrk(void) {
    TEST_FUNCTION("l_strpbrk");

    char *p;
    p = l_strpbrk("hello world", "aeiou");
    TEST_ASSERT(p != (char *)0 && *p == 'e', "first vowel in 'hello world' is 'e'");

    p = l_strpbrk("hello", "xyz");
    TEST_ASSERT(p == (char *)0, "no match returns NULL");

    p = l_strpbrk("", "abc");
    TEST_ASSERT(p == (char *)0, "empty string returns NULL");

    p = l_strpbrk("abc", "");
    TEST_ASSERT(p == (char *)0, "empty accept returns NULL");

    p = l_strpbrk("abcdef", "fed");
    TEST_ASSERT(p != (char *)0 && *p == 'd', "first of fed in abcdef is 'd'");

    p = l_strpbrk("abcdef", "a");
    TEST_ASSERT(p != (char *)0 && p[0] == 'a', "single char match at start");

    p = l_strpbrk("abcdef", "f");
    TEST_ASSERT(p != (char *)0 && *p == 'f', "single char match at end");

    TEST_SECTION_PASS("l_strpbrk");
}

// ===================== l_strtok_r =====================

void test_strtok_r(void) {
    TEST_FUNCTION("l_strtok_r");

    char buf[64];
    char *save;
    char *tok;

    // basic comma-delimited tokenization
    l_strncpy(buf, "one,two,three", sizeof(buf));
    save = (char *)0;
    tok = l_strtok_r(buf, ",", &save);
    TEST_ASSERT(tok != (char *)0 && l_strcmp(tok, "one") == 0, "first token 'one'");
    tok = l_strtok_r((char *)0, ",", &save);
    TEST_ASSERT(tok != (char *)0 && l_strcmp(tok, "two") == 0, "second token 'two'");
    tok = l_strtok_r((char *)0, ",", &save);
    TEST_ASSERT(tok != (char *)0 && l_strcmp(tok, "three") == 0, "third token 'three'");
    tok = l_strtok_r((char *)0, ",", &save);
    TEST_ASSERT(tok == (char *)0, "no more tokens returns NULL");

    // multiple delimiters and leading/trailing
    l_strncpy(buf, "  hello world  ", sizeof(buf));
    save = (char *)0;
    tok = l_strtok_r(buf, " ", &save);
    TEST_ASSERT(tok != (char *)0 && l_strcmp(tok, "hello") == 0, "skip leading spaces");
    tok = l_strtok_r((char *)0, " ", &save);
    TEST_ASSERT(tok != (char *)0 && l_strcmp(tok, "world") == 0, "second word");
    tok = l_strtok_r((char *)0, " ", &save);
    TEST_ASSERT(tok == (char *)0, "trailing spaces consumed");

    // single token (no delimiter in string)
    l_strncpy(buf, "hello", sizeof(buf));
    save = (char *)0;
    tok = l_strtok_r(buf, ",", &save);
    TEST_ASSERT(tok != (char *)0 && l_strcmp(tok, "hello") == 0, "single token");
    tok = l_strtok_r((char *)0, ",", &save);
    TEST_ASSERT(tok == (char *)0, "exhausted after single token");

    // empty string
    l_strncpy(buf, "", sizeof(buf));
    save = (char *)0;
    tok = l_strtok_r(buf, ",", &save);
    TEST_ASSERT(tok == (char *)0, "empty string returns NULL immediately");

    TEST_SECTION_PASS("l_strtok_r");
}

// ===================== l_basename / l_dirname =====================

void test_basename_dirname(void) {
    TEST_FUNCTION("l_basename / l_dirname");
    char buf[256];

    // l_basename
    TEST_ASSERT(l_strcmp(l_basename("/usr/bin/test"), "test") == 0, "unix path");
    TEST_ASSERT(l_strcmp(l_basename("C:\\Users\\file.txt"), "file.txt") == 0, "windows path");
    TEST_ASSERT(l_strcmp(l_basename("nodir"), "nodir") == 0, "no separator");
    TEST_ASSERT(l_strcmp(l_basename("/"), "") == 0, "root trailing slash");
    TEST_ASSERT(l_strcmp(l_basename("a/b/c"), "c") == 0, "relative unix path");
    TEST_ASSERT(l_strcmp(l_basename("a\\b\\c"), "c") == 0, "relative windows path");
    TEST_ASSERT(l_basename("") != (const char*)0 && l_basename("")[0] == '\0', "empty string returns empty");
    TEST_ASSERT(l_basename((const char*)0) == (const char*)0, "NULL returns NULL");

    // l_dirname
    l_dirname("/usr/bin/test", buf, sizeof(buf));
    TEST_ASSERT(l_strcmp(buf, "/usr/bin") == 0, "unix dir");
    l_dirname("C:\\Users\\file.txt", buf, sizeof(buf));
    TEST_ASSERT(l_strcmp(buf, "C:\\Users") == 0, "windows dir");
    l_dirname("nodir", buf, sizeof(buf));
    TEST_ASSERT(l_strcmp(buf, ".") == 0, "no separator gives dot");
    l_dirname("/file", buf, sizeof(buf));
    TEST_ASSERT(l_strcmp(buf, "/") == 0, "root dir");
    l_dirname("", buf, sizeof(buf));
    TEST_ASSERT(l_strcmp(buf, ".") == 0, "empty gives dot");
    l_dirname((const char*)0, buf, sizeof(buf));
    TEST_ASSERT(l_strcmp(buf, ".") == 0, "NULL gives dot");
    l_dirname("a/b/c", buf, sizeof(buf));
    TEST_ASSERT(l_strcmp(buf, "a/b") == 0, "relative unix dir");
    l_dirname("a\\b\\c", buf, sizeof(buf));
    TEST_ASSERT(l_strcmp(buf, "a\\b") == 0, "relative windows dir");

    // small buffer
    l_dirname("/usr/bin/test", buf, 4);
    TEST_ASSERT(l_strcmp(buf, "/us") == 0, "truncated dir in small buf");

    TEST_SECTION_PASS("l_basename / l_dirname");
}

// ===================== l_unlink / l_rmdir =====================

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

// ===================== l_rename / l_access =====================

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

// ===================== l_chmod =====================

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

// ===================== l_stat / l_fstat =====================

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

// ===================== l_opendir / l_readdir / l_closedir =====================

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

// ===================== mmap / munmap =====================

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

// ===================== l_symlink / l_readlink / l_realpath =====================

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

// ===================== l_pipe / l_dup2 =====================

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

// ===================== l_spawn / l_wait =====================

void test_spawn_wait(void) {
    TEST_FUNCTION("l_spawn / l_wait");

    char test_path[256];
    build_bin_path("test", test_path, sizeof(test_path));

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
    build_bin_path("test", test_path, sizeof(test_path));

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

    cap_len = read_fd_all(cap_pipe[0], cap_buf, sizeof(cap_buf));
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
    build_bin_path("sort", sort_path, sizeof(sort_path));
    build_bin_path("printenv", printenv_path, sizeof(printenv_path));

    L_FD mid_pipe[2];
    L_FD cap_pipe[2];
    TEST_ASSERT(l_pipe(mid_pipe) == 0, "pipeline dup2 creates first pipe");
    TEST_ASSERT(l_pipe(cap_pipe) == 0, "pipeline dup2 creates capture pipe");

    char *envp[] = {"LS_CHILD_PIPE=banana", NULL};
    char *left_args[] = {"printenv", "LS_CHILD_PIPE", NULL};
    char *right_args[] = {"sort", NULL};

    L_PID left_pid = spawn_with_redirects(printenv_path, left_args, envp,
                                          L_SPAWN_INHERIT, mid_pipe[1], L_SPAWN_INHERIT,
                                          &left_restore_ok);
    TEST_ASSERT(left_pid != -1, "spawn left via l_spawn succeeds");
    TEST_ASSERT(left_restore_ok, "left l_spawn restores parent stdio");
    l_close(mid_pipe[1]);

    L_PID right_pid = spawn_with_redirects(sort_path, right_args, (char *const *)0,
                                           mid_pipe[0], cap_pipe[1], L_SPAWN_INHERIT,
                                           &right_restore_ok);
    TEST_ASSERT(right_pid != -1, "spawn right via l_spawn succeeds");
    TEST_ASSERT(right_restore_ok, "right l_spawn restores parent stdio");
    l_close(mid_pipe[0]);
    l_close(cap_pipe[1]);

    char pipe_buf[64];
    int pipe_len = read_fd_all(cap_pipe[0], pipe_buf, sizeof(pipe_buf));
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

// ===================== l_spawn_stdio =====================

void test_spawn_stdio(void) {
    TEST_FUNCTION("l_spawn_stdio");

    char sort_path[64];
    char printenv_path[64];
    build_bin_path("sort", sort_path, sizeof(sort_path));
    build_bin_path("printenv", printenv_path, sizeof(printenv_path));

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
    int sort_len = read_fd_all(out_pipe[0], sort_buf, sizeof(sort_buf));
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
    int pipe_len = read_fd_all(cap_pipe[0], pipe_buf, sizeof(pipe_buf));
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

    build_bin_path("test", test_path, sizeof(test_path));
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

// ===================== l_find_executable =====================

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
        build_bin_path("test", self, sizeof(self));
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

// ===================== l_errno / l_strerror =====================

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

// ===================== new feature tests =====================

static int int_cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

void test_min_max_clamp_abs(void) {
    TEST_FUNCTION("L_MIN/L_MAX/L_CLAMP/l_abs");
    TEST_ASSERT(L_MIN(3, 5) == 3, "min(3,5)");
    TEST_ASSERT(L_MIN(5, 3) == 3, "min(5,3)");
    TEST_ASSERT(L_MAX(3, 5) == 5, "max(3,5)");
    TEST_ASSERT(L_MAX(5, 3) == 5, "max(5,3)");
    TEST_ASSERT(L_CLAMP(2, 1, 10) == 2, "clamp in range");
    TEST_ASSERT(L_CLAMP(-5, 0, 10) == 0, "clamp below");
    TEST_ASSERT(L_CLAMP(20, 0, 10) == 10, "clamp above");
    TEST_ASSERT(l_abs(5) == 5, "abs positive");
    TEST_ASSERT(l_abs(-5) == 5, "abs negative");
    TEST_ASSERT(l_abs(0) == 0, "abs zero");
    TEST_ASSERT(l_labs(-100000L) == 100000L, "labs negative");
    TEST_SECTION_PASS("L_MIN/L_MAX/L_CLAMP/l_abs");
}

void test_isprint_isxdigit(void) {
    TEST_FUNCTION("l_isprint/l_isxdigit");
    TEST_ASSERT(l_isprint(' '), "space is printable");
    TEST_ASSERT(l_isprint('A'), "A is printable");
    TEST_ASSERT(l_isprint('~'), "tilde is printable");
    TEST_ASSERT(!l_isprint('\n'), "newline not printable");
    TEST_ASSERT(!l_isprint(0x7f), "DEL not printable");
    TEST_ASSERT(!l_isprint(0x00), "NUL not printable");
    TEST_ASSERT(l_isxdigit('0'), "0 is hex");
    TEST_ASSERT(l_isxdigit('9'), "9 is hex");
    TEST_ASSERT(l_isxdigit('a'), "a is hex");
    TEST_ASSERT(l_isxdigit('f'), "f is hex");
    TEST_ASSERT(l_isxdigit('A'), "A is hex");
    TEST_ASSERT(l_isxdigit('F'), "F is hex");
    TEST_ASSERT(!l_isxdigit('g'), "g not hex");
    TEST_ASSERT(!l_isxdigit('G'), "G not hex");
    TEST_SECTION_PASS("l_isprint/l_isxdigit");
}

void test_rand_srand(void) {
    TEST_FUNCTION("l_rand/l_srand");
    l_srand(42);
    unsigned int r1 = l_rand();
    unsigned int r2 = l_rand();
    TEST_ASSERT(r1 != r2, "consecutive calls differ");
    l_srand(42);
    TEST_ASSERT(l_rand() == r1, "same seed same sequence");
    TEST_ASSERT(l_rand() == r2, "same seed same sequence (2)");
    int has_nonzero = 0;
    for (int i = 0; i < 100; i++) if (l_rand() != 0) has_nonzero = 1;
    TEST_ASSERT(has_nonzero, "produces nonzero values");
    TEST_SECTION_PASS("l_rand/l_srand");
}

void test_qsort_bsearch(void) {
    TEST_FUNCTION("l_qsort/l_bsearch");
    {
        int arr[] = {5, 3, 1, 4, 2};
        l_qsort(arr, 5, sizeof(int), int_cmp);
        TEST_ASSERT(arr[0] == 1 && arr[1] == 2 && arr[2] == 3 && arr[3] == 4 && arr[4] == 5, "qsort int ascending");
    }
    {
        int arr[] = {42};
        l_qsort(arr, 1, sizeof(int), int_cmp);
        TEST_ASSERT(arr[0] == 42, "qsort single element");
    }
    {
        int arr[] = {1, 2, 3, 4, 5};
        l_qsort(arr, 5, sizeof(int), int_cmp);
        TEST_ASSERT(arr[0] == 1 && arr[4] == 5, "qsort already sorted");
    }
    {
        int arr[] = {1, 2, 3, 4, 5};
        int key = 3;
        int *found = (int *)l_bsearch(&key, arr, 5, sizeof(int), int_cmp);
        TEST_ASSERT(found != (void *)0 && *found == 3, "bsearch found");
        key = 6;
        found = (int *)l_bsearch(&key, arr, 5, sizeof(int), int_cmp);
        TEST_ASSERT(found == (void *)0, "bsearch not found");
    }
    TEST_SECTION_PASS("l_qsort/l_bsearch");
}

void test_time(void) {
    TEST_FUNCTION("l_time");
    {
        long long t1 = l_time((long long *)0);
        TEST_ASSERT(t1 > 1700000000LL, "time is after 2023");
        long long t2 = 0;
        long long t3 = l_time(&t2);
        TEST_ASSERT(t2 == t3, "pointer and return match");
        TEST_ASSERT(t2 >= t1, "time doesn't go backward");
    }
    TEST_SECTION_PASS("l_time");
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

// ===================== main =====================

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

void test_arena(void) {
    TEST_FUNCTION("L_Arena");

    L_Arena a = l_arena_init(4096);
    TEST_ASSERT(a.base != (unsigned char *)0, "arena init returns non-NULL base");
    TEST_ASSERT(a.cap == 4096, "arena init sets cap");
    TEST_ASSERT(a.used == 0, "arena init sets used=0");

    void *p1 = l_arena_alloc(&a, 16);
    TEST_ASSERT(p1 != (void *)0, "arena alloc returns non-NULL");
    TEST_ASSERT(((size_t)p1 % 8) == 0, "arena alloc aligned to 8");

    void *p2 = l_arena_alloc(&a, 32);
    TEST_ASSERT(p2 != (void *)0, "arena second alloc non-NULL");
    TEST_ASSERT(((size_t)p2 % 8) == 0, "arena second alloc aligned");
    TEST_ASSERT((unsigned char *)p2 >= (unsigned char *)p1 + 16, "allocs do not overlap");

    // Write and read back through arena
    l_memset(p1, 0xAB, 16);
    l_memset(p2, 0xCD, 32);
    TEST_ASSERT(((unsigned char *)p1)[0] == 0xAB, "arena write/read p1");
    TEST_ASSERT(((unsigned char *)p2)[0] == 0xCD, "arena write/read p2");

    // Reset and reuse
    l_arena_reset(&a);
    TEST_ASSERT(a.used == 0, "arena reset sets used=0");
    void *p3 = l_arena_alloc(&a, 16);
    TEST_ASSERT(p3 == a.base, "alloc after reset reuses base region");

    // Arena full
    L_Arena tiny = l_arena_init(64);
    TEST_ASSERT(tiny.base != (unsigned char *)0, "small arena init");
    void *fill = l_arena_alloc(&tiny, 64);
    TEST_ASSERT(fill != (void *)0, "fill small arena");
    void *over = l_arena_alloc(&tiny, 1);
    TEST_ASSERT(over == (void *)0, "arena full returns NULL");
    l_arena_free(&tiny);
    TEST_ASSERT(tiny.base == (unsigned char *)0, "arena free sets base NULL");

    l_arena_free(&a);
    TEST_ASSERT(a.base == (unsigned char *)0, "arena free main");

    TEST_SECTION_PASS("L_Arena");
}

void test_buf(void) {
    TEST_FUNCTION("L_Buf");

    L_Buf b;
    l_buf_init(&b);
    TEST_ASSERT(b.data == (unsigned char *)0, "buf init zeroes data");
    TEST_ASSERT(b.len == 0, "buf init zeroes len");
    TEST_ASSERT(b.cap == 0, "buf init zeroes cap");

    // Push and verify
    const char *hello = "hello";
    TEST_ASSERT(l_buf_push(&b, hello, 5) == 0, "buf push succeeds");
    TEST_ASSERT(b.len == 5, "buf len after push");
    TEST_ASSERT(l_memcmp(b.data, "hello", 5) == 0, "buf data matches");

    // Multiple pushes accumulate
    TEST_ASSERT(l_buf_push(&b, " world", 6) == 0, "buf second push");
    TEST_ASSERT(b.len == 11, "buf len accumulates");
    TEST_ASSERT(l_memcmp(b.data, "hello world", 11) == 0, "buf accumulated data");

    // Printf
    int wrote = l_buf_printf(&b, " %d", 42);
    TEST_ASSERT(wrote == 3, "buf printf returns bytes written");
    TEST_ASSERT(b.len == 14, "buf len after printf");
    TEST_ASSERT(l_memcmp(b.data, "hello world 42", 14) == 0, "buf printf data");

    // Clear
    l_buf_clear(&b);
    TEST_ASSERT(b.len == 0, "buf clear resets len");
    TEST_ASSERT(b.data != (unsigned char *)0, "buf clear keeps data");
    TEST_ASSERT(b.cap > 0, "buf clear keeps cap");

    // Push beyond initial capacity triggers growth
    size_t old_cap = b.cap;
    size_t big = old_cap + 1;
    unsigned char *tmp = (unsigned char *)l_mmap((void *)0, big, L_PROT_READ | L_PROT_WRITE,
                                                 L_MAP_PRIVATE | L_MAP_ANONYMOUS, (L_FD)-1, 0);
    l_memset(tmp, 0x77, big);
    TEST_ASSERT(l_buf_push(&b, tmp, big) == 0, "buf push triggers growth");
    TEST_ASSERT(b.cap > old_cap, "buf cap grew");
    TEST_ASSERT(b.len == big, "buf len after large push");
    TEST_ASSERT(b.data[0] == 0x77 && b.data[big - 1] == 0x77, "buf large push data correct");
    l_munmap(tmp, big);

    // Large push > 4096
    l_buf_clear(&b);
    size_t huge = 8192;
    unsigned char *htmp = (unsigned char *)l_mmap((void *)0, huge, L_PROT_READ | L_PROT_WRITE,
                                                  L_MAP_PRIVATE | L_MAP_ANONYMOUS, (L_FD)-1, 0);
    l_memset(htmp, 0xAA, huge);
    TEST_ASSERT(l_buf_push(&b, htmp, huge) == 0, "buf large push >4096");
    TEST_ASSERT(b.len == huge, "buf len after huge push");
    TEST_ASSERT(b.data[0] == 0xAA && b.data[huge - 1] == 0xAA, "buf huge data correct");
    l_munmap(htmp, huge);

    // Free
    l_buf_free(&b);
    TEST_ASSERT(b.data == (unsigned char *)0, "buf free zeroes data");
    TEST_ASSERT(b.len == 0, "buf free zeroes len");
    TEST_ASSERT(b.cap == 0, "buf free zeroes cap");

    TEST_SECTION_PASS("L_Buf");
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
    TEST_ASSERT(l_path_exists("test/test.c") == 1, "path_exists on existing file");
    TEST_ASSERT(l_path_exists("nonexistent_xyz_123") == 0, "path_exists on missing file");

    // l_path_isdir
    TEST_ASSERT(l_path_isdir(".") == 1, "path_isdir on current dir");

    TEST_SECTION_PASS("path_utils");
}

void test_ansi_helpers(void) {
    TEST_FUNCTION("l_ansi_move / l_ansi_color / ANSI macros");

    char buf[64];

    // l_ansi_move
    l_ansi_move(buf, sizeof(buf), 5, 10);
    TEST_ASSERT(l_strcmp(buf, "\033[5;10H") == 0, "ansi_move 5,10");

    // l_ansi_color fg-only
    l_ansi_color(buf, sizeof(buf), 1, -1);
    TEST_ASSERT(l_strcmp(buf, "\033[31m") == 0, "ansi_color fg-only red");

    // l_ansi_color fg+bg
    l_ansi_color(buf, sizeof(buf), 7, 0);
    TEST_ASSERT(l_strcmp(buf, "\033[37;40m") == 0, "ansi_color fg+bg white on black");

    // ANSI macros
    TEST_ASSERT(l_strcmp(L_ANSI_CLEAR, "\033[2J") == 0, "L_ANSI_CLEAR value");
    TEST_ASSERT(l_strcmp(L_ANSI_RESET, "\033[0m") == 0, "L_ANSI_RESET value");

    TEST_SECTION_PASS("ansi_helpers");
}

#ifdef L_WITHSOCKETS
void test_sockets(void) {
    TEST_FUNCTION("l_htons / l_htonl");
    TEST_ASSERT(l_htons(0x1234) == 0x3412 || l_htons(0x1234) == 0x1234, "htons works");
    TEST_ASSERT(l_htonl(0x01020304) == 0x04030201 || l_htonl(0x01020304) == 0x01020304, "htonl works");

    TEST_FUNCTION("l_inet_addr");
    unsigned int addr = l_inet_addr("127.0.0.1");
    TEST_ASSERT(addr != 0, "inet_addr parses localhost");
    {
        unsigned char *b = (unsigned char *)&addr;
        TEST_ASSERT(b[0] == 127 && b[1] == 0 && b[2] == 0 && b[3] == 1,
                     "inet_addr correct network byte order");
    }
    TEST_ASSERT(l_inet_addr("192.168.1.1") != 0, "inet_addr parses 192.168.1.1");
    TEST_ASSERT(l_inet_addr("invalid") == 0, "inet_addr rejects invalid");
    TEST_ASSERT(l_inet_addr("256.0.0.1") == 0, "inet_addr rejects octet > 255");

#ifndef _WIN32
    TEST_FUNCTION("l_socket_tcp");
    {
        L_SOCKET s = l_socket_tcp();
        TEST_ASSERT(s >= 0, "socket creation succeeds");
        l_socket_close(s);
    }

    TEST_FUNCTION("l_socket_bind / l_socket_listen");
    {
        L_SOCKET s = l_socket_tcp();
        TEST_ASSERT(s >= 0, "socket for bind");
        int br = l_socket_bind(s, 0);
        if (br == 0) {
            test_count++;
            puts("    [OK] bind to port 0 succeeds\n");
            TEST_ASSERT(l_socket_listen(s, 5) == 0, "listen succeeds");
        } else {
            test_count += 2;
            puts("    [SKIP] bind not available (QEMU user-mode)\n");
        }
        l_socket_close(s);
    }
#else
    TEST_FUNCTION("l_socket_tcp (Windows)");
    {
        L_SOCKET s = l_socket_tcp();
        TEST_ASSERT(s >= 0, "socket creation succeeds");
        l_socket_close(s);
    }

    TEST_FUNCTION("l_socket_bind / l_socket_listen (Windows)");
    {
        L_SOCKET s = l_socket_tcp();
        TEST_ASSERT(s >= 0, "socket for bind");
        int br = l_socket_bind(s, 0);
        if (br == 0) {
            test_count++;
            puts("    [OK] bind to port 0 succeeds\n");
            TEST_ASSERT(l_socket_listen(s, 5) == 0, "listen succeeds");
        } else {
            test_count += 2;
            puts("    [SKIP] bind not available\n");
        }
        l_socket_close(s);
    }
#endif

    TEST_SECTION_PASS("sockets");
}
#endif // L_WITHSOCKETS

void test_math(void) {
    TEST_FUNCTION("l_fabs");
    TEST_ASSERT(l_fabs(-3.14) == 3.14, "fabs(-3.14) == 3.14");
    TEST_ASSERT(l_fabs(3.14) == 3.14, "fabs(3.14) == 3.14");
    TEST_ASSERT(l_fabs(0.0) == 0.0, "fabs(0.0) == 0.0");

    TEST_FUNCTION("l_floor");
    TEST_ASSERT(l_floor(2.7) == 2.0, "floor(2.7) == 2.0");
    TEST_ASSERT(l_floor(-2.3) == -3.0, "floor(-2.3) == -3.0");
    TEST_ASSERT(l_floor(5.0) == 5.0, "floor(5.0) == 5.0");

    TEST_FUNCTION("l_ceil");
    TEST_ASSERT(l_ceil(2.3) == 3.0, "ceil(2.3) == 3.0");
    TEST_ASSERT(l_ceil(-2.7) == -2.0, "ceil(-2.7) == -2.0");
    TEST_ASSERT(l_ceil(5.0) == 5.0, "ceil(5.0) == 5.0");

    TEST_FUNCTION("l_fmod");
    TEST_ASSERT(l_fabs(l_fmod(5.5, 2.0) - 1.5) < 1e-10, "fmod(5.5, 2.0) ~= 1.5");
    TEST_ASSERT(l_fabs(l_fmod(10.0, 3.0) - 1.0) < 1e-10, "fmod(10.0, 3.0) ~= 1.0");

    TEST_FUNCTION("l_sqrt");
    TEST_ASSERT(l_fabs(l_sqrt(4.0) - 2.0) < 1e-10, "sqrt(4.0) == 2.0");
    TEST_ASSERT(l_fabs(l_sqrt(2.0) - 1.4142135623730951) < 1e-10, "sqrt(2.0) ~= 1.41421356...");
    TEST_ASSERT(l_sqrt(0.0) == 0.0, "sqrt(0.0) == 0.0");

    TEST_FUNCTION("l_sin");
    TEST_ASSERT(l_fabs(l_sin(0.0)) < 1e-10, "sin(0) == 0");
    TEST_ASSERT(l_fabs(l_sin(L_PI_2) - 1.0) < 1e-10, "sin(pi/2) ~= 1.0");
    TEST_ASSERT(l_fabs(l_sin(L_PI)) < 1e-10, "sin(pi) ~= 0");

    TEST_FUNCTION("l_cos");
    TEST_ASSERT(l_fabs(l_cos(0.0) - 1.0) < 1e-10, "cos(0) == 1.0");
    TEST_ASSERT(l_fabs(l_cos(L_PI) + 1.0) < 1e-10, "cos(pi) ~= -1.0");

    TEST_FUNCTION("l_exp");
    TEST_ASSERT(l_exp(0.0) == 1.0, "exp(0) == 1.0");
    TEST_ASSERT(l_fabs(l_exp(1.0) - L_E) < 1e-10, "exp(1) ~= e");

    TEST_FUNCTION("l_log");
    TEST_ASSERT(l_fabs(l_log(L_E) - 1.0) < 1e-10, "log(e) ~= 1.0");
    TEST_ASSERT(l_fabs(l_log(1.0)) < 1e-10, "log(1) == 0");

    TEST_FUNCTION("l_pow");
    TEST_ASSERT(l_fabs(l_pow(2.0, 10.0) - 1024.0) < 1e-10, "pow(2, 10) == 1024");
    TEST_ASSERT(l_pow(5.0, 0.0) == 1.0, "pow(5, 0) == 1.0");

    TEST_FUNCTION("l_atan2");
    TEST_ASSERT(l_fabs(l_atan2(1.0, 1.0) - L_PI / 4.0) < 1e-10, "atan2(1,1) ~= pi/4");
    TEST_ASSERT(l_fabs(l_atan2(0.0, 1.0)) < 1e-10, "atan2(0,1) ~= 0");

    TEST_SECTION_PASS("math");
}

// ---------------------------------------------------------------------------
// L_Str tests
// ---------------------------------------------------------------------------

void test_str_constructors(void) {
    TEST_FUNCTION("L_Str constructors");

    L_Str s1 = l_str("hello");
    TEST_ASSERT(s1.len == 5, "l_str hello len=5");
    TEST_ASSERT(l_memcmp(s1.data, "hello", 5) == 0, "l_str hello data");

    L_Str s2 = l_str("");
    TEST_ASSERT(s2.len == 0, "l_str empty len=0");

    L_Str s3 = l_str((const char *)0);
    TEST_ASSERT(s3.len == 0, "l_str NULL len=0");

    L_Str s4 = l_str_from("hello", 3);
    TEST_ASSERT(s4.len == 3, "l_str_from len=3");
    TEST_ASSERT(l_memcmp(s4.data, "hel", 3) == 0, "l_str_from data");

    L_Str s5 = l_str_null();
    TEST_ASSERT(s5.data == (const char *)0, "l_str_null data=NULL");
    TEST_ASSERT(s5.len == 0, "l_str_null len=0");

    TEST_SECTION_PASS("L_Str constructors");
}

void test_str_comparison(void) {
    TEST_FUNCTION("L_Str comparison");

    // l_str_eq
    TEST_ASSERT(l_str_eq(l_str("abc"), l_str("abc")) == 1, "str_eq equal");
    TEST_ASSERT(l_str_eq(l_str("abc"), l_str("xyz")) == 0, "str_eq different");
    TEST_ASSERT(l_str_eq(l_str("ab"), l_str("abc")) == 0, "str_eq diff length");
    TEST_ASSERT(l_str_eq(l_str(""), l_str("")) == 1, "str_eq both empty");

    // l_str_cmp
    TEST_ASSERT(l_str_cmp(l_str("abc"), l_str("abd")) < 0, "str_cmp less");
    TEST_ASSERT(l_str_cmp(l_str("abd"), l_str("abc")) > 0, "str_cmp greater");
    TEST_ASSERT(l_str_cmp(l_str("abc"), l_str("abc")) == 0, "str_cmp equal");
    TEST_ASSERT(l_str_cmp(l_str("ab"), l_str("abc")) < 0, "str_cmp prefix shorter");

    // l_str_startswith
    TEST_ASSERT(l_str_startswith(l_str("hello world"), l_str("hello")) == 1, "startswith true");
    TEST_ASSERT(l_str_startswith(l_str("hello"), l_str("world")) == 0, "startswith false");
    TEST_ASSERT(l_str_startswith(l_str("hi"), l_str("hello")) == 0, "startswith prefix longer");
    TEST_ASSERT(l_str_startswith(l_str("hello"), l_str("")) == 1, "startswith empty prefix");

    // l_str_endswith
    TEST_ASSERT(l_str_endswith(l_str("hello world"), l_str("world")) == 1, "endswith true");
    TEST_ASSERT(l_str_endswith(l_str("hello"), l_str("world")) == 0, "endswith false");
    TEST_ASSERT(l_str_endswith(l_str("hi"), l_str("hello")) == 0, "endswith suffix longer");
    TEST_ASSERT(l_str_endswith(l_str("hello"), l_str("")) == 1, "endswith empty suffix");

    // l_str_contains
    TEST_ASSERT(l_str_contains(l_str("hello world"), l_str("lo w")) == 1, "contains found");
    TEST_ASSERT(l_str_contains(l_str("hello"), l_str("xyz")) == 0, "contains not found");
    TEST_ASSERT(l_str_contains(l_str("hello"), l_str("")) == 1, "contains empty needle");
    TEST_ASSERT(l_str_contains(l_str("hi"), l_str("hello")) == 0, "contains needle longer");

    TEST_SECTION_PASS("L_Str comparison");
}

void test_str_slicing(void) {
    TEST_FUNCTION("L_Str slicing");

    // l_str_sub
    L_Str hw = l_str("hello world");
    L_Str sub1 = l_str_sub(hw, 6, 5);
    TEST_ASSERT(sub1.len == 5, "str_sub normal len");
    TEST_ASSERT(l_memcmp(sub1.data, "world", 5) == 0, "str_sub normal data");

    L_Str sub2 = l_str_sub(hw, 50, 5);
    TEST_ASSERT(sub2.len == 0, "str_sub start beyond end");

    L_Str sub3 = l_str_sub(hw, 8, 100);
    TEST_ASSERT(sub3.len == 3, "str_sub len clamped");
    TEST_ASSERT(l_memcmp(sub3.data, "rld", 3) == 0, "str_sub clamped data");

    // l_str_trim
    L_Str trimmed = l_str_trim(l_str("  hello  "));
    TEST_ASSERT(l_str_eq(trimmed, l_str("hello")), "trim leading+trailing");
    TEST_ASSERT(l_str_trim(l_str("\t\n hi \t")).len == 2, "trim tabs/newlines");
    TEST_ASSERT(l_str_trim(l_str("hello")).len == 5, "trim no whitespace");
    TEST_ASSERT(l_str_trim(l_str("   ")).len == 0, "trim all whitespace");
    TEST_ASSERT(l_str_trim(l_str("")).len == 0, "trim empty");

    // l_str_ltrim / l_str_rtrim
    TEST_ASSERT(l_str_eq(l_str_ltrim(l_str("  hi")), l_str("hi")), "ltrim leading");
    TEST_ASSERT(l_str_eq(l_str_rtrim(l_str("hi  ")), l_str("hi")), "rtrim trailing");

    // l_str_chr
    TEST_ASSERT(l_str_chr(l_str("abcabc"), 'b') == 1, "str_chr found first");
    TEST_ASSERT(l_str_chr(l_str("abc"), 'z') == -1, "str_chr not found");

    // l_str_rchr
    TEST_ASSERT(l_str_rchr(l_str("abcabc"), 'b') == 4, "str_rchr last");
    TEST_ASSERT(l_str_rchr(l_str("abc"), 'z') == -1, "str_rchr not found");

    // l_str_find
    TEST_ASSERT(l_str_find(l_str("hello world"), l_str("world")) == 6, "str_find found");
    TEST_ASSERT(l_str_find(l_str("hello"), l_str("xyz")) == -1, "str_find not found");
    TEST_ASSERT(l_str_find(l_str("hello"), l_str("")) == 0, "str_find empty needle");

    TEST_SECTION_PASS("L_Str slicing");
}

void test_str_arena(void) {
    TEST_FUNCTION("L_Str arena ops");

    L_Arena a = l_arena_init(4096);

    // l_str_dup
    L_Str orig = l_str("hello");
    L_Str dup = l_str_dup(&a, orig);
    TEST_ASSERT(l_str_eq(dup, orig), "str_dup content matches");
    TEST_ASSERT(dup.data != orig.data, "str_dup pointers differ");

    // l_str_cat
    L_Str cat = l_str_cat(&a, l_str("foo"), l_str("bar"));
    TEST_ASSERT(cat.len == 6, "str_cat len");
    TEST_ASSERT(l_str_eq(cat, l_str("foobar")), "str_cat content");

    // l_str_cstr
    L_Str src = l_str("test");
    char *cs = l_str_cstr(&a, src);
    TEST_ASSERT(l_strcmp(cs, "test") == 0, "str_cstr null-terminated");
    TEST_ASSERT(cs[4] == '\0', "str_cstr explicit NUL");

    // l_str_from_cstr
    L_Str fc = l_str_from_cstr(&a, "arena copy");
    TEST_ASSERT(l_str_eq(fc, l_str("arena copy")), "str_from_cstr content");
    TEST_ASSERT(fc.data != "arena copy", "str_from_cstr is a copy");

    l_arena_free(&a);
    TEST_SECTION_PASS("L_Str arena ops");
}

void test_str_split_join(void) {
    TEST_FUNCTION("L_Str split/join");

    L_Arena a = l_arena_init(4096);

    // l_str_split normal
    L_Str *parts;
    int n = l_str_split(&a, l_str("a,b,c"), l_str(","), &parts);
    TEST_ASSERT(n == 3, "split a,b,c count=3");
    TEST_ASSERT(l_str_eq(parts[0], l_str("a")), "split part[0]=a");
    TEST_ASSERT(l_str_eq(parts[1], l_str("b")), "split part[1]=b");
    TEST_ASSERT(l_str_eq(parts[2], l_str("c")), "split part[2]=c");

    // split no delimiter found
    L_Str *parts2;
    int n2 = l_str_split(&a, l_str("hello"), l_str(","), &parts2);
    TEST_ASSERT(n2 == 1, "split no delim count=1");
    TEST_ASSERT(l_str_eq(parts2[0], l_str("hello")), "split no delim part");

    // split empty string
    L_Str *parts3;
    int n3 = l_str_split(&a, l_str(""), l_str(","), &parts3);
    TEST_ASSERT(n3 == 1, "split empty str count=1");
    TEST_ASSERT(parts3[0].len == 0, "split empty str part len=0");

    // l_str_join
    L_Str jp[3] = { l_str("a"), l_str("b"), l_str("c") };
    L_Str joined = l_str_join(&a, jp, 3, l_str(","));
    TEST_ASSERT(l_str_eq(joined, l_str("a,b,c")), "join a,b,c");

    // join with empty separator
    L_Str joined2 = l_str_join(&a, jp, 3, l_str(""));
    TEST_ASSERT(l_str_eq(joined2, l_str("abc")), "join empty sep");

    l_arena_free(&a);
    TEST_SECTION_PASS("L_Str split/join");
}

void test_str_case(void) {
    TEST_FUNCTION("L_Str case conversion");

    L_Arena a = l_arena_init(4096);

    TEST_ASSERT(l_str_eq(l_str_upper(&a, l_str("hello")), l_str("HELLO")), "upper hello");
    TEST_ASSERT(l_str_eq(l_str_lower(&a, l_str("HELLO")), l_str("hello")), "lower HELLO");
    TEST_ASSERT(l_str_eq(l_str_upper(&a, l_str("hElLo")), l_str("HELLO")), "upper mixed");
    TEST_ASSERT(l_str_eq(l_str_lower(&a, l_str("ABC123")), l_str("abc123")), "lower with digits");

    l_arena_free(&a);
    TEST_SECTION_PASS("L_Str case conversion");
}

void test_str_buf(void) {
    TEST_FUNCTION("L_Str buf helpers");

    L_Buf b;
    l_buf_init(&b);

    // l_buf_push_str
    TEST_ASSERT(l_buf_push_str(&b, l_str("hello")) == 0, "buf_push_str ok");
    TEST_ASSERT(b.len == 5, "buf_push_str len");
    TEST_ASSERT(l_memcmp(b.data, "hello", 5) == 0, "buf_push_str data");

    // l_buf_push_cstr
    TEST_ASSERT(l_buf_push_cstr(&b, " world") == 0, "buf_push_cstr ok");
    TEST_ASSERT(b.len == 11, "buf_push_cstr len");

    // l_buf_push_int
    l_buf_clear(&b);
    TEST_ASSERT(l_buf_push_int(&b, 42) == 0, "buf_push_int 42 ok");
    TEST_ASSERT(l_str_eq(l_buf_as_str(&b), l_str("42")), "buf_push_int 42 content");

    l_buf_clear(&b);
    TEST_ASSERT(l_buf_push_int(&b, -7) == 0, "buf_push_int negative ok");
    TEST_ASSERT(l_str_eq(l_buf_as_str(&b), l_str("-7")), "buf_push_int negative content");

    // l_buf_as_str
    l_buf_clear(&b);
    l_buf_push_cstr(&b, "view");
    L_Str view = l_buf_as_str(&b);
    TEST_ASSERT(view.len == 4, "buf_as_str len");
    TEST_ASSERT(l_memcmp(view.data, "view", 4) == 0, "buf_as_str data");

    l_buf_free(&b);
    TEST_SECTION_PASS("L_Str buf helpers");
}

int main(int argc, char* argv[]) {
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

    // String functions
    test_strlen();
    test_strstr();
    test_strchr();
    test_strrchr();
    test_strcpy();
    test_strncpy();
    test_strcat();
    test_strncat();
    test_strcmp();
    test_strncmp();
    test_reverse();
    test_strcasecmp();
    test_strspn();
    test_strpbrk();
    test_strtok_r();
    test_basename_dirname();

    // Conversion functions
    test_isdigit();
    test_isspace();
    test_char_classification();
    test_atoi_atol();
    test_strtoul_strtol();
    test_strtoull_strtoll();
    test_strtod_atof();
    test_strtof();
    test_itoa();
    test_itoa_atoi_roundtrip();

    // Memory functions
    test_memcmp();
    test_memcpy();
    test_memset();
    test_memmove();
    test_memchr();
    test_memrchr();
    test_strnlen();
    test_memmem();

    // Formatted output
    test_snprintf();
    test_snprintf_float();

    // Wide string
    test_wcslen();

    // File operations
    test_file_operations();
    test_open_append();
    test_system_functions();
    test_sleep_ms();
    test_getenv();
    test_env_iter();
    test_unlink_rmdir();
    test_rename_access();
    test_chmod();
    test_stat();
    test_opendir_readdir();
    test_mmap();
    test_getcwd_chdir();
    test_symlink_readlink();
    test_pipe_dup2();
    test_dup();
    test_spawn_wait();
    test_spawn_inherits_dup2();
    test_spawn_pipeline_via_dup2();
    test_spawn_stdio();
    test_spawn_stdio_child_fd_cleanup();
    test_find_executable();

    // Error reporting
    test_errno_strerror();

    // Option parsing
    test_getopt();

    test_lseek();
    test_mkdir();
    test_sched_yield();

    // New features
    test_min_max_clamp_abs();
    test_isprint_isxdigit();
    test_rand_srand();
    test_qsort_bsearch();
    test_time();
    test_dprintf();
    test_read_line();
    test_getpid_kill_llabs();

    // Arena and buffer
    test_arena();
    test_buf();

    // L_Str fat strings
    test_str_constructors();
    test_str_comparison();
    test_str_slicing();
    test_str_arena();
    test_str_split_join();
    test_str_case();
    test_str_buf();

    // Path and ANSI utilities
    test_path_utils();
    test_ansi_helpers();

    // Sockets
#ifdef L_WITHSOCKETS
    test_sockets();
#endif

    // Math functions
    test_math();

    puts("\n");
    puts("=====================================\n");
    puts("ALL TESTS PASSED!\n");
    puts("=====================================\n");

    return 0;
}
