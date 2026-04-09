#define L_MAINFILE
#include "l_gfx.h"
#include "test_support.h"

// test/gfx_test.c — Smoke test for l_gfx.h pixel graphics
//
// When a display is available, opens a canvas, draws shapes, and exits.
// When no display is available (CI), tests pixel-buffer operations in memory.

TEST_DECLARE_COUNTERS();

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
    TEST_CHECK(c.pixels[0] == L_BLACK, "clear sets first pixel");
    TEST_CHECK(c.pixels[w * h - 1] == L_BLACK, "clear sets last pixel");

    // Test pixel set/get
    l_pixel(&c, 10, 10, L_RED);
    TEST_CHECK(l_get_pixel(&c, 10, 10) == L_RED, "pixel set/get");

    // Test out-of-bounds (should not crash)
    l_pixel(&c, -1, -1, L_WHITE);
    l_pixel(&c, w, h, L_WHITE);
    TEST_CHECK(l_get_pixel(&c, -1, -1) == 0, "out-of-bounds get returns 0");

    // Test fill_rect
    l_fill_rect(&c, 0, 0, 8, 8, L_GREEN);
    TEST_CHECK(l_get_pixel(&c, 0, 0) == L_GREEN, "fill_rect top-left");
    TEST_CHECK(l_get_pixel(&c, 7, 7) == L_GREEN, "fill_rect bottom-right");
    TEST_CHECK(l_get_pixel(&c, 8, 8) != L_GREEN, "fill_rect doesn't exceed bounds");

    // Test line (horizontal)
    l_canvas_clear(&c, L_BLACK);
    l_line(&c, 0, 0, 10, 0, L_BLUE);
    TEST_CHECK(l_get_pixel(&c, 0, 0) == L_BLUE, "hline start");
    TEST_CHECK(l_get_pixel(&c, 5, 0) == L_BLUE, "hline middle");
    TEST_CHECK(l_get_pixel(&c, 10, 0) == L_BLUE, "hline end");

    // Test line (vertical)
    l_canvas_clear(&c, L_BLACK);
    l_line(&c, 0, 0, 0, 10, L_RED);
    TEST_CHECK(l_get_pixel(&c, 0, 0) == L_RED, "vline start");
    TEST_CHECK(l_get_pixel(&c, 0, 5) == L_RED, "vline middle");
    TEST_CHECK(l_get_pixel(&c, 0, 10) == L_RED, "vline end");

    // Test rect outline
    l_canvas_clear(&c, L_BLACK);
    l_rect(&c, 5, 5, 10, 10, L_WHITE);
    TEST_CHECK(l_get_pixel(&c, 5, 5) == L_WHITE, "rect corner");
    TEST_CHECK(l_get_pixel(&c, 14, 14) == L_WHITE, "rect opposite corner");
    TEST_CHECK(l_get_pixel(&c, 9, 9) == L_BLACK, "rect interior empty");

    // Test circle
    l_canvas_clear(&c, L_BLACK);
    l_circle(&c, 32, 32, 10, L_CYAN);
    TEST_CHECK(l_get_pixel(&c, 42, 32) == L_CYAN, "circle right point");
    TEST_CHECK(l_get_pixel(&c, 32, 32) == L_BLACK, "circle center empty");

    // Test fill_circle
    l_canvas_clear(&c, L_BLACK);
    l_fill_circle(&c, 32, 32, 5, L_YELLOW);
    TEST_CHECK(l_get_pixel(&c, 32, 32) == L_YELLOW, "fill_circle center");
    TEST_CHECK(l_get_pixel(&c, 32, 27) == L_YELLOW, "fill_circle top");

    // Test draw_char
    l_canvas_clear(&c, L_BLACK);
    l_draw_char(&c, 0, 0, 'A', L_WHITE);
    // 'A' glyph row 0 = 0x0C = 00001100 → with LSB=left: pixels at col 2 and 3
    TEST_CHECK(l_get_pixel(&c, 2, 0) == L_WHITE, "draw_char 'A' row0 bit");
    TEST_CHECK(l_get_pixel(&c, 0, 0) == L_BLACK, "draw_char 'A' row0 empty bit");

    // Test draw_text
    l_canvas_clear(&c, L_BLACK);
    l_draw_text(&c, 0, 0, "Hi", L_RED);
    // 'H' glyph row 0 = 0x33 = 00110011 → LSB=left: pixels at col 0,1,4,5
    TEST_CHECK(l_get_pixel(&c, 0, 0) == L_RED, "draw_text first char");
    // 'i' at x=8: glyph row 0 = 0x0C → LSB=left: pixels at col 2,3 → x=10,11
    TEST_CHECK(l_get_pixel(&c, 10, 0) == L_RED, "draw_text second char");

    // Test color macros
    TEST_CHECK(L_RGB(255, 0, 0) == L_RED, "L_RGB red");
    TEST_CHECK(L_RGB(0, 255, 0) == L_GREEN, "L_RGB green");
    TEST_CHECK(L_RGB(0, 0, 255) == L_BLUE, "L_RGB blue");
    TEST_CHECK(L_RGBA(255, 255, 255, 255) == L_WHITE, "L_RGBA white");
}

static void test_backend_helpers(void) {
    L_Canvas c;
    int mx = -1;
    int my = -1;

    memset(&c, 0, sizeof(c));

#ifdef _WIN32
    c.closed = 1;
    TEST_CHECK(l_canvas_alive(&c) == 0, "canvas_alive false when closed");
    c.closed = 0;
    TEST_CHECK(l_canvas_alive(&c) == 1, "canvas_alive true when open");
    c.keys[0] = 'A';
    c.key_head = 1;
    TEST_CHECK(l_canvas_key(&c) == 'A', "canvas_key drains queued key");
    TEST_CHECK(l_canvas_key(&c) == 0, "canvas_key returns 0 when queue empty");
#else
    c.mouse_fd = -1;
    TEST_CHECK(l_canvas_alive(&c) == 0, "canvas_alive false without framebuffer");
    c.fb_mem = (uint8_t *)1;
    TEST_CHECK(l_canvas_alive(&c) == 1, "canvas_alive true with framebuffer memory");
    c.fb_mem = 0;
#endif

    c.mouse_x = 12;
    c.mouse_y = 34;
    c.mouse_btn = 3;
    TEST_CHECK(l_canvas_mouse(&c, &mx, &my) == 3 &&
               mx == 12 && my == 34,
               "canvas_mouse reports cached state");

    memset(&c, 0, sizeof(c));
#ifndef _WIN32
    c.mouse_fd = -1;
#endif
    l_canvas_close(&c);
    TEST_CHECK(c.width == 0 && c.height == 0 && c.pixels == 0,
               "canvas_close tolerates unopened canvas");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    puts("Testing l_gfx.h pixel graphics (in-memory)...\n");
    test_in_memory();
    test_backend_helpers();

    l_test_print_summary(passed_count, test_count);

    if (passed_count != test_count) {
        puts("FAIL\n");
        exit(1);
    }
    puts("PASS\n");
    return 0;
}
