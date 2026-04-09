#define L_MAINFILE
#include "l_gfx.h"

// plasma.c — Classic plasma effect using fixed-point sine tables
// 320x240 canvas, animated rainbow plasma, Q/ESC=quit

#define W 320
#define H 240

// Sine table: 256 entries, values 0..255 (128 = zero crossing)
static const uint8_t sin_table[256] = {
    128,131,134,137,140,143,146,149,152,155,158,162,165,167,170,173,
    176,179,182,185,188,190,193,196,198,201,203,206,208,211,213,215,
    218,220,222,224,226,228,230,232,234,235,237,239,240,241,243,244,
    245,246,248,249,250,250,251,252,253,253,254,254,254,255,255,255,
    255,255,255,255,254,254,254,253,253,252,251,250,250,249,248,246,
    245,244,243,241,240,239,237,235,234,232,230,228,226,224,222,220,
    218,215,213,211,208,206,203,201,198,196,193,190,188,185,182,179,
    176,173,170,167,165,162,158,155,152,149,146,143,140,137,134,131,
    128,125,122,119,116,113,110,107,104,101, 98, 94, 91, 89, 86, 83,
     80, 77, 74, 71, 68, 66, 63, 60, 58, 55, 53, 50, 48, 45, 43, 41,
     38, 36, 34, 32, 30, 28, 26, 24, 22, 21, 19, 17, 16, 15, 13, 12,
     11, 10,  8,  7,  6,  6,  5,  4,  3,  3,  2,  2,  2,  1,  1,  1,
      1,  1,  1,  1,  2,  2,  2,  3,  3,  4,  5,  6,  6,  7,  8, 10,
     11, 12, 13, 15, 16, 17, 19, 21, 22, 24, 26, 28, 30, 32, 34, 36,
     38, 41, 43, 45, 48, 50, 53, 55, 58, 60, 63, 66, 68, 71, 74, 77,
     80, 83, 86, 89, 91, 94, 98,101,104,107,110,113,116,119,122,125
};

// Rainbow palette: 256 entries cycling through HSV hues
static uint32_t palette[256];

static void init_palette(void) {
    for (int i = 0; i < 256; i++) {
        int r, g, b;
        int phase = i % 256;
        // 6-segment HSV rainbow approximation using integer math
        int seg = phase / 43;        // 0..5
        int frac = (phase % 43) * 6; // 0..252
        switch (seg) {
        case 0: r = 255;       g = frac;      b = 0;         break;
        case 1: r = 255 - frac; g = 255;      b = 0;         break;
        case 2: r = 0;         g = 255;       b = frac;      break;
        case 3: r = 0;         g = 255 - frac; b = 255;      break;
        case 4: r = frac;      g = 0;         b = 255;       break;
        default: r = 255;      g = 0;         b = 255 - frac; break;
        }
        palette[i] = L_RGB(r, g, b);
    }
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas canvas;
    if (l_canvas_open(&canvas, W, H, "Plasma") != 0) {
        puts("No display available\n");
        return 0;
    }

    init_palette();
    int t = 0;

    while (l_canvas_alive(&canvas)) {
        int key = l_canvas_key(&canvas);
        if (key == 'q' || key == 'Q' || key == 27) break;

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                // Combine several sine waves at different frequencies
                int v1 = sin_table[(x + t) & 255];
                int v2 = sin_table[((y * 3 + t * 2) >> 1) & 255];
                int v3 = sin_table[((x + y + t) >> 1) & 255];
                int v4 = sin_table[((x * 2 - y + t * 3) >> 2) & 255];
                int idx = (v1 + v2 + v3 + v4) >> 2;  // average to 0..255
                l_pixel(&canvas, x, y, palette[idx & 255]);
            }
        }

        l_draw_text(&canvas, 2, H - 10, "Q:quit", L_RGB(0, 0, 0));
        l_canvas_flush(&canvas);
        l_sleep_ms(16);
        t++;
    }

    l_canvas_close(&canvas);
    return 0;
}
