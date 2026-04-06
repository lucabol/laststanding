// String, conversion, memory, and formatting coverage split from the old monolith.

#define L_WITHSNPRINTF
#define L_MAINFILE
#include "l_os.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

static int int_cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

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

    /* Alignment-boundary tests: word-at-a-time path must be correct at
     * every alignment offset.  Use a 32-byte buffer filled with a sentinel
     * byte and place the target at each of the first 24 positions. */
    {
        char abuf[32];
        int ok = 1;
        int i;
        l_memset(abuf, 'x', 31);
        abuf[31] = '\0';
        for (i = 0; i <= 23; i++) {
            abuf[i] = 'Q';
            if (l_strchr(abuf, 'Q') != abuf + i) { ok = 0; break; }
            abuf[i] = 'x';
        }
        TEST_ASSERT(ok, "word-at-a-time: correct at alignment offsets 0-23");
    }

    /* Target byte == 0xFF edge case (high bit set) */
    {
        char hbuf[16];
        l_memset(hbuf, 'a', 15);
        hbuf[8]  = (char)0xFF;
        hbuf[15] = '\0';
        TEST_ASSERT(l_strchr(hbuf, 0xFF) == hbuf + 8, "high-bit target byte found");
        TEST_ASSERT(l_strchr(hbuf, 0xFE) == NULL, "high-bit target byte not found");
    }

    /* Long string: target near end (forces multi-word scan) */
    {
        char lbuf[128];
        l_memset(lbuf, 'a', 127);
        lbuf[120] = 'Z';
        lbuf[127] = '\0';
        TEST_ASSERT(l_strchr(lbuf, 'Z') == lbuf + 120, "target near end of long string");
        TEST_ASSERT(l_strchr(lbuf, 'W') == NULL, "absent char in long string returns NULL");
    }

    TEST_SECTION_PASS("l_strchr");
}

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

    /* Alignment-boundary tests: exercise all phases of word-at-a-time path.
     * Build two 48-byte buffers filled with 'A'; place an 'X' difference at
     * each offset 0-15 to hit every alignment phase (prologue, word loop,
     * epilogue) and every byte lane within a machine word. */
    {
        char a[48], b[48];
        int ok = 1;
        int i;
        l_memset(a, 'A', 47); a[47] = '\0';
        l_memset(b, 'A', 47); b[47] = '\0';
        for (i = 0; i < 16; i++) {
            b[i] = 'X';
            if (!(l_strcmp(a, b) < 0)) { ok = 0; break; }
            if (!(l_strcmp(b, a) > 0)) { ok = 0; break; }
            b[i] = 'A';
        }
        TEST_ASSERT(ok, "word-at-a-time: strcmp correct at alignment offsets 0-15");
    }
    /* Long equal strings — stress the word loop. */
    {
        char a[128], b[128];
        l_memset(a, 'Z', 127); a[127] = '\0';
        l_memset(b, 'Z', 127); b[127] = '\0';
        TEST_ASSERT(l_strcmp(a, b) == 0, "word-at-a-time: long equal strings");
        b[126] = 'Y';
        TEST_ASSERT(l_strcmp(a, b) > 0, "word-at-a-time: difference at last byte");
    }

    TEST_SECTION_PASS("l_strcmp");
}

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

    /* Alignment-boundary tests: offset s2 by i bytes from an aligned buffer
     * to exercise unaligned s2 fallback, and place the difference at various
     * byte positions to cover all word-loop lanes. */
    {
        char a[64], b[64];
        int ok = 1;
        int i;
        l_memset(a, 'B', 63); a[63] = '\0';
        l_memset(b, 'B', 63); b[63] = '\0';
        for (i = 0; i < 16; i++) {
            b[i] = 'C';
            if (!(l_strncmp(a, b, 63) < 0)) { ok = 0; break; }
            if (!(l_strncmp(b, a, 63) > 0)) { ok = 0; break; }
            b[i] = 'B';
        }
        TEST_ASSERT(ok, "word-at-a-time: strncmp correct at alignment offsets 0-15");
    }
    /* n cuts off before any difference — must return 0. */
    TEST_ASSERT(l_strncmp("abcXYZ", "abcABC", 3) == 0, "n stops before difference");
    /* Long equal — stress word loop. */
    {
        char a[128], b[128];
        l_memset(a, 'Q', 127); a[127] = '\0';
        l_memset(b, 'Q', 127); b[127] = '\0';
        TEST_ASSERT(l_strncmp(a, b, 127) == 0, "word-at-a-time: long equal strings");
        b[120] = 'P';
        TEST_ASSERT(l_strncmp(a, b, 127) > 0, "word-at-a-time: difference deep in string");
    }

    TEST_SECTION_PASS("l_strncmp");
}

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

    /* Edge: lone "." with no adjacent digits is not a valid number.
     * endptr must point to the original string, not past the dot. */
    ep = (char *)0;
    l_strtod(".", &ep);
    TEST_ASSERT(ep != (char *)0 && ep[0] == '.', "strtod lone dot -> endptr at start");

    /* Edge: sign-only with a lone dot */
    ep = (char *)0;
    l_strtod("-.", &ep);
    TEST_ASSERT(ep != (char *)0 && ep[0] == '-', "strtod lone sign+dot -> endptr at start");

    /* Special: inf */
    double inf_val = l_atof("inf");
    TEST_ASSERT(inf_val > DBL_MAX, "atof 'inf' is infinity");
    double ninf_val = l_atof("-inf");
    TEST_ASSERT(ninf_val < -DBL_MAX, "atof '-inf' is negative infinity");

    /* "infinity" — endptr must advance past all 8 chars */
    ep = (char *)0;
    l_strtod("infinity", &ep);
    TEST_ASSERT(ep != (char *)0 && *ep == '\0', "strtod 'infinity' endptr at end");
    ep = (char *)0;
    l_strtod("INFINITY rest", &ep);
    TEST_ASSERT(ep != (char *)0 && ep[0] == ' ', "strtod 'INFINITY' endptr after word");
    ep = (char *)0;
    l_strtod("Infsomething", &ep); /* "Inf" is valid, stops there */
    TEST_ASSERT(ep != (char *)0 && ep[0] == 's', "strtod 'Inf' alone consumes only 3 chars");

    /* Special: nan */
    double nan_val = l_atof("nan");
    TEST_ASSERT(nan_val != nan_val, "atof 'nan' is NaN");

    /* case variations */
    TEST_ASSERT(l_atof("INF") > DBL_MAX, "atof 'INF' is infinity");
    TEST_ASSERT(l_atof("Infinity") > DBL_MAX, "atof 'Infinity' is infinity");
    TEST_ASSERT(l_atof("NaN") != l_atof("NaN"), "atof 'NaN' is NaN");

    TEST_SECTION_PASS("l_strtod / l_atof");
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

    /* Word-at-a-time alignment sweep: target placed at every position in a
     * 128-byte buffer across 16 alignment offsets.  Verifies the prologue,
     * word loop, and epilogue handle every byte-lane correctly. */
    {
        char abuf[144]; /* 128 data bytes + 16 alignment headroom */
        int ok = 1;
        int base;
        for (base = 0; base < 16 && ok; base++) {
            char *b = abuf + base;
            l_memset(b, 'a', 128);
            int i;
            for (i = 0; i < 128 && ok; i++) {
                b[i] = 'X';
                if (l_memrchr(b, 'X', 128) != (void *)(b + i)) ok = 0;
                b[i] = 'a';
            }
        }
        TEST_ASSERT(ok, "word-at-a-time: memrchr correct at all positions/alignments");
    }

    /* Rightmost occurrence: multiple hits, ensure the last one wins. */
    {
        char rbuf[64];
        l_memset(rbuf, 'Z', 64);
        rbuf[10] = 'X'; rbuf[30] = 'X'; rbuf[50] = 'X';
        TEST_ASSERT(l_memrchr(rbuf, 'X', 64) == (void *)(rbuf + 50), "rightmost of three");
        TEST_ASSERT(l_memrchr(rbuf, 'X', 51) == (void *)(rbuf + 50), "rightmost in limited range");
        TEST_ASSERT(l_memrchr(rbuf, 'X', 31) == (void *)(rbuf + 30), "rightmost excludes later hit");
    }

    TEST_SECTION_PASS("l_memrchr");
}

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

    /* %lld long long */
    l_snprintf(buf, sizeof(buf), "%lld", (long long)-1234567890123LL);
    TEST_ASSERT(l_strcmp(buf, "-1234567890123") == 0, "%lld negative");

    l_snprintf(buf, sizeof(buf), "%lld", (long long)0LL);
    TEST_ASSERT(l_strcmp(buf, "0") == 0, "%lld zero");

    /* %llu unsigned long long */
    l_snprintf(buf, sizeof(buf), "%llu", (unsigned long long)18446744073709551615ULL);
    TEST_ASSERT(l_strcmp(buf, "18446744073709551615") == 0, "%llu max");

    l_snprintf(buf, sizeof(buf), "%llu", (unsigned long long)0ULL);
    TEST_ASSERT(l_strcmp(buf, "0") == 0, "%llu zero");

    /* %p pointer */
    l_snprintf(buf, sizeof(buf), "%p", (void *)0);
    TEST_ASSERT(l_strcmp(buf, "0x0") == 0, "%p null pointer");

    l_snprintf(buf, sizeof(buf), "%p", (void *)(uintptr_t)0xABCD);
    TEST_ASSERT(l_strcmp(buf, "0xabcd") == 0, "%p non-null");

    /* %zu size_t */
    l_snprintf(buf, sizeof(buf), "%zu", (size_t)42);
    TEST_ASSERT(l_strcmp(buf, "42") == 0, "%zu basic");

    l_snprintf(buf, sizeof(buf), "%zu", (size_t)0);
    TEST_ASSERT(l_strcmp(buf, "0") == 0, "%zu zero");

    TEST_SECTION_PASS("l_snprintf");
}

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

