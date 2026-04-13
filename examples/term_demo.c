// term_demo.c — Terminal pixel graphics demo using half-block rendering
// Renders shapes in any terminal with truecolor support.
// Set L_GFX_TERM=1 on Windows to force terminal mode (Linux auto-detects).

#define L_MAINFILE
#include "l_gfx.h"

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    l_setenv("L_GFX_TERM", "1");

    L_Canvas c;
    if (l_canvas_open(&c, -1, -1, "Terminal Demo") != 0) {
        l_puts("Failed to open terminal canvas\n");
        return 1;
    }

    // Dark background
    l_fill_rect(&c, 0, 0, c.width, c.height, 0xFF1a1a2e);

    // Red rectangle
    int rw = c.width / 4, rh = c.height / 4;
    l_fill_rect(&c, 4, 4, rw, rh, 0xFFe74c3c);

    // Blue circle
    int radius = (c.width < c.height ? c.width : c.height) / 6;
    l_fill_circle(&c, c.width / 2, c.height / 2, radius, 0xFF3498db);

    // Green rectangle
    l_fill_rect(&c, c.width - rw - 4, c.height - rh - 4, rw, rh, 0xFF2ecc71);

    // Label
    l_draw_text(&c, 2, 2, "Terminal GFX!", 0xFFffffff);

    l_canvas_flush(&c);

    // Wait for Escape or 'q'
    while (l_canvas_alive(&c)) {
        int key = l_canvas_key(&c);
        if (key == 27 || key == 'q') break;
        l_sleep_ms(16);
    }

    l_canvas_close(&c);
    return 0;
}
