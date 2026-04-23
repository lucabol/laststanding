#define L_MAINFILE
#include "l_gfx.h"

// Static drawing demo from the README.
// Exit: press Escape. Window is resizable — content redraws to fit.

static void draw_scene(L_Canvas *c) {
    int s = c->scale;
    l_canvas_clear(c, L_BLACK);
    // Scale shapes relative to canvas size
    int w = c->width, h = c->height;
    l_fill_rect(c, w * 50 / 320, h * 50 / 240, w * 100 / 320, h * 80 / 240, L_RED);
    l_circle(c, w * 200 / 320, h * 120 / 240, (w < h ? w : h) * 60 / 240, L_GREEN);
    l_draw_text_scaled(c, w * 60 / 320, h * 85 / 240, "Hello!", L_WHITE, s, s);
    l_canvas_flush(c);
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas c;
    if (l_canvas_open(&c, 320, 240, "Demo") != 0) return 1;

    draw_scene(&c);

    while (l_canvas_alive(&c) && l_canvas_key(&c) != 27) {
        if (l_canvas_resized(&c))
            draw_scene(&c);
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
