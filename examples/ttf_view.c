// ttf_view — Minimal freestanding TrueType font viewer.
//
// Loads a .ttf / .otf file, rasterizes a pangram at several pixel sizes, and
// displays the result via `l_tt_draw_text` (subpixel positioning + 2×
// supersampled rasterization + gamma-approximated compositing — see l_tt.h).
//
// Usage: ttf_view [font_path]
//   If font_path is omitted, tries a short list of common system fonts
//   (Segoe UI Semibold / Arial Bold on Windows, DejaVu Sans on Linux,
//   Arial on macOS).
//
// Exit: press Escape or close the window.

#define L_MAINFILE
#include "l_gfx.h"
#include "l_tt.h"

// Fallback font list. Bolder variants are preferred first because
// stb_truetype does not implement TrueType hinting, so heavier stems
// hold up better at small pixel sizes.
static const char *FALLBACK_FONTS[] = {
    "C:\\Windows\\Fonts\\seguisb.ttf",   // Segoe UI Semibold (Windows)
    "C:\\Windows\\Fonts\\arialbd.ttf",   // Arial Bold (Windows)
    "C:\\Windows\\Fonts\\segoeui.ttf",
    "C:\\Windows\\Fonts\\arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "/usr/share/fonts/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/Library/Fonts/Arial.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
    0
};

static unsigned char *load_file(const char *path, long long *out_size) {
    L_FD fd = l_open(path, 0, 0);
    if (fd < 0) return 0;
    long long sz = l_lseek(fd, 0, 2);
    l_lseek(fd, 0, 0);
    if (sz <= 0 || sz > 32 * 1024 * 1024) { l_close(fd); return 0; }
    unsigned char *data = (unsigned char *)l_mmap(0, (size_t)sz,
        L_PROT_READ, L_MAP_PRIVATE, fd, 0);
    l_close(fd);
    if (data == (unsigned char *)L_MAP_FAILED) return 0;
    *out_size = sz;
    return data;
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    const char *path = 0;
    long long sz = 0;
    unsigned char *font_data = 0;

    if (argc >= 2) {
        path = argv[1];
        font_data = load_file(path, &sz);
        if (!font_data) {
            l_puts("Error: cannot read font file: ");
            l_puts(argv[1]);
            l_puts("\n");
            return 1;
        }
    } else {
        for (int i = 0; FALLBACK_FONTS[i]; i++) {
            font_data = load_file(FALLBACK_FONTS[i], &sz);
            if (font_data) { path = FALLBACK_FONTS[i]; break; }
        }
        if (!font_data) {
            l_puts("Usage: ttf_view <font.ttf>\n");
            l_puts("No system font found. Pass an explicit path.\n");
            return 0;
        }
    }

    stbtt_fontinfo info;
    if (!l_tt_init_font(font_data, (int)sz, &info)) {
        l_puts("Error: not a valid TrueType font\n");
        return 1;
    }

    L_Canvas c;
    if (l_canvas_open(&c, 900, 520, "ttf_view — l_tt.h") != 0) return 1;

    const char *pangram =
        "The quick brown fox jumps over the lazy dog 0123456789";

    while (l_canvas_alive(&c)) {
        l_canvas_clear(&c, 0xFF101018);

        // Header: the font's path, rendered with the built-in bitmap font.
        l_draw_text(&c, 16, 12, "Font:", L_YELLOW);
        l_draw_text(&c, 64, 12, path, L_WHITE);
        l_line(&c, 0, 28, c.width, 28, 0xFF303030);

        // TrueType-rendered samples at a few pixel sizes.
        float y = 70.0f;
        float sizes[] = { 16.0f, 20.0f, 28.0f, 40.0f, 56.0f, 80.0f };
        uint32_t palette[] = {
            0xFFFFFFFF, 0xFFA0D8FF, 0xFFFFD880, 0xFFA0FFA0,
            0xFFFF80A0, 0xFFE0C0FF
        };
        for (int i = 0; i < (int)(sizeof(sizes) / sizeof(sizes[0])); i++) {
            l_tt_draw_text(c.pixels, c.width, c.height, c.stride,
                           &info, 16.0f, y, sizes[i], pangram, palette[i]);
            y += sizes[i] * 1.25f;
            if (y > (float)(c.height - 20)) break;
        }

        // Footer: hint.
        l_draw_text(&c, 16, c.height - 18,
                    "Esc to exit  -  pass any .ttf/.otf path on the command line",
                    0xFF606060);

        l_canvas_flush(&c);
        if (l_canvas_key(&c) == 27) break;
        l_sleep_ms(16);
    }

    l_canvas_close(&c);
    return 0;
}
