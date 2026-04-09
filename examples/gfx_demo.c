#define L_MAINFILE
#include "l_gfx.h"

// Static drawing demo from the README.
// Exit: press Escape

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas c;
    if (l_canvas_open(&c, 320, 240, "Demo") != 0) return 1;

    l_canvas_clear(&c, L_BLACK);
    l_fill_rect(&c, 50, 50, 100, 80, L_RED);
    l_circle(&c, 200, 120, 60, L_GREEN);
    l_draw_text(&c, 60, 85, "Hello!", L_WHITE);
    l_canvas_flush(&c);

    while (l_canvas_alive(&c) && l_canvas_key(&c) != 27)
        l_sleep_ms(16);
    l_canvas_close(&c);
    return 0;
}
