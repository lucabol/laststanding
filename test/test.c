// This tests multiple files including the library with just one of them defined as L_MAINFILE
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

    // Conversion functions
    test_isdigit();
    test_isspace();
    test_char_classification();
    test_atoi_atol();
    test_itoa();
    test_itoa_atoi_roundtrip();

    // Memory functions
    test_memcmp();
    test_memcpy();
    test_memset();
    test_memmove();

    // Wide string
    test_wcslen();

    // File operations
    test_file_operations();
    test_open_append();
    test_system_functions();
    test_sleep_ms();
    test_getenv();

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
