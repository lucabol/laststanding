#define L_MAINFILE
#include "l_gfx.h"
#include "l_img.h"

// img_view — Minimal freestanding image viewer.
// Usage: img_view <image_file>
// Loads PNG/JPEG/BMP/GIF/TGA and displays it in a window. Exit: Escape.

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    if (argc < 2) {
        l_puts("Usage: img_view <image_file>\n");
        l_puts("Displays a PNG, JPEG, BMP, GIF, or TGA image.\n");
        return 0;
    }

    // Read the entire file into memory
    L_FD fd = l_open(argv[1], 0 /*O_RDONLY*/, 0);
    if (fd < 0) { l_puts("Error: cannot open file\n"); return 1; }

    // Get file size via lseek
    long long fsize = l_lseek(fd, 0, 2 /*SEEK_END*/);
    l_lseek(fd, 0, 0 /*SEEK_SET*/);
    if (fsize <= 0 || fsize > 16 * 1024 * 1024) {
        l_puts("Error: file too large or empty (max 16MB)\n");
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

    // Decode image
    int w = 0, h = 0;
    uint32_t *pixels = l_img_load_mem(fdata, (int)fsize, &w, &h);
    l_munmap(fdata, (size_t)fsize);

    if (!pixels) {
        l_puts("Error: unsupported or corrupt image\n");
        return 1;
    }

    // Open canvas — scale image to fit 1024x768 if larger
    int maxw = 1024, maxh = 768;
    int cw = w, ch = h;
    if (cw > maxw || ch > maxh) {
        // Scale down preserving aspect ratio
        int sw = (w * maxh) / h;  // width if we fit height
        int sh = (h * maxw) / w;  // height if we fit width
        if (sw <= maxw) { cw = sw; ch = maxh; }
        else            { cw = maxw; ch = sh; }
    }

    L_Canvas c;
    if (l_canvas_open(&c, cw, ch, argv[1]) != 0) {
        l_puts("Error: cannot open display\n");
        l_img_free_pixels(pixels, w, h);
        return 1;
    }

    // Scale image to canvas using nearest-neighbor. Use the physical canvas
    // dimensions (c.width/c.height) so the image fills the whole window on
    // HiDPI displays, not just a logical-sized rectangle in the top-left.
    l_canvas_clear(&c, L_BLACK);
    {
        int stride_px = c.stride / 4;
        for (int dy = 0; dy < c.height; dy++) {
            int sy = (dy * h) / c.height;
            for (int dx = 0; dx < c.width; dx++) {
                int sx = (dx * w) / c.width;
                c.pixels[dy * stride_px + dx] = pixels[sy * w + sx];
            }
        }
    }

    // Overlay filename
    int s = c.scale;
    l_draw_text_scaled(&c, 4 * s, c.height - 12 * s, argv[1], L_WHITE, s, s);

    l_canvas_flush(&c);

    while (l_canvas_alive(&c) && l_canvas_key(&c) != 27)
        l_sleep_ms(16);

    l_canvas_close(&c);
    l_img_free_pixels(pixels, w, h);
    return 0;
}
