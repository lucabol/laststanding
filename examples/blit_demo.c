#define L_MAINFILE
#include "l_gfx.h"

// blit_demo — Demonstrates l_blit and l_blit_alpha.
// Draws a small sprite and blits it at various positions. Exit: Escape.

// 8x8 smiley sprite (ARGB)
static uint32_t smiley[64];

static void make_smiley(void) {
    // Yellow circle with black features
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++) {
            int dx = x - 3, dy = y - 3;
            if (dx * dx + dy * dy <= 12)
                smiley[y * 8 + x] = L_YELLOW;
            else
                smiley[y * 8 + x] = L_RGBA(0, 0, 0, 0); // transparent
        }
    // Eyes
    smiley[2 * 8 + 2] = L_BLACK;
    smiley[2 * 8 + 5] = L_BLACK;
    // Mouth
    smiley[5 * 8 + 2] = L_BLACK;
    smiley[5 * 8 + 3] = L_BLACK;
    smiley[5 * 8 + 4] = L_BLACK;
    smiley[5 * 8 + 5] = L_BLACK;
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    make_smiley();

    L_Canvas c;
    if (l_canvas_open(&c, 320, 240, "Blit Demo") != 0) return 1;
    int s = c.scale;

    while (l_canvas_alive(&c)) {
        l_canvas_clear(&c, L_RGB(32, 32, 64));

        l_draw_text_scaled(&c, 10 * s, 10 * s, "l_blit (opaque):",
                           L_WHITE, s, s);
        for (int i = 0; i < 10; i++)
            l_blit(&c, (10 + i * 20) * s, 30 * s, 8, 8, smiley, 8 * 4);

        l_draw_text_scaled(&c, 10 * s, 60 * s, "l_blit_alpha (blended):",
                           L_WHITE, s, s);
        // Draw a gradient background, then blit with alpha
        l_fill_rect(&c, 10 * s, 80 * s, 200 * s, 30 * s, L_RED);
        l_fill_rect(&c, 110 * s, 80 * s, 100 * s, 30 * s, L_GREEN);
        for (int i = 0; i < 10; i++)
            l_blit_alpha(&c, (10 + i * 20) * s, 85 * s, 8, 8, smiley, 8 * 4);

        l_draw_text_scaled(&c, 10 * s, 130 * s, "Scaled + Blit!",
                           L_CYAN, 2 * s, 2 * s);

        l_canvas_flush(&c);
        if (l_canvas_key(&c) == 27) break;
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
