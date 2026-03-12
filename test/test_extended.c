// Extended edge-case tests for l_* functions
// Covers: l_strlen, l_strstr, l_strcmp, l_strchr, l_strrchr, l_strcpy, l_strncpy,
//         l_strncmp, l_memcmp, l_memcpy, l_memset, l_memmove, l_reverse,
//         l_atoi, l_atol, l_itoa, l_isdigit

#include "l_os.h"
#include "l_os.h"

#define L_MAINFILE
#include "l_os.h"

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

// ===================== l_strlen =====================

void test_strlen(void) {
    TEST_FUNCTION("l_strlen");

    TEST_ASSERT(l_strlen("") == 0, "empty string has length 0");
    TEST_ASSERT(l_strlen("a") == 1, "single char has length 1");
    TEST_ASSERT(l_strlen("Hello") == 5, "'Hello' has length 5");
    TEST_ASSERT(l_strlen("Hello World") == 11, "'Hello World' has length 11");

    // String with embedded special chars (but still null-terminated)
    TEST_ASSERT(l_strlen("\t\n\r") == 3, "tab/newline/return has length 3");

    // Longer string
    char buf[256];
    l_memset(buf, 'x', 255);
    buf[255] = '\0';
    TEST_ASSERT(l_strlen(buf) == 255, "255-char string has length 255");

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
    // Note: l_strstr("", "") returns NULL (differs from libc which returns "")
    // This is a known behavior of the current implementation
    char empty[] = "";
    TEST_ASSERT(l_strstr(empty, "") == NULL, "empty hay + empty needle returns NULL");
    TEST_ASSERT(l_strstr("", "a") == NULL, "empty hay, non-empty needle");
    TEST_ASSERT(l_strstr(hay, "Hello World") == hay, "needle equals haystack");
    TEST_ASSERT(l_strstr(hay, "Hello World!") == NULL, "needle longer than haystack");

    // Repeated pattern
    char rep[] = "abcabcabc";
    TEST_ASSERT(l_strstr(rep, "abc") == rep, "repeated pattern finds first");
    TEST_ASSERT(l_strstr(rep, "cab") == rep + 2, "overlapping match");

    TEST_SECTION_PASS("l_strstr");
}

// ===================== l_strchr =====================

void test_strchr(void) {
    TEST_FUNCTION("l_strchr edge cases");

    char s[] = "abcdef";
    TEST_ASSERT(l_strchr(s, 'a') == s, "find first char");
    TEST_ASSERT(l_strchr(s, 'f') == s + 5, "find last char");
    TEST_ASSERT(l_strchr(s, 'z') == NULL, "missing char returns NULL");
    TEST_ASSERT(l_strchr("", 'a') == NULL, "empty string returns NULL");

    // Duplicate chars
    char dup[] = "aabaa";
    TEST_ASSERT(l_strchr(dup, 'a') == dup, "finds first of duplicates");
    TEST_ASSERT(l_strchr(dup, 'b') == dup + 2, "finds unique in duplicates");

    TEST_SECTION_PASS("l_strchr edge cases");
}

// ===================== l_strrchr =====================

void test_strrchr(void) {
    TEST_FUNCTION("l_strrchr edge cases");

    char s[] = "abcabc";
    TEST_ASSERT(l_strrchr(s, 'a') == s + 3, "finds last 'a'");
    TEST_ASSERT(l_strrchr(s, 'c') == s + 5, "finds last 'c'");
    TEST_ASSERT(l_strrchr(s, 'z') == NULL, "missing char returns NULL");
    TEST_ASSERT(l_strrchr("", 'a') == NULL, "empty string returns NULL");

    // Single char string
    char sx[] = "x";
    TEST_ASSERT(l_strrchr(sx, 'x') == sx, "single char match");
    TEST_ASSERT(l_strrchr(sx, 'y') == NULL, "single char no match");

    TEST_SECTION_PASS("l_strrchr edge cases");
}

// ===================== l_strcpy =====================

void test_strcpy(void) {
    TEST_FUNCTION("l_strcpy edge cases");

    // Empty string
    char dst[32];
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

    TEST_SECTION_PASS("l_strcpy edge cases");
}

// ===================== l_strncmp =====================

