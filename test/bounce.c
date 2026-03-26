#define L_MAINFILE
#include "l_gfx.h"

// Bouncing ball — animation loop demo from the README.
// Exit: press Escape

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    L_Canvas c;
    if (l_canvas_open(&c, 320, 240, "Bounce") != 0) return 1;

    int x = 160, y = 120, dx = 2, dy = 1;
    while (l_canvas_alive(&c)) {
        if (l_canvas_key(&c) == 27) break;
        l_canvas_clear(&c, L_BLACK);
        l_fill_circle(&c, x, y, 10, L_RED);
        l_canvas_flush(&c);
        x += dx; y += dy;
        if (x <= 10 || x >= 310) dx = -dx;
        if (y <= 10 || y >= 230) dy = -dy;
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
