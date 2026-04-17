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

    int radius = 20;
    int total = 0;
    char label[64];

    while (l_canvas_alive(&c)) {
        if (l_canvas_key(&c) == 27) break;

        int mx, my;
        int btn = l_canvas_mouse(&c, &mx, &my);
        int wheel = l_canvas_wheel(&c);

        if (wheel) {
            radius += wheel * 4;
            if (radius < 2)   radius = 2;
            if (radius > 200) radius = 200;
            total += wheel;
        }
        if (btn & 1) { radius = 20; total = 0; }

        l_canvas_clear(&c, L_BLACK);
        l_fill_circle(&c, mx, my, radius, L_CYAN);

        l_itoa(total, label, 10);
        l_draw_text(&c, 8, 8, "wheel total: ", L_WHITE);
        l_draw_text(&c, 8 + 13 * 8, 8, label, L_YELLOW);
        l_draw_text(&c, 8, 20, "scroll: resize  |  click: reset  |  esc: quit", L_WHITE);

        l_canvas_flush(&c);
        l_sleep_ms(16);
    }

    l_canvas_close(&c);
    return 0;
}
