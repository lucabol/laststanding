#define L_MAINFILE
#include "l_gfx.h"

// life.c — Conway's Game of Life
// 320x240 canvas, 80x60 grid (4x4 pixel cells)
// Space=pause, R=randomize, C=clear, Q/ESC=quit

#define W 320
#define H 240
#define GW 80
#define GH 60
#define CELL 4

static uint8_t grid[GH][GW];
static uint8_t next[GH][GW];

static void randomize(void) {
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            grid[y][x] = (l_rand() & 3) == 0 ? 1 : 0;
}

static void clear_grid(void) {
    l_memset(grid, 0, sizeof(grid));
}

static int count_neighbors(int cx, int cy) {
    int n = 0;
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = (cx + dx + GW) % GW;
            int ny = (cy + dy + GH) % GH;
            n += grid[ny][nx];
        }
    return n;
}

static void step(void) {
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++) {
            int n = count_neighbors(x, y);
            if (grid[y][x])
                next[y][x] = (n == 2 || n == 3) ? 1 : 0;
            else
                next[y][x] = (n == 3) ? 1 : 0;
        }
    l_memcpy(grid, next, sizeof(grid));
}

static void draw(L_Canvas *c) {
    l_canvas_clear(c, L_BLACK);
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            if (grid[y][x])
                l_fill_rect(c, x * CELL, y * CELL, CELL, CELL, L_GREEN);

    l_draw_text(c, 2, H - 10, "SPC:pause R:rand C:clear Q:quit", L_RGB(128, 128, 128));
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas canvas;
    if (l_canvas_open(&canvas, W, H, "Game of Life") != 0) {
        puts("No display available\n");
        return 0;
    }

    randomize();
    int paused = 0;

    while (l_canvas_alive(&canvas)) {
        int key = l_canvas_key(&canvas);
        if (key == 'q' || key == 'Q' || key == 27) break;
        if (key == ' ') paused = !paused;
        if (key == 'r' || key == 'R') { l_srand(l_rand_state + 7777); randomize(); }
        if (key == 'c' || key == 'C') clear_grid();

        if (!paused) step();

        draw(&canvas);
        l_canvas_flush(&canvas);
        l_sleep_ms(16);
    }

    l_canvas_close(&canvas);
    return 0;
}
