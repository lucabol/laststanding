// l_gfx.h — Freestanding pixel graphics for laststanding
//
// Usage: #define L_MAINFILE
//        #include "l_gfx.h"   // pulls in l_os.h automatically
//
// Linux:   renders via X11 wire protocol if $DISPLAY is set,
//          falls back to /dev/fb0 (framebuffer console) otherwise.
//          No X11 library linking — direct socket protocol.
// Windows: opens a native GDI window (user32 + gdi32, no external deps)

// X11 backend needs sockets (Unix domain socket to X server)
#if !defined(_WIN32) && !defined(__wasi__)
#ifndef L_WITHSOCKETS
#define L_WITHSOCKETS
#endif
#endif

#include "l_os.h"

// Suppress warnings for GNU statement expressions used in syscall macros
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wgnu-statement-expression-from-macro-expansion"
#endif

#ifdef _WIN32
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#endif

// ── Color helpers ────────────────────────────────────────────────────────────

/// Composes a 32-bit ARGB color from red, green, blue (0-255).
#define L_RGB(r, g, b)       ((uint32_t)0xFF000000u | ((uint32_t)(r)<<16) | ((uint32_t)(g)<<8) | (uint32_t)(b))
/// Composes a 32-bit ARGB color from red, green, blue, alpha (0-255).
#define L_RGBA(r, g, b, a)   (((uint32_t)(a)<<24) | ((uint32_t)(r)<<16) | ((uint32_t)(g)<<8) | (uint32_t)(b))
#define L_BLACK   ((uint32_t)0xFF000000u)
#define L_WHITE   ((uint32_t)0xFFFFFFFFu)
#define L_RED     ((uint32_t)0xFFFF0000u)
#define L_GREEN   ((uint32_t)0xFF00FF00u)
#define L_BLUE    ((uint32_t)0xFF0000FFu)
#define L_YELLOW  ((uint32_t)0xFFFFFF00u)
#define L_CYAN    ((uint32_t)0xFF00FFFFu)
#define L_MAGENTA ((uint32_t)0xFFFF00FFu)

// ── Canvas type ──────────────────────────────────────────────────────────────
typedef struct {
    int       width;
    int       height;
    int       stride;       // bytes per row in pixel buffer
    uint32_t *pixels;       // user-accessible ARGB pixel buffer
    int       mouse_x;     // current mouse x position
    int       mouse_y;     // current mouse y position
    int       mouse_btn;   // button bitmask: 1=left, 2=right, 4=middle
    int       resized;     // set to 1 when window was resized; cleared by l_canvas_resized()

#ifdef _WIN32
    // Windows GDI internals
    void     *hwnd;         // HWND
    void     *hdc_mem;      // memory DC for the DIB
    void     *hbmp_old;     // previous bitmap in hdc_mem
    int       closed;       // set when WM_CLOSE received
    int       keys[16];     // small ring buffer for key events
    int       key_head;
    int       key_tail;
#else
    // Linux internals — X11 or framebuffer
    int       backend;      // 0=framebuffer, 1=X11
    // Framebuffer fields
    int       fb_fd;        // fd to /dev/fb0
    uint8_t  *fb_mem;       // mmap'd framebuffer
    int       fb_size;      // total mmap size
    int       fb_stride;    // framebuffer bytes per row (may differ from canvas stride)
    int       fb_bpp;       // framebuffer bits per pixel
    int       fb_xoff;      // x offset in virtual framebuffer
    int       fb_yoff;      // y offset in virtual framebuffer
    unsigned long saved_tty; // saved terminal mode for restore
    int       mouse_fd;    // fd to /dev/input/mice (-1 if unavailable)
    // X11 fields
    int       x11_fd;       // socket to X server
    uint32_t  x11_wid;      // window resource ID
    uint32_t  x11_gc;       // graphics context ID
    uint32_t  x11_root;     // root window ID
    int       x11_depth;    // screen depth
    int       closed;       // set when window is destroyed
    int       keys[16];     // small ring buffer for key events
    int       key_head;
    int       key_tail;
#endif
} L_Canvas;