void test_strncmp(void) {
    TEST_FUNCTION("l_strncmp edge cases");

    // Zero length comparison
    TEST_ASSERT(l_strncmp("abc", "xyz", 0) == 0, "n=0 always equal");

    // Equal strings, n beyond length
    TEST_ASSERT(l_strncmp("abc", "abc", 10) == 0, "equal strings with large n");

    // Empty strings
    TEST_ASSERT(l_strncmp("", "", 5) == 0, "two empty strings");
    TEST_ASSERT(l_strncmp("", "a", 1) < 0, "empty vs non-empty");
    TEST_ASSERT(l_strncmp("a", "", 1) > 0, "non-empty vs empty");

    // Single char differences
    TEST_ASSERT(l_strncmp("a", "b", 1) < 0, "'a' < 'b'");
    TEST_ASSERT(l_strncmp("b", "a", 1) > 0, "'b' > 'a'");

    TEST_SECTION_PASS("l_strncmp edge cases");
}

// ===================== l_strcmp =====================

void test_strcmp(void) {
    TEST_FUNCTION("l_strcmp");

    // Equal strings
    TEST_ASSERT(l_strcmp("abc", "abc") == 0, "equal strings");
    TEST_ASSERT(l_strcmp("", "") == 0, "two empty strings");
    TEST_ASSERT(l_strcmp("a", "a") == 0, "single equal char");

    // Less than
    TEST_ASSERT(l_strcmp("abc", "abd") < 0, "'abc' < 'abd'");
    TEST_ASSERT(l_strcmp("abc", "abcd") < 0, "prefix is less");
    TEST_ASSERT(l_strcmp("", "a") < 0, "empty < non-empty");
    TEST_ASSERT(l_strcmp("A", "a") < 0, "'A' < 'a'");

    // Greater than
    TEST_ASSERT(l_strcmp("abd", "abc") > 0, "'abd' > 'abc'");
    TEST_ASSERT(l_strcmp("abcd", "abc") > 0, "longer > prefix");
    TEST_ASSERT(l_strcmp("a", "") > 0, "non-empty > empty");
    TEST_ASSERT(l_strcmp("b", "a") > 0, "'b' > 'a'");

    // Unsigned comparison (high-bit chars)
    TEST_ASSERT(l_strcmp("\x80", "\x01") > 0, "unsigned: 0x80 > 0x01");

    TEST_SECTION_PASS("l_strcmp");
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

// ===================== l_memcmp =====================

void test_memcmp(void) {
    TEST_FUNCTION("l_memcmp edge cases");

    TEST_ASSERT(l_memcmp("abc", "abc", 3) == 0, "equal buffers");
    TEST_ASSERT(l_memcmp("abc", "abd", 3) < 0, "'abc' < 'abd'");
    TEST_ASSERT(l_memcmp("abd", "abc", 3) > 0, "'abd' > 'abc'");
    TEST_ASSERT(l_memcmp("abc", "xyz", 0) == 0, "zero length always equal");

    // Compare with embedded zeros
    char a[] = {1, 0, 3};
    char b[] = {1, 0, 3};
    char c[] = {1, 0, 4};
    TEST_ASSERT(l_memcmp(a, b, 3) == 0, "equal with embedded zeros");
    TEST_ASSERT(l_memcmp(a, c, 3) < 0, "differ after embedded zero");

    // Single byte
    TEST_ASSERT(l_memcmp("x", "x", 1) == 0, "single byte equal");
    TEST_ASSERT(l_memcmp("a", "b", 1) < 0, "single byte less");

    TEST_SECTION_PASS("l_memcmp edge cases");
}

// ===================== l_memcpy =====================

void test_memcpy(void) {
    TEST_FUNCTION("l_memcpy edge cases");

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

    TEST_SECTION_PASS("l_memcpy edge cases");
}

// ===================== l_memset =====================

void test_memset(void) {
    TEST_FUNCTION("l_memset edge cases");

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

    TEST_SECTION_PASS("l_memset edge cases");
}

// ===================== l_memmove =====================

void test_memmove(void) {
    TEST_FUNCTION("l_memmove edge cases");

    // Non-overlapping forward
    char src[] = "ABCDE";
    char dst[10];
    l_memmove(dst, src, 5);
    TEST_ASSERT(l_memcmp(dst, "ABCDE", 5) == 0, "non-overlapping copy");

    // Overlapping: dst before src (copy forward)
    char overlap1[] = "123456789";
    l_memmove(overlap1, overlap1 + 2, 5);
    TEST_ASSERT(l_memcmp(overlap1, "34567", 5) == 0, "overlap: src after dst");

    // Overlapping: dst after src (copy backward)
    char overlap2[] = "123456789";
    l_memmove(overlap2 + 2, overlap2, 5);
    TEST_ASSERT(l_memcmp(overlap2 + 2, "12345", 5) == 0, "overlap: dst after src");

    // Zero length
    char zero[] = "unchanged";
    l_memmove(zero, "XXX", 0);
    TEST_ASSERT(l_strncmp(zero, "unchanged", 9) == 0, "zero-length memmove");

    // Single byte
    char a = 'A', b = 'B';
    l_memmove(&a, &b, 1);
    TEST_ASSERT(a == 'B', "single byte memmove");

    TEST_SECTION_PASS("l_memmove edge cases");
}

// ===================== l_reverse =====================

void test_reverse(void) {
    TEST_FUNCTION("l_reverse edge cases");

    // Even length
    char even[] = "abcd";
    l_reverse(even, 4);
    TEST_ASSERT(l_strncmp(even, "dcba", 4) == 0, "reverse even-length string");

    // Odd length
    char odd[] = "abcde";
    l_reverse(odd, 5);
    TEST_ASSERT(l_strncmp(odd, "edcba", 5) == 0, "reverse odd-length string");

    // Single char
    char one[] = "x";
    l_reverse(one, 1);
    TEST_ASSERT(one[0] == 'x', "reverse single char is no-op");

    // Two chars
    char two[] = "ab";
    l_reverse(two, 2);
    TEST_ASSERT(l_strncmp(two, "ba", 2) == 0, "reverse two chars");

    // Palindrome
    char pal[] = "abcba";
    l_reverse(pal, 5);
    TEST_ASSERT(l_strncmp(pal, "abcba", 5) == 0, "reverse palindrome unchanged");

    // Double reverse restores original
    char dbl[] = "Hello";
    l_reverse(dbl, 5);
    l_reverse(dbl, 5);
    TEST_ASSERT(l_strncmp(dbl, "Hello", 5) == 0, "double reverse restores original");

    TEST_SECTION_PASS("l_reverse edge cases");
}

// ===================== l_isdigit =====================

void test_isdigit(void) {
    TEST_FUNCTION("l_isdigit edge cases");

    // All digits
    for (char c = '0'; c <= '9'; c++) {
        if (!l_isdigit(c)) {
            TEST_ASSERT(0, "l_isdigit must accept all 0-9");
        }
    }
    TEST_ASSERT(1, "all digits 0-9 recognized");

    // Non-digits
    TEST_ASSERT(l_isdigit('a') == 0, "rejects 'a'");
    TEST_ASSERT(l_isdigit('Z') == 0, "rejects 'Z'");
    TEST_ASSERT(l_isdigit(' ') == 0, "rejects space");
    TEST_ASSERT(l_isdigit('\0') == 0, "rejects null");
    TEST_ASSERT(l_isdigit('-') == 0, "rejects minus");
    TEST_ASSERT(l_isdigit('+') == 0, "rejects plus");
    TEST_ASSERT(l_isdigit('.') == 0, "rejects dot");

    // Boundary: chars just outside '0'-'9'
    TEST_ASSERT(l_isdigit('0' - 1) == 0, "rejects char before '0'");
    TEST_ASSERT(l_isdigit('9' + 1) == 0, "rejects char after '9'");

    TEST_SECTION_PASS("l_isdigit edge cases");
}

// ===================== l_atoi / l_atol =====================

void test_atoi_atol(void) {
    TEST_FUNCTION("l_atoi / l_atol edge cases");

    // Basic conversions
    TEST_ASSERT(l_atoi("0") == 0, "atoi '0'");
    TEST_ASSERT(l_atoi("1") == 1, "atoi '1'");
    TEST_ASSERT(l_atoi("-1") == -1, "atoi '-1'");
    TEST_ASSERT(l_atoi("42") == 42, "atoi '42'");
    TEST_ASSERT(l_atoi("-42") == -42, "atoi '-42'");

    // Leading zeros
    TEST_ASSERT(l_atoi("007") == 7, "atoi leading zeros");
    TEST_ASSERT(l_atoi("00") == 0, "atoi '00'");

    // Larger numbers
    TEST_ASSERT(l_atoi("12345") == 12345, "atoi '12345'");
    TEST_ASSERT(l_atoi("-99999") == -99999, "atoi '-99999'");
    TEST_ASSERT(l_atoi("2147483647") == 2147483647, "atoi INT_MAX");

    // l_atol specific
    TEST_ASSERT(l_atol("0") == 0L, "atol '0'");
    TEST_ASSERT(l_atol("-1") == -1L, "atol '-1'");
    TEST_ASSERT(l_atol("100000") == 100000L, "atol '100000'");
    TEST_ASSERT(l_atol("999999999") == 999999999L, "atol large value");

    // Trailing non-digit chars (should stop at non-digit)
    TEST_ASSERT(l_atoi("123abc") == 123, "atoi stops at non-digit");
    TEST_ASSERT(l_atoi("0x10") == 0, "atoi stops at 'x'");

    TEST_SECTION_PASS("l_atoi / l_atol edge cases");
}

// ===================== l_itoa =====================

void test_itoa(void) {
    TEST_FUNCTION("l_itoa edge cases");

    char buf[64];

    // Zero
    l_itoa(0, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "0", 1) == 0, "itoa 0 decimal");
    TEST_ASSERT(buf[1] == '\0', "itoa 0 null-terminated");

    // Positive
    l_itoa(1, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "1", 1) == 0, "itoa 1");

    l_itoa(9, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "9", 1) == 0, "itoa single digit");

    l_itoa(10, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "10", 2) == 0, "itoa 10");

    l_itoa(12345, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "12345", 5) == 0, "itoa 12345");

    // Negative
    l_itoa(-1, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "-1", 2) == 0, "itoa -1");

    l_itoa(-999, buf, 10);
    TEST_ASSERT(l_strncmp(buf, "-999", 4) == 0, "itoa -999");

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

    TEST_SECTION_PASS("l_itoa edge cases");
}

