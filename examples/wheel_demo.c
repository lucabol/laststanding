#define L_MAINFILE
#include "l_gfx.h"

// Mouse wheel demo — scroll the wheel to grow/shrink a circle that follows
// the cursor. Left-click resets the radius. Press Escape to exit.
// Supported on Windows (WM_MOUSEWHEEL) and Linux X11 (buttons 4/5).
// On framebuffer/terminal backends the wheel reads as 0.

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas c;
    if (l_canvas_open(&c, 480, 360, "Wheel demo") != 0) return 1;
    int s = c.scale;

    int radius = 20 * s;
    int total = 0;
    char label[64];

    while (l_canvas_alive(&c)) {
        if (l_canvas_key(&c) == 27) break;

        int mx, my;
        int btn = l_canvas_mouse(&c, &mx, &my);
        int wheel = l_canvas_wheel(&c);

        if (wheel) {
            radius += wheel * 4 * s;
            if (radius < 2 * s)   radius = 2 * s;
            if (radius > 200 * s) radius = 200 * s;
            total += wheel;
        }
        if (btn & 1) { radius = 20 * s; total = 0; }

        l_canvas_clear(&c, L_BLACK);
        l_fill_circle(&c, mx, my, radius, L_CYAN);

        l_itoa(total, label, 10);
        l_draw_text_scaled(&c, 8 * s, 8 * s, "wheel total: ",
                           L_WHITE, s, s);
        l_draw_text_scaled(&c, (8 + 13 * 8) * s, 8 * s, label,
                           L_YELLOW, s, s);
        l_draw_text_scaled(&c, 8 * s, 20 * s,
                           "scroll: resize  |  click: reset  |  esc: quit",
                           L_WHITE, s, s);

        l_canvas_flush(&c);
        l_sleep_ms(16);
    }

    l_canvas_close(&c);
    return 0;
}
