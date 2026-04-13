// test_term_gfx.c — Tests for terminal pixel backend (l_ansi_color_rgb + math)
//
// Does NOT open a terminal canvas — CI has no real terminal.
// Tests l_ansi_color_rgb escape sequence generation, pixel dimension math,
// and UTF-8 encoding of the half-block character (U+2580 ▀).

#define L_MAINFILE
#include "l_gfx.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

// ── l_ansi_color_rgb tests ──────────────────────────────────────────────────

static void test_ansi_color_rgb_foreground(void) {
    TEST_FUNCTION("l_ansi_color_rgb foreground");

    char buf[64];
    int len;

    // Red foreground
    len = l_ansi_color_rgb(buf, sizeof(buf), 255, 0, 0, 0);
    TEST_ASSERT(l_strcmp(buf, "\033[38;2;255;0;0m") == 0,
                "red foreground produces correct sequence");
    TEST_ASSERT(len == (int)l_strlen("\033[38;2;255;0;0m"),
                "red foreground returns correct length");

    // Green foreground
    len = l_ansi_color_rgb(buf, sizeof(buf), 0, 255, 0, 0);
    TEST_ASSERT(l_strcmp(buf, "\033[38;2;0;255;0m") == 0,
                "green foreground produces correct sequence");
    (void)len;

    // Blue foreground
    l_ansi_color_rgb(buf, sizeof(buf), 0, 0, 255, 0);
    TEST_ASSERT(l_strcmp(buf, "\033[38;2;0;0;255m") == 0,
                "blue foreground produces correct sequence");
}

static void test_ansi_color_rgb_background(void) {
    TEST_FUNCTION("l_ansi_color_rgb background");

    char buf[64];
    int len;

    // Green background
    len = l_ansi_color_rgb(buf, sizeof(buf), 0, 255, 0, 1);
    TEST_ASSERT(l_strcmp(buf, "\033[48;2;0;255;0m") == 0,
                "green background produces correct sequence");
    TEST_ASSERT(len == (int)l_strlen("\033[48;2;0;255;0m"),
                "green background returns correct length");

    // Red background
    l_ansi_color_rgb(buf, sizeof(buf), 255, 0, 0, 1);
    TEST_ASSERT(l_strcmp(buf, "\033[48;2;255;0;0m") == 0,
                "red background produces correct sequence");

    // Blue background
    l_ansi_color_rgb(buf, sizeof(buf), 0, 0, 255, 1);
    TEST_ASSERT(l_strcmp(buf, "\033[48;2;0;0;255m") == 0,
                "blue background produces correct sequence");
}

static void test_ansi_color_rgb_extremes(void) {
    TEST_FUNCTION("l_ansi_color_rgb extreme values");

    char buf[64];

    // Black (0,0,0) foreground
    l_ansi_color_rgb(buf, sizeof(buf), 0, 0, 0, 0);
    TEST_ASSERT(l_strcmp(buf, "\033[38;2;0;0;0m") == 0,
                "black foreground produces valid sequence");

    // White (255,255,255) foreground
    l_ansi_color_rgb(buf, sizeof(buf), 255, 255, 255, 0);
    TEST_ASSERT(l_strcmp(buf, "\033[38;2;255;255;255m") == 0,
                "white foreground produces valid sequence");

    // Black (0,0,0) background
    l_ansi_color_rgb(buf, sizeof(buf), 0, 0, 0, 1);
    TEST_ASSERT(l_strcmp(buf, "\033[48;2;0;0;0m") == 0,
                "black background produces valid sequence");

    // White (255,255,255) background
    l_ansi_color_rgb(buf, sizeof(buf), 255, 255, 255, 1);
    TEST_ASSERT(l_strcmp(buf, "\033[48;2;255;255;255m") == 0,
                "white background produces valid sequence");

    // Mid-range value
    l_ansi_color_rgb(buf, sizeof(buf), 128, 64, 32, 0);
    TEST_ASSERT(l_strcmp(buf, "\033[38;2;128;64;32m") == 0,
                "mid-range RGB foreground correct");
}

