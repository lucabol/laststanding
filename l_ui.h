// l_ui.h — Freestanding immediate-mode UI for laststanding
//
// Usage: #define L_MAINFILE
//        #include "l_ui.h"   // pulls in l_gfx.h (and l_os.h) automatically
//
// Immediate-mode: declare widgets every frame between l_ui_begin/l_ui_end.
// Widget functions return state (e.g. l_ui_button returns 1 if clicked).
// No heap allocation — all state in a single stack-allocated L_UI struct.

#ifndef L_UI_H
#define L_UI_H

#include "l_gfx.h"

// ---------------------------------------------------------------------------
// Theme
// ---------------------------------------------------------------------------

typedef struct {
    uint32_t bg;           // panel/widget background
    uint32_t bg_hover;     // hover state
    uint32_t bg_active;    // pressed/active state
    uint32_t fg;           // text color
    uint32_t fg_dim;       // dimmed text (labels, disabled)
    uint32_t border;       // widget border
    uint32_t accent;       // focused widget, slider thumb
    uint32_t input_bg;     // textbox background
} L_UI_Theme;

// Default dark theme
#define L_UI_THEME_DARK { \
    L_RGB(45, 45, 48),     /* bg */        \
    L_RGB(62, 62, 66),     /* bg_hover */  \
    L_RGB(37, 37, 38),     /* bg_active */ \
    L_RGB(220, 220, 220),  /* fg */        \
    L_RGB(150, 150, 150),  /* fg_dim */    \
    L_RGB(80, 80, 85),     /* border */    \
    L_RGB(0, 122, 204),    /* accent */    \
    L_RGB(30, 30, 30)      /* input_bg */  \
}

// ---------------------------------------------------------------------------
// Widget ID (FNV-1a hash for immediate-mode identity)
// ---------------------------------------------------------------------------

static inline uint32_t l_ui__hash(const char *s, int x, int y) {
    uint32_t h = 2166136261u;
    while (*s) { h ^= (uint8_t)*s++; h *= 16777619u; }
    h ^= (uint32_t)x * 2654435761u;
    h ^= (uint32_t)y * 2246822519u;
    return h ? h : 1;  // 0 means "no widget"
}

// ---------------------------------------------------------------------------
// Context
// ---------------------------------------------------------------------------

#define L_UI_MAX_TEXT 256   // max textbox content length

typedef struct {
    L_Canvas  *canvas;
    L_UI_Theme theme;

    // Input state (captured at l_ui_begin)
    int        mouse_x, mouse_y, mouse_btn;
    int        mouse_btn_prev;   // previous frame's button state
    int        key;              // last key press this frame

    // Widget state
    uint32_t   hot;       // widget under mouse cursor
    uint32_t   active;    // widget being interacted with (mouse held down)
    uint32_t   focused;   // widget with keyboard focus (for textbox)

    // Keyboard navigation
    int        tab_pressed;   // Tab was pressed this frame
    int        enter_pressed; // Enter was pressed this frame

    // Text editing state (shared — only one textbox active at a time)
    int        cursor_pos;    // cursor position in active textbox
    int        cursor_blink;  // blink counter

    // Layout cursor (for auto-layout helpers)
    int        layout_x, layout_y;
    int        layout_spacing;
    int        layout_dir;  // 0 = horizontal (row), 1 = vertical (col)
    int        layout_active;

    // Font scale: 1 = 8x8, 2 = 16x16
    int        font_scale;
} L_UI;

// ---------------------------------------------------------------------------
// Scaled text drawing helpers
// ---------------------------------------------------------------------------

static inline void l_ui__draw_text(L_Canvas *c, int x, int y, const char *text,
                                   uint32_t color, int scale) {
    if (scale <= 1) {
        l_draw_text(c, x, y, text, color);
        return;
    }
    // Draw each character scaled up
    while (*text) {
        if (*text >= 32 && *text <= 126) {
            const uint8_t *glyph = l_font8x8[*text - 32];
            for (int row = 0; row < 8; row++)
                for (int col = 0; col < 8; col++)
                    if (glyph[row] & (1 << col))
                        l_fill_rect(c, x + col * scale, y + row * scale,
                                    scale, scale, color);
        }
        x += 8 * scale;
        text++;
    }
}

static inline int l_ui__text_width(const char *text, int scale) {
    return (int)l_strlen(text) * 8 * scale;
}

static inline int l_ui__text_height(int scale) {
    return 8 * scale;
}

// ---------------------------------------------------------------------------
// Frame functions
// ---------------------------------------------------------------------------

/// Begins a UI frame. Call once per frame before declaring widgets.
static inline void l_ui_begin(L_UI *ui, L_Canvas *canvas) {
    ui->canvas = canvas;
    ui->mouse_btn_prev = ui->mouse_btn;
    ui->mouse_btn = l_canvas_mouse(canvas, &ui->mouse_x, &ui->mouse_y);
    ui->key = l_canvas_key(canvas);
    ui->hot = 0;  // reset — widgets will claim it
    ui->tab_pressed = (ui->key == '\t' || ui->key == 9);
    ui->enter_pressed = (ui->key == '\r' || ui->key == 13);
    ui->cursor_blink++;
}

/// Ends a UI frame. Handles releasing active widget when mouse released.
static inline void l_ui_end(L_UI *ui) {
    // If mouse released, clear active
    if (!(ui->mouse_btn & 1) && (ui->mouse_btn_prev & 1)) {
        ui->active = 0;
    }
    // If clicked but not on any widget, clear focus
    if ((ui->mouse_btn & 1) && !(ui->mouse_btn_prev & 1) && ui->hot == 0) {
        ui->focused = 0;
    }
}

/// Initializes a UI context with the default dark theme and font scale 1.
static inline void l_ui_init(L_UI *ui) {
    l_memset(ui, 0, sizeof(*ui));
    L_UI_Theme dark = L_UI_THEME_DARK;
    ui->theme = dark;
    ui->font_scale = 1;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static inline int l_ui__in_rect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px < x + w && py >= y && py < y + h;
}

// Was mouse just pressed this frame? (transition from up to down)
static inline int l_ui__mouse_pressed(L_UI *ui) {
    return (ui->mouse_btn & 1) && !(ui->mouse_btn_prev & 1);
}

// Was mouse just released this frame?
static inline int l_ui__mouse_released(L_UI *ui) {
    return !(ui->mouse_btn & 1) && (ui->mouse_btn_prev & 1);
}

#endif // L_UI_H