void test_strsep(void) {
    TEST_FUNCTION("l_strsep");

    /* Basic comma-separated parsing */
    {
        char str[] = "one,two,three";
        char *p = str;
        char *tok;

        tok = l_strsep(&p, ",");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "one") == 0, "strsep first token");
        tok = l_strsep(&p, ",");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "two") == 0, "strsep second token");
        tok = l_strsep(&p, ",");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "three") == 0, "strsep third token");
        tok = l_strsep(&p, ",");
        TEST_ASSERT(tok == (void *)0, "strsep returns NULL when done");
        TEST_ASSERT(p == (void *)0, "strsep sets *stringp to NULL at end");
    }

    /* Empty tokens (differs from strtok_r) */
    {
        char str[] = "a,,b";
        char *p = str;
        char *tok;

        tok = l_strsep(&p, ",");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "a") == 0, "strsep before empty");
        tok = l_strsep(&p, ",");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "") == 0, "strsep empty token");
        tok = l_strsep(&p, ",");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "b") == 0, "strsep after empty");
        tok = l_strsep(&p, ",");
        TEST_ASSERT(tok == (void *)0, "strsep done after empty tokens");
    }

    /* NULL input */
    {
        char *p = (char *)0;
        char *tok = l_strsep(&p, ",");
        TEST_ASSERT(tok == (void *)0, "strsep NULL input returns NULL");
    }

    /* No delimiter found */
    {
        char str[] = "hello";
        char *p = str;
        char *tok = l_strsep(&p, ",");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "hello") == 0, "strsep no delim returns whole string");
        TEST_ASSERT(p == (void *)0, "strsep no delim sets *stringp to NULL");
    }

    /* Multiple delimiters */
    {
        char str[] = "a:b;c";
        char *p = str;
        char *tok;
        tok = l_strsep(&p, ":;");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "a") == 0, "strsep multi-delim first");
        tok = l_strsep(&p, ":;");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "b") == 0, "strsep multi-delim second");
        tok = l_strsep(&p, ":;");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "c") == 0, "strsep multi-delim third");
    }

    /* Leading delimiter produces empty first token */
    {
        char str[] = ",hello";
        char *p = str;
        char *tok = l_strsep(&p, ",");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "") == 0, "strsep leading delim gives empty token");
        tok = l_strsep(&p, ",");
        TEST_ASSERT(tok != (void *)0 && l_strcmp(tok, "hello") == 0, "strsep after leading delim");
    }

    TEST_SECTION_PASS("l_strsep");
}