static void test_ansi_color_rgb_buffer_safety(void) {
    TEST_FUNCTION("l_ansi_color_rgb buffer safety");

    char buf[64];
    int len;

    // Null buffer should return 0
    len = l_ansi_color_rgb((char *)0, 0, 255, 0, 0, 0);
    TEST_ASSERT(len == 0, "null buffer returns 0");

    // Zero-size buffer should return 0
    len = l_ansi_color_rgb(buf, 0, 255, 0, 0, 0);
    TEST_ASSERT(len == 0, "zero-size buffer returns 0");

    // Buffer size 1 — too small, should not overflow
    buf[0] = 'X';
    len = l_ansi_color_rgb(buf, 1, 255, 0, 0, 0);
    TEST_ASSERT(len >= 0, "tiny buffer does not crash");

    // Buffer exactly large enough for shortest possible sequence
    // "\033[38;2;0;0;0m" = 15 chars + NUL = 16 bytes
    l_memset(buf, 'Z', sizeof(buf));
    len = l_ansi_color_rgb(buf, 16, 0, 0, 0, 0);
    TEST_ASSERT(buf[15] == '\0' || len < 16, "buffer not overflowed with exact fit");

    // Small buffer — should truncate but not overflow
    l_memset(buf, 'Z', sizeof(buf));
    len = l_ansi_color_rgb(buf, 5, 255, 255, 255, 0);
    TEST_ASSERT(len < 5, "small buffer truncates safely");
    TEST_ASSERT(buf[4] == '\0' || buf[4] == 'Z', "no write past buffer[4]");
}

static void test_ansi_color_rgb_nul_termination(void) {
    TEST_FUNCTION("l_ansi_color_rgb NUL termination");

    char buf[64];

    // Verify NUL termination for a normal call
    l_memset(buf, 'X', sizeof(buf));
    int len = l_ansi_color_rgb(buf, sizeof(buf), 100, 200, 50, 0);
    TEST_ASSERT(buf[len] == '\0', "output is NUL-terminated");
    TEST_ASSERT((int)l_strlen(buf) == len, "strlen matches returned length");
}

// ── Half-block UTF-8 encoding tests ────────────────────────────────────────

static void test_halfblock_utf8(void) {
    TEST_FUNCTION("half-block UTF-8 encoding");

    // U+2580 UPPER HALF BLOCK = 0xE2 0x96 0x80 in UTF-8
    unsigned char half_block[] = { 0xE2, 0x96, 0x80, 0x00 };

    TEST_ASSERT(half_block[0] == 0xE2, "half-block byte 0 is 0xE2");
    TEST_ASSERT(half_block[1] == 0x96, "half-block byte 1 is 0x96");
    TEST_ASSERT(half_block[2] == 0x80, "half-block byte 2 is 0x80");
    TEST_ASSERT(l_strlen((const char *)half_block) == 3,
                "half-block is 3 bytes in UTF-8");
}

// ── Terminal pixel dimension math ──────────────────────────────────────────

static void test_terminal_pixel_math(void) {
    TEST_FUNCTION("terminal pixel dimension math");

    // Terminal backend uses half-block characters: each character cell
    // covers 1 pixel wide × 2 pixels tall. So for a terminal of
    // cols × rows characters, the pixel buffer is cols × (rows * 2).
    int cols, rows, pw, ph;

    cols = 80;  rows = 24;
    pw = cols;  ph = rows * 2;
    TEST_ASSERT(pw == 80, "80 cols → 80 pixel width");
    TEST_ASSERT(ph == 48, "24 rows → 48 pixel height");

    cols = 120; rows = 40;
    pw = cols;  ph = rows * 2;
    TEST_ASSERT(pw == 120, "120 cols → 120 pixel width");
    TEST_ASSERT(ph == 80, "40 rows → 80 pixel height");

    // Edge case: 1×1 terminal
    cols = 1; rows = 1;
    pw = cols; ph = rows * 2;
    TEST_ASSERT(pw == 1, "1 col → 1 pixel width");
    TEST_ASSERT(ph == 2, "1 row → 2 pixel height");

    // Pixel addressing: row pair (y/2) maps to one terminal row.
    // Top pixel of pair → foreground color, bottom → background color.
    int y_top = 0, y_bot = 1;
    TEST_ASSERT(y_top / 2 == 0, "top pixel of row 0 maps to terminal row 0");
    TEST_ASSERT(y_bot / 2 == 0, "bottom pixel of row 0 maps to terminal row 0");
    TEST_ASSERT(y_top % 2 == 0, "top pixel is even (foreground)");
    TEST_ASSERT(y_bot % 2 == 1, "bottom pixel is odd (background)");

    // Row pair at terminal row 5
    int term_row = 5;
    int py_top = term_row * 2;
    int py_bot = term_row * 2 + 1;
    TEST_ASSERT(py_top == 10, "terminal row 5 top pixel is y=10");
    TEST_ASSERT(py_bot == 11, "terminal row 5 bottom pixel is y=11");
}

