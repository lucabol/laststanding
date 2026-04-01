#define L_MAINFILE
#include "l_gfx.h"

// mandelbrot — Interactive fixed-point fractal renderer
//
// No floating point — uses 28.4 fixed-point arithmetic to stay freestanding.
// Controls: arrow keys = pan, +/- = zoom, r = reset, q = quit
// Usage: mandelbrot

#define W 320
#define H 240

// Fixed-point: 20.12 format (12 fractional bits)
// Range: roughly -524288.0 to +524287.999 with precision of ~0.000244
#define FP_SHIFT 12
#define FP_ONE   (1 << FP_SHIFT)
#define FP_HALF  (FP_ONE / 2)

typedef int fp_t;  // 32-bit — use long long only in fp_mul to avoid overflow

static inline fp_t fp_from_int(int v) { return v << FP_SHIFT; }
static inline fp_t fp_mul(fp_t a, fp_t b) { return (fp_t)(((long long)a * b) >> FP_SHIFT); }

// Color palette — 16 colors cycling through the spectrum
static uint32_t palette[256];

static void init_palette(void) {
    for (int i = 0; i < 256; i++) {
        int r, g, b;
        int t = i % 16;
        switch (t) {
        case  0: r=  0; g=  0; b=  0; break;
        case  1: r= 25; g=  7; b= 26; break;
        case  2: r=  9; g=  1; b= 47; break;
        case  3: r=  4; g=  4; b= 73; break;
        case  4: r=  0; g=  7; b=100; break;
        case  5: r= 12; g= 44; b=138; break;
        case  6: r= 24; g= 82; b=177; break;
        case  7: r= 57; g=125; b=209; break;
        case  8: r=134; g=181; b=229; break;
        case  9: r=211; g=236; b=248; break;
        case 10: r=241; g=233; b=191; break;
        case 11: r=248; g=201; b= 95; break;
        case 12: r=255; g=170; b=  0; break;
        case 13: r=204; g=128; b=  0; break;
        case 14: r=153; g= 87; b=  0; break;
        case 15: r=106; g= 52; b=  3; break;
        default: r=g=b=0; break;
        }
        palette[i] = L_RGB(r, g, b);
    }
}

static int max_iter = 64;

// View window in fixed-point
static fp_t view_cx, view_cy, view_scale;

static void reset_view(void) {
    view_cx    = fp_from_int(-1) + FP_HALF;  // center at -0.5
    view_cy    = fp_from_int(0);              // center at 0
    view_scale = fp_from_int(3);              // width = 3.0 in complex plane
}

static void render(L_Canvas *c) {
    fp_t x_min = view_cx - view_scale / 2;
    fp_t y_min = view_cy - (fp_mul(view_scale, fp_from_int(H)) / W) / 2;
    fp_t step  = view_scale / W;

    for (int py = 0; py < H; py++) {
        fp_t ci = y_min + step * py;
        for (int px = 0; px < W; px++) {
            fp_t cr = x_min + step * px;
            fp_t zr = 0, zi = 0;
            int iter = 0;

            // z = z^2 + c, bail out when |z|^2 > 4
            fp_t bail = fp_from_int(4);
            while (iter < max_iter) {
                fp_t zr2 = fp_mul(zr, zr);
                fp_t zi2 = fp_mul(zi, zi);
                if (zr2 + zi2 > bail) break;
                fp_t new_zr = zr2 - zi2 + cr;
                zi = 2 * fp_mul(zr, zi) + ci;
                zr = new_zr;
                iter++;
            }

            uint32_t color;
            if (iter >= max_iter) {
                color = L_BLACK;
            } else {
                color = palette[iter % 256];
            }
            l_pixel(c, px, py, color);
        }
    }
}

static void draw_hud(L_Canvas *c) {
    l_fill_rect(c, 0, H - 12, W, 12, L_RGB(0, 0, 0));
    l_draw_text(c, 2, H - 10, "Arrows:pan +/-:zoom I:iter R:reset Q:quit", L_RGB(180, 180, 180));
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    L_Canvas canvas;
    if (l_canvas_open(&canvas, W, H, "Mandelbrot") != 0) {
        puts("Cannot open display (no framebuffer or window system)\n");
        puts("This is a graphical demo — run on a system with a display.\n");
        return 0;  // graceful exit for CI
    }

    init_palette();
    reset_view();

    int dirty = 1;
    while (l_canvas_alive(&canvas)) {
        if (dirty) {
            render(&canvas);
            draw_hud(&canvas);
            l_canvas_flush(&canvas);
            dirty = 0;
        }

        int key = l_canvas_key(&canvas);
        if (key == 'q' || key == 'Q' || key == 27) break;

        fp_t pan = view_scale / 8;
        switch (key) {
        case 1001: // left
        case 'a': case 'A':
            view_cx -= pan; dirty = 1; break;
        case 1002: // right
        case 'd': case 'D':
            view_cx += pan; dirty = 1; break;
        case 1003: // up
        case 'w': case 'W':
            view_cy -= pan; dirty = 1; break;
        case 1004: // down
        case 's': case 'S':
            view_cy += pan; dirty = 1; break;
        case '+': case '=':
            view_scale = view_scale * 3 / 4; dirty = 1; break;
        case '-': case '_':
            view_scale = view_scale * 4 / 3; dirty = 1; break;
        case 'i': case 'I':
            max_iter = max_iter < 512 ? max_iter * 2 : max_iter;
            dirty = 1; break;
        case 'r': case 'R':
            reset_view(); max_iter = 64; dirty = 1; break;
        }

        if (!dirty) l_sleep_ms(16);  // ~60fps idle
    }

    l_canvas_close(&canvas);
    return 0;
}
