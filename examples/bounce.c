#define L_MAINFILE
#include "l_gfx.h"

// Bouncing ball — animation loop demo from the README.
// Usage: bounce [-f]    (-f = fullscreen)
// Exit: press Escape

int main(int argc, char *argv[]) {
    int fullscreen = 0;
    for (int i = 1; i < argc; i++)
        if (l_strcmp(argv[i], "-f") == 0) fullscreen = 1;

    int w = fullscreen ? 0 : 320;
    int h = fullscreen ? 0 : 240;

    L_Canvas c;
    if (l_canvas_open(&c, w, h, "Bounce") != 0) return 1;

    int x = c.width / 2, y = c.height / 2, dx = 2, dy = 1, r = 10;
    while (l_canvas_alive(&c)) {
        if (l_canvas_key(&c) == 27) break;
        l_canvas_clear(&c, L_BLACK);
        l_fill_circle(&c, x, y, r, L_RED);
        l_canvas_flush(&c);
        x += dx; y += dy;
        if (x <= r || x >= c.width - r) dx = -dx;
        if (y <= r || y >= c.height - r) dy = -dy;
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
