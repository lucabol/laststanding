#define L_MAINFILE
#include "l_gfx.h"

// starfield.c — 3D starfield simulation
// 320x240 canvas, 200 stars flying toward viewer, Q/ESC=quit

#define W 320
#define H 240
#define CX 160
#define CY 120
#define NUM_STARS 200
#define Z_FAR 1024
#define Z_NEAR 2

static int star_x[NUM_STARS];
static int star_y[NUM_STARS];
static int star_z[NUM_STARS];

static int rng_range(int lo, int hi) {
    return lo + (int)(l_rand() % (unsigned int)(hi - lo + 1));
}

static void spawn_star(int i) {
    star_x[i] = rng_range(-CX * 4, CX * 4);
    star_y[i] = rng_range(-CY * 4, CY * 4);
    star_z[i] = rng_range(Z_NEAR + 1, Z_FAR);
}

static void init_stars(void) {
    for (int i = 0; i < NUM_STARS; i++)
        spawn_star(i);
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas canvas;
    if (l_canvas_open(&canvas, W, H, "Starfield") != 0) {
        puts("No display available\n");
        return 0;
    }

    init_stars();
    int speed = 4;

    while (l_canvas_alive(&canvas)) {
        int key = l_canvas_key(&canvas);
        if (key == 'q' || key == 'Q' || key == 27) break;
        if (key == '+' || key == '=') { if (speed < 16) speed++; }
        if (key == '-' || key == '_') { if (speed > 1) speed--; }

        l_canvas_clear(&canvas, L_BLACK);

        int cx = canvas.width / 2;
        int cy = canvas.height / 2;
        int s = canvas.scale;

        for (int i = 0; i < NUM_STARS; i++) {
            star_z[i] -= speed;
            if (star_z[i] <= Z_NEAR) {
                spawn_star(i);
                continue;
            }

            int sx = (star_x[i] * 256) / star_z[i] + cx;
            int sy = (star_y[i] * 256) / star_z[i] + cy;

            if (sx < 0 || sx >= canvas.width || sy < 0 || sy >= canvas.height) {
                spawn_star(i);
                continue;
            }

            // Brightness: closer = brighter
            int bright = 255 - (star_z[i] * 255 / Z_FAR);
            if (bright < 40) bright = 40;
            if (bright > 255) bright = 255;
            uint32_t color = L_RGB(bright, bright, bright);

            // Draw bigger pixels for closer stars
            if (star_z[i] < Z_FAR / 4) {
                l_fill_rect(&canvas, sx, sy, 2 * s, 2 * s, color);
            } else if (s > 1) {
                l_fill_rect(&canvas, sx, sy, s, s, color);
            } else {
                l_pixel(&canvas, sx, sy, color);
            }
        }

        l_draw_text_scaled(&canvas, 2 * s, canvas.height - 10 * s,
                           "+/-:speed Q:quit", L_RGB(100, 100, 100), s, s);
        l_canvas_flush(&canvas);
        l_sleep_ms(16);
    }

    l_canvas_close(&canvas);
    return 0;
}
