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

        // Also show the icon inside the window so you can see what was set.
        // Draw a few scaled copies, nearest-neighbor style, via l_blit_alpha.
        int s = c.scale;
        int x0 = (c.width  - ICON_SZ * 4 * s) / 2;
        int y0 = (c.height - ICON_SZ * 4 * s) / 2;
        // Manual nearest-neighbor upscale of the 32x32 ARGB icon.
        int scale = 4 * s;
        for (int yy = 0; yy < ICON_SZ * scale; yy++) {
            int sy = yy / scale;
            for (int xx = 0; xx < ICON_SZ * scale; xx++) {
                int sx = xx / scale;
                uint32_t p = icon[sy * ICON_SZ + sx];
                if ((p >> 24) != 0) l_pixel(&c, x0 + xx, y0 + yy, p);
            }
        }

        l_draw_text_scaled(&c, 8 * s, 8 * s,
            "l_canvas_set_icon: see title bar / taskbar", L_WHITE, s, s);
        l_draw_text_scaled(&c, 8 * s, c.height - 16 * s,
            "Esc to exit", 0xFF808080, s, s);

        l_canvas_flush(&c);
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
