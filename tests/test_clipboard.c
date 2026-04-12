#define L_MAINFILE
#include "l_gfx.h"
#include "test_support.h"

// test/test_clipboard.c — Tests for l_clipboard_set / l_clipboard_get
//
// Uses an in-memory canvas (no real display) to test the internal-buffer
// clipboard path (framebuffer backend on Linux, direct API on Windows).

TEST_DECLARE_COUNTERS();

static void test_empty_get(void) {
    TEST_FUNCTION("Clipboard empty get");
    L_Canvas c;
    memset(&c, 0, sizeof(c));
#ifndef _WIN32
    c.mouse_fd = -1;
    c.backend = 0;
#endif
    char buf[64];
    int r = l_clipboard_get(&c, buf, (int)sizeof(buf));
    /* On Windows, system clipboard may contain data from prior tests,
       so we only check that it doesn't crash and returns >= 0 */
#ifdef _WIN32
    TEST_ASSERT(r >= 0, "empty get returns >= 0 on Windows");
#else
    TEST_ASSERT(r == 0, "empty get returns 0");
    TEST_ASSERT(buf[0] == '\0', "empty get NUL-terminates");
#endif
    TEST_SECTION_PASS("Empty get");
}

static void test_roundtrip(void) {
    TEST_FUNCTION("Clipboard set+get roundtrip");
    L_Canvas c;
    memset(&c, 0, sizeof(c));
#ifndef _WIN32
    c.mouse_fd = -1;
    c.backend = 0;
#endif
    const char *text = "hello clipboard";
    int tlen = (int)l_strlen(text);
    int sr = l_clipboard_set(&c, text, tlen);
    TEST_ASSERT(sr == 0, "set returns 0");

    char buf[64];
    int gr = l_clipboard_get(&c, buf, (int)sizeof(buf));
    TEST_ASSERT(gr == tlen, "get returns correct length");
    TEST_ASSERT(l_memcmp(buf, text, (size_t)tlen) == 0, "get returns correct data");
    TEST_ASSERT(buf[gr] == '\0', "get NUL-terminates");
    TEST_SECTION_PASS("Roundtrip");
}

static void test_overwrite(void) {
    TEST_FUNCTION("Clipboard overwrite");
    L_Canvas c;
    memset(&c, 0, sizeof(c));
#ifndef _WIN32
    c.mouse_fd = -1;
    c.backend = 0;
#endif
    l_clipboard_set(&c, "first", 5);
    l_clipboard_set(&c, "second", 6);

    char buf[64];
    int gr = l_clipboard_get(&c, buf, (int)sizeof(buf));
    TEST_ASSERT(gr == 6, "overwrite: get returns latest length");
    TEST_ASSERT(l_memcmp(buf, "second", 6) == 0, "overwrite: get returns latest data");
    TEST_SECTION_PASS("Overwrite");
}

static void test_truncation(void) {
    TEST_FUNCTION("Clipboard truncation");
    L_Canvas c;
    memset(&c, 0, sizeof(c));
#ifndef _WIN32
    c.mouse_fd = -1;
    c.backend = 0;
#endif
    l_clipboard_set(&c, "hello world", 11);

    char buf[6]; /* only room for 5 chars + NUL */
    int gr = l_clipboard_get(&c, buf, (int)sizeof(buf));
    TEST_ASSERT(gr == 5, "truncation: returns truncated length");
    TEST_ASSERT(l_memcmp(buf, "hello", 5) == 0, "truncation: correct prefix");
    TEST_ASSERT(buf[5] == '\0', "truncation: NUL-terminated");
    TEST_SECTION_PASS("Truncation");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    puts("Testing l_gfx.h clipboard...\n");
    test_empty_get();
    test_roundtrip();
    test_overwrite();
    test_truncation();

    l_test_print_summary(passed_count, test_count);

    if (passed_count != test_count) {
        puts("FAIL\n");
        exit(1);
    }
    puts("PASS\n");
    return 0;
}
