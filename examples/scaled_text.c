#define L_MAINFILE
#include "l_gfx.h"

// scaled_text — Scalable text demo using l_draw_text_scaled.
// Displays text at multiple sizes. Exit: press Escape.

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas c;
    if (l_canvas_open(&c, 640, 400, "Scaled Text Demo") != 0) return 1;
    int s = c.scale;

    while (l_canvas_alive(&c)) {
        l_canvas_clear(&c, L_BLACK);

        l_draw_text_scaled(&c, 10 * s, 10 * s, "1x: Hello, laststanding!",
                           L_WHITE, s, s);
        l_draw_text_scaled(&c, 10 * s, 30 * s, "2x: Hello!", L_GREEN, 2 * s, 2 * s);
        l_draw_text_scaled(&c, 10 * s, 70 * s, "3x: Hello!", L_CYAN, 3 * s, 3 * s);
        l_draw_text_scaled(&c, 10 * s, 110 * s, "4x: Big!", L_YELLOW, 4 * s, 4 * s);
        l_draw_text_scaled(&c, 10 * s, 170 * s, "6x", L_RED, 6 * s, 6 * s);
        l_draw_text_scaled(&c, 10 * s, 250 * s, "Wide!", L_MAGENTA, 4 * s, 2 * s);
        l_draw_text_scaled(&c, 10 * s, 290 * s, "Tall!", L_BLUE, 1 * s, 4 * s);

        l_canvas_flush(&c);
        if (l_canvas_key(&c) == 27) break;
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
