#define L_MAINFILE
#include "l_gfx.h"

// scaled_text — Scalable text demo using l_draw_text_scaled.
// Displays text at multiple sizes. Exit: press Escape.

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas c;
    if (l_canvas_open(&c, 640, 400, "Scaled Text Demo") != 0) return 1;

    while (l_canvas_alive(&c)) {
        l_canvas_clear(&c, L_BLACK);

        l_draw_text(&c, 10, 10, "1x: Hello, laststanding!", L_WHITE);
        l_draw_text_scaled(&c, 10, 30, "2x: Hello!", L_GREEN, 2, 2);
        l_draw_text_scaled(&c, 10, 70, "3x: Hello!", L_CYAN, 3, 3);
        l_draw_text_scaled(&c, 10, 110, "4x: Big!", L_YELLOW, 4, 4);
        l_draw_text_scaled(&c, 10, 170, "6x", L_RED, 6, 6);
        l_draw_text_scaled(&c, 10, 250, "Wide!", L_MAGENTA, 4, 2);
        l_draw_text_scaled(&c, 10, 290, "Tall!", L_BLUE, 1, 4);

        l_canvas_flush(&c);
        if (l_canvas_key(&c) == 27) break;
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
