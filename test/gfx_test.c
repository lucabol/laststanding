#define L_MAINFILE
#include "l_gfx.h"

// test/gfx_test.c — Smoke test for l_gfx.h pixel graphics
//
// When a display is available, opens a canvas, draws shapes, and exits.
// When no display is available (CI), tests pixel-buffer operations in memory.

static int tests_run = 0;
static int tests_passed = 0;

static void check(int cond, const char *desc) {
    tests_run++;
    if (cond) {
        tests_passed++;
    } else {
        puts("  [FAIL] ");
        puts(desc);
        puts("\n");
    }
}

// Test pixel-buffer operations without needing a display
static void test_in_memory(void) {
    // Allocate a small pixel buffer manually
    int w = 64, h = 64;
    uint32_t buf[64 * 64];
    L_Canvas c;
    memset(&c, 0, sizeof(c));
    c.width  = w;
    c.height = h;
    c.stride = w * 4;
    c.pixels = buf;

    // Test clear
    l_canvas_clear(&c, L_BLACK);
    check(c.pixels[0] == L_BLACK, "clear sets first pixel");
    check(c.pixels[w * h - 1] == L_BLACK, "clear sets last pixel");

    // Test pixel set/get
    l_pixel(&c, 10, 10, L_RED);
    check(l_get_pixel(&c, 10, 10) == L_RED, "pixel set/get");

    // Test out-of-bounds (should not crash)
    l_pixel(&c, -1, -1, L_WHITE);
    l_pixel(&c, w, h, L_WHITE);
    check(l_get_pixel(&c, -1, -1) == 0, "out-of-bounds get returns 0");

    // Test fill_rect
    l_fill_rect(&c, 0, 0, 8, 8, L_GREEN);
    check(l_get_pixel(&c, 0, 0) == L_GREEN, "fill_rect top-left");
    check(l_get_pixel(&c, 7, 7) == L_GREEN, "fill_rect bottom-right");
    check(l_get_pixel(&c, 8, 8) != L_GREEN, "fill_rect doesn't exceed bounds");

    // Test line (horizontal)
    l_canvas_clear(&c, L_BLACK);
    l_line(&c, 0, 0, 10, 0, L_BLUE);
    check(l_get_pixel(&c, 0, 0) == L_BLUE, "hline start");
    check(l_get_pixel(&c, 5, 0) == L_BLUE, "hline middle");
    check(l_get_pixel(&c, 10, 0) == L_BLUE, "hline end");

    // Test line (vertical)
    l_canvas_clear(&c, L_BLACK);
    l_line(&c, 0, 0, 0, 10, L_RED);
    check(l_get_pixel(&c, 0, 0) == L_RED, "vline start");
    check(l_get_pixel(&c, 0, 5) == L_RED, "vline middle");
    check(l_get_pixel(&c, 0, 10) == L_RED, "vline end");

    // Test rect outline
    l_canvas_clear(&c, L_BLACK);
    l_rect(&c, 5, 5, 10, 10, L_WHITE);
    check(l_get_pixel(&c, 5, 5) == L_WHITE, "rect corner");
    check(l_get_pixel(&c, 14, 14) == L_WHITE, "rect opposite corner");
    check(l_get_pixel(&c, 9, 9) == L_BLACK, "rect interior empty");

    // Test circle
    l_canvas_clear(&c, L_BLACK);
    l_circle(&c, 32, 32, 10, L_CYAN);
    check(l_get_pixel(&c, 42, 32) == L_CYAN, "circle right point");
    check(l_get_pixel(&c, 32, 32) == L_BLACK, "circle center empty");

    // Test fill_circle
    l_canvas_clear(&c, L_BLACK);
    l_fill_circle(&c, 32, 32, 5, L_YELLOW);
    check(l_get_pixel(&c, 32, 32) == L_YELLOW, "fill_circle center");
    check(l_get_pixel(&c, 32, 27) == L_YELLOW, "fill_circle top");

    // Test draw_char
    l_canvas_clear(&c, L_BLACK);
    l_draw_char(&c, 0, 0, 'A', L_WHITE);
    // 'A' glyph row 0 = 0x0C = 00001100 → with MSB=left: pixels at col 4 and 5
    check(l_get_pixel(&c, 4, 0) == L_WHITE, "draw_char 'A' row0 bit");
    check(l_get_pixel(&c, 0, 0) == L_BLACK, "draw_char 'A' row0 empty bit");

    // Test draw_text
    l_canvas_clear(&c, L_BLACK);
    l_draw_text(&c, 0, 0, "Hi", L_RED);
    // 'H' glyph row 0 = 0x33 = 00110011 → pixels at col 2,3,6,7
    check(l_get_pixel(&c, 2, 0) == L_RED, "draw_text first char");
    // 'i' at x=8: glyph row 0 = 0x0C → pixels at col 4,5 → x=12,13
    check(l_get_pixel(&c, 12, 0) == L_RED, "draw_text second char");

    // Test color macros
    check(L_RGB(255, 0, 0) == L_RED, "L_RGB red");
    check(L_RGB(0, 255, 0) == L_GREEN, "L_RGB green");
    check(L_RGB(0, 0, 255) == L_BLUE, "L_RGB blue");
    check(L_RGBA(255, 255, 255, 255) == L_WHITE, "L_RGBA white");
}

static void print_num(int n) {
    char buf[16];
    itoa(n, buf, sizeof(buf));
    puts(buf);
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    puts("Testing l_gfx.h pixel graphics (in-memory)...\n");
    test_in_memory();

    puts("  ");
    print_num(tests_passed);
    puts("/");
    print_num(tests_run);
    puts(" tests passed\n");

    if (tests_passed != tests_run) {
        puts("FAIL\n");
        exit(1);
    }
    puts("PASS\n");
    return 0;
}
