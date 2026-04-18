#define L_MAINFILE
#include "l_gfx.h"

// Demonstrates l_canvas_set_icon: paints a 32x32 ARGB icon in memory and
// applies it to the window (shows up in the taskbar, Alt-Tab, and title bar
// on Windows; in the taskbar/window list via _NET_WM_ICON on X11).
//
// Press ESC to exit.

#define ICON_SZ 32

static void build_icon(uint32_t *px) {
    int cx = ICON_SZ / 2, cy = ICON_SZ / 2;
    int r2 = (ICON_SZ / 2 - 1) * (ICON_SZ / 2 - 1);
    for (int y = 0; y < ICON_SZ; y++) {
        for (int x = 0; x < ICON_SZ; x++) {
            int dx = x - cx, dy = y - cy;
            int d2 = dx * dx + dy * dy;
            if (d2 > r2) {
                px[y * ICON_SZ + x] = 0x00000000u; // transparent
            } else {
                // Purple-to-cyan gradient disk with a white highlight.
                uint32_t r = (uint32_t)(40 + (x * 200) / ICON_SZ);
                uint32_t g = (uint32_t)(40 + (y * 200) / ICON_SZ);
                uint32_t b = 220;
                if (dx >= -6 && dx <= -2 && dy >= -6 && dy <= -2) {
                    r = g = b = 255;
                }
                px[y * ICON_SZ + x] = 0xFF000000u | (r << 16) | (g << 8) | b;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    L_Canvas c;
    if (l_canvas_open(&c, 320, 200, "icon_demo") != 0) return 1;

    uint32_t icon[ICON_SZ * ICON_SZ];
    build_icon(icon);
    if (l_canvas_set_icon(&c, icon, ICON_SZ, ICON_SZ) != 0) {
        puts("l_canvas_set_icon failed (unsupported backend?)\n");
    } else {
        puts("icon set; check the window title bar / taskbar\n");
    }

    while (l_canvas_alive(&c)) {
        if (l_canvas_key(&c) == 27) break;
        l_canvas_clear(&c, L_RGB(30, 30, 60));
        l_canvas_flush(&c);
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