void test_bin2hex_hex2bin(void) {
    TEST_FUNCTION("l_bin2hex / l_hex2bin");
    {
        unsigned char data[] = {0xDE, 0xAD, 0xBE, 0xEF};
        char hex[9];
        int n = l_bin2hex(hex, data, 4);
        TEST_ASSERT(n == 8, "bin2hex returns 8 for 4 bytes");
        TEST_ASSERT(l_strcmp(hex, "deadbeef") == 0, "bin2hex produces deadbeef");
    }
    {
        unsigned char out[4];
        int n = l_hex2bin(out, "DEADBEEF", 8);
        TEST_ASSERT(n == 4, "hex2bin returns 4 for 8 hex chars");
        TEST_ASSERT(out[0] == 0xDE && out[1] == 0xAD && out[2] == 0xBE && out[3] == 0xEF, "hex2bin uppercase");
    }
    {
        unsigned char out[4];
        int n = l_hex2bin(out, "deadbeef", 8);
        TEST_ASSERT(n == 4, "hex2bin lowercase returns 4");
        TEST_ASSERT(out[0] == 0xDE && out[1] == 0xAD, "hex2bin lowercase values");
    }
    {
        unsigned char out[1];
        int n = l_hex2bin(out, "ZZ", 2);
        TEST_ASSERT(n == -1, "hex2bin invalid char returns -1");
    }
    {
        unsigned char out[1];
        int n = l_hex2bin(out, "abc", 3);
        TEST_ASSERT(n == -1, "hex2bin odd length returns -1");
    }
    {
        char hex[1];
        int n = l_bin2hex(hex, "", 0);
        TEST_ASSERT(n == 0, "bin2hex zero length returns 0");
        TEST_ASSERT(hex[0] == '\0', "bin2hex zero length NUL-terminates");
    }
    /* Roundtrip */
    {
        unsigned char data[] = {0x00, 0x01, 0x7F, 0x80, 0xFF};
        char hex[11];
        unsigned char back[5];
        l_bin2hex(hex, data, 5);
        int n = l_hex2bin(back, hex, 10);
        TEST_ASSERT(n == 5, "roundtrip returns 5");
        TEST_ASSERT(l_memcmp(data, back, 5) == 0, "roundtrip matches");
    }
    TEST_SECTION_PASS("l_bin2hex / l_hex2bin");
}