// ── Embedded 8×8 bitmap font (ASCII 32–126) ─────────────────────────────────
// Each character is 8 bytes, one byte per row, MSB = leftmost pixel.
// 95 printable characters × 8 bytes = 760 bytes.
static const uint8_t l_font8x8[95][8] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // 32 ' '
    {0x18,0x3C,0x3C,0x18,0x18,0x00,0x18,0x00}, // 33 '!'
    {0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00}, // 34 '"'
    {0x36,0x36,0x7F,0x36,0x7F,0x36,0x36,0x00}, // 35 '#'
    {0x0C,0x3E,0x03,0x1E,0x30,0x1F,0x0C,0x00}, // 36 '$'
    {0x00,0x63,0x33,0x18,0x0C,0x66,0x63,0x00}, // 37 '%'
    {0x1C,0x36,0x1C,0x6E,0x3B,0x33,0x6E,0x00}, // 38 '&'
    {0x06,0x06,0x03,0x00,0x00,0x00,0x00,0x00}, // 39 '''
    {0x18,0x0C,0x06,0x06,0x06,0x0C,0x18,0x00}, // 40 '('
    {0x06,0x0C,0x18,0x18,0x18,0x0C,0x06,0x00}, // 41 ')'
    {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, // 42 '*'
    {0x00,0x0C,0x0C,0x3F,0x0C,0x0C,0x00,0x00}, // 43 '+'
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x06}, // 44 ','
    {0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00}, // 45 '-'
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00}, // 46 '.'
    {0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00}, // 47 '/'
    {0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00}, // 48 '0'
    {0x0C,0x0E,0x0C,0x0C,0x0C,0x0C,0x3F,0x00}, // 49 '1'
    {0x1E,0x33,0x30,0x1C,0x06,0x33,0x3F,0x00}, // 50 '2'
    {0x1E,0x33,0x30,0x1C,0x30,0x33,0x1E,0x00}, // 51 '3'
    {0x38,0x3C,0x36,0x33,0x7F,0x30,0x78,0x00}, // 52 '4'
    {0x3F,0x03,0x1F,0x30,0x30,0x33,0x1E,0x00}, // 53 '5'
    {0x1C,0x06,0x03,0x1F,0x33,0x33,0x1E,0x00}, // 54 '6'
    {0x3F,0x33,0x30,0x18,0x0C,0x0C,0x0C,0x00}, // 55 '7'
    {0x1E,0x33,0x33,0x1E,0x33,0x33,0x1E,0x00}, // 56 '8'
    {0x1E,0x33,0x33,0x3E,0x30,0x18,0x0E,0x00}, // 57 '9'
    {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x00}, // 58 ':'
    {0x00,0x0C,0x0C,0x00,0x00,0x0C,0x0C,0x06}, // 59 ';'
    {0x18,0x0C,0x06,0x03,0x06,0x0C,0x18,0x00}, // 60 '<'
    {0x00,0x00,0x3F,0x00,0x00,0x3F,0x00,0x00}, // 61 '='
    {0x06,0x0C,0x18,0x30,0x18,0x0C,0x06,0x00}, // 62 '>'
    {0x1E,0x33,0x30,0x18,0x0C,0x00,0x0C,0x00}, // 63 '?'
    {0x3E,0x63,0x7B,0x7B,0x7B,0x03,0x1E,0x00}, // 64 '@'
    {0x0C,0x1E,0x33,0x33,0x3F,0x33,0x33,0x00}, // 65 'A'
    {0x3F,0x66,0x66,0x3E,0x66,0x66,0x3F,0x00}, // 66 'B'
    {0x3C,0x66,0x03,0x03,0x03,0x66,0x3C,0x00}, // 67 'C'
    {0x1F,0x36,0x66,0x66,0x66,0x36,0x1F,0x00}, // 68 'D'
    {0x7F,0x46,0x16,0x1E,0x16,0x46,0x7F,0x00}, // 69 'E'
    {0x7F,0x46,0x16,0x1E,0x16,0x06,0x0F,0x00}, // 70 'F'
    {0x3C,0x66,0x03,0x03,0x73,0x66,0x7C,0x00}, // 71 'G'
    {0x33,0x33,0x33,0x3F,0x33,0x33,0x33,0x00}, // 72 'H'
    {0x1E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, // 73 'I'
    {0x78,0x30,0x30,0x30,0x33,0x33,0x1E,0x00}, // 74 'J'
    {0x67,0x66,0x36,0x1E,0x36,0x66,0x67,0x00}, // 75 'K'
    {0x0F,0x06,0x06,0x06,0x46,0x66,0x7F,0x00}, // 76 'L'
    {0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00}, // 77 'M'
    {0x63,0x67,0x6F,0x7B,0x73,0x63,0x63,0x00}, // 78 'N'
    {0x1C,0x36,0x63,0x63,0x63,0x36,0x1C,0x00}, // 79 'O'
    {0x3F,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00}, // 80 'P'
    {0x1E,0x33,0x33,0x33,0x3B,0x1E,0x38,0x00}, // 81 'Q'
    {0x3F,0x66,0x66,0x3E,0x36,0x66,0x67,0x00}, // 82 'R'
    {0x1E,0x33,0x07,0x0E,0x38,0x33,0x1E,0x00}, // 83 'S'
    {0x3F,0x2D,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, // 84 'T'
    {0x33,0x33,0x33,0x33,0x33,0x33,0x3F,0x00}, // 85 'U'
    {0x33,0x33,0x33,0x33,0x33,0x1E,0x0C,0x00}, // 86 'V'
    {0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, // 87 'W'
    {0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00}, // 88 'X'
    {0x33,0x33,0x33,0x1E,0x0C,0x0C,0x1E,0x00}, // 89 'Y'
    {0x7F,0x63,0x31,0x18,0x4C,0x66,0x7F,0x00}, // 90 'Z'
    {0x1E,0x06,0x06,0x06,0x06,0x06,0x1E,0x00}, // 91 '['
    {0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00}, // 92 '\'
    {0x1E,0x18,0x18,0x18,0x18,0x18,0x1E,0x00}, // 93 ']'
    {0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00}, // 94 '^'
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}, // 95 '_'
    {0x0C,0x0C,0x18,0x00,0x00,0x00,0x00,0x00}, // 96 '`'
    {0x00,0x00,0x1E,0x30,0x3E,0x33,0x6E,0x00}, // 97 'a'
    {0x07,0x06,0x06,0x3E,0x66,0x66,0x3B,0x00}, // 98 'b'
    {0x00,0x00,0x1E,0x33,0x03,0x33,0x1E,0x00}, // 99 'c'
    {0x38,0x30,0x30,0x3E,0x33,0x33,0x6E,0x00}, //100 'd'
    {0x00,0x00,0x1E,0x33,0x3F,0x03,0x1E,0x00}, //101 'e'
    {0x1C,0x36,0x06,0x0F,0x06,0x06,0x0F,0x00}, //102 'f'
    {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x1F}, //103 'g'
    {0x07,0x06,0x36,0x6E,0x66,0x66,0x67,0x00}, //104 'h'
    {0x0C,0x00,0x0E,0x0C,0x0C,0x0C,0x1E,0x00}, //105 'i'
    {0x30,0x00,0x30,0x30,0x30,0x33,0x33,0x1E}, //106 'j'
    {0x07,0x06,0x66,0x36,0x1E,0x36,0x67,0x00}, //107 'k'
    {0x0E,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00}, //108 'l'
    {0x00,0x00,0x33,0x7F,0x7F,0x6B,0x63,0x00}, //109 'm'
    {0x00,0x00,0x1F,0x33,0x33,0x33,0x33,0x00}, //110 'n'
    {0x00,0x00,0x1E,0x33,0x33,0x33,0x1E,0x00}, //111 'o'
    {0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F}, //112 'p'
    {0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78}, //113 'q'
    {0x00,0x00,0x3B,0x6E,0x66,0x06,0x0F,0x00}, //114 'r'
    {0x00,0x00,0x3E,0x03,0x1E,0x30,0x1F,0x00}, //115 's'
    {0x08,0x0C,0x3E,0x0C,0x0C,0x2C,0x18,0x00}, //116 't'
    {0x00,0x00,0x33,0x33,0x33,0x33,0x6E,0x00}, //117 'u'
    {0x00,0x00,0x33,0x33,0x33,0x1E,0x0C,0x00}, //118 'v'
    {0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00}, //119 'w'
    {0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00}, //120 'x'
    {0x00,0x00,0x33,0x33,0x33,0x3E,0x30,0x1F}, //121 'y'
    {0x00,0x00,0x3F,0x19,0x0C,0x26,0x3F,0x00}, //122 'z'
    {0x38,0x0C,0x0C,0x07,0x0C,0x0C,0x38,0x00}, //123 '{'
    {0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00}, //124 '|'
    {0x07,0x0C,0x0C,0x38,0x0C,0x0C,0x07,0x00}, //125 '}'
    {0x6E,0x3B,0x00,0x00,0x00,0x00,0x00,0x00}, //126 '~'
};

// ── API declarations ─────────────────────────────────────────────────────────

/// Opens a canvas. Returns 0 on success, -1 on error (e.g. no display).
static inline int  l_canvas_open(L_Canvas *c, int width, int height, const char *title);
/// Closes the canvas and frees resources.
static inline void l_canvas_close(L_Canvas *c);
/// Returns non-zero if the canvas is still alive (window not closed).
static inline int  l_canvas_alive(L_Canvas *c);
/// Copies the pixel buffer to the screen.
static inline void l_canvas_flush(L_Canvas *c);
/// Fills the entire pixel buffer with a single color.
static inline void l_canvas_clear(L_Canvas *c, uint32_t color);
/// Returns the next key press (ASCII or arrow codes), or 0 if none. Non-blocking.
static inline int  l_canvas_key(L_Canvas *c);
/// Returns mouse button bitmask (1=left, 2=right, 4=middle) and writes position to *x, *y.
static inline int  l_canvas_mouse(L_Canvas *c, int *x, int *y);
/// Returns 1 if the window was resized since the last call, 0 otherwise. Clears the flag.
/// When resized, c->width, c->height, c->stride, and c->pixels are already updated.
/// Not supported on Linux framebuffer backend (always returns 0).
static inline int  l_canvas_resized(L_Canvas *c);

// ── Drawing primitives (platform-independent, operate on pixels[]) ───────────

/// Sets a single pixel at (x, y) to the given color. No-op if out of bounds.
static inline void l_pixel(L_Canvas *c, int x, int y, uint32_t color) {
    if (x >= 0 && x < c->width && y >= 0 && y < c->height)
        c->pixels[y * (c->stride / 4) + x] = color;
}

/// Returns the color of the pixel at (x, y), or 0 if out of bounds.
static inline uint32_t l_get_pixel(L_Canvas *c, int x, int y) {
    if (x >= 0 && x < c->width && y >= 0 && y < c->height)
        return c->pixels[y * (c->stride / 4) + x];
    return 0;
}

/// Draws a line from (x0,y0) to (x1,y1) using Bresenham's algorithm.
static inline void l_line(L_Canvas *c, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 - x0, dy = y1 - y0;
    int sx = dx > 0 ? 1 : -1, sy = dy > 0 ? 1 : -1;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    int err = dx - dy;
    for (;;) {
        l_pixel(c, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

/// Draws an outline rectangle at (x,y) with width w and height h.
static inline void l_rect(L_Canvas *c, int x, int y, int w, int h, uint32_t color) {
    l_line(c, x, y, x + w - 1, y, color);
    l_line(c, x, y + h - 1, x + w - 1, y + h - 1, color);
    l_line(c, x, y, x, y + h - 1, color);
    l_line(c, x + w - 1, y, x + w - 1, y + h - 1, color);
}

/// Draws a filled rectangle at (x,y) with width w and height h.
static inline void l_fill_rect(L_Canvas *c, int x, int y, int w, int h, uint32_t color) {
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w > c->width  ? c->width  : x + w;
    int y1 = y + h > c->height ? c->height : y + h;
    int s = c->stride / 4;
    for (int py = y0; py < y1; py++)
        for (int px = x0; px < x1; px++)
            c->pixels[py * s + px] = color;
}

/// Draws an outline circle centered at (cx,cy) with radius r (midpoint algorithm).
static inline void l_circle(L_Canvas *c, int cx, int cy, int r, uint32_t color) {
    int x = r, y = 0, d = 1 - r;
    while (x >= y) {
        l_pixel(c, cx+x, cy+y, color); l_pixel(c, cx-x, cy+y, color);
        l_pixel(c, cx+x, cy-y, color); l_pixel(c, cx-x, cy-y, color);
        l_pixel(c, cx+y, cy+x, color); l_pixel(c, cx-y, cy+x, color);
        l_pixel(c, cx+y, cy-x, color); l_pixel(c, cx-y, cy-x, color);
        y++;
        if (d <= 0) { d += 2*y + 1; }
        else        { x--; d += 2*(y - x) + 1; }
    }
}

/// Draws a filled circle centered at (cx,cy) with radius r.
static inline void l_fill_circle(L_Canvas *c, int cx, int cy, int r, uint32_t color) {
    int x = r, y = 0, d = 1 - r;
    while (x >= y) {
        l_line(c, cx-x, cy+y, cx+x, cy+y, color);
        l_line(c, cx-x, cy-y, cx+x, cy-y, color);
        l_line(c, cx-y, cy+x, cx+y, cy+x, color);
        l_line(c, cx-y, cy-x, cx+y, cy-x, color);
        y++;
        if (d <= 0) { d += 2*y + 1; }
        else        { x--; d += 2*(y - x) + 1; }
    }
}

/// Draws a horizontal line from x0 to x1 at row y (used internally by fill_circle).
static inline void l_hline(L_Canvas *c, int x0, int x1, int y, uint32_t color) {
    if (y < 0 || y >= c->height) return;
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (x0 < 0) x0 = 0;
    if (x1 >= c->width) x1 = c->width - 1;
    int s = c->stride / 4;
    for (int x = x0; x <= x1; x++)
        c->pixels[y * s + x] = color;
}

// ── Text rendering ───────────────────────────────────────────────────────────

/// Draws a single character at (x,y) using the embedded 8x8 bitmap font.
static inline void l_draw_char(L_Canvas *c, int x, int y, char ch, uint32_t color) {
    if (ch < 32 || ch > 126) return;
    const uint8_t *glyph = l_font8x8[ch - 32];
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
            if (glyph[row] & (1 << col))
                l_pixel(c, x + col, y + row, color);
}

/// Draws a null-terminated string at (x,y), advancing 8 pixels per character.
static inline void l_draw_text(L_Canvas *c, int x, int y, const char *text, uint32_t color) {
    while (*text) {
        l_draw_char(c, x, y, *text, color);
        x += 8;
        text++;
    }
}

/// Draws a single character at (x,y) scaled by (sx,sy) using nearest-neighbor.
static inline void l_draw_char_scaled(L_Canvas *c, int x, int y, char ch,
                                       uint32_t color, int sx, int sy) {
    if (ch < 32 || ch > 126 || sx < 1 || sy < 1) return;
    const uint8_t *glyph = l_font8x8[ch - 32];
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
            if (glyph[row] & (1 << col))
                l_fill_rect(c, x + col * sx, y + row * sy, sx, sy, color);
}

/// Draws a string at (x,y) with each glyph scaled by (sx,sy).
static inline void l_draw_text_scaled(L_Canvas *c, int x, int y, const char *text,
                                       uint32_t color, int sx, int sy) {
    while (*text) {
        l_draw_char_scaled(c, x, y, *text, color, sx, sy);
        x += 8 * sx;
        text++;
    }
}

// ── Pixel blitting ───────────────────────────────────────────────────────────

/// Blit a rectangle of ARGB pixels onto the canvas at (dx, dy).
/// src points to width*height pixels in row-major ARGB format; src_stride is bytes per source row.
static inline void l_blit(L_Canvas *c, int dx, int dy, int w, int h,
                           const uint32_t *src, int src_stride) {
    int s = c->stride / 4;
    for (int y = 0; y < h; y++) {
        int cy = dy + y;
        if (cy < 0 || cy >= c->height) continue;
        const uint32_t *row = (const uint32_t *)((const uint8_t *)src + y * src_stride);
        for (int x = 0; x < w; x++) {
            int cx = dx + x;
            if (cx >= 0 && cx < c->width)
                c->pixels[cy * s + cx] = row[x];
        }
    }
}

/// Blit with alpha blending (source-over). Assumes pre-multiplied alpha in the A channel.
static inline void l_blit_alpha(L_Canvas *c, int dx, int dy, int w, int h,
                                 const uint32_t *src, int src_stride) {
    int s = c->stride / 4;
    for (int y = 0; y < h; y++) {
        int cy = dy + y;
        if (cy < 0 || cy >= c->height) continue;
        const uint32_t *row = (const uint32_t *)((const uint8_t *)src + y * src_stride);
        for (int x = 0; x < w; x++) {
            int cx = dx + x;
            if (cx < 0 || cx >= c->width) continue;
            uint32_t sp = row[x];
            uint32_t sa = (sp >> 24) & 0xFF;
            if (sa == 0xFF) { c->pixels[cy * s + cx] = sp; continue; }
            if (sa == 0) continue;
            uint32_t dp = c->pixels[cy * s + cx];
            uint32_t inv = 255 - sa;
            uint32_t r = ((sp >> 16) & 0xFF) + (((dp >> 16) & 0xFF) * inv / 255);
            uint32_t g = ((sp >> 8) & 0xFF)  + (((dp >> 8) & 0xFF) * inv / 255);
            uint32_t b = (sp & 0xFF)         + ((dp & 0xFF) * inv / 255);
            c->pixels[cy * s + cx] = 0xFF000000u | (r << 16) | (g << 8) | b;
        }
    }
}

// ── Platform implementations ─────────────────────────────────────────────────

#ifdef L_WITHDEFS

#ifdef _WIN32
// ═══════════════════════════════════════════════════════════════════════════════
// Windows GDI backend
// ═══════════════════════════════════════════════════════════════════════════════

// Window class name
static const wchar_t l_gfx_classname[] = L"LGfxWindow";
static L_Canvas *l_gfx_active_canvas = 0;

static LRESULT CALLBACK l_gfx_wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    L_Canvas *c = l_gfx_active_canvas;
    switch (msg) {
    case WM_CLOSE:
        if (c) c->closed = 1;
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (c) {
            int key = 0;
            switch (wp) {
            case VK_LEFT:   key = 1001; break;
            case VK_RIGHT:  key = 1002; break;
            case VK_UP:     key = 1003; break;
            case VK_DOWN:   key = 1004; break;
            case VK_ESCAPE: key = 27;   break;
            case VK_BACK:   key = 8;    break;
            case VK_TAB:    key = 9;    break;
            default: break;  // printable chars handled by WM_CHAR
            }
            if (key) {
                int next = (c->key_head + 1) % 16;
                if (next != c->key_tail) {
                    c->keys[c->key_head] = key;
                    c->key_head = next;
                }
            }
        }
        return 0;
    case WM_CHAR:
        if (c) {
            int key = (int)wp;
            // Accept printable ASCII and Enter (translated from VK_RETURN)
            if ((key >= 32 && key <= 126) || key == 13) {
                int next = (c->key_head + 1) % 16;
                if (next != c->key_tail) {
                    c->keys[c->key_head] = key;
                    c->key_head = next;
                }
            }
        }
        return 0;
    case WM_MOUSEMOVE:
        if (c) { c->mouse_x = (short)LOWORD(lp); c->mouse_y = (short)HIWORD(lp); }
        return 0;
    case WM_LBUTTONDOWN: if (c) c->mouse_btn |=  1; return 0;
    case WM_LBUTTONUP:   if (c) c->mouse_btn &= ~1; return 0;
    case WM_RBUTTONDOWN: if (c) c->mouse_btn |=  2; return 0;
    case WM_RBUTTONUP:   if (c) c->mouse_btn &= ~2; return 0;
    case WM_MBUTTONDOWN: if (c) c->mouse_btn |=  4; return 0;
    case WM_MBUTTONUP:   if (c) c->mouse_btn &= ~4; return 0;
    case WM_SIZE:
        if (c && wp != 1 /* SIZE_MINIMIZED */) {
            int nw = (int)LOWORD(lp);
            int nh = (int)HIWORD(lp);
            if (nw > 0 && nh > 0 && (nw != c->width || nh != c->height)) {
                /* Save old dimensions and pixels for scaling */
                int ow = c->width, oh = c->height;
                uint32_t *old_px = c->pixels;

                /* Reallocate the DIB at the new size */
                HDC hdc_win = GetDC(hwnd);
                if (c->hdc_mem) {
                    SelectObject((HDC)c->hdc_mem, (HGDIOBJ)c->hbmp_old);
                    DeleteDC((HDC)c->hdc_mem);
                }
                BITMAPINFO bmi;
                l_memset(&bmi, 0, sizeof(bmi));
                bmi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth    = nw;
                bmi.bmiHeader.biHeight   = -nh;
                bmi.bmiHeader.biPlanes   = 1;
                bmi.bmiHeader.biBitCount = 32;
                void *bits = 0;
                HBITMAP hbmp = CreateDIBSection(hdc_win, &bmi, DIB_RGB_COLORS, &bits, 0, 0);
                HDC hdc_mem = CreateCompatibleDC(hdc_win);
                HGDIOBJ old = SelectObject(hdc_mem, hbmp);
                ReleaseDC(hwnd, hdc_win);

                /* Scale old content into new buffer (nearest-neighbor) */
                if (old_px && bits && ow > 0 && oh > 0) {
                    uint32_t *dst = (uint32_t *)bits;
                    for (int y = 0; y < nh; y++) {
                        int sy = y * oh / nh;
                        for (int x = 0; x < nw; x++) {
                            int sx = x * ow / nw;
                            dst[y * nw + x] = old_px[sy * ow + sx];
                        }
                    }
                }

                c->hdc_mem  = hdc_mem;
                c->hbmp_old = old;
                c->pixels   = (uint32_t *)bits;
                c->width    = nw;
                c->height   = nh;
                c->stride   = nw * 4;
                c->resized  = 1;
                InvalidateRect(hwnd, NULL, FALSE); /* repaint with scaled content */
            }
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (c && c->hdc_mem)
            BitBlt(hdc, 0, 0, c->width, c->height, (HDC)c->hdc_mem, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        if (c) c->closed = 1;
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static inline int l_canvas_open(L_Canvas *c, int width, int height, const char *title) {
    l_memset(c, 0, sizeof(*c));
    l_gfx_active_canvas = c;

    int fullscreen = (width <= 0 && height <= 0);
    if (fullscreen) {
        width  = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    }

    c->width  = width;
    c->height = height;
    c->stride = width * 4;

    WNDCLASSW wc;
    l_memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc   = l_gfx_wndproc;
    wc.hInstance      = GetModuleHandleW(0);
    wc.lpszClassName  = l_gfx_classname;
    wc.hCursor        = LoadCursorW(0, (LPCWSTR)IDC_ARROW);
    RegisterClassW(&wc);

    DWORD style = fullscreen
        ? (DWORD)WS_POPUP
        : (DWORD)WS_OVERLAPPEDWINDOW;

    // Calculate window size to fit client area exactly
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, style, FALSE);

    int wx = fullscreen ? 0 : CW_USEDEFAULT;
    int wy = fullscreen ? 0 : CW_USEDEFAULT;

    // Convert title to wide
    wchar_t wtitle[256];
    int i = 0;
    if (title) {
        while (title[i] && i < 255) { wtitle[i] = (wchar_t)title[i]; i++; }
    }
    wtitle[i] = 0;

    c->hwnd = CreateWindowExW(0, l_gfx_classname, wtitle, style,
        wx, wy,
        rect.right - rect.left, rect.bottom - rect.top,
        0, 0, GetModuleHandleW(0), 0);
    if (!c->hwnd) return -1;

    HDC hdc_win = GetDC((HWND)c->hwnd);
    c->hdc_mem = CreateCompatibleDC(hdc_win);

    BITMAPINFO bmi;
    l_memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = width;
    bmi.bmiHeader.biHeight      = -height;  // top-down
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *bits = 0;
    HBITMAP hbmp = CreateDIBSection(hdc_win, &bmi, DIB_RGB_COLORS, &bits, 0, 0);
    ReleaseDC((HWND)c->hwnd, hdc_win);
    if (!hbmp || !bits) {
        DestroyWindow((HWND)c->hwnd);
        return -1;
    }

    c->pixels   = (uint32_t *)bits;
    c->hbmp_old = SelectObject((HDC)c->hdc_mem, hbmp);

    ShowWindow((HWND)c->hwnd, SW_SHOW);
    UpdateWindow((HWND)c->hwnd);
    return 0;
}

static inline void l_canvas_close(L_Canvas *c) {
    if (c->hdc_mem) {
        HBITMAP hbmp = (HBITMAP)SelectObject((HDC)c->hdc_mem, (HGDIOBJ)c->hbmp_old);
        DeleteObject(hbmp);
        DeleteDC((HDC)c->hdc_mem);
    }
    if (c->hwnd) DestroyWindow((HWND)c->hwnd);
    if (l_gfx_active_canvas == c) l_gfx_active_canvas = 0;
    l_memset(c, 0, sizeof(*c));
}

static inline int l_canvas_alive(L_Canvas *c) {
    return !c->closed;
}

static inline void l_canvas_flush(L_Canvas *c) {
    // Pump messages
    MSG msg;
    while (PeekMessageW(&msg, (HWND)c->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    // Blit
    HDC hdc = GetDC((HWND)c->hwnd);
    BitBlt(hdc, 0, 0, c->width, c->height, (HDC)c->hdc_mem, 0, 0, SRCCOPY);
    ReleaseDC((HWND)c->hwnd, hdc);
}

static inline void l_canvas_clear(L_Canvas *c, uint32_t color) {
    int n = c->width * c->height;
    for (int i = 0; i < n; i++)
        c->pixels[i] = color;
}

static inline int l_canvas_key(L_Canvas *c) {
    // Pump messages first
    MSG msg;
    while (PeekMessageW(&msg, (HWND)c->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (c->key_head == c->key_tail) return 0;
    int key = c->keys[c->key_tail];
    c->key_tail = (c->key_tail + 1) % 16;
    return key;
}

static inline int l_canvas_mouse(L_Canvas *c, int *x, int *y) {
    MSG msg;
    while (PeekMessageW(&msg, (HWND)c->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (x) *x = c->mouse_x;
    if (y) *y = c->mouse_y;
    return c->mouse_btn;
}

static inline int l_canvas_resized(L_Canvas *c) {
    int r = c->resized;
    c->resized = 0;
    return r;
}

#else
// ═══════════════════════════════════════════════════════════════════════════════
// Linux backend — X11 wire protocol or framebuffer
// ═══════════════════════════════════════════════════════════════════════════════

// Framebuffer ioctl numbers
#define L_FBIOGET_VSCREENINFO 0x4600
#define L_FBIOGET_FSCREENINFO 0x4602

struct l_fb_var_screeninfo {
    uint32_t xres;
    uint32_t yres;
    uint32_t xres_virtual;
    uint32_t yres_virtual;
    uint32_t xoffset;
    uint32_t yoffset;
    uint32_t bits_per_pixel;
    uint32_t grayscale;
    uint32_t pad[56];
};

struct l_fb_fix_screeninfo {
    char     id[16];
    uint64_t smem_start;
    uint32_t smem_len;
    uint32_t type;
    uint32_t type_aux;
    uint32_t visual;
    uint16_t xpanstep;
    uint16_t ypanstep;
    uint16_t ywrapstep;
    uint32_t line_length;
    uint32_t pad[16];
};

// ── X11 wire protocol helpers ────────────────────────────────────────────────

/* Read exactly n bytes from fd, retrying on short reads */
static inline int l_x11_read_full(int fd, void *buf, int n) {
    int total = 0;
    while (total < n) {
        ssize_t r = l_read(fd, (uint8_t *)buf + total, (size_t)(n - total));
        if (r <= 0) return -1;
        total += (int)r;
    }
    return 0;
}

/* Write exactly n bytes to fd */
static inline int l_x11_write_full(int fd, const void *buf, int n) {
    int total = 0;
    while (total < n) {
        ssize_t r = l_write(fd, (const uint8_t *)buf + total, (size_t)(n - total));
        if (r <= 0) return -1;
        total += (int)r;
    }
    return 0;
}

/* Parse $DISPLAY to extract display number; returns -1 if not local Unix */
static inline int l_x11_parse_display(const char *disp, char *sockpath, int pathsz) {
    if (!disp || !disp[0]) return -1;
    if (disp[0] != ':') return -1;
    int dnum = l_atoi(disp + 1);
    if (dnum < 0) dnum = 0;
    /* Build "/tmp/.X11-unix/X{dnum}" manually */
    const char *prefix = "/tmp/.X11-unix/X";
    int i = 0;
    while (prefix[i] && i < pathsz - 1) { sockpath[i] = prefix[i]; i++; }
    /* Append display number as decimal digits */
    char numbuf[12];
    int nlen = 0;
    int d = dnum;
    if (d == 0) { numbuf[nlen++] = '0'; }
    else { while (d > 0 && nlen < 10) { numbuf[nlen++] = (char)('0' + d % 10); d /= 10; } }
    for (int j = nlen - 1; j >= 0 && i < pathsz - 1; j--) sockpath[i++] = numbuf[j];
    sockpath[i] = '\0';
    return dnum;
}

/* Read ~/.Xauthority and find MIT-MAGIC-COOKIE-1 for the given display.
   Returns cookie length (should be 16) or 0 if not found. */
static inline int l_x11_read_xauth(int display_num, uint8_t *cookie, int cookie_sz) {
    char path[256];
    const char *home = l_getenv("HOME");
    if (!home) return 0;
    /* Build "$HOME/.Xauthority" manually */
    int i = 0;
    while (home[i] && i < 240) { path[i] = home[i]; i++; }
    const char *suffix = "/.Xauthority";
    int j = 0;
    while (suffix[j] && i < 255) { path[i++] = suffix[j++]; }
    path[i] = '\0';
    L_FD fd = l_open(path, 0 /*O_RDONLY*/, 0);
    if (fd < 0) return 0;

    /* Xauthority format: repeated entries of:
       uint16_t family (big-endian)
       uint16_t addr_len, addr_len bytes of address
       uint16_t number_len, number_len bytes of display number string
       uint16_t name_len, name_len bytes of protocol name
       uint16_t data_len, data_len bytes of auth data */
    char dnum_str[16];
    /* Convert display_num to string manually */
    {
        int d = display_num, pos = 0;
        char tmp[16];
        if (d == 0) { tmp[pos++] = '0'; }
        else { while (d > 0 && pos < 14) { tmp[pos++] = (char)('0' + d % 10); d /= 10; } }
        for (int k = 0; k < pos; k++) dnum_str[k] = tmp[pos - 1 - k];
        dnum_str[pos] = '\0';
    }
    int dnum_len = (int)l_strlen(dnum_str);

    for (;;) {
        uint8_t hdr[2];
        if (l_x11_read_full((int)fd, hdr, 2) < 0) break;
        /* family is big-endian uint16 */
        /* Read address */
        uint8_t lenbuf[2];
        if (l_x11_read_full((int)fd, lenbuf, 2) < 0) break;
        int alen = (lenbuf[0] << 8) | lenbuf[1];
        char abuf[256];
        if (alen > 255) { l_close(fd); return 0; }
        if (l_x11_read_full((int)fd, abuf, alen) < 0) break;
        /* Read display number */
        if (l_x11_read_full((int)fd, lenbuf, 2) < 0) break;
        int nlen = (lenbuf[0] << 8) | lenbuf[1];
        char nbuf[64];
        if (nlen > 63) { l_close(fd); return 0; }
        if (l_x11_read_full((int)fd, nbuf, nlen) < 0) break;
        /* Read protocol name */
        if (l_x11_read_full((int)fd, lenbuf, 2) < 0) break;
        int plen = (lenbuf[0] << 8) | lenbuf[1];
        char pbuf[64];
        if (plen > 63) { l_close(fd); return 0; }
        if (l_x11_read_full((int)fd, pbuf, plen) < 0) break;
        /* Read auth data */
        if (l_x11_read_full((int)fd, lenbuf, 2) < 0) break;
        int dlen = (lenbuf[0] << 8) | lenbuf[1];
        uint8_t dbuf[64];
        if (dlen > 64) { l_close(fd); return 0; }
        if (l_x11_read_full((int)fd, dbuf, dlen) < 0) break;

        /* Check if this matches our display */
        if (nlen == dnum_len && l_memcmp(nbuf, dnum_str, (size_t)nlen) == 0) {
            /* Check protocol is MIT-MAGIC-COOKIE-1 */
            if (plen == 18 && l_memcmp(pbuf, "MIT-MAGIC-COOKIE-1", 18) == 0) {
                int copy = dlen < cookie_sz ? dlen : cookie_sz;
                l_memcpy(cookie, dbuf, (size_t)copy);
                l_close(fd);
                return copy;
            }
        }
    }
    l_close(fd);
    return 0;
}

/* Connect to X server, perform setup handshake.
   On success: fills c->x11_fd, x11_wid, x11_gc, x11_root, x11_depth.
   Returns 0 on success, -1 on failure. */
static inline int l_x11_connect(L_Canvas *c, int width, int height, const char *title) {
    const char *disp = l_getenv("DISPLAY");
    char sockpath[128];
    int display_num = l_x11_parse_display(disp, sockpath, (int)sizeof(sockpath));
    if (display_num < 0) return -1;

    int fd = (int)l_socket_unix_connect(sockpath);
    if (fd < 0) return -1;

    /* Read Xauthority cookie */
    uint8_t cookie[16];
    int cookie_len = l_x11_read_xauth(display_num, cookie, 16);
    const char *auth_name = "MIT-MAGIC-COOKIE-1";
    int auth_name_len = cookie_len > 0 ? 18 : 0;
    int auth_data_len = cookie_len > 0 ? cookie_len : 0;

    /* Pad to 4-byte boundary */
    int name_pad = (4 - (auth_name_len % 4)) % 4;
    int data_pad = (4 - (auth_data_len % 4)) % 4;

    /* Build setup request */
    uint8_t setup[12];
    setup[0] = 0x6C; /* little-endian ('l') */
    setup[1] = 0;
    setup[2] = 11; setup[3] = 0; /* major version 11 */
    setup[4] = 0;  setup[5] = 0; /* minor version 0 */
    setup[6] = (uint8_t)(auth_name_len & 0xFF); setup[7] = (uint8_t)((auth_name_len >> 8) & 0xFF);
    setup[8] = (uint8_t)(auth_data_len & 0xFF); setup[9] = (uint8_t)((auth_data_len >> 8) & 0xFF);
    setup[10] = 0; setup[11] = 0;

    if (l_x11_write_full(fd, setup, 12) < 0) goto fail;
    if (auth_name_len > 0) {
        if (l_x11_write_full(fd, auth_name, auth_name_len) < 0) goto fail;
        if (name_pad > 0) { uint8_t pad[4] = {0}; if (l_x11_write_full(fd, pad, name_pad) < 0) goto fail; }
        if (l_x11_write_full(fd, cookie, auth_data_len) < 0) goto fail;
        if (data_pad > 0) { uint8_t pad[4] = {0}; if (l_x11_write_full(fd, pad, data_pad) < 0) goto fail; }
    }

    /* Read setup reply header */
    uint8_t reply_hdr[8];
    if (l_x11_read_full(fd, reply_hdr, 8) < 0) goto fail;
    if (reply_hdr[0] != 1) goto fail; /* 1 = success */

    /* Read additional data length (in 4-byte units) */
    uint16_t extra_len;
    l_memcpy(&extra_len, reply_hdr + 6, 2);
    int extra_bytes = (int)extra_len * 4;
    if (extra_bytes <= 0 || extra_bytes > 65536) goto fail;

    uint8_t *reply = (uint8_t *)l_mmap(0, (size_t)extra_bytes, L_PROT_READ | L_PROT_WRITE,
                                        L_MAP_PRIVATE | L_MAP_ANONYMOUS, -1, 0);
    if (reply == (uint8_t *)L_MAP_FAILED) goto fail;
    if (l_x11_read_full(fd, reply, extra_bytes) < 0) { l_munmap(reply, (size_t)extra_bytes); goto fail; }

    /* Parse setup reply:
       offset 0-3: release number
       offset 4-7: resource-id-base
       offset 8-11: resource-id-mask
       offset 12-15: motion-buffer-size
       offset 16-17: length of vendor
       offset 18-19: max request length
       offset 20: number of screens
       offset 21: number of formats
       ... vendor string (padded to 4) ...
       ... format list (8 bytes each) ...
       ... screens ... */
    uint32_t id_base, id_mask;
    l_memcpy(&id_base, reply + 4, 4);
    l_memcpy(&id_mask, reply + 8, 4);
    uint16_t vendor_len;
    l_memcpy(&vendor_len, reply + 16, 2);
    uint16_t max_req_len;
    l_memcpy(&max_req_len, reply + 18, 2);
    uint8_t num_screens = reply[20];
    uint8_t num_formats = reply[21];
    if (num_screens < 1) { l_munmap(reply, (size_t)extra_bytes); goto fail; }

    int vendor_pad = (4 - (vendor_len % 4)) % 4;
    int screen_offset = 24 + vendor_len + vendor_pad + num_formats * 8;
    if (screen_offset + 40 > extra_bytes) { l_munmap(reply, (size_t)extra_bytes); goto fail; }

    /* Parse first screen:
       offset 0-3: root window
       offset 4-7: default colormap
       offset 8-11: white pixel
       offset 12-15: black pixel
       ... skip some ...
       offset 20-21: width in pixels
       offset 22-23: height in pixels
       ... skip ...
       offset 28-29: root depth (at offset 38 in some versions)
       offset 38: root_depth */
    uint8_t *scr = reply + screen_offset;
    uint32_t root_wid;
    l_memcpy(&root_wid, scr, 4);
    uint8_t root_depth = scr[38];
    uint32_t root_visual = 0;
    /* root visual is at offset 32 */
    l_memcpy(&root_visual, scr + 32, 4);

    c->x11_fd = fd;
    c->x11_root = root_wid;
    c->x11_depth = (int)root_depth;
    c->x11_wid = id_base;
    c->x11_gc = id_base | 1;

    /* Store max request length for PutImage chunking */
    int max_req_bytes = (int)max_req_len * 4;
    (void)max_req_bytes;

    l_munmap(reply, (size_t)extra_bytes);

    /* CreateWindow: opcode 1 */
    {
        uint32_t vals[2];
        vals[0] = 1; /* event mask: override-redirect would go here but skip it */
        /* We want: KeyPress(1) | KeyRelease(2) | ButtonPress(4) | ButtonRelease(8) |
           PointerMotion(64) | Exposure(32768) | StructureNotify(131072) */
        vals[0] = 1 | 2 | 4 | 8 | 64 | 32768 | 131072;
        uint32_t value_mask = 0x800; /* event-mask */
        uint8_t req[32 + 4]; /* CreateWindow fixed=32 + 4 bytes for event mask value */
        int reqlen = 32 + 4;
        req[0] = 1; /* CreateWindow opcode */
        req[1] = root_depth; /* depth */
        uint16_t wlen = (uint16_t)(reqlen / 4);
        l_memcpy(req + 2, &wlen, 2);
        l_memcpy(req + 4, &c->x11_wid, 4); /* wid */
        l_memcpy(req + 8, &root_wid, 4); /* parent */
        int16_t x = 0, y = 0;
        l_memcpy(req + 12, &x, 2);
        l_memcpy(req + 14, &y, 2);
        uint16_t w = (uint16_t)width, h = (uint16_t)height;
        l_memcpy(req + 16, &w, 2);
        l_memcpy(req + 18, &h, 2);
        uint16_t bw = 0;
        l_memcpy(req + 20, &bw, 2);
        uint16_t wclass = 1; /* InputOutput */
        l_memcpy(req + 22, &wclass, 2);
        l_memcpy(req + 24, &root_visual, 4); /* visual — copy parent */
        l_memcpy(req + 28, &value_mask, 4);
        l_memcpy(req + 32, &vals[0], 4);
        if (l_x11_write_full(fd, req, reqlen) < 0) goto fail;
    }

    /* CreateGC: opcode 55 */
    {
        uint8_t req[16];
        req[0] = 55;
        req[1] = 0;
        uint16_t wlen = 4;
        l_memcpy(req + 2, &wlen, 2);
        l_memcpy(req + 4, &c->x11_gc, 4);
        l_memcpy(req + 8, &c->x11_wid, 4);
        uint32_t vmask = 0;
        l_memcpy(req + 12, &vmask, 4);
        if (l_x11_write_full(fd, req, 16) < 0) goto fail;
    }

    /* MapWindow: opcode 8 */
    {
        uint8_t req[8];
        req[0] = 8;
        req[1] = 0;
        uint16_t wlen = 2;
        l_memcpy(req + 2, &wlen, 2);
        l_memcpy(req + 4, &c->x11_wid, 4);
        if (l_x11_write_full(fd, req, 8) < 0) goto fail;
    }

    /* Set window title using ChangeProperty: opcode 18 */
    if (title && title[0]) {
        int tlen = (int)l_strlen(title);
        int tpad = (4 - (tlen % 4)) % 4;
        int reqlen = 24 + tlen + tpad;
        uint8_t req[280]; /* title up to 256 chars */
        if (reqlen > (int)sizeof(req)) reqlen = (int)sizeof(req);
        l_memset(req, 0, (size_t)reqlen);
        req[0] = 18; /* ChangeProperty */
        req[1] = 0;  /* Replace */
        uint16_t wlen = (uint16_t)(reqlen / 4);
        l_memcpy(req + 2, &wlen, 2);
        l_memcpy(req + 4, &c->x11_wid, 4);
        uint32_t prop = 39; /* WM_NAME atom */
        l_memcpy(req + 8, &prop, 4);
        uint32_t type = 31; /* STRING atom */
        l_memcpy(req + 12, &type, 4);
        uint8_t fmt = 8;
        req[16] = fmt;
        uint32_t datalen = (uint32_t)tlen;
        l_memcpy(req + 20, &datalen, 4);
        l_memcpy(req + 24, title, (size_t)tlen);
        if (l_x11_write_full(fd, req, reqlen) < 0) goto fail;
    }

    return 0;
fail:
    l_close(fd);
    return -1;
}

/* Process pending X11 events. Non-blocking. */
static inline void l_x11_pump_events(L_Canvas *c) {
    uint8_t ev[32];
    for (;;) {
        ssize_t n = l_read_nonblock(c->x11_fd, ev, 32);
        if (n < 32) break;
        uint8_t code = ev[0] & 0x7F; /* strip "generated" flag */
        switch (code) {
        case 2: /* KeyPress */ {
            uint8_t keycode = ev[1];
            /* Minimal keycode→keysym mapping (X11 keycodes: key = keycode - 8 for US layout) */
            int key = 0;
            if (keycode == 9) key = 27; /* Escape */
            else if (keycode == 36) key = 13; /* Return */
            else if (keycode == 22) key = 8; /* Backspace */
            else if (keycode == 23) key = 9; /* Tab */
            else if (keycode == 65) key = ' ';
            else if (keycode == 113) key = 1001; /* Left */
            else if (keycode == 114) key = 1002; /* Right */
            else if (keycode == 111) key = 1003; /* Up */
            else if (keycode == 116) key = 1004; /* Down */
            /* Letters: keycodes 24-33 = q,w,e,r,t,y,u,i,o,p */
            else if (keycode >= 24 && keycode <= 33) {
                static const char row1[] = "qwertyuiop";
                key = row1[keycode - 24];
            }
            /* keycodes 38-46 = a,s,d,f,g,h,j,k,l */
            else if (keycode >= 38 && keycode <= 46) {
                static const char row2[] = "asdfghjkl";
                key = row2[keycode - 38];
            }
            /* keycodes 52-58 = z,x,c,v,b,n,m */
            else if (keycode >= 52 && keycode <= 58) {
                static const char row3[] = "zxcvbnm";
                key = row3[keycode - 52];
            }
            /* Numbers: keycodes 10-19 = 1,2,...,9,0 */
            else if (keycode >= 10 && keycode <= 18) key = '0' + (keycode - 9);
            else if (keycode == 19) key = '0';
            /* Minus, equal, etc */
            else if (keycode == 20) key = '-';
            else if (keycode == 21) key = '=';
            else if (keycode == 34) key = '[';
            else if (keycode == 35) key = ']';
            else if (keycode == 47) key = ';';
            else if (keycode == 48) key = '\'';
            else if (keycode == 49) key = '`';
            else if (keycode == 51) key = '\\';
            else if (keycode == 59) key = ',';
            else if (keycode == 60) key = '.';
            else if (keycode == 61) key = '/';
            if (key) {
                int next = (c->key_head + 1) % 16;
                if (next != c->key_tail) {
                    c->keys[c->key_head] = key;
                    c->key_head = next;
                }
            }
            break;
        }
        case 4: /* ButtonPress */ {
            uint8_t btn = ev[1];
            if (btn == 1) c->mouse_btn |= 1;
            else if (btn == 2) c->mouse_btn |= 4;
            else if (btn == 3) c->mouse_btn |= 2;
            break;
        }
        case 5: /* ButtonRelease */ {
            uint8_t btn = ev[1];
            if (btn == 1) c->mouse_btn &= ~1;
            else if (btn == 2) c->mouse_btn &= ~4;
            else if (btn == 3) c->mouse_btn &= ~2;
            break;
        }
        case 6: /* MotionNotify */ {
            uint16_t mx, my;
            l_memcpy(&mx, ev + 24, 2); /* event-x */
            l_memcpy(&my, ev + 26, 2); /* event-y */
            c->mouse_x = (int)mx;
            c->mouse_y = (int)my;
            break;
        }
        case 17: /* DestroyNotify */
            c->closed = 1;
            break;
        case 33: /* ClientMessage — may be WM_DELETE_WINDOW */
            c->closed = 1;
            break;
        case 12: /* Expose — we always redraw on flush, ignore */
            break;
        case 22: /* ConfigureNotify — window resized or moved */ {
            uint16_t nw, nh;
            l_memcpy(&nw, ev + 20, 2); /* width */
            l_memcpy(&nh, ev + 22, 2); /* height */
            int new_w = (int)nw, new_h = (int)nh;
            if (new_w > 0 && new_h > 0 && (new_w != c->width || new_h != c->height)) {
                int ow = c->width, oh = c->height;
                int old_size = c->stride * c->height;
                int new_stride = new_w * 4;
                int new_size = new_stride * new_h;
                uint32_t *new_pixels = (uint32_t *)l_mmap(0, (size_t)new_size,
                    L_PROT_READ | L_PROT_WRITE, L_MAP_PRIVATE | L_MAP_ANONYMOUS, -1, 0);
                if (new_pixels != (uint32_t *)L_MAP_FAILED) {
                    /* Scale old content into new buffer (nearest-neighbor) */
                    if (c->pixels && c->pixels != (uint32_t *)L_MAP_FAILED && ow > 0 && oh > 0) {
                        for (int y = 0; y < new_h; y++) {
                            int sy = y * oh / new_h;
                            for (int x = 0; x < new_w; x++) {
                                int sx = x * ow / new_w;
                                new_pixels[y * new_w + x] = c->pixels[sy * ow + sx];
                            }
                        }
                    }
                    if (c->pixels && c->pixels != (uint32_t *)L_MAP_FAILED)
                        l_munmap(c->pixels, (size_t)old_size);
                    c->pixels  = new_pixels;
                    c->width   = new_w;
                    c->height  = new_h;
                    c->stride  = new_stride;
                    c->resized = 1;
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

/* Send pixel buffer to X server via PutImage. Handles chunking for large images. */
static inline void l_x11_put_image(L_Canvas *c) {
    int depth = c->x11_depth;
    int bpp = (depth <= 16) ? 2 : 4;
    int row_bytes = c->width * bpp;
    int row_pad = (4 - (row_bytes % 4)) % 4;
    int padded_row = row_bytes + row_pad;

    /* PutImage header is 24 bytes. Max request is 262140 bytes (65535 * 4).
       Send in horizontal bands. */
    int max_data = 262140 - 24;
    int rows_per_chunk = max_data / padded_row;
    if (rows_per_chunk < 1) rows_per_chunk = 1;
    if (rows_per_chunk > c->height) rows_per_chunk = c->height;

    for (int y = 0; y < c->height; y += rows_per_chunk) {
        int band_h = rows_per_chunk;
        if (y + band_h > c->height) band_h = c->height - y;
        int data_size = band_h * padded_row;
        int reqlen = 24 + data_size;
        uint16_t wlen = (uint16_t)(reqlen / 4);

        /* Build header */
        uint8_t hdr[24];
        hdr[0] = 72; /* PutImage */
        hdr[1] = 2;  /* ZPixmap */
        l_memcpy(hdr + 2, &wlen, 2);
        l_memcpy(hdr + 4, &c->x11_wid, 4);
        l_memcpy(hdr + 8, &c->x11_gc, 4);
        uint16_t w = (uint16_t)c->width, h = (uint16_t)band_h;
        l_memcpy(hdr + 12, &w, 2);
        l_memcpy(hdr + 14, &h, 2);
        int16_t dx = 0, dy = (int16_t)y;
        l_memcpy(hdr + 16, &dx, 2);
        l_memcpy(hdr + 18, &dy, 2);
        hdr[20] = 0; /* left-pad */
        hdr[21] = (uint8_t)depth;
        hdr[22] = 0; hdr[23] = 0;
        if (l_x11_write_full(c->x11_fd, hdr, 24) < 0) return;

        /* Convert and send pixel data row by row.
           Our pixels are ARGB (0xAARRGGBB). X11 ZPixmap at depth 24/32 on little-endian
           expects BGRX (B in byte 0, G in byte 1, R in byte 2, X/A in byte 3). */
        for (int row = 0; row < band_h; row++) {
            uint32_t *src = c->pixels + (y + row) * (c->stride / 4);
            if (bpp == 4) {
                /* ARGB → BGRA: swap R and B channels */
                uint8_t rowbuf[4096 * 4]; /* up to 4096 pixels wide */
                int rw = c->width;
                if (rw > 4096) rw = 4096;
                for (int x = 0; x < rw; x++) {
                    uint32_t p = src[x];
                    rowbuf[x * 4 + 0] = (uint8_t)(p);        /* B */
                    rowbuf[x * 4 + 1] = (uint8_t)(p >> 8);   /* G */
                    rowbuf[x * 4 + 2] = (uint8_t)(p >> 16);  /* R */
                    rowbuf[x * 4 + 3] = (uint8_t)(p >> 24);  /* A */
                }
                if (l_x11_write_full(c->x11_fd, rowbuf, rw * 4) < 0) return;
                if (row_pad > 0) {
                    uint8_t pad[4] = {0};
                    if (l_x11_write_full(c->x11_fd, pad, row_pad) < 0) return;
                }
            } else {
                /* 16-bit: convert ARGB → RGB565 */
                uint8_t rowbuf[4096 * 2];
                int rw = c->width;
                if (rw > 4096) rw = 4096;
                for (int x = 0; x < rw; x++) {
                    uint32_t p = src[x];
                    uint16_t c16 = (uint16_t)(((p >> 8) & 0xF800) | ((p >> 5) & 0x07E0) | ((p >> 3) & 0x001F));
                    rowbuf[x * 2 + 0] = (uint8_t)(c16);
                    rowbuf[x * 2 + 1] = (uint8_t)(c16 >> 8);
                }
                if (l_x11_write_full(c->x11_fd, rowbuf, rw * 2) < 0) return;
                if (row_pad > 0) {
                    uint8_t pad[4] = {0};
                    if (l_x11_write_full(c->x11_fd, pad, row_pad) < 0) return;
                }
            }
        }
    }
}

// ── Framebuffer open (extracted for backend selection) ────────────────────────

static inline int l_fb_canvas_open(L_Canvas *c, int width, int height) {
    c->fb_fd = (int)l_open("/dev/fb0", 2, 0);
    if (c->fb_fd < 0) return -1;

    struct l_fb_var_screeninfo vinfo;
    struct l_fb_fix_screeninfo finfo;
    l_memset(&vinfo, 0, sizeof(vinfo));
    l_memset(&finfo, 0, sizeof(finfo));

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    long vret = my_syscall3(__NR_ioctl, c->fb_fd, L_FBIOGET_VSCREENINFO, &vinfo);
    if (vret < 0) { l_close(c->fb_fd); return -1; }
    long fret = my_syscall3(__NR_ioctl, c->fb_fd, L_FBIOGET_FSCREENINFO, &finfo);
    if (fret < 0) { l_close(c->fb_fd); return -1; }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    c->width  = (width  > 0 && width  < (int)vinfo.xres) ? width  : (int)vinfo.xres;
    c->height = (height > 0 && height < (int)vinfo.yres) ? height : (int)vinfo.yres;
    c->stride = c->width * 4;
    c->fb_stride = (int)finfo.line_length;
    c->fb_bpp    = (int)vinfo.bits_per_pixel;
    c->fb_xoff   = (int)vinfo.xoffset;
    c->fb_yoff   = (int)vinfo.yoffset;

    int fb_size = (int)(finfo.line_length * vinfo.yres_virtual);
    if (fb_size <= 0) fb_size = (int)(finfo.line_length * vinfo.yres);
    c->fb_size = fb_size;
    c->fb_mem = (uint8_t *)l_mmap(0, (size_t)fb_size, L_PROT_READ | L_PROT_WRITE, L_MAP_SHARED, c->fb_fd, 0);
    if (c->fb_mem == (uint8_t *)L_MAP_FAILED) { l_close(c->fb_fd); return -1; }

    int pix_size = c->stride * c->height;
    c->pixels = (uint32_t *)l_mmap(0, (size_t)pix_size, L_PROT_READ | L_PROT_WRITE,
                                    L_MAP_PRIVATE | L_MAP_ANONYMOUS, -1, 0);
    if (c->pixels == (uint32_t *)L_MAP_FAILED) {
        l_munmap(c->fb_mem, (size_t)c->fb_size);
        l_close(c->fb_fd);
        return -1;
    }

    c->saved_tty = l_term_raw();
    c->mouse_fd = (int)l_open("/dev/input/mice", 0, 0);
    return 0;
}

// ── Public API implementations ───────────────────────────────────────────────

static inline int l_canvas_open(L_Canvas *c, int width, int height, const char *title) {
    l_memset(c, 0, sizeof(*c));
    c->mouse_fd = -1;

    /* Try X11 first if $DISPLAY is set */
    const char *disp = l_getenv("DISPLAY");
    if (disp && disp[0]) {
        if (width <= 0) width = 800;
        if (height <= 0) height = 600;
        c->width = width;
        c->height = height;
        c->stride = width * 4;

        /* Allocate pixel buffer */
        int pix_size = c->stride * c->height;
        c->pixels = (uint32_t *)l_mmap(0, (size_t)pix_size, L_PROT_READ | L_PROT_WRITE,
                                        L_MAP_PRIVATE | L_MAP_ANONYMOUS, -1, 0);
        if (c->pixels != (uint32_t *)L_MAP_FAILED) {
            if (l_x11_connect(c, width, height, title) == 0) {
                c->backend = 1;
                return 0;
            }
            l_munmap(c->pixels, (size_t)pix_size);
        }
    }

    /* Fall back to framebuffer */
    c->backend = 0;
    return l_fb_canvas_open(c, width, height);
}

static inline void l_canvas_close(L_Canvas *c) {
    if (c->backend == 1) {
        /* X11 cleanup */
        if (c->x11_fd > 0) l_close(c->x11_fd);
        if (c->pixels && c->pixels != (uint32_t *)L_MAP_FAILED)
            l_munmap(c->pixels, (size_t)(c->stride * c->height));
    } else {
        /* Framebuffer cleanup */
        if (c->mouse_fd >= 0) l_close(c->mouse_fd);
        if (c->saved_tty) l_term_restore(c->saved_tty);
        if (c->pixels && c->pixels != (uint32_t *)L_MAP_FAILED)
            l_munmap(c->pixels, (size_t)(c->stride * c->height));
        if (c->fb_mem && c->fb_mem != (uint8_t *)L_MAP_FAILED)
            l_munmap(c->fb_mem, (size_t)c->fb_size);
        if (c->fb_fd > 0) l_close(c->fb_fd);
    }
    l_memset(c, 0, sizeof(*c));
}

static inline int l_canvas_alive(L_Canvas *c) {
    if (c->backend == 1)
        return !c->closed && c->x11_fd > 0;
    return c->fb_mem != 0 && c->fb_mem != (uint8_t *)L_MAP_FAILED;
}

static inline void l_canvas_flush(L_Canvas *c) {
    if (c->backend == 1) {
        l_x11_pump_events(c);
        l_x11_put_image(c);
        return;
    }
    /* Framebuffer flush */
    if (!c->fb_mem || c->fb_mem == (uint8_t *)L_MAP_FAILED) return;
    for (int y = 0; y < c->height; y++) {
        uint32_t *src = c->pixels + y * (c->stride / 4);
        uint8_t  *dst = c->fb_mem + (y + c->fb_yoff) * c->fb_stride + c->fb_xoff * (c->fb_bpp / 8);
        if (c->fb_bpp == 32) {
            l_memcpy(dst, src, (size_t)(c->width * 4));
        } else if (c->fb_bpp == 16) {
            uint16_t *d16 = (uint16_t *)dst;
            for (int x = 0; x < c->width; x++) {
                uint32_t p = src[x];
                d16[x] = (uint16_t)(((p >> 8) & 0xF800) | ((p >> 5) & 0x07E0) | ((p >> 3) & 0x001F));
            }
        } else if (c->fb_bpp == 24) {
            for (int x = 0; x < c->width; x++) {
                uint32_t p = src[x];
                dst[x*3+0] = (uint8_t)(p);
                dst[x*3+1] = (uint8_t)(p >> 8);
                dst[x*3+2] = (uint8_t)(p >> 16);
            }
        }
    }
}

static inline void l_canvas_clear(L_Canvas *c, uint32_t color) {
    int n = c->width * c->height;
    for (int i = 0; i < n; i++)
        c->pixels[i] = color;
}

static inline int l_canvas_key(L_Canvas *c) {
    if (c->backend == 1) {
        l_x11_pump_events(c);
        if (c->key_head == c->key_tail) return 0;
        int key = c->keys[c->key_tail];
        c->key_tail = (c->key_tail + 1) % 16;
        return key;
    }
    /* Framebuffer: terminal raw mode */
    char buf[8];
    ssize_t n = l_read_nonblock(L_STDIN, buf, sizeof(buf));
    if (n <= 0) return 0;
    if (n == 1) return (unsigned char)buf[0];
    if (n >= 3 && buf[0] == '\033' && buf[1] == '[') {
        switch (buf[2]) {
        case 'A': return 1003;
        case 'B': return 1004;
        case 'C': return 1002;
        case 'D': return 1001;
        }
    }
    return (unsigned char)buf[0];
}

static inline int l_canvas_mouse(L_Canvas *c, int *x, int *y) {
    if (c->backend == 1) {
        l_x11_pump_events(c);
    } else if (c->mouse_fd >= 0) {
        uint8_t pkt[3];
        while (l_read_nonblock(c->mouse_fd, pkt, 3) == 3) {
            int dx = (int)pkt[1] - ((pkt[0] & 0x10) ? 256 : 0);
            int dy = -((int)pkt[2] - ((pkt[0] & 0x20) ? 256 : 0));
            c->mouse_x += dx;
            c->mouse_y += dy;
            if (c->mouse_x < 0) c->mouse_x = 0;
            if (c->mouse_y < 0) c->mouse_y = 0;
            if (c->mouse_x >= c->width)  c->mouse_x = c->width  - 1;
            if (c->mouse_y >= c->height) c->mouse_y = c->height - 1;
            c->mouse_btn = (pkt[0] & 1) | ((pkt[0] & 2) ? 2 : 0) | ((pkt[0] & 4) ? 4 : 0);
        }
    }
    if (x) *x = c->mouse_x;
    if (y) *y = c->mouse_y;
    return c->mouse_btn;
}

static inline int l_canvas_resized(L_Canvas *c) {
    int r = c->resized;
    c->resized = 0;
    return r;
}

#endif // _WIN32 / __unix__
#endif // L_WITHDEFS
