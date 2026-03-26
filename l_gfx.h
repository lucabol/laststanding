// l_gfx.h — Freestanding pixel graphics for laststanding
//
// Usage: #define L_MAINFILE
//        #include "l_gfx.h"   // pulls in l_os.h automatically
//
// Linux:   renders to /dev/fb0 (framebuffer console, no X11/Wayland)
// Windows: opens a native GDI window (user32 + gdi32, no external deps)

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
    // Linux framebuffer internals
    int       fb_fd;        // fd to /dev/fb0
    uint8_t  *fb_mem;       // mmap'd framebuffer
    int       fb_size;      // total mmap size
    int       fb_stride;    // framebuffer bytes per row (may differ from canvas stride)
    int       fb_bpp;       // framebuffer bits per pixel
    int       fb_xoff;      // x offset in virtual framebuffer
    int       fb_yoff;      // y offset in virtual framebuffer
    unsigned long saved_tty; // saved terminal mode for restore
    int       mouse_fd;    // fd to /dev/input/mice (-1 if unavailable)
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
            case VK_LEFT:  key = 1001; break;
            case VK_RIGHT: key = 1002; break;
            case VK_UP:    key = 1003; break;
            case VK_DOWN:  key = 1004; break;
            case VK_ESCAPE: key = 27;  break;
            case VK_RETURN: key = 13;  break;
            case VK_BACK:   key = 8;   break;
            default:
                key = (int)wp;
                if (key >= 'A' && key <= 'Z' && !(GetKeyState(VK_SHIFT) & 0x8000))
                    key += 32;  // lowercase
                break;
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
    case WM_MOUSEMOVE:
        if (c) { c->mouse_x = (short)LOWORD(lp); c->mouse_y = (short)HIWORD(lp); }
        return 0;
    case WM_LBUTTONDOWN: if (c) c->mouse_btn |=  1; return 0;
    case WM_LBUTTONUP:   if (c) c->mouse_btn &= ~1; return 0;
    case WM_RBUTTONDOWN: if (c) c->mouse_btn |=  2; return 0;
    case WM_RBUTTONUP:   if (c) c->mouse_btn &= ~2; return 0;
    case WM_MBUTTONDOWN: if (c) c->mouse_btn |=  4; return 0;
    case WM_MBUTTONUP:   if (c) c->mouse_btn &= ~4; return 0;
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
        : (DWORD)(WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX));

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

#else
// ═══════════════════════════════════════════════════════════════════════════════
// Linux framebuffer backend
// ═══════════════════════════════════════════════════════════════════════════════

// Framebuffer ioctl numbers
#define L_FBIOGET_VSCREENINFO 0x4600
#define L_FBIOGET_FSCREENINFO 0x4602

// Simplified fb_var_screeninfo (first 40 bytes are what we need)
struct l_fb_var_screeninfo {
    uint32_t xres;
    uint32_t yres;
    uint32_t xres_virtual;
    uint32_t yres_virtual;
    uint32_t xoffset;
    uint32_t yoffset;
    uint32_t bits_per_pixel;
    uint32_t grayscale;
    // We don't need the rest, but pad to safe ioctl size
    uint32_t pad[56];
};

// Simplified fb_fix_screeninfo
struct l_fb_fix_screeninfo {
    char     id[16];
    uint64_t smem_start;    // unsigned long on kernel side
    uint32_t smem_len;
    uint32_t type;
    uint32_t type_aux;
    uint32_t visual;
    uint16_t xpanstep;
    uint16_t ypanstep;
    uint16_t ywrapstep;
    uint32_t line_length;
    // Pad to safe size
    uint32_t pad[16];
};