void test_strstr_aligned(void) {
    TEST_FUNCTION("l_strstr alignment");
    /* Place the needle at each of the first 16 byte offsets inside a long
     * haystack.  This exercises the word-at-a-time l_memchr skip path across
     * all alignment phases. */
    char hay[128];
    const char *needle = "MATCH";
    int i;
    int ok = 1;
    for (i = 0; i < 16; i++) {
        l_memset(hay, 'x', (int)(sizeof(hay) - 1));
        hay[sizeof(hay) - 1] = '\0';
        l_memcpy(hay + i, needle, 5);
        char *found = l_strstr(hay, needle);
        if (found != hay + i) { ok = 0; break; }
    }
    TEST_ASSERT(ok, "word-at-a-time: finds needle at alignment offsets 0-15");
    /* Long haystack: needle near the end to maximise word-loop iterations. */
    {
        char lh[200];
        l_memset(lh, 'a', (int)(sizeof(lh) - 1));
        lh[sizeof(lh) - 1] = '\0';
        l_memcpy(lh + 180, "DONE", 4);
        TEST_ASSERT(l_strstr(lh, "DONE") == lh + 180, "finds needle after many word iterations");
        TEST_ASSERT(l_strstr(lh, "MISS") == NULL,      "not-found after long scan");
    }
    TEST_SECTION_PASS("l_strstr alignment");
}

