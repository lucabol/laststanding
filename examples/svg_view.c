#define L_MAINFILE
#include "l_gfx.h"
#include "l_svg.h"

// svg_view — Minimal freestanding SVG viewer.
// Usage: svg_view <svg_file> [width] [height]
// Rasterizes SVG and displays it in a window. Exit: Escape.
// NanoSVG subset: paths, shapes, gradients, transforms. No text, images, or clipping.

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    if (argc < 2) {
        l_puts("Usage: svg_view <svg_file> [width] [height]\n");
        l_puts("Displays an SVG (icons, diagrams, vector art).\n");
        l_puts("Subset supported: paths, shapes, gradients, transforms.\n");
        l_puts("Not supported: text, embedded images, clipping, masks.\n");
        return 0;
    }

    // Read the entire file into memory
    L_FD fd = l_open(argv[1], 0 /*O_RDONLY*/, 0);
    if (fd < 0) { l_puts("Error: cannot open file\n"); return 1; }

    // Get file size via lseek
    long long fsize = l_lseek(fd, 0, 2 /*SEEK_END*/);
    l_lseek(fd, 0, 0 /*SEEK_SET*/);
    if (fsize <= 0 || fsize > 8 * 1024 * 1024) {
        l_puts("Error: file too large or empty (max 8MB)\n");
        l_close(fd);
        return 1;
    }

    // Map the file into memory
    unsigned char *fdata = (unsigned char *)l_mmap(0, (size_t)fsize,
        L_PROT_READ, L_MAP_PRIVATE, fd, 0);
    l_close(fd);
    if (fdata == (unsigned char *)L_MAP_FAILED) {
        l_puts("Error: cannot read file\n");
        return 1;
    }

    // Parse command-line dimensions (or use defaults)
    int req_w = argc > 2 ? l_atoi(argv[2]) : 512;
    int req_h = argc > 3 ? l_atoi(argv[3]) : 512;
    if (req_w <= 0) req_w = 512;
    if (req_h <= 0) req_h = 512;

    // Rasterize SVG
    L_SvgOptions opt = { req_w, req_h, 96.0f };
    int w = 0, h = 0;
    uint32_t *pixels = l_svg_load_mem(fdata, (int)fsize, &opt, &w, &h);
    l_munmap(fdata, (size_t)fsize);

    if (!pixels) {
        l_puts("Error: unsupported or corrupt SVG\n");
        return 1;
    }

    // Open canvas
    L_Canvas c;
    if (l_canvas_open(&c, w, h, argv[1]) != 0) {
        l_puts("Error: cannot open display\n");
        l_svg_free_pixels(pixels, w, h);
        return 1;
    }

    // Display the rasterized SVG
    l_canvas_clear(&c, L_WHITE);
    l_blit_alpha(&c, 0, 0, w, h, pixels, w * 4);
    l_draw_text(&c, 4, h - 12, argv[1], L_BLACK);
    l_canvas_flush(&c);

    while (l_canvas_alive(&c) && l_canvas_key(&c) != 27)
        l_sleep_ms(16);

    l_canvas_close(&c);
    l_svg_free_pixels(pixels, w, h);
    return 0;
}