static inline int l_canvas_open(L_Canvas *c, int width, int height, const char *title) {
    (void)title;
    l_memset(c, 0, sizeof(*c));

    // Try to open framebuffer
    c->fb_fd = (int)l_open("/dev/fb0", 2 /* O_RDWR */, 0);  // O_RDWR = 2
    if (c->fb_fd < 0) return -1;

    struct l_fb_var_screeninfo vinfo;
    struct l_fb_fix_screeninfo finfo;
    l_memset(&vinfo, 0, sizeof(vinfo));
    l_memset(&finfo, 0, sizeof(finfo));

    // GCC -Wpedantic warns about GNU statement expressions in my_syscall macros
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    long vret = my_syscall3(__NR_ioctl, c->fb_fd, L_FBIOGET_VSCREENINFO, &vinfo);
    if (vret < 0) {
        l_close(c->fb_fd);
        return -1;
    }
    long fret = my_syscall3(__NR_ioctl, c->fb_fd, L_FBIOGET_FSCREENINFO, &finfo);
    if (fret < 0) {
        l_close(c->fb_fd);
        return -1;
    }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    // Use requested size or framebuffer size, whichever is smaller
    c->width  = (width  > 0 && width  < (int)vinfo.xres) ? width  : (int)vinfo.xres;
    c->height = (height > 0 && height < (int)vinfo.yres) ? height : (int)vinfo.yres;
    c->stride = c->width * 4;
    c->fb_stride = (int)finfo.line_length;
    c->fb_bpp    = (int)vinfo.bits_per_pixel;
    c->fb_xoff   = (int)vinfo.xoffset;
    c->fb_yoff   = (int)vinfo.yoffset;

    // Map framebuffer
    int fb_size = (int)(finfo.line_length * vinfo.yres_virtual);
    if (fb_size <= 0) fb_size = (int)(finfo.line_length * vinfo.yres);
    c->fb_size = fb_size;
    c->fb_mem = (uint8_t *)l_mmap(0, (size_t)fb_size, L_PROT_READ | L_PROT_WRITE, L_MAP_SHARED, c->fb_fd, 0);
    if (c->fb_mem == (uint8_t *)L_MAP_FAILED) {
        l_close(c->fb_fd);
        return -1;
    }

    // Allocate pixel buffer (anonymous mmap)
    int pix_size = c->stride * c->height;
    c->pixels = (uint32_t *)l_mmap(0, (size_t)pix_size, L_PROT_READ | L_PROT_WRITE,
                                    L_MAP_PRIVATE | L_MAP_ANONYMOUS, -1, 0);
    if (c->pixels == (uint32_t *)L_MAP_FAILED) {
        l_munmap(c->fb_mem, (size_t)c->fb_size);
        l_close(c->fb_fd);
        return -1;
    }

    // Enter raw terminal mode for keyboard input
    c->saved_tty = l_term_raw();

    // Try to open PS/2 mouse device (failure is non-fatal)
    c->mouse_fd = (int)l_open("/dev/input/mice", 0 /* O_RDONLY */, 0);
    return 0;
}

static inline void l_canvas_close(L_Canvas *c) {
    if (c->mouse_fd >= 0) l_close(c->mouse_fd);
    if (c->saved_tty) l_term_restore(c->saved_tty);
    if (c->pixels && c->pixels != (uint32_t *)L_MAP_FAILED)
        l_munmap(c->pixels, (size_t)(c->stride * c->height));
    if (c->fb_mem && c->fb_mem != (uint8_t *)L_MAP_FAILED)
        l_munmap(c->fb_mem, (size_t)c->fb_size);
    if (c->fb_fd > 0) l_close(c->fb_fd);
    l_memset(c, 0, sizeof(*c));
}

static inline int l_canvas_alive(L_Canvas *c) {
    return c->fb_mem != 0 && c->fb_mem != (uint8_t *)L_MAP_FAILED;
}

static inline void l_canvas_flush(L_Canvas *c) {
    if (!c->fb_mem || c->fb_mem == (uint8_t *)L_MAP_FAILED) return;
    // Copy pixel buffer → framebuffer, handling stride and bpp differences
    for (int y = 0; y < c->height; y++) {
        uint32_t *src = c->pixels + y * (c->stride / 4);
        uint8_t  *dst = c->fb_mem + (y + c->fb_yoff) * c->fb_stride + c->fb_xoff * (c->fb_bpp / 8);
        if (c->fb_bpp == 32) {
            l_memcpy(dst, src, (size_t)(c->width * 4));
        } else if (c->fb_bpp == 16) {
            // Convert ARGB8888 → RGB565
            uint16_t *d16 = (uint16_t *)dst;
            for (int x = 0; x < c->width; x++) {
                uint32_t p = src[x];
                d16[x] = (uint16_t)(((p >> 8) & 0xF800) | ((p >> 5) & 0x07E0) | ((p >> 3) & 0x001F));
            }
        } else if (c->fb_bpp == 24) {
            for (int x = 0; x < c->width; x++) {
                uint32_t p = src[x];
                dst[x*3+0] = (uint8_t)(p);        // B
                dst[x*3+1] = (uint8_t)(p >> 8);   // G
                dst[x*3+2] = (uint8_t)(p >> 16);  // R
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
    (void)c;
    char buf[8];
    ssize_t n = l_read_nonblock(L_STDIN, buf, sizeof(buf));
    if (n <= 0) return 0;
    // Decode escape sequences for arrow keys
    if (n == 1) return (unsigned char)buf[0];
    if (n >= 3 && buf[0] == '\033' && buf[1] == '[') {
        switch (buf[2]) {
        case 'A': return 1003; // up
        case 'B': return 1004; // down
        case 'C': return 1002; // right
        case 'D': return 1001; // left
        }
    }
    return (unsigned char)buf[0];
}

static inline int l_canvas_mouse(L_Canvas *c, int *x, int *y) {
    if (c->mouse_fd >= 0) {
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

#endif // _WIN32 / __unix__
#endif // L_WITHDEFS