void test_linebuf(void) {
    TEST_FUNCTION("l_linebuf_read");
    L_FD fds[2];
    TEST_ASSERT(l_pipe(fds) == 0, "pipe for linebuf test");
    const char *data = "hello\nworld\nfoo\n";
    l_write(fds[1], data, l_strlen(data));
    l_close(fds[1]);

    L_LineBuf lb;
    l_linebuf_init(&lb, fds[0]);

    char buf[64];
    ptrdiff_t n;

    n = l_linebuf_read(&lb, buf, sizeof(buf));
    TEST_ASSERT(n == 5 && l_strcmp(buf, "hello") == 0, "first line");

    n = l_linebuf_read(&lb, buf, sizeof(buf));
    TEST_ASSERT(n == 5 && l_strcmp(buf, "world") == 0, "second line");

    n = l_linebuf_read(&lb, buf, sizeof(buf));
    TEST_ASSERT(n == 3 && l_strcmp(buf, "foo") == 0, "third line");

    n = l_linebuf_read(&lb, buf, sizeof(buf));
    TEST_ASSERT(n == -1, "EOF returns -1");

    /* CRLF: Windows-style line endings are stripped. */
    L_FD fds2[2];
    TEST_ASSERT(l_pipe(fds2) == 0, "pipe for CRLF test");
    l_write(fds2[1], "line1\r\nline2\r\n", 14);
    l_close(fds2[1]);
    L_LineBuf lb2;
    l_linebuf_init(&lb2, fds2[0]);
    n = l_linebuf_read(&lb2, buf, sizeof(buf));
    TEST_ASSERT(n == 5 && l_strcmp(buf, "line1") == 0, "CRLF first line");
    n = l_linebuf_read(&lb2, buf, sizeof(buf));
    TEST_ASSERT(n == 5 && l_strcmp(buf, "line2") == 0, "CRLF second line");
    l_close(fds2[0]);

    l_close(fds[0]);
    TEST_SECTION_PASS("l_linebuf_read");
}

void test_printf_family(void) {
    TEST_FUNCTION("l_printf / l_fprintf / l_vprintf / l_vfprintf");
    /* Test via l_snprintf and l_vsnprintf since printf is a thin wrapper */
    {
        char buf[128];
        int n = l_snprintf(buf, sizeof(buf), "hello %s %d", "world", 42);
        TEST_ASSERT(n == 14, "snprintf returns 14");
        TEST_ASSERT(l_strcmp(buf, "hello world 42") == 0, "snprintf formats correctly");
    }
    /* l_fprintf to a pipe and read back */
    {
        L_FD fds[2];
        int r = l_pipe(fds);
        TEST_ASSERT(r == 0, "pipe for fprintf test");
        int n = l_fprintf(fds[1], "test %d %s", 99, "ok");
        l_close(fds[1]);
        TEST_ASSERT(n > 0, "fprintf returns positive");
        char buf[64];
        ptrdiff_t rd = l_read(fds[0], buf, sizeof(buf) - 1);
        TEST_ASSERT(rd > 0, "read from pipe");
        buf[rd] = '\0';
        TEST_ASSERT(l_strcmp(buf, "test 99 ok") == 0, "fprintf output matches");
        l_close(fds[0]);
    }
    TEST_SECTION_PASS("l_printf family");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
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
    test_isdigit();
    test_isspace();
    test_char_classification();
    test_atoi_atol();
    test_strtoul_strtol();
    test_strtoull_strtoll();
    test_strtod_atof();
    test_itoa();
    test_itoa_atoi_roundtrip();
    test_memcmp();
    test_memcpy();
    test_memset();
    test_memmove();
    test_memchr();
    test_memrchr();
    test_strnlen();
    test_memmem();
    test_snprintf();
    test_snprintf_float();
    test_wcslen();
    test_strcasecmp();
    test_strspn();
    test_strpbrk();
    test_strtok_r();
    test_strsep();
    test_basename_dirname();
    test_min_max_clamp_abs();
    test_isprint_isxdigit();
    test_rand_srand();
    test_qsort_bsearch();
    test_bin2hex_hex2bin();
    test_printf_family();
    test_strstr_aligned();
    test_linebuf();

    l_test_print_summary(passed_count, test_count);
    puts("PASS\n");
    return 0;
}
