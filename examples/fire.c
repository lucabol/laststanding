#define L_MAINFILE
#include "l_gfx.h"

// fire.c — Doom-style fire effect
// 320x240 canvas, bottom-up fire propagation, Q/ESC=quit

#define W 320
#define H 240
#define PAL_SIZE 37

// Fire buffer: each cell holds a palette index 0..36
static uint8_t fire[H][W];

// Fire palette: black -> red -> orange -> yellow -> white
static uint32_t fire_palette[PAL_SIZE];

static void init_palette(void) {
    // Manually define the 37-entry doom fire palette
    static const uint8_t pal[PAL_SIZE][3] = {
        {  7,  7,  7}, { 31,  7,  7}, { 47, 15,  7}, { 71, 15,  7},
        { 87, 23,  7}, {103, 31,  7}, {119, 31,  7}, {143, 39,  7},
        {159, 47,  7}, {175, 63,  7}, {191, 71,  7}, {199, 71,  7},
        {223, 79,  7}, {223, 87,  7}, {223, 87,  7}, {215, 95,  7},
        {215,103, 15}, {207,111, 15}, {207,119, 15}, {207,127, 15},
        {207,135, 23}, {199,135, 23}, {199,143, 23}, {199,151, 31},
        {191,159, 31}, {191,159, 31}, {191,167, 39}, {191,167, 39},
        {191,175, 47}, {183,175, 47}, {183,183, 47}, {183,183, 55},
        {207,207,111}, {223,223,159}, {239,239,199}, {255,255,231},
        {255,255,255}
    };
    for (int i = 0; i < PAL_SIZE; i++)
        fire_palette[i] = L_RGB(pal[i][0], pal[i][1], pal[i][2]);
}

static void init_fire(void) {
    l_memset(fire, 0, sizeof(fire));
    // Set bottom row to max heat
    for (int x = 0; x < W; x++)
        fire[H - 1][x] = PAL_SIZE - 1;
}

static void spread_fire(void) {
    for (int y = 0; y < H - 1; y++) {
        for (int x = 0; x < W; x++) {
            int src_y = y + 1;
            int rand_offset = (int)(l_rand() % 3) - 1;  // -1, 0, or 1
            int src_x = x + rand_offset;
            if (src_x < 0) src_x = 0;
            if (src_x >= W) src_x = W - 1;

            int decay = (int)(l_rand() & 1);  // 0 or 1
            int val = fire[src_y][src_x] - decay;
            if (val < 0) val = 0;
            fire[y][x] = (uint8_t)val;
        }
    }
}

static void draw(L_Canvas *c) {
    // Stretch the fixed 320x240 fire buffer to fill the whole canvas with
    // nearest-neighbor sampling so it looks right at any window size / DPI.
    int cw = c->width, ch = c->height;
    for (int dy = 0; dy < ch; dy++) {
        int sy = (dy * H) / ch;
        for (int dx = 0; dx < cw; dx++) {
            int sx = (dx * W) / cw;
            l_pixel(c, dx, dy, fire_palette[fire[sy][sx]]);
        }
    }
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas canvas;
    if (l_canvas_open(&canvas, W, H, "Doom Fire") != 0) {
        puts("No display available\n");
        return 0;
    }

    init_palette();
    init_fire();

    while (l_canvas_alive(&canvas)) {
        int key = l_canvas_key(&canvas);
        if (key == 'q' || key == 'Q' || key == 27) break;
        if (key == 'r' || key == 'R') init_fire();

        spread_fire();
        draw(&canvas);
        int s = canvas.scale;
        l_draw_text_scaled(&canvas, 2 * s, 2 * s, "R:reset Q:quit",
                           L_RGB(40, 40, 40), s, s);
        l_canvas_flush(&canvas);
        l_sleep_ms(16);
    }

    l_canvas_close(&canvas);
    return 0;
}
