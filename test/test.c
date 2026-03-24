// This tests multiple files including the library with just one of them defined as L_MAINFILE
#define L_WITHSNPRINTF
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

// ===================== l_itoa =====================

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

// ===================== Unix-only: l_lseek =====================

#ifndef _WIN32
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
#endif

// ===================== Unix-only: l_dup =====================

#ifndef _WIN32
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
#endif

// ===================== Unix-only: l_mkdir =====================

#ifndef _WIN32
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
#endif

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

// ===================== Unix-only: l_sched_yield =====================

#ifndef _WIN32
void test_sched_yield(void) {
    TEST_FUNCTION("l_sched_yield");

    int ret = l_sched_yield();
    TEST_ASSERT(ret == 0, "l_sched_yield returns 0 on success");

    TEST_SECTION_PASS("l_sched_yield");
}
#endif

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

// ===================== l_strcasecmp / l_strncasecmp =====================

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
#ifdef _WIN32
    CreateDirectoryA(tmpdir, NULL);
#else
    l_mkdir(tmpdir, 0755);
#endif
    TEST_ASSERT(l_rmdir(tmpdir) == 0, "rmdir empty directory");

    // rmdir non-existent should fail
    TEST_ASSERT(l_rmdir("nonexistent_dir_xyz") != 0, "rmdir non-existent fails");

    TEST_SECTION_PASS("l_unlink / l_rmdir");
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

    // Save cwd, chdir to known location, verify, chdir back
    char saved[512];
    l_getcwd(saved, sizeof(saved));

#ifdef _WIN32
    // On Windows, chdir to C:\ (always exists)
    int cd_ret = l_chdir("C:\\");
    TEST_ASSERT(cd_ret == 0, "chdir to C:\\ succeeds");
    l_getcwd(cwd, sizeof(cwd));
    TEST_ASSERT(cwd[0] == 'C', "getcwd after chdir starts with C");
#else
    // On Unix, chdir to / (always exists)
    int cd_ret = l_chdir("/");
    TEST_ASSERT(cd_ret == 0, "chdir to / succeeds");
    l_getcwd(cwd, sizeof(cwd));
    TEST_ASSERT(cwd[0] == '/' && cwd[1] == '\0', "getcwd after chdir is /");
#endif

    // Restore original cwd
    l_chdir(saved);

    TEST_SECTION_PASS("l_getcwd / l_chdir");
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

    TEST_SECTION_PASS("l_pipe / l_dup2");
}

// ===================== l_spawn / l_wait =====================

void test_spawn_wait(void) {
    TEST_FUNCTION("l_spawn / l_wait");

#ifdef _WIN32
    // Test: spawn cmd.exe with "exit 0"
    char *args[] = {"cmd.exe", "/c", "exit 0", NULL};
    L_PID pid = l_spawn("C:\\Windows\\System32\\cmd.exe", args, (char *const *)0);
    TEST_ASSERT(pid != -1, "spawn cmd.exe succeeds");
    int exitcode = -1;
    int ret = l_wait(pid, &exitcode);
    TEST_ASSERT(ret == 0, "wait succeeds");
    TEST_ASSERT(exitcode == 0, "exit code is 0");

    // Test non-zero exit code
    char *args2[] = {"cmd.exe", "/c", "exit 42", NULL};
    L_PID pid2 = l_spawn("C:\\Windows\\System32\\cmd.exe", args2, (char *const *)0);
    TEST_ASSERT(pid2 != -1, "spawn cmd.exe (exit 42) succeeds");
    int exitcode2 = -1;
    l_wait(pid2, &exitcode2);
    TEST_ASSERT(exitcode2 == 42, "exit code is 42");
#else
    // Test fork + waitpid directly (works under QEMU too)
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

    // Test l_spawn + l_wait with /bin/true (native Linux only)
    char *args[] = {"/bin/true", NULL};
    L_PID pid = l_spawn("/bin/true", args, (char *const *)0);
    if (pid > 0) {
        int exitcode = -1;
        int ret = l_wait(pid, &exitcode);
        TEST_ASSERT(ret == 0, "wait on /bin/true succeeds");
        TEST_ASSERT(exitcode == 0, "/bin/true exits 0");
    }

    // Test non-zero exit with /bin/false
    char *args2[] = {"/bin/false", NULL};
    L_PID pid2 = l_spawn("/bin/false", args2, (char *const *)0);
    if (pid2 > 0) {
        int exitcode2 = -1;
        l_wait(pid2, &exitcode2);
        TEST_ASSERT(exitcode2 != 0, "/bin/false exits non-zero");
    }
#endif

    TEST_SECTION_PASS("l_spawn / l_wait");
}

// ===================== main =====================

int main(int argc, char* argv[]) {
    l_getenv_init(argc, argv);

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
    test_basename_dirname();

    // Conversion functions
    test_isdigit();
    test_isspace();
    test_char_classification();
    test_atoi_atol();
    test_strtoul_strtol();
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

    // Wide string
    test_wcslen();

    // File operations
    test_file_operations();
    test_open_append();
    test_system_functions();
    test_sleep_ms();
    test_getenv();
    test_unlink_rmdir();
    test_stat();
    test_opendir_readdir();
    test_mmap();
    test_getcwd_chdir();
    test_pipe_dup2();
    test_spawn_wait();

#ifndef _WIN32
    // Unix-only
    test_lseek();
    test_dup();
    test_mkdir();
    test_sched_yield();
#endif

    puts("\n");
    puts("=====================================\n");
    puts("ALL TESTS PASSED!\n");
    puts("=====================================\n");

    return 0;
}