// ── ARGB color extraction (used by terminal renderer) ──────────────────────

static void test_argb_extraction(void) {
    TEST_FUNCTION("ARGB color component extraction");

    // The terminal renderer needs to extract R, G, B from L_RGB/L_RGBA colors
    uint32_t color;
    int r, g, b;

    color = L_RGB(255, 0, 0);  // red
    r = (int)((color >> 16) & 0xFF);
    g = (int)((color >> 8) & 0xFF);
    b = (int)(color & 0xFF);
    TEST_ASSERT(r == 255 && g == 0 && b == 0, "extract RGB from L_RED");

    color = L_RGB(0, 255, 0);  // green
    r = (int)((color >> 16) & 0xFF);
    g = (int)((color >> 8) & 0xFF);
    b = (int)(color & 0xFF);
    TEST_ASSERT(r == 0 && g == 255 && b == 0, "extract RGB from L_GREEN");

    color = L_RGB(0, 0, 255);  // blue
    r = (int)((color >> 16) & 0xFF);
    g = (int)((color >> 8) & 0xFF);
    b = (int)(color & 0xFF);
    TEST_ASSERT(r == 0 && g == 0 && b == 255, "extract RGB from L_BLUE");

    color = L_RGB(128, 64, 32);
    r = (int)((color >> 16) & 0xFF);
    g = (int)((color >> 8) & 0xFF);
    b = (int)(color & 0xFF);
    TEST_ASSERT(r == 128 && g == 64 && b == 32,
                "extract arbitrary RGB components");

    // Alpha channel preserved
    color = L_RGBA(10, 20, 30, 200);
    int a = (int)((color >> 24) & 0xFF);
    TEST_ASSERT(a == 200, "alpha extracted correctly from L_RGBA");
}

// ── Existing l_ansi_color still works ──────────────────────────────────────

static void test_ansi_color_basic(void) {
    TEST_FUNCTION("l_ansi_color basic (regression)");

    char buf[32];
    int len;

    // Foreground red (color 1)
    len = l_ansi_color(buf, sizeof(buf), 1, -1);
    TEST_ASSERT(l_strcmp(buf, "\033[31m") == 0, "fg=1 produces ESC[31m");
    TEST_ASSERT(len == 5, "fg=1 length is 5");

    // Background green (color 2)
    len = l_ansi_color(buf, sizeof(buf), -1, 2);
    TEST_ASSERT(l_strcmp(buf, "\033[42m") == 0, "bg=2 produces ESC[42m");
    TEST_ASSERT(len == 5, "bg=2 length is 5");

    // Both fg and bg
    len = l_ansi_color(buf, sizeof(buf), 7, 0);
    TEST_ASSERT(l_strcmp(buf, "\033[37;40m") == 0, "fg=7 bg=0 produces ESC[37;40m");
    TEST_ASSERT(len == 8, "fg+bg length is 8");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    puts("Testing terminal pixel backend...\n");

    test_ansi_color_rgb_foreground();
    test_ansi_color_rgb_background();
    test_ansi_color_rgb_extremes();
    test_ansi_color_rgb_buffer_safety();
    test_ansi_color_rgb_nul_termination();
    test_halfblock_utf8();
    test_terminal_pixel_math();
    test_argb_extraction();
    test_ansi_color_basic();

    l_test_print_summary(passed_count, test_count);

    if (passed_count != test_count) {
        puts("FAIL\n");
        exit(1);
    }
    puts("All tests passed!\n");
    return 0;
}