// ===================== Roundtrip: itoa -> atoi =====================

void test_itoa_atoi_roundtrip(void) {
    TEST_FUNCTION("itoa/atoi roundtrip");

    char buf[32];
    int values[] = {0, 1, -1, 42, -42, 100, -100, 9999, -9999, 2147483, -2147483};
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

// ===================== l_wcslen =====================

void test_wcslen(void) {
    TEST_FUNCTION("l_wcslen");

    TEST_ASSERT(l_wcslen(L"") == 0, "empty wide string has length 0");
    TEST_ASSERT(l_wcslen(L"Hello") == 5, "L\"Hello\" has length 5");
    TEST_ASSERT(l_wcslen(L"a") == 1, "single wide char has length 1");
    TEST_ASSERT(l_wcslen(L"Hello World") == 11, "L\"Hello World\" has length 11");

    // Manually constructed wide string
    const wchar_t ws[] = {L'A', L'B', L'C', L'\0'};
    TEST_ASSERT(l_wcslen(ws) == 3, "manually constructed wide string has length 3");

    TEST_SECTION_PASS("l_wcslen");
}

// ===================== l_lseek =====================

#ifndef _WIN32
void test_lseek(void) {
    TEST_FUNCTION("l_lseek");

    char *msg = "Hello World!";
    int msg_len = l_strlen(msg);

    // Write data to a file
    L_FD fd = l_open_write("test_lseek_file");
    TEST_ASSERT(fd >= 0, "open file for writing");
    ssize_t written = l_write(fd, msg, msg_len);
    TEST_ASSERT(written == msg_len, "write data to file");
    l_close(fd);

    // Open for read, read once, seek back, read again
    fd = l_open_read("test_lseek_file");
    TEST_ASSERT(fd >= 0, "open file for reading");
    char buf[32];
    ssize_t n = l_read(fd, buf, msg_len);
    TEST_ASSERT(n == msg_len, "first read returns all bytes");
    TEST_ASSERT(l_memcmp(buf, msg, msg_len) == 0, "first read matches written data");

    // Seek back to beginning
    off_t pos = l_lseek(fd, 0, SEEK_SET);
    TEST_ASSERT(pos == 0, "SEEK_SET to 0 returns offset 0");

    // Read again after seek
    l_memset(buf, 0, 32);
    n = l_read(fd, buf, msg_len);
    TEST_ASSERT(n == msg_len, "read after seek returns correct byte count");
    TEST_ASSERT(l_memcmp(buf, msg, msg_len) == 0, "data after seek matches original");

    // Seek to middle
    pos = l_lseek(fd, 6, SEEK_SET);
    TEST_ASSERT(pos == 6, "SEEK_SET to 6 returns offset 6");
    n = l_read(fd, buf, 6);
    TEST_ASSERT(n == 6, "read 6 bytes from offset 6");
    TEST_ASSERT(l_memcmp(buf, "World!", 6) == 0, "data from offset 6 is 'World!'");

    // SEEK_CUR
    l_lseek(fd, 0, SEEK_SET);
    l_lseek(fd, 5, SEEK_CUR);
    pos = l_lseek(fd, 0, SEEK_CUR);
    TEST_ASSERT(pos == 5, "SEEK_CUR advances correctly");

    // SEEK_END
    pos = l_lseek(fd, 0, SEEK_END);
    TEST_ASSERT(pos == msg_len, "SEEK_END returns file size");

    l_close(fd);

    TEST_SECTION_PASS("l_lseek");
}
#endif

// ===================== l_dup =====================

#ifndef _WIN32
void test_dup(void) {
    TEST_FUNCTION("l_dup");

    // Open a file for writing
    L_FD fd = l_open_write("test_dup_file");
    TEST_ASSERT(fd >= 0, "open file for writing");

    // Duplicate the fd
    L_FD fd2 = l_dup(fd);
    TEST_ASSERT(fd2 >= 0, "dup returns valid fd");
    TEST_ASSERT(fd2 != fd, "dup returns different fd number");

    // Write via original fd
    char *msg1 = "Hello";
    ssize_t w1 = l_write(fd, msg1, 5);
    TEST_ASSERT(w1 == 5, "write via original fd succeeds");

    // Write via duplicated fd
    char *msg2 = "World";
    ssize_t w2 = l_write(fd2, msg2, 5);
    TEST_ASSERT(w2 == 5, "write via duplicated fd succeeds");

    // Close both
    l_close(fd);
    l_close(fd2);

    // Read back and verify both writes
    L_FD rfd = l_open_read("test_dup_file");
    TEST_ASSERT(rfd >= 0, "open file for reading back");
    char buf[16];
    ssize_t n = l_read(rfd, buf, 10);
    TEST_ASSERT(n == 10, "read back all 10 bytes");
    TEST_ASSERT(l_memcmp(buf, "HelloWorld", 10) == 0, "both fd writes captured in order");
    l_close(rfd);

    TEST_SECTION_PASS("l_dup");
}
#endif

// ===================== l_mkdir =====================

#ifndef _WIN32
void test_mkdir(void) {
    TEST_FUNCTION("l_mkdir");

    // Create a temp directory
    int ret = l_mkdir("test_mkdir_tmpdir", 0755);
    TEST_ASSERT(ret == 0, "mkdir creates directory successfully");

    // Verify it exists by chdir-ing into it
    ret = l_chdir("test_mkdir_tmpdir");
    TEST_ASSERT(ret == 0, "chdir into new directory succeeds");

    // Go back to parent
    ret = l_chdir("..");
    TEST_ASSERT(ret == 0, "chdir back to parent succeeds");

    // Creating the same directory again should fail (already exists)
    ret = l_mkdir("test_mkdir_tmpdir", 0755);
    TEST_ASSERT(ret < 0, "mkdir on existing directory fails");

    TEST_SECTION_PASS("l_mkdir");
}
#endif

// ===================== main =====================

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    puts("=== Extended l_* function tests ===\n");

    test_strlen();
    test_strstr();
    test_strchr();
    test_strrchr();
    test_strcpy();
    test_strncmp();
    test_strcmp();
    test_strncpy();
    test_memcmp();
    test_memcpy();
    test_memset();
    test_memmove();
    test_reverse();
    test_isdigit();
    test_atoi_atol();
    test_itoa();
    test_itoa_atoi_roundtrip();
    test_wcslen();
#ifndef _WIN32
    test_lseek();
    test_dup();
    test_mkdir();
#endif

    puts("\n");
    puts("=====================================\n");
    puts("ALL EXTENDED TESTS PASSED!\n");
    puts("=====================================\n");

    return 0;
}
